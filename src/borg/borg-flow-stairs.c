/**
 * \file borg-flow-stairs.c
 * \brief Flow (move) toward stairs
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

#include "borg-flow-stairs.h"

#ifdef ALLOW_BORG

#include "borg-flow-kill.h"
#include "borg-flow.h"
#include "borg-io.h"
#include "borg-magic.h"
#include "borg-prepared.h"
#include "borg-store-sell.h"
#include "borg-trait.h"

/*
 * Track "stairs up"
 */
struct borg_track track_less;

/*
 * Track "stairs down"
 */
struct borg_track track_more;

/*
 * Do a Stair-Flow.  Look at how far away this grid is to my closest stair
 */
int borg_flow_cost_stair(int y, int x, int b_stair)
{
    int cost = 255;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Paranoid */
    if (b_stair == -1)
        return 0;

    /* Enqueue the player's grid */
    borg_flow_enqueue_grid(track_less.y[b_stair], track_less.x[b_stair]);

    /* Spread, but do NOT optimize */
    borg_flow_spread(250, false, false, false, b_stair, false);

    /* Distance from the grid to the stair */
    cost = borg_data_cost->data[y][x];

    return (cost);
}

/*
 * Prepare to flee the level via stairs
 */
bool borg_flow_stair_both(int why, bool sneak)
{
    int i;

    /* None to flow to */
    if (!track_less.num && !track_more.num)
        return false;

    /* don't go down if hungry or low on food, unless fleeing a scary town */
    if (!borg.goal.fleeing && !scaryguy_on_level && !track_less.num
        && (avoidance <= borg.trait[BI_CURHP] * 15 / 10)
        && (borg.trait[BI_ISWEAK] || borg.trait[BI_ISHUNGRY]
            || borg.trait[BI_FOOD] < 2))
        return false;

    /* Absolutely no diving if no light */
    if (borg.trait[BI_CURLITE] == 0 && borg.trait[BI_CDEPTH] != 0
        && borg.munchkin_mode == false)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_less.num; i++) {
        /* Not if a monster is parked on the stair */
        if (borg_grids[track_less.y[i]][track_less.x[i]].kill)
            continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_less.y[i], track_less.x[i]);
    }

    /* Enqueue useful grids */
    for (i = 0; i < track_more.num; i++) {
        /* Not if a monster is parked on the stair */
        if (borg_grids[track_more.y[i]][track_more.x[i]].kill)
            continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_more.y[i], track_more.x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, false, false, false, -1, sneak);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("stairs", why))
        return false;

    /* Take one step */
    if (!borg_flow_old(why))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to flow towards "up" stairs
 */
bool borg_flow_stair_less(int why, bool sneak)
{
    int i;

    /* None to flow to */
    if (!track_less.num)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_less.num; i++) {
        /* Not if a monster is parked on the stair */
        if (borg_grids[track_less.y[i]][track_less.x[i]].kill)
            continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_less.y[i], track_less.x[i]);
    }

    if (borg.trait[BI_CLEVEL] > 35 || borg.trait[BI_CURLITE] == 0) {
        /* Spread the flow */
        borg_flow_spread(250, true, false, false, -1, sneak);
    } else {
        /* Spread the flow, No Optimize, Avoid */
        borg_flow_spread(250, false, !borg_desperate, false, -1, sneak);
    }

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("up-stairs", why))
        return false;

    /* Take one step */
    if (!borg_flow_old(why))
        return false;

    /* Success */
    return true;
}

/*
 * Prepare to flow towards "down" stairs
 */
bool borg_flow_stair_more(int why, bool sneak, bool brave)
{
    int i;

    /* None to flow to */
    if (!track_more.num)
        return false;

    /* if there are no down stairs, don't filter use of up stairs */
    if (track_less.num) {
        /* not unless safe or munchkin/Lunal Mode or brave */
        if (!borg.lunal_mode && !borg.munchkin_mode && !brave
            && (char *)NULL != borg_prepared(borg.trait[BI_CDEPTH] + 1))
            return false;

        /* dont go down if hungry or low on food, unless fleeing a scary town */
        if (!brave && borg.trait[BI_CDEPTH] && !scaryguy_on_level
            && (borg.trait[BI_ISWEAK] || borg.trait[BI_ISHUNGRY]
                || borg.trait[BI_FOOD] < 2))
            return false;

        /* If I need to sell crap, then don't go down */
        if (borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 25
            && borg.trait[BI_GOLD] < 25000 && borg_count_sell() >= 13
            && !borg.munchkin_mode)
            return false;

        /* No diving if no light */
        if (borg.trait[BI_CURLITE] == 0 && borg.munchkin_mode == false)
            return false;
    }

    /* don't head for the stairs if you are recalling,  */
    /* even if you are fleeing. */
    if (borg.goal.recalling)
        return false;

    /* Clear the flow codes */
    borg_flow_clear();

    /* Enqueue useful grids */
    for (i = 0; i < track_more.num; i++) {
        /* Not if a monster is parked on the stair */
        if (borg_grids[track_more.y[i]][track_more.x[i]].kill)
            continue;

        /* Enqueue the grid */
        borg_flow_enqueue_grid(track_more.y[i], track_more.x[i]);
    }

    /* Spread the flow */
    borg_flow_spread(250, true, false, false, -1, sneak);

    /* Attempt to Commit the flow */
    if (!borg_flow_commit("down-stairs", why))
        return false;

    /* Take one step */
    if (!borg_flow_old(why))
        return false;

    /* Success */
    return true;
}

