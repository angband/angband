MKPATH=../mk/
include $(MKPATH)buildsys.mk

# import source list
include Makefile.src
INCLUDE = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(ZFILES)
SRCS    = ${OBJECTS:.o=.c}
SUBDIRS = gtk monster object player
PROG    = angband
CFLAGS += -I.

angband: angband.o $(MAINFILES)
	$(CC) -o angband angband.o $(MAINFILES) $(LDFLAGS) $(LDADD)
	@printf "%10s %-20s\n" LINK $@

angband.o: $(OBJECTS)
	$(LD) -r -o $@ $(OBJECTS)
	@printf "%10s %-20s\n" LINK $@

clean-extra:
	-rm autoconf.h

tests:
	echo "make..."
	$(MAKE) -C tests all
	echo "made"

.PHONY : tests
