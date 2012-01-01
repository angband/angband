MKPATH=../mk/
include $(MKPATH)buildsys.mk

# import source list
include Makefile.src
INCLUDE = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(ZFILES)
SRCS    = ${OBJECTS:.o=.c} ${MAINFILES:.o=.c}
SUBDIRS = gtk monster object player
PROG    = angband$(EXEEXT)
VERSION := $(shell ../scripts/version.sh)
CFLAGS += -DBUILD_ID=${VERSION} -I. -std=c99 -Wdeclaration-after-statement -O0

# gcov intermediate data
GCOBJS = $(OBJECTS:.o=.gcno) $(OBJECTS:.o=.gcda)
GCOVS = $(OBJECTS:.o=.c.gcov)

CLEAN = angband.o $(PROG) win/angband.res
DISTCLEAN = autoconf.h

export CFLAGS LDFLAGS LIBS

angband$(EXEEXT): angband.o $(MAINFILES)
	$(CC) -o angband$(EXEEXT) angband.o $(MAINFILES) $(LDFLAGS) $(LDADD) $(LIBS)
	@printf "%10s %-20s\n" LINK $@

win/angband.res: win/angband.rc
	$(RC) $< -O coff -o $@

angband.o: $(OBJECTS)
	$(LD) -nostdlib -r -o $@ $(OBJECTS)
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
