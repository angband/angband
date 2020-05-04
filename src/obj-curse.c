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
#include "obj-curse.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-timed.h"

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
 * Copy all the curses from a template to an actual object.
 *
 * \param obj the object the curses are being attached to
 * \param source the curses being copied
 */
void copy_curses(struct object *obj, int *source)
{
	int i;

	if (!source) return;

	if (!obj->curses) {
		obj->curses = mem_zalloc(z_info->curse_max * sizeof(struct curse_data));
	}

	for (i = 0; i < z_info->curse_max; i++) {
		if (!source[i]) continue;
		obj->curses[i].power = source[i];

		/* Timeouts need to be set for new objects */
		obj->curses[i].timeout = randcalc(curses[i].obj->time, 0, RANDOMISE);
	}
}

/**
 * Check whether two objects have the exact same curses
 *
 * \param obj1 the first object
 * \param obj2 the second object
 */
bool curses_are_equal(const struct object *obj1, const struct object *obj2)
{
	int i;

	if (!obj1->curses && !obj2->curses) return true;
	if (obj1->curses && !obj2->curses) return false;
	if (!obj1->curses && obj2->curses) return false;

	for (i = 0; i < z_info->curse_max; i++) {
		if (obj1->curses[i].power != obj2->curses[i].power) return false;
	}

	return true;
}

/**
 * Detect if a curse is in the conflict list of another curse
 */
static bool curses_conflict(int first, int second)
{
	struct curse *c = &curses[first];
	char buf[80] = "|";

	/* First curse has no conflicts */
	if (!c->conflict) {
		return false;
	}

	/* Build the conflict strong and search for it */
	my_strcat(buf, curses[second].name, sizeof(buf));
	my_strcat(buf, "|", sizeof(buf));
	if (strstr(c->conflict, buf)) {
		return true;
	}

	return false;
}

/**
 * Check an object for active curses, and remove the "curses" field if
 * none is found
 */
static void check_object_curses(struct object *obj)
{
	int i;

	/* Look for a valid curse, return if one found */
	for (i = 0; i < z_info->curse_max; i++) {
		if (obj->curses[i].power) {
			return;
		}
	}

	/* Free the curse structure */
	mem_free(obj->curses);
	obj->curses = NULL;
}


/**
 * Append a given curse with a given power to an object
 *
 * \param the object to curse
 * \param pick the curse to append
 * \param power the power of the new curse
 */
bool append_object_curse(struct object *obj, int pick, int power)
{
	struct curse *c = &curses[pick];
	int i;

	if (!obj->curses)
		obj->curses = mem_zalloc(z_info->curse_max * sizeof(struct curse_data));

	/* Reject conflicting curses */
	for (i = 0; i < z_info->curse_max; i++) {
		if (obj->curses[i].power && curses_conflict(i, pick)) {
			check_object_curses(obj);
			return false;
		}
	}

	/* Reject curses with effects foiled by an existing object property */
	if (c->obj->effect && c->obj->effect->index == effect_lookup("TIMED_INC")) {
		int idx = c->obj->effect->subtype;
		struct timed_effect_data *status;
		assert(idx < TMD_MAX);
		status = &timed_effects[idx];
		if (status->fail_code == TMD_FAIL_FLAG_OBJECT) {
			if (of_has(obj->flags, status->fail)) {
				check_object_curses(obj);
				return false;
			}
		} else if (status->fail_code == TMD_FAIL_FLAG_RESIST) {
			if (obj->el_info[status->fail].res_level > 0) {
				check_object_curses(obj);
				return false;
			}
		} else if (status->fail_code == TMD_FAIL_FLAG_VULN) {
			if (obj->el_info[status->fail].res_level < 0) {
				check_object_curses(obj);
				return false;
			}
		}
	}

	/* Reject curses which explicitly conflict with an object property */
	for (i = of_next(c->conflict_flags, FLAG_START); i != FLAG_END;
		 i = of_next(c->conflict_flags, i + 1)) {
		if (of_has(obj->flags, i)) {
			check_object_curses(obj);
			return false;
		}
	}

	/* Adjust power if our pick is a duplicate */
	if (power > obj->curses[pick].power) {
		obj->curses[pick].power = power;
		obj->curses[pick].timeout = randcalc(c->obj->time, 0, RANDOMISE);
		return true;
	}

	check_object_curses(obj);
	return false;
}

