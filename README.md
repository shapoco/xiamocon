# [WIP] Xiamocon

![](./image/cover.jpg)

Game Console-Shaped Motherboard for XIAO RP2350/ESP32S3

> [!CAUTION]
> Everything is under construction. Breaking changes may occur frequently and without notice. 

## Hardware Architecture

[Schematic](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fshapoco%2Fxiamocon%2Fblob%2Fmain%2Fhardware%2Fkicad%2Fxiamocon.kicad_sch)

![](./image/arch-hw.png)

## Setup Development Environment (Linux/WSL2)

### for RP2350

1. Install [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set `PICO_SDK_PATH` environment variable.
2. Install [Pico Extras](https://github.com/raspberrypi/pico-extras) and set `PICO_EXTRAS_PATH` environment variable.

### for ESP32S3

Install [PlatformIO](https://docs.platformio.org/).

```bash
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
```

### Install Xiamocon SDK

```bash
mkdir -p ${HOME}
cd ${HOME}
git clone https://github.com/shapoco/xiamocon
```

> [!TIP]
> It is recommended to set the `XMC_REPO_PATH` environment variable in `~/.bashrc` or similar.
>
> ```bash
> export XMC_REPO_PATH=${HOME}/xiamocon
> ```

## Creating Your Project (Linux/WSL2)

```bash
export XMC_REPO_PATH=${HOME}/xiamocon
source ${XMC_REPO_PATH}/setup.shrc
mkdir -p my_project
cd my_project
xmc init
cp -r $XMC_REPO_PATH/cpp/example/hello_world/src .
# for RP2350:
xmc build -p rp2350_pico_sdk
# for ESP32S3:
xmc run -p esp32s3_pio_arduino
```

## Pictures

![](./image/front.jpg)

![](./image/back.jpg)

## Videos

[![](./image/thumb-jumping-game.jpg)](https://x.com/shapoco/status/2041168789505794345)

[![](./image/thumb-3d-gfx.jpg)](https://x.com/shapoco/status/2038788120653824424)
