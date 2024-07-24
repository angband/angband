/**
 * \file borg-item-use.c
 * \brief Use potions, scrolls, rods, wand, staffs and rings
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

#include "borg-item-use.h"

#ifdef ALLOW_BORG

#include "../effects.h"
#include "../ui-menu.h"

#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-analyze.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Quaff a potion of cure critical wounds.  This is a special case
 *   for several reasons.
 *   1) it is usually the only healing potion we have on us
 *   2) we should try to conserve a couple for when we really need them
 *   3) if we are burning through them fast we should probably teleport out of
 *      the fight.
 *   4) When it is the only/best way out of danger, drink away
 */
bool borg_quaff_crit(bool no_check)
{
    static int16_t when_last_quaff = 0;

    if (no_check) {
        if (borg_quaff_potion(sv_potion_cure_critical)) {
            when_last_quaff = borg_t;
            return true;
        }
        return false;
    }

    /* Avoid drinking CCW twice in a row */
    if (when_last_quaff > (borg_t - 4) && when_last_quaff <= borg_t
        && (randint1(100) < 75))
        return false;

    /* Save the last two for when we really need them */
    if (borg.trait[BI_ACCW] < 2)
        return false;

    if (borg_quaff_potion(sv_potion_cure_critical)) {
        when_last_quaff = borg_t;
        return true;
    }
    return false;
}

/*
 * Attempt to quaff the given potion (by sval)
 */
