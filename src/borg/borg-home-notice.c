/**
 * \file borg-home-notice.c
 * \brief Extract the bonuses for items in the home.
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

#include "borg-home-notice.h"

#ifdef ALLOW_BORG

#include "../init.h"
#include "../obj-util.h"

#include "borg-item-activation.h"
#include "borg-item-analyze.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-store.h"
#include "borg-trait.h"

/*
 * Various "amounts" (for the home)
 */

int16_t num_food;
int16_t num_fuel;
int16_t num_mold;
int16_t num_ident;
int16_t num_recall;
int16_t num_phase;
int16_t num_escape;
int16_t num_tele_staves;
int16_t num_teleport;
int16_t num_berserk;
int16_t num_teleport_level;
int16_t num_recharge;

int16_t num_cure_critical;
int16_t num_cure_serious;

int16_t num_pot_rheat;
int16_t num_pot_rcold;

int16_t num_missile;

int16_t num_book[9];

int16_t num_fix_stat[STAT_MAX];
int16_t home_stat_add[STAT_MAX];

int16_t num_fix_exp;
int16_t num_mana;
int16_t num_heal;
int16_t num_heal_true;
int16_t num_ezheal;
int16_t num_ezheal_true;
int16_t num_life;
int16_t num_life_true;
int16_t num_pfe;
int16_t num_glyph;
int16_t num_mass_genocide;
int16_t num_speed;

int16_t num_enchant_to_a;
int16_t num_enchant_to_d;
int16_t num_enchant_to_h;
int16_t num_brand_weapon; /* brand bolts */
int16_t num_genocide;

int16_t num_artifact;
int16_t num_ego;

int16_t home_slot_free;
int16_t home_un_id;
int16_t home_damage;
int16_t num_duplicate_items;
int16_t num_slow_digest;
int16_t num_regenerate;
int16_t num_telepathy;
int16_t num_LIGHT;
int16_t num_see_inv;
int16_t num_invisible; /*  */

int16_t num_ffall;
int16_t num_free_act;
int16_t num_hold_life;
int16_t num_immune_acid;
int16_t num_immune_elec;
int16_t num_immune_fire;
int16_t num_immune_cold;
int16_t num_resist_acid;
int16_t num_resist_elec;
int16_t num_resist_fire;
int16_t num_resist_cold;
int16_t num_resist_pois;
int16_t num_resist_conf;
int16_t num_resist_sound;
int16_t num_resist_LIGHT;
int16_t num_resist_dark;
int16_t num_resist_chaos;
int16_t num_resist_disen;
int16_t num_resist_shard;
int16_t num_resist_nexus;
int16_t num_resist_blind;
int16_t num_resist_neth;
int16_t num_sustain_str;
int16_t num_sustain_int;
int16_t num_sustain_wis;
int16_t num_sustain_dex;
int16_t num_sustain_con;
int16_t num_sustain_all;

int16_t num_edged_weapon;
int16_t num_bad_gloves;
int16_t num_weapons;
int16_t num_bow;
int16_t num_rings;
int16_t num_neck;
int16_t num_armor;
int16_t num_cloaks;
int16_t num_shields;
int16_t num_hats;
int16_t num_gloves;
int16_t num_boots;

/*
 * Helper function -- clear counters for home equipment
 *
 * !FIX This needs to change into an array like borg.trait
 */
