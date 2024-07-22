/**
 * \file borg-danger.c
 * \brief Determine how dangerous a square is
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

#include "borg-danger.h"

#ifdef ALLOW_BORG

#include "../game-world.h"
#include "../player-calcs.h"

#include "borg-cave-util.h"
#include "borg-cave.h"
#include "borg-fight-attack.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Hack -- extra fear per "region"
 */
uint16_t borg_fear_region[(AUTO_MAX_Y / 11) + 1][(AUTO_MAX_X / 11) + 1];

/*
 * Hack -- extra fear per "region" induced from extra monsters.
 */
uint16_t borg_fear_monsters[AUTO_MAX_Y + 1][AUTO_MAX_X + 1];

/*
 * Recalculate danger
 */
bool borg_danger_wipe = false;

/*
 * Calculate base danger from a monster's physical attacks
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 *
 * We reduce the danger from distant "sleeping" monsters.
 * PFE reduces my fear of an area.
 *
 */
static int borg_danger_physical(int i, bool full_damage)
{
    int k, n = 0;
    int pfe = 0;
    int power, chance;

    int16_t ac                 = borg.trait[BI_ARMOR];

    borg_kill *kill            = &borg_kills[i];

    struct monster_race *r_ptr = &r_info[kill->r_idx];

    /* shields gives +50 to ac and deflects some missiles and balls*/
    if (borg.temp.shield)
        ac += 50;

    /*  PFE gives a protection.  */
    /* Hack -- Apply PROTECTION_FROM_EVIL */
    if ((borg.temp.prot_from_evil) && (rf_has(r_ptr->flags, RF_EVIL))
        && ((borg.trait[BI_CLEVEL]) >= r_ptr->level)) {
        pfe = 1;
    }

    /* Mega-Hack -- unknown monsters */
    if (kill->r_idx == 0)
        return 1000;
    if (kill->r_idx >= z_info->r_max)
        return 1000;

    /* Analyze each physical attack */
    for (k = 0; k < z_info->mon_blows_max; k++) {
        int z = 0;

        /* Extract the attack infomation */
        struct blow_effect *effect = r_ptr->blow[k].effect;
        struct blow_method *method = r_ptr->blow[k].method;
        int                 d_dice = r_ptr->blow[k].dice.dice;
        int                 d_side = r_ptr->blow[k].dice.sides;

        power                      = 0;

        /* Done */
        if (!method)
            break;

        /* Analyze the attack */
        switch (borg_mon_blow_effect(effect->name)) {
        case MONBLOW_HURT:
            z = (d_dice * d_side);
            /* stun */
            if ((d_side < 3) && (z > d_dice * d_side)) {
                n += 200;
            }
            /* fudge- only mystics kick and they tend to KO.  Avoid close */
            /* combat like the plague */
            if (method->stun) {
                n += 400;
            }
            power = 60;
            if ((pfe) && !borg_attacking) {
                z /= 2;
            }
            break;

        case MONBLOW_POISON:
            z     = (d_dice * d_side);
            power = 5;
            if (borg.trait[BI_RPOIS])
                break;
            if (borg.temp.res_pois)
                break;
            /* Add fear for the effect */
            z += 10;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_DISENCHANT:
            z     = (d_dice * d_side);
            power = 20;
            if (borg.trait[BI_RDIS])
                break;
            /* Add fear for the effect */
            z += 500;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_DRAIN_CHARGES:
            z = (d_dice * d_side);
            /* Add fear for the effect */
            z += 20;
            power = 15;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EAT_GOLD:
            z = (d_dice * d_side);
            /* if in town and low level avoid them stupid urchins */
            if (borg.trait[BI_CLEVEL] < 5)
                z += 50;
            power = 5;
            if (100 <= adj_dex_safe[borg.stat_ind[STAT_DEX]]
                           + borg.trait[BI_CLEVEL])
                break;
            if (borg.trait[BI_GOLD] < 100)
                break;
            if (borg.trait[BI_GOLD] > 100000)
                break;
            /* Add fear for the effect */
            z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EAT_ITEM:
            z     = (d_dice * d_side);
            power = 5;
            if (100 <= adj_dex_safe[borg.stat_ind[STAT_DEX]]
                           + borg.trait[BI_CLEVEL])
                break;
            /* Add fear for the effect */
            z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EAT_FOOD:
            z     = (d_dice * d_side);
            power = 5;
            if (borg.trait[BI_FOOD] > 5)
                break;
            /* Add fear for the effect */
            z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EAT_LIGHT:
            z     = (d_dice * d_side);
            power = 5;
            if (borg.trait[BI_CURLITE] == 0)
                break;
            if (borg.trait[BI_AFUEL] > 5)
                break;
            /* Add fear for the effect */
            z += 5;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_ACID:
            if (borg.trait[BI_IACID])
                break;
            z = (d_dice * d_side);
            if (borg.trait[BI_RACID])
                z = (z + 2) / 3;
            if (borg.temp.res_acid)
                z = (z + 2) / 3;
            /* Add fear for the effect */
            z += 200; /* We don't want our armour corroded. */
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_ELEC:
            if (borg.trait[BI_IELEC])
                break;
            z     = (d_dice * d_side);
            power = 10;
            if (borg.trait[BI_RELEC])
                z = (z + 2) / 3;
            if (borg.temp.res_elec)
                z = (z + 2) / 3;
            /* Add fear for the effect */
            z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_FIRE:
            if (borg.trait[BI_IFIRE])
                break;
            z     = (d_dice * d_side);
            power = 10;
            if (borg.trait[BI_RFIRE])
                z = (z + 2) / 3;
            if (borg.temp.res_fire)
                z = (z + 2) / 3;
            /* Add fear for the effect */
            z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_COLD:
            if (borg.trait[BI_ICOLD])
                break;
            z     = (d_dice * d_side);
            power = 10;
            if (borg.trait[BI_RCOLD])
                z = (z + 2) / 3;
            if (borg.temp.res_acid)
                z = (z + 2) / 3;
            /* Add fear for the effect */
            z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_BLIND:
            z     = (d_dice * d_side);
            power = 2;
            if (borg.trait[BI_RBLIND])
                break;
            /* Add fear for the effect */
            z += 10;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                z += 75;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_CONFUSE:
            z     = (d_dice * d_side);
            power = 10;
            if (borg.trait[BI_RCONF])
                break;
            /* Add fear for the effect */
            z += 200;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                z += 200;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_TERRIFY:
            z     = (d_dice * d_side);
            power = 10;
            if (borg.trait[BI_RFEAR])
                break;
            /* Add fear for the effect */
            z = z * 2;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_PARALYZE:
            z     = (d_dice * d_side);
            power = 2;
            if (borg.trait[BI_FRACT])
                break;
            z += 200;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_LOSE_STR:
            z = (d_dice * d_side);
            if (borg.trait[BI_SSTR])
                break;
            if (borg.stat_cur[STAT_STR] <= 3)
                break;
            if (borg_spell_legal(RESTORATION))
                break;
            if (borg_spell_legal(REVITALIZE))
                break;
            if (borg_spell_legal(UNHOLY_REPRIEVE))
                break;
            z += 150;
            /* extra scary to have str drain below 10 */
            if (borg.stat_cur[STAT_STR] < 10)
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_LOSE_DEX:
            z = (d_dice * d_side);
            if (borg.trait[BI_SDEX])
                break;
            if (borg.stat_cur[STAT_DEX] <= 3)
                break;
            if (borg_spell_legal(RESTORATION))
                break;
            if (borg_spell_legal(REVITALIZE))
                break;
            z += 150;
            /* extra scary to have drain below 10 */
            if (borg.stat_cur[STAT_DEX] < 10)
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_LOSE_CON:
            z = (d_dice * d_side);
            if (borg.trait[BI_SCON])
                break;
            if (borg.stat_cur[STAT_CON] <= 3)
                break;
            if (borg_spell_legal(RESTORATION))
                break;
            if (borg_spell_legal(REVITALIZE))
                break;
            if (borg_spell_legal(UNHOLY_REPRIEVE))
                break;
            /* Add fear for the effect */
            z += 150;
            /* extra scary to have con drain below 8 */
            if (borg.stat_cur[STAT_STR] < 8)
                z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_LOSE_INT:
            z = (d_dice * d_side);
            if (borg.trait[BI_SINT])
                break;
            if (borg.stat_cur[STAT_INT] <= 3)
                break;
            if (borg_spell_legal(RESTORATION))
                break;
            if (borg_spell_legal(REVITALIZE))
                break;
            if (borg_spell_legal(UNHOLY_REPRIEVE))
                break;
            z += 150;
            /* extra scary for spell caster */
            if (borg_spell_stat() == STAT_INT)
                z += 50;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_LOSE_WIS:
            z = (d_dice * d_side);
            if (borg.trait[BI_SWIS])
                break;
            if (borg.stat_cur[STAT_WIS] <= 3)
                break;
            if (borg_spell_legal(RESTORATION))
                break;
            if (borg_spell_legal(REVITALIZE))
                break;
            z += 150;
            /* extra scary for pray'er */
            if (borg_spell_stat() == STAT_WIS)
                z += 50;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_LOSE_ALL:
            z     = (d_dice * d_side);
            power = 2;
            /* only morgoth. HACK to make it easier to fight him */
            break;

        case MONBLOW_SHATTER:
            z = (d_dice * d_side);
            z -= (z * ((ac < 150) ? ac : 150) / 250);
            power = 60;
            /* Add fear for the effect */
            z += 150;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EXP_10:
            z = (d_dice * d_side);
            if (borg.trait[BI_HLIFE])
                break;
            /* do not worry about drain exp after level 50 */
            if (borg.trait[BI_CLEVEL] == 50)
                break;
            if (borg_spell_legal(REMEMBRANCE)
                || borg_spell_legal(UNHOLY_REPRIEVE)
                || borg_spell_legal(REVITALIZE))
                break;
            /* Add fear for the effect */
            z += 100;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EXP_20:
            z = (d_dice * d_side);
            if (borg.trait[BI_HLIFE])
                break;
            /* do not worry about drain exp after level 50 */
            if (borg.trait[BI_CLEVEL] >= 50)
                break;
            if (borg_spell_legal(REMEMBRANCE)
                || borg_spell_legal(UNHOLY_REPRIEVE)
                || borg_spell_legal(REVITALIZE))
                break;
            /* Add fear for the effect */
            z += 150;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EXP_40:
            z = (d_dice * d_side);
            if (borg.trait[BI_HLIFE])
                break;
            /* do not worry about drain exp after level 50 */
            if (borg.trait[BI_CLEVEL] >= 50)
                break;
            if (borg_spell_legal(REMEMBRANCE)
                || borg_spell_legal(UNHOLY_REPRIEVE)
                || borg_spell_legal(REVITALIZE))
                break;
            /* Add fear for the effect */
            z += 200;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_EXP_80:
            z = (d_dice * d_side);
            if (borg.trait[BI_HLIFE])
                break;
            /* do not worry about drain exp after level 50 */
            if (borg.trait[BI_CLEVEL] >= 50)
                break;
            if (borg_spell_legal(REMEMBRANCE)
                || borg_spell_legal(UNHOLY_REPRIEVE)
                || borg_spell_legal(REVITALIZE))
                break;
            /* Add fear for the effect */
            z += 250;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;

        case MONBLOW_HALLU:
            z = (d_dice * d_side);
            /* Add fear for the effect */
            z += 250;
            if ((pfe) && !borg_attacking)
                z /= 2;
            break;
        }

        /* reduce by damage reduction */
        z -= borg.trait[BI_DAM_RED];
        if (z < 0)
            z = 0;

        /* if we are doing partial damage reduce for % chance that it will */
        /* hit you. */
        if (!full_damage) {
            /* figure out chance that monster will hit you. */
            /* add a 50% bonus in to account for bad luck. */
            if (borg_fighting_unique || (r_ptr->level + power) > 0)
                chance
                    = 150
                      - (((ac * 300) / 4) / 1 + ((r_ptr->level + power) * 3));
            else
                chance = -1;

            /* always have a 5% chance of hitting. */
            if (chance < 5)
                chance = 5;

            /* Modify the damage by the chance of getting hit */
            z = (z * chance) / 100;
        }

        /* Add in damage */
        n += z;
    }

    /* Danger */
    return n;
}

