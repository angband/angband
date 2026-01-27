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

#include "../game-world.h"
#include "../ui-input.h"
#include "../ui-keymap.h"

#include "borg-init.h"
#include "borg-io.h"
#include "borg-log.h"
#include "borg-messages-react.h"
#include "borg-messages.h"
#include "borg-think.h"
#include "borg-trait.h"
#include "borg-util.h"

bool borg_cheat_death;

#ifdef BABLOS
extern bool auto_play;
extern bool keep_playing;
#endif
/* bablos */

/*
 * Use a simple internal random number generator
 */
bool     borg_rand_quick; /* Save system setting */
uint32_t borg_rand_value; /* Save system setting */
uint32_t borg_rand_local; /* Save personal setting */

/*
 * Date of the last change
 */
char borg_engine_date[] = __DATE__;

/*
 * Borg settings information, ScreenSaver or continual play mode;
 */
int *borg_cfg;

/*
 * Status variables
 */
bool borg_active; /* Actually active */
bool borg_cancel; /* Being cancelled */
bool borg_save          = false; /* do a save next level */
bool borg_graphics      = false; /* graphics mode */

int16_t old_depth       = 128;
int16_t borg_respawning = 0;

int w_x; /* Current panel offset (X) */
int w_y; /* Current panel offset (Y) */

/*
 * Time variables
 */
int16_t borg_t = 0L; /* Current "time" */
int32_t borg_began; /* When this level began */
int32_t borg_time_town; /* how long it has been since I was in town */
int16_t borg_t_morgoth = 0L; /* Last time I saw Morgoth */

/*
 * Number of turns to (manually) step for (zero means forever)
 */
uint16_t borg_step = 0;

// !FIX double check this comment
/*
 * This file implements the Borg, an "Automatic Angband Player".
 *
 * Use of the Borg requires re-compilation with ALLOW_BORG defined,
 * and with the various "borg*.c" files linked into the executable.
 *
 * The "do_cmd_borg()" function, called when the user hits "^Z", allows
 * the user to interact with the Borg.  You do so by typing "Borg Commands",
 * including 'z' to activate (or re-activate), 'K' to show monsters, 'T' to
 * show objects, 'd' to toggle "demo mode", 'f' to open/shut the "log file",
 * 'i' to display internal flags, etc.  See "do_cmd_borg()" for more info.
 *
 * The first time you enter a Borg command:
 *
 * (1) The various "borg" modules are initialized.
 *
 * (2) Some important "state" information is extracted, including the level
 *     and race/class of the player, and some more initialization is done.
 *
 * (3) Some "historical" information (killed uniques, maximum dungeon depth)
 *     is "stolen" from the game.
 *
 * The Borg is only supposed to "know" what is visible on the screen,
 * which it learns by using the "term.c" screen access function "COLOUR_what()",
 * the cursor location function "COLOUR_locate()", and the cursor visibility
 * extraction function "COLOUR_get_cursor()".
 *
 * The Borg is only supposed to "send" keypresses when the "COLOUR_inkey()"
 * function asks for a keypress, which is accomplished by using a special
 * function hook in the "z-term.c" file, which allows the Borg to "steal"
 * control from the "COLOUR_inkey()" and "COLOUR_flush(0, 0, 0)" functions. This
 * allows the Borg to pretend to be a normal user.
 *
 * The Borg is thus allowed to examine the screen directly (by efficient
 * direct access of the "Term->scr->a" and "Term->scr->c" arrays, which
 * could be replaced by calls to "COLOUR_grab()"), and to access the cursor
 * location (via "COLOUR_locate()") and visibility (via "COLOUR_get_cursor()"),
 * and, as mentioned above, the Borg is allowed to send keypresses directly
 * to the game, and only when needed, using the "COLOUR_inkey_hook" hook, and
 * uses the same hook to know when it should discard all pending keypresses.
 *
 * Note that any "user input" will be ignored, and will cancel the Borg,
 * after the Borg has completed any key-sequences currently in progress.
 *
 * Note that the "borg_t" parameter bears a close resemblance to the number of
 * "player turns" that have gone by.  Except that occasionally, the Borg will
 * do something that he *thinks* will take time but which actually does not
 * (for example, attempting to open a hallucinatory door), and that sometimes,
 * the Borg performs a "repeated" command (rest, open, tunnel, or search),
 * which may actually take longer than a single turn.  This has the effect
 * that the "borg_t" variable is slightly lacking in "precision".  Note that
 * we can store every time-stamp in a 'int16_t', since we reset the clock to
 * 1000 on each new level, and we refuse to stay on any level longer than
 * 30000 turns, unless we are totally stuck, in which case we abort.
 *
 * The Borg assumes that the "maximize" flag is off, and that the
 * "preserve" flag is on, since he cannot actually set those flags.
 * If the "maximize" flag is on, the Borg may not work correctly.
 * If the "preserve" flag is off, the Borg may miss artifacts.
 */

