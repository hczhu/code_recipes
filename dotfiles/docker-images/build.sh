#!/bin/bash

prog_name=$(basename "$0")
dir_name=$(cd "$(dirname "$0")" || exit 1; pwd -P)

cd ${dir_name}/..
DOCKER_BUILDKIT=1 docker build -t hczhu.dev -f docker-images/dev.Dockerfile . --build-arg BUILDKIT_INLINE_CACHE=1 ${*}
