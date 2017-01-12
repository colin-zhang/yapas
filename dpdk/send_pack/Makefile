CC=gcc
RTE_SDK=/opt/yapas/pre/tmp/dpdk-16.11
LIBS=-lrte_eal -lpthread -ldl -lrte_ethdev -lrte_mempool

#CC_OPTS = -Wall -Werror -lev -ggdb3 -I./include
CC_OPTS = -I$(RTE_SDK)/x86_64-native-linuxapp-gcc/include -m64 -march=native -pthread
LD_OPTS = -L$(RTE_SDK)/x86_64-native-linuxapp-gcc/lib $(LIBS)

#-mssse3

all: send_pack


send_pack: send_pack.o
	$(CC) $(LD_OPTS) -o $@ $<

send_pack.o: send_pack.c
	$(CC) $(CC_OPTS) -o $@ -c $<


clean:
	rm -rf *.o send_pack
