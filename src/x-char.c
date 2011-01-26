/*
 * File: x-char.c
 * Purpose: Enable extended character sets for the Operating Systems that
 * can handle it without affecting the ASCII-only OSs.
 *
 * Copyright (c) 2007 Leon Marrick, Hugo Cornelius, Diego Gonzalez, Jeff Greene
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
#include "angband.h"
#include "z-util.h"
#include "z-term.h"



/*
 * Link to the xchar_trans function.
 */
void xchar_trans_hook(char *s, int encoding)
{
 	/* Option to translate into ASCII */
 	if (encoding == ASCII)
 	{
 		if (*s < 0) *s = seven_bit_translation[128 + *s];
 	}

 	/* Option to translate into system-specific character set */
 	else if (encoding == SYSTEM_SPECIFIC)
 	{
 		if (*s < 0) *s = xchar_trans(*s);
	}
}


/*
 * Given what we think is an encode, return a Latin-1 character position.
 */
static byte encode_to_xchar(char *encode)
{
 	int i;

 	/* Scan until we hit the end-of-table marker */
 	for (i = 0; latin1_encode[i].c; i++)
 	{
 		/* We found the encode; return the character */
 		if (streq(encode, latin1_encode[i].tag))
 			return (latin1_encode[i].c);
 	}

 	/* This encode is not recognized */
 	return (0);
}

/*
 * Read an encode.  Return the Latin-1 character position if successful.
 */
bool get_encode(char *str, char *c)
{
 	int n = 0;
 	char *s;
 	char encode[80];

 	/* Assume empty char */
 	*c = '\0';

 	/* An encode must start with a '[' */
 	if (str[0] != '[') return (FALSE);

 	/* Copy the encode (between brackets) */
 	for (s = str + 1; ((n < 80) && (*s) && (*s != ']')); n++)
 	{
 		encode[n] = *s++;
 	}

 	/* End the encode */
 	encode[n] = '\0';

 	/* We have a trailing bracket */
 	if (*s == ']')
 	{
 		/* Look up extended character */
 		*c = (char)encode_to_xchar(encode);

 		/* Encode is legal -- return the char */
 		if (*c) return (TRUE);
 	}

 	/* Illegal encode */
 	return (FALSE);
}



/*
 * Take a 7-bit ASCII string and use any encodes in it to insert 8-bit
 * characters.  Use the Latin-1 (ISO) standard by default.  -LM-
 *
 * Optionally, translate into 7-bit ASCII or a system-specific character set.
 *
 * The input string must be '\0'-terminated, and should not be greater than
 * 1024 character in length (we check this).
 */
void xstr_trans(char *str, int encoding)
{
 	/* Assume no encodes in this string */
 	bool flag = FALSE;

 	int n, c;

 	char *s, *b;
 	char buf[1024];
 	char encode[80];

 	/* Require a string */
 	if (!str) return;

 	/* Start at the beginning */
 	s = str;
 	b = buf;

 	/* Scan the string */
 	for (; *s;)
 	{
 		/* Paranoia -- check bounds */
 		if (b - buf > 1022) break;

 		/* Character is a [ */
 		if (*s == '[')
 		{
 			/* Remember where we are */
 			char *s_old = s;

 			/* Assume illegal */
 			c = 0;

 			/* Skip past the open bracket */
 			s += 1;

 			/* Copy the encode (between brackets) */
 			for (n = 0; ((n < 79) && (*s) && (*s != ']')); n++)
 			{
 				encode[n] = *s++;
 			}

 			/* End the encode */
 			encode[n] = '\0';

 			/* We have a trailing bracket */
 			if (*s == ']')
 			{
 				/* Go to next character */
 				s++;

 				/* Look up extended character */
 				c = (char)encode_to_xchar(encode);
 			}

 			/* Encode is legal */
 			if (c)
 			{
 				/* Note the encode */
 				flag = TRUE;

				/* Save it */
 				*b++ = c;
 			}

 			/* Encode is illegal */
 			else
 			{
 				/* Return to start of encode */
 				s = s_old;

 				/* Copy the '[', go to the next character */
 				*b++ = *s++;
 			}
 		}

 		/* No encoding recognized */
 		else
 		{
 			/* Copy the character */
 			*b++ = *s++;
 		}
	}

 	/* End the string */
 	*b = '\0';

 	/* Copy the edited string back, if we've found encodes */
 	if (flag) strcpy(str, buf);

	/* Translate the string if we don't want standard Latin-1 */
	if (encoding != LATIN1)
	{
		for (s = str; *s; s++) xchar_trans_hook(s, encoding);
	}
}

/*
 *  Translate a Latin-1 string into escaped ASCII
 *  We assume that the contents of the source string use the Latin-1 encoding
 */
void escape_latin1(char *dest, size_t max, const char *src)
{
	size_t i = 0;

	/* Make space for the trailing null character */
	if (max > 0) --max;

	/* Copy the source string into the ouput string escaping the non-ascii characters */
	while (*src && (i < max))
	{
		/* Make a copy of the character */
		byte chr = (byte)*src++;

		/* Non-ascii characters get special treatment */
		if (chr > 127)
		{
			int j;
			const char *tag = NULL;

			/* Find the escape secuence of the character */
			for (j = 0; latin1_encode[j].c > 0; j++)
			{
				if (latin1_encode[j].c == chr)
				{
					tag = latin1_encode[j].tag;

					break;
				}
			}

			/* Found? */
			if (tag)
			{
				/* Append the opening delimiter */
				if (i < max) dest[i++] = '[';

				/* Append the escape secuence */
				for (j = 0; tag[j] && (i < max); j++)
				{
					dest[i++] = tag[j];
				}

				/* Append the closing delimiter */
				if (i < max) dest[i++] = ']';

				/* Done */
				continue;
			}
		}

		/* Common case. We just append the character */
		dest[i++] = (char)chr;
	}

	/* Trailing null character */
	dest[i] = '\0';
}





/*
 * Translate from ISO Latin-1 characters 128+ to 7-bit ASCII.
 *
 * We use this table to maintain compatibility with systems that cannot
 * display 8-bit characters.  We also use it whenever we wish to suppress
 * accents or ensure that a character is 7-bit.
 */
const char seven_bit_translation[128] =
{
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
 	'A', 'A', 'A', 'A', 'A', 'A', ' ', 'C',
 	'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I',
 	'D', 'N', 'O', 'O', 'O', 'O', 'O', ' ',
 	'O', 'U', 'U', 'U', 'U', 'Y', ' ', ' ',
 	'a', 'a', 'a', 'a', 'a', 'a', ' ', 'c',
 	'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
 	'o', 'n', 'o', 'o', 'o', 'o', 'o', ' ',
	'o', 'u', 'u', 'u', 'u', 'y', ' ', 'y'
};

/*
 * Given a position in the ISO Latin-1 character set (which Angband uses
 * internally), return the correct display character on this system.
 * Assume ASCII-only if no special hook is available.  -LM-
 */
char xchar_trans(byte c)
{
 	char s;

 	/* Use the hook, if available */
 	if (Term->xchar_hook) return ((char)(Term->xchar_hook)(c));

 	/* 7-bit characters are not translated */
 	if (c < 128) return (c);

 	/* Translate to 7-bit (strip accent or convert to space) */
 	s = seven_bit_translation[c - 128];

 	if (s == 0) return (c);
 	else return (s);
}

