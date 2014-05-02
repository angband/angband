/*
  \file obj-slays.h
  \brief Structures and functions for dealing with slays and brands
 *
 * Copyright (c) 2014 Chris Carr, Nick McConnell
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

/*
 * Slay cache. Used for looking up slay values in obj-power.c
 */
struct slay_cache {
	struct brand *brands;   	/* Brands */
	struct slay *slays;   	/* Slays */
	s32b value;            		/* Value of this combination */
};


/*** Functions ***/
void copy_slay(struct slay **dest, struct slay *source);
void copy_brand(struct brand **dest, struct brand *source);
bool append_random_brand(struct brand *current, char **name);
bool append_random_slay(struct slay *current, char **name);
int brand_count(struct brand *brands);
int slay_count(struct slay *slays);
struct brand *brand_collect(const object_type *obj1, const object_type *obj2,
							int *total,	bool known);
struct slay *slay_collect(const object_type *obj1, const object_type *obj2,
							  int *total, bool known);
void object_notice_brands(object_type *o_ptr, const monster_type *m_ptr);
void object_notice_slays(object_type *o_ptr, const monster_type *m_ptr);
void improve_attack_modifier(object_type *o_ptr, const monster_type	*m_ptr, 
							 const struct brand **brand_used, 
							 const struct slay **slay_used, 
							 char *verb, bool real, bool known_only);
bool react_to_slay(struct object *obj, const struct monster *mon);
void wipe_brands(struct brand *brands);
void wipe_slays(struct slay *slays);
errr create_slay_cache(struct ego_item *items);
s32b check_slay_cache(const object_type *obj);
bool fill_slay_cache(const object_type *obj, s32b value);
void free_slay_cache(void);

#endif /* INCLUDED_SLAYS_H */
