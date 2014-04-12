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
 * Info about slays (see src/slays.h for structure)
 */
static const struct slay slay_table[] =
{
	#define SLAY(a, b, c, d, e, f, g, h, i, j)	\
		{ SL_##a, b, c, d, e, f, g, h, i, j},
	#include "list-slays.h"
	#undef SLAY
};

/**
 * Cache of slay values (for object_power)
 */
static struct slay_cache *slay_cache;

struct brand_info {
	const char *active_verb;
	const char *melee_verb;
	const char *melee_verb_weak;
	int resist_flag;
};

/**
 * Brand info - until there's a better place NRM
 */
const struct brand_info brand_names[] = {
	{ "spits", "dissolve", "corrode", RF_IM_ACID },
	{ "crackles", "shock", "zap", RF_IM_ELEC },
	{ "flares", "burn", "singe", RF_IM_FIRE },
	{ "grows cold", "freeze", "chill", RF_IM_COLD },
	{ "seethes", "poison", "sicken", RF_IM_POIS }
};

/**
 * Copy all the slays from one structure to another
 */
void copy_slay(struct new_slay **dest, struct new_slay *source)
{
	struct new_slay *s = source;

	while (s) {
		struct new_slay *os = mem_zalloc(sizeof *os);
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

/**
 * Remove slays which are duplicates, i.e. they have exactly the same "monster
 * flag" and the same "resist flag". The one with highest multiplier is kept.
 *
 * \param flags is the flagset from which to remove duplicates.
 * count is the number of dups removed.
 */
static int dedup_slays(bitflag *flags) {
	int i, j;
	int count = 0;

	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(flags, s_ptr->object_flag)) {
			for (j = i + 1; j < SL_MAX; j++) {
				const struct slay *t_ptr = &slay_table[j];
				if (of_has(flags, t_ptr->object_flag) &&
						(t_ptr->monster_flag == s_ptr->monster_flag) &&
						(t_ptr->resist_flag == s_ptr->resist_flag) &&
						(t_ptr->mult != s_ptr->mult)) {
					count++;
					if (t_ptr->mult > s_ptr->mult)
						of_off(flags, s_ptr->object_flag);
					else
						of_off(flags, t_ptr->object_flag);
				}
			}
		}
	}

	return count;
}


/**
 * Get a random slay (or brand).
 * We use randint1 because the first entry in slay_table is null.
 *
 * \param mask is the set of slays from which we are choosing.
 */
const struct slay *random_slay(const bitflag mask[OF_SIZE])
{
	const struct slay *s_ptr;
	do {
		s_ptr = &slay_table[randint1(SL_MAX - 1)];
	} while (!of_has(mask, s_ptr->object_flag));

	return s_ptr;
}


/**
 * Match slays in flags against a chosen flag mask.
 *
 * count is the number of matches
 * \param flags is the flagset to analyse for matches
 * \param mask is the flagset against which to test
 * \param slays is the array of slays found in the supplied flags - can be null
 * \param dedup is whether or not to remove duplicates
 *
 * slays[] must be >= SL_MAX in size
 */
int list_slays(const bitflag flags[OF_SIZE], const bitflag mask[OF_SIZE],
			   int slays[], bool dedup)
{
	int i, count = 0;
	bitflag f[OF_SIZE];

	/* We are only interested in the flags specified in mask */
	of_copy(f, flags);
	of_inter(f, mask);

	/* Remove "duplicate" flags if desired */
	if (dedup) dedup_slays(f);

	/* Collect slays */
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(f, s_ptr->object_flag)) {
			if (slays)
				slays[count] = i;

			count++;
		}
	}

	return count;
}

/**
 * Fills in information about the given a list of `slays` such as returned by 
 * list_slays().
 * 
 * \param slays is the array of slays to look up info for
 * \param desc is the array of descriptions of matching slays - can be null
 * \param brand is the array of descriptions of brands - can be null
 * \param mult is the array of multipliers of those slays - can be null
 *
 * slays[], desc[], brand[] and mult[] must be >= SL_MAX in size
 */
int slay_info_collect(const int slays[], const char *desc[], 
					  const char *brand[], int mult[], int max_n)
{
	int i, count = 0;

	for (i = 0; i < max_n; i++) {
		if (slays[i]) {
			const struct slay *s_ptr = &slay_table[slays[i]];

			if (mult)
				mult[count] = s_ptr->mult;
			if (brand)
				brand[count] = s_ptr->brand;
			if (desc)
				desc[count] = s_ptr->desc;
			count++;
		}
	}

	return count;
}

/**
 * Collect the (optionally known) brands from one or two objects into a
 * linked array
 * \param obj1 the first object (not NULL)
 * \param obj2 the second object (can be NULL)
 * \known whether we are after only known brands
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
 * \known whether we are after only known slays
 * \return a pointer to the first slay
 */
struct new_slay *slay_collect(const object_type *obj1, const object_type *obj2,
							  int *total, bool known)
{
	int i, count = 0;
	struct new_slay *s, *s_new = NULL;

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
bool react_to_specific_slay(struct new_slay *slay, const struct monster *mon)
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
	struct new_slay *s;

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
							 const struct new_slay **slay_used, 
							 char **verb, bool real, bool known_only)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);
	struct brand *b;
	struct new_slay *s;
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
	struct new_slay *s;

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

bool slays_are_equal(struct new_slay *slay1, struct new_slay *slay2)
{
	struct new_slay *s1, *s2;
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
