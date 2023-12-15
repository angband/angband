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

#ifndef INCLUDED_BORG_FLOW_STAIRS_H
#define INCLUDED_BORG_FLOW_STAIRS_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-flow.h"

/*
 * Track "stairs up"
 */
extern struct borg_track track_less;

/*
 * Track "stairs down"
 */
extern struct borg_track track_more;

/*
 * Do a Stair-Flow.  Look at how far away this grid is to my closest stair
 */
extern int borg_flow_cost_stair(int y, int x, int b_stair);

/*
 * Prepare to flee the level via stairs
 */
extern bool borg_flow_stair_both(int why, bool sneak);

/*
 * Prepare to flow towards "up" stairs
 */
extern bool borg_flow_stair_less(int why, bool sneak);

/*
 * Prepare to flow towards "down" stairs
 */
extern bool borg_flow_stair_more(int why, bool sneak, bool brave);

/*
 * Cast spells before leaving the level
 */
extern bool borg_prep_leave_level_spells(void);

extern void borg_init_flow_stairs(void);
extern void borg_free_flow_stairs(void);

#endif
#endif
