/**
 * \file mon-summon.h
 * \brief Monster summoning

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

#ifndef MONSTER_SUMMON_H
#define MONSTER_SUMMON_H

#include "monster.h"

/**
 * Flags for summon_specific()
 */
enum summon_flag {
	#define S(a, b, c, d, e, f, g, h) S_##a, 
	#include "list-summon-types.h"
	#undef S
};

/** Variables **/
struct monster_base *kin_base;


/** Functions **/
int summon_name_to_idx(const char *name);
const char *summon_desc(int type);
int summon_message_type(int summon_type);
int summon_specific(int y1, int x1, int lev, int type, bool delay, bool call);

#endif /* MONSTER_SUMMON_H */
