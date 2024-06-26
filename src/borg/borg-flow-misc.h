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

#ifndef INCLUDED_BORG_FLOW_MISC_H
#define INCLUDED_BORG_FLOW_MISC_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Locate the store doors
 */
extern int *track_shop_x;
extern int *track_shop_y;

/*
 * Track the mineral veins with treasure
 */
extern struct borg_track track_vein;

/*
 * check to make sure there are no monsters around
 * that should prevent resting
 */
extern bool borg_check_rest(int y, int x);

/*
 * Do a "reverse" flow from the player outwards
 */
extern void borg_flow_reverse(int depth, bool optimize, bool avoid,
    bool tunneling, int stair_idx, bool sneak);

/*
 * Check a floor grid for "happy" status
 */
extern bool borg_happy_grid_bold(int y, int x);

/*
 * go someplace safe to rest
 */
extern bool borg_flow_recover(bool viewable, int dist);

/*
 * Prepare to "flow" towards mineral veins with treasure
 */
extern bool borg_flow_vein(bool viewable, int nearness);

/*
 * Search carefully for secret doors and such
 */
extern bool borg_flow_spastic(bool bored);

/*
 * Prepare to "flow" towards a specific shop entry
 */
extern bool borg_flow_shop_entry(int i);

/*
 * Prepare to flow towards light
 */
extern bool borg_flow_light(int why);

/*
 * Prepare to flow towards a vault grid which can be excavated
 */
extern bool borg_flow_vault(int nearness);

/*
 * Act twitchy
 */
extern bool borg_twitchy(void);

/*
 * Given a "source" and "target" locations, extract a "direction",
 */
extern int borg_extract_dir(int y1, int x1, int y2, int x2);

/*
 * Given a "source" and "target" locations, travel in a "direction",
 */
extern int borg_goto_dir(int y1, int x1, int y2, int x2);

/*
 * Make sure the given square isn't "too far" from the stairs.
 */
extern bool borg_flow_far_from_stairs(int x, int y, int b_stair);
extern bool borg_flow_far_from_stairs_dist(int x, int y, int b_stair, int distance);

extern void borg_init_flow_misc(void);
extern void borg_free_flow_misc(void);

#endif
#endif
