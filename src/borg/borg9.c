/* File: borg9.c */

/* Purpose: Highest level functions for the Borg -BEN- */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "../cave.h"
#include "../obj-desc.h"
#include "../obj-gear.h"
#include "../obj-init.h"
#include "../obj-make.h"
#include "../obj-knowledge.h"
#include "../obj-pile.h"
#include "../obj-power.h"
#include "../obj-randart.h"
#include "../obj-tval.h"
#include "../game-input.h"
#include "../game-world.h"
#include "../player-birth.h"
#include "../player-calcs.h"
#include "../player-spell.h"
#include "../player-timed.h"
#include "../player-util.h"
#include "../project.h"
#include "../store.h"
#include "../target.h"
#include "../ui-command.h"
#include "../ui-game.h"
#include "../ui-map.h"
#include "../ui-prefs.h"
#include "../ui-input.h"
#include "../ui-output.h"
#include "../trap.h"

#include "borg1.h"
#include "borg2.h"
#include "borg3.h"
#include "borg4.h"
#include "borg5.h"
#include "borg6.h"
#include "borg7.h"
#include "borg8.h"
#include "borg9.h"

#ifdef BABLOS
extern bool     auto_play;
extern bool     keep_playing;
#endif /* bablos */
bool            borg_cheat_death;

static int key_mode; /* KEYMAP_MODE_ROGUE or KEYMAP_MODE_ORIG */

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
 * control from the "COLOUR_inkey()" and "COLOUR_flush(0, 0, 0)" functions.  This
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
  * Some variables
  */

static bool initialized;    /* Hack -- Initialized */
static bool game_closed;    /* Has the game been closed since the borg was
                               initialized */


#ifndef BABLOS

static void borg_log_death(void)
{
    char buf[1024];
    ang_file* borg_log_file;
    time_t death_time;

    /* Build path to location of the definition file */
    path_build(buf, 1024, ANGBAND_DIR_USER, "borg-log.txt");

    /* Append to the file */
    borg_log_file = file_open(buf, MODE_APPEND, FTYPE_TEXT);

    /* Failure */
    if (!borg_log_file) return;

    /* Get time of death */
    (void)time(&death_time);

    /* Save the date */
    strftime(buf, 80, "%Y/%m/%d %H:%M\n", localtime(&death_time));

    file_put(borg_log_file, buf);

    file_putf(borg_log_file, "%s the %s %s, Level %d/%d\n",
        player->full_name,
        player->race->name,
        player->class->name,
        player->lev, player->max_lev);

    file_putf(borg_log_file, "Exp: %lu  Gold: %lu  Turn: %lu\n", (long)player->max_exp + (100 * player->max_depth), (long)player->au, (long)turn);
    file_putf(borg_log_file, "Killed on level: %d (max. %d) by %s\n", player->depth, player->max_depth, player->died_from);

    file_putf(borg_log_file, "Borg Compile Date: %s\n", borg_engine_date);

    file_put(borg_log_file, "----------\n\n");

    file_close(borg_log_file);
}

#endif /* BABLOS */

static void borg_log_death_data(void)
{
    char buf[1024];
    ang_file* borg_log_file;
    time_t death_time;

    path_build(buf, 1024, ANGBAND_DIR_USER, "borg.dat");

    /* Append to the file */
    borg_log_file = file_open(buf, MODE_APPEND, FTYPE_TEXT);

    /* Failure */
    if (!borg_log_file) return;

    /* Get time of death */
    (void)time(&death_time);

    /* dump stuff for easy import to database */
    file_putf(borg_log_file, "%s, %s, %s, %d, %d, %s\n", borg_engine_date, player->race->name,
        player->class->name, player->lev, player->depth, player->died_from);


    file_close(borg_log_file);
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
static bool borg_think(void)
{
    int i;

    uint8_t t_a;

    char buf[128];
    static char svSavefile[1024];
    static char svSavefile2[1024];
    static bool justSaved = false;


    /* Fill up the borg_skill[] array */
    (void)borg_update_frame();

    /*** Process inventory/equipment ***/

    /* Cheat */
    if (borg_do_equip)
    {
        /* Only do it once */
        borg_do_equip = false;

        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Done */
        return (false);
    }

    /* Cheat */
    if (borg_do_inven)
    {
        /* Only do it once */
        borg_do_inven = false;

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Done */
        return (false);
    }

    /* save now */
    if (borg_save && borg_save_game())
    {
        /* Log */
        borg_note("# Auto Save!");

        borg_save = false;

        /* Create a scum file */
        if (borg_skill[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL] ||
            strstr(player->died_from, "starvation"))
        {
            memcpy(svSavefile, savefile, sizeof(savefile));
            /* Process the player name */
            for (i = 0; player->full_name[i]; i++)
            {
                char c = player->full_name[i];

                /* No control characters */
                if (iscntrl(c))
                {
                    /* Illegal characters */
                    quit_fmt("Illegal control char (0x%02X) in player name", c);
                }

                /* Convert all non-alphanumeric symbols */
                if (!isalpha(c) && !isdigit(c)) c = '_';

                /* Build "file_name" */
                svSavefile2[i] = c;
            }
            svSavefile2[i] = 0;

            path_build(savefile, 1024, ANGBAND_DIR_USER, svSavefile2);


            justSaved = true;
        }
        return (true);
    }
    if (justSaved)
    {
        memcpy(savefile, svSavefile, sizeof(savefile));
        borg_save_game();
        justSaved = false;
        return (true);
    }

    /* Parse "equip" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
        (streq(buf, "(Equipment) ")))
    {
        /* Parse the "equip" screen */
        /* borg_parse_equip(); */

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (true);
    }


    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
        (streq(buf, "(Inventory) ")))
    {
        /* Parse the "inven" screen */
        /* borg_parse_inven(); */

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (true);
    }

    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 6, &t_a, buf)) &&
        (streq(buf, "(Inven")))
    {
        if (borg_best_item != -1)
            borg_keypress(all_letters_nohjkl[borg_best_item]);

        /* Leave this mode */
        borg_keypress(ESCAPE);
        borg_best_item = -1;

        /* Done */
        return (true);
    }

    /*** Find books ***/

    /* Only if needed */
    if (borg_do_spell && (borg_do_spell_aux == 0))
    {
        /* Assume no books */
        for (i = 0; i < 9; i++) borg_book[i] = -1;

        /* Scan the pack */
        for (i = 0; i < z_info->pack_size; i++)
        {
            int book_num;
            borg_item* item = &borg_items[i];

            for (book_num = 0; book_num < player->class->magic.num_books; book_num++)
            {
                struct class_book book = player->class->magic.books[book_num];
                if (item->tval == book.tval && item->sval == book.sval)
                {
                    /* Note book locations */
                    borg_book[book_num] = i;
                    break;
                }
            }
        }
    }


    /*** Process books ***/
    /* Hack -- Warriors never browse */
    if (borg_class == CLASS_WARRIOR) borg_do_spell = false;

    /* Hack -- Blind or Confused prevents browsing */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) borg_do_spell = false;

    /* XXX XXX XXX Dark */

    /* Hack -- Stop doing spells when done */
    if (borg_do_spell_aux > 8) borg_do_spell = false;

    /* Cheat */
    if (borg_do_spell)
    {
        /* Look for the book */
        i = borg_book[borg_do_spell_aux];

        /* Cheat the "spell" screens (all of them) */
        if (i >= 0)
        {
            /* Cheat that page */
            borg_cheat_spell(borg_do_spell_aux);
        }

        /* Advance to the next book */
        borg_do_spell_aux++;

        /* Done */
        return (false);
    }

    /* Check for "browse" mode */
    if ((0 == borg_what_text(COL_SPELL, ROW_SPELL, -12, &t_a, buf)) &&
        (streq(buf, "Lv Mana Fail")))
    {
        /* Parse the "spell" screen */
        /* borg_parse_spell(borg_do_spell_aux); */

        /* Advance to the next book */
        /* borg_do_spell_aux++; */

        /* Leave that mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (true);
    }

    /* If king, maybe retire. */
    if (borg_skill[BI_KING])
    {
        /* Prepare to retire */
        if (borg_cfg[BORG_STOP_KING])
        {
#ifndef BABLOS
            borg_write_map(false);
#endif /* bablos */
            borg_oops("retire");
        }
        /* Borg will be respawning */
        if (borg_cfg[BORG_RESPAWN_WINNERS])
        {
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
            resurrect_borg();

            borg_flush();
#endif /* bablos */
        }

    }

    /* Hack -- always revert shapechanged players to normal form */
    if (player_is_shapechanged(player))
    {
        borg_keypress('m');
        borg_keypress('r');
        return true;
    }

    /*** Handle stores ***/

    /* Hack -- Check for being in a store CHEAT*/
    if ((0 == borg_what_text(1, 3, 4, &t_a, buf)) &&
        (streq(buf, "Stor") ||
            streq(buf, "Home")))
    {
        /* Cheat the store number */
        shop_num = square_shopnum(cave, player->grid);

        /* Clear the goal (the goal was probably going to a shop number) */
        goal = 0;

        /* Hack -- Reset food counter for money scumming */
        if (shop_num == 0) borg_food_onsale = 0;

        /* Hack -- Reset fuel counter for money scumming */
        if (shop_num == 0) borg_fuel_onsale = 0;

        /* Extract the current gold (unless in home) */
        borg_gold = (long)player->au;

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
        my_power = borg_power();

        /* Hack -- allow user abort */
        if (borg_cancel) return (true);

        /* Do not allow a user key to interrupt the borg while in a store */
        borg_in_shop = true;

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
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf)) &&
        (prefix(buf, "Map sector ")))
    {
        /* Hack -- get the panel info */
        w_y = (buf[12] - '0') * (SCREEN_HGT / 2);
        w_x = (buf[14] - '0') * (SCREEN_WID / 2);

        /* Leave panel mode */
        borg_keypress(ESCAPE);

        /* Done */
        return (true);
    }

    /* Check panel */
    if (borg_do_panel)
    {
        /* Only do it once */
        borg_do_panel = false;

        /* Enter "panel" mode */
        borg_keypress('L');

        /* Done */
        return (true);
    }

    /*** Analyze the Frame ***/

    /* Analyze the frame */
    if (borg_do_frame)
    {
        /* Only once */
        borg_do_frame = false;

        /* Analyze the "frame" */
        borg_update_frame();
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
    if (borg_skill[BI_CLEVEL] > borg_skill[BI_MAXCLEVEL]) borg_skill[BI_MAXCLEVEL] = borg_skill[BI_CLEVEL];
    if (borg_skill[BI_CDEPTH] > borg_skill[BI_MAXDEPTH])
    {
        borg_skill[BI_MAXDEPTH] = borg_skill[BI_CDEPTH];
    }

    /*** Think about it ***/

    /* Increment the clock */
    borg_t++;

    /* Increment the panel clock */
    time_this_panel++;

    /* Examine the screen */
    borg_update();

    /* Examine the equipment/inventory */
    borg_notice(true);

    /* Evaluate the current world */
    my_power = borg_power();

    /* Hack -- allow user abort */
    if (borg_cancel) return (true);

    /* Do something */
    return (borg_think_dungeon());
}

static char** suffix_pain;

/*
 * Hack -- methods of killing a monster (order not important).
 *
 * See "mon_take_hit()" for details.
 */
static const char* prefix_kill[] =
{
    "You have killed ",
    "You have slain ",
    "You have destroyed ",
    NULL
};


/*
 * Hack -- methods of monster death (order not important).
 *
 * See "project_m()", "do_cmd_fire()", "mon_take_hit()" for details.
 */
static const char* suffix_died[] =
{
    " dies.",
    " is destroyed.",
    " is drained dry!",
    NULL
};
static const char* suffix_blink[] =
{
    " disappears!",      /* from teleport other */
    " intones strange words.",         /* from polymorph spell */
    " teleports away.",  /* RF6_TPORT */
    " blinks.",                /* RF6_BLINK */
    " makes a soft 'pop'.",
    NULL
};


/* a message can have up to three parts broken up by variables */
/* ex: "{name} hits {pronoun} followers with {type} ax." */
/* " hits ", " followers with ", " ax." */
/* if the message has more parts than that, they are ignored so  */
/* ex: "{name} hits {pronoun} followers with {type} ax and {type} breath." */
/* would end up as */
/* " hits ", " followers with ", " ax and " */
/* hopefully this is enough to keep the messages as unique as possible */
struct borg_read_message
{
    char* message_p1;
    char* message_p2;
    char* message_p3;
};

struct borg_read_messages
{
    int count;
    int allocated;
    struct borg_read_message* messages;
    int* index;
};

/*  methods of hitting the player */
static struct borg_read_messages suffix_hit_by;

/*  methods of casting spells at the player */
static struct borg_read_messages spell_msgs;
static struct borg_read_messages spell_invis_msgs;

static bool borg_message_contains(const char* value, struct borg_read_message* message)
{
    if (strstr(value, message->message_p1) &&
        (!message->message_p2 || strstr(value, message->message_p2)) &&
        (!message->message_p3 || strstr(value, message->message_p3)))
        return true;
    return false;
}


#if 0
/* XXX XXX XXX */
msg("%s looks healthier.", m_name);
msg("%s looks REALLY healthy!", m_name);
#endif



/*
 * Hack -- Spontaneous level feelings (order important).
 *
 * See "do_cmd_feeling()" for details.
 */
static const char* prefix_feeling_danger[] =
{
    "You are still uncertain about this place",
    "Omens of death haunt this place",
    "This place seems murderous",
    "This place seems terribly dangerous",
    "You feel anxious about this place",
    "You feel nervous about this place",
    "This place does not seem too risky",
    "This place seems reasonably safe",
    "This seems a tame, sheltered place",
    "This seems a quiet, peaceful place",
    NULL
};
static const char* suffix_feeling_stuff[] =
{
    "Looks like any other level.",
    "you sense an item of wondrous power!",
    "there are superb treasures here.",
    "there are excellent treasures here.",
    "there are very good treasures here.",
    "there are good treasures here.",
    "there may be something worthwhile here.",
    "there may not be much interesting here.",
    "there aren't many treasures here.",
    "there are only scraps of junk here.",
    "there is naught but cobwebs here.",
    NULL
};


/*
 * Hack -- Parse a message from the world
 *
 * Note that detecting "death" is EXTREMELY important, to prevent
 * all sorts of errors arising from attempting to parse the "tomb"
 * screen, and to allow the user to "observe" the "cause" of death.
 *
 * Note that detecting "failure" is EXTREMELY important, to prevent
 * bizarre situations after failing to use a staff of perceptions,
 * which would otherwise go ahead and send the "item index" which
 * might be a legal command (such as "a" for "aim").  This method
 * is necessary because the Borg cannot parse "prompts", and must
 * assume the success of the prompt-inducing command, unless told
 * otherwise by a failure message.  Also, we need to detect failure
 * because some commands, such as detection spells, need to induce
 * furthur processing if they succeed, but messages are only given
 * if the command fails.
 *
 * Note that certain other messages may contain useful information,
 * and so they are "analyzed" and sent to "borg_react()", which just
 * queues the messages for later analysis in the proper context.
 *
 * Along with the actual message, we send a special formatted buffer,
 * containing a leading "opcode", which may contain extra information,
 * such as the index of a spell, and an "argument" (for example, the
 * capitalized name of a monster), with a "colon" to separate them.
 *
 * XXX XXX XXX Several message strings take a "possessive" of the form
 * "his" or "her" or "its".  These strings are all represented by the
 * encoded form "XXX" in the various match strings.  Unfortunately,
 * the encode form is never decoded, so the Borg currently ignores
 * messages about several spells (heal self and haste self).
 *
 * XXX XXX XXX We notice a few "terrain feature" messages here so
 * we can acquire knowledge about wall types and door types.
 */
