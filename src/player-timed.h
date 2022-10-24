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
	TMD_FAIL_FLAG_PLAYER,
	TMD_FAIL_FLAG_TIMED_EFFECT
};

/**
 * Bits in timed_effect_data's flags field
 */
enum {
	/* Increases to duration will be blocked if effect is already active */
	TMD_FLAG_NONSTACKING = 0x01,
};

struct timed_grade {
	int grade;
	uint8_t color;
	int max;
	char *name;
	char *up_msg;
	char *down_msg;
	struct timed_grade *next;
};

struct timed_failure {
	struct timed_failure *next;
	int code; /* one of the TMD_FAIL_FLAG_* constants */
	int idx; /* index for object or player flag, timed effect, element */
};

/**
 * Data struct
 */
struct timed_effect_data {
	const char *name;
	uint32_t flag_redraw;
	uint32_t flag_update;

	char *desc;
	char *on_end;
	char *on_increase;
	char *on_decrease;
	int msgt;
	struct timed_failure *fail;
	struct timed_grade *grade;
	/* This effect chain is triggered when the timed effect starts. */
	struct effect *on_begin_effect;
	/* This effect chain is triggered when the timed effect lapses. */
	struct effect *on_end_effect;
	bitflag flags;
	int lower_bound;
	int oflag_dup;
	bool oflag_syn;
	int temp_resist;
	int temp_brand;
	int temp_slay;
};

/**
 * Holds state while parsing.  Exposed for use by unit test cases.
 */
struct timed_effect_parse_state {
	/* Points to timed effect being populated.  May be NULL. */
	struct timed_effect_data *t;
	/*
	 * Points to the most recent effect chain being modified in the timed
	 * effect.  May be NULL.
	 */
	struct effect* e;
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
bool player_set_timed(struct player *p, int idx, int v, bool notify,
	bool can_disturb);
bool player_inc_check(struct player *p, int idx, bool lore);
bool player_inc_timed(struct player *p, int idx, int v, bool notify,
	bool can_disturb, bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify,
	bool can_disturb);
bool player_clear_timed(struct player *p, int idx, bool notify,
	bool can_disturb);

#endif /* !PLAYER_TIMED_H */
