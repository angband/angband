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
 * Hook to convert a wide character (in whatever encoding the platform uses)
 * back to a multibyte representation using UTF-8.  Expected to behave like
 * wctomb().
 */
int (*text_wctomb_hook)(char *s, wchar_t wchar) = NULL;

/**
 * Hook to get the maximum number of bytes needed to store a wide character
 * converted to a multibyte representation using UTF-8.
 */
int (*text_wcsz_hook)(void) = NULL;

/**
 * Hook to test whether a given wide character is printable.
 */
int (*text_iswprint_hook)(wint_t wc) = NULL;

/**
 * Count the number of characters in a UTF-8 encoded string
 *
 * Taken from http://canonical.org/~kragen/strlen-utf8.html
 */
size_t utf8_strlen(const char *s)
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
 * Advance a pointer to a UTF-8 buffer by a given number of Unicode code points.
 * \param s Is the pointer to advance.
 * \param n Is the number of code points to skip over.
 * \param lim If not NULL, is the limit for how far to advance.
 * \return If s could be advanced by n code points before reaching lim (if
 * lim is not NULL) or before reaching the end of the input, the returned
 * value is the incremented pointer.  Otherwise, the returned value is NULL.
 * If n is zero, the return value may be different than s if s does not
 * point to the start of a code point.  In that case, the returned value will
 * point to the start of the next code point.
 */
char *utf8_fskip(char *s, size_t n, char *lim)
{
	while (1) {
		if (*s == 0) {
			/* Reached the end of the input. */
			return (n > 0) ? NULL : s;
		}
		if ((*s & 0xc0) != 0x80) {
			/* It's not marked as a continuation byte. */
			if (n == 0) {
				return s;
			}
			--n;
		}
		if (s == lim) {
			return NULL;
		}
		++s;
	}
}

/**
 * Decrement a pointer to a UTF-8 buffer by a given number of Unicode code
 * points.
 * \param s Is the pointer to decrement.
 * \param n Is the number of code points to skip over.
 * \param lim Is the limit for how far to backtrack.  Must not be NULL.
 * \return If s could be decremented by n code points before reaching lim,
 * the returned value is the decremented pointer.  Otherwise, the returned
 * value is NULL.  If n is zero, the return value may be different than s
 * if s does not point to the start of a code point.  In that case, the
 * returned valued will be the start of the first code point prior to s or
 * NULL, if the start of a code point could not be found before reaching lim.
 */
char *utf8_rskip(char *s, size_t n, char *lim)
{
	while (1) {
		if ((*s & 0xc0) != 0x80) {
			/* It's not marked as a continuation byte. */
			if (n == 0) {
				return s;
			}
			--n;
		}
		if (s == lim) {
			return NULL;
		}
		--s;
	}
}

/**
 * Convert a sequence of UTF-32 values, in the native byte order, to UTF-8.
 * \param out Is the pointer to the buffer to hold the conversion.
 * \param n_out Is the number of char-sized units that can be placed in out.
 * \param in Is the pointer to the sequence to convert.
 * \param n_in Is the maximum number of values to convert from in.  Conversion
 * will terminate before that if the sequence to convert contains a zero or
 * something that is not a valid Unicode code point (either larger than
 * 0x10FFFF or in the range of 0xD800 to 0xDFFF reserved for surrogate pairs)
 * or if the next value to convert would cause the output buffer to overflow.
 * \param pn_cnvt If not NULL, *pn_cnvt will be set to the number of UTF-32
 * values converted.
 * \return The returned value is the number of char-sized units, excluding
 * the terminating null character, written to out.  The returned value will
 * be less than n_out if n_out is greater than zero.
 */
