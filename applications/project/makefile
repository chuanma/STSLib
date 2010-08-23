CFLAGS = 

.SUFFIXES: .cc

.cc.o:
	g++ $(CFLAGS) -c $<

project:	project_main.o project.o
	g++ $(CFLAGS) -o project project_main.o project.o -lbdd -lsts
	strip project
###################

.PHONY : clean
clean:
	rm *.o project