/*
 * Cast spells before leaving the level
 */
bool borg_prep_leave_level_spells(void)
{
    /* if we are running away, just run. */
    if (borg.goal.fleeing)
        return false;

    /* if we are low on mana, don't prep. */
    if (borg.trait[BI_CURSP] < ((borg.trait[BI_MAXSP] * 6) / 10))
        return false;

    /* Cast haste */
    if (!borg.temp.fast && borg_spell_fail(HASTE_SELF, 15)) {
        borg_note("# Casting speed spell before leaving level.");
        borg.no_rest_prep = 5000;
        return true;
    }

    /* Cast resistance */
    if (borg.temp.res_fire + borg.temp.res_acid + borg.temp.res_elec
                + borg.temp.res_cold
            < 3
        && borg_spell_fail(RESISTANCE, 15)) {
        borg_note("# Casting Resistance spell before leaving level.");
        borg.no_rest_prep = 21000;
        return true;
    }

    /* Cast fastcast */
    if (!borg.temp.fastcast && borg_spell_fail(MANA_CHANNEL, 15)) {
        borg_note("# Casting Mana Channel spell before leaving level.");
        borg.no_rest_prep = 6000;
        return true;
    }

    /* Cast Berserk Strength */
    if (!borg.temp.berserk && borg_spell_fail(BERSERK_STRENGTH, 15)) {
        borg_note("# Casting Berserk Strength spell before leaving level.");
        borg.no_rest_prep = 10000;
        return true;
    }

    /* Cast heroism if above level it gives temp attribute */
    if (!borg.temp.hero && borg_spell_fail(HEROISM, 15)
        && borg.trait[BI_CLEVEL] > borg_heroism_level()) {
        borg_note("# Casting Heroism spell before leaving level.");
        borg.no_rest_prep = 3000;
        return true;
    }

    /* Cast regen just before returning to dungeon */
    if (!borg.temp.regen && borg_spell_fail(RAPID_REGENERATION, 15)) {
        borg_note("# Casting Regen before leaving level.");
        borg.no_rest_prep = 6000;
        return true;
    }

    /* Cast Smite Evil just before returning to dungeon */
    if (!borg.temp.smite_evil && !borg.trait[BI_WS_EVIL]
        && borg_spell_fail(SMITE_EVIL, 15)) {
        borg_note("# Casting Smite Evil before leaving level.");
        borg.no_rest_prep = 21000;
        return true;
    }

    /* Cast Venom just before returning to dungeon */
    if (!borg.temp.venom && !borg.trait[BI_WB_POIS]
        && borg_spell_fail(VENOM, 15)) {
        borg_note("# Casting Venom before leaving level.");
        borg.no_rest_prep = 18000;
        return true;
    }

    /* Cast PFE just before returning to dungeon */
    if (!borg.temp.prot_from_evil
        && borg_spell_fail(PROTECTION_FROM_EVIL, 15)) {
        borg_note("# Casting PFE before leaving level.");
        borg.no_rest_prep = borg.trait[BI_CLEVEL] * 1000;
        return true;
    }

    /* Cast bless prep things */
    if ((!borg.temp.bless
            && (borg_spell_fail(BLESS, 15)
                || borg_spell_fail(DEMON_BANE, 15)))) {
        borg_note("# Casting blessing before leaving level.");
        borg.no_rest_prep = 11000;
        return true;
    }
    return false;
}

void borg_init_flow_stairs(void)
{
    /* Track "up" stairs */
    borg_init_track(&track_less, 16);

    /* Track "down" stairs */
    borg_init_track(&track_more, 16);
}

void borg_free_flow_stairs(void)
{
    borg_free_track(&track_more);
    borg_free_track(&track_less);
}

#endif