/*
 * Calculate base danger from a monster's spell attacks
 *
 * We attempt to take account of various resistances, both in
 * terms of actual damage, and special effects, as appropriate.
 *
 * We reduce the danger from distant "sleeping" monsters.
 *
 * We reduce the danger if the monster is immobile or not LOS
 */
static int borg_danger_spell(
    int i, int y, int x, int d, bool average, bool full_damage)
{
    int q, n = 0, pfe = 0, glyph = 0, glyph_check = 0;

    int spot_x, spot_y, spot_safe = 1;

    int hp, total_dam = 0, av;

    bool bolt       = false;

    borg_kill *kill = &borg_kills[i];

    borg_grid *ag;

    struct monster_race *r_ptr = &r_info[kill->r_idx];

    /*  PFE gives a protection.  */
    /* Hack -- Apply PROTECTION_FROM_EVIL */
    if ((borg.temp.prot_from_evil) && (rf_has(r_ptr->flags, RF_EVIL))
        && ((borg.trait[BI_CLEVEL]) >= r_ptr->level)) {
        pfe = 1;
    }

    /* glyph of warding rune of protection provides some small
     * protection with some ranged atacks; mainly summon attacks.
     * We should reduce the danger commensurate to the probability of the
     * monster breaking the glyph as defined by melee2.c
     */
    if (borg_on_glyph) {
        glyph = 1;
    } else if (track_glyph.num) {
        /* Check all existing glyphs */
        for (glyph_check = 0; glyph_check < track_glyph.num; glyph_check++) {
            if ((track_glyph.y[glyph_check] == y)
                && (track_glyph.x[glyph_check] == x)) {
                /* Reduce the danger */
                glyph = 1;
            }
        }
    }

    /* Mega-Hack -- unknown monsters */
    if (kill->r_idx == 0)
        return 1000;
    if (kill->r_idx >= z_info->r_max)
        return 1000;

    /* Paranoia -- Nothing to cast */
    if (!kill->ranged_attack)
        return 0;

    /* Extract hit-points */
    hp = kill->power;

    /* Analyze the spells */
    for (q = 0; q < kill->ranged_attack; q++) {
        int p = 0;

        int z = 0;

        /* Cast the spell. */
        switch (kill->spell[q]) {
        case RSF_SHRIEK:
            /* if looking at full damage, things that are just annoying */
            /* do not count.*/
            /* Add fear for the effect */
            p += 5;
            break;

        case RSF_WHIP:
            if (d < 3) {
                z = 100;
            }
            break;

        case RSF_SPIT:
            if (d < 4) {
                z = 100;
            }
            break;

        case RSF_SHOT:
            z = ((r_ptr->spell_power / 8) + 1) * 5;
            break;

        case RSF_ARROW:
            z = ((r_ptr->spell_power / 8) + 1) * 6;
            break;

        case RSF_BOLT:
            z = ((r_ptr->spell_power / 8) + 1) * 7;
            break;

        case RSF_BR_ACID:
            if (borg.trait[BI_IACID])
                break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg.trait[BI_RACID])
                z = (z + 2) / 3;
            if (borg.temp.res_acid)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count.*/
            /* Add fear for the effect */
            p += 40;
            break;

        case RSF_BR_ELEC:
            if (borg.trait[BI_IELEC])
                break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg.trait[BI_RELEC])
                z = (z + 2) / 3;
            if (borg.temp.res_elec)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count.*/
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BR_FIRE:
            if (borg.trait[BI_IFIRE])
                break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg.trait[BI_RFIRE])
                z = (z + 2) / 3;
            if (borg.temp.res_fire)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 40;
            break;

        case RSF_BR_COLD:
            if (borg.trait[BI_ICOLD])
                break;
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            if (borg.trait[BI_RCOLD])
                z = (z + 2) / 3;
            if (borg.temp.res_cold)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BR_POIS:
            z = (hp / 3);
            /* max damage */
            if (z > 800)
                z = 800;
            if (borg.trait[BI_RPOIS])
                z = (z + 2) / 3;
            if (borg.temp.res_pois)
                z = (z + 2) / 3;
            if (borg.temp.res_pois)
                break;
            if (borg.trait[BI_RPOIS])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BR_NETH:
            z = (hp / 6);
            /* max damage */
            if (z > 600)
                z = 600;
            if (borg.trait[BI_RNTHR]) {
                z = (z * 6) / 9;
                break;
            }
            /* Add fear for the effect */
            p += 125;
            break;

        case RSF_BR_LIGHT:
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg.trait[BI_RLITE]) {
                z = (z * 2) / 3;
                break;
            }
            if (borg.trait[BI_RBLIND])
                break;
            p += 20;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                p += 20;
            break;

        case RSF_BR_DARK:
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg.trait[BI_RDARK])
                z = (z * 2) / 3;
            if (borg.trait[BI_RDARK])
                break;
            if (borg.trait[BI_RBLIND])
                break;
            p += 20;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                p += 20;
            break;

        case RSF_BR_SOUN:
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg.trait[BI_RSND])
                z = (z * 5) / 9;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                z += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                z += 1000;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_BR_CHAO:
            z = (hp / 6);
            /* max damage */
            if (z > 600)
                z = 600;
            if (borg.trait[BI_RKAOS])
                z = (z * 6) / 9;
            /* Add fear for the effect */
            p += 100;
            if (borg.trait[BI_RKAOS])
                break;
            p += 200;
            break;

        case RSF_BR_DISE:
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg.trait[BI_RDIS])
                z = (z * 6) / 10;
            if (borg.trait[BI_RDIS])
                break;
            p += 500;
            break;

        case RSF_BR_NEXU:
            z = (hp / 6);
            /* max damage */
            if (z > 400)
                z = 400;
            if (borg.trait[BI_RNXUS])
                z = (z * 6) / 10;
            if (borg.trait[BI_RNXUS])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 100;
            break;

        case RSF_BR_TIME:
            z = (hp / 3);
            /* max damage */
            if (z > 150)
                z = 150;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 250;
            break;

        case RSF_BR_INER:
            z = (hp / 6);
            /* max damage */
            if (z > 200)
                z = 200;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 100;
            break;

        case RSF_BR_GRAV:
            z = (hp / 3);
            /* max damage */
            if (z > 200)
                z = 200;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 100;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                z += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                z += 1000;
            break;

        case RSF_BR_SHAR:
            z = (hp / 6);
            /* max damage */
            if (z > 500)
                z = 500;
            if (borg.trait[BI_RSHRD])
                z = (z * 6) / 9;
            if (borg.trait[BI_RSHRD])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_BR_PLAS:
            z = (hp / 6);
            /* max damage */
            if (z > 150)
                z = 150;
            if (borg.trait[BI_RSND])
                break;
            /* Pump this up if you have goi so that the borg is sure */
            /* to be made nervous */
            p += 100;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                z += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                z += 1000;
            break;

        case RSF_BR_WALL:
            z = (hp / 6);
            /* max damage */
            if (z > 200)
                z = 200;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                z += 100;
            if (borg.trait[BI_ISHEAVYSTUN])
                z += 500;
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_BR_MANA:
            /* Nothing currently breaths mana but best to have a check */
            z = (hp / 3);
            /* max damage */
            if (z > 1600)
                z = 1600;
            break;

        case RSF_BOULDER:
            z    = ((1 + r_ptr->spell_power / 7) * 12);
            bolt = true;
            break;

        case RSF_WEAVE:
            /* annoying creation of traps */
            break;

        case RSF_BA_ACID:
            if (borg.trait[BI_IACID])
                break;
            z = (r_ptr->spell_power * 3) + 15;
            if (borg.trait[BI_RACID])
                z = (z + 2) / 3;
            if (borg.temp.res_acid)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 40;
            break;

        case RSF_BA_ELEC:
            if (borg.trait[BI_IELEC])
                break;
            z = (r_ptr->spell_power * 3) / 2 + 8;
            if (borg.trait[BI_RELEC])
                z = (z + 2) / 3;
            if (borg.temp.res_elec)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BA_FIRE:
            if (borg.trait[BI_IFIRE])
                break;
            z = (r_ptr->spell_power * 7) / 2 + 10;
            if (borg.trait[BI_RFIRE])
                z = (z + 2) / 3;
            if (borg.temp.res_fire)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 40;
            break;

        case RSF_BA_COLD:
            if (borg.trait[BI_ICOLD])
                break;
            z = (r_ptr->spell_power * 3 / 2) + 10;
            if (borg.trait[BI_RCOLD])
                z = (z + 2) / 3;
            if (borg.temp.res_cold)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BA_POIS:
            z = (r_ptr->spell_power / 2 + 3) * 4;
            if (borg.trait[BI_RPOIS])
                z = (z + 2) / 3;
            if (borg.temp.res_pois)
                z = (z + 2) / 3;
            if (borg.temp.res_pois)
                break;
            if (borg.trait[BI_RPOIS])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BA_SHAR:
            z = ((r_ptr->spell_power * 3) / 2) + 10;
            if (borg.trait[BI_RSHRD])
                z = (z * 6) / 9;
            if (borg.trait[BI_RSHRD])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BA_NETH:
            z = (r_ptr->spell_power * 4) + (10 * 10);
            if (borg.trait[BI_RNTHR])
                z = (z * 6) / 8;
            if (borg.trait[BI_RNTHR])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 250;
            break;

        case RSF_BA_WATE:
            z = (r_ptr->spell_power * 5) / 2 + 50;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                p += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                p += 1000;
            if (borg.trait[BI_RCONF])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 50;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                p += 20;
            break;

        case RSF_BA_MANA:
            z = ((r_ptr->spell_power * 5) + (10 * 10));
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_BA_HOLY:
            z = 10 + ((r_ptr->spell_power * 3) / 2 + 1) / 2;
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_BA_DARK:
            z = ((r_ptr->spell_power * 4) + (10 * 10));
            if (borg.trait[BI_RDARK])
                z = (z * 6) / 9;
            if (borg.trait[BI_RDARK])
                break;
            if (borg.trait[BI_RBLIND])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                p += 20;
            break;

        case RSF_BA_LIGHT:
            z = 10 + (r_ptr->spell_power * 3 / 2);
            if (borg.trait[BI_RLITE])
                z = (z * 6) / 9;
            if (borg.trait[BI_RLITE])
                break;
            if (borg.trait[BI_RBLIND])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                p += 20;
            break;

        case RSF_STORM:
            z = 70 + (r_ptr->spell_power * 5);
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                p += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                p += 1000;
            if (borg.trait[BI_RCONF])
                break;
            break;

        case RSF_DRAIN_MANA:
            if (borg.trait[BI_MAXSP])
                p += 100;
            break;

        case RSF_MIND_BLAST:
            if (borg.trait[BI_SAV] < 100)
                z = (r_ptr->spell_power / 2 + 1);
            break;

        case RSF_BRAIN_SMASH:
            z = (12 * (15 + 1)) / 2;
            p += 200 - 2 * borg.trait[BI_SAV];
            if (p < 0)
                p = 0;
            break;

        case RSF_WOUND:
            if (borg.trait[BI_SAV] >= 100)
                break;
            z = ((r_ptr->spell_power / 3 * 2) * 5);
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            /* reduce by % chance of save  (add 20% for fudge) */
            z = z * (120 - borg.trait[BI_SAV]) / 100;
            break;

        case RSF_BO_ACID:
            bolt = true;
            if (borg.trait[BI_IACID])
                break;
            z = ((7 * 8) + (r_ptr->spell_power / 3));
            if (borg.trait[BI_RACID])
                z = (z + 2) / 3;
            if (borg.temp.res_acid)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 40;
            break;

        case RSF_BO_ELEC:
            if (borg.trait[BI_IELEC])
                break;
            bolt = true;
            z    = ((4 * 8) + (r_ptr->spell_power / 3));
            if (borg.trait[BI_RELEC])
                z = (z + 2) / 3;
            if (borg.temp.res_elec)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BO_FIRE:
            if (borg.trait[BI_IFIRE])
                break;
            bolt = true;
            z    = ((9 * 8) + (r_ptr->spell_power / 3));
            if (borg.trait[BI_RFIRE])
                z = (z + 2) / 3;
            if (borg.temp.res_fire)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 40;
            break;

        case RSF_BO_COLD:
            if (borg.trait[BI_ICOLD])
                break;
            bolt = true;
            z    = ((6 * 8) + (r_ptr->spell_power / 3));
            if (borg.trait[BI_RCOLD])
                z = (z + 2) / 3;
            if (borg.temp.res_cold)
                z = (z + 2) / 3;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_BO_POIS:
            if (borg.trait[BI_IPOIS])
                break;
            z = ((9 * 8) + (r_ptr->spell_power / 3));
            if (borg.trait[BI_RPOIS])
                z = (z + 2) / 3;
            if (borg.temp.res_pois)
                z = (z + 2) / 3;
            bolt = true;
            break;

        case RSF_BO_NETH:
            bolt = true;
            z    = (5 * 5) + (r_ptr->spell_power * 3 / 2) + 50;
            if (borg.trait[BI_RNTHR])
                z = (z * 6) / 8;
            if (borg.trait[BI_RNTHR])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 200;
            break;

        case RSF_BO_WATE:
            z    = (10 * 10) + (r_ptr->spell_power);
            bolt = true;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                p += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                p += 1000;
            if (borg.trait[BI_RCONF])
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            if (borg.trait[BI_CLASS] == CLASS_MAGE)
                p += 20;
            break;

        case RSF_BO_MANA:
            z    = (r_ptr->spell_power * 5) / 2 + 50;
            bolt = true;
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_BO_PLAS:
            z    = (10 + (8 * 7) + (r_ptr->spell_power));
            bolt = true;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                z += 500;
            if (borg.trait[BI_ISHEAVYSTUN])
                z += 1000;
            break;

        case RSF_BO_ICE:
            z    = (6 * 6) + (r_ptr->spell_power);
            bolt = true;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            if (borg.trait[BI_RSND])
                break;
            /* if already stunned be REALLY nervous about this */
            if (borg.trait[BI_ISSTUN])
                z += 50;
            if (borg.trait[BI_ISHEAVYSTUN])
                z += 1000;
            break;

        case RSF_MISSILE:
            z    = ((2 * 6) + (r_ptr->spell_power / 3));
            bolt = true;
            break;

        case RSF_BE_ELEC:
            if (borg.trait[BI_IELEC])
                break;
            z = ((5 * 5) + (r_ptr->spell_power * 2) + 30);
            if (borg.trait[BI_RELEC])
                z = (z + 2) / 3;
            if (borg.temp.res_elec)
                z = (z + 2) / 3;
            bolt = true;
            break;

        case RSF_BE_NETH:
            bolt = true;
            z    = ((5 * 5) + (r_ptr->spell_power * 2) + 30);
            if (borg.trait[BI_RNTHR])
                z = (z * 6) / 8;
            if (borg.trait[BI_RNTHR])
                break;
            bolt = true;
            break;

        case RSF_SCARE:
            if (borg.trait[BI_SAV] >= 100)
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_BLIND:
            if (borg.trait[BI_RBLIND])
                break;
            if (borg.trait[BI_SAV] >= 100)
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_CONF:
            if (borg.trait[BI_RCONF])
                break;
            if (borg.trait[BI_SAV] >= 100)
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_SLOW:
            if (borg.trait[BI_FRACT])
                break;
            if (borg.trait[BI_SAV] >= 100)
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 5;
            break;

        case RSF_HOLD:
            if (borg.trait[BI_FRACT])
                break;
            if (borg.trait[BI_SAV] >= 100)
                break;
            p += 150;
            break;

        case RSF_HASTE:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_HEAL:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_HEAL_KIN:
            break;

        case RSF_BLINK:
            break;

        case RSF_TPORT:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_TELE_TO:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_TELE_SELF_TO:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 20;
            break;

        case RSF_TELE_AWAY:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 10;
            break;

        case RSF_TELE_LEVEL:
            if (borg.trait[BI_SAV] >= 100)
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_DARKNESS:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 5;
            break;

        case RSF_TRAPS:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 50;
            break;

        case RSF_FORGET:
            if (borg.trait[BI_SAV] >= 100)
                break;
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            {
                /* if you are a spell caster, this is a big issue */
                if (borg.trait[BI_CURSP] < 15) {
                    p += 500;
                } else {
                    p += 30;
                }
            }
            break;

        case RSF_SHAPECHANGE:
            /* if looking at full damage, things that are just annoying */
            /* do not count. */
            /* Add fear for the effect */
            p += 200;
            break;

            /* Summoning is only as dangerous as the monster that is
             * actually summoned but the monsters that summon are a priority
             * to kill.  PFE reduces danger from some evil summoned monsters
             * One Problem with GOI and Create Door is that the GOI reduces
             * the fear so much that the borg won't cast the Create Door,
             * even though it would be a good idea.
             */

        case RSF_S_KIN:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(
                            ag)) { /* track the safe areas for calculating
                                      danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 3;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;

            break;

        case RSF_S_HI_DEMON:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 6;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_MONSTER:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p += 0;
            else {
                p += (r_ptr->spell_power) * 5;
                p = p / spot_safe;
            }
            break;

        case RSF_S_MONSTERS:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p += 0;
            else {
                p += (r_ptr->spell_power) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_ANIMAL:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p += 0;
            else {
                p += (r_ptr->spell_power) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_SPIDER:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p += 0;
            else {
                p += (r_ptr->spell_power) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_HOUND:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe || glyph || borg_create_door || borg_fighting_unique)
                p += 0;
            else {
                p += (r_ptr->spell_power) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_HYDRA:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 2;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 5;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_AINU:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe || borg_fighting_unique) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 3;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_DEMON:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 3;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = (p * 75) / 100;
            break;

        case RSF_S_UNDEAD:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 3;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_DRAGON:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 3;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 7;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_HI_UNDEAD:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 6;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_HI_DRAGON:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 6;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_WRAITH:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door || borg_fighting_unique) {
                p += (r_ptr->spell_power) * 6;
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 12;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;

        case RSF_S_UNIQUE:
            /* This is used to calculate the free squares next to us.
             * This is important when dealing with summoners.
             */
            for (spot_x = -1; spot_x <= 1; spot_x++) {
                for (spot_y = -1; spot_y <= 1; spot_y++) {
                    /* Acquire location */
                    x  = spot_x + kill->pos.x;
                    y  = spot_y + kill->pos.y;

                    ag = &borg_grids[y][x];

                    /* skip our own spot */
                    if (x == kill->pos.x && y == kill->pos.y)
                        continue;

                    /* track spaces already protected */
                    if (borg_feature_protected(ag)) {
                        /* track the safe areas for calculating danger */
                        spot_safe++;

                        /* Just in case */
                        if (spot_safe == 0)
                            spot_safe = 1;
                        if (spot_safe == 8)
                            spot_safe = 100;
                        if (borg_morgoth_position || borg_as_position)
                            spot_safe = 1000;
                    }
                }
            }
            if (pfe) {
                p += (r_ptr->spell_power);
                p = p / spot_safe;
            } else if (glyph || borg_create_door) {
                p += (r_ptr->spell_power)
                     * 3; /* slightly reduced danger for unique */
                p = p / spot_safe;
            } else {
                p += (r_ptr->spell_power) * 6;
                p = p / spot_safe;
            }
            /* reduce the fear if it is a unique */
            if (rf_has(r_info->flags, RF_UNIQUE))
                p = p * 75 / 100;
            break;
        }

        /* A bolt spell cannot jump monsters to hit the borg. */
        if (bolt == true
            && !borg_projectable_pure(
                kill->pos.y, kill->pos.x, borg.c.y, borg.c.x))
            z = 0;

        /* Some borgs are concerned with the 'effects' of an attack.  ie, cold
         * attacks shatter potions, fire attacks burn scrolls, electric attacks
         * zap rings.
         */
        if (borg.trait[BI_MAXDEPTH] >= 75)
            p = 0;

        /* Notice damage */
        p += z;

        /* Track the most dangerous spell */
        if (p > n)
            n = p;

        /* Track the damage of all the spells, used in averaging */
        total_dam += p;
    }

    /* reduce by damage reduction */
    total_dam -= borg.trait[BI_DAM_RED];
    if (total_dam < 0)
        total_dam = 0;

    /* Slightly decrease the danger if the borg is sitting in
     * a sea of runes.
     */
    if (borg_morgoth_position || borg_as_position)
        total_dam = total_dam * 7 / 10;

    /* Average damage of all the spells & compare to most dangerous spell */
    av = total_dam / kill->ranged_attack;

    /* If the most dangerous spell is alot bigger than the average,
     * then return the dangerous one.
     *
     * There is a problem when dealing with defense maneuvers.
     * If the borg is considering casting a spell like
     * Resistance and the monster also has a non
     * resistible attack (like Disenchant) then the damage
     * returned will be for that spell, since the danger of the
     * others (like fire, cold) will be greatly reduced by the
     * proposed defense spell.  The result will be the borg will
     * not cast the resistance spell even though it may be a very
     * good idea.
     *
     * Example: a monster has three breath attacks (Fire, Ice,
     * Disenchant) and each hits for 800 pts of damage.  The
     * borg currently resists all three, so the danger would be
     * 500. If the borg were to use a Res Heat Potion that would
     * decrease the danger to:
     * Fire:  333
     * Ice:   500
     * Disen: 500
     * Now the Average is 444.  Not really worth it, nominal change.
     * But if the resistance spell was both Fire and Ice, then
     * it would be:
     * Fire:  333
     * Ice:   333
     * Disen: 500
     * With an average of 388. Probably worth it, but the borg
     * would see that the Disen attack is a quite dangerous and
     * would return the result of 500.
     *
     * To fix this, the flag 'average' is added to the
     * borg_danger() to skip this check and return the average
     * damage.  If the flag is false then the formula below is
     * SKIPPED and the value returned with be the average.
     * If the flag is true, then the formula below will be used
     * to determine the returned value.  Currently the elemental
     * resistance spells and PFE have the flag set as false.
     *
     */
    if (!average)
        return (av);
    if (n >= av * 15 / 10 || n > borg.trait[BI_CURHP] * 8 / 10)
        return n;
    else
        /* Average Danger */
        return (av);
}

