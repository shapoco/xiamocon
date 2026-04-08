#!/bin/bash

set -eu

source ${XMC_REPO_PATH}/setup.shrc

PLATFORMS=(
    rp2350_pico_sdk
    esp32s3_pio_arduino
)

for proj in *; do
    if [ -d "$proj" ]; then
        pushd "$proj" > /dev/null
            for platform in "${PLATFORMS[@]}"; do
                echo "Building $proj for platform $platform..."
                set +e
                xmc build -p "$platform" > "build_${platform}.log" 2>&1
                grep -E "error" "build_${platform}.log"
                set -e
            done
        popd > /dev/null
    fi
done
