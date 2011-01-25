/*
 * File: ui-event.c
 * Purpose: Utility functions relating to UI events
 *
 * Copyright (c) 2011 Andi Sidwell
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
#include "angband.h"
#include "ui-event.h"

/**
 * Map keycodes to their textual equivalent.
 */
static const struct {
	keycode_t code;
	const char *desc;
} mappings[] = {
	{ ESCAPE, "Escape" },
	{ KC_RETURN, "Return" },
	{ KC_ENTER, "Enter" },
	{ KC_TAB, "Tab" },
	{ KC_DELETE, "Delete" },
	{ KC_BACKSPACE, "Backspace" },
	{ ARROW_DOWN, "Down" },
	{ ARROW_LEFT, "Left" },
	{ ARROW_RIGHT, "Right" },
	{ ARROW_UP, "Up" },
	{ KC_F1, "F1" },
	{ KC_F2, "F2" },
	{ KC_F3, "F3" },
	{ KC_F4, "F4" },
	{ KC_F5, "F5" },
	{ KC_F6, "F6" },
	{ KC_F7, "F7" },
	{ KC_F8, "F8" },
	{ KC_F9, "F9" },
	{ KC_F10, "F10" },
	{ KC_F11, "F11" },
	{ KC_F12, "F12" },
	{ KC_F13, "F13" },
	{ KC_F14, "F14" },
	{ KC_F15, "F15" },
	{ KC_HELP, "Help" },
	{ KC_HOME, "Home" },
	{ KC_PGUP, "PageUp" },
	{ KC_END, "End" },
	{ KC_PGDOWN, "PageDown" },
	{ KC_INSERT, "Insert" },
	{ KC_PAUSE, "Pause" },
	{ KC_BREAK, "Break" }
};


/**
 * Given a string, try and find it in "mappings".
 */
keycode_t keycode_find_code(const char *str, size_t len)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(mappings); i++) {
		if (strncmp(str, mappings[i].desc, len) == 0)
			return mappings[i].code;
	}
	return 0;
}


/**
 * Given a keycode, return its textual mapping.
 */
const char *keycode_find_desc(keycode_t kc)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(mappings); i++) {
		if (mappings[i].code == kc)
			return mappings[i].desc;
	}
	return NULL;
}




/*
 * Convert a hexidecimal-digit into a decimal
 */
static int dehex(char c)
{
	if (isdigit((unsigned char)c)) return (D2I(c));
	if (isalpha((unsigned char)c)) return (A2I(tolower((unsigned char)c)) + 10);
	return (0);
}

/**
 * Convert an encoding of a set of keypresses into actual keypresses.
 */
void keypress_from_text(struct keypress *buf, size_t len, const char *str)
{
	size_t cur = 0;

	memset(buf, 0, len * sizeof *buf);

	/* Analyze the "ascii" string */
	while (*str && cur < len)
	{
		buf[cur].type = EVT_KBRD;

		/* Backslash codes */
		if (*str == '\\')
		{
			str++;
			if (*str == '\0') break;

			switch (*str)
			{
				/* Hex-mode */
				case 'x':
				{
					if (isxdigit((unsigned char)(*(str + 1))) &&
					    isxdigit((unsigned char)(*(str + 2))))
					{
						buf[cur].code = 16 * dehex(*++str);
						buf[cur].code += 16 * dehex(*++str);
						cur++;
					}
					else
					{
						/* HACK - Invalid hex number */
						buf[cur++].code = '?';
					}
					break;
				}

				case 'e': buf[cur++].code = ESCAPE; break;
				case 's': buf[cur++].code = ' '; break;
				case 'b': buf[cur++].code = '\b'; break;
				case 'n': buf[cur++].code = '\n'; break;
				case 'r': buf[cur++].code = '\r'; break;
				case 't': buf[cur++].code = '\t'; break;
				case 'a': buf[cur++].code = '\a'; break;
				case '\\': buf[cur++].code = '\\'; break;
				case '^': buf[cur++].code = '^'; break;
				default: buf[cur++].code = *str; break;
			}

			/* Skip the final char */
			str++;
		}

		/* Normal Control codes */
		else if (*str == '^')
		{
			str++;
			if (*str == '\0') break;

			buf[cur].code = KTRL(*str);
			buf[cur++].mods = KC_MOD_CONTROL;
			str++;
		}

		/* Normal chars */
		else
		{
			buf[cur++].code = *str++;
		}
	}

	/* Terminate */
	cur = MIN(cur, len);
	buf[cur].type = EVT_NONE;
}

/*
 * Convert a string of keypresses into their textual equivalent.
 */
void keypress_to_text(char *buf, size_t len, const struct keypress *src)
{
	size_t cur = 0;
	size_t end = 0;

	while (src[cur].type == EVT_KBRD) {
		keycode_t i = src[cur].code;

		switch (i) {
			case ESCAPE: strnfcat(buf, len, &end, "\e"); break;
			case ' ':    strnfcat(buf, len, &end, " "); break;
			case '\b': strnfcat(buf, len, &end, "\b"); break;
			case '\t': strnfcat(buf, len, &end, "\t"); break;
			case '\a': strnfcat(buf, len, &end, "\a"); break;
			case '\n': strnfcat(buf, len, &end, "\n"); break;
			case '\r': strnfcat(buf, len, &end, "\r"); break;
			case '\\': strnfcat(buf, len, &end, "\\"); break;
			case '^': strnfcat(buf, len, &end, "\\^"); break;
			default: {
				if (i < 32)
					strnfcat(buf, len, &end, "^%c", UN_KTRL(i));
				else if (i < 127)
					strnfcat(buf, len, &end, "%c", i);
				else
					strnfcat(buf, len, &end, "\\x%02x", (int)i);
				break;
			}
		}

		cur++;
	}

	/* Terminate */
	buf[end] = '\0';
}