/*
 * saved initialization data to be restored when the borg stops
 */
struct borg_save_init borg_init_save;


static struct keypress internal_borg_inkey(int flush_first);

/*
 * **START HERE FOR BORG PROCESSING**
 *
 * This routine is what captures control from Angband and feeds back keystrokes
 * It wraps the main keypress routine to enable capture of the keys generated
 */
static struct keypress borg_inkey_hack(int flush_first)
{
    return save_keypress_history(internal_borg_inkey(flush_first));
}

/*
 * set the entry point for the game.
 */
void borg_update_entrypoint(bool start)
{
    if (start) {
        inkey_hack = borg_inkey_hack;
    }
    else {
        inkey_hack = NULL;
    }
}

/*
 * This function lets the Borg "steal" control from the user.
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
 * We handle "broken" messages, in which long messages are "broken" into
 * pieces, and all but the first message are "indented" by one space, by
 * collecting all the pieces into a complete message and then parsing the
 * message once it is known to be complete.
 *
 * This function hook automatically removes itself when it realizes that
 * it should no longer be active.  Note that this may take place after
 * the game has asked for the next keypress, but the various "keypress"
 * routines should be able to handle this.
 */
static struct keypress internal_borg_inkey(int flush_first)
{
    keycode_t       borg_ch;
    struct keypress key = { EVT_KBRD, 0, 0 };

    ui_event ch_evt;

    int y = 0;
    int x = ((Term->wid /* - (COL_MAP)*/ - 1) / (tile_width));

    uint8_t t_a;

    char buffer[1024];
    char *buf = buffer;

    bool borg_prompt; /* For now we can just use this locally.
                          in the 283 borg he uses this to optimize knowing if
                          we are waiting at a prompt for info */
    /* Locate the cursor */
    (void)Term_locate(&x, &y);

    /* Refresh the screen */
    Term_fresh();

    /* Deactivate */
    if (!borg_active) {
        /* Message */
        borg_note("# Removing keypress hook");

        /* Remove hook */
        inkey_hack = NULL;

        /* Flush keys */
        borg_flush();

        /* Flush */
        flush(0, 0, 0);

        /* Restore user key mode */
        if (borg_init_save.key_mode == KEYMAP_MODE_ROGUE) {
            option_set("rogue_like_commands", true);
        } else if (borg_init_save.key_mode == KEYMAP_MODE_ORIG) {
            option_set("rogue_like_commands", false);
        }

        borg_reset_ignore();

        /* Done */
        /* Need to flush the key buffer to change modes */
        key.type = EVT_KBRD;
        key.code = ESCAPE;
        return key;
    }

    /* Mega-Hack -- flush keys */
    if (flush_first) {
        /* Only flush if needed */
        if (borg_inkey(false) != 0) {
            /* Message */
            borg_note("# Flushing keypress buffer");

            /* Flush keys */
            borg_flush();

            /* Cycle a few times to catch up if needed */
            if (borg.time_this_panel > 250) {
                borg_respawning = 3;
            }
        }
    }

    /* Assume no prompt/message is available */
    borg_prompt = false;

    /* due to changes in the way messages are handled sometimes the code */
    /* seems to be getting blanks before the message or blanks then -more- */
    /* trying to see if I can code around this. */

    /* get everything on the message line */
    buf = buffer;
    borg_what_text(0, 0, ((Term->wid - 1) / (tile_width)), &t_a, buffer);
#if 0
    /* just used for debugging.  Not so useful in general */
    if (borg_cfg[BORG_VERBOSE])
        borg_note(format("got message '%s'", buf));
#endif
    /* Trim whitespace */
    buf = borg_trim(buf);

    /* Mega-Hack -- check for possible prompts/messages */
    /* If the first four characters on the message line all */
    /* have the same attribute (or are all spaces), and they */
    /* are not all spaces (ascii value 0x20)... */
    if ((t_a != COLOUR_DARK)
        && (buf[0] != ' ' || buf[1] != ' ' || buf[2] != ' ' || buf[3] != ' ')) {
        /* Assume a prompt/message is available */
        borg_prompt = true;
    }

    if (borg_prompt && prefix(buf, "Type")) {
        borg_prompt = false;
    }

    /* handle the messages the borg has to react to immediately */
    if (borg_prompt && !inkey_flag && strlen(buf)) {
        if (borg_react_prompted(buf, &key, x, y))
            return key;
    }

