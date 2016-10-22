/**
 * \file obj-curse.c
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
#include "angband.h"
#include "effects.h"
#include "init.h"
#include "object.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-pile.h"

struct curse *curses;

/**
 * Return the index of the curse with the given name
 */
int lookup_curse(const char *name)
{
	int i;

	for (i = 1; i < z_info->curse_max; i++) {
		struct curse *curse = &curses[i];
		if (curse->name && streq(name, curse->name))
			return i;
	}
	return 0;
}

/**
 * Copy all the curses from one structure to another
 *
 * \param dest the address the curses are going to
 * \param source the curses being copied
 * \param randomise whether some values on the curse object need randomising
 */
void copy_curse(struct curse **dest, struct curse *source, bool randomise,
				bool new)
{
	struct curse *c = source;

	while (c) {
		struct curse *new_c, *check_c = *dest;
		bool dupe = false;

		/* Check for dupes */
		while (check_c) {
			if (streq(check_c->name, c->name)) {
				dupe = true;
				break;
			}
			check_c = check_c->next;
		}
		if (dupe) {
			c = c->next;
			continue;
		}

		/* Copy */
		new_c = mem_zalloc(sizeof *new_c);
		new_c->name = string_make(c->name);
		new_c->power = c->power;
		if (c->obj) {
			new_c->obj = object_new();
			object_copy(new_c->obj, c->obj);

			/* Because struct object doesn't allow random values, generic curse
			 * objects represent these by fixed values to be randomised on
			 * application to an actual object */
			if (randomise) {
				int i;
				if (new_c->obj->to_h) {
					int to_h = new_c->obj->to_h;
					new_c->obj->to_h = SGN(to_h) * randint1(ABS(to_h));
				}
				if (new_c->obj->to_d) {
					int to_d = new_c->obj->to_d;
					new_c->obj->to_d = SGN(to_d) * randint1(ABS(to_d));
				}
				if (new_c->obj->to_a) {
					int to_a = new_c->obj->to_a;
					new_c->obj->to_a = SGN(to_a) * randint1(ABS(to_a));
				}
				for (i = 0; i < OBJ_MOD_MAX; i++) {
					if (new_c->obj->modifiers[i]) {
						int m = new_c->obj->modifiers[i];
						new_c->obj->modifiers[i] = SGN(m) * randint1(ABS(m));
					}
				}
			}

			/* Timeouts need to be set for new objects */
			if (new) {
				new_c->obj->timeout = randcalc(new_c->obj->time, 0, RANDOMISE);
			}
		}
		new_c->next = *dest;
		*dest = new_c;
		c = c->next;
	}
}

/**
 * Free all the curses in a structure
 *
 * \param source the slays being freed
 * \param complete whether to free the curse objects or not (we don't want to
 * if we are dealing with the known version of an object)
 */
void free_curse(struct curse *source, bool complete)
{
	struct curse *c = source, *c_next;
	while (c) {
		c_next = c->next;
		mem_free(c->desc);
		if (complete && c->obj) {
			free_effect(c->obj->effect);
			mem_free(c->obj);
		}
		string_free(c->name);
		mem_free(c);
		c = c_next;
	}
}

/**
 * Determine whether two lists of curses are the same
 *
 * \param curse1 the lists being compared
 * \param curse2 the lists being compared
 */
bool curses_are_equal(struct curse *curse1, struct curse *curse2)
{
	struct curse *c1 = curse1, *c2;
	int count = 0, match = 0;

	while (c1) {
		count++;
		c2 = curse2;
		while (c2) {
			/* Count if the same */
			if (streq(c1->name, c2->name))
				match++;
			c2 = c2->next;
		}

		/* Fail if we didn't find a match */
		if (match != count) return false;

		c1 = c1->next;
	}

	/* Now count back and make sure curse2 isn't strictly bigger */
	c2 = curse2;
	while (c2) {
		count--;
		c2 = c2->next;
	}

	if (count != 0) return false;

	return true;
}

/**
 * Append a given curse with a given power
 *
 * \param current the list of curses the object already has
 * \param pick the curse to append
 * \param power the power of the new curse
 */
bool append_curse(struct curse **current, int pick, int power)
{
	struct curse *c, *c_last = NULL;

	/* Adjust power if our pick is a duplicate */
	for (c = *current; c; c = c->next) {
		if (streq(c->name, curses[pick].name)) {
			if (power > c->power) {
				c->power = power;
				return true;
			} else {
				return false;
			}
		}
		c_last = c;
	}

	/* We can add the new one now */
	c = mem_zalloc(sizeof(*c));
	c->name = string_make(curses[pick].name);
	c->power = power;
	if (curses[pick].obj) {
		c->obj = object_new();
		object_copy(c->obj, curses[pick].obj);
		c->obj->timeout = randcalc(c->obj->time, 0, RANDOMISE);
	}
	if (c_last) {
		c_last->next = c;
	} else {
		*current = c;
	}

	return true;
}

/**
 * Do a curse effect.  
 */
bool do_curse_effect(struct curse *curse)
{
	struct effect *effect = curse->obj->effect;
	bool ident = false;
	bool was_aware = player_knows_curse(player, curse);
	int dir = randint1(8);

	if (dir > 4) {
		dir++;
	}
	if (curse->obj->effect_msg) {
		msgt(MSG_GENERIC, curse->obj->effect_msg);
	}
	effect_do(effect, NULL, &ident, was_aware, dir, 0, 0);
	curse->obj->timeout = randcalc(curse->obj->time, 0, RANDOMISE);
	return !was_aware && ident;
}