bool borg_quaff_potion(int sval)
{
    int i;

    /* Look for that potion */
    i = borg_slot(TV_POTION, sval);

    /* None available */
    if (i < 0)
        return false;

    /* Log the message */
    borg_note(format("# Quaffing %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Attempt to quaff an unknown potion
 */
bool borg_quaff_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require correct tval */
        if (item->tval != TV_POTION)
            continue;

        /* Skip aware items */
        if (item->kind)
            continue;

        /* Save this item */
        n = i;
    }

    /* None available */
    if (n < 0)
        return false;

    /* Log the message */
    borg_note(format("# Quaffing unknown potion %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(all_letters_nohjkl[n]);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Hack -- attempt to read the given scroll (by sval)
 */
bool borg_read_scroll(int sval)
{
    int i;

    /* Dark */
    if (no_light(player))
        return false;

    /* Blind or Confused or Amnesia*/
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return false;

    /* Look for that scroll */
    i = borg_slot(TV_SCROLL, sval);

    /* None available */
    if (i < 0)
        return false;

    /* Log the message */
    borg_note(format("# Reading %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress('r');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Hack -- attempt to read an unknown scroll
 */
bool borg_read_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require correct tval */
        if (item->tval != TV_SCROLL)
            continue;

        /* Skip aware items */
        if (item->kind)
            continue;

        /* Save this item */
        n = i;
    }

    /* None available */
    if (n < 0)
        return false;

    /* Dark */
    if (no_light(player))
        return false;

    /* Blind or Confused */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return false;

    /* Log the message */
    borg_note(format("# Reading unknown scroll %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('r');
    borg_keypress(all_letters_nohjkl[n]);

    /* Incase it is ID scroll, ESCAPE out. */
    borg_keypress(ESCAPE);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Hack -- attempt to eat the given food or mushroom
 */
bool borg_eat(int tval, int sval)
{
    int i;

    /* Look for that food */
    i = borg_slot(tval, sval);

    /* None available */
    if (i < 0)
        return false;

    /* Log the message */
    borg_note(format("# Eating %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Hack -- attempt to eat an unknown food/mushroom.
 * This is done in emergencies.
 */
bool borg_eat_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require correct tval */
        if (item->tval != TV_FOOD && item->tval != TV_MUSHROOM)
            continue;

        /* Skip aware items */
        if (item->kind)
            continue;

        /* Save this item */
        n = i;
    }

    /* None available */
    if (n < 0)
        return false;

    /* Log the message */
    borg_note(format("# Eating unknown mushroom %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(all_letters_nohjkl[n]);

    /* Hack -- Clear "shop" goals */
    borg.goal.shop = borg.goal.ware = borg.goal.item = -1;

    /* Success */
    return true;
}

/*
 * Prevent starvation by any means possible
 */
bool borg_eat_food_any(void)
{
    int i;

    /* Scan the inventory for "normal" food */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip unknown food */
        if (!item->kind)
            continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD)
            continue;

        /* Eat something of that type */
        if (borg_eat(item->tval, item->sval))
            return true;
    }

    /* Scan the inventory for "okay" food */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip unknown food */
        if (!item->kind)
            continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD && item->tval != TV_MUSHROOM)
            continue;

        /* Skip non-food */
        if (!borg_obj_has_effect(item->kind, EF_NOURISH, 0)
            && !borg_obj_has_effect(item->kind, EF_NOURISH, 2)
            && !borg_obj_has_effect(item->kind, EF_NOURISH, 3))
            continue;

        /* Eat something of that type */
        if (borg_eat(item->tval, item->sval))
            return true;
    }

    /*
     * Try potions that can provide nutrition.  First try ones that are
     * pure nutrition without additional effects.
     */
    if (borg_quaff_potion(sv_potion_slime_mold))
        return true;
    /*
     * Then try those that, besides the nourishment, only have negative
     * effects.  But only try if there's protection against the negative effect.
     */
    if (((borg.trait[BI_FRACT])
            && (borg_quaff_potion(sv_potion_sleep)
                || borg_quaff_potion(sv_potion_slowness)))
        || ((borg.trait[BI_RBLIND]) && (borg_quaff_potion(sv_potion_blindness)))
        || ((borg.trait[BI_RCONF])
            && (borg_quaff_potion(sv_potion_confusion)))) {
        return true;
    }
    /* Consume in order, when hurting */
    if ((borg.trait[BI_CURHP] < 4
            || (borg.trait[BI_CURHP] <= borg.trait[BI_MAXHP]))
        && (borg_quaff_potion(sv_potion_cure_light)
            || borg_quaff_potion(sv_potion_cure_serious)
            || borg_quaff_potion(sv_potion_cure_critical)
            || borg_quaff_potion(sv_potion_healing))) {
        return true;
    }

    /* Nothing */
    return false;
}
/*
 * Hack -- checks rod (by sval) and
 * make a fail check on it.
 */
bool borg_equips_rod(int sval)
{
    int i, skill, lev;
    int fail;

    /* Look for that staff */
    i = borg_slot(TV_ROD, sval);

    /* None available */
    if (i < 0)
        return false;

    /* No charges */
    if (!borg_items[i].pval)
        return false;

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg.trait[BI_DEV];

    /* Confusion hurts skill */
    if (borg.trait[BI_ISCONFUSED])
        skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage (at least 1/2 chance of success. */
    if (fail > 500)
        return false;

    /* Yep we got one */
    return true;
}

/*
 * Hack -- attempt to zap the given (charged) rod (by sval)
 */
bool borg_zap_rod(int sval)
{
    int i, lev, fail;
    int skill;

    /* Look for that rod */
    i = borg_slot(TV_ROD, sval);

    /* None available */
    if (i < 0)
        return false;

    /* Hack -- Still charging */
    if (!borg_items[i].pval)
        return false;

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg.trait[BI_DEV];

    /* Confusion hurts skill */
    if (borg.trait[BI_ISCONFUSED])
        skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage */
    if (sval != sv_rod_recall) {
        if (fail > 500)
            return false;
    }

    /* Log the message */
    borg_note(format("# Zapping %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('z');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return true;
}

/*
 * Hack -- attempt to use the given (charged) staff (by sval)
 */
bool borg_use_staff(int sval)
{
    int i;

    /* Look for that staff */
    i = borg_slot(TV_STAFF, sval);

    /* None available */
    if (i < 0)
        return false;

    /* No charges */
    if (!borg_items[i].pval)
        return false;

    /* Log the message */
    borg_note(format("# Using %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return true;
}

/*
 * Hack -- attempt to use an unknown staff.  This is done in emergencies.
 */
bool borg_use_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require correct tval */
        if (item->tval != TV_STAFF)
            continue;

        /* Skip aware items */
        if (item->kind)
            continue;

        /* Save this item */
        n = i;
    }

    /* None available */
    if (n < 0)
        return false;

    /* Log the message */
    borg_note(format("# Using unknown Staff %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(all_letters_nohjkl[n]);

    /* Incase it is ID staff, ESCAPE out. */
    borg_keypress(ESCAPE);

    /* Success */
    return true;
}

/*
 * Hack -- attempt to use the given (charged) staff (by sval) and
 * make a fail check on it.
 */
bool borg_use_staff_fail(int sval)
{
    int i, fail, lev;
    int skill;

    /* Look for that staff */
    i = borg_slot(TV_STAFF, sval);

    /* None available */
    if (i < 0)
        return false;

    /* No charges */
    if (!borg_items[i].pval)
        return false;

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg.trait[BI_DEV];

    /* Confusion hurts skill */
    if (borg.trait[BI_ISCONFUSED])
        skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500) {
        if (sval != sv_staff_teleportation) {
            return false;
        }

        /* We need to give some "desperation attempt to teleport staff" */
        if (!borg.trait[BI_ISCONFUSED] && !borg.trait[BI_ISBLIND]) /* Dark? */
        {
            /* We really have no chance, return false, attempt the scroll */
            if (fail > 500)
                return false;
        }
        /* We might have a slight chance, or we cannot not read */
    }

    /* Log the message */
    borg_note(format("# Using %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return true;
}

/*
 * Hack -- checks staff (by sval) and
 * make a fail check on it.
 */
bool borg_equips_staff_fail(int sval)
{
    int i, fail, lev;
    int skill;

    /* Look for that staff */
    i = borg_slot(TV_STAFF, sval);

    /* None available */
    if (i < 0)
        return false;

    /* No charges */
    if (!borg_items[i].pval)
        return false;

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg.trait[BI_DEV];

    /* Confusion hurts skill */
    if (borg.trait[BI_ISCONFUSED])
        skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* If its a Destruction, we only use it in emergencies, attempt it */
    if (sval == sv_staff_destruction) {
        return true;
    }

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500) {
        /* No real chance of success on other types of staffs */
        if (sval != sv_staff_teleportation) {
            return false;
        }

        /* We need to give some "desperation attempt to teleport staff" */
        if (sval == sv_staff_teleportation && !borg.trait[BI_ISCONFUSED]) {
            /* We really have no chance, return false, attempt the scroll */
            if (fail < 650)
                return false;
        }

        /* We might have a slight chance (or its a Destruction), continue on */
    }

    /* Yep we got one */
    return true;
}

/*
 * Hack -- attempt to aim the given (charged) wand (by sval)
 */
bool borg_aim_wand(int sval)
{
    int i;

    /* Look for that wand */
    i = borg_slot(TV_WAND, sval);

    /* None available */
    if (i < 0)
        return false;

    /* No charges */
    if (!borg_items[i].pval)
        return false;

    /* Log the message */
    borg_note(format("# Aiming %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('a');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return true;
}

/*
 * Hack -- check and see if borg is wielding a ring and if
 * he will pass a fail check.
 */
bool borg_equips_ring(int ring_sval)
{
    int lev, fail, i;
    int skill;

    for (i = INVEN_RIGHT; i < INVEN_LEFT; i++) {
        borg_item *item = &borg_items[i];

        /* Skip incorrect armours */
        if (item->tval != TV_RING)
            continue;
        if (item->sval != ring_sval)
            continue;

        /* Check charge */
        if (item->timeout)
            continue;

        /*  Make Sure is IDed */
        if (!item->ident)
            continue;

        /* check on fail rate
         */

        /* Extract the item level */
        lev = borg_items[i].level;

        /* Base chance of success */
        skill = borg.trait[BI_DEV];

        /* Confusion hurts skill */
        if (borg.trait[BI_ISCONFUSED])
            skill = skill * 75 / 100;

        /* High level objects are harder */
        fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

        /* Roll for usage, but if its a Teleport be generous. */
        if (fail > 500)
            continue;

        /* Success */
        return true;
    }

    return false;
}

/*
 *  Hack -- attempt to use the given ring
 */
bool borg_activate_ring(int ring_sval)
{
    int i;

    /* Check the equipment */
    for (i = INVEN_RIGHT; i < INVEN_LEFT; i++) {
        borg_item *item = &borg_items[i];

        /* Skip incorrect mails */
        if (item->tval != TV_RING)
            continue;
        if (item->sval != ring_sval)
            continue;

        /* Check charge */
        if (item->timeout)
            continue;

        /*  Make Sure item is IDed */
        if (!item->ident)
            continue;

        /* Log the message */
        borg_note(format("# Activating ring %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);

        /* Success */
        return true;
    }

    return false;
}

/*
 * Hack -- check and see if borg is wielding a dragon armor and if
 * he will pass a fail check.
 */
bool borg_equips_dragon(int drag_sval)
{
    int lev, fail;
    int skill;
    int numerator;
    int denominator;

    /* Check the equipment */
    borg_item *item = &borg_items[INVEN_BODY];

    /* Skip incorrect armours */
    if (item->tval != TV_DRAG_ARMOR)
        return false;
    if (item->sval != drag_sval)
        return false;

    /* Check charge */
    if (item->timeout)
        return false;

    /*  Make Sure Mail is IDed */
    if (!item->ident)
        return false;

    /* check on fail rate
     * The fail check is automatic for dragon armor.  It is an attack
     * item.  He should not sit around failing 5 or 6 times in a row.
     * he should attempt to activate it, and if he is likely to fail, then
     * eh should look at a different attack option.  We are assuming
     * that the fail rate is about 50%.  So He may still try to activate it
     * and fail.  But he will not even try if he has negative chance or
     * less than twice the USE_DEVICE variable
     */
    /* Extract the item level */
    lev = borg_items[INVEN_BODY].level;

    /* Base chance of success */
    skill = borg.trait[BI_DEV];

    /* Confusion hurts skill */
    if (borg.trait[BI_ISCONFUSED])
        skill = skill * 75 / 100;

    /* High level objects are harder */
    numerator   = (skill - lev) - (141 - 1);
    denominator = (lev - skill) - (100 - 10);

    /* Make sure that we don't divide by zero */
    if (denominator == 0)
        denominator = numerator > 0 ? 1 : -1;

    fail = (100 * numerator) / denominator;

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500)
        return false;

    /* Success */
    return true;
}

/*
 *  Hack -- attempt to use the given dragon armour
 */
bool borg_activate_dragon(int drag_sval)
{
    /* Check the equipment */

    borg_item *item = &borg_items[INVEN_BODY];

    /* Skip incorrect mails */
    if (item->tval != TV_DRAG_ARMOR)
        return false;
    if (item->sval != drag_sval)
        return false;

    /* Check charge */
    if (item->timeout)
        return false;

    /*  Make Sure Mail is IDed */
    if (!item->ident)
        return false;

    /* Log the message */
    borg_note(format("# Activating dragon scale %s.", item->desc));

    /* Perform the action */
    borg_keypress('A');
    borg_keypress(all_letters_nohjkl[INVEN_BODY - INVEN_WIELD]);

    /* Success */
    return true;
}

/*
 * Attempt to use the given artifact
 */
bool borg_activate_item(int activation)
{
    int i;

    /* a quick check of the array */
    if (!borg.activation[activation])
        return false;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
        borg_item *item = &borg_items[i];

        /* Skip wrong activation*/
        if (item->activ_idx != activation)
            continue;

        /* Check charge */
        if (item->timeout)
            continue;

        /* Log the message */
        borg_note(format("# Activating item %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);

        /* Success */
        return true;
    }

    /* Oops */
    return false;
}

/*
 * Hack -- check and see if borg is wielding an item with this activation
 */
bool borg_equips_item(int activation, bool check_charge)
{
    int i;

    /* a quick check of the array */
    if (!borg.activation[activation])
        return false;
    else if (!check_charge)
        return true;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
        borg_item *item = &borg_items[i];

        /* Skip wrong activation */
        if (item->activ_idx != activation)
            continue;

        /* Check charge.  */
        if (check_charge && (item->timeout >= 1))
            continue;

        /* Success */
        return true;
    }

    /* I do not have it or it is not charged */
    return false;
}

/* Return the relative chance for failure to activate an item.
 * The lower the number, the better the chance of success
 * 200 is 80% of success
 * 600 is 40% chance of success
 */
int borg_activate_failure(int tval, int sval)
{
    int lev;
    int skill;
    int fail;
    int i;

    /* Look for that item */
    i = borg_slot(tval, sval);

    /* None available */
    if (i < 0)
        return 100;

    /* No charges */
    if (!borg_items[i].pval)
        return 100;

    /* no known activation */
    if (!borg_items[i].activ_idx)
        return 100;

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg.trait[BI_DEV];

    /* Confusion hurts skill */
    if (borg.trait[BI_ISCONFUSED])
        skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Yep we got one */
    return (fail);
}

/*
 * Use things in a useful, but non-essential, manner
 */
bool borg_use_things(void)
{
    int i;

    /* Quaff experience restoration potion */
    if (borg.trait[BI_ISFIXEXP]
        && (borg_spell(REVITALIZE) || borg_spell(REMEMBRANCE)
            || (borg.trait[BI_CURHP] > 90 && borg_spell(UNHOLY_REPRIEVE))
            || borg_activate_item(act_restore_exp)
            || borg_activate_item(act_restore_st_lev)
            || borg_activate_item(act_restore_life)
            || borg_quaff_potion(sv_potion_restore_life))) {
        return true;
    }

    /* just drink the stat gains, at this dlevel we wont need cash */
    if (borg_quaff_potion(sv_potion_inc_str)
        || borg_quaff_potion(sv_potion_inc_int)
        || borg_quaff_potion(sv_potion_inc_wis)
        || borg_quaff_potion(sv_potion_inc_dex)
        || borg_quaff_potion(sv_potion_inc_con)) {
        return true;
    }

    /* Quaff potions of "restore" stat if needed */
    if ((borg.trait[BI_ISFIXSTR]
            && (borg_quaff_potion(sv_potion_inc_str)
                || borg_eat(TV_MUSHROOM, sv_mush_purging)
                || borg_activate_item(act_shroom_purging)
                || borg_activate_item(act_restore_str)
                || borg_activate_item(act_restore_all)
                || borg_eat(TV_MUSHROOM, sv_mush_restoring)))
        || (borg.trait[BI_ISFIXINT]
            && (borg_quaff_potion(sv_potion_inc_int)
                || borg_activate_item(act_restore_int)
                || borg_activate_item(act_restore_all)
                || borg_eat(TV_MUSHROOM, sv_mush_restoring)))
        || (borg.trait[BI_ISFIXWIS]
            && (borg_quaff_potion(sv_potion_inc_wis)
                || borg_activate_item(act_restore_wis)
                || borg_activate_item(act_restore_all)
                || borg_eat(TV_MUSHROOM, sv_mush_restoring)))
        || (borg.trait[BI_ISFIXDEX]
            && (borg_quaff_potion(sv_potion_inc_dex)
                || borg_activate_item(act_restore_dex)
                || borg_activate_item(act_restore_all)
                || borg_eat(TV_MUSHROOM, sv_mush_restoring)))
        || (borg.trait[BI_ISFIXCON]
            && (borg_quaff_potion(sv_potion_inc_con)
                || borg_activate_item(act_restore_con)
                || borg_activate_item(act_restore_all)
                || borg_eat(TV_MUSHROOM, sv_mush_purging)
                || borg_activate_item(act_shroom_purging)
                || borg_eat(TV_MUSHROOM, sv_mush_restoring)))) {
        return true;
    }

    /* Use some items right away */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Process "force" items */
        switch (item->tval) {
        case TV_POTION: {
            /* Check the scroll */
            if (item->sval == sv_potion_enlightenment) {
                /* Never quaff these in town */
                if (!borg.trait[BI_CDEPTH])
                    break;
            } else if (item->sval == sv_potion_inc_all)
                /* Try quaffing the potion */
                if (borg_quaff_potion(item->sval))
                    return true;

            break;
        }
        case TV_SCROLL: {
            /* Hack -- check Blind/Confused */
            if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
                break;

            /* XXX XXX XXX Dark */

            /* Check the scroll */
            if (item->sval == sv_scroll_mapping
                || item->sval == sv_scroll_acquirement
                || item->sval == sv_scroll_star_acquirement) {
                /* Never read these in town */
                if (!borg.trait[BI_CDEPTH])
                    break;

                /* Try reading the scroll */
                if (borg_read_scroll(item->sval))
                    return true;
            }

            break;
        }
        }
    }

    /* Eat food */
    if (borg.trait[BI_ISHUNGRY]) {
        /* Attempt to satisfy hunger */
        if (borg_spell(REMOVE_HUNGER) || borg_spell(HERBAL_CURING)
            || borg_quaff_potion(sv_potion_slime_mold)
            || borg_eat(TV_FOOD, sv_food_slime_mold)
            || borg_eat(TV_FOOD, sv_food_slice)
            || borg_eat(TV_FOOD, sv_food_apple)
            || borg_eat(TV_FOOD, sv_food_pint)
            || borg_eat(TV_FOOD, sv_food_handful)
            || borg_eat(TV_FOOD, sv_food_honey_cake)
            || borg_eat(TV_FOOD, sv_food_ration)
            || borg_eat(TV_FOOD, sv_food_waybread)
            || borg_eat(TV_FOOD, sv_food_draught)
            || borg_activate_item(act_food_waybread)) {
            return true;
        }
    }

    /* Nothing to do */
    return false;
}

/*
 * Recharge things
 *
 * XXX XXX XXX Prioritize available items
 */
bool borg_recharging(void)
{
    int  i      = -1;
    bool charge = false;

    /* Forbid blind/confused */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return false;

    /* XXX XXX XXX Dark */

    /* Look for an item to recharge */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip non-identified items */
        if (!item->ident)
            continue;

        if (item->note && strstr(item->note, "empty"))
            continue;

        /* assume we can't charge it. */
        charge = false;

        /* Wands with no charges can be charged */
        if ((item->tval == TV_WAND) && (item->pval <= 1))
            charge = true;

        /* recharge staves sometimes */
        if (item->tval == TV_STAFF) {
            /* Allow staves to be recharged at 2 charges if
             * the borg has the big recharge spell. And its not a *Dest*
             */
            if ((item->pval < 2) && (borg_spell_okay_fail(RECHARGING, 96))
                && item->sval < sv_staff_power)
                charge = true;

            /* recharge any staff at 0 charges */
            if (item->pval <= 1)
                charge = true;

            /* Staves of teleport get recharged at 2 charges in town */
            if ((item->sval == sv_staff_teleportation) && (item->pval < 3)
                && !borg.trait[BI_CDEPTH])
                charge = true;

            /* They stack.  If quantity is 4 and pval is 1, then there are 4
             * charges. */
            if ((item->iqty + item->pval >= 4) && item->pval >= 1)
                charge = false;
        }

        /* get the next item if we are not to charge this one */
        if (!charge)
            continue;

        /* Attempt to recharge */
        if (borg_read_scroll(sv_scroll_recharging)
            || borg_spell_fail(RECHARGING, 96)
            || borg_activate_item(act_recharge)) {
            /* Message */
            borg_note(format("Recharging %s with current charge of %d",
                item->desc, item->pval));

            /* Recharge the item */
            borg_keypress(all_letters_nohjkl[i]);

            /* Remove the {empty} if present */
            if (item->note && strstr(item->note, "empty"))
                borg_deinscribe(i);

            /* Success */
            return true;
        } else
            /* if we fail once, no need to try again. */
            break;
    }

    /* Nope */
    return false;
}

#endif
