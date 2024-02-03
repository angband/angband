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
 * Hook to search a wide character in a wide character string.
 */
wchar_t *(*text_wcschr_hook)(const wchar_t *wcs, wchar_t wc) = NULL;

/**
 * Hook to determine the length of a wide-character string.
 */
size_t (*text_wcslen_hook)(const wchar_t *s) = NULL;

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
 * Determine if string "t" is a suffix of string "s" - case insensitive
 */
bool suffix_i(const char *s, const char *t)
{
	size_t tlen = strlen(t);
	size_t slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (false);

	/* Compare "t" to the end of "s" */
	return !my_stricmp(s + slen - tlen, t);
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
 * Gives the integer value of a hexadecimal character
 * Returns -1 if invalid
 */
static int hex_char_to_int(char c) {
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	return -1;
}

/**
 * Gives the integer value of a hexadecimal string
 * hex_str_to_int("4A") returns 74 == 0x4A
 * Returns -1 if invalid
 */
int hex_str_to_int(const char *s) {
	int result = 0;
	while (*s) {
		int current = hex_char_to_int(*s);
		if (current == -1)
			return -1;
		result *= 16;
		result += current;
		++s;
	}
	return result;
}

/**
 * Rewrite string s in-place, replacing encoded representations of escaped characters
 * ("\\r", etc.) with their literal character counterparts.
 * This does only handle escape sequences visible on the ascii manpage (and "\e" and "\x").
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
			case 'x': {
				char hex[3];
				if (*++in == 0) {
					/* Add back the unmodified sequence */
					*out++ = '\\';
					*out++ = 'x';
					continue;
				}
				hex[0] = *in;
				if (*++in == 0) {
					/* Add back the unmodified sequence */
					*out++ = '\\';
					*out++ = 'x';
					*out++ = hex[0];
					continue;
				}
				hex[1] = *in;
				hex[2] = 0;
				int result = hex_str_to_int(hex);
				if (result == -1) {
					/* Add back the unmodified sequence */
					*out++ = '\\';
					*out++ = 'x';
					*out++ = hex[0];
					*out++ = hex[1];
				}
				*out++ = result;
				break;
				}
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
 * Return pointer to the first occurrence of wc in the wide-character
 * string pointed to by wcs, or NULL if wc does not occur in the
 * string.
 */
wchar_t *text_wcschr(const wchar_t *wcs, wchar_t wc)
{
	return (text_wcschr_hook) ?
		(*text_wcschr_hook)(wcs, wc) : wcschr(wcs, wc);
}

/**
 * Return the number of wide characters in s.
 */
