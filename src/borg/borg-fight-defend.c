/**
 * \file borg-fight-defend.c
 * \brief Defensive moves during a fight
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

#include "borg-fight-defend.h"

#ifdef ALLOW_BORG

#include "../player-timed.h"
#include "../ui-term.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-danger.h"
#include "borg-escape.h"
#include "borg-fight-attack.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-use.h"
#include "borg-item-val.h"
#include "borg-item.h"
#include "borg-light.h"
#include "borg-magic.h"
#include "borg-projection.h"
#include "borg-trait.h"
#include "borg.h"

bool borg_attempting_refresh_resist = false; /* for the Resistance spell */

/*
 *
 * There are several types of setup moves:
 *
 *   Temporary speed
 *   Protect From Evil
 *   Bless\Prayer
 *   Berserk\Heroism
 *   Temp Resist (either all or just some)
 *   Shield
 *   Teleport away
 *   Glyph of Warding
 *   See inviso
 *
 * * and many others
 */
enum {
    BD_BLESS,
    BD_SPEED,
    BD_GRIM_PURPOSE,
    BD_RESIST_FECAP,
    BD_RESIST_F,
    BD_RESIST_C, /* 5*/
    BD_RESIST_A,
    BD_RESIST_E,
    BD_RESIST_P,
    BD_PROT_FROM_EVIL,
    BD_SHIELD,
    BD_TELE_AWAY, /* 10 */
    BD_HERO,
    BD_BERSERK,
    BD_SMITE_EVIL,
    BD_REGEN,
    BD_GLYPH,
    BD_CREATE_DOOR,
    BD_MASS_GENOCIDE, /* 15 */
    BD_GENOCIDE,
    BD_GENOCIDE_NASTIES,
    BD_EARTHQUAKE,
    BD_DESTRUCTION,
    BD_TPORTLEVEL, /* 20 */
    BD_BANISHMENT, /* Priest spell */
    BD_DETECT_INVISO,
    BD_LIGHT_BEAM,
    BD_SHIFT_PANEL,
    BD_REST,
    BD_TELE_AWAY_MORGOTH,
    BD_BANISHMENT_MORGOTH,
    BD_LIGHT_MORGOTH,

    BD_MAX
};

/* Log the pathway and feature of the spell pathway
 * Useful for debugging beams and Tport Other spell
 */
static void borg_log_spellpath(bool beam)
{
    int n_x, n_y, x, y;

    int dist = 0;

    borg_grid *ag;
    borg_kill *kill;

    y   = borg_target_loc.y;
    x   = borg_target_loc.x;
    n_x = borg.c.x;
    n_y = borg.c.y;

    while (1) {
        ag   = &borg_grids[n_y][n_x];
        kill = &borg_kills[ag->kill];

        /* Note the Pathway */
        if (!borg_cave_floor_grid(ag)) {
            borg_note(format(
                "# Logging Spell pathway (%d,%d): Wall grid.", n_y, n_x));
            break;
        } else if (ag->kill) {
            borg_note(format("# Logging Spell pathway (%d,%d): %s, danger %d",
                n_y, n_x, (r_info[kill->r_idx].name),
                borg_danger_one_kill(
                    borg.c.y, borg.c.x, 1, ag->kill, true, false)));
        } else if (n_y == borg.c.y && n_x == borg.c.x) {
            borg_note(
                format("# Logging Spell pathway (%d,%d): My grid.", n_y, n_x));
        } else {
            borg_note(format("# Logging Spell pathway (%d,%d).", n_y, n_x));
        }

        /* Stop loop if we reach our target if using bolt */
        if (n_x == x && n_y == y)
            break;

        /* Safegaurd not to loop */
        dist++;
        if (dist >= z_info->max_range)
            break;

        /* Calculate the new location */
        borg_inc_motion(&n_y, &n_x, borg.c.y, borg.c.x, y, x);
    }
}

/*
 * Bless/Prayer to prepare for battle
 */
static int borg_defend_aux_bless(int p1)
{
    int        fail_allowed = 25;
    borg_grid *ag           = &borg_grids[borg.c.y][borg.c.x];

    int i;

    bool borg_near_kill = false;

    /* already blessed */
    if (borg.temp.bless)
        return 0;

    /* Cant when Blind */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Dark */
    if (!(ag->info & BORG_GLOW) && borg.trait[BI_CURLITE] == 0)
        return 0;

    /* no spell */
    if (!borg_spell_okay_fail(BLESS, fail_allowed)
        && !borg_equips_item(act_blessing, true)
        && !borg_equips_item(act_blessing2, true)
        && !borg_equips_item(act_blessing3, true)
        && -1 == borg_slot(TV_SCROLL, sv_scroll_blessing)
        && -1 == borg_slot(TV_SCROLL, sv_scroll_holy_chant)
        && -1 == borg_slot(TV_SCROLL, sv_scroll_holy_prayer))
        return 0;

    /* Check if a monster is close to me .
     */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 5)
            continue;

        /* Check the distance XXX XXX XXX */
        if (distance(borg.c, kill->pos) > 3)
            continue;

        /* kill near me */
        borg_near_kill = true;
    }

    /* if we are in some danger but not much, go for a quick bless */
    if ((p1 > avoidance / 12 || borg.trait[BI_CLEVEL] <= 15) && p1 > 0
        && borg_near_kill && p1 < avoidance / 2) {
        /* Simulation */
        /* bless is a low priority */
        if (borg_simulate)
            return 1;

        borg_note("# Attempting to cast Bless");

        /* No resting to recoup mana */
        borg.no_rest_prep = 11000;

        /* do it! */
        if (borg_spell(BLESS) || borg_activate_item(act_blessing)
            || borg_activate_item(act_blessing2)
            || borg_activate_item(act_blessing3)
            || borg_read_scroll(sv_scroll_blessing)
            || borg_read_scroll(sv_scroll_holy_chant)
            || borg_read_scroll(sv_scroll_holy_prayer))
            return 1;
    }

    return 0;
}

/*
 * Speed to prepare for battle
 */
static int borg_defend_aux_speed(int p1)
{
    int  p2           = 0;
    bool good_speed   = false;
    bool speed_spell  = false;
    bool speed_staff  = false;
    bool speed_rod    = false;
    int  fail_allowed = 25;

    /* already fast */
    if (borg.temp.fast)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    /* only cast defense spells if fail rate is not too high */
    if (borg_spell_okay_fail(HASTE_SELF, fail_allowed))
        speed_spell = true;

    /* staff must have charges */
    if (borg_equips_staff_fail(sv_staff_speed))
        speed_staff = true;

    /* rod can't be charging */
    if (borg_equips_rod(sv_rod_speed))
        speed_rod = true;

    /* Need some form */
    if (0 > borg_slot(TV_POTION, sv_potion_speed) && !speed_staff && !speed_rod
        && !speed_spell && !borg_equips_item(act_haste, true)
        && !borg_equips_item(act_haste1, true)
        && !borg_equips_item(act_haste2, true))
        return 0;

    /* if we have an infinite/large suppy of speed we can */
    /* be generous with our use */
    if (speed_rod || speed_spell || speed_staff
        || borg_equips_item(act_haste, true)
        || borg_equips_item(act_haste1, true)
        || borg_equips_item(act_haste2, true))
        good_speed = true;

    /* pretend we are protected and look again */
    borg.temp.fast = true;
    p2             = borg_danger(borg.c.y, borg.c.x, 1, true, false);
    borg.temp.fast = false;

    /* if scaryguy around cast it. */
    if (scaryguy_on_level) {
        /* HACK pretend that it was scary and will be safer */
        p2 = p2 * 3 / 10;
    }

    /* if we are fighting a unique cast it. */
    if (good_speed && borg_fighting_unique) {
        /* HACK pretend that it was scary and will be safer */
        p2 = p2 * 7 / 10;
    }
    /* if we are fighting a unique and a summoner cast it. */
    if (borg_fighting_summoner && borg_fighting_unique) {
        /* HACK pretend that it was scary and will be safer */
        p2 = p2 * 7 / 10;
    }
    /* if the unique is Sauron cast it */
    if (borg.trait[BI_CDEPTH] == 99 && borg_fighting_unique >= 10) {
        p2 = p2 * 6 / 10;
    }

    /* if the unique is a rather nasty one. */
    if (borg_fighting_unique
        && (streq(r_info[unique_on_level].name, "Bullroarer the Hobbit")
            || streq(r_info[unique_on_level].name, "Mughash the Kobold Lord")
            || streq(
                r_info[unique_on_level].name, "Wormtongue, Agent of Saruman")
            || streq(r_info[unique_on_level].name, "Lagduf, the Snaga")
            || streq(r_info[unique_on_level].name, "Brodda, the Easterling")
            || streq(r_info[unique_on_level].name, "Orfax, Son of Boldor"))) {
        p2 = p2 * 6 / 10;
    }

    /* if the unique is Morgoth cast it */
    if (borg.trait[BI_CDEPTH] == 100 && borg_fighting_unique >= 10) {
        p2 = p2 * 5 / 10;
    }

    /* Attempt to conserve Speed at end of game */
    if (borg.trait[BI_CDEPTH] >= 97 && !borg_fighting_unique && !good_speed)
        p2 = 9999;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (((p1 > p2)
            && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                           : (avoidance / 2))
            && (p1 > (avoidance / 5)) && good_speed)
        || ((p1 > p2)
            && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                           : (avoidance / 3))
            && (p1 > (avoidance / 7)))) {

        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast Speed");

        /* No resting to recoup mana */
        borg.no_rest_prep = borg.trait[BI_CLEVEL] * 1000;

        /* do it! */
        if (borg_zap_rod(sv_rod_speed) || borg_activate_item(act_haste)
            || borg_activate_item(act_haste1) || borg_activate_item(act_haste2)
            || borg_use_staff(sv_staff_speed)
            || borg_quaff_potion(sv_potion_speed))
            /* Value */
            return (p1 - p2);

        if (borg_spell_fail(HASTE_SELF, fail_allowed))
            return (p1 - p2);
    }
    /* default to can't do it. */
    return 0;
}

/* Grim Purpose */
static int borg_defend_aux_grim_purpose(int p1)
{
    int p2           = 0;
    int fail_allowed = 25;

    bool save_conf   = borg.trait[BI_RCONF];
    bool save_fa     = borg.trait[BI_FRACT];

    /* already protected */
    if (save_conf && save_fa)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(GRIM_PURPOSE, fail_allowed))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1 = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    /* pretend we are protected and look again */
    borg.trait[BI_RCONF] = true;
    borg.trait[BI_FRACT] = true;
    p2                   = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.trait[BI_RCONF] = save_conf;
    borg.trait[BI_FRACT] = save_fa;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {

        /* Simulation */
        if (borg_simulate)
            return (p1 - p2 + 2);

        borg_note("# Attempting to cast Grim Purpose");

        /* do it! */
        if (borg_spell(GRIM_PURPOSE))
            /* No resting to recoup mana */
            borg.no_rest_prep = 13000;

        /* Value */
        return (p1 - p2 + 2);
    }

    /* default to can't do it. */
    return 0;
}

