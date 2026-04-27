# [WIP] Xiamocon

![](./image/cover.jpg)

Game-console-style motherboard for XIAO RP2350/ESP32S3

> [!CAUTION]
> Everything is currently under development. Breaking changes may occur frequently without notice.

## Hardware Architecture

[Schematic](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fshapoco%2Fxiamocon%2Fblob%2Fmain%2Fhardware%2Fkicad%2Fxiamocon.kicad_sch)

![](./image/arch-hw.png)

## Development Environment Setup (Linux/WSL2)

### 1. Install Build Tools

#### Install required packages

```sh
sudo apt update
sudo apt -y install curl git
sudo apt -y install build-essential cmake gcc-arm-none-eabi
sudo apt -y install python3 python3-pip python3-venv
```

#### For XIAO RP2350

1. Install [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set the `PICO_SDK_PATH` environment variable.

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    git clone https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update --init --recursive
    export PICO_SDK_PATH=${HOME}/.xmc/pico-sdk
    ```

2. Install [Pico Extras](https://github.com/raspberrypi/pico-extras) and set the `PICO_EXTRAS_PATH` environment variable.

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    git clone https://github.com/raspberrypi/pico-extras.git
    export PICO_EXTRAS_PATH=${HOME}/.xmc/pico-extras
    ```

#### For XIAO ESP32S3

Install [PlatformIO](https://docs.platformio.org/) and update the `PATH` environment variable.

```sh
curl -L -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
export PATH=$PATH:${HOME}/.platformio/penv/bin
```

### 2. Install the Xiamocon SDK

```sh
cd ${HOME}/.xmc
git clone https://github.com/shapoco/xiamocon
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

### 3. Configure Environment Variables

To avoid setting these every time you open a new terminal, it is recommended to add the following environment variables to your shell configuration file (e.g., `.bashrc`, `.zshrc`).

```sh
# For RP2350:
export PICO_SDK_PATH=${HOME}/.xmc/pico-sdk
export PICO_EXTRAS_PATH=${HOME}/.xmc/pico-extras

# For ESP32S3:
export PATH=$PATH:${HOME}/.platformio/penv/bin

# Common
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

## Build and Run the Hello World Sample (Linux/WSL2)

### For XIAO RP2350

1. Connect Xiamocon to your PC via USB and enter mass storage mode.
    1. Press and hold the Down button on Xiamocon.
    2. Press and hold the power button for 3 seconds to reset (it will be mounted as a mass storage device).
    3. Release the Down button.
2. Initialize the environment and move to the sample app directory.

    ```sh
    cd ${XMC_REPO_PATH}
    source setup.shrc
    cd cpp/example/hello_world
    ```

3. Build and flash the firmware.
    - On WSL, you can specify the drive letter with the `-d` option to build and flash in one step (if the drive letter is E). You may be prompted for an administrator password.

        ```sh
        xmc run -p rp2350_pico_sdk -d E
        ```

    - On non-WSL environments, run `xmc build` instead of `xmc run`, then manually copy the generated UF2 file to the drive.

        ```sh
        xmc build -p rp2350_pico_sdk
        ```

### For XIAO ESP32S3

1. Connect Xiamocon to your PC via USB and check the serial port.
    - On WSL, use [WSL USB Manager](https://github.com/nickbeth/wsl-usb-manager) or similar tools to attach Xiamocon's USB device to WSL.
2. Initialize the environment and move to the sample app directory.

    ```sh
    cd ${XMC_REPO_PATH}
    source setup.shrc
    cd cpp/example/hello_world
    ```

3. Build and flash while specifying the serial port with the `-s` option.

    ```sh
    xmc run -p esp32s3_pio_arduino -s /dev/ttyACM0
    ```

    If you want to use PlatformIO's default serial port, you can omit the `-s` option.

>[!NOTE]
> - You can omit the `-p` option by setting the platform in the `XMC_DEFAULT_PLATFORM` environment variable.
> - You can omit the `-d` option by setting the drive letter in the `XMC_DEFAULT_DRIVE` environment variable.
> - You can omit the `-s` option by setting the serial port in the `XMC_DEFAULT_SERIAL` environment variable.

>[!NOTE]
> On ESP32S3, you can also build and flash using the PlatformIO extension in VS Code.

## Create a Project (Linux/WSL2)

```sh
source ${XMC_REPO_PATH}/setup.shrc
mkdir -p my_project
cd my_project
xmc init
cp -r ${XMC_REPO_PATH}/cpp/example/hello_world/src .
```

## Photos

![](./image/front.jpg)

![](./image/back.jpg)

## Videos

[![](./image/thumb-jumping-game.jpg)](https://x.com/shapoco/status/2041168789505794345)

[![](./image/thumb-3d-gfx.jpg)](https://x.com/shapoco/status/2038788120653824424)

## License

T.B.D.