size_t text_wcslen(const wchar_t *s)
{
	return (text_wcslen_hook) ? (*text_wcslen_hook)(s) : wcslen(s);
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
 * Return a + b coreced to be in [INT_MIN, INT_MAX] if it would overflow.
 */
int add_guardi(int a, int b)
{
	if (a < 0) {
		return (b >= 0 || (b > INT_MIN && a >= INT_MIN - b)) ?
			a + b : INT_MIN;
	}
	return (b <= 0 || a <= INT_MAX - b) ? a + b : INT_MAX;
}

/**
 * Return a - b coerced to be in [INT_MIN, INT_MAX] if it would overflow.
 */
int sub_guardi(int a, int b)
{
	if (a < 0) {
		return (b <= 0 || a >= INT_MIN + b) ? a - b : INT_MIN;
	}
	return (b >= 0 || a <= INT_MAX + b) ? a - b : INT_MAX;
}

/**
 * Return a + b coreced to be in [-32768, 32767] if it would overflow.
 */
int add_guardi16(int16_t a, int16_t b)
{
	if (a < 0) {
		return (b >= 0 || (b > -32768 && a >= -32768 - b)) ?
			a + b : -32768;
	}
	return (b <= 0 || a <= 32767 - b) ? a + b : 32767;
}

/**
 * Return a - b coerced to be in [-32768, 32767] if it would overflow.
 */
int sub_guardi16(int16_t a, int16_t b)
{
	if (a < 0) {
		return (b <= 0 || a >= -32768 + b) ? a - b : -32768;
	}
	return (b >= 0 || a <= 32767 + b) ? a - b : 32767;
}

/**
 * Initialize a multiprecision integer from an unsigned int.
 *
 * \param r points to n uint16_t values to store the result.
 * \param n is the number of digits in base 2^16 in r.
 * \param a is the unsigned value to copy.
 * \return true if a is greater than or equal to 2^(16*n) or false if a is
 * less than 2^(16*n) (i.e. fits in r without overflow).
 *
 * Could use the GNU multiprecision library or something similar for this.  Do
 * this instead to avoid the extra library dependency.
 */
static bool ini_u16n(uint16_t *r, size_t n, unsigned int a)
{
	const unsigned int mask = (1U << 16) - 1;
	size_t i = n;

	while (i > 0) {
		--i;
		r[i] = (uint16_t)(a & mask);
		a >>= 16;
	}
	return a != 0;
}

/**
 * Set all digits of a multiprecision integer to zero.
 * \param r points to n uint16_t digits.
 * \param n is the number of digits in base 2^16.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static void zer_u16n(uint16_t *r, size_t n)
{
	size_t i;

	for (i = 0; i < n; ++i) {
		r[i] = 0;
	}
}

/**
 * Extract the least significant CHAR_BIT * sizeof(unsigned int) bits of a
 * multiprecision integer into an unsigned int.
 * \param a points to the n uint16_t digits of the value.
 * \param n is the number of digits in base 2^16.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static unsigned int ext_u16n(const uint16_t *a, size_t n)
{
	unsigned int result = 0;
	size_t i = n;
	size_t rb = sizeof(unsigned int) * CHAR_BIT;
	size_t shift = 0;

	while (1) {
		if (i == 0 || rb == 0) {
			break;
		}
		--i;
		if (rb < 16) {
			result += (a[i] & ((1U << rb) - 1)) << shift;
			break;
		}
		result += (unsigned int)a[i] << shift;
		rb -= 16;
		shift += 16;
	}
	return result;
}

/**
 * Return the most significant nonzero bit in a multiprecision integer.
 * \param a points to the n uint16_t digits of the value
 * \param n is the number of digits in base 2^16.
 * \return the 1-based index of the most significant nonzero bit (1 is
 * the least significant bit) or zero if the value is zero.
 */
static size_t msb_u16n(const uint16_t *a, size_t n)
{
	size_t i = 0;

	while (1) {
		if (i == n) {
			return 0;
		}
		if (a[i]) {
			unsigned int lo = 0, hi = 16;

			while (lo < hi - 1) {
				/*
				 * Is a bit set in the upper part of the range?
				 */
				unsigned int half = (hi - lo + 1) / 2;
				uint16_t mask = (((uint16_t)1 << half) - 1)
					<< (hi - half);

				if (a[i] & mask) {
					lo = hi - half;
				} else {
					hi = hi - half;
				}
			}
			return lo + 1 + 16 * ((n - 1) - i);
		}
		++i;
	}
}

/**
 * Compare the multiprecision integers a and b.
 *
 * \param a points to na uint16_t digits.
 * \param b points to nb uint16_t digits.
 * \param na is the number of digits in base 2^16 for a.
 * \param nb is the number of digits in base 2^16 for b.
 * \return 1 if a is greater than b, 0 if a equals b, and -1 if a is less
 * than b.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static int cmp_u16n(const uint16_t *a, const uint16_t *b, size_t na, size_t nb)
{
	size_t ia = 0, ib = 0;

	if (na >= nb) {
		while (ia < na - nb) {
			if (a[ia]) {
				return 1;
			}
			++ia;
		}
	} else {
		while (ib < nb - na) {
			if (b[ib]) {
				return -1;
			}
			++ib;
		}
	}
	while (ia < na) {
		assert(ib < nb);
		if (a[ia] > b[ib]) {
			return 1;
		}
		if (a[ia] < b[ib]) {
			return -1;
		}
		++ia;
		++ib;
	}
	return 0;
}

/**
 * Add b to a returning the result in a.
 *
 * \param a points to na uint16_t digits.
 * \param b points to nb uint16_t digits.  b can overlap with a if b's digits
 * are from the end of a and end with the least significant digit of a.
 * nb.  Otherwise, a and b must not overlap.
 * \param na is the number of digits in base 2^16 for a.
 * \param nb is the number of digits in base 2^16 for b must be less than or
 * equal to na.
 * \return the amount of overflow in the result.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static uint16_t addip_u16n(uint16_t *a, const uint16_t *b, size_t na, size_t nb)
{
	const uint32_t mask = ((uint32_t)1 << 16) - 1;
	uint16_t carry = 0;
	size_t ia = na, ib = nb;

	while (ib > 0) {
		uint32_t t;

		assert(ia);
		--ia;
		--ib;
		t = (uint32_t)a[ia] + (uint32_t)b[ib] + (uint32_t)carry;
		a[ia] = t & mask;
		carry = (uint16_t)(t >> 16);
	}
	while (ia > 0 && carry) {
		uint32_t t;

		--ia;
		t = (uint32_t)a[ia] + (uint32_t)carry;
		a[ia] = t & mask;
		carry = (uint16_t)(t >> 16);
	}
	return carry;
}

/**
 * Subtract b from a returning the result in a.
 *
 * \param a points to na uint16_t digits.
 * \param b points to nb uint16_t digits.  cmp_u16n(a, b) must be greater than
 * or equal to zero.  b can be the same as a if na equals nb.  Otherwise, a and
 * b must not overlap.
 * \param na is the number of digits in base 2^16 for a.
 * \param nb is the number of digits in base 2^16 for b.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static void subip_u16n(uint16_t *a, const uint16_t *b, size_t na, size_t nb)
{
	bool carry = false;
	size_t ia = na, ib = nb;

	while (ib > 0 && ia > 0) {
		--ia;
		--ib;
		if (carry) {
			if (a[ia] > b[ib]) {
				a[ia] -= b[ib] + 1;
				carry = false;
			} else {
				a[ia] += (uint16_t)65535 - b[ib];
				carry = true;
			}
		} else if (a[ia] >= b[ib]) {
			a[ia] -= b[ib];
		} else {
			a[ia] += (uint16_t)65535 - (b[ib] - 1);
			carry = true;
		}
	}
#ifndef NDEBUG
	while (ib > 0) {
		--ib;
		assert(!b[ib]);
	}
#endif
	while (ia > 0 && carry) {
		--ia;
		if (a[ia]) {
			a[ia] -= 1;
			carry = false;
		} else {
			a[ia] = 65535;
		}
	}
	assert(!carry);
}


/**
 * Multiply two multiprecision integers.  Both have the most significant digit
 * first.
 * \param r points to nr uint16_t digits to store the result.  Can not overlap
 * with a or b.
 * \param nr is the number of digits for r in base 2^16.
 * \param a points to the na uint16_t digits for one of the values to multiply.
 * \param b points to the nb uint16_t digits for one of the values to multiply.
 * \param na is the number of digits for a in base 2^16.
 * \param nb is the number of digits for b in base 2^16.
 * \return true if the result is larger than can be stored in r or false if
 * the result fits in r without overflow.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static bool mul_u16n(uint16_t *r, const uint16_t *a, const uint16_t *b,
		size_t nr, size_t na, size_t nb)
{
	const uint32_t mask = ((uint32_t)1 << 16) - 1;
	size_t ia = na;
	bool over = false;

	zer_u16n(r, nr);
	while (ia > 0) {
		size_t ib = nb;

		--ia;
		while (ib > 0) {
			size_t rir = (na - 1 - ia) + (nb - ib), ir;
			uint32_t p, carry, t;

			if (rir >= nr) {
				over = true;
				break;
			}
			--ib;
			ir = (nr - rir) - 1;
			p = (uint32_t)a[ia] * (uint32_t)b[ib];
			carry = p >> 16;
			t = (uint32_t)r[ir] + (p & mask);
			r[ir] = t & mask;
			carry += t >> 16;
			while (carry > 0) {
				if (ir == 0) {
					over = true;
					break;
				}
				--ir;
				t = (uint32_t)r[ir] + carry;
				r[ir] = t & mask;
				carry = t >> 16;
			}
		}
	}
	return over;
}

/**
 * Divide two multiprecision integer.  Both have the most significant digit
 * first.
 * \param q points to nq uint16_t digits to store the quotient.  Can not
 * overlap with r, w, or den.
 * \param r points to n uint16_t digits to store the remainder.  Can not
 * overlap with q, r, w, num, or den.
 * \param w points nd + 1 uint16_t digits of working space.  Can not overlap
 * with q, r, num, or den.
 * \param num points to the n uint16_t digits of the integer to be divided.
 * \param den points to the nd uint16_t digits of the divisor.
 * \param nq is the number of base 2^16 digits in q.
 * \param n is the number of base 2^16 digits in r and num.
 * \param nd is the number of base 2^16 digits in den.
 * \return zero if the divisor is greater than zero and the quotient could
 * be stored without overflow, one if the divisor is greater than zero and
 * only the least significant bits of the quotient could be stored, or two
 * if the divisor is zero.  If the divisor is zero, the contents or q and r
 * are not modified.
 *
 * The rationale for this is the same as for ini_u16n.
 */
static int div_u16n(uint16_t *q, uint16_t *r, uint16_t *w, const uint16_t *num,
		const uint16_t *den, size_t nq, size_t n, size_t nd)
{
	size_t msb_d = msb_u16n(den, nd), msb_r;
	size_t nqu, ir, iqr, iqrb;

	if (msb_d == 0) {
		return 2;
	}
	msb_r = msb_u16n(num, n);
	if (msb_r > msb_d) {
		nqu = ((msb_r - msb_d) + 15) / 16;
	} else {
		nqu = 1;
	}

	for (ir = 0; ir < n; ++ir) {
		r[ir] = num[ir];
	}

	for (iqr = MAX(nq, nqu), iqrb = MAX(nq, nqu) * 16; iqr > 0;
			--iqr, iqrb -= 16) {
		uint16_t lo;
		uint32_t hi;
		bool redo_w;

		if (msb_d + iqrb - 16 > msb_r) {
			if (iqr <= nq) {
				q[nq - iqr] = 0;
			}
			continue;
		}
		/* Use binary search to determine the digit in the quotient. */
		lo = 0;
		hi = (uint32_t)1 << MIN(16, (msb_r - msb_d  + 1 - (iqrb - 16)));
		redo_w = true;
		assert (lo < hi - 1);
		while (1) {
			uint16_t try = (uint16_t)((lo + hi) / 2);
			bool over = mul_u16n(w, den, &try, nd + 1, nd, 1);
			int c;

			if (over) {
				assert(0);
			}
			c = cmp_u16n(r, w, n - (iqr - 1), nd + 1);
			if (c < 0) {
				hi = try;
				redo_w = true;
			} else {
				lo = try;
				redo_w = false;
			}
			if (!c || lo == hi - 1) {
				if (redo_w) {
					over = mul_u16n(w, den, &lo, nd + 1,
						nd, 1);
					assert(!over);
					assert(cmp_u16n(r, w, n - (iqr - 1),
						nd + 1) >= 0);
				}
				if (iqr <= nq) {
					q[nq - iqr] = lo;
				}
				subip_u16n(r, w, n - (iqr - 1), nd + 1);
				msb_r = msb_u16n(r, n);
				break;
			}
		}
	}
	return (nqu > nq) ? 1 : 0;
}

/**
 * Arithmetic mean of the first 'size' entries of the array 'nums'
 * If frac is NULL, the returned result is rounded to the nearest integer.
 * If frac is not NULL, the returned result is the largest value less than or
 * equal to the true value and *frac is set to the fractional difference
 * between the true value and returned result.
 */
int mean(const int *nums, int size, struct my_rational *frac)
{
#define MP_DIGITS ((sizeof(unsigned int) + (sizeof(uint16_t) - 1)) / sizeof(uint16_t))
	/* Holds the absolute value of INT_MIN in an unsigned int. */
	unsigned int abs_int_min = (unsigned int)(-(INT_MIN + 1)) + 1;
	/*
	 * Store the positive constructions to sum(nums[i]) in sp and the
	 * negative contributions to sum(nums[i]) in sn.
	 */
	uint16_t sp[2 * MP_DIGITS], sn[2 * MP_DIGITS];
	uint16_t q[MP_DIGITS], r[2 * MP_DIGITS], w[MP_DIGITS + 1];
	uint16_t sz[MP_DIGITS];
	struct my_rational f;
	unsigned int result;
	int i;
	/*
	 * Avoid compiler warnings about set but unused variables when
	 * assertions are stripped.
	 */
#ifndef NDEBUG
	bool over;
#define CHECK_OVERFLOW(x) over = x; assert(!over)
	uint16_t carry;
#define CHECK_CARRY(x) carry = x; assert(carry == 0)
#else
#define CHECK_OVERFLOW(x) x
#define CHECK_CARRY(x) x
#endif

	if (size <= 0) {
		if (frac) {
			*frac = my_rational_construct(0, 1);
		}
		return 0;
	}

	zer_u16n(sp, N_ELEMENTS(sp));
	zer_u16n(sn, N_ELEMENTS(sn));

	for (i = 0; i < size; ++i) {
		uint16_t v[MP_DIGITS];

		if (nums[i] == 0) {
			continue;
		}
		if (nums[i] > 0) {
			CHECK_OVERFLOW(ini_u16n(v, N_ELEMENTS(v), nums[i]));
			CHECK_CARRY(addip_u16n(sp, v, N_ELEMENTS(sp), N_ELEMENTS(v)));
		} else if (nums[i] < 0) {
			CHECK_OVERFLOW(ini_u16n(v, N_ELEMENTS(v), abs_int_min - (unsigned int)(nums[i] - INT_MIN)));
			CHECK_CARRY(addip_u16n(sn, v, N_ELEMENTS(sn), N_ELEMENTS(v)));
		}
	}

	CHECK_OVERFLOW(ini_u16n(sz, N_ELEMENTS(sz), size));
	if (cmp_u16n(sp, sn, N_ELEMENTS(sp), N_ELEMENTS(sn)) < 0) {
		subip_u16n(sn, sp, N_ELEMENTS(sn), N_ELEMENTS(sp));
		i = div_u16n(q, r, w, sn, sz, N_ELEMENTS(q), N_ELEMENTS(sn),
			N_ELEMENTS(sz));
		assert(i == 0);
		assert(msb_u16n(q, N_ELEMENTS(q))
			<= CHAR_BIT * sizeof(unsigned int));
		result = ext_u16n(q, N_ELEMENTS(q));
		assert(result <= abs_int_min);
		assert(cmp_u16n(sz, r, N_ELEMENTS(sz), N_ELEMENTS(r)) > 0);
		f = my_rational_construct(ext_u16n(r, N_ELEMENTS(r)), size);
		if (frac) {
			/*
			 * Will be negating m, but the returned fraction must
			 * be non-negative so increment m and replace the
			 * fractional part with (size - remainder) / size.
			 */
			if (f.n > 0) {
				assert(result < abs_int_min);
				++result;
				assert(f.n < f.d);
				frac->n = f.d - f.n;
				frac->d = f.d;
			} else {
				*frac = f;
			}
		} else {
			/* Round to nearest. */
			if (f.n >= (f.d + 1) / 2) {
				assert(result < abs_int_min);
				++result;
			}
		}
		return (result == 0) ?
			0 : INT_MIN + (int)(abs_int_min - result);
	}
	subip_u16n(sp, sn, N_ELEMENTS(sp), N_ELEMENTS(sn));
	i = div_u16n(q, r, w, sp, sz, N_ELEMENTS(q), N_ELEMENTS(sp), N_ELEMENTS(sz));
	assert(i == 0);
	assert(msb_u16n(q, N_ELEMENTS(q)) <= CHAR_BIT * sizeof(unsigned int));
	result = ext_u16n(q, N_ELEMENTS(q));
	assert(result <= INT_MAX);
	assert(cmp_u16n(sz, r, N_ELEMENTS(sz), N_ELEMENTS(r)) > 0);
	f = my_rational_construct(ext_u16n(r, N_ELEMENTS(r)), size);
	if (frac) {
		*frac = f;
	} else {
		/* Round to nearest. */
		if (f.n >= (f.d + 1) / 2) {
			assert(result < INT_MAX);
			++result;
		}
	}

	return result;
#undef MP_DIGITS
#undef CHECK_OVERFLOW
#undef CHECK_CARRY
}

/**
 * Variance of the first 'size' entries of the array 'nums'
 * If unbiased is true, the variance is computed as
 * sum((nums[i] - mean(nums))^2) / (size - 1); otherwise it is
 * sum((nums[i] - mean(nums))^2) / size.
 * If of_mean is true, the returned variance is scaled by an additional
 * 1 / size to correspond to the variance of the estimate of the mean.
 * If frac is NULL, the result is rounded to the nearest integer.  If frac
 * is not NULL, the result is rounded down and *frac is set to the fractional
 * part (perhaps approximated).  If the result would be larger than INT_MAX,
 * the returned result is INT_MAX and *frac, if frac is not NULL, will be zero.
 */
int variance(const int *nums, int size, bool unbiased, bool of_mean,
		struct my_rational *frac)
{
#define MP_DIGITS ((sizeof(unsigned int) + (sizeof(uint16_t) - 1)) / sizeof(uint16_t))
	/* Holds the absolute value of INT_MIN in an unsigned int. */
	unsigned int abs_int_min = (unsigned int)(-(INT_MIN + 1)) + 1;
	/*
	 * The unbiased variance estimator, sum((nums[i] - sum(nums[i]) /
	 * size)^2) / (size - 1), is equivalent to (sum(nums[i]^2) -
	 * (sum(nums[i]))^2 / size) / (size - 1).  Store sum(nums[i]^2) in
	 * ss, the positive contributions to sum(nums[i]) in sp, and the
	 * negative contributions to sum(nums[i]) in sn.
	 */
	uint16_t ss[3 * MP_DIGITS], sp[2 * MP_DIGITS], sn[2 * MP_DIGITS];
	uint16_t sqs[4 * MP_DIGITS];
	uint16_t q[4 * MP_DIGITS], r0[4 * MP_DIGITS], r1[3 * MP_DIGITS];
	uint16_t r2[3 * MP_DIGITS];
	uint16_t sz[MP_DIGITS], nm[MP_DIGITS], w[MP_DIGITS + 1];
	struct my_rational f, part;
	unsigned int var;
	int norm, i;
	/*
	 * Avoid compiler warnings about set but unused variables when
	 * assertions are stripped.
	 */
#ifndef NDEBUG
	bool over;
#define CHECK_OVERFLOW(x) over = x; assert(!over)
	uint16_t carry;
#define CHECK_CARRY(x) carry = x; assert(carry == 0)
#else
#define CHECK_OVERFLOW(x) x
#define CHECK_CARRY(x) x
#endif

	if (size <= 1) {
		if (frac) {
			*frac = my_rational_construct(0, 1);
		}
		return 0;
	}

	norm = size - ((unbiased) ? 1 : 0);
	zer_u16n(ss, N_ELEMENTS(ss));
	zer_u16n(sp, N_ELEMENTS(sp));
	zer_u16n(sn, N_ELEMENTS(sn));

	for (i = 0; i < size; ++i) {
		uint16_t a[MP_DIGITS], a2[2 * MP_DIGITS];

		if (nums[i] == 0) {
			continue;
		}
		if (nums[i] > 0) {
			CHECK_OVERFLOW(ini_u16n(a, N_ELEMENTS(a), nums[i]));
			CHECK_CARRY(addip_u16n(sp, a, N_ELEMENTS(sp), N_ELEMENTS(a)));
		} else if (nums[i] < 0) {
			CHECK_OVERFLOW(ini_u16n(a, N_ELEMENTS(a), abs_int_min - (unsigned int)(nums[i] - INT_MIN)));
			CHECK_CARRY(addip_u16n(sn, a, N_ELEMENTS(sn), N_ELEMENTS(a)));
		}
		CHECK_OVERFLOW(mul_u16n(a2, a, a, N_ELEMENTS(a2), N_ELEMENTS(a), N_ELEMENTS(a)));
		CHECK_CARRY(addip_u16n(ss, a2, N_ELEMENTS(ss), N_ELEMENTS(a2)));
	}

	if (cmp_u16n(sp, sn, N_ELEMENTS(sp), N_ELEMENTS(sn)) >= 0) {
		subip_u16n(sp, sn, N_ELEMENTS(sp), N_ELEMENTS(sn));
		CHECK_OVERFLOW(mul_u16n(sqs, sp, sp, N_ELEMENTS(sqs), N_ELEMENTS(sp), N_ELEMENTS(sp)));
	} else {
		subip_u16n(sn, sp, N_ELEMENTS(sn), N_ELEMENTS(sp));
		CHECK_OVERFLOW(mul_u16n(sqs, sn, sn, N_ELEMENTS(sqs), N_ELEMENTS(sn), N_ELEMENTS(sn)));
	}
	CHECK_OVERFLOW(ini_u16n(sz, N_ELEMENTS(sz), size));
	i = div_u16n(q, r0, w, sqs, sz, N_ELEMENTS(q), N_ELEMENTS(sqs),
		N_ELEMENTS(sz));
	assert(i == 0);
	if (msb_u16n(r0, N_ELEMENTS(r0)) > 0) {
		/*
		 * Since this remainder is subtracted, add one to the
		 * quotient and convert the remainder to size - r0.  That way
		 * later calculations can treat it as a non-negative term, like
		 * the other fractional parts.
		 */
		uint16_t tmp[4 * MP_DIGITS];

		CHECK_OVERFLOW(ini_u16n(tmp, MP_DIGITS, 1));
		addip_u16n(q, tmp, N_ELEMENTS(q), MP_DIGITS);
		for (i = 0; i < (int)(N_ELEMENTS(r0)); ++i) {
			tmp[i] = r0[i];
		}
		CHECK_OVERFLOW(ini_u16n(r0, N_ELEMENTS(r0), size));
		assert(cmp_u16n(r0, tmp, N_ELEMENTS(r0), N_ELEMENTS(tmp)) > 0);
		subip_u16n(r0, tmp, N_ELEMENTS(r0), N_ELEMENTS(tmp));
	}
	assert(cmp_u16n(ss, q, N_ELEMENTS(ss), N_ELEMENTS(q)) >= 0);
	subip_u16n(ss, q, N_ELEMENTS(ss), N_ELEMENTS(q));
	CHECK_OVERFLOW(ini_u16n(nm, N_ELEMENTS(nm), norm));
	i = div_u16n(q, r1, w, ss, nm, 3 * MP_DIGITS, N_ELEMENTS(ss),
		N_ELEMENTS(nm));
	assert(i == 0);
	if (of_mean) {
		uint16_t qcpy[3 * MP_DIGITS];

		for (i = 0; i < (int)(N_ELEMENTS(qcpy)); ++i) {
			qcpy[i] = q[i];
		}
		i = div_u16n(q, r2, w, qcpy, sz, 3 * MP_DIGITS,
			N_ELEMENTS(qcpy), N_ELEMENTS(sz));
		assert(i == 0);
	}
	if (msb_u16n(q, 3 * MP_DIGITS) > CHAR_BIT * sizeof(unsigned int)) {
		/* Variance is greater than UINT_MAX. */
		if (frac) {
			*frac = my_rational_construct(0, 1);
		}
		return INT_MAX;
	}
	var = ext_u16n(q, 3 * MP_DIGITS);
	if (var > INT_MAX) {
		if (frac) {
			*frac = my_rational_construct(0, 1);
		}
		return INT_MAX;
	}
	/*
	 * Account for the fractional part.  If of_mean is false, that is
	 * r1 / norm + r0 / (size * norm).  If of_mean is true,
	 * that is r2 / size + r1 / (size * norm) + r0 /
	 * (size * size * norm).  Since 0 <= r0 < size,
	 * 0 <= r1 < norm, and 0 <= r2 < size, the total of the fractional
	 * terms is greater than or equal to zero and less than one.
	 */
	assert(cmp_u16n(sz, r0, N_ELEMENTS(sz), N_ELEMENTS(r0)) > 0);
	assert(cmp_u16n(nm, r1, N_ELEMENTS(nm), N_ELEMENTS(r1)) > 0);
	assert(!of_mean || cmp_u16n(sz, r2, N_ELEMENTS(sz), N_ELEMENTS(r2)) > 0);
	f = my_rational_construct(ext_u16n(r0, N_ELEMENTS(r0)), size);
	part = my_rational_construct(1, norm);
	f = my_rational_product(&f, &part);
	part = my_rational_construct(ext_u16n(r1, N_ELEMENTS(r1)), norm);
	f = my_rational_sum(&f, &part);
	if (of_mean) {
		part = my_rational_construct(1, size);
		f = my_rational_product(&f, &part);
		part = my_rational_construct(ext_u16n(r2, N_ELEMENTS(r2)), size);
		f = my_rational_sum(&f, &part);
	}
	if (frac) {
		*frac = f;
	} else {
		/* Round to nearest. */
		if (f.n >= (f.d + 1) / 2 && var < INT_MAX) {
			++var;
		}
	}

	return var;
#undef MP_DIGITS
#undef CHECK_CARRY
#undef CHECK_OVERFLOW
}

/**
 * Return the greatest common divisor of the two arguments.
 */
unsigned int gcd(unsigned int a, unsigned int b)
{
	/* Use the division-based version of Euclid's algorithm. */
	while (b) {
		unsigned int t = b;

		b = a % b;
		a = t;
	}
	return a;
}

/**
 * Construct a rational value.
 */
struct my_rational my_rational_construct(unsigned int numerator,
		unsigned int denominator)
{
	struct my_rational result;

	assert(denominator > 0);
	if (numerator == 0) {
		/* Use 0 / 1 as the way to represent zero. */
		result.n = 0;
		result.d = 1;
	} else {
		unsigned int g = gcd(numerator, denominator);

		result.n = numerator / g;
		result.d = denominator / g;
	}
	return result;
}

/**
 * Scale a rational value and return the result.
 *
 * \param a points to the rational value to scale.
 * \param scale is the scale factor to apply.
 * \param remainder will, if not NULL, be dereferenced and set to numerator
 * of the fraction (denominator is a->d) that is not included in the return
 * value so the caller can use it for rounding or other purposes.
 */
unsigned int my_rational_to_uint(const struct my_rational *a,
		unsigned int scale, unsigned int *remainder)
{
	unsigned int result, r, q, r2, t;

	if (!scale) {
		if (remainder) {
			*remainder = 0;
		}
		return 0;
	}
	result = a->n / a->d;
	if (result > UINT_MAX / scale) {
		if (remainder) {
			*remainder = 0;
		}
		return UINT_MAX;
	}
	result *= scale;
	r = a->n % a->d;
	q = scale / a->d;
	if (result > UINT_MAX - q * r) {
		if (remainder) {
			*remainder = 0;
		}
		return UINT_MAX;
	}
	result += q * r;
	r2 = scale - q * a->d;
	if (r && r2 > UINT_MAX / r) {
		/*
		 * The product of the remainders overflows in the native
		 * arithmetic so use multiprecision integers.
		 */
#define MP_DIGITS ((sizeof(unsigned int) + (sizeof(uint16_t) - 1)) / sizeof(uint16_t))
		uint16_t r_u16n[MP_DIGITS], r2_u16n[MP_DIGITS];
		uint16_t d_u16n[MP_DIGITS], p_u16n[2 * MP_DIGITS];
		uint16_t qr_u16n[MP_DIGITS], rr_u16n[2 * MP_DIGITS];
		uint16_t w_u16n[MP_DIGITS + 1];
		bool over;

		over = ini_u16n(r_u16n, MP_DIGITS, r);
		if (over) {
			assert(0);
		}
		over = ini_u16n(r2_u16n, MP_DIGITS, r2);
		if (over) {
			assert(0);
		}
		over = ini_u16n(d_u16n, MP_DIGITS, a->d);
		if (over) {
			assert(0);
		}
		over = mul_u16n(p_u16n, r_u16n, r2_u16n, 2 * MP_DIGITS,
			MP_DIGITS, MP_DIGITS);
		if (over) {
			assert(0);
		}
		over = (div_u16n(qr_u16n, rr_u16n, w_u16n, p_u16n, d_u16n,
			MP_DIGITS, 2 * MP_DIGITS, MP_DIGITS) != 0);
		if (over) {
			assert(0);
		}
		assert(msb_u16n(qr_u16n, MP_DIGITS)
			<= sizeof(unsigned int) * CHAR_BIT);
		q = ext_u16n(qr_u16n, MP_DIGITS);
		if (result <= UINT_MAX - q) {
			result += q;
			if (remainder) {
				assert(msb_u16n(rr_u16n, 2 * MP_DIGITS)
					<= sizeof(unsigned int) * CHAR_BIT);
				*remainder = ext_u16n(rr_u16n, 2 * MP_DIGITS);
				assert(*remainder < a->d);
			}
		} else {
			result = UINT_MAX;
			if (remainder) {
				*remainder = 0;
			}
		}
		return result;
#undef MP_DIGITS
	}
	t = r * r2;
	q = t / a->d;
	if (result > UINT_MAX - q) {
		if (remainder) {
			*remainder = 0;
		}
		return UINT_MAX;
	}
	result += q;
	if (remainder) {
		*remainder = t - q * a->d;
	}
	return result;
}

/**
 * Multiply two rational values and return the result.
 */
struct my_rational my_rational_product(const struct my_rational *a,
		const struct my_rational *b)
{
	unsigned int g1 = gcd(a->n, b->d);
	unsigned int g2 = gcd(a->d, b->n);
	unsigned int anr = a->n / g1;
	unsigned int adr = a->d / g2;
	unsigned int bnr = b->n / g2;
	unsigned int bdr = b->d / g1;
	struct my_rational result;

	if ((bnr && anr > UINT_MAX / bnr) || (adr > UINT_MAX / bdr)) {
		/* Overflows in native arithmetic so approximate. */
#define MP_DIGITS ((sizeof(unsigned int) + (sizeof(uint16_t) - 1)) / sizeof(uint16_t))
		uint16_t a_u16n[MP_DIGITS], b_u16n[MP_DIGITS];
		uint16_t n_u16n[2 * MP_DIGITS], d_u16n[2 * MP_DIGITS];
		uint16_t q_u16n[MP_DIGITS], r_u16n[3 * MP_DIGITS];
		uint16_t sr_u16n[3 * MP_DIGITS], w_u16n[2 * MP_DIGITS + 1];
		unsigned int rn, rd;
		bool over;

		over = ini_u16n(a_u16n, MP_DIGITS, anr);
		if (over) {
			assert(0);
		}
		over = ini_u16n(b_u16n, MP_DIGITS, bnr);
		if (over) {
			assert(0);
		}
		over = mul_u16n(n_u16n, a_u16n, b_u16n, 2 * MP_DIGITS,
			MP_DIGITS, MP_DIGITS);
		if (over) {
			assert(0);
		}
		over = ini_u16n(a_u16n, MP_DIGITS, adr);
		if (over) {
			assert(0);
		}
		over = ini_u16n(b_u16n, MP_DIGITS, bdr);
		if (over) {
			assert(0);
		}
		over = mul_u16n(d_u16n, a_u16n, b_u16n, 2 * MP_DIGITS,
			MP_DIGITS, MP_DIGITS);
		if (over) {
			assert(0);
		}
		over = (div_u16n(q_u16n, r_u16n, w_u16n, n_u16n, d_u16n,
			MP_DIGITS, 2 * MP_DIGITS, 2 * MP_DIGITS) != 0);
		if (!over && msb_u16n(q_u16n, MP_DIGITS)
				<= sizeof(unsigned int) * CHAR_BIT) {
			unsigned int t;

			rn = ext_u16n(q_u16n, MP_DIGITS);
			/*
			 * Use as large as possible of a denominator so the
			 * approximation is as accurate as possible.
			 */
			rd = (rn < UINT_MAX) ? UINT_MAX / (rn + 1) : 1;
			rn *= rd;
			over = ini_u16n(a_u16n, MP_DIGITS, rd);
			if (over) {
				assert(0);
			}
			over = mul_u16n(sr_u16n, r_u16n, a_u16n, 3 * MP_DIGITS,
				2 * MP_DIGITS, MP_DIGITS);
			if (over) {
				assert(0);
			}
			over = div_u16n(q_u16n, r_u16n, w_u16n, sr_u16n, d_u16n,
				MP_DIGITS, 3 * MP_DIGITS, 2 * MP_DIGITS);
			if (over) {
				assert(0);
			}
			assert(msb_u16n(q_u16n, MP_DIGITS)
				<= sizeof(unsigned int) * CHAR_BIT);
			t = ext_u16n(q_u16n, MP_DIGITS);
			assert(rn <= UINT_MAX - t);
			rn += t;
			/* Approximate rounding to the nearest. */
			if (msb_u16n(r_u16n, 3 * MP_DIGITS) + 1
					>= msb_u16n(d_u16n, 2 * MP_DIGITS)) {
				assert(rn < UINT_MAX);
				++rn;
			}
		} else {
			rn = UINT_MAX;
			rd = 1;
		}
		return my_rational_construct(rn, rd);
#undef MP_DIGITS
	}
	result.n = anr * bnr;
	result.d = adr * bdr;
	return result;
}

/**
 * Add two rational values and return the result.
 */
struct my_rational my_rational_sum(const struct my_rational *a,
		const struct my_rational *b)
{
	unsigned int g = gcd(a->d, b->d);
	unsigned int adr = a->d / g, bdr = b->d / g;
	unsigned int resn, resd;

	if (adr <= UINT_MAX / b->d
			&& a->n <= UINT_MAX / bdr
			&& b->n <= UINT_MAX / adr
			&& a->n * bdr <= UINT_MAX - b->n * adr) {
		resn = a->n * bdr + b->n * adr;
		resd = adr * b->d;
	} else {
		/* Overflows in native arithmetic so approximate. */
#define MP_DIGITS ((sizeof(unsigned int) + (sizeof(uint16_t) - 1)) / sizeof(uint16_t))
		uint16_t a_u16n[MP_DIGITS], b_u16n[MP_DIGITS];
		uint16_t n_u16n[2 * MP_DIGITS + 1], d_u16n[2 * MP_DIGITS];
		uint16_t q_u16n[MP_DIGITS], r_u16n[2 * MP_DIGITS + 1];
		uint16_t sr_u16n[3 * MP_DIGITS + 1], rr_u16n[3 * MP_DIGITS + 1];
		uint16_t w_u16n[2 * MP_DIGITS + 1];
		bool over;

		over = ini_u16n(a_u16n, MP_DIGITS, a->n);
		if (over) {
			assert(0);
		}
		over = ini_u16n(b_u16n, MP_DIGITS, bdr);
		if (over) {
			assert(0);
		}
		over = mul_u16n(sr_u16n, a_u16n, b_u16n, 2 * MP_DIGITS,
			MP_DIGITS, MP_DIGITS);
		if (over) {
			assert(0);
		}
		over = ini_u16n(a_u16n, MP_DIGITS, adr);
		if (over) {
			assert(0);
		}
		over = ini_u16n(b_u16n, MP_DIGITS, b->n);
		if (over) {
			assert(0);
		}
		over = mul_u16n(n_u16n + 1, a_u16n, b_u16n, 2 * MP_DIGITS,
			MP_DIGITS, MP_DIGITS);
		if (over) {
			assert(0);
		}
		n_u16n[0] = addip_u16n(n_u16n + 1, sr_u16n, 2 * MP_DIGITS,
			2 * MP_DIGITS);
		over = ini_u16n(b_u16n, MP_DIGITS, b->d);
		if (over) {
			assert(0);
		}
		over = mul_u16n(d_u16n, a_u16n, b_u16n, 2 * MP_DIGITS,
			MP_DIGITS, MP_DIGITS);
		if (over) {
			assert(0);
		}
		over = (div_u16n(q_u16n, r_u16n, w_u16n, n_u16n, d_u16n,
			MP_DIGITS, 2 * MP_DIGITS + 1, 2 * MP_DIGITS) != 0);
		if (!over && msb_u16n(q_u16n, MP_DIGITS)
				<= sizeof(unsigned int) * CHAR_BIT) {
			unsigned int t;

			resn = ext_u16n(q_u16n, MP_DIGITS);
			/*
			 * Use as large as a numerator as possible so the
			 * approximation is as accurate as possible.
			 */
			resd = (resn < UINT_MAX) ? UINT_MAX / (resn + 1) : 1;
			resn *= resd;
			over = ini_u16n(a_u16n, MP_DIGITS, resd);
			if (over) {
				assert(0);
			}
			over = mul_u16n(sr_u16n, r_u16n, a_u16n,
				3 * MP_DIGITS + 1, 2 * MP_DIGITS + 1,
				MP_DIGITS);
			if (over) {
				assert(0);
			}
			over = (div_u16n(q_u16n, rr_u16n, w_u16n, sr_u16n,
				d_u16n, MP_DIGITS, 3 * MP_DIGITS + 1,
				2 * MP_DIGITS) != 0);
			if (over) {
				assert(0);
			}
			assert(msb_u16n(q_u16n, MP_DIGITS)
				<= sizeof(unsigned int) * CHAR_BIT);
			t = ext_u16n(q_u16n, MP_DIGITS);
			if (resn <= UINT_MAX - t) {
				resn += t;
				/* Approximate rounding to the nearest. */
				if (msb_u16n(rr_u16n, 3 * MP_DIGITS + 1) + 1
						>= msb_u16n(d_u16n,
						2 * MP_DIGITS)
						&& resn < UINT_MAX) {
					++resn;
				}
			} else {
				assert(resd == 1);
				resn = UINT_MAX;
			}
		} else {
			resn = UINT_MAX;
			resd = 1;
		}
#undef MP_DIGITS
	}
	return my_rational_construct(resn, resd);
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

