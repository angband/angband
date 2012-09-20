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
 * OPTION: Compile on a Macintosh
 */
/* #define MACINTOSH */

/*
 * OPTION: Compile on Windows (automatic)
 */
/* #define _Windows */

/*
 * OPTION: Compile on an IBM (automatic)
 */
/* #define MSDOS */

/*
 * OPTION: Compile on a SYS V version of UNIX
 */
/* #define SYS_V */

/*
 * OPTION: Compile on a SYS III version of UNIX
 */
/* #define SYS_III */

/*
 * OPTION: Compile on an ATARI ST with Mark Williams C
 * Warning: This option has not been tested recently
 */
/* #define ATARIST_MWC */

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
# define ultrix
#endif

/*
 * Hack -- treat "SOLARIS" as a form of "SYS_V"
 */
#if defined(SOLARIS)
# define SYS_V
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
 * OPTION: Define "L64" if a "long" is 64-bits.  See "h-types.h".
 * The only such platform that angband is ported to is currently
 * DEC Alpha AXP running OSF/1 (OpenVMS uses 32-bit longs).
 */
#if defined(__alpha) && defined(__osf__)
# define L64
#endif



/*
 * OPTION: set "SET_UID" if the machine is a "multi-user" machine
 * Assume this is a multi-user machine, then cancel if necessary.
 * This option is used to verify the use of "uids" and "gids" for
 * various "Unix" calls, and of "pids" for getting a random seed,
 * and of the "umask()" call for various reasons, and to guess if
 * the "kill()" function is available, and for permission to use
 * functions to extract user names and expand "tildes" in filenames.
 * Basically, SET_UID should *only* be TRUE for "Unix" machines
 */
#undef SET_UID
#define SET_UID
#if defined(MACINTOSH) || defined(MSDOS) || defined(AMIGA) || \
    defined(__MINT__) || defined(__EMX__) || defined(_Windows)
# undef SET_UID
#endif



/*
 * Lots of systems use USG, whatever that means
 */
#if defined(MACINTOSH) || defined(MSDOS) || defined(AMIGA) || \
    defined (__MINT__) || defined(__EMX__) || \
    defined(SYS_III) || defined(SYS_V) || defined(HPUX) || \
    defined(ATARIST_MWC) || defined(SGI)
# undef USG
# define USG
#endif



/*
 * Every system seems to use its own symbol as a path separator.
 * Default to the standard Unix slash, but attempt to change this
 * for various other systems.
 */
#undef PATH_SEP
#define PATH_SEP "/"
#if defined(MSDOS) || defined(OS2) || defined(__EMX__)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#if defined(WINNT)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#if defined(ATARIST_MWC) || defined(ATARI) || defined(ATARIST)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif
#ifdef __GO32__
# undef PATH_SEP
# define PATH_SEP "/"
#endif
#ifdef AMIGA
# undef PATH_SEP
# define PATH_SEP "/"
#endif
#ifdef MACINTOSH
# undef PATH_SEP
# define PATH_SEP ":"
#endif


/*
 * OPTION: Hack -- Make sure "strchr" will work
 */
#if defined(SYS_V) || defined(MSDOS)
# if !defined(SOLARIS) && !defined(__TURBOC__) && !defined(__WATCOMC__)
#  define strchr index
# endif
#endif


/*
 * OPTION: Define "HAS_MEMSET" only if "memset()" exists.
 */
#define HAS_MEMSET


/*
 * OPTION: Define "HAS_USLEEP" only if "usleep()" exists.
 */
#if !defined(HPUX) && !defined(ultrix) && !defined(SOLARIS) && \
    !defined(SGI) && !defined(ISC)
# define HAS_USLEEP
#endif



#endif