static void borg_notice_home_clear(borg_item *in_item, bool no_items)
{

    /*** Reset counters ***/

    /* Reset basic */
    num_food                = 0;
    num_fuel                = 0;
    num_mold                = 0;
    num_ident               = 0;
    num_recall              = 0;
    num_phase               = 0;
    num_escape              = 0;
    num_tele_staves         = 0;
    num_teleport            = 0;
    num_teleport_level      = 0;
    num_recharge            = 0;

    num_artifact            = 0;
    num_ego                 = 0;

    num_invisible           = 0;
    num_pfe                 = 0;
    num_glyph               = 0;
    num_genocide            = 0;
    num_mass_genocide       = 0;
    num_berserk             = 0;
    num_pot_rheat           = 0;
    num_pot_rcold           = 0;
    num_speed               = 0;

    num_slow_digest         = 0;
    num_regenerate          = 0;
    num_telepathy           = 0;
    num_see_inv             = 0;
    num_ffall               = 0;
    num_free_act            = 0;
    num_hold_life           = 0;
    num_immune_acid         = 0;
    num_immune_elec         = 0;
    num_immune_fire         = 0;
    num_immune_cold         = 0;
    num_resist_acid         = 0;
    num_resist_elec         = 0;
    num_resist_fire         = 0;
    num_resist_cold         = 0;
    num_resist_pois         = 0;
    num_resist_conf         = 0;
    num_resist_sound        = 0;
    num_resist_LIGHT        = 0;
    num_resist_dark         = 0;
    num_resist_chaos        = 0;
    num_resist_disen        = 0;
    num_resist_shard        = 0;
    num_resist_nexus        = 0;
    num_resist_blind        = 0;
    num_resist_neth         = 0;
    num_sustain_str         = 0;
    num_sustain_int         = 0;
    num_sustain_wis         = 0;
    num_sustain_dex         = 0;
    num_sustain_con         = 0;
    num_sustain_all         = 0;

    home_stat_add[STAT_STR] = 0;
    home_stat_add[STAT_INT] = 0;
    home_stat_add[STAT_WIS] = 0;
    home_stat_add[STAT_DEX] = 0;
    home_stat_add[STAT_CON] = 0;

    num_weapons             = 0;

    num_bow                 = 0;
    num_rings               = 0;
    num_neck                = 0;
    num_armor               = 0;
    num_cloaks              = 0;
    num_shields             = 0;
    num_hats                = 0;
    num_gloves              = 0;
    num_boots               = 0;
    num_LIGHT               = 0;
    num_speed               = 0;
    num_edged_weapon        = 0;
    num_bad_gloves          = 0;

    /* Reset healing */
    num_cure_critical = 0;
    num_cure_serious  = 0;
    num_fix_exp       = 0;
    num_mana          = 0;
    num_heal          = 0;
    num_ezheal        = 0;
    num_life          = 0;
    if (!in_item && !no_items)
        num_ezheal_true = 0;
    if (!in_item && !no_items)
        num_heal_true = 0;
    if (!in_item && !no_items)
        num_life_true = 0;

    /* Reset missiles */
    num_missile = 0;

    /* Reset books */
    num_book[0] = 0;
    num_book[1] = 0;
    num_book[2] = 0;
    num_book[3] = 0;
    num_book[4] = 0;
    num_book[5] = 0;
    num_book[6] = 0;
    num_book[7] = 0;
    num_book[8] = 0;

    /* Reset various */
    num_fix_stat[STAT_STR] = 0;
    num_fix_stat[STAT_INT] = 0;
    num_fix_stat[STAT_WIS] = 0;
    num_fix_stat[STAT_DEX] = 0;
    num_fix_stat[STAT_CON] = 0;

    /* Reset enchantment */
    num_enchant_to_a    = 0;
    num_enchant_to_d    = 0;
    num_enchant_to_h    = 0;

    home_slot_free      = 0;
    home_damage         = 0;
    home_un_id          = 0;

    num_duplicate_items = 0;
}

/*
 * This checks for duplicate items in the home
 */
