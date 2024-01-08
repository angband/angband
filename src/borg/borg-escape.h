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

#ifndef INCLUDED_BORG_ESCAPE_H
#define INCLUDED_BORG_ESCAPE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Attempt to induce WORD_OF_RECALL
 */
extern bool borg_recall(void);

/*
 * Evaluate the "freedom" of the given location
 */
extern int borg_freedom(int y, int x);

/*
 * Help determine if "Teleport" seems like a good idea
 */
extern bool borg_caution_teleport(int emergency, int turns);

/*
 * Help determine if "Phase Door" seems like a good idea
 */
extern bool borg_caution_phase(int emergency, int turns);

/*
 * special teleports
 */
extern bool borg_shadow_shift(int allow_fail);
extern bool borg_dimension_door(int allow_fail);
extern bool borg_allow_teleport(void);

/*
 * Evaluate the likelihood of the borg getting surrounded
 */
extern bool borg_surrounded(void);

/*
 * try to run away
 */
extern bool borg_escape(int b_q);

#endif
#endif
