include ../mk/rules.mk
include ../mk/init.mk
include ../mk/objective.mk

# import source list
include Makefile.src
HEADERS = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(MAINFILES) $(ZFILES)
SOURCES = ${OBJECTS:.o=.c}

# These are for OS-specific ports; ignore 'em until we use autoconf everywhere.
# SUBDIRS = osx win

OBJECTIVE_BINS = angband

CFLAGS += -I.

angband: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDADD) -o $@ 
	@printf "%10s     %-20s\n" LINK $@

install-posthook:
	@if [ "x$(SETEGID)" != "x" ]; then \
		printf "%10s     $(DESTDIR)$(BINDIR)/angband\n" CHOWN; \
		if [ "x$(DRY)" = "x" ]; then \
			chown root:${SETEGID} $(DESTDIR)$(BINDIR)/angband; \
		fi; \
	fi
