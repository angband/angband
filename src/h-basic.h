/*
 * File: h-basic.h
 *
 * The most basic "include" file.
 */

#ifndef INCLUDED_H_BASIC_H
#define INCLUDED_H_BASIC_H

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */


/*
 * h-config sets various system-specific defines, relied on later in this file
 * and throughout the game.
 */
#include "h-config.h"



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

#ifdef SET_UID
# include <sys/types.h>
#endif

#if defined(__DJGPP__) || defined(__MWERKS__)
#include <unistd.h>
#endif

/** Other headers **/

#if defined(MACINTOSH) && defined(__MWERKS__)
# include <unix.h>
#endif

#if defined(WINDOWS) || defined(MSDOS) || defined(USE_EMX)
# include <io.h>
#endif



/*** Define the basic game types ***/

/*
 * cptr is a shortcut type for "const char *".  XXX
 * errr is an error code
 *
 * A "byte" is an unsigned byte of memory.
 * s16b/u16b are exactly 2 bytes (where possible)
 * s32b/u32b are exactly 4 bytes (where possible)
 *
 * We define a "bool" as a char.  We should really be able to use the C89 types
 * where available. XXX
 */

/* C++ defines its own bool type, so we hack around it */
#undef bool
#define bool bool_hack


typedef const char *cptr;
typedef int errr;


#if defined(HAVE_STDBOOL_H)

  #include <stdbool.h>

  #define TRUE  true
  #define FALSE false

#else

  typedef char bool;

  #undef TRUE
  #undef FALSE

  #define TRUE   1
  #define FALSE  0

#endif


/* C99/stdint.h provide guaranteed-size ints */
#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L) || defined HAVE_STDINT_H

  /* Use guaranteed-size types */
  #include <stdint.h>

  typedef uint8_t byte;

  typedef uint16_t u16b;
  typedef int16_t s16b;

  typedef uint32_t u32b;
  typedef int32_t s32b;

#else /* __STDC__ */

  /* Try hacks instead (not guaranteed to work) */
  typedef unsigned char byte;
  typedef signed short s16b;
  typedef unsigned short u16b;

  /* Detect >32-bit longs */
  #if (UINT_MAX == 0xFFFFFFFFUL) && (ULONG_MAX > 0xFFFFFFFFUL)
    typedef signed int s32b;
    typedef unsigned int u32b;
  #else
    typedef signed long s32b;
    typedef unsigned long u32b;
  #endif /* L64 */

#endif /* __STDC__ */



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


#endif


