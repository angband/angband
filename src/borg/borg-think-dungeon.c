/**
 * \file borg-think-dungeon.c
 * \brief Decide on an action while in the dungeon
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

#include "borg-think-dungeon.h"

#ifdef ALLOW_BORG

#include "../ui-command.h"
#include "../ui-menu.h"

#include "borg-attack-munchkin.h"
#include "borg-caution.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-escape.h"
#include "borg-fight-attack.h"
#include "borg-fight-defend.h"
#include "borg-fight-perm.h"
#include "borg-flow-dark.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow-take.h"
#include "borg-flow.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-enchant.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-item-wear.h"
#include "borg-junk.h"
#include "borg-light.h"
#include "borg-magic-play.h"
#include "borg-magic.h"
#include "borg-prepared.h"
#include "borg-projection.h"
#include "borg-recover.h"
#include "borg-store-sell.h"
#include "borg-store.h"
#include "borg-think-dungeon-util.h"
#include "borg-think-store.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg.h"

#ifdef BABLOS
extern bool borg_clock_over;
#endif /* bablos */

/*
 * Current level "feeling"
 */
int borg_feeling_danger = 0;
int borg_feeling_stuff  = 0;

/*
 * This is an exploitation function.  The borg will stair scum
 * in the dungeon to get to the bottom of the dungeon asap.
 * Once down there, he can be told to do something.
 *
 * Dive if stairs are close and safe.
 * Retreat off level if monster has LOS to borg.
 * Fill Lantern
 * Eat food
 * Call light.  might be dangerous because monster get a chance to hit us.
 */
static bool borg_think_dungeon_lunal(void)
{
    bool safe_place = false;

    int j, b_j = -1;
    int i;

    borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

    uint8_t feat  = square(cave, borg.c)->feat;

    enum borg_need need;

    /* examine equipment and swaps */
    borg_notice(true);

    /* No Lunal mode if starving or in town */
    if (borg.trait[BI_CDEPTH] == 0 || borg.trait[BI_ISWEAK]) {
        borg_note("# Leaving Lunal Mode. (Town or Weak)");
        borg.lunal_mode = false;
        return false;
    }

    /* if borg is just starting on this level, he may not
     * know that a stair is under him.  Cheat to see if one is
     * there
     */
    if (feat == FEAT_MORE && ag->feat != FEAT_MORE) {

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++) {
            /* We already knew about that one */
            if ((track_more.x[i] == borg.c.x) && (track_more.y[i] == borg.c.y))
                break;
        }

        /* Track the newly discovered "down stairs" */
        if ((i == track_more.num) && (i < track_more.size)) {
            track_more.x[i] = borg.c.x;
            track_more.y[i] = borg.c.y;
            track_more.num++;
        }
        /* tell the array */
        ag->feat = FEAT_MORE;
    }

    if (feat == FEAT_LESS && ag->feat != FEAT_LESS) {

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            /* We already knew about this one */
            if ((track_less.x[i] == borg.c.x) && (track_less.y[i] == borg.c.y))
                continue;
        }

        /* Track the newly discovered "up stairs" */
        if ((i == track_less.num) && (i < track_less.size)) {
            track_less.x[i] = borg.c.x;
            track_less.y[i] = borg.c.y;
            track_less.num++;
        }

        /* Tell the array */
        ag->feat = FEAT_LESS;
    }

    /* Act normal on 1 unless stairs are seen*/
    if (borg.trait[BI_CDEPTH] == 1 && track_more.num == 0) {
        borg.lunal_mode = false;
        return false;
    }

    /* If no down stair is known, act normal */
    if (track_more.num == 0 && track_less.num == 0) {
        borg_note("# Leaving Lunal Mode. (No Stairs seen)");
        borg.lunal_mode = false;
        return false;
    }

    /* If self scumming and getting closer to zone, act normal */
    if (borg_cfg[BORG_SELF_LUNAL]) {
        if (borg.trait[BI_MAXDEPTH] <= borg.trait[BI_CDEPTH] + 15
            || (char *)NULL != borg_prepared(borg.trait[BI_CDEPTH] - 5)
            || borg.trait[BI_CDEPTH] >= 50 || borg.trait[BI_CDEPTH] == 0
            || borg.trait[BI_ISWEAK]) {
            borg.lunal_mode         = false;
            borg.goal.fleeing       = false;
            borg.goal.fleeing_lunal = false;
            borg_note("# Self Lunal mode disengaged normally.");
            return false;
        }
    }

    /** First deal with staying alive **/

    /* Hack -- require light */
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note("# Lunal. (need fuel)");

    /* No Light at all */
    if (borg.trait[BI_CURLITE] == 0 && borg_items[INVEN_LIGHT].tval == 0) {
        borg_note("# No Light at all.");
        return false;
    }

    /* Define if safe_place is true or not */
    safe_place = borg_check_rest(borg.c.y, borg.c.x);

    /* Light Room, looking for monsters */
    /* if (safe_place && borg_check_LIGHT_only()) return true; */

    /* Check for stairs and doors and such */
    /* if (safe_place && borg_check_LIGHT()) return true; */

    /* Recover from any nasty condition */
    if (safe_place && borg_recover())
        return true;

    /* Consume needed things */
    if (safe_place && borg_use_things())
        return true;

    /* Consume needed things */
    if (borg.trait[BI_ISHUNGRY] && borg_use_things())
        return true;

    /* Crush junk if convenient */
    if (safe_place && borg_drop_junk())
        return true;

    /** Track down some interesting gear **/
    /* XXX Should we allow him great flexibility in retrieving loot? (not always
     * safe?)*/
    /* Continue flowing towards objects */
    if (safe_place && borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a (viewable) object */
    if (safe_place && borg_flow_take_lunal(true, 4))
        return true;

    /*leave level right away. */
    borg_note("# Fleeing level. Lunal Mode");
    borg.goal.fleeing_lunal = true;
    borg.goal.fleeing       = true;

    /* Full of Items - Going up */
    if (track_less.num && borg_items[PACK_SLOTS - 2].iqty
        && (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
        int y, x;
        int closeness     = 8;

        borg_grid *tmp_ag = &borg_grids[borg.c.y][borg.c.x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* if on depth 1, try to venture more to get back to town */
        if (borg.trait[BI_CDEPTH] == 1) {
            if (track_less.num) {
                closeness = 20;
            }
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) || tmp_ag->feat == FEAT_LESS) {

            /* Note */
            borg_note("# Lunal Mode.  Power Climb (needing to sell). ");

            /* Set to help borg move better */
            borg.goal.less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                borg_note(
                    "# Looking for stairs. Lunal Mode (needing to sell).");

                /* Success */
                return true;
            }

            if (tmp_ag->feat == FEAT_LESS) {
                /* Take the Up Stair */
                borg_keypress('<');
                return true;
            }
        }
    }

    /* Lunal Mode - Going down */
    if (track_more.num
        && (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS
            || borg.trait[BI_CDEPTH] < 30)) {
        int y, x;

        if (track_more.num >= 2)
            borg_note("# Lunal Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++) {
            x = track_more.x[i];
            y = track_more.y[i];

            /* How far is the nearest down stairs */
            j = distance(borg.c, loc(x, y));

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* if the downstair is close and path is safe, continue on */
        if ((b_j < 8 && safe_place) || ag->feat == FEAT_MORE
            || borg.trait[BI_CDEPTH] < 30) {
            /* Note */
            borg_note("# Lunal Mode.  Power Diving. ");

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to DownStair */
            if (borg_flow_stair_more(GOAL_FLEE, true, false))
                return true;

            /* if standing on a stair */
            if (ag->feat == FEAT_MORE) {
                /* Take the downstairs */
                borg_keypress('>');

                return true;
            }
        }
    }

    /* Lunal Mode - Going up */
    if (track_less.num && borg.trait[BI_CDEPTH] != 1
        && (safe_place || ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
        int y, x;

        borg_grid *tmp_ag = &borg_grids[borg.c.y][borg.c.x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < 8 && safe_place) || tmp_ag->feat == FEAT_LESS) {

            /* Note */
            borg_note("# Lunal Mode.  Power Climb. ");

            /* Set to help borg move better */
            borg.goal.less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                borg_note("# Looking for stairs. Lunal Mode.");

                /* Success */
                return true;
            }

            if (tmp_ag->feat == FEAT_LESS) {
                /* Take the Up Stair */
                borg_keypress('<');
                return true;
            }
        }
    }

    /* Special case where the borg is off a stair and there
     * is a monster in LOS.  He could freeze and unhook, or
     * move to the closest stair and risk the run.
     */
    if (borg.trait[BI_CDEPTH] >= 2) {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Note */
        borg_note("# Lunal Mode.  Any Stair. ");

        /* Try to find some stairs */
        if (borg_flow_stair_both(GOAL_FLEE, true))
            return true;
    }

    /* Lunal Mode - Reached 99 */
    if (borg.trait[BI_CDEPTH] == 99) {
        borg_note("# Lunal Mode ended at depth.");
    }

    /* Unable to do it */
    if (borg.trait[BI_CDEPTH] > 1) {
        borg_note("# Lunal Mode ended incorrectly.");
    }

    /* return to normal borg_think_dungeon */
    borg_note("Leaving Lunal Mode. (End of Lunal Mode)");
    borg.lunal_mode   = false;
    borg.goal.fleeing = borg.goal.fleeing_lunal = false;
    return false;
}

