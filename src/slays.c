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
#include "cave.h"
#include "effects.h"
#include "monster/monster.h"
#include "spells.h"
#include "slays.h"


/*
 * Info about slays (see src/slays.h for structure)
 */
const slays slay_table[] =
{
        #define SLAY(a, b, c, d, e, f, g, h, i, j) \
		 { SL_##a, b, c, d, e, f, g, h, i, j},
        #include "list-slays.h"
        #undef SLAY
};


/*
 * Slays which are in some sense duplicates. *Slay* dragon supercedes slay
 * dragon, for example.
 */
const struct {
        u16b minor;
        u16b major;
} slay_dups[] =
{
        { OF_SLAY_DRAGON, OF_KILL_DRAGON },
        { OF_SLAY_DEMON, OF_KILL_DEMON },
        { OF_SLAY_UNDEAD, OF_KILL_UNDEAD },
};


/**
 * Count the number of slays and brands in flags.
 */
int count_slays(bitflag *flags);
{
	int i = 0;
	const slays *s_ptr;

	for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++) {
		if (of_has(flags, s_ptr->slay_flag))
			i++;
	}

	return i;
}


/**
 * Count the number of brands in flags.
 */
int count_brands(bitflag *flags);
{
	int i = 0;
	const slays *s_ptr;

	for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++) {
		if (of_has(flags, s_ptr->slay_flag) && s_ptr->brand)
			i++;
	}

	return i;
}


/**
 * Get a random slay (or brand) by index.
 * We use randint1 because the first entry in slay_table is null.
 */
slays random_slay(bool brand);
{
	const slays *s_ptr;

	while ((brand && !s_ptr->brand) || (!brand && s_ptr->brand) ||
			!s_ptr->index) {
		s_ptr = &slay_table[randint1(SL_MAX - 1)];
	}

	return s_ptr;
}

/**
 * Collect a set of descriptions and multipliers of slays in flags
 *
 * cnt is the number of matches
 *
 * list[] and mult[] must be >= SL_MAX in size
 */
int collect_slays(const char *desc[], int mult[], bitflag *flags)
{
        int cnt = 0;
        u16b i;
        const slays *s_ptr;

        /* Remove "duplicate" flags e.g. *slay* and slay the same
         * monster type
         */
        for (i = 0; i < N_ELEMENTS(slay_dups); i++) {
                if (of_has(flags, slay_dups[i].minor) &&
                                of_has(flags, slay_dups[i].major)) {
                        of_off(flags, slay_dups[i].minor);
                }
        }

        /* Collect slays */
        for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++) {
                if (of_has(flags, s_ptr->slay_flag)) {
                        mult[cnt] = s_ptr->mult;
                        desc[cnt++] = s_ptr->desc;
                }
        }

        return cnt;
}


/**
 * Match slays in flags against a chosen flag mask
 *
 * count is the number of matches
 * \param list_p is the array of descriptions of matching slays
 */
int slay_descriptions(const bitflag flags[OF_SIZE], const bitflag mask[OF_SIZE],
	const char ***list_p)
{
	const char *slay_descs[SL_MAX - 1];
	const slays *s_ptr;
	int count = 0;

	flags_init(slay_mask, OF_SIZE, mask, FLAG_END);

	for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++)
	{
		if (!of_has(flags, s_ptr->slay_flag)) continue;

		if (of_has(mask, s_ptr->slay_flag))
			slay_descs[count++] = s_ptr->desc;
	}
	list_p = slay_descs;
	return count;
}


/**
 * Notice a particular slay on a particular object.
 */
void object_notice_slay(object_type *o_ptr, int flag)                                                              
{
        const slays *s_ptr;
        bool learned = object_notice_flag(o_ptr, flag);

        /* if you learn a slay, learn the ego and print a message */
        if (EASY_LEARN && learned)
        {
                object_notice_ego(o_ptr);

                for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++)
                {
                        if (s_ptr->slay_flag == flag)
                        {
                                char o_name[40];
                                object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
                                msg("Your %s %s!", o_name, s_ptr->active_verb);
                        }
                }
        }

        object_check_for_ident(o_ptr);
}


/**
 * Notice any brands on an object. If any are learned, learn the ego
 * and print a message.
 */
void object_notice_brands(object_type *o_ptr)
{
        const slays *s_ptr;
        bool learned
	bitflag f[OF_SIZE];

	object_flags(o_ptr, f);

	for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++) {
		if (of_has(f, s_ptr->slay_flag) && s_ptr->brand) {
			learned = object_notice_flag(o_ptr, s_ptr->slay_flag);
			if (EASY_LEARN && learned) {
				object_notice_ego(o_ptr);
                                char o_name[40];
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
 */
void improve_attack_modifier(object_type *o_ptr, const monster_type *m_ptr,
	const slays **best_s_ptr)
{
        const slays *s_ptr;

        monster_race *r_ptr = &r_info[m_ptr->r_idx];
        monster_lore *l_ptr = &l_list[m_ptr->r_idx];

        bitflag f[OF_SIZE], known_f[OF_SIZE];
        object_flags(o_ptr, f);
        object_flags_known(o_ptr, known_f);

        for (s_ptr = slay_table; s_ptr->index < SL_MAX; s_ptr++)
        {
                if (!of_has(f, s_ptr->slay_flag)) continue;

                /* Learn about monster resistance/vulnerability IF:
                 * 1) The slay flag on the object is known
                 * 2) The monster does not possess the appropriate resistance
                 * 3) The monster does possess the appropriate vulnerability
                 */
                if (of_has(known_f, s_ptr->slay_flag) ||
	              		(s_ptr->monster_flag && rf_has(r_ptr->flags,
				s_ptr->monster_flag)) ||
               			(s_ptr->resist_flag && !rf_has(r_ptr->flags,
				s_ptr->resist_flag))) {

                        if (m_ptr->ml && s_ptr->monster_flag)
                                rf_on(l_ptr->flags, s_ptr->monster_flag);

                        if (m_ptr->ml && s_ptr->resist_flag)
                                rf_on(l_ptr->flags, s_ptr->resist_flag);
                }

                /* If the monster doesn't resist or the slay flag matches */
                if ((s_ptr->brand && !rf_has(r_ptr->flags, s_ptr->resist_flag))
                		|| rf_has(r_ptr->flags, s_ptr->monster_flag))
                {
                        /* notice any brand or slay that would affect monster */
                        object_notice_slay(o_ptr, s_ptr->slay_flag);

                        /* compare multipliers to determine best attack */
                        if ((*best_s_ptr == NULL) || 
					((*best_s_ptr)->mult < s_ptr->mult))
                                *best_s_ptr = s_ptr;
                }
        }
}


