/**
 * \file borg-think-dungeon-util.c
 * \brief Things to think about while in the dungeon that don't fit elsewhere
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

#include "borg-think-dungeon-util.h"

#ifdef ALLOW_BORG

#include "../game-world.h"
#include "../ui-event.h"
#include "../ui-term.h"

#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-escape.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow-take.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-item-wear.h"
#include "borg-light.h"
#include "borg-magic.h"
#include "borg-messages-react.h"
#include "borg-prepared.h"
#include "borg-projection.h"
#include "borg-store-sell.h"
#include "borg-think-dungeon.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/*
 * Hack -- importance of the various "level feelings"
 * Try to explore the level for at least this many turns
 */
static int borg_stuff_feeling[]
    = { 50000, /* 0 is no feeling yet given, stick around to get one */
          8000, 8000, 8000, 8000, 5000, 5000, 100, 100, 100, 100, 0 };

/*
 * money Scumming is a type of town scumming for money
 */
bool borg_money_scum(void)
{

    int dir     = -1;
    int divisor = 2;

    borg_grid *ag;

    /* Just a quick check to make sure we are supposed to do this */
    if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] == 0)
        return false;

    /* Take note */
    borg_note(format("# Waiting for towns people to breed.  I need %d...",
        borg_cfg[BORG_MONEY_SCUM_AMOUNT] - borg.trait[BI_GOLD]));

    /* I'm not in a store */
    borg.in_shop = false;

    /* Rest for 9 months */
    if (borg.trait[BI_CLEVEL] >= 35) {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('5');
        borg_keypress('0');
        borg_keypress('0');
        borg_keypress(KC_ENTER);
    } else if (borg.trait[BI_CLEVEL] >= 15) {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('7');
        borg_keypress('5');
        borg_keypress(KC_ENTER);
    } else /* Low level, dont want to get mobbed */
    {
        borg_keypress(ESCAPE);
        borg_keypress('R');
        borg_keypress('2');
        borg_keypress('5');
        borg_keypress(KC_ENTER);
    }

    /* Don't rest too long at night.  We tend to crash the game if too many
     * townsfolk are on the level
     */
    /* Night or day up in town */
    if ((turn % (10L * z_info->day_length)) < ((10L * z_info->day_length) / 2))
        divisor = 5;

    /* sometimes twitch in order to move around some */
    if (borg_t % divisor) {
        borg_keypress(ESCAPE);

        /* Pick a random direction */
        while (dir == -1 || dir == 5 || dir == 0) {
            dir = randint0(10);

            /* Hack -- set goal */
            borg.goal.g.x = borg.c.x + ddx[dir];
            borg.goal.g.y = borg.c.y + ddy[dir];

            ag            = &borg_grids[borg.goal.g.y][borg.goal.g.x];

            /* Skip walls and shops */
            if (ag->feat != FEAT_FLOOR)
                dir = -1;
        }

        /* Normally move */
        /* Send direction */
        borg_keypress(I2D(dir));
    }

    /* reset the clocks */
    borg_t               = 10;
    borg.time_this_panel = 1;
    borg_began           = 1;

    /* Done */
    return true;
}

