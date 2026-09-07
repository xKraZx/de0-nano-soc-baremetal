#!/bin/bash
set -e

if [ -z "$QUARTUS_ROOTDIR" ]; then
    echo "----- QUARTUS_ROOTDIR is not set:"
    echo "----- export QUARTUS_ROOTDIR=/path/to/your/quartus/dir/---/quartus/"
    exit 1
fi

if [ -z "$PROJECT_ROOT" ]; then
    echo "----- PROJECT_ROOT is not set:"
    echo "----- export PROJECT_ROOT=/path/to/this/project/---/"
    exit 1
fi

export PATH="$QUARTUS_ROOTDIR/bin:$QUARTUS_ROOTDIR/sopc_builder/bin:$PATH"


if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
else
    SUDO="sudo"
fi

echo "Preparing files HW..."
make QUARTUS_ROOTDIR=$QUARTUS_ROOTDIR -C $PROJECT_ROOT/hw all
echo "Preparing files SW..."
make -C $PROJECT_ROOT/sw all
echo "Creating SD image..."
$SUDO python3 $PROJECT_ROOT/tools/make_sdimage_p3.py \
  -f \
  -P sw/temp/baremetal-with-spl.sfp,num=1,format=raw,size=10M,type=A2 \
  -s 20M \
  -n sdcard.img
