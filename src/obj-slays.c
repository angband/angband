/**
 * \file obj-slays.c
 * \brief Functions for manipulating slays/brands
 *
 * Copyright (c) 2010 Chris Carr and Peter Denison
 * Copyright (c) 2014 Nick McConnell
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
#include "init.h"
#include "mon-lore.h"
#include "obj-desc.h"
#include "obj-init.h"
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "obj-util.h"


struct slay *slays;
struct brand *brands;

/**
 * Check if two slays affect the same set of monsters
 */
bool same_monsters_slain(int slay1, int slay2)
{
	if (slays[slay1].race_flag != slays[slay2].race_flag) return false;
	if (!slays[slay1].base && !slays[slay2].base) return true;
	if (slays[slay1].base && !slays[slay2].base) return false;
	if (!slays[slay1].base && slays[slay2].base) return false;
	if (streq(slays[slay1].base, slays[slay2].base)) return true;
	return false;
}

/**
 * Add all the slays from one structure to another
 *
 * \param dest the address the slays are going to
 * \param source the slays being copied
 */
void copy_slays(bool **dest, bool *source)
{
	int i;

	if (!source) return;

	if (!(*dest))
		*dest = mem_zalloc(z_info->slay_max * sizeof(bool));

	for (i = 0; i < z_info->slay_max; i++)
		(*dest)[i] |= source[i];
}

/**
 * Add all the brands from one structure to another
 *
 * \param dest the address the brands are going to
 * \param source the brands being copied
 */
void copy_brands(bool **dest, bool *source)
{
	int i;

	if (!source) return;

	if (!(*dest))
		*dest = mem_zalloc(z_info->brand_max * sizeof(bool));

	for (i = 0; i < z_info->brand_max; i++)
		(*dest)[i] |= source[i];
}

/**
 * Append a random brand, currently to a randart
 * This will later change so that selection is done elsewhere
 *
 * \param current the list of brands the object already has
 * \param name the name to report for randart logging
 */
bool append_random_brand(bool **current, char **name)
{
	int i, pick;

	pick = randint1(z_info->brand_max - 1);
	*name = brands[pick].name;

	/* No existing brands means OK to add */
	if (!(*current)) {
		*current = mem_zalloc(z_info->brand_max * sizeof(bool));
		(*current)[pick] = true;
		return true;
	}

	/* Check the existing brands for name matches */
	for (i = 1; i < z_info->brand_max; i++) {
		if ((*current)[i]) {
			/* If we get the same race, check the multiplier */
			if (streq(brands[i].name, brands[pick].name)) {
				/* Same multiplier or smaller, fail */
				if (brands[pick].multiplier <= brands[i].multiplier)
					return false;

				/* Greater multiplier, replace and accept */
				(*current)[i] = false;
				(*current)[pick] = true;
				return true;
			}
		}
	}

	/* We can add the new one now */
	(*current)[pick] = true;

	return true;
}

/**
 * Append a random slay, currently to a randart
 * This will later change so that selection is done elsewhere
 *
 * \param current the list of slays the object already has
 * \param name the name to report for randart logging
 */
bool append_random_slay(bool **current, char **name)
{
	int i, pick;

	pick = randint1(z_info->slay_max - 1);
	*name = slays[pick].name;

	/* No existing slays means OK to add */
	if (!(*current)) {
		*current = mem_zalloc(z_info->slay_max * sizeof(bool));
		(*current)[pick] = true;
		return true;
	}

	/* Check the existing slays for base/flag matches */
	for (i = 1; i < z_info->slay_max; i++) {
		if ((*current)[i]) {
			/* If we get the same race, check the multiplier */
			if (streq(slays[i].name, slays[pick].name) &&
				(slays[i].race_flag == slays[pick].race_flag)) {
				/* Same multiplier or smaller, fail */
				if (slays[pick].multiplier <= slays[i].multiplier)
					return false;

				/* Greater multiplier, replace and accept */
				(*current)[i] = false;
				(*current)[pick] = true;
				return true;
			}
		}
	}

	/* We can add the new one now */
	(*current)[pick] = true;

	return true;
}

