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
#include "obj-knowledge.h"
#include "obj-slays.h"
#include "obj-util.h"


/**
 * Cache of slay values (for object_power)
 */
static struct slay_cache *slay_cache;

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
const struct brand_info brand_names[] = {
	{ "acid", "dissolve", "corrode", RF_IM_ACID },
	{ "lightning", "shock", "zap", RF_IM_ELEC },
	{ "fire", "burn", "singe", RF_IM_FIRE },
	{ "cold", "freeze", "chill", RF_IM_COLD },
	{ "poison", "poison", "sicken", RF_IM_POIS }
};

struct slay_info {
	const char *name;
	int race_flag;
	int multiplier;
};

/**
 * Slay info
 * These should go into obj-randart.c, but can wait for brands to be done
 * (because they're more complicated) - NRM
 */
const struct slay_info slay_names[] = {
	{ "evil creatures", RF_EVIL, 2 },
	{ "animals", RF_ANIMAL, 2 },
	{ "orcs", RF_ORC, 3 },
	{ "trolls", RF_TROLL, 3 },
	{ "giants", RF_GIANT, 3 },
	{ "demons", RF_DEMON, 3 },
	{ "dragons", RF_DRAGON, 3 },
	{ "undead", RF_UNDEAD, 3 },
	{ "demons", RF_DEMON, 5 },
	{ "dragons", RF_DRAGON, 5 },
	{ "undead", RF_UNDEAD, 5 }
};

/**
 * Copy all the slays from one structure to another
 *
 * \param dest the address the slays are going to
 * \param source the slays being copied
 */
