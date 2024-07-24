/**
 * \file borg-cave-util.c
 * \brief Misc routines for cave or square analysis
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

#include "borg-cave-util.h"

#ifdef ALLOW_BORG

#include "../cave.h"
#include "../ui-term.h"

#include "borg-cave.h"

/* Is this grid a grid which can be stepped on or can I see through it */
bool borg_cave_floor_bold(int y, int X)
{
    if (square_in_bounds_fully(cave, loc(X, y))) {
        if ((borg_grids[y][X].feat == FEAT_FLOOR) || (borg_grids[y][X].trap)
            || (borg_grids[y][X].feat == FEAT_LESS)
            || (borg_grids[y][X].feat == FEAT_MORE)
            || (borg_grids[y][X].feat == FEAT_BROKEN)
            || (borg_grids[y][X].feat == FEAT_OPEN))
            return true;
    }
    return false;
}

/* is this a floor grid */
bool borg_cave_floor_grid(borg_grid *ag)
{
    if (ag->feat == FEAT_NONE || ag->feat == FEAT_FLOOR || ag->feat == FEAT_OPEN
        || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS
        || ag->feat == FEAT_BROKEN || ag->feat == FEAT_PASS_RUBBLE
        || ag->feat == FEAT_LAVA)
        return true;
    return false;
}

/*
 * Checks if a grid doesn't need a glyph to protect it
 */
bool borg_feature_protected(borg_grid *ag)
{
    if (ag->glyph || ag->kill
        || ((ag->feat >= FEAT_CLOSED) && (ag->feat <= FEAT_PERM)))
        return true;
    return false;
}

/*
 * get the panel height
 */
int borg_panel_hgt(void)
{
    int panel_hgt;

    /* Use dimensions that match those in ui-output.c. */
    if (Term == term_screen) {
        panel_hgt = SCREEN_HGT;
    } else {
        panel_hgt = Term->hgt / tile_height;
    }
    /* Bound below to avoid division by zero. */
    return MAX(panel_hgt, 1);
}

/*
 * get the panel width
 */
int borg_panel_wid(void)
{
    int panel_wid;

    /* Use dimensions that match those in ui-output.c. */
    if (Term == term_screen) {
        panel_wid = SCREEN_WID;
    } else {
        panel_wid = Term->wid / tile_width;
    }
    /* Bound below to avoid division by zero. */
    return MAX(panel_wid, 1);
}

#endif
