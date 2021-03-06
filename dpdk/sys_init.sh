#!/bin/sh
set -e 

#yum list gcc-c++
#gcc、gcc-c++、kernel-devel
#yum install kernel-devel-$(uname -r)
#
#yum install vargrind
#apt install oprofile
#

#echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

#On a NUMA machine, pages should be allocated explicitly on separate nodes:
echo 2048 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 2048 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
if [[ ! -d /mnt/huge ]]; then
    mkdir /mnt/huge
fi
mount -t hugetlbfs nodev /mnt/huge


#sudo modprobe uio
#sudo insmod kmod/igb_uio.ko
sudo modprobe uio
sudo rmmod igb_uio.ko
sudo insmod ${DVLP_}/lib/modules/$(uname -r)/extra/dpdk/igb_uio.ko

dpdk-devbind --bind=igb_uio $1

echo 1
