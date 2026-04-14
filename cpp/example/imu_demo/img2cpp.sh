#!/bin/bash

set -eux

GLTF2CPP="python3 ${XMC_REPO_PATH}/bin/gltf2cpp"
IMG2CPP="python3 ${XMC_REPO_PATH}/bin/img2cpp"
MATERIAL_DIR="$(cd ${XMC_REPO_PATH}/material && pwd)"
OUT_DIR="$(pwd)/include"

${IMG2CPP} \
    ${MATERIAL_DIR}/texture/earth_surface.png \
    ${OUT_DIR}/earth_surface.hpp \
    --dither diffusion \
    --format rgb565

${IMG2CPP} \
    ${MATERIAL_DIR}/texture/earth_cloud.png \
    ${OUT_DIR}/earth_cloud.hpp \
    --dither diffusion \
    --format argb4444
