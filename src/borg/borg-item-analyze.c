/**
 * \file borg-item-analyze.c
 * \brief Evaluate items to determine what powers the borg knows about
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

#include "borg-item-analyze.h"

#ifdef ALLOW_BORG

#include "../init.h"
#include "../obj-curse.h"
#include "../obj-knowledge.h"
#include "../obj-power.h"
#include "../obj-slays.h"
#include "../obj-util.h"

#include "borg-item-activation.h"
#include "borg-item-val.h"

/*
 * Determine the "base price" of a known item (see below)
 *
 * This function is adapted from "object_value_known()".
 *
 * This routine is called only by "borg_item_analyze()", which
 * uses this function to guess at the "value" of an item, if it
 * was to be sold to a store, with perfect "charisma" modifiers.
 */
static int32_t borg_object_value_known(borg_item *item)
{
    int32_t value;

    struct object_kind *k_ptr = &k_info[item->kind];

    /* Worthless items */
    if (!k_ptr->cost)
        return (0L);

    /* Extract the base value */
    value = k_ptr->cost;

    /* Hack -- use artifact base costs */
    if (item->art_idx) {
        struct artifact *a_ptr = &a_info[item->art_idx];

        /* Worthless artifacts */
        if (!a_ptr->cost)
            return (0L);

        /* Hack -- use the artifact cost */
        value = a_ptr->cost;
    }

    /* Hack -- add in ego-item bonus cost */
    if (item->ego_idx) {
        struct ego_item *e_ptr = &e_info[item->ego_idx];

        /* Worthless ego-items */
        if (!e_ptr->cost)
            return (0L);

        /* Hack -- reward the ego-item cost */
        value += e_ptr->cost;
    }

    /* Analyze pval bonus */
    switch (item->tval) {
        /* Wands/Staffs */
    case TV_WAND:
    case TV_STAFF: {
        /* Pay extra for charges */
        value += ((value / 20) * item->pval);

        break;
    }

    /* Wearable items */
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
    case TV_LIGHT:
    case TV_AMULET:
    case TV_RING: {
        /* Hack -- Negative "pval" is always bad */
        if (item->pval < 0)
            return (0L);

        /* No pval */
        if (!item->pval)
            break;

        /* Give credit for stat bonuses */
        if (of_has(item->flags, STAT_STR))
            value += (item->pval * 200L);
        if (of_has(item->flags, STAT_INT))
            value += (item->pval * 200L);
        if (of_has(item->flags, STAT_WIS))
            value += (item->pval * 200L);
        if (of_has(item->flags, STAT_DEX))
            value += (item->pval * 200L);
        if (of_has(item->flags, STAT_CON))
            value += (item->pval * 200L);

        /* Give credit for stealth and searching */
        value += (item->modifiers[OBJ_MOD_STEALTH] * 100L);
        value += (item->modifiers[OBJ_MOD_SEARCH] * 100L);

        /* Give credit for infra-vision and tunneling */
        value += (item->modifiers[OBJ_MOD_INFRA] * 50L);
        value += (item->modifiers[OBJ_MOD_TUNNEL] * 50L);

        /* Give credit for extra attacks */
        if (item->modifiers[OBJ_MOD_BLOWS]) {
            if (item->modifiers[OBJ_MOD_BLOWS] > MAX_BLOWS) {
                value += (MAX_BLOWS * 2000L);
            } else {
                value += (item->modifiers[OBJ_MOD_BLOWS] * 2000L);
            }
        }
        value += (item->modifiers[OBJ_MOD_SHOTS] * 2000L);

        /* Give credit for speed bonus */
        value += (item->modifiers[OBJ_MOD_SPEED] * 30000L);

        /* Give credit for glowing bonus */
        value += (item->modifiers[OBJ_MOD_LIGHT] * 100L);

        /* Give credit for might */
        value += (item->modifiers[OBJ_MOD_MIGHT] * 100L);

        /* Give credit for moves */
        value += (item->modifiers[OBJ_MOD_MOVES] * 100L);

        /* Give credit for damage reduction */
        value += (item->modifiers[OBJ_MOD_DAM_RED] * 200L);

        break;
    }
    }

    /* Analyze the item */
    switch (item->tval) {
        /* Rings/Amulets */
    case TV_RING:
        /* HACK special case */
        if (item->sval == sv_ring_dog)
            return (0L);

    /* Fall through */
    case TV_AMULET: {
        /* Hack -- negative bonuses are bad */
        if (item->to_a < 0)
            return (0L);
        if (item->to_h < 0)
            return (0L);
        if (item->to_d < 0)
            return (0L);

        /* Give credit for bonuses */
        value += ((item->to_h + item->to_d + item->to_a) * 100L);

        break;
    }

    /* Armor */
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_CLOAK:
    case TV_CROWN:
    case TV_HELM:
    case TV_SHIELD:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR: {
        /* Hack -- negative armor bonus */
        if (item->to_a < 0)
            return (0L);

        /* Give credit for bonuses */
        /* ignore low to_hit on armor for now since the base armor is marked */
        /* and that should be built into the value */
        if (item->to_h < 0 && item->to_h > -5)
            value += ((item->to_d + item->to_a) * 100L);
        else
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

        break;
    }

    /* Bows/Weapons */
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_SWORD:
    case TV_POLEARM: {
        /* Hack -- negative hit/damage bonuses */
        if (item->to_h + item->to_d < 0)
            return (0L);

        /* Factor in the bonuses */
        value += ((item->to_h + item->to_d + item->to_a) * 100L);

        /* Hack -- Factor in extra damage dice */
        if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds)) {
            value += (item->dd - k_ptr->dd) * item->ds * 200L;
        }

        break;
    }

    /* Ammo */
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT: {
        /* Hack -- negative hit/damage bonuses */
        if (item->to_h + item->to_d < 0)
            return (0L);

        /* Factor in the bonuses */
        value += ((item->to_h + item->to_d) * 5L);

        /* Hack -- Factor in extra damage dice */
        if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds)) {
            value += (item->dd - k_ptr->dd) * item->ds * 5L;
        }

        break;
    }
    }

    /* Return the value */
    return (value);
}

