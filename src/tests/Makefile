# Makefile for tests - builds unit-test binaries

CFLAGS+=-I../ -I. -g
LDFLAGS+=-lm
-include config

all : build run

SUITES := $(shell find . -maxdepth 1 -mindepth 1 -type d)
include $(patsubst %,%/suite.mk,$(SUITES))

build : $(TESTPROGS)

run :
	./run-tests

% : %.o
	$(CC) -o $@ $^ ../angband.o $(LDFLAGS)

clean :
	$(RM) $(TESTPROGS)

.PHONY : all clean
