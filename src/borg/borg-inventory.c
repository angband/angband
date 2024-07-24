/**
 * \file borg-inventory.c
 * \brief
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

#include "borg-inventory.h"

#ifdef ALLOW_BORG

#include "../obj-desc.h"

#include "borg-item-analyze.h"
#include "borg-item-id.h"
#include "borg-item.h"
#include "borg-trait.h"

/*
 * track if we need to crush junk
 */
bool borg_do_crush_junk = false;

/*
 * Return the slot that items of the given type are wielded into
 * XXX this just duplicates Angband's version and should use that instead
 *
 * Returns "-1" if the item cannot be wielded
 */
int borg_wield_slot(const borg_item *item)
{
    switch (item->tval) {
    case TV_SWORD:
    case TV_POLEARM:
    case TV_HAFTED:
    case TV_DIGGING:
        return INVEN_WIELD;

    case TV_DRAG_ARMOR:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
        return INVEN_BODY;

    case TV_SHIELD:
        return INVEN_ARM;

    case TV_CROWN:
    case TV_HELM:
        return INVEN_HEAD;

    case TV_BOW:
        return INVEN_BOW;
    case TV_RING:
        return INVEN_LEFT;
    case TV_AMULET:
        return INVEN_NECK;
    case TV_LIGHT:
        return INVEN_LIGHT;
    case TV_CLOAK:
        return INVEN_OUTER;
    case TV_GLOVES:
        return INVEN_HANDS;
    case TV_BOOTS:
        return INVEN_FEET;
    }

    /* No slot available */
    return -1;
}

/*
 * Find the slot of an item with the given tval/sval, if available.
 * Given multiple choices, choose the item with the largest "pval".
 * Given multiple choices, choose the smallest available pile.
 */
int borg_slot(int tval, int sval)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++) {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Skip un-aware items */
        if (!item->kind)
            continue;

        /* Require correct tval */
        if (item->tval != tval)
            continue;

        /* Require correct sval */
        if (item->sval != sval)
            continue;

        /* Prefer smallest pile */
        if ((n >= 0) && (item->iqty > borg_items[n].iqty))
            continue;

        /* Prefer largest "pval" (even if smaller pile)*/
        if ((n >= 0) && (item->pval < borg_items[n].pval)
            && (item->iqty > borg_items[n].iqty))
            continue;

        /* Save this item */
        n = i;
    }

    /* Done */
    return n;
}

/*
 * Cheat the quiver part of equipment
 */
static void borg_cheat_quiver(void)
{
    char buf[256];

    /* Extract the quiver */
    for (int j = 0, i = QUIVER_START; j < z_info->quiver_size; i++, j++) {
        memset(&borg_items[i], 0, sizeof(borg_item));

        struct object *obj = player->upkeep->quiver[j];
        if (obj) {
            /* Default to "nothing" */
            buf[0] = '\0';

            /* skip non items */
            if (!obj->kind)
                continue;

            /* Default to "nothing" */
            buf[0] = '\0';

            /* Describe a real item */
            if (obj->kind) {
                /* Describe it */
                object_desc(buf, sizeof(buf), obj, ODESC_FULL, player);

                /* Analyze the item (no price) */
                borg_item_analyze(&borg_items[i], obj, buf, false);

                /* Uninscribe items with ! or borg inscriptions */
                if (strstr(borg_items[i].desc, "!") || strstr(borg_items[i].desc, "borg"))
                    borg_deinscribe(i);
            }
        }
    }
}

/*
 * Cheat the "equip" screen
 */
void borg_cheat_equip(void)
{
    char buf[256];

    /* Extract the equipment */
    int count = player->body.count + z_info->pack_size;
    for (int j = 0, i = z_info->pack_size; i < count; i++, j++) {
        memset(&borg_items[i], 0, sizeof(borg_item));

        struct object *obj = player->body.slots[j].obj;
        if (obj) {
            /* Default to "nothing" */
            buf[0] = '\0';

            /* skip non items */
            if (!obj->kind)
                continue;

            /* Default to "nothing" */
            buf[0] = '\0';

            /* Describe a real item */
            if (obj->kind) {
                /* Describe it */
                object_desc(buf, sizeof(buf), obj, ODESC_FULL, player);

                /* Analyze the item (no price) */
                borg_item_analyze(&borg_items[i], obj, buf, false);

                /* Uninscribe items with ! or borg inscriptions */
                if (strstr(borg_items[i].desc, "!") || strstr(borg_items[i].desc, "borg"))
                    borg_deinscribe(i);
            }
        }
    }
    borg_cheat_quiver();
}

/*
 * Cheat the "inven" screen
 */
void borg_cheat_inven(void)
{
    int i;

    char buf[256];

    /* Extract the inventory */
    for (i = 0; i < z_info->pack_size; i++) {
        struct object *obj = player->upkeep->inven[i];
        memset(&borg_items[i], 0, sizeof(borg_item));

        /* Skip non-items */
        if (!obj || !obj->kind)
            continue;

        /* Default to "nothing" */
        buf[0] = '\0';

        /* Describe it */
        object_desc(buf, sizeof(buf), obj, ODESC_FULL, player);

        /* Skip Empty slots */
        if (streq(buf, "(nothing)"))
            continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&borg_items[i], obj, buf, false);

        /* Note changed inventory */
        borg_do_crush_junk = true;

        /* Uninscribe items with ! or borg inscriptions */
        if (strstr(borg_items[i].desc, "!") || strstr(borg_items[i].desc, "borg"))
            borg_deinscribe(i);
    }
}

/*
 * helper to find an empty slot
 */
int borg_first_empty_inventory_slot(void)
{
    int i;

    for (i = PACK_SLOTS - 1; i >= 0; i--)
        if (borg_items[i].iqty) {
            if ((i + 1) < PACK_SLOTS)
                return i + 1;
            break;
        }

    return -1;
}

/**
 * Work out if it's worth using ID on an item.  Also used in other places
 * as a general litmus test for whether an item is worth keeping hold of
 * while it's not ID'd.
 */
bool borg_item_worth_id(const borg_item *item)
{
    int        slot;

    if (!borg_item_note_needs_id(item))
        return false;

    /* Things that can't be wielded can't be ID'd (potions, scrolls etc) */
    slot = borg_wield_slot(item);
    if (slot < 0)
        return false;

    return true;
}

#endif
