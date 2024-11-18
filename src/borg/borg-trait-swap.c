/**
 * \file borg-trait-swap.c
 * \brief This is used to calculate the attributes of swap weapons and armor
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

#include "borg-trait-swap.h"

#ifdef ALLOW_BORG

#include "borg-flow-kill.h"
#include "borg-inventory.h"
#include "borg-item.h"
#include "borg-magic.h"
#include "borg-trait.h"
#include "borg.h"

int weapon_swap; /* location of my swap weapon (+1 so zero is none) */
int armour_swap; /* my swap of armour (+1 so zero is none) */

int32_t weapon_swap_value;
int32_t armour_swap_value;

bool decurse_weapon_swap; /* my swap is great, except its cursed */
int  enchant_weapon_swap_to_h; /* my swap is great, except its cursed */
int  enchant_weapon_swap_to_d; /* my swap is great, except its cursed */
bool decurse_armour_swap; /* my swap is great, except its cursed */
int  enchant_armour_swap_to_a; /* my swap is great, except its cursed */

int16_t weapon_swap_digger;
uint8_t weapon_swap_slay_animal;
uint8_t weapon_swap_slay_evil;
uint8_t weapon_swap_slay_undead;
uint8_t weapon_swap_slay_demon;
uint8_t weapon_swap_slay_orc;
uint8_t weapon_swap_slay_troll;
uint8_t weapon_swap_slay_giant;
uint8_t weapon_swap_slay_dragon;
uint8_t weapon_swap_impact;
uint8_t weapon_swap_brand_acid;
uint8_t weapon_swap_brand_elec;
uint8_t weapon_swap_brand_fire;
uint8_t weapon_swap_brand_cold;
uint8_t weapon_swap_brand_pois;
uint8_t weapon_swap_see_infra;
uint8_t weapon_swap_slow_digest;
uint8_t weapon_swap_aggravate;
uint8_t weapon_swap_bad_curse;
uint8_t weapon_swap_regenerate;
uint8_t weapon_swap_telepathy;
uint8_t weapon_swap_light;
uint8_t weapon_swap_see_invis;
uint8_t weapon_swap_ffall;
uint8_t weapon_swap_free_act;
uint8_t weapon_swap_hold_life;
uint8_t weapon_swap_immune_fire;
uint8_t weapon_swap_immune_acid;
uint8_t weapon_swap_immune_cold;
uint8_t weapon_swap_immune_elec;
uint8_t weapon_swap_resist_acid;
uint8_t weapon_swap_resist_elec;
uint8_t weapon_swap_resist_fire;
uint8_t weapon_swap_resist_cold;
uint8_t weapon_swap_resist_pois;
uint8_t weapon_swap_resist_conf;
uint8_t weapon_swap_resist_sound;
uint8_t weapon_swap_resist_light;
uint8_t weapon_swap_resist_dark;
uint8_t weapon_swap_resist_chaos;
uint8_t weapon_swap_resist_disen;
uint8_t weapon_swap_resist_shard;
uint8_t weapon_swap_resist_nexus;
uint8_t weapon_swap_resist_blind;
uint8_t weapon_swap_resist_neth;
uint8_t weapon_swap_resist_fear;
uint8_t armour_swap_slay_animal;
uint8_t armour_swap_slay_evil;
uint8_t armour_swap_slay_undead;
uint8_t armour_swap_slay_demon;
uint8_t armour_swap_slay_orc;
uint8_t armour_swap_slay_troll;
uint8_t armour_swap_slay_giant;
uint8_t armour_swap_slay_dragon;
uint8_t armour_swap_impact;
uint8_t armour_swap_brand_acid;
uint8_t armour_swap_brand_elec;
uint8_t armour_swap_brand_fire;
uint8_t armour_swap_brand_cold;
uint8_t armour_swap_brand_pois;
uint8_t armour_swap_see_infra;
uint8_t armour_swap_slow_digest;
uint8_t armour_swap_aggravate;
uint8_t armour_swap_bad_curse;
uint8_t armour_swap_regenerate;
uint8_t armour_swap_telepathy;
uint8_t armour_swap_light;
uint8_t armour_swap_see_invis;
uint8_t armour_swap_ffall;
uint8_t armour_swap_free_act;
uint8_t armour_swap_hold_life;
uint8_t armour_swap_immune_fire;
uint8_t armour_swap_immune_acid;
uint8_t armour_swap_immune_cold;
uint8_t armour_swap_immune_elec;
uint8_t armour_swap_resist_acid;
uint8_t armour_swap_resist_elec;
uint8_t armour_swap_resist_fire;
uint8_t armour_swap_resist_cold;
uint8_t armour_swap_resist_pois;
uint8_t armour_swap_resist_conf;
uint8_t armour_swap_resist_sound;
uint8_t armour_swap_resist_light;
uint8_t armour_swap_resist_dark;
uint8_t armour_swap_resist_chaos;
uint8_t armour_swap_resist_disen;
uint8_t armour_swap_resist_shard;
uint8_t armour_swap_resist_nexus;
uint8_t armour_swap_resist_blind;
uint8_t armour_swap_resist_neth;
uint8_t armour_swap_resist_fear;

/*
 * for swap items for now lump all curses together as "bad"
 */
static bool borg_has_bad_curse(borg_item *item)
{
    if (item->curses[BORG_CURSE_TELEPORTATION]
        || item->curses[BORG_CURSE_POISON] || item->curses[BORG_CURSE_SIREN]
        || item->curses[BORG_CURSE_HALLUCINATION]
        || item->curses[BORG_CURSE_PARALYSIS]
        || item->curses[BORG_CURSE_DEMON_SUMMON]
        || item->curses[BORG_CURSE_DRAGON_SUMMON]
        || item->curses[BORG_CURSE_UNDEAD_SUMMON]
        || item->curses[BORG_CURSE_STONE]
        || item->curses[BORG_CURSE_ANTI_TELEPORTATION]
        || item->curses[BORG_CURSE_TREACHEROUS_WEAPON]
        || item->curses[BORG_CURSE_UNKNOWN])
        return true;
    return false;
}

/*
 * Helper function -- notice the player swap weapon
 */
