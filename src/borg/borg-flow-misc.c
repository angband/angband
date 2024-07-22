/**
 * \file borg-flow-misc.c
 * \brief Misc movement (flow) routines
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

#include "borg-flow-misc.h"

#ifdef ALLOW_BORG

#include "../cave.h"
#include "../store.h"
#include "../ui-term.h"

#include "borg-cave-light.h"
#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-escape.h"
#include "borg-flow-kill.h"
#include "borg-flow-stairs.h"
#include "borg-flow.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/*
 * Locate the store doors
 */
int *track_shop_x;
int *track_shop_y;

/*
 * Track the mineral veins with treasure
 */
struct borg_track track_vein;

/*
 * Do a "reverse" flow from the player outwards
 */
void borg_flow_reverse(int depth, bool optimize, bool avoid, bool tunneling,
    int stair_idx, bool sneak)
{
    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the player's grid */
    borg_flow_enqueue_grid(borg.c.y, borg.c.x);

    /* Spread, but do NOT optimize */
    borg_flow_spread(depth, optimize, avoid, tunneling, stair_idx, sneak);
}

/*
 * Check a floor grid for "happy" status
 *
 * These grids are floor grids which contain stairs, or which
 * are non-corners in corridors, or which are directly adjacent
 * to pillars, or grids which we have stepped on before.
 *  Stairs are good because they can be used to leave
 * the level.  Corridors are good because you can back into them
 * to avoid groups of monsters and because they can be used for
 * escaping.  Pillars are good because while standing next to a
 * pillar, you can walk "around" it in two different directions,
 * allowing you to retreat from a single normal monster forever.
 * Stepped on grids are good because they likely stem from an area
 * which has been cleared of monsters.
 */
bool borg_happy_grid_bold(int y, int x)
{
    int i;

    borg_grid *ag = &borg_grids[y][x];

    /* Bounds Check */
    if (y >= DUNGEON_HGT - 2 || y <= 2 || x >= DUNGEON_WID - 2 || x <= 2)
        return false;

    /* Accept stairs */
    if (ag->feat == FEAT_LESS)
        return true;
    if (ag->feat == FEAT_MORE)
        return true;
    if (ag->glyph)
        return true;
    if (ag->feat == FEAT_LAVA && !borg.trait[BI_IFIRE])
        return false;

    /* Hack -- weak/dark is very unhappy */
    if (borg.trait[BI_ISWEAK] || borg.trait[BI_CURLITE] == 0)
        return false;

    /* Apply a control effect so that he does not get stuck in a loop */
    if ((borg_t - borg_began) >= 2000)
        return false;

    /* Case 1a: north-south corridor */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x)
        && !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1)
        && !borg_cave_floor_bold(y + 1, x - 1)
        && !borg_cave_floor_bold(y + 1, x + 1)
        && !borg_cave_floor_bold(y - 1, x - 1)
        && !borg_cave_floor_bold(y - 1, x + 1)) {
        /* Happy */
        return true;
    }

    /* Case 1b: east-west corridor */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1)
        && !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x)
        && !borg_cave_floor_bold(y + 1, x - 1)
        && !borg_cave_floor_bold(y + 1, x + 1)
        && !borg_cave_floor_bold(y - 1, x - 1)
        && !borg_cave_floor_bold(y - 1, x + 1)) {
        /* Happy */
        return true;
    }

    /* Case 1aa: north-south doorway */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x)
        && !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1)) {
        /* Happy */
        return true;
    }

    /* Case 1ba: east-west doorway */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1)
        && !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x)) {
        /* Happy */
        return true;
    }

    /* Case 2a: north pillar */
    if (!borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y - 1, x - 1)
        && borg_cave_floor_bold(y - 1, x + 1)
        && borg_cave_floor_bold(y - 2, x)) {
        /* Happy */
        return true;
    }

    /* Case 2b: south pillar */
    if (!borg_cave_floor_bold(y + 1, x) && borg_cave_floor_bold(y + 1, x - 1)
        && borg_cave_floor_bold(y + 1, x + 1)
        && borg_cave_floor_bold(y + 2, x)) {
        /* Happy */
        return true;
    }

    /* Case 2c: east pillar */
    if (!borg_cave_floor_bold(y, x + 1) && borg_cave_floor_bold(y - 1, x + 1)
        && borg_cave_floor_bold(y + 1, x + 1)
        && borg_cave_floor_bold(y, x + 2)) {
        /* Happy */
        return true;
    }

    /* Case 2d: west pillar */
    if (!borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y - 1, x - 1)
        && borg_cave_floor_bold(y + 1, x - 1)
        && borg_cave_floor_bold(y, x - 2)) {
        /* Happy */
        return true;
    }

    /* check for grids that have been stepped on before */
    for (i = 0; i < track_step.num; i++) {
        /* Enqueue the grid */
        if ((track_step.y[i] == y) && (track_step.x[i] == x)) {
            /* Recent step is good */
            if (i < 25) {
                return true;
            }
        }
    }

    /* Not happy */
    return false;
}

