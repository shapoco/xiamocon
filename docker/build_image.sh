#!/bin/bash

set -eux

cd ${XMC_REPO_PATH}
docker build \
    -t xiamocon:latest \
    -f docker/Dockerfile \
    .
