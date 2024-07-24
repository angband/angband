/**
 * \file borg-item-decurse.c
 * \brief Handle removing curses
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

#include "borg-item-decurse.h"

#ifdef ALLOW_BORG

#include "ui-menu.h"

#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg.h"

/*
 * Remove Curse swap armour
 */
bool borg_decurse_armour(void)
{
    /* Nothing to decurse */
    if (!borg_cfg[BORG_USES_SWAPS] || !decurse_armour_swap)
        return false;

    if (-1 == borg_slot(TV_SCROLL, sv_scroll_remove_curse)
        && !borg_equips_staff_fail(sv_staff_remove_curse)
        && !borg_spell_okay_fail(REMOVE_CURSE, 40)
        && -1 == borg_slot(TV_SCROLL, sv_scroll_star_remove_curse)
        && !borg_equips_item(act_remove_curse, true)
        && !borg_equips_item(act_remove_curse2, true)) {
        return false;
    }

    /* remove the curse */
    if (borg_read_scroll(sv_scroll_remove_curse)
        || borg_use_staff(sv_staff_remove_curse) || borg_spell(REMOVE_CURSE)
        || borg_read_scroll(sv_scroll_star_remove_curse)
        || borg_activate_item(act_remove_curse)
        || borg_activate_item(act_remove_curse2)) {
        /* pick the item */
        borg_keypress(all_letters_nohjkl[armour_swap - 1]);
        /* pick first curse */
        borg_keypress(KC_ENTER);

        /* Shekockazol! */
        return true;
    }

    /* Nothing to do */
    return false;
}

/*
 * Remove Curse swap weapon
 */
bool borg_decurse_weapon(void)
{
    /* Nothing to decurse */
    if (!borg_cfg[BORG_USES_SWAPS] || !decurse_weapon_swap)
        return false;

    /* Ability for curses */
    if (-1 == borg_slot(TV_SCROLL, sv_scroll_remove_curse)
        && !borg_equips_staff_fail(sv_staff_remove_curse)
        && !borg_spell_okay_fail(REMOVE_CURSE, 40)
        && -1 == borg_slot(TV_SCROLL, sv_scroll_star_remove_curse)
        && !borg_equips_item(act_remove_curse, true)
        && !borg_equips_item(act_remove_curse2, true)) {
        return false;
    }

    /* remove the curse */
    if (borg_read_scroll(sv_scroll_remove_curse)
        || borg_use_staff(sv_staff_remove_curse) || borg_spell(REMOVE_CURSE)
        || borg_read_scroll(sv_scroll_star_remove_curse)
        || borg_activate_item(act_remove_curse)
        || borg_activate_item(act_remove_curse2)) {
        borg_keypress(all_letters_nohjkl[weapon_swap - 1]);
        /* pick first curse */
        borg_keypress(KC_ENTER);

        /* Shekockazol! */
        return true;
    }

    /* Nothing to do */
    return false;
}

/*
 * Remove Curse any
 */
bool borg_decurse_any(void)
{
    if (borg.trait[BI_FIRST_CURSED]) {
        if (-1 == borg_slot(TV_SCROLL, sv_scroll_remove_curse)
            && !borg_equips_staff_fail(sv_staff_remove_curse)
            && !borg_spell_okay_fail(REMOVE_CURSE, 40)
            && -1 == borg_slot(TV_SCROLL, sv_scroll_star_remove_curse)
            && !borg_equips_item(act_remove_curse, true)
            && !borg_equips_item(act_remove_curse2, true)) {
            return false;
        }

        /* remove the curse */
        if (borg_read_scroll(sv_scroll_remove_curse)
            || borg_use_staff(sv_staff_remove_curse) || borg_spell(REMOVE_CURSE)
            || borg_read_scroll(sv_scroll_star_remove_curse)
            || borg_activate_item(act_remove_curse)
            || borg_activate_item(act_remove_curse2)) {
            /* pick the item */
            if (borg.trait[BI_FIRST_CURSED] <= INVEN_WIELD) {
                borg_keypress(
                    all_letters_nohjkl[borg.trait[BI_FIRST_CURSED] - 1]);
            } else if (borg.trait[BI_FIRST_CURSED] <= QUIVER_START) {
                if (borg.trait[BI_WHERE_CURSED] & BORG_INVEN)
                    borg_keypress('/');

                borg_keypress(all_letters_nohjkl[borg.trait[BI_FIRST_CURSED]
                                                 - INVEN_WIELD - 1]);
            } else {
                if (borg.trait[BI_WHERE_CURSED] & 1
                    || borg.trait[BI_WHERE_CURSED] & BORG_EQUIP)
                    borg_keypress('|');
                borg_keypress(
                    '0' + (borg.trait[BI_FIRST_CURSED] - 1 - QUIVER_START));
            }
            /* pick first curse */
            borg_keypress(KC_ENTER);

            /* Shekockazol! */
            return true;
        }
    }

    /* Nothing to do */
    return false;
}

#endif
