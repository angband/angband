/* File: h-config.h */

#ifndef INCLUDED_H_CONFIG_H
#define INCLUDED_H_CONFIG_H

/*
 * Choose the hardware, operating system, and compiler.
 * Also, choose various "system level" compilation options.
 * A lot of these definitions take effect in "h-system.h"
 *
 * Note that you may find it simpler to define some of these
 * options in the "Makefile", especially any options describing
 * what "system" is being used.
 */


/*
 * no system definitions are needed for 4.3BSD, SUN OS, DG/UX
 */

/*
 * OPTION: Compile on a Macintosh (see "A-mac-h" or "A-mac-pch")
 */
/* #define MACINTOSH */

/*
 * OPTION: Compile on Windows (automatic)
 */
/* #define WINDOWS */

/*
 * OPTION: Compile on an IBM (automatic)
 */
/* #define MSDOS */

/*
 * OPTION: Compile on a SYS III version of UNIX
 */
/* #define SYS_III */

/*
 * OPTION: Compile on a SYS V version of UNIX (not Solaris)
 */
/* #define SYS_V */

/*
 * OPTION: Compile on a HPUX version of UNIX
 */
/* #define HPUX */

/*
 * OPTION: Compile on an SGI running IRIX
 */
/* #define SGI */

/*
 * OPTION: Compile on Solaris, treat it as System V
 */
/* #define SOLARIS */

/*
 * OPTION: Compile on an ultrix/4.2BSD/Dynix/etc. version of UNIX,
 * Do not define this if you are on any kind of SUN OS.
 */
/* #define ultrix */


/*
 * OPTION: Compile on Pyramid, treat it as Ultrix
 */
#if defined(Pyramid)
# ifndef ultrix
#  define ultrix
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
 * Extract the "ACORN" flag from the compiler
 */
#ifdef __riscos
# ifndef ACORN
#  define ACORN
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
#if defined(_Windows) || defined(__WIN32__)
# ifndef WINDOWS
#  define WINDOWS
# endif
#endif


/*
 * OPTION: Define "L64" if a "long" is 64-bits.  See "h-types.h".
 * The only such platform that angband is ported to is currently
 * DEC Alpha AXP running OSF/1 (OpenVMS uses 32-bit longs).
 */
#if defined(__alpha) && defined(__osf__)
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
#if !defined(MACINTOSH) && !defined(WINDOWS) && \
    !defined(MSDOS) && !defined(__EMX__) && \
    !defined(AMIGA) && !defined(ACORN)
# define SET_UID
#endif


/*
 * OPTION: Set "USG" for "System V" versions of Unix
 * This is used to choose a "lock()" function, and to choose
 * which header files ("string.h" vs "strings.h") to include.
 * It is also used to allow certain other options, such as options
 * involving userid's, or multiple users on a single machine, etc.
 */
#ifdef SET_UID
# if defined(SYS_III) || defined(SYS_V) || defined(SOLARIS) || \
     defined(HPUX) || defined(SGI) || defined(ATARI)
#  ifndef USG
#   define USG
#  endif
# endif
#endif


/*
 * Every system seems to use its own symbol as a path separator.
 * Default to the standard Unix slash, but attempt to change this
 * for various other systems.  Note that any system that uses the
 * "period" as a separator (i.e. ACORN) will have to pretend that
 * it uses the slash, and do its own mapping of period <-> slash.
 */
#undef PATH_SEP
#define PATH_SEP "/"
#ifdef MACINTOSH
# undef PATH_SEP
# define PATH_SEP ":"
#endif
#if defined(WINDOWS) || defined(WINNT)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#if defined(MSDOS) || defined(OS2) || defined(__EMX__)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#ifdef AMIGA
# undef PATH_SEP
# define PATH_SEP "/"
#endif
#ifdef __GO32__
# undef PATH_SEP
# define PATH_SEP "/"
#endif


/*
 * OPTION: Hack -- Make sure "strchr" will work
 */
#if defined(SYS_III) || defined(SYS_V) || defined(MSDOS)
# if !defined(__TURBOC__) && !defined(__WATCOMC__)
#  define strchr index
# endif
#endif


/*
 * OPTION: Define "HAS_STRICMP" only if "stricmp()" exists.
 * Note that "stricmp()" is not actually used by Angband.
 */
/* #define HAS_STRICMP */

/*
 * Linux has "stricmp()" with a different name
 */
#if defined(linux)
# define HAS_STRICMP
# define stricmp strcasecmp
#endif


/*
 * OPTION: Define "HAS_MEMSET" only if "memset()" exists.
 * Note that the "memset()" routines are used in "z-virt.h"
 */
#define HAS_MEMSET


/*
 * OPTION: Define "HAS_USLEEP" only if "usleep()" exists.
 * Note that this is only relevant for "SET_UID" machines
 */
#ifdef SET_UID
# if !defined(HPUX) && !defined(ultrix) && !defined(SOLARIS) && \
     !defined(SGI) && !defined(ISC)
#  define HAS_USLEEP
# endif
#endif



#endif