/*
 * Attempt to flow to a safe grid in order to rest up properly.  Following a
 * battle, a borg needs to heal up. He will attempt to heal up right where the
 * fight was, but if he cannot, then he needs to retreat a bit. This will help
 * him find a good safe place to hide.
 */
bool borg_flow_recover(bool viewable, int dist)
{
    int i, x, y;

    /* Sometimes we loop on this */
    if (borg.time_this_panel > 500)
        return false;

    /* No retreating and recovering when low level */
    if (borg.trait[BI_CLEVEL] <= 5)
        return false;

    /* Mana for spell casters */
    if (borg_primarily_caster()) {
        if (borg.trait[BI_CURHP] > borg.trait[BI_MAXHP] / 3
            && borg.trait[BI_CURSP] > borg.trait[BI_MAXSP] / 4
            && /* Non spell casters? */
            !borg.trait[BI_ISCUT] && !borg.trait[BI_ISSTUN]
            && !borg.trait[BI_ISHEAVYSTUN] && !borg.trait[BI_ISAFRAID])
            return false;
    } else /* Non Spell Casters */
    {
        /* do I need to recover some? */
        if (borg.trait[BI_CURHP] > borg.trait[BI_MAXHP] / 3
            && !borg.trait[BI_ISCUT] && !borg.trait[BI_ISSTUN]
            && !borg.trait[BI_ISHEAVYSTUN] && !borg.trait[BI_ISAFRAID])
            return false;
    }

    /* If Fleeing, then do not rest */
    if (borg.goal.fleeing)
        return false;

    /* If Scumming, then do not rest */
    if (borg.lunal_mode || borg.munchkin_mode)
        return false;

    /* No need if hungry */
    if (borg.trait[BI_ISHUNGRY])
        return false;

    /* Nothing found */
    borg_temp_n = 0;

    /* Scan some known Grids
     * Favor the following types of grids:
     * 1. Happy grids
     */

    /* look at grids within 20 grids of me */
    for (y = borg.c.y - 25; y < borg.c.y + 25; y++) {

        for (x = borg.c.x - 25; x < borg.c.x + 25; x++) {
            /* Stay in bounds */
            if (!square_in_bounds(cave, loc(x, y)))
                continue;

            /* Skip my own grid */
            if (y == borg.c.y && x == borg.c.x)
                continue;

            /* Skip grids that are too close to me */
            if (distance(borg.c, loc(x, y)) < 7)
                continue;

            /* Is this grid a happy grid? */
            if (!borg_happy_grid_bold(y, x))
                continue;

            /* Can't rest on a wall grid. */
            /* HACK depends on FEAT order, kinda evil */
            if (borg_grids[y][x].feat >= FEAT_SECRET
                && borg_grids[y][x].feat != FEAT_PASS_RUBBLE)
                continue;

            /* Can I rest on that one? */
            if (!borg_check_rest(y, x))
                continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;
        }
    }

    /* Nothing to kill */
    if (!borg_temp_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look through the good grids */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(dist, false, true, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("Recover Grid", GOAL_RECOVER))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_RECOVER))
        return false;

    return true;
}

/*
 * Prepare to "flow" towards mineral veins with treasure
 */
