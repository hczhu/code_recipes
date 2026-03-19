#!/usr/bin/env bash
# Usage: app_network_domains.sh [-m <seconds>] <app-name>
#
# Shows domains a running app communicates with.
#   Default : lists current connections (reverse DNS + system DNS cache).
#   -m <N>  : monitor mode — sniff live TLS SNI for N seconds (default 15).
#             Requires access_bpf group membership (see README).
#
# Requires: lsof, dscacheutil, dig, tcpdump, python3 (all on macOS by default)

MONITOR=0
DURATION=15

while getopts "m:" opt; do
  case $opt in
    m) MONITOR=1; DURATION="$OPTARG" ;;
    *) echo "Usage: $0 [-m <seconds>] <app-name>" >&2; exit 1 ;;
  esac
done
shift $((OPTIND - 1))

APP="${1:-}"
if [[ -z "$APP" ]]; then
  echo "Usage: $0 [-m <seconds>] <app-name>" >&2
  exit 1
fi

# Find PIDs — exact match first, then partial
PIDS=$(pgrep -xi "$APP" 2>/dev/null || pgrep -i "$APP" 2>/dev/null || true)
if [[ -z "$PIDS" ]]; then
  echo "No running process found for: $APP" >&2
  exit 1
fi

PID_LIST=$(echo "$PIDS" | tr '\n' ',' | sed 's/,$//')

# Get active network connections via lsof
# -i: network files  -n: numeric IPs  -P: numeric ports  -a -p: filter by PID
RAW=$(lsof -i -n -P -a -p "$PID_LIST" 2>/dev/null || true)
if [[ -z "$RAW" ]]; then
  echo "No active network connections found for '$APP'."
  exit 0
fi

