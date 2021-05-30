/**
 * \file obj-slays.h
 * \brief Structures and functions for dealing with slays and brands
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

extern struct slay *slays;
extern struct brand *brands;

/*** Functions ***/
bool same_monsters_slain(int slay1, int slay2);
void copy_slays(bool **dest, bool *source);
void copy_brands(bool **dest, bool *source);
bool append_random_brand(bool **current, struct brand **brand);
bool append_random_slay(bool **current, struct slay **slay);
int brand_count(const bool *brands_on);
int slay_count(const bool *slays_on);
bool player_has_temporary_brand(int idx);
bool player_has_temporary_slay(int idx);
int get_monster_brand_multiplier(const struct monster *mon, const struct brand *b);
void improve_attack_modifier(struct object *obj, const struct monster *mon, 
							 int *brand_used, int *slay_used, char *verb,
							 bool range);
bool react_to_slay(struct object *obj, const struct monster *mon);

#endif /* OBJECT_SLAYS_H */
