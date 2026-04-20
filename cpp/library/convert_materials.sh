#!/bin/bash

set -eux

IMG2CPP="python3 ${XMC_REPO_PATH}/bin/img2cpp"
MATERIAL_DIR="$(pwd)/material"
OUT_DIR="$(pwd)/include/xmc"

${IMG2CPP} \
    ${MATERIAL_DIR}/power_off_message.png \
    ${OUT_DIR}/power_off_message.hpp \
    --format gray1
