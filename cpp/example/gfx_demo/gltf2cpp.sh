#!/bin/bash

set -eux

TOOL_DIR="$(cd ../../../tool && pwd)"
MATERIAL_DIR="$(cd ../../../material && pwd)"
OUT_DIR="$(pwd)/include"
GLTF2CPP="python3 ${TOOL_DIR}/gltf2cpp/gltf2cpp.py"

${GLTF2CPP} ${MATERIAL_DIR}/3d/tulip.glb ${OUT_DIR}/tulip.hpp

