#!/bin/bash
set -euo pipefail

echo "Configuring Postfix for direct delivery (no relay)..."
sudo postconf -e relayhost=''
sudo postconf -e myhostname=mail.hczhu.me
sudo postconf -e myorigin=hczhu.me
sudo postconf -e smtp_tls_security_level=may
sudo postconf -e smtp_tls_wrappermode=no

echo "Reloading Postfix..."
sudo postfix reload

echo "Flushing deferred queue..."
sudo postqueue -f

echo "Done. Watching mail log (Ctrl-C to stop)..."
tail -f /tmp/mail.log
