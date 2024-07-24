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

#include "../cmds.h"
#include "../game-input.h"
#include "../game-world.h"
#include "../player-timed.h"
#include "../project.h"
#include "../ui-input.h"
#include "../ui-keymap.h"
#include "../ui-map.h"

#include "borg-cave-util.h"
#include "borg-cave-view.h"
#include "borg-danger.h"
#include "borg-flow-glyph.h"
#include "borg-flow-kill.h"
#include "borg-flow-take.h"
#include "borg-flow.h"
#include "borg-home-notice.h"
#include "borg-home-power.h"
#include "borg-init.h"
#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-id.h"
#include "borg-log.h"
#include "borg-magic.h"
#include "borg-messages-react.h"
#include "borg-messages.h"
#include "borg-power.h"
#include "borg-prepared.h"
#include "borg-projection.h"
#include "borg-reincarnate.h"
#include "borg-store.h"
#include "borg-think.h"
#include "borg-trait-swap.h"
#include "borg-trait.h"
#include "borg-update.h"
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
bool borg_flag_save     = false; /* Save savefile at each level */
bool borg_save          = false; /* do a save next level */
bool borg_graphics      = false; /* rr9's graphics */

int16_t old_depth       = 128;
int16_t borg_respawning = 0;

int w_x; /* Current panel offset (X) */
int w_y; /* Current panel offset (Y) */

/*
 * Hack -- Time variables
 */
int16_t borg_t = 0L; /* Current "time" */
int32_t borg_began; /* When this level began */
int32_t borg_time_town; /* how long it has been since I was in town */
int16_t borg_t_morgoth = 0L; /* Last time I saw Morgoth */

/*
 * Number of turns to (manually) step for (zero means forever)
 */
uint16_t borg_step = 0;

// !FIX !AJG double check this comment
/*
 * This file implements the "Ben Borg", an "Automatic Angband Player".
 *
 * Use of the "Ben Borg" requires re-compilation with ALLOW_BORG defined,
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
 * The Ben Borg is only supposed to "know" what is visible on the screen,
 * which it learns by using the "term.c" screen access function "COLOUR_what()",
 * the cursor location function "COLOUR_locate()", and the cursor visibility
 * extraction function "COLOUR_get_cursor()".
 *
 * The Ben Borg is only supposed to "send" keypresses when the "COLOUR_inkey()"
 * function asks for a keypress, which is accomplished by using a special
 * function hook in the "z-term.c" file, which allows the Borg to "steal"
 * control from the "COLOUR_inkey()" and "COLOUR_flush(0, 0, 0)" functions. This
 * allows the Ben Borg to pretend to be a normal user.
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
 * KEYMAP_MODE_ROGUE or KEYMAP_MODE_ORIG
 */
