/**
 * \file  borg-cave-light.c
 * \brief handle lighted squares in the cave
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

#include "borg-cave-light.h"

#ifdef ALLOW_BORG

#include "../cave.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-projection.h"
#include "borg-trait.h"

/*
 * Maintain a set of grids marked as "BORG_LIGHT"
 */

int16_t borg_light_n = 0;
uint8_t borg_light_x[AUTO_LIGHT_MAX];
uint8_t borg_light_y[AUTO_LIGHT_MAX];

/*
 * Maintain a set of grids marked as "BORG_GLOW"
 */

int16_t borg_glow_n = 0;

uint8_t borg_glow_x[AUTO_LIGHT_MAX];
uint8_t borg_glow_y[AUTO_LIGHT_MAX];

/*
 * XXX XXX XXX
 *
 * This macro allows us to efficiently add a grid to the "lite" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "lite" array, and we are never
 * called when the "lite" array is full.
 */
#define borg_cave_light_hack(y, x)                                             \
    borg_grids[y][x].info |= BORG_LIGHT;                                       \
    borg_light_y[borg_light_n] = (y);                                          \
    borg_light_x[borg_light_n] = (x);                                          \
    borg_light_n++

/*
 * Update the set of grids "illuminated" by the player's lite.
 *
 * See "update_LIGHT" in "cave.c" for complete documentation
 *
 * It is very important that the "player grid" be the first grid in the
 * array of "BORG_LIGHT" grids, since this is assumed in several places.
 */
void borg_update_light(void)
{
    int i, x, y, min_x, max_x, min_y, max_y;

    /*** Clear old grids ***/

    /* Clear them all */
    for (i = 0; i < borg_light_n; i++) {
        y = borg_light_y[i];
        x = borg_light_x[i];

        /* Mark the grid as not "lite" */
        borg_grids[y][x].info &= ~BORG_LIGHT;
    }

    /* None left */
    borg_light_n = 0;

    /* Hack -- Player has no lite */
    if (borg_items[INVEN_LIGHT].iqty <= 0)
        return;

    /*** Collect the new "lite" grids ***/

    /* Player grid */
    borg_cave_light_hack(borg.c.y, borg.c.x);

    /* Radius 1 -- torch radius */
    if (borg.trait[BI_LIGHT] >= 1) {
        /* Adjacent grid */
        borg_cave_light_hack(borg.c.y + 1, borg.c.x);
        borg_cave_light_hack(borg.c.y - 1, borg.c.x);
        borg_cave_light_hack(borg.c.y, borg.c.x + 1);
        borg_cave_light_hack(borg.c.y, borg.c.x - 1);

        /* Diagonal grids */
        borg_cave_light_hack(borg.c.y + 1, borg.c.x + 1);
        borg_cave_light_hack(borg.c.y + 1, borg.c.x - 1);
        borg_cave_light_hack(borg.c.y - 1, borg.c.x + 1);
        borg_cave_light_hack(borg.c.y - 1, borg.c.x - 1);
    }

    /* Radius 2 -- lantern radius */
    if (borg.trait[BI_LIGHT] >= 2 && borg.c.y + 2 < AUTO_MAX_Y
        && borg.c.y - 2 > 0 && borg.c.x + 2 < AUTO_MAX_X && borg.c.x - 2 > 0) {
        /* South of the player */
        if (borg_cave_floor_bold(borg.c.y + 2, borg.c.x)) {
            borg_cave_light_hack(borg.c.y + 2, borg.c.x);
            borg_cave_light_hack(borg.c.y + 2, borg.c.x + 2);
            borg_cave_light_hack(borg.c.y + 2, borg.c.x - 2);
        }

        /* North of the player */
        if (borg_cave_floor_bold(borg.c.y - 2, borg.c.x)) {
            borg_cave_light_hack(borg.c.y - 2, borg.c.x);
            borg_cave_light_hack(borg.c.y - 2, borg.c.x + 2);
            borg_cave_light_hack(borg.c.y - 2, borg.c.x - 2);
        }

        /* East of the player */
        if (borg_cave_floor_bold(borg.c.y, borg.c.x + 2)) {
            borg_cave_light_hack(borg.c.y, borg.c.x + 2);
            borg_cave_light_hack(borg.c.y + 1, borg.c.x + 2);
            borg_cave_light_hack(borg.c.y - 1, borg.c.x + 2);
        }

        /* West of the player */
        if (borg_cave_floor_bold(borg.c.y, borg.c.x - 2)) {
            borg_cave_light_hack(borg.c.y, borg.c.x - 2);
            borg_cave_light_hack(borg.c.y + 2, borg.c.x - 2);
            borg_cave_light_hack(borg.c.y - 2, borg.c.x - 2);
        }
    }

    /* Radius 3+ -- artifact radius */
    if (borg.trait[BI_LIGHT] >= 3 && borg.c.y + 3 < AUTO_MAX_Y
        && borg.c.y - 3 > 0 && borg.c.x + 3 < AUTO_MAX_X && borg.c.x - 3 > 0) {
        int d, p;

        /* Maximal radius */
        p = borg.trait[BI_LIGHT];

        /* Paranoia -- see "LITE_MAX" */
        if (p > 5)
            p = 5;

        /* South-East of the player */
        if (borg_cave_floor_bold(borg.c.y + 3, borg.c.x + 3)) {
            borg_cave_light_hack(borg.c.y + 3, borg.c.x + 3);
        }

        /* South-West of the player */
        if (borg_cave_floor_bold(borg.c.y + 3, borg.c.x - 3)) {
            borg_cave_light_hack(borg.c.y + 3, borg.c.x - 3);
        }

        /* North-East of the player */
        if (borg_cave_floor_bold(borg.c.y - 3, borg.c.x + 3)) {
            borg_cave_light_hack(borg.c.y - 3, borg.c.x + 3);
        }

        /* North-West of the player */
        if (borg_cave_floor_bold(borg.c.y - 3, borg.c.x - 3)) {
            borg_cave_light_hack(borg.c.y - 3, borg.c.x - 3);
        }

        /* Maximal north */
        min_y = borg.c.y - p;
        if (min_y < 0)
            min_y = 0;

        /* Maximal south */
        max_y = borg.c.y + p;
        if (max_y > AUTO_MAX_Y - 1)
            max_y = AUTO_MAX_Y - 1;

        /* Maximal west */
        min_x = borg.c.x - p;
        if (min_x < 0)
            min_x = 0;

        /* Maximal east */
        max_x = borg.c.x + p;
        if (max_x > AUTO_MAX_X - 1)
            max_x = AUTO_MAX_X - 1;

        /* Scan the maximal box */
        for (y = min_y; y <= max_y; y++) {
            for (x = min_x; x <= max_x; x++) {
                int dy = (borg.c.y > y) ? (borg.c.y - y) : (y - borg.c.y);
                int dx = (borg.c.x > x) ? (borg.c.x - x) : (x - borg.c.x);

                /* Skip the "central" grids (above) */
                if ((dy <= 2) && (dx <= 2))
                    continue;

                /* Hack -- approximate the distance */
                d = (dy > dx) ? (dy + (dx >> 1)) : (dx + (dy >> 1));

                /* Skip distant grids */
                if (d > p)
                    continue;

                /* Viewable, nearby, grids get "torch lit" */
                if (borg_grids[y][x].info & BORG_VIEW) {
                    /* This grid is "torch lit" */
                    borg_cave_light_hack(y, x);
                }
            }
        }
    }
}

#endif
