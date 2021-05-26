dnl
dnl Copyright (c) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017,
dnl               2018, 2020, 2021
dnl   Jonathan Schleifer <js@nil.im>
dnl
dnl https://fossil.nil.im/buildsys
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
	AC_REQUIRE([AC_CANONICAL_BUILD])
	AC_REQUIRE([AC_CANONICAL_HOST])

	case "$build_os" in
	darwin*)
		case "$host_os" in
		darwin*)
			AC_SUBST(BUILD_AND_HOST_ARE_DARWIN, yes)
			;;
		esac
		;;
	esac

	AC_PROG_INSTALL
	case "$INSTALL" in
	./install-sh*)
		INSTALL="$(MKPATH)/../$INSTALL"
		;;
	esac

	AC_CONFIG_COMMANDS_PRE([
		AS_IF([test x"$GCC" = x"yes"],
			[AC_SUBST(DEP_CFLAGS, '-MD -MF $${out%.o}.dep')])
		AS_IF([test x"$GXX" = x"yes"],
			[AC_SUBST(DEP_CXXFLAGS, '-MD -MF $${out%.o}.dep')])
		AS_IF([test x"$GOBJC" = x"yes"],
			[AC_SUBST(DEP_OBJCFLAGS, '-MD -MF $${out%.o}.dep')])
		AS_IF([test x"$GOBJCXX" = x"yes"],
			[AC_SUBST(DEP_OBJCXXFLAGS, '-MD -MF $${out%.o}.dep')])

		AC_SUBST(AMIGA_LIB_CFLAGS)
		AC_SUBST(AMIGA_LIB_LDFLAGS)

		case "$build_os" in
		morphos*)
			dnl Don't use tput on MorphOS: The colored output is
			dnl quite unreadable and in some MorphOS versions the
			dnl output from tput is not 8-bit safe, with awk (for
			dnl AC_SUBST) failing as a result.
			;;
		*)
			AC_PATH_PROG(TPUT, tput)
			;;
		esac

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
				AC_SUBST(TERM_SETAF2,
					"$($TPUT setaf 2 2>/dev/null)")
				AC_SUBST(TERM_SETAF3,
					"$($TPUT setaf 3 2>/dev/null)")
				AC_SUBST(TERM_SETAF4,
					"$($TPUT setaf 4 2>/dev/null)")
				AC_SUBST(TERM_SETAF6,
					"$($TPUT setaf 6 2>/dev/null)")
			dnl OpenBSD seems to want 3 parameters for terminals
			dnl ending in -256color, but the additional two
			dnl parameters don't seem to do anything, so we set
			dnl them to 0.
			elif x=$($TPUT setaf 1 0 0 2>/dev/null); then
				AC_SUBST(TERM_SETAF1, "$x")
				AC_SUBST(TERM_SETAF2,
					"$($TPUT setaf 2 0 0 2>/dev/null)")
				AC_SUBST(TERM_SETAF3,
					"$($TPUT setaf 3 0 0 2>/dev/null)")
				AC_SUBST(TERM_SETAF4,
					"$($TPUT setaf 4 0 0 2>/dev/null)")
				AC_SUBST(TERM_SETAF6,
					"$($TPUT setaf 6 0 0 2>/dev/null)")
			else
				AC_SUBST(TERM_SETAF1,
					"$($TPUT AF 1 2>/dev/null)")
				AC_SUBST(TERM_SETAF2,
					"$($TPUT AF 2 2>/dev/null)")
				AC_SUBST(TERM_SETAF3,
					"$($TPUT AF 3 2>/dev/null)")
				AC_SUBST(TERM_SETAF4,
					"$($TPUT AF 4 2>/dev/null)")
				AC_SUBST(TERM_SETAF6,
					"$($TPUT AF 6 2>/dev/null)")
			fi
		])
	])
])

AC_DEFUN([BUILDSYS_CHECK_IOS], [
	case "$host_os" in
	darwin*)
		AC_MSG_CHECKING(whether host is iOS)
		AC_EGREP_CPP(yes, [
			#include <TargetConditionals.h>

			#if (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) || \
			    (defined(TARGET_OS_SIMULATOR) && \
			    TARGET_OS_SIMULATOR)
			yes
			#endif
		], [
			host_is_ios="yes"
		], [
			host_is_ios="no"
		])
		AC_MSG_RESULT($host_is_ios)
		;;
	esac
])

