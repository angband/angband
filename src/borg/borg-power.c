/**
 * \file borg-power.c
 * \brief The calculations to determine how powerful the borg thinks
 *          it is given the value arrays (borg.trait/has/activation)
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

#include "borg-power.h"

#ifdef ALLOW_BORG

#include "../cmd-core.h"
#include "../player-calcs.h"
#include "../player-spell.h"

#include "borg-flow-kill.h"
#include "borg-formulas.h"
#include "borg-home-notice.h"
#include "borg-item-activation.h"
#include "borg-item-analyze.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-prepared.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Helper function -- calculate "power" of equipment
 */
static int32_t borg_power_equipment(void)
{
    int hold;
    int damage, dam;

    int i;

    int cur_wgt   = 0;
    int max_wgt   = 0;

    int32_t value = 0L;

    borg_item *item;

    /* Obtain the "hold" value (weight limit for weapons) */
    hold = adj_str_hold[borg.trait[BI_STR_INDEX]];

    /*** Analyze weapon ***/

    /* Examine current weapon */
    item = &borg_items[INVEN_WIELD];

    /* We give a bonus to wearing an unID'D sword in order to use it and
     * garner a pseudoID from it.  We do not do this late in the game though
     * because our weapon often has traits that we need in order to be deep (FA,
     * SeeInvis)
     */
    if (borg.trait[BI_CDEPTH] < 10 && borg.trait[BI_MAXCLEVEL] < 15
        && item->iqty && item->ident != true)
        value += 1000000;

    /* Calculate "average" damage per "normal" blow  */
    /* and assume we can enchant up to +8 if borg.trait[BI_CLEVEL] > 25 */
    damage = (item->dd * item->ds * 20L);

    /* Reward "damage" and increased blows per round*/
    value += damage * (borg.trait[BI_BLOWS] + 1);

    /* Reward "bonus to hit" */
    value += ((borg.trait[BI_TOHIT] + item->to_h) * 100L);

    /* Reward "bonus to dam" */
    value += ((borg.trait[BI_TODAM] + item->to_d) * 30L);

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE]) {
        value += ((borg.trait[BI_TOHIT] + item->to_h) * 15L);
    }

    /* extra boost for deep dungeon */
    if (borg.trait[BI_MAXDEPTH] >= 75) {
        value += ((borg.trait[BI_TOHIT] + item->to_h) * 15L);

        value += item->dd * item->ds * 20L * 2 * borg.trait[BI_BLOWS];
    }

    /* assume 2x base damage for x% of creatures */
    dam = damage * 2 * borg.trait[BI_BLOWS];
    if (borg.trait[BI_WS_ANIMAL])
        value += (dam * 2) / 2;
    if (borg.trait[BI_WS_EVIL])
        value += (dam * 7) / 2;

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE]) {
        value += (dam);
    }

    /* assume 3x base damage for x% of creatures */
    dam = damage * 3 * borg.trait[BI_BLOWS];
    if (borg.trait[BI_WS_UNDEAD] && (!borg.trait[BI_WK_UNDEAD]))
        value += (dam * 5) / 2;
    if (borg.trait[BI_WS_DEMON] && (!borg.trait[BI_WK_DEMON]))
        value += (dam * 3) / 2;
    if (borg.trait[BI_WS_DRAGON] && (!borg.trait[BI_WK_DRAGON]))
        value += (dam * 6) / 2;
    if (borg.trait[BI_WS_GIANT])
        value += (dam * 4) / 2;
    if (borg.trait[BI_WB_ACID])
        value += (dam * 4) / 2;
    if (borg.trait[BI_WB_ELEC])
        value += (dam * 5) / 2;
    if (borg.trait[BI_WB_FIRE])
        value += (dam * 3) / 2;
    if (borg.trait[BI_WB_COLD])
        value += (dam * 3) / 2;
    /* SOrc and STroll get 1/2 of reward now */
    if (borg.trait[BI_WS_ORC])
        value += (dam * 1) / 2;
    if (borg.trait[BI_WS_TROLL])
        value += (dam * 2) / 2;
    /* and the other 2/2 if SEvil not possessed */
    if (borg.trait[BI_WS_ORC] && !borg.trait[BI_WS_EVIL])
        value += (dam * 1) / 2;
    if (borg.trait[BI_WS_TROLL] && !borg.trait[BI_WS_EVIL])
        value += (dam * 1) / 2;

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE]) {
        value += (dam);
    }

    /* assume 5x base damage for x% of creatures */
    dam = damage * 5 * borg.trait[BI_BLOWS];
    if (borg.trait[BI_WK_UNDEAD])
        value += (dam * 5) / 2;
    if (borg.trait[BI_WK_DEMON])
        value += (dam * 5) / 2;
    if (borg.trait[BI_WK_DRAGON])
        value += (dam * 5) / 2;
    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE]) {
        value += (dam);
    }

    /* It used to be only on Grond */
    if (borg.trait[BI_W_IMPACT])
        value += 50L;

    /* It is hard to hold a heavy weapon */
    if (borg.trait[BI_HEAVYWEPON])
        value -= 500000L;

    /* We want low level borgs to have high blows (dagger, whips) */
    if (borg.trait[BI_CLEVEL] <= 10)
        value += borg.trait[BI_BLOWS] * 45000L;

    /*** Analyze bow ***/

    /* Examine current bow */
    item = &borg_items[INVEN_BOW];

    /* We give a bonus to wearing an unID'D bow in order to use it and
     * garner a pseudoID from it.  We do not do this late in the game though
     * because our weapon often has traits that we need in order to be deep (FA,
     * SeeInvis)
     */
    if (borg.trait[BI_CDEPTH] < 10 && borg.trait[BI_MAXCLEVEL] < 15
        && item->iqty && item->ident != true)
        value += 6000000;

    /* Calculate "average" damage per "normal" shot (times 2) */
    if (item->to_d > 8 || borg.trait[BI_CLEVEL] < 25)
        damage = ((borg.trait[BI_AMMO_SIDES]) + (item->to_d))
                 * borg.trait[BI_AMMO_POWER];
    else
        damage = (borg.trait[BI_AMMO_SIDES] + 8) * borg.trait[BI_AMMO_POWER];

    /* Reward "damage" */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE]) {
        value += (borg.trait[BI_SHOTS] * damage * 11L);
    } else {
        value += (borg.trait[BI_SHOTS] * damage * 9L);
    }

    /* Extra bonus for low levels, they need a ranged weapon */
    if (borg.trait[BI_CLEVEL] < 15)
        value += (borg.trait[BI_SHOTS] * damage * 200L);

    /* slings force you to carry heavy ammo.  Penalty for that unless you have
     * lots of str  */
    if (item->sval == sv_sling && !item->art_idx
        && borg.trait[BI_STR] < 9) {
        value -= 5000L;
    }

    /* Bonus if level 1 to buy a sling, they are cheap ranged weapons */
    if (item->sval == sv_sling && borg.trait[BI_CLEVEL] == 1
        && borg.trait[BI_STR] >= 9)
        value += 8000;

    /* Reward "bonus to hit" */
    value += ((borg.trait[BI_TOHIT] + item->to_h) * 100L);

    /* extra damage for some */
    if (borg_cfg[BORG_WORSHIPS_DAMAGE]) {
        value += ((borg.trait[BI_TOHIT] + item->to_h) * 25L);
    }

    /* Prefer bows */
    if (borg.trait[BI_FAST_SHOTS]
        && borg.trait[BI_AMMO_TVAL] == TV_ARROW)
        value += 30000L;

    /* It is hard to hold a heavy weapon */
    if (hold < item->weight / 10)
        value -= 500000L;

    /*** Reward various things ***/

    /* Reward light radius */
    /* necromancers like the dark */
    if (borg.trait[BI_CLASS] == CLASS_NECROMANCER)
        value -= ((borg.trait[BI_LIGHT] - 1) * 10000L);
    else if (borg.trait[BI_LIGHT] <= 3)
        value += (borg.trait[BI_LIGHT] * 10000L);
    else if (borg.trait[BI_LIGHT] > 3)
        value += (30000L) + (borg.trait[BI_LIGHT] * 1000);

    value += borg.trait[BI_MOD_MOVES] * (3000L);
    value += borg.trait[BI_DAM_RED] * (10000L);

    /* Reward speed
     * see if speed can be a bonus if good speed; not +3.
     * reward higher for +10 than +50 speed (decreased return).
     */
    if (borg_cfg[BORG_WORSHIPS_SPEED]) {
        if (borg.trait[BI_SPEED] >= 150)
            value += (((borg.trait[BI_SPEED] - 120) * 1500L) + 185000L);

        if (borg.trait[BI_SPEED] >= 145 && borg.trait[BI_SPEED] <= 149)
            value += (((borg.trait[BI_SPEED] - 120) * 1500L) + 180000L);

        if (borg.trait[BI_SPEED] >= 140 && borg.trait[BI_SPEED] <= 144)
            value += (((borg.trait[BI_SPEED] - 120) * 1500L) + 175000L);

        if (borg.trait[BI_SPEED] >= 135 && borg.trait[BI_SPEED] <= 139)
            value += (((borg.trait[BI_SPEED] - 120) * 1500L) + 175000L);

        if (borg.trait[BI_SPEED] >= 130 && borg.trait[BI_SPEED] <= 134)
            value += (((borg.trait[BI_SPEED] - 120) * 1500L) + 160000L);

        if (borg.trait[BI_SPEED] >= 125 && borg.trait[BI_SPEED] <= 129)
            value += (((borg.trait[BI_SPEED] - 110) * 1500L) + 135000L);

        if (borg.trait[BI_SPEED] >= 120 && borg.trait[BI_SPEED] <= 124)
            value += (((borg.trait[BI_SPEED] - 110) * 1500L) + 110000L);

        if (borg.trait[BI_SPEED] >= 115 && borg.trait[BI_SPEED] <= 119)
            value += (((borg.trait[BI_SPEED] - 110) * 1500L) + 85000L);

        if (borg.trait[BI_SPEED] >= 110 && borg.trait[BI_SPEED] <= 114)
            value += (((borg.trait[BI_SPEED] - 110) * 1500L) + 65000L);

        if (borg.trait[BI_SPEED] < 110)
            value += (((borg.trait[BI_SPEED] - 110) * 2500L));
    } else {
        if (borg.trait[BI_SPEED] >= 140)
            value += (((borg.trait[BI_SPEED] - 120) * 1000L) + 175000L);

        if (borg.trait[BI_SPEED] >= 135 && borg.trait[BI_SPEED] <= 139)
            value += (((borg.trait[BI_SPEED] - 120) * 1000L) + 165000L);

        if (borg.trait[BI_SPEED] >= 130 && borg.trait[BI_SPEED] <= 134)
            value += (((borg.trait[BI_SPEED] - 120) * 1000L) + 150000L);

        if (borg.trait[BI_SPEED] >= 125 && borg.trait[BI_SPEED] <= 129)
            value += (((borg.trait[BI_SPEED] - 110) * 1000L) + 125000L);

        if (borg.trait[BI_SPEED] >= 120 && borg.trait[BI_SPEED] <= 124)
            value += (((borg.trait[BI_SPEED] - 110) * 1000L) + 100000L);

        if (borg.trait[BI_SPEED] >= 115 && borg.trait[BI_SPEED] <= 119)
            value += (((borg.trait[BI_SPEED] - 110) * 1000L) + 75000L);

        if (borg.trait[BI_SPEED] >= 110 && borg.trait[BI_SPEED] <= 114)
            value += (((borg.trait[BI_SPEED] - 110) * 1000L) + 55000L);

        if (borg.trait[BI_SPEED] < 110)
            value += (((borg.trait[BI_SPEED] - 110) * 2500L));
    }

    /* Reward strength bonus */
    value += (borg.trait[BI_STR_INDEX] * 100L);

    /* Reward spell stat bonus */
    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0) {
        value += (borg.trait[BI_STR_INDEX + spell_stat] * 500L);

        /* Bonus for sp. */
        if (borg_cfg[BORG_WORSHIPS_MANA]) {
            value += (borg.trait[BI_SP_ADJ] / 2) * 255L;
        } else {
            value += (borg.trait[BI_SP_ADJ] / 2) * 155L;
        }

        /* bonus for low fail rate */
        value += (100 - spell_chance(0)) * 100;

        /* should try to get min fail to 0 */
        if (player_has(player, PF_ZERO_FAIL)) {
            /* other fail rates */
            if (spell_chance(0) < 1)
                value += 30000L;
        }
    }

    /* Dexterity Bonus --good for attacking and ac*/
    if (borg.trait[BI_DEX_INDEX] <= 37) {
        /* Reward bonus */
        value += (borg.trait[BI_DEX_INDEX] * 120L);
    }

    /* Constitution Bonus */
    if (borg.trait[BI_CON_INDEX] <= 37) {

        if (borg_cfg[BORG_WORSHIPS_HP]) {
            value += (borg.trait[BI_CON_INDEX] * 250L);
            /* Reward hp bonus */
            /* This is a bit weird because we are not really giving */
            /* a bonus for what hp you have, but the 'bonus' */
            /* hp you get getting over 800hp is very important. */
            if (borg.trait[BI_HP_ADJ] < 800)
                value += borg.trait[BI_HP_ADJ] * 450L;
            else
                value += (borg.trait[BI_HP_ADJ] - 800) * 100L + (350L * 500);
        } else /*does not worship hp */
        {
            value += (borg.trait[BI_CON_INDEX] * 150L);
            /* Reward hp bonus */
            /* This is a bit weird because we are not really giving */
            /* a bonus for what hp you have, but the 'bonus' */
            /* hp you get getting over 500hp is very important. */
            if (borg.trait[BI_HP_ADJ] < 500)
                value += borg.trait[BI_HP_ADJ] * 350L;
            else
                value += (borg.trait[BI_HP_ADJ] - 500) * 100L + (350L * 500);
        }
    }

    /* HACK - a small bonus for adding to stats even above max. */
    /*        This will allow us to swap a ring of int +6 for */
    /*        our ring of int +2 even though we are at max int because */
    /*        we are wielding a weapon that has +4 int */
    /*        later it might be nice to swap to a weapon that does not */
    /*        have an int bonus */
    for (i = 0; i < STAT_MAX; i++)
        value += borg.trait[BI_ASTR+i];

    /* Tiny rewards */
    value += (borg.trait[BI_DISP] * 2L);
    value += (borg.trait[BI_DISM] * 2L);
    value += (borg.trait[BI_DEV] * 25L);
    value += (borg.trait[BI_SAV] * 25L);
    /* perfect saves are very nice */
    if (borg.trait[BI_SAV] > 99)
        value += 10000;
    value += (borg.trait[BI_STL] * 2L);
    value += (borg.trait[BI_SRCH] * 1L);
    value += (borg.trait[BI_THN] * 5L);
    value += (borg.trait[BI_THB] * 35L);
    value += (borg.trait[BI_THT] * 2L);
    value += (borg.trait[BI_DIG] * 2L);

    /*** Reward current flags ***/

    /* Various flags */
    if (borg.trait[BI_SDIG])
        value += 750L;
    if (borg.trait[BI_SDIG] && borg.trait[BI_ISHUNGRY])
        value += 7500L;
    if (borg.trait[BI_SDIG] && borg.trait[BI_ISWEAK])
        value += 7500L;

    /* Feather Fall if low level is nice */
    if (borg.trait[BI_MAXDEPTH] < 20) {
        if (borg.trait[BI_FEATH])
            value += 500L;
    } else {
        if (borg.trait[BI_FEATH])
            value += 50;
    }

    if (borg.trait[BI_LIGHT])
        value += 2000L;

    if (borg.trait[BI_ESP]) {
        if (borg.trait[BI_SINV])
            value += 500L;
    }

    if (!borg.trait[BI_DINV]) {
        if (borg.trait[BI_SINV])
            value += 5000L;
    }

    if (borg.trait[BI_FRACT])
        value += 10000L;

    /* after you max out you are pretty safe from drainers.*/
    if (borg.trait[BI_MAXCLEVEL] < 50) {
        if (borg.trait[BI_HLIFE])
            value += 2000L;
    } else {
        if (borg.trait[BI_HLIFE])
            value += 200L;
    }
    if (borg.trait[BI_REG])
        value += 2000L;
    if (borg.trait[BI_ESP])
        value += 80000L;

    /* Immunity flags */
    if (borg.trait[BI_ICOLD])
        value += 65000L;
    if (borg.trait[BI_IELEC])
        value += 40000L;
    if (borg.trait[BI_IFIRE])
        value += 80000L;
    if (borg.trait[BI_IACID])
        value += 50000L;

    /* Resistance flags */
    if (borg.trait[BI_RCOLD])
        value += 3000L;
    if (borg.trait[BI_RELEC])
        value += 4000L;
    if (borg.trait[BI_RACID])
        value += 6000L;
    if (borg.trait[BI_RFIRE])
        value += 8000L;
    /* extra bonus for getting all basic resist */
    if (borg.trait[BI_RFIRE] 
        && borg.trait[BI_RACID] 
        && borg.trait[BI_RELEC]
        && borg.trait[BI_RCOLD])
        value += 10000L;
    if (borg.trait[BI_RPOIS])
        value += 20000L;
    if (borg.trait[BI_RSND])
        value += 3500L;
    if (borg.trait[BI_RLITE])
        value += 800L;
    if (borg.trait[BI_RDARK])
        value += 800L;
    if (borg.trait[BI_RKAOS])
        value += 5000L;

    /* this is way boosted to avoid carrying stuff you don't need */
    if (borg.trait[BI_RCONF])
        value += 80000L;

    /* mages need a slight boost for this */
    if (borg.trait[BI_CLASS] == CLASS_MAGE && borg.trait[BI_RCONF])
        value += 2000L;
    if (borg.trait[BI_RDIS])
        value += 5000L;
    if (borg.trait[BI_RSHRD])
        value += 100L;
    if (borg.trait[BI_RNXUS])
        value += 100L;
    if (borg.trait[BI_RBLIND])
        value += 5000L;
    if (borg.trait[BI_RNTHR])
        value += 5500L;
    if (borg.trait[BI_RFEAR])
        value += 2000L;

    /* Sustain flags */
    if (borg.trait[BI_SSTR])
        value += 50L;
    if (borg.trait[BI_SINT])
        value += 50L;
    if (borg.trait[BI_SWIS])
        value += 50L;
    if (borg.trait[BI_SCON])
        value += 50L;
    if (borg.trait[BI_SDEX])
        value += 50L;
    /* boost for getting them all */
    if (borg.trait[BI_SSTR] 
        && borg.trait[BI_SINT] 
        && borg.trait[BI_SWIS]
        && borg.trait[BI_SDEX] 
        && borg.trait[BI_SCON])
        value += 1000L;

    /*** XXX XXX XXX Reward "necessary" flags ***/

    /* Mega-Hack -- See invisible (level 10) */
    if ((borg.trait[BI_SINV] || borg.trait[BI_ESP])
        && (borg.trait[BI_MAXDEPTH] + 1 >= 10))
        value += 100000L;

    /* Mega-Hack -- Free action (level 20) */
    if (borg.trait[BI_FRACT] && (borg.trait[BI_MAXDEPTH] + 1 >= 20))
        value += 100000L;

    /*  Mega-Hack -- resists (level 25) */
    if (borg.trait[BI_RFIRE] && (borg.trait[BI_MAXDEPTH] + 1 >= 25))
        value += 100000L;

    /*  Mega-Hack -- resists (level 40) */
    if (borg.trait[BI_RPOIS] && (borg.trait[BI_MAXDEPTH] + 1 >= 40))
        value += 100000L;
    if (borg.trait[BI_RELEC] && (borg.trait[BI_MAXDEPTH] + 1 >= 40))
        value += 100000L;
    if (borg.trait[BI_RACID] && (borg.trait[BI_MAXDEPTH] + 1 >= 40))
        value += 100000L;
    if (borg.trait[BI_RCOLD] && (borg.trait[BI_MAXDEPTH] + 1 >= 40))
        value += 100000L;

    /*  Mega-Hack -- Speed / Hold Life (level 46) and maxed out */
    if ((borg.trait[BI_HLIFE] && (borg.trait[BI_MAXDEPTH] + 1 >= 46)
            && (borg.trait[BI_MAXCLEVEL] < 50)))
        value += 100000L;
    if ((borg.trait[BI_SPEED] >= 115) && (borg.trait[BI_MAXDEPTH] + 1 >= 46))
        value += 100000L;
    if (borg.trait[BI_RCONF] && (borg.trait[BI_MAXDEPTH] + 1 >= 46))
        value += 100000L;

    /*  Mega-Hack -- resist Nether is -very- nice to have at level 50 */
    if (borg.trait[BI_RNTHR] && (borg.trait[BI_MAXDEPTH] + 1 >= 50))
        value += 55000L;

    /*  Mega-Hack -- resist Sound to avoid being KO'd */
    if (borg.trait[BI_RSND] && (borg.trait[BI_MAXDEPTH] + 1 >= 50))
        value += 100000L;

    /*  Mega-Hack -- resists & Telepathy (level 55) */
    if (borg.trait[BI_RBLIND] && (borg.trait[BI_MAXDEPTH] + 1 >= 55))
        value += 100000L;
    if (borg.trait[BI_ESP] && (borg.trait[BI_MAXDEPTH] + 1 >= 55))
        value += 100000L;
    if (borg.trait[BI_RNTHR] && (borg.trait[BI_MAXDEPTH] + 1 >= 60))
        value += 55000L;

    /*  Mega-Hack -- resists & +10 speed (level 60) */
    if (borg.trait[BI_RKAOS] && (borg.trait[BI_MAXDEPTH] + 1 >= 60))
        value += 104000L;
    if (borg.trait[BI_RDIS] && (borg.trait[BI_MAXDEPTH] + 1 >= 60))
        value += 90000L;
    if ((borg.trait[BI_SPEED] >= 120) && (borg.trait[BI_MAXDEPTH] + 1 >= 60))
        value += 100000L;

    /*  Must have +20 speed (level 80) */
    if ((borg.trait[BI_SPEED] >= 130) && (borg.trait[BI_MAXDEPTH] + 1 >= 80))
        value += 100000L;

    /* Not Req, but a good idea:
     * Extra boost to Nether deeper down
     * RDark for deeper uniques
     * Good to have +30 speed
     */
    if (borg.trait[BI_RNTHR] && (borg.trait[BI_MAXDEPTH] + 1 >= 80))
        value += 15000L;
    if (borg.trait[BI_RDARK] && (borg.trait[BI_MAXDEPTH] + 1 >= 80))
        value += 25000L;
    if ((borg.trait[BI_SPEED] >= 140) && (borg.trait[BI_MAXDEPTH] + 1 >= 80)
        && borg.trait[BI_CLASS] == CLASS_WARRIOR)
        value += 100000L;

    /*** Reward powerful armor ***/

    /* Reward armor */
    if (borg_cfg[BORG_WORSHIPS_AC]) {
        if (borg.trait[BI_ARMOR] < 15)
            value += ((borg.trait[BI_ARMOR]) * 2500L);
        if (borg.trait[BI_ARMOR] >= 15 && borg.trait[BI_ARMOR] < 75)
            value += ((borg.trait[BI_ARMOR]) * 2000L) + 28250L;
        if (borg.trait[BI_ARMOR] >= 75)
            value += ((borg.trait[BI_ARMOR]) * 1500L) + 73750L;
    } else {
        if (borg.trait[BI_ARMOR] < 15)
            value += ((borg.trait[BI_ARMOR]) * 2000L);
        if (borg.trait[BI_ARMOR] >= 15 && borg.trait[BI_ARMOR] < 75)
            value += ((borg.trait[BI_ARMOR]) * 500L) + 28350L;
        if (borg.trait[BI_ARMOR] >= 75)
            value += ((borg.trait[BI_ARMOR]) * 100L) + 73750L;
    }

    /*** Penalize various things ***/

    /* Penalize various flags */
    if (borg.trait[BI_CRSAGRV])
        value -= 800000L;
    if (borg.trait[BI_CRSHPIMP])
        value -= 35000;
    if (borg.trait[BI_CLASS] != CLASS_WARRIOR && borg.trait[BI_CRSMPIMP])
        value -= 15000;
    if ((borg.trait[BI_CLASS] == CLASS_MAGE
            || borg.trait[BI_CLASS] == CLASS_PRIEST
            || borg.trait[BI_CLASS] == CLASS_DRUID
            || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
        && borg.trait[BI_CRSMPIMP])
        value -= 15000;
    if (borg.trait[BI_CRSFEAR])
        value -= 400000L;
    if (borg.trait[BI_CRSFEAR] && borg.trait[BI_CLASS] != CLASS_MAGE)
        value -= 200000L;
    if (borg.trait[BI_CRSDRAIN_XP])
        value -= 400000L;
    if (borg.trait[BI_CRSFVULN])
        value -= 30000;
    if (borg.trait[BI_CRSEVULN])
        value -= 30000;
    if (borg.trait[BI_CRSCVULN])
        value -= 30000;
    if (borg.trait[BI_CRSAVULN])
        value -= 30000;

    if (borg.trait[BI_CRSTELE])
        value -= 100000L;
    if (borg.trait[BI_CRSENVELOPING])
        value -= 50000L;
    if (borg.trait[BI_CRSIRRITATION])
        value -= 20000L;
    if (borg.trait[BI_CRSPOIS])
        value -= 10000L;
    if (borg.trait[BI_CRSSIREN])
        value -= 800000L;
    if (borg.trait[BI_CRSHALU])
        value -= 100000L;
    if (borg.trait[BI_CRSPARA])
        value -= 800000L;
    if (borg.trait[BI_CRSSDEM])
        value -= 100000L;
    if (borg.trait[BI_CRSSDRA])
        value -= 100000L;
    if (borg.trait[BI_CRSSUND])
        value -= 100000L;
    if (borg.trait[BI_CRSSTONE] && borg.trait[BI_SPEED] < 140)
        value -= 10000L;
    if (borg.trait[BI_CRSSTEELSKIN] && borg.trait[BI_SPEED] < 140)
        value -= 10000L;
    if (borg.trait[BI_CRSNOTEL])
        value -= 700000L;
    if (borg.trait[BI_CRSTWEP])
        value -= 100000L;
    if (borg.trait[BI_CRSAIRSWING])
        value -= 10000L;
    if (borg.trait[BI_CRSUNKNO])
        value -= 9999999L;

#if 0 /* I wonder if this causes the borg to change his gear so radically at   \
         depth 99 */
    /* HUGE MEGA MONDO HACK! prepare for the big fight */
    /* go after Morgoth new priorities. */
    if ((borg.trait[BI_MAXDEPTH] + 1 == 100 || borg.trait[BI_CDEPTH] == 100) && (!borg.trait[BI_KING])) {
        /* protect from stat drain */
        if (borg.trait[BI_SSTR]) value_sec += 35000L;
        /* extra bonus for spell casters */
        if (player->class->spell_book == TV_MAGIC_BOOK && borg.trait[BI_SINT]) value_sec += 45000L;
        /* extra bonus for spell casters */
        if (player->class->spell_book == TV_PRAYER_BOOK && borg.trait[BI_SWIS]) value_sec += 35000L;
        if (borg.trait[BI_SCON]) value_sec += 55000L;
        if (borg.trait[BI_SDEX]) value_sec += 15000L;
        if (borg.trait[BI_WS_EVIL])  value_sec += 15000L;

        /* Another bonus for resist nether, poison and base four */
        if (borg.trait[BI_RNTHR]) value_sec += 15000L;
        if (borg.trait[BI_RDIS]) value_sec += 15000L;

        /* to protect against summoned baddies */
        if (borg.trait[BI_RPOIS]) value_sec += 100000L;
        if (borg.trait[BI_RFIRE] &&
            borg.trait[BI_RACID] &&
            borg.trait[BI_RELEC] &&
            borg.trait[BI_RCOLD]) value_sec += 100000L;
    }
#endif

    /* bonus for something with multiple useful bonuses */
    value += 3000L * borg.trait[BI_MULTIPLE_BONUSES];

    /* bonus for wearing something that needs an ID */
    value += 10000L * borg.trait[BI_WORN_NEED_ID];

    int activation_bonus ;
    int act;
    for (act = 1; act < z_info->act_max; act++) {
        if (!borg.activation[act])
            continue;
        activation_bonus = 0;
        /* an extra bonus for activations */
        if (act_illumination == act)
            activation_bonus += 500;
        else if (act_mapping == act)
            activation_bonus += 550;
        else if (act_clairvoyance == act)
            activation_bonus += 600;
        else if (act_fire_bolt == act)
            activation_bonus += (500 + (9 * (8 + 1) / 2));
        else if (act_cold_bolt == act)
            activation_bonus += (500 + (6 * (8 + 1) / 2));
        else if (act_elec_bolt == act)
            activation_bonus += (500 + (4 * (8 + 1) / 2));
        else if (act_acid_bolt == act)
            activation_bonus += (500 + (5 * (8 + 1) / 2));
        else if (act_mana_bolt == act)
            activation_bonus += (500 + (12 * (8 + 1) / 2));
        else if (act_stinking_cloud == act)
            activation_bonus += (500 + (24));
        else if (act_cold_ball50 == act)
            activation_bonus += (500 + (96));
        else if (act_cold_ball100 == act)
            activation_bonus += (500 + (200));
        else if (act_fire_ball72 == act)
            activation_bonus += (500 + (72));
        else if (act_cold_bolt2 == act)
            activation_bonus += (500 + (12 * (8 + 1) / 2));
        else if (act_fire_ball == act)
            activation_bonus += (500 + (144));
        else if (act_dispel_evil == act)
            activation_bonus += (500 + (10 + (borg.trait[BI_CLEVEL] * 5) / 2));
        else if (act_confuse2 == act)
            activation_bonus += 0; /* no code to handle this activation */
        else if (act_haste == act)
            activation_bonus += 0; /* handled by adding to speed available */
        else if (act_haste1 == act)
            activation_bonus += 0; /* handled by adding to speed available */
        else if (act_haste2 == act)
            activation_bonus += 0; /* handled by adding to speed available */
        else if (act_detect_objects == act)
            activation_bonus += 10;
        else if (act_probing == act)
            activation_bonus += 0; /* no code to handle this activation */
        else if (act_stone_to_mud == act)
            activation_bonus += 0; /* handled by adding to digger available */
        else if (act_tele_other == act) {
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                activation_bonus += 500;
            else
                activation_bonus += (500 + (500));
        } else if (act_drain_life1 == act)
            activation_bonus += (500 + 90);
        else if (act_drain_life2 == act)
            activation_bonus += (500 + 120);
        else if (act_berserker == act)
            activation_bonus += (500);
        else if (act_cure_light == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_cure_serious == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_cure_critical == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_cure_full2 == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_loskill == act)
            activation_bonus += (500 + 200);
        else if (act_recall == act)
            activation_bonus += 0; /* handled by adding to recall available */
        else if (act_arrow == act)
            activation_bonus += (500 + (150));
        else if (act_rem_fear_pois == act) {
            if (borg.trait[BI_CLASS] == CLASS_MAGE
                || borg.trait[BI_CLASS] == CLASS_PRIEST
                || borg.trait[BI_CLASS] == CLASS_DRUID)
                activation_bonus += 500;
            else
                activation_bonus += (500 + (200));
        } else if (act_tele_phase == act)
            activation_bonus += 500;
        else if (act_detect_all == act)
            activation_bonus += 0; /* handled by adding to detects available */
        else if (act_cure_full == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_heal1 == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_heal2 == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_heal3 == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_cure_nonorlybig == act)
            activation_bonus += 0; /* handled by adding to healing available */
        else if (act_protevil == act)
            activation_bonus += 0; /* handled by adding to PFE available */
        else if (act_destroy_doors == act)
            activation_bonus += 50;
        else if (act_banishment == act)
            activation_bonus += 1000;
        else if (act_resist_all == act) {
            activation_bonus += (500 + (150));
            /* extra bonus if you can't cast RESISTANCE */
            if (borg.trait[BI_CLASS] != CLASS_MAGE)
                activation_bonus += 25000;
        } else if (act_sleepii == act) {
            activation_bonus += 500;
            /* extra bonus if you can't cast a sleep type spell */
            if ((borg.trait[BI_CLASS] != CLASS_DRUID)
                && (borg.trait[BI_CLASS] != CLASS_NECROMANCER))
                activation_bonus += 200;
        } else if (act_recharge == act) {
            activation_bonus += 500;
            /* extra bonus if you can't cast a charge type spell */
            if ((borg.trait[BI_CLASS] != CLASS_MAGE)
                && (borg.trait[BI_CLASS] != CLASS_ROGUE))
                activation_bonus += 100;
        } else if (act_tele_long == act)
            activation_bonus += 300;
        else if (act_missile == act)
            activation_bonus += (500 + (2 * (6 + 1) / 2));
        else if (act_cure_temp == act)
            activation_bonus += 500;
        else if (act_starlight2 == act)
            activation_bonus += 100 + (10 * (8 + 1)) / 2;
        else if (act_bizarre == act)
            activation_bonus += (999999); /* HACK this is the one ring */
        else if (act_star_ball == act)
            activation_bonus += (500 + (300));
        else if (act_rage_bless_resist == act) {
            activation_bonus += (500 + (150));
            /* extra bonus if you can't cast RESISTANCE */
            if (borg.trait[BI_CLASS] != CLASS_MAGE)
                activation_bonus += 25000;
        } else if (act_polymorph == act) {
            activation_bonus
                += 0; /* no activation_bonus, borg doesn't use polymorph */
        } else if (act_starlight == act)
            activation_bonus += 100 + (6 * (8 + 1)) / 2;
        else if (act_light == act)
            activation_bonus += 0; /* handled by adding to ALITE */
        else if (act_firebrand == act)
            activation_bonus += 500;
        else if (act_restore_life == act)
            activation_bonus += 0; /* handled by adding to the rll available */
        else if (act_restore_exp == act)
            activation_bonus += 0; /* handled by adding to the rll available */
        else if (act_restore_st_lev == act)
            activation_bonus += 0; /* handled by adding to the rll available */
        else if (act_enlightenment == act)
            activation_bonus += 500;
        else if (act_hero == act)
            activation_bonus += 500;
        else if (act_shero == act)
            activation_bonus += 500;
        else if (act_cure_paranoia == act)
            activation_bonus += 100;
        else if (act_cure_mind == act)
            activation_bonus += 1000;
        else if (act_cure_body == act)
            activation_bonus += 1000;
        else if (act_mon_slow == act)
            activation_bonus += 500;
        else if (act_mon_confuse == act)
            activation_bonus += 500;
        else if (act_sleep_all == act)
            activation_bonus += 500;
        else if (act_mon_scare == act)
            activation_bonus += 500;
        else if (act_light_line == act)
            activation_bonus += 50;
        else if (act_disable_traps == act)
            activation_bonus += 50;
        else if (act_drain_life3 == act)
            activation_bonus += (500 + 150);
        else if (act_drain_life4 == act)
            activation_bonus += (500 + 250);
        else if (act_elec_ball == act)
            activation_bonus += 500 + 64;
        else if (act_elec_ball2 == act)
            activation_bonus += 500 + 250;
        else if (act_acid_bolt2 == act)
            activation_bonus += (500 + (10 * (8 + 1) / 2));
        else if (act_acid_bolt3 == act)
            activation_bonus += (500 + (12 * (8 + 1) / 2));
        else if (act_acid_ball == act)
            activation_bonus += 500 + 120;
        else if (act_cold_ball160 == act)
            activation_bonus += 500 + 160;
        else if (act_cold_ball2 == act)
            activation_bonus += 500 + 200;
        else if (act_fire_ball2 == act)
            activation_bonus += 500 + 120;
        else if (act_fire_ball200 == act)
            activation_bonus += 500 + 200;
        else if (act_fire_bolt2 == act)
            activation_bonus += (500 + (12 * (8 + 1) / 2));
        else if (act_fire_bolt3 == act)
            activation_bonus += (500 + (16 * (8 + 1) / 2));
        else if (act_dispel_evil60 == act)
            activation_bonus += 500 + 60;
        else if (act_dispel_undead == act)
            activation_bonus += 500 + 60;
        else if (act_dispel_all == act)
            activation_bonus += 500 + 60 * 2;
        else if (act_deep_descent == act)
            activation_bonus += 0; // !FIX no code to handle
        else if (act_earthquakes == act)
            activation_bonus += 500;
        else if (act_destruction2 == act)
            activation_bonus += 500;
        else if (act_losslow == act)
            activation_bonus += 50;
        else if (act_lossleep == act)
            activation_bonus += 100;
        else if (act_losconf == act)
            activation_bonus += 100;
        else if (act_satisfy == act)
            activation_bonus += 50;
        else if (act_blessing == act)
            activation_bonus += 50;
        else if (act_blessing2 == act)
            activation_bonus += 50;
        else if (act_blessing3 == act)
            activation_bonus += 50;
        else if (act_glyph == act)
            activation_bonus += 0; /* handled by adding to skill */
        else if (act_tele_level == act)
            activation_bonus += 5000L;
        else if (act_confusing == act)
            activation_bonus += 0; /* scroll only ever read to get rid of it */
        else if (act_enchant_tohit == act)
            activation_bonus
                += 0; /* handled by adding to "amount of bonus available" */
        else if (act_enchant_todam == act)
            activation_bonus
                += 0; /* handled by adding to "amount of bonus available" */
        else if (act_enchant_weapon == act)
            activation_bonus
                += 0; /* handled by adding to "amount of bonus available" */
        else if (act_enchant_armor == act)
            activation_bonus
                += 0; /* handled by adding to "amount of bonus available" */
        else if (act_enchant_armor2 == act)
            activation_bonus
                += 0; /* handled by adding to "amount of bonus available" */
        else if (act_remove_curse == act)
            activation_bonus += 9000;
        else if (act_remove_curse2 == act)
            activation_bonus += 10000;
        else if (act_detect_treasure == act)
            activation_bonus += 0; /* borg never uses this */
        else if (act_detect_invis == act)
            activation_bonus += 50;
        else if (act_detect_evil == act)
            activation_bonus += 50;
        else if (act_restore_mana == act)
            activation_bonus += 5000;
        /* the potion equivalent of increase stat with dec */
        /*  are only consumed to get rid of them */
        else if (act_brawn == act)
            activation_bonus += 0;
        else if (act_intellect == act)
            activation_bonus += 0;
        else if (act_contemplation == act)
            activation_bonus += 0;
        else if (act_toughness == act)
            activation_bonus += 0;
        else if (act_nimbleness == act)
            activation_bonus += 0;
        else if (act_restore_str == act)
            activation_bonus += 50;
        else if (act_restore_int == act)
            activation_bonus += 50;
        else if (act_restore_wis == act)
            activation_bonus += 50;
        else if (act_restore_dex == act)
            activation_bonus += 50;
        else if (act_restore_con == act)
            activation_bonus += 50;
        else if (act_restore_all == act)
            activation_bonus += 150;
        else if (act_tmd_free_act == act)
            activation_bonus += 0; /* !FIX no code to handle */
        else if (act_tmd_infra == act)
            activation_bonus += 0; /* potion only consumed to get rid of it */
        else if (act_tmd_sinvis == act)
            activation_bonus += 50;
        else if (act_tmd_esp == act)
            activation_bonus += 50;
        else if (act_resist_acid == act)
            activation_bonus += 100;
        else if (act_resist_elec == act)
            activation_bonus += 100;
        else if (act_resist_fire == act)
            activation_bonus += 100;
        else if (act_resist_cold == act)
            activation_bonus += 100;
        else if (act_resist_pois == act)
            activation_bonus += 150;
        else if (act_cure_confusion == act)
            activation_bonus += 50;
        else if (act_wonder == act)
            activation_bonus += 300;
        else if (act_wand_breath == act)
            activation_bonus += 0; /* !FIX no code to handle (currently no code
                                      for wands of drag breath) */
        else if (act_staff_magi == act)
            activation_bonus += 0; /* handled by adding to BI_ASTFMAGI */
        else if (act_staff_holy == act)
            activation_bonus += 1000;
        else if (act_drink_breath == act)
            activation_bonus
                += 0; /* !FIX no code to handle (nor for the potion) */
        else if (act_food_waybread == act)
            activation_bonus += 50;
        else if (act_shroom_emergency == act)
            activation_bonus += 0; /* mushroom only consumed to get rid of it */
        else if (act_shroom_terror == act)
            activation_bonus += 0;
        else if (act_shroom_stone == act)
            activation_bonus += 50;
        else if (act_shroom_debility == act)
            activation_bonus += 0;
        else if (act_shroom_sprinting == act)
            activation_bonus += 0; /* mushroom only consumed to get rid of it */
        else if (act_shroom_purging == act)
            activation_bonus += 50;
        else if (act_ring_acid == act)
            activation_bonus += 10000;
        else if (act_ring_flames == act)
            activation_bonus += 25000;
        else if (act_ring_ice == act)
            activation_bonus += 15000;
        else if (act_ring_lightning == act)
            activation_bonus += 10000;
        else if (act_dragon_blue == act)
            activation_bonus += 1100;
        else if (act_dragon_green == act)
            activation_bonus += 2750;
        else if (act_dragon_red == act)
            activation_bonus += 1100;
        else if (act_dragon_multihued == act)
            activation_bonus += 3250;
        else if (act_dragon_gold == act)
            activation_bonus += 5150;
        else if (act_dragon_chaos == act)
            activation_bonus += 5150;
        else if (act_dragon_law == act)
            activation_bonus += 5150;
        else if (act_dragon_balance == act)
            activation_bonus += 5150;
        else if (act_dragon_shining == act)
            activation_bonus += 5150;
        else if (act_dragon_power == act)
            activation_bonus += 5150;

        /* bonus is per item that has this activation */
        value += activation_bonus * borg.activation[act];
    }

    /* Hack-- Reward the borg for carrying a NON-ID items that have random
     * powers
     */
    if (borg_items[INVEN_OUTER].iqty) {
        item = &borg_items[INVEN_OUTER];
        if (((borg_ego_has_random_power(&e_info[item->ego_idx])
            && !item->ident)))
            value += 999999L;
    }

    /*** Penalize armor weight ***/
    if (borg.trait[BI_STR_INDEX] < 15) {
        if (borg_item_weight(&borg_items[INVEN_BODY]) > 200)
            value -= (borg_item_weight(&borg_items[INVEN_BODY]) - 200) * 15;
        if (borg_item_weight(&borg_items[INVEN_HEAD]) > 30)
            value -= 250;
        if (borg_item_weight(&borg_items[INVEN_ARM]) > 10)
            value -= 250;
        if (borg_item_weight(&borg_items[INVEN_FEET]) > 50)
            value -= 250;
    }

    /* Compute the total armor weight */
    cur_wgt += borg_item_weight(&borg_items[INVEN_BODY]);
    cur_wgt += borg_item_weight(&borg_items[INVEN_HEAD]);
    cur_wgt += borg_item_weight(&borg_items[INVEN_ARM]);
    cur_wgt += borg_item_weight(&borg_items[INVEN_OUTER]);
    cur_wgt += borg_item_weight(&borg_items[INVEN_HANDS]);
    cur_wgt += borg_item_weight(&borg_items[INVEN_FEET]);

    /* Determine the weight allowance */
    max_wgt = player->class->magic.spell_weight;

    /* Heavy armor hurts magic */
    if (player->class->magic.total_spells && ((cur_wgt - max_wgt) / 10) > 0) {
        /* max sp must be calculated in case it changed with the armor */
        int max_sp = borg.trait[BI_SP_ADJ] / 100 + 1;
        max_sp -= ((cur_wgt - max_wgt) / 10);
        /* Mega-Hack -- Penalize heavy armor which hurts mana */
        if (max_sp >= 300 && max_sp <= 350)
            value -= (((cur_wgt - max_wgt) / 10) * 400L);
        if (max_sp >= 200 && max_sp <= 299)
            value -= (((cur_wgt - max_wgt) / 10) * 800L);
        if (max_sp >= 100 && max_sp <= 199)
            value -= (((cur_wgt - max_wgt) / 10) * 1600L);
        if (max_sp <= 99)
            value -= (((cur_wgt - max_wgt) / 10) * 3200L);
    }

    /* Result */
    return (value);
}

