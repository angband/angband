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
#ifndef INCLUDED_BORG_CAVE_UTIL_H
#define INCLUDED_BORG_CAVE_UTIL_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-cave.h"

/*
 * Determine if a grid is a floor grid and only a floor grid
 */
extern bool borg_cave_floor_bold(int y, int X);

/*
 * Grid based version of "borg_cave_floor_bold()"
 */
extern bool borg_cave_floor_grid(borg_grid *ag);

/*
 * A square is protected (doesn't need a glyph)
 */
extern bool borg_feature_protected(borg_grid *ag);

/*
 * get the panel height
 */
extern int borg_panel_hgt(void);

/*
 * get the panel width
 */
extern int borg_panel_wid(void);

#endif
#endif
