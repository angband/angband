MKPATH=../mk/
include $(MKPATH)buildsys.mk

# import source list
include Makefile.src
INCLUDE = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(ZFILES)
SRCS    = ${OBJECTS:.o=.c} ${MAINFILES:.o=.c}
SUBDIRS = gtk monster object player
PROG    = angband
VERSION := $(shell ../scripts/version.sh)
CFLAGS += -DBUILD_ID=${VERSION} -I. -std=c99 -Wdeclaration-after-statement -O0

# gcov intermediate data
GCOBJS = $(OBJECTS:.o=.gcno) $(OBJECTS:.o=.gcda)
GCOVS = $(OBJECTS:.o=.c.gcov)

CLEAN = angband.o angband.exe win/angband.res
DISTCLEAN = autoconf.h

export CFLAGS LDFLAGS

angband: angband.o $(MAINFILES)
	$(CC) -o angband angband.o $(MAINFILES) $(LDFLAGS) $(LDADD)
	@printf "%10s %-20s\n" LINK $@

angband.o: $(OBJECTS)
	$(LD) -r -o $@ $(OBJECTS)
	@printf "%10s %-20s\n" LINK $@

tests: angband.o
	$(MAKE) -C tests all

test-clean:
	$(MAKE) -C tests clean

splint:
	splint -f .splintrc ${OBJECTS:.o=.c} main.c main-gcu.c

clean-coverage:
	rm -f tests/ran-already
	rm -f $(GCOVS) $(GCOBJS)

coverage: CFLAGS+=-fprofile-arcs -ftest-coverage
coverage: LDFLAGS+=-lgcov
coverage: clean-coverage clean gen-covs

tests/ran-already : tests
	touch $@

gen-covs: tests/ran-already $(GCOVS)
	./gen-coverage $(GCOVS)

%.gcov: %
	(gcov -o $(dir $^) -p $^ >/dev/null)

.PHONY : tests coverage clean-coverage tests/ran-already
