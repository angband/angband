
/* File: borg6.c */
/* Purpose: Medium level stuff for the Borg -BEN- */

#include "../angband.h"
#ifdef ALLOW_BORG

#include "../cave.h"
#include "../game-input.h"
#include "../mon-spell.h"
#include "../obj-knowledge.h"
#include "../obj-slays.h"
#include "../player-timed.h"
#include "../project.h"
#include "../trap.h"

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg4.h"
#include "borg5.h"
#include "borg6.h"
#include "borg7.h"

extern const int adj_str_hold[STAT_RANGE];
extern const int adj_str_blow[STAT_RANGE];

static bool borg_desperate = false;

static int borg_thrust_damage_one(int i);

/*
 * This file is responsible for the low level dungeon goals.
 *
 * This includes calculating the danger from monsters, determining
 * how and when to attack monsters, and calculating "flow" paths
 * from place to place for various reasons.
 *
 * Notes:
 *   We assume that invisible/offscreen monsters are dangerous
 *   We consider physical attacks, missile attacks, spell attacks,
 *     wand attacks, etc, as variations on a single theme.
 *   We take account of monster resistances and susceptibilities
 *   We try not to wake up sleeping monsters by throwing things
 *
 *
 * Bugs:
 */





 /*
  * Given a "source" and "target" locations, extract a "direction",
  * which will move one step from the "source" towards the "target".
  *
  * Note that we use "diagonal" motion whenever possible.
  *
  * We return "5" if no motion is needed.
  */
static int borg_extract_dir(int y1, int x1, int y2, int x2)
{
    /* No movement required */
    if ((y1 == y2) && (x1 == x2)) return (5);

    /* South or North */
    if (x1 == x2) return ((y1 < y2) ? 2 : 8);

    /* East or West */
    if (y1 == y2) return ((x1 < x2) ? 6 : 4);

    /* South-east or South-west */
    if (y1 < y2) return ((x1 < x2) ? 3 : 1);

    /* North-east or North-west */
    if (y1 > y2) return ((x1 < x2) ? 9 : 7);

    /* Paranoia */
    return (5);
}


/*
 * Given a "source" and "target" locations, extract a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * We prefer "non-diagonal" motion, which allows us to save the
 * "diagonal" moves for avoiding pillars and other obstacles.
 *
 * If no "obvious" path is available, we use "borg_extract_dir()".
 *
 * We return "5" if no motion is needed.
 */
static int borg_goto_dir(int y1, int x1, int y2, int x2)
{
    int d, e;

    int ay = (y2 > y1) ? (y2 - y1) : (y1 - y2);
    int ax = (x2 > x1) ? (x2 - x1) : (x1 - x2);


    /* Default direction */
    e = borg_extract_dir(y1, x1, y2, x2);


    /* Adjacent location, use default */
    if ((ax <= 1) && (ay <= 1)) return (e);


    /* Try south/north (primary) */
    if (ay > ax)
    {
        d = (y1 < y2) ? 2 : 8;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);
    }

    /* Try east/west (primary) */
    if (ay < ax)
    {
        d = (x1 < x2) ? 6 : 4;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);
    }


    /* Try diagonal */
    d = borg_extract_dir(y1, x1, y2, x2);

    /* Check for walls */
    if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);


    /* Try south/north (secondary) */
    if (ay <= ax)
    {
        d = (y1 < y2) ? 2 : 8;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);
    }

    /* Try east/west (secondary) */
    if (ay >= ax)
    {
        d = (x1 < x2) ? 6 : 4;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);
    }


    /* Circle obstacles */
    if (!ay)
    {
        /* Circle to the south */
        d = (x1 < x2) ? 3 : 1;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);

        /* Circle to the north */
        d = (x1 < x2) ? 9 : 7;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);
    }

    /* Circle obstacles */
    if (!ax)
    {
        /* Circle to the east */
        d = (y1 < y2) ? 3 : 9;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);

        /* Circle to the west */
        d = (y1 < y2) ? 1 : 7;
        if (borg_cave_floor_bold(y1 + ddy[d], x1 + ddx[d])) return (d);
    }


    /* Oops */
    return (e);
}



/*
 * Clear the "flow" information
 *
 */
static void borg_flow_clear(void)
{
    /* Reset the "cost" fields */
    memcpy(borg_data_cost, borg_data_hard, sizeof(borg_data));

    /* Wipe costs and danger */
    if (borg_danger_wipe)
    {
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
 * enqueue "dangeous" destination grids, but does not need to set
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
 * nor to Monster grids if borg_desperate or borg_lunal_mode are
 * set.
 *
 * "Sneak" will have the borg avoid grids which are adjacent to a monster.
 *
 */
static void borg_flow_spread(int depth, bool optimize, bool avoid, bool tunneling, int stair_idx, bool sneak)
{
    int i;
    int n, o = 0;
    int x1, y1;
    int x, y;
    int fear = 0;
    int ii;
    int yy, xx;
    bool bad_sneak = false;
    int origin_y, origin_x;
    bool twitchy = false;

    /* Default starting points */
    origin_y = c_y;
    origin_x = c_x;

    /* Is the borg moving under boosted bravery? */
    if (avoidance > borg_skill[BI_CURHP]) twitchy = true;

    /* Use the closest stair for calculation distance (cost) from the stair to the goal */
    if (stair_idx >= 0 && borg_skill[BI_CLEVEL] < 15)
    {
        origin_y = track_less.y[stair_idx];
        origin_x = track_less.x[stair_idx];
        optimize = false;
    }

    /* Now process the queue */
    while (flow_head != flow_tail)
    {
        /* Extract the next entry */
        x1 = borg_flow_x[flow_tail];
        y1 = borg_flow_y[flow_tail];

        /* Circular queue -- dequeue the next entry */
        if (++flow_tail == AUTO_FLOW_MAX) flow_tail = 0;


        /* Cost (one per movement grid) */
        n = borg_data_cost->data[y1][x1] + 1;

        /* New depth */
        if (n > o)
        {
            /* Optimize (if requested) */
            if (optimize && (n > borg_data_cost->data[origin_y][origin_x])) break;

            /* Limit depth */
            if (n > depth) break;

            /* Save */
            o = n;
        }

        /* Queue the "children" */
        for (i = 0; i < 8; i++)
        {
            int old_head;

            borg_grid* ag;


            /* reset bad_sneak */
            bad_sneak = false;

            /* Neighbor grid */
            x = x1 + ddx_ddd[i];
            y = y1 + ddy_ddd[i];


            /* only on legal grids */
            if (!square_in_bounds_fully(cave, loc(x, y))) continue;

            /* Skip "reached" grids */
            if (borg_data_cost->data[y][x] <= n) continue;


            /* Access the grid */
            ag = &borg_grids[y][x];


            if (sneak)
            {
                /* Scan the neighbors */
                for (ii = 0; ii < 8; ii++)
                {
                    /* Neighbor grid */
                    xx = x + ddx_ddd[ii];
                    yy = y + ddy_ddd[ii];


                    /* only on legal grids */
                    if (!square_in_bounds_fully(cave, loc(xx, yy))) continue;

                    /* Make sure no monster is on this grid, which is
                     * adjacent to the grid on which, I am thinking about stepping.
                     */
                    if (borg_grids[yy][xx].kill)
                    {
                        bad_sneak = true;
                        break;
                    }
                }
            }
            /* The grid I am thinking about is adjacent to a monster */
            if (sneak && bad_sneak && !borg_desperate && !twitchy) continue;

            /* Avoid "wall" grids (not doors) unless tunneling*/
            /* HACK depends on FEAT order, kinda evil */
            if (!tunneling && (ag->feat >= FEAT_SECRET && 
                               ag->feat != FEAT_PASS_RUBBLE && 
                               ag->feat != FEAT_LAVA)) continue;

            /* Avoid "perma-wall" grids */
            if (ag->feat == FEAT_PERM) continue;

            /* Avoid "Lava" grids (for now) */
            if (ag->feat == FEAT_LAVA && !borg_skill[BI_IFIRE]) continue;

            /* Avoid unknown grids (if requested or retreating)
             * unless twitchy.  In which case, expore it
             */
            if ((avoid || borg_desperate) && (ag->feat == FEAT_NONE) &&
                !twitchy) continue;

            /* Avoid Monsters if Desprerate, lunal */
            if ((ag->kill) && (borg_desperate || borg_lunal_mode || borg_munchkin_mode)) continue;

            /* Avoid Monsters if low level, unless twitchy */
            if ((ag->kill) && !twitchy &&
                borg_skill[BI_FOOD] >= 2 && borg_skill[BI_MAXCLEVEL] < 5) continue;

            /* Avoid shop entry points if I am not heading to that shop */
            if (goal_shop >= 0 && feat_is_shop(ag->feat) &&
                (ag->store != goal_shop) && y != c_y && x != c_x) continue;


            /* Avoid Traps if low level-- unless brave */
            if (ag->trap && !ag->glyph && !twitchy)
            {
                /* Do not disarm when you could end up dead */
                if (borg_skill[BI_CURHP] < 60) continue;

                /* Do not disarm when clumsy */
                /* since traps can be physical or magical, gotta check both */
                if (borg_skill[BI_DISP] < 30 && borg_skill[BI_CLEVEL] < 20) continue;
                if (borg_skill[BI_DISP] < 45 && borg_skill[BI_CLEVEL] < 10) continue;
                if (borg_skill[BI_DISM] < 30 && borg_skill[BI_CLEVEL] < 20) continue;
                if (borg_skill[BI_DISM] < 45 && borg_skill[BI_CLEVEL] < 10) continue;

                /* NOTE:  Traps are tough to deal with as a low
                 * level character.  If any modifications are made above,
                 * then the same changes must be made to borg_flow_direct()
                 * and borg_flow_interesting()
                 */
            }

            /* Ignore "icky" grids */
            if (borg_data_icky->data[y][x]) continue;


            /* Analyze every grid once */
            if (!borg_data_know->data[y][x])
            {
                int p;


                /* Mark as known */
                borg_data_know->data[y][x] = true;

                if (!borg_desperate && !borg_lunal_mode && !borg_munchkin_mode && !borg_digging)
                {
                    /* Get the danger */
                    p = borg_danger(y, x, 1, true, false);

                    /* Increase bravery */
                    if (borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 5 / 10;
                    if (borg_skill[BI_MAXCLEVEL] != 50) fear = avoidance * 3 / 10;
                    if (scaryguy_on_level) fear = avoidance * 2;
                    if (unique_on_level && vault_on_level && borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 3;
                    if (scaryguy_on_level && borg_skill[BI_CLEVEL] <= 5) fear = avoidance * 3;
                    if (goal_ignoring) fear = avoidance * 5;
                    if (borg_t - borg_began > 5000) fear = avoidance * 25;
                    if (borg_skill[BI_FOOD] == 0) fear = avoidance * 100;

                    /* Normal in town */
                    if (borg_skill[BI_CLEVEL] == 0) fear = avoidance * 3 / 10;

                    /* Dangerous grid */
                    if (p > fear)
                    {
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
static void borg_flow_enqueue_grid(int y, int x)
{
    int old_head;
    int fear = 0;
    int p;

    /* Avoid icky grids */
    if (borg_data_icky->data[y][x]) return;

    /* Unknown */
    if (!borg_data_know->data[y][x])
    {
        /* Mark as known */
        borg_data_know->data[y][x] = true;

        /** Mark dangerous grids as icky **/

        /* Get the danger */
        p = borg_danger(y, x, 1, true, false);

        /* Increase bravery */
        if (borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 5 / 10;
        if (borg_skill[BI_MAXCLEVEL] != 50) fear = avoidance * 3 / 10;
        if (scaryguy_on_level) fear = avoidance * 2;
        if (unique_on_level && vault_on_level && borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 3;
        if (scaryguy_on_level && borg_skill[BI_CLEVEL] <= 5) fear = avoidance * 3;
        if (goal_ignoring) fear = avoidance * 5;
        if (borg_t - borg_began > 5000) fear = avoidance * 25;
        if (borg_skill[BI_FOOD] == 0) fear = avoidance * 100;

        /* Normal in town */
        if (borg_skill[BI_CLEVEL] == 0) fear = avoidance * 3 / 10;

        /* Dangerous grid */
        if ((p > fear) &&
            !borg_desperate && !borg_lunal_mode && !borg_munchkin_mode && !borg_digging)
        {
            /* Icky */
            borg_data_icky->data[y][x] = true;

            /* Avoid */
            return;
        }
    }


    /* Only enqueue a grid once */
    if (!borg_data_cost->data[y][x]) return;


    /* Save the flow cost (zero) */
    borg_data_cost->data[y][x] = 0;

    /* Enqueue that entry */
    borg_flow_y[flow_head] = y;
    borg_flow_x[flow_head] = x;


    /* Circular queue -- memorize head */
    old_head = flow_head;

    /* Circular queue -- insert with wrap */
    if (++flow_head == AUTO_FLOW_MAX) flow_head = 0;

    /* Circular queue -- handle overflow */
    if (flow_head == flow_tail) flow_head = old_head;
}


/* Do a Stair-Flow.  Look at how far away this grid is to my closest stair */
static int borg_flow_cost_stair(int y, int x, int b_stair)
{
    int cost = 255;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Paranoid */
    if (b_stair == -1) return (0);

    /* Enqueue the player's grid */
    borg_flow_enqueue_grid(track_less.y[b_stair], track_less.x[b_stair]);

    /* Spread, but do NOT optimize */
    borg_flow_spread(250, false, false, false, b_stair, false);

    /* Distance from the grid to the stair */
    cost = borg_data_cost->data[y][x];

    return (cost);
}


/*
 * Do a "reverse" flow from the player outwards
 */
static void borg_flow_reverse(int depth, bool optimize, bool avoid, bool tunneling, int stair_idx, bool sneak)
{
    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the player's grid */
    borg_flow_enqueue_grid(c_y, c_x);

    /* Spread, but do NOT optimize */
    borg_flow_spread(depth, optimize, avoid, tunneling, stair_idx, sneak);
}





/*
 * Attempt to induce WORD_OF_RECALL
 * artifact activations added throughout this code
 */
bool borg_recall(void)
{
    /* Multiple "recall" fails */
    if (!goal_recalling)
    {
        /* Try to "recall" */
        if (borg_zap_rod(sv_rod_recall) ||
            borg_activate_item(act_recall) ||
            borg_spell_fail(WORD_OF_RECALL, 60) ||
            borg_read_scroll(sv_scroll_word_of_recall))
        {
            /* Do reset depth at certain times. */
            if (borg_skill[BI_CDEPTH] < borg_skill[BI_MAXDEPTH] &&
                ((borg_skill[BI_MAXDEPTH] >= 60 && borg_skill[BI_CDEPTH] >= 40) ||
                    (borg_skill[BI_CLEVEL] < 48 && borg_skill[BI_CDEPTH] >= borg_skill[BI_MAXDEPTH] - 3) ||
                    (borg_skill[BI_CLEVEL] < 48 && borg_skill[BI_CDEPTH] >= 15 &&
                        borg_skill[BI_MAXDEPTH] - borg_skill[BI_CDEPTH] > 10)))
            {
                /* Special check on deep levels */
                if (borg_skill[BI_CDEPTH] >= 80 && borg_skill[BI_CDEPTH] < 100 && /* Deep */
                    borg_race_death[546] != 0) /* Sauron is Dead */
                {
                    /* Do reset Depth */
                    borg_note("# Resetting recall depth.");
                    borg_keypress('y');
                }
                else if (goal_fleeing_munchkin == true)
                {
                    /* Do not reset Depth */
                    borg_note("# Resetting recall depth during munchkin mode.");
                    borg_keypress('y');
                }
                else if (borg_skill[BI_CDEPTH] >= 100 && !borg_skill[BI_KING])
                {
                    /* Do reset Depth */
                    borg_note("# Not Resetting recall depth.");
                    borg_keypress('n');
                }
                else
                {
                    /* Do reset Depth */
                    borg_note("# Resetting recall depth.");
                    borg_keypress('y');

                }
            }

            /* reset recall depth in dungeon? */
            else if (borg_skill[BI_CDEPTH] < borg_skill[BI_MAXDEPTH] &&
                borg_skill[BI_CDEPTH] != 0)
            {
                /* Do not reset Depth */
                borg_note("# Not resetting recall depth.");
                borg_keypress('n');
            }

            borg_keypress(ESCAPE);

            /* Success */
            return (true);
        }
    }

    /* Nothing */
    return (false);
}



/*
 * Prevent starvation by any means possible
 */
static bool borg_eat_food_any(void)
{
    int i;

    /* Scan the inventory for "normal" food */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD) continue;

        /* Eat something of that type */
        if (borg_eat_food(item->tval, item->sval)) return (true);
    }

    /* Scan the inventory for "okay" food */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD && item->tval != TV_MUSHROOM) continue;

        /* Skip non-food */
        if (!borg_obj_has_effect(item->kind, EF_NOURISH, 0) &&
            !borg_obj_has_effect(item->kind, EF_NOURISH, 2) &&
            !borg_obj_has_effect(item->kind, EF_NOURISH, 3))
            continue;

        /* Eat something of that type */
        if (borg_eat_food(item->tval, item->sval)) return (true);
    }

    /*
     * Try potions that can provide nutrition.  First try ones that are
     * pure nutrition without additional effects.
     */
    if (borg_quaff_potion(sv_potion_slime_mold)) return (true);
    /*
     * Then try those that, besides the nourishment, only have negative
     * effects.  But only try if there's protection against the negative effect.
     */
    if (((borg_skill[BI_FRACT]) && (borg_quaff_potion(sv_potion_sleep) ||
            borg_quaff_potion(sv_potion_slowness))) ||
            ((borg_skill[BI_RBLIND]) && (borg_quaff_potion(sv_potion_blindness))) ||
            ((borg_skill[BI_RCONF]) && (borg_quaff_potion(sv_potion_confusion))))
    {
        return (true);
    }
    /* Consume in order, when hurting */
    if ((borg_skill[BI_CURHP] < 4 ||
        (borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP])) &&
        (borg_quaff_potion(sv_potion_cure_light) ||
            borg_quaff_potion(sv_potion_cure_serious) ||
            borg_quaff_potion(sv_potion_cure_critical) ||
            borg_quaff_potion(sv_potion_healing)))
    {
        return (true);
    }

    /* Nothing */
    return (false);
}
/*
 * Hack -- evaluate the likelihood of the borg getting surrounded
 * by a bunch of monsters.  This is called from borg_danger() when
 * he looking for a strategic retreat.  It is hopeful that the borg
 * will see that several monsters are approaching him and he may
 * become surrouned then die.  This routine looks at near-by monsters
 * and determines the likelihood of him getting surrouned.
 */
static bool borg_surrounded(void)
{
    borg_kill* kill;
    struct monster_race* r_ptr;

    int safe_grids = 8;
    int non_safe_grids = 0;
    int monsters = 0;
    int adjacent_monsters = 0;

    int x9, y9, ax, ay, d;
    int i;

    /* Evaluate the local monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        kill = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        x9 = kill->x;
        y9 = kill->y;

        /* Distance components */
        ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
        ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* if the monster is too far then skip it. */
        if (d > 3) continue;

        /* if he cant see me then forget it.*/
        if (!borg_los(c_y, c_x, y9, x9)) continue;

        /* if asleep, don't consider this one */
        if (!kill->awake) continue;

        /* Monsters with Pass Wall are dangerous, no escape from them */
        if (rf_has(r_ptr->flags, RF_PASS_WALL)) continue;
        if (rf_has(r_ptr->flags, RF_KILL_WALL)) continue;

        /* Cant really run away from Breeders very well */
        /* if (rf_has(r_ptr->flags, RF_MULTIPLY)) continue; */

        /* Monsters who never move cant surround */
        /* if (rf_has(r_ptr->flags, RF_NEVER_MOVE)) continue; */

        /* keep track of monsters touching me */
        if (d == 1) adjacent_monsters++;

        /* Add them up. */
        monsters++;

    }

    /* Evaluate the Non Safe Grids, (walls, closed doors, traps, monsters) */
    for (i = 0; i < 8; i++)
    {
        int x = c_x + ddx_ddd[i];
        int y = c_y + ddy_ddd[i];

        /* Access the grid */
        borg_grid* ag = &borg_grids[y][x];

        /* Bound check */
        if (!square_in_bounds_fully(cave, loc(x, y))) continue;

        /* Skip walls/doors */
        if (!borg_cave_floor_grid(ag)) non_safe_grids++;

        /* Skip unknown grids */
        else if (ag->feat == FEAT_NONE) non_safe_grids++;

        /* Skip monster grids */
        else if (ag->kill) non_safe_grids++;

        /* Mega-Hack -- skip stores XXX XXX XXX */
        else if (feat_is_shop(ag->feat)) non_safe_grids++;

        /* Mega-Hack -- skip traps XXX XXX XXX */
        if (ag->trap && !ag->glyph) non_safe_grids++;

    }

    /* Safe grids are decreased */
    safe_grids = safe_grids - non_safe_grids;

    /* Am I in hallway? If so don't worry about it */
    if (safe_grids == 1 && adjacent_monsters == 1) return (false);

    /* I am likely to get surrouned */
    if (monsters > safe_grids)
    {
        borg_note(format("# Possibility of being surrounded (monsters/safegrids)(%d/%d)",
            monsters, safe_grids));

        /* The borg can get trapped by continuing to flee
         * into a dead-end.  So he needs to be able to trump this
         * routine.
         */
        if (goal_ignoring)
        {
            /* borg_note("# Ignoring the fact that I am surrounded.");
             * return (false);
             */
        }
        else return (true);
    }

    /* Probably will not be surrouned */
    return (false);
}

/*
 * Mega-Hack -- evaluate the "freedom" of the given location
 *
 * The theory is that often, two grids will have equal "danger",
 * but one will be "safer" than the other, perhaps because it
 * is closer to stairs, or because it is in a corridor, or has
 * some other characteristic that makes it "safer".
 *
 * Then, if the Borg is in danger, say, from a normal speed monster
 * which is standing next to him, he will know that walking away from
 * the monster is "pointless", because the monster will follow him,
 * but if the resulting location is "safer" for some reason, then
 * he will consider it.  This will allow him to flee towards stairs
 * in the town, and perhaps towards corridors in the dungeon.
 *
 * This method is used in town to chase the stairs.
 *
 * XXX XXX XXX We should attempt to walk "around" buildings.
 */
static int borg_freedom(int y, int x)
{
    int d, f = 0;

    /* Hack -- chase down stairs in town */
    if (!borg_skill[BI_CDEPTH] && track_more.num)
    {
        /* Love the stairs! */
        d = double_distance(y, x, track_more.y[0], track_more.x[0]);

        /* Proximity is good */
        f += (1000 - d);

        /* Close proximity is great */
        if (d < 4) f += (2000 - (d * 500));
    }

    /* Hack -- chase Up Stairs in dungeon */
    if (borg_skill[BI_CDEPTH] && track_less.num)
    {
        /* Love the stairs! */
        d = double_distance(y, x, track_less.y[0], track_less.x[0]);

        /* Proximity is good */
        f += (1000 - d);

        /* Close proximity is great */
        if (d < 4) f += (2000 - (d * 500));
    }

    /* Freedom */
    return (f);
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
static bool borg_happy_grid_bold(int y, int x)
{
    int i;

    borg_grid* ag = &borg_grids[y][x];

    /* Bounds Check */
    if (y >= DUNGEON_HGT - 2 || y <= 2 ||
        x >= DUNGEON_WID - 2 || x <= 2) return (false);

    /* Accept stairs */
    if (ag->feat == FEAT_LESS) return (true);
    if (ag->feat == FEAT_MORE) return (true);
    if (ag->glyph) return (true);
    if (ag->feat == FEAT_LAVA && !borg_skill[BI_IFIRE]) return (false);

    /* Hack -- weak/dark is very unhappy */
    if (borg_skill[BI_ISWEAK] || borg_skill[BI_CURLITE] == 0) return (false);

    /* Apply a control effect so that he does not get stuck in a loop */
    if ((borg_t - borg_began) >= 2000)  return (false);

    /* Case 1a: north-south corridor */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
        !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1) &&
        !borg_cave_floor_bold(y + 1, x - 1) && !borg_cave_floor_bold(y + 1, x + 1) &&
        !borg_cave_floor_bold(y - 1, x - 1) && !borg_cave_floor_bold(y - 1, x + 1))
    {
        /* Happy */
        return (true);
    }

    /* Case 1b: east-west corridor */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
        !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x) &&
        !borg_cave_floor_bold(y + 1, x - 1) && !borg_cave_floor_bold(y + 1, x + 1) &&
        !borg_cave_floor_bold(y - 1, x - 1) && !borg_cave_floor_bold(y - 1, x + 1))
    {
        /* Happy */
        return (true);
    }

    /* Case 1aa: north-south doorway */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
        !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1))
    {
        /* Happy */
        return (true);
    }

    /* Case 1ba: east-west doorway */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
        !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x))
    {
        /* Happy */
        return (true);
    }


    /* Case 2a: north pillar */
    if (!borg_cave_floor_bold(y - 1, x) &&
        borg_cave_floor_bold(y - 1, x - 1) &&
        borg_cave_floor_bold(y - 1, x + 1) &&
        borg_cave_floor_bold(y - 2, x))
    {
        /* Happy */
        return (true);
    }

    /* Case 2b: south pillar */
    if (!borg_cave_floor_bold(y + 1, x) &&
        borg_cave_floor_bold(y + 1, x - 1) &&
        borg_cave_floor_bold(y + 1, x + 1) &&
        borg_cave_floor_bold(y + 2, x))
    {
        /* Happy */
        return (true);
    }

    /* Case 2c: east pillar */
    if (!borg_cave_floor_bold(y, x + 1) &&
        borg_cave_floor_bold(y - 1, x + 1) &&
        borg_cave_floor_bold(y + 1, x + 1) &&
        borg_cave_floor_bold(y, x + 2))
    {
        /* Happy */
        return (true);
    }

    /* Case 2d: west pillar */
    if (!borg_cave_floor_bold(y, x - 1) &&
        borg_cave_floor_bold(y - 1, x - 1) &&
        borg_cave_floor_bold(y + 1, x - 1) &&
        borg_cave_floor_bold(y, x - 2))
    {
        /* Happy */
        return (true);
    }

    /* check for grids that have been stepped on before */
    for (i = 0; i < track_step.num; i++)
    {
        /* Enqueue the grid */
        if ((track_step.y[i] == y) &&
            (track_step.x[i] == x))
        {
            /* Recent step is good */
            if (i < 25)
            {
                return (true);
            }
        }
    }

    /* Not happy */
    return (false);
}

/* This will look down a hallway and possibly light it up using
 * the Light Beam mage spell.  This spell is mostly used when
 * the borg is moving through the dungeon under boosted bravery.
 * This will allow him to "see" if anyone is there.
 *
 * It might also come in handy if he's in a hallway and gets shot, or
 * if resting in a hallway.  He may want to cast it to make
 * sure no previously unknown monsters are in the hall.
 * NOTE:  ESP will alter the value of this spell.
 *
 * Borg has a problem when not on map centering mode and casting the beam
 * repeatedly, down or up when at the edge of a panel.
 */
bool borg_LIGHT_beam(bool simulation)
{
    int dir = 5;
    bool spell_ok = false;
    int i;
    bool blocked = false;

    borg_grid* ag = &borg_grids[c_y][c_x];

    /* Hack -- weak/dark is very unhappy */
    if (borg_skill[BI_ISWEAK]) return (false);

    /* Require the abilitdy */
    if (borg_spell_okay_fail(SPEAR_OF_LIGHT, 20) ||
        (-1 != borg_slot(TV_WAND, sv_wand_light) &&
            borg_items[borg_slot(TV_WAND, sv_wand_light)].pval) ||
        borg_equips_rod(sv_rod_light))
        spell_ok = true;

    /*** North Direction Test***/

    /* Quick Boundary check */
    if (c_y - borg_skill[BI_CURLITE] - 1 > 0)
    {
        /* Look just beyond my light */
        ag = &borg_grids[c_y - borg_skill[BI_CURLITE] - 1][c_x];

        /* Must be on the panel */
        if (panel_contains(c_y - borg_skill[BI_CURLITE] - 1, c_x))
        {
            /* Check each grid in our light radius along the course */
            for (i = 0; i <= borg_skill[BI_CURLITE]; i++)
            {
                if (borg_cave_floor_bold(c_y - i, c_x) &&
                    !borg_cave_floor_bold(c_y - borg_skill[BI_CURLITE] - 1, c_x) &&
                    ag->feat < FEAT_OPEN && blocked == false)
                {
                    /* note the direction */
                    dir = 8;
                }
                else
                {
                    dir = 5;
                    blocked = true;
                }
            }
        }
    }

    /*** South Direction Test***/

    /* Quick Boundary check */
    if (c_y + borg_skill[BI_CURLITE] + 1 < AUTO_MAX_Y && dir == 5)
    {
        /* Look just beyond my light */
        ag = &borg_grids[c_y + borg_skill[BI_CURLITE] + 1][c_x];

        /* Must be on the panel */
        if (panel_contains(c_y + borg_skill[BI_CURLITE] + 1, c_x))
        {
            /* Check each grid in our light radius along the course */
            for (i = 0; i <= borg_skill[BI_CURLITE]; i++)
            {
                if (borg_cave_floor_bold(c_y + i, c_x) && /* all floors */
                    !borg_cave_floor_bold(c_y + borg_skill[BI_CURLITE] + 1, c_x) &&
                    ag->feat < FEAT_OPEN && blocked == false)
                {
                    /* note the direction */
                    dir = 2;
                }
                else
                {
                    dir = 5;
                    blocked = true;
                }
            }
        }
    }

    /*** East Direction Test***/

    /* Quick Boundary check */
    if (c_x + borg_skill[BI_CURLITE] + 1 < AUTO_MAX_X && dir == 5)
    {
        /* Look just beyond my light */
        ag = &borg_grids[c_y][c_x + borg_skill[BI_CURLITE] + 1];

        /* Must be on the panel */
        if (panel_contains(c_y, c_x + borg_skill[BI_CURLITE] + 1))
        {
            /* Check each grid in our light radius along the course */
            for (i = 0; i <= borg_skill[BI_CURLITE]; i++)
            {
                if (borg_cave_floor_bold(c_y, c_x + i) && /* all floors */
                    !borg_cave_floor_bold(c_y, c_x + borg_skill[BI_CURLITE] + 1) &&
                    ag->feat < FEAT_OPEN && blocked == false)
                {
                    /* note the direction */
                    dir = 6;
                }
                else
                {
                    dir = 5;
                    blocked = true;
                }
            }
        }
    }

    /*** West Direction Test***/

    /* Quick Boundary check */
    if (c_x - borg_skill[BI_CURLITE] - 1 > 0 && dir == 5)
    {
        /* Look just beyond my light */
        ag = &borg_grids[c_y][c_x - borg_skill[BI_CURLITE] - 1];

        /* Must be on the panel */
        if (panel_contains(c_y, c_x - borg_skill[BI_CURLITE] - 1))
        {
            /* Check each grid in our light radius along the course */
            for (i = 1; i <= borg_skill[BI_CURLITE]; i++)
            {
                /* Verify that there are no blockers in my light radius and
                 * the 1st grid beyond my light is not a floor nor a blocker
                 */
                if (borg_cave_floor_bold(c_y, c_x - i) && /* all see through */
                    !borg_cave_floor_bold(c_y, c_x - borg_skill[BI_CURLITE] - 1) &&
                    ag->feat < FEAT_OPEN && blocked == false)
                {
                    /* note the direction */
                    dir = 4;
                }
                else
                {
                    dir = 5;
                    blocked = true;
                }
            }
        }
    }

    /* Dont do it if on the edge of shifting the panel. */
    if (dir == 5 || spell_ok == false || blocked == true ||
        (dir == 2 && (c_y == 18 || c_y == 19 ||
            c_y == 29 || c_y == 30 ||
            c_y == 40 || c_y == 41 ||
            c_y == 51 || c_y == 52)) ||
        (dir == 8 && (c_y == 13 || c_y == 14 ||
            c_y == 24 || c_y == 25 ||
            c_y == 35 || c_y == 36 ||
            c_y == 46 || c_y == 47)))
        return (false);

    /* simulation */
    if (simulation) return (true);

    /* cast the light beam */
    if (borg_spell_fail(SPEAR_OF_LIGHT, 20) ||
        borg_zap_rod(sv_rod_light) ||
        borg_aim_wand(sv_wand_light))
    {   /* apply the direction */
        borg_keypress(I2D(dir));
        borg_note("# Illuminating this hallway");
        return(true);
    }

    /* cant do it */
    return (false);
}

/*
 * Scan the monster lists for certain types of monster that we
 * should be concerned over.
 * This only works for monsters we know about.  If one of the
 * monsters around is misidentified then it may be a unique
 * and we wouldn't know.  Special consideration is given to Morgoth
 */
void borg_near_monster_type(int dist)
{
    borg_kill* kill;
    struct monster_race* r_ptr;

    int x9, y9, ax, ay, d;
    int i;
    int breeder_count = 0;

    /* reset the borg flags */
    borg_fighting_summoner = false;
    borg_fighting_unique = 0;
    borg_fighting_evil_unique = false;
    borg_kills_summoner = -1;


    /* Scan the monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        kill = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;


        /* Count breeders */
        if (rf_has(r_ptr->flags, RF_MULTIPLY)) breeder_count++;

        /*** Scan for Scary Guys ***/

        /* Do ScaryGuys now, before distance checks.  We are
         * Looking for scary guys on level, not scary guys
         * near me
         */

         /* run from certain scaries */
        if (borg_skill[BI_CLEVEL] <= 5 &&
            (strstr(r_ptr->name, "Squint"))) scaryguy_on_level = true;

        /* Mage and priest are extra fearful */
        if (borg_skill[BI_CLEVEL] <= 6 &&
            (borg_class == CLASS_MAGE ||
                borg_class == CLASS_PRIEST) &&
            (strstr(r_ptr->name, "Squint"))) scaryguy_on_level = true;

        /* run from certain dungeon scaries */
        if (borg_skill[BI_CLEVEL] <= 5 &&
            (strstr(r_ptr->name, "Grip") ||
                strstr(r_ptr->name, "Fang") ||
                strstr(r_ptr->name, "Small kobold"))) scaryguy_on_level = true;

        /* run from certain scaries */
        if (borg_skill[BI_CLEVEL] <= 8 &&
            (strstr(r_ptr->name, "Novice") ||
                strstr(r_ptr->name, "Kobold") ||
                strstr(r_ptr->name, "Kobold archer") ||
                strstr(r_ptr->name, "Jackal") ||
                strstr(r_ptr->name, "Shrieker") ||
                strstr(r_ptr->name, "Farmer Maggot") ||
                strstr(r_ptr->name, "Filthy street urchin") ||
                strstr(r_ptr->name, "Battle-scarred veteran") ||
                strstr(r_ptr->name, "Mean-looking mercenary"))) scaryguy_on_level = true;

        if (borg_skill[BI_CLEVEL] <= 15 &&
            (strstr(r_ptr->name, "Bullr") ||
                ((strstr(r_ptr->name, "Giant white mouse") ||
                    strstr(r_ptr->name, "White worm mass") ||
                    strstr(r_ptr->name, "Green worm mass")) && breeder_count >= borg_skill[BI_CLEVEL])))  scaryguy_on_level = true;

        if (borg_skill[BI_CLEVEL] <= 20 &&
            (strstr(r_ptr->name, "Cave spider") ||
                strstr(r_ptr->name, "Pink naga") ||
                strstr(r_ptr->name, "Giant pink frog") ||
                strstr(r_ptr->name, "Radiation eye") ||
                (strstr(r_ptr->name, "Yellow worm mass") && breeder_count >= borg_skill[BI_CLEVEL]))) scaryguy_on_level = true;

        if (borg_skill[BI_CLEVEL] < 45 &&
            (strstr(r_ptr->name, "Gravity") ||
                strstr(r_ptr->name, "Inertia") ||
                strstr(r_ptr->name, "Ancient") ||
                strstr(r_ptr->name, "Beorn") ||
                strstr(r_ptr->name, "Dread") /* Appear in Groups */)) scaryguy_on_level = true;

        /* Nether breath is bad */
        if (!borg_skill[BI_SRNTHR] &&
            (strstr(r_ptr->name, "Azriel") ||
                strstr(r_ptr->name, "Dracolich") ||
                strstr(r_ptr->name, "Dracolisk"))) scaryguy_on_level = true;

        /* Blindness is really bad */
        if ((!borg_skill[BI_SRBLIND]) &&
            ((strstr(r_ptr->name, "Light hound") && !borg_skill[BI_SRLITE]) ||
                (strstr(r_ptr->name, "Dark hound") && !borg_skill[BI_SRDARK]))) scaryguy_on_level = true;

        /* Chaos and Confusion are really bad */
        if ((!borg_skill[BI_SRKAOS] && !borg_skill[BI_SRCONF]) &&
            (strstr(r_ptr->name, "Chaos"))) scaryguy_on_level = true;
        if (!borg_skill[BI_SRCONF] &&
            (strstr(r_ptr->name, "Pukelman") ||
                strstr(r_ptr->name, "Nightmare"))) scaryguy_on_level = true;


        /* Poison is really Bad */
        if (!borg_skill[BI_RPOIS] && /* Note the RPois not SRPois */
            (strstr(r_ptr->name, "Drolem"))) scaryguy_on_level = true;


        /* Now do distance considerations */
        x9 = kill->x;
        y9 = kill->y;

        /* Distance components */
        ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
        ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* if the guy is too far then skip it unless in town. */
        if (d > dist && borg_skill[BI_CDEPTH]) continue;

        /* Special check here for Searching since we are
         * already scanning the monster list
         */
        if (borg_needs_searching)
        {
            if (d < 7) borg_needs_searching = false;
        }

        /*** Scan for Uniques ***/

        /* this is a unique. */
        if (rf_has(r_ptr->flags, RF_UNIQUE))

        {
            /* Set a flag for use with certain types of spells */
            unique_on_level = kill->r_idx;

            /* return 1 if not Morgy, +10 if it is Morgy or Sauron */
            if (rf_has(r_ptr->flags, RF_QUESTOR))
            {
                borg_fighting_unique += 10;
            }

            /* regular unique */
            borg_fighting_unique++;

            /* Note that fighting a Questor would result in a 11 value */
            if (rf_has(r_ptr->flags, RF_EVIL)) borg_fighting_evil_unique = true;

        }


        /*** Scan for Summoners ***/
        if ((rsf_has(r_ptr->spell_flags, RSF_S_KIN)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_HOUND)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_AINU)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_DEMON)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_HI_DRAGON)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH)) ||
            (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE)))
        {
            /* mark the flag */
            borg_fighting_summoner = true;

            /* recheck the distance to see if close
             * and mark the index for as-corridor
             */
            if (d < 8)
            {
                borg_kills_summoner = i;
            }
        }
    }
}

#ifdef INCLUDE_UNUSED_FUNCTIONS
static int borg_damage_and_power(int ag_kill, int x, int y)
{
    borg_kill* kill;
    int d, p;

    /* Calculate "average" damage */
    d = borg_thrust_damage_one(ag_kill);

    /* No damage */
    if (d <= 0) return 0;

    /* Obtain the monster */
    kill = &borg_kills[ag_kill];

    /* Hack -- avoid waking most "hard" sleeping monsters */
    if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
    {
        /* Calculate danger */
        p = borg_danger_aux(y, x, 1, ag_kill, true, true);

        if (p > avoidance * 2) return 0;
    }

    /* Hack -- ignore sleeping town monsters */
    if (!borg_skill[BI_CDEPTH] && !kill->awake) return 0;

    /* Calculate "danger" to player */
    p = borg_danger_aux(c_y, c_x, 2, ag_kill, true, true);

    /* Reduce "bonus" of partial kills when higher level */
    if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

    /* Add the danger-bonus to the damage */
    d += p;

    return d;
}
#endif

/*
 * Help determine if PHASE_DOOR seems like a good idea
 */
bool borg_caution_phase(int emergency, int turns)
{
    int n, k, i, d, x, y, p;

    int dis = 10;
    int min = dis / 2;

    borg_grid* ag = &borg_grids[c_y][c_x];


    /* must have the ability */
    if (!borg_skill[BI_APHASE]) return (false);

    /* Simulate 100 attempts */
    for (n = k = 0; k < 100; k++)
    {
        /* Pick a location */
        for (i = 0; i < 100; i++)
        {
            /* Pick a (possibly illegal) location */
            while (1)
            {
                y = rand_spread(c_y, dis);
                x = rand_spread(c_x, dis);
                d = borg_distance(c_y, c_x, y, x);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if ((y <= 0) || (y >= AUTO_MAX_Y - 1)) continue;
            if ((x <= 0) || (x >= AUTO_MAX_X - 1)) continue;

            /* Access */
            ag = &borg_grids[y][x];

            /* Skip unknown grids */
            if (ag->feat == FEAT_NONE) continue;

            /* Skip walls */
            if (!borg_cave_floor_bold(y, x)) continue;

            /* Skip monsters */
            if (ag->kill) continue;

            /* Stop looking */
            break;
        }

        /* If low level, unknown squares are scary */
        if (ag->feat == FEAT_NONE && borg_skill[BI_MAXHP] < 30)
        {
            n++;
            continue;
        }

        /* No location */
        /* in the real code it would keep trying but here we should */
        /* assume that there is unknown spots that you would be able */
        /* to go but may be dangerious. */
        if (i >= 100)
        {
            n++;
            continue;
        }

        /* Examine */
        p = borg_danger(y, x, turns, true, false);

        /* if *very* scary, do not allow jumps at all */
        if (p > borg_skill[BI_CURHP]) n++;
    }

    /* Too much danger */
    /* in an emergency try with extra danger allowed */
    if (n > emergency)
    {
        borg_note(format("# No Phase. scary squares: %d", n));
        return (false);
    }
    else
        borg_note(format("# Safe to Phase. scary squares: %d", n));

    /* Okay */
    return (true);
}
/*
 * Help determine if PHASE_DOOR with Shoot N Scoot seems like
 * a good idea.
 * Good Idea on two levels:
 * 1.  We are the right class, we got some good ranged weapons
 * 2.  The possible landing grids are ok.
 * Almost a copy of the borg_caution_phase above.
 * The emergency is the number of dangerous grids out of 100
 * that we tolerate.  If we have 80, then we accept the risk
 * of landing on a grid that is 80% likely to be bad.  A low
 * number, like 20, means that we are less like to risk the
 * phase door and we require more of the possible grids to be
 * safe.
 *
 * The pattern of ShootN'Scoot works like this:
 * 1. Shoot monster that is far away.
 * 2. Monsters walks closer and closer each turn
 * 3. Borg shoots monster each step it takes as it approaches.
 * 4. Monster gets within 1 grid of the borg.
 * 5. Borg phases away.
 * 6. Go back to #1
 */
static bool borg_shoot_scoot_safe(int emergency, int turns, int b_p)
{
    int n, k, i, d, x, y, p, u;

    int dis = 10;

    int min = dis / 2;

    bool adjacent_monster = false;

    borg_grid* ag;
    borg_kill* kill;
    struct monster_race* r_ptr;

    /* no need if high level in town */
    if (borg_skill[BI_CLEVEL] >= 8 &&
        borg_skill[BI_CDEPTH] == 0) return (false);

    /* must have the ability */
    if (!borg_skill[BI_APHASE]) return (false);

    /* Not if No Light */
    if (!borg_skill[BI_CURLITE]) return (false);

    /* Cheat the floor grid */
    /* Not if in a vault since it throws us out of the vault */
    if (square_isvault(cave, loc(c_x, c_y))) return (false);

    /*** Need Missiles or cheap spells ***/

    /* classes that are mainly spellcaster */
    if (player->class->magic.num_books > 3)
    {
        /* Low mana */
        if (borg_skill[BI_CLEVEL] >= 45 &&
            borg_skill[BI_CURSP] < 15) return (false);

        /* Low mana, low level, generally OK */
        if (borg_skill[BI_CLEVEL] < 45 &&
            borg_skill[BI_CURSP] < 5) return (false);
    }
    else /* Other classes need some missiles */
    {
        if (borg_skill[BI_AMISSILES] < 5 || borg_skill[BI_CLEVEL] >= 45) return (false);
    }

    /* Not if I am in a safe spot for killing special monsters */
    if (borg_morgoth_position || borg_as_position) return (false);

    /* scan the adjacent grids for an awake monster */
    for (i = 0; i < 8; i++)
    {
        /* Grid in that direction */
        x = c_x + ddx_ddd[i];
        y = c_y + ddy_ddd[i];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];
        r_ptr = &r_info[kill->r_idx];

        /* If a qualifying monster is adjacent to me. */
        if ((ag->kill && kill->awake) &&
            !(rf_has(r_ptr->flags, RF_NEVER_MOVE)) &&
            !(rf_has(r_ptr->flags, RF_PASS_WALL)) &&
            !(rf_has(r_ptr->flags, RF_KILL_WALL)) &&
            (kill->power >= borg_skill[BI_CLEVEL]))
        {
            /* Spell casters shoot at everything */
            if (borg_spell_okay(MAGIC_MISSILE))
            {
                adjacent_monster = true;
            }
            else if (borg_spell_okay(ORB_OF_DRAINING))
            {
                adjacent_monster = true;
            }
            else if (borg_spell_okay(NETHER_BOLT))
            {
                adjacent_monster = true;
            }

            /* All other borgs need to make sure he would shoot.
             * In an effort to conserve missiles, the borg will
             * not shoot at certain types of monsters.  That list
             * is defined in borg_launch_damage_one().
             *
             * We need this aforementioned list to match the one
             * following.  Otherwise Rogues and Warriors will
             * burn up Phases as he scoots away but never fire
             * the missiles.  That totally defeats the purpose
             * of this routine.
             *
             * The following criteria are exactly the same as the
             * list in borg_launch_damage_one()
             */
            else if ((borg_danger_aux(kill->y, kill->x, 1, i, true, false) > avoidance * 3 / 10) ||
                ((r_ptr->friends || r_ptr->friends_base) /* monster has friends*/ &&
                    kill->level >= borg_skill[BI_CLEVEL] - 5 /* close levels */) ||
                (kill->ranged_attack /* monster has a ranged attack */) ||
                (rf_has(r_ptr->flags, RF_UNIQUE)) ||
                (rf_has(r_ptr->flags, RF_MULTIPLY)) ||
                (borg_skill[BI_CLEVEL] <= 5 /* stil very weak */))
            {
                adjacent_monster = true;
            }
        }
    }

    /* if No Adjacent_monster no need for it */
    if (adjacent_monster == false) return (false);

    /* Simulate 100 attempts */
    for (n = k = 0; k < 100; k++)
    {
        /* Pick a location */
        for (i = 0; i < 100; i++)
        {
            /* Pick a (possibly illegal) location */
            while (1)
            {
                y = rand_spread(c_y, dis);
                x = rand_spread(c_x, dis);
                d = borg_distance(c_y, c_x, y, x);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if ((y <= 0) || (y >= AUTO_MAX_Y - 2)) continue;
            if ((x <= 0) || (x >= AUTO_MAX_X - 2)) continue;

            /* Access */
            ag = &borg_grids[y][x];

            /* Skip unknown grids */
            if (ag->feat == FEAT_NONE) continue;

            /* Skip walls */
            if (!borg_cave_floor_bold(y, x)) continue;

            /* Skip monsters */
            if (ag->kill) continue;


            /* Stop looking.  Really, the game would keep
             * looking for a grid.  The borg could check
             * all the known grids but I dont think that
             * is not a good idea, especially if the area is
             * not fully explored.
             */
            break;
        }

        /* No location */
        /* In the real code it would keep trying but here we should */
        /* assume that there is unknown spots that you would be able */
        /* to go but we define it as dangerous. */
        if (i >= 100)
        {
            n++;
            continue;
        }

        /* Examine danger of that grid */
        p = borg_danger(y, x, turns, true, false);

        /* if more scary than my current one, do not allow jumps at all */
        if (p > b_p)
        {
            n++;
            continue;
        }

        /* Should not land next to a monster either.
         * Scan the adjacent grids for a monster.
         * Reuse the adjacent_monster variable.
         */
        for (u = 0; u < 8; u++)
        {
            /* Access the grid */
            ag = &borg_grids[y + ddy_ddd[u]][x + ddx_ddd[u]];

            /* Obtain the monster */
            kill = &borg_kills[ag->kill];

            /* If monster adjacent to that grid...
             */
            if (ag->kill && kill->awake) n++;
        }

    }

    /* Too much danger */
    /* in an emergency try with extra danger allowed */
    if (n > emergency)
    {
        borg_note(format("# No Shoot'N'Scoot. scary squares: %d/100", n));
        return (false);
    }
    else
        borg_note(format("# Safe to Shoot'N'Scoot. scary squares: %d/100", n));

    /* Okay */
    return (true);
}
/*
 * Help determine if "Teleport" seems like a good idea
 */
static bool borg_caution_teleport(int emergency, int turns)
{
    int n, k, i, d, x, y, p;

    int dis = 100;
    int min = dis / 2;
    int q_x, q_y;


    borg_grid* ag = &borg_grids[c_y][c_x];

    /* Extract panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();

    /* must have the ability */
    if (!borg_skill[BI_ATELEPORT] || !borg_skill[BI_AESCAPE]) return (false);

    /* Simulate 100 attempts */
    for (n = k = 0; k < 100; k++)
    {
        /* Pick a location */
        for (i = 0; i < 100; i++)
        {
            /* Pick a (possibly illegal) location */
            while (1)
            {
                y = rand_spread(c_y, dis);
                x = rand_spread(c_x, dis);
                d = borg_distance(c_y, c_x, y, x);
                if ((d >= min) && (d <= dis)) break;
            }

            /* Ignore illegal locations */
            if ((y <= 0) || (y >= AUTO_MAX_Y - 1)) continue;
            if ((x <= 0) || (x >= AUTO_MAX_X - 1)) continue;

            /* Access */
            ag = &borg_grids[y][x];

            /* Skip unknown grids if explored, or been on level for a while, otherwise, consider ok*/
            if (ag->feat == FEAT_NONE &&
                ((borg_detect_wall[q_y + 0][q_x + 0] == true &&
                    borg_detect_wall[q_y + 0][q_x + 1] == true &&
                    borg_detect_wall[q_y + 1][q_x + 0] == true &&
                    borg_detect_wall[q_y + 1][q_x + 1] == true) ||
                    borg_t > 2000)) continue;

            /* Skip walls */
            if (!borg_cave_floor_bold(y, x)) continue;

            /* Skip monsters */
            if (ag->kill) continue;

            /* Stop looking */
            break;
        }

        /* If low level, unknown squares are scary */
        if (ag->feat == FEAT_NONE && borg_skill[BI_MAXHP] < 30)
        {
            n++;
            continue;
        }

        /* No location */
        /* in the real code it would keep trying but here we should */
        /* assume that there is unknown spots that you would be able */
        /* to go but may be dangerious. */
        if (i >= 100)
        {
            n++;
            continue;
        }

        /* Examine */
        p = borg_danger(y, x, turns, true, false);

        /* if *very* scary, do not allow jumps at all */
        if (p > borg_skill[BI_CURHP]) n++;
    }

    /* Too much danger */
    /* in an emergency try with extra danger allowed */
    if (n > emergency)
    {
        borg_note(format("# No Teleport. scary squares: %d", n));
        return (false);
    }
    /* Okay */
    return (true);
}
/*
 * Hack -- If the borg is standing on a stair and is in some danger, just leave the level.
 * No need to hang around on that level, try conserving the teleport scrolls
 */
static bool borg_escape_stair(void)
{
    /* Current grid */
    borg_grid* ag = &borg_grids[c_y][c_x];

    /* Usable stairs */
    if (ag->feat == FEAT_LESS)
    {
        /* Take the stairs */
        borg_on_dnstairs = true;
        borg_note("# Escaping level via stairs.");
        borg_keypress('<');

        /* Success */
        return (true);
    }

    return (false);
}

bool borg_allow_teleport(void)
{
    /* No teleporting in arena levels */
    if (player->upkeep->arena_level) return false;

    /* Check for a no teleport grid */
    if (square_isno_teleport(cave, loc(c_x, c_y)))
        return false;

    /* Check for a no teleport curse */
    if (borg_skill[BI_CRSNOTEL])
        return false;

    return true;
}

/* short range teleport + pain */
bool borg_shadow_shift(int allow_fail)
{
    /* disallow if hp too low */
    if (borg_skill[BI_CURHP] < 12)
        return (false);
    return borg_spell_fail(SHADOW_SHIFT, allow_fail);
}

bool borg_dimension_door(int allow_fail)
{
    int x_off, y_off;
    int t_x, t_y;
    int best_t_x, best_t_y;
    int d, best_d = 0;
    struct loc target;

    /* for now keep the range at under 50, for performance */
    int range = 50;

    /* Require ability (right now) */
    if (!borg_spell_okay_fail(DIMENSION_DOOR, allow_fail)) return (0);

    /* if we are attacking, calculate gains, but if this is just a teleport */
    /* the current danger is the starting point */
    best_d = borg_fear_region[c_y][c_x];

    /* Pick a location */
    for (x_off = range * -1; x_off < range; x_off++)
    {
        for (y_off = range * -1; y_off < range; y_off++)
        {
            t_x = c_x + x_off;
            t_y = c_y + y_off;

            if (t_x < 0 || t_y < 0) continue;

            target = loc(c_x + x_off, c_y + y_off);

            if (!square_in_bounds_fully(cave, target)) continue;

            d = borg_danger(t_y, t_x, 2, true, false);
            if (d < best_d)
            {
                best_d = d;
                best_t_x = t_x;
                best_t_y = t_y;
            }
        }
    }

    if (best_d < borg_fear_region[c_y][c_x])
    {
        borg_target(best_t_y, best_t_x);

        borg_spell(DIMENSION_DOOR);

        /* pick target */
        borg_keypress('5');
        return true;
    }
    return false;
}

/*
 * Try to phase door or teleport
 * b_q is the danger of the least dangerious square around us.
 */
static bool borg_escape(int b_q)
{

    int risky_boost = 0;
    int j;
    int glyphs = 0;

    borg_grid* ag;

    /* only escape with spell if fail is low */
    int allow_fail = 25;
    int sv_mana;

    /* if very healthy, allow extra fail */
    if (((borg_skill[BI_CURHP] * 100) / borg_skill[BI_MAXHP]) > 70)
        allow_fail = 10;

    /* comprimised, get out of the fight */
    if (borg_skill[BI_ISHEAVYSTUN])
        allow_fail = 35;

    /* for emergencies */
    sv_mana = borg_skill[BI_CURSP];

    /* Borgs who are bleeding to death or dying of poison may sometimes
     * phase around the last two hit points right before they enter a
     * shop.  He knows to make a bee-line for the temple but the danger
     * trips this routine.  So we must bypass this routine for some
     * particular circumstances.
     */
    if (!borg_skill[BI_CDEPTH] && (borg_skill[BI_ISPOISONED] || borg_skill[BI_ISWEAK] || borg_skill[BI_ISCUT])) return (false);

    /* Borgs who are in a sea of runes or trying to build one
     * and mostly healthy stay put
     */
    if ((borg_skill[BI_CDEPTH] == 100) &&
        borg_skill[BI_CURHP] >= (borg_skill[BI_MAXHP] * 5 / 10))
    {
        /* In a sea of runes */
        if (borg_morgoth_position)
            return (false);

        /* Scan neighbors */
        for (j = 0; j < 8; j++)
        {
            int y = c_y + ddy_ddd[j];
            int x = c_x + ddx_ddd[j];

            /* Get the grid */
            ag = &borg_grids[y][x];

            /* Skip unknown grids (important) */
            if (ag->glyph) glyphs++;
        }
        /* Touching at least 3 glyphs */
        if (glyphs >= 3) return (false);
    }

    /* Hack -- If the borg is weak (no food, starving) on depth 1 and he has no idea where the stairs
     * may be, run the risk of diving deeper against the benefit of rising to town.
     */
    if (borg_skill[BI_ISWEAK] && borg_skill[BI_CDEPTH] == 1)
    {
        if (borg_read_scroll(sv_scroll_teleport_level))
        {
            borg_note("# Attempting to get to town immediately");
            return (true);
        }
    }

    /* Risky borgs are more likely to stay in a fight */
    if (borg_cfg[BORG_PLAYS_RISKY]) risky_boost = 3;

    /* 1. really scary, I'm about to die */
    /* Try an emergency teleport, or phase door as last resort */
    if (borg_skill[BI_ISHEAVYSTUN] ||
        (b_q > avoidance * (45 + risky_boost) / 10) ||
        ((b_q > avoidance * (40 + risky_boost) / 10) && borg_fighting_unique >= 10 && borg_skill[BI_CDEPTH] == 100 && borg_skill[BI_CURHP] < 600) ||
        ((b_q > avoidance * (30 + risky_boost) / 10) && borg_fighting_unique >= 10 && borg_skill[BI_CDEPTH] == 99 && borg_skill[BI_CURHP] < 600) ||
        ((b_q > avoidance * (25 + risky_boost) / 10) && borg_fighting_unique >= 1 && borg_fighting_unique <= 8 && borg_skill[BI_CDEPTH] >= 95 && borg_skill[BI_CURHP] < 550) ||
        ((b_q > avoidance * (17 + risky_boost) / 10) && borg_fighting_unique >= 1 && borg_fighting_unique <= 8 && borg_skill[BI_CDEPTH] < 95) ||
        ((b_q > avoidance * (15 + risky_boost) / 10) && !borg_fighting_unique))
    {

        int tmp_allow_fail = 15;


        if (borg_escape_stair() ||
            (borg_allow_teleport() &&
                (borg_dimension_door(tmp_allow_fail - 10) ||
                    borg_spell_fail(TELEPORT_SELF, tmp_allow_fail - 10) ||
                    borg_spell_fail(PORTAL, tmp_allow_fail - 10) ||
                    borg_shadow_shift(tmp_allow_fail - 10) ||
                    borg_read_scroll(sv_scroll_teleport) ||
                    borg_read_scroll(sv_scroll_teleport_level) ||
                    borg_use_staff_fail(sv_staff_teleportation) ||
                    borg_activate_item(act_tele_long) ||
                    /* revisit spells, increased fail rate */
                    borg_dimension_door(tmp_allow_fail + 9) ||
                    borg_spell_fail(TELEPORT_SELF, tmp_allow_fail + 9) ||
                    borg_spell_fail(PORTAL, tmp_allow_fail + 9) ||
                    borg_shadow_shift(tmp_allow_fail + 9) ||
                    /* revisit teleport, increased fail rate */
                    borg_use_staff(sv_staff_teleportation) ||
                    /* Attempt Teleport Level */
                    borg_spell_fail(TELEPORT_LEVEL, tmp_allow_fail + 9) ||
                    /* try phase at least, with some hedging of the safety of landing zone */
                    (borg_caution_phase(75, 2) &&
                        (borg_read_scroll(sv_scroll_phase_door) ||
                            borg_activate_item(act_tele_phase) ||
                            borg_spell_fail(PHASE_DOOR, tmp_allow_fail) ||
                            borg_spell_fail(PORTAL, tmp_allow_fail))))))
        {
            /* Flee! */
            borg_note("# Danger Level 1.");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;
            return (true);
        }

        borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];

        /* try to teleport, get far away from here */
        if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 10 &&
            (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 1 / 10) &&
            borg_allow_teleport() &&
            (borg_dimension_door(90) ||
                borg_spell(TELEPORT_SELF) ||
                borg_spell(PORTAL)))
        {
            /* verify use of spell */
            /* borg_keypress('y');  */

            /* Flee! */
            borg_note("# Danger Level 1.1  Critical Attempt");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;
            return (true);
        }

        /* emergency phase activation no concern for safety of landing zone. */
        if (borg_skill[BI_CDEPTH] &&
            ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 1 / 10 ||
                b_q > avoidance * (45 + risky_boost) / 10) &&
                (borg_activate_item(act_tele_phase) ||
                    borg_read_scroll(sv_scroll_phase_door))))
        {
            /* Flee! */
            borg_escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 1.2  Critical Phase");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;
            return (true);
        }

        /* emergency phase spell */
        if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 10 &&
            (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] * 1 / 10) &&
            ((borg_spell_fail(PHASE_DOOR, 15) ||
                borg_spell(PORTAL))))
        {
            /* verify use of spell */
            /* borg_keypress('y'); */

            /* Flee! */
            borg_note("# Danger Level 1.3  Critical Attempt");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;
            return (true);
        }

        /* Restore the real mana level */
        borg_skill[BI_CURSP] = sv_mana;
    }

    /* If fighting a unique and at the end of the game try to stay and
     * finish the fight.  Only bail out in extreme danger as above.
     */
    if (b_q < avoidance * (25 + risky_boost) / 10 &&
        borg_fighting_unique >= 1 &&
        borg_fighting_unique <= 3 &&
        borg_skill[BI_CDEPTH] >= 97) return (false);


    /* 2 - a bit more scary/
     * Attempt to teleport (usually)
     * do not escape from uniques so quick
     */
    if (borg_skill[BI_ISHEAVYSTUN] ||
        ((b_q > avoidance * (3 + risky_boost) / 10) && borg_class == CLASS_MAGE && borg_skill[BI_CURSP] <= 20 && borg_skill[BI_MAXCLEVEL] >= 45) ||
        ((b_q > avoidance * (13 + risky_boost) / 10) && borg_fighting_unique >= 1 && borg_fighting_unique <= 8 && borg_skill[BI_CDEPTH] != 99) ||
        ((b_q > avoidance * (11 + risky_boost) / 10) && !borg_fighting_unique))
    {

        /* Try teleportation */
        if (borg_escape_stair() ||
            (borg_allow_teleport() &&
                (borg_dimension_door(allow_fail - 10) ||
                    borg_spell_fail(TELEPORT_SELF, allow_fail - 10) ||
                    borg_spell_fail(PORTAL, allow_fail - 10) ||
                    borg_shadow_shift(allow_fail - 10) ||
                    borg_use_staff_fail(sv_staff_teleportation) ||
                    borg_activate_item(act_tele_long) ||
                    borg_read_scroll(sv_scroll_teleport) ||
                    borg_read_scroll(sv_scroll_teleport_level) ||
                    borg_dimension_door(allow_fail) ||
                    borg_spell_fail(TELEPORT_SELF, allow_fail) ||
                    borg_spell_fail(PORTAL, allow_fail) ||
                    borg_shadow_shift(allow_fail) ||
                    borg_use_staff(sv_staff_teleportation))))
        {
            /* Flee! */
            borg_note("# Danger Level 2.1");

            /* Success */
           /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;
            return (true);
        }
        /* Phase door, if useful */
        if (borg_caution_phase(50, 2) &&
            borg_t - borg_t_antisummon > 50 &&
            (borg_spell(PHASE_DOOR) ||
                borg_spell(PORTAL) ||
                borg_read_scroll(sv_scroll_phase_door) ||
                borg_activate_item(act_tele_phase)))
        {
            /* Flee! */
            borg_note("# Danger Level 2.2");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;
            /* Success */
            return (true);
        }

    }

    /* 3- not too bad */
    /* also run if stunned or it is scary here */
    if (borg_skill[BI_ISHEAVYSTUN] ||
        ((b_q > avoidance * (13 + risky_boost) / 10) && borg_fighting_unique >= 2 && borg_fighting_unique <= 8) ||
        ((b_q > avoidance * (10 + risky_boost) / 10) && !borg_fighting_unique) ||
        ((b_q > avoidance * (10 + risky_boost) / 10) && borg_skill[BI_ISAFRAID] && (borg_skill[BI_AMISSILES] <= 0 &&
            borg_class == CLASS_WARRIOR)))
    {
        /* Phase door, if useful */
        if ((borg_escape_stair() ||
            borg_caution_phase(25, 2)) &&
            borg_t - borg_t_antisummon > 50 &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* Flee! */
            borg_escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 3.1");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* Teleport via spell */
        if (borg_allow_teleport() &&
            (borg_dimension_door(allow_fail) ||
                borg_spell_fail(TELEPORT_SELF, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_shadow_shift(allow_fail) ||
                borg_activate_item(act_tele_long) ||
                borg_use_staff_fail(sv_staff_teleportation) ||
                borg_read_scroll(sv_scroll_teleport) ||
                borg_activate_item(act_tele_phase)))
        {
            /* Flee! */
            borg_note("# Danger Level 3.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }
        /* Phase door, if useful */
        if (borg_caution_phase(75, 2) &&
            borg_t - borg_t_antisummon > 50 &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_shadow_shift(allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* Flee! */
            borg_escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 3.3");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* Use Tport Level after the above attempts failed. */
        if (borg_read_scroll(sv_scroll_teleport_level))
        {
            /* Flee! */
            borg_note("# Danger Level 3.4");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* if we got this far we tried to escape but couldn't... */
        /* time to flee */
        if (!goal_fleeing && (!borg_fighting_unique || borg_skill[BI_CLEVEL] < 35) && !vault_on_level)
        {
            /* Note */
            borg_note("# Fleeing (failed to teleport)");

            /* Start fleeing */
            goal_fleeing = true;
        }

        /* Flee now */
        if (!goal_leaving && (!borg_fighting_unique || borg_skill[BI_CLEVEL] < 35) && !vault_on_level)
        {
            /* Flee! */
            borg_note("# Leaving (failed to teleport)");

            /* Start leaving */
            goal_leaving = true;
        }

    }
    /* 4- not too scary but I'm comprimized */
    if ((b_q > avoidance * (8 + risky_boost) / 10 &&
        (borg_skill[BI_CLEVEL] < 35 || borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] / 3)) ||
        ((b_q > avoidance * (9 + risky_boost) / 10) && borg_fighting_unique >= 1 && borg_fighting_unique <= 8 &&
            (borg_skill[BI_CLEVEL] < 35 || borg_skill[BI_CURHP] <= borg_skill[BI_MAXHP] / 3)) ||
        ((b_q > avoidance * (6 + risky_boost) / 10) && borg_skill[BI_CLEVEL] <= 20 && !borg_fighting_unique) ||
        ((b_q > avoidance * (6 + risky_boost) / 10) && borg_skill[BI_CLEVEL] <= 35))
    {
        /* Phase door, if useful */
        if ((borg_escape_stair() ||
            borg_caution_phase(20, 2)) &&
            borg_t - borg_t_antisummon > 50 &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_shadow_shift(allow_fail) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* Flee! */
            borg_escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 4.1");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* Teleport via spell */
        if (borg_allow_teleport() &&
            (borg_dimension_door(allow_fail) ||
                borg_spell_fail(TELEPORT_SELF, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_activate_item(act_tele_long) ||
                borg_shadow_shift(allow_fail) ||
                borg_read_scroll(sv_scroll_teleport) ||
                borg_use_staff_fail(sv_staff_teleportation)))
        {
            /* Flee! */
            borg_note("# Danger Level 4.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* if we got this far we tried to escape but couldn't... */
        /* time to flee */
        if (!goal_fleeing && !borg_fighting_unique && borg_skill[BI_CLEVEL] < 25 && !vault_on_level)
        {
            /* Note */
            borg_note("# Fleeing (failed to teleport)");

            /* Start fleeing */
            goal_fleeing = true;
        }

        /* Flee now */
        if (!goal_leaving && !borg_fighting_unique && !vault_on_level)
        {
            /* Flee! */
            borg_note("# Leaving (failed to teleport)");

            /* Start leaving */
            goal_leaving = true;
        }
        /* Emergency Phase door if a weak mage */
        if (((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER) && borg_skill[BI_CLEVEL] <= 35) &&
            borg_caution_phase(65, 2) &&
            borg_t - borg_t_antisummon > 50 &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_activate_item(act_tele_long) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* Flee! */
            borg_escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 4.3");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

    }

    /* 5- not too scary but I'm very low level  */
    if (borg_skill[BI_CLEVEL] < 10 &&
        (b_q > avoidance * (5 + risky_boost) / 10 ||
            (b_q > avoidance * (7 + risky_boost) / 10 && borg_fighting_unique >= 1 && borg_fighting_unique <= 8)))
    {
        /* Phase door, if useful */
        if ((borg_escape_stair() ||
            borg_caution_phase(20, 2)) &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_shadow_shift(allow_fail) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* Flee! */
            borg_note("# Danger Level 5.1");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* Teleport via spell */
        if (borg_allow_teleport() &&
            (borg_dimension_door(allow_fail) ||
                borg_spell_fail(TELEPORT_SELF, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_shadow_shift(allow_fail) ||
                borg_activate_item(act_tele_long) ||
                borg_read_scroll(sv_scroll_teleport) ||
                borg_use_staff_fail(sv_staff_teleportation)))
        {
            /* Flee! */
            borg_note("# Danger Level 5.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* if we got this far we tried to escape but couldn't... */
        /* time to flee */
        if (!goal_fleeing && !borg_fighting_unique)
        {
            /* Note */
            borg_note("# Fleeing (failed to teleport)");

            /* Start fleeing */
            goal_fleeing = true;
        }

        /* Flee now */
        if (!goal_leaving && !borg_fighting_unique)
        {
            /* Flee! */
            borg_note("# Leaving (failed to teleport)");

            /* Start leaving */
            goal_leaving = true;
        }
        /* Emergency Phase door if a weak mage */
        if (((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER) && borg_skill[BI_CLEVEL] <= 8) &&
            borg_caution_phase(65, 2) &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_read_scroll(sv_scroll_phase_door) ||
                borg_activate_item(act_tele_long)))
        {
            /* Flee! */
            borg_escapes--; /* a phase isn't really an escape */
            borg_note("# Danger Level 5.3");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

    }

    /* 6- not too scary but I'm out of mana  */
    if ((borg_class == CLASS_MAGE || borg_class == CLASS_PRIEST || borg_class == CLASS_NECROMANCER) &&
        (b_q > avoidance * (6 + risky_boost) / 10 ||
            (b_q > avoidance * (8 + risky_boost) / 10 && borg_fighting_unique >= 1 && borg_fighting_unique <= 8)) &&
        (borg_skill[BI_CURSP] <= (borg_skill[BI_MAXSP] * 1 / 10) && borg_skill[BI_MAXSP] >= 100))
    {
        /* Phase door, if useful */
        if ((borg_escape_stair() ||
            borg_caution_phase(20, 2)) &&
            borg_t - borg_t_antisummon > 50 &&
            (borg_spell_fail(PHASE_DOOR, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_activate_item(act_tele_phase) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* Flee! */
            borg_note("# Danger Level 6.1");
            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }

        /* Teleport via spell */
        if (borg_allow_teleport() &&
            (borg_dimension_door(allow_fail) ||
                borg_spell_fail(TELEPORT_SELF, allow_fail) ||
                borg_spell_fail(PORTAL, allow_fail) ||
                borg_activate_item(act_tele_long) ||
                borg_read_scroll(sv_scroll_teleport) ||
                borg_use_staff_fail(sv_staff_teleportation)))
        {
            /* Flee! */
            borg_note("# Danger Level 6.2");

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }
    }

    /* 7- Shoot N Scoot */
    if ((borg_spell_okay_fail(PHASE_DOOR, allow_fail) ||
        borg_spell_okay_fail(PORTAL, allow_fail)) &&
        borg_shoot_scoot_safe(20, 2, b_q))
    {
        /* Phase door */
        if (borg_spell_fail(PHASE_DOOR, allow_fail) ||
            borg_spell_fail(PORTAL, allow_fail))
        {
            /* Flee! */
            borg_note("# Shoot N Scoot. (Danger Level 7.1)");
            borg_escapes--; /* a phase isn't really an escape */

            /* Reset timer if borg was in a anti-summon corridor */
            if (borg_t - borg_t_antisummon < 50) borg_t_antisummon = 0;

            /* Success */
            return (true);
        }
    }


    return (false);
}


/*
 * ** Try healing **
 * this function tries to heal the borg before trying to flee.
 * The ez_heal items (*Heal* and Life) are reserved for Morgoth.
 * In severe emergencies the borg can drink an ez_heal item but that is
 * checked in borg_caution().  He should bail out of the fight before
 * using an ez_heal.
 */
static bool borg_heal(int danger)
{
    int hp_down;
    int pct_down;
    int allow_fail = 15;
    int chance;
    int clw_heal = 15;
    int csw_heal = 25;
    int ccw_heal = 30;
    int cmw_heal = 50;

    int heal_heal = 300;

    int stats_needing_fix = 0;

    bool rod_good = false;

    hp_down = borg_skill[BI_MAXHP] - borg_skill[BI_CURHP];
    pct_down = ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]) * 100 / borg_skill[BI_MAXHP]);
    clw_heal = ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]) * 15 / 100);
    csw_heal = ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]) * 20 / 100);
    ccw_heal = ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]) * 25 / 100);
    cmw_heal = ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]) * 30 / 100);
    heal_heal = ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP]) * 35 / 100);

    if (clw_heal < 15) clw_heal = 15;
    if (csw_heal < 25) csw_heal = 25;
    if (ccw_heal < 30) ccw_heal = 30;
    if (cmw_heal < 50) cmw_heal = 50;
    if (heal_heal < 300) heal_heal = 300;



    /* Quick check for rod success (used later on) */
    if (borg_slot(TV_ROD, sv_rod_healing) != -1)
    {
        /* Reasonable chance of success */
        if (borg_activate_failure(TV_ROD, sv_rod_healing) < 500)
            rod_good = true;
    }

    /* when fighting Morgoth, we want the borg to use Life potion to fix his
     * stats.  So we need to add up the ones that are dropped.
     */
    if (borg_skill[BI_ISFIXSTR]) stats_needing_fix++;
    if (borg_skill[BI_ISFIXINT]) stats_needing_fix++;
    if (borg_skill[BI_ISFIXWIS]) stats_needing_fix++;
    if (borg_skill[BI_ISFIXDEX]) stats_needing_fix++;
    if (borg_skill[BI_ISFIXCON]) stats_needing_fix++;

    /* Special cases get a second vote */
    if (borg_class == CLASS_MAGE && borg_skill[BI_ISFIXINT]) stats_needing_fix++;
    if (borg_class == CLASS_PRIEST && borg_skill[BI_ISFIXWIS]) stats_needing_fix++;
    if (borg_class == CLASS_DRUID && borg_skill[BI_ISFIXWIS]) stats_needing_fix++;
    if (borg_class == CLASS_NECROMANCER && borg_skill[BI_ISFIXINT]) stats_needing_fix++;
    if (borg_class == CLASS_WARRIOR && borg_skill[BI_ISFIXCON]) stats_needing_fix++;
    if (borg_skill[BI_MAXHP] <= 850 && borg_skill[BI_ISFIXCON]) stats_needing_fix++;
    if (borg_skill[BI_MAXHP] <= 700 && borg_skill[BI_ISFIXCON]) stats_needing_fix += 3;
    if (borg_class == CLASS_PRIEST && borg_skill[BI_MAXSP] < 100 && borg_skill[BI_ISFIXWIS])
        stats_needing_fix += 5;
    if (borg_class == CLASS_MAGE && borg_skill[BI_MAXSP] < 100 && borg_skill[BI_ISFIXINT])
        stats_needing_fix += 5;


    /*  Hack -- heal when confused. This is deadly.*/
    /* This is checked twice, once, here, to see if he is in low danger
     * and again at the end of borg_caution, when all other avenues have failed */
    if (borg_skill[BI_ISCONFUSED])
    {
        if ((pct_down >= 80) && danger - heal_heal < borg_skill[BI_CURHP] &&
            borg_quaff_potion(sv_potion_healing))
        {
            borg_note("# Fixing Confusion. Level 1");
            return (true);
        }
        if ((pct_down >= 85) && danger >= borg_skill[BI_CURHP] * 2 &&
            (borg_quaff_potion(sv_potion_star_healing) ||
                borg_quaff_potion(sv_potion_life)))
        {
            borg_note("# Fixing Confusion. Level 1.a");
            return (true);
        }
        if (danger < borg_skill[BI_CURHP] + csw_heal &&
            (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind) ||
                borg_quaff_potion(sv_potion_cure_serious) ||
                borg_quaff_crit(false) ||
                borg_quaff_potion(sv_potion_healing) ||
                borg_use_staff_fail(sv_staff_healing) ||
                borg_use_staff_fail(sv_staff_curing)))
        {
            borg_note("# Fixing Confusion. Level 2");
            return (true);
        }

        /* If my ability to use a teleport staff is really
         * bad, then I should heal up then use the staff.
         */
         /* Check for a charged teleport staff */
        if (borg_equips_staff_fail(sv_staff_teleportation))
        {
            /* check my skill, drink a potion */
            if ((borg_activate_failure(TV_STAFF, sv_staff_teleportation) > 650) &&
                (danger < (avoidance + ccw_heal) * 15 / 10) &&
                (borg_quaff_crit(true) ||
                    borg_quaff_potion(sv_potion_healing)))
            {
                borg_note("# Fixing Confusion. Level 3");
                return (true);
            }
            /* However, if I am in really big trouble and there is no way
             * I am going to be able to
             * survive another round, take my chances on the staff.
             */
            else if (danger > avoidance * 2)
            {
                borg_note("# Too scary to fix Confusion. Level 4");
                return (false);
            }

        }
        else
        {
            /* If I do not have a staff to teleport, take the potion
             * and try to fix the confusion
             */
            if ((borg_quaff_crit(true) ||
                borg_quaff_potion(sv_potion_cure_serious) ||
                borg_quaff_potion(sv_potion_healing)))
            {
                borg_note("# Fixing Confusion. Level 5");
                return (true);
            }
        }
    }
    /*  Hack -- heal when blind. This is deadly.*/
    if (borg_skill[BI_ISBLIND] && (randint0(100) < 85))
    {
        /* if in extreme danger, use teleport then fix the
         * blindness later.
         */
        if (danger > avoidance * 25 / 10)
        {
            /* Check for a charged teleport staff */
            if (borg_equips_staff_fail(sv_staff_teleportation)) return (0);
        }
        if ((hp_down >= 300) && borg_quaff_potion(sv_potion_healing))
        {
            return (true);
        }
        /* Warriors with ESP won't need it so quickly */
        if (!(borg_class == CLASS_WARRIOR && borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 4 &&
            borg_skill[BI_ESP]))
        {
            if (borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
                borg_quaff_potion(sv_potion_cure_light) ||
                borg_quaff_potion(sv_potion_cure_serious) ||
                borg_quaff_crit(true) ||
                borg_use_staff_fail(sv_staff_healing) ||
                borg_use_staff_fail(sv_staff_curing) ||
                borg_quaff_potion(sv_potion_healing))
            {
                borg_note("# Fixing Blindness.");
                return (true);
            }
        }
    }


    /* We generally try to conserve ez-heal pots */
    if ((borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) &&
        ((hp_down >= 400) || (danger > borg_skill[BI_CURHP] * 5 && hp_down > 100)) &&
        borg_quaff_potion(sv_potion_star_healing))
    {
        borg_note("# Fixing Confusion/Blind.");
        return (true);
    }

    /* Healing and fighting Morgoth. */
    if (borg_fighting_unique >= 10)
    {
        if (borg_skill[BI_CURHP] <= 700 &&
            ((borg_skill[BI_CURHP] > 250 && borg_spell_fail(HOLY_WORD, 14)) ||  /* Holy Word */
                /* Choose Life over *Healing* to fix stats*/
                (stats_needing_fix >= 5 && borg_quaff_potion(sv_potion_life)) ||
                /* Choose Life over Healing if way down on pts*/
                (hp_down > 500 && borg_has[borg_lookup_kind(TV_POTION, sv_potion_star_healing)] <= 0 && borg_quaff_potion(sv_potion_life)) ||
                borg_quaff_potion(sv_potion_star_healing) ||
                borg_quaff_potion(sv_potion_healing) ||
                borg_activate_item(act_heal1) ||
                borg_activate_item(act_heal2) ||
                borg_activate_item(act_heal3) ||
                (borg_skill[BI_CURHP] < 250 && borg_spell_fail(HOLY_WORD, 5)) ||  /* Holy Word */
                (borg_skill[BI_CURHP] > 550 && borg_spell_fail(HOLY_WORD, 15)) ||  /* Holy Word */
                borg_spell_fail(HEALING, 15) ||
                borg_quaff_potion(sv_potion_life) ||
                borg_zap_rod(sv_rod_healing)))
        {
            borg_note("# Healing in Questor Combat.");
            return (true);
        }
    }

    /* restore Mana */
    /* note, blow the staff charges easy because the staff will not last. */
    if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] / 5) && (randint0(100) < 50))
    {
        if (borg_use_staff_fail(sv_staff_the_magi))
        {
            borg_note("# Use Magi Staff");
            return (true);
        }
    }
    /* blowing potions is harder */
    /* NOTE: must have enough mana to keep up or do a HEAL */
    if (borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] / 10) ||
        ((borg_skill[BI_CURSP] < 70 && borg_skill[BI_MAXSP] > 200)))
    {
        /*  use the potion if battling a unique and not too dangerous */
        if (borg_fighting_unique >= 10 ||
            (borg_fighting_unique && danger < avoidance * 2) ||
            (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] == 0 && danger > avoidance))
        {
            if (borg_use_staff_fail(sv_staff_the_magi) ||
                borg_quaff_potion(sv_potion_restore_mana))
            {
                borg_note("# Restored My Mana");
                return (true);
            }
        }
    }

    /* if unhurt no healing needed */
    if (hp_down == 0)
        return false;

    /* Don't bother healing if not in danger */
    if (danger == 0 && !borg_skill[BI_ISPOISONED] && !borg_skill[BI_ISCUT])
        return (false);

    /* Restoring while fighting Morgoth */
    if (stats_needing_fix >= 5 && borg_fighting_unique >= 10 &&
        borg_skill[BI_CURHP] > 650 &&
        borg_eat_food(TV_MUSHROOM, sv_mush_restoring))
    {
        borg_note("# Trying to fix stats in combat.");
        return(true);
    }

    /* No further Healing considerations if fighting Questors */
    if (borg_fighting_unique >= 10)
    {
        /* No further healing considerations right now */
        return (false);
    }


    /* Hack -- heal when wounded a percent of the time */
    chance = randint0(100);

    /* if we are fighting a unique increase the odds of healing */
    if (borg_fighting_unique) chance -= 10;

    /* if danger is close to the hp and healing will help, do it */
    if (danger >= borg_skill[BI_CURHP] && danger < borg_skill[BI_MAXHP])
        chance -= 75;
    else
    {
        if (borg_class != CLASS_PRIEST &&
            borg_class != CLASS_PALADIN)
            chance -= 25;
    }


    /* Risky Borgs are less likely to heal in the fight */
    if (borg_cfg[BORG_PLAYS_RISKY]) chance += 5;

    if (((pct_down <= 15 && chance < 98) ||
        (pct_down >= 16 && pct_down <= 25 && chance < 95) ||
        (pct_down >= 26 && pct_down <= 50 && chance < 80) ||
        (pct_down >= 51 && pct_down <= 65 && chance < 50) ||
        (pct_down >= 66 && pct_down <= 74 && chance < 25) ||
        (pct_down >= 75 && chance < 1)) &&
        (!borg_skill[BI_ISHEAVYSTUN] && !borg_skill[BI_ISSTUN] && !borg_skill[BI_ISPOISONED] && !borg_skill[BI_ISCUT]))
        return false;


    /* Cure light Wounds (2d10) */
    if (pct_down >= 30 && (pct_down <= 40 || borg_skill[BI_CLEVEL] < 10) &&
        ((danger) < borg_skill[BI_CURHP] + clw_heal) &&
        (clw_heal > danger / 3) &&  /* No rope-a-doping */
        (borg_spell_fail(MINOR_HEALING, allow_fail) ||
            borg_quaff_potion(sv_potion_cure_light) ||
            borg_activate_item(act_cure_light)))
    {
        borg_note("# Healing Level 1.");
        return (true);
    }
    /* Cure Serious Wounds (4d10) */
    if (pct_down >= 40 && (pct_down <= 50 || borg_skill[BI_CLEVEL] < 20) &&
        ((danger) < borg_skill[BI_CURHP] + csw_heal) &&
        (csw_heal > danger / 3) &&  /* No rope-a-doping */
        (borg_quaff_potion(sv_potion_cure_serious) ||
            borg_activate_item(act_cure_serious)))
    {
        borg_note("# Healing Level 2.");
        return (true);
    }

    /* Cure Critical Wounds (6d10) */
    if (pct_down >= 50 && pct_down <= 55 &&
        ((danger) < borg_skill[BI_CURHP] + ccw_heal) &&
        (ccw_heal > danger / 3) &&  /* No rope-a-doping */
        (borg_activate_item(act_cure_critical) ||
            borg_quaff_crit(false)))
    {
        borg_note("# Healing Level 3.");
        return (true);
    }

    /* If in danger try  one more Cure Critical if it will help */
    if (danger >= borg_skill[BI_CURHP] &&
        danger < borg_skill[BI_MAXHP] &&
        borg_skill[BI_CURHP] < 50 &&
        danger < ccw_heal &&
        borg_quaff_crit(true))
    {
        borg_note("# Healing Level 5.");
        return (true);
    }

    /* if deep, and low on HP, but in a zero danger spot, drink some CCW to add a few HP before resting */
    if (borg_skill[BI_CDEPTH] >= 80 &&
        danger < 50 &&
        pct_down >= 20 &&
        borg_quaff_potion(sv_potion_cure_critical))
    {
        borg_note("# Healing Level 5B.");
        return (true);
    }

    /* Heal step one (200hp) */
    if (pct_down >= 55 &&
        danger < borg_skill[BI_CURHP] + heal_heal &&
        ((((!borg_skill[BI_ATELEPORT] && !borg_skill[BI_AESCAPE]) || rod_good) &&
            borg_zap_rod(sv_rod_healing)) ||
            borg_activate_item(act_cure_full) ||
            borg_activate_item(act_cure_full2) ||
            borg_activate_item(act_cure_nonorlybig) ||
            borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2) ||
            borg_activate_item(act_heal3) ||
            borg_use_staff_fail(sv_staff_healing) ||
            borg_spell_fail(HEALING, allow_fail)))
    {
        borg_note("# Healing Level 6.");
        return (true);
    }

    /* Generally continue to heal.  But if we are preparing for the end
     * game uniques, then bail out here in order to save our heal pots.
     * (unless morgoth is dead)
     * Priests wont need to bail, they have good heal spells.
     */
    if (borg_skill[BI_MAXDEPTH] >= 98 && !borg_skill[BI_KING] && !borg_fighting_unique &&
        borg_class != CLASS_PRIEST)
    {
        /* Bail out to save the heal pots for Morgoth*/
        return (false);
    }

    /* Heal step two (300hp) */
    if (pct_down > 50 &&
        danger < borg_skill[BI_CURHP] + heal_heal &&
        (borg_use_staff_fail(sv_staff_healing) ||
            (borg_fighting_evil_unique && borg_spell_fail(HOLY_WORD, allow_fail)) || /* holy word */
            borg_spell_fail(HEALING, allow_fail) ||
            (((!borg_skill[BI_ATELEPORT] && !borg_skill[BI_AESCAPE]) || rod_good) &&
                borg_zap_rod(sv_rod_healing)) ||
            borg_zap_rod(sv_rod_healing) ||
            borg_quaff_potion(sv_potion_healing)))
    {
        borg_note("# Healing Level 7.");
        return (true);
    }

    /* Healing step three (300hp).  */
    if (pct_down > 60 &&
        danger < borg_skill[BI_CURHP] + heal_heal &&
        ((borg_fighting_evil_unique && borg_spell_fail(HOLY_WORD, allow_fail)) || /* holy word */
            (((!borg_skill[BI_ATELEPORT] && !borg_skill[BI_AESCAPE]) || rod_good) &&
                borg_zap_rod(sv_rod_healing)) ||
            borg_spell_fail(HEALING, allow_fail) ||
            borg_use_staff_fail(sv_staff_healing) ||
            borg_quaff_potion(sv_potion_healing) ||
            borg_activate_item(act_cure_full) ||
            borg_activate_item(act_cure_full2) ||
            borg_activate_item(act_cure_nonorlybig) ||
            borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2) ||
            borg_activate_item(act_heal3)))
    {
        borg_note("# Healing Level 8.");
        return (true);
    }

    /* Healing.  First use of EZ_Heals
     */
    if (pct_down > 65 && (danger < borg_skill[BI_CURHP] + heal_heal) &&
        ((borg_fighting_evil_unique && borg_spell_fail(HOLY_WORD, allow_fail)) || /* holy word */
            borg_spell_fail(HEALING, allow_fail) ||
            borg_use_staff_fail(sv_staff_healing) ||
            (((!borg_skill[BI_ATELEPORT] && !borg_skill[BI_AESCAPE]) || rod_good) &&
                borg_zap_rod(sv_rod_healing)) ||
            borg_quaff_potion(sv_potion_healing) ||
            borg_activate_item(act_cure_full) ||
            borg_activate_item(act_cure_full2) ||
            borg_activate_item(act_cure_nonorlybig) ||
            borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2) ||
            borg_activate_item(act_heal3) ||
            (borg_fighting_unique &&
                (borg_quaff_potion(sv_potion_star_healing) ||
                    borg_quaff_potion(sv_potion_healing) ||
                    borg_quaff_potion(sv_potion_life)))))
    {
        borg_note("# Healing Level 9.");
        return (true);
    }

    /* Healing final check.  Note that *heal* and Life potions are not
     * wasted.  They are saved for Morgoth and emergencies.  The
     * Emergency check is at the end of borg_caution() which is after the
     * borg_escape() routine.
     */
    if (pct_down > 75 && danger > borg_skill[BI_CURHP] &&
        borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] <= 0 &&
        (borg_quaff_potion(sv_potion_healing) ||
            borg_quaff_potion(sv_potion_star_healing) ||
            borg_quaff_potion(sv_potion_life)))
    {
        borg_note("# Healing Level 10.");
        return (true);
    }

    /*** Cures ***/

    /* Dont do these in the middle of a fight, teleport out then try it */
    if (danger > avoidance * 2 / 10) return (false);

    /* Hack -- cure poison when poisoned
     * This was moved from borg_caution.
     */
    if (borg_skill[BI_ISPOISONED] && (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2))
    {
        if (borg_spell_fail(CURE_POISON, 60) ||
            borg_spell_fail(HERBAL_CURING, 60) ||
            borg_quaff_potion(sv_potion_cure_poison) ||
            borg_activate_item(act_cure_body) ||
            borg_activate_item(act_cure_critical) ||
            borg_activate_item(act_cure_temp) ||
            borg_activate_item(act_rem_fear_pois) ||
            borg_activate_item(act_cure_full) ||
            borg_activate_item(act_cure_full2) ||
            borg_activate_item(act_cure_nonorlybig) ||
            borg_use_staff(sv_staff_curing) ||
            borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
            borg_eat_food(TV_MUSHROOM, sv_mush_purging) ||
            /* buy time */
            borg_quaff_crit(true) ||
            borg_spell_fail(HEALING, 60) ||
            borg_spell_fail(HOLY_WORD, 60) ||
            borg_use_staff_fail(sv_staff_healing))
        {
            borg_note("# Curing.");
            return (true);
        }

        /* attempt to fix mana then poison on next round */
        if ((borg_spell_legal(CURE_POISON) ||
            borg_spell_legal(HERBAL_CURING)) &&
            (borg_quaff_potion(sv_potion_restore_mana)))
        {
            borg_note("# Curing next round.");
            return (true);
        }
    }


    /* Hack -- cure poison when poisoned CRITICAL CHECK
     */
    if (borg_skill[BI_ISPOISONED] && (borg_skill[BI_CURHP] < 2 || borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20))
    {
        int sv_mana = borg_skill[BI_CURSP];

        borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];

        if (borg_spell(CURE_POISON) ||
            borg_spell(HERBAL_CURING) ||
            borg_spell(HOLY_WORD) ||
            borg_spell(HEALING))
        {
            /* verify use of spell */
            /* borg_keypress('y'); */

            /* Flee! */
            borg_note("# Emergency Cure Poison! Gasp!!!....");

            return (true);
        }
        borg_skill[BI_CURSP] = sv_mana;

        /* Quaff healing pots to buy some time- in this emergency.  */
        if (borg_quaff_potion(sv_potion_cure_light) ||
            borg_quaff_potion(sv_potion_cure_serious)) return (true);

        /* Try to Restore Mana */
        if (borg_quaff_potion(sv_potion_restore_mana)) return (true);

        /* Emergency check on healing.  Borg_heal has already been checked but
         * but we did not use our ez_heal potions.  All other attempts to save
         * ourself have failed.  Use the ez_heal if I have it.
         */
        if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20 &&
            (borg_quaff_potion(sv_potion_star_healing) ||
                borg_quaff_potion(sv_potion_life) ||
                borg_quaff_potion(sv_potion_healing)))
        {
            borg_note("# Healing. Curing section.");
            return (true);
        }

        /* Quaff unknown potions in this emergency.  We might get luck */
        if (borg_quaff_unknown()) return (true);

        /* Eat unknown mushroom in this emergency.  We might get luck */
        if (borg_eat_unknown()) return (true);

        /* Use unknown Staff in this emergency.  We might get luck */
        if (borg_use_unknown()) return (true);

    }

    /* Hack -- cure wounds when bleeding, also critical check */
    if (borg_skill[BI_ISCUT] && (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3 || randint0(100) < 20))
    {
        if (borg_quaff_potion(sv_potion_cure_serious) ||
            borg_quaff_potion(sv_potion_cure_light) ||
            borg_quaff_crit(borg_skill[BI_CURHP] < 10) ||
            borg_spell(MINOR_HEALING) ||
            borg_quaff_potion(sv_potion_cure_critical))
        {
            return (true);
        }
    }
    /* bleeding and about to die CRITICAL CHECK*/
    if (borg_skill[BI_ISCUT] && ((borg_skill[BI_CURHP] < 2) || borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20))
    {
        int sv_mana = borg_skill[BI_CURSP];

        borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];

        /* Quaff healing pots to buy some time- in this emergency.  */
        if (borg_quaff_potion(sv_potion_cure_light) ||
            borg_quaff_potion(sv_potion_cure_serious)) return (true);

        /* Try to Restore Mana */
        if (borg_quaff_potion(sv_potion_restore_mana)) return (true);

        /* Emergency check on healing.  Borg_heal has already been checked but
         * but we did not use our ez_heal potions.  All other attempts to save
         * ourself have failed.  Use the ez_heal if I have it.
         */
        if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 20 &&
            (borg_quaff_potion(sv_potion_healing) ||
                borg_quaff_potion(sv_potion_star_healing) ||
                borg_quaff_potion(sv_potion_life)))
        {
            borg_note("# Healing.  Bleeding.");
            return (true);
        }

        /* Cast a spell, go into negative mana */
        if (borg_spell(MINOR_HEALING))
        {
            /* verify use of spell */
            /* borg_keypress('y'); */

            /* Flee! */
            borg_note("# Emergency Wound Patch! Gasp!!!....");

            return (true);
        }
        borg_skill[BI_CURSP] = sv_mana;

        /* Quaff unknown potions in this emergency.  We might get luck */
        if (borg_quaff_unknown()) return (true);

        /* Eat unknown mushroom in this emergency.  We might get luck */
        if (borg_eat_unknown()) return (true);

        /* Use unknown Staff in this emergency.  We might get luck */
        if (borg_use_unknown()) return (true);
    }

    /* nothing to do */
    return (false);

}

static bool borg_prep_leave_level_spells(void)
{
    /* if we are running away, just run. */
    if (goal_fleeing)
        return (false);

    /* if we are low on mana, don't prep. */
    if (borg_skill[BI_CURSP] < ((borg_skill[BI_MAXSP] * 6) / 10))
        return (false);

    /* Cast haste */
    if (!borg_speed && borg_spell_fail(HASTE_SELF, 15))
    {
        borg_note("# Casting speed spell before leaving level.");
        borg_no_rest_prep = 5000;
        return (true);
    }

    /* Cast resistance */
    if (borg_skill[BI_TRFIRE] + borg_skill[BI_TRCOLD] + borg_skill[BI_TRACID] +
        borg_skill[BI_TRELEC] + borg_skill[BI_TRPOIS] < 3 && borg_spell_fail(RESISTANCE, 15))
    {
        borg_note("# Casting Resistance spell before leaving level.");
        borg_no_rest_prep = 21000;
        return (true);
    }

    /* Cast fastcast */
    if (!borg_fastcast && borg_spell_fail(MANA_CHANNEL, 15))
    {
        borg_note("# Casting Mana Channel spell before leaving level.");
        borg_no_rest_prep = 6000;
        return (true);
    }

    /* Cast Berserk Strength */
    if (!borg_berserk && borg_spell_fail(BERSERK_STRENGTH, 15))
    {
        borg_note("# Casting Berserk Strength spell before leaving level.");
        borg_no_rest_prep = 10000;
        return (true);
    }

    /* Cast heroism */
    if (!borg_hero && borg_spell_fail(HEROISM, 15))
    {
        borg_note("# Casting Heroism spell before leaving level.");
        borg_no_rest_prep = 3000;
        return (true);
    }

    /* Cast regen just before returning to dungeon */
    if (!borg_regen && borg_spell_fail(RAPID_REGENERATION, 15))
    {
        borg_note("# Casting Regen before leaving level.");
        borg_no_rest_prep = 6000;
        return (true);
    }

    /* Cast Smite Evil just before returning to dungeon */
    if (!borg_smite_evil && !borg_skill[BI_WS_EVIL] && borg_spell_fail(SMITE_EVIL, 15))
    {
        borg_note("# Casting Smite Evil before leaving level.");
        borg_no_rest_prep = 21000;
        return (true);
    }

    /* Cast Venom just before returning to dungeon */
    if (!borg_venom && !borg_skill[BI_WB_POIS] && borg_spell_fail(VENOM, 15))
    {
        borg_note("# Casting Venom before leaving level.");
        borg_no_rest_prep = 18000;
        return (true);
    }

    /* Cast PFE just before returning to dungeon */
    if (!borg_prot_from_evil && borg_spell_fail(PROTECTION_FROM_EVIL, 15))
    {
        borg_note("# Casting PFE before leaving level.");
        borg_no_rest_prep = borg_skill[BI_CLEVEL] * 1000;
        return (true);
    }

    /* Cast bless prep things */
    if ((!borg_bless && (borg_spell_fail(BLESS, 15) ||
        borg_spell_fail(DEMON_BANE, 15))))
    {
        borg_note("# Casting blessing before leaving level.");
        borg_no_rest_prep = 11000;
        return (true);
    }
    return (false);
}

/*
 * Be "cautious" and attempt to prevent death or dishonor.
 *
 * Strategy:
 *
 *   (1) Caution
 *   (1a) Analyze the situation
 *   (1a1) try to heal
 *   (1a2) try a defence
 *   (1b) Teleport from danger
 *   (1c) Handle critical stuff
 *   (1d) Retreat to happy grids
 *   (1e) Back away from danger
 *   (1f) Heal various conditions
 *
 *   (2) Attack
 *   (2a) Simulate possible attacks
 *   (2b) Perform optimal attack
 *
 *   (3) Recover
 *   (3a) Recover by spells/prayers
 *   (3b) Recover by items/etc
 *   (3c) Recover by resting
 *
 * XXX XXX XXX
 * In certain situations, the "proper" course of action is to simply
 * attack a nearby monster, since often most of the danger is due to
 * a single monster which can sometimes be killed in a single blow.
 *
 * Actually, both "borg_caution()" and "borg_recover()" need to
 * be more intelligent, and should probably take into account
 * such things as nearby monsters, and/or the relative advantage
 * of simply pummeling nearby monsters instead of recovering.
 *
 * Note that invisible/offscreen monsters contribute to the danger
 * of an extended "region" surrounding the observation, so we will
 * no longer rest near invisible monsters if they are dangerous.
 *
 * XXX XXX XXX
 * We should perhaps reduce the "fear" values of each region over
 * time, to take account of obsolete invisible monsters.
 *
 * Note that walking away from a fast monster is counter-productive,
 * since the monster will often just follow us, so we use a special
 * method which allows us to factor in the speed of the monster and
 * predict the state of the world after we move one step.  Of course,
 * walking away from a spell casting monster is even worse, since the
 * monster will just get to use the spell attack multiple times.  But,
 * if we are trying to get to known safety, then fleeing in such a way
 * might make sense.  Actually, this has been done too well, note that
 * it makes sense to flee some monsters, if they "stumble", or if we
 * are trying to get to stairs.  XXX XXX XXX
 *
 * Note that the "flow" routines attempt to avoid entering into
 * situations that are dangerous, but sometimes we do not see the
 * danger coming, and then we must attempt to survive by any means.
 *
 * We will attempt to "teleport" if the danger in the current situation,
 * as well as that resulting from attempting to "back away" from danger,
 * are sufficient to kill us in one or two blows.  This allows us to
 * avoid teleportation in situations where simply backing away is the
 * proper course of action, for example, when standing next to a nasty
 * stationary monster, but also to teleport when backing away will not
 * reduce the danger sufficiently.
 *
 * But note that in "nasty" situations (when we are running out of light,
 * or when we are starving, blind, confused, or hallucinating), we will
 * ignore the possibility of "backing away" from danger, when considering
 * the possibility of using "teleport" to escape.  But if the teleport
 * fails, we will still attempt to "retreat" or "back away" if possible.
 *
 * XXX XXX XXX Note that it should be possible to do some kind of nasty
 * "flow" algorithm which would use a priority queue, or some reasonably
 * efficient normal queue stuff, to determine the path which incurs the
 * smallest "cumulative danger", and minimizes the total path length.
 * It may even be sufficient to treat each step as having a cost equal
 * to the danger of the destination grid, plus one for the actual step.
 * This would allow the Borg to prefer a ten step path passing through
 * one grid with danger 10, to a five step path, where each step has
 * danger 9.  Currently, he often chooses paths of constant danger over
 * paths with small amounts of high danger.  However, the current method
 * is very fast, which is certainly a point in its favor...
 *
 * When in danger, attempt to "flee" by "teleport" or "recall", and if
 * this is not possible, attempt to "heal" damage, if needed, and else
 * attempt to "flee" by "running".
 *
 * XXX XXX XXX Both "borg_caution()" and "borg_recover()" should only
 * perform the HEALING tasks if they will cure more "damage"/"stuff"
 * than may be re-applied in the next turn, this should prevent using
 * wimpy healing spells next to dangerous monsters, and resting to regain
 * mana near a mana-drainer.
 *
 * Whenever we are in a situation in which, even when fully healed, we
 * could die in a single round, we set the "goal_fleeing" flag, and if
 * we could die in two rounds, we set the "goal_leaving" flag.
 *
 * In town, whenever we could die in two rounds if we were to stay still,
 * we set the "goal_leaving" flag.  In combination with the "retreat" and
 * the "back away" code, this should allow us to leave town before getting
 * into situations which might be fatal.
 *
 * Flag "goal_fleeing" means get off this level right now, using recall
 * if possible when we get a chance, and otherwise, take stairs, even if
 * it is very dangerous to do so.
 *
 * Flag "goal_leaving" means get off this level when possible, using
 * stairs if possible when we get a chance.
 *
 * We will also take stairs if we happen to be standing on them, and we
 * could die in two rounds.  This is often "safer" than teleportation,
 * and allows the "retreat" code to retreat towards stairs, knowing that
 * once there, we will leave the level.
 *
 * If we can, we should try to hit a monster with an offset  spell.
 * A Druj can not move but they are really dangerous.  So we should retreat
 * to a happy grid (meaning we have los and it does not), we should target
 * one space away from the bad guy then blast away with ball spells.
 *
 * Hack -- Special checks for dealing with Morgoth.
 * The borg would like to stay put on level 100 and use
 * spells to attack Morgoth then use Teleport Other as he
 * gets too close.
 * 1.  Make certain borg is sitting in a central room.
 * 2.  Attack Morgoth with spells.
 * 3.  Use Teleport Other on Morgoth as he approches.
 * 4.  Use Teleport Other/Mass Banishment on all other monsters
 *     if borg is correctly positioned in a good room.
 * 5.  Stay put and rest until Morgoth returns.
 */
bool borg_caution(void)
{
    int j, pos_danger;
    bool borg_surround = false;
    bool nasty = false;
    bool on_dnstair = false;
    bool on_upstair = false;

    /*** Notice "nasty" situations ***/

    /* About to run out of light is extremely nasty */
    if (!borg_skill[BI_LIGHT] && borg_items[INVEN_LIGHT].timeout < 250) nasty = true;

    /* Starvation is nasty */
    if (borg_skill[BI_ISWEAK]) nasty = true;

    /* Blind-ness is nasty */
    if (borg_skill[BI_ISBLIND]) nasty = true;

    /* Confusion is nasty */
    if (borg_skill[BI_ISCONFUSED]) nasty = true;

    /* Hallucination is nasty */
    if (borg_skill[BI_ISIMAGE]) nasty = true;

    /* if on level 100 and not ready for Morgoth, run */
    if (borg_skill[BI_CDEPTH] == 100 && borg_t - borg_began < 10 &&
        !borg_morgoth_position)
    {
        if (borg_ready_morgoth == 0 && !borg_skill[BI_KING])
        {
            /* teleport level up to 99 to finish uniques */
            if (borg_spell(TELEPORT_LEVEL) ||
                borg_read_scroll(sv_scroll_teleport_level))
            {
                borg_note("# Rising one dlevel (Not ready for Morgoth)");
                return (true);
            }

            /* Start leaving */
            if (!goal_leaving)
            {
                /* Note */
                borg_note("# Leaving (Not ready for Morgoth now)");

                /* Start leaving */
                goal_leaving = true;
            }
        }
    }

    /*** Evaluate local danger ***/

    /* Monsters on all sides of me? */
    borg_surround = borg_surrounded();

    /* No searching if scary guys on the level */
    if (scaryguy_on_level == true) borg_needs_searching = false;

    /* Only allow three 'escapes' per level unless heading for morogoth
       or fighting a unique, then allow 85. */
    if ((borg_escapes > 3 && !unique_on_level && !borg_ready_morgoth) ||
        borg_escapes > 55)
    {
        /* No leaving if going after questors */
        if (borg_skill[BI_CDEPTH] <= 98)
        {
            /* Start leaving */
            if (!goal_leaving)
            {
                /* Note */
                borg_note("# Leaving (Too many escapes)");

                /* Start leaving */
                goal_leaving = true;
            }

            /* Start fleeing */
            if (!goal_fleeing && borg_escapes > 3)
            {
                /* Note */
                borg_note("# Fleeing (Too many escapes)");

                /* Start fleeing */
                goal_fleeing = true;
            }
        }
    }

    /* No hanging around if nasty here. */
    if (scaryguy_on_level)
    {
        /* Note */
        borg_note("# Scary guy on level.");

        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note("# Leaving (Scary guy on level)");

            /* Start leaving */
            goal_leaving = true;
        }

        /* Start fleeing */
        if (!goal_fleeing)
        {
            /* Note */
            borg_note("# Fleeing (Scary guy on level)");

            /* Start fleeing */
            goal_fleeing = true;
        }

        /* Return to town quickly after leaving town */
        if (borg_skill[BI_CDEPTH] == 0) goal_fleeing_to_town = true;
    }

    /* Make a note if Ignoring monsters (no fighting) */
    if (goal_ignoring)
    {
        /* Note */
        borg_note("# Ignoring combat with monsters.");
    }

    /* Note if ignorig messages */
    if (borg_dont_react)
    {
        borg_note("# Borg ignoring messges.");
    }

    /* Look around */
    pos_danger = borg_danger(c_y, c_x, 1, true, false);

    /* Describe (briefly) the current situation */
    /* Danger (ignore stupid "fear" danger) */
    if ((((pos_danger > avoidance / 10) || (pos_danger > borg_fear_region[c_y / 11][c_x / 11]) ||
        borg_morgoth_position || borg_skill[BI_ISWEAK]) ||
        borg_skill[BI_CDEPTH] == 100) && !borg_skill[BI_KING])
    {
        /* Describe (briefly) the current situation */
        borg_note(format("# Loc:%d,%d Dep:%d Lev:%d HP:%d/%d SP:%d/%d Danger:p=%d",
            c_y, c_x, borg_skill[BI_CDEPTH], borg_skill[BI_CLEVEL],
            borg_skill[BI_CURHP], borg_skill[BI_MAXHP], borg_skill[BI_CURSP], borg_skill[BI_MAXSP], pos_danger));
        if (borg_resistance)
        {
            borg_note(format("# Protected by Resistance (borg turns:%d; game turns:%d)", borg_resistance / borg_game_ratio, player->timed[TMD_OPP_ACID]));
        }
        if (borg_shield)
        {
            borg_note("# Protected by Mystic Shield");
        }
        if (borg_prot_from_evil)
        {
            borg_note("# Protected by PFE");
        }
        if (borg_morgoth_position)
        {
            borg_note("# Protected by Sea of Runes.");
        }
        if (borg_fighting_unique >= 10)
        {
            borg_note("# Questor Combat.");
        }
        if (borg_as_position)
        {
            borg_note("# Protected by anti-summon corridor.");
        }
    }
    /* Comment on glyph */
    if (track_glyph.num)
    {
        int i;
        for (i = 0; i < track_glyph.num; i++)
        {
            /* Enqueue the grid */
            if ((track_glyph.y[i] == c_y) &&
                (track_glyph.x[i] == c_x))
            {
                /* if standing on one */
                borg_note(format("# Standing on Glyph"));
            }
        }
    }
    /* Comment on stair */
    if (track_less.num)
    {
        int i;
        for (i = 0; i < track_less.num; i++)
        {
            /* Enqueue the grid */
            if ((track_less.y[i] == c_y) &&
                (track_less.x[i] == c_x))
            {
                /* if standing on one */
                borg_note(format("# Standing on up-stairs"));
                on_upstair = false;
            }
        }
    }
    /* Comment on stair */
    if (track_more.num)
    {
        int i;
        for (i = 0; i < track_more.num; i++)
        {
            /* Enqueue the grid */
            if ((track_more.y[i] == c_y) &&
                (track_more.x[i] == c_x))
            {
                /* if standing on one */
                borg_note(format("# Standing on dn-stairs"));
                on_dnstair = false;
            }
        }
    }

    if (!goal_fleeing)
    {
        /* Start being cautious and trying to not die */
        if (borg_class == CLASS_MAGE && !borg_morgoth_position && !borg_as_position &&
            !borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCUT] &&
            !borg_skill[BI_ISPOISONED] && !borg_skill[BI_ISCONFUSED])
        {
            /* do some defence before running away */
            if (borg_defend(pos_danger))
                return true;

            /* try healing before running away */
            if (borg_heal(pos_danger))
                return true;
        }
        else
        {
            /* try healing before running away */
            if (borg_heal(pos_danger))
                return true;

            /* do some defence before running away! */
            if (borg_defend(pos_danger))
                return true;
        }
    }


    if (borg_cfg[BORG_USES_SWAPS])
    {
        /* do some swapping before running away! */
        if (pos_danger > (avoidance / 3))
        {
            if (borg_backup_swap(pos_danger))
                return true;
        }
    }

    /* If I am waiting for recall,  & safe, then stay put. */
    if (goal_recalling && borg_check_rest(c_y, c_x) &&
        borg_skill[BI_CDEPTH] &&
        !borg_skill[BI_ISHUNGRY])
    {
        /* rest here until lift off */
        borg_note("# Resting for Recall.");
        borg_keypress('R');
        borg_keypress('5');
        borg_keypress('0');
        borg_keypress('0');
        borg_keypress(KC_ENTER);

        /* I'm not in a store */
        borg_in_shop = false;

        return (true);
    }

    /* If I am waiting for recall in town */
    if (goal_recalling && goal_recalling <= (borg_game_ratio * 2) && !borg_skill[BI_CDEPTH])
    {
        if (borg_prep_leave_level_spells())
            return (true);
    }

    /*** Danger ***/

    /* Impending doom */
    /* Don't take off in the middle of a fight */
    /* just to restock and it is useless to restock */
    /* if you have just left town. */
    if (borg_restock(borg_skill[BI_CDEPTH]) &&
        !borg_fighting_unique &&
        (borg_time_town + (borg_t - borg_began)) > 200)
    {
        /* Start leaving */
        if (!goal_leaving)
        {
            /* Note */
            borg_note(format("# Leaving (restock) %s", borg_restock(borg_skill[BI_CDEPTH])));

            /* Start leaving */
            goal_leaving = true;
        }
        /* Start fleeing */
        if (!goal_fleeing && borg_skill[BI_ACCW] < 2 && borg_skill[BI_FOOD] > 3 &&
            borg_skill[BI_AFUEL] > 2)
        {
            /* Flee */
            borg_note(format("# Fleeing (restock) %s", borg_restock(borg_skill[BI_CDEPTH])));

            /* Start fleeing */
            goal_fleeing = true;
        }
    }
    /* Excessive danger */
    else if (pos_danger > (borg_skill[BI_CURHP] * 2))
    {
        /* Start fleeing */
        /* do not flee level if going after Morgoth or fighting a unique */
        if (!goal_fleeing && !borg_fighting_unique && (borg_skill[BI_CLEVEL] < 50) &&
            !vault_on_level && (borg_skill[BI_CDEPTH] < 100 && borg_ready_morgoth == 1))
        {
            /* Note */
            borg_note("# Fleeing (excessive danger)");

            /* Start fleeing */
            goal_fleeing = true;
        }
    }
    /* Potential danger (near death) in town */
    else if (!borg_skill[BI_CDEPTH] && (pos_danger > borg_skill[BI_CURHP]) && (borg_skill[BI_CLEVEL] < 50))
    {
        /* Flee now */
        if (!goal_leaving)
        {
            /* Flee! */
            borg_note("# Leaving (potential danger)");

            /* Start leaving */
            goal_leaving = true;
        }
    }


    /*** Stairs ***/

    /* Leaving or Fleeing, take stairs */
    if (goal_leaving || goal_fleeing || scaryguy_on_level || goal_fleeing_lunal || goal_fleeing_munchkin ||
        ((pos_danger > avoidance || (borg_skill[BI_CLEVEL] < 5 && pos_danger > avoidance / 2)) &&
            borg_grids[c_y][c_x].feat == FEAT_LESS)) /* danger and standing on stair */
    {
        if (borg_ready_morgoth == 0 && !borg_skill[BI_KING])
        {
            stair_less = true;
            if (scaryguy_on_level) borg_note("# Fleeing and leaving the level. (scaryguy)");
            if (goal_fleeing_lunal) borg_note("# Fleeing and leaving the level. (fleeing_lunal)");
            if (goal_fleeing_munchkin) borg_note("# Fleeing and leaving the level. (fleeing munchkin)");
            if (pos_danger > avoidance && borg_skill[BI_CLEVEL] <= 49 && borg_grids[c_y][c_x].feat == FEAT_LESS)
                borg_note("# Leaving level,  Some danger but I'm on a stair.");
        }

        if (scaryguy_on_level) stair_less = true;

        /* Only go down if fleeing or prepared */
        if (goal_fleeing == true || goal_fleeing_lunal == true || goal_fleeing_munchkin) stair_more = true;

        if ((char*)NULL == borg_prepared(borg_skill[BI_CDEPTH] + 1))
            stair_more = true;

        if (!track_less.num && (borg_skill[BI_CURLITE] == 0 || borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] < 2))
            stair_more = false;

        /* If I need to sell crap, then don't go down */
        if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
            borg_count_sell() >= 13) stair_more = false;

        /* Its ok to go one level deep if evading scary guy */
        if (scaryguy_on_level) stair_more = true;

        /* if fleeing town, then dive */
        if (!borg_skill[BI_CDEPTH]) stair_more = true;
    }

    /* Take stairs up */
    if (stair_less)
    {
        /* Current grid */
        borg_grid* ag = &borg_grids[c_y][c_x];

        /* Usable stairs */
        if (ag->feat == FEAT_LESS || borg_on_upstairs || on_upstair)
        {
            /* Log it */
            borg_note(format("# Leaving via up stairs."));

            /* Take the stairs */
            borg_on_dnstairs = true;
            borg_keypress('<');

            /* Success */
            return (true);
        }
    }


    /* Take stairs down */
    if (stair_more && !goal_recalling)
    {
        /* Current grid */
        borg_grid* ag = &borg_grids[c_y][c_x];

        /* Usable stairs */
        if (ag->feat == FEAT_MORE || borg_on_dnstairs || on_dnstair)
        {
            /* Do these if not lunal mode */
            if (!goal_fleeing_lunal && !goal_fleeing_munchkin)
            {
                if (borg_prep_leave_level_spells())
                    return (true);
            }

            /* Take the stairs */
            borg_on_upstairs = true;
            borg_keypress('>');

            /* Success */
            return (true);
        }
    }


    /*** Deal with critical situations ***/

    /* Hack -- require light */
    if (!borg_skill[BI_CURLITE] && !borg_skill[BI_LIGHT]) /* No Lite, AND Not Glowing */
    {
        enum borg_need need = borg_maintain_light();
        if (need == BORG_MET_NEED)
            return true;
        else if ((need == BORG_UNMET_NEED) && borg_skill[BI_CDEPTH])
        {
            /* Flee for fuel */
            /* Start leaving */
            if (!goal_leaving)
            {
                /* Flee */
                borg_note("# Leaving (need fuel)");

                /* Start leaving */
                goal_leaving = true;
            }
        }
    }

    /* Hack -- prevent starvation */
    if (borg_skill[BI_ISWEAK])
    {
        /* Attempt to satisfy hunger */
        if (borg_eat_food_any() ||
            borg_spell(REMOVE_HUNGER) ||
            borg_spell(HERBAL_CURING))
        {
            /* Success */
            return (true);
        }

        /* Try to restore mana then cast the spell next round */
        if (borg_quaff_potion(sv_potion_restore_mana)) return (true);

        /* Flee for food */
        if (borg_skill[BI_CDEPTH])
        {
            /* Start leaving */
            if (!goal_leaving)
            {
                /* Flee */
                borg_note("# Leaving (need food)");

                /* Start leaving */
                goal_leaving = true;
            }

            /* Start fleeing */
            if (!goal_fleeing)
            {
                /* Flee */
                borg_note("# Fleeing (need food)");

                /* Start fleeing */
                goal_fleeing = true;
            }
        }
    }

    /* Prevent breeder explosions when low level */
    if (breeder_level && borg_skill[BI_CLEVEL] < 15)
    {
        /* Start leaving */
        if (!goal_fleeing)
        {
            /* Flee */
            borg_note("# Fleeing (breeder level)");

            /* Start fleeing */
            goal_fleeing = true;
        }

    }

    /*** Flee on foot ***/

    /* Desperation Head for stairs */
    /* If you are low level and near the stairs and you can */
    /* hop onto them in very few steps, try to head to them */
    /* out of desperation */
    if ((track_less.num || track_more.num) &&
        (goal_fleeing || scaryguy_on_level || (pos_danger > avoidance && borg_skill[BI_CLEVEL] < 35)))
    {
        int y, x, i;
        int b_j = -1;
        int m;
        int b_m = -1;
        bool safe = true;


        borg_grid* ag;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++)
        {
            x = track_less.x[i];
            y = track_less.y[i];

            ag = &borg_grids[y][x];

            /* How far is the nearest up stairs */
            j = borg_distance(c_y, c_x, y, x);

            /* Skip stairs if a monster is on the stair */
            if (ag->kill) continue;

            /* skip the closer ones */
            if (b_j >= j) continue;

            /* track it */
            b_j = j;
        }

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++)
        {
            x = track_more.x[i];
            y = track_more.y[i];

            ag = &borg_grids[y][x];

            /* How far is the nearest up stairs */
            m = borg_distance(c_y, c_x, y, x);

            /* Skip stairs if a monster is on the stair */
            if (ag->kill) continue;

            /* skip the closer ones */
            if (b_m >= m) continue;

            /* track it */
            b_m = m;
        }

        /* If you are within a few (3) steps of the stairs */
        /* and you can take some damage to get there */
        /* go for it */
        if (b_j < 3 && b_j != -1 &&
            pos_danger < borg_skill[BI_CURHP])
        {
            borg_desperate = true;
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                /* Note */
                borg_note("# Desperate for Stairs (one)");

                borg_desperate = false;
                return (true);
            }
            borg_desperate = false;
        }

        /* If you are next to steps of the stairs go for it */
        if (b_j <= 2 && b_j != -1)
        {
            borg_desperate = true;
            if (borg_flow_stair_less(GOAL_FLEE, false))
            {
                /* Note */
                borg_note("# Desperate for Stairs (two)");

                borg_desperate = false;
                return (true);
            }
            borg_desperate = false;
        }

        /* Low level guys tend to waste money reading the recall scrolls */
        if (b_j < 20 && b_j != -1 && scaryguy_on_level && borg_skill[BI_CLEVEL] < 20)
        {
            /* do not attempt it if an adjacent monster is faster than me */
            for (i = 0; i < 8; i++)
            {
                x = c_x + ddx_ddd[i];
                y = c_y + ddy_ddd[i];

                /* check for bounds */
                if (!square_in_bounds(cave, loc(x, y))) continue;

                /* Monster there ? */
                if (!borg_grids[y][x].kill) continue;

                /* Access the monster and check it's speed */
                if (borg_kills[borg_grids[y][x].kill].speed > borg_skill[BI_SPEED]) safe = false;

            }

            /* Dont run from Grip or Fang */
            if ((borg_skill[BI_CDEPTH] <= 5 && borg_skill[BI_CDEPTH] != 0 && borg_fighting_unique) ||
                !safe)
            {
                /* try to take them on, you cant outrun them */
            }
            else
            {
                borg_desperate = true;
                if (borg_flow_stair_less(GOAL_FLEE, false))
                {
                    /* Note */
                    borg_note("# Desperate for Stairs (three)");

                    borg_desperate = false;
                    return (true);
                }
                borg_desperate = false;
            }
        }

        /* If you are next to steps of the down stairs go for it */
        if (b_m <= 2 && b_m != -1)
        {
            borg_desperate = true;
            if (borg_flow_stair_more(GOAL_FLEE, false, false))
            {
                /* Note */
                borg_note("# Desperate for Stairs (four)");

                borg_desperate = false;
                return (true);
            }
            borg_desperate = false;
        }

    }


    /*
     * Strategic retreat
     *
     * Do not retreat if
     * 1) we are icky (poisoned, blind, confused etc
     * 2) we have boosted our avoidance because we are stuck
     * 3) we are in a Sea of Runes
     * 4) we are not in a vault
     */
    if (((pos_danger > avoidance / 3 && !nasty && !borg_no_retreat) ||
        (borg_surround && pos_danger != 0)) &&
        !borg_morgoth_position && (borg_t - borg_t_antisummon >= 50) &&
        !borg_skill[BI_ISCONFUSED] &&
        !square_isvault(cave, loc(c_x, c_y)) &&
        borg_skill[BI_CURHP] < 500)
    {
        int d, b_d = -1;
        int r, b_r = -1;
        int b_p = -1, p1 = -1;
        int b_x = c_x;
        int b_y = c_y;
        int ii;

        /* Scan the useful viewable grids */
        for (j = 1; j < borg_view_n; j++)
        {
            int x1 = c_x;
            int y1 = c_y;

            int x2 = borg_view_x[j];
            int y2 = borg_view_y[j];

            /* Cant if confused: no way to predict motion */
            if (borg_skill[BI_ISCONFUSED]) continue;

            /* Require "floor" grids */
            if (!borg_cave_floor_bold(y2, x2)) continue;

            /* Try to avoid pillar dancing if at good health */
            if ((borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 7 / 10 &&
                ((track_step.num > 2 && 
                    (track_step.y[track_step.num - 2] == y2 &&
                    track_step.x[track_step.num - 2] == x2 &&
                    track_step.y[track_step.num - 3] == c_y &&
                    track_step.x[track_step.num - 3] == c_x)))) ||
                    time_this_panel >= 300) continue;

            /* XXX -- Borgs in an unexplored hall (& with only a torch)
             * will always return false for Happy Grids:
             *
             *  222222      Where 2 = unknown grid.  Borg has a torch.
             *  2221.#      Borg will consider both the . and the 1
             *     #@#      for a retreat from the C. But the . will be
             *     #C#      false d/t adjacent wall to the east.  1 will
             *     #'#      will be false d/t unknown grid to the west.
             *              So he makes no attempt to retreat.
             * However, the next function (backing away), allows him
             * to back up to 1 safely.
             *
             * To play safer, the borg should not retreat to grids where
             * he has not previously been.  This tends to run him into
             * more monsters.  It is better for him to retreat to grids
             * previously travelled, where the monsters are most likely
             * dead, and the path is clear.  However, there is not (yet)
             * tag for those grids.  Something like BORG_BEEN would work.
             */

             /* Require "happy" grids (most of the time)*/
            if (!borg_happy_grid_bold(y2, x2)) continue;

            /* Track "nearest" grid */
            if (b_r >= 0)
            {
                int ay = ((y2 > y1) ? (y2 - y1) : (y1 - y2));
                int ax = ((x2 > x1) ? (x2 - x1) : (x1 - x2));

                /* Ignore "distant" locations */
                if ((ax > b_r) || (ay > b_r)) continue;
            }

            /* Reset */
            r = 0;

            /* Simulate movement */
            while (1)
            {
                borg_grid* ag;

                /* Obtain direction */
                d = borg_goto_dir(y1, x1, y2, x2);

                /* Verify direction */
                if ((d == 0) || (d == 5)) break;

                /* Track distance */
                r++;

                /* Simulate the step */
                y1 += ddy[d];
                x1 += ddx[d];

                /* Obtain the grid */
                ag = &borg_grids[y1][x1];

                /* Lets make one more check that we are not bouncing */
                if ((borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 7 / 10 &&
                    ((track_step.num > 2 &&
                        (track_step.y[track_step.num - 2] == y1 &&
                        track_step.x[track_step.num - 2] == x1 &&
                        track_step.y[track_step.num - 3] == c_y &&
                        track_step.x[track_step.num - 3] == c_x)))) ||
                        time_this_panel >= 300) break;

                /* Require floor */
                if (!borg_cave_floor_grid(ag) || 
                    (ag->feat == FEAT_LAVA && !borg_skill[BI_IFIRE]))
                    break;

                /* Require it to be somewhat close */
                if (r >= 10) break;

                /* Check danger of that spot */
                p1 = borg_danger(y1, x1, 1, true, false);
                if (p1 >= pos_danger) break;

                /* make sure it is not dangerous to take the first step; unless surrounded. */
                if (r == 1)
                {
                    /* Not surrounded or surrounded and ignoring*/
                    if (!borg_surround || (borg_surround && goal_ignoring))
                    {
                        if (p1 >= borg_skill[BI_CURHP] * 4 / 10) break;

                        /* Ought to be worth it */;
                        if (p1 > pos_danger * 5 / 10) break;
                    }
                    else
                        /* Surrounded, try to back-up */
                    {
                        if (borg_skill[BI_CLEVEL] >= 20)
                        {
                            if (p1 >= (b_r <= 5 ? borg_skill[BI_CURHP] * 15 / 10 : borg_skill[BI_CURHP])) break;
                        }
                        else
                        {
                            if (p1 >= borg_skill[BI_CURHP] * 4) break;
                        }

                    }

                    /*
                     * Skip this grid if it is adjacent to a monster.  He will just hit me
                     * when I land on that grid.
                     */
                    for (ii = 1; ii < borg_kills_nxt; ii++)
                    {
                        borg_kill* kill;

                        /* Monster */
                        kill = &borg_kills[ii];

                        /* Skip dead monsters */
                        if (!kill->r_idx) continue;

                        /* Require current knowledge */
                        if (kill->when < borg_t - 2) continue;

                        /* Check distance -- 1 grid away */
                        if (borg_distance(kill->y, kill->x, y1, x1) <= 1 &&
                            kill->speed > borg_skill[BI_SPEED] && !borg_surround) break;
                    }
                }


                /* Skip monsters */
                if (ag->kill) break;

                /* Skip traps */
                if (ag->trap && !ag->glyph) break;

                /* Safe arrival */
                if ((x1 == x2) && (y1 == y2))
                {
                    /* Save distance */
                    b_r = r;
                    b_p = p1;

                    /* Save location */
                    b_x = x2;
                    b_y = y2;

                    /* Done */
                    break;
                }
            }
        }

        /* Retreat */
        if (b_r >= 0)
        {
            /* Save direction */
            b_d = borg_goto_dir(c_y, c_x, b_y, b_x);

            /* Hack -- set goal */
            g_x = c_x + ddx[b_d];
            g_y = c_y + ddy[b_d];

            /* Note */
            borg_note(format("# Retreating to %d,%d (distance %d) via %d,%d (%d > %d)",
                b_y, b_x, b_r, g_y, g_x, pos_danger, b_p));

            /* Strategic retreat */
            borg_keypress(I2D(b_d));

            /* Reset my Movement and Flow Goals */
            goal = 0;

            /* Success */
            return (true);
        }
    }

    /*** Escape if possible ***/

    /* Attempt to escape via spells */
    if (borg_escape(pos_danger))
    {
        /* increment the escapes this level counter */
        borg_escapes++;

        /* Clear any Flow queues */
        goal = 0;

        /* Success */
        return (true);
    }

    /*** Back away ***/
    /* Do not back up if
     * 1) we are nasty (poisoned, blind, confused etc
     * 2) we are boosting our avoidance because we are stuck
     * 3) we are in a sweet Morgoth position (sea of runes)
     * 4) the monster causing concern is asleep
     * 5) we are not in a vault
     * 6) loads of HP
     */
    if (((pos_danger > (avoidance * 4 / 10) && !nasty && !borg_no_retreat) || (borg_surround && pos_danger != 0)) &&
        !borg_morgoth_position && (borg_t - borg_t_antisummon >= 50) && !borg_skill[BI_ISCONFUSED] &&
        !square_isvault(cave, loc(c_x, c_y)) &&
        borg_skill[BI_CURHP] < 500)
    {
        int i = -1, b_i = -1;
        int k = -1, b_k = -1;
        int f = -1, b_f = -1;
        int g_k = 0;
        int ii;
        bool adjacent_monster = false;

        /* Current danger */
        b_k = pos_danger;

        /* Fake the danger down if surounded so that he can move. */
        if (borg_surround) b_k = (b_k * 12 / 10);

        /* Check the freedom */
        b_f = borg_freedom(c_y, c_x);

        /* Attempt to find a better grid */
        for (i = 0; i < 8; i++)
        {
            int x = c_x + ddx_ddd[i];
            int y = c_y + ddy_ddd[i];

            /* Access the grid */
            borg_grid* ag = &borg_grids[y][x];

            /* Cant if confused: no way to predict motion */
            if (borg_skill[BI_ISCONFUSED]) continue;

            /* Skip walls/doors */
            if (!borg_cave_floor_grid(ag)) continue;

            /* Skip monster grids */
            if (ag->kill) continue;

            /* Mega-Hack -- skip stores XXX XXX XXX */
            if (feat_is_shop(ag->feat)) continue;

            /* Mega-Hack -- skip traps XXX XXX XXX */
            if (ag->trap && !ag->glyph) break;

            /* If i was here last round and 3 rounds ago, suggesting a "bounce" */
            if ((borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 7 / 10 &&
                ((track_step.num > 2 &&
                 (track_step.y[track_step.num - 2] == y &&
                  track_step.x[track_step.num - 2] == x &&
                  track_step.y[track_step.num - 3] == c_y &&
                  track_step.x[track_step.num - 3] == c_x)))) ||
                time_this_panel >= 300) continue;

            /*
             * Skip this grid if it is adjacent to a monster.  He will just hit me
             * when I land on that grid.
             */
            for (ii = 1; ii < borg_kills_nxt; ii++)
            {
                borg_kill* kill;

                /* Monster */
                kill = &borg_kills[ii];

                /* Skip dead monsters */
                if (!kill->r_idx) continue;

                /* Require current knowledge */
                if (kill->when < borg_t - 2) continue;

                /* Check distance -- 1 grid away */
                if (borg_distance(kill->y, kill->x, y, x) <= 1 && !borg_surround) adjacent_monster = true;

                /* Check distance -- 2 grids away and he is faster than me */
                if (borg_distance(kill->y, kill->x, y, x) <= 2 &&
                    kill->speed > borg_skill[BI_SPEED] && !borg_surround) adjacent_monster = true;
            }

            /* Skip this grid consideration because it is next to a monster */
            if (adjacent_monster == true) continue;

            /* Extract the danger there */
            k = borg_danger(y, x, 1, true, false);

            /* Skip this grid if danger is higher than my HP.
             * Take my chances with fighting.
             */
            if (k > avoidance) continue;

            /* Skip this grid if it is not really worth backing up.  Look for a 40%
             * reduction in the danger if higher level.  If the danger of the new grid
             * is close to the danger of my current grid, I'll stay and fight.
             */
            if (borg_skill[BI_MAXCLEVEL] >= 35 && k > b_k * 6 / 10) continue;

            /* Skip this grid if it is not really worth backing up.  If the danger of the new grid
             * is close to the danger of my current grid, I'll stay and fight unless I am low
             * level and there is an adjacent monster.
             */
            if (borg_skill[BI_MAXCLEVEL] < 35 && adjacent_monster == false && k > b_k * 8 / 10) continue;

            /* Skip higher danger */
            /*     note: if surrounded, then b_k has been adjusted to a higher number to make his current
             *     grid seem more dangerous.  This will encourage him to Back-Up.
             */
            if (k > b_k) continue;

            /* Record the danger of this prefered grid */
            g_k = k;

            /* Check the freedom there */
            f = borg_freedom(y, x);

            /* Danger is the same, so look at the nature of the grid */
            if (b_k == k)
            {
                /* If I am low level, reward backing-up if safe */
                if (borg_skill[BI_CLEVEL] <= 10 && borg_skill[BI_CDEPTH] &&
                    (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] ||
                        borg_skill[BI_CURSP] < borg_skill[BI_MAXSP]))
                {
                    /* do consider the retreat */
                }
                else
                {
                    /* Freedom of my grid is better than the next grid
                     * so stay put and fight.
                     */
                    if (b_f > f || borg_skill[BI_CDEPTH] >= 85) continue;
                }
            }

            /* Save the info */
            b_i = i; b_k = k; b_f = f;
        }

        /* Back away */
        if (b_i >= 0)
        {
            /* Hack -- set goal */
            g_x = c_x + ddx_ddd[b_i];
            g_y = c_y + ddy_ddd[b_i];

            /* Note */
            borg_note(format("# Backing up to %d,%d (%d > %d)",
                g_x, g_y, pos_danger, g_k));

            /* Back away from danger */
            borg_keypress(I2D(ddd[b_i]));

            /* Reset my Movement and Flow Goals */
            goal = 0;

            /* Success */
            return (true);
        }

    }


    /*** Cures ***/

    /* cure confusion, second check, first (slightly different) in borg_heal */
    if (borg_skill[BI_ISCONFUSED])
    {
        if (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 300 &&
            (borg_quaff_potion(sv_potion_healing) ||
                borg_quaff_potion(sv_potion_star_healing) ||
                borg_quaff_potion(sv_potion_life)))
        {
            borg_note("# Healing.  Confusion.");
            return (true);
        }
        if (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind) ||
            borg_quaff_potion(sv_potion_cure_serious) ||
            borg_quaff_crit(false) ||
            borg_quaff_potion(sv_potion_healing) ||
            borg_use_staff_fail(sv_staff_healing))
        {
            borg_note("# Healing.  Confusion.");
            return (true);
        }
    }

    /* Hack -- cure fear when afraid */
    if ((borg_skill[BI_ISAFRAID] && !borg_skill[BI_CRSFEAR]) &&
        (randint0(100) < 70 ||
            (borg_class == CLASS_WARRIOR && borg_skill[BI_AMISSILES] <= 0)))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind) ||
            borg_quaff_potion(sv_potion_boldness) ||
            borg_quaff_potion(sv_potion_heroism) ||
            borg_quaff_potion(sv_potion_berserk) ||
            borg_spell_fail(BERSERK_STRENGTH, 25) || /* berserk */
            borg_spell_fail(HEROISM, 25) || /* hero */
            borg_activate_item(act_cure_paranoia) ||
            borg_activate_item(act_hero) ||
            borg_activate_item(act_shero) ||
            borg_activate_item(act_cure_mind) ||
            borg_activate_item(act_rage_bless_resist) ||
            borg_activate_item(act_rem_fear_pois) ||
            borg_spell_fail(HOLY_WORD, 25))
        {
            return (true);
        }
    }


    /*** Note impending death XXX XXX XXX ***/

    /* Flee from low hit-points */
    if (((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3) ||
        ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) && borg_skill[BI_CURHP] < (borg_skill[BI_CLEVEL] * 3))) &&
        (borg_skill[BI_ACCW] < 3) &&
        (borg_skill[BI_AHEAL] < 1))
    {
        /* Flee from low hit-points */
        if (borg_skill[BI_CDEPTH] && (randint0(100) < 25))
        {
            /* Start leaving */
            if (!goal_leaving)
            {
                /* Flee */
                borg_note("# Leaving (low hit-points)");

                /* Start leaving */
                goal_leaving = true;

            }
            /* Start fleeing */
            if (!goal_fleeing)
            {
                /* Flee */
                borg_note("# Fleeing (low hit-points)");

                /* Start fleeing */
                goal_fleeing = true;
            }

        }
    }

    /* Flee from bleeding wounds or poison and no heals */
    if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) && (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2))
    {
        /* Flee from bleeding wounds */
        if (borg_skill[BI_CDEPTH] && (randint0(100) < 25))
        {
            /* Start leaving */
            if (!goal_leaving)
            {
                /* Flee */
                borg_note("# Leaving (bleeding/posion)");

                /* Start leaving */
                goal_leaving = true;
            }

            /* Start fleeing */
            if (!goal_fleeing)
            {
                /* Flee */
                borg_note("# Fleeing (bleeding/poison)");

                /* Start fleeing */
                goal_fleeing = true;
            }
        }
    }

    /* Emergency check on healing.  Borg_heal has already been checked but
     * but we did not use our ez_heal potions.  All other attempts to save
     * ourself have failed.  Use the ez_heal if I have it.
     */
    if ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 10 || /* dangerously low HP -OR-*/
        (pos_danger > borg_skill[BI_CURHP] && /* extreme danger -AND-*/
            (borg_skill[BI_ATELEPORT] + borg_skill[BI_AESCAPE] <= 2 && borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 4)) || /* low on escapes */
        (borg_skill[BI_AEZHEAL] > 5 && borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 4) || /* moderate danger, lots of heals */
        (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 600 && borg_fighting_unique && borg_skill[BI_CDEPTH] >= 85)) && /* moderate danger, unique, deep */
        (borg_quaff_potion(sv_potion_star_healing) ||
            borg_quaff_potion(sv_potion_healing) ||
            borg_quaff_potion(sv_potion_life)))
    {
        borg_note("# Using reserve EZ_Heal.");
        return (true);
    }

    /* Hack -- use "recall" to flee if possible */
    if (goal_fleeing && !goal_fleeing_munchkin && !goal_fleeing_to_town && borg_skill[BI_CDEPTH] >= 1 && (borg_recall()))
    {
        /* Note */
        borg_note("# Fleeing the level (recall)");

        /* Success */
        return (true);
    }

    /* If I am waiting for recall,and in danger, buy time with
     * phase and cure_anythings.
     */
    if (goal_recalling && (pos_danger > avoidance * 2))
    {
        if (!borg_skill[BI_ISCONFUSED] && !borg_skill[BI_ISBLIND] && borg_skill[BI_MAXSP] > 60 &&
            borg_skill[BI_CURSP] < (borg_skill[BI_CURSP] / 4) && borg_quaff_potion(sv_potion_restore_mana))
        {
            borg_note("# Buying time waiting for Recall.(1)");
            return (true);
        }

        if (borg_caution_phase(50, 1) &&
            (borg_read_scroll(sv_scroll_phase_door) ||
                borg_spell_fail(PHASE_DOOR, 30) ||
                borg_spell_fail(PORTAL, 30) ||
                borg_activate_item(act_tele_phase)))
        {
            borg_note("# Buying time waiting for Recall.(2)");
            return (true);
        }

        if ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] < 100) &&
            (borg_quaff_crit(true) ||
                borg_quaff_potion(sv_potion_cure_serious) ||
                borg_quaff_potion(sv_potion_cure_light)))
        {
            borg_note("# Buying time waiting for Recall.(3)");
            return (true);
        }

        if ((borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] > 150) &&
            (borg_zap_rod(sv_rod_healing) ||
                borg_quaff_potion(sv_potion_healing) ||
                borg_quaff_potion(sv_potion_star_healing) ||
                borg_quaff_potion(sv_potion_life) ||
                borg_quaff_crit(true) ||
                borg_quaff_potion(sv_potion_cure_serious) ||
                borg_quaff_potion(sv_potion_cure_light)))
        {
            borg_note("# Buying time waiting for Recall.(4)");
            return (true);
        }

    }



    /* if I am gonna die next round, and I have no way to escape
     * use the unknown stuff (if I am low level).
     */
    if (pos_danger > (borg_skill[BI_CURHP] * 4) && borg_skill[BI_CLEVEL] < 20 && !borg_skill[BI_MAXSP])
    {
        if (borg_use_unknown() ||
            borg_read_unknown() ||
            borg_quaff_unknown() ||
            borg_eat_unknown()) return (true);
    }

    /* Nothing */
    return (false);
}


/*
 * New method for handling attacks, missiles, and spells
 *
 * Every turn, we evaluate every known method of causing damage
 * to monsters, and evaluate the "reward" inherent in each of
 * the known methods which is usable at that time, and then
 * we actually use whichever method, if any, scores highest.
 *
 * For each attack, we need a function which will determine the best
 * possible result of using that attack, and return its value.  Also,
 * if requested, the function should actually perform the action.
 *
 * Note that the functions should return zero if the action is not
 * usable, or if the action is not useful.
 *
 * These functions need to apply some form of "cost" evaluation, to
 * prevent the use of expensive spells with minimal reward.  Also,
 * we should always prefer attacking by hand to using spells if the
 * damage difference is "small", since there is no "cost" in making
 * a physical attack.
 *
 * We should take account of "spell failure", as well as "missile
 * missing" and "blow missing" probabilities.
 *
 * Note that the functions may store local state information when
 * doing a "simulation" and then they can use this information if
 * they are asked to implement their strategy.
 *
 * There are several types of damage inducers:
 *
 *   Attacking physically
 *   Launching missiles
 *   Throwing objects
 *   Casting spells
 *   Praying prayers
 *   Using wands
 *   Using rods
 *   Using staffs
 *   Using scrolls
 *   Activating Artifacts
 *   Activate Dragon Armour
 */
enum
{
    BF_REST,
    BF_THRUST,
    BF_OBJECT,
    BF_LAUNCH,
    BF_SPELL_MAGIC_MISSILE,
    BF_SPELL_MAGIC_MISSILE_RESERVE, /* 20 */
    BF_SPELL_STINK_CLOUD,
    BF_SPELL_LIGHT_BEAM,
    BF_SPELL_COLD_BOLT,
    BF_SPELL_STONE_TO_MUD,
    BF_SPELL_SLOW_MONSTER,
    BF_SPELL_SLEEP_III,
    BF_SPELL_FIRE_BALL,
    BF_SPELL_SHOCK_WAVE,
    BF_SPELL_EXPLOSION,
    BF_SPELL_CONFUSE_MONSTER,
    BF_SPELL_COLD_STORM,
    BF_SPELL_METEOR_SWARM,
    BF_SPELL_RIFT, /* 40 */
    BF_SPELL_MANA_STORM,
    BF_SPELL_BLIND_CREATURE,
    BF_SPELL_TRANCE,
    BF_PRAYER_HOLY_ORB_BALL,
    BF_PRAYER_DISP_UNDEAD,
    BF_PRAYER_DISP_EVIL,
    BF_PRAYER_DISP_SPIRITS,
    BF_PRAYER_HOLY_WORD, /* 50 */
    BF_SPELL_ANNIHILATE,

    BF_SPELL_ELECTRIC_ARC, /* new spells in 4.x */
    BF_SPELL_ACID_SPRAY,
    BF_SPELL_MANA_BOLT,
    BF_SPELL_THRUST_AWAY,
    BF_SPELL_LIGHTNING_STRIKE,
    BF_SPELL_EARTH_RISING,
    BF_SPELL_VOLCANIC_ERUPTION,
    BF_SPELL_RIVER_OF_LIGHTNING,
    BF_SPELL_SPEAR_OF_OROME,
    BF_SPELL_LIGHT_OF_MANWE,
    BF_SPELL_NETHER_BOLT,
    BF_SPELL_TAP_UNLIFE,
    BF_SPELL_CRUSH,
    BF_SPELL_SLEEP_EVIL,
    BF_SPELL_DISENCHANT,
    BF_SPELL_FRIGHTEN,
    BF_SPELL_VAMPIRE_STRIKE,
    BF_PRAYER_DISPEL_LIFE,
    BF_SPELL_DARK_SPEAR,
    BF_SPELL_UNLEASH_CHAOS,
    BF_SPELL_STORM_OF_DARKNESS,
    BF_SPELL_CURSE,
    BF_SPELL_WHIRLWIND_ATTACK,
    BF_SPELL_LEAP_INTO_BATTLE,
    BF_SPELL_MAIM_FOE,
    BF_SPELL_HOWL_OF_THE_DAMNED,


    BF_ROD_ELEC_BOLT,
    BF_ROD_COLD_BOLT,
    BF_ROD_ACID_BOLT,
    BF_ROD_FIRE_BOLT,
    BF_ROD_LIGHT_BEAM,
    BF_ROD_DRAIN_LIFE,
    BF_ROD_ELEC_BALL, /* 60 */
    BF_ROD_COLD_BALL,
    BF_ROD_ACID_BALL,
    BF_ROD_FIRE_BALL,
    BF_ROD_SLOW_MONSTER,
    BF_ROD_SLEEP_MONSTER,
    BF_ROD_UNKNOWN,
    BF_STAFF_SLEEP_MONSTERS,
    BF_STAFF_SLOW_MONSTERS,
    BF_STAFF_DISPEL_EVIL,
    BF_STAFF_POWER,
    BF_STAFF_HOLINESS,
    BF_WAND_UNKNOWN,
    BF_WAND_MAGIC_MISSILE,
    BF_WAND_ELEC_BOLT,
    BF_WAND_COLD_BOLT,
    BF_WAND_ACID_BOLT,
    BF_WAND_FIRE_BOLT,
    BF_WAND_SLOW_MONSTER,
    BF_WAND_HOLD_MONSTER,
    BF_WAND_CONFUSE_MONSTER,
    BF_WAND_FEAR_MONSTER,
    BF_WAND_ANNIHILATION,
    BF_WAND_DRAIN_LIFE,
    BF_WAND_LIGHT_BEAM,
    BF_WAND_STINKING_CLOUD,
    BF_WAND_ELEC_BALL,
    BF_WAND_COLD_BALL,
    BF_WAND_ACID_BALL,
    BF_WAND_FIRE_BALL,
    BF_WAND_WONDER,
    BF_WAND_DRAGON_COLD,
    BF_WAND_DRAGON_FIRE,
    BF_EF_FIRE1,
    BF_EF_FIRE2,
    BF_EF_FIRE3,
    BF_EF_FROST1,
    BF_EF_FROST2,
    BF_EF_FROST3,
    BF_EF_FROST4,
    BF_EF_DRAIN_LIFE1,
    BF_EF_DRAIN_LIFE2,
    BF_EF_STINKING_CLOUD,
    BF_EF_CONFUSE,
    BF_EF_ARROW,
    BF_EF_MISSILE,
    BF_EF_SLEEP,
    BF_EF_LIGHTNING_BOLT,
    BF_EF_ACID1,
    BF_EF_DISP_EVIL,
    BF_EF_MANA_BOLT,
    BF_EF_STAR_BALL, /* Razorback and Mediator */
    BF_EF_STARLIGHT2,
    BF_EF_STARLIGHT,
    BF_RING_ACID,
    BF_RING_FIRE,
    BF_RING_ICE,
    BF_RING_LIGHTNING,
    BF_DRAGON_BLUE,
    BF_DRAGON_WHITE,
    BF_DRAGON_BLACK,
    BF_DRAGON_GREEN,
    BF_DRAGON_RED,
    BF_DRAGON_MULTIHUED,
    BF_DRAGON_GOLD,
    BF_DRAGON_CHAOS,
    BF_DRAGON_LAW,
    BF_DRAGON_BALANCE,
    BF_DRAGON_SHINING,
    BF_DRAGON_POWER,
    BF_MAX
};



/*
 * Guess how much damage a physical attack will do to a monster
 */
static int borg_thrust_damage_one(int i)
{
    int dam;
    int mult;

    borg_kill* kill;

    struct monster_race* r_ptr;

    borg_item* item;

    int chance;

    /* Examine current weapon */
    item = &borg_items[INVEN_WIELD];

    /* Monster record */
    kill = &borg_kills[i];

    /* Monster race */
    r_ptr = &r_info[kill->r_idx];

    /* Damage */
    dam = (item->dd * (item->ds + 1) / 2);

    /* here is the place for slays and such */
    mult = 1;

    if (((borg_skill[BI_WS_ANIMAL]) && (rf_has(r_ptr->flags, RF_ANIMAL))) ||
        ((borg_skill[BI_WS_EVIL]) && (rf_has(r_ptr->flags, RF_EVIL))))
        mult = 2;
    if (((borg_skill[BI_WS_UNDEAD]) && (rf_has(r_ptr->flags, RF_UNDEAD))) ||
        ((borg_skill[BI_WS_DEMON]) && (rf_has(r_ptr->flags, RF_DEMON))) ||
        ((borg_skill[BI_WS_ORC]) && (rf_has(r_ptr->flags, RF_ORC))) ||
        ((borg_skill[BI_WS_TROLL]) && (rf_has(r_ptr->flags, RF_TROLL))) ||
        ((borg_skill[BI_WS_GIANT]) && (rf_has(r_ptr->flags, RF_GIANT))) ||
        ((borg_skill[BI_WS_DRAGON]) && (rf_has(r_ptr->flags, RF_DRAGON))) ||
        ((borg_skill[BI_WB_ACID]) && !(rf_has(r_ptr->flags, RF_IM_ACID))) ||
        ((borg_skill[BI_WB_FIRE]) && !(rf_has(r_ptr->flags, RF_IM_FIRE))) ||
        ((borg_skill[BI_WB_COLD]) && !(rf_has(r_ptr->flags, RF_IM_COLD))) ||
        ((borg_skill[BI_WB_POIS]) && !(rf_has(r_ptr->flags, RF_IM_POIS))) ||
        ((borg_skill[BI_WB_ELEC]) && !(rf_has(r_ptr->flags, RF_IM_ELEC))))
        mult = 3;
    if (((borg_skill[BI_WK_UNDEAD]) && (rf_has(r_ptr->flags, RF_UNDEAD))) ||
        ((borg_skill[BI_WK_DEMON]) && (rf_has(r_ptr->flags, RF_DEMON))) ||
        ((borg_skill[BI_WK_DRAGON]) && (rf_has(r_ptr->flags, RF_DRAGON))))
        mult = 5;

    /* add the multiplier */
    dam *= mult;

    /* add weapon bonuses */
    dam += item->to_d;

    /* add player bonuses */
    dam += borg_skill[BI_TODAM];

    /* multiply the damage for the whole round of attacks */
    dam *= borg_skill[BI_BLOWS];

    /* Bonuses for combat */
    chance = (borg_skill[BI_THN] + ((borg_skill[BI_TOHIT] + item->to_h) * 3));

    /* Chance of hitting the monsters AC */
    if (chance < (r_ptr->ac * 3 / 4) * 8 / 10) dam = 0;

    /* 5% automatic success/fail */
    if (chance > 95) chance = 95;
    if (chance < 5) chance = 5;

    /* add 10% to chance to give a bit more wieght to weapons */
    if (borg_skill[BI_CLEVEL] > 15) chance += 10;

    /* Mages with Mana do not get that bonus, they should cast */
    if ((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER) &&
        borg_skill[BI_CURSP] > 1) chance -= 10;

    /* reduce damage by the % chance to hit */
    dam = (dam * chance) / 100;

    /* Try to place a minimal amount of damage */
    if (dam <= 0) dam = 1;

    /* Limit damage to twice maximal hitpoints */
    if (dam > kill->power * 2 && !rf_has(r_ptr->flags, RF_UNIQUE)) dam = kill->power * 2;

    /* Reduce the damage if a mage, they should not melee if they can avoid it */
    if ((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER) &&
        borg_skill[BI_MAXCLEVEL] < 40 &&
        borg_skill[BI_CURSP] > 1) dam = (dam * 8 / 10) + 1;

    /*
     * Enhance the preceived damage on Uniques.  This way we target them
     * Keep in mind that he should hit the uniques but if he has a
     * x5 great bane of dragons, he will tend attack the dragon since the
     * precieved (and actual) damage is higher.  But don't select
     * the town uniques (maggot does no damage)
     *
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg_skill[BI_CDEPTH] >= 1) dam += (dam * 5);

    /* Hack -- ignore Maggot until later.  Player will chase Maggot
     * down all accross the screen waking up all the monsters.  Then
     * he is stuck in a comprimised situation.
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg_skill[BI_CDEPTH] == 0)
    {
        dam = dam * 2 / 3;

        /* Dont hunt maggot until later */
        if (borg_skill[BI_CLEVEL] < 5) dam = 0;
    }

    /* give a small bonus for whacking a breeder */
    if (rf_has(r_ptr->flags, RF_MULTIPLY))
        dam = (dam * 3 / 2);

    /* Enhance the perceived damage to summoner in order to influence the
     * choice of targets.
     */
    if ((rsf_has(r_ptr->spell_flags, RSF_S_KIN)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HOUND)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_AINU)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_DEMON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE)))
        dam += ((dam * 3) / 2);

    /*
     * Apply massive damage bonus to Questor monsters to
     * encourage borg to strike them.
     */
    if (rf_has(r_ptr->flags, RF_QUESTOR)) dam += (dam * 5);

    /* Damage */
    return (dam);
}


/*
 * Simulate/Apply the optimal result of making a physical attack
 */
extern int borg_attack_aux_thrust(void)
{
    int p, dir;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid* ag;

    borg_kill* kill;

    /* Too afraid to attack */
    if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR]) return (0);


    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++)
    {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require "adjacent" */
        if (borg_distance(c_y, c_x, y, x) > 1) continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill);

        /* No damage */
        if (d <= 0) continue;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
        {
            /* Calculate danger */
            p = borg_danger_aux(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg_skill[BI_CDEPTH] && !kill->awake) continue;


        /* Calculate "danger" to player */
        p = borg_danger_aux(c_y, c_x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d)) continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0) return (0);

    /* Simulation */
    if (borg_simulate) return (b_d);

    /* Save the location */
    g_x = borg_temp_x[b_i];
    g_y = borg_temp_y[b_i];

    ag = &borg_grids[g_y][g_x];
    kill = &borg_kills[ag->kill];

    /* Note */
    borg_note(format("# Facing %s at (%d,%d) who has %d Hit Points.", (r_info[kill->r_idx].name), g_y, g_x, kill->power));
    borg_note(format("# Attacking with weapon '%s'",
        borg_items[INVEN_WIELD].desc));

    /* Get a direction for attacking */
    dir = borg_extract_dir(c_y, c_x, g_y, g_x);

    /* Attack the grid */
    borg_keypress('+');
    borg_keypress(I2D(dir));

    /* Success */
    return (b_d);
}




/*
 * Target a location.  Can be used alone or at "Direction?" prompt.
 *
 * Warning -- This will only work for locations on the current panel
 */
bool borg_target(int y, int x)
{
    int x1, y1, x2, y2;

    borg_grid* ag;
    borg_kill* kill;

    ag = &borg_grids[y][x];
    kill = &borg_kills[ag->kill];


    /* Log */
    /* Report a little bit */
    if (ag->kill)
    {
        borg_note(format("# Targeting %s who has %d Hit Points (%d,%d).", (r_info[kill->r_idx].name), kill->power, y, x));
    }
    else
    {
        borg_note(format("# Targetting location (%d,%d)", y, x));
    }

    /* Target mode */
    borg_keypress('*');

    /* Target a location */
    borg_keypress('p');

    /* Determine "path" */
    x1 = c_x;
    y1 = c_y;
    x2 = x;
    y2 = y;

    /* Move to the location (diagonals) */
    for (; (y1 < y2) && (x1 < x2); y1++, x1++) borg_keypress('3');
    for (; (y1 < y2) && (x1 > x2); y1++, x1--) borg_keypress('1');
    for (; (y1 > y2) && (x1 < x2); y1--, x1++) borg_keypress('9');
    for (; (y1 > y2) && (x1 > x2); y1--, x1--) borg_keypress('7');

    /* Move to the location */
    for (; y1 < y2; y1++) borg_keypress('2');
    for (; y1 > y2; y1--) borg_keypress('8');
    for (; x1 < x2; x1++) borg_keypress('6');
    for (; x1 > x2; x1--) borg_keypress('4');

    /* Select the target */
    borg_keypress('5');

    /* Carry these variables to be used on reporting spell
     * pathway
     */
    borg_target_y = y;
    borg_target_x = x;

    /* Success */
    return (true);
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
    int n_x, n_y;
    bool found = false;
    bool y_hall = false;
    bool x_hall = false;

    borg_grid* ag;
    struct monster_race* r_ptr;
    borg_kill* kill;

    borg_note(format("# Perhaps wall near targetted location (%d,%d)", y, x));

    /* Determine "path" */
    n_x = c_x;
    n_y = c_y;

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

    if ((borg_grids[c_y + 1][c_x].feat == FEAT_FLOOR &&
        borg_grids[c_y - 1][c_x].feat == FEAT_FLOOR &&
        (borg_grids[c_y + 2][c_x].feat == FEAT_FLOOR ||
            borg_grids[c_y - 2][c_x].feat == FEAT_FLOOR)) &&
        (borg_grids[c_y][c_x + 1].feat != FEAT_FLOOR &&
            borg_grids[c_y][c_x - 1].feat != FEAT_FLOOR))
        x_hall = true;

    /* check for 'in a hall' y axis.
     * Again, we want to place the suspected wall off our
     * hallway.
     */
    if ((borg_grids[c_y][c_x + 1].feat == FEAT_FLOOR &&
        borg_grids[c_y][c_x - 1].feat == FEAT_FLOOR &&
        (borg_grids[c_y][c_x + 2].feat == FEAT_FLOOR ||
            borg_grids[c_y][c_x - 2].feat == FEAT_FLOOR)) &&
        (borg_grids[c_y + 1][c_x].feat != FEAT_FLOOR &&
            borg_grids[c_y - 1][c_x].feat != FEAT_FLOOR))
        y_hall = true;

    while (1)
    {
        ag = &borg_grids[n_y][n_x];
        kill = &borg_kills[ag->kill];
        r_ptr = &r_info[kill->r_idx];

        if (rf_has(r_ptr->flags, RF_PASS_WALL))
        {
            borg_note(format("# Guessing wall (%d,%d) under ghostly target (%d,%d)", n_y, n_x, n_y, n_x));
            borg_grids[n_y][n_x].feat = FEAT_GRANITE;
            found = true;
            return (found); /* not sure... should we return here? */
        }

        if (borg_grids[n_y][n_x].feat == FEAT_NONE &&
            ((n_y != c_y) || !y_hall) &&
            ((n_x != c_x) || !x_hall))
        {
            borg_note(format("# Guessing wall (%d,%d) near target (%d,%d)", n_y, n_x, y, x));
            borg_grids[n_y][n_x].feat = FEAT_GRANITE;
            found = true;
            return (found); /* not sure... should we return here?
                             maybe should mark ALL unknowns in path... */
        }

        /* Pathway found the target. */
        if (n_x == x && n_y == y)
        {
            /* end of the pathway */
            mmove2(&n_y, &n_x, y, x, c_y, c_x);
            borg_note(format("# Guessing wall (%d,%d) near target (%d,%d)", n_y, n_x, y, x));
            borg_grids[n_y][n_x].feat = FEAT_GRANITE;
            found = true;
            return (found);
        }

        /* Calculate the new location */
        mmove2(&n_y, &n_x, c_y, c_x, y, x);
    }
}

/* adapted from player_attack.c make_ranged_shot() */
static int borg_best_mult(borg_item* obj, struct monster_race* r_ptr)
{
    int i;
    int max_mult = 1;

    /* Brands */
    for (i = 1; i < z_info->brand_max; i++)
    {
        struct brand* brand = &brands[i];
        if (obj) 
        {
            /* Brand is on an object */
            if (!obj->brands[i]) continue;
        }
        else 
        {
            /* Temporary brand */
            if (!player_has_temporary_brand(player, i)) continue;
        }

        /* Is the monster vulnerable? */
        if (!rf_has(r_ptr->flags, brand->resist_flag))
        {
            int mult = brand->multiplier;
            if (brand->vuln_flag && rf_has(r_ptr->flags, brand->vuln_flag))
            {
                mult *= 2;
            }
            max_mult = MAX(mult, max_mult);
        }
    }

    /* Slays */
    for (i = 1; i < z_info->slay_max; i++)
    {
        struct slay* slay = &slays[i];
        if (obj) 
        {
            /* Slay is on an object */
            if (!obj->slays || !obj->slays[i]) continue;
        }
        else {
            /* Temporary slay */
            if (!player_has_temporary_slay(player, i)) continue;
        }

        if (rf_has(r_ptr->flags, slay->race_flag))
        {
            max_mult = MAX(slay->multiplier, max_mult);
        }
    }
    return max_mult;
}


/*
 * Guess how much damage a spell attack will do to a monster
 *
 * We only handle the "standard" damage types.
 *
 * We are paranoid about monster resistances
 *
 * He tends to waste all of his arrows on a monsters immediately adjacent
 * to him.  Then he has no arrows for the rest of the level.  We will
 * decrease the damage if the monster is adjacent and we are getting low
 * on missiles.
 *
 * We will also decrease the value of the missile attack on breeders or
 * high clevel borgs town scumming.
 */
int borg_launch_damage_one(int i, int dam, int typ, int ammo_location)
{
    int p1, p2 = 0;
    int j;
    bool borg_use_missile = false;
    int ii;
    int vault_grids = 0;
    int x, y;
    int k;
    bool gold_eater = false;
    int chance = 0;
    int bonus = 0;
    int cur_dis = 0;
    int armor = 0;

    borg_kill* kill;
    borg_grid* ag;

    struct monster_race* r_ptr;

    /* Monster record */
    kill = &borg_kills[i];

    /* Monster race */
    r_ptr = &r_info[kill->r_idx];

    /* How far away is the target? */
    cur_dis = borg_distance(c_y, c_x, kill->y, kill->x);

    /* Calculation our chance of hitting.  Player bonuses, Bow bonuses, Ammo Bonuses */
    bonus = (borg_skill[BI_TOHIT] + borg_items[INVEN_BOW].to_h + borg_items[ammo_location].to_h);
    chance = (borg_skill[BI_THB] + (bonus * BTH_PLUS_ADJ));
    armor = r_ptr->ac + cur_dis;

    /* Very quickly look for gold eating monsters */
    for (k = 0; k < 4; k++)
    {
        /* gold eater */
        if (r_ptr->blow[k].effect && borg_mon_blow_effect(r_ptr->blow[k].effect->name) == MONBLOW_EAT_GOLD) gold_eater = true;
    }

    /* Analyze the damage type */
    switch (typ)
    {
        /* Magic Missile */
    case BORG_ATTACK_MISSILE:
    break;

    case BORG_ATTACK_ARROW:
    {
        borg_item* bow = &borg_items[INVEN_BOW];
        borg_item* ammo = &borg_items[ammo_location];
        int mult = borg_best_mult(bow, r_ptr);
        mult = MAX(mult, borg_best_mult(ammo, r_ptr));
        dam *= mult;
        /* don't point blank non-uniques */
        if (cur_dis == 1 && !(rf_has(r_ptr->flags, RF_UNIQUE))) dam /= 5;
        /* Do I hit regularly? (80%)*/
        if (chance < armor * 8 / 10) dam = 0;
    }
    break;

    /* Pure damage */
    case BORG_ATTACK_MANA:
    if (borg_fighting_unique && borg_has[kv_potion_restore_mana] > 3)
        dam *= 2;
    break;

    /* Meteor -- powerful magic missile */
    case BORG_ATTACK_METEOR:
    break;


    /* Acid */
    case BORG_ATTACK_ACID:
    if (rf_has(r_ptr->flags, RF_IM_ACID)) dam = 0;
    break;

    /* Electricity */
    case BORG_ATTACK_ELEC:
    if (rf_has(r_ptr->flags, RF_IM_ELEC)) dam = 0;
    break;

    /* Fire damage */
    case BORG_ATTACK_FIRE:
    if (rf_has(r_ptr->flags, RF_IM_FIRE)) dam = 0;
    if ((rf_has(r_ptr->flags, RF_HURT_FIRE))) dam *= 2;
    break;

    /* Cold */
    case BORG_ATTACK_COLD:
    if (rf_has(r_ptr->flags, RF_IM_COLD)) dam = 0;
    if (rf_has(r_ptr->flags, RF_HURT_COLD)) dam *= 2;
    break;

    /* Poison */
    case BORG_ATTACK_POIS:
    if (rf_has(r_ptr->flags, RF_IM_POIS)) dam = 0;
    break;

    /* Ice */
    case BORG_ATTACK_ICE:
    if (rf_has(r_ptr->flags, RF_IM_COLD)) dam = 0;
    break;


    /* Holy Orb */
    case BORG_ATTACK_HOLY_ORB:
    if (rf_has(r_ptr->flags, RF_EVIL)) dam *= 2;
    break;

    /* dispel undead */
    case BORG_ATTACK_DISP_UNDEAD:
    if (!(rf_has(r_ptr->flags, RF_UNDEAD))) dam = 0;
    break;

    /* dispel spirits */
    case BORG_ATTACK_DISP_SPIRITS:
    if (!(rf_has(r_ptr->flags, RF_SPIRIT))) dam = 0;
    break;

    /*  Dispel Evil */
    case BORG_ATTACK_DISP_EVIL:
    if (!(rf_has(r_ptr->flags, RF_EVIL))) dam = 0;
    break;

    /*  Dispel life */
    case BORG_ATTACK_DRAIN_LIFE:
    if (!(rf_has(r_ptr->flags, RF_NONLIVING))) dam = 0;
    if (!(rf_has(r_ptr->flags, RF_UNDEAD))) dam = 0;
    break;

    /*  Holy Word */
    case BORG_ATTACK_HOLY_WORD:
    if (!(rf_has(r_ptr->flags, RF_EVIL))) dam = 0;
    break;


    /* Weak Lite */
    case BORG_ATTACK_LIGHT_WEAK:
    if (!(rf_has(r_ptr->flags, RF_HURT_LIGHT))) dam = 0;
    break;


    /* Drain Life */
    case BORG_ATTACK_OLD_DRAIN:
    if (borg_distance(c_y, c_x, kill->y, kill->x) == 1) dam /= 5;
    if ((rf_has(r_ptr->flags, RF_UNDEAD)) ||
        (rf_has(r_ptr->flags, RF_DEMON)) ||
        (strchr("Egv", r_ptr->d_char)))
    {
        dam = 0;
    }
    break;

    /* Stone to Mud */
    case BORG_ATTACK_KILL_WALL:
    if (!(rf_has(r_ptr->flags, RF_HURT_ROCK))) dam = 0;
    break;

    /* New mage spell */
    case BORG_ATTACK_NETHER:
    {
        if (rf_has(r_ptr->flags, RF_UNDEAD))
        {
            dam = 0;
        }
        else if (rsf_has(r_ptr->spell_flags, RSF_BR_NETH))
        {
            dam *= 3; dam /= 9;
        }
        else if (rf_has(r_ptr->flags, RF_EVIL))
        {
            dam /= 2;
        }
    }
    break;

    /* New mage spell */
    case BORG_ATTACK_CHAOS:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_CHAO))
    {
        dam *= 3; dam /= 9;
    }
    /* If the monster is Unique full damage ok.
     * Otherwise, polymorphing will reset HP
     */
    if (!(rf_has(r_ptr->flags, RF_UNIQUE))) dam = -999;
    break;

    /* New mage spell */
    case BORG_ATTACK_GRAVITY:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_GRAV))
    {
        dam *= 3; dam /= 9;
    }
    break;

    /* New mage spell */
    case BORG_ATTACK_SHARD:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_SHAR))
    {
        dam *= 3; dam /= 9;
    }
    break;

    /* New mage spell */
    case BORG_ATTACK_SOUND:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_SOUN))
    {
        dam *= 3; dam /= 9;
    }
    break;

    /* Weird attacks */
    case BORG_ATTACK_PLASMA:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_PLAS))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_CONFU:
    if (rf_has(r_ptr->flags, RF_NO_CONF))
    {
        dam = 0;
    }
    break;

    case BORG_ATTACK_DISEN:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_DISE))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_NEXUS:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_NEXU))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_FORCE:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_WALL))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_INERTIA:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_INER))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_TIME:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_TIME))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_LIGHT:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_LIGHT))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_DARK:
    if (rsf_has(r_ptr->spell_flags, RSF_BR_DARK))
    {
        dam *= 3; dam /= 9;
    }
    break;

    case BORG_ATTACK_WATER:
    if (rsf_has(r_ptr->spell_flags, RSF_BA_WATE))
    {
        dam *= 3; dam /= 9;
    }
    dam /= 2;
    break;


    /* Various */
    case BORG_ATTACK_OLD_HEAL:
    case BORG_ATTACK_OLD_CLONE:
    case BORG_ATTACK_OLD_SPEED:
    case BORG_ATTACK_DARK_WEAK:
    case BORG_ATTACK_KILL_DOOR:
    case BORG_ATTACK_KILL_TRAP:
    case BORG_ATTACK_MAKE_WALL:
    case BORG_ATTACK_MAKE_DOOR:
    case BORG_ATTACK_MAKE_TRAP:
    case BORG_ATTACK_AWAY_UNDEAD:
    case BORG_ATTACK_TURN_EVIL:
    dam = 0;
    break;

    /* These spells which put the monster out of commission, we
     * look at the danger of the monster prior to and after being
     * put out of commission.  The difference is the damage.
     * The following factors are considered when we
     * consider the spell:
     *
     * 1. Is it already comprised by that spell?
     * 2. Is it comprimised by another spell?
     * 3. Does it resist the modality?
     * 4. Will it make it's savings throw better than half the time?
     * 5. We generally ignore these spells for breeders.
     *
     * The spell sleep II and sanctuary have a special consideration
     * since the monsters must be adjacent to the player.
     */

    case BORG_ATTACK_AWAY_ALL:
    /* Teleport Other works differently.  Basically the borg
     * will keep a list of all the monsters in the line of
     * fire.  Then when he checks the danger, he will not
     * include those monsters.
     */

     /* try not to teleport away uniques. These are the guys you are trying */
     /* to kill! */
    if (rf_has(r_ptr->flags, RF_UNIQUE))
    {
        /* This unique is low on HP, finish it off */
        if (kill->injury >= 60)	dam = -9999;

        /* I am sitting pretty in an AS-Corridor */
        else if (borg_as_position) dam = -9999;

        /* If this unique is causing the danger, get rid of it */
        else if (dam > avoidance * 13 / 10 && borg_skill[BI_CDEPTH] <= 98)
        {
            /* get rid of this unique by storing his info */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n] = kill->y;
            borg_tp_other_x[borg_tp_other_n] = kill->x;
            borg_tp_other_n++;
        }

        /* If fighting multiple uniques, get rid of one */
        else if (borg_fighting_unique >= 2 && borg_fighting_unique <= 8)
        {
            /* get rid of one unique or both if they are in a beam-line */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n] = kill->y;
            borg_tp_other_x[borg_tp_other_n] = kill->x;
            borg_tp_other_n++;
        }
        /* Unique is adjacent to Borg */
        else if (borg_class == CLASS_MAGE &&
            borg_distance(c_y, c_x, kill->y, kill->x) <= 2)
        {
            /* get rid of unique next to me */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n] = kill->y;
            borg_tp_other_x[borg_tp_other_n] = kill->x;
            borg_tp_other_n++;

        }
        /* Unique in a vault, get rid of it, clean vault */
        else if (vault_on_level)
        {
            /* Scan grids adjacent to monster */
            for (ii = 0; ii < 8; ii++)
            {
                x = kill->x + ddx_ddd[ii];
                y = kill->y + ddy_ddd[ii];

                /* Access the grid */
                ag = &borg_grids[y][x];

                /* Skip unknown grids (important) */
                if (ag->feat == FEAT_NONE) continue;

                /* Count adjacent Permas */
                if (ag->feat == FEAT_PERM) vault_grids++;
            }

            /* Near enough perma grids? */
            if (vault_grids >= 2)
            {
                /* get rid of unique next to perma grids */
                borg_tp_other_index[borg_tp_other_n] = i;
                borg_tp_other_y[borg_tp_other_n] = kill->y;
                borg_tp_other_x[borg_tp_other_n] = kill->x;
                borg_tp_other_n++;
            }

        }
        else dam = -999;
    }
    else /* not a unique */
    {
        /* get rid of this non-unique by storing his info */
        borg_tp_other_index[borg_tp_other_n] = i;
        borg_tp_other_y[borg_tp_other_n] = kill->y;
        borg_tp_other_x[borg_tp_other_n] = kill->x;
        borg_tp_other_n++;
    }
    break;

    /* This teleport away is used to teleport away all monsters
     * as the borg goes through his special attacks.
     */
    case BORG_ATTACK_AWAY_ALL_MORGOTH:
    /* Mostly no damage */
    dam = 0;

    /* If its touching a glyph grid, nail it. */
    for (j = 0; j < 8; j++)
    {
        int y2 = kill->y + ddy_ddd[j];
        int x2 = kill->x + ddx_ddd[j];

        /* Get the grid */
        ag = &borg_grids[y2][x2];

        /* If its touching a glyph grid, nail it. */
        if (ag->glyph)
        {
            /* get rid of this one by storing his info */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n] = kill->y;
            borg_tp_other_x[borg_tp_other_n] = kill->x;
            borg_tp_other_n++;
            dam = 300;
        }
    }

    /* If the borg is not in a good position, do it */
    if (morgoth_on_level && !borg_morgoth_position)
    {
        /* get rid of this one by storing his info */
        borg_tp_other_index[borg_tp_other_n] = i;
        borg_tp_other_y[borg_tp_other_n] = kill->y;
        borg_tp_other_x[borg_tp_other_n] = kill->x;
        borg_tp_other_n++;
        dam = 100;
    }

    /* If the borg does not have enough Mana to attack this
     * round and cast Teleport Away next round, then do it now.
     */
    if (borg_skill[BI_CURSP] <= 35)
    {
        /* get rid of this unique by storing his info */
        borg_tp_other_index[borg_tp_other_n] = i;
        borg_tp_other_y[borg_tp_other_n] = kill->y;
        borg_tp_other_x[borg_tp_other_n] = kill->x;
        borg_tp_other_n++;
        dam = 150;
    }
    break;

    /* This BORG_ATTACK_ is hacked to work for Mass Genocide.  Since
     * we cannot mass gen uniques.
     */
    case BORG_ATTACK_DISP_ALL:
    if (rf_has(r_ptr->flags, RF_UNIQUE))
    {
        dam = 0;
        break;
    }
    dam = borg_danger_aux(c_y, c_x, 1, i, true, true);
    break;

    case BORG_ATTACK_OLD_CONF:
    dam = 0;
    if (rf_has(r_ptr->flags, RF_NO_CONF)) break;
    if (rf_has(r_ptr->flags, RF_MULTIPLY)) break;
    if (kill->speed < r_ptr->speed - 5) break;
    if (kill->confused) break;
    if (!kill->awake) break;
    if ((kill->level >
        (borg_skill[BI_CLEVEL] < 13 ? 10 : (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10))) break;
    dam = -999;
    if (rf_has(r_ptr->flags, RF_UNIQUE)) break;
    borg_confuse_spell = false;
    p1 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    /* Make certain monsters appear to have more danger so the borg is more likely to use this attack */
    if (kill->afraid && borg_skill[BI_CLEVEL] <= 10) p1 = p1 + 20;
    borg_confuse_spell = true;
    p2 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    borg_confuse_spell = false;
    dam = (p1 - p2);
    break;

    case BORG_ATTACK_TURN_ALL:
    dam = 0;
    if (kill->speed < r_ptr->speed - 5) break;
    if (rf_has(r_ptr->flags, RF_NO_FEAR)) break;
    if (kill->confused) break;
    if (!kill->awake) break;
    if ((kill->level >
        (borg_skill[BI_CLEVEL] < 13 ? 10 : (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10))) break;
    dam = -999;
    if (rf_has(r_ptr->flags, RF_UNIQUE)) break;
    borg_fear_mon_spell = false;
    p1 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    /* Make certain monsters appear to have more danger so the borg is more likely to use this attack */
    if (kill->afraid && borg_skill[BI_CLEVEL] <= 10) p1 = p1 + 20;
    borg_fear_mon_spell = true;
    p2 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    borg_fear_mon_spell = false;
    dam = (p1 - p2);
    break;

    case BORG_ATTACK_OLD_SLOW:
    dam = 0;
    if (kill->speed < r_ptr->speed - 5) break;
    if (kill->confused) break;
    if (!kill->awake) break;
    if ((kill->level >
        (borg_skill[BI_CLEVEL] < 13 ? 10 : (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10))) break;
    dam = -999;
    if (rf_has(r_ptr->flags, RF_UNIQUE)) break;
    borg_slow_spell = false;
    p1 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    /* Make certain monsters appear to have more danger so the borg is more likely to use this attack */
    if (kill->afraid && borg_skill[BI_CLEVEL] <= 10) p1 = p1 + 20;
    borg_slow_spell = true;
    p2 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    borg_slow_spell = false;
    dam = (p1 - p2);
    break;

    case BORG_ATTACK_OLD_SLEEP:
    case BORG_ATTACK_SLEEP_EVIL:
    dam = 0;
    if (rf_has(r_ptr->flags, RF_NO_SLEEP)) break;
    if (!rf_has(r_ptr->flags, RF_EVIL) && typ == BORG_ATTACK_SLEEP_EVIL) break;
    if (kill->speed < r_ptr->speed - 5) break;
    if (kill->confused) break;
    if (!kill->awake) break;
    if ((kill->level >
        (borg_skill[BI_CLEVEL] < 13 ? 10 : (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10))) break;
    dam = -999;
    if (rf_has(r_ptr->flags, RF_UNIQUE)) break;
    borg_sleep_spell = false;
    p1 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    /* Make certain monsters appear to have more danger so the borg is more likely to use this attack */
    if (kill->afraid && borg_skill[BI_CLEVEL] <= 10) p1 = p1 + 20;
    borg_sleep_spell = true;
    p2 = borg_danger_aux(c_y, c_x, 1, i, true, true);
    borg_sleep_spell = false;
    dam = (p1 - p2);
    break;


    case BORG_ATTACK_OLD_POLY:
    dam = 0;
    if ((kill->level >
        (borg_skill[BI_CLEVEL] < 13 ? 10 : (((borg_skill[BI_CLEVEL] - 10) / 4) * 3) + 10))) break;
    dam = -999;
    if (rf_has(r_ptr->flags, RF_UNIQUE)) break;
    dam = borg_danger_aux(c_y, c_x, 2, i, true, true);
    /* dont bother unless he is a scary monster */
    if ((dam < avoidance * 2) && !kill->afraid) dam = 0;
    break;

    case BORG_ATTACK_TURN_UNDEAD:
    if (rf_has(r_ptr->flags, RF_UNDEAD))
    {
        dam = 0;
        if (kill->confused) break;
        if (kill->speed < r_ptr->speed - 5) break;
        if (!kill->awake) break;
        if (kill->level > borg_skill[BI_CLEVEL] - 5) break;
        borg_fear_mon_spell = false;
        p1 = borg_danger_aux(c_y, c_x, 1, i, true, true);
        borg_fear_mon_spell = true;
        p2 = borg_danger_aux(c_y, c_x, 1, i, true, true);
        borg_fear_mon_spell = false;
        dam = (p1 - p2);
    }
    else
    {
        dam = 0;
    }
    break;

    /* Banishment-- cast when in extreme danger (checked in borg_defense). */
    case BORG_ATTACK_AWAY_EVIL:
    if (rf_has(r_ptr->flags, RF_EVIL))
    {
        /* try not teleport away uniques. */
        if (rf_has(r_ptr->flags, RF_UNIQUE))
        {
            /* Banish ones with escorts */
            if (r_ptr->friends || r_ptr->friends_base)
            {
                dam = 0;
            }
            else
            {
                /* try not Banish non escorted uniques */
                dam = -500;
            }

        }
        else
        {
            /* damage is the danger of the baddie */
            dam = borg_danger_aux(c_y, c_x, 1, i, true, true);
        }
    }
    else
    {
        dam = 0;
    }
    break;


    case BORG_ATTACK_TAP_UNLIFE:
    /* for now ignore the gain in sp */
    if (!(rf_has(r_ptr->flags, RF_UNDEAD)))
        dam = 0;
    else
    {
        int sp_drain = borg_skill[BI_CURSP] - borg_skill[BI_CURSP];
        if (sp_drain < kill->power)
            dam = kill->power - sp_drain;
    }
    break;
    }

    /* use Missiles on certain types of monsters */
    if ((borg_skill[BI_CDEPTH] >= 1) &&
        (borg_danger_aux(kill->y, kill->x, 1, i, true, true) > avoidance * 2 / 10 ||
            ((r_ptr->friends || r_ptr->friends_base) /* monster has friends*/ &&
                kill->level >= borg_skill[BI_CLEVEL] - 5 /* close levels */) ||
            kill->ranged_attack /* monster has a ranged attack */ ||
            rf_has(r_ptr->flags, RF_UNIQUE) ||
            rf_has(r_ptr->flags, RF_MULTIPLY) ||
            gold_eater || /* Monster can steal gold */
            rf_has(r_ptr->flags, RF_NEVER_MOVE) /* monster never moves */ ||
            borg_skill[BI_CLEVEL] <= 20 /* stil very weak */))
    {
        borg_use_missile = true;
    }

    /* Return Damage as pure danger of the monster */
    if (typ == BORG_ATTACK_AWAY_ALL || typ == BORG_ATTACK_AWAY_EVIL ||
        typ == BORG_ATTACK_AWAY_ALL_MORGOTH) return (dam);

    /* Limit damage to twice maximal hitpoints */
    if (dam > kill->power * 2 && !rf_has(r_ptr->flags, RF_UNIQUE)) dam = kill->power * 2;

    /* give a small bonus for whacking a unique */
    /* this should be just enough to give prefrence to wacking uniques */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg_skill[BI_CDEPTH] >= 1)
        dam = (dam * 3);

    /* Hack -- ignore Maggot until later.  Player will chase Maggot
     * down all accross the screen waking up all the monsters.  Then
     * he is stuck in a compromised situation.
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg_skill[BI_CDEPTH] == 0)
    {
        dam = dam * 2 / 3;

        /* Dont hunt maggot until later */
        if (borg_skill[BI_CLEVEL] < 5) dam = 0;
    }

    /* give a small bonus for whacking a breeder */
    if (rf_has(r_ptr->flags, RF_MULTIPLY))
        dam = (dam * 3 / 2);

    /* Enhance the perceived damage to summoner in order to influence the
     * choice of targets.
     */
    if ((rsf_has(r_ptr->spell_flags, RSF_S_KIN)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HOUND)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_AINU)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_DEMON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HI_DRAGON)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH)) ||
        (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE)))
        dam += ((dam * 3) / 2);

    /*
     * Apply massive damage bonus to Questor monsters to
     * encourage borg to strike them.
     */
    if (rf_has(r_ptr->flags, RF_QUESTOR)) dam += (dam * 9);

    /*  Try to conserve missiles.
     */
    if (typ == BORG_ATTACK_ARROW)
    {
        if (!borg_use_missile)
            /* set damage to zero, force borg to melee attack */
            dam = 0;
    }

    /* Damage */
    return (dam);
}
/*
 * Simulate / Invoke the launching of a bolt at a monster
 */
static int borg_launch_bolt_aux_hack(int i, int dam, int typ, int ammo_location)
{
    int d, p2, p1, x, y;
    int o_y = 0;
    int o_x = 0;
    int walls = 0;
    int unknown = 0;

    borg_grid* ag;

    borg_kill* kill;

    struct monster_race* r_ptr;

    /* Monster */
    kill = &borg_kills[i];

    /* monster race */
    r_ptr = &r_info[kill->r_idx];

    /* Skip dead monsters */
    if (!kill->r_idx) return (0);

    /* Require current knowledge */
    if (kill->when < borg_t - 2) return (0);

    /* Acquire location */
    x = kill->x;
    y = kill->y;

    /* Acquire the grid */
    ag = &borg_grids[y][x];

    /* Never shoot walls/doors */
    if (!borg_cave_floor_grid(ag)) return (0);

    /* dont shoot at ghosts if not on known floor grid */
    if ((rf_has(r_ptr->flags, RF_PASS_WALL)) &&
        (ag->feat != FEAT_FLOOR &&
            ag->feat != FEAT_OPEN &&
            ag->feat != FEAT_BROKEN &&
            !ag->trap)) return (0);

    /* dont shoot at ghosts in walls, not perfect */
    if (rf_has(r_ptr->flags, RF_PASS_WALL))
    {
        /* if 2 walls and 1 unknown skip this monster */
        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        for (o_x = -1; o_x <= 1; o_x++)
        {
            for (o_y = -1; o_y <= 1; o_y++)
            {
                /* Acquire location */
                x = kill->x + o_x;
                y = kill->y + o_y;

                ag = &borg_grids[y][x];

                if (ag->feat >= FEAT_MAGMA &&
                    ag->feat <= FEAT_PERM) walls++;
                if (ag->feat == FEAT_NONE) unknown++;
            }
        }
        /* Is the ghost likely in a wall? */
        if (walls >= 2 && unknown >= 1) return (0);
    }



    /* Calculate damage */
    d = borg_launch_damage_one(i, dam, typ, ammo_location);

    /* Return Damage, on Teleport Other, true damage is
     * calculated elsewhere */
    if (typ == BORG_ATTACK_AWAY_ALL || typ == BORG_ATTACK_AWAY_ALL_MORGOTH) return (d);

    /* Return Damage as pure danger of the monster */
    if (typ == BORG_ATTACK_AWAY_EVIL) return (d);

    /* Return 0 if the true damge (w/o the danger bonus) is 0 */
    if (d <= 0) return (d);

    /* Calculate danger */
    p2 = borg_danger_aux(y, x, 1, i, true, false);

    /* Hack -- avoid waking most "hard" sleeping monsters */
    if (!kill->awake &&
        (p2 > avoidance / 2) &&
        (d < kill->power) && !borg_munchkin_mode)
    {
        return (-999);
    }

    /* Hack -- ignore sleeping town monsters */
    if (!borg_skill[BI_CDEPTH] && !kill->awake)
    {
        return (0);
    }

    /* Hack -- ignore nonthreatening town monsters when low level */
    if (!borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 3
        /* && monster_is_nonthreatening_test */)
    {
        /* Nothing yet */
    }

    /* Calculate "danger" to player */
    p1 = borg_danger_aux(c_y, c_x, 1, i, true, false);

    /* Extra "bonus" if attack kills */
    if (d >= kill->power) d = 2 * d;

    /* Add in dangers */
    d = d + p1;

    /* Result */
    return (d);
}


/*
 * Determine the "reward" of launching a beam/bolt/ball at a location
 *
 * An "unreachable" location always has zero reward.
 *
 * Basically, we sum the "rewards" of doing the appropriate amount of
 * damage to each of the "affected" monsters.
 *
 * We will attempt to apply the offset-ball attack here
 */
static int borg_launch_bolt_aux(int y, int x, int rad, int dam, int typ, int max, int ammo_location)
{
    int ry, rx;

    int x1, y1;
    int x2, y2;

    int dist;

    int r, n;

    borg_grid* ag;
    struct monster_race* r_ptr;
    borg_kill* kill;

    int q_x, q_y;

    /* Extract panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();

    /* Reset damage */
    n = 0;

    /* Initial location */
    x1 = c_x; y1 = c_y;

    /* Final location */
    x2 = x; y2 = y;

    /* Bounds Check */
    if (!square_in_bounds_fully(cave, loc(x, y)))
        return 0;

    /* Start over */
    x = x1; y = y1;

    /* Simulate the spell/missile path */
    for (dist = 0; dist < max; dist++)
    {
        /* Bounds Check */
        if (dist && !square_in_bounds_fully(cave, loc(x, y))) break;

        /* Get the grid of the targetted monster */
        ag = &borg_grids[y2][x2];
        kill = &borg_kills[ag->kill];
        r_ptr = &r_info[kill->r_idx];

        /* Get the grid of the pathway */
        ag = &borg_grids[y][x];

        /* Stop at walls */
        /* note: beams end at walls.  */
        if (dist)
        {
            /* Stop at walls */
            /* note if beam, this is the end of the beam */
            /* dispel spells act like beams (sort of) */
            if (!borg_cave_floor_grid(ag) || ag->feat == FEAT_PASS_RUBBLE)
            {
                if (rad != -1 && rad != 10)
                    return (0);
                else
                    return (n);
            }
        }


        /* Collect damage (bolts/beams) */
        if (rad <= 0 || rad == 10) n += borg_launch_bolt_aux_hack(ag->kill, dam, typ, ammo_location);

        /* Check for arrival at "final target" */
        /* except beams, which keep going. */
        if ((rad != -1 && rad != 10) && ((x == x2) && (y == y2))) break;

        /* Stop bolts at monsters  */
        if (!rad && ag->kill) return (n);

        /* The missile path can be complicated.  There are several checks
         * which need to be made.  First we assume that we targetting
         * a monster.  That monster could be known from either sight or
         * ESP.  If the entire pathway from us to the monster is known,
         * then there is no concern.  But if the borg is shooting through
         * unknown grids, then there is a concern when he has ESP; without
         * ESP he would not see that monster if the unknown grids
         * contained walls or closed doors.
         *
         * 1.  ESP Inactive
         *   A.  No Infravision
         *       -Then the monster must be in a lit grid. OK to shoot
         *   B.  Yes Infravision
         *       -Then the monster must be projectable()  OK to shoot
         * 2.  ESP Active
         *   A. No Infravision
         *       -Then the monster could be in a lit grid.  Try to shoot
         *       -Or I detect it with ESP and it's not projectable().
         *   B.  Yes Infravision
         *       -Then the monster could be projectable()
         *       -Or I detect it with ESP and it's not projectable().
         *   -In the cases of ESP Active, the borg will test fire a missile.
         *    Then wait for a 'painful ouch' from the monster.
         *
         * Low level borgs will not take the shot unless they have
         * a clean and known pathway.  Borgs over a certain clevel,
         * will attempt the shot and listen for the 'ouch' repsonse
         * to know that the clear.  If no 'Ouch' is heard, then the
         * borg will assume there is a wall in the way.  Exception to
         * this is with arrows.  Arrows can miss the target or fall
         * fall short, in which case no 'ouch' is heard.  So the borg
         * allowed to miss two shots with arrows/bolts/thrown objects.
         */

         /* dont do the check if esp */
        if (!borg_skill[BI_ESP])
        {
            /* Check the missile path--no Infra, no HAS_LIGHT */
            if (dist && (borg_skill[BI_INFRA] <= 0)
                && !(r_ptr->light > 0))
            {
                /* Stop at unknown grids (see above) */
                /* note if beam, dispel, this is the end of the beam */
                if (ag->feat == FEAT_NONE)
                {
                    if (rad != -1 && rad != 10)
                        return (0);
                    else
                        return (n);
                }

                /* Stop at unseen walls */
                /* We just shot and missed, this is our next shot */
                if (successful_target < 0)
                {
                    /* When throwing things, it is common to just 'miss' */
                    /* Skip only one round in this case */
                    if (successful_target <= -12)
                        successful_target = 0;
                    if (rad != -1 && rad != 10)
                        return (0);
                    else
                        return (n);
                }
            }
            else  /* I do have infravision or it's a lite monster */
            {
                /* Stop at unseen walls */
                /* We just shot and missed, this is our next shot */
                if (successful_target < 0)
                {
                    /* When throwing things, it is common to just 'miss' */
                    /* Skip only one round in this case */
                    if (successful_target <= -12)
                        successful_target = 0;
                    if (rad != -1 && rad != 10)
                        return (0);
                    else
                        return (n);
                }
            }
        }
        else /* I do have ESP */
        {
            /* Check the missile path */
            if (dist)
            {
                /* if this area has been magic mapped,
                * ok to shoot in the dark
                */
                if (!borg_detect_wall[q_y + 0][q_x + 0] &&
                    !borg_detect_wall[q_y + 0][q_x + 1] &&
                    !borg_detect_wall[q_y + 1][q_x + 0] &&
                    !borg_detect_wall[q_y + 1][q_x + 1] &&
                    borg_fear_region[c_y / 11][c_x / 11] < avoidance / 20)
                {

                    /* Stop at unknown grids (see above) */
                    /* note if beam, dispel, this is the end of the beam */
                    if (ag->feat == FEAT_NONE)
                    {
                        if (rad != -1 && rad != 10)
                            return (0);
                        else
                            return (n);
                    }
                    /* Stop at unseen walls */
                    /* We just shot and missed, this is our next shot */
                    if (successful_target < 0)
                    {
                        /* When throwing things, it is common to just 'miss' */
                        /* Skip only one round in this case */
                        if (successful_target <= -12)
                            successful_target = 0;
                        if (rad != -1 && rad != 10)
                            return (0);
                        else
                            return (n);
                    }
                }

                /* Stop at unseen walls */
                /* We just shot and missed, this is our next shot */
                if (successful_target < 0)
                {
                    /* When throwing things, it is common to just 'miss' */
                    /* Skip only one round in this case */
                    if (successful_target <= -12)
                        successful_target = 0;

                    if (rad != -1 && rad != 10)
                        return (0);
                    else
                        return (n);
                }
            }
        }

        /* Calculate the new location */
        mmove2(&y, &x, y1, x1, y2, x2);
    }

    /* Bolt/Beam attack */
    if (rad <= 0) return (n);

    /* Excessive distance */
    if (dist >= max) return (0);

    /* Check monsters and objects in blast radius */
    for (ry = y2 - rad; ry < y2 + rad; ry++)
    {
        for (rx = x2 - rad; rx < x2 + rad; rx++)
        {

            /* Bounds check */
            if (!square_in_bounds(cave, loc(rx, ry))) continue;

            /* Get the grid */
            ag = &borg_grids[ry][rx];

            /* Check distance */
            r = borg_distance(y2, x2, ry, rx);

            /* Maximal distance */
            if (r > rad) continue;

            /* Never pass through walls*/
            if (!borg_los(y2, x2, ry, rx)) continue;

            /*  dispel spells should hurt the same no matter the rad: make r= y  and x */
            if (rad == 10) r = 0;

            /* Collect damage, lowered by distance */
            n += borg_launch_bolt_aux_hack(ag->kill, dam / (r + 1), typ, ammo_location);

            /* probable damage int was just changed by b_l_b_a_h*/

            /* check destroyed stuff. */
            if (ag->take && borg_takes[ag->take].kind)
            {
                struct borg_take* take = &borg_takes[ag->take];
                struct object_kind* k_ptr = take->kind;

                switch (typ)
                {
                case BORG_ATTACK_ACID:
                {
                    /* rings/boots cost extra (might be speed!) */
                    if (k_ptr->tval == TV_BOOTS && !k_ptr->aware)
                    {
                        n -= 20;
                    }
                    break;
                }
                case BORG_ATTACK_ELEC:
                {
                    /* rings/boots cost extra (might be speed!) */
                    if (k_ptr->tval == TV_RING && !k_ptr->aware)
                    {
                        n -= 20;
                    }
                    if (k_ptr->tval == TV_RING && k_ptr->sval == sv_ring_speed)
                    {
                        n -= 2000;
                    }
                    break;
                }

                case BORG_ATTACK_FIRE:
                {
                    /* rings/boots cost extra (might be speed!) */
                    if (k_ptr->tval == TV_BOOTS && !k_ptr->aware)
                    {
                        n -= 20;
                    }
                    break;
                }
                case BORG_ATTACK_COLD:
                {
                    if (k_ptr->tval == TV_POTION)
                    {
                        n -= 20;

                        /* Extra penalty for cool potions */
                        if (!k_ptr->aware ||
                            k_ptr->sval == sv_potion_healing || k_ptr->sval == sv_potion_star_healing ||
                            k_ptr->sval == sv_potion_life ||
                            (k_ptr->sval == sv_potion_inc_str && amt_add_stat[STAT_STR] >= 1000) ||
                            (k_ptr->sval == sv_potion_inc_int && amt_add_stat[STAT_INT] >= 1000) ||
                            (k_ptr->sval == sv_potion_inc_wis && amt_add_stat[STAT_WIS] >= 1000) ||
                            (k_ptr->sval == sv_potion_inc_dex && amt_add_stat[STAT_DEX] >= 1000) ||
                            (k_ptr->sval == sv_potion_inc_con && amt_add_stat[STAT_CON] >= 1000)) n -= 2000;
                    }
                    break;
                }
                case BORG_ATTACK_MANA:
                {
                    /* Used against uniques, allow the stuff to burn */
                    break;
                }
                }
            }
        }
    }

    /* Result */
    return (n);
}


/*
 * Simulate/Apply the optimal result of launching a beam/bolt/ball
 *
 * Note that "beams" have a "rad" of "-1", "bolts" have a "rad" of "0",
 * and "balls" have a "rad" of "2" or "3", depending on "blast radius".
 *  dispel spells have a rad  of 10
 */
static int borg_launch_bolt(int rad, int dam, int typ, int max, int ammo_location)
{
    int i = 0;
    int b_i = -1;
    int n = 0;
    int b_n = -1;
    int b_o_y = 0, b_o_x = 0;
    int o_y = 0, o_x = 0;
    int d, b_d = z_info->max_range;



    /* Examine possible destinations */

    /* This will allow the borg to target places adjacent to a monster
     * in order to exploit and abuse a feature of the game.  Whereas,
     * the borg, while targeting a monster will not score d/t walls, he
     * could land a successful hit by targeting adjacent to the monster.
     * For example:
     * ######################
     * #####....@......######
     * ############Px........
     * ######################
     * In order to hit the P, the borg must target the x and not the P.
     *
     */
    for (i = 0; i < borg_temp_n; i++)
    {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Consider each adjacent spot to and on top of the monster */
        for (o_x = -1; o_x <= 1; o_x++)
        {
            for (o_y = -1; o_y <= 1; o_y++)
            {
                /* Acquire location */
                x = borg_temp_x[i] + o_x;
                y = borg_temp_y[i] + o_y;

                /* Reset Teleport Other variables */
                borg_tp_other_n = 0;
                n = 0;

                /* Bounds check */
                if (!square_in_bounds(cave, loc(x, y))) continue;

                /* Remember how far away the monster is */
                d = borg_distance(c_y, c_x, borg_temp_y[i], borg_temp_x[i]);

                /* Skip certain types of Offset attacks */
                if ((x != borg_temp_x[i] ||
                    y != borg_temp_y[i]) &&
                    typ == BORG_ATTACK_AWAY_ALL) continue;

                /* Skip places that are out of range */
                if (borg_distance(c_y, c_x, y, x) > max) continue;

                /* Consider it if its a ball spell or right on top of it */
                if ((rad >= 2 && borg_grids[y][x].feat != FEAT_NONE) ||
                    (y == borg_temp_y[i] &&
                        x == borg_temp_x[i])) n = borg_launch_bolt_aux(y, x, rad, dam, typ, max, ammo_location);

                /* Teleport Other is now considered */
                if (typ == BORG_ATTACK_AWAY_ALL && n > 0)
                {
                    /* Consider danger with certain monsters removed
                     * from the danger check.  They were removed from the list of
                     * considered monsters (borg_tp_other array)
                     */
                    n = borg_danger(c_y, c_x, 1, true, false);

                    /* Skip Offsets that do only 1 damage */
                    if (n == 1) n = -10;
                }

                /* Reset Teleport Other variables */
                borg_tp_other_n = 0;

                /* Skip useless attacks */
                if (n <= 0) continue;

                /* The game forbids targetting the outside walls */
                if (x == 0 || y == 0 || x == DUNGEON_WID - 1 || y == DUNGEON_HGT - 1)
                    continue;

                /* Collect best attack */
                if ((b_i >= 0) && (n < b_n)) continue;

                /* Skip attacking farther monster if rewards are equal. */
                if (n == b_n && d > b_d) continue;

                /* Track it */
                b_i = i;
                b_n = n;
                b_o_y = o_y;
                b_o_x = o_x;
                b_d = d;
            }
        }
    }
    if (b_i == -1) return (b_n);

    /* Reset Teleport Other variables */
    borg_tp_other_n = 0;

    /* Simulation */
    if (borg_simulate) return (b_n);


    /* Save the location */
    g_x = borg_temp_x[b_i] + b_o_x;
    g_y = borg_temp_y[b_i] + b_o_y;

    /* Target the location */
    (void)borg_target(g_y, g_x);

    /* Result */
    return (b_n);
}

/*
 * Simulate/Apply the optimal result of launching a missile
 */
static int borg_attack_aux_launch(void)
{
    int n, b_n = 0;

    int k, b_k = -1;
    int d = -1;
    int v, b_v = -1;

    borg_item* bow = &borg_items[INVEN_BOW];

    /* skip if we don't have a bow */
    if (!bow || bow->iqty == 0) return 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);

    /* Scan the quiver */
    for (k = QUIVER_START; k < QUIVER_END; k++)
    {

        borg_item* item = &borg_items[k];

        /* Skip empty items */
        if (!item->iqty) break;

        /* Skip missiles that don't match the bow */
        if (item->tval != borg_skill[BI_AMMO_TVAL]) continue;

        /* Skip worthless missiles */
        if (item->value <= 0) continue;

        /* Determine average damage */
        d = (item->dd * (item->ds + 1) / 2);
        d = d + item->to_d + bow->to_d;
        d = d * borg_skill[BI_AMMO_POWER] * borg_skill[BI_SHOTS];

        v = item->value;

        /* Boost the perceived damage on unID'd ones so he can get a quick pseudoID on it */
        if (borg_item_note_needs_id(item)) d = d * 99;

        /* Paranoia */
        if (d <= 0) continue;

        /* Choose optimal target of bolt */
        n = borg_launch_bolt(0, d, BORG_ATTACK_ARROW, 6 + 2 * borg_skill[BI_AMMO_POWER], k);

        /* if two attacks are equal, pick the cheaper ammo */
        if (n == b_n && v >= b_v)
            continue;

        if (n >= b_n)
        {
            b_n = n;
            b_v = v;
            b_k = k;
        }
    }

    /* Nothing to use */
    if (b_n < 0) return (0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Do it */
    borg_note(format("# Firing missile '%s'", borg_items[b_k].desc));

    /* Fire */
    borg_keypress('f');

    /* Use the missile from the quiver */
    borg_keypress(((b_k - QUIVER_START) + '0'));

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -2;

    /* Value */
    return (b_n);
}


/* Attempt to rest on the grid to allow the monster to approach me.
 * Make sure the monster does not have a ranged attack and that I am
 * inclined to attack him.
 */
static int borg_attack_aux_rest(void)
{
    int i;
    bool resting_is_good = false;

    int my_danger = borg_danger(c_y, c_x, 1, false, false);

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];

        int x9 = kill->x;
        int y9 = kill->y;
        int ax, ay, d;

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Distance components */
        ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
        ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Minimal and maximal distance */
        if (d != 2) continue;

        /* Ranged Attacks, don't rest. */
        if (kill->ranged_attack) continue;

        /* Skip the sleeping ones */
        if (!kill->awake) continue;

        /* need to have seen it recently */
        if (borg_t - kill->when > 10) continue;

        /* Skip monsters that dont chase */
        if (rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE)) continue;

        /* Monster better not be faster than me */
        if (kill->speed - borg_skill[BI_SPEED] >= 5) continue;

        /* Should be flowing towards the monster */
        if (goal != GOAL_KILL || borg_flow_y[0] != kill->y) continue;

        /* Cant have an obstacle between us */
        if (!borg_los(c_y, c_x, kill->y, kill->x)) continue;

        /* Might be a little dangerous to just wait here */
        if (my_danger > borg_skill[BI_CURHP]) continue;

        /* Should be a good idea to wait for monster here. */
        resting_is_good = true;
    }

    /* Not a good idea */
    if (resting_is_good == false) return (0);

    /* Return some value for this rest */
    if (borg_simulate) return (1);

    /* Rest */
    borg_keypress(',');
    borg_note(format("# Resting on grid (%d, %d), waiting for monster to approach.", c_y, c_x));

    /* All done */
    return (1);
}

/* look for a throwable item */
static bool borg_has_throwable(void)
{
    int i;
    for (i = 0; i < QUIVER_END; i++)
    {
        /* it will show wield in the list */ 
        /* but not if that is the only thing */
        if (i == INVEN_WIELD) continue;

        if (!borg_items[i].iqty)
            continue;

        if (of_has(borg_items[i].flags, OF_THROWING))
            return true;
    }
    return false;
}

/*
 * Simulate/Apply the optimal result of throwing an object
 *
 * First choose the "best" object to throw, then check targets.
 */
static int borg_attack_aux_object(void)
{
    int b_n;

    int b_r = 0;

    int k, b_k = -1;
    int d, b_d = -1;

    int div, mul;

    /* Scan the pack */
    for (k = 0; k < z_info->pack_size; k++)
    {
        borg_item* item = &borg_items[k];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip my spell/prayer book */
        if (obj_kind_can_browse(&k_info[item->kind])) continue;

        /* Skip "equipment" items (not ammo) */
        if (borg_wield_slot(item) >= 0) continue;

        /* Determine average damage from object */
        d = (k_info[item->kind].dd * (k_info[item->kind].ds + 1) / 2);

        /* Skip things that are worth money unless they do a lot of damage */
        if (item->value > 100 && d < 5) continue;

        /* Skip useless stuff */
        if (d <= 0) continue;

        /* Hack -- Save Heals and cool stuff */
        if (item->tval == TV_POTION) continue;

        /* Hack -- Save last flasks for fuel, if needed */
        if (item->tval == TV_FLASK &&
            (borg_skill[BI_AFUEL] <= 1 && !borg_fighting_unique)) continue;

        /* Dont throw wands or rods */
        if (item->tval == TV_WAND || item->tval == TV_ROD) continue;

        /* Ignore worse damage */
        if ((b_k >= 0) && (d <= b_d)) continue;

        /* Track */
        b_k = k;
        b_d = d;

        /* Extract a "distance multiplier" */
        mul = 10;

        /* Enforce a minimum "weight" of one pound */
        div = ((item->weight > 10) ? item->weight : 10);

        /* Hack -- Distance -- Reward strength, penalize weight */
        b_r = (adj_str_blow[my_stat_ind[STAT_STR]] + 20) * mul / div;

        /* Max distance of 10 */
        if (b_r > 10) b_r = 10;
    }

    /* Nothing to use */
    if (b_k < 0) return (0);


    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Choose optimal location */
    b_n = borg_launch_bolt(0, b_d, BORG_ATTACK_ARROW, 6 + 2 * borg_skill[BI_AMMO_POWER], b_k);

    /* Simulation */
    if (borg_simulate) return (b_n);


    /* Do it */
    borg_note(format("# Throwing painful object '%s'",
        borg_items[b_k].desc));

    /* Fire */
    borg_keypress('v');

    if (borg_has_throwable())
        borg_keypress('/');

    /* Use the object */
    borg_keypress(all_letters_nohjkl[b_k]);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -2;

    /* Value */
    return (b_n);
}


/*
 * Simulate/Apply the optimal result of using a "normal" attack spell
 *
 * Take into account the failure rate of spells/objects/etc.  XXX XXX XXX
 */
static int borg_attack_aux_spell_bolt(const enum borg_spells spell, int rad, int dam, int typ, int max_range)
{
    int b_n;
    int penalty = 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);

    /* Paranoia */
    if (borg_simulate &&
        ((borg_class != CLASS_MAGE && borg_class != CLASS_NECROMANCER) && borg_skill[BI_CLEVEL] <= 2) &&
        (randint0(100) < 1)) return (0);

    /* Not if money scumming in town */
    if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] && borg_skill[BI_CDEPTH] == 0) return (0);

    /* Not if low on food */
    if (borg_skill[BI_FOOD] == 0 &&
        (borg_skill[BI_ISWEAK] && (borg_spell_legal(REMOVE_HUNGER) || borg_spell_legal(HERBAL_CURING)))) return (0);

    /* Require ability (right now) */
    if (!borg_spell_okay_fail(spell, (borg_fighting_unique ? 40 : 25))) return (0);


    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, max_range, 0);

    enum borg_spells primary_spell_for_class = MAGIC_MISSILE;
    switch (borg_class)
    {
    case CLASS_MAGE:
    primary_spell_for_class = MAGIC_MISSILE;
    break;
    case CLASS_DRUID:
    primary_spell_for_class = STINKING_CLOUD;
    break;
    case CLASS_PRIEST:
    primary_spell_for_class = ORB_OF_DRAINING;
    break;
    case CLASS_NECROMANCER:
    primary_spell_for_class = NETHER_BOLT;
    break;
    case CLASS_PALADIN:
    case CLASS_ROGUE:
    case CLASS_RANGER:
    case CLASS_BLACKGUARD:
    break;
    }

    /* weak mages need that spell, they dont get penalized */
    /* weak == those that can't teleport reliably anyway */
    if (spell == primary_spell_for_class &&
        (!borg_spell_legal_fail(TELEPORT_SELF, 15) || borg_skill[BI_MAXCLEVEL] <= 30))
    {
        if (borg_simulate) return (b_n);
    }

    /* Penalize mana usage except on MM */
    int spell_power = borg_get_spell_power(spell);
    if (spell != primary_spell_for_class)
    {
        /* Standard penalty */
        b_n = b_n - spell_power;

        /* Extra penalty if the cost far outweighs the damage */
        if (borg_skill[BI_MAXSP] < 50 && spell_power > b_n) b_n = b_n - spell_power;

        /* Penalize use of reserve mana */
        if (borg_skill[BI_CURSP] - spell_power < borg_skill[BI_MAXSP] / 2) b_n = b_n - (spell_power * 3);

        /* Penalize use of deep reserve mana */
        if (borg_skill[BI_CURSP] - spell_power < borg_skill[BI_MAXSP] / 3) b_n = b_n - (spell_power * 5);

    }

    /* Really penalize use of mana needed for final teleport */
    if (borg_class == CLASS_MAGE) penalty = 6;
    if (borg_class == CLASS_RANGER) penalty = 22;
    if (borg_class == CLASS_NECROMANCER) penalty = 10;
    if (borg_class == CLASS_ROGUE) penalty = 20;
    if (borg_class == CLASS_PRIEST) penalty = 8;
    if (borg_class == CLASS_PALADIN) penalty = 20;
    if ((borg_skill[BI_MAXSP] > 30) &&
        (borg_skill[BI_CURSP] - spell_power < penalty))
        b_n = b_n - (spell_power * 750);

    /* Simulation */
    if (borg_simulate) return (b_n);


    /* Cast the spell */
    (void)borg_spell(spell);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return (b_n);
}

/* This routine is the same as the one above only in an emergency case.
 * The borg will enter negative mana casting this
 */
static int borg_attack_aux_spell_bolt_reserve(const enum borg_spells spell, int rad, int dam, int typ, int max_range)
{
    int b_n;
    int i;

    int x9, y9, ax, ay, d;
    int near_monsters = 0;

    /* Fake our Mana */
    int sv_mana = borg_skill[BI_CURSP];

    /* Only Weak guys should try this */
    if (borg_skill[BI_CLEVEL] >= 15) return (0);

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);

    /* Not if low on food */
    if (borg_skill[BI_FOOD] == 0 &&
        (borg_skill[BI_ISWEAK] && borg_spell_legal(REMOVE_HUNGER))) return (0);

    /* Must not have enough mana right now */
    if (borg_spell_okay_fail(spell, 25)) return (0);

    /* Must be dangerous */
    if (borg_danger(c_y, c_x, 1, true, false) < avoidance * 2) return (0);

    /* Find the monster */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* check the location */
        x9 = kill->x;
        y9 = kill->y;

        /* Distance components */
        ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
        ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Count the number of close monsters
         * There should only be one close monster.
         * We do not want to risk fainting.
         */
        if (d < 7) near_monsters++;

        /* If it has too many hp to be taken out with this */
        /* spell, don't bother trying */
        /* NOTE: the +4 is because the damage is toned down
                 as an 'average damage' */
        if (kill->power > (dam + 4))
            return (0);

        /* Do not use it in town */
        if (borg_skill[BI_CDEPTH] == 0) return (0);

        break;
    }

    /* Should only be 1 near monster */
    if (near_monsters > 1) return (0);

    /* Require ability (with faked mana) */
    borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];
    if (!borg_spell_okay_fail(spell, 25))
    {
        /* Restore Mana */
        borg_skill[BI_CURSP] = sv_mana;
        return (0);
    }

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, max_range, 0);

    /* return the value */
    if (borg_simulate)
    {
        /* Restore Mana */
        borg_skill[BI_CURSP] = sv_mana;
        return (b_n);
    }

    /* Cast the spell with fake mana */
    borg_skill[BI_CURSP] = borg_skill[BI_MAXSP];
    if (borg_spell_fail(spell, 25))
    {
        /* Note the use of the emergency spell */
        borg_note("# Emergency use of an Attack Spell.");

        /* verify use of spell */
        /* borg_keypress('y'); */
    }

    /* Use target */
    borg_queue_direction('5');


    /* Set our shooting flag */
    successful_target = -1;

    /* restore true mana */
    borg_skill[BI_CURSP] = 0;

    /* Value */
    return (b_n);
}



/*
 *  Simulate/Apply the optimal result of using a "dispel" attack spell
 */
static int borg_attack_aux_spell_dispel(const enum borg_spells spell, int dam, int typ)
{
    int b_n;
    int penalty = 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);

    /* Not if low on food */
    if (borg_skill[BI_FOOD] == 0 &&
        (borg_skill[BI_ISWEAK] && (borg_spell_legal(REMOVE_HUNGER) || borg_spell_legal(HERBAL_CURING)))) return (0);

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);


    /* Require ability */
    if (!borg_spell_okay_fail(spell, 25)) return (0);

    /* Choose optimal location--radius defined as 10 */
    b_n = borg_launch_bolt(10, dam, typ, z_info->max_range, 0);

    int spell_power = borg_get_spell_power(spell);

    /* Penalize mana usage */
    b_n = b_n - spell_power;

    /* Penalize use of reserve mana */
    if (borg_skill[BI_CURSP] - spell_power < borg_skill[BI_MAXSP] / 2) b_n = b_n - (spell_power * 3);

    /* Penalize use of deep reserve mana */
    if (borg_skill[BI_CURSP] - spell_power < borg_skill[BI_MAXSP] / 3) b_n = b_n - (spell_power * 5);

    /* Really penalize use of mana needed for final teleport */
    if (borg_class == CLASS_MAGE) penalty = 6;
    if (borg_class == CLASS_RANGER) penalty = 22;
    if (borg_class == CLASS_ROGUE) penalty = 20;
    if (borg_class == CLASS_PRIEST) penalty = 8;
    if (borg_class == CLASS_PALADIN) penalty = 20;
    if (borg_class == CLASS_NECROMANCER) penalty = 10;
    if ((borg_skill[BI_MAXSP] > 30) && (borg_skill[BI_CURSP] - spell_power < penalty))
        b_n = b_n - (spell_power * 750);

    /* Really penalize use of mana needed for final teleport */
    /* (6 pts for mage) */
    if ((borg_skill[BI_MAXSP] > 30) && (borg_skill[BI_CURSP] - spell_power) < 6)
        b_n = b_n - (spell_power * 750);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Cast the prayer */
    (void)borg_spell(spell);


    /* Value */
    return (b_n);
}

/*
 *  Simulate/Apply the optimal result of using a "dispel" staff
 * Which would be dispel evil, power, holiness.  Genocide handeled later.
 */
static int borg_attack_aux_staff_dispel(int sval, int rad, int dam, int typ)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);

    /* look for the staff */
    if (!borg_equips_staff_fail(sval)) return (0);

    /* Choose optimal location--radius defined as 10 */
    b_n = borg_launch_bolt(10, dam, typ, z_info->max_range, 0);

    /* Big Penalize charge usage */
    b_n = b_n - 50;

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Cast the prayer */
    (void)borg_use_staff(sval);


    /* Value */
    return (b_n);
}



/*
 * Simulate/Apply the optimal result of using a "normal" attack rod
 */
static int borg_attack_aux_rod_bolt(int sval, int rad, int dam, int typ)
{
    int b_n;


    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);

    /* Not likely to be successful in the activation */
    if (500 < borg_activate_failure(TV_ROD, sval)) return (0);

    /* Look for that rod */
    if (!borg_equips_rod(sval)) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Zap the rod */
    (void)borg_zap_rod(sval);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return (b_n);
}



/*
 * Simulate/Apply the optimal result of using a "normal" attack wand
 */
static int borg_attack_aux_wand_bolt(int sval, int rad, int dam, int typ, int selection)
{
    int i;

    int b_n;


    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);

    /* Dont use wands in town, charges are too spendy */
    if (!borg_skill[BI_CDEPTH]) return (0);

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);


    /* Look for that wand */
    i = borg_slot(TV_WAND, sval);

    /* None available */
    if (i < 0) return (0);

    /* No charges */
    if (!borg_items[i].pval) return (0);

    /* Not likely to be successful in the activation */
    if (500 < borg_activate_failure(TV_WAND, sval)) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Penalize charge usage */
    if (borg_skill[BI_CLEVEL] > 5) b_n = b_n - 5;


    /* Wands of wonder are used in last ditch efforts.  They behave
     * randomly, so the best use of them is an emergency.  I have seen
     * borgs die from hill orcs with fully charged wonder wands.  Odds
     * are he could have taken the orcs with the wand.  So use them in
     * an emergency after all the borg_caution() steps have failed
     */
    if (sval == sv_wand_wonder && !borg_munchkin_mode)
    {
        /* check the danger */
        if (b_n > 0 && borg_danger(c_y, c_x, 1, true, false) >= (avoidance * 7 / 10))
        {
            /* make the wand appear deadly */
            b_n = 999;

            /* note the use of the wand in the emergency */
            borg_note(format("# Emergency use of a Wand of Wonder."));
        }
        else
        {
            b_n = 0;
        }
    }

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Aim the wand */
    (void)borg_aim_wand(sval);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /*  select the correct effect */
    if (selection != -1)
        borg_keypress('b' + selection);

    /* Value */
    return (b_n);
}

/*
 * Simulate/Apply the optimal result of using an un-id'd wand
 */
static int borg_attack_aux_wand_bolt_unknown(int dam, int typ)
{
    int i;
    int b_i = -1;
    int b_n;


    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Paranoia */
    if (borg_simulate && (randint0(100) < 5)) return (0);


    /* Look for an un-id'd wand */
    for (i = 0; i < z_info->pack_size; i++)
    {
        if (borg_items[i].tval != TV_WAND) continue;

        /* known */
        if (borg_items[i].kind) continue;

        /* No charges */
        if (!borg_items[i].pval) continue;
        if (strstr(borg_items[i].desc, "empty")) continue;

        /* Select this wand */
        b_i = i;
    }


    /* None available */
    if (b_i < 0) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(0, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Log the message */
    borg_note(format("# Aiming unknown wand '%s.'", borg_items[b_i].desc));

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = b_i;

    /* Perform the action */
    borg_keypress('a');
    borg_keypress(all_letters_nohjkl[b_i]);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return (b_n);
}

/*
 * Simulate/Apply the optimal result of using an un-id'd rod
 */
static int borg_attack_aux_rod_bolt_unknown(int dam, int typ)
{
    int i;
    int b_i = -1;
    int b_n;


    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Paranoia */
    if (borg_simulate && (randint0(100) < 5)) return (0);


    /* Look for an un-id'd wand */
    for (i = 0; i < z_info->pack_size; i++)
    {
        if (borg_items[i].tval != TV_ROD) continue;

        /* known */
        if (borg_items[i].kind) continue;

        /* No charges */
        if (!borg_items[i].pval) continue;

        /* Not an attacker */
        if (strstr(borg_items[i].desc, "tried")) continue;

        /* Select this rod */
        b_i = i;
    }


    /* None available */
    if (b_i < 0) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(0, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Log the message */
    borg_note(format("# Aiming unknown rod '%s.'", borg_items[b_i].desc));

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = b_i;

    /* Perform the action */
    borg_keypress('z');
    borg_keypress(all_letters_nohjkl[b_i]);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return (b_n);
}

/*
 * Simulate/Apply the optimal result of ACTIVATING an attack artifact
 *
 */
static int borg_attack_aux_activation(int activation, int rad, int dam, int typ, bool aim, int selection)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);


    /* Look for and item with that activation and to see if it is charged */
    if (!borg_equips_item(activation, true)) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Activate the artifact */
    (void)borg_activate_item(activation);

    /* Use target */
    if (aim)
    {
        borg_keypress('5');

        /* Set our shooting flag */
        successful_target = -1;
    }

    /*  select the correct effect */
    if (selection != -1)
        borg_keypress('b' + selection);

    /* Value */
    return (b_n);
}

/*
 * Simulate/Apply the optimal result of ACTIVATING an attack ring
 *
 */
static int borg_attack_aux_ring(int ring_name, int rad, int dam, int typ)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);

    /* Look for that ring and to see if it is charged */
    if (!borg_equips_ring(ring_name)) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Activate the artifact */
    (void)borg_activate_ring(ring_name);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return (b_n);
}

/*
 * Simulate/Apply the optimal result of ACTIVATING a DRAGON ARMOUR
 *
 */
static int borg_attack_aux_dragon(int sval, int rad, int dam, int typ, int selection)
{
    int b_n;


    /* No firing while blind, confused, or hallucinating */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) return (0);


    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2)) return (0);

    /* Randart dragon armors do not activate for breath */
    if (borg_items[INVEN_BODY].art_idx) return (0);

    /* Look for that scale mail and charged*/
    if (!borg_equips_dragon(sval)) return (0);

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate) return (b_n);

    /* Activate the scale mail */
    (void)borg_activate_dragon(sval);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /*  select the correct effect */
    if (selection != -1)
        borg_keypress('b' + selection);

    /* Value */
    return (b_n);
}

/*
 * trying the Whirlwind Attack spell
 */
static int borg_attack_aux_whirlwind_attack(void)
{
    int p;

    int i;
    int d;
    int total_d = 0;

    borg_grid* ag;

    borg_kill* kill;

    /* Can I do it */
    if (!borg_spell_okay_fail(WHIRLWIND_ATTACK, (borg_fighting_unique ? 40 : 25)))
        return (0);

    /* int original_danger = borg_danger(c_y, c_x, 1, false, false); */
    int blows = (borg_skill[BI_CLEVEL] + 10) / 15;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++)
    {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require "adjacent" */
        if (borg_distance(c_y, c_x, y, x) > 1) continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill);

        /* No damage */
        if (d <= 0) continue;

        /* get to do "blows" attacks */
        d = d * blows;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
        {
            /* Calculate danger */
            p = borg_danger_aux(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg_skill[BI_CDEPTH] && !kill->awake) continue;


        /* Calculate "danger" to player */
        p = borg_danger_aux(c_y, c_x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        total_d += d;
    }

    /* Nothing to attack */
    if (total_d < 0) return (0);

    /* Simulation */
    if (borg_simulate) return (total_d);

    /* try the spell */
    if (borg_spell(WHIRLWIND_ATTACK))
        return (total_d);
    return (0);
}


/* trying the Leap into Battle spell */
static int borg_attack_aux_leap_into_battle(void)
{
    int p;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid* ag;

    borg_kill* kill;

    /* Can I do it */
    if (!borg_spell_okay_fail(LEAP_INTO_BATTLE, (borg_fighting_unique ? 40 : 25)))
        return (0);

    /* Too afraid to attack */
    if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR]) return (0);

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++)
    {
        int blows;
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require up to distance 4 */
        int m_dist = borg_distance(c_y, c_x, y, x);
        if (m_dist > 4) continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill);
        blows = (borg_skill[BI_CLEVEL] + 5) / 15;
        blows = ((blows * m_dist + 2) / 4) + 1;
        d *= blows;

        /* No damage */
        if (d <= 0) continue;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
        {
            /* Calculate danger */
            p = borg_danger_aux(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg_skill[BI_CDEPTH] && !kill->awake) continue;


        /* Calculate "danger" to player */
        p = borg_danger_aux(c_y, c_x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d)) continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0) return (0);

    /* Simulation */
    if (borg_simulate) return (b_d);

    /* Save the location */
    g_x = borg_temp_x[b_i];
    g_y = borg_temp_y[b_i];

    ag = &borg_grids[g_y][g_x];
    kill = &borg_kills[ag->kill];

    /* Note */
    borg_note(format("# Leaping at %s at (%d,%d dist %d) who has %d Hit Points.", 
        (r_info[kill->r_idx].name), g_y, g_x, borg_distance(c_y, c_x, g_y, g_x), kill->power));
    borg_note(format("# Attacking with weapon '%s'", borg_items[INVEN_WIELD].desc));

    /* Attack the grid */
    borg_target(g_y, g_x);
    borg_spell(LEAP_INTO_BATTLE);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Success */
    return (b_d);
}


/* trying the Maim Foe spell */
/* this is a thrust but you get 1 blow/15 levels */
/* it also has a chance to stun but ignoring that for now. */
static int borg_attack_aux_maim_foe(void)
{
    int blows;
    int p, dir;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid* ag;

    borg_kill* kill;

    /* Too afraid to attack */
    if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR]) return (0);

    /* Can I do it */
    if (!borg_spell_okay_fail(MAIM_FOE, (borg_fighting_unique ? 40 : 25)))
        return (0);

    blows = borg_skill[BI_CLEVEL] / 15;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++)
    {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require "adjacent" */
        if (borg_distance(c_y, c_x, y, x) > 1) continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill) * blows;

        /* No damage */
        if (d <= 0) continue;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
        {
            /* Calculate danger */
            p = borg_danger_aux(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg_skill[BI_CDEPTH] && !kill->awake) continue;

        /* Calculate "danger" to player */
        p = borg_danger_aux(c_y, c_x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d)) continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0) return (0);

    /* Simulation */
    if (borg_simulate) return (b_d);

    /* Save the location */
    g_x = borg_temp_x[b_i];
    g_y = borg_temp_y[b_i];

    ag = &borg_grids[g_y][g_x];
    kill = &borg_kills[ag->kill];

    /* Get a direction for attacking */
    dir = borg_extract_dir(c_y, c_x, g_y, g_x);

    /* Simulation */
    if (borg_simulate) return (d);

    borg_spell(MAIM_FOE);
    borg_keypress(I2D(dir));

    return (d);
}


/* trying the Curse spell */
static int borg_attack_aux_curse(void)
{
    int p;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid* ag;

    borg_kill* kill;

    /* costs 100hp to cast.  Don't kill yourself doing it */
    if (borg_skill[BI_CURHP] < 120) return (0);

    /* Can I do it */
    if (!borg_spell_okay_fail(CURSE, (borg_fighting_unique ? 40 : 25)))
        return (0);

    /* Too afraid to attack */
    if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR]) return (0);

    /* Examine possible kills */
    for (i = 0; i < borg_temp_n; i++)
    {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Calculate "average" damage */
        d = (((((kill->injury * kill->power) / 100) + 1) / 2) + 50) * (borg_skill[BI_CLEVEL] / 12 + 1);

        /* No damage */
        if (d <= 0) continue;

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
        {
            /* Calculate danger */
            p = borg_danger_aux(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg_skill[BI_CDEPTH] && !kill->awake) continue;


        /* Calculate "danger" to player */
        p = borg_danger_aux(c_y, c_x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d)) continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0) return (0);

    /* Simulation */
    if (borg_simulate) return (b_d);

    /* Save the location */
    g_x = borg_temp_x[b_i];
    g_y = borg_temp_y[b_i];

    ag = &borg_grids[g_y][g_x];
    kill = &borg_kills[ag->kill];

    /* Attack the grid */
    borg_target(g_y, g_x);
    borg_spell(CURSE);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Success */
    return (b_d);
}

/* trying the Vampire Strike spell */
static int borg_attack_aux_vampire_strike(void)
{
    int p;

    int i /* , b_i */ = -1;
    int d, b_d = -1;
    int dist, best_dist = z_info->max_range;
    bool abort_attack = false;

    borg_grid* ag;
    borg_kill* kill;

    /* Can I do it */
    if (!borg_spell_okay_fail(VAMPIRE_STRIKE, (borg_fighting_unique ? 40 : 25)))
        return (0);

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++)
    {
        bool new_low = false;
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];
        int o_x, o_y, x2, y2;

        /* Consider each adjacent spot to the monster */
        /* there must be an empty spot */
        bool found = false;
        for (o_x = -1; o_x <= 1 && !found; o_x++)
        {
            for (o_y = -1; o_y <= 1 && !found; o_y++)
            {
                /* but not the monsters location */
                if (!o_x && !o_y) continue;

                /* Acquire location */
                x2 = borg_temp_x[i] + o_x;
                y2 = borg_temp_y[i] + o_y;

                ag = &borg_grids[y2][x2];
                if (!ag->kill && 
                    ag->feat == FEAT_FLOOR &&
                    (y2 != c_y || x2 != c_x))
                    found = true;
            }
        }
        /* must have an empty square next to the monster */
        if (!found) continue;

        /* Check the projectable, assume unknown grids are walls */
        if (!borg_offset_projectable(c_y, c_x, y, x)) continue;

        /* closest distance */
        dist = borg_distance(c_y, c_x, y, x);
        if (dist > best_dist) continue;
        if (dist < best_dist)
        {
            best_dist = dist;
            new_low = true;
            abort_attack = false;
        }

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_skill[BI_CLEVEL] * 2;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        struct monster_race* r_ptr = &r_info[kill->r_idx];
        if (rf_has(r_ptr->flags, RF_NONLIVING) || rf_has(r_ptr->flags, RF_UNDEAD)) continue;

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg_munchkin_mode)
        {
            /* Calculate danger */
            p = borg_danger_aux(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
            {
                abort_attack = true;
                continue;
            }
        }

        /* Calculate "danger" to player */
        p = borg_danger_aux(c_y, c_x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg_skill[BI_MAXCLEVEL] > 15) p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* if this is a new closest, save the damage otherwise, average it in */
        /* since we will only hit one */
        if (new_low)
            b_d = d;
        else
            b_d = (d + b_d) / 2;
    }

    /* Nothing to attack, require relatively close */
    if (best_dist > 20 || abort_attack) return (0);

    /* Simulation */
    if (borg_simulate) return (b_d);

    /* cast the spell */
    borg_spell(VAMPIRE_STRIKE);

    /* Success */
    return (b_d);
}

/* trying the Crush spell */
/* right now it is coded so that if a monster is partially */
/* crushed, it is still fully a danger */
static int borg_attack_aux_crush(void)
{
    int p1 = 0;
    int p2 = 0;
    int d = 0;


    /* Can I do it */
    if (!borg_spell_okay(CRUSH))
        return (0);

    /* don't kill yourself or leave less than 10hp */
    if ((borg_skill[BI_CURHP] + 10) < (borg_skill[BI_CLEVEL] * 4)) return (0);

    /* Obtain initial danger */
    borg_crush_spell = false;
    p1 = borg_danger(c_y, c_x, 4, true, false);

    /* What effect is there? */
    borg_crush_spell = true;
    p2 = borg_danger(c_y, c_x, 4, true, false);
    borg_crush_spell = false;

    /* damage is reduction in danger */
    d = (p1 - p2);

    /* if there is still danger afterward, make sure the reductioning in HP */
    /* doesn't make this put us in danger */
    int new_hp = (borg_skill[BI_CURHP] - (borg_skill[BI_CLEVEL] * 2));
    if (borg_simulate && (p2 >= new_hp || new_hp <= 5))
        return 0;

    int spell_power = borg_get_spell_power(CRUSH);

    /* Penalize mana usage */
    d = d - spell_power;

    /* Penalize use of reserve mana */
    if (borg_skill[BI_CURSP] - spell_power < borg_skill[BI_MAXSP] / 2) d = d - (spell_power * 10);

    /* Simulation */
    if (borg_simulate) return (d);

    /* Cast the spell */
    if (borg_spell(CRUSH))
        return (d);
    else
        return (0);
}

/*
 * Try to sleep an adjacent bad guy
 * This had been a defence maneuver, which explains the format.
 * This is used for the sleep ii spell and the sanctuary prayer,
 * also the holcolleth activation.
 *
 * There is a slight concern with the level of the artifact and the
 * savings throw.  Currently the borg uses his own level to determine
 * the save.  The artifact level may be lower and the borg will have
 * the false impression that spell will work when in fact the monster
 * may easily save against the attack.
 */
static int borg_attack_aux_trance(void)
{
    int p1 = 0;
    int p2 = 0;
    int d = 0;


    /* Can I do it */
    if (!borg_spell_okay(TRANCE))
        return (0);

    /* Obtain initial danger */
    borg_sleep_spell_ii = false;
    p1 = borg_danger(c_y, c_x, 4, true, false);

    /* What effect is there? */
    borg_sleep_spell_ii = true;
    p2 = borg_danger(c_y, c_x, 4, true, false);
    borg_sleep_spell_ii = false;

    /* value is d, enhance the value for rogues and rangers so that
     * they can use their critical hits.
     */
    d = (p1 - p2);

    int spell_power = borg_get_spell_power(TRANCE);

    /* Penalize mana usage */
    d = d - spell_power;

    /* Penalize use of reserve mana */
    if (borg_skill[BI_CURSP] - spell_power < borg_skill[BI_MAXSP] / 2) d = d - (spell_power * 10);

    /* Simulation */
    if (borg_simulate) return (d);

    /* Cast the spell */
    if (borg_spell(TRANCE))
        return (d);

    return (0);
}
static int borg_attack_aux_artifact_holcolleth(void)
{
    int p1 = 0;
    int p2 = 0;
    int d = 0;

    if (!borg_equips_item(act_sleepii, true))
        return (0);

    /* Obtain initial danger */
    borg_sleep_spell_ii = false;
    p1 = borg_danger(c_y, c_x, 4, true, false);


    /* What effect is there? */
    borg_sleep_spell_ii = true;
    p2 = borg_danger(c_y, c_x, 4, true, false);
    borg_sleep_spell_ii = false;

    /* value is d, enhance the value for rogues and rangers so that
     * they can use their critical hits.
     */
    d = (p1 - p2);

    /* Simulation */
    if (borg_simulate) return (d);

    /* Cast the spell */
    if (borg_activate_item(act_sleepii))
    {
        /* Value */
        return (d);
    }
    else
    {
        borg_note("# Failed to properly activate the artifact");
        return (0);
    }
}


/*
 * Simulate/Apply the optimal result of using the given "type" of attack
 */
static int borg_attack_aux(int what)
{
    int dam = 0;
    int rad = 0;

    /* Analyze */
    switch (what)
    {
        /* Wait on grid for monster to approach me */
    case BF_REST:
    return (borg_attack_aux_rest());

    /* Physical attack */
    case BF_THRUST:
    return (borg_attack_aux_thrust());

    /* Fired missile attack */
    case BF_LAUNCH:
    return (borg_attack_aux_launch());

    /* Object attack */
    case BF_OBJECT:
    return (borg_attack_aux_object());


    /* Spell -- slow monster */
    case BF_SPELL_SLOW_MONSTER:
    dam = 10;
    return (borg_attack_aux_spell_bolt(SLOW_MONSTER, rad, dam, BORG_ATTACK_OLD_SLOW, z_info->max_range));

    /* Spell -- confuse monster */
    case BF_SPELL_CONFUSE_MONSTER:
    rad = 0;
    dam = 10;
    return (borg_attack_aux_spell_bolt(CONFUSE_MONSTER, rad, dam, BORG_ATTACK_OLD_CONF, z_info->max_range));

    case BF_SPELL_SLEEP_III:
    dam = 10;
    return (borg_attack_aux_spell_dispel(MASS_SLEEP, dam, BORG_ATTACK_OLD_SLEEP));

    /* Spell -- magic missile */
    case BF_SPELL_MAGIC_MISSILE:
    rad = 0;
    dam = ((((borg_skill[BI_CLEVEL] - 1) / 5) + 3) * (4 + 1)) / 2;
    return (borg_attack_aux_spell_bolt(MAGIC_MISSILE, rad, dam, BORG_ATTACK_MISSILE, z_info->max_range));

    /* Spell -- magic missile EMERGENCY*/
    case BF_SPELL_MAGIC_MISSILE_RESERVE:
    rad = 0;
    dam = ((((borg_skill[BI_CLEVEL] - 1) / 5) + 3) * (4 + 1));
    return (borg_attack_aux_spell_bolt_reserve(MAGIC_MISSILE, rad, dam, BORG_ATTACK_MISSILE, z_info->max_range));

    /* Spell -- cold bolt */
    case BF_SPELL_COLD_BOLT:
    rad = 0;
    dam = ((((borg_skill[BI_CLEVEL] - 5) / 3) + 6) * (8 + 1)) / 2;
    return (borg_attack_aux_spell_bolt(FROST_BOLT, rad, dam, BORG_ATTACK_COLD, z_info->max_range));

    /* Spell -- kill wall */
    case BF_SPELL_STONE_TO_MUD:
    rad = 0;
    dam = (20 + (30 / 2));
    return (borg_attack_aux_spell_bolt(TURN_STONE_TO_MUD, rad, dam, BORG_ATTACK_KILL_WALL, z_info->max_range));

    /* Spell -- light beam */
    case BF_SPELL_LIGHT_BEAM:
    rad = -1;
    dam = (6 * (8 + 1) / 2);
    return (borg_attack_aux_spell_bolt(SPEAR_OF_LIGHT, rad, dam, BORG_ATTACK_LIGHT_WEAK, z_info->max_range));

    /* Spell -- stinking cloud */
    case BF_SPELL_STINK_CLOUD:
    rad = 2;
    dam = (10 + (borg_skill[BI_CLEVEL] / 2));
    return (borg_attack_aux_spell_bolt(STINKING_CLOUD, rad, dam, BORG_ATTACK_POIS, z_info->max_range));

    /* Spell -- fire ball */
    case BF_SPELL_FIRE_BALL:
    rad = 2;
    dam = (borg_skill[BI_CLEVEL] * 2);
    return (borg_attack_aux_spell_bolt(FIRE_BALL, rad, dam, BORG_ATTACK_FIRE, z_info->max_range));

    /* Spell -- Ice Storm */
    case BF_SPELL_COLD_STORM:
    rad = 3;
    dam = (3 * ((borg_skill[BI_CLEVEL] * 3) + 1)) / 2;
    return (borg_attack_aux_spell_bolt(ICE_STORM, rad, dam, BORG_ATTACK_ICE, z_info->max_range));

    /* Spell -- Meteor Swarm */
    case BF_SPELL_METEOR_SWARM:
    rad = 3;
    dam = (30 + borg_skill[BI_CLEVEL] / 2) + (borg_skill[BI_CLEVEL] / 20) + 2;
    return (borg_attack_aux_spell_bolt(METEOR_SWARM, rad, dam, BORG_ATTACK_METEOR, z_info->max_range));

    /* Spell -- Rift */
    case BF_SPELL_RIFT:
    rad = -1;
    dam = ((borg_skill[BI_CLEVEL] * 3) + 40);
    return (borg_attack_aux_spell_bolt(RIFT, rad, dam, BORG_ATTACK_GRAVITY, z_info->max_range));

    /* Spell -- mana storm */
    case BF_SPELL_MANA_STORM:
    rad = 3;
    dam = (300 + (borg_skill[BI_CLEVEL] * 2));
    return (borg_attack_aux_spell_bolt(MANA_STORM, rad, dam, BORG_ATTACK_MANA, z_info->max_range));

    /* Spell -- Shock Wave */
    case BF_SPELL_SHOCK_WAVE:
    dam = (borg_skill[BI_CLEVEL] * 2);
    rad = 2;
    return (borg_attack_aux_spell_bolt(SHOCK_WAVE, rad, dam, BORG_ATTACK_SOUND, z_info->max_range));

    /* Spell -- Explosion */
    case BF_SPELL_EXPLOSION:
    dam = ((borg_skill[BI_CLEVEL] * 2) + (borg_skill[BI_CLEVEL] / 5)); /* hack pretend it is all shards */
    rad = 2;
    return (borg_attack_aux_spell_bolt(EXPLOSION, rad, dam, BORG_ATTACK_SHARD, z_info->max_range));

    /* Prayer -- orb of draining */
    case BF_PRAYER_HOLY_ORB_BALL:
    rad = ((borg_skill[BI_CLEVEL] >= 30) ? 3 : 2);
    dam = ((borg_skill[BI_CLEVEL] * 3) / 2) + (3 * (6 + 1)) / 2;
    return (borg_attack_aux_spell_bolt(ORB_OF_DRAINING, rad, dam, BORG_ATTACK_HOLY_ORB, z_info->max_range));

    /* Prayer -- blind creature */
    case BF_SPELL_BLIND_CREATURE:
    rad = 0;
    dam = 10;
    return (borg_attack_aux_spell_bolt(FRIGHTEN, rad, dam, BORG_ATTACK_OLD_CONF, z_info->max_range));

    /* Druid - Trance */
    case BF_SPELL_TRANCE:
    return (borg_attack_aux_trance());

    /* Prayer -- Dispel Undead */
    case BF_PRAYER_DISP_UNDEAD:
    dam = (((borg_skill[BI_CLEVEL] * 5) + 1) / 2);
    return (borg_attack_aux_spell_dispel(DISPEL_UNDEAD, dam, BORG_ATTACK_DISP_UNDEAD));

    /* Prayer -- Dispel Evil */
    case BF_PRAYER_DISP_EVIL:
    dam = (((borg_skill[BI_CLEVEL] * 5) + 1) / 2);
    return (borg_attack_aux_spell_dispel(DISPEL_EVIL, dam, BORG_ATTACK_DISP_EVIL));

    /* Prayer -- Dispel Undead */
    case BF_PRAYER_DISP_SPIRITS:
    dam = (100);
    return (borg_attack_aux_spell_dispel(BANISH_SPIRITS, dam, BORG_ATTACK_DISP_SPIRITS));

    /* Prayer -- Banishment (teleport evil away)*/
    /* This is a defense spell:  done in borg_defense() */

    /* Prayer -- Holy Word also has heal effect and is considered in borg_heal */
    case BF_PRAYER_HOLY_WORD:
    if (borg_skill[BI_MAXHP] - borg_skill[BI_CURHP] >= 300)
        /* force him to think the spell is more deadly to get him to
         * cast it.  This will provide some healing for him.
         */
    {
        dam = ((borg_skill[BI_CLEVEL] * 10));
        return (borg_attack_aux_spell_dispel(HOLY_WORD, dam, BORG_ATTACK_DISP_EVIL));
    }
    else /* If he is not wounded dont cast this, use Disp Evil instead. */
    {
        dam = ((borg_skill[BI_CLEVEL] * 3) / 2) - 50;
        return (borg_attack_aux_spell_dispel(DISPEL_EVIL, dam, BORG_ATTACK_DISP_EVIL));
    }

    /* Prayer -- Annihilate */
    case BF_SPELL_ANNIHILATE:
    rad = 0;
    dam = (borg_skill[BI_CLEVEL] * 4);
    return (borg_attack_aux_spell_bolt(ANNIHILATE, rad, dam, BORG_ATTACK_OLD_DRAIN, z_info->max_range));

    /* Spell -- Electric Arc */
    case BF_SPELL_ELECTRIC_ARC:
    rad = 0;
    dam = ((((borg_skill[BI_CLEVEL] - 1) / 5) + 3) * (6 + 1)) / 2;
    return (borg_attack_aux_spell_bolt(ELECTRIC_ARC, rad, dam, BORG_ATTACK_ELEC, borg_skill[BI_CLEVEL]));

    case BF_SPELL_ACID_SPRAY:
    rad = 3; /* HACK just pretend it is wide. */
    dam = ((borg_skill[BI_CLEVEL] / 2) * (8 + 1)) / 2;
    return (borg_attack_aux_spell_bolt(ACID_SPRAY, rad, dam, BORG_ATTACK_ACID, 10));

    /* Spell -- mana bolt */
    case BF_SPELL_MANA_BOLT:
    rad = 0;
    dam = ((borg_skill[BI_CLEVEL] - 10) * (8 + 1) / 2);
    return (borg_attack_aux_spell_bolt(MANA_BOLT, rad, dam, BORG_ATTACK_MANA, z_info->max_range));

    /* Spell -- thrust away */
    case BF_SPELL_THRUST_AWAY:
    rad = 0;
    dam = (borg_skill[BI_CLEVEL] * (8 + 1) / 2);
    return (borg_attack_aux_spell_bolt(THRUST_AWAY, rad, dam, BORG_ATTACK_FORCE, (borg_skill[BI_CLEVEL] / 10) + 1));

    /* Spell -- Lightning Strike */
    case BF_SPELL_LIGHTNING_STRIKE:
    rad = 0;
    dam = ((borg_skill[BI_CLEVEL] / 4) * (4 + 1) / 2) + borg_skill[BI_CLEVEL] + 5;  /* HACK pretend it is all elec */
    return (borg_attack_aux_spell_bolt(LIGHTNING_STRIKE, rad, dam, BORG_ATTACK_ELEC, z_info->max_range));

    /* Spell -- Earth Rising */
    case BF_SPELL_EARTH_RISING:
    rad = 0;
    dam = (((borg_skill[BI_CLEVEL] / 3) + 2) * (6 + 1) / 2) + borg_skill[BI_CLEVEL] + 5;
    return (borg_attack_aux_spell_bolt(EARTH_RISING, rad, dam, BORG_ATTACK_SHARD, (borg_skill[BI_CLEVEL] / 5) + 4));

    /* Spell -- Volcanic Eruption */
    /* just count the damage.  The earthquake defence is a side bennie, perhaps... */
    case BF_SPELL_VOLCANIC_ERUPTION:
    rad = 0;
    dam = (((borg_skill[BI_CLEVEL] * 3) / 2) * ((borg_skill[BI_CLEVEL] * 3) + 1)) / 2;
    return (borg_attack_aux_spell_bolt(VOLCANIC_ERUPTION, rad, dam, BORG_ATTACK_FIRE, z_info->max_range));

    /* Spell -- River of Lightning */
    case BF_SPELL_RIVER_OF_LIGHTNING:
    rad = 2;
    dam = (borg_skill[BI_CLEVEL] + 10) * (8 + 1) / 2;
    return (borg_attack_aux_spell_bolt(RIVER_OF_LIGHTNING, rad, dam, BORG_ATTACK_PLASMA, 20));

    /* spell -- Spear of Oromë */
    case BF_SPELL_SPEAR_OF_OROME:
    rad = 0;
    dam = ((borg_skill[BI_CLEVEL] / 2) + (8 + 1)) / 2;
    return (borg_attack_aux_spell_bolt(SPEAR_OF_OROME, rad, dam, BORG_ATTACK_HOLY_ORB, z_info->max_range));

    /* spell -- Light of Manwë */
    case BF_SPELL_LIGHT_OF_MANWE:
    rad = 0;
    dam = borg_skill[BI_CLEVEL] * 5 + 100;
    return (borg_attack_aux_spell_bolt(LIGHT_OF_MANWE, rad, dam, BORG_ATTACK_LIGHT, z_info->max_range));

    /* spell -- Nether Bolt */
    case BF_SPELL_NETHER_BOLT:
    rad = 0;
    dam = ((((borg_skill[BI_CLEVEL] / 4) + 3) * (4 + 1)) / 2);
    return (borg_attack_aux_spell_bolt(NETHER_BOLT, rad, dam, BORG_ATTACK_NETHER, z_info->max_range));

    /* spell -- Tap Unlife */
    case BF_SPELL_TAP_UNLIFE:
    dam = ((((borg_skill[BI_CLEVEL] / 4) + 3) * (4 + 1)) / 2);
    return (borg_attack_aux_spell_dispel(TAP_UNLIFE, dam, BORG_ATTACK_TAP_UNLIFE));

    /* Spell - Crush */
    case BF_SPELL_CRUSH:
    return (borg_attack_aux_crush());

    case BF_SPELL_SLEEP_EVIL:
    dam = borg_skill[BI_CLEVEL] * 10 + 500;
    return (borg_attack_aux_spell_dispel(SLEEP_EVIL, dam, BORG_ATTACK_SLEEP_EVIL));

    /* spell -- Disenchant */
    case BF_SPELL_DISENCHANT:
    rad = 0;
    dam = ((((borg_skill[BI_CLEVEL] * 2) + 10) + 1) / 2) * 2;
    return (borg_attack_aux_spell_bolt(DISENCHANT, rad, dam, BORG_ATTACK_DISEN, z_info->max_range));

    /* spell -- Frighten */
    case BF_SPELL_FRIGHTEN:
    rad = 0;
    dam = borg_skill[BI_CLEVEL];
    return (borg_attack_aux_spell_bolt(FRIGHTEN, rad, dam, BORG_ATTACK_TURN_ALL, z_info->max_range));

    /* Spell - Vampire Strike*/
    case BF_SPELL_VAMPIRE_STRIKE:
    return (borg_attack_aux_vampire_strike());

    /* Spell - Dispel Life */
    case BF_PRAYER_DISPEL_LIFE:
    rad = 0;
    dam = ((borg_skill[BI_CLEVEL] * 3) + 1) / 2;
    return (borg_attack_aux_spell_bolt(DISPEL_LIFE, rad, dam, BORG_ATTACK_DRAIN_LIFE, z_info->max_range));

    /* spell -- Dark Spear */
    case BF_SPELL_DARK_SPEAR:
    rad = 0;
    dam = (((borg_skill[BI_CLEVEL] * 2) + 1) / 2) * 2;
    return (borg_attack_aux_spell_bolt(DARK_SPEAR, rad, dam, BORG_ATTACK_DARK, z_info->max_range));

    /* spell -- Unleash Chaos */
    case BF_SPELL_UNLEASH_CHAOS:
    rad = 0;
    dam = ((borg_skill[BI_CLEVEL] + 1) / 2) * 8;
    return (borg_attack_aux_spell_bolt(UNLEASH_CHAOS, rad, dam, BORG_ATTACK_CHAOS, z_info->max_range));

    /* Spell -- Storm of Darkness */
    case BF_SPELL_STORM_OF_DARKNESS:
    rad = 4;
    dam = (((borg_skill[BI_CLEVEL] * 2) + 1) / 2) * 4;
    return (borg_attack_aux_spell_bolt(STORM_OF_DARKNESS, rad, dam, BORG_ATTACK_DARK, z_info->max_range));

    /* Spell - Curse */
    case BF_SPELL_CURSE:
    return (borg_attack_aux_curse());

    /* spell - Whirlwind Attack */
    case BF_SPELL_WHIRLWIND_ATTACK:
    return (borg_attack_aux_whirlwind_attack());

    /* spell - Leap into Battle */
    case BF_SPELL_LEAP_INTO_BATTLE:
    return (borg_attack_aux_leap_into_battle());

    /* spell - Leap into Battle */
    case BF_SPELL_MAIM_FOE:
    return (borg_attack_aux_maim_foe());

    /* spell - Howl of the Damned */
    case BF_SPELL_HOWL_OF_THE_DAMNED:
    dam = borg_skill[BI_CLEVEL];
    return (borg_attack_aux_spell_dispel(HOWL_OF_THE_DAMNED, dam, BORG_ATTACK_TURN_ALL));

    /* ROD -- slow monster */
    case BF_ROD_SLOW_MONSTER:
    dam = 10;
    rad = 0;
    return (borg_attack_aux_rod_bolt(sv_rod_slow_monster, rad, dam, BORG_ATTACK_OLD_SLOW));

    /* ROD -- sleep monster */
    case BF_ROD_SLEEP_MONSTER:
    dam = 10;
    rad = 0;
    return (borg_attack_aux_rod_bolt(sv_rod_sleep_monster, rad, dam, BORG_ATTACK_OLD_SLEEP));

    /* Rod -- elec bolt */
    case BF_ROD_ELEC_BOLT:
    rad = -1;
    dam = 6 * (6 + 1) / 2;
    return (borg_attack_aux_rod_bolt(sv_rod_elec_bolt, rad, dam, BORG_ATTACK_ELEC));

    /* Rod -- cold bolt */
    case BF_ROD_COLD_BOLT:
    rad = 0;
    dam = 12 * (8 + 1) / 2;
    return (borg_attack_aux_rod_bolt(sv_rod_cold_bolt, rad, dam, BORG_ATTACK_COLD));

    /* Rod -- acid bolt */
    case BF_ROD_ACID_BOLT:
    rad = 0;
    dam = 12 * (8 + 1) / 2;
    return (borg_attack_aux_rod_bolt(sv_rod_acid_bolt, rad, dam, BORG_ATTACK_ACID));

    /* Rod -- fire bolt */
    case BF_ROD_FIRE_BOLT:
    rad = 0;
    dam = 12 * (8 + 1) / 2;
    return (borg_attack_aux_rod_bolt(sv_rod_fire_bolt, rad, dam, BORG_ATTACK_FIRE));

    /* Rod -- light beam */
    case BF_ROD_LIGHT_BEAM:
    rad = -1;
    dam = (6 * (8 + 1) / 2);
    return (borg_attack_aux_rod_bolt(sv_rod_light, rad, dam, BORG_ATTACK_LIGHT_WEAK));

    /* Rod -- drain life */
    case BF_ROD_DRAIN_LIFE:
    rad = 0;
    dam = (150);
    return (borg_attack_aux_rod_bolt(sv_rod_drain_life, rad, dam, BORG_ATTACK_OLD_DRAIN));

    /* Rod -- elec ball */
    case BF_ROD_ELEC_BALL:
    rad = 2;
    dam = 64;
    return (borg_attack_aux_rod_bolt(sv_rod_elec_ball, rad, dam, BORG_ATTACK_ELEC));

    /* Rod -- acid ball */
    case BF_ROD_COLD_BALL:
    rad = 2;
    dam = 100;
    return (borg_attack_aux_rod_bolt(sv_rod_cold_ball, rad, dam, BORG_ATTACK_COLD));

    /* Rod -- acid ball */
    case BF_ROD_ACID_BALL:
    rad = 2;
    dam = 120;
    return (borg_attack_aux_rod_bolt(sv_rod_acid_ball, rad, dam, BORG_ATTACK_ACID));

    /* Rod -- fire ball */
    case BF_ROD_FIRE_BALL:
    rad = 2;
    dam = 144;
    return (borg_attack_aux_rod_bolt(sv_rod_fire_ball, rad, dam, BORG_ATTACK_FIRE));

    /* Rod -- unid'd rod */
    case BF_ROD_UNKNOWN:
    rad = 0;
    dam = 75;
    return (borg_attack_aux_rod_bolt_unknown(dam, BORG_ATTACK_MISSILE));

    /* Wand -- unid'd wand */
    case BF_WAND_UNKNOWN:
    rad = 0;
    dam = 75;
    return (borg_attack_aux_wand_bolt_unknown(dam, BORG_ATTACK_MISSILE));

    /* Wand -- magic missile */
    case BF_WAND_MAGIC_MISSILE:
    rad = 0;
    dam = 3 * (4 + 1) / 2;
    return (borg_attack_aux_wand_bolt(sv_wand_magic_missile, rad, dam, BORG_ATTACK_MISSILE, -1));

    /* Wand -- slow monster */
    case BF_WAND_SLOW_MONSTER:
    rad = 0;
    dam = 10;
    return (borg_attack_aux_wand_bolt(sv_wand_slow_monster, rad, dam, BORG_ATTACK_OLD_SLOW, -1));

    /* Wand -- sleep monster */
    case BF_WAND_HOLD_MONSTER:
    rad = 0;
    dam = 10;
    return (borg_attack_aux_wand_bolt(sv_wand_hold_monster, rad, dam, BORG_ATTACK_OLD_SLEEP, -1));

    /* Wand -- fear monster */
    case BF_WAND_FEAR_MONSTER:
    rad = 0;
    dam = 2 * (6 + 1) / 2;
    return (borg_attack_aux_wand_bolt(sv_wand_fear_monster, rad, dam, BORG_ATTACK_TURN_ALL, -1));

    /* Wand -- conf monster */
    case BF_WAND_CONFUSE_MONSTER:
    rad = 0;
    dam = 2 * (6 + 1) / 2;
    return (borg_attack_aux_wand_bolt(sv_wand_confuse_monster, rad, dam, BORG_ATTACK_OLD_CONF, -1));

    /* Wand -- elec bolt */
    case BF_WAND_ELEC_BOLT:
    dam = 6 * (6 + 1) / 2;
    rad = -1;
    return (borg_attack_aux_wand_bolt(sv_wand_elec_bolt, rad, dam, BORG_ATTACK_ELEC, -1));

    /* Wand -- cold bolt */
    case BF_WAND_COLD_BOLT:
    dam = 12 * (8 + 1) / 2;
    rad = 0;
    return (borg_attack_aux_wand_bolt(sv_wand_cold_bolt, rad, dam, BORG_ATTACK_COLD, -1));

    /* Wand -- acid bolt */
    case BF_WAND_ACID_BOLT:
    rad = 0;
    dam = 5 * (8 + 1) / 2;
    return (borg_attack_aux_wand_bolt(sv_wand_acid_bolt, rad, dam, BORG_ATTACK_ACID, -1));

    /* Wand -- fire bolt */
    case BF_WAND_FIRE_BOLT:
    rad = 0;
    dam = 12 * (8 + 1) / 2;
    return (borg_attack_aux_wand_bolt(sv_wand_fire_bolt, rad, dam, BORG_ATTACK_FIRE, -1));

    /* Spell -- light beam */
    case BF_WAND_LIGHT_BEAM:
    rad = -1;
    dam = (6 * (8 + 1) / 2);
    return (borg_attack_aux_wand_bolt(sv_wand_light, rad, dam, BORG_ATTACK_LIGHT_WEAK, -1));

    /* Wand -- stinking cloud */
    case BF_WAND_STINKING_CLOUD:
    rad = 2;
    dam = 12;
    return (borg_attack_aux_wand_bolt(sv_wand_stinking_cloud, rad, dam, BORG_ATTACK_POIS, -1));

    /* Wand -- elec ball */
    case BF_WAND_ELEC_BALL:
    rad = 2;
    dam = 64;
    return (borg_attack_aux_wand_bolt(sv_wand_elec_ball, rad, dam, BORG_ATTACK_ELEC, -1));

    /* Wand -- acid ball */
    case BF_WAND_COLD_BALL:
    rad = 2;
    dam = 100;
    return (borg_attack_aux_wand_bolt(sv_wand_cold_ball, rad, dam, BORG_ATTACK_COLD, -1));

    /* Wand -- acid ball */
    case BF_WAND_ACID_BALL:
    rad = 2;
    dam = 120;
    return (borg_attack_aux_wand_bolt(sv_wand_acid_ball, rad, dam, BORG_ATTACK_ACID, -1));

    /* Wand -- fire ball */
    case BF_WAND_FIRE_BALL:
    rad = 2;
    dam = 144;
    return (borg_attack_aux_wand_bolt(sv_wand_fire_ball, rad, dam, BORG_ATTACK_FIRE, -1));

    /* Wand -- dragon cold */
    case BF_WAND_DRAGON_COLD:
    rad = 3;
    dam = 160;
    return (borg_attack_aux_wand_bolt(sv_wand_dragon_cold, rad, dam, BORG_ATTACK_COLD, -1));

    /* Wand -- dragon fire */
    case BF_WAND_DRAGON_FIRE:
    rad = 3;
    dam = 200;
    return (borg_attack_aux_wand_bolt(sv_wand_dragon_fire, rad, dam, BORG_ATTACK_FIRE, -1));

    /* Wand -- annihilation */
    case BF_WAND_ANNIHILATION:
    dam = 250;
    return (borg_attack_aux_wand_bolt(sv_wand_annihilation, rad, dam, BORG_ATTACK_OLD_DRAIN, -1));

    /* Wand -- drain life */
    case BF_WAND_DRAIN_LIFE:
    dam = 150;
    return (borg_attack_aux_wand_bolt(sv_wand_drain_life, rad, dam, BORG_ATTACK_OLD_DRAIN, -1));

    /* Wand -- wand of wonder */
    case BF_WAND_WONDER:
    dam = 35;
    return (borg_attack_aux_wand_bolt(sv_wand_wonder, rad, dam, BORG_ATTACK_MISSILE, -1));

    /* Staff -- Sleep Monsters */
    case BF_STAFF_SLEEP_MONSTERS:
    dam = 60;
    return (borg_attack_aux_staff_dispel(sv_staff_sleep_monsters, rad, dam, BORG_ATTACK_OLD_SLEEP));

    /* Staff -- Slow Monsters */
    case BF_STAFF_SLOW_MONSTERS:
    dam = 60;
    rad = 10;
    return (borg_attack_aux_staff_dispel(sv_staff_slow_monsters, rad, dam, BORG_ATTACK_OLD_SLOW));

    /* Staff -- Dispel Evil */
    case BF_STAFF_DISPEL_EVIL:
    dam = 60;
    return (borg_attack_aux_staff_dispel(sv_staff_dispel_evil, rad, dam, BORG_ATTACK_DISP_EVIL));

    /* Staff -- Power */
    case BF_STAFF_POWER:
    dam = 120;
    return (borg_attack_aux_staff_dispel(sv_staff_power, rad, dam, BORG_ATTACK_TURN_ALL));

    /* Staff -- holiness */
    case BF_STAFF_HOLINESS:
    if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) dam = 500;
    else dam = 120;
    return (borg_attack_aux_staff_dispel(sv_staff_holiness, rad, dam, BORG_ATTACK_DISP_EVIL));


    /* Artifact -- Narthanc- fire bolt 9d8*/
    case BF_EF_FIRE1:
    rad = 0;
    dam = (9 * (8 + 1) / 2);
    return (borg_attack_aux_activation(act_fire_bolt, rad, dam, BORG_ATTACK_FIRE, true, -1));

    /* Artifact -- Anduril & Firestar- fire bolt 72*/
    case BF_EF_FIRE2:
    rad = 2;
    dam = 72;
    return (borg_attack_aux_activation(act_fire_bolt72, rad, dam, BORG_ATTACK_FIRE, true, -1));

    /* Artifact -- Gothmog- FIRE BALL 144 */
    case BF_EF_FIRE3:
    rad = 2;
    dam = 144;
    return (borg_attack_aux_activation(act_fire_ball, rad, dam, BORG_ATTACK_FIRE, true, -1));

    /* Artifact -- Nimthanc & Paurnimmen- frost bolt 6d8*/
    case BF_EF_FROST1:
    rad = 0;
    dam = (6 * (8 + 1) / 2);
    return (borg_attack_aux_activation(act_cold_bolt, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Belangil- frost ball 50 */
    case BF_EF_FROST2:
    rad = 2;
    dam = 50;
    return (borg_attack_aux_activation(act_cold_ball50, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Aranrúth- frost bolt 12d8*/
    case BF_EF_FROST4:
    rad = 0;
    dam = (12 * (8 + 1) / 2);
    return (borg_attack_aux_activation(act_cold_bolt2, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Ringil- frost ball 100*/
    case BF_EF_FROST3:
    rad = 2;
    dam = 100;
    return (borg_attack_aux_activation(act_cold_ball100, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Dethanc- electric bolt 6d6*/
    case BF_EF_LIGHTNING_BOLT:
    rad = -1;
    dam = (6 * (6 + 1) / 2);
    return (borg_attack_aux_activation(act_elec_bolt, rad, dam, BORG_ATTACK_ELEC, true, -1));

    /* Artifact -- Rilia- poison gas 12*/
    case BF_EF_STINKING_CLOUD:
    rad = 2;
    dam = 12;
    return (borg_attack_aux_activation(act_stinking_cloud, rad, dam, BORG_ATTACK_POIS, true, -1));

    /* Artifact -- Theoden- drain Life 120*/
    case BF_EF_DRAIN_LIFE2:
    rad = 0;
    dam = 120;
    return (borg_attack_aux_activation(act_drain_life2, rad, dam, BORG_ATTACK_OLD_DRAIN, true, -1));

    /* Artifact -- Totila- confustion */
    case BF_EF_CONFUSE:
    rad = 0;
    dam = 20;
    return (borg_attack_aux_activation(act_confuse2, rad, dam, BORG_ATTACK_OLD_CONF, true, -1));

    /* Artifact -- Holcolleth -- sleep ii and sanctuary */
    case BF_EF_SLEEP:
    dam = 10;
    return (borg_attack_aux_artifact_holcolleth());

    /* Artifact -- TURMIL- drain life 90 */
    case BF_EF_DRAIN_LIFE1:
    rad = 0;
    dam = 90;
    return (borg_attack_aux_activation(act_drain_life1, rad, dam, BORG_ATTACK_OLD_DRAIN, true, -1));

    /* Artifact -- Fingolfin- spikes 150 */
    case BF_EF_ARROW:
    rad = 0;
    dam = 150;
    return (borg_attack_aux_activation(act_arrow, rad, dam, BORG_ATTACK_MISSILE, true, -1));

    /* Artifact -- Cammithrim- Magic Missile 3d4 */
    case BF_EF_MISSILE:
    rad = 0;
    dam = (3 * (4 + 1) / 2);
    return (borg_attack_aux_activation(act_missile, rad, dam, BORG_ATTACK_MISSILE, true, -1));

    /* Artifact -- Paurnen- ACID bolt 5d8 */
    case BF_EF_ACID1:
    rad = 0;
    dam = (5 * (8 + 1) / 2);
    return (borg_attack_aux_activation(act_acid_bolt, rad, dam, BORG_ATTACK_ACID, true, -1));

    /* Artifact -- INGWE- DISPEL EVIL X5 */
    case BF_EF_DISP_EVIL:
    rad = 10;
    dam = (10 + (borg_skill[BI_CLEVEL] * 5) / 2);
    return (borg_attack_aux_activation(act_dispel_evil, rad, dam, BORG_ATTACK_DISP_EVIL, true, -1));

    /* Artifact -- Eöl -- Mana Bolt 12d8 */
    case BF_EF_MANA_BOLT:
    rad = 0;
    dam = (12 * (8 + 1)) / 2;
    return (borg_attack_aux_activation(act_mana_bolt, rad, dam, BORG_ATTACK_MANA, true, -1));

    /* Artifact -- Razorback and Mediator */
    case BF_EF_STAR_BALL:
    rad = 3;
    dam = 150;
    return (borg_attack_aux_activation(act_star_ball, rad, dam, BORG_ATTACK_ELEC, true, -1));

    /* Artifact -- Gil-galad */
    case BF_EF_STARLIGHT2:
    rad = 7;
    dam = (10 * (8 + 1)) / 2;
    return (borg_attack_aux_activation(act_starlight2, rad, dam, BORG_ATTACK_LIGHT, false, -1));

    /* Artifact -- randart */
    case BF_EF_STARLIGHT:
    rad = 7;
    dam = (6 * (8 + 1)) / 2;
    return (borg_attack_aux_activation(act_starlight, rad, dam, BORG_ATTACK_LIGHT, false, -1));


    /* Ring of ACID */
    case BF_RING_ACID:
    rad = 2;
    dam = 70;
    return (borg_attack_aux_ring(sv_ring_acid, rad, dam, BORG_ATTACK_ACID));

    /* Ring of FLAMES */
    case BF_RING_FIRE:
    rad = 2;
    dam = 80;
    return (borg_attack_aux_ring(sv_ring_flames, rad, dam, BORG_ATTACK_FIRE));

    /* Ring of ICE */
    case BF_RING_ICE:
    rad = 2;
    dam = 75;
    return (borg_attack_aux_ring(sv_ring_ice, rad, dam, BORG_ATTACK_ICE));

    /* Ring of LIGHTNING */
    case BF_RING_LIGHTNING:
    rad = 2;
    dam = 85;
    return (borg_attack_aux_ring(sv_ring_lightning, rad, dam, BORG_ATTACK_ELEC));


    /* Hack -- Dragon Scale Mail can be activated as well */
    case BF_DRAGON_BLUE:
    rad = 2;
    dam = 150;
    return (borg_attack_aux_dragon(sv_dragon_blue, rad, dam, BORG_ATTACK_ELEC, -1));

    case BF_DRAGON_WHITE:
    rad = 2;
    dam = 100;
    return (borg_attack_aux_dragon(sv_dragon_white, rad, dam, BORG_ATTACK_COLD, -1));

    case BF_DRAGON_BLACK:
    rad = 2;
    dam = 120;
    return (borg_attack_aux_dragon(sv_dragon_black, rad, dam, BORG_ATTACK_ACID, -1));

    case BF_DRAGON_GREEN:
    rad = 2;
    dam = 150;
    return (borg_attack_aux_dragon(sv_dragon_green, rad, dam, BORG_ATTACK_POIS, -1));

    case BF_DRAGON_RED:
    rad = 2;
    dam = 200;
    return (borg_attack_aux_dragon(sv_dragon_red, rad, dam, BORG_ATTACK_FIRE, -1));

    case BF_DRAGON_MULTIHUED:
    {
        int     value[5];
        int     type[5] =
            {BORG_ATTACK_ELEC, 
             BORG_ATTACK_COLD, 
             BORG_ATTACK_ACID, 
             BORG_ATTACK_POIS, 
             BORG_ATTACK_FIRE};
        int     biggest = 0;
        bool    tmp_simulate = borg_simulate;

        rad = 2;
        dam = 250;
        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 5; x++)
            value[x] = borg_attack_aux_dragon(sv_dragon_multihued, rad, dam, type[x], x);

        for (int x = 1; x < 5; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(sv_dragon_multihued, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_GOLD:
    rad = 2;
    dam = 150;
    return (borg_attack_aux_dragon(sv_dragon_gold, rad, dam, BORG_ATTACK_SOUND, -1));

    case BF_DRAGON_CHAOS:
    {
        int     value[2];
        int     type[2] = { BORG_ATTACK_CHAOS, BORG_ATTACK_DISEN };
        int     biggest = 0;
        bool    tmp_simulate = borg_simulate;

        rad = 2;
        dam = 220;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_dragon(sv_dragon_chaos, rad, dam, type[x], x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(sv_dragon_chaos, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_LAW:
    {
        int     value[2];
        int     type[2] ={ BORG_ATTACK_SOUND, BORG_ATTACK_SHARD };
            int     biggest = 0;
        bool    tmp_simulate = borg_simulate;

        rad = 2;
        dam = 220;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_dragon(sv_dragon_law, rad, dam, type[x], x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(sv_dragon_law, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_BALANCE:
    {
        int     value[4];
        int     type[4] =
            {BORG_ATTACK_CHAOS,
             BORG_ATTACK_DISEN,
             BORG_ATTACK_SOUND,
             BORG_ATTACK_SHARD};
        int     biggest = 0;
        bool    tmp_simulate = borg_simulate;

        rad = 2;
        dam = 250;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 4; x++)
            value[x] = borg_attack_aux_dragon(sv_dragon_balance, rad, dam, type[x], x);

        for (int x = 1; x < 4; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(sv_dragon_balance, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_SHINING:
    {
        int     value[2];
        int     type[2] = { BORG_ATTACK_LIGHT, BORG_ATTACK_DARK };
        int     biggest = 0;
        bool    tmp_simulate = borg_simulate;

        rad = 2;
        dam = 200;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_dragon(sv_dragon_shining, rad, dam, type[x], x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(sv_dragon_shining, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_POWER:
    rad = 2;
    dam = 300;
    return (borg_attack_aux_dragon(sv_dragon_power, rad, dam, BORG_ATTACK_MISSILE, -1));
    }

    /* Oops */
    return (0);
}


/*
 * Attack nearby monsters, in the best possible way, if any.
 *
 * We consider a variety of possible attacks, including physical attacks
 * on adjacent monsters, missile attacks on nearby monsters, spell/prayer
 * attacks on nearby monsters, and wand/rod attacks on nearby monsters.
 *
 * Basically, for each of the known "types" of attack, we "simulate" the
 * "optimal" result of using that attack, and then we "apply" the "type"
 * of attack which appears to have the "optimal" result.
 *
 * When calculating the "result" of using an attack, we only consider the
 * effect of the attack on visible, on-screen, known monsters, which are
 * within 16 grids of the player.  This prevents most "spurious" attacks,
 * but we can still be fooled by situations like creeping coins which die
 * while out of sight, leaving behind a pile of coins, which we then find
 * again, and attack with distance attacks, which have no effect.  Perhaps
 * we should "expect" certain results, and take note of failure to observe
 * those effects.  XXX XXX XXX
 *
 * See above for the "semantics" of each "type" of attack.
 */
bool borg_attack(bool boosted_bravery)
{
    int i, x, y;
    int a_y, a_x;

    int n, b_n = 0;
    int g, b_g = -1;
    bool adjacent_monster = false;

    borg_grid* ag;
    struct monster_race* r_ptr;

    /* Nobody around */
    if (!borg_kills_cnt) return (false);

    /* Set the attacking flag so that danger is boosted for monsters */
    /* we want to attack first. */
    borg_attacking = true;

    /* Reset list */
    borg_temp_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Ignore multiplying monsters and when fleeing from scaries*/
        if (goal_ignoring && !borg_skill[BI_ISAFRAID] &&
            (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY))) continue;

        /* Acquire location */
        a_x = kill->x;
        a_y = kill->y;

        /* Low level mages need to conserve the mana in town. These guys don't fight back */
        if (borg_class == CLASS_MAGE && borg_skill[BI_MAXCLEVEL] < 10 &&
            borg_skill[BI_CDEPTH] == 0 &&
            (strstr(r_ptr->name, "Farmer")
                /* strstr(r_ptr->name, "Blubbering") || */
                /* strstr(r_ptr->name, "Boil") || */
                /* strstr(r_ptr->name, "Village") || */
                /*strstr(r_ptr->name, "Pitiful") || */
                /* strstr(r_ptr->name, "Mangy") */)) continue;

        /* Check if there is a monster adjacent to me or he's close and fast. */
        if ((kill->speed > borg_skill[BI_SPEED] && borg_distance(c_y, c_x, a_y, a_x) <= 2) ||
            borg_distance(c_y, c_x, a_y, a_x) <= 1) adjacent_monster = true;

        /* no attacking most scaryguys, try to get off the level */
        if (scaryguy_on_level)
        {
            /* probably Grip or Fang. */
            if (strstr(r_ptr->name, "Grip") ||
                strstr(r_ptr->name, "Fang"))
            {
                /* Try to fight Grip and Fang. */
            }
            else if (borg_skill[BI_CDEPTH] <= 5 && borg_skill[BI_CDEPTH] != 0 &&
                (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY)))
            {
                /* Try to fight single worms and mice. */
            }
            else if (borg_t - borg_began >= 2000 || borg_time_town + (borg_t - borg_began) >= 3000)
            {
                /* Try to fight been there too long. */
            }
            else if (boosted_bravery ||
                borg_no_retreat >= 1 ||
                goal_recalling)
            {
                /* Try to fight if being Boosted or recall engaged. */
                borg_note("# Bored, or recalling and fighting a monster on Scaryguy Level.");
            }
            else if (borg_skill[BI_CDEPTH] * 4 <= borg_skill[BI_CLEVEL] &&
                borg_skill[BI_CLEVEL] > 10)
            {
                /* Try to fight anyway. */
                borg_note("# High clevel fighting monster on Scaryguy Level.");
            }
            else if (adjacent_monster)
            {
                /* Try to fight if there is a monster next to me */
                borg_note("# Adjacent to monster on Scaryguy Level.");
            }
            else
            {
                /* Flee from other scary guys */
                continue;
            }

        }

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW)) continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range) continue;

        /* Sometimes the borg can lose a monster index in the grid if there are lots of monsters
         * on screen.  If he does lose one, reinject the index here. */
        if (!ag->kill) borg_grids[kill->y][kill->x].kill = i;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n)
    {
        borg_attacking = false;
        return (false);
    }


    /* Simulate */
    borg_simulate = true;

    /* Analyze the possible attacks */
    for (g = 0; g < BF_MAX; g++)
    {

        /* Simulate */
        n = borg_attack_aux(g);

        /* Track "best" attack  <= */
        if (n <= b_n) continue;

        /* Track best */
        b_g = g;
        b_n = n;
    }

    /* Nothing good */
    if (b_n <= 0)
    {
        borg_attacking = false;
        return (false);
    }


    /* Note */
    borg_note(format("# Performing attack type %d with value %d.", b_g, b_n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_attack_aux(b_g);

    borg_attacking = false;

    /* Success */
    return (true);
}

/* Munchkin Attack - Magic
 *
 * The early mages have a very difficult time surviving until they level up some.
 * This routine will allow the mage to do some very limited attacking while he is
 * doing the munchking start (stair scumming for items).
 *
 * Basically, he will rest on stairs to recuperate mana, then use MM to attack some
 * easy to kill monsters.  If the monster gets too close, he will flee via the stairs.
 * He hope to be able to kill the monster in two shots from the MM.  A perfect scenario
 * would be a mold which does not move, then he could rest/shoot/rest.
 */

bool borg_munchkin_mage(void)
{

    int i, x, y;
    int a_y, a_x;

    int b_dam = -1, dam = 0;
    int b_n = -1;

    borg_grid* ag;

    /* Must be standing on a stair */
    if (borg_grids[c_y][c_x].feat != FEAT_MORE && borg_grids[c_y][c_x].feat != FEAT_LESS) return (false);

    /* Not if too dangerous */
    if ((borg_danger(c_y, c_x, 1, true, true) > avoidance * 7 / 10) ||
        borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3) return (false);
    if (borg_skill[BI_ISCONFUSED]) return (false);

    /* Nobody around */
    if (!borg_kills_cnt) return (false);

    /* Set the attacking flag so that danger is boosted for monsters */
    /* we want to attack first. */
    borg_attacking = true;

    /* Reset list */
    borg_temp_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Acquire location */
        a_x = kill->x;
        a_y = kill->y;

        /* Not in town.  This should not be reached, but just in case we add it */
        if (borg_skill[BI_CDEPTH] == 0) continue;

        /* Check if there is a monster adjacent to me or he's close and fast. */
        if ((kill->speed > borg_skill[BI_SPEED] && borg_distance(c_y, c_x, a_y, a_x) <= 2) ||
            borg_distance(c_y, c_x, a_y, a_x) <= 1) return (false);

        /* no attacking most scaryguys, try to get off the level */
        if (scaryguy_on_level) return (false);

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW)) continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range) continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n)
    {
        borg_attacking = false;
        return (false);
    }


    /* Simulate */
    borg_simulate = true;

    /* Simulated */
    for (i = 0; i < BF_MAX; i++)
    {
        /* Skip certain ones */
        if (i <= 1) continue;

        dam = borg_attack_aux(i);

        /* Track the best attack method */
        if (dam >= b_dam && dam > 0)
        {
            b_dam = dam;
            b_n = i;
        }
    }

    /* Nothing good */
    if (b_n < 0 || b_dam <= 0)
    {
        borg_attacking = false;
        return (false);
    }

    /* Note */
    borg_note(format("# Performing munchkin attack with value %d.", b_dam));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_attack_aux(b_n);

    borg_attacking = false;

    /* Success */
    return (true);
}

/* Munchkin Attack - Melee
 *
 * The early borgs have a very difficult time surviving until they level up some.
 * This routine will allow the borg to do some very limited attacking while he is
 * doing the munchking start (stair scumming for items).
 *
 * Basically, he will rest on stairs to recuperate HP, then use melee to attack some
 * easy to kill adjacent monsters.
 */

bool borg_munchkin_melee(void)
{

    int i, x, y;

    int n = 0;

    borg_grid* ag;

    /* No Mages for now */
    if ((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER)) return (false);

    /* Must be standing on a stair */
    if (borg_grids[c_y][c_x].feat != FEAT_MORE && borg_grids[c_y][c_x].feat != FEAT_LESS) return (false);

    /* Nobody around */
    if (!borg_kills_cnt) return (false);

    /* Not if too dangerous */
    if ((borg_danger(c_y, c_x, 1, true, true) > avoidance * 7 / 10) ||
        borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 3) return (false);
    if (borg_skill[BI_ISCONFUSED]) return (false);

    /* Set the attacking flag so that danger is boosted for monsters */
    /* we want to attack first. */
    borg_attacking = true;

    /* Reset list */
    borg_temp_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Not in town.  This should not be reached, but just in case we add it */
        if (borg_skill[BI_CDEPTH] == 0) continue;

        /* no attacking most scaryguys, try to get off the level */
        if (scaryguy_on_level) return (false);

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW)) continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) != 1) continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n)
    {
        borg_attacking = false;
        return (false);
    }


    /* Simulate */
    borg_simulate = true;

    /* Simulated */
    n = borg_attack_aux(BF_THRUST);

    /* Nothing good */
    if (n <= 0)
    {
        borg_attacking = false;
        return (false);
    }

    /* Note */
    borg_note(format("# Performing munchkin attack with value %d.", n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_attack_aux(BF_THRUST);

    borg_attacking = false;

    /* Success */
    return (true);
}

/* Log the pathway and feature of the spell pathway
 * Useful for debugging beams and Tport Other spell
 */
static void borg_log_spellpath(bool beam)
{
    int n_x, n_y, x, y;

    int dist = 0;

    borg_grid* ag;
    borg_kill* kill;

    y = borg_target_y;
    x = borg_target_x;
    n_x = c_x;
    n_y = c_y;

    while (1)
    {
        ag = &borg_grids[n_y][n_x];
        kill = &borg_kills[ag->kill];

        /* Note the Pathway */
        if (!borg_cave_floor_grid(ag))
        {
            borg_note(format("# Logging Spell pathway (%d,%d): Wall grid.", n_y, n_x));
            break;
        }
        else if (ag->kill)
        {
            borg_note(format("# Logging Spell pathway (%d,%d): %s, danger %d",
                n_y, n_x, (r_info[kill->r_idx].name),
                borg_danger_aux(c_y, c_x, 1, ag->kill, true, false)));
        }
        else if (n_y == c_y && n_x == c_x)
        {
            borg_note(format("# Logging Spell pathway (%d,%d): My grid.",
                n_y, n_x));
        }
        else
        {
            borg_note(format("# Logging Spell pathway (%d,%d).", n_y, n_x));
        }

        /* Stop loop if we reach our target if using bolt */
        if (n_x == x && n_y == y) break;

        /* Safegaurd not to loop */
        dist++;
        if (dist >= z_info->max_range) break;

        /* Calculate the new location */
        mmove2(&n_y, &n_x, c_y, c_x, y, x);
    }
}



/*
 *
 * There are several types of setup moves:
 *
 *   Temporary speed
 *   Protect From Evil
 *   Bless\Prayer
 *   Berserk\Heroism
 *   Temp Resist (either all or just cold/fire?)
 *   Shield
 *   Teleport away
 *   Glyph of Warding
 *   See inviso
 *
 * * and many others
 */
enum
{
    BD_BLESS,
    BD_SPEED,
    BD_GRIM_PURPOSE,
    BD_RESIST_FECAP,
    BD_RESIST_F,
    BD_RESIST_C, /* 5*/
    BD_RESIST_A,
    BD_RESIST_P,
    BD_PROT_FROM_EVIL,
    BD_SHIELD,
    BD_TELE_AWAY, /* 10 */
    BD_HERO,
    BD_BERSERK,
    BD_SMITE_EVIL,
    BD_REGEN,
    BD_GLYPH,
    BD_CREATE_DOOR,
    BD_MASS_GENOCIDE, /* 15 */
    BD_GENOCIDE,
    BD_GENOCIDE_NASTIES,
    BD_EARTHQUAKE,
    BD_DESTRUCTION,
    BD_TPORTLEVEL,  /* 20 */
    BD_BANISHMENT,  /* Priest spell */
    BD_DETECT_INVISO,
    BD_LIGHT_BEAM,
    BD_SHIFT_PANEL,
    BD_REST,
    BD_TELE_AWAY_MORGOTH,
    BD_BANISHMENT_MORGOTH,
    BD_LIGHT_MORGOTH,

    BD_MAX
};

/*
 * Bless/Prayer to prepare for battle
 */
static int borg_defend_aux_bless(int p1)
{
    int fail_allowed = 25;
    borg_grid* ag = &borg_grids[c_y][c_x];

    int i;

    bool borg_near_kill = false;

    /* already blessed */
    if (borg_bless)
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Dark */
    if (!(ag->info & BORG_GLOW) && borg_skill[BI_CURLITE] == 0) return (0);


    /* no spell */
    if (!borg_spell_okay_fail(BLESS, fail_allowed) &&
        -1 == borg_slot(TV_SCROLL, sv_scroll_blessing) &&
        -1 == borg_slot(TV_SCROLL, sv_scroll_holy_chant) &&
        -1 == borg_slot(TV_SCROLL, sv_scroll_holy_prayer))
        return (0);

    /* Check if a monster is close to me .
     */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 5) continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, kill->y, kill->x) > 3) continue;

        /* kill near me */
        borg_near_kill = true;
    }

    /* if we are in some danger but not much, go for a quick bless */
    if ((p1 > avoidance / 12 || borg_skill[BI_CLEVEL] <= 15) &&
        p1 > 0 && borg_near_kill && p1 < avoidance / 2)
    {
        /* Simulation */
        /* bless is a low priority */
        if (borg_simulate) return (1);

        borg_note("# Attempting to cast Bless");

        /* No resting to recoop mana */
        borg_no_rest_prep = 11000;

        /* do it! */
        if (borg_spell(BLESS) ||
            borg_read_scroll(sv_scroll_blessing) ||
            borg_read_scroll(sv_scroll_holy_chant) ||
            borg_read_scroll(sv_scroll_holy_prayer))
            return 1;
    }

    return (0);
}

/*
 * Speed to prepare for battle
 */
static int borg_defend_aux_speed(int p1)
{
    int p2 = 0;
    bool good_speed = false;
    bool speed_spell = false;
    bool speed_staff = false;
    bool speed_rod = false;
    int fail_allowed = 25;

    /* already fast */
    if (borg_speed)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    /* only cast defence spells if fail rate is not too high */
    if (borg_spell_okay_fail(HASTE_SELF, fail_allowed))
        speed_spell = true;

    /* staff must have charges */
    if (borg_equips_staff_fail(sv_staff_speed))
        speed_staff = true;

    /* rod can't be charging */
    if (borg_equips_rod(sv_rod_speed))
        speed_rod = true;

    /* Need some form */
    if (0 > borg_slot(TV_POTION, sv_potion_speed) &&
        !speed_staff &&
        !speed_rod &&
        !speed_spell &&
        !borg_equips_item(act_haste, true) &&
        !borg_equips_item(act_haste1, true) &&
        !borg_equips_item(act_haste2, true))
        return (0);

    /* if we have an infinite/large suppy of speed we can */
    /* be generious with our use */
    if (speed_rod || speed_spell || speed_staff ||
        borg_equips_item(act_haste, true) ||
        borg_equips_item(act_haste1, true) ||
        borg_equips_item(act_haste2, true))
        good_speed = true;

    /* pretend we are protected and look again */
    borg_speed = true;
    p2 = borg_danger(c_y, c_x, 1, true, false);
    borg_speed = false;

    /* if scaryguy around cast it. */
    if (scaryguy_on_level)
    {
        /* HACK pretend that it was scary and will be safer */
        p2 = p2 * 3 / 10;
    }

    /* if we are fighting a unique cast it. */
    if (good_speed && borg_fighting_unique)
    {
        /* HACK pretend that it was scary and will be safer */
        p2 = p2 * 7 / 10;
    }
    /* if we are fighting a unique and a summoner cast it. */
    if (borg_fighting_summoner && borg_fighting_unique)
    {
        /* HACK pretend that it was scary and will be safer */
        p2 = p2 * 7 / 10;
    }
    /* if the unique is Sauron cast it */
    if (borg_skill[BI_CDEPTH] == 99 && borg_fighting_unique >= 10)
    {
        p2 = p2 * 6 / 10;
    }

    /* if the unique is a rather nasty one. */
    if (borg_fighting_unique && 
        (streq(r_info[unique_on_level].name, "Bullroarer the Hobbit") ||
         streq(r_info[unique_on_level].name, "Mughash the Kobold Lord") ||
         streq(r_info[unique_on_level].name, "Wormtongue, Agent of Saruman") ||
         streq(r_info[unique_on_level].name, "Lagduf, the Snaga") ||
         streq(r_info[unique_on_level].name, "Brodda, the Easterling") ||
         streq(r_info[unique_on_level].name, "Orfax, Son of Boldor")))
    {
        p2 = p2 * 6 / 10;
    }

    /* if the unique is Morgoth cast it */
    if (borg_skill[BI_CDEPTH] == 100 && borg_fighting_unique >= 10)
    {
        p2 = p2 * 5 / 10;
    }

    /* Attempt to conserve Speed at end of game */
    if (borg_skill[BI_CDEPTH] >= 97 && !borg_fighting_unique && !good_speed) p2 = 9999;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (((p1 > p2) &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        (p1 > (avoidance / 5)) && good_speed) ||
        ((p1 > p2) &&
            p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 3)) &&
            (p1 > (avoidance / 7))))
    {

        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to cast Speed");

        /* No resting to recoop mana */
        borg_no_rest_prep = borg_skill[BI_CLEVEL] * 1000;

        /* do it! */
        if (borg_zap_rod(sv_rod_speed) ||
            borg_activate_item(act_haste) ||
            borg_activate_item(act_haste1) ||
            borg_activate_item(act_haste2) ||
            borg_use_staff(sv_staff_speed) ||
            borg_quaff_potion(sv_potion_speed))
            /* Value */
            return (p1 - p2);

        if (borg_spell_fail(HASTE_SELF, fail_allowed))
            return (p1 - p2);

    }
    /* default to can't do it. */
    return (0);
}

/* Grim Purpose */
static int borg_defend_aux_grim_purpose(int p1)
{
    int p2 = 0;
    int fail_allowed = 25;

    bool save_conf = borg_skill[BI_RCONF];
    bool save_fa = borg_skill[BI_FRACT];

    /* already protected */
    if (save_conf && save_fa)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(GRIM_PURPOSE, fail_allowed))
        return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    /* pretend we are protected and look again */
    borg_skill[BI_RCONF] = true;
    borg_skill[BI_FRACT] = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_skill[BI_RCONF] = save_conf;
    borg_skill[BI_FRACT] = save_fa;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {

        /* Simulation */
        if (borg_simulate) return (p1 - p2 + 2);

        borg_note("# Attempting to cast Grim Purpose");

        /* do it! */
        if (borg_spell(GRIM_PURPOSE))
            /* No resting to recoop mana */
            borg_no_rest_prep = 13000;

        /* Value */
        return (p1 - p2 + 2);
    }

    /* default to can't do it. */
    return (0);
}

/* all resists */
static int borg_defend_aux_resist_fecap(int p1)
{
    int p2 = 0;
    bool    save_fire = false,
            save_acid = false,
            save_poison = false,
            save_elec = false,
            save_cold = false;

    if (borg_skill[BI_TRFIRE] &&
        borg_skill[BI_TRACID] &&
        borg_skill[BI_TRPOIS] &&
        borg_skill[BI_TRELEC] &&
        borg_skill[BI_TRCOLD])
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    if (!borg_equips_item(act_resist_all, true) &&
        !borg_equips_item(act_rage_bless_resist, true))
        return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    /* pretend we are protected and look again */
    save_fire = borg_skill[BI_TRFIRE];
    save_elec = borg_skill[BI_TRELEC];
    save_cold = borg_skill[BI_TRCOLD];
    save_acid = borg_skill[BI_TRACID];
    save_poison = borg_skill[BI_TRPOIS];
    borg_skill[BI_TRFIRE] = true;
    borg_skill[BI_TRELEC] = true;
    borg_skill[BI_TRCOLD] = true;
    borg_skill[BI_TRACID] = true;
    borg_skill[BI_TRPOIS] = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_skill[BI_TRFIRE] = save_fire;
    borg_skill[BI_TRELEC] = save_elec;
    borg_skill[BI_TRCOLD] = save_cold;
    borg_skill[BI_TRACID] = save_acid;
    borg_skill[BI_TRPOIS] = save_poison;

    /* Hack -
     * If the borg is fighting a particular unique enhance the
     * benefit of the spell.
     */
    if (borg_fighting_unique &&
        (streq(r_info[unique_on_level].name, "The Tarrasque"))) 
        p2 = p2 * 8 / 10;

    /* Hack -
     * If borg is high enough level, he does not need to worry
     * about mana consumption.  Cast the good spell.
     */
    if (borg_skill[BI_CLEVEL] >= 45) p2 = p2 * 8 / 10;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {

        /* Simulation */
        if (borg_simulate) return (p1 - p2 + 2);

        borg_note("# Attempting to cast FECAP");

        /* do it! */
        if (borg_activate_item(act_resist_all) ||
            borg_activate_item(act_rage_bless_resist))

            /* No resting to recoop mana */
            borg_no_rest_prep = 21000;

        /* Value */
        return (p1 - p2 + 2);
    }

    /* default to can't do it. */
    return (0);
}

/* fire */
static int borg_defend_aux_resist_f(int p1)
{

    int p2 = 0;
    int fail_allowed = 25;
    bool    save_fire = false;

    save_fire = borg_skill[BI_TRFIRE];

    if (borg_skill[BI_TRFIRE])
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed) &&
        !borg_equips_item(act_resist_all, true) &&
        !borg_equips_item(act_rage_bless_resist, true) &&
        !borg_equips_ring(sv_ring_flames) &&
        -1 == borg_slot(TV_POTION, sv_potion_resist_heat))
        return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    /* pretend we are protected and look again */
    borg_skill[BI_TRFIRE] = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_skill[BI_TRFIRE] = save_fire;

    /* Hack -
     * If the borg is fighting a particular unique enhance the
     * benefit of the spell.
     */
    if (borg_fighting_unique &&
        (streq(r_info[unique_on_level].name, "The Tarrasque")))
        p2 = p2 * 8 / 10;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to cast RFire");
        /* do it! */
        if (borg_activate_ring(sv_ring_flames))
        {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }
        if (borg_activate_item(act_resist_all) ||
            borg_activate_item(act_rage_bless_resist) ||
            borg_spell_fail(RESISTANCE, fail_allowed) ||
            borg_quaff_potion(sv_potion_resist_heat))

            /* No resting to recoop mana */
            borg_no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return (0);
}

/* cold */
static int borg_defend_aux_resist_c(int p1)
{

    int p2 = 0;
    int fail_allowed = 25;
    bool    save_cold = false;

    if (borg_skill[BI_TRCOLD])
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed) &&
        !borg_equips_item(act_resist_all, true) &&
        !borg_equips_item(act_rage_bless_resist, true) &&
        !borg_equips_ring(sv_ring_ice) &&
        -1 == borg_slot(TV_POTION, sv_potion_resist_cold))
        return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    save_cold = borg_skill[BI_TRCOLD];
    /* pretend we are protected and look again */
    borg_skill[BI_TRCOLD] = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_skill[BI_TRCOLD] = save_cold;

    /* Hack -
     * If the borg is fighting a particular unique enhance the
     * benefit of the spell.
     */
    if (borg_fighting_unique &&
        (streq(r_info[unique_on_level].name, "The Tarrasque")))
        p2 = p2 * 8 / 10;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to cast RCold");

        /* do it! */
        if (borg_activate_ring(sv_ring_ice))
        {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }
        if (borg_activate_item(act_resist_all) ||
            borg_activate_item(act_rage_bless_resist) ||
            borg_spell_fail(RESISTANCE, fail_allowed) ||
            borg_quaff_potion(sv_potion_resist_cold))

            /* No resting to recoop mana */
            borg_no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return (0);
}

/* acid */
static int borg_defend_aux_resist_a(int p1)
{

    int p2 = 0;
    int fail_allowed = 25;
    bool    save_acid = false;

    if (borg_skill[BI_TRACID])
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed) &&
        !borg_equips_item(act_resist_all, true) &&
        !borg_equips_item(act_rage_bless_resist, true) &&
        !borg_equips_ring(sv_ring_acid))
        return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    save_acid = borg_skill[BI_TRACID];
    /* pretend we are protected and look again */
    borg_skill[BI_TRACID] = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_skill[BI_TRACID] = save_acid;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to cast RAcid");

        /* do it! */
        if (borg_spell(RESISTANCE))
        {
            return (p1 - p2);
        }

        if (borg_activate_ring(sv_ring_acid))
        {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }

        if (borg_activate_item(act_resist_all) ||
            borg_activate_item(act_rage_bless_resist))

            /* No resting to recoop mana */
            borg_no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }
    /* default to can't do it. */
    return (0);
}

/* poison */
static int borg_defend_aux_resist_p(int p1)
{
    int p2 = 0;
    int fail_allowed = 25;
    bool    save_poison = false;

    if (borg_skill[BI_TRPOIS])
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESIST_POISON, fail_allowed) &&
        !borg_equips_item(act_resist_all, true) &&
        !borg_equips_item(act_rage_bless_resist, true) &&
        !borg_spell_okay_fail(RESISTANCE, fail_allowed))
        return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    save_poison = borg_skill[BI_TRPOIS];
    /* pretend we are protected and look again */
    borg_skill[BI_TRPOIS] = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_skill[BI_TRPOIS] = save_poison;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to cast RPois");

        /* do it! */
        if (borg_spell_fail(RESIST_POISON, fail_allowed) ||
            borg_activate_item(act_resist_all) ||
            borg_activate_item(act_rage_bless_resist) ||
            borg_spell_fail(RESISTANCE, fail_allowed))

            /* No resting to recoop mana */
            borg_no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return (0);
}

static int borg_defend_aux_prot_evil(int p1)
{
    int p2 = 0;
    int fail_allowed = 25;
    bool pfe_spell = false;
    borg_grid* ag = &borg_grids[c_y][c_x];


    /* if already protected */
    if (borg_prot_from_evil)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 5;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (borg_spell_okay_fail(PROTECTION_FROM_EVIL, fail_allowed)) pfe_spell = true;

    if (0 <= borg_slot(TV_SCROLL, sv_scroll_protection_from_evil)) pfe_spell = true;

    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE])
        pfe_spell = false;

    if (!(ag->info & BORG_GLOW) && borg_skill[BI_CURLITE] == 0) pfe_spell = false;

    if (borg_equips_item(act_protevil, true)) pfe_spell = true;

    if (pfe_spell == false) return (0);

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as such. */
    p1 = borg_danger(c_y, c_x, 1, false, false);

    /* pretend we are protected and look again */
    borg_prot_from_evil = true;
    p2 = borg_danger(c_y, c_x, 1, false, false);
    borg_prot_from_evil = false;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */

    if ((p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7)) ||
        (borg_cfg[BORG_MONEY_SCUM_AMOUNT] >= 1 && borg_skill[BI_CDEPTH] == 0))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to cast PFE");

        /* do it! */
        if (borg_spell_fail(PROTECTION_FROM_EVIL, fail_allowed) ||
            borg_activate_item(act_protevil) ||
            borg_read_scroll(sv_scroll_protection_from_evil))

            /* No resting to recoop mana */
            borg_no_rest_prep = borg_skill[BI_CLEVEL] * 1000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return (0);
}

static int borg_defend_aux_shield(int p1)
{
    int p2 = 0;

    /* if already protected */
    if (borg_shield)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    if (borg_has[kv_mush_stoneskin] <= 0)
        return (0);

    /* pretend we are protected and look again */
    borg_shield = true;
    p2 = borg_danger(c_y, c_x, 1, true, false);
    borg_shield = false;

    /* slightly enhance the value if fighting a unique */
    if (borg_fighting_unique)  p2 = (p2 * 7 / 10);


    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        borg_note("# Attempting to eat a stone skin");

        /* do it! */
        if (borg_eat_food(TV_MUSHROOM, sv_mush_stoneskin))
        {
            /* No resting to recoop mana */
            borg_no_rest_prep = 2000;
            return (p1 - p2);
        }
    }


    /* default to can't do it. */
    return (0);
}

/*
 * Try to get rid of all of the non-uniques around so you can go at it
 * 'mano-e-mano' with the unique. Teleport Other.
 */
static int borg_defend_aux_tele_away(int p1)
{
    int p2 = p1;
    int fail_allowed = 50;
    bool  spell_ok = false;
    int i, x, y;

    borg_grid* ag;


    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /*
     * Only tport monster away if scared or getting low on mana
     */
    if (borg_fighting_unique)
    {
        if (p1 < avoidance * 7 / 10 && borg_skill[BI_CURSP] > 30 && borg_simulate)
            return (0);
    }
    else
    {
        if (p1 < avoidance * 5 / 10 && borg_skill[BI_CURSP] > 30 && borg_simulate)
            return (0);
    }

    /* No real Danger to speak of */
    if (p1 < avoidance * 4 / 10 && borg_simulate) return (0);

    spell_ok = false;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance * 3)
        fail_allowed -= 10;
    else
        /* scary */
        if (p1 > avoidance * 2)
            fail_allowed -= 5;
        else
            /* a little scary */
            if (p1 > (avoidance * 5) / 2)
                fail_allowed += 5;

    /* do I have the ability? */
    if (borg_spell_okay_fail(TELEPORT_OTHER, fail_allowed) ||
        borg_equips_item(act_tele_other, true) ||
        (-1 != borg_slot(TV_WAND, sv_wand_teleport_away) &&
            borg_items[borg_slot(TV_WAND, sv_wand_teleport_away)].pval))
        spell_ok = true;

    if (!spell_ok) return (0);

    /* No Teleport Other if surrounded */
    if (borg_surrounded() == true) return (0);

    /* Borg_temp_n temporarily stores several things.
     * Some of the borg_attack() sub-routines use these numbers,
     * which would have been filled in borg_attack().
     * Since this is a defence manuever which will move into
     * and borrow some of the borg_attack() subroutines, we need
     * to make sure that the borg_temp_n arrays are properly
     * filled.  Otherwise, the borg will attempt to consider
     * these grids which were left filled by some other routine.
     * Which was probably a flow routine which stored about 200
     * grids into the array.
     * Any change in inclusion/exclusion criteria for filling this
     * array in borg_attack() should be included here also.
     */
     /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate) return (0);

    /* Reset list */
    borg_temp_n = 0;
    borg_tp_other_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW)) continue;
        if ((ag->feat >= FEAT_RUBBLE) &&
            (ag->feat <= FEAT_PERM)) continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range) continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No targets for me. */
    if (!borg_temp_n && borg_simulate) return (0);

    /* choose, then target a bad guy.
     * Damage will be the danger to my grid which the monster creates.
     * We are targetting the single most dangerous monster.
     * p2 will be the original danger (p1) minus the danger from the most dangerous
     * monster eliminated.
     * ie:  if we are fighting only a single monster who is generating 500 danger and we
     * target him, then p2 _should_ end up 0, since p1 - his danger is 500-500.
     * If we are fighting two guys each creating 500 danger, then p2 will be 500, since
     * 1000-500 = 500.
     */
    p2 = p1 - borg_launch_bolt(-1, p1, BORG_ATTACK_AWAY_ALL, z_info->max_range, 0);


    /* check to see if I am left better off */
    if (borg_simulate)
    {
        /* Reset list */
        borg_temp_n = 0;
        borg_tp_other_n = 0;

        if (p1 > p2 &&
            p2 < avoidance / 2)
        {
            /* Simulation */
            return (p1 - p2);
        }
        else return (0);
    }

    /* Log the Path for Debug */
    borg_log_spellpath(true);

    /* Log additional info for debug */
    for (i = 0; i < borg_tp_other_n; i++)
    {
        borg_note(format("# T.O. %d, index %d (%d,%d)", borg_tp_other_n,
            borg_tp_other_index[i], borg_tp_other_y[i],
            borg_tp_other_x[i]));
    }

    /* Reset list */
    borg_temp_n = 0;
    borg_tp_other_n = 0;

    /* Cast the spell */
    if (borg_spell(TELEPORT_OTHER) ||
        borg_activate_item(act_tele_other) ||
        borg_aim_wand(sv_wand_teleport_away))
    {
        /* Use target */
        borg_keypress('5');

        /* Set our shooting flag */
        successful_target = -1;

        /* Value */
        return (p2);
    }

    return (0);
}

/*
 * Hero to prepare for battle, +12 tohit.
 */
static int borg_defend_aux_hero(int p1)
{
    int fail_allowed = 15;

    /* already hero */
    if (borg_hero)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    if (!borg_spell_okay_fail(HEROISM, fail_allowed) &&
        -1 == borg_slot(TV_POTION, sv_potion_heroism))
        return (0);

    /* if we are in some danger but not much, go for a quick bless */
    if ((p1 > avoidance * 1 / 10 && p1 < avoidance * 5 / 10) ||
        (borg_fighting_unique && p1 < avoidance * 7 / 10))
    {
        /* Simulation */
        /* hero is a low priority */
        if (borg_simulate) return (1);

        borg_note("# Attempting to cast Hero");

        /* do it! */
        if (borg_spell(HEROISM) ||
            borg_quaff_potion(sv_potion_heroism))
        {
            /* No resting to recoop mana */
            borg_no_rest_prep = 10000;
            return 1;
        }
    }

    return (0);
}

/*
 * Rapid Regen to prepare for battle
 */
static int borg_defend_aux_regen(int p1)
{
    int fail_allowed = 15;

    /* already regenerating */
    if (borg_regen)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* don't bother if not much to regenerate */
    if (borg_skill[BI_MAXHP] < 100) return (0);

    if (!borg_spell_okay_fail(RAPID_REGENERATION, fail_allowed))
        return (0);

    /* if we are in some danger but not much, go for a quick bless */
    if ((p1 > avoidance * 1 / 10 && p1 < avoidance * 5 / 10) ||
        (borg_fighting_unique && p1 < avoidance * 7 / 10))
    {
        /* Simulation */
        /* regen is a low priority */
        if (borg_simulate) return (1);

        /* do it! */
        if (borg_spell(RAPID_REGENERATION))
        {
            /* No resting to recoop mana */
            borg_no_rest_prep = 10000;
            return 1;
        }
    }

    return (0);
}

/*
 * Berserk to prepare for battle, +24 tohit, -10 AC
 */
static int borg_defend_aux_berserk(int p1)
{
    int fail_allowed = 15;

    /* already berserk */
    if (borg_berserk)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    if (!borg_spell_okay_fail(BERSERK_STRENGTH, fail_allowed) &&
        -1 == borg_slot(TV_POTION, sv_potion_berserk) &&
        !borg_equips_item(act_berserker, true) &&
        !borg_equips_item(act_rage_bless_resist, true) &&
        !borg_equips_item(act_shero, true))
        return (0);

    /* if we are in some danger but not much, go for a quick bless */
    if ((p1 > avoidance * 1 / 10 && p1 < avoidance * 5 / 10) ||
        (borg_fighting_unique && p1 < avoidance * 7 / 10))
    {
        /* Simulation */
        /* berserk is a low priority */
        if (borg_simulate) return (5);

        /* do it! */
        if (borg_spell(BERSERK_STRENGTH) ||
            borg_activate_item(act_berserker) ||
            borg_activate_item(act_rage_bless_resist) ||
            borg_activate_item(act_shero) ||
            borg_quaff_potion(sv_potion_berserk))
            return (5);
    }

    return (0);
}
/*
 * Smite Evil to prepare for battle
 */
static int borg_defend_aux_smite_evil(int p1)
{
    int fail_allowed = 15;

    /* already smiting evil */
    if (borg_smite_evil || borg_skill[BI_WS_EVIL])
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    if (!borg_spell_okay_fail(SMITE_EVIL, fail_allowed))
        return (0);

    // !FIX !TODO !AJG we should probably figure out if we are about to fight something evil.

        /* if we are in some danger but not much, go for a quick bless */
    if ((p1 > avoidance * 1 / 10 && p1 < avoidance * 5 / 10) ||
        (borg_fighting_unique && p1 < avoidance * 7 / 10))
    {
        /* Simulation */
        /* smite evil is a low priority */
        if (borg_simulate) return (5);

        /* do it! */
        if (borg_spell(SMITE_EVIL))
            return (5);
    }

    return (0);
}

/* Glyph of Warding and Rune of Protection */
static int borg_defend_aux_glyph(int p1)
{
    int p2 = 0, i;
    int fail_allowed = 25;
    bool glyph_spell = false;

    borg_grid* ag = &borg_grids[c_y][c_x];

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* He should not cast it while on an object.
     * I have addressed this inadequately in borg9.c when dealing with
     * messages.  The message "the object resists" will delete the glyph
     * from the array.  Then I set a broken door on that spot, the borg ignores
     * broken doors, so he won't loop.
     */

    if ((ag->take) ||
        (ag->trap) ||
        (ag->feat == FEAT_LESS) ||
        (ag->feat == FEAT_MORE) ||
        (ag->feat == FEAT_OPEN) ||
        (ag->feat == FEAT_BROKEN))
    {
        return (0);
    }

    /* Morgoth breaks these in one try so its a waste of mana against him */
    if (borg_fighting_unique >= 10) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 5;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 20;

    if (borg_spell_okay_fail(GLYPH_OF_WARDING, fail_allowed)) glyph_spell = true;

    if (0 <= borg_slot(TV_SCROLL, sv_scroll_rune_of_protection)) glyph_spell = true;

    if ((borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE]) && glyph_spell)
        glyph_spell = false;

    if (!(ag->info & BORG_GLOW) && borg_skill[BI_CURLITE] == 0) glyph_spell = false;

    if (!glyph_spell) return (0);

    /* pretend we are protected and look again */
    borg_on_glyph = true;
    p2 = borg_danger(c_y, c_x, 1, true, false);
    borg_on_glyph = false;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        /* do it! */
        if (borg_spell_fail(GLYPH_OF_WARDING, fail_allowed) ||
            borg_read_scroll(sv_scroll_rune_of_protection))
        {
            /* Check for an existing glyph */
            for (i = 0; i < track_glyph.num; i++)
            {
                /* Stop if we already new about this glyph */
                if ((track_glyph.x[i] == c_x) && (track_glyph.y[i] == c_y)) return (p1 - p2);
            }

            /* Track the newly discovered glyph */
            if (track_glyph.num < track_glyph.size)
            {
                borg_note("# Noting the creation of a glyph.");
                track_glyph.x[track_glyph.num] = c_x;
                track_glyph.y[track_glyph.num] = c_y;
                track_glyph.num++;
            }
            return (p1 - p2);
        }

    }

    /* default to can't do it. */
    return (0);
}

/* Create Door */
static int borg_defend_aux_create_door(int p1)
{
    int p2 = 0;
    int fail_allowed = 30;
    int door_bad = 0;
    int door_x = 0, door_y = 0,
        x = 0, y = 0;

    borg_grid* ag;


    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* any summoners near?*/
    if (!borg_fighting_summoner) return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 5;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 20;

    if (!borg_spell_okay_fail(DOOR_CREATION, fail_allowed))
        return (0);

    /* Do not cast if surounded by doors or something */
    /* Get grid */
    for (door_x = -1; door_x <= 1; door_x++)
    {
        for (door_y = -1; door_y <= 1; door_y++)
        {
            /* Acquire location */
            x = door_x + c_x;
            y = door_y + c_y;

            ag = &borg_grids[y][x];

            /* track spaces already protected */
            if ((ag->glyph) || ag->kill ||
                ((ag->feat == FEAT_GRANITE) || (ag->feat == FEAT_PERM) || (ag->feat == FEAT_CLOSED)))
            {
                door_bad++;
            }

            /* track spaces that cannot be protected */
            if ((ag->take) ||
                (ag->trap) ||
                (ag->feat == FEAT_LESS) ||
                (ag->feat == FEAT_MORE) ||
                (ag->feat == FEAT_OPEN) ||
                (ag->feat == FEAT_BROKEN) ||
                (ag->kill))
            {
                door_bad++;
            }
        }
    }


    /* Track it */
    /* lets make sure that we going to be benifited */
    if (door_bad >= 6)
    {
        /* not really worth it.  Only 2 spaces protected */
        return (0);
    }

    /* pretend we are protected and look again */
    borg_create_door = true;
    p2 = borg_danger(c_y, c_x, 1, true, false);
    borg_create_door = false;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 7))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        /* do it! */
        if (borg_spell_fail(DOOR_CREATION, fail_allowed))
        {
            /* Set the breeder flag to keep doors closed. Avoid summons */
            breeder_level = true;

            /* Must make a new Sea too */
            borg_needs_new_sea = true;

            /* Value */
            return (p1 - p2);
        }
    }

    /* default to can't do it. */
    return (0);
}



/* This will simulate and cast the mass genocide spell.
 */
static int borg_defend_aux_mass_genocide(int p1)
{
    int hit = 0, i = 0, p2;
    int b_p = 0, p;

    borg_kill* kill;
    struct monster_race* r_ptr;

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* see if prayer is legal */
    if (!borg_spell_okay_fail(MASS_BANISHMENT, 40) &&
        !borg_equips_item(act_banishment, true) &&
        (borg_skill[BI_AMASSBAN] == 0))/* Mass Banishment scroll */
        return (0);

    /* See if he is in real danger */
    if (p1 < avoidance * 12 / 10 && borg_simulate)
        return (0);

    /* Find a monster and calculate its danger */
    for (i = 1; i < borg_kills_nxt; i++)
    {

        /* Monster */
        kill = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Check the distance */
        if (borg_distance(c_y, c_x, kill->y, kill->x) > 20) continue;

        /* we try not to genocide uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

        /* Calculate danger */
        p = borg_danger_aux(c_y, c_x, 1, i, true, true);

        /* store the danger for this type of monster */
        b_p = b_p + p;
        hit = hit + 3;
    }

    /* normalize the value */
    p2 = (p1 - b_p);
    if (p2 < 0) p2 = 0;

    /* if strain (plus a pad incase we did not know about some monsters)
     * is greater than hp, don't cast it
     */
    if ((hit * 12 / 10) >= borg_skill[BI_CURHP]) return (0);

    /* Penalize the strain from casting the spell */
    p2 = p2 + hit;

    /* Be more likely to use this if fighting Morgoth */
    if (borg_fighting_unique >= 10 && (hit / 3 > 8))
    {
        p2 = p2 * 6 / 10;
    }

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? (avoidance * 2 / 3) : (avoidance / 2)))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

        /* Cast the spell */
        if (borg_read_scroll(sv_scroll_mass_banishment) ||
            borg_activate_item(act_banishment) ||
            borg_spell(MASS_BANISHMENT))
        {

            /* Remove monsters from the borg_kill */
            for (i = 1; i < borg_kills_nxt; i++)
            {
                borg_kill* tmp_kill;
                struct monster_race* tmp_r_ptr;

                /* Monster */
                tmp_kill = &borg_kills[i];
                tmp_r_ptr = &r_info[tmp_kill->r_idx];

                /* Cant kill uniques like this */
                if (rf_has(tmp_r_ptr->flags, RF_UNIQUE)) continue;

                /* remove this monster */
                borg_delete_kill(i);
            }

            /* Value */
            return (p1 - p2);
        }
    }
    /* Not worth it */
    return (0);

}
/* This will simulate and cast the genocide spell.
 * There are two seperate functions happening here.
 * 1. will genocide the race which is immediately threatening the borg.
 * 2. will genocide the race which is most dangerous on the level.  Though it may not be
 *    threatening the borg right now.  It was considered to nuke the escorts of a unique.
 *    But it could also be used to nuke a race if it becomes too dangerous, for example
 *    a summoner called up 15-20 hounds, and they must be dealt with.
 * The first option may be called at any time.  While the 2nd option is only called when the
 * borg is in relatively good health.
 */
static int borg_defend_aux_genocide(int p1)
{
    int i, p, u, b_i = 0;
    int p2 = 0;
    int threat = 0;
    int max = 1;

    int b_p[256];
    int b_num[256];
    int b_threat[256];
    int b_threat_num[256];

    int total_danger_to_me = 0;

    char tmp_genocide_target = (char)0;
    unsigned char b_threat_id = (char)0;

    bool genocide_spell = false;
    int fail_allowed = 25;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Normalize the p1 value.  It contains danger added from
     * regional fear and monster fear.  Which wont be counted
     * in the post-genocide checks
     */
    if (borg_fear_region[c_y / 11][c_x / 11]) p1 -= borg_fear_region[c_y / 11][c_x / 11];
    if (borg_fear_monsters[c_y][c_x]) p1 -= borg_fear_monsters[c_y][c_x];


    /* Make sure I have the spell */
    if (borg_spell_okay_fail(BANISHMENT, fail_allowed) ||
        borg_equips_item(act_banishment, true) ||
        borg_equips_staff_fail(sv_staff_banishment) ||
        (-1 != borg_slot(TV_SCROLL, sv_scroll_banishment)))
    {
        genocide_spell = true;
    }

    if (genocide_spell == false) return (0);


    /* Don't try it if really weak */
    if (borg_skill[BI_CURHP] <= 75) return (0);

    /* two methods to calculate the threat:
     *1. cycle each character of monsters on screen
     *   collect collective threat of each char
     *2  select race of most dangerous guy, and choose him.
     * Method 2 is cheaper and faster.
     *
     * The borg uses method #1
     */

     /* Clear previous dangers */
    for (i = 0; i < 256; i++)
    {
        b_p[i] = 0;
        b_num[i] = 0;
        b_threat[i] = 0;
        b_threat_num[i] = 0;
    }

    /* Find a monster and calculate its danger */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;
        struct monster_race* r_ptr;

        /* Monster */
        kill = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Our char of the monster */
        u = r_ptr->d_char;

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* we try not to genocide uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

        /* Calculate danger */
        /* Danger to me by this monster */
        p = borg_danger_aux(c_y, c_x, 1, i, true, true);

        /* Danger of this monster to his own grid */
        threat = borg_danger_aux(kill->y, kill->x, 1, i, true, true);

        /* store the danger for this type of monster */
        b_p[u] = b_p[u] + p; /* Danger to me */
        total_danger_to_me += p;
        b_threat[u] = b_threat[u] + threat; /* Danger to monsters grid */

        /* Store the number of this type of monster */
        b_num[u]++;
        b_threat_num[u]++;
    }

    /* Now, see which race contributes the most danger
     * both to me and danger on the level
     */

    for (i = 0; i < 256; i++)
    {

        /* skip the empty ones */
        if (b_num[i] == 0 && b_threat_num[i] == 0) continue;

        /* for the race threatening me right now */
        if (b_p[i] > max)
        {
            /* track the race */
            max = b_p[i];
            b_i = i;

            /* note the danger with this race gone.  Note that the borg does max his danger
             * at 2000 points.  It could be much, much higher at depth 99 or so.
             * What the borg should do is recalculate the danger without considering this monster
             * instead of this hack which does not yeild the true danger.
             */
            p2 = total_danger_to_me - b_p[b_i];
        }

        /* for this race on the whole level */
        if (b_threat[i] > max)
        {
            /* track the race */
            max = b_threat[i];
            b_threat_id = i;
        }

        /* Leave an interesting note for debugging */
        if (!borg_simulate) borg_note(format("# Race '%c' is a threat with total danger %d from %d individuals.", i, b_threat[i], b_threat_num[i]));

    }

    /* This will track and decide if it is worth genociding this dangerous race for the level */
    if (b_threat_id)
    {
        /* Not if I am weak (should have 400 HP really in case of a Pit) */
        if (borg_skill[BI_CURHP] < 375) b_threat_id = 0;

        /* The threat must be real */
        if (b_threat[b_threat_id] < borg_skill[BI_MAXHP] * 3) b_threat_id = 0;

        /* Too painful to cast it (padded to be safe incase of unknown monsters) */
        if ((b_num[b_threat_id] * 4) * 12 / 10 >= borg_skill[BI_CURHP]) b_threat_id = 0;

        /* Loads of monsters might be a pit, in which case, try not to nuke them */
        if (b_num[b_threat_id] >= 75) b_threat_id = 0;

        /* Do not perform in Danger */
        if (p1 > avoidance / 5) b_threat_id = 0;

        /* report the danger and most dangerous race */
        if (b_threat_id)
        {
            borg_note(format("# Race '%c' is a real threat with total danger %d from %d individuals.", b_threat_id, b_threat[b_threat_id], b_threat_num[b_threat_id]));
        }

        /* Genociding this race would reduce the danger of the level */
        tmp_genocide_target = b_threat_id;

    }

    /* Consider the immediate threat genocide */
    if (b_i)
    {
        /* Too painful to cast it (padded to be safe incase of unknown monsters) */
        if ((b_num[b_i] * 4) * 12 / 10 >= borg_skill[BI_CURHP]) b_i = 0;

        /* See if he is in real danger, generally,
         * or deeper in the dungeon, conservatively,
         */
        if (p1 < avoidance * 7 / 10 ||
            (borg_skill[BI_CDEPTH] > 75 && p1 < avoidance * 6 / 10)) b_i = 0;

        /* Did this help improve my situation? */
        if (p2 <= (avoidance / 2)) b_i = 0;

        /* Genociding this race would help me immediately */
        tmp_genocide_target = b_i;

    }

    /* Complete the genocide routine */
    if (tmp_genocide_target)
    {
        if (borg_simulate)
        {
            /* Simulation for immediate threat */
            if (b_i) return (p1 - p2);

            /* Simulation for immediate threat */
            if (b_threat_id) return (b_threat[b_threat_id]);
        }

        if (b_i) borg_note(format("# Banishing race '%c' (qty:%d).  Danger after spell:%d", tmp_genocide_target, b_num[b_i], p2));
        if (b_threat_id) borg_note(format("# Banishing race '%c' (qty:%d).  Danger from them:%d", tmp_genocide_target, b_threat_num[b_threat_id], b_threat[b_threat_id]));

        /* do it! ---use scrolls first since they clutter inventory */
        if (borg_read_scroll(sv_scroll_banishment) ||
            borg_spell(BANISHMENT) ||
            borg_activate_item(act_banishment) ||
            borg_use_staff(sv_staff_banishment))
        {
            /* and the winner is.....*/
            borg_keypress((tmp_genocide_target));
        }

        /* Remove this race from the borg_kill */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill* kill;
            struct monster_race* r_ptr;

            /* Monster */
            kill = &borg_kills[i];
            r_ptr = &r_info[kill->r_idx];

            /* Our char of the monster */
            if (r_ptr->d_char != tmp_genocide_target) continue;

            /* we do not genocide uniques */
            if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

            /* remove this monster */
            borg_delete_kill(i);
        }

        return (p1 - p2);

    }
    /* default to can't do it. */
    return (0);
}

/* This will cast the genocide spell on Hounds and other
 * really nasty guys like Angels, Demons, Dragons and Liches
 * at the beginning of each level or when they get too numerous.
 * The acceptable numbers are defined in borg_nasties_limit[]
 * The definition for the list is in borg1.c
 * borg_nasties[7] = "ZAVULWD"
 *
 */
static int borg_defend_aux_genocide_nasties(int p1)
{
    int i = 0;
    int b_i = -1;

    bool genocide_spell = false;

    /* Not if I am weak */
    if (borg_skill[BI_CURHP] < (borg_skill[BI_MAXHP] * 7 / 10) ||
        borg_skill[BI_CURHP] < 250) return (0);

    /* only do it when Hounds start to show up, */
    if (borg_skill[BI_CDEPTH] < 25) return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Do not perform in Danger */
    if (p1 > avoidance / 4)
        return (0);

    if (borg_spell_okay_fail(BANISHMENT, 35) ||
        borg_equips_item(act_banishment, true) ||
        borg_equips_staff_fail(sv_staff_banishment))
    {
        genocide_spell = true;
    }

    if (genocide_spell == false) return (0);

    /* Find the numerous nasty in order of nastiness */
    for (i = 0; i < borg_nasties_num; i++)
    {
        if (borg_nasties_count[i] >= borg_nasties_limit[i]) b_i = i;
    }

    /* Nothing good to Genocide */
    if (b_i == -1) return (0);

    if (borg_simulate) return (10);

    /* Note it */
    borg_note(format("# Banishing nasties '%c' (qty:%d).", borg_nasties[b_i],
        borg_nasties_count[b_i]));

    /* Execute -- Nice pun*/
    if (borg_activate_item(act_banishment) ||
        borg_use_staff(sv_staff_banishment) ||
        borg_spell(BANISHMENT))
    {
        /* and the winner is.....*/
        borg_keypress(borg_nasties[b_i]);

        /* set the count to not do it again */
        borg_nasties_count[b_i] = 0;

        /* Remove this race from the borg_kill */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill* kill;
            struct monster_race* r_ptr;

            /* Monster */
            kill = &borg_kills[i];
            r_ptr = &r_info[kill->r_idx];

            /* Our char of the monster */
            if (r_ptr->d_char != borg_nasties[b_i]) continue;

            /* remove this monster */
            borg_delete_kill(i);
        }

        return (10);
    }

    /* default to can't do it. */
    return (0);
}

/* Earthquake, priest and mage spells.
 */
static int borg_defend_aux_earthquake(int p1)
{
    int p2 = 9999;
    int i;
    int threat_count = 0;

    borg_kill* kill;


    /* Cast the spell */
    if (!borg_simulate &&
        (borg_spell(TREMOR) ||
            borg_spell(QUAKE) ||
            borg_spell(GRONDS_BLOW)))
    {
        /* Must make a new Sea too */
        borg_needs_new_sea = true;
        return (p2);
    }

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Can I cast the spell? */
    if (!borg_spell_okay_fail(TREMOR, 35) &&
        !borg_spell_okay_fail(QUAKE, 35) &&
        !borg_spell_okay_fail(GRONDS_BLOW, 35))
        return (0);

    /* See if he is in real danger or fighting summoner*/
    if (p1 < avoidance * 6 / 10 && !borg_fighting_summoner)
        return (0);

    /* Several monsters can see the borg and they have ranged attacks */
    for (i = 0; i < borg_kills_nxt; i++)
    {
        kill = &borg_kills[i];

        /* Look for threats */
        if (borg_los(c_y, c_x, kill->y, kill->x) &&
            kill->ranged_attack &&
            borg_distance(kill->y, kill->x, c_y, c_x) >= 2)
        {
            /* They can hit me */
            threat_count++;
        }
    }

    /* Real danger? */
    if (threat_count >= 4 && p1 > avoidance * 7 / 10) p2 = p1 / 3;
    if (threat_count == 3 && p1 > avoidance * 7 / 10) p2 = p1 * 6 / 10;

    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)) &&
        p1 > (avoidance / 5))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);

    }
    return (0);
}

/* Word of Destruction, priest and mage spells.  Death is right around the
 *  corner, so kill everything.
 */
static int borg_defend_aux_destruction(int p1)
{
    int p2 = 0;
    int d = 0;
    bool spell = false;
    bool real_danger = false;

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Cast the spell */
    if (!borg_simulate)
    {
        if (borg_spell(WORD_OF_DESTRUCTION) ||
            borg_use_staff(sv_staff_destruction))
        {
            /* Must make a new Sea too */
            borg_needs_new_sea = true;

            /* borg9.c will check for the success of the spell and remove the danger from the grids. */

        }

        return (500);
    }

    /* Not if in a sea of runes */
    if (borg_morgoth_position) return (0);

    /* See if he is in real danger */
    if (p1 > avoidance) real_danger = true;
    if (p1 > avoidance * 8 / 10 && borg_skill[BI_CDEPTH] >= 90 &&
        borg_skill[BI_CURHP] <= 300)
        real_danger = true;

    if (real_danger == false) return (0);

    /* Borg_defend() is called before borg_escape().  He may have some
     * easy ways to escape (teleport scroll) but he may attempt this spell
     * of Destruction instead of using the scrolls.
     * Note that there will be some times when it is better for
     * the borg to use Destruction instead of Teleport;  too
     * often he will die out-of-the-fryingpan-into-the-fire.
     * So we have him to a quick check on safe landing zones.
     */

     /* Examine landing zones from teleport scrolls instead of WoD */
    if ((borg_skill[BI_ATELEPORT] || borg_skill[BI_ATELEPORTLVL]) &&
        !borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCONFUSED] &&
        borg_fighting_unique <= 4 && borg_skill[BI_CURHP] >= 275)
    {
        if (borg_caution_teleport(75, 2)) return (0);
    }

    /* Examine Landing zones from teleport staff instead of WoD */
    if (borg_skill[BI_AESCAPE] >= 2 && borg_skill[BI_CURHP] >= 275)
    {
        if (borg_caution_teleport(75, 2)) return (0);
    }

    /* capable of casting the spell */
    if (borg_spell_okay_fail(WORD_OF_DESTRUCTION, 55) ||
        borg_equips_staff_fail(sv_staff_destruction))
        spell = true;

    /* Special check for super danger--no fail check */
    if ((p1 > (avoidance * 4) ||
        (p1 > avoidance && borg_skill[BI_CURHP] <= 150)) &&
        borg_equips_staff_fail(sv_staff_destruction))
        spell = true;

    if (spell == false) return (0);

    /* What effect is there? */
    p2 = 0;

    /* value is d */
    d = (p1 - p2);

    /* Try not to cast this against uniques */
    if (borg_fighting_unique <= 2 && p1 < avoidance * 2) d = 0;
    if (borg_fighting_unique >= 10) d = 0;

    /* Simulation */
    if (borg_simulate) return (d);

    return (0);
}

/* Teleport Level, priest and mage spells.  Death is right around the
 *  corner, Get off the level now.
 */
static int borg_defend_aux_teleportlevel(int p1)
{
    /* Cast the spell */
    if (!borg_simulate)
    {
        if (borg_spell(TELEPORT_LEVEL))
        {
            /* Must make a new Sea too */
            borg_needs_new_sea = true;
            return (500);
        }
    }

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* See if he is in real danger */
    if (p1 < avoidance * 2)
        return (0);

    /* Borg_defend() is called before borg_escape().  He may have some
     * easy ways to escape (teleport scroll) but he may attempt this spell
     * of this spell instead of using the scrolls.
     * Note that there will be some times when it is better for
     * the borg to use this instead of Teleport;  too
     * often he will die out-of-the-fryingpan-into-the-fire.
     * So we have him to a quick check on safe landing zones.
     */

     /* Use teleport scrolls instead if safe to land */
    if ((borg_skill[BI_ATELEPORT] || borg_skill[BI_ATELEPORTLVL]) &&
        !borg_skill[BI_ISBLIND] && !borg_skill[BI_ISCONFUSED])
    {
        if (borg_caution_teleport(65, 2)) return (0);
    }

    /* Use teleport staff instead if safe to land */
    if (borg_skill[BI_AESCAPE] >= 2)
    {
        if (borg_caution_teleport(65, 2)) return (0);
    }

    /* capable of casting the spell */
    if (!borg_spell_okay_fail(TELEPORT_LEVEL, 55))
        return (0);

    /* Try not to cast this against special uniques */
    if (morgoth_on_level || (borg_fighting_unique >= 1 && borg_as_position)) return (0);

    /* Simulation */
    if (borg_simulate) return (p1);

    return (0);
}

/* Remove Evil guys within LOS.  The Priest Spell */
static int borg_defend_aux_banishment(int p1)
{
    int p2 = 0;
    int fail_allowed = 15;
    int i;
    int banished_monsters = 0;
    bool using_artifact;

    borg_grid* ag;

    /* Only tell away if scared */
    if (p1 < avoidance * 1 / 10)
        return (0);

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance * 4)
        fail_allowed -= 10;

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    using_artifact = borg_equips_item(act_loskill, true) && borg_has[BI_CURHP] > 100;

    if (!using_artifact &&
        !borg_spell_okay_fail(BANISH_EVIL, fail_allowed))
        return (0);

    /* reset initial danger */
    p1 = 1;

    /* Two passes to determine exact danger */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        ag = &borg_grids[kill->y][kill->x];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Check the LOS */
        if (!borg_projectable(c_y, c_x, kill->y, kill->x)) continue;

        /* Calculate danger of who is left over */
        p1 += borg_danger_aux(c_y, c_x, 1, i, true, true);
    }

    /* Set P2 to be P1 and subtract the danger from each monster
     * which will be booted.  Non booted monsters wont decrement
     * the p2
     */
    p2 = p1;

    /* Pass two -- Find a monster and calculate its danger */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;
        struct monster_race* r_ptr;

        /* Monster */
        kill = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        ag = &borg_grids[kill->y][kill->x];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Check the LOS */
        if (!borg_projectable(c_y, c_x, kill->y, kill->x)) continue;

        /* Note who gets considered */
        if (!borg_simulate)
        {
            borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. is considered.",
                kill->y, kill->x, (r_info[kill->r_idx].name),
                borg_danger_aux(c_y, c_x, 1, ag->kill, true, false)));
        }

        /* Non evil monsters*/
        if (!(rf_has(r_ptr->flags, RF_EVIL)))
        {
            /* Note who gets to stay */
            if (!borg_simulate)
            {
                borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. Stays (not evil).",
                    kill->y, kill->x, (r_info[kill->r_idx].name),
                    borg_danger_aux(c_y, c_x, 1, ag->kill, true, false)));
            }

            continue;
        }

        /* Unique Monster in good health*/
        if (rf_has(r_ptr->flags, RF_UNIQUE) && kill->injury > 60)
        {
            /* Note who gets to stay */
            if (!borg_simulate)
            {
                borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. Unique not considered: Injury %d.",
                    kill->y, kill->x, (r_info[kill->r_idx].name),
                    borg_danger_aux(c_y, c_x, 1, ag->kill, true, false), kill->injury));
            }

            continue;
        }

        /* Monsters in walls cant be booted */
        if (!borg_cave_floor_bold(kill->y, kill->x))
        {
            /* Note who gets banished */
            if (!borg_simulate)
            {
                borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. Stays (in wall).",
                    kill->y, kill->x, (r_info[kill->r_idx].name),
                    borg_danger_aux(c_y, c_x, 1, ag->kill, true, true)));
            }
            continue;
        }

        /* Note who gets banished */
        if (!borg_simulate)
        {
            borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. Booted.",
                kill->y, kill->x, (r_info[kill->r_idx].name),
                borg_danger_aux(c_y, c_x, 1, ag->kill, true, true)));
            borg_delete_kill(i);

        }

        /* Count */
        banished_monsters++;

        /* Calculate danger of who is left over */
        p2 -= borg_danger_aux(c_y, c_x, 1, i, true, true);

    }

    if (!borg_simulate)
    {
        /* attempt the banish */
        if (using_artifact)
            if (borg_activate_item(act_loskill))
                return (p1 - p2);
        if (borg_spell(BANISH_EVIL))
            return (p1 - p2);
    }

    /* p2 is the danger after all the bad guys are removed. */
    /* no negatives */
    if (p2 <= 0) p2 = 0;

    /* No monsters get booted */
    if (banished_monsters == 0) p2 = 9999;

    /* Try not to cast this against Morgy/Sauron */
    if (borg_fighting_unique >= 10 && borg_skill[BI_CURHP] > 250 && borg_skill[BI_CDEPTH] == 99) p2 = 9999;
    if (borg_fighting_unique >= 10 && borg_skill[BI_CURHP] > 350 && borg_skill[BI_CDEPTH] == 100) p2 = 9999;

    /* check to see if I am left better off */
    if (p1 > p2 &&
        p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3) : (avoidance / 2)))
    {
        /* Simulation */
        if (borg_simulate) return (p1 - p2);
    }
    return (0);
}



/*
 * Detect Inviso/Monsters
 * Used only if I am hit by an unseen guy.
 * Casts detect invis.
 */
static int borg_defend_aux_inviso(int p1)
{
    int fail_allowed = 25;
    borg_grid* ag = &borg_grids[c_y][c_x];


    /* no need */
    if (borg_skill[BI_ISFORGET] || borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_see_inv)
        return (0);

    /* not recent */
    if (borg_t > need_see_inviso + 5) return (0);


    /* too dangerous to cast */
    if (p1 > avoidance * 2) return (0);

    /* Do I have anything that will work? */
    if (-1 == borg_slot(TV_POTION, sv_potion_detect_invis) &&
        -1 == borg_slot(TV_SCROLL, sv_scroll_detect_invis) &&
        !borg_equips_staff_fail(sv_staff_detect_invis) &&
        !borg_equips_staff_fail(sv_staff_detect_evil) &&
        !borg_spell_okay_fail(SENSE_INVISIBLE, fail_allowed) &&
        !borg_spell_okay_fail(DETECTION, fail_allowed))
        return (0);

    /* Darkness */
    if (!(ag->info & BORG_GLOW) && !borg_skill[BI_CURLITE]) return (0);

    /* No real value known, but lets cast it to find the bad guys. */
    if (borg_simulate) return (10);


    /* smoke em if you got em */
    /* short time */
    /* snap shot */
    if (borg_spell_fail(REVEAL_MONSTERS, fail_allowed) ||
        borg_read_scroll(sv_scroll_detect_invis) ||
        borg_use_staff(sv_staff_detect_invis) ||
        borg_use_staff(sv_staff_detect_evil))
    {
        borg_see_inv = 3000; /* hack, actually a snap shot, no ignition message */
        return (10);
    }
    if (borg_quaff_potion(sv_potion_detect_invis))
    {
        borg_see_inv = 18000;
        borg_no_rest_prep = 18000;
        return (10);
    }
    /* long time */
    if (borg_spell_fail(SENSE_INVISIBLE, fail_allowed))
    {
        borg_see_inv = 30000;
        borg_no_rest_prep = 16000;
        return (10);
    }

    /* ah crap, I guess I wont be able to see them */
    return (0);

}

/*
 * Light Beam to spot lurkers
 * Used only if I am hit by an unseen guy.
 * Lights up a hallway.
 */
static int borg_defend_aux_lbeam(void)
{
    bool hallway = false;
    int x = c_x;
    int y = c_y;


    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Light Beam section to spot non seen guys */
        /* not recent, dont bother */
    if (borg_t > (need_see_inviso + 2))
        return (0);

    /* Check to see if I am in a hallway */
    /* Case 1a: north-south corridor */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
        !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1) &&
        !borg_cave_floor_bold(y + 1, x - 1) && !borg_cave_floor_bold(y + 1, x + 1) &&
        !borg_cave_floor_bold(y - 1, x - 1) && !borg_cave_floor_bold(y - 1, x + 1))
    {
        /* ok to light up */
        hallway = true;
    }

    /* Case 1b: east-west corridor */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
        !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x) &&
        !borg_cave_floor_bold(y + 1, x - 1) && !borg_cave_floor_bold(y + 1, x + 1) &&
        !borg_cave_floor_bold(y - 1, x - 1) && !borg_cave_floor_bold(y - 1, x + 1))
    {
        /* ok to light up */
        hallway = true;
    }

    /* Case 1aa: north-south doorway */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x) &&
        !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1))
    {
        /* ok to light up */
        hallway = true;
    }

    /* Case 1ba: east-west doorway */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1) &&
        !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x))
    {
        /* ok to light up */
        hallway = true;
    }


    /* not in a hallway */
    if (!hallway) return (0);

    /* Make sure I am not in too much danger */
    /* XXX '(' replaces previous use of global variable that was always
     * '('.  This is a BUG.  I however have no idea how to fix it bceause
     * I don't know the code well enough. -AS */
    if (borg_simulate && '(' > avoidance * 3 / 4) return (0);

    /* test the beam function */
    if (!borg_LIGHT_beam(true)) return (0);

    /* return some value */
    if (borg_simulate) return (10);


    /* if in a hallway call the Light Beam routine */
    if (borg_LIGHT_beam(false))
    {
        return (10);
    }
    return (0);
}

/* Shift the panel to locate offscreen monsters */
static int borg_defend_aux_panel_shift(void)
{
    int dir = 0;
    int wx = Term->offset_x / borg_panel_wid();
    int wy = Term->offset_y / borg_panel_hgt();

    /* no need */
    if (!need_shift_panel && borg_skill[BI_CDEPTH] < 70)
        return (0);

    /* if Morgy is on my panel, dont do it */
    if (borg_skill[BI_CDEPTH] == 100 && w_y == morgy_panel_y &&
        w_x == morgy_panel_x) return (0);

    /* Which direction do we need to move? */
    /* Shift panel to the right */
    if (c_x >= 52 && c_x <= 60 && wx == 0) dir = 6;
    if (c_x >= 84 && c_x <= 94 && wx == 1) dir = 6;
    if (c_x >= 116 && c_x <= 123 && wx == 2) dir = 6;
    if (c_x >= 148 && c_x <= 159 && wx == 3) dir = 6;
    /* Shift panel to the left */
    if (c_x <= 142 && c_x >= 136 && wx == 4) dir = 4;
    if (c_x <= 110 && c_x >= 103 && wx == 3) dir = 4;
    if (c_x <= 78 && c_x >= 70 && wx == 2) dir = 4;
    if (c_x <= 46 && c_x >= 37 && wx == 1) dir = 4;

    /* Shift panel down */
    if (c_y >= 15 && c_y <= 19 && wy == 0) dir = 2;
    if (c_y >= 25 && c_y <= 30 && wy == 1) dir = 2;
    if (c_y >= 36 && c_y <= 41 && wy == 2) dir = 2;
    if (c_y >= 48 && c_y <= 52 && wy == 3) dir = 2;
    /* Shift panel up */
    if (c_y <= 51 && c_y >= 47 && wy == 4) dir = 8;
    if (c_y <= 39 && c_y >= 35 && wy == 3) dir = 8;
    if (c_y <= 28 && c_y >= 24 && wy == 2) dir = 8;
    if (c_y <= 17 && c_y >= 13 && wy == 1) dir = 8;

    /* Do the Shift if needed, then note it,  reset the flag */
    if (need_shift_panel == true)
    {
        /* Send action (view panel info) */
        borg_keypress('L');

        if (dir) borg_keypress(I2D(dir));
        borg_keypress(ESCAPE);

        borg_note("# Shifted panel to locate offscreen monster.");
        need_shift_panel = false;

        /* Leave the panel shift mode */
        borg_keypress(ESCAPE);
    }
    else
        /* check to make sure its appropriate */
    {

        /* Hack Not if I just did one */
        if (when_shift_panel &&
            (borg_t - when_shift_panel <= 10 ||
                borg_t - borg_t_morgoth <= 10))
        {
            /* do nothing */
        }
        else
        {
            /* if not the first step */
            if (track_step.num)
            {
                /* shift up? only if a north corridor */
                if (dir == 8 && borg_projectable_pure(c_y, c_x, c_y - 2, c_x) &&
                    track_step.y[track_step.num - 1] != c_y - 1)
                {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    if (dir) borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
                /* shift down? only if a south corridor */
                else if (dir == 2 && borg_projectable_pure(c_y, c_x, c_y + 2, c_x) &&
                    track_step.y[track_step.num - 1] != c_y + 1)
                {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
                /* shift Left? only if a west corridor */
                else if (dir == 4 && borg_projectable_pure(c_y, c_x, c_y, c_x - 2) &&
                    track_step.x[track_step.num - 1] != c_x - 1)
                {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    if (dir) borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
                /* shift Right? only if a east corridor */
                else if (dir == 6 && borg_projectable_pure(c_y, c_x, c_y, c_x + 2) &&
                    track_step.x[track_step.num - 1] != c_x + 1)
                {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    if (dir) borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
            }
        }
    }
    /* This uses no energy */
    return (0);
}

/* This and the next routine is used on level 100 and when
 * attacking Morgoth. The borg has found a safe place to wait
 * for Morgoth to show.
 *
 * If the borg is not being threatened immediately by a monster,
 * then rest right here.
 *
 * Only borgs with teleport away and a good attack spell do this
 * routine.
 */
static int borg_defend_aux_rest(void)
{
    int i;

    if (!borg_morgoth_position && (!borg_as_position || borg_t - borg_t_antisummon >= 50)) return (0);

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level && (!borg_as_position || borg_t - borg_t_antisummon >= 50)) return (0);

    /* Not if I can not teleport others away */
#if 0
    if (!borg_spell_okay_fail(3, 1, 30) &&
        !borg_spell_okay_fail(4, 2, 30)) return (0);
#endif
    /* Not if a monster can see me */
    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];

        int x9 = kill->x;
        int y9 = kill->y;
        int ax, ay, d;

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Distance components */
        ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
        ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Minimal distance */
        if (d > z_info->max_range) continue;

        /* If I can see Morgoth or a guy with Ranged Attacks, don't rest. */
        if (borg_los(c_y, c_x, kill->y, kill->x) &&
            (kill->r_idx == borg_morgoth_id || kill->ranged_attack) &&
            avoidance <= borg_skill[BI_CURHP])
        {
            borg_note("# Not resting. I can see Morgoth or a shooter.");
            return(0);
        }

        /* If a little twitchy, its ok to stay put */
        if (avoidance > borg_skill[BI_CURHP]) continue;
    }

    /* Return some value for this rest */
    if (borg_simulate) return (200);

    /* Rest */
    borg_keypress(',');
    borg_note(format("# Resting on grid (%d, %d), waiting for Morgoth.", c_y, c_x));

    /* All done */
    return (200);
}

/*
 * Try to get rid of all of the monsters while I build my
 * Sea of Runes.
 */
static int borg_defend_aux_tele_away_morgoth(void)
{
    int p2 = 0;
    int fail_allowed = 40;
    int i, x, y;

    borg_grid* ag;

    /* Only if on level 100 */
    if (!(borg_skill[BI_CDEPTH] == 100)) return (0);

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level) return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Do I have the T.O. spell? */
    if (!borg_spell_okay_fail(TELEPORT_OTHER, fail_allowed)) return (0);

    /* Do I have the Glyph spell? No good to use TO if I cant build the sea of runes */
    if (borg_skill[BI_AGLYPH] < 10) return (0);

    /* No Teleport Other if surrounded */
    if (borg_surrounded() == true) return (0);

    /* Borg_temp_n temporarily stores several things.
     * Some of the borg_attack() sub-routines use these numbers,
     * which would have been filled in borg_attack().
     * Since this is a defence manuever which will move into
     * and borrow some of the borg_attack() subroutines, we need
     * to make sure that the borg_temp_n arrays are properly
     * filled.  Otherwise, the borg will attempt to consider
     * these grids which were left filled by some other routine.
     * Which was probably a flow routine which stored about 200
     * grids into the array.
     * Any change in inclusion/exclusion criteria for filling this
     * array in borg_attack() should be included here also.
     */

     /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate) return (0);

    /* Reset list */
    borg_temp_n = 0;
    borg_tp_other_n = 0;

    /* Find "nearby" monsters */
    for (i = 0; i < borg_kills_nxt; i++)
    {
        borg_kill* kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY)) continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW)) continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range) continue;

        /* Check the LOS */
        if (!borg_projectable(c_y, c_x, kill->y, kill->x)) continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n && borg_simulate) return (0);

    /* choose then target a bad guy or several
     * If left as bolt, he targets the single most nasty guy.
     * If left as beam, he targets the collection of monsters.
     */
    p2 = borg_launch_bolt(-1, 50, BORG_ATTACK_AWAY_ALL_MORGOTH, z_info->max_range, 0);

    /* Normalize the value a bit */
    if (p2 > 1000) p2 = 1000;

    /* Reset list */
    borg_temp_n = 0;
    borg_tp_other_n = 0;

    /* Return a good score to make him do it */
    if (borg_simulate) return (p2);

    /* Log the Path for Debug */
    borg_log_spellpath(true);

    /* Log additional info for debug */
    for (i = 0; i < borg_tp_other_n; i++)
    {
        borg_note(format("# %d, index %d (%d,%d)", borg_tp_other_n,
            borg_tp_other_index[i], borg_tp_other_y[i],
            borg_tp_other_x[i]));
    }

    borg_note("# Attempting to cast T.O. for depth 100.");

    /* Cast the spell */
    if (borg_spell(TELEPORT_OTHER) ||
        borg_activate_item(act_tele_other) ||
        borg_aim_wand(sv_wand_teleport_away))
    {
        /* Use target */
        borg_keypress('5');

        /* Set our shooting flag */
        successful_target = -1;

        /* Value */
        return (p2);
    }

    return (0);
}

/*
 * Try to get rid of all of the monsters while I build my
 * Sea of Runes.
 */
static int borg_defend_aux_banishment_morgoth(void)
{
    int fail_allowed = 50;
    int i, x, y;
    int count = 0;
    int glyphs = 0;

    borg_grid* ag;
    borg_kill* kill;
    struct monster_race* r_ptr;

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level) return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Scan grids looking for glyphs */
    for (i = 0; i < 8; i++)
    {
        /* Access offset */
        x = c_x + ddx_ddd[i];
        y = c_y + ddy_ddd[i];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Check for Glyphs */
        if (ag->glyph) glyphs++;
    }

    /* Only if on level 100 and in a sea of runes or
     * in the process of building one
     */
#if 0
    if (!borg_morgoth_position && glyphs < 3) return (0);
#endif

    /* Do I have the spell? (Banish Evil) */
    if (!borg_spell_okay_fail(MASS_BANISHMENT, fail_allowed) &&
        !borg_spell_okay_fail(BANISH_EVIL, fail_allowed)) return (0);

    /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate) return (0);

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        r_ptr = &r_info[kill->r_idx];

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never try on non-evil guys if Priest */
        if (borg_class == CLASS_PRIEST &&
            !(rf_has(r_ptr->flags, RF_EVIL))) continue;


        /* Check the distance  */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range) continue;

        /* Monster must be LOS */
        if (!borg_projectable(c_y, c_x, kill->y, kill->x)) continue;

        /* Count the number of monsters too close double*/
        if (borg_distance(c_y, c_x, y, x) <= 7) count++;

        /* Count the number of monster on screen */
        count++;
    }

    /* No destinations */
    if (count <= 7 && borg_simulate) return (0);

    /* Return a good score to make him do it */
    if (borg_simulate) return (1500);

    borg_note(format("# Attempting to cast Banishment for depth 100.  %d monsters ", count));

    /* Cast the spell */
    if (borg_spell(MASS_BANISHMENT) ||
        borg_spell(BANISH_EVIL))
    {
        /* Remove this race from the borg_kill */
        for (i = 0; i < borg_kills_nxt; i++)
        {
            borg_kill* tmp_kill;
            struct monster_race* tmp_r_ptr;

            /* Monster */
            tmp_kill = &borg_kills[i];
            tmp_r_ptr = &r_info[tmp_kill->r_idx];

            /* Cant kill uniques like this */
            if (rf_has(tmp_r_ptr->flags, RF_UNIQUE)) continue;

            /* remove this monster */
            borg_delete_kill(i);
        }

        /* Value */
        return (1000);
    }

    return (0);
}

/*
 * Sometimes the borg will not fire on Morgoth as he approaches
 * while tunneling through rock.  The borg still remembers and
 * assumes that the rock is unknown grid.
 */
static int borg_defend_aux_light_morgoth(void)
{
    int fail_allowed = 50;
    int i, x, y;
    int b_y = -1;
    int b_x = -1;
    int count = 0;

    borg_kill* kill;

    /* Only if on level 100 and in a sea of runes */
    if (!borg_morgoth_position) return (0);

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level) return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* Do I have the spell? */
    if (!borg_spell_okay_fail(SPEAR_OF_LIGHT, fail_allowed) &&
        !borg_spell_okay_fail(CLAIRVOYANCE, fail_allowed) &&
        !borg_spell_okay_fail(FUME_OF_MORDOR, fail_allowed)) return (0);

    /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate) return (0);

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Skip non- Morgoth monsters */
        if (kill->r_idx != borg_morgoth_id) continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2) continue;

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Check the distance  */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range) continue;
        if (borg_distance(c_y, c_x, y, x) <= 5) continue;

        /* We want at least one dark spot on the path */
        if (!borg_projectable_dark(c_y, c_x, y, x)) continue;

        /* Count Morgoth so I try the spell */
        count++;
        b_y = y;
        b_x = x;
    }

    /* No destinations */
    if (count <= 0 && borg_simulate) return (0);

    /* Return a good score to make him do it */
    if (borg_simulate) return (500);

    borg_note(format("# Attempting to Illuminate a Pathway to (%d, %d)", b_y, b_x));

    /* Target Morgoth Grid */
    (void)borg_target(b_y, b_x);

    /* Cast the spell */
    if (borg_spell(SPEAR_OF_LIGHT) ||
        borg_spell(CLAIRVOYANCE) ||
        borg_spell(FUME_OF_MORDOR))
    {
        /* Select the target */
        borg_keypress('5');

        /* Value */
        return (200);
    }

    return (0);
}

/*
 * Simulate/Apply the optimal result of using the given "type" of defence
 * p1 is the current danger level (passed in for effiency)
 */
static int borg_defend_aux(int what, int p1)
{
    /* Analyze */
    switch (what)
    {
    case BD_SPEED:
    {
        return (borg_defend_aux_speed(p1));
    }
    case BD_PROT_FROM_EVIL:
    {
        return (borg_defend_aux_prot_evil(p1));
    }
    case BD_GRIM_PURPOSE:
    {
        return (borg_defend_aux_grim_purpose(p1));
    }
    case BD_RESIST_FECAP:
    {
        return (borg_defend_aux_resist_fecap(p1));
    }
    case BD_RESIST_F:
    {
        return (borg_defend_aux_resist_f(p1));
    }
    case BD_RESIST_C:
    {
        return (borg_defend_aux_resist_c(p1));
    }
    case BD_RESIST_A:
    {
        return (borg_defend_aux_resist_a(p1));
    }
    case BD_RESIST_P:
    {
        return (borg_defend_aux_resist_p(p1));
    }
    case BD_BLESS:
    {
        return (borg_defend_aux_bless(p1));
    }
    case BD_HERO:
    {
        return (borg_defend_aux_hero(p1));
    }
    case BD_BERSERK:
    {
        return (borg_defend_aux_berserk(p1));
    }
    case BD_SMITE_EVIL:
    {
        return (borg_defend_aux_smite_evil(p1));
    }
    case BD_REGEN:
    {
        return (borg_defend_aux_regen(p1));
    }
    case BD_SHIELD:
    {
        return (borg_defend_aux_shield(p1));
    }
    case BD_TELE_AWAY:
    {
        return (borg_defend_aux_tele_away(p1));
    }
    case BD_GLYPH:
    {
        return (borg_defend_aux_glyph(p1));
    }
    case BD_CREATE_DOOR:
    {
        return (borg_defend_aux_create_door(p1));
    }
    case BD_MASS_GENOCIDE:
    {
        return (borg_defend_aux_mass_genocide(p1));
    }
    case BD_GENOCIDE:
    {
        return (borg_defend_aux_genocide(p1));
    }
    case BD_GENOCIDE_NASTIES:
    {
        return (borg_defend_aux_genocide_nasties(p1));
    }
    case BD_EARTHQUAKE:
    {
        return (borg_defend_aux_earthquake(p1));
    }
    case BD_TPORTLEVEL:
    {
        return (borg_defend_aux_teleportlevel(p1));
    }
    case BD_DESTRUCTION:
    {
        return (borg_defend_aux_destruction(p1));
    }
    case BD_BANISHMENT:
    {
        return (borg_defend_aux_banishment(p1));
    }
    case BD_DETECT_INVISO:
    {
        return (borg_defend_aux_inviso(p1));
    }
    case BD_LIGHT_BEAM:
    {
        return (borg_defend_aux_lbeam());
    }
    case BD_SHIFT_PANEL:
    {
        return (borg_defend_aux_panel_shift());
    }
    case BD_REST:
    {
        return (borg_defend_aux_rest());
    }
    case BD_TELE_AWAY_MORGOTH:
    {
        return (borg_defend_aux_tele_away_morgoth());
    }
    case BD_BANISHMENT_MORGOTH:
    {
        return (borg_defend_aux_banishment_morgoth());
    }
    case BD_LIGHT_MORGOTH:
    {
        return (borg_defend_aux_light_morgoth());
    }

    }
    return (0);
}

/*
 * prepare to attack... this is setup for a battle.
 */
bool borg_defend(int p1)
{
    int n, b_n = 0;
    int g, b_g = -1;

    /* Simulate */
    borg_simulate = true;


    /* if you have Resist All and it is about to drop, */
    /* refresh it (if you can) */
    if (borg_resistance && borg_resistance < (borg_game_ratio * 2))
    {
        int p;

        /* check 'true' danger. This will make sure we do not */
        /* refresh our Resistance if no-one is around */
        borg_attacking = true;
        p = borg_danger(c_y, c_x, 1, false, false); /* Note false for danger!! */
        borg_attacking = false;
        if (p > borg_fear_region[c_y / 11][c_x / 11] ||
            borg_fighting_unique)
        {
            if (borg_spell(RESISTANCE))
            {
                borg_note(format("# Refreshing Resistance.  borg_resistance=%d, player->=%d, (ratio=%d)", borg_resistance, player->timed[TMD_OPP_ACID], borg_game_ratio));
                borg_attempting_refresh_resist = true;
                borg_resistance = 25000;
                return (true);
            }
        }
    }

    /* Analyze the possible setup moves */
    for (g = 0; g < BD_MAX; g++)
    {
        /* Simulate */
        n = borg_defend_aux(g, p1);

        /* Track "best" attack */
        if (n <= b_n) continue;

        /* Track best */
        b_g = g;
        b_n = n;
    }

    /* Nothing good */
    if (b_n <= 0)
    {
        return (false);
    }

    /* Note */
    borg_note(format("# Performing defence type %d with value %d", b_g, b_n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_defend_aux(b_g, p1);

    /* Success */
    return (true);
}

/*
 * Perma spells.  Some are cool to have on all the time, so long as their
 * mana cost is not too much.
 * There are several types of setup moves:
 *
 *   Temporary speed
 *   Protect From Evil
 *   Prayer
 *   Temp Resist (either all or just cold/fire?)
 *   Shield
 */
enum
{
    BP_SPEED,
    BP_PROT_FROM_EVIL,
    BP_BLESS,

    BP_RESIST_ALL,
    BP_RESIST_ALL_COLLUIN,
    BP_RESIST_P,

    BP_FASTCAST,
    BP_HERO,
    BP_BERSERK,
    BP_BERSERK_POTION,

    BP_SMITE_EVIL,
    BP_VENOM,
    BP_REGEN,

    BP_GLYPH,
    BP_SEE_INV,

    BP_MAX
};

/*
 * Prayer to prepare for battle
 */
static int borg_perma_aux_bless(void)
{
    int fail_allowed = 15, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 20;
    if (borg_fighting_unique) fail_allowed = 25;

    /* already blessed */
    if (borg_bless)
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

    /* XXX Dark */

    if (!borg_spell_okay_fail(BLESS, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(BLESS);

    /* If its cheap, go ahead */
    if (borg_skill[BI_CLEVEL] > 10 &&
        cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    /* bless is a low priority */
    if (borg_simulate) return (1);

    /* do it! */
    borg_spell(BLESS);

    /* No resting to recoop mana */
    borg_no_rest_prep = 10000;

    return (1);
}
/* all resists FECAP*/
static int borg_perma_aux_resist(void)
{
    int cost = 0;
    int fail_allowed = 5;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    if (borg_skill[BI_TRFIRE] + borg_skill[BI_TRACID] + borg_skill[BI_TRPOIS] +
        borg_skill[BI_TRELEC] + borg_skill[BI_TRCOLD] >= 3)
        return (0);

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(RESISTANCE);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    if (borg_simulate) return (2);

    /* do it! */
    borg_spell_fail(RESISTANCE, fail_allowed);

    /* No resting to recoop mana */
    borg_no_rest_prep = 21000;

    /* default to can't do it. */
    return (2);
}

/* all resists from the cloak*/
static int borg_perma_aux_resist_colluin(void)
{
    if (borg_skill[BI_TRFIRE] + borg_skill[BI_TRACID] + borg_skill[BI_TRPOIS] +
        borg_skill[BI_TRELEC] + borg_skill[BI_TRCOLD] >= 3)
        return (0);

    /* Only use it when Unique is close */
    if (!borg_fighting_unique) return (0);

    if (!borg_equips_item(act_resist_all, true) &&
        !borg_equips_item(act_rage_bless_resist, true))
        return (0);

    /* Simulation */
    if (borg_simulate) return (2);

    /* do it! */
    if (borg_activate_item(act_resist_all) || borg_activate_item(act_rage_bless_resist))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 21000;
    }

    /* Value */
    return (2);
}

/* resists--- Only bother if a Unique is on the level.*/
static int borg_perma_aux_resist_p(void)
{
    int cost = 0;
    int fail_allowed = 5;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;


    if (borg_skill[BI_TRPOIS] || !unique_on_level)
        return (0);

    if (!borg_spell_okay_fail(RESIST_POISON, fail_allowed))
        return (0);

    /* Skip it if I can do the big spell */
    if (borg_spell_okay_fail(RESISTANCE, fail_allowed)) return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(RESIST_POISON);

    /* If its cheap, go ahead */
    if (cost >= borg_skill[BI_CURSP] / 20) return (0);

    /* Simulation */
    if (borg_simulate) return (1);

    /* do it! */
    if (borg_spell_fail(RESIST_POISON, fail_allowed))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 21000;

        /* Value */
        return (1);
    }

    /* default to can't do it. */
    return (0);
}


/*
 * Speed to prepare for battle
 */
static int borg_perma_aux_speed(void)
{
    int fail_allowed = 7;
    int cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already fast */
    if (borg_speed)
        return (0);

    /* only cast defence spells if fail rate is not too high */
    if (!borg_spell_okay_fail(HASTE_SELF, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(HASTE_SELF);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    if (borg_simulate) return (5);

    /* do it! */
    if (borg_spell_fail(HASTE_SELF, fail_allowed))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = borg_skill[BI_CLEVEL] * 1000;
        return (5);
    }

    /* default to can't do it. */
    return (0);
}

static int borg_perma_aux_prot_evil(void)
{
    int cost = 0;
    int fail_allowed = 5;

    /* if already protected */
    if (borg_prot_from_evil)
        return (0);

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;


    if (!borg_spell_okay_fail(PROTECTION_FROM_EVIL, fail_allowed)) return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(PROTECTION_FROM_EVIL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    if (borg_simulate) return (3);

    /* do it! */
    if (borg_spell_fail(PROTECTION_FROM_EVIL, fail_allowed))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = borg_skill[BI_CLEVEL] * 1000;

        /* Value */
        return (3);
    }

    /* default to can't do it. */
    return (0);
}

/*
 * Mana Channel to prepare for battle
 */
static int borg_perma_aux_fastcast(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already fast */
    if (borg_fastcast)
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

    if (!borg_spell_okay_fail(MANA_CHANNEL, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(MANA_CHANNEL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    /* fastcast is a low priority */
    if (borg_simulate) return (5);

    /* do it! */
    if (borg_spell(MANA_CHANNEL))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 6000;
        return 1;
    }

    return (0);
}

/*
 * Hero to prepare for battle
 */
static int borg_perma_aux_hero(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already blessed */
    if (borg_hero)
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

    /* XXX Dark */

    if (!borg_spell_okay_fail(HEROISM, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(HEROISM);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    /* hero is a low priority */
    if (borg_simulate) return (1);

    /* do it! */
    if (borg_spell(HEROISM))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 3000;
        return 1;
    }


    return (0);
}

/*
 * Rapid Regen to prepare for battle
 */
static int borg_perma_aux_regen(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already regenerating */
    if (borg_regen)
        return (0);

    /* Cant when screwed */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISFORGET]) return (0);

    /* don't bother if not much to regenerate */
    if (borg_skill[BI_MAXHP] < 100) return (0);

    if (!borg_spell_okay_fail(RAPID_REGENERATION, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(RAPID_REGENERATION);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* do it! */
    if (borg_spell(RAPID_REGENERATION))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 6000;
        return 1;
    }

    return (0);
}

/*
 * Smite evil to prepare for battle
 */
static int borg_perma_aux_smite_evil(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already smoting */
    if (borg_smite_evil || borg_skill[BI_WS_EVIL])
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

    if (!borg_spell_okay_fail(SMITE_EVIL, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(SMITE_EVIL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    /* smite evil is a low priority */
    if (borg_simulate) return (3);

    /* do it! */
    if (borg_spell(SMITE_EVIL))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 21000;
        return 3;
    }

    return (0);
}

/*
 * Poison your weapon to prepare for battle
 */
static int borg_perma_aux_venom(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already smoting */
    if (borg_venom || borg_skill[BI_WB_POIS])
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

    if (!borg_spell_okay_fail(VENOM, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(SMITE_EVIL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    /* smite evil is a low priority */
    if (borg_simulate) return (3);

    /* do it! */
    if (borg_spell(VENOM))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 19000;
        return 3;
    }

    return (0);
}
/*
 * Berserk to prepare for battle
 */
static int borg_perma_aux_berserk(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level) fail_allowed = 10;
    if (borg_fighting_unique) fail_allowed = 15;

    /* already blessed */
    if (borg_berserk)
        return (0);

    /* Cant when Blind */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (0);

    if (!borg_spell_okay_fail(BERSERK_STRENGTH, fail_allowed))
        return (0);

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(BERSERK_STRENGTH);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg_skill[BI_CURSP] / 7 : borg_skill[BI_CURSP] / 10)) return (0);

    /* Simulation */
    /* Berserk is a low priority */
    if (borg_simulate) return (2);

    /* do it! */
    if (borg_spell(BERSERK_STRENGTH))
    {
        /* No resting to recoop mana */
        borg_no_rest_prep = 11000;
        return 2;
    }

    return (0);
}

/*
 * Berserk to prepare for battle
 */
static int borg_perma_aux_berserk_potion(void)
{

    /* Saver the potions */
    if (!borg_fighting_unique) return (0);

    /* already blessed */
    if (borg_hero || borg_berserk)
        return (0);

    /* do I have any? */
    if (-1 == borg_slot(TV_POTION, sv_potion_berserk))
        return (0);

    /* Simulation */
    /* Berserk is a low priority */
    if (borg_simulate) return (2);

    /* do it! */
    if (borg_quaff_potion(sv_potion_berserk))
        return (2);

    return (0);
}

#ifdef UNUSED
/* Glyph of Warding in a a-s corridor */
static int borg_perma_aux_glyph(void)
{
    int i, wall_y, wall_x, wall_count = 0, y, x;
    int fail_allowed = 20;

    borg_grid* ag = &borg_grids[c_y][c_x];

    /* check to make sure a summoner is near */
    if (borg_kills_summoner == -1) return (0);

    /* make sure I have the spell */
    if (!borg_spell_okay_fail(3, 4, fail_allowed) &&
        !borg_spell_okay_fail(6, 4, fail_allowed)) return (0);


    /* He should not cast it while on an object.
     * I have addressed this inadequately in borg9.c when dealing with
     * messages.  The message "the object resists" will delete the glyph
     * from the array.  Then I set a broken door on that spot, the borg ignores
     * broken doors, so he won't loop.
     */
    if ((ag->take) ||
        (ag->feat == FEAT_GLYPH) ||
        ((ag->feat >= FEAT_TRAP_HEAD) && (ag->feat <= FEAT_TRAP_TAIL)) ||
        ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_TAIL)) ||
        (ag->feat == FEAT_LESS) ||
        (ag->feat == FEAT_MORE) ||
        (ag->feat == FEAT_OPEN) ||
        (ag->feat == FEAT_BROKEN))
    {
        return (0);
    }

    /* Check for an existing glyph that is not found in the auto_grid */
    for (i = 0; i < track_glyph.num; i++)
    {
        /* Stop if we are on a glyph */
        if ((track_glyph.x[i] == c_x) && (track_glyph.y[i] == c_y)) return (0);
    }

    /* This spell is cast while he is digging and AS Corridor */
    /* Get grid */
    for (wall_x = -1; wall_x <= 1; wall_x++)
    {
        for (wall_y = -1; wall_y <= 1; wall_y++)
        {
            /* Acquire location */
            x = wall_x + c_x;
            y = wall_y + c_y;

            ag = &borg_grids[y][x];

            /* track adjacent walls */
            if ((ag->feat == FEAT_GLYPH) ||
                ((ag->feat >= FEAT_MAGMA) && (ag->feat <= FEAT_WALL_SOLID)))
            {
                wall_count++;
            }
        }
    }

    /* must be in a corridor */
    if (wall_count < 6) return (0);

    /* Simulation */
    if (borg_simulate) return (10);

    /* do it! */
    if (borg_spell_fail(3, 4, fail_allowed) ||
        borg_spell_fail(6, 4, fail_allowed) ||
        borg_read_scroll(sv_scroll_rune_of_protection))
    {
        /* Check for an existing glyph */
        for (i = 0; i < track_glyph.num; i++)
        {
            /* Stop if we already new about this glyph */
            if ((track_glyph.x[i] == c_x) && (track_glyph.y[i] == c_y)) return (p1 - p2);
        }

        /* Track the newly discovered glyph */
        if (track_glyph.num < track_glyph.size)
        {
            borg_note("# Noting the creation of a corridor glyph.");
            track_glyph.x[track_glyph.num] = c_x;
            track_glyph.y[track_glyph.num] = c_y;
            track_glyph.num++;
        }
        return (p1 - p2);
    }

    /* default to can't do it. */
    return (0);
}
#endif

/*
 * Detect Inviso/Monsters
 * Casts detect invis.
 */
static int borg_perma_aux_see_inv(void)
{
    int fail_allowed = 25;
    borg_grid* ag = &borg_grids[c_y][c_x];


    /* no need */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
        borg_skill[BI_SINV] || borg_see_inv)
        return (0);

    /* Do I have anything that will work? */
    if (!borg_spell_okay_fail(SENSE_INVISIBLE, fail_allowed) /* &&
        !borg_spell_okay_fail(2, 6, fail_allowed) */)
        return (0);

    /* Darkness */
    if (!(ag->info & BORG_GLOW) && !borg_skill[BI_CURLITE]) return (0);

    /* No real value known, but lets cast it to find the bad guys. */
    if (borg_simulate) return (10);


    /* long time */
    if (borg_spell_fail(SENSE_INVISIBLE, fail_allowed) /* ||
        borg_spell_fail(2, 6, fail_allowed) */)
    {
        borg_see_inv = 32000;
        borg_no_rest_prep = 16000;
        return (10);
    }

    /* ah crap, I guess I wont be able to see them */
    return (0);

}



/*
 * Simulate/Apply the optimal result of using the given "type" of set-up
 */
static int borg_perma_aux(int what)
{

    /* Analyze */
    switch (what)
    {
    case BP_SPEED:
    {
        return (borg_perma_aux_speed());
    }

    case BP_PROT_FROM_EVIL:
    {
        return (borg_perma_aux_prot_evil());
    }
    case BP_RESIST_ALL:
    {
        return (borg_perma_aux_resist());
    }
    case BP_RESIST_ALL_COLLUIN:
    {
        return (borg_perma_aux_resist_colluin());
    }
    case BP_RESIST_P:
    {
        return (borg_perma_aux_resist_p());
    }
    case BP_BLESS:
    {
        return (borg_perma_aux_bless());
    }
    case BP_FASTCAST:
    {
        return (borg_perma_aux_fastcast());
    }
    case BP_HERO:
    {
        return (borg_perma_aux_hero());
    }
    case BP_BERSERK:
    {
        return (borg_perma_aux_berserk());
    }
    case BP_BERSERK_POTION:
    {
        return (borg_perma_aux_berserk_potion());
    }
    case BP_SMITE_EVIL:
    {
        return (borg_perma_aux_smite_evil());
    }
    case BP_VENOM:
    {
        return (borg_perma_aux_venom());
    }
    case BP_REGEN:
    {
        return (borg_perma_aux_regen());
    }
    case BP_GLYPH:
    {
        /* return (borg_perma_aux_glyph()); Tends to use too much mana doing this */
        return (0);
    }
    case BP_SEE_INV:
    {
        return (borg_perma_aux_see_inv());
    }
    }
    return (0);
}


/*
 * Walk around with certain spells on if you can afford to do so.
 */
bool borg_perma_spell(void)
{
    int n, b_n = 0;
    int g, b_g = -1;


    /* Simulate */
    borg_simulate = true;

    /* Not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);

    /* Not in shallow dungeon */
    if (borg_skill[BI_CDEPTH] < borg_skill[BI_CLEVEL] / 3 ||
        borg_skill[BI_CDEPTH] < 7) return (false);

    /* Low Level, save your mana, use the Defence maneuvers above */
    if (borg_skill[BI_CLEVEL] <= 10) return (false);

    /* Only when lots of mana is on hand */
    if (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * 75 / 100) return (false);

    /* Analyze the possible setup moves */
    for (g = 0; g < BP_MAX; g++)
    {
        /* Simulate */
        n = borg_perma_aux(g);

        /* Track "best" move */
        if (n <= b_n) continue;

        /* Track best */
        b_g = g;
        b_n = n;
    }

    /* Nothing good */
    if (b_n <= 0)
    {
        return (false);
    }

    /* Note */
    borg_note(format("# Performing perma-spell type %d with value %d", b_g, b_n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_perma_aux(b_g);
    /* Success */
    return (true);

}

/*
 * check to make sure there are no monsters around
 * that should prevent resting
 */
bool borg_check_rest(int y, int x)
{
    int i, ii;
    bool borg_in_vault = false;

    /* never rest to recover SP (if HP at max) if you only recover */
    /* sp in combat */
    if (borg_skill[BI_CURHP] == borg_skill[BI_MAXHP] &&
        player_has(player, PF_COMBAT_REGEN))
        return false;


    /* Do not rest recently after killing a multiplier */
    /* This will avoid the problem of resting next to */
    /* an unkown area full of breeders */
    if (when_last_kill_mult > (borg_t - 4) &&
        when_last_kill_mult <= borg_t)
        return (false);

    /* No resting if Blessed and good HP and good SP */
    /* don't rest for SP if you do combat regen */
    if ((borg_bless || borg_hero || borg_berserk || borg_fastcast || borg_regen || borg_smite_evil) &&
        !borg_munchkin_mode &&
        (borg_skill[BI_CURHP] >= borg_skill[BI_MAXHP] * 8 / 10) &&
        (borg_skill[BI_CURSP] >= borg_skill[BI_MAXSP] * 7 / 10)) return (false);

    /* Set this to Zero */
    when_last_kill_mult = 0;

    /* Most of the time, its ok to rest in a vault */
    if (vault_on_level)
    {
        for (i = -1; i < 1; i++)
        {
            for (ii = -1; ii < 1; ii++)
            {
                /* check bounds */
                if (!square_in_bounds_fully(cave, loc(c_x + ii, c_y + i))) continue;

                if (borg_grids[c_y + i][c_x + ii].feat == FEAT_PERM) borg_in_vault = true;
            }
        }
    }

    /* No resting to recover if I just cast a prepatory spell
     * which is what I like to do right before I take a stair,
     * Unless I am down by three quarters of my SP.
     */
    if (borg_no_rest_prep >= 1 && !borg_munchkin_mode && borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] / 4 &&
        borg_skill[BI_CDEPTH] < 85) return (false);

    /* Don't rest on lava unless we are immune to fire */
    if (borg_grids[y][x].feat == FEAT_LAVA && !borg_skill[BI_IFIRE]) return (false);

    /* Dont worry about fears if in a vault */
    if (!borg_in_vault)
    {
        /* Be concerned about the Regional Fear. */
        if (borg_fear_region[y / 11][x / 11] > borg_skill[BI_CURHP] / 20 &&
            borg_skill[BI_CDEPTH] != 100) return (false);

        /* Be concerned about the Monster Fear. */
        if (borg_fear_monsters[y][x] > borg_skill[BI_CURHP] / 10 &&
            borg_skill[BI_CDEPTH] != 100) return (false);

        /* Be concerned about the Monster Danger. */
        if (borg_danger(y, x, 1, true, false) > borg_skill[BI_CURHP] / 40 &&
            borg_skill[BI_CDEPTH] >= 85) return (false);

        /* Be concerned if low on food */
        if ((borg_skill[BI_CURLITE] == 0 || borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] < 2) && !borg_munchkin_mode)
            return (false);
    }

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];
        struct monster_race* r_ptr = &r_info[kill->r_idx];

        int x9 = kill->x;
        int y9 = kill->y;
        int ax, ay, d;
        int p = 0;

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Distance components */
        ax = (x9 > x) ? (x9 - x) : (x - x9);
        ay = (y9 > y) ? (y9 - y) : (y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Minimal distance */
        if (d > z_info->max_range) continue;

        /* if too close to a Mold or other Never-Mover, don't rest */
        if (d < 2 && !(rf_has(r_ptr->flags, RF_NEVER_MOVE))) return (false);
        if (d == 1) return (false);

        /* if too close to a Multiplier, don't rest */
        if (d < 10 && (rf_has(r_ptr->flags, RF_MULTIPLY))) return (false);

        /* If monster is asleep, dont worry */
        if (!kill->awake && d > 8 && !borg_munchkin_mode) continue;

        /* one call for dangers */
        p = borg_danger_aux(y9, x9, 1, i, true, true);

        /* Ignore proximity checks while inside a vault */
        if (!borg_in_vault)
        {
            /* Real scary guys pretty close */
            if (d < 5 && (p > avoidance / 3) && !borg_munchkin_mode) return (false);

            /* scary guys far away */
            /*if (d < 17 && d > 5 && (p > avoidance/3)) return (false); */

        }

        /* should check LOS... monster to me concerned for Ranged Attacks */
        if (borg_los(y9, x9, y, x) && kill->ranged_attack) return false;

        /* Special handling for the munchkin mode */
        if (borg_munchkin_mode && borg_los(y9, x9, y, x) &&
            (kill->awake && !(rf_has(r_ptr->flags, RF_NEVER_MOVE)))) return false;

        /* if it walks through walls, not safe */
        if ((rf_has(r_ptr->flags, RF_PASS_WALL)) && !borg_in_vault) return false;
        if (rf_has(r_ptr->flags, RF_KILL_WALL) && !borg_in_vault) return false;

    }
    return true;
}

/*
 * Attempt to recover from damage and such after a battle
 *
 * Note that resting while in danger is counter-productive, unless
 * the danger is low, in which case it may induce "farming".
 *
 * Note that resting while recall is active will often cause you
 * to lose hard-won treasure from nasty monsters, so we disable
 * resting when waiting for recall in the dungeon near objects.
 *
 * First we try spells/prayers, which are "free", and then we
 * try food, potions, scrolls, staffs, rods, artifacts, etc.
 *
 * XXX XXX XXX
 * Currently, we use healing spells as if they were "free", but really,
 * this is only true if the "danger" is less than the "reward" of doing
 * the healing spell, and if there are no monsters which might soon step
 * around the corner and attack.
 */
bool borg_recover(void)
{
    int p = 0;
    int q;
    enum borg_need need;

    /*** Handle annoying situations ***/
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note(format("# Need to refuel but cant!"));

    /*** Do not recover when in danger ***/

    /* Look around for danger */
    p = borg_danger(c_y, c_x, 1, true, false);

    /* Never recover in dangerous situations */
    if (p > avoidance / 4) return (false);


    /*** Roll for "paranoia" ***/

    /* Base roll */
    q = randint0(100);

    /* Half dead */
    if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) q = q - 10;

    /* Almost dead */
    if (borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 4) q = q - 10;


    /*** Use "cheap" cures ***/

    /* Hack -- cure stun */
    if (borg_skill[BI_ISSTUN] && (q < 75))
    {
        if (borg_activate_item(act_cure_body) ||
            borg_activate_item(act_cure_critical) ||
            borg_activate_item(act_cure_full) ||
            borg_activate_item(act_cure_full2) ||
            borg_activate_item(act_cure_temp) ||
            borg_activate_item(act_heal3) ||
            borg_spell(MINOR_HEALING) ||
            borg_spell(HEALING) ||
            borg_spell(HERBAL_CURING) ||
            borg_spell(HOLY_WORD))

        {
            /* Take note */
            borg_note(format("# Cure Stun - danger %d", p));

            return (true);
        }
    }

    /* Hack -- cure stun */
    if (borg_skill[BI_ISHEAVYSTUN])
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
            borg_activate_item(act_cure_body) ||
            borg_activate_item(act_cure_critical) ||
            borg_activate_item(act_cure_full) ||
            borg_activate_item(act_cure_full2) ||
            borg_activate_item(act_cure_temp) ||
            borg_activate_item(act_heal3) ||
            borg_spell(MINOR_HEALING) ||
            borg_spell(HEALING) ||
            borg_spell(HERBAL_CURING) ||
            borg_spell(HOLY_WORD))
        {
            /* Take note */
            borg_note(format("# Cure Heavy Stun - danger %d", p));

            return (true);
        }
    }

    /* Hack -- cure cuts */
    if (borg_skill[BI_ISCUT] && (q < 75))
    {
        if (borg_activate_item(act_cure_light) ||
            borg_spell(MINOR_HEALING) ||
            borg_spell(HEALING) ||
            borg_spell(HERBAL_CURING) ||
            borg_spell(HOLY_WORD))
        {
            /* Take note */
            borg_note(format("# Cure Cuts - danger %d", p));

            return (true);
        }
    }

    /* Hack -- cure poison */
    if (borg_skill[BI_ISPOISONED] && (q < 75))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
            borg_activate_item(act_rem_fear_pois) ||
            borg_spell(HERBAL_CURING) ||
            borg_spell(CURE_POISON))
        {
            /* Take note */
            borg_note(format("# Cure poison - danger %d", p));

            return (true);
        }
    }

    /* Hack -- cure fear */
    if (borg_skill[BI_ISAFRAID] && !borg_skill[BI_CRSFEAR] && (q < 75))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind) ||
            borg_activate_item(act_rem_fear_pois) ||
            borg_spell(HEROISM) ||
            borg_spell(BERSERK_STRENGTH) ||
            borg_spell(HOLY_WORD))
        {
            /* Take note */
            borg_note(format("# Cure fear - danger %d", p));

            return (true);
        }
    }

    /* Hack -- satisfy hunger */
    if ((borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) && (q < 75))
    {
        if (borg_spell(REMOVE_HUNGER) ||
            borg_spell(HERBAL_CURING))
        {
            return (true);
        }
    }

    /* Hack -- hallucination */
    if (borg_skill[BI_ISIMAGE] && (q < 75))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind))
        {
            return (true);
        }
    }

    /* Hack -- heal damage */
    if ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) && (q < 75) && p == 0 &&
        (borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] / 4))
    {
        if (borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2) ||
            borg_activate_item(act_heal3) ||
            borg_spell(HEALING) ||
            borg_spell(HOLY_WORD) ||
            borg_spell(MINOR_HEALING) ||
            borg_spell(HEROISM))
        {
            /* Take note */
            borg_note(format("# heal damage (recovering)"));

            return (true);
        }
    }

    /* cure experience loss with prayer */
    if (borg_skill[BI_ISFIXEXP] && (borg_activate_item(act_restore_exp) ||
        borg_activate_item(act_restore_life) ||
        borg_spell(REVITALIZE) ||
        borg_spell(REMEMBRANCE) ||
        (borg_skill[BI_CURHP] > 90 && borg_spell(UNHOLY_REPRIEVE))))
    {
        return (true);
    }

    /* cure stat drain with prayer */
    if ((borg_skill[BI_ISFIXSTR] ||
        borg_skill[BI_ISFIXINT] ||
        borg_skill[BI_ISFIXWIS] ||
        borg_skill[BI_ISFIXDEX] ||
        borg_skill[BI_ISFIXCON] ||
        borg_skill[BI_ISFIXALL]) &&
        (borg_spell(RESTORATION) ||
         borg_spell(REVITALIZE)))
    {
        return (true);
    }

    /* cure stat drain with prayer */
    if ((borg_skill[BI_ISFIXSTR] ||
        borg_skill[BI_ISFIXINT] ||
        borg_skill[BI_ISFIXCON]) &&
        borg_skill[BI_CURHP] > 90 &&
        borg_spell(UNHOLY_REPRIEVE))
    {
        return (true);
    }

    /*** Use "expensive" cures ***/

    /* Hack -- cure stun */
    if (borg_skill[BI_ISSTUN] && (q < 25))
    {
        if (borg_use_staff_fail(sv_staff_curing) ||
            borg_zap_rod(sv_rod_curing) ||
            borg_zap_rod(sv_rod_healing) ||
            borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2) ||
            borg_quaff_crit(false))
        {
            return (true);
        }
    }

    /* Hack -- cure heavy stun */
    if (borg_skill[BI_ISHEAVYSTUN] && (q < 95))
    {
        if (borg_quaff_crit(true) ||
            borg_use_staff_fail(sv_staff_curing) ||
            borg_zap_rod(sv_rod_curing) ||
            borg_zap_rod(sv_rod_healing) ||
            borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2))
        {
            return (true);
        }
    }

    /* Hack -- cure cuts */
    if (borg_skill[BI_ISCUT] && (q < 25))
    {
        if (borg_use_staff_fail(sv_staff_curing) ||
            borg_zap_rod(sv_rod_curing) ||
            borg_zap_rod(sv_rod_healing) ||
            borg_activate_item(act_heal1) ||
            borg_activate_item(act_heal2) ||
            borg_quaff_crit(borg_skill[BI_CURHP] < 10))
        {
            return (true);
        }
    }

    /* Hack -- cure poison */
    if (borg_skill[BI_ISPOISONED] && (q < 25))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
            borg_quaff_potion(sv_potion_cure_poison) ||
            borg_eat_food(TV_FOOD, sv_food_waybread) ||
            borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
            borg_quaff_crit(borg_skill[BI_CURHP] < 10) ||
            borg_use_staff_fail(sv_staff_curing) ||
            borg_zap_rod(sv_rod_curing) ||
            borg_activate_item(act_rem_fear_pois))
        {
            return (true);
        }
    }

    /* Hack -- cure blindness */
    if (borg_skill[BI_ISBLIND] && (q < 25))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_fast_recovery) ||
            borg_eat_food(TV_FOOD, sv_food_waybread) ||
            borg_quaff_potion(sv_potion_cure_light) ||
            borg_quaff_potion(sv_potion_cure_serious) ||
            borg_quaff_crit(false) ||
            borg_use_staff_fail(sv_staff_curing) ||
            borg_zap_rod(sv_rod_curing))
        {
            return (true);
        }
    }

    /* Hack -- cure confusion */
    if (borg_skill[BI_ISCONFUSED] && (q < 25))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind) ||
            borg_quaff_potion(sv_potion_cure_serious) ||
            borg_quaff_crit(false) ||
            borg_use_staff_fail(sv_staff_curing) ||
            borg_zap_rod(sv_rod_curing))
        {
            return (true);
        }
    }

    /* Hack -- cure fear */
    if (borg_skill[BI_ISAFRAID] && !borg_skill[BI_CRSFEAR] && (q < 25))
    {
        if (borg_eat_food(TV_MUSHROOM, sv_mush_cure_mind) ||
            borg_quaff_potion(sv_potion_boldness) ||
            borg_quaff_potion(sv_potion_heroism) ||
            borg_quaff_potion(sv_potion_berserk) ||
            borg_activate_item(act_rem_fear_pois))
        {
            return (true);
        }
    }

    /* Hack -- satisfy hunger */
    if ((borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) && (q < 25))
    {
        if (borg_read_scroll(sv_scroll_satisfy_hunger))
        {
            return (true);
        }
    }

    /* Hack -- heal damage */
    if ((borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] / 2) && (q < 25))
    {
        if (borg_zap_rod(sv_rod_healing) ||
            borg_quaff_potion(sv_potion_cure_serious) ||
            borg_quaff_crit(false) ||
            borg_activate_item(act_cure_serious))
        {
            return (true);
        }
    }

    /* Hack -- Rest to recharge Rods of Healing or Recall*/
    if (borg_has[kv_rod_recall] || borg_has[kv_rod_healing])
    {
        /* Step 1.  Recharge just 1 rod. */
        if ((borg_has[kv_rod_healing] && !borg_items[borg_slot(TV_ROD, sv_rod_healing)].pval) ||
            (borg_has[kv_rod_recall] && !borg_items[borg_slot(TV_ROD, sv_rod_recall)].pval))
        {
            /* Mages can cast the recharge spell */

            /* Rest until at least one recharges */
            if (!borg_skill[BI_ISWEAK] && !borg_skill[BI_ISCUT] && !borg_skill[BI_ISHUNGRY] && !borg_skill[BI_ISPOISONED] &&
                borg_check_rest(c_y, c_x) && !borg_spell_okay(RECHARGING))
            {
                /* Take note */
                borg_note("# Resting to recharge a rod...");

                /* Reset the Bouncing-borg Timer */
                time_this_panel = 0;

                /* Rest until done */
                borg_keypress('R');
                borg_keypress('1');
                borg_keypress('0');
                borg_keypress('0');
                borg_keypress(KC_ENTER);

                /* I'm not in a store */
                borg_in_shop = false;

                /* Done */
                return (true);
            }
        }
    }

    /*** Just Rest ***/

    /* Hack -- rest until healed */
    if (!borg_skill[BI_ISBLIND] && !borg_skill[BI_ISPOISONED] && !borg_skill[BI_ISCUT] &&
        !borg_skill[BI_ISWEAK] && !borg_skill[BI_ISHUNGRY] &&
        (borg_skill[BI_ISCONFUSED] || borg_skill[BI_ISIMAGE] || borg_skill[BI_ISAFRAID] || borg_skill[BI_ISSTUN] || borg_skill[BI_ISHEAVYSTUN] ||
            borg_skill[BI_CURHP] < borg_skill[BI_MAXHP] || borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] * (borg_skill[BI_CDEPTH] > 85 ? 7 : 6) / 10))
    {
        if (borg_check_rest(c_y, c_x) && !scaryguy_on_level &&
            p <= borg_fear_region[c_y / 11][c_x / 11] && goal != GOAL_RECOVER)
        {

            /* check for then call lite in dark room before resting */
            if (!borg_check_LIGHT_only())
            {
                /* Take note */
                borg_note(format("# Resting to recover HP/SP..."));

                /* Rest until done */
                borg_keypress('R');
                borg_keypress('&');
                borg_keypress(KC_ENTER);

                /* Reset our panel clock, we need to be here */
                time_this_panel = 0;

                /* reset the inviso clock to avoid loops */
                need_see_inviso = borg_t - 50;

                /* Done */
                return (true);
            }
            else
            {
                /* Must have been a dark room */
                borg_note(format("# Lighted the darkened room instead of resting."));
                return (true);
            }
        }
    }

    /* Hack to recharge mana if a low level mage or priest */
    if (borg_skill[BI_MAXSP] && (borg_skill[BI_CLEVEL] <= 40 || borg_skill[BI_CDEPTH] >= 85) &&
        borg_skill[BI_CURSP] < (borg_skill[BI_MAXSP] * 8 / 10) &&
        p < avoidance * 1 / 10 && borg_check_rest(c_y, c_x))
    {
        if (!borg_skill[BI_ISWEAK] && !borg_skill[BI_ISCUT] &&
            !borg_skill[BI_ISHUNGRY] && !borg_skill[BI_ISPOISONED] && borg_skill[BI_FOOD] > 2 && !borg_munchkin_mode)
        {
            /* Take note */
            borg_note(format("# Resting to gain Mana. (danger %d)...", p));

            /* Rest until done */
            borg_keypress('R');
            borg_keypress('*');
            borg_keypress(KC_ENTER);

            /* I'm not in a store */
            borg_in_shop = false;

            /* Done */
            return (true);
        }
    }

    /* Hack to recharge mana if a low level mage in munchkin mode */
    if (borg_skill[BI_MAXSP] && borg_munchkin_mode == true &&
        (borg_skill[BI_CURSP] < borg_skill[BI_MAXSP] ||
            borg_skill[BI_CURHP] < borg_skill[BI_MAXHP]) && borg_check_rest(c_y, c_x))
    {
        if (!borg_skill[BI_ISWEAK] && !borg_skill[BI_ISCUT] &&
            !borg_skill[BI_ISHUNGRY] && !borg_skill[BI_ISPOISONED] && borg_skill[BI_FOOD] > 2 &&
            (borg_grids[c_y][c_x].feat == FEAT_MORE || borg_grids[c_y][c_x].feat == FEAT_LESS))
        {
            /* Take note */
            borg_note(format("# Resting to gain munchkin HP/mana. (danger %d)...", p));

            /* Rest until done */
            borg_keypress('R');
            borg_keypress('*');
            borg_keypress(KC_ENTER);

            /* I'm not in a store */
            borg_in_shop = false;

            /* Done */
            return (true);
        }
    }

    /* Hack to heal blindness if in munchkin mode */
    if (borg_skill[BI_ISBLIND] && borg_munchkin_mode == true)
    {
        /* Take note */
        borg_note("# Resting to cure problem. (danger %d)...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('*');
        borg_keypress(KC_ENTER);

        /* I'm not in a store */
        borg_in_shop = false;

        /* Done */
        return (true);
    }


    /* Nope */
    return (false);
}

/*
 * Check if the borg can dig.  
 *   check_fail = check if the spell failure rate is too high
 *   hard = check if hard things, like granite, can be dug
 */
static bool borg_can_dig(bool check_fail, bool hard)
{
    /* No digging when hungry */
    if (borg_skill[BI_ISHUNGRY])
        return false;

    int dig_check = hard ? BORG_DIG_HARD : BORG_DIG;
    if ((weapon_swap && borg_skill[BI_DIG] >= dig_check && borg_items[weapon_swap -1].tval == TV_DIGGING) ||
        (borg_skill[BI_DIG] >= dig_check + 20))
        return true;
        
    if (check_fail)
    {
        if (borg_spell_legal_fail(TURN_STONE_TO_MUD, 40) ||
            borg_spell_legal_fail(SHATTER_STONE, 40) ||
            borg_equips_item(act_stone_to_mud, true) ||
            borg_equips_ring(sv_ring_digging))
            return true;
    }
    else
    {
        if (borg_spell_legal(TURN_STONE_TO_MUD) ||
            borg_spell_legal(SHATTER_STONE) ||
            borg_equips_item(act_stone_to_mud, false) ||
            borg_equips_ring(sv_ring_digging))
            return true;
    }

    return false;
}


/*
 * Take one "step" towards the given location, return true if possible
 */
static bool borg_play_step(int y2, int x2)
{
    borg_grid* ag;
    borg_grid* ag2;
    ui_event ch_evt = EVENT_EMPTY;
    int dir, x, y, ox, oy, i;

    int o_y = 0, o_x = 0, door_found = 0;

    /* Breeder levels, close all doors */
    if (breeder_level)
    {
        /* scan the adjacent grids */
        for (ox = -1; ox <= 1; ox++)
        {
            for (oy = -1; oy <= 1; oy++)
            {
                /* skip our own spot */
                if ((oy + c_y == c_y) && (ox + c_x == c_x)) continue;

                /* Acquire location */
                ag = &borg_grids[oy + c_y][ox + c_x];

                /* skip non open doors */
                if (ag->feat != FEAT_OPEN) continue;

                /* skip monster on door */
                if (ag->kill) continue;

                /* Skip repeatedly closed doors */
                if (track_door.num >= 255) continue;

                /* skip our orignal goal */
                if ((oy + c_y == y2) && (ox + c_x == x2)) continue;

                /* save this spot */
                o_y = oy;
                o_x = ox;
                door_found++;
            }
        }

        /* Is there a door to close? */
        if (door_found)
        {
            /* Get a direction, if possible */
            dir = borg_goto_dir(c_y, c_x, c_y + o_y, c_x + o_x);

            /* Obtain the destination */
            x = c_x + ddx[dir];
            y = c_y + ddy[dir];

            /* Hack -- set goal */
            g_x = x;
            g_y = y;

            /* Close */
            borg_note("# Closing a door");
            borg_keypress('c');
            borg_queue_direction(I2D(dir));

            /* Check for an existing flag */
            for (i = 0; i < track_door.num; i++)
            {
                /* Stop if we already new about this door */
                if ((track_door.x[i] == x) && (track_door.y[i] == y)) return (true);
            }

            /* Track the newly closed door */
            if (i == track_door.num && i < track_door.size)
            {

                borg_note("# Noting the closing of a door.");
                track_door.num++;
                track_door.x[i] = x;
                track_door.y[i] = y;
            }
            return (true);

        }
    }

    /* Stand stairs up */
    if (goal_less)
    {

        /* Define the grid we are looking at to be our own grid */
        ag = &borg_grids[c_y][c_x];

        /* Up stairs. Cheat the game grid info in. (cave_feat[c_y][c_x] == FEAT_LESS) */
        if (ag->feat == FEAT_LESS)
        {
            /* Stand on stairs */
            borg_on_dnstairs = true;
            goal_less = false;

            borg_keypress('<');

            /* Success */
            return (true);
        }
    }


    /* Get a direction, if possible */
    dir = borg_goto_dir(c_y, c_x, y2, x2);

    /* We have arrived */
    if (dir == 5) return (false);

    /* Obtain the destination */
    x = c_x + ddx[dir];
    y = c_y + ddy[dir];

    /* Access the grid we are stepping on */
    ag = &borg_grids[y][x];

    /* Hack -- set goal */
    g_x = x;
    g_y = y;


    /* Monsters -- Attack */
    if (ag->kill)
    {
        borg_kill* kill = &borg_kills[ag->kill];

        /* can't attack someone if afraid! */
        if (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR])
            return (false);

        /* Hack -- ignore Maggot until later.  */
        if ((rf_has(r_info[kill->r_idx].flags, RF_UNIQUE)) && borg_skill[BI_CDEPTH] == 0 &&
            borg_skill[BI_CLEVEL] < 5)
            return (false);

        /* Message */
        borg_note(format("# Walking into a '%s' at (%d,%d)",
            r_info[kill->r_idx].name,
            kill->y, kill->x));

        /* Walk into it */
        if (my_no_alter)
        {
            borg_keypress(';');
            my_no_alter = false;
        }
        else
        {
            borg_keypress('+');
        }
        borg_keypress(I2D(dir));
        return (true);
    }


    /* Objects -- Take */
    if (ag->take && borg_takes[ag->take].kind)
    {
        borg_take* take = &borg_takes[ag->take];

        /*** Handle Chests ***/
        /* The borg will cheat when it comes to chests.
         * He does not have to but it makes him faster and
         * it does not give him any information that a
         * person would not know by looking at the trap.
         * So there is no advantage to the borg.
         */

        if (strstr(take->kind->name, "chest") &&
            !strstr(take->kind->name, "Ruined"))
        {
            struct object* o_ptr = square_object(cave, loc(x2, y2));

            /* this should only happen when something picks up the chest */
            /* outside the borgs view.  */
            if (!o_ptr)
            {
                borg_delete_take(ag->take);
                return false;
            }

            /* Traps. Disarm it w/ fail check */
            if (o_ptr->pval > 1 && o_ptr->known &&
                borg_skill[BI_DEV] - o_ptr->pval >= borg_cfg[BORG_CHEST_FAIL_TOLERANCE])
            {
                borg_note(format("# Disarming a '%s' at (%d,%d)",
                    take->kind->name, take->y, take->x));

                /* Open it */
                borg_keypress('D');
                borg_queue_direction(I2D(dir));
                return (true);
            }


            /* No trap, or unknown trap that passed above checks - Open it */
            if (o_ptr->pval < 0 || !o_ptr->known)
            {
                borg_note(format("# Opening a '%s' at (%d,%d)",
                    take->kind->name,
                    take->y, take->x));

                /* Open it */
                borg_keypress('o');
                borg_queue_direction(I2D(dir));
                return (true);
            }

            /* Empty chest */
            /* continue in routine and pick it up */
        }


        /*** Handle Orb of Draining ***/

        /* Priest/Paladin borgs who have very limited ID ability can save some money and
         * inventory space my casting Orb of Draining on objects.  Cursed objects will melt
         * under the Orb of Draining spell.  This will save the borg from carrying the item
         * around until he can ID it.
         *
         * By default, the flag ORBED is set to false when an item is created.  If the borg
         * gets close to an item, and the conditions are favorable, he will cast OoD on the
         * item and change the flag.
         */
        if (take->orbed == false &&
            (take->tval >= TV_SHOT && take->tval < TV_STAFF))
        {
            if (borg_distance(take->y, take->x, c_y, c_x) == 1)
            {
                if (borg_spell_okay_fail(ORB_OF_DRAINING, 25))
                {
                    /* Target the Take location */
                    borg_target(take->y, take->x);

                    /* Cast the prayer */
                    borg_spell(ORB_OF_DRAINING);

                    /* Message */
                    borg_note("# Orbing an object to check for cursed item.");

                    /* use the old target */
                    borg_keypress('5');

                    /* Change the take flag */
                    take->orbed = true;

                    /* check the blast radius of the prayer for other items */
                    for (i = 0; i < 24; i++)
                    {
                        /* Extract the location */
                        int xx = take->x + borg_ddx_ddd[i];
                        int yy = take->y + borg_ddy_ddd[i];

                        /* Check the grid for a take */
                        if (!square_in_bounds_fully(cave, loc(xx, yy))) continue;
                        ag2 = &borg_grids[yy][xx];
                        if (ag2->take)
                        {
                            /* This item was orbed (mostly true)*/
                            borg_takes[borg_grids[yy][xx].take].orbed = true;
                        }

                    }

                    /* Return */
                    return (true);
                }
            }
        }

        /*** Handle other takes ***/
        /* Message */
        borg_note(format("# Walking onto and deleting a '%s' at (%d,%d)",
            take->kind->name,
            take->y, take->x));

        /* Delete the item from the list */
        borg_delete_take(ag->take);

        /* Walk onto it */
        borg_keypress(I2D(dir));


        return (true);
    }


    /* Glyph of Warding */
    if (ag->glyph)
    {
        /* Message */
        borg_note(format("# Walking onto a glyph of warding."));

        /* Walk onto it */
        borg_keypress(I2D(dir));
        return (true);
    }


    /* Traps -- disarm -- */
    if (borg_skill[BI_CURLITE] &&
        !borg_skill[BI_ISBLIND] &&
        !borg_skill[BI_ISCONFUSED] &&
        !scaryguy_on_level &&
        ag->trap)
    {

        /* NOTE: If a scary guy is on the level, we allow the borg to run over the
         * trap in order to escape this level.
        */

        /* allow "destroy doors" */
        /* don't bother unless we are near full mana */
        if (borg_skill[BI_CURSP] > ((borg_skill[BI_MAXSP] * 4) / 5))
        {
            if (borg_spell(DISABLE_TRAPS_DESTROY_DOORS))
            {
                borg_note("# Disable Traps, Destroy Doors");
                ag->trap = 0;
                /* since this just disables the trap and doesn't remove it, */
                /* don't rest next to it */
                borg_no_rest_prep = 3000;
                return (true);
            }
        }

        /* Disarm */
        borg_note("# Disarming a trap");
        borg_keypress('D');
        borg_queue_direction(I2D(dir));

        /* We are not sure if the trap will get 'untrapped'. pretend it will*/
        ag->trap = 0;
        return (true);
    }


    /* Closed Doors -- Open */
    if (ag->feat == FEAT_CLOSED)
    {
        /* Paranoia XXX XXX XXX */
        if (!randint0(100)) return (false);

        /* Not a good idea to open locked doors if a monster
         * is next to the borg beating on him
         */

         /* scan the adjacent grids */
        for (i = 0; i < 8; i++)
        {
            /* Grid in that direction */
            x = c_x + ddx_ddd[i];
            y = c_y + ddy_ddd[i];

            /* Access the grid */
            ag2 = &borg_grids[y][x];

            /* If monster adjacent to me and I'm weak, dont
             * even try to open the door
             */
            if (ag2->kill && borg_skill[BI_CLEVEL] < 15 && !borg_skill[BI_ISAFRAID]) return (false);
        }

        /* Use other techniques from time to time */
        if (!randint0(100) || time_this_panel >= 500)
        {
            /* Mega-Hack -- allow "destroy doors" */
            if (borg_spell(DISABLE_TRAPS_DESTROY_DOORS))
            {
                borg_note("# Disable Traps, Destroy Doors");
                return (true);
            }

            /* Mega-Hack -- allow "stone to mud" */
            if (borg_spell(TURN_STONE_TO_MUD) ||
                borg_spell(SHATTER_STONE) ||
                borg_activate_ring(sv_ring_digging) ||
                borg_activate_item(act_stone_to_mud))
            {
                borg_note("# Melting a door");
                borg_keypress(I2D(dir));

                /* Remove this closed door from the list.
                * Its faster to clear all doors from the list
                * then rebuild the list.
                */
                if (track_closed.num)
                {
                    track_closed.num = 0;
                }
                return (true);
            }
        }

        /* Open */
        if (my_need_alter)
        {
            borg_keypress('+');
            my_need_alter = false;
        }
        else
        {
            borg_note("# Opening a door");
            borg_keypress('o');
        }
        borg_queue_direction(I2D(dir));

        /* Remove this closed door from the list.
         * Its faster to clear all doors from the list
         * then rebuild the list.
         */
        if (track_closed.num)
        {
            track_closed.num = 0;
        }

        return (true);
    }

    /* Rubble, Treasure, Seams, Walls -- Tunnel or Melt */
    /* HACK depends on FEAT order, kinda evil. */
    if (ag->feat >= FEAT_SECRET && ag->feat <= FEAT_GRANITE)
    {
        /* Don't dig walls and seams when exploring (do dig rubble) */
        if (ag->feat != FEAT_RUBBLE && goal == GOAL_DARK) return false;

        /* Don't bother digging without sufficient dig ability */
        if (!borg_can_dig(false, ag->feat == FEAT_GRANITE))
        {
            goal = 0;
            return false;
        }

        /* Use Stone to Mud when available */
        if (borg_spell(TURN_STONE_TO_MUD) ||
            borg_spell(SHATTER_STONE) ||
            borg_activate_ring(sv_ring_digging) ||
            borg_activate_item(act_stone_to_mud)) {
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
    if (feat_is_shop(ag->feat))
    {
        /* Message */
        borg_note(format("# Entering a '%d' shop", ag->store));

        /* Enter the shop */
        borg_keypress(I2D(dir));
        return (true);
    }

    /* Walk in that direction */
    if (my_need_alter)
    {
        borg_keypress('+');
        my_need_alter = false;
    }
    else
    {
        /* nothing */
    }

    /* Actually enter the direction */
    borg_keypress(I2D(dir));

    /* I'm not in a store */
    borg_in_shop = false;

    /* for some reason, selling and buying in the store sets the event handler to Select.
     * This is a game bug not a borg bug.  The borg is trying to overcome the game bug.
     * But he still has some troubles unhooking in town after shopping.  Again, this is
     * due to the event handler.  The handler should release the EVT_SELECT but it does not.
     */

    if (ch_evt.type & EVT_SELECT) ch_evt.type = EVT_KBRD;
    if (ch_evt.type & EVT_MOVE) ch_evt.type = EVT_KBRD;

    /* Did something */
    return (true);
}




/*
 * Act twitchy
 */
bool borg_twitchy(void)
{
    int dir = 5;
    int count;

    /* This is a bad thing */
    borg_note("# Twitchy!");

    /* try to phase out of it */
    if (borg_allow_teleport())
    {
        if (borg_caution_phase(15, 2) &&
            (borg_spell_fail(PHASE_DOOR, 40) ||
                borg_spell_fail(PORTAL, 40) ||
                borg_shadow_shift(40) ||
                borg_activate_item(act_tele_phase) ||
                borg_activate_item(act_tele_long) ||
                borg_read_scroll(sv_scroll_phase_door)))
        {
            /* We did something */
            return (true);
        }
    }

    /* Pick a random direction */
    count = 100;
    while (true)
    {
        dir = randint0(9);
        if (dir == 5 || dir == 0)
            continue;
        if (!(count--))
            break;
        /* Hack -- set goal */
        g_x = c_x + ddx[dir];
        g_y = c_y + ddy[dir];

        if (!square_in_bounds_fully(cave, loc(g_x, g_y))) 
            continue;

        if (borg_grids[g_y][g_x].feat >= FEAT_SECRET &&
            borg_grids[g_y][g_x].feat <= FEAT_PERM)
            continue;
        break;
    }
    if (!count)
    {
        bool all_walls = true;
        for (dir = 1; dir < 10; dir++)
        {
            if (dir == 5)
                continue;

            if (!square_in_bounds_fully(cave, loc(g_x, g_y)))
                continue;

            if (borg_grids[g_y][g_x].feat >= FEAT_SECRET &&
                borg_grids[g_y][g_x].feat <= FEAT_PERM)
                continue;
            all_walls = false;
            break;
        }
        if (all_walls)
        {
            /* Rest until done */
            borg_keypress('R');
            borg_keypress('1');
            borg_keypress('0');
            borg_keypress('0');
            borg_keypress(KC_ENTER);
            /* We did something */
            return (true);
        }
    }

    /* Normally move */
    /* Send direction */
    borg_keypress(I2D(dir));

    /* We did something */
    return (true);
}





/*
 * Commit the current "flow"
 */
static bool borg_flow_commit(const char* who, int why)
{
    int cost;

    /* Cost of current grid */
    cost = borg_data_cost->data[c_y][c_x];

    /* Verify the total "cost" */
    if (cost >= 250) return (false);

    /* Message */
    if (who) borg_note(format("# Flowing toward %s at cost %d", who, cost));

    /* Obtain the "flow" information */
    memcpy(borg_data_flow, borg_data_cost, sizeof(borg_data));

    /* Save the goal type */
    goal = why;

    /* Success */
    return (true);
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
    if (goal == why)
    {
        int b_n = 0;

        int i, b_i = -1;

        int c, b_c;


        /* Flow cost of current grid */
        b_c = borg_data_flow->data[c_y][c_x] * 10;

        /* Prevent loops */
        b_c = b_c - 5;


        /* Look around */
        for (i = 0; i < 8; i++)
        {
            /* Grid in that direction */
            x = c_x + ddx_ddd[i];
            y = c_y + ddy_ddd[i];

            /* Flow cost at that grid */
            c = borg_data_flow->data[y][x] * 10;

            /* Never backtrack */
            if (c > b_c) continue;

            /* avoid screen edgeds */
            if (x > AUTO_MAX_X - 1 ||
                x < 1 ||
                y > AUTO_MAX_Y - 1 ||
                y < 1)
                continue;


            /* Notice new best value */
            if (c < b_c) b_n = 0;

            /* Apply the randomizer to equivalent values */
            if (borg_skill[BI_CDEPTH] == 0 && (++b_n >= 2) && (randint0(b_n) != 0)) continue;
            else if (borg_skill[BI_CDEPTH] >= 1 && ++b_n >= 2) continue;

            /* Special case when digging anti-summon corridor */
            if (goal == GOAL_DIGGING && (ddx_ddd[i] == 0 || ddy_ddd[i] == 0))
            {
                /* No straight lines */
                if (borg_distance(c_y, c_x, borg_flow_y[0], borg_flow_x[0]) <= 2) continue;
            }

            /* Track it */
            b_i = i; b_c = c;
        }

        /* Try it */
        if (b_i >= 0)
        {
            /* Access the location */
            x = c_x + ddx_ddd[b_i];
            y = c_y + ddy_ddd[b_i];

            /* Attempt motion */
            if (borg_play_step(y, x)) return (true);
        }

        /* Mark a timestamp to wait on a anti-summon spot for a few turns */
        if (goal == GOAL_DIGGING && c_y == borg_flow_y[0] && c_x == borg_flow_x[0])
            borg_t_antisummon = borg_t;

        /* Cancel goal */
        goal = 0;
    }

    /* Nothing to do */
    return (false);
}




/*
 * Prepare to flee the level via stairs
 */
bool borg_flow_stair_both(int why, bool sneak)
{
    int i;

    /* None to flow to */
    if (!track_less.num && !track_more.num) return (false);


    /* dont go down if hungry or low on food, unless fleeing a scary town */
    if (!goal_fleeing && !scaryguy_on_level && !track_less.num && (avoidance <= borg_skill[BI_CURHP] * 15 / 10) &&
        (borg_skill[BI_ISWEAK] ||
            borg_skill[BI_ISHUNGRY] || borg_skill[BI_FOOD] < 2))
        return (false);

    /* Absolutely no diving if no light */
    if (borg_skill[BI_CURLITE] == 0 && borg_skill[BI_CDEPTH] != 0 && borg_munchkin_mode == false) return (false);

    /* clear the possible searching flag */
    borg_needs_searching = false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_less.num; i++)
    {
        /* Not if a monster is parked on the stiar */
        if (borg_grids[track_less.y[i]][track_less.x[i]].kill) continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_less.y[i], track_less.x[i]);
    }

    /* Enqueue useful grids */
    for (i = 0; i < track_more.num; i++)
    {
        /* Not if a monster is parked on the stiar */
        if (borg_grids[track_more.y[i]][track_more.x[i]].kill) continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_more.y[i], track_more.x[i]);
    }


    /* Spread the flow */
    borg_flow_spread(250, false, false, false, -1, sneak);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("stairs", why)) return (false);


    /* Take one step */
    if (!borg_flow_old(why)) return (false);

    /* Success */
    return (true);
}




/*
 * Prepare to flow towards "up" stairs
 */
bool borg_flow_stair_less(int why, bool sneak)
{
    int i;

    /* None to flow to */
    if (!track_less.num) return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* clear the possible searching flag */
    borg_needs_searching = false;

    /* Enqueue useful grids */
    for (i = 0; i < track_less.num; i++)
    {
        /* Not if a monster is parked on the stiar */
        if (borg_grids[track_less.y[i]][track_less.x[i]].kill) continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_less.y[i], track_less.x[i]);
    }

    if (borg_skill[BI_CLEVEL] > 35 || borg_skill[BI_CURLITE] == 0)
    {
        /* Spread the flow */
        borg_flow_spread(250, true, false, false, -1, sneak);
    }
    else
    {
        /* Spread the flow, No Optimize, Avoid */
        borg_flow_spread(250, false, !borg_desperate, false, -1, sneak);
    }

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("up-stairs", why)) return (false);

    /* Take one step */
    if (!borg_flow_old(why)) return (false);

    /* Success */
    return (true);
}


/*
 * Prepare to flow towards "down" stairs
 */
bool borg_flow_stair_more(int why, bool sneak, bool brave)
{
    int i;

    /* None to flow to */
    if (!track_more.num) return (false);

    /* not unless safe or munchkin/Lunal Mode or brave */
    if (!borg_lunal_mode && !borg_munchkin_mode && !brave && (char*)NULL != borg_prepared(borg_skill[BI_CDEPTH] + 1))
        return (false);

    /* dont go down if hungry or low on food, unless fleeing a scary town */
    if (!brave && borg_skill[BI_CDEPTH] && !scaryguy_on_level &&
        (borg_skill[BI_ISWEAK] || borg_skill[BI_ISHUNGRY] ||
            borg_skill[BI_FOOD] < 2))
        return (false);

    /* If I need to sell crap, then don't go down */
    if (borg_skill[BI_CDEPTH] && borg_skill[BI_CLEVEL] < 25 && borg_gold < 25000 &&
        borg_count_sell() >= 13 && !borg_munchkin_mode) return (false);


    /* No diving if no light */
    if (borg_skill[BI_CURLITE] == 0 && borg_munchkin_mode == false) return (false);

    /* don't head for the stairs if you are recalling,  */
    /* even if you are fleeing. */
    if (goal_recalling)
        return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_more.num; i++)
    {
        /* Not if a monster is parked on the stiar */
        if (borg_grids[track_more.y[i]][track_more.x[i]].kill) continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_more.y[i], track_more.x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, sneak);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("down-stairs", why)) return (false);

    /* Take one step */
    if (!borg_flow_old(why)) return (false);

    /* Success */
    return (true);
}

/*
 * Hack -- Glyph creating
 */

static uint8_t glyph_x;
static uint8_t glyph_y;
static uint8_t glyph_y_center = 0;
static uint8_t glyph_x_center = 0;

/*
 * Prepare to flow towards a location and create a
 * special glyph of warding pattern.
 *
 * The borg will look for a room that is at least 7x7.
 * ##########
 * #3.......#
 * #2.xxxxx.#
 * #1.xxxxx.#
 * #0.xx@xx.#
 * #1.xxxxx.#
 * #2.xxxxx.#
 * #3.......#
 * # 3210123#
 * ##########
 * and when he locates one, he will attempt to:
 * 1. flow to a central location and
 * 2. begin planting Runes in a pattern. When complete,
 * 3. move to the center of it.
 */
 /*
  * ghijk  The borg will use the following ddx and ddy to search
  * d827a  for a suitable grid in an open room.
  * e4@3b
  * f615c
  * lmnop  24 grids
  *
  */
bool borg_flow_glyph(int why)
{
    int i;
    int cost;

    int x, y;
    int v = 0;

    int b_x = c_x;
    int b_y = c_y;
    int b_v = -1;
    int goal_glyph = 0;
    int glyph = 0;

    borg_grid* ag;

    if ((glyph_y_center == 0 && glyph_x_center == 0) ||
        borg_distance(c_y, c_x, glyph_y_center, glyph_x_center) >= 50)
    {
        borg_needs_new_sea = true;
    }

    /* We have arrived */
    if ((glyph_x == c_x) && (glyph_y == c_y))
    {
        /* Cancel */
        glyph_x = 0;
        glyph_y = 0;

        /* Store the center of the glyphs */
        if (borg_needs_new_sea)
        {
            glyph_y_center = c_y;
            glyph_x_center = c_x;
        }

        borg_needs_new_sea = false;

        /* Take note */
        borg_note(format("# Glyph Creating at (%d,%d)", c_x, c_y));

        /* Create the Glyph */
        if (borg_spell_fail(GLYPH_OF_WARDING, 30) ||
            borg_read_scroll(sv_scroll_rune_of_protection))
        {
            /* Check for an existing glyph */
            for (i = 0; i < track_glyph.num; i++)
            {
                /* Stop if we already new about this glyph */
                if ((track_glyph.x[i] == c_x) && (track_glyph.y[i] == c_y)) return (false);
            }

            /* Track the newly discovered glyph */
            if (track_glyph.num < track_glyph.size)
            {
                borg_note("# Noting the creation of a glyph.");
                track_glyph.x[track_glyph.num] = c_x;
                track_glyph.y[track_glyph.num] = c_y;
                track_glyph.num++;
            }

            /* Success */
            return (true);
        }

        /* Nope */
        return (false);
    }

    /* Reverse flow */
    borg_flow_reverse(250, true, false, false, -1, false);

    /* Scan the entire map */
    for (y = 15; y < AUTO_MAX_Y - 15; y++)
    {
        for (x = 50; x < AUTO_MAX_X - 50; x++)
        {
            borg_grid* ag_ptr[24];

            int floor = 0;
            int tmp_glyph = 0;


            /* Acquire the grid */
            ag = &borg_grids[y][x];

            /* Skip every non floor/glyph */
            if (ag->feat != FEAT_FLOOR && ag->glyph) continue;

            /* Acquire the cost */
            cost = borg_data_cost->data[y][x];

            /* Skip grids that are really far away.  He probably
             * won't be able to safely get there
             */
            if (cost >= 75) continue;

            /* Extract adjacent locations to each considered grid */
            for (i = 0; i < 24; i++)
            {
                /* Extract the location */
                int xx = x + borg_ddx_ddd[i];
                int yy = y + borg_ddy_ddd[i];

                /* Get the grid contents */
                ag_ptr[i] = &borg_grids[yy][xx];
            }

            /* Center Grid */
            if (borg_needs_new_sea)
            {
                goal_glyph = 24;

                /* Count Adjacent Flooors */
                for (i = 0; i < 24; i++)
                {
                    ag = ag_ptr[i];
                    if (ag->feat == FEAT_FLOOR || ag->glyph) floor++;
                }

                /* Not a good location if not the center of the sea */
                if (floor != 24)
                {
                    continue;
                }

                /* Count floors already glyphed */
                for (i = 0; i < 24; i++)
                {
                    ag = ag_ptr[i];

                    /* Glyphs */
                    if (ag->glyph)
                    {
                        tmp_glyph++;
                    }
                }

                /* Tweak -- Reward certain floors, punish distance */
                v = 100 + (tmp_glyph * 500) - (cost * 1);
                if (borg_grids[y][x].feat == FEAT_FLOOR) v += 3000;

                /* If this grid is surrounded by glyphs, select it */
                if (tmp_glyph == goal_glyph) v += 5000;

                /* If this grid is already glyphed but not
                 * surrounded by glyphs, then choose another.
                 */
                if (tmp_glyph != goal_glyph && borg_grids[y][x].glyph)
                    v = -1;

                /* The grid is not searchable */
                if (v <= 0) continue;

                /* Track "best" grid */
                if ((b_v >= 0) && (v < b_v)) continue;

                /* Save the data */
                b_v = v; b_x = x; b_y = y;
            }
            /* old center, making outlying glyphs, */
            else
            {
                /* Count Adjacent Flooors */
                for (i = 0; i < 24; i++)
                {
                    /* Leave if this grid is not in good array */
                    if (glyph_x_center + borg_ddx_ddd[i] != x) continue;
                    if (glyph_y_center + borg_ddy_ddd[i] != y) continue;

                    /* Already got a glyph on it */
                    if (borg_grids[y][x].glyph) continue;

                    /* Tweak -- Reward certain floors, punish distance */
                    v = 500 + (tmp_glyph * 500) - (cost * 1);

                    /* The grid is not searchable */
                    if (v <= 0) continue;

                    /* Track "best" grid */
                    if ((b_v >= 0) && (v < b_v)) continue;

                    /* Save the data */
                    b_v = v; b_x = x; b_y = y;
                }
            }
        }
    }

    /* Extract adjacent locations to each considered grid */
    if (glyph_y_center != 0 &&
        glyph_x_center != 0)
    {

        for (i = 0; i < 24; i++)
        {
            /* Extract the location */
            int xx = glyph_x_center + borg_ddx_ddd[i];
            int yy = glyph_y_center + borg_ddy_ddd[i];

            borg_grid* ag_ptr[24];

            /* Get the grid contents */
            ag_ptr[i] = &borg_grids[yy][xx];
            ag = ag_ptr[i];

            /* If it is not a glyph, skip it */
            if (ag->glyph) glyph++;

            /* Save the data */
            if (glyph == 24)
            {
                b_v = 5000; b_x = glyph_x_center; b_y = glyph_y_center;
            }
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (b_v < 0) return (false);


    /* Access grid */
    ag = &borg_grids[b_y][b_x];

    /* Memorize */
    glyph_x = b_x;
    glyph_y = b_y;

    /* Enqueue the grid */
    borg_flow_enqueue_grid(b_y, b_x);

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("Glyph", GOAL_MISC)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_MISC)) return (false);

    /* Success */
    return (true);
}


/*
 * Prepare to flow towards light
 */
bool borg_flow_light(int why)
{
    int y, x, i;


    /* reset counters */
    borg_glow_n = 0;
    i = 0;

    /* build the glow array */
    /* Scan map */
    for (y = w_y; y < w_y + SCREEN_HGT; y++)
    {
        for (x = w_x; x < w_x + SCREEN_WID; x++)
        {
            borg_grid* ag = &borg_grids[y][x];

            /* Not a perma-lit, and not our spot. */
            if (!(ag->info & BORG_GLOW)) continue;

            /* keep count */
            borg_glow_y[borg_glow_n] = y;
            borg_glow_x[borg_glow_n] = x;
            borg_glow_n++;

        }
    }
    /* None to flow to */
    if (!borg_glow_n) return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_glow_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_glow_y[i], borg_glow_x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("a lighted area", why)) return (false);

    /* Take one step */
    if (!borg_flow_old(why)) return (false);

    /* Success */
    return (true);
}

/*
 * Prepare to flow towards a vault grid which can be excavated
 */
bool borg_flow_vault(int nearness)
{
    int y, x, i;
    int b_y, b_x;
    bool can_dig_hard;

    borg_grid* ag;

    /* reset counters */
    borg_temp_n = 0;
    i = 0;

    /* no need if no vault on level */
    if (!vault_on_level) return (false);

    /* no need if we can't dig */
    if (!borg_can_dig(false, false)) return (false);

    can_dig_hard = borg_can_dig(false, true);

    /* build the array -- Scan screen */
    for (y = w_y; y < w_y + SCREEN_HGT; y++)
    {
        for (x = w_x; x < w_x + SCREEN_WID; x++)
        {

            /* only bother with near ones */
            if (borg_distance(c_y, c_x, y, x) > nearness) continue;

            /* only deal with excavatable walls */
            if (can_dig_hard)
            {
                if (borg_grids[y][x].feat != FEAT_FLOOR &&
                    borg_grids[y][x].feat != FEAT_LAVA &&
                    borg_grids[y][x].feat != FEAT_GRANITE &&
                    borg_grids[y][x].feat != FEAT_RUBBLE &&
                    borg_grids[y][x].feat != FEAT_QUARTZ &&
                    borg_grids[y][x].feat != FEAT_MAGMA &&
                    borg_grids[y][x].feat != FEAT_QUARTZ_K &&
                    borg_grids[y][x].feat != FEAT_MAGMA_K)
                    continue;
            }
            else
            {
                if (borg_grids[y][x].feat != FEAT_FLOOR &&
                    borg_grids[y][x].feat != FEAT_LAVA &&
                    borg_grids[y][x].feat != FEAT_RUBBLE &&
                    borg_grids[y][x].feat != FEAT_QUARTZ_K &&
                    borg_grids[y][x].feat != FEAT_MAGMA_K)
                    continue;
            }

            /* Examine grids adjacent to this grid to see if there is a perma wall adjacent */
            for (i = 0; i < 8; i++)
            {
                b_x = x + ddx_ddd[i];
                b_y = y + ddy_ddd[i];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(b_x, b_y))) continue;

                /* Access the grid */
                ag = &borg_grids[b_y][b_x];

                /* Not a perma, and not our spot. */
                if (ag->feat != FEAT_PERM) continue;

                /* keep count */
                borg_temp_y[borg_temp_n] = y;
                borg_temp_x[borg_temp_n] = x;
                borg_temp_n++;
            }

        }
    }

    /* None to flow to */
    if (!borg_temp_n) return (false);

    /* Examine each ones */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("vault excavation", GOAL_VAULT)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_VAULT)) return (false);

    /* Success */
    return (true);
}

/* Excavate an existing vault using ranged spells.
 * Stand where you are, use stone to mud to excavate the vault.  This will allow the druid
 * or blackgaurd borgs to get a few more attack spells on the monster.  Without this routine, 
 * he would approach the vault and use Stone to Mud when he was adjacent to the wall, giving him
 * only 1 or 2 shots before the monster is next to the borg.
 *
 */
bool borg_excavate_vault(int range)
{
    int y, x, i, ii;
    int b_y, b_x;

    borg_grid* ag;

    /* reset counters */
    borg_temp_n = 0;
    i = 0;
    ii = 0;

    /* no need if no vault on level */
    if (!vault_on_level) return (false);

    /* only if you can cast the spell */
    if (!borg_spell_okay_fail(TURN_STONE_TO_MUD, 30) && !borg_spell_okay_fail(SHATTER_STONE, 30)) return (false);

    /* Danger/bad idea checks */

    /* build the array -- Scan screen */
    for (y = w_y; y < w_y + SCREEN_HGT; y++)
    {
        for (x = w_x; x < w_x + SCREEN_WID; x++)
        {

            /* only bother with near ones */
            if (borg_distance(c_y, c_x, y, x) > range) continue;

            /* only deal with excavatable walls */
            if (borg_grids[y][x].feat != FEAT_FLOOR &&
                borg_grids[y][x].feat != FEAT_LAVA &&
                borg_grids[y][x].feat != FEAT_GRANITE &&
                borg_grids[y][x].feat != FEAT_RUBBLE &&
                borg_grids[y][x].feat != FEAT_QUARTZ &&
                borg_grids[y][x].feat != FEAT_MAGMA &&
                borg_grids[y][x].feat != FEAT_QUARTZ_K &&
                borg_grids[y][x].feat != FEAT_MAGMA_K)
                continue;

            /* Examine grids adjacent to this grid to see if there is a perma wall adjacent */
            for (i = 0; i < 8; i++)
            {
                b_x = x + ddx_ddd[i];
                b_y = y + ddy_ddd[i];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(b_x, b_y))) continue;

                ag = &borg_grids[b_y][b_x];

                /* Not a perma, and not our spot. */
                if (ag->feat != FEAT_PERM) continue;

                /* Track the new grid */
                for (ii = 0; ii < borg_temp_n; ii++)
                {
                    if (borg_temp_y[ii] == y &&
                        borg_temp_x[ii] == x) break;
                }

                /* Track the newly discovered excavatable wall */
                if ((ii == borg_temp_n) && (ii < AUTO_TEMP_MAX))
                {
                    borg_temp_x[ii] = x;
                    borg_temp_y[ii] = y;
                    borg_temp_n++;

                    /* do not overflow */
                    if (borg_temp_n > AUTO_TEMP_MAX) borg_temp_n = AUTO_TEMP_MAX;
                }

            }
        }
    }

    /* None to excavate */
    if (!borg_temp_n) return (false);

    /* Review the useful grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* skip non-projectable grids grid (I cant shoot them) */
        if (!borg_los(c_y, c_x, borg_temp_y[i], borg_temp_x[i])) continue;

        /* Attempt to target the grid */
        borg_target(borg_temp_y[i], borg_temp_x[i]);

        /* Attempt to excavate it with "stone to mud" */
        if (borg_spell(TURN_STONE_TO_MUD) ||
            borg_spell(SHATTER_STONE) ||
            borg_activate_ring(sv_ring_digging) ||
            borg_activate_item(act_stone_to_mud))
        {
            borg_note("# Excavation of vault");
            borg_keypress('5');

            /* turn that wall into a floor grid.  If the spell failed, it will still look
             * like a wall and the borg_update routine will redefine it as a wall
             */
            borg_do_update_view = true;
            borg_do_update_lite = true;

            /* Not Lit */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].info &= ~BORG_GLOW;
            /* Dark */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].info |= BORG_GLOW;
            /* Feat Floor */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].feat = FEAT_FLOOR;



            return (true);
        }

        /* Success */
        return (true);
    }

    /* No grid to excavate */
    return (false);
}


/*
 * Prepare to "flow" towards any non-visited shop
 */
bool borg_flow_shop_visit(void)
{
    /* Borg is allowed to cheat the store inventory as of 320.  No need to visit each one */
    return (false);
#if 0
    int i, x, y;

    /* Must be in town */
    if (borg_skill[BI_CDEPTH]) return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Visit the shops */
    for (i = 0; i < MAX_STORES; i++)
    {
        /* If low Level skip certain buildings in town
         * in order to reduce time spent in town.
         */
        if (borg_skill[BI_CLEVEL] <= 10)
        {
            /* Skip Magic Shop unless Mage */
            if (i == 5 &&
                (borg_class != CLASS_MAGE))
            {
                borg_shops[i].when = borg_t;
                continue;
            }

            /* Skip Black Market */
            if (i == 6)
            {
                borg_shops[i].when = borg_t;
                continue;
            }

            /* Skip Home */
            if (i == 7)
            {
                borg_shops[i].when = borg_t;
                continue;
            }
        }
        /* Must not be visited */
        if (borg_shops[i].when) continue;

        /* if poisoned or bleeding skip non temples */
        if ((borg_skill[BI_ISCUT] || borg_skill[BI_ISPOISONED]) &&
            (i != 3 && i != 7)) continue;

        /* if starving--skip non food places */
        if (borg_skill[BI_FOOD] == 0 &&
            (i != 0 && i != 7)) continue;

        /* if dark--skip non food places */
        if (borg_skill[BI_CURLITE] == 0 && (i != 0) && borg_skill[BI_CLEVEL] >= 2) continue;

        /* if only torch-- go directly to Gen Store --Get a Lantern */
        if (borg_skill[BI_CURLITE] == 1 && i != 0 &&
            /* !borg_shops[0].when && */ borg_gold >= 75) continue;

        /* Obtain the location */
        x = track_shop_x[i];
        y = track_shop_y[i];

        /* Hack -- Must be known and not under the player */
        if (!x || !y || ((c_x == x) && (c_y == y))) continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("un-visited shops", GOAL_MISC)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_MISC)) return (false);

    /* Success */
    return (true);
#endif

}


/*
 * Prepare to "flow" towards a specific shop entry
 */
bool borg_flow_shop_entry(int i)
{
    int x, y;

    const char* name = (f_info[stores[i].feat].name);

    /* Must be in town */
    if (borg_skill[BI_CDEPTH]) return (false);

    /* Obtain the location */
    x = track_shop_x[i];
    y = track_shop_y[i];

    /* Hack -- Must be known */
    if (!x || !y) return (false);

    /* Hack -- re-enter a shop if needed */
    if ((x == c_x) && (y == c_y))
    {
        /* Note */
        borg_note("# Re-entering a shop");

        /* Enter the store */
        borg_keypress('5');

        /* Success */
        return (true);
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue the grid */
    borg_flow_enqueue_grid(y, x);

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit(name, GOAL_MISC)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_MISC)) return (false);

    /* Success */
    return (true);
}

/*
 * The borg can take a shot from a distance
 *
 */
static bool borg_has_distance_attack(void)
{

    /* line up Magic Missle shots (covers Mages) */
    if (borg_attack_aux_spell_bolt(MAGIC_MISSILE, 0, 10, BORG_ATTACK_MISSILE, z_info->max_range))
        return true;

    /* line up Nether Bolt shots (covers Necromancers) */
    if (borg_attack_aux_spell_bolt(NETHER_BOLT, 0, 10, BORG_ATTACK_NETHER, z_info->max_range))
        return true;

    /* or arrows (covers warrior/ranger/paladins/rogues) */
    if (borg_attack_aux_launch() > 0)
        return true;

    /* not lining up Priests (OOD has area of effect, will line up more naturally) */
    /* or Druids (Stinking cloud is area of effect again) */
    /* Blackguards should be doing HTH */

    return false;
}


/*
 * Take a couple of steps to line up a shot
 *
 */
bool borg_flow_kill_aim(bool viewable)
{
    int o_y, o_x;
    int s_c_y = c_y;
    int s_c_x = c_x;
    int i;

    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt) return (false);

    /* Sometimes we loop on this if we back  up to a point where */
    /* the monster is out of site */
    if (time_this_panel > 500) return (false);

    /* Not if Weak from hunger or no food */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] == 0) return (false);

    /* If you can shoot from where you are, don't bother reaiming */
    if (borg_has_distance_attack()) return (false);

    /* Consider each adjacent spot */
    for (o_x = -2; o_x <= 2; o_x++)
    {
        for (o_y = -2; o_y <= 2; o_y++)
        {
            /* borg_attack would have already checked
               for a shot from where I currently am */
            if (o_x == 0 && o_y == 0)
                continue;

            /* XXX  Mess with where the program thinks the
               player is */
            c_x = s_c_x + o_x;
            c_y = s_c_y + o_y;

            /* avoid screen edgeds */
            if (c_x > AUTO_MAX_X - 2 ||
                c_x < 2 ||
                c_y > AUTO_MAX_Y - 2 ||
                c_y < 2)
                continue;

            /* Make sure we do not end up next to a monster */
            for (i = 0; i < borg_kills_nxt; i++)
            {
                if (borg_distance(c_y, c_x,
                    borg_kills[i].y, borg_kills[i].x) == 1)
                    break;
            }
            if (i != borg_kills_nxt)
                continue;

            /* Check for a distance attack from here */
            if (borg_has_distance_attack())
            {
                /* Clear the flow codes */
                borg_flow_clear();

                /* Enqueue the grid */
                borg_flow_enqueue_grid(c_y, c_x);

                /* restore the saved player position */
                c_x = s_c_x;
                c_y = s_c_y;

                /* Spread the flow */
                borg_flow_spread(5, true, !viewable, false, -1, false);

                /* Attempt to Commit the flow */
                if (!borg_flow_commit("targetable position", GOAL_KILL)) return (false);

                /* Take one step */
                if (!borg_flow_old(GOAL_KILL)) return (false);

                return (true);
            }
        }
    }

    /* restore the saved player position */
    c_x = s_c_x;
    c_y = s_c_y;

    return false;
}
/*
 * Dig an anti-summon corridor. Type I
 *
 *            ############## We want the borg to dig a tunnel which
 *            #............# limits the LOS of summoned monsters.
 *          ###............# It works better in hallways.
 *         ##@#............#
 *         #p##............# The borg will build an array of grids
 * ########## #######+###### near him.  Then look at specific patterns
 * #                  #      to find the good grids to excavate.
 * # ################ #
 *   #              # #
 * ###              # #
 *
 * Look at wall array to see if it is acceptable
 * We want to find this in the array:
 *
 * #####  ..@..  ####.  .####
 * ##.##  ##.##	 ##.#.  .#.##
 * #.#.#  #.#.#  #.#.@  @.#.#
 * ##.##  ##.##  ##.#.  .#.##
 * ..@..  #####  ####.  .####
 *
 * NORTH  SOUTH  WEST   East
 *
 */
bool borg_flow_kill_corridor_1(bool viewable)
{
    int o_y = 0;
    int o_x = 0;
    int m_x = 0;
    int m_y = 0;
    int b_y = 0, b_x = 0;
    int b_distance = 99;

    int i;
    bool b_n = false;
    bool b_s = false;
    bool b_e = false;
    bool b_w = false;

    int n_array[25] = { 1,0,0,0,1,
                       1,0,1,0,1,
                       0,1,0,1,0,
                       0,0,1,0,0,
                       1,1,1,1,1 };
    int ny[25] = { -4,-4,-4,-4,-4,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0 };
    int nx[25] = { -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2 };

    int s_array[25] = { 1,1,1,1,1,
                       0,0,1,0,0,
                       0,1,0,1,0,
                       1,0,1,0,1,
                       1,0,0,0,1 };
    int sy[25] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4 };
    int sx[25] = { -2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2 };

    int e_array[25] = { 1,0,0,1,1,
                       1,0,1,0,0,
                       1,1,0,1,0,
                       1,0,1,0,0,
                       1,0,0,1,1 };
    int ey[25] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2 };
    int ex[25] = { 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4 };

    int w_array[25] = { 1,1,0,0,1,
                       0,0,1,0,1,
                       0,1,0,1,1,
                       0,0,1,0,1,
                       1,1,0,0,1 };
    int wy[25] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2 };
    int wx[25] = { -4,-3,-2,-1, 0,-4,-3,-2,-1, 0,-4,-3,-2,-1, 0,-4,-3,-2,-1, 0,-4,-3,-2,-1, 0 };

    int wall_north = 0;
    int wall_south = 0;
    int wall_east = 0;
    int wall_west = 0;
    int q_x;
    int q_y;

    borg_kill* kill;

    borg_digging = false;


    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt) return (false);

    /* Only do this to summoners when they are close*/
    if (borg_kills_summoner == -1) return (false);

    /* Hungry,starving */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Sometimes we loop on this */
    if (time_this_panel > 500) return (false);

    /* Do not dig when confused */
    if (borg_skill[BI_ISCONFUSED]) return (false);

    /* Not when darkened */
    if (borg_skill[BI_CURLITE] == 0) return (false);

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position) return (false);
    if (borg_as_position) return (false);

    /* get the summoning monster */
    kill = &borg_kills[borg_kills_summoner];

    /* Summoner must be mobile */
    if (rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE)) return(false);
    /* Summoner must be able to pass through walls */
    if (rf_has(r_info[kill->r_idx].flags, RF_PASS_WALL)) return(false);
    if (rf_has(r_info[kill->r_idx].flags, RF_KILL_WALL)) return(false);

    /* Summoner has to be awake (so he will chase me */
    if (!kill->awake) return (false);

    /* Must have Stone to Mud spell */
    if (!borg_spell_okay(TURN_STONE_TO_MUD) &&
        !borg_spell_okay(SHATTER_STONE) &&
        !borg_equips_ring(sv_ring_digging) &&
        !borg_equips_item(act_stone_to_mud, true)) return (false);

    /* Summoner needs to be able to follow me.
     * So I either need to be able to
     * 1) have LOS on him or
     * 2) this panel needs to have had Magic Map or Wizard light cast on it.
     * If Mapped, then the flow codes needs to be used.
     */
    if (!borg_los(kill->y, kill->x, c_y, c_x))
    {
        /* Extract panel */
        q_x = w_x / borg_panel_wid();
        q_y = w_y / borg_panel_hgt();

        if (borg_detect_wall[q_y + 0][q_x + 0] == true &&
            borg_detect_wall[q_y + 0][q_x + 1] == true &&
            borg_detect_wall[q_y + 1][q_x + 0] == true &&
            borg_detect_wall[q_y + 1][q_x + 1] == true)
        {
            borg_flow_clear();
            borg_digging = true;
            borg_flow_enqueue_grid(kill->y, kill->x);
            borg_flow_spread(10, true, false, false, -1, false);
            if (!borg_flow_commit("Monster Path", GOAL_KILL)) return (false);
        }
        else
        {
            borg_flow_clear();
            borg_digging = true;
            borg_flow_enqueue_grid(kill->y, kill->x);
            borg_flow_spread(10, true, true, false, -1, false);
            if (!borg_flow_commit("Monster Path", GOAL_KILL)) return (false);
        }
    }

    /* NORTH -- Consider each area near the borg, looking for a good spot to hide */
    for (o_y = -2; o_y < 1; o_y++)
    {
        /* Resest Wall count */
        wall_north = 0;

        /* No E-W offset when looking North-South */
        o_x = 0;

        for (i = 0; i < 25; i++)
        {
            borg_grid* ag;

            /* Check grids near borg */
            m_y = c_y + o_y + ny[i];
            m_x = c_x + o_x + nx[i];

            /* avoid screen edgeds */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y)))
            {
                continue;
            }

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (n_array[i] == 0 && ((ag->feat == FEAT_NONE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* This is a good grid */
                wall_north++;
            }
            if (n_array[i] == 1 && ((ag->feat <= FEAT_MORE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* A good wall would score 25. */
                wall_north++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_north == 25)
        {
            if (borg_distance(c_y, c_x, c_y + o_y + ny[7], c_x + o_x + nx[7]) < b_distance)
            {
                b_y = o_y;
                b_x = o_x;
                b_n = true;
                b_distance = borg_distance(c_y, c_x, c_y + o_y + ny[7], c_x + o_x + nx[7]);
            }
        }
    }

    /* SOUTH -- Consider each area near the borg, looking for a good spot to hide */
    for (o_y = -1; o_y < 2; o_y++)
    {
        /* Resest Wall count */
        wall_south = 0;

        for (i = 0; i < 25; i++)
        {
            borg_grid* ag;

            /* No lateral offset on South check */
            o_x = 0;

            /* Check grids near borg */
            m_y = c_y + o_y + sy[i];
            m_x = c_x + o_x + sx[i];

            /* avoid screen edgeds */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y))) continue;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (s_array[i] == 0 && ((ag->feat == FEAT_NONE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* This is a good grid */
                wall_south++;
            }
            if (s_array[i] == 1 && ((ag->feat <= FEAT_MORE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* A good wall would score 25. */
                wall_south++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_south == 25)
        {
            if (borg_distance(c_y, c_x, c_y + o_y + sy[17], c_x + o_x + sx[17]) < b_distance)
            {
                b_y = o_y;
                b_x = o_x;
                b_s = true;
                b_n = false;
                b_distance = borg_distance(c_y, c_x, c_y + b_y + sy[17], c_x + b_x + sx[17]);
            }
        }
    }

    /* EAST -- Consider each area near the borg, looking for a good spot to hide */
    for (o_x = -1; o_x < 2; o_x++)
    {
        /* Resest Wall count */
        wall_east = 0;

        /* No N-S offset check when looking E-W */
        o_y = 0;

        for (i = 0; i < 25; i++)
        {
            borg_grid* ag;

            /* Check grids near borg */
            m_y = c_y + o_y + ey[i];
            m_x = c_x + o_x + ex[i];

            /* avoid screen edgeds */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y))) continue;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (e_array[i] == 0 && ((ag->feat == FEAT_NONE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* This is a good grid */
                wall_east++;
            }
            if (e_array[i] == 1 && ((ag->feat <= FEAT_MORE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* A good wall would score 25. */
                wall_east++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_east == 25)
        {
            if (borg_distance(c_y, c_x, c_y + o_y + ey[13], c_x + o_x + ex[13]) < b_distance)
            {
                b_y = o_y;
                b_x = o_x;
                b_e = true;
                b_s = false;
                b_n = false;
                b_distance = borg_distance(c_y, c_x, c_y + b_y + ey[13], c_x + b_x + ex[13]);
            }
        }
    }

    /* WEST -- Consider each area near the borg, looking for a good spot to hide */
    for (o_x = -2; o_x < 1; o_x++)
    {
        /* Resest Wall count */
        wall_west = 0;

        /* No N-S offset check when looking E-W */
        o_y = 0;

        for (i = 0; i < 25; i++)
        {
            borg_grid* ag;

            /* Check grids near borg */
            m_y = c_y + o_y + wy[i];
            m_x = c_x + o_x + wx[i];

            /* avoid screen edgeds */
            if (!square_in_bounds_fully(cave, loc(m_x, m_y))) continue;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* Certain grids must not be floor types */
            if (w_array[i] == 0 && ((ag->feat == FEAT_NONE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* This is a good grid */
                wall_west++;
            }
            if (w_array[i] == 1 && ((ag->feat <= FEAT_MORE) ||
                (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_QUARTZ_K) ||
                ag->feat == FEAT_GRANITE))
            {
                /* A good wall would score 25. */
                wall_west++;
            }
        }

        /* If I found 25 grids, then that spot will work well */
        if (wall_west == 25)
        {
            if (borg_distance(c_y, c_x, c_y + o_y + wy[11], c_x + o_x + wx[11]) < b_distance)
            {
                b_y = o_y;
                b_x = o_x;
                b_w = true;
                b_e = false;
                b_s = false;
                b_n = false;
                b_distance = borg_distance(c_y, c_x, c_y + o_y + wy[11], c_x + o_x + wx[11]);
            }
        }
    }

    /* Attempt to enqueu the grids that should be floor grids and have the borg
     * move onto those grids
     */
    if (b_n == true)
    {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(c_y + b_y + ny[7], c_x + b_x + nx[7]);

        /* Spread the flow */
        borg_flow_spread(5, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor north type 1", GOAL_DIGGING)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING)) return (false);

        return (true);
    }
    if (b_s == true)
    {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(c_y + b_y + sy[17], c_x + b_x + sx[17]);

        /* Spread the flow */
        borg_flow_spread(6, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor south type 1", GOAL_DIGGING)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING)) return (false);

        return (true);
    }
    if (b_e == true)
    {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(c_y + b_y + ey[13], c_x + b_x + ex[13]);

        /* Spread the flow */
        borg_digging = true;
        borg_flow_spread(5, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor east type 1", GOAL_DIGGING)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING)) return (false);

        return (true);
    }
    if (b_w == true)
    {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid where I will hide */
        borg_digging = true;
        borg_flow_enqueue_grid(c_y + b_y + wy[11], c_x + b_x + wx[11]);

        /* Spread the flow */
        borg_flow_spread(5, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor west type 1", GOAL_DIGGING)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_DIGGING)) return (false);

        return (true);
    }

    return false;
}

/*
 * Dig an anti-summon corridor
 *
 *            ############## We want the borg to not dig #1
 *            #............# but to dig #2, and hopefully shoot from the
 *      #######............# last #2 and try to avoid standing on #3.
 *      #222223............# This is great for offset ball attacks but
 *      #2#####..s.........# not for melee.  Warriors need to dig a wall
 * ######2###########+###### adjacent to the monsters so he can swing on them.
 * #            1     #
 * # ################ #
 *   #              # #
 * ###              # #
 *
 */
bool borg_flow_kill_corridor_2(bool viewable)
{
    int o_y, o_x;
    int m_x, m_y;
    int f_y, f_x;
    int floors = 0;
    int b_y = 0, b_x = 0;
    int perma_grids = 0;

    borg_kill* kill;

    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt) return (false);

    /* Only do this to summoners when they are close*/
    if (borg_kills_summoner == -1) return (false);

    /* Do not dig when weak. It takes too long */
    if (borg_skill[BI_STR] < 17) return (false);

    /* Hungry,starving */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK]) return (false);

    /* Sometimes we loop on this */
    if (time_this_panel > 500) return (false);

    /* Do not dig when confused */
    if (borg_skill[BI_ISCONFUSED]) return (false);

    /* Not when darkened */
    if (borg_skill[BI_CURLITE] == 0) return (false);

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position) return (false);

    /* get the summoning monster */
    kill = &borg_kills[borg_kills_summoner];

    /* Consider each adjacent spot to monster*/
    for (o_x = -1; o_x <= 1; o_x++)
    {
        for (o_y = -1; o_y <= 1; o_y++)
        {
            borg_grid* ag;

            /* Check grids near monster */
            m_x = kill->x + o_x;
            m_y = kill->y + o_y;

            /* grid the grid */
            ag = &borg_grids[m_y][m_x];

            /* avoid screen edgeds */
            if (m_x > AUTO_MAX_X - 2 ||
                m_x < 2 ||
                m_y > AUTO_MAX_Y - 2 ||
                m_y < 2)
                continue;

            /* Can't tunnel a non wall or permawall*/
            if (ag->feat != FEAT_NONE && ag->feat < FEAT_MAGMA) continue;
            if (ag->feat == FEAT_PERM)
            {
                perma_grids++;
                continue;
            }

            /* Do not dig unless we appear strong enough to succeed or we have a digger */
            bool hard = ag->feat == FEAT_GRANITE || ag->feat == FEAT_QUARTZ || ag->feat == FEAT_MAGMA;
            if (!borg_can_dig(false, hard))
                continue;

            /* reset floors counter */
            floors = 0;

            /* That grid must not have too many floors adjacent */
            for (f_x = -1; f_x <= 1; f_x++)
            {
                for (f_y = -1; f_y <= 1; f_y++)
                {
                    /* grid the grid */
                    ag = &borg_grids[m_y + f_y][m_x + f_x];

                    /* check if this neighbor is a floor */
                    if (ag->feat == FEAT_FLOOR ||
                        ag->feat == FEAT_BROKEN) floors++;
                }
            }

            /* Do not dig if too many floors near. */
            if (floors >= 5) continue;

            /* Track the good location */
            b_y = m_y;
            b_x = m_x;
        }
    }
    /* NOTE: Perma_grids count the number of grids which contain permawalls.
     * The borg may try to flow to an unknown grid but may get stuck on a perma
     * wall.  This will keep him from flowing to a summoner if the summoner is
     * near a perma grid.  The real fix would to be in the flow_spread so that
     * he will not flow through perma_grids.  I will work on that next.
     */
    if (b_y != 0 && b_x != 0 && perma_grids == 0)
    {
        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid */
        borg_flow_enqueue_grid(m_y, m_x);

        /* Spread the flow */
        borg_flow_spread(15, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("anti-summon corridor", GOAL_KILL)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_KILL)) return (false);

        return (true);
    }

    return false;
}


/*
 * Attempt to flow to a safe grid in order to rest up properly.  Following a battle, a borg needs to heal up.
 * He will attempt to heal up right where the fight was, but if he cannot, then he needs to retreat a bit.
 * This will help him find a good safe place to hide.
 *
 */
bool borg_flow_recover(bool viewable, int dist)
{
    int i, x, y;

    /* Sometimes we loop on this */
    if (time_this_panel > 500) return (false);

    /* No retreating and recovering when low level */
    if (borg_skill[BI_CLEVEL] <= 5) return (false);

    /* Mana for spell casters */
    if (player->class->magic.num_books > 3)
    {
        if (borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 3 &&
            borg_skill[BI_CURSP] > borg_skill[BI_MAXSP] / 4 && /* Non spell casters? */
            !borg_skill[BI_ISCUT] && !borg_skill[BI_ISSTUN] &&
            !borg_skill[BI_ISHEAVYSTUN] && !borg_skill[BI_ISAFRAID]) return (false);
    }
    else /* Non Spell Casters */
    {
        /* do I need to recover some? */
        if (borg_skill[BI_CURHP] > borg_skill[BI_MAXHP] / 3 &&
            !borg_skill[BI_ISCUT] && !borg_skill[BI_ISSTUN] &&
            !borg_skill[BI_ISHEAVYSTUN] && !borg_skill[BI_ISAFRAID]) return (false);
    }

    /* If Fleeing, then do not rest */
    if (goal_fleeing) return (false);

    /* If Scumming, then do not rest */
    if (borg_lunal_mode || borg_munchkin_mode) return (false);

    /* No need if hungry */
    if (borg_skill[BI_ISHUNGRY]) return (false);

    /* Nothing found */
    borg_temp_n = 0;

    /* Scan some known Grids
     * Favor the following types of grids:
     * 1. Happy grids
     */

     /* look at grids within 20 grids of me */
    for (y = c_y - 25; y < c_y + 25; y++)
    {

        for (x = c_x - 25; x < c_x + 25; x++)
        {
            /* Stay in bounds */
            if (!square_in_bounds(cave, loc(x, y))) continue;

            /* Skip my own grid */
            if (y == c_y && x == c_x) continue;

            /* Skip grids that are too close to me */
            if (borg_distance(c_y, c_x, y, x) < 7) continue;

            /* Is this grid a happy grid? */
            if (!borg_happy_grid_bold(y, x)) continue;

            /* Can't rest on a wall grid. */
            /* HACK depends on FEAT order, kinda evil */
            if (borg_grids[y][x].feat >= FEAT_SECRET && borg_grids[y][x].feat != FEAT_PASS_RUBBLE)
                continue;

            /* Can I rest on that one? */
            if (!borg_check_rest(y, x)) continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;
        }
    }

    /* Nothing to kill */
    if (!borg_temp_n) return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look through the good grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(dist, false, true, false, -1, false);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("Recover Grid", GOAL_RECOVER)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_RECOVER)) return (false);

    return (true);
}


/*
 * Prepare to "flow" towards monsters to "kill"
 * But in a few phases, viewable, near and far.
 * Note that monsters under the player are always deleted
 */
bool borg_flow_kill(bool viewable, int nearness)
{
    int i, x, y, p, j, b_j = -1;
    int b_stair = -1;

    bool borg_in_hall = false;
    int hall_y, hall_x, hall_walls = 0;
    bool skip_monster = false;

    borg_grid* ag;


    /* Efficiency -- Nothing to kill */
    if (!borg_kills_cnt) return (false);

    /* Don't chase down town monsters when you are just starting out */
    if (borg_skill[BI_CDEPTH] == 0 && borg_skill[BI_CLEVEL] < 20) return (false);

    /* YOU ARE NOT A WARRIOR!! DON'T ACT LIKE ONE!! */
    if ((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER) &&
        borg_skill[BI_CLEVEL] < (borg_skill[BI_CDEPTH] ? 35 : 25)) return (false);

    /* Not if Weak from hunger or no food */
    if (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] == 0) return (false);

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position) return (false);

    /* Nothing found */
    borg_temp_n = 0;

    /* check to see if in a hall, used later */
    for (hall_x = -1; hall_x <= 1; hall_x++)
    {
        for (hall_y = -1; hall_y <= 1; hall_y++)
        {
            /* Acquire location */
            x = hall_x + c_x;
            y = hall_y + c_y;

            ag = &borg_grids[y][x];

            /* track walls */
            if ((ag->glyph) ||
                ((ag->feat >= FEAT_MAGMA) && (ag->feat <= FEAT_PERM)))
            {
                hall_walls++;
            }

            /* addem up */
            if (hall_walls >= 5) borg_in_hall = true;
        }
    }


    /* Check distance away from stairs, used later */

    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* Scan the monster list */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];
        int x9 = kill->x;
        int y9 = kill->y;
        int ax, ay, d;

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Distance components */
        ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
        ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* dont bother flowing to an adjacent monster when I am afraid */
        if (d == 1 && (borg_skill[BI_ISAFRAID] || borg_skill[BI_CRSFEAR])) continue;

        /* Ignore multiplying monsters */
        if (goal_ignoring && !borg_skill[BI_ISAFRAID] &&
            (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY))) continue;

        /* Ignore molds when low level */
        if (borg_skill[BI_MAXCLEVEL] < 10 &&
            (rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE))) continue;

        /* Avoid flowing to a fight if a scary guy is on the level */
        if (scaryguy_on_level) continue;

        /* Avoid multiplying monsters when low level */
        if (borg_skill[BI_CLEVEL] < 10 && (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY))) continue;

        /* Hack -- ignore Maggot until later.  Player will chase Maggot
         * down all accross the screen waking up all the monsters.  Then
         * he is stuck in a compromised situation.
         */
        if ((rf_has(r_info[kill->r_idx].flags, RF_UNIQUE)) && borg_skill[BI_CDEPTH] == 0 &&
            borg_skill[BI_CLEVEL] < 5) continue;

        /* Access the location */
        x = kill->x;
        y = kill->y;

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Calculate danger */
        p = borg_danger(y, x, 1, true, false);


        /* Hack -- Skip "deadly" monsters unless uniques*/
        if (borg_skill[BI_CLEVEL] > 25 && (!rf_has(r_info->flags, RF_UNIQUE)) &&
            p > avoidance / 2) continue;
        if (borg_skill[BI_CLEVEL] <= 15 && p > avoidance / 3) continue;

        /* Skip ones that make me wander too far */
        if (b_stair != -1 && borg_skill[BI_CLEVEL] < 10)
        {
            /* Check the distance of this monster to the stair */
            j = borg_distance(track_less.y[b_stair], track_less.x[b_stair],
                y, x);
            /* skip far away monsters while I am close to stair */
            if (b_j <= borg_skill[BI_CLEVEL] * 5 + 9 &&
                j >= borg_skill[BI_CLEVEL] * 5 + 9) continue;
        }

        /* Hack -- Avoid getting surrounded */
        if (borg_in_hall && (rf_has(r_info[kill->r_idx].flags, RF_GROUP_AI)))
        {
            /* check to see if monster is in a hall, */
            for (hall_x = -1; hall_x <= 1; hall_x++)
            {
                for (hall_y = -1; hall_y <= 1; hall_y++)
                {
                    if (!square_in_bounds_fully(cave, loc(hall_x + x, hall_y + y))) continue;
                    ag = &borg_grids[hall_y + y][hall_x + x];

                    /* track walls */
                    if ((ag->glyph) ||
                        ((ag->feat >= FEAT_MAGMA) && (ag->feat <= FEAT_PERM)))
                    {
                        hall_walls++;
                    }

                    /* we want the monster to be in a hall also
                     *
                     *  ########################
                     *  ############      S  ###
                     *  #         @'     SSS ###
                     *  # ##########       SS###
                     *  # #        #       Ss###
                     *  # #        ###### ######
                     *  # #             # #
                     * Currently, we would like the borg to avoid
                     * flowing to a situation like the one above.
                     * We would like him to stay in the hall and
                     * attack from a distance.  One problem is the
                     * lower case 's' in the corner, He will show
                     * up as being in a corner, and the borg may
                     * flow to it.  Let's hope that is a rare case.
                     *
                     * The borg might flow to the 'dark' south exit
                     * of the room.  This would be dangerous for
                     * him as well.
                     */
                     /* add 'em up */
                    if (hall_walls < 4)
                    {
                        /* This monster is not in a hallway.
                         * It may not be safe to fight.
                         */
                        skip_monster = true;
                    }
                }
            }
        }

        /* Skip this one if it is just 2 grids from me and it can attack me as soon as I
         * move 1 grid closer to it.  Note that some monsters are faster than me and it
         * could still cover the 1 grid and hit me. I'll fix it (based on my speed) later XXX
         */
        if (d == 2 && /* Spacing is important */
            (!(kill->ranged_attack)) && /* Ranged Attacks, don't rest. */
            (!(rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE)))) /* Skip monsters that dont chase */
        {
            skip_monster = true;
        }


        /* skip certain ones */
        if (skip_monster) continue;

        /* Clear the flow codes */
        borg_flow_clear();

        /* Check the distance to stair for this proposed grid and leash*/
        if (borg_flow_cost_stair(y, x, b_stair) > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to kill */
    if (!borg_temp_n) return (false);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to kill */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward monsters that we can see, make sure they */
    /* are at least easily reachable.  The second flag is whether or not */
    /* to avoid unknown squares.  This was for performance when we have ESP. */
    borg_flow_spread(nearness, true, !viewable, false, -1, false);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("kill", GOAL_KILL)) return (false);


    /* Take one step */
    if (!borg_flow_old(GOAL_KILL)) return (false);


    /* Success */
    return (true);
}

/*
 * Prepare to "flow" towards mineral veins with treasure
 *
 */
bool borg_flow_vein(bool viewable, int nearness)
{
    int i, x, y;
    int b_stair = -1, j, b_j = -1;
    int cost = 0;
    int leash = borg_skill[BI_CLEVEL] * 3 + 9;

    borg_grid* ag;


    /* Efficiency -- Nothing to take */
    if (!track_vein.num) return (false);

    /* Increase leash */
    if (borg_skill[BI_CLEVEL] >= 20) leash = 250;

    /* Not needed if rich */
    if (borg_gold >= 100000) return (false);

    /* Require digger, capacity, or skill */
    if (!borg_can_dig(true, false)) return (false);

    /* Nothing yet */
    borg_temp_n = 0;

    /* Set the searching flag for low level borgs */
    borg_needs_searching = true;

    /* Check distance away from stairs, used later */
    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* Scan the vein list */
    for (i = 0; i < track_vein.num; i++)
    {
        /* Access the location */
        x = track_vein.x[i];
        y = track_vein.y[i];

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Clear the flow codes */
        borg_flow_clear();

        /* obtain the number of steps from this take to the stairs */
        cost = borg_flow_cost_stair(y, x, b_stair);

        /* Check the distance to stair for this proposed grid, unless i am looking for very close items (leash) */
        if (nearness > 5 && cost > leash && borg_skill[BI_CLEVEL] < 20) continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to mine */
    if (!borg_temp_n) return (false);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unkown squares.  This was for performance. */
    borg_flow_spread(nearness, true, !viewable, false, -1, false);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("vein", GOAL_TAKE)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE)) return (false);

    /* Success */
    return (true);
}


/*
 * Prepare to "flow" towards objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take(bool viewable, int nearness)
{
    int i, x, y;
    int b_stair = -1, j, b_j = -1;
    int leash = borg_skill[BI_CLEVEL] * 3 + 9;
    int full_quiver;

    borg_grid* ag;

    /* Missile carry limit */
    /* allow shooters to two quiver slots full */
    if (player_has(player, PF_FAST_SHOT))
        full_quiver = (z_info->quiver_slot_size - 1) * 2;
    else 
        full_quiver = z_info->quiver_slot_size - 1;

    /* Efficiency -- Nothing to take */
    if (!borg_takes_cnt) return (false);

    /* Require one empty slot */
    if (borg_items[PACK_SLOTS - 1].iqty) return (false);

    /* If ScaryGuy, no chasing down items */
    if (scaryguy_on_level) return (false);

    /* If out of fuel, don't mess around */
    if (!borg_skill[BI_CURLITE]) return (false);

    /* Not if sitting in a sea of runes */
    if (borg_morgoth_position) return (false);

    /* increase leash */
    if (borg_skill[BI_CLEVEL] >= 20) leash = 250;

    /* Starting over on count */
    borg_temp_n = 0;

    /* Set the searching flag for low level borgs */
    borg_needs_searching = true;

    /* if the borg is running on Boosted Bravery, no
     * searching
     */
    if (borg_no_retreat >= 1) borg_needs_searching = false;

    /* Check distance away from stairs, used later */
    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* Scan the object list */
    for (i = 1; i < borg_takes_nxt; i++)
    {
        borg_take* take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind) continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* Skip ones that make me wander too far */
        if (b_stair != -1 && borg_skill[BI_CLEVEL] < 10)
        {
            /* Check the distance of this 'take' to the stair */
            j = borg_distance(track_less.y[b_stair], track_less.x[b_stair],
                y, x);
            /* skip far away takes while I am close to stair*/
            if (b_j <= leash &&
                j >= leash) continue;
        }

        /* skip worthless items */
        if (take->value <= 0) continue;

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Don't bother with ammo if I am at capacity */

        if (take->tval == borg_skill[BI_AMMO_TVAL]  && borg_skill[BI_AMISSILES] >= full_quiver) continue;
        /* No need to chase certain things down after a certain amount.  Dont chase:
         * Money
         * Other spell books
         * Wrong ammo
         */
        if (borg_gold >= 500000)
        {
            if (take->tval == TV_GOLD) continue;
            if (!obj_kind_can_browse(&k_info[take->kind->kidx])) continue;
            if ((take->tval == TV_SHOT || take->tval == TV_ARROW || take->tval == TV_BOLT) &&
                take->tval != borg_skill[BI_AMMO_TVAL] ) continue;
            /*
            Restore Mana for warriors?
            low level potions
            low level scrolls
            */

        }

        /* Clear the flow codes */
        borg_flow_clear();

        /* Check the distance to stair for this proposed grid and leash*/
        if (nearness > 5 && borg_flow_cost_stair(y, x, b_stair) > leash && borg_skill[BI_CLEVEL] < 20) continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to take */
    if (!borg_temp_n) return (false);


    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unkown squares.  This was for performance. */
    borg_flow_spread(nearness, true, !viewable, false, -1, false);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("item", GOAL_TAKE)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE)) return (false);

    /* Success */
    return (true);
}

/*
 * Prepare to "flow" towards special objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take_scum(bool viewable, int nearness)
{
    int i, x, y;
    int j;
    int b_j = -1;
    int b_stair = -1;

    borg_grid* ag;


    /* Efficiency -- Nothing to take */
    if (!borg_takes_cnt) return (false);

    /* Require one empty slot */
    if (borg_items[PACK_SLOTS - 1].iqty) return (false);

    /* Nothing yet */
    borg_temp_n = 0;

    /* Set the searching flag for low level borgs */
    borg_needs_searching = true;

    /* Check distance away from stairs, used later */

    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* Scan the object list -- set filter*/
    for (i = 1; i < borg_takes_nxt; i++)
    {
        borg_take* take = &borg_takes[i];

        /* Skip dead objects */
        if (!take->kind) continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* skip worthless items */
        if (take->value <= 0) continue;

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Clear the flow codes */
        borg_flow_clear();

        /* Check the distance to stair for this proposed grid with leash */
        if (borg_flow_cost_stair(y, x, b_stair) > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;


        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to take */
    if (!borg_temp_n) return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unknown squares.  This was for performance. */
    borg_flow_spread(nearness, true, !viewable, false, -1, true);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("Scum item", GOAL_TAKE)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE)) return (false);

    /* Success */
    return (true);
}

/*
 * Prepare to "flow" towards special objects to "take"
 *
 * Note that objects under the player are always deleted
 */
bool borg_flow_take_lunal(bool viewable, int nearness)
{
    int i, ii, x, y;
    int j;
    int b_j = -1;
    int b_stair = -1;

    borg_grid* ag;

    /* Efficiency -- Nothing to take */
    if (!borg_takes_cnt) return (false);

    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* Nothing yet */
    borg_temp_n = 0;

    /* Set the searching flag for low level borgs */
    borg_needs_searching = true;

    /* Scan the object list -- set filter*/
    for (i = 1; i < borg_takes_nxt; i++)
    {
        borg_take* take = &borg_takes[i];
        struct object_kind* k_ptr = take->kind;

        bool item_bad;

        /* Skip dead objects */
        if (!k_ptr) continue;

        /* Access the location */
        x = take->x;
        y = take->y;

        /* all items start bad */
        item_bad = true;

        /* Gold is good to have */
        if (take->tval == TV_GOLD)
        {
            borg_note(format("# Lunal Item %s, at %d,%d", take->kind->name, y, x));
            item_bad = false;
        }

        /* If full can I absorb the item into an existing stack */
        if (item_bad && take->value > 0)
        {
            if (borg_is_ammo(take->tval))
            {
                /* Scan the quiver */
                for (ii = QUIVER_START; ii < QUIVER_END; ii++)
                {
                    /* skip empty slots */
                    if (!borg_items[ii].iqty) 
                        continue;

                    /* skip fullslots */
                    if (borg_items[ii].iqty == z_info->quiver_slot_size)
                        continue;

                    /* Both objects should have the same ID value */
                    if (take->kind->kidx != borg_items[ii].kind) 
                        continue;

                    if (k_ptr->sval == borg_items[ii].sval &&
                        k_ptr->tval == borg_items[ii].tval)
                    {
                        item_bad = false;
                    }
                }
            }
            else if (borg_items[PACK_SLOTS - 1].iqty)
            {
                /* Scan the inventory */
                for (ii = 0; ii < PACK_SLOTS; ii++)
                {
                    /* skip empty slots */
                    if (!borg_items[ii].iqty) continue;

                    /* Both objects should have the same ID value */
                    if (take->kind->kidx != borg_items[ii].kind) continue;

                    /* Certain types of items can stack */
                    if (k_ptr->sval == borg_items[ii].sval &&
                        k_ptr->tval == borg_items[ii].tval &&
                        (borg_items[ii].tval == TV_POTION ||
                            borg_items[ii].tval == TV_SCROLL ||
                            borg_items[ii].tval == TV_ROD))
                    {
                        item_bad = false;
                    }
                }
            }
        }

        /* Require one empty slot */
        if (!borg_items[PACK_SLOTS - 1].iqty && item_bad == true)
        {
            /* for ammo, make sure the quiver isn't full */
            if (!borg_is_ammo(take->tval) || borg_items[QUIVER_END - 1].iqty == 0)
            {

                /* Certain Potions are worthless */
                if (take->tval == TV_POTION &&
                    (take->kind->sval >= sv_potion_inc_str) &&
                    (take->kind->sval <= sv_potion_detect_invis))
                {
                    borg_note(format("# Lunal Item %s, at %d,%d", take->kind->name, y, x));
                    item_bad = false;
                }


                /* Certain insta_arts are good.  Note that there is no top end of this.  So if an item
                 * were added after the last artifact, it would also be picked up.
                 */
                if (kf_has(take->kind->kind_flags, KF_INSTA_ART))
                {
                    borg_note(format("# Lunal Item %s, at %d,%d", take->kind->name, y, x));
                    item_bad = false;
                }

                /* if scumming the start of the game, take all items to sell them */
                if (borg_cfg[BORG_MUNCHKIN_START])
                {
                    /* Certain known items are junky and should be ignored.  Grab only
                     * things of value
                     */
                    if (take->value >= 1)
                        item_bad = false;
                }
            }
        }

        /* Get the grid */
        ag = &borg_grids[y][x];

        /* Require line of sight if requested */
        if (viewable && !(ag->info & BORG_VIEW)) continue;

        /* Clear the flow codes */
        borg_flow_clear();

        /* Check the distance to stair for this proposed grid */
        if (borg_flow_cost_stair(y, x, b_stair) > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;


        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing to take */
    if (!borg_temp_n) return (false);

    /* Clear the flow codes */
    borg_flow_clear();

    /* Look for something to take */
    for (i = 0; i < borg_temp_n; i++)
    {
        /* Enqueue the grid */
        borg_flow_enqueue_grid(borg_temp_y[i], borg_temp_x[i]);
    }

    /* Spread the flow */
    /* if we are not flowing toward items that we can see, make sure they */
    /* are at least easily reachable.  The second flag is weather or not  */
    /* to avoid unknown squares.  This was for performance. */
    borg_flow_spread(nearness, false, !viewable, false, -1, true);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("munchkin item", GOAL_TAKE)) return (false);

    /* Check for monsters before walking over to the item */
    if (borg_check_LIGHT()) return (true);

    /* Take one step */
    if (!borg_flow_old(GOAL_TAKE)) return (false);

    /* Success */
    return (true);
}


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

    borg_grid* ag;

    /* Have the borg so some Searching */
    borg_needs_searching = true;



    /* Get the borg_grid */
    ag = &borg_grids[y][x];

    /* Explore unknown grids */
    if (ag->feat == FEAT_NONE) return (true);

    /* Efficiency -- Ignore "boring" grids */
    if (ag->feat < FEAT_SECRET) return (false);

    /* Explore "known treasure" */
    if ((ag->feat == FEAT_MAGMA_K) || (ag->feat == FEAT_QUARTZ_K))
    {
        /* Do not dig when confused */
        if (borg_skill[BI_ISCONFUSED]) return (false);

        /* Do not bother if super rich */
        if (borg_gold >= 100000) return (false);

        /* Not when darkened */
        if (borg_skill[BI_CURLITE] == 0) return (false);

        /* don't try to dig if we can't */
        if (!borg_can_dig(false, false)) return (false);

        /* Okay */
        return (true);
    }

    /* "Vaults" Explore non perma-walls adjacent to a perma wall */
    if (ag->feat == FEAT_GRANITE || ag->feat == FEAT_MAGMA ||
        ag->feat == FEAT_QUARTZ)
    {
        /* Do not attempt when confused */
        if (borg_skill[BI_ISCONFUSED]) return (false);

        /* hack and cheat.  No vaults  on this level */
        if (!vault_on_level) return (false);

        /* AJG Do not attempt on the edge */
        if (x < AUTO_MAX_X - 1
            && y < AUTO_MAX_Y - 1
            && x > 1
            && y > 1)
        {
            /* scan the adjacent grids */
            for (ox = -1; ox <= 1; ox++)
            {
                for (oy = -1; oy <= 1; oy++)
                {

                    /* Acquire location */
                    ag = &borg_grids[oy + y][ox + x];

                    /* skip non perma grids wall */
                    if (ag->feat != FEAT_PERM) continue;

                    /* make sure we can dig */
                    if (!borg_can_dig(false, true)) return (false);

                    /* Glove up and dig in */
                    return (true);
                }
            }
        }

        /* not adjacent to a GCV,  Restore Grid */
        ag = &borg_grids[y][x];
    }

    /* Explore "rubble" */
    if (ag->feat == FEAT_RUBBLE && !borg_skill[BI_ISWEAK])
    {
        return (true);
    }

    /* Explore "closed doors" */
    if (ag->feat == FEAT_CLOSED)
    {
        /* some closed doors leave alone */
        if (breeder_level)
        {
            /* Did I close this one */
            for (i = 0; i < track_door.num; i++)
            {
                /* mark as icky if I closed this one */
                if ((track_door.x[i] == x) && (track_door.y[i] == y))
                {
                    /* not interesting */
                    return (false);
                }
            }

        }
        /* this door should be ok to open */
        return (true);
    }


    /* Explore "visible traps" */
    if (feat_is_trap_holding(ag->feat))
    {
        /* Do not disarm when blind */
        if (borg_skill[BI_ISBLIND]) return (false);

        /* Do not disarm when confused */
        if (borg_skill[BI_ISCONFUSED]) return (false);

        /* Do not disarm when hallucinating */
        if (borg_skill[BI_ISIMAGE]) return (false);

        /* Do not flow without lite */
        if (borg_skill[BI_CURLITE] == 0) return (false);

        /* Do not disarm trap doors on level 99 */
        if (borg_skill[BI_CDEPTH] == 99 && ag->trap && !ag->glyph) return (false);

        /* Do not disarm when you could end up dead */
        if (borg_skill[BI_CURHP] < 60) return (false);

        /* Do not disarm when clumsy */
        if (borg_skill[BI_DISP] < 30 && borg_skill[BI_CLEVEL] < 20) return (false);
        if (borg_skill[BI_DISP] < 45 && borg_skill[BI_CLEVEL] < 10) return (false);
        if (borg_skill[BI_DISM] < 30 && borg_skill[BI_CLEVEL] < 20) return (false);
        if (borg_skill[BI_DISM] < 45 && borg_skill[BI_CLEVEL] < 10) return (false);

        /* Do not explore if a Scaryguy on the Level */
        if (scaryguy_on_level) return (false);

        /* NOTE: the flow code allows a borg to flow through a trap and so he may
         * still try to disarm one on his way to the other interesting grid.  If mods
         * are made to the above criteria for disarming traps, then mods must also be
         * made to borg_flow_spread() and borg_flow_direct()
         */

         /* Okay */
        return (true);
    }


    /* Ignore other grids */
    return (false);
}


/*
 * Determine if a grid is "reachable" (and can be explored)
 */
static bool borg_flow_dark_reachable(int y, int x)
{
    int j;

    borg_grid* ag;

    /* Scan neighbors */
    for (j = 0; j < 8; j++)
    {
        int y2 = y + ddy_ddd[j];
        int x2 = x + ddx_ddd[j];

        /* Get the grid */
        ag = &borg_grids[y2][x2];

        /* Skip unknown grids (important) */
        if (ag->feat == FEAT_NONE) continue;

        /* Accept known floor grids */
        if (borg_cave_floor_grid(ag)) return (true);
    }

    /* Failure */
    return (false);
}

/* Dig a straight Tunnel to a close monster */
bool borg_flow_kill_direct(bool viewable, bool twitchy)
{
    int i;
    int b_i = -1;
    int d;
    int b_d = z_info->max_sight;

    borg_kill* kill;


    /* Do not dig when weak. It takes too long */
    if (!borg_can_dig(false, false)) return (false);

    /* Not if Weak from hunger or no food */
    if (!twitchy && (borg_skill[BI_ISHUNGRY] || borg_skill[BI_ISWEAK] || borg_skill[BI_FOOD] == 0)) return (false);

    /* Only when sitting for too long or twitchy */
    if (!twitchy && borg_t - borg_began < 3000 && borg_times_twitch < 5) return (false);

    /* Do not dig when confused */
    if (borg_skill[BI_ISCONFUSED]) return (false);

    /* Not when darkened */
    if (borg_skill[BI_CURLITE] == 0) return (false);

    /* Efficiency -- Nothing to kill */
    if (borg_kills_cnt)
    {
        /* Scan the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            kill = &borg_kills[i];

            /* Skip "dead" monsters */
            if (!kill->r_idx) continue;

            /* Distance away */
            d = borg_distance(kill->y, kill->x, c_y, c_x);

            /* Track closest one */
            if (d > b_d) continue;

            /* Track it */
            b_i = i; b_d = d;
        }
    }

    /* If no Kill, then pick the center of the map */
    if (b_i == -1)
    {

        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid */
        borg_flow_enqueue_grid(AUTO_MAX_Y / 2, AUTO_MAX_X / 2);

        /* Spread the flow */
        borg_flow_spread(150, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("center direct", GOAL_KILL)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_KILL)) return (false);

        return (true);
    }

    if (b_i) /* don't want it near permawall */
    {
        /* get the closest monster */
        kill = &borg_kills[b_i];

        /* Clear the flow codes */
        borg_flow_clear();

        /* Enqueue the grid */
        borg_flow_enqueue_grid(kill->y, kill->x);

        /* Spread the flow */
        borg_flow_spread(15, true, false, true, -1, false);

        /* Attempt to Commit the flow */
        if (!borg_flow_commit("kill direct", GOAL_KILL)) return (false);

        /* Take one step */
        if (!borg_flow_old(GOAL_KILL)) return (false);

        return (true);
    }

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

    borg_grid* ag;


    /* Avoid icky grids */
    if (borg_data_icky->data[y][x]) return;

    /* Unknown */
    if (!borg_data_know->data[y][x])
    {
        /* Mark as known */
        borg_data_know->data[y][x] = true;

        /* Get the danger */
        p = borg_danger(y, x, 1, true, false);

        /* Increase bravery */
        if (borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 5 / 10;
        if (borg_skill[BI_MAXCLEVEL] != 50) fear = avoidance * 3 / 10;
        if (scaryguy_on_level) fear = avoidance * 2;
        if (unique_on_level && vault_on_level && borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 3;
        if (scaryguy_on_level && borg_skill[BI_CLEVEL] <= 5) fear = avoidance * 3;
        if (goal_ignoring) fear = avoidance * 5;
        if (borg_t - borg_began > 5000) fear = avoidance * 25;
        if (borg_skill[BI_FOOD] == 0) fear = avoidance * 100;

        /* Normal in town */
        if (borg_skill[BI_CLEVEL] == 0) fear = avoidance * 1 / 10;

        /* Mark dangerous grids as icky */
        if (p > fear)
        {
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
    y2 = c_y;
    x2 = c_x;

    /* Calculate distance components */
    ay = (y2 < y1) ? (y1 - y2) : (y2 - y1);
    ax = (x2 < x1) ? (x1 - x2) : (x2 - x1);

    /* Path */
    while (1)
    {
        /* Check for arrival at player */
        if ((x == x2) && (y == y2)) return;

        /* Next */
        n++;

        /* Move mostly vertically */
        if (ay > ax)
        {
            /* Extract a shift factor XXX */
            shift = (n * ax + (ay - 1) / 2) / ay;

            /* Sometimes move along the minor axis */
            x = (x2 < x1) ? (x1 - shift) : (x1 + shift);

            /* Always move along major axis */
            y = (y2 < y1) ? (y1 - n) : (y1 + n);
        }

        /* Move mostly horizontally */
        else
        {
            /* Extract a shift factor XXX */
            shift = (n * ay + (ax - 1) / 2) / ax;

            /* Sometimes move along the minor axis */
            y = (y2 < y1) ? (y1 - shift) : (y1 + shift);

            /* Always move along major axis */
            x = (x2 < x1) ? (x1 - n) : (x1 + n);
        }


        /* Access the grid */
        ag = &borg_grids[y][x];


        /* Ignore "wall" grids */
        if (!borg_cave_floor_grid(ag)) return;

        /* Avoid Traps if low level-- unless brave or scaryguy. */
        if (ag->trap &&
            avoidance <= borg_skill[BI_CURHP] && !scaryguy_on_level)
        {
            /* Do not disarm when you could end up dead */
            if (borg_skill[BI_CURHP] < 60) return;

            /* Do not disarm when clumsy */
            if (borg_skill[BI_DISP] < 30 && borg_skill[BI_CLEVEL] < 20) return;
            if (borg_skill[BI_DISP] < 45 && borg_skill[BI_CLEVEL] < 10) return;
            if (borg_skill[BI_DISM] < 30 && borg_skill[BI_CLEVEL] < 20) return;
            if (borg_skill[BI_DISM] < 45 && borg_skill[BI_CLEVEL] < 10) return;
        }

        /* Abort at "icky" grids */
        if (borg_data_icky->data[y][x]) return;

        /* Analyze every grid once */
        if (!borg_data_know->data[y][x])
        {
            /* Mark as known */
            borg_data_know->data[y][x] = true;

            /* Get the danger */
            p = borg_danger(y, x, 1, true, false);

            /* Increase bravery */
            if (borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 5 / 10;
            if (borg_skill[BI_MAXCLEVEL] != 50) fear = avoidance * 3 / 10;
            if (scaryguy_on_level) fear = avoidance * 2;
            if (unique_on_level && vault_on_level && borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 3;
            if (scaryguy_on_level && borg_skill[BI_CLEVEL] <= 5) fear = avoidance * 3;
            if (goal_ignoring) fear = avoidance * 5;
            if (borg_t - borg_began > 5000) fear = avoidance * 25;
            if (borg_skill[BI_FOOD] == 0) fear = avoidance * 100;

            /* Normal in town */
            if (borg_skill[BI_CLEVEL] == 0) fear = avoidance * 1 / 10;

            /* Avoid dangerous grids (forever) */
            if (p > fear)
            {
                /* Mark as icky */
                borg_data_icky->data[y][x] = true;

                /* Abort */
                return;
            }
        }

        /* Abort "pointless" paths if possible */
        if (borg_data_cost->data[y][x] <= n) break;

        /* Save the new flow cost */
        borg_data_cost->data[y][x] = n;
    }
}

/* Currently not used, I thought I might need it for anti-summoning */
extern void borg_flow_direct_dig(int y, int x)
{
    int n = 0;

    int x1, y1, x2, y2;

    int ay, ax;

    int shift;

    int p, fear = 0;

#if 0
    /* Avoid icky grids */
    if (borg_data_icky->data[y][x]) return;

    /* Unknown */
    if (!borg_data_know->data[y][x])
    {
        /* Mark as known */
        borg_data_know->data[y][x] = true;

        /* Mark dangerous grids as icky */
        if (borg_danger(y, x, 1, true, false) > avoidance / 3)
        {
            /* Icky */
            borg_data_icky->data[y][x] = true;

            /* Avoid */
            return;
        }
    }

#endif

    /* Save the flow cost (zero) */
    borg_data_cost->data[y][x] = 0;


    /* Save "origin" */
    y1 = y;
    x1 = x;

    /* Save "destination" */
    y2 = c_y;
    x2 = c_x;

    /* Calculate distance components */
    ay = (y2 < y1) ? (y1 - y2) : (y2 - y1);
    ax = (x2 < x1) ? (x1 - x2) : (x2 - x1);

    /* Path */
    while (1)
    {
        /* Check for arrival at player */
        if ((x == x2) && (y == y2)) return;

        /* Next */
        n++;

        /* Move mostly vertically */
        if (ay > ax)
        {
            /* Extract a shift factor XXX */
            shift = (n * ax + (ay - 1) / 2) / ay;

            /* Sometimes move along the minor axis */
            x = (x2 < x1) ? (x1 - shift) : (x1 + shift);

            /* Always move along major axis */
            y = (y2 < y1) ? (y1 - n) : (y1 + n);
        }

        /* Move mostly horizontally */
        else
        {
            /* Extract a shift factor XXX */
            shift = (n * ay + (ax - 1) / 2) / ax;

            /* Sometimes move along the minor axis */
            y = (y2 < y1) ? (y1 - shift) : (y1 + shift);

            /* Always move along major axis */
            x = (x2 < x1) ? (x1 - n) : (x1 + n);
        }


        /* Abort at "icky" grids */
        if (borg_data_icky->data[y][x]) return;

        /* Analyze every grid once */
        if (!borg_data_know->data[y][x])
        {
            /* Mark as known */
            borg_data_know->data[y][x] = true;

            /* Get the danger */
            p = borg_danger(y, x, 1, true, false);

            /* Increase bravery */
            if (borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 5 / 10;
            if (borg_skill[BI_MAXCLEVEL] != 50) fear = avoidance * 3 / 10;
            if (scaryguy_on_level) fear = avoidance * 2;
            if (unique_on_level && vault_on_level && borg_skill[BI_MAXCLEVEL] == 50) fear = avoidance * 3;
            if (scaryguy_on_level && borg_skill[BI_CLEVEL] <= 5) fear = avoidance * 3;
            if (goal_ignoring) fear = avoidance * 5;
            if (borg_t - borg_began > 5000) fear = avoidance * 25;
            if (borg_skill[BI_FOOD] == 0) fear = avoidance * 100;

            /* Normal in town */
            if (borg_skill[BI_CLEVEL] == 0) fear = avoidance * 1 / 10;

            /* Avoid dangerous grids (forever) */
            if (p > fear)
            {
                /* Mark as icky */
                borg_data_icky->data[y][x] = true;

                /* Abort */
                return;
            }
        }

        /* Abort "pointless" paths if possible */
        if (borg_data_cost->data[y][x] <= n) break;

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
    for (y = y1; y <= y2; y++)
    {
        /* Avoid/Clear west edge */
        borg_data_know->data[y][x1] = stop;
        borg_data_icky->data[y][x1] = stop;

        /* Avoid/Clear east edge */
        borg_data_know->data[y][x2] = stop;
        borg_data_icky->data[y][x2] = stop;
    }

    /* Scan north/south edges */
    for (x = x1; x <= x2; x++)
    {
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
    int cost;
    int x, y;


    /* Hack -- not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);


    /* Reset */
    borg_temp_n = 0;

    /* Scan torch-lit grids */
    for (i = 0; i < borg_LIGHT_n; i++)
    {
        y = borg_LIGHT_y[i];
        x = borg_LIGHT_x[i];

        /* Skip "boring" grids (assume reachable) */
        if (!borg_flow_dark_interesting(y, x, b_stair)) continue;

        /* Clear the flow codes */
        borg_flow_clear();

        /* obtain the number of steps from this take to the stairs */
        cost = borg_flow_cost_stair(y, x, b_stair);

        /* Check the distance to stair for this proposed grid if dangerous */
        if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 && cost > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing */
    if (!borg_temp_n) return (false);


    /* Wipe icky codes from grids if needed */
    if (goal_ignoring || scaryguy_on_level) borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Create paths to useful grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Create a path */
        borg_flow_direct(y, x);
    }


    /* Attempt to Commit the flow */
    if (!borg_flow_commit(NULL, GOAL_DARK)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK)) return (false);

    /* Forget goal */
    /* goal = 0; */

    /* Success */
    return (true);
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
    int cost;
    int x, y;

    borg_grid* ag;


    /* Hack -- not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);

    /* Set the searching flag for low level borgs */
    borg_needs_searching = true;

    /* Maximal radius */
    r = borg_skill[BI_CURLITE] + 1;


    /* Reset */
    borg_temp_n = 0;

    /* Four directions */
    for (i = 0; i < 4; i++)
    {
        y = c_y + ddy_ddd[i] * r;
        x = c_x + ddx_ddd[i] * r;

        /* Check legality */
        if (y < 1) continue;
        if (x < 1) continue;
        if (y > AUTO_MAX_Y - 2) continue;
        if (x > AUTO_MAX_X - 2) continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Require unknown */
        if (ag->feat != FEAT_NONE) continue;

        /* Require viewable */
        if (!(ag->info & BORG_VIEW)) continue;

        /* if it makes me wander, skip it */

        /* Clear the flow codes */
        borg_flow_clear();

        /* obtain the number of steps from this take to the stairs */
        cost = borg_flow_cost_stair(y, x, b_stair);

        /* Check the distance to stair for this proposed grid */
        if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 && cost > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;

        /* Careful -- Remember it */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* Nothing */
    if (!borg_temp_n) return (false);

    /* Wipe icky codes from grids if needed */
    if (goal_ignoring || scaryguy_on_level) borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Create paths to useful grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Create a path */
        borg_flow_direct(y, x);
    }


    /* Attempt to Commit the flow */
    if (!borg_flow_commit(NULL, GOAL_DARK)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK)) return (false);

    /* Forget goal */
    /* goal = 0; */

    /* Success */
    return (true);
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
    int cost;
    int x, y;

    int x1, y1, x2, y2;


    /* Hack -- not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);


    /* Local region */
    y1 = c_y - 4;
    x1 = c_x - 4;
    y2 = c_y + 4;
    x2 = c_x + 4;

    /* Restrict to "legal" grids */
    if (y1 < 1) y1 = 1;
    if (x1 < 1) x1 = 1;
    if (y2 > AUTO_MAX_Y - 2) y2 = AUTO_MAX_Y - 2;
    if (x2 > AUTO_MAX_X - 2) x2 = AUTO_MAX_X - 2;


    /* Reset */
    borg_temp_n = 0;

    /* Examine the region */
    for (y = y1; y <= y2; y++)
    {
        /* Examine the region */
        for (x = x1; x <= x2; x++)
        {
            /* Skip "boring" grids */
            if (!borg_flow_dark_interesting(y, x, b_stair)) continue;

            /* Skip "unreachable" grids */
            if (!borg_flow_dark_reachable(y, x)) continue;

            /* Clear the flow codes */
            borg_flow_clear();

            /* obtain the number of steps from this take to the stairs */
            cost = borg_flow_cost_stair(y, x, b_stair);

            /* Check the distance to stair for this proposed grid */
            if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5 &&
                cost > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;


            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;
        }
    }

    /* Nothing interesting */
    if (!borg_temp_n) return (false);

    /* Wipe icky codes from grids if needed */
    if (goal_ignoring || scaryguy_on_level) borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }

    /* Spread the flow (limit depth) */
    borg_flow_spread(5, false, true, false, -1, false);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit(NULL, GOAL_DARK)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK)) return (false);

    /* Success */
    return (true);
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
    int cost;
    int x1, y1, x2, y2;
    int leash = 250;

    /* Hack -- not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);

    /* Hack -- Not if a vault is on the level */
    if (vault_on_level) return (false);

    /* Local region */
    y1 = c_y - 11;
    x1 = c_x - 11;
    y2 = c_y + 11;
    x2 = c_x + 11;

    /* Restrict to "legal" grids */
    if (y1 < 1) y1 = 1;
    if (x1 < 1) x1 = 1;
    if (y2 > AUTO_MAX_Y - 2) y2 = AUTO_MAX_Y - 2;
    if (x2 > AUTO_MAX_X - 2) x2 = AUTO_MAX_X - 2;


    /* Nothing yet */
    borg_temp_n = 0;

    /* check the leash length */
    if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5) leash = borg_skill[BI_CLEVEL] * 3 + 9;

    /* Examine the panel */
    for (y = y1; y <= y2; y++)
    {
        /* Examine the panel */
        for (x = x1; x <= x2; x++)
        {
            /* Skip "boring" grids */
            if (!borg_flow_dark_interesting(y, x, b_stair)) continue;

            /* Skip "unreachable" grids */
            if (!borg_flow_dark_reachable(y, x)) continue;

            /* Clear the flow codes */
            borg_flow_clear();

            /* obtain the number of steps from this take to the stairs */
            cost = borg_flow_cost_stair(y, x, b_stair);

            /* Check the distance to stair for this proposed grid */
            if (cost > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;
        }
    }

    /* Nothing useful */
    if (!borg_temp_n) return (false);

    /* Wipe icky codes from grids if needed */
    if (goal_ignoring || scaryguy_on_level) borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }


    /* Expand borders */
    y1--; x1--; y2++; x2++;

    /* Avoid the edges */
    borg_flow_border(y1, x1, y2, x2, true);

    /* Spread the flow (limit depth Leash) */
    if (borg_skill[BI_CLEVEL] < 15)
    {
        /* Short Leash */
        borg_flow_spread(leash, true, true, false, -1, false);
    }
    else
    {
        /* Long Leash */
        borg_flow_spread(250, true, true, false, -1, false);
    }

    /* Clear the edges */
    borg_flow_border(y1, x1, y2, x2, false);


    /* Attempt to Commit the flow */
    if (!borg_flow_commit("dark-4", GOAL_DARK)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK)) return (false);

    /* Success */
    return (true);
}


/*
 * Prepare to "flow" towards "interesting" grids (method 5)
 */
static bool borg_flow_dark_5(int b_stair)
{
    int i, x, y;
    int cost;
    int leash = 250;

    /* Hack -- not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);


    /* Nothing yet */
    borg_temp_n = 0;

    /* check the leash length */
    if (borg_skill[BI_CDEPTH] >= borg_skill[BI_CLEVEL] - 5) leash = borg_skill[BI_CLEVEL] * 3 + 9;

    /* Examine every "legal" grid */
    for (y = 1; y < AUTO_MAX_Y - 1; y++)
    {
        for (x = 1; x < AUTO_MAX_X - 1; x++)
        {
            /* Skip "boring" grids */
            if (!borg_flow_dark_interesting(y, x, b_stair)) continue;

            /* Skip "unreachable" grids */
            if (!borg_flow_dark_reachable(y, x)) continue;

            /* Clear the flow codes */
            borg_flow_clear();

            /* obtain the number of steps from this take to the stairs */
            cost = borg_flow_cost_stair(y, x, b_stair);

            /* Check the distance to stair for this proposed grid */
            if (cost > borg_skill[BI_CLEVEL] * 3 + 9 && borg_skill[BI_CLEVEL] < 20) continue;

            /* Careful -- Remember it */
            borg_temp_x[borg_temp_n] = x;
            borg_temp_y[borg_temp_n] = y;
            borg_temp_n++;

            /* Paranoia -- Check for overflow */
            if (borg_temp_n == AUTO_TEMP_MAX)
            {
                /* Hack -- Double break */
                y = AUTO_MAX_Y;
                x = AUTO_MAX_X;
                break;
            }
        }
    }

    /* Nothing useful */
    if (!borg_temp_n) return (false);

    /* Wipe icky codes from grids if needed */
    if (goal_ignoring || scaryguy_on_level) borg_danger_wipe = true;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < borg_temp_n; i++)
    {
        y = borg_temp_y[i];
        x = borg_temp_x[i];

        /* Enqueue the grid */
        borg_flow_enqueue_grid(y, x);
    }

    /* Spread the flow */
    if (borg_skill[BI_CLEVEL] <= 5 && avoidance <= borg_skill[BI_CURHP])
    {
        /* Short Leash */
        borg_flow_spread(leash, true, true, false, -1, false);
    }
    else if (borg_skill[BI_CLEVEL] <= 30 && avoidance <= borg_skill[BI_CURHP])
    {
        /* Short Leash */
        borg_flow_spread(leash, true, true, false, -1, false);
    }
    else
    {
        /* Long Leash */
        borg_flow_spread(250, true, true, false, -1, false);
    }

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("dark-5", GOAL_DARK)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_DARK)) return (false);

    /* Success */
    return (true);
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
    if (borg_morgoth_position && morgoth_on_level) return (false);

    /* Paranoia */
    if (borg_flow_dark_interesting(c_y, c_x, -1))
    {
        return (false);
    }

    /* Check distance away from stairs, used later */
    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* Near */
    if (neer)
    {
        /* Method 1 */
        if (borg_flow_dark_1(b_stair)) return (true);

        /* Method 2 */
        if (borg_flow_dark_2(b_stair)) return (true);

        /* Method 3 */
        if (borg_flow_dark_3(b_stair)) return (true);
    }
    /* Far */
    else
    {
        /* Method 4 */
        if (borg_flow_dark_4(b_stair)) return (true);

        /* Method 5 */
        if (borg_flow_dark_5(b_stair)) return (true);
    }

    /* Fail */
    return (false);
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

    int b_x = c_x;
    int b_y = c_y;
    int b_v = -1;
    int j, b_j = -1;
    int b_stair = -1;

    borg_grid* ag;


    /* Hack -- not in town */
    if (!borg_skill[BI_CDEPTH]) return (false);

    /* Hack -- Not if starving */
    if (borg_skill[BI_ISWEAK]) return (false);

    /* Hack -- Not if hopeless unless twitchy */
    if (borg_t - borg_began > 3000 && avoidance <= borg_skill[BI_CURHP]) return (false);

    /* Not bored */
    if (!bored)
    {
        /* Look around for danger */
        int p = borg_danger(c_y, c_x, 1, true, false);

        /* Avoid searching when in danger */
        if (p > avoidance / 4) return (false);
    }

    /* Check distance away from stairs, used later */
    /* Check for an existing "up stairs" */
    for (i = 0; i < track_less.num; i++)
    {
        x = track_less.x[i];
        y = track_less.y[i];

        /* How far is the nearest up stairs */
        j = borg_distance(c_y, c_x, y, x);

        /* skip the closer ones */
        if (b_j >= j) continue;

        /* track it */
        b_j = j;
        b_stair = i;
    }

    /* We have arrived */
    if ((spastic_x == c_x) && (spastic_y == c_y))
    {
        /* Cancel */
        spastic_x = 0;
        spastic_y = 0;

        ag = &borg_grids[c_y][c_x];

        /* Take note */
        borg_note(format("# Spastic Searching at (%d,%d)...value:%d", c_x, c_y, ag->xtra));

        /* Count searching */
        for (i = 0; i < 9; i++)
        {
            /* Extract the location */
            int xx = c_x + ddx_ddd[i];
            int yy = c_y + ddy_ddd[i];

            /* Current grid */
            ag = &borg_grids[yy][xx];

            /* Tweak -- Remember the search */
            if (ag->xtra < 100) ag->xtra += 5;
        }

        /* we searched here */
        return (false);
    }


    /* Reverse flow */
    borg_flow_reverse(250, true, false, false, -1, false);

    /* Scan the entire map */
    for (y = 1; y < AUTO_MAX_Y - 1; y++)
    {
        for (x = 1; x < AUTO_MAX_X - 1; x++)
        {
            borg_grid* ag_ptr[8];

            int wall = 0;
            int supp = 0;
            int diag = 0;
            int monsters = 0;

            /* Acquire the grid */
            ag = &borg_grids[y][x];

            /* Skip unknown grids */
            if (ag->feat == FEAT_NONE) continue;

            /* Skip trap grids */
            if (ag->trap) continue;

            /* Skip walls/doors */
            if (!borg_cave_floor_grid(ag)) continue;

            /* Acquire the cost */
            cost = borg_data_cost->data[y][x];

            /* Skip "unreachable" grids */
            if (cost >= 250) continue;

            /* Skip grids that are really far away.  He probably
             * won't find anything and it takes lots of turns
             */
            if (cost >= 25 && borg_skill[BI_CLEVEL] < 30) continue;
            if (cost >= 50) continue;

            /* Tweak -- Limit total searches */
            if (ag->xtra >= 50) continue;
            if (ag->xtra >= borg_skill[BI_CLEVEL]) continue;

            /* Limit initial searches until bored */
            if (!bored && (ag->xtra > 5)) continue;

            /* Avoid searching detected sectors */
            if (borg_detect_door[y / borg_panel_hgt()][x / borg_panel_wid()]) continue;

            /* Skip ones that make me wander too far unless twitchy (Leash)*/
            if (b_stair != -1 && borg_skill[BI_CLEVEL] < 15 &&
                avoidance <= borg_skill[BI_CURHP])
            {
                /* Check the distance of this grid to the stair */
                j = borg_distance(track_less.y[b_stair], track_less.x[b_stair],
                    y, x);
                /* Distance of me to the stairs */
                b_j = borg_distance(c_y, c_x, track_less.y[b_stair], track_less.x[b_stair]);

                /* skip far away grids while I am close to stair*/
                if (b_j <= borg_skill[BI_CLEVEL] * 3 + 9 &&
                    j >= borg_skill[BI_CLEVEL] * 3 + 9) continue;

                /* If really low level don't do this much */
                if (borg_skill[BI_CLEVEL] <= 3 &&
                    b_j <= borg_skill[BI_CLEVEL] + 9 &&
                    j >= borg_skill[BI_CLEVEL] + 9) continue;

                /* Do not Venture too far from stair */
                if (borg_skill[BI_CLEVEL] <= 3 &&
                    j >= borg_skill[BI_CLEVEL] + 5) continue;

                /* Do not Venture too far from stair */
                if (borg_skill[BI_CLEVEL] <= 10 &&
                    j >= borg_skill[BI_CLEVEL] + 9) continue;
            }


            /* Extract adjacent locations */
            for (i = 0; i < 8; i++)
            {
                /* Extract the location */
                int xx = x + ddx_ddd[i];
                int yy = y + ddy_ddd[i];

                /* Get the grid contents */
                ag_ptr[i] = &borg_grids[yy][xx];
            }


            /* Count possible door locations */
            for (i = 0; i < 4; i++)
            {
                ag = ag_ptr[i];
                if (ag->feat >= FEAT_GRANITE) wall++;
            }

            /* No possible secret doors */
            if (wall < 1) continue;


            /* Count supporting evidence for secret doors */
            for (i = 0; i < 4; i++)
            {
                ag = ag_ptr[i];

                /* Rubble */
                if (ag->feat == FEAT_RUBBLE) continue;

                /* Walls, Doors */
                if (((ag->feat >= FEAT_SECRET) && (ag->feat <= FEAT_GRANITE)) ||
                    ((ag->feat == FEAT_OPEN) || (ag->feat == FEAT_BROKEN)) ||
                    (ag->feat == FEAT_CLOSED))
                {
                    supp++;
                }
            }

            /* Count supporting evidence for secret doors */
            for (i = 4; i < 8; i++)
            {
                ag = ag_ptr[i];

                /* Rubble */
                if (ag->feat == FEAT_RUBBLE) continue;

                /* Walls */
                if (ag->feat >= FEAT_SECRET)
                {
                    diag++;
                }
            }

            /* No possible secret doors */
            if (diag < 2) continue;

            /* Count monsters */
            for (i = 0; i < 8; i++)
            {
                ag = ag_ptr[i];

                /* monster */
                if (ag->kill) monsters++;
            }

            /* No search near monsters */
            if (monsters >= 1) continue;

            /* Tweak -- Reward walls, punish visitation, distance, time on level */
            v = (supp * 500) + (diag * 100) - (ag->xtra * 40) - (cost * 2) - (borg_t - borg_began);

            /* Punish low level and searching too much */
            v -= (50 - borg_skill[BI_CLEVEL]) * 5;

            /* The grid is not searchable */
            if (v <= 0) continue;


            /* Tweak -- Minimal interest until bored */
            if (!bored && (v < 1500)) continue;


            /* Track "best" grid */
            if ((b_v >= 0) && (v < b_v)) continue;

            /* Save the data */
            b_v = v; b_x = x; b_y = y;
        }
    }

    /* Clear the flow codes */
    borg_flow_clear();

    /* Hack -- Nothing found */
    if (b_v < 0) return (false);


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
    if (!borg_flow_commit("spastic", GOAL_XTRA)) return (false);

    /* Take one step */
    if (!borg_flow_old(GOAL_XTRA)) return (false);

    /* Success */
    return (true);
}




/*
 * Initialize this file
 */
void borg_init_6(void)
{
    /* Nothing */
}

/*
 * Release resources allocated by borg_init_6().
 */
void borg_clean_6(void)
{
    /* Nothing */
}

#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif /* ALLOW_BORG */
