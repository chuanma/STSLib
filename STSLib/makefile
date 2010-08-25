INCLUDE ?= /usr/local/include
LIB ?= /usr/local/lib

############### Dont change anything below
OBJS = 	automaton.o \
	bdd_interface.o \
	bdd_synthesis.o \
	event.o \
	fstring.o \
	holon.o \
	myiostream.o \
	state_tree.o \
	state.o  \
	sts.o \
	tct_engine.o \
	transition.o \
	bufferGenerator.o \
	relation.o 


.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

all: install

libsts.a: $(OBJS)
	ar r libsts.a $(OBJS)
	ranlib libsts.a

install: libsts.a
	cp *.h $(INCLUDE)
	cp libsts.a $(LIB)

###################

.PHONY: clean tgz
clean:
	rm -rf *.o libsts.a
tgz:
	tar -vczf STSLib.tgz *.cc *.h makefile README 
	chmod 400 STSLib.tgz
