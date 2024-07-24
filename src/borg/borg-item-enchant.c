/**
 * \file borg-item-enchant.c
 * \brief Enchant weapons and armor
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

#include "borg-item-enchant.h"

#ifdef ALLOW_BORG

#include "../ui-menu.h"

#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-decurse.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * select the given armor
 */
static void borg_pick_armor(int i)
{
    /* Choose from equipment */
    if (i >= INVEN_WIELD) {
        /* if there is armor in inventory, you have to press */
        /* '/' to get to equipment */
        bool found = false;
        for (int e = 0; e < z_info->pack_size; e++) {
            if (borg_items[e].iqty
                && (borg_items[e].tval == TV_BOOTS
                    || borg_items[e].tval == TV_GLOVES
                    || borg_items[e].tval == TV_CLOAK
                    || borg_items[e].tval == TV_CROWN
                    || borg_items[e].tval == TV_HELM
                    || borg_items[e].tval == TV_SHIELD
                    || borg_items[e].tval == TV_HELM
                    || borg_items[e].tval == TV_SOFT_ARMOR
                    || borg_items[e].tval == TV_HARD_ARMOR
                    || borg_items[e].tval == TV_DRAG_ARMOR)) {
                found = true;
                break;
            }
        }
        if (found)
            borg_keypress('/');

        /* Choose that item */
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
    } else
        /* Choose that item */
        borg_keypress(all_letters_nohjkl[i]);
}

/* select the given weapon */
static void borg_pick_weapon(int i)
{
    /* Choose from equipment */
    if (i < INVEN_WIELD) {
        borg_keypress(all_letters_nohjkl[i]);
    } else {
        /* if there is a weapon in inventory, you have to press */
        /* '/' to get to equipment or '|' to go to quiver */
        bool found = false;
        for (int e = 0; e < z_info->pack_size; e++) {
            if (borg_items[e].iqty
                && (borg_items[e].tval == TV_BOW
                    || borg_items[e].tval == TV_DIGGING
                    || borg_items[e].tval == TV_HAFTED
                    || borg_items[e].tval == TV_POLEARM
                    || borg_items[e].tval == TV_SWORD)) {
                found = true;
                break;
            }
        }

        if (i < QUIVER_START) {
            if (found)
                borg_keypress('/');

            borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
        } else {
            /* Quiver Slot */
            if (found || borg_items[INVEN_WIELD].iqty != 0
                || borg_items[INVEN_BOW].iqty != 0)
                borg_keypress('|');
            borg_keypress('0' + (i - QUIVER_START));
        }
    }
}

/*
 * Enchant armor, not including my swap armour
 */
static bool borg_enchant_to_a(void)
{
    int i, b_i = -1;
    int a, b_a = 99;

    /* Nothing to enchant */
    if (!borg.trait[BI_NEED_ENCHANT_TO_A])
        return false;

    /* Need "enchantment" ability */
    if ((!borg.trait[BI_AENCH_ARM]) && (!borg.trait[BI_AENCH_SARM]))
        return false;

    /* Look for armor that needs enchanting */
    for (i = INVEN_BODY; i < INVEN_TOTAL; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip non-identified items */
        if (!item->ident)
            continue;

        /* Obtain the bonus */
        a = item->to_a;

        /* Skip "boring" items */
        if (borg_spell_okay_fail(ENCHANT_ARMOUR, 65)
            || borg.trait[BI_AENCH_SARM] >= 1) {
            if (a >= borg_cfg[BORG_ENCHANT_LIMIT])
                continue;
        } else {
            if (a >= 8)
                continue;
        }

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a))
            continue;

        /* Save the info */
        b_i = i;
        b_a = a;
    }

    /* Nothing */
    if (b_i < 0)
        return false;

    /* Enchant it */
    if (borg_read_scroll(sv_scroll_star_enchant_armor)
        || borg_read_scroll(sv_scroll_enchant_armor)
        || borg_spell_fail(ENCHANT_ARMOUR, 65)) {
        borg_pick_armor(b_i);

        /* Success */
        return true;
    }

    /* Nothing to do */
    return false;
}

