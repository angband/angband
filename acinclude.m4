# generated automatically by aclocal 1.10 -*- Autoconf -*-

# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
# 2005, 2006  Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

AC_DEFUN([_MY_PROG_MAKE_VAR],
	 [AC_ARG_VAR([MAKE], [Name of a make program to use])dnl
my_make=${MAKE-make}
])
AC_DEFUN([MY_PROG_MAKE_SYSVINC], [
AC_REQUIRE([_MY_PROG_MAKE_VAR])dnl

AC_CACHE_CHECK([if $my_make supports SysV-style inclusion],
	[my_cv_make_inclusion_sysv],
	[my_cv_make_inclusion_sysv=no
	cat >conftest.inc <<EOF
conftest.foo:
	@echo baz
EOF
	cat >conftest.mk <<EOF
inc@&t@lude conftest.inc
EOF
	if test x"`$my_make -f conftest.mk conftest.foo | grep baz`" = x"baz"; then
		my_cv_make_inclusion_sysv=yes
	else
		my_cv_make_inclusion_sysv=no
	fi
	rm -f conftest.inc conftest.mk])

if test x$my_cv_make_inclusion_sysv = xno ; then
	AC_MSG_ERROR([A 'make' supporting SysV file inclusion is required.])
fi])

AC_DEFUN([MY_PROG_MAKE_SINCLUDE], [
AC_REQUIRE([_MY_PROG_MAKE_VAR])dnl
AC_CACHE_CHECK([for $my_make silent inc@&t@lude syntax],
	[my_cv_make_sinc@&t@lude_syntax],
	[my_cv_make_sinc@&t@lude_syntax=none
	if test x$my_cv_make_sinc@&t@lude_syntax = xnone ; then
		cat >conftest.mk <<EOF
-inc[]lude "conftest.inc"
conftest.foo:
EOF
		if $my_make -f conftest.mk conftest.foo &>/dev/null ; then
			my_cv_make_sinc@&t@lude_syntax=gnu
		fi
		rm conftest.mk
	fi
	if test x$my_cv_make_sinc@&t@lude_syntax = xnone ; then
		cat >conftest.mk <<EOF
.sinc[]lude "conftest.inc"
conftest.foo:
EOF
		if $my_make -f conftest.mk conftest.foo &>/dev/null ; then
			my_cv_make_sinc@&t@lude_syntax=bsd
		fi
		rm conftest.mk
	fi])

AC_SUBST([MAKE_SINCLUDE])
if test x$my_cv_make_sinc@&t@lude_syntax = xgnu ; then
	MAKE_SINCLUDE=-inc@&t@lude
else
	if test x$my_cv_make_sinc@&t@lude_syntax = xbsd ; then
		MAKE_SINCLUDE=[.sinc@&t@lude]
	else
		AC_MSG_ERROR([$my_make does not support a supported silent inc@&t@lude syntax])
	fi
fi])

AC_DEFUN([MY_EXPAND_DIR],
	[$1=$2
	$1=`(
		test "x$prefix" = xNONE && prefix="$ac_default_prefix"
		test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
		eval echo \""[$]$1"\"
	)`]
)


