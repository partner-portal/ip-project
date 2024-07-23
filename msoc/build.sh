#!/bin/bash

set -e

CFLAGS=

if [ -z ${TOOLCHAIN_DIR} ]; then
    TOOLCHAIN_DIR=~/.vintgo/.conan/data/yocto_sdk/1.0.0.0/vcore_pcu_hpc/release/package/676e5395efa6edf0b27bb6823c9684bee7fb023e/bin
fi


echo "------------------------Building provencore for MSOC------------------------" 

cd ./provencore/
export CROSS_COMPILE64=""$TOOLCHAIN_DIR"/aarch64-none-linux-gnu-"


make b1_msoc_config 
make config
make programs 
make  all 
