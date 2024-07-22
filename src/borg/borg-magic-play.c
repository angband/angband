/**
 * \file borg-magic-play.c
 * \brief Cast spells just to learn the spell.
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

#include "borg-magic-play.h"

#ifdef ALLOW_BORG

#include "../effect-handler.h"
#include "../effects.h"
#include "../ui-menu.h"

#include "borg-cave-view.h"
#include "borg-cave.h"
#include "borg-io.h"
#include "borg-magic.h"
#include "borg-trait.h"

static const struct effect_kind effects[]
    = { { EF_NONE, false, NULL, NULL, NULL, NULL },
#define F(x)                        effect_handler_##x
#define EFFECT(x, a, b, c, d, e, f) { EF_##x, a, b, F(x), e, f },
#include "list-effects.h"
#undef EFFECT
#undef F
          { EF_MAX, false, NULL, NULL, NULL, NULL } };

/* Check if a spell can be cast just to test it */
static bool borg_can_play_spell(borg_magic *as)
{
    /* There are some spells not worth testing */
    /* they can just be used when the borg is ready to use it. */
    switch (as->spell_enum) {
    case LIGHTNING_STRIKE:
    case TAP_UNLIFE:
    case TAP_MAGICAL_ENERGY:
    case REMOVE_CURSE:
    case TREMOR:
    case WORD_OF_DESTRUCTION:
    case GRONDS_BLOW:
    case DECOY:
    case GLYPH_OF_WARDING:
    case SINGLE_COMBAT:
    case VAMPIRE_STRIKE:
    case COMMAND:
    return false;
    default:
        break;
    }

    if (as->effect_index == EF_BRAND_BOLTS)
        return false; // !FIX !TODO !AJG check for a bolt
    if (as->effect_index == EF_CREATE_ARROWS)
        return false; // !FIX !TODO !AJG check for a staff
    if (as->effect_index == EF_BRAND_AMMO)
        return false; // !FIX !TODO !AJG check for ammo
    if (as->effect_index == EF_ENCHANT)
        return false; // !FIX !TODO !AJG check something to enchant
    if (as->effect_index == EF_IDENTIFY)
        return false; // !FIX !TODO !AJG check something to identify
    if (as->effect_index == EF_RECHARGE)
        return false; // !FIX !TODO !AJG check for a wand or rod or staff
    return true;
}

/*
 * Study and/or Test spells/prayers
 */
bool borg_play_magic(bool bored)
{
    int r, b_r = -1;
    int spell_num, b_spell_num;

    /* Hack -- must use magic or prayers */
    if (!player->class->magic.total_spells)
        return false;

    /* Hack -- blind/confused */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        return false;

    /* Dark */
    if (!borg.trait[BI_CURLITE])
        return false;
    if (borg_grids[borg.c.y][borg.c.x].info == BORG_DARK)
        return false;

    /* loop through spells backward */
    for (spell_num = player->class->magic.total_spells - 1; spell_num >= 0;
         spell_num--) {
        borg_magic *as = &borg_magics[spell_num];

        /* Look for the book in inventory*/
        if (borg.book_idx[as->book] < 0)
            continue;

        /* Require "learnable" status */
        if (as->status != BORG_MAGIC_OKAY)
            continue;

        /* Obtain "rating" */
        r = as->rating;

        /* Skip "boring" spells/prayers */
        if (!bored && (r <= 50))
            continue;

        /* Skip "icky" spells/prayers */
        if (r <= 0)
            continue;

        /* Skip "worse" spells/prayers */
        if (r <= b_r)
            continue;

        /* Track it */
        b_r         = r;
        b_spell_num = spell_num;
    }

    /* Study */
    if (borg.trait[BI_ISSTUDY] && (b_r > 0)) {
        borg_magic *as = &borg_magics[b_spell_num];

        /* Debugging Info */
        borg_note(format("# Studying spell/prayer %s.", as->name));

        /* Learn the spell */
        borg_keypress('G');

        /* Specify the book */
        borg_keypress(all_letters_nohjkl[borg.book_idx[as->book]]);

        /* Specify the spell  */
        if (player_has(player, PF_CHOOSE_SPELLS)) {
            /* Specify the spell */
            borg_keypress(all_letters_nohjkl[as->book_offset]);
        }

        /* Success */
        return true;
    }

    /* Hack -- only in town */
    if (borg.trait[BI_CDEPTH] && !borg.munchkin_mode)
        return false;

    /* Hack -- only when bored */
    if (!bored)
        return false;

    /* Check each spell (backwards) */
    for (spell_num = player->class->magic.total_spells - 1; spell_num >= 0;
         spell_num--) {
        borg_magic *as = &borg_magics[spell_num];

        /* No such book */
        if (borg.book_idx[as->book] < 0)
            continue;

        /* Only try "untried" spells/prayers */
        if (as->status != BORG_MAGIC_TEST)
            continue;

        /* some spells can't be "played with" in town */
        if (!borg_can_play_spell(as))
            continue;

        /* Some spells should not be tested in munchkin mode */
        if (borg.munchkin_mode) {
            /* Mage types */
            if (player_has(player, PF_CHOOSE_SPELLS)
                && as->spell_enum == MAGIC_MISSILE)
                continue;

            /* Priest type */
            if (!player_has(player, PF_CHOOSE_SPELLS)
                && as->spell_enum == BLESS)
                continue;
        }

        /* Note */
        borg_note("# Testing untried spell/prayer");

        /* Hack -- Use spell or prayer */
        if (borg_spell(as->spell_enum)) {
            /* Hack -- Allow attack spells */
            /* MEGAHACK -- assume "Random" is shooting.  */
            if (effects[as->effect_index].aim || as->effect_index == EF_RANDOM
                || as->effect_index == EF_TELEPORT_TO) {
                /* Hack -- target self */
                borg_keypress('*');
                borg_keypress('t');
            }

            /* Hack -- Allow spells that require selection of a monster type */
            if (as->effect_index == EF_BANISH) {
                /* Hack -- target Maggot */
                borg_keypress('h');
            }

            /* Success */
            return true;
        }
    }

    /* Nope */
    return false;
}

#endif
