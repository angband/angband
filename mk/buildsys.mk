#
#  Copyright (c) 2007, Jonathan Schleifer <js-buildsys@webkeks.org>
#
#  Permission to use, copy, modify, and/or distribute this software for any
#  purpose with or without fee is hereby granted, provided that the above
#  copyright notice and this permission notice is present in all copies.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

include $(MKPATH)extra.mk

OBJS1 = ${SRCS:.c=.o}
OBJS2 = ${OBJS1:.cc=.o}
OBJS3 = ${OBJS2:.cxx=.o}
OBJS4 = ${OBJS3:.d=.o}
OBJS5 = ${OBJS4:.erl=.beam}
OBJS += ${OBJS5:.m=.o}

.SILENT:
.SUFFIXES: .beam .c .cc .cxx .d .erl .m
.PHONY: all subdirs pre-depend depend install install-extra uninstall uninstall-extra clean distclean

all:
	for i in subdirs depend ${STATIC_LIB} ${STATIC_LIB_NOINST} ${LIB} ${LIB_NOINST} ${PLUGIN} ${PLUGIN_NOINST} ${PROG} ${PROG_NOINST}; do \
		${MAKE} ${MFLAGS} $$i || exit 1; \
	done

subdirs:
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} || exit 1; \
		${DIR_LEAVE}; \
	done

depend: pre-depend ${SRCS}
	regen=0; \
	test -f .deps || regen=1; \
	for i in ${SRCS}; do test $$i -nt .deps && regen=1; done; \
	if test x"$$regen" = x"1"; then \
		list=""; \
		echo > .deps; \
		${DEPEND_STATUS}; \
		for i in ${SRCS}; do \
			case $${i##*.} in \
			c|cc|cxx|m) \
				${CPP} ${CPPFLAGS} -M $$i -o .deptemp; \
				cat .deptemp >> .deps; \
				$(RM) .deptemp; \
				;; \
			esac; \
		done; \
	fi

pre-depend:

${PROG} ${PROG_NOINST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${LD} -o $@ ${OBJS} ${LDFLAGS} ${LIBS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

${LIB} ${LIB_NOINST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${LD} -o $@ ${OBJS} ${LIB_LDFLAGS} ${LDFLAGS} ${LIBS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

${PLUGIN} ${PLUGIN_NONST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${LD} -o $@ ${OBJS} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

${STATIC_LIB} ${STATIC_LIB_NOINST}: ${EXT_DEPS} ${OBJS}
	${LINK_STATUS}
	if ${AR} cr $@ ${OBJS}; then \
		${LINK_OK}; \
	else \
		${LINK_FAILED}; \
	fi

.c.o:
	${COMPILE_STATUS}
	if ${CC} ${CFLAGS} ${CPPFLAGS} ${INCLUDE} -c -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

.cc.o .cxx.o:
	${COMPILE_STATUS}
	if ${CXX} ${CXXFLAGS} ${CPPFLAGS} -c -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

.d.o:
	${COMPILE_STATUS}
	if test x"$(basename ${DC})" = x"dmd"; then \
		if ${DC} ${DFLAGS} -c -of$@ $<; then \
			${COMPILE_OK}; \
		else \
			${COMPILE_FAILED}; \
		fi \
	else \
		if ${DC} ${DFLAGS} -c -o $@ $<; then \
			${COMPILE_OK}; \
		else \
			${COMPILE_FAILED}; \
		fi \
	fi

.erl.beam:
	${COMPILE_STATUS}
	if ${ERLC} ${ERLCFLAGS} -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

.m.o:
	${COMPILE_STATUS}
	if ${OBJC} ${OBJCFLAGS} ${CPPFLAGS} -c -o $@ $<; then \
		${COMPILE_OK}; \
	else \
		${COMPILE_FAILED}; \
	fi

install: ${LIB} ${STATIC_LIB} ${PLUGIN} ${PROG} ${CONFIG} ${LIBDATA} ${VARDATA} ${INCLUDES} ${MAN} install-extra
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} install || exit 1; \
		${DIR_LEAVE}; \
	done

	for i in ${LIB}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${libdir} && ${INSTALL_LIB}; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${STATIC_LIB}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${libdir} && ${INSTALL} -m 644 $$i ${DESTDIR}${libdir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${PLUGIN}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${plugindir} && ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${VARDATA}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} $$(dirname ${DESTDIR}${vardatadir}${PACKAGE}/$$i) && ${INSTALL} -m 644 $$i ${DESTDIR}${vardatadir}${PACKAGE}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${LIBDATA}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} $$(dirname ${DESTDIR}${libdatadir}${PACKAGE}/$$i) && ${INSTALL} -m 644 $$i ${DESTDIR}${libdatadir}${PACKAGE}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${CONFIG}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} $$(dirname ${DESTDIR}${configdir}${PACKAGE}/$$i) && ${INSTALL} -m 644 $$i ${DESTDIR}${configdir}${PACKAGE}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${PROG}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${bindir} && ${INSTALL} -m 755 $$i ${DESTDIR}${bindir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi; \
		\
		if test "x$(SETEGID)" != "x"; then \
			printf "%10s $(DESTDIR)${bindir}/$$i\n" CHOWN; \
			chown root:${SETEGID} $(DESTDIR)${bindir}/$$i; \
			chmod g+s $(DESTDIR)${bindir}/$$i; \
		fi \
	done

	for i in ${INCLUDES}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${includedir}/${includesubdir} && ${INSTALL} -m 644 $$i ${DESTDIR}${includedir}/${includesubdir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

	for i in ${MAN}; do \
		${INSTALL_STATUS}; \
		if ${MKDIR_P} ${DESTDIR}${mandir}/${mansubdir} && ${INSTALL} -m 644 $$i ${DESTDIR}${mandir}/${mansubdir}/$$i; then \
			${INSTALL_OK}; \
		else \
			${INSTALL_FAILED}; \
		fi \
	done

install-extra:

uninstall: uninstall-extra
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} uninstall || exit 1; \
		${DIR_LEAVE}; \
	done

	for i in ${LIB}; do \
		if test -f ${DESTDIR}${libdir}/$$i; then \
			if ${UNINSTALL_LIB}; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi; \
	done

	for i in ${STATIC_LIB}; do \
		if test -f ${DESTDIR}${libdir}/$$i; then \
			if rm -f ${DESTDIR}${libdir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

	for i in ${PLUGIN}; do \
		if test -f ${DESTDIR}${plugindir}/$$i; then \
			if rm -f ${DESTDIR}${plugindir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done
	-rmdir ${DESTDIR}${plugindir} >/dev/null 2>&1

	for i in ${DATA}; do \
		if test -f ${DESTDIR}${datadir}/${PACKAGE}/$$i; then \
			if rm -f ${DESTDIR}${datadir}/${PACKAGE}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

	for i in ${PROG}; do \
		if test -f ${DESTDIR}${bindir}/$$i; then \
			if rm -f ${DESTDIR}${bindir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

	for i in ${INCLUDES}; do \
		if test -f ${DESTDIR}${includedir}/${includesubdir}/$$i; then \
			if rm -f ${DESTDIR}${includedir}/${includesubdir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done
	-rmdir ${DESTDIR}${includedir}/${includesubdir} >/dev/null 2>&1

	for i in ${MAN}; do \
		if test -f ${DESTDIR}${mandir}/${mansubdir}/$$i; then \
			if rm -f ${DESTDIR}${mandir}/${mansubdir}/$$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

uninstall-extra:

clean::
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} clean || exit 1; \
		${DIR_LEAVE}; \
	done

	for i in ${OBJS} ${CLEAN} ${CLEAN_LIB} .deps; do \
		if test -f $$i -o -d $$i; then \
			if rm -fr $$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done

distclean: clean
	for i in ${SUBDIRS}; do \
		${DIR_ENTER}; \
		${MAKE} ${MFLAGS} distclean || exit 1; \
		${DIR_LEAVE}; \
	done

	for i in ${PROG} ${PROG_NOINST} ${LIB} ${LIB_NOINST} ${STATIC_LIB} ${STATIC_LIB_NOINST} ${PLUGIN} ${PLUGIN_NOINST} ${DISTCLEAN} *~; do \
		if test -f $$i -o -d $$i; then \
			if rm -fr $$i; then \
				${DELETE_OK}; \
			else \
				${DELETE_FAILED}; \
			fi \
		fi \
	done
	rm -rf configure *.mk autom4te.cache config.log config.status aclocal.m4 src/autoconf.* mk/extra.mk mk/sinclude.mk

DIR_ENTER = cd $$i || exit 1
DIR_LEAVE = cd .. || exit 1
DEPEND_STATUS = printf "Generating dependencies...\r"
DEPEND_OK = printf "Successfully generated dependencies.\n"
DEPEND_FAILED = printf "Failed to generate dependencies\!\n"; exit 1
COMPILE_STATUS = printf "\033[K\033[0;33mCompiling \033[1;33m$<\033[0;33m...\033[0m\r"
COMPILE_OK = printf "\033[K\033[0;32mSuccessfully compiled \033[1;32m$<\033[0;32m.\033[0m\n"
COMPILE_FAILED = printf "\033[K\033[0;31mFailed to compile \033[1;31m$<\033[0;31m!\033[0m\n"; exit 1
LINK_STATUS = printf "\033[K\033[0;33mLinking \033[1;33m$@\033[0;33m...\033[0m\r"
LINK_OK = printf "\033[K\033[0;32mSuccessfully linked \033[1;32m$@\033[0;32m.\033[0m\n"
LINK_FAILED = printf "\033[K\033[0;31mFailed to link \033[1;31m$@\033[0;31m!\033[0m\n"; exit 1
INSTALL_STATUS = printf "\033[K\033[0;33mInstalling \033[1;33m$$i\033[0;33m...\033[0m\r"
INSTALL_OK = printf "\033[K\033[0;32mSuccessfully installed \033[1;32m$$i\033[0;32m.\033[0m\n"
INSTALL_FAILED = printf "\033[K\033[0;31mFailed to install \033[1;31m$$i\033[0;31m!\033[0m\n"; exit 1
DELETE_OK = printf "\033[K\033[0;34mDeleted \033[1;34m$$i\033[0;34m.\033[0m\n"
DELETE_FAILED = printf "\033[K\033[0;31mFailed to delete \033[1;31m$$i\033[0;31m!\033[0m\n"; exit 1

FILE := .depend
include $(MKPATH)sinclude.mk