/*
 * Helper function -- calculate power of inventory
 */
static int32_t borg_power_inventory(void)
{
    int k, book;

    int32_t value = 0L;

    /*** Basic abilities ***/

    /* Reward fuel */
    k = 0;
    for (; k < 6 && k < borg.trait[BI_AFUEL]; k++)
        value += 60000L;
    if (borg.trait[BI_STR] >= 15) {
        for (; k < 10 && k < borg.trait[BI_AFUEL]; k++)
            value += 6000L - (k * 100);
    }

    /* Reward Food */
    /* if hungry, food is THE top priority */
    if ((borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        && borg.trait[BI_FOOD])
        value += 100000;
    k = 0;
    for (; k < 7 && k < borg.trait[BI_FOOD]; k++)
        value += 50000L;
    if (borg.trait[BI_STR] >= 15) {
        for (; k < 10 && k < borg.trait[BI_FOOD]; k++)
            value += 200L;
    }
    if (borg.trait[BI_REG] && borg.trait[BI_CLEVEL] <= 15) {
        k = 0;
        for (; k < 15 && k < borg.trait[BI_FOOD]; k++)
            value += 700L;
    } 

    /* Prefer to buy HiCalorie foods over LowCalorie */
    k = 0;
    for (; k < 7 && k < borg.trait[BI_FOOD_HI]; k++)
        value += 52L;
    k = 0;
    for (; k < 15 && k < borg.trait[BI_FOOD_LO]; k++)
        value -= 2L;

    /* Reward Cure Poison and Cuts*/
    if ((borg.trait[BI_ISCUT] || borg.trait[BI_ISPOISONED])
        && borg.trait[BI_ACCW])
        value += 100000;
    if ((borg.trait[BI_ISCUT] || borg.trait[BI_ISPOISONED])
        && borg.trait[BI_AHEAL])
        value += 50000;
    if ((borg.trait[BI_ISCUT] || borg.trait[BI_ISPOISONED])
        && borg.trait[BI_ACSW]) { /* usually takes more than one */
        k = 0;
        for (; k < 5 && k < borg.trait[BI_ACSW]; k++)
            value += 25000L;
    }
    if (borg.trait[BI_ISPOISONED] && borg.trait[BI_ACUREPOIS])
        value += 15000;

    /* collect Resistance pots if not immune -- All Classes */
    if (!borg.trait[BI_IPOIS] && borg.trait[BI_ACUREPOIS] <= 20) {
        /* Not if munchkin starting */
        if (borg_cfg[BORG_MUNCHKIN_START]
            && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
            /* Do not carry these until later */
        } else {
            for (; k < 4 && k < borg.trait[BI_ARESPOIS]; k++)
                value += 300L;
        }
    }

    /* Reward Resistance Potions for Warriors */
    if (borg.trait[BI_CLASS] == CLASS_WARRIOR && borg.trait[BI_MAXDEPTH] > 20) {
        k = 0;
        /* Not if munchkin starting */
        if (borg_cfg[BORG_MUNCHKIN_START]
            && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
            /* Do not carry these until later */
        } else {
            /* collect pots if not immune */
            if (!borg.trait[BI_IFIRE]) {
                for (; k < 4 && k < borg.trait[BI_ARESHEAT]; k++)
                    value += 500L;
            }
            k = 0;
            /* collect pots if not immune */
            if (!borg.trait[BI_ICOLD]) {
                for (; k < 4 && k < borg.trait[BI_ARESCOLD]; k++)
                    value += 500L;
            }
            /* collect pots if not immune */
            if (!borg.trait[BI_IPOIS]) {
                for (; k < 4 && k < borg.trait[BI_ARESPOIS]; k++)
                    value += 500L;
            }
        }
    }

    /* Reward ident */
    k = 0;
    if (borg.trait[BI_CLEVEL] >= 10) {
        for (; k < 5 && k < borg.trait[BI_AID]; k++)
            value += 6000L;
        if (borg.trait[BI_STR] >= 15) {
            for (; k < 15 && k < borg.trait[BI_AID]; k++)
                value += 600L;
        }
    }
    /* Reward ID if I am carrying a {magical} or {excellent} item */
    if (borg.trait[BI_ALL_NEED_ID]) {
        k = 0;
        for (; k < borg.trait[BI_ALL_NEED_ID] && k < borg.trait[BI_AID]; k++)
            value += 6000L;
    }

    /*  Reward PFE carry lots of these*/
    k = 0;
    /* Not if munchkin starting */
    if (borg_cfg[BORG_MUNCHKIN_START]
        && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
        /* Do not carry these until later */
    } else {
        for (; k < 10 && k < borg.trait[BI_APFE]; k++)
            value += 10000L;
        for (; k < 25 && k < borg.trait[BI_APFE]; k++)
            value += 2000L;
    }
    /*  Reward Glyph- Rune of Protection-  carry lots of these*/
    k = 0;
    for (; k < 10 && k < borg.trait[BI_AGLYPH]; k++)
        value += 10000L;
    for (; k < 25 && k < borg.trait[BI_AGLYPH]; k++)
        value += 2000L;
    if (borg.trait[BI_MAXDEPTH] >= 100) {
        k = 0;
        for (; k < 10 && k < borg.trait[BI_AGLYPH]; k++)
            value += 2500L;
        for (; k < 75 && k < borg.trait[BI_AGLYPH]; k++)
            value += 2500L;
    }

    /* Reward Scroll of Mass Genocide, only when fighting Morgoth */
    if (borg.trait[BI_MAXDEPTH] >= 100) {
        k = 0;
        for (; k < 99 && k < borg.trait[BI_AMASSBAN]; k++)
            value += 2500L;
    }

    /* Reward recall */
    k = 0;
    if (borg.trait[BI_CLEVEL] > 7) {
        /* Not if munchkin starting */
        if (borg_cfg[BORG_MUNCHKIN_START]
            && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
            /* Do not carry these until later */
        } else {
            for (; k < 3 && k < borg.trait[BI_RECALL]; k++)
                value += 50000L;
            if (borg.trait[BI_STR] >= 15) {
                for (; k < 7 && k < borg.trait[BI_RECALL]; k++)
                    value += 5000L;
            }
            /* Deep borgs need the rod to avoid low mana traps */
            if (borg.trait[BI_MAXDEPTH] >= 50 && borg.has[kv_rod_recall])
                value += 12000;
        }
    }

    /* Reward phase */
    k = 1;
    /* first phase door is very important */
    if (borg.trait[BI_APHASE])
        value += 50000;
    /* Not if munchkin starting */
    if (borg_cfg[BORG_MUNCHKIN_START]
        && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
        /* Do not carry these until later */
    } else {
        for (; k < 8 && k < borg.trait[BI_APHASE]; k++)
            value += 500L;
        if (borg.trait[BI_STR] >= 15) {
            for (; k < 15 && k < borg.trait[BI_APHASE]; k++)
                value += 500L;
        }
    }

    /* Reward escape (staff of teleport or artifact) */
    k = 0;
    if (borg_cfg[BORG_MUNCHKIN_START]
        && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
        /* Do not carry these until later */
    } else {
        for (; k < 2 && k < borg.trait[BI_AESCAPE]; k++)
            value += 10000L;
        if (borg.trait[BI_MAXDEPTH] > 70) {
            k = 0;
            for (; k < 3 && k < borg.trait[BI_AESCAPE]; k++)
                value += 10000L;
        }
    }

    /* Reward teleport scroll*/
    k = 0;
    if (borg.trait[BI_CLEVEL] >= 3) {
        if (borg.trait[BI_ATELEPORT])
            value += 10000L;
    }
    if (borg.trait[BI_CLEVEL] >= 7) {
        for (; k < 3 && k < borg.trait[BI_ATELEPORT]; k++)
            value += 10000L;
    }
    if (borg.trait[BI_CLEVEL] >= 30) {
        for (; k < 10 && k < borg.trait[BI_ATELEPORT]; k++)
            value += 10000L;
    }

    /* Reward Teleport Level scrolls */
    k = 0;
    if (borg.trait[BI_CLEVEL] >= 15) {
        for (; k < 5 && k < borg.trait[BI_ATELEPORTLVL]; k++)
            value += 5000L;
    }

    /*** Healing ***/
    /* !TODO !FIX make sure these numbers make sense for the new classes */
    if (borg.trait[BI_CLASS] == CLASS_WARRIOR
        || borg.trait[BI_CLASS] == CLASS_ROGUE
        || borg.trait[BI_CLASS] == CLASS_BLACKGUARD) {
        k = 0;
        for (; k < 15 && k < borg.trait[BI_AHEAL]; k++)
            value += 8000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
        if (borg.trait[BI_MAXDEPTH] >= 46) {
            if (borg.trait[BI_PREP_BIG_FIGHT]) {
                for (; k < 1 && k < borg.trait[BI_AEZHEAL]; k++)
                    value += 10000L;
            } else {
                for (; k < 2 && k < borg.trait[BI_AEZHEAL]; k++)
                    value += 10000L;
            }
        }

        /* These guys need to carry the rods more, town runs low on supply. */
        k = 0;
        for (; k < 6 && k < borg.has[kv_rod_healing]; k++)
            value += 20000L;
    } else if (borg.trait[BI_CLASS] == CLASS_RANGER
               || borg.trait[BI_CLASS] == CLASS_PALADIN
               || borg.trait[BI_CLASS] == CLASS_NECROMANCER
               || borg.trait[BI_CLASS] == CLASS_MAGE) {
        k = 0;
        for (; k < 10 && k < borg.trait[BI_AHEAL]; k++)
            value += 4000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
        if (borg.trait[BI_MAXDEPTH] >= 46) {
            if (borg.trait[BI_PREP_BIG_FIGHT]) {
                for (; k < 1 && k < borg.trait[BI_AEZHEAL]; k++)
                    value += 10000L;
            } else {
               for (; k < 2 && k < borg.trait[BI_AEZHEAL]; k++)
                    value += 10000L;
            }
        }

        if (borg.trait[BI_CLASS] == CLASS_PALADIN) {
            /* Reward heal potions */
            k = 0;
            for (; k < 3 && k < borg.has[kv_potion_healing]; k++)
                value += 5000L;
        }

        /* These guys need to carry the rods more, town runs low on supply. */
        k = 0;
        for (; k < 4 && k < borg.has[kv_rod_healing]; k++)
            value += 20000L;

    } else if (borg.trait[BI_CLASS] == CLASS_PRIEST
               || borg.trait[BI_CLASS] == CLASS_DRUID) {
        /* Level 1 priests are given a Potion of Healing.  It is better
         * for them to sell that potion and buy equipment or several
         * Cure Crits with it.
         */
        if (borg.trait[BI_CLEVEL] == 1) {
            k = 0;
            for (; k < 10 && k < borg.has[kv_potion_healing]; k++)
                value -= 2000L;
        }
        /* Reward heal potions */
        k = 0;
        for (; k < 5 && k < borg.has[kv_potion_healing]; k++)
            value += 2000L;

        k = 0; /* carry a couple for emergency. Store the rest. */
        if (borg.trait[BI_MAXDEPTH] >= 46) {
            if (borg.trait[BI_PREP_BIG_FIGHT]) {
                for (; k < 1 && k < borg.trait[BI_AEZHEAL]; k++)
                    value += 10000L;
            } else {
                for (; k < 2 && k < borg.trait[BI_AEZHEAL]; k++)
                    value += 10000L;
            }
        }
    }

    /* Collecting Potions, prepping for Morgoth/Sauron fight */
    /* only carry them if we aren't prepping, otherwise they store at */
    /* home */
    if (borg.trait[BI_MAXDEPTH] >= 99 && !borg.trait[BI_PREP_BIG_FIGHT]) {
        k = 0;
        for (; k < 99 && k < borg.has[kv_potion_healing]; k++)
            value += 8000L;
        k = 0;
        for (; k < 99 && k < borg.trait[BI_AEZHEAL]; k++)
            value += 10000L;
        k = 0;
        for (; k < 99 && k < borg.trait[BI_ASPEED]; k++)
            value += 8000L;
        k = 0;
        for (; k < 99 && k < borg.trait[BI_ALIFE]; k++)
            value += 10000L;
        k = 0;
        if (borg.trait[BI_CLASS] != CLASS_WARRIOR) {
            for (; k < 99 && k < borg.has[kv_potion_restore_mana]; k++)
                value += 5000L;
        }
        k = 0;
        for (; k < 40 && k < borg.has[kv_mush_stoneskin]; k++)
            value += 5000L;

        /* Sauron is dead -- store them unless I have enough */
        if (borg.trait[BI_SAURON_DEAD]) {
            /* Reward Scroll of Mass Genocide, only when fighting Morgoth */
            k = 0;
            for (; k < 99 && k < borg.trait[BI_AMASSBAN]; k++)
                value += 2500L;
        }
    }

    /* Restore Mana */
    if (borg.trait[BI_MAXSP] > 100) {
        k = 0;
        for (; k < 10 && k < borg.has[kv_potion_restore_mana]; k++)
            value += 4000L;
        k = 0;
        for (; k < 100 && k < borg.trait[BI_ASTFMAGI]; k++)
            value += 4000L;
    }

    /* Reward Cure Critical.  Heavy reward on first 5 */
    if (borg.trait[BI_CLEVEL] < 35 && borg.trait[BI_CLEVEL] > 10) {
        k = 0;
        for (; k < 10 && k < borg.trait[BI_ACCW]; k++)
            value += 5000L;
    } else if (borg.trait[BI_CLEVEL] >= 35) {
        /* Reward cure critical.  Later on in game. */
        k = 0;
        for (; k < 10 && k < borg.trait[BI_ACCW]; k++)
            value += 5000L;
        if (borg.trait[BI_STR] > 15) {
            for (; k < 15 && k < borg.trait[BI_ACCW]; k++)
                value += 500L;
        }
    }

    /* Reward cure serious -- only reward serious if low on crits */
    if (borg.trait[BI_ACCW] < 5 && borg.trait[BI_MAXCLEVEL] > 10
        && (borg.trait[BI_CLEVEL] < 35 || !borg.trait[BI_RCONF])) {
        k = 0;
        for (; k < 7 && k < borg.trait[BI_ACSW]; k++)
            value += 50L;
        if (borg.trait[BI_STR] > 15) {
            for (; k < 10 && k < borg.trait[BI_ACSW]; k++)
                value += 5L;
        }
    }

    /* Reward cure light -- Low Level Characters */
    if ((borg.trait[BI_ACCW] + borg.trait[BI_ACSW] < 5)
        && borg.trait[BI_CLEVEL] < 8) {
        k = 0;
        for (; k < 5 && k < borg.trait[BI_ACLW]; k++)
            value += 550L;
    }

    /* Reward Cures */
    if (!borg.trait[BI_RCONF]) {
        if (borg_cfg[BORG_MUNCHKIN_START] && borg.trait[BI_MAXCLEVEL] < 10) {
            /* Do not carry these until later */
        } else {
            k = 0;
            for (; k < 10 && k < borg.trait[BI_FOOD_CURE_CONF]; k++)
                value += 400L;
        }
    }
    if (!borg.trait[BI_RBLIND]) {
        k = 0;
        if (borg_cfg[BORG_MUNCHKIN_START]
            && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
            /* Do not carry these until later */
        } else {
            for (; k < 5 && k < borg.trait[BI_FOOD_CURE_BLIND]; k++)
                value += 300L;
        }
    }
    if (!borg.trait[BI_RPOIS]) {
        k = 0;
        if (borg_cfg[BORG_MUNCHKIN_START]
            && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
            /* Do not carry these until later */
        } else {
            for (; k < 5 && k < borg.trait[BI_ACUREPOIS]; k++)
                value += 250L;
        }
    }
    /*** Detection ***/

    /* Reward detect trap */
    k = 0;
    for (; k < 1 && k < borg.trait[BI_ADETTRAP]; k++)
        value += 4000L;

    /* Reward detect door */
    k = 0;
    for (; k < 1 && k < borg.trait[BI_ADETDOOR]; k++)
        value += 2000L;

    /* Reward detect evil */
    if (!borg.trait[BI_ESP]) {
        k = 0;
        for (; k < 1 && k < borg.trait[BI_ADETEVIL]; k++)
            value += 1000L;
    }

    /* Reward magic mapping */
    k = 0;
    for (; k < 1 && k < borg.trait[BI_AMAGICMAP]; k++)
        value += 4000L;

    /* Reward call lite */
    if (borg.trait[BI_CLASS] != CLASS_NECROMANCER) {
        k = 0;
        for (; k < 1 && k < borg.trait[BI_ALITE]; k++)
            value += 1000L;
    }

    /* Genocide scrolls. Just scrolls, mainly used for Morgoth */
    if (borg.trait[BI_MAXDEPTH] >= 100) {
        k = 0;
        for (; k < 10 && k < borg.has[kv_scroll_mass_banishment]; k++)
            value += 10000L;
        for (; k < 25 && k < borg.has[kv_scroll_mass_banishment]; k++)
            value += 2000L;
    }

    /* Reward speed potions/rods/staves (but no staves deeper than depth 95) */
    k = 0;
    if (borg_cfg[BORG_MUNCHKIN_START]
        && borg.trait[BI_MAXCLEVEL] < borg_cfg[BORG_MUNCHKIN_LEVEL]) {
        /* Do not carry these until later */
    } else {
        for (; k < 20 && k < borg.trait[BI_ASPEED]; k++)
            value += 5000L;
    }

    /* Reward Recharge ability */
    if (borg.trait[BI_ARECHARGE] && borg.trait[BI_MAXDEPTH] < 99)
        value += 5000L;

    /*** Missiles ***/

    /* Reward missiles */
    if (borg.trait[BI_CLASS] == CLASS_RANGER
        || borg.trait[BI_CLASS] == CLASS_WARRIOR) {
        k = 0;
        for (; k < 40 && k < borg.trait[BI_AMISSILES]; k++)
            value += 100L;
        if (borg.trait[BI_STR] > 15 && borg.trait[BI_STR] <= 18) {
            for (; k < 80 && k < borg.trait[BI_AMISSILES]; k++)
                value += 10L;
        }
        if (borg.trait[BI_STR] > 18) {
            for (; k < 180 && k < borg.trait[BI_AMISSILES]; k++)
                value += 8L;
        }

        /* penalize use of too many quiver slots */
        for (k = 4; k < borg.trait[BI_QUIVER_SLOTS]; k++)
            value -= 10000L;

    } else {
        k = 0;
        for (; k < 20 && k < borg.trait[BI_AMISSILES]; k++)
            value += 100L;
        if (borg.trait[BI_STR] > 15) {
            for (; k < 50 && k < borg.trait[BI_AMISSILES]; k++)
                value += 10L;
        }
        /* Don't carry too many */
        if (borg.trait[BI_STR] <= 15 && borg.trait[BI_AMISSILES] > 20)
            value -= 1000L;
        /* penalize use of too many quiver slots */
        for (k = 2; k < borg.trait[BI_QUIVER_SLOTS]; k++)
            value -= 10000L;
    }

    /* cursed arrows are "bad" */
    value -= 1000L * borg.trait[BI_AMISSILES_CURSED];

    /* ego arrows are worth a bonus */
    value += 100L * borg.trait[BI_AMISSILES_SPECIAL];

    /*** Various ***/

    /*  -- Reward carrying a staff of destruction. */
    if (borg.trait[BI_ASTFDEST])
        value += 5000L;
    k = 0;
    for (; k < 9 && k < borg.trait[BI_ASTFDEST]; k++)
        value += 200L;

    /*  -- Reward carrying a wand of Teleport Other. */
    if (borg.trait[BI_ATPORTOTHER])
        value += 5000L;
    /* Much greater reward for Warrior */
    if (borg.trait[BI_CLASS] == CLASS_WARRIOR && borg.trait[BI_ATPORTOTHER])
        value += 50000L;
    /* reward per charge */
    k = 0;
    for (; k < 15 && k < borg.trait[BI_ATPORTOTHER]; k++)
        value += 5000L;

    /*  -- Reward carrying an attack wand.
     */
    if ((borg.has[kv_wand_magic_missile] || borg.has[kv_wand_stinking_cloud])
        && borg.trait[BI_MAXDEPTH] < 30)
        value += 5000L;
    if (borg.has[kv_wand_annihilation] && borg.trait[BI_CDEPTH] < 30)
        value += 5000L;
    /* Much greater reward for Warrior or lower level  */
    if ((borg.trait[BI_CLASS] == CLASS_WARRIOR || borg.trait[BI_CLEVEL] <= 20)
        && (borg.has[kv_wand_magic_missile] || borg.has[kv_wand_annihilation]
            || borg.has[kv_wand_stinking_cloud]))
        value += 10000L;
    /* reward per charge */
    value += borg.trait[BI_GOOD_W_CHG] * 50L;

    /* These staves are great but do not clutter inven with them */
    /*  -- Reward carrying a staff of holiness/power */
    if (borg.trait[BI_GOOD_S_CHG])
        value += 2500L;
    k = 0;
    for (; k < 3 && k < borg.trait[BI_GOOD_S_CHG]; k++)
        value += 500L;

    /* Rods of attacking are good too */
    k = 0;
    for (; k < 6 && k < borg.trait[BI_AROD1]; k++)
        value += 8000;
    k = 0;
    for (; k < 6 && k < borg.trait[BI_AROD2]; k++)
        value += 12000;

    /* Reward being at max stat */
    if (!borg.need_statgain[STAT_STR])
        value += 50000;
    if (!borg.need_statgain[STAT_INT])
        value += 20000;
    if (!borg.need_statgain[STAT_WIS])
        value += 20000;
    if (!borg.need_statgain[STAT_DEX])
        value += 50000;
    if (!borg.need_statgain[STAT_CON])
        value += 50000;

    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0)
        if (!borg.need_statgain[spell_stat])
            value += 50000;

    /* Reward stat potions */
    if (borg.amt_statgain[STAT_STR] && borg.trait[BI_CSTR] < (18 + 100))
        value += 550000;
    if (borg.amt_statgain[STAT_INT] && borg.trait[BI_CINT] < (18 + 100))
        value += 520000;
    if (spell_stat >= 0)
        if (borg.amt_statgain[spell_stat]
            && borg.trait[BI_CSTR + spell_stat] < (18 + 100))
            value += 575000;
    if (borg.amt_statgain[STAT_WIS] && borg.trait[BI_CWIS] < (18 + 100))
        value += 520000;
    if (borg.amt_statgain[STAT_DEX] && borg.trait[BI_CDEX] < (18 + 100))
        value += 550000;
    if (borg.amt_statgain[STAT_CON] && borg.trait[BI_CCON] < (18 + 100))
        value += 550000;

    /* Reward Remove Curse */
    if (borg.trait[BI_FIRST_CURSED]) {
        if (borg.has[kv_scroll_star_remove_curse])
            value += 90000;
        if (borg.has[kv_scroll_remove_curse])
            value += 90000;
    }

    /* Restore experience */
    if (borg.trait[BI_HASFIXEXP])
        value += 50000;

    /*** Enchantment ***/

    /* Reward enchant armor */
    if (borg.trait[BI_AENCH_ARM] < 1000 && borg.trait[BI_NEED_ENCHANT_TO_A])
        value += 540L;

    /* Reward enchant weapon to hit */
    if (borg.trait[BI_AENCH_TOH] < 1000 && borg.trait[BI_NEED_ENCHANT_TO_H])
        value += 540L;

    /* Reward enchant weapon to damage */
    if (borg.trait[BI_AENCH_TOD] < 1000 && borg.trait[BI_NEED_ENCHANT_TO_D])
        value += 500L;

    /* Reward *enchant weapon* to damage */
    if (borg.trait[BI_AENCH_SWEP])
        value += 5000L;

    /* Reward *enchant armour*  */
    if (borg.trait[BI_AENCH_SARM])
        value += 5000L;

    /* Reward empty slots (up to 6) */
    for (k = 1; k < 6 && k < borg.trait[BI_EMPTY]; k++)
        value += 40L;

    /* first empty slot is highly rewarded */
    if (borg.trait[BI_EMPTY])
        value += 4000L;

    /* Reward carrying a shovel if low level */
    if (borg.trait[BI_MAXDEPTH] <= 40 && borg.trait[BI_MAXDEPTH] >= 25
        && borg.trait[BI_GOLD] < 100000
        && borg_items[INVEN_WIELD].tval != TV_DIGGING
        && borg.trait[BI_ADIGGER] == 1)
        value += 5000L;

    /*** Books ***/
    /*   Reward books    */
    for (book = 0; book < 9; book++) {
        /* No copies */
        if (!borg.amt_book[book])
            continue;

        /* The "hard" books */
        if (player->class->magic.books[book].dungeon) {
            int what;

            /* Scan the spells */
            for (what = 0; what < 9; what++) {
                borg_magic *as = borg_get_spell_entry(book, what);
                if (!as)
                    break;

                /* Track minimum level */
                if (as->level > borg.trait[BI_MAXCLEVEL])
                    continue;

                /* Track Mana req. */
                if (as->power > borg.trait[BI_MAXSP])
                    continue;

                /* Reward the book based on the spells I can cast */
                value += 15000L;
            }
        }

        /* The "easy" books */
        else {
            int what, when = 99;

            /* Scan the spells */
            for (what = 0; what < 9; what++) {
                borg_magic *as = borg_get_spell_entry(book, what);
                if (!as)
                    break;

                /* Track minimum level */
                if (as->level < when)
                    when = as->level;

                /* Track Mana req. */
                /* if (as->power < mana) mana = as->power; */
            }

            /* Ignore "difficult" normal books */
            if ((when > 5) && (when >= borg.trait[BI_MAXCLEVEL] + 2))
                continue;
            /* if (mana > borg.trait[BI_MAXSP]) continue; */

            /* Reward the book */
            k = 0;
            for (; k < 1 && k < borg.amt_book[book]; k++)
                value += 500000L;
            if (borg.trait[BI_STR] > 5)
                for (; k < 2 && k < borg.amt_book[book]; k++)
                    value += 10000L;
        }
    }

    /* Apply "encumbrance" from weight */

    /* XXX XXX XXX Apply "encumbrance" from weight */
    if (borg.trait[BI_WEIGHT] > borg.trait[BI_CARRY] / 2) {
        /* *HACK*  when testing items, the borg puts them in the last empty */
        /* slot so this is POSSIBLY just a test item */
        borg_item *item = NULL;
        for (int i = PACK_SLOTS; i >= 0; i--) {
            if (borg_items[i].iqty) {
                item = &borg_items[i];
                break;
            }
        }

        /* Some items will be used immediately and should not contribute to
         * encumbrance */
        if (item && item->iqty && item->aware
            && ((item->tval == TV_SCROLL
                    && ((item->sval == sv_scroll_enchant_armor
                            && borg.trait[BI_AENCH_ARM] < 1000
                            && borg.trait[BI_NEED_ENCHANT_TO_A])
                        || (item->sval == sv_scroll_enchant_weapon_to_hit
                            && borg.trait[BI_AENCH_TOH] < 1000
                            && borg.trait[BI_NEED_ENCHANT_TO_H])
                        || (item->sval == sv_scroll_enchant_weapon_to_dam
                            && borg.trait[BI_AENCH_TOD] < 1000
                            && borg.trait[BI_NEED_ENCHANT_TO_D])
                        || item->sval == sv_scroll_star_enchant_weapon
                        || item->sval == sv_scroll_star_enchant_armor))
                || (item->tval == TV_POTION
                    && (item->sval == sv_potion_inc_str
                        || item->sval == sv_potion_inc_int
                        || item->sval == sv_potion_inc_wis
                        || item->sval == sv_potion_inc_dex
                        || item->sval == sv_potion_inc_con
                        || item->sval == sv_potion_inc_all)))) {
            /* No encumbrance penalty for purchasing these items */
        } else {
            value -= ((borg.trait[BI_WEIGHT] - (borg.trait[BI_CARRY] / 2))
                      / (borg.trait[BI_CARRY] / 10) * 1000L);
        }
    }

    /* Return the value */
    return (value);
}

/*
 * Calculate the "power" of the Borg
 */
int32_t borg_power(void)
{
    int     i = 1;
    int32_t value = 0L;

    if (borg_cfg[BORG_USES_DYNAMIC_CALCS]) {
        value += borg_power_dynamic();
    } else {
        /* Process the equipment */
        value += borg_power_equipment();

        /* Process the inventory */
        value += borg_power_inventory();
    }

    /* Add a bonus for deep level prep */
    /* Dump prep codes */
    /* Scan from surface to deep , stop when not preped */
    for (i = 1; i <= borg.trait[BI_MAXDEPTH] + 50; i++) {
        /* Dump fear code*/
        if ((char *)NULL != borg_prepared(i))
            break;
    }
    value += ((i - 1) * 40000L);

    /* Add the value for the swap items */
    value += weapon_swap_value;
    value += armour_swap_value;

    /* Return the value */
    return (value);
}
#endif
