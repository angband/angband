/**
 * \file borg-cave.c
 * \brief Track what the borg thinks the dungeon looks like
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

#include "borg-cave.h"

#ifdef ALLOW_BORG

#include "../init.h"

#include "borg-init.h"
#include "borg-io.h"

borg_grid *borg_grids[AUTO_MAX_Y]; /* The grids */

void borg_init_cave(void)
{
    /* sanity check  */
    if (DUNGEON_WID != z_info->dungeon_wid
        || DUNGEON_HGT != z_info->dungeon_hgt) {
        borg_note("**STARTUP FAILURE** dungeon size miss match");
        borg_init_failure = true;
    }

    /* Make each row of grids */
    for (int y = 0; y < AUTO_MAX_Y; y++) {
        /* Make each row */
        borg_grids[y] = mem_zalloc(AUTO_MAX_X * sizeof(borg_grid));
    }
}

void borg_free_cave(void)
{
    for (int y = 0; y < AUTO_MAX_Y; ++y) {
        mem_free(borg_grids[y]);
        borg_grids[y] = NULL;
    }
}

#endif
