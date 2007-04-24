dnl Process this file with autoconf to produce a configure script.
AC_INIT([angband], [3.0.8], [http://angband.rogueforge.net/trac])
AC_PREREQ([2.50])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_HEADER(src/autoconf.h)

dnl OMK bootstrap
AC_SUBST([PACKAGE], [AC_PACKAGE_TARNAME])
AC_SUBST([VERSION], [AC_PACKAGE_VERSION])

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package])

dnl Check for C compiler
AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_C_BIGENDIAN

test "$GCC" = "yes" && CFLAGS="$CFLAGS -Wall -pipe -g -fno-strength-reduce"
PICFLAGS="-fPIC"
AC_SUBST(PICFLAGS)

dnl Checks for various programs
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PATH_PROG(RM, rm)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(RANLIB, ranlib)

AC_ARG_WITH(setgid,
[  --with-setgid=NAME      install angband as group NAME],
[case "${withval}" in
  no) SETEGID="games";;
  yes) AC_MSG_ERROR(missing argument for --with-setgid);;
  *) SETEGID="$withval";;
esac])
AC_SUBST(SETEGID)

AC_LANG([C])

dnl generate the installation path for the ./lib/ folder
if test "$GAMEGROUP" != ""; then
  MY_EXPAND_DIR(game_datadir, "$datadir/$PACKAGE/")
else
  MY_EXPAND_DIR(game_datadir, "./lib/")
  bindir=".."
fi

dnl overwrite the path with an user-specified value
AC_ARG_WITH(libpath,
[  --with-libpath=path     specify the path to the Angband lib folder],

[case "/$withval" in
  */) game_datadir="$withval"  ;;
  *)  game_datadir="$withval/" ;;
esac])

AC_DEFINE_UNQUOTED([DEFAULT_PATH], "$game_datadir",
                   [Path to the game's lib directory])
DATA_PATH="$game_datadir"
AC_SUBST(DATA_PATH)

dnl Frontends
AC_ARG_ENABLE(gtk,
	[  --enable-gtk                         Enables GTK 2.x frontend (default: disabled)],
	[enable_gtk=$enableval],
	[enable_gtk=no])
AC_ARG_ENABLE(curses,
	[  --enable-curses                      Enables Curses frontend (default: enabled)],
	[enable_curses=$enableval],
	[enable_curses=yes])
AC_ARG_ENABLE(x11,
	[  --enable-x11                         Enables X11 frontend (default: disabled)],
	[enable_x11=$enableval],
	[enable_x11=no])
AC_ARG_ENABLE(sdl,
	[  --enable-sdl                         Enables SDL frontend (default: disabled)],
	[enable_sdl=$enableval],
	[enable_sdl=no])

dnl curses checking
if test "$enable_curses" = "yes"; then
	AC_CHECK_LIB(curses  , initscr, found_curses=yes,found_curses=no)
	AC_CHECK_LIB(ncurses , initscr, found_ncurses=yes,found_ncurses=no)
	AC_CHECK_LIB(pdcurses, initscr, found_pdcurses=yes,found_pdcurses=no)
	if test "$found_ncurses" = "no"; then
		if test "$found_pdcurses" = "no"; then
			if test "$found_curses" = "no"; then
				AC_DEFINE(USE_NCURSES, 0, [Define to 1 if NCurses is found.])
				AC_DEFINE(USE_CURSES, 0, [Define to 1 if Curses or PDCurses is found, but NCurses is not.])
				AC_DEFINE(USE_GCU, 0, [Define to 1 if using the Curses frontend.])
			else
				AC_DEFINE(USE_NCURSES, 0, [Define to 1 if NCurses is found.])
				AC_DEFINE(USE_CURSES, 1, [Define to 1 if Curses or PDCurses is found, but NCurses is not.])
				AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
				CFFLAGS+=" -I/usr/include/curses"
				LDFLAGS+=" -lcurses"
				with_curses=yes
			fi
		else
			AC_DEFINE(USE_NCURSES, 0, [Define to 1 if NCurses is found.])
			AC_DEFINE(USE_CURSES, 1, [Define to 1 if Curses or PDCurses is found, but NCurses is not.])
			AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
			LDFLAGS+=" -lpdcurses"
			with_curses=yes
		fi
	else
		AC_DEFINE(USE_CURSES, 0, [Define to 1 if Curses or PDCurses is found, but NCurses is not.])
		AC_DEFINE(USE_NCURSES, 1, [Define to 1 if NCurses is found.])
		AC_DEFINE(USE_GCU, 1, [Define to 1 if using the Curses frontend.])
		LDFLAGS+=" -lncurses"
		with_curses=yes
	fi
fi

dnl X11 checking
if test "$enable_x11" = "yes"; then
	AC_PATH_X
	if test "$x_libraries" = "no"; then
		AC_DEFINE(USE_X11, 0, [Define to 1 if using the X11 frontend and X11 libraries are found.])
		with_x11=no
	else
		AC_DEFINE(USE_X11, 1, [Define to 1 if using the X11 frontend and X11 libraries are found.])
		CFLAGS+=" -I/usr/X11R6/include -I/usr/include"
		LDFLAGS+=" $x_libraries"
		with_x11=yes
	fi
fi

dnl SDL checking
if test "$enable_sdl" = "yes"; then
	AM_PATH_SDL(1.2.0,,)
	if test "$SDL_CONFIG" = "no"; then
		with_sdl=no
		AC_DEFINE(USE_SDL, 0, [Define to 1 if using the SDL interface and SDL & SDL_ttf are found.])
	else
		AC_DEFINE(USE_SDL, 1, [Define to 1 if using the SDL interface and SDL & SDL_ttf are found.])
		CFLAGS+=" $SDL_CFLAGS"
		LDFLAGS+=" $SDL_LIBS"
		with_sdl=yes
	fi
fi

dnl GTK checking
if test "$enable_gtk" = "yes"; then
	PKG_CHECK_MODULES(GTK, [glib-2.0 >= 2.6.0 gtk+-2.0 >= 2.6.0 gthread-2.0 pango],
		[with_gtk=yes],
		[with_gtk=no]
	)
	PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.6.0],
		[with_get=yes],
		[with_gtk=no]
	)
	if test "$with_gtk" = "yes"; then
		AC_DEFINE(USE_GTK, 1, [Define to 1 if using the GTK+ 2.x interface and GTK+ 2.x is found.])
		CFLAGS+=" $GTK_CFLAGS"
		LDFLAGS+=" $GTK_LIBS"
	else
		AC_DEFINE(USE_GTK, 0, [Define to 1 if using the GTK+ 2.x interface and GTK+ 2.x is found.])
	fi
fi

dnl Check for headers.
dnl AC_HEADER_STDBOOL -- not yet
AC_HEADER_TIME
AC_CHECK_HEADERS([unistd.h fcntl.h limits.h stdint.h sys/time.h termios.h])

dnl Check for types.
AC_TYPE_SIGNAL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

dnl Check for structs.
AC_STRUCT_TM

dnl Check for functions.
AC_FUNC_STAT
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([memmove memset strrchr mkdir strtol usleep mkstemp setegid can_change_color])

dnl XXX: Work around some autoconf bugs.
if test "$prefix" = "NONE"; then
        prefix="${ac_default_prefix}"
fi
if test -z "$datadir"; then
	datadir="${prefix}/share"
fi


AC_CONFIG_FILES([
	mk/rules.mk
])

AC_OUTPUT


echo
echo "Configuration:"
echo
echo "  Install path:                           ${prefix}"
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
