/**
 * \file borg-flow-dark.c
 * \brief Flow (move) toward interesting (dark) grids
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

#include "borg-flow-dark.h"

#ifdef ALLOW_BORG

#include "../cave.h"

#include "borg-cave-light.h"
#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-danger.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Determine if a grid is "interesting" (and should be explored)
 *
 * A grid is "interesting" if it is a closed door, rubble, hidden treasure,
 * or a visible trap, or an "unknown" grid.
 * or a non-perma-wall adjacent to a perma-wall. (GCV)
 *
 * b_stair is the index to the closest upstairs.
 */
static bool borg_flow_dark_interesting(int y, int x, int b_stair)
{
    int oy;
    int ox, i;

    borg_grid *ag;

    /* Get the borg_grid */
    ag = &borg_grids[y][x];

    /* Explore unknown grids */
    if (ag->feat == FEAT_NONE)
        return true;

    /* Efficiency -- Ignore "boring" grids */
    if (ag->feat < FEAT_SECRET && ag->feat != FEAT_CLOSED)
        return false;

    /* Explore "known treasure" */
    if ((ag->feat == FEAT_MAGMA_K) || (ag->feat == FEAT_QUARTZ_K)) {
        /* Do not dig when confused */
        if (borg.trait[BI_ISCONFUSED])
            return false;

        /* Do not bother if super rich */
        if (borg.trait[BI_GOLD] >= 100000)
            return false;

        /* Not when darkened */
        if (borg.trait[BI_CURLITE] == 0)
            return false;

        /* don't try to dig if we can't */
        if (!borg_can_dig(false, ag->feat))
            return false;

        /* Okay */
        return true;
    }

    /* "Vaults" Explore non perma-walls adjacent to a perma wall */
    if (ag->feat == FEAT_GRANITE || ag->feat == FEAT_MAGMA
        || ag->feat == FEAT_QUARTZ) {

        /* Do not attempt when confused */
        if (borg.trait[BI_ISCONFUSED])
            return false;

        /* hack and cheat.  No vaults  on this level */
        if (!vault_on_level)
            return false;

        /* make sure we can dig */
        if (!borg_can_dig(false, ag->feat))
            return false;

        /* Do not attempt on the edge */
        if (x < AUTO_MAX_X - 1 && y < AUTO_MAX_Y - 1 && x > 1 && y > 1) {
            /* scan the adjacent grids */
            for (ox = -1; ox <= 1; ox++) {
                for (oy = -1; oy <= 1; oy++) {

                    /* Acquire location */
                    ag = &borg_grids[oy + y][ox + x];

                    /* skip non perma grids wall */
                    if (ag->feat != FEAT_PERM)
                        continue;

                    /* at least one perm wall next to this, dig it out */
                    return true;
                }
            }
        }

        /* not adjacent to a GCV,  Restore Grid */
        ag = &borg_grids[y][x];
    }

    /* Explore "rubble" */
    if (ag->feat == FEAT_RUBBLE && !borg.trait[BI_ISWEAK]) {
        return true;
    }

    /* Explore "closed doors" */
    if (ag->feat == FEAT_CLOSED) {
        /* some closed doors leave alone */
        if (breeder_level) {
            /* Did I close this one */
            for (i = 0; i < track_door.num; i++) {
                /* mark as icky if I closed this one */
                if ((track_door.x[i] == x) && (track_door.y[i] == y)) {
                    /* not interesting */
                    return false;
                }
            }
        }
        /* this door should be ok to open */
        return true;
    }

    /* Explore "visible traps" */
    if (feat_is_trap_holding(ag->feat)) {
        /* Do not disarm when blind */
        if (borg.trait[BI_ISBLIND])
            return false;

        /* Do not disarm when confused */
        if (borg.trait[BI_ISCONFUSED])
            return false;

        /* Do not disarm when hallucinating */
        if (borg.trait[BI_ISIMAGE])
            return false;

        /* Do not flow without lite */
        if (borg.trait[BI_CURLITE] == 0)
            return false;

        /* Do not disarm trap doors on level 99 */
        if (borg.trait[BI_CDEPTH] == 99 && ag->trap && !ag->glyph)
            return false;

        /* Do not disarm when you could end up dead */
        if (borg.trait[BI_CURHP] < 60)
            return false;

        /* Do not disarm when clumsy */
        if (borg.trait[BI_DISP] < 30 && borg.trait[BI_CLEVEL] < 20)
            return false;
        if (borg.trait[BI_DISP] < 45 && borg.trait[BI_CLEVEL] < 10)
            return false;
        if (borg.trait[BI_DISM] < 30 && borg.trait[BI_CLEVEL] < 20)
            return false;
        if (borg.trait[BI_DISM] < 45 && borg.trait[BI_CLEVEL] < 10)
            return false;

        /* Do not explore if a Scaryguy on the Level */
        if (scaryguy_on_level)
            return false;

        /* NOTE: the flow code allows a borg to flow through a trap and so he
         * may still try to disarm one on his way to the other interesting grid.
         * If mods are made to the above criteria for disarming traps, then mods
         * must also be made to borg_flow_spread() and borg_flow_direct()
         */

        /* Okay */
        return true;
    }

    /* Ignore other grids */
    return false;
}