void borg_notice_weapon_swap(void)
{
    int i;
    int b_i     = -1;

    int32_t v   = -1;
    int32_t b_v = -1;

    int        dam, damage;
    borg_item *item;

    weapon_swap = 0;
    weapon_swap_value = -1;

    /*** Process the inventory ***/
    for (i = 0; i < z_info->pack_size; i++) {
        item = &borg_items[i];

        /* reset counter */
        v      = -1L;
        dam    = 0;
        damage = 0;

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Hack -- skip un-aware items */
        if (!item->kind)
            continue;

        /* Skip non-wearable items */
        if (borg_wield_slot(item) == -1)
            continue;

        /* Don't carry swaps until dlevel 50.  They are heavy.
           Unless the item is a digger, then carry it */
        if (borg.trait[BI_MAXDEPTH] < 50 && item->tval != TV_DIGGING)
            continue;

        /* borg option to not use swaps (again, except diggers) */
        if (!borg_uses_swaps() && item->tval != TV_DIGGING)
            return;

        /* Clear all the swap weapon flags as I look at each one. */
        weapon_swap_digger       = 0;
        weapon_swap_slay_animal  = 0;
        weapon_swap_slay_evil    = 0;
        weapon_swap_slay_undead  = 0;
        weapon_swap_slay_demon   = 0;
        weapon_swap_slay_orc     = 0;
        weapon_swap_slay_troll   = 0;
        weapon_swap_slay_giant   = 0;
        weapon_swap_slay_dragon  = 0;
        weapon_swap_impact       = false;
        weapon_swap_brand_acid   = false;
        weapon_swap_brand_elec   = false;
        weapon_swap_brand_fire   = false;
        weapon_swap_brand_cold   = false;
        weapon_swap_brand_pois   = false;
        weapon_swap_see_infra    = false;
        weapon_swap_slow_digest  = false;
        weapon_swap_aggravate    = false;
        weapon_swap_bad_curse    = false;
        weapon_swap_regenerate   = false;
        weapon_swap_telepathy    = false;
        weapon_swap_light        = false;
        weapon_swap_see_invis    = false;
        weapon_swap_ffall        = false;
        weapon_swap_free_act     = false;
        weapon_swap_hold_life    = false;
        weapon_swap_immune_fire  = false;
        weapon_swap_immune_acid  = false;
        weapon_swap_immune_cold  = false;
        weapon_swap_immune_elec  = false;
        weapon_swap_resist_acid  = false;
        weapon_swap_resist_elec  = false;
        weapon_swap_resist_fire  = false;
        weapon_swap_resist_cold  = false;
        weapon_swap_resist_pois  = false;
        weapon_swap_resist_conf  = false;
        weapon_swap_resist_sound = false;
        weapon_swap_resist_light = false;
        weapon_swap_resist_dark  = false;
        weapon_swap_resist_chaos = false;
        weapon_swap_resist_disen = false;
        weapon_swap_resist_shard = false;
        weapon_swap_resist_nexus = false;
        weapon_swap_resist_blind = false;
        weapon_swap_resist_neth  = false;
        decurse_weapon_swap      = false;

        /* Analyze the item */
        switch (item->tval) {

            /* weapons */
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_DIGGING: {

            /* Digging */
            if (of_has(item->flags, OF_DIG_1) || of_has(item->flags, OF_DIG_2)
                || of_has(item->flags, OF_DIG_3)) {
                /* Don't notice digger if we can turn stone to mud,
                 * or I am using one.
                 */
                /* Hack -- ignore worthless ones (including cursed) */
                if (item->value <= 0)
                    break;
                if (item->cursed)
                    break;
                if (!borg_spell_legal_fail(TURN_STONE_TO_MUD, 40)
                    && !of_has(borg_items[INVEN_WIELD].flags, OF_DIG_1)
                    && !of_has(borg_items[INVEN_WIELD].flags, OF_DIG_2)
                    && !of_has(borg_items[INVEN_WIELD].flags, OF_DIG_3))
                    weapon_swap_digger = item->pval;
            }

            /* various slays */
            if (of_has(item->flags, OF_IMPACT))
                weapon_swap_impact = true;

            weapon_swap_slay_animal = item->slays[RF_ANIMAL];
            weapon_swap_slay_evil   = item->slays[RF_EVIL];
            weapon_swap_slay_undead = item->slays[RF_UNDEAD];
            weapon_swap_slay_demon  = item->slays[RF_DEMON];
            weapon_swap_slay_orc    = item->slays[RF_ORC];
            weapon_swap_slay_troll  = item->slays[RF_TROLL];
            weapon_swap_slay_giant  = item->slays[RF_GIANT];
            weapon_swap_slay_dragon = item->slays[RF_DRAGON];

            if (item->brands[ELEM_ACID])
                weapon_swap_brand_acid = true;
            if (item->brands[ELEM_ELEC])
                weapon_swap_brand_elec = true;
            if (item->brands[ELEM_FIRE])
                weapon_swap_brand_fire = true;
            if (item->brands[ELEM_COLD])
                weapon_swap_brand_cold = true;
            if (item->brands[ELEM_POIS])
                weapon_swap_brand_pois = true;

            /* Affect infravision */
            weapon_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];

            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST))
                weapon_swap_slow_digest = true;
            if (of_has(item->flags, OF_AGGRAVATE))
                weapon_swap_aggravate = true;
            if (of_has(item->flags, OF_REGEN))
                weapon_swap_regenerate = true;
            if (of_has(item->flags, OF_TELEPATHY))
                weapon_swap_telepathy = true;
            if (of_has(item->flags, OF_LIGHT_2)
                || of_has(item->flags, OF_LIGHT_3))
                weapon_swap_light = true;
            if (of_has(item->flags, OF_SEE_INVIS))
                weapon_swap_see_invis = true;
            if (of_has(item->flags, OF_FEATHER))
                weapon_swap_ffall = true;
            if (of_has(item->flags, OF_FREE_ACT))
                weapon_swap_free_act = true;
            if (of_has(item->flags, OF_HOLD_LIFE))
                weapon_swap_hold_life = true;
            if (of_has(item->flags, OF_PROT_CONF))
                weapon_swap_resist_conf = true;
            if (of_has(item->flags, OF_PROT_BLIND))
                weapon_swap_resist_blind = true;

            /* curses */
            if (borg_has_bad_curse(item))
                weapon_swap_bad_curse = true;

            /* Immunity flags */
            if (item->el_info[ELEM_FIRE].res_level == 3)
                weapon_swap_immune_fire = true;
            if (item->el_info[ELEM_ACID].res_level == 3)
                weapon_swap_immune_acid = true;
            if (item->el_info[ELEM_COLD].res_level == 3)
                weapon_swap_immune_cold = true;
            if (item->el_info[ELEM_ELEC].res_level == 3)
                weapon_swap_immune_elec = true;

            /* Resistance flags */
            if (item->el_info[ELEM_ACID].res_level > 0)
                weapon_swap_resist_acid = true;
            if (item->el_info[ELEM_ELEC].res_level > 0)
                weapon_swap_resist_elec = true;
            if (item->el_info[ELEM_FIRE].res_level > 0)
                weapon_swap_resist_fire = true;
            if (item->el_info[ELEM_COLD].res_level > 0)
                weapon_swap_resist_cold = true;
            if (item->el_info[ELEM_POIS].res_level > 0)
                weapon_swap_resist_pois = true;
            if (item->el_info[ELEM_SOUND].res_level > 0)
                weapon_swap_resist_sound = true;
            if (item->el_info[ELEM_LIGHT].res_level > 0)
                weapon_swap_resist_light = true;
            if (item->el_info[ELEM_DARK].res_level > 0)
                weapon_swap_resist_dark = true;
            if (item->el_info[ELEM_CHAOS].res_level > 0)
                weapon_swap_resist_chaos = true;
            if (item->el_info[ELEM_DISEN].res_level > 0)
                weapon_swap_resist_disen = true;
            if (item->el_info[ELEM_SHARD].res_level > 0)
                weapon_swap_resist_shard = true;
            if (item->el_info[ELEM_NEXUS].res_level > 0)
                weapon_swap_resist_nexus = true;
            if (item->el_info[ELEM_NETHER].res_level > 0)
                weapon_swap_resist_neth = true;
            if (item->uncursable)
                decurse_weapon_swap = true;

            /* Sustain flags */

            /* calculating the value of the swap weapon. */
            damage = (item->dd * (item->ds) * 25L);

            /* Reward "damage" and increased blows per round*/
            v += damage * (borg.trait[BI_BLOWS] + 1);

            /* Reward "bonus to hit" */
            v += ((borg.trait[BI_TOHIT] + item->to_h) * 100L);

            /* Reward "bonus to dam" */
            v += ((borg.trait[BI_TODAM] + item->to_d) * 75L);

            dam = damage * borg.trait[BI_BLOWS];

            /* assume 2x base damage for x% of creatures */
            dam = damage * 2 * borg.trait[BI_BLOWS];
            /* reward SAnimal if no electric brand */
            if (!borg.trait[BI_WS_ANIMAL] && !borg.trait[BI_WB_ELEC]
                && weapon_swap_slay_animal)
                v += (dam * weapon_swap_slay_animal) / 2;
            if (!borg.trait[BI_WS_EVIL] && weapon_swap_slay_evil)
                v += (dam * weapon_swap_slay_evil) / 2;

            /* assume 3x base damage for x% of creatures */
            dam = damage * 3 * borg.trait[BI_BLOWS];

            /* half of the reward now for SOrc and STroll*/
            if (!borg.trait[BI_WS_ORC] && weapon_swap_slay_orc)
                v += (dam * weapon_swap_slay_orc) / 2;
            if (!borg.trait[BI_WS_TROLL] && weapon_swap_slay_troll)
                v += (dam * 2) / 2;

            if (!borg.trait[BI_WS_UNDEAD] && weapon_swap_slay_undead)
                v += (dam * weapon_swap_slay_undead) / 2;
            if (!borg.trait[BI_WS_DEMON] && weapon_swap_slay_demon)
                v += (dam * weapon_swap_slay_demon) / 2;
            if (!borg.trait[BI_WS_GIANT] && weapon_swap_slay_giant)
                v += (dam * weapon_swap_slay_giant) / 2;
            if (!borg.trait[BI_WS_DRAGON] && !borg.trait[BI_WK_DRAGON]
                && weapon_swap_slay_dragon)
                v += (dam * weapon_swap_slay_dragon) / 2;
            if (!borg.trait[BI_WB_ACID] && weapon_swap_brand_acid)
                v += (dam * 4) / 2;
            if (!borg.trait[BI_WB_ELEC] && weapon_swap_brand_elec)
                v += (dam * 5) / 2;
            if (!borg.trait[BI_WB_FIRE] && weapon_swap_brand_fire)
                v += (dam * 3) / 2;
            if (!borg.trait[BI_WB_COLD] && weapon_swap_brand_cold)
                v += (dam * 3) / 2;
            if (!borg.trait[BI_WB_POIS] && weapon_swap_brand_pois)
                v += (dam * 3) / 2;
            /* Orcs and Trolls get the second half of the reward if SEvil is */
            /* not possessed. */
            if (!borg.trait[BI_WS_ORC] && !borg.trait[BI_WS_EVIL]
                && weapon_swap_slay_orc)
                v += (dam * weapon_swap_slay_orc) / 2;
            if (!borg.trait[BI_WS_TROLL] && !borg.trait[BI_WS_EVIL]
                && weapon_swap_slay_troll)
                v += (dam * weapon_swap_slay_troll) / 2;

            /* reward the Tunnel factor when low level */
            if (borg.trait[BI_MAXDEPTH] <= 40 && borg.trait[BI_MAXDEPTH] >= 25
                && borg.trait[BI_GOLD] < 100000 && weapon_swap_digger)
                v += (weapon_swap_digger * 3500L) + 1000L;

            /* Other Skills */
            if (!borg.trait[BI_SDIG] && weapon_swap_slow_digest)
                v += 10L;
            if (weapon_swap_aggravate)
                v -= 8000L;
            if (weapon_swap_bad_curse)
                v -= 100000L;
            if (decurse_weapon_swap)
                v -= 5000L;
            if (!borg.trait[BI_REG] && weapon_swap_regenerate)
                v += 2000L;
            if (!borg.trait[BI_ESP] && weapon_swap_telepathy)
                v += 5000L;
            if (!borg.trait[BI_LIGHT] && weapon_swap_light)
                v += 2000L;
            if (!borg.trait[BI_SINV] && weapon_swap_see_invis)
                v += 50000L;
            if (!borg.trait[BI_FEATH] && weapon_swap_ffall)
                v += 10L;
            if (!borg.trait[BI_FRACT] && weapon_swap_free_act)
                v += 10000L;
            if (!borg.trait[BI_HLIFE] && (borg.trait[BI_MAXCLEVEL] < 50)
                && weapon_swap_hold_life)
                v += 2000L;
            if (!borg.trait[BI_IFIRE] && weapon_swap_immune_fire)
                v += 70000L;
            if (!borg.trait[BI_IACID] && weapon_swap_immune_acid)
                v += 30000L;
            if (!borg.trait[BI_ICOLD] && weapon_swap_immune_cold)
                v += 50000L;
            if (!borg.trait[BI_IELEC] && weapon_swap_immune_elec)
                v += 25000L;
            if (!borg.trait[BI_RFIRE] && weapon_swap_resist_fire)
                v += 8000L;
            if (!borg.trait[BI_RACID] && weapon_swap_resist_acid)
                v += 6000L;
            if (!borg.trait[BI_RCOLD] && weapon_swap_resist_cold)
                v += 4000L;
            if (!borg.trait[BI_RELEC] && weapon_swap_resist_elec)
                v += 3000L;
            /* extra bonus for getting all basic resist */
            if (weapon_swap_resist_fire && weapon_swap_resist_acid
                && weapon_swap_resist_elec && weapon_swap_resist_cold)
                v += 10000L;
            if (!borg.trait[BI_RPOIS] && weapon_swap_resist_pois)
                v += 20000L;
            if (!borg.trait[BI_RCONF] && weapon_swap_resist_conf)
                v += 5000L;
            if (!borg.trait[BI_RSND] && weapon_swap_resist_sound)
                v += 2000L;
            if (!borg.trait[BI_RLITE] && weapon_swap_resist_light)
                v += 800L;
            if (!borg.trait[BI_RDARK] && weapon_swap_resist_dark)
                v += 800L;
            if (!borg.trait[BI_RKAOS] && weapon_swap_resist_chaos)
                v += 8000L;
            if (!borg.trait[BI_RDIS] && weapon_swap_resist_disen)
                v += 5000L;
            if (!borg.trait[BI_RSHRD] && weapon_swap_resist_shard)
                v += 100L;
            if (!borg.trait[BI_RNXUS] && weapon_swap_resist_nexus)
                v += 100L;
            if (!borg.trait[BI_RBLIND] && weapon_swap_resist_blind)
                v += 5000L;
            if (!borg.trait[BI_RNTHR] && weapon_swap_resist_neth)
                v += 5500L;
            if (!borg.trait[BI_RFEAR] && weapon_swap_resist_fear)
                v += 5500L;

            /* Special concern if Tarrasque is alive */
            if (borg.trait[BI_MAXDEPTH] >= 75
                && ((!borg.trait[BI_ICOLD] && weapon_swap_immune_cold)
                    || (!borg.trait[BI_IFIRE] && weapon_swap_immune_fire))) {
                /* If Tarraseque is alive */
                if (borg_race_death[borg_tarrasque_id] == 0) {
                    if (!borg.trait[BI_ICOLD] && weapon_swap_immune_cold)
                        v += 90000L;
                    if (!borg.trait[BI_IFIRE] && weapon_swap_immune_fire)
                        v += 90000L;
                }
            }

            /*  Mega-Hack -- resists (level 60) */
            /* its possible that he will get a sword and a cloak
             * both with the same high resist and keep each based
             * on that resist.  We want him to check to see
             * that the other swap does not already have the high resist.
             */
            if (!borg.trait[BI_RNTHR] && (borg.trait[BI_MAXDEPTH] + 1 >= 55)
                && weapon_swap_resist_neth)
                v += 100000L;
            if (!borg.trait[BI_RKAOS] && (borg.trait[BI_MAXDEPTH] + 1 >= 60)
                && weapon_swap_resist_chaos)
                v += 100000L;
            if (!borg.trait[BI_RDIS] && (borg.trait[BI_MAXDEPTH] + 1 >= 60)
                && weapon_swap_resist_disen)
                v += 100000L;

            /* some artifacts would make good back ups for their activation */

            /* skip useless ones */
            if (v <= 1000)
                continue;

            /* collect the best one */
            if (v < b_v)
                continue;

            /* track it */
            b_i = i;
            b_v = v;
        }
        }
    }

    /* Clear all the swap weapon flags. */
    weapon_swap_slay_animal  = 0;
    weapon_swap_slay_evil    = 0;
    weapon_swap_slay_undead  = 0;
    weapon_swap_slay_demon   = 0;
    weapon_swap_slay_orc     = 0;
    weapon_swap_slay_troll   = 0;
    weapon_swap_slay_giant   = 0;
    weapon_swap_slay_dragon  = 0;
    weapon_swap_impact       = false;
    weapon_swap_brand_acid   = false;
    weapon_swap_brand_elec   = false;
    weapon_swap_brand_fire   = false;
    weapon_swap_brand_cold   = false;
    weapon_swap_brand_pois   = false;
    weapon_swap_see_infra    = false;
    weapon_swap_slow_digest  = false;
    weapon_swap_aggravate    = false;
    weapon_swap_bad_curse    = false;
    weapon_swap_regenerate   = false;
    weapon_swap_telepathy    = false;
    weapon_swap_light        = false;
    weapon_swap_see_invis    = false;
    weapon_swap_ffall        = false;
    weapon_swap_free_act     = false;
    weapon_swap_hold_life    = false;
    weapon_swap_immune_fire  = false;
    weapon_swap_immune_acid  = false;
    weapon_swap_immune_cold  = false;
    weapon_swap_immune_elec  = false;
    weapon_swap_resist_acid  = false;
    weapon_swap_resist_elec  = false;
    weapon_swap_resist_fire  = false;
    weapon_swap_resist_cold  = false;
    weapon_swap_resist_pois  = false;
    weapon_swap_resist_conf  = false;
    weapon_swap_resist_sound = false;
    weapon_swap_resist_light = false;
    weapon_swap_resist_dark  = false;
    weapon_swap_resist_chaos = false;
    weapon_swap_resist_disen = false;
    weapon_swap_resist_shard = false;
    weapon_swap_resist_nexus = false;
    weapon_swap_resist_blind = false;
    weapon_swap_resist_neth  = false;
    decurse_weapon_swap      = false;

    /* Assume no enchantment needed */
    enchant_weapon_swap_to_h = 0;
    enchant_weapon_swap_to_d = 0;

    if (b_i == -1)
        return;

    /* mark the swap item and its value */
    weapon_swap_value = b_v;
    weapon_swap       = b_i + 1;

    /* Now that we know who the best swap is lets set our swap
     * flags and get a move on
     */
    /*** Process the best inven item ***/

    item = &borg_items[b_i];

    /* Enchant swap weapons (to hit) */
    if ((borg_spell_legal_fail(ENCHANT_WEAPON, 65)
            || borg.trait[BI_AENCH_SWEP] >= 1)
        && item->tval != TV_DIGGING) {
        if (item->to_h < 10) {
            enchant_weapon_swap_to_h += (10 - item->to_h);
        }

        /* Enchant my swap (to damage) */
        if (item->to_d < 10) {
            enchant_weapon_swap_to_d += (10 - item->to_d);
        }
    } else if (item->tval != TV_DIGGING) {
        if (item->to_h < 8) {
            enchant_weapon_swap_to_h += (8 - item->to_h);
        }

        /* Enchant my swap (to damage) */
        if (item->to_d < 8) {
            enchant_weapon_swap_to_d += (8 - item->to_d);
        }
    }

    /* various slays */
    weapon_swap_slay_animal = item->slays[RF_ANIMAL];
    weapon_swap_slay_evil   = item->slays[RF_EVIL];
    weapon_swap_slay_undead = item->slays[RF_UNDEAD];
    weapon_swap_slay_demon  = item->slays[RF_DEMON];
    weapon_swap_slay_orc    = item->slays[RF_ORC];
    weapon_swap_slay_troll  = item->slays[RF_TROLL];
    weapon_swap_slay_giant  = item->slays[RF_GIANT];
    weapon_swap_slay_dragon = item->slays[RF_DRAGON];
    weapon_swap_slay_undead = item->slays[RF_UNDEAD];
    weapon_swap_slay_demon  = item->slays[RF_DEMON];

    if (item->brands[ELEM_ACID])
        weapon_swap_brand_acid = true;
    if (item->brands[ELEM_ELEC])
        weapon_swap_brand_elec = true;
    if (item->brands[ELEM_FIRE])
        weapon_swap_brand_fire = true;
    if (item->brands[ELEM_COLD])
        weapon_swap_brand_cold = true;
    if (item->brands[ELEM_POIS])
        weapon_swap_brand_pois = true;

    /* Affect infravision */
    weapon_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];
    /* Affect various skills */
    /* Affect speed */

    /* Various flags */
    if (of_has(item->flags, OF_IMPACT))
        weapon_swap_impact = true;
    if (of_has(item->flags, OF_SLOW_DIGEST))
        weapon_swap_slow_digest = true;
    if (of_has(item->flags, OF_AGGRAVATE))
        weapon_swap_aggravate = true;
    if (of_has(item->flags, OF_REGEN))
        weapon_swap_regenerate = true;
    if (of_has(item->flags, OF_TELEPATHY))
        weapon_swap_telepathy = true;
    if (of_has(item->flags, OF_LIGHT_2) || of_has(item->flags, OF_LIGHT_3))
        weapon_swap_light = true;
    if (of_has(item->flags, OF_SEE_INVIS))
        weapon_swap_see_invis = true;
    if (of_has(item->flags, OF_FEATHER))
        weapon_swap_ffall = true;
    if (of_has(item->flags, OF_FREE_ACT))
        weapon_swap_free_act = true;
    if (of_has(item->flags, OF_HOLD_LIFE))
        weapon_swap_hold_life = true;
    if (of_has(item->flags, OF_PROT_CONF))
        weapon_swap_resist_conf = true;
    if (of_has(item->flags, OF_PROT_BLIND))
        weapon_swap_resist_blind = true;

    /* curses */
    if (borg_has_bad_curse(item))
        weapon_swap_bad_curse = true;

    /* Immunity flags */
    if (item->el_info[ELEM_FIRE].res_level == 3)
        weapon_swap_immune_fire = true;
    if (item->el_info[ELEM_ACID].res_level == 3)
        weapon_swap_immune_acid = true;
    if (item->el_info[ELEM_COLD].res_level == 3)
        weapon_swap_immune_cold = true;
    if (item->el_info[ELEM_ELEC].res_level == 3)
        weapon_swap_immune_elec = true;

    /* Resistance flags */
    if (item->el_info[ELEM_ELEC].res_level > 0)
        weapon_swap_resist_acid = true;
    if (item->el_info[ELEM_ELEC].res_level > 0)
        weapon_swap_resist_elec = true;
    if (item->el_info[ELEM_FIRE].res_level > 0)
        weapon_swap_resist_fire = true;
    if (item->el_info[ELEM_COLD].res_level > 0)
        weapon_swap_resist_cold = true;
    if (item->el_info[ELEM_POIS].res_level > 0)
        weapon_swap_resist_pois = true;
    if (item->el_info[ELEM_SOUND].res_level > 0)
        weapon_swap_resist_sound = true;
    if (item->el_info[ELEM_LIGHT].res_level > 0)
        weapon_swap_resist_light = true;
    if (item->el_info[ELEM_DARK].res_level > 0)
        weapon_swap_resist_dark = true;
    if (item->el_info[ELEM_CHAOS].res_level > 0)
        weapon_swap_resist_chaos = true;
    if (item->el_info[ELEM_DISEN].res_level > 0)
        weapon_swap_resist_disen = true;
    if (item->el_info[ELEM_SHARD].res_level > 0)
        weapon_swap_resist_shard = true;
    if (item->el_info[ELEM_NEXUS].res_level > 0)
        weapon_swap_resist_nexus = true;
    if (item->el_info[ELEM_NETHER].res_level > 0)
        weapon_swap_resist_neth = true;
    if (item->uncursable)
        decurse_weapon_swap = true;
}

