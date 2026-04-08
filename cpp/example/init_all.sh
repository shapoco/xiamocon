#!/bin/bash

set -eu

source ${XMC_REPO_PATH}/setup.shrc

PLATFORMS=(
    rp2350_pico_sdk
    esp32s3_pio_arduino
)

for proj in *; do
    if [ -d "$proj" ]; then
        if [ "$proj" != "hello_world" ]; then
            pushd "$proj" > /dev/null
                for platform in "${PLATFORMS[@]}"; do
                    echo "Initializing $proj for platform $platform..."
                    set +e
                    xmc init -p "$platform"
                    set -e
                done
            popd > /dev/null
        fi
    fi
done