/* all resists */
static int borg_defend_aux_resist_fecap(int p1)
{
    int  p2        = 0;
    bool save_fire = false, save_acid = false, save_poison = false,
         save_elec = false, save_cold = false;

    if (borg.temp.res_fire && borg.temp.res_acid && borg.temp.res_pois
        && borg.temp.res_elec && borg.temp.res_cold)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    if (!borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_rage_bless_resist, true))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1 = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    /* pretend we are protected and look again */
    save_fire          = borg.temp.res_fire;
    save_elec          = borg.temp.res_elec;
    save_cold          = borg.temp.res_cold;
    save_acid          = borg.temp.res_acid;
    save_poison        = borg.temp.res_pois;
    borg.temp.res_fire = true;
    borg.temp.res_elec = true;
    borg.temp.res_cold = true;
    borg.temp.res_acid = true;
    borg.temp.res_pois = true;
    p2                 = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.res_fire = save_fire;
    borg.temp.res_elec = save_elec;
    borg.temp.res_cold = save_cold;
    borg.temp.res_acid = save_acid;
    borg.temp.res_pois = save_poison;

    /* Hack -
     * If the borg is fighting a particular unique enhance the
     * benefit of the spell.
     */
    if (borg_fighting_unique
        && (streq(r_info[unique_on_level].name, "The Tarrasque")))
        p2 = p2 * 8 / 10;

    /* Hack -
     * If borg is high enough level, he does not need to worry
     * about mana consumption.  Cast the good spell.
     */
    if (borg.trait[BI_CLEVEL] >= 45)
        p2 = p2 * 8 / 10;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {

        /* Simulation */
        if (borg_simulate)
            return (p1 - p2 + 2);

        borg_note("# Attempting to cast FECAP");

        /* do it! */
        if (borg_activate_item(act_resist_all)
            || borg_activate_item(act_rage_bless_resist))

            /* No resting to recoup mana */
            borg.no_rest_prep = 21000;

        /* Value */
        return (p1 - p2 + 2);
    }

    /* default to can't do it. */
    return 0;
}

/* fire */
static int borg_defend_aux_resist_f(int p1)
{

    int  p2           = 0;
    int  fail_allowed = 25;
    bool save_fire    = false;

    save_fire         = borg.temp.res_fire;

    if (borg.temp.res_fire)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed)
        && !borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_resist_fire, true)
        && !borg_equips_item(act_rage_bless_resist, true)
        && !borg_equips_ring(sv_ring_flames)
        && !borg_equips_item(act_ring_flames, true)
        && -1 == borg_slot(TV_POTION, sv_potion_resist_heat))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1 = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    /* pretend we are protected and look again */
    borg.temp.res_fire = true;
    p2                 = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.res_fire = save_fire;

    /* Hack -
     * If the borg is fighting a particular unique enhance the
     * benefit of the spell.
     */
    if (borg_fighting_unique
        && (streq(r_info[unique_on_level].name, "The Tarrasque")))
        p2 = p2 * 8 / 10;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast RFire");
        /* do it! */
        if (borg_activate_ring(sv_ring_flames)
            || borg_activate_item(act_ring_flames)) {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }
        if (borg_activate_item(act_resist_all)
            || borg_activate_item(act_resist_fire)
            || borg_activate_item(act_rage_bless_resist)
            || borg_spell_fail(RESISTANCE, fail_allowed)
            || borg_quaff_potion(sv_potion_resist_heat))

            /* No resting to recoup mana */
            borg.no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return 0;
}

/* cold */
static int borg_defend_aux_resist_c(int p1)
{
    int  p2           = 0;
    int  fail_allowed = 25;
    bool save_cold    = false;

    if (borg.temp.res_cold)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed)
        && !borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_rage_bless_resist, true)
        && !borg_equips_item(act_resist_cold, true)
        && !borg_equips_ring(sv_ring_ice)
        && !borg_equips_item(act_ring_ice, true)
        && -1 == borg_slot(TV_POTION, sv_potion_resist_cold))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1        = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    save_cold = borg.temp.res_cold;
    /* pretend we are protected and look again */
    borg.temp.res_cold = true;
    p2                 = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.res_cold = save_cold;

    /* Hack -
     * If the borg is fighting a particular unique enhance the
     * benefit of the spell.
     */
    if (borg_fighting_unique
        && (streq(r_info[unique_on_level].name, "The Tarrasque")))
        p2 = p2 * 8 / 10;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast RCold");

        /* do it! */
        if (borg_activate_ring(sv_ring_ice)
            || borg_activate_item(act_ring_ice)) {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }
        if (borg_activate_item(act_resist_all)
            || borg_activate_item(act_rage_bless_resist)
            || borg_activate_item(act_resist_cold)
            || borg_spell_fail(RESISTANCE, fail_allowed)
            || borg_quaff_potion(sv_potion_resist_cold))

            /* No resting to recoup mana */
            borg.no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return 0;
}

/* acid */
static int borg_defend_aux_resist_a(int p1)
{
    int  p2           = 0;
    int  fail_allowed = 25;
    bool save_acid    = false;

    if (borg.temp.res_acid)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed)
        && !borg_equips_item(act_resist_acid, true)
        && !borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_rage_bless_resist, true)
        && !borg_equips_ring(sv_ring_acid))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1        = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    save_acid = borg.temp.res_acid;
    /* pretend we are protected and look again */
    borg.temp.res_acid = true;
    p2                 = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.res_acid = save_acid;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast RAcid");

        /* do it! */
        if (borg_spell(RESISTANCE)) {
            return (p1 - p2);
        }

        if (borg_activate_ring(sv_ring_acid)
            || borg_activate_item(act_ring_acid)) {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }

        if (borg_activate_item(act_resist_acid)
            || borg_activate_item(act_resist_all)
            || borg_activate_item(act_rage_bless_resist))

            /* No resting to recoup mana */
            borg.no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }
    /* default to can't do it. */
    return 0;
}

/* electricity */
static int borg_defend_aux_resist_e(int p1)
{
    int  p2           = 0;
    int  fail_allowed = 25;
    bool save_elec    = false;

    if (borg.temp.res_elec)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESISTANCE, fail_allowed)
        && !borg_equips_item(act_resist_elec, true)
        && !borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_rage_bless_resist, true)
        && !borg_equips_ring(sv_ring_lightning)
        && !borg_equips_item(act_ring_lightning, true))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1        = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    save_elec = borg.temp.res_elec;
    /* pretend we are protected and look again */
    borg.temp.res_elec = true;
    p2                 = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.res_elec = save_elec;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast RAcid");

        /* do it! */
        if (borg_spell(RESISTANCE)) {
            return (p1 - p2);
        }

        if (borg_activate_ring(sv_ring_lightning)
            || borg_activate_item(act_ring_lightning)) {
            /* Ring also attacks so target self */
            borg_keypress('*');
            borg_keypress('5');
            return (p1 - p2);
        }

        if (borg_activate_item(act_resist_elec)
            || borg_activate_item(act_resist_all)
            || borg_activate_item(act_rage_bless_resist))

            /* No resting to recoup mana */
            borg.no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }
    /* default to can't do it. */
    return 0;
}

/* poison */
static int borg_defend_aux_resist_p(int p1)
{
    int  p2           = 0;
    int  fail_allowed = 25;
    bool save_poison  = false;

    if (borg.temp.res_pois)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (!borg_spell_okay_fail(RESIST_POISON, fail_allowed)
        && !borg_equips_item(act_resist_pois, true)
        && !borg_equips_item(act_resist_all, true)
        && !borg_equips_item(act_rage_bless_resist, true)
        && -1 == borg_slot(TV_POTION, sv_potion_resist_pois))
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1          = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    save_poison = borg.temp.res_pois;
    /* pretend we are protected and look again */
    borg.temp.res_pois = true;
    p2                 = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.res_pois = save_poison;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast RPois");

        /* do it! */
        if (borg_spell_fail(RESIST_POISON, fail_allowed)
            || borg_activate_item(act_resist_pois)
            || borg_activate_item(act_resist_all)
            || borg_activate_item(act_rage_bless_resist)
            || borg_quaff_potion(sv_potion_resist_pois))

            /* No resting to recoup mana */
            borg.no_rest_prep = 21000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return 0;
}

/* pfe */
static int borg_defend_aux_prot_evil(int p1)
{
    int        p2           = 0;
    int        fail_allowed = 25;
    bool       pfe_spell    = false;
    borg_grid *ag           = &borg_grids[borg.c.y][borg.c.x];

    /* if already protected */
    if (borg.temp.prot_from_evil)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 5;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    if (borg_spell_okay_fail(PROTECTION_FROM_EVIL, fail_allowed))
        pfe_spell = true;

    if (0 <= borg_slot(TV_SCROLL, sv_scroll_protection_from_evil))
        pfe_spell = true;

    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISIMAGE])
        pfe_spell = false;

    if (!(ag->info & BORG_GLOW) && borg.trait[BI_CURLITE] == 0)
        pfe_spell = false;

    if (borg_equips_item(act_protevil, true))
        pfe_spell = true;

    if (pfe_spell == false)
        return 0;

    /* elemental and PFE use the 'averaging' method for danger.  Redefine p1 as
     * such. */
    p1 = borg_danger(borg.c.y, borg.c.x, 1, false, false);

    /* pretend we are protected and look again */
    borg.temp.prot_from_evil = true;
    p2                       = borg_danger(borg.c.y, borg.c.x, 1, false, false);
    borg.temp.prot_from_evil = false;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */

    if ((p1 > p2
            && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                           : (avoidance / 2))
            && p1 > (avoidance / 7))
        || (borg_cfg[BORG_MONEY_SCUM_AMOUNT] >= 1
            && borg.trait[BI_CDEPTH] == 0)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to cast PFE");

        /* do it! */
        if (borg_spell_fail(PROTECTION_FROM_EVIL, fail_allowed)
            || borg_activate_item(act_protevil)
            || borg_read_scroll(sv_scroll_protection_from_evil))

            /* No resting to recoup mana */
            borg.no_rest_prep = borg.trait[BI_CLEVEL] * 1000;

        /* Value */
        return (p1 - p2);
    }

    /* default to can't do it. */
    return 0;
}

/* shield */
static int borg_defend_aux_shield(int p1)
{
    int p2 = 0;

    /* if already protected */
    if (borg.temp.shield)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    if (borg.has[kv_mush_stoneskin] <= 0
        && !borg_equips_item(act_shroom_stone, true))
        return 0;

    /* pretend we are protected and look again */
    borg.temp.shield = true;
    p2               = borg_danger(borg.c.y, borg.c.x, 1, true, false);
    borg.temp.shield = false;

    /* slightly enhance the value if fighting a unique */
    if (borg_fighting_unique)
        p2 = (p2 * 7 / 10);

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        borg_note("# Attempting to eat a stone skin");

        /* do it! */
        if (borg_eat(TV_MUSHROOM, sv_mush_stoneskin)
            || borg_activate_item(act_shroom_stone)) {
            /* No resting to recoup mana */
            borg.no_rest_prep = 2000;
            return (p1 - p2);
        }
    }

    /* default to can't do it. */
    return 0;
}