/**
 * Count a set of brands
 * \param brands The brands to count.
 */
int brand_count(bool *brands)
{
	int i, count = 0;

	/* Count the brands */
	for (i = 0; i < z_info->brand_max; i++) {
		if (brands[i]) {
			count++;
		}
	}

	return count;
}


/**
 * Count a set of slays
 * \param slays The slays to count.
 */
int slay_count(bool *slays)
{
	int i, count = 0;

	/* Count the slays */
	for (i = 0; i < z_info->slay_max; i++) {
		if (slays[i]) {
			count++;
		}
	}

	return count;
}

/**
 * React to slays which hurt a monster
 * 
 * \param slay is the slay we're testing for effectiveness
 * \param mon is the monster we're testing for being slain
 */
bool react_to_specific_slay(struct slay *slay, const struct monster *mon)
{
	if (!slay->name) return false;
	if (!mon->race->base) return false;

	/* Check the race flag */
	if (rf_has(mon->race->flags, slay->race_flag))
		return true;

	/* Check for monster base */
	if (slay->base && streq(slay->base, mon->race->base->name))
		return true;

	return false;
}


/**
 * Extract the multiplier from a given object hitting a given monster.
 *
 * \param obj is the object being used to attack
 * \param mon is the monster being attacked
 * \param brand_used is the brand that gave the best multiplier, or NULL
 * \param slay_used is the slay that gave the best multiplier, or NULL
 * \param verb is the verb used in the attack ("smite", etc)
 * \param real is whether this is a real attack (where we update lore) or a
 *  simulation (where we don't)
 */
void improve_attack_modifier(struct object *obj, const struct monster *mon,
							 int *brand_used, int *slay_used, char *verb,
							 bool range, bool real)
{
	int i, best_mult = 1;
	struct monster_lore *lore = get_lore(mon->race);

	if (!obj) return;

	/* Brands */
	for (i = 1; i < z_info->brand_max; i++) {
		struct brand *b = &brands[i];
		if (!obj->brands || !obj->brands[i]) continue;
 
		/* Is the monster is vulnerable? */
		if (!rf_has(mon->race->flags, b->resist_flag)) {
			/* Record the best multiplier */
			if (best_mult < b->multiplier) {
				best_mult = b->multiplier;
				*brand_used = i;
				my_strcpy(verb, b->verb, 20);
				if (range)
					my_strcat(verb, "s", 20);
			}
			/* Learn from real attacks */
			if (real) {
				/* Learn about the brand */
				object_learn_brand(player, obj, i);

				/* Learn about the monster */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, b->resist_flag);
			}
		} else if (real && player_knows_brand(player, i)) {
				/* Learn about resistant monsters */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, b->resist_flag);
		}
	}

	/* Slays */
	for (i = 1; i < z_info->slay_max; i++) {
		struct slay *s = &slays[i];
		if (!obj->slays || !obj->slays[i]) continue;
 
		/* Is the monster is vulnerable? */
		if (react_to_specific_slay(s, mon)) {
			/* Record the best multiplier */
			if (best_mult < s->multiplier) {
				best_mult = s->multiplier;
				*brand_used = 0;
				*slay_used = i;
				if (range) {
					my_strcpy(verb, s->range_verb, 20);
				} else {
					my_strcpy(verb, s->melee_verb, 20);
				}
			}
			/* Learn from real attacks */
			if (real) {
				/* Learn about the slay */
				object_learn_slay(player, obj, i);

				/* Learn about the monster */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, s->race_flag);
			}
		} else if (real && player_knows_slay(player, i)) {
				/* Learn about resistant monsters */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, s->race_flag);
		}
	}
}


/**
 * React to slays which hurt a monster
 * 
 * \param obj is the object we're testing for slays
 * \param mon is the monster we're testing for being slain
 */
bool react_to_slay(struct object *obj, const struct monster *mon)
{
	int i;

	for (i = 0; i < z_info->slay_max; i++) {
		struct slay *s = &slays[i];;
		if (react_to_specific_slay(s, mon)) {
			return true;
		}
	}

	return false;
}
