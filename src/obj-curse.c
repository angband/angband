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
#include "player-util.h"

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
	if (c->obj->effect && c->obj->effect->index
			== effect_lookup("TIMED_INC")) {
		int idx = c->obj->effect->subtype;
		const struct timed_failure *f;

		assert(idx >= 0 && idx < TMD_MAX);
		f = timed_effects[idx].fail;
		while (f) {
			if (f->code == TMD_FAIL_FLAG_OBJECT) {
				if (of_has(obj->flags, f->idx)) {
					check_object_curses(obj);
					return false;
				}
			} else if (f->code == TMD_FAIL_FLAG_RESIST) {
				assert(f->idx >= 0 && f->idx < ELEM_MAX);
				if (obj->el_info[f->idx].res_level > 0) {
					check_object_curses(obj);
					return false;
				}
			} else if (f->code == TMD_FAIL_FLAG_VULN) {
				assert(f->idx >= 0 && f->idx < ELEM_MAX);
				if (obj->el_info[f->idx].res_level < 0) {
					check_object_curses(obj);
					return false;
				}
			}
			f = f->next;
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
 * Remove a curse from an object.
 *
 * \param obj is the object to manipulate
 * \param pick is the index of the curse to be removed
 * \param message if true, causes a message to be displayed if a curse was
 * removed
 * \return true if the object had the given curse; otherwise, return false
 */
bool remove_object_curse(struct object *obj, int pick, bool message)
{
	struct curse_data *c = &obj->curses[pick];
	bool result;

	if (c->power > 0) {
		result = true;
		c->power = 0;
		c->timeout = 0;
		/* Remove the curses array if that was the last curse. */
		check_object_curses(obj);
		if (message) {
			msg("The %s curse is removed!", curses[pick].name);
		}
	} else {
		result = false;
	}
	return result;
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
	if (c->obj->effect && c->obj->effect->index
			== effect_lookup("TIMED_INC")) {
		int idx = c->obj->effect->subtype;
		const struct timed_failure *f;

		assert(idx >= 0 && idx < TMD_MAX);
		f = timed_effects[idx].fail;
		while (f) {
			if (f->code == TMD_FAIL_FLAG_OBJECT) {
				if (of_has(art->flags, f->idx)) {
					check_artifact_curses(art);
					return true;
				}
			} else if (f->code == TMD_FAIL_FLAG_RESIST) {
				assert(f->idx >= 0 && f->idx < ELEM_MAX);
				if (art->el_info[f->idx].res_level > 0) {
					check_artifact_curses(art);
					return true;
				}
			} else if (f->code == TMD_FAIL_FLAG_VULN) {
				assert(f->idx >= 0 && f->idx < ELEM_MAX);
				if (art->el_info[f->idx].res_level < 0) {
					check_artifact_curses(art);
					return true;
				}
			}
			f = f->next;
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
		msgt(MSG_GENERIC, "%s", curse->obj->effect_msg);
	}
	effect_do(effect, source_object(obj), NULL, &ident, was_aware, dir, 0, 0, NULL);
	curse->obj->known->effect = curse->obj->effect;
	disturb(player);
	return !was_aware && ident;
}

/**
 * Modify the given weight for the ith curse.
 *
 * \param i is the index of the curse; it must be greater than or equal to zero
 * and less than z_info->curse_max.
 * \param weight is the weight, in 1/10ths of a pound, to modify.  It will be
 * coerced to be non-negative.
 * \return the modified weight in 1/10ths of a pound.
 */
int16_t modify_weight_for_curse(int i, int16_t weight)
{
	const struct object *curse_obj;
	int16_t result;

	assert(i >= 0 && i < z_info->curse_max);
	curse_obj = curses[i].obj;

	if (of_has(curse_obj->flags, OF_MULTIPLY_WEIGHT)) {
		/*
		 * Apply a multiplicative factor of curse_obj->weight over 100
		 * and round to the nearest integer.  Coerce the incoming weight
		 * to be at least one if the multiplicative factor is greater
		 * than one.  That allows multiplicative factors to be of some
		 * use with otherwise weightless items.
		 */
		int32_t scaled, q;

		assert(curse_obj->weight >= 0);
		if (curse_obj->weight > 100) {
			scaled = MAX(weight, 1);
		} else {
			scaled = MAX(weight, 0);
		}
		scaled *= curse_obj->weight;
		q = scaled / 100;
		if (q < 32767) {
			result = q;
			if (scaled % 100 >= 50) {
				++result;
			}
		} else {
			result = 32767;
		}
	} else {
		weight = MAX(0, weight);
		if (curse_obj->weight < 0) {
			result = weight + curse_obj->weight;
			if (result < 0) {
				result = 0;
			}
		} else {
			result = (weight < 32767 - curse_obj->weight) ?
				weight + curse_obj->weight : 32767;
		}
	}

	return result;
}

/**
 * Merge the attributes of active curses into the base attributes of an object.
 *
 * \param i will, if non-negative, cause the ith curse in the curses array
 * (i >= 0 and i < z_info->curse_max) to be ignored when merging attributes
 * \param obj is the object record to modify.
 *
 * The curses field of obj is left as is; the caller will likely want to clear
 * it (memfree(obj->curses); obj->curses == NULL;) since the curse attributes
 * are included directly in the other fields of obj.
 * If there is a reference to a known object in obj, its attributes are not
 * modified here.
 * The brands and slays are not modified: the logic in
 * improve_attack_modifier() does not look at curse objects for brands or
 * slays and the data file for curses does not allow them to be specified.
 * Object destruction code does not look at the element flags for curses so
 * those are not merged into the base attributes.
 */
void apply_curse_attributes(int i, struct object *obj)
{
	int j;

	if (!obj->curses) {
		/* There are no curses so there is nothing to merge. */
		return;
	}

	for (j = 0; j < z_info->curse_max; ++j) {
		const struct object *curse_obj;
		int k;

		/* Ignore the requested curse and any that are not active. */
		if (j == i || !obj->curses[j].power) {
			continue;
		}

		curse_obj = curses[j].obj;

		/* The curse may modify the object's weight. */
		obj->weight = modify_weight_for_curse(j, obj->weight);

		/*
		 * The logic in player-calcs.c will add the base AC from a curse
		 * to the total AC of the player so combine that base AC with
		 * that of the object.  The data file for curses does not allow
		 * specifying the base AC for a curse.
		 */
		obj->ac = add_guardi16(obj->ac, curse_obj->ac);

		/* Curses can adjust the AC, hit and to-dam modifiers. */
		obj->to_a = add_guardi16(obj->to_a, curse_obj->to_a);
		obj->to_h = add_guardi16(obj->to_h, curse_obj->to_h);
		obj->to_d = add_guardi16(obj->to_d, curse_obj->to_d);

		/* The curse may extend the object's flags. */
		of_union(obj->flags, curse_obj->flags);

		/*
		 * The curse's modifiers combine additively with those from
		 * the object.
		 */
		for (k = 0; k < OBJ_MOD_MAX; ++k) {
			obj->modifiers[k] = add_guardi16(obj->modifiers[k],
				curse_obj->modifiers[k]);
		}

		/*
		 * Resistances combine with the standard logic for combining
		 * them.
		 */
		for (k = 0; k < ELEM_MAX; ++k) {
			if (obj->el_info[k].res_level >= 3) {
				/*
				 * Already immune; the curse can not change
				 * that.
				 */
				continue;
			}
			if (obj->el_info[k].res_level == 1) {
				/*
				 * Has resistance.  An immunity will override
				 * that.  A resistance or no resistance on
				 * the curse will do nothing.  A vulnerability
				 * will convert the resistance to
				 * vulnerability + resistance.
				 */
				if (curse_obj->el_info[k].res_level >= 3) {
					obj->el_info[k].res_level = 3;
				} else if (curse_obj->el_info[k].res_level < 0) {
					obj->el_info[k].res_level = -32768;
				}
			} else if (obj->el_info[k].res_level == -32768) {
				/*
				 * Combined result so far is a vulnerability +
				 * resistance.  That only changes if there is
				 * an immunity.
				 */
				if (curse_obj->el_info[k].res_level >= 3) {
					obj->el_info[k].res_level = 3;
				}
			} else if (obj->el_info[k].res_level < 0) {
				/*
				 * Has vulnerability.  An immunity will override
				 * that.  A vulnerability or no resistance on
				 * the curse will do nothing.  A resistance will
				 * convert the vulnerability to vulnerability +
				 * resistance.
				 */
				if (curse_obj->el_info[k].res_level >= 3) {
					obj->el_info[k].res_level = 3;
				} else if (curse_obj->el_info[k].res_level == 1) {
					obj->el_info[k].res_level = -32768;
				}
			} else {
				/*
				 * With no resistance in the base attributes,
				 * the merged result weill be the same as
				 * whatever is in the curse.
				 */
				assert(obj->el_info[k].res_level == 0);
				obj->el_info[k].res_level =
					curse_obj->el_info[k].res_level;
			}
		}
	}

	/*
	 * Fix up any resistances that ended up as vulnerability + resistance
	 * so they look like no resistance to the caller.
	 */
	for (j = 0; j < ELEM_MAX; ++j) {
		if (obj->el_info[j].res_level == -32768) {
			obj->el_info[j].res_level = 0;
		}
	}
}
