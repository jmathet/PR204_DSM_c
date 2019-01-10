ALL: default

CC           = gcc
CLINKER      = $(CC)
OPTFLAGS     = -O0


SHELL = /bin/sh

CFLAGS  =   -DREENTRANT
CCFLAGS = $(CFLAGS)
LIBS =  -lpthread

EXECS = exemple dsm.o dsmexec dsmwrap test

default: $(EXECS)


exemple: exemple.o dsm.o
	$(CLINKER) $(OPTFLAGS) -o exemple exemple.o dsm.o  $(LIBS)

dsmexec: dsmexec.o dsm.o
	$(CLINKER) $(OPTFLAGS) -o dsmexec dsmexec.o  dsm.o $(LIBS)
	mv dsmexec ./bin

dsmwrap: dsmwrap.o dsm.o
	$(CLINKER) $(OPTFLAGS) -o dsmwrap dsmwrap.o  dsm.o $(LIBS)
	mv dsmwrap ./bin

test: test.o dsm.o
		$(CLINKER) $(OPTFLAGS) -o test test.o  dsm.o $(LIBS)
		mv test ./bin

clean:
	@-/bin/rm -f *.o *~ PI* $(EXECS) *.out core
.c:
	$(CC) $(CFLAGS) -o $* $< $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $<
.o:
	${CLINKER} $(OPTFLAGS) -o $* $*.o $(LIBS)
