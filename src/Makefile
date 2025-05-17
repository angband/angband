MKPATH=../mk/
include $(MKPATH)buildsys.mk

# import source list
include Makefile.src
INCLUDE = $(HDRS) $(INCS)
OBJECTS = $(ANGFILES) $(ZFILES)
SRCS    = ${OBJECTS:.o=.c} ${MAINFILES:.o=.c}
PROG    = $(PROGNAME)$(PROG_SUFFIX)
# Will dynamically generate version.h with the build number.
CFLAGS += -DHAVE_VERSION_H

CFLAGS += -I. -fPIC -std=c99 -O0
# Replace above line with the two below and then look at gmon.out
# to do performance monitoring
# CFLAGS += -g -pg -I. -fPIC -std=c99 -O0
# LDFLAGS += -g -pg

# gcov intermediate data
GCOBJS = $(OBJECTS:.o=.gcno) $(OBJECTS:.o=.gcda)
GCOVS = $(OBJECTS:.o=.c.gcov)

# buildsys's default clean will take care of any .o from SRCS; $(PROGNAME).o
# is a relic and only there to clean up an intermediate from previous versions
# of this Makefile.
CLEAN = $(PROGNAME).a $(PROGNAME).o $(ALLMAINFILES) ${ALLMAINFILES:.o=.dep} version.h
DISTCLEAN = autoconf.h tests/.deps

$(PROG): $(PROGNAME).a $(MAINFILES)
	$(CC) -o $@ $(MAINFILES) $(PROGNAME).a $(LDFLAGS) $(LDADD) $(LIBS)
	@printf "%10s %-20s\n" LINK $@

win/$(PROGNAME).res: win/$(PROGNAME).rc
	$(RC) $< -O coff -o $@

$(PROGNAME).a: $(OBJECTS)
	$(LINK_STATUS)
	@if test ! -e $@ || $(RM) $@ ; then \
		if $(AR) -rc $@ $(OBJECTS) && $(RANLIB) $@ ; then \
			$(LINK_OK); \
		else \
			$(LINK_FAILED); \
		fi; \
	else \
		$(LINK_FAILED); \
	fi

check: tests
tests: $(PROGNAME).a
	env CC="$(CC)" CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS)" \
		LDFLAGS="$(LDFLAGS)" LDADD="$(LDADD)" LIBS="$(TEST_LIBS)" \
		CROSS_COMPILE="$(CROSS_COMPILE)" \
		TEST_WORKING_DIRECTORY="$(TEST_WORKING_DIRECTORY)" \
		$(MAKE) -C tests all

test-depgen:
	env CC="$(CC)" $(MAKE) -C tests depgen

test-clean:
	env RM="$(RM)" $(MAKE) -C tests clean

# Hack to descend into tests and clean since it isn't included in SUBDIRS.
pre-clean: test-clean

# Track the build number in the dynamically generated file, version.h.
# Use INSTALL_STATUS/INSTALL_OK from buildsys in lieu of something more
# appropriate for automatically generated source files.
version.h: FORCE
	@xversion=`../scripts/version.sh` ; \
	i="$@" ; \
	if test -r "$$i" ; then \
		xoldversion=`grep -E '^/\*"' "$$i" | sed -e 's%^/\*"%%' -e 's%"\*/$$%%' -e 'q'` ; \
	else \
		xoldversion=x$$xversion ; \
	fi ; \
	if test "x$$xversion" != "x$$xoldversion" ; then \
		${INSTALL_STATUS} ; \
		echo "#ifndef VERSION_H" > "$$i" ; \
		echo "#define VERSION_H" >> "$$i" ; \
		echo '/*"'"$$xversion"'"*/' >> "$$i" ; \
		echo " $$xversion" | sed -e '/^[ \t\r]$$/q' -e 's/^ /#define VERSION_STRING "/' -e 's/$$/"/' -e 'q' >> "$$i" ; \
		echo "#endif" >> "$$i" ; \
		${INSTALL_OK} ; \
	fi

# Since version.h is dynamically generated, explicitly specify everything that
# depends on it.
buildid.o: version.h

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

install-extra:
	# For Windows, copy the executable and the DLLs it needs.
	if test x"${ENABLEWIN}" = xyes ; then \
		for i in ${PROG} ; do \
			${INSTALL_STATUS}; \
			if ${INSTALL} -m 755 $$i ../$$i ; then \
				${INSTALL_OK}; \
			else \
				${INSTALL_FAILED}; \
			fi \
		done ; \
		for i in zlib1.dll libpng12.dll ; do \
			${INSTALL_STATUS}; \
			if ${INSTALL} -m 644 win/dll/$$i ../$$i ; then \
				${INSTALL_OK}; \
			else \
				${INSTALL_FAILED}; \
			fi \
		done \
	fi

post-install:
	# For a shared installation, set appropriate owner for executable
	# and mark it as setgid.
	if [ "x$(SETEGID)" != "x" ]; then \
		${INSTALL_STATUS}; \
		if chown root:${SETEGID} ${DESTDIR}${bindir}/${PROG} && chmod g+s ${DESTDIR}${bindir}/${PROG} ; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	fi

FORCE :
.PHONY : check tests coverage clean-coverage tests/ran-already