static void borg_notice_home_dupe(borg_item *item, bool check_sval, int i)
{
    /* eventually check for power overlap... armor of resistence is same as weak
     * elvenkind.*/
    /*  two armors of elvenkind that resist poison is a dupe.  AJG*/

    int              dupe_count, x;
    borg_item       *item2;
    struct ego_item *e_ptr = &e_info[item->ego_idx];

    /* check for a duplicate.  */
    /* be carefull about extra powers (elvenkind/magi) */
    if (borg_ego_has_random_power(e_ptr))
        return;

    /* if it isn't identified, it isn't duplicate */
    if (item->needs_ident)
        return;

    /* if this is a stack of items then all after the first are a */
    /* duplicate */
    dupe_count = item->iqty - 1;

    /* Look for other items before this one that are the same */
    for (x = 0; x < i; x++) {
        if (x < z_info->store_inven_max)
            item2 = &borg_shops[7].ware[x];
        else
            /* Check what the borg has on as well.*/
            item2 = &borg_items[((x - z_info->store_inven_max) + INVEN_WIELD)];

        /* if everything matches it is a duplicate item */
        /* Note that we only check sval on certain items.  This */
        /* is because, for example, two pairs of dragon armor */
        /* are not the same unless their subtype (color) matches */
        /* but a defender is a defender even if one is a dagger and */
        /* one is a mace */
        if ((item->tval == item2->tval)
            && (check_sval ? (item->sval == item2->sval) : true)
            && (item->art_idx == item2->art_idx)
            && (item->ego_idx == item2->ego_idx)) {
            dupe_count++;
        }
    }

    /* there can be one dupe of rings because there are two ring slots. */
    if (item->tval == TV_RING && dupe_count)
        dupe_count--;

    /* Add this items count to the total duplicate count */
    num_duplicate_items += dupe_count;
}

/*
 * Helper function -- notice the home inventory
 */
