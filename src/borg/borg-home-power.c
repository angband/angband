/**
 * \file  borg-home-power.c
 * \brief Determine the power value of the home
 *           this is used to decide what items are best to keep at home
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

#include "borg-home-power.h"

#ifdef ALLOW_BORG

#include "borg-home-notice.h"
#include "borg-magic.h"
#include "borg-trait.h"

/*
 * Helper function -- calculate power of equipment in the home
 */
static int32_t borg_power_home_aux1(void)
{
    int32_t value = 0L;

    /* This would be better separated by item type (so 1 bonus for resist cold
     * armor */
    /*   1 bonus for resist cold shield... but that would take a bunch more
     * code. */

    /* try to collect at least 2 of each resist/power (for swapping) */
    /* This can be used to get rid of extra artifacts... */

    /* spare lite sources.  Artifacts only */
    if (num_LIGHT == 1)
        value += 150L;
    else if (num_LIGHT == 2)
        value += 170L;
    else if (num_LIGHT > 2)
        value += 170L + (num_LIGHT - 2) * 5L;

    if (num_slow_digest == 1)
        value += 50L;
    else if (num_slow_digest == 2)
        value += 70L;
    else if (num_slow_digest > 2)
        value += 70L + (num_slow_digest - 2) * 5L;

    if (num_regenerate == 1)
        value += 75L;
    else if (num_regenerate == 2)
        value += 100L;
    else if (num_regenerate > 2)
        value += 100L + (num_regenerate - 2) * 10L;

    if (num_telepathy == 1)
        value += 1000L;
    else if (num_telepathy == 2)
        value += 1500L;
    else if (num_telepathy > 2)
        value += 1500L + (num_telepathy - 2) * 10L;

    if (num_see_inv == 1)
        value += 800L;
    else if (num_see_inv == 2)
        value += 1200L;
    else if (num_see_inv > 2)
        value += 1200L + (num_see_inv - 2) * 10L;

    if (num_ffall == 1)
        value += 10L;
    else if (num_ffall == 2)
        value += 15L;
    else if (num_ffall > 2)
        value += 15L + (num_ffall - 2) * 1L;

    if (num_free_act == 1)
        value += 1000L;
    else if (num_free_act == 2)
        value += 1500L;
    else if (num_free_act > 2)
        value += 1500L + (num_free_act - 2) * 10L;

    if (num_hold_life == 1)
        value += 1000L;
    else if (num_hold_life == 2)
        value += 1500L;
    else if (num_hold_life > 2)
        value += 1500L + (num_hold_life - 2) * 10L;

    if (num_resist_acid == 1)
        value += 1000L;
    else if (num_resist_acid == 2)
        value += 1500L;
    else if (num_resist_acid > 2)
        value += 1500L + (num_resist_acid - 2) * 1L;
    if (num_immune_acid == 1)
        value += 3000L;
    else if (num_immune_acid == 2)
        value += 5000L;
    else if (num_immune_acid > 2)
        value += 5000L + (num_immune_acid - 2) * 30L;

    if (num_resist_elec == 1)
        value += 1000L;
    else if (num_resist_elec == 2)
        value += 1500L;
    else if (num_resist_elec > 2)
        value += 1500L + (num_resist_elec - 2) * 1L;
    if (num_immune_elec == 1)
        value += 3000L;
    else if (num_immune_elec == 2)
        value += 5000L;
    else if (num_immune_elec > 2)
        value += 5000L + (num_immune_elec - 2) * 30L;

    if (num_resist_fire == 1)
        value += 1000L;
    else if (num_resist_fire == 2)
        value += 1500L;
    else if (num_resist_fire > 2)
        value += 1500L + (num_resist_fire - 2) * 1L;
    if (num_immune_fire == 1)
        value += 3000L;
    else if (num_immune_fire == 2)
        value += 5000L;
    else if (num_immune_fire > 2)
        value += 5000L + (num_immune_fire - 2) * 30L;

    if (num_resist_cold == 1)
        value += 1000L;
    else if (num_resist_cold == 2)
        value += 1500L;
    else if (num_resist_cold > 2)
        value += 1500L + (num_resist_cold - 2) * 1L;
    if (num_immune_cold == 1)
        value += 3000L;
    else if (num_immune_cold == 2)
        value += 5000L;
    else if (num_immune_cold > 2)
        value += 5000L + (num_immune_cold - 2) * 30L;

    if (num_resist_pois == 1)
        value += 5000L;
    else if (num_resist_pois == 2)
        value += 9000L;
    else if (num_resist_pois > 2)
        value += 9000L + (num_resist_pois - 2) * 40L;

    if (num_resist_conf == 1)
        value += 2000L;
    else if (num_resist_conf == 2)
        value += 8000L;
    else if (num_resist_conf > 2)
        value += 8000L + (num_resist_conf - 2) * 45L;

    if (num_resist_sound == 1)
        value += 500L;
    else if (num_resist_sound == 2)
        value += 700L;
    else if (num_resist_sound > 2)
        value += 700L + (num_resist_sound - 2) * 30L;

    if (num_resist_LIGHT == 1)
        value += 100L;
    else if (num_resist_LIGHT == 2)
        value += 150L;
    else if (num_resist_LIGHT > 2)
        value += 150L + (num_resist_LIGHT - 2) * 1L;

    if (num_resist_dark == 1)
        value += 100L;
    else if (num_resist_dark == 2)
        value += 150L;
    else if (num_resist_dark > 2)
        value += 150L + (num_resist_dark - 2) * 1L;

    if (num_resist_chaos == 1)
        value += 1000L;
    else if (num_resist_chaos == 2)
        value += 1500L;
    else if (num_resist_chaos > 2)
        value += 1500L + (num_resist_chaos - 2) * 10L;

    if (num_resist_disen == 1)
        value += 5000L;
    else if (num_resist_disen == 2)
        value += 7000L;
    else if (num_resist_disen > 2)
        value += 7000L + (num_resist_disen - 2) * 35L;

    if (num_resist_shard == 1)
        value += 100L;
    else if (num_resist_shard == 2)
        value += 150L;
    else if (num_resist_shard > 2)
        value += 150L + (num_resist_shard - 2) * 1L;

    if (num_resist_nexus == 1)
        value += 200L;
    else if (num_resist_nexus == 2)
        value += 300L;
    else if (num_resist_nexus > 2)
        value += 300L + (num_resist_nexus - 2) * 2L;

    if (num_resist_blind == 1)
        value += 500L;
    else if (num_resist_blind == 2)
        value += 1000L;
    else if (num_resist_blind > 2)
        value += 1000L + (num_resist_blind - 2) * 5L;

    if (num_resist_neth == 1)
        value += 3000L;
    else if (num_resist_neth == 2)
        value += 4000L;
    else if (num_resist_neth > 2)
        value += 4000L + (num_resist_neth - 2) * 45L;

    /* stat gain items as well...(good to carry ring of dex +6 in */
    /*                            house even if I don't need it right now) */
    if (home_stat_add[STAT_STR] < 9)
        value += home_stat_add[STAT_STR] * 300L;
    else if (home_stat_add[STAT_STR] < 15)
        value += 9 * 300L + (home_stat_add[STAT_STR] - 9) * 200L;
    else
        value += 9 * 300L + 6 * 200L + (home_stat_add[STAT_STR] - 15) * 1L;

    if (home_stat_add[STAT_DEX] < 9)
        value += home_stat_add[STAT_DEX] * 300L;
    else if (home_stat_add[STAT_DEX] < 15)
        value += 9 * 300L + (home_stat_add[STAT_DEX] - 9) * 200L;
    else
        value += 9 * 300L + 6 * 200L + (home_stat_add[STAT_DEX] - 15) * 1L;

    /* HACK extra con for thorin and other such things */
    if (home_stat_add[STAT_CON] < 15)
        value += home_stat_add[STAT_CON] * 300L;
    else if (home_stat_add[STAT_CON] < 21)
        value += 15 * 300L + (home_stat_add[STAT_CON] - 15) * 200L;
    else
        value += 15 * 300L + 6 * 200L + (home_stat_add[STAT_CON] - 21) * 1L;

    /* spell stat is only bonused for spell casters. */
    int spell_stat = borg_spell_stat();
    if (spell_stat >= 0) {
        if (home_stat_add[spell_stat] < 20)
            value += home_stat_add[spell_stat] * 400L;
        else if (home_stat_add[spell_stat] < 26)
            value += 20 * 400L + (home_stat_add[spell_stat] - 20) * 300L;
        else
            value
                += 20 * 100L + 6 * 300L + (home_stat_add[spell_stat] - 26) * 5L;
    }

    /* Sustains */
    if (num_sustain_str == 1)
        value += 200L;
    else if (num_sustain_str == 2)
        value += 250L;
    else if (num_sustain_str > 2)
        value += 250L + (num_sustain_str - 2) * 1L;

    if (num_sustain_int == 1)
        value += 200L;
    else if (num_sustain_int == 2)
        value += 250L;
    else if (num_sustain_int > 2)
        value += 250L + (num_sustain_int - 2) * 1L;

    if (num_sustain_wis == 1)
        value += 200L;
    else if (num_sustain_wis == 2)
        value += 250L;
    else if (num_sustain_wis > 2)
        value += 250L + (num_sustain_wis - 2) * 1L;

    if (num_sustain_con == 1)
        value += 200L;
    else if (num_sustain_con == 2)
        value += 250L;
    else if (num_sustain_con > 2)
        value += 250L + (num_sustain_con - 2) * 1L;

    if (num_sustain_dex == 1)
        value += 200L;
    else if (num_sustain_dex == 2)
        value += 250L;
    else if (num_sustain_dex > 2)
        value += 250L + (num_sustain_dex - 2) * 1L;

    if (num_sustain_all == 1)
        value += 1000L;
    else if (num_sustain_all == 2)
        value += 1500L;
    else if (num_sustain_all > 2)
        value += 1500L + (num_sustain_all - 2) * 1L;

    /* do a minus for too many duplicates.  This way we do not store */
    /* useless items and spread out types of items. */
    if (num_weapons > 5)
        value -= (num_weapons - 5) * 2000L;
    else if (num_weapons > 1)
        value -= (num_weapons - 1) * 100L;
    if (num_bow > 2)
        value -= (num_bow - 2) * 1000L;
    if (num_rings > 6)
        value -= (num_rings - 6) * 4000L;
    else if (num_rings > 4)
        value -= (num_rings - 4) * 2000L;
    if (num_neck > 3)
        value -= (num_neck - 3) * 1500L;
    else if (num_neck > 3)
        value -= (num_neck - 3) * 700L;
    if (num_armor > 6)
        value -= (num_armor - 6) * 1000L;
    if (num_cloaks > 3)
        value -= (num_cloaks - 3) * 1000L;
    if (num_shields > 3)
        value -= (num_shields - 3) * 1000L;
    if (num_hats > 4)
        value -= (num_hats - 4) * 1000L;
    if (num_gloves > 3)
        value -= (num_gloves - 3) * 1000L;
    if (num_boots > 3)
        value -= (num_boots - 3) * 1000L;

    value += home_damage;

    /* if edged and priest, dump it   */
    value -= num_edged_weapon * 3000L;

    /* if gloves and mage or ranger and not FA/Dex, dump it. */
    value -= num_bad_gloves * 3000L;

    /* do not allow duplication of items. */
    value -= num_duplicate_items * 50000L;

    /* Return the value */
    return (value);
}

