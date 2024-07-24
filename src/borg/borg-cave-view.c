/**
 * \file borg-cave-view.c
 * \brief Routines to update the cave view
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

#include "borg-cave-view.h"

#ifdef ALLOW_BORG

#include "borg-cave-view.h"

#include "../cave.h"
#include "../init.h"

#include "borg-cave-util.h"
#include "borg-cave.h"
#include "borg-projection.h"
#include "borg-trait.h"

/*
 * Maintain a set of grids marked as "BORG_VIEW"
 */

int16_t borg_view_n = 0;

uint8_t borg_view_x[AUTO_VIEW_MAX];
uint8_t borg_view_y[AUTO_VIEW_MAX];

/*
 * Clear the viewable space
 */
void borg_forget_view(void)
{
    int i;

    borg_grid *ag;

    /* None to forget */
    if (!borg_view_n)
        return;

    /* Clear them all */
    for (i = 0; i < borg_view_n; i++) {
        int y = borg_view_y[i];
        int x = borg_view_x[i];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Forget that the grid is viewable */
        ag->info &= ~BORG_VIEW;
    }

    /* None left */
    borg_view_n = 0;
}

/*
 * This macro allows us to efficiently add a grid to the "view" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "view" array, and we are never
 * called when the "view" array is full.
 */
#define borg_cave_view_hack(A, y, X) \
    (A)->info |= BORG_VIEW;          \
    borg_view_y[borg_view_n] = (y);  \
    borg_view_x[borg_view_n] = (X);  \
    borg_view_n++

/*
 * Helper function for "borg_update_view()" below
 *
 * See "update_view_aux()" in "cave.c" for complete documentation.
 */