bool borg_flow_vein(bool viewable, int nearness)
{
    int i, x, y;
    int b_stair = -1, j, b_j = -1;
    int cost  = 0;
    int leash = borg.trait[BI_CLEVEL] * 3 + 9;

    borg_grid *ag;

    /* Efficiency -- Nothing to take */
    if (!track_vein.num)
        return false;

    /* Increase leash */
    if (borg.trait[BI_CLEVEL] >= 20)
        leash = 250;

    /* Not needed if rich */
    if (borg.trait[BI_GOLD] >= 100000)
        return false;

    /* Require digger, capacity, or skill to dig at least Quartz */
    if (!borg_can_dig(true, FEAT_QUARTZ_K))
        return false;

    /* Nothing yet */
    borg_temp_n = 0;

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

    /* Scan the vein list */
    for (i = 0; i < track_vein.num; i++) {
        /* Access the location */
        x = track_vein.x[i];
        y = track_vein.y[i];

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW))
            continue;

        /* Clear the flow codes */
        borg_flow_clear();

        /* obtain the number of steps from this take to the stairs */
        if (nearness > 5 && borg.trait[BI_CLEVEL] < 20) {
            cost = borg_flow_cost_stair(y, x, b_stair);

            /* Check the distance to stair for this proposed grid, unless i am
             * looking for very close items (leash) */
            if (cost > leash)
                continue;
        }
        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to mine */
    if (!borg_temp_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unkown squares.  This was for performance. */
    borg_flow_spread(nearness, true, !viewable, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("vein", GOAL_TAKE))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE))
        return false;

    /* Success */
    return true;
}

/*
 * Hack -- spastic searching
 */

static uint8_t spastic_x;
static uint8_t spastic_y;

/*
 * Search carefully for secret doors and such
 */
bool borg_flow_spastic(bool bored)
{
    int cost;

    int i, x, y, v;

    int b_x = borg.c.x;
    int b_y = borg.c.y;
    int b_v = -1;
    int j, b_j = -1;
    int b_stair = -1;

    borg_grid *ag;

    /* Hack -- not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Hack -- Not if starving */
    if (borg.trait[BI_ISWEAK])
        return false;

    /* Hack -- Not if hopeless unless twitchy */
    if (borg_t - borg_began > 3000 && avoidance <= borg.trait[BI_CURHP])
        return false;

    /* Not bored */
    if (!bored) {
        /* Look around for danger */
        int p = borg_danger(borg.c.y, borg.c.x, 1, true, false);

        /* Avoid searching when in danger */
        if (p > avoidance / 4)
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

    /* We have arrived */
    if ((spastic_x == borg.c.x) && (spastic_y == borg.c.y)) {
        /* Cancel */
        spastic_x = 0;
        spastic_y = 0;

        ag        = &borg_grids[borg.c.y][borg.c.x];

        /* Take note */
        borg_note(format("# Spastic Searching at (%d,%d)...value:%d", borg.c.x,
            borg.c.y, ag->xtra));

        /* Count searching */
        for (i = 0; i < 9; i++) {
            /* Extract the location */
            int xx = borg.c.x + ddx_ddd[i];
            int yy = borg.c.y + ddy_ddd[i];

            /* Current grid */
            ag = &borg_grids[yy][xx];

            /* Tweak -- Remember the search */
            if (ag->xtra < 100)
                ag->xtra += 5;
        }

        /* we searched here */
        return false;
    }

    /* Reverse flow */
    borg_flow_reverse(250, true, false, false, -1, false);

    /* Scan the entire map */
    for (y = 1; y < AUTO_MAX_Y - 1; y++) {
        for (x = 1; x < AUTO_MAX_X - 1; x++) {
            borg_grid *ag_ptr[8];

            int wall     = 0;
            int supp     = 0;
            int diag     = 0;
            int monsters = 0;

            /* Acquire the grid */
            ag = &borg_grids[y][x];

            /* Skip unknown grids */
            if (ag->feat == FEAT_NONE)
                continue;

            /* Skip trap grids */
            if (ag->trap)
                continue;

            /* Skip walls/doors */
            if (!borg_cave_floor_grid(ag))
                continue;

            /* Acquire the cost */
            cost = borg_data_cost->data[y][x];

            /* Skip "unreachable" grids */
            if (cost >= 250)
                continue;

            /* Skip grids that are really far away.  He probably
             * won't find anything and it takes lots of turns
             */
            if (cost >= 25 && borg.trait[BI_CLEVEL] < 30)
                continue;
            if (cost >= 50)
                continue;

            /* Tweak -- Limit total searches */
            if (ag->xtra >= 50)
                continue;
            if (ag->xtra >= borg.trait[BI_CLEVEL])
                continue;

            /* Limit initial searches until bored */
            if (!bored && (ag->xtra > 5))
                continue;

            /* Avoid searching detected sectors */
            if (borg_detect_door[y / borg_panel_hgt()][x / borg_panel_wid()])
                continue;

            /* Skip ones that make me wander too far unless twitchy (Leash)*/
            if (b_stair != -1 && borg.trait[BI_CLEVEL] < 15
                && avoidance <= borg.trait[BI_CURHP]) {
                /* Check the distance of this grid to the stair */
                j = borg_distance(
                    track_less.y[b_stair], track_less.x[b_stair], y, x);
                /* Distance of me to the stairs */
                b_j = borg_distance(borg.c.y, borg.c.x, track_less.y[b_stair],
                    track_less.x[b_stair]);

                /* skip far away grids while I am close to stair*/
                if (b_j <= borg.trait[BI_CLEVEL] * 3 + 9
                    && j >= borg.trait[BI_CLEVEL] * 3 + 9)
                    continue;

                /* If really low level don't do this much */
                if (borg.trait[BI_CLEVEL] <= 3
                    && b_j <= borg.trait[BI_CLEVEL] + 9
                    && j >= borg.trait[BI_CLEVEL] + 9)
                    continue;

                /* Do not Venture too far from stair */
                if (borg.trait[BI_CLEVEL] <= 3
                    && j >= borg.trait[BI_CLEVEL] + 5)
                    continue;

                /* Do not Venture too far from stair */
                if (borg.trait[BI_CLEVEL] <= 10
                    && j >= borg.trait[BI_CLEVEL] + 9)
                    continue;
            }

            /* Extract adjacent locations */
            for (i = 0; i < 8; i++) {
                /* Extract the location */
                int xx = x + ddx_ddd[i];
                int yy = y + ddy_ddd[i];

                /* Get the grid contents */
                ag_ptr[i] = &borg_grids[yy][xx];
            }

            /* Count possible door locations */
            for (i = 0; i < 4; i++) {
                ag = ag_ptr[i];
                if (ag->feat >= FEAT_GRANITE)
                    wall++;
            }

            /* No possible secret doors */
            if (wall < 1)
                continue;

            /* Count supporting evidence for secret doors */
            for (i = 0; i < 4; i++) {
                ag = ag_ptr[i];

                /* Rubble */
                if (ag->feat == FEAT_RUBBLE)
                    continue;

                /* Walls, Doors */
                if (((ag->feat >= FEAT_SECRET) && (ag->feat <= FEAT_GRANITE))
                    || ((ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN))
                    || (ag->feat == FEAT_CLOSED)) {
                    supp++;
                }
            }

            /* Count supporting evidence for secret doors */
            for (i = 4; i < 8; i++) {
                ag = ag_ptr[i];

                /* Rubble */
                if (ag->feat == FEAT_RUBBLE)
                    continue;

                /* Walls */
                if (ag->feat >= FEAT_SECRET) {
                    diag++;
                }
            }

            /* No possible secret doors */
            if (diag < 2)
                continue;

            /* Count monsters */
            for (i = 0; i < 8; i++) {
                ag = ag_ptr[i];

                /* monster */
                if (ag->kill)
                    monsters++;
            }

            /* No search near monsters */
            if (monsters >= 1)
                continue;

            /* Tweak -- Reward walls, punish visitation, distance, time on level
             */
            v = (supp * 500) + (diag * 100) - (ag->xtra * 40) - (cost * 2)
                - (borg_t - borg_began);

            /* Punish low level and searching too much */
            v -= (50 - borg.trait[BI_CLEVEL]) * 5;

            /* The grid is not searchable */
            if (v <= 0)
                continue;

            /* Tweak -- Minimal interest until bored */
            if (!bored && (v < 1500))
                continue;

            /* Track "best" grid */
            if ((b_v >= 0) && (v < b_v))
                continue;

            /* Save the data */
            b_v = v;
            b_x = x;
            b_y = y;
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (b_v < 0)
        return false;

    /* Access grid */
    ag = &borg_grids[b_y][b_x];

    /* Memorize */
    spastic_x = b_x;
    spastic_y = b_y;

    /* Enqueue the grid */
    borg_flow_enqueue_grid(b_y, b_x);

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("spastic", GOAL_XTRA))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_XTRA))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to "flow" towards a specific shop entry
 */
bool borg_flow_shop_entry(int i)
{
    int x, y;

    const char *name = (f_info[stores[i].feat].name);

    /* Must be in town */
    if (borg.trait[BI_CDEPTH])
        return false;

    /* Obtain the location */
    x = track_shop_x[i];
    y = track_shop_y[i];

    /* Hack -- Must be known */
    if (!x || !y)
        return false;

    /* Hack -- re-enter a shop if needed */
    if ((x == borg.c.x) && (y == borg.c.y)) {
        /* Note */
        borg_note("# Re-entering a shop");

        /* Enter the store */
        borg_keypress('5');

        /* Success */
        return true;
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the grid */
    borg_flow_enqueue_grid(y, x);

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit(name, GOAL_MISC))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_MISC))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to flow towards light
 */
