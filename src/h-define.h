/* File: h-define.h */

#ifndef INCLUDED_H_DEFINE_H
#define INCLUDED_H_DEFINE_H

/*
 * Define some simple constants
 */


/*
 * Hack -- Define NULL
 */
#ifndef NULL
# ifdef __STDC__
#  define NULL ((void *)0)
# else
#  define NULL ((char *)0)
# endif /* __STDC__ */
#endif /* NULL */


/*
 * Hack -- force definition
 */
#ifndef O_BINARY
# define O_BINARY 0x00
#endif

/*
 * Hack -- force definition
 */
#ifndef L_SET
# define L_SET 0
#endif



/*
 * Some standard typed NULL pointers
 */

#define C_NULL	((cptr)NULL)
#define V_NULL	((vptr)NULL)


/* Some simple real numbers */

#define ZERO	0.0
#define ONE 	1.0
#define TWO 	2.0

#define RAD2	1.41421356237
#define RAD3	1.73205080757

#define PI  	3.14159265358979324
#define TWOPI	6.28318530717958648


/* The constants "TRUE" and "FALSE" */

#undef TRUE
#define TRUE	1

#undef FALSE
#define FALSE	0


/* Simple Emacs-like Control-cursor keys */

#define CURSOR_UP	('P' - '@')
#define CURSOR_DN	('N' - '@')
#define CURSOR_LT	('B' - '@')
#define CURSOR_RT	('F' - '@')

#define CURSOR_HOME	('A' - '@')
#define CURSOR_END	('E' - '@')

#define CURSOR_BS	('H' - '@')
#define CURSOR_DEL	('D' - '@')

#define CURSOR_KILL	('K' - '@')


/* Ascii equivelents to standard keys */

#define CURSOR_SPC	' '
#define CURSOR_TAB	'\t'
#define CURSOR_RET	'\n'
#define CURSOR_ESC	'\033'



/**** Assertions and Preventions ****/

#ifdef CHECK_ASSERTIONS

/* Note that the following functions require "z-form.h" */

/* Allow Certain Conditions 'C' to be ABSOLUTELY ILLEGAL */
#define PREVENT(C) \
        if (C) core_fmt("PREVENT(Line %d in file %s)", __LINE__, __FILE__);

/* Allow Certain Conditions 'C' to be ABSOLUTELY NECESSARY */
#define ASSERT(C) \
        if (!(C)) core_fmt("ASSERT(Line %d in file %s)", __LINE__, __FILE__);

#else

/* Ignore Preventions */
#define PREVENT(C)

/* Ignore Assertions */
#define ASSERT(C)

#endif



/**** Simple "Macros" ****/

/*
 * Force a character to lowercase/uppercase
 */
#define FORCELOWER(A)  ((isupper((A))) ? tolower((A)) : (A))
#define FORCEUPPER(A)  ((islower((A))) ? toupper((A)) : (A))


/*
 * Non-typed minimum value macro
 */
#undef MIN
#define MIN(a,b)	(((a) > (b)) ? (b)  : (a))

/*
 * Non-typed maximum value macro
 */
#undef MAX
#define MAX(a,b)	(((a) < (b)) ? (b)  : (a))

/*
 * Non-typed absolute value macro
 */
#undef ABS
#define ABS(a)		(((a) < 0)   ? (-(a)) : (a))

/*
 * Non-typed sign extractor macro
 */
#undef SGN
#define SGN(a)		(((a) < 0)   ? (-1) : ((a) != 0))


#endif

