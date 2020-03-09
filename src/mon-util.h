/**
 * \file mon-util.h
 * \brief Functions for monster utilities.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef MONSTER_UTILITIES_H
#define MONSTER_UTILITIES_H

#include "monster.h"
#include "mon-msg.h"

const char *describe_race_flag(int flag);
void create_mon_flag_mask(bitflag *f, ...);
struct monster_race *lookup_monster(const char *name);
struct monster_base *lookup_monster_base(const char *name);
bool match_monster_bases(const struct monster_base *base, ...);
void update_mon(struct monster *mon, struct chunk *c, bool full);
void update_monsters(bool full);
bool monster_carry(struct chunk *c, struct monster *mon, struct object *obj);
void monster_swap(struct loc grid1, struct loc grid2);
void monster_wake(struct monster *mon, bool notify, int aware_chance);
bool monster_can_see(struct chunk *c, struct monster *mon, struct loc grid);
void become_aware(struct monster *m);
void update_smart_learn(struct monster *mon, struct player *p, int flag,
						int pflag, int element);
bool find_any_nearby_injured_kin(struct chunk *c, const struct monster *mon);
struct monster *choose_nearby_injured_kin(struct chunk *c, const struct monster *mon);
void monster_death(struct monster *mon, bool stats);
bool mon_take_nonplayer_hit(int dam, struct monster *t_mon,
							enum mon_messages hurt_msg,
							enum mon_messages die_msg);
bool mon_take_hit(struct monster *mon, int dam, bool *fear, const char *note);
void kill_arena_monster(struct monster *mon);
void monster_take_terrain_damage(struct monster *mon);
bool monster_taking_terrain_damage(struct monster *mon);
struct monster *get_commanded_monster(void);
struct object *get_random_monster_object(struct monster *mon);
void steal_monster_item(struct monster *mon, int midx);
bool monster_change_shape(struct monster *mon);
bool monster_revert_shape(struct monster *mon);

#endif /* MONSTER_UTILITIES_H */