bool borg_flow_light(int why)
{
    int y, x, i;

    /* reset counters */
    borg_glow_n = 0;
    i           = 0;

    /* build the glow array */
    /* Scan map */
    for (y = w_y; y < w_y + SCREEN_HGT; y++) {
        for (x = w_x; x < w_x + SCREEN_WID; x++) {
            borg_grid *ag = &borg_grids[y][x];

            /* Not a perma-lit, and not our spot. */
            if (!(ag->info & BORG_GLOW))
                continue;

            /* keep count */
            borg_glow_y[borg_glow_n] = y;
            borg_glow_x[borg_glow_n] = x;
            borg_glow_n++;
        }
    }
    /* None to flow to */
    if (!borg_glow_n)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_glow_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_glow_y[i], borg_glow_x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("a lighted area", why))
        return false;

    /* Take one step */
    if (!borg_flow_old(why))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to flow towards a vault grid which can be excavated
 */
bool borg_flow_vault(int nearness)
{
    int  y, x, i;
    int  b_y, b_x;
    bool can_dig_hard;

    borg_grid *ag;

    /* reset counters */
    borg_temp_n = 0;
    i           = 0;

    /* no need if no vault on level */
    if (!vault_on_level)
        return false;

    /* no need if we can't dig at least quartz */
    if (!borg_can_dig(false, FEAT_QUARTZ))
        return false;

    can_dig_hard = borg_can_dig(false, FEAT_GRANITE);

    /* build the array -- Scan screen */
    for (y = w_y; y < w_y + SCREEN_HGT; y++) {
        for (x = w_x; x < w_x + SCREEN_WID; x++) {

            /* only bother with near ones */
            if (distance(borg.c, loc(x, y)) > nearness)
                continue;

            uint8_t feat = borg_grids[y][x].feat;

            /* only deal with excavatable walls */
            if (feat != FEAT_RUBBLE
                && feat != FEAT_QUARTZ 
                && feat != FEAT_MAGMA
                && feat != FEAT_QUARTZ_K 
                && feat != FEAT_MAGMA_K) {
                /* only deal with granite if we are good diggers */
                if (!can_dig_hard || feat != FEAT_GRANITE)
                    continue;
            }

            /* Examine grids adjacent to this grid to see if there is a perma
             * wall adjacent */
            for (i = 0; i < 8; i++) {
                b_x = x + ddx_ddd[i];
                b_y = y + ddy_ddd[i];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(b_x, b_y)))
                    continue;

                /* Access the grid */
                ag = &borg_grids[b_y][b_x];

                /* Not a perma, and not our spot. */
                if (ag->feat != FEAT_PERM)
                    continue;

                /* keep count */
                borg_temp_y[borg_temp_n] = y;
                borg_temp_x[borg_temp_n] = x;
                borg_temp_n++;
            }
        }
    }

    /* None to flow to */
    if (!borg_temp_n)
        return false;

    /* Examine each ones */
    for (i = 0; i < borg_temp_n; i++) {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("vault excavation", GOAL_VAULT))
        return false;

    /* Take one step */
    if (!borg_flow_old(GOAL_VAULT))
        return false;

    /* Success */
    return true;
}

