/**
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_IO_H
#define INCLUDED_BORG_IO_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "../ui-event.h"

/*
 * Size of Keypress buffer
 */
#define KEY_SIZE 8192

/*
 * confirm selected target
 */
extern bool borg_confirm_target;

/*
 * Query the "attr/chars" at a given location on the screen
 */
extern errr borg_what_text(int x, int y, int n, uint8_t *a, char *s);

/*
 * Memorize a message, Log it, Search it, and Display it in pieces
 */
extern void borg_note(const char *what);

/*
 * Memorize a warning, Log it, Search it, and Display it in pieces
 */
extern void borg_warning(const char *what);

/*
 * Add a keypress to the "queue" (fake event)
 */
extern errr borg_keypress(keycode_t k);

/*
 * Add a keypresses to the "queue" (fake event)
 */
extern errr borg_keypresses(const char *str);

/*
 * Add a keypresses to history
 */
extern struct keypress save_keypress_history(struct keypress kp);

/*
 * Dump keypress history
 */
extern void borg_dump_recent_keys(int num);

/*
 * Get the next Borg keypress
 */
extern keycode_t borg_inkey(bool take);

/*
 * Clear all Borg keypress
 */
extern void borg_flush(void);

/*
 *  Save and retrieve direction when the command may be ambiguous.
 */
extern void      borg_queue_direction(keycode_t k);
extern keycode_t borg_get_queued_direction(void);

/*
 * Handle non-ASCII characters in some names
 */
extern char *borg_massage_special_chars(char *name);

extern void borg_init_io(void);
extern void borg_free_io(void);

#endif
#endif
