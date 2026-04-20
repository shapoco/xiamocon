#!/bin/bash

set -eux

GLTF2CPP="python3 ${XMC_REPO_PATH}/bin/gltf2cpp"
IMG2CPP="python3 ${XMC_REPO_PATH}/bin/img2cpp"
MATERIAL_DIR="$(cd ${XMC_REPO_PATH}/material && pwd)"
OUT_DIR="$(pwd)/include"

${GLTF2CPP} \
    ${MATERIAL_DIR}/3d/tulip.glb \
    ${OUT_DIR}/tulip.hpp \
    --dither diffusion \
    --texformat rgb565

${GLTF2CPP} \
    ${MATERIAL_DIR}/3d/cave_hole.glb \
    ${OUT_DIR}/cave_hole.hpp \
    --dither diffusion \
    --texformat rgb565

${GLTF2CPP} \
    ${MATERIAL_DIR}/3d/cave_light.glb \
    ${OUT_DIR}/cave_light.hpp

${GLTF2CPP} \
    ${MATERIAL_DIR}/3d/quarts.glb \
    ${OUT_DIR}/quarts.hpp

${IMG2CPP} \
    ${MATERIAL_DIR}/texture/cave_env_texture.png \
    ${OUT_DIR}/cave_env_texture.hpp \
    --dither diffusion \
    --format rgb565
