/**
 * \file borg-junk.c
 * \brief Handle junk items or clearing inventory to make space
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

#include "borg-junk.h"

#ifdef ALLOW_BORG

#include "../obj-util.h"
#include "../ui-menu.h"

#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-flow-take.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-analyze.h"
#include "borg-item-id.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-power.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Check if there is an object that needs to be trashed under the borg
 * and destroy it
 */
bool borg_destroy_floor(void)
{
    if (!borg_grids[borg.c.y][borg.c.x].take)
        return false;

    /* double check that there is an object here.  This is simulating doing */
    /* "look" so it is okay */
    if (!borg_get_top_object(cave, borg.c))
        return false;

    borg_take *take = &borg_takes[borg_grids[borg.c.y][borg.c.x].take];
    if (take->value != -10)
        return false;

    if (take->kind && take->kind->base)
        borg_note(format("# Destroying floor item %s%s", take->kind->base->name, take->kind->name));
    else
        borg_note("# Destroying floor item.");

    /* ignore it now */
    borg_keypress('k');
    borg_keypress('-');
    borg_keypress('a');
    borg_keypress('a');
    return true;
}

/*
 * Attempt to consume an item as a method of destroying it.
 */
static bool borg_consume(int i)
{
    borg_item *item = &borg_items[i];

    /* Special destruction */
    switch (item->tval) {
    case TV_POTION:

        /* Check the potion */
        if (item->sval == sv_potion_slime_mold
            || item->sval == sv_potion_cure_light
            || item->sval == sv_potion_cure_serious
            || item->sval == sv_potion_cure_critical
            || item->sval == sv_potion_healing
            || item->sval == sv_potion_star_healing
            || item->sval == sv_potion_life
            || item->sval == sv_potion_restore_life
            || item->sval == sv_potion_restore_mana
            || item->sval == sv_potion_heroism
            || item->sval == sv_potion_berserk
            || item->sval == sv_potion_resist_heat
            || item->sval == sv_potion_resist_cold
            || item->sval == sv_potion_infravision
            || item->sval == sv_potion_detect_invis
            || item->sval == sv_potion_cure_poison
            || item->sval == sv_potion_speed
            || item->sval == sv_potion_inc_exp) {
            /* Try quaffing the potion */
            if (borg_quaff_potion(item->sval))
                return true;
            break;
        }

        /* Gain one/lose one potions */
        if (item->sval == sv_potion_inc_str2) {
            /* Maxed out no need. Don't lose another stat */
            if (borg.trait[BI_CSTR] >= 118)
                return false;

            /* This class does not want to risk losing a different stat */
            if (borg.trait[BI_CLASS] == CLASS_MAGE
                || borg.trait[BI_CLASS] == CLASS_DRUID
                || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
                return false;

            /* Otherwise, it should be ok */
            if (borg_quaff_potion(item->sval))
                return true;
        }

        if (item->sval == sv_potion_inc_int2) {
            /* Maxed out no need. Don't lose another stat */
            if (borg.trait[BI_CINT] >= 118)
                return false;

            /* This class does not want to risk losing a different stat */
            if (borg.trait[BI_CLASS] != CLASS_MAGE
                && borg.trait[BI_CLASS] != CLASS_NECROMANCER)
                return false;

            /* Otherwise, it should be ok */
            if (borg_quaff_potion(item->sval))
                return true;
            break;
        }

        if (item->sval == sv_potion_inc_wis2) {
            /* Maxed out no need. Don't lose another stat */
            if (borg.trait[BI_CWIS] >= 118)
                return false;

            /* This class does not want to risk losing a different stat */
            if (borg.trait[BI_CLASS] != CLASS_PRIEST
                && borg.trait[BI_CLASS] != CLASS_DRUID)
                return false;

            /* Otherwise, it should be ok */
            if (borg_quaff_potion(item->sval))
                return true;
            break;
        }

        if (item->sval == sv_potion_inc_dex2) {
            /* Maxed out no need. Don't lose another stat */
            if (borg.trait[BI_CDEX] >= 118)
                return false;

            /* This class does not want to risk losing a different stat */
            if (borg.trait[BI_CLASS] == CLASS_MAGE
                || borg.trait[BI_CLASS] == CLASS_PRIEST
                || borg.trait[BI_CLASS] == CLASS_DRUID
                || borg.trait[BI_CLASS] == CLASS_NECROMANCER)
                return false;

            /* Otherwise, it should be ok */
            if (borg_quaff_potion(item->sval))
                return true;
            break;
        }

        if (item->sval == sv_potion_inc_con2) {
            /* Maxed out no need. Don't lose another stat */
            if (borg.trait[BI_CCON] >= 118)
                return false;

            /* Otherwise, it should be ok */
            if (borg_quaff_potion(item->sval))
                return true;
            break;
        }

        break;

    case TV_SCROLL:

        /* Check the scroll */
        if (item->sval == sv_scroll_light
            || item->sval == sv_scroll_monster_confusion
            || item->sval == sv_scroll_trap_door_destruction
            || item->sval == sv_scroll_satisfy_hunger
            || item->sval == sv_scroll_dispel_undead
            || item->sval == sv_scroll_blessing
            || item->sval == sv_scroll_holy_chant
            || item->sval == sv_scroll_holy_prayer) {
            /* Try reading the scroll */
            if (borg_read_scroll(item->sval))
                return true;
        }

        break;

    case TV_FOOD:
        /* Check the grub */
        if (item->sval == sv_food_ration 
            || item->sval == sv_food_slime_mold
            || item->sval == sv_food_waybread)

            /* Try eating the food (unless Bloated) */
            if (!borg.trait[BI_ISFULL] && borg_eat(item->tval, item->sval))
                return true;

        break;

    case TV_MUSHROOM:

        /* Check the grub */
        if (item->sval == sv_mush_second_sight
            || item->sval == sv_mush_fast_recovery
            || item->sval == sv_mush_cure_mind
            || item->sval == sv_mush_restoring
            || item->sval == sv_mush_emergency 
            || item->sval == sv_mush_terror
            || item->sval == sv_mush_stoneskin 
            || item->sval == sv_mush_debility
            || item->sval == sv_mush_sprinting 
            || item->sval == sv_mush_purging)

            /* Try eating the food (unless Bloated) */
            if (!borg.trait[BI_ISFULL] && borg_eat(item->tval, item->sval))
                return true;

        break;
    }

    /* Nope */
    return false;
}

/*
 * Drop "junk" items, marking them for destruction
 */
bool borg_drop_junk(void)
{
    int     i;
    bool    fix = false;
    int32_t p;
    int32_t value;

    /* Hack -- no need */
    if (!borg_do_crush_junk)
        return false;

    /* not while recalling since this is a two step process */
    /* and if the recall kicks in in the middle it confuses things */
    if (borg.goal.recalling)
        return false;

    /* No crush if even slightly dangerous */
    if (borg_danger(borg.c.y, borg.c.x, 1, true, false)
        > borg.trait[BI_CURHP] / 10)
        return false;

    /* Destroy actual "junk" items */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* dont crush the swap */
        if (weapon_swap && i == weapon_swap - 1)
            continue;
        if (armour_swap && i == armour_swap - 1)
            continue;

            /* Dont crush weapons if we are weilding a digger */
#if 0
        if (item->tval >= TV_DIGGING && item->tval <= TV_SWORD &&
            borg_items[INVEN_WIELD].tval == TV_DIGGING) continue;
#endif

        /* dont crush our spell books */
        if (obj_kind_can_browse(&k_info[item->kind]))
            continue;

        /* Do not crush unID'd Scrolls, sell them in town */
        if (item->tval == TV_SCROLL && (!item->ident && !item->kind))
            continue;

        /* Do not crush Boots, they could be SPEED */
        if (item->tval == TV_BOOTS && !item->ident)
            continue;

        /* save the items value */
        value = item->value;

        /* Crush Stacked Wands and Staves that are empty.
         * ie. 5 Staffs of Teleportation (2 charges).
         * Only 2 charges in 5 staves means top 3 are empty.
         */
        if ((item->tval == TV_STAFF || item->tval == TV_WAND)
            && (item->ident || (item->note && strstr(item->note, "empty")))) {
            if (item->iqty > item->pval)
                value = 0L;
        }

        /* Skip non "worthless" items */
        if (item->tval >= TV_CHEST) {
            /* unknown and not worthless */
            if (!item->ident && value > 0)
                continue;

            /* skip items that are 'valuable'.  This is level dependent */
            /* try to make the borg junk +1,+1 dagger at level 40 */

            /* if the item gives a bonus to a stat, boost its value */
            if ((item->modifiers[OBJ_MOD_STR] > 0
                    || item->modifiers[OBJ_MOD_INT] > 0
                    || item->modifiers[OBJ_MOD_WIS] > 0
                    || item->modifiers[OBJ_MOD_DEX] > 0
                    || item->modifiers[OBJ_MOD_CON] > 0)
                && value > 0) {
                value += 2000L;
            }

            /* Keep some stuff */
            if ((item->tval == borg.trait[BI_AMMO_TVAL] && value > 0)
                || ((item->tval == TV_POTION
                        && item->sval == sv_potion_restore_mana)
                    && (borg.trait[BI_MAXSP] >= 1))
                || (item->tval == TV_POTION && item->sval == sv_potion_healing)
                || (item->tval == TV_POTION
                    && item->sval == sv_potion_star_healing)
                || (item->tval == TV_POTION && item->sval == sv_potion_life)
                || (item->tval == TV_POTION && item->sval == sv_potion_speed)
                || (item->tval == TV_ROD && item->sval == sv_rod_drain_life)
                || (item->tval == TV_ROD && item->sval == sv_rod_healing)
                || (item->tval == TV_ROD && item->sval == sv_rod_mapping
                    && borg.trait[BI_CLASS] == CLASS_WARRIOR)
                || (item->tval == TV_STAFF
                    && item->sval == sv_staff_dispel_evil)
                || (item->tval == TV_STAFF && item->sval == sv_staff_power)
                || (item->tval == TV_STAFF && item->sval == sv_staff_holiness)
                || (item->tval == TV_WAND && item->sval == sv_wand_drain_life)
                || (item->tval == TV_WAND && item->sval == sv_wand_annihilation)
                || (item->tval == TV_WAND && item->sval == sv_wand_teleport_away
                    && borg.trait[BI_CLASS] == CLASS_WARRIOR)
                || (item->ego_idx
                    && borg_ego_has_random_power(&e_info[item->ego_idx]))
                || (item->tval == TV_SCROLL
                    && item->sval == sv_scroll_teleport_level
                    && borg.trait[BI_ATELEPORTLVL] < 1000)
                || (item->tval == TV_SCROLL
                    && item->sval == sv_scroll_protection_from_evil))

            {
                value += 5000L;
            }

            /* Go Ahead and crush diggers */
            if (item->tval == TV_DIGGING)
                value = 0L;

            /* Crush missiles that aren't mine */
            if (item->tval == TV_SHOT || item->tval == TV_ARROW
                || item->tval == TV_BOLT) {
                if (item->tval != borg.trait[BI_AMMO_TVAL])
                    value = 0L;
            }

            /* borg_worships_gold will sell all kinds of stuff,
             * except {cursed} is junk
             */
            if (item->value > 0
                && ((borg_cfg[BORG_WORSHIPS_GOLD]
                        || borg.trait[BI_MAXCLEVEL] < 10)
                    || ((borg_cfg[BORG_MONEY_SCUM_AMOUNT] < borg.trait[BI_GOLD])
                        && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0))
                && borg.trait[BI_MAXCLEVEL] <= 20 && !item->cursed)
                continue;

            /* up to level 5, keep anything of any value */
            if (borg.trait[BI_CDEPTH] < 5 && value > 0)
                continue;
            /* up to level 10, keep anything of any value */
            if (borg.trait[BI_CDEPTH] < 10 && value > 15)
                continue;
            /* up to level 15, keep anything of value 100 or better */
            if (borg.trait[BI_CDEPTH] < 15 && value > 100)
                continue;
            /* up to level 30, keep anything of value 500 or better */
            if (borg.trait[BI_CDEPTH] < 30 && value > 500)
                continue;
            /* up to level 40, keep anything of value 1000 or better */
            if (borg.trait[BI_CDEPTH] < 40 && value > 1000)
                continue;
            /* up to level 60, keep anything of value 1200 or better */
            if (borg.trait[BI_CDEPTH] < 60 && value > 1200)
                continue;
            /* up to level 80, keep anything of value 1400 or better */
            if (borg.trait[BI_CDEPTH] < 80 && value > 1400)
                continue;
            /* up to level 90, keep anything of value 1600 or better */
            if (borg.trait[BI_CDEPTH] < 90 && value > 1600)
                continue;
            /* up to level 95, keep anything of value 4800 or better */
            if (borg.trait[BI_CDEPTH] < 95 && value > 4800)
                continue;
            /* below level 127, keep anything of value 2000 or better */
            if (borg.trait[BI_CDEPTH] < 127 && value > 5600)
                continue;

            /* Save the item */
            memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

            /* Destroy the item */
            memset(&borg_items[i], 0, sizeof(borg_item));

            /* Fix later */
            fix = true;

            /* Examine the inventory */
            borg_notice(false);

            /* Evaluate the inventory */
            p = borg_power();

            /* Restore the item */
            memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

            /* skip things we are using */
            if (p < borg.power)
                continue;
        }

        /* re-examine the inventory */
        if (fix)
            borg_notice(true);

        /* Hack -- skip good un-id'd "artifacts" */
        if (borg_item_note_needs_id(item))
            continue;

        /* hack --  with random artifacts some are good and bad */
        /*         so check them all */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
            continue;

        /* Message */
        borg_note(format("# Dropping junk %s (valued at %d)", item->desc, value));

        /* inscribe "borg ignore". The borg crushes all items */
        /* on the floor that are inscribed this way */
        borg_keypress('{');
        borg_keypress(all_letters_nohjkl[i]);
        borg_keypresses("borg ignore");
        borg_keypress(KC_ENTER);

        /* drop it then ignore it */
        borg_keypress('d');
        borg_keypress(all_letters_nohjkl[i]);

        if (item->iqty > 1) {
            borg_keypress('1');
            borg_keypress(KC_ENTER);
        }

        /* Success */
        return true;
    }

    /* re-examine the inventory */
    if (fix)
        borg_notice(true);

    /* Hack -- no need */
    borg_do_crush_junk = false;

    /* Nothing to destroy */
    return false;
}

