/*
 * File: src/monster/mon-power.h
 * Purpose: structures and functions for monster power
 *
 * Copyright (c) 2011 Chris Carr
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

#ifndef MONSTER_POWER_H
#define MONSTER_POWER_H

#include "monster/monster.h"

/** Variables **/
extern s32b tot_mon_power;

/** Functions **/
errr eval_r_power(struct monster_race *races);


#endif /* MONSTER_POWER_H */
