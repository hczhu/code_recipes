#!/bin/bash
# start.sh — start the Bedrock server with a named pipe for sending commands.
# Usage: ./start.sh
#
# To send a command to the running server from another terminal:
#   echo "say Hello!" > /tmp/mc_input
#   echo "setworldspawn 304 95 -25" > /tmp/mc_input

set -euo pipefail

SERVER_DIR="$(cd "$(dirname "$0")" && pwd)"
INPUT_PIPE=./mc_input

echo "============================================"
echo " Minecraft Bedrock Server Startup"
echo " $(date)"
echo "============================================"
echo ""
echo "Input pipe: $INPUT_PIPE"
echo "Send commands with:  echo \"<command>\" > $INPUT_PIPE"
echo ""

# Create the named pipe if it doesn't exist
[[ -p "$INPUT_PIPE" ]] || mkfifo "$INPUT_PIPE"

cd "$SERVER_DIR"

# After the server is ready, feed startup_commands.txt into the pipe.
STARTUP_CMDS="$SERVER_DIR/startup_commands.txt"
if [[ -f "$STARTUP_CMDS" ]]; then
    (
        sleep 5
        echo "Sending startup commands from $STARTUP_CMDS..."
        while IFS= read -r cmd || [[ -n "$cmd" ]]; do
            [[ -z "$cmd" || "$cmd" == \#* ]] && continue   # skip blanks/comments
            echo "  > $cmd"
            echo "$cmd" > "$INPUT_PIPE"
            sleep 0.2
        done < "$STARTUP_CMDS"
    ) &
fi

# tail -f keeps the pipe open so the server never sees EOF
tail -f "$INPUT_PIPE" | ./bedrock_server 2>&1 | tee server.logs
