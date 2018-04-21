/**
 * \file player-timed.h
 * \brief Timed effects handling
 *
 * Copyright (c) 1997 Ben Harrison
 * Copyright (c) 2007 A Sidwell <andi@takkaria.org>
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

#include "player.h"

/**
 * Player food values
 */
#define PY_FOOD_MAX 	17000	/* Food value (Bloated) */
#define PY_FOOD_FULL	10000	/* Food value (Normal) */
#define PY_FOOD_ALERT	2000	/* Food value (Hungry) */
#define PY_FOOD_WEAK	1000	/* Food value (Weak) */
#define PY_FOOD_FAINT	500		/* Food value (Fainting) */
#define PY_FOOD_STARVE	100		/* Food value (Starving) */

/**
 * Effect failure flag types
 */
enum {
	TMD_FAIL_FLAG_OBJECT = 1,
	TMD_FAIL_FLAG_RESIST,
	TMD_FAIL_FLAG_VULN
};

/**
 * Timed effects
 */
enum
{
	#define TMD(a, b, c, d, e, f, g, h, i, j, k) TMD_##a,
	#include "list-player-timed.h"
	#undef TMD
	TMD_MAX
};

int timed_name_to_idx(const char *name);
const char *timed_idx_to_name(int type);
const char *timed_idx_to_desc(int type);
int timed_protect_flag(int type);
bool player_set_timed(struct player *p, int idx, int v, bool notify);
bool player_inc_timed(struct player *p, int idx, int v, bool notify,
					  bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify);
bool player_clear_timed(struct player *p, int idx, bool notify);
bool player_set_food(struct player *p, int v);