/*
 * Determine if a grid is "reachable" (and can be explored)
 */
static bool borg_flow_dark_reachable(int y, int x)
{
    int j;

    borg_grid *ag;

    /* Scan neighbors */
    for (j = 0; j < 8; j++) {
        int y2 = y + ddy_ddd[j];
        int x2 = x + ddx_ddd[j];

        /* Get the grid */
        ag = &borg_grids[y2][x2];

        /* Skip unknown grids (important) */
        if (ag->feat == FEAT_NONE)
            continue;

        /* Accept known floor grids */
        if (borg_cave_floor_grid(ag))
            return true;
    }

    /* Failure */
    return false;
}

/*
 * Place a "direct path" into the flow array, checking danger
 *
 * Modify the "cost" array in such a way that from any point on
 * one "direct" path from the player to the given grid, as long
 * as the rest of the path is "safe" and "clear", the Borg will
 * walk along the path to the given grid.
 *
 * This function is used by "borg_flow_dark_1()" to provide an
 * optimized "flow" during the initial exploration of a level.
 * It is also used by "borg_flow_dark_2()" in a similar fashion.
 */
static void borg_flow_direct(int y, int x)
{
    int n = 0;

    int x1, y1, x2, y2;

    int ay, ax;

    int shift;

    int p, fear = 0;

    borg_grid *ag;

    /* Avoid icky grids */
    if (borg_data_icky->data[y][x])
        return;

    /* Unknown */
    if (!borg_data_know->data[y][x]) {
        /* Mark as known */
        borg_data_know->data[y][x] = true;

        /* Get the danger */
        p = borg_danger(y, x, 1, true, false);

        /* Increase bravery */
        if (borg.trait[BI_MAXCLEVEL] == 50)
            fear = avoidance * 5 / 10;
        if (borg.trait[BI_MAXCLEVEL] != 50)
            fear = avoidance * 3 / 10;
        if (scaryguy_on_level)
            fear = avoidance * 2;
        if (unique_on_level && vault_on_level && borg.trait[BI_MAXCLEVEL] == 50)
            fear = avoidance * 3;
        if (scaryguy_on_level && borg.trait[BI_CLEVEL] <= 5)
            fear = avoidance * 3;
        if (borg.goal.ignoring)
            fear = avoidance * 5;
        if (borg_t - borg_began > 5000)
            fear = avoidance * 25;
        if (borg.trait[BI_FOOD] == 0)
            fear = avoidance * 100;

        /* Normal in town */
        if (borg.trait[BI_CLEVEL] == 0)
            fear = avoidance * 1 / 10;

        /* Mark dangerous grids as icky */
        if (p > fear) {
            /* Icky */
            borg_data_icky->data[y][x] = true;

            /* Avoid */
            return;
        }
    }

    /* Save the flow cost (zero) */
    borg_data_cost->data[y][x] = 0;

    /* Save "origin" */
    y1 = y;
    x1 = x;

    /* Save "destination" */
    y2 = borg.c.y;
    x2 = borg.c.x;

    /* Calculate distance components */
    ay = (y2 < y1) ? (y1 - y2) : (y2 - y1);
    ax = (x2 < x1) ? (x1 - x2) : (x2 - x1);

    /* Path */
    while (1) {
        /* Check for arrival at player */
        if ((x == x2) && (y == y2))
            return;

        /* Next */
        n++;

        /* Move mostly vertically */
        if (ay > ax) {
            /* Extract a shift factor XXX */
            shift = (n * ax + (ay - 1) / 2) / ay;

            /* Sometimes move along the minor axis */
            x = (x2 < x1) ? (x1 - shift) : (x1 + shift);

            /* Always move along major axis */
            y = (y2 < y1) ? (y1 - n) : (y1 + n);
        }

        /* Move mostly horizontally */
        else {
            /* Extract a shift factor XXX */
            shift = (n * ay + (ax - 1) / 2) / ax;

            /* Sometimes move along the minor axis */
            y = (y2 < y1) ? (y1 - shift) : (y1 + shift);

            /* Always move along major axis */
            x = (x2 < x1) ? (x1 - n) : (x1 + n);
        }

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Ignore "wall" grids and lava (unless immune to fire) */
        if (!borg_cave_floor_grid(ag)
            || (ag->feat == FEAT_LAVA && !borg.trait[BI_IFIRE]))
            return;

        /* Avoid Traps if low level-- unless brave or scaryguy. */
        if (ag->trap && avoidance <= borg.trait[BI_CURHP]
            && !scaryguy_on_level) {
            /* Do not disarm when you could end up dead */
            if (borg.trait[BI_CURHP] < 60)
                return;

            /* Do not disarm when clumsy */
            if (borg.trait[BI_DISP] < 30 && borg.trait[BI_CLEVEL] < 20)
                return;
            if (borg.trait[BI_DISP] < 45 && borg.trait[BI_CLEVEL] < 10)
                return;
            if (borg.trait[BI_DISM] < 30 && borg.trait[BI_CLEVEL] < 20)
                return;
            if (borg.trait[BI_DISM] < 45 && borg.trait[BI_CLEVEL] < 10)
                return;
        }

        /* Abort at "icky" grids */
        if (borg_data_icky->data[y][x])
            return;

        /* Analyze every grid once */
        if (!borg_data_know->data[y][x]) {
            /* Mark as known */
            borg_data_know->data[y][x] = true;

            /* Get the danger */
            p = borg_danger(y, x, 1, true, false);

            /* Increase bravery */
            if (borg.trait[BI_MAXCLEVEL] == 50)
                fear = avoidance * 5 / 10;
            if (borg.trait[BI_MAXCLEVEL] != 50)
                fear = avoidance * 3 / 10;
            if (scaryguy_on_level)
                fear = avoidance * 2;
            if (unique_on_level && vault_on_level
                && borg.trait[BI_MAXCLEVEL] == 50)
                fear = avoidance * 3;
            if (scaryguy_on_level && borg.trait[BI_CLEVEL] <= 5)
                fear = avoidance * 3;
            if (borg.goal.ignoring)
                fear = avoidance * 5;
            if (borg_t - borg_began > 5000)
                fear = avoidance * 25;
            if (borg.trait[BI_FOOD] == 0)
                fear = avoidance * 100;

            /* Normal in town */
            if (borg.trait[BI_CLEVEL] == 0)
                fear = avoidance * 1 / 10;

            /* Avoid dangerous grids (forever) */
            if (p > fear) {
                /* Mark as icky */
                borg_data_icky->data[y][x] = true;

                /* Abort */
                return;
            }
        }

        /* Abort "pointless" paths if possible */
        if (borg_data_cost->data[y][x] <= n)
            break;

        /* Save the new flow cost */
        borg_data_cost->data[y][x] = n;
    }
}