static void borg_notice_home_aux(borg_item *in_item, bool no_items)
{
    int i;

    borg_item *item = NULL;

    borg_shop *shop = &borg_shops[7];
    bitflag    f[OF_SIZE];

    /*** Process the inventory ***/

    /* Scan the home */
    for (i = 0; i < (z_info->store_inven_max + (INVEN_TOTAL - INVEN_WIELD));
         i++) {
        if (no_items)
            break;

        if (!in_item)
            if (i < z_info->store_inven_max)
                item = &shop->ware[i];
            else
                item = &borg_items[(
                    (i - z_info->store_inven_max) + INVEN_WIELD)];
        else
            item = in_item;

        /* Skip empty items */
        if (!item->iqty && (i < z_info->store_inven_max)) {
            home_slot_free++;
            continue;
        }

        /* Hack -- skip un-aware items */
        if (!item->kind && (i < z_info->store_inven_max)) {
            home_slot_free++;
            continue;
        }

        if (of_has(item->flags, OF_SLOW_DIGEST))
            num_slow_digest += item->iqty;
        if (of_has(item->flags, OF_REGEN))
            num_regenerate += item->iqty;
        if (of_has(item->flags, OF_TELEPATHY))
            num_telepathy += item->iqty;
        if (of_has(item->flags, OF_SEE_INVIS))
            num_see_inv += item->iqty;
        if (of_has(item->flags, OF_FEATHER))
            num_ffall += item->iqty;
        if (of_has(item->flags, OF_FREE_ACT))
            num_free_act += item->iqty;
        if (of_has(item->flags, OF_HOLD_LIFE))
            num_hold_life += item->iqty;
        if (of_has(item->flags, OF_PROT_CONF))
            num_resist_conf += item->iqty;
        if (of_has(item->flags, OF_PROT_BLIND))
            num_resist_blind += item->iqty;
        if (item->el_info[ELEM_FIRE].res_level == 3) {
            num_immune_fire += item->iqty;
            num_resist_fire += item->iqty;
        }
        if (item->el_info[ELEM_ACID].res_level == 3) {
            num_immune_acid += item->iqty;
            num_resist_acid += item->iqty;
        }
        if (item->el_info[ELEM_COLD].res_level == 3) {
            num_immune_cold += item->iqty;
            num_resist_cold += item->iqty;
        }
        if (item->el_info[ELEM_ELEC].res_level == 3) {
            num_immune_elec += item->iqty;
            num_resist_elec += item->iqty;
        }
        if (item->el_info[ELEM_ACID].res_level == 1)
            num_resist_acid += item->iqty;
        if (item->el_info[ELEM_ELEC].res_level == 1)
            num_resist_elec += item->iqty;
        if (item->el_info[ELEM_FIRE].res_level == 1)
            num_resist_fire += item->iqty;
        if (item->el_info[ELEM_COLD].res_level == 1)
            num_resist_cold += item->iqty;
        if (item->el_info[ELEM_POIS].res_level == 1)
            num_resist_pois += item->iqty;
        if (item->el_info[ELEM_SOUND].res_level == 1)
            num_resist_sound += item->iqty;
        if (item->el_info[ELEM_LIGHT].res_level == 1)
            num_resist_LIGHT += item->iqty;
        if (item->el_info[ELEM_DARK].res_level == 1)
            num_resist_dark += item->iqty;
        if (item->el_info[ELEM_CHAOS].res_level == 1)
            num_resist_chaos += item->iqty;
        if (item->el_info[ELEM_DISEN].res_level == 1)
            num_resist_disen += item->iqty;
        if (item->el_info[ELEM_SHARD].res_level == 1)
            num_resist_shard += item->iqty;
        if (item->el_info[ELEM_NEXUS].res_level == 1)
            num_resist_nexus += item->iqty;
        if (item->el_info[ELEM_NETHER].res_level == 1)
            num_resist_neth += item->iqty;

        /* Count Sustains */
        if (of_has(item->flags, OF_SUST_STR))
            num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_INT))
            num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_WIS))
            num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_DEX))
            num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_CON))
            num_sustain_str += item->iqty;
        if (of_has(item->flags, OF_SUST_STR) && of_has(item->flags, OF_SUST_INT)
            && of_has(item->flags, OF_SUST_WIS)
            && of_has(item->flags, OF_SUST_DEX)
            && of_has(item->flags, OF_SUST_CON))
            num_sustain_all += item->iqty;

        /* count up bonus to stats */
        /* HACK only collect stat rings above +3 */

        if (item->modifiers[OBJ_MOD_STR]) {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_STR] > 3)
                home_stat_add[STAT_STR]
                    += item->modifiers[OBJ_MOD_STR] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_INT]) {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_INT] > 3)
                home_stat_add[STAT_INT]
                    += item->modifiers[OBJ_MOD_INT] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_WIS]) {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_WIS] > 3)
                home_stat_add[STAT_WIS]
                    += item->modifiers[OBJ_MOD_WIS] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_DEX]) {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_DEX] > 3)
                home_stat_add[STAT_DEX]
                    += item->modifiers[OBJ_MOD_DEX] * item->iqty;
        }
        if (item->modifiers[OBJ_MOD_CON]) {
            if (item->tval != TV_RING || item->modifiers[OBJ_MOD_CON] > 3)
                home_stat_add[STAT_CON]
                    += item->modifiers[OBJ_MOD_CON] * item->iqty;
        }

        /* count up bonus to speed */
        num_speed += item->modifiers[OBJ_MOD_SPEED] * item->iqty;

        /* count artifacts */
        if (item->art_idx) {
            num_artifact += item->iqty;
        }
        /* count egos that need *ID* */
        if (borg_ego_has_random_power(&e_info[item->ego_idx])
            && item->needs_ident) {
            num_ego += item->iqty;
        }

        /* count up unidentified stuff */
        if (item->needs_ident && (i < z_info->store_inven_max)) {
            home_un_id++;
        }

        /* Analyze the item */
        switch (item->tval) {
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
            num_armor += item->iqty;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, false, i);
            break;

        case TV_DRAG_ARMOR:
            num_armor += item->iqty;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, true, i);
            break;

        case TV_CLOAK:
            num_cloaks += item->iqty;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, false, i);

            break;

        case TV_SHIELD:
            num_shields += item->iqty;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, false, i);
            break;

        case TV_HELM:
        case TV_CROWN:
            num_hats += item->iqty;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, false, i);

            break;

        case TV_GLOVES:
            num_gloves += item->iqty;

            /* gloves of slaying give a damage bonus */
            home_damage += item->to_d * 3;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, false, i);

            break;

        case TV_FLASK:
            /* Use as fuel if we equip a lantern */
            if (borg_items[INVEN_LIGHT].sval == sv_light_lantern) {
                num_fuel += item->iqty;
                /* borg_note(format("1.num_fuel=%d",num_fuel)); */
            }
            break;

        case TV_LIGHT:
            /* Fuel */
            if (borg_items[INVEN_LIGHT].sval == sv_light_torch) {
                num_fuel += item->iqty;
            }

            /* Artifacts */
            if (item->art_idx) {
                num_LIGHT += item->iqty;
            }
            break;

        case TV_BOOTS:
            num_boots += item->iqty;

            /* see if this item is duplicated */
            borg_notice_home_dupe(item, false, i);
            break;

        case TV_SWORD:
        case TV_POLEARM:
        case TV_HAFTED:
            /* case TV_DIGGING: */
            {
                int16_t num_blow;

                num_weapons += item->iqty;
                /*  most edged weapons hurt magic for priests */
                if (player_has(player, PF_BLESS_WEAPON)) {
                    /* Penalize non-blessed edged weapons */
                    if ((item->tval == TV_SWORD || item->tval == TV_POLEARM)
                        && !of_has(item->flags, OF_BLESSED)) {
                        num_edged_weapon += item->iqty;
                    }
                }

                num_blow = borg_calc_blows(item);
                if (item->to_d > 8 || borg.trait[BI_CLEVEL] < 15) {
                    home_damage += num_blow
                                   * (item->dd * (item->ds)
                                       + (borg.trait[BI_TODAM] + item->to_d));
                } else {
                    home_damage += num_blow
                                   * (item->dd * (item->ds)
                                       + (borg.trait[BI_TODAM] + 8));
                }

                /* see if this item is a duplicate */
                borg_notice_home_dupe(item, false, i);
                break;
            }

        case TV_BOW:
            num_bow += item->iqty;

            /* see if this item is a duplicate */
            borg_notice_home_dupe(item, false, i);
            break;

        case TV_RING:
            num_rings += item->iqty;

            /* see if this item is a duplicate */
            borg_notice_home_dupe(item, true, i);

            break;

        case TV_AMULET:
            num_neck += item->iqty;

            /* see if this item is a duplicate */
            borg_notice_home_dupe(item, true, i);
            break;

        /* Books */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        case TV_NATURE_BOOK:
        case TV_SHADOW_BOOK:
        case TV_OTHER_BOOK:

            /* Skip incorrect books (if we can browse this book, it is good) */
            if (!obj_kind_can_browse(&k_info[item->kind]))
                break;

            /* only ever store non-dungeon books */
            if (kf_has(k_info[item->kind].kind_flags, KF_GOOD))
                break;

            /* Count the books */
            num_book[item->sval] += item->iqty;

            break;

        /* Food */
        case TV_FOOD:

            if (item->sval == sv_food_ration)
                num_food += item->iqty;
            else if (item->sval == sv_food_slime_mold)
                num_mold += item->iqty;
            else if (item->sval == sv_mush_purging) {
                num_fix_stat[STAT_CON] += item->iqty;
                num_fix_stat[STAT_STR] += item->iqty;
            } else if (item->sval == sv_mush_restoring) {
                num_fix_stat[STAT_STR] += item->iqty;
                num_fix_stat[STAT_INT] += item->iqty;
                num_fix_stat[STAT_WIS] += item->iqty;
                num_fix_stat[STAT_DEX] += item->iqty;
                num_fix_stat[STAT_CON] += item->iqty;
            }
            break;

        /* Potions */
        case TV_POTION:

            /* Analyze */
            if (item->sval == sv_potion_cure_critical)
                num_cure_critical += item->iqty;
            else if (item->sval == sv_potion_cure_serious)
                num_cure_serious += item->iqty;
            else if (item->sval == sv_potion_resist_heat)
                num_pot_rheat += item->iqty;
            else if (item->sval == sv_potion_resist_cold)
                num_pot_rcold += item->iqty;
            else if (item->sval == sv_potion_restore_life)
                num_fix_exp += item->iqty;
            else if (item->sval == sv_potion_restore_mana)
                num_mana += item->iqty;
            else if (item->sval == sv_potion_healing) {
                num_heal += item->iqty;
                if (!in_item && !no_items)
                    num_heal_true += item->iqty;
            } else if (item->sval == sv_potion_star_healing) {
                num_ezheal += item->iqty;
                if (!in_item && !no_items)
                    num_ezheal_true += item->iqty;
            } else if (item->sval == sv_potion_life) {
                num_life += item->iqty;
                if (!in_item && !no_items)
                    num_life_true += item->iqty;
            } else if (item->sval == sv_potion_berserk)
                num_berserk += item->iqty;
            else if (item->sval == sv_potion_speed)
                num_speed += item->iqty;

            break;

        /* Scrolls */
        case TV_SCROLL:

            /* Analyze the scroll */
            if (item->sval == sv_scroll_identify)
                num_ident += item->iqty;
            else if (item->sval == sv_scroll_phase_door)
                num_phase += item->iqty;
            else if (item->sval == sv_scroll_teleport)
                num_teleport += item->iqty;
            else if (item->sval == sv_scroll_word_of_recall)
                num_recall += item->iqty;
            else if (item->sval == sv_scroll_enchant_armor)
                num_enchant_to_a += item->iqty;
            else if (item->sval == sv_scroll_enchant_weapon_to_hit)
                num_enchant_to_h += item->iqty;
            else if (item->sval == sv_scroll_enchant_weapon_to_dam)
                num_enchant_to_d += item->iqty;
            else if (item->sval == sv_scroll_protection_from_evil)
                num_pfe += item->iqty;
            else if (item->sval == sv_scroll_rune_of_protection)
                num_glyph += item->iqty;
            else if (item->sval == sv_scroll_teleport_level)
                num_teleport_level += item->iqty;
            else if (item->sval == sv_scroll_recharging)
                num_recharge += item->iqty;
            else if (item->sval == sv_scroll_mass_banishment)
                num_mass_genocide += item->iqty;

            break;

        /* Rods */
        case TV_ROD:

            /* Analyze */
            if (item->sval == sv_rod_recall)
                num_recall += item->iqty * 100;

            break;

        /* Staffs */
        case TV_STAFF:

            /* only collect staves with more than 3 charges at high level */
            if (item->pval <= 3 && borg.trait[BI_CLEVEL] > 30)
                break;

            /* Analyze */
            if (item->sval == sv_staff_teleportation) {
                num_escape += item->pval * item->iqty;
                num_tele_staves++;
            }

            break;

        /* Missiles */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:

            /* Hack -- ignore invalid missiles */
            if (item->tval != borg.trait[BI_AMMO_TVAL])
                break;

            /* Hack -- ignore worthless missiles */
            if (item->value <= 0)
                break;

            /* Count them */
            num_missile += item->iqty;

            break;
        }

        /* if only doing one item, break. */
        if (in_item)
            break;
    }

    /*** Process the Spells and Prayers ***/

    /* Handle "satisfy hunger" -> infinite food */
    if (borg_spell_legal(REMOVE_HUNGER) || borg_spell_legal(HERBAL_CURING)) {
        num_food += 1000;
    }

    /* Handle "identify" -> infinite identifies */
    if (borg_spell_legal(IDENTIFY_RUNE)) {
        num_ident += 1000;
    }

    /* Handle ENCHANT_WEAPON */
    if (borg_spell_legal_fail(ENCHANT_WEAPON, 65)) {
        num_enchant_to_h += 1000;
        num_enchant_to_d += 1000;
    }

    /*  Handle PROTECTION_FROM_EVIL */
    if (borg_spell_legal(PROTECTION_FROM_EVIL)) {
        num_pfe += 1000;
    }

    /*  Handle "rune of protection" glyph */
    if (borg_spell_legal(GLYPH_OF_WARDING)
        || borg_equips_item(act_glyph, false)) {
        num_glyph += 1000;
    }

    /* handle restore */

    /* Handle recall */
    if (borg_spell_legal(WORD_OF_RECALL)) {
        num_recall += 1000;
    }

    /* Handle teleport_level */
    if (borg_spell_legal(TELEPORT_LEVEL)) {
        num_teleport_level += 1000;
    }

    /* Handle recharge */
    if (borg_spell_legal(RECHARGING)) {
        num_recharge += 1000;
    }

    /*** Process the Needs ***/

    /* Hack -- No need for stat repair */
    if (borg.trait[BI_SSTR])
        num_fix_stat[STAT_STR] += 1000;
    if (borg.trait[BI_SINT])
        num_fix_stat[STAT_INT] += 1000;
    if (borg.trait[BI_SWIS])
        num_fix_stat[STAT_WIS] += 1000;
    if (borg.trait[BI_SDEX])
        num_fix_stat[STAT_DEX] += 1000;
    if (borg.trait[BI_SCON])
        num_fix_stat[STAT_CON] += 1000;

    /* Extract the player flags */
    player_flags(player, f);

    /* Good flags */
    if (of_has(f, OF_SLOW_DIGEST))
        num_slow_digest++;
    if (of_has(f, OF_FEATHER))
        num_ffall++;
    if (of_has(f, OF_LIGHT_2) || rf_has(f, OF_LIGHT_3))
        num_LIGHT++;
    if (of_has(f, OF_REGEN))
        num_regenerate++;
    if (of_has(f, OF_TELEPATHY))
        num_telepathy++;
    if (of_has(f, OF_SEE_INVIS))
        num_see_inv++;
    if (of_has(f, OF_FREE_ACT))
        num_free_act++;
    if (of_has(f, OF_HOLD_LIFE))
        num_hold_life++;
    if (of_has(f, OF_PROT_CONF))
        num_resist_conf++;
    if (of_has(f, OF_PROT_BLIND))
        num_resist_blind++;

    /* Weird flags */

    /* Bad flags */

    /* Immunity flags */
    if (player->race->el_info[ELEM_FIRE].res_level == 3)
        num_immune_fire++;
    if (player->race->el_info[ELEM_ACID].res_level == 3)
        num_immune_acid++;
    if (player->race->el_info[ELEM_COLD].res_level == 3)
        num_immune_cold++;
    if (player->race->el_info[ELEM_ELEC].res_level == 3)
        num_immune_elec++;

    /* Resistance flags */
    if (player->race->el_info[ELEM_ACID].res_level > 0)
        num_resist_acid++;
    if (player->race->el_info[ELEM_ELEC].res_level > 0)
        num_resist_elec++;
    if (player->race->el_info[ELEM_FIRE].res_level > 0)
        num_resist_fire++;
    if (player->race->el_info[ELEM_COLD].res_level > 0)
        num_resist_cold++;
    if (player->race->el_info[ELEM_POIS].res_level > 0)
        num_resist_pois++;
    if (player->race->el_info[ELEM_LIGHT].res_level > 0)
        num_resist_LIGHT++;
    if (player->race->el_info[ELEM_DARK].res_level > 0)
        num_resist_dark++;
    if (player->race->el_info[ELEM_SOUND].res_level > 0)
        num_resist_sound++;
    if (player->race->el_info[ELEM_SHARD].res_level > 0)
        num_resist_shard++;
    if (player->race->el_info[ELEM_NEXUS].res_level > 0)
        num_resist_nexus++;
    if (player->race->el_info[ELEM_NETHER].res_level > 0)
        num_resist_neth++;
    if (player->race->el_info[ELEM_CHAOS].res_level > 0)
        num_resist_chaos++;
    if (player->race->el_info[ELEM_DISEN].res_level > 0)
        num_resist_disen++;

    /* Sustain flags */
    if (rf_has(f, OF_SUST_STR))
        num_sustain_str++;
    if (rf_has(f, OF_SUST_INT))
        num_sustain_int++;
    if (rf_has(f, OF_SUST_WIS))
        num_sustain_wis++;
    if (rf_has(f, OF_SUST_DEX))
        num_sustain_dex++;
    if (rf_has(f, OF_SUST_CON))
        num_sustain_con++;
}

/*
 * Extract the bonuses for items in the home.
 *
 * in_item is passed in if you want to pretend that in_item is
 *          the only item in the home.
 * no_items is passed in as true if you want to pretend that the
 *          home is empty.
 */
void borg_notice_home(borg_item *in_item, bool no_items)
{
    /* Notice the home equipment */
    borg_notice_home_clear(in_item, no_items);

    /* Notice the home inventory */
    borg_notice_home_aux(in_item, no_items);
}

#endif