dnl PKG_CHECK_MODULES(GSTUFF, gtk+-2.0 >= 1.3 glib = 1.3.4, action-if, action-not)
dnl defines GSTUFF_LIBS, GSTUFF_CFLAGS, see pkg-config man page
dnl also defines GSTUFF_PKG_ERRORS on error
AC_DEFUN([PKG_CHECK_MODULES], [
  succeeded=no

  if test -z "$PKG_CONFIG"; then
    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  fi

  if test "$PKG_CONFIG" = "no" ; then
     echo "*** The pkg-config script could not be found. Make sure it is"
     echo "*** in your path, or set the PKG_CONFIG environment variable"
     echo "*** to the full path to pkg-config."
     echo "*** Or see http://www.freedesktop.org/software/pkgconfig to get pkg-config."
  else
     PKG_CONFIG_MIN_VERSION=0.9.0
     if $PKG_CONFIG --atleast-pkgconfig-version $PKG_CONFIG_MIN_VERSION; then
        AC_MSG_CHECKING(for $2)

        if $PKG_CONFIG --exists "$2" ; then
            AC_MSG_RESULT(yes)
            succeeded=yes

            AC_MSG_CHECKING($1_CFLAGS)
            $1_CFLAGS=`$PKG_CONFIG --cflags "$2"`
            AC_MSG_RESULT($$1_CFLAGS)

            AC_MSG_CHECKING($1_LIBS)
            $1_LIBS=`$PKG_CONFIG --libs "$2"`
            AC_MSG_RESULT($$1_LIBS)
        else
	    AC_MSG_RESULT(no)
            $1_CFLAGS=""
            $1_LIBS=""
            ## If we have a custom action on failure, don't print errors, but 
            ## do set a variable so people can do so.
            $1_PKG_ERRORS=`$PKG_CONFIG --errors-to-stdout --print-errors "$2"`
            ifelse([$4], ,echo $$1_PKG_ERRORS,)
        fi

        AC_SUBST($1_CFLAGS)
        AC_SUBST($1_LIBS)
     else
        echo "*** Your version of pkg-config is too old. You need version $PKG_CONFIG_MIN_VERSION or newer."
        echo "*** See http://www.freedesktop.org/software/pkgconfig"
     fi
  fi

  if test $succeeded = yes; then
     ifelse([$3], , :, [$3])
  else
     ifelse([$4], , AC_MSG_ERROR([Library requirements ($2) not met; consider adjusting the PKG_CONFIG_PATH environment variable if your libraries are in a nonstandard prefix so pkg-config can find them.]), [$4])
  fi
])


