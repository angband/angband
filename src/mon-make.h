/**
 * \file mon-make.h
 * \brief Structures and functions for monster creation / deletion.
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

#ifndef MONSTER_MAKE_H
#define MONSTER_MAKE_H

#include "monster.h"

void delete_monster_idx(int m_idx);
void delete_monster(struct loc grid);
void monster_index_move(int i1, int i2);
void compact_monsters(int num_to_compact);
void wipe_mon_list(struct chunk *c, struct player *p);
s16b mon_pop(struct chunk *c);
void get_mon_num_prep(bool (*get_mon_num_hook)(struct monster_race *race));
struct monster_race *get_mon_num(int level);
int mon_create_drop_count(const struct monster_race *race, bool maximize,
	bool specific, int *specific_count);
void mon_create_mimicked_object(struct chunk *c, struct monster *mon,
								int index);
s16b place_monster(struct chunk *c, struct loc grid, struct monster *mon,
				   byte origin);
int mon_hp(const struct monster_race *race, aspect hp_aspect);
bool place_new_monster(struct chunk *c, struct loc grid,
					   struct monster_race *race, bool sleep, bool group_ok,
					   struct monster_group_info group_info, byte origin);
bool pick_and_place_monster(struct chunk *c, struct loc grid, int depth, 
							bool sleep,	bool group_okay, byte origin);
bool pick_and_place_distant_monster(struct chunk *c, struct player *p, int dis,
									bool sleep, int depth);

#endif /* MONSTER_MAKE_H */
