dnl Process this file with autoconf to produce a configure script.
AC_INIT([Angband],[4.2.5],[bugs@rephial.org])
AC_CONFIG_SRCDIR(src)
AC_CONFIG_MACRO_DIR([m4])

for i in configure.ac m4/*; do
	AS_IF([test $i -nt configure], [
		AC_MSG_ERROR([$i is newer than configure! Run ./autogen.sh!])
	])
done

BUILDSYS_INIT

AC_SUBST(PACKAGE, angband)

AC_CONFIG_HEADERS(src/autoconf.h)


AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package])

AC_ARG_WITH(setgid,
	[AS_HELP_STRING([--with-setgid=NAME], [install angband as group NAME])],
	[wsetgid=yes])
AC_ARG_WITH(private_dirs,
	[AS_HELP_STRING([--with-private-dirs], [use private scorefiles/savefiles])])
AC_ARG_WITH(no_install,
	[AS_HELP_STRING([--with-no-install], [don't install, just run in-place])])

AS_IF([test "x$with_setgid" = "xyes"],
	[AC_MSG_ERROR([Please specify a group to install as.])])

AS_IF([test "x$wsetgid" = "x" && test "x$with_no_install" = "x"],
	[with_private_dirs="yes"])

AS_IF([test "x$wsetgid$with_private_dirs$with_no_install" != "xyes"],
	[echo "Please run ./configure with only one of:

--with-setgid=<groupname>
    This configures the game to store savefiles and scorefiles in a central
    location on the system, writeable only by the group \"groupname\".  If you
    want a traditional installation of Angband, where all users share a
    scorefile, then use this option, and set the groupname to your \"games\"
    group.  This option requires the game to have the \"setgid\" bit set on its
    permissions, and thus this option requires root when installing.

--with-private-dirs
    This configures the game to store savefiles and scorefiles for each user
    in that user's home directory.  The game requires no special privileges if
    you do this, and should be used for most installations of the game. This
    is the default, if you specify none of these three options.

--with-no-install
    This leaves the game to run from the directory into which it was extracted
    and compiled, touching nothing else on the system - make install is not
    run.
"
	AC_MSG_ERROR([Please specify an installation method.])])

AS_IF([test "x$with_no_install" = "x"],
	[echo "Note: You have chosen to compile for installation, with data files
    in standard locations. For development, you may wish to consider using
    --with-no-install which will leave the game to run from the directory
    into which it was extracted and compiled.
"])

AC_ARG_WITH(sphinx,
	[AS_HELP_STRING([--with-sphinx], [build the documentation with sphinx-build])])
AC_PATH_PROGS([SPHINXBUILD],
	[sphinx-build sphinx-build3 sphinx-build2],
	[NOTFOUND],
	[$PATH$PATH_SEPARATOR/usr/share/sphinx/scripts/python3$PATH_SEPARATOR/usr/share/sphinx/scripts/python2])
AC_ARG_VAR([SPHINXBUILD], [full path to sphinx-build for building documentation; overrides auto-detected path])
AC_ARG_VAR([DOC_HTML_THEME], [the builtin Sphinx HTML theme to use or, if not set or empty, use the theme configured in docs/conf.py (currently the better theme)])
AS_IF([test "x$with_sphinx" = "xyes"],
	[AS_IF([test x"$SPHINXBUILD" = x || test x"$SPHINXBUILD" = xNOTFOUND],
		[AC_MSG_ERROR([--with-sphinx specified but could not locate sphinx-build.  Set SPHINXBUILD to the full path to sphinx-build.])])],
	[SPHINXBUILD=])

# Prefer cc to gcc (default behavior is the opposite) for better compatibility
# with OpenBSD and FreeBSD.
AC_PROG_CC([cc gcc clang])
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_RANLIB
AC_CHECK_TOOL(RC, windres, no)
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)

AC_HEADER_DIRENT
AC_CHECK_HEADERS([fcntl.h])
AC_HEADER_STDBOOL
AC_CHECK_FUNCS([mkdir setresgid setegid stat])

dnl needed because h-basic.h checks for this define for autoconf support.
CPPFLAGS="$CPPFLAGS -DHAVE_CONFIG_H"
CPPFLAGS="$CPPFLAGS -I." 

AC_ARG_ENABLE(release,
	[AS_HELP_STRING([--enable-release], [enable a release build])],
	[AS_CASE(${enableval},
		[yes], [release=yes],
		[no], [release=no],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-release])])],
	[release=no])

dnl Currently only adding -DNDEBUG to CFLAGS.  Also leave something in
dnl autoconf.h so reconfiguring with or without release build will trigger
dnl rebuilds without running make clean.
AS_IF([test x"$release" = xyes],
	[CFLAGS="$CFLAGS -DNDEBUG"
	AC_DEFINE(RELEASE_BUILD, 1, [Define to mark as a release build with extra optimizations.])])

AS_IF([test "$GCC" = "yes"],
	[CFLAGS="$CFLAGS -W -Wall -Wextra -Wno-unused-parameter -pedantic"
	CFLAGS="$CFLAGS -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings"
	CFLAGS="$CFLAGS -Wnested-externs -Wshadow"])

MY_PROG_MAKE_SYSVINC
MY_PROG_MAKE_SINCLUDE

dnl Work around an autoconf bug.
AS_IF([test "$prefix" = "NONE"],
	[prefix="${ac_default_prefix}"])
AS_IF([test "$exec_prefix" = "NONE"],
	[exec_prefix="${prefix}"])

AS_IF([test "x$with_private_dirs" != "x"],
	[AC_DEFINE(USE_PRIVATE_PATHS, 1, [Define to use private save and score paths.])])

# Only change bindir if it's the configure-supplied default, which handily doesn't expand exec_prefix
AS_IF([test "x$bindir" = "x\${exec_prefix}/bin"],
	[bindir=${exec_prefix}/games])

AS_IF([test "x$with_no_install" != "x"],
	[configpath="${PWD}/lib/"],
	[AS_IF([test "x$enable_win" = xyes],
		[configpath=".\\\\lib\\\\"],
		[configpath="${sysconfdir}/${PACKAGE}/"])])

AS_IF([test "x$enable_win" != xyes],
	[AS_CASE(["/$configpath"],
		[[*/]], [MY_EXPAND_DIR(configdir, "$configpath")],
		[MY_EXPAND_DIR(configdir, "$configpath/")])],
	[configdir="$configpath"])

