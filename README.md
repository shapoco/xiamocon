# [WIP] Xiamocon

![](./image/cover.jpg)

Game-console-style motherboard for XIAO RP2350/ESP32S3

> [!CAUTION]
> Everything is currently under development. Breaking changes may occur frequently without notice.

## Hardware Architecture

[Schematic](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fshapoco%2Fxiamocon%2Fblob%2Fmain%2Fhardware%2Fkicad%2Fxiamocon.kicad_sch)

![](./image/arch-hw.png)

## Development Environment Setup (Ubuntu24.04/macOS)

### 1. Install Build Tools

#### Install uv

See [Installing uv](https://docs.astral.sh/uv/getting-started/installation/).

#### Install other required packages

- Ubuntu24.04 (WSL2):

    ```sh
    sudo apt update
    sudo apt -y install curl git
    sudo apt -y install build-essential cmake gcc-arm-none-eabi
    ```

- macOS:

    First, install Command Line Developer Tools, [Homebrew](https://brew.sh/), and [CMake](https://cmake.org/download/), then run the following command:

    ```sh
    sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install
    brew install --cask gcc-arm-embedded
    ```

### 2. Install the Xiamocon SDK

```sh
curl -LsSf https://github.com/shapoco/xiamocon/raw/refs/heads/main/install.sh | bash
```

Python virtual enviroment, [Pico-SDK](https://github.com/raspberrypi/pico-sdk),
[Pico-Extras](https://github.com/raspberrypi/pico-extras), [PlatformIO](https://platformio.org/),
and this repository will be automatically installed in `${HOME}/.xmc/` directory.

### 3. Configure Environment Variables

To avoid setting the repository path every time, add the following line to your shell configuration file (e.g., `~/.bashrc`, `~/.zshrc`).

```sh
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

## Build and Run the Hello World Sample (Ubuntu24.04/macOS)

### For XIAO RP2350

1. Connect Xiamocon to your PC via USB and enter mass storage mode.
    1. Press and hold the Down button on Xiamocon.
    2. Press and hold the power button for 3 seconds to reset (it will be mounted as a mass storage device).
    3. Release the Down button.
2. Initialize the environment and move to the sample app directory.

    ```sh
    source ${HOME}/.xmc/setup.shrc
    cd ${XMC_REPO_PATH}
    cd cpp/example/hello_world
    ```

3. Build and flash the firmware.
    - On WSL2, you can specify the drive letter with the `-d` option to build and flash in one step (if the drive letter is E). You may be prompted for an administrator password.

        ```sh
        xmc run -p rp2350_pico_sdk -d E
        ```

    - On non-WSL2 environments, run `xmc build` instead of `xmc run`, then manually copy the generated UF2 file to the drive.

        ```sh
        xmc build -p rp2350_pico_sdk
        ```

### For XIAO ESP32S3

1. Connect Xiamocon to your PC via USB and check the serial port.
    - On WSL2, use [WSL USB Manager](https://github.com/nickbeth/wsl-usb-manager) or similar tools to attach Xiamocon's USB device to WSL2.
2. Initialize the environment and move to the sample app directory.

    ```sh
    source ${HOME}/.xmc/setup.shrc
    cd ${XMC_REPO_PATH}
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
> On ESP32S3, you can also build and flash using the PlatformIO extension in VSCode.

## Create a Project (Ubuntu24.04/macOS)

```sh
source ${HOME}/.xmc/setup.shrc
mkdir -p my_project
cd my_project
xmc init
cp -r ${XMC_REPO_PATH}/cpp/example/hello_world/src .
```

## Detailed Documentation

See the [Reference Manual](https://shapoco.github.io/xiamocon/ref/).

## Photos

![](./image/front.jpg)

![](./image/back.jpg)

## Videos

[![](./image/thumb-jumping-game.jpg)](https://x.com/shapoco/status/2041168789505794345)

[![](./image/thumb-3d-gfx.jpg)](https://x.com/shapoco/status/2038788120653824424)

## License

T.B.D.
