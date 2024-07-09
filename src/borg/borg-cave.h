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

#ifndef INCLUDED_BORG_CAVE_H
#define INCLUDED_BORG_CAVE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Maximum possible dungeon size
 */
/* NOTE: this corresponds to z_info->dungeon_hgt/dungeon_wid */
/* a test is done at the start of borg to make sure the values are right */
#define DUNGEON_WID 198
#define DUNGEON_HGT 66

#define AUTO_MAX_X DUNGEON_WID
#define AUTO_MAX_Y DUNGEON_HGT

/*
 * Forward declare
 */
typedef struct borg_grid borg_grid;

/*
 * A grid in the dungeon.
 *
 * There is a terrain feature type, which may be incorrect.  It is
 * more or less based on the standard "feature" values, but some of
 * the legal values are never used, such as "secret door", and others
 * are used in bizarre ways, such as "invisible trap".
 *
 * There is an object (take) index into the "object tracking" array.
 *
 * There is a monster (kill) index into the "monster tracking" array.
 *
 * There is a byte "xtra" which tracks how much "searching" has been done
 * in the grid or in any grid next to the grid.
 *
 * To perform "navigation" from one place to another, the "flow" routines
 * are used, which place "cost" information into the "cost" fields.  Then,
 * if the path is clear, the "cost" information is copied into the "flow"
 * fields, which are used for the actual navigation.  This allows multiple
 * routines to check for "possible" flowing, without hurting the current
 * flow, which may have taken a long time to construct.  We also assume
 * that the Borg never needs to follow a path longer than 250 grids long.
 * Note that the "cost" fields have been moved into external arrays.
 *
 * traps and glyphs are now separate flags
 *
 * Hack -- note that the "char" zero will often crash the system!
 */
struct borg_grid {
    uint8_t  feat; /* Grid type */
    uint16_t info; /* Grid flags */
    bool     trap;
    bool     glyph;
    bool     web;
    uint8_t  store;

    uint8_t  take; /* Object index */
    uint8_t  kill; /* Monster index */

    uint8_t  xtra; /* Extra field (search count) */
};
/*
 * The current map
 */
extern borg_grid *borg_grids[AUTO_MAX_Y]; /* The grids */

extern void borg_init_cave(void);
extern void borg_free_cave(void);

#endif
#endif
