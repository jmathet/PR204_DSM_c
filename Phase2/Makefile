ALL: default 

CC           = gcc
CLINKER      = $(CC)
OPTFLAGS     = -O0


SHELL = /bin/sh

CFLAGS  =   -DREENTRANT
CCFLAGS = $(CFLAGS)
LIBS =  -lpthread

EXECS = exemple

default: $(EXECS)


exemple: exemple.o dsm.o 
	$(CLINKER) $(OPTFLAGS) -o exemple exemple.o dsm.o  $(LIBS)

dsmexec: dsmexec.o common.o
	$(CLINKER) $(OPTFLAGS) -o dsmexec dsmexec.o  common.o $(LIBS)
	mv dsmexec ./bin
	
dsmwrap: dsmwrap.o common.o
	$(CLINKER) $(OPTFLAGS) -o dsmwrap dsmwrap.o  common.o $(LIBS)
	mv dsmwrap ./bin

clean:
	@-/bin/rm -f *.o *~ PI* $(EXECS) *.out core  
.c:
	$(CC) $(CFLAGS) -o $* $< $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $<
.o:
	${CLINKER} $(OPTFLAGS) -o $* $*.o $(LIBS)
