# vim: noet sts=8 sw=8
CFLAGS ?= -Wall
STSAPP_BIN ?= /usr/local/bin

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

nbc:	nbc_main.o
	g++ $(CFLAGS) -o nbc nbc_main.o -lsts -lbdd

install: nbc
	cp nbc $(STSAPP_BIN)

###################

.PHONY : clean
clean:
	rm -f *.o nbc
