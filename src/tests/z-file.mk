# Simple UNIX-only makefile for the z-file tests
EXE = z-file

CC = gcc
CFLAGS = -Wall -O2 -Wno-unused-parameter -fprofile-arcs -ftest-coverage
LDFLAGS = -fprofile-arcs -ftest-coverage

OBJS = z-file.o ../z-file.o ../z-virt.o ../z-util.o ../z-form.o 

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

clean:
	-rm -f *.o $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

z-file.o: ../z-file.o ../z-form.o
../z-file.o: ../z-virt.o ../z-util.o ../z-form.o