/*
 * Drop something to make a free inventory slot.
 *
 * This function evaluates the possible worlds that result from
 * the getting rid of each inventory slot, and attempts to drop from
 * that slot which causes the best possible resulting world.
 *
 * We attempt to avoid dropping unknown items by "rewarding" the
 * presence of unknown items by a massively heuristic value.
 *
 * The borg makes two attempts to find something to drop.  The first
 * time important things are never dropped.  The second time, things 
 * that are somewhat important, like things not identified yet, can be 
 * considered.
 * 
 * If the Borg cannot find something to drop, which should only
 * happen if he fills up with artifacts, then he will probably act
 * rather twitchy for a while.
 *
 * This function does not have to be very efficient.
 */
bool borg_drop_hole(bool desperate)
{
    int     i, b_i = -1;
    int32_t p, b_p = 0L;
    int32_t w, b_w = 0L;

    int32_t value;
    int32_t value_boost;

    bool fix = false;

    /* Do not destroy items unless we need the space */
    if (!borg_items[PACK_SLOTS - 1].iqty)
        return false;
    
    /* only on the floor */
    if (borg_grids[borg.c.y][borg.c.x].feat != FEAT_FLOOR)
        return false;

    /* No crush if even slightly dangerous */
    if (borg.trait[BI_CDEPTH]
        && (borg_danger(borg.c.y, borg.c.x, 1, true, false)
                > borg.trait[BI_CURHP] / 10
            && (borg.trait[BI_CURHP] != borg.trait[BI_MAXHP]
                || borg_danger(borg.c.y, borg.c.x, 1, true, false)
                       > (borg.trait[BI_CURHP] * 2) / 3)))
        return false;

    /* Scan the inventory */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        value_boost = 0L;

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Hack -- skip "artifacts" */
        if (item->art_idx)
            continue;

        /* skip food */
        if (item->tval == TV_FOOD && borg.trait[BI_FOOD] < 5)
            continue;

        /* don't crush the swap weapon */
        if (weapon_swap && i == weapon_swap - 1)
            continue;
        if (armour_swap && i == armour_swap - 1)
            continue;

        /* don't crush our spell books */
        if (obj_kind_can_browse(&k_info[item->kind]))
            continue;

        /* Do not crush Boots, they could be SPEED */
        if (item->tval == TV_BOOTS && !item->ident) {
            if (desperate)
                value_boost = 10000;
            else
                continue;
        }

        /* Don't crush weapons if we are wielding a digger */
        if (item->tval >= TV_DIGGING && item->tval <= TV_SWORD
            && borg_items[INVEN_WIELD].tval == TV_DIGGING) {
            if (desperate)
                value_boost = 10000L;
            else
                continue;
        }

        /* Hack -- skip "artifacts" */
        if (item->art_idx && !item->ident)
            continue;

        /* things with unknown runes */
        if (borg_item_note_needs_id(item)) {
            if (desperate)
                value_boost = 5000L;
            else
                continue;
        }

        /* never crush cool stuff that we might be needing later */
        if (!desperate) {
            if ((item->tval == TV_POTION && item->sval == sv_potion_restore_mana)
                && (borg.trait[BI_MAXSP] >= 1))
                continue;
            if (item->tval == TV_POTION && item->sval == sv_potion_healing)
                continue;
            if (item->tval == TV_POTION && item->sval == sv_potion_star_healing)
                continue;
            if (item->tval == TV_POTION && item->sval == sv_potion_life)
                continue;
            if (item->tval == TV_POTION && item->sval == sv_potion_speed)
                continue;
            if (item->tval == TV_SCROLL
                && item->sval == sv_scroll_protection_from_evil)
                continue;
            if (item->tval == TV_SCROLL
                && item->sval == sv_scroll_rune_of_protection)
                continue;
            if (item->tval == TV_SCROLL && item->sval == sv_scroll_teleport_level
                && borg.trait[BI_ATELEPORTLVL] < 1000)
                continue;
            if (item->tval == TV_ROD
                && (item->sval == sv_rod_healing
                    || (item->sval == sv_rod_mapping
                        && borg.trait[BI_CLASS] == CLASS_WARRIOR))
                && item->iqty <= 5)
                continue;
            if (item->tval == TV_WAND
                && item->sval == sv_wand_teleport_away
                && borg.trait[BI_CLASS] == CLASS_WARRIOR
                && borg.trait[BI_ATPORTOTHER] <= 8)
                continue;
            if (item->tval == TV_ROD
                && (item->sval == sv_rod_light && borg.trait[BI_CURLITE] <= 0))
                continue;
        } else
            value_boost = 7000;

        /* a boost for things with random powers */
        if (item->ego_idx && borg_ego_has_random_power(&e_info[item->ego_idx])
            && !item->ident) {
            if (desperate)
                value_boost = 10000L;
            else
                continue;
        }

        /* save the items value */
        value = item->value + value_boost;

        /* save the items weight */
        w = item->weight * item->iqty;

        /* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Destroy the item */
        memset(&borg_items[i], 0, sizeof(borg_item));

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Penalize loss of "gold" */

        /* if the item gives a bonus to a stat, boost its value */
        if (item->modifiers[OBJ_MOD_STR] > 0 
            || item->modifiers[OBJ_MOD_INT] > 0
            || item->modifiers[OBJ_MOD_WIS] > 0
            || item->modifiers[OBJ_MOD_DEX] > 0
            || item->modifiers[OBJ_MOD_CON] > 0) {
            value += 20000L;
        }

        /* Keep the correct types of missiles which have value
         * if we do have have tons already. unless in town, you can junk em in
         * town.
         */
        if ((item->tval == borg.trait[BI_AMMO_TVAL]) && (value > 0)
            && (borg.trait[BI_AMISSILES] <= 35) && borg.trait[BI_CDEPTH] >= 1) {
            value += 5000L;
        }

        /* Hack  show preference for destroying things we will not use */
        /* if we are high enough level not to worry about gold. */
        if (borg.trait[BI_CLEVEL] > 35) {
            switch (item->tval) {
                /* rings are under valued. */
            case TV_RING:
                value = (item->iqty * value * 10);
                break;

            case TV_AMULET:
            case TV_BOW:
            case TV_HAFTED:
            case TV_POLEARM:
            case TV_SWORD:
            case TV_BOOTS:
            case TV_GLOVES:
            case TV_HELM:
            case TV_CROWN:
            case TV_SHIELD:
            case TV_SOFT_ARMOR:
            case TV_HARD_ARMOR:
            case TV_DRAG_ARMOR:
                value = (item->iqty * value * 5);
                break;

            case TV_CLOAK:
                if (item->ego_idx
                    && borg_ego_has_random_power(&e_info[item->ego_idx]))
                    value = (item->iqty * (300000L));
                else
                    value = (item->iqty * value);
                break;

            case TV_ROD:
                /* BIG HACK! don't crush cool stuff. */
                if ((item->sval != sv_rod_drain_life)
                    || (item->sval != sv_rod_acid_ball)
                    || (item->sval != sv_rod_elec_ball)
                    || (item->sval != sv_rod_fire_ball)
                    || (item->sval != sv_rod_cold_ball))
                    value = (item->iqty * (300000L)); /* value at 30k */
                else
                    value = (item->iqty * value);
                break;

            case TV_STAFF:
                /* BIG HACK! don't crush cool stuff. */
                if (item->sval != sv_staff_dispel_evil
                    || ((item->sval != sv_staff_power
                            || item->sval != sv_staff_holiness)
                        && borg.trait[BI_GOOD_S_CHG] < 2)
                    || (item->sval != sv_staff_destruction
                        && borg.trait[BI_ASTFDEST] < 2))
                    value = (item->iqty * (300000L)); /* value at 30k */
                else
                    value = (item->iqty * (value / 2));
                break;

            case TV_WAND:
                /* BIG HACK! don't crush cool stuff. */
                if ((item->sval != sv_wand_drain_life)
                    || (item->sval != sv_wand_teleport_away)
                    || (item->sval != sv_wand_acid_ball)
                    || (item->sval != sv_wand_elec_ball)
                    || (item->sval != sv_wand_fire_ball)
                    || (item->sval != sv_wand_cold_ball)
                    || (item->sval != sv_wand_annihilation)
                    || (item->sval != sv_wand_dragon_fire)
                    || (item->sval != sv_wand_dragon_cold))
                    value = (item->iqty * (300000L)); /* value at 30k */
                else
                    value = (item->iqty * (value / 2));
                break;

            /* scrolls and potions crush easy */
            case TV_SCROLL:
                if ((item->sval != sv_scroll_protection_from_evil)
                    || (item->sval != sv_scroll_rune_of_protection))
                    value = (item->iqty * (30000L));
                else
                    value = (item->iqty * (value / 10));
                break;

            case TV_POTION:
                /* BIG HACK! don't crush heal/mana potions.  It could be */
                /* that we are in town and are collecting them. */
                if ((item->sval != sv_potion_healing)
                    || (item->sval != sv_potion_star_healing)
                    || (item->sval != sv_potion_life)
                    || (item->sval != sv_potion_restore_mana))
                    value = (item->iqty * (300000L)); /* value at 30k */
                else
                    value = (item->iqty * (value / 10));
                break;

            default:
                value = (item->iqty * (value / 3));
                break;
            }
        } else {
            value = (item->iqty * value);
        }

        /* Hack -- try not to destroy "unaware" items
         * unless deep
         */
        if (!item->kind && (value > 0)) {
            /* Hack -- Reward "unaware" items */
            switch (item->tval) {
            case TV_RING:
            case TV_AMULET:
                value = (borg.trait[BI_MAXDEPTH] * 5000L);
                break;

            case TV_ROD:
                value = (borg.trait[BI_MAXDEPTH] * 3000L);
                break;

            case TV_STAFF:
            case TV_WAND:
                value = (borg.trait[BI_MAXDEPTH] * 2000L);
                break;

            case TV_SCROLL:
            case TV_POTION:
                value = (borg.trait[BI_MAXDEPTH] * 500L);
                break;

            case TV_FOOD:
                value = (borg.trait[BI_MAXDEPTH] * 10L);
                break;
            }
        }

        /* Hack -- try not to destroy "unknown" items */
        if (!item->ident && (value > 0) && borg_item_worth_id(item)) {
            /* Reward "unknown" items */
            switch (item->tval) {
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
                value += 100L;
                break;

            case TV_BOW:
                value += 20000L;
                break;

            case TV_DIGGING:
                value += 10L;
                break;

            case TV_HAFTED:
            case TV_POLEARM:
            case TV_SWORD:
                value += 10000L;
                break;

            case TV_BOOTS:
            case TV_GLOVES:
            case TV_HELM:
            case TV_CROWN:
            case TV_SHIELD:
            case TV_CLOAK:
                value += 15000L;
                break;

            case TV_SOFT_ARMOR:
            case TV_HARD_ARMOR:
            case TV_DRAG_ARMOR:
                value += 15000L;
                break;

            case TV_AMULET:
            case TV_RING:
                value += 5000L;
                break;

            case TV_STAFF:
            case TV_WAND:
                value += 1000L;
                break;
            }
        }

        /* power is much more important than gold. */
        value = value / 100;

        /* If I have no food, and in town, I must have a free spot to buy food
         */
        if (borg.trait[BI_CDEPTH] == 0 && borg.trait[BI_FOOD] == 0) {
            /* Power is way more important than gold */
            value = value / 500;
        }

        p -= value;

        /* Ignore "bad" swaps */
        if ((b_i >= 0) && (p < b_p))
            continue;

        /* all things being equal, get rid of heavy stuff first */
        if (b_p == p && w < b_w)
            continue;

        /* Maintain the "best" */
        b_i = i;
        b_p = p;
        b_w = w;
    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Attempt to destroy it */
    if (b_i >= 0) {
        borg_item *item = &borg_items[b_i];

        /* Debug */
        borg_note(format(
            "# Junking %ld gold (full)", (long int)borg.power * 100 - b_p));

        /* Try to consume the junk */
        if (borg_consume(b_i))
            return true;

        /* Message */
        borg_note(format("# Dropping %s.", item->desc));

        /* Drop that item */
        /* inscribe "borg ignore". The borg crushes all items */
        /* on the floor that are inscribed this way */
        borg_keypress('{');
        borg_keypress(all_letters_nohjkl[b_i]);
        borg_keypresses("borg ignore");
        borg_keypress(KC_ENTER);

        /* drop it */
        borg_keypress('d');
        borg_keypress(all_letters_nohjkl[b_i]);
        if (item->iqty > 1) {
            borg_keypress('*');
            borg_keypress(KC_ENTER);
        }

        /* Success */
        return true;
    }

    /* if we got to here, we need to make room but have nothing we can crush */
    /* try again but allow more things to be crushed */
    if (!desperate)
        return borg_drop_hole(true);

    return false;
}

