/**
 * \file player-util.h
 * \brief Player utility functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2014 Nick McConnell
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

#ifndef PLAYER_UTIL_H
#define PLAYER_UTIL_H

#include "cmd-core.h"
#include "player.h"

/* Player regeneration constants */
#define PY_REGEN_NORMAL		197		/* Regen factor*2^16 when full */
#define PY_REGEN_WEAK		98		/* Regen factor*2^16 when weak */
#define PY_REGEN_FAINT		33		/* Regen factor*2^16 when fainting */
#define PY_REGEN_HPBASE		1442	/* Min amount hp regen*2^16 */
#define PY_REGEN_MNBASE		524		/* Min amount mana regen*2^16 */

/* Player over-exertion */
enum {
	PY_EXERT_NONE = 0x00,
	PY_EXERT_CON = 0x01,
	PY_EXERT_FAINT = 0x02,
	PY_EXERT_SCRAMBLE = 0x04,
	PY_EXERT_CUT = 0x08,
	PY_EXERT_CONF = 0x10,
	PY_EXERT_HALLU = 0x20,
	PY_EXERT_SLOW = 0x40,
	PY_EXERT_HP = 0x80
};

/**
 * Special values for the number of turns to rest, these need to be
 * negative numbers, as postive numbers are taken to be a turncount,
 * and zero means "not resting". 
 */
enum 
{
	REST_COMPLETE = -2,
	REST_ALL_POINTS = -1,
	REST_SOME_POINTS = -3
};

/**
 * Minimum number of turns required for regeneration to kick in during resting.
 */
#define REST_REQUIRED_FOR_REGEN 5

int dungeon_get_next_level(int dlev, int added);
void player_set_recall_depth(struct player *p);
bool player_get_recall_depth(struct player *p);
void dungeon_change_level(struct player *p, int dlev);
void take_hit(struct player *p, int dam, const char *kb_str);
void death_knowledge(struct player *p);
int energy_per_move(struct player *p);
s16b modify_stat_value(int value, int amount);
void player_regen_hp(struct player *p);
void player_regen_mana(struct player *p);
void player_adjust_hp_precise(struct player *p, s32b hp_gain);
s32b player_adjust_mana_precise(struct player *p, s32b sp_gain);
void convert_mana_to_hp(struct player *p, s32b sp);
void player_update_light(struct player *p);
void player_over_exert(struct player *p, int flag, int chance, int amount);
struct object *player_best_digger(struct player *p, bool forbid_stack);
bool player_attack_random_monster(struct player *p);
int player_check_terrain_damage(struct player *p, struct loc grid);
void player_take_terrain_damage(struct player *p, struct loc grid);
struct player_shape *lookup_player_shape(const char *name);
int shape_name_to_idx(const char *name);
struct player_shape *player_shape_by_idx(int index);
bool player_get_resume_normal_shape(struct player *p, struct command *cmd);
void player_resume_normal_shape(struct player *p);
bool player_is_shapechanged(struct player *p);
bool player_is_trapsafe(struct player *p);
bool player_can_cast(struct player *p, bool show_msg);
bool player_can_study(struct player *p, bool show_msg);
bool player_can_read(struct player *p, bool show_msg);
bool player_can_fire(struct player *p, bool show_msg);
bool player_can_refuel(struct player *p, bool show_msg);
bool player_can_cast_prereq(void);
bool player_can_study_prereq(void);
bool player_can_read_prereq(void);
bool player_can_fire_prereq(void);
bool player_can_refuel_prereq(void);
bool player_book_has_unlearned_spells(struct player *p);
bool player_confuse_dir(struct player *p, int *dir, bool too);
bool player_resting_is_special(s16b count);
bool player_is_resting(struct player *p);
s16b player_resting_count(struct player *p);
void player_resting_set_count(struct player *p, s16b count);
void player_resting_cancel(struct player *p, bool disturb);
bool player_resting_can_regenerate(struct player *p);
void player_resting_step_turn(struct player *p);
void player_resting_complete_special(struct player *p);
int player_get_resting_repeat_count(struct player *p);
void player_set_resting_repeat_count(struct player *p, s16b count);
bool player_of_has(struct player *p, int flag);
bool player_resists(struct player *p, int element);
bool player_is_immune(struct player *p, int element);
void player_place(struct chunk *c, struct player *p, struct loc grid);
void disturb(struct player *p);
void search(struct player *p);

#endif /* !PLAYER_UTIL_H */
