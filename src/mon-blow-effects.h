/**
 * \file mon-blow-effects.h
 * \brief Functions for managing monster melee effects.
 *
 * Copyright (c) 1997 Ben Harrison, David Reeve Sward, Keldon Jones.
 *               2013 Ben Semmler
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

#ifndef MON_BLOW_EFFECTS_H
#define MON_BLOW_EFFECTS_H

#include "player.h"
#include "mon-blow-methods.h"
#include "monster.h"

/**
 * List of monster blow effects
 */
enum monster_blow_effect_e {
	#define RBE(x, p, e, d) RBE_##x,
	#include "list-blow-effects.h"
	#undef RBE
};

/**
 * Storage for context information for effect handlers called in
 * make_attack_normal().
 *
 * The members of this struct are initialized in an order-dependent way
 * (to be more cross-platform). If the members change, make sure to change
 * any initializers. Ideally, this should eventually used named initializers.
 */
typedef struct melee_effect_handler_context_s {
	struct player * const p;
	struct monster * const mon;
	const int rlev;
	const monster_blow_method_t method;
	const int ac;
	const char *ddesc;
	bool obvious;
	bool blinked;
	bool do_break;
	int damage;
} melee_effect_handler_context_t;

/**
 * Melee blow effect handler.
 */
typedef void (*melee_effect_handler_f)(melee_effect_handler_context_t *);

/**
 * Storage class for monster_blow_effect_e.
 */
typedef byte monster_blow_effect_t;

/* Functions */
extern int monster_blow_effect_power(monster_blow_effect_t effect);
extern int monster_blow_effect_eval(monster_blow_effect_t effect);
extern bool monster_blow_effect_is_valid(monster_blow_effect_t effect);
extern const char *monster_blow_effect_description(monster_blow_effect_t effect);
extern melee_effect_handler_f melee_handler_for_blow_effect(monster_blow_effect_t effect);
extern monster_blow_effect_t blow_effect_name_to_idx(const char *string);

#endif /* MON_BLOW_EFFECTS_H */
