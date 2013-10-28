/*
 * File: keymap.c
 * Purpose: Keymap handling
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
#include "keymap.h"

/**
 * Keymap implementation.
 *
 * Keymaps are defined in pref files and map onto the internal game keyset,
 * which is roughly what you get if you have roguelike keys turned off.
 *
 * We store keymaps by pairing triggers with actions; the trigger is a single
 * keypress and the action is stored as a string of keypresses, terminated
 * with a keypress with type == EVT_NONE.
 *
 * XXX We should note when we read in keymaps that are "official game" keymaps
 * and ones which are user-defined.  Then we can avoid writing out official
 * game ones and messing up everyone's pref files with a load of junk.
 */


/**
 * Struct for a keymap.
 */
struct keymap {
	struct keypress key;
	struct keypress *actions;

	bool user;		/* User-defined keymap */

	struct keymap *next;
};


/**
 * List of keymaps.
 */
static struct keymap *keymaps[KEYMAP_MODE_MAX];


/**
 * Find a keymap, given a keypress.
 */
const struct keypress *keymap_find(int keymap, struct keypress kc)
{
	struct keymap *k;
	assert(keymap >= 0 && keymap < KEYMAP_MODE_MAX);
	for (k = keymaps[keymap]; k; k = k->next) {
		if (k->key.code == kc.code && k->key.mods == kc.mods)
			return k->actions;
	}

	return NULL;
}


/**
 * Duplicate a given keypress string and return the duplicate.
 */
static struct keypress *keymap_make(const struct keypress *actions)
{
	struct keypress *new;
	size_t n = 0;
	while (actions[n].type) {
		n++;
	}

	/* Make room for the terminator */
	n += 1;

	new = mem_zalloc(sizeof *new * n);
	memcpy(new, actions, sizeof *new * n);

	new[n - 1].type = EVT_NONE;

	return new;
}


/**
 * Add a keymap to the mappings table.
 */
void keymap_add(int keymap, struct keypress trigger, struct keypress *actions, bool user)
{
	struct keymap *k = mem_zalloc(sizeof *k);
	assert(keymap >= 0 && keymap < KEYMAP_MODE_MAX);

	keymap_remove(keymap, trigger);

	k->key = trigger;
	k->actions = keymap_make(actions);
	k->user = user;

	k->next = keymaps[keymap];
	keymaps[keymap] = k;

	return;
}


/**
 * Remove a keymap.  Return TRUE if one was removed.
 */
bool keymap_remove(int keymap, struct keypress trigger)
{
	struct keymap *k;
	struct keymap *prev = NULL;
	assert(keymap >= 0 && keymap < KEYMAP_MODE_MAX);

	for (k = keymaps[keymap]; k; k = k->next) {
		if (k->key.code == trigger.code && k->key.mods == trigger.mods) {
			mem_free(k->actions);
			if (prev)
				prev->next = k->next;
			else
				keymaps[keymap] = k->next;
			mem_free(k);
			return TRUE;
		}

		prev = k;
	}

	return FALSE;
}


/**
 * Forget and free all keymaps.
 */
void keymap_free(void)
{
	size_t i;
	struct keymap *k;
	for (i = 0; i < N_ELEMENTS(keymaps); i++) {
		k = keymaps[i];
		while (k) {
			struct keymap *next = k->next;
			mem_free(k->actions);
			mem_free(k);
			k = next;
		}
	}
}


/*
 * Append active keymaps to a given file.
 */
void keymap_dump(ang_file *fff)
{
	int mode;
	struct keymap *k;

	if (OPT(rogue_like_commands))
		mode = KEYMAP_MODE_ROGUE;
	else
		mode = KEYMAP_MODE_ORIG;

	for (k = keymaps[mode]; k; k = k->next) {
		char buf[1024];
		struct keypress key[2] = { { 0 }, { 0 } };

		if (!k->user) continue;

		/* Encode the action */
		keypress_to_text(buf, sizeof(buf), k->actions, FALSE);
		file_putf(fff, "A:%s\n", buf);

		/* Convert the key into a string */
		key[0] = k->key;
		keypress_to_text(buf, sizeof(buf), key, TRUE);
		file_putf(fff, "C:%d:%s\n", mode, buf);

		file_putf(fff, "\n");
	}
}
