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
#include "object/slays.h"


/**
 * Info about slays (see src/slays.h for structure)
 */
const struct slay slay_table[] =
{
	#define SLAY(a, b, c, d, e, f, g, h, i, j) \
		{ SL_##a, b, c, d, e, f, g, h, i, j},
	#include "list-slays.h"
	#undef SLAY
};

/**
 * Cache of slay values (for object_power)
 */
static struct flag_cache *slay_cache;


/**
 * Remove slays which are duplicates, i.e. they have exactly the same "monster
 * flag" and the same "resist flag". The one with highest multiplier is kept. 
 * 
 * \param flags is the flagset from which to remove duplicates.
 * count is the number of dups removed.
 */
int dedup_slays(bitflag *flags) {
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
 * Match slays in flags against a chosen flag mask
 *
 * count is the number of matches
 * \param flags is the flagset to analyse for matches
 * \param mask is the flagset against which to test
 * \param desc[] is the array of descriptions of matching slays - can be null
 * \param brand[] is the array of descriptions of brands - can be null
 * \param mult[] is the array of multipliers of those slays - can be null
 * \param dedup is whether or not to remove duplicates
 *
 * desc[], brand[] and mult[] must be >= SL_MAX in size
 */
int list_slays(const bitflag flags[OF_SIZE], const bitflag mask[OF_SIZE],
	const char *desc[], const char *brand[], int mult[], bool dedup)
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
 * Notice any slays on a particular object which are in mask.
 *
 * \param o_ptr is the object on which we are noticing slays
 * \param mask is the flagset within which we are noticing them 
 */
void object_notice_slays(object_type *o_ptr, const bitflag mask[OF_SIZE])
{
	bool learned;
	bitflag f[OF_SIZE];
	char o_name[40];
	int i;

	/* We are only interested in the flags specified in mask */
	object_flags(o_ptr, f);
	of_inter(f, mask);

	/* if you learn a slay, learn the ego and print a message */
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(f, s_ptr->object_flag)) {
			learned = object_notice_flag(o_ptr, s_ptr->object_flag);
			if (EASY_LEARN && learned) {
				object_notice_ego(o_ptr);
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
				msg("Your %s %s!", o_name, s_ptr->active_verb);
			}
		}
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
 * \param lore is whether this is a real attack (where we update lore) or a
 *  simulation (where we don't)
 * \param known_only is whether we are using all the object flags, or only
 * the ones we *already* know about
 */
void improve_attack_modifier(object_type *o_ptr, const monster_type
	*m_ptr, const struct slay **best_s_ptr, bool real, bool known_only)
{
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];
	bitflag f[OF_SIZE], known_f[OF_SIZE], note_f[OF_SIZE];
	int i;

	object_flags(o_ptr, f);
	object_flags_known(o_ptr, known_f);

	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if ((known_only && !of_has(known_f, s_ptr->object_flag)) ||
				(!known_only && !of_has(f, s_ptr->object_flag))) continue;

		/* In a real attack, learn about monster resistance or slay match if:
		 * EITHER the slay flag on the object is known,
		 * OR the monster is vulnerable to the slay/brand
		 */
		if (real && (of_has(known_f, s_ptr->object_flag) || (s_ptr->monster_flag
				&& rf_has(r_ptr->flags,	s_ptr->monster_flag)) ||
				(s_ptr->resist_flag && !rf_has(r_ptr->flags,
				s_ptr->resist_flag)))) {

			/* notice any brand or slay that would affect monster */
			of_wipe(note_f);
			of_on(note_f, s_ptr->object_flag);
			object_notice_slays(o_ptr, note_f);

			if (m_ptr->ml && s_ptr->monster_flag)
				rf_on(l_ptr->flags, s_ptr->monster_flag);

			if (m_ptr->ml && s_ptr->resist_flag)
				rf_on(l_ptr->flags, s_ptr->resist_flag);
		}

		/* If the monster doesn't resist or the slay flag matches */
		if ((s_ptr->brand && !rf_has(r_ptr->flags, s_ptr->resist_flag)) ||
				(s_ptr->monster_flag && rf_has(r_ptr->flags,
				s_ptr->monster_flag))) {

			/* compare multipliers to determine best attack */
			if ((*best_s_ptr == NULL) || ((*best_s_ptr)->mult < s_ptr->mult))
				*best_s_ptr = s_ptr;
		}
	}
}


/**
 * React to slays which hurt a monster
 * 
 * \param obj_flags is the set of flags we're testing for slays
 * \param mon_flags is the set of flags we're adjusting as a result
 */
void react_to_slay(bitflag *obj_flags, bitflag *mon_flags)
{
	int i;
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(obj_flags, s_ptr->object_flag) && s_ptr->monster_flag)
			rf_on(mon_flags, s_ptr->monster_flag);
	}
}


/**
 * Check the slay cache for a combination of slays and return a slay value
 * 
 * \param index is the set of slay flags to look for
 */
s32b check_slay_cache(bitflag *index)
{
	int i;

	for (i = 0; !of_is_empty(slay_cache[i].flags); i++)
		if (of_is_equal(index, slay_cache[i].flags)) break;

	return slay_cache[i].value;
}


/**
 * Fill in a value in the slay cache. Return TRUE if a change is made.
 *
 * \param index is the set of slay flags whose value we are adding
 * \param value is the value of the slay flags in index
 */
bool fill_slay_cache(bitflag *index, s32b value)
{
	int i;

	for (i = 0; !of_is_empty(slay_cache[i].flags); i++) {
		if (of_is_equal(index, slay_cache[i].flags)) {
			slay_cache[i].value = value;
			return TRUE;
		}
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
    bitflag cacheme[OF_SIZE];
    bitflag slay_mask[OF_SIZE];
    bitflag **dupcheck;
    ego_item_type *e_ptr;

    /* Build the slay mask */
	create_mask(slay_mask, FALSE, OFT_SLAY, OFT_KILL, OFT_BRAND, OFT_MAX);

    /* Calculate necessary size of slay_cache */
    dupcheck = C_ZNEW(z_info->e_max, bitflag *);

    for (i = 0; i < z_info->e_max; i++) {
        dupcheck[i] = C_ZNEW(OF_SIZE, bitflag);
        e_ptr = items + i;

        /* Find the slay flags on this ego */
        of_copy(cacheme, e_ptr->flags);
        of_inter(cacheme, slay_mask);

        /* Only consider non-empty combinations of slay flags */
        if (!of_is_empty(cacheme)) {
            /* Skip previously scanned combinations */
            for (j = 0; j < i; j++)
                if (of_is_equal(cacheme, dupcheck[j])) continue;

            /* msg("Found a new slay combo on an ego item"); */
            count++;
            of_copy(dupcheck[i], cacheme);
        }
    }

    /* Allocate slay_cache with an extra empty element for an iteration stop */
    slay_cache = C_ZNEW((count + 1), struct flag_cache);
    count = 0;

    /* Populate the slay_cache */
    for (i = 0; i < z_info->e_max; i++) {
        if (!of_is_empty(dupcheck[i])) {
            of_copy(slay_cache[count].flags, dupcheck[i]);
            slay_cache[count].value = 0;
            count++;
            /*msg("Cached a slay combination");*/
        }
    }

    for (i = 0; i < z_info->e_max; i++)
        FREE(dupcheck[i]);
    FREE(dupcheck);

    /* Success */
    return 0;
}

void free_slay_cache(void)
{
	mem_free(slay_cache);
}
