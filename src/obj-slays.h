/**
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
#ifndef OBJECT_SLAYS_H
#define OBJECT_SLAYS_H

#include "monster.h"

/**
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
void free_slay(struct slay *source);
void free_brand(struct brand *source);
bool append_random_brand(struct brand **current, char **name);
bool append_random_slay(struct slay **current, char **name);
int brand_count(struct brand *brands);
int slay_count(struct slay *slays);
struct brand *brand_collect(struct brand *b, const struct object *obj2);
struct slay *slay_collect(struct slay *s, const struct object *obj2);
void improve_attack_modifier(struct object *obj, const struct monster *mon, 
							 const struct brand **brand_used, 
							 const struct slay **slay_used, 
							 char *verb, bool range, bool real);
bool react_to_slay(struct object *obj, const struct monster *mon);
bool brands_are_equal(struct brand *brand1, struct brand *brand2);
bool slays_are_equal(struct slay *slay1, struct slay *slay2);
void wipe_brands(struct brand *brands);
void wipe_slays(struct slay *slays);
errr create_slay_cache(struct ego_item *items);
s32b check_slay_cache(const struct object *obj);
bool fill_slay_cache(const struct object *obj, s32b value);
void free_slay_cache(void);

#endif /* OBJECT_SLAYS_H */
