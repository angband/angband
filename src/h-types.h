/* File: h-types.h */

#ifndef INCLUDED_H_TYPES_H
#define INCLUDED_H_TYPES_H

/*
 * Basic "types".
 *
 * Note the attempt to make all basic types have 4 letters.
 * This improves readibility and standardizes the code.
 *
 * Likewise, all complex types are at least 4 letters.
 * Thus, almost every three letter word is a legal variable.
 * But beware of certain reserved words ('for' and 'if' and 'do').
 *
 * Note that the type used in structures for bit flags should be uint.
 * As long as these bit flags are sequential, they will be space smart.
 *
 * Note that on some machines, apparently "signed char" is illegal.
 *
 * It must be true that char/byte takes exactly 1 byte
 * It must be true that sind/uind takes exactly 2 bytes
 * It must be true that sbig/ubig takes exactly 4 bytes
 *
 * On Sparc's, a sint takes 4 bytes (2 is legal)
 * On Sparc's, a uint takes 4 bytes (2 is legal)
 * On Sparc's, a long takes 4 bytes (8 is legal)
 * On Sparc's, a huge takes 4 bytes (8 is legal)
 * On Sparc's, a vptr takes 4 bytes (8 is legal)
 * On Sparc's, a real takes 8 bytes (4 is legal)
 *
 * Note that some files have already been included by "h-include.h"
 * These include <stdio.h> and <sys/types>, which define some types
 * In particular, uint is defined so we do not have to define it
 *
 * Also, see <limits.h> for min/max values for sind, uind, long, huge
 * (SHRT_MIN, SHRT_MAX, USHRT_MAX, LONG_MIN, LONG_MAX, ULONG_MAX)
 * These limits should be verified and coded into "h-constant.h".
 */



/*** Special 4 letter names for some standard types ***/


/* A standard pointer (to "void" because ANSI C says so) */
typedef void *vptr;

/* A simple pointer (to unmodifiable strings) */
typedef const char *cptr;


/* Since float's are silly, hard code real numbers as doubles */
typedef double real;


/* Error codes for function return values */
/* Success = 0, Failure = -N, Problem = +N */
typedef int errr;


/*
 * Some annoying machines define "uint" in some "include" file
 * Note that this "redefinition" should work on any machine.
 */
#if !defined(MACINTOSH) && !defined(__EMX__)
# define uint uint_hack
#endif

/*
 * Some annoying machines (Windows with Turbo C) reserve "huge".
 * Note that this "redefinition" should work on any machine.
 */
#if defined(_Windows)
# define huge huge_hack
#endif
#ifdef __WATCOMC__
# undef huge
#endif

/*
 * Hack -- The Amiga appears to reserve "byte"
 * So we will pre-empt "byte" and use "byte_hack" instead.
 * XXX If this does not work, then remove it and surround
 * the "typedef" of "byte" with "#ifndef AMIGA"/"#endif"
 */
#ifdef AMIGA
# undef byte
# define byte byte_hack
#endif


/* Note that "signed char" is not always "defined" */
/* A (possibly signed) char (a byte) */
/* typedef char char; */

/* An unsigned byte of memory */
typedef unsigned char byte;

/* Simple True/False type */
typedef char bool;


/* A signed, standard integer (at least 2 bytes) */
typedef int sint;

/* An unsigned, "standard" integer (usually pre-defined) */
typedef unsigned int uint;


/* The largest signed integer there is (pre-defined) */
/* typedef long long; */

/* The largest unsigned integer there is */
typedef unsigned long huge;


/* Signed/Unsigned 16 bit value */
typedef signed short s16b;
typedef unsigned short u16b;

/* Signed/Unsigned 32 bit value */
#ifdef L64	/* 64 bit longs */
typedef signed int s32b;
typedef unsigned int u32b;
#else
typedef signed long s32b;
typedef unsigned long u32b;
#endif




/*** Pointers to all the basic types defined above ***/

typedef real *real_ptr;
typedef errr *errr_ptr;
typedef char *char_ptr;
typedef byte *byte_ptr;
typedef bool *bool_ptr;
typedef sint *sint_ptr;
typedef uint *uint_ptr;
typedef long *long_ptr;
typedef huge *huge_ptr;
typedef s16b *u16b_ptr;
typedef u16b *s16b_ptr;
typedef s32b *u32b_ptr;
typedef u32b *s32b_ptr;
typedef vptr *vptr_ptr;
typedef cptr *cptr_ptr;



/*** Pointers to Functions with simple return types and any args ***/

typedef void	(*func_void)();
typedef errr	(*func_errr)();
typedef char	(*func_char)();
typedef byte	(*func_byte)();
typedef bool	(*func_bool)();
typedef sint	(*func_sint)();
typedef uint	(*func_uint)();
typedef real	(*func_real)();
typedef vptr	(*func_vptr)();
typedef cptr	(*func_cptr)();



/*** Pointers to Functions of special types (for various purposes) ***/

/* A generic function takes a user data and a special data */
typedef errr	(*func_gen)(vptr, vptr);

/* An equality testing function takes two things to compare (bool) */
typedef bool	(*func_eql)(vptr, vptr);

/* A comparison function takes two things and to compare (-1,0,+1) */
typedef sint	(*func_cmp)(vptr, vptr);

/* A hasher takes a thing (and a max hash size) to hash (0 to siz - 1) */
typedef uint	(*func_hsh)(vptr, uint);

/* A key extractor takes a thing and returns (a pointer to) some key */
typedef vptr	(*func_key)(vptr);



#endif