/*
 * Try to get rid of all of the non-uniques around so you can go at it
 * 'mano-e-mano' with the unique. Teleport Other.
 */
static int borg_defend_aux_tele_away(int p1)
{
    int  p2           = p1;
    int  fail_allowed = 50;
    bool spell_ok     = false;
    int  i;

    borg_grid *ag;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /*
     * Only tport monster away if scared or getting low on mana
     */
    if (borg_fighting_unique) {
        if (p1 < avoidance * 7 / 10 && borg.trait[BI_CURSP] > 30
            && borg_simulate)
            return 0;
    } else {
        if (p1 < avoidance * 5 / 10 && borg.trait[BI_CURSP] > 30
            && borg_simulate)
            return 0;
    }

    /* No real Danger to speak of */
    if (p1 < avoidance * 4 / 10 && borg_simulate)
        return 0;

    spell_ok = false;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance * 3)
        fail_allowed -= 10;
    else
        /* scary */
        if (p1 > avoidance * 2)
            fail_allowed -= 5;
        else
            /* a little scary */
            if (p1 > (avoidance * 5) / 2)
                fail_allowed += 5;

    /* do I have the ability? */
    if (borg_spell_okay_fail(TELEPORT_OTHER, fail_allowed)
        || borg_equips_item(act_tele_other, true)
        || (-1 != borg_slot(TV_WAND, sv_wand_teleport_away)
            && borg_items[borg_slot(TV_WAND, sv_wand_teleport_away)].pval))
        spell_ok = true;

    if (!spell_ok)
        return 0;

    /* No Teleport Other if surrounded */
    if (borg_surrounded() == true)
        return 0;

    /* Borg_temp_n temporarily stores several things.
     * Some of the borg_attack() sub-routines use these numbers,
     * which would have been filled in borg_attack().
     * Since this is a defense maneuver which will move into
     * and borrow some of the borg_attack() subroutines, we need
     * to make sure that the borg_temp_n arrays are properly
     * filled.  Otherwise, the borg will attempt to consider
     * these grids which were left filled by some other routine.
     * Which was probably a flow routine which stored about 200
     * grids into the array.
     * Any change in inclusion/exclusion criteria for filling this
     * array in borg_attack() should be included here also.
     */
    /* Nobody around so don't worry */
    if (!borg_kills_cnt && borg_simulate)
        return 0;

    /* Reset list */
    borg_temp_n     = 0;
    borg_tp_other_n = 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Get grid */
        ag = &borg_grids[kill->pos.y][kill->pos.x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY))
            continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW))
            continue;
        if ((ag->feat >= FEAT_RUBBLE) && (ag->feat <= FEAT_PERM))
            continue;

        /* Check the distance XXX XXX XXX */
        if (distance(borg.c, kill->pos) > z_info->max_range)
            continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = kill->pos.x;
        borg_temp_y[borg_temp_n] = kill->pos.y;
        borg_temp_n++;
    }

    /* No targets for me. */
    if (!borg_temp_n && borg_simulate)
        return 0;

    /* choose, then target a bad guy.
     * Damage will be the danger to my grid which the monster creates.
     * We are targeting the single most dangerous monster.
     * p2 will be the original danger (p1) minus the danger from the most
     * dangerous monster eliminated. ie:  if we are fighting only a single
     * monster who is generating 500 danger and we target him, then p2 _should_
     * end up 0, since p1 - his danger is 500-500. If we are fighting two guys
     * each creating 500 danger, then p2 will be 500, since 1000-500 = 500.
     */
    p2 = p1
         - borg_launch_bolt(-1, p1, BORG_ATTACK_AWAY_ALL, z_info->max_range, 0);

    /* check to see if I am left better off */
    if (borg_simulate) {
        /* Reset list */
        borg_temp_n     = 0;
        borg_tp_other_n = 0;

        if (p1 > p2 && p2 < avoidance / 2) {
            /* Simulation */
            return (p1 - p2);
        } else
            return 0;
    }

    /* Log the Path for Debug */
    borg_log_spellpath(true);

    /* Log additional info for debug */
    for (i = 0; i < borg_tp_other_n; i++) {
        borg_note(format("# T.O. %d, index %d (%d,%d)", borg_tp_other_n,
            borg_tp_other_index[i], borg_tp_other_y[i], borg_tp_other_x[i]));
    }

    /* Reset list */
    borg_temp_n     = 0;
    borg_tp_other_n = 0;

    /* Cast the spell */
    if (borg_spell(TELEPORT_OTHER) || borg_activate_item(act_tele_other)
        || borg_aim_wand(sv_wand_teleport_away)) {
        /* Use target */
        borg_keypress('5');

        /* Set our shooting flag */
        successful_target = -1;

        /* Value */
        return (p2);
    }

    return 0;
}

/*
 * Hero to prepare for battle, +12 tohit.
 */
static int borg_defend_aux_hero(int p1)
{
    int  fail_allowed = 15;
    bool potion, spell;

    /* already hero */
    if (borg.temp.hero)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Heroism part of the heroism spell only kicks in after level 19 */
    spell = borg_spell_okay_fail(HEROISM, fail_allowed)
            && borg.trait[BI_CLEVEL] >= borg_heroism_level();
    potion = (-1 != borg_slot(TV_POTION, sv_potion_heroism));
    if (!potion && !spell)
        return 0;

    /* if we are in some danger but not much, go for a quick bless */
    /* "some danger" defined as "10% of x and not more than 50% of x */
    /* (not more than 70% when fighting a unique) */
    /* where x is the danger we are avoiding, usually current hp */
    if (p1 > avoidance / 10 && 
        p1 < (avoidance * (borg_fighting_unique ? 7 : 5)) / 10) {
        /* Simulation */
        /* hero is a low priority */
        if (borg_simulate)
            return 1;

        borg_note("# Attempting to cast Hero");

        /* do it! */
        if ((spell && borg_spell(HEROISM))
            || borg_quaff_potion(sv_potion_heroism)) {
            /* No resting to recoup mana */
            borg.no_rest_prep = 10000;
            return 1;
        }
    }

    return 0;
}

/*
 * Rapid Regen to prepare for battle
 */
static int borg_defend_aux_regen(int p1)
{
    int fail_allowed = 15;

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

    /* if we are in some danger but not much, go for a quick regen */
    /* "some danger" defined as "10% of x and not more than 50% of x */
    /* (not more than 70% when fighting a unique) */
    /* where x is the danger we are avoiding, usually current hp */
    if (p1 > avoidance / 10 &&
        p1 < (avoidance * (borg_fighting_unique ? 7 : 5)) / 10) {
        /* Simulation */
        /* regen is a low priority */
        if (borg_simulate)
            return 1;

        /* do it! */
        if (borg_spell(RAPID_REGENERATION)) {
            /* No resting to recoup mana */
            borg.no_rest_prep = 10000;
            return 1;
        }
    }

    return 0;
}

/*
 * Berserk to prepare for battle, +24 tohit, -10 AC
 */
static int borg_defend_aux_berserk(int p1)
{
    int fail_allowed = 15;

    /* already berserk */
    if (borg.temp.berserk)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    if (!borg_spell_okay_fail(BERSERK_STRENGTH, fail_allowed)
        && -1 == borg_slot(TV_POTION, sv_potion_berserk)
        && !borg_equips_item(act_berserker, true)
        && !borg_equips_item(act_rage_bless_resist, true)
        && !borg_equips_item(act_shero, true))
        return 0;

    /* if we are in some danger but not much, go for a quick bless */
    /* "some danger" defined as "10% of x and not more than 50% of x */
    /* (not more than 70% when fighting a unique) */
    /* where x is the danger we are avoiding, usually current hp */
    if (p1 > avoidance / 10 &&
        p1 < (avoidance * (borg_fighting_unique ? 7 : 5)) / 10) {
        /* Simulation */
        /* berserk is a low priority */
        if (borg_simulate)
            return 5;

        /* do it! */
        if (borg_spell(BERSERK_STRENGTH) || borg_activate_item(act_berserker)
            || borg_activate_item(act_rage_bless_resist)
            || borg_activate_item(act_shero)
            || borg_quaff_potion(sv_potion_berserk))
            return 5;
    }

    return 0;
}

/* 
 * See if the borg is near something evil 
 */
static bool near_evil(void)
{
    int        i;
    borg_grid *ag;
    borg_kill *kill;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Get grid */
        ag = &borg_grids[kill->pos.y][kill->pos.x];

        /* don't count off-screen */
        if (!(ag->info & BORG_OKAY))
            continue;

        /* Check the distance XXX XXX XXX */
        if (distance(borg.c, kill->pos) > 3)
            continue;

        /* Monster race is evil */
        if (rf_has((r_info[kill->r_idx].flags), RF_EVIL))
            return true;
    }

    return false;
}

/*
 * Smite Evil to prepare for battle
 */
static int borg_defend_aux_smite_evil(int p1)
{
    int fail_allowed = 15;

    /* already smiting evil */
    if (borg.temp.smite_evil || borg.trait[BI_WS_EVIL])
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    if (!borg_spell_okay_fail(SMITE_EVIL, fail_allowed))
        return 0;

    // if the borg is not about to fight something evil.
    if (!near_evil())
        return 0;

    /* if we are in some danger but not much, go for a quick smite */
    /* "some danger" defined as "10% of x and not more than 50% of x */
    /* (not more than 70% when fighting a unique) */
    /* where x is the danger we are avoiding, usually current hp */
    if (p1 > avoidance / 10 &&
        p1 < (avoidance * (borg_fighting_unique ? 7 : 5)) / 10) {

        /* Simulation */
        /* smite evil is a low priority */
        if (borg_simulate)
            return 5;

        /* do it! */
        if (borg_spell(SMITE_EVIL))
            return 5;
    }

    return 0;
}

