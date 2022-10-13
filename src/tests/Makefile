# Makefile for tests - builds unit-test binaries

CFLAGS+=-I../ -I. -g
LDFLAGS+=-lm

all : run

# Sorted alphabetically
SUITES = \
	artifact/suite.mk \
	cave/suite.mk \
	command/suite.mk \
	effects/suite.mk \
	game/suite.mk \
	message/suite.mk \
	monster/suite.mk \
	object/suite.mk \
	parse/suite.mk \
	player/suite.mk \
	trivial/suite.mk \
	z-dice/suite.mk \
	z-expression/suite.mk \
	z-file/suite.mk \
	z-quark/suite.mk \
	z-textblock/suite.mk \
	z-util/suite.mk \
	z-virt/suite.mk

include $(SUITES)

TESTOBJS  := $(TESTPROGS:%=%.o)
# Add an extension so suffix rules can be used.
TESTPROGS := $(TESTPROGS:%=%.exe)

TESTOBJS += test-utils.o unit-test.o

include Makefile.inc

build : $(TESTPROGS)

run : build
	@./run-tests

.SUFFIXES : .exe

.c.o :
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(TESTPROGS) : ../angband.o test-utils.o unit-test.o
.o.exe :
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $< \
		../angband.o test-utils.o unit-test.o \
		$(LDFLAGS) $(LDADD) $(LIBS)
	@echo "  CC $@"

clean :
	-$(RM) $(TESTOBJS) $(TESTPROGS)

.PHONY : all clean
.PRECIOUS : %.o
