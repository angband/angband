/* File: borg2.c */
/* Purpose: Low level dungeon mapping skills -BEN- */

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"


#ifdef ALLOW_BORG

#include "borg1.h"
#include "borg2.h"


/*
 * This file helps the Borg understand mapping the dungeon.
 *
 * Currently, this includes general routines involving dungeon grids,
 * including calculating "flow" values from place to place, determining
 * "line of sight", plus "field of view" and "torch-lit grids", setting
 * the target to a given location, and extracting the optimal direction
 * for "movement" from place to place.
 *
 * Note that the dungeon is assumed smaller than 256 by 256.
 *
 * This file also supplies the (compiled out) support for "automatic
 * room extraction".  This code will automatically group regions of
 * the dungeon into rooms, and do the "flow" navigation on those rooms
 * instead of on grids.  Often, this takes less space, and is faster,
 * howver, it is more complicated, and does not allow "specialized"
 * flow calculations that penalize grids by variable amounts.
 */

/* Is this grid a grid which can be stepped on or can I see through it */
bool borg_cave_floor_bold(int Y, int X)
{
	if (in_bounds_fully(Y,X))
	{
		if ((borg_grids[Y][X].feat == FEAT_FLOOR) ||
			(borg_grids[Y][X].feat >= FEAT_TRAP_HEAD && borg_grids[Y][X].feat <= FEAT_TRAP_TAIL) ||
			(borg_grids[Y][X].feat == FEAT_LESS) ||
			(borg_grids[Y][X].feat == FEAT_MORE) ||
			(borg_grids[Y][X].feat == FEAT_BROKEN) ||
			(borg_grids[Y][X].feat == FEAT_OPEN) ||
			(borg_grids[Y][X].feat == FEAT_GLYPH)) return (TRUE);
	}
	return (FALSE);
}

