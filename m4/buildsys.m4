dnl
dnl Copyright (c) 2007 - 2009, Jonathan Schleifer <js@webkeks.org>
dnl
dnl https://webkeks.org/hg/buildsys/
dnl
dnl Permission to use, copy, modify, and/or distribute this software for any
dnl purpose with or without fee is hereby granted, provided that the above
dnl copyright notice and this permission notice is present in all copies.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
dnl LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
dnl CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl

AC_DEFUN([BUILDSYS_INIT], [
	AC_PATH_PROG(TPUT, tput)

	AS_IF([test x"$TPUT" != x""], [
		if x=$($TPUT el 2>/dev/null); then
			AC_SUBST(TERM_EL, "$x")
		else
			AC_SUBST(TERM_EL, "$($TPUT ce 2>/dev/null)")
		fi

		if x=$($TPUT sgr0 2>/dev/null); then
			AC_SUBST(TERM_SGR0, "$x")
		else
			AC_SUBST(TERM_SGR0, "$($TPUT me 2>/dev/null)")
		fi

		if x=$($TPUT bold 2>/dev/null); then
			AC_SUBST(TERM_BOLD, "$x")
		else
			AC_SUBST(TERM_BOLD, "$($TPUT md 2>/dev/null)")
		fi

		if x=$($TPUT setaf 1 2>/dev/null); then
			AC_SUBST(TERM_SETAF1, "$x")
			AC_SUBST(TERM_SETAF2, "$($TPUT setaf 2 2>/dev/null)")
			AC_SUBST(TERM_SETAF3, "$($TPUT setaf 3 2>/dev/null)")
			AC_SUBST(TERM_SETAF4, "$($TPUT setaf 4 2>/dev/null)")
			AC_SUBST(TERM_SETAF6, "$($TPUT setaf 6 2>/dev/null)")
		else
			AC_SUBST(TERM_SETAF1, "$($TPUT AF 1 2>/dev/null)")
			AC_SUBST(TERM_SETAF2, "$($TPUT AF 2 2>/dev/null)")
			AC_SUBST(TERM_SETAF3, "$($TPUT AF 3 2>/dev/null)")
			AC_SUBST(TERM_SETAF4, "$($TPUT AF 4 2>/dev/null)")
			AC_SUBST(TERM_SETAF6, "$($TPUT AF 6 2>/dev/null)")
		fi
	], [
		AC_SUBST(TERM_EL, '\033\133K')
		AC_SUBST(TERM_SGR0, '\033\133m')
		AC_SUBST(TERM_BOLD, '\033\1331m')
		AC_SUBST(TERM_SETAF1, '\033\13331m')
		AC_SUBST(TERM_SETAF2, '\033\13332m')
		AC_SUBST(TERM_SETAF3, '\033\13333m')
		AC_SUBST(TERM_SETAF4, '\033\13334m')
		AC_SUBST(TERM_SETAF6, '\033\13336m')
	])
])

AC_DEFUN([BUILDSYS_PROG_IMPLIB], [
	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_MSG_CHECKING(whether we need an implib)
	case "$host_os" in
		cygwin* | mingw*)
			AC_MSG_RESULT(yes)
			PROG_IMPLIB_NEEDED='yes'
			PROG_IMPLIB_LDFLAGS='-Wl,-export-all-symbols,--out-implib,lib${PROG}.a'
			;;
		*)
			AC_MSG_RESULT(no)
			PROG_IMPLIB_NEEDED='no'
			PROG_IMPLIB_LDFLAGS=''
			;;
	esac

	AC_SUBST(PROG_IMPLIB_NEEDED)
	AC_SUBST(PROG_IMPLIB_LDFLAGS)
])

AC_DEFUN([BUILDSYS_SHARED_LIB], [
	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_MSG_CHECKING(for shared library system)
	case "$host_os" in
		darwin*)
			AC_MSG_RESULT(Darwin)
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-dynamiclib -current_version ${LIB_MAJOR}.${LIB_MINOR} -compatibility_version ${LIB_MAJOR}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.dylib'
			LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
			PLUGIN_CFLAGS='-fPIC -DPIC'
			PLUGIN_LDFLAGS='-bundle -undefined dynamic_lookup'
			PLUGIN_SUFFIX='.impl'
			INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			CLEAN_LIB=''
			;;
		solaris*)
			AC_MSG_RESULT(Solaris)
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-shared -Wl,-soname=${LIB}.${LIB_MAJOR}.${LIB_MINOR}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so'
			LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
			PLUGIN_CFLAGS='-fPIC -DPIC'
			PLUGIN_LDFLAGS='-shared'
			PLUGIN_SUFFIX='.so'
			INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR} && rm -f ${DESTDIR}${libdir}/$$i && ${LN_S} $$i.${LIB_MAJOR}.${LIB_MINOR} ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}'
			CLEAN_LIB=''
			;;
		openbsd* | mirbsd*)
			AC_MSG_RESULT(OpenBSD)
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-shared'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so.${LIB_MAJOR}.${LIB_MINOR}'
			LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
			PLUGIN_CFLAGS='-fPIC -DPIC'
			PLUGIN_LDFLAGS='-shared'
			PLUGIN_SUFFIX='.so'
			INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i'
			CLEAN_LIB=''
			;;
		cygwin* | mingw*)
			AC_MSG_RESULT(Win32)
			LIB_CFLAGS=''
			LIB_LDFLAGS='-shared -Wl,--out-implib,${LIB}.a'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.dll'
			LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
			PLUGIN_CFLAGS=''
			PLUGIN_LDFLAGS='-shared'
			PLUGIN_SUFFIX='.dll'
			INSTALL_LIB='&& ${MKDIR_P} ${DESTDIR}${bindir} && ${INSTALL} -m 755 $$i ${DESTDIR}${bindir}/$$i && ${INSTALL} -m 755 $$i.a ${DESTDIR}${libdir}/$$i.a'
			UNINSTALL_LIB='&& rm -f ${DESTDIR}${bindir}/$$i ${DESTDIR}${libdir}/$$i.a'
			CLEAN_LIB='${LIB}.a'
			;;
		*)
			AC_MSG_RESULT(GNU)
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-shared -Wl,-soname=${LIB}.${LIB_MAJOR}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so'
			LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
			PLUGIN_CFLAGS='-fPIC -DPIC'
			PLUGIN_LDFLAGS='-shared'
			PLUGIN_SUFFIX='.so'
			INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0 && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i'
			UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0'
			CLEAN_LIB=''
			;;
	esac

	AC_SUBST(LIB_CFLAGS)
	AC_SUBST(LIB_LDFLAGS)
	AC_SUBST(LIB_PREFIX)
	AC_SUBST(LIB_SUFFIX)
	AC_SUBST(LDFLAGS_RPATH)
	AC_SUBST(PLUGIN_CFLAGS)
	AC_SUBST(PLUGIN_LDFLAGS)
	AC_SUBST(PLUGIN_SUFFIX)
	AC_SUBST(INSTALL_LIB)
	AC_SUBST(UNINSTALL_LIB)
	AC_SUBST(CLEAN_LIB)
])

AC_DEFUN([BUILDSYS_TOUCH_DEPS], [
	${as_echo:="echo"} ${as_me:="configure"}": touching .deps files"
	for i in $(find . -name Makefile); do
		DEPSFILE="$(dirname $i)/.deps"
		test -f "$DEPSFILE" && rm "$DEPSFILE"
		touch -t 0001010000 "$DEPSFILE"
	done
])
