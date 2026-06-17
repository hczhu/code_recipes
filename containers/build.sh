#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(dirname "$SCRIPT_DIR")"

docker build \
    -t code-recipes-dev \
    -f "$SCRIPT_DIR/dev-container.Dockerfile" \
    "$REPO_DIR"
