CFLAGS = -Wall

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

nbc:	nbc_main.o
	g++ $(CFLAGS) -o nbc nbc_main.o -lbdd -lsts

###################

.PHONY : clean
clean:
	rm *.o nbc
