/**
 * \file borg-fight-attack.c
 * \brief Find the best attack
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

#include "borg-fight-attack.h"

#ifdef ALLOW_BORG

#include "../generate.h"
#include "../obj-slays.h"
#include "../obj-util.h"
#include "../player-calcs.h"
#include "../ui-menu.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-danger.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-take.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-id.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-item.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/* The use of this is a bit random. */
/*  Zero is "haven't shot" */
/*  positive is "good to shoot current target" */
/*  negative is "shot and missed" */
/*  negative 12 and lower is "do not shoot" */
int successful_target = 0;
int target_closest = 0;

/*
 * Maintain a set of special grids used for Teleport Other
 */
int16_t borg_tp_other_n = 0;
uint8_t borg_tp_other_x[255];
uint8_t borg_tp_other_y[255];
int     borg_tp_other_index[255];

/*
 * What effect does a blow from a monster have?
 *
 * *HACK* must match code in mon-blows.c
 */
int borg_mon_blow_effect(const char *name)
{
    static const struct {
        const char       *name;
        enum BORG_MONBLOW val;
    } monblow[] = { { "NONE", MONBLOW_NONE }, { "HURT", MONBLOW_HURT },
        { "POISON", MONBLOW_POISON }, { "DISENCHANT", MONBLOW_DISENCHANT },
        { "DRAIN_CHARGES", MONBLOW_DRAIN_CHARGES },
        { "EAT_GOLD", MONBLOW_EAT_GOLD }, { "EAT_ITEM", MONBLOW_EAT_ITEM },
        { "EAT_FOOD", MONBLOW_EAT_FOOD }, { "EAT_LIGHT", MONBLOW_EAT_LIGHT },
        { "ACID", MONBLOW_ACID }, { "ELEC", MONBLOW_ELEC },
        { "FIRE", MONBLOW_FIRE }, { "COLD", MONBLOW_COLD },
        { "BLIND", MONBLOW_BLIND }, { "CONFUSE", MONBLOW_CONFUSE },
        { "TERRIFY", MONBLOW_TERRIFY }, { "PARALYZE", MONBLOW_PARALYZE },
        { "LOSE_STR", MONBLOW_LOSE_STR }, { "LOSE_INT", MONBLOW_LOSE_INT },
        { "LOSE_WIS", MONBLOW_LOSE_WIS }, { "LOSE_DEX", MONBLOW_LOSE_DEX },
        { "LOSE_CON", MONBLOW_LOSE_CON }, { "LOSE_ALL", MONBLOW_LOSE_ALL },
        { "SHATTER", MONBLOW_SHATTER }, { "EXP_10", MONBLOW_EXP_10 },
        { "EXP_20", MONBLOW_EXP_20 }, { "EXP_40", MONBLOW_EXP_40 },
        { "EXP_80", MONBLOW_EXP_80 }, { "HALLU", MONBLOW_HALLU },
        { "BLACK_BREATH", MONBLOW_BLACK_BREATH }, { NULL, MONBLOW_NONE } };
    unsigned long i;
    for (i = 0; i < sizeof(monblow) / sizeof(monblow[0]); ++i)
        if (!strcmp(name, monblow[i].name))
            return monblow[i].val;
    return MONBLOW_NONE;
}

/*
 * Guess how much damage a physical attack will do to a monster
 */
static int borg_thrust_damage_one(int i)
{
    int dam;
    int mult;

    borg_kill *kill;

    struct monster_race *r_ptr;

    borg_item *item;

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

    if (((borg.trait[BI_WS_ANIMAL]) && (rf_has(r_ptr->flags, RF_ANIMAL)))
        || ((borg.trait[BI_WS_EVIL]) && (rf_has(r_ptr->flags, RF_EVIL))))
        mult = 2;
    if (((borg.trait[BI_WS_UNDEAD]) && (rf_has(r_ptr->flags, RF_UNDEAD)))
        || ((borg.trait[BI_WS_DEMON]) && (rf_has(r_ptr->flags, RF_DEMON)))
        || ((borg.trait[BI_WS_ORC]) && (rf_has(r_ptr->flags, RF_ORC)))
        || ((borg.trait[BI_WS_TROLL]) && (rf_has(r_ptr->flags, RF_TROLL)))
        || ((borg.trait[BI_WS_GIANT]) && (rf_has(r_ptr->flags, RF_GIANT)))
        || ((borg.trait[BI_WS_DRAGON]) && (rf_has(r_ptr->flags, RF_DRAGON)))
        || ((borg.trait[BI_WB_ACID]) && !(rf_has(r_ptr->flags, RF_IM_ACID)))
        || ((borg.trait[BI_WB_FIRE]) && !(rf_has(r_ptr->flags, RF_IM_FIRE)))
        || ((borg.trait[BI_WB_COLD]) && !(rf_has(r_ptr->flags, RF_IM_COLD)))
        || ((borg.trait[BI_WB_POIS]) && !(rf_has(r_ptr->flags, RF_IM_POIS)))
        || ((borg.trait[BI_WB_ELEC]) && !(rf_has(r_ptr->flags, RF_IM_ELEC))))
        mult = 3;
    if (((borg.trait[BI_WK_UNDEAD]) && (rf_has(r_ptr->flags, RF_UNDEAD)))
        || ((borg.trait[BI_WK_DEMON]) && (rf_has(r_ptr->flags, RF_DEMON)))
        || ((borg.trait[BI_WK_DRAGON]) && (rf_has(r_ptr->flags, RF_DRAGON))))
        mult = 5;

    /* add the multiplier */
    dam *= mult;

    /* add weapon bonuses */
    dam += item->to_d;

    /* add player bonuses */
    dam += borg.trait[BI_TODAM];

    /* multiply the damage for the whole round of attacks */
    dam *= borg.trait[BI_BLOWS];

    /* Bonuses for combat */
    chance = (borg.trait[BI_THN] + ((borg.trait[BI_TOHIT] + item->to_h) * 3));

    /* Chance of hitting the monsters AC */
    if (chance < (r_ptr->ac * 3 / 4) * 8 / 10)
        dam = 0;

    /* 5% automatic success/fail */
    if (chance > 95)
        chance = 95;
    if (chance < 5)
        chance = 5;

    /* add 10% to chance to give a bit more weight to weapons */
    if (borg.trait[BI_CLEVEL] > 15)
        chance += 10;

    /* Mages with Mana do not get that bonus, they should cast */
    if ((borg.trait[BI_CLASS] == CLASS_MAGE
            || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
        && borg.trait[BI_CURSP] > 1)
        chance -= 10;

    /* reduce damage by the % chance to hit */
    dam = (dam * chance) / 100;

    /* Try to place a minimal amount of damage */
    if (dam <= 0)
        dam = 1;

    /* Limit damage to twice maximal hitpoints */
    if (dam > kill->power * 2 && !rf_has(r_ptr->flags, RF_UNIQUE))
        dam = kill->power * 2;

    /*
     * Reduce the damage if a mage, they should not melee if they can avoid it
     */
    if ((borg.trait[BI_CLASS] == CLASS_MAGE
            || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
        && borg.trait[BI_MAXCLEVEL] < 40 && borg.trait[BI_CURSP] > 1)
        dam = (dam * 8 / 10) + 1;

    /*
     * Enhance the perceived damage on Uniques.  This way we target them
     * Keep in mind that he should hit the uniques but if he has a
     * x5 great bane of dragons, he will tend attack the dragon since the
     * perceived (and actual) damage is higher.  But don't select
     * the town uniques (maggot does no damage)
     *
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg.trait[BI_CDEPTH] >= 1)
        dam += (dam * 5);

    /* Hack -- ignore Maggot until later.  Player will chase Maggot
     * down all across the screen waking up all the monsters.  Then
     * he is stuck in a compromised situation.
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg.trait[BI_CDEPTH] == 0) {
        dam = dam * 2 / 3;

        /* Dont hunt maggot until later */
        if (borg.trait[BI_CLEVEL] < 5)
            dam = 0;
    }

    /* give a small bonus for whacking a breeder */
    if (rf_has(r_ptr->flags, RF_MULTIPLY))
        dam = (dam * 3 / 2);

    /* Enhance the perceived damage to summoner in order to influence the
     * choice of targets.
     */
    if ((rsf_has(r_ptr->spell_flags, RSF_S_KIN))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER))
        || (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS))
        || (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL))
        || (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HOUND))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA))
        || (rsf_has(r_ptr->spell_flags, RSF_S_AINU))
        || (rsf_has(r_ptr->spell_flags, RSF_S_DEMON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD))
        || (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD))
        || (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH))
        || (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE)))
        dam += ((dam * 3) / 2);

    /*
     * Apply massive damage bonus to Questor monsters to
     * encourage borg to strike them.
     */
    if (rf_has(r_ptr->flags, RF_QUESTOR))
        dam += (dam * 5);

    /* Damage */
    return dam;
}

/*
 * Simulate/Apply the optimal result of making a physical attack
 */
static int borg_attack_aux_thrust(void)
{
    int p, dir;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid *ag;

    borg_kill *kill;

    /* Too afraid to attack */
    if (borg.trait[BI_ISAFRAID] || borg.trait[BI_CRSFEAR])
        return 0;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++) {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require "adjacent" */
        if (borg_distance(borg.c.y, borg.c.x, y, x) > 1)
            continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill);

        /* No damage */
        if (d <= 0)
            continue;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg.munchkin_mode) {
            /* Calculate danger */
            p = borg_danger_one_kill(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg.trait[BI_CDEPTH] && !kill->awake)
            continue;

        /* Calculate "danger" to player */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg.trait[BI_MAXCLEVEL] > 15)
            p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d))
            continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return b_d;

    /* Save the location */
    borg.goal.g.x = borg_temp_x[b_i];
    borg.goal.g.y = borg_temp_y[b_i];

    ag            = &borg_grids[borg.goal.g.y][borg.goal.g.x];
    kill          = &borg_kills[ag->kill];

    /* Note */
    borg_note(format("# Facing %s at (%d,%d) who has %d Hit Points.",
        (r_info[kill->r_idx].name), borg.goal.g.y, borg.goal.g.x, kill->power));
    borg_note(
        format("# Attacking with weapon '%s'", borg_items[INVEN_WIELD].desc));

    /* Get a direction for attacking */
    dir = borg_extract_dir(borg.c.y, borg.c.x, borg.goal.g.y, borg.goal.g.x);

    /* Attack the grid */
    borg_keypress('+');
    borg_keypress(I2D(dir));

    /* Success */
    return b_d;
}

