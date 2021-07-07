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

struct timed_grade {
	int grade;
	byte color;
	int max;
	char *name;
	char *up_msg;
	char *down_msg;
	struct timed_grade *next;
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
	char *on_end;
	char *on_increase;
	char *on_decrease;
	int msgt;
	int fail_code;
	int fail;
	struct timed_grade *grade;
	int oflag_dup;
	bool oflag_syn;
	int temp_resist;
	int temp_brand;
	int temp_slay;
};

/**
 * Player food values
 */
extern int PY_FOOD_MAX; 	/* Food value (Bloated) */
extern int PY_FOOD_FULL;	/* Food value (Normal) */
extern int PY_FOOD_HUNGRY;	/* Food value (Hungry) */
extern int PY_FOOD_WEAK;	/* Food value (Weak) */
extern int PY_FOOD_FAINT;	/* Food value (Fainting) */
extern int PY_FOOD_STARVE;	/* Food value (Starving) */

extern struct file_parser player_timed_parser;
extern struct timed_effect_data timed_effects[TMD_MAX];

int timed_name_to_idx(const char *name);
bool player_timed_grade_eq(struct player *p, int idx, const char *match);
bool player_set_timed(struct player *p, int idx, int v, bool notify);
bool player_inc_check(struct player *p, int idx, bool lore);
bool player_inc_timed(struct player *p, int idx, int v, bool notify,
					  bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify);
bool player_clear_timed(struct player *p, int idx, bool notify);

#endif /* !PLAYER_TIMED_H */
