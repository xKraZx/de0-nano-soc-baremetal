
FROM ubuntu:18.04 AS quartus

ENV DEBIAN_FRONTEND=noninteractive
ARG QUARTUS_URL="https://download.altera.com/akdlm/software/acdsinst/18.1std/625/ib_tar/Quartus-lite-18.1.0.625-linux.tar"
ARG QUARTUS_CHECKSUM="eaf82392603b92dae632cc0f356b08aa"

RUN apt-get update && apt-get install -y \
    aria2 \
    ca-certificates \
    tar \
    gzip \
    perl \
    && rm -rf /var/lib/apt/lists/*

RUN aria2c \
    -x 8 \
    -s 8 \
    -k 16M \
    --file-allocation=none \
    --summary-interval=1 \
    --continue=true \
    --user-agent="Mozilla/5.0 (X11; Linux x86_64; rv:153.0) Gecko/20100101 Firefox/153.0" \
    --header="Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" \
    --header="Accept-Language: en-US,en;q=0.9" \
    --header="Accept-Encoding: gzip, deflate, br, zstd" \
    --header="Referer: https://www.altera.com/" \
    --header="Upgrade-Insecure-Requests: 1" \
    --checksum="md5=${QUARTUS_CHECKSUM}" \
    --dir=/tmp \
    -o Quartus-lite-18.1.0.625-linux.tar \
    "${QUARTUS_URL}" \
    && mkdir -p /tmp/quartus \
    && tar -xf /tmp/Quartus-lite-18.1.0.625-linux.tar \
        -C /tmp/quartus \
    && /tmp/quartus/setup.sh \
        --mode unattended \
        --accept_eula 1 \
        --installdir /opt/intelFPGA_lite \
        --disable-components \
        arria_lite,cyclone,cyclone10lp,max,max10,modelsim_ase,modelsim_ae \
    && rm -rf \
        /tmp/quartus \
        /tmp/Quartus-lite-18.1.0.625-linux.tar


FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

COPY --from=quartus /opt/intelFPGA_lite /opt/intelFPGA_lite

RUN apt-get update && apt-get install -y \
    build-essential \
    device-tree-compiler \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    bc \
    bison \
    flex \
    libssl-dev \
    python3 \
    python3-pip \
    parted \
    udev \
    curl \
    locales \
    libudev1 \
    libx11-6 \
    libxext6 \
    libxrender1 \
    libxft2 \
    libxtst6 \
    libxi6 \
    libsm6 \
    libglib2.0-0 \
    libfontconfig1 \
    libfreetype6 \
    ca-certificates \
    && sed -i 's/^# *en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && locale-gen en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8


RUN curl -4 -fL \
    -o /tmp/libpng12.deb \
    "https://archive.ubuntu.com/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1.1_amd64.deb" \
    && mkdir /tmp/libpng12 \
    && dpkg-deb -x /tmp/libpng12.deb /tmp/libpng12 \
    && cp /tmp/libpng12/lib/x86_64-linux-gnu/libpng12.so.0* \
        /usr/lib/x86_64-linux-gnu/ \
    && ldconfig \
    && rm -rf /tmp/libpng12 /tmp/libpng12.deb


ENV PATH="/usr/local/bin:/opt/intelFPGA_lite/quartus/bin:/usr/local/sbin:/usr/sbin:/sbin:/usr/bin:/bin"
ENV QUARTUS_ROOTDIR="/opt/intelFPGA_lite/quartus"

ENV LD_PRELOAD=/lib/x86_64-linux-gnu/libudev.so.1

WORKDIR /project

RUN printf '%s\n' \
    '#!/bin/sh' \
    'export LD_PRELOAD=/lib/x86_64-linux-gnu/libudev.so.1' \
    'export QT_X11_NO_MITSHM=1' \
    'exec /opt/intelFPGA_lite/quartus/bin/quartus --64bit "$@"' \
    > /usr/local/bin/quartus \
    && chmod +x /usr/local/bin/quartus

CMD ["bash"]