/* adapted from player_attack.c make_ranged_shot() */
static int borg_best_mult(borg_item *obj, struct monster_race *r_ptr)
{
    int i;
    int max_mult = 1;

    /* Brands */
    for (i = 1; i < z_info->brand_max; i++) {
        struct brand *brand = &brands[i];
        if (obj) {
            /* Brand is on an object */
            if (!obj->brands[i])
                continue;
        } else {
            /* Temporary brand */
            if (!player_has_temporary_brand(player, i))
                continue;
        }

        /* Is the monster vulnerable? */
        if (!rf_has(r_ptr->flags, brand->resist_flag)) {
            int mult = brand->multiplier;
            if (brand->vuln_flag && rf_has(r_ptr->flags, brand->vuln_flag)) {
                mult *= 2;
            }
            max_mult = MAX(mult, max_mult);
        }
    }

    /* Slays */
    for (i = 1; i < z_info->slay_max; i++) {
        struct slay *slay = &slays[i];
        if (obj) {
            /* Slay is on an object */
            if (!obj->slays[i])
                continue;
        } else {
            /* Temporary slay */
            if (!player_has_temporary_slay(player, i))
                continue;
        }

        if (rf_has(r_ptr->flags, slay->race_flag)) {
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
static int borg_launch_damage_one(int i, int dam, int typ, int ammo_location)
{
    int  p1, p2 = 0;
    int  j;
    bool borg_use_missile = false;
    int  ii;
    int  vault_grids = 0;
    int  x, y;
    int  k;
    bool gold_eater = false;
    int  chance     = 0;
    int  bonus      = 0;
    int  cur_dis    = 0;
    int  armor      = 0;

    borg_kill *kill;
    borg_grid *ag;

    struct monster_race *r_ptr;

    /* Monster record */
    kill = &borg_kills[i];

    /* Monster race */
    r_ptr = &r_info[kill->r_idx];

    /* How far away is the target? */
    cur_dis = distance(borg.c, kill->pos);

    /* Calculation our chance of hitting.  Player bonuses, Bow bonuses, Ammo
     * Bonuses */
    bonus  = (borg.trait[BI_TOHIT] + borg_items[INVEN_BOW].to_h
             + borg_items[ammo_location].to_h);
    chance = (borg.trait[BI_THB] + (bonus * BTH_PLUS_ADJ));
    armor  = r_ptr->ac + cur_dis;

    /* Very quickly look for gold eating monsters */
    for (k = 0; k < 4; k++) {
        /* gold eater */
        if (r_ptr->blow[k].effect
            && borg_mon_blow_effect(r_ptr->blow[k].effect->name)
                   == MONBLOW_EAT_GOLD)
            gold_eater = true;
    }

    /* Analyze the damage type */
    switch (typ) {
        /* Magic Missile */
    case BORG_ATTACK_MISSILE:
        break;

    case BORG_ATTACK_ARROW: {
        borg_item *bow  = &borg_items[INVEN_BOW];
        borg_item *ammo = &borg_items[ammo_location];
        int        mult = borg_best_mult(bow, r_ptr);
        mult            = MAX(mult, borg_best_mult(ammo, r_ptr));
        dam *= mult;
        /* don't point blank non-uniques */
        if (cur_dis == 1 && !(rf_has(r_ptr->flags, RF_UNIQUE)))
            dam /= 5;
        /* Do I hit regularly? (80%)*/
        if (chance < armor * 8 / 10)
            dam = 0;
    } break;

    /* Pure damage */
    case BORG_ATTACK_MANA:
        if (borg_fighting_unique && borg.has[kv_potion_restore_mana] > 3)
            dam *= 2;
        break;

    /* Meteor -- powerful magic missile */
    case BORG_ATTACK_METEOR:
        break;

    /* Acid */
    case BORG_ATTACK_ACID:
        if (rf_has(r_ptr->flags, RF_IM_ACID))
            dam = 0;
        break;

    /* Electricity */
    case BORG_ATTACK_ELEC:
        if (rf_has(r_ptr->flags, RF_IM_ELEC))
            dam = 0;
        break;

    /* Fire damage */
    case BORG_ATTACK_FIRE:
        if (rf_has(r_ptr->flags, RF_IM_FIRE))
            dam = 0;
        if ((rf_has(r_ptr->flags, RF_HURT_FIRE)))
            dam *= 2;
        break;

    /* Cold */
    case BORG_ATTACK_COLD:
        if (rf_has(r_ptr->flags, RF_IM_COLD))
            dam = 0;
        if (rf_has(r_ptr->flags, RF_HURT_COLD))
            dam *= 2;
        break;

    /* Poison */
    case BORG_ATTACK_POIS:
        if (rf_has(r_ptr->flags, RF_IM_POIS))
            dam = 0;
        break;

    /* Ice */
    case BORG_ATTACK_ICE:
        if (rf_has(r_ptr->flags, RF_IM_COLD))
            dam = 0;
        break;

    /* Holy Orb */
    case BORG_ATTACK_HOLY_ORB:
        if (rf_has(r_ptr->flags, RF_EVIL))
            dam *= 2;
        break;

    /* dispel undead */
    case BORG_ATTACK_DISP_UNDEAD:
        if (!(rf_has(r_ptr->flags, RF_UNDEAD)))
            dam = 0;
        break;

    /* dispel spirits */
    case BORG_ATTACK_DISP_SPIRITS:
        if (!(rf_has(r_ptr->flags, RF_SPIRIT)))
            dam = 0;
        break;

    /*  Dispel Evil */
    case BORG_ATTACK_DISP_EVIL:
        if (!(rf_has(r_ptr->flags, RF_EVIL)))
            dam = 0;
        break;

    /*  Dispel life */
    case BORG_ATTACK_DRAIN_LIFE:
        if (!(rf_has(r_ptr->flags, RF_NONLIVING)))
            dam = 0;
        if (!(rf_has(r_ptr->flags, RF_UNDEAD)))
            dam = 0;
        break;

    /*  Holy Word */
    case BORG_ATTACK_HOLY_WORD:
        if (!(rf_has(r_ptr->flags, RF_EVIL)))
            dam = 0;
        break;

    /* Weak Lite */
    case BORG_ATTACK_LIGHT_WEAK:
        if (!(rf_has(r_ptr->flags, RF_HURT_LIGHT)))
            dam = 0;
        break;

    /* Drain Life */
    case BORG_ATTACK_OLD_DRAIN:
        if (distance(borg.c, kill->pos) == 1)
            dam /= 5;
        if ((rf_has(r_ptr->flags, RF_UNDEAD))
            || (rf_has(r_ptr->flags, RF_DEMON))
            || (strchr("Egv", r_ptr->d_char))) {
            dam = 0;
        }
        break;

    /* Stone to Mud */
    case BORG_ATTACK_KILL_WALL:
        if (!(rf_has(r_ptr->flags, RF_HURT_ROCK)))
            dam = 0;
        break;

    /* New mage spell */
    case BORG_ATTACK_NETHER: {
        if (rf_has(r_ptr->flags, RF_UNDEAD)) {
            dam = 0;
        } else if (rsf_has(r_ptr->spell_flags, RSF_BR_NETH)) {
            dam *= 3;
            dam /= 9;
        } else if (rf_has(r_ptr->flags, RF_EVIL)) {
            dam /= 2;
        }
    } break;

    /* New mage spell */
    case BORG_ATTACK_CHAOS:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_CHAO)) {
            dam *= 3;
            dam /= 9;
        }
        /* If the monster is Unique full damage ok.
         * Otherwise, polymorphing will reset HP
         */
        if (!(rf_has(r_ptr->flags, RF_UNIQUE)))
            dam = -999;
        break;

    /* New mage spell */
    case BORG_ATTACK_GRAVITY:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_GRAV)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    /* New mage spell */
    case BORG_ATTACK_SHARD:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_SHAR)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    /* New mage spell */
    case BORG_ATTACK_SOUND:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_SOUN)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    /* Weird attacks */
    case BORG_ATTACK_PLASMA:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_PLAS)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_CONFU:
        if (rf_has(r_ptr->flags, RF_NO_CONF)) {
            dam = 0;
        }
        break;

    case BORG_ATTACK_DISEN:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_DISE)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_NEXUS:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_NEXU)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_FORCE:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_WALL)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_INERTIA:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_INER)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_TIME:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_TIME)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_LIGHT:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_LIGHT)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_DARK:
        if (rsf_has(r_ptr->spell_flags, RSF_BR_DARK)) {
            dam *= 3;
            dam /= 9;
        }
        break;

    case BORG_ATTACK_WATER:
        if (rsf_has(r_ptr->spell_flags, RSF_BA_WATE)) {
            dam *= 3;
            dam /= 9;
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
         * 2. Is it compromised by another spell?
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

        /* try not to teleport away uniques. These are the guys you are trying
         */
        /* to kill! */
        if (rf_has(r_ptr->flags, RF_UNIQUE)) {
            /* This unique is low on HP, finish it off */
            if (kill->injury >= 60)
                dam = -9999;

            /* I am sitting pretty in an AS-Corridor */
            else if (borg_as_position)
                dam = -9999;

            /* If this unique is causing the danger, get rid of it */
            else if (dam > avoidance * 13 / 10 && borg.trait[BI_CDEPTH] <= 98) {
                /* get rid of this unique by storing his info */
                borg_tp_other_index[borg_tp_other_n] = i;
                borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
                borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
                borg_tp_other_n++;
            }

            /* If fighting multiple uniques, get rid of one */
            else if (borg_fighting_unique >= 2 && borg_fighting_unique <= 8) {
                /* get rid of one unique or both if they are in a beam-line */
                borg_tp_other_index[borg_tp_other_n] = i;
                borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
                borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
                borg_tp_other_n++;
            }
            /* Unique is adjacent to Borg */
            else if (borg.trait[BI_CLASS] == CLASS_MAGE
                     && distance(borg.c, kill->pos) <= 2) {
                /* get rid of unique next to me */
                borg_tp_other_index[borg_tp_other_n] = i;
                borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
                borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
                borg_tp_other_n++;

            }
            /* Unique in a vault, get rid of it, clean vault */
            else if (vault_on_level) {
                /* Scan grids adjacent to monster */
                for (ii = 0; ii < 8; ii++) {
                    x = kill->pos.x + ddx_ddd[ii];
                    y = kill->pos.y + ddy_ddd[ii];

                    /* Access the grid */
                    ag = &borg_grids[y][x];

                    /* Skip unknown grids (important) */
                    if (ag->feat == FEAT_NONE)
                        continue;

                    /* Count adjacent Permas */
                    if (ag->feat == FEAT_PERM)
                        vault_grids++;
                }

                /* Near enough perma grids? */
                if (vault_grids >= 2) {
                    /* get rid of unique next to perma grids */
                    borg_tp_other_index[borg_tp_other_n] = i;
                    borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
                    borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
                    borg_tp_other_n++;
                }

            } else
                dam = -999;
        } else /* not a unique */
        {
            /* get rid of this non-unique by storing his info */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
            borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
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
        for (j = 0; j < 8; j++) {
            int y2 = kill->pos.y + ddy_ddd[j];
            int x2 = kill->pos.x + ddx_ddd[j];

            /* Get the grid */
            ag = &borg_grids[y2][x2];

            /* If its touching a glyph grid, nail it. */
            if (ag->glyph) {
                /* get rid of this one by storing his info */
                borg_tp_other_index[borg_tp_other_n] = i;
                borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
                borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
                borg_tp_other_n++;
                dam = 300;
            }
        }

        /* If the borg is not in a good position, do it */
        if (morgoth_on_level && !borg_morgoth_position) {
            /* get rid of this one by storing his info */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
            borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
            borg_tp_other_n++;
            dam = 100;
        }

        /* If the borg does not have enough Mana to attack this
         * round and cast Teleport Away next round, then do it now.
         */
        if (borg.trait[BI_CURSP] <= 35) {
            /* get rid of this unique by storing his info */
            borg_tp_other_index[borg_tp_other_n] = i;
            borg_tp_other_y[borg_tp_other_n]     = kill->pos.y;
            borg_tp_other_x[borg_tp_other_n]     = kill->pos.x;
            borg_tp_other_n++;
            dam = 150;
        }
        break;

    /* This BORG_ATTACK_ is hacked to work for Mass Genocide.  Since
     * we cannot mass gen uniques.
     */
    case BORG_ATTACK_DISP_ALL:
        if (rf_has(r_ptr->flags, RF_UNIQUE)) {
            dam = 0;
            break;
        }
        dam = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        break;

    case BORG_ATTACK_OLD_CONF:
        dam = 0;
        if (rf_has(r_ptr->flags, RF_NO_CONF))
            break;
        if (rf_has(r_ptr->flags, RF_MULTIPLY))
            break;
        if (kill->speed < r_ptr->speed - 5)
            break;
        if (kill->confused)
            break;
        if (!kill->awake)
            break;
        if ((kill->level
                > (borg.trait[BI_CLEVEL] < 13
                        ? 10
                        : (((borg.trait[BI_CLEVEL] - 10) / 4) * 3) + 10)))
            break;
        dam = -999;
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            break;
        borg_confuse_spell = false;
        p1 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        /* Make certain monsters appear to have more danger so the borg is more
         * likely to use this attack */
        if (kill->afraid && borg.trait[BI_CLEVEL] <= 10)
            p1 = p1 + 20;
        borg_confuse_spell = true;
        p2 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        borg_confuse_spell = false;
        dam                = (p1 - p2);
        break;

    case BORG_ATTACK_TURN_ALL:
        dam = 0;
        if (kill->speed < r_ptr->speed - 5)
            break;
        if (rf_has(r_ptr->flags, RF_NO_FEAR))
            break;
        if (kill->confused)
            break;
        if (!kill->awake)
            break;
        if ((kill->level
                > (borg.trait[BI_CLEVEL] < 13
                        ? 10
                        : (((borg.trait[BI_CLEVEL] - 10) / 4) * 3) + 10)))
            break;
        dam = -999;
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            break;
        borg_fear_mon_spell = false;
        p1 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        /* Make certain monsters appear to have more danger so the borg is more
         * likely to use this attack */
        if (kill->afraid && borg.trait[BI_CLEVEL] <= 10)
            p1 = p1 + 20;
        borg_fear_mon_spell = true;
        p2 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        borg_fear_mon_spell = false;
        dam                 = (p1 - p2);
        break;

    case BORG_ATTACK_OLD_SLOW:
        dam = 0;
        if (kill->speed < r_ptr->speed - 5)
            break;
        if (kill->confused)
            break;
        if (!kill->awake)
            break;
        if ((kill->level
                > (borg.trait[BI_CLEVEL] < 13
                        ? 10
                        : (((borg.trait[BI_CLEVEL] - 10) / 4) * 3) + 10)))
            break;
        dam = -999;
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            break;
        borg_slow_spell = false;
        p1 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        /* Make certain monsters appear to have more danger so the borg is more
         * likely to use this attack */
        if (kill->afraid && borg.trait[BI_CLEVEL] <= 10)
            p1 = p1 + 20;
        borg_slow_spell = true;
        p2 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        borg_slow_spell = false;
        dam             = (p1 - p2);
        break;

    case BORG_ATTACK_OLD_SLEEP:
    case BORG_ATTACK_SLEEP_EVIL:
        dam = 0;
        if (rf_has(r_ptr->flags, RF_NO_SLEEP))
            break;
        if (!rf_has(r_ptr->flags, RF_EVIL) && typ == BORG_ATTACK_SLEEP_EVIL)
            break;
        if (kill->speed < r_ptr->speed - 5)
            break;
        if (kill->confused)
            break;
        if (!kill->awake)
            break;
        if ((kill->level
                > (borg.trait[BI_CLEVEL] < 13
                        ? 10
                        : (((borg.trait[BI_CLEVEL] - 10) / 4) * 3) + 10)))
            break;
        dam = -999;
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            break;
        borg_sleep_spell = false;
        p1 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        /* Make certain monsters appear to have more danger so the borg is more
         * likely to use this attack */
        if (kill->afraid && borg.trait[BI_CLEVEL] <= 10)
            p1 = p1 + 20;
        borg_sleep_spell = true;
        p2 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
        borg_sleep_spell = false;
        dam              = (p1 - p2);
        break;

    case BORG_ATTACK_OLD_POLY:
        dam = 0;
        if ((kill->level
                > (borg.trait[BI_CLEVEL] < 13
                        ? 10
                        : (((borg.trait[BI_CLEVEL] - 10) / 4) * 3) + 10)))
            break;
        dam = -999;
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            break;
        dam = borg_danger_one_kill(borg.c.y, borg.c.x, 2, i, true, true);
        /* don't bother unless he is a scary monster */
        if ((dam < avoidance * 2) && !kill->afraid)
            dam = 0;
        break;

    case BORG_ATTACK_TURN_UNDEAD:
        if (rf_has(r_ptr->flags, RF_UNDEAD)) {
            dam = 0;
            if (kill->confused)
                break;
            if (kill->speed < r_ptr->speed - 5)
                break;
            if (!kill->awake)
                break;
            if (kill->level > borg.trait[BI_CLEVEL] - 5)
                break;
            borg_fear_mon_spell = false;
            p1 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
            borg_fear_mon_spell = true;
            p2 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
            borg_fear_mon_spell = false;
            dam                 = (p1 - p2);
        } else {
            dam = 0;
        }
        break;

    /* Banishment-- cast when in extreme danger (checked in borg_defense). */
    case BORG_ATTACK_AWAY_EVIL:
        if (rf_has(r_ptr->flags, RF_EVIL)) {
            /* try not teleport away uniques. */
            if (rf_has(r_ptr->flags, RF_UNIQUE)) {
                /* Banish ones with escorts */
                if (r_ptr->friends || r_ptr->friends_base) {
                    dam = 0;
                } else {
                    /* try not Banish non escorted uniques */
                    dam = -500;
                }

            } else {
                /* damage is the danger of the baddie */
                dam = borg_danger_one_kill(
                    borg.c.y, borg.c.x, 1, i, true, true);
            }
        } else {
            dam = 0;
        }
        break;

    case BORG_ATTACK_TAP_UNLIFE:
        /* for now ignore the gain in sp */
        if (!(rf_has(r_ptr->flags, RF_UNDEAD)))
            dam = 0;
        else {
            int sp_drain = borg.trait[BI_CURSP] - borg.trait[BI_CURSP];
            if (sp_drain < kill->power)
                dam = kill->power - sp_drain;
        }
        break;
    }

    /* use Missiles on certain types of monsters */
    if ((borg.trait[BI_CDEPTH] >= 1)
        && (borg_danger_one_kill(kill->pos.y, kill->pos.x, 1, i, true, true)
                > avoidance * 2 / 10
            || ((r_ptr->friends || r_ptr->friends_base) /* monster has friends*/
                && kill->level >= borg.trait[BI_CLEVEL] - 5 /* close levels */)
            || kill->ranged_attack /* monster has a ranged attack */
            || rf_has(r_ptr->flags, RF_UNIQUE)
            || rf_has(r_ptr->flags, RF_MULTIPLY) || gold_eater
            || /* Monster can steal gold */
            rf_has(r_ptr->flags, RF_NEVER_MOVE) /* monster never moves */
            || borg.trait[BI_CLEVEL] <= 20 /* stil very weak */)) {
        borg_use_missile = true;
    }

    /* Return Damage as pure danger of the monster */
    if (typ == BORG_ATTACK_AWAY_ALL || typ == BORG_ATTACK_AWAY_EVIL
        || typ == BORG_ATTACK_AWAY_ALL_MORGOTH)
        return dam;

    /* Limit damage to twice maximal hitpoints */
    if (dam > kill->power * 2 && !rf_has(r_ptr->flags, RF_UNIQUE))
        dam = kill->power * 2;

    /* give a small bonus for whacking a unique */
    /* this should be just enough to give preference to whacking uniques */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg.trait[BI_CDEPTH] >= 1)
        dam = (dam * 3);

    /* Hack -- ignore Maggot until later.  Player will chase Maggot
     * down all across the screen waking up all the monsters.  Then
     * he is stuck in a compromised situation.
     */
    if ((rf_has(r_ptr->flags, RF_UNIQUE)) && borg.trait[BI_CDEPTH] == 0) {
        dam = dam * 2 / 3;

        /* Don't hunt maggot until later */
        if (borg.trait[BI_CLEVEL] < 5)
            dam = 0;
    }

    /* give a small bonus for whacking a breeder */
    if (rf_has(r_ptr->flags, RF_MULTIPLY))
        dam = (dam * 3 / 2);

    /* Enhance the perceived damage to summoner in order to influence the
     * choice of targets.
     */
    if ((rsf_has(r_ptr->spell_flags, RSF_S_KIN))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER))
        || (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS))
        || (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL))
        || (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HOUND))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA))
        || (rsf_has(r_ptr->spell_flags, RSF_S_AINU))
        || (rsf_has(r_ptr->spell_flags, RSF_S_DEMON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD))
        || (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HI_DRAGON))
        || (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD))
        || (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH))
        || (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE)))
        dam += ((dam * 3) / 2);

    /*
     * Apply massive damage bonus to Questor monsters to
     * encourage borg to strike them.
     */
    if (rf_has(r_ptr->flags, RF_QUESTOR))
        dam += (dam * 9);

    /*  Try to conserve missiles.
     */
    if (typ == BORG_ATTACK_ARROW) {
        if (!borg_use_missile)
            /* set damage to zero, force borg to melee attack */
            dam = 0;
    }

    /* Damage */
    return dam;
}

/*
 * Simulate / Invoke the launching of a bolt at a monster
 */
static int borg_launch_bolt_aux_hack(int i, int dam, int typ, int ammo_location)
{
    int d, p2, p1, x, y;
    int o_y     = 0;
    int o_x     = 0;
    int walls   = 0;
    int unknown = 0;

    borg_grid *ag;

    borg_kill *kill;

    struct monster_race *r_ptr;

    /* Monster */
    kill = &borg_kills[i];

    /* monster race */
    r_ptr = &r_info[kill->r_idx];

    /* Skip dead monsters */
    if (!kill->r_idx)
        return 0;

    /* Require current knowledge */
    if (kill->when < borg_t - 2)
        return 0;

    /* Acquire location */
    x = kill->pos.x;
    y = kill->pos.y;

    /* Acquire the grid */
    ag = &borg_grids[y][x];

    /* Never shoot walls/doors */
    if (!borg_cave_floor_grid(ag))
        return 0;

    /* dont shoot at ghosts if not on known floor grid */
    if ((rf_has(r_ptr->flags, RF_PASS_WALL))
        && (ag->feat != FEAT_FLOOR && ag->feat != FEAT_OPEN
            && ag->feat != FEAT_BROKEN && !ag->trap))
        return 0;

    /* dont shoot at ghosts in walls, not perfect */
    if (rf_has(r_ptr->flags, RF_PASS_WALL)) {
        /* if 2 walls and 1 unknown skip this monster */
        /* Acquire location */
        x = kill->pos.x;
        y = kill->pos.y;

        /* Get grid */
        for (o_x = -1; o_x <= 1; o_x++) {
            for (o_y = -1; o_y <= 1; o_y++) {
                /* Acquire location */
                x  = kill->pos.x + o_x;
                y  = kill->pos.y + o_y;

                ag = &borg_grids[y][x];

                if (ag->feat >= FEAT_MAGMA && ag->feat <= FEAT_PERM)
                    walls++;
                if (ag->feat == FEAT_NONE)
                    unknown++;
            }
        }
        /* Is the ghost likely in a wall? */
        if (walls >= 2 && unknown >= 1)
            return 0;
    }

    /* Calculate damage */
    d = borg_launch_damage_one(i, dam, typ, ammo_location);

    /* Return Damage, on Teleport Other, true damage is
     * calculated elsewhere */
    if (typ == BORG_ATTACK_AWAY_ALL || typ == BORG_ATTACK_AWAY_ALL_MORGOTH)
        return d;

    /* Return Damage as pure danger of the monster */
    if (typ == BORG_ATTACK_AWAY_EVIL)
        return d;

    /* Return 0 if the true damage (w/o the danger bonus) is 0 */
    if (d <= 0)
        return d;

    /* Calculate danger */
    p2 = borg_danger_one_kill(y, x, 1, i, true, false);

    /* Hack -- avoid waking most "hard" sleeping monsters */
    if (!kill->awake && (p2 > avoidance / 2) && (d < kill->power)
        && !borg.munchkin_mode) {
        return (-999);
    }

    /* Hack -- ignore sleeping town monsters */
    if (!borg.trait[BI_CDEPTH] && !kill->awake) {
        return 0;
    }

    /* Hack -- ignore nonthreatening town monsters when low level */
    if (!borg.trait[BI_CDEPTH] && borg.trait[BI_CLEVEL] < 3
        /* && monster_is_nonthreatening_test */) {
        /* Nothing yet */
    }

    /* Calculate "danger" to player */
    p1 = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, false);

    /* Extra "bonus" if attack kills */
    if (d >= kill->power)
        d = 2 * d;

    /* Add in dangers */
    d = d + p1;

    /* Result */
    return d;
}


