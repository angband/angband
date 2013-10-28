# Makefile for tests - builds unit-test binaries

CFLAGS+=-I../ -I. -g
LDFLAGS+=-lm
-include config

all : run

SUITES := $(shell find . -maxdepth 1 -mindepth 1 -type d)
SUITES := $(filter-out ./bin,$(SUITES))
include $(patsubst %,%/suite.mk,$(SUITES))

TESTOBJS  := $(patsubst %,%.o,$(TESTPROGS))
TESTPROGS := $(patsubst %,bin/%,$(TESTPROGS))

TESTOBJS += test-utils.o unit-test.o

build : $(TESTPROGS)

run : build
	@./run-tests

%.o : %.c
	@$(CC) $(CFLAGS) -c -o $@ $^

bin/% : %.o ../angband.o test-utils.o unit-test.o
	@mkdir -p $(shell echo "$$(dirname $@)")
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "  CC $@"

clean :
	$(RM) $(TESTPROGS) $(TESTOBJS)

.PHONY : all clean
.PRECIOUS : %.o
