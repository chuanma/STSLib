# vim: noet sts=8 sw=8
CFLAGS ?= -Wall
STSAPP_BIN ?= /usr/local/bin

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

SYNC:	SYNC_main.o SYNC.o
	g++ $(CFLAGS) -o SYNC SYNC_main.o SYNC.o -lbdd -lsts
	strip SYNC

install: SYNC
	cp SYNC $(STSAPP_BIN)

###################

.PHONY : clean
clean:
	rm -f *.o SYNC
