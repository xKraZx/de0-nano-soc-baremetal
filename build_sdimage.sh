#!/bin/bash
set -e

export QUARTUS_ROOTDIR=/root/intelFPGA_lite/18.1/quartus/

echo "Preparing files HW..."
make QUARTUS_ROOTDIR=$QUARTUS_ROOTDIR -C hw all
echo "Preparing files SW..."
make -C sw all
echo "Creating SD image..."
sudo python3 ./tools/make_sdimage_p3.py \
  -f \
  -P sw/temp/baremetal-with-spl.sfp,num=1,format=raw,size=10M,type=A2 \
  -s 20M \
  -n sdcard.img