void copy_slay(struct slay **dest, struct slay *source)
{
	struct slay *s = source;

	while (s) {
		struct slay *new_s, *check_s = *dest;
		bool dupe = false;

		/* Check for dupes */
		while (check_s) {
			if (streq(check_s->name, s->name) &&
				(check_s->race_flag == s->race_flag) &&
				(check_s->multiplier = s->multiplier)) {
				dupe = true;
				break;
			}
			check_s = check_s->next;
		}
		if (dupe) {
			s = s->next;
			continue;
		}

		/* Copy */
		new_s = mem_zalloc(sizeof *new_s);
		new_s->name = string_make(s->name);
		new_s->race_flag = s->race_flag;
		new_s->multiplier = s->multiplier;
		new_s->next = *dest;
		*dest = new_s;
		s = s->next;
	}
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
 * Free all the slays in a structure
 *
 * \param source the slays being freed
 */
void free_slay(struct slay *source)
{
	struct slay *s = source, *s_next;
	while (s) {
		s_next = s->next;
		mem_free(s->name);
		mem_free(s);
		s = s_next;
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
bool append_random_slay(struct slay **current, char **name)
{
	int pick;
	struct slay *s, *s_last = NULL;

	pick = randint0(N_ELEMENTS(slay_names));
	for (s = *current; s; s = s->next) {
		/* If we get the same race, check the multiplier */
		if (streq(s->name, slay_names[pick].name) &&
			(s->race_flag == slay_names[pick].race_flag)) {
			/* Same multiplier or smaller, fail */
			if (slay_names[pick].multiplier <= s->multiplier)
				return false;

			/* Greater multiplier, increase and accept */
			s->multiplier = slay_names[pick].multiplier;
			*name = s->name;
			return true;
		}

		/* Remember the last one */
		s_last = s;
	}

	/* We can add the new one now */
	s = mem_zalloc(sizeof(*s));
	s->name = string_make(slay_names[pick].name);
	s->race_flag = slay_names[pick].race_flag;
	s->multiplier = slay_names[pick].multiplier;
	if (s_last)
		s_last->next = s;
	else
		*current = s;
	*name = s->name;

	/* Append to the game list */
	add_game_slay(s);

	return true;
}

/**
 * Count the brands in a struct brand
 * \param brands 
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
 * Count the slays in a struct slay
 * \param slays 
 */
int slay_count(struct slay *slays)
{
	int count = 0;
	struct slay *s;

	/* Count the slays */
	for (s = slays; s; s = s->next)
		count++;

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
 * Collect the slays from a set of slays and an object into a linked array
 * \param s the set of slays (can be NULL)
 * \param obj the object (can be NULL)
 * \return a pointer to the first slay
 */
struct slay *slay_collect(struct slay *s, const struct object *obj)
{
	bool moved = false;
	struct slay *s_new;
	struct slay *s_last = NULL;
	struct slay *collected_slays = NULL;

	/* Use the object if there are no given slays */
	if (!s && !moved && obj) {
		s = obj->slays;
		moved = true;
	}

	/* Allocate and populate */
	while (s) {
		s_new = mem_zalloc(sizeof(*s_new));

		/* First one is what we will return */
		if (!collected_slays)
			collected_slays = s_new;

		/* Link the allocated slays in a chain */
		if (s_last)
			s_last->next = s_new;

		/* Fill in the data */
		s_new->name = string_make(s->name);
		s_new->race_flag = s->race_flag;
		s_new->multiplier = s->multiplier;

		/* Move to the next slay */
		s = s->next;

		/* Move to the object if we're done with the given slays */
		if (!s && !moved && obj) {
			s = obj->slays;
			moved = true;
		}

		s_last = s_new;
	}
	return collected_slays;
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
	if (streq(slay->name, mon->race->base->name))
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
 * \param known_only is whether we are using all the brands and slays, or only
 * the ones we *already* know about
 */
void improve_attack_modifier(struct object *obj, const struct monster *mon,
							 const struct brand **brand_used, 
							 const struct slay **slay_used, 
							 char *verb, bool range, bool real)
{
	struct brand *b;
	struct slay *s;
	int best_mult = 1;

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
				struct monster_lore *lore = get_lore(mon->race);

				/* Learn about the brand */
				object_learn_brand(player, obj, b);

				/* Learn about the monster */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, brand_names[b->element].resist_flag);
			}
		}
	}

	/* Slays */
	for (s = obj->slays; s; s = s->next) {
		/* Is the monster is vulnerable? */
		if (react_to_specific_slay(s, mon)) {
			/* Record the best multiplier */
			if (best_mult < s->multiplier) {
				best_mult = s->multiplier;
				*brand_used = NULL;
				*slay_used = s;
				if (range) {
					if (s->multiplier <= 3)
						my_strcpy(verb, "pierces", 20);
					else
						my_strcpy(verb, "deeply pierces", 20);
				} else {
					if (s->multiplier <= 3)
						my_strcpy(verb, "smite", 20);
					else
						my_strcpy(verb, "fiercely smite", 20);
				}
			}
			/* Learn from real attacks */
			if (real) {
				struct monster_lore *lore = get_lore(mon->race);

				/* Learn about the slay */
				object_learn_slay(player, obj, s);

				/* Learn about the monster */
				if (mflag_has(mon->mflag, MFLAG_VISIBLE))
					rf_on(lore->flags, s->race_flag);
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
	struct slay *s;

	for (s = obj->slays; s; s = s->next) {
		if (react_to_specific_slay(s, mon))
			return true;
	}

	return false;
}

/**
 * Determine whether two lists of brands are the same
 *
 * \param brand1
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
 * Determine whether two lists of slays are the same
 *
 * \param slay1
 * \param slay2 the lists being compared
 */
bool slays_are_equal(struct slay *slay1, struct slay *slay2)
{
	struct slay *s1 = slay1, *s2;
	int count = 0, match = 0;

	while (s1) {
		count++;
		s2 = slay2;
		while (s2) {
			/* Count if the same */
			if (streq(s1->name, s2->name) && (s1->race_flag == s2->race_flag) &&
				(s1->multiplier == s2->multiplier))
				match++;
			s2 = s2->next;
		}

		/* Fail if we didn't find a match */
		if (match != count) return false;

		s1 = s1->next;
	}

	/* Now count back and make sure slay2 isn't strictly bigger */
	s2 = slay2;
	while (s2) {
		count--;
		s2 = s2->next;
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

/**
 * Remove a list of slays and de-allocate their memory
 */
void wipe_slays(struct slay *slays)
{
	struct slay *s = slays, *s1;
	while (s) {
		s1 = s;
		s = s->next;
		mem_free(s1);
	}
}


/**
 * Check the slay cache for a combination of slays and brands
 * 
 * \param obj is the object the combination is on
 * \return the power value of the combination
 */
s32b check_slay_cache(const struct object *obj)
{
	int i = 0;

	while ((slay_cache[i].brands != NULL) || (slay_cache[i].slays != NULL)) {
		if (brands_are_equal(obj->brands, slay_cache[i].brands) &&
			slays_are_equal(obj->slays, slay_cache[i].slays)) 
			break;
		i++;
	}

	return slay_cache[i].value;
}


/**
 * Fill in a value in the slay cache. Return true if a change is made.
 *
 * \param obj is the object the combination is on
 * \param value is the value of the slay flags on the object
 */
bool fill_slay_cache(const struct object *obj, s32b value)
{
	int i = 0;

	while ((slay_cache[i].brands != NULL) || (slay_cache[i].slays != NULL)) {
		if (brands_are_equal(obj->brands, slay_cache[i].brands) &&
			slays_are_equal(obj->slays, slay_cache[i].slays)) {
			slay_cache[i].value = value;
			return true;
		}
		i++;
	}

	return false;
}

/**
 * Create a cache of slay/brand combinations found on ego items, and the values
 * of these combinations. This is to speed up slay_power(), which will be called
 * many times for ego items during the game.
 *
 * \param items is the set of ego types from which we are extracting slay
 * combinations
 */
errr create_slay_cache(struct ego_item *items)
{
    int i;
    int j;
    int count = 0;
    struct slay_cache *dupcheck;
    struct ego_item *ego;

    /* Calculate necessary size of slay_cache */
    dupcheck = mem_zalloc(z_info->e_max * sizeof(struct slay_cache));

    for (i = 0; i < z_info->e_max; i++) {
        ego = items + i;

        /* Only consider things with brands and slays */
        if (!ego->brands && !ego->slays) continue;

		/* Check previously scanned combinations */
		for (j = 0; j < i; j++) {
			if (!dupcheck[j].brands && !dupcheck[j].slays) continue;
			if (!brands_are_equal(ego->brands, dupcheck[j].brands))
				continue;
			if (!slays_are_equal(ego->slays, dupcheck[j].slays))
				continue;

			/* Both equal, we don't want this one */
			break;
		}

		/* If we left early, we found a match */
		if (j != i) continue;

		/* msg("Found a new slay combo on an ego item"); */
		count++;
		copy_brand(&dupcheck[i].brands, ego->brands);
		copy_slay(&dupcheck[i].slays, ego->slays);
	}

    /* Allocate slay_cache with an extra empty element for an iteration stop */
    slay_cache = mem_zalloc((count + 1) * sizeof(struct slay_cache));
    count = 0;

    /* Populate the slay_cache */
    for (i = 0; i < z_info->e_max; i++) {
		if (!dupcheck[i].brands && !dupcheck[i].slays) continue;

		copy_brand(&slay_cache[count].brands, dupcheck[i].brands);
		copy_slay(&slay_cache[count].slays, dupcheck[i].slays);
		free_brand(dupcheck[i].brands);
		free_slay(dupcheck[i].slays);
		slay_cache[count].value = 0;
		count++;
		/*msg("Cached a slay combination");*/
	}

    mem_free(dupcheck);

    /* Success */
    return 0;
}

/**
 * Free the slay cache
 */
void free_slay_cache(void)
{
	int i = 0;
	while (slay_cache[i].slays || slay_cache[i].brands) {
		free_slay(slay_cache[i].slays);
		free_brand(slay_cache[i].brands);
		i++;
	}
	mem_free(slay_cache);
}
