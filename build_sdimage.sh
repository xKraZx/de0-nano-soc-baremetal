#!/bin/bash
set -e

if [ -z "$QUARTUS_ROOTDIR" ]; then
    echo "----- QUARTUS_ROOTDIR is not set:"
    echo "----- export QUARTUS_ROOTDIR=/path/to/your/quartus/dir/---/quartus/"
    exit 1
fi

export PATH="$QUARTUS_ROOTDIR/bin:$QUARTUS_ROOTDIR/sopc_builder/bin:$PATH"


if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

echo "Preparing files HW..."
make QUARTUS_ROOTDIR=$QUARTUS_ROOTDIR -C hw all
echo "Preparing files SW..."
make -C sw all
echo "Creating SD image..."
$SUDO python3 ./tools/make_sdimage_p3.py \
  -f \
  -P sw/temp/baremetal-with-spl.sfp,num=1,format=raw,size=10M,type=A2 \
  -s 20M \
  -n sdcard.img