/*
 * Guess the value of un-id'd items
 */
static int32_t borg_object_value_guess(borg_item *item)
{
    int32_t value;

    /* Guess at value */
    switch (item->tval) {
    case TV_FOOD:
        value = 5L;
        break;
    case TV_POTION:
        value = 20L;
        break;
    case TV_SCROLL:
        value = 20L;
        break;
    case TV_STAFF:
        value = 70L;
        break;
    case TV_WAND:
        value = 50L;
        break;
    case TV_ROD:
        value = 90L;
        break;
    case TV_RING:
    case TV_AMULET:
        value = 45L;

        /* Hack -- negative bonuses are bad */
        if (item->to_a < 0)
            value = 0;
        if (item->to_h < 0)
            value = 0L;
        if (item->to_d < 0)
            value = 0L;
        break;
    default:
        value = 20L;

        /* Hack -- negative bonuses are bad */
        if (item->to_a < 0)
            value = 0;
        if (item->to_h < 0)
            value = 0L;
        if (item->to_d < 0)
            value = 0L;
        break;
    }

    return value;
}

/*
 * Convert from the object slays structure to a basic multiplier per race
 */
static void borg_set_slays(borg_item *item, const struct object *o)
{
    int i;
    for (i = 0; i < z_info->slay_max; i++)
        if (o->slays[i])
            item->slays[slays[i].race_flag] = slays[i].multiplier;
}

/*
 * Convert from the dynamic curses to the set the borg knows
 */