/*
 * Enchant weapons to hit
 */
static bool borg_enchant_to_h(void)
{
    int i, b_i = -1;
    int a, s_a, b_a = 99;

    /* Nothing to enchant */
    if (!borg.trait[BI_NEED_ENCHANT_TO_H] && !enchant_weapon_swap_to_h)
        return false;

    /* Need "enchantment" ability */
    if ((!borg.trait[BI_AENCH_TOH]) && (!borg.trait[BI_AENCH_SWEP]))
        return false;

    /* Look for a weapon that needs enchanting */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip non-identified items */
        if (!item->ident)
            continue;

        /* Skip my swap digger */
        if (item->tval == TV_DIGGING)
            continue;

        /* Obtain the bonus */
        a = item->to_h;

        /* Skip "boring" items */
        if (borg_spell_okay_fail(ENCHANT_WEAPON, 65)
            || borg.trait[BI_AENCH_SWEP] >= 1) {
            if (a >= borg_cfg[BORG_ENCHANT_LIMIT])
                continue;
        } else {
            if (a >= 8)
                continue;
        }

        /* Most classes store the enchants until they get
         * a 3x shooter (like a long bow).
         * Generally, we do not want the x2 shooter enchanted,
         * since it wastes scrolls.  But if the sword is at +5
         * and the sling at +2, then the sling will be selected
         * because its enchantment is lower.  The borg tries to
         * enchant the least enchanted item.  This will make sure
         * the x2 shooter is skipped and the sword is enchanted,
         * if needed.  If the sword is at +9,+9, and the sling at
         * +0,+0 and the borg has some enchant scrolls, he should
         * store them instead of wasting them on the sling.
         */
        if (i == INVEN_BOW && /* bow */
            borg.trait[BI_AMMO_POWER] < 3 && /* 3x shooter */
            (!item->art_idx && !item->ego_idx)) /* Not artifact or ego */
            continue;

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a))
            continue;

        /* Save the info */
        b_i = i;
        b_a = a;
    }
    if (weapon_swap) {
        bool       skip = false;
        borg_item *item = &borg_items[weapon_swap - 1];

        /* Skip my swap digger and anything unid'd */
        if (item->ident && item->tval != TV_DIGGING) {
            /* Obtain the bonus */
            s_a = item->to_h;

            /* Skip items that are already enchanted */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65)
                || borg.trait[BI_AENCH_SWEP] >= 1) {
                if (s_a >= borg_cfg[BORG_ENCHANT_LIMIT])
                    skip = true;
            } else {
                if (s_a >= 8)
                    skip = true;
            }

            /* Find the least enchanted item */
            if (b_a > s_a && !skip) {
                /* Save the info */
                b_i = weapon_swap - 1;
                b_a = s_a;
            }
        }
    }
    /* Nothing, check ammo */
    if (b_i < 0) {
        /* look through inventory for ammo */
        for (i = QUIVER_START; i < QUIVER_END; i++) {
            borg_item *item = &borg_items[i];

            /* Only enchant ammo if we have a good shooter,
             * otherwise, store the enchants in the home.
             */
            if (borg.trait[BI_AMMO_POWER] < 3
                || (!borg_items[INVEN_BOW].art_idx
                    && !borg_items[INVEN_BOW].ego_idx))
                continue;

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5)
                continue;

            /* Skip non-identified items  */
            if (!item->ident)
                continue;

            /* Make sure it is the right type if missile */
            if (item->tval != borg.trait[BI_AMMO_TVAL])
                continue;

            /* Obtain the bonus  */
            a = item->to_h;

            /* Skip items that are already enchanted */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65)
                || borg.trait[BI_AENCH_SWEP] >= 1) {
                if (a >= 10)
                    continue;
            } else {
                if (a >= 8)
                    continue;
            }

            /* Find the least enchanted item */
            if ((b_i >= 0) && (b_a < a))
                continue;

            /* Save the info  */
            b_i = i;
            b_a = a;
        }
    }

    /* Nothing */
    if (b_i < 0)
        return false;

    /* Enchant it */
    if (borg_read_scroll(sv_scroll_star_enchant_weapon)
        || borg_read_scroll(sv_scroll_enchant_weapon_to_hit)
        || borg_spell_fail(ENCHANT_WEAPON, 65)) {
        borg_pick_weapon(b_i);

        /* Success */
        return true;
    }

    /* Nothing to do */
    return false;
}

