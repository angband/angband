dnl Process this file with autoconf to produce a configure script.
AC_INIT([Angband], [3.2.0], [bugs@rephial.org], [angband])
AC_PREREQ([2.59])

AC_CONFIG_HEADER(src/autoconf.h)
AC_CONFIG_SRCDIR([configure.ac])

AC_SUBST([PACKAGE], [AC_PACKAGE_TARNAME])
AC_SUBST([VERSION], [AC_PACKAGE_VERSION])

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package])

AC_ARG_WITH(setgid,
	[  --with-setgid=NAME      install angband as group NAME],
	[wsetgid=yes])
AC_ARG_WITH(private_dirs,
	[  --with-private-dirs     use private scorefiles/savefiles])
AC_ARG_WITH(no_install,
	[  --with-no-install       don't install, just run in-place])

if test "x$with_setgid" = "xyes"; then
	AC_MSG_ERROR([Please specify a group to install as.])
fi

if test "x$wsetgid" = "x" && test "x$with_no_install" = "x"; then
	with_private_dirs="yes"
fi

if test "x$wsetgid$with_private_dirs$with_no_install" != "xyes";  then

	echo "Please run ./configure with only one of:

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

	AC_MSG_ERROR([Please specify an installation method.])
fi

AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_MKDIR_P

AC_PATH_PROG(RM, rm)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)

AC_HEADER_DIRENT
AC_CHECK_HEADERS([fcntl.h stdint.h])
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([mkdir setresgid setegid stat])

dnl needed because h-basic.h checks for this define for autoconf support.
CFLAGS="$CFLAGS -DHAVE_CONFIG_H"
CPPFLAGS="$CPPFLAGS -I." 

test "$GCC" = "yes" && CFLAGS="$CFLAGS -fno-strength-reduce -W -Wall -Wno-unused-parameter -Wno-missing-field-initializers -pedantic"

MY_PROG_MAKE_SYSVINC
MY_PROG_MAKE_SINCLUDE

dnl Work around an autoconf bug.
if test "$prefix" = "NONE"; then
	prefix="${ac_default_prefix}"
fi
if test "$exec_prefix" = "NONE"; then
	exec_prefix="${prefix}"
fi

if test "x$with_private_dirs" != "x"; then
	AC_DEFINE(USE_PRIVATE_PATHS, 1, [Define to use private save and score paths.])
fi

# Only change bindir if it's the configure-supplied default, which handily doesn't expand exec_prefix
if test "x$bindir" = "x\${exec_prefix}/bin"; then
  bindir=${exec_prefix}/games
fi

AC_ARG_WITH(configpath, [  --with-configpath=PATH  use PATH as the path to the Angband configuration folder])

if test "x$with_configpath" != "x"; then
	configpath="${with_configpath}"
elif test "x$with_no_install" != "x"; then
	configpath="${PWD}/lib/"
else
	configpath="${prefix}/etc/${PACKAGE}/"
fi

case "/$configpath" in
	*/) MY_EXPAND_DIR(configdir, "$configpath")  ;;
	*)  MY_EXPAND_DIR(configdir, "$configpath/") ;;
esac

AC_ARG_WITH(libpath, [  --with-libpath=PATH     use PATH as the path to the Angband lib folder])

if test "x$with_libpath" != "x"; then
	libpath="${with_libpath}"
elif test "x$with_no_install" != "x"; then
	libpath="${PWD}/lib/"
	bindir=".."
else
	libpath="${prefix}/share/${PACKAGE}/"
fi

case "/$libpath" in
	*/) MY_EXPAND_DIR(libdatadir, "$libpath")  ;;
	*)  MY_EXPAND_DIR(libdatadir, "$libpath/") ;;
esac

AC_ARG_WITH(varpath, [  --with-varpath=PATH     use PATH as the path to the Angband saved data folder])

if test "x$with_varpath" != "x"; then
	varpath="${with_varpath}"
elif test "x$with_no_install" != "x"; then
	varpath="${PWD}/lib/"