/*
 * Drop "junk" when slow (in the dungeon).
 *
 * We penalize the loss of both power and monetary value, and reward
 * the loss of weight that may be slowing us down.  The weight loss
 * is worth one gold per tenth of a pound.  This causes things like
 * lanterns and chests and spikes to be considered "annoying".
 */
bool borg_drop_slow(void)
{
    int     i, b_i = -1;
    int32_t p, b_p = 0L;

    int32_t temp;

    int32_t greed;

    bool fix = false;

    /* No crush if even slightly dangerous */
    if (borg_danger(borg.c.y, borg.c.x, 1, true, false)
        > borg.trait[BI_CURHP] / 20)
        return false;

    /* Hack -- never in town */
    if (borg.trait[BI_CDEPTH] == 0)
        return false;

    /* Do not crush items unless we are slow */
    if (borg.trait[BI_SPEED] >= 110)
        return false;

    /* Not if in munchkin mode */
    if (borg.munchkin_mode)
        return false;

    /* Calculate "greed" factor */
    greed = (borg.trait[BI_GOLD] / 100L) + 100L;

    /* Minimal and maximal greed */
    if (greed < 500L && borg.trait[BI_CLEVEL] > 35)
        greed = 500L;
    if (greed > 25000L)
        greed = 25000L;

    /* Decrease greed by our slowness */
    greed -= (110 - borg.trait[BI_SPEED]) * 500;
    if (greed <= 0)
        greed = 0L;

    /* Scan for junk */
    for (i = 0; i < QUIVER_END; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip our equipment, but do not skip the quiver */
        if (i >= z_info->pack_size && i <= INVEN_FEET)
            continue;

        /* don't crush the swap weapon */
        if (weapon_swap && i == weapon_swap - 1)
            continue;
        if (armour_swap && i == armour_swap - 1)
            continue;

        /* Skip "good" unknown items (unless "icky") */
        if (!item->ident && borg_item_worth_id(item))
            continue;

        /* Do not crush Boots, they could be SPEED */
        if (item->tval == TV_BOOTS && !item->ident)
            continue;

        /* Hack -- Skip artifacts */
        if (OPT(player, birth_randarts) && item->art_idx && !item->ident)
            continue;
        if (borg_item_note_needs_id(item))
            continue;

        /* Don't crush weapons if we are wielding a digger */
        if (item->tval >= TV_DIGGING && item->tval <= TV_SWORD
            && borg_items[INVEN_WIELD].tval == TV_DIGGING)
            continue;

        /* Don't crush it if it is our only source of light */
        if (item->tval == TV_ROD
            && (item->sval == sv_rod_light && borg.trait[BI_CURLITE] <= 0))
            continue;

        /* Rods of healing are too hard to come by */
        if (item->tval == TV_ROD && item->sval == sv_rod_healing)
            continue;

        /* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Destroy one of the items */
        borg_items[i].iqty--;

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Obtain the base price */
        temp = ((item->value < 30000L) ? item->value : 30000L);

        /* Hack -- ignore very cheap items */
        if (temp < greed)
            temp = 0L;

        /* Penalize */
        p -= temp;

        /* Obtain the base weight */
        temp = item->weight;

        /* Reward */
        p += (temp * 100);

        /* Ignore "bad" swaps */
        if (p < b_p)
            continue;

        /* Maintain the "best" */
        b_i = i;
        b_p = p;
    }

    /* Examine the inventory */
    if (fix)
        borg_notice(true);

    /* Destroy "useless" things */
    if ((b_i >= 0) && (b_p >= (borg.power))) {
        borg_item *item = &borg_items[b_i];

        /* Message */
        borg_note(format("# Junking %ld power (slow) value %d",
            (long int)b_p - borg.power, item->value));

        /* Attempt to consume it */
        if (borg_consume(b_i))
            return true;

        /* Message */
        borg_note(format("# Dropping %s.", item->desc));

        /* inscribe "borg ignore". The borg crushes all items */
        /* on the floor that are inscribed this way */
        borg_keypress('{');
        if (b_i < INVEN_WIELD) {
            borg_keypress(all_letters_nohjkl[b_i]);
        } else if (b_i < QUIVER_START) {
            borg_keypress('/');

            borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);
        } else {
            /* Quiver Slot */
            borg_keypress('|');
            borg_keypress('0' + (b_i - QUIVER_START));
        }
        borg_keypresses("borg ignore");
        borg_keypress(KC_ENTER);

        /* Drop one item */
        borg_keypress('d');
        if (b_i < INVEN_WIELD) {
            borg_keypress(all_letters_nohjkl[b_i]);
        } else if (b_i < QUIVER_START) {
            borg_keypress('/');

            borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);
        } else {
            /* Quiver Slot */
            borg_keypress('|');
            borg_keypress('0' + (b_i - QUIVER_START));
        }
        if (item->iqty > 1) {
            borg_keypress('1');
            borg_keypress(KC_ENTER);
        }

        return true;
    }
    /* Nothing to destroy */
    return false;
}

