/**
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
#ifndef INCLUDED_BORG_ITEM_H
#define INCLUDED_BORG_ITEM_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "../init.h"
#include "../monster.h"

/*
 * Forward declare
 */
typedef struct borg_item borg_item;

enum {
    BORG_CURSE_UNKNOWN                  = 0,
    BORG_CURSE_VULNERABILITY            = 1,
    BORG_CURSE_TELEPORTATION            = 2,
    BORG_CURSE_DULLNESS                 = 3,
    BORG_CURSE_SICKLINESS               = 4,
    BORG_CURSE_ENVELOPING               = 5,
    BORG_CURSE_IRRITATION               = 6,
    BORG_CURSE_WEAKNESS                 = 7,
    BORG_CURSE_CLUMSINESS               = 8,
    BORG_CURSE_SLOWNESS                 = 9,
    BORG_CURSE_ANNOYANCE                = 10,
    BORG_CURSE_POISON                   = 11,
    BORG_CURSE_SIREN                    = 12,
    BORG_CURSE_HALLUCINATION            = 13,
    BORG_CURSE_PARALYSIS                = 14,
    BORG_CURSE_DRAGON_SUMMON            = 15,
    BORG_CURSE_DEMON_SUMMON             = 16,
    BORG_CURSE_UNDEAD_SUMMON            = 17,
    BORG_CURSE_IMPAIR_MANA_RECOVERY     = 18,
    BORG_CURSE_IMPAIR_HITPOINT_RECOVERY = 19,
    BORG_CURSE_COWARDICE                = 20,
    BORG_CURSE_STONE                    = 21,
    BORG_CURSE_ANTI_TELEPORTATION       = 22,
    BORG_CURSE_TREACHEROUS_WEAPON       = 23,
    BORG_CURSE_BURNING_UP               = 24,
    BORG_CURSE_CHILLED_TO_THE_BONE      = 25,
    BORG_CURSE_STEELSKIN                = 26,
    BORG_CURSE_AIR_SWING                = 27,
    BORG_CURSE_MAX                      = 28,
};

/*
 * A structure holding information about an object.  120 bytes.
 *
 * The "iqty" is zero if the object is "missing"
 * The "kind" is zero if the object is "unaware" (or missing)
 * The "able" is zero if the object is "unknown" (or unaware or missing)
 *
 * Note that unaware items will have a "tval" but an invalid "sval".
 */
struct borg_item {
    char desc[80]; /* Actual Description */

    char *note; /* Pointer to tail of 'desc' */

    uint32_t kind; /* Kind index */

    bool ident; /* True if item is identified */
    bool needs_ident; /* True if item needs to be identified */
    /* (not all items have runes that can be identified) */
    bool aware; /* Player is aware of the effects */

    bool xxxx; /* Unused */

    uint8_t tval; /* Item type */
    uint8_t sval; /* Item sub-type */
    int16_t pval; /* Item extra-info */

    uint8_t iqty; /* Number of items */

    int16_t weight; /* Probable weight */

    uint8_t art_idx; /* Artifact index (if any) */
    uint8_t ego_idx; /* Ego-item index (if any) */
    int     activ_idx; /* Activation index (if any) */
    bool    one_ring; /* is this the one ring */

    int16_t timeout; /* Timeout counter */

    int16_t to_h; /* Bonus to hit */
    int16_t to_d; /* Bonus to dam */
    int16_t to_a; /* Bonus to ac */
    int16_t ac; /* Armor class */
    uint8_t dd; /* Damage dice */
    uint8_t ds; /* Damage sides */

    uint8_t level; /* Level  */

    int32_t cost; /* Cost (in stores) */

    int32_t value; /* Value (estimated) */

    bool cursed; /* Item is cursed */
    bool uncursable; /* Item can be uncursed */
    bool curses[BORG_CURSE_MAX];

    bitflag             flags[OF_SIZE]; /**< Object flags */
    int16_t             modifiers[OBJ_MOD_MAX]; /**< Object modifiers*/
    struct element_info el_info[ELEM_MAX]; /**< Object element info */
    bool                brands[254]; /**< Flag absence/presence of each brand */
    /* HACK this should be dynamic but we don't know when borg_item's go away */
    int slays[RF_MAX]; /**< power of slays based on race flag */
};

/*
 * Indexes used for various "equipment" slots (hard-coded by savefiles, etc).
 */
#define INVEN_WIELD z_info->pack_size
#define INVEN_BOW   (z_info->pack_size + 1)
#define INVEN_RIGHT (z_info->pack_size + 2)
#define INVEN_LEFT  (z_info->pack_size + 3)
#define INVEN_NECK  (z_info->pack_size + 4)
#define INVEN_LIGHT (z_info->pack_size + 5)
#define INVEN_BODY  (z_info->pack_size + 6)
#define INVEN_OUTER (z_info->pack_size + 7)
#define INVEN_ARM   (z_info->pack_size + 8)
#define INVEN_HEAD  (z_info->pack_size + 9)
#define INVEN_HANDS (z_info->pack_size + 10)
#define INVEN_FEET  (z_info->pack_size + 11)

/*
 * Total number of inventory slots.
 */
#define INVEN_TOTAL (z_info->pack_size + 12)

/*
 * Total number of pack slots available (not used by quiver)
 */
#define PACK_SLOTS (z_info->pack_size - borg.trait[BI_QUIVER_SLOTS])

/* Quiver */
#define QUIVER_START INVEN_TOTAL
#define QUIVER_SIZE  (z_info->quiver_size)
#define QUIVER_END   (QUIVER_START + QUIVER_SIZE)

/*
 * Current "inventory"
 */
extern borg_item *borg_items;

/*
 * Safety arrays for simulating possible worlds
 */

extern borg_item *safe_items; /* Safety "inventory" */

/* get the items inscription (note) */
extern const char *borg_get_note(const borg_item *item);

/* remove the items inscription (note) */
extern void borg_deinscribe(int i);

/* initialize and free items */
extern void borg_init_item(void);
extern void borg_free_item(void);

#endif
#endif