/**
 * Check an artifact template for active curses, remove conflicting curses, and
 * remove the "curses" field if no curses remain
 */
void check_artifact_curses(struct artifact *art)
{
	int i;

	/* Look for a valid curse, return if one found */
	for (i = 0; i < z_info->curse_max; i++) {
		if (art->curses && art->curses[i]) {
			return;
		}
	}

	/* Free the curse structure */
	mem_free(art->curses);
	art->curses = NULL;
}

/**
 *
 */
bool artifact_curse_conflicts(struct artifact *art, int pick)
{
	struct curse *c = &curses[pick];
	int i;

	/* Reject curses with effects foiled by an existing artifact property */
	if (c->obj->effect && c->obj->effect->index == effect_lookup("TIMED_INC")) {
		int idx = c->obj->effect->subtype;
		struct timed_effect_data *status;
		assert(idx < TMD_MAX);
		status = &timed_effects[idx];
		if (status->fail_code == TMD_FAIL_FLAG_OBJECT) {
			if (of_has(art->flags, status->fail)) {
				check_artifact_curses(art);
				return true;
			}
		} else if (status->fail_code == TMD_FAIL_FLAG_RESIST) {
			if (art->el_info[status->fail].res_level > 0) {
				check_artifact_curses(art);
				return true;
			}
		} else if (status->fail_code == TMD_FAIL_FLAG_VULN) {
			if (art->el_info[status->fail].res_level < 0) {
				check_artifact_curses(art);
				return true;
			}
		}
	}

	/* Reject curses which explicitly conflict with an artifact property */
	for (i = of_next(c->conflict_flags, FLAG_START); i != FLAG_END;
		 i = of_next(c->conflict_flags, i + 1)) {
		if (of_has(art->flags, i)) {
			check_artifact_curses(art);
			return true;
		}
	}

	return false;
}

/**
 * Append a given curse with a given power to an artifact
 *
 * \param the artifact to curse
 * \param pick the curse to append
 * \param power the power of the new curse
 */
bool append_artifact_curse(struct artifact *art, int pick, int power)
{
	int i;

	if (!art->curses)
		art->curses = mem_zalloc(z_info->curse_max * sizeof(int));

	/* Reject conflicting curses */
	for (i = 0; i < z_info->curse_max; i++) {
		if (art->curses[i] && curses_conflict(i, pick)) {
			check_artifact_curses(art);
			return false;
		}
	}

	/* Reject curses with effects foiled by an existing artifact property */
	if (artifact_curse_conflicts(art, pick)) {
		check_artifact_curses(art);
		return false;
	}

	/* Adjust power if our pick is a duplicate */
	if (power > art->curses[pick]) {
		art->curses[pick] = power;
	}

	check_artifact_curses(art);
	return true;
}

/**
 * Do a curse effect.
 *
 * \param i the index into the curses array
 */
bool do_curse_effect(int i, struct object *obj)
{
	struct curse *curse = &curses[i];
	struct effect *effect = curse->obj->effect;
	bool ident = false;
	bool was_aware = player_knows_curse(player, i);
	int dir = randint1(8);

	if (dir > 4) {
		dir++;
	}
	if (curse->obj->effect_msg) {
		msgt(MSG_GENERIC, curse->obj->effect_msg);
	}
	effect_do(effect, source_object(obj), NULL, &ident, was_aware, dir, 0, 0, NULL);
	curse->obj->known->effect = curse->obj->effect;
	return !was_aware && ident;
}
