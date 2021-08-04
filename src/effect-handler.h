/**
 * \file effect-handler.h
 * \brief Internal header for effect handler functions
 *
 * Copyright (c) 2007 Andi Sidwell
 * Copyright (c) 2016 Ben Semmler, Nick McConnell
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

#ifndef INCLUDED_EFFECT_HANDLER_H
#define INCLUDED_EFFECT_HANDLER_H

#include "effects.h"

/**
 * Bit flags for the enchant() function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOBOTH  0x03
#define ENCH_TOAC	0x04

typedef struct effect_handler_context_s {
	const effect_index effect;
	const struct source origin;
	const struct object *obj;
	const bool aware;
	const int dir;
	const int beam;
	const int boost;
	const random_value value;
	const int subtype, radius, other, y, x;
	const char *msg;
	bool ident;
	struct command *cmd;
} effect_handler_context_t;

typedef bool (*effect_handler_f)(effect_handler_context_t *);

/**
 * Structure for effects
 */
struct effect_kind {
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	const char *info;    /* Effect info (for spell tips) */
	effect_handler_f handler;    /* Function to perform the effect */
	const char *desc;    /* Effect description */
	const char *menu_name;       /* Format string for short name */
};

/* Prototype every effect handler */
#define EFFECT(x, a, b, c, d, e, f)	bool effect_handler_##x(effect_handler_context_t *context);
#include "list-effects.h"
#undef EFFECT

int effect_calculate_value(effect_handler_context_t *context, bool use_boost);
struct monster *monster_target_monster(effect_handler_context_t *context);

#endif /* INCLUDED_EFFECT_HANDLER_H */