/* This is an exploitation function.  The borg will stair scum
 * in the dungeon to get to a sweet spot to gather items.
 *
 * Dive if stairs are close and safe.
 * Retreat off level if monster has LOS to borg.
 * Fill Lantern
 * Eat food
 * Call light.  might be dangerous because monster get a chance to hit us.
 */
static bool borg_think_dungeon_munchkin(void)
{
    bool safe_place = false;
    int  bb_j       = z_info->max_range;
    int  j, b_j = -1;
    int  i, ii, x, y;
    int  closeness = 8;

    borg_grid *ag  = &borg_grids[borg.c.y][borg.c.x];

    uint8_t feat   = square(cave, borg.c)->feat;

    enum borg_need need;

    /* examine equipment and swaps */
    borg_notice(true);

    /* Not if starving or in town */
    if (borg.trait[BI_CDEPTH] == 0 || borg.trait[BI_ISWEAK]) {
        borg_note("# Leaving munchkin Mode. (Town or Weak)");
        borg.munchkin_mode = false;
        return false;
    }

    /* if borg is just starting on this level, he may not
     * know that a stair is under him.  Cheat to see if one is
     * there
     */
    if (feat == FEAT_MORE && ag->feat != FEAT_MORE) {

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++) {
            /* We already knew about that one */
            if ((track_more.x[i] == borg.c.x) && (track_more.y[i] == borg.c.y))
                break;
        }

        /* Track the newly discovered "down stairs" */
        if ((i == track_more.num) && (i < track_more.size)) {
            track_more.x[i] = borg.c.x;
            track_more.y[i] = borg.c.y;
            track_more.num++;
        }
        /* tell the array */
        ag->feat = FEAT_MORE;
    }

    if (feat == FEAT_LESS && ag->feat != FEAT_LESS) {

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            /* We already knew about this one */
            if ((track_less.x[i] == borg.c.x) && (track_less.y[i] == borg.c.y))
                continue;
        }

        /* Track the newly discovered "up stairs" */
        if ((i == track_less.num) && (i < track_less.size)) {
            track_less.x[i] = borg.c.x;
            track_less.y[i] = borg.c.y;
            track_less.num++;
        }

        /* Tell the array */
        ag->feat = FEAT_LESS;
    }

    /* Act normal on 1 unless stairs are seen*/
    if (borg.trait[BI_CDEPTH] == 1 && track_more.num == 0) {
        borg.munchkin_mode = false;
        return false;
    }

    /* If no down stair is known, act normal */
    if (track_more.num == 0 && track_less.num == 0) {
        borg_note("# Leaving Munchkin Mode. (No Stairs seen)");
        borg.munchkin_mode = false;
        return false;
    }

    /** First deal with staying alive **/

    /* Hack -- require light */
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note("# Munchkin. (need fuel)");

    /* No Light at all */
    if (borg.trait[BI_CURLITE] == 0) {
        borg_note("# No Light at all.");
    }

    /* Define if safe_place is true or not */
    safe_place = borg_check_rest(borg.c.y, borg.c.x);

    /* Can do a little attacking. */
    if (borg_munchkin_mage())
        return true;
    if (borg_munchkin_melee())
        return true;

    /* Consume needed things */
    if (safe_place && borg_use_things())
        return true;

    /* Consume needed things */
    if (borg.trait[BI_ISHUNGRY] && borg_use_things())
        return true;

    /* Wear stuff and see if it's good */
    if (safe_place && borg_wear_stuff())
        return true;

    if (safe_place && borg_remove_stuff())
        return true;

    /* Crush junk if convenient */
    if (safe_place && borg_drop_junk())
        return true;

    /* Learn learn and test useful spells */
    if (safe_place && borg_play_magic(true))
        return true;

    /** Track down some interesting gear **/
    /* XXX Should we allow him great flexibility in retrieving loot? (not always
     * safe?)*/
    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Borg may be off the stair and a monster showed up. */

    /* Find a (viewable) object */
    if (safe_place && borg_flow_take_lunal(true, 5))
        return true;

    /* Recover from any nasty condition */
    if (safe_place && borg_recover())
        return true;

    /*leave level right away. */
    borg_note("# Fleeing level. Munchkin Mode");
    borg.goal.fleeing_munchkin = true;
    borg.goal.fleeing          = true;

    /* Increase the range of the borg a bit */
    if (borg.trait[BI_CDEPTH] <= 10)
        closeness
            += (borg.trait[BI_CLEVEL] - 10) + (10 - borg.trait[BI_CDEPTH]);

    /* Full of Items - Going up */
    if (track_less.num && (borg_items[PACK_SLOTS - 2].iqty)
        && (safe_place || ag->feat == FEAT_LESS
            || borg.trait[BI_CURLITE] == 0)) {
        borg_grid *tmp_ag = &borg_grids[borg.c.y][borg.c.x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, borg.c.y, borg.c.x))
                continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* if on depth 1, try to venture more to get back to town */
        if (borg.trait[BI_CDEPTH] == 1) {
            if (track_less.num) {
                closeness = 20;
            }
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) || tmp_ag->feat == FEAT_LESS) {

            /* Note */
            borg_note("# Munchkin Mode.  Power Climb (needing to sell). ");

            /* Set to help borg move better */
            borg.goal.less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, true)) {
                borg_note(
                    "# Looking for stairs. Munchkin Mode (needing to sell).");

                /* Success */
                return true;
            }

            if (tmp_ag->feat == FEAT_LESS) {
                /* Take the Up Stair */
                borg_keypress('<');
                return true;
            }
        }
    }

    /* Too deep. trying to gradually move shallow.  Going up */
    if ((track_less.num
            && borg.trait[BI_CDEPTH] > borg_cfg[BORG_MUNCHKIN_DEPTH])
        && (safe_place || ag->feat == FEAT_LESS)) {

        borg_grid *tmp_ag = &borg_grids[borg.c.y][borg.c.x];

        /* Reset */
        b_j = -1;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, borg.c.y, borg.c.x))
                continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
            if (b_j < bb_j)
                bb_j = b_j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) || tmp_ag->feat == FEAT_LESS) {

            /* Note */
            borg_note("# Munchkin Mode.  Power Climb. ");

            /* Set to help borg move better */
            borg.goal.less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, true)) {
                borg_note("# Looking for stairs. Munchkin Mode.");

                /* Success */
                return true;
            }

            if (tmp_ag->feat == FEAT_LESS) {
                /* Take the Up Stair */
                borg_keypress('<');
                return true;
            }
        }
    }

    /* Going down */
    if ((track_more.num
            && borg.trait[BI_CDEPTH] < borg_cfg[BORG_MUNCHKIN_DEPTH])
        && (safe_place || ag->feat == FEAT_MORE)) {
        /* Reset */
        b_j = -1;

        if (track_more.num >= 1)
            borg_note("# Munchkin Mode: I know of a down stair.");

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++) {
            x = track_more.x[i];
            y = track_more.y[i];

            /* How far is the nearest down stairs */
            j = distance(borg.c, loc(x, y));

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, borg.c.y, borg.c.x))
                continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* if the downstair is close and path is safe, continue on */
        if ((b_j < closeness && safe_place) || ag->feat == FEAT_MORE
            || borg.trait[BI_CDEPTH] == 1) {
            /* Note */
            borg_note("# Munchkin Mode.  Power Diving. ");

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to DownStair */
            if (borg_flow_stair_more(GOAL_FLEE, true, false))
                return true;

            /* if standing on a stair */
            if (ag->feat == FEAT_MORE) {
                /* Take the DownStair */
                borg_keypress('>');

                return true;
            }
        }
    }

    /* Going up */
    if ((track_less.num && borg.trait[BI_CDEPTH] != 1 && safe_place)
        || ag->feat == FEAT_LESS) {
        borg_grid *tmp_ag = &borg_grids[borg.c.y][borg.c.x];

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* Is it reachable or behind a wall? */
            if (!borg_projectable(y, x, borg.c.y, borg.c.x))
                continue;

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* if the upstair is close and safe path, continue */
        if ((b_j < closeness && safe_place) || tmp_ag->feat == FEAT_LESS) {

            /* Note */
            borg_note("# Munchkin Mode.  Power Climb. ");

            /* Set to help borg move better */
            borg.goal.less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, true)) {
                borg_note("# Looking for stairs. Munchkin Mode.");

                /* Success */
                return true;
            }

            if (tmp_ag->feat == FEAT_LESS) {
                /* Take the Up Stair */
                borg_keypress('<');
                return true;
            }
        }
    }

    /* Special case where the borg is off a stair and there
     * is a monster in LOS.  He could freeze and unhook, or
     * move to the closest stair and risk the run.
     */
    if (borg.trait[BI_CDEPTH] >= 2 || !safe_place) {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Note */
        borg_note("# Munchkin Mode.  Any Stair. ");

        /* Adjacent Monster.  Either attack it, or try to outrun it */
        for (i = 0; i < 8; i++) {
            y = borg.c.y + ddy_ddd[i];
            x = borg.c.x + ddx_ddd[i];

            /* Bounds check */
            if (!square_in_bounds(cave, loc(x, y)))
                continue;

            /* Get the grid */
            ag = &borg_grids[y][x];

            /* Monster is adjacent to the borg */
            if (ag->kill) {
                /* Check for an existing "up stairs" */
                for (ii = 0; ii < track_less.num; ii++) {
                    x = track_less.x[ii];
                    y = track_less.y[ii];

                    /* How far is the nearest up stairs */
                    j = distance(borg.c, loc(x, y));

                    /* Is it reachable or behind a wall? */
                    if (!borg_projectable(y, x, borg.c.y, borg.c.x))
                        continue;

                    /* skip the far ones */
                    if (b_j <= j && b_j != -1)
                        continue;

                    /* track it */
                    b_j = j;
                }

                /* Check for an existing "down stairs" */
                for (ii = 0; ii < track_more.num; ii++) {
                    x = track_more.x[ii];
                    y = track_more.y[ii];

                    /* How far is the nearest down stairs */
                    j = distance(borg.c, loc(x, y));

                    /* Is it reachable or behind a wall? */
                    if (!borg_projectable(y, x, borg.c.y, borg.c.x))
                        continue;

                    /* skip the far ones */
                    if (b_j <= j && b_j != -1)
                        continue;

                    /* track it */
                    b_j = j;
                }

                /* Can the borg risk the run? */
                if (b_j <= 3) {
                    /* Try to find some stairs */
                    if (borg_flow_stair_both(GOAL_FLEE, false))
                        return true;
                } else {
                    /* Try to kill it */
                    if (borg_attack(false))
                        return true;
                }
            } /* Adjacent to kill */
        } /* Scanning neighboring grids */

        /* Try to find some stairs */
        if (borg_flow_stair_both(GOAL_FLEE, false))
            return true;
        if (ag->feat == FEAT_LESS) {
            /* Take the Up Stair */
            borg_keypress('<');
            return true;
        }
        if (ag->feat == FEAT_MORE) {
            /* Take the Stair */
            borg_keypress('>');
            return true;
        }
    }

    /* return to normal borg_think_dungeon */
    borg_note("Leaving Munchkin Mode. (End of Mode)");
    borg.munchkin_mode = false;
    borg.goal.fleeing = borg.goal.fleeing_munchkin = false;
    return false;
}

