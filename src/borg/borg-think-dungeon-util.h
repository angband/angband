/**
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_THINK_DUNGEON_UTIL_H
#define INCLUDED_BORG_THINK_DUNGEON_UTIL_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * money Scumming is a type of town scumming for money
 */
extern bool borg_money_scum(void);

/*
 * Attempt a series of maneuvers to stay alive when you run out of light
 */
extern bool borg_think_dungeon_light(void);

/*
 * This is an exploitation function.  The borg will stair scum
 * in the dungeon to grab items close to the stair.
 */
extern bool borg_think_stair_scum(bool from_town);

/*
 * Leave the level if necessary (or bored)
 */
extern bool borg_leave_level(bool bored);

/*
 * Excavate an existing vault using ranged spells.
 */
extern bool borg_excavate_vault(int range);

#endif
#endif