/*
 * A simple, fast, integer-based line-of-sight algorithm.
 *
 * See "los()" in "cave.c" for complete documentation
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
    if ((ax < 2) && (ay < 2)) return (TRUE);


    /* Paranoia -- require "safe" origin */
    if (!in_bounds_fully(y1, x1)) return (FALSE);


    /* Directly South/North */
    if (!dx)
    {
        /* South -- check for walls */
        if (dy > 0)
        {
            for (ty = y1 + 1; ty < y2; ty++)
            {
                if (!borg_cave_floor_bold(ty, x1)) return (FALSE);
            }
        }

        /* North -- check for walls */
        else
        {
            for (ty = y1 - 1; ty > y2; ty--)
            {
                if (!borg_cave_floor_bold(ty, x1)) return (FALSE);
            }
        }

        /* Assume los */
        return (TRUE);
    }

    /* Directly East/West */
    if (!dy)
    {
        /* East -- check for walls */
        if (dx > 0)
        {
            for (tx = x1 + 1; tx < x2; tx++)
            {
                if (!borg_cave_floor_bold(y1, tx)) return (FALSE);
            }
        }

        /* West -- check for walls */
        else
        {
            for (tx = x1 - 1; tx > x2; tx--)
            {
                if (!borg_cave_floor_bold(y1, tx)) return (FALSE);
            }
        }

        /* Assume los */
        return (TRUE);
    }


    /* Extract some signs */
    sx = (dx < 0) ? -1 : 1;
    sy = (dy < 0) ? -1 : 1;


    /* Vertical "knights" */
    if (ax == 1)
    {
        if (ay == 2)
        {
            if (borg_cave_floor_bold(y1 + sy, x1)) return (TRUE);
        }
    }

    /* Horizontal "knights" */
    else if (ay == 1)
    {
        if (ax == 2)
        {
            if (borg_cave_floor_bold(y1, x1 + sx)) return (TRUE);
        }
    }


    /* Calculate scale factor div 2 */
    f2 = (ax * ay);

    /* Calculate scale factor */
    f1 = f2 << 1;


    /* Travel horizontally */
    if (ax >= ay)
    {
        /* Let m = dy / dx * 2 * (dy * dx) = 2 * dy * dy */
        qy = ay * ay;
        m = qy << 1;

        tx = x1 + sx;

        /* Consider the special case where slope == 1. */
        if (qy == f2)
        {
            ty = y1 + sy;
            qy -= f1;
        }
        else
        {
            ty = y1;
        }

        /* Note (below) the case (qy == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (x2 - tx)
        {
            if (!borg_cave_floor_bold(ty, tx)) return (FALSE);

            qy += m;

            if (qy < f2)
            {
                tx += sx;
            }
            else if (qy > f2)
            {
                ty += sy;
                if (!borg_cave_floor_bold(ty, tx)) return (FALSE);
                qy -= f1;
                tx += sx;
            }
            else
            {
                ty += sy;
                qy -= f1;
                tx += sx;
            }
        }
    }

    /* Travel vertically */
    else
    {
        /* Let m = dx / dy * 2 * (dx * dy) = 2 * dx * dx */
        qx = ax * ax;
        m = qx << 1;

        ty = y1 + sy;

        if (qx == f2)
        {
            tx = x1 + sx;
            qx -= f1;
        }
        else
        {
            tx = x1;
        }

        /* Note (below) the case (qx == f2), where */
        /* the LOS exactly meets the corner of a tile. */
        while (y2 - ty)
        {
            if (!borg_cave_floor_bold(ty, tx)) return (FALSE);

            qx += m;

            if (qx < f2)
            {
                ty += sy;
            }
            else if (qx > f2)
            {
                tx += sx;
                if (!borg_cave_floor_bold(ty, tx)) return (FALSE);
                qx -= f1;
                ty += sy;
            }
            else
            {
                tx += sx;
                qx -= f1;
                ty += sy;
            }
        }
    }

    /* Assume los */
    return (TRUE);


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
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

        if ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3 ||
            borg_morgoth_position || scaryguy_on_level))
        {
            /* Assume all unknown grids more than distance 20 from you
             * are walls--when I am wounded. This will make me more fearful
             * of the grids that are up to 19 spaces away.  I treat them as
             * regular floor grids.  Which means monsters are assumed to have
             * LOS on me.  I am also more likely to shoot into the dark grids.
             */
            if ((dist > 20) && (ag->feat == FEAT_NONE)) break;
        }
        else if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2)
        {
            /* Assume all unknow grids more than distance 10 from you
             * are walls--when I am wounded. This will make me more fearful
             * of the grids that are up to 9 spaces away.  I treat them as
             * regular floor grids.
             */
            if ((dist > 10) && (ag->feat == FEAT_NONE)) break;
        }
		else if (borg_fear_region[c_y/11][c_x/11] >= avoidance / 20)
		{
			/* If a non-LOS monster is attacking me, then it is probably has
			 * LOS to me, so do not place walls on unknown grids.  This will allow
			 * me the chance to attack monsters.
			 *
			 * This does not work if the non-LOS monster is invisible.
			 * This helps in a case like this:
			 *####################			1.  Player has ESP and can sense the priest.
			 *......@......      p			2.  Priest has cast a spell at the player.
			 *#############					3.  Unknown grids are between player and priest
			 *								4.  Borg has created regional fear from non-LOS priest.
			 *
			 */
            if ((dist > MAX_RANGE) && (ag->feat == FEAT_NONE)) break;
		}
        else
        {
            /* Assume all unknow grids more than distance 3 from you
             * are walls.  This makes me brave and chancey.
             */
            if ((dist > 2) && (ag->feat == FEAT_NONE)) break;
        }
        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag))) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
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
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Assume all unknown grids are walls. */
        if ((dist) && (ag->feat == FEAT_NONE)) break;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag))) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
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
    int dist, y, x;
    borg_grid *ag;

    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Hack -- assume unknown grids are walls */
        if (dist && (ag->feat == FEAT_NONE)) break;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag))) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2)) return (TRUE);

        /* Stop at monsters */
        if (ag->kill) break;

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
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
    int dist, y, x;
    int unknown = 0;
    borg_grid *ag;

    /* Start at the initial location */
    y = y1; x = x1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist <= MAX_RANGE; dist++)
    {
        /* Get the grid */
        ag = &borg_grids[y][x];

        /* We want at least 1 unknown grid */
        if (dist && (ag->feat == FEAT_NONE)) unknown ++;

        /* Never pass through walls/doors */
        if (dist && (!borg_cave_floor_grid(ag))) break;

        /* Check for arrival at "final target" */
        if ((x == x2) && (y == y2) && unknown >= 1) return (TRUE);

        /* Stop at monsters */
        if (ag->kill) break;

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Assume obstruction */
    return (FALSE);
}



