/**
 * \file obj-randart.h
 * \brief Random artifact generation
 *
 * Copyright (c) 1998 Greg Wooledge, Ben Harrison, Robert Ruhlmann
 * Copyright (c) 2001 Chris Carr, Chris Robertson
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
 *
 * Original random artifact generator (randart) by Greg Wooledge.
 * Updated by Chris Carr / Chris Robertson 2001-2010.
 */

#ifndef OBJECT_RANDART_H
#define OBJECT_RANDART_H

#include "object.h"

#define MAX_TRIES 200
#define BUFLEN 1024

#define MIN_NAME_LEN 5
#define MAX_NAME_LEN 9

/**
 * Inhibiting factors for large bonus values
 * "HIGH" values use INHIBIT_WEAK
 * "VERYHIGH" values use INHIBIT_STRONG
 */
#define INHIBIT_STRONG  (one_in_(6))
#define INHIBIT_WEAK    (one_in_(2))

/**
 * Power rating below which uncursed randarts cannot aggravate
 * (so that aggravate is found only on endgame-quality items or
 * cursed items)
 */
#define AGGR_POWER 300

/**
 * Numerical index values for the different learned probabilities
 * These are to make the code more readable.
 */
enum {
	#define ART_IDX(a, b) ART_IDX_##a,
	#include "list-randart-properties.h"
	#undef ART_IDX
};

struct artifact_data {
	/* Mean start and increment values for to_hit, to_dam and AC */
	int hit_increment;
	int dam_increment;
	int hit_startval;
	int dam_startval;
	int ac_startval;
	int ac_increment;

	/* Data structures for learned probabilities */
	int *art_probs;
	int *base_probs;
	int bow_total;
	int melee_total;
	int boot_total;
	int glove_total;
	int headgear_total;
	int shield_total;
	int cloak_total;
	int armor_total;
	int other_total;
	int total;

	/* Arrays for holding frequency values */
	int art_freq[ART_IDX_TOTAL];  	/* artifact attributes */
	int *base_freq; 			/* base items */

	/* Artifact power ratings */
	int *base_power;
	int max_power;
	int min_power;
	int avg_power;
	int var_power;

	/* Base item levels */
	int *base_item_level;

	/* Base item rarities */
	int *base_item_prob;

	/* Artifact rarities */
	int *base_art_alloc;
};


char *artifact_gen_name(struct artifact *a, const char ***wordlist);
void do_randart(u32b randart_seed);

#endif /* OBJECT_RANDART_H */
