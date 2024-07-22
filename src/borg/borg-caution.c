/**
 * \file borg-caution.c
 * \brief Try to be careful and not die when things are dangerous
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

#include "borg-caution.h"

#ifdef ALLOW_BORG

#include "../player-timed.h"
#include "../ui-event.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-danger.h"
#include "borg-escape.h"
#include "borg-fight-defend.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow-stairs.h"
#include "borg-flow.h"
#include "borg-inventory.h"
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
#include "borg-trait.h"
#include "borg.h"

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
    int clw_heal          = 15;
    int csw_heal          = 25;
    int ccw_heal          = 30;
    int cmw_heal          = 50;

    int heal_heal         = 300;

    int stats_needing_fix = 0;

    bool rod_good         = false;

    hp_down               = borg.trait[BI_MAXHP] - borg.trait[BI_CURHP];
    pct_down              = ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP]) * 100
                / borg.trait[BI_MAXHP]);
    clw_heal  = ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP]) * 15 / 100);
    csw_heal  = ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP]) * 20 / 100);
    ccw_heal  = ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP]) * 25 / 100);
    cmw_heal  = ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP]) * 30 / 100);
    heal_heal = ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP]) * 35 / 100);

    if (clw_heal < 15)
        clw_heal = 15;
    if (csw_heal < 25)
        csw_heal = 25;
    if (ccw_heal < 30)
        ccw_heal = 30;
    if (cmw_heal < 50)
        cmw_heal = 50;
    if (heal_heal < 300)
        heal_heal = 300;

    /* Quick check for rod success (used later on) */
    if (borg_slot(TV_ROD, sv_rod_healing) != -1) {
        /* Reasonable chance of success */
        if (borg_activate_failure(TV_ROD, sv_rod_healing) < 500)
            rod_good = true;
    }

    /* when fighting Morgoth, we want the borg to use Life potion to fix his
     * stats.  So we need to add up the ones that are dropped.
     */
    if (borg.trait[BI_ISFIXSTR])
        stats_needing_fix++;
    if (borg.trait[BI_ISFIXINT])
        stats_needing_fix++;
    if (borg.trait[BI_ISFIXWIS])
        stats_needing_fix++;
    if (borg.trait[BI_ISFIXDEX])
        stats_needing_fix++;
    if (borg.trait[BI_ISFIXCON])
        stats_needing_fix++;

    /* Special cases get a second vote */
    if (borg.trait[BI_CLASS] == CLASS_MAGE && borg.trait[BI_ISFIXINT])
        stats_needing_fix++;
    if (borg.trait[BI_CLASS] == CLASS_PRIEST && borg.trait[BI_ISFIXWIS])
        stats_needing_fix++;
    if (borg.trait[BI_CLASS] == CLASS_DRUID && borg.trait[BI_ISFIXWIS])
        stats_needing_fix++;
    if (borg.trait[BI_CLASS] == CLASS_NECROMANCER && borg.trait[BI_ISFIXINT])
        stats_needing_fix++;
    if (borg.trait[BI_CLASS] == CLASS_WARRIOR && borg.trait[BI_ISFIXCON])
        stats_needing_fix++;
    if (borg.trait[BI_MAXHP] <= 850 && borg.trait[BI_ISFIXCON])
        stats_needing_fix++;
    if (borg.trait[BI_MAXHP] <= 700 && borg.trait[BI_ISFIXCON])
        stats_needing_fix += 3;
    if (borg.trait[BI_CLASS] == CLASS_PRIEST && borg.trait[BI_MAXSP] < 100
        && borg.trait[BI_ISFIXWIS])
        stats_needing_fix += 5;
    if (borg.trait[BI_CLASS] == CLASS_MAGE && borg.trait[BI_MAXSP] < 100
        && borg.trait[BI_ISFIXINT])
        stats_needing_fix += 5;

    /*  Hack -- heal when confused. This is deadly.*/
    /* This is checked twice, once, here, to see if he is in low danger
     * and again at the end of borg_caution, when all other avenues have failed
     */
    if (borg.trait[BI_ISCONFUSED]) {
        if ((pct_down >= 80) && danger - heal_heal < borg.trait[BI_CURHP]
            && borg_quaff_potion(sv_potion_healing)) {
            borg_note("# Fixing Confusion. Level 1");
            return true;
        }
        if ((pct_down >= 85) && danger >= borg.trait[BI_CURHP] * 2
            && (borg_quaff_potion(sv_potion_star_healing)
                || borg_quaff_potion(sv_potion_life))) {
            borg_note("# Fixing Confusion. Level 1.a");
            return true;
        }
        if (danger < borg.trait[BI_CURHP] + csw_heal
            && (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)
                || borg_quaff_potion(sv_potion_cure_serious)
                || borg_quaff_crit(false)
                || borg_quaff_potion(sv_potion_healing)
                || borg_use_staff_fail(sv_staff_healing)
                || borg_activate_item(act_cure_confusion)
                || borg_use_staff_fail(sv_staff_curing))) {
            borg_note("# Fixing Confusion. Level 2");
            return true;
        }

        /* If my ability to use a teleport staff is really
         * bad, then I should heal up then use the staff.
         */
        /* Check for a charged teleport staff */
        if (borg_equips_staff_fail(sv_staff_teleportation)) {
            /* check my skill, drink a potion */
            if ((borg_activate_failure(TV_STAFF, sv_staff_teleportation) > 650)
                && (danger < (avoidance + ccw_heal) * 15 / 10)
                && (borg_quaff_crit(true)
                    || borg_quaff_potion(sv_potion_healing))) {
                borg_note("# Fixing Confusion. Level 3");
                return true;
            }
            /* However, if I am in really big trouble and there is no way
             * I am going to be able to
             * survive another round, take my chances on the staff.
             */
            else if (danger > avoidance * 2) {
                borg_note("# Too scary to fix Confusion. Level 4");
                return false;
            }

        } else {
            /* If I do not have a staff to teleport, take the potion
             * and try to fix the confusion
             */
            if ((borg_quaff_crit(true)
                    || borg_quaff_potion(sv_potion_cure_serious)
                    || borg_quaff_potion(sv_potion_healing))) {
                borg_note("# Fixing Confusion. Level 5");
                return true;
            }
        }
    }
    /*  Hack -- heal when blind. This is deadly.*/
    if (borg.trait[BI_ISBLIND] && (randint0(100) < 85)) {
        /* if in extreme danger, use teleport then fix the
         * blindness later.
         */
        if (danger > avoidance * 25 / 10) {
            /* Check for a charged teleport staff */
            if (borg_equips_staff_fail(sv_staff_teleportation))
                return 0;
        }
        if ((hp_down >= 300) && borg_quaff_potion(sv_potion_healing)) {
            return true;
        }
        /* Warriors with ESP won't need it so quickly */
        if (!(borg.trait[BI_CLASS] == CLASS_WARRIOR
                && borg.trait[BI_CURHP] > borg.trait[BI_MAXHP] / 4
                && borg.trait[BI_ESP])) {
            if (borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
                || borg_quaff_potion(sv_potion_cure_light)
                || borg_quaff_potion(sv_potion_cure_serious)
                || borg_quaff_crit(true)
                || borg_use_staff_fail(sv_staff_healing)
                || borg_use_staff_fail(sv_staff_curing)
                || borg_quaff_potion(sv_potion_healing)) {
                borg_note("# Fixing Blindness.");
                return true;
            }
        }
    }

    /* We generally try to conserve ez-heal pots */
    if ((borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        && ((hp_down >= 400)
            || (danger > borg.trait[BI_CURHP] * 5 && hp_down > 100))
        && borg_quaff_potion(sv_potion_star_healing)) {
        borg_note("# Fixing Confusion/Blind.");
        return true;
    }

    /* Healing and fighting Morgoth. */
    if (borg_fighting_unique >= 10) {
        if (borg.trait[BI_CURHP] <= 700
            && ((borg.trait[BI_CURHP] > 250 && borg_spell_fail(HOLY_WORD, 14))
                || /* Holy Word */
                /* Choose Life over *Healing* to fix stats*/
                (stats_needing_fix >= 5 && borg_quaff_potion(sv_potion_life)) ||
                /* Choose Life over Healing if way down on pts*/
                (hp_down > 500
                    && borg.has[borg_lookup_kind(
                           TV_POTION, sv_potion_star_healing)]
                           <= 0
                    && borg_quaff_potion(sv_potion_life))
                || borg_quaff_potion(sv_potion_star_healing)
                || borg_quaff_potion(sv_potion_healing)
                || borg_activate_item(act_heal1)
                || borg_activate_item(act_heal2)
                || borg_activate_item(act_heal3)
                || (borg.trait[BI_CURHP] < 250 && borg_spell_fail(HOLY_WORD, 5))
                || /* Holy Word */
                (borg.trait[BI_CURHP] > 550 && borg_spell_fail(HOLY_WORD, 15))
                || /* Holy Word */
                borg_spell_fail(HEALING, 15)
                || borg_quaff_potion(sv_potion_life)
                || borg_zap_rod(sv_rod_healing))) {
            borg_note("# Healing in Questor Combat.");
            return true;
        }
    }

    /* restore Mana */
    /* note, blow the staff charges easy because the staff will not last. */
    if (borg.trait[BI_CURSP] < (borg.trait[BI_MAXSP] / 5)
        && (randint0(100) < 50)) {
        if (borg_use_staff_fail(sv_staff_the_magi)
            || borg_activate_item(act_staff_magi)) {
            borg_note("# Use Magi Staff");
            return true;
        }
    }
    /* blowing potions is harder */
    /* NOTE: must have enough mana to keep up or do a HEAL */
    if (borg.trait[BI_CURSP] < (borg.trait[BI_MAXSP] / 10)
        || ((borg.trait[BI_CURSP] < 70 && borg.trait[BI_MAXSP] > 200))) {
        /*  use the potion if battling a unique and not too dangerous */
        if (borg_fighting_unique >= 10
            || (borg_fighting_unique && danger < avoidance * 2)
            || (borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] == 0
                && danger > avoidance)) {
            if (borg_use_staff_fail(sv_staff_the_magi)
                || borg_quaff_potion(sv_potion_restore_mana)
                || borg_activate_item(act_restore_mana)
                || borg_activate_item(act_staff_magi)) {
                borg_note("# Restored My Mana");
                return true;
            }
        }
    }

    /* if unhurt no healing needed */
    if (hp_down == 0)
        return false;

    /* Don't bother healing if not in danger */
    if (danger == 0 && !borg.trait[BI_ISPOISONED] && !borg.trait[BI_ISCUT])
        return false;

    /* Restoring while fighting Morgoth */
    if (stats_needing_fix >= 5 && borg_fighting_unique >= 10
        && borg.trait[BI_CURHP] > 650
        && (borg_eat(TV_MUSHROOM, sv_mush_restoring)
            || borg_activate_item(act_restore_all))) {
        borg_note("# Trying to fix stats in combat.");
        return true;
    }

    /* No further Healing considerations if fighting Questors */
    if (borg_fighting_unique >= 10) {
        /* No further healing considerations right now */
        return false;
    }

    /* Hack -- heal when wounded a percent of the time */
    chance = randint0(100);

    /* if we are fighting a unique increase the odds of healing */
    if (borg_fighting_unique)
        chance -= 10;

    /* if danger is close to the hp and healing will help, do it */
    if (danger >= borg.trait[BI_CURHP] && danger < borg.trait[BI_MAXHP])
        chance -= 75;
    else {
        if (borg.trait[BI_CLASS] != CLASS_PRIEST
            && borg.trait[BI_CLASS] != CLASS_PALADIN)
            chance -= 25;
    }

    /* Risky Borgs are less likely to heal in the fight */
    if (borg_cfg[BORG_PLAYS_RISKY])
        chance += 5;

    if (((pct_down <= 15 && chance < 98)
            || (pct_down >= 16 && pct_down <= 25 && chance < 95)
            || (pct_down >= 26 && pct_down <= 50 && chance < 80)
            || (pct_down >= 51 && pct_down <= 65 && chance < 50)
            || (pct_down >= 66 && pct_down <= 74 && chance < 25)
            || (pct_down >= 75 && chance < 1))
        && (!borg.trait[BI_ISHEAVYSTUN] && !borg.trait[BI_ISSTUN]
            && !borg.trait[BI_ISPOISONED] && !borg.trait[BI_ISCUT]))
        return false;

    /* Cure light Wounds (2d10) */
    if (pct_down >= 30 && (pct_down <= 40 || borg.trait[BI_CLEVEL] < 10)
        && ((danger) < borg.trait[BI_CURHP] + clw_heal)
        && (clw_heal > danger / 3) && /* No rope-a-doping */
        (borg_spell_fail(MINOR_HEALING, allow_fail)
            || borg_quaff_potion(sv_potion_cure_light)
            || borg_activate_item(act_cure_light))) {
        borg_note("# Healing Level 1.");
        return true;
    }
    /* Cure Serious Wounds (4d10) */
    if (pct_down >= 40 && (pct_down <= 50 || borg.trait[BI_CLEVEL] < 20)
        && ((danger) < borg.trait[BI_CURHP] + csw_heal)
        && (csw_heal > danger / 3) && /* No rope-a-doping */
        (borg_quaff_potion(sv_potion_cure_serious)
            || borg_activate_item(act_cure_serious))) {
        borg_note("# Healing Level 2.");
        return true;
    }

    /* Cure Critical Wounds (6d10) */
    if (pct_down >= 50 && pct_down <= 55
        && ((danger) < borg.trait[BI_CURHP] + ccw_heal)
        && (ccw_heal > danger / 3) && /* No rope-a-doping */
        (borg_activate_item(act_cure_critical) || borg_quaff_crit(false))) {
        borg_note("# Healing Level 3.");
        return true;
    }

    /* If in danger try  one more Cure Critical if it will help */
    if (danger >= borg.trait[BI_CURHP] && danger < borg.trait[BI_MAXHP]
        && borg.trait[BI_CURHP] < 50 && danger < ccw_heal
        && borg_quaff_crit(true)) {
        borg_note("# Healing Level 5.");
        return true;
    }

    /* if deep, and low on HP, but in a zero danger spot, drink some CCW to add
     * a few HP before resting */
    if (borg.trait[BI_CDEPTH] >= 80 && danger < 50 && pct_down >= 20
        && borg_quaff_potion(sv_potion_cure_critical)) {
        borg_note("# Healing Level 5B.");
        return true;
    }

    /* Heal step one (200hp) */
    if (pct_down >= 55 && danger < borg.trait[BI_CURHP] + heal_heal
        && ((((!borg.trait[BI_ATELEPORT] && !borg.trait[BI_AESCAPE])
                 || rod_good)
                && borg_zap_rod(sv_rod_healing))
            || borg_activate_item(act_cure_full)
            || borg_activate_item(act_cure_full2)
            || borg_activate_item(act_cure_nonorlybig)
            || borg_activate_item(act_heal1) || borg_activate_item(act_heal2)
            || borg_activate_item(act_heal3)
            || borg_use_staff_fail(sv_staff_healing)
            || borg_spell_fail(HEALING, allow_fail))) {
        borg_note("# Healing Level 6.");
        return true;
    }

    /* Generally continue to heal.  But if we are preparing for the end
     * game uniques, then bail out here in order to save our heal pots.
     * (unless morgoth is dead)
     * Priests wont need to bail, they have good heal spells.
     */
    if (borg.trait[BI_MAXDEPTH] >= 98 && !borg.trait[BI_KING]
        && !borg_fighting_unique && borg.trait[BI_CLASS] != CLASS_PRIEST) {
        /* Bail out to save the heal pots for Morgoth*/
        return false;
    }

    /* Heal step two (300hp) */
    if (pct_down > 50 && danger < borg.trait[BI_CURHP] + heal_heal
        && (borg_use_staff_fail(sv_staff_healing)
            || (borg_fighting_evil_unique
                && borg_spell_fail(HOLY_WORD, allow_fail))
            || /* holy word */
            borg_spell_fail(HEALING, allow_fail)
            || (((!borg.trait[BI_ATELEPORT] && !borg.trait[BI_AESCAPE])
                    || rod_good)
                && borg_zap_rod(sv_rod_healing))
            || borg_zap_rod(sv_rod_healing)
            || borg_quaff_potion(sv_potion_healing))) {
        borg_note("# Healing Level 7.");
        return true;
    }

    /* Healing step three (300hp).  */
    if (pct_down > 60 && danger < borg.trait[BI_CURHP] + heal_heal
        && ((borg_fighting_evil_unique
                && borg_spell_fail(HOLY_WORD, allow_fail))
            || /* holy word */
            (((!borg.trait[BI_ATELEPORT] && !borg.trait[BI_AESCAPE])
                 || rod_good)
                && borg_zap_rod(sv_rod_healing))
            || borg_spell_fail(HEALING, allow_fail)
            || borg_use_staff_fail(sv_staff_healing)
            || borg_quaff_potion(sv_potion_healing)
            || borg_activate_item(act_cure_full)
            || borg_activate_item(act_cure_full2)
            || borg_activate_item(act_cure_nonorlybig)
            || borg_activate_item(act_heal1) || borg_activate_item(act_heal2)
            || borg_activate_item(act_heal3))) {
        borg_note("# Healing Level 8.");
        return true;
    }

    /* Healing.  First use of EZ_Heals
     */
    if (pct_down > 65 && (danger < borg.trait[BI_CURHP] + heal_heal)
        && ((borg_fighting_evil_unique
                && borg_spell_fail(HOLY_WORD, allow_fail))
            || /* holy word */
            borg_spell_fail(HEALING, allow_fail)
            || borg_use_staff_fail(sv_staff_healing)
            || (((!borg.trait[BI_ATELEPORT] && !borg.trait[BI_AESCAPE])
                    || rod_good)
                && borg_zap_rod(sv_rod_healing))
            || borg_quaff_potion(sv_potion_healing)
            || borg_activate_item(act_cure_full)
            || borg_activate_item(act_cure_full2)
            || borg_activate_item(act_cure_nonorlybig)
            || borg_activate_item(act_heal1) || borg_activate_item(act_heal2)
            || borg_activate_item(act_heal3)
            || (borg_fighting_unique
                && (borg_quaff_potion(sv_potion_star_healing)
                    || borg_quaff_potion(sv_potion_healing)
                    || borg_quaff_potion(sv_potion_life))))) {
        borg_note("# Healing Level 9.");
        return true;
    }

    /* Healing final check.  Note that *heal* and Life potions are not
     * wasted.  They are saved for Morgoth and emergencies.  The
     * Emergency check is at the end of borg_caution() which is after the
     * borg_escape() routine.
     */
    if (pct_down > 75 && danger > borg.trait[BI_CURHP]
        && borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] <= 0
        && (borg_quaff_potion(sv_potion_healing)
            || borg_quaff_potion(sv_potion_star_healing)
            || borg_quaff_potion(sv_potion_life))) {
        borg_note("# Healing Level 10.");
        return true;
    }

    /*** Cures ***/

    /* Dont do these in the middle of a fight, teleport out then try it */
    if (danger > avoidance * 2 / 10)
        return false;

    /* Hack -- cure poison when poisoned
     * This was moved from borg_caution.
     */
    if (borg.trait[BI_ISPOISONED]
        && (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2)) {
        if (borg_spell_fail(CURE_POISON, 60)
            || borg_spell_fail(HERBAL_CURING, 60)
            || borg_quaff_potion(sv_potion_cure_poison)
            || borg_activate_item(act_cure_body)
            || borg_activate_item(act_cure_critical)
            || borg_activate_item(act_cure_temp)
            || borg_activate_item(act_rem_fear_pois)
            || borg_activate_item(act_cure_full)
            || borg_activate_item(act_cure_full2)
            || borg_activate_item(act_cure_nonorlybig)
            || borg_use_staff(sv_staff_curing)
            || borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
            || borg_eat(TV_MUSHROOM, sv_mush_purging)
            || borg_activate_item(act_shroom_purging) ||
            /* buy time */
            borg_quaff_crit(true) || borg_spell_fail(HEALING, 60)
            || borg_spell_fail(HOLY_WORD, 60)
            || borg_use_staff_fail(sv_staff_healing)) {
            borg_note("# Curing.");
            return true;
        }

        /* attempt to fix mana then poison on next round */
        if ((borg_spell_legal(CURE_POISON) || borg_spell_legal(HERBAL_CURING))
            && (borg_quaff_potion(sv_potion_restore_mana)
                || borg_activate_item(act_restore_mana))) {
            borg_note("# Curing next round.");
            return true;
        }
    }

    /* Hack -- cure poison when poisoned CRITICAL CHECK
     */
    if (borg.trait[BI_ISPOISONED]
        && (borg.trait[BI_CURHP] < 2
            || borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 20)) {
        int sv_mana          = borg.trait[BI_CURSP];

        borg.trait[BI_CURSP] = borg.trait[BI_MAXSP];

        if (borg_spell(CURE_POISON) || borg_spell(HERBAL_CURING)
            || borg_spell(HOLY_WORD) || borg_spell(HEALING)) {
            /* verify use of spell */
            /* borg_keypress('y'); */

            /* Flee! */
            borg_note("# Emergency Cure Poison! Gasp!!!....");

            return true;
        }
        borg.trait[BI_CURSP] = sv_mana;

        /* Quaff healing pots to buy some time- in this emergency.  */
        if (borg_quaff_potion(sv_potion_cure_light)
            || borg_quaff_potion(sv_potion_cure_serious))
            return true;

        /* Try to Restore Mana */
        if (borg_quaff_potion(sv_potion_restore_mana)
            || borg_activate_item(act_restore_mana))
            return true;

        /* Emergency check on healing.  Borg_heal has already been checked but
         * but we did not use our ez_heal potions.  All other attempts to save
         * ourself have failed.  Use the ez_heal if I have it.
         */
        if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 20
            && (borg_quaff_potion(sv_potion_star_healing)
                || borg_quaff_potion(sv_potion_life)
                || borg_quaff_potion(sv_potion_healing))) {
            borg_note("# Healing. Curing section.");
            return true;
        }

        /* Quaff unknown potions in this emergency.  We might get luck */
        if (borg_quaff_unknown())
            return true;

        /* Eat unknown mushroom in this emergency.  We might get luck */
        if (borg_eat_unknown())
            return true;

        /* Use unknown Staff in this emergency.  We might get luck */
        if (borg_use_unknown())
            return true;
    }

    /* Hack -- cure wounds when bleeding, also critical check */
    if (borg.trait[BI_ISCUT]
        && (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 3
            || randint0(100) < 20)) {
        if (borg_quaff_potion(sv_potion_cure_serious)
            || borg_quaff_potion(sv_potion_cure_light)
            || borg_quaff_crit(borg.trait[BI_CURHP] < 10)
            || borg_spell(MINOR_HEALING)
            || borg_quaff_potion(sv_potion_cure_critical)) {
            return true;
        }
    }
    /* bleeding and about to die CRITICAL CHECK*/
    if (borg.trait[BI_ISCUT]
        && ((borg.trait[BI_CURHP] < 2)
            || borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 20)) {
        int sv_mana          = borg.trait[BI_CURSP];

        borg.trait[BI_CURSP] = borg.trait[BI_MAXSP];

        /* Quaff healing pots to buy some time- in this emergency.  */
        if (borg_quaff_potion(sv_potion_cure_light)
            || borg_quaff_potion(sv_potion_cure_serious))
            return true;

        /* Try to Restore Mana */
        if (borg_quaff_potion(sv_potion_restore_mana)
            || borg_activate_item(act_restore_mana))
            return true;

        /* Emergency check on healing.  Borg_heal has already been checked but
         * but we did not use our ez_heal potions.  All other attempts to save
         * ourselves have failed.  Use the ez_heal if I have it.
         */
        if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 20
            && (borg_quaff_potion(sv_potion_healing)
                || borg_quaff_potion(sv_potion_star_healing)
                || borg_quaff_potion(sv_potion_life))) {
            borg_note("# Healing.  Bleeding.");
            return true;
        }

        /* Cast a spell, go into negative mana */
        if (borg_spell(MINOR_HEALING)) {
            /* verify use of spell */
            /* borg_keypress('y'); */

            /* Flee! */
            borg_note("# Emergency Wound Patch! Gasp!!!....");

            return true;
        }
        borg.trait[BI_CURSP] = sv_mana;

        /* Quaff unknown potions in this emergency.  We might get luck */
        if (borg_quaff_unknown())
            return true;

        /* Eat unknown mushroom in this emergency.  We might get luck */
        if (borg_eat_unknown())
            return true;

        /* Use unknown Staff in this emergency.  We might get luck */
        if (borg_use_unknown())
            return true;
    }

    /* nothing to do */
    return false;
}