/*
 * Examine the quiver and dump any worthless items
 *
 * Borg will scan the quiver slots for items which are cursed or have
 * negative bonuses.  Then shoot those items to get rid of them.
 * He needs to do so when safe.
 */
bool borg_dump_quiver(void)
{
    int i, b_i = -1;
    int quiver_capacity;

    borg_item *item;

    /* hack to prevent the swap till you drop loop */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000)
        return false;
    if (borg.time_this_panel > 150)
        return false;

    /* don't crush stuff unless we are on a floor */
    if (borg_grids[borg.c.y][borg.c.x].feat != FEAT_FLOOR)
        return false;

    /* How many should I carry */
    if (borg.trait[BI_CLASS] == CLASS_RANGER
        || borg.trait[BI_CLASS] == CLASS_WARRIOR)
        quiver_capacity = (kb_info[TV_ARROW].max_stack - 1) * 2;
    else
        quiver_capacity = kb_info[TV_ARROW].max_stack - 1;

    quiver_capacity *= z_info->quiver_size;

    /* Scan equip */
    for (i = QUIVER_END - 1; i >= QUIVER_START; i--) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip if it is not cursed and matches my ammo.  If it is not cursed
         * but does not match my ammo, then it is dumped.
         */
        if (!item->cursed && item->tval == borg.trait[BI_AMMO_TVAL]) {
            /* It has some value */
            if (item->to_d > 0 && item->to_h > 0)
                continue;
            if (borg_item_note_needs_id(item))
                continue;

            /* Limit the amount of missiles carried */
            if (borg.trait[BI_AMISSILES] <= quiver_capacity && item->to_d >= 0
                && item->to_h >= 0)
                continue;
        }

        /* Track a crappy one */
        b_i = i;
    }

    /* No item */
    if (b_i >= 0) {
        /* Get the item */
        item = &borg_items[b_i];

        /* Log */
        borg_note(format("# Dumping %s.  Bad ammo in quiver.", item->desc));

        /* inscribe it */
        borg_keypress('{');
        borg_keypress('|');
        borg_keypress(b_i - QUIVER_START + '0');
        borg_keypresses("borg ignore");
        borg_keypress(KC_ENTER);

        /* Drop it */
        borg_keypress('d');
        borg_keypress('|');
        borg_keypress(b_i - QUIVER_START + '0');
        if (item->iqty > 1) {
            borg_keypress('*');
            borg_keypress(KC_ENTER);
        }

        /* Did something */
        borg.time_this_panel++;
        return true;
    }

    /* Nope */
    return false;
}

