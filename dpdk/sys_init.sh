#!/bin/sh
set -e 

#yum list gcc-c++
#gcc、gcc-c++、kernel-devel
#yum install kernel-devel-$(uname -r)
#

echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

#On a NUMA machine, pages should be allocated explicitly on separate nodes:
#echo 1024 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
#echo 1024 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages

mkdir /mnt/huge
mount -t hugetlbfs nodev /mnt/huge


#sudo modprobe uio
#sudo insmod kmod/igb_uio.ko