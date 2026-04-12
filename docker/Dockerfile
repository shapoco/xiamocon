FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# 1. Install required packages for RP2350
RUN apt-get update && \
    apt-get install -y \
        curl \
        build-essential \
        cmake \
        git \
        gcc-arm-none-eabi \
        python3 \
        python3-pip \
        python3-venv && \
    rm -rf /var/lib/apt/lists/*

# 2. Setup Python venv and install dependencies
COPY requirements.txt /opt/requirements.txt
RUN pip install \
        --break-system-packages \
        -r /opt/requirements.txt && \
    rm /opt/requirements.txt

# 3. Install Pico SDK
RUN mkdir -p /opt/.xmc && \
    cd /opt/.xmc && \
    git clone --depth 1 https://github.com/raspberrypi/pico-sdk.git && \
    cd pico-sdk && \
    git submodule update --init --recursive
ENV PICO_SDK_PATH=/opt/.xmc/pico-sdk

# 4. Install Pico Extras
RUN cd /opt/.xmc && \
    git clone --depth 1 https://github.com/raspberrypi/pico-extras.git
ENV PICO_EXTRAS_PATH=/opt/.xmc/pico-extras

# 5. Specify PlatformIO Core directory
ENV PLATFORMIO_CORE_DIR=/opt/.xmc/xiamocon/docker/.platformio
ENV PATH="${PATH}:/opt/.xmc/xiamocon/docker/.platformio/penv/bin"

# 6. Specify Xiamocon SDK path
ENV XMC_REPO_PATH=/opt/.xmc/xiamocon
ENV PATH="${PATH}:/opt/.xmc/xiamocon/bin"

# 6. Add entrypoint script
RUN apt-get update && \
    apt-get install -y \
    gosu && \
    rm -rf /var/lib/apt/lists/*
COPY docker/entrypoint.sh /usr/local/bin/.
RUN chmod +x /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
