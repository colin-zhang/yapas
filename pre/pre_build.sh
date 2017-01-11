#!/bin/sh
set  -e

INSTALL_DIR=${PWD}/../opt/local
DPDK_RTE_TARGET=x86_64-native-linuxapp-gcc


if [ ! -d tmp ]; then
    mkdir tmp
fi

if [ ! -f package/dpdk-16.11.tar.xz ]; then
    wget http://fast.dpdk.org/rel/dpdk-16.11.tar.xz -O package/dpdk-16.11.tar.xz
fi

tar xf package/dpdk-16.11.tar.xz -C tmp

make -C tmp/dpdk-16.11/  -j install T=${DPDK_RTE_TARGET} DESTDIR=${INSTALL_DIR} V=s 