else
	varpath="${prefix}/var/games/${PACKAGE}/"
fi

case "/$varpath" in
	*/) MY_EXPAND_DIR(vardatadir, "$varpath")  ;;
	*)  MY_EXPAND_DIR(vardatadir, "$varpath/") ;;
esac

AC_SUBST([configdir])
AC_SUBST([libdatadir])
AC_SUBST([vardatadir])

AC_DEFINE_UNQUOTED([DEFAULT_CONFIG_PATH], "${configdir}", [Path to the game's configuration data])
AC_DEFINE_UNQUOTED([DEFAULT_LIB_PATH], "${libdatadir}", [Path to the game's lib directory])
AC_DEFINE_UNQUOTED([DEFAULT_DATA_PATH], "${vardatadir}", [Path to the game's variable data])

NOINSTALL="$with_noinstall"; AC_SUBST(NOINSTALL)
SETEGID="$with_setgid";      AC_SUBST(SETEGID)

dnl Frontends
AC_ARG_ENABLE(gtk,
	[AS_HELP_STRING([--enable-gtk],       [Enables GTK 2.x frontend (default: disabled)])],
	[enable_gtk=$enableval],
	[enable_gtk=no])
AC_ARG_ENABLE(curses,
	[AS_HELP_STRING([--enable-curses],    [Enables Curses frontend (default: enabled)])],
	[enable_curses=$enableval],
	[enable_curses=yes])
AC_ARG_ENABLE(x11,
	[AS_HELP_STRING([--enable-x11],       [Enables X11 frontend (default: enabled)])],
	[enable_x11=$enableval],
	[enable_x11=yes])
AC_ARG_ENABLE(sdl,
	[AS_HELP_STRING([--enable-sdl],       [Enables SDL frontend (default: disabled)])],
	[enable_sdl=$enableval],
	[enable_sdl=no])
AC_ARG_ENABLE(test,
	[AS_HELP_STRING([--enable-test],      [Enables test frontend (default: disabled)])],
	[enable_test=$enableval],
	[enable_test=no])
AC_ARG_ENABLE(stats,
	[AS_HELP_STRING([--enable-stats],     [Enables stats frontend (default: disabled)])],
	[enable_stats=$enableval],
	[enable_stats=no])

dnl Sound modules
AC_ARG_ENABLE(sdl_mixer,
	[AS_HELP_STRING([--enable-sdl-mixer], [Enables SDL mixer sound support (default: enabled)])],
	[enable_sdl_mixer=$enable_sdl_mixer],
	[enable_sdl_mixer=yes])


if test "x$enable_gtk" = "xyes"; then
	if test "x$wsetgid" = "xyes"; then
		AC_MSG_ERROR([The "setgid" option and the GTK port are incompatible.])
	fi
fi


dnl curses checking
if test "$enable_curses" = "yes"; then
	AC_CHECK_LIB(curses  , initscr, found_curses=yes,found_curses=no)
	AC_CHECK_LIB(ncurses , initscr, found_ncurses=yes,found_ncurses=no)
	AC_CHECK_LIB(pdcurses, initscr, found_pdcurses=yes,found_pdcurses=no)
	if test "x$found_ncurses" = "xno"; then
		if test "x$found_pdcurses" = "xno"; then
			if test "x$found_curses" = "xno"; then
				with_curses=no
			else
				AC_DEFINE(USE_CURSES, 1, [Define to 1 if Curses or PDCurses is found, but NCurses is not.])
				AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
				LDFLAGS="${LDFLAGS} -lcurses"
				with_curses=yes
			fi
		else
			AC_DEFINE(USE_CURSES, 1, [Define to 1 if Curses or PDCurses is found, but NCurses is not.])
			AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
			LDFLAGS="${LDFLAGS} -lpdcurses"
			with_curses=yes
		fi
	else
		AC_DEFINE(USE_NCURSES, 1, [Define to 1 if NCurses is found.])
		AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
		LDFLAGS="${LDFLAGS} -lncurses"
		with_curses=yes
	fi
fi

AC_CHECK_FUNCS([use_default_colors can_change_color])

dnl X11 checking
if test "$enable_x11" = "yes"; then
	AC_PATH_XTRA

	if test "x$have_x" != "xyes"; then
		with_x11=no
	else
		AC_DEFINE(USE_X11, 1, [Define to 1 if using the X11 frontend and X11 libraries are found.])
		CFLAGS="$CFLAGS $X_CFLAGS"
		LDFLAGS="$LDFLAGS $X_PRE_LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"
		with_x11=yes
	fi
fi

dnl SDL checking
if test "$enable_sdl" = "yes"; then
	AM_PATH_SDL(1.2.10,,)

	if test "$SDL_CONFIG" = "no"; then
		with_sdl=no
	else
		AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, with_sdl=yes, with_sdl=no)
		AC_CHECK_LIB(SDL_ttf, TTF_Init, with_sdlx=yes, with_sdl=no)

		if test "$with_sdl" = "yes"; then
			AC_DEFINE(USE_SDL, 1, [Define to 1 if using the SDL interface and SDL is found.])
			CFLAGS="${CFLAGS} ${SDL_CFLAGS}"
			LDFLAGS="${LDFLAGS} ${SDL_LIBS} -lSDL_image -lSDL_ttf"
		fi
	fi
fi


dnl GTK checking
if test "$enable_gtk" = "yes"; then
	PKG_CHECK_MODULES(GTK, [glib-2.0 >= 2.6.0 gtk+-2.0 >= 2.6.0 gthread-2.0 pango libglade-2.0],
		[with_gtk=yes],
		[with_gtk=no]
	)
	PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.6.0],
		[with_gtkx=yes],
		[with_gtk=no]
	)

	if test "x$SETEGID" != "x"; then
		enable_gtk=no
		with_gtk=no
	fi

	if test "$with_gtk" = "yes"; then
		AC_DEFINE(USE_GTK, 1, [Define to 1 if using the GTK+ 2.x interface and GTK+ 2.x is found.])
		CFLAGS="${CFLAGS} $GTK_CFLAGS"
		LDFLAGS="${LDFLAGS} $GTK_LIBS -rdynamic -export-dynamic"
	fi
fi


dnl SDL mixer checking
if test "$enable_sdl_mixer" = "yes"; then
	AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio, found_sdl_mixer=yes, found_sdl_mixer=no)

	if test "$found_sdl_mixer" = "yes"; then
		AC_DEFINE(SOUND_SDL, 1, [Define to 1 if using SDL_mixer sound support and it's found.])
		SDL_CFLAGS=`sdl-config --cflags`
		CFLAGS="${CFLAGS} ${SDL_CFLAGS}"
		LDFLAGS="${LDFLAGS} -lSDL_mixer"
	fi
fi


LDFLAGS="${LDFLAGS} -lm"


dnl Test checking
if test "$enable_test" = "yes"; then
	AC_DEFINE(USE_TEST, 1, [Define to 1 to build the test frontend])
fi

dnl Stats checking

LDFLAGS_SAVE="$LDFLAGS"
if test "$enable_stats" = "yes"; then
	# SQLite3 detection
	SQLITE3_OK=yes
	AC_CHECK_HEADER(sqlite3.h, [], [
		SQLITE3_OK=missing
		for sqlite3_path in $SEARCH_DIR_HEADERS; do
			if test "x$ac_cv_header_sqlite3_h" != xyes; then
				unset ac_cv_header_sqlite3_h
				AC_CHECK_HEADER($sqlite3_path/sqlite3.h,
					[
						SQLITE3_CFLAGS="-I$sqlite3_path"
						SQLITE3_OK=yes
					]
				)
			fi
		done
	])
	if test "x$SQLITE3_OK" = xyes; then
		AC_CHECK_LIB(sqlite3, sqlite3_open, [
			SQLITE3_LDFLAGS="-lsqlite3"
		], [
			SQLITE3_OK=missing
			for sqlite3_path in $SEARCH_DIR_LIBS; do
				if test "x$ac_cv_lib_sqlite3_sqlite3_open" != xyes; then
					unset ac_cv_lib_sqlite3_sqlite3_open
					LDFLAGS="$LDFLAGS_SAVE -L$sqlite3_path"
					AC_CHECK_LIB(sqlite3, sqlite3_open, [
						SQLITE3_LDFLAGS="-L$sqlite3_path -lsqlite3"
						SQLITE3_OK=yes
					])
				fi
			done	
		])
	fi
	if test "x$SQLITE3_OK" = xyes; then
		AC_DEFINE(USE_STATS, 1, [Define to 1 to build the stats frontend])
		CFLAGS="${CFLAGS} ${SQLITE3_CFLAGS}"
		LDFLAGS="${LDFLAGS_SAVE} ${SQLITE3_LDFLAGS}"
		AC_SUBST(USE_STATS, 1)
	else
		AC_MSG_ERROR(Could not find sqlite3 library; disabling stats)
		enable_stats=no
		AC_DEFINE(USE_STATS, 0, [Define to 0 to omit the stats frontend])
		LDFLAGS="$LDFLAGS_SAVE"
		AC_SUBST(USE_STATS, 0)
	fi
fi


AC_CONFIG_FILES([mk/extra.mk mk/sinc@&t@lude.mk])
AC_OUTPUT

if test "x$with_private_dirs" = "xyes"; then
	displayedvardatadir="(not used)"
else
	displayedvardatadir="$vardatadir"
fi

if test "x$with_no_install" = "xyes"; then
	displayedprefix="(not used)"
else
	displayedprefix="$prefix"
fi

echo
echo "Configuration:"
echo
echo "  Install path:                           ${displayedprefix}"
echo "  binary path:                            ${bindir}"
echo "  config path:                            ${configdir}"
echo "  lib path:                               ${libdatadir}"
echo "  var path:                               ${displayedvardatadir}"

if test "x$wsetgid" = "xyes"; then
	echo "  (as group ${SETEGID})"
elif test "x$with_private_dirs" = "xyes"; then
	echo "  (with private save and score files in ~/.angband/Angband/)"
fi

echo
echo "-- Frontends --"
if test "$enable_curses" = "yes"; then
	if test "$with_curses" = "no"; then
		echo "- Curses                                  No; missing libraries"
	else
		echo "- Curses                                  Yes"
	fi
else
	echo "- Curses                                  Disabled"
fi
if test "$enable_gtk" = "yes"; then
	if test "$with_gtk" = "no"; then
		echo "- GTK 2.x                                 No; missing libraries"
	else
		echo "- GTK 2.x                                 Yes"
	fi
else
	echo "- GTK 2.x                                 Disabled"
fi
if test "$enable_x11" = "yes"; then
	if test "$with_x11" = "no"; then
		echo "- X11                                     No; missing libraries"
	else
		echo "- X11                                     Yes"
	fi
else
	echo "- X11                                     Disabled"
fi
if test "$enable_sdl" = "yes"; then
	if test "$with_sdl" = "no"; then
		echo "- SDL                                     No; missing libraries"
	else
		echo "- SDL                                     Yes"
	fi
else
	echo "- SDL                                     Disabled"
fi

if test "$enable_test" = "yes"; then
	echo "- Test                                    Yes"
else
    echo "- Test                                    No"
fi

if test "$enable_stats" = "yes"; then
	echo "- Stats                                   Yes"
else
    echo "- Stats                                   No"
fi

echo

if test "$enable_sdl_mixer" = "yes"; then
	if test "$found_sdl_mixer" = "no"; then
		echo "- SDL sound                               No; missing libraries"
	else
		echo "- SDL sound                               Yes"
	fi
else
	echo "- SDL sound                               Disabled"
fi
