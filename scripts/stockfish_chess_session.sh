 2>&1 &
stockfish_pid=$!

# Open FD 3 for writing
exec 3> "$pipe_file"

# Initialization
echo "setoption name Threads value ${num_threads}" >&3
echo "isready" >&3

moves_history=""

# Function to clear the log file or mark position? 
# We can't easily clear, so we just tail from current position.
# We will use 'grep -m 1' on a tail stream to catch the next bestmove.

think() {
    # Construct position command
    if [[ -z "$moves_history" ]]; then
        cmd="position fen ${FEN}"
    else
        cmd="position fen ${FEN} moves${moves_history}"
    fi
    
    echo "$cmd" >&3
    echo "go movetime $((max_time_seconds * 1000))" >&3
    
    # Wait for bestmove
    # tail -n 0 -f starts reading from NOW. 
    # grep -m 1 waits for the first match then exits.
 [200~#!/bin/bash

# Default max time if not provided
if [[ ${1} =~ ^[0-9]+$ ]]; then
  max_time_seconds=${1} && shift
else
  max_time_seconds=300
fi

# Remaining args are the FEN
FEN="${*}"
if [[ -z "$FEN" ]]; then
    echo "Usage: $0 <seconds> <FEN>"
    exit 1
fi

set -e

# Setup Named Pipes and Logs
num_threads=$(nproc)
pipe_file=$(mktemp -u)
mkfifo "$pipe_file"
log_file=$(mktemp)

cleanup() {
    rm -f "$pipe_file" "$log_file"
    if [[ -n "$stockfish_pid" ]]; then
        kill "$stockfish_pid" 2>/dev/null || true
    fi
}
trap cleanup EXIT

echo "Starting Stockfish..."
echo "  Log: $log_file"
echo "  Threads: $num_threads"
echo "  Base FEN: $FEN"
echo "----------------------------------------"

# Start Stockfish
./stockfish < "$pipe_file" > "$log_file"   # This causes tail to receive SIGPIPE eventually and die.
    result=$(grep -m 1 "^bestmove" <(tail -n 0 -f "$log_file") || true)
    
    echo "Engine: $result"
}

# Main Interaction Loop
while true; do
    think
    
    read -e -p "Enter opponent move (or 'q' to quit): " user_move
    
    if [[ "$user_move" == "q" ]] || [[ "$user_move" == "quit" ]]; then
        break
    fi
    
    if [[ -n "$user_move" ]]; then
        moves_history="$moves_history $user_move"
    fi
done

echo "Exiting."
exec 3>&-[201~
