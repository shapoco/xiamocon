#!/bin/bash

IMG2CPP=${XMC_REPO_PATH}/bin/img2cpp
SRC_DIR=${XMC_REPO_PATH}/material
OUT_DIR=./include

set -eux

${IMG2CPP} -f argb4444 ${SRC_DIR}/icon/icon16_folder.png ${OUT_DIR}/icon16_folder.hpp
${IMG2CPP} -f argb4444 ${SRC_DIR}/icon/icon16_file.png ${OUT_DIR}/icon16_file.hpp
