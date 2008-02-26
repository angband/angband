/*
 * File: effects.h
 * Purpose: List of effect types
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
#ifndef INCLUDED_EFFECTS_H
#define INCLUDED_EFFECTS_H

bool do_effect(int effect, bool *ident, int dir, int beam);
bool effect_aim(int effect);
const char *effect_desc(int effect);

/* Types of effect */
typedef enum
{
	#define EFFECT(x, y, z)		EF_##x,
	#include "list-effects.h"
	#undef EFFECT

	EF_MAX
} effect_type;

#endif /* INCLUDED_EFFECTS_H */
