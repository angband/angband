MKPATH=../mk/
include $(MKPATH)rules.mk
include $(MKPATH)objective.mk

# import source list
include Makefile.src
HEADERS = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(MAINFILES) $(ZFILES)
SOURCES = ${OBJECTS:.o=.c}

SUBDIRS = gtk

OBJECTIVE_BINS = angband

CFLAGS += -I.

angband: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDADD) -o $@ 
	@printf "%10s %-20s\n" LINK $@

clean-prehook:
	-rm autoconf.h

install-posthook:
	@if [ "x$(SETEGID)" != "x" ]; then \
		printf "%10s $(DESTDIR)$(BINDIR)/angband\n" CHOWN; \
		if [ "x$(DRY)" = "x" ]; then \
			chown root:${SETEGID} $(DESTDIR)$(BINDIR)/angband; \
			chmod g+s $(DESTDIR)$(BINDIR)/angband; \
		fi; \
	fi
