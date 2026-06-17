#!/usr/bin/env bash
# Usage: ./run.sh [command...]
#   No args  — drops into an interactive bash shell
#   With args — runs the given command inside the container
#
# Examples:
#   ./run.sh
#   ./run.sh bash -c "cd golang && go test ./..."
#   ./run.sh bash -c "cd python && pytest test_async_crawler.py -n auto"
#   ./run.sh bash -c "cd ai-ml/mlight && make"
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(dirname "$SCRIPT_DIR")"

docker run -it --rm \
    -v "$REPO_DIR":/workspace \
    -w /workspace \
    code-recipes-dev \
    "${@:-bash}"