/*
 * Clear the lite grids
 */
void borg_forget_LIGHT(void)
{
    int i;

    /* None to forget */
    if (!borg_LIGHT_n) return;

    /* Clear them all */
    for (i = 0; i < borg_LIGHT_n; i++)
    {
        int y = borg_LIGHT_y[i];
        int x = borg_LIGHT_x[i];

        /* Forget that the grid is lit */
        borg_grids[y][x].info &= ~BORG_LIGHT;
    }

    /* None left */
    borg_LIGHT_n = 0;
}



/*
 * XXX XXX XXX
 *
 * This macro allows us to efficiently add a grid to the "lite" array,
 * note that we are never called for illegal grids, or for grids which
 * have already been placed into the "lite" array, and we are never
 * called when the "lite" array is full.
 */
#define borg_cave_LIGHT_hack(Y,X) \
    borg_grids[Y][X].info |= BORG_LIGHT; \
    borg_LIGHT_y[borg_LIGHT_n] = (Y); \
    borg_LIGHT_x[borg_LIGHT_n] = (X); \
    borg_LIGHT_n++



/*
 * Update the set of grids "illuminated" by the player's lite.
 *
 * See "update_LIGHT" in "cave.c" for complete documentation
 *
 * It is very important that the "player grid" be the first grid in the
 * array of "BORG_LIGHT" grids, since this is assumed in several places.
 */
