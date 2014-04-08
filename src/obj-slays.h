/*
 * File: obj-slays.h
 * Purpose: Structures and functions for dealing with slays and brands
 *
 * Copyright (c) 2010 Chris Carr
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

#include "monster.h"

/* Types of slay (including brands) */
enum
{
	#define SLAY(a, b, c, d, e, f, g, h, i, j)    SL_##a,
	#include "list-slays.h"
	#undef SLAY

	SL_MAX
};


/*
 * Slay type.  Used for the table of brands/slays and their effects.
 */
struct slay {
	u16b index;					/* Numerical index */
	int object_flag;			/* Object flag for the slay */
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
struct flag_cache {
	bitflag flags[OF_SIZE];   	/* Combination of slays and brands */
	s32b value;            		/* Value of this combination */
};


/*** Functions ***/
void add_slay(struct new_slay **dest, struct new_slay *source);
void add_brand(struct brand **dest, struct brand *source);
const struct slay *random_slay(const bitflag mask[OF_SIZE]);
int list_slays(const bitflag flags[OF_SIZE], const bitflag mask[OF_SIZE],
			   int slay_list[], bool dedup);
int slay_info_collect(const int slays[], const char *desc[], 
					  const char *brand[], int mult[], int max_n);
void object_notice_brands(object_type *o_ptr, const monster_type *m_ptr);
void object_notice_slays(object_type *o_ptr, const monster_type *m_ptr);
void improve_attack_modifier(object_type *o_ptr, const monster_type	*m_ptr, 
							 const struct brand **brand_used, 
							 const struct new_slay **slay_used, 
							 char **verb, bool real, bool known_only);
bool react_to_slay(struct object *obj, const struct monster *mon);
errr create_slay_cache(struct ego_item *items);
s32b check_slay_cache(bitflag *index);
bool fill_slay_cache(bitflag *index, s32b value);
void free_slay_cache(void);

#endif /* INCLUDED_SLAYS_H */