/*
 * Act twitchy
 */
bool borg_twitchy(void)
{
    int dir = 5;
    int count;
    bool all_walls = true;
    borg_grid * grid = NULL;
    struct loc l;

    /* This is a bad thing */
    borg_note("# Twitchy!");

    /* try to phase out of it */
    if (borg_allow_teleport()) {
        if (borg_caution_phase(15, 2)
            && (borg_spell_fail(PHASE_DOOR, 40) || borg_spell_fail(PORTAL, 40)
                || borg_shadow_shift(40) || borg_activate_item(act_tele_phase)
                || borg_activate_item(act_tele_long)
                || borg_read_scroll(sv_scroll_phase_door))) {
            /* We did something */
            return true;
        }
    }

    /* Pick a random direction */
    count = 20;
    while (true) {
        dir = randint0(10);
        if (dir == 5 || dir == 0)
            continue;

        if (!count)
            break;

        count--;

        /* Hack -- set goal */
        borg.goal.g.x = borg.c.x + ddx[dir];
        borg.goal.g.y = borg.c.y + ddy[dir];

        if (!square_in_bounds_fully(cave, borg.goal.g))
            continue;

        grid = &borg_grids[borg.goal.g.y][borg.goal.g.x];

        /* don't twitch into walls */
        if (grid->feat >= FEAT_SECRET && grid->feat <= FEAT_PERM)
            continue;

        /* monsters count as walls if afraid */
        if (grid->kill && borg.trait[BI_ISAFRAID])
            continue;

        break;
    }

    /* if we can't find a direction, maybe we are surrounded by walls */
    if (!count) {
        for (dir = 1; dir < 10; dir++) {
            if (dir == 5)
                continue;

            /* get the location of postion + direction */
            l.x = borg.c.x + ddx[dir];
            l.y = borg.c.y + ddy[dir];

            if (!square_in_bounds_fully(cave, l))
                continue;

            grid = &borg_grids[l.y][l.x];

            /* don't twitch into walls */
            if (grid->feat >= FEAT_SECRET && grid->feat <= FEAT_PERM) {
                /* unless afraid (and even then, not perm walls) */
                if (!borg.trait[BI_ISAFRAID] || grid->feat == FEAT_PERM)
                    continue;
            }

            /* monsters count as walls if afraid */
            if (grid->kill && borg.trait[BI_ISAFRAID])
                continue;

            all_walls = false;
            break;
        }
        if (all_walls) {
            /* Rest until done */
            borg_keypress('R');
            borg_keypress('1');
            borg_keypress('0');
            borg_keypress('0');
            borg_keypress(KC_ENTER);
            /* We did something */
            return true;
        }
    }

    /* Normally move */

    /* if afraid, we need to try to dig. We have tried everything else */
    /* digging will at least take an action so maybe give time to not be */
    /* afraid */
    if (borg.trait[BI_ISAFRAID])
        borg_keypress('+');

    /* Send direction */
    borg_keypress(I2D(dir));

    /* We did something */
    return true;
}

/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * Note that we use "diagonal" motion whenever possible.
 *
 * We return "5" if no motion is needed.
 */
int borg_extract_dir(int y1, int x1, int y2, int x2)
{
    /* No movement required */
    if ((y1 == y2) && (x1 == x2))
        return 5;

    /* South or North */
    if (x1 == x2)
        return ((y1 < y2) ? 2 : 8);

    /* East or West */
    if (y1 == y2)
        return ((x1 < x2) ? 6 : 4);

    /* South-east or South-west */
    if (y1 < y2)
        return ((x1 < x2) ? 3 : 1);

    /* North-east or North-west */
    if (y1 > y2)
        return ((x1 < x2) ? 9 : 7);

    /* Paranoia */
    return 5;
}

/*
 * Given a "source" and "target" locations, travel in a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * We prefer "non-diagonal" motion, which allows us to save the
 * "diagonal" moves for avoiding pillars and other obstacles.
 *
 * If no "obvious" path is available, we use "borg_extract_dir()".
 *
 * We return "5" if no motion is needed.
 */
