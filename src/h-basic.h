/**
 * \file h-basic.h
 * \brief The lowest level header
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_H_BASIC_H
#define INCLUDED_H_BASIC_H

/**
 * Include autoconf autodetections, otherwise try to autodetect ourselves
 */
#ifdef HAVE_CONFIG_H

# include "autoconf.h"

#else

/**
 * Using C99, assume we have stdint and stdbool
 */
# if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || (defined(_MSC_VER) && _MSC_VER >= 1600L)
#  define HAVE_STDINT_H 1
# endif

# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define HAVE_STDbool_H
# endif

/**
 * Everyone except RISC OS has fcntl.h and sys/stat.h
 */
#define HAVE_FCNTL_H
#define HAVE_STAT

#endif /* HAVE_CONFIG_H */

/**
 * Extract the "WINDOWS" flag from the compiler
 */
# if defined(_Windows) || defined(__WINDOWS__) || \
     defined(__WIN32__) || defined(WIN32) || \
     defined(__WINNT__) || defined(__NT__)
#  ifndef WINDOWS
#   define WINDOWS
#  endif
# endif

/**
 * Define UNIX if our OS is UNIXy
 */
#if !defined(WINDOWS) && !defined(GAMEBOY) && !defined(NDS)
# define UNIX

# ifndef HAVE_DIRENT_H
#  define HAVE_DIRENT_H
# endif

/**
 * May need to be tightened:  without autoconf.h assume all Unixes have mkdir().
 */
# if !defined(HAVE_MKDIR) && !defined(HAVE_CONFIG_H)
#   define HAVE_MKDIR
# endif

#endif

/**
 * Every system seems to use its own symbol as a path separator.
 *
 * Default to the standard Unix slash, but attempt to change this
 * for various other systems.  Note that any system that uses the
 * "period" as a separator (i.e. RISCOS) will have to pretend that
 * it uses the slash, and do its own mapping of period <-> slash.
 *
 * It is most definitely wrong to have such things here.  Platform-specific
 * code should handle shifting Angband filenames to platform ones. XXX
 */
#undef PATH_SEP
#define PATH_SEP "/"
#define PATH_SEPC '/'

#ifdef WINDOWS
# undef PATH_SEP
# undef PATH_SEPC
# define PATH_SEP "\\"
# define PATH_SEPC '\\'
#endif


/**
 * ------------------------------------------------------------------------
 * Include the library header files
 * ------------------------------------------------------------------------ */


/* Use various POSIX functions if available */
#undef _GNU_SOURCE
#define _GNU_SOURCE

/** ANSI C headers **/
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include <wchar.h>
#include <wctype.h>

/** POSIX headers **/
#ifdef UNIX
# include <pwd.h>
# include <sys/stat.h>
# include <unistd.h>
#endif



/**
 * ------------------------------------------------------------------------
 * Define the basic game types
 * ------------------------------------------------------------------------ */


/**
 * errr is an error code
 *
 * byte/s16b/u16b/s32b/u32b/s64b/u64b have been deprecated; use the
 * standard uint8_t, int16_t, uint16_t, ... types instead.  Those typedefs
 * will still be defined if SKIP_ANGBAND_OLD_INT_TYPEDEFS is not set.
 *
 * A "byte" is an unsigned byte of memory.
 * s16b/u16b are exactly 2 bytes (where possible)
 * s32b/u32b are exactly 4 bytes (where possible)
 */

typedef int errr;

#ifndef SKIP_ANGBAND_OLD_INT_TYPEDEFS
/* Use guaranteed-size types */
typedef uint8_t byte;

typedef uint16_t u16b;
typedef int16_t s16b;

typedef uint32_t u32b;
typedef int32_t s32b;

typedef uint64_t u64b;
typedef int64_t s64b;
#endif

/** Debugging macros ***/

#define DSTRINGIFY(x) #x
#define DSTRING(x)    DSTRINGIFY(x)
#define DHERE         __FILE__ ":" DSTRING(__LINE__) ": "


/**
 * ------------------------------------------------------------------------
 * Basic math macros
 * ------------------------------------------------------------------------ */


#undef MIN
#undef MAX
#undef ABS
#undef SGN
#undef CMP

#define MIN(a,b)	(((a) > (b)) ? (b)  : (a))
#define MAX(a,b)	(((a) < (b)) ? (b)  : (a))
#define ABS(a)		(((a) < 0)   ? (-(a)) : (a))
#define SGN(a)		(((a) < 0)   ? (-1) : ((a) != 0))
#define CMP(a,b) ((a) < (b) ? -1 : ((b) < (a) ? 1 : 0))


/**
 * ------------------------------------------------------------------------
 * Useful fairly generic macros
 * ------------------------------------------------------------------------ */


/**
 * Given an array, determine how many elements are in it.
 */
#define N_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))

/**
 * ------------------------------------------------------------------------
 * Some hackish character manipulation
 * ------------------------------------------------------------------------ */


/**
 * Note that all "index" values must be "lowercase letters", while
 * all "digits" must be "digits".
 */
#define A2I(X)	((X) - 'a')
#define I2A(X)	((X) + 'a')
#define D2I(X)	((X) - '0')
#define I2D(X)	((X) + '0')

#endif /* INCLUDED_H_BASIC_H */
