# vim: noet sts=8 sw=8
CFLAGS ?= -Wall
STSAPP_BIN ?= /usr/local/bin

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

SYNC:	SYNC_main.o SYNC.o
	g++ $(CFLAGS) -o SYNC SYNC_main.o SYNC.o -lsts -lbdd
	strip SYNC

install: SYNC
	test -d $(STSAPP_BIN) || mkdir -p $(STSAPP_BIN)
	cp SYNC $(STSAPP_BIN)

###################

.PHONY : clean
clean:
	rm -f *.o SYNC