AC_DEFUN([BUILDSYS_PROG_IMPLIB], [
	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_MSG_CHECKING(whether we need an implib)
	case "$host_os" in
	cygwin* | mingw*)
		AC_MSG_RESULT(yes)
		PROG_IMPLIB_NEEDED='yes'
		PROG_IMPLIB_LDFLAGS='-Wl,--export-all-symbols,--out-implib,lib${PROG}.a'
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
	AC_REQUIRE([BUILDSYS_CHECK_IOS])
	AC_MSG_CHECKING(for shared library system)

	case "$host" in
	*-*-darwin*)
		AC_MSG_RESULT(Darwin)
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-dynamiclib -current_version ${LIB_MAJOR}.${LIB_MINOR} -compatibility_version ${LIB_MAJOR}'
		LIB_LDFLAGS_INSTALL_NAME='-Wl,-install_name,${libdir}/$${out%.dylib}.${LIB_MAJOR}.dylib'
		LIB_PREFIX='lib'
		LIB_SUFFIX='.dylib'
		LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-bundle ${PLUGIN_LDFLAGS_BUNDLE_LOADER}'
		PLUGIN_SUFFIX='.bundle'
		AS_IF([test x"$host_is_ios" = x"yes"], [
			LINK_PLUGIN='rm -fr $$out && ${MKDIR_P} $$out && if test -f Info.plist; then ${INSTALL} -m 644 Info.plist $$out/Info.plist; fi && ${LD} -o $$out/$${out%${PLUGIN_SUFFIX}} ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS} && ${CODESIGN} -fs ${CODESIGN_IDENTITY} --timestamp=none $$out'
		], [
			LINK_PLUGIN='rm -fr $$out && ${MKDIR_P} $$out/Contents/MacOS && if test -f Info.plist; then ${INSTALL} -m 644 Info.plist $$out/Contents/Info.plist; fi && ${LD} -o $$out/Contents/MacOS/$${out%${PLUGIN_SUFFIX}} ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS} && ${CODESIGN} -fs ${CODESIGN_IDENTITY} --timestamp=none $$out'
		])
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib && ${LN_S} -f $${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib ${DESTDIR}${libdir}/$$i'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%.dylib}.${LIB_MAJOR}.${LIB_MINOR}.dylib'
		INSTALL_PLUGIN='&& rm -fr ${DESTDIR}${plugindir}/$$i && cp -R $$i ${DESTDIR}${plugindir}/'
		UNINSTALL_PLUGIN='&& rm -fr ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	*-*-mingw* | *-*-cygwin*)
		AC_MSG_RESULT(MinGW / Cygwin)
		LIB_CFLAGS=''
		LIB_LDFLAGS='-shared -Wl,--export-all-symbols,--out-implib,lib$${out%${LIB_SUFFIX}}.a'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX=''
		LIB_SUFFIX='${LIB_MAJOR}.dll'
		LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
		PLUGIN_CFLAGS=''
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.dll'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${MKDIR_P} ${DESTDIR}${bindir} && ${INSTALL} -m 755 $$i ${DESTDIR}${bindir}/$$i && ${INSTALL} -m 755 lib$${i%${LIB_SUFFIX}}.a ${DESTDIR}${libdir}/lib$${i%${LIB_SUFFIX}}.a'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${bindir}/$$i ${DESTDIR}${libdir}/lib$$i.a'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB='${SHARED_LIB}.a ${SHARED_LIB_NOINST}.a'
		;;
	*-*-openbsd* | *-*-mirbsd*)
		AC_MSG_RESULT(OpenBSD)
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-shared'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX='lib'
		LIB_SUFFIX='.so.${LIB_MAJOR}.${LIB_MINOR}'
		LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.so'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	*-*-solaris*)
		AC_MSG_RESULT(Solaris)
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-shared -Wl,-soname=$$out.${LIB_MAJOR}.${LIB_MINOR}'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX='lib'
		LIB_SUFFIX='.so'
		LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.so'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR} && rm -f ${DESTDIR}${libdir}/$$i && ${LN_S} $$i.${LIB_MAJOR}.${LIB_MINOR} ${DESTDIR}${libdir}/$$i'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	*-*-android*)
		AC_MSG_RESULT(Android)
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-shared -Wl,-soname=$$out.${LIB_MAJOR}'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX='lib'
		LIB_SUFFIX='.so'
		LDFLAGS_RPATH=''
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.so'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0 && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	hppa*-*-hpux*)
		AC_MSG_RESULT([HP-UX (PA-RISC)])
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-shared -Wl,+h,$$out'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX='lib'
		LIB_SUFFIX='.${LIB_MAJOR}'
		LINK_LIB='&& rm -f $${out%%.*}.sl && ${LN_S} $$out $${out%%.*}.sl'
		LDFLAGS_RPATH='-Wl,+b,${libdir}'
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.sl'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i && ${LN_S} -f $$i ${DESTDIR}${libdir}/$${i%%.*}.sl'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$${i%%.*}.sl'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	ia64*-*-hpux*)
		AC_MSG_RESULT([HP-UX (Itanium)])
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-shared -Wl,+h,$$out'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX='lib'
		LIB_SUFFIX='.${LIB_MAJOR}'
		LINK_LIB='&& rm -f $${out%%.*}.so && ${LN_S} $$out $${out%%.*}.so'
		LDFLAGS_RPATH='-Wl,+b,${libdir}'
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.so'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i && ${LN_S} -f $$i ${DESTDIR}${libdir}/$${i%%.*}.so'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$${i%%.*}.so'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	*)
		AC_MSG_RESULT(ELF)
		LIB_CFLAGS='-fPIC -DPIC'
		LIB_LDFLAGS='-shared -Wl,-soname=$$out.${LIB_MAJOR}'
		LIB_LDFLAGS_INSTALL_NAME=''
		LIB_PREFIX='lib'
		LIB_SUFFIX='.so'
		LDFLAGS_RPATH='-Wl,-rpath,${libdir}'
		PLUGIN_CFLAGS='-fPIC -DPIC'
		PLUGIN_LDFLAGS='-shared'
		PLUGIN_SUFFIX='.so'
		LINK_PLUGIN='${LD} -o $$out ${PLUGIN_OBJS} ${PLUGIN_OBJS_EXTRA} ${PLUGIN_LDFLAGS} ${LDFLAGS} ${LIBS}'
		INSTALL_LIB='&& ${INSTALL} -m 755 $$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0 && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} && ${LN_S} -f $$i.${LIB_MAJOR}.${LIB_MINOR}.0 ${DESTDIR}${libdir}/$$i'
		UNINSTALL_LIB='&& rm -f ${DESTDIR}${libdir}/$$i ${DESTDIR}${libdir}/$$i.${LIB_MAJOR} ${DESTDIR}${libdir}/$$i.${LIB_MAJOR}.${LIB_MINOR}.0'
		INSTALL_PLUGIN='&& ${INSTALL} -m 755 $$i ${DESTDIR}${plugindir}/$$i'
		UNINSTALL_PLUGIN='&& rm -f ${DESTDIR}${plugindir}/$$i'
		CLEAN_LIB=''
		;;
	esac

	AC_SUBST(LIB_CFLAGS)
	AC_SUBST(LIB_LDFLAGS)
	AC_SUBST(LIB_LDFLAGS_INSTALL_NAME)
	AC_SUBST(LIB_PREFIX)
	AC_SUBST(LIB_SUFFIX)
	AC_SUBST(LINK_LIB)
	AC_SUBST(LDFLAGS_RPATH)
	AC_SUBST(PLUGIN_CFLAGS)
	AC_SUBST(PLUGIN_LDFLAGS)
	AC_SUBST(PLUGIN_SUFFIX)
	AC_SUBST(LINK_PLUGIN)
	AC_SUBST(INSTALL_LIB)
	AC_SUBST(UNINSTALL_LIB)
	AC_SUBST(INSTALL_PLUGIN)
	AC_SUBST(UNINSTALL_PLUGIN)
	AC_SUBST(CLEAN_LIB)
])

