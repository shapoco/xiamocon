#!/bin/bash

set -e

if [ -n "$HOST_UID" ]; then
    groupadd -g $HOST_GID appuser || true
    useradd -u $HOST_UID -g $HOST_GID -o -m appuser || true

    # Install PlatformIO for current user
    if [ -z "${PLATFORMIO_CORE_DIR}" ]; then
        export PLATFORMIO_CORE_DIR=${XMC_REPO_PATH}/docker/.platformio
    fi
    if [ ! -e ${PLATFORMIO_CORE_DIR} ]; then
        mkdir -p ${PLATFORMIO_CORE_DIR}
        curl -L \
            -o get-platformio.py \
            https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
        python3 get-platformio.py
        rm -f get-platformio.py
        chown -R appuser:appuser ${PLATFORMIO_CORE_DIR}
    fi
    export PATH="${PATH}:${PLATFORMIO_CORE_DIR}/penv/bin"

    exec gosu appuser "$@"
else
    exec "$@"
fi