/*
 * Determine the penalty for destroying stuff by launching a spell at it
 */
static int borg_launch_destroy_stuff(borg_grid *ag, int typ)
{
    struct borg_take *take = &borg_takes[ag->take];
    struct object_kind *k_ptr = take->kind;

    switch (typ) {
    case BORG_ATTACK_ACID:
    {
        /* rings/boots cost extra (might be speed!) */
        if (k_ptr->tval == TV_BOOTS && !k_ptr->aware) {
            return 20;
        }
        break;
    }
    case BORG_ATTACK_ELEC:
    {
        /* rings/boots cost extra (might be speed!) */
        if (k_ptr->tval == TV_RING && !k_ptr->aware) {
            return 20;
        }
        if (k_ptr->tval == TV_RING
            && k_ptr->sval == sv_ring_speed) {
            return 2000;
        }
        break;
    }

    case BORG_ATTACK_FIRE:
    {
        /* rings/boots cost extra (might be speed!) */
        if (k_ptr->tval == TV_BOOTS && !k_ptr->aware) {
            return 20;
        }
        break;
    }
    case BORG_ATTACK_COLD:
    {
        if (k_ptr->tval == TV_POTION) {
            /* Extra penalty for cool potions */
            if (!k_ptr->aware || k_ptr->sval == sv_potion_healing
                || k_ptr->sval == sv_potion_star_healing
                || k_ptr->sval == sv_potion_life
                || (k_ptr->sval == sv_potion_inc_str
                    && borg.need_statgain[STAT_STR])
                || (k_ptr->sval == sv_potion_inc_int
                    && borg.need_statgain[STAT_INT])
                || (k_ptr->sval == sv_potion_inc_wis
                    && borg.need_statgain[STAT_WIS])
                || (k_ptr->sval == sv_potion_inc_dex
                    && borg.need_statgain[STAT_DEX])
                || (k_ptr->sval == sv_potion_inc_con
                    && borg.need_statgain[STAT_CON]))
                return 2000;

            return 20;

        }
        break;
    }
    case BORG_ATTACK_MANA:
    {
        /* Used against uniques, allow the stuff to burn */
        break;
    }
    }

    return 0;
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
static int borg_launch_bolt_at_location(
    int y, int x, int rad, int dam, int typ, int max, int ammo_location)
{
    int ry, rx;

    int x1, y1;
    int x2, y2;

    int dist;

    int r, n;

    borg_grid           *ag;
    struct monster_race *r_ptr;
    borg_kill           *kill;

    int q_x, q_y;

    /* Extract panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();

    /* Reset damage */
    n = 0;

    /* Initial location */
    x1 = borg.c.x;
    y1 = borg.c.y;

    /* Final location */
    x2 = x;
    y2 = y;

    /* Bounds Check */
    if (!square_in_bounds_fully(cave, loc(x, y)))
        return 0;

    /* Start over */
    x = x1;
    y = y1;

    /* Get the grid of the targetted monster */
    ag = &borg_grids[y2][x2];
    kill = &borg_kills[ag->kill];
    r_ptr = &r_info[kill->r_idx];

    /* Simulate the spell/missile path */
    for (dist = 1; dist < max; dist++) {
        /* Calculate the new location */
        borg_inc_motion(&y, &x, y1, x1, y2, x2);

        /* Bounds Check */
        if (!square_in_bounds_fully(cave, loc(x, y)))
            break;

        /* Get the grid of the pathway */
        ag = &borg_grids[y][x];

        /* Stop at walls */
        /* note if beam, this is the end of the beam */
        /* dispel spells act like beams (sort of) */
        if (!borg_cave_floor_grid(ag) || ag->feat == FEAT_PASS_RUBBLE) {
            if (rad != -1 && rad != 10)
                return 0;
            else
                return n;
        }

        /* Collect damage (bolts/beams) */
        if (rad <= 0 || rad == 10)
            n += borg_launch_bolt_aux_hack(ag->kill, dam, typ, ammo_location);

        /* Check for arrival at "final target" */
        /* except beams, which keep going. */
        if ((rad != -1 && rad != 10) && ((x == x2) && (y == y2)))
            break;

        /* Stop bolts at monsters  */
        if (!rad && ag->kill)
            return n;

        /* The missile path can be complicated.  There are several checks
         * which need to be made.  First we assume that we targeting
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
         * will attempt the shot and listen for the 'ouch' response
         * to know that the clear.  If no 'Ouch' is heard, then the
         * borg will assume there is a wall in the way.  Exception to
         * this is with arrows.  Arrows can miss the target or fall
         * fall short, in which case no 'ouch' is heard.  So the borg
         * allowed to miss two shots with arrows/bolts/thrown objects.
         */

        /* dont do the check if esp */
        if (!borg.trait[BI_ESP]) {
            /* Check the missile path--no Infra, no HAS_LIGHT */
            if ((borg.trait[BI_INFRA] <= 0) && !(r_ptr->light > 0)) {
                /* Stop at unknown grids (see above) */
                /* note if beam, dispel, this is the end of the beam */
                if (ag->feat == FEAT_NONE) {
                    if (rad != -1 && rad != 10)
                        return 0;
                    else
                        return n;
                }
            } 

            /* Stop at unseen walls */
            /* We just shot and missed, this is our next shot */
            if (successful_target < 0) {
                /* When throwing things, it is common to just 'miss' */
                /* Skip only one round in this case */
                if (successful_target <= -12)
                    successful_target = 0;
                if (rad != -1 && rad != 10)
                    return 0;
                else
                    return n;
            }
        } else /* I do have ESP */
        {
            /* if this area has been magic mapped,
             * ok to shoot in the dark
             */
            if (!borg_detect_wall[q_y + 0][q_x + 0]
                && !borg_detect_wall[q_y + 0][q_x + 1]
                && !borg_detect_wall[q_y + 1][q_x + 0]
                && !borg_detect_wall[q_y + 1][q_x + 1]
                && borg_fear_region[borg.c.y / 11][borg.c.x / 11]
                < avoidance / 20) {

                /* Stop at unknown grids (see above) */
                /* note if beam, dispel, this is the end of the beam */
                if (ag->feat == FEAT_NONE) {
                    if (rad != -1 && rad != 10)
                        return 0;
                    else
                        return n;
                }
                /* Stop at unseen walls */
                /* We just shot and missed, this is our next shot */
                if (successful_target < 0) {
                    /* When throwing things, it is common to just 'miss' */
                    /* Skip only one round in this case */
                    if (successful_target <= -12)
                        successful_target = 0;
                    if (rad != -1 && rad != 10)
                        return 0;
                    else
                        return n;
                }
            }

            /* Stop at unseen walls */
            /* We just shot and missed, this is our next shot */
            if (successful_target < 0) {
                /* When throwing things, it is common to just 'miss' */
                /* Skip only one round in this case */
                if (successful_target <= -12)
                    successful_target = 0;

                if (rad != -1 && rad != 10)
                    return 0;
                else
                    return n;
            }
        }
    }

    /* Bolt/Beam attack */
    if (rad <= 0)
        return n;

    /* Excessive distance */
    if (dist >= max)
        return 0;

    /* Check monsters and objects in blast radius */
    for (ry = y2 - rad; ry < y2 + rad; ry++) {
        for (rx = x2 - rad; rx < x2 + rad; rx++) {

            /* Bounds check */
            if (!square_in_bounds(cave, loc(rx, ry)))
                continue;

            /* Get the grid */
            ag = &borg_grids[ry][rx];

            /* Check distance */
            r = borg_distance(y2, x2, ry, rx);

            /* Maximal distance */
            if (r > rad)
                continue;

            /* Never pass through walls*/
            if (!borg_los(y2, x2, ry, rx))
                continue;

            /*  dispel spells should hurt the same no matter the rad: make r= y
             * and x */
            if (rad == 10)
                r = 0;

            /* Collect damage, lowered by distance */
            n += borg_launch_bolt_aux_hack(
                ag->kill, dam / (r + 1), typ, ammo_location);

            /* probable damage int was just changed by b_l_b_a_h*/

            /* check destroyed stuff. */
            if (ag->take && borg_takes[ag->take].kind)
                n -= borg_launch_destroy_stuff(ag, typ);
        }
    }

    /* Result */
    return n;
}

/*
 * Simulate/Apply the optimal result of launching a beam/bolt/ball
 *
 * Note that "beams" have a "rad" of "-1", "bolts" have a "rad" of "0",
 * and "balls" have a "rad" of "2" or "3", depending on "blast radius".
 *  dispel spells have a rad  of 10
 */
int borg_launch_bolt(int rad, int dam, int typ, int max, int ammo_location)
{
    int i     = 0;
    int b_i   = -1;
    int n     = 0;
    int b_n   = -1;
    int b_o_y = 0, b_o_x = 0;
    int o_y = 0, o_x = 0;
    int d, b_d       = z_info->max_range;

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
    for (i = 0; i < borg_temp_n; i++) {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Consider each adjacent spot to and on top of the monster */
        for (o_x = -1; o_x <= 1; o_x++) {
            for (o_y = -1; o_y <= 1; o_y++) {
                /* Acquire location */
                x = borg_temp_x[i] + o_x;
                y = borg_temp_y[i] + o_y;

                /* Reset Teleport Other variables */
                borg_tp_other_n = 0;
                n               = 0;

                /* Bounds check */
                if (!square_in_bounds(cave, loc(x, y)))
                    continue;

                /* Remember how far away the monster is */
                d = distance(borg.c, loc(borg_temp_x[i], borg_temp_y[i]));

                /* Skip certain types of Offset attacks */
                if ((x != borg_temp_x[i] || y != borg_temp_y[i])
                    && typ == BORG_ATTACK_AWAY_ALL)
                    continue;

                /* Skip places that are out of range */
                if (distance(borg.c, loc(x, y)) > max)
                    continue;

                /* Consider it if its a ball spell or right on top of it */
                if ((rad >= 2 && borg_grids[y][x].feat != FEAT_NONE)
                    || (y == borg_temp_y[i] && x == borg_temp_x[i]))
                    n = borg_launch_bolt_at_location(
                        y, x, rad, dam, typ, max, ammo_location);

                /* Teleport Other is now considered */
                if (typ == BORG_ATTACK_AWAY_ALL && n > 0) {
                    /* Consider danger with certain monsters removed
                     * from the danger check.  They were removed from the list
                     * of considered monsters (borg_tp_other array)
                     */
                    n = borg_danger(borg.c.y, borg.c.x, 1, true, false);

                    /* Skip Offsets that do only 1 damage */
                    if (n == 1)
                        n = -10;
                }

                /* Reset Teleport Other variables */
                borg_tp_other_n = 0;

                /* Skip useless attacks */
                if (n <= 0)
                    continue;

                /* The game forbids targetting the outside walls */
                if (x == 0 || y == 0 || x == DUNGEON_WID - 1
                    || y == DUNGEON_HGT - 1)
                    continue;

                /* Collect best attack */
                if ((b_i >= 0) && (n < b_n))
                    continue;

                /* Skip attacking farther monster if rewards are equal. */
                if (n == b_n && d > b_d)
                    continue;

                /* Track it */
                b_i   = i;
                b_n   = n;
                b_o_y = o_y;
                b_o_x = o_x;
                b_d   = d;
            }
        }
    }
    if (b_i == -1)
        return b_n;

    /* Reset Teleport Other variables */
    borg_tp_other_n = 0;

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Save the location */
    borg.goal.g.x = borg_temp_x[b_i] + b_o_x;
    borg.goal.g.y = borg_temp_y[b_i] + b_o_y;

    /* Target the location */
    (void)borg_target(borg.goal.g);

    /* Result */
    return b_n;
}

/*
 * Determine the "reward" of launching an arc/spray/cone at a location
 * 
 * This code is copied from borg_launch_bolt_at_location
 * both need to be optimized for shared code !FIX
 *
 */
static int borg_launch_arc_at_location(
    int y, int x, int degrees, int dam, int typ, int max)
{
    int ry, rx;

    int x1, y1;
    int x2, y2;

    int dist;

    int r, n;

    borg_grid *ag;
    struct monster_race *r_ptr;
    borg_kill *kill;

    int q_x, q_y;

    struct loc path_grids[256];

    /* Extract panel */
    q_x = w_x / borg_panel_wid();
    q_y = w_y / borg_panel_hgt();

    /* Reset damage */
    n = 0;

    /* Initial location */
    x1 = borg.c.x;
    y1 = borg.c.y;

    /* Final location */
    x2 = x;
    y2 = y;

    /* Bounds Check */
    if (!square_in_bounds_fully(cave, loc(x, y)))
        return 0;

    /* Start over */
    x = x1;
    y = y1;

    /* Get the grid of the targetted monster */
    ag = &borg_grids[y2][x2];
    kill = &borg_kills[ag->kill];
    r_ptr = &r_info[kill->r_idx];

    /* starting square is always good */
    path_grids[0].x = x;
    path_grids[0].y = y;

    /* Simulate the spell/missile path */
    for (dist = 1; dist < max; dist++) {

        /* Calculate the new location */
        borg_inc_motion(&y, &x, y1, x1, y2, x2);

        /* Bounds Check */
        if (!square_in_bounds_fully(cave, loc(x, y)))
            break;

        ag = &borg_grids[y][x];

        /* Stop at walls */
        /* note if beam, this is the end of the beam */
        /* dispel spells act like beams (sort of) */
        if (!borg_cave_floor_grid(ag) || ag->feat == FEAT_PASS_RUBBLE) {
            break;
        }

        path_grids[dist].x = x;
        path_grids[dist].y = y;

        /* Check for arrival at "final target" */
        /* except beams, which keep going. */
        if ((x == x2) && (y == y2))
            break;

        /* check unmapped squares */
        if (ag->feat == FEAT_NONE) {

            /* Check if we have ESP */
            if (borg.trait[BI_ESP]) {
                /* no Infra, no HAS_LIGHT */
                if ((borg.trait[BI_INFRA] <= 0) && !(r_ptr->light > 0))
                    break;
            } else /* no ESP */ {
                /* if this area has been magic mapped,
                 * ok to shoot in the dark
                 */
                if (!borg_detect_wall[q_y + 0][q_x + 0]
                    && !borg_detect_wall[q_y + 0][q_x + 1]
                    && !borg_detect_wall[q_y + 1][q_x + 0]
                    && !borg_detect_wall[q_y + 1][q_x + 1]
                    && borg_fear_region[borg.c.y / 11][borg.c.x / 11]
                    < avoidance / 20) {
                    break;
                }
            }

            /* Stop if we missed previously */
            if (successful_target < 0) {
                /* reset the "miss" so only one shot is skipped  */
                if (successful_target <= -12)
                    successful_target = 0;
                break;
            }
        }
    }

    if (dist < 21)
        dist = dist - 1;
    else
        dist = 20;

    /* Use angle comparison to delineate an arc. */
    int n1x, n1y, n2y, n2x, tmp, rotate, diff;

    /* reorient arc beam */
    n1y = path_grids[dist].y - y1 + 20;
    n1x = path_grids[dist].x - x1 + 20;

    /* Check monsters and objects in blast radius and on the arc */
    /* for an arc, blast radius starts at player */
    for (ry = y1 - max; ry < y1 + max; ry++) {
        for (rx = x1 - max; rx < x1 + max; rx++) {

            /* Bounds check */
            if (!square_in_bounds(cave, loc(rx, ry)))
                continue;

            /* Get the grid */
            ag = &borg_grids[ry][rx];

            /* Check distance */
            r = borg_distance(y1, x1, ry, rx);

            /* Maximal distance */
            if (r > max)
                continue;

            /* Never pass through walls*/
            if (!borg_los(y1, x1, ry, rx))
                continue;

            /* check on angle */
            /* Reorient current grid for table access. */
            n2y = y - y1 + 20;
            n2x = x - x1 + 20;

            /* Find the angular difference (/2) between the lines to
                * the end of the arc's center-line and to the current grid.
                */
            rotate = 90 - get_angle_to_grid[n1y][n1x];
            tmp = ABS(get_angle_to_grid[n2y][n2x] + rotate) % 180;
            diff = ABS(90 - tmp);

            /* If difference is greater then that allowed, skip it,
                * unless it's on the target path */
            if (diff >= (degrees + 6) / 4)
                continue;

            if (ag->kill)
                /* Collect damage, lowered by distance */
                n += borg_launch_bolt_aux_hack(ag->kill, dam / (r + 1), typ, 0);

            if (ag->take && borg_takes[ag->take].kind) 
                n -= borg_launch_destroy_stuff(ag, typ);
        }
    }

    /* Result */
    return n;
}

/*
 * Simulate/Apply the optimal result of launching an arc/cone/spray
 */
static int borg_launch_arc(int degrees, int dam, int typ, int max)
{
    int i = 0;
    int b_i = -1;
    int n = 0;
    int b_n = -1;

    int b_o_y = 0, b_o_x = 0;
    int o_y = 0, o_x = 0;
    int d, b_d = z_info->max_range;

    if (max > 20)
        max = 20;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++) {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* reset result */
        n = 0;

        /* Remember how far away the monster is */
        d = distance(borg.c, loc(borg_temp_x[i], borg_temp_y[i]));

        /* Skip places that are out of range */
        if (d > max)
            continue;

        /* Consider it if its a ball spell or right on top of it */
        n = borg_launch_arc_at_location(y, x, degrees, dam, typ, max);

        /* Skip useless attacks */
        if (n <= 0)
            continue;

        /* Collect best attack */
        if ((b_i >= 0) && (n < b_n))
            continue;

        /* Skip attacking farther monster if rewards are equal. */
        if (n == b_n && d > b_d)
            continue;

        /* Track it */
        b_i = i;
        b_n = n;
        b_o_y = o_y;
        b_o_x = o_x;
        b_d = d;
    }
    if (b_i == -1)
        return b_n;

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Save the location */
    borg.goal.g.x = borg_temp_x[b_i] + b_o_x;
    borg.goal.g.y = borg_temp_y[b_i] + b_o_y;

    /* Target the location */
    (void)borg_target(borg.goal.g);

    /* Result */
    return b_n;
}