/*
 * Hack -- mark off the edges of a rectangle as "avoid" or "clear"
 */
static void borg_flow_border(int y1, int x1, int y2, int x2, bool stop)
{
    int x, y;

    /* Scan west/east edges */
    for (y = y1; y <= y2; y++) {
        /* Avoid/Clear west edge */
        borg_data_know->data[y][x1] = stop;
        borg_data_icky->data[y][x1] = stop;

        /* Avoid/Clear east edge */
        borg_data_know->data[y][x2] = stop;
        borg_data_icky->data[y][x2] = stop;
    }

    /* Scan north/south edges */
    for (x = x1; x <= x2; x++) {
        /* Avoid/Clear north edge */
        borg_data_know->data[y1][x] = stop;
        borg_data_icky->data[y1][x] = stop;

        /* Avoid/Clear south edge */
        borg_data_know->data[y2][x] = stop;
        borg_data_icky->data[y2][x] = stop;
    }
}

/*
 * Prepare to "flow" towards "interesting" grids (method 1)
 *
 * This function examines the torch-lit grids for "interesting" grids.
 */
static bool borg_flow_dark_1(int b_stair)
{
    int i;
    int x, y;

    /* Hack -- not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Reset */
    borg_temp_n = 0;

    /* Scan torch-lit grids */
    for (i = 0; i < borg_light_n; i++) {
        y = borg_light_y[i];
        x = borg_light_x[i];

        /* Skip "boring" grids (assume reachable) */
        if (!borg_flow_dark_interesting(y, x, b_stair))
            continue;

        /* don't go too far from the stairs */
        if (borg_flow_far_from_stairs(x, y, b_stair))
            continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing */
    if (!borg_temp_n)
        return false;

    /* Wipe icky codes from grids if needed */
    if (borg.goal.ignoring || scaryguy_on_level)
        borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Create paths to useful grids */
    for (i = 0; i < borg_temp_n; i++) {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Create a path */
        borg_flow_direct(y, x);
    }

    /* Attempt to Commit the flow */
    if (!borg_flow_commit(NULL, GOAL_DARK))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK))
        return false;

    /* Forget goal */
    /* goal = 0; */

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards "interesting" grids (method 2)
 *
 * This function is only used when the player is at least 4 grids away
 * from the outer dungeon wall, to prevent any nasty memory errors.
 *
 * This function examines the grids just outside the torch-lit grids
 * for "unknown" grids, and flows directly towards them (one step).
 */
