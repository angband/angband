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

#ifndef PLAYER_TIMED_H
#define PLAYER_TIMED_H

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
 * Player cut timer values
 */
#define TMD_CUT_NONE    0
#define TMD_CUT_GRAZE   10
#define TMD_CUT_LIGHT   25
#define TMD_CUT_BAD     50
#define TMD_CUT_NASTY   100
#define TMD_CUT_SEVERE  200
#define TMD_CUT_DEEP    1000

/**
 * Timed effects
 */
enum
{
	#define TMD(a, b, c) TMD_##a,
	#include "list-player-timed.h"
	#undef TMD
	TMD_MAX
};

/**
 * Effect failure flag types
 */
enum {
	TMD_FAIL_FLAG_OBJECT = 1,
	TMD_FAIL_FLAG_RESIST,
	TMD_FAIL_FLAG_VULN,
	TMD_FAIL_FLAG_PLAYER
};

/**
 * Data struct
 */
struct timed_effect_data {
	const char *name;
	u32b flag_redraw;
	u32b flag_update;

	int index;
	char *desc;
	char *on_begin;
	char *on_end;
	char *on_increase;
	char *on_decrease;
	int msgt;
	int fail_code;
	int fail;
};

extern struct file_parser player_timed_parser;
extern struct timed_effect_data timed_effects[TMD_MAX];

int timed_name_to_idx(const char *name);
bool player_set_timed(struct player *p, int idx, int v, bool notify);
bool player_inc_check(struct player *p, int idx, bool lore);
bool player_inc_timed(struct player *p, int idx, int v, bool notify,
					  bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify);
bool player_clear_timed(struct player *p, int idx, bool notify);
bool player_set_food(struct player *p, int v);

#endif /* !PLAYER_TIMED_H */