/*
 * Simulate/Apply the optimal result of launching a missile
 */
int borg_attack_aux_launch(void)
{
    int n, b_n = 0;

    int k, b_k = -1;
    int d = -1;
    int v, b_v = -1;

    borg_item *bow = &borg_items[INVEN_BOW];

    /* skip if we don't have a bow */
    if (!bow || bow->iqty == 0)
        return 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Scan the quiver */
    for (k = QUIVER_START; k < QUIVER_END; k++) {

        borg_item *item = &borg_items[k];

        /* Skip empty items */
        if (!item->iqty)
            break;

        /* Skip missiles that don't match the bow */
        if (item->tval != borg.trait[BI_AMMO_TVAL])
            continue;

        /* Skip worthless missiles */
        if (item->value <= 0)
            continue;

        /* Determine average damage */
        d = (item->dd * (item->ds + 1) / 2);
        d = d + item->to_d + bow->to_d;
        d = d * borg.trait[BI_AMMO_POWER] * borg.trait[BI_SHOTS];

        v = item->value;

        /* Boost the perceived damage on unID'd ones so he can get a quick
         * pseudoID on it */
        if (borg_item_note_needs_id(item))
            d = d * 99;

        /* Paranoia */
        if (d <= 0)
            continue;

        /* Choose optimal target of bolt */
        n = borg_launch_bolt(
            0, d, BORG_ATTACK_ARROW, 6 + 2 * borg.trait[BI_AMMO_POWER], k);

        /* if two attacks are equal, pick the cheaper ammo */
        if (n == b_n && v >= b_v)
            continue;

        if (n >= b_n) {
            b_n = n;
            b_v = v;
            b_k = k;
        }
    }

    /* Nothing to use */
    if (b_n < 0)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return b_n;

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
    return b_n;
}

/* Attempt to rest on the grid to allow the monster to approach me.
 * Make sure the monster does not have a ranged attack and that I am
 * inclined to attack him.
 */
static int borg_attack_aux_rest(void)
{
    int  i;
    bool resting_is_good = false;

    int my_danger        = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        int x9          = kill->pos.x;
        int y9          = kill->pos.y;
        int ax, ay, d;

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Distance components */
        ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
        ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Minimal and maximal distance */
        if (d != 2)
            continue;

        /* Ranged Attacks, don't rest. */
        if (kill->ranged_attack)
            continue;

        /* Skip the sleeping ones */
        if (!kill->awake)
            continue;

        /* need to have seen it recently */
        if (borg_t - kill->when > 10)
            continue;

        /* Skip monsters that dont chase */
        if (rf_has(r_info[kill->r_idx].flags, RF_NEVER_MOVE))
            continue;

        /* Monster better not be faster than me */
        if (kill->speed - borg.trait[BI_SPEED] >= 5)
            continue;

        /* Should be flowing towards the monster */
        if (borg.goal.type != GOAL_KILL || borg_flow_y[0] != kill->pos.y)
            continue;

        /* Cant have an obstacle between us */
        if (!borg_los(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x))
            continue;

        /* Might be a little dangerous to just wait here */
        if (my_danger > borg.trait[BI_CURHP])
            continue;

        /* Should be a good idea to wait for monster here. */
        resting_is_good = true;
    }

    /* Not a good idea */
    if (resting_is_good == false)
        return 0;

    /* Return some value for this rest */
    if (borg_simulate)
        return 1;

    /* Rest */
    borg_keypress(',');
    borg_note(
        format("# Resting on grid (%d, %d), waiting for monster to approach.",
            borg.c.y, borg.c.x));

    /* All done */
    return 1;
}

/* look for a throwable item */
static bool borg_has_throwable(void)
{
    int i;
    for (i = 0; i < QUIVER_END; i++) {
        /* it will show wield in the list */
        /* but not if that is the only thing */
        if (i == INVEN_WIELD)
            continue;

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
    for (k = 0; k < z_info->pack_size; k++) {
        borg_item *item = &borg_items[k];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip my spell/prayer book */
        if (obj_kind_can_browse(&k_info[item->kind]))
            continue;

        /* Skip "equipment" items (not ammo) */
        if (borg_wield_slot(item) >= 0)
            continue;

        /* Determine average damage from object */
        d = (k_info[item->kind].dd * (k_info[item->kind].ds + 1) / 2);

        /* Skip things that are worth money unless they do a lot of damage */
        if (item->value > 100 && d < 5)
            continue;

        /* Skip useless stuff */
        if (d <= 0)
            continue;

        /* Hack -- Save Heals and cool stuff */
        if (item->tval == TV_POTION)
            continue;

        /* Hack -- Save last flasks for fuel, if needed */
        if (item->tval == TV_FLASK
            && (borg.trait[BI_AFUEL] <= 1 && !borg_fighting_unique))
            continue;

        /* Dont throw wands or rods */
        if (item->tval == TV_WAND || item->tval == TV_ROD)
            continue;

        /* Ignore worse damage */
        if ((b_k >= 0) && (d <= b_d))
            continue;

        /* Track */
        b_k = k;
        b_d = d;

        /* Extract a "distance multiplier" */
        mul = 10;

        /* Enforce a minimum "weight" of one pound */
        div = ((item->weight > 10) ? item->weight : 10);

        /* Hack -- Distance -- Reward strength, penalize weight */
        b_r = (adj_str_blow[borg.stat_ind[STAT_STR]] + 20) * mul / div;

        /* Max distance of 10 */
        if (b_r > 10)
            b_r = 10;
    }

    /* Nothing to use */
    if (b_k < 0)
        return 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(
        0, b_d, BORG_ATTACK_ARROW, 6 + 2 * borg.trait[BI_AMMO_POWER], b_k);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Do it */
    borg_note(format("# Throwing painful object '%s'", borg_items[b_k].desc));

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
    return b_n;
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack spell
 *
 * Take into account the failure rate of spells/objects/etc.  XXX XXX XXX
 */
int borg_attack_aux_spell_bolt(
    const enum borg_spells spell, int rad, int dam, int typ, int max_range, bool is_arc)
{
    int b_n;
    int penalty = 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate
        && ((borg.trait[BI_CLASS] != CLASS_MAGE
                && borg.trait[BI_CLASS] != CLASS_NECROMANCER)
            && borg.trait[BI_CLEVEL] <= 2)
        && (randint0(100) < 1))
        return 0;

    /* Not if money scumming in town */
    if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] && borg.trait[BI_CDEPTH] == 0)
        return 0;

    /* Not if low on food */
    if (borg.trait[BI_FOOD] == 0
        && (borg.trait[BI_ISWEAK]
            && (borg_spell_legal(REMOVE_HUNGER)
                || borg_spell_legal(HERBAL_CURING))))
        return 0;

    /* Require ability (right now) */
    if (!borg_spell_okay_fail(spell, (borg_fighting_unique ? 40 : 25)))
        return 0;

    /* Choose optimal location */
    if (is_arc)
        b_n = borg_launch_arc(rad, dam, typ, max_range);
    else
        b_n = borg_launch_bolt(rad, dam, typ, max_range, 0);

    enum borg_spells primary_spell_for_class = MAGIC_MISSILE;
    switch (borg.trait[BI_CLASS]) {
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

    /* weak mages need that spell, they don't get penalized */
    /* weak == those that can't teleport reliably anyway */
    if (spell == primary_spell_for_class
        && (!borg_spell_legal_fail(TELEPORT_SELF, 15)
            || borg.trait[BI_MAXCLEVEL] <= 30)) {
        if (borg_simulate)
            return b_n;
    }

    /* Penalize mana usage except on MM */
    int spell_power = borg_get_spell_power(spell);
    if (spell != primary_spell_for_class) {
        /* Standard penalty */
        b_n = b_n - spell_power;

        /* Extra penalty if the cost far outweighs the damage */
        if (borg.trait[BI_MAXSP] < 50 && spell_power > b_n)
            b_n = b_n - spell_power;

        /* Penalize use of reserve mana */
        if (borg.trait[BI_CURSP] - spell_power < borg.trait[BI_MAXSP] / 2)
            b_n = b_n - (spell_power * 3);

        /* Penalize use of deep reserve mana */
        if (borg.trait[BI_CURSP] - spell_power < borg.trait[BI_MAXSP] / 3)
            b_n = b_n - (spell_power * 5);
    }

    /* Really penalize use of mana needed for final teleport */
    switch (borg.trait[BI_CLASS]) {
    case CLASS_MAGE:
        penalty = 6;
        break;
    case CLASS_RANGER:
        penalty = 22;
        break;
    case CLASS_ROGUE:
        penalty = 20;
        break;
    case CLASS_PRIEST:
        penalty = 8;
        break;
    case CLASS_PALADIN:
        penalty = 20;
        break;
    case CLASS_NECROMANCER:
        penalty = 10;
        break;
    }
    if ((borg.trait[BI_MAXSP] > 30)
        && (borg.trait[BI_CURSP] - spell_power < penalty))
        b_n = b_n - (spell_power * 750);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Cast the spell */
    (void)borg_spell(spell);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return b_n;
}

/* This routine is the same as the one above only in an emergency case.
 * The borg will enter negative mana casting this
 */
static int borg_attack_aux_spell_bolt_reserve(
    const enum borg_spells spell, int rad, int dam, int typ, int max_range)
{
    int b_n;
    int i;

    int x9, y9, ax, ay, d;
    int near_monsters = 0;

    /* Fake our Mana */
    int sv_mana = borg.trait[BI_CURSP];

    /* Only Weak guys should try this */
    if (borg.trait[BI_CLEVEL] >= 15)
        return 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Not if low on food */
    if (borg.trait[BI_FOOD] == 0
        && (borg.trait[BI_ISWEAK] && borg_spell_legal(REMOVE_HUNGER)))
        return 0;

    /* Must not have enough mana right now */
    if (borg_spell_okay_fail(spell, 25))
        return 0;

    /* Must be dangerous */
    if (borg_danger(borg.c.y, borg.c.x, 1, true, false) < avoidance * 2)
        return 0;

    /* Find the monster */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* check the location */
        x9 = kill->pos.x;
        y9 = kill->pos.y;

        /* Distance components */
        ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
        ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Count the number of close monsters
         * There should only be one close monster.
         * We do not want to risk fainting.
         */
        if (d < 7)
            near_monsters++;

        /* If it has too many hp to be taken out with this */
        /* spell, don't bother trying */
        /* NOTE: the +4 is because the damage is toned down
                 as an 'average damage' */
        if (kill->power > (dam + 4))
            return 0;

        /* Do not use it in town */
        if (borg.trait[BI_CDEPTH] == 0)
            return 0;

        break;
    }

    /* Should only be 1 near monster */
    if (near_monsters > 1)
        return 0;

    /* Require ability (with faked mana) */
    borg.trait[BI_CURSP] = borg.trait[BI_MAXSP];
    if (!borg_spell_okay_fail(spell, 25)) {
        /* Restore Mana */
        borg.trait[BI_CURSP] = sv_mana;
        return 0;
    }

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, max_range, 0);

    /* return the value */
    if (borg_simulate) {
        /* Restore Mana */
        borg.trait[BI_CURSP] = sv_mana;
        return b_n;
    }

    /* Cast the spell with fake mana */
    borg.trait[BI_CURSP] = borg.trait[BI_MAXSP];
    if (borg_spell_fail(spell, 25)) {
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
    borg.trait[BI_CURSP] = 0;

    /* Value */
    return b_n;
}

/*
 *  Simulate/Apply the optimal result of using a "dispel" attack spell
 */
static int borg_attack_aux_spell_dispel(
    const enum borg_spells spell, int dam, int typ)
{
    int b_n;
    int penalty = 0;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Not if low on food */
    if (borg.trait[BI_FOOD] == 0
        && (borg.trait[BI_ISWEAK]
            && (borg_spell_legal(REMOVE_HUNGER)
                || borg_spell_legal(HERBAL_CURING))))
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* Require ability */
    if (!borg_spell_okay_fail(spell, 25))
        return 0;

    /* Choose optimal location--radius defined as 10 */
    b_n             = borg_launch_bolt(10, dam, typ, z_info->max_range, 0);

    int spell_power = borg_get_spell_power(spell);

    /* Penalize mana usage */
    b_n = b_n - spell_power;

    /* Penalize use of reserve mana */
    if (borg.trait[BI_CURSP] - spell_power < borg.trait[BI_MAXSP] / 2)
        b_n = b_n - (spell_power * 3);

    /* Penalize use of deep reserve mana */
    if (borg.trait[BI_CURSP] - spell_power < borg.trait[BI_MAXSP] / 3)
        b_n = b_n - (spell_power * 5);

    /* Really penalize use of mana needed for final teleport */
    switch (borg.trait[BI_CLASS]) {
    case CLASS_MAGE:
        penalty = 6;
        break;
    case CLASS_RANGER:
        penalty = 22;
        break;
    case CLASS_ROGUE:
        penalty = 20;
        break;
    case CLASS_PRIEST:
        penalty = 8;
        break;
    case CLASS_PALADIN:
        penalty = 20;
        break;
    case CLASS_NECROMANCER:
        penalty = 10;
        break;
    }
    if ((borg.trait[BI_MAXSP] > 30)
        && (borg.trait[BI_CURSP] - spell_power < penalty))
        b_n = b_n - (spell_power * 750);

    /* Really penalize use of mana needed for final teleport */
    /* (6 pts for mage) */
    if ((borg.trait[BI_MAXSP] > 30) && (borg.trait[BI_CURSP] - spell_power) < 6)
        b_n = b_n - (spell_power * 750);

    /* never tap if we previously missed a tap */
    if (target_closest < 0 && BORG_ATTACK_TAP_UNLIFE == typ && b_n > 0) {
        target_closest = 0;
        return 0;
    }

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Cast the prayer */
    (void)borg_spell(spell);

    /* Value */
    return b_n;
}

/*
 *  Simulate/Apply the optimal result of using a "dispel" staff
 * Which would be dispel evil, power, holiness.  Genocide handled later.
 */
static int borg_attack_aux_staff_dispel(int sval, int rad, int dam, int typ)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* look for the staff */
    if (!borg_equips_staff_fail(sval))
        return 0;

    /* Choose optimal location--radius defined as 10 */
    b_n = borg_launch_bolt(10, dam, typ, z_info->max_range, 0);

    /* Big Penalize charge usage */
    b_n = b_n - 50;

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Cast the prayer */
    (void)borg_use_staff(sval);

    /* Value */
    return b_n;
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack rod
 */
static int borg_attack_aux_rod_bolt(int sval, int rad, int dam, int typ)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* Not likely to be successful in the activation */
    if (500 < borg_activate_failure(TV_ROD, sval))
        return 0;

    /* Look for that rod */
    if (!borg_equips_rod(sval))
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Zap the rod */
    (void)borg_zap_rod(sval);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return b_n;
}

/*
 * Simulate/Apply the optimal result of using a "normal" attack wand
 */