/*
 * Hack -- perform an action in the dungeon under boosted bravery
 *
 * This function is a sub-set of the standard dungeon goals, and is
 * only executed when all of the standard dungeon goals fail, because
 * of excessive danger, or because the level is "bizarre".
 */
static bool borg_think_dungeon_brave(void)
{
    /*** Local stuff ***/
    int p1 = borg_danger(borg.c.y, borg.c.x, 1, true, false);

    /* Try a defense maneuver on 100 */
    if (borg.trait[BI_CDEPTH] == 100 && borg_defend(p1))
        return true;

    /* Attack monsters */
    if (borg_attack(true))
        return true;

    /* Cast a light beam to remove fear of an area */
    if (borg_light_beam(false))
        return true;

    /*** Flee (or leave) the level ***/

    /* Take stairs down */
    /* Usable stairs */
    if (borg_grids[borg.c.y][borg.c.x].feat == FEAT_MORE) {
        /* Take the stairs */
        borg_note("# Fleeing via stairs.");
        borg_keypress('>');

        /* Success */
        return true;
    }

    /* Return to Stairs, but not use them */
    if (borg.goal.less) {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs */
        if (scaryguy_on_level && !borg.trait[BI_CDEPTH]
            && borg_flow_stair_both(GOAL_FLEE, false))
            return true;

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, false)) {
            borg_note("# Looking for stairs. Goal_less, brave.");
            return true;
        }
    }

    /* Flee the level */
    if (borg.goal.fleeing || borg.goal.leaving || scaryguy_on_level) {
        /* Hack -- Take the next stairs */
        borg.stair_less = borg.goal.fleeing;

        if (borg.ready_morgoth == 0)
            borg.stair_less = true;

        if (borg.stair_less == true) {
            borg_note("# Fleeing and leaving the level. Brave Thinking.");
        }

        /* Go down if fleeing or prepared. */
        borg.stair_more = borg.goal.fleeing;
        if ((char *)NULL == borg_prepared(borg.trait[BI_CDEPTH] + 1))
            borg.stair_more = true;

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs up */
        if (borg.stair_less)
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                borg_note("# Looking for stairs. Flee, brave.");
                return true;
            }

        /* Try to find some stairs down */
        if (borg.stair_more)
            if (borg_flow_stair_more(GOAL_FLEE, false, true))
                return true;
    }

    /* Do short looks on special levels */
    if (vault_on_level) {
        /* Continue flowing towards monsters */
        if (borg_flow_old(GOAL_KILL))
            return true;

        /* Find a (viewable) monster */
        if (borg_flow_kill(true, 35))
            return true;

        /* Continue flowing towards objects */
        if (borg_flow_old(GOAL_TAKE))
            return true;

        /* Find a (viewable) object */
        if (borg_flow_take(true, 35))
            return true;
        if (borg_flow_vein(true, 35))
            return true;

        /* Continue to dig out a vault */
        if (borg_flow_old(GOAL_VAULT))
            return true;

        /* Find a vault to excavate */
        if (borg_flow_vault(35))
            return true;
    }

    /* Continue flowing towards monsters */
    if (borg_flow_old(GOAL_KILL))
        return true;

    /* Find a (viewable) monster */
    if (borg_flow_kill(true, 250))
        return true;

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a (viewable) object */
    if (borg_flow_take(true, 250))
        return true;
    if (borg_flow_vein(true, 250))
        return true;

    /*** Exploration ***/

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_MISC))
        return true;

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_DARK))
        return true;

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_XTRA))
        return true;

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_BORE))
        return true;

    /*** Explore the dungeon ***/

    /* Explore interesting grids */
    if (borg_flow_dark(true))
        return true;

    /* Explore interesting grids */
    if (borg_flow_dark(false))
        return true;

    /* Search for secret door via spell before spastic */
    if (!borg.when_detect_doors || (borg_t - borg.when_detect_doors >= 500)) {
        if (borg_check_light())
            return true;
    }

    /*** Track down old stuff ***/

    /* Chase old objects */
    if (borg_flow_take(false, 250))
        return true;
    if (borg_flow_vein(false, 250))
        return true;

    /* Chase old monsters */
    if (borg_flow_kill(false, 250))
        return true;

    /* Search for secret door via spell before spastic */
    if (!borg.when_detect_doors || (borg_t - borg.when_detect_doors >= 500)) {
        if (borg_check_light())
            return true;
    }

    /* Attempt to leave the level */
    if (borg_leave_level(true))
        return true;

    /* Search for secret doors */
    if (borg_flow_spastic(true))
        return true;

    /* Nothing */
    return false;
}

