/*
 * File: x-char.c
 * Purpose: Enable Latin-1 encodings for supported platforms.
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
#include "x-char.h"

/*
 * Translate from encodes to extended 8-bit characters and back again.
 */
static const xchar_type latin1_encode[] =
{
    { "`A", 192 },  { "'A", 193 },  { "^A", 194 },  { "~A", 195 },
    { "\"A", 196 },  { "*A", 197 },  { ",C", 199 },  { "`E", 200 },
    { "'E", 201 },  { "^E", 202 }, { "\"E", 203 },  { "`I", 204 },
    { "'I", 205 },  { "^I", 206 }, { "\"I", 207 },  { "~N", 209 },
    { "`O", 210 },  { "'O", 211 },  { "^O", 212 },  { "~O", 213 },
	{ "\"O", 214 },  { "/O", 216 },  { "`U", 217 },  { "'U", 218 },
    { "^U", 219 }, { "\"U", 220 },  { "'Y", 221 },  { "`a", 224 },
    { "'a", 225 },  { "^a", 226 },  { "~a", 227 }, { "\"a", 228 },
    { "*a", 229 },  { ",c", 231 },  { "`e", 232 },  { "'e", 233 },
    { "^e", 234 }, { "\"e", 235 },  { "`i", 236 },  { "'i", 237 },
    { "^i", 238 }, { "\"i", 239 },  { "~n", 241 },  { "`o", 242 },
    { "'o", 243 },  { "^o", 244 },  { "~o", 245 }, { "\"o", 246 },
    { "/o", 248 },  { "`u", 249 },  { "'u", 250 },  { "^u", 251 },
    { "\"u", 252 },  { "'y", 253 }, { "\"y", 255 },

    { "iexcl", 161 }, { "euro", 162 }, { "pound", 163 }, { "curren", 164 },
    { "yen", 165 },   { "brvbar", 166 }, { "sect", 167 }, { "Agrave", 192 },
    { "Aacute", 193 }, { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 },
    { "Aring", 197 }, { "Aelig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 },
    { "Eacute", 201 }, { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 },
    { "Iacute", 205 }, { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 },
    { "Otilde", 213 }, { "Ouml", 214 }, { "Oslash", 216 }, { "Ugrave", 217 },
    { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 }, { "Yacute", 221 },
    { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 }, { "aacute", 225 },
    { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 }, { "aring", 229 },
    { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 }, { "eacute", 233 },
    { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 }, { "iacute", 237 },
    { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 },   { "ntilde", 241 },
    { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 }, { "otilde", 245 },
    { "ouml", 246 }, { "oslash", 248 }, { "ugrave", 249 }, { "uacute", 250 },
    { "ucirc", 251 }, { "uuml", 252 }, { "yacute", 253 }, { "thorn", 254 },
    { "yuml", 255 },   { "\0", 0 }
};

/*
 * Translate from ISO Latin-1 characters 128+ to 7-bit ASCII.
 *
 * We use this table to maintain compatibility with systems that cannot
 * display 8-bit characters.  We also use it whenever we wish to suppress
 * accents or ensure that a character is 7-bit.
 */
static const char seven_bit_translation[128] =
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
 * Link to the xchar_trans function.
 */
static void xchar_trans_hook(char *s, int encoding)
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

