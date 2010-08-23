CFLAGS = 

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

simsup:	simsup_main.o simsup.o 
	g++ $(CFLAGS) -o simsup simsup_main.o simsup.o -lbdd -lsts
	strip simsup
###################

.PHONY : clean
clean:
	rm *.o simsup