static void borg_set_curses(borg_item *item, const struct object *o)
{
    int   i;
    bool *item_curses = item->curses;

    item->uncursable  = false;
    for (i = 0; i < z_info->curse_max; i++) {
        struct curse *c = &curses[i];
        if (o->curses[i].power > 0) {
            item->cursed = true;
            if (o->curses[i].power < 100)
                item->uncursable = true;

            if (streq(c->name, "vulnerability"))
                item_curses[BORG_CURSE_VULNERABILITY] = true;
            else if (streq(c->name, "teleportation"))
                item_curses[BORG_CURSE_TELEPORTATION] = true;
            else if (streq(c->name, "dullness"))
                item_curses[BORG_CURSE_DULLNESS] = true;
            else if (streq(c->name, "sickliness"))
                item_curses[BORG_CURSE_SICKLINESS] = true;
            else if (streq(c->name, "enveloping"))
                item_curses[BORG_CURSE_ENVELOPING] = true;
            else if (streq(c->name, "irritation"))
                item_curses[BORG_CURSE_IRRITATION] = true;
            else if (streq(c->name, "weakness"))
                item_curses[BORG_CURSE_WEAKNESS] = true;
            else if (streq(c->name, "clumsiness"))
                item_curses[BORG_CURSE_CLUMSINESS] = true;
            else if (streq(c->name, "slowness"))
                item_curses[BORG_CURSE_SLOWNESS] = true;
            else if (streq(c->name, "annoyance"))
                item_curses[BORG_CURSE_ANNOYANCE] = true;
            else if (streq(c->name, "poison"))
                item_curses[BORG_CURSE_POISON] = true;
            else if (streq(c->name, "siren"))
                item_curses[BORG_CURSE_SIREN] = true;
            else if (streq(c->name, "hallucination"))
                item_curses[BORG_CURSE_HALLUCINATION] = true;
            else if (streq(c->name, "paralysis"))
                item_curses[BORG_CURSE_PARALYSIS] = true;
            else if (streq(c->name, "dragon summon"))
                item_curses[BORG_CURSE_DRAGON_SUMMON] = true;
            else if (streq(c->name, "demon summon"))
                item_curses[BORG_CURSE_DEMON_SUMMON] = true;
            else if (streq(c->name, "undead summon"))
                item_curses[BORG_CURSE_UNDEAD_SUMMON] = true;
            else if (streq(c->name, "impair mana recovery"))
                item_curses[BORG_CURSE_IMPAIR_MANA_RECOVERY] = true;
            else if (streq(c->name, "impair hitpoint recovery"))
                item_curses[BORG_CURSE_IMPAIR_HITPOINT_RECOVERY] = true;
            else if (streq(c->name, "cowardice"))
                item_curses[BORG_CURSE_COWARDICE] = true;
            else if (streq(c->name, "stone"))
                item_curses[BORG_CURSE_STONE] = true;
            else if (streq(c->name, "anti-teleportation"))
                item_curses[BORG_CURSE_ANTI_TELEPORTATION] = true;
            else if (streq(c->name, "treacherous weapon"))
                item_curses[BORG_CURSE_TREACHEROUS_WEAPON] = true;
            else if (streq(c->name, "burning up"))
                item_curses[BORG_CURSE_BURNING_UP] = true;
            else if (streq(c->name, "chilled to the bone"))
                item_curses[BORG_CURSE_CHILLED_TO_THE_BONE] = true;
            else if (streq(c->name, "steelskin"))
                item_curses[BORG_CURSE_STEELSKIN] = true;
            else if (streq(c->name, "air swing"))
                item_curses[BORG_CURSE_AIR_SWING] = true;
            else
                item_curses[BORG_CURSE_UNKNOWN] = true;
        }
    }
}

/*
 * Analyze an item, also given its name
 *
 * This cheats all the information, and maybe is getting information
 * that the player doesn't always get.  The best way to fix this is to
 * refactor the main game code to get it to make a 'fake' object that
 * contains only known info and copy from that.
 */
