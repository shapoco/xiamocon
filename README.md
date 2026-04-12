# [WIP] Xiamocon

![](./image/cover.jpg)

Game Console-Shaped Motherboard for XIAO RP2350/ESP32S3

> [!CAUTION]
> Everything is under construction. Breaking changes may occur frequently and without notice. 

## Hardware Architecture

[Schematic](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fshapoco%2Fxiamocon%2Fblob%2Fmain%2Fhardware%2Fkicad%2Fxiamocon.kicad_sch)

![](./image/arch-hw.png)

## Setup Development Environment (Linux/WSL2)

### 1. Install Build Tools

#### for RP2350

1. Install required packages.

    ```sh
    sudo apt update
    sudo apt install build-essential cmake git
    ```

2. Install [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/gnu-rm)

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    wget https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz
    tar xf arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz
    export PICO_TOOLCHAIN_PATH=${HOME}/.xmc/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin
    ```

3. Install [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set `PICO_SDK_PATH` environment variable.

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    git clone https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update --init --recursive
    export PICO_SDK_PATH=${HOME}/.xmc/pico-sdk
    ```

4. Install [Pico Extras](https://github.com/raspberrypi/pico-extras) and set `PICO_EXTRAS_PATH` environment variable.

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    git clone https://github.com/raspberrypi/pico-extras.git
    export PICO_EXTRAS_PATH=${HOME}/.xmc/pico-extras
    ```

#### for ESP32S3

1. Install required packages.

    ```sh
    sudo apt update
    sudo apt install python3 python3-venv
    ```

2. Install [PlatformIO](https://docs.platformio.org/).

    ```sh
    curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
    python3 get-platformio.py
    export PATH=$PATH:${HOME}/.platformio/penv/bin
    ```

### 2. Install Xiamocon SDK

```sh
cd ${HOME}/.xmc
git clone https://github.com/shapoco/xiamocon
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

### 3. Set environment variables

It is recommended to add the environment variables to your shell configuration file (e.g., `.bashrc`, `.zshrc`) to avoid setting them every time you open a new terminal session.

```sh
# for RP2350:
export PICO_TOOLCHAIN_PATH=${HOME}/.xmc/gcc-arm-none-eabi-10.3-2021.10/bin
export PICO_SDK_PATH=${HOME}/.xmc/pico-sdk
export PICO_EXTRAS_PATH=${HOME}/.xmc/pico-extras

# for ESP32S3:
export PATH=$PATH:${HOME}/.platformio/penv/bin

# for All
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

## Build and Run Hello World Example (Linux/WSL2)

1. Connect your Xiamocon to your computer via USB.
2. for XIAO RP2350:
    1. Press and hold DOWN key.
    2. hold POWER key for 3 seconds.
    3. release POWER key (Mass Storage device should be mounted).
    4. release DOWN key.
3. build and run the example.

    ```sh
    cd ${XMC_REPO_PATH}
    source ${XMC_REPO_PATH}/setup.shrc
    cd cpp/example/hello_world
    
    # for RP2350
    xmc build -p rp2350_pico_sdk
    # --> copy UF2 file to the mounted drive.

    # for ESP32S3
    xmc run -p esp32s3_pio_arduino
    ```

for ESP32S3, you can also use PlatformIO extension in VSCode to build and upload.

## Creating Your Project (Linux/WSL2)

```sh
source ${XMC_REPO_PATH}/setup.shrc
mkdir -p my_project
cd my_project
xmc init
cp -r $XMC_REPO_PATH/cpp/example/hello_world/src .
```

## Pictures

![](./image/front.jpg)

![](./image/back.jpg)

## Videos

[![](./image/thumb-jumping-game.jpg)](https://x.com/shapoco/status/2041168789505794345)

[![](./image/thumb-3d-gfx.jpg)](https://x.com/shapoco/status/2038788120653824424)