/*
 * Enchant weapons to dam
 */
static bool borg_enchant_to_d(void)
{
    int i, b_i = -1;
    int a, s_a, b_a = 99;

    /* Nothing to enchant */
    if (!borg.trait[BI_NEED_ENCHANT_TO_D] && !enchant_weapon_swap_to_d)
        return false;

    /* Need "enchantment" ability */
    if ((!borg.trait[BI_AENCH_TOD]) && (!borg.trait[BI_AENCH_SWEP]))
        return false;

    /* Look for a weapon that needs enchanting */
    for (i = INVEN_WIELD; i <= INVEN_BOW; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip non-identified items */
        if (!item->ident)
            continue;

        /* Skip my swap digger */
        if (item->tval == TV_DIGGING)
            continue;

        /* Obtain the bonus */
        a = item->to_d;

        /* Skip "boring" items */
        if (borg_spell_okay_fail(ENCHANT_WEAPON, 65)
            || borg.trait[BI_AENCH_SWEP] >= 1) {
            if (a >= borg_cfg[BORG_ENCHANT_LIMIT])
                continue;
        } else {
            if (a >= 8)
                continue;
        }

        /* Most classes store the enchants until they get
         * a 3x shooter (like a long bow).
         * Generally, we do not want the x2 shooter enchanted,
         * since it wastes scrolls.  But if the sword is at +5
         * and the sling at +2, then the sling will be selected
         * because its enchantment is lower.  The borg tries to
         * enchant the least enchanted item.  This will make sure
         * the x2 shooter is skipped and the sword is enchanted,
         * if needed.  If the sword is at +9,+9, and the sling at
         * +0,+0 and the borg has some enchant scrolls, he should
         * store them instead of wasting them on the sling.
         */
        if (i == INVEN_BOW && /* bow */
            borg.trait[BI_AMMO_POWER] < 3 && /* 3x shooter */
            (!item->art_idx && !item->ego_idx)) /* Not artifact or ego */
            continue;

        /* Find the least enchanted item */
        if ((b_i >= 0) && (b_a < a))
            continue;

        /* Save the info */
        b_i = i;
        b_a = a;
    }
    if (weapon_swap) {
        bool       skip = false;
        borg_item *item = &borg_items[weapon_swap - 1];

        /* Skip non-identified items and diggers */
        if (item->ident && item->tval != TV_DIGGING) {
            /* Obtain the bonus */
            s_a = item->to_d;

            /* Skip "boring" items */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65)
                || borg.trait[BI_AENCH_SWEP] >= 1) {
                if (s_a >= borg_cfg[BORG_ENCHANT_LIMIT])
                    skip = true;
            } else {
                if (s_a >= 8)
                    skip = true;
            }

            /* Find the least enchanted item */
            if (b_a > s_a && !skip) {
                /* Save the info */
                b_i = weapon_swap - 1;
                b_a = s_a;
            }
        }
    }
    /* Nothing, check ammo */
    if (b_i < 0) {
        /* look through inventory for ammo */
        for (i = QUIVER_START; i < QUIVER_END; i++) {
            borg_item *item = &borg_items[i];

            /* Only enchant ammo if we have a good shooter,
             * otherwise, store the enchants in the home.
             */
            if (borg.trait[BI_AMMO_POWER] < 3
                || (!borg_items[INVEN_BOW].art_idx
                    && !borg_items[INVEN_BOW].ego_idx))
                continue;

            /* Only enchant ammo if we have a good shooter,
             * otherwise, store the enchants in the home.
             */
            if (borg.trait[BI_AMMO_POWER] < 3)
                continue;

            /* Only enchant if qty >= 5 */
            if (item->iqty < 5)
                continue;

            /* Skip non-identified items  */
            if (!item->ident)
                continue;

            /* Make sure it is the right type if missile */
            if (item->tval != borg.trait[BI_AMMO_TVAL])
                continue;

            /* Obtain the bonus  */
            a = item->to_d;

            /* Skip items that are already enchanted */
            if (borg_spell_okay_fail(ENCHANT_WEAPON, 65)
                || borg.trait[BI_AENCH_SWEP] >= 1) {
                if (a >= 10)
                    continue;
            } else {
                if (a >= 8)
                    continue;
            }

            /* Find the least enchanted item */
            if ((b_i >= 0) && (b_a < a))
                continue;

            /* Save the info  */
            b_i = i;
            b_a = a;
        }
    }

    /* Nothing */
    if (b_i < 0)
        return false;

    /* Enchant it */
    if (borg_read_scroll(sv_scroll_star_enchant_weapon)
        || borg_read_scroll(sv_scroll_enchant_weapon_to_dam)
        || borg_spell_fail(ENCHANT_WEAPON, 65)) {
        borg_pick_weapon(b_i);

        /* Success */
        return true;
    }

    /* Nothing to do */
    return false;
}

