# vim: noet sts=8 sw=8
CFLAGS ?= -Wall
STSAPP_BIN ?= /usr/local/bin

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

simsup:	simsup_main.o simsup.o 
	g++ $(CFLAGS) -o simsup simsup_main.o simsup.o -lsts -lbdd
	strip simsup

install: simsup
	test -d $(STSAPP_BIN) || mkdir -p $(STSAPP_BIN)
	cp simsup $(STSAPP_BIN)

###################

.PHONY : clean
clean:
	rm -f *.o simsup