static int borg_attack_aux_wand_bolt(
    int sval, int rad, int dam, int typ, int selection)
{
    int i;

    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Dont use wands in town, charges are too spendy */
    if (!borg.trait[BI_CDEPTH])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* Look for that wand */
    i = borg_slot(TV_WAND, sval);

    /* None available */
    if (i < 0)
        return 0;

    /* No charges */
    if (!borg_items[i].pval)
        return 0;

    /* Not likely to be successful in the activation */
    if (500 < borg_activate_failure(TV_WAND, sval))
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Penalize charge usage */
    if (borg.trait[BI_CLEVEL] > 5)
        b_n = b_n - 5;

    /* Wands of wonder are used in last ditch efforts.  They behave
     * randomly, so the best use of them is an emergency.  I have seen
     * borgs die from hill orcs with fully charged wonder wands.  Odds
     * are he could have taken the orcs with the wand.  So use them in
     * an emergency after all the borg_caution() steps have failed
     */
    if (sval == sv_wand_wonder && !borg.munchkin_mode) {
        /* check the danger */
        if (b_n > 0
            && borg_danger(borg.c.y, borg.c.x, 1, true, false)
                   >= (avoidance * 7 / 10)) {
            /* make the wand appear deadly */
            b_n = 999;

            /* note the use of the wand in the emergency */
            borg_note(format("# Emergency use of a Wand of Wonder."));
        } else {
            b_n = 0;
        }
    }

    /* Simulation */
    if (borg_simulate)
        return b_n;

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
    return b_n;
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
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 5))
        return 0;

    /* Look for an un-id'd wand */
    for (i = 0; i < z_info->pack_size; i++) {
        if (borg_items[i].tval != TV_WAND)
            continue;

        /* known */
        if (borg_items[i].kind)
            continue;

        /* No charges */
        if (!borg_items[i].pval)
            continue;
        if (strstr(borg_items[i].desc, "empty"))
            continue;

        /* Select this wand */
        b_i = i;
    }

    /* None available */
    if (b_i < 0)
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(0, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Log the message */
    borg_note(format("# Aiming unknown wand '%s.'", borg_items[b_i].desc));

    /* Perform the action */
    borg_keypress('a');
    borg_keypress(all_letters_nohjkl[b_i]);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return b_n;
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
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 5))
        return 0;

    /* Look for an un-id'd wand */
    for (i = 0; i < z_info->pack_size; i++) {
        if (borg_items[i].tval != TV_ROD)
            continue;

        /* known */
        if (borg_items[i].kind)
            continue;

        /* No charges */
        if (!borg_items[i].pval)
            continue;

        /* Not an attacker */
        if (strstr(borg_items[i].desc, "tried"))
            continue;

        /* Select this rod */
        b_i = i;
    }

    /* None available */
    if (b_i < 0)
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(0, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Log the message */
    borg_note(format("# Aiming unknown rod '%s.'", borg_items[b_i].desc));

    /* Perform the action */
    borg_keypress('z');
    borg_keypress(all_letters_nohjkl[b_i]);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return b_n;
}

/*
 * Simulate/Apply the optimal result of ACTIVATING an attack artifact
 */
static int borg_attack_aux_activation(
    int activation, int rad, int dam, int typ, bool aim, int selection)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* Look for and item with that activation and to see if it is charged */
    if (!borg_equips_item(activation, true))
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Activate the artifact */
    (void)borg_activate_item(activation);

    /* Use target */
    if (aim) {
        borg_keypress('5');

        /* Set our shooting flag */
        successful_target = -1;
    }

    /*  select the correct effect */
    if (selection != -1)
        borg_keypress('b' + selection);

    /* Value */
    return b_n;
}

/*
 * Simulate/Apply the optimal result of ACTIVATING an attack ring
 */
static int borg_attack_aux_ring(int ring_name, int rad, int dam, int typ)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* Look for that ring and to see if it is charged */
    if (!borg_equips_ring(ring_name))
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_bolt(rad, dam, typ, z_info->max_range, 0);

    /* Simulation */
    if (borg_simulate)
        return b_n;

    /* Activate the artifact */
    (void)borg_activate_ring(ring_name);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Value */
    return b_n;
}

/*
 * Simulate/Apply the optimal result of ACTIVATING a DRAGON ARMOUR
 */
static int borg_attack_aux_dragon(
    int sval, int rad, int dam, int typ, int selection)
{
    int b_n;

    /* No firing while blind, confused, or hallucinating */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        return 0;

    /* Paranoia */
    if (borg_simulate && (randint0(100) < 2))
        return 0;

    /* Randart dragon armors do not activate for breath */
    if (borg_items[INVEN_BODY].art_idx)
        return 0;

    /* Look for that scale mail and charged*/
    if (!borg_equips_dragon(sval))
        return 0;

    /* Choose optimal location */
    b_n = borg_launch_arc(rad, dam, typ, z_info->max_range);

    /* Simulation */
    if (borg_simulate)
        return b_n;

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
    return b_n;
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

    borg_grid *ag;

    borg_kill *kill;

    /* Can I do it */
    if (!borg_spell_okay_fail(
            WHIRLWIND_ATTACK, (borg_fighting_unique ? 40 : 25)))
        return 0;

    /* int original_danger = borg_danger(borg.c.y, borg.c.x, 1, false, false);
     */
    int blows = (borg.trait[BI_CLEVEL] + 10) / 15;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++) {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require "adjacent" */
        if (distance(borg.c, loc(x, y)) > 1)
            continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill);

        /* No damage */
        if (d <= 0)
            continue;

        /* get to do "blows" attacks */
        d = d * blows;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg.munchkin_mode) {
            /* Calculate danger */
            p = borg_danger_one_kill(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg.trait[BI_CDEPTH] && !kill->awake)
            continue;

        /* Calculate "danger" to player */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg.trait[BI_MAXCLEVEL] > 15)
            p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        total_d += d;
    }

    /* Nothing to attack */
    if (total_d < 0)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return (total_d);

    /* try the spell */
    if (borg_spell(WHIRLWIND_ATTACK))
        return (total_d);
    return 0;
}

/* trying the Leap into Battle spell */
static int borg_attack_aux_leap_into_battle(void)
{
    int p;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid *ag;

    borg_kill *kill;

    /* Can I do it */
    if (!borg_spell_okay_fail(
            LEAP_INTO_BATTLE, (borg_fighting_unique ? 40 : 25)))
        return 0;

    /* Too afraid to attack */
    if (borg.trait[BI_ISAFRAID] || borg.trait[BI_CRSFEAR])
        return 0;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++) {
        int blows;
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require up to distance 4 */
        int m_dist = distance(borg.c, loc(x, y));
        if (m_dist > 4)
            continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d     = borg_thrust_damage_one(ag->kill);
        blows = (borg.trait[BI_CLEVEL] + 5) / 15;
        blows = ((blows * m_dist + 2) / 4) + 1;
        d *= blows;

        /* No damage */
        if (d <= 0)
            continue;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg.munchkin_mode) {
            /* Calculate danger */
            p = borg_danger_one_kill(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg.trait[BI_CDEPTH] && !kill->awake)
            continue;

        /* Calculate "danger" to player */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg.trait[BI_MAXCLEVEL] > 15)
            p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d))
            continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return b_d;

    /* Save the location */
    borg.goal.g.x = borg_temp_x[b_i];
    borg.goal.g.y = borg_temp_y[b_i];

    ag            = &borg_grids[borg.goal.g.y][borg.goal.g.x];
    kill          = &borg_kills[ag->kill];

    /* Note */
    borg_note(
        format("# Leaping at %s at (%d,%d dist %d) who has %d Hit Points.",
            (r_info[kill->r_idx].name), borg.goal.g.y, borg.goal.g.x,
            distance(borg.c, borg.goal.g), kill->power));
    borg_note(
        format("# Attacking with weapon '%s'", borg_items[INVEN_WIELD].desc));

    /* Attack the grid */
    borg_target(borg.goal.g);
    borg_spell(LEAP_INTO_BATTLE);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Success */
    return b_d;
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

    borg_grid *ag;

    borg_kill *kill;

    /* Too afraid to attack */
    if (borg.trait[BI_ISAFRAID] || borg.trait[BI_CRSFEAR])
        return 0;

    /* Can I do it */
    if (!borg_spell_okay_fail(MAIM_FOE, (borg_fighting_unique ? 40 : 25)))
        return 0;

    blows = borg.trait[BI_CLEVEL] / 15;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++) {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Require "adjacent" */
        if (distance(borg.c, loc(x, y)) > 1)
            continue;

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Calculate "average" damage */
        d = borg_thrust_damage_one(ag->kill) * blows;

        /* No damage */
        if (d <= 0)
            continue;

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg.munchkin_mode) {
            /* Calculate danger */
            p = borg_danger_one_kill(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg.trait[BI_CDEPTH] && !kill->awake)
            continue;

        /* Calculate "danger" to player */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg.trait[BI_MAXCLEVEL] > 15)
            p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d))
            continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i < 0)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return b_d;

    /* Save the location */
    borg.goal.g.x = borg_temp_x[b_i];
    borg.goal.g.y = borg_temp_y[b_i];

    ag            = &borg_grids[borg.goal.g.y][borg.goal.g.x];
    kill          = &borg_kills[ag->kill];

    /* Get a direction for attacking */
    dir = borg_extract_dir(borg.c.y, borg.c.x, borg.goal.g.y, borg.goal.g.x);

    /* Simulation */
    if (borg_simulate)
        return d;

    borg_spell(MAIM_FOE);
    borg_keypress(I2D(dir));

    return d;
}

/* trying the Curse spell */
static int borg_attack_aux_curse(void)
{
    int p;

    int i, b_i = -1;
    int d, b_d = -1;

    borg_grid *ag;

    borg_kill *kill;

    /* costs 100hp to cast.  Don't kill yourself doing it */
    if (borg.trait[BI_CURHP] < 120)
        return 0;

    /* Can I do it */
    if (!borg_spell_okay_fail(CURSE, (borg_fighting_unique ? 40 : 25)))
        return 0;

    /* Too afraid to attack */
    if (borg.trait[BI_ISAFRAID] || borg.trait[BI_CRSFEAR])
        return 0;

    /* Examine possible kills */
    for (i = 0; i < borg_temp_n; i++) {
        int x = borg_temp_x[i];
        int y = borg_temp_y[i];

        /* Acquire grid */
        ag = &borg_grids[y][x];

        /* Obtain the monster */
        kill = &borg_kills[ag->kill];

        /* Calculate "average" damage */
        d = (((((kill->injury * kill->power) / 100) + 1) / 2) + 50)
            * (borg.trait[BI_CLEVEL] / 12 + 1);

        /* No damage */
        if (d <= 0)
            continue;

        /* Hack -- avoid waking most "hard" sleeping monsters */
        if (!kill->awake && (d <= kill->power) && !borg.munchkin_mode) {
            /* Calculate danger */
            p = borg_danger_one_kill(y, x, 1, ag->kill, true, true);

            if (p > avoidance * 2)
                continue;
        }

        /* Hack -- ignore sleeping town monsters */
        if (!borg.trait[BI_CDEPTH] && !kill->awake)
            continue;

        /* Calculate "danger" to player */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 2, ag->kill, true, true);

        /* Reduce "bonus" of partial kills when higher level */
        if (d <= kill->power && borg.trait[BI_MAXCLEVEL] > 15)
            p = p / 10;

        /* Add the danger-bonus to the damage */
        d += p;

        /* Ignore lower damage */
        if ((b_i >= 0) && (d < b_d))
            continue;

        /* Save the info */
        b_i = i;
        b_d = d;
    }

    /* Nothing to attack */
    if (b_i <= 0)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return b_d;

    /* Save the location */
    borg.goal.g.x = borg_temp_x[b_i];
    borg.goal.g.y = borg_temp_y[b_i];

    ag            = &borg_grids[borg.goal.g.y][borg.goal.g.x];
    kill          = &borg_kills[ag->kill];

    /* Attack the grid */
    borg_target(borg.goal.g);
    borg_spell(CURSE);

    /* Use target */
    borg_keypress('5');

    /* Set our shooting flag */
    successful_target = -1;

    /* Success */
    return b_d;
}

/* trying the Vampire Strike spell */
static int borg_attack_aux_vampire_strike(void)
{
    int p;

    int  i, b_i = -1;
    int  d;
    int  dist, best_dist = z_info->max_range;
    int  o_x, o_y, x2, y2;
    int  x = 0, y = 0;

    borg_grid *ag;
    borg_kill *kill;

    /* Can I do it */
    if (!borg_spell_okay_fail(VAMPIRE_STRIKE, (borg_fighting_unique ? 40 : 25)))
        return 0;

    /* Examine possible destinations */
    for (i = 0; i < borg_temp_n; i++) {
        x = borg_temp_x[i];
        y = borg_temp_y[i];

        /* closest distance */
        dist = distance(borg.c, loc(x, y));
        if (dist > best_dist)
            continue;

        best_dist = dist;
        b_i = i;
    }

    /* if we didn't find anyone, done */
    if (b_i == -1)
        return 0;

    /* Nothing to attack, require relatively close */
    if (dist >= 20)
        return 0;

    x = borg_temp_x[b_i];
    y = borg_temp_y[b_i];

    /* Consider each adjacent spot to the monster */
    /* there must be an empty spot */
    bool found = false;
    for (o_x = -1; o_x <= 1 && !found; o_x++) {
        for (o_y = -1; o_y <= 1 && !found; o_y++) {
            /* but not the monsters location */
            if (!o_x && !o_y)
                continue;

            /* Acquire location */
            x2 = borg_temp_x[b_i] + o_x;
            y2 = borg_temp_y[b_i] + o_y;

            ag = &borg_grids[y2][x2];
            if (!ag->kill && ag->feat == FEAT_FLOOR 
                && !ag->web
                && !ag->glyph
                && (y2 != borg.c.y || x2 != borg.c.x))
                found = true;
        }
    }
    /* must have an empty square next to the monster */
    if (!found)
        return 0;

    /* Check the projectable, assume unknown grids are walls */
    if (!borg_offset_projectable(borg.c.y, borg.c.x, y, x))
        return 0;

    /* Acquire grid */
    ag = &borg_grids[y][x];

    /* Calculate "average" damage */
    d = borg.trait[BI_CLEVEL] * 2;

    /* Obtain the monster */
    kill                       = &borg_kills[ag->kill];

    struct monster_race *r_ptr = &r_info[kill->r_idx];
    if (rf_has(r_ptr->flags, RF_NONLIVING)
        || rf_has(r_ptr->flags, RF_UNDEAD))
        return 0;

    /* Hack -- avoid waking most "hard" sleeping monsters */
    if (!kill->awake && (d <= kill->power) && !borg.munchkin_mode) {
        /* Calculate danger */
        p = borg_danger_one_kill(y, x, 1, ag->kill, true, true);

        if (p > avoidance * 2) 
            return 0;
    }

    /* Calculate "danger" to player */
    p = borg_danger_one_kill(borg.c.y, borg.c.x, 2, ag->kill, true, true);

    /* Reduce "bonus" of partial kills when higher level */
    if (d <= kill->power && borg.trait[BI_MAXCLEVEL] > 15)
        p = p / 10;

    /* Add the danger-bonus to the damage */
    d += p;

    /* never vampire strike if we previously missed a vampire strike (or other shot) */
    if (target_closest < 0 && d > 0) {
        target_closest = 0;
        return 0;
    }

    /* Simulation */
    if (borg_simulate)
        return d;

    /* cast the spell */
    borg_spell(VAMPIRE_STRIKE);

    /* Success */
    return d;
}

/* trying the Crush spell */
/* right now it is coded so that if a monster is partially */
/* crushed, it is still fully a danger */
static int borg_attack_aux_crush(void)
{
    int p1 = 0;
    int p2 = 0;
    int d  = 0;

    /* Can I do it */
    if (!borg_spell_okay(CRUSH))
        return 0;

    /* don't kill yourself or leave less than 10hp */
    if ((borg.trait[BI_CURHP] + 10) < (borg.trait[BI_CLEVEL] * 4))
        return 0;

    /* Obtain initial danger */
    borg_crush_spell = false;
    p1               = borg_danger(borg.c.y, borg.c.x, 4, true, false);

    /* What effect is there? */
    borg_crush_spell = true;
    p2               = borg_danger(borg.c.y, borg.c.x, 4, true, false);
    borg_crush_spell = false;

    /* damage is reduction in danger */
    d = (p1 - p2);

    /* if there is still danger afterward, make sure the reductioning in HP */
    /* doesn't make this put us in danger */
    int new_hp = (borg.trait[BI_CURHP] - (borg.trait[BI_CLEVEL] * 2));
    if (borg_simulate && (p2 >= new_hp || new_hp <= 5))
        return 0;

    int spell_power = borg_get_spell_power(CRUSH);

    /* Penalize mana usage */
    d = d - spell_power;

    /* Penalize use of reserve mana */
    if (borg.trait[BI_CURSP] - spell_power < borg.trait[BI_MAXSP] / 2)
        d = d - (spell_power * 10);

    /* Simulation */
    if (borg_simulate)
        return d;

    /* Cast the spell */
    if (borg_spell(CRUSH))
        return d;
    else
        return 0;
}

/*
 * Try to sleep an adjacent bad guy
 * This had been a defense maneuver, which explains the format.
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
    int d  = 0;

    /* Can I do it */
    if (!borg_spell_okay(TRANCE))
        return 0;

    /* Obtain initial danger */
    borg_sleep_spell_ii = false;
    p1                  = borg_danger(borg.c.y, borg.c.x, 4, true, false);

    /* What effect is there? */
    borg_sleep_spell_ii = true;
    p2                  = borg_danger(borg.c.y, borg.c.x, 4, true, false);
    borg_sleep_spell_ii = false;

    /* value is d, enhance the value for rogues and rangers so that
     * they can use their critical hits.
     */
    d               = (p1 - p2);

    int spell_power = borg_get_spell_power(TRANCE);

    /* Penalize mana usage */
    d = d - spell_power;

    /* Penalize use of reserve mana */
    if (borg.trait[BI_CURSP] - spell_power < borg.trait[BI_MAXSP] / 2)
        d = d - (spell_power * 10);

    /* Simulation */
    if (borg_simulate)
        return d;

    /* Cast the spell */
    if (borg_spell(TRANCE))
        return d;

    return 0;
}

