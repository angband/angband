/**
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
#ifndef INCLUDED_BORG_PROJECTION_H
#define INCLUDED_BORG_PROJECTION_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"
#include "../z-type.h"

#ifdef ALLOW_BORG

/*
 * Current targetted location
 */
extern struct loc borg_target_loc;

/*
 * Check a path for line of sight
 */
extern bool borg_los(int y1, int x1, int y2, int x2);

/*
 * Check the projection from (x1,y1) to (x2,y2)
 */
extern bool borg_projectable(int y1, int x1, int y2, int x2);
extern bool borg_offset_projectable(int y1, int x1, int y2, int x2);

/*
 * Check the projection from (x1,y1) to (x2,y2).
 */
extern bool borg_projectable_pure(int y1, int x1, int y2, int x2);
extern bool borg_projectable_dark(int y1, int x1, int y2, int x2);

/*
 * Calculate "incremental motion".
 */
extern void borg_inc_motion(int *y, int *x, int y1, int x1, int y2, int x2);

/*
 * Helper to change the old x/y/x2/y2 to loc/loc
 */
extern int borg_distance(int y, int x, int y2, int x2);

/*
 * Target a location.  Can be used alone or at "Direction?" prompt.
 */
extern bool borg_target(struct loc target);

/*
 * Find a wall to target.
 */
extern bool borg_target_unknown_wall(int y, int x);

#endif
#endif