AC_DEFUN([BUILDSYS_FRAMEWORK], [
	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_REQUIRE([BUILDSYS_CHECK_IOS])
	AC_REQUIRE([BUILDSYS_SHARED_LIB])

	AC_CHECK_TOOL(CODESIGN, codesign)

	case "$host_os" in
	darwin*)
		AS_IF([test x"$host_is_ios" = x"yes"], [
			FRAMEWORK_LDFLAGS='-dynamiclib -current_version ${LIB_MAJOR}.${LIB_MINOR} -compatibility_version ${LIB_MAJOR}'
			FRAMEWORK_LDFLAGS_INSTALL_NAME='-Wl,-install_name,@executable_path/Frameworks/$$out/$${out%.framework}'
		], [
			FRAMEWORK_LDFLAGS='-dynamiclib -current_version ${LIB_MAJOR}.${LIB_MINOR} -compatibility_version ${LIB_MAJOR}'
			FRAMEWORK_LDFLAGS_INSTALL_NAME='-Wl,-install_name,@executable_path/../Frameworks/$$out/$${out%.framework}'
		])

		AC_SUBST(FRAMEWORK_LDFLAGS)
		AC_SUBST(FRAMEWORK_LDFLAGS_INSTALL_NAME)
		AC_SUBST(FRAMEWORK_LIBS)

		$1
		;;
	esac
])
