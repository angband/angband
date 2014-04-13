/*
 * File: slays.c
 * Purpose: encapsulation of slay_table and accessor functions for slays/brands
 *
 * Copyright (c) 2010 Chris Carr and Peter Denison
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
#include "obj-desc.h"
#include "obj-identify.h"
#include "obj-slays.h"
#include "obj-util.h"
#include "mon-util.h"


/**
 * Cache of slay values (for object_power)
 */
static struct slay_cache *slay_cache;

struct brand_info {
	const char* name;
	const char *active_verb;
	const char *melee_verb;
	const char *melee_verb_weak;
	int resist_flag;
};

/**
 * Brand info - until there's a better place NRM
 */
const struct brand_info brand_names[] = {
	{ "acid", "spits", "dissolve", "corrode", RF_IM_ACID },
	{ "lightning", "crackles", "shock", "zap", RF_IM_ELEC },
	{ "fire", "flares", "burn", "singe", RF_IM_FIRE },
	{ "cold", "grows cold", "freeze", "chill", RF_IM_COLD },
	{ "poison", "seethes", "poison", "sicken", RF_IM_POIS }
};

struct slay_info {
	const char *name;
	int race_flag;
	int multiplier;
};

/**
 * Slay info - until there's a better place NRM
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
 */
void copy_slay(struct slay **dest, struct slay *source)
{
	struct slay *s = source;

	while (s) {
		struct slay *os = mem_zalloc(sizeof *os);
		os->name = string_make(s->name);
		os->race_flag = s->race_flag;
		os->multiplier = s->multiplier;
		os->next = *dest;
		*dest = os;
		s = s->next;
	}
}

/**
 * Copy all the brands from one structure to another
 */
void copy_brand(struct brand **dest, struct brand *source)
{
	struct brand *b = source;

	while (b) {
		struct brand *ob = mem_zalloc(sizeof *ob);
		ob->name = string_make(b->name);
		ob->element = b->element;
		ob->multiplier = b->multiplier;
		ob->next = *dest;
		*dest = ob;
		b = b->next;
	}
}

bool append_random_brand(struct brand *current, char **name)
{
	int pick;
	struct brand *b, *b_last = NULL;

	pick = randint0(N_ELEMENTS(brand_names));
	for (b = current; b; b = b->next) {
		/* If we get the same one, fail */
		if (streq(b->name, brand_names[pick].name) &&
			(b->element == pick) && 
			(b->multiplier == 3))
			return FALSE;

		/* Remember the last one */
		b_last = b;
	}

	/* We can add the new one now */
	b = mem_zalloc(sizeof(*b));
	b->name = string_make(brand_names[pick].name);
	b->element = pick;
	b->multiplier = 3;
	if (b_last) b_last->next = b;
	*name = b->name;

	return TRUE;
}

