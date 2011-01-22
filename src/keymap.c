/*
 * File: keymap.c
 * Purpose: Keymap handling
 *
 * Copyright (c) 2010 Andi Sidwell
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
 * Struct for a keymap.
 */
struct keymap {
	keycode_t key;
	char *actions;

	struct keymap *next;
};


/**
 * List of keymaps.
 *
 * XXX the number of keymap listings should be macro'd
 */
struct keymap *keymaps[2];


const char *keymap_find(int keymap, keycode_t kc)
{
	struct keymap *k;
	for (k = keymaps[keymap]; k; k = k->next) {
		if (k->key == kc)
			return k->actions;
	}

	return NULL;
}


/**
 * Add a keymap to the mappings table.
 */
void keymap_add(int keymap, keycode_t trigger, char *actions)
{
	struct keymap *k = mem_zalloc(sizeof *k);

	keymap_remove(keymap, trigger);

	k->key = trigger;
	k->actions = string_make(actions);

	k->next = keymaps[keymap];
	keymaps[keymap] = k;

	return;
}


/**
 * Remove a keymap.  Return TRUE if one was removed.
 */
bool keymap_remove(int keymap, keycode_t trigger)
{
	struct keymap *k;
	struct keymap *prev = NULL;

	for (k = keymaps[keymap]; k; k = k->next) {
		if (k->key == trigger) {
			string_free(k->actions);
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
			string_free(k->actions);
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

		char key[2] = "?";

		/* Encode the action */
		ascii_to_text(buf, sizeof(buf), k->actions);
		file_putf(fff, "A:%s\n", buf);

		/* Convert the key into a string */
		key[0] = (unsigned char) k->key;
		ascii_to_text(buf, sizeof(buf), key);
		file_putf(fff, "C:%d:%s\n", mode, buf);

		file_putf(fff, "\n");
	}

}