/*
 * Helper function -- notice the player swap armour
 */
void borg_notice_armour_swap(void)
{
    int     i;
    int     b_i = -1;
    int32_t v   = -1L;
    int32_t b_v = 0L;
    int     dam, damage;

    borg_item *item;

    armour_swap = 0;
    armour_swap_value = -1;

    /* borg option to not use them */
    if (!borg_uses_swaps())
        return;

    /*** Process the inventory ***/
    for (i = 0; i < z_info->pack_size; i++) {
        item = &borg_items[i];

        /* reset counter */
        v      = -1L;
        dam    = 0;
        damage = 0;

        /* Skip empty items */
        if (!item->iqty)
            continue;

        /* Hack -- skip un-aware items */
        if (!item->kind)
            continue;

        /* Skip non-wearable items */
        if (borg_wield_slot(item) == -1)
            continue;

        /* Dont carry swaps until dlevel 50.  They are heavy */
        if (borg.trait[BI_MAXDEPTH] < 50)
            continue;

        /* Skip it if it is not decursable */
        if (item->cursed && !item->uncursable)
            continue;

        /* One Ring is not a swap */
        if (item->one_ring)
            continue;

        /* Clear all the swap weapon flags as I look at each one. */
        armour_swap_slay_animal  = 0;
        armour_swap_slay_evil    = 0;
        armour_swap_slay_undead  = 0;
        armour_swap_slay_demon   = 0;
        armour_swap_slay_orc     = 0;
        armour_swap_slay_troll   = 0;
        armour_swap_slay_giant   = 0;
        armour_swap_slay_dragon  = 0;
        armour_swap_impact       = false;
        armour_swap_brand_acid   = false;
        armour_swap_brand_elec   = false;
        armour_swap_brand_fire   = false;
        armour_swap_brand_cold   = false;
        armour_swap_brand_pois   = false;
        armour_swap_see_infra    = false;
        armour_swap_slow_digest  = false;
        armour_swap_aggravate    = false;
        armour_swap_bad_curse    = false;
        armour_swap_regenerate   = false;
        armour_swap_telepathy    = false;
        armour_swap_light        = false;
        armour_swap_see_invis    = false;
        armour_swap_ffall        = false;
        armour_swap_free_act     = false;
        armour_swap_hold_life    = false;
        armour_swap_immune_fire  = false;
        armour_swap_immune_acid  = false;
        armour_swap_immune_cold  = false;
        armour_swap_immune_elec  = false;
        armour_swap_resist_acid  = false;
        armour_swap_resist_elec  = false;
        armour_swap_resist_fire  = false;
        armour_swap_resist_cold  = false;
        armour_swap_resist_pois  = false;
        armour_swap_resist_conf  = false;
        armour_swap_resist_sound = false;
        armour_swap_resist_light = false;
        armour_swap_resist_dark  = false;
        armour_swap_resist_chaos = false;
        armour_swap_resist_disen = false;
        armour_swap_resist_shard = false;
        armour_swap_resist_nexus = false;
        armour_swap_resist_blind = false;
        armour_swap_resist_neth  = false;
        decurse_armour_swap      = false;

        /* Analyze the item */
        switch (item->tval) {
            /* ARMOUR TYPE STUFF */
        case TV_RING:
        case TV_AMULET:
        case TV_BOOTS:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR: {
            /* various slays */
            /* as of 280, armours don't have slays but random artifacts might.
             */
            armour_swap_slay_animal = item->slays[RF_ANIMAL];
            armour_swap_slay_evil   = item->slays[RF_EVIL];
            armour_swap_slay_undead = item->slays[RF_UNDEAD];
            armour_swap_slay_demon  = item->slays[RF_DEMON];
            armour_swap_slay_orc    = item->slays[RF_ORC];
            armour_swap_slay_troll  = item->slays[RF_TROLL];
            armour_swap_slay_giant  = item->slays[RF_GIANT];
            armour_swap_slay_dragon = item->slays[RF_DRAGON];
            if (of_has(item->flags, OF_IMPACT))
                armour_swap_impact = true;
            if (item->brands[ELEM_ACID])
                armour_swap_brand_acid = true;
            if (item->brands[ELEM_ELEC])
                armour_swap_brand_elec = true;
            if (item->brands[ELEM_FIRE])
                armour_swap_brand_fire = true;
            if (item->brands[ELEM_COLD])
                armour_swap_brand_cold = true;
            if (item->brands[ELEM_POIS])
                armour_swap_brand_pois = true;

            /* Affect infravision */
            armour_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];
            /* Affect various skills */
            /* Affect speed */

            /* Various flags */
            if (of_has(item->flags, OF_SLOW_DIGEST))
                armour_swap_slow_digest = true;
            if (of_has(item->flags, OF_AGGRAVATE))
                armour_swap_aggravate = true;
            if (of_has(item->flags, OF_REGEN))
                armour_swap_regenerate = true;
            if (of_has(item->flags, OF_TELEPATHY))
                armour_swap_telepathy = true;
            if (of_has(item->flags, OF_LIGHT_2)
                || of_has(item->flags, OF_LIGHT_3))
                armour_swap_light = true;
            if (of_has(item->flags, OF_SEE_INVIS))
                armour_swap_see_invis = true;
            if (of_has(item->flags, OF_FEATHER))
                armour_swap_ffall = true;
            if (of_has(item->flags, OF_FREE_ACT))
                armour_swap_free_act = true;
            if (of_has(item->flags, OF_HOLD_LIFE))
                armour_swap_hold_life = true;
            if (of_has(item->flags, OF_PROT_CONF))
                armour_swap_resist_conf = true;
            if (of_has(item->flags, OF_PROT_BLIND))
                armour_swap_resist_blind = true;

            if (borg_has_bad_curse(item))
                armour_swap_bad_curse = true;

            /* Immunity flags */
            if (item->el_info[ELEM_FIRE].res_level == 3)
                armour_swap_immune_fire = true;
            if (item->el_info[ELEM_ACID].res_level == 3)
                armour_swap_immune_acid = true;
            if (item->el_info[ELEM_COLD].res_level == 3)
                armour_swap_immune_cold = true;
            if (item->el_info[ELEM_ELEC].res_level == 3)
                armour_swap_immune_elec = true;

            /* Resistance flags */
            if (item->el_info[ELEM_ACID].res_level > 0)
                armour_swap_resist_acid = true;
            if (item->el_info[ELEM_ELEC].res_level > 0)
                armour_swap_resist_elec = true;
            if (item->el_info[ELEM_FIRE].res_level > 0)
                armour_swap_resist_fire = true;
            if (item->el_info[ELEM_COLD].res_level > 0)
                armour_swap_resist_cold = true;
            if (item->el_info[ELEM_POIS].res_level > 0)
                armour_swap_resist_pois = true;
            if (item->el_info[ELEM_SOUND].res_level > 0)
                armour_swap_resist_sound = true;
            if (item->el_info[ELEM_LIGHT].res_level > 0)
                armour_swap_resist_light = true;
            if (item->el_info[ELEM_DARK].res_level > 0)
                armour_swap_resist_dark = true;
            if (item->el_info[ELEM_CHAOS].res_level > 0)
                armour_swap_resist_chaos = true;
            if (item->el_info[ELEM_DISEN].res_level > 0)
                armour_swap_resist_disen = true;
            if (item->el_info[ELEM_SHARD].res_level > 0)
                armour_swap_resist_shard = true;
            if (item->el_info[ELEM_NEXUS].res_level > 0)
                armour_swap_resist_nexus = true;
            if (item->el_info[ELEM_NETHER].res_level > 0)
                armour_swap_resist_neth = true;
            if (item->uncursable)
                decurse_armour_swap = true;

            /* Sustain flags */

            /* calculating the value of the swap weapon. */
            damage = (item->dd * item->ds * 35L);

            /* Reward "damage" and increased blows per round*/
            v += damage * (borg.trait[BI_BLOWS] + 1);

            /* Reward "bonus to hit" */
            v += ((borg.trait[BI_TOHIT] + item->to_h) * 100L);

            /* Reward "bonus to dam" */
            v += ((borg.trait[BI_TODAM] + item->to_d) * 35L);

            dam = damage * borg.trait[BI_BLOWS];

            /* assume 2x base damage for x% of creatures */
            dam = damage * 2 * borg.trait[BI_BLOWS];

            if (!borg.trait[BI_WS_ANIMAL] && !borg.trait[BI_WB_ELEC]
                && armour_swap_slay_animal)
                v += (dam * armour_swap_slay_animal) / 2;
            if (!borg.trait[BI_WS_EVIL] && armour_swap_slay_evil)
                v += (dam * armour_swap_slay_evil) / 2;
            /* assume 3x base damage for x% of creatures */
            dam = damage * 3 * borg.trait[BI_BLOWS];

            if (!borg.trait[BI_WS_UNDEAD] && armour_swap_slay_undead)
                v += (dam * armour_swap_slay_undead) / 2;
            if (!borg.trait[BI_WS_DEMON] && armour_swap_slay_demon)
                v += (dam * armour_swap_slay_demon) / 2;
            if (!borg.trait[BI_WS_GIANT] && armour_swap_slay_giant)
                v += (dam * armour_swap_slay_giant) / 2;
            if (!borg.trait[BI_WS_DRAGON] && !borg.trait[BI_WK_DRAGON]
                && armour_swap_slay_dragon)
                v += (dam * armour_swap_slay_dragon) / 2;
            if (!borg.trait[BI_WB_ACID] && armour_swap_brand_acid)
                v += (dam * 4) / 2;
            if (!borg.trait[BI_WB_ELEC] && armour_swap_brand_elec)
                v += (dam * 5) / 2;
            if (!borg.trait[BI_WB_FIRE] && armour_swap_brand_fire)
                v += (dam * 3) / 2;
            if (!borg.trait[BI_WB_COLD] && armour_swap_brand_cold)
                v += (dam * 3) / 2;
            if (!borg.trait[BI_WB_POIS] && armour_swap_brand_pois)
                v += (dam * 3) / 2;
            /* SOrc and STroll get 1/2 reward now */
            if (!borg.trait[BI_WS_ORC] && armour_swap_slay_orc)
                v += (dam * armour_swap_slay_orc) / 2;
            if (!borg.trait[BI_WS_TROLL] && armour_swap_slay_troll)
                v += (dam * armour_swap_slay_troll) / 2;
            /* SOrc and STroll get 2/2 reward if slay evil not possessed */
            if (!borg.trait[BI_WS_ORC] && !borg.trait[BI_WS_EVIL]
                && armour_swap_slay_orc)
                v += (dam * armour_swap_slay_orc) / 2;
            if (!borg.trait[BI_WS_TROLL] && !borg.trait[BI_WS_EVIL]
                && armour_swap_slay_troll)
                v += (dam * armour_swap_slay_troll) / 2;

            if (!borg.trait[BI_SDIG] && armour_swap_slow_digest)
                v += 10L;
            if (armour_swap_aggravate)
                v -= 8000L;
            /* for now, all "bad" curses are lumped together */
            if (armour_swap_bad_curse)
                v -= 100000L;
            if (decurse_armour_swap)
                v -= 5000L;
            if (!borg.trait[BI_REG] && armour_swap_regenerate)
                v += 2000L;
            if (!borg.trait[BI_ESP] && armour_swap_telepathy)
                v += 5000L;
            if (!borg.trait[BI_LIGHT] && armour_swap_light)
                v += 2000L;
            if (!borg.trait[BI_SINV] && armour_swap_see_invis)
                v += 50000L;
            if (!borg.trait[BI_FEATH] && armour_swap_ffall)
                v += 10L;
            if (!borg.trait[BI_FRACT] && armour_swap_free_act)
                v += 10000L;
            if (!borg.trait[BI_HLIFE] && (borg.trait[BI_MAXCLEVEL] < 50)
                && armour_swap_hold_life)
                v += 2000L;
            if (!borg.trait[BI_IFIRE] && armour_swap_immune_fire)
                v += 70000L;
            if (!borg.trait[BI_IACID] && armour_swap_immune_acid)
                v += 30000L;
            if (!borg.trait[BI_ICOLD] && armour_swap_immune_cold)
                v += 50000L;
            if (!borg.trait[BI_IELEC] && armour_swap_immune_elec)
                v += 25000L;
            if (!borg.trait[BI_RFIRE] && armour_swap_resist_fire)
                v += 8000L;
            if (!borg.trait[BI_RACID] && armour_swap_resist_acid)
                v += 6000L;
            if (!borg.trait[BI_RCOLD] && armour_swap_resist_cold)
                v += 4000L;
            if (!borg.trait[BI_RELEC] && armour_swap_resist_elec)
                v += 3000L;
            /* extra bonus for getting all basic resist */
            if (armour_swap_resist_fire && armour_swap_resist_acid
                && armour_swap_resist_elec && armour_swap_resist_cold)
                v += 10000L;
            if (!borg.trait[BI_RPOIS] && armour_swap_resist_pois)
                v += 20000L;
            if (!borg.trait[BI_RCONF] && armour_swap_resist_conf)
                v += 5000L;
            if (!borg.trait[BI_RSND] && armour_swap_resist_sound)
                v += 2000L;
            if (!borg.trait[BI_RLITE] && armour_swap_resist_light)
                v += 800L;
            if (!borg.trait[BI_RDARK] && armour_swap_resist_dark)
                v += 800L;
            if (!borg.trait[BI_RKAOS] && armour_swap_resist_chaos)
                v += 8000L;
            if (!borg.trait[BI_RDIS] && armour_swap_resist_disen)
                v += 5000L;
            if (!borg.trait[BI_RSHRD] && armour_swap_resist_shard)
                v += 100L;
            if (!borg.trait[BI_RNXUS] && armour_swap_resist_nexus)
                v += 100L;
            if (!borg.trait[BI_RBLIND] && armour_swap_resist_blind)
                v += 5000L;
            if (!borg.trait[BI_RNTHR] && armour_swap_resist_neth)
                v += 5500L;
            /* Special concern if Tarrasque is alive */
            if (borg.trait[BI_MAXDEPTH] >= 75
                && ((!borg.trait[BI_ICOLD] && armour_swap_immune_cold)
                    || (!borg.trait[BI_IFIRE] && armour_swap_immune_fire))) {
                /* If Tarrasque is alive */
                if (borg_race_death[borg_tarrasque_id] == 0) {
                    if (!borg.trait[BI_ICOLD] && armour_swap_immune_cold)
                        v += 90000L;
                    if (!borg.trait[BI_IFIRE] && armour_swap_immune_fire)
                        v += 90000L;
                }
            }

            /*  Mega-Hack -- resists (level 60) */
            /* Its possible that he will get a sword and a cloak
             * both with the same high resist and keep each based
             * on that resist.  We want him to check to see
             * that the other swap does not already have the high resist.
             */
            if (!borg.trait[BI_RNTHR] && borg.trait[BI_MAXDEPTH] + 1 >= 55
                && !weapon_swap_resist_neth && armour_swap_resist_neth)
                v += 105000L;
            if (!borg.trait[BI_RKAOS] && borg.trait[BI_MAXDEPTH] + 1 >= 60
                && !weapon_swap_resist_chaos && armour_swap_resist_chaos)
                v += 104000L;
            if (!borg.trait[BI_RDIS] && borg.trait[BI_MAXDEPTH] + 1 >= 60
                && !weapon_swap_resist_disen && armour_swap_resist_disen)
                v += 100000L;

            /* some artifacts would make good back ups for their activation */
        }

            /* skip useless ones */
            if (v <= 1000)
                continue;

            /* collect the best one */
            if ((b_i >= 0) && (v < b_v))
                continue;

            /* track it */
            b_i               = i;
            b_v               = v;
            armour_swap_value = v;
            armour_swap       = i + 1;
        }
    }

    /* Now that we know who the best swap is lets set our swap
     * flags and get a move on
     */

    /* Clear all the swap weapon flags as I look at each one. */
    armour_swap_slay_animal  = 0;
    armour_swap_slay_evil    = 0;
    armour_swap_slay_undead  = 0;
    armour_swap_slay_demon   = 0;
    armour_swap_slay_orc     = 0;
    armour_swap_slay_troll   = 0;
    armour_swap_slay_giant   = 0;
    armour_swap_slay_dragon  = 0;
    armour_swap_impact       = false;
    armour_swap_brand_acid   = false;
    armour_swap_brand_elec   = false;
    armour_swap_brand_fire   = false;
    armour_swap_brand_cold   = false;
    armour_swap_brand_pois   = false;
    armour_swap_see_infra    = false;
    armour_swap_slow_digest  = false;
    armour_swap_aggravate    = false;
    armour_swap_bad_curse    = false;
    armour_swap_regenerate   = false;
    armour_swap_telepathy    = false;
    armour_swap_light        = false;
    armour_swap_see_invis    = false;
    armour_swap_ffall        = false;
    armour_swap_free_act     = false;
    armour_swap_hold_life    = false;
    armour_swap_immune_fire  = false;
    armour_swap_immune_acid  = false;
    armour_swap_immune_cold  = false;
    armour_swap_immune_elec  = false;
    armour_swap_resist_acid  = false;
    armour_swap_resist_elec  = false;
    armour_swap_resist_fire  = false;
    armour_swap_resist_cold  = false;
    armour_swap_resist_pois  = false;
    armour_swap_resist_conf  = false;
    armour_swap_resist_sound = false;
    armour_swap_resist_light = false;
    armour_swap_resist_dark  = false;
    armour_swap_resist_chaos = false;
    armour_swap_resist_disen = false;
    armour_swap_resist_shard = false;
    armour_swap_resist_nexus = false;
    armour_swap_resist_blind = false;
    armour_swap_resist_neth  = false;
    decurse_armour_swap      = false;

    if (b_i == -1)
        return;

    /*** Process the best inven item ***/
    item = &borg_items[b_i];

    /* various slays */
    armour_swap_slay_animal = item->slays[RF_ANIMAL];
    armour_swap_slay_evil   = item->slays[RF_EVIL];
    armour_swap_slay_undead = item->slays[RF_UNDEAD];
    armour_swap_slay_demon  = item->slays[RF_DEMON];
    armour_swap_slay_orc    = item->slays[RF_ORC];
    armour_swap_slay_troll  = item->slays[RF_TROLL];
    armour_swap_slay_giant  = item->slays[RF_GIANT];
    armour_swap_slay_dragon = item->slays[RF_DRAGON];

    if (item->brands[ELEM_ACID])
        armour_swap_brand_acid = true;
    if (item->brands[ELEM_ELEC])
        armour_swap_brand_elec = true;
    if (item->brands[ELEM_FIRE])
        armour_swap_brand_fire = true;
    if (item->brands[ELEM_COLD])
        armour_swap_brand_cold = true;
    if (item->brands[ELEM_POIS])
        armour_swap_brand_pois = true;

    /* Affect infravision */
    armour_swap_see_infra += item->modifiers[OBJ_MOD_INFRA];
    /* Affect various skills */
    /* Affect speed */

    /* Various flags */
    if (of_has(item->flags, OF_IMPACT))
        armour_swap_impact = true;
    if (of_has(item->flags, OF_SLOW_DIGEST))
        armour_swap_slow_digest = true;
    if (of_has(item->flags, OF_AGGRAVATE))
        armour_swap_aggravate = true;
    if (of_has(item->flags, OF_REGEN))
        armour_swap_regenerate = true;
    if (of_has(item->flags, OF_TELEPATHY))
        armour_swap_telepathy = true;
    if (of_has(item->flags, OF_LIGHT_2) || of_has(item->flags, OF_LIGHT_3))
        armour_swap_light = true;
    if (of_has(item->flags, OF_SEE_INVIS))
        armour_swap_see_invis = true;
    if (of_has(item->flags, OF_FEATHER))
        armour_swap_ffall = true;
    if (of_has(item->flags, OF_FREE_ACT))
        armour_swap_free_act = true;
    if (of_has(item->flags, OF_HOLD_LIFE))
        armour_swap_hold_life = true;
    if (of_has(item->flags, OF_PROT_CONF))
        armour_swap_resist_conf = true;
    if (of_has(item->flags, OF_PROT_BLIND))
        armour_swap_resist_blind = true;

    /* curses */
    if (borg_has_bad_curse(item))
        armour_swap_bad_curse = true;

    /* Immunity flags */
    if (item->el_info[ELEM_FIRE].res_level == 3)
        armour_swap_immune_fire = true;
    if (item->el_info[ELEM_ACID].res_level == 3)
        armour_swap_immune_acid = true;
    if (item->el_info[ELEM_COLD].res_level == 3)
        armour_swap_immune_cold = true;
    if (item->el_info[ELEM_ELEC].res_level == 3)
        armour_swap_immune_elec = true;

    /* Resistance flags */
    if (item->el_info[ELEM_ACID].res_level > 0)
        armour_swap_resist_acid = true;
    if (item->el_info[ELEM_ELEC].res_level > 0)
        armour_swap_resist_elec = true;
    if (item->el_info[ELEM_FIRE].res_level > 0)
        armour_swap_resist_fire = true;
    if (item->el_info[ELEM_COLD].res_level > 0)
        armour_swap_resist_cold = true;
    if (item->el_info[ELEM_POIS].res_level > 0)
        armour_swap_resist_pois = true;
    if (item->el_info[ELEM_SOUND].res_level > 0)
        armour_swap_resist_sound = true;
    if (item->el_info[ELEM_LIGHT].res_level > 0)
        armour_swap_resist_light = true;
    if (item->el_info[ELEM_DARK].res_level > 0)
        armour_swap_resist_dark = true;
    if (item->el_info[ELEM_CHAOS].res_level > 0)
        armour_swap_resist_chaos = true;
    if (item->el_info[ELEM_DISEN].res_level > 0)
        armour_swap_resist_disen = true;
    if (item->el_info[ELEM_SHARD].res_level > 0)
        armour_swap_resist_shard = true;
    if (item->el_info[ELEM_NEXUS].res_level > 0)
        armour_swap_resist_nexus = true;
    if (item->el_info[ELEM_NETHER].res_level > 0)
        armour_swap_resist_neth = true;
    if (item->uncursable)
        decurse_armour_swap = true;

    enchant_armour_swap_to_a = 0;

    /* don't look for enchantment on non armours */
    if (item->tval >= TV_LIGHT)
        return;

    /* Hack -- enchant the swap equipment (armor) */
    /* Note need for enchantment */
    if (borg_spell_legal_fail(ENCHANT_ARMOUR, 65)
        || borg.trait[BI_AENCH_SARM] >= 1) {
        if (item->to_a < 10) {
            enchant_armour_swap_to_a += (10 - item->to_a);
        }
    } else {
        if (item->to_a < 8) {
            enchant_armour_swap_to_a += (8 - item->to_a);
        }
    }
}

#endif
