/*
 * File: slays.h
 * Purpose: List of slay types
 *
 * Copyright (c) 2007 Andrew Sidwell
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
#ifndef INCLUDED_SLAYS_H
#define INCLUDED_SLAYS_H


/* Types of slay (including brands) */
typedef enum
{
	#define SLAY(a, b, c, d, e, f, g, h, i, j)    SL_##a,
	#include "list-slays.h"
	#undef SLAY

	SL_MAX
} slay_type;


/*
 * Slay type.  Used for the global table of brands/slays and their effects.
 */
typedef struct slay slay;

struct slay {
	u16b index;					/* Numerical index */
	int slay_flag;				/* Object flag for the slay */
	int monster_flag;			/* Which monster flag(s) make it vulnerable */
	int resist_flag;			/* Which monster flag(s) make it resist */
	int mult;					/* Slay multiplier */
	const char *range_verb;		/* attack verb for ranged hits */
	const char *melee_verb; 	/* attack verb for melee hits */
	const char *active_verb; 	/* verb for when the object is active */
	const char *desc;			/* description of vulnerable creatures */
	const char *brand;			/* name of brand */
};


/*
 * Slay cache. Used for looking up slay values in obj-power.c
 */
typedef struct flag_cache flag_cache;

struct flag_cache {
        bitflag flags[OF_SIZE];   	/* Combination of slays and brands */
        s32b value;            		/* Value of this combination */
};


/*** Functions ***/
int dedup_slays(bitflag flags[OF_SIZE]);
void random_slay(const slay *s_ptr, bool brand);
int list_slays(const bitflag flags[OF_SIZE], const bitflag mask[OF_SIZE],
	const char *desc[], int mult[], bool dedup);
void object_notice_slays(object_type *o_ptr, const bitflag mask[OF_SIZE]);
void improve_attack_modifier(object_type *o_ptr, const monster_type
	*m_ptr, const slay **best_s_ptr, bool lore, const bitflag flags[OF_SIZE]);

#endif /* INCLUDED_SLAYS_H */