/*
 * Calculate the danger to a grid from a monster  XXX XXX XXX
 *
 * Note that we are paranoid, especially about "monster speed",
 * since even if a monster is slower than us, it will occasionally
 * get one full turn to attack us.
 *
 * Note that we assume that monsters can walk through walls and
 * other monsters to get to the player.  XXX XXX XXX
 *
 * This function attempts to consider possibilities such as movement plus
 * spell attacks, physical attacks and spell attacks together,
 * and other similar situations.
 *
 * Currently we assume that "sleeping" monsters are less dangerous
 * unless you get near them, which may wake them up.
 *
 * We attempt to take into account things like monsters which sometimes
 * "stumble", and monsters which only "sometimes" use powerful spells.
 */
int borg_danger_one_kill(
    int y, int x, int c, int i, bool average, bool full_damage)
{
    borg_kill *kill            = &borg_kills[i];

    struct monster_race *r_ptr = &r_info[kill->r_idx];

    int x9                     = kill->pos.x;
    int y9                     = kill->pos.y;
    int y_temp, x_temp;

    int ax, ay, d;

    int q = 0, r, p, v1 = 0, v2 = 0, b_v2 = 0, b_v1 = 0;

    int fake_speed    = borg.trait[BI_SPEED];
    int monster_speed = kill->speed;
    int t, e;

    int ii;
    int chance;

    /* Paranoia */
    if (!kill->r_idx)
        return 0;

    /* Skip certain monster indexes.
     * These have been listed mainly in Teleport Other
     * checks in borg6.c in the defense maneuvers.
     */
    if (borg_tp_other_n) {
        for (ii = 1; ii <= borg_tp_other_n; ii++) {
            /* Is the current danger check same as a saved monster index? */
            if (i == borg_tp_other_index[ii]) {
                return 0;
            }
        }
    }

    /* Distance components */
    ax = (x9 > x) ? (x9 - x) : (x - x9);
    ay = (y9 > y) ? (y9 - y) : (y - y9);

    /* Distance */
    d = MAX(ax, ay);

    /* Minimal distance */
    if (d < 1)
        d = 1;

    /* Minimal distance */
    if (d > 20)
        return 0;

    /* A very speedy borg will miscalculate danger of some monsters */
    if (borg.trait[BI_SPEED] >= 135)
        fake_speed = (borg_fighting_unique ? 120 : 125);

    /* Consider the character haste and slow monster spells */
    if (borg.temp.fast)
        fake_speed += 10;
    if (borg_slow_spell)
        monster_speed -= 10;

    /* Assume monsters are a little fast when you are low level */
    if (borg.trait[BI_MAXHP] < 20 && borg.trait[BI_CDEPTH])
        monster_speed += 3;

    /* Player energy per game turn  */
    e = extract_energy[(fake_speed)];

    /* Game turns per player move  */
    t = (100 + (e - 1)) / e;

    /*  Monster energy per game turn  */
    e = extract_energy[monster_speed];

    /* Monster moves */
    q = c * ((t * e) / 10);

    /* allow partial hits when not calculating full possible damage */
    if (full_damage)
        q = (int)((q + 9) / 10) * 10;

    /* Minimal energy.  Monsters get at least some energy.
     * If the borg is very fast relative to a monster, then the
     * monster danger is artificially low due to the way the borg
     * will calculate the danger and energy.  So the monsters must
     * be given some base energy to equate the borg's.
     * ie:  the borg with speed +40 (speed = 150) is attacking
     * a monster with normal speed (speed = 110).  One would
     * think that the borg gets 4 attacks per turn over the monster.
     * and this does happen.  What if the monster can deal out
     * 1000 damage pts per monster attack turn?  The borg will
     * reduce the danger to 250 because the borg is 4x faster
     * than the monster.  But eventually the borg will get hit
     * by that 1000 pt damage attack.  And when it does, its
     * going to hurt.
     * So we make sure the monster is at least as fast as us.
     * But the monster is allowed to be faster than us.
     */
    if (q <= 10)
        q = 10;

    /** Danger from physical attacks **/

    /* Physical attacks */
    v1 = borg_danger_physical(i, full_damage);

    /* Hack -- Under Stressful Situation.
     */
    if (borg.time_this_panel > 1200 || borg_t > 25000) {
        /* he might be stuck and could overflow */
        v1 = v1 / 5;
    }

    /* No attacks for some monsters */
    if (rf_has(r_ptr->flags, RF_NEVER_BLOW)) {
        v1 = 0;
    }

    /* No movement for some monsters */
    if ((rf_has(r_ptr->flags, RF_NEVER_MOVE)) && (d > 1)) {
        v1 = 0;
    }

    /* multipliers yeild some trouble when I am weak */
    if ((rf_has(r_ptr->flags, RF_MULTIPLY))
        && (borg.trait[BI_CLEVEL] < 20)) { /* extra 50% */
        v1 = v1 + (v1 * 15 / 10);
    }

    /* Friends yeild some trouble when I am weak */
    if ((r_ptr->friends || r_ptr->friends_base)
        && (borg.trait[BI_CLEVEL] < 20)) {
        if (borg.trait[BI_CLEVEL] < 15) {
            /* extra 80% */
            v1 = v1 + (v1 * 18 / 10);
        } else {
            /* extra 30% */
            v1 = v1 + (v1 * 13 / 10);
        }
    }

    /* Reduce danger from sleeping monsters */
    if (!kill->awake) {
        int inc = r_ptr->sleep + 5;
        /* Reduce the fear if Borg is higher level */
        if (borg.trait[BI_CLEVEL] >= 25) {
            v1 = v1 / 2;
        }

        /* Tweak danger based on the "alertness" of the monster */
        /* increase the danger for light sleepers */
        v1 = v1 + (v1 * inc / 100);
    }
    /* Reduce danger from sleeping monsters with the sleep 2 spell*/
    if (borg_sleep_spell_ii) {
        if ((d == 1) && (kill->awake) && (!(rf_has(r_ptr->flags, RF_NO_SLEEP)))
            && (!(rf_has(r_ptr->flags, RF_UNIQUE)))
            && (kill->level <= (borg.trait[BI_CLEVEL] - 15))) {
            /* Under special circumstances force the damage to 0 */
            if (borg.trait[BI_CLEVEL] < 20
                && borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2) {
                v1 = 0;
            } else {
                v1 = v1 / 3;
            }
        }
    }
    /* Reduce danger from sleeping monsters with the sleep 1,3 spell*/
    if (borg_sleep_spell) {
        if (kill->awake && (!(rf_has(r_ptr->flags, RF_NO_SLEEP)))
            && (!(rf_has(r_ptr->flags, RF_UNIQUE)))
            && (kill->level <= (borg.trait[BI_CLEVEL] - 15))) {
            /* Under special circumstances force the damage to 0 */
            if (borg.trait[BI_CLEVEL] < 20
                && borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2) {
                v1 = 0;
            } else {
                v1 = v1 / (d + 2);
            }
        }
    }
    if (borg_crush_spell) {
        /* HACK for now, either it dies or it doesn't.  */
        /* If we discover it isn't using this spell much, we can modify */
        if ((kill->power * kill->injury) / 100 < borg.trait[BI_CLEVEL] * 4)
            v1 = 0;
    }

    /* Reduce danger from confused monsters */
    if (kill->confused) {
        v1 = v1 / 2;
    }
    if (kill->stunned) {
        v1 = v1 * 10 / 13;
    }
    if (borg_confuse_spell) {
        if (kill->awake && !kill->confused
            && (!(rf_has(r_ptr->flags, RF_NO_SLEEP)))
            && (!(rf_has(r_ptr->flags, RF_UNIQUE)))
            && (kill->level <= (borg.trait[BI_CLEVEL] - 15))) {
            /* Under special circumstances force the damage to 0 */
            if (borg.trait[BI_CLEVEL] < 20
                && borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2) {
                v1 = 0;
            } else {
                v1 = v1 / (d + 2);
            }
        }
    }
    /* Perceive a reduce danger from scared monsters */
    if (borg_fear_mon_spell) {
        v1 = 0;
    }

    /* Hack -- Physical attacks require proximity
     *
     * Note that we do try to consider a fast monster moving and attacking
     * in the same round.  We should consider monsters that have a speed 2 or 3
     * classes higher than ours, but most times, the borg will only encounter
     * monsters with a single category higher speed.
     */
    if (q > 10 && d != 1 && !(rf_has(r_ptr->flags, RF_NEVER_MOVE))) {
        b_v1 = 0;

        /* Check for a single grid movement, simulating the monster's move
         * action. */
        for (ii = 0; ii < 8; ii++) {
            /* Obtain a grid to which the monster might move */
            y_temp = y9 + ddy_ddd[ii];
            x_temp = x9 + ddx_ddd[ii];

            /* Check for legality */
            if (!square_in_bounds_fully(cave, loc(x_temp, y_temp)))
                continue;

            /* Cannot occupy another monster's grid */
            if (borg_grids[y_temp][x_temp].kill)
                continue;

            /* Cannot occupy a closed door */
            if (borg_grids[y_temp][x_temp].feat == FEAT_CLOSED)
                continue;

            /* Cannot occupy a perma-wall */
            if (borg_grids[y_temp][x_temp].feat == FEAT_PERM)
                continue;

            /* Cannot occupy a wall/seam grid (unless pass_wall or kill_wall) */
            if (borg_grids[y_temp][x_temp].feat == FEAT_GRANITE
                || (borg_grids[y_temp][x_temp].feat == FEAT_MAGMA
                    || borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ
                    || borg_grids[y_temp][x_temp].feat == FEAT_MAGMA_K
                    || borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ_K
                    || borg_grids[y_temp][x_temp].feat == FEAT_RUBBLE)) {
                /* legally on a wall of some sort, check for closeness*/
                if (rf_has(r_ptr->flags, RF_PASS_WALL)) {
                    if (borg_distance(y_temp, x_temp, y, x) == 1)
                        b_v1 = v1;
                }
                if (rf_has(r_ptr->flags, RF_KILL_WALL)) {
                    if (borg_distance(y_temp, x_temp, y, x) == 1)
                        b_v1 = v1;
                }
            }

            /* Is this grid being considered adjacent to the grid for which the
             * borg_danger() was called? */
            if (borg_distance(y_temp, x_temp, y, x) > 1)
                continue;

            /* A legal floor grid */
            if (borg_cave_floor_bold(y_temp, x_temp)) {
                /* Really fast monster can hit me more than once after it's move
                 */
                b_v1 = v1 * (q / (d * 10));
            }
        }

        /* Monster is not able to move and threaten me in the same round */
        v1 = b_v1;
    }

    /* Consider a monster that is fast and can strike more than once per round
     */
    if (q > 10 && d == 1) {
        v1 = v1 * q / 10;
    }

    /* Need to be close if you are normal speed */
    if (q == 10 && d > 1) {
        v1 = 0;
    }

    /** Ranged Attacks **/
    v2 = borg_danger_spell(i, y, x, d, average, full_damage);

    /* Never cast spells */
    if (!r_ptr->freq_innate && !r_ptr->freq_spell) {
        v2 = 0;
    }

    /* Hack -- verify distance */
    if (borg_distance(y9, x9, y, x) > z_info->max_range) {
        v2 = 0;
    }

    /* Hack -- verify line of sight (both ways) for monsters who can only move 1
     * grid. */
    if (q <= 10 && !borg_projectable(y9, x9, y, x)
        && !borg_projectable(y, x, y9, x9)) {
        v2 = 0;
    }

    /* Hack -- verify line of sight (both ways) for monsters who can only move >
     *1 grid. Some fast monsters can take a move action and range attack in the
     *same round. Basically, we see how many grids the monster can move and
     *check LOS from each of those grids to determine the relative danger.  We
     *need to make sure that the monster is not passing through walls unless he
     *has that ability. Consider a fast monster who can move and cast a spell in
     *the same round. This is important for a borg looking for a safe grid from
     *a ranged attacker. If the attacker is fast then he might be able to move
     *to a grid which does have LOS to the grid the borg is considering.
     *
     * ##############
     * #.....#.#.1#D#   Grids marked 2 are threatened by the D currently.
     * #####.#..##@##	Grids marked 1 are safe currently, but the fast D will
     *be able
     * #####.#..1221#	to move to the borg's grid after he moves and the D will
     *be able
     * ##############	to use a ranged attack to grids 1, all in the same
     *round. The borg should not consider grid 1 as safe.
     */
    if (q >= 20) {
        int b_q = q;
        b_v2    = 0;

        /* Maximal speed check */
        if (q > 20)
            q = 20;

        /* Check for a single grid movement, simulating the monster's move
         * action. */
        for (ii = 0; ii < 8; ii++) {
            /* Obtain a grid to which the monster might move */
            y_temp = y9 + ddy_ddd[ii];
            x_temp = x9 + ddx_ddd[ii];

            /* Check for legality */
            if (!square_in_bounds_fully(cave, loc(x_temp, y_temp)))
                continue;

            /* Cannot occupy another monster's grid */
            if (borg_grids[y_temp][x_temp].kill)
                continue;

            /* Cannot occupy a closed door */
            if (borg_grids[y_temp][x_temp].feat == FEAT_CLOSED)
                continue;

            /* Cannot occupy a perma-wall */
            if (borg_grids[y_temp][x_temp].feat == FEAT_PERM)
                continue;

            /* Cannot occupy a wall/seam grid (unless pass_wall or kill_wall) */
            if (borg_grids[y_temp][x_temp].feat >= FEAT_GRANITE
                || (borg_grids[y_temp][x_temp].feat == FEAT_MAGMA
                    || borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ
                    || borg_grids[y_temp][x_temp].feat == FEAT_MAGMA_K
                    || borg_grids[y_temp][x_temp].feat == FEAT_QUARTZ_K
                    || borg_grids[y_temp][x_temp].feat == FEAT_RUBBLE)) {
                /* legally on a wall of some sort, check for LOS*/
                if (rf_has(r_ptr->flags, RF_PASS_WALL)) {
                    if (borg_projectable(y_temp, x_temp, y, x))
                        b_v2 = v2 * b_q / 10;
                }
                if (rf_has(r_ptr->flags, RF_KILL_WALL)) {
                    if (borg_projectable(y_temp, x_temp, y, x))
                        b_v2 = v2 * b_q / 10;
                }
            }

            /* Monster on a legal floor grid.  Can he see me? */
            else if (borg_projectable(y_temp, x_temp, y, x))
                b_v2 = v2 * b_q / 10;
        }

        /* Monster is not able to move and threaten me in the same round */
        v2 = b_v2;
    }

    /* Hack -- Under Stressful Situation.
     */
    if (borg.time_this_panel > 1200 || borg_t > 25000) {
        /* he might be stuck and could overflow */
        v2 = v2 / 5;
    }

    /* multipliers yield some trouble when I am weak */
    if ((rf_has(r_ptr->flags, RF_MULTIPLY)) && (borg.trait[BI_CLEVEL] < 20)) {
        v2 = v2 + (v2 * 12 / 10);
    }

    /* Friends yield some trouble when I am weak */
    if ((r_ptr->friends || r_ptr->friends_base)
        && (borg.trait[BI_CLEVEL] < 20)) {
        v2 = v2 + (v2 * 12 / 10);
    }

    /* Reduce danger from sleeping monsters */
    if (!kill->awake) {
        int inc = r_ptr->sleep + 5;
        /* weaklings and should still fear */
        if (borg.trait[BI_CLEVEL] >= 25) {
            v2 = v2 / 2;
        }

        /* Tweak danger based on the "alertness" of the monster */
        /* increase the danger for light sleepers */
        v2 = v2 + (v2 * inc / 100);
    }

    /* Reduce danger from sleeping monsters with the sleep 2 spell*/
    if (borg_sleep_spell_ii) {

        if ((d == 1) && (kill->awake) && (!(rf_has(r_ptr->flags, RF_NO_SLEEP)))
            && (!(rf_has(r_ptr->flags, RF_UNIQUE)))
            && (kill->level
                <= ((borg.trait[BI_CLEVEL] < 15)
                        ? borg.trait[BI_CLEVEL]
                        : (((borg.trait[BI_CLEVEL] - 10) / 4) * 3) + 10))) {
            v2 = v2 / 3;
        }
    }

    if (borg_crush_spell) {
        /* HACK for now, either it dies or it doesn't.  */
        /* If we discover it isn't using this spell much, we can modify */
        if ((kill->power * kill->injury) / 100 < borg.trait[BI_CLEVEL] * 4)
            v2 = 0;
    }

    /* Reduce danger from sleeping monsters with the sleep 1,3 spell*/
    if (borg_sleep_spell) {
        v2 = v2 / (d + 2);
    }
    /* Reduce danger from confused monsters */
    if (kill->confused) {
        v2 = v2 / 2;
    }
    /* Reduce danger from stunned monsters  */
    if (kill->stunned) {
        v2 = v2 * 10 / 13;
    }
    if (borg_confuse_spell) {
        v2 = v2 / 6;
    }

#if 0 /* They still cast spells, they are still dangerous */
    /* Reduce danger from scared monsters */
    if (borg_fear_mon_spell) {
        v2 = v2 * 8 / 10;
    }
    if (kill->afraid) {
        v2 = v2 * 8 / 10;
    }
#endif
    if (!full_damage) {
        /* reduce for frequency. */
        chance = (r_ptr->freq_innate + r_ptr->freq_spell) / 2;
        if (chance < 11)
            v2 = ((v2 * 4) / 10);
        else if (chance < 26)
            v2 = ((v2 * 6) / 10);
        else if (chance < 51)
            v2 = ((v2 * 8) / 10);
    }

    /* Danger */
    if (v2) {
        /* Full power */
        r = q;

        /* Total danger */
        v2 = v2 * r / 10;
    }

    /* Maximal danger */
    p = MAX(v1, v2);
    if (p > 2000)
        p = 2000;

    /* Result */
    return (p);
}