void borg_update_LIGHT(void)
{
    int i, x, y, min_x, max_x, min_y, max_y;


    /*** Clear old grids ***/

    /* Clear them all */
    for (i = 0; i < borg_LIGHT_n; i++)
    {
        y = borg_LIGHT_y[i];
        x = borg_LIGHT_x[i];

        /* Mark the grid as not "lite" */
        borg_grids[y][x].info &= ~BORG_LIGHT;
    }

    /* None left */
    borg_LIGHT_n = 0;

    /* Hack -- Player has no lite */
    if (borg_skill[BI_CURLITE] <= 0) return;


    /*** Collect the new "lite" grids ***/

    /* Player grid */
    borg_cave_LIGHT_hack(c_y, c_x);

    /* Radius 1 -- torch radius */
    if (borg_skill[BI_CURLITE] >= 1)
    {
        /* Adjacent grid */
        borg_cave_LIGHT_hack(c_y+1, c_x);
        borg_cave_LIGHT_hack(c_y-1, c_x);
        borg_cave_LIGHT_hack(c_y, c_x+1);
        borg_cave_LIGHT_hack(c_y, c_x-1);

        /* Diagonal grids */
        borg_cave_LIGHT_hack(c_y+1, c_x+1);
        borg_cave_LIGHT_hack(c_y+1, c_x-1);
        borg_cave_LIGHT_hack(c_y-1, c_x+1);
        borg_cave_LIGHT_hack(c_y-1, c_x-1);
    }

    /* Radius 2 -- lantern radius */
    if (borg_skill[BI_CURLITE] >= 2 &&
		c_y + 2 < AUTO_MAX_Y &&
		c_y - 2 > 0 &&
		c_x + 2 < AUTO_MAX_X &&
		c_x - 2 > 0)
    {
		/* South of the player */
        if (borg_cave_floor_bold(c_y+2, c_x))
        {
            borg_cave_LIGHT_hack(c_y+2, c_x);
            borg_cave_LIGHT_hack(c_y+2, c_x+2);
            borg_cave_LIGHT_hack(c_y+2, c_x-2);
        }

        /* North of the player */
        if (borg_cave_floor_bold(c_y-2, c_x))
        {
            borg_cave_LIGHT_hack(c_y-2, c_x);
            borg_cave_LIGHT_hack(c_y-2, c_x+2);
            borg_cave_LIGHT_hack(c_y-2, c_x-2);
        }

        /* East of the player */
        if (borg_cave_floor_bold(c_y, c_x+2))
        {
            borg_cave_LIGHT_hack(c_y, c_x+2);
            borg_cave_LIGHT_hack(c_y+1, c_x+2);
            borg_cave_LIGHT_hack(c_y-1, c_x+2);
        }

        /* West of the player */
        if (borg_cave_floor_bold(c_y, c_x-2))
        {
            borg_cave_LIGHT_hack(c_y, c_x-2);
            borg_cave_LIGHT_hack(c_y+2, c_x-2);
            borg_cave_LIGHT_hack(c_y-2, c_x-2);
        }
    }

    /* Radius 3+ -- artifact radius */
    if (borg_skill[BI_CURLITE] >= 3 &&
		c_y + 3 < AUTO_MAX_Y &&
		c_y - 3 > 0 &&
		c_x + 3 < AUTO_MAX_X &&
		c_x - 3 > 0)
    {
        int d, p;

        /* Maximal radius */
        p = borg_skill[BI_CURLITE];

        /* Paranoia -- see "LITE_MAX" */
        if (p > 5) p = 5;

        /* South-East of the player */
        if (borg_cave_floor_bold(c_y+3, c_x+3))
        {
            borg_cave_LIGHT_hack(c_y+3, c_x+3);
        }

        /* South-West of the player */
        if (borg_cave_floor_bold(c_y+3, c_x-3))
        {
            borg_cave_LIGHT_hack(c_y+3, c_x-3);
        }

        /* North-East of the player */
        if (borg_cave_floor_bold(c_y-3, c_x+3))
        {
            borg_cave_LIGHT_hack(c_y-3, c_x+3);
        }

        /* North-West of the player */
        if (borg_cave_floor_bold(c_y-3, c_x-3))
        {
            borg_cave_LIGHT_hack(c_y-3, c_x-3);
        }

        /* Maximal north */
        min_y = c_y - p;
        if (min_y < 0) min_y = 0;

        /* Maximal south */
        max_y = c_y + p;
        if (max_y > AUTO_MAX_Y-1) max_y = AUTO_MAX_Y-1;

        /* Maximal west */
        min_x = c_x - p;
        if (min_x < 0) min_x = 0;

        /* Maximal east */
        max_x = c_x + p;
        if (max_x > AUTO_MAX_X-1) max_x = AUTO_MAX_X-1;

        /* Scan the maximal box */
        for (y = min_y; y <= max_y; y++)
        {
            for (x = min_x; x <= max_x; x++)
            {
                int dy = (c_y > y) ? (c_y - y) : (y - c_y);
                int dx = (c_x > x) ? (c_x - x) : (x - c_x);

                /* Skip the "central" grids (above) */
                if ((dy <= 2) && (dx <= 2)) continue;

                /* Hack -- approximate the distance */
                d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

                /* Skip distant grids */
                if (d > p) continue;

                /* Viewable, nearby, grids get "torch lit" */
                if (borg_grids[y][x].info & BORG_VIEW)
                {
                    /* This grid is "torch lit" */
                    borg_cave_LIGHT_hack(y, x);
                }
            }
        }
    }
}





/*
 * Clear the viewable space
 */