/* Glyph of Warding and Rune of Protection */
static int borg_defend_aux_glyph(int p1)
{
    int  p2           = 0, i;
    int  fail_allowed = 25;
    bool glyph_spell  = false;

    borg_grid *ag     = &borg_grids[borg.c.y][borg.c.x];

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* He should not cast it while on an object.
     * I have addressed this inadequately in borg9.c when dealing with
     * messages.  The message "the object resists" will delete the glyph
     * from the array.  Then I set a broken door on that spot, the borg ignores
     * broken doors, so he won't loop.
     */

    if ((ag->take) || (ag->trap) || (ag->feat == FEAT_LESS)
        || (ag->feat == FEAT_MORE) || (ag->feat == FEAT_OPEN)
        || (ag->feat == FEAT_BROKEN)) {
        return 0;
    }

    /* Morgoth breaks these in one try so its a waste of mana against him */
    if (borg_fighting_unique >= 10)
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 5;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 20;

    if (borg_spell_okay_fail(GLYPH_OF_WARDING, fail_allowed))
        glyph_spell = true;

    if (0 <= borg_slot(TV_SCROLL, sv_scroll_rune_of_protection))
        glyph_spell = true;

    if (borg_equips_item(act_glyph, true))
        glyph_spell = true;

    if ((borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
            || borg.trait[BI_ISIMAGE])
        && glyph_spell)
        glyph_spell = false;

    if (!(ag->info & BORG_GLOW) && borg.trait[BI_CURLITE] == 0)
        glyph_spell = false;

    if (!glyph_spell)
        return 0;

    /* pretend we are protected and look again */
    borg_on_glyph = true;
    p2            = borg_danger(borg.c.y, borg.c.x, 1, true, false);
    borg_on_glyph = false;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        /* do it! */
        if (borg_spell_fail(GLYPH_OF_WARDING, fail_allowed)
            || borg_read_scroll(sv_scroll_rune_of_protection)
            || borg_activate_item(act_glyph)) {
            /* Check for an existing glyph */
            for (i = 0; i < track_glyph.num; i++) {
                /* Stop if we already new about this glyph */
                if ((track_glyph.x[i] == borg.c.x)
                    && (track_glyph.y[i] == borg.c.y))
                    return (p1 - p2);
            }

            /* Track the newly discovered glyph */
            if (track_glyph.num < track_glyph.size) {
                borg_note("# Noting the creation of a glyph.");
                track_glyph.x[track_glyph.num] = borg.c.x;
                track_glyph.y[track_glyph.num] = borg.c.y;
                track_glyph.num++;
            }
            return (p1 - p2);
        }
    }

    /* default to can't do it. */
    return 0;
}

/* Create Door */
static int borg_defend_aux_create_door(int p1)
{
    int p2           = 0;
    int fail_allowed = 30;
    int door_bad     = 0;
    int door_x = 0, door_y = 0, x = 0, y = 0;

    borg_grid *ag;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* any summoners near?*/
    if (!borg_fighting_summoner)
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 5;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 20;

    if (!borg_spell_okay_fail(DOOR_CREATION, fail_allowed))
        return 0;

    /* Do not cast if surounded by doors or something */
    /* Get grid */
    for (door_x = -1; door_x <= 1; door_x++) {
        for (door_y = -1; door_y <= 1; door_y++) {
            /* Acquire location */
            x  = door_x + borg.c.x;
            y  = door_y + borg.c.y;

            ag = &borg_grids[y][x];

            /* track spaces already protected */
            if ((ag->glyph) || ag->kill
                || ((ag->feat == FEAT_GRANITE) || (ag->feat == FEAT_PERM)
                    || (ag->feat == FEAT_CLOSED))) {
                door_bad++;
            }

            /* track spaces that cannot be protected */
            if ((ag->take) || (ag->trap) || (ag->feat == FEAT_LESS)
                || (ag->feat == FEAT_MORE) || (ag->feat == FEAT_OPEN)
                || (ag->feat == FEAT_BROKEN) || (ag->kill)) {
                door_bad++;
            }
        }
    }

    /* Track it */
    /* lets make sure that we going to be benifited */
    if (door_bad >= 6) {
        /* not really worth it.  Only 2 spaces protected */
        return 0;
    }

    /* pretend we are protected and look again */
    borg_create_door = true;
    p2               = borg_danger(borg.c.y, borg.c.x, 1, true, false);
    borg_create_door = false;

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 7)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        /* do it! */
        if (borg_spell_fail(DOOR_CREATION, fail_allowed)) {
            /* Set the breeder flag to keep doors closed. Avoid summons */
            breeder_level = true;

            /* Must make a new Sea too */
            borg_needs_new_sea = true;

            /* Value */
            return (p1 - p2);
        }
    }

    /* default to can't do it. */
    return 0;
}

/* This will simulate and cast the mass genocide spell.
 */
static int borg_defend_aux_mass_genocide(int p1)
{
    int hit = 0, i = 0, p2;
    int b_p = 0, p;

    borg_kill           *kill;
    struct monster_race *r_ptr;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* see if prayer is legal */
    if (!borg_spell_okay_fail(MASS_BANISHMENT, 40)
        && !borg_equips_item(act_banishment, true)
        && (borg.trait[BI_AMASSBAN] == 0)) /* Mass Banishment scroll */
        return 0;

    /* See if he is in real danger */
    if (p1 < avoidance * 12 / 10 && borg_simulate)
        return 0;

    /* Find a monster and calculate its danger */
    for (i = 1; i < borg_kills_nxt; i++) {

        /* Monster */
        kill  = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Check the distance */
        if (distance(borg.c, kill->pos) > 20)
            continue;

        /* we try not to genocide uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            continue;

        /* Calculate danger */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);

        /* store the danger for this type of monster */
        b_p = b_p + p;
        hit = hit + 3;
    }

    /* normalize the value */
    p2 = (p1 - b_p);
    if (p2 < 0)
        p2 = 0;

    /* if strain (plus a pad incase we did not know about some monsters)
     * is greater than hp, don't cast it
     */
    if ((hit * 12 / 10) >= borg.trait[BI_CURHP])
        return 0;

    /* Penalize the strain from casting the spell */
    p2 = p2 + hit;

    /* Be more likely to use this if fighting Morgoth */
    if (borg_fighting_unique >= 10 && (hit / 3 > 8)) {
        p2 = p2 * 6 / 10;
    }

    /* if this is an improvement and we may not avoid monster now and */
    /* we may have before */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? (avoidance * 2 / 3)
                                       : (avoidance / 2))) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);

        /* Cast the spell */
        if (borg_read_scroll(sv_scroll_mass_banishment)
            || borg_activate_item(act_banishment)
            || borg_spell(MASS_BANISHMENT)) {

            /* Remove monsters from the borg_kill */
            for (i = 1; i < borg_kills_nxt; i++) {
                borg_kill           *tmp_kill;
                struct monster_race *tmp_r_ptr;

                /* Monster */
                tmp_kill  = &borg_kills[i];
                tmp_r_ptr = &r_info[tmp_kill->r_idx];

                /* Cant kill uniques like this */
                if (rf_has(tmp_r_ptr->flags, RF_UNIQUE))
                    continue;

                /* remove this monster */
                borg_delete_kill(i);
            }

            /* Value */
            return (p1 - p2);
        }
    }
    /* Not worth it */
    return 0;
}

/* This will simulate and cast the genocide spell.
 * There are two seperate functions happening here.
 * 1. will genocide the race which is immediately threatening the borg.
 * 2. will genocide the race which is most dangerous on the level.  Though it
 * may not be threatening the borg right now.  It was considered to nuke the
 * escorts of a unique. But it could also be used to nuke a race if it becomes
 * too dangerous, for example a summoner called up 15-20 hounds, and they must
 * be dealt with. The first option may be called at any time.  While the 2nd
 * option is only called when the borg is in relatively good health.
 */
static int borg_defend_aux_genocide(int p1)
{
    int i;

    /* current power of kill */
    int p = 0;
    /* power of kill to level */
    int p_threat = 0;
    /* power of danger without this kill */
    int p_without_kill = 0;
    int max_threat;

    /* character of the kill */
    unsigned char kill_char;

    /* arrays based on kill's char */
    int b_kill_count[UCHAR_MAX];
    /* current danger to the borg */
    int b_danger[UCHAR_MAX];
    /* danger in general */
    int b_threat[UCHAR_MAX];

    borg_kill           *kill;
    struct monster_race *r_ptr;

    int total_danger_to_me            = 0;

    char          tmp_genocide_target = (char)0;
    unsigned char biggest_threat;
    unsigned char biggest_danger;

    bool genocide_spell = false;
    int  fail_allowed   = 25;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance)
        fail_allowed -= 19;
    else
        /* a little scary */
        if (p1 > (avoidance * 2) / 3)
            fail_allowed -= 10;
        else
            /* not very scary, allow lots of fail */
            if (p1 < avoidance / 3)
                fail_allowed += 10;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Normalize the p1 value.  It contains danger added from
     * regional fear and monster fear.  Which wont be counted
     * in the post-genocide checks
     */
    if (borg_fear_region[borg.c.y / 11][borg.c.x / 11])
        p1 -= borg_fear_region[borg.c.y / 11][borg.c.x / 11];
    if (borg_fear_monsters[borg.c.y][borg.c.x])
        p1 -= borg_fear_monsters[borg.c.y][borg.c.x];

    /* Make sure I have the spell */
    if (borg_spell_okay_fail(BANISHMENT, fail_allowed)
        || borg_equips_item(act_banishment, true)
        || borg_equips_staff_fail(sv_staff_banishment)
        || (-1 != borg_slot(TV_SCROLL, sv_scroll_banishment))) {
        genocide_spell = true;
    }

    if (genocide_spell == false)
        return 0;

    /* Don't try it if really weak */
    if (borg.trait[BI_CURHP] <= 75)
        return 0;

    /* two methods to calculate the threat:
     * 1. cycle each character of monsters on the level
     *    sum collective threat of each char to the player
     * 2. cycle each character of monsters on the level
     *    sum collective threat of each char in general
     */

    /* Clear previous dangers */
    biggest_danger = (char)0;
    biggest_threat = (char)0;
    for (i = 0; i < UCHAR_MAX; i++) {
        b_danger[i]     = 0;
        b_kill_count[i] = 0;
        b_threat[i]     = 0;
    }

    /* start with a danger of 1 so non-dangerous (0) will not be saved */
    max_threat = 1;

    /* Find a monster and calculate its danger */
    for (i = 1; i < borg_kills_nxt; i++) {
        /* Monster */
        kill  = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* we try not to genocide uniques */
        if (rf_has(r_ptr->flags, RF_UNIQUE))
            continue;

        /* Calculate danger */
        /* Danger to me by this monster */
        p = borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);

        /* Danger of this monster to his own grid */
        p_threat
            = borg_danger_one_kill(kill->pos.y, kill->pos.x, 1, i, true, true);

        /* Our char of the monster.  This is used because genocide uses a */
        /* single char designation for selecting the kill */
        kill_char = (unsigned char)r_ptr->d_char;

        /* total danger to me */
        total_danger_to_me += p;

        /* store the danger for this type of monster */
        /* Danger to me */
        b_danger[kill_char] = b_danger[kill_char] + p;
        /* Danger added to level */
        b_threat[kill_char] = b_threat[kill_char] + p_threat;

        /* Store the count of this type of monster */
        b_kill_count[kill_char]++;
    }

    /* Now, see which race contributes the most danger
     * both to me and danger on the level
     */
    for (i = 0; i < UCHAR_MAX; i++) {

        /* skip the ones not found on this level */
        if (b_kill_count[i] == 0)
            continue;

        /* for the race threatening me right now */
        if (b_danger[i] > max_threat) {
            /* track the race */
            max_threat     = b_danger[i];
            biggest_danger = i;

            /* note the danger with this race gone.  Note that the borg does max
             * his danger from a single monster at 2000 points.  It could be
             * much, much higher at depth 99 or so. What the borg should do is
             * recalculate the danger without considering this monster instead
             * of this hack which does not yield the true danger.
             */
            p_without_kill = total_danger_to_me - b_danger[biggest_danger];
        }

        /* for this race on the whole level */
        if (b_threat[i] > max_threat) {
            /* track the race */
            max_threat     = b_threat[i];
            biggest_threat = i;
        }

        /* Leave an interesting note for debugging */
        if (!borg_simulate)
            borg_note(format("# Race '%c' is a threat with total danger %d "
                             "from %d individuals.",
                i, b_threat[i], b_kill_count[i]));
    }

    /* This will track and decide if it is worth genociding this dangerous race
     * for the level */
    if (biggest_threat) {
        /* Not if I am weak (should have 400 HP really in case of a Pit) */
        if (borg.trait[BI_CURHP] < 375)
            biggest_threat = 0;

        /* The threat must be real */
        if (b_threat[biggest_threat] < borg.trait[BI_MAXHP] * 3)
            biggest_threat = 0;

        /* Too painful to cast it (padded to be safe incase of unknown monsters)
         */
        if ((b_kill_count[biggest_threat] * 4) * 12 / 10
            >= borg.trait[BI_CURHP])
            biggest_threat = 0;

        /* Loads of monsters might be a pit, in which case, try not to nuke them
         */
        if (b_kill_count[biggest_threat] >= 75)
            biggest_threat = 0;

        /* Do not perform in Danger */
        if (p1 > avoidance / 5)
            biggest_threat = 0;

        /* report the danger and most dangerous race */
        if (biggest_threat) {
            borg_note(format("# Race '%c' is a real threat with total danger "
                             "%d from %d individuals.",
                biggest_threat, b_threat[biggest_threat],
                b_kill_count[biggest_threat]));
        }

        /* Genociding this race would reduce the danger of the level */
        tmp_genocide_target = biggest_threat;
    }

    /* Consider the immediate threat genocide */
    if (biggest_danger) {
        /* Too painful to cast it (padded to be safe incase of unknown monsters)
         */
        if ((b_kill_count[biggest_danger] * 4) * 12 / 10
            >= borg.trait[BI_CURHP])
            biggest_danger = 0;

        /* See if he is in real danger, generally,
         * or deeper in the dungeon, conservatively,
         */
        if (p1 < avoidance * 7 / 10
            || (borg.trait[BI_CDEPTH] > 75 && p1 < avoidance * 6 / 10))
            biggest_danger = 0;

        /* Did this help improve my situation? */
        if (p_without_kill <= (avoidance / 2))
            biggest_danger = 0;

        /* Genociding this race would help me immediately */
        if (biggest_danger)
            tmp_genocide_target = biggest_danger;
    }

    /* Complete the genocide routine */
    if (tmp_genocide_target) {
        if (borg_simulate) {
            /* Simulation for immediate danger */
            if (biggest_danger)
                return (p1 - p_without_kill);

            /* Simulation for threat to level */
            if (biggest_threat)
                return (b_threat[biggest_threat]);
        }

        if (biggest_danger)
            borg_note(
                format("# Banishing race '%c' (qty:%d).  Danger after spell:%d",
                    tmp_genocide_target, b_kill_count[biggest_danger],
                    p_without_kill));
        if (biggest_threat)
            borg_note(
                format("# Banishing race '%c' (qty:%d).  Danger from them:%d",
                    tmp_genocide_target, b_kill_count[biggest_threat],
                    b_threat[biggest_threat]));

        /* do it! ---use scrolls first since they clutter inventory */
        if (borg_read_scroll(sv_scroll_banishment) || borg_spell(BANISHMENT)
            || borg_activate_item(act_banishment)
            || borg_use_staff(sv_staff_banishment)) {
            /* and the winner is.....*/
            borg_keypress(tmp_genocide_target);

            /* Remove this race from the borg_kill */
            for (i = 1; i < borg_kills_nxt; i++) {
                /* Monster */
                kill  = &borg_kills[i];
                r_ptr = &r_info[kill->r_idx];

                /* Our char of the monster */
                if (r_ptr->d_char != tmp_genocide_target)
                    continue;

                /* we do not genocide uniques */
                if (rf_has(r_ptr->flags, RF_UNIQUE))
                    continue;

                /* remove this monster */
                borg_delete_kill(i);
            }
        }

        /* immediate danger to borg */
        if (biggest_danger)
            return (p1 - p_without_kill);

        /* threat to level */
        if (biggest_threat)
            return (b_threat[biggest_threat]);
    }
    /* default to can't do it. */
    return 0;
}

/* This will cast the genocide spell on Hounds and other
 * really nasty guys like Ainur, Demons, Dragons and Liches
 * at the beginning of each level or when they get too numerous.
 * The acceptable numbers are defined in borg_nasties_limit[]
 * The definition for the list is in borg-fight-attack.h
 * borg_nasties[7] = "ZAVULWD"
 *
 */
static int borg_defend_aux_genocide_nasties(int p1)
{
    int i               = 0;
    int b_i             = -1;

    bool genocide_spell = false;

    /* Not if I am weak */
    if (borg.trait[BI_CURHP] < (borg.trait[BI_MAXHP] * 7 / 10)
        || borg.trait[BI_CURHP] < 250)
        return 0;

    /* only do it when Hounds start to show up, */
    if (borg.trait[BI_CDEPTH] < 25)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Do not perform in Danger */
    if (p1 > avoidance / 4)
        return 0;

    if (borg_spell_okay_fail(BANISHMENT, 35)
        || borg_equips_item(act_banishment, true)
        || borg_equips_staff_fail(sv_staff_banishment)) {
        genocide_spell = true;
    }

    if (genocide_spell == false)
        return 0;

    /* Find the numerous nasty in order of nastiness */
    for (i = 0; i < borg_nasties_num; i++) {
        if (borg_nasties_count[i] >= borg_nasties_limit[i])
            b_i = i;
    }

    /* Nothing good to Genocide */
    if (b_i == -1)
        return 0;

    if (borg_simulate)
        return (10);

    /* Note it */
    borg_note(format("# Banishing nasties '%c' (qty:%d).", borg_nasties[b_i],
        borg_nasties_count[b_i]));

    /* Execute -- Nice pun*/
    if (borg_activate_item(act_banishment)
        || borg_use_staff(sv_staff_banishment) || borg_spell(BANISHMENT)) {
        /* and the winner is.....*/
        borg_keypress(borg_nasties[b_i]);

        /* set the count to not do it again */
        borg_nasties_count[b_i] = 0;

        /* Remove this race from the borg_kill */
        for (i = 1; i < borg_kills_nxt; i++) {
            borg_kill           *kill;
            struct monster_race *r_ptr;

            /* Monster */
            kill  = &borg_kills[i];
            r_ptr = &r_info[kill->r_idx];

            /* Our char of the monster */
            if (r_ptr->d_char != borg_nasties[b_i])
                continue;

            /* remove this monster */
            borg_delete_kill(i);
        }

        return (10);
    }

    /* default to can't do it. */
    return 0;
}

/* Earthquake, priest and mage spells.
 */
static int borg_defend_aux_earthquake(int p1)
{
    int p2 = 9999;
    int i;
    int threat_count = 0;

    borg_kill *kill;

    /* Cast the spell */
    if (!borg_simulate) {
        /* Must make a new Sea too */
        borg_needs_new_sea = true;

        /* for now aim Tremor around the borg */
        if (borg_spell(TREMOR)) {
            borg_keypress('*');
            borg_keypress('5');
        } else
            /* other spells don't need aim */
            if (borg_spell(QUAKE) || borg_spell(GRONDS_BLOW)
                || borg_activate_item(act_earthquakes))
                return (p2);
    }

    /* Can't when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Can I cast the spell? */
    if (!borg_spell_okay_fail(TREMOR, 35) && !borg_spell_okay_fail(QUAKE, 35)
        && !borg_spell_okay_fail(GRONDS_BLOW, 35)
        && !borg_equips_item(act_earthquakes, true))
        return 0;

    /* See if he is in real danger or fighting summoner*/
    if (p1 < avoidance * 6 / 10 && !borg_fighting_summoner)
        return 0;

    /* Several monsters can see the borg and they have ranged attacks */
    for (i = 0; i < borg_kills_nxt; i++) {
        kill = &borg_kills[i];

        /* Look for threats */
        if (borg_los(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x)
            && kill->ranged_attack && distance(kill->pos, borg.c) >= 2) {
            /* They can hit me */
            threat_count++;
        }
    }

    /* Real danger? */
    if (threat_count >= 4 && p1 > avoidance * 7 / 10)
        p2 = p1 / 3;
    if (threat_count == 3 && p1 > avoidance * 7 / 10)
        p2 = p1 * 6 / 10;

    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))
        && p1 > (avoidance / 5)) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);
    }
    return 0;
}

/* Word of Destruction, priest and mage spells.  Death is right around the
 *  corner, so kill everything.
 */
static int borg_defend_aux_destruction(int p1)
{
    int  p2          = 0;
    int  d           = 0;
    bool spell       = false;
    bool real_danger = false;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Cast the spell */
    if (!borg_simulate) {
        if (borg_spell(WORD_OF_DESTRUCTION)
            || borg_use_staff(sv_staff_destruction)
            || borg_activate_item(act_destruction2)) {
            /* Must make a new Sea too */
            borg_needs_new_sea = true;

            /* borg9.c will check for the success of the spell and remove the
             * danger from the grids.
             */
        }

        return (500);
    }

    /* Not if in a sea of runes */
    if (borg_morgoth_position)
        return 0;

    /* See if he is in real danger */
    if (p1 > avoidance)
        real_danger = true;
    if (p1 > avoidance * 8 / 10 && borg.trait[BI_CDEPTH] >= 90
        && borg.trait[BI_CURHP] <= 300)
        real_danger = true;

    if (real_danger == false)
        return 0;

    /* Borg_defend() is called before borg_escape().  He may have some
     * easy ways to escape (teleport scroll) but he may attempt this spell
     * of Destruction instead of using the scrolls.
     * Note that there will be some times when it is better for
     * the borg to use Destruction instead of Teleport;  too
     * often he will die out-of-the-frying-pan-into-the-fire.
     * So we have him to a quick check on safe landing zones.
     */

    /* Examine landing zones from teleport scrolls instead of WoD */
    if ((borg.trait[BI_ATELEPORT] || borg.trait[BI_ATELEPORTLVL])
        && !borg.trait[BI_ISBLIND] && !borg.trait[BI_ISCONFUSED]
        && borg_fighting_unique <= 4 && borg.trait[BI_CURHP] >= 275) {
        if (borg_caution_teleport(75, 2))
            return 0;
    }

    /* Examine Landing zones from teleport staff instead of WoD */
    if (borg.trait[BI_AESCAPE] >= 2 && borg.trait[BI_CURHP] >= 275) {
        if (borg_caution_teleport(75, 2))
            return 0;
    }

    /* capable of casting the spell */
    if (borg_spell_okay_fail(WORD_OF_DESTRUCTION, 55)
        || borg_equips_staff_fail(sv_staff_destruction)
        || borg_equips_item(act_destruction2, true))
        spell = true;

    /* Special check for super danger--no fail check */
    if ((p1 > (avoidance * 4)
            || (p1 > avoidance && borg.trait[BI_CURHP] <= 150))
        && borg_equips_staff_fail(sv_staff_destruction))
        spell = true;

    if (spell == false)
        return 0;

    /* What effect is there? */
    p2 = 0;

    /* value is d */
    d = (p1 - p2);

    /* Try not to cast this against uniques */
    if (borg_fighting_unique <= 2 && p1 < avoidance * 2)
        d = 0;
    if (borg_fighting_unique >= 10)
        d = 0;

    /* Simulation */
    if (borg_simulate)
        return d;

    return 0;
}

/* Teleport Level, priest and mage spells.  Death is right around the
 *  corner, Get off the level now.
 */
static int borg_defend_aux_teleportlevel(int p1)
{
    /* Cast the spell */
    if (!borg_simulate) {
        if (borg_spell(TELEPORT_LEVEL)) {
            /* Must make a new Sea too */
            borg_needs_new_sea = true;
            return (500);
        }
    }

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* See if he is in real danger */
    if (p1 < avoidance * 2)
        return 0;

    /* Borg_defend() is called before borg_escape().  He may have some
     * easy ways to escape (teleport scroll) but he may attempt this spell
     * of this spell instead of using the scrolls.
     * Note that there will be some times when it is better for
     * the borg to use this instead of Teleport;  too
     * often he will die out-of-the-frying-pan-into-the-fire.
     * So we have him to a quick check on safe landing zones.
     */

    /* Use teleport scrolls instead if safe to land */
    if ((borg.trait[BI_ATELEPORT] || borg.trait[BI_ATELEPORTLVL])
        && !borg.trait[BI_ISBLIND] && !borg.trait[BI_ISCONFUSED]) {
        if (borg_caution_teleport(65, 2))
            return 0;
    }

    /* Use teleport staff instead if safe to land */
    if (borg.trait[BI_AESCAPE] >= 2) {
        if (borg_caution_teleport(65, 2))
            return 0;
    }

    /* capable of casting the spell */
    if (!borg_spell_okay_fail(TELEPORT_LEVEL, 55))
        return 0;

    /* Try not to cast this against special uniques */
    if (morgoth_on_level || (borg_fighting_unique >= 1 && borg_as_position))
        return 0;

    /* Simulation */
    if (borg_simulate)
        return (p1);

    return 0;
}

/* Remove Evil guys within LOS.  The Priest Spell */
static int borg_defend_aux_banishment(int p1)
{
    int  p2           = 0;
    int  fail_allowed = 15;
    int  i;
    int  banished_monsters = 0;
    bool using_artifact;

    borg_grid *ag;

    /* Only tell away if scared */
    if (p1 < avoidance * 1 / 10)
        return 0;

    /* if very scary, do not allow for much chance of fail */
    if (p1 > avoidance * 4)
        fail_allowed -= 10;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    using_artifact
        = borg_equips_item(act_loskill, true) && borg.has[BI_CURHP] > 100;

    if (!using_artifact && !borg_spell_okay_fail(BANISH_EVIL, fail_allowed))
        return 0;

    /* reset initial danger */
    p1 = 1;

    /* Two passes to determine exact danger */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        ag   = &borg_grids[kill->pos.y][kill->pos.x];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Check the LOS */
        if (!borg_projectable(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x))
            continue;

        /* Calculate danger of who is left over */
        p1 += borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
    }

    /* Set P2 to be P1 and subtract the danger from each monster
     * which will be booted.  Non booted monsters wont decrement
     * the p2
     */
    p2 = p1;

    /* Pass two -- Find a monster and calculate its danger */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill           *kill;
        struct monster_race *r_ptr;

        /* Monster */
        kill  = &borg_kills[i];
        r_ptr = &r_info[kill->r_idx];

        ag    = &borg_grids[kill->pos.y][kill->pos.x];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Check the LOS */
        if (!borg_projectable(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x))
            continue;

        /* Note who gets considered */
        if (!borg_simulate) {
            borg_note(format(
                "# Banishing Evil: (%d,%d): %s, danger %d. is considered.",
                kill->pos.y, kill->pos.x, (r_info[kill->r_idx].name),
                borg_danger_one_kill(
                    borg.c.y, borg.c.x, 1, ag->kill, true, false)));
        }

        /* Non evil monsters*/
        if (!(rf_has(r_ptr->flags, RF_EVIL))) {
            /* Note who gets to stay */
            if (!borg_simulate) {
                borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. "
                                 "Stays (not evil).",
                    kill->pos.y, kill->pos.x, (r_info[kill->r_idx].name),
                    borg_danger_one_kill(
                        borg.c.y, borg.c.x, 1, ag->kill, true, false)));
            }

            continue;
        }

        /* Unique Monster in good health*/
        if (rf_has(r_ptr->flags, RF_UNIQUE) && kill->injury > 60) {
            /* Note who gets to stay */
            if (!borg_simulate) {
                borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. "
                                 "Unique not considered: Injury %d.",
                    kill->pos.y, kill->pos.x, (r_info[kill->r_idx].name),
                    borg_danger_one_kill(
                        borg.c.y, borg.c.x, 1, ag->kill, true, false),
                    kill->injury));
            }

            continue;
        }

        /* Monsters in walls cant be booted */
        if (!borg_cave_floor_bold(kill->pos.y, kill->pos.x)) {
            /* Note who gets banished */
            if (!borg_simulate) {
                borg_note(format("# Banishing Evil: (%d,%d): %s, danger %d. "
                                 "Stays (in wall).",
                    kill->pos.y, kill->pos.x, (r_info[kill->r_idx].name),
                    borg_danger_one_kill(
                        borg.c.y, borg.c.x, 1, ag->kill, true, true)));
            }
            continue;
        }

        /* Note who gets banished */
        if (!borg_simulate) {
            borg_note(
                format("# Banishing Evil: (%d,%d): %s, danger %d. Booted.",
                    kill->pos.y, kill->pos.x, (r_info[kill->r_idx].name),
                    borg_danger_one_kill(
                        borg.c.y, borg.c.x, 1, ag->kill, true, true)));
            borg_delete_kill(i);
        }

        /* Count */
        banished_monsters++;

        /* Calculate danger of who is left over */
        p2 -= borg_danger_one_kill(borg.c.y, borg.c.x, 1, i, true, true);
    }

    if (!borg_simulate) {
        /* attempt the banish */
        if (using_artifact)
            if (borg_activate_item(act_loskill))
                return (p1 - p2);
        if (borg_spell(BANISH_EVIL))
            return (p1 - p2);
    }

    /* p2 is the danger after all the bad guys are removed. */
    /* no negatives */
    if (p2 <= 0)
        p2 = 0;

    /* No monsters get booted */
    if (banished_monsters == 0)
        p2 = 9999;

    /* Try not to cast this against Morgy/Sauron */
    if (borg_fighting_unique >= 10 && borg.trait[BI_CURHP] > 250
        && borg.trait[BI_CDEPTH] == 99)
        p2 = 9999;
    if (borg_fighting_unique >= 10 && borg.trait[BI_CURHP] > 350
        && borg.trait[BI_CDEPTH] == 100)
        p2 = 9999;

    /* check to see if I am left better off */
    if (p1 > p2
        && p2 <= (borg_fighting_unique ? ((avoidance * 2) / 3)
                                       : (avoidance / 2))) {
        /* Simulation */
        if (borg_simulate)
            return (p1 - p2);
    }
    return 0;
}

/*
 * Detect Inviso/Monsters
 * Used only if I am hit by an unseen guy.
 * Casts detect invis.
 */
static int borg_defend_aux_inviso(int p1)
{
    int        fail_allowed = 25;
    borg_grid *ag           = &borg_grids[borg.c.y][borg.c.x];

    /* no need */
    if (borg.trait[BI_ISFORGET] || borg.trait[BI_ISBLIND]
        || borg.trait[BI_ISCONFUSED] || borg.see_inv)
        return 0;

    /* not recent */
    if (borg_t > borg.need_see_invis + 5)
        return 0;

    /* too dangerous to cast */
    if (p1 > avoidance * 2)
        return 0;

    /* Do I have anything that will work? */
    if (-1 == borg_slot(TV_POTION, sv_potion_detect_invis)
        && -1 == borg_slot(TV_SCROLL, sv_scroll_detect_invis)
        && !borg_equips_staff_fail(sv_staff_detect_invis)
        && !borg_equips_staff_fail(sv_staff_detect_evil)
        && !borg_spell_okay_fail(SENSE_INVISIBLE, fail_allowed)
        && !borg_spell_okay_fail(DETECTION, fail_allowed)
        && !borg_equips_item(act_detect_invis, true)
        && !borg_equips_item(act_tmd_sinvis, true)
        && !borg_equips_item(act_tmd_esp, true)
        && !borg_equips_item(act_detect_evil, true))
        return 0;

    /* Darkness */
    if (!(ag->info & BORG_GLOW) && !borg.trait[BI_CURLITE])
        return 0;

    /* No real value known, but lets cast it to find the bad guys. */
    if (borg_simulate)
        return (10);

    /* smoke em if you got em */
    /* short time */
    /* snap shot */
    if (borg_spell_fail(REVEAL_MONSTERS, fail_allowed)
        || borg_read_scroll(sv_scroll_detect_invis)
        || borg_use_staff(sv_staff_detect_invis)
        || borg_use_staff(sv_staff_detect_evil)
        || borg_activate_item(act_detect_invis)
        || borg_activate_item(act_tmd_sinvis) || borg_activate_item(act_tmd_esp)
        || borg_activate_item(act_detect_evil)) {
        borg.see_inv
            = 3000; /* hack, actually a snap shot, no ignition message */
        return (10);
    }
    if (borg_quaff_potion(sv_potion_detect_invis)) {
        borg.see_inv      = 18000;
        borg.no_rest_prep = 18000;
        return (10);
    }
    /* long time */
    if (borg_spell_fail(SENSE_INVISIBLE, fail_allowed)) {
        borg.see_inv      = 30000;
        borg.no_rest_prep = 16000;
        return (10);
    }

    /* ah crap, I guess I wont be able to see them */
    return 0;
}

/*
 * Light Beam to spot lurkers
 * Used only if I am hit by an unseen guy.
 * Lights up a hallway.
 */
static int borg_defend_aux_lbeam(int p1)
{
    bool hallway = false;
    int  x       = borg.c.x;
    int  y       = borg.c.y;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Light Beam section to spot non seen guys */
    /* not recent, don't bother */
    if (borg_t > (borg.need_see_invis + 2))
        return 0;

    /* Check to see if I am in a hallway */
    /* Case 1a: north-south corridor */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x)
        && !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1)
        && !borg_cave_floor_bold(y + 1, x - 1)
        && !borg_cave_floor_bold(y + 1, x + 1)
        && !borg_cave_floor_bold(y - 1, x - 1)
        && !borg_cave_floor_bold(y - 1, x + 1)) {
        /* ok to light up */
        hallway = true;
    }

    /* Case 1b: east-west corridor */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1)
        && !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x)
        && !borg_cave_floor_bold(y + 1, x - 1)
        && !borg_cave_floor_bold(y + 1, x + 1)
        && !borg_cave_floor_bold(y - 1, x - 1)
        && !borg_cave_floor_bold(y - 1, x + 1)) {
        /* ok to light up */
        hallway = true;
    }

    /* Case 1aa: north-south doorway */
    if (borg_cave_floor_bold(y - 1, x) && borg_cave_floor_bold(y + 1, x)
        && !borg_cave_floor_bold(y, x - 1) && !borg_cave_floor_bold(y, x + 1)) {
        /* ok to light up */
        hallway = true;
    }

    /* Case 1ba: east-west doorway */
    if (borg_cave_floor_bold(y, x - 1) && borg_cave_floor_bold(y, x + 1)
        && !borg_cave_floor_bold(y - 1, x) && !borg_cave_floor_bold(y + 1, x)) {
        /* ok to light up */
        hallway = true;
    }

    /* not in a hallway */
    if (!hallway)
        return 0;

    /* Make sure I am not in too much danger */
    if (borg_simulate && p1 > (avoidance * 3) / 4)
        return 0;

    /* test the beam function */
    if (!borg_light_beam(true))
        return 0;

    /* return some value */
    if (borg_simulate)
        return (10);

    /* if in a hallway call the Light Beam routine */
    if (borg_light_beam(false)) {
        return (10);
    }
    return 0;
}

/* Shift the panel to locate offscreen monsters */
static int borg_defend_aux_panel_shift(void)
{
    int dir = 0;
    int wx  = Term->offset_x / borg_panel_wid();
    int wy  = Term->offset_y / borg_panel_hgt();

    /* no need */
    if (!borg.need_shift_panel && borg.trait[BI_CDEPTH] < 70)
        return 0;

    /* if Morgy is on my panel, dont do it */
    if (borg.trait[BI_CDEPTH] == 100 && w_y == morgy_panel_y
        && w_x == morgy_panel_x)
        return 0;

    /* Which direction do we need to move? */
    /* Shift panel to the right */
    if (borg.c.x >= 52 && borg.c.x <= 60 && wx == 0)
        dir = 6;
    if (borg.c.x >= 84 && borg.c.x <= 94 && wx == 1)
        dir = 6;
    if (borg.c.x >= 116 && borg.c.x <= 123 && wx == 2)
        dir = 6;
    if (borg.c.x >= 148 && borg.c.x <= 159 && wx == 3)
        dir = 6;
    /* Shift panel to the left */
    if (borg.c.x <= 142 && borg.c.x >= 136 && wx == 4)
        dir = 4;
    if (borg.c.x <= 110 && borg.c.x >= 103 && wx == 3)
        dir = 4;
    if (borg.c.x <= 78 && borg.c.x >= 70 && wx == 2)
        dir = 4;
    if (borg.c.x <= 46 && borg.c.x >= 37 && wx == 1)
        dir = 4;

    /* Shift panel down */
    if (borg.c.y >= 15 && borg.c.y <= 19 && wy == 0)
        dir = 2;
    if (borg.c.y >= 25 && borg.c.y <= 30 && wy == 1)
        dir = 2;
    if (borg.c.y >= 36 && borg.c.y <= 41 && wy == 2)
        dir = 2;
    if (borg.c.y >= 48 && borg.c.y <= 52 && wy == 3)
        dir = 2;
    /* Shift panel up */
    if (borg.c.y <= 51 && borg.c.y >= 47 && wy == 4)
        dir = 8;
    if (borg.c.y <= 39 && borg.c.y >= 35 && wy == 3)
        dir = 8;
    if (borg.c.y <= 28 && borg.c.y >= 24 && wy == 2)
        dir = 8;
    if (borg.c.y <= 17 && borg.c.y >= 13 && wy == 1)
        dir = 8;

    /* Do the Shift if needed, then note it,  reset the flag */
    if (borg.need_shift_panel == true) {
        /* Send action (view panel info) */
        borg_keypress('L');

        if (dir)
            borg_keypress(I2D(dir));
        borg_keypress(ESCAPE);

        borg_note("# Shifted panel to locate offscreen monster.");
        borg.need_shift_panel = false;

        /* Leave the panel shift mode */
        borg_keypress(ESCAPE);
    } else
    /* check to make sure its appropriate */
    {

        /* Hack Not if I just did one */
        if (borg.when_shift_panel
            && (borg_t - borg.when_shift_panel <= 10
                || borg_t - borg_t_morgoth <= 10)) {
            /* do nothing */
        } else {
            /* if not the first step */
            if (track_step.num) {
                /* shift up? only if a north corridor */
                if (dir == 8
                    && borg_projectable_pure(
                        borg.c.y, borg.c.x, borg.c.y - 2, borg.c.x)
                    && track_step.y[track_step.num - 1] != borg.c.y - 1) {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    if (dir)
                        borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    borg.when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
                /* shift down? only if a south corridor */
                else if (dir == 2
                         && borg_projectable_pure(
                             borg.c.y, borg.c.x, borg.c.y + 2, borg.c.x)
                         && track_step.y[track_step.num - 1] != borg.c.y + 1) {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    borg.when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
                /* shift Left? only if a west corridor */
                else if (dir == 4
                         && borg_projectable_pure(
                             borg.c.y, borg.c.x, borg.c.y, borg.c.x - 2)
                         && track_step.x[track_step.num - 1] != borg.c.x - 1) {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    if (dir)
                        borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    borg.when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
                /* shift Right? only if a east corridor */
                else if (dir == 6
                         && borg_projectable_pure(
                             borg.c.y, borg.c.x, borg.c.y, borg.c.x + 2)
                         && track_step.x[track_step.num - 1] != borg.c.x + 1) {
                    /* Send action (view panel info) */
                    borg_keypress('L');
                    if (dir)
                        borg_keypress(I2D(dir));
                    borg_note("# Shifted panel as a precaution.");
                    /* Mark the time to avoid loops */
                    borg.when_shift_panel = borg_t;
                    /* Leave the panel shift mode */
                    borg_keypress(ESCAPE);
                }
            }
        }
    }
    /* This uses no energy */
    return 0;
}

/* This and the next routine is used on level 100 and when
 * attacking Morgoth. The borg has found a safe place to wait
 * for Morgoth to show.
 *
 * If the borg is not being threatened immediately by a monster,
 * then rest right here.
 *
 * Only borgs with teleport away and a good attack spell do this
 * routine.
 */
static int borg_defend_aux_rest(void)
{
    int i;

    if (!borg_morgoth_position
        && (!borg_as_position || borg_t - borg_t_antisummon >= 50))
        return 0;

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level
        && (!borg_as_position || borg_t - borg_t_antisummon >= 50))
        return 0;

        /* Not if I can not teleport others away */
#if 0
    if (!borg_spell_okay_fail(3, 1, 30) &&
        !borg_spell_okay_fail(4, 2, 30)) return 0;
#endif
    /* Not if a monster can see me */
    /* Examine all the monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        borg_kill *kill = &borg_kills[i];

        int x9          = kill->pos.x;
        int y9          = kill->pos.y;
        int ax, ay, d;

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Distance components */
        ax = (x9 > borg.c.x) ? (x9 - borg.c.x) : (borg.c.x - x9);
        ay = (y9 > borg.c.y) ? (y9 - borg.c.y) : (borg.c.y - y9);

        /* Distance */
        d = MAX(ax, ay);

        /* Minimal distance */
        if (d > z_info->max_range)
            continue;

        /* If I can see Morgoth or a guy with Ranged Attacks, don't rest. */
        if (borg_los(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x)
            && (kill->r_idx == borg_morgoth_id || kill->ranged_attack)
            && avoidance <= borg.trait[BI_CURHP]) {
            borg_note("# Not resting. I can see Morgoth or a shooter.");
            return 0;
        }

        /* If a little twitchy, its ok to stay put */
        if (avoidance > borg.trait[BI_CURHP])
            continue;
    }

    /* Return some value for this rest */
    if (borg_simulate)
        return (200);

    /* Rest */
    borg_keypress(',');
    borg_note(format("# Resting on grid (%d, %d), waiting for Morgoth.",
        borg.c.y, borg.c.x));

    /* All done */
    return (200);
}

/*
 * Try to get rid of all of the monsters while I build my
 * Sea of Runes.
 */
static int borg_defend_aux_tele_away_morgoth(void)
{
    int p2           = 0;
    int fail_allowed = 40;
    int i;

    borg_grid *ag;

    /* Only if on level 100 */
    if (!(borg.trait[BI_CDEPTH] == 100))
        return 0;

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Do I have the T.O. spell? */
    if (!borg_spell_okay_fail(TELEPORT_OTHER, fail_allowed))
        return 0;

    /* Do I have the Glyph spell? No good to use TO if I cant build the sea of
     * runes */
    if (borg.trait[BI_AGLYPH] < 10)
        return 0;

    /* No Teleport Other if surrounded */
    if (borg_surrounded() == true)
        return 0;

    /* Borg_temp_n temporarily stores several things.
     * Some of the borg_attack() sub-routines use these numbers,
     * which would have been filled in borg_attack().
     * Since this is a defense maneuver which will move into
     * and borrow some of the borg_attack() subroutines, we need
     * to make sure that the borg_temp_n arrays are properly
     * filled.  Otherwise, the borg will attempt to consider
     * these grids which were left filled by some other routine.
     * Which was probably a flow routine which stored about 200
     * grids into the array.
     * Any change in inclusion/exclusion criteria for filling this
     * array in borg_attack() should be included here also.
     */

    /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate)
        return 0;

    /* Reset list */
    borg_temp_n     = 0;
    borg_tp_other_n = 0;

    /* Find "nearby" monsters */
    for (i = 0; i < borg_kills_nxt; i++) {
        borg_kill *kill;

        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Get grid */
        ag = &borg_grids[kill->pos.y][kill->pos.x];

        /* Never shoot off-screen */
        if (!(ag->info & BORG_OKAY))
            continue;

        /* Never shoot through walls */
        if (!(ag->info & BORG_VIEW))
            continue;

        /* Check the distance XXX XXX XXX */
        if (distance(borg.c, kill->pos) > z_info->max_range)
            continue;

        /* Check the LOS */
        if (!borg_projectable(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x))
            continue;

        /* Save the location (careful) */
        borg_temp_x[borg_temp_n] = kill->pos.x;
        borg_temp_y[borg_temp_n] = kill->pos.y;
        borg_temp_n++;
    }

    /* No destinations */
    if (!borg_temp_n && borg_simulate)
        return 0;

    /* choose then target a bad guy or several
     * If left as bolt, he targets the single most nasty guy.
     * If left as beam, he targets the collection of monsters.
     */
    p2 = borg_launch_bolt(
        -1, 50, BORG_ATTACK_AWAY_ALL_MORGOTH, z_info->max_range, 0);

    /* Normalize the value a bit */
    if (p2 > 1000)
        p2 = 1000;

    /* Reset list */
    borg_temp_n     = 0;
    borg_tp_other_n = 0;

    /* Return a good score to make him do it */
    if (borg_simulate)
        return (p2);

    /* Log the Path for Debug */
    borg_log_spellpath(true);

    /* Log additional info for debug */
    for (i = 0; i < borg_tp_other_n; i++) {
        borg_note(format("# %d, index %d (%d,%d)", borg_tp_other_n,
            borg_tp_other_index[i], borg_tp_other_y[i], borg_tp_other_x[i]));
    }

    borg_note("# Attempting to cast T.O. for depth 100.");

    /* Cast the spell */
    if (borg_spell(TELEPORT_OTHER) || borg_activate_item(act_tele_other)
        || borg_aim_wand(sv_wand_teleport_away)) {
        /* Use target */
        borg_keypress('5');

        /* Set our shooting flag */
        successful_target = -1;

        /* Value */
        return (p2);
    }

    return 0;
}