/*
 * Hack -- Calculate the "danger" of the given grid.
 *
 * Currently based on the physical power of nearby monsters, as well
 * as the spell power of monsters which can target the given grid.
 *
 * This function is extremely expensive, mostly due to the number of
 * times it is called, and also to the fact that it calls its helper
 * functions about thirty times each per call.
 *
 * We need to do more intelligent processing with the "c" parameter,
 * since currently the Borg does not realize that backing into a
 * hallway is a good idea, since as far as he can tell, many of
 * the nearby monsters can "squeeze" into a single grid.
 *
 * Note that we also take account of the danger of the "region" in
 * which the grid is located, which allows us to apply some "fear"
 * of invisible monsters and things of that nature.
 *
 * Generally bool Average is true.
 */
int borg_danger(int y, int x, int c, bool average, bool full_damage)
{
    int i, p = 0;

    struct loc l = loc(x, y);
    if (!square_in_bounds(cave, l))
        return 2000;

    /* Base danger (from regional fear) but not within a vault.  Cheating the
     * floor grid */
    if (!square_isvault(cave, l) && borg.trait[BI_CDEPTH] <= 80) {
        p += borg_fear_region[y / 11][x / 11] * c;
    }

    /* Reduce regional fear on Depth 100 */
    if (borg.trait[BI_CDEPTH] == 100 && p >= 300)
        p = 300;

    /* Added danger (from a lot of monsters).
     * But do not add it if we have been sitting on
     * this panel for too long, or monster's in a vault.  The fear_monsters[][]
     * can induce some bouncy behavior.
     */
    if (borg.time_this_panel <= 200 && !square_isvault(cave, loc(x, y)))
        p += borg_fear_monsters[y][x] * c;

    full_damage = true;

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Collect danger from monster */
        p += borg_danger_one_kill(y, x, c, i, average, full_damage);
    }

    /* Return the danger */
    return (p > 2000 ? 2000 : p);
}

#endif
