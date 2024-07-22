/**
 * \file borg-flow.c
 * \brief The basic things used to flow (move) around
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

#include "borg-flow.h"

#ifdef ALLOW_BORG

#include "../ui-input.h"

#include "borg-danger.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow-take.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Hack -- use "flow" array as a queue
 */

int flow_head = 0;
int flow_tail = 0;

/*
 * Maintain a circular queue of grids
 */

int16_t borg_flow_n = 0;

uint8_t borg_flow_x[AUTO_FLOW_MAX];
uint8_t borg_flow_y[AUTO_FLOW_MAX];

/*
 * Some variables
 */
borg_data *borg_data_flow; /* Current "flow" data */
borg_data *borg_data_cost; /* Current "cost" data */
borg_data *borg_data_hard; /* Constant "hard" data */
borg_data *borg_data_know; /* Current "know" flags */
borg_data *borg_data_icky; /* Current "icky" flags */

/*
 * Maintain a temporary set of grids
 * Used to store monster info.
 */

int16_t borg_temp_n = 0;

uint8_t borg_temp_x[AUTO_TEMP_MAX];
uint8_t borg_temp_y[AUTO_TEMP_MAX];

/*
 * Track Steps
 */
struct borg_track track_step;

/*
 * Track closed doors which I have closed
 */
struct borg_track track_door;

/*
 * Track closed doors which started closed
 */
struct borg_track track_closed;

bool borg_desperate = false;

/*
 * HACK assume a permeant wall in the center is a part of a vault
 */
bool vault_on_level;

/*
 * Anti-Summon
 */
int  borg_t_antisummon; /* Timestamp when in a AS spot */
bool borg_as_position; /* Sitting in an anti-summon corridor */
bool borg_digging; /* used in Anti-summon corridor */
bool my_need_alter; /* incase i hit a wall or door */
bool my_no_alter;
bool my_need_redraw; /* incase i hit a wall or door */

int16_t avoidance = 0; /* Current danger thresh-hold */

/*
 * ghijk  The borg will use the following ddx and ddy to search
 * d827a  for a suitable grid in an open room.
 * e4@3b
 * f615c
 * lmnop  24 grids
 *
 */
const int16_t borg_ddx_ddd[24] = { 0, 0, 1, -1, 1, -1, 1, -1, 2, 2, 2, -2, -2,
    -2, -2, -1, 0, 1, 2, -2, -1, 0, 1, 2 };
const int16_t borg_ddy_ddd[24] = { 1, -1, 0, 0, 1, 1, -1, -1, -1, 0, 1, -1, 0,
    1, -2, -2, -2, -2, -2, 2, 2, 2, 2, 2 };

/*
 * Check if the borg can dig.
 *   check_fail = check if the spell failure rate is too high
 *   hard = check if hard things, like granite, can be dug
 */
bool borg_can_dig(bool check_fail, uint8_t feat)
{
    /* No digging when hungry */
    if (borg.trait[BI_ISHUNGRY])
        return false;

    /* some features can't be dug out */
    if (feat == FEAT_PERM || feat == FEAT_LAVA || feat < FEAT_SECRET)
        return false;

    int dig_check = feat == FEAT_GRANITE ? BORG_DIG_HARD : BORG_DIG;
    if ((weapon_swap && borg.trait[BI_DIG] >= dig_check
            && borg_items[weapon_swap - 1].tval == TV_DIGGING)
        || (borg.trait[BI_DIG] >= dig_check + 20))
        return true;

    if (feat == FEAT_RUBBLE && !borg.trait[BI_ISWEAK])
        return true;

    if (check_fail) {
        if (borg_spell_okay_fail(TURN_STONE_TO_MUD, 40)
            || borg_spell_okay_fail(SHATTER_STONE, 40)
            || borg_equips_item(act_stone_to_mud, true)
            || borg_equips_ring(sv_ring_digging))
            return true;
    } else {
        if (borg_spell_okay(TURN_STONE_TO_MUD) 
            || borg_spell_okay(SHATTER_STONE)
            || borg_equips_item(act_stone_to_mud, false)
            || borg_equips_ring(sv_ring_digging))
            return true;
    }

    return false;
}

/*
 * Clear the "flow" information
 */
void borg_flow_clear(void)
{
    /* Reset the "cost" fields */
    memcpy(borg_data_cost, borg_data_hard, sizeof(borg_data));

    /* Wipe costs and danger */
    if (borg_danger_wipe) {
        /* Wipe the "know" flags */
        memset(borg_data_know, 0, sizeof(borg_data));

        /* Wipe the "icky" flags */
        memset(borg_data_icky, 0, sizeof(borg_data));

        /* Wipe complete */
        borg_danger_wipe = false;
    }

    /* Start over */
    flow_head = 0;
    flow_tail = 0;
}

/*
 * Spread a "flow" from the "destination" grids outwards
 *
 * We fill in the "cost" field of every grid that the player can
 * "reach" with the number of steps needed to reach that grid,
 * if the grid is "reachable", and otherwise, with "255", which
 * is the largest possible value that can be stored in a byte.
 *
 * Thus, certain grids which are actually "reachable" but only by
 * a path which is at least 255 steps in length will thus appear
 * to be "unreachable", but this is not a major concern.
 *
 * We use the "flow" array as a "circular queue", and thus we must
 * be careful not to allow the "queue" to "overflow".  This could
 * only happen with a large number of distinct destination points,
 * each several units away from every other destination point, and
 * in a dungeon with no walls and no dangerous monsters.  But this
 * is technically possible, so we must check for it just in case.
 *
 * We do not need a "priority queue" because the cost from grid to
 * grid is always "one" and we process them in order.  If we did
 * use a priority queue, this function might become unusably slow,
 * unless we reactivated the "room building" code.
 *
 * We handle both "walls" and "danger" by marking every grid which
 * is "impassible", due to either walls, or danger, as "ICKY", and
 * marking every grid which has been "checked" as "KNOW", allowing
 * us to only check the wall/danger status of any grid once.  This
 * provides some important optimization, since many "flows" can be
 * done before the "ICKY" and "KNOW" flags must be reset.
 *
 * Note that the "borg_enqueue_grid()" function should refuse to
 * enqueue "dangerous" destination grids, but does not need to set
 * the "KNOW" or "ICKY" flags, since having a "cost" field of zero
 * means that these grids will never be queued again.  In fact,
 * the "borg_enqueue_grid()" function can be used to enqueue grids
 * which are "walls", such as "doors" or "rubble".
 *
 * This function is extremely expensive, and is a major bottleneck
 * in the code, due more to internal processing than to the use of
 * the "borg_danger()" function, especially now that the use of the
 * "borg_danger()" function has been optimized several times.
 *
 * The "optimize" flag allows this function to stop as soon as it
 * finds any path which reaches the player, since in general we are
 * looking for paths to destination grids which the player can take,
 * and we can stop this function as soon as we find any usable path,
 * since it will always be as short a path as possible.
 *
 * We queue the "children" in reverse order, to allow any "diagonal"
 * neighbors to be processed first, since this may boost efficiency.
 *
 * Note that we should recalculate "danger", and reset all "flows"
 * if we notice that a wall has disappeared, and if one appears, we
 * must give it a maximal cost, and mark it as "icky", in case it
 * was currently included in any flow.
 *
 * If a "depth" is given, then the flow will only be spread to that
 * depth, note that the maximum legal value of "depth" is 250.
 *
 * "Avoid" flag means the borg will not move onto unknown grids,
 * nor to Monster grids if borg_desperate or borg.lunal_mode are
 * set.
 *
 * "Sneak" will have the borg avoid grids which are adjacent to a monster.
 *
 */
void borg_flow_spread(int depth, bool optimize, bool avoid, bool tunneling,
    int stair_idx, bool sneak)
{
    int  i;
    int  n, o = 0;
    int  x1, y1;
    int  x, y;
    int  fear = 0;
    int  ii;
    int  yy, xx;
    bool bad_sneak = false;
    int  origin_y, origin_x;
    bool twitchy = false;

    /* Default starting points */
    origin_y = borg.c.y;
    origin_x = borg.c.x;

    /* Is the borg moving under boosted bravery? */
    if (avoidance > borg.trait[BI_CURHP])
        twitchy = true;

    /* Use the closest stair for calculation distance (cost) from the stair to
     * the goal */
    if (stair_idx >= 0 && borg.trait[BI_CLEVEL] < 15) {
        origin_y = track_less.y[stair_idx];
        origin_x = track_less.x[stair_idx];
        optimize = false;
    }

    /* Now process the queue */
    while (flow_head != flow_tail) {
        /* Extract the next entry */
        x1 = borg_flow_x[flow_tail];
        y1 = borg_flow_y[flow_tail];

        /* Circular queue -- dequeue the next entry */
        if (++flow_tail == AUTO_FLOW_MAX)
            flow_tail = 0;

        /* Cost (one per movement grid) */
        n = borg_data_cost->data[y1][x1] + 1;

        /* New depth */
        if (n > o) {
            /* Optimize (if requested) */
            if (optimize && (n > borg_data_cost->data[origin_y][origin_x]))
                break;

            /* Limit depth */
            if (n > depth)
                break;

            /* Save */
            o = n;
        }

        /* Queue the "children" */
        for (i = 0; i < 8; i++) {
            int old_head;

            borg_grid *ag;

            /* reset bad_sneak */
            bad_sneak = false;

            /* Neighbor grid */
            x = x1 + ddx_ddd[i];
            y = y1 + ddy_ddd[i];

            /* only on legal grids */
            if (!square_in_bounds_fully(cave, loc(x, y)))
                continue;

            /* Skip "reached" grids */
            if (borg_data_cost->data[y][x] <= n)
                continue;

            /* Access the grid */
            ag = &borg_grids[y][x];

            if (sneak) {
                /* Scan the neighbors */
                for (ii = 0; ii < 8; ii++) {
                    /* Neighbor grid */
                    xx = x + ddx_ddd[ii];
                    yy = y + ddy_ddd[ii];

                    /* only on legal grids */
                    if (!square_in_bounds_fully(cave, loc(xx, yy)))
                        continue;

                    /* Make sure no monster is on this grid, which is
                     * adjacent to the grid on which, I am thinking about
                     * stepping.
                     */
                    if (borg_grids[yy][xx].kill) {
                        bad_sneak = true;
                        break;
                    }
                }
            }
            /* The grid I am thinking about is adjacent to a monster */
            if (sneak && bad_sneak && !borg_desperate && !twitchy)
                continue;

            /* Avoid "wall" grids (not doors) unless tunneling*/
            /* HACK depends on FEAT order, kinda evil */
            if (!tunneling
                && (ag->feat >= FEAT_SECRET && ag->feat != FEAT_PASS_RUBBLE
                    && ag->feat != FEAT_LAVA))
                continue;

            /* Avoid "perma-wall" grids */
            if (ag->feat == FEAT_PERM)
                continue;

            /* Avoid "Lava" grids (for now) */
            if (ag->feat == FEAT_LAVA && !borg.trait[BI_IFIRE])
                continue;

            /* Avoid unknown grids (if requested or retreating)
             * unless twitchy.  In which case, explore it
             */
            if ((avoid || borg_desperate) && (ag->feat == FEAT_NONE)
                && !twitchy)
                continue;

            /* flowing into monsters */
            if ((ag->kill)) {
                /* Avoid if Desperate, lunal */
                if (borg_desperate || borg.lunal_mode || borg.munchkin_mode)
                    continue;

                /* Avoid if afraid */
                if (borg.trait[BI_ISAFRAID])
                    continue;

                /* Avoid if low level, unless twitchy */
                if (!twitchy && borg.trait[BI_FOOD] >= 2
                    && borg.trait[BI_MAXCLEVEL] < 5)
                    continue;
            }

            /* Avoid shop entry points if I am not heading to that shop */
            if (borg.goal.shop >= 0 && feat_is_shop(ag->feat)
                && (ag->store != borg.goal.shop) && y != borg.c.y
                && x != borg.c.x)
                continue;

            /* Avoid Traps if low level-- unless brave */
            if (ag->trap && !ag->glyph && !twitchy) {
                /* Do not disarm when you could end up dead */
                if (borg.trait[BI_CURHP] < 60)
                    continue;

                /* Do not disarm when clumsy */
                /* since traps can be physical or magical, gotta check both */
                if (borg.trait[BI_DISP] < 30 && borg.trait[BI_CLEVEL] < 20)
                    continue;
                if (borg.trait[BI_DISP] < 45 && borg.trait[BI_CLEVEL] < 10)
                    continue;
                if (borg.trait[BI_DISM] < 30 && borg.trait[BI_CLEVEL] < 20)
                    continue;
                if (borg.trait[BI_DISM] < 45 && borg.trait[BI_CLEVEL] < 10)
                    continue;

                /* NOTE:  Traps are tough to deal with as a low
                 * level character.  If any modifications are made above,
                 * then the same changes must be made to borg_flow_direct()
                 * and borg_flow_interesting()
                 */
            }

            /* Ignore "icky" grids */
            if (borg_data_icky->data[y][x])
                continue;

            /* Analyze every grid once */
            if (!borg_data_know->data[y][x]) {
                int p;

                /* Mark as known */
                borg_data_know->data[y][x] = true;

                if (!borg_desperate && !borg.lunal_mode && !borg.munchkin_mode
                    && !borg_digging) {
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
                        fear = avoidance * 3 / 10;

                    /* Dangerous grid */
                    if (p > fear) {
                        /* Mark as icky */
                        borg_data_icky->data[y][x] = true;

                        /* Ignore this grid */
                        continue;
                    }
                }
            }

            /* Save the flow cost */
            borg_data_cost->data[y][x] = n;

            /* Enqueue that entry */
            borg_flow_x[flow_head] = x;
            borg_flow_y[flow_head] = y;

            /* Circular queue -- memorize head */
            old_head = flow_head;

            /* Circular queue -- insert with wrap */
            if (++flow_head == AUTO_FLOW_MAX)
                flow_head = 0;

            /* Circular queue -- handle overflow (badly) */
            if (flow_head == flow_tail)
                flow_head = old_head;
        }
    }

    /* Forget the flow info */
    flow_head = flow_tail = 0;
}

/*
 * Enqueue a fresh (legal) starting grid, if it is safe
 */
void borg_flow_enqueue_grid(int y, int x)
{
    int old_head;
    int fear = 0;
    int p;

    /* Avoid icky grids */
    if (borg_data_icky->data[y][x])
        return;

    /* Unknown */
    if (!borg_data_know->data[y][x]) {
        /* Mark as known */
        borg_data_know->data[y][x] = true;

        /** Mark dangerous grids as icky **/

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
            fear = avoidance * 3 / 10;

        /* Dangerous grid */
        if ((p > fear) && !borg_desperate && !borg.lunal_mode
            && !borg.munchkin_mode && !borg_digging) {
            /* Icky */
            borg_data_icky->data[y][x] = true;

            /* Avoid */
            return;
        }
    }

    /* Only enqueue a grid once */
    if (!borg_data_cost->data[y][x])
        return;

    /* Save the flow cost (zero) */
    borg_data_cost->data[y][x] = 0;

    /* Enqueue that entry */
    borg_flow_y[flow_head] = y;
    borg_flow_x[flow_head] = x;

    /* Circular queue -- memorize head */
    old_head = flow_head;

    /* Circular queue -- insert with wrap */
    if (++flow_head == AUTO_FLOW_MAX)
        flow_head = 0;

    /* Circular queue -- handle overflow */
    if (flow_head == flow_tail)
        flow_head = old_head;
}

/*
 * Commit the current "flow"
 */
bool borg_flow_commit(const char *who, int why)
{
    int cost;

    /* Cost of current grid */
    cost = borg_data_cost->data[borg.c.y][borg.c.x];

    /* Verify the total "cost" */
    if (cost >= 250)
        return false;

    /* Message */
    if (who)
        borg_note(format("# Flowing toward %s at cost %d", who, cost));

    /* Obtain the "flow" information */
    memcpy(borg_data_flow, borg_data_cost, sizeof(borg_data));

    /* Save the goal type */
    borg.goal.type = why;

    /* Success */
    return true;
}

/*
 * Take one "step" towards the given location, return true if possible
 */
static bool borg_play_step(int y2, int x2)
{
    borg_grid *ag;
    borg_grid *ag2;
    ui_event   ch_evt = EVENT_EMPTY;
    int        dir, x, y, ox, oy, i;

    int o_y = 0, o_x = 0, door_found = 0;

    /* Breeder levels, close all doors */
    if (breeder_level) {
        /* scan the adjacent grids */
        for (ox = -1; ox <= 1; ox++) {
            for (oy = -1; oy <= 1; oy++) {
                /* skip our own spot */
                if ((oy + borg.c.y == borg.c.y) && (ox + borg.c.x == borg.c.x))
                    continue;

                /* Acquire location */
                ag = &borg_grids[oy + borg.c.y][ox + borg.c.x];

                /* skip non open doors */
                if (ag->feat != FEAT_OPEN)
                    continue;

                /* skip monster on door */
                if (ag->kill)
                    continue;

                /* Skip repeatedly closed doors */
                if (track_door.num >= 255)
                    continue;

                /* skip our original goal */
                if ((oy + borg.c.y == y2) && (ox + borg.c.x == x2))
                    continue;

                /* save this spot */
                o_y = oy;
                o_x = ox;
                door_found++;
            }
        }

        /* Is there a door to close? */
        if (door_found) {
            /* Get a direction, if possible */
            dir = borg_goto_dir(
                borg.c.y, borg.c.x, borg.c.y + o_y, borg.c.x + o_x);

            /* Obtain the destination */
            x = borg.c.x + ddx[dir];
            y = borg.c.y + ddy[dir];

            /* Hack -- set goal */
            borg.goal.g.x = x;
            borg.goal.g.y = y;

            /* Close */
            borg_note("# Closing a door");
            borg_keypress('c');
            borg_queue_direction(I2D(dir));

            /* Check for an existing flag */
            for (i = 0; i < track_door.num; i++) {
                /* Stop if we already new about this door */
                if ((track_door.x[i] == x) && (track_door.y[i] == y))
                    return true;
            }

            /* Track the newly closed door */
            if (i == track_door.num && i < track_door.size) {

                borg_note("# Noting the closing of a door.");
                track_door.num++;
                track_door.x[i] = x;
                track_door.y[i] = y;
            }
            return true;
        }
    }

    /* Stand stairs up */
    if (borg.goal.less) {

        /* Define the grid we are looking at to be our own grid */
        ag = &borg_grids[borg.c.y][borg.c.x];

        /* Up stairs. Cheat the game grid info in.
         * (cave_feat[borg.c.y][borg.c.x] == FEAT_LESS) */
        if (ag->feat == FEAT_LESS) {

            borg.goal.less   = false;
            borg_keypress('<');

            /* Success */
            return true;
        }
    }

    /* Get a direction, if possible */
    dir = borg_goto_dir(borg.c.y, borg.c.x, y2, x2);

    /* We have arrived */
    if (dir == 5)
        return false;

    /* Obtain the destination */
    x = borg.c.x + ddx[dir];
    y = borg.c.y + ddy[dir];

    /* Access the grid we are stepping on */
    ag = &borg_grids[y][x];

    /* Hack -- set goal */
    borg.goal.g.x = x;
    borg.goal.g.y = y;

    /* Monsters -- Attack */
    if (ag->kill) {
        borg_kill *kill = &borg_kills[ag->kill];

        /* can't attack someone if afraid! */
        if (borg.trait[BI_ISAFRAID] || borg.trait[BI_CRSFEAR])
            return false;

        /* Hack -- ignore Maggot until later.  */
        if ((rf_has(r_info[kill->r_idx].flags, RF_UNIQUE))
            && borg.trait[BI_CDEPTH] == 0 && borg.trait[BI_CLEVEL] < 5)
            return false;

        /* Message */
        borg_note(format("# Walking into a '%s' at (%d,%d)",
            r_info[kill->r_idx].name, kill->pos.y, kill->pos.x));

        /* Walk into it */
        if (my_no_alter) {
            borg_keypress(';');
            my_no_alter = false;
        } else {
            borg_keypress('+');
        }
        borg_keypress(I2D(dir));
        return true;
    }

    /* Objects -- Take */
    if (ag->take && borg_takes[ag->take].kind) {
        borg_take *take = &borg_takes[ag->take];

        /*** Handle Chests ***/
        /* The borg will cheat when it comes to chests.
         * He does not have to but it makes him faster and
         * it does not give him any information that a
         * person would not know by looking at the trap.
         * So there is no advantage to the borg.
         */

        if (strstr(take->kind->name, "chest")
            && !strstr(take->kind->name, "Ruined")) {
            struct object *o_ptr = borg_get_top_object(cave, loc(x2, y2));

            /* this should only happen when something picks up the chest */
            /* outside the borgs view.  */
            if (!o_ptr) {
                borg_delete_take(ag->take);
                return false;
            }

            /* Traps. Disarm it w/ fail check */
            if (o_ptr->pval > 1 && o_ptr->known
                && borg.trait[BI_DEV] - o_ptr->pval
                       >= borg_cfg[BORG_CHEST_FAIL_TOLERANCE]) {
                borg_note(format("# Disarming a '%s' at (%d,%d)",
                    take->kind->name, take->y, take->x));

                /* Open it */
                borg_keypress('D');
                borg_queue_direction(I2D(dir));
                return true;
            }

            /* No trap, or unknown trap that passed above checks - Open it */
            if (o_ptr->pval < 0 || !o_ptr->known) {
                borg_note(format("# Opening a '%s' at (%d,%d)",
                    take->kind->name, take->y, take->x));

                /* Open it */
                borg_keypress('o');
                borg_queue_direction(I2D(dir));
                return true;
            }

            /* Empty chest */
            /* continue in routine and pick it up */
        }

        /*** Handle Orb of Draining ***/

        /* Priest/Paladin borgs who have very limited ID ability can save some
         * money and inventory space my casting Orb of Draining on objects.
         * Cursed objects will melt under the Orb of Draining spell.  This will
         * save the borg from carrying the item around until he can ID it.
         *
         * By default, the flag ORBED is set to false when an item is created.
         * If the borg gets close to an item, and the conditions are favorable,
         * he will cast OoD on the item and change the flag.
         */
        if (take->orbed == false
            && (take->tval >= TV_SHOT && take->tval < TV_STAFF)) {
            if (distance(loc(take->x, take->y), borg.c) == 1) {
                if (borg_spell_okay_fail(ORB_OF_DRAINING, 25)) {
                    /* Target the Take location */
                    borg_target(loc(take->x, take->y));

                    /* Cast the prayer */
                    borg_spell(ORB_OF_DRAINING);

                    /* Message */
                    borg_note("# Orbing an object to check for cursed item.");

                    /* use the old target */
                    borg_keypress('5');

                    /* Change the take flag */
                    take->orbed = true;

                    /* check the blast radius of the prayer for other items */
                    for (i = 0; i < 24; i++) {
                        /* Extract the location */
                        int xx = take->x + borg_ddx_ddd[i];
                        int yy = take->y + borg_ddy_ddd[i];

                        /* Check the grid for a take */
                        if (!square_in_bounds_fully(cave, loc(xx, yy)))
                            continue;
                        ag2 = &borg_grids[yy][xx];
                        if (ag2->take) {
                            /* This item was orbed (mostly true)*/
                            borg_takes[borg_grids[yy][xx].take].orbed = true;
                        }
                    }

                    /* Return */
                    return true;
                }
            }
        }

        /*** Handle other takes ***/
        /* Message */
        borg_note(format("# Walking onto and deleting a '%s' at (%d,%d)",
            take->kind->name, take->y, take->x));

        /* Delete the item from the list */
        borg_delete_take(ag->take);

        /* Walk onto it */
        borg_keypress(I2D(dir));

        return true;
    }

    /* Glyph of Warding */
    if (ag->glyph) {
        /* Message */
        borg_note(format("# Walking onto a glyph of warding."));

        /* Walk onto it */
        borg_keypress(I2D(dir));
        return true;
    }

    /* Traps -- disarm -- */
    if (borg.trait[BI_CURLITE] && !borg.trait[BI_ISBLIND]
        && !borg.trait[BI_ISCONFUSED] && !scaryguy_on_level && ag->trap) {

        /* NOTE: If a scary guy is on the level, we allow the borg to run over
         * the trap in order to escape this level.
         */

        /* allow "destroy doors" */
        /* don't bother unless we are near full mana */
        if (borg.trait[BI_CURSP] > ((borg.trait[BI_MAXSP] * 4) / 5)) {
            if (borg_spell(DISABLE_TRAPS_DESTROY_DOORS)
                || borg_activate_item(act_disable_traps)) {
                borg_note("# Disable Traps, Destroy Doors");
                ag->trap = 0;
                /* since this just disables the trap and doesn't remove it, */
                /* don't rest next to it */
                borg.no_rest_prep = 3000;
                return true;
            }
        }

        /* Disarm */
        borg_note("# Disarming a trap");
        borg_keypress('D');
        borg_queue_direction(I2D(dir));

        /* We are not sure if the trap will get 'untrapped'. pretend it will*/
        ag->trap = 0;
        return true;
    }

    /* Closed Doors -- Open */
    if (ag->feat == FEAT_CLOSED) {
        /* Paranoia XXX XXX XXX */
        if (!randint0(100))
            return false;

        /* Not a good idea to open locked doors if a monster
         * is next to the borg beating on him
         */

        /* scan the adjacent grids */
        for (i = 0; i < 8; i++) {
            /* Grid in that direction */
            x = borg.c.x + ddx_ddd[i];
            y = borg.c.y + ddy_ddd[i];

            /* Access the grid */
            ag2 = &borg_grids[y][x];

            /* If monster adjacent to me and I'm weak, don't
             * even try to open the door
             */
            if (ag2->kill && borg.trait[BI_CLEVEL] < 15
                && !borg.trait[BI_ISAFRAID])
                return false;
        }

        /* Use other techniques from time to time */
        if (!randint0(100) || borg.time_this_panel >= 500) {
            /* Mega-Hack -- allow "destroy doors" */
            if (borg_spell(DISABLE_TRAPS_DESTROY_DOORS)
                || borg_activate_item(act_destroy_doors)) {
                borg_note("# Disable Traps, Destroy Doors");
                return true;
            }

            /* Mega-Hack -- allow "stone to mud" */
            if (borg_spell(TURN_STONE_TO_MUD) || borg_spell(SHATTER_STONE)
                || borg_activate_ring(sv_ring_digging)
                || borg_activate_item(act_stone_to_mud)) {
                borg_note("# Melting a door");
                borg_keypress(I2D(dir));

                /* Remove this closed door from the list.
                 * Its faster to clear all doors from the list
                 * then rebuild the list.
                 */
                if (track_closed.num) {
                    track_closed.num = 0;
                }
                return true;
            }
        }

        /* Open */
        if (my_need_alter) {
            borg_keypress('+');
            my_need_alter = false;
        } else {
            borg_note("# Opening a door");
            borg_keypress('o');
        }
        borg_queue_direction(I2D(dir));

        /* Remove this closed door from the list.
         * Its faster to clear all doors from the list
         * then rebuild the list.
         */
        if (track_closed.num) {
            track_closed.num = 0;
        }

        return true;
    }

    /* Rubble, Treasure, Seams, Walls -- Tunnel or Melt */
    /* HACK depends on FEAT order, kinda evil. */
    if (ag->feat >= FEAT_SECRET && ag->feat <= FEAT_GRANITE) {
        /* Don't dig walls and seams when exploring (do dig rubble) */
        if (ag->feat != FEAT_RUBBLE && borg.goal.type == GOAL_DARK)
            return false;

        /* Don't bother digging without sufficient dig ability */
        if (!borg_can_dig(false, ag->feat)) {
            borg.goal.type = 0;
            return false;
        }

        /* Use Stone to Mud when available */
        if (borg_spell(TURN_STONE_TO_MUD) || borg_spell(SHATTER_STONE)
            || borg_activate_ring(sv_ring_digging)
            || borg_activate_item(act_stone_to_mud)) {
            borg_note("# Melting a wall/etc");
            borg_keypress(I2D(dir));

            /* Forget number of mineral veins to force rebuild of vein list */
            track_vein.num = 0;

            return true;
        }

        /* Mega-Hack -- prevent infinite loops */
        if (randint0(500) <= 5 && !vault_on_level)
            return false;

        /* Switch to a digger if we have one is automatic */

        /* Dig */
        borg_note("# Digging through wall/etc");
        borg_keypress('T');
        borg_keypress(I2D(dir));

        /* Forget number of mineral veins to force rebuild of vein list */
        /* XXX Maybe only do this if successful? */
        track_vein.num = 0;

        return true;
    }

    /* Shops -- Enter */
    if (feat_is_shop(ag->feat)) {
        /* Message */
        borg_note(format("# Entering a '%d' shop", ag->store));

        /* Enter the shop */
        borg_keypress(I2D(dir));
        return true;
    }

    /* Walk in that direction */
    if (my_need_alter) {
        borg_keypress('+');
        my_need_alter = false;
    } else {
        /* nothing */
    }

    /* Actually enter the direction */
    borg_keypress(I2D(dir));

    /* I'm not in a store */
    borg.in_shop = false;

    /* for some reason, selling and buying in the store sets the event handler
     * to Select. This is a game bug not a borg bug.  The borg is trying to
     * overcome the game bug. But he still has some troubles unhooking in town
     * after shopping.  Again, this is due to the event handler.  The handler
     * should release the EVT_SELECT but it does not.
     */

    if (ch_evt.type & EVT_SELECT)
        ch_evt.type = EVT_KBRD;
    if (ch_evt.type & EVT_MOVE)
        ch_evt.type = EVT_KBRD;

    /* Did something */
    return true;
}

/*
 * Attempt to take an optimal step towards the current goal location
 *
 * Note that the "borg_update()" routine notices new monsters and objects,
 * and movement of monsters and objects, and cancels any flow in progress.
 *
 * Note that the "borg_update()" routine notices when a grid which was
 * not thought to block motion is discovered to in fact be a grid which
 * blocks motion, and removes that grid from any flow in progress.
 *
 * When given multiple alternative steps, this function attempts to choose
 * the "safest" path, by penalizing grids containing embedded gold, monsters,
 * rubble, doors, traps, store doors, and even floors.  This allows the Borg
 * to "step around" dangerous grids, even if this means extending the path by
 * a step or two, and encourages him to prefer grids such as objects and stairs
 * which are not only interesting but which are known not to be invisible traps.
 *
 * XXX XXX XXX XXX This function needs some work.  It should attempt to
 * analyze the "local region" around the player and determine the optimal
 * choice of locations based on some useful computations.
 *
 * If it works, return true, otherwise, cancel the goal and return false.
 */
bool borg_flow_old(int why)
{
    int x, y;

    /* Continue */
    if (borg.goal.type == why) {
        int b_n = 0;

        int i, b_i = -1;

        int c, b_c;

        /* Flow cost of current grid */
        b_c = borg_data_flow->data[borg.c.y][borg.c.x] * 10;

        /* Prevent loops */
        b_c = b_c - 5;

        /* Look around */
        for (i = 0; i < 8; i++) {
            /* Grid in that direction */
            x = borg.c.x + ddx_ddd[i];
            y = borg.c.y + ddy_ddd[i];

            /* Flow cost at that grid */
            c = borg_data_flow->data[y][x] * 10;

            /* Never backtrack */
            if (c > b_c)
                continue;

            /* avoid screen edges */
            if (x > AUTO_MAX_X - 1 || x < 1 || y > AUTO_MAX_Y - 1 || y < 1)
                continue;

            /* Notice new best value */
            if (c < b_c)
                b_n = 0;

            /* Apply the randomizer to equivalent values */
            if (borg.trait[BI_CDEPTH] == 0 && (++b_n >= 2)
                && (randint0(b_n) != 0))
                continue;
            else if (borg.trait[BI_CDEPTH] >= 1 && ++b_n >= 2)
                continue;

            /* Special case when digging anti-summon corridor */
            if (borg.goal.type == GOAL_DIGGING
                && (ddx_ddd[i] == 0 || ddy_ddd[i] == 0)) {
                /* No straight lines */
                if (distance(borg.c, loc(borg_flow_x[0], borg_flow_y[0])) <= 2)
                    continue;
            }

            /* Track it */
            b_i = i;
            b_c = c;
        }

        /* Try it */
        if (b_i >= 0) {
            /* Access the location */
            x = borg.c.x + ddx_ddd[b_i];
            y = borg.c.y + ddy_ddd[b_i];

            /* Attempt motion */
            if (borg_play_step(y, x))
                return true;
        }

        /* Mark a timestamp to wait on a anti-summon spot for a few turns */
        if (borg.goal.type == GOAL_DIGGING && borg.c.y == borg_flow_y[0]
            && borg.c.x == borg_flow_x[0])
            borg_t_antisummon = borg_t;

        /* Cancel goal */
        borg.goal.type = 0;
    }

    /* Nothing to do */
    return false;
}

/*
 * Initialize location tracking data
 */
void borg_init_track(struct borg_track *track, int size)
{
    track->num  = 0;
    track->size = size;
    track->x    = mem_zalloc(size * sizeof(int));
    track->y    = mem_zalloc(size * sizeof(int));
}

/*
 * Free the memory allocated to track locations
 */
void borg_free_track(struct borg_track *track)
{
    track->num  = 0;
    track->size = 0;
    mem_free(track->x);
    track->x = NULL;
    mem_free(track->y);
    track->y = NULL;
}

void borg_init_flow(void)
{
    int x, y;

    /*** Grid data ***/

    /* Allocate */
    borg_data_flow = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_cost = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_hard = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_know = mem_zalloc(sizeof(borg_data));

    /* Allocate */
    borg_data_icky = mem_zalloc(sizeof(borg_data));

    /* Prepare "borg_data_hard" */
    for (y = 0; y < AUTO_MAX_Y; y++) {
        for (x = 0; x < AUTO_MAX_X; x++) {
            /* Prepare "borg_data_hard" */
            borg_data_hard->data[y][x] = 255;
        }
    }

    /* Track Steps */
    borg_init_track(&track_step, 100);

    /* Track doors closed by borg */
    borg_init_track(&track_door, 100);

    /* Track closed doors on map */
    borg_init_track(&track_closed, 100);

    borg_init_flow_stairs();
    borg_init_flow_glyph();
    borg_init_flow_misc();
}

void borg_free_flow(void)
{
    borg_free_flow_misc();
    borg_free_flow_glyph();
    borg_free_flow_stairs();

    borg_free_track(&track_closed);
    borg_free_track(&track_door);
    borg_free_track(&track_step);

    mem_free(borg_data_icky);
    borg_data_icky = NULL;
    mem_free(borg_data_know);
    borg_data_know = NULL;
    mem_free(borg_data_hard);
    borg_data_hard = NULL;
    mem_free(borg_data_cost);
    borg_data_cost = NULL;
    mem_free(borg_data_flow);
    borg_data_flow = NULL;
}

#endif