int key_mode;


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

    bool borg_prompt; /* ajg  For now we can just use this locally.
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
        if (key_mode == KEYMAP_MODE_ROGUE) {
            option_set("rogue_like_commands", true);
        } else if (key_mode == KEYMAP_MODE_ORIG) {
            option_set("rogue_like_commands", false);
        }

        /* Done */
        /* HACK need to flush the key buffer to change modes */
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

    /* Mega-Hack -- Catch "Die? [y/n]" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And the text acquired above is "Die?" */
    if (borg_prompt && !inkey_flag && (y == 0) && (x >= 4)
        && prefix(buf, "Die?")
        && borg_cheat_death) {
        /* Flush messages */
        borg_parse(NULL);

        /* flush the buffer */
        borg_flush();

        /* Take note */
        borg_note("# Cheating death...");

#ifndef BABLOS
        /* Dump the Character Map*/
        if (borg.trait[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL]
            || strstr(player->died_from, "starvation"))
            borg_write_map(false);

        /* Log the death */
        borg_log_death();
        borg_log_death_data();

#if 0
        /* Note the score */
        borg_enter_score();
#endif

        if (!borg_cfg[BORG_CHEAT_DEATH]) {
            reincarnate_borg();
            borg_respawning = 7;
        } else
            do_cmd_wiz_cure_all(0);
#endif /* BABLOS */

        key.code = 'n';
        return key;
    }

    /* with 292, there is a flush(0, 0, 0) introduced as it asks for
     * confirmation. This flush is messing up the borg.  This will allow the
     * borg to work around the flush Attempt to catch "Attempt it anyway? [y/n]"
     */
    if (borg_prompt && !inkey_flag && (y == 0) && (x >= 4)
        && prefix(buf, "Atte")) {
        /* Return the confirmation */
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# Confirming use of Spell/Prayer when low on mana.");
        key.code = 'y';
        return key;
    }

    /* Wearing two rings.  Place this on the left hand */
    if (borg_prompt && !inkey_flag && (y == 0) && (x >= 12)
        && (prefix(buf, "(Equip: c-d,"))) {
        /* Left hand */
        key.code = 'c';
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# Putting ring on the left hand.");
        return key;
    }

    /* 
     * with 292, there is a flush(0, 0, 0) introduced as it asks for
     * confirmation. This flush is messing up the borg.  This will allow the
     * borg to work around the flush This is used only with emergency use of
     * spells like Magic Missile Attempt to catch "Direction (5 old target"
     */
    if (borg_prompt && !inkey_flag && (y == 0) && !borg_inkey(false)
        && (x >= 10) && strncmp(buf, "Direction", 9) == 0) {
        if (borg_confirm_target) {
            if (borg_cfg[BORG_VERBOSE])
                borg_note("# Expected request for Direction.");
            /* reset the flag */
            borg_confirm_target = false;
            /* Return queued target */
            key.code = borg_get_queued_direction();
            return key;
        } else {
            borg_note("** UNEXPECTED REQUEST FOR DIRECTION Dumping keypress history ***");
            borg_note(format("** line starting <%s> ***", buf));
            borg_dump_recent_keys(20);
            borg_oops("unexpected request for direction");
            /* Hack -- Escape */
            key.code = ESCAPE;
            return key;
        }
    }

    /* Stepping on a stack when the inventory is full gives a message */
    /* and the keypress when given this message is requeued so the borg */
    /* thinks it is a user keypress when it isn't */
    if (borg_prompt && !inkey_flag && (y == 0) && (x >= 12)
        && (prefix(buf, "You have no room"))) {
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# 'You have no room' is a no-op");
        /* key code 0 seems to be a no-op and ignored as a user keypress */
        key.code = 0;
        return key;
    }

    /* ***MEGA-HACK***  */
    /* This will be hit if the borg uses an unidentified effect that has */
    /* EF_SELECT/multiple effects. Always pick "one of the following at random"
     */
    /* when the item is used post ID, an effect will be selected */
    if (borg_prompt && !inkey_flag && !borg_inkey(false) && (y == 1)
        && prefix(buf, "Which effect?")) {
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# Use of unknown object with multiple effects");
        /* the first selection (a) is random */
        key.code = 'a';
        return key;
    }

    /* prompt for stepping in lava.  This should be avoided but */
    /* if the borg is stuck, give him a pass */
    if (borg_prompt && !inkey_flag && (y == 0) && (x >= 12)
        && (prefix(buf, "The lava will") || prefix(buf, "Lava blocks y"))) {
        if (borg_cfg[BORG_VERBOSE])
            borg_note("# ignoring Lava warning");
        /* yes step in */
        key.code = 'y';
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

    /* Hack to keep him active in town. */
    if (borg.trait[BI_CDEPTH] >= 1)
        borg.in_shop = false;

    if (!borg.in_shop && (ch_evt.type & EVT_KBRD) && ch_evt.key.code > 0
        && ch_evt.key.code != 10) {
        /* Oops */
        borg_note(format(
            "# User key press <%d><%c>", ch_evt.key.code, ch_evt.key.code));
        borg_note(format("# Key type was <%d><%c>", ch_evt.type, ch_evt.type));
        borg_oops("user abort");

        /* Hack -- Escape */
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

    /* DVE- Update the status screen */
    borg_status();

    /* Save the local random info */
    borg_rand_local = Rand_value;

    /* Restore the system random info */
    Rand_quick = borg_rand_quick;
    Rand_value = borg_rand_value;

    /* Hack -- allow stepping to induce a clean cancel */
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

    /* Hack -- Escape */
    key.code = ESCAPE;
    return key;
}

/* wrapper around keypress capture */
static struct keypress borg_inkey_hack(int flush_first)
{
    struct keypress k = internal_borg_inkey(flush_first);

    save_keypress_history(&k);

    return k;
}


/*
 * Hack -- interact with the "Ben Borg".
 */
void do_cmd_borg(void)
{
    char cmd;

#ifdef BABLOS
    if (auto_play) {
        auto_play    = false;
        keep_playing = true;
        cmd.code     = 'z';
    } else {

#endif /* BABLOS */

        /* Get a "Borg command", or abort */
#ifdef XSCREENSAVER
        if (auto_start_borg == false)
#endif
        {
            if (!get_com("Borg command: ", &cmd))
                return;
        }

#ifdef BABLOS
    }

#endif /* BABLOS */

    /* *HACK* set the player location */
    borg.c = player->grid;

    /* Simple help */
    if (cmd == '?') {
        int i = 2;

        /* Save the screen */
        Term_save();

        /* Clear the screen */
        Term_clear();

        i++;
        Term_putstr(2, i, -1, COLOUR_WHITE, "Command 'z' activates the Borg.");
        Term_putstr(42, i++, -1, COLOUR_WHITE, "Command 'u' updates the Borg.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'x' steps the Borg.");
        Term_putstr(
            42, i, -1, COLOUR_WHITE, "Command 'f' modifies the normal flags.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command 'c' modifies the cheat flags.");
        Term_putstr(
            42, i, -1, COLOUR_WHITE, "Command 'l' activates a log file.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command 's' activates search mode.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'i' displays grid info.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command 'g' displays grid feature.");
        Term_putstr(
            42, i, -1, COLOUR_WHITE, "Command 'a' displays avoidances.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command 'k' displays monster info.");
        Term_putstr(
            42, i, -1, COLOUR_WHITE, "Command 't' displays object info.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command '%' displays targeting flow.");
        Term_putstr(
            42, i, -1, COLOUR_WHITE, "Command '#' displays danger grid.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command '_' Regional Fear info.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'p' Borg Power.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '1' change max depth.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command '2' level prep info.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '3' Feature of grid.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command '!' Time.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '@' Borg LOS.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'w' My Swap Weapon.");
        Term_putstr(
            2, i++, -1, COLOUR_WHITE, "Command 'q' Auto stop on level.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'v' Version stamp.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'd' Dump spell info.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'h' Borg_Has function.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '$' Reload Borg.txt.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'y' Last 75 steps.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'm' money Scum.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command '^' Flow Pathway.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'R' Respawn Borg.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'o' Object Flags.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'r' Restock Stores.");

        /* Prompt for key */
        msg("Commands: ");
        inkey();

        /* Restore the screen */
        Term_load();

        /* Done */
        return;
    }

    /*
     * Hack -- force initialization or reinitialize if the game was closed
     * and restarted without exiting since the last initialization
     */
    if (!borg_initialized || game_closed) {
        if (borg_initialized) {
            borg_free();
        }
        borg_init();

        if (borg_init_failure) {
            borg_initialized = false;
            borg_free();
            borg_note("** startup failure borg cannot run ** ");
            Term_fresh();
            return;
        }
    }

    switch (cmd) {
        /* Command: Nothing */
    case '$': {
        /*** Hack -- initialize borg.ini options ***/
        borg_init_txt_file();
        break;
    }
    /* Command: Activate */
    case 'z':
    case 'Z': {
        /* make sure the important game options are set correctly */
        borg_reinit_options();

        /* Activate */
        borg_active = true;

        /* Reset cancel */
        borg_cancel = false;

        /* Step forever */
        borg_step = 0;

        borg_notice_player();

        if (player->opts.lazymove_delay != 0) {
            borg_note("# Turning off lazy movement controls");
            player->opts.lazymove_delay = 0;
        }

        /* Message */
        borg_note("# Installing keypress hook");

        /* If the clock overflowed, fix that  */
        if (borg_t > 9000)
            borg_t = 9000;

        /* Activate the key stealer */
        inkey_hack = borg_inkey_hack;

        break;
    }

    /* Command: Update */
    case 'u':
    case 'U': {
        /* make sure the important game options are set correctly */
        borg_reinit_options();

        /* Activate */
        borg_active = true;

        /* Immediate cancel */
        borg_cancel = true;

        /* Step forever */
        borg_step = 0;

        borg_notice_player();

        /* Message */
        borg_note("# Installing keypress hook");

        /* Activate the key stealer */
        inkey_hack = borg_inkey_hack;

        break;
    }

    /* Command: Step */
    case 'x':
    case 'X': {
        /* make sure the important game options are set correctly */
        borg_reinit_options();

        /* Activate */
        borg_active = true;

        /* Reset cancel */
        borg_cancel = false;

        /* Step N times */
        borg_step = get_quantity("Step how many times? ", 1000);
        if (borg_step < 1)
            borg_step = 1;

        borg_notice_player();

        /* Message */
        borg_note("# Installing keypress hook");
        borg_note(format("# Stepping Borg %d times", borg_step));

        /* If the clock overflowed, fix that  */
        if (borg_t > 9000)
            borg_t = 9000;

        /* Activate the key stealer */
        inkey_hack = borg_inkey_hack;

        break;
    }

    /* Command: toggle "flags" */
    case 'f':
    case 'F': {
        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Toggle Flag: (m/d/s/f/g) ", &cmd))
            return;

        switch (cmd) {
            /* Give borg thought messages in window */
        case 'm':
        case 'M': {
            msg("Command No Longer Useful");
            break;
        }

        /* Give borg the ability to use graphics ----broken */
        case 'g':
        case 'G': {
            borg_graphics = !borg_graphics;
            msg("Borg -- borg_graphics is now %d.", borg_graphics);
            break;
        }

        /* Dump savefile at each level */
        case 's':
        case 'S': {
            borg_flag_save = !borg_flag_save;
            msg("Borg -- borg_flag_save is now %d.", borg_flag_save);
            break;
        }

        /* clear 'fear' levels */
        case 'f':
        case 'F': {
            msg("Command No Longer Useful");
            break;
        }
        }
        break;
    }

    /* Command: toggle "cheat" flags */
    case 'c': {
        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Toggle Cheat: (d)", &cmd))
            return;

        switch (cmd) {
        case 'd':
        case 'D': {
            borg_cheat_death = !borg_cheat_death;
            msg("Borg -- borg_cheat_death is now %d.", borg_cheat_death);
            break;
        }
        }
        break;
    }

    /* List the Nasties on the level */
    case 'C': {
        int i;

        /* Log Header */
        borg_note("Borg Nasties Count");

        /* Find the numerous nasty in order of nastiness */
        for (i = 0; i < borg_nasties_num; i++) {
            borg_note(format("Nasty: [%c] Count: %d, limited: %d",
                borg_nasties[i], borg_nasties_count[i], borg_nasties_limit[i]));
        }

        /* Done */
        msg("Borg Nasty Dump Complete.  Examine Log.");
        break;
    }

    /* Activate a search string */
    case 's':
    case 'S': {
        /* Get the new search string (or cancel the matching) */
        if (!get_string("Borg Match String: ", borg_match, 70)) {
            /* Cancel it */
            my_strcpy(borg_match, "", sizeof(borg_match));

            /* Message */
            msg("Borg Match String de-activated.");
        }
        break;
    }

    /* Command: check Grid "feature" flags */
    case 'g': {
        int x, y;

        uint16_t low, high = 0;
        bool     trap  = false;
        bool     glyph = false;

        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Show grids: ", &cmd))
            return;

        /* Extract a flag */
        switch (cmd) {
        case '0':
            low = high = 1 << 0;
            break;
        case '1':
            low = high = 1 << 1;
            break;
        case '2':
            low = high = 1 << 2;
            break;
        case '3':
            low = high = 1 << 3;
            break;
        case '4':
            low = high = 1 << 4;
            break;
        case '5':
            low = high = 1 << 5;
            break;
        case '6':
            low = high = 1 << 6;
            break;
        case '7':
            low = high = 1 << 7;
            break;

        case '.':
            low = high = FEAT_FLOOR;
            break;
        case ' ':
            low = high = FEAT_NONE;
            break;
        case ';':
            low = high = -1;
            glyph      = true;
            break;
        case ',':
            low = high = FEAT_OPEN;
            break;
        case 'x':
            low = high = FEAT_BROKEN;
            break;
        case '<':
            low = high = FEAT_LESS;
            break;
        case '>':
            low = high = FEAT_MORE;
            break;
        case '@':
            low  = FEAT_STORE_GENERAL;
            high = FEAT_HOME;
            break;
        case '^':
            low = high = -1;
            glyph      = true;
            break;
        case '+':
            low  = FEAT_CLOSED;
            high = FEAT_CLOSED;
            break;
        case 's':
            low = high = FEAT_SECRET;
            break;
        case ':':
            low = high = FEAT_RUBBLE;
            break;
        case 'm':
            low = high = FEAT_MAGMA;
            break;
        case 'q':
            low = high = FEAT_QUARTZ;
            break;
        case 'k':
            low = high = FEAT_MAGMA_K;
            break;
        case '&':
            low = high = FEAT_QUARTZ_K;
            break;
        case 'w':
            low  = FEAT_GRANITE;
            high = FEAT_GRANITE;
            break;
        case 'p':
            low  = FEAT_PERM;
            high = FEAT_PERM;
            break;

        default:
            low = high = 0x00;
            break;
        }

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
            for (x = 1; x <= AUTO_MAX_X - 1; x++) {
                uint8_t a     = COLOUR_RED;

                borg_grid *ag = &borg_grids[y][x];

                /* show only those grids */
                if (!trap && !glyph) {
                    if (!(ag->feat >= low && ag->feat <= high))
                        continue;
                } else {
                    if (!((ag->glyph && glyph) || (ag->trap && trap)))
                        continue;
                }

                /* Color */
                if (borg_cave_floor_bold(y, x))
                    a = COLOUR_YELLOW;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("Press any key.");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Display Feature of a targetted grid */
    case 'G': {
        int y = 1;
        int x = 1;

        uint16_t mask;

        mask = borg_grids[y][x].feat;

        struct loc l;

        target_get(&l);
        y            = l.y;
        x            = l.x;

        uint8_t feat = square(cave, borg.c)->feat;

        borg_note(format("Borg's Feat for grid (%d, %d) is %d, game Feat is %d",
            y, x, mask, feat));
        prt_map();
        break;
    }

    /* Command: check "info" flags */
    case 'i': {
        int x, y;

        uint16_t mask;

        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Show grids: ", &cmd))
            return;

        /* Extract a flag */
        switch (cmd) {
        case '0':
            mask = 1 << 0;
            break; /* Mark */
        case '1':
            mask = 1 << 1;
            break; /* Glow */
        case '2':
            mask = 1 << 2;
            break; /* Dark */
        case '3':
            mask = 1 << 3;
            break; /* Okay */
        case '4':
            mask = 1 << 4;
            break; /* Lite */
        case '5':
            mask = 1 << 5;
            break; /* View */
        case '6':
            mask = 1 << 6;
            break; /* Temp */
        case '7':
            mask = 1 << 7;
            break; /* Xtra */

        case 'm':
            mask = BORG_MARK;
            break;
        case 'g':
            mask = BORG_GLOW;
            break;
        case 'd':
            mask = BORG_DARK;
            break;
        case 'o':
            mask = BORG_OKAY;
            break;
        case 'l':
            mask = BORG_LIGHT;
            break;
        case 'v':
            mask = BORG_VIEW;
            break;
        case 't':
            mask = BORG_TEMP;
            break;
        case 'x':
            mask = BORG_XTRA;
            break;

        default:
            mask = 0x000;
            break;
        }

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
            for (x = 1; x <= AUTO_MAX_X - 1; x++) {
                uint8_t a     = COLOUR_RED;

                borg_grid *ag = &borg_grids[y][x];

                /* Given mask, show only those grids */
                if (mask && !(ag->info & mask))
                    continue;

                /* Given no mask, show unknown grids */
                if (!mask && (ag->info & BORG_MARK))
                    continue;

                /* Color */
                if (borg_cave_floor_bold(y, x))
                    a = COLOUR_YELLOW;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("Press any key.");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Display Info of a targetted grid */
    case 'I': {
        int        i;
        int        y = 1;
        int        x = 1;
        struct loc l;

        target_get(&l);
        y = l.y;
        x = l.x;

        if (borg_grids[y][x].info & BORG_MARK)
            msg("Borg Info for grid (%d, %d) is MARK", y, x);
        if (borg_grids[y][x].info & BORG_GLOW)
            msg("Borg Info for grid (%d, %d) is GLOW", y, x);
        if (borg_grids[y][x].info & BORG_DARK)
            msg("Borg Info for grid (%d, %d) is DARK", y, x);
        if (borg_grids[y][x].info & BORG_OKAY)
            msg("Borg Info for grid (%d, %d) is OKAY", y, x);
        if (borg_grids[y][x].info & BORG_LIGHT)
            msg("Borg Info for grid (%d, %d) is LITE", y, x);
        if (borg_grids[y][x].info & BORG_VIEW)
            msg("Borg Info for grid (%d, %d) is VIEW", y, x);
        if (borg_grids[y][x].info & BORG_TEMP)
            msg("Borg Info for grid (%d, %d) is TEMP", y, x);
        if (borg_grids[y][x].info & BORG_XTRA)
            msg("Borg Info for grid (%d, %d) is XTRA", y, x);

        for (i = 0; i < SQUARE_MAX; i++)
            if (sqinfo_has(square(cave, l)->info, i))
                msg(format("Sys Info for grid (%d, %d) is %d", y, x, i));
        prt_map();
        break;
    }

    /* Command: check "avoidances" */
    case 'a':
    case 'A': {
        int x, y, p;

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
            for (x = 1; x <= AUTO_MAX_X - 1; x++) {
                uint8_t a = COLOUR_RED;

                /* Obtain danger */
                p = borg_danger(y, x, 1, true, false);

                /* Skip non-avoidances */
                if (p < avoidance / 10)
                    continue;

                /* Use colors for less painful */
                if (p < avoidance / 2)
                    a = COLOUR_ORANGE;
                if (p < avoidance / 4)
                    a = COLOUR_YELLOW;
                if (p < avoidance / 6)
                    a = COLOUR_GREEN;
                if (p < avoidance / 8)
                    a = COLOUR_BLUE;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("(%d,%d of %d,%d) Avoidance value %d.", borg.c.y, borg.c.x,
            Term->offset_y / borg_panel_hgt(),
            Term->offset_x / borg_panel_wid(), avoidance);
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: check previous steps */
    case 'y': {
        int i;

        /* Scan map */
        uint8_t a = COLOUR_RED;
        /* Check for an existing step */
        for (i = 0; i < track_step.num; i++) {
            /* Display */
            print_rel('*', a, track_step.y[track_step.num - i],
                track_step.x[track_step.num - i]);
            msg("(-%d) Steps noted %d,%d", i, track_step.y[track_step.num - i],
                track_step.x[track_step.num - i]);
            event_signal(EVENT_MESSAGE_FLUSH);
            print_rel('*', COLOUR_ORANGE, track_step.y[track_step.num - i],
                track_step.x[track_step.num - i]);
        }
        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: show "monsters" */
    case 'k':
    case 'K': {
        int i, n = 0;

        /* Scan the monsters */
        for (i = 1; i < borg_kills_nxt; i++) {
            borg_kill *kill = &borg_kills[i];

            /* Still alive */
            if (kill->r_idx) {
                int x = kill->pos.x;
                int y = kill->pos.y;

                /* Display */
                print_rel('*', COLOUR_RED, y, x);

                /* Count */
                n++;
            }
        }

        /* Get keypress */
        msg("There are %d known monsters.", n);
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: show "objects" */
    case 't':
    case 'T': {
        int i, n = 0;

        /* Scan the objects */
        for (i = 1; i < borg_takes_nxt; i++) {
            borg_take *take = &borg_takes[i];

            /* Still alive */
            if (take->kind) {
                int x = take->x;
                int y = take->y;

                /* Display */
                print_rel('*', COLOUR_RED, y, x);

                /* Count */
                n++;
            }
        }

        /* Get keypress */
        msg("There are %d known objects.", n);
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: debug -- current target flow */
    case '%': {
        int        x, y;
        int        n_x;
        int        n_y;
        struct loc l;

        uint8_t svDelay = player->opts.delay_factor;
        player->opts.delay_factor = 200;

        /* Determine "path" */
        n_x = borg.c.x;
        n_y = borg.c.y;
        target_get(&l);
        x = l.x;
        y = l.y;

        /* Borg's pathway */
        while (1) {

            /* Display */
            print_rel('*', COLOUR_RED, n_y, n_x);

            if (n_x == x && n_y == y)
                break;

            /* Calculate the new location */
            borg_inc_motion(&n_y, &n_x, borg.c.y, borg.c.x, y, x);
        }

        msg("Borg's Targeting Path");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Determine "path" */
        n_x = borg.c.x;
        n_y = borg.c.y;
        x   = l.x;
        y   = l.y;

        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Show Arc (Y/y): ", &cmd))
            return;

        msg("Actual Targeting Path");
        event_signal(EVENT_MESSAGE_FLUSH);

        if (cmd != 'Y' && cmd != 'y')
            /* Real LOS - beam*/
            project(source_player(), 0, loc(x, y), 1, PROJ_MISSILE, PROJECT_BEAM, 0,
                0, NULL);
        else
            /* Real LOS - arc */
            project(source_player(), 10, loc(x, y), 50, PROJ_MISSILE, PROJECT_ARC, 60,
                4, NULL);

        player->opts.delay_factor = svDelay;

        /* Redraw map */
        prt_map();

        break;
    }
    /* Display the intended path to the flow */
    case '^': {
        int x, y;
        int o;
        int false_y, false_x;

        false_y = borg.c.y;
        false_x = borg.c.x;

        /* Continue */
        for (o = 0; o < 250; o++) {
            int b_n = 0;

            int i, b_i = -1;

            int c, b_c;

            /* Flow cost of current grid */
            b_c = borg_data_flow->data[borg.c.y][borg.c.x] * 10;

            /* Prevent loops */
            b_c = b_c - 5;

            /* Look around */
            for (i = 0; i < 8; i++) {
                /* Grid in that direction */
                x = false_x + ddx_ddd[i];
                y = false_y + ddy_ddd[i];

                /* Flow cost at that grid */
                c = borg_data_flow->data[y][x] * 10;

                /* Never backtrack */
                if (c > b_c)
                    continue;

                /* Notice new best value */
                if (c < b_c)
                    b_n = 0;

                /* Apply the randomizer to equivalent values */
                if ((++b_n >= 2) && (randint0(b_n) != 0))
                    continue;

                /* Track it */
                b_i = i;
                b_c = c;
            }

            /* Try it */
            if (b_i >= 0) {
                /* Access the location */
                x = false_x + ddx_ddd[b_i];
                y = false_y + ddy_ddd[b_i];

                /* Display */
                print_rel('*', COLOUR_RED, y, x);

                /* Simulate motion */
                false_y = y;
                false_x = x;
            }
        }
        print_rel('*', COLOUR_YELLOW, borg_flow_y[0], borg_flow_x[0]);
        msg("Probable Flow Path");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: debug -- danger of grid */
    case '#': {
        int        n;
        struct loc l;

        target_get(&l);

        /* Turns */
        n = get_quantity("Quantity: ", 10);

        /* Danger of grid */
        msg("Danger(%d,%d,%d) is %d", l.x, l.y, n,
            borg_danger(l.y, l.x, n, true, false));
        break;
    }

    /* Command: Regional Fear Info*/
    case '_': {
        int x, y, p;

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++) {
            for (x = 1; x <= AUTO_MAX_X - 1; x++) {
                uint8_t a = COLOUR_RED;

                /* Obtain danger */
                p = borg_fear_region[y / 11][x / 11];

                /* Skip non-fears */
                if (p < avoidance / 10)
                    continue;

                /* Use colors = less painful */
                if (p < avoidance / 2)
                    a = COLOUR_ORANGE;
                if (p < avoidance / 4)
                    a = COLOUR_YELLOW;
                if (p < avoidance / 6)
                    a = COLOUR_GREEN;
                if (p < avoidance / 8)
                    a = COLOUR_BLUE;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("(%d,%d of %d,%d) Regional Fear.", borg.c.y, borg.c.x,
            Term->offset_y / borg_panel_hgt(),
            Term->offset_x / borg_panel_wid());
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y; y++) {
            for (x = 1; x <= AUTO_MAX_X; x++) {
                uint8_t a = COLOUR_BLUE;

                /* Obtain danger */
                p = borg_fear_monsters[y][x];

                /* Skip non-fears */
                if (p <= 0)
                    continue;

                /* Color Defines */
                if (p == 1)
                    a = COLOUR_L_BLUE;

                /* Color Defines */
                if (p < avoidance / 20 && p > 1)
                    a = COLOUR_BLUE;

                /* Color Defines */
                if (p < avoidance / 10 && p > avoidance / 20)
                    a = COLOUR_GREEN;

                /* Color Defines */
                if (p < avoidance / 4 && p > avoidance / 10)
                    a = COLOUR_YELLOW;

                /* Color Defines */
                if (p < avoidance / 2 && p > avoidance / 4)
                    a = COLOUR_ORANGE;

                /* Color Defines */
                if (p > avoidance / 2)
                    a = COLOUR_RED;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("(%d,%d of %d,%d) Monster Fear.", borg.c.y, borg.c.x,
            Term->offset_y / borg_panel_hgt(),
            Term->offset_x / borg_panel_wid());
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: debug -- Power */
    case 'p':
    case 'P': {
        int32_t p;

        /* Examine the screen */
        borg_notice_player();

        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Cheat the "inven" screen */
        borg_cheat_store();

        /* Examine the screen */
        borg_update();

        /* Examine the inventory */
        borg_object_fully_id();
        borg_notice(true);
        /* Evaluate */
        p = borg_power();

        borg_notice_home(NULL, false);

        /* Report it */
        msg("Current Borg Power %ld", p);
        msg("Current Home Power %ld", borg_power_home());

        break;
    }

    /* Command: Show time */
    case '!': {
        int32_t time = borg_t - borg_began;
        msg("time: (%d) ", time);
        time = (borg_time_town + (borg_t - borg_began));
        msg("; from town (%d)", time);
        msg("; on this panel (%d)", borg.time_this_panel);
        msg("; need inviso (%d)", borg.need_see_invis);
        break;
    }

    /* Command: LOS */
    case '@': {
        int x, y;

        /* Scan map */
        for (y = w_y; y < w_y + SCREEN_HGT; y++) {
            for (x = w_x; x < w_x + SCREEN_WID; x++) {
                uint8_t a = COLOUR_RED;

                /* Obtain danger */
                if (!borg_los(borg.c.y, borg.c.x, y, x))
                    continue;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("Borg has Projectable to these places.");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Scan map */
        for (y = w_y; y < w_y + SCREEN_HGT; y++) {
            for (x = w_x; x < w_x + SCREEN_WID; x++) {
                uint8_t a = COLOUR_YELLOW;

                if (!square_in_bounds(cave, loc(x, y)))
                    continue;

                /* Obtain danger */
                if (!borg_projectable_dark(borg.c.y, borg.c.x, y, x))
                    continue;

                /* Display */
                print_rel('*', a, y, x);
            }
        }
        msg("Borg has Projectable Dark to these places.");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Scan map */
        for (y = w_y; y < w_y + SCREEN_HGT; y++) {
            for (x = w_x; x < w_x + SCREEN_WID; x++) {
                uint8_t a = COLOUR_GREEN;

                /* Obtain danger */
                if (!borg_los(borg.c.y, borg.c.x, y, x))
                    continue;

                /* Display */
                print_rel('*', a, y, x);
            }
        }
        msg("Borg has LOS to these places.");
        event_signal(EVENT_MESSAGE_FLUSH);
        /* Redraw map */
        prt_map();
        break;
    }
    /*  command: debug -- change max depth */
    case '1': {
        int new_borg_skill;
        /* Get the new max depth */
        new_borg_skill
            = get_quantity("Enter new Max Depth: ", z_info->max_depth - 1);

        /* Allow user abort */
        if (new_borg_skill >= 0) {
            player->max_depth       = new_borg_skill;
            borg.trait[BI_MAXDEPTH] = new_borg_skill;
        }

        break;
    }
    /*  command: debug -- allow borg to stop */
    case 'q': {
        int new_borg_stop_dlevel = 127;
        int new_borg_stop_clevel = 51;

        /* Get the new max depth */
        new_borg_stop_dlevel = get_quantity(
            "Enter new auto-stop dlevel: ", z_info->max_depth - 1);
        new_borg_stop_clevel = get_quantity("Enter new auto-stop clevel: ", 51);
        get_com("Stop when Morgoth Dies? (y or n)? ", &cmd);

        borg_cfg[BORG_STOP_DLEVEL] = new_borg_stop_dlevel;
        borg_cfg[BORG_STOP_CLEVEL] = new_borg_stop_clevel;
        if (cmd == 'n' || cmd == 'N')
            borg_cfg[BORG_STOP_KING] = false;

        break;
    }

    /* command: money Scum-- allow borg to stop when he gets a certain amount of
     * money*/
    case 'm': {
        int new_borg_money_scum_amount = 0;

        /* report current status */
        msg("money Scumming for %d, I need %d more.",
            borg_cfg[BORG_MONEY_SCUM_AMOUNT],
            borg_cfg[BORG_MONEY_SCUM_AMOUNT] - borg.trait[BI_GOLD]);

        /* Get the new amount */
        new_borg_money_scum_amount = get_quantity(
            "Enter new dollar amount for money scumming (0 for no scumming):",
            INT_MAX);

        borg_cfg[BORG_MONEY_SCUM_AMOUNT] = new_borg_money_scum_amount;

        break;
    }
    /* Command:  HACK debug -- preparation for level */
    case '2': {
        int i = 0;

        /* Extract some "hidden" variables */
        borg_cheat_equip();
        borg_cheat_inven();

        /* Examine the screen */
        borg_notice_player();
        borg_update();

        /* Examine the inventory */
        borg_object_fully_id();
        borg_notice(true);
        borg_notice_home(NULL, false);

        /* Dump prep codes */
        for (i = 1; i <= 101; i++) {
            /* Dump fear code*/
            if ((char *)NULL != borg_prepared(i))
                break;
        }
        msg("Max Level: %d  Prep'd For: %d  Reason: %s",
            borg.trait[BI_MAXDEPTH], i - 1, borg_prepared(i));
        if (borg.ready_morgoth == 1) {
            msg("You are ready for the big fight!!");
        } else if (borg.ready_morgoth == 0) {
            msg("You are NOT ready for the big fight!!");
        } else if (borg.ready_morgoth == -1) {
            msg("No readiness check done.");
        }

        break;
    }
    /* Command: debug -- stat information */
    case '3': {

        int i;
        for (i = 0; i < STAT_MAX; i++) {
            borg_note(format("stat # %d, is: %d", i, borg.stat_cur[i]));
        }
#if 0
        artifact_type *a_ptr;

        int i;
        for (i = 0; i < z_info->a_max; i++) {
            a_ptr = &a_info[i];
            borg_note(format("(%d) %d, %d (act:%d)", i, a_ptr->name, a_ptr->text, a_ptr->activation));
        }
#endif
        break;
    }

    /* Command: List the swap weapon and armour */
    case 'w':
    case 'W': {
        borg_item *item;

        /* Cheat the "equip" screen */
        borg_cheat_equip();
        /* Cheat the "inven" screen */
        borg_cheat_inven();
        /* Examine the inventory */
        borg_notice(true);
        borg_notice_home(NULL, false);
        /* Check the power */
        borg_power();

        /* Examine the screen */
        borg_update();

        /* Examine the screen */
        borg_notice_player();

        /* note the swap items */
        if (weapon_swap) {
            item = &borg_items[weapon_swap - 1];
            msg("Swap Weapon:  %s, value= %d", item->desc, weapon_swap_value);
        } else {
            msg("Swap Weapon:  NONE");
        }

        if (armour_swap) {
            item = &borg_items[armour_swap - 1];
            msg("Swap Armour:  %s, value= %d", item->desc, armour_swap_value);
        } else {
            msg("Swap Armour:  NONE");
        }
        break;
    }
    case 'd': {
        int ii = 1;

        /* Save the screen */
        Term_save();

        /* Dump the spells */
        if (borg_can_cast()) {

            int i;

            for (i = 0; i < player->class->magic.total_spells; i++) {
                /* Clear the screen */
                Term_clear();

                ii = 2;
                Term_putstr(1, ii, -1, COLOUR_WHITE, "[ Spells ].");
                borg_magic *as          = &borg_magics[i];
                int         failpercent = 0;

                if (as->level < 99) {
                    const char *legal
                        = (borg_spell_legal(as->spell_enum) ? "legal"
                                                            : "Not Legal ");
                    failpercent = (borg_spell_fail_rate(as->spell_enum));

                    Term_putstr(1, ii++, -1, COLOUR_WHITE,
                        format("%s, %s, attempted %d times, fail rate:%d",
                            as->name, legal, as->times, failpercent));
                }
                get_com(
                    "Exam spell books.  Press any key for next book.", &cmd);
            } /* dumps */
        } /* spells */

        /* Restore the screen */
        Term_load();

        /* Done */
        return;
    }

    /* dump borg 'has' information */
    case 'h':
    case 'H': {
        int item, to;

        /* Get a "Borg command", or abort */
        if (!get_com("Dynamic Borg Has What: "
                     "((a)ny/(i)nv/(w)orn/a(r)tifact/(s)kill) ",
                &cmd))
            return;

        switch (cmd) {
        case 'a':
        case 'A':
            item = 0;
            to   = z_info->k_max;
            break;
        case 'i':
        case 'I':
            item = 0;
            to   = z_info->pack_size;
            break;
        case 'w':
        case 'W':
            item = INVEN_WIELD;
            to   = QUIVER_END;
            break;
        case 'r':
        case 'R':
            item = 0;
            to   = QUIVER_END;
            break;
        default:
            item = 0;
            to   = BI_MAX;
            break;
        }
        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Examine the screen */
        borg_notice_player();

        /* Examine the screen */
        borg_update();

        /* Examine the inventory */
        borg_object_fully_id();
        borg_notice(true);
        borg_notice_home(NULL, false);
        for (; item < to; item++) {
            switch (cmd) {
            case 'a':
            case 'A':
                if (borg.has[item]) {
                    borg_note(format("Item-Kind:%03d name=%s value= %d.", item,
                        k_info[item].name, borg.has[item]));
                }
                break;
            case 'i':
            case 'I':
                if (borg_items[item].iqty) {
                    borg_note(format("Item-Invn:%03d desc= %s qty %d.", item,
                        borg_items[item].desc, borg_items[item].iqty));
                }
                break;
            case 'w':
            case 'W':
                if (borg_items[item].iqty) {
                    borg_note(format("Item-Worn:%03d desc= %s qty %d.", item,
                        borg_items[item].desc, borg_items[item].iqty));
                }
                break;
            case 'r':
            case 'R':
                if (borg_items[item].iqty && borg_items[item].art_idx)
                    borg_note(format("Item-Arti:%03d name= %s.", item,
                        a_info[borg_items[item].art_idx].name));
                break;
            default: {
                borg_note(format("trait %d (%s) value= %d.", item,
                    prefix_pref[item], borg.trait[item]));
                break;
            }
            }
        }

        /* note the completion. */
        msg("Borg_has[] dump complete.  Examine Log. ");
        break;
    }

    /* Version of the game */
    case 'v':
    case 'V': {
        msg("APWBorg Version: %s", borg_engine_date);
        break;
    }
    /* Command: Display all known info on item */
    case 'o':
    case 'O': {
        int n = 0;

        struct object *item2;

        /* use this item */
        // XXX replace this with an item selector
        n = get_quantity("Which item?", z_info->pack_size);

        /* Cheat the "equip" screen */
        borg_cheat_equip();
        /* Cheat the "inven" screen */
        borg_cheat_inven();
        /* Examine the inventory */
        borg_notice(true);
        borg_notice_home(NULL, false);
        /* Check the power */
        borg_power();

        /* Examine the screen */
        borg_update();

        /* Examine the screen */
        borg_notice_player();

        /* Save the screen */
        Term_save();

        /* get the item */
        item2 = player->upkeep->inven[n];

        /* Display the special screen */
        borg_display_item(item2, n);

        /* pause for study */
        msg("Borg believes: ");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Restore the screen */
        Term_load();

        break;
    }

    /* Command: Resurrect Borg */
    case 'R': {
        /* Confirm it */
        get_com("Are you sure you want to Respawn this borg? (y or n)? ", &cmd);

        if (cmd == 'y' || cmd == 'Y') {
            reincarnate_borg();
        }
        break;
    }

    /* Command: Restock the Stores */
    case 'r': {
        /* Confirm it */
        get_com(
            "Are you sure you want to Restock the stores? (y or n)? ", &cmd);

        if (cmd == 'y' || cmd == 'Y') {
            /* Message */
            msg("Updating Shops... currently not allowed");
#if false
            msg("Updating Shops...");
            // need to change base code to make store_maint accessable .. trying not to change that too much right now.  
            // this functionality seems a bit bogus anyway !FIX !TODO !AJG
                            /* Maintain each shop (except home) */
            for (n = 0; n < MAX_STORES; n++) {
                /* Skip the home */
                if (n == STORE_HOME) continue;

                /* Maintain */
                store_maint(&stores[n]);
            }
#endif
        }

        break;
    }

    case ';': {
        int glyph_check;

        uint8_t a = COLOUR_RED;

        for (glyph_check = 0; glyph_check < track_glyph.num; glyph_check++) {
            /* Display */
            print_rel(
                '*', a, track_glyph.y[glyph_check], track_glyph.x[glyph_check]);
            msg("Borg has Glyph (%d)noted.", glyph_check);
            event_signal(EVENT_MESSAGE_FLUSH);
        }

        /* Get keypress */
    } break;
    /* Oops */
    default: {
        /* Message */
        msg("That is not a legal Borg command.");
        break;
    }
    }
}

#endif