static void borg_parse_aux(char* msg, int len)
{
    int i, tmp;

    int y9;
    int x9;
    int ax, ay;
    int d;

    char who[256];
    char buf[256];

    borg_grid* ag = &borg_grids[g_y][g_x];

    /* Log (if needed) */
    if (borg_cfg[BORG_VERBOSE]) 
        borg_note(format("# Parse Msg bite <%s>", msg));

    /* Hack -- Notice death */
    if (prefix(msg, "You die."))
    {
        /* Abort (unless cheating) */
        if (!(player->wizard || OPT(player, cheat_live) || borg_cheat_death))
        {
            /* Abort */
            borg_oops("death");

            /* Abort right now! */
            borg_active = false;
            /* Noise XXX XXX XXX */
            Term_xtra(TERM_XTRA_NOISE, 1);
        }

        /* Done */
        return;
    }

    /* Hack -- Notice "failure" */
    if (prefix(msg, "You failed "))
    {
        /* Hack -- store the keypress */
        borg_note("# Normal failure.");

        /* Set the failure flag */
        borg_failure = true;

        /* Flush our key-buffer */
        borg_flush();

        /* If we were casting a targetted spell and failed */
        /* it does not mean we can't target that location */
        successful_target = 0;

        /* Incase we failed our emergency use of MM */
        borg_confirm_target = false;

        /* check for glyphs since we no longer have a launch message */
        if (borg_casted_glyph)
        {
            /* Forget the newly created-though-failed  glyph */
            track_glyph.num--;
            track_glyph.x[track_glyph.num] = 0;
            track_glyph.y[track_glyph.num] = 0;
            borg_note("# Removing glyph from array,");
            borg_casted_glyph = false;
        }

        /* Incase it was a Resistance refresh */
        if (borg_attempting_refresh_resist)
        {
            if (borg_resistance > 1) borg_resistance -= 25000;
            borg_attempting_refresh_resist = false;
        }

        return;

    }

    /* Mega-Hack -- Check against the search string */
    if (borg_match[0] && strstr(msg, borg_match))
    {
        /* Clean cancel */
        borg_cancel = true;
    }


    /* Ignore teleport trap */
    if (prefix(msg, "You hit a teleport")) return;

    /* Ignore arrow traps */
    if (prefix(msg, "An arrow ")) return;

    /* Ignore dart traps */
    if (prefix(msg, "A small dart ")) return;

    if (prefix(msg, "The cave "))
    {
        borg_react(msg, "QUAKE:Somebody");
        borg_needs_new_sea = true;
        return;
    }

    /* need to check stat */
    if (prefix(msg, "You feel very") ||
        prefix(msg, "You feel less") ||
        prefix(msg, "Wow!  You feel very"))
    {
        /* need to check str */
        if (prefix(msg, "You feel very weak"))
        {
            my_need_stat_check[STAT_STR] = true;
        }
        if (prefix(msg, "You feel less weak"))
        {
            my_need_stat_check[STAT_STR] = true;
        }
        if (prefix(msg, "Wow!  You feel very strong"))
        {
            my_need_stat_check[STAT_STR] = true;
        }

        /* need to check int */
        if (prefix(msg, "You feel very stupid"))
        {
            my_need_stat_check[STAT_INT] = true;
        }
        if (prefix(msg, "You feel less stupid"))
        {
            my_need_stat_check[STAT_INT] = true;
        }
        if (prefix(msg, "Wow!  You feel very smart"))
        {
            my_need_stat_check[STAT_INT] = true;
        }

        /* need to check wis */
        if (prefix(msg, "You feel very naive"))
        {
            my_need_stat_check[STAT_WIS] = true;
        }
        if (prefix(msg, "You feel less naive"))
        {
            my_need_stat_check[STAT_WIS] = true;
        }
        if (prefix(msg, "Wow!  You feel very wise"))
        {
            my_need_stat_check[STAT_WIS] = true;
        }

        /* need to check dex */
        if (prefix(msg, "You feel very clumsy"))
        {
            my_need_stat_check[STAT_DEX] = true;
        }
        if (prefix(msg, "You feel less clumsy"))
        {
            my_need_stat_check[STAT_DEX] = true;
        }
        if (prefix(msg, "Wow!  You feel very dextrous"))
        {
            my_need_stat_check[STAT_DEX] = true;
        }

        /* need to check con */
        if (prefix(msg, "You feel very sickly"))
        {
            my_need_stat_check[STAT_CON] = true;
        }
        if (prefix(msg, "You feel less sickly"))
        {
            my_need_stat_check[STAT_CON] = true;
        }
        if (prefix(msg, "Wow!  You feel very healthy"))
        {
            my_need_stat_check[STAT_CON] = true;
        }
    }

    /* time attacks, just do all stats. */
    if (prefix(msg, "You're not as"))
    {
        for (i = 0; i < STAT_MAX; i++)
            my_need_stat_check[i] = true;
    }

    /* Nexus attacks, need to check everything! */
    if (prefix(msg, "Your body starts to scramble..."))
    {
        for (i = 0; i < STAT_MAX; i++)
        {
            my_need_stat_check[i] = true;
            /* max stats may have lowered */
            my_stat_max[i] = 0;
        }
    }

    /* amnesia attacks, re-id wands, staves, equipment. */
    if (prefix(msg, "You feel your memories fade."))
    {
        /* Set the borg flag */
        borg_skill[BI_ISFORGET] = true;
    }
    if (streq(msg, "Your memories come flooding back."))
    {
        borg_skill[BI_ISFORGET] = false;
    }

    if (streq(msg, "You have been knocked out."))
    {
        borg_note("Ignoring Messages While KO'd");
        borg_dont_react = true;
    }
    if (streq(msg, "You are paralyzed"))
    {
        borg_note("Ignoring Messages While Paralyzed");
        borg_dont_react = true;
    }

    /* Hallucination -- Open */
    if (streq(msg, "You feel drugged!"))
    {
        borg_note("# Hallucinating.  Special control of wanks.");
        borg_skill[BI_ISIMAGE] = true;
    }

    /* Hallucination -- Close */
    if (streq(msg, "You can see clearly again."))
    {
        borg_note("# Hallucination ended.  Normal control of wanks.");
        borg_skill[BI_ISIMAGE] = false;
    }

    /* Hit somebody */
    if (prefix(msg, "You hit "))
    {
        tmp = strlen("You hit ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%s", who);
        borg_react(msg, buf);
        return;
    }
    if (prefix(msg, "You bite "))
    {
        tmp = strlen("You hit ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody */
    if (prefix(msg, "You miss "))
    {
        tmp = strlen("You miss ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody (because of fear) */
    if (prefix(msg, "You are too afraid to attack "))
    {
        tmp = strlen("You are too afraid to attack ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%s", who);
        borg_react(msg, buf);
        return;
    }

    /* "Your <equipment> is unaffected!"
     * Note that this check must be before the suffix_pain
     * because suffix_pain will look for 'is unaffected!' and
     * assume it is talking about a monster which in turn will
     * yeild to the Player Ghost being created.
     */
    if (prefix(msg, "Your "))
    {
        if (suffix(msg, " is unaffected!"))
        {
            /* Your equipment ignored the attack.
             * Ignore the message
             */
            return;
        }
    }
    else
    {
        /* "It screams in pain." (etc) */
        for (i = 0; suffix_pain[i]; i++)
        {
            /* "It screams in pain." (etc) */
            if (suffix(msg, suffix_pain[i]))
            {
                tmp = strlen(suffix_pain[i]);
                strnfmt(who, 1 + len - tmp, "%s", msg);
                strnfmt(buf, 256, "PAIN:%s", who);
                borg_react(msg, buf);
                return;
            }
        }


        /* "You have killed it." (etc) */
        for (i = 0; prefix_kill[i]; i++)
        {
            /* "You have killed it." (etc) */
            if (prefix(msg, prefix_kill[i]))
            {
                tmp = strlen(prefix_kill[i]);
                strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
                strnfmt(buf, 256, "KILL:%s", who);
                borg_react(msg, buf);
                return;
            }
        }


        /* "It dies." (etc) */
        for (i = 0; suffix_died[i]; i++)
        {
            /* "It dies." (etc) */
            if (suffix(msg, suffix_died[i]))
            {
                tmp = strlen(suffix_died[i]);
                strnfmt(who, 1 + len - tmp, "%s", msg);
                strnfmt(buf, 256, "DIED:%s", who);
                borg_react(msg, buf);
                return;
            }
        }

        /* "It blinks or telports." (etc) */
        for (i = 0; suffix_blink[i]; i++)
        {
            /* "It teleports." (etc) */
            if (suffix(msg, suffix_blink[i]))
            {
                tmp = strlen(suffix_blink[i]);
                strnfmt(who, 1 + len - tmp, "%s", msg);
                strnfmt(buf, 256, "BLINK:%s", who);
                borg_react(msg, buf);
                return;
            }
        }

        /* "It misses you." */
        if (suffix(msg, " misses you."))
        {
            tmp = strlen(" misses you.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "MISS_BY:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* "It is repelled.." */
        /* treat as a miss */
        if (suffix(msg, " is repelled."))
        {
            tmp = strlen(" is repelled.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "MISS_BY:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* "It hits you." (etc) */
        for (i = 0; suffix_hit_by.messages[i].message_p1; i++)
        {
            /* "It hits you." (etc) */
            if (borg_message_contains(msg, &suffix_hit_by.messages[i]))
            {
                char* start = strstr(msg, suffix_hit_by.messages[i].message_p1);
                if (start)
                {
                    strnfmt(who, (start - msg), "%s", msg);
                    strnfmt(buf, 256, "HIT_BY:%s", who);
                    borg_react(msg, buf);

                    /* If I was hit, then I am not on a glyph */
                    if (track_glyph.num)
                    {
                        /* erase them all and
                         * allow the borg to scan the screen and rebuild the array.
                         * He won't see the one under him though.  So a special check
                         * must be made.
                         */
                         /* Remove the entire array */
                        for (i = 0; i < track_glyph.num; i++)
                        {
                            /* Stop if we already new about this glyph */
                            track_glyph.x[i] = 0;
                            track_glyph.y[i] = 0;
                        }
                        track_glyph.num = 0;

                        /* Check for glyphs under player -- Cheat*/
                        if (square_iswarded(cave, loc(c_x, c_y)))
                        {
                            track_glyph.x[track_glyph.num] = c_x;
                            track_glyph.y[track_glyph.num] = c_y;
                            track_glyph.num++;
                        }
                    }
                    return;
                }
            }
        }

        for (i = 0; spell_invis_msgs.messages[i].message_p1; i++)
        {
            /* get rid of the messages that aren't for invisible spells */
            if (!prefix(msg, "Something ") && !prefix(msg, "You "))
                break;
            if (borg_message_contains(msg, &spell_invis_msgs.messages[i]))
            {
                strnfmt(buf, 256, "SPELL_%03d:%s", spell_invis_msgs.index[i], "Something");
                borg_react(msg, buf);
                return;
            }
        }
        for (i = 0; spell_msgs.messages[i].message_p1; i++)
        {
            if (borg_message_contains(msg, &spell_msgs.messages[i]))
            {
                char* start = strstr(msg, spell_msgs.messages[i].message_p1);
                if (start)
                {
                    strnfmt(who, (start - msg), "%s", msg);
                    strnfmt(buf, 256, "SPELL_%03d:%s", spell_msgs.index[i], who);
                    borg_react(msg, buf);
                    return;
                }
            }
        }


        /* State -- Asleep */
        if (suffix(msg, " falls asleep!"))
        {
            tmp = strlen(" falls asleep!");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_SLEEP:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- confused */
        if (suffix(msg, " looks confused."))
        {
            tmp = strlen(" looks confused.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_CONFUSED:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- confused */
        if (suffix(msg, " looks more confused."))
        {
            tmp = strlen(" looks more confused.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_CONFUSED:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Asleep */
        if (suffix(msg, " wakes up."))
        {
            tmp = strlen(" wakes up.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE_AWAKE:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Afraid */
        if (suffix(msg, " flees in terror!"))
        {
            tmp = strlen(" flees in terror!");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__FEAR:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Afraid */
        if (suffix(msg, " recovers his courage."))
        {
            tmp = strlen(" recovers his courage.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__BOLD:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Afraid */
        if (suffix(msg, " recovers her courage."))
        {
            tmp = strlen(" recovers her courage.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__BOLD:%s", who);
            borg_react(msg, buf);
            return;
        }

        /* State -- Not Afraid */
        if (suffix(msg, " recovers its courage."))
        {
            tmp = strlen(" recovers its courage.");
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "STATE__BOLD:%s", who);
            borg_react(msg, buf);
            return;
        }
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "The door appears to be broken."))
    {
        /* Only process open doors */
        if (ag->feat == FEAT_OPEN)
        {
            /* Mark as broken */
            ag->feat = FEAT_BROKEN;

            /* Clear goals */
            goal = 0;
        }
        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "This seems to be permanent rock."))
    {
        /* Only process walls */
        if ((ag->feat >= FEAT_GRANITE) && (ag->feat <= FEAT_PERM))
        {
            /* Mark the wall as permanent */
            ag->feat = FEAT_PERM;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the granite wall."))
    {
        /* reseting my panel clock */
        time_this_panel = 1;

        /* Only process walls */
        if ((ag->feat >= FEAT_GRANITE) && (ag->feat <= FEAT_PERM))
        {
            /* Mark the wall as granite */
            ag->feat = FEAT_GRANITE;

            /* Clear goals */
            goal = 0;
        }

        return;
    }


    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the quartz vein."))
    {
        /* Process magma veins with treasure */
        if (ag->feat == FEAT_MAGMA_K)
        {
            /* Mark the vein */
            ag->feat = FEAT_QUARTZ_K;

            /* Clear goals */
            goal = 0;
        }

        /* Process magma veins */
        else if (ag->feat == FEAT_MAGMA)
        {
            /* Mark the vein */
            ag->feat = FEAT_QUARTZ;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Feature XXX XXX XXX */
    if (streq(msg, "You tunnel into the magma vein."))
    {
        /* Process quartz veins with treasure */
        if (ag->feat == FEAT_QUARTZ_K)
        {
            /* Mark the vein */
            ag->feat = FEAT_MAGMA_K;

            /* Clear goals */
            goal = 0;
        }

        /* Process quartz veins */
        else if (ag->feat == FEAT_QUARTZ)
        {
            /* Mark the vein */
            ag->feat = FEAT_MAGMA;

            /* Clear goals */
            goal = 0;
        }

        return;
    }

    /* Word of Recall -- Ignition */
    if (prefix(msg, "The air about you becomes "))
    {
        /* Initiate recall */
        /* Guess how long it will take to lift off */
        goal_recalling = 15000 + 5000; /* Guess. game turns x 1000 ( 15+rand(20))*/
        return;
    }

    /* Word of Recall -- Lift off */
    if (prefix(msg, "You feel yourself yanked "))
    {
        /* Recall complete */
        goal_recalling = 0;
        return;
    }

    /* Word of Recall -- Cancelled */
    if (prefix(msg, "A tension leaves "))
    {
        /* Hack -- Oops */
        goal_recalling = 0;
        return;
    }

    /* Wearing Cursed Item */
    if (prefix(msg, "Oops! It feels deathly cold!"))
    {
        /* this should only happen with STICKY items, The Crown of Morgoth or The One Ring */
/* !FIX !TODO !AJG handle crown eventually */
        return;
    }

    /* protect from evil */
    if (prefix(msg, "You feel safe from evil!"))
    {
        borg_prot_from_evil = true;
        return;
    }
    if (prefix(msg, "You no longer feel safe from evil."))
    {
        borg_prot_from_evil = false;
        return;
    }
    /* haste self */
    if (prefix(msg, "You feel yourself moving faster!"))
    {
        borg_speed = true;
        return;
    }
    if (prefix(msg, "You feel yourself slow down."))
    {
        borg_speed = false;
        return;
    }
    /* Bless */
    if (prefix(msg, "You feel righteous"))
    {
        borg_bless = true;
        return;
    }
    if (prefix(msg, "The prayer has expired."))
    {
        borg_bless = false;
        return;
    }

    /* fastcast */
    if (prefix(msg, "You feel your mind accelerate."))
    {
        borg_fastcast = true;
        return;
    }
    if (prefix(msg, "You feel your mind slow again."))
    {
        borg_fastcast = false;
        return;
    }

    /* hero */
    if (prefix(msg, "You feel like a hero!"))
    {
        borg_hero = true;
        return;
    }
    if (prefix(msg, "You no longer feel heroic."))
    {
        borg_hero = false;
        return;
    }

    /* berserk */
    if (prefix(msg, "You feel like a killing machine!"))
    {
        borg_berserk = true;
        return;
    }
    if (prefix(msg, "You no longer feel berserk."))
    {
        borg_berserk = false;
        return;
    }

    /* Sense Invisible */
    if (prefix(msg, "Your eyes feel very sensitive!"))
    {
        borg_see_inv = 30000;
        return;
    }
    if (prefix(msg, "Your eyes no longer feel so sensitive."))
    {
        borg_see_inv = 0;
        return;
    }

    /* check for wall blocking but not when confused*/
    if ((prefix(msg, "There is a wall ") &&
        (!borg_skill[BI_ISCONFUSED])))
    {
        my_need_redraw = true;
        my_need_alter = true;
        goal = 0;
        return;
    }


    /* check for closed door but not when confused*/
    if ((prefix(msg, "There is a closed door blocking your way.") &&
        (!borg_skill[BI_ISCONFUSED] &&
            !borg_skill[BI_ISIMAGE])))
    {
        my_need_redraw = true;
        my_need_alter = true;
        goal = 0;
        return;
    }

    /* check for mis-alter command.  Sometime induced by never_move guys*/
    if (prefix(msg, "You spin around.") &&
        !borg_skill[BI_ISCONFUSED])
    {
        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {

            borg_kill* kill = &borg_kills[i];

            /* Skip dead monsters */
            if (!kill->r_idx) continue;

            /* Now do distance considerations */
            x9 = kill->x;
            y9 = kill->y;

            /* Distance components */
            ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
            ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

            /* Distance */
            d = MAX(ax, ay);

            /* if the guy is too close then delete him. */
            if (d < 4)
            {
                /* Hack -- kill em */
                borg_delete_kill(i);
            }
        }

        my_no_alter = true;
        goal = 0;
        return;
    }

    /* Check for the missing staircase */
    if (suffix(msg, " staircase here."))
    {
        /* make sure the aligned dungeon is on */

        /* make sure the borg does not think he's on one */
        /* Remove all stairs from the array. */
        track_less.num = 0;
        track_more.num = 0;
        borg_on_dnstairs = false;
        borg_on_upstairs = false;
        borg_grids[c_y][c_x].feat = FEAT_BROKEN;

        return;
    }

    /* Feature XXX XXX XXX */
    if (prefix(msg, "You see nothing there "))
    {
        ag->feat = FEAT_BROKEN;

        my_no_alter = true;
        /* Clear goals */
        goal = 0;
        return;

    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Illegal "))
    {
        /* Hack -- Oops */
        borg_respawning = 7;
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel += 100;
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "You have nothing to identify"))
    {
        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel += 100;

        /* ID all items (equipment) */
        for (i = INVEN_WIELD; i <= INVEN_FEET; i++)
        {
            borg_item* item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            item->ident = true;
        }

        /* ID all items  (inventory) */
        for (i = 0; i <= z_info->pack_size; i++)
        {
            borg_item* item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            item->ident = true;
        }
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Identifying The Phial"))
    {

        /* ID item (equipment) */
        borg_item* item = &borg_items[INVEN_LIGHT];
        item->ident = true;

        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel += 100;
    }

    /* resist acid */
    if (prefix(msg, "You feel resistant to acid!"))
    {
        borg_skill[BI_TRACID] = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to acid."))
    {
        borg_skill[BI_TRACID] = false;
        return;
    }
    /* resist electricity */
    if (prefix(msg, "You feel resistant to electricity!"))
    {
        borg_skill[BI_TRELEC] = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to electricity."))
    {
        borg_skill[BI_TRELEC] = false;
        return;
    }
    /* resist fire */
    if (prefix(msg, "You feel resistant to fire!"))
    {
        borg_skill[BI_TRFIRE] = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to fire."))
    {
        borg_skill[BI_TRFIRE] = false;
        return;
    }
    /* resist cold */
    if (prefix(msg, "You feel resistant to cold!"))
    {
        borg_skill[BI_TRCOLD] = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to cold."))
    {
        borg_skill[BI_TRCOLD] = false;
        return;
    }
    /* resist poison */
    if (prefix(msg, "You feel resistant to poison!"))
    {
        borg_skill[BI_TRPOIS] = true;
        return;
    }
    if (prefix(msg, "You are no longer resistant to poison."))
    {
        borg_skill[BI_TRPOIS] = false;
        return;
    }

    /* Shield */
    if (prefix(msg, "A mystic shield forms around your body!") ||
        prefix(msg, "Your skin turns to stone."))
    {
        borg_shield = true;
        return;
    }
    if (prefix(msg, "Your mystic shield crumbles away.") ||
        prefix(msg, "A fleshy shade returns to your skin."))
    {
        borg_shield = false;
        return;
    }

    /* Glyph of Warding (the spell no longer gives a report)*/
    /* Sadly  Rune of Protection has no message */
    if (prefix(msg, "You inscribe a mystic symbol on the ground!"))
    {
        /* Check for an existing glyph */
        for (i = 0; i < track_glyph.num; i++)
        {
            /* Stop if we already new about this glyph */
            if ((track_glyph.x[i] == c_x) && (track_glyph.y[i] == c_y)) break;
        }

        /* Track the newly discovered glyph */
        if ((i == track_glyph.num) && (i < track_glyph.size))
        {
            borg_note("# Noting the creation of a glyph.");
            track_glyph.x[i] = c_x;
            track_glyph.y[i] = c_y;
            track_glyph.num++;
        }

        return;
    }
    if (prefix(msg, "The rune of protection is broken!"))
    {
        /* we won't know which is broken so erase them all and
         * allow the borg to scan the screen and rebuild the array.
         * He won't see the one under him though.  So a special check
         * must be made.
         */

         /* Remove the entire array */
        for (i = 0; i < track_glyph.num; i++)
        {
            /* Stop if we already new about this glyph */
            track_glyph.x[i] = 0;
            track_glyph.y[i] = 0;

        }
        /* no known glyphs */
        track_glyph.num = 0;

        /* Check for glyphs under player -- Cheat*/
        if (square_iswarded(cave, loc(c_x, c_y)))
        {
            track_glyph.x[track_glyph.num] = c_x;
            track_glyph.y[track_glyph.num] = c_y;
            track_glyph.num++;
        }
        return;
    }
    /* failed glyph spell message */
    if (prefix(msg, "The object resists the spell") ||
        prefix(msg, "There is no clear floor"))
    {

        /* Forget the newly created-though-failed  glyph */
        track_glyph.x[track_glyph.num] = 0;
        track_glyph.y[track_glyph.num] = 0;
        track_glyph.num--;

        /* note it */
        borg_note("# Removing the Glyph under me, placing with broken door.");

        /* mark that we are not on a clear spot.  The borg ignores
         * broken doors and this will keep him from casting it again.
         */
        ag->feat = FEAT_BROKEN;
        return;
    }

    /* Removed rubble.  Important when out of lite */
    if (prefix(msg, "You have removed the "))
    {
        int x, y;
        /* remove rubbles from array */
        for (y = c_y - 1; y < c_y + 1; y++)
        {
            for (x = c_x - 1; x < c_x + 1; x++)
            {
                /* replace all rubble with broken doors, the borg ignores
                 * broken doors.  This routine is only needed if the borg
                 * is out of lite and searching in the dark.
                 */
                if (borg_skill[BI_CURLITE]) continue;

                if (ag->feat == FEAT_RUBBLE) ag->feat = FEAT_BROKEN;
            }
        }
        return;
    }

    if (prefix(msg, "The enchantment failed"))
    {
        /* reset our panel clock for this */
        time_this_panel = 1;
        return;
    }

    /* need to kill monsters when WoD is used */
    if (prefix(msg, "There is a searing blast of light!"))
    {
        /* Examine all the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill* kill = &borg_kills[i];

            x9 = kill->x;
            y9 = kill->y;

            /* Skip dead monsters */
            if (!kill->r_idx) continue;

            /* Distance components */
            ax = (x9 > c_x) ? (x9 - c_x) : (c_x - x9);
            ay = (y9 > c_y) ? (y9 - c_y) : (c_y - y9);

            /* Distance */
            d = MAX(ax, ay);

            /* Minimal distance */
            if (d > 12) continue;

            /* Hack -- kill em */
            borg_delete_kill(i);
        }

        /* Remove the region fear as well */
        borg_fear_region[c_y / 11][c_x / 11] = 0;

        return;
    }

    /* Be aware and concerned of busted doors */
    if (prefix(msg, "You hear a door burst open!"))
    {
        /* on level 1 and 2 be concerned.  Could be Grip or Fang */
        if (borg_skill[BI_CDEPTH] <= 3 && borg_skill[BI_CLEVEL] <= 5) scaryguy_on_level = true;
    }

    /* Some spells move the borg from his grid */
    if (prefix(msg, "commands you to return.") ||
        prefix(msg, "teleports you away.") ||
        prefix(msg, "gestures at your feet."))
    {
        /* If in Lunal mode better shut that off, he is not on the stairs anymore */
        borg_lunal_mode = false;
        borg_note("# Disconnecting Lunal Mode due to monster spell.");
    }

    /* Sometimes the borg will overshoot the range limit of his shooter */
    if (prefix(msg, "Target out of range."))
    {
        /* Fire Anyway? [Y/N] */
        borg_keypress('y');
    }

    /* Feelings about the level */
    for (i = 0; prefix_feeling_danger[i]; i++)
    {
        /* "You feel..." (etc) */
        if (prefix(msg, prefix_feeling_danger[i]))
        {
            strnfmt(buf, 256, "FEELING_DANGER:%d", i);
            borg_react(msg, buf);
            return;
        }
    }

    for (i = 0; suffix_feeling_stuff[i]; i++)
    {
        /* "You feel..." (etc) */
        if (suffix(msg, suffix_feeling_stuff[i]))
        {
            strnfmt(buf, 256, "FEELING_STUFF:%d", i);
            borg_react(msg, buf);
            return;
        }
    }
}



/*
 * Parse a message, piece of a message, or set of messages.
 *
 * We must handle long messages which are "split" into multiple
 * pieces, and also multiple messages which may be "combined"
 * into a single set of messages.
 */
static void borg_parse(char* msg)
{
    static int len = 0;
    static char buf[1024];

    /* Note the long message */
    if (borg_cfg[BORG_VERBOSE] && msg) 
        borg_note(format("# Parsing msg <%s>", msg));

    /* Flush messages */
    if (len && (!msg || (msg[0] != ' ')))
    {
        int i, j;

        /* Split out punctuation */
        for (j = i = 0; i < len - 1; i++)
        {
            /* Check for punctuation */
            if ((buf[i] == '.') ||
                (buf[i] == '!') ||
                (buf[i] == '?') ||
                (buf[i] == '"'))
            {
                /* Require space */
                if (buf[i + 1] == ' ')
                {
                    /* Terminate */
                    buf[i + 1] = '\0';

                    /* Parse fragment */
                    borg_parse_aux(buf + j, (i + 1) - j);

                    /* Restore */
                    buf[i + 1] = ' ';

                    /* Advance past spaces */
                    for (j = i + 2; buf[j] == ' '; j++) /* loop */;
                }
            }
        }

        /* Parse tail */
        borg_parse_aux(buf + j, len - j);

        /* Forget */
        len = 0;
    }


    /* No message */
    if (!msg)
    {
        /* Start over */
        len = 0;
    }

    /* Continued message */
    else if (msg[0] == ' ')
    {
        /* Collect, verify, and grow */
        len += strnfmt(buf + len, 1024 - len, "%s", msg + 1);
    }

    /* New message */
    else
    {
        /* Collect, verify, and grow */
        len = strnfmt(buf, 1024, "%s", msg);
    }
}



#ifndef BABLOS

#if 0
static int16_t stat_use[6];

static int adjust_stat_borg(int value, int amount, int borg_roll)
{
    return (modify_stat_value(value, amount));
}

static void get_stats_borg_aux(void)
{
    int i, j;

    int bonus;

    int dice[18];

    /* Roll and verify some stats */
    while (true)
    {
        /* Roll some dice */
        for (j = i = 0; i < 18; i++)
        {
            /* Roll the dice */
            dice[i] = randint1(3 + i % 3);

            /* Collect the maximum */
            j += dice[i];
        }

        /* Verify totals */
        if ((j > 42) && (j < 54)) break;
    }

    /* Roll the stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Extract 5 + 1d3 + 1d4 + 1d5 */
        j = 5 + dice[3 * i] + dice[3 * i + 1] + dice[3 * i + 2];

        /* Save that value */
        player->stat_max[i] = j;

        /* Obtain a "bonus" for "race" and "class" */
        bonus = player->race->r_adj[i] + player->class->c_adj[i];

        /* Variable stat maxes */
        /* Start fully healed */
        player->stat_cur[i] = player->stat_max[i];

        /* Efficiency -- Apply the racial/class bonuses */
        stat_use[i] = modify_stat_value(player->stat_max[i], bonus);
    }
}
/*
 * Roll for a new characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats_borg(void)
{
    int i;

    int stat_limit[6];

    int32_t borg_round = 0L;

    /* load up min. stats */
    stat_limit[0] = 14; /* Str */
    stat_limit[1] = 0; /* Int */
    stat_limit[2] = 0; /* Wis */
    stat_limit[3] = 14; /* Dex */
    stat_limit[4] = 14;/* Con */
    stat_limit[5] = 0; /* Chr */

    switch (player->class->cidx)
    {
    case CLASS_WARRIOR:
    stat_limit[0] = 17;
    stat_limit[3] = 16;
    break;
    case CLASS_MAGE:
    stat_limit[1] = 17;
    stat_limit[3] = 16;
    break;
    case CLASS_PRIEST:
    stat_limit[2] = 17;
    stat_limit[0] = 16;
    break;
    case CLASS_ROGUE:
    stat_limit[0] = 17;
    stat_limit[3] = 16;
    break;
    case CLASS_PALADIN:
    stat_limit[0] = 17;
    stat_limit[3] = 16;
    break;
    case CLASS_RANGER:
    stat_limit[0] = 17;
    stat_limit[3] = 16;
    break;
    }

    /* Minimal stats selected */
    if (stat_limit[0] + stat_limit[1] + stat_limit[2] + stat_limit[3] +
        stat_limit[4] + stat_limit[5] >= 1)
    {
        /* Auto-roll */
        while (1)
        {
            bool accept = true;

            /* Get a new character */
            get_stats_borg_aux();

            /* Advance the round */
            borg_round++;

            /* Hack -- Prevent overflow */
            if (borg_round >= 750000L)
            {
                borg_note("# Minimal Stats too high.");
                break;
            }

            /* Check and count acceptable stats */
            for (i = 0; i < A_MAX; i++)
            {
                /* This stat is okay (JesperN)*/
                if (player->stat_max[i] >= stat_limit[i])

                {
                    accept = true;
                }

                /* This stat is not okay */
                else
                {
                    accept = false;
                    break;
                }
            }

            /* Break if "happy" */
            if (accept) break;
        } /* while */

    /* Note the number of rolls to achieve stats */
        borg_note(format("# Minimal stats rolled in %d turns.", borg_round));

    } /* minimal stats */
    else /* Otherwise just get a character */
    {
        borg_note("# Rolling random stats.");
        get_stats_borg_aux();
    }


}

/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra_borg(void)
{
    int i, j, min_value, max_value;


    /* Level one */
    player->max_lev = player->lev = 1;

    /* Experience factor */
    player->expfact = player->race->r_exp + player->class->c_exp;

    /* Hitdice */
    player->hitdie = player->race->r_mhp + player->class->c_mhp;

    /* Initial hitpoints */
    player->mhp = player->hitdie;

    /* Minimum hitpoints at highest level */
    min_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 3) / 8;
    min_value += PY_MAX_LEVEL;

    /* Maximum hitpoints at highest level */
    max_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 5) / 8;
    max_value += PY_MAX_LEVEL;

    /* Pre-calculate level 1 hitdice */
    player->player_hp[0] = player->hitdie;

    /* Roll out the hitpoints */
    while (true)
    {
        /* Roll the hitpoint values */
        for (i = 1; i < PY_MAX_LEVEL; i++)
        {
            j = randint0(player->hitdie);
            player->player_hp[i] = player->player_hp[i - 1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (player->player_hp[PY_MAX_LEVEL - 1] < min_value) continue;
        if (player->player_hp[PY_MAX_LEVEL - 1] > max_value) continue;

        /* Acceptable */
        break;
    }
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history_borg(void)
{
    int i, roll, social_class;
    struct history_chart* chart;
    struct history_entry* entry;
    char* res = NULL;


    /* Clear the previous history strings */
    player->history[0] = '\0';


    /* Initial social class */
    social_class = randint1(4);

    /* Starting place */
    chart = player->race->history;


    /* Process the history */
    while (chart) {
        roll = randint1(100);
        for (entry = chart->entries; entry; entry = entry->next)
            if (roll <= entry->roll)
                break;
        assert(entry);

        res = string_append(res, entry->text);
        social_class += entry->bonus - 50;
        chart = entry->succ;
    }
#if 0
    while (chart)
    {
        /* Start over */
        i = 0;

        /* Roll for nobility */
        roll = randint1(100);

        /* Get the proper entry in the table */
        while ((chart != h_info[i].chart) || (roll > h_info[i].roll)) i++;

        /* Get the textual history */
        my_strcat(player->history, h_info[i].text, sizeof(player->history));

        /* Add in the social class */
        social_class += (int)(h_info[i].bonus) - 50;

        /* Enter the next chart */
        chart = h_info[i].next;
    }
#endif
    /* Verify social class */
    if (social_class > 100) social_class = 100;
    else if (social_class < 1) social_class = 1;

    /* Save the social class */
    player->sc = social_class;

}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw_borg(void)
{
    /* Calculate the age */
    player->age = player->race->b_age + randint0(player->race->m_age);

    /* Calculate the height/weight for males */
    if (player->psex == SEX_MALE)
    {
        player->ht = Rand_normal(player->race->m_b_ht, player->race->m_m_ht);
        player->wt = Rand_normal(player->race->m_b_wt, player->race->m_m_wt);
    }

    /* Calculate the height/weight for females */
    else if (player->psex == SEX_FEMALE)
    {
        player->ht = Rand_normal(player->race->f_b_ht, player->race->f_m_ht);
        player->wt = Rand_normal(player->race->f_b_wt, player->race->f_m_wt);
    }
}




/*
 * Get the player's starting money
 */
static void get_money_borg(void)
{
    int i;

    int gold = 0;

    /* Social Class determines starting gold */
    gold = (player->sc * 6) + randint1(100) + 300;

    /* Process the stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Mega-Hack -- reduce gold for high stats */
        if (stat_use[i] >= 18 + 50) gold -= 300;
        else if (stat_use[i] >= 18 + 20) gold -= 200;
        else if (stat_use[i] > 18) gold -= 150;
        else gold -= (stat_use[i] - 8) * 10;
    }

    /* Minimum 100 gold */
    if (gold < 100) gold = 100;

    /* Save the gold */
    player->au = gold;
}
#endif

/*
 * Name segments for random player names
 * Copied Cth by DvE
 * Copied from borgband by APW
 */

 /* Dwarves */
static const char* dwarf_syllable1[] =
{
    "B", "D", "F", "G", "Gl", "H", "K", "L", "M", "N", "R", "S", "T", "Th", "V",
};

static const char* dwarf_syllable2[] =
{
    "a", "e", "i", "o", "oi", "u",
};

static const char* dwarf_syllable3[] =
{
    "bur", "fur", "gan", "gnus", "gnar", "li", "lin", "lir", "mli", "nar", "nus", "rin", "ran", "sin", "sil", "sur",
};

/* Elves */
static const char* elf_syllable1[] =
{
    "Al", "An", "Bal", "Bel", "Cal", "Cel", "El", "Elr", "Elv", "Eow", "Ear", "F", "Fal", "Fel", "Fin", "G", "Gal", "Gel", "Gl", "Is", "Lan", "Leg", "Lom", "N", "Nal", "Nel",  "S", "Sal", "Sel", "T", "Tal", "Tel", "Thr", "Tin",
};

static const char* elf_syllable2[] =
{
    "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra", "ilma", "il-Ga", "ili", "o", "orfi", "u", "y",
};

static const char* elf_syllable3[] =
{
    "l", "las", "lad", "ldor", "ldur", "linde", "lith", "mir", "n", "nd", "ndel", "ndil", "ndir", "nduil", "ng", "mbor", "r", "rith", "ril", "riand", "rion", "s", "thien", "viel", "wen", "wyn",
};

/* Gnomes */
static const char* gnome_syllable1[] =
{
    "Aar", "An", "Ar", "As", "C", "H", "Han", "Har", "Hel", "Iir", "J", "Jan", "Jar", "K", "L", "M", "Mar", "N", "Nik", "Os", "Ol", "P", "R", "S", "Sam", "San", "T", "Ter", "Tom", "Ul", "V", "W", "Y",
};

static const char* gnome_syllable2[] =
{
    "a", "aa",  "ai", "e", "ei", "i", "o", "uo", "u", "uu",
};

static const char* gnome_syllable3[] =
{
    "ron", "re", "la", "ki", "kseli", "ksi", "ku", "ja", "ta", "na", "namari", "neli", "nika", "nikki", "nu", "nukka", "ka", "ko", "li", "kki", "rik", "po", "to", "pekka", "rjaana", "rjatta", "rjukka", "la", "lla", "lli", "mo", "nni",
};

/* Hobbit */
static const char* hobbit_syllable1[] =
{
    "B", "Ber", "Br", "D", "Der", "Dr", "F", "Fr", "G", "H", "L", "Ler", "M", "Mer", "N", "P", "Pr", "Per", "R", "S", "T", "W",
};

static const char* hobbit_syllable2[] =
{
    "a", "e", "i", "ia", "o", "oi", "u",
};

static const char* hobbit_syllable3[] =
{
    "bo", "ck", "decan", "degar", "do", "doc", "go", "grin", "lba", "lbo", "lda", "ldo", "lla", "ll", "lo", "m", "mwise", "nac", "noc", "nwise", "p", "ppin", "pper", "tho", "to",
};

/* Human */
static const char* human_syllable1[] =
{
    "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar", "B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et", "Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha", "Ib", "Jer", "K", "Ka", "Ked", "L", "Loth", "Lar", "Leg", "M", "Mir", "N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T", "Tr", "Th", "V", "Y", "Z", "W", "Wic",
};

static const char* human_syllable2[] =
{
    "a", "ae", "au", "ao", "are", "ale", "ali", "ay", "ardo", "e", "ei", "ea", "eri", "era", "ela", "eli", "enda", "erra", "i", "ia", "ie", "ire", "ira", "ila", "ili", "ira", "igo", "o", "oa", "oi", "oe", "ore", "u", "y",
};

static const char* human_syllable3[] =
{
    "a", "and", "b", "bwyn", "baen", "bard", "c", "ctred", "cred", "ch", "can", "d", "dan", "don", "der", "dric", "dfrid", "dus", "f", "g", "gord", "gan", "l", "li", "lgrin", "lin", "lith", "lath", "loth", "ld", "ldric", "ldan", "m", "mas", "mos", "mar", "mond", "n", "nydd", "nidd", "nnon", "nwan", "nyth", "nad", "nn", "nnor", "nd", "p", "r", "ron", "rd", "s", "sh", "seth", "sean", "t", "th", "tha", "tlan", "trem", "tram", "v", "vudd", "w", "wan", "win", "wyn", "wyr", "wyr", "wyth",
};

/* Orc */
static const char* orc_syllable1[] =
{
    "B", "Er", "G", "Gr", "H", "P", "Pr", "R", "V", "Vr", "T", "Tr", "M", "Dr",
};

static const char* orc_syllable2[] =
{
    "a", "i", "o", "oo", "u", "ui",
};

static const char* orc_syllable3[] =
{
    "dash", "dish", "dush", "gar", "gor", "gdush", "lo", "gdish", "k", "lg", "nak", "rag", "rbag", "rg", "rk", "ng", "nk", "rt", "ol", "urk", "shnak", "mog", "mak", "rak",
};


/*
 * Random Name Generator
 * based on a Javascript by Michael Hensley
 * "http://geocities.com/timessquare/castle/6274/"
 * Copied from Cth by DvE
 * Copied from borgband by APW
 */
static void create_random_name(int race, char* name, size_t name_len)
{
    /* Paranoia */
    if (!name) return;

    /* Select the monster type */
    switch (race)
    {
        /* Create the monster name */
    case RACE_DWARF:
    my_strcpy(name, dwarf_syllable1[randint0(sizeof(dwarf_syllable1) / sizeof(char*))], name_len);
    my_strcat(name, dwarf_syllable2[randint0(sizeof(dwarf_syllable2) / sizeof(char*))], name_len);
    my_strcat(name, dwarf_syllable3[randint0(sizeof(dwarf_syllable3) / sizeof(char*))], name_len);
    break;
    case RACE_ELF:
    case RACE_HALF_ELF:
    case RACE_HIGH_ELF:
    my_strcpy(name, elf_syllable1[randint0(sizeof(elf_syllable1) / sizeof(char*))], name_len);
    my_strcat(name, elf_syllable2[randint0(sizeof(elf_syllable2) / sizeof(char*))], name_len);
    my_strcat(name, elf_syllable3[randint0(sizeof(elf_syllable3) / sizeof(char*))], name_len);
    break;
    case RACE_GNOME:
    my_strcpy(name, gnome_syllable1[randint0(sizeof(gnome_syllable1) / sizeof(char*))], name_len);
    my_strcat(name, gnome_syllable2[randint0(sizeof(gnome_syllable2) / sizeof(char*))], name_len);
    my_strcat(name, gnome_syllable3[randint0(sizeof(gnome_syllable3) / sizeof(char*))], name_len);
    break;
    case RACE_HOBBIT:
    my_strcpy(name, hobbit_syllable1[randint0(sizeof(hobbit_syllable1) / sizeof(char*))], name_len);
    my_strcat(name, hobbit_syllable2[randint0(sizeof(hobbit_syllable2) / sizeof(char*))], name_len);
    my_strcat(name, hobbit_syllable3[randint0(sizeof(hobbit_syllable3) / sizeof(char*))], name_len);
    break;
    case RACE_HUMAN:
    case RACE_DUNADAN:
    my_strcpy(name, human_syllable1[randint0(sizeof(human_syllable1) / sizeof(char*))], name_len);
    my_strcat(name, human_syllable2[randint0(sizeof(human_syllable2) / sizeof(char*))], name_len);
    my_strcat(name, human_syllable3[randint0(sizeof(human_syllable3) / sizeof(char*))], name_len);
    break;
    case RACE_HALF_ORC:
    case RACE_HALF_TROLL:
    case RACE_KOBOLD:
    my_strcpy(name, orc_syllable1[randint0(sizeof(orc_syllable1) / sizeof(char*))], name_len);
    my_strcat(name, orc_syllable2[randint0(sizeof(orc_syllable2) / sizeof(char*))], name_len);
    my_strcat(name, orc_syllable3[randint0(sizeof(orc_syllable3) / sizeof(char*))], name_len);
    break;
    /* Create an empty name */
    default:
    name[0] = '\0';
    break;
    }
}


/*
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit_borg(struct player* p)
{
    int i;
    const struct start_item* si;
    struct object* obj, * known_obj;

    /* Currently carrying nothing */
    p->upkeep->total_weight = 0;

    /* Give the player obvious object knowledge */
    p->obj_k->dd = 1;
    p->obj_k->ds = 1;
    p->obj_k->ac = 1;
    for (i = 1; i < OF_MAX; i++) {
        struct obj_property* prop = lookup_obj_property(OBJ_PROPERTY_FLAG, i);
        if (prop->subtype == OFT_LIGHT) of_on(p->obj_k->flags, i);
        if (prop->subtype == OFT_DIG) of_on(p->obj_k->flags, i);
        if (prop->subtype == OFT_THROW) of_on(p->obj_k->flags, i);
    }

    /* Give the player starting equipment */
    for (si = p->class->start_items; si; si = si->next) {
        int num = rand_range(si->min, si->max);
        struct object_kind* kind = lookup_kind(si->tval, si->sval);
        assert(kind);

        /* Without start_kit, only start with 1 food and 1 light */
        if (!OPT(p, birth_start_kit)) {
            if (!tval_is_food_k(kind) && !tval_is_light_k(kind))
                continue;

            num = 1;
        }

        /* Exclude if configured to do so based on birth options. */
        if (si->eopts) {
            bool included = true;
            int eind = 0;

            while (si->eopts[eind] && included) {
                if (si->eopts[eind] > 0) {
                    if (p->opts.opt[si->eopts[eind]]) {
                        included = false;
                    }
                }
                else {
                    if (!p->opts.opt[-si->eopts[eind]]) {
                        included = false;
                    }
                }
                ++eind;
            }
            if (!included) continue;
        }

        /* Prepare a new item */
        obj = object_new();
        object_prep(obj, kind, 0, MINIMISE);
        obj->number = num;
        obj->origin = ORIGIN_BIRTH;

        known_obj = object_new();
        obj->known = known_obj;
        object_set_base_known(p, obj);
        object_flavor_aware(p, obj);
        obj->known->pval = obj->pval;
        obj->known->effect = obj->effect;
        obj->known->notice |= OBJ_NOTICE_ASSESSED;

        /* Deduct the cost of the item from starting cash */
        p->au -= object_value_real(obj, obj->number);

        /* Carry the item */
        inven_carry(p, obj, true, false);
        kind->everseen = true;
    }

    /* Sanity check */
    if (p->au < 0)
        p->au = 0;

    /* Now try wielding everything */
    wield_all(p);

    /* Update knowledge */
    update_player_object_knowledge(p);
}

static void borg_roll_hp(void)
{
    int i, j, min_value, max_value;

    /* Minimum hitpoints at highest level */
    min_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 3) / 8;
    min_value += PY_MAX_LEVEL;

    /* Maximum hitpoints at highest level */
    max_value = (PY_MAX_LEVEL * (player->hitdie - 1) * 5) / 8;
    max_value += PY_MAX_LEVEL;

    /* Roll out the hitpoints */
    while (true) {
        /* Roll the hitpoint values */
        for (i = 1; i < PY_MAX_LEVEL; i++) {
            j = randint1(player->hitdie);
            player->player_hp[i] = player->player_hp[i - 1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (player->player_hp[PY_MAX_LEVEL - 1] < min_value) continue;
        if (player->player_hp[PY_MAX_LEVEL - 1] > max_value) continue;

        /* Acceptable */
        break;
    }
}

/* Allow the borg to play continously.  Reset all values, */
void resurrect_borg(void)
{
    char buf[80];
    int i;
    struct player* p = player;

    /* save the existing dungeon.  It is cleared later but needs to */
    /* be blank when  creating the new player */
    struct chunk* sv_cave = cave;
    cave = NULL;

    /* Cheat death */
    player->is_dead = false;
    borg_skill[BI_MAXDEPTH] = 0;
    borg_skill[BI_MAXCLEVEL] = 1;

    /* Flush message buffer */
    borg_parse(NULL);
    borg_clear_reactions();

    /* flush the commands */
    borg_flush();

    /* remove the spell counters */
    for (i = 0; i < player->class->magic.total_spells; i++)
    {
        /* get the magics */
        borg_magic* as = &borg_magics[i];
        /* reset the counter */
        as->times = 0;
    }

    /*** Wipe the player ***/
    player_init(player);

    borg_skill[BI_ISCUT] = borg_skill[BI_ISSTUN] = borg_skill[BI_ISHEAVYSTUN] = borg_skill[BI_ISIMAGE] = borg_skill[BI_ISSTUDY] = false;

    /* reset our panel clock */
    time_this_panel = 1;

    /* reset our vault/unique check */
    vault_on_level = false;
    unique_on_level = 0;
    scaryguy_on_level = false;

    /* reset our breeder flag */
    breeder_level = false;

    /* Assume not leaving the level */
    goal_leaving = false;

    /* Assume not fleeing the level */
    goal_fleeing = false;

    /* Assume not fleeing the level */
    goal_fleeing_to_town = false;

    /* Assume not ignoring monsters */
    goal_ignoring = false;

    flavor_init();

    /** Roll up a new character **/
    struct player_race* p_race = NULL;
    struct player_class* p_class = NULL;
    if (borg_cfg[BORG_RESPAWN_RACE] != -1)
        p_race = player_id2race(borg_cfg[BORG_RESPAWN_RACE]);
    else
        p_race = player_id2race(randint0(MAX_RACES));
    if (borg_cfg[BORG_RESPAWN_CLASS] != -1)
        p_class = player_id2class(borg_cfg[BORG_RESPAWN_CLASS]);
    else
        p_class = player_id2class(randint0(MAX_CLASSES));
    player_generate(player, p_race, p_class, false);

    /* The dungeon is not ready nor is the player */
    character_dungeon = false;
    character_generated = false;

    /* Start in town */
    player->depth = 0;

    /* Hack -- seed for flavors */
    seed_flavor = randint0(0x10000000);

    /* Embody */
    memcpy(&p->body, &bodies[p->race->body], sizeof(p->body));
    my_strcpy(buf, bodies[p->race->body].name, sizeof(buf));
    p->body.name = string_make(buf);
    p->body.slots = mem_zalloc(p->body.count * sizeof(struct equip_slot));
    for (i = 0; i < p->body.count; i++) {
        p->body.slots[i].type = bodies[p->race->body].slots[i].type;
        my_strcpy(buf, bodies[p->race->body].slots[i].name, sizeof(buf));
        p->body.slots[i].name = string_make(buf);
    }

    /* Get a random name */
    create_random_name(player->race->ridx, player->full_name, sizeof(player->full_name));

    /* Give the player some money */
    player->au = player->au_birth = z_info->start_gold;

    /* Hack - need some HP */
    borg_roll_hp();

    /* Hack - player knows all combat runes.  Maybe make them not runes? NRM */
    player->obj_k->to_a = 1;
    player->obj_k->to_h = 1;
    player->obj_k->to_d = 1;

    /* Player learns innate runes */
    player_learn_innate(player);

    /* Initialise the spells */
    player_spells_init(player);

    /* outfit the player */
    player_outfit_borg(player);

    /* generate town */
    player->upkeep->generate_level = true;
    player->upkeep->playing = true;

    struct command fake_cmd;
    /* fake up a command */
    my_strcpy(fake_cmd.arg[0].name, "choice", sizeof(fake_cmd.arg[0].name));
    fake_cmd.arg[0].data.choice = 1;
    do_cmd_reset_stats(&fake_cmd);

    /* Initialise the stores, dungeon */
    store_reset();
    chunk_list_max = 0;

    /* Restore the standard artifacts (randarts may have been loaded) */
    cleanup_parser(&randart_parser);
    deactivate_randart_file();
    run_parser(&artifact_parser);

    /* Now only randomize the artifacts if required */
    if (OPT(player, birth_randarts)) {
        seed_randart = randint0(0x10000000);
        do_randart(seed_randart, true);
        deactivate_randart_file();
    }

    /* Hack -- flush it */
    Term_fresh();

    /*** Hack -- Extract race ***/

    /* Insert the player Race--cheat */
    borg_race = player->race->ridx;

    /* Cheat the class */
    borg_class = player->class->cidx;

    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    borg_prepare_race_class_info();

    /* need to check all stats */
    for (int tmp_i = 0; tmp_i < STAT_MAX; tmp_i++)
        my_need_stat_check[tmp_i] = true;

    /* Allowable Cheat -- Obtain "recall" flag */
    goal_recalling = player->word_recall * 1000;

    /* Allowable Cheat -- Obtain "prot_from_evil" flag */
    borg_prot_from_evil = (player->timed[TMD_PROTEVIL] ? true : false);
    /* Allowable Cheat -- Obtain "speed" flag */
    borg_speed = (player->timed[TMD_FAST] ? true : false);
    /* Allowable Cheat -- Obtain "resist" flags */
    borg_skill[BI_TRACID] = (player->timed[TMD_OPP_ACID] ? true : false);
    borg_skill[BI_TRELEC] = (player->timed[TMD_OPP_ELEC] ? true : false);
    borg_skill[BI_TRFIRE] = (player->timed[TMD_OPP_FIRE] ? true : false);
    borg_skill[BI_TRCOLD] = (player->timed[TMD_OPP_COLD] ? true : false);
    borg_skill[BI_TRPOIS] = (player->timed[TMD_OPP_POIS] ? true : false);
    borg_bless = (player->timed[TMD_BLESSED] ? true : false);
    borg_shield = (player->timed[TMD_SHIELD] ? true : false);
    borg_hero = (player->timed[TMD_HERO] ? true : false);
    borg_fastcast = (player->timed[TMD_FASTCAST] ? true : false);
    borg_berserk = (player->timed[TMD_SHERO] ? true : false);
    if (player->timed[TMD_SINVIS]) borg_see_inv = 10000;

    /* Message */
    borg_note("# Respawning");
    borg_respawning = 5;

    /* fully healed and rested */
    player->chp = player->mhp;
    player->csp = player->msp;

    /* restore the cave */
    cave = sv_cave;

    /* the new player is now ready */
    character_generated = true;

    /* Mark savefile as borg cheater */
    if (!(player->noscore & NOSCORE_BORG)) player->noscore |= NOSCORE_BORG;

    /* Done.  Play on */
}
#endif /* bablos */

/*
 * Mega-Hack -- special "inkey_hack" hook.  XXX XXX XXX
 *
 * A special function hook (see "util.c") which allows the Borg to take
 * control of the "inkey()" function, and substitute in fake keypresses.
 */
extern struct keypress(*inkey_hack)(int flush_first);

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
static struct keypress borg_inkey_hack(int flush_first)
{
    keycode_t borg_ch;
    struct keypress key = { EVT_KBRD, 0, 0 };

    ui_event ch_evt;

    int y = 0;
    int x = ((Term->wid /* - (COL_MAP)*/ - 1) / (tile_width));

    uint8_t t_a;

    char buf[1024];

    bool borg_prompt;  /* ajg  For now we can just use this locally.
                           in the 283 borg he uses this to optimize knowing if
                           we are waiting at a prompt for info */
                           /* Locate the cursor */
    (void)Term_locate(&x, &y);

    /* Refresh the screen */
    Term_fresh();

    /* Deactivate */
    if (!borg_active)
    {
        /* Message */
        borg_note("# Removing keypress hook");

        /* Remove hook */
        inkey_hack = NULL;

        /* Flush keys */
        borg_flush();

        /* Flush */
        flush(0, 0, 0);

        /* Restore user key mode */
        if (key_mode == KEYMAP_MODE_ROGUE)
        {
            option_set("rogue_like_commands", true);
        }
        else if (key_mode == KEYMAP_MODE_ORIG)
        {
            option_set("rogue_like_commands", false);
        }

        /* Done */
        /* HACK need to flush the key buffer to change modes */
        key.type = EVT_KBRD;
        key.code = ESCAPE;
        return key;
    }


    /* Mega-Hack -- flush keys */
    if (flush_first)
    {
        /* Only flush if needed */
        if (borg_inkey(false) != 0)
        {
            /* Message */
            borg_note("# Flushing keypress buffer");

            /* Flush keys */
            borg_flush();

            /* Cycle a few times to catch up if needed */
            if (time_this_panel > 250)
            {
                borg_respawning = 3;
            }
        }
    }



    /* Assume no prompt/message is available */
    borg_prompt = false;


    /* Mega-Hack -- check for possible prompts/messages */
    /* If the first four characters on the message line all */
    /* have the same attribute (or are all spaces), and they */
    /* are not all spaces (ascii value 0x20)... */
    if ((0 == borg_what_text(0, 0, 4, &t_a, buf)) &&
        (t_a != COLOUR_DARK) &&
        (buf[0] != ' ' || buf[1] != ' ' || buf[2] != ' ' || buf[3] != ' '))
    {
        /* Assume a prompt/message is available */
        borg_prompt = true;
    }
    if (borg_prompt && streq(buf, "Type"))
    {
        borg_prompt = false;
    }

    /* Mega-Hack -- Catch "Die? [y/n]" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And the text acquired above is "Die?" */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 4) &&
        streq(buf, "Die?") &&
        borg_cheat_death)
    {
        /* Flush messages */
        borg_parse(NULL);

        /* flush the buffer */
        borg_flush();

        /* Take note */
        borg_note("# Cheating death...");

#ifndef BABLOS
        /* Dump the Character Map*/
        if (borg_skill[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL] ||
            strstr(player->died_from, "starvation")) borg_write_map(false);

        /* Log the death */
        borg_log_death();
        borg_log_death_data();

#if 0
        /* Note the score */
        borg_enter_score();
#endif

        resurrect_borg();
#endif /* BABLOS */

        key.code = 'n';
        return key;
    }

    /* with 292, there is a flush(0, 0, 0) introduced as it asks for confirmation.
     * This flush is messing up the borg.  This will allow the borg to
     * work around the flush
     * Attempt to catch "Attempt it anyway? [y/n]"
     */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 4) &&
        streq(buf, "Atte"))
    {
        /* Return the confirmation */
        borg_note("# Confirming use of Spell/Prayer.");
        key.code = 'y';
        return key;
    }

    /* with 292, there is a flush(0, 0, 0) introduced as it asks for confirmation.
     * This flush is messing up the borg.  This will allow the borg to
     * work around the flush
     * This is used only with emergency use of spells like Magic Missile
     * Attempt to catch "Direction (5 old target"
     */
    if (borg_prompt && !inkey_flag && borg_confirm_target &&
        (y == 0) && (x >= 4) &&
        streq(buf, "Dire"))
    {
        /* reset the flag */
        borg_confirm_target = false;
        /* Return queued target */
        key.code = borg_get_queued_direction();
        return key;
    }

    /* Wearing two rings.  Place this on the left hand */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 12) &&
        (0 == borg_what_text(0, y, 12, &t_a, buf)) &&
        (streq(buf, "(Equip: c-d,")))
    {
        /* Left hand */
        key.code = 'c';
        return key;
    }

    /* ***MEGA-HACK***  */
    /* This will be hit if the borg uses an unidentified effect that has */
    /* EF_SELECT/multiple effects. Always pick "one of the following at random" */
    /* when the item is used post ID, an effect will be selected */
    if (borg_prompt && !inkey_flag && !borg_inkey(false) &&
        (y == 1) &&
        (0 == borg_what_text(0, 0, 13, &t_a, buf)) &&
        streq(buf, "Which effect?"))
    {
        /* the first selection (a) is random */
        key.code = 'a';
        return key;
    }

    /* prompt for stepping in lava.  This should be avoided but */
    /* if the borg is stuck, give him a pass */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 12) &&
        (0 == borg_what_text(0, y, 13, &t_a, buf)) &&
        (streq(buf, "The lava will") || 
         streq(buf, "Lava blocks y")))
    {
        /* yes step in */
        key.code = 'y';
        return key;
    }

    /* Mega-Hack -- Handle death */
    if (player->is_dead)
    {
#ifndef BABLOS
        /* Print the map */
        if (borg_skill[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL] ||
            strstr(player->died_from, "starvation"))  borg_write_map(false);

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
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 7) &&
        (0 == borg_what_text(x - 7, y, 7, &t_a, buf)) &&
        (streq(buf, " -more-")))
    {
        /* Get the message */
        if (0 == borg_what_text(0, 0, x - 7, &t_a, buf))
        {
            /* Parse it */
            borg_parse(buf);
        }
        /* Clear the message */
        key.code = ' ';
        return key;
    }

    /* in the odd case where a we get here before the message */
    /* about cheating death comes up.  */
    if (!character_dungeon)
    {
        /* do nothing */
        key.code = ' ';
        return key;
    }

    /* Mega-Hack -- catch normal messages */
    /* If there is text on the first line... */
    /* And the game wants a command */
    if (borg_prompt && inkey_flag)
    {
        /* Get the message(s) */
        if (0 == borg_what_text(0, 0, ((Term->wid - 1) / (tile_width)), &t_a, buf))
        {
            int k = strlen(buf);

            /* Strip trailing spaces */
            while ((k > 0) && (buf[k - 1] == ' ')) k--;

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
    if (borg_skill[BI_CDEPTH] >= 1) borg_in_shop = false;


    if (!borg_in_shop && (ch_evt.type & EVT_KBRD) && ch_evt.key.code > 0 &&
        ch_evt.key.code != 10)
    {
        /* Oops */
        borg_note(format("# User key press <%d><%c>", ch_evt.key.code, ch_evt.key.code));
        borg_note(format("# Key type was <%d><%c>", ch_evt.type, ch_evt.type));
        borg_oops("user abort");

        /* Hack -- Escape */
        key.code = ESCAPE;
        return key;
    }

    /* for some reason, selling and buying in the store sets the event handler to Select. */
    if (ch_evt.type & EVT_SELECT) ch_evt.type = EVT_KBRD;
    if (ch_evt.type & EVT_MOVE) ch_evt.type = EVT_KBRD;

    /* Save the system random info */
    borg_rand_quick = Rand_quick;
    borg_rand_value = Rand_value;

    /* Use the local random info */
    Rand_quick = true;
    Rand_value = borg_rand_local;

    /* Don't interrupt our own resting or a repeating command */
    if (player->upkeep->resting || cmd_get_nrepeats() > 0)
    {
        key.type = EVT_NONE;
        return key;
    }

    /* Think */
    while (!borg_think()) /* loop */;

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

/*
 * Output a long int in binary format.
 */
static void borg_prt_binary(uint32_t flags, int row, int col)
{
    int        	i;
    uint32_t        bitmask;

    /* Scan the flags */
    for (i = bitmask = 1; i <= 32; i++, bitmask *= 2)
    {
        /* Dump set bits */
        if (flags & bitmask)
        {
            Term_putch(col++, row, COLOUR_BLUE, '*');
        }

        /* Dump unset bits */
        else
        {
            Term_putch(col++, row, COLOUR_WHITE, '-');
        }
    }
}

/* this will display the values which the borg believes an
 * item has.  Select the item by inven # prior to hitting
 * the ^zo.
 */
static void borg_display_item(struct object* item2, int n)
{
    int j = 0;

    bitflag f[OF_SIZE];

    borg_item* item;

    item = &borg_items[n];

    /* Extract the flags */
    object_flags(item2, f);

    /* Clear screen */
    Term_clear();

    /* Describe fully */
    prt(item->desc, 2, j);

    prt(format("kind = %-5d  level = %-4d  tval = %-5d  sval = %-5d",
        item->kind, item->level,
        item->tval, item->sval), 4, j);

    prt(format("number = %-3d  wgt = %-6d  ac = %-5d    damage = %dd%d",
        item->iqty, item->weight,
        item->ac, item->dd, item->ds), 5, j);

    prt(format("pval = %-5d  toac = %-5d  tohit = %-4d  todam = %-4d",
        item->pval, item->to_a, item->to_h, item->to_d), 6, j);

    prt(format("name1 = %-4d  name2 = %-4d  value = %ld   cursed = %d   can uncurse = %d",
        item->art_idx, item->ego_idx, (long)item->value, item->cursed, item->uncursable), 7, j);

    prt(format("ident = %d      timeout = %-d",
        item->ident, item->timeout), 8, j);

    /* maybe print the inscription */
    prt(format("Inscription: %s, chance: %d", borg_get_note(item), borg_skill[BI_DEV] - item->level), 9, j);

    prt("+------------FLAGS1------------+", 10, j);
    prt("AFFECT..........SLAY.......BRAND", 11, j);
    prt("                ae      xxxpaefc", 12, j);
    prt("siwdcc  ssidsasmnvudotgddduoclio", 13, j);
    prt("tnieoh  trnipthgiinmrrnrrmniierl", 14, j);
    prt("rtsxna..lcfgdkttmldncltggndsdced", 15, j);
    if (item->ident) borg_prt_binary(f[0], 16, j);

    prt("+------------FLAGS2------------+", 17, j);
    prt("SUST........IMM.RESIST.........", 18, j);
    prt("            afecaefcpfldbc s n  ", 19, j);
    prt("siwdcc      cilocliooeialoshnecd", 20, j);
    prt("tnieoh      irelierliatrnnnrethi", 21, j);
    prt("rtsxna......decddcedsrekdfddxhss", 22, j);
    if (item->ident) borg_prt_binary(f[1], 23, j);

    prt("+------------FLAGS3------------+", 10, j + 32);
    prt("s   ts h     tadiiii   aiehs  hp", 11, j + 32);
    prt("lf  eefo     egrgggg  bcnaih  vr", 12, j + 32);
    prt("we  lerln   ilgannnn  ltssdo  ym", 13, j + 32);
    prt("da reiedo   merirrrr  eityew ccc", 14, j + 32);
    prt("itlepnelf   ppanaefc  svaktm uuu", 15, j + 32);
    prt("ghigavaiu   aoveclio  saanyo rrr", 16, j + 32);
    prt("seteticfe   craxierl  etropd sss", 17, j + 32);
    prt("trenhstel   tttpdced  detwes eee", 18, j + 32);
    if (item->ident) borg_prt_binary(f[2], 19, j + 32);
}

static int
borg_getval(char** string, const char* val)
{
    char    string2[4];
    int     retval;

    if (!prefix(*string, val))
    {
        return -1000;
    }
    (*string) += strlen(val);
    memmove(string2, *string, 3);
    string2[3] = 0;
    sscanf(string2, "%d", &retval);
    *string += 3;
    return retval;
}

#if false
static bool borg_load_formula(char* string)
{
    int formula_num;
    int iformula = 0;
    int x = 0;
    int value = 0;
    char string2[4];

    memmove(string2, string, 3);
    string2[3] = 0;
    sscanf(string2, "%d", &formula_num);
    string += 4;
    if (formula[formula_num])
    {
        borg_note(format("formula defined twice %03d", formula_num));
        return false;
    }
    formula[formula_num] = mem_zalloc(MAX_FORMULA_ELEMENTS * sizeof(int));

    while (string && *string)
    {
        switch (*string)
        {
        case ' ':
        string++;
        continue;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        sscanf(string, "%d", &value);
        if (iformula + 2 > MAX_FORMULA_ELEMENTS)
        {
            borg_note(format("too many elements in formula %03d", formula_num));
            formula[formula_num][0] = BFO_NUMBER;
            formula[formula_num][1] = 0;
            return false;
        }
        formula[formula_num][iformula++] = BFO_NUMBER;
        formula[formula_num][iformula++] = value;
        break;
        case '_':
        if (iformula + 2 > MAX_FORMULA_ELEMENTS)
        {
            borg_note(format("too many elements in formula %03d", formula_num));
            formula[formula_num][0] = BFO_NUMBER;
            formula[formula_num][1] = 0;
            return false;
        }
        formula[formula_num][iformula++] = BFO_VARIABLE;
        if (-1000 != (value = borg_getval(&string, "_ITEM")))
        {
            formula[formula_num][iformula++] = value;
            break;
        }
        if (-1000 != (value = borg_getval(&string, "_WITEM")))
        {
            formula[formula_num][iformula++] = value + z_info->k_max;
            break;
        }
        if (-1000 != (value = borg_getval(&string, "_ARTIFACT")))
        {
            formula[formula_num][iformula++] = value +
                z_info->k_max +
                z_info->k_max;
            break;
        }

        for (x = 0; x < BI_MAX; x++)
        {
            if (prefix(string, prefix_pref[x]))
            {
                value = z_info->k_max + z_info->k_max + z_info->a_max + x;
                formula[formula_num][iformula++] = value;
                break;
            }
        }
        if (x == BI_MAX)
        {
            formula[formula_num][0] = BFO_NUMBER;
            formula[formula_num][1] = 0;
            borg_note(format("bad item in formula%03d %s", formula_num, string));
            return false;
        }

        break;
        default:
        if (iformula + 1 > MAX_FORMULA_ELEMENTS)
        {
            borg_note(format("too many elements in formula %03d", formula_num));
            formula[formula_num][0] = BFO_NUMBER;
            formula[formula_num][1] = 0;
            return false;
        }
        if (*string == '>')
        {
            if (*(string + 1) == '=')
            {
                formula[formula_num][iformula++] = BFO_GTE;
                break;
            }
            formula[formula_num][iformula++] = BFO_GT;
            break;
        }
        if (*string == '<')
        {
            if (*(string + 1) == '=')
            {
                formula[formula_num][iformula++] = BFO_LTE;
                break;
            }
            formula[formula_num][iformula++] = BFO_LT;
            break;
        }
        if (*string == '!')
        {
            if (*(string + 1) == '=')
                formula[formula_num][iformula++] = BFO_NEQ;
            else
                formula[formula_num][iformula++] = BFO_NOT;
            break;
        }
        if (*string == '=')
        {
            formula[formula_num][iformula++] = BFO_EQ;
            break;
        }
        if (*string == '&')
        {
            formula[formula_num][iformula++] = BFO_AND;
            break;
        }
        if (*string == '-')
        {
            /* - followed by space is a minus.  */
            if (*(string + 1) == ' ')
            {
                formula[formula_num][iformula++] = BFO_MINUS;
                break;
            }
            if (iformula + 1 > MAX_FORMULA_ELEMENTS)
            {
                borg_note(format("too many elements in formula %03d", formula_num));
                formula[formula_num][0] = BFO_NUMBER;
                formula[formula_num][1] = 0;
                return false;
            }

            /* - followed by anything else is a negative number */
            sscanf(string, "%d", &value);
            formula[formula_num][iformula++] = BFO_NUMBER;
            formula[formula_num][iformula++] = value;
            break;
        }
        if (*string == '+')
        {
            formula[formula_num][iformula++] = BFO_PLUS;
            break;
        }
        if (*string == '/')
        {
            formula[formula_num][iformula++] = BFO_DIVIDE;
            break;
        }
        if (*string == '*')
        {
            formula[formula_num][iformula++] = BFO_MULT;
            break;
        }
        if (*string == '|')
        {
            formula[formula_num][iformula++] = BFO_OR;
            break;
        }
        borg_note(format("bad item in formula %03d %s", formula_num, string));
        formula[formula_num][0] = BFO_NUMBER;
        formula[formula_num][1] = 0;
        return false;
        }
        string = strchr(string, ' ');
    }
    if (!borg_check_formula(formula[formula_num]))
    {
        borg_note(format("bad formula %03d", formula_num));
        formula[formula_num][0] = BFO_NUMBER;
        formula[formula_num][1] = 0;
        return false;
    }
    return true;
}
#endif

static bool add_power_item(int class_num,
    int depth_num,
    int cnd_num,
    int range_to,
    int range_from,
    bool each,
    int item_num,
    int power)
{
    if ((class_num > MAX_CLASSES &&
        class_num != 999) ||
        depth_num >= z_info->max_depth ||
        item_num >= (z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX) ||
        range_to < range_from)
    {
        borg_note("Malformed item power in borg.txt: values out of range");
        return false;
    }
    /* The class 999 is for all classes */
    if (class_num == 999)
    {
        for (class_num = 0; class_num < MAX_CLASSES; class_num++)
        {
            borg_power_item[class_num][n_pwr[class_num]].depth = depth_num;
            borg_power_item[class_num][n_pwr[class_num]].cnd = cnd_num;
            borg_power_item[class_num][n_pwr[class_num]].item = item_num;
            borg_power_item[class_num][n_pwr[class_num]].power = power;
            borg_power_item[class_num][n_pwr[class_num]].from = range_from;
            borg_power_item[class_num][n_pwr[class_num]].to = range_to;
            borg_power_item[class_num][n_pwr[class_num]].each = each;
            n_pwr[class_num]++;
        }
    }
    else
    {
        borg_power_item[class_num][n_pwr[class_num]].depth = depth_num;
        borg_power_item[class_num][n_pwr[class_num]].cnd = cnd_num;
        borg_power_item[class_num][n_pwr[class_num]].item = item_num;
        borg_power_item[class_num][n_pwr[class_num]].power = power;
        borg_power_item[class_num][n_pwr[class_num]].from = range_from;
        borg_power_item[class_num][n_pwr[class_num]].to = range_to;
        borg_power_item[class_num][n_pwr[class_num]].each = each;
        n_pwr[class_num]++;
    }
    return true;
}

static bool borg_load_power(char* string)
{
    int class_num = -1;
    int depth_num = -1;
    int cnd_num = -1;
    int range_to = -1;
    int range_from = -1;
    bool each = false;
    int item_num = -1;
    int power = -1;
    int x;


    if (-1000 == (class_num = borg_getval(&string, "_CLASS")))
    {
        borg_note("Malformed item power in borg.txt: missing _CLASS");
        return false;
    }
    if (-1000 == (depth_num = borg_getval(&string, "_DEPTH")))
    {
        borg_note("Malformed item power in borg.txt: missing _DEPTH");
        return false;
    }
    if (-1000 == (cnd_num = borg_getval(&string, "_CND")))
    {
        /* condition is optional */
        cnd_num = -1;
    }
    if (-1000 == (range_from = borg_getval(&string, "_RANGE")))
    {
        borg_note("Malformed item power in borg.txt: missing _RANGE");
        return false;
    }
    if (-1000 == (range_to = borg_getval(&string, "TO")))
    {
        borg_note("Malformed item power in borg.txt: messed up _RANGE");
        return false;
    }

    if (-1000 != (item_num = borg_getval(&string, "_FORMULA")))
    {
        if (range_to != 999 || range_from != 0)
        {
            borg_note("Malformed item power in borg.txt: range must be 0-999 formulas");
            return false;
        }
        return add_power_item(class_num,
            depth_num,
            cnd_num,
            range_to,
            range_from,
            each,
            -1,
            item_num);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ITEM")))
    {
        string++;
        sscanf(string, "%d", &power);
        if (strstr(string, "EACH"))
            each = true;

        return add_power_item(class_num,
            depth_num,
            cnd_num,
            range_to,
            range_from,
            each,
            item_num,
            power);
    }
    if (-1000 != (item_num = borg_getval(&string, "_WITEM")))
    {
        string++;
        sscanf(string, "%d", &power);
        if (strstr(string, "EACH"))
            each = true;

        return add_power_item(class_num,
            depth_num,
            cnd_num,
            range_to,
            range_from,
            each,
            z_info->k_max + item_num,
            power);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ARTIFACT")))
    {
        string++;
        sscanf(string, "%d", &power);
        if (strstr(string, "EACH"))
            each = true;
        item_num += z_info->k_max + z_info->k_max;
        return add_power_item(class_num,
            depth_num,
            cnd_num,
            range_to,
            range_from,
            each,
            item_num,
            power);
    }

    for (x = 0; x < BI_MAX; x++)
    {

        if (prefix(string, prefix_pref[x]))
        {
            string += strlen(prefix_pref[x]);
            item_num = z_info->k_max + z_info->k_max + z_info->a_max + x;
            string++;
            sscanf(string, "%d", &power);
            if (strstr(string, "EACH"))
                each = true;
            return add_power_item(class_num,
                depth_num,
                cnd_num,
                range_to,
                range_from,
                each,
                item_num,
                power);
        }
    }
    borg_note("Malformed item power in borg.txt");
    return false;
}
static bool add_required_item(int class_num, int depth_num, int item_num, int number_items)
{
    if ((class_num > MAX_CLASSES &&
        class_num != 999) ||
        depth_num >= z_info->max_depth ||
        item_num >= (z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX))
    {
        borg_note("Malformed item requirment in borg.txt: value out of range");
        return false;
    }
    /* The class 999 is for all classes */
    if (class_num == 999)
    {
        for (class_num = 0; class_num < MAX_CLASSES; class_num++)
        {
            borg_required_item[class_num][n_req[class_num]].depth = depth_num;
            borg_required_item[class_num][n_req[class_num]].item = item_num;
            borg_required_item[class_num][n_req[class_num]].number = number_items;
            n_req[class_num]++;
        }
    }
    else
    {
        borg_required_item[class_num][n_req[class_num]].depth = depth_num;
        borg_required_item[class_num][n_req[class_num]].item = item_num;
        borg_required_item[class_num][n_req[class_num]].number = number_items;
        n_req[class_num]++;
    }
    return true;
}

static bool borg_load_requirement(char* string)
{
    int class_num = -1;
    int depth_num = -1;
    int item_num = -1;
    int number_items = -1;
    int x = -1;

    if (-1000 == (class_num = borg_getval(&string, "_CLASS")))
    {
        borg_note("Malformed item requirment in borg.txt");
        return false;
    }
    if (-1000 == (depth_num = borg_getval(&string, "_DEPTH")))
    {
        borg_note("Malformed item requirment in borg.txt");
        return false;
    }
    if (-1000 != (item_num = borg_getval(&string, "_FORMULA")))
    {
        return add_required_item(class_num, depth_num, -1, item_num);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ITEM")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num, item_num, number_items);
    }
    if (-1000 != (item_num = borg_getval(&string, "_WITEM")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num, z_info->k_max + item_num, number_items);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ARTIFACT")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num, z_info->k_max + z_info->k_max + item_num, number_items);
    }

    for (x = 0; x < BI_MAX; x++)
    {

        if (prefix(string, prefix_pref[x]))
        {
            string += strlen(prefix_pref[x]);
            item_num = z_info->k_max + z_info->k_max + z_info->a_max + x;
            string++;
            sscanf(string, "%d", &number_items);
            return add_required_item(class_num, depth_num, item_num, number_items);
        }
    }
    borg_note("Malformed item requirment in borg.txt");
    return false;
}

/* just used to do a quick sort on the required items array */
static int borg_item_cmp(const void* item1, const void* item2)
{
    if (((req_item*)item1)->depth != ((req_item*)item2)->depth)
        return ((req_item*)item1)->depth - ((req_item*)item2)->depth;
    if (((req_item*)item1)->item != ((req_item*)item2)->item)
        return ((req_item*)item1)->item - ((req_item*)item2)->item;
    return ((req_item*)item1)->number - ((req_item*)item2)->number;
}

/*
* read a line from the config file and parse the setting value, if there is one.
*/
static bool borg_proc_setting(int setting, const char * setting_string, char type, const char* line)
{
    bool negative = false;

    /* skip leading space */
    while (*line == ' ') line++;
    if (!prefix_i(line, setting_string))
        return false;

    line += strlen(setting_string);

    /* accept either */
    /* value true or */
    /* value=true or */
    /* value=1 or */
    /* value y */
    while (*line)
    {
        char ch = *line;
        if (ch == ' ' || ch == '=')
        {
            line++;
            continue;
        }

        if (type == 'b')
        {
            if (ch == 'T' ||
                ch == 't' ||
                ch == '1' ||
                ch == 'Y' ||
                ch == 'y')
                borg_cfg[setting] = true;
            else
                borg_cfg[setting] = false;

            break;
        }
        if (type == 'i')
        {
            if (ch == '-')
            {
                negative = true;
                line++;
                continue;
            }
            if (isdigit(ch))
            {
                borg_cfg[setting] = atoi(line);
                break;
            }
        }
    }

    if (negative)
        borg_cfg[setting] *= -1;

    return true;
}

/*
 * Initialize borg.txt
 */
static void init_borg_txt_file(void)
{
    struct borg_setting
    {
        const char* setting_string;
        const char  setting_type;  /* b (bool) or i (int) */
        int         default_value;
    };

    /*
     * Borg settings information, ScreenSaver or continual play mode;
     */
    struct borg_setting borg_settings[] =
    {
        { "borg_verbose", 'b', false},
        { "borg_munchkin_start", 'b', false},
        { "borg_munchkin_level", 'i', 12},
        { "borg_munchkin_depth", 'i', 16},
        { "borg_worships_damage", 'b', false},
        { "borg_worships_speed", 'b', false},
        { "borg_worships_hp", 'b', false},
        { "borg_worships_mana", 'b', false},
        { "borg_worships_ac", 'b', false},
        { "borg_worships_gold", 'b', false},
        { "borg_plays_risky", 'b', false},
        { "borg_kills_uniques", 'b', false},
        { "borg_uses_swaps", 'b', true},
        { "borg_uses_dynamic_calcs", 'b', false},
        { "borg_slow_optimizehome", 'b', false},
        { "borg_stop_dlevel", 'i', 128},
        { "borg_stop_clevel", 'i', 51},
        { "borg_no_deeper", 'i', 127},
        { "borg_stop_king", 'b', true},
        { "borg_respawn_winners", 'b', false},
        { "borg_respawn_class", 'i', -1},
        { "borg_respawn_race", 'i', -1},
        { "borg_chest_fail_tolerance", 'i', 7},
        { "borg_delay_factor", 'i', 0},
        { "borg_money_scum_amount", 'i', 0},
        { "borg_self_scum", 'b', true},
        { "borg_lunal_mode", 'b', false},
        { "borg_self_lunal", 'b', false},
        { "borg_enchant_limit", 'i', 12},
        { "borg_dump_level", 'i', 1},
        { "borg_save_death", 'i', 1},
        { 0, 0, 0 }
    };

    ang_file* fp;

    char buf[1024];
    int i;

    /* Array of borg variables is stored as */
    /* 0 to k_max = items in inventory */
    /* k_max to end of array = Other skills/possessions */
    size_obj = z_info->k_max + BI_MAX;

    /* note: C_MAKE automaticly 0 inits things */

    for (i = 0; i < MAX_CLASSES; i++)
    {
        borg_required_item[i] = mem_zalloc(400 * sizeof(req_item)); /* externalize the 400 later */
        n_req[i] = 0;
        borg_power_item[i] = mem_zalloc(400 * sizeof(power_item)); /* externalize the 400 later */
        n_pwr[i] = 0;
    }
    for (i = 0; i < 999; i++)
    {
        formula[i] = 0;
    }
    borg_has = mem_zalloc(size_obj * sizeof(int));
    borg_skill = borg_has + z_info->k_max;

    /* a couple of spot checks on settings definitiosn */
    if (!streq(borg_settings[BORG_MUNCHKIN_LEVEL].setting_string, "borg_munchkin_level") ||
        !streq(borg_settings[BORG_RESPAWN_RACE].setting_string, "borg_respawn_race"))
    {
        msg("borg settings structures not correct.  aborting. ");
        borg_init_failure = true;
    }

    path_build(buf, 1024, ANGBAND_DIR_USER, "borg.txt");

    /* Open the file */
    fp = file_open(buf, MODE_READ, -1);

    /* No file, use defaults*/
    if (!fp)
    {
        /* Complain */
        msg("*****WARNING***** You do not have a proper BORG.TXT file!");
        msg("writing borg.txt to the \\user\\ subdirectory with default values");
        msg("Which is: %s", buf);
        event_signal(EVENT_MESSAGE_FLUSH);

        fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);
        if (!fp)
        {
            msg("*****WARNING***** unable to write default BORG.TXT file!");
            return;
        }
        file_putf(fp, "# BORG.txt default settings \n");
        file_putf(fp, "# A more descriptive version of this file is delivered. \n");
        file_putf(fp, "# Check your original install for borg.txt and \n");
        file_putf(fp, "# replace this one with the delivered version. \n\n");

        for (i = 0; i < BORG_MAX_SETTINGS; i++)
        {
            if (borg_settings[i].setting_type == 'b')
                file_putf(fp, "%s = %s\n", borg_settings[i].setting_string,
                    borg_settings[i].default_value ? "TRUE" : "FALSE");
            if (borg_settings[i].setting_type == 'i')
                file_putf(fp, "%s = %d\n", borg_settings[i].setting_string, 
                    borg_settings[i].default_value);
        }
        file_close(fp);
        fp = file_open(buf, MODE_READ, -1);
    }

    /* allocate the config data */
    borg_cfg = mem_alloc(sizeof(int) * BORG_MAX_SETTINGS);

    /* start the config with the default values */
    for (i = 0; i < BORG_MAX_SETTINGS; i++)
        borg_cfg[i] = borg_settings[i].default_value;

    /* Parse the file */
    while (file_getl(fp, buf, sizeof(buf) - 1))
    {
        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Chop the buffer */
        buf[sizeof(buf) - 1] = '\0';

        for (i = 0; i < BORG_MAX_SETTINGS; i++)
        {
            if (borg_proc_setting(i, borg_settings[i].setting_string,
                borg_settings[i].setting_type, buf))
                break;
        }

        /* other settings */
        if (prefix_i(buf, "REQ"))
        {
            if (!borg_load_requirement(buf + strlen("REQ")))
                borg_note(buf);
            continue;
        }
        if (prefix_i(buf, "FORMULA"))
        {
#if false
// For now ignore the dynamic formulas in borg.txt ... they are waaaay out of date !FIX !TODO !AJG
            if (!borg_load_formula(buf + strlen("FORMULA")))
                borg_note(buf);
#endif 
            continue;
        }
        if (prefix_i(buf, "CND"))
        {
#if false
            if (!borg_load_formula(buf + strlen("CND")))
                borg_note(buf);
#endif 
            continue;
        }
        if (prefix_i(buf, "POWER"))
        {
            if (!borg_load_power(buf + strlen("POWER")))
                borg_note(buf);

            continue;
        }
    }

    /* Close it */
    file_close(fp);

    if (borg_cfg[BORG_USES_DYNAMIC_CALCS])
    {
        msg("Dynamic calcs (borg_uses_dynamic-calcs) is configured on but currently ignored.");
        borg_cfg[BORG_USES_DYNAMIC_CALCS] = false;
    }

    /* lunal mode is a default rather than a setting */
    borg_lunal_mode = borg_cfg[BORG_LUNAL_MODE];

    /* a few sanity range checks */
    if (borg_cfg[BORG_MUNCHKIN_LEVEL] <= 1) 
        borg_cfg[BORG_MUNCHKIN_LEVEL] = 1;
    if (borg_cfg[BORG_MUNCHKIN_LEVEL] >= 50) 
        borg_cfg[BORG_MUNCHKIN_LEVEL] = 50;

    if (borg_cfg[BORG_MUNCHKIN_DEPTH] <= 1)
        borg_cfg[BORG_MUNCHKIN_DEPTH] = 8;
    if (borg_cfg[BORG_MUNCHKIN_DEPTH] >= 100)
        borg_cfg[BORG_MUNCHKIN_DEPTH] = 100;

    if (borg_cfg[BORG_ENCHANT_LIMIT] <= 8)
        borg_cfg[BORG_ENCHANT_LIMIT] = 8;
    if (borg_cfg[BORG_ENCHANT_LIMIT] >= 15)
        borg_cfg[BORG_ENCHANT_LIMIT] = 15;

    if (borg_cfg[BORG_RESPAWN_RACE] >= MAX_RACES ||
        borg_cfg[BORG_RESPAWN_RACE] < -1)
        borg_cfg[BORG_RESPAWN_RACE] = 0;

    if (borg_cfg[BORG_RESPAWN_CLASS] >= MAX_CLASSES ||
        borg_cfg[BORG_RESPAWN_CLASS] < -1)
        borg_cfg[BORG_RESPAWN_CLASS] = 0;


    for (i = 0; i < MAX_CLASSES; i++)
        qsort(borg_required_item[i], n_req[i], sizeof(req_item), borg_item_cmp);

    /* make sure it continues to run if reset */
    if (borg_cfg[BORG_RESPAWN_WINNERS]) 
        borg_cfg[BORG_STOP_KING] = false;

    /* Success */
    return;
}

/*
 * Release resources allocated by init_borg_txt_file().
 */
static void clean_borg_txt_file(void)
{
    int i;

    mem_free(borg_cfg);
    borg_cfg = NULL;
    mem_free(borg_has);
    borg_has = NULL;
    borg_skill = NULL;
    for (i = 0; i < MAX_CLASSES; ++i) {
        mem_free(borg_power_item[i]);
        borg_power_item[i] = NULL;
        n_pwr[i] = 0;
        mem_free(borg_required_item[i]);
        borg_required_item[i] = NULL;
        n_req[i] = 0;
    }
}

/* all parts equal or same nullness */
static bool borg_read_message_equal(struct borg_read_message* msg1, struct borg_read_message* msg2)
{
    if (((msg1->message_p1 && msg2->message_p1 && streq(msg1->message_p1, msg2->message_p1)) ||
        (!msg1->message_p1 && !msg2->message_p1)) &&
        ((msg1->message_p2 && msg2->message_p2 && streq(msg1->message_p2, msg2->message_p2)) ||
            (!msg1->message_p2 && !msg2->message_p2)) &&
        ((msg1->message_p3 && msg2->message_p3 && streq(msg1->message_p3, msg2->message_p3)) ||
            (!msg1->message_p3 && !msg2->message_p3)))
        return true;
    return false;
}

static void insert_msg(struct borg_read_messages* msgs, struct borg_read_message* msg, int spell_number)
{
    int i;
    bool found_dup = false;

    /* this way we don't have to pre-create the array */
    if (msgs->messages == NULL)
    {
        msgs->allocated = 10;
        msgs->messages = mem_alloc(sizeof(struct borg_read_message) * msgs->allocated);
        msgs->index = mem_alloc(sizeof(int) * msgs->allocated);
    }

    if (msg == NULL)
    {
        msgs->messages[msgs->count].message_p1 = NULL;
        msgs->messages[msgs->count].message_p2 = NULL;
        msgs->messages[msgs->count].message_p3 = NULL;
        msgs->count++;
        /* shrink array down, we are done*/
        msgs->messages = mem_realloc(msgs->messages, sizeof(struct borg_read_message) * msgs->count);
        msgs->index = mem_realloc(msgs->index, sizeof(int) * msgs->count);
        msgs->allocated = msgs->count;
        return;
    }

    for (i = 0; i < msgs->count; i++)
    {
        if (borg_read_message_equal(&msgs->messages[i], msg))
        {
            found_dup = true;
            break;
        }
    }
    if (!found_dup)
    {
        memcpy(&(msgs->messages[msgs->count]), msg, sizeof(struct borg_read_message));
        msgs->index[msgs->count] = spell_number;
        msgs->count++;
        if (msgs->count == msgs->allocated)
        {
            msgs->allocated += 10;
            msgs->messages = mem_realloc(msgs->messages, sizeof(struct borg_read_message) * msgs->allocated);
            msgs->index = mem_realloc(msgs->index, sizeof(int) * msgs->allocated);
        }
    } else {
        string_free(msg->message_p1);
        string_free(msg->message_p2);
        string_free(msg->message_p3);
    }
}

static void clean_msgs(struct borg_read_messages *msgs)
{
    int i;

    for (i = 0; i < msgs->count; ++i) {
        string_free(msgs->messages[i].message_p1);
        string_free(msgs->messages[i].message_p2);
        string_free(msgs->messages[i].message_p3);
    }
    mem_free(msgs->messages);
    msgs->messages = NULL;
    mem_free(msgs->index);
    msgs->index = NULL;
    msgs->count = 0;
    msgs->allocated = 0;
}

/* get rid of leading spaces */
static char* borg_trim_lead_space(char* orig)
{
    if (!orig || orig[0] != ' ')
        return orig;

    return &orig[1];
}

/*
 * break a string into a borg_read_message
 *
 * a message can have up to three parts broken up by variables
 * ex: "{name} hits {pronoun} followers with {type} ax."
 * " hits ", " followers with ", " ax."
 * if the message has more parts than that, they are ignored so
 * ex: "{name} hits {pronoun} followers with {type} ax and {type} breath."
 * would end up as
 * " hits ", " followers with ", " ax and "
 * hopefully this is enough to keep the messages as unique as possible
 */
static void borg_load_read_message(char* message, struct borg_read_message* read_message)
{
    read_message->message_p1 = NULL;
    read_message->message_p2 = NULL;
    read_message->message_p3 = NULL;

    char* suffix = strchr(message, '}');
    if (!suffix)
    {
        /* no variables, use message as is */
        read_message->message_p1 = string_make(borg_trim_lead_space(message));
        return;
    }
    /* skip leading variable, if there is one */
    if (message[0] == '{')
        suffix++;
    else
        suffix = message;
    char* var = strchr(suffix, '{');
    if (!var)
    {
        /* one variable, use message post variable */
        read_message->message_p1 = string_make(borg_trim_lead_space(suffix));
        return;
    }
    while (suffix[0] == ' ') suffix++;
    int part_len = strlen(suffix) - strlen(var);
    read_message->message_p1 = string_make(format("%.*s", part_len, suffix));
    suffix += part_len;
    suffix = strchr(var, '}');
    if (!suffix)
        return; /* this should never happen but ... if a string is { with no }*/
    suffix++;
    var = strchr(suffix, '{');
    if (!var)
    {
        while (suffix[0] == ' ') suffix++;

        /* two variables, ignore if last part is just . */
        if (strlen(suffix) && !streq(suffix, "."))
            read_message->message_p2 = string_make(suffix);
        return;
    }
    while (suffix[0] == ' ') suffix++;
    part_len = strlen(suffix) - strlen(var);
    if (part_len)
    {
        read_message->message_p2 = string_make(borg_trim_lead_space(
            format("%.*s", part_len, suffix)));
    }
    suffix += part_len;
    suffix = strchr(var, '}');
    if (!suffix)
        return; /* this should never happen but ... if a string is { with no }*/
    suffix++;
    var = strchr(suffix, '{');
    if (!var)
    {
        while (suffix[0] == ' ') suffix++;
        /* three variables, ignore if last part is just . */
        if (strlen(suffix) && !streq(suffix, "."))
        {
            if (read_message->message_p2)
            {
                read_message->message_p3 = string_make(borg_trim_lead_space(suffix));
            }
            else
            {
                read_message->message_p2 = string_make(borg_trim_lead_space(suffix));
            }
        }
        return;
    }
    while (suffix[0] == ' ') suffix++;
    part_len = strlen(suffix) - strlen(var);
    if (read_message->message_p2)
        read_message->message_p3 = string_make(borg_trim_lead_space(
            format("%.*s", part_len, suffix)));
    else
        read_message->message_p2 = string_make(borg_trim_lead_space(
            format("%.*s", part_len, suffix)));

    return;
}


static void borg_init_spell_messages(void)
{
    const struct monster_spell* spell = monster_spells;
    const struct monster_spell_level* spell_level;
    struct borg_read_message read_message;

    while (spell)
    {
        spell_level = spell->level;
        while (spell_level)
        {
            if (spell_level->blind_message)
            {
                borg_load_read_message(spell_level->blind_message, &read_message);
                insert_msg(&spell_invis_msgs, &read_message, spell->index);
            }
            if (spell_level->message)
            {
                borg_load_read_message(spell_level->message, &read_message);
                insert_msg(&spell_msgs, &read_message, spell->index);
            }
            if (spell_level->miss_message)
            {
                borg_load_read_message(spell_level->miss_message, &read_message);
                insert_msg(&spell_msgs, &read_message, spell->index);
            }
            spell_level = spell_level->next;
        }
        spell = spell->next;
    }
    /* null terminate */
    insert_msg(&spell_invis_msgs, NULL, 0);
    insert_msg(&spell_msgs, NULL, 0);
}

/* HACK pluralize ([|] parsing) code stolen from mon-msg.c */
/* State machine constants for get_message_text() */
#define MSG_PARSE_NORMAL	0
#define MSG_PARSE_SINGLE	1
#define MSG_PARSE_PLURAL	2

static char* borg_get_parsed_pain(const char* pain, bool do_plural)
{
    size_t buflen = strlen(pain) + 1;
    char* buf = mem_zalloc(buflen);

    int state = MSG_PARSE_NORMAL;
    size_t maxlen = strlen(pain);
    size_t pos = 1;

    /* for the borg, always start with a space */
    buf[0] = ' ';

    /* Put the message characters in the buffer */
    /* XXX This logic should be used everywhere for pluralising strings */
    for (size_t i = 0; i < maxlen && pos < buflen - 1; i++) {
        char cur = pain[i];

        /*
         * The characters '[|]' switch parsing mode and are never output.
         * The syntax is [singular|plural]
         */
        if (state == MSG_PARSE_NORMAL && cur == '[') {
            state = MSG_PARSE_SINGLE;
        }
        else if (state == MSG_PARSE_SINGLE && cur == '|') {
            state = MSG_PARSE_PLURAL;
        }
        else if (state != MSG_PARSE_NORMAL && cur == ']') {
            state = MSG_PARSE_NORMAL;
        }
        else if (state == MSG_PARSE_NORMAL ||
            (state == MSG_PARSE_SINGLE && do_plural == false) ||
            (state == MSG_PARSE_PLURAL && do_plural == true)) {
            /* Copy the characters according to the mode */
            buf[pos++] = cur;
        }
    }
    return buf;
}

static void borg_insert_pain(const char* pain, int* capacity, int* count)
{
    char* new_message;
    if (*capacity <= (*count) + 2)
    {
        *capacity += 14;
        suffix_pain = mem_realloc(suffix_pain, sizeof(char*) * (*capacity));
    }

    new_message = borg_get_parsed_pain(pain, false);
    suffix_pain[(*count)++] = new_message;
    new_message = borg_get_parsed_pain(pain, true);
    suffix_pain[(*count)++] = new_message;
}

static void borg_init_pain_messages(void)
{
    int capacity = 1;
    int count = 0;
    int idx, i;
    struct monster_pain* pain;

    suffix_pain = mem_alloc(sizeof(char*) * capacity);

    for (idx = 0; idx < z_info->mp_max; idx++)
    {
        pain = &pain_messages[idx];
        for (i = 0; i < 7; i++)
        {
            if (pain == NULL || pain->messages[i] == NULL)
                break;
            borg_insert_pain(pain->messages[i], &capacity, &count);
        }
    }

    if ((count + 1) != capacity)
        suffix_pain = mem_realloc(suffix_pain, sizeof(char*) * (count + 1));
    suffix_pain[count] = NULL;
}

static void borg_init_hit_by_messages(void)
{
    struct borg_read_message read_message;

    for (int i = 0; i < z_info->blow_methods_max; i++)
    {
        struct blow_message* messages = blow_methods[i].messages;

        while (messages)
        {
            if (messages->act_msg)
            {
                borg_load_read_message(messages->act_msg, &read_message);
                insert_msg(&suffix_hit_by, &read_message, blow_methods[i].msgt);
            }
            messages = messages->next;
        }
    }
    /* null terminate */
    insert_msg(&suffix_hit_by, NULL, 0);
}

static void borg_init_messages(void)
{
    borg_init_spell_messages();
    borg_init_pain_messages();
    borg_init_hit_by_messages();
}

static void borg_clean_messages(void)
{
    int i;

    if (suffix_pain) {
        for (i = 0; suffix_pain[i]; ++i) {
            mem_free(suffix_pain[i]);
        }
        mem_free(suffix_pain);
        suffix_pain = NULL;
    }
    clean_msgs(&suffix_hit_by);
    clean_msgs(&spell_invis_msgs);
    clean_msgs(&spell_msgs);
}

static void borg_reinit_options(void)
{
    /* Save current key mode */
    key_mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

    /* The Borg uses the original keypress codes */
    option_set("rogue_like_commands", false);

    /* No auto_more */
    option_set("auto_more", false);

    /* We pick up items when we step on them */
    option_set("pickup_always", true);

    /* We do not want verbose messages */
    option_set("pickup_detail", false);

    /* We specify targets by hand */
    option_set("use_old_target", false);

    /* We must pick items up without verification */
    option_set("pickup_inven", true);

    /* Pile symbol '&' confuse the borg */
    option_set("show_piles", false);

    /* We need space */
    option_set("show_labels", false);

    /* flavors are confusing */
    option_set("show_flavors", false);

    /* The "easy" options confuse the Borg */
    option_set("easy_open", false);
    option_set("easy_alter", false);

    /* Efficiency */
    player->opts.hitpoint_warn = 0;
}

/*
 * Tell the borg that the game was closed.
 */
static void borg_leave_game(game_event_type ev_type, game_event_data *ev_data,
        void *user)
{
    assert(ev_type == EVENT_LEAVE_GAME && ev_data == NULL && user == NULL);
    game_closed = true;
}

/*
 * Initialize the Borg
 */
void borg_init_9(void)
{
    uint8_t* test;

    /*** Hack -- verify system ***/

    /* Message */
    prt("Initializing the Borg... (memory)", 0, 0);
    borg_init_failure = false;

    /* Hack -- flush it */
    Term_fresh();

    /* Mega-Hack -- verify memory */
    test = mem_zalloc(400 * 1024L * sizeof(uint8_t));
    mem_free(test);

    /*** Hack -- initialize some stuff ***/
    borg_required_item = mem_zalloc(MAX_CLASSES * sizeof(req_item*));
    n_req = mem_zalloc(MAX_CLASSES * sizeof(int));
    borg_power_item = mem_zalloc(MAX_CLASSES * sizeof(power_item*));
    n_pwr = mem_zalloc(MAX_CLASSES * sizeof(int));


    /*** Hack -- initialize borg.ini options ***/

    /* Message */
    prt("Initializing the Borg... (borg.txt)", 0, 0);
    init_borg_txt_file();


    /*** Hack -- initialize game options ***/

    /* Message */
    prt("Initializing the Borg... (options)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();

    /* Make sure it rolls up a new guy at death */
    if (screensaver)
    {
        /* We need the borg to keep playing after he dies */
        option_set("cheat_live", true);
    }

#ifndef ALLOW_BORG_GRAPHICS
    if (!borg_graphics)
    {
        int i, j;
        /* Reset the # and % -- Scan the features */
        for (i = 1; i < FEAT_MAX; i++)
        {
            struct feature* f_ptr = &f_info[i];
#if false
            /* Skip non-features */
            if (!f_ptr->name) continue;

            /* Switch off "graphics" */
            f_ptr->x_attr[3] = f_ptr->d_attr;
            f_ptr->x_char[3] = f_ptr->d_char;
#endif

            /* Assume we will use the underlying values */
            for (j = 0; j < LIGHTING_MAX; j++) {
                feat_x_attr[j][i] = f_ptr->d_attr;
                feat_x_char[j][i] = f_ptr->d_char;
            }

        }
    }
#endif

#ifdef USE_GRAPHICS
    /* The Borg can't work with graphics on, so switch it off */
    if (use_graphics)
    {
        /* Reset to ASCII mode */
        use_graphics = false;
        arg_graphics = false;

        /* Reset visuals */
        reset_visuals(true);
    }
#endif /* USE_GRAPHICS */

    /*** Redraw ***/
    /* Redraw map */
    player->upkeep->redraw |= (PR_MAP);

    /* Redraw everything */
    do_cmd_redraw();
    /*** Various ***/

    /* Message */
    prt("Initializing the Borg... (various)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();


    /*** Cheat / Panic ***/

    /* more cheating */
    borg_cheat_death = false;

    /* set the continous play mode if the game cheat death is on */
    if (OPT(player, cheat_live)) borg_cheat_death = true;

    /*** Initialize ***/

    borg_init_messages();

    /* Initialize */
    borg_init_1();
    borg_init_2();
    borg_init_3();
    borg_init_4();
    borg_init_5();
    borg_init_6();
    borg_init_7();
    borg_init_8();

#if 0
    /* Maintain a correct base for randart values. */
    for (i = 0; i < artifact_count; i++)
    {
        artifact_type* a_ptr = &a_info[i];

        borg_art_save_tval[i] = a_ptr->tval;
        borg_art_save_sval[i] = a_ptr->sval;
        borg_art_save_pval[i] = a_ptr->pval;

        borg_art_save_to_h[i] = a_ptr->to_h;
        borg_art_save_to_d[i] = a_ptr->to_d;
        borg_art_save_to_a[i] = a_ptr->to_a;
        borg_art_save_ac[i] = a_ptr->ac;

        borg_art_save_dd[i] = a_ptr->dd;
        borg_art_save_ds[i] = a_ptr->ds;

        borg_art_save_weight[i] = a_ptr->weight;

        borg_art_save_cost[i] = a_ptr->cost;

        borg_art_save_flags1[i] = a_ptr->flags1;
        borg_art_save_flags2[i] = a_ptr->flags2;
        borg_art_save_flags3[i] = a_ptr->flags3;

        borg_art_save_level[i] = a_ptr->level;
        borg_art_save_rarity[i] = a_ptr->rarity;

        borg_art_save_activation[i] = a_ptr->activation;
        borg_art_save_time[i] = a_ptr->time;
        borg_art_save_randtime[i] = a_ptr->randtime;
    }
#endif

    /*** Hack -- Extract race ***/

    /* Insert the player Race--cheat */
    borg_race = player->race->ridx;

    /*** Hack -- Extract class ***/
    borg_class = player->class->cidx;

    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    borg_prepare_race_class_info();

    /*
     * Notice if the game is closed so a reinitialization can be done if the
     * game is restarted without exiting.
     */
    game_closed = false;
    event_add_handler(EVENT_LEAVE_GAME, borg_leave_game, NULL);

    /*** All done ***/

    /* Done initialization */
    prt("Initializing the Borg... done.", 0, 0);

    /* Clear line */
    prt("", 0, 0);

    /* Reset the clock */
    borg_t = 10;

    /* Official message */
    if (!borg_init_failure)
        borg_note("# Ready...");

    /* Now it is ready */
    initialized = true;
}

/*
 * Clean up resources allocated for the borg.
 */
void borg_clean_9(void)
{
    /* Undo the allocations in reverse order from what borg_init_9() does. */
    event_remove_handler(EVENT_LEAVE_GAME, borg_leave_game, NULL);
    borg_clean_8();
    borg_clean_7();
    borg_clean_6();
    borg_clean_5();
    borg_clean_4();
    borg_clean_3();
    borg_clean_2();
    borg_clean_1();
    borg_clean_messages();
    clean_borg_txt_file();
    mem_free(n_pwr);
    n_pwr = NULL;
    mem_free(borg_power_item);
    borg_power_item = NULL;
    mem_free(n_req);
    n_req = NULL;
    mem_free(borg_required_item);
    borg_required_item = NULL;
}

/* 
 * Convert an inventory index into a one character label.
 *
 * Note that the label does NOT distinguish inven/equip.
 */
static char borg_index_to_label(int i)
{
    /* Indexes for "inven" are easy */
    if (i < INVEN_WIELD) 
        return all_letters_nohjkl[i];

    /* Indexes for "equip" are offset */
    return all_letters_nohjkl[i - INVEN_WIELD];
}

#ifndef BABLOS
/*
 * Write a file with the current dungeon info (Borg)
 * and his equipment, inventory and home (Player)
 * and his swap armor, weapon (Borg)
 * From Dennis Van Es,  With an addition of last messages from me (APW)
 */
void borg_write_map(bool ask)
{
    char buf2[1024];
    char buf[80];
    ang_file* borg_map_file = NULL;
    wchar_t* line;
    char* ch_line;

    borg_item* item;
    int i, j;
    int to, itemm;

    int16_t m_idx;

    struct store* st_ptr = &stores[7];

    char o_name[80];

    /* Process the player name */
    for (i = 0; player->full_name[i]; i++)
    {
        char c = player->full_name[i];

        /* No control characters */
        if (iscntrl(c))
        {
            /* Illegal characters */
            quit_fmt("Illegal control char (0x%02X) in player name", c);
        }

        /* Convert all non-alphanumeric symbols */
        if (!isalpha(c) && !isdigit(c)) c = '_';

        /* Build "file_name" */
        buf[i] = c;
    }

    /* Terminate */
    buf[i++] = '.';
    buf[i++] = 'm';
    buf[i++] = 'a';
    buf[i++] = 'p';
    buf[i++] = '\0';

    path_build(buf2, 1024, ANGBAND_DIR_USER, buf);

    /* XXX XXX XXX Get the name and open the map file */
    if (ask && get_string("Borg map File: ", buf2, 70))
    {
        /* Open a new file */
        borg_map_file = file_open(buf2, MODE_WRITE, FTYPE_TEXT);

        /* Failure */
        if (!borg_map_file) msg("Cannot open that file.");
    }
    else if (!ask) borg_map_file = file_open(buf2, MODE_WRITE, FTYPE_TEXT);

    file_putf(borg_map_file, "%s the %s %s, Level %d/%d\n", player->full_name,
        player->race->name,
        player->class->name,
        player->lev, player->max_lev);

    file_putf(borg_map_file, "Exp: %lu  Gold: %lu  Turn: %lu\n", (long)(player->max_exp + (100 * player->max_depth)), (long)player->au, (long)turn);
    file_putf(borg_map_file, "Killed on level: %d (max. %d) by %s\n\n", player->depth, player->max_depth, player->died_from);
    file_putf(borg_map_file, "Borg Compile Date: %s\n", borg_engine_date);

    line = mem_zalloc((DUNGEON_WID + 1) * sizeof(wchar_t));
    ch_line = mem_zalloc((DUNGEON_WID + 1) * sizeof(char));
    for (i = 0; i < DUNGEON_HGT; i++)
    {
        for (j = 0; j < DUNGEON_WID; j++)
        {
            wchar_t ch;

            borg_grid* ag = &borg_grids[i][j];
            if (square_monster(cave, loc(j, i)))
                m_idx = square_monster(cave, loc(j, i))->midx;
            else
                m_idx = 0;

            /* reset the ch each time through */
            ch = ' ';

            /* Known grids */
            if (ag->feat)
            {
                ch = f_info[ag->feat].d_char;
            }

            /* Known Items */
            if (ag->take)
            {
                borg_take* take = &borg_takes[ag->take];
                if (take->kind)
                {
                    struct object_kind* k_ptr = take->kind;
                    ch = k_ptr->d_char;
                }
            }

            /* UnKnown Monsters */
            if (m_idx)
            {
                ch = '&';
            }

            /* Known Monsters */
            if (ag->kill)
            {
                borg_kill* kill = &borg_kills[ag->kill];
                struct monster_race* r_ptr = &r_info[kill->r_idx];
                ch = r_ptr->d_char;
            }


            /* The Player */
            if ((i == c_y) && (j == c_x)) ch = '@';

            line[j] = ch;
        }
        /* terminate the line */
        line[j++] = '\0';

        wcstombs(ch_line, line, wcslen(line) + 1);

        file_putf(borg_map_file, "%s\n", ch_line);
    }
    mem_free(line);
    mem_free(ch_line);

    /* Known/Seen monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill* kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Note */
        file_putf(borg_map_file, "monster '%s' (%d) at (%d,%d) speed:%d \n",
            (r_info[kill->r_idx].name), kill->r_idx,
            kill->y, kill->x, kill->speed);
    }

    /*** Dump the last few messages ***/
    i = messages_num();
    if (i > 250) i = 250;
    file_putf(borg_map_file, "\n\n  [Last Messages]\n\n");
    while (i-- > 0)
    {
        const char* msg = message_str((int16_t)i);

        /* Eliminate some lines */
        if (prefix(msg, "# Matched")
            || prefix(msg, "# There is")
            || prefix(msg, "# Tracking")
            || prefix(msg, "# MISS_BY:")
            || prefix(msg, "# HIT_BY:")
            || prefix(msg, "> "))
            continue;

        file_putf(borg_map_file, "%s\n", msg);
    }

    /*** Player Equipment ***/
    file_putf(borg_map_file, "\n\n  [Character Equipment]\n\n");
    for (i = 0; i < player->body.count; i++)
    {
        struct object* obj = player->body.slots[i].obj;
        object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, player);
        file_putf(borg_map_file, "%c) %s\n", borg_index_to_label(i), o_name);
    }
    file_putf(borg_map_file, "\n\n  [Character Quiver]\n\n");
    for (i = 0; i < z_info->quiver_size; i++)
    {
        struct object* obj = player->upkeep->quiver[i];
        object_desc(o_name, sizeof(o_name), obj, ODESC_FULL, player);
        file_putf(borg_map_file, "%c) %s\n", borg_index_to_label(i), o_name);
    }

    file_putf(borg_map_file, "\n\n");


    /* Dump the inventory */
    file_putf(borg_map_file, "  [Character Inventory]\n\n");
    for (i = 0; i < z_info->pack_size; i++)
    {
        item = &borg_items[i];

        file_putf(borg_map_file, "%c) %s\n",
            borg_index_to_label(i), item->desc);
    }
    file_putf(borg_map_file, "\n\n");


    /* Dump the Home (page 1) */
    file_putf(borg_map_file, "  [Home Inventory (page 1)]\n\n");
    struct object** list = mem_zalloc(sizeof(struct object*) * z_info->store_inven_max);
    store_stock_list(st_ptr, list, z_info->store_inven_max);
    for (i = 0; i < z_info->store_inven_max / 2; i++)
    {
        object_desc(o_name, sizeof(o_name), list[i], ODESC_FULL, player);
        file_putf(borg_map_file, "%c) %s\n", all_letters_nohjkl[i % 12], o_name);
    }
    file_putf(borg_map_file, "\n\n");

    /* Dump the Home (page 2) */
    file_putf(borg_map_file, "  [Home Inventory (page 2)]\n\n");
    for (i = z_info->store_inven_max / 2; i < z_info->store_inven_max; i++)
    {
        object_desc(o_name, sizeof(o_name), list[i], ODESC_FULL, player);
        file_putf(borg_map_file, "%c) %s\n", all_letters_nohjkl[i % 12], o_name);
    }
    file_putf(borg_map_file, "\n\n");
    mem_free(list);

    /* Write swap info */
    if (borg_cfg[BORG_USES_SWAPS])
    {
        file_putf(borg_map_file, "  [Swap info]\n\n");
        if (weapon_swap)
        {
            item = &borg_items[weapon_swap - 1];
            file_putf(borg_map_file, "Swap Weapon:  %s\n", item->desc);
        }
        else
        {
            file_put(borg_map_file, "Swap Weapon:  NONE\n");
        }
        if (armour_swap)
        {
            item = &borg_items[armour_swap - 1];
            file_putf(borg_map_file, "Swap Armour:  %s\n", item->desc);
        }
        else
        {
            file_put(borg_map_file, "Swap Armour:  NONE\n");
        }
        file_putf(borg_map_file, "\n\n");
    }
    file_putf(borg_map_file, "   [Player State at Death] \n\n");

    /* Dump the player state */
    file_putf(borg_map_file, "Current speed: %d. \n", borg_skill[BI_SPEED]);

    if (player->timed[TMD_BLIND])
    {
        file_putf(borg_map_file, "You cannot see.\n");
    }
    if (player->timed[TMD_CONFUSED])
    {
        file_putf(borg_map_file, "You are confused.\n");
    }
    if (player->timed[TMD_AFRAID])
    {
        file_putf(borg_map_file, "You are terrified.\n");
    }
    if (player->timed[TMD_CUT])
    {
        file_putf(borg_map_file, "You are bleeding.\n");
    }
    if (player->timed[TMD_STUN])
    {
        file_putf(borg_map_file, "You are stunned.\n");
    }
    if (player->timed[TMD_POISONED])
    {
        file_putf(borg_map_file, "You are poisoned.\n");
    }
    if (player->timed[TMD_IMAGE])
    {
        file_putf(borg_map_file, "You are hallucinating.\n");
    }
    if (player_of_has(player, OF_AGGRAVATE))
    {
        file_putf(borg_map_file, "You aggravate monsters.\n");
    }
    if (player->timed[TMD_BLESSED])
    {
        file_putf(borg_map_file, "You feel rightous.\n");
    }
    if (player->timed[TMD_HERO])
    {
        file_putf(borg_map_file, "You feel heroic.\n");
    }
    if (player->timed[TMD_SHERO])
    {
        file_putf(borg_map_file, "You are in a battle rage.\n");
    }
    if (player->timed[TMD_PROTEVIL])
    {
        file_putf(borg_map_file, "You are protected from evil.\n");
    }
    if (player->timed[TMD_SHIELD])
    {
        file_putf(borg_map_file, "You are protected by a mystic shield.\n");
    }
    if (player->timed[TMD_INVULN])
    {
        file_putf(borg_map_file, "You are temporarily invulnerable.\n");
    }
    if (player->timed[TMD_CONFUSED])
    {
        file_putf(borg_map_file, "Your hands are glowing dull red.\n");
    }
    if (player->word_recall)
    {
        file_putf(borg_map_file, "You will soon be recalled.  (%d turns)\n", player->word_recall);
    }
    if (player->timed[TMD_OPP_FIRE])
    {
        file_putf(borg_map_file, "You resist fire exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_ACID])
    {
        file_putf(borg_map_file, "You resist acid exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_ELEC])
    {
        file_putf(borg_map_file, "You resist elec exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_COLD])
    {
        file_putf(borg_map_file, "You resist cold exceptionally well.\n");
    }
    if (player->timed[TMD_OPP_POIS])
    {
        file_putf(borg_map_file, "You resist poison exceptionally well.\n");
    }
    file_putf(borg_map_file, "\n\n");

    /* Dump the Time Variables */
    file_putf(borg_map_file, "Time on this panel; %d\n", time_this_panel);
    file_putf(borg_map_file, "Time on this level; %d\n", borg_t - borg_began);
    file_putf(borg_map_file, "Time since left town; %d\n", borg_time_town + (borg_t - borg_began));
    file_putf(borg_map_file, "Food in town; %d\n", borg_food_onsale);
    file_putf(borg_map_file, "Fuel in town; %d\n", borg_fuel_onsale);
    file_putf(borg_map_file, "Borg_no_retreat; %d\n", borg_no_retreat);
    file_putf(borg_map_file, "Breeder_level; %d\n", breeder_level);
    file_putf(borg_map_file, "Unique_on_level; %d\n", unique_on_level);
    if ((turn % (10L * z_info->day_length)) < ((10L * z_info->day_length) / 2))
        file_putf(borg_map_file, "It is daytime in town.\n");
    else file_putf(borg_map_file, "It is night-time in town.\n");
    file_putf(borg_map_file, "\n\n");

    file_putf(borg_map_file, "borg_uses_swaps; %d\n", borg_cfg[BORG_USES_SWAPS]);
    file_putf(borg_map_file, "borg_worships_damage; %d\n", borg_cfg[BORG_WORSHIPS_DAMAGE]);
    file_putf(borg_map_file, "borg_worships_speed; %d\n", borg_cfg[BORG_WORSHIPS_SPEED]);
    file_putf(borg_map_file, "borg_worships_hp; %d\n", borg_cfg[BORG_WORSHIPS_HP]);
    file_putf(borg_map_file, "borg_worships_mana; %d\n", borg_cfg[BORG_WORSHIPS_MANA]);
    file_putf(borg_map_file, "borg_worships_ac; %d\n", borg_cfg[BORG_WORSHIPS_AC]);
    file_putf(borg_map_file, "borg_worships_gold; %d\n", borg_cfg[BORG_WORSHIPS_GOLD]);
    file_putf(borg_map_file, "borg_plays_risky; %d\n", borg_cfg[BORG_PLAYS_RISKY]);
    file_putf(borg_map_file, "borg_slow_optimizehome; %d\n\n", borg_cfg[BORG_SLOW_OPTIMIZEHOME]);
    file_putf(borg_map_file, "borg_scumming_pots; %d\n\n", borg_scumming_pots);
    file_putf(borg_map_file, "\n\n");


    /* Dump the spells */
    if (player->class->magic.total_spells)
    {
        file_putf(borg_map_file, "\n\n   [ Spells ] \n\n");
        file_putf(borg_map_file, "Name                           Legal Times cast\n");
        for (i = 0; i < player->class->magic.total_spells; i++)
        {
            borg_magic* as = &borg_magics[i];
            int failpercent = 0;

            if (as->level < 99)
            {
                const char *legal = (borg_spell_legal(as->spell_enum) ? "Yes" : "No ");
                failpercent = (borg_spell_fail_rate(as->spell_enum));

                file_putf(borg_map_file, "%-30s   %s   %ld   fail:%d \n", as->name, legal, (long)as->times, failpercent);
            }
            file_putf(borg_map_file, "\n");
        }
    }

    /* Dump the borg_skill[] information */
    itemm = z_info->k_max;
    to = z_info->k_max + BI_MAX;
    for (; itemm < to; itemm++)
    {
        file_putf(borg_map_file, "skill %d (%s) value= %d.\n", itemm,
            prefix_pref[itemm - z_info->k_max], borg_has[itemm]);
    }

#if 0
    /* Allocate the "okay" array */
    C_MAKE(okay, z_info->a_max, bool);

    /*** Dump the Uniques and Artifact Lists ***/

    /* Scan the artifacts */
    for (k = 0; k < z_info->a_max; k++)
    {
        artifact_type* a_ptr = &a_info[k];

        /* Default */
        okay[k] = false;

        /* Skip "empty" artifacts */
        if (!a_ptr->name) continue;

        /* Skip "uncreated" artifacts */
        if (!a_ptr->cur_num) continue;

        /* Assume okay */
        okay[k] = true;
    }

    /* Check the dungeon */
    for (y = 0; y < DUNGEON_HGT; y++)
    {
        for (x = 0; x < DUNGEON_WID; x++)
        {
            int16_t this_o_idx, next_o_idx = 0;

            /* Scan all objects in the grid */
            for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
            {
                object_type* o_ptr;

                /* Get the object */
                o_ptr = &o_list[this_o_idx];

                /* Get the next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Ignore non-artifacts */
                if (!artifact_p(o_ptr)) continue;

                /* Ignore known items */
                if (object_is_known(o_ptr)) continue;

                /* Note the artifact */
                okay[o_ptr->name1] = false;
            }
        }
    }

    /* Check the inventory and equipment */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        object_type* o_ptr = &player->inventory[i];

        /* Ignore non-objects */
        if (!o_ptr->k_idx) continue;

        /* Ignore non-artifacts */
        if (!artifact_p(o_ptr)) continue;

        /* Ignore known items */
        if (object_is_known(o_ptr)) continue;

        /* Note the artifact */
        okay[o_ptr->name1] = false;
    }

    file_putf(borg_map_file, "\n\n");


    /* Hack -- Build the artifact name */
    file_putf(borg_map_file, "   [Artifact Info] \n\n");

    /* Scan the artifacts */
    for (k = 0; k < z_info->a_max; k++)
    {
        artifact_type* a_ptr = &a_info[k];

        /* List "dead" ones */
        if (!okay[k]) continue;

        /* Paranoia */
        my_strcpy(o_name, "Unknown Artifact", sizeof(o_name));

        /* Obtain the base object type */
        z = borg_lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Real object */
        if (z)
        {
            object_type* i_ptr;
            object_type object_type_body;

            /* Get local object */
            i_ptr = &object_type_body;

            /* Create fake object */
            object_prep(i_ptr, z);

            /* Make it an artifact */
            i_ptr->name1 = k;

            /* Describe the artifact */
            object_desc_spoil(o_name, sizeof(o_name), i_ptr, false, 0);
        }

        /* Hack -- Build the artifact name */
        file_putf(borg_map_file, "The %s\n", o_name);
    }

    /* Free the "okay" array */
    mem_free(okay);
    file_putf(borg_map_file, "\n\n");

    /* Display known uniques
     *
     * Note that the player ghosts are ignored.  XXX XXX XXX
     */
     /* Allocate the "who" array */
    C_MAKE(who, z_info->r_max, uint16_t);

    /* Collect matching monsters */
    for (i = 1, n = 0; i < z_info->r_max; i++)
    {
        monster_race* r_ptr = &r_info[i];
        monster_lore* l_ptr = &l_list[i];

        /* Require known monsters */
        if (!cheat_know && !l_ptr->r_sights) continue;

        /* Require unique monsters */
        if (!(rf_has(r_ptr->flags, RF_UNIQUE))) continue;

        /* Collect "appropriate" monsters */
        who[n++] = i;
    }

    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort the array by dungeon depth of monsters */
    borg_sort(who, &why, n);


    /* Hack -- Build the artifact name */
    file_putf(borg_map_file, "   [Unique Info] \n\n");

    /* Print the monsters */
    for (i = 0; i < n; i++)
    {
        monster_race* r_ptr = &r_info[who[i]];
        bool dead = (r_ptr->max_num == 0);

        /* Print a message */
        file_putf(borg_map_file, "%s is %s\n",
            (r_ptr->name),
            (dead ? "dead" : "alive"));
    }

    /* Free the "who" array */
    mem_free(who);

#endif  /* extra dump stuff */

    file_close(borg_map_file);
}

/* Try to make a scumfile so we can see what went wrong. */
void borg_save_scumfile(void)
{
    static char svSavefile[1024];
    static char svSavefile2[1024];
    int i;

    /* Create a scum file */
    if (borg_cfg[BORG_SAVE_DEATH] &&
        (borg_skill[BI_CLEVEL] >= borg_cfg[BORG_DUMP_LEVEL] ||
            strstr(player->died_from, "starvation")))
    {
        memcpy(svSavefile, savefile, sizeof(savefile));
        /* Process the player name */
        for (i = 0; player->full_name[i]; i++)
        {
            char c = player->full_name[i];

            /* No control characters */
            if (iscntrl(c))
            {
                /* Illegal characters */
                quit_fmt("Illegal control char (0x%02X) in player name", c);
            }

            /* Convert all non-alphanumeric symbols */
            if (!isalpha(c) && !isdigit(c)) c = '_';

            /* Build "file_name" */
            svSavefile2[i] = c;
        }
        svSavefile2[i] = 0;

        path_build(savefile, 1024, ANGBAND_DIR_USER, svSavefile2);
    }

}
#endif /* BABLOS */

/* DVE's function for displaying the status of various info */
/* Display what the borg is thinking DvE*/
void borg_status(void)
{
    int j;

    /* Scan windows */
    for (j = 0; j < 8; j++)
    {
        term* old = Term;

        /* Unused */
        if (!angband_term[j]) continue;

        /* Check for borg status term */
        if (window_flag[j] & (PW_BORG_2))
        {
            uint8_t attr;

            /* Activate */
            Term_activate(angband_term[j]);

            /* Display what resists the borg (thinks he) has */
            Term_putstr(5, 0, -1, COLOUR_WHITE, "RESISTS");

            /* Basic four */
            attr = COLOUR_SLATE;
            if (borg_skill[BI_RACID]) attr = COLOUR_BLUE;
            if (borg_skill[BI_TRACID]) attr = COLOUR_GREEN;
            if (borg_skill[BI_IACID]) attr = COLOUR_WHITE;
            Term_putstr(1, 1, -1, attr, "Acid");

            attr = COLOUR_SLATE;
            if (borg_skill[BI_RELEC]) attr = COLOUR_BLUE;
            if (borg_skill[BI_TRELEC]) attr = COLOUR_GREEN;
            if (borg_skill[BI_IELEC]) attr = COLOUR_WHITE;
            Term_putstr(1, 2, -1, attr, "Elec");

            attr = COLOUR_SLATE;
            if (borg_skill[BI_RFIRE]) attr = COLOUR_BLUE;
            if (borg_skill[BI_TRFIRE]) attr = COLOUR_GREEN;
            if (borg_skill[BI_IFIRE]) attr = COLOUR_WHITE;
            Term_putstr(1, 3, -1, attr, "Fire");

            attr = COLOUR_SLATE;
            if (borg_skill[BI_RCOLD]) attr = COLOUR_BLUE;
            if (borg_skill[BI_TRCOLD]) attr = COLOUR_GREEN;
            if (borg_skill[BI_ICOLD]) attr = COLOUR_WHITE;
            Term_putstr(1, 4, -1, attr, "Cold");

            /* High resists */
            attr = COLOUR_SLATE;
            if (borg_skill[BI_RPOIS]) attr = COLOUR_BLUE;
            if (borg_skill[BI_TRPOIS]) attr = COLOUR_GREEN;
            Term_putstr(1, 5, -1, attr, "Pois");

            if (borg_skill[BI_RFEAR]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(1, 6, -1, attr, "Fear");

            if (borg_skill[BI_RLITE]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(1, 7, -1, attr, "Lite");

            if (borg_skill[BI_RDARK]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(1, 8, -1, attr, "Dark");

            if (borg_skill[BI_RBLIND]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 1, -1, attr, "Blind");

            if (borg_skill[BI_RCONF]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 2, -1, attr, "Confu");

            if (borg_skill[BI_RSND]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 3, -1, attr, "Sound");

            if (borg_skill[BI_RSHRD]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 4, -1, attr, "Shard");

            if (borg_skill[BI_RNXUS]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 5, -1, attr, "Nexus");

            if (borg_skill[BI_RNTHR]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 6, -1, attr, "Nethr");

            if (borg_skill[BI_RKAOS]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 7, -1, attr, "Chaos");

            if (borg_skill[BI_RDIS]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(6, 8, -1, attr, "Disen");

            /* Other abilities */
            if (borg_skill[BI_SDIG]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 1, -1, attr, "S.Dig");

            if (borg_skill[BI_FEATH]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 2, -1, attr, "Feath");

            if (borg_skill[BI_LIGHT]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 3, -1, attr, "PLite");

            if (borg_skill[BI_REG]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 4, -1, attr, "Regen");

            if (borg_skill[BI_ESP]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 5, -1, attr, "Telep");

            if (borg_skill[BI_SINV]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 6, -1, attr, "Invis");

            if (borg_skill[BI_FRACT]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 7, -1, attr, "FrAct");

            if (borg_skill[BI_HLIFE]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(12, 8, -1, attr, "HLife");

            /* Display the slays */
            Term_putstr(5, 10, -1, COLOUR_WHITE, "Weapon Slays:");

            if (borg_skill[BI_WS_ANIMAL]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(1, 11, -1, attr, "Animal");

            if (borg_skill[BI_WS_EVIL]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(8, 11, -1, attr, "Evil");

            if (borg_skill[BI_WS_UNDEAD]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(15, 11, -1, attr, "Undead");

            if (borg_skill[BI_WS_DEMON]) attr = COLOUR_BLUE;
            if (borg_skill[BI_WK_DEMON]) attr = COLOUR_GREEN;
            else attr = COLOUR_SLATE;
            Term_putstr(22, 11, -1, attr, "Demon");

            if (borg_skill[BI_WS_ORC]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(1, 12, -1, attr, "Orc");

            if (borg_skill[BI_WS_TROLL]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(8, 12, -1, attr, "Troll");

            if (borg_skill[BI_WS_GIANT]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(15, 12, -1, attr, "Giant");

            if (borg_skill[BI_WS_DRAGON]) attr = COLOUR_BLUE;
            if (borg_skill[BI_WK_DRAGON]) attr = COLOUR_GREEN;
            else attr = COLOUR_SLATE;
            Term_putstr(22, 12, -1, attr, "Dragon");

            if (borg_skill[BI_WB_ACID]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(1, 13, -1, attr, "Acid");

            if (borg_skill[BI_WB_COLD]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(8, 13, -1, attr, "Cold");

            if (borg_skill[BI_WB_ELEC]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(15, 13, -1, attr, "Elec");

            if (borg_skill[BI_WB_FIRE]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(22, 13, -1, attr, "Fire");


            /* Display the Concerns */
            Term_putstr(36, 10, -1, COLOUR_WHITE, "Concerns:");

            if (borg_skill[BI_FIRST_CURSED]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(29, 11, -1, attr, "Cursed");

            if (borg_skill[BI_ISWEAK]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(36, 11, -1, attr, "Weak");

            if (borg_skill[BI_ISPOISONED]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(43, 11, -1, attr, "Poison");

            if (borg_skill[BI_ISCUT]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(29, 12, -1, attr, "Cut");

            if (borg_skill[BI_ISSTUN]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(36, 12, -1, attr, "Stun");

            if (borg_skill[BI_ISCONFUSED]) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(43, 12, -1, attr, "Confused");

            if (goal_fleeing) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(29, 13, -1, attr, "Goal Fleeing");

            if (borg_no_rest_prep > 0) attr = COLOUR_BLUE;
            else attr = COLOUR_SLATE;
            Term_putstr(43, 13, -1, attr, "No Resting");

            /* Display the Time */
            Term_putstr(60, 10, -1, COLOUR_WHITE, "Time:");

            Term_putstr(54, 11, -1, COLOUR_SLATE, "This Level         ");
            Term_putstr(65, 11, -1, COLOUR_WHITE, format("%d", borg_t - borg_began));

            Term_putstr(54, 12, -1, COLOUR_SLATE, "Since Town         ");
            Term_putstr(65, 12, -1, COLOUR_WHITE, format("%d", borg_time_town + (borg_t - borg_began)));

            Term_putstr(54, 13, -1, COLOUR_SLATE, "This Panel         ");
            Term_putstr(65, 13, -1, COLOUR_WHITE, format("%d", time_this_panel));


            /* Sustains */
            Term_putstr(19, 0, -1, COLOUR_WHITE, "Sustains");

            if (borg_skill[BI_SSTR]) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(21, 1, -1, attr, "STR");

            if (borg_skill[BI_SINT]) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(21, 2, -1, attr, "INT");

            if (borg_skill[BI_SWIS]) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(21, 3, -1, attr, "WIS");

            if (borg_skill[BI_SDEX]) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(21, 4, -1, attr, "DEX");

            if (borg_skill[BI_SCON]) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(21, 5, -1, attr, "CON");


            /* Temporary effects */
            Term_putstr(28, 0, -1, COLOUR_WHITE, "Temp Effects");

            if (borg_prot_from_evil) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 1, -1, attr, "Prot. Evil");

            if (borg_fastcast) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 2, -1, attr, "Fastcast");

            if (borg_hero) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 3, -1, attr, "Heroism");

            if (borg_berserk) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 4, -1, attr, "Berserk");

            if (borg_shield) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 5, -1, attr, "Shielded");

            if (borg_bless) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 6, -1, attr, "Blessed");

            if (borg_speed) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 7, -1, attr, "Fast");

            if (borg_see_inv >= 1) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(28, 8, -1, attr, "See Inv");

            /* Temporary effects */
            Term_putstr(42, 0, -1, COLOUR_WHITE, "Level Information");

            if (vault_on_level) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(42, 1, -1, attr, "Vault on level");

            if (unique_on_level) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(42, 2, -1, attr, "Unique on level");
            if (unique_on_level) Term_putstr(58, 2, -1, attr, format("(%s)",
                r_info[unique_on_level].name));
            else Term_putstr(58, 2, -1, attr, "                                                ");

            if (scaryguy_on_level) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(42, 3, -1, attr, "Scary Guy on level");

            if (breeder_level) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(42, 4, -1, attr, "Breeder level (closing doors)");

            if (borg_kills_summoner != -1) attr = COLOUR_WHITE;
            else attr = COLOUR_SLATE;
            Term_putstr(42, 5, -1, attr, "Summoner very close (AS-Corridor)");

            /* level preparedness */
            attr = COLOUR_SLATE;
            Term_putstr(42, 6, -1, attr, "Reason for not diving:");
            attr = COLOUR_WHITE;
            Term_putstr(64, 6, -1, attr, format("%s                              ", borg_prepared(borg_skill[BI_MAXDEPTH] + 1)));

            attr = COLOUR_SLATE;
            Term_putstr(42, 7, -1, attr, "Scumming: not active                          ");
            if (borg_cfg[BORG_MONEY_SCUM_AMOUNT] != 0)
            {
                attr = COLOUR_WHITE;
                Term_putstr(42, 7, -1, attr, format("Scumming: $%d                  ", borg_cfg[BORG_MONEY_SCUM_AMOUNT]));
            }
            attr = COLOUR_SLATE;
            Term_putstr(42, 8, -1, attr, "Maximal Depth:");
            attr = COLOUR_WHITE;
            Term_putstr(56, 8, -1, attr, format("%d    ", borg_skill[BI_MAXDEPTH]));

            /* Important endgame information */
            if (borg_skill[BI_MAXDEPTH] >= 50) /* 85 */
            {
                Term_putstr(5, 15, -1, COLOUR_WHITE, "Important Deep Events:");

                attr = COLOUR_SLATE;
                Term_putstr(1, 16, -1, attr, "Home *Heal*:        ");
                attr = COLOUR_WHITE;
                Term_putstr(13, 16, -1, attr, format("%d   ", num_ezheal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 17, -1, attr, "Home Heal:        ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 17, -1, attr, format("%d   ", num_heal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 18, -1, attr, "Home Life:        ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 18, -1, attr, format("%d   ", num_life));

                attr = COLOUR_SLATE;
                Term_putstr(1, 19, -1, attr, "Res_Mana:        ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 19, -1, attr, format("%d   ", num_mana));

                if (morgoth_on_level)  attr = COLOUR_BLUE;
                else attr = COLOUR_SLATE;
                Term_putstr(1, 20, -1, attr, format("Morgoth on Level.  Last seen:%d       ", borg_t - borg_t_morgoth));

                if (borg_morgoth_position)  attr = COLOUR_BLUE;
                else attr = COLOUR_SLATE;
                if (borg_needs_new_sea) attr = COLOUR_WHITE;
                Term_putstr(1, 21, -1, attr, "Sea of Runes.");

                if (borg_ready_morgoth)  attr = COLOUR_BLUE;
                else attr = COLOUR_SLATE;
                Term_putstr(1, 22, -1, attr, "Ready for Morgoth.");
            }
            else
            {
                Term_putstr(5, 15, -1, COLOUR_WHITE, "                        ");

                attr = COLOUR_SLATE;
                Term_putstr(1, 16, -1, attr, "                    ");
                attr = COLOUR_WHITE;
                Term_putstr(10, 16, -1, attr, format("%d       ", num_ezheal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 17, -1, attr, "                    ");
                attr = COLOUR_WHITE;
                Term_putstr(10, 17, -1, attr, format("%d       ", num_life));

                attr = COLOUR_SLATE;
                Term_putstr(1, 18, -1, attr, "                    ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 18, -1, attr, format("%d       ", num_heal));

                attr = COLOUR_SLATE;
                Term_putstr(1, 19, -1, attr, "                   ");
                attr = COLOUR_WHITE;
                Term_putstr(11, 19, -1, attr, format("%d       ", num_mana));
            }


            /* Fresh */
            Term_fresh();

            /* Restore */
            Term_activate(old);
        }
    }
}


/*
 * Hack -- forward declare
 */
void do_cmd_borg(void);


/*
 * Hack -- interact with the "Ben Borg".
 */
void do_cmd_borg(void)
{
    char cmd;

#ifdef BABLOS
    if (auto_play)
    {
        auto_play = false;
        keep_playing = true;
        cmd.code = 'z';
    }
    else
    {

#endif /* BABLOS */

        /* Get a "Borg command", or abort */
#ifdef XSCREENSAVER
        if (auto_start_borg == false)
#endif
        {
            if (!get_com("Borg command: ", &cmd)) return;
        }

#ifdef BABLOS

    }

#endif /* BABLOS */

    /* Simple help */
    if (cmd == '?')
    {
        int i = 2;

        /* Save the screen */
        Term_save();

        /* Clear the screen */
        Term_clear();

        i++;
        Term_putstr(2, i, -1, COLOUR_WHITE, "Command 'z' activates the Borg.");
        Term_putstr(42, i++, -1, COLOUR_WHITE, "Command 'u' updates the Borg.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'x' steps the Borg.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'f' modifies the normal flags.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'c' modifies the cheat flags.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'l' activates a log file.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 's' activates search mode.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'i' displays grid info.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'g' displays grid feature.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'a' displays avoidances.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'k' displays monster info.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 't' displays object info.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '%' displays targetting flow.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command '#' displays danger grid.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '_' Regional Fear info.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'p' Borg Power.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '1' change max depth.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command '2' level prep info.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '3' Feature of grid.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command '!' Time.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command '@' Borg LOS.");
        Term_putstr(42, i, -1, COLOUR_WHITE, "Command 'w' My Swap Weapon.");
        Term_putstr(2, i++, -1, COLOUR_WHITE, "Command 'q' Auto stop on level.");
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
    if (!initialized || game_closed)
    {
        if (initialized) {
            borg_clean_9();
        }
        borg_init_9();

        if (borg_init_failure)
        {
            initialized = false;
            borg_note("** startup failure borg cannot run ** ");
            Term_fresh();
            return;
        }
    }

    switch (cmd)
    {
        /* Command: Nothing */
    case '$':
    {
        int j;

        /*** Hack -- initialize borg.ini options ***/

        /* Message */
        borg_note("Reloading the Borg rules... (borg.txt)");

        for (j = 0; j < MAX_CLASSES; j++)
        {
            mem_free(borg_required_item[j]); /* externalize the 400 later */
            mem_free(borg_power_item[j]); /* externalize the 400 later */
        }
        mem_free(borg_required_item); /* externalize the 400 later */
        mem_free(borg_power_item); /* externalize the 400 later */
        mem_free(borg_has);
        for (j = 0; j < 1000; j++)
        {
            if (formula[j])
            {
                mem_free(formula[j]);
                formula[j] = 0;
            }
        }
        mem_free(n_req);
        mem_free(n_pwr);

        borg_required_item = mem_zalloc(MAX_CLASSES * sizeof(req_item*));
        n_req = mem_zalloc(MAX_CLASSES * sizeof(int));
        borg_power_item = mem_zalloc(MAX_CLASSES * sizeof(power_item*));
        n_pwr = mem_zalloc(MAX_CLASSES * sizeof(int));

        init_borg_txt_file();
        borg_note("# Ready...");
        break;
    }
    /* Command: Activate */
    case 'z':
    case 'Z':
    {
        /* make sure the important game options are set correctly */
        borg_reinit_options();

        /* Activate */
        borg_active = true;

        /* Reset cancel */
        borg_cancel = false;

        /* Step forever */
        borg_step = 0;

        /* need to check all stats */
        for (int i = 0; i < STAT_MAX; i++)
            my_need_stat_check[i] = true;

        /* Allowable Cheat -- Obtain "recall" flag */
        goal_recalling = player->word_recall * 1000;

        /* Allowable Cheat -- Obtain "prot_from_evil" flag */
        borg_prot_from_evil = (player->timed[TMD_PROTEVIL] ? true : false);
        /* Allowable Cheat -- Obtain "speed" flag */
        borg_speed = (player->timed[TMD_FAST] ? true : false);
        /* Allowable Cheat -- Obtain "resist" flags */
        borg_skill[BI_TRACID] = (player->timed[TMD_OPP_ACID] ? true : false);
        borg_skill[BI_TRELEC] = (player->timed[TMD_OPP_ELEC] ? true : false);
        borg_skill[BI_TRFIRE] = (player->timed[TMD_OPP_FIRE] ? true : false);
        borg_skill[BI_TRCOLD] = (player->timed[TMD_OPP_COLD] ? true : false);
        borg_skill[BI_TRPOIS] = (player->timed[TMD_OPP_POIS] ? true : false);
        borg_bless = (player->timed[TMD_BLESSED] ? true : false);
        borg_shield = (player->timed[TMD_SHIELD] ? true : false);
        borg_hero = (player->timed[TMD_HERO] ? true : false);
        borg_fastcast = (player->timed[TMD_FASTCAST] ? true : false);
        borg_berserk = (player->timed[TMD_SHERO] ? true : false);
        if (player->timed[TMD_SINVIS]) borg_see_inv = 10000;

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
    case 'U':
    {
        /* make sure the important game options are set correctly */
        borg_reinit_options();

        /* Activate */
        borg_active = true;

        /* Immediate cancel */
        borg_cancel = true;

        /* Step forever */
        borg_step = 0;

        /* Allowable Cheat -- Obtain "recall" flag */
        goal_recalling = player->word_recall * 1000;

        /* Allowable Cheat -- Obtain "prot_from_evil" flag */
        borg_prot_from_evil = (player->timed[TMD_PROTEVIL] ? true : false);
        /* Allowable Cheat -- Obtain "speed" flag */
        borg_speed = (player->timed[TMD_FAST] ? true : false);
        /* Allowable Cheat -- Obtain "resist" flags */
        borg_skill[BI_TRACID] = (player->timed[TMD_OPP_ACID] ? true : false);
        borg_skill[BI_TRELEC] = (player->timed[TMD_OPP_ELEC] ? true : false);
        borg_skill[BI_TRFIRE] = (player->timed[TMD_OPP_FIRE] ? true : false);
        borg_skill[BI_TRCOLD] = (player->timed[TMD_OPP_COLD] ? true : false);
        borg_skill[BI_TRPOIS] = (player->timed[TMD_OPP_POIS] ? true : false);
        borg_bless = (player->timed[TMD_BLESSED] ? true : false);
        borg_shield = (player->timed[TMD_SHIELD] ? true : false);
        borg_fastcast = (player->timed[TMD_FASTCAST] ? true : false);
        borg_hero = (player->timed[TMD_HERO] ? true : false);
        borg_berserk = (player->timed[TMD_SHERO] ? true : false);
        if (player->timed[TMD_SINVIS]) borg_see_inv = 10000;

        /* Message */
        borg_note("# Installing keypress hook");

        /* Activate the key stealer */
        inkey_hack = borg_inkey_hack;

        break;
    }


    /* Command: Step */
    case 'x':
    case 'X':
    {
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

        /* need to check all stats */
        for (int i = 0; i < STAT_MAX; i++)
            my_need_stat_check[i] = true;

        /* Allowable Cheat -- Obtain "prot_from_evil" flag */
        borg_prot_from_evil = (player->timed[TMD_PROTEVIL] ? true : false);
        /* Allowable Cheat -- Obtain "speed" flag */
        borg_speed = (player->timed[TMD_FAST] ? true : false);
        /* Allowable Cheat -- Obtain "resist" flags */
        borg_skill[BI_TRACID] = (player->timed[TMD_OPP_ACID] ? true : false);
        borg_skill[BI_TRELEC] = (player->timed[TMD_OPP_ELEC] ? true : false);
        borg_skill[BI_TRFIRE] = (player->timed[TMD_OPP_FIRE] ? true : false);
        borg_skill[BI_TRCOLD] = (player->timed[TMD_OPP_COLD] ? true : false);
        borg_skill[BI_TRPOIS] = (player->timed[TMD_OPP_POIS] ? true : false);
        borg_bless = (player->timed[TMD_BLESSED] ? true : false);
        borg_shield = (player->timed[TMD_SHIELD] ? true : false);
        borg_hero = (player->timed[TMD_HERO] ? true : false);
        borg_berserk = (player->timed[TMD_SHERO] ? true : false);
        if (player->timed[TMD_SINVIS]) borg_see_inv = 10000;

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
    case 'F':
    {
        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Toggle Flag: (m/d/s/f/g) ", &cmd)) return;

        switch (cmd)
        {
            /* Give borg thought messages in window */
        case 'm':
        case 'M':
        {
            break;
        }

        /* Give borg the ability to use graphics ----broken */
        case 'g':
        case 'G':
        {
            borg_graphics = !borg_graphics;
            msg("Borg -- borg_graphics is now %d.",
                borg_graphics);
            break;
        }

        /* Dump savefile at each death */
        case 'd':
        case 'D':
        {
            borg_flag_dump = !borg_flag_dump;
            msg("Borg -- borg_flag_dump is now %d.",
                borg_flag_dump);
            break;
        }

        /* Dump savefile at each level */
        case 's':
        case 'S':
        {
            borg_flag_save = !borg_flag_save;
            msg("Borg -- borg_flag_save is now %d.",
                borg_flag_save);
            break;
        }

        /* clear 'fear' levels */
        case 'f':
        case 'F':
        {
            msg("Command No Longer Usefull");
            break;
        }
        }
        break;
    }



    /* Command: toggle "cheat" flags */
    case 'c':
    {
        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Toggle Cheat: (d)", &cmd))
            return;

        switch (cmd)
        {
        case 'd':
        case 'D':
        {
            borg_cheat_death = !borg_cheat_death;
            msg("Borg -- borg_cheat_death is now %d.",
                borg_cheat_death);
            break;
        }
        }
        break;

    }

    /* List the Nasties on the level */
    case 'C':
    {
        int i;

        /* Log Header */
        borg_note("Borg Nasties Count");

        /* Find the numerous nasty in order of nastiness */
        for (i = 0; i < borg_nasties_num; i++)
        {
            borg_note(format("Nasty: [%c] Count: %d, limited: %d", borg_nasties[i],
                borg_nasties_count[i], borg_nasties_limit[i]));
        }

        /* Done */
        msg("Borg Nasty Dump Complete.  Examine Log.");
        break;
    }

    /* Activate a search string */
    case 's':
    case 'S':
    {
        /* Get the new search string (or cancel the matching) */
        if (!get_string("Borg Match String: ", borg_match, 70))
        {
            /* Cancel it */
            my_strcpy(borg_match, "", sizeof(borg_match));

            /* Message */
            msg("Borg Match String de-activated.");
        }
        break;
    }

    /* Command: check Grid "feature" flags */
    case 'g':
    {
        int x, y;

        uint16_t low, high = 0;
        bool trap = false;
        bool glyph = false;

        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Show grids: ", &cmd)) return;

        /* Extract a flag */
        switch (cmd)
        {
        case '0': low = high = 1 << 0; break;
        case '1': low = high = 1 << 1; break;
        case '2': low = high = 1 << 2; break;
        case '3': low = high = 1 << 3; break;
        case '4': low = high = 1 << 4; break;
        case '5': low = high = 1 << 5; break;
        case '6': low = high = 1 << 6; break;
        case '7': low = high = 1 << 7; break;

        case '.': low = high = FEAT_FLOOR; break;
        case ' ': low = high = FEAT_NONE; break;
        case ';': low = high = -1; glyph = true;  break;
        case ',': low = high = FEAT_OPEN; break;
        case 'x': low = high = FEAT_BROKEN; break;
        case '<': low = high = FEAT_LESS; break;
        case '>': low = high = FEAT_MORE; break;
        case '@': low = FEAT_STORE_GENERAL;
            high = FEAT_HOME;
            break;
        case '^': low = high = -1; glyph = true;  break;
        case '+': low = FEAT_CLOSED;
            high = FEAT_CLOSED; break;
        case 's': low = high = FEAT_SECRET; break;
        case ':': low = high = FEAT_RUBBLE; break;
        case 'm': low = high = FEAT_MAGMA; break;
        case 'q': low = high = FEAT_QUARTZ; break;
        case 'k': low = high = FEAT_MAGMA_K; break;
        case '&': low = high = FEAT_QUARTZ_K; break;
        case 'w': low = FEAT_GRANITE;
            high = FEAT_GRANITE;
            break;
        case 'p': low = FEAT_PERM;
            high = FEAT_PERM;
            break;

        default: low = high = 0x00; break;
        }

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++)
        {
            for (x = 1; x <= AUTO_MAX_X - 1; x++)
            {
                uint8_t a = COLOUR_RED;

                borg_grid* ag = &borg_grids[y][x];

                /* show only those grids */
                if (!trap && !glyph)
                {
                    if (!(ag->feat >= low && ag->feat <= high)) continue;
                }
                else
                {
                    if (!((ag->glyph && glyph) || (ag->trap && trap))) continue;
                }

                /* Color */
                if (borg_cave_floor_bold(y, x)) a = COLOUR_YELLOW;

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
    case 'G':
    {
        int y = 1;
        int x = 1;

        uint16_t mask;

        mask = borg_grids[y][x].feat;

        struct loc l;

        target_get(&l);
        y = l.y;
        x = l.x;

        uint8_t feat = square(cave, loc(c_x, c_y))->feat;

        borg_note(format("Borg's Feat for grid (%d, %d) is %d, game Feat is %d", y, x, mask, feat));
        prt_map();
        break;
    }

    /* Command: check "info" flags */
    case 'i':
    {
        int x, y;

        uint16_t mask;

        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Show grids: ", &cmd)) return;

        /* Extract a flag */
        switch (cmd)
        {
        case '0': mask = 1 << 0; break; /* Mark */
        case '1': mask = 1 << 1; break; /* Glow */
        case '2': mask = 1 << 2; break; /* Dark */
        case '3': mask = 1 << 3; break; /* Okay */
        case '4': mask = 1 << 4; break; /* Lite */
        case '5': mask = 1 << 5; break; /* View */
        case '6': mask = 1 << 6; break; /* Temp */
        case '7': mask = 1 << 7; break; /* Xtra */

        case 'm': mask = BORG_MARK; break;
        case 'g': mask = BORG_GLOW; break;
        case 'd': mask = BORG_DARK; break;
        case 'o': mask = BORG_OKAY; break;
        case 'l': mask = BORG_LIGHT; break;
        case 'v': mask = BORG_VIEW; break;
        case 't': mask = BORG_TEMP; break;
        case 'x': mask = BORG_XTRA; break;

        default: mask = 0x000; break;
        }

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++)
        {
            for (x = 1; x <= AUTO_MAX_X - 1; x++)
            {
                uint8_t a = COLOUR_RED;

                borg_grid* ag = &borg_grids[y][x];

                /* Given mask, show only those grids */
                if (mask && !(ag->info & mask)) continue;

                /* Given no mask, show unknown grids */
                if (!mask && (ag->info & BORG_MARK)) continue;

                /* Color */
                if (borg_cave_floor_bold(y, x)) a = COLOUR_YELLOW;

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
    case 'I':
    {
        int i;
        int y = 1;
        int x = 1;
        struct loc l;

        target_get(&l);
        y = l.y;
        x = l.x;

        if (borg_grids[y][x].info & BORG_MARK) msg("Borg Info for grid (%d, %d) is MARK", y, x);
        if (borg_grids[y][x].info & BORG_GLOW) msg("Borg Info for grid (%d, %d) is GLOW", y, x);
        if (borg_grids[y][x].info & BORG_DARK) msg("Borg Info for grid (%d, %d) is DARK", y, x);
        if (borg_grids[y][x].info & BORG_OKAY)	msg("Borg Info for grid (%d, %d) is OKAY", y, x);
        if (borg_grids[y][x].info & BORG_LIGHT)	msg("Borg Info for grid (%d, %d) is LITE", y, x);
        if (borg_grids[y][x].info & BORG_VIEW)	msg("Borg Info for grid (%d, %d) is VIEW", y, x);
        if (borg_grids[y][x].info & BORG_TEMP)	msg("Borg Info for grid (%d, %d) is TEMP", y, x);
        if (borg_grids[y][x].info & BORG_XTRA)	msg("Borg Info for grid (%d, %d) is XTRA", y, x);

        for (i = 0; i < SQUARE_MAX; i++)
            if (sqinfo_has(square(cave, l)->info, i))
                msg(format("Sys Info for grid (%d, %d) is %d", y, x, i));
        prt_map();
        break;
    }

    /* Command: check "avoidances" */
    case 'a':
    case 'A':
    {
        int x, y, p;

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++)
        {
            for (x = 1; x <= AUTO_MAX_X - 1; x++)
            {
                uint8_t a = COLOUR_RED;

                /* Obtain danger */
                p = borg_danger(y, x, 1, true, false);

                /* Skip non-avoidances */
                if (p < avoidance / 10) continue;

                /* Use colors for less painful */
                if (p < avoidance / 2) a = COLOUR_ORANGE;
                if (p < avoidance / 4) a = COLOUR_YELLOW;
                if (p < avoidance / 6) a = COLOUR_GREEN;
                if (p < avoidance / 8) a = COLOUR_BLUE;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("(%d,%d of %d,%d) Avoidance value %d.", c_y, c_x, Term->offset_y / borg_panel_hgt(), Term->offset_x / borg_panel_wid(), avoidance);
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }


    /* Command: check previous steps */
    case 'y':
    {
        int i;

        /* Scan map */
        uint8_t a = COLOUR_RED;
        /* Check for an existing step */
        for (i = 0; i < track_step.num; i++)
        {
            /* Display */
            print_rel('*', a, track_step.y[track_step.num - i], track_step.x[track_step.num - i]);
            msg("(-%d) Steps noted %d,%d", i, track_step.y[track_step.num - i], track_step.x[track_step.num - i]);
            event_signal(EVENT_MESSAGE_FLUSH);
            print_rel('*', COLOUR_ORANGE, track_step.y[track_step.num - i], track_step.x[track_step.num - i]);
        }
        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: show "monsters" */
    case 'k':
    case 'K':
    {
        int i, n = 0;

        /* Scan the monsters */
        for (i = 1; i < borg_kills_nxt; i++)
        {
            borg_kill* kill = &borg_kills[i];

            /* Still alive */
            if (kill->r_idx)
            {
                int x = kill->x;
                int y = kill->y;

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
    case 'T':
    {
        int i, n = 0;

        /* Scan the objects */
        for (i = 1; i < borg_takes_nxt; i++)
        {
            borg_take* take = &borg_takes[i];

            /* Still alive */
            if (take->kind)
            {
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
    case '%':
    {
        int x, y;
        int n_x;
        int n_y;
        struct loc l;

        /* Determine "path" */
        n_x = player->grid.x;
        n_y = player->grid.y;
        target_get(&l);
        x = l.x;
        y = l.y;

        /* Borg's pathway */
        while (1)
        {

            /* Display */
            print_rel('*', COLOUR_RED, n_y, n_x);

            if (n_x == x && n_y == y) break;

            /* Calculate the new location */
            mmove2(&n_y, &n_x, player->grid.y, player->grid.x, y, x);

        }

        msg("Borg's Targetting Path");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Determine "path" */
        n_x = player->grid.x;
        n_y = player->grid.y;
        x = l.x;
        y = l.y;

        /* Real LOS */
        project(source_player(), 0, loc(x, y), 1, PROJ_MISSILE, PROJECT_BEAM, 0, 0, NULL);

        msg("Actual Targetting Path");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();

        break;
    }
    /* Display the intended path to the flow */
    case '^':
    {
        int x, y;
        int o;
        int false_y, false_x;

        false_y = c_y;
        false_x = c_x;

        /* Continue */
        for (o = 0; o < 250; o++)
        {
            int b_n = 0;

            int i, b_i = -1;

            int c, b_c;


            /* Flow cost of current grid */
            b_c = borg_data_flow->data[c_y][c_x] * 10;

            /* Prevent loops */
            b_c = b_c - 5;

            /* Look around */
            for (i = 0; i < 8; i++)
            {
                /* Grid in that direction */
                x = false_x + ddx_ddd[i];
                y = false_y + ddy_ddd[i];

                /* Flow cost at that grid */
                c = borg_data_flow->data[y][x] * 10;

                /* Never backtrack */
                if (c > b_c) continue;

                /* Notice new best value */
                if (c < b_c) b_n = 0;

                /* Apply the randomizer to equivalent values */
                if ((++b_n >= 2) && (randint0(b_n) != 0)) continue;

                /* Track it */
                b_i = i; b_c = c;
            }

            /* Try it */
            if (b_i >= 0)
            {
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
    case '#':
    {
        int n;
        struct loc l;

        target_get(&l);

        /* Turns */
        n = get_quantity("Quantity: ", 10);

        /* Danger of grid */
        msg("Danger(%d,%d,%d) is %d",
            l.x, l.y, n,
            borg_danger(l.y, l.x, n, true, false));
        break;
    }

    /* Command: Regional Fear Info*/
    case '_':
    {
        int x, y, p;

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y - 1; y++)
        {
            for (x = 1; x <= AUTO_MAX_X - 1; x++)
            {
                uint8_t a = COLOUR_RED;

                /* Obtain danger */
                p = borg_fear_region[y / 11][x / 11];

                /* Skip non-fears */
                if (p < avoidance / 10) continue;

                /* Use colors = less painful */
                if (p < avoidance / 2) a = COLOUR_ORANGE;
                if (p < avoidance / 4) a = COLOUR_YELLOW;
                if (p < avoidance / 6) a = COLOUR_GREEN;
                if (p < avoidance / 8) a = COLOUR_BLUE;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("(%d,%d of %d,%d) Regional Fear.", c_y, c_x, Term->offset_y / borg_panel_hgt(), Term->offset_x / borg_panel_wid());
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();

        /* Scan map */
        for (y = 1; y <= AUTO_MAX_Y; y++)
        {
            for (x = 1; x <= AUTO_MAX_X; x++)
            {
                uint8_t a = COLOUR_BLUE;

                /* Obtain danger */
                p = borg_fear_monsters[y][x];

                /* Skip non-fears */
                if (p <= 0) continue;

                /* Color Defines */
                if (p == 1)  a = COLOUR_L_BLUE;

                /* Color Defines */
                if (p < avoidance / 20 &&
                    p > 1) a = COLOUR_BLUE;

                /* Color Defines */
                if (p < avoidance / 10 &&
                    p > avoidance / 20) a = COLOUR_GREEN;

                /* Color Defines */
                if (p < avoidance / 4 &&
                    p > avoidance / 10) a = COLOUR_YELLOW;

                /* Color Defines */
                if (p < avoidance / 2 &&
                    p > avoidance / 4) a = COLOUR_ORANGE;

                /* Color Defines */
                if (p > avoidance / 2)  a = COLOUR_RED;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("(%d,%d of %d,%d) Monster Fear.", c_y, c_x, Term->offset_y / borg_panel_hgt(), Term->offset_x / borg_panel_wid());
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Redraw map */
        prt_map();
        break;
    }

    /* Command: debug -- Power */
    case 'p':
    case 'P':
    {
        int32_t p;

        /* Examine the screen */
        borg_update_frame();

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
    case '!':
    {
        int32_t time = borg_t - borg_began;
        msg("time: (%d) ", time);
        time = (borg_time_town + (borg_t - borg_began));
        msg("; from town (%d)", time);
        msg("; on this panel (%d)", time_this_panel);
        msg("; need inviso (%d)", need_see_inviso);
        break;
    }

    /* Command: LOS */
    case '@':
    {
        int x, y;

        /* Scan map */
        for (y = w_y; y < w_y + SCREEN_HGT; y++)
        {
            for (x = w_x; x < w_x + SCREEN_WID; x++)
            {
                uint8_t a = COLOUR_RED;

                /* Obtain danger */
                if (!borg_los(c_y, c_x, y, x)) continue;

                /* Display */
                print_rel('*', a, y, x);
            }
        }

        /* Get keypress */
        msg("Borg has Projectable to these places.");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Scan map */
        for (y = w_y; y < w_y + SCREEN_HGT; y++)
        {
            for (x = w_x; x < w_x + SCREEN_WID; x++)
            {
                uint8_t a = COLOUR_YELLOW;

                if (!square_in_bounds(cave, loc(x, y)))
                    continue;

                /* Obtain danger */
                if (!borg_projectable_dark(c_y, c_x, y, x)) continue;

                /* Display */
                print_rel('*', a, y, x);
            }
        }
        msg("Borg has Projectable Dark to these places.");
        event_signal(EVENT_MESSAGE_FLUSH);

        /* Scan map */
        for (y = w_y; y < w_y + SCREEN_HGT; y++)
        {
            for (x = w_x; x < w_x + SCREEN_WID; x++)
            {
                uint8_t a = COLOUR_GREEN;

                /* Obtain danger */
                if (!borg_los(c_y, c_x, y, x)) continue;

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
    case '1':
    {
        int new_borg_skill;
        /* Get the new max depth */
        new_borg_skill = get_quantity("Enter new Max Depth: ", z_info->max_depth - 1);

        /* Allow user abort */
        if (new_borg_skill >= 0)
        {
            player->max_depth = new_borg_skill;
            borg_skill[BI_MAXDEPTH] = new_borg_skill;
        }

        break;
    }
    /*  command: debug -- allow borg to stop */
    case 'q':
    {
        int new_borg_stop_dlevel = 127;
        int new_borg_stop_clevel = 51;

        /* Get the new max depth */
        new_borg_stop_dlevel = get_quantity("Enter new auto-stop dlevel: ", z_info->max_depth - 1);
        new_borg_stop_clevel = get_quantity("Enter new auto-stop clevel: ", 51);
        get_com("Stop when Morgoth Dies? (y or n)? ", &cmd);

        borg_cfg[BORG_STOP_DLEVEL] = new_borg_stop_dlevel;
        borg_cfg[BORG_STOP_CLEVEL] = new_borg_stop_clevel;
        if (cmd == 'n' || cmd == 'N') borg_cfg[BORG_STOP_KING] = false;

        break;
    }

    /* command: money Scum-- allow borg to stop when he gets a certain amount of money*/
    case 'm':
    {
        int new_borg_money_scum_amount = 0;

        /* report current status */
        msg("money Scumming for %d, I need %d more.", borg_cfg[BORG_MONEY_SCUM_AMOUNT],
            borg_cfg[BORG_MONEY_SCUM_AMOUNT] - borg_gold);

        /* Get the new amount */
        new_borg_money_scum_amount = get_quantity("Enter new dollar amount for money scumming (0 for no scumming):", INT_MAX);

        borg_cfg[BORG_MONEY_SCUM_AMOUNT] = new_borg_money_scum_amount;

        break;
    }
    /* Command:  HACK debug -- preparation for level */
    case '2':
    {
        int i = 0;

        /* Extract some "hidden" variables */
        borg_cheat_equip();
        borg_cheat_inven();

        /* Examine the screen */
        borg_update_frame();
        borg_update();


        /* Examine the inventory */
        borg_object_fully_id();
        borg_notice(true);
        borg_notice_home(NULL, false);

        /* Dump prep codes */
        for (i = 1; i <= 101; i++)
        {
            /* Dump fear code*/
            if ((char*)NULL != borg_prepared(i)) break;
        }
        msg("Max Level: %d  Prep'd For: %d  Reason: %s", borg_skill[BI_MAXDEPTH], i - 1, borg_prepared(i));
        if (borg_ready_morgoth == 1)
        {
            msg("You are ready for the big fight!!");
        }
        else if (borg_ready_morgoth == 0)
        {
            msg("You are NOT ready for the big fight!!");
        }
        else if (borg_ready_morgoth == -1)
        {
            msg("No readiness check done.");
        }

        break;
    }
    /* Command: debug -- stat information */
    case '3':
    {

        int i;
        for (i = 0; i < STAT_MAX; i++)
        {
            borg_note(format("stat # %d, is: %d", i, my_stat_cur[i]));
        }
#if 0
        artifact_type* a_ptr;

        int i;
        for (i = 0; i < z_info->a_max; i++)
        {
            a_ptr = &a_info[i];
            borg_note(format("(%d) %d, %d (act:%d)", i, a_ptr->name, a_ptr->text, a_ptr->activation));
        }
#endif
        break;
    }

    /* Command: List the swap weapon and armour */
    case 'w':
    case 'W':
    {
        borg_item* item;


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
        borg_update_frame();

        /* note the swap items */
        if (weapon_swap)
        {
            item = &borg_items[weapon_swap - 1];
            msg("Swap Weapon:  %s, value= %d", item->desc, weapon_swap_value);
        }
        else
        {
            msg("Swap Weapon:  NONE");
        }

        if (armour_swap)
        {
            item = &borg_items[armour_swap - 1];
            msg("Swap Armour:  %s, value= %d", item->desc, armour_swap_value);
        }
        else
        {
            msg("Swap Armour:  NONE");
        }
        break;
    }
    case 'd':
    {
        int ii = 1;

        /* Save the screen */
        Term_save();

        /* Dump the spells */
        if (player->class->magic.total_spells)
        {

            int i;

            for (i = 0; i < player->class->magic.total_spells; i++)
            {
                /* Clear the screen */
                Term_clear();

                ii = 2;
                Term_putstr(1, ii, -1, COLOUR_WHITE, "[ Spells ].");
                borg_magic* as = &borg_magics[i];
                int failpercent = 0;

                if (as->level < 99)
                {
                    const char *legal = (borg_spell_legal(as->spell_enum) ? "legal" : "Not Legal ");
                    failpercent = (borg_spell_fail_rate(as->spell_enum));

                    Term_putstr(1, ii++, -1, COLOUR_WHITE, format("%s, %s, attempted %d times, fail rate:%d", as->name, legal, as->times, failpercent));
                }
                get_com("Exam spell books.  Press any key for next book.", &cmd);
            } /* dumps */
        } /* spells */

       /* Restore the screen */
        Term_load();

        /* Done */
        return;
    }

    /* dump borg 'has' information */
    case 'h':
    case 'H':
    {
        int item, to;

        /* Get a "Borg command", or abort */
        if (!get_com("Dynamic Borg Has What: ((a)ny/(i)nv/(w)orn/a(r)tifact/(s)kill) ", &cmd)) return;

        switch (cmd)
        {
        case 'a':
        case 'A':
        item = 0;
        to = z_info->k_max;
        break;
        case 'i':
        case 'I':
        item = 0;
        to = z_info->pack_size;
        break;
        case 'w':
        case 'W':
        item = INVEN_WIELD;
        to = QUIVER_END;
        break;
        case 'r':
        case 'R':
        item = 0;
        to = QUIVER_END;
        break;
        default:
        item = 0;
        to = BI_MAX;
        break;
        }
        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Examine the screen */
        borg_update_frame();

        /* Examine the screen */
        borg_update();


        /* Examine the inventory */
        borg_object_fully_id();
        borg_notice(true);
        borg_notice_home(NULL, false);
        for (; item < to; item++)
        {
            switch (cmd)
            {
            case 'a':
            case 'A':
            if (borg_has[item])
            {
                borg_note(format("Item-Kind:%03d name=%s value= %d.", item, 
                    k_info[item].name, borg_has[item]));
            }
            break;
            case 'i':
            case 'I':
            if (borg_items[item].iqty)
            {
                borg_note(format("Item-Invn:%03d desc= %s qty %d.", item,
                    borg_items[item].desc, borg_items[item].iqty));
            }
            break;
            case 'w':
            case 'W':
            if (borg_items[item].iqty)
            {
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
            default:
            {
                borg_note(format("skill %d (%s) value= %d.", item, 
                    prefix_pref[item], borg_skill[item]));
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
    case 'V':
    {
        msg("APWBorg Version: %s", borg_engine_date);
        break;
    }
    /* Command: Display all known info on item */
    case 'o':
    case 'O':
    {
        int n = 0;

        struct object* item2;

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
        borg_update_frame();

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
    case 'R':
    {
        /* Confirm it */
        get_com("Are you sure you want to Respawn this borg? (y or n)? ", &cmd);

        if (cmd == 'y' || cmd == 'Y')
        {
            resurrect_borg();
        }
        break;
    }

    /* Command: Restock the Stores */
    case 'r':
    {
        /* Confirm it */
        get_com("Are you sure you want to Restock the stores? (y or n)? ", &cmd);

        if (cmd == 'y' || cmd == 'Y')
        {
            /* Message */
            msg("Updating Shops... currently not allowed");
#if false
            msg("Updating Shops...");
            // need to change base code to make store_maint accessable .. trying not to change that too much right now.  
            // this functionality seems a bit bogus anyway !FIX !TODO !AJG
                            /* Maintain each shop (except home) */
            for (n = 0; n < MAX_STORES; n++)
            {
                /* Skip the home */
                if (n == STORE_HOME) continue;

                /* Maintain */
                store_maint(&stores[n]);
            }
#endif
        }

        break;
    }

    case ';':
    {
        int glyph_check;

        uint8_t a = COLOUR_RED;

        for (glyph_check = 0; glyph_check < track_glyph.num; glyph_check++)
        {
            /* Display */
            print_rel('*', a, track_glyph.y[glyph_check], track_glyph.x[glyph_check]);
            msg("Borg has Glyph (%d)noted.", glyph_check);
            event_signal(EVENT_MESSAGE_FLUSH);
        }

        /* Get keypress */
    }
    break;
    /* Oops */
    default:
    {
        /* Message */
        msg("That is not a legal Borg command.");
        break;
    }
    }
}



#ifdef MACINTOSH
static int HACK = 0;
#endif

#endif /* ALLOW_BORG */
