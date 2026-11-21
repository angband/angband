/**
 * \file borg-reincarnate.c
 * \brief Resurrect the borg with new stats and class
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

#include "borg-reincarnate.h"

#ifdef ALLOW_BORG

#include "../ui-menu.h"

#include "borg-init.h"
#include "borg-io.h"
#include "borg-messages-react.h"
#include "borg-trait.h"
#include "borg.h"

/*
 * Kill the borg and force a reincarnation.
 */
void borg_force_reincarnate(void)
{
    /* Kill the borg */
    borg_keypress('Q');
    borg_keypress('y');
    borg_keypress('@');

    /* Respawn the borg */
    borg_reincarnate_start();
}


/*
 * Wrap up the reincarnation process
 */
void borg_reincarnate_end(void)
{
    borg.player = player;

    /* Notice the new race and class */
    borg_prepare_race_class_info();

    borg_notice_player();

    /* clear out the old reactions again in case more messages came after */
    /* the borg died */
    borg_clear_reactions();

    /* set the old depth so we know we are on a new level */
    borg.status.old_depth = 128;

    /* Mark savefile as borg cheater */
    if (!(player->noscore & NOSCORE_BORG))
        player->noscore |= NOSCORE_BORG;

    borg_reinit_options();

    /* We need the borg to keep playing after he dies */
    option_set("cheat_live", true);

    /* Message */
    borg_note("# Done respawning");
}

/*
 * Allow the borg to play continuously.  Reset all values,
 */
void borg_reincarnate_start(void)
{
    unsigned int  race;
    unsigned int  class;

    borg.status.respawning = true;

    /* clear out the old reactions */
    borg_clear_reactions();

    borg_note("# Respawning");

    /* New character */
    borg_keypress('n');
    borg_keypress('y');

    /* Not based on old character  */
    borg_keypress('n');

    if (borg_cfg[BORG_RESPAWN_RACE] != -1)
        race = borg_cfg[BORG_RESPAWN_RACE];
    else
        race = randint0(MAX_RACES);
    if (borg_cfg[BORG_RESPAWN_CLASS] != -1)
        class = borg_cfg[BORG_RESPAWN_CLASS];
    else
        class = randint0(MAX_CLASSES);

    /* pick race */
    if (race != player->race->ridx) {
        borg_keypress(all_letters_nohjkl[race]);
    }
    borg_keypress(KC_ENTER);

    /* pick class */
    if (class != player->class->cidx) {
        borg_keypress(all_letters_nohjkl[class]);
    }
    borg_keypress(KC_ENTER);

    /* point based */
    borg_keypress(KC_ENTER);

    /* accept points */
    borg_keypress(KC_ENTER);

    /* Get a random name */
    borg_keypress('*');
    borg_keypress(KC_ENTER);

    /* accept history */
    borg_keypress(KC_ENTER);

    /* continue */
    borg_keypress(KC_ENTER);
}

#endif