static bool borg_flow_dark_2(int b_stair)
{
    int i, r;
    int x, y;

    borg_grid *ag;

    /* Hack -- not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Maximal radius */
    r = borg.trait[BI_CURLITE] + 1;

    /* Reset */
    borg_temp_n = 0;

    /* Four directions */
    for (i = 0; i < 4; i++) {
        y = borg.c.y + ddy_ddd[i] * r;
        x = borg.c.x + ddx_ddd[i] * r;

        /* Check legality */
        if (y < 1)
            continue;
        if (x < 1)
            continue;
        if (y > AUTO_MAX_Y - 2)
            continue;
        if (x > AUTO_MAX_X - 2)
            continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Require unknown */
        if (ag->feat != FEAT_NONE)
            continue;

        /* Require viewable */
        if (!(ag->info & BORG_VIEW))
            continue;

        /* don't go too far from the stairs */
        if (borg_flow_far_from_stairs(x, y, b_stair))
            continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing */
    if (!borg_temp_n)
        return false;

    /* Wipe icky codes from grids if needed */
    if (borg.goal.ignoring || scaryguy_on_level)
        borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Create paths to useful grids */
    for (i = 0; i < borg_temp_n; i++) {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Create a path */
        borg_flow_direct(y, x);
    }

    /* Attempt to Commit the flow */
    if (!borg_flow_commit(NULL, GOAL_DARK))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK))
        return false;

    /* Forget goal */
    /* goal = 0; */

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards "interesting" grids (method 3)
 *
 * Note the use of a limit on the "depth" of the flow, and of the flag
 * which avoids "unknown" grids when calculating the flow, both of which
 * help optimize this function to only handle "easily reachable" grids.
 *
 * The "borg_temp" array is much larger than any "local region".
 */