/* Attempt a series of maneuvers to stay alive when you run out of light */
bool borg_think_dungeon_light(void)
{
    int        ii, x, y;
    bool       not_safe = false;
    borg_grid *ag;

    /* Consume needed things */
    if (borg.trait[BI_ISHUNGRY] && borg_use_things())
        return true;

    if (!borg.trait[BI_LIGHT]
        && (borg.trait[BI_CURLITE] <= 0 || borg_items[INVEN_LIGHT].timeout <= 3)
        && borg.trait[BI_CDEPTH] >= 1) {
        enum borg_need need;

        /* I am recalling, sit here till it engages. */
        if (borg.goal.recalling) {
            /* just wait */
            borg_keypress('R');
            borg_keypress('9');
            borg_keypress(KC_ENTER);
            return true;
        }

        /* wear stuff and see if it glows */
        if (borg_wear_stuff())
            return true;

        /* attempt to refuel/swap */
        need = borg_maintain_light();
        if (need == BORG_MET_NEED)
            return true;
        if (need == BORG_NO_NEED)
            return false;

        /* Can I recall out with a rod */
        if (!borg.goal.recalling && borg_zap_rod(sv_rod_recall))
            return true;

        /* Can I recall out with a spell */
        if (!borg.goal.recalling && borg_recall())
            return true;

        /* Log */
        borg_note("# Testing for stairs .");

        /* Test for stairs */
        borg_keypress('<');

        /* If on a glowing grid, got some food, and low mana, then rest here */
        if ((borg.trait[BI_CURSP] < borg.trait[BI_MAXSP]
                && borg.trait[BI_MAXSP] > 0)
            && (borg_grids[borg.c.y][borg.c.x].info & BORG_GLOW)
            && !borg.trait[BI_ISWEAK]
            && (borg_spell_legal(HERBAL_CURING)
                || borg_spell_legal(REMOVE_HUNGER)
                || borg.trait[BI_FOOD] >= borg.trait[BI_CDEPTH])
            && (borg_spell_legal(CALL_LIGHT) || borg_spell_legal(LIGHT_ROOM))
            && (!borg_spell_okay(CALL_LIGHT) && !borg_spell_okay(LIGHT_ROOM))) {
            /* Scan grids adjacent to me */
            for (ii = 0; ii < 8; ii++) {
                x = borg.c.x + ddx_ddd[ii];
                y = borg.c.y + ddy_ddd[ii];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(x, y)))
                    continue;

                /* Access the grid */
                ag = &borg_grids[y][x];

                /* Check for adjacent Monster */
                if (ag->kill) {
                    not_safe = true;
                }
            }

            /* Be concerned about the Regional Fear. */
            if (borg_fear_region[borg.c.y / 11][borg.c.x / 11]
                > borg.trait[BI_CURHP] / 10)
                not_safe = true;

            /* Be concerned about the Monster Fear. */
            if (borg_fear_monsters[borg.c.y][borg.c.x]
                > borg.trait[BI_CURHP] / 10)
                not_safe = true;

            /* rest here to gain some mana */
            if (!not_safe) {
                borg_note("# Resting on this Glowing Grid to gain mana.");
                borg_keypress('R');
                borg_keypress('*');
                borg_keypress(KC_ENTER);
                return true;
            }
        }

        /* If I have the capacity to Call Light, then do so if adjacent to a
         * dark grid. We can illuminate the entire dungeon, looking for stairs
         * but not if we just did so.
         */
        if (borg.when_call_light == 0 || (borg_t - borg.when_call_light) > 7) {
            /* Scan grids adjacent to me */
            for (ii = 0; ii < 8; ii++) {
                x = borg.c.x + ddx_ddd[ii];
                y = borg.c.y + ddy_ddd[ii];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(x, y)))
                    continue;

                /* Access the grid */
                ag = &borg_grids[y][x];

                /* skip the Wall grids */
                if (ag->feat >= FEAT_RUBBLE && ag->feat <= FEAT_PERM)
                    continue;

                /* Problem with casting Call Light on Open Doors */
                if ((ag->feat == FEAT_OPEN || ag->feat == FEAT_BROKEN)
                    && (y == borg.c.y && x == borg.c.x)) {
                    /* Cheat the grid info to see if the door is lit */
                    if (square_isglow(cave, borg.c))
                        ag->info |= BORG_GLOW;
                    continue;
                }

                /* Look for a dark one */
                if ((ag->info & BORG_DARK) || /* Known to be dark */
                    ag->feat == FEAT_NONE || /* Nothing known about feature */
                    !(ag->info & BORG_MARK) || /* Nothing known about info */
                    !(ag->info & BORG_GLOW)) /* not glowing */
                {
                    /* Attempt to Call Light */
                    if (borg_activate_item(act_illumination)
                        || borg_activate_item(act_light)
                        || borg_zap_rod(sv_rod_illumination)
                        || borg_use_staff(sv_staff_light)
                        || borg_read_scroll(sv_scroll_light)
                        || borg_spell(CALL_LIGHT) || borg_spell(LIGHT_ROOM)) {
                        borg_note("# Illuminating the region while dark.");
                        borg_react("SELF:lite", "SELF:lite");
                        borg.when_call_light = borg_t;

                        return true;
                    }

                    /* Attempt to use Light Beam requiring a direction. */
                    if (borg_light_beam(false))
                        return true;
                }
            }
        }

        /* Try to flow to upstairs if on level one */
        if (borg_flow_stair_less(GOAL_FLEE, false)) {
            /* Take the stairs */
            /* Log */
            borg_note("# Taking up Stairs stairs (low Light).");
            borg_keypress('<');
            return true;
        }

        /* Try to flow to a lite */
        if (borg.trait[BI_RECALL] && borg_flow_light(GOAL_FLEE)) {
            return true;
        }
    }
    /* Nothing to do */
    return false;
}

