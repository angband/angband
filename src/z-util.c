/**
 * \file z-util.c
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

#include <stdlib.h>

#include "z-util.h"

/**
 * Convenient storage of the program name
 */
char *argv0 = NULL;

/**
 * Hook for platform-specific wide character handling
 */
size_t (*text_mbcs_hook)(wchar_t *dest, const char *src, int n) = NULL;

/**
 * Count the number of characters in a UTF-8 encoded string
 *
 * Taken from http://canonical.org/~kragen/strlen-utf8.html
 */
size_t utf8_strlen(char *s)
{
	size_t i = 0, j = 0;
	while (s[i]) {
		if ((s[i] & 0xc0) != 0x80) j++;
		i++;
	}
	return j;
}

/**
 * Clip a null-terminated UTF-8 string 's' to 'n' unicode characters.
 * e.g. utf8_clipto("example", 4) will clip after 'm', resulting in 'exam'.
 */
void utf8_clipto(char *s, size_t n)
{
	size_t i = 0, j = 0;
	bool terminate_next = false;

	if (n == 0) {
		s[i] = 0;
		return;
	}

	while (s[i]) {
		if ((s[i] & 0xc0) != 0x80) {
			j++;
			if (terminate_next)
				break;
			if (j == n)
				terminate_next = true;
		}
		i++;
	}
	s[i] = 0;
}

/**
 * Case insensitive comparison between two strings
 */
int my_stricmp(const char *s1, const char *s2)
{
	char ch1 = 0;
	char ch2 = 0;

	/* Just loop */
	while (true) {
		/* We've reached the end of both strings simultaneously */
		if ((*s1 == 0) && (*s2 == 0)) {
			/* We're still here, so s1 and s2 are equal */
			return (0);
		}

		ch1 = toupper((unsigned char) *s1);
		ch2 = toupper((unsigned char) *s2);

		/* If the characters don't match */
		if (ch1 != ch2) {
			/* return the difference between them */
			return ((int)(ch1 - ch2));
		}

		/* Step on through both strings */
		s1++;
		s2++;
	}
}


/**
 * Case insensitive comparison between the first n characters of two strings
 */
int my_strnicmp(const char *a, const char *b, int n)
{
	const char *s1, *s2;
	char z1, z2;

	/* Scan the strings */
	for (s1 = a, s2 = b; n > 0; s1++, s2++, n--) {
		z1 = toupper((unsigned char)*s1);
		z2 = toupper((unsigned char)*s2);
		if (z1 < z2) return (-1);
		if (z1 > z2) return (1);
		if (!z1) return (0);
	}

	return 0;
}

/**
 * An ANSI version of strstr() with case insensitivity.
 *
 * In the public domain; found at:
 *    http://c.snippets.org/code/stristr.c
 */
char *my_stristr(const char *string, const char *pattern)
{
	const char *pptr, *sptr;
	char *start;

	for (start = (char *)string; *start != 0; start++) {
		/* Find start of pattern in string */
		for ( ; ((*start != 0) && (toupper((unsigned char)*start) != toupper((unsigned char)*pattern))); start++)
			;
		if (*start == 0)
			return NULL;

		pptr = (const char *)pattern;
		sptr = (const char *)start;

		while (toupper((unsigned char)*sptr) == toupper((unsigned char)*pptr)) {
			sptr++;
			pptr++;

			/* If end of pattern then pattern was found */
			if (*pptr == 0)
				return (start);
		}
	}

	return NULL;
}


/**
 * The my_strcpy() function copies up to 'bufsize'-1 characters from 'src'
 * to 'buf' and NUL-terminates the result.  The 'buf' and 'src' strings may
 * not overlap.
 *
 * my_strcpy() returns strlen(src).  This makes checking for truncation
 * easy.  Example: if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
size_t my_strcpy(char *buf, const char *src, size_t bufsize)
{
	size_t len = strlen(src);
	size_t ret = len;

	/* Paranoia */
	if (bufsize == 0) return ret;

	/* Truncate */
	if (len >= bufsize) len = bufsize - 1;

	/* Copy the string and terminate it */
	(void)memcpy(buf, src, len);
	buf[len] = '\0';

	/* Return strlen(src) */
	return ret;
}


/**
 * The my_strcat() tries to append a string to an existing NUL-terminated
 * string.
 * It never writes more characters into the buffer than indicated by 'bufsize'
 * and NUL-terminates the buffer.  The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src).  This makes checking for
 * truncation easy.  Example:
 * if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
size_t my_strcat(char *buf, const char *src, size_t bufsize)
{
	size_t dlen = strlen(buf);

	/* Is there room left in the buffer? */
	if (dlen + 1 < bufsize) {
		/* Append as much as possible  */
		return (dlen + my_strcpy(buf + dlen, src, bufsize - dlen));
	} else {
		/* Return without appending */
		return (dlen + strlen(src));
	}
}

