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
#include "obj-gear.h"
#include "obj-init.h"
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "obj-tval.h"
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
int brand_count(const bool *brands_on)
{
	int i, count = 0;

	/* Count the brands */
	for (i = 0; i < z_info->brand_max; i++) {
		if (brands_on[i]) {
			count++;
		}
	}

	return count;
}


/**
 * Count a set of slays
 * \param slays The slays to count.
 */
int slay_count(const bool *slays_on)
{
	int i, count = 0;

	/* Count the slays */
	for (i = 0; i < z_info->slay_max; i++) {
		if (slays_on[i]) {
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
static bool react_to_specific_slay(struct slay *slay, const struct monster *mon)
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
 * \param p is the player
 * \param idx is the index of the brand
 */
bool player_has_temporary_brand(const struct player *p, int idx)
{
	int i = 0;

	while (i < TMD_MAX) {
		if (timed_effects[i].temp_brand == idx && p->timed[i]) {
			return true;
		}
		++i;
	}
	return false;
}

/**
 * Player has a temporary slay
 *
 * \param p is the player
 * \param idx is the index of the slay
 */
bool player_has_temporary_slay(const struct player *p, int idx)
{
	int i = 0;

	while (i < TMD_MAX) {
		if (timed_effects[i].temp_slay == idx && p->timed[i]) {
			return true;
		}
		++i;
	}
	return false;
}

/**
 * Return the multiplicative factor for a brand hitting a given monster.
 * Account for any elemental vulnerabilities but not for resistances.
 */
int get_monster_brand_multiplier(const struct monster *mon,
		const struct brand *b, bool is_o_combat)
{
	int mult = (is_o_combat) ? b->o_multiplier : b->multiplier;

	if (b->vuln_flag && rf_has(mon->race->flags, b->vuln_flag)) {
		/*
		 * If especially vulnerable, apply a factor of two to the
		 * extra damage from the brand.
		 */
		if (is_o_combat) {
			mult = 2 * (mult - 10) + 10;
		} else {
			mult *= 2;
		}
	}

	return mult;
}

/**
 * Extract the multiplier from a given object hitting a given monster.
 *
 * \param player is the player performing the attack
 * \param obj is the object being used to attack
 * \param mon is the monster being attacked
 * \param brand_used is the brand that gave the best multiplier, or NULL
 * \param slay_used is the slay that gave the best multiplier, or NULL
 * \param verb is the verb used in the attack ("smite", etc)
 * \param range is whether or not this is a ranged attack
 */
void improve_attack_modifier(struct player *p, struct object *obj,
	const struct monster *mon, int *brand_used, int *slay_used,
	char *verb, bool range)
{
	bool pctdam = OPT(p, birth_percent_damage);
	int i, best_mult = 1;

	/* Set the current best multiplier */
	if (*brand_used) {
		struct brand *b = &brands[*brand_used];
		best_mult = MAX(best_mult,
			get_monster_brand_multiplier(mon, b, pctdam));
	} else if (*slay_used) {
		struct slay *s = &slays[*slay_used];
		int mult = (pctdam) ? s->o_multiplier : s->multiplier;
		best_mult = MAX(best_mult, mult);
	}

	/* Brands */
	for (i = 1; i < z_info->brand_max; i++) {
		struct brand *b = &brands[i];
		if (obj) {
			/* Brand is on an object */
			if (!obj->brands || !obj->brands[i]) continue;
		} else {
			/* Temporary brand */
			if (!player_has_temporary_brand(p, i)) continue;
		}
 
		/* Is the monster vulnerable? */
		if (!rf_has(mon->race->flags, b->resist_flag)) {
			int mult = get_monster_brand_multiplier(mon, b, pctdam);

			/* Record the best multiplier */
			if (best_mult < mult) {
				best_mult = mult;
				*brand_used = i;
				my_strcpy(verb, b->verb, 20);
				if (range)
					my_strcat(verb, "s", 20);
			}
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
			if (!player_has_temporary_slay(p, i)) continue;
		}
 
		/* Is the monster is vulnerable? */
		if (react_to_specific_slay(s, mon)) {
			int mult = pctdam ? s->o_multiplier : s->multiplier;

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


/**
 * Help learn_brand_slay_{melee,launch,throw}().
 *
 * \param p is the player learning from the experience.
 * \param obj1 is an object directly involved in the attack.
 * \param obj2 is an auxiliary object (i.e. a launcher) involved in the attack.
 * \param mon is the monster being attacked.
 * \param allow_off is whether to include brands or slays from equipment that
 * isn't a weapon or launcher.
 * \param allow_temp is whether to include temporary brands or slays.
 */
static void learn_brand_slay_helper(struct player *p, struct object *obj1,
		struct object *obj2, const struct monster *mon, bool allow_off,
		bool allow_temp)
{
	struct monster_lore *lore = get_lore(mon->race);
	struct object **objs = mem_alloc((2 + p->body.count) * sizeof(*objs));
	int i;

	/* Handle brands. */
	for (i = 1; i < z_info->brand_max; i++) {
		int n = 0, j;
		struct brand *b;

		/* Check the objects directly involved. */
		if (obj1 && obj1->brands && obj1->brands[i]) {
			objs[n++] = obj1;
		}
		if (obj2 && obj2->brands && obj2->brands[i]) {
			objs[n++] = obj2;
		}

		/* Check for an off-weapon brand. */
		if (allow_off) {
			for (j = 0; j < p->body.count; ++j) {
				struct object *obj = slot_object(p, j);

				if (obj && obj->brands && obj->brands[i]
						&& !tval_is_weapon(obj)
						&& !tval_is_launcher(obj)) {
					objs[n++] = obj;
				}
			}
		}

		/*
		 * Check for the temporary brand (only relevant if the brand
		 * is not already present).
		 */
		if (n == 0 && allow_temp && !player_has_temporary_brand(p, i)) {
			continue;
		}

		b = &brands[i];
		if (!rf_has(mon->race->flags, b->resist_flag)) {
			/* Learn about the equipment. */
			for (j = 0; j < n; ++j) {
				object_learn_brand(p, objs[j], i);
			}

			/* Learn about the monster. */
			lore_learn_flag_if_visible(lore, mon, b->resist_flag);
			if (b->vuln_flag) {
				lore_learn_flag_if_visible(lore, mon,
					b->vuln_flag);
			}
		} else if (player_knows_brand(p, i)) {
			/* Learn about the monster. */
			lore_learn_flag_if_visible(lore, mon, b->resist_flag);
			if (b->vuln_flag) {
				lore_learn_flag_if_visible(lore, mon,
					b->vuln_flag);
			}
		}
	}

	/* Handle slays. */
	for (i = 1; i < z_info->slay_max; ++i) {
		int n = 0, j;
		struct slay *s;

		/* Check the objects directly involved. */
		if (obj1 && obj1->slays && obj1->slays[i]) {
			objs[n++] = obj1;
		}
		if (obj2 && obj2->slays && obj2->slays[i]) {
			objs[n++] = obj2;
		}

		/* Check for an off-weapon slay. */
		if (allow_off) {
			for (j = 0; j < p->body.count; ++j) {
				struct object *obj = slot_object(p, j);

				if (obj && obj->slays && obj->slays[i]
						&& !tval_is_weapon(obj)
						&& !tval_is_launcher(obj)) {
					objs[n++] = obj;
				}
			}
		}

		/*
		 * Check for the temporary slay (only relevant if the slay
		 * is not already present.
		 */
		if (n == 0 && allow_temp && !player_has_temporary_slay(p, i)) {
			continue;
		}

		s = &slays[i];
		if (react_to_specific_slay(s, mon)) {
			/* Learn about the monster. */
			lore_learn_flag_if_visible(lore, mon, s->race_flag);
			if (monster_is_visible(mon)) {
				/* Learn about the equipment. */
				for (j = 0; j < n; ++j) {
					object_learn_slay(p, objs[j], i);
				}
			}
		} else if (player_knows_slay(p, i)) {
			/* Learn about unaffected monsters. */
			lore_learn_flag_if_visible(lore, mon, s->race_flag);
		}
	}

	mem_free(objs);
}


/**
 * Learn about object and monster properties related to slays and brands from
 * a melee attack.
 *
 * \param p is the player learning from the experience.
 * \param weapon is the equipped weapon used in the attack; this is a parameter
 * to allow for the possibility of dual-wielding or body types with multiple
 * equipped weapons.  May be NULL for an unarmed attack.
 * \param mon is the monster being attacked.
 */
void learn_brand_slay_from_melee(struct player *p, struct object *weapon,
		const struct monster *mon)
{
	learn_brand_slay_helper(p, weapon, NULL, mon, true, true);
}


/**
 * Learn about object and monster properties related to slays and brands
 * from a ranged attack with a missile launcher.
 *
 * \param p is the player learning from the experience.
 * \param missile is the missile used in the attack.  Must not be NULL.
 * \param launcher is the launcher used in the attack; this is a parameter
 * to allow for body types with multiple equipped launchers.  Must not be NULL.
 * \param mon is the monster being attacked.
 */
void learn_brand_slay_from_launch(struct player *p, struct object *missile,
		struct object *launcher, const struct monster *mon)
{
	assert(missile && launcher);
	learn_brand_slay_helper(p, missile, launcher, mon, false, false);
}


/**
 * Learn about object and monster properties related to slays and brands
 * from a ranged attack with a thrown object.
 *
 * \param p is the player learning from the experience.
 * \param missile is the missile used in the attack.  Must not be NULL.
 * \param launcher is the launcher used in the attack; this is a parameter
 * to allow for body types with multiple equipped launchers.
 * \param mon is the monster being attacked.
 */
void learn_brand_slay_from_throw(struct player *p, struct object *missile,
		const struct monster *mon)
{
	assert(missile);
	learn_brand_slay_helper(p, missile, NULL, mon, false, false);
}