/*
 * This is an exploitation function.  The borg will stair scum
 * in the dungeon to grab items close to the stair.
 */
bool borg_think_stair_scum(bool from_town)
{
    int j, b_j = -1;
    int i;

    borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

    uint8_t feat  = square(cave, borg.c)->feat;

    enum borg_need need;

    /* examine equipment and swaps */
    borg_notice(true);

    /* No scumming mode if starving or in town */
    if (borg.trait[BI_CDEPTH] == 0 || borg.trait[BI_ISWEAK]) {
        borg_note("# Leaving Scumming Mode. (Town or Weak)");
        borg.lunal_mode = false;
        return false;
    }

    /* No scumming if inventory is full.  Require one empty slot */
    if (borg_items[PACK_SLOTS - 1].iqty)
        return false;

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

    /** First deal with staying alive **/

    /* Hack -- require light */
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note("# Scum. (need fuel)");

    /** Track down some interesting gear **/
    /* XXX Should we allow him great flexibility in retrieving loot? (not always
     * safe?)*/
    /* Continue flowing towards objects */
    if (borg_flow_old(GOAL_TAKE))
        return true;

    /* Find a (viewable) object */
    if (borg_flow_take_scum(true, 6))
        return true;

    /*leave level right away. */
    borg_note("# Fleeing level. Scumming Mode");
    borg.goal.fleeing = true;

    /* Scumming Mode - Going down */
    if (track_more.num
        && (ag->feat == FEAT_MORE || ag->feat == FEAT_LESS
            || borg.trait[BI_CDEPTH] < 30)) {
        int y, x;

        if (track_more.num >= 2)
            borg_note("# Scumming Mode: I know of a down stair.");

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
        if (b_j < 8 || ag->feat == FEAT_MORE || borg.trait[BI_CDEPTH] < 30) {
            /* Note */
            borg_note("# Scumming Mode.  Power Diving. ");

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to DownStair */
            if (borg_flow_stair_more(GOAL_FLEE, false, false))
                return true;

            /* if standing on a stair */
            if (ag->feat == FEAT_MORE) {
                /* Take the DownStair */
                borg_keypress('>');

                return true;
            }
        }
    }

    /* Scumming Mode - Going up */
    if (track_less.num && borg.trait[BI_CDEPTH] != 1
        && (ag->feat == FEAT_MORE || ag->feat == FEAT_LESS)) {
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
        if (b_j < 8 || tmp_ag->feat == FEAT_LESS) {

            /* Note */
            borg_note("# Scumming Mode.  Power Climb. ");

            /* Set to help borg move better */
            borg.goal.less = true;

            /* Continue leaving the level */
            if (borg_flow_old(GOAL_FLEE))
                return true;

            /* Flow to UpStair */
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                borg_note("# Looking for stairs. Scumming Mode.");

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
        borg_note("# Scumming Mode.  Any Stair. ");

        /* Try to find some stairs */
        if (borg_flow_stair_both(GOAL_FLEE, true))
            return true;
    }

    /* return to normal borg_think_dungeon */
    return false;
}

/*
 * how long should the borg explore?
 */
static int borg_time_to_stay_on_level(bool bored)
{
    /* at low level, don't stay too long, */
    /* but long enough to hope for a feeling */
    if (borg.trait[BI_MAXCLEVEL] < 20)
        return z_info->feeling_need * 100;

    /* at very low level, stay less time */
    if (borg.trait[BI_CLEVEL] < 10)
        return borg.trait[BI_CLEVEL] * 250;

    /* at slightly low level, try not to run out of food staying */
    if (borg.trait[BI_CLEVEL] < 15)
        return borg.trait[BI_REG] ? 2000 : 2500;

    if (bored)
        return borg_stuff_feeling[borg_feeling_stuff] / 10;

    return borg_stuff_feeling[borg_feeling_stuff];
}

/*
 * Leave the level if necessary (or bored)
 * Scumming defined in borg_prepared.
 */
bool borg_leave_level(bool bored)
{
    int  sellable_item_count, g = 0;
    bool try_not_to_descend = false;

    bool need_restock       = false;

    /* Hack -- waiting for "recall" other than depth 1 */
    if (borg.goal.recalling && borg.trait[BI_CDEPTH] != 1)
        return false;

    /* Not bored if I have seen Morgoth recently */
    if (borg.trait[BI_CDEPTH] == 100 && morgoth_on_level
        && (borg_t - borg_t_morgoth < 5000)) {
        borg.goal.leaving = false;
        borg.goal.rising  = false;
        bored             = false;
    }

    /* There is a great concern about recalling back to level 100.
     * Often the borg will fall down a trap door to level 100 when he is not
     * prepared to be there.  Some classes can use Teleport Level to get
     * back up to 99,  But Warriors cannot.  Realistically the borg needs
     * be be able to scum deep in the dungeon.  But he cannot risk being
     * on 100 and using the few *Healing* pots that he managed to collect.
     * It is better for warriors to walk all the way down to 98 and scum.
     * It seems like a long and nasty crawl, but it is the best way to
     * make sure the borg survives.  Along the way he will collect the
     * Healing, Life and *Healing* that he needs.
     *
     * The other classes (or at least those who can use the Teleport Level
     * spell) will not need to do this nasty crawl.  Risky Borgs will
     * not crawl either.
     */

    /* Town */
    if (!borg.trait[BI_CDEPTH]) {
        /* Cancel rising */
        borg.goal.rising = false;

        /* Wait until bored */
        if (!bored)
            return false;

        /* Case for those who cannot Teleport Level */
        if (borg.trait[BI_MAXDEPTH] == 100 && !borg_cfg[BORG_PLAYS_RISKY]) {
            if (borg.trait[BI_ATELEPORTLVL] == 0) {
                /* These pple must crawl down to 100, Sorry */
                borg.goal.fleeing = true;
                borg.goal.leaving = true;
                borg.stair_more   = true;

                /* Note */
                borg_note(
                    "# Borg must crawl to deep dungeon- no recall to 100.");

                /* Attempt to use those stairs */
                if (borg_flow_stair_more(GOAL_BORE, false, false))
                    return true;

                /* Oops */
                return false;
            }
        }

        /* Hack -- Recall into dungeon */
        if ((borg.trait[BI_MAXDEPTH] >= (borg_cfg[BORG_WORSHIPS_GOLD] ? 10 : 8))
            && (borg.trait[BI_RECALL] >= 3)
            && (((char *)NULL
                    == borg_prepared(borg.trait[BI_MAXDEPTH] * 6 / 10))
                || borg_cfg[BORG_PLAYS_RISKY])
            && borg_recall()) {
            /* Note */
            borg_note("# Recalling into dungeon.");

            /* Give it a shot */
            return true;
        } else {
            /* note why we didn't recall. */
            if (borg.trait[BI_MAXDEPTH]
                < (borg_cfg[BORG_WORSHIPS_GOLD] ? 10 : 8)) {
                borg_note("# Not deep enough to recall");
            } else {
                if (borg.trait[BI_RECALL] <= 2) {
                    borg_note("# Not enough recalls to recall");
                } else {
                    /* recall unless way out of our league */
                    if ((char *)NULL
                        != borg_prepared(borg.trait[BI_MAXDEPTH] * 6 / 10)) {
                        borg_note(
                            format("# Way too scary to recall down there!   %s",
                                borg_prepared(borg.trait[BI_MAXDEPTH])));
                    } else {
                        borg_note("# failed to recall when I wanted to");
                    }
                }
            }
            borg.goal.fleeing = true;
            borg.goal.leaving = true;
        }

        borg.stair_more = true;

        /* Attempt to use those stairs */
        if (borg_flow_stair_more(GOAL_BORE, false, false))
            return true;

        /* Oops */
        return false;
    }

    /** In the Dungeon **/
    const char *prep_cur_depth  = borg_prepared(borg.trait[BI_CDEPTH]);
    const char *prep_next_depth = borg_prepared(borg.trait[BI_CDEPTH] + 1);

    /* if not prepared for this level, head upward */
    if (NULL != prep_cur_depth) {
        g = -1;
        borg_note(format(
            "# heading up, not prep for current level: %s)", prep_cur_depth));
    }

    /* Count sellable items */
    sellable_item_count = borg_count_sell();

    /* Do not dive when "full" of items */
    if (sellable_item_count >= 12)
        try_not_to_descend = true;

    /* Do not dive when drained */
    if (g && borg.trait[BI_ISFIXEXP])
        try_not_to_descend = true;

    /* Rise a level if bored and unable to dive. */
    if (bored && (NULL != prep_next_depth)) {
        g = -1;
        borg_note(format(
            "# heading up (bored and unable to dive: %s)", prep_next_depth));
    }

    /* Rise a level if bored and spastic. */
    else if (bored && avoidance > borg.trait[BI_CURHP]) {
        if (NULL != prep_next_depth) {
            g = -1;
            borg_note("# heading up (bored and spastic).");
        } else {
            g = 1;
            borg_note("# heading down (bored and spastic).");
        }
    }

    /* Power dive if I am playing too shallow*/
    else if (!try_not_to_descend
             && NULL == borg_prepared(borg.trait[BI_CDEPTH] + 5)
             && sellable_item_count < 13) {
        g = 1;
        borg_note("# power dive, playing too shallow.");
    }

    /* Power dive if I am playing deep */
    else if (!try_not_to_descend && NULL == prep_next_depth
             && borg.trait[BI_CDEPTH] >= 75 && borg.trait[BI_CDEPTH] < 100) {
        g = 1;
        borg_note("# power dive, head deep.");
    }

    /* Hack -- Power-climb upwards when needed */
    if (NULL != prep_cur_depth) {
        /* Certain checks are bypassed if Unique monster on level */
        if (!unique_on_level) {
            /* if I am really out of depth go to town */
            if (!g && NULL != borg_prepared(borg.trait[BI_MAXDEPTH] * 5 / 10)
                && borg.trait[BI_MAXDEPTH] > 65) {
                borg_note(format(
                    "# Returning to town (too deep: %s)", prep_cur_depth));
                borg.goal.rising = true;
            } else {
                borg_note(format("# Climbing (too deep: %s)", prep_cur_depth));
                g = -1;
            }
        }

        /* if I must  go to town without delay */
        if (NULL != borg_restock(borg.trait[BI_CDEPTH])) {
            borg_note(format("# returning to town to restock(too deep: %s)",
                borg_restock(borg.trait[BI_CDEPTH])));
            borg.goal.rising = true;
            need_restock     = true;
        }

        /* I must return to collect stock from the house. */
        if (strstr(prep_cur_depth, "Collect from house")) {
            borg_note("# Returning to town to Collect stock.");
            borg.goal.rising = true;
            need_restock     = true;
        }
    }

    /* Hack -- if I am playing way too shallow return to town */
    if (NULL == borg_prepared(borg.trait[BI_CDEPTH] + 20)
        && NULL == borg_prepared(borg.trait[BI_MAXDEPTH] * 6 / 10)
        && borg.trait[BI_MAXDEPTH] > borg.trait[BI_CDEPTH] + 20
        && (borg.trait[BI_RECALL] >= 3 || borg.trait[BI_GOLD] > 2000)) {
        borg_note("# returning to town to recall back down (too shallow)");
        borg.goal.rising = true;
    }

    /* Return to town to sell stuff -- No recall allowed.*/
    if (((borg_cfg[BORG_WORSHIPS_GOLD] || borg.trait[BI_MAXCLEVEL] < 15)
            && borg.trait[BI_MAXCLEVEL] <= 25)
        && (sellable_item_count >= 12)) {
        borg_note("# Going to town (Sell Stuff, Worshipping Gold).");
        borg.goal.rising = true;
    }

    /* Return to town to sell stuff (use Recall) */
    if ((bored && borg.trait[BI_MAXCLEVEL] >= 26)
        && (sellable_item_count >= 12)) {
        borg_note("# Going to town (Sell Stuff).");
        borg.goal.rising = true;
    }

    /* Return to town when level drained */
    if (borg.trait[BI_ISFIXLEV]) {
        borg_note("# Going to town (Fix Level).");
        borg.goal.rising = true;
    }

    /* Return to town to restore experience */
    if (bored && borg.trait[BI_ISFIXEXP] && borg.trait[BI_CLEVEL] != 50) {
        borg_note("# Going to town (Fix Experience).");
        borg.goal.rising = true;
    }

    /* return to town if it has been a while */
    if ((!borg.goal.rising && bored && !vault_on_level && !borg_fighting_unique
            && borg_time_town + borg_t - borg_began > 8000)
        || (borg_time_town + borg_t - borg_began > 12000)) {
        borg_note("# Going to town (I miss my home).");
        borg.goal.rising = true;
    }

    /* return to town if been scumming for a bit */
    if (borg.trait[BI_MAXDEPTH] >= borg.trait[BI_CDEPTH] + 10
        && borg.trait[BI_CDEPTH] <= 12
        && borg_time_town + borg_t - borg_began > 3500) {
        borg_note("# Going to town (scumming check).");
        borg.goal.rising = true;
    }

    /* Return to town to drop off some scumming stuff */
    if (!vault_on_level
        && (borg.trait[BI_AEZHEAL] >= 3 || borg.trait[BI_ALIFE] >= 1)) {
        borg_note("# Going to town (Dropping off Potions).");
        borg.goal.rising = true;
    }

    /* Hack -- It is much safer to scum for items on 98
     * Check to see if depth 99, if Sauron is dead and Im not read to fight
     * the final battle
     */
    if (borg.trait[BI_CDEPTH] == 99 && borg.trait[BI_SAURON_DEAD]
        && borg.ready_morgoth != 1) {
        borg_note("# Returning to level 98 to scum for items.");
        g = -1;
    }

    /* Power dive if Morgoth is dead */
    if (borg.trait[BI_KING])
        g = 1;

    /* Power dive to 99 if ready */
    if (borg.trait[BI_CDEPTH] < 100 && NULL == borg_prepared(99))
        g = 1;

    /* Climb if deeper than I want to be */
    if (!g && borg.trait[BI_CDEPTH] > borg_cfg[BORG_NO_DEEPER]) {
        borg_note(format(
            "# Going up a bit (No Deeper than %d).", borg_cfg[BORG_NO_DEEPER]));
        g = -1;
    }

    /* if returning to town, try to go upstairs */
    if (!g && borg.goal.rising)
        g = -1;

    /* Mega-Hack -- spend time on the first level to rotate shops */
    if (borg.trait[BI_CLEVEL] > 10 && (borg.trait[BI_CDEPTH] == 1)
        && (borg_t - borg_began < 200) && (g < 0)
        && (borg.trait[BI_FOOD] > 1)) {
        borg_note("# Staying on level 1 to rotate shops.");
        g = 0;
    }

    /* do not hangout on boring levels for *too* long */
    if (!g && (borg_t - borg_began) > borg_time_to_stay_on_level(bored)) {
        /* Note */
        borg_note(format(
            "# Spent too long (%d) on level, leaving.", borg_t - borg_began));

        /* if we are trying not to go down, go up*/
        if (try_not_to_descend)
            g = -1;
        else
            /* otherwise use random stairs */
            g = ((randint0(100) < 50) ? -1 : 1);
    }

    /* Go Up */
    if (g < 0) {
        /* Take next stairs */
        borg_note("# Looking for up stairs.  Going up.");
        borg.stair_less = true;

        /* Hack -- recall if going to town */
        if (borg.goal.rising && ((borg_time_town + (borg_t - borg_began)) > 200)
            && (borg.trait[BI_CDEPTH] >= 5) && borg_recall()) {
            borg_note("# Recalling to town (goal rising)");
            return true;
        }

        /* Hack -- Recall if needing to Restock */
        if (need_restock && borg.trait[BI_CDEPTH] >= 5 && borg_recall()) {
            borg_note("# Recalling to town (need to restock)");
        }

        /* Attempt to use stairs */
        if (borg_flow_stair_less(GOAL_BORE, false)) {
            borg_note("# Looking for stairs. I'm bored.");
            return true;
        }

        /* Cannot find any stairs */
        if (borg.goal.rising && bored && (borg_t - borg_began) >= 1000) {
            if (borg_recall()) {
                borg_note("# Recalling to town (no stairs)");
                return true;
            }
        }

        /* No up stairs found. do down then back up */
        if (track_less.num == 0)
            g = 1;
    }

    /* Go Down */
    if (g > 0) {
        /* Take next stairs */
        borg.stair_more = true;

        /* Attempt to use those stairs */
        if (borg_flow_stair_more(GOAL_BORE, false, false))
            return true;
    }

    /* Failure */
    return false;
}

/*
 * Excavate an existing vault using ranged spells.
 * Stand where you are, use stone to mud to excavate the vault.  This will allow
 * the druid or blackgaurd borgs to get a few more attack spells on the monster.
 * Without this routine, he would approach the vault and use Stone to Mud when
 * he was adjacent to the wall, giving him only 1 or 2 shots before the monster
 * is next to the borg.
 *
 */
bool borg_excavate_vault(int range)
{
    int y, x, i, ii;
    int b_y, b_x;

    borg_grid *ag;

    /* reset counters */
    borg_temp_n = 0;
    i           = 0;
    ii          = 0;

    /* no need if no vault on level */
    if (!vault_on_level)
        return false;

    /* only if you can cast the spell */
    if (!borg_spell_okay_fail(TURN_STONE_TO_MUD, 30)
        && !borg_spell_okay_fail(SHATTER_STONE, 30))
        return false;

    /* Danger/bad idea checks */

    /* build the array -- Scan screen */
    for (y = w_y; y < w_y + SCREEN_HGT; y++) {
        for (x = w_x; x < w_x + SCREEN_WID; x++) {

            /* only bother with near ones */
            if (distance(borg.c, loc(x, y)) > range)
                continue;

            /* only deal with excavatable walls */
            if (borg_grids[y][x].feat != FEAT_GRANITE
                && borg_grids[y][x].feat != FEAT_RUBBLE
                && borg_grids[y][x].feat != FEAT_QUARTZ
                && borg_grids[y][x].feat != FEAT_MAGMA
                && borg_grids[y][x].feat != FEAT_QUARTZ_K
                && borg_grids[y][x].feat != FEAT_MAGMA_K)
                continue;

            /* Examine grids adjacent to this grid to see if there is a perma
             * wall adjacent */
            for (i = 0; i < 8; i++) {
                b_x = x + ddx_ddd[i];
                b_y = y + ddy_ddd[i];

                /* Bounds check */
                if (!square_in_bounds_fully(cave, loc(b_x, b_y)))
                    continue;

                ag = &borg_grids[b_y][b_x];

                /* Not a perma, and not our spot. */
                if (ag->feat != FEAT_PERM)
                    continue;

                /* Track the new grid */
                for (ii = 0; ii < borg_temp_n; ii++) {
                    if (borg_temp_y[ii] == y && borg_temp_x[ii] == x)
                        break;
                }

                /* Track the newly discovered excavatable wall */
                if ((ii == borg_temp_n) && (ii < AUTO_TEMP_MAX)) {
                    borg_temp_x[ii] = x;
                    borg_temp_y[ii] = y;
                    borg_temp_n++;
                }
            }
        }
    }

    /* None to excavate */
    if (!borg_temp_n)
        return false;

    /* Review the useful grids */
    for (i = 0; i < borg_temp_n; i++) {
        /* skip non-projectable grids grid (I cant shoot them) */
        if (!borg_los(borg.c.y, borg.c.x, borg_temp_y[i], borg_temp_x[i]))
            continue;

        /* Attempt to target the grid */
        borg_target(loc(borg_temp_x[i], borg_temp_y[i]));

        /* Attempt to excavate it with "stone to mud" */
        if (borg_spell(TURN_STONE_TO_MUD) || borg_spell(SHATTER_STONE)
            || borg_activate_ring(sv_ring_digging)
            || borg_activate_item(act_stone_to_mud)) {
            borg_note("# Excavation of vault");
            borg_keypress('5');

            /* turn that wall into a floor grid.  If the spell failed and the
             * grid is visible, it will still look like a wall and the
             * borg_update routine will redefine it as a wall
             */
            borg_do_update_view = true;
            borg_do_update_lite = true;

            /* Not Lit */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].info &= ~BORG_GLOW;
            /* Dark */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].info |= BORG_GLOW;
            /* Feat Floor */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].feat = FEAT_FLOOR;
            /*
             * If the grid is not seen, prefer what the borg remembers over
             * what map_info() returns (i.e. optimistically assume that the
             * excavation was successful.
             */
            borg_grids[borg_temp_y[i]][borg_temp_x[i]].info |= BORG_IGNORE_MAP;
            /* Forget number of mineral veins to force rebuild of vein list */
            track_vein.num = 0;

            return true;
        }

        /* Success */
        return true;
    }

    /* No grid to excavate */
    return false;
}

#endif
