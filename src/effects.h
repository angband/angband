/*
 * File: effects.h
 * Purpose: List of effect types
 *
 * Copyright (c) 2007 Andi Sidwell
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
#ifndef INCLUDED_EFFECTS_H
#define INCLUDED_EFFECTS_H

/* Types of effect */
typedef enum
{
	#define EFFECT(x, a, r, h, v, c, d)	EF_##x,
	#include "list-effects.h"
	#undef EFFECT
} effect_type;

/*** Functions ***/

bool effect_do(effect_type effect, bool *ident, bool aware, int dir, int beam,
	int boost);
bool effect_aim(effect_type effect);
const char *effect_desc(effect_type effect);
int effect_power(effect_type effect);
bool effect_obvious(effect_type effect);
bool effect_wonder(int dir, int die, int beam);
bool effect_valid(effect_type effect);
effect_type effect_lookup(const char *name);

#endif /* INCLUDED_EFFECTS_H */
