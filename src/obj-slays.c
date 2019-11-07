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
#include "mon-predicate.h"
#include "obj-desc.h"
#include "obj-init.h"
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "obj-util.h"
#include "player-timed.h"


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
	int i, j;

	/* Check structures */
	if (!source) return;
	if (!(*dest)) {
		*dest = mem_zalloc(z_info->slay_max * sizeof(bool));
	}

	/* Copy */
	for (i = 0; i < z_info->slay_max; i++) {
		(*dest)[i] |= source[i];
	}

	/* Check for duplicates */
	for (i = 0; i < z_info->slay_max; i++) {
		for (j = 0; j < i; j++) {
			if ((*dest)[i] && (*dest)[j] && same_monsters_slain(i, j)) {
				if (slays[i].multiplier < slays[j].multiplier) {
					(*dest)[i] = false;
				} else {
					(*dest)[j] = false;
				}
			}
		}
	}
}

/**
 * Add all the brands from one structure to another
 *
 * \param dest the address the brands are going to
 * \param source the brands being copied
 */
void copy_brands(bool **dest, bool *source)
{
	int i, j;

	/* Check structures */
	if (!source) return;
	if (!(*dest))
		*dest = mem_zalloc(z_info->brand_max * sizeof(bool));

	/* Copy */
	for (i = 0; i < z_info->brand_max; i++)
		(*dest)[i] |= source[i];

	/* Check for duplicates */
	for (i = 0; i < z_info->brand_max; i++) {
		for (j = 0; j < i; j++) {
			if ((*dest)[i] && (*dest)[j] &&
				streq(brands[i].name, brands[j].name)) {
				if (brands[i].multiplier < brands[j].multiplier) {
					(*dest)[i] = false;
				} else {
					(*dest)[j] = false;
				}
			}
		}
	}
}

/**
 * Append a random brand, currently to a randart
 * This will later change so that selection is done elsewhere
 *
 * \param current the list of brands the object already has
 * \param name the name to report for randart logging
 */
bool append_random_brand(bool **current, struct brand **brand)
{
	int i, pick;

	pick = randint1(z_info->brand_max - 1);
	*brand = &brands[pick];

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
			if (streq(brands[i].name, (*brand)->name)) {
				/* Same multiplier or smaller, fail */
				if ((*brand)->multiplier <= brands[i].multiplier)
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
bool append_random_slay(bool **current, struct slay **slay)
{
	int i, pick;

	pick = randint1(z_info->slay_max - 1);
	*slay = &slays[pick];

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
			if (streq(slays[i].name, (*slay)->name) &&
				(slays[i].race_flag == (*slay)->race_flag)) {
				/* Same multiplier or smaller, fail */
				if ((*slay)->multiplier <= slays[i].multiplier)
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
 * Player has a temporary brand
 *
 * \param idx is the index of the brand
 */
bool player_has_temporary_brand(int idx)
{
	if (player->timed[TMD_ATT_ACID] && streq(brands[idx].code, "ACID_3")) {
		return true;
	}
	if (player->timed[TMD_ATT_ELEC] && streq(brands[idx].code, "ELEC_3")) {
		return true;
	}
	if (player->timed[TMD_ATT_FIRE] && streq(brands[idx].code, "FIRE_3")) {
		return true;
	}
	if (player->timed[TMD_ATT_COLD] && streq(brands[idx].code, "COLD_3")) {
		return true;
	}
	if (player->timed[TMD_ATT_POIS] && streq(brands[idx].code, "POIS_3")) {
		return true;
	}

	return false;
}

/**
 * Player has a temporary slay
 *
 * \param idx is the index of the slay
 */
bool player_has_temporary_slay(int idx)
{
	if (player->timed[TMD_ATT_EVIL] && streq(slays[idx].code, "EVIL_2")) {
		return true;
	}
	if (player->timed[TMD_ATT_DEMON] && streq(slays[idx].code, "DEMON_5")) {
		return true;
	}

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
							 bool range)
{
	int i, best_mult = 1;
	struct monster_lore *lore = get_lore(mon->race);

	/* Set the current best multiplier */
	if (*brand_used) {
		struct brand *b = &brands[*brand_used];
		if (!OPT(player, birth_percent_damage)) {
			best_mult = MAX(best_mult, b->multiplier);
		} else {
			best_mult = MAX(best_mult, b->o_multiplier);
		}
	} else if (*slay_used) {
		struct slay *s = &slays[*slay_used];
		if (!OPT(player, birth_percent_damage)) {
			best_mult = MAX(best_mult, s->multiplier);
		} else {
			best_mult = MAX(best_mult, s->o_multiplier);
		}
	}

	/* Brands */
	for (i = 1; i < z_info->brand_max; i++) {
		struct brand *b = &brands[i];
		if (obj) {
			/* Brand is on an object */
			if (!obj->brands || !obj->brands[i]) continue;
		} else {
			/* Temporary brand */
			if (!player_has_temporary_brand(i)) continue;
		}
 
		/* Is the monster is vulnerable? */
		if (!rf_has(mon->race->flags, b->resist_flag)) {
			int mult = OPT(player, birth_percent_damage) ?
				b->o_multiplier : b->multiplier;

			/* Record the best multiplier */
			if (best_mult < mult) {
				best_mult = mult;
				*brand_used = i;
				my_strcpy(verb, b->verb, 20);
				if (range)
					my_strcat(verb, "s", 20);
			}
			/* Learn about the brand */
			if (obj) {
				object_learn_brand(player, obj, i);
			}

			/* Learn about the monster */
			if (monster_is_visible(mon)) {
				rf_on(lore->flags, b->resist_flag);
			}
		} else if (player_knows_brand(player, i)) {
			/* Learn about resistant monsters */
			if (monster_is_visible(mon))
				rf_on(lore->flags, b->resist_flag);
		}
	}

	/* Slays */
	for (i = 1; i < z_info->slay_max; i++) {
		struct slay *s = &slays[i];
		if (obj) {
			/* Slay is on an object */
			if (!obj->slays || !obj->slays[i]) continue;
		} else {
			/* Temporary slay */
			if (!player_has_temporary_slay(i)) continue;
		}
 
		/* Is the monster is vulnerable? */
		if (react_to_specific_slay(s, mon)) {
			int mult = OPT(player, birth_percent_damage) ?
				s->o_multiplier : s->multiplier;

			/* Record the best multiplier */
			if (best_mult < mult) {
				best_mult = mult;
				*brand_used = 0;
				*slay_used = i;
				if (range) {
					my_strcpy(verb, s->range_verb, 20);
				} else {
					my_strcpy(verb, s->melee_verb, 20);
				}
			}
			/* Learn about the monster */
			if (monster_is_visible(mon)) {
				rf_on(lore->flags, s->race_flag);
				/* Learn about the slay */
				if (obj) {
					object_learn_slay(player, obj, i);
				}
			}
		} else if (player_knows_slay(player, i)) {
			/* Learn about resistant monsters */
			if (monster_is_visible(mon))
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

	if (!obj->slays) return false;

	for (i = 0; i < z_info->slay_max; i++) {
		struct slay *s = &slays[i];
		if (obj->slays[i] && react_to_specific_slay(s, mon)) {
			return true;
		}
	}

	return false;
}
