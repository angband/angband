/**
 * \file ui-spell.h
 * \brief Spell UI handing
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

#ifndef INCLUDED_UI_SPELL_H
#define INCLUDED_UI_SPELL_H

struct player;

void textui_book_browse(const struct object *obj);
void textui_spell_browse(void);
int textui_get_spell_from_book(struct player *p, const char *verb,
	struct object *book, const char *error,
	bool (*spell_filter)(const struct player *p, int spell_index));
int textui_get_spell(struct player *p, const char *verb,
	item_tester book_filter, cmd_code cmd, const char *book_error,
	bool (*spell_filter)(const struct player *p, int spell_index),
	const char *spell_error, struct object **rtn_book);

#endif /* INCLUDED_UI_SPELL_H */