static bool borg_flow_dark_3(int b_stair)
{
    int i;
    int x, y;

    int x1, y1, x2, y2;

    /* Hack -- not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Local region */
    y1 = borg.c.y - 4;
    x1 = borg.c.x - 4;
    y2 = borg.c.y + 4;
    x2 = borg.c.x + 4;

    /* Restrict to "legal" grids */
    if (y1 < 1)
        y1 = 1;
    if (x1 < 1)
        x1 = 1;
    if (y2 > AUTO_MAX_Y - 2)
        y2 = AUTO_MAX_Y - 2;
    if (x2 > AUTO_MAX_X - 2)
        x2 = AUTO_MAX_X - 2;

    /* Reset */
    borg_temp_n = 0;

    /* Examine the region */
    for (y = y1; y <= y2; y++) {
        /* Examine the region */
        for (x = x1; x <= x2; x++) {
            /* Skip "boring" grids */
            if (!borg_flow_dark_interesting(y, x, b_stair))
                continue;

            /* Skip "unreachable" grids */
            if (!borg_flow_dark_reachable(y, x))
                continue;

            /* don't go too far from the stairs */
            if (borg_flow_far_from_stairs(x, y, b_stair))
                continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;
        }
    }

    /* Nothing interesting */
    if (!borg_temp_n)
        return false;

    /* Wipe icky codes from grids if needed */
    if (borg.goal.ignoring || scaryguy_on_level)
        borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_temp_n; i++) {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }

    /* Spread the flow (limit depth) */
    borg_flow_spread(5, false, true, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit(NULL, GOAL_DARK))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards "interesting" grids (method 4)
 *
 * Note that we avoid grids close to the edge of the panel, since they
 * induce panel scrolling, which is "expensive" in terms of CPU usage,
 * and because this allows us to "expand" the border by several grids
 * to lay down the "avoidance" border in known legal grids.
 *
 * We avoid paths that would take us into different panels by setting
 * the "icky" flag for the "border" grids to prevent path construction,
 * and then clearing them when done, to prevent confusion elsewhere.
 *
 * The "borg_temp" array is large enough to hold one panel full of grids.
 */
static bool borg_flow_dark_4(int b_stair)
{
    int i, x, y;
    int x1, y1, x2, y2;
    int leash = 250;

    /* Hack -- not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Hack -- Not if a vault is on the level */
    if (vault_on_level)
        return false;

    /* Local region */
    y1 = borg.c.y - 11;
    x1 = borg.c.x - 11;
    y2 = borg.c.y + 11;
    x2 = borg.c.x + 11;

    /* Restrict to "legal" grids */
    if (y1 < 1)
        y1 = 1;
    if (x1 < 1)
        x1 = 1;
    if (y2 > AUTO_MAX_Y - 2)
        y2 = AUTO_MAX_Y - 2;
    if (x2 > AUTO_MAX_X - 2)
        x2 = AUTO_MAX_X - 2;

    /* Nothing yet */
    borg_temp_n = 0;

    /* check the leash length */
    if (borg.trait[BI_CDEPTH] >= borg.trait[BI_CLEVEL] - 5)
        leash = borg.trait[BI_CLEVEL] * 3 + 9;

    /* Examine the panel */
    for (y = y1; y <= y2; y++) {
        /* Examine the panel */
        for (x = x1; x <= x2; x++) {
            /* Skip "boring" grids */
            if (!borg_flow_dark_interesting(y, x, b_stair))
                continue;

            /* Skip "unreachable" grids */
            if (!borg_flow_dark_reachable(y, x))
                continue;

            /* don't go too far from the stairs */
            if (borg_flow_far_from_stairs_dist(x, y, b_stair, leash))
                continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;
        }
    }

    /* Nothing useful */
    if (!borg_temp_n)
        return false;

    /* Wipe icky codes from grids if needed */
    if (borg.goal.ignoring || scaryguy_on_level)
        borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_temp_n; i++) {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }

    /* Expand borders */
    y1--;
    x1--;
    y2++;
    x2++;

    /* Avoid the edges */
    borg_flow_border(y1, x1, y2, x2, true);

    /* Spread the flow (limit depth Leash) */
    if (borg.trait[BI_CLEVEL] < 15) {
        /* Short Leash */
        borg_flow_spread(leash, true, true, false, -1, false);
    } else {
        /* Long Leash */
        borg_flow_spread(250, true, true, false, -1, false);
    }

    /* Clear the edges */
    borg_flow_border(y1, x1, y2, x2, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("dark-4", GOAL_DARK))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards "interesting" grids (method 5)
 */