void borg_item_analyze(
    borg_item *item, const struct object *real_item, char *desc, bool in_store)
{
    char                *scan;
    int                  i;
    const struct object *o;

    /* Wipe the item */
    memset(item, 0, sizeof(borg_item));

    /* Non-item */
    if (!real_item->kind || !real_item->number)
        return;

    /* see if the object is fully identified.  If it is, use the base object */
    item->ident = object_fully_known(real_item);
    if (item->ident)
        o = real_item;
    else {
        /* this needs to be a good pointer or object_flags_know will crash */
        if (!real_item->known)
            o = real_item;
        else
            o = real_item->known;
    }
    item->needs_ident = !object_runes_known(real_item);

    /* Extract data from the game */
    object_flags_known(real_item, item->flags);

    /* Save the item description */
    my_strcpy(item->desc, desc, sizeof item->desc);

    /* Advance to the "inscription" or end of string and save */
    for (scan = item->desc; *scan && (*scan != '{'); scan++) /* loop */
        ;
    item->note = scan;

    /* Get various info */
    item->tval    = real_item->tval;
    item->sval    = real_item->sval;
    item->iqty    = real_item->number;
    item->weight  = object_weight_one(real_item);
    item->timeout = real_item->timeout;
    item->level   = real_item->kind->level;
    item->aware   = object_flavor_is_aware(real_item);

    /* get info from the known part of the object */
    item->ac   = o->ac;
    item->dd   = o->dd;
    item->ds   = o->ds;
    item->to_h = o->to_h;
    item->to_d = o->to_d;
    item->to_a = o->to_a;

    /* copy modifiers that are known */
    for (i = 0; i < OBJ_MOD_MAX; i++)
        item->modifiers[i] = o->modifiers[i];

    for (i = 0; i < ELEM_MAX; i++) {
        item->el_info[i].res_level = o->el_info[i].res_level;
        item->el_info[i].flags     = o->el_info[i].flags;
    }

    if (o->curses != NULL)
        borg_set_curses(item, o);

    if (o->slays)
        borg_set_slays(item, o);

    if (o->brands) {
        for (i = 0; i < z_info->brand_max; i++)
            item->brands[i] = o->brands[i];
    }

    /* check if we know this is the one ring */
    /* HACK we assume The One Ring is the only artifact that does BIZARRE */
    if (o->activation) {
        if (o->activation->index == act_bizarre)
            item->one_ring = true;
        item->activ_idx = o->activation->index;
    } else {
        /* assign special activations that are now effects */
        if (item->tval == TV_RING) {
            if (item->sval == sv_ring_flames)
                item->activ_idx = act_ring_flames;
            if (item->sval == sv_ring_acid)
                item->activ_idx = act_ring_acid;
            if (item->sval == sv_ring_ice)
                item->activ_idx = act_ring_ice;
            if (item->sval == sv_ring_lightning)
                item->activ_idx = act_ring_lightning;
        }
        /* NOTE two activations are missed (don't have activation indexes) */
        /* white and black dragon */
        if (item->tval == TV_DRAG_ARMOR) {
            if (item->sval == sv_dragon_blue)
                item->activ_idx = act_dragon_blue;
            if (item->sval == sv_dragon_red)
                item->activ_idx = act_dragon_red;
            if (item->sval == sv_dragon_green)
                item->activ_idx = act_dragon_green;
            if (item->sval == sv_dragon_multihued)
                item->activ_idx = act_dragon_multihued;
            if (item->sval == sv_dragon_shining)
                item->activ_idx = act_dragon_shining;
            if (item->sval == sv_dragon_law)
                item->activ_idx = act_dragon_law;
            if (item->sval == sv_dragon_gold)
                item->activ_idx = act_dragon_gold;
            if (item->sval == sv_dragon_chaos)
                item->activ_idx = act_dragon_chaos;
            if (item->sval == sv_dragon_balance)
                item->activ_idx = act_dragon_balance;
            if (item->sval == sv_dragon_power)
                item->activ_idx = act_dragon_power;
        }
    }

    /* default the pval */
    if (item->ident)
        item->pval = o->pval;

    /* Rods are considered pval 1 if charged */
    if (item->tval == TV_ROD) {
        /* XXX There should be an obj_rod_charging() function for this logic */
        /* This was ripped from object/obj-desc.c */
        if (item->iqty == 1) {
            item->pval = real_item->timeout ? 0 : 1;
        } else {
            int power;
            int time_base = randcalc(real_item->kind->time, 0, MINIMISE);
            if (!time_base)
                time_base = 1;

            /*
             * Find out how many rods are charging, by dividing
             * current timeout by each rod's maximum timeout.
             * Ensure that any remainder is rounded up.  Display
             * very discharged stacks as merely fully discharged.
             */
            power      = (real_item->timeout + (time_base - 1)) / time_base;
            item->pval = (power < item->iqty) ? 1 : 0;
        }
    } else if (item->tval == TV_STAFF || item->tval == TV_WAND) {
        /* Staffs & wands considered charged unless they are known empty */

        /* Assume good */
        item->pval = 1;

        /* if Known, get correct pval */
        if (item->ident)
            item->pval = real_item->pval;

        /* if seen {empty} assume pval 0 */
        if (!item->aware && !o->pval)
            item->pval = 0;
        if (strstr(borg_get_note(item), "empty"))
            item->pval = 0;
    }

    /* Kind index -- Only if partially ID or this is a store object */
    if (item->aware || in_store)
        item->kind = o->kind->kidx;

    if (o->artifact)
        item->art_idx = o->artifact->aidx;

    if (o->ego)
        item->ego_idx = o->ego->eidx;

    /* Notice values */
    if (item->ident) {
        item->value = borg_object_value_known(item);
    } else if (item->aware) {
        item->value = o->kind->cost;
    } else {
        item->value = borg_object_value_guess(item);
    }

    /* If it's not The One Ring, then it's worthless if cursed */
    if (item->cursed && !item->one_ring)
        item->value = 0L;
}

/*
 * Helper to see if an object does a certain thing
 *
 * kind     - kind index
 * index    - index of the effect
 * subtype  - subtype of the effect
 */
bool borg_obj_has_effect(uint32_t kind, int index, int subtype)
{
    struct effect *ke = k_info[kind].effect;
    while (ke) {
        if (ke->index == index && (ke->subtype == subtype || subtype == -1))
            return true;
        ke = ke->next;
    }
    return false;
}

/*
 * This checks if an item has a random power or sustain
 */
bool borg_ego_has_random_power(struct ego_item *e_ptr)
{
    if (kf_has(e_ptr->kind_flags, KF_RAND_POWER)
        || kf_has(e_ptr->kind_flags, KF_RAND_SUSTAIN)
        || kf_has(e_ptr->kind_flags, KF_RAND_BASE_RES)
        || kf_has(e_ptr->kind_flags, KF_RAND_HI_RES)
        || kf_has(e_ptr->kind_flags, KF_RAND_RES_POWER))
        return true;
    return false;
}

#endif
