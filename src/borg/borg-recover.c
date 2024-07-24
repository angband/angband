/**
 * \file borg-recover.c
 * \brief Attempt to recover from damage and such after a battle
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

#include "borg-recover.h"

#ifdef ALLOW_BORG

#include "../ui-event.h"

#include "borg-danger.h"
#include "borg-flow-kill.h"
#include "borg-flow-misc.h"
#include "borg-flow.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-light.h"
#include "borg-magic.h"
#include "borg-trait.h"
#include "borg.h"

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
    int            p = 0;
    int            q;
    enum borg_need need;

    /*** Handle annoying situations ***/
    need = borg_maintain_light();
    if (need == BORG_MET_NEED)
        return true;
    else if (need == BORG_UNMET_NEED)
        borg_note(format("# Need to refuel but cant!"));

    /*** Do not recover when in danger ***/

    /* Look around for danger */
    p = borg_danger(borg.c.y, borg.c.x, 1, true, false);

    /* Never recover in dangerous situations */
    if (p > avoidance / 4)
        return false;

    /*** Roll for "paranoia" ***/

    /* Base roll */
    q = randint0(100);

    /* Half dead */
    if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2)
        q = q - 10;

    /* Almost dead */
    if (borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 4)
        q = q - 10;

    /*** Use "cheap" cures ***/

    /* Hack -- cure stun */
    if (borg.trait[BI_ISSTUN] && (q < 75)) {
        if (borg_activate_item(act_cure_body)
            || borg_activate_item(act_cure_critical)
            || borg_activate_item(act_cure_full)
            || borg_activate_item(act_cure_full2)
            || borg_activate_item(act_cure_temp)
            || borg_activate_item(act_heal3) || borg_spell(MINOR_HEALING)
            || borg_spell(HEALING) || borg_spell(HERBAL_CURING)
            || borg_spell(HOLY_WORD))

        {
            /* Take note */
            borg_note(format("# Cure Stun - danger %d", p));

            return true;
        }
    }

    /* Hack -- cure stun */
    if (borg.trait[BI_ISHEAVYSTUN]) {
        if (borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
            || borg_activate_item(act_cure_body)
            || borg_activate_item(act_cure_critical)
            || borg_activate_item(act_cure_full)
            || borg_activate_item(act_cure_full2)
            || borg_activate_item(act_cure_temp)
            || borg_activate_item(act_heal3) || borg_spell(MINOR_HEALING)
            || borg_spell(HEALING) || borg_spell(HERBAL_CURING)
            || borg_spell(HOLY_WORD)) {
            /* Take note */
            borg_note(format("# Cure Heavy Stun - danger %d", p));

            return true;
        }
    }

    /* Hack -- cure cuts */
    if (borg.trait[BI_ISCUT] && (q < 75)) {
        if (borg_activate_item(act_cure_light) || borg_spell(MINOR_HEALING)
            || borg_spell(HEALING) || borg_spell(HERBAL_CURING)
            || borg_spell(HOLY_WORD)) {
            /* Take note */
            borg_note(format("# Cure Cuts - danger %d", p));

            return true;
        }
    }

    /* Hack -- cure poison */
    if (borg.trait[BI_ISPOISONED] && (q < 75)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
            || borg_activate_item(act_rem_fear_pois)
            || borg_spell(HERBAL_CURING) || borg_spell(CURE_POISON)) {
            /* Take note */
            borg_note(format("# Cure poison - danger %d", p));

            return true;
        }
    }

    /* Hack -- cure fear */
    if (borg.trait[BI_ISAFRAID] && !borg.trait[BI_CRSFEAR] && (q < 75)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)
            || borg_activate_item(act_rem_fear_pois) || borg_spell(HEROISM)
            || borg_spell(BERSERK_STRENGTH) || borg_spell(HOLY_WORD)) {
            /* Take note */
            borg_note(format("# Cure fear - danger %d", p));

            return true;
        }
    }

    /* Hack -- satisfy hunger */
    if ((borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK]) && (q < 75)) {
        if (borg_spell(REMOVE_HUNGER) || borg_spell(HERBAL_CURING)) {
            return true;
        }
    }

    /* Hack -- hallucination */
    if (borg.trait[BI_ISIMAGE] && (q < 75)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)) {
            return true;
        }
    }

    /* Hack -- heal damage */
    if ((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2) && (q < 75) && p == 0
        && (borg.trait[BI_CURSP] > borg.trait[BI_MAXSP] / 4)) {
        if (borg_activate_item(act_heal1) || borg_activate_item(act_heal2)
            || borg_activate_item(act_heal3) || borg_spell(HEALING)
            || borg_spell(HOLY_WORD) || borg_spell(MINOR_HEALING)
            || borg_spell(HEROISM)) {
            /* Take note */
            borg_note(format("# heal damage (recovering)"));

            return true;
        }
    }

    /* cure experience loss with prayer */
    if (borg.trait[BI_ISFIXEXP]
        && (borg_activate_item(act_restore_exp)
            || borg_activate_item(act_restore_st_lev)
            || borg_activate_item(act_restore_life) || borg_spell(REVITALIZE)
            || borg_spell(REMEMBRANCE)
            || (borg.trait[BI_CURHP] > 90 && borg_spell(UNHOLY_REPRIEVE)))) {
        return true;
    }

    /* cure stat drain with prayer */
    if ((borg.trait[BI_ISFIXSTR] || borg.trait[BI_ISFIXINT]
            || borg.trait[BI_ISFIXWIS] || borg.trait[BI_ISFIXDEX]
            || borg.trait[BI_ISFIXCON] || borg.trait[BI_ISFIXALL])
        && (borg_spell(RESTORATION) || borg_spell(REVITALIZE))) {
        return true;
    }

    /* cure stat drain with prayer */
    if ((borg.trait[BI_ISFIXSTR] || borg.trait[BI_ISFIXINT]
            || borg.trait[BI_ISFIXCON])
        && borg.trait[BI_CURHP] > 90 && borg_spell(UNHOLY_REPRIEVE)) {
        return true;
    }

    /*** Use "expensive" cures ***/

    /* Hack -- cure stun */
    if (borg.trait[BI_ISSTUN] && (q < 25)) {
        if (borg_use_staff_fail(sv_staff_curing) || borg_zap_rod(sv_rod_curing)
            || borg_zap_rod(sv_rod_healing) || borg_activate_item(act_heal1)
            || borg_activate_item(act_heal2) || borg_quaff_crit(false)) {
            return true;
        }
    }

    /* Hack -- cure heavy stun */
    if (borg.trait[BI_ISHEAVYSTUN] && (q < 95)) {
        if (borg_quaff_crit(true) || borg_use_staff_fail(sv_staff_curing)
            || borg_zap_rod(sv_rod_curing) || borg_zap_rod(sv_rod_healing)
            || borg_activate_item(act_heal1) || borg_activate_item(act_heal2)) {
            return true;
        }
    }

    /* Hack -- cure cuts */
    if (borg.trait[BI_ISCUT] && (q < 25)) {
        if (borg_use_staff_fail(sv_staff_curing) || borg_zap_rod(sv_rod_curing)
            || borg_zap_rod(sv_rod_healing) || borg_activate_item(act_heal1)
            || borg_activate_item(act_heal2)
            || borg_quaff_crit(borg.trait[BI_CURHP] < 10)) {
            return true;
        }
    }

    /* Hack -- cure poison */
    if (borg.trait[BI_ISPOISONED] && (q < 25)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
            || borg_quaff_potion(sv_potion_cure_poison)
            || borg_eat(TV_FOOD, sv_food_waybread)
            || borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
            || borg_quaff_crit(borg.trait[BI_CURHP] < 10)
            || borg_use_staff_fail(sv_staff_curing)
            || borg_zap_rod(sv_rod_curing)
            || borg_activate_item(act_rem_fear_pois)
            || borg_activate_item(act_food_waybread)) {
            return true;
        }
    }

    /* Hack -- cure blindness */
    if (borg.trait[BI_ISBLIND] && (q < 25)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_fast_recovery)
            || borg_eat(TV_FOOD, sv_food_waybread)
            || borg_quaff_potion(sv_potion_cure_light)
            || borg_quaff_potion(sv_potion_cure_serious)
            || borg_quaff_crit(false) || borg_use_staff_fail(sv_staff_curing)
            || borg_zap_rod(sv_rod_curing)
            || borg_activate_item(act_food_waybread)) {
            return true;
        }
    }

    /* Hack -- cure confusion */
    if (borg.trait[BI_ISCONFUSED] && (q < 25)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)
            || borg_quaff_potion(sv_potion_cure_serious)
            || borg_quaff_crit(false) || borg_use_staff_fail(sv_staff_curing)
            || borg_activate_item(act_cure_confusion)
            || borg_zap_rod(sv_rod_curing)) {
            return true;
        }
    }

    /* Hack -- cure fear */
    if (borg.trait[BI_ISAFRAID] && !borg.trait[BI_CRSFEAR] && (q < 25)) {
        if (borg_eat(TV_MUSHROOM, sv_mush_cure_mind)
            || borg_quaff_potion(sv_potion_boldness)
            || borg_quaff_potion(sv_potion_heroism)
            || borg_quaff_potion(sv_potion_berserk)
            || borg_activate_item(act_rem_fear_pois)) {
            return true;
        }
    }

    /* Hack -- satisfy hunger */
    if ((borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK]) && (q < 25)) {
        if (borg_read_scroll(sv_scroll_satisfy_hunger)
            || borg_activate_item(act_satisfy)) {
            return true;
        }
    }

    /* Hack -- heal damage */
    if ((borg.trait[BI_CURHP] < borg.trait[BI_MAXHP] / 2) && (q < 25)) {
        if (borg_zap_rod(sv_rod_healing)
            || borg_quaff_potion(sv_potion_cure_serious)
            || borg_quaff_crit(false) || borg_activate_item(act_cure_serious)) {
            return true;
        }
    }

    /* Hack -- Rest to recharge Rods of Healing or Recall*/
    if (borg.has[kv_rod_recall] || borg.has[kv_rod_healing]) {
        /* Step 1.  Recharge just 1 rod. */
        if ((borg.has[kv_rod_healing]
                && !borg_items[borg_slot(TV_ROD, sv_rod_healing)].pval)
            || (borg.has[kv_rod_recall]
                && !borg_items[borg_slot(TV_ROD, sv_rod_recall)].pval)) {
            /* Mages can cast the recharge spell */

            /* Rest until at least one recharges */
            if (!borg.trait[BI_ISWEAK] && !borg.trait[BI_ISCUT]
                && !borg.trait[BI_ISHUNGRY] && !borg.trait[BI_ISPOISONED]
                && borg_check_rest(borg.c.y, borg.c.x)
                && !borg_spell_okay(RECHARGING)) {
                /* Take note */
                borg_note("# Resting to recharge a rod...");

                /* Reset the Bouncing-borg Timer */
                borg.time_this_panel = 0;

                /* Rest until done */
                borg_keypress('R');
                borg_keypress('1');
                borg_keypress('0');
                borg_keypress('0');
                borg_keypress(KC_ENTER);

                /* I'm not in a store */
                borg.in_shop = false;

                /* Done */
                return true;
            }
        }
    }

    /*** Just Rest ***/

    /* Hack -- rest until healed */
    if (!borg.trait[BI_ISBLIND] && !borg.trait[BI_ISPOISONED]
        && !borg.trait[BI_ISCUT] && !borg.trait[BI_ISWEAK]
        && !borg.trait[BI_ISHUNGRY]
        && (borg.trait[BI_ISCONFUSED] || borg.trait[BI_ISIMAGE]
            || borg.trait[BI_ISAFRAID] || borg.trait[BI_ISSTUN]
            || borg.trait[BI_ISHEAVYSTUN]
            || borg.trait[BI_CURHP] < borg.trait[BI_MAXHP]
            || borg.trait[BI_CURSP] < borg.trait[BI_MAXSP]
                                          * (borg.trait[BI_CDEPTH] > 85 ? 7 : 6)
                                          / 10)) {
        if (borg_check_rest(borg.c.y, borg.c.x) && !scaryguy_on_level
            && p <= borg_fear_region[borg.c.y / 11][borg.c.x / 11]
            && borg.goal.type != GOAL_RECOVER) {

            /* check for then call lite in dark room before resting */
            if (!borg_check_light_only()) {
                /* Take note */
                borg_note(format("# Resting to recover HP/SP..."));

                /* Rest until done */
                borg_keypress('R');
                borg_keypress('&');
                borg_keypress(KC_ENTER);

                /* Reset our panel clock, we need to be here */
                borg.time_this_panel = 0;

                /* reset the inviso clock to avoid loops */
                borg.need_see_invis = borg_t - 50;

                /* Done */
                return true;
            } else {
                /* Must have been a dark room */
                borg_note(
                    format("# Lighted the darkened room instead of resting."));
                return true;
            }
        }
    }

    /* Hack to recharge mana if a low level mage or priest */
    if (borg.trait[BI_MAXSP]
        && (borg.trait[BI_CLEVEL] <= 40 || borg.trait[BI_CDEPTH] >= 85)
        && borg.trait[BI_CURSP] < (borg.trait[BI_MAXSP] * 8 / 10)
        && p < avoidance * 1 / 10 && borg_check_rest(borg.c.y, borg.c.x)) {
        if (!borg.trait[BI_ISWEAK] && !borg.trait[BI_ISCUT]
            && !borg.trait[BI_ISHUNGRY] && !borg.trait[BI_ISPOISONED]
            && borg.trait[BI_FOOD] > 2 && !borg.munchkin_mode) {
            /* Take note */
            borg_note(format("# Resting to gain Mana. (danger %d)...", p));

            /* Rest until done */
            borg_keypress('R');
            borg_keypress('*');
            borg_keypress(KC_ENTER);

            /* I'm not in a store */
            borg.in_shop = false;

            /* Done */
            return true;
        }
    }

    /* Hack to recharge mana if a low level mage in munchkin mode */
    if (borg.trait[BI_MAXSP] && borg.munchkin_mode == true
        && (borg.trait[BI_CURSP] < borg.trait[BI_MAXSP]
            || borg.trait[BI_CURHP] < borg.trait[BI_MAXHP])
        && borg_check_rest(borg.c.y, borg.c.x)) {
        if (!borg.trait[BI_ISWEAK] && !borg.trait[BI_ISCUT]
            && !borg.trait[BI_ISHUNGRY] && !borg.trait[BI_ISPOISONED]
            && borg.trait[BI_FOOD] > 2
            && (borg_grids[borg.c.y][borg.c.x].feat == FEAT_MORE
                || borg_grids[borg.c.y][borg.c.x].feat == FEAT_LESS)) {
            /* Take note */
            borg_note(format(
                "# Resting to gain munchkin HP/mana. (danger %d)...", p));

            /* Rest until done */
            borg_keypress('R');
            borg_keypress('*');
            borg_keypress(KC_ENTER);

            /* I'm not in a store */
            borg.in_shop = false;

            /* Done */
            return true;
        }
    }

    /* Hack to heal blindness if in munchkin mode */
    if (borg.trait[BI_ISBLIND] && borg.munchkin_mode == true) {
        /* Take note */
        borg_note("# Resting to cure problem. (danger %d)...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('*');
        borg_keypress(KC_ENTER);

        /* I'm not in a store */
        borg.in_shop = false;

        /* Done */
        return true;
    }

    /* Nope */
    return false;
}

#endif