# Configure paths for SDL2
# Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SDL2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDL2, and define SDL2_CFLAGS and SDL2_LIBS
dnl
AC_DEFUN([AM_PATH_SDL2],
[dnl 
dnl Get the cflags and libraries from the sdl2-config script
dnl
AC_ARG_WITH(sdl2-prefix,[  --with-sdl2-prefix=PFX   Prefix where SDL2 is installed (optional)],
            sdl2_prefix="$withval", sdl2_prefix="")
AC_ARG_WITH(sdl2-exec-prefix,[  --with-sdl2-exec-prefix=PFX Exec prefix where SDL2 is installed (optional)],
            sdl2_exec_prefix="$withval", sdl2_exec_prefix="")
AC_ARG_ENABLE(sdl2test, [  --disable-sdl2test       Do not try to compile and run a test SDL2 program],
		    , enable_sdl2test=yes)

  if test x$sdl2_exec_prefix != x ; then
     sdl2_args="$sdl2_args --exec-prefix=$sdl2_exec_prefix"
     if test x${SDL2_CONFIG+set} != xset ; then
        SDL2_CONFIG=$sdl2_exec_prefix/bin/sdl2-config
     fi
  fi
  if test x$sdl2_prefix != x ; then
     sdl2_args="$sdl2_args --prefix=$sdl2_prefix"
     if test x${SDL2_CONFIG+set} != xset ; then
        SDL2_CONFIG=$sdl2_prefix/bin/sdl2-config
     fi
  fi

  AC_PATH_PROG(SDL2_CONFIG, sdl2-config, no)
  min_sdl2_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDL2 - version >= $min_sdl2_version)
  no_sdl2=""
  if test "$SDL2_CONFIG" = "no" ; then
    no_sdl2=yes
  else
    SDL2_CFLAGS=`$SDL2_CONFIG $sdl2conf_args --cflags`
    SDL2_LIBS=`$SDL2_CONFIG $sdl2conf_args --libs`

    sdl2_major_version=`$SDL2_CONFIG $sdl2_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl2_minor_version=`$SDL2_CONFIG $sdl2_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl2_micro_version=`$SDL2_CONFIG $sdl2_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdl2test" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $SDL2_CFLAGS"
      LIBS="$LIBS $SDL2_LIBS"
dnl
dnl Now check if the installed SDL2 is sufficiently new. (Also sanity
dnl checks the results of sdl2-config to some extent
dnl
      rm -f conf.sdl2test
      AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdl2test");
  */
  { FILE *fp = fopen("conf.sdl2test", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl2_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl2_version");
     exit(1);
   }

   if (($sdl2_major_version > major) ||
      (($sdl2_major_version == major) && ($sdl2_minor_version > minor)) ||
      (($sdl2_major_version == major) && ($sdl2_minor_version == minor) && ($sdl2_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl2-config --version' returned %d.%d.%d, but the minimum version\n", $sdl2_major_version, $sdl2_minor_version, $sdl2_micro_version);
      printf("*** of SDL2 required is %d.%d.%d. If sdl2-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl2-config was wrong, set the environment variable SDL2_CONFIG\n");
      printf("*** to point to the correct copy of sdl2-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

]])],[],[no_sdl2=yes],[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdl2" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SDL2_CONFIG" = "no" ; then
       echo "*** The sdl2-config script installed by SDL2 could not be found"
       echo "*** If SDL2 was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDL2_CONFIG environment variable to the"
       echo "*** full path to sdl2-config."
     else
       if test -f conf.sdl2test ; then
        :
       else
          echo "*** Could not run SDL2 test program, checking why..."
          CFLAGS="$CFLAGS $SDL2_CFLAGS"
          LIBS="$LIBS $SDL2_LIBS"
          AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include "SDL.h"
]], [[ return 0; ]])],[ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDL2 or finding the wrong"
          echo "*** version of SDL2. If it is not finding SDL2, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],[ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SDL2 was incorrectly installed"
          echo "*** or that you have moved SDL2 since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl2-config script: $SDL2_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDL2_CFLAGS=""
     SDL2_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDL2_CFLAGS)
  AC_SUBST(SDL2_LIBS)
  rm -f conf.sdl2test
])

# Configure paths for SDL
# Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SDL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDL, and define SDL_CFLAGS and SDL_LIBS
dnl
AC_DEFUN([AM_PATH_SDL],
[dnl 
dnl Get the cflags and libraries from the sdl-config script
dnl
AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)],
            sdl_prefix="$withval", sdl_prefix="")
AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)],
            sdl_exec_prefix="$withval", sdl_exec_prefix="")
AC_ARG_ENABLE(sdltest, [  --disable-sdltest       Do not try to compile and run a test SDL program],
		    , enable_sdltest=yes)

  if test x$sdl_exec_prefix != x ; then
     sdl_args="$sdl_args --exec-prefix=$sdl_exec_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_exec_prefix/bin/sdl-config
     fi
  fi
  if test x$sdl_prefix != x ; then
     sdl_args="$sdl_args --prefix=$sdl_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_prefix/bin/sdl-config
     fi
  fi

  AC_PATH_PROG(SDL_CONFIG, sdl-config, no)
  min_sdl_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)
  no_sdl=""
  if test "$SDL_CONFIG" = "no" ; then
    no_sdl=yes
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdlconf_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdlconf_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_micro_version=`$SDL_CONFIG $sdl_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $SDL_CFLAGS"
      LIBS="$LIBS $SDL_LIBS"
dnl
dnl Now check if the installed SDL is sufficiently new. (Also sanity
dnl checks the results of sdl-config to some extent
dnl
      rm -f conf.sdltest
      AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

]])],[],[no_sdl=yes],[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdl" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$SDL_CONFIG" = "no" ; then
       echo "*** The sdl-config script installed by SDL could not be found"
       echo "*** If SDL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDL_CONFIG environment variable to the"
       echo "*** full path to sdl-config."
     else
       if test -f conf.sdltest ; then
        :
       else
          echo "*** Could not run SDL test program, checking why..."
          CFLAGS="$CFLAGS $SDL_CFLAGS"
          LIBS="$LIBS $SDL_LIBS"
          AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include "SDL.h"
]], [[ return 0; ]])],[ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDL or finding the wrong"
          echo "*** version of SDL. If it is not finding SDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],[ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SDL was incorrectly installed"
          echo "*** or that you have moved SDL since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl-config script: $SDL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDL_CFLAGS=""
     SDL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
  rm -f conf.sdltest
])