/*
 * Helper function -- calculate power of items in the home
 *
 * The weird calculations help spread out the purchase order
 */
static int32_t borg_power_home_aux2(void)
{
    int k, book;

    int32_t value = 0L;

    /*** Basic abilities ***/

    /* Collect food */
    if (borg.trait[BI_MAXCLEVEL] < 10) {
        for (k = 0; k < kb_info[TV_FOOD].max_stack && k < num_food; k++)
            value += 8000L - k * 10L;
    }

    /* Collect ident */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_ident; k++)
        value += 2000L - k * 10L;

    /* Collect enchantments armour */
    if (borg.trait[BI_CLEVEL] < 45) {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_enchant_to_a;
             k++)
            value += 500L - k * 10L;
    }
    /* Collect enchantments to hit */
    if (borg.trait[BI_CLEVEL] < 45) {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_enchant_to_h;
             k++)
            value += 500L - k * 10L;
    }
    /* Collect enchantments to dam */
    if (borg.trait[BI_CLEVEL] < 45) {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_enchant_to_d;
             k++)
            value += 500L - k * 10L;
    }

    /* Collect pfe */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_pfe; k++)
        value += 500L - k * 10L;

    /* Collect glyphs */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_glyph; k++)
        value += 500L - k * 10L;

    /* Reward Genocide scrolls. Just scrolls, mainly used for Morgoth */
    for (k = 0; k < (kb_info[TV_SCROLL].max_stack * 2) && k < num_genocide; k++)
        value += 500L - k * 10L;

    /* Reward Mass Genocide scrolls. Just scrolls, mainly used for Morgoth */
    for (k = 0; k < (kb_info[TV_SCROLL].max_stack * 2) && k < num_mass_genocide;
         k++)
        value += 500L;

    /* Collect Recharge ability */
    for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_recharge; k++)
        value += 500L - k * 10L;

    /* Reward Resistance Potions for Warriors */
    if (borg.trait[BI_CLASS] == CLASS_WARRIOR && borg.trait[BI_MAXDEPTH] > 20
        && borg.trait[BI_MAXDEPTH] < 80) {
        k = 0;
        for (; k < kb_info[TV_POTION].max_stack && k < num_pot_rheat; k++)
            value += 100L - k * 10L;
        for (; k < kb_info[TV_POTION].max_stack && k < num_pot_rcold; k++)
            value += 100L - k * 10L;
    }

    /* Collect recall - stick to 5 spare, for if you run out of money */
    for (k = 0; k < 5 && k < num_recall; k++)
        value += 100L;

    /* Collect escape  (staff of teleport) */
    if (borg.trait[BI_MAXCLEVEL] < 40) {
        for (k = 0; k < 85 && k < num_escape; k++)
            value += 2000L - k * 10L;
    }

    /* Collect a maximal number of staves in the home */
    for (k = 0; k < kb_info[TV_STAFF].max_stack && k < num_tele_staves; k++)
        value -= 50000L;

    /* Collect teleport */
    for (k = 0; k < 85 && k < num_teleport; k++)
        value += 5000L;

    /* Collect phase */
    if (borg.trait[BI_MAXCLEVEL] < 10) {
        for (k = 0; k < kb_info[TV_SCROLL].max_stack && k < num_phase; k++)
            value += 5000L;
    }

    /* Collect Speed */
    /* for (k = 0; k < 85 && k < num_speed; k++) value += 5000L - k*10L; */

    /* collect mana/ */
    if (borg.trait[BI_MAXSP] > 1) {
        for (k = 0; k < kb_info[TV_POTION].max_stack && k < num_mana; k++)
            value += 6000L - k * 8L;
    }

    /* Level 1 priests are given a Potion of Healing.  It is better
     * for them to sell that potion and buy equipment or several
     * Cure Crits with it.
     */
    if (borg.trait[BI_CLEVEL] == 1) {
        k = 0;
        for (; k < 10 && k < num_heal; k++)
            value -= 5000L;
    }

    /*** Healing ***/

    /* Collect cure critical */
    for (k = 0; k < kb_info[TV_POTION].max_stack && k < num_cure_critical; k++)
        value += 1500L - k * 10L;

    /* Collect heal, *Heal*, Life */
    for (k = 0; k < 90 && k < num_heal; k++)
        value += 3000L;
    for (k = 0; k < 198 && k < num_ezheal; k++)
        value += 8000L;
    for (k = 0; k < 198 && k < num_life; k++)
        value += 9000L;

    /* junk cure serious if we have some in the home */
    /* don't bother keeping them if high level */
    if (borg.trait[BI_CLEVEL] > 35)
        for (k = 0; k < 90 && k < num_cure_serious; k++)
            value -= 1500L - k * 10L;

    /*** Various ***/

    /* Fixing Stats */
    if (borg.trait[BI_CLEVEL] == 50 && num_fix_exp)
        value -= 7500L;
    if (borg.trait[BI_CLEVEL] > 35 && borg.trait[BI_CLEVEL] <= 49)
        for (k = 0; k < 70 && k < num_fix_exp; k++)
            value += 1000L - k * 10L;
    else if (borg.trait[BI_CLEVEL] <= 35)
        for (k = 0; k < 5 && k < num_fix_exp; k++)
            value += 1000L - k * 10L;

    /*** Hack -- books ***/

    /* Reward books */
    for (book = 0; book < 9; book++) {
        /* only collect books up to level 14.  */
        /* After that, just buy them, they are always in stock*/
        if (borg.trait[BI_CLEVEL] < 15) {
            /* Collect up to 5 copies of each normal book */
            for (k = 0; k < 5 && k < num_book[book]; k++) {
                /* Hack -- only stockpile useful books */
                if (num_book[book])
                    value += 5000L - k * 10L;
            }
        }
    }

    /* Reward artifacts in the home */
    value += num_artifact * 500L;

    /* Reward certain types of egos in the home */
    value += num_ego * 5000L;

    /* Only allow unid'd stuff if we can't id them */
    if (home_un_id)
        value += (home_un_id - borg.trait[BI_AID]) * 1005L;

    /* Return the value */
    return (value);
}

/*
 * Calculate the "power" of the home
 */
int32_t borg_power_home(void)
{
    int32_t value = 0L;

    /* Process the home equipment */
    value += borg_power_home_aux1();

    /* Process the home inventory */
    value += borg_power_home_aux2();

    /* Return the value */
    return (value);
}

#endif
