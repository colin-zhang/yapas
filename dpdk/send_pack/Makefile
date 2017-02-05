CC=g++
LIBS= -lpthread -ldl -lm  -Wl,--whole-archive -ldpdk -Wl,--no-whole-archive

#CCOPTS = -Wall -Werror -lev -ggdb3 -I./include
#-mssse3 -mssse4
CC_OPTS = -I$(DVLP_)/include/dpdk -m64 -march=native -pthread --std=c++11
LD_OPTS = -L$(DVLP_)/lib $(LIBS)

all: send_pack

send_pack: send_pack.o
	$(CC) $(LD_OPTS) -o $@ $<

send_pack.o: send_pack.cc
	$(CC) $< $(CC_OPTS) -o $@ -c

clean:
	rm -rf *.o send_pack
