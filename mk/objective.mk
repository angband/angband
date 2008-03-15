.SILENT:

OBJECTIVE_DIRECTORIES = 
OBJECTIVE_LIBS = 
OBJECTIVE_LIBS_NOINST = 
OBJECTIVE_BINS = 
OBJECTIVE_DATA = 
SUBDIRS = 
HEADERS = 
V = 0
VERBOSE ?= $(V)
VERBOSITY = 0
SHOW_CFLAGS ?= $(VERBOSE)

LIBDIR = $(libdir)
BINDIR = $(bindir)
INCLUDEDIR = $(pkgincludedir)
CFLAGS += -DHAVE_CONFIG_H -I/usr/pkg/include -I/usr/pkg/xorg/include
CXXFLAGS += -DHAVE_CONFIG_H -I/usr/pkg/include -I/usr/pkg/xorg/include

default: build

install: build
	$(MAKE) install-prehook
	@for i in $(BINDIR) $(LIBDIR) $(INCLUDEDIR); do \
		if [ ! -d $(DESTDIR)$$i ]; then \
			$(INSTALL) -d -m 755 $(DESTDIR)$$i; \
		fi; \
	done;
	@if [ "x$(SUBDIRS)" != "x" ]; then \
		for i in $(SUBDIRS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[installing subobjective: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) install || exit; cd ..; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_DIRECTORIES)" != "x" ]; then \
		for i in $(OBJECTIVE_DIRECTORIES); do \
			printf "%10s     %-20s\n" MKDIR $$i; \
			$(INSTALL) -d -m 755 $(DESTDIR)$$i; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_BINS)" != "x" ]; then \
		for i in $(OBJECTIVE_BINS); do \
			printf "%10s     %-20s (-> $(DESTDIR)$(BINDIR))\n" INSTALL $$i; \
			if [ "x$(DRY)" = "x" ]; then \
				$(INSTALL) $(INSTALL_OVERRIDE) $$i $(DESTDIR)$(BINDIR)/$$i; \
			fi; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_DATA)" != "x" ]; then \
	 if [ "x$(NOINSTALL)" = "x" ]; then \
		for i in $(OBJECTIVE_DATA); do \
			source=`echo $$i | cut -d ":" -f1`; \
			destination=`echo $$i | cut -d ":" -f2`; \
			printf "%10s     %-20s (-> $(DESTDIR)$$destination)\n" INSTALL $$source; \
			if [ "x$(DRY)" = "x" ]; then \
				if [ ! -d $(DESTDIR)$$destination ]; then \
					$(INSTALL) -d -m 755 $(DESTDIR)$$destination; \
				fi; \
				$(INSTALL_DATA) $(INSTALL_OVERRIDE) $$source $(DESTDIR)$$destination; \
			fi; \
		done; \
	fi; \
	fi
	$(MAKE) install-posthook
	@if [ $(VERBOSITY) -gt 0 ]; then \
		echo "[all objectives installed]"; \
	fi

clean:
	$(MAKE) clean-prehook
	@if [ "x$(OVERLAYS)" != "x" ]; then \
		for i in `find $(OVERLAYS) -type d -maxdepth 1 -mindepth 1`; do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[cleaning overlay: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) clean || exit; cd ..; \
		done; \
	fi
	@if [ "x$(SUBDIRS)" != "x" ]; then \
		for i in $(SUBDIRS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[cleaning subobjective: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) clean || exit; cd ..; \
		done; \
	fi
	$(MAKE) clean-posthook
	rm -f *.o *.lo *.so *.a *.sl .depend-done .depend
	touch .depend
	@if [ "x$(OBJECTIVE_BINS)" != "x" ]; then \
		for i in $(OBJECTIVE_BINS); do \
			rm -f $$i; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_LIBS)" != "x" ]; then \
		for i in $(OBJECTIVE_LIBS); do \
			rm -f $$i; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_LIBS_NOINST)" != "x" ]; then \
		for i in $(OBJECTIVE_LIBS_NOINST); do \
			rm -f $$i; \
		done; \
	fi
	@if [ $(VERBOSITY) -gt 0 ]; then \
		echo "[all objectives cleaned]"; \
	fi