static bool borg_flow_dark_5(int b_stair)
{
    int i, x, y;
    int leash = 250;

    /* Hack -- not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Nothing yet */
    borg_temp_n = 0;

    /* check the leash length */
    if (borg.trait[BI_CDEPTH] >= borg.trait[BI_CLEVEL] - 5)
        leash = borg.trait[BI_CLEVEL] * 3 + 9;

    /* Examine every "legal" grid */
    for (y = 1; y < AUTO_MAX_Y - 1; y++) {
        for (x = 1; x < AUTO_MAX_X - 1; x++) {
            /* Skip "boring" grids */
            if (!borg_flow_dark_interesting(y, x, b_stair))
                continue;

            /* Skip "unreachable" grids */
            if (!borg_flow_dark_reachable(y, x))
                continue;

            /* don't go too far from the stairs */
            if (borg_flow_far_from_stairs_dist(x, y, b_stair, leash))
                continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;

            /* Paranoia -- Check for overflow */
            if (borg_temp_n == AUTO_TEMP_MAX) {
                /* Hack -- Double break */
                y = AUTO_MAX_Y;
                x = AUTO_MAX_X;
                break;
            }
        }
    }

    /* Nothing useful */
    if (!borg_temp_n)
        return false;

    /* Wipe icky codes from grids if needed */
    if (borg.goal.ignoring || scaryguy_on_level)
        borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_temp_n; i++) {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }

    /* Spread the flow */
    if (borg.trait[BI_CLEVEL] <= 5 && avoidance <= borg.trait[BI_CURHP]) {
        /* Short Leash */
        borg_flow_spread(leash, true, true, false, -1, false);
    } else if (borg.trait[BI_CLEVEL] <= 30
               && avoidance <= borg.trait[BI_CURHP]) {
        /* Short Leash */
        borg_flow_spread(leash, true, true, false, -1, false);
    } else {
        /* Long Leash */
        borg_flow_spread(250, true, true, false, -1, false);
    }

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("dark-5", GOAL_DARK))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards "interesting" grids
 *
 * The "exploration" routines are broken into "near" and "far"
 * exploration, and each set is chosen via the flag below.
 */
bool borg_flow_dark(bool neer)
{
    int i;
    int x, y, j, b_j = -1;
    int b_stair = -1;

    /* Not if sitting in a sea of runes and we saw Morgoth recently */
    if (borg_morgoth_position && morgoth_on_level)
        return false;

    /* Paranoia */
    if (borg_flow_dark_interesting(borg.c.y, borg.c.x, -1)) {
        return false;
    }

    /* Check distance away from stairs, used later */
    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++) {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = distance(borg.c, loc(x, y));

        /* skip the closer ones */
        if (b_j >= j)
            continue;

        /* track it */
        b_j     = j;
        b_stair = i;
    }

    /* Near */
    if (neer) {
        /* Method 1 */
        if (borg_flow_dark_1(b_stair))
            return true;

        /* Method 2 */
        if (borg_flow_dark_2(b_stair))
            return true;

        /* Method 3 */
        if (borg_flow_dark_3(b_stair))
            return true;
    }
    /* Far */
    else {
        /* Method 4 */
        if (borg_flow_dark_4(b_stair))
            return true;

        /* Method 5 */
        if (borg_flow_dark_5(b_stair))
            return true;
    }

    /* Fail */
    return false;
}

#endif
