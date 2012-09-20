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
#  define NULL ((void*)0)
# else
#  define NULL ((char*)0)
# endif /* __STDC__ */
#endif /* NULL */


/*
 * Hack -- fake definitions for Acorn (see "main-acn.c")
 */
#ifdef ACORN

# define     O_RDONLY        0
# define     O_WRONLY        1
# define     O_RDWR          2

# define     O_CREAT         0x10
# define     O_TRUNC         0x20
# define     O_EXCL          0x40
# define     O_BINARY        0x80

# define     EEXIST          17

#endif


/*
 * Hack -- force definition
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif

/*
 * Hack -- force definitions -- see fd_seek()
 */
#ifndef SEEK_SET
# define SEEK_SET	0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR	1
#endif
#ifndef SEEK_END
# define SEEK_END	2
#endif

/*
 * Hack -- force definitions -- see fd_lock()  XXX XXX XXX
 */
#ifndef F_UNLCK
# define F_UNLCK	0
#endif
#ifndef F_RDLCK
# define F_RDLCK	1
#endif
#ifndef F_WRLCK
# define F_WRLCK	2
#endif


/*
 * Some simple real numbers
 */

#define RAD2	1.41421356237
#define RAD3	1.73205080757

#define PI  	3.14159265358979324
#define TWOPI	6.28318530717958648


/*
 * The constants "TRUE" and "FALSE"
 */

#undef TRUE
#define TRUE	1

#undef FALSE
#define FALSE	0


/*
 * Hack -- Simple Emacs-like Control-cursor keys
 */

#define CURSOR_UP	('P' - '@')
#define CURSOR_DN	('N' - '@')
#define CURSOR_LT	('B' - '@')
#define CURSOR_RT	('F' - '@')

#define CURSOR_HOME	('A' - '@')
#define CURSOR_END	('E' - '@')

#define CURSOR_BS	('H' - '@')
#define CURSOR_DEL	('D' - '@')

#define CURSOR_KILL	('K' - '@')


/*
 * Ascii equivelents to standard keys
 */

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

