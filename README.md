# [WIP] Xiamocon

![](./image/cover.jpg)

XIAO RP2350/ESP32S3 向けのゲーム機型マザーボード

> [!CAUTION]
> すべては現在開発中です。破壊的変更が予告なく頻繁に行われます。

## ハードウェア構成

[回路図](https://kicanvas.org/?repo=https%3A%2F%2Fgithub.com%2Fshapoco%2Fxiamocon%2Fblob%2Fmain%2Fhardware%2Fkicad%2Fxiamocon.kicad_sch)

![](./image/arch-hw.png)

## 開発環境のセットアップ（Linux/WSL2）

### 1. ビルドツールのインストール

#### 必要なパッケージをインストール

```sh
sudo apt update
sudo apt -y install curl git
sudo apt -y install build-essential cmake gcc-arm-none-eabi
sudo apt -y install python3 python3-pip python3-venv
```

#### XIAO RP2350 の場合

1. [Pico SDK](https://github.com/raspberrypi/pico-sdk) をインストールし、環境変数 `PICO_SDK_PATH` を設定します。

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    git clone https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update --init --recursive
    export PICO_SDK_PATH=${HOME}/.xmc/pico-sdk
    ```

2. [Pico Extras](https://github.com/raspberrypi/pico-extras) をインストールし、環境変数 `PICO_EXTRAS_PATH` を設定します。

    ```sh
    mkdir -p ${HOME}/.xmc
    cd ${HOME}/.xmc
    git clone https://github.com/raspberrypi/pico-extras.git
    export PICO_EXTRAS_PATH=${HOME}/.xmc/pico-extras
    ```

#### XIAO ESP32S3 の場合

[PlatformIO](https://docs.platformio.org/) をインストールし、環境変数 `PATH` を設定します。

```sh
curl -L -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
export PATH=$PATH:${HOME}/.platformio/penv/bin
```

### 2. Xiamocon SDK のインストール

```sh
cd ${HOME}/.xmc
git clone https://github.com/shapoco/xiamocon
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

### 3. 環境変数の設定

新しいターミナルを開くたびに設定する手間を避けるため、以下の環境変数はシェル設定ファイル（例: `.bashrc`, `.zshrc`）に追記することを推奨します。

```sh
# RP2350 向け:
export PICO_SDK_PATH=${HOME}/.xmc/pico-sdk
export PICO_EXTRAS_PATH=${HOME}/.xmc/pico-extras

# ESP32S3 向け:
export PATH=$PATH:${HOME}/.platformio/penv/bin

# 共通
export XMC_REPO_PATH=${HOME}/.xmc/xiamocon
```

## Hello World サンプルのビルドと実行（Linux/WSL2）

### XIAO RP2350 の場合

1. Xiamocon を USB で PC に接続し、マスストレージモードにします。
    1. Xiamocon の↓キーを押したままにします。
    2. 電源ボタンを 3 秒間押してリセットします (マスストレージデバイスとしてマウントされます)。
    3. ↓キーを離します。
2. 環境の初期設定を行い、サンプルアプリのディレクトリに移動します。

    ```sh
    cd ${XMC_REPO_PATH}
    source ${XMC_REPO_PATH}/setup.shrc
    cd cpp/example/hello_world
    ```

3. ビルドして書き込みを実行します。
    - WSL の場合は `-d` オプションでドライブレターを指定してビルドと書き込みを一度に行うことができます (ドライブレターが E の場合)。管理者パスワードの入力が求められることがあります。

        ```sh
        xmc run -p rp2350_pico_sdk -d E
        ```

    - WSL 以外の場合は `xmc run` の代わりに `xmc build` でビルドを行い、生成された UF2 ファイルを手動でドライブにコピーします。

        ```sh
        xmc build -p rp2350_pico_sdk
        ```

### XIAO ESP32S3 の場合

1. Xiamocon を USB で PC に接続しシリアルポートを確認します。
    - WSL の場合は [WSL USB Manager](https://github.com/nickbeth/wsl-usb-manager) などを使用して Xiamocon の USB デバイスを WSL にアタッチしてください。
2. 環境の初期設定を行い、サンプルアプリのディレクトリに移動します。

    ```sh
    cd ${XMC_REPO_PATH}
    source ${XMC_REPO_PATH}/setup.shrc
    cd cpp/example/hello_world
    ```

3. `-s` オプションでシリアルポートを指定してビルドと書き込みを実行します。

    ```sh
    xmc run -p esp32s3_pio_arduino -s /dev/ttyACM0
    ```

    PlatformIO のデフォルトのシリアルポートを使用する場合は `-s` オプションを省略できます。

>[!NOTE]
> - 環境変数`XMC_DEFAULT_PLATFORM` にプラットフォームを指定しておくことで、`-p` オプションを省略できます。
> - 環境変数 `XMC_DEFAULT_DRIVE` にドライブレターを設定しておくことで `-d` オプションを省略できます。
> - 環境変数 `XMC_DEFAULT_SERIAL` にシリアルポートを設定しておくことで `-s` オプションを省略できます。

>[!NOTE]
> ESP32S3 では、VSCode の PlatformIO 拡張機能を使ってビルド・書き込みすることもできます。

## プロジェクト作成（Linux/WSL2）

```sh
source ${XMC_REPO_PATH}/setup.shrc
mkdir -p my_project
cd my_project
xmc init
cp -r ${XMC_REPO_PATH}/cpp/example/hello_world/src .
```

## 写真

![](./image/front.jpg)

![](./image/back.jpg)

## 動画

[![](./image/thumb-jumping-game.jpg)](https://x.com/shapoco/status/2041168789505794345)

[![](./image/thumb-3d-gfx.jpg)](https://x.com/shapoco/status/2038788120653824424)

## ライセンス

T.B.D.
