#!/bin/sh

HUGEPGSZ=`cat /proc/meminfo  | grep Hugepagesize | cut -d : -f 2 | tr -d ' '`

RTE_SDK=${PWD}/dpdk

remove_igb_uio_module()
{
	echo "Unloading any existing DPDK UIO module"
	/sbin/lsmod | grep -s igb_uio > /dev/null
	if [ $? -eq 0 ] ; then
		sudo /sbin/rmmod igb_uio
	fi
}

remove_kni_module()
{
	echo "Unloading any existing DPDK KNI module"
	/sbin/lsmod | grep -s rte_kni > /dev/null
	if [ $? -eq 0 ] ; then
		sudo /sbin/rmmod rte_kni
	fi
}

load_module()
{
	remove_igb_uio_module
	remove_kni_module

	echo "Loading DPDK UIO module"
	/sbin/lsmod | grep -s uio > /dev/null
	if [ $? -ne 0 ] ; then
		modinfo uio > /dev/null
		if [ $? -eq 0 ]; then
			echo "Loading uio module"
			/sbin/modprobe uio
		fi
	fi

	echo "Loading DPDK UIO module"
	insmod ${RTE_SDK}/kmod/igb_uio.ko
	if [ $? -ne 0 ] ; then
		echo "## ERROR: Could not load kmod/igb_uio.ko."
		quit
	fi

	echo "Loading DPDK KNI module"
	insmod ${RTE_SDK}/kmod/rte_kni.ko
	if [ $? -ne 0 ] ; then
		echo "## ERROR: Could not load kmod/rte_kni.ko."
		quit
	fi
}

remove_mnt_huge()
{
	echo "Unmounting /mnt/huge and removing directory"
	grep -s '/mnt/huge' /proc/mounts > /dev/null
	if [ $? -eq 0 ] ; then
		umount /mnt/huge
	fi

	if [ -d /mnt/huge ] ; then
		rm -R /mnt/huge
	fi
}

clear_huge_pages()
{
	echo > .echo_tmp
	for d in /sys/devices/system/node/node? ; do
		echo "echo 0 > $d/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages" >> .echo_tmp
	done
	echo "Removing currently reserved hugepages"
	sh .echo_tmp
	rm -f .echo_tmp

	remove_mnt_huge
}

create_mnt_huge()
{
	echo "Creating /mnt/huge and mounting as hugetlbfs"
	mkdir -p /mnt/huge

	grep -s '/mnt/huge' /proc/mounts > /dev/null
	if [ $? -ne 0 ] ; then
		mount -t hugetlbfs nodev /mnt/huge
	fi
}

set_non_numa_pages()
{
	clear_huge_pages

	echo ""
	echo "echo $@ > /sys/kernel/mm/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages" > .echo_tmp

	echo "Reserving hugepages : $@ * 2MB pages"
	sh .echo_tmp
	rm -f .echo_tmp

	create_mnt_huge
}

#
# Creates hugepages on specific NUMA nodes.
#
set_numa_pages()
{
	clear_huge_pages

	echo ""
	echo > .echo_tmp
	for d in /sys/devices/system/node/node? ; do
		node=$(basename $d)
		echo "echo $@ > $d/hugepages/hugepages-${HUGEPGSZ}/nr_hugepages" >> .echo_tmp
	done
	echo "Reserving hugepages : $@ * 2MB * 2 pages"
	sh .echo_tmp
	rm -f .echo_tmp

	create_mnt_huge
}

grep_meminfo()
{
	grep -i huge /proc/meminfo
}

show_nics()
{
	if [ -d /sys/module/vfio_pci -o -d /sys/module/igb_uio ]; then
		${RTE_SDK}/tools/dpdk-devbind.py --status
	else
		echo "# Please load the 'igb_uio' or 'vfio-pci' kernel module before "
		echo "# querying or adjusting NIC device bindings"
	fi
}

bind_nics_to_igb_uio()
{
	echo ""
	if [ -d /sys/module/igb_uio ]; then
		sudo ${RTE_SDK}/tools/dpdk-devbind.py -b igb_uio $@ && echo "bind nic <$@> to igb_uio OK"
	else
		echo "# Please load the 'igb_uio' kernel module before querying or "
		echo "# adjusting NIC device bindings"
	fi
}

load_module

#set_non_numa_pages 1024
set_numa_pages 2048
grep_meminfo

bind_nics_to_igb_uio 04:00.1
show_nics
