/**
 * \file borg-projection.c
 * \brief used to determine paths between two points
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

#include "borg-projection.h"

#ifdef ALLOW_BORG

#include "../cave.h"

#include "borg-cave-util.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-flow-kill.h"
#include "borg-flow.h"
#include "borg-io.h"
#include "borg-trait.h"

/*
 * Current targetted location
 */
struct loc borg_target_loc;

/*
 * A simple, fast, integer-based line-of-sight algorithm.
 *
 * See "los()" in "cave-view.c" for complete documentation
 */
bool borg_los(int y1, int x1, int y2, int x2)
{

    /* Delta */
    int dx, dy;

    /* Absolute */
    int ax, ay;

    /* Signs */
    int sx, sy;

    /* Fractions */
    int qx, qy;

    /* Scanners */
    int tx, ty;

    /* Scale factors */
    int f1, f2;

    /* Slope, or 1/Slope, of LOS */
    int m;

    /* Extract the offset */
    dy = y2 - y1;
    dx = x2 - x1;

    /* Extract the absolute offset */
    ay = ABS(dy);
    ax = ABS(dx);

    /* Handle adjacent (or identical) grids */
    if ((ax < 2) && (ay < 2))
        return true;

    /* Paranoia -- require "safe" origin */
    if (!square_in_bounds_fully(cave, loc(x1, y1)))
        return false;

    /* Directly South/North */
    if (!dx) {
        /* South -- check for walls */
        if (dy > 0) {
            for (ty = y1 + 1; ty < y2; ty++) {
                if (!borg_cave_floor_bold(ty, x1))
                    return false;
            }
        }

        /* North -- check for walls */
        else {
            for (ty = y1 - 1; ty > y2; ty--) {
                if (!borg_cave_floor_bold(ty, x1))
                    return false;
            }
        }

        /* Assume los */
        return true;
    }

    /* Directly East/West */
    if (!dy) {
        /* East -- check for walls */
        if (dx > 0) {
            for (tx = x1 + 1; tx < x2; tx++) {
                if (!borg_cave_floor_bold(y1, tx))
                    return false;
            }
        }

        /* West -- check for walls */
        else {
            for (tx = x1 - 1; tx > x2; tx--) {
                if (!borg_cave_floor_bold(y1, tx))
                    return false;
            }
        }

        /* Assume los */
        return true;
    }

    /* Extract some signs */
    sx = (dx < 0) ? -1 : 1;
    sy = (dy < 0) ? -1 : 1;

    /* Vertical "knights" */
    if (ax == 1) {
        if (ay == 2) {
            if (borg_cave_floor_bold(y1 + sy, x1))
                return true;
        }
    }

    /* Horizontal "knights" */
    else if (ay == 1) {
        if (ax == 2) {
            if (borg_cave_floor_bold(y1, x1 + sx))
                return true;
        }
    }

    /* Calculate scale factor div 2 */
    f2 = (ax * ay);

    /* Calculate scale factor */
    f1 = f2 << 1;

    /* Travel horizontally */
    if (ax >= ay) {
        /* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
        qy = ay * ay;
        m  = qy << 1;

        tx = x1 + sx;

        /* Consider the special case where slope == 1. */
        if (qy == f2) {
            ty = y1 + sy;
            qy -= f1;
        } else {
            ty = y1;
        }

        /* Note (below) the case (qy == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (x2 - tx) {
            if (!borg_cave_floor_bold(ty, tx))
                return false;

            qy += m;

            if (qy < f2) {
                tx += sx;
            } else if (qy > f2) {
                ty += sy;
                if (!borg_cave_floor_bold(ty, tx))
                    return false;
                qy -= f1;
                tx += sx;
            } else {
                ty += sy;
                qy -= f1;
                tx += sx;
            }
        }
    }

    /* Travel vertically */
    else {
        /* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
        qx = ax * ax;
        m  = qx << 1;

        ty = y1 + sy;

        if (qx == f2) {
            tx = x1 + sx;
            qx -= f1;
        } else {
            tx = x1;
        }

        /* Note (below) the case (qx == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (y2 - ty) {
            if (!borg_cave_floor_bold(ty, tx))
                return false;

            qx += m;

            if (qx < f2) {
                ty += sy;
            } else if (qx > f2) {
                tx += sx;
                if (!borg_cave_floor_bold(ty, tx))
                    return false;
                qx -= f1;
                ty += sy;
            } else {
                tx += sx;
                qx -= f1;
                ty += sy;
            }
        }
    }

    /* Assume los */
    return true;
}

/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that there is no monster in the way.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 */
bool borg_projectable(int y1, int x1, int y2, int x2)
{
    int dist, y, x;

    borg_grid *ag;

    /* Start at the initial location */
    y = y1;
    x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= z_info->max_range; dist++) {
        /* Get the grid */
        ag = &borg_grids[y][x];

        if ((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 3
                || borg_morgoth_position || scaryguy_on_level)) {
            /* Assume all unknown grids more than distance 20 from you
             * are walls--when I am wounded. This will make me more fearful
             * of the grids that are up to 19 spaces away.  I treat them as
             * regular floor grids.  Which means monsters are assumed to have
             * LOS on me.  I am also more likely to shoot into the dark grids.
             */
            if ((dist > 20) && (ag->feat == FEAT_NONE))
                break;
        } else if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2) {
            /* Assume all unknow grids more than distance 10 from you
             * are walls--when I am wounded. This will make me more fearful
             * of the grids that are up to 9 spaces away.  I treat them as
             * regular floor grids.
             */
            if ((dist > 10) && (ag->feat == FEAT_NONE))
                break;
        } else if (borg_fear_region[borg.c.y / 11][borg.c.x / 11]
                   >= avoidance / 20) {
            /* If a non-LOS monster is attacking me, then it is probably has
             * LOS to me, so do not place walls on unknown grids.  This will
             *allow me the chance to attack monsters.
             *
             * This does not work if the non-LOS monster is invisible.
             * This helps in a case like this:
             *####################			1.  Player has ESP and can sense the
             *priest.
             *......@......      p			2.  Priest has cast a spell at the
             *player.
             *#############					3.  Unknown grids are between player and
             *priest
             *								4.  Borg has created regional fear from non-LOS
             *priest.
             *
             */
            if ((dist > z_info->max_range) && (ag->feat == FEAT_NONE))
                break;
        } else {
            /* Assume all unknow grids more than distance 3 from you
             * are walls.  This makes me brave and chancey.
             */
            if ((dist > 2) && (ag->feat == FEAT_NONE))
                break;
        }
        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag)))
            break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2))
            return true;

        /* Calculate the new location */
        borg_inc_motion(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return false;
}