static bool borg_update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, vis1, vis2, wall;

    borg_grid *ag;

    borg_grid *g1_ag;
    borg_grid *g2_ag;

    /* Access the grids */
    g1_ag = &borg_grids[y1][x1];
    g2_ag = &borg_grids[y2][x2];

    /* Check for walls */
    f1 = (borg_cave_floor_grid(g1_ag));
    f2 = (borg_cave_floor_grid(g2_ag));

    /* Totally blocked by physical walls */
    if (!f1 && !f2)
        return true;

    /* Check for visibility */
    v1 = (f1 && (g1_ag->info & BORG_VIEW));
    v2 = (f2 && (g2_ag->info & BORG_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2)
        return true;

    /* Access the grid */
    ag = &borg_grids[y][x];

    /* Check for walls */
    wall = (!borg_cave_floor_grid(ag));

    /* Check the "ease" of visibility */
    vis1 = (v1 && (g1_ag->info & BORG_XTRA));
    vis2 = (v2 && (g2_ag->info & BORG_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (vis1 && vis2) {
        ag->info |= BORG_XTRA;

        borg_cave_view_hack(ag, y, x);

        return wall;
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (vis1) {
        borg_cave_view_hack(ag, y, x);

        return wall;
    }

    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2) {
        /* ag->info |= BORG_XTRA; */

        borg_cave_view_hack(ag, y, x);

        return wall;
    }

    /* Mega-Hack -- the "borg_los()" function works poorly on walls */
    if (wall) {
        borg_cave_view_hack(ag, y, x);

        return wall;
    }

    /* Hack -- check line of sight */
    if (borg_los(borg.c.y, borg.c.x, y, x)) {
        borg_cave_view_hack(ag, y, x);

        return wall;
    }

    /* Assume no line of sight. */
    return true;
}

/*
 * Calculate the region currently "viewable" by the player
 *
 * See "update_view()" in "cave.c" for complete documentation
 *
 * It is very important that the "player grid" be the first grid in the
 * array of "BORG_VIEW" grids, since this is assumed in several places.
 */
void borg_update_view(void)
{
    int n, m, d, k, y, x, z;

    int se, sw, ne, nw, es, en, ws, wn;

    int full, over;

    borg_grid *ag;

    /*** Initialize ***/

    /* Full radius (20) */
    full = z_info->max_sight;

    /* Octagon factor (30) */
    over = z_info->max_sight * 3 / 2;

    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < borg_view_n; n++) {
        y = borg_view_y[n];
        x = borg_view_x[n];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Mark the grid as not in "view" */
        ag->info &= ~(BORG_VIEW);
    }

    /* Start over with the "view" array */
    borg_view_n = 0;

    /*** Step 1 -- adjacent grids ***/

    /* Now start on the player */
    y = borg.c.y;
    x = borg.c.x;

    /* Access the grid */
    ag = &borg_grids[y][x];

    /* Assume the player grid is easily viewable */
    ag->info |= BORG_XTRA;

    /* Assume the player grid is viewable */
    borg_cave_view_hack(ag, y, x);

    /*** Step 2 -- Major Diagonals ***/

    /* Hack -- Limit */
    z = full * 2 / 3;

    /* Scan south-east */
    for (d = 1; d <= z; d++) {
        if (!square_in_bounds_fully(cave, loc(x + d, y + d)))
            continue;

        ag = &borg_grids[y + d][x + d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y + d, x + d);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Scan south-west */
    for (d = 1; d <= z; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x - d, y + d)))
            continue;

        ag = &borg_grids[y + d][x - d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y + d, x - d);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Scan north-east */
    for (d = 1; d <= z; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x + d, y - d)))
            continue;

        ag = &borg_grids[y - d][x + d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y - d, x + d);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Scan north-west */
    for (d = 1; d <= z; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x - d, y - d)))
            continue;

        ag = &borg_grids[y - d][x - d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y - d, x - d);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x, y + d)))
            continue;
        ag = &borg_grids[y + d][x];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y + d, x);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x, y - d)))
            continue;

        ag = &borg_grids[y - d][x];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y - d, x);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x + d, y)))
            continue;
        ag = &borg_grids[y][x + d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x + d);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++) {
        /* Caution */
        if (!square_in_bounds_fully(cave, loc(x - d, y)))
            continue;

        ag = &borg_grids[y][x - d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x - d);
        if (!borg_cave_floor_grid(ag))
            break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;

    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++) {
        int ypn, ymn, xpn, xmn;

        /* Acquire the "bounds" of the maximal circle */
        z = over - n - n;
        if (z > full - n)
            z = full - n;
        while ((z + n + (n >> 1)) > full)
            z--;

        /* Access the four diagonal grids */
        ypn = y + n;
        ymn = y - n;
        xpn = x + n;
        xmn = x - n;

        /* South strip */
        if (ypn < AUTO_MAX_Y - 1) {
            /* Maximum distance */
            m = MIN(z, (AUTO_MAX_Y - 1) - ypn);

            /* East side */
            if ((xpn <= AUTO_MAX_X - 1) && (n < se)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn + d, xpn, ypn + d - 1, xpn - 1,
                            ypn + d - 1, xpn)) {
                        if (n + d >= se)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                se = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < sw)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn + d, xmn, ypn + d - 1, xmn + 1,
                            ypn + d - 1, xmn)) {
                        if (n + d >= sw)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                sw = k + 1;
            }
        }

        /* North strip */
        if (ymn > 0) {
            /* Maximum distance */
            m = MIN(z, ymn);

            /* East side */
            if ((xpn <= AUTO_MAX_X - 1) && (n < ne)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn - d, xpn, ymn - d + 1, xpn - 1,
                            ymn - d + 1, xpn)) {
                        if (n + d >= ne)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ne = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < nw)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn - d, xmn, ymn - d + 1, xmn + 1,
                            ymn - d + 1, xmn)) {
                        if (n + d >= nw)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                nw = k + 1;
            }
        }

        /* East strip */
        if (xpn < AUTO_MAX_X - 1) {
            /* Maximum distance */
            m = MIN(z, (AUTO_MAX_X - 1) - xpn);

            /* South side */
            if ((ypn <= AUTO_MAX_Y - 1) && (n < es)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn, xpn + d, ypn - 1, xpn + d - 1,
                            ypn, xpn + d - 1)) {
                        if (n + d >= es)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                es = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < en)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn, xpn + d, ymn + 1, xpn + d - 1,
                            ymn, xpn + d - 1)) {
                        if (n + d >= en)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                en = k + 1;
            }
        }

        /* West strip */
        if (xmn > 0) {
            /* Maximum distance */
            m = MIN(z, xmn);

            /* South side */
            if ((ypn <= AUTO_MAX_Y - 1) && (n < ws)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn, xmn - d, ypn - 1, xmn - d + 1,
                            ypn, xmn - d + 1)) {
                        if (n + d >= ws)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ws = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < wn)) {
                /* Scan */
                for (k = n, d = 1; d <= m; d++) {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn, xmn - d, ymn + 1, xmn - d + 1,
                            ymn, xmn - d + 1)) {
                        if (n + d >= wn)
                            break;
                    }

                    /* Track most distant "non-blockage" */
                    else {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                wn = k + 1;
            }
        }
    }

    /*** Step 5 -- Complete the algorithm ***/

    /* Update all the new grids */
    for (n = 0; n < borg_view_n; n++) {
        y = borg_view_y[n];
        x = borg_view_x[n];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Clear the "BORG_XTRA" flag */
        ag->info &= ~BORG_XTRA;
    }
}

#endif
