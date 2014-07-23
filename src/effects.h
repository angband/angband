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
	AEF_ATOMIC_NONE,
	#define EFFECT(x, a, d)	AEF_##x,
	#include "list-atomic-effects.h"
	#undef EFFECT
	AEF_ATOMIC_MAX
} atomic_effect_index;

struct effect {
	struct effect *next;
	u16b index;		/**< The effect index */
	dice_t *dice;	/**< Dice expression used in the effect */
	int params[2];	/**< Extra parameters to be passed to the handler */
};

/*** Functions ***/

void free_effect(struct effect *source);
bool atomic_effect_do(struct effect *effect, bool *ident, bool aware, int dir, int beam, int boost);
bool effect_aim(struct effect *effect);
const char *effect_desc(struct effect *effect);
bool effect_wonder(int dir, int die, int beam);
bool effect_valid(struct effect *effect);

#endif /* INCLUDED_EFFECTS_H */
