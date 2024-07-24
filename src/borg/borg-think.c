/**
 * \file borg-think.c
 * \brief The entry point for the borg thinking about what to do
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

#include "borg-think.h"

#ifdef ALLOW_BORG

#include "../player-util.h"
#include "../ui-game.h"
#include "../ui-menu.h"

#include "borg-inventory.h"
#include "borg-io.h"
#include "borg-item-wear.h"
#include "borg-log.h"
#include "borg-magic.h"
#include "borg-power.h"
#include "borg-reincarnate.h"
#include "borg-store.h"
#include "borg-think-dungeon.h"
#include "borg-think-store.h"
#include "borg-trait.h"
#include "borg-update.h"
#include "borg.h"

/*
 * Hack -- location of the "Lv Mana Fail" prompt
 */
#define ROW_SPELL 1
#define COL_SPELL 20 + 35

/*
 * Current shop index
 */
int16_t shop_num = -1;

/*
 * Strategy flags -- examine the world
 */
bool    borg_do_inven     = true; /* Acquire "inven" info */
bool    borg_do_equip     = true; /* Acquire "equip" info */
bool    borg_do_panel     = true; /* Acquire "panel" info */
bool    borg_do_frame     = true; /* Acquire "frame" info */
bool    borg_do_spell     = true; /* Acquire "spell" info */
uint8_t borg_do_spell_aux = 0; /* Hack -- book for "borg_do_spell" */

/*
 * Abort the Borg, noting the reason
 */
void borg_oops(const char *what)
{
    /* Stop processing */
    borg_active = false;

    /* Give a warning */
    borg_note(format("# Aborting (%s).", what));

    /* Forget borg keys */
    borg_flush();
}

/*
 * Attempt to save the game
 */
static bool borg_save_game(void)
{
    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Save the game */
    borg_keypress('^');
    borg_keypress('S');

    /* Cancel everything */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);

    /* Success */
    return true;
}

/*
 * Think about the world and perform an action
 *
 * Check inventory/equipment/spells/panel once per "turn"
 *
 * Process "store" and other modes when necessary
 *
 * Note that the non-cheating "inventory" and "equipment" parsers
 * will get confused by a "weird" situation involving an ant ("a")
 * on line one of the screen, near the left, next to a shield, of
 * the same color, and using --(-- the ")" symbol, directly to the
 * right of the ant.  This is very rare, but perhaps not completely
 * impossible.  I ignore this situation.  :-)
 *
 * The handling of stores is a complete and total hack, but seems
 * to work remarkably well, considering... :-)  Note that while in
 * a store, time does not pass, and most actions are not available,
 * and a few new commands are available ("sell" and "purchase").
 *
 * Note the use of "cheat" functions to extract the current inventory,
 * the current equipment, the current panel, and the current spellbook
 * information.  These can be replaced by (very expensive) "parse"
 * functions, which cause an insane amount of "screen flashing".
 *
 * Technically, we should attempt to parse all the messages that
 * indicate that it is necessary to re-parse the equipment, the
 * inventory, or the books, and only set the appropriate flags
 * at that point.  This would not only reduce the potential
 * screen flashing, but would also optimize the code a lot,
 * since the "cheat_inven()" and "cheat_equip()" functions
 * are expensive.  For paranoia, we could always select items
 * and spells using capital letters, and keep a global verification
 * buffer, and induce failure and recheck the inventory/equipment
 * any time we get a mis-match.  We could even do some of the state
 * processing by hand, for example, charge reduction and such.  This
 * might also allow us to keep track of how long we have held objects,
 * especially if we attempt to do "item tracking" in the inventory
 * extraction code.
 */
