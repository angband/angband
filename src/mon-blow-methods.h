/**
 * \file mon-blow-methods.h
 * \brief Functions for managing monster melee methods.
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

#ifndef MON_BLOW_METHODS_H
#define MON_BLOW_METHODS_H

/**
 * List of blow method constants
 */
enum monster_blow_method_e {
	#define RBM(x, c, s, miss, p, m, a, d) RBM_##x,
	#include "list-blow-methods.h"
	#undef RBM
};

/**
 * Storage class for monster_blow_method_e.
 */
typedef byte monster_blow_method_t;

/* Functions */
extern bool monster_blow_method_cut(monster_blow_method_t method);
extern bool monster_blow_method_stun(monster_blow_method_t method);
extern bool monster_blow_method_miss(monster_blow_method_t method);
extern bool monster_blow_method_physical(monster_blow_method_t method);
extern bool monster_blow_method_is_valid(monster_blow_method_t method);
extern int monster_blow_method_message(monster_blow_method_t method);
extern const char *monster_blow_method_action(monster_blow_method_t method);
extern const char *monster_blow_method_description(monster_blow_method_t method);
extern monster_blow_method_t blow_method_name_to_idx(const char *string);

#endif /* MON_BLOW_METHODS_H */