# Parse lsof NAME column (e.g. "1.2.3.4:5678->9.10.11.12:443 (ESTABLISHED)")
# Split on "->", strip trailing :port, skip loopback/wildcard
ADDRS=$(echo "$RAW" | awk 'NR>1 {
  name = ""
  for (i = 1; i <= NF; i++) {
    if ($i ~ /->/ || $i ~ /^[0-9].*:[0-9]/ || $i ~ /^\[/) { name = $i; break }
  }
  if (name == "") next
  n = split(name, sides, "->")
  for (i = 1; i <= n; i++) {
    addr = sides[i]
    if (addr ~ /^\[/) {
      sub(/^\[/, "", addr); sub(/\]:[0-9]+$/, "", addr)
    } else {
      sub(/:[0-9]+$/, "", addr)
    }
    if (addr ~ /^\*$/ || addr == "" || addr ~ /^127\./ || \
        addr == "::1" || addr ~ /^fe80/ || addr ~ /^0\.0\.0\.0/) next
    print addr
  }
}' | sort -u)

if [[ -z "$ADDRS" ]]; then
  echo "No external network addresses found for '$APP'."
  exit 0
fi

# ── Monitor mode: capture TLS SNI from live traffic ──────────────────────────
if [[ "$MONITOR" -eq 1 ]]; then
  # Detect the default outbound interface (avoids macOS DLT_PKTAP from 'any')
  IFACE=$(route get default 2>/dev/null | awk '/interface:/{print $2}')
  IFACE="${IFACE:-en0}"

  TMPPCAP=$(mktemp /tmp/app_sniff_XXXXXX.pcap)
  trap "rm -f '$TMPPCAP'" EXIT

  # Build BPF filter using the app's local source ports — these are unique per
  # connection and effectively restrict capture to this app's traffic only.
  SRC_PORTS=$(echo "$RAW" | awk 'NR>1 {
    for (i = 1; i <= NF; i++) {
      if ($i ~ /->/) {
        split($i, sides, "->")
        n = split(sides[1], parts, ":")
        print parts[n]   # local (source) port is last field
        break
      }
    }
  }' | sort -u)

  if [[ -z "$SRC_PORTS" ]]; then
    echo "No active outbound connections found for '$APP'." >&2
    exit 1
  fi

  PORT_FILTER=$(echo "$SRC_PORTS" | awk 'BEGIN{f=""} {f=f (NR>1?" or ":"") "src port "$1} END{print f}')
  BPF_FILTER="tcp port 443 and ($PORT_FILTER)"

  echo "Capturing on $IFACE for ${DURATION}s..."

  tcpdump -ni "$IFACE" "$BPF_FILTER" -w "$TMPPCAP" 2>/dev/null &
  T=$!; sleep "$DURATION"; kill $T 2>/dev/null; wait $T 2>/dev/null

  if [[ ! -s "$TMPPCAP" ]]; then
    echo "No packets captured on $IFACE. Try a different interface or check sudo access."
    exit 1
  fi

  DOMAINS=$(python3 - "$TMPPCAP" <<'PYEOF'
import sys, struct

path = sys.argv[1]
with open(path, 'rb') as f:
    data = f.read()

if len(data) < 24:
    sys.exit(0)

# pcap global header
magic = struct.unpack_from('<I', data, 0)[0]
endian = '<' if magic == 0xa1b2c3d4 else '>'
link_type = struct.unpack_from(endian + 'I', data, 20)[0]

def extract_sni(tls):
    """Return SNI hostname from a TLS ClientHello byte string, or None."""
    i = 0
    while i < len(tls) - 9:
        if tls[i:i+2] == b'\x16\x03' and tls[i+5:i+6] == b'\x01':
            try:
                p = i + 9               # past record hdr(5) + HS type(1) + HS len(3)
                p += 2 + 32             # client version + random
                sl = tls[p]; p += 1 + sl
                cl = struct.unpack_from('>H', tls, p)[0]; p += 2 + cl
                ml = tls[p]; p += 1 + ml
                if p + 2 > len(tls): break
                etotal = struct.unpack_from('>H', tls, p)[0]; p += 2
                eend = p + etotal
                while p + 4 <= eend:
                    et = struct.unpack_from('>H', tls, p)[0]
                    el = struct.unpack_from('>H', tls, p+2)[0]
                    if et == 0 and p + 9 <= len(tls):   # SNI
                        nl = struct.unpack_from('>H', tls, p+7)[0]
                        return tls[p+9:p+9+nl].decode('ascii', 'ignore')
                    p += 4 + el
            except Exception:
                pass
        i += 1
    return None

domains = set()
pos = 24  # skip global header
while pos + 16 <= len(data):
    incl_len = struct.unpack_from(endian + 'I', data, pos + 8)[0]
    pkt = data[pos + 16: pos + 16 + incl_len]
    pos += 16 + incl_len

    # Strip link-layer header to reach IP/TCP payload
    if link_type == 1:      # Ethernet: 14-byte header
        offset = 14
        if len(pkt) <= offset: continue
        ethertype = struct.unpack_from('>H', pkt, 12)[0]
        if ethertype != 0x0800: continue   # IPv4 only
    elif link_type == 0:    # BSD loopback: 4-byte header
        offset = 4
    else:
        offset = 0          # Unknown: scan from start

    ip = pkt[offset:]
    if len(ip) < 20: continue
    ihl = (ip[0] & 0x0f) * 4
    if ip[9] != 6: continue          # TCP only
    tcp = ip[ihl:]
    if len(tcp) < 20: continue
    tcp_hdr_len = ((tcp[12] >> 4) & 0xf) * 4
    tls_payload = tcp[tcp_hdr_len:]
    if not tls_payload: continue

    sni = extract_sni(tls_payload)
    if sni:
        domains.add(sni)

for d in sorted(domains):
    print(d)
PYEOF
)

  if [[ -z "$DOMAINS" ]]; then
    echo "No TLS SNI captured. See the NOTE above about new connections."
  else
    echo "Domains captured via TLS SNI:"
    echo "$DOMAINS" | sed 's/^/  /'
  fi
  exit 0
fi

# ── Default mode: current connections with DNS resolution ─────────────────────
# Try macOS DNS cache first (forward mappings: domain -> IP), then reverse DNS.
# Note: apps using DNS-over-HTTPS (e.g. Chrome) bypass the system cache;
#       use -m mode for those.
CACHE=$(dscacheutil -cachedump -entries Host 2>/dev/null)

COUNT=$(echo "$ADDRS" | wc -l | tr -d ' ')
echo "Current connections ($COUNT unique addresses) — DNS cache then reverse DNS:"
echo "(For browsers using DoH, use -m <seconds> to capture live TLS SNI)"
echo ""
printf "%-50s %s\n" "DOMAIN / HOSTNAME" "IP ADDRESS"
printf "%-50s %s\n" "--------------------------------------------------" "---------------"

while IFS= read -r IP; do
  # 1. Forward DNS cache lookup (works for system-resolver apps)
  HOST=$(echo "$CACHE" | awk -v target="$IP" '
    /name:/           { name = $2 }
    /(ipv4|ipv6)_address:/ && $2 == target { print name; exit }
  ')
  # 2. Reverse DNS fallback
  if [[ -z "$HOST" ]]; then
    HOST=$(dig +short +time=2 +tries=1 -x "$IP" 2>/dev/null | sed 's/\.$//' | head -1)
  fi
  printf "%-50s %s\n" "${HOST:-(no reverse DNS)}" "$IP"
done <<< "$ADDRS"