distclean: clean
	@if [ "x$(OVERLAYS)" != "x" ]; then \
		for i in `find $(OVERLAYS) -type d -maxdepth 1 -mindepth 1`; do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[distcleaning overlay: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) distclean || exit; cd ..; \
		done; \
	fi
	@if [ "x$(SUBDIRS)" != "x" ]; then \
		for i in $(SUBDIRS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[distcleaning subobjective: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) distclean || exit; cd ..; \
		done; \
	fi
	@if [ -f mk/rules.mk ]; then \
		rm -f mk/rules.mk; \
	fi

build: depend
	$(MAKE) build-prehook
	@if [ "x$(OVERLAYS)" != "x" ]; then \
		for i in `find $(OVERLAYS) -type d -maxdepth 1 -mindepth 1`; do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[building overlay: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) || exit; cd ..; \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[finished overlay: $$i]"; \
			fi; \
		done; \
	fi
	@if [ "x$(SUBDIRS)" != "x" ]; then \
		for i in $(SUBDIRS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[building subobjective: $$i]"; \
			fi; \
			cd $$i; OVERLAYS="" $(MAKE) || exit; cd ..; \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[finished subobjective: $$i]"; \
			fi; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_LIBS)" != "x" ]; then \
		for i in $(OBJECTIVE_LIBS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[building library objective: $$i]"; \
			fi; \
			$(MAKE) $$i || exit; \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[finished library objective: $$i]"; \
			fi; \
		done; \
	fi
	@if [ "x$(OBJECTIVE_LIBS_NOINST)" != "x" ]; then \
		for i in $(OBJECTIVE_LIBS_NOINST); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[building library dependency: $$i]"; \
			fi; \
			$(MAKE) $$i || exit; \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[finished library dependency: $$i]"; \
			fi; \
		done; \
	fi
	@if test "x$(OBJECTIVE_BINS)" != "x"; then \
		for i in $(OBJECTIVE_BINS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[building binary objective: $$i]"; \
			fi; \
			$(MAKE) $$i || exit; \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[finished binary objective: $$i]"; \
			fi; \
		done; \
	fi
	$(MAKE) build-posthook
	@if [ $(VERBOSITY) -gt 0 ]; then \
		echo "[all objectives built]"; \
	fi

.c.o:
	@if [ $(SHOW_CFLAGS) -eq 1 ]; then	\
		printf "%10s %-20s (%s)\n" CC $< "${CFLAGS}";	\
	else \
		printf "%10s %-20s\n" CC $<;	\
	fi;
	$(CC) $(CFLAGS) -c $< -o $@

.cc.o .cxx.o:
	@if [ $(SHOW_CFLAGS) -eq 1 ]; then	\
		printf "%10s %-20s (%s)\n" CXX $< "${CXXFLAGS}";	\
	else \
		printf "%10s %-20s\n" CXX $<;	\
	fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECTIVE_LIBS): $(OBJECTS)
	if [ "x$(OBJECTS)" != "x" ]; then \
		$(MAKE) $(OBJECTS) || exit;		\
		printf "%10s     %-20s\n" LINK $@; \
		(if [ "x$(SHARED_SUFFIX)" = "x.so" ]; then \
			(if [ "x$(OBJECTIVE_SONAME_SUFFIX)" != "x" ]; then \
				$(CC) $(PICLDFLAGS) -o $@ -Wl,-h$@.$(OBJECTIVE_SONAME_SUFFIX) $(OBJECTS) $(LDFLAGS) $(LIBADD); \
			else \
				$(CC) $(PICLDFLAGS) -o $@ -Wl,-h$@ $(OBJECTS) $(LDFLAGS) $(LIBADD); \
			fi;) \
		 else \
			$(CC) $(PICLDFLAGS) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBADD); \
		 fi;) \
	fi

%.a: $(OBJECTS)
	if [ "x$(OBJECTS)" != "x" ]; then \
		$(MAKE) $(OBJECTS) || exit;		\
		printf "%10s     %-20s\n" LINK $@; \
		$(AR) cr $@ $(OBJECTS); \
	fi

$(OBJECTIVE_BINS): $(OBJECTS)
	if [ "x$(OBJECTS)" != "x" ]; then \
		$(MAKE) $(OBJECTS) || exit;		\
		printf "%10s     %-20s\n" LINK $@; \
		$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBADD); \
	fi

clean-prehook:
clean-posthook:
build-prehook:
build-posthook:
install-prehook:
install-posthook:

mk/rules.mk:
	@if [ -f "configure" ]; then \
		echo "[building rules.mk for posix target, run configure manually if you do not want this]"; \
		sh configure $(CONFIG_OPTS); \
		echo "[complete]"; \
	fi

.PHONY: .depend depend clean distclean
.depend:

# default depend rule. if something else is needed -- override depend target
depend:
	@if [ "x$(SUBDIRS)" != "x" ]; then \
		for i in $(SUBDIRS); do \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[building depend file for subobjective: $$i]"; \
			fi; \
			cd $$i; touch .depend; $(MAKE) -f Makefile depend || exit; cd ..; \
			if [ $(VERBOSITY) -gt 0 ]; then \
				echo "[finished subobjective: $$i]"; \
			fi; \
		done; \
	fi
	if [ ! -f .depend-done ]; then \
		for i in ${SOURCES}; do \
			echo "[generating dependencies for objective: $$i]"; \
			${CC} -MM ${PICFLAGS} ${CPPFLAGS} ${CFLAGS} $$i >> .depend; \
		done; \
		touch .depend-done; \
	fi;

# compatibility with automake follows
am--refresh:

-include .depend
