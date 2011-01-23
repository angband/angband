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