/*
 * Be "cautious" and attempt to prevent death or dishonor.
 *
 * Strategy:
 *
 *   (1) Caution
 *   (1a) Analyze the situation
 *   (1a1) try to heal
 *   (1a2) try a defense
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
 * could die in a single round, we set the "goal.fleeing" flag, and if
 * we could die in two rounds, we set the "goal.leaving" flag.
 *
 * In town, whenever we could die in two rounds if we were to stay still,
 * we set the "goal.leaving" flag.  In combination with the "retreat" and
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
    int  j, pos_danger;
    bool borg_surround = false;
    bool nasty         = false;
    bool on_dnstair    = false;
    bool on_upstair    = false;

    /*** Notice "nasty" situations ***/

    /* About to run out of light is extremely nasty */
    if (!borg.trait[BI_LIGHT] && borg_items[INVEN_LIGHT].timeout < 250)
        nasty = true;

    /* Starvation is nasty */
    if (borg.trait[BI_ISWEAK])
        nasty = true;

    /* Blind-ness is nasty */
    if (borg.trait[BI_ISBLIND])
        nasty = true;

    /* Confusion is nasty */
    if (borg.trait[BI_ISCONFUSED])
        nasty = true;

    /* Hallucination is nasty */
    if (borg.trait[BI_ISIMAGE])
        nasty = true;

    /* if on level 100 and not ready for Morgoth, run */
    if (borg.trait[BI_CDEPTH] == 100 && borg_t - borg_began < 10
        && !borg_morgoth_position) {
        if (borg.ready_morgoth == 0 && !borg.trait[BI_KING]) {
            /* teleport level up to 99 to finish uniques */
            if (borg_spell(TELEPORT_LEVEL)
                || borg_read_scroll(sv_scroll_teleport_level)
                || borg_activate_item(act_tele_level)) {
                borg_note("# Rising one dlevel (Not ready for Morgoth)");
                return true;
            }

            /* Start leaving */
            if (!borg.goal.leaving) {
                /* Note */
                borg_note("# Leaving (Not ready for Morgoth now)");

                /* Start leaving */
                borg.goal.leaving = true;
            }
        }
    }

    /*** Evaluate local danger ***/

    /* Monsters on all sides of me? */
    borg_surround = borg_surrounded();

    /* Only allow three 'escapes' per level unless heading for morogoth
       or fighting a unique, then allow 85. */
    if ((borg.escapes > 3 && !unique_on_level && !borg.ready_morgoth)
        || borg.escapes > 55) {
        /* No leaving if going after questors */
        if (borg.trait[BI_CDEPTH] <= 98) {
            /* Start leaving */
            if (!borg.goal.leaving) {
                /* Note */
                borg_note("# Leaving (Too many escapes)");

                /* Start leaving */
                borg.goal.leaving = true;
            }

            /* Start fleeing */
            if (!borg.goal.fleeing && borg.escapes > 3) {
                /* Note */
                borg_note("# Fleeing (Too many escapes)");

                /* Start fleeing */
                borg.goal.fleeing = true;
            }
        }
    }

    /* No hanging around if nasty here. */
    if (scaryguy_on_level) {
        /* Note */
        borg_note("# Scary guy on level.");

        /* Start leaving */
        if (!borg.goal.leaving) {
            /* Note */
            borg_note("# Leaving (Scary guy on level)");

            /* Start leaving */
            borg.goal.leaving = true;
        }

        /* Start fleeing */
        if (!borg.goal.fleeing) {
            /* Note */
            borg_note("# Fleeing (Scary guy on level)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }

        /* Return to town quickly after leaving town */
        if (borg.trait[BI_CDEPTH] == 0)
            borg.goal.fleeing_to_town = true;
    }

    /* Make a note if Ignoring monsters (no fighting) */
    if (borg.goal.ignoring) {
        /* Note */
        borg_note("# Ignoring combat with monsters.");
    }

    /* Note if ignorig messages */
    if (borg_dont_react) {
        borg_note("# Borg ignoring messges.");
    }

    /* Look around */
    pos_danger = borg_danger(borg.c.y, borg.c.x, 1, true, false);

    /* Describe (briefly) the current situation */
    /* Danger (ignore stupid "fear" danger) */
    if ((((pos_danger > avoidance / 10)
             || (pos_danger > borg_fear_region[borg.c.y / 11][borg.c.x / 11])
             || borg_morgoth_position || borg.trait[BI_ISWEAK])
            || borg.trait[BI_CDEPTH] == 100)
        && !borg.trait[BI_KING]) {
        /* Describe (briefly) the current situation */
        borg_note(format(
            "# Loc:%d,%d Dep:%d Lev:%d HP:%d/%d SP:%d/%d Danger:p=%d", borg.c.y,
            borg.c.x, borg.trait[BI_CDEPTH], borg.trait[BI_CLEVEL],
            borg.trait[BI_CURHP], borg.trait[BI_MAXHP], borg.trait[BI_CURSP],
            borg.trait[BI_MAXSP], pos_danger));
        if (borg.resistance) {
            borg_note(format(
                "# Protected by Resistance (borg turns:%d; game turns:%d)",
                borg.resistance / borg_game_ratio,
                player->timed[TMD_OPP_ACID]));
        }
        if (borg.temp.shield) {
            borg_note("# Protected by Mystic Shield");
        }
        if (borg.temp.prot_from_evil) {
            borg_note("# Protected by PFE");
        }
        if (borg_morgoth_position) {
            borg_note("# Protected by Sea of Runes.");
        }
        if (borg_fighting_unique >= 10) {
            borg_note("# Questor Combat.");
        }
        if (borg_as_position) {
            borg_note("# Protected by anti-summon corridor.");
        }
    }
    /* Comment on glyph */
    if (track_glyph.num) {
        int i;
        for (i = 0; i < track_glyph.num; i++) {
            /* Enqueue the grid */
            if ((track_glyph.y[i] == borg.c.y)
                && (track_glyph.x[i] == borg.c.x)) {
                /* if standing on one */
                borg_note(format("# Standing on Glyph"));
            }
        }
    }
    /* Comment on stair */
    if (track_less.num) {
        int i;
        for (i = 0; i < track_less.num; i++) {
            /* Enqueue the grid */
            if ((track_less.y[i] == borg.c.y)
                && (track_less.x[i] == borg.c.x)) {
                /* if standing on one */
                borg_note(format("# Standing on up-stairs"));
                on_upstair = false;
            }
        }
    }
    /* Comment on stair */
    if (track_more.num) {
        int i;
        for (i = 0; i < track_more.num; i++) {
            /* Enqueue the grid */
            if ((track_more.y[i] == borg.c.y)
                && (track_more.x[i] == borg.c.x)) {
                /* if standing on one */
                borg_note(format("# Standing on dn-stairs"));
                on_dnstair = false;
            }
        }
    }

    if (!borg.goal.fleeing) {
        /* Start being cautious and trying to not die */
        if (borg.trait[BI_CLASS] == CLASS_MAGE && !borg_morgoth_position
            && !borg_as_position && !borg.trait[BI_ISBLIND]
            && !borg.trait[BI_ISCUT] && !borg.trait[BI_ISPOISONED]
            && !borg.trait[BI_ISCONFUSED]) {
            /* do some defense before running away */
            if (borg_defend(pos_danger))
                return true;

            /* try healing before running away */
            if (borg_heal(pos_danger))
                return true;
        } else {
            /* try healing before running away */
            if (borg_heal(pos_danger))
                return true;

            /* do some defense before running away! */
            if (borg_defend(pos_danger))
                return true;
        }
    }

    if (borg_cfg[BORG_USES_SWAPS]) {
        /* do some swapping before running away! */
        if (pos_danger > (avoidance / 3)) {
            if (borg_backup_swap(pos_danger))
                return true;
        }
    }

    /* If I am waiting for recall,  & safe, then stay put. */
    if (borg.goal.recalling && borg_check_rest(borg.c.y, borg.c.x)
        && borg.trait[BI_CDEPTH] && !borg.trait[BI_ISHUNGRY]) {
        /* rest here until lift off */
        borg_note("# Resting for Recall.");
        borg_keypress('R');
        borg_keypress('5');
        borg_keypress('0');
        borg_keypress('0');
        borg_keypress(KC_ENTER);

        /* I'm not in a store */
        borg.in_shop = false;

        return true;
    }

    /* If I am waiting for recall in town */
    if (borg.goal.recalling && borg.goal.recalling <= (borg_game_ratio * 2)
        && !borg.trait[BI_CDEPTH]) {
        if (borg_prep_leave_level_spells())
            return true;
    }

    /*** Danger ***/

    /* Impending doom */
    /* Don't take off in the middle of a fight */
    /* just to restock and it is useless to restock */
    /* if you have just left town. */
    if (borg_restock(borg.trait[BI_CDEPTH]) && !borg_fighting_unique
        && (borg_time_town + (borg_t - borg_began)) > 200) {
        /* Start leaving */
        if (!borg.goal.leaving) {
            /* Note */
            borg_note(format(
                "# Leaving (restock) %s", borg_restock(borg.trait[BI_CDEPTH])));

            /* Start leaving */
            borg.goal.leaving = true;
        }
        /* Start fleeing */
        if (!borg.goal.fleeing && borg.trait[BI_ACCW] < 2
            && borg.trait[BI_FOOD] > 3 && borg.trait[BI_AFUEL] > 2) {
            /* Flee */
            borg_note(format(
                "# Fleeing (restock) %s", borg_restock(borg.trait[BI_CDEPTH])));

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }
    /* Excessive danger */
    else if (pos_danger > (borg.trait[BI_CURHP] * 2)) {
        /* Start fleeing */
        /* do not flee level if going after Morgoth or fighting a unique */
        if (!borg.goal.fleeing && !borg_fighting_unique
            && (borg.trait[BI_CLEVEL] < 50) && !vault_on_level
            && (borg.trait[BI_CDEPTH] < 100 && borg.ready_morgoth == 1)) {
            /* Note */
            borg_note("# Fleeing (excessive danger)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }
    /* Potential danger (near death) in town */
    else if (!borg.trait[BI_CDEPTH] && (pos_danger > borg.trait[BI_CURHP])
             && (borg.trait[BI_CLEVEL] < 50)) {
        /* Flee now */
        if (!borg.goal.leaving) {
            /* Flee! */
            borg_note("# Leaving (potential danger)");

            /* Start leaving */
            borg.goal.leaving = true;
        }
    }

    /*** Stairs ***/

    /* Leaving or Fleeing, take stairs */
    if (borg.goal.leaving || borg.goal.fleeing || scaryguy_on_level
        || borg.goal.fleeing_lunal || borg.goal.fleeing_munchkin
        || ((pos_danger > avoidance
                || (borg.trait[BI_CLEVEL] < 5 && pos_danger > avoidance / 2))
            && on_upstair)) /* danger and standing on stair */
    {
        if (borg.ready_morgoth == 0 && !borg.trait[BI_KING]) {
            borg.stair_less = true;
            if (scaryguy_on_level)
                borg_note("# Fleeing and leaving the level. (scaryguy)");
            if (borg.goal.fleeing_lunal)
                borg_note("# Fleeing and leaving the level. (fleeing_lunal)");
            if (borg.goal.fleeing_munchkin)
                borg_note(
                    "# Fleeing and leaving the level. (fleeing munchkin)");
            if (pos_danger > avoidance && borg.trait[BI_CLEVEL] <= 49
                && borg_grids[borg.c.y][borg.c.x].feat == FEAT_LESS)
                borg_note("# Leaving level,  Some danger but I'm on a stair.");
        }

        if (scaryguy_on_level)
            borg.stair_less = true;

        /* Only go down if fleeing or prepared */
        if (borg.goal.fleeing == true || borg.goal.fleeing_lunal == true
            || borg.goal.fleeing_munchkin)
            borg.stair_more = true;

        if ((char *)NULL == borg_prepared(borg.trait[BI_CDEPTH] + 1))
            borg.stair_more = true;

        /* don't go down if we can go up and are hungry */
        if (track_less.num
            && (borg.trait[BI_CURLITE] == 0 || borg.trait[BI_ISHUNGRY]
                || borg.trait[BI_ISWEAK] || borg.trait[BI_FOOD] < 2))
            borg.stair_more = false;

        /* If I need to sell crap, then don't go down if I can go up */
        if (track_less.num && borg.trait[BI_CDEPTH]
            && borg.trait[BI_CLEVEL] < 25 && borg.trait[BI_GOLD] < 25000
            && borg_count_sell() >= 13)
            borg.stair_more = false;

        /* Its ok to go one level deep if evading scary guy */
        if (scaryguy_on_level)
            borg.stair_more = true;

        /* if fleeing town, then dive */
        if (!borg.trait[BI_CDEPTH])
            borg.stair_more = true;
    }

    /* Take stairs up */
    if (borg.stair_less) {
        /* Current grid */
        borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

        /* Usable stairs */
        if (ag->feat == FEAT_LESS || on_upstair) {
            /* Log it */
            borg_note(format("# Leaving via up stairs."));

            /* Take the stairs */
            borg_keypress('<');

            /* Success */
            return true;
        }
    }

    /* Take stairs down */
    if (borg.stair_more && !borg.goal.recalling) {
        /* Current grid */
        borg_grid *ag = &borg_grids[borg.c.y][borg.c.x];

        /* Usable stairs */
        if (ag->feat == FEAT_MORE || on_dnstair) {
            /* Do these if not lunal mode */
            if (!borg.goal.fleeing_lunal && !borg.goal.fleeing_munchkin) {
                if (borg_prep_leave_level_spells())
                    return true;
            }

            /* Take the stairs */
            borg_keypress('>');

            /* Success */
            return true;
        }
    }

    /*** Deal with critical situations ***/

    /* Hack -- require light */
    if (!borg.trait[BI_CURLITE]
        && !borg.trait[BI_LIGHT]) /* No Lite, AND Not Glowing */
    {
        enum borg_need need = borg_maintain_light();
        if (need == BORG_MET_NEED)
            return true;
        else if ((need == BORG_UNMET_NEED) && borg.trait[BI_CDEPTH]) {
            /* Flee for fuel */
            /* Start leaving */
            if (!borg.goal.leaving) {
                /* Flee */
                borg_note("# Leaving (need fuel)");

                /* Start leaving */
                borg.goal.leaving = true;
            }
        }
    }

    /* Hack -- prevent starvation */
    if (borg.trait[BI_ISWEAK]) {
        /* Attempt to satisfy hunger */
        if (borg_eat_food_any() || borg_spell(REMOVE_HUNGER)
            || borg_spell(HERBAL_CURING)) {
            /* Success */
            return true;
        }

        /* Try to restore mana then cast the spell next round */
        if (borg_quaff_potion(sv_potion_restore_mana)
            || borg_activate_item(act_restore_mana))
            return true;

        /* Flee for food */
        if (borg.trait[BI_CDEPTH]) {
            /* Start leaving */
            if (!borg.goal.leaving) {
                /* Flee */
                borg_note("# Leaving (need food)");

                /* Start leaving */
                borg.goal.leaving = true;
            }

            /* Start fleeing */
            if (!borg.goal.fleeing) {
                /* Flee */
                borg_note("# Fleeing (need food)");

                /* Start fleeing */
                borg.goal.fleeing = true;
            }
        }
    }

    /* Prevent breeder explosions when low level */
    if (breeder_level && borg.trait[BI_CLEVEL] < 15) {
        /* Start leaving */
        if (!borg.goal.fleeing) {
            /* Flee */
            borg_note("# Fleeing (breeder level)");

            /* Start fleeing */
            borg.goal.fleeing = true;
        }
    }

    /*** Flee on foot ***/

    /* Desperation Head for stairs */
    /* If you are low level and near the stairs and you can */
    /* hop onto them in very few steps, try to head to them */
    /* out of desperation */
    if ((track_less.num || track_more.num)
        && (borg.goal.fleeing || scaryguy_on_level
            || (pos_danger > avoidance && borg.trait[BI_CLEVEL] < 35))) {
        int  y, x, i;
        int  b_j = -1;
        int  m;
        int  b_m  = -1;
        bool safe = true;

        borg_grid *ag;

        /* Check for an existing "up stairs" */
        for (i = 0; i < track_less.num; i++) {
            x  = track_less.x[i];
            y  = track_less.y[i];

            ag = &borg_grids[y][x];

            /* How far is the nearest up stairs */
            j = distance(borg.c, loc(x, y));

            /* Skip stairs if a monster is on the stair */
            if (ag->kill)
                continue;

            /* skip the closer ones */
            if (b_j >= j)
                continue;

            /* track it */
            b_j = j;
        }

        /* Check for an existing "down stairs" */
        for (i = 0; i < track_more.num; i++) {
            x  = track_more.x[i];
            y  = track_more.y[i];

            ag = &borg_grids[y][x];

            /* How far is the nearest up stairs */
            m = distance(borg.c, loc(x, y));

            /* Skip stairs if a monster is on the stair */
            if (ag->kill)
                continue;

            /* skip the closer ones */
            if (b_m >= m)
                continue;

            /* track it */
            b_m = m;
        }

        /* If you are within a few (3) steps of the stairs */
        /* and you can take some damage to get there */
        /* go for it */
        if (b_j < 3 && b_j != -1 && pos_danger < borg.trait[BI_CURHP]) {
            borg_desperate = true;
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                /* Note */
                borg_note("# Desperate for Stairs (one)");

                borg_desperate = false;
                return true;
            }
            borg_desperate = false;
        }

        /* If you are next to steps of the stairs go for it */
        if (b_j <= 2 && b_j != -1) {
            borg_desperate = true;
            if (borg_flow_stair_less(GOAL_FLEE, false)) {
                /* Note */
                borg_note("# Desperate for Stairs (two)");

                borg_desperate = false;
                return true;
            }
            borg_desperate = false;
        }

        /* Low level guys tend to waste money reading the recall scrolls */
        if (b_j < 20 && b_j != -1 && scaryguy_on_level
            && borg.trait[BI_CLEVEL] < 20) {
            /* do not attempt it if an adjacent monster is faster than me */
            for (i = 0; i < 8; i++) {
                x = borg.c.x + ddx_ddd[i];
                y = borg.c.y + ddy_ddd[i];

                /* check for bounds */
                if (!square_in_bounds(cave, loc(x, y)))
                    continue;

                /* Monster there ? */
                if (!borg_grids[y][x].kill)
                    continue;

                /* Access the monster and check it's speed */
                if (borg_kills[borg_grids[y][x].kill].speed
                    > borg.trait[BI_SPEED])
                    safe = false;
            }

            /* Don't run from Grip or Fang */
            if ((borg.trait[BI_CDEPTH] <= 5 && borg.trait[BI_CDEPTH] != 0
                    && borg_fighting_unique)
                || !safe) {
                /* try to take them on, you cant outrun them */
            } else {
                borg_desperate = true;
                if (borg_flow_stair_less(GOAL_FLEE, false)) {
                    /* Note */
                    borg_note("# Desperate for Stairs (three)");

                    borg_desperate = false;
                    return true;
                }
                borg_desperate = false;
            }
        }

        /* If you are next to steps of the down stairs go for it */
        if (b_m <= 2 && b_m != -1) {
            borg_desperate = true;
            if (borg_flow_stair_more(GOAL_FLEE, false, false)) {
                /* Note */
                borg_note("# Desperate for Stairs (four)");

                borg_desperate = false;
                return true;
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
    if (((pos_danger > avoidance / 3 && !nasty && !borg.no_retreat)
            || (borg_surround && pos_danger != 0))
        && !borg_morgoth_position && (borg_t - borg_t_antisummon >= 50)
        && !borg.trait[BI_ISCONFUSED] && !square_isvault(cave, borg.c)
        && borg.trait[BI_CURHP] < 500) {
        int d, b_d = -1;
        int r, b_r = -1;
        int b_p = -1, p1 = -1;
        int b_x = borg.c.x;
        int b_y = borg.c.y;
        int ii;

        /* Scan the useful viewable grids */
        for (j = 1; j < borg_view_n; j++) {
            int x1 = borg.c.x;
            int y1 = borg.c.y;

            int x2 = borg_view_x[j];
            int y2 = borg_view_y[j];

            /* Cant if confused: no way to predict motion */
            if (borg.trait[BI_ISCONFUSED])
                continue;

            /* Require "floor" grids */
            if (!borg_cave_floor_bold(y2, x2))
                continue;

            /* Try to avoid pillar dancing if at good health */
            if ((borg.trait[BI_CURHP] >= borg.trait[BI_MAXHP] * 7 / 10
                    && ((track_step.num > 2
                         && (track_step.y[track_step.num - 2] == y2
                             && track_step.x[track_step.num - 2] == x2
                             && track_step.y[track_step.num - 3] == borg.c.y
                             && track_step.x[track_step.num - 3] == borg.c.x))))
                || borg.time_this_panel >= 300)
                continue;

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
            if (!borg_happy_grid_bold(y2, x2))
                continue;

            /* Track "nearest" grid */
            if (b_r >= 0) {
                int ay = ((y2 > y1) ? (y2 - y1) : (y1 - y2));
                int ax = ((x2 > x1) ? (x2 - x1) : (x1 - x2));

                /* Ignore "distant" locations */
                if ((ax > b_r) || (ay > b_r))
                    continue;
            }

            /* Reset */
            r = 0;

            /* Simulate movement */
            while (1) {
                borg_grid *ag;

                /* Obtain direction */
                d = borg_goto_dir(y1, x1, y2, x2);

                /* Verify direction */
                if ((d == 0) || (d == 5))
                    break;

                /* Track distance */
                r++;

                /* Simulate the step */
                y1 += ddy[d];
                x1 += ddx[d];

                /* Obtain the grid */
                ag = &borg_grids[y1][x1];

                /* Lets make one more check that we are not bouncing */
                if ((borg.trait[BI_CURHP] >= borg.trait[BI_MAXHP] * 7 / 10
                        && ((track_step.num > 2
                             && (track_step.y[track_step.num - 2] == y1
                                 && track_step.x[track_step.num - 2] == x1
                                 && track_step.y[track_step.num - 3] == borg.c.y
                                 && track_step.x[track_step.num - 3]
                                        == borg.c.x))))
                    || borg.time_this_panel >= 300)
                    break;

                /* Require floor */
                if (!borg_cave_floor_grid(ag)
                    || (ag->feat == FEAT_LAVA && !borg.trait[BI_IFIRE]))
                    break;

                /* Require it to be somewhat close */
                if (r >= 10)
                    break;

                /* Check danger of that spot */
                p1 = borg_danger(y1, x1, 1, true, false);
                if (p1 >= pos_danger)
                    break;

                /* make sure it is not dangerous to take the first step; unless
                 * surrounded. */
                if (r == 1) {
                    /* Not surrounded or surrounded and ignoring*/
                    if (!borg_surround
                        || (borg_surround && borg.goal.ignoring)) {
                        if (p1 >= borg.trait[BI_CURHP] * 4 / 10)
                            break;

                        /* Ought to be worth it */;
                        if (p1 > pos_danger * 5 / 10)
                            break;
                    } else
                    /* Surrounded, try to back-up */
                    {
                        if (borg.trait[BI_CLEVEL] >= 20) {
                            if (p1 >= (b_r <= 5 ? borg.trait[BI_CURHP] * 15 / 10
                                                : borg.trait[BI_CURHP]))
                                break;
                        } else {
                            if (p1 >= borg.trait[BI_CURHP] * 4)
                                break;
                        }
                    }

                    /*
                     * Skip this grid if it is adjacent to a monster.  He will
                     * just hit me when I land on that grid.
                     */
                    for (ii = 1; ii < borg_kills_nxt; ii++) {
                        borg_kill *kill;

                        /* Monster */
                        kill = &borg_kills[ii];

                        /* Skip dead monsters */
                        if (!kill->r_idx)
                            continue;

                        /* Require current knowledge */
                        if (kill->when < borg_t - 2)
                            continue;

                        /* Check distance -- 1 grid away */
                        if (distance(kill->pos, loc(x1, y1)) <= 1
                            && kill->speed > borg.trait[BI_SPEED]
                            && !borg_surround)
                            break;
                    }
                }

                /* Skip monsters */
                if (ag->kill)
                    break;

                /* Skip traps */
                if (ag->trap && !ag->glyph)
                    break;

                /* Safe arrival */
                if ((x1 == x2) && (y1 == y2)) {
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
        if (b_r >= 0) {
            /* Save direction */
            b_d = borg_goto_dir(borg.c.y, borg.c.x, b_y, b_x);

            /* Hack -- set goal */
            borg.goal.g.x = borg.c.x + ddx[b_d];
            borg.goal.g.y = borg.c.y + ddy[b_d];

            /* Note */
            borg_note(format(
                "# Retreating to %d,%d (distance %d) via %d,%d (%d > %d)", b_y,
                b_x, b_r, borg.goal.g.y, borg.goal.g.x, pos_danger, b_p));

            /* Strategic retreat */
            borg_keypress(I2D(b_d));

            /* Reset my Movement and Flow Goals */
            borg.goal.type = 0;

            /* Success */
            return true;
        }
    }

    /*** Escape if possible ***/

    /* Attempt to escape via spells */
    if (borg_escape(pos_danger)) {
        /* increment the escapes this level counter */
        borg.escapes++;

        /* Clear any Flow queues */
        borg.goal.type = 0;

        /* Success */
        return true;
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
    if (((pos_danger > (avoidance * 4 / 10) && !nasty && !borg.no_retreat)
            || (borg_surround && pos_danger != 0))
        && !borg_morgoth_position && (borg_t - borg_t_antisummon >= 50)
        && !borg.trait[BI_ISCONFUSED] && !square_isvault(cave, borg.c)
        && borg.trait[BI_CURHP] < 500) {
        int  i = -1, b_i = -1;
        int  k = -1, b_k = -1;
        int  f = -1, b_f = -1;
        int  g_k = 0;
        int  ii;
        bool adjacent_monster = false;

        /* Current danger */
        b_k = pos_danger;

        /* Fake the danger down if surrounded so that he can move. */
        if (borg_surround)
            b_k = (b_k * 12 / 10);

        /* Check the freedom */
        b_f = borg_freedom(borg.c.y, borg.c.x);

        /* Attempt to find a better grid */
        for (i = 0; i < 8; i++) {
            int x = borg.c.x + ddx_ddd[i];
            int y = borg.c.y + ddy_ddd[i];

            /* Access the grid */
            borg_grid *ag = &borg_grids[y][x];

            /* Cant if confused: no way to predict motion */
            if (borg.trait[BI_ISCONFUSED])
                continue;

            /* Skip walls/doors */
            if (!borg_cave_floor_grid(ag))
                continue;

            /* Skip monster grids */
            if (ag->kill)
                continue;

            /* Mega-Hack -- skip stores XXX XXX XXX */
            if (feat_is_shop(ag->feat))
                continue;

            /* Mega-Hack -- skip traps XXX XXX XXX */
            if (ag->trap && !ag->glyph)
                break;

            /* If i was here last round and 3 rounds ago, suggesting a "bounce"
             */
            if ((borg.trait[BI_CURHP] >= borg.trait[BI_MAXHP] * 7 / 10
                    && ((track_step.num > 2
                         && (track_step.y[track_step.num - 2] == y
                             && track_step.x[track_step.num - 2] == x
                             && track_step.y[track_step.num - 3] == borg.c.y
                             && track_step.x[track_step.num - 3] == borg.c.x))))
                || borg.time_this_panel >= 300)
                continue;

            /*
             * Skip this grid if it is adjacent to a monster.  He will just hit
             * me when I land on that grid.
             */
            for (ii = 1; ii < borg_kills_nxt; ii++) {
                borg_kill *kill;

                /* Monster */
                kill = &borg_kills[ii];

                /* Skip dead monsters */
                if (!kill->r_idx)
                    continue;

                /* Require current knowledge */
                if (kill->when < borg_t - 2)
                    continue;

                /* Check distance -- 1 grid away */
                if (distance(kill->pos, loc(x, y)) <= 1 && !borg_surround)
                    adjacent_monster = true;

                /* Check distance -- 2 grids away and he is faster than me */
                if (distance(kill->pos, loc(x, y)) <= 2
                    && kill->speed > borg.trait[BI_SPEED] && !borg_surround)
                    adjacent_monster = true;
            }

            /* Skip this grid consideration because it is next to a monster */
            if (adjacent_monster == true)
                continue;

            /* Extract the danger there */
            k = borg_danger(y, x, 1, true, false);

            /* Skip this grid if danger is higher than my HP.
             * Take my chances with fighting.
             */
            if (k > avoidance)
                continue;

            /* Skip this grid if it is not really worth backing up.  Look for a
             * 40% reduction in the danger if higher level.  If the danger of
             * the new grid is close to the danger of my current grid, I'll stay
             * and fight.
             */
            if (borg.trait[BI_MAXCLEVEL] >= 35 && k > b_k * 6 / 10)
                continue;

            /* Skip this grid if it is not really worth backing up.  If the
             * danger of the new grid is close to the danger of my current grid,
             * I'll stay and fight unless I am low level and there is an
             * adjacent monster.
             */
            if (borg.trait[BI_MAXCLEVEL] < 35 && adjacent_monster == false
                && k > b_k * 8 / 10)
                continue;

            /* Skip higher danger */
            /*     note: if surrounded, then b_k has been adjusted to a higher
             * number to make his current grid seem more dangerous.  This will
             * encourage him to Back-Up.
             */
            if (k > b_k)
                continue;

            /* Record the danger of this preferred grid */
            g_k = k;

            /* Check the freedom there */
            f = borg_freedom(y, x);

            /* Danger is the same, so look at the nature of the grid */
            if (b_k == k) {
                /* If I am low level, reward backing-up if safe */
                if (borg.trait[BI_CLEVEL] <= 10 && borg.trait[BI_CDEPTH]
                    && (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP]
                        || borg.trait[BI_CURSP] < borg.trait[BI_MAXSP])) {
                    /* do consider the retreat */
                } else {
                    /* Freedom of my grid is better than the next grid
                     * so stay put and fight.
                     */
                    if (b_f > f || borg.trait[BI_CDEPTH] >= 85)
                        continue;
                }
            }

            /* Save the info */
            b_i = i;
            b_k = k;
            b_f = f;
        }

        /* Back away */
        if (b_i >= 0) {
            /* Hack -- set goal */
            borg.goal.g.x = borg.c.x + ddx_ddd[b_i];
            borg.goal.g.y = borg.c.y + ddy_ddd[b_i];

            /* Note */
            borg_note(format("# Backing up to %d,%d (%d > %d)", borg.goal.g.x,
                borg.goal.g.y, pos_danger, g_k));

            /* Back away from danger */
            borg_keypress(I2D(ddd[b_i]));

            /* Reset my Movement and Flow Goals */
            borg.goal.type = 0;

            /* Success */
            return true;
        }
    }

    /*** Cures ***/

    /* cure confusion, second check, first (slightly different) in borg_heal */
    if (borg.trait[BI_ISCONFUSED]) {
        if (borg.trait[BI_MAXHP] - borg.trait[BI_CURHP] >= 300
            && (borg_quaff_potion(sv_potion_healing)
                || borg_quaff_potion(sv_potion_star_healing)
                || borg_quaff_potion(sv_potion_life))) {
            borg_note("# Healing.  Confusion.");
            return true;
        }
        if (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)
            || borg_quaff_potion(sv_potion_cure_serious)
            || borg_quaff_crit(false) || borg_quaff_potion(sv_potion_healing)
            || borg_activate_item(act_cure_confusion)
            || borg_use_staff_fail(sv_staff_healing)) {
            borg_note("# Healing.  Confusion.");
            return true;
        }
    }

    /* Hack -- cure fear when afraid */
    if ((borg.trait[BI_ISAFRAID] && !borg.trait[BI_CRSFEAR])
        && (randint0(100) < 70
            || (borg.trait[BI_CLASS] == CLASS_WARRIOR
                && borg.trait[BI_AMISSILES] <= 0))) {
        if (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)
            || borg_quaff_potion(sv_potion_boldness)
            || borg_quaff_potion(sv_potion_heroism)
            || borg_quaff_potion(sv_potion_berserk)
            || borg_spell_fail(BERSERK_STRENGTH, 25) || /* berserk */
            borg_spell_fail(HEROISM, 25) || /* hero */
            borg_activate_item(act_cure_paranoia)
            || borg_activate_item(act_hero) || borg_activate_item(act_shero)
            || borg_activate_item(act_cure_mind)
            || borg_activate_item(act_rage_bless_resist)
            || borg_activate_item(act_rem_fear_pois)
            || borg_spell_fail(HOLY_WORD, 25)) {
            return true;
        }
    }

    /*** Note impending death XXX XXX XXX ***/

    /* Flee from low hit-points */
    if (((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 3)
            || ((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2)
                && borg.trait[BI_CURHP] < (borg.trait[BI_CLEVEL] * 3)))
        && (borg.trait[BI_ACCW] < 3) && (borg.trait[BI_AHEAL] < 1)) {
        /* Flee from low hit-points */
        if (borg.trait[BI_CDEPTH] && (randint0(100) < 25)) {
            /* Start leaving */
            if (!borg.goal.leaving) {
                /* Flee */
                borg_note("# Leaving (low hit-points)");

                /* Start leaving */
                borg.goal.leaving = true;
            }
            /* Start fleeing */
            if (!borg.goal.fleeing) {
                /* Flee */
                borg_note("# Fleeing (low hit-points)");

                /* Start fleeing */
                borg.goal.fleeing = true;
            }
        }
    }

    /* Flee from bleeding wounds or poison and no heals */
    if ((borg.trait[BI_ISCUT] || borg.trait[BI_ISPOISONED])
        && (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2)) {
        /* Flee from bleeding wounds */
        if (borg.trait[BI_CDEPTH] && (randint0(100) < 25)) {
            /* Start leaving */
            if (!borg.goal.leaving) {
                /* Flee */
                borg_note("# Leaving (bleeding/poison)");

                /* Start leaving */
                borg.goal.leaving = true;
            }

            /* Start fleeing */
            if (!borg.goal.fleeing) {
                /* Flee */
                borg_note("# Fleeing (bleeding/poison)");

                /* Start fleeing */
                borg.goal.fleeing = true;
            }
        }
    }

    /* Emergency check on healing.  Borg_heal has already been checked but
     * but we did not use our ez_heal potions.  All other attempts to save
     * ourselves have failed.  Use the ez_heal if I have it.
     */
    if ((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 10
            || /* dangerously low HP -OR-*/
            (pos_danger > borg.trait[BI_CURHP] && /* extreme danger -AND-*/
                (borg.trait[BI_ATELEPORT] + borg.trait[BI_AESCAPE] <= 2
                    && borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 4))
            || /* low on escapes */
            (borg.trait[BI_AEZHEAL] > 5
                && borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 4)
            || /* moderate danger, lots of heals */
            (borg.trait[BI_MAXHP] - borg.trait[BI_CURHP] >= 600
                && borg_fighting_unique && borg.trait[BI_CDEPTH] >= 85))
        && /* moderate danger, unique, deep */
        (borg_quaff_potion(sv_potion_star_healing)
            || borg_quaff_potion(sv_potion_healing)
            || borg_quaff_potion(sv_potion_life))) {
        borg_note("# Using reserve EZ_Heal.");
        return true;
    }

    /* Hack -- use "recall" to flee if possible */
    if (borg.goal.fleeing && !borg.goal.fleeing_munchkin
        && !borg.goal.fleeing_to_town && borg.trait[BI_CDEPTH] >= 1
        && (borg_recall())) {
        /* Note */
        borg_note("# Fleeing the level (recall)");

        /* Success */
        return true;
    }

    /* If I am waiting for recall,and in danger, buy time with
     * phase and cure_anythings.
     */
    if (borg.goal.recalling && (pos_danger > avoidance * 2)) {
        if (!borg.trait[BI_ISCONFUSED] && !borg.trait[BI_ISBLIND]
            && borg.trait[BI_MAXSP] > 60
            && borg.trait[BI_CURSP] < (borg.trait[BI_CURSP] / 4)
            && (borg_quaff_potion(sv_potion_restore_mana)
                || borg_activate_item(act_restore_mana))) {
            borg_note("# Buying time waiting for Recall.(1)");
            return true;
        }

        if (borg_caution_phase(50, 1)
            && (borg_read_scroll(sv_scroll_phase_door)
                || borg_spell_fail(PHASE_DOOR, 30)
                || borg_spell_fail(PORTAL, 30)
                || borg_activate_item(act_tele_phase))) {
            borg_note("# Buying time waiting for Recall.(2)");
            return true;
        }

        if ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP] < 100)
            && (borg_quaff_crit(true)
                || borg_quaff_potion(sv_potion_cure_serious)
                || borg_quaff_potion(sv_potion_cure_light))) {
            borg_note("# Buying time waiting for Recall.(3)");
            return true;
        }

        if ((borg.trait[BI_MAXHP] - borg.trait[BI_CURHP] > 150)
            && (borg_zap_rod(sv_rod_healing)
                || borg_quaff_potion(sv_potion_healing)
                || borg_quaff_potion(sv_potion_star_healing)
                || borg_quaff_potion(sv_potion_life) || borg_quaff_crit(true)
                || borg_quaff_potion(sv_potion_cure_serious)
                || borg_quaff_potion(sv_potion_cure_light))) {
            borg_note("# Buying time waiting for Recall.(4)");
            return true;
        }
    }

    /* if I am gonna die next round, and I have no way to escape
     * use the unknown stuff (if I am low level).
     */
    if (pos_danger > (borg.trait[BI_CURHP] * 4) && borg.trait[BI_CLEVEL] < 20
        && !borg.trait[BI_MAXSP]) {
        if (borg_use_unknown() || borg_read_unknown() || borg_quaff_unknown()
            || borg_eat_unknown())
            return true;
    }

    /* Nothing */
    return false;
}

#endif
