/**
 * \file ui-effect.c
 * \brief Implement functions for the text UI's handling of effects
 *
 * Copyright (c) 2021 Eric Branlund
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
#include "effects-info.h"
#include "ui-effect.h"
#include "ui-menu.h"


/**
 * Create and initialize an effect menu.
 * \param effect is the pointer to the first effect in the linked list.
 * \param count is the number of elements to take from the linked list.
 * \param allow_random if true, include an option to choose one of the effects
 * at random.
 * \return a pointer to the menu structure if successful or NULL if creating
 * the menu failed
 */
static struct menu *effect_menu_new(struct effect *effect, int count,
		bool allow_random)
{
	struct menu *m = menu_new(MN_SKIN_SCROLL,
		menu_find_iter(MN_ITER_STRINGS));
	int width = 0;
	region loc = { 0, 1, -99, -99 };
	int ms_count = 0;
	char **ms;
	char buf[80];

	m->selections = all_letters_nohjkl;
	m->flags = MN_KEYMAP_ESC;

	/* Collect a string for each effect. */
	if (count > 0) {
		if (allow_random) ++count;
		/*
		 * There's one extra element to serve as a sentinel for
		 * effect_menu_destroy().
		 */
		ms = mem_alloc((count + 1) * sizeof(*ms));
		if (allow_random) {
			ms[ms_count] =
				string_make("one of the following at random");
			width = MAX(width, (int)MIN(strlen(ms[ms_count])
				+ 3, (size_t)(Term->wid)));
			++ms_count;
		}
	} else {
		ms = NULL;
	}
	while (ms_count < count) {
		size_t len;

		len = effect_get_menu_name(buf, sizeof(buf), effect);
		if (!len) {
			int j;

			for (j = 0; j < ms_count; ++j) {
				string_free(ms[j]);
			}
			mem_free(ms);
			mem_free(m);
			msg("Mismatched count and effect list passed to effect_menu_new().  Please report this bug.");
			return NULL;
		}
		ms[ms_count] = string_make(buf);
		width = MAX(width, (int)MIN(len + 3, (size_t)(Term->wid)));
		++ms_count;
		effect = effect_next(effect);
	}
	if (count > 0) {
		/* Set the sentinel element. */
		ms[ms_count] = NULL;
	}

	menu_setpriv(m, ms_count, ms);

	/* Set size. */
	loc.width = width;
	loc.page_rows = MIN(ms_count, Term->hgt - 2);
	menu_layout(m, &loc);

	return m;
}


static int effect_menu_select(struct menu *m, const char *prompt,			bool allow_random)
{
	ui_event out;
	int selection;

	screen_save();
	region_erase_bordered(&m->active);
	prt((prompt) ? prompt : "Which effect? ", 0, 0);
	out = menu_select(m, 0, false);
	if (out.type & EVT_SELECT) {
		selection = m->cursor;
		if (allow_random) {
			if (selection == 0) {
				selection = -2;
			} else if (selection > 0) {
				--selection;
			}
		}
	} else {
		selection = -1;
	}
	screen_load();

	return selection;
}


/**
 * Release resources allocated by effect_menu_new().
 * \param m is a menu returned by effect_menu_new().
 */
static void effect_menu_destroy(struct menu *m)
{
	if (m) {
		char **ms = menu_priv(m);
		int i = 0;

		while (ms[i]) {
			string_free(ms[i]);
			++i;
		}
		mem_free(ms);
		mem_free(m);
	}
}


/**
 * Display a menu to select an effect from the given list.
 * \param prompt is the string to use as the prompt for the menu.  May be NULL
 * to use a default prompt.
 * \param effect is the pointer to the first element in the linked list of
 * effects.
 * \param count is the number of effects to take from the linked list.
 * \param allow_random if true, also include an option to get a random element
 * from the list.
 * \return the index of the selected item, -2 if the user selected the random
 * option enabled by allow_random, or -1 to indicate that the selection was
 * canceled or invalid
 */
int textui_get_effect_from_list(const char *prompt, struct effect *effect,
	int count, bool allow_random)
{
	struct menu *m;
	int choice;

	if (count == -1) {
		struct effect *cursor = effect;

		count = 0;
		while (cursor) {
			++count;
			cursor = effect_next(cursor);
		}
	}

	m = effect_menu_new(effect, count, allow_random);
	if (m) {
		choice = effect_menu_select(m, prompt, allow_random);
		effect_menu_destroy(m);
	} else {
		choice = -1;
	}

	return choice;
}
