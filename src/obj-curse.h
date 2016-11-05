/**
 * \file obj-curse.h
 * \brief functions to deal with object curses
 *
 * Copyright (c) 2016 Nick McConnell
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
#ifndef INCLUDED_OBJ_CURSE_H
#define INCLUDED_OBJ_CURSE_H

/**
 * Curse type
 */
struct curse {
	struct curse *next;
	char *name;
	bool *poss;
	struct object *obj;
	int power;
	char *desc;
};

extern struct curse *curses;

int lookup_curse(const char *name);
void copy_curse(struct curse **dest, struct curse *src, bool randomise,
				bool new);
void free_curse(struct curse *source, bool free_object, bool free_eff);
bool curses_are_equal(struct curse *curse1, struct curse *curse2);
bool append_curse(struct curse **current, int pick, int power);
void wipe_curses(struct curse *curses);
bool do_curse_effect(struct curse *curse);

#endif /* !INCLUDED_OBJ_CURSE_H */