void borg_forget_view(void)
{
    int i;

    borg_grid *ag;

    /* None to forget */
    if (!borg_view_n) return;

    /* Clear them all */
    for (i = 0; i < borg_view_n; i++)
    {
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
#define borg_cave_view_hack(A,Y,X) \
    (A)->info |= BORG_VIEW; \
    borg_view_y[borg_view_n] = (Y); \
    borg_view_x[borg_view_n] = (X); \
    borg_view_n++



/*
 * Helper function for "borg_update_view()" below
 *
 * See "update_view_aux()" in "cave.c" for complete documentation.
 */
static bool borg_update_view_aux(int y, int x, int y1, int x1, int y2, int x2)
{
    bool f1, f2, v1, v2, z1, z2, wall;

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
    if (!f1 && !f2) return (TRUE);


    /* Check for visibility */
    v1 = (f1 && (g1_ag->info & BORG_VIEW));
    v2 = (f2 && (g2_ag->info & BORG_VIEW));

    /* Totally blocked by "unviewable neighbors" */
    if (!v1 && !v2) return (TRUE);


    /* Access the grid */
    ag = &borg_grids[y][x];


    /* Check for walls */
    wall = (!borg_cave_floor_grid(ag));


    /* Check the "ease" of visibility */
    z1 = (v1 && (g1_ag->info & BORG_XTRA));
    z2 = (v2 && (g2_ag->info & BORG_XTRA));

    /* Hack -- "easy" plus "easy" yields "easy" */
    if (z1 && z2)
    {
        ag->info |= BORG_XTRA;

        borg_cave_view_hack(ag, y, x);

        return (wall);
    }

    /* Hack -- primary "easy" yields "viewed" */
    if (z1)
    {
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Hack -- "view" plus "view" yields "view" */
    if (v1 && v2)
    {
        /* ag->info |= BORG_XTRA; */

        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Mega-Hack -- the "borg_los()" function works poorly on walls */
    if (wall)
    {
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Hack -- check line of sight */
    if (borg_los(c_y, c_x, y, x))
    {
        borg_cave_view_hack(ag, y, x);

        return (wall);
    }


    /* Assume no line of sight. */
    return (TRUE);
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
    full = MAX_SIGHT;

    /* Octagon factor (30) */
    over = MAX_SIGHT * 3 / 2;


    /*** Step 0 -- Begin ***/

    /* Save the old "view" grids for later */
    for (n = 0; n < borg_view_n; n++)
    {
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
    y = c_y;
    x = c_x;

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
    for (d = 1; d <= z; d++)
    {
		if (!in_bounds_fully(y+d, x+d)) continue;

        ag = &borg_grids[y+d][x+d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y+d, x+d);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Scan south-west */
    for (d = 1; d <= z; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y+d, x-d)) continue;

		ag = &borg_grids[y+d][x-d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y+d, x-d);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Scan north-east */
    for (d = 1; d <= z; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y-d, x+d)) continue;

		ag = &borg_grids[y-d][x+d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y-d, x+d);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Scan north-west */
    for (d = 1; d <= z; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y-d, x-d)) continue;

		ag = &borg_grids[y-d][x-d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y-d, x-d);
        if (!borg_cave_floor_grid(ag)) break;
    }


    /*** Step 3 -- major axes ***/

    /* Scan south */
    for (d = 1; d <= full; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y+d, x)) continue;
        ag = &borg_grids[y+d][x];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y+d, x);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Initialize the "south strips" */
    se = sw = d;

    /* Scan north */
    for (d = 1; d <= full; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y-d, x)) continue;

		ag = &borg_grids[y-d][x];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y-d, x);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Initialize the "north strips" */
    ne = nw = d;

    /* Scan east */
    for (d = 1; d <= full; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y, x+d)) continue;
        ag = &borg_grids[y][x+d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x+d);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Initialize the "east strips" */
    es = en = d;

    /* Scan west */
    for (d = 1; d <= full; d++)
    {
		/* Caution */
		if (!in_bounds_fully(y, x-d)) continue;

		ag = &borg_grids[y][x-d];
        ag->info |= BORG_XTRA;
        borg_cave_view_hack(ag, y, x-d);
        if (!borg_cave_floor_grid(ag)) break;
    }

    /* Initialize the "west strips" */
    ws = wn = d;


    /*** Step 4 -- Divide each "octant" into "strips" ***/

    /* Now check each "diagonal" (in parallel) */
    for (n = 1; n <= over / 2; n++)
    {
        int ypn, ymn, xpn, xmn;


        /* Acquire the "bounds" of the maximal circle */
        z = over - n - n;
        if (z > full - n) z = full - n;
        while ((z + n + (n>>1)) > full) z--;


        /* Access the four diagonal grids */
        ypn = y + n;
        ymn = y - n;
        xpn = x + n;
        xmn = x - n;


        /* South strip */
        if (ypn < AUTO_MAX_Y-1)
        {
            /* Maximum distance */
            m = MIN(z, (AUTO_MAX_Y-1) - ypn);

            /* East side */
            if ((xpn <= AUTO_MAX_X-1) && (n < se))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn+d, xpn, ypn+d-1, xpn-1, ypn+d-1, xpn))
                    {
                        if (n + d >= se) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                se = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < sw))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn+d, xmn, ypn+d-1, xmn+1, ypn+d-1, xmn))
                    {
                        if (n + d >= sw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                sw = k + 1;
            }
        }


        /* North strip */
        if (ymn > 0)
        {
            /* Maximum distance */
            m = MIN(z, ymn);

            /* East side */
            if ((xpn <= AUTO_MAX_X-1) && (n < ne))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn-d, xpn, ymn-d+1, xpn-1, ymn-d+1, xpn))
                    {
                        if (n + d >= ne) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ne = k + 1;
            }

            /* West side */
            if ((xmn >= 0) && (n < nw))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn-d, xmn, ymn-d+1, xmn+1, ymn-d+1, xmn))
                    {
                        if (n + d >= nw) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                nw = k + 1;
            }
        }


        /* East strip */
        if (xpn < AUTO_MAX_X-1)
        {
            /* Maximum distance */
            m = MIN(z, (AUTO_MAX_X-1) - xpn);

            /* South side */
            if ((ypn <= AUTO_MAX_Y-1) && (n < es))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn, xpn+d, ypn-1, xpn+d-1, ypn, xpn+d-1))
                    {
                        if (n + d >= es) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                es = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < en))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn, xpn+d, ymn+1, xpn+d-1, ymn, xpn+d-1))
                    {
                        if (n + d >= en) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                en = k + 1;
            }
        }


        /* West strip */
        if (xmn > 0)
        {
            /* Maximum distance */
            m = MIN(z, xmn);

            /* South side */
            if ((ypn <= AUTO_MAX_Y-1) && (n < ws))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ypn, xmn-d, ypn-1, xmn-d+1, ypn, xmn-d+1))
                    {
                        if (n + d >= ws) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
                        k = n + d;
                    }
                }

                /* Limit the next strip */
                ws = k + 1;
            }

            /* North side */
            if ((ymn >= 0) && (n < wn))
            {
                /* Scan */
                for (k = n, d = 1; d <= m; d++)
                {
                    /* Check grid "d" in strip "n", notice "blockage" */
                    if (borg_update_view_aux(ymn, xmn-d, ymn+1, xmn-d+1, ymn, xmn-d+1))
                    {
                        if (n + d >= wn) break;
                    }

                    /* Track most distant "non-blockage" */
                    else
                    {
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
    for (n = 0; n < borg_view_n; n++)
    {
        y = borg_view_y[n];
        x = borg_view_x[n];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Clear the "BORG_XTRA" flag */
        ag->info &= ~BORG_XTRA;
    }
}





/*
 * Init this file.
 */
void borg_init_2(void)
{
    /* Nothing */
}



#else

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif
