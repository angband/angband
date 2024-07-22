/**
 * \file borg-fight-perm.c
 * \brief Do moves we want as persistent during a fight and beyond.
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

#include "borg-fight-perm.h"

#ifdef ALLOW_BORG

#include "borg-cave-view.h"
#include "borg-flow-kill.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-magic.h"
#include "borg-trait.h"

/*
 * Perma spells.  Some are cool to have on all the time, so long as their
 * mana cost is not too much.
 * There are several types of setup moves:
 *
 *   Temporary speed
 *   Protect From Evil
 *   Prayer
 *   Temp Resist (either all or just cold/fire?)
 *   Shield
 */
enum {
    BP_SPEED,
    BP_PROT_FROM_EVIL,
    BP_BLESS,

    BP_RESIST_ALL,
    BP_RESIST_ALL_COLLUIN,
    BP_RESIST_P,

    BP_FASTCAST,
    BP_HERO,
    BP_BERSERK,
    BP_BERSERK_POTION,

    BP_SMITE_EVIL,
    BP_VENOM,
    BP_REGEN,

    BP_GLYPH,
    BP_SEE_INV,

    BP_MAX
};

/*
 * Prayer to prepare for battle
 */
static int borg_perma_aux_bless(void)
{
    int fail_allowed = 15, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 20;
    if (borg_fighting_unique)
        fail_allowed = 25;

    /* already blessed */
    if (borg.temp.bless)
        return 0;

    /* Cant when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return 0;

    /* XXX Dark */

    if (!borg_spell_okay_fail(BLESS, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(BLESS);

    /* If its cheap, go ahead */
    if (borg.trait[BI_CLEVEL] > 10
        && cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                      : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    /* bless is a low priority */
    if (borg_simulate)
        return 1;

    /* do it! */
    borg_spell(BLESS);

    /* No resting to recoup mana */
    borg.no_rest_prep = 10000;

    return 1;
}

/* all resists FECAP*/
static int borg_perma_aux_resist(void)
{
    int cost         = 0;
    int fail_allowed = 5;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    if (borg.temp.res_fire + borg.temp.res_acid + borg.temp.res_elec
            + borg.temp.res_cold
        >= 3)
        return 0;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(RESISTANCE);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    if (borg_simulate)
        return 2;

    /* do it! */
    borg_spell_fail(RESISTANCE, fail_allowed);

    /* No resting to recoup mana */
    borg.no_rest_prep = 21000;

    /* default to can't do it. */
    return 2;
}

/* all resists from the cloak*/
static int borg_perma_aux_resist_colluin(void)
{
    if (borg.temp.res_fire + borg.temp.res_acid + borg.temp.res_pois
            + borg.temp.res_elec + borg.temp.res_cold
        >= 3)
        return 0;

    /* Only use it when Unique is close */
    if (!borg_fighting_unique)
        return 0;

    if (!borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_rage_bless_resist, true))
        return 0;

    /* Simulation */
    if (borg_simulate)
        return 2;

    /* do it! */
    if (borg_activate_item(act_resist_all)
        || borg_activate_item(act_rage_bless_resist)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 21000;
    }

    /* Value */
    return 2;
}

/* resists--- Only bother if a Unique is on the level.*/
static int borg_perma_aux_resist_p(void)
{
    int cost         = 0;
    int fail_allowed = 5;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    if (borg.temp.res_pois || !unique_on_level)
        return 0;

    if (!borg_spell_okay_fail(RESIST_POISON, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(RESIST_POISON);

    /* If its cheap, go ahead */
    if (cost >= borg.trait[BI_CURSP] / 20)
        return 0;

    /* Simulation */
    if (borg_simulate)
        return 1;

    /* do it! */
    if (borg_spell_fail(RESIST_POISON, fail_allowed)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 21000;

        /* Value */
        return 1;
    }

    /* default to can't do it. */
    return 0;
}

/*
 * Speed to prepare for battle
 */
static int borg_perma_aux_speed(void)
{
    int fail_allowed = 7;
    int cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already fast */
    if (borg.temp.fast)
        return 0;

    /* only cast defense spells if fail rate is not too high */
    if (!borg_spell_okay_fail(HASTE_SELF, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(HASTE_SELF);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    if (borg_simulate)
        return 5;

    /* do it! */
    if (borg_spell_fail(HASTE_SELF, fail_allowed)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = borg.trait[BI_CLEVEL] * 1000;
        return 5;
    }

    /* default to can't do it. */
    return 0;
}

/*
 * PFE to prepare for battle
 */
static int borg_perma_aux_prot_evil(void)
{
    int cost         = 0;
    int fail_allowed = 5;

    /* if already protected */
    if (borg.temp.prot_from_evil)
        return 0;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    if (!borg_spell_okay_fail(PROTECTION_FROM_EVIL, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(PROTECTION_FROM_EVIL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    if (borg_simulate)
        return 3;

    /* do it! */
    if (borg_spell_fail(PROTECTION_FROM_EVIL, fail_allowed)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = borg.trait[BI_CLEVEL] * 1000;

        /* Value */
        return 3;
    }

    /* default to can't do it. */
    return 0;
}

/*
 * Mana Channel to prepare for battle
 */
static int borg_perma_aux_fastcast(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already fast */
    if (borg.temp.fastcast)
        return 0;

    /* Cant when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return 0;

    if (!borg_spell_okay_fail(MANA_CHANNEL, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(MANA_CHANNEL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    /* fastcast is a low priority */
    if (borg_simulate)
        return 5;

    /* do it! */
    if (borg_spell(MANA_CHANNEL)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 6000;
        return 1;
    }

    return 0;
}

/*
 * Hero to prepare for battle
 */
static int borg_perma_aux_hero(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already heroism */
    if (borg.temp.hero)
        return 0;

    /* Can't when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return 0;

    /* Heroism part of the Heroism spell doesn't kick in till later */
    if (borg.trait[BI_CLEVEL] <= borg_heroism_level())
        return 0;

    /* XXX Dark */

    if (!borg_spell_okay_fail(HEROISM, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(HEROISM);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    /* hero is a low priority */
    if (borg_simulate)
        return 1;

    /* do it! */
    if (borg_spell(HEROISM)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 3000;
        return 1;
    }

    return 0;
}

/*
 * Rapid Regen to prepare for battle
 */
static int borg_perma_aux_regen(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already regenerating */
    if (borg.temp.regen)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* don't bother if not much to regenerate */
    if (borg.trait[BI_MAXHP] < 100)
        return 0;

    if (!borg_spell_okay_fail(RAPID_REGENERATION, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(RAPID_REGENERATION);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* do it! */
    if (borg_spell(RAPID_REGENERATION)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 6000;
        return 1;
    }

    return 0;
}

/*
 * Smite evil to prepare for battle
 */
static int borg_perma_aux_smite_evil(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already smiting */
    if (borg.temp.smite_evil || borg.trait[BI_WS_EVIL])
        return 0;

    /* Cant when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return 0;

    if (!borg_spell_okay_fail(SMITE_EVIL, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(SMITE_EVIL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    /* smite evil is a low priority */
    if (borg_simulate)
        return 3;

    /* do it! */
    if (borg_spell(SMITE_EVIL)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 21000;
        return 3;
    }

    return 0;
}

/*
 * Poison your weapon to prepare for battle
 */
static int borg_perma_aux_venom(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already smiting */
    if (borg.temp.venom || borg.trait[BI_WB_POIS])
        return 0;

    /* Cant when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return 0;

    if (!borg_spell_okay_fail(VENOM, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(SMITE_EVIL);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    /* smite evil is a low priority */
    if (borg_simulate)
        return 3;

    /* do it! */
    if (borg_spell(VENOM)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 19000;
        return 3;
    }

    return 0;
}

/*
 * Berserk to prepare for battle
 */
static int borg_perma_aux_berserk(void)
{
    int fail_allowed = 5, cost;

    /* increase the threshold */
    if (unique_on_level)
        fail_allowed = 10;
    if (borg_fighting_unique)
        fail_allowed = 15;

    /* already blessed */
    if (borg.temp.berserk)
        return 0;

    /* Cant when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return 0;

    if (!borg_spell_okay_fail(BERSERK_STRENGTH, fail_allowed))
        return 0;

    /* Obtain the cost of the spell */
    cost = borg_get_spell_power(BERSERK_STRENGTH);

    /* If its cheap, go ahead */
    if (cost >= ((unique_on_level) ? borg.trait[BI_CURSP] / 7
                                   : borg.trait[BI_CURSP] / 10))
        return 0;

    /* Simulation */
    /* Berserk is a low priority */
    if (borg_simulate)
        return 2;

    /* do it! */
    if (borg_spell(BERSERK_STRENGTH)) {
        /* No resting to recoup mana */
        borg.no_rest_prep = 11000;
        return 2;
    }

    return 0;
}

/*
 * Berserk to prepare for battle
 */
static int borg_perma_aux_berserk_potion(void)
{

    /* Saver the potions */
    if (!borg_fighting_unique)
        return 0;

    /* already blessed */
    if (borg.temp.hero || borg.temp.berserk)
        return 0;

    /* do I have any? */
    if (-1 == borg_slot(TV_POTION, sv_potion_berserk))
        return 0;

    /* Simulation */
    /* Berserk is a low priority */
    if (borg_simulate)
        return 2;

    /* do it! */
    if (borg_quaff_potion(sv_potion_berserk))
        return 2;

    return 0;
}

/*
 * Detect Inviso/Monsters
 * Casts detect invis.
 */
static int borg_perma_aux_see_inv(void)
{
    int        fail_allowed = 25;
    borg_grid *ag           = &borg_grids[borg.c.y][borg.c.x];

    /* no need */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_SINV] || borg.see_inv)
        return 0;

    /* Do I have anything that will work? */
    if (!borg_spell_okay_fail(SENSE_INVISIBLE, fail_allowed) /* &&
        !borg_spell_okay_fail(2, 6, fail_allowed) */
    )
        return 0;

    /* Darkness */
    if (!(ag->info & BORG_GLOW) && !borg.trait[BI_CURLITE])
        return 0;

    /* No real value known, but lets cast it to find the bad guys. */
    if (borg_simulate)
        return (10);

    /* long time */
    if (borg_spell_fail(SENSE_INVISIBLE, fail_allowed) /* ||
        borg_spell_fail(2, 6, fail_allowed) */
    ) {
        borg.see_inv      = 32000;
        borg.no_rest_prep = 16000;
        return (10);
    }

    /* ah crap, I guess I wont be able to see them */
    return 0;
}

/*
 * Simulate/Apply the optimal result of using the given "type" of set-up
 */
static int borg_perma_aux(int what)
{

    /* Analyze */
    switch (what) {
    case BP_SPEED: {
        return (borg_perma_aux_speed());
    }

    case BP_PROT_FROM_EVIL: {
        return (borg_perma_aux_prot_evil());
    }
    case BP_RESIST_ALL: {
        return (borg_perma_aux_resist());
    }
    case BP_RESIST_ALL_COLLUIN: {
        return (borg_perma_aux_resist_colluin());
    }
    case BP_RESIST_P: {
        return (borg_perma_aux_resist_p());
    }
    case BP_BLESS: {
        return (borg_perma_aux_bless());
    }
    case BP_FASTCAST: {
        return (borg_perma_aux_fastcast());
    }
    case BP_HERO: {
        return (borg_perma_aux_hero());
    }
    case BP_BERSERK: {
        return (borg_perma_aux_berserk());
    }
    case BP_BERSERK_POTION: {
        return (borg_perma_aux_berserk_potion());
    }
    case BP_SMITE_EVIL: {
        return (borg_perma_aux_smite_evil());
    }
    case BP_VENOM: {
        return (borg_perma_aux_venom());
    }
    case BP_REGEN: {
        return (borg_perma_aux_regen());
    }
    case BP_GLYPH: {
        /* return (borg_perma_aux_glyph()); Tends to use too much mana doing
         * this */
        return 0;
    }
    case BP_SEE_INV: {
        return (borg_perma_aux_see_inv());
    }
    }
    return 0;
}

/*
 * Walk around with certain spells on if you can afford to do so.
 */
bool borg_perma_spell(void)
{
    int n, b_n = 0;
    int g, b_g = -1;

    /* Simulate */
    borg_simulate = true;

    /* Not in town */
    if (!borg.trait[BI_CDEPTH])
        return false;

    /* Not in shallow dungeon */
    if (borg.trait[BI_CDEPTH] < borg.trait[BI_CLEVEL] / 3
        || borg.trait[BI_CDEPTH] < 7)
        return false;

    /* Low Level, save your mana, use the defense maneuvers above */
    if (borg.trait[BI_CLEVEL] <= 10)
        return false;

    /* Only when lots of mana is on hand */
    if (borg.trait[BI_CURSP] < borg.trait[BI_MAXSP] * 75 / 100)
        return false;

    /* Analyze the possible setup moves */
    for (g = 0; g < BP_MAX; g++) {
        /* Simulate */
        n = borg_perma_aux(g);

        /* Track "best" move */
        if (n <= b_n)
            continue;

        /* Track best */
        b_g = g;
        b_n = n;
    }

    /* Nothing good */
    if (b_n <= 0) {
        return false;
    }

    /* Note */
    borg_note(
        format("# Performing perma-spell type %d with value %d", b_g, b_n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_perma_aux(b_g);
    /* Success */
    return true;
}

#endif
