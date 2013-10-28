/*
 * File: mon-make.h
 * Purpose: Structures and functions for monster creation / deletion.
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

#include "angband.h"

/** Constants **/

/** Macros **/

/** Structures **/

/** Variables **/

/** Functions **/
void delete_monster_idx(int i);
void delete_monster(int y, int x);
void compact_monsters(int size);
void wipe_mon_list(struct cave *c, struct player *p);
void get_mon_num_prep(void);
s16b get_mon_num(int level);
void player_place(struct cave *c, struct player *p, int y, int x);
s16b place_monster(int y, int x, monster_type *n_ptr, byte origin);
bool place_new_monster(struct cave *, int y, int x, int r_idx, bool slp,
	bool grp, byte origin);
bool pick_and_place_monster(struct cave *c, int y, int x, int depth, bool slp,
	bool grp, byte origin);
bool pick_and_place_distant_monster(struct cave *c, struct loc loc, int dis, bool slp, int depth);
void monster_death(int m_idx, bool stats);
bool mon_take_hit(int m_idx, int dam, bool *fear, const char *note);



#endif /* MONSTER_MAKE_H */