bool append_random_slay(struct slay *current, char **name)
{
	int pick;
	struct slay *s, *s_last = NULL;

	pick = randint0(N_ELEMENTS(slay_names));
	for (s = current; s; s = s->next) {
		/* If we get the same race, check the multiplier */
		if (streq(s->name, slay_names[pick].name) &&
			(s->race_flag == slay_names[pick].race_flag)) {
			/* Same multiplier or smaller, fail */
			if (slay_names[pick].multiplier <= s->multiplier)
				return FALSE;

			/* Greater multiplier, increase and accept */
			s->multiplier = slay_names[pick].multiplier;
			return TRUE;
		}

		/* Remember the last one */
		s_last = s;
	}

	/* We can add the new one now */
	s = mem_zalloc(sizeof(*s));
	s->name = string_make(slay_names[pick].name);
	s->race_flag = slay_names[pick].race_flag;
	s->multiplier = slay_names[pick].multiplier;
	if (s_last) s_last->next = s;
	*name = s->name;

	return TRUE;
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
 * Collect the (optionally known) brands from one or two objects into a
 * linked array
 * \param obj1 the first object (not NULL)
 * \param obj2 the second object (can be NULL)
 * \param known whether we are after only known brands
 * \return a pointer to the first brand
 */
struct brand *brand_collect(const object_type *obj1, const object_type *obj2,
							int *total, bool known)
{
	int i, count = 0;
	struct brand *b, *b_new = NULL;

	/* Count the brands */
	for (b = obj1->brands; b; b = b->next)
		if (!known || b->known) count++;

	if (obj2)
		for (b = obj2->brands; b; b = b->next)
			if (!known || b->known) count++;
	*total = count;

	if (!count) return b_new;

	/* Allocate and populate */
	b_new = mem_zalloc(count * sizeof(*b_new));
	b = obj1->brands;
	for (i = 0; i < count; i++) {
		/* Set the next (for later bounds checking) */
		if (i > 0) b_new[i - 1].next = &b_new[i];

		/* Skip unknowns that should be known */
		if (known && !b->known) {
			/* Move to the next brand */
			b = b->next;

			/* Move to the second object if we're done with the first */
			if (!b && obj2) b = obj2->brands;
			continue;
		}
		/* Fill in the data */
		b_new[i].name = string_make(b->name);
		b_new[i].element = b->element;
		b_new[i].multiplier = b->multiplier;

		/* Move to the next brand */
		b = b->next;

		/* Move to the second object if we're done with the first */
		if (!b && obj2) b = obj2->brands;
	}
	return b_new;
}

/**
 * Collect the (optionally known) slays from one or two objects into a
 * linked array
 * \param obj1 the first object (not NULL)
 * \param obj2 the second object (can be NULL)
 * \param known whether we are after only known slays
 * \return a pointer to the first slay
 */
struct slay *slay_collect(const object_type *obj1, const object_type *obj2,
							  int *total, bool known)
{
	int i, count = 0;
	struct slay *s, *s_new = NULL;

	/* Count the slays */
	for (s = obj1->slays; s; s = s->next)
		if (!known || s->known) count++;

	if (obj2)
		for (s = obj2->slays; s; s = s->next)
			if (!known || s->known) count++;
	*total = count;

	if (!count) return s_new;

	/* Allocate and populate */
	s_new = mem_zalloc(count * sizeof(*s_new));
	s = obj1->slays;
	for (i = 0; i < count; i++) {
		/* Set the next (for later bounds checking) */
		if (i > 0) s_new[i - 1].next = &s_new[i];

		/* Skip unknowns that should be known */
		if (known && !s->known) {
			/* Move to the next slay */
			s = s->next;

			/* Move to the second object if we're done with the first */
			if (!s && obj2) s = obj2->slays;
			continue;
		}
		/* Fill in the data */
		s_new[i].name = string_make(s->name);
		s_new[i].race_flag = s->race_flag;
		s_new[i].multiplier = s->multiplier;

		/* Move to the next slay */
		s = s->next;

		/* Move to the second object if we're done with the first */
		if (!s && obj2) s = obj2->slays;
	}
	return s_new;
}


/**
 * React to slays which hurt a monster
 * 
 * \param slay is the slay we're testing for effectiveness
 * \param mon is the monster we're testing for being slain
 */
bool react_to_specific_slay(struct slay *slay, const struct monster *mon)
{
	if (!slay->name) return FALSE;
	if (!mon->race->base) return FALSE;

	/* Check the race flag */
	if (rf_has(mon->race->flags, slay->race_flag))
		return TRUE;

	/* Check for monster base */
	if (streq(slay->name, mon->race->base->name))
		return TRUE;

	return FALSE;
}


/**
 * Notice any brands on a particular object which affect a particular monster.
 *
 * \param o_ptr is the object on which we are noticing brands
 * \param m_ptr the monster we are hitting, if there is one
 */
void object_notice_brands(object_type *o_ptr, const monster_type *m_ptr)
{
	char o_name[40];
	struct brand *b;

	for (b = o_ptr->brands; b; b = b->next) {
		/* Already know it */
		if (b->known) continue;

		/* Not applicable */
		if (m_ptr && rf_has(m_ptr->race->flags,
						brand_names[b->element].resist_flag))
			continue;

		/* Learn */
		b->known = TRUE;
		object_notice_ego(o_ptr);
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE | ODESC_SINGULAR);
		msg("Your %s %s!", o_name, brand_names[b->element].active_verb);
	}

	object_check_for_ident(o_ptr);
}

