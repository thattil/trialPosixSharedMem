CCX			=	g++
CFLAGS		=  -Wall -g
LDLIBS		= 	-lm  -lstdc++ -pthread -lrt 
INCLUDES	= 	-I.


PROGS		= 	trial
DESTDIR		= 	bin
INSTMODE	= 	0777

SRC			=	trial.cpp SharedMemory.cpp MultiDet.cpp SingleDet.cpp
OBJS		= 	$(SRC:.cpp=.o)


all: clean $(PROGS)  

boot: $(OBJS) 

$(PROGS): 
	mkdir -p $(DESTDIR) 
	$(CCX)  -o $@   $(SRC) $(INCLUDES)  $(CFLAGS) $(LDLIBS) 
	mv $(PROGS) $(DESTDIR)
	

clean:
	rm -rf  *.o $(DESTDIR)/$(PROGS)
	

	