/*
 * Remove useless equipment.
 *
 * Look through the inventory for equipment that is reducing power.
 *
 * Basically, we evaluate the world both with the current set of
 * equipment, and in the alternate world in which various items
 * are removed, and we take one step towards the world in which 
 * we have the most "power".
 */
bool borg_remove_stuff(void)
{
    int hole = borg_first_empty_inventory_slot();

    int32_t p, b_p = 0L, w_p = 0L;

    int i, b_i = -1;

    borg_item *item;

    bool fix = false;

    /* if there was no hole, done */
    if (hole == -1)
        return false;

    /*  hack to prevent the swap till you drop loop */
    if (borg.trait[BI_ISHUNGRY] || borg.trait[BI_ISWEAK])
        return false;

    /* Forbid if been sitting on level forever */
    /*    Just come back and work through the loop later */
    if (borg_t - borg_began > 2000)
        return false;
    if (borg.time_this_panel > 150)
        return false;

    /* Start with good power */
    b_p = borg.power;

    /* Scan equip */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
        item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Require "aware" */
        if (!item->kind)
            continue;

        /* Require "known" (or needs id) unless we know it is cursed */
        if (borg_item_note_needs_id(item) && !item->cursed)
            continue;

        /* skip it if it has not been decursed */
        if (item->one_ring)
            continue;

        /* Save the hole */
        memcpy(&safe_items[hole], &borg_items[hole], sizeof(borg_item));

        /* Save the item */
        memcpy(&safe_items[i], &borg_items[i], sizeof(borg_item));

        /* Take off the item */
        memcpy(&borg_items[hole], &safe_items[i], sizeof(borg_item));

        /* Erase the item from equip */
        memset(&borg_items[i], 0, sizeof(borg_item));

        /* Fix later */
        fix = true;

        /* Examine the inventory */
        borg_notice(false);

        /* Evaluate the inventory */
        p = borg_power();

        /* Restore the item */
        memcpy(&borg_items[i], &safe_items[i], sizeof(borg_item));

        /* Restore the hole */
        memcpy(&borg_items[hole], &safe_items[hole], sizeof(borg_item));

        /* Track the crappy items */
        /* crappy includes things that do not add to power */

        if (p >= b_p) {
            b_i = i;
            w_p = p;
        }
    }

    /* Restore bonuses */
    if (fix)
        borg_notice(true);

    /* No item */
    if (b_i >= 0) {
        /* Get the item */
        item = &borg_items[b_i];

        if (borg_cfg[BORG_VERBOSE]) {
            /* dump list and power...  for debugging */
            borg_note(format("Equip Item %d %s.", i, safe_items[i].desc));
            borg_note(format("With Item     (borg_power %ld)", (long int)b_p));
            borg_note(format("Removed Item  (best power %ld)", (long int)p));
        }

        /* Log */
        borg_note(format("# Removing %s.  Power with: (%ld) Power w/o (%ld)",
            item->desc, (long int)b_p, (long int)w_p));

        /* Wear it */
        borg_keypress('t');
        borg_keypress(all_letters_nohjkl[b_i - INVEN_WIELD]);

        /* Did something */
        borg.time_this_panel++;
        return true;
    }

    /* Nope */
    return false;
}

#endif
