#!/bin/bash

arg_install_dir=${HOME}/.xmc

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--install-dir)  arg_install_dir="$2"; shift; shift;;
        -*) echo "[ERROR] Unknown option $1"; exit 1;;
    esac
done

set -eux

mkdir -p ${arg_install_dir}
cd ${arg_install_dir}

uv venv .venv --python 3.11
source .venv/bin/activate

if [ -d "pico-sdk" ]; then
    echo "pico-sdk already exists, skipping cloning."
else
    git clone https://github.com/raspberrypi/pico-sdk.git
    pushd pico-sdk
    git submodule update --init --recursive
    popd
fi

if [ -d "pico-extras" ]; then
    echo "pico-extras already exists, skipping cloning."
else
    git clone https://github.com/raspberrypi/pico-extras.git
fi

if [ -d "platformuio" ]; then
    echo "PlatformIO already exists, skipping installation."
else
    export PLATFORMIO_CORE_DIR=${arg_install_dir}/platformuio
    curl -L -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
    python3 get-platformio.py
    rm -f get-platformio.py
fi

if [ -d "xiamocon" ]; then
    echo "Xiamocon SDK already exists, skipping cloning."
else
    git clone https://github.com/shapoco/xiamocon
    pushd xiamocon
    uv pip install -r "requirements.txt"
    popd
fi

setup_file=${arg_install_dir}/setup.shrc

rm -f ${setup_file}
echo "export PICO_SDK_PATH=${arg_install_dir}/pico-sdk" >> ${setup_file}
echo "export PICO_EXTRAS_PATH=${arg_install_dir}/pico-extras" >> ${setup_file}
echo "export PLATFORMIO_CORE_DIR=${arg_install_dir}/platformuio" >> ${setup_file}
echo "export XMC_REPO_PATH=${arg_install_dir}/xiamocon" >> ${setup_file}
echo "source ${arg_install_dir}/.venv/bin/activate" >> ${setup_file}

deactivate

set +x
echo "--------------------------------------------------"
echo "Installation completed!"
echo "--------------------------------------------------"
echo "To start developing Xiamocon, type:"
echo ""
echo "    source ${setup_file}"
echo ""
