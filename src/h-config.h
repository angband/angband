/* File: h-config.h */

#ifndef INCLUDED_H_CONFIG_H
#define INCLUDED_H_CONFIG_H

/*
 * Choose the hardware, operating system, and compiler.
 * Also, choose various "system level" compilation options.
 * A lot of these definitions take effect in "h-system.h"
 *
 * Note that most of these "options" are defined by the compiler,
 * the "Makefile", the "project file", or something similar, and
 * should not be defined by the user.
 */

#include <limits.h>

/*
 * OPTION: Compile on a Windows machine
 */
#ifndef WINDOWS
/* #define WINDOWS */
#endif

/*
 * OPTION: Compile on an MSDOS machine
 */
#ifndef MSDOS
/* #define MSDOS */
#endif

/*
 * OPTION: Compile on a SYS III version of UNIX
 */
#ifndef SYS_III
/* #define SYS_III */
#endif

/*
 * OPTION: Compile on a SYS V version of UNIX
 */
#ifndef SYS_V
/* #define SYS_V */
#endif

/*
 * OPTION: Compile on a HPUX version of UNIX
 */
#ifndef HPUX
/* #define HPUX */
#endif

/*
 * OPTION: Compile on an SGI running IRIX
 */
#ifndef SGI
/* #define SGI */
#endif

/*
 * OPTION: Compile on a SunOS machine
 */
#ifndef SUNOS
/* #define SUNOS */
#endif

/*
 * OPTION: Compile on a Solaris machine
 */
#ifndef SOLARIS
/* #define SOLARIS */
#endif

/*
 * OPTION: Compile on an ultrix/4.2BSD/Dynix/etc. version of UNIX,
 * Do not define this if you are on any kind of SunOS.
 */
#ifndef ULTRIX
/* #define ULTRIX */
#endif



/*
 * Extract the "SUNOS" flag from the compiler
 */
#if defined(sun)
# ifndef SUNOS
#   define SUNOS
# endif
#endif

/*
 * Extract the "ULTRIX" flag from the compiler
 */
#if defined(ultrix) || defined(Pyramid)
# ifndef ULTRIX
#  define ULTRIX
# endif
#endif

/*
 * Extract the "ATARI" flag from the compiler [cjh]
 */
#if defined(__atarist) || defined(__atarist__)
# ifndef ATARI
#  define ATARI
# endif
#endif

/*
 * Extract the "RISCOS" flag from the compiler
 */
#ifdef __riscos
# ifndef RISCOS
#  define RISCOS
# endif
#endif

/*
 * Extract the "SGI" flag from the compiler
 */
#ifdef sgi
# ifndef SGI
#  define SGI
# endif
#endif

/*
 * Extract the "MSDOS" flag from the compiler
 */
#ifdef __MSDOS__
# ifndef MSDOS
#  define MSDOS
# endif
#endif

/*
 * Extract the "WINDOWS" flag from the compiler
 */
#if defined(_Windows) || defined(__WINDOWS__) || \
    defined(__WIN32__) || defined(WIN32) || \
    defined(__WINNT__) || defined(__NT__)
# ifndef WINDOWS
#  define WINDOWS
# endif
#endif

/*
 * Remove the MSDOS flag when using WINDOWS
 */
#ifdef WINDOWS
# ifdef MSDOS
#  undef MSDOS
# endif
#endif

/*
 * OPTION: Define "L64" if a "long" is 64-bits.  See "h-types.h".
 * The only such platform that angband is ported to is currently
 * DEC Alpha AXP running OSF/1 (OpenVMS uses 32-bit longs).
 *
 * Try to use __WORDSIZE to test for 64-bit platforms.
 * I don't know how portable this is.
 * -CJN-
 */
#ifdef __WORDSIZE
# if __WORDSIZE == 64
#  define L64
# endif
#endif

#if defined(__alpha) && defined(__osf__) && !defined(L64)
# define L64
#endif



/*
 * OPTION: set "SET_UID" if the machine is a "multi-user" machine.
 * This option is used to verify the use of "uids" and "gids" for
 * various "Unix" calls, and of "pids" for getting a random seed,
 * and of the "umask()" call for various reasons, and to guess if
 * the "kill()" function is available, and for permission to use
 * functions to extract user names and expand "tildes" in filenames.
 * It is also used for "locking" and "unlocking" the score file.
 * Basically, SET_UID should *only* be set for "Unix" machines,
 * or for the "Atari" platform which is Unix-like, apparently
 */
#if !defined(MACH_O_CARBON) && !defined(WINDOWS) && \
    !defined(MSDOS) && !defined(USE_EMX) && \
    !defined(AMIGA) && !defined(RISCOS)
# define SET_UID
#endif


/*
 * Every system seems to use its own symbol as a path separator.
 * Default to the standard Unix slash, but attempt to change this
 * for various other systems.  Note that any system that uses the
 * "period" as a separator (i.e. RISCOS) will have to pretend that
 * it uses the slash, and do its own mapping of period <-> slash.
 */
#undef PATH_SEP
#define PATH_SEP "/"

#if defined(WINDOWS) || defined(WINNT) || defined(MSDOS) || defined(OS2) || defined(USE_EMX)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif

#if defined(AMIGA) || defined(__GO32__)
# undef PATH_SEP
# define PATH_SEP "/"
#endif


/*
 * Mac support
 */
#ifdef MACH_O_CARBON

/* OS X uses filetypes when creating files. */
# define FILE_TYPE_TEXT 'TEXT'
# define FILE_TYPE_DATA 'DATA'
# define FILE_TYPE_SAVE 'SAVE'
# define FILE_TYPE(X) (_ftype = (X))

extern u32b _fcreator;
extern u32b _ftype;

/* Mac OS X has usleep(). */
# define HAVE_USLEEP

#else

# define FILE_TYPE(X) ((void)0)

#endif

/*
 * OPTION: Define "HAVE_USLEEP" only if "usleep()" exists.
 *
 * Note that this is only relevant for "SET_UID" machines.
 *
 * (Set in autoconf.h when HAVE_CONFIG_H -- i.e. when configure is used.)
 */
#if defined(SET_UID) && !defined(HAVE_CONFIG_H)
# if !defined(HPUX) && !defined(ULTRIX) && !defined(ISC)
#  define HAVE_USLEEP
# endif
#endif



#endif /* INCLUDED_H_CONFIG_H */


