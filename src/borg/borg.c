/**
 * \file borg.c
 * \brief Entry point for borg code.
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

#include "borg.h"

#ifdef ALLOW_BORG

#include "../ui-input.h"

#include "borg-init.h"
#include "borg-io.h"
#include "borg-log.h"
#include "borg-messages.h"
#include "borg-messages-react.h"
#include "borg-reincarnate.h"
#include "borg-think.h"

 /*
  * All the information the borg knows about itself
  */
struct borg_struct borg;

/*
 * Configuration
 */
int *borg_cfg = NULL;

/*
 * Random number generator
 */
uint32_t borg_rand_local = 0;

/*
 * Engine version/date
 */
char borg_engine_date[32] = __DATE__;

/*
 * Step control
 */
uint16_t borg_step = 0;

/*
 * Panel/view offsets
 */
int w_x = 0;                 /* Current panel offset (X) */
int w_y = 0;                 /* Current panel offset (Y) */

 /*
  * Special "inkey_hack" hook.  This is used in ui-input.c and other places
  * to allow keys to come from someplace other than the keyboard
  */
static struct keypress generate_keypress(int flush_first);

/*
 * **START HERE FOR BORG PROCESSING**
 *
 * This routine is what captures control from Angband and feeds back keystrokes
 */
static struct keypress borg_entry_point(int flush_first)
{
    return borg_save_keypress(generate_keypress(flush_first));
}

/*
 * set the entry point for the game.
 */
void borg_update_entrypoint(bool start)
{
    if (start) {
        inkey_hack = borg_entry_point;
    }
    else {
        inkey_hack = NULL;
    }
}

/*
 * Check for borg deactivation or player death.
 */
static bool check_for_deactivate_or_death(struct keypress* key)
{
    /* Deactivate */
    if (!borg.status.active) {
        /* Message */
        borg_note("# Removing keypress hook");

        /* Remove hook */
        inkey_hack = NULL;

        /* Flush keys */
        borg_flush();

        /* Flush */
        flush(0, 0, 0);

        /* put the game back in a state for manual play */
        borg_reset_settings();

        /* Done */
        /* Need to flush the key buffer to change modes */
        key->code = ESCAPE;
        return true;
    }

    /* Handle death */
    if (player->is_dead && !borg.status.respawning) {
        /* Print the map */
        if (borg.trait[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL]
            || strstr(player->died_from, "starvation"))
            borg_write_map(false);

        /* Log death */
        borg_log_death();
        borg_log_death_data();

        /* flush the buffer */
        borg_flush();
        borg_parse(NULL);
        borg_clear_reactions();

        /* Oops  */
        borg_oops("player died");

        /* Useless keypress */
        key->code = KTRL('C');
        return true;
    }

    /* If king, maybe retire. */
    if (borg.trait[BI_KING]) {
        /* Prepare to retire */
        if (borg_cfg[BORG_STOP_KING]) {
            borg_write_map(false);
            borg_oops("retire");
        }
        /* Borg will be respawning */
        if (borg_cfg[BORG_RESPAWN_WINNERS]) {
            borg_write_map(false);
#if 0
            /* Note the score */
            borg_enter_score();
#endif
            /* Write to log and borg.dat */
            borg_log_death();
            borg_log_death_data();

            borg_flush();

            /* respawn */
            borg_force_reincarnate();

            return false;
        }
    }

    /* Allow user to stop the borg on certain levels */
    if (borg.trait[BI_CDEPTH] == borg_cfg[BORG_STOP_DLEVEL]) {
        borg_oops("Auto-stop for user DLevel.");

        /* Useless keypress */
        key->code = KTRL('C');
        return true;

    }

    if (borg.trait[BI_CLEVEL] == borg_cfg[BORG_STOP_CLEVEL]) {
        borg_oops("Auto-stop for user CLevel.");

        /* Useless keypress */
        key->code = KTRL('C');
        return true;
    }

    /* HACK to end all hacks,,, allow the borg to stop if money scumming */
    if (borg.trait[BI_GOLD] > borg_cfg[BORG_MONEY_SCUM_AMOUNT]
        && borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0 && !borg.trait[BI_CDEPTH]
        && !borg_cfg[BORG_SELF_SCUM]) {
        borg_oops("Money Scum complete.");

        /* Useless keypress */
        key->code = KTRL('C');
        return true;
    }

    return false;
}

/*
 * Handle manual abort
 */
