ALL: default

CC           = gcc
CLINKER      = $(CC)
OPTFLAGS     = -O0


SHELL = /bin/sh

CFLAGS  =   -DREENTRANT -std=c99
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

<<<<<<< HEAD
test: test.o common.o
=======
test: test.o dsm.o
>>>>>>> fab7b7fc86bfbc1b85bce381638e05b48ae603dd
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
