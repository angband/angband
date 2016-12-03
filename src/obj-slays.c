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

struct brand_info {
	const char* name;
	const char *melee_verb;
	const char *melee_verb_weak;
	int resist_flag;
};

/**
 * Brand info
 * The verbs here probably should go in list-elements.h, when that's been
 * sorted properly, and there will also need to be a list of possibilities
 * in obj-randart.c
 */
static const struct brand_info brand_names[] = {
	{ "acid", "dissolve", "corrode", RF_IM_ACID },
	{ "lightning", "shock", "zap", RF_IM_ELEC },
	{ "fire", "burn", "singe", RF_IM_FIRE },
	{ "cold", "freeze", "chill", RF_IM_COLD },
	{ "poison", "poison", "sicken", RF_IM_POIS }
};

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
 * Copy all the brands from one structure to another
 *
 * \param dest the address the brands are going to
 * \param source the brands being copied
 */
void copy_brand(struct brand **dest, struct brand *source)
{
	struct brand *b = source;

	while (b) {
		struct brand *new_b, *check_b = *dest;
		bool dupe = false;

		/* Check for dupes */
		while (check_b) {
			if (check_b && streq(check_b->name, b->name) &&
				(check_b->element == b->element) &&
				(check_b->multiplier = b->multiplier)) {
				dupe = true;
				break;
			}
			check_b = check_b->next;
		}
		if (dupe) {
			b = b->next;
			continue;
		}

		/* Copy */
		new_b = mem_zalloc(sizeof *new_b);
		new_b->name = string_make(b->name);
		new_b->element = b->element;
		new_b->multiplier = b->multiplier;
		new_b->next = *dest;
		*dest = new_b;
		b = b->next;
	}
}

/**
 * Free all the brands in a structure
 *
 * \param source the brands being freed
 */
void free_brand(struct brand *source)
{
	struct brand *b = source, *b_next;
	while (b) {
		b_next = b->next;
		mem_free(b->name);
		mem_free(b);
		b = b_next;
	}
}

/**
 * Append a random brand, currently to a randart
 * This will later change so that selection is done elsewhere
 *
 * \param current the list of brands the object already has
 * \param name the name to report for randart logging
 */
bool append_random_brand(struct brand **current, char **name)
{
	int pick, mult = 2 + randint0(2);
	struct brand *b, *b_last = NULL;

	pick = randint0(N_ELEMENTS(brand_names));
	for (b = *current; b; b = b->next) {
		/* If we get the same one, check the multiplier */
		if (streq(b->name, brand_names[pick].name) && (b->element == pick)) {
			/* Same multiplier or smaller, fail */
			if (b->multiplier >= mult)
				return false;

			/* Greater multiplier, increase and accept */
			b->multiplier = mult;
			*name = b->name;
			return true;
		}

		/* Remember the last one */
		b_last = b;
	}

	/* We can add the new one now */
	b = mem_zalloc(sizeof(*b));
	b->name = string_make(brand_names[pick].name);
	b->element = pick;
	b->multiplier = mult;
	if (b_last)
		b_last->next = b;
	else
		*current = b;
	*name = b->name;

	/* Append to the game list */
	add_game_brand(b);

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
 * Count the brands in a struct brand
 * \param brands The brands to count.
 */
int brand_count(struct brand *brands)
{
	int count = 0;
	struct brand *b;

	/* Count the brands */
	for (b = brands; b; b = b->next)
		count++;

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
 * Collect the brands from a set of brands and an object into a linked array
 * \param b the set of brands (can be NULL)
 * \param obj the object (can be NULL)
 * \return a pointer to the first brand
 */
struct brand *brand_collect(struct brand *b, const struct object *obj)
{
	bool moved = false;
	struct brand *b_new;
	struct brand *b_last = NULL;
	struct brand *collected_brands = NULL;

	/* Use the object if there are no given brands */
	if (!b && obj) {
		b = obj->brands;
		moved = true;
	}

	/* Allocate and populate */
	while (b) {
		b_new = mem_zalloc(sizeof(*b_new));

		/* First one is what we will return */
		if (!collected_brands)
			collected_brands = b_new;

		/* Link the allocated brands in a chain */
		if (b_last)
			b_last->next = b_new;

		/* Fill in the data */
		b_new->name = string_make(b->name);
		b_new->element = b->element;
		b_new->multiplier = b->multiplier;

		/* Move to the next brand */
		b = b->next;

		/* Move to the object if we're done with the given brands */
		if (!b && !moved && obj) {
			b = obj->brands;
			moved = true;
		}

		b_last = b_new;
	}
	return collected_brands;
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
							 const struct brand **brand_used, int *slay_used, 
							 char *verb, bool range, bool real)
{
	struct brand *b;
	int i, best_mult = 1;
	struct monster_lore *lore = get_lore(mon->race);

	if (!obj) return;

	/* Brands */
	for (b = obj->brands; b; b = b->next) {
		/* Is the monster is vulnerable? */
		if (!rf_has(mon->race->flags,
					brand_names[b->element].resist_flag)) {
			/* Record the best multiplier */
			if (best_mult < b->multiplier) {
				best_mult = b->multiplier;
				*brand_used = b;
				if (b->multiplier < 3)
					my_strcpy(verb, brand_names[b->element].melee_verb_weak, 
							  20);
				else
					my_strcpy(verb, brand_names[b->element].melee_verb, 20);
				if (range)
					my_strcat(verb, "s", 20);
			}
			/* Learn from real attacks */
			if (real) {
				/* Learn about the brand */
				object_learn_brand(player, obj, b);

				/* Learn about the monster */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, brand_names[b->element].resist_flag);
			}
		} else if (real && player_knows_brand(player, b)) {
				/* Learn about resistant monsters */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, brand_names[b->element].resist_flag);
		}
	}

	/* Slays */
	for (i = 1; i < z_info->slay_max; i++) {
		struct slay *s = &slays[i];
 
		/* Is the monster is vulnerable? */
		if (react_to_specific_slay(s, mon)) {
			/* Record the best multiplier */
			if (best_mult < s->multiplier) {
				best_mult = s->multiplier;
				*brand_used = NULL;
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

/**
 * Determine whether two lists of brands are the same
 *
 * \param brand1 the lists being compared
 * \param brand2 the lists being compared
 */
bool brands_are_equal(struct brand *brand1, struct brand *brand2)
{
	struct brand *b1 = brand1, *b2;
	int count = 0, match = 0;

	while (b1) {
		count++;
		b2 = brand2;
		while (b2) {
			/* Count if the same */
			if (streq(b1->name, b2->name) && (b1->element == b2->element) &&
				(b1->multiplier == b2->multiplier))
				match++;
			b2 = b2->next;
		}

		/* Fail if we didn't find a match */
		if (match != count) return false;

		b1 = b1->next;
	}

	/* Now count back and make sure brand2 isn't strictly bigger */
	b2 = brand2;
	while (b2) {
		count--;
		b2 = b2->next;
	}

	if (count != 0) return false;

	return true;
}

/**
 * Remove a list of brands and de-allocate their memory
 */
void wipe_brands(struct brand *brands)
{
	struct brand *b = brands, *b1;
	while (b) {
		b1 = b;
		b = b->next;
		mem_free(b1);
	}
}
