dnl Process this file with autoconf to produce a configure script.
AC_INIT([Angband], [3.0.9], [bugs@rephial.org], [angband])
AC_PREREQ([2.50])

AC_CONFIG_HEADER(src/autoconf.h)

dnl OMK bootstrap
AC_SUBST([PACKAGE], [AC_PACKAGE_TARNAME])
AC_SUBST([VERSION], [AC_PACKAGE_VERSION])

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package])

AC_ARG_WITH(setgid,       [  --with-setgid=NAME      install angband as group NAME])
AC_ARG_WITH(private_dirs, [  --with-private-dirs     use private scorefiles/savefiles/datafiles],
            [], [with_private_dirs=yes])
AC_ARG_WITH(noinstall,    [  --with-noinstall        don't install globally at all])

if test "x$with_setgid" != "xyes"; then
	if test "x$with_setgid" != "x"; then
		wsetgid=yes
		with_private_dirs=
	fi
fi
if test "x$with_noinstall" == "xyes"; then
	with_private_dirs=
fi
#if test "x$with_segtid" == "xyes"; then
#	with_private_dirs=
#fi
if test "x$wsetgid$with_private_dirs$with_noinstall" != "xyes";  then

	echo "Please run ./configure with one of:

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
    you do this, and should be used for most installations of the game.

--with-noinstall
    This configures the game to store savefiles and scorefiles in the "lib"
    directory that the game uses.  This is useful if you don't want to install
    system-wide at all; \"make install\" will just move the executable to the
    directory above \"src\" if you choose this option.  This option is
    recommended for developers.
"

	AC_MSG_ERROR([Please specify an installation method.])
fi


dnl Check for C compiler
AC_LANG([C])
AC_PROG_CC

test "$GCC" = "yes" && CFLAGS="$CFLAGS -fno-strength-reduce -Wall"

dnl Checks for various programs
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_PATH_PROG(RM, rm)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)

dnl Work around an autoconf bug.
if test "$prefix" = "NONE"; then
	prefix="${ac_default_prefix}"
fi



if test "x$with_private_dirs" != "x"; then
	AC_DEFINE(USE_PRIVATE_PATHS, 1, [Define to use private user paths.])
fi

AC_ARG_WITH(libpath, [  --with-libpath=PATH     use PATH as the path to the Angband lib folder])

if test "$with_libpath" != ""; then
	libpath="${with_libpath}"
elif test "$with_noinstall" != ""; then
	libpath="./lib/"
	bindir=".."
else
	libpath="${prefix}/share/${PACKAGE}/"
fi

case "/$libpath" in
	*/) MY_EXPAND_DIR(game_datadir, "$libpath")  ;;
	*)  MY_EXPAND_DIR(game_datadir, "$libpath/") ;;
esac

AC_DEFINE_UNQUOTED([DEFAULT_PATH], "${game_datadir}", [Path to the game's lib directory])

NOINSTALL="$with_noinstall"; AC_SUBST(NOINSTALL)
SETEGID="$with_setgid";      AC_SUBST(SETEGID)
DATA_PATH="$game_datadir";   AC_SUBST(DATA_PATH)

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



dnl Check for headers, types, functions
AC_CHECK_HEADERS([fcntl.h stdint.h dirent.h])
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([setresgid setegid can_change_color stat])



AC_CONFIG_FILES([mk/rules.mk])
AC_OUTPUT


echo
echo "Configuration:"
echo
echo "  Install path:                           ${prefix}"
echo "  lib/ path:                              ${game_datadir}"
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
