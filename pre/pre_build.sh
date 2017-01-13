#!/bin/sh
set  -e

INSTALL_DIR=${PWD}/../opt/local

# T=i686-native-linuxapp-gcc for 32 bits
DPDK_RTE_TARGET=x86_64-native-linuxapp-gcc

#DPDK_VERSION=dpdk-16.07.2
#DPDK_VERSION=dpdk-16.11

if [ ! -d tmp ]; then
    mkdir tmp
fi


if [ ! -f package/${DPDK_VERSION}.tar.xz ]; then
    wget http://fast.dpdk.org/rel/${DPDK_VERSION}.tar.xz -O package/${DPDK_VERSION}.tar.xz
fi

tar xf package/${DPDK_VERSION}.tar.xz -C tmp

make -C tmp/${DPDK_VERSION} -j install T=${DPDK_RTE_TARGET} DESTDIR=${INSTALL_DIR} V=s 