static bool user_abort(void)
{
    ui_event ch_evt;

    /* Check for user abort */
    (void)Term_inkey(&ch_evt, false, true);

    if (!borg.in_shop && (((ch_evt.type & EVT_KBRD) && ch_evt.key.code > 0
        && ch_evt.key.code != 10) || ch_evt.type == EVT_DISCONNECT)) {
        /* Oops */
        if (ch_evt.type == EVT_DISCONNECT) {
            borg_oops("terminal disconnect abort");
        }
        else {
            if (ch_evt.key.code >= 32 && ch_evt.key.code <= 126) {
                borg_note(format("# User key press <%lu><%c>",
                    (unsigned long)ch_evt.key.code, (char)ch_evt.key.code));
            }
            else {
                borg_note(format("# User key press <%lu>",
                    (unsigned long)ch_evt.key.code));
            }
            borg_note(format("# Key type was <%d><%c>", ch_evt.type, ch_evt.type));
            borg_oops("user abort");
        }

        return true;
    }

    return false;
}

/*
 * This function lets the Borg "steal" control from the user and
 * return a single keypress.
 *
 * The "util.c" file provides a special "inkey_hack" hook which we use
 * to steal control of the keyboard, using the special function below.
 *
 * Since this function bypasses the code in "inkey()" which "refreshes"
 * the screen whenever the game has to wait for a keypress, the screen
 * will only get refreshed when (1) an option such as "fresh_before"
 * induces regular screen refreshing or (2) various explicit calls to
 * "Term_fresh" are made, such as in the "project()" function.  This
 * has the interesting side effect that the screen is never refreshed
 * while the Borg is browsing stores, checking his inventory/equipment,
 * browsing spell books, checking the current panel, or examining an
 * object, which reduces the "screen flicker" considerably.  :-)
 *
 * The only way that the Borg can be stopped once it is started, unless
 * it dies or encounters an error, is to press any key.  This function
 * checks for real user input on a regular basic, and if any is found,
 * it is flushed, and after completing any actions in progress, this
 * function hook is removed, and control is returned to the user.
 *
 * This function hook automatically removes itself when it realizes that
 * it should no longer be active.  Note that this may take place after
 * the game has asked for the next keypress, but the various "keypress"
 * routines should be able to handle this.
 */
static struct keypress generate_keypress(int flush_first)
{
    keycode_t       borg_ch;

    struct keypress key = { EVT_KBRD, 0, 0 };
    struct loc cursor;

    bool     rand_quick; /* Save system setting */
    uint32_t rand_value; /* Save system setting */

    /* Locate the cursor */
    Term_locate(&cursor.x, &cursor.y);

    /* Refresh the screen */
    Term_fresh();

    /* Analyze the players current stats */
    /* this needs to be done before any check for deactivation */
    /* that might use player stats */
    borg_notice_player();

    if (check_for_deactivate_or_death(&key))
        return key;

    /* Mega-Hack -- flush keys */
    if (flush_first) {
        /* Only flush if needed */
        if (borg_inkey(false) != 0) {
            /* Message */
            borg_note("# Flushing keypress buffer");

            /* Flush keys */
            borg_flush();
        }
    }

    /* get the messages from the top of the screen */
    /* some need to be responded to immediately, like -more- */
    /* others can be queued to be processed later. */
    if (borg_get_messages(&key, cursor))
        return key;

    /* Flush messages */
    borg_parse(NULL);
    borg.dont_react = false;

    /* Check for key on the queue */
    borg_ch = borg_inkey(true);

    /* Use the key if there is one */
    if (borg_ch) {
        key.code = borg_ch;
        return key;
    }

    /* check if the user is stopping the borg */
    if (user_abort()) {
        key.code = ESCAPE;
        return key;
    }

    /* Keep him active in town */
    if (borg.trait[BI_CDEPTH] >= 1)
        borg.in_shop = false;

    /* Don't interrupt our own resting or a repeating command */
    if (player->upkeep->resting || cmd_get_nrepeats() > 0) {
        key.type = EVT_NONE;
        return key;
    }

    /* Allow respawning borgs to update their variables */
    if (borg.status.respawning) {
        if (!player->is_dead) {
            borg_reincarnate_end();
            borg.status.respawning = false;
        } else {
            /* We shouldnt' get here, hit escape till the game is ready */
            key.code = ESCAPE;
            return key;
        }
    }

    /* no longer need to confirm the target */
    borg.targeting = false;

    /* Save the system random info */
    rand_quick = Rand_quick;
    rand_value = Rand_value;

    /* Use the local random info */
    Rand_quick = true;
    Rand_value = borg_rand_local;

    /* Think */
    while (!borg_think()) /* loop */
        ;

    /* Update the status screen */
    borg_display_status();

    /* Save the local random info */
    borg_rand_local = Rand_value;

    /* Restore the system random info */
    Rand_quick = rand_quick;
    Rand_value = rand_value;

    /* Allow stepping to induce a clean cancel */
    if (borg_step && (!--borg_step))
        borg.status.cancel = true;

    /* Check for key */
    borg_ch = borg_inkey(true);

    /* Use the key */
    if (borg_ch) {
        key.code = borg_ch;
        return key;
    }

    /* Oops */
    borg_oops("normal abort");

    key.code = ESCAPE;
    return key;
}

#endif