    /* Mega-Hack -- Handle death */
    if (player->is_dead) {
#ifndef BABLOS
        /* Print the map */
        if (borg.trait[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL]
            || strstr(player->died_from, "starvation"))
            borg_write_map(false);

        /* Log death */
        borg_log_death();
        borg_log_death_data();
#if 0
        /* Note the score */
        borg_enter_score();
#endif
#endif /* bablos */
        /* flush the buffer */
        borg_flush();
        borg_parse(NULL);
        borg_clear_reactions();

        /* Oops  */
        borg_oops("player died");

        /* Useless keypress */
        key.code = KTRL('C');
        return key;
    }

    /* Mega-Hack -- Catch "-more-" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And there is text before the cursor... */
    /* And that text is "-more-" */
    buf = buffer;
    if (borg_prompt && !inkey_flag && (y == 0) && (x >= 7)
        && (0 == borg_what_text(x - 7, y, 7, &t_a, buffer))
        && (suffix(buf, " -more-"))) {

        if (borg_cfg[BORG_VERBOSE])
            borg_note("# message with -more-");

        /* Get the message */
        if (0 == borg_what_text(0, 0, x - 7, &t_a, buffer)) {
            /* Parse it */
            borg_parse(buf);
        }
        /* Clear the message */
        if (borg_cfg[BORG_VERBOSE])
            borg_note("clearing -more-");
        key.code = ' ';
        return key;
    }

    /* in the odd case where a we get here before the message */
    /* about cheating death comes up.  */
    if (!character_dungeon) {
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# Mid reincarnation, no map yet");
        /* do nothing */
        key.code = KC_ENTER;

        /* there is an odd case I can't track down where the borg */
        /* tries to respawn but gets caught in a loop. */
        borg_respawning--;
        if (borg_respawning <= 0)
            borg_oops("reincarnation failure");

        return key;
    }

    /* Mega-Hack -- catch normal messages */
    /* If there is text on the first line... */
    /* And the game wants a command */
    if (borg_prompt && inkey_flag) {
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# parse normal message");
        /* Get the message(s) */
        buf = buffer;
        if (0
            == borg_what_text(
                0, 0, ((Term->wid - 1) / (tile_width)), &t_a, buffer)) {
            int k = strlen(buf);

            /* Strip trailing spaces */
            while ((k > 0) && (buf[k - 1] == ' '))
                k--;

            /* Terminate */
            buf[k] = '\0';

            /* Parse it */
            borg_parse(buf);
        }

        /* Clear the message */
        key.code = ' ';
        return key;
    }
    /* Flush messages */
    borg_parse(NULL);
    borg_dont_react = false;

    /* Check for key */
    borg_ch = borg_inkey(true);

    /* Use the key */
    if (borg_ch) {
        key.code = borg_ch;
        return key;
    }

    /* Check for user abort */
    (void)Term_inkey(&ch_evt, false, true);

    /* Keep him active in town */
    if (borg.trait[BI_CDEPTH] >= 1)
        borg.in_shop = false;

    if (!borg.in_shop && (ch_evt.type & EVT_KBRD) && ch_evt.key.code > 0
        && ch_evt.key.code != 10) {
        /* Oops */
        if (ch_evt.key.code >= 32 && ch_evt.key.code <= 126) {
            borg_note(format("# User key press <%lu><%c>",
                (unsigned long)ch_evt.key.code, (char)ch_evt.key.code));
        } else {
            borg_note(format("# User key press <%lu>",
                (unsigned long)ch_evt.key.code));
        }
        borg_note(format("# Key type was <%d><%c>", ch_evt.type, ch_evt.type));
        borg_oops("user abort");

        key.code = ESCAPE;
        return key;
    }

    /* for some reason, selling and buying in the store sets the event handler
     * to Select. */
    if (ch_evt.type & EVT_SELECT)
        ch_evt.type = EVT_KBRD;
    if (ch_evt.type & EVT_MOVE)
        ch_evt.type = EVT_KBRD;

    /* Don't interrupt our own resting or a repeating command */
    if (player->upkeep->resting || cmd_get_nrepeats() > 0) {
        key.type = EVT_NONE;
        return key;
    }

    /* done with buffered and repeated commands, the confirm should be done*/
    borg_confirm_target = false;

    /* Save the system random info */
    borg_rand_quick = Rand_quick;
    borg_rand_value = Rand_value;

    /* Use the local random info */
    Rand_quick = true;
    Rand_value = borg_rand_local;

    /* Think */
    while (!borg_think()) /* loop */
        ;

    /* Update the status screen */
    borg_status();

    /* Save the local random info */
    borg_rand_local = Rand_value;

    /* Restore the system random info */
    Rand_quick = borg_rand_quick;
    Rand_value = borg_rand_value;

    /* Allow stepping to induce a clean cancel */
    if (borg_step && (!--borg_step))
        borg_cancel = true;

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