int borg_goto_dir(int y1, int x1, int y2, int x2)
{
    int d, e;

    int ay = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int ax = (x2 > x1) ? (x2 - x1) : (x1 - x2);

    /* Default direction */
    e = borg_extract_dir(y1, x1, y2, x2);

    /* Adjacent location, use default */
    if ((ax <= 1) && (ay <= 1))
        return (e);

    /* Try south/north (primary) */
    if (ay > ax) {
        d = (y1 < y2) ? 2 : 8;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;
    }

    /* Try east/west (primary) */
    if (ay < ax) {
        d = (x1 < x2) ? 6 : 4;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;
    }

    /* Try diagonal */
    d = borg_extract_dir(y1, x1, y2, x2);

    /* Check for walls */
    if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
        return d;

    /* Try south/north (secondary) */
    if (ay <= ax) {
        d = (y1 < y2) ? 2 : 8;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;
    }

    /* Try east/west (secondary) */
    if (ay >= ax) {
        d = (x1 < x2) ? 6 : 4;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;
    }

    /* Circle obstacles */
    if (!ay) {
        /* Circle to the south */
        d = (x1 < x2) ? 3 : 1;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;

        /* Circle to the north */
        d = (x1 < x2) ? 9 : 7;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;
    }

    /* Circle obstacles */
    if (!ax) {
        /* Circle to the east */
        d = (y1 < y2) ? 3 : 9;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;

        /* Circle to the west */
        d = (y1 < y2) ? 1 : 7;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d]))
            return d;
    }

    /* Oops */
    return (e);
}

/*
 * check to make sure there are no monsters around that should prevent resting
 */