/**
 * Notice any slays on a particular object which affect a particular monster.
 *
 * \param o_ptr is the object on which we are noticing slays
 * \param m_ptr the monster we are trying to slay
 */
void object_notice_slays(object_type *o_ptr, const monster_type *m_ptr)
{
	char o_name[40];
	struct slay *s;

	for (s = o_ptr->slays; s; s = s->next) {
		/* Already know it */
		if (s->known) continue;

		/* Not applicable */
		if (!react_to_specific_slay(s, m_ptr))
			continue;

		/* Learn */
		s->known = TRUE;
		object_notice_ego(o_ptr);
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE | ODESC_SINGULAR);
		msg("Your %s glows%s!", o_name, s->multiplier > 3 ? " brightly" : "");
	}

	object_check_for_ident(o_ptr);
}


/**
 * Extract the multiplier from a given object hitting a given monster.
 *
 * \param o_ptr is the object being used to attack
 * \param m_ptr is the monster being attacked
 * \param best_s_ptr is the best applicable slay_table entry, or NULL if no
 *  slay already known
 * \param real is whether this is a real attack (where we update lore) or a
 *  simulation (where we don't)
 * \param known_only is whether we are using all the object flags, or only
 * the ones we *already* know about
 */
void improve_attack_modifier(object_type *o_ptr, const monster_type *m_ptr,
							 const struct brand **brand_used, 
							 const struct slay **slay_used, 
							 char **verb, bool real, bool known_only)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);
	struct brand *b;
	struct slay *s;
	int best_mult = 1;

	/* Brands */
	for (b = o_ptr->brands; b; b = b->next) {
		if (known_only && !b->known) continue;

		/* If the monster is vulnerable, record and learn from real attacks */
		if (!rf_has(m_ptr->race->flags,
					brand_names[b->element].resist_flag)) {
			if (best_mult < b->multiplier) {
				best_mult = b->multiplier;
				*brand_used = b;
				if (b->multiplier < 3)
					my_strcpy(*verb, brand_names[b->element].melee_verb_weak, 
							  20);
				else
					my_strcpy(*verb, brand_names[b->element].melee_verb, 20);
			}
			if (real) {
				object_notice_brands(o_ptr, m_ptr);
				if (m_ptr->ml)
					rf_on(l_ptr->flags, brand_names[b->element].resist_flag);
			}
		}

		/* Brand is known, attack is real, learn about the monster */
		if (b->known && m_ptr->ml && real)
			rf_on(l_ptr->flags, brand_names[b->element].resist_flag);
	}

	/* Slays */
	for (s = o_ptr->slays; s; s = s->next) {
		if (known_only && !s->known) continue;

		/* If the monster is vulnerable, record and learn from real attacks */
		if (react_to_specific_slay(s, m_ptr)) {
			if (best_mult < s->multiplier) {
				best_mult = s->multiplier;
				*brand_used = NULL;
				*slay_used = s;
				if (s->multiplier <= 3)
					my_strcpy(*verb, "smite", 20);
				else
					my_strcpy(*verb, "fiercely smite", 20);
			}
			if (real) {
				object_notice_slays(o_ptr, m_ptr);
				if (m_ptr->ml)
					rf_on(l_ptr->flags, s->race_flag);
			}
		}

		/* Slay is known, attack is real, learn about the monster */
		if (s->known && m_ptr->ml && real)
			rf_on(l_ptr->flags, s->race_flag);
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
			return TRUE;
	}

	return FALSE;
}

bool brands_are_equal(struct brand *brand1, struct brand *brand2)
{
	struct brand *b1, *b2;
	int count = 0, match = 0;

	for (b1 = brand1; b1; b1 = b1->next) {
		count++;
		for (b2 = brand2; b2; b2 = b2->next) {
			/* Go to the next one if any differences */
			if (!streq(b1->name, b2->name)) continue;
			if (b1->element != b2->element) continue;
			if (b1->multiplier != b2->multiplier) continue;

			/* Count if the same */
			match++;
		}

		/* Fail if we didn't find a match */
		if (match != count) return FALSE;
	}

	/* Now count back and make sure brand2 isn't strictly bigger */
	for (b2 = brand2; b2; b2 = b2->next)
		count--;

	if (count != 0) return FALSE;

	return TRUE;
}