AS_IF([test "x$with_no_install" != "x"],
	[libpath="${PWD}/lib/"
	bindir=".."],
	[AS_IF([test "x$enable_win" = xyes],
		[libpath=".\\\\lib\\\\"],
		[libpath="${datarootdir}/${PACKAGE}/"])])

AS_IF([test "x$enable_win" != xyes],
	[AS_CASE(["/$libpath"],
		[[*/]], [MY_EXPAND_DIR(libdatadir, "$libpath")],
		[MY_EXPAND_DIR(libdatadir, "$libpath/")])],
	[libdatadir="$libpath"])

AS_IF([test "x$with_no_install" != "x"],
	[docdir="${PWD}/doc/"],
	[# Only change docdir if it's the configure-supplied default, which handily doesn't expand prefix
	AS_IF([test "x$docdir" = "x\${datarootdir}/doc/\${PACKAGE_TARNAME}"],
		[docdir=${datarootdir}/doc/${PACKAGE}])])

AS_CASE(["/$docdir"],
	[[*/]], [MY_EXPAND_DIR(docdatadir, "$docdir")],
	[MY_EXPAND_DIR(docdatadir, "$docdir/")])

AS_IF([test "x$with_no_install" != "x"],
	[varpath="${PWD}/lib/"],
	[AS_IF([test "x$enable_win" = xyes],
		[varpath=".\\\\lib\\\\"],
		[varpath="${localstatedir}/games/${PACKAGE}/"])])

AS_IF([test "x$enable_win" != xyes],
	[AS_CASE([/$varpath],
		[[*/]], [MY_EXPAND_DIR(vardatadir, "$varpath")],
		[MY_EXPAND_DIR(vardatadir, "$varpath/")])],
	[vardatadir="$varpath"])

AS_IF([test "x$with_private_dirs" != "x"],
	[varshareddatadir="${vardatadir}user/"],
	[varshareddatadir="${vardatadir}"])

AC_ARG_WITH(gamedata_in_lib,
	[AS_HELP_STRING([--with-gamedata-in-lib], [store the game data in the lib path.])])

GAMEDATA_IN_LIB="false"
AS_IF([test "x$with_gamedata_in_lib" = "xyes"],
	[GAMEDATA_IN_LIB="true"
	AC_DEFINE(GAMEDATA_IN_LIB, 1, [Define to store the game data in the lib path.])])