/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that there is no monster in the way.
 * Hack -- we refuse to assume that unknown grids are floors
 * Adapted from "projectable()" in "spells1.c".
 * This is used by borg_offset()
 */
bool borg_offset_projectable(int y1, int x1, int y2, int x2)
{
    int dist, y, x;

    borg_grid *ag;

    /* Start at the initial location */
    y = y1;
    x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= z_info->max_range; dist++) {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Assume all unknown grids are walls. */
        if ((dist) && (ag->feat == FEAT_NONE))
            break;

        /* Never pass through rubble */
        if (ag->feat == FEAT_PASS_RUBBLE)
            break;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag)))
            break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2))
            return true;

        /* Calculate the new location */
        borg_inc_motion(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return false;
}

/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that monsters in the way will stop the projection
 * Hack -- we refuse to assume that unknown grids are floors
 * In fact, we assume they are walls.
 * Adapted from "projectable()" in "spells1.c".
 */
bool borg_projectable_pure(int y1, int x1, int y2, int x2)
{
    int        dist, y, x;
    borg_grid *ag;

    /* Start at the initial location */
    y = y1;
    x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= z_info->max_range; dist++) {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Hack -- assume unknown grids are walls */
        if (dist && (ag->feat == FEAT_NONE))
            break;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag)))
            break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2))
            return true;

        /* Stop at monsters */
        if (ag->kill)
            break;

        /* Calculate the new location */
        borg_inc_motion(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return false;
}

/*
 * Check the projection from (x1,y1) to (x2,y2).
 * Assume that monsters in the way will stop the projection.
 * Assume that an unknown grid is a floor grid.
 * We want at least one unknown grid.
 *
 * This routine is used mainly aiming beams of light and
 * shooting into darkness, testing the projection path.
 *
 */
bool borg_projectable_dark(int y1, int x1, int y2, int x2)
{
    int        dist, y, x;
    int        unknown = 0;
    borg_grid *ag;

    /* Start at the initial location */
    y = y1;
    x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= z_info->max_range; dist++) {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* We want at least 1 unknown grid */
        if (dist && (ag->feat == FEAT_NONE))
            unknown++;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag)))
            break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2) && unknown >= 1)
            return true;

        /* Stop at monsters */
        if (ag->kill)
            break;

        /* Calculate the new location */
        borg_inc_motion(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return false;
}

/*
 * Calculate "incremental motion". Used by project() and shoot().
 * Assumes that (*y,*x) lies on the path from (y1,x1) to (y2,x2).
 */