/*
 * Perform an action in the dungeon
 *
 * Return true if a "meaningful" action was performed
 * Otherwise, return false so we will be called again
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 *   Explore the dungeon and revisit old grids
 *
 * Fleeing:
 *   Use word of recall when level is "scary"
 *   Flee to stairs when there is a chance of death
 *   Avoid "stair bouncing" if at all possible
 *
 * Note that the various "flow" actions allow the Borg to flow
 * "through" closed doors, which will be opened when he attempts
 * to pass through them, so we do not have to pursue terrain until
 * all monsters and objects have been dealt with.
 *
 * XXX XXX XXX The poor Borg often kills a nasty monster, and
 * then takes a nap to recover from damage, but gets yanked
 * back to town before he can collect his reward.
 */
bool borg_think_dungeon(void)
{
    int i, j;
    int b_j = -1;

    /* Delay Factor */
    int msec = ((player->opts.delay_factor * player->opts.delay_factor)
                + (borg_cfg[BORG_DELAY_FACTOR] * borg_cfg[BORG_DELAY_FACTOR]));

    /* HACK allows user to stop the borg on certain levels */
    if (borg.trait[BI_CDEPTH] == borg_cfg[BORG_STOP_DLEVEL])
        borg_oops("Auto-stop for user DLevel.");

    if (borg.trait[BI_CLEVEL] == borg_cfg[BORG_STOP_CLEVEL])
        borg_oops("Auto-stop for user CLevel.");

    /* HACK to end all hacks,,, allow the borg to stop if money scumming */
    if (borg.trait[BI_GOLD] > borg_cfg[BORG_MONEY_SCUM_AMOUNT]
        && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 && !borg.trait[BI_CDEPTH]
        && !borg_cfg[BORG_SELF_SCUM]) {
        borg_oops("Money Scum complete.");
    }

    /* Hack -- Stop the borg if money scumming and the shops are out of food. */
    if (!borg.trait[BI_CDEPTH] && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0
        && (borg_food_onsale == 0 && borg.trait[BI_FOOD] < 5)) {
        /* Town out of food.  If player initiated borg, stop here */
        if (borg_cfg[BORG_SELF_SCUM] == false) {
            borg_oops("Money Scum stopped.  No more food in shop.");
            return true;
        } else
        /* Borg doing it himself */
        {
            /* move money goal to 0 and leave the level */
            borg_cfg[BORG_MONEY_SCUM_AMOUNT] = 0;
        }
    }

    /* Hack -- prevent clock wrapping Step 1*/
    if ((borg_t >= 12000 && borg_t <= 12025)
        || (borg_t >= 25000 && borg_t <= 25025)) {
        /* Clear Possible errors */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Re-examine inven and equip */
        borg_do_inven = true;
        borg_do_equip = true;

        /* enter a special routine to handle this behavior.  Messing with
         * the old_level forces him to re-explore this level, and reshop,
         * if in town.
         */
        old_depth = 126;

        /* Continue on */
        return true;
    }

    /* if standing on something valueless, destroy it */
    if (borg_destroy_floor())
        return true;
 
    /* Hack -- prevent clock wrapping Step 2*/
    if (borg_t >= 30000) {
        /* Panic */
        borg_oops("clock overflow");

#ifdef BABLOS
        /* Clock overflow escape code */
        printf("Clock overflow code!\n");
        player->playing = false;
        player->leaving = true;
        borg_clock_over = true;
#endif /* BABLOS */

        /* Oops */
        return true;
    }

    /* Allow respawning borgs to update their variables */
    if (borg_respawning > 1) {
        borg_note(
            format("# Pressing 'escape' to catch up and get in sync (%d).",
                borg_respawning));
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_respawning--;
        return true;
    }

    /* add a short pause to slow the borg down for viewing */
    Term_xtra(TERM_XTRA_DELAY, msec);

    /* redraw the screen if we need to */
    if (my_need_redraw) {
        borg_note(format("#  Redrawing screen."));
        do_cmd_redraw();
        my_need_redraw = false;
    }

    /* Prevent clock overflow */
    if (borg_t - borg_began >= 10000) {
        /* Start leaving */
        if (!borg.goal.leaving) {
            /* Note */
            borg_note("# Leaving (boredom)");

            /* Start leaving */
            borg.goal.leaving = true;
        }

        /* Start fleeing */
        if (!borg.goal.fleeing) {
            /* Note */
            borg_note("# Fleeing (boredom)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }

    /* am I fighting a unique or a summoner, or scaryguy? */
    borg_near_monster_type(
        borg.trait[BI_MAXCLEVEL] < 15 ? z_info->max_sight : 12);

    /* Allow borg to jump back up to town if needed.  He probably fled town
     * because he saw a scaryguy (BSV, SER, Maggot).  Since he is here on depth
     * 1, do a quick check for items near the stairs that I can pick up before I
     * return to town.
     */
    if (borg.trait[BI_CDEPTH] == 1 && borg.goal.fleeing_to_town) {

        /* Try to grab a close item while I'm down here */
        if (borg_think_stair_scum(true))
            return true;

        /* Start leaving */
        if (!borg.goal.leaving) {
            /* Note */
            borg_note("# Leaving (finish shopping)");

            /* Start leaving */
            borg.goal.leaving = true;
        }

        /* Start fleeing */
        if (!borg.goal.fleeing) {
            /* Note */
            borg_note("# Fleeing (finish shopping)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }

    /* Prevent a "bouncing Borg" bug. Where borg with telepathy
     * will sit in a narrow area bouncing between 2 or 3 places
     * tracking and flowing to a bouncing monster behind a wall.
     * First, reset goals.
     * Second, clear all known monsters/takes
     * Third, Flee the level
     */
    if (borg.trait[BI_CDEPTH]
        && (borg.time_this_panel >= 300 && borg.time_this_panel <= 303)) {
        /* Clear Goals */
        borg.goal.type = 0;
    }
    if (borg.trait[BI_CDEPTH]
        && (borg.time_this_panel >= 500 && borg.time_this_panel <= 503)) {
        /* Forget old objects */
        for (i = 1; i < borg_takes_nxt; i++)
            borg_delete_take(i);

        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Forget old monsters */
        for (i = 1; i < borg_kills_nxt; i++)
            borg_delete_kill(i);

        /* No monsters here */
        borg_kills_cnt = 0;
        borg_kills_nxt = 1;
    }

    if (borg.trait[BI_CDEPTH] && (borg.time_this_panel >= 700)) {
        /* Start leaving */
        if (!borg.goal.leaving) {
            /* Note */
            borg_note("# Leaving (bouncing-borg)");

            /* Start leaving */
            borg.goal.leaving = true;
        }

        /* Start fleeing */
        if (!borg.goal.fleeing) {
            /* Note */
            borg_note("# Fleeing (bouncing-borg)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }

    /* Count the awake breeders */
    for (j = 0, i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Skip sleeping monsters */
        if (!kill->awake)
            continue;

        /* Count the monsters which are "breeders" */
        if (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY))
            j++;
    }

    /* hack -- close doors on breeder levels */
    if (j >= 3) {
        /* set the flag to close doors */
        breeder_level = true;
    }

    /* Hack -- caution from breeders*/
    if ((j >= MIN(borg.trait[BI_CLEVEL] + 2, 5))
        && (borg.trait[BI_RECALL] <= 0 || borg.trait[BI_CLEVEL] < 35)) {
        /* Ignore monsters from caution */
        if (!borg.goal.ignoring && borg_t >= 2500) {
            /* Flee */
            borg_note("# Ignoring breeders (no recall)");

            /* Ignore multipliers */
            borg.goal.ignoring = true;
        }

        /* Start leaving */
        if (!borg.goal.leaving) {
            /* Note */
            borg_note("# Leaving (no recall)");

            /* Start leaving */
            borg.goal.leaving = true;
        }

        /* Start fleeing */
        if (!borg.goal.fleeing) {
            /* Note */
            borg_note("# Fleeing (no recall)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }

    /* Reset avoidance */
    if (avoidance != borg.trait[BI_CURHP]) {
        /* Reset "avoidance" */
        avoidance = borg.trait[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Forget goals */
    }

    /* Keep borg on a short leash */
    if (track_less.num
        && (borg.trait[BI_MAXHP] < 30 || borg.trait[BI_CLEVEL] < 15)
        && borg.trait[BI_CDEPTH] >= borg.trait[BI_CLEVEL] - 5) {
        int y, x;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x = track_less.x[i];
            y = track_less.y[i];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* skip the far ones */
            if (b_j <= j && b_j != -1)
                continue;

            /* track it */
            b_j = j;
        }

        /* Return to the upstair-- too far away? */
        if ((!borg.goal.less) && b_j > borg.trait[BI_CLEVEL] * 3 + 14) {
            /* Return to Stairs */
            if (!borg.goal.less) {
                /* Note */
                borg_note(
                    format("# Return to Stair (wandered too far.  Leash: %d)",
                        borg.trait[BI_CLEVEL] * 3 + 14));

                /* Start returning */
                borg.goal.less = true;
            }

        }

        /* Clear the flag to Return to the upstair-- we are close enough now */
        else if (borg.goal.less && b_j < 3) {
            /* Note */
            borg_note("# Close enough to Stair.");

            /* Clear the flag */
            borg.goal.less = false;
            borg.goal.type = 0;
        }
    }

    /* Quick check to see if borg needs to engage his lunal mode */
    if (borg_cfg[BORG_SELF_LUNAL]
        && !borg_cfg[BORG_PLAYS_RISKY]) /* Risky borg in a hurry */
    {
        if ((char *)NULL == borg_prepared(borg.trait[BI_CDEPTH] + 15)
            && /* Prepared */
            borg.trait[BI_MAXDEPTH] >= borg.trait[BI_CDEPTH] + 15
            && /* Right zone */
            borg.trait[BI_CDEPTH] >= 1 && /* In dungeon fully */
            borg.trait[BI_CDEPTH] > borg.trait[BI_CLEVEL] / 3) /* Not shallow */
        {
            borg.lunal_mode = true;

            /* Enter the Lunal scumming mode */
            if (borg.lunal_mode && borg_think_dungeon_lunal())
                return true;
        }
    }

    /* Quick check to see if borg needs to engage his lunal mode for
     * munchkin_start */
    if (borg_cfg[BORG_MUNCHKIN_START] && borg.trait[BI_MAXCLEVEL] < 12) {
        if (borg.trait[BI_CDEPTH] >= 1) {
            borg.munchkin_mode = true;

            /* Enter the Lunal scumming mode */
            if (borg_think_dungeon_munchkin())
                return true;
        }

        /* Must not be in munchkin mode then */
        borg.munchkin_mode = false;
    }

    /* Keep borg on a suitable level */
    if (track_less.num && borg.trait[BI_CLEVEL] < 10 && !borg.goal.less
        && (char *)NULL != borg_prepared(borg.trait[BI_CDEPTH])) {
        /* Note */
        borg_note("# Needing to get back on correct depth");

        /* Start returning */
        borg.goal.less = true;

        /* Take stairs */
        if (borg_grids[borg.c.y][borg.c.x].feat == FEAT_LESS) {
            borg_keypress('<');
            return true;
        }
    }

    /*** crucial goals ***/

    /* examine equipment and swaps */
    borg_notice(true);

    /* require light-- Special handle for being out of a light source.*/
    if (borg_think_dungeon_light())
        return true;

    /* Decrease the amount of time not allowed to retreat */
    if (borg.no_retreat > 0)
        borg.no_retreat--;

    /*** Important goals ***/

    /* Continue flowing towards good anti-summon grid */
    if (borg_flow_old(GOAL_DIGGING))
        return true;

    /* Try not to die */
    if (borg_caution())
        return true;

    /*** if returning from dungeon in bad shape...***/
    if (borg.trait[BI_CURLITE] == 0 || borg.trait[BI_ISCUT]
        || borg.trait[BI_ISPOISONED] || borg.trait[BI_FOOD] == 0) {
        /* First try to wear something */
        if (borg.trait[BI_CURLITE] == 0) {
            /* attempt to refuel/swap */
            if (borg_maintain_light() == BORG_MET_NEED)
                return true;

            /* wear stuff and see if it glows */
            if (borg_wear_stuff())
                return true;
        }

        /* Recover from damage */
        if (borg_recover())
            return true;

        /* If full of items, we wont be able to buy stuff, crush stuff */
        if (borg_items[PACK_SLOTS - 1].iqty && borg_drop_hole(false))
            return true;

        if (borg_choose_shop()) {
            /* Try and visit a shop, if so desired */
            if (borg_flow_shop_entry(borg.goal.shop))
                return true;
        }
    }

    /* if I must go to town without delay */
    if ((char *)NULL != borg_restock(borg.trait[BI_CDEPTH])) {
        if (borg_leave_level(false))
            return true;
    }

    /* Learn useful spells immediately */
    if (borg_play_magic(false))
        return true;

    /* If using a digger, Wear "useful" equipment before fighting monsters */
    if (borg_items[INVEN_WIELD].tval == TV_DIGGING && borg_wear_stuff())
        return true;

    /* If not using anything, Wear "useful" equipment before fighting monsters
     */
    if (!borg_items[INVEN_WIELD].tval && borg_wear_stuff())
        return true;

    /* Dig an anti-summon corridor */
    if (borg_flow_kill_corridor(true))
        return true;

    /* Attack monsters */
    if (borg_attack(false))
        return true;

    /* Wear things that need to be worn, but try to avoid swap loops */
    /* if (borg_best_stuff()) return true; */
    if (borg_wear_stuff())
        return true;
    if (borg_swap_rings())
        return true;
    if (borg_wear_rings())
        return true;

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a really close object */
    if (borg_flow_take(true, 5))
        return true;

    /* Remove "backwards" rings */
    /* Only do this in Stores to avoid loops     if (borg_swap_rings()) return
     * (true); */

    /* Repair "backwards" rings */
    if (borg_wear_rings())
        return true;

    /* Remove stuff that is useless or detrimental */
    if (borg_remove_stuff())
        return true;
    if (borg_dump_quiver())
        return true;

    /* Check the light */
    if (borg_check_light())
        return true;

    /* Continue flowing to a safe grid on which I may recover */
    if (borg_flow_old(GOAL_RECOVER))
        return true;

    /* Recover from damage */
    if (borg_recover())
        return true;

    /* Attempt to find a grid which is safe and I can recover on it.  This
     * should work closely with borg_recover. */
    if (borg_flow_recover(false, 50))
        return true;

    /* Perform "cool" perma spells */
    if (borg_perma_spell())
        return true;

    /* Try to stick close to stairs if weak */
    if (borg.trait[BI_CLEVEL] < 10 && borg.trait[BI_MAXSP]
        && borg.trait[BI_CURSP] == 0 && borg.no_rest_prep <= 1
        && !borg.temp.bless && !borg.temp.hero && !borg.temp.berserk
        && !borg.temp.fastcast && !player_has(player, PF_COMBAT_REGEN)) {
        if (borg.trait[BI_CDEPTH]) {
            int tmp_i, y, x;

            /* Check for an existing "up stairs" */
            for (tmp_i = 0; tmp_i < track_less.num; tmp_i++) {
                x = track_less.x[tmp_i];
                y = track_less.y[tmp_i];

                /* Not on a stair */
                if (borg.c.y != y || borg.c.x != x)
                    continue;

                /* I am standing on a stair */

                /* reset the goal_less flag */
                borg.goal.less = false;

                /* if not dangerous, wait here */
                if (borg_danger(borg.c.y, borg.c.x, 1, true, false) == 0) {
                    /* rest here a moment */
                    borg_note("# Resting on stair to gain Mana.");
                    borg_keypress(',');
                    return true;
                }
            }
        } else /* in town */
        {
            int tmp_i, y, x;

            /* Check for an existing "dn stairs" */
            for (tmp_i = 0; tmp_i < track_more.num; tmp_i++) {
                x = track_more.x[tmp_i];
                y = track_more.y[tmp_i];

                /* Not on a stair */
                if (borg.c.y != y || borg.c.x != x)
                    continue;

                /* I am standing on a stair */

                /* if not dangerous, wait here */
                if (borg_danger(borg.c.y, borg.c.x, 1, true, false) == 0) {
                    /* rest here a moment */
                    borg_note("# Resting on town stair to gain Mana.");
                    borg_keypress(',');
                    return true;
                }
            }
        }

        /* In town, standing on stairs, sit tight */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, true)) {
            borg_note("# Looking for stairs. Stair hugging.");
            return true;
        }
    }

    /* If in town and have no money, and nothing to sell,
     * then do not stay in town, its too dangerous.
     */
    if (borg.trait[BI_CDEPTH] == 0 && borg.trait[BI_CLEVEL] < 6
        && borg.trait[BI_GOLD] < 10 && borg_count_sell() < 5) {
        borg_note("# Nothing to sell in town (leaving).");
        borg.goal.leaving = true;

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs down */
        if (borg_flow_stair_more(GOAL_FLEE, false, false))
            return true;
    }

    /*** Flee the level XXX XXX XXX ***/

    /* Return to Stairs, but not use them */
    if (borg.goal.less) {
        /* Continue fleeing to stair */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs */
        if (scaryguy_on_level && borg_flow_stair_both(GOAL_FLEE, false))
            return true;

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, false)) {
            borg_note("# Looking for stairs. Goal_less, Fleeing.");
            return true;
        }
    }

    /* Flee the level */
    if (borg.goal.fleeing && !borg.goal.recalling) {
        /* Hack -- Take the next stairs */
        borg.stair_less = borg.stair_more = true;
        borg_note("# Fleeing and leaving the level. (Looking for any stair)");

        /* Continue fleeing the level */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs */
        if (scaryguy_on_level && borg_flow_stair_both(GOAL_FLEE, false))
            return true;

        /* Try to find some stairs up */
        if (borg_flow_stair_less(GOAL_FLEE, false)) {
            borg_note("# Looking for stairs. Fleeing.");
            return true;
        }

        /* Try to find some stairs down */
        if (borg_flow_stair_more(GOAL_FLEE, false, false))
            return true;
    }

    /* Flee to a safe Morgoth grid if appropriate */
    if (!borg.trait[BI_KING] && morgoth_on_level && !borg_morgoth_position
        && (borg.trait[BI_AGLYPH] >= 10
            && (!borg.trait[BI_ISBLIND] && !borg.trait[BI_ISCONFUSED]))) {
        /* Continue flowing towards good morgoth grid */
        if (borg_flow_old(GOAL_MISC))
            return true;

        /* Attempt to locate a good Glyphed grid */
        if (borg_flow_glyph(GOAL_MISC))
            return true;

        /* Have the borg excavate the dungeon with Stone to Mud */
    }

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a really close object */
    if (borg_flow_take(false, 5))
        return true;
    if (borg_flow_vein(true, 5))
        return true;

    /* Continue flowing towards (the hopefully close) monsters */
    if (borg_flow_old(GOAL_KILL))
        return true;

    /* Find a really close monster */
    if (borg_flow_kill(true, 20))
        return true;

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a really close object */
    if (borg_flow_take(false, 10))
        return true;
    if (borg_flow_vein(false, 10))
        return true;

    /* Continue flowing towards monsters */
    if (borg_flow_old(GOAL_KILL))
        return true;

    /* Continue towards a vault */
    if (borg_flow_old(GOAL_VAULT))
        return true;

    /* Find a viewable monster and line up a shot on him */
    if (borg_flow_kill_aim(true))
        return true;

    /*** Deal with inventory objects ***/

    /* check for anything that should be inscribed */
    /* if (borg_inscribe_food()) return true; */

    /* Use things */
    if (borg_use_things())
        return true;

    /* Identify unknown things */
    if (borg_test_stuff())
        return true;

    /* Enchant things */
    if (borg_enchanting())
        return true;

    /* Recharge things */
    if (borg_recharging())
        return true;

    /* Drop junk */
    if (borg_drop_junk())
        return true;

    /* Drop items to make space */
    if (borg_drop_hole(false))
        return true;

    /* Drop items if we are slow */
    if (borg_drop_slow())
        return true;

    /*** Flow towards objects ***/

    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a (viewable) object */
    if (borg_flow_take(true, 250))
        return true;
    if (borg_flow_vein(true, 250))
        return true;

    /*** Leave the level XXX XXX XXX ***/

    /* Leave the level */
    if ((borg.goal.leaving && !borg.goal.recalling && !unique_on_level)
        || (borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 25
            && borg.trait[BI_GOLD] < 25000 && borg_count_sell() >= 13)) {
        /* Hack -- Take the next stairs */
        if (borg.ready_morgoth == 0) {
            borg_note(
                "# Fleeing and leaving the level (Looking for Up Stair).");
            borg.stair_less = true;
        }

        /* Only go down if fleeing or prepared. */
        if ((char *)NULL == borg_prepared(borg.trait[BI_CDEPTH] + 1))
            borg.stair_more = true;

        /* Continue leaving the level */
        if (borg_flow_old(GOAL_FLEE))
            return true;

        /* Try to find some stairs up */
        if (borg.stair_less) {
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                borg_note("# Looking for stairs. Goal_Leaving.");

                return true;
            }
        }

        /* Only go up if needing to sell */
        if (borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 25
            && borg.trait[BI_GOLD] < 25000 && borg_count_sell() >= 13)
            borg.stair_more = false;

        /* Try to find some stairs down */
        if (borg.stair_more)
            if (borg_flow_stair_more(GOAL_FLEE, false, false))
                return true;
    }

    /* Power dive if I am playing too shallow
     * This is also seen in leave_level().  If
     * this formula is modified here, change it
     * in leave_level too.
     */
    if (borg.trait[BI_CDEPTH] != 0
        && (char *)NULL == borg_prepared(borg.trait[BI_CDEPTH] + 5)
        && !borg.stair_less) {
        /* Take next stairs */
        borg.stair_more = true;

        /* Continue leaving the level */
        if (borg_flow_old(GOAL_BORE))
            return true;

        /* No down if needing to sell */
        if (borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 25
            && borg.trait[BI_GOLD] < 25000 && borg_count_sell() >= 13) {
            borg.stair_more = false;
        }

        /* Attempt to use those stairs */
        if (borg.stair_more && borg_flow_stair_more(GOAL_BORE, true, false)) {
            /* Leave a note */
            borg_note("# Powerdiving.");
            return true;
        }
    }

    /*** Exploration ***/

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_MISC))
        return true;

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_DARK))
        return true;

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_XTRA))
        return true;

    /* Continue flowing (see below) */
    if (borg_flow_old(GOAL_BORE))
        return true;

    if (borg_flow_old(GOAL_VAULT))
        return true;

    /*** Explore the dungeon ***/

    if (vault_on_level) {

        /* Chase close monsters */
        if (borg_flow_kill(false, z_info->max_range + 1))
            return true;

        /* Chase close objects */
        if (borg_flow_take(false, 35))
            return true;
        if (borg_flow_vein(false, 35))
            return true;

        /* Excavate a vault safely */
        if (borg_excavate_vault(z_info->max_range - 2))
            return true;

        /* Find a vault to excavate */
        if (borg_flow_vault(35))
            return true;

        /* Explore close interesting grids */
        if (borg_flow_dark(true))
            return true;
    }

    /* Chase old monsters */
    if (borg_flow_kill(false, 250))
        return true;

    /* Chase old objects */
    if (borg_flow_take(false, 250))
        return true;
    if (borg_flow_vein(false, 250))
        return true;

    /* Explore interesting grids */
    if (borg_flow_dark(true))
        return true;

    /* Leave the level (if needed) */
    if (borg.trait[BI_GOLD] < borg_cfg[BORG_MONEY_SCUM_AMOUNT]
        && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 && !borg.trait[BI_CDEPTH]
        && borg.trait[BI_LIGHT]) {
        /* Stay in town and scum for money after shopping */
    } else {
        if (borg_leave_level(false))
            return true;
    }

    /* Explore interesting grids */
    if (borg_flow_dark(false))
        return true;

    /*** Deal with shops ***/

    /* Hack -- Visit the shops */
    if (borg_choose_shop()) {
        /* Try and visit a shop, if so desired */
        if (borg_flow_shop_entry(borg.goal.shop))
            return true;
    }

    /*** Leave the Level ***/

    /* Study/Test boring spells/prayers */
    if (!borg.goal.fleeing && borg_play_magic(true))
        return true;

    /* Search for secret door via spell before spastic */
    if (!borg.when_detect_doors || (borg_t - borg.when_detect_doors >= 500)) {
        if (borg_check_light())
            return true;
    }

    /* Search for secret doors */
    if (borg_flow_spastic(false))
        return true;

    /* Flow directly to a monster if not able to be spastic */
    if (borg_flow_kill_direct(false, false))
        return true;

    /* Recharge items before leaving the level */
    if (borg_wear_recharge())
        return true;

    /* Leave the level (if possible) */
    if (borg.trait[BI_GOLD] < borg_cfg[BORG_MONEY_SCUM_AMOUNT]
        && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 && !borg.trait[BI_CDEPTH]
        && borg.trait[BI_LIGHT]
        && !borg_cfg[BORG_PLAYS_RISKY]) /* risky borgs are in a hurry */
    {
        /* Stay in town, scum for money now that shopping is done. */
        if (borg_money_scum())
            return true;
    } else {
        if (borg_leave_level(true))
            return true;
    }

    /* Search for secret door via spell before spastic */
    if (!borg.when_detect_doors || (borg_t - borg.when_detect_doors >= 500)) {
        if (borg_check_light())
            return true;
    }

    /* Search for secret doors */
    if (borg_flow_spastic(true))
        return true;

    /* Flow directly to a monster if not able to be spastic */
    if (borg_flow_kill_direct(true, false))
        return true;

    /*** Wait for recall ***/

    /* Wait for recall, unless in danger */
    if (borg.goal.recalling
        && (borg_danger(borg.c.y, borg.c.x, 1, true, false) <= 0)) {
        /* Take note */
        borg_note("# Waiting for Recall...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('9');
        borg_keypress(KC_ENTER);

        /* Done */
        return true;
    }

    /*** Nothing to do ***/

    /* Twitching in town can be fatal.  Really he should not become twitchy
     * but sometimes he cant recall to the dungeon and that may induce the
     * twitchy behavior.  So we reset the level if this happens.  That will
     * force him to go shopping all over again.
     */
    if ((borg.trait[BI_CDEPTH] == 0 && borg_t - borg_began > 800)
        || borg_t > 28000)
        old_depth = 126;

    /* Set a flag that the borg is  not allowed to retreat for 5 rounds */
    borg.no_retreat = 5;

    /* Boost slightly */
    if (avoidance < borg.trait[BI_CURHP] * 2) {
        bool done = false;

        /* Note */
        borg_note(format("# Boosting bravery (1) from %d to %d!", avoidance,
            borg.trait[BI_CURHP] * 2));

        /* Hack -- ignore some danger */
        avoidance = (borg.trait[BI_CURHP] * 2);

        /* Forget the danger fields */
        borg_danger_wipe = true;

        /* Try anything */
        if (borg_think_dungeon_brave())
            done = true;

        /* Reset "avoidance" */
        avoidance = borg.trait[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Forget goals */
        /*        goal = 0;*/

        /* Done */
        if (done)
            return true;
    }

    /* try phase before boosting bravery further and acting goofy */
    borg.times_twitch++;

    /* Phase to get out of being twitchy up to 3 times per level. */
    if (borg.times_twitch < 3) {
        borg_note("# Considering Phase (twitchy)");

        /* Phase */
        if (borg_allow_teleport()
            && (borg_spell(PHASE_DOOR) || borg_activate_item(act_tele_phase)
                || borg_read_scroll(sv_scroll_phase_door)
                || borg_dimension_door(90) || borg_spell(TELEPORT_SELF)
                || borg_spell(PORTAL) || borg_shadow_shift(90))) {
            /* Success */
            return true;
        }
    }

    /* Set a flag that the borg is not allowed */
    /*  to retreat for 10 rounds */
    borg.no_retreat = 10;

    /* Boost some more */
    if (avoidance < borg.trait[BI_MAXHP] * 4) {
        bool done = false;

        /* Note */
        borg_note(format("# Boosting bravery (2) from %d to %d!", avoidance,
            borg.trait[BI_MAXHP] * 4));

        /* Hack -- ignore some danger */
        avoidance = (borg.trait[BI_MAXHP] * 4);

        /* Forget the danger fields */
        borg_danger_wipe = true;

        /* Try anything */
        if (borg_think_dungeon_brave())
            done = true;

        /* Reset "avoidance" */
        avoidance = borg.trait[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Forget goals */
        /*        goal = 0;*/

        /* Done */
        if (done)
            return true;
    }

    /* Boost a lot */
    if (avoidance < 30000) {
        bool done = false;

        /* Note */
        borg_note(
            format("# Boosting bravery (3) from %d to %d!", avoidance, 30000));

        /* Hack -- ignore some danger */
        avoidance = 30000;

        /* Forget the danger fields */
        borg_danger_wipe = true;

        /* Reset multiple factors to jumpstart the borg */
        unique_on_level   = 0;
        scaryguy_on_level = false;

        /* reset our breeder flag */
        breeder_level = false;

        /* Forget goals */
        borg.goal.type = 0;

        /* Hack -- cannot rise past town */
        if (!borg.trait[BI_CDEPTH])
            borg.goal.rising = false;

        /* Assume not ignoring monsters */
        borg.goal.ignoring = false;

        /* No known stairs */
        track_less.num = 0;
        track_more.num = 0;

        /* No known glyph */
        track_glyph.num = 0;

        /* No known steps */
        track_step.num = 0;

        /* No known doors */
        track_door.num = 0;

        /* No known doors */
        track_closed.num = 0;

        /* No mineral veins */
        track_vein.num = 0;

        /* No objects here */
        borg_takes_cnt = 0;
        borg_takes_nxt = 1;

        /* Try anything */
        if (borg_think_dungeon_brave())
            done = true;

        /* Reset "avoidance" */
        avoidance = borg.trait[BI_CURHP];

        /* Re-calculate danger */
        borg_danger_wipe = true;

        /* Done */
        if (done)
            return true;
    }

    /* try teleporting before acting goofy */
    borg.times_twitch++;

    /* Teleport to get out of being twitchy up to 5 times per level. */
    if (borg.times_twitch < 5) {
        /* Teleport */
        if (borg_allow_teleport()
            && (borg_dimension_door(90) || borg_spell(TELEPORT_SELF)
                || borg_spell(PORTAL) || borg_shadow_shift(90)
                || borg_use_staff(sv_staff_teleportation)
                || borg_read_scroll(sv_scroll_teleport)
                || borg_read_scroll(sv_scroll_teleport_level)
                || borg_activate_item(act_tele_level))) {
            /* Success */
            borg_note("# Teleport (twitchy)");
            return true;
        }
    }

    /* Recall to town */
    if (borg.trait[BI_CDEPTH] && (borg_recall())) {
        /* Note */
        borg_note("# Recalling (twitchy)");

        /* Success */
        return true;
    }

    /* Reset multiple factors to jumpstart the borg */
    unique_on_level   = 0;
    scaryguy_on_level = false;

    /* reset our breeder flag */
    breeder_level = false;

    /* No objects here */
    borg_takes_cnt = 0;
    borg_takes_nxt = 1;

    /* No monsters here */
    borg_kills_cnt = 0;
    borg_kills_nxt = 1;

    /* if we twitch a lot, time to leave */
    if (borg.times_twitch > 20)
        borg.goal.fleeing = true;

    /* Attempt to dig to the center of the dungeon */
    if (borg_flow_kill_direct(true, true))
        return true;

    /* Twitch around */
    if (borg_twitchy())
        return true;

    /* Oops */
    return false;
}

#endif
