MKPATH=../mk/
include $(MKPATH)buildsys.mk

# import source list
include Makefile.src
INCLUDE = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(ZFILES)
SRCS    = ${OBJECTS:.o=.c} ${MAINFILES:.o=.c}
SUBDIRS = gtk monster object player
PROG    = angband
CFLAGS += -I. -std=c99

angband: angband.o $(MAINFILES)
	$(CC) -o angband angband.o $(MAINFILES) $(LDFLAGS) $(LDADD)
	@printf "%10s %-20s\n" LINK $@

angband.o: $(OBJECTS)
	$(LD) -r -o $@ $(OBJECTS)
	@printf "%10s %-20s\n" LINK $@

clean-extra:
	-rm autoconf.h

tests:
	$(MAKE) -C tests all

.PHONY : tests