/* changing this to be more like project_path */
/* note that this is much slower but much more accurate */
void borg_inc_motion(int *py, int *px, int y1, int x1, int y2, int x2)
{
    int dy, dx;
    int sy, sx;
    int y, x;

    /* Scale factors */
    int full, half;

    /* Fractions */
    int frac;

    /* Slope */
    int m;

    /* Distance */
    int k = 0;

    /* Extract the distance travelled */
    /* Analyze "dy" */
    if (y2 < y1) {
        dy = (y1 - y2);
        sy = -1;
    } else {
        dy = (y2 - y1);
        sy = 1;
    }

    /* Analyze "dx" */
    if (x2 < x1) {
        dx = (x1 - x2);
        sx = -1;
    } else {
        dx = (x2 - x1);
        sx = 1;
    }

    /* Paranoia -- Hack -- no motion */
    if (!dy && !dx)
        return;

    /* Number of "units" in one "half" grid */
    half = (dy * dx);

    /* Number of "units" in one "full" grid */
    full = half << 1;

    /* First step is fixed */
    if (*px == x1 && *py == y1) {
        if (dy > dx) {
            *py += sy;
            return;
        } else if (dx > dy) {
            *px += sx;
            return;
        } else {
            *px += sx;
            *py += sy;
            return;
        }
    }

    /* Move mostly vertically */
    if (dy > dx) {
        k = dy;

        /* Start at tile edge */
        frac = dx * dx;

        /* Let m = ((dx/dy) * full) = (dx * dx * 2) = (frac * 2) */
        m = frac << 1;

        /* Start */
        y = y1 + sy;
        x = x1;

        /* Create the projection path */
        while (1) {
            if (x == *px && y == *py)
                k = 1;

            /* Slant */
            if (m) {
                /* Advance (X) part 1 */
                frac += m;

                /* Horizontal change */
                if (frac >= half) {
                    /* Advance (X) part 2 */
                    x += sx;

                    /* Advance (X) part 3 */
                    frac -= full;
                }
            }

            /* Advance (Y) */
            y += sy;

            /* Track distance */
            k--;

            if (!k) {
                *px = x;
                *py = y;
                return;
            }
        }
    }
    /* Move mostly horizontally */
    else if (dx > dy) {
        /* Start at tile edge */
        frac = dy * dy;

        /* Let m = ((dy/dx) * full) = (dy * dy * 2) = (frac * 2) */
        m = frac << 1;

        /* Start */
        y = y1;
        x = x1 + sx;
        k = dx;

        /* Create the projection path */
        while (1) {
            if (x == *px && y == *py)
                k = 1;

            /* Slant */
            if (m) {
                /* Advance (Y) part 1 */
                frac += m;

                /* Vertical change */
                if (frac >= half) {
                    /* Advance (Y) part 2 */
                    y += sy;

                    /* Advance (Y) part 3 */
                    frac -= full;
                }
            }

            /* Advance (X) */
            x += sx;

            /* Track distance */
            k--;

            if (!k) {
                *px = x;
                *py = y;
                return;
            }
        }
    }
    /* Diagonal */
    else {
        /* Start */
        k = dy;
        y = y1 + sy;
        x = x1 + sx;

        /* Create the projection path */
        while (1) {
            if (x == *px && y == *py)
                k = 1;

            /* Advance (Y) */
            y += sy;

            /* Advance (X) */
            x += sx;

            /* Track distance */
            k--;

            if (!k) {
                *px = x;
                *py = y;
                return;
            }
        }
    }
}

/*
 * Helper to change the old y/x/y2/x2 to loc/loc
 *  could do with a macro but that gets ugly.
 */
int borg_distance(int y, int x, int y2, int x2)
{
    return distance(loc(x, y), loc(x2, y2));
}

/*
 * Target a location.  Can be used alone or at "Direction?" prompt.
 *
 * Warning -- This will only work for locations on the current panel
 */
bool borg_target(struct loc t)
{
    int x1, y1, x2, y2;

    borg_grid *ag;
    borg_kill *kill;

    ag   = &borg_grids[t.y][t.x];
    kill = &borg_kills[ag->kill];

    /* Log */
    /* Report a little bit */
    if (ag->kill) {
        borg_note(format("# Targeting %s who has %d Hit Points (%d,%d).",
            (r_info[kill->r_idx].name), kill->power, t.y, t.x));
    } else {
        borg_note(format("# Targetting location (%d,%d)", t.y, t.x));
    }

    /* Target mode */
    borg_keypress('*');

    /* Target a location */
    borg_keypress('p');

    /* Determine "path" */
    x1 = borg.c.x;
    y1 = borg.c.y;
    x2 = t.x;
    y2 = t.y;

    /* Move to the location (diagonals) */
    for (; (y1 < y2) && (x1 < x2); y1++, x1++)
        borg_keypress('3');
    for (; (y1 < y2) && (x1 > x2); y1++, x1--)
        borg_keypress('1');
    for (; (y1 > y2) && (x1 < x2); y1--, x1++)
        borg_keypress('9');
    for (; (y1 > y2) && (x1 > x2); y1--, x1--)
        borg_keypress('7');

    /* Move to the location */
    for (; y1 < y2; y1++)
        borg_keypress('2');
    for (; y1 > y2; y1--)
        borg_keypress('8');
    for (; x1 < x2; x1++)
        borg_keypress('6');
    for (; x1 > x2; x1--)
        borg_keypress('4');

    /* Select the target */
    borg_keypress('5');

    /* Carry these variables to be used on reporting spell
     * pathway
     */
    borg_target_loc.y = t.y;
    borg_target_loc.x = t.x;

    /* Success */
    return true;
}

