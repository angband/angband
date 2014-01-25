/*
 * File: mon-lore.h
 * Purpose: Structures and functions for monster recall.
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

#ifndef MONSTER_LORE_H
#define MONSTER_LORE_H

#include "angband.h"

/** Constants **/

/** Macros **/

/** Structures **/

/** Variables **/

/** Functions **/
void cheat_monster_lore(const monster_race *r_ptr, monster_lore *l_ptr);
void wipe_monster_lore(const monster_race *r_ptr, monster_lore *l_ptr);
void lore_do_probe(struct monster *m);
void monster_flags_known(const monster_race *r_ptr, const monster_lore *l_ptr, bitflag flags[RF_SIZE]);
void lore_treasure(struct monster *m_ptr, int num_item, int num_gold);
void lore_title(textblock *tb, const monster_race *r_ptr);
void lore_description(textblock *tb, const monster_race *race, const monster_lore *original_lore, bool spoilers);
void lore_show_interactive(const monster_race *race, const monster_lore *lore);
void lore_show_subwindow(const monster_race *race, const monster_lore *lore);

#endif /* MONSTER_LORE_H */