/**
 * Capitalise the first letter of string 'str'.
 */
void my_strcap(char *buf)
{
	if (buf && buf[0])
		buf[0] = toupper((unsigned char) buf[0]);
}


/**
 * Determine if string "a" is equal to string "b"
 */
#undef streq
bool streq(const char *a, const char *b)
{
	return (!strcmp(a, b));
}


/**
 * Determine if string "t" is a suffix of string "s"
 */
bool suffix(const char *s, const char *t)
{
	size_t tlen = strlen(t);
	size_t slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (false);

	/* Compare "t" to the end of "s" */
	return (!strcmp(s + slen - tlen, t));
}


/**
 * Determine if string "t" is a prefix of string "s"
 */
bool prefix(const char *s, const char *t)
{
	/* Scan "t" */
	while (*t)
	{
		/* Compare content and length */
		if (*t++ != *s++) return (false);
	}

	/* Matched, we have a prefix */
	return (true);
}


/**
 * Determine if string "t" is a prefix of string "s" - case insensitive.
 */
bool prefix_i(const char *s, const char *t)
{
	/* Scan "t" */
	while (*t)
	{
		if (toupper((unsigned char)*t) != toupper((unsigned char)*s))
			return (false);
		else
		{
			t++;
			s++;
		}
	}

	/* Matched, we have a prefix */
	return (true);
}

/**
 * Rewrite string s in-place "skipping" every occurrence of character c except
 * those preceded by character e
 */
void strskip(char *s, const char c, const char e) {
	char *in = s;
	char *out = s;
	bool escapeseen = false;
	while (*in) {
		if ((*in != c) && ((*in != e) || escapeseen)) {
			if (escapeseen) {
				/* Not escaping anything */
				*out = e;
				out++;
			}
			*out = *in;
			out++;
			escapeseen = false;
		} else if (*in == e) {
			/* Maybe escaping something */
			escapeseen = true;
		} else if (escapeseen) {
			/* Add the escaped character */
			*out = *in;
			out++;
			escapeseen = false;
		}
		in++;
	}
	*out = 0;
}

/**
 * Rewrite string s in-place removing escape character c
 * note that pairs of c will leave one instance of c in out
 */
void strescape(char *s, const char c) {
	char *in = s;
	char *out = s;
	bool escapenext = false;
	while (*in) {
		if (*in != c || escapenext) {
			*out = *in;
			out++;
			escapenext = false;
		} else if (*in == c) {
			escapenext = true;
		}
		in++;
	}
	*out = 0;
}

/**
 * returns true if string only contains spaces
 */
bool contains_only_spaces(const char* s){
	const char spaces[]=" \t";
	size_t nsp = strspn(s, spaces);

	return s[nsp] == '\0';
}

/**
 * Check a char for "vowel-hood"
 */
bool is_a_vowel(int ch)
{
	switch (tolower((unsigned char) ch))
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		{
			return (true);
		}
	}

	return (false);
}

/**
 * Allow override of the multi-byte to wide char conversion
 */
size_t text_mbstowcs(wchar_t *dest, const char *src, int n)
{
	if (text_mbcs_hook)
		return (*text_mbcs_hook)(dest, src, n);
	else
		return mbstowcs(dest, src, n);
}

/**
 * Redefinable "plog" action
 */
void (*plog_aux)(const char *) = NULL;

/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(const char *str)
{
	/* Use the "alternative" function if possible */
	if (plog_aux) (*plog_aux)(str);

	/* Just do a labeled fprintf to stderr */
	else (void)(fprintf(stderr, "%s: %s\n", argv0 ? argv0 : "?", str));
}



/**
 * Redefinable "quit" action
 */
void (*quit_aux)(const char *) = NULL;

/**
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(EXIT_SUCCESS)".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(const char *str)
{
	/* Attempt to use the aux function */
	if (quit_aux) (*quit_aux)(str);

	/* Success */
	if (!str) exit(EXIT_SUCCESS);

	/* Send the string to plog() */
	plog(str);

	/* Failure */
	exit(EXIT_FAILURE);
}

/**
 * Arithmetic mean of the first 'size' entries of the array 'nums'
 */
int mean(int *nums, int size)
{
	int i, total = 0;

	for(i = 0; i < size; i++) total += nums[i];

	return total / size;
}

/**
 * Variance of the first 'size' entries of the array 'nums'
 */
int variance(int *nums, int size)
{
	int i, avg, total = 0;

	avg = mean(nums, size);

	for(i = 0; i < size; i++)
	{
		int delta = nums[i] - avg;
		total += delta * delta;
	}

	return total / size;
}

void sort(void *base, size_t nmemb, size_t smemb,
	  int (*comp)(const void *, const void *))
{
	qsort(base, nmemb, smemb, comp);
}

u32b djb2_hash(const char *str)
{
	u32b hash = 5381;
	int c = *str;

	while (c)
	{
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		c = *++str;
	}

	return hash;
}

