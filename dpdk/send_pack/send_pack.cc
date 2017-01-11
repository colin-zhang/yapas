#include <cstdio>
#include "dpdk/rte_eal.h"
#include "dpdk/rte_lcore.h"


int main(int argc, char *argv[])
{
    int ret;
    ret = rte_eal_init(argc, argv);
    if (ret < 0) printf("Cannot init EAL\n");
    argc -= ret;
    argv += ret;

    /* Check if this application can use 1 core*/
    ret = rte_lcore_count ();
    if (ret != 1) printf("This application needs exactly 1 cores. \n");
    return 0;
}