/*
 * Brand Bolts
 */
static bool borg_brand_weapon(void)
{
    int i, b_i = -1;
    int a, b_a = 0;

    /* Nothing to brand */
    if (!borg.trait[BI_NEED_BRAND_WEAPON])
        return false;

    /* Need "brand" ability */
    if (!borg.trait[BI_ABRAND])
        return false;

    /* look through inventory for ammo */
    for (i = QUIVER_START; i < QUIVER_END; i++) {
        borg_item *item = &borg_items[i];

        /* Only enchant if qty >= 20 */
        if (item->iqty < 20)
            continue;

        /* Skip non-identified items  */
        if (!item->ident)
            continue;

        /* Make sure it is the right type if missile */
        if (item->tval != borg.trait[BI_AMMO_TVAL])
            continue;

        /* Obtain the bonus  */
        a = item->to_h;

        /* Skip branded items */
        if (item->ego_idx)
            continue;

        /* Find the most enchanted item */
        if ((b_i >= 0) && (b_a > a))
            continue;

        /* Save the info  */
        b_i = i;
        b_a = a;
    }

    /* Nothing to Brand */
    if (b_i == -1)
        return false;

    /* Enchant it */
    if (borg_activate_item(act_firebrand)
        || borg_spell_fail(BRAND_AMMUNITION, 65)) {
        borg_pick_weapon(b_i);

        /* Success */
        return true;
    }

    /* Nothing to do */
    return false;
}

/*
 * Enchant things
 */
bool borg_enchanting(void)
{
    /* Forbid blind/confused */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and finish the job later */
    if ((borg_t - borg_began > 5500 && borg.trait[BI_CDEPTH] >= 1)
        || (borg_t - borg_began > 3501 && borg.trait[BI_CDEPTH] == 0))
        return false;

    /* Remove Curses */
    if (borg_decurse_armour())
        return true;
    if (borg_decurse_weapon())
        return true;
    if (borg_decurse_any())
        return true;

    /* Only in town */
    if (borg.trait[BI_CDEPTH])
        return false;

    /* Enchant things */
    if (borg_brand_weapon())
        return true;
    if (borg_enchant_to_h())
        return true;
    if (borg_enchant_to_d())
        return true;
    if (borg_enchant_to_a())
        return true;

    /* Nope */
    return false;
}

#endif
