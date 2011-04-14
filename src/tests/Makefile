# Makefile for tests - builds unit-test binaries

CFLAGS+=-I../ -I. -g
LDFLAGS+=-lm
-include config

all : run

SUITES := $(shell find . -maxdepth 1 -mindepth 1 -type d)
SUITES := $(filter-out ./bin,$(SUITES))
include $(patsubst %,%/suite.mk,$(SUITES))

TESTPROGS := $(patsubst %,bin/%,$(TESTPROGS))

build : $(TESTPROGS)

run : build
	@./run-tests

%.o : %.c
	@$(CC) $(CFLAGS) -c -o $@ $^

bin/% : %.o
	@mkdir -p $(shell echo "$$(dirname $@)")
	@$(CC) $(CFLAGS) -o $@ $^ ../angband.o $(LDFLAGS)
	@echo "  CC $@"

clean :
	$(RM) $(TESTPROGS)

.PHONY : all clean
