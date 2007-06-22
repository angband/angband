/*
 * File: h-basic.h
 *
 * The most basic "include" file.
 */

#ifndef INCLUDED_H_BASIC_H
#define INCLUDED_H_BASIC_H

/*
 * Include autoconf autodetections, otherwise try to autodetect ourselves
 */
#ifdef HAVE_CONFIG_H

# include "autoconf.h"

#else

/*
 * Extract the "RISCOS" flag from the compiler
 */
# ifdef __riscos
#  ifndef RISCOS
#   define RISCOS
#  endif
# endif

/*
 * Extract the "WINDOWS" flag from the compiler
 */
# if defined(_Windows) || defined(__WINDOWS__) || \
     defined(__WIN32__) || defined(WIN32) || \
     defined(__WINNT__) || defined(__NT__)
#  ifndef WINDOWS
#   define WINDOWS
#  endif
# endif



/*
 * Using C99, assume we have stdint and stdbool
 */
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define HAVE_STDINT_H
#  define HAVE_STDBOOL_H
# endif



/*
 * Everyone except RISC OS has fcntl.h
 */
#ifndef RISCOS
# define HAVE_FCNTL_H
#endif

#endif /* HAVE_CONFIG_H */




/*
 * OPTION: set "SET_UID" if the machine is a "multi-user" wmachine.
 *
 * This option is used to verify the use of "uids" and "gids" for
 * various "Unix" calls, and of "pids" for getting a random seed,
 * and of the "umask()" call for various reasons, and to guess if
 * the "kill()" function is available, and for permission to use
 * functions to extract user names and expand "tildes" in filenames.
 * It is also used for "locking" and "unlocking" the score file.
 * Basically, SET_UID should *only* be set for "Unix" machines.
 */
#if !defined(MACH_O_CARBON) && !defined(WINDOWS) && \
    !defined(RISCOS) && !defined(GAMEBOY)
# define SET_UID

/* Without autoconf, turn on some things */
# ifndef HAVE_CONFIG_H
#  define HAVE_DIRENT_H
#  define HAVE_SETEGID
#  if defined(linux)
#   define HAVE_SETRESGID
#  endif
# endif

#endif


/*
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

#ifdef WINDOWS
# undef PATH_SEP
# define PATH_SEP "\\"
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

#else

# define FILE_TYPE(X) ((void)0)

#endif



/*** Include the library header files ***/

/** ANSI C headers **/

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** POSIX headers **/

#if defined(SET_UID) || defined(MACH_O_CARBON)
# include <pwd.h>
# include <sys/stat.h>
# include <unistd.h>
#endif



/*** Define the basic game types ***/

/*
 * cptr is a shortcut type for "const char *".  XXX
 * errr is an error code
 *
 * A "byte" is an unsigned byte of memory.
 * s16b/u16b are exactly 2 bytes (where possible)
 * s32b/u32b are exactly 4 bytes (where possible)
 */

/* C++ defines its own bool type, so we hack around it */
#undef bool
#define bool bool_hack


typedef const char *cptr;
typedef int errr;


/*
 * Use a real bool type where possible
 */
#ifdef HAVE_STDBOOL_H

  #include <stdbool.h>

  #define TRUE  true
  #define FALSE false

#else

  /* Use a char otherwise */
  typedef char bool;

  #undef TRUE
  #undef FALSE

  #define TRUE   1
  #define FALSE  0

#endif



/*
 * Use guaranteed-size ints where possible
 */
#ifdef HAVE_STDINT_H

  /* Use guaranteed-size types */
  #include <stdint.h>

  typedef uint8_t byte;

  typedef uint16_t u16b;
  typedef int16_t s16b;

  typedef uint32_t u32b;
  typedef int32_t s32b;

#define MAX_UCHAR		UINT8_MAX
#define MAX_SHORT		INT16_MAX

#else /* HAVE_STDINT_H */

  /* Try hacks instead (not guaranteed to work) */
  typedef unsigned char byte;
  typedef signed short s16b;
  typedef unsigned short u16b;

#define MAX_UCHAR		UCHAR_MAX
#define MAX_SHORT		32767

  /* Detect >32-bit longs */
  #if (UINT_MAX == 0xFFFFFFFFUL) && (ULONG_MAX > 0xFFFFFFFFUL)
    typedef signed int s32b;
    typedef unsigned int u32b;
  #else
    typedef signed long s32b;
    typedef unsigned long u32b;
  #endif

#endif /* HAVE_STDINT_H */



/*** Simple constants ***/

/* Define NULL */
#ifndef NULL
# define NULL ((void*)0)
#endif



/*** Basic math macros ***/

#undef MIN
#undef MAX
#undef ABS
#undef SGN

#define MIN(a,b)	(((a) > (b)) ? (b)  : (a))
#define MAX(a,b)	(((a) < (b)) ? (b)  : (a))
#define ABS(a)		(((a) < 0)   ? (-(a)) : (a))
#define SGN(a)		(((a) < 0)   ? (-1) : ((a) != 0))


/*** Useful array length macro ***/

/*
 * Given an array, determine how many elements are in the array.
 */
#define N_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))


/*** Some hackish character manipulation ***/

/*
 * Note that all "index" values must be "lowercase letters", while
 * all "digits" must be "digits".  Control characters can be made
 * from any legal characters.  XXX XXX XXX
 */
#define A2I(X)	((X) - 'a')
#define I2A(X)	((X) + 'a')
#define D2I(X)	((X) - '0')
#define I2D(X)	((X) + '0')
#define KTRL(X)	((X) & 0x1F)
#define UN_KTRL(X)	((X) + 64)
#define ESCAPE	'\033'


/*
 * System-independent definitions for the arrow keys.
 */
#define ARROW_DOWN	'\x8A'
#define ARROW_LEFT	'\x8B'
#define ARROW_RIGHT	'\x8C'
#define ARROW_UP	'\x8D'

/* Analogous to isdigit() etc in ctypes */
#define isarrow(c)	((c >= ARROW_DOWN) && (c <= ARROW_UP))


#endif /* INCLUDED_H_BASIC_H */
