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
} effect_index;

struct effect {
	struct effect *next;
	byte index;		/**< The effect index */
	dice_t *dice;	/**< Dice expression used in the effect */
	int params[2];	/**< Extra parameters to be passed to the handler */
};

/*** Functions ***/

bool effect_do(effect_index effect, bool *ident, bool aware, int dir, int beam,
	int boost);
bool effect_aim(effect_index effect);
const char *effect_desc(effect_index effect);
int effect_power(effect_index effect);
bool effect_obvious(effect_index effect);
bool effect_wonder(int dir, int die, int beam);
bool effect_valid(effect_index effect);
effect_index effect_lookup(const char *name);

#endif /* INCLUDED_EFFECTS_H */