# Configure paths for ncursesw
# stolen from Sam Lantinga 9/21/99 from directly above (SDL)
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_NCURSESW([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for ncursesw, and define NCURSES_CFLAGS and NCURSES_LIBS
dnl
AC_DEFUN([AM_PATH_NCURSESW],
[dnl 
dnl Get the cflags and libraries from the ncursesw5-config script
dnl
AC_ARG_WITH(ncurses-prefix,[  --with-ncurses-prefix=PFX   Prefix where ncurses is installed (optional)],
            ncurses_prefix="$withval", ncurses_prefix="")
AC_ARG_WITH(ncurses-exec-prefix,[  --with-ncurses-exec-prefix=PFX Exec prefix where ncurses is installed (optional)],
            ncurses_exec_prefix="$withval", ncurses_exec_prefix="")
AC_ARG_ENABLE(ncursestest, [  --disable-ncursestest       Do not try to compile and run a test ncurses program],
		    , enable_ncursestest=yes)

  if test x$ncurses_exec_prefix != x ; then
     ncurses_args="$ncurses_args --exec-prefix=$ncurses_exec_prefix"
     if test x${NCURSES_CONFIG+set} != xset ; then
        NCURSES_CONFIG=$ncurses_exec_prefix/bin/ncursesw5-config
     fi
  fi
  if test x$ncurses_prefix != x ; then
     ncurses_args="$ncurses_args --prefix=$ncurses_prefix"
     if test x${NCURSES_CONFIG+set} != xset ; then
        NCURSES_CONFIG=$ncurses_prefix/bin/ncursesw5-config
     fi
  fi

  AC_PATH_PROG(NCURSES_CONFIG, ncursesw5-config, no)
  AC_MSG_CHECKING(for ncurses - wide char support)
  no_ncurses=""
  if test "$NCURSES_CONFIG" = "no" ; then
    no_ncurses=yes
  else
    NCURSES_CFLAGS=`$NCURSES_CONFIG $ncurses_args --cflags`
    NCURSES_LIBS=`$NCURSES_CONFIG $ncurses_args --libs`

    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $NCURSES_CFLAGS"
    LIBS="$LIBS $NCURSES_LIBS"
dnl
dnl Now check if the installed ncurses is installed OK. (Also sanity
dnl checks the results of ncursesw5-config to some extent)
dnl
    rm -f conf.ncursestest
    AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include "ncurses.h"

int main (int argc, char *argv[])
{
  { FILE *fp = fopen("conf.ncursestest", "a"); if ( fp ) fclose(fp); }

  return 0;
}

]])],[],[no_ncurses=yes],[echo $ac_n "cross compiling; assumed OK... $ac_c"])
  	CFLAGS="$ac_save_CFLAGS"
  	LIBS="$ac_save_LIBS"
  fi

  if test "x$no_ncurses" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$1], , :, [$1])     
  else
     AC_MSG_RESULT(no)
     if test "$NCURSES_CONFIG" = "no" ; then
       echo "*** The ncursesw5-config script installed by ncursesw could not be found"
       echo "*** If ncursesw was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the NCURSES_CONFIG environment variable to the"
       echo "*** full path to ncursesw5-config."
	 else
	   if test -f conf.ncursestest ; then
        :
       else
          echo "*** Could not run ncurses test program, checking why..."
          CFLAGS="$CFLAGS $NCURSES_CFLAGS"
          LIBS="$LIBS $NCURSES_LIBS"
          AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include "ncurses.h"
]], [[ return 0; ]])],[ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding ncursesw. If it is not finding"
          echo "*** ncursesw, you'll need to set your LD_LIBRARY_PATH environment variable,"
          echo "*** or edit /etc/ld.so.conf to point to the installed location.  Also, make"
          echo "*** sure you have run ldconfig if that is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],[ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means ncursesw was incorrectly"
          echo "*** installed or that you have moved ncursesw since it was installed. In the"
          echo "*** latter case, you may want to edit the ncursesw5-config script:"
          echo "*** $NCURSES_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
	 
     fi
     NCURSES_CFLAGS=""
     NCURSES_LIBS=""
     ifelse([$2], , :, [$2])
  fi
  AC_SUBST(NCURSES_CFLAGS)
  AC_SUBST(NCURSES_LIBS)
  rm -f conf.ncursestest
])