size_t utf32_to_utf8(char *out, size_t n_out, const uint32_t *in, size_t n_in,
	size_t *pn_cnvt)
{
	size_t nwritten = 0;
	const uint32_t *in_orig = in;
	const uint32_t *in_lim = in + n_in;

	while (1) {
		if (in == in_lim) {
			break;
		}
		if (*in <= 0x7f) {
			/* Encoded as single byte. */
			if (*in == 0) {
				break;
			}
			if (n_out <= 1) {
				break;
			}
			out[nwritten++] = (char) *in;
			--n_out;
		} else if (*in <= 0x7ff) {
			/* Encoded as two bytes. */
			if (n_out <= 2) {
				break;
			}
			out[nwritten++] = 0xc0 + ((*in & 0x7c0) >> 6);
			out[nwritten++] = 0x80 + (*in & 0x3f);
			n_out -= 2;
		} else if (*in <= 0xffff) {
			/* Encoded as three bytes. */
			if (*in >= 0xd800 && *in <= 0xdfff) {
				/*
				 * Those are reserved for UTF-16 surrogate
				 * pairs and should not be encoded.
				 */
				break;
			}
			if (n_out <= 3) {
				break;
			}
			out[nwritten++] = 0xe0 + ((*in & 0xf000) >> 12);
			out[nwritten++] = 0x80 + ((*in & 0xfc0) >> 6);
			out[nwritten++] = 0x80 + (*in & 0x3f);
			n_out -= 3;
		} else if (*in <= 0x10ffff) {
			/*
			 * Encoded as four bytes.  The upper limit of 0x10ffff
			 * is imposed by the limits of UTF-16.  Without that,
			 * the four byte encoding can handle values up to
			 * 0x1fffff.
			 */
			if (n_out <= 4) {
				break;
			}
			out[nwritten++] = 0xf0 + ((*in & 0x1c0000) >> 18);
			out[nwritten++] = 0x80 + ((*in & 0x3f000) >> 12);
			out[nwritten++] = 0x80 + ((*in & 0xfc0) >> 6);
			out[nwritten++] = 0x80 + (*in & 0x3f);
			n_out -= 4;
		} else {
			break;
		}
		++in;
	}
	if (n_out > 0) {
		out[nwritten] = 0;
	}
	if (pn_cnvt) {
		*pn_cnvt = in - in_orig;
	}
	return nwritten;
}

/**
 * Return whether a given UTF-32 value corresponds to a printable character.
 *
 * The similar standard library functions are isprint() and iswprint().
 * Choose not to use those because both depend on the locale and only want
 * to use the locale when converting a keyboard event to a keycode and when
 * converting internally stored text to a final form for display.  Between
 * those two points, use fixed encodings:  UTF-32 for single keycodes and
 * UTF-8 for bulk storage of text.  Also, isprint() is, in general, limited
 * to distinguishing 8 bits, and the wchar_t for iswprint() is, at least on
 * Windows, a 16-bit type.
 */
bool utf32_isprint(uint32_t v)
{
	/* Switch based on the plane (each plane has 2^16 code points). */
        switch ((v & 0xff0000) >> 16) {
        case 0:
                /* Is the basic multilingual plane.  Most things are here. */
                switch ((v & 0xff00) >> 8) {
                case 0:
                        /*
                         * C0 control characters, DEL, and C1 controls are
                         * not printable.
                         */
                        if (v <= 0x1f || (v >= 0x7f && v <= 0x9f)) {
                                return false;
                        }
                        break;

                case 0xd8:
                case 0xd9:
                case 0xda:
                case 0xdb:
                case 0xdc:
                case 0xdd:
                case 0xde:
                case 0xdf:
                        /* Used for surrogate pairs in UTF-16. */
                        return false;

                case 0xfd:
                        /*
                         * Part of the arabic presentation forms-a block is
                         * guaranteed to not be used for characters.
                         */
                        if (v >= 0xfdd0 && v <= 0xfdef) {
                                return false;
                        }
                        break;

                case 0xfe:
                        /*
                         * The variation selectors indicate how to present a
                         * preceding character.  Treat as not printable.
                         * Also exclude the byte-order mark, 0xfeff.
                         */
                        if ((v & 0xfff0) == 0xfe00 || v == 0xfeff) {
                                return false;
                        }
                        break;

                case 0xff:
                        /* Interlinear annotation marks are not printable. */
                        if (v >= 0xfff9 && v <= 0xfffb) {
                                return false;
                        }
                        break;

                default:
                        /* Do no special casing for the rest. */
                        break;
                }
                break;

        case 1:
                /* Is the supplemental multilingual plane. */
        case 2:
                /* Is the supplmental ideographic plane. */
        case 3:
                /* Is the tertiary ideographic plane. */
                /* Assume no no special casing for those planes. */
                break;

        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
                /*
                 * Those planes are currently unassigned.  Assume unprintable.
                 */
                return false;

        case 14:
                /*
                 * Is the supplemental special-purpose plane.  Used for tags
                 * to modify a preceding character or to indicate a variant
                 * form of an ideograph.  Assume all are unprintable.
                 */
                return false;

        case 15:
        case 16:
                /*
                 * These are private use planes.  Assume that no special
                 * cases are necessary.
                 */
                break;

        default:
                /* Is not a valid Unicode code point. */
                return false;
        }

        /*
         * Assume printable unless it is xxfffe or xxffff which are guaranteed
         * to not be characters (i.e. the code points in the basic multilingual
         * plane used as byte-order marks).
         */
        return ((v & 0xfffe) != 0xfffe);
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
 * Rewrite string s in-place, replacing encoded representations of escaped characters
 * ("\\r", etc.) with their literal character counterparts.
 * This does only handle escape sequences visible on the ascii manpage (and "\e").
 */
void strunescape(char *s) {
	char *in = s;
	char *out = s;
	bool unescapenext = false;

	while (*in) {
		if (unescapenext) {
			unescapenext = false;

			switch (*in) {
			case '0':
				*out++ = '\0';
				break;
			case 'a':
				*out++ = '\a';
				break;
			case 'b':
				*out++ = '\b';
				break;
			case 't':
				*out++ = '\t';
				break;
			case 'n':
				*out++ = '\n';
				break;
			case 'v':
				*out++ = '\v';
				break;
			case 'f':
				*out++ = '\f';
				break;
			case 'r':
				*out++ = '\r';
				break;
			case '\\':
				*out++ = '\\';
				break;
			case 'e':
				*out++ = '\x1B';
				break;
			default:
				/* Add back the unmodified sequence */
				*out++ = '\\';
				*out++ = *in;
				break;
			}

			in++;
			continue;
		}

		if (*in == '\\') {
			unescapenext = true;
			in++;
			continue;
		}

		*out++ = *in++;
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
 * Convert a wide character to a multibyte representation.
 * \param s Points to a buffer to hold the converted result.  That buffer must
 * have at least text_wcsz() bytes.  With the exception of the case where wchar
 * is 0, the contents written to the buffer will not be null terminated.
 * \param wchar Is the wide character to convert.
 * \return The returned value is the number of bytes in the converted character
 * or -1 if the character could not be recognized and converted.
 */
int text_wctomb(char *s, wchar_t wchar)
{
	return (text_wctomb_hook) ?
		(*text_wctomb_hook)(s, wchar) : wctomb(s, wchar);
}

/**
 * Get the maximum size to store a wide character converted to multibyte.
 */
int text_wcsz(void)
{
	return (text_wcsz_hook) ? (*text_wcsz_hook)() : MB_LEN_MAX;
}

/**
 * Return whether the given wide character is printable.
 */
int text_iswprint(wint_t wc)
{
	return (text_iswprint_hook) ? (*text_iswprint_hook)(wc) : iswprint(wc);
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
int mean(const int *nums, int size)
{
	int i, total = 0;

	for(i = 0; i < size; i++) total += nums[i];

	return total / size;
}

/**
 * Variance of the first 'size' entries of the array 'nums'
 */
int variance(const int *nums, int size)
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

uint32_t djb2_hash(const char *str)
{
	uint32_t hash = 5381;
	int c = *str;

	while (c)
	{
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		c = *++str;
	}

	return hash;
}

