/**
 * \file z-util.h
 * \brief Low-level string handling and other utilities.
 *
 * Copyright (c) 1997-2005 Ben Harrison, Robert Ruehlmann.
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_Z_UTIL_H
#define INCLUDED_Z_UTIL_H

#include "h-basic.h"


/**
 * ------------------------------------------------------------------------
 * Available variables
 * ------------------------------------------------------------------------ */


/**
 * The name of the program.
 */
extern char *argv0;


/**
 * Aux functions
 */
extern size_t (*text_mbcs_hook)(wchar_t *dest, const char *src, int n);
extern int (*text_wctomb_hook)(char *s, wchar_t wchar);
extern int (*text_wcsz_hook)(void);
extern int (*text_iswprint_hook)(wint_t wc);
extern void (*plog_aux)(const char *);
extern void (*quit_aux)(const char *);


/**
 * ------------------------------------------------------------------------
 * Available Functions
 * ------------------------------------------------------------------------ */


/**
 * Return "s" (or not) depending on whether n is singular.
 */
#define PLURAL(n)		((n) == 1 ? "" : "s")

/**
 * Return the verb form matching the given count
 */
#define VERB_AGREEMENT(count, singular, plural)    (((count) == 1) ? (singular) : (plural))


/**
 * Count the number of characters in a UTF-8 encoded string
 */
size_t utf8_strlen(const char *s);

/**
 * Clip a null-terminated UTF-8 string 's' to 'n' unicode characters.
 * e.g. utf8_clipto("example", 4) will clip after 'm', resulting in 'exam'.
 */
void utf8_clipto(char *s, size_t n);

/**
 * Advance a pointer to a UTF-8 buffer by a given number of Unicode code points.
 */
char *utf8_fskip(char *s, size_t n, char *lim);

/**
 * Decrement a pointer to a UTF-8 buffer by a given number of Unicode code
 * points.
 */
char *utf8_rskip(char *s, size_t n, char *lim);

/**
 * Convert a sequence of UTF-32 values, in the native byte order, to UTF-8.
 */
size_t utf32_to_utf8(char *out, size_t n_out, const uint32_t *in, size_t n_in,
	size_t *pn_cnvt);

/**
 * Return whether a given UTF-32 value corresponds to a printable character.
 */
bool utf32_isprint(uint32_t v);

/**
 * Case insensitive comparison between two strings
 */
extern int my_stricmp(const char *s1, const char *s2);

/**
 * Case insensitive comparison between two strings, up to n characters long.
 */
extern int my_strnicmp(const char *a, const char *b, int n);

/**
 * Case-insensitive strstr
 */
extern char *my_stristr(const char *string, const char *pattern);

/**
 * Copy up to 'bufsize'-1 characters from 'src' to 'buf' and NULL-terminate
 * the result.  The 'buf' and 'src' strings may not overlap.
 *
 * Returns: strlen(src).  This makes checking for truncation
 * easy.  Example:
 *   if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
extern size_t my_strcpy(char *buf, const char *src, size_t bufsize);

/**
 * Try to append a string to an existing NULL-terminated string, never writing
 * more characters into the buffer than indicated by 'bufsize', and
 * NULL-terminating the buffer.  The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src).  This makes checking for
 * truncation easy.  Example:
 *   if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
extern size_t my_strcat(char *buf, const char *src, size_t bufsize);

/**
 * Capitalise string 'buf'
 */
void my_strcap(char *buf);

/**
 * Test equality, prefix, suffix
 */
extern bool streq(const char *s, const char *t);
extern bool prefix(const char *s, const char *t);
extern bool prefix_i(const char *s, const char *t);
extern bool suffix(const char *s, const char *t);

#define streq(s, t)		(!strcmp(s, t))

/**
 * Skip occurrences of a characters
 */
extern void strskip(char *s, const char c, const char e);
extern void strescape(char *s, const char c);

/**
 * Change escaped characters into their literal representation
 */
extern void strunescape(char *s);

/**
 * Determines if a string is "empty"
 */
bool contains_only_spaces(const char* s);

/**
 * Check if a char is a vowel
 */
bool is_a_vowel(int ch);


/**
 * Allow override of the multi-byte to wide char conversion
 */
size_t text_mbstowcs(wchar_t *dest, const char *src, int n);

/**
 * Convert a wide character to multibyte representation.
 */
int text_wctomb(char *s, wchar_t wchar);

/**
 * Get the maximum size to store a wide character converted to multibyte.
 */
int text_wcsz(void);

/**
 * Return whether the given wide character is printable.
 */
int text_iswprint(wint_t wc);

/**
 * Print an error message
 */
extern void plog(const char *str);

/**
 * Exit, with optional message
 */
extern void quit(const char *str);


/**
 * Sorting functions
 */
extern void sort(void *array, size_t nmemb, size_t smemb,
		 int (*comp)(const void *a, const void *b));

/**
 * Create a hash for a string
 */
uint32_t djb2_hash(const char *str);

/**
 * Mathematical functions
 */
int mean(const int *nums, int size);
int variance(const int *nums, int size);

#endif /* INCLUDED_Z_UTIL_H */