AC_SUBST(GAMEDATA_IN_LIB)

AC_SUBST([configdir])
AC_SUBST([libdatadir])
AC_SUBST([vardatadir])
AC_SUBST([varshareddatadir])
AC_SUBST([docdatadir])

AC_DEFINE_UNQUOTED([DEFAULT_CONFIG_PATH], "${configdir}", [Path to the game's configuration data])
AC_DEFINE_UNQUOTED([DEFAULT_LIB_PATH], "${libdatadir}", [Path to the game's lib directory])
AC_DEFINE_UNQUOTED([DEFAULT_DATA_PATH], "${vardatadir}", [Path to the game's variable data])

NOINSTALL="$with_no_install"; AC_SUBST(NOINSTALL)
SETEGID="$with_setgid";      AC_SUBST(SETEGID)
AS_IF([test "x$wsetgid" = "xyes"],
	[AC_DEFINE(SETGID, 1, [Define if running as a central install on a multiuser system that has setresgid or setegid support.])])

dnl Borg
AC_ARG_ENABLE(borg,
	[AS_HELP_STRING([--enable-borg], [enable Borg (default enabled)])],
	[enable_borg=$enableval],
	[enable_borg=yes])
AC_ARG_ENABLE(borg_high_scores,
	[AS_HELP_STRING([--enable-borg-high-scores], [allow Borg characters to appear in the high scores (default: disabled)])],
	[enable_borg_high_scores=$enableval],
	[enable_borg_high_scores=no])

AS_IF([test x"$enable_borg" = xyes],
	[AC_DEFINE(ALLOW_BORG, 1, [Define if you want Borg support compiled.])])
AS_IF([test x"$enable_borg_high_scores" = xyes],
	[AC_DEFINE(SCORE_BORGS, 1, [Define if you want Borg characters to appear in the high scores.])])

dnl Frontends
AC_ARG_ENABLE(curses,
	[AS_HELP_STRING([--enable-curses], [enable Curses frontend (default: enabled)])],
	[enable_curses=$enableval],
	[enable_curses=default])
AC_ARG_ENABLE(x11,
	[AS_HELP_STRING([--enable-x11], [enable X11 frontend (default: enabled)])],
	[enable_x11=$enableval],
	[enable_x11=default])
AC_ARG_ENABLE(sdl2,
	[AS_HELP_STRING([--enable-sdl2], [enable SDL2 frontend (default: disabled)])],
	[enable_sdl2=$enableval],
	[enable_sdl2=no])
AC_ARG_ENABLE(sdl,
	[AS_HELP_STRING([--enable-sdl], [enable SDL frontend (default: disabled)])],
	[enable_sdl=$enableval],
	[enable_sdl=no])
AC_ARG_ENABLE(win,
	[AS_HELP_STRING([--enable-win], [enable Windows frontend (default: disabled)])],
	[enable_win=$enableval],
	[enable_win=no])
AC_ARG_ENABLE(test,
	[AS_HELP_STRING([--enable-test], [enable test frontend (default: disabled)])],
	[enable_test=$enableval],
	[enable_test=no])
AC_ARG_ENABLE(stats,
	[AS_HELP_STRING([--enable-stats], [enable stats frontend (default: disabled)])],
	[enable_stats=$enableval],
	[enable_stats=no])
AC_ARG_ENABLE(spoil,
	[AS_HELP_STRING([--enable-spoil], [enable command-line spoiler generation (default: enabled)])],
	[enable_spoil=$enableval],
	[enable_spoil=default])

dnl Sound modules
AC_ARG_ENABLE(sdl2_mixer,
	[AS_HELP_STRING([--enable-sdl2-mixer], [enable SDL2 mixer sound support (default: disabled unless SDL2 enabled)])],
	[enable_sdl2_mixer=$enable_sdl2_mixer],
	[enable_sdl2_mixer=$enable_sdl2])
AC_ARG_ENABLE(sdl_mixer,
	[AS_HELP_STRING([--enable-sdl-mixer], [enable SDL mixer sound support (default: disabled unless SDL enabled)])],
	[enable_sdl_mixer=$enable_sdl_mixer],
	[enable_sdl_mixer=$enable_sdl])

MAINFILES="\$(BASEMAINFILES)"

dnl The libraries needed when linking the test cases start out the same
dnl as what is needed to link the game but, typically, do not need to pick
dnl up the extra dependencies needed by the front ends.
TEST_LIBS="${LIBS}"

dnl The Windows front end does not allow access to the others so disable them
dnl unless explicitly told to build them.
AS_IF([test x"$enable_win" = xyes],
	[default_override=no],
	[default_override=yes])
AS_IF([test x"$enable_curses" = xdefault],
	[enable_curses="$default_override"])
AS_IF([test x"$enable_x11" = xdefault],
	[enable_x11="$default_override"])
AS_IF([test x"$enable_spoil" = xdefault],
	[enable_spoil="$default_override"])

dnl curses checking
AS_IF([test "$enable_curses" = "yes"],
	[AM_PATH_NCURSESW(with_curses=yes, with_curses=no)
	AS_IF([test "$with_curses" = "yes"],
		[AC_DEFINE(USE_NCURSES, 1, [Define to 1 if NCurses is found.])
		AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
		CPPFLAGS="${CPPFLAGS} ${NCURSES_CFLAGS}"
		LIBS="${LIBS} ${NCURSES_LIBS}"
		MAINFILES="${MAINFILES} \$(GCUMAINFILES)"],
		[AC_CHECK_LIB([ncursesw], [initscr], [
			AC_DEFINE(USE_NCURSES, 1, [Define to 1 if NCurses is found.])
			AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
			AC_DEFINE(_XOPEN_SOURCE_EXTENDED, 1, [Defined for systems that guard ncurses decls with _XOPEN_SOURCE_EXTENDED])
			with_curses=yes
			LIBS="${LIBS} -lncursesw"
			MAINFILES="${MAINFILES} \$(GCUMAINFILES)"
		])
		AC_SEARCH_LIBS([keypad], [tinfow tinfo])])])

AC_CHECK_FUNCS([mvwaddnwstr use_default_colors can_change_color])

dnl X11 checking
with_x11=no
AS_IF([test "$enable_x11" = "yes"],
	[AC_PATH_XTRA
	AS_IF([test "x$have_x" = "xyes"],
		[AC_DEFINE(USE_X11, 1, [Define to 1 if using the X11 frontend and X11 libraries are found.])
		CPPFLAGS="$CPPFLAGS $X_CFLAGS"
		LIBS="${LIBS} ${X_PRE_LIBS} ${X_LIBS} -lX11 ${X_EXTRA_LIBS}"
		MAINFILES="${MAINFILES} \$(X11MAINFILES)"
		with_x11=yes])])
ENABLEX11="$with_x11"; AC_SUBST(ENABLEX11)

with_sdl2=no
AS_IF([test "$enable_sdl2" = "yes" || test "$enable_sdl2_mixer" = "yes"],
	[dnl SDL2 checking
	AM_PATH_SDL2(2.0.0,,)

	found_sdl2_mixer=no
	AS_IF([test "$SDL2_CONFIG" != "no"],
		[hold_CPPFLAGS="${CPPFLAGS}"
		hold_LIBS="${LIBS}"
		CPPFLAGS="${CPPFLAGS} ${SDL2_CFLAGS}"
		LIBS="${LIBS} ${SDL2_LIBS}"
		AS_IF([test "$enable_sdl2" = "yes"],
			[AC_CHECK_LIB(SDL2_image, IMG_LoadPNG_RW, with_sdl2=yes, with_sdl2=no)
			AC_CHECK_LIB(SDL2_ttf, TTF_Init, with_sdl2x=yes, with_sdl2=no)
			AS_IF([test "$with_sdl2" = "yes"],
				[AC_DEFINE(USE_SDL2, 1, [Define to 1 if using the SDL2 interface and SDL2 is found.])
				LIBS="${LIBS} -lSDL2_image -lSDL2_ttf"
				MAINFILES="${MAINFILES} \$(SDL2MAINFILES)"])])
		AS_IF([test "$enable_sdl2_mixer" = "yes"],
			[AC_CHECK_LIB(SDL2_mixer, Mix_OpenAudio, found_sdl2_mixer=yes, found_sdl2_mixer=no)
			AS_IF([test "$found_sdl2_mixer" = "yes"],
				[AC_DEFINE(SOUND_SDL2, 1, [Define to 1 if using SDL2_mixer sound support and it's found.])
				AC_DEFINE(SOUND, 1, [Define to 1 if including sound support.])
				LIBS="${LIBS} -lSDL2_mixer"
				MAINFILES="${MAINFILES} \$(SNDSDLFILES)"])])
		AS_IF([test "$with_sdl2" = "yes" || test "$found_sdl2_mixer" = "yes"],
			[AS_IF([test "$enable_sdl" = "yes"],
				[AC_MSG_WARN([disabling SDL:  --enable-sdl2 or --enable-sdl2-mixer also set and SDL2 was found; can not mix SDL2 and SDL])
				enable_sdl=no])
			AS_IF([test "$enable_sdl_mixer" = "yes"],
				[AC_MSG_WARN([disabling SDL mixer:  --enable-sdl2 or --enable-sdl2-mixer also set and SDL2 was found; can not mix SDL2 and SDL])
				enable_sdl_mixer=no])],
			[dnl Not using SDL2 so restore flags.
			CPPFLAGS="${hold_CPPFLAGS}"
			LIBS="${hold_LIBS}"])])])
ENABLESDL2="$with_sdl2"; AC_SUBST(ENABLESDL2)
with_sdl=no
AS_IF([test "$enable_sdl" = "yes" || test "$enable_sdl_mixer" = "yes"],
	[dnl SDL checking
	AM_PATH_SDL(1.2.10,,)

	found_sdl_mixer=no
	AS_IF([test "$SDL_CONFIG" != "no"],
		[hold_CPPFLAGS="${CPPFLAGS}"
		hold_LIBS="${LIBS}"
		CPPFLAGS="${CPPFLAGS} ${SDL_CFLAGS}"
		LIBS="${LIBS} ${SDL_LIBS}"
		AS_IF([test "$enable_sdl" = "yes"],
			[AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, with_sdl=yes, with_sdl=no)
			AC_CHECK_LIB(SDL_ttf, TTF_Init, with_sdlx=yes, with_sdl=no)
			AS_IF([test "$with_sdl" = "yes"],
				[AC_DEFINE(USE_SDL, 1, [Define to 1 if using the SDL interface and SDL is found.])
				LIBS="${LIBS} -lSDL_image -lSDL_ttf"
				MAINFILES="${MAINFILES} \$(SDLMAINFILES)"])])
		AS_IF([test "$enable_sdl_mixer" = "yes"],
			[AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio, found_sdl_mixer=yes, found_sdl_mixer=no)
			AS_IF([test "$found_sdl_mixer" = "yes"],
				[AC_DEFINE(SOUND_SDL, 1, [Define to 1 if using SDL_mixer sound support and it's found.])
				AC_DEFINE(SOUND, 1, [Define to 1 if including sound support.])
				LIBS="${LIBS} -lSDL_mixer"
				MAINFILES="${MAINFILES} \$(SNDSDLFILES)"])])
		AS_IF([test "$with_sdl" = "no" && test "$found_sdl_mixer" = "no"],
			[dnl Not using SDL so restore flags.
			CPPFLAGS="${hold_CPPFLAGS}"
			LIB="${hold_LIBS}"])])])
ENABLESDL="$with_sdl"; AC_SUBST(ENABLESDL)

dnl Test checking
AS_IF([test "$enable_test" = "yes"],
	[AC_DEFINE(USE_TEST, 1, [Define to 1 to build the test frontend])
	MAINFILES="${MAINFILES} \$(TESTMAINFILES)"])

dnl Stats checking
LDFLAGS_SAVE="$LDFLAGS"
AS_IF([test "$enable_stats" = "yes"],
	[# SQLite3 detection
	SQLITE3_OK=yes
	AC_CHECK_HEADER(sqlite3.h, [], [
		SQLITE3_OK=missing
		for sqlite3_path in $SEARCH_DIR_HEADERS; do
			AS_IF([test "x$ac_cv_header_sqlite3_h" != xyes],
				[unset ac_cv_header_sqlite3_h
				AC_CHECK_HEADER($sqlite3_path/sqlite3.h,
					[
						SQLITE3_CFLAGS="-I$sqlite3_path"
						SQLITE3_OK=yes
					]
				)])
		done
	])
	AS_IF([test "x$SQLITE3_OK" = xyes],
		[AC_CHECK_LIB(sqlite3, sqlite3_open, [
			SQLITE3_LIBS="-lsqlite3"
		], [
			SQLITE3_OK=missing
			for sqlite3_path in $SEARCH_DIR_LIBS; do
				AS_IF([test "x$ac_cv_lib_sqlite3_sqlite3_open" != xyes],
					[unset ac_cv_lib_sqlite3_sqlite3_open
					LDFLAGS="${LDFLAGS_SAVE} -L$sqlite3_path"
					AC_CHECK_LIB(sqlite3, sqlite3_open, [
						SQLITE3_LDFLAGS="-L$sqlite3_path"
						SQLITE3_LIBS="-lsqlite3"
						SQLITE3_OK=yes
					])])
			done	
		])])
	AS_IF([test "x$SQLITE3_OK" = xyes],
		[AC_DEFINE(USE_STATS, 1, [Define to 1 to build the stats frontend])
		CPPFLAGS="${CPPFLAGS} ${SQLITE3_CFLAGS}"
		LDFLAGS="${LDFLAGS_SAVE} ${SQLITE3_LDFLAGS}"
		LIBS="${LIBS} ${SQLITE3_LIBS} -lm"
		MAINFILES="${MAINFILES} \$(STATSMAINFILES)"
		AC_SUBST(USE_STATS, 1)],
		[AC_MSG_ERROR(Could not find sqlite3 library; disabling stats)
		enable_stats=no
		AC_DEFINE(USE_STATS, 0, [Define to 0 to omit the stats frontend])
		LDFLAGS="$LDFLAGS_SAVE"
		AC_SUBST(USE_STATS, 0)])])

dnl Spoiler checking
AS_IF([test "$enable_spoil" = "yes"],
	[AC_DEFINE(USE_SPOIL, 1, [Define to 1 to build the command-line spoiler generation])
	MAINFILES="${MAINFILES} \$(SPOILMAINFILES)"])

dnl Windows checking
AS_IF([test "$enable_win" = "yes"],
	[AS_IF([test x"$with_no_install" != x || test x"$with_setgid" != x],
		[AC_MSG_ERROR([--enable-win is not compatible with --with-no-install or --with-setgid])])
	AC_DEFINE(USE_WIN, 1, [Define to 1 if using the Windows interface.])
	AC_DEFINE(SOUND, 1, [Define to 1 if including sound support.])
	CPPFLAGS="${CPPFLAGS} -DWINDOWS -Iwin/include"
	CFLAGS="${CFLAGS} -static"
	LDFLAGS="${LDFLAGS} -Lwin/lib"
	LIBS="${LIBS} -mwindows -lwinmm -lzlib -llibpng -lmsimg32"
	TEST_LIBS="${TEST_LIBS} -mwindows"
	dnl Note complete replacement of all main files
	MAINFILES="\$(WINMAINFILES)"])
# Set up for Windows-specific behavior in the Makefiles.
ENABLEWIN="$enable_win"; AC_SUBST(ENABLEWIN)

dnl Remember if we are cross compiling.  Currently used so "make tests" only
dnl compiles but does not run test cases when cross compiling.
AS_IF([test x"$build_alias" = x"$host_alias"],
	[CROSS_COMPILE=no],
	[CROSS_COMPILE=yes])
AC_SUBST(CROSS_COMPILE)

dnl If installing elsewhere or using the Windows front end, allow the test
dnl cases which use set_file_paths() to find the data files.  This relies on
dnl the setting for TEST_DEFAULT_PATH in src/tests/unit-test.h which uses a
dnl relative path and has the drawback that a test case will not work if it
dnl uses set_file_paths() and the working directory is not the top level
dnl directory of a distribution.  Also set TEST_WORKING_DIRECTORY so, that
dnl when run by src/tests/run-tests, the working directory will match that.
AS_IF([test x"$with_no_install" = x || test x"$enable_win" = xyes],
	[CFLAGS="${CFLAGS} -DTEST_OVERRIDE_PATHS"
	AS_CASE(["$srcdir"],
		[[.]], [TEST_WORKING_DIRECTORY="$PWD"],
		[[\\/]* | ?:[\\/]*], [TEST_WORKING_DIRECTORY="$srcdir"],
		[TEST_WORKING_DIRECTORY="$PWD/$srcdir"])],
	[TEST_WORKING_DIRECTORY=])
AC_SUBST(TEST_WORKING_DIRECTORY)

AC_SUBST(MAINFILES, ${MAINFILES})
AC_SUBST(TEST_LIBS, ${TEST_LIBS})
AC_CONFIG_FILES([mk/buildsys.mk mk/extra.mk])
AC_OUTPUT

AS_IF([test "x$with_private_dirs" = "xyes"],
	[displayedvardatadir="(not used)"],
	[displayedvardatadir="$vardatadir"])

AS_IF([test "x$with_no_install" = "xyes" || test "x$enable_win" = "xyes"],
	[displayedprefix="(not used)"
	displayedbindir="(not used)"
	displayeddocdir="(not used)"],
	[displayedprefix="${prefix}"
	displayedbindir="${bindir}"
	displayeddocdir="${docdatadir}"])

AS_IF([test "x$enable_win" = "xyes"],
	[displayedconfigdir="./lib"
	displayedlibdatadir="./lib"],
	[displayedconfigdir="${configdir}"
	displayedlibdatadir="${libdatadir}"])

echo
echo "Configuration:"
echo
echo "  Install path:                           ${displayedprefix}"
echo "  binary path:                            ${displayedbindir}"
echo "  config path:                            ${displayedconfigdir}"
echo "  lib path:                               ${displayedlibdatadir}"
echo "  doc path:                               ${displayeddocdir}"
echo "  var path:                               ${displayedvardatadir}"
AS_IF([test "x$with_gamedata_in_lib" = "xyes"],
	[echo "  gamedata path:                          ${displayedlibdatadir}"],
	[echo "  gamedata path:                          ${displayedconfigdir}"])
AS_IF([test "x$wsetgid" = "xyes"],
	[echo "  (as group ${SETEGID})"],
	[AS_IF([test "x$with_private_dirs" = "xyes" && test ! "x$enable_win" = "xyes"],
		[echo "  (with private save and score files in ~/.angband/Angband/)"])])
AS_IF([test "x$with_sphinx" = "xyes"],
	[echo "  documentation:                          Yes"],
	[echo "  documentation:                          No"])

echo
echo "-- Frontends --"
local_player_frontends=""
AS_IF([test "$enable_curses" = "yes"],
	[AS_IF([test "$with_curses" = "no"],
		[echo "- Curses                                  No; missing libraries"],
		[echo "- Curses                                  Yes"
		local_player_frontends="$local_player_frontends"+])],
	[echo "- Curses                                  Disabled"])
AS_IF([test "$enable_x11" = "yes"],
	[AS_IF([test "$with_x11" = "no"],
		[echo "- X11                                     No; missing libraries"],
		[echo "- X11                                     Yes"
		local_player_frontends="$local_player_frontends"+])],
	[echo "- X11                                     Disabled"])
AS_IF([test "$enable_sdl2" = "yes"],
	[AS_IF([test "$with_sdl2" = "no"],
		[echo "- SDL2                                    No; missing libraries"],
		[echo "- SDL2                                    Yes"
		local_player_frontends="$local_player_frontends"+])],
	[echo "- SDL2                                    Disabled"])
AS_IF([test "$enable_sdl" = "yes"],
	[AS_IF([test "$with_sdl" = "no"],
		[echo "- SDL                                     No; missing libraries"],
		[echo "- SDL                                     Yes"
		local_player_frontends="$local_player_frontends"+])],
	[echo "- SDL                                     Disabled"])

AS_IF([test "$enable_win" = "yes"],
	[echo "- Windows                                 Yes"
	local_player_frontends="$local_player_frontends"+],
	[echo "- Windows                                 Disabled"])

AS_IF([test "$enable_test" = "yes"],
	[echo "- Test                                    Yes"],
	[echo "- Test                                    No"])

AS_IF([test "$enable_stats" = "yes"],
	[echo "- Stats                                   Yes"],
	[echo "- Stats                                   No"])

AS_IF([test "$enable_spoil" = "yes"],
	[echo "- Spoilers                                Yes"],
	[echo "- Spoilers                                No"])

echo

AS_IF([test "$enable_sdl2_mixer" = "yes"],
	[AS_IF([test "$found_sdl2_mixer" = "no"],
		[echo "- SDL2 sound                              No; missing libraries"],
		[echo "- SDL2 sound                              Yes"])],
	[echo "- SDL2 sound                              Disabled"])
AS_IF([test "$enable_sdl_mixer" = "yes"],
	[AS_IF([test "$found_sdl_mixer" = "no"],
		[echo "- SDL sound                               No; missing libraries"],
		[echo "- SDL sound                               Yes"])],
	[echo "- SDL sound                               Disabled"])

AS_IF([test x"$local_player_frontends" = "x"],
	[AC_MSG_WARN([No player frontends are enabled.  Check your --enable options and prerequisites for the frontends.])])