static int borg_attack_aux_artifact_holcolleth(void)
{
    int p1 = 0;
    int p2 = 0;
    int d  = 0;

    if (!borg_equips_item(act_sleepii, true))
        return 0;

    /* Obtain initial danger */
    borg_sleep_spell_ii = false;
    p1                  = borg_danger(borg.c.y, borg.c.x, 4, true, false);

    /* What effect is there? */
    borg_sleep_spell_ii = true;
    p2                  = borg_danger(borg.c.y, borg.c.x, 4, true, false);
    borg_sleep_spell_ii = false;

    /* value is d, enhance the value for rogues and rangers so that
     * they can use their critical hits.
     */
    d = (p1 - p2);

    /* Simulation */
    if (borg_simulate)
        return d;

    /* Cast the spell */
    if (borg_activate_item(act_sleepii)) {
        /* Value */
        return d;
    } else {
        borg_note("# Failed to properly activate the artifact");
        return 0;
    }
}

/*
 * Simulate/Apply the optimal result of using the given "type" of attack
 */
int borg_calculate_attack_effectiveness(int attack_type)
{
    int dam = 0;
    int rad = 0;

    /* Analyze */
    switch (attack_type) {
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
        return (borg_attack_aux_spell_bolt(
            SLOW_MONSTER, rad, dam, BORG_ATTACK_OLD_SLOW, z_info->max_range, false));

    /* Spell -- confuse monster */
    case BF_SPELL_CONFUSE_MONSTER:
        rad = 0;
        dam = 10;
        return (borg_attack_aux_spell_bolt(CONFUSE_MONSTER, rad, dam,
            BORG_ATTACK_OLD_CONF, z_info->max_range, false));

    case BF_SPELL_SLEEP_III:
        dam = 10;
        return (borg_attack_aux_spell_dispel(
            MASS_SLEEP, dam, BORG_ATTACK_OLD_SLEEP));

    /* Spell -- magic missile */
    case BF_SPELL_MAGIC_MISSILE:
        rad = 0;
        dam = ((((borg.trait[BI_CLEVEL] - 1) / 5) + 3) * (4 + 1)) / 2;
        return (borg_attack_aux_spell_bolt(
            MAGIC_MISSILE, rad, dam, BORG_ATTACK_MISSILE, z_info->max_range, false));

    /* Spell -- magic missile EMERGENCY*/
    case BF_SPELL_MAGIC_MISSILE_RESERVE:
        rad = 0;
        dam = ((((borg.trait[BI_CLEVEL] - 1) / 5) + 3) * (4 + 1));
        return (borg_attack_aux_spell_bolt_reserve(
            MAGIC_MISSILE, rad, dam, BORG_ATTACK_MISSILE, z_info->max_range));

    /* Spell -- cold bolt */
    case BF_SPELL_COLD_BOLT:
        rad = 0;
        dam = ((((borg.trait[BI_CLEVEL] - 5) / 3) + 6) * (8 + 1)) / 2;
        return (borg_attack_aux_spell_bolt(
            FROST_BOLT, rad, dam, BORG_ATTACK_COLD, z_info->max_range, false));

    /* Spell -- kill wall */
    case BF_SPELL_STONE_TO_MUD:
        rad = 0;
        dam = (20 + (30 / 2));
        return (borg_attack_aux_spell_bolt(TURN_STONE_TO_MUD, rad, dam,
            BORG_ATTACK_KILL_WALL, z_info->max_range, false));

    /* Spell -- light beam */
    case BF_SPELL_LIGHT_BEAM:
        rad = -1;
        dam = (6 * (8 + 1) / 2);
        return (borg_attack_aux_spell_bolt(SPEAR_OF_LIGHT, rad, dam,
            BORG_ATTACK_LIGHT_WEAK, z_info->max_range, false));

    /* Spell -- stinking cloud */
    case BF_SPELL_STINK_CLOUD:
        rad = 2;
        dam = (10 + (borg.trait[BI_CLEVEL] / 2));
        return (borg_attack_aux_spell_bolt(
            STINKING_CLOUD, rad, dam, BORG_ATTACK_POIS, z_info->max_range, false));

    /* Spell -- fire ball */
    case BF_SPELL_FIRE_BALL:
        rad = 2;
        dam = (borg.trait[BI_CLEVEL] * 2);
        return (borg_attack_aux_spell_bolt(
            FIRE_BALL, rad, dam, BORG_ATTACK_FIRE, z_info->max_range, false));

    /* Spell -- Ice Storm */
    case BF_SPELL_COLD_STORM:
        dam = (3 * ((borg.trait[BI_CLEVEL] * 3) + 1)) / 2;
        return (borg_attack_aux_spell_dispel(
            ICE_STORM, dam, BORG_ATTACK_ICE));

    /* Spell -- Meteor Swarm */
    case BF_SPELL_METEOR_SWARM:
        rad = 1;
        dam = (30 + borg.trait[BI_CLEVEL] / 2) + (borg.trait[BI_CLEVEL] / 20)
              + 2;
        return (borg_attack_aux_spell_bolt(
            METEOR_SWARM, rad, dam, BORG_ATTACK_METEOR, z_info->max_range, false));

    /* Spell -- Rift */
    case BF_SPELL_RIFT:
        rad = -1;
        dam = ((borg.trait[BI_CLEVEL] * 3) + 40);
        return (borg_attack_aux_spell_bolt(
            RIFT, rad, dam, BORG_ATTACK_GRAVITY, z_info->max_range, false));

    /* Spell -- mana storm */
    case BF_SPELL_MANA_STORM:
        rad = 3;
        dam = (300 + (borg.trait[BI_CLEVEL] * 2));
        return (borg_attack_aux_spell_bolt(
            MANA_STORM, rad, dam, BORG_ATTACK_MANA, z_info->max_range, false));

    /* Spell -- Shock Wave */
    case BF_SPELL_SHOCK_WAVE:
        dam = (borg.trait[BI_CLEVEL] * 2);
        rad = 2;
        return (borg_attack_aux_spell_bolt(
            SHOCK_WAVE, rad, dam, BORG_ATTACK_SOUND, z_info->max_range, false));

    /* Spell -- Explosion */
    case BF_SPELL_EXPLOSION:
        dam = ((borg.trait[BI_CLEVEL] * 2)
               + (borg.trait[BI_CLEVEL]
                   / 5)); /* hack pretend it is all shards */
        rad = 2;
        return (borg_attack_aux_spell_bolt(
            EXPLOSION, rad, dam, BORG_ATTACK_SHARD, z_info->max_range, false));

    /* Prayer -- orb of draining */
    case BF_PRAYER_HOLY_ORB_BALL:
        rad = ((borg.trait[BI_CLEVEL] >= 30) ? 3 : 2);
        dam = ((borg.trait[BI_CLEVEL] * 3) / 2) + (3 * (6 + 1)) / 2;
        return (borg_attack_aux_spell_bolt(ORB_OF_DRAINING, rad, dam,
            BORG_ATTACK_HOLY_ORB, z_info->max_range, false));

    /* Prayer -- blind creature */
    case BF_SPELL_BLIND_CREATURE:
        rad = 0;
        dam = 10;
        return (borg_attack_aux_spell_bolt(
            FRIGHTEN, rad, dam, BORG_ATTACK_OLD_CONF, z_info->max_range, false));

    /* Druid - Trance */
    case BF_SPELL_TRANCE:
        return (borg_attack_aux_trance());

    /* Prayer -- Dispel Undead */
    case BF_PRAYER_DISP_UNDEAD:
        dam = (((borg.trait[BI_CLEVEL] * 5) + 1) / 2);
        return (borg_attack_aux_spell_dispel(
            DISPEL_UNDEAD, dam, BORG_ATTACK_DISP_UNDEAD));

    /* Prayer -- Dispel Evil */
    case BF_PRAYER_DISP_EVIL:
        dam = (((borg.trait[BI_CLEVEL] * 5) + 1) / 2);
        return (borg_attack_aux_spell_dispel(
            DISPEL_EVIL, dam, BORG_ATTACK_DISP_EVIL));

    /* Prayer -- Dispel Undead */
    case BF_PRAYER_DISP_SPIRITS:
        dam = (100);
        return (borg_attack_aux_spell_dispel(
            BANISH_SPIRITS, dam, BORG_ATTACK_DISP_SPIRITS));

    /* Prayer -- Banishment (teleport evil away)*/
    /* This is a defense spell:  done in borg_defense() */

    /* Prayer -- Holy Word also has heal effect and is considered in borg_heal
     */
    case BF_PRAYER_HOLY_WORD:
        if (borg.trait[BI_MAXHP] - borg.trait[BI_CURHP] >= 300)
        /* force him to think the spell is more deadly to get him to
         * cast it.  This will provide some healing for him.
         */
        {
            dam = ((borg.trait[BI_CLEVEL] * 10));
            return (borg_attack_aux_spell_dispel(
                HOLY_WORD, dam, BORG_ATTACK_DISP_EVIL));
        } else /* If he is not wounded don't cast this, use Disp Evil instead.
                */
        {
            dam = ((borg.trait[BI_CLEVEL] * 3) / 2) - 50;
            return (borg_attack_aux_spell_dispel(
                DISPEL_EVIL, dam, BORG_ATTACK_DISP_EVIL));
        }

    /* Prayer -- Annihilate */
    case BF_SPELL_ANNIHILATE:
        rad = 0;
        dam = (borg.trait[BI_CLEVEL] * 4);
        return (borg_attack_aux_spell_bolt(
            ANNIHILATE, rad, dam, BORG_ATTACK_OLD_DRAIN, z_info->max_range, false));

    /* Spell -- Electric Arc */
    case BF_SPELL_ELECTRIC_ARC:
        rad = 0;
        dam = ((((borg.trait[BI_CLEVEL] - 1) / 5) + 3) * (6 + 1)) / 2;
        return (borg_attack_aux_spell_bolt(
            ELECTRIC_ARC, rad, dam, BORG_ATTACK_ELEC, borg.trait[BI_CLEVEL], false));

    case BF_SPELL_ACID_SPRAY:
        rad = 60; 
        dam = ((borg.trait[BI_CLEVEL] / 2) * (8 + 1)) / 2;
        return (borg_attack_aux_spell_bolt(
            ACID_SPRAY, rad, dam, BORG_ATTACK_ACID, 10, true));

    /* Spell -- mana bolt */
    case BF_SPELL_MANA_BOLT:
        rad = 0;
        dam = ((borg.trait[BI_CLEVEL] - 10) * (8 + 1) / 2);
        return (borg_attack_aux_spell_bolt(
            MANA_BOLT, rad, dam, BORG_ATTACK_MANA, z_info->max_range, false));

    /* Spell -- thrust away */
    case BF_SPELL_THRUST_AWAY:
        rad = 0;
        dam = (borg.trait[BI_CLEVEL] * (8 + 1) / 2);
        return (borg_attack_aux_spell_bolt(THRUST_AWAY, rad, dam,
            BORG_ATTACK_FORCE, (borg.trait[BI_CLEVEL] / 10) + 1, false));

    /* Spell -- Lightning Strike */
    case BF_SPELL_LIGHTNING_STRIKE:
        rad = 0;
        dam = ((borg.trait[BI_CLEVEL] / 4) * (4 + 1) / 2)
              + borg.trait[BI_CLEVEL] + 5; /* HACK pretend it is all elec */
        return (borg_attack_aux_spell_bolt(
            LIGHTNING_STRIKE, rad, dam, BORG_ATTACK_ELEC, z_info->max_range, false));

    /* Spell -- Earth Rising */
    case BF_SPELL_EARTH_RISING:
        rad = 0;
        dam = (((borg.trait[BI_CLEVEL] / 3) + 2) * (6 + 1) / 2)
              + borg.trait[BI_CLEVEL] + 5;
        return (borg_attack_aux_spell_bolt(EARTH_RISING, rad, dam,
            BORG_ATTACK_SHARD, (borg.trait[BI_CLEVEL] / 5) + 4, false));

    /* Spell -- Volcanic Eruption */
    /* just count the damage.  The earthquake defense is a side bennie,
     * perhaps... */
    case BF_SPELL_VOLCANIC_ERUPTION:
        rad = 0;
        dam = (((borg.trait[BI_CLEVEL] * 3) / 2)
                  * ((borg.trait[BI_CLEVEL] * 3) + 1))
              / 2;
        return (borg_attack_aux_spell_bolt(
            VOLCANIC_ERUPTION, rad, dam, BORG_ATTACK_FIRE, z_info->max_range, false));

    /* Spell -- River of Lightning */
    case BF_SPELL_RIVER_OF_LIGHTNING:
        rad = 20;
        dam = (borg.trait[BI_CLEVEL] + 10) * (8 + 1) / 2;
        return (borg_attack_aux_spell_bolt(
            RIVER_OF_LIGHTNING, rad, dam, BORG_ATTACK_PLASMA, 20, true));

    /* spell -- Spear of Oromë */
    case BF_SPELL_SPEAR_OF_OROME:
        rad = 0;
        dam = ((borg.trait[BI_CLEVEL] / 2) + (8 + 1)) / 2;
        return (borg_attack_aux_spell_bolt(
            SPEAR_OF_OROME, rad, dam, BORG_ATTACK_HOLY_ORB, z_info->max_range, false));

    /* spell -- Light of Manwë */
    case BF_SPELL_LIGHT_OF_MANWE:
        rad = 0;
        dam = borg.trait[BI_CLEVEL] * 5 + 100;
        return (borg_attack_aux_spell_bolt(
            LIGHT_OF_MANWE, rad, dam, BORG_ATTACK_LIGHT, z_info->max_range, false));

    /* spell -- Nether Bolt */
    case BF_SPELL_NETHER_BOLT:
        rad = 0;
        dam = ((((borg.trait[BI_CLEVEL] / 4) + 3) * (4 + 1)) / 2);
        return (borg_attack_aux_spell_bolt(
            NETHER_BOLT, rad, dam, BORG_ATTACK_NETHER, z_info->max_range, false));

    /* spell -- Tap Unlife */
    case BF_SPELL_TAP_UNLIFE:
        dam = ((((borg.trait[BI_CLEVEL] / 4) + 3) * (4 + 1)) / 2);
        return (borg_attack_aux_spell_dispel(
            TAP_UNLIFE, dam, BORG_ATTACK_TAP_UNLIFE));

    /* Spell - Crush */
    case BF_SPELL_CRUSH:
        return (borg_attack_aux_crush());

    case BF_SPELL_SLEEP_EVIL:
        dam = borg.trait[BI_CLEVEL] * 10 + 500;
        return (borg_attack_aux_spell_dispel(
            SLEEP_EVIL, dam, BORG_ATTACK_SLEEP_EVIL));

    /* spell -- Disenchant */
    case BF_SPELL_DISENCHANT:
        rad = 0;
        dam = ((((borg.trait[BI_CLEVEL] * 2) + 10) + 1) / 2) * 2;
        return (borg_attack_aux_spell_bolt(
            DISENCHANT, rad, dam, BORG_ATTACK_DISEN, z_info->max_range, false));

    /* spell -- Frighten */
    case BF_SPELL_FRIGHTEN:
        rad = 0;
        dam = borg.trait[BI_CLEVEL];
        return (borg_attack_aux_spell_bolt(
            FRIGHTEN, rad, dam, BORG_ATTACK_TURN_ALL, z_info->max_range, false));

    /* Spell - Vampire Strike*/
    case BF_SPELL_VAMPIRE_STRIKE:
        return (borg_attack_aux_vampire_strike());

    /* Spell - Dispel Life */
    case BF_PRAYER_DISPEL_LIFE:
        rad = 0;
        dam = ((borg.trait[BI_CLEVEL] * 3) + 1) / 2;
        return (borg_attack_aux_spell_bolt(
            DISPEL_LIFE, rad, dam, BORG_ATTACK_DRAIN_LIFE, z_info->max_range, false));

    /* spell -- Dark Spear */
    case BF_SPELL_DARK_SPEAR:
        rad = 0;
        dam = (((borg.trait[BI_CLEVEL] * 2) + 1) / 2) * 2;
        return (borg_attack_aux_spell_bolt(
            DARK_SPEAR, rad, dam, BORG_ATTACK_DARK, z_info->max_range, false));

    /* spell -- Unleash Chaos */
    case BF_SPELL_UNLEASH_CHAOS:
        rad = 0;
        dam = ((borg.trait[BI_CLEVEL] + 1) / 2) * 8;
        return (borg_attack_aux_spell_bolt(
            UNLEASH_CHAOS, rad, dam, BORG_ATTACK_CHAOS, z_info->max_range, false));

    /* Spell -- Storm of Darkness */
    case BF_SPELL_STORM_OF_DARKNESS:
        rad = 4;
        dam = (((borg.trait[BI_CLEVEL] * 2) + 1) / 2) * 4;
        return (borg_attack_aux_spell_bolt(
            STORM_OF_DARKNESS, rad, dam, BORG_ATTACK_DARK, z_info->max_range, false));

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
        dam = borg.trait[BI_CLEVEL];
        return (borg_attack_aux_spell_dispel(
            HOWL_OF_THE_DAMNED, dam, BORG_ATTACK_TURN_ALL));

    /* ROD -- slow monster */
    case BF_ROD_SLOW_MONSTER:
        dam = 10;
        rad = 0;
        return (borg_attack_aux_rod_bolt(
            sv_rod_slow_monster, rad, dam, BORG_ATTACK_OLD_SLOW));

    /* ROD -- sleep monster */
    case BF_ROD_SLEEP_MONSTER:
        dam = 10;
        rad = 0;
        return (borg_attack_aux_rod_bolt(
            sv_rod_sleep_monster, rad, dam, BORG_ATTACK_OLD_SLEEP));

    /* Rod -- elec bolt */
    case BF_ROD_ELEC_BOLT:
        rad = -1;
        dam = 6 * (6 + 1) / 2;
        return (borg_attack_aux_rod_bolt(
            sv_rod_elec_bolt, rad, dam, BORG_ATTACK_ELEC));

    /* Rod -- cold bolt */
    case BF_ROD_COLD_BOLT:
        rad = 0;
        dam = 12 * (8 + 1) / 2;
        return (borg_attack_aux_rod_bolt(
            sv_rod_cold_bolt, rad, dam, BORG_ATTACK_COLD));

    /* Rod -- acid bolt */
    case BF_ROD_ACID_BOLT:
        rad = 0;
        dam = 12 * (8 + 1) / 2;
        return (borg_attack_aux_rod_bolt(
            sv_rod_acid_bolt, rad, dam, BORG_ATTACK_ACID));

    /* Rod -- fire bolt */
    case BF_ROD_FIRE_BOLT:
        rad = 0;
        dam = 12 * (8 + 1) / 2;
        return (borg_attack_aux_rod_bolt(
            sv_rod_fire_bolt, rad, dam, BORG_ATTACK_FIRE));

    /* Rod -- light beam */
    case BF_ROD_LIGHT_BEAM:
        rad = -1;
        dam = (6 * (8 + 1) / 2);
        return (borg_attack_aux_rod_bolt(
            sv_rod_light, rad, dam, BORG_ATTACK_LIGHT_WEAK));

    /* Rod -- drain life */
    case BF_ROD_DRAIN_LIFE:
        rad = 0;
        dam = (150);
        return (borg_attack_aux_rod_bolt(
            sv_rod_drain_life, rad, dam, BORG_ATTACK_OLD_DRAIN));

    /* Rod -- elec ball */
    case BF_ROD_ELEC_BALL:
        rad = 2;
        dam = 64;
        return (borg_attack_aux_rod_bolt(
            sv_rod_elec_ball, rad, dam, BORG_ATTACK_ELEC));

    /* Rod -- acid ball */
    case BF_ROD_COLD_BALL:
        rad = 2;
        dam = 100;
        return (borg_attack_aux_rod_bolt(
            sv_rod_cold_ball, rad, dam, BORG_ATTACK_COLD));

    /* Rod -- acid ball */
    case BF_ROD_ACID_BALL:
        rad = 2;
        dam = 120;
        return (borg_attack_aux_rod_bolt(
            sv_rod_acid_ball, rad, dam, BORG_ATTACK_ACID));

    /* Rod -- fire ball */
    case BF_ROD_FIRE_BALL:
        rad = 2;
        dam = 144;
        return (borg_attack_aux_rod_bolt(
            sv_rod_fire_ball, rad, dam, BORG_ATTACK_FIRE));

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
        return (borg_attack_aux_wand_bolt(
            sv_wand_magic_missile, rad, dam, BORG_ATTACK_MISSILE, -1));

    /* Wand -- slow monster */
    case BF_WAND_SLOW_MONSTER:
        rad = 0;
        dam = 10;
        return (borg_attack_aux_wand_bolt(
            sv_wand_slow_monster, rad, dam, BORG_ATTACK_OLD_SLOW, -1));

    /* Wand -- sleep monster */
    case BF_WAND_HOLD_MONSTER:
        rad = 0;
        dam = 10;
        return (borg_attack_aux_wand_bolt(
            sv_wand_hold_monster, rad, dam, BORG_ATTACK_OLD_SLEEP, -1));

    /* Wand -- fear monster */
    case BF_WAND_FEAR_MONSTER:
        rad = 0;
        dam = 2 * (6 + 1) / 2;
        return (borg_attack_aux_wand_bolt(
            sv_wand_fear_monster, rad, dam, BORG_ATTACK_TURN_ALL, -1));

    /* Wand -- conf monster */
    case BF_WAND_CONFUSE_MONSTER:
        rad = 0;
        dam = 2 * (6 + 1) / 2;
        return (borg_attack_aux_wand_bolt(
            sv_wand_confuse_monster, rad, dam, BORG_ATTACK_OLD_CONF, -1));

    /* Wand -- elec bolt */
    case BF_WAND_ELEC_BOLT:
        dam = 6 * (6 + 1) / 2;
        rad = -1;
        return (borg_attack_aux_wand_bolt(
            sv_wand_elec_bolt, rad, dam, BORG_ATTACK_ELEC, -1));

    /* Wand -- cold bolt */
    case BF_WAND_COLD_BOLT:
        dam = 12 * (8 + 1) / 2;
        rad = 0;
        return (borg_attack_aux_wand_bolt(
            sv_wand_cold_bolt, rad, dam, BORG_ATTACK_COLD, -1));

    /* Wand -- acid bolt */
    case BF_WAND_ACID_BOLT:
        rad = 0;
        dam = 5 * (8 + 1) / 2;
        return (borg_attack_aux_wand_bolt(
            sv_wand_acid_bolt, rad, dam, BORG_ATTACK_ACID, -1));

    /* Wand -- fire bolt */
    case BF_WAND_FIRE_BOLT:
        rad = 0;
        dam = 12 * (8 + 1) / 2;
        return (borg_attack_aux_wand_bolt(
            sv_wand_fire_bolt, rad, dam, BORG_ATTACK_FIRE, -1));

    /* Wand -- light beam */
    case BF_WAND_LIGHT_BEAM:
        rad = -1;
        dam = (6 * (8 + 1) / 2);
        return (borg_attack_aux_wand_bolt(
            sv_wand_light, rad, dam, BORG_ATTACK_LIGHT_WEAK, -1));

    /* Wand -- stinking cloud */
    case BF_WAND_STINKING_CLOUD:
        rad = 2;
        dam = 12;
        return (borg_attack_aux_wand_bolt(
            sv_wand_stinking_cloud, rad, dam, BORG_ATTACK_POIS, -1));

    /* Wand -- elec ball */
    case BF_WAND_ELEC_BALL:
        rad = 2;
        dam = 64;
        return (borg_attack_aux_wand_bolt(
            sv_wand_elec_ball, rad, dam, BORG_ATTACK_ELEC, -1));

    /* Wand -- acid ball */
    case BF_WAND_COLD_BALL:
        rad = 2;
        dam = 100;
        return (borg_attack_aux_wand_bolt(
            sv_wand_cold_ball, rad, dam, BORG_ATTACK_COLD, -1));

    /* Wand -- acid ball */
    case BF_WAND_ACID_BALL:
        rad = 2;
        dam = 120;
        return (borg_attack_aux_wand_bolt(
            sv_wand_acid_ball, rad, dam, BORG_ATTACK_ACID, -1));

    /* Wand -- fire ball */
    case BF_WAND_FIRE_BALL:
        rad = 2;
        dam = 144;
        return (borg_attack_aux_wand_bolt(
            sv_wand_fire_ball, rad, dam, BORG_ATTACK_FIRE, -1));

    /* Wand -- dragon cold */
    case BF_WAND_DRAGON_COLD:
        rad = 3;
        dam = 160;
        return (borg_attack_aux_wand_bolt(
            sv_wand_dragon_cold, rad, dam, BORG_ATTACK_COLD, -1));

    /* Wand -- dragon fire */
    case BF_WAND_DRAGON_FIRE:
        rad = 3;
        dam = 200;
        return (borg_attack_aux_wand_bolt(
            sv_wand_dragon_fire, rad, dam, BORG_ATTACK_FIRE, -1));

    /* Wand -- annihilation */
    case BF_WAND_ANNIHILATION:
        dam = 250;
        return (borg_attack_aux_wand_bolt(
            sv_wand_annihilation, rad, dam, BORG_ATTACK_OLD_DRAIN, -1));

    /* Wand -- drain life */
    case BF_WAND_DRAIN_LIFE:
        dam = 150;
        return (borg_attack_aux_wand_bolt(
            sv_wand_drain_life, rad, dam, BORG_ATTACK_OLD_DRAIN, -1));

    /* Wand -- wand of wonder */
    case BF_WAND_WONDER:
        dam = 35;
        return (borg_attack_aux_wand_bolt(
            sv_wand_wonder, rad, dam, BORG_ATTACK_MISSILE, -1));

    /* Staff -- Sleep Monsters */
    case BF_STAFF_SLEEP_MONSTERS:
        dam = 60;
        return (borg_attack_aux_staff_dispel(
            sv_staff_sleep_monsters, rad, dam, BORG_ATTACK_OLD_SLEEP));

    /* Staff -- Slow Monsters */
    case BF_STAFF_SLOW_MONSTERS:
        dam = 60;
        rad = 10;
        return (borg_attack_aux_staff_dispel(
            sv_staff_slow_monsters, rad, dam, BORG_ATTACK_OLD_SLOW));

    /* Staff -- Dispel Evil */
    case BF_STAFF_DISPEL_EVIL:
        dam = 60;
        return (borg_attack_aux_staff_dispel(
            sv_staff_dispel_evil, rad, dam, BORG_ATTACK_DISP_EVIL));

    /* Staff -- Power */
    case BF_STAFF_POWER:
        dam = 120;
        return (borg_attack_aux_staff_dispel(
            sv_staff_power, rad, dam, BORG_ATTACK_TURN_ALL));

    /* Staff -- holiness */
    case BF_STAFF_HOLINESS:
        if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2)
            dam = 500;
        else
            dam = 120;
        return (borg_attack_aux_staff_dispel(
            sv_staff_holiness, rad, dam, BORG_ATTACK_DISP_EVIL));

    /* Artifact -- Narthanc- fire bolt 9d8*/
    case BF_ACT_FIRE_BOLT:
        rad = 0;
        dam = (9 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_fire_bolt, rad, dam, BORG_ATTACK_FIRE, true, -1));

    /* Artifact -- Anduril & Firestar- fire bolt 72*/
    case BF_ACT_FIRE_BOLT72:
        rad = 0;
        dam = 72;
        return (borg_attack_aux_activation(
            act_fire_bolt72, rad, dam, BORG_ATTACK_FIRE, true, -1));

    /* Artifact -- Gothmog- FIRE BALL 144 */
    case BF_ACT_FIRE_BALL:
        rad = 2;
        dam = 144;
        return (borg_attack_aux_activation(
            act_fire_ball, rad, dam, BORG_ATTACK_FIRE, true, -1));

    /* Artifact -- Nimthanc & Paurnimmen- frost bolt 6d8*/
    case BF_ACT_COLD_BOLT:
        rad = 0;
        dam = (6 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_cold_bolt, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Belangil- frost ball 50 */
    case BF_ACT_COLD_BALL50:
        rad = 2;
        dam = 50;
        return (borg_attack_aux_activation(
            act_cold_ball50, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Aranrúth- frost bolt 12d8*/
    case BF_ACT_COLD_BOLT2:
        rad = 0;
        dam = (12 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_cold_bolt2, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Ringil- frost ball 100*/
    case BF_ACT_COLD_BALL100:
        rad = 2;
        dam = 100;
        return (borg_attack_aux_activation(
            act_cold_ball100, rad, dam, BORG_ATTACK_COLD, true, -1));

    /* Artifact -- Dethanc- electric bolt 6d6*/
    case BF_ACT_ELEC_BOLT:
        rad = -1;
        dam = (6 * (6 + 1) / 2);
        return (borg_attack_aux_activation(
            act_elec_bolt, rad, dam, BORG_ATTACK_ELEC, true, -1));

    /* Artifact -- Rilia- poison gas 12*/
    case BF_ACT_STINKING_CLOUD:
        rad = 2;
        dam = 12;
        return (borg_attack_aux_activation(
            act_stinking_cloud, rad, dam, BORG_ATTACK_POIS, true, -1));

    /* Artifact -- Theoden- drain Life 120*/
    case BF_ACT_DRAIN_LIFE2:
        rad = 0;
        dam = 120;
        return (borg_attack_aux_activation(
            act_drain_life2, rad, dam, BORG_ATTACK_OLD_DRAIN, true, -1));

    /* Artifact -- Totila- confustion */
    case BF_ACT_CONFUSE2:
        rad = 0;
        dam = 20;
        return (borg_attack_aux_activation(
            act_confuse2, rad, dam, BORG_ATTACK_OLD_CONF, true, -1));

    /* Artifact -- Holcolleth -- sleep ii and sanctuary */
    case BF_ACT_SLEEPII:
        dam = 10;
        return (borg_attack_aux_artifact_holcolleth());

    /* Artifact -- TURMIL- drain life 90 */
    case BF_ACT_DRAIN_LIFE1:
        rad = 0;
        dam = 90;
        return (borg_attack_aux_activation(
            act_drain_life1, rad, dam, BORG_ATTACK_OLD_DRAIN, true, -1));

    /* Artifact -- Fingolfin- spikes 150 */
    case BF_ACT_ARROW:
        rad = 0;
        dam = 150;
        return (borg_attack_aux_activation(
            act_arrow, rad, dam, BORG_ATTACK_MISSILE, true, -1));

    /* Artifact -- Cammithrim- Magic Missile 3d4 */
    case BF_ACT_MISSILE:
        rad = 0;
        dam = (3 * (4 + 1) / 2);
        return (borg_attack_aux_activation(
            act_missile, rad, dam, BORG_ATTACK_MISSILE, true, -1));

    /* Artifact -- Paurnen- ACID bolt 5d8 */
    case BF_ACT_ACID_BOLT:
        rad = 0;
        dam = (5 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_acid_bolt, rad, dam, BORG_ATTACK_ACID, true, -1));

    /* Artifact -- INGWE- DISPEL EVIL X5 */
    case BF_ACT_DISPEL_EVIL:
        rad = 10;
        dam = (10 + (borg.trait[BI_CLEVEL] * 5) / 2);
        return (borg_attack_aux_activation(
            act_dispel_evil, rad, dam, BORG_ATTACK_DISP_EVIL, true, -1));

    /* Artifact -- Eöl -- Mana Bolt 12d8 */
    case BF_ACT_MANA_BOLT:
        rad = 0;
        dam = (12 * (8 + 1)) / 2;
        return (borg_attack_aux_activation(
            act_mana_bolt, rad, dam, BORG_ATTACK_MANA, true, -1));

    /* Artifact -- Razorback and Mediator */
    case BF_ACT_STAR_BALL:
        rad = 3;
        dam = 150;
        return (borg_attack_aux_activation(
            act_star_ball, rad, dam, BORG_ATTACK_ELEC, true, -1));

    /* Artifact -- Gil-galad */
    case BF_ACT_STARLIGHT2:
        rad = 7;
        dam = (10 * (8 + 1)) / 2;
        return (borg_attack_aux_activation(
            act_starlight2, rad, dam, BORG_ATTACK_LIGHT, false, -1));

    /* Artifact -- randarts */
    case BF_ACT_STARLIGHT:
        rad = 7;
        dam = (6 * (8 + 1)) / 2;
        return (borg_attack_aux_activation(
            act_starlight, rad, dam, BORG_ATTACK_LIGHT, false, -1));

    case BF_ACT_MON_SLOW:
        rad = 0;
        dam = 20;
        return (borg_attack_aux_activation(
            act_mon_slow, rad, dam, BORG_ATTACK_OLD_SLOW, true, -1));

    case BF_ACT_MON_CONFUSE:
        rad = 0;
        dam = 2 * (6 + 1) / 2;
        return (borg_attack_aux_activation(
            act_mon_confuse, rad, dam, BORG_ATTACK_OLD_CONF, true, -1));

    case BF_ACT_SLEEP_ALL:
        rad = 0;
        dam = 60;
        return (borg_attack_aux_activation(
            act_sleep_all, rad, dam, BORG_ATTACK_OLD_SLEEP, false, -1));

    case BF_ACT_FEAR_MONSTER:
        rad = 0;
        dam = 2 * (6 + 1) / 2;
        return (borg_attack_aux_activation(
            act_mon_scare, rad, dam, BORG_ATTACK_TURN_ALL, true, -1));

    case BF_ACT_LIGHT_BEAM:
        rad = -1;
        dam = (6 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_light_line, rad, dam, BORG_ATTACK_LIGHT_WEAK, true, -1));

    case BF_ACT_DRAIN_LIFE3:
        rad = 0;
        dam = 150;
        return (borg_attack_aux_activation(
            act_drain_life3, rad, dam, BORG_ATTACK_OLD_DRAIN, true, -1));

    case BF_ACT_DRAIN_LIFE4:
        rad = 0;
        dam = 250;
        return (borg_attack_aux_activation(
            act_drain_life4, rad, dam, BORG_ATTACK_OLD_DRAIN, true, -1));

    case BF_ACT_ELEC_BALL:
        rad = 2;
        dam = 64;
        return (borg_attack_aux_activation(
            act_elec_ball, rad, dam, BORG_ATTACK_ELEC, true, -1));

    case BF_ACT_ELEC_BALL2:
        rad = 2;
        dam = 250;
        return (borg_attack_aux_activation(
            act_elec_ball2, rad, dam, BORG_ATTACK_ELEC, true, -1));

    case BF_ACT_ACID_BOLT2:
        rad = 0;
        dam = (10 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_acid_bolt2, rad, dam, BORG_ATTACK_ACID, true, -1));

    case BF_ACT_ACID_BOLT3:
        rad = 0;
        dam = (12 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_acid_bolt2, rad, dam, BORG_ATTACK_ACID, true, -1));

    case BF_ACT_ACID_BALL:
        rad = 2;
        dam = 120;
        return (borg_attack_aux_activation(
            act_acid_ball, rad, dam, BORG_ATTACK_ACID, true, -1));

    case BF_ACT_COLD_BALL160:
        rad = 2;
        dam = 160;
        return (borg_attack_aux_activation(
            act_cold_ball160, rad, dam, BORG_ATTACK_COLD, true, -1));

    case BF_ACT_COLD_BALL2:
        rad = 2;
        dam = 200;
        return (borg_attack_aux_activation(
            act_cold_ball2, rad, dam, BORG_ATTACK_COLD, true, -1));

    case BF_ACT_FIRE_BALL2:
        rad = 2;
        dam = 120;
        return (borg_attack_aux_activation(
            act_fire_ball2, rad, dam, BORG_ATTACK_FIRE, true, -1));

    case BF_ACT_FIRE_BALL200:
        rad = 2;
        dam = 200;
        return (borg_attack_aux_activation(
            act_fire_ball200, rad, dam, BORG_ATTACK_FIRE, true, -1));

    case BF_ACT_FIRE_BOLT2:
        rad = 0;
        dam = (12 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_fire_bolt2, rad, dam, BORG_ATTACK_FIRE, true, -1));

    case BF_ACT_FIRE_BOLT3:
        rad = 0;
        dam = (16 * (8 + 1) / 2);
        return (borg_attack_aux_activation(
            act_fire_bolt3, rad, dam, BORG_ATTACK_FIRE, true, -1));

    case BF_ACT_DISPEL_EVIL60:
        rad = 10;
        dam = 60;
        return (borg_attack_aux_activation(
            act_dispel_evil60, rad, dam, BORG_ATTACK_DISP_EVIL, false, -1));

    case BF_ACT_DISPEL_UNDEAD:
        rad = 10;
        dam = 60;
        return (borg_attack_aux_activation(
            act_dispel_undead, rad, dam, BORG_ATTACK_DISP_UNDEAD, false, -1));

    case BF_ACT_DISPEL_ALL:
        rad = 10;
        dam = 60;
        return (borg_attack_aux_activation(
            act_dispel_undead, rad, dam, BORG_ATTACK_DISP_ALL, false, -1));

    case BF_ACT_LOSSLOW:
        rad = 10;
        dam = 20;
        return (borg_attack_aux_activation(
            act_losslow, rad, dam, BORG_ATTACK_OLD_SLOW, false, -1));

    case BF_ACT_LOSSLEEP:
        rad = 10;
        dam = 20;
        return (borg_attack_aux_activation(
            act_lossleep, rad, dam, BORG_ATTACK_OLD_SLEEP, false, -1));

    case BF_ACT_LOSCONF:
        rad = 10;
        dam = 5 + ((5 + 1) / 2);
        return (borg_attack_aux_activation(
            act_losconf, rad, dam, BORG_ATTACK_OLD_CONF, false, -1));

    case BF_ACT_WONDER:
        dam = 5 + ((5 + 1) / 2);
        return (borg_attack_aux_activation(
            act_wonder, rad, dam, BORG_ATTACK_MISSILE, true, -1));

    case BF_ACT_STAFF_HOLY:
        if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2)
            dam = 500;
        else
            dam = 120;
        return (borg_attack_aux_activation(
            act_staff_holy, rad, dam, BORG_ATTACK_DISP_EVIL, false, -1));

    case BF_ACT_RING_ACID:
        rad = 2;
        dam = 70;
        return (borg_attack_aux_activation(
            act_ring_acid, rad, dam, BORG_ATTACK_ACID, true, -1));

    case BF_ACT_RING_FIRE:
        rad = 2;
        dam = 80;
        return (borg_attack_aux_activation(
            act_ring_flames, rad, dam, BORG_ATTACK_FIRE, true, -1));

    case BF_ACT_RING_ICE:
        rad = 2;
        dam = 75;
        return (borg_attack_aux_activation(
            act_ring_ice, rad, dam, BORG_ATTACK_ICE, true, -1));

    case BF_ACT_RING_LIGHTNING:
        rad = 2;
        dam = 85;
        return (borg_attack_aux_activation(
            act_ring_lightning, rad, dam, BORG_ATTACK_ELEC, true, -1));

    case BF_ACT_DRAGON_BLUE:
        rad = 2;
        dam = 150;
        return (borg_attack_aux_activation(
            act_dragon_blue, rad, dam, BORG_ATTACK_ELEC, true, -1));

    case BF_ACT_DRAGON_GREEN:
        rad = 2;
        dam = 150;
        return (borg_attack_aux_activation(
            act_dragon_green, rad, dam, BORG_ATTACK_POIS, true, -1));

    case BF_ACT_DRAGON_RED:
        rad = 2;
        dam = 200;
        return (borg_attack_aux_activation(
            act_dragon_red, rad, dam, BORG_ATTACK_FIRE, true, -1));

    case BF_ACT_DRAGON_MULTIHUED: {
        int  value[5];
        int  type[5] = { BORG_ATTACK_ELEC, BORG_ATTACK_COLD, BORG_ATTACK_ACID,
             BORG_ATTACK_POIS, BORG_ATTACK_FIRE };
        int  biggest = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 2;
        dam               = 250;
        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 5; x++)
            value[x] = borg_attack_aux_activation(
                act_dragon_multihued, rad, dam, type[x], true, x);

        for (int x = 1; x < 5; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_activation(
                act_dragon_multihued, rad, dam, type[biggest], true, biggest);

        return value[biggest];
    }

    case BF_ACT_DRAGON_GOLD:
        rad = 2;
        dam = 150;
        return (borg_attack_aux_activation(
            act_dragon_gold, rad, dam, BORG_ATTACK_SOUND, true, -1));

    case BF_ACT_DRAGON_CHAOS: {
        int  value[2];
        int  type[2]      = { BORG_ATTACK_CHAOS, BORG_ATTACK_DISEN };
        int  biggest      = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 2;
        dam               = 220;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_activation(
                act_dragon_chaos, rad, dam, type[x], true, x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_activation(
                act_dragon_chaos, rad, dam, type[biggest], true, biggest);

        return value[biggest];
    }

    case BF_ACT_DRAGON_LAW: {
        int  value[2];
        int  type[2]      = { BORG_ATTACK_SOUND, BORG_ATTACK_SHARD };
        int  biggest      = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 2;
        dam               = 220;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_activation(
                act_dragon_law, rad, dam, type[x], true, x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_activation(
                act_dragon_law, rad, dam, type[biggest], true, biggest);

        return value[biggest];
    }

    case BF_ACT_DRAGON_BALANCE: {
        int value[4];
        int type[4] = { BORG_ATTACK_CHAOS, BORG_ATTACK_DISEN, BORG_ATTACK_SOUND,
            BORG_ATTACK_SHARD };
        int biggest = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 2;
        dam               = 250;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 4; x++)
            value[x] = borg_attack_aux_activation(
                act_dragon_balance, rad, dam, type[x], true, x);

        for (int x = 1; x < 4; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_activation(
                act_dragon_balance, rad, dam, type[biggest], true, biggest);

        return value[biggest];
    }

    case BF_ACT_DRAGON_SHINING: {
        int  value[2];
        int  type[2]      = { BORG_ATTACK_LIGHT, BORG_ATTACK_DARK };
        int  biggest      = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 2;
        dam               = 200;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_activation(
                act_dragon_shining, rad, dam, type[x], true, x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_activation(
                act_dragon_shining, rad, dam, type[biggest], true, biggest);

        return value[biggest];
    }

    case BF_ACT_DRAGON_POWER:
        rad = 2;
        dam = 300;
        return (borg_attack_aux_activation(
            act_dragon_power, rad, dam, BORG_ATTACK_MISSILE, true, -1));

    /* Ring of ACID */
    case BF_RING_ACID:
        rad = 2;
        dam = 70;
        return (borg_attack_aux_ring(sv_ring_acid, rad, dam, BORG_ATTACK_ACID));

    /* Ring of FLAMES */
    case BF_RING_FIRE:
        rad = 2;
        dam = 80;
        return (
            borg_attack_aux_ring(sv_ring_flames, rad, dam, BORG_ATTACK_FIRE));

    /* Ring of ICE */
    case BF_RING_ICE:
        rad = 2;
        dam = 75;
        return (borg_attack_aux_ring(sv_ring_ice, rad, dam, BORG_ATTACK_ICE));

    /* Ring of LIGHTNING */
    case BF_RING_LIGHTNING:
        rad = 2;
        dam = 85;
        return (borg_attack_aux_ring(
            sv_ring_lightning, rad, dam, BORG_ATTACK_ELEC));

    /* Hack -- Dragon Scale Mail can be activated as well */
    case BF_DRAGON_BLUE:
        rad = 20;
        dam = 150;
        return (borg_attack_aux_dragon(
            sv_dragon_blue, rad, dam, BORG_ATTACK_ELEC, -1));

    case BF_DRAGON_WHITE:
        rad = 20;
        dam = 100;
        return (borg_attack_aux_dragon(
            sv_dragon_white, rad, dam, BORG_ATTACK_COLD, -1));

    case BF_DRAGON_BLACK:
        rad = 20;
        dam = 120;
        return (borg_attack_aux_dragon(
            sv_dragon_black, rad, dam, BORG_ATTACK_ACID, -1));

    case BF_DRAGON_GREEN:
        rad = 20;
        dam = 150;
        return (borg_attack_aux_dragon(
            sv_dragon_green, rad, dam, BORG_ATTACK_POIS, -1));

    case BF_DRAGON_RED:
        rad = 2;
        dam = 200;
        return (borg_attack_aux_dragon(
            sv_dragon_red, rad, dam, BORG_ATTACK_FIRE, -1));

    case BF_DRAGON_MULTIHUED: {
        int  value[5];
        int  type[5] = { BORG_ATTACK_ELEC, BORG_ATTACK_COLD, BORG_ATTACK_ACID,
             BORG_ATTACK_POIS, BORG_ATTACK_FIRE };
        int  biggest = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 20;
        dam               = 250;
        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 5; x++)
            value[x] = borg_attack_aux_dragon(
                sv_dragon_multihued, rad, dam, type[x], x);

        for (int x = 1; x < 5; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(
                sv_dragon_multihued, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_GOLD:
        rad = 20;
        dam = 150;
        return (borg_attack_aux_dragon(
            sv_dragon_gold, rad, dam, BORG_ATTACK_SOUND, -1));

    case BF_DRAGON_CHAOS: {
        int  value[2];
        int  type[2]      = { BORG_ATTACK_CHAOS, BORG_ATTACK_DISEN };
        int  biggest      = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 20;
        dam               = 220;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x]
                = borg_attack_aux_dragon(sv_dragon_chaos, rad, dam, type[x], x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(
                sv_dragon_chaos, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_LAW: {
        int  value[2];
        int  type[2]      = { BORG_ATTACK_SOUND, BORG_ATTACK_SHARD };
        int  biggest      = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 20;
        dam               = 220;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x]
                = borg_attack_aux_dragon(sv_dragon_law, rad, dam, type[x], x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(
                sv_dragon_law, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_BALANCE: {
        int value[4];
        int type[4] = { BORG_ATTACK_CHAOS, BORG_ATTACK_DISEN, BORG_ATTACK_SOUND,
            BORG_ATTACK_SHARD };
        int biggest = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 20;
        dam               = 250;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 4; x++)
            value[x] = borg_attack_aux_dragon(
                sv_dragon_balance, rad, dam, type[x], x);

        for (int x = 1; x < 4; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(
                sv_dragon_balance, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_SHINING: {
        int  value[2];
        int  type[2]      = { BORG_ATTACK_LIGHT, BORG_ATTACK_DARK };
        int  biggest      = 0;
        bool tmp_simulate = borg_simulate;

        rad               = 20;
        dam               = 200;

        if (!borg_simulate)
            borg_simulate = true;
        for (int x = 0; x < 2; x++)
            value[x] = borg_attack_aux_dragon(
                sv_dragon_shining, rad, dam, type[x], x);

        for (int x = 1; x < 2; x++)
            if (value[x] > value[biggest])
                biggest = x;

        borg_simulate = tmp_simulate;
        if (!borg_simulate)
            value[biggest] = borg_attack_aux_dragon(
                sv_dragon_shining, rad, dam, type[biggest], biggest);

        return value[biggest];
    }

    case BF_DRAGON_POWER:
        rad = 20;
        dam = 300;
        return (borg_attack_aux_dragon(
            sv_dragon_power, rad, dam, BORG_ATTACK_MISSILE, -1));
    }

    /* Oops */
    return 0;
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
    int i;

    int  n, b_n = 0;
    int  g, b_g = -1;
    bool adjacent_monster = false;

    borg_grid           *ag;
    struct monster_race *r_ptr;

    /* Nobody around */
    if (!borg_kills_cnt)
        return false;

    /* Set the attacking flag so that danger is boosted for monsters */
    /* we want to attack first. */
    borg_attacking = true;

    /* Reset list */
    borg_temp_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill  = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Ignore multiplying monsters and when fleeing from scaries*/
        if (borg.goal.ignoring && !borg.trait[BI_ISAFRAID]
            && (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY)))
            continue;

        /* Low level mages need to conserve the mana in town. These guys don't
         * fight back */
        if (borg.trait[BI_CLASS] == CLASS_MAGE && borg.trait[BI_MAXCLEVEL] < 10
            && borg.trait[BI_CDEPTH] == 0
            && (strstr(r_ptr->name, "Farmer")
                /* strstr(r_ptr->name, "Blubbering") || */
                /* strstr(r_ptr->name, "Boil") || */
                /* strstr(r_ptr->name, "Village") || */
                /*strstr(r_ptr->name, "Pitiful") || */
                /* strstr(r_ptr->name, "Mangy") */))
            continue;

        /* Check if there is a monster adjacent to me or he's close and fast. */
        if ((kill->speed > borg.trait[BI_SPEED]
                && distance(borg.c, kill->pos) <= 2)
            || distance(borg.c, kill->pos) <= 1)
            adjacent_monster = true;

        /* no attacking most scaryguys, try to get off the level */
        if (scaryguy_on_level) {
            /* probably Grip or Fang. */
            if (strstr(r_ptr->name, "Grip") || strstr(r_ptr->name, "Fang")) {
                /* Try to fight Grip and Fang. */
            } else if (borg.trait[BI_CDEPTH] <= 5 && borg.trait[BI_CDEPTH] != 0
                       && (rf_has(r_info[kill->r_idx].flags, RF_MULTIPLY))) {
                /* Try to fight single worms and mice. */
            } else if (borg_t - borg_began >= 2000
                       || borg_time_town + (borg_t - borg_began) >= 3000) {
                /* Try to fight been there too long. */
            } else if (boosted_bravery || borg.no_retreat >= 1
                       || borg.goal.recalling) {
                /* Try to fight if being Boosted or recall engaged. */
                borg_note("# Bored, or recalling and fighting a monster on "
                          "Scaryguy Level.");
            } else if (borg.trait[BI_CDEPTH] * 4 <= borg.trait[BI_CLEVEL]
                       && borg.trait[BI_CLEVEL] > 10) {
                /* Try to fight anyway. */
                borg_note("# High clevel fighting monster on Scaryguy Level.");
            } else if (adjacent_monster) {
                /* Try to fight if there is a monster next to me */
                borg_note("# Adjacent to monster on Scaryguy Level.");
            } else {
                /* Flee from other scary guys */
                continue;
            }
        }

        /* Get grid */
        ag = &borg_grids[kill->pos.y][kill->pos.x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY))
            continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW))
            continue;

        /* Check the distance XXX XXX XXX */
        if (distance(borg.c, kill->pos) > z_info->max_range)
            continue;

        /* Sometimes the borg can lose a monster index in the grid if there are
         * lots of monsters on screen.  If he does lose one, reinject the index
         * here. */
        if (!ag->kill)
            borg_grids[kill->pos.y][kill->pos.x].kill = i;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = kill->pos.x;
        borg_temp_y[borg_temp_n] = kill->pos.y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n) {
        borg_attacking = false;
        return false;
    }

    /* Simulate */
    borg_simulate = true;

    /* some attacks are only possible for random artifacts */
    /* those are lumped at the end so people without random artifacts */
    /* don't look for them. */
    int max_attacks = BF_MAX;
    if (!OPT(player, birth_randarts))
        max_attacks = BF_ACT_STARLIGHT;

    /* Analyze the possible attacks */
    for (g = 0; g < max_attacks; g++) {

        /* Simulate */
        n = borg_calculate_attack_effectiveness(g);

        /* Track "best" attack  <= */
        if (n <= b_n)
            continue;

        /* Track best */
        b_g = g;
        b_n = n;
    }

    /* Nothing good */
    if (b_n <= 0) {
        borg_attacking = false;
        return false;
    }

    /* Note */
    borg_note(format("# Performing attack type %d with value %d.", b_g, b_n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_calculate_attack_effectiveness(b_g);

    borg_attacking = false;

    /* Success */
    return true;
}

#endif