bool borg_think(void)
{
    int i;

    uint8_t t_a;

    char        buf[128];
    static char svSavefile[1024];
    static char svSavefile2[1024];
    static bool justSaved = false;

    /* Fill up part of the borg.trait[] array */
    (void)borg_notice_player();

    /*** Process inventory/equipment ***/

    /* Cheat */
    if (borg_do_equip) {
        /* Only do it once */
        borg_do_equip = false;

        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Done */
        return false;
    }

    /* Cheat */
    if (borg_do_inven) {
        /* Only do it once */
        borg_do_inven = false;

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Done */
        return false;
    }

    /* save now */
    if (borg_save && borg_save_game()) {
        /* Log */
        borg_note("# Auto Save!");

        borg_save = false;

        /* Create a scum file */
        if (borg.trait[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL]
            || strstr(player->died_from, "starvation")) {
            memcpy(svSavefile, savefile, sizeof(savefile));
            /* Process the player name */
            for (i = 0; player->full_name[i]; i++) {
                char c = player->full_name[i];

                /* No control characters */
                if (iscntrl(c)) {
                    /* Illegal characters */
                    quit_fmt("Illegal control char (0x%02X) in player name", c);
                }

                /* Convert all non-alphanumeric symbols */
                if (!isalpha(c) && !isdigit(c))
                    c = '_';

                /* Build "file_name" */
                svSavefile2[i] = c;
            }
            svSavefile2[i] = 0;

            path_build(savefile, 1024, ANGBAND_DIR_USER, svSavefile2);

            justSaved = true;
        }
        return true;
    }
    if (justSaved) {
        memcpy(savefile, svSavefile, sizeof(savefile));
        borg_save_game();
        justSaved = false;
        return true;
    }

    /* Parse "equip" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf))
        && (streq(buf, "(Equipment) "))) {
        /* Parse the "equip" screen */
        /* borg_parse_equip(); */

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return true;
    }

    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf))
        && (streq(buf, "(Inventory) "))) {
        /* Parse the "inven" screen */
        /* borg_parse_inven(); */

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return true;
    }

    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 6, &t_a, buf)) && (streq(buf, "Wear o"))) {
        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return true;
    }

    /*** Find books ***/

    /* Only if needed */
    if (borg_do_spell && (borg_do_spell_aux == 0)) {
        /* Assume no books */
        for (i = 0; i < 9; i++)
            borg.book_idx[i] = -1;

        /* Scan the pack */
        for (i = 0; i < z_info->pack_size; i++) {
            int        book_num;
            borg_item *item = &borg_items[i];

            for (book_num = 0; book_num < player->class->magic.num_books;
                 book_num++) {
                struct class_book book = player->class->magic.books[book_num];
                if (item->tval == book.tval && item->sval == book.sval) {
                    /* Note book locations */
                    borg.book_idx[book_num] = i;
                    break;
                }
            }
        }
    }

    /*** Process books ***/
    /* Hack -- Warriors never browse */
    if (borg.trait[BI_CLASS] == CLASS_WARRIOR)
        borg_do_spell = false;

    /* Hack -- Blind or Confused prevents browsing */
    if (borg.trait[BI_ISBLIND] || borg.trait[BI_ISCONFUSED])
        borg_do_spell = false;

    /* XXX XXX XXX Dark */

    /* Hack -- Stop doing spells when done */
    if (borg_do_spell_aux > 8)
        borg_do_spell = false;

    /* Cheat */
    if (borg_do_spell) {
        /* Look for the book */
        i = borg.book_idx[borg_do_spell_aux];

        /* Cheat the "spell" screens (all of them) */
        if (i >= 0) {
            /* Cheat that page */
            borg_cheat_spell(borg_do_spell_aux);
        }

        /* Advance to the next book */
        borg_do_spell_aux++;

        /* Done */
        return false;
    }

    /* Check for "browse" mode */
    if ((0 == borg_what_text(COL_SPELL, ROW_SPELL, -12, &t_a, buf))
        && (streq(buf, "Lv Mana Fail"))) {
        /* Parse the "spell" screen */
        /* borg_parse_spell(borg_do_spell_aux); */

        /* Advance to the next book */
        /* borg_do_spell_aux++; */

        /* Leave that mode */
        borg_keypress(ESCAPE);

        /* Done */
        return true;
    }

    /* If king, maybe retire. */
    if (borg.trait[BI_KING]) {
        /* Prepare to retire */
        if (borg_cfg[BORG_STOP_KING]) {
#ifndef BABLOS
            borg_write_map(false);
#endif /* bablos */
            borg_oops("retire");
        }
        /* Borg will be respawning */
        if (borg_cfg[BORG_RESPAWN_WINNERS]) {
#ifndef BABLOS
            borg_write_map(false);
#if 0
            /* Note the score */
            borg_enter_score();
#endif
            /* Write to log and borg.dat */
            borg_log_death();
            borg_log_death_data();

            /* respawn */
            reincarnate_borg();

            borg_flush();
#endif /* bablos */
        }
    }

    /* Hack -- always revert shapechanged players to normal form */
    if (player_is_shapechanged(player)) {
        borg_keypress('m');
        borg_keypress('r');
        return true;
    }

    /*** Handle stores ***/

    /* Hack -- Check for being in a store CHEAT*/
    if ((0 == borg_what_text(1, 3, 4, &t_a, buf))
        && (streq(buf, "Stor") || streq(buf, "Home"))) {
        /* Cheat the store number */
        shop_num = square_shopnum(cave, player->grid);

        /* Clear the goal (the goal was probably going to a shop number) */
        borg.goal.type = 0;

        /* Hack -- Reset food counter for money scumming */
        if (shop_num == 0)
            borg_food_onsale = 0;

        /* Hack -- Reset fuel counter for money scumming */
        if (shop_num == 0)
            borg_fuel_onsale = 0;

        /* Extract the current gold (unless in home) */
        borg.trait[BI_GOLD] = (long)player->au;

        /* Cheat the store (or home) inventory (all pages) */
        borg_cheat_store();

        /* Recheck inventory */
        borg_do_inven = true;

        /* Recheck equipment */
        borg_do_equip = true;

        /* Recheck spells */
        borg_do_spell = true;

        /* Restart spells */
        borg_do_spell_aux = 0;

        /* Examine the inventory */
        borg_notice(true);

        /* Evaluate the current world */
        borg.power = borg_power();

        /* Hack -- allow user abort */
        if (borg_cancel)
            return true;

        /* Do not allow a user key to interrupt the borg while in a store */
        borg.in_shop = true;

        /* Think until done */
        return (borg_think_store());
    }

    /*** Determine panel ***/

    /* Hack -- cheat */
    w_y = Term->offset_y;
    w_x = Term->offset_x;

    /* Done */
    borg_do_panel = false;

    /* Hack -- Check for "sector" mode */
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf))
        && (prefix(buf, "Map sector "))) {
        /* Hack -- get the panel info */
        w_y = (buf[12] - '0') * (SCREEN_HGT / 2);
        w_x = (buf[14] - '0') * (SCREEN_WID / 2);

        /* Leave panel mode */
        borg_keypress(ESCAPE);

        /* Done */
        return true;
    }

    /* Check panel */
    if (borg_do_panel) {
        /* Only do it once */
        borg_do_panel = false;

        /* Enter "panel" mode */
        borg_keypress('L');

        /* Done */
        return true;
    }

    /*** Analyze the Frame ***/

    /* Analyze the frame */
    if (borg_do_frame) {
        /* Only once */
        borg_do_frame = false;

        /* Analyze the "frame" */
        borg_notice_player();
    }

    /*** Re-activate Tests ***/

    /* Check equip again later */
    borg_do_equip = true;

    /* Check inven again later */
    borg_do_inven = true;

    /* Check panel again later */
    borg_do_panel = true;

    /* Check frame again later */
    borg_do_frame = true;

    /* Check spells again later */
    borg_do_spell = true;

    /* Hack -- Start the books over */
    borg_do_spell_aux = 0;

    /*** Analyze status ***/

    /* Track best level */
    if (borg.trait[BI_CLEVEL] > borg.trait[BI_MAXCLEVEL])
        borg.trait[BI_MAXCLEVEL] = borg.trait[BI_CLEVEL];
    if (borg.trait[BI_CDEPTH] > borg.trait[BI_MAXDEPTH]) {
        borg.trait[BI_MAXDEPTH] = borg.trait[BI_CDEPTH];
    }

    /*** Think about it ***/

    /* Increment the clock */
    borg_t++;

    /* Increment the panel clock */
    borg.time_this_panel++;

    /* Examine the equipment/inventory */
    borg_notice(true);

    /* Examine the screen */
    borg_update();

    /* Evaluate the current world */
    borg.power = borg_power();

    /* Hack -- allow user abort */
    if (borg_cancel)
        return true;

    /* Do something */
    return (borg_think_dungeon());
}

#endif
