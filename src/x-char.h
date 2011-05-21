<<<<<<< HEAD
#ifndef INCLUDED_X_CHAR_H
#define INCLUDED_X_CHAR_H


/* Column titles for character information table */
#define COLUMN_TO_UPPER 	0
#define COLUMN_TO_LOWER 	1
#define COLUMN_CHAR_TYPE	2

#define CHAR_TABLE_SLOTS	3

/* Bit flags for COLUMN_CHAR_TYPE */
#define CHAR_BLANK  0x01
#define CHAR_UPPER  0x02
#define CHAR_LOWER  0x04
#define CHAR_PUNCT  0x08
#define CHAR_SYMBOL 0x10
#define CHAR_DIGIT  0x20
#define CHAR_VOWEL  0x40
#define CHAR_XXXX3  0x80


/*
 * Modes of operation for the "xstr_trans()" function.
 */
#define LATIN1  0
#define SYSTEM_SPECIFIC 1
#define ASCII 2

/*
 * Set of customized macros for use with 256 character set
 */
#define	my_islower(Y) \
		(char_tables[(byte)(Y)][COLUMN_CHAR_TYPE] & (CHAR_LOWER))

#define	my_isspace(Y) \
		(char_tables[(byte)(Y)][COLUMN_CHAR_TYPE] & (CHAR_BLANK))

#define	my_isprint(Y) \
 		(char_tables[(byte)(Y)][COLUMN_CHAR_TYPE] & (CHAR_BLANK | CHAR_UPPER | CHAR_LOWER | \
 	 									           CHAR_PUNCT | CHAR_DIGIT))

#define my_toupper(Y) \
		(char_tables[(byte)(Y)][COLUMN_TO_UPPER])

/*
 * An extended character translation.  Using a tag,
 * get a 8-bit character.  Or, using an 8-bit character,
 * get a tag.
 */
typedef struct xchar_type xchar_type;
struct xchar_type
{
	const char *tag;
	byte c;
};

/*
 * x-char.c function declarations are located in externs.h.
 * x-char-c tables are loctaed in tables.c
 */


#endif /* INCLUDED_X_CHAR_H */

=======
#ifndef INCLUDED_X_CHAR_H
#define INCLUDED_X_CHAR_H


/* Column titles for character information table */
#define COLUMN_TO_UPPER 	0
#define COLUMN_TO_LOWER 	1
#define COLUMN_CHAR_TYPE	2

#define CHAR_TABLE_SLOTS	3

/* Bit flags for COLUMN_CHAR_TYPE */
#define CHAR_BLANK  0x01
#define CHAR_UPPER  0x02
#define CHAR_LOWER  0x04
#define CHAR_PUNCT  0x08
#define CHAR_SYMBOL 0x10
#define CHAR_DIGIT  0x20
#define CHAR_VOWEL  0x40
#define CHAR_XXXX3  0x80


/*
 * Modes of operation for the "xstr_trans()" function.
 */
#define LATIN1  0
#define SYSTEM_SPECIFIC 1
#define ASCII 2

/*
 * Set of customized macros for use with 256 character set
 */
#define	my_islower(Y) \
		(char_tables[(byte)(Y)][COLUMN_CHAR_TYPE] & (CHAR_LOWER))

#define	my_isspace(Y) \
		(char_tables[(byte)(Y)][COLUMN_CHAR_TYPE] & (CHAR_BLANK))

#define	my_isprint(Y) \
 		(char_tables[(byte)(Y)][COLUMN_CHAR_TYPE] & (CHAR_BLANK | CHAR_UPPER | CHAR_LOWER | \
 	 									           CHAR_PUNCT | CHAR_DIGIT))

#define my_toupper(Y) \
		(char_tables[(byte)(Y)][COLUMN_TO_UPPER])

/*
 * An extended character translation.  Using a tag,
 * get a 8-bit character.  Or, using an 8-bit character,
 * get a tag.
 */
typedef struct xchar_type xchar_type;
struct xchar_type
{
	const char *tag;
	byte c;
};

/*
 * x-char.c function declarations are located in externs.h.
 * x-char-c tables are loctaed in tables.c
 */


#endif /* INCLUDED_X_CHAR_H */

>>>>>>> Remove unused is_xxx() and to_xxx() macros from x-char.h
