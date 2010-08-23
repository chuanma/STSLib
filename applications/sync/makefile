CFLAGS = 

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

SYNC:	SYNC_main.o SYNC.o $(BASE_OBJS)
	g++ $(CFLAGS) -o SYNC SYNC_main.o SYNC.o $(BASE_OBJS) -lbdd -lsts
	strip SYNC
###################

.PHONY : clean
clean:
	rm *.o SYNC