/*
 * Mark spot along the target path a wall.
 * This will mark the unknown squares as a wall.  This might not be
 * the wall we ran into but also might be.
 *
 * Warning -- This will only work for locations on the current panel
 */
bool borg_target_unknown_wall(int y, int x)
{
    int  n_x, n_y;
    bool found  = false;
    bool y_hall = false;
    bool x_hall = false;

    borg_grid           *ag;
    struct monster_race *r_ptr;
    borg_kill           *kill;

    borg_note(format("# Perhaps wall near targetted location (%d,%d)", y, x));

    /* Determine "path" */
    n_x = borg.c.x;
    n_y = borg.c.y;

    /* check for 'in a hall' x axis */
    /* This check is for this: */
    /*
     *      x        x
     *    ..@.  or  .@..
     *      x        x
     *
     * 'x' being 'not a floor' and '.' being a floor.
     *
     * We would like to know if in a hall so we can place
     * the suspect wall off the hallway path.
     * like this:######x  P
     * ........@....
     * ##################
     * The shot may miss and we want the borg to guess the
     * wall to be at the X instead of first unkown grid which
     * is 3 west and 1 south of the X.
     */

    if ((borg_grids[borg.c.y + 1][borg.c.x].feat == FEAT_FLOOR
            && borg_grids[borg.c.y - 1][borg.c.x].feat == FEAT_FLOOR
            && (borg_grids[borg.c.y + 2][borg.c.x].feat == FEAT_FLOOR
                || borg_grids[borg.c.y - 2][borg.c.x].feat == FEAT_FLOOR))
        && (borg_grids[borg.c.y][borg.c.x + 1].feat != FEAT_FLOOR
            && borg_grids[borg.c.y][borg.c.x - 1].feat != FEAT_FLOOR))
        x_hall = true;

    /* check for 'in a hall' y axis.
     * Again, we want to place the suspected wall off our
     * hallway.
     */
    if ((borg_grids[borg.c.y][borg.c.x + 1].feat == FEAT_FLOOR
            && borg_grids[borg.c.y][borg.c.x - 1].feat == FEAT_FLOOR
            && (borg_grids[borg.c.y][borg.c.x + 2].feat == FEAT_FLOOR
                || borg_grids[borg.c.y][borg.c.x - 2].feat == FEAT_FLOOR))
        && (borg_grids[borg.c.y + 1][borg.c.x].feat != FEAT_FLOOR
            && borg_grids[borg.c.y - 1][borg.c.x].feat != FEAT_FLOOR))
        y_hall = true;

    while (1) {
        ag    = &borg_grids[n_y][n_x];
        kill  = &borg_kills[ag->kill];
        r_ptr = &r_info[kill->r_idx];

        if (rf_has(r_ptr->flags, RF_PASS_WALL)) {
            borg_note(
                format("# Guessing wall (%d,%d) under ghostly target (%d,%d)",
                    n_y, n_x, n_y, n_x));
            borg_grids[n_y][n_x].feat = FEAT_GRANITE;
            found                     = true;
            return (found); /* not sure... should we return here? */
        }

        if (borg_grids[n_y][n_x].feat == FEAT_NONE
            && ((n_y != borg.c.y) || !y_hall)
            && ((n_x != borg.c.x) || !x_hall)) {
            borg_note(format(
                "# Guessing wall (%d,%d) near target (%d,%d)", n_y, n_x, y, x));
            borg_grids[n_y][n_x].feat = FEAT_GRANITE;
            found                     = true;
            return (found); /* not sure... should we return here?
                             maybe should mark ALL unknowns in path... */
        }

        /* Pathway found the target. */
        if (n_x == x && n_y == y) {
            /* end of the pathway */
            borg_inc_motion(&n_y, &n_x, y, x, borg.c.y, borg.c.x);
            borg_note(format(
                "# Guessing wall (%d,%d) near target (%d,%d)", n_y, n_x, y, x));
            borg_grids[n_y][n_x].feat = FEAT_GRANITE;
            found                     = true;
            return (found);
        }

        /* Calculate the new location */
        borg_inc_motion(&n_y, &n_x, borg.c.y, borg.c.x, y, x);
    }
}

#endif