bool slays_are_equal(struct slay *slay1, struct slay *slay2)
{
	struct slay *s1, *s2;
	int count = 0, match = 0;

	for (s1 = slay1; s1; s1 = s1->next) {
		count++;
		for (s2 = slay2; s2; s2 = s2->next) {
			/* Go to the next one if any differences */
			if (!streq(s1->name, s2->name)) continue;
			if (s1->race_flag != s2->race_flag) continue;
			if (s1->multiplier != s2->multiplier) continue;

			/* Count if the same */
			match++;
		}

		/* Fail if we didn't find a match */
		if (match != count) return FALSE;
	}

	/* Now count back and make sure slay2 isn't strictly bigger */
	for (s2 = slay2; s2; s2 = s2->next)
		count--;

	if (count != 0) return FALSE;

	return TRUE;
}

void wipe_brands(struct brand *brands)
{
	struct brand *b = brands, *b1;
	while (b) {
		b1 = b;
		b = b->next;
		mem_free(b1);
	}
}

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
 * Check the slay cache for a combination of slays and return a slay value
 * 
 * \param index is the set of slay flags to look for
 */
s32b check_slay_cache(const object_type *obj)
{
	int i = 0;

	while ((slay_cache[i].brands != NULL) && (slay_cache[i].slays != NULL)) {
		if (brands_are_equal(obj->brands, slay_cache[i].brands) &&
			slays_are_equal(obj->slays, slay_cache[i].slays)) 
			break;
	}

	return slay_cache[i].value;
}


/**
 * Fill in a value in the slay cache. Return TRUE if a change is made.
 *
 * \param index is the set of slay flags whose value we are adding
 * \param value is the value of the slay flags in index
 */
bool fill_slay_cache(const object_type *obj, s32b value)
{
	int i = 0;

	while ((slay_cache[i].brands != NULL) && (slay_cache[i].slays != NULL)) {
		if (brands_are_equal(obj->brands, slay_cache[i].brands) &&
			slays_are_equal(obj->slays, slay_cache[i].slays)) {
			slay_cache[i].value = value;
			return TRUE;
		}
		i++;
	}

	return FALSE;
}

/**
 * Create a cache of slay combinations found on ego items, and the values of
 * these combinations. This is to speed up slay_power(), which will be called
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
    ego_item_type *e_ptr;

    /* Calculate necessary size of slay_cache */
    dupcheck = mem_zalloc(z_info->e_max * sizeof(struct slay_cache));

    for (i = 0; i < z_info->e_max; i++) {
        e_ptr = items + i;

        /* Only consider things with brands and slays */
        if (!e_ptr->brands && !e_ptr->slays) continue;

		/* Check previously scanned combinations */
		for (j = 0; j < i; j++) {
			if (!dupcheck[j].brands && !dupcheck[j].slays) continue;
			if (!brands_are_equal(e_ptr->brands, dupcheck[j].brands)) continue;
			if (!slays_are_equal(e_ptr->slays, dupcheck[j].slays)) continue;

			/* Both equal, we don't want this one */
			break;
		}

		/* If we left early, we found a match */
		if (j != i) continue;

		/* msg("Found a new slay combo on an ego item"); */
		count++;
		copy_brand(&dupcheck[i].brands, e_ptr->brands);
		copy_slay(&dupcheck[i].slays, e_ptr->slays);
	}

    /* Allocate slay_cache with an extra empty element for an iteration stop */
    slay_cache = mem_zalloc((count + 1) * sizeof(struct slay_cache));
    count = 0;

    /* Populate the slay_cache */
    for (i = 0; i < z_info->e_max; i++) {
		if (!dupcheck[i].brands && !dupcheck[i].slays) continue;

		copy_brand(&slay_cache[count].brands, dupcheck[i].brands);
		copy_slay(&slay_cache[count].slays, dupcheck[i].slays);
		slay_cache[count].value = 0;
		count++;
		/*msg("Cached a slay combination");*/
	}

    mem_free(dupcheck);

    /* Success */
    return 0;
}

void free_slay_cache(void)
{
	mem_free(slay_cache);
}