/*
 * Try to get rid of all of the monsters while I build my
 * Sea of Runes.
 */
static int borg_defend_aux_banishment_morgoth(void)
{
    int fail_allowed = 50;
    int i, x, y;
    int count  = 0;
    int glyphs = 0;

    borg_grid           *ag;
    borg_kill           *kill;
    struct monster_race *r_ptr;

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Scan grids looking for glyphs */
    for (i = 0; i < 8; i++) {
        /* Access offset */
        x = borg.c.x + ddx_ddd[i];
        y = borg.c.y + ddy_ddd[i];

        /* Access the grid */
        ag = &borg_grids[y][x];

        /* Check for Glyphs */
        if (ag->glyph)
            glyphs++;
    }

    /* Only if on level 100 and in a sea of runes or
     * in the process of building one
     */
#if 0
    if (!borg_morgoth_position && glyphs < 3) return 0;
#endif

    /* Do I have the spell? (Banish Evil) */
    if (!borg_spell_okay_fail(MASS_BANISHMENT, fail_allowed)
        && !borg_spell_okay_fail(BANISH_EVIL, fail_allowed))
        return 0;

    /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate)
        return 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        r_ptr = &r_info[kill->r_idx];

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Acquire location */
        x = kill->pos.x;
        y = kill->pos.y;

        /* Get grid */
        ag = &borg_grids[y][x];

        /* Never try on non-evil guys if Priest */
        if (borg.trait[BI_CLASS] == CLASS_PRIEST
            && !(rf_has(r_ptr->flags, RF_EVIL)))
            continue;

        /* Check the distance  */
        if (distance(borg.c, kill->pos) > z_info->max_range)
            continue;

        /* Monster must be LOS */
        if (!borg_projectable(borg.c.y, borg.c.x, kill->pos.y, kill->pos.x))
            continue;

        /* Count the number of monsters too close double*/
        if (distance(borg.c, kill->pos) <= 7)
            count++;

        /* Count the number of monster on screen */
        count++;
    }

    /* No destinations */
    if (count <= 7 && borg_simulate)
        return 0;

    /* Return a good score to make him do it */
    if (borg_simulate)
        return (1500);

    borg_note(format(
        "# Attempting to cast Banishment for depth 100.  %d monsters ", count));

    /* Cast the spell */
    if (borg_spell(MASS_BANISHMENT) || borg_spell(BANISH_EVIL)) {
        /* Remove this race from the borg_kill */
        for (i = 0; i < borg_kills_nxt; i++) {
            borg_kill           *tmp_kill;
            struct monster_race *tmp_r_ptr;

            /* Monster */
            tmp_kill  = &borg_kills[i];
            tmp_r_ptr = &r_info[tmp_kill->r_idx];

            /* Cant kill uniques like this */
            if (rf_has(tmp_r_ptr->flags, RF_UNIQUE))
                continue;

            /* remove this monster */
            borg_delete_kill(i);
        }

        /* Value */
        return 1000;
    }

    return 0;
}

/*
 * Sometimes the borg will not fire on Morgoth as he approaches
 * while tunneling through rock.  The borg still remembers and
 * assumes that the rock is unknown grid.
 */
static int borg_defend_aux_light_morgoth(void)
{
    int        fail_allowed = 50;
    int        i, x, y;
    struct loc best;
    int        count = 0;

    borg_kill *kill;

    /* Only if on level 100 and in a sea of runes */
    if (!borg_morgoth_position)
        return 0;

    /* Not if Morgoth is not on this level */
    if (!morgoth_on_level)
        return 0;

    /* Cant when screwed */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED]
        || borg.trait[BI_ISFORGET])
        return 0;

    /* Do I have the spell? */
    if (!borg_spell_okay_fail(SPEAR_OF_LIGHT, fail_allowed)
        && !borg_spell_okay_fail(CLAIRVOYANCE, fail_allowed)
        && !borg_spell_okay_fail(FUME_OF_MORDOR, fail_allowed))
        return 0;

    /* Nobody around so dont worry */
    if (!borg_kills_cnt && borg_simulate)
        return 0;

    /* Find "nearby" monsters */
    for (i = 1; i < borg_kills_nxt; i++) {
        /* Monster */
        kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx)
            continue;

        /* Skip non- Morgoth monsters */
        if (kill->r_idx != borg_morgoth_id)
            continue;

        /* Require current knowledge */
        if (kill->when < borg_t - 2)
            continue;

        /* Acquire location */
        x = kill->pos.x;
        y = kill->pos.y;

        /* Check the distance  */
        if (distance(borg.c, kill->pos) > z_info->max_range)
            continue;
        if (distance(borg.c, kill->pos) <= 5)
            continue;

        /* We want at least one dark spot on the path */
        if (!borg_projectable_dark(borg.c.y, borg.c.x, y, x))
            continue;

        /* Count Morgoth so I try the spell */
        count++;
        best.y = y;
        best.x = x;
    }

    /* No destinations */
    if (count <= 0 && borg_simulate)
        return 0;

    /* Return a good score to make him do it */
    if (borg_simulate)
        return (500);

    borg_note(format(
        "# Attempting to Illuminate a Pathway to (%d, %d)", best.y, best.x));

    /* Target Morgoth Grid */
    (void)borg_target(best);

    /* Cast the spell */
    if (borg_spell(SPEAR_OF_LIGHT) || borg_spell(CLAIRVOYANCE)
        || borg_spell(FUME_OF_MORDOR)) {
        /* Select the target */
        borg_keypress('5');

        /* Value */
        return (200);
    }

    return 0;
}

/*
 * Simulate/Apply the optimal result of using the given "type" of defense
 * p1 is the current danger level (passed in for effiency)
 */
static int borg_calculate_defense_effectiveness(int what, int p1)
{
    /* Analyze */
    switch (what) {
    case BD_SPEED: {
        return (borg_defend_aux_speed(p1));
    }
    case BD_PROT_FROM_EVIL: {
        return (borg_defend_aux_prot_evil(p1));
    }
    case BD_GRIM_PURPOSE: {
        return (borg_defend_aux_grim_purpose(p1));
    }
    case BD_RESIST_FECAP: {
        return (borg_defend_aux_resist_fecap(p1));
    }
    case BD_RESIST_F: {
        return (borg_defend_aux_resist_f(p1));
    }
    case BD_RESIST_C: {
        return (borg_defend_aux_resist_c(p1));
    }
    case BD_RESIST_A: {
        return (borg_defend_aux_resist_a(p1));
    }
    case BD_RESIST_E: {
        return (borg_defend_aux_resist_e(p1));
    }
    case BD_RESIST_P: {
        return (borg_defend_aux_resist_p(p1));
    }
    case BD_BLESS: {
        return (borg_defend_aux_bless(p1));
    }
    case BD_HERO: {
        return (borg_defend_aux_hero(p1));
    }
    case BD_BERSERK: {
        return (borg_defend_aux_berserk(p1));
    }
    case BD_SMITE_EVIL: {
        return (borg_defend_aux_smite_evil(p1));
    }
    case BD_REGEN: {
        return (borg_defend_aux_regen(p1));
    }
    case BD_SHIELD: {
        return (borg_defend_aux_shield(p1));
    }
    case BD_TELE_AWAY: {
        return (borg_defend_aux_tele_away(p1));
    }
    case BD_GLYPH: {
        return (borg_defend_aux_glyph(p1));
    }
    case BD_CREATE_DOOR: {
        return (borg_defend_aux_create_door(p1));
    }
    case BD_MASS_GENOCIDE: {
        return (borg_defend_aux_mass_genocide(p1));
    }
    case BD_GENOCIDE: {
        return (borg_defend_aux_genocide(p1));
    }
    case BD_GENOCIDE_NASTIES: {
        return (borg_defend_aux_genocide_nasties(p1));
    }
    case BD_EARTHQUAKE: {
        return (borg_defend_aux_earthquake(p1));
    }
    case BD_TPORTLEVEL: {
        return (borg_defend_aux_teleportlevel(p1));
    }
    case BD_DESTRUCTION: {
        return (borg_defend_aux_destruction(p1));
    }
    case BD_BANISHMENT: {
        return (borg_defend_aux_banishment(p1));
    }
    case BD_DETECT_INVISO: {
        return (borg_defend_aux_inviso(p1));
    }
    case BD_LIGHT_BEAM: {
        return (borg_defend_aux_lbeam(p1));
    }
    case BD_SHIFT_PANEL: {
        return (borg_defend_aux_panel_shift());
    }
    case BD_REST: {
        return (borg_defend_aux_rest());
    }
    case BD_TELE_AWAY_MORGOTH: {
        return (borg_defend_aux_tele_away_morgoth());
    }
    case BD_BANISHMENT_MORGOTH: {
        return (borg_defend_aux_banishment_morgoth());
    }
    case BD_LIGHT_MORGOTH: {
        return (borg_defend_aux_light_morgoth());
    }
    }
    return 0;
}

/*
 * prepare to attack... this is setup for a battle.
 */
bool borg_defend(int p1)
{
    int n, b_n = 0;
    int g, b_g = -1;

    /* Simulate */
    borg_simulate = true;

    /* if you have Resist All and it is about to drop, */
    /* refresh it (if you can) */
    if (borg.resistance && borg.resistance < (borg_game_ratio * 2)) {
        int p;

        /* check 'true' danger. This will make sure we do not */
        /* refresh our Resistance if no-one is around */
        borg_attacking = true;
        p              = borg_danger(
            borg.c.y, borg.c.x, 1, false, false); /* Note false for danger!! */
        borg_attacking = false;
        if (p > borg_fear_region[borg.c.y / 11][borg.c.x / 11]
            || borg_fighting_unique) {
            if (borg_spell(RESISTANCE)) {
                borg_note(format("# Refreshing Resistance.  "
                                 "borg.resistance=%d, player->=%d, (ratio=%d)",
                    borg.resistance, player->timed[TMD_OPP_ACID],
                    borg_game_ratio));
                borg_attempting_refresh_resist = true;
                borg.resistance                = 25000;
                return true;
            }
        }
    }

    /* Analyze the possible setup moves */
    for (g = 0; g < BD_MAX; g++) {
        /* Simulate */
        n = borg_calculate_defense_effectiveness(g, p1);

        /* Track "best" attack */
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
    borg_note(format("# Performing defense type %d with value %d", b_g, b_n));

    /* Instantiate */
    borg_simulate = false;

    /* Instantiate */
    (void)borg_calculate_defense_effectiveness(b_g, p1);

    /* Success */
    return true;
}

#endif
