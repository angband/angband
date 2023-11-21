/**
 * \file borg-attack-munchkin.c
 * \brief attacks while in "munchkin mode" which is super cautious
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

#include "borg-attack-munchkin.h"

#ifdef ALLOW_BORG

#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-fight-attack.h"
#include "borg-flow-kill.h"
#include "borg-flow.h"
#include "borg-io.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg.h"

/* Munchkin Attack - Magic
 *
 * The early mages have a very difficult time surviving until they level up
 * some. This routine will allow the mage to do some very limited attacking
 * while he is doing the munchkin start (stair scumming for items).
 *
 * Basically, he will rest on stairs to recuperate mana, then use MM to attack
 * some easy to kill monsters.  If the monster gets too close, he will flee via
 * the stairs. He hope to be able to kill the monster in two shots from the MM.
 * A perfect scenario would be a mold which does not move, then he could
 * rest/shoot/rest.
 */
bool borg_munchkin_mage(void)
{
    int i, x, y;
    int a_y, a_x;

    int b_dam = -1, dam = 0;
    int b_n = -1;

    borg_grid *ag;

    /* Must be standing on a stair */
    if (borg_grids[c_y][c_x].feat != FEAT_MORE
        && borg_grids[c_y][c_x].feat != FEAT_LESS)
        return (false);

    /* Not if too dangerous */
    if ((borg_danger(c_y, c_x, 1, true, true) > avoidance * 7 / 10)
        || borg_trait[BI_CURHP] < borg_trait[BI_MAXHP] / 3)
        return (false);
    if (borg_trait[BI_ISCONFUSED])
        return (false);

    /* Nobody around */
    if (!borg_kills_cnt)
        return (false);

    /* Set the attacking flag so that danger is boosted for monsters */
    /* we want to attack first. */
    borg_attacking = true;

    /* Reset list */
    borg_temp_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Acquire location */
        a_x = kill->x;
        a_y = kill->y;

        /* Not in town.  This should not be reached, but just in case we add it
         */
        if (borg_trait[BI_CDEPTH] == 0)
            continue;

        /* Check if there is a monster adjacent to me or he's close and fast. */
        if ((kill->speed > borg_trait[BI_SPEED]
                && borg_distance(c_y, c_x, a_y, a_x) <= 2)
            || borg_distance(c_y, c_x, a_y, a_x) <= 1)
            return (false);

        /* no attacking most scaryguys, try to get off the level */
        if (scaryguy_on_level)
            return (false);

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY))
            continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW))
            continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) > z_info->max_range)
            continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n) {
        borg_attacking = false;
        return (false);
    }

    /* Simulate */
    borg_simulate = true;

    /* Simulated */
    for (i = 0; i < BF_MAX; i++) {
        /* Skip certain ones */
        if (i <= 1)
            continue;

        dam = borg_calculate_attack_effectiveness(i);

        /* Track the best attack method */
        if (dam >= b_dam && dam > 0) {
            b_dam = dam;
            b_n   = i;
        }
    }

    /* Nothing good */
    if (b_n < 0 || b_dam <= 0) {
        borg_attacking = false;
        return (false);
    }

    /* Note */
    borg_note(format("# Performing munchkin attack with value %d.", b_dam));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_calculate_attack_effectiveness(b_n);

    borg_attacking = false;

    /* Success */
    return (true);
}

/* Munchkin Attack - Melee
 *
 * The early borgs have a very difficult time surviving until they level up
 * some. This routine will allow the borg to do some very limited attacking
 * while he is doing the munchking start (stair scumming for items).
 *
 * Basically, he will rest on stairs to recuperate HP, then use melee to attack
 * some easy to kill adjacent monsters.
 */
bool borg_munchkin_melee(void)
{
    int i, x, y;

    int n = 0;

    borg_grid *ag;

    /* No Mages for now */
    if ((borg_class == CLASS_MAGE || borg_class == CLASS_NECROMANCER))
        return (false);

    /* Must be standing on a stair */
    if (borg_grids[c_y][c_x].feat != FEAT_MORE
        && borg_grids[c_y][c_x].feat != FEAT_LESS)
        return (false);

    /* Nobody around */
    if (!borg_kills_cnt)
        return (false);

    /* Not if too dangerous */
    if ((borg_danger(c_y, c_x, 1, true, true) > avoidance * 7 / 10)
        || borg_trait[BI_CURHP] < borg_trait[BI_MAXHP] / 3)
        return (false);
    if (borg_trait[BI_ISCONFUSED])
        return (false);

    /* Set the attacking flag so that danger is boosted for monsters */
    /* we want to attack first. */
    borg_attacking = true;

    /* Reset list */
    borg_temp_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Not in town.  This should not be reached, but just in case we add it
         */
        if (borg_trait[BI_CDEPTH] == 0)
            continue;

        /* no attacking most scaryguys, try to get off the level */
        if (scaryguy_on_level)
            return (false);

        /* Acquire location */
        x = kill->x;
        y = kill->y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY))
            continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW))
            continue;

        /* Check the distance XXX XXX XXX */
        if (borg_distance(c_y, c_x, y, x) != 1)
            continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = x;
        borg_temp_y[borg_temp_n] = y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n) {
        borg_attacking = false;
        return (false);
    }

    /* Simulate */
    borg_simulate = true;

    /* Simulated */
    n = borg_calculate_attack_effectiveness(BF_THRUST);

    /* Nothing good */
    if (n <= 0) {
        borg_attacking = false;
        return (false);
    }

    /* Note */
    borg_note(format("# Performing munchkin attack with value %d.", n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_calculate_attack_effectiveness(BF_THRUST);

    borg_attacking = false;

    /* Success */
    return (true);
}

#endif