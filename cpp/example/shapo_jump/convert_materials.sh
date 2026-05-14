#!/bin/bash

set -eux

IMG2CPP="python3 ${XMC_REPO_PATH}/bin/img2cpp"
MATERIAL_DIR="$(pwd)/material"
OUT_DIR="$(pwd)/include"

${IMG2CPP} \
    ${MATERIAL_DIR}/bmp_bg.png \
    ${OUT_DIR}/bmp_bg.hpp \
    --format argb4444 \
    --key-color 808080

${IMG2CPP} \
    ${MATERIAL_DIR}/bmp_chara.png \
    ${OUT_DIR}/bmp_chara.hpp \
    --format argb4444 \
    --key-color 808080

${IMG2CPP} \
    ${MATERIAL_DIR}/bmp_shapo.png \
    ${OUT_DIR}/bmp_shapo.hpp \
    --format argb4444 \
    --key-color 808080
