include ../mk/rules.mk
include ../mk/init.mk
include ../mk/objective.mk

# import source list
include Makefile.src
HEADERS = $(HDRS) $(INCS)
SOURCES = $(addsuffix .c,$(ANGFILES) $(MAINFILES) $(ZFILES))
OBJECTS = $(addsuffix .o,$(ANGFILES) $(MAINFILES) $(ZFILES))


# These are for OS-specific ports; ignore 'em until we use autoconf everywhere.
# SUBDIRS = osx win

OBJECTIVE_BINS = angband

CFLAGS += \
	-I.

angband: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDADD) -o $@ 
	@printf "%10s     %-20s\n" LINK $@

localinstall: angband
	@if test "${DATA_PATH}" != "./lib/"; then \
		echo; \
		echo "WARNING: data path is ${DATA_PATH}"; \
		echo "If this isn't './lib/', your angband may not function correctly."; \
	fi
	cp angband ..