bool borg_check_rest(int y, int x)
{
    int  i, ii;
    bool borg_in_vault = false;

    /* never rest to recover SP (if HP at max) if you only recover */
    /* sp in combat */
    if (borg.trait[BI_CURHP] == borg.trait[BI_MAXHP]
        && player_has(player, PF_COMBAT_REGEN))
        return false;

    /* Do not rest recently after killing a multiplier */
    /* This will avoid the problem of resting next to */
    /* an unkown area full of breeders */
    if (borg.when_last_kill_mult > (borg_t - 4)
        && borg.when_last_kill_mult <= borg_t)
        return false;

    /* No resting if Blessed and good HP and good SP */
    /* don't rest for SP if you do combat regen */
    if ((borg.temp.bless || borg.temp.hero || borg.temp.berserk
            || borg.temp.fastcast || borg.temp.regen || borg.temp.smite_evil)
        && !borg.munchkin_mode
        && (borg.trait[BI_CURHP] >= borg.trait[BI_MAXHP] * 8 / 10)
        && (borg.trait[BI_CURSP] >= borg.trait[BI_MAXSP] * 7 / 10))
        return false;

    /* Set this to Zero */
    borg.when_last_kill_mult = 0;

    /* Most of the time, its ok to rest in a vault */
    if (vault_on_level) {
        for (i = -1; i < 1; i++) {
            for (ii = -1; ii < 1; ii++) {
                /* check bounds */
                if (!square_in_bounds_fully(
                        cave, loc(borg.c.x + ii, borg.c.y + i)))
                    continue;

                if (borg_grids[borg.c.y + i][borg.c.x + ii].feat == FEAT_PERM)
                    borg_in_vault = true;
            }
        }
    }

    /* No resting to recover if I just cast a prepatory spell
     * which is what I like to do right before I take a stair,
     * Unless I am down by three quarters of my SP.
     */
    if (borg.no_rest_prep >= 1 && !borg.munchkin_mode
        && borg.trait[BI_CURSP] > borg.trait[BI_MAXSP] / 4
        && borg.trait[BI_CDEPTH] < 85)
        return false;

    /* Don't rest on lava unless we are immune to fire */
    if (borg_grids[y][x].feat == FEAT_LAVA && !borg.trait[BI_IFIRE])
        return false;

    /* Dont worry about fears if in a vault */
    if (!borg_in_vault) {
        /* Be concerned about the Regional Fear. */
        if (borg_fear_region[y / 11][x / 11] > borg.trait[BI_CURHP] / 20
            && borg.trait[BI_CDEPTH] != 100)
            return false;

        /* Be concerned about the Monster Fear. */
        if (borg_fear_monsters[y][x] > borg.trait[BI_CURHP] / 10
            && borg.trait[BI_CDEPTH] != 100)
            return false;

        /* Be concerned about the Monster Danger. */
        if (borg_danger(y, x, 1, true, false) > borg.trait[BI_CURHP] / 40
            && borg.trait[BI_CDEPTH] >= 85)
            return false;

        /* Be concerned if low on food */
        if ((borg.trait[BI_CURLITE] == 0 || borg.trait[BI_ISWEAK]
                || borg.trait[BI_FOOD] < 2)
            && !borg.munchkin_mode)
            return false;
    }

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill           *kill  = &borg_kills[i];
        struct monster_race *r_ptr = &r_info[kill->r_idx];

        int x9                     = kill->pos.x;
        int y9                     = kill->pos.y;
        int ax, ay, d;
        int p = 0;

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Distance components */
        ax = (x9 > x) ? (x9 - x) : (x - x9);
        ay = (y9 > y) ? (y9 - y) : (y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Minimal distance */
        if (d > z_info->max_range)
            continue;

        /* if too close to a Mold or other Never-Mover, don't rest */
        if (d < 2 && !(rf_has(r_ptr->flags, RF_NEVER_MOVE)))
            return false;
        if (d == 1)
            return false;

        /* if too close to a Multiplier, don't rest */
        if (d < 10 && (rf_has(r_ptr->flags, RF_MULTIPLY)))
            return false;

        /* If monster is asleep, dont worry */
        if (!kill->awake && d > 8 && !borg.munchkin_mode)
            continue;

        /* one call for dangers */
        p = borg_danger_one_kill(y9, x9, 1, i, true, true);

        /* Ignore proximity checks while inside a vault */
        if (!borg_in_vault) {
            /* Real scary guys pretty close */
            if (d < 5 && (p > avoidance / 3) && !borg.munchkin_mode)
                return false;

            /* scary guys far away */
            /*if (d < 17 && d > 5 && (p > avoidance/3)) return false; */
        }

        /* should check LOS... monster to me concerned for Ranged Attacks */
        if (borg_los(y9, x9, y, x) && kill->ranged_attack)
            return false;

        /* Special handling for the munchkin mode */
        if (borg.munchkin_mode && borg_los(y9, x9, y, x)
            && (kill->awake && !(rf_has(r_ptr->flags, RF_NEVER_MOVE))))
            return false;

        /* if it walks through walls, not safe */
        if ((rf_has(r_ptr->flags, RF_PASS_WALL)) && !borg_in_vault)
            return false;
        if (rf_has(r_ptr->flags, RF_KILL_WALL) && !borg_in_vault)
            return false;
    }
    return true;
}

/* check if this spot is too far from the stairs */
bool borg_flow_far_from_stairs(int x, int y, int b_stair)
{
    return borg_flow_far_from_stairs_dist(
        x, y, b_stair, borg.trait[BI_CLEVEL] * 3 + 9);
}

/* check if this spot is too far from the stairs */
bool borg_flow_far_from_stairs_dist(int x, int y, int b_stair, int distance)
{
    if (borg.trait[BI_CDEPTH] >= borg.trait[BI_CLEVEL] - 5
        && borg.trait[BI_CLEVEL] < 20) {

        /* obtain the number of steps from this take to the stairs */
        int cost = borg_flow_cost_stair(y, x, b_stair);

        /* Check the distance to stair for this proposed grid */
        if (cost > distance)
            return true;
    }

    return false;
}

void borg_init_flow_misc(void)
{
    /* Track the shop locations */
    track_shop_x = mem_zalloc(9 * sizeof(int));
    track_shop_y = mem_zalloc(9 * sizeof(int));

    /* Track mineral veins with treasure. */
    borg_init_track(&track_vein, 100);
}

void borg_free_flow_misc(void)
{
    borg_free_track(&track_vein);

    mem_free(track_shop_y);
    track_shop_y = NULL;
    mem_free(track_shop_x);
    track_shop_x = NULL;
}

#endif
