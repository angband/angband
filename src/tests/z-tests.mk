# Simple UNIX-only makefile for the z-tests tests.
EXE = z-tests

CC = gcc
CFLAGS = -Wall -O2 -Wno-unused-parameter -fprofile-arcs -ftest-coverage
LDFLAGS = -fprofile-arcs -ftest-coverage

OBJS = z-tests.o ../z-tests.o

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

clean:
	-rm -f *.o $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

z-tests.o: ../z-tests.o

