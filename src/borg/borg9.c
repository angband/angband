/* File: borg9.c */

/* Purpose: Highest level functions for the Borg -BEN- */
#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"
#include "target.h"
#include "spells.h"
#include "object/inventory.h"

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
extern bool auto_play;
extern bool keep_playing;
#endif /* bablos */
bool borg_cheat_death;

static s16b stat_use[6];

/*
 * This file implements the "Ben Borg", an "Automatic Angband Player".
 *
 * This version of the "Ben Borg" is designed for use with Angband 2.7.9v6.
 *
 * Use of the "Ben Borg" requires re-compilation with ALLOW_BORG defined,
 * and with the various "borg*.c" files linked into the executable.
 *
 * Note that you can only use the Borg if your character has been marked
 * as a "Borg User".  You can do this, if necessary, by responding "y"
 * when asked if you really want to use the Borg.  This will (normally)
 * result in your character being inelligible for the high score list.
 *
 * The "do_cmd_borg()" function, called when the user hits "^Z", allows
 * the user to interact with the Borg.  You do so by typing "Borg Commands",
 * including 'z' to activate (or re-activate), 'K' to show monsters, 'T' to
 * show objects, 'd' to toggle "demo mode", 'f' to open/shut the "log file",
 * 'i' to display internal flags, etc.  See "do_cmd_borg()" for more info.
 *
 * The first time you enter a Borg command, the Borg is initialized.  This
 * consists of three major steps, and requires at least 400K of free memory,
 * if the memory is not available, the game may abort.
 *
 * (1) The various "borg" modules are initialized.
 *
 * (2) Some important "state" information is extracted, including the level
 *     and race/class of the player, and some more initialization is done.
 *
 * (3) Some "historical" information (killed uniques, maximum dungeon depth)
 *     is "stolen" from the game.
 *
 * When the Ben Borg is "activated", it uses the "Term_inkey_hook" to steal
 * control from the user.  Later, if it detects any input from the real user,
 * it gracefully relinquishes control by clearing the "Term_inkey_hook" after
 * any pending key-sequences are complete.
 *
 * The Borg will abort if it detects any "errors", or if it detects any
 * "situations" such as "death", or if it detects any "panic" situations,
 * such as "imminent death", if the appropriate flags are set.
 *
 * The Ben Borg is only supposed to "know" what is visible on the screen,
 * which it learns by using the "term.c" screen access function "Term_what()",
 * the cursor location function "Term_locate()", and the cursor visibility
 * extraction function "Term_get_cursor()".
 *
 * The Ben Borg is only supposed to "send" keypresses when the "Term_inkey()"
 * function asks for a keypress, which is accomplished by using a special
 * function hook in the "z-term.c" file, which allows the Borg to "steal"
 * control from the "Term_inkey()" and "Term_flush()" functions.  This
 * allows the Ben Borg to pretend to be a normal user.
 *
 * Note that if properly designed, the Ben Borg could be run as an external
 * process, which would actually examine the screen (or pseudo-terminal),
 * and send keypresses directly to the keyboard (or pseudo-terminal).  Thus
 * it should never access any "game variables", unless it is able to extract
 * those variables for itself by code duplication or complex interactions,
 * or, in certain situations, if those variables are not actually "required".
 *
 * Currently, the Ben Borg is a few steps away from being able to be run as
 * an external process, primarily in the "low level" details, such as knowing
 * when the game is ready for a keypress.  Also, the Ben Borg assumes that a
 * character has already been rolled, and maintains no state between saves,
 * which is partially offset by "cheating" to "acquire" the maximum dungeon
 * depth, without which equipment analysis will be faulty.
 *
 * The "theory" behind the Borg is that is should be able to run as a
 * separate process, playing Angband in a window just like a human, that
 * is, examining the screen for information, and sending keypresses to
 * the game.  The current Borg does not actually do this, because it would
 * be very slow and would not run except on Unix machines, but as far as
 * possible, I have attempted to make sure that the Borg *could* run that
 * way.  This involves "cheating" as little as possible, where "cheating"
 * means accessing information not available to a normal Angband player.
 * And whenever possible, this "cheating" should be optional, that is,
 * there should be software options to disable the cheating, and, if
 * necessary, to replace it with "complex" parsing of the screen.
 *
 * Thus, the Borg COULD be written as a separate process which runs Angband
 * in a pseudo-terminal and "examines" the "screen" and sends keypresses
 * directly (as with a terminal emulator), although it would then have
 * to explicitly "wait" to make sure that the game was completely done
 * sending information.
 *
 * The Borg is thus allowed to examine the screen directly (by efficient
 * direct access of the "Term->scr->a" and "Term->scr->c" arrays, which
 * could be replaced by calls to "Term_grab()"), and to access the cursor
 * location (via "Term_locate()") and visibility (via "Term_get_cursor()"),
 * and, as mentioned above, the Borg is allowed to send keypresses directly
 * to the game, and only when needed, using the "Term_inkey_hook" hook, and
 * uses the same hook to know when it should discard all pending keypresses.
 *
 * The Borg should not know when the game is ready for a keypress, and
 * should really do something nasty such as "pause" between turns for
 * some amount of time to ensure that the game is really waiting for
 * a keypress.
 *
 * Various other "cheats" (mostly optional) are described where they are
 * used, primarily in this file.
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
 * we can store every time-stamp in a 's16b', since we reset the clock to
 * 1000 on each new level, and we refuse to stay on any level longer than
 * 30000 turns, unless we are totally stuck, in which case we abort.
 *
 * Note that as of 2.7.9, the Borg can play any class, that is, he can make
 * "effective" use of at least a few spells/prayers, and is not "broken"
 * by low strength, blind-ness, hallucination, etc.  This does not, however,
 * mean that he plays all classes equally well, especially since he is so
 * dependant on a high strength for so many things.  The "demo" mode is useful
 * for many classes (especially Mage) since it allows the Borg to "die" a few
 * times, without being penalized.
 *
 * The Borg assumes that the "maximize" flag is off, and that the
 * "preserve" flag is on, since he cannot actually set those flags.
 * If the "maximize" flag is on, the Borg may not work correctly.
 * If the "preserve" flag is off, the Borg may miss artifacts.
 */


/*
 * We currently handle:
 *   Level changing (intentionally or accidentally)
 *   Embedded objects (gold) that must be extracted
 *   Ignore embedded objects if too "weak" to extract
 *   Traps (disarm), Doors (open/etc), Rubble (tunnel)
 *   Stores (enter via movement, exit via escape)
 *   Stores (limited commands, and different commands)
 *   Always deal with objects one at a time, not in piles
 *   Discard junk before trying to pick up more stuff
 *   Use "identify" to obtain knowledge and/or money
 *   Rely on "sensing" objects as much as possible
 *   Do not sell junk or worthless items to any shop
 *   Do not attempt to buy something without the cash
 *   Use the non-optimal stairs if stuck on a level
 *   Use "flow" code for all tasks for consistency
 *   Cancel all goals when major world changes occur
 *   Use the "danger" code to avoid potential death
 *   Use the "danger" code to avoid inconvenience
 *   Try to avoid danger (both actively and passively)
 *   Handle "Mace of Disruption", "Scythe of Slicing", etc
 *   Learn spells, and use them when appropriate
 *   Remember that studying prayers is slightly random
 *   Do not try to read scrolls when blind or confused
 *   Do not study/use spells/prayers when blind/confused
 *   Use spells/prayers at least once for the experience
 *   Attempt to heal when "confused", "blind", etc
 *   Attempt to fix "fear", "poison", "cuts", etc
 *   Analyze potential equipment in proper context
 *   Priests should avoid edged weapons (spell failure)
 *   Mages should avoid most gloves (lose mana)
 *   Non-warriors should avoid heavy armor (lose mana)
 *   Keep "best" ring on "tight" right finger in stores
 *   Remove items which do not contribute to total fitness
 *   Wear/Remove/Sell/Buy items in most optimal order
 *   Pursue optimal combination of available equipment
 *   Notice "failure" when using rods/staffs/artifacts
 *   Notice "failure" when attempting spells/prayers
 *   Attempt to correctly track terrain, objects, monsters
 *   Take account of "clear" and "multi-hued" monsters
 *   Take account of "flavored" (randomly colored) objects
 *   Handle similar objects/monsters (mushrooms, coins)
 *   Multi-hued/Clear monsters, and flavored objects
 *   Keep in mind that some monsters can move (quickly)
 *   Do not fire at monsters that may not actually exist
 *   Assume everything is an object until proven otherwise
 *   Parse messages to correct incorrect assumptions
 *   Search for secret doors after exploring the level
 *   React intelligently to changes in the wall structure
 *   Do not recalculate "flow" information unless required
 *   Collect monsters/objects/terrain not currently in view
 *   Keep in mind that word of recall is a delayed action
 *   Keep track of charging items (rods and artifacts)
 *   Be very careful not to access illegal locations!
 *   Do not rest next to dangerous (or immobile) monsters
 *   Recall into dungeon if prepared for resulting depth
 *   Do not attempt to destroy cursed ("terrible") artifacts
 *   Attempted destruction will yield "terrible" inscription
 *   Use "maximum" level and depth to prevent "thrashing"
 *   Use "maximum" hp's and sp's when checking "potentials"
 *   Attempt to recognize large groups of "disguised" monsters
 *   Beware of firing at a monster which is no longer present
 *   Stockpile items in the Home, and use those stockpiles
 *   Discounted spell-books (low level ones are ignored)
 *   Take items out of the home to sell them when no longer needed
 *   Trappers and Mimics (now treated as invisible monsters)
 *   Invisible monsters (induce "fear" of nearby regions)
 *   Fleeing monsters are "followed" down corridors and such
 *
 * We ignore:
 *   Long object descriptions may have clipped inscriptions
 *
 * We need to handle:
 *   Technically a room can have no exits, requiring digging
 *   Try to use a shovel/pick to help with tunnelling
 *   Conserve memory space (each grid byte costs about 15K)
 *   Conserve computation time (especially with the flow code)
 *   Becoming "afraid" (attacking takes a turn for no effect)
 *   Beware of firing missiles at a food ration under a mushroom
 *
 * We need to handle "loading" saved games:
 *   The "max_depth" value is lost if loaded in the town
 *   If we track "dead uniques" then this information is lost
 *   The "map" info, "flow" info, "tracking" info, etc is lost
 *   The contents of the shops (and the home) are lost
 *   We may be asked to "resume" a non-Borg character (icky)
 */


/*
 * Currently, the Borg "cheats" in a few situations...
 *
 * Cheats that are significant, and possibly unavoidable:
 *   Knowledge of when we are being asked for a keypress.
 *   Note that this could be avoided by LONG timeouts/delays
 *
 * Cheats "required" by implementation, but not signifant:
 *   Direct access to the "screen image" (parsing screen)
 *   Direct access to the "keypress queue" (sending keys)
 *   Direct access to the "cursor visibility" (game state)
 *
 * Cheats that could be avoided by simple (ugly) code:
 *   Direct modification of the "current options"
 *
 * Cheats that could be avoided by duplicating code:
 *   Use of the tables in "tables.c"
 *   Use of the arrays initialized in "init.c"
 *
 * Cheats that the Borg would love:
 *   Immunity to hallucination, blindness, confusion
 *   Unique attr/char codes for every monster and object
 *   Removal of the "mimic" and "trapper" monsters
 *   Removal of the "mushroom" and "gold" monsters
 */


/*
 * Stat advantages:
 *   High STR (attacks, to-dam, digging, weight limit)
 *   High DEX (attacks, to-hit, armor class)
 *   High CON (hitpoints, recovery)
 *   High WIS (better prayers, saving throws)
 *   High INT (better spells, disarming, device usage)
 *   High CHR (better item costs)
 *
 * Class advantages:
 *   Warrior (good fighting, sensing)
 *   Mage (good spells)
 *   Priest (good prayers, fighting)
 *   Ranger (some spells, fighting)
 *   Rogue (some spells, fighting, sensing)
 *   Paladin (prayers, fighting, sensing)
 *
 * Race advantages:
 *   Gnome (free action)
 *   Dwarf (resist blindness)
 *   High elf (see invisible)
 *   Non-human (infravision)
 */




/*
 * Some variables
 */

static bool initialized;    /* Hack -- Initialized */


#ifndef BABLOS

void borg_log_death(void)
{
   char buf[1024];
   ang_file *borg_log_file;
   time_t death_time;

	/* Build path to location of the definition file */
   path_build(buf, 1024, ANGBAND_DIR_USER, "borg-log.txt");

   /* Hack -- drop permissions */
   safe_setuid_drop();

   /* Append to the file */
   borg_log_file = file_open(buf, MODE_APPEND, FTYPE_TEXT);

    /* Hack -- grab permissions */
   safe_setuid_grab();

   /* Failure */
   if (!borg_log_file) return;

   /* Get time of death */
   (void)time(&death_time);

   /* Save the date */
   strftime(buf, 80, "%Y/%m/%d %H:%M\n", localtime(&death_time));

   file_putf(borg_log_file, buf);

   file_putf(borg_log_file, "%s the %s %s, Level %d/%d\n",
		   op_ptr->full_name,
           p_ptr->race->name,
		   p_ptr->class->name,
           p_ptr->lev, p_ptr->max_lev);

   file_putf(borg_log_file, "Exp: %lu  Gold: %lu  Turn: %lu\n", (long)p_ptr->max_exp + (100 * p_ptr->max_depth), (long)p_ptr->au, (long)turn);
   file_putf(borg_log_file, "Killed on level: %d (max. %d) by %s\n", p_ptr->depth, p_ptr->max_depth, p_ptr->died_from);

   file_putf(borg_log_file, "Borg Compile Date: %s\n", borg_engine_date);

   file_putf(borg_log_file, "----------\n\n");

   file_close(borg_log_file);
}

#endif /* BABLOS */

void borg_log_death_data(void)
{
   char buf[1024];
   ang_file *borg_log_file;
   time_t death_time;

   path_build(buf, 1024, ANGBAND_DIR_USER, "borg.dat");

   /* Hack -- drop permissions */
   safe_setuid_drop();

   /* Append to the file */
   borg_log_file = file_open(buf, MODE_APPEND, FTYPE_TEXT);

    /* Hack -- grab permissions */
   safe_setuid_grab();

   /* Failure */
   if (!borg_log_file) return;

   /* Get time of death */
   (void)time(&death_time);

    /* dump stuff for easy import to database */
   file_putf(borg_log_file, "%s, %s, %s, %d, %d, %s\n",borg_engine_date, p_ptr->race->name,
   p_ptr->class->name, p_ptr->lev, p_ptr->depth, p_ptr->died_from);


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

    byte t_a;

    char buf[128];
    static char svSavefile[1024];
    static char svSavefile2[1024];
    static bool justSaved = FALSE;


	/* Fill up the borg_skill[] array */
	(void)borg_update_frame();

	/*** Process inventory/equipment ***/

    /* Cheat */
    if (borg_do_equip)
    {
        /* Only do it once */
        borg_do_equip = FALSE;

        /* Cheat the "equip" screen */
       borg_cheat_equip();

       /* Done */
       return (FALSE);
    }

    /* Cheat */
    if (borg_do_inven)
    {
        /* Only do it once */
        borg_do_inven = FALSE;

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Done */
        return (FALSE);
    }

    /* save now */
    if (borg_save && borg_save_game())
    {
        /* Log */
        borg_note("# Auto Save!");

        borg_save = FALSE;

        /* Create a scum file */
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(p_ptr->died_from, "starvation"))
        {
            memcpy(svSavefile, savefile, sizeof(savefile));
            /* Process the player name */
            for (i = 0; op_ptr->full_name[i]; i++)
            {
                char c = op_ptr->full_name[i];

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
            svSavefile2[i]  = 0;

             path_build(savefile, 1024, ANGBAND_DIR_USER, svSavefile2);


            justSaved = TRUE;
        }
        return (TRUE);
    }
    if (justSaved)
    {
        memcpy(savefile, svSavefile, sizeof(savefile));
        borg_save_game();
        justSaved = FALSE;
        return (TRUE);
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
        return (TRUE);
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
        return (TRUE);
    }

    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 6, &t_a, buf)) &&
		(streq(buf, "(Inven")) && borg_best_item != -1)
	{
		borg_keypress(I2A(borg_best_item));


        /* Leave this mode */
        borg_keypress(ESCAPE);
		borg_best_item = -1;

        /* Done */
        return (TRUE);
    }

    /*** Find books ***/

    /* Only if needed */
    if (borg_do_spell && (borg_do_spell_aux == 0))
    {
        /* Assume no books */
        for (i = 0; i < 9; i++) borg_book[i] = -1;

        /* Scan the pack */
        for (i = 0; i < INVEN_MAX_PACK; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip non-books */
            if (item->tval != p_ptr->class->spell_book) continue;

            /* Note book locations */
            borg_book[item->sval] = i;
        }
    }


    /*** Process books ***/
    /* Hack -- Warriors never browse */
    if (borg_class == CLASS_WARRIOR) borg_do_spell = FALSE;

    /* Hack -- Blind or Confused prevents browsing */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) borg_do_spell = FALSE;

    /* XXX XXX XXX Dark */

    /* Hack -- Stop doing spells when done */
    if (borg_do_spell_aux > 8) borg_do_spell = FALSE;

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
        return (FALSE);
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
        return (TRUE);
    }

	/* If king, maybe retire. */
    if (borg_skill[BI_KING])
    {
        /* Prepare to retire */
        if (borg_stop_king)
        {
#ifndef BABLOS
            borg_write_map(FALSE);
#endif /* bablos */
            borg_oops("retire");
        }
        /* Borg will be respawning */
        if (borg_respawn_winners)
        {
#ifndef BABLOS
            borg_write_map(FALSE);
#if 0
            /* Note the score */
            borg_enter_score();
#endif
            /* Write to log and borg.dat */
            borg_log_death();
            borg_log_death_data();

            /* respawn */
            resurrect_borg();
#endif /* bablos */
        }

    }

    /*** Handle stores ***/

    /* Hack -- Check for being in a store CHEAT*/
    if ((0 == borg_what_text(1, 3, 4, &t_a, buf)) &&
		(streq(buf, "Stor") ||
		 streq(buf, "Home")))
    {
        /* Cheat the store number */
		shop_num = (cave->feat[p_ptr->py][p_ptr->px] - FEAT_SHOP_HEAD);

        /* Clear the goal (the goal was probably going to a shop number) */
        goal = 0;

        /* Hack -- Reset food counter for money scumming */
        if (shop_num == 0) borg_food_onsale = 0;

        /* Hack -- Reset fuel counter for money scumming */
        if (shop_num == 0) borg_fuel_onsale = 0;

        /* Extract the current gold (unless in home) */
        borg_gold = (long)p_ptr->au;

        /* Cheat the store (or home) inventory (all pages) */
		borg_cheat_store();

        /* Recheck inventory */
        borg_do_inven = TRUE;

        /* Recheck equipment */
        borg_do_equip = TRUE;

        /* Recheck spells */
        borg_do_spell = TRUE;

        /* Restart spells */
        borg_do_spell_aux = 0;

        /* Examine the inventory */
        borg_notice(TRUE);

        /* Evaluate the current world */
        my_power = borg_power();

        /* Hack -- allow user abort */
        if (borg_cancel) return (TRUE);

		/* Do not allow a user key to interrupt the borg while in a store */
		borg_in_shop = TRUE;

        /* Think until done */
        return (borg_think_store());
    }


    /*** Determine panel ***/

    /* Hack -- cheat */
    w_y = Term->offset_y;
    w_x = Term->offset_x;

    /* Done */
    borg_do_panel = FALSE;

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
        return (TRUE);
    }

    /* Check panel */
    if (borg_do_panel)
    {
        /* Only do it once */
        borg_do_panel = FALSE;

        /* Enter "panel" mode */
        borg_keypress('L');

        /* Done */
        return (TRUE);
    }

    /*** Analyze the Frame ***/

    /* Analyze the frame */
    if (borg_do_frame)
    {
        /* Only once */
        borg_do_frame = FALSE;

        /* Analyze the "frame" */
        borg_update_frame();
    }

    /*** Re-activate Tests ***/

    /* Check equip again later */
    borg_do_equip = TRUE;

    /* Check inven again later */
    borg_do_inven = TRUE;

    /* Check panel again later */
    borg_do_panel = TRUE;

    /* Check frame again later */
    borg_do_frame = TRUE;

    /* Check spells again later */
    borg_do_spell = TRUE;

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
    borg_notice(TRUE);

	/* Evaluate the current world */
    my_power = borg_power();

    /* Hack -- allow user abort */
    if (borg_cancel) return (TRUE);

    /* Do something */
    return (borg_think_dungeon());
}



/*
 * Hack -- methods of hurting a monster (order not important).
 *
 * See "message_pain()" for details.
 */
static cptr suffix_pain[] =
{
    " is unharmed."
    " barely notices.",
    " flinches.",
    " squelches.",
    " quivers in pain.",
    " writhes about.",
    " writhes in agony.",
    " jerks limply.",

    " spawns!",
    " looks healthier.",
    " starts moving faster.",
    " starts moving slower.",

    " is unaffected!",
    " is immune.",
    " resists a lot.",
    " resists.",
    " resists somewhat.",

    " shrugs off the attack.",
    " snarls with pain.",
    " yelps in pain.",
    " howls in pain.",
    " howls in agony.",
    /* xxx */
    " yelps feebly.",

    " ignores the attack.",
    " grunts with pain.",
    " squeals in pain.",
    " shrieks in pain.",
    " shrieks in agony.",
    /* xxx */
    " cries out feebly.",

    /* xxx */
    /* xxx */
    " cries out in pain.",
    " screams in pain.",
    " screams in agony.",
    /* xxx */
    " cringes from the light!",
    " loses some skin!",

    " is hit hard.",

    NULL
};


/*
 * Hack -- methods of killing a monster (order not important).
 *
 * See "mon_take_hit()" for details.
 */
static cptr prefix_kill[] =
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
static cptr suffix_died[] =
{
    " dies.",
    " is destroyed.",
    " dissolves!",
    " shrivels away in the light!",
    NULL
};
static cptr suffix_blink[] =
{
    " disappears!",      /* from teleport other */
    " changes!",         /* from polymorph spell */
    " teleports away.",  /* RF6_TPORT */
    " blinks away.",                /* RF6_BLINK */
    NULL
};

/*
 * Hack -- methods of hitting the player (order not important).
 *
 * The "insult", "moan", and "begs you for money" messages are ignored.
 *
 * See "make_attack_normal()" for details.
 */
static cptr suffix_hit_by[] =
{
    " hits you.",
    " touches you.",
    " punches you.",
    " kicks you.",
    " claws you.",
    " bites you.",
    " stings you.",
    " butts you.",
    " crushes you.",
    " engulfs you.",
    " crawls on you.",
    " drools on you.",
    " spits on you.",
    " gazes at you.",
    " wails at you.",
    " releases spores at you.",
    NULL
};


/*
 * Hack -- methods of casting spells at the player (order important).
 *
 * See "make_attack_spell()" for details.
 */
static cptr suffix_spell[] =
{
    " makes a high pitched shriek.",        /* 0 RF4_SHRIEK */
    " tries to cast a spell, but fails.",   /* 1 RF4_FAILS */
    " does something.",                     /* 2 RF4_XXX3X4 */
    " does something.",                     /* 3 RF4_XXX4X4 */
    " fires an arrow.",                     /* 4 RF4_ARROW_1 */
    " fires an arrow!",                     /* 5 RF4_ARROW_2 */
    " fires a missile.",                    /* 6 RF4_ARROW_3 */
    " fires a missile!",                    /* 7 RF4_ARROW_4 */
    " breathes acid.",                      /* 8 RF4_BR_ACID */
    " breathes lightning.",                 /* 9 RF4_BR_ELEC */
    " breathes fire.",                      /*10 RF4_BR_FIRE */
    " breathes frost.",                     /*11 RF4_BR_COLD */
    " breathes gas.",                       /*12 RF4_BR_POIS */
    " breathes nether.",                    /*13 RF4_BR_NETH */
    " breathes light.",                     /*14 RF4_BR_LIGHT */
    " breathes darkness.",                  /*15 RF4_BR_DARK */
    " breathes confusion.",                 /*16 RF4_BR_CONF */
    " breathes sound.",                     /*17 RF4_BR_SOUN */
    " breathes chaos.",                     /*18 RF4_BR_CHAO */
    " breathes disenchantment.",            /*19 RF4_BR_DISE */
    " breathes nexus.",                     /*20 RF4_BR_NEXU */
    " breathes time.",                      /*21 RF4_BR_TIME */
    " breathes inertia.",                   /*22 RF4_BR_INER */
    " breathes gravity.",                   /*23 RF4_BR_GRAV */
    " breathes shards.",                    /*24 RF4_BR_SHAR */
    " breathes plasma.",                    /*25 RF4_BR_PLAS */
    " breathes force.",                     /*26 RF4_BR_WALL */
    " does something.",                     /*27 RF4_BR_MANA */
    " does something.",                     /*28 RF4_XXX5X4 */
    " does something.",                     /*29 RF4_XXX6X4 */
    " does something.",                     /*30 RF4_XXX7X4 */
    " hurls a boulder at you!",             /*31 RF4_BOULDER */
    " casts an acid ball.",                 /*32 RF5_BA_ACID */
    " casts a lightning ball.",             /*33 RF5_BA_ELEC */
    " casts a fire ball.",                  /*34 RF5_BA_FIRE */
    " casts a frost ball.",                 /*35 RF5_BA_COLD */
    " casts a stinking cloud.",             /*36 RF5_BA_POIS */
    " casts a nether ball.",                /*37 RF5_BA_NETH */
    " gestures fluidly.",                   /*38 RF5_BA_WATE */
    " invokes a mana storm.",               /*39 RF5_BA_MANA */
    " invokes a darkness storm.",           /*40 RF5_BA_DARK */
    " draws psychic energy from you!",      /*41 RF5_DRAIN_MANA */
    " gazes deep into your eyes.",          /*42 RF5_MIND_BLAST */
    " looks deep into your eyes.",          /*43 RF5_BRAIN_SMASH */
    " points at you and curses.",           /*44 RF5_CAUSE_1 */
    " points at you and curses horribly.",  /*45 RF5_CAUSE_2 */
    " points at you, incanting terribly!",  /*46 RF5_CAUSE_3 */
    " points at you, screaming the word DIE!",  /*47 RF5_CAUSE_4 */
    " casts a acid bolt.",                  /*48 RF5_BO_ACID */
    " casts a lightning bolt.",             /*49 RF5_BO_ELEC */
    " casts a fire bolt.",                  /*50 RF5_BO_FIRE */
    " casts a frost bolt.",                 /*51 RF5_BO_COLD */
    " does something.",                     /*52 RF5_BO_POIS */
    " casts a nether bolt.",                /*53 RF5_BO_NETH */
    " casts a water bolt.",                 /*54 RF5_BO_WATE */
    " casts a mana bolt.",                  /*55 RF5_BO_MANA */
    " casts a plasma bolt.",                /*56 RF5_BO_PLAS */
    " casts an ice bolt.",                  /*57 RF5_BO_ICEE */
    " casts a magic missile.",              /*58 RF5_MISSILE */
    " casts a fearful illusion.",           /*59 RF5_SCARE */
    " casts a spell, burning your eyes!",   /*60 RF5_BLIND */
    " creates a mesmerising illusion.",     /*61 RF5_CONF */
    " drains power from your muscles!",     /*62 RF5_SLOW */
    " stares deep into your eyes!",         /*63 RF5_HOLD */
    " concentrates on XXX body.",           /*64 RF6_HASTE */
    " does something.",                     /*65 RF6_XXX1X6 */
    " concentrates on XXX wounds.",         /*66 RF6_HEAL */
    " does something.",                     /*67 RF6_XXX2X6 */
    " does something.",                     /*68 RF6_XXX3X6 */
    " does something.",                     /*69 RF6_XXX4X6 */
    " commands you to return.",             /*70 RF6_TELE_TO */
    " teleports you away.",                 /*71 RF6_TELE_AWAY */
    " gestures at your feet.",              /*72 RF6_TELE_LEVEL */
    " does something.",                     /*73 RF6_XXX5 */
    " gestures in shadow.",                 /*74 RF6_DARKNESS */
    " casts a spell and cackles evilly.",   /*75 RF6_TRAPS */
    " tries to blank your mind.",           /*76 RF6_FORGET */
    " does something.",                     /*77 RF6_XXX6X6 */
    " does something.",                     /*78 RF6_XXX7X6 */
    " does something.",                     /*79 RF6_XXX8X6 */
    " magically summons help!",             /*80 RF6_S_MONSTER */
    " magically summons monsters!",         /*81 RF6_S_MONSTERS */
    " magically summons animals.",             /*82 RF6_S_ANIMAL */
    " magically summons spiders.",          /*83 RF6_S_SPIDER */
    " magically summons hounds.",           /*84 RF6_S_HOUND */
    " magically summons hydras.",           /*85 RF6_S_HYDRA */
    " magically summons an angel!",         /*86 RF6_S_ANGEL */
    " magically summons a hellish adversary!",  /*87 RF6_S_DEMON */
    " magically summons an undead adversary!",  /*88 RF6_S_UNDEAD */
    " magically summons a dragon!",         /*89 RF6_S_DRAGON */
    " magically summons greater undead!",   /*90 RF6_S_HI_UNDEAD */
    " magically summons ancient dragons!",  /*91 RF6_S_HI_DRAGON */
    " magically summons mighty undead opponents!",  /*92 RF6_S_WRAITH */
    " magically summons special opponents!",        /*93 RF6_S_UNIQUE */

    NULL
};



#if 0
/* XXX XXX XXX */
msg("%^s looks healthier.", m_name);
msg("%^s looks REALLY healthy!", m_name);
#endif



/*
 * Hack -- Spontaneous level feelings (order important).
 *
 * See "do_cmd_feeling()" for details.
 */
static cptr prefix_feeling[] =
{
    "Looks like any other level",
    "You feel there is something special",
    "You have a superb feeling",
    "You have an excellent feeling",
    "You have a very good feeling",
    "You have a good feeling",
    "You feel strangely lucky",
    "You feel your luck is turning",
    "You like the look of this place",
    "This level can't be all bad",
    "What a boring place",
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
static void borg_parse_aux(cptr msg, int len)
{
    int i, tmp;

	int y9;
	int x9;
 	int ax,ay;
	int d;

    char who[256];
    char buf[256];

    borg_grid *ag = &borg_grids[g_y][g_x];

    /* Log (if needed) */
	if (borg_verbose) borg_note(format("# Parse Msg bite <%s>", msg));

    /* Hack -- Notice death */
    if (prefix(msg, "You die."))
    {
        /* Abort (unless cheating) */
        if (!(p_ptr->wizard || op_ptr->opt[OPT_cheat_live]))
        {
            /* Abort */
            borg_oops("death");

            /* Abort right now! */
            borg_active = FALSE;
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
        borg_failure = TRUE;

        /* Flush our key-buffer */
        borg_flush();

        /* If we were casting a targetted spell and failed */
        /* it does not mean we can't target that location */
        successful_target = 0;

        /* Incase we failed our emergency use of MM */
        borg_confirm_target = FALSE;

        /* Check to see if it was a door then convert it */
        if (ag->feat == FEAT_DOOR_HEAD)
        {
            /* What is my chance of opening the door? */
            if (borg_skill[BI_DIS] < 20)
            {
                /* Set door as jammed, then bash it */
                ag->feat = FEAT_DOOR_HEAD + 0x08;
            }
        }

        /* check for glyphs since we no longer have a launch message */
        if (borg_casted_glyph)
        {
            /* Forget the newly created-though-failed  glyph */
            track_glyph_num --;
            track_glyph_x[track_glyph_num] = 0;
            track_glyph_y[track_glyph_num] = 0;
            borg_note("# Removing glyph from array,");
            borg_casted_glyph = FALSE;
        }

        /* Incase it was a Resistance refresh */
        if (borg_attempting_refresh_resist)
        {
            if (borg_resistance > 1) borg_resistance -=25000;
            borg_attempting_refresh_resist = FALSE;
        }

        return;

    }


    /* Ignore teleport trap */
    if (prefix(msg, "You hit a teleport")) return;

    /* Ignore arrow traps */
    if (prefix(msg, "An arrow ")) return;

    /* Ignore dart traps */
    if (prefix(msg, "A small dart ")) return;

    if (prefix(msg, "The cave "))
    {
        borg_react(msg, "QUAKE");
        borg_needs_new_sea = TRUE;
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
            my_need_stat_check[0] = TRUE;
        }
        if (prefix(msg, "You feel less weak"))
        {
            my_need_stat_check[0] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very strong"))
        {
            my_need_stat_check[0] = TRUE;
        }

        /* need to check int */
        if (prefix(msg, "You feel very stupid"))
        {
            my_need_stat_check[1] = TRUE;
        }
        if (prefix(msg, "You feel less stupid"))
        {
            my_need_stat_check[1] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very smart"))
        {
            my_need_stat_check[1] = TRUE;
        }

        /* need to check wis */
        if (prefix(msg, "You feel very naive"))
        {
            my_need_stat_check[2] = TRUE;
        }
        if (prefix(msg, "You feel less naive"))
        {
            my_need_stat_check[2] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very wise"))
        {
            my_need_stat_check[2] = TRUE;
        }

        /* need to check dex */
        if (prefix(msg, "You feel very clumsy"))
        {
            my_need_stat_check[3] = TRUE;
        }
        if (prefix(msg, "You feel less clumsy"))
        {
            my_need_stat_check[3] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very dextrous"))
        {
            my_need_stat_check[3] = TRUE;
        }

        /* need to check con */
        if (prefix(msg, "You feel very sickly"))
        {
            my_need_stat_check[4] = TRUE;
        }
        if (prefix(msg, "You feel less sickly"))
        {
            my_need_stat_check[4] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very healthy"))
        {
            my_need_stat_check[4] = TRUE;
        }

        /* need to check cha */
        if (prefix(msg, "You feel very ugly"))
        {
            my_need_stat_check[5] = TRUE;
        }
        if (prefix(msg, "You feel less ugly"))
        {
            my_need_stat_check[5] = TRUE;
        }
        if (prefix(msg, "Wow!  You feel very cute"))
        {
            my_need_stat_check[5] = TRUE;
        }
    }

    /* time attacks, just do all stats. */
    if (prefix(msg, "You're not as"))
    {
        my_need_stat_check[0] = TRUE;
        my_need_stat_check[1] = TRUE;
        my_need_stat_check[2] = TRUE;
        my_need_stat_check[3] = TRUE;
        my_need_stat_check[4] = TRUE;
        my_need_stat_check[5] = TRUE;
    }

    /* Nexus attacks, need to check everything! */
    if (prefix(msg, "Your body starts to scramble..."))
    {
        my_need_stat_check[0] = TRUE;
        my_need_stat_check[1] = TRUE;
        my_need_stat_check[2] = TRUE;
        my_need_stat_check[3] = TRUE;
        my_need_stat_check[4] = TRUE;
        my_need_stat_check[5] = TRUE;

        /* max stats may have lowered */
        my_stat_max[0] = 0;
        my_stat_max[1] = 0;
        my_stat_max[2] = 0;
        my_stat_max[3] = 0;
        my_stat_max[4] = 0;
        my_stat_max[5] = 0;

    }

    /* A bug in the 280 game fails to inscribe {empty} on a staff-wand after
     * being hit by amnesia (if the item had a sale inscription).
     * So we will try to use the wand, see that it is empty then inscribe
     * it ourselves.
     */
    if (strstr(msg, " has no charges."))
    {
        /* make the inscription */

        /* not needed in 285,  the game bug was fixed. */
        borg_keypress('{');
        borg_keypress(I2A(zap_slot));

        /* "you inscribe the " */
        borg_keypress('e');
        borg_keypress('m');
        borg_keypress('p');
        borg_keypress('t');
        borg_keypress('y');
        borg_keypress('\n');

        /* done */

	}
    /* amnesia attacks, re-id wands, staves, equipment. */
    if (prefix(msg, "You feel your memories fade."))
    {
		/* Set the borg flag */
		borg_skill[BI_ISFORGET] = TRUE;

#if 0 /* 309 modified the amnesia attack, we dont forget */
        int i;

        /* I was hit by amnesia, forget things */
        /* forget equipment */
        /* Look for an item to forget (equipment) */
        for (i = INVEN_WIELD; i <= INVEN_FEET; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* Skip known items */
            if (item->fully_identified) continue;

            /* skip certain easy know items */
            if ((item->tval == TV_RING) &&
                ((item->sval == SV_RING_FREE_ACTION) ||
                 (item->sval == SV_RING_SEE_INVIS) ||
                 (item->sval <= SV_RING_SUSTAIN_CHR))) continue;

            /* skip already forgotten or non id'd items */
            if (!item->ident) continue;

            /* forget it */
            item->ident = FALSE;

            /* note the forgeting */
            borg_note(format("Borg 'forgetting' qualities of %s",item->desc));

        }

        /* Look for an item to forget (inventory) */
        for (i = 0; i <= INVEN_MAX_PACK; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            /* skip certain easy know items */
            if ((item->tval == TV_RING) &&
                (of_has(item->flags, OF_EASY_KNOW)) continue;

            if (item->fully_identified) continue;

            switch (item->tval)
            {
                /* forget wands, staffs, weapons, armour */
                case TV_WAND:
                case TV_STAFF:
                case TV_ROD:
                case TV_RING:
                case TV_AMULET:
                case TV_LIGHT:
                case TV_SHOT:
                case TV_ARROW:
                case TV_BOLT:
                case TV_BOW:
                case TV_DIGGING:
                case TV_HAFTED:
                case TV_POLEARM:
                case TV_SWORD:
                case TV_BOOTS:
                case TV_GLOVES:
                case TV_HELM:
                case TV_CROWN:
                case TV_SHIELD:
                case TV_CLOAK:
                case TV_SOFT_ARMOR:
                case TV_HARD_ARMOR:
                case TV_DRAG_ARMOR:
                break;

                default:
                    continue;
            }
                /* forget it */
                item->ident = FALSE;

                /* note the forgetting */
                borg_note(format("Borg 'forgetting' qualities of %s",item->desc));
         }
#endif /* Amnesia attack */
    }
    if (streq(msg, "Your memories come flooding back."))
    {

        borg_skill[BI_ISFORGET] = FALSE;
    }

    if (streq(msg, "You have been knocked out."))
    {
        borg_note("Ignoring Messages While KO'd");
        borg_dont_react = TRUE;
    }
    if (streq(msg, "You are paralyzed"))
    {
        borg_note("Ignoring Messages While Paralyzed");
        borg_dont_react = TRUE;
    }

    /* Hallucination -- Open */
    if (streq(msg, "You feel drugged!"))
    {
        borg_note("# Hallucinating.  Special control of wanks.");
        borg_skill[BI_ISIMAGE] = TRUE;
    }

    /* Hallucination -- Close */
    if (streq(msg, "You can see clearly again."))
    {
        borg_note("# Hallucination ended.  Normal control of wanks.");
        borg_skill[BI_ISIMAGE] = FALSE;
    }

    /* Hit somebody */
    if (prefix(msg, "You hit "))
    {
        tmp = strlen("You hit ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "HIT:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody */
    if (prefix(msg, "You miss "))
    {
        tmp = strlen("You miss ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody (because of fear) */
    if (prefix(msg, "You are too afraid to attack "))
    {
        tmp = strlen("You are too afraid to attack ");
        strnfmt(who, 1 + len - (tmp + 1), "%s", msg + tmp);
        strnfmt(buf, 256, "MISS:%^s", who);
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

    /* "It screams in pain." (etc) */
    for (i = 0; suffix_pain[i]; i++)
    {
        /* "It screams in pain." (etc) */
        if (suffix(msg, suffix_pain[i]))
        {
            tmp = strlen(suffix_pain[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "PAIN:%^s", who);
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
            strnfmt(buf, 256, "KILL:%^s", who);
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
            strnfmt(buf, 256, "DIED:%^s", who);
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
            strnfmt(buf, 256, "BLINK:%^s", who);
            borg_react(msg, buf);
            return;
        }
    }

    /* "It misses you." */
    if (suffix(msg, " misses you."))
    {
        tmp = strlen(" misses you.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "MISS_BY:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* "It is repelled.." */
    /* treat as a miss */
    if (suffix(msg, " is repelled."))
    {
        tmp = strlen(" is repelled.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "MISS_BY:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* "It hits you." (etc) */
    for (i = 0; suffix_hit_by[i]; i++)
    {
        /* "It hits you." (etc) */
        if (suffix(msg, suffix_hit_by[i]))
        {
            tmp = strlen(suffix_hit_by[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "HIT_BY:%^s", who);
            borg_react(msg, buf);

            /* If I was hit, then I am not on a glyph */
            if (track_glyph_num)
            {
                /* erase them all and
                 * allow the borg to scan the screen and rebuild the array.
                 * He won't see the one under him though.  So a special check
                 * must be made.
                 */
                byte feat = cave->feat[c_y][c_x];

                 /* Remove the entire array */
                 for (i = 0; i < track_glyph_num; i++)
                 {
                     /* Stop if we already new about this glyph */
                     track_glyph_x[i] = 0;
                     track_glyph_y[i] = 0;
                 }
                 track_glyph_num = 0;

                /* Check for glyphs under player -- Cheat*/
                if (feat == FEAT_GLYPH)
                {
                    track_glyph_x[track_glyph_num] = c_x;
                    track_glyph_y[track_glyph_num] = c_y;
                    track_glyph_num++;
                }
            }
            return;
        }
    }


    /* "It casts a spell." (etc) */
    for (i = 0; suffix_spell[i]; i++)
    {
        /* "It casts a spell." (etc) */
        if (suffix(msg, suffix_spell[i]))
        {
            tmp = strlen(suffix_spell[i]);
            strnfmt(who, 1 + len - tmp, "%s", msg);
            strnfmt(buf, 256, "SPELL_%03d:%^s", i, who);
            borg_react(msg, buf);
            return;
        }
    }


    /* State -- Asleep */
    if (suffix(msg, " falls asleep!"))
    {
        tmp = strlen(" falls asleep!");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_SLEEP:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- confused */
    if (suffix(msg, " looks confused."))
    {
        tmp = strlen(" looks confused.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_CONFUSED:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- confused */
    if (suffix(msg, " looks more confused."))
    {
        tmp = strlen(" looks more confused.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_CONFUSED:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Asleep */
    if (suffix(msg, " wakes up."))
    {
        tmp = strlen(" wakes up.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE_AWAKE:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Afraid */
    if (suffix(msg, " flees in terror!"))
    {
        tmp = strlen(" flees in terror!");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__FEAR:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers his courage."))
    {
        tmp = strlen(" recovers his courage.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__BOLD:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers her courage."))
    {
        tmp = strlen(" recovers her courage.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__BOLD:%^s", who);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers its courage."))
    {
        tmp = strlen(" recovers its courage.");
        strnfmt(who, 1 + len - tmp, "%s", msg);
        strnfmt(buf, 256, "STATE__BOLD:%^s", who);
        borg_react(msg, buf);
        return;
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
    if (streq(msg, "The door appears to be stuck."))
    {
        /* Only process non-jammed doors */
        if ((ag->feat >= FEAT_DOOR_HEAD) && (ag->feat <= FEAT_DOOR_HEAD + 0x07))
        {
            /* Mark the door as jammed */
            ag->feat = FEAT_DOOR_HEAD + 0x08;

            /* Clear goals */
            goal = 0;
        }

        return;
    }



    /* Feature XXX XXX XXX */
    if (streq(msg, "This seems to be permanent rock."))
    {
        /* Only process walls */
        if ((ag->feat >= FEAT_WALL_EXTRA) && (ag->feat <= FEAT_PERM_SOLID))
        {
            /* Mark the wall as permanent */
            ag->feat = FEAT_PERM_EXTRA;

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
        if ((ag->feat >= FEAT_WALL_EXTRA) && (ag->feat <= FEAT_PERM_SOLID))
        {
            /* Mark the wall as granite */
            ag->feat = FEAT_WALL_EXTRA;

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
    if ((prefix(msg, "Oops! It feels deathly cold!")) ||
        (suffix(msg, " seems to be cursed.")) ||
        (suffix(msg, " appears to be cursed.")))
    {
        /* Hack -- Oops */
        borg_wearing_cursed =TRUE;
        return;
    }

    /* protect from evil */
    if (prefix(msg, "You feel safe from evil!"))
    {
        borg_prot_from_evil = TRUE;
        return;
    }
    if (prefix(msg, "You no longer feel safe from evil."))
    {
        borg_prot_from_evil = FALSE;
        return;
    }
    /* haste self */
    if (prefix(msg, "You feel yourself moving faster!"))
    {
        borg_speed = TRUE;
        return;
    }
    if (prefix(msg, "You feel yourself slow down."))
    {
        borg_speed = FALSE;
        return;
    }
    /* Bless */
    if (prefix(msg, "You feel righteous"))
    {
        borg_bless = TRUE;
        return;
    }
    if (prefix(msg, "The prayer has expired."))
    {
        borg_bless = FALSE;
        return;
    }

    /* hero */
    if (prefix(msg, "You feel like a hero!"))
    {
        borg_hero = TRUE;
        return;
    }
    if (prefix(msg, "You no longer feel heroic."))
    {
        borg_hero = FALSE;
        return;
    }

    /* berserk */
    if (prefix(msg, "You feel like a killing machine!"))
    {
        borg_berserk = TRUE;
        return;
    }
    if (prefix(msg, "You no longer feel berserk."))
    {
        borg_berserk = FALSE;
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
        my_need_redraw = TRUE;
        my_need_alter = TRUE;
        goal = 0;
        return;
    }


    /* check for closed door but not when confused*/
    if ((prefix(msg, "There is a closed door blocking your way.") &&
        (!borg_skill[BI_ISCONFUSED] &&
         !borg_skill[BI_ISIMAGE])))
    {
        my_need_redraw = TRUE;
        my_need_alter = TRUE;
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

            borg_kill *kill = &borg_kills[i];

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

        my_no_alter = TRUE;
        goal = 0;
        return;
    }

	/* Check for the missing staircase */
	if (suffix(msg, " staircase here."))
	{
		/* make sure the aligned dungeon is on */

		/* make sure the borg does not think he's on one */
		/* Remove all stairs from the array. */
        track_less_num = 0;
        track_more_num = 0;
		borg_on_dnstairs = FALSE;
		borg_on_upstairs = FALSE;
		borg_grids[c_y][c_x].feat = FEAT_BROKEN;

		return;
	}

    /* Feature XXX XXX XXX */
    if (prefix(msg, "You see nothing there "))
    {
        ag->feat = FEAT_BROKEN;

        my_no_alter = TRUE;
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
        time_this_panel +=100;
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "You have nothing to identify"))
    {
        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel +=100;

        /* ID all items (equipment) */
        for (i = INVEN_WIELD; i <= INVEN_FEET; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            item->ident = TRUE;
        }

        /* ID all items  (inventory) */
        for (i = 0; i <= INVEN_MAX_PACK; i++)
        {
            borg_item *item = &borg_items[i];

            /* Skip empty items */
            if (!item->iqty) continue;

            item->ident = TRUE;
        }
        return;
    }

    /* Hack to protect against clock overflows and errors */
    if (prefix(msg, "Identifying The Phial"))
    {

        /* ID item (equipment) */
        borg_item *item = &borg_items[INVEN_LIGHT];
        item->ident = TRUE;

        /* Hack -- Oops */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        time_this_panel +=100;
    }

    /* resist acid */
    if (prefix(msg, "You feel resistant to acid!"))
    {
        borg_skill[BI_TRACID] = TRUE;
        return;
    }
    if (prefix(msg, "You are no longer resistant to acid."))
    {
        borg_skill[BI_TRACID] = FALSE;
        return;
    }
    /* resist electricity */
    if (prefix(msg, "You feel resistant to electricity!"))
    {
        borg_skill[BI_TRELEC] = TRUE;
        return;
    }
    if (prefix(msg, "You are no longer resistant to electricity."))
    {
        borg_skill[BI_TRELEC] = FALSE;
        return;
    }
    /* resist fire */
    if (prefix(msg, "You feel resistant to fire!"))
    {
        borg_skill[BI_TRFIRE] = TRUE;
        return;
    }
    if (prefix(msg, "You are no longer resistant to fire."))
    {
        borg_skill[BI_TRFIRE] = FALSE;
        return;
    }
    /* resist cold */
    if (prefix(msg, "You feel resistant to cold!"))
    {
        borg_skill[BI_TRCOLD] = TRUE;
        return;
    }
    if (prefix(msg, "You are no longer resistant to cold."))
    {
        borg_skill[BI_TRCOLD] = FALSE;
        return;
    }
    /* resist poison */
    if (prefix(msg, "You feel resistant to poison!"))
    {
        borg_skill[BI_TRPOIS] = TRUE;
        return;
    }
    if (prefix(msg, "You are no longer resistant to poison."))
    {
        borg_skill[BI_TRPOIS] = FALSE;
        return;
    }

    /* Shield */
    if (prefix(msg, "A mystic shield forms around your body!") ||
		prefix(msg, "Your skin turns to stone."))
    {
        borg_shield = TRUE;
        return;
    }
    if (prefix(msg, "Your mystic shield crumbles away.") ||
		prefix(msg, "A fleshy shade returns to your skin."))
    {
        borg_shield = FALSE;
        return;
    }

    /* Glyph of Warding (the spell no longer gives a report)*/
    /* Sadly  Rune of Protection has no message */
    if (prefix(msg, "You inscribe a mystic symbol on the ground!"))
    {
            /* Check for an existing glyph */
            for (i = 0; i < track_glyph_num; i++)
            {
                /* Stop if we already new about this glyph */
                if ((track_glyph_x[i] == c_x) && (track_glyph_y[i] == c_y)) break;
            }

            /* Track the newly discovered glyph */
            if ((i == track_glyph_num) && (i < track_glyph_size))
            {
                borg_note("# Noting the creation of a glyph.");
                track_glyph_x[i] = c_x;
                track_glyph_y[i] = c_y;
                track_glyph_num++;
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
        byte feat = cave->feat[c_y][c_x];

         /* Remove the entire array */
         for (i = 0; i < track_glyph_num; i++)
         {
             /* Stop if we already new about this glyph */
             track_glyph_x[i] = 0;
             track_glyph_y[i] = 0;

         }
         /* no known glyphs */
         track_glyph_num = 0;

        /* Check for glyphs under player -- Cheat*/
        if (feat == FEAT_GLYPH)
        {
            track_glyph_x[track_glyph_num] = c_x;
            track_glyph_y[track_glyph_num] = c_y;
            track_glyph_num++;
        }
        return;
    }
    /* failed glyph spell message */
    if (prefix(msg, "The object resists the spell") ||
		prefix(msg, "There is no clear floor"))
    {

        /* Forget the newly created-though-failed  glyph */
        track_glyph_x[track_glyph_num] = 0;
        track_glyph_y[track_glyph_num] = 0;
        track_glyph_num --;

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
        for (y = c_y -1; y < c_y +1; y++)
        {
            for (x = c_x -1; x < c_x +1; x++)
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
            borg_kill *kill = &borg_kills[i];

            int x9 = kill->x;
            int y9 = kill->y;
            int ax, ay, d;

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
        borg_fear_region[c_y/11][c_x/11] = 0;

        return;
    }

    /* Be aware and concerned of busted doors */
    if (prefix(msg, "You hear a door burst open!"))
    {
        /* on level 1 and 2 be concerned.  Could be Grip or Fang */
        if (borg_skill[BI_CDEPTH] <= 3 && borg_skill[BI_CLEVEL] <= 5) scaryguy_on_level = TRUE;
    }

	/* Some spells move the borg from his grid */
	if (prefix(msg, "commands you to return.") ||
	    prefix(msg, "teleports you away.") ||
	    prefix(msg, "gestures at your feet."))
	{
		/* If in Lunal mode better shut that off, he is not on the stairs anymore */
		if (borg_lunal_mode) borg_lunal_mode = FALSE;
		borg_note("# Disconnecting Lunal Mode due to monster spell.");
	}

    /* Sometimes the borg will overshoot the range limit of his shooter */
    if (prefix(msg, "Target out of range."))
    {
        /* Fire Anyway? [Y/N] */
        borg_keypress('y');
    }

	/* Feelings about the level */
    for (i = 0; prefix_feeling[i]; i++)
    {
        /* "You feel..." (etc) */
        if (prefix(msg, prefix_feeling[i]))
        {
            strnfmt(buf, 256, "FEELING:%d", i);
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
static void borg_parse(cptr msg)
{
    static char len = 0;
    static char buf[1024];

	/* Note the long message */
	if (borg_verbose && msg) borg_note(format("# Parsing msg <%s>", msg));

    /* Flush messages */
    if (len && (!msg || (msg[0] != ' ')))
    {
        int i, j;

        /* Split out punctuation */
        for (j = i = 0; i < len-1; i++)
        {
            /* Check for punctuation */
            if ((buf[i] == '.') ||
                (buf[i] == '!') ||
                (buf[i] == '?') ||
                (buf[i] == '"'))
            {
                /* Require space */
                if (buf[i+1] == ' ')
                {
                    /* Terminate */
                    buf[i+1] = '\0';

                    /* Parse fragment */
                    borg_parse_aux(buf + j, (i + 1) - j);

                    /* Restore */
                    buf[i+1] = ' ';

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
        len += strnfmt(buf+len, 1024-len, "%s", msg+1);
    }

    /* New message */
    else
    {
        /* Collect, verify, and grow */
        len = strnfmt(buf, 1024, "%s", msg);
    }
}



#ifndef BABLOS

static int adjust_stat_borg(int value, int amount, int borg_roll)
{
    /* Negative amounts or maximize mode */
    if ((amount < 0) || op_ptr->opt[OPT_birth_maximize])
    {
        return (modify_stat_value(value, amount));
    }

    /* Special hack */
    else
    {
        int i;

        /* Apply reward */
        for (i = 0; i < amount; i++)
        {
            if (value < 18)
            {
                value++;
            }
            else if (value < 18+70)
            {
                value += ((borg_roll ? 15 : randint1(15)) + 5);
            }
            else if (value < 18+90)
            {
                value += ((borg_roll ? 6 : randint1(6)) + 2);
            }
            else if (value < 18+100)
            {
                value++;
            }
        }
    }

    /* Return the result */
    return (value);
}

static void get_stats_borg_aux(void)
{
    int i, j;

    int bonus;

    int dice[18];

    /* Roll and verify some stats */
    while (TRUE)
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
        j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

        /* Save that value */
        p_ptr->stat_max[i] = j;

        /* Obtain a "bonus" for "race" and "class" */
        bonus = p_ptr->race->r_adj[i] + p_ptr->class->c_adj[i];

        /* Variable stat maxes */
        if (op_ptr->opt[OPT_birth_maximize])
        {
            /* Start fully healed */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i];

            /* Efficiency -- Apply the racial/class bonuses */
            stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
        }

        /* Fixed stat maxes */
        else
        {
            /* Apply the bonus to the stat (somewhat randomly) */
            stat_use[i] = adjust_stat_borg(p_ptr->stat_max[i], bonus, FALSE);

            /* Save the resulting stat maximum */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
        }
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

    s32b borg_round = 0L;

    /* load up min. stats */
    stat_limit[0] = 14; /* Str */
    stat_limit[1] = 0; /* Int */
    stat_limit[2] = 0; /* Wis */
    stat_limit[3] = 14; /* Dex */
    stat_limit[4] = 14;/* Con */
    stat_limit[5] = 0; /* Chr */

    switch (p_ptr->class->cidx)
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
            bool accept = TRUE;

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
                if (p_ptr->stat_max[i] >= stat_limit[i])

                {
                    accept = TRUE;
                }

                /* This stat is not okay */
                else
                {
                    accept = FALSE;
                    break;
                }
             }

             /* Break if "happy" */
             if (accept) break;
        } /* while */

    /* Note the number of rolls to achieve stats */
    borg_note(format("# Minimal stats rolled in %d turns.",borg_round));

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
    p_ptr->max_lev = p_ptr->lev = 1;

    /* Experience factor */
    p_ptr->expfact = p_ptr->race->r_exp + p_ptr->class->c_exp;

    /* Hitdice */
    p_ptr->hitdie = p_ptr->race->r_mhp + p_ptr->class->c_mhp;

    /* Initial hitpoints */
    p_ptr->mhp = p_ptr->hitdie;

    /* Minimum hitpoints at highest level */
    min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
    min_value += PY_MAX_LEVEL;

    /* Maximum hitpoints at highest level */
    max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
    max_value += PY_MAX_LEVEL;

    /* Pre-calculate level 1 hitdice */
    p_ptr->player_hp[0] = p_ptr->hitdie;

    /* Roll out the hitpoints */
    while (TRUE)
    {
        /* Roll the hitpoint values */
        for (i = 1; i < PY_MAX_LEVEL; i++)
        {
            j = randint0(p_ptr->hitdie);
            p_ptr->player_hp[i] = p_ptr->player_hp[i-1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (p_ptr->player_hp[PY_MAX_LEVEL-1] < min_value) continue;
        if (p_ptr->player_hp[PY_MAX_LEVEL-1] > max_value) continue;

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
	struct history_chart *chart;
	struct history_entry *entry;
	char *res = NULL;


    /* Clear the previous history strings */
    p_ptr->history[0] = '\0';


    /* Initial social class */
    social_class = randint1(4);

    /* Starting place */
    chart = p_ptr->race->history;


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
        my_strcat(p_ptr->history, h_info[i].text, sizeof(p_ptr->history));

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
    p_ptr->sc = social_class;

}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw_borg(void)
{
    /* Calculate the age */
    p_ptr->age = p_ptr->race->b_age + randint0(p_ptr->race->m_age);

    /* Calculate the height/weight for males */
    if (p_ptr->psex == SEX_MALE)
    {
        p_ptr->ht = Rand_normal(p_ptr->race->m_b_ht, p_ptr->race->m_m_ht);
        p_ptr->wt = Rand_normal(p_ptr->race->m_b_wt, p_ptr->race->m_m_wt);
    }

    /* Calculate the height/weight for females */
    else if (p_ptr->psex == SEX_FEMALE)
    {
        p_ptr->ht = Rand_normal(p_ptr->race->f_b_ht, p_ptr->race->f_m_ht);
        p_ptr->wt = Rand_normal(p_ptr->race->f_b_wt, p_ptr->race->f_m_wt);
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
    gold = (p_ptr->sc * 6) + randint1(100) + 300;

    /* Process the stats */
    for (i = 0; i < A_MAX; i++)
    {
        /* Mega-Hack -- reduce gold for high stats */
        if (stat_use[i] >= 18+50) gold -= 300;
        else if (stat_use[i] >= 18+20) gold -= 200;
        else if (stat_use[i] > 18) gold -= 150;
        else gold -= (stat_use[i] - 8) * 10;
    }

    /* Minimum 100 gold */
    if (gold < 100) gold = 100;

    /* Save the gold */
    p_ptr->au = gold;
}
/*
 * Name segments for random player names
 * Copied Cth by DvE
 * Copied from borgband by APW
 */

/* Dwarves */
static char *dwarf_syllable1[] =
{
    "B", "D", "F", "G", "Gl", "H", "K", "L", "M", "N", "R", "S", "T", "Th", "V",
};

static char *dwarf_syllable2[] =
{
    "a", "e", "i", "o", "oi", "u",
};

static char *dwarf_syllable3[] =
{
    "bur", "fur", "gan", "gnus", "gnar", "li", "lin", "lir", "mli", "nar", "nus", "rin", "ran", "sin", "sil", "sur",
};

/* Elves */
static char *elf_syllable1[] =
{
    "Al", "An", "Bal", "Bel", "Cal", "Cel", "El", "Elr", "Elv", "Eow", "Ear", "F", "Fal", "Fel", "Fin", "G", "Gal", "Gel", "Gl", "Is", "Lan", "Leg", "Lom", "N", "Nal", "Nel",  "S", "Sal", "Sel", "T", "Tal", "Tel", "Thr", "Tin",
};

static char *elf_syllable2[] =
{
    "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra", "ilma", "il-Ga", "ili", "o", "orfi", "u", "y",
};

static char *elf_syllable3[] =
{
    "l", "las", "lad", "ldor", "ldur", "linde", "lith", "mir", "n", "nd", "ndel", "ndil", "ndir", "nduil", "ng", "mbor", "r", "rith", "ril", "riand", "rion", "s", "thien", "viel", "wen", "wyn",
};

/* Gnomes */
static char *gnome_syllable1[] =
{
    "Aar", "An", "Ar", "As", "C", "H", "Han", "Har", "Hel", "Iir", "J", "Jan", "Jar", "K", "L", "M", "Mar", "N", "Nik", "Os", "Ol", "P", "R", "S", "Sam", "San", "T", "Ter", "Tom", "Ul", "V", "W", "Y",
};

static char *gnome_syllable2[] =
{
    "a", "aa",  "ai", "e", "ei", "i", "o", "uo", "u", "uu",
};

static char *gnome_syllable3[] =
{
    "ron", "re", "la", "ki", "kseli", "ksi", "ku", "ja", "ta", "na", "namari", "neli", "nika", "nikki", "nu", "nukka", "ka", "ko", "li", "kki", "rik", "po", "to", "pekka", "rjaana", "rjatta", "rjukka", "la", "lla", "lli", "mo", "nni",
};

/* Hobbit */
static char *hobbit_syllable1[] =
{
    "B", "Ber", "Br", "D", "Der", "Dr", "F", "Fr", "G", "H", "L", "Ler", "M", "Mer", "N", "P", "Pr", "Per", "R", "S", "T", "W",
};

static char *hobbit_syllable2[] =
{
    "a", "e", "i", "ia", "o", "oi", "u",
};

static char *hobbit_syllable3[] =
{
    "bo", "ck", "decan", "degar", "do", "doc", "go", "grin", "lba", "lbo", "lda", "ldo", "lla", "ll", "lo", "m", "mwise", "nac", "noc", "nwise", "p", "ppin", "pper", "tho", "to",
};

/* Human */
static char *human_syllable1[] =
{
    "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar", "B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et", "Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha", "Ib", "Jer", "K", "Ka", "Ked", "L", "Loth", "Lar", "Leg", "M", "Mir", "N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T", "Tr", "Th", "V", "Y", "Z", "W", "Wic",
};

static char *human_syllable2[] =
{
    "a", "ae", "au", "ao", "are", "ale", "ali", "ay", "ardo", "e", "ei", "ea", "eri", "era", "ela", "eli", "enda", "erra", "i", "ia", "ie", "ire", "ira", "ila", "ili", "ira", "igo", "o", "oa", "oi", "oe", "ore", "u", "y",
};

static char *human_syllable3[] =
{
    "a", "and", "b", "bwyn", "baen", "bard", "c", "ctred", "cred", "ch", "can", "d", "dan", "don", "der", "dric", "dfrid", "dus", "f", "g", "gord", "gan", "l", "li", "lgrin", "lin", "lith", "lath", "loth", "ld", "ldric", "ldan", "m", "mas", "mos", "mar", "mond", "n", "nydd", "nidd", "nnon", "nwan", "nyth", "nad", "nn", "nnor", "nd", "p", "r", "ron", "rd", "s", "sh", "seth", "sean", "t", "th", "tha", "tlan", "trem", "tram", "v", "vudd", "w", "wan", "win", "wyn", "wyr", "wyr", "wyth",
};

/* Orc */
static char *orc_syllable1[] =
{
    "B", "Er", "G", "Gr", "H", "P", "Pr", "R", "V", "Vr", "T", "Tr", "M", "Dr",
};

static char *orc_syllable2[] =
{
    "a", "i", "o", "oo", "u", "ui",
};

static char *orc_syllable3[] =
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
static void create_random_name(int race, char *name)
{
    /* Paranoia */
    if (!name) return;

    /* Select the monster type */
    switch (race)
    {
        /* Create the monster name */
    case RACE_DWARF:
        strcpy(name, dwarf_syllable1[randint0(sizeof(dwarf_syllable1) / sizeof(char*))]);
        strcat(name, dwarf_syllable2[randint0(sizeof(dwarf_syllable2) / sizeof(char*))]);
        strcat(name, dwarf_syllable3[randint0(sizeof(dwarf_syllable3) / sizeof(char*))]);
        break;
    case RACE_ELF:
    case RACE_HALF_ELF:
    case RACE_HIGH_ELF:
        strcpy(name, elf_syllable1[randint0(sizeof(elf_syllable1) / sizeof(char*))]);
        strcat(name, elf_syllable2[randint0(sizeof(elf_syllable2) / sizeof(char*))]);
        strcat(name, elf_syllable3[randint0(sizeof(elf_syllable3) / sizeof(char*))]);
        break;
    case RACE_GNOME:
        strcpy(name, gnome_syllable1[randint0(sizeof(gnome_syllable1) / sizeof(char*))]);
        strcat(name, gnome_syllable2[randint0(sizeof(gnome_syllable2) / sizeof(char*))]);
        strcat(name, gnome_syllable3[randint0(sizeof(gnome_syllable3) / sizeof(char*))]);
        break;
    case RACE_HOBBIT:
        strcpy(name, hobbit_syllable1[randint0(sizeof(hobbit_syllable1) / sizeof(char*))]);
        strcat(name, hobbit_syllable2[randint0(sizeof(hobbit_syllable2) / sizeof(char*))]);
        strcat(name, hobbit_syllable3[randint0(sizeof(hobbit_syllable3) / sizeof(char*))]);
        break;
    case RACE_HUMAN:
    case RACE_DUNADAN:
        strcpy(name, human_syllable1[randint0(sizeof(human_syllable1) / sizeof(char*))]);
        strcat(name, human_syllable2[randint0(sizeof(human_syllable2) / sizeof(char*))]);
        strcat(name, human_syllable3[randint0(sizeof(human_syllable3) / sizeof(char*))]);
        break;
    case RACE_HALF_ORC:
    case RACE_HALF_TROLL:
    case RACE_KOBOLD:
        strcpy(name, orc_syllable1[randint0(sizeof(orc_syllable1) / sizeof(char*))]);
        strcat(name, orc_syllable2[randint0(sizeof(orc_syllable2) / sizeof(char*))]);
        strcat(name, orc_syllable3[randint0(sizeof(orc_syllable3) / sizeof(char*))]);
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
static void player_outfit_borg(struct player *p)
{
	const struct start_item *si;
	object_type object_type_body;

	/* Give the player starting equipment */
	for (si = p_ptr->class->start_items; si; si = si->next)
	{
		/* Get local object */
		struct object *i_ptr = &object_type_body;

		/* Prepare the item */
		object_prep(i_ptr, si->kind, 0, MINIMISE);
		i_ptr->number = (byte)rand_range(si->min, si->max);
		i_ptr->origin = ORIGIN_BIRTH;

		object_flavor_aware(i_ptr);
		object_notice_everything(i_ptr);

		inven_carry(p, i_ptr);
		si->kind->everseen = TRUE;

		/* Deduct the cost of the item from starting cash */
		p->au -= object_value(i_ptr, i_ptr->number, FALSE);
	}

	/* Sanity check */
	if (p->au < 0)
		p->au = 0;

	/* Now try wielding everything */
	wield_all(p);
}


/* Allow the borg to play continously.  Reset all values, */
void resurrect_borg(void)
{
	int i,j;
	int stats[A_MAX];

	/* Cheat death */
    p_ptr->is_dead = FALSE;
    borg_skill[BI_MAXDEPTH] = 0;
    borg_skill[BI_MAXCLEVEL] = 1;

    /* Flush message buffer */
    borg_parse(NULL);

    /* flush the commands */
    borg_flush();

    /* remove the spell counters */
    if (p_ptr->class->spell_book)
    {
        for (i = 0; i < 9; i++ )
        {
           for (j = 0; j < 8; j++)
            {
                /* get the magics */
                borg_magic *as = &borg_magics[i][j];
                /* reset the counter */
                as->times = 0;
            }
        }
    }

    /*** Wipe the player ***/
	player_init(p_ptr);

	borg_skill[BI_ISCUT] = borg_skill[BI_ISSTUN] = borg_skill[BI_ISHEAVYSTUN] = borg_skill[BI_ISIMAGE] = borg_skill[BI_ISSTUDY] = FALSE;

    /* reset our panel clock */
    time_this_panel =1;

    /* reset our vault/unique check */
    vault_on_level = FALSE;
    unique_on_level = 0;
    scaryguy_on_level = FALSE;

    /* reset our breeder flag */
    breeder_level = FALSE;

    /* Assume not leaving the level */
    goal_leaving = FALSE;

    /* Assume not fleeing the level */
    goal_fleeing = FALSE;

    /* Assume not fleeing the level */
    borg_fleeing_town = FALSE;

    /* Assume not ignoring monsters */
    goal_ignoring = FALSE;

    flavor_init();


	/** Roll up a new character **/

	/* Full Random */
    if (borg_respawn_class == -1 && borg_respawn_race == -1)
    {
		player_generate(p_ptr, NULL, NULL, NULL);
	}

	/* Selected Race, random Class */
    if (borg_respawn_race != -1 && borg_respawn_class == -1)
    {
		player_generate(p_ptr, NULL, player_id2race(borg_respawn_race), NULL);
	}

	/* Random Race, Selected Class */
    if (borg_respawn_race == -1 && borg_respawn_class != -1)
    {
		player_generate(p_ptr, NULL, NULL, player_id2class(borg_respawn_class));
	}

		/* The dungeon is not ready */
		character_dungeon = FALSE;

		/* Start in town */
		p_ptr->depth = 0;

		/* Hack -- seed for flavors */
		seed_flavor = randint0(0x10000000);

		/* Hack -- seed for town layout */
		seed_town = randint0(0x10000000);

		/* Hack -- seed for random artifacts */
		seed_randart = randint0(0x10000000);

		/* Roll up a new character. Quickstart is allowed if ht_birth is set */
		player_birth(TRUE);

		/* Randomize the artifacts if required */
		if (OPT(birth_randarts) &&
				(!OPT(birth_keep_randarts) || !p_ptr->randarts)) {
			do_randart(seed_randart, TRUE);
			p_ptr->randarts = TRUE;
		}
#if 0
	/* Borrow commands from birth.c */
	get_stats(stats);
	get_bonuses();
	get_ahw(p_ptr);
	p_ptr->history = get_history(p_ptr->race->history, &p_ptr->sc);
	p_ptr->sc_birth = p_ptr->sc;
	roll_hp();
	get_money();
	store_reset();
#endif
#if 0
	/* Some Extra things */
    get_stats_borg();
    get_extra_borg();
    get_ahw_borg();
    get_history_borg();
   get_money_borg();
#endif

    /* Get a random name */
   create_random_name(p_ptr->race->ridx,op_ptr->full_name);


    /* outfit the player */
	C_MAKE(p_ptr->inventory, ALL_INVEN_TOTAL, struct object);
    (void)player_outfit_borg(p_ptr);

    /* Hack -- flush it */
    Term_fresh();

    /*** Hack -- Extract race ***/

    /* Insert the player Race--cheat */
	borg_race = p_ptr->race->ridx;

	/* Cheat the class */
    borg_class = p_ptr->class->cidx;

    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    prepare_race_class_info();


    /* need to check all stats */
    my_need_stat_check[0] = TRUE;
    my_need_stat_check[1] = TRUE;
    my_need_stat_check[2] = TRUE;
    my_need_stat_check[3] = TRUE;
    my_need_stat_check[4] = TRUE;
    my_need_stat_check[5] = TRUE;

    /* Allowable Cheat -- Obtain "recall" flag */
    goal_recalling = p_ptr->word_recall * 1000;

    /* Allowable Cheat -- Obtain "prot_from_evil" flag */
    borg_prot_from_evil = (p_ptr->timed[TMD_PROTEVIL] ? TRUE : FALSE);
    /* Allowable Cheat -- Obtain "speed" flag */
    borg_speed = (p_ptr->timed[TMD_FAST] ? TRUE : FALSE);
    /* Allowable Cheat -- Obtain "resist" flags */
    borg_skill[BI_TRACID] = (p_ptr->timed[TMD_OPP_ACID] ? TRUE : FALSE);
    borg_skill[BI_TRELEC] = (p_ptr->timed[TMD_OPP_ELEC] ? TRUE : FALSE);
    borg_skill[BI_TRFIRE] = (p_ptr->timed[TMD_OPP_FIRE] ? TRUE : FALSE);
    borg_skill[BI_TRCOLD] = (p_ptr->timed[TMD_OPP_COLD] ? TRUE : FALSE);
    borg_skill[BI_TRPOIS] = (p_ptr->timed[TMD_OPP_POIS] ? TRUE : FALSE);
    borg_bless = (p_ptr->timed[TMD_BLESSED] ? TRUE : FALSE);
    borg_shield = (p_ptr->timed[TMD_SHIELD] ? TRUE : FALSE);
    borg_hero = (p_ptr->timed[TMD_HERO] ? TRUE : FALSE);
    borg_berserk = (p_ptr->timed[TMD_SHERO] ? TRUE : FALSE);
    if (p_ptr->timed[TMD_SINVIS]) borg_see_inv = 10000;

    /* Message */
    borg_note("# Respawning");
    borg_respawning = 5;

    /* fully healed and rested */
    p_ptr->chp = p_ptr->mhp;
    p_ptr->csp = p_ptr->msp;


	/* Mark savefile as borg cheater */
	if (!(p_ptr->noscore & 0x0010)) p_ptr->noscore |= 0x0010;

   /* Done.  Play on */
}
#endif /* bablos */

/*
 * Mega-Hack -- special "inkey_hack" hook.  XXX XXX XXX
 *
 * A special function hook (see "util.c") which allows the Borg to take
 * control of the "inkey()" function, and substitute in fake keypresses.
 */
extern char (*inkey_hack)(int flush_first);

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
static char borg_inkey_hack(int flush_first)
{
    char borg_ch;

	ui_event ch_evt;

    int y = 0;
    int x = ((Term->wid /* - (COL_MAP)*/ - 1) / (tile_width));

    byte t_a;

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
        flush();

        /* Done */
        return (0);
    }


    /* Mega-Hack -- flush keys */
    if (flush_first)
    {
        /* Only flush if needed */
        if (borg_inkey(FALSE) != 0)
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
    borg_prompt = FALSE;


    /* Mega-Hack -- check for possible prompts/messages */
    /* If the first four characters on the message line all */
    /* have the same attribute (or are all spaces), and they */
    /* are not all spaces (ascii value 0x20)... */
    if ((0 == borg_what_text(0, 0, 4, &t_a, buf)) &&
        (t_a != TERM_DARK) &&
        (*((u32b*)(buf)) != 0x20202020))
    {
        /* Assume a prompt/message is available */
        borg_prompt = TRUE;
    }
	if (borg_prompt && streq(buf, "Type"))
    {
        borg_prompt = FALSE;
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
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(p_ptr->died_from, "starvation")) borg_write_map(FALSE);

        /* Log the death */
        borg_log_death();
        borg_log_death_data();

#if 0
		/* Note the score */
        borg_enter_score();
#endif
        /* Reset the player game data then resurrect a new player */
        resurrect_borg();

#endif /* BABLOS */

        /* Cheat death */
        return ('n');
    }

    /* with 292, there is a flush() introduced as it asks for confirmation.
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
        return ('y');
    }

    /* with 292, there is a flush() introduced as it asks for confirmation.
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
        borg_confirm_target = FALSE;
        /* Return 5 for old target */
            return ('5');
    }

	/* Wearing two rings.  Place this on the left hand */
	if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 12) &&
        (0 == borg_what_text(0, y, 12, &t_a, buf)) &&
        (streq(buf, "(Equip: c-d,")))
    {
        /* Left hand */
        return ('c');
    }

	/* Mega-Hack -- Handle death */
    if (p_ptr->is_dead)
    {
#ifndef BABLOS
        /* Print the map */
        if (borg_skill[BI_CLEVEL] >= borg_dump_level ||
            strstr(p_ptr->died_from, "starvation"))  borg_write_map(FALSE);

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

        if (borg_cheat_death)
        {
            /* Reset death flag */
            p_ptr->is_dead = FALSE;
#ifndef BABLOS
            /* Reset the player game data then resurrect a new player */
            resurrect_borg();
#endif /* bablos */
        }
        else
        {
            /* Oops  */
            borg_oops("player died");

            /* Useless keypress */
            return (KTRL('C'));
        }
    }


    /* Mega-Hack -- Catch "-more-" messages */
    /* If there is text on the first line... */
    /* And the game does not want a command... */
    /* And the cursor is on the top line... */
    /* And there is text before the cursor... */
    /* And that text is "-more-" */
    if (borg_prompt && !inkey_flag &&
        (y == 0) && (x >= 7) &&
        (0 == borg_what_text(x-7, y, 7, &t_a, buf)) &&
        (streq(buf, " -more-")))
    {
        /* Get the message */
        if (0 == borg_what_text(0, 0, x-7, &t_a, buf))
        {
            /* Parse it */
            borg_parse(buf);
        }
        /* Clear the message */
        return (' ');
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
            while ((k > 0) && (buf[k-1] == ' ')) k--;

            /* Terminate */
            buf[k] = '\0';

            /* Parse it */
            borg_parse(buf);
        }

        /* Clear the message */
        return (' ');

    }
    /* Flush messages */
    borg_parse(NULL);
    borg_dont_react = FALSE;

    /* Check for key */
    borg_ch = borg_inkey(TRUE);

    /* Use the key */
    if (borg_ch) return (borg_ch);


	/* Check for user abort */
	(void)Term_inkey(&ch_evt, FALSE, TRUE);

	/* Hack to keep him active in town. */
	if (borg_skill[BI_CDEPTH] >= 1) borg_in_shop = FALSE;


	if (!borg_in_shop && (ch_evt.type & EVT_KBRD) && ch_evt.key.code > 0 &&
		ch_evt.key.code != 10)
	{
		/* Oops */
		borg_note(format("# User key press <%d><%c>",ch_evt.key.code ,ch_evt.key.code));
		borg_note(format("# Key type was <%d><%c>",ch_evt.type,ch_evt.type));
		borg_oops("user abort");

		/* Hack -- Escape */
		return (ESCAPE);
	}

	/* for some reason, selling and buying in the store sets the event handler to Select. */
	if (ch_evt.type & EVT_SELECT) ch_evt.type = EVT_KBRD;
	if (ch_evt.type & EVT_MOVE) ch_evt.type = EVT_KBRD;

    /* Save the system random info */
    borg_rand_quick = Rand_quick;
    borg_rand_value = Rand_value;

    /* Use the local random info */
    Rand_quick = TRUE;
    Rand_value = borg_rand_local;


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
    if (borg_step && (!--borg_step)) borg_cancel = TRUE;


    /* Check for key */
    borg_ch = borg_inkey(TRUE);

    /* Use the key */
    if (borg_ch) return (borg_ch);


    /* Oops */
    borg_oops("normal abort");

    /* Hack -- Escape */
    return (ESCAPE);
}

/*
 * Output a long int in binary format.
 */
static void borg_prt_binary(u32b flags, int row, int col)
{
	int        	i;
	u32b        bitmask;

	/* Scan the flags */
	for (i = bitmask = 1; i <= 32; i++, bitmask *= 2)
	{
		/* Dump set bits */
		if (flags & bitmask)
		{
			Term_putch(col++, row, TERM_BLUE, '*');
		}

		/* Dump unset bits */
		else
		{
			Term_putch(col++, row, TERM_WHITE, '-');
		}
	}
}

/* this will display the values which the borg believes an
 * item has.  Select the item by inven # prior to hitting
 * the ^zo.
 */
static void borg_display_item(object_type *item2)
{
	int j = 0;

	bitflag f[OF_SIZE];

	borg_item *item;

	item = &borg_items[p_ptr->command_arg];

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

	prt(format("name1 = %-4d  name2 = %-4d  value = %d   cursed = %ld",
	           item->name1, item->name2, (long)item->value, item->cursed), 7, j);

	prt(format("*id*need = %d  ident = %d      fully_id = %d  timeout = %-d",
	           item->needs_I, item->ident, item->fully_identified, item->timeout), 8, j);

	/* maybe print the inscription */
	prt(format("Inscription: %s, activation: %d, chance: %d",item->note, item->activation, borg_skill[BI_DEV] -
		item->level),9,j);

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
	if (item->fully_identified) borg_prt_binary(f[1], 23, j);

	prt("+------------FLAGS3------------+", 10, j+32);
	prt("s   ts h     tadiiii   aiehs  hp", 11, j+32);
	prt("lf  eefo     egrgggg  bcnaih  vr", 12, j+32);
	prt("we  lerln   ilgannnn  ltssdo  ym", 13, j+32);
	prt("da reiedo   merirrrr  eityew ccc", 14, j+32);
	prt("itlepnelf   ppanaefc  svaktm uuu", 15, j+32);
	prt("ghigavaiu   aoveclio  saanyo rrr", 16, j+32);
	prt("seteticfe   craxierl  etropd sss", 17, j+32);
	prt("trenhstel   tttpdced  detwes eee", 18, j+32);
	if (item->fully_identified) borg_prt_binary(f[2], 19, j+32);
}


#ifdef ALLOW_BORG_GRAPHICS

glyph translate_visuals[255][255];

/*
 * Return the "attr" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define borg_object_kind_attr(T) \
   (((T)->flavor) ? \
    (flavor_info[(T)->flavor].x_attr) : \
    ((T)->x_attr))

/*
 * Return the "char" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define borg_object_kind_char(T) \
   (((T)->flavor) ? \
    (flavor_info[(T)->flavor].x_char) : \
    ((T)->x_char))

void init_translate_visuals(void)
{
    int i, j;

    bool graf_new = (use_graphics && streq(ANGBAND_GRAF, "new"));
    bool graf_david = (use_graphics && streq(ANGBAND_GRAF, "david"));


    /* Extract default attr/char code for features */
    for (i = 0; i < z_info->f_max; i++)
    {
        feature_type *f_ptr = &f_info[i];

        if (!f_ptr->name) continue;

        /* Store the underlying values */
        translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
        translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

        /* Add the various ASCII lighting levels */
        if (f_ptr->x_attr == TERM_WHITE)
        {
            translate_visuals[TERM_YELLOW][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
            translate_visuals[TERM_YELLOW][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

            translate_visuals[TERM_L_DARK][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
            translate_visuals[TERM_L_DARK][(byte)f_ptr->x_char].d_char = f_ptr->d_char;

            translate_visuals[TERM_SLATE][(byte)f_ptr->x_char].d_attr = f_ptr->d_attr;
            translate_visuals[TERM_SLATE][(byte)f_ptr->x_char].d_char = f_ptr->d_char;
        }
        else if (graf_new && feat_supports_lighting((byte)i) &&
                 (f_ptr->x_char & 0x80) && (f_ptr->x_attr & 0x80))
        {
            translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char + 1].d_attr = f_ptr->d_attr;
            translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char + 1].d_char = f_ptr->d_char;

            translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char + 2].d_attr = f_ptr->d_attr;
            translate_visuals[(byte)f_ptr->x_attr][(byte)f_ptr->x_char + 2].d_char = f_ptr->d_char;
        }
    }

    /* Extract default attr/char code for objects */
    for (i = 0; i < z_info->k_max; i++)
    {
        object_kind *k_ptr = &k_info[i];

        if (!k_ptr->name) continue;

        /* Store the underlying values */
        translate_visuals[(byte)borg_object_kind_attr(k_ptr)][(byte)borg_object_kind_char(k_ptr)].d_attr = k_ptr->d_attr;
        translate_visuals[(byte)borg_object_kind_attr(k_ptr)][(byte)borg_object_kind_char(k_ptr)].d_char = k_ptr->d_char;
    }

    /* Extract default attr/char code for monsters */
    for (i = 0; i < z_info->r_max; i++)
    {
        monster_race *r_ptr = &r_info[i];

        if (!r_ptr->name) continue;

        /* Store the underlying values */
        translate_visuals[(byte)r_ptr->x_attr][(byte)r_ptr->x_char].d_attr = r_ptr->d_attr;
        translate_visuals[(byte)r_ptr->x_attr][(byte)r_ptr->x_char].d_char = r_ptr->d_char;

        /* Multi-hued monster in ASCII mode */
        if (rf_has(r_ptr->flags, RF_CHAR_MULTI) &&
            !((r_ptr->x_attr & 0x80) && (r_ptr->x_char & 0x80)))
        {
            for (j = 0; j < 16; j++)
            {
                translate_visuals[j][(byte)r_ptr->x_char].d_attr = j;
                translate_visuals[j][(byte)r_ptr->x_char].d_char = r_ptr->d_char;
            }
        }
    }
}

#endif /* ALLOW_BORG_GRAPHICS */

static int
borg_getval(char ** string, char * val)
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
    *string+=3;
    return retval;
}

static bool borg_load_formula(char * string)
{
    int formula_num;
    int iformula = 0;
    int x = 0;
    int value= 0;
    char string2[4];

    memmove(string2, string, 3);
    string2[3] = 0;
    sscanf(string2, "%d", &formula_num);
    string+=4;
    if (formula[formula_num])
    {
        borg_note(format("formula defined twice %03d", formula_num));
        return FALSE;
    }
    C_MAKE(formula[formula_num], MAX_FORMULA_ELEMENTS, int);

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
                if (iformula+2 > MAX_FORMULA_ELEMENTS)
                {
                    borg_note(format("too many elements in formula %03d", formula_num));
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    return FALSE;
                }
                formula[formula_num][iformula++] = BFO_NUMBER;
                formula[formula_num][iformula++] = value;
                break;
            case '_':
                if (iformula+2 > MAX_FORMULA_ELEMENTS)
                {
                    borg_note(format("too many elements in formula %03d", formula_num));
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    return FALSE;
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
                    return FALSE;
                }

                break;
            default:
                if (iformula+1 > MAX_FORMULA_ELEMENTS)
                {
                    borg_note(format("too many elements in formula %03d", formula_num));
                    formula[formula_num][0] = BFO_NUMBER;
                    formula[formula_num][1] = 0;
                    return FALSE;
                }
                if (*string == '>')
                {
                    if (*(string+1) == '=')
                    {
                        formula[formula_num][iformula++] = BFO_GTE;
                        break;
                    }
                    formula[formula_num][iformula++] = BFO_GT;
                    break;
                }
                if (*string == '<')
                {
                    if (*(string+1) == '=')
                    {
                        formula[formula_num][iformula++] = BFO_LTE;
                        break;
                    }
                    formula[formula_num][iformula++] = BFO_LT;
                    break;
                }
                if (*string == '!')
                {
                    if (*(string+1) == '=')
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
                    if (*(string+1) == ' ')
                    {
                        formula[formula_num][iformula++] = BFO_MINUS;
                        break;
                    }
                    if (iformula+1 > MAX_FORMULA_ELEMENTS)
                    {
                        borg_note(format("too many elements in formula %03d", formula_num));
                        formula[formula_num][0] = BFO_NUMBER;
                        formula[formula_num][1] = 0;
                        return FALSE;
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
                return FALSE;
        }
        string = strchr(string, ' ');
    }
    if (!borg_check_formula(formula[formula_num]))
    {
        borg_note(format("bad formula %03d", formula_num));
        formula[formula_num][0] = BFO_NUMBER;
        formula[formula_num][1] = 0;
        return FALSE;
    }
    return TRUE;
}


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
        class_num != 999 ) ||
        depth_num >= MAX_DEPTH ||
        item_num >= (z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX) ||
        range_to < range_from)
    {
        borg_note("Malformed item power in borg.txt: values out of range");
        return FALSE;
    }
    /* The class 999 is for all classes */
    if (class_num == 999)
    {
        for (class_num = 0; class_num < MAX_CLASSES; class_num ++)
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
    return TRUE;
}

static bool borg_load_power(char * string)
{
    int class_num= -1;
    int depth_num= -1;
    int cnd_num = -1;
    int range_to= -1;
    int range_from= -1;
    bool each = FALSE;
    int item_num= -1;
    int power= -1;
    int x;


    if (-1000 == (class_num = borg_getval(&string, "_CLASS")))
    {
        borg_note("Malformed item power in borg.txt: missing _CLASS");
        return FALSE;
    }
    if (-1000 == (depth_num = borg_getval(&string, "_DEPTH")))
    {
        borg_note("Malformed item power in borg.txt: missing _DEPTH");
        return FALSE;
    }
    if (-1000 == (cnd_num = borg_getval(&string, "_CND")))
    {
        /* condition is optional */
        cnd_num = -1;
    }
    if (-1000 == (range_from = borg_getval(&string, "_RANGE")))
    {
        borg_note("Malformed item power in borg.txt: missing _RANGE");
        return FALSE;
    }
    if (-1000 == (range_to = borg_getval(&string, "TO")))
    {
        borg_note("Malformed item power in borg.txt: messed up _RANGE");
        return FALSE;
    }

    if (-1000 != (item_num = borg_getval(&string, "_FORMULA")))
    {
        if (range_to != 999 || range_from != 0)
        {
            borg_note("Malformed item power in borg.txt: range must be 0-999 formulas");
            return FALSE;
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
            each = TRUE;

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
            each = TRUE;

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
            each = TRUE;
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
                each = TRUE;
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
    return FALSE;
}
static bool add_required_item(int class_num, int depth_num, int item_num, int number_items)
{
    if ((class_num >MAX_CLASSES &&
        class_num != 999 ) ||
        depth_num >= MAX_DEPTH ||
        item_num >= (z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX))
    {
        borg_note("Malformed item requirment in borg.txt: value out of range");
        return FALSE;
    }
    /* The class 999 is for all classes */
    if (class_num == 999)
    {
        for (class_num = 0; class_num < MAX_CLASSES; class_num ++)
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
    return TRUE;
}

static bool borg_load_requirement(char * string)
{
    int class_num=-1;
    int depth_num=-1;
    int item_num=-1;
    int number_items=-1;
    int x=-1;

    if (-1000 == (class_num = borg_getval(&string, "_CLASS")))
    {
        borg_note("Malformed item requirment in borg.txt");
        return FALSE;
    }
    if (-1000 == (depth_num = borg_getval(&string, "_DEPTH")))
    {
        borg_note("Malformed item requirment in borg.txt");
        return FALSE;
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
        return add_required_item(class_num, depth_num,  z_info->k_max + item_num, number_items);
    }
    if (-1000 != (item_num = borg_getval(&string, "_ARTIFACT")))
    {
        string++;
        sscanf(string, "%d", &number_items);
        return add_required_item(class_num, depth_num, z_info->k_max + z_info->k_max +item_num, number_items);
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
    return FALSE;
}

/* just used to do a quick sort on the required items array */
int borg_item_cmp(const void * item1, const void * item2)
{
    if (((req_item*)item1)->depth != ((req_item*)item2)->depth)
        return ((req_item*)item1)->depth - ((req_item*)item2)->depth;
    if (((req_item*)item1)->item != ((req_item*)item2)->item)
        return ((req_item*)item1)->item - ((req_item*)item2)->item;
    return ((req_item*)item1)->number - ((req_item*)item2)->number;
}


/*
 * Initialize borg.txt
 */
void init_borg_txt_file(void)
{

    ang_file *fp;

    char buf[1024];
    int i;

    /* Array of borg variables is stored as */
    /* 0 to k_max = items in inventory */
    /* k_max to 2*k_max  = items being worn */
    /* 2*k_max to a_max  = artifacts worn */
    /* 2*k_max + a_max to end of array = Other skills/possessions */
    size_obj = z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX;

    /* note: C_MAKE automaticly 0 inits things */

    for (i = 0; i < MAX_CLASSES; i++)
    {
        C_MAKE(borg_required_item[i], 400, req_item); /* externalize the 400 later */
        n_req[i] = 0;
        C_MAKE(borg_power_item[i], 400, power_item); /* externalize the 400 later */
        n_pwr[i] = 0;
    }
    for (i = 0; i < 999; i++)
    {
        formula[i] = 0;
    }
    C_MAKE(borg_has, size_obj, int);

    /* make some shortcut pointers into the array */
    borg_has_on = borg_has + z_info->k_max;
    borg_artifact = borg_has_on + z_info->k_max;
    borg_skill = borg_artifact + z_info->a_max;

    path_build(buf, 1024, ANGBAND_DIR_USER, "borg.txt");

    /* Open the file */
		fp = file_open(buf, MODE_READ, -1);
	    /*fp = fopen(buf, "r"); */

    /* No file, use defaults*/
    if (!fp)
    {
        /* Complain */
        msg("*****WARNING***** You do not have a proper BORG.TXT file!");
        msg("Make sure BORG.TXT is located in the \\user\\ subdirectory!");
        msg(NULL);

        /* use default values */
        borg_worships_damage = FALSE;
        borg_worships_speed = FALSE;
        borg_worships_hp= FALSE;
        borg_worships_mana = FALSE;
        borg_worships_ac = FALSE;
        borg_worships_gold = FALSE;
        borg_plays_risky = FALSE;
        borg_scums_uniques = TRUE;
        borg_kills_uniques = FALSE;
        borg_uses_swaps = TRUE;
        borg_slow_optimizehome = FALSE;
        borg_stop_dlevel = 128;
        borg_stop_clevel = 55;
		borg_no_deeper = 127;
        borg_stop_king = TRUE;
        borg_uses_calcs = FALSE;
        borg_respawn_winners = FALSE;
        borg_respawn_class = -1;
        borg_respawn_race = -1;
        borg_chest_fail_tolerance = 7;
        borg_delay_factor = 1;
        borg_money_scum_amount = 0;
		borg_self_scum = TRUE;
		borg_lunal_mode = FALSE;
		borg_self_lunal = TRUE;
		borg_verbose = FALSE;
		borg_munchkin_start = FALSE;
		borg_munchkin_level = 12;
		borg_munchkin_depth = 16;
		borg_enchant_limit = 10;

        return;
    }


    /* Parse the file */
/* AJG needed to make this wider so I could read long formulas */
    while (file_getl(fp, buf, sizeof(buf)-1))
    {
        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#')) continue;

        /* Chop the buffer */
        buf[sizeof(buf)-1] = '\0';

        /* Extract the true/false */
        if (prefix(buf, "borg_worships_damage ="))
        {
            if (buf[strlen("borg_worships_damage =")+1] == 'T' ||
                buf[strlen("borg_worships_damage =")+1] == '1' ||
                buf[strlen("borg_worships_damage =")+1] == 't')
                borg_worships_damage=TRUE;
            else
                borg_worships_damage = FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_speed ="))
        {
            if (buf[strlen("borg_worships_speed =")+1] == 'T' ||
                buf[strlen("borg_worships_speed =")+1] == '1' ||
                buf[strlen("borg_worships_speed =")+1] == 't') borg_worships_speed=TRUE;
            else borg_worships_speed = FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_hp ="))
        {
            if (buf[strlen("borg_worships_hp =")+1] == 'T' ||
                buf[strlen("borg_worships_hp =")+1] == '1' ||
                buf[strlen("borg_worships_hp =")+1] == 't') borg_worships_hp=TRUE;
            else borg_worships_hp= FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_mana ="))
        {
            if (buf[strlen("borg_worships_mana =")+1] == 'T' ||
                buf[strlen("borg_worships_mana =")+1] == '1' ||
                buf[strlen("borg_worships_mana =")+1] == 't') borg_worships_mana=TRUE;
            else borg_worships_mana = FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_ac ="))
        {
            if (buf[strlen("borg_worships_ac =")+1] == 'T' ||
                buf[strlen("borg_worships_ac =")+1] == '1' ||
                buf[strlen("borg_worships_ac =")+1] == 't') borg_worships_ac=TRUE;
            else borg_worships_ac= FALSE;
            continue;
        }

        if (prefix(buf, "borg_worships_gold ="))
        {
            if (buf[strlen("borg_worships_gold =")+1] == 'T' ||
                buf[strlen("borg_worships_gold =")+1] == '1' ||
                buf[strlen("borg_worships_gold =")+1] == 't') borg_worships_gold=TRUE;
            else borg_worships_gold= FALSE;
            continue;
        }


        if (prefix(buf, "borg_plays_risky ="))
        {
            if (buf[strlen("borg_plays_risky =")+1] == 'T' ||
                buf[strlen("borg_plays_risky =")+1] == '1' ||
                buf[strlen("borg_plays_risky =")+1] == 't') borg_plays_risky=TRUE;
            else borg_plays_risky = FALSE;
            continue;
        }

        if (prefix(buf, "borg_scums_uniques ="))
        {
            if (buf[strlen("borg_scums_uniques =")+1] == 'T' ||
                buf[strlen("borg_scums_uniques =")+1] == '1' ||
                buf[strlen("borg_scums_uniques =")+1] == 't') borg_scums_uniques=TRUE;
            else borg_scums_uniques = FALSE;
            continue;
        }
        if (prefix(buf, "borg_kills_uniques ="))
        {
            if (buf[strlen("borg_kills_uniques =")+1] == 'T' ||
                buf[strlen("borg_kills_uniques =")+1] == '1' ||
                buf[strlen("borg_kills_uniques =")+1] == 't') borg_kills_uniques=TRUE;
            else borg_kills_uniques = FALSE;
            continue;
        }
        if (prefix(buf, "borg_uses_swaps ="))
        {
            if (buf[strlen("borg_uses_swaps =")+1] == 'T' ||
                buf[strlen("borg_uses_swaps =")+1] == '1' ||
                buf[strlen("borg_uses_swaps =")+1] == 't') borg_uses_swaps=TRUE;
            else borg_uses_swaps = FALSE;
            continue;
        }

        if (prefix(buf, "borg_slow_optimizehome ="))
        {
            if (buf[strlen("borg_slow_optimizehome =")+1] == 'T' ||
                buf[strlen("borg_slow_optimizehome =")+1] == '1' ||
                buf[strlen("borg_slow_optimizehome =")+1] == 't') borg_slow_optimizehome=TRUE;
            else borg_slow_optimizehome = FALSE;

            /* for now always leave as false since its broken */
            borg_slow_optimizehome = FALSE;
            continue;
        }

        if (prefix(buf, "borg_stop_king ="))
        {
            if (buf[strlen("borg_stop_king =")+1] == 'T' ||
                buf[strlen("borg_stop_king =")+1] == '1' ||
                buf[strlen("borg_stop_king =")+1] == 't') borg_stop_king=TRUE;
            else borg_stop_king = FALSE;
            continue;
        }


        if (prefix(buf, "borg_uses_dynamic_calcs ="))
        {
            if (buf[strlen("borg_uses_dynamic_calcs =")+1] == 'T' ||
                buf[strlen("borg_uses_dynamic_calcs =")+1] == '1' ||
                buf[strlen("borg_uses_dynamic_calcs =")+1] == 't') borg_uses_calcs=TRUE;
            else borg_uses_calcs = FALSE;
            continue;
        }

        if (prefix(buf, "borg_respawn_winners ="))
        {
            if (buf[strlen("borg_respawn_winners =")+1] == 'T' ||
                buf[strlen("borg_respawn_winners =")+1] == '1' ||
                buf[strlen("borg_respawn_winners =")+1] == 't') borg_respawn_winners = TRUE;
            else borg_respawn_winners = FALSE;
            continue;
        }

        if (prefix(buf, "borg_lunal_mode ="))
        {
            if (buf[strlen("borg_lunal_mode =")+1] == 'T' ||
                buf[strlen("borg_lunal_mode =")+1] == '1' ||
                buf[strlen("borg_lunal_mode =")+1] == 't') borg_lunal_mode = TRUE;
            else borg_lunal_mode = FALSE;
            continue;
        }

        if (prefix(buf, "borg_self_lunal ="))
        {
            if (buf[strlen("borg_self_lunal =")+1] == 'T' ||
                buf[strlen("borg_self_lunal =")+1] == '1' ||
                buf[strlen("borg_self_lunal =")+1] == 't') borg_self_lunal = TRUE;
            else borg_self_lunal = FALSE;
            continue;
        }

        if (prefix(buf, "borg_self_scum ="))
        {
            if (buf[strlen("borg_self_scum =")+1] == 'T' ||
                buf[strlen("borg_self_scum =")+1] == '1' ||
                buf[strlen("borg_self_scum =")+1] == 't') borg_self_scum = TRUE;
            else borg_self_scum = FALSE;
            continue;
        }

        if (prefix(buf, "borg_verbose ="))
        {
            if (buf[strlen("borg_verbose =")+1] == 'T' ||
                buf[strlen("borg_verbose =")+1] == '1' ||
                buf[strlen("borg_verbose =")+1] == 't') borg_verbose = TRUE;
            else borg_verbose = FALSE;
            continue;
        }

        if (prefix(buf, "borg_munchkin_start ="))
        {
            if (buf[strlen("borg_munchkin_start =")+1] == 'T' ||
                buf[strlen("borg_munchkin_start =")+1] == '1' ||
                buf[strlen("borg_munchkin_start =")+1] == 't') borg_munchkin_start = TRUE;
            else borg_munchkin_start = FALSE;
            continue;
        }

        if (prefix(buf, "borg_munchkin_level ="))
        {
            sscanf(buf+strlen("borg_munchkin_level =")+1, "%d", &borg_munchkin_level);
            if (borg_munchkin_level <= 1) borg_munchkin_level = 1;
            if (borg_munchkin_level >= 50) borg_munchkin_level = 50;
            continue;
        }

        if (prefix(buf, "borg_munchkin_depth ="))
        {
            sscanf(buf+strlen("borg_munchkin_depth =")+1, "%d", &borg_munchkin_depth);
            if (borg_munchkin_depth <= 1) borg_munchkin_depth = 8;
            if (borg_munchkin_depth >= 100) borg_munchkin_depth = 100;
            continue;
        }

        if (prefix(buf, "borg_enchant_limit ="))
        {
            sscanf(buf+strlen("borg_enchant_limit =")+1, "%d", &borg_enchant_limit);
            if (borg_enchant_limit <= 8) borg_enchant_limit = 8;
            if (borg_enchant_limit >= 15) borg_enchant_limit = 15;
            continue;
        }

		/* Extract the integers */
        if (prefix(buf, "borg_respawn_race ="))
        {
            sscanf(buf+strlen("borg_respawn_race =")+1, "%d", &borg_respawn_race);
            continue;
        }
        if (prefix(buf, "borg_respawn_class ="))
        {
            sscanf(buf+strlen("borg_respawn_class =")+1, "%d", &borg_respawn_class);
            continue;
        }
        if (prefix(buf, "borg_dump_level ="))
        {
            sscanf(buf+strlen("borg_dump_level =")+1, "%d", &borg_dump_level);
            continue;
        }

        if (prefix(buf, "borg_save_death ="))
        {
            sscanf(buf+strlen("borg_save_death =")+1, "%d", &borg_save_death);
            continue;
        }

        if (prefix(buf, "borg_stop_clevel ="))
        {
            sscanf(buf+strlen("borg_stop_clevel =")+1, "%d", &borg_stop_clevel);
            continue;
        }
        if (prefix(buf, "borg_stop_dlevel ="))
        {
            sscanf(buf+strlen("borg_stop_dlevel =")+1, "%d", &borg_stop_dlevel);
            continue;
        }
        if (prefix(buf, "borg_no_deeper ="))
        {
            sscanf(buf+strlen("borg_no_deeper =")+1, "%d", &borg_no_deeper);
            continue;
        }
        if (prefix(buf, "borg_chest_fail_tolerance ="))
        {
            sscanf(buf+strlen("borg_chest_fail_tolerance =")+1, "%d", &borg_chest_fail_tolerance);
            continue;
        }
        if (prefix(buf, "borg_delay_factor ="))
        {
            sscanf(buf+strlen("borg_delay_factor =")+1, "%d", &borg_delay_factor);
            if (borg_delay_factor >= 9) borg_delay_factor = 9;
            continue;
        }
        if (prefix(buf, "borg_money_scum_amount ="))
        {
            sscanf(buf+strlen("borg_money_scum_amount =")+1, "%d",  &borg_money_scum_amount);
            continue;
        }
        if (prefix(buf, "REQ"))
        {
            if (!borg_load_requirement(buf+strlen("REQ")))
                borg_note(buf);
            continue;
        }
        if (prefix(buf, "FORMULA"))
        {
            if (!borg_load_formula(buf+strlen("FORMULA")))
                borg_note(buf);
            continue;
        }
        if (prefix(buf, "CND"))
        {
            if (!borg_load_formula(buf+strlen("CND")))
            borg_note(buf);
            continue;
        }
        if (prefix(buf, "POWER"))
        {
            if (!borg_load_power(buf+strlen("POWER")))
                borg_note(buf);

            continue;
        }
    }

    /* Close it */
    file_close(fp);

    for (i = 0; i < MAX_CLASSES; i++)
        qsort(borg_required_item[i], n_req[i], sizeof(req_item), borg_item_cmp);

    /* make sure it continues to run if reset */
    if (borg_respawn_winners) borg_stop_king = FALSE;

    /* Success */
    return;
}

/*
 * Initialize the Borg
 */
void borg_init_9(void)
{
    byte *test;
	int i;


    /*** Hack -- verify system ***/

    /* Message */
    prt("Initializing the Borg... (memory)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();

    /* Mega-Hack -- verify memory */
    C_MAKE(test, 400 * 1024L, byte);
    FREE(test);


    /*** Hack -- initialize some stuff ***/
    C_MAKE(borg_required_item, MAX_CLASSES, req_item*);
    C_MAKE(n_req, MAX_CLASSES, int);
    C_MAKE(borg_power_item, MAX_CLASSES, power_item*);
    C_MAKE(n_pwr, MAX_CLASSES, int);


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
		option_set("cheat_live", TRUE);
	}

	/* We use the original keypress codes */
	option_set("rogue_like_commands", FALSE);

	/* No auto_more */
	option_set("auto_more", FALSE);

    /* We pick up items when we step on them */
	option_set("pickup_always", TRUE);

	/* We do not want verbose messages */
	option_set("pickup_detail", FALSE);

    /* We specify targets by hand */
	option_set("use_old_target", FALSE);

    /* We must pick items up without verification */
	option_set("pickup_inven", TRUE);

	/* Pile symbol '&' confuse the borg */
	option_set("show_piles", FALSE);

	/* We repeat by hand */
    /* always_repeat = FALSE; */

    /* We do not haggle */
	/* auto_haggle = TRUE; */

    /* We need space */
	option_set("show_labels", FALSE);

	/* show_weights = FALSE; */
	option_set("show_flavors", FALSE);


    /* Allow items to stack */
    /* stack_force_notes = TRUE; */
    /* stack_force_costs = TRUE; */


    /* Ignore discounts */
    /* stack_force_costs = TRUE; */

    /* Ignore inscriptions */
    /* stack_force_notes = TRUE; */

    /* Efficiency */
    /* avoid_abort = TRUE; */

    /* Efficiency */
    op_ptr->hitpoint_warn = 0;


   /* The "easy" options confuse the Borg */
	option_set("easy_open", FALSE);
   	option_set("easy_alter", FALSE);
   /* easy_floor = FALSE; */

#ifndef ALLOW_BORG_GRAPHICS
    if (!borg_graphics)
    {
        /* Reset the # and % -- Scan the features */
        for (i = 1; i < z_info->f_max; i++)
        {
            feature_type *f_ptr = &f_info[i];

            /* Skip non-features */
            if (!f_ptr->name) continue;

            /* Switch off "graphics" */
            f_ptr->x_attr[3] = f_ptr->d_attr;
            f_ptr->x_char[3] = f_ptr->d_char;
        }
    }
#endif

#ifdef ALLOW_BORG_GRAPHICS

   init_translate_visuals();

#else /* ALLOW_BORG_GRAPHICS */

#ifdef USE_GRAPHICS
   /* The Borg can't work with graphics on, so switch it off */
   if (use_graphics)
   {
       /* Reset to ASCII mode */
       use_graphics = FALSE;
       arg_graphics = FALSE;

       /* Reset visuals */
       reset_visuals(TRUE);
   }
#endif /* USE_GRAPHICS */

#endif /* ALLOW_BORG_GRAPHICS */

    /*** Redraw ***/
    /* Redraw map */
    p_ptr->redraw |= (PR_MAP);

    /* Redraw everything */
    do_cmd_redraw();
    /*** Various ***/

    /* Message */
    prt("Initializing the Borg... (various)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();


    /*** Cheat / Panic ***/

    /* more cheating */
    borg_cheat_death = FALSE;

    /* set the continous play mode if the game cheat death is on */
    if (op_ptr->opt[OPT_cheat_live]) borg_cheat_death = TRUE;

    /*** Initialize ***/

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
		artifact_type *a_ptr = &a_info[i];

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
    borg_race = p_ptr->race->ridx;

    /*** Hack -- Extract class ***/
    borg_class = p_ptr->class->cidx;

    /*** Hack -- react to race and class ***/

    /* Notice the new race and class */
    prepare_race_class_info();


    /*** All done ***/

    /* Done initialization */
    prt("Initializing the Borg... done.", 0, 0);

    /* Clear line */
    prt("", 0, 0);

    /* Reset the clock */
    borg_t = 10;

    /* Official message */
    borg_note("# Ready...");

    /* Now it is ready */
    initialized = TRUE;
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
    ang_file *borg_map_file;
    char line[DUNGEON_WID + 1];

    borg_item *item;
	player_state *state = &p_ptr->state;
    int i,j;
	int to, itemm;

    s16b m_idx;

    struct store *st_ptr = &stores[7];

    bool *okay;

    char o_name[80];

    /* Allocate the "okay" array */
    C_MAKE(okay, z_info->a_max, bool);

    /* Hack -- drop permissions */
    safe_setuid_drop();

    /* Process the player name */
    for (i = 0; op_ptr->full_name[i]; i++)
    {
        char c = op_ptr->full_name[i];

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
        borg_map_file = file_open(buf2,MODE_WRITE, FTYPE_TEXT);

        /* Failure */
        if (!borg_map_file) msg("Cannot open that file.");
    }
    else if (!ask) borg_map_file = file_open(buf2, MODE_WRITE, FTYPE_TEXT);

    /* Hack -- grab permissions */
    safe_setuid_grab();

   file_putf(borg_map_file, "%s the %s %s, Level %d/%d\n", op_ptr->full_name,
           p_ptr->race->name,
           p_ptr->class->name,
           p_ptr->lev, p_ptr->max_lev);

   file_putf(borg_map_file, "Exp: %lu  Gold: %lu  Turn: %lu\n", (long)(p_ptr->max_exp + (100 * p_ptr->max_depth)), (long)p_ptr->au, (long)turn);
   file_putf(borg_map_file, "Killed on level: %d (max. %d) by %s\n\n", p_ptr->depth, p_ptr->max_depth, p_ptr->died_from);
   file_putf(borg_map_file, "Borg Compile Date: %s\n", borg_engine_date);

    for (i = 0; i < DUNGEON_HGT; i++)
    {
        for (j = 0; j < DUNGEON_WID; j++)
        {
            char ch;

            borg_grid *ag= &borg_grids[i][j];
            m_idx = cave->m_idx[i][j];


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
                borg_take *take = &borg_takes[ag->take];
                object_kind *k_ptr = &k_info[take->k_idx];
                ch = k_ptr->d_char;
            }

            /* UnKnown Monsters */
            if (m_idx)
            {
                ch = '&';
            }

            /* Known Monsters */
            if (ag->kill)
            {
                borg_kill *kill = &borg_kills[ag->kill];
                monster_race *r_ptr = &r_info[kill->r_idx];
                ch = r_ptr->d_char;
            }


            /* The Player */
            if ((i == c_y) && (j == c_x)) ch = '@';

            line[j] = ch;
        }
        /* terminate the line */
        line[j++] = '\0';

        file_putf(borg_map_file, "%s\n", line);
    }


    /* Known/Seen monsters */
    for (i = 1; i < borg_kills_nxt; i++)
    {
        borg_kill *kill = &borg_kills[i];

        /* Skip dead monsters */
        if (!kill->r_idx) continue;

        /* Note */
        file_putf(borg_map_file,"monster '%s' (%d) at (%d,%d) speed:%d \n",
                         (r_info[kill->r_idx].name), kill->r_idx,
                         kill->y, kill->x, kill->speed);
    }

    /*** Dump the last few messages ***/
    i = messages_num();
    if (i > 250) i = 250;
    file_putf(borg_map_file, "\n\n  [Last Messages]\n\n");
    while (i-- >0)
    {
        cptr msg  = message_str((s16b)i);

        /* Eliminate some lines */
        if (prefix(msg, "# Matched")
        ||  prefix(msg, "# There is")
        ||  prefix(msg, "# Tracking")
        ||  prefix(msg, "# MISS_BY:")
        ||  prefix(msg, "# HIT_BY:")
        ||  prefix(msg, "> "))
            continue;

        file_putf(borg_map_file, "%s\n", msg);
    }

    /*** Player Equipment ***/
    file_putf(borg_map_file, "\n\n  [Character Equipment]\n\n");
    for (i = INVEN_WIELD; i < QUIVER_END; i++)
    {
        object_desc(o_name, sizeof(o_name), &p_ptr->inventory[i], ODESC_FULL);
        file_putf(borg_map_file, "%c) %s\n",
                index_to_label(i), o_name);
    }

    file_putf(borg_map_file, "\n\n");


    /* Dump the inventory */
    file_putf(borg_map_file, "  [Character Inventory]\n\n");
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
		borg_item *item = &borg_items[i];

        file_putf(borg_map_file, "%c) %s\n",
                index_to_label(i), item->desc);
    }
    file_putf(borg_map_file, "\n\n");


    /* Dump the Home (page 1) */
    file_putf(borg_map_file, "  [Home Inventory (page 1)]\n\n");
    for (i = 0; i < 12; i++)
    {
        object_desc(o_name, sizeof(o_name), &st_ptr->stock[i], ODESC_FULL);
        file_putf(borg_map_file, "%c) %s\n", I2A(i%12), o_name);
    }
    file_putf(borg_map_file, "\n\n");

    /* Dump the Home (page 2) */
    file_putf(borg_map_file, "  [Home Inventory (page 2)]\n\n");
    for (i = 12; i < 24; i++)
    {
        object_desc(o_name, sizeof(o_name), &st_ptr->stock[i], ODESC_FULL);
        file_putf(borg_map_file, "%c) %s\n", I2A(i%12), o_name);
    }
    file_putf(borg_map_file, "\n\n");

    /* Write swap info */
    if (borg_uses_swaps)
    {
        file_putf(borg_map_file, "  [Swap info]\n\n");
        item = &borg_items[weapon_swap];
        file_putf(borg_map_file,"Swap Weapon:  %s\n", item->desc);
        item = &borg_items[armour_swap];
        file_putf(borg_map_file,"Swap Armour:  %s", item->desc);
        file_putf(borg_map_file, "\n\n");
    }
    file_putf(borg_map_file, "   [Player State at Death] \n\n");

    /* Dump the player state */
    file_putf(borg_map_file,  format("Current speed: %d. \n", borg_skill[BI_SPEED]));

    if (p_ptr->timed[TMD_BLIND])
    {
        file_putf(borg_map_file,  "You cannot see.\n");
    }
    if (p_ptr->timed[TMD_CONFUSED])
    {
        file_putf(borg_map_file,  "You are confused.\n");
    }
    if (p_ptr->timed[TMD_AFRAID])
    {
        file_putf(borg_map_file,  "You are terrified.\n");
    }
    if (p_ptr->timed[TMD_CUT])
    {
        file_putf(borg_map_file,  "You are bleeding.\n");
    }
    if (p_ptr->timed[TMD_STUN])
    {
        file_putf(borg_map_file,  "You are stunned.\n");
    }
    if (p_ptr->timed[TMD_POISONED])
    {
        file_putf(borg_map_file,  "You are poisoned.\n");
    }
    if (p_ptr->timed[TMD_IMAGE])
    {
        file_putf(borg_map_file,  "You are hallucinating.\n");
    }
    if (check_state(p_ptr, OF_AGGRAVATE, p_ptr->state.flags))
    {
        file_putf(borg_map_file,  "You aggravate monsters.\n");
    }
    if (p_ptr->timed[TMD_BLESSED])
    {
        file_putf(borg_map_file,  "You feel rightous.\n");
    }
    if (p_ptr->timed[TMD_HERO])
    {
        file_putf(borg_map_file,  "You feel heroic.\n");
    }
    if (p_ptr->timed[TMD_SHERO])
    {
        file_putf(borg_map_file,  "You are in a battle rage.\n");
    }
    if (p_ptr->timed[TMD_PROTEVIL])
    {
        file_putf(borg_map_file,  "You are protected from evil.\n");
    }
    if (p_ptr->timed[TMD_SHIELD])
    {
        file_putf(borg_map_file,  "You are protected by a mystic shield.\n");
    }
    if (p_ptr->timed[TMD_INVULN])
    {
        file_putf(borg_map_file,  "You are temporarily invulnerable.\n");
    }
    if (p_ptr->timed[TMD_CONFUSED])
    {
        file_putf(borg_map_file,  "Your hands are glowing dull red.\n");
    }
    if (p_ptr->word_recall)
    {
        file_putf(borg_map_file,  format("You will soon be recalled.  (%d turns)\n", p_ptr->word_recall));
    }
    if (p_ptr->timed[TMD_OPP_FIRE])
    {
        file_putf(borg_map_file,  format("You resist fire exceptionally well.\n"));
    }
    if (p_ptr->timed[TMD_OPP_ACID])
    {
        file_putf(borg_map_file,  format("You resist acid exceptionally well.\n"));
    }
    if (p_ptr->timed[TMD_OPP_ELEC])
    {
        file_putf(borg_map_file,  format("You resist elec exceptionally well.\n"));
    }
    if (p_ptr->timed[TMD_OPP_COLD])
    {
        file_putf(borg_map_file,  format("You resist cold exceptionally well.\n"));
    }
    if (p_ptr->timed[TMD_OPP_POIS])
    {
        file_putf(borg_map_file,  format("You resist poison exceptionally well.\n"));
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
    if ((turn % (10L * TOWN_DAWN)) < ((10L * TOWN_DAWN) / 2))
		file_putf(borg_map_file, "It is daytime in town.\n");
    else file_putf(borg_map_file, "It is night-time in town.\n");
    file_putf(borg_map_file, "\n\n");

    file_putf(borg_map_file, "borg_uses_swaps; %d\n", borg_uses_swaps);
    file_putf(borg_map_file, "borg_worships_damage; %d\n", borg_worships_damage);
    file_putf(borg_map_file, "borg_worships_speed; %d\n", borg_worships_speed);
    file_putf(borg_map_file, "borg_worships_hp; %d\n", borg_worships_hp);
    file_putf(borg_map_file, "borg_worships_mana; %d\n",borg_worships_mana);
    file_putf(borg_map_file, "borg_worships_ac; %d\n",borg_worships_ac);
    file_putf(borg_map_file, "borg_worships_gold; %d\n",borg_worships_gold);
    file_putf(borg_map_file, "borg_plays_risky; %d\n",borg_plays_risky);
    file_putf(borg_map_file, "borg_slow_optimizehome; %d\n\n",borg_slow_optimizehome);
    file_putf(borg_map_file, "borg_scumming_pots; %d\n\n",borg_scumming_pots);
    file_putf(borg_map_file, "\n\n");


    /* Dump the spells */
    if (p_ptr->class->spell_book)
    {
        file_putf(borg_map_file,"\n\n   [ Spells ] \n\n");
        file_putf(borg_map_file,"Name                           Legal Times cast\n");
        for (i = 0; i < 9; i++ )
        {
            for (j = 0; j < 8; j++)
            {
                borg_magic *as = &borg_magics[i][j];
                cptr legal;
                int failpercent =0;

                if (as->level <99)
                {
                    if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
                    {
                        legal = (borg_prayer_legal(i, j) ? "Yes" : "No ");
	                    failpercent = (borg_prayer_fail_rate( i,  j));
                    }
                    else
                    {
                        legal = (borg_spell_legal(i, j) ? "Yes" : "No ");
	                    failpercent = (borg_spell_fail_rate(i, j));
                    }

                    file_putf(borg_map_file,"%-30s   %s   %d   fail:%d \n",as->name, legal, (long)as->times, failpercent);
                }
            }
            file_putf(borg_map_file,"\n");
        }
    }

	/* Dump the borg_skill[] information */
    itemm = z_info->k_max + z_info->k_max + z_info->a_max;
    to = z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX;
    for (;itemm < to; itemm++)
    {
         file_putf(borg_map_file,"skill %d (%s) value= %d.\n",itemm,
                      prefix_pref[itemm -
                      z_info->k_max -
                      z_info->k_max -
                      z_info->a_max], borg_has[itemm]);
	}

#if 0
    /*** Dump the Uniques and Artifact Lists ***/

    /* Scan the artifacts */
    for (k = 0; k < z_info->a_max; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* Default */
        okay[k] = FALSE;

        /* Skip "empty" artifacts */
        if (!a_ptr->name) continue;

        /* Skip "uncreated" artifacts */
        if (!a_ptr->cur_num) continue;

        /* Assume okay */
        okay[k] = TRUE;
    }

    /* Check the dungeon */
    for (y = 0; y < DUNGEON_HGT; y++)
    {
        for (x = 0; x < DUNGEON_WID; x++)
        {
            s16b this_o_idx, next_o_idx = 0;

            /* Scan all objects in the grid */
            for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
            {
                object_type *o_ptr;

                /* Get the object */
                o_ptr = &o_list[this_o_idx];

                /* Get the next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Ignore non-artifacts */
                if (!artifact_p(o_ptr)) continue;

                /* Ignore known items */
                if (object_is_known(o_ptr)) continue;

                /* Note the artifact */
                okay[o_ptr->name1] = FALSE;
            }
        }
    }

    /* Check the inventory and equipment */
    for (i = 0; i < INVEN_TOTAL; i++)
    {
        object_type *o_ptr = &p_ptr->inventory[i];

        /* Ignore non-objects */
        if (!o_ptr->k_idx) continue;

        /* Ignore non-artifacts */
        if (!artifact_p(o_ptr)) continue;

        /* Ignore known items */
        if (object_is_known(o_ptr)) continue;

        /* Note the artifact */
        okay[o_ptr->name1] = FALSE;
    }

    file_putf(borg_map_file, "\n\n");


    /* Hack -- Build the artifact name */
    file_putf(borg_map_file, "   [Artifact Info] \n\n");

    /* Scan the artifacts */
    for (k = 0; k < z_info->a_max; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* List "dead" ones */
        if (!okay[k]) continue;

        /* Paranoia */
        strcpy(o_name, "Unknown Artifact");

        /* Obtain the base object type */
        z = borg_lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Real object */
        if (z)
        {
            object_type *i_ptr;
            object_type object_type_body;

            /* Get local object */
            i_ptr = &object_type_body;

            /* Create fake object */
            object_prep(i_ptr, z);

            /* Make it an artifact */
            i_ptr->name1 = k;

            /* Describe the artifact */
            object_desc_spoil(o_name, sizeof(o_name), i_ptr, FALSE, 0);
        }

        /* Hack -- Build the artifact name */
        file_putf(borg_map_file, "The %s\n", o_name);
    }

    /* Free the "okay" array */
    FREE(okay);
    file_putf(borg_map_file, "\n\n");

 /* Display known uniques
  *
  * Note that the player ghosts are ignored.  XXX XXX XXX
  */
    /* Allocate the "who" array */
    C_MAKE(who, z_info->r_max, u16b);

    /* Collect matching monsters */
    for (i = 1, n = 0; i < z_info->r_max; i++)
    {
        monster_race *r_ptr = &r_info[i];
        monster_lore *l_ptr = &l_list[i];

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
        monster_race *r_ptr = &r_info[who[i]];
        bool dead = (r_ptr->max_num == 0);

        /* Print a message */
        file_putf(borg_map_file, "%s is %s\n",
                (r_ptr->name),
                (dead ? "dead" : "alive"));
    }

    /* Free the "who" array */
    FREE(who);

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
    if (borg_save_death  &&
        (borg_skill[BI_CLEVEL] >= borg_dump_level ||
        strstr(p_ptr->died_from, "starvation")))
    {
        memcpy(svSavefile, savefile, sizeof(savefile));
        /* Process the player name */
        for (i = 0; op_ptr->full_name[i]; i++)
        {
            char c = op_ptr->full_name[i];

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
        svSavefile2[i]  = 0;

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
  term *old = Term;

  /* Unused */
  if (!angband_term[j]) continue;

  /* Check for borg status term */
  if (op_ptr->window_flag[j] & (PW_BORG_2))
  {
   byte attr;

   /* Activate */
   Term_activate(angband_term[j]);

   /* Display what resists the borg (thinks he) has */
   Term_putstr(5, 0, -1,TERM_WHITE, "RESISTS");

   /* Basic four */
   attr = TERM_SLATE;
   if (borg_skill[BI_RACID]) attr = TERM_BLUE;
   if (borg_skill[BI_TRACID]) attr = TERM_GREEN;
   if (borg_skill[BI_IACID]) attr = TERM_WHITE;
   Term_putstr(1, 1, -1, attr, "Acid");

   attr = TERM_SLATE;
   if (borg_skill[BI_RELEC]) attr = TERM_BLUE;
   if (borg_skill[BI_TRELEC]) attr = TERM_GREEN;
   if (borg_skill[BI_IELEC]) attr = TERM_WHITE;
   Term_putstr(1, 2, -1, attr, "Elec");

   attr = TERM_SLATE;
   if (borg_skill[BI_RFIRE]) attr = TERM_BLUE;
   if (borg_skill[BI_TRFIRE]) attr = TERM_GREEN;
   if (borg_skill[BI_IFIRE]) attr = TERM_WHITE;
   Term_putstr(1, 3, -1, attr, "Fire");

   attr = TERM_SLATE;
   if (borg_skill[BI_RCOLD]) attr = TERM_BLUE;
   if (borg_skill[BI_TRCOLD]) attr = TERM_GREEN;
   if (borg_skill[BI_ICOLD]) attr = TERM_WHITE;
   Term_putstr(1, 4, -1, attr, "Cold");

   /* High resists */
   attr = TERM_SLATE;
   if (borg_skill[BI_RPOIS]) attr = TERM_BLUE;
   if (borg_skill[BI_TRPOIS]) attr = TERM_GREEN;
   Term_putstr(1, 5, -1, attr, "Pois");

   if (borg_skill[BI_RFEAR]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 6, -1, attr, "Fear");

   if (borg_skill[BI_RLITE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 7, -1, attr, "Lite");

   if (borg_skill[BI_RDARK]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 8, -1, attr, "Dark");

   if (borg_skill[BI_RBLIND]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 1, -1, attr, "Blind");

   if (borg_skill[BI_RCONF]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 2, -1, attr, "Confu");

   if (borg_skill[BI_RSND]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 3, -1, attr, "Sound");

   if (borg_skill[BI_RSHRD]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 4, -1, attr, "Shard");

   if (borg_skill[BI_RNXUS]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 5, -1, attr, "Nexus");

   if (borg_skill[BI_RNTHR]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 6, -1, attr, "Nethr");

   if (borg_skill[BI_RKAOS]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 7, -1, attr, "Chaos");

   if (borg_skill[BI_RDIS]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(6, 8, -1, attr, "Disen");

   /* Other abilities */
   if (borg_skill[BI_SDIG]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 1, -1, attr, "S.Dig");

   if (borg_skill[BI_FEATH]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 2, -1, attr, "Feath");

   if (borg_skill[BI_LIGHT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 3, -1, attr, "PLite");

   if (borg_skill[BI_REG]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 4, -1, attr, "Regen");

   if (borg_skill[BI_ESP]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 5, -1, attr, "Telep");

   if (borg_skill[BI_SINV]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 6, -1, attr, "Invis");

   if (borg_skill[BI_FRACT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 7, -1, attr, "FrAct");

   if (borg_skill[BI_HLIFE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(12, 8, -1, attr, "HLife");

   /* Display the slays */
   Term_putstr(5, 10, -1,TERM_WHITE, "Weapon Slays:");

   if (borg_skill[BI_WS_ANIMAL]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 11, -1, attr, "Animal");

   if (borg_skill[BI_WS_EVIL]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(8, 11, -1, attr, "Evil");

   if (borg_skill[BI_WS_UNDEAD]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(15, 11, -1, attr, "Undead");

   if (borg_skill[BI_WS_DEMON]) attr = TERM_BLUE;
   if (borg_skill[BI_WK_DEMON]) attr = TERM_GREEN;
   else attr = TERM_SLATE;
   Term_putstr(22, 11, -1, attr, "Demon");

   if (borg_skill[BI_WS_ORC]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 12, -1, attr, "Orc");

   if (borg_skill[BI_WS_TROLL]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(8, 12, -1, attr, "Troll");

   if (borg_skill[BI_WS_GIANT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(15, 12, -1, attr, "Giant");

   if (borg_skill[BI_WS_DRAGON]) attr = TERM_BLUE;
   if (borg_skill[BI_WK_DRAGON]) attr = TERM_GREEN;
   else attr = TERM_SLATE;
   Term_putstr(22, 12, -1, attr, "Dragon");

   if (borg_skill[BI_WB_ACID]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(1, 13, -1, attr, "Acid");

   if (borg_skill[BI_WB_COLD]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(8, 13, -1, attr, "Cold");

   if (borg_skill[BI_WB_ELEC]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(15, 13, -1, attr, "Elec");

   if (borg_skill[BI_WB_FIRE]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(22, 13, -1, attr, "Fire");


   /* Display the Concerns */
   Term_putstr(36, 10, -1,TERM_WHITE, "Concerns:");

   if (borg_wearing_cursed) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(29, 11, -1, attr, "Cursed");

   if (borg_skill[BI_ISWEAK]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(36, 11, -1, attr, "Weak");

   if (borg_skill[BI_ISPOISONED]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(43, 11, -1, attr, "Poison");

   if (borg_skill[BI_ISCUT]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(29, 12, -1, attr, "Cut");

   if (borg_skill[BI_ISSTUN]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(36, 12, -1, attr, "Stun");

   if (borg_skill[BI_ISCONFUSED]) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(43, 12, -1, attr, "Confused");

   if (goal_fleeing) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(29, 13, -1, attr, "Goal Fleeing");

   if (borg_no_rest_prep > 0) attr = TERM_BLUE;
   else attr = TERM_SLATE;
   Term_putstr(43, 13, -1, attr, "No Resting");

   /* Display the Time */
   Term_putstr(60, 10, -1,TERM_WHITE, "Time:");

   Term_putstr(54, 11, -1, TERM_SLATE, "This Level         ");
   Term_putstr(65, 11, -1, TERM_WHITE, format("%d",borg_t - borg_began));

   Term_putstr(54, 12, -1, TERM_SLATE, "Since Town         ");
   Term_putstr(65, 12, -1, TERM_WHITE, format("%d",borg_time_town + (borg_t - borg_began)));

   Term_putstr(54, 13, -1, TERM_SLATE, "This Panel         ");
   Term_putstr(65, 13, -1, TERM_WHITE, format("%d",time_this_panel));


   /* Sustains */
   Term_putstr(19, 0, -1, TERM_WHITE, "Sustains");

   if (borg_skill[BI_SSTR]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 1, -1, attr, "STR");

   if (borg_skill[BI_SINT]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 2, -1, attr, "INT");

   if (borg_skill[BI_SWIS]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 3, -1, attr, "WIS");

   if (borg_skill[BI_SDEX]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 4, -1, attr, "DEX");

   if (borg_skill[BI_SCON]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 5, -1, attr, "CON");

   if (borg_skill[BI_SCHR]) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(21, 6, -1, attr, "CHR");


   /* Temporary effects */
   Term_putstr(28, 0, -1, TERM_WHITE, "Temp Effects");

   if (borg_prot_from_evil) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 1, -1, attr, "Prot. Evil");

   if (borg_hero) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 2, -1, attr, "Heroism");

   if (borg_berserk) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 3, -1, attr, "Berserk");

   if (borg_shield) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 4, -1, attr, "Shielded");

   if (borg_bless) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(28, 5, -1, attr, "Blessed");

    if (borg_speed) attr = TERM_WHITE;
    else attr = TERM_SLATE;
    Term_putstr(28, 6, -1, attr, "Fast");

    if (borg_see_inv >= 1) attr = TERM_WHITE;
    else attr = TERM_SLATE;
    Term_putstr(28, 6, -1, attr, "See Inv");

   /* Temporary effects */
   Term_putstr(42, 0, -1, TERM_WHITE, "Level Information");

   if (vault_on_level) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 1, -1, attr, "Vault on level");

   if (unique_on_level) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 2, -1, attr, "Unique on level");
   if (unique_on_level) Term_putstr(58, 2, -1, attr, format("(%s)",
                       r_info[unique_on_level].name));
   else Term_putstr(58, 2, -1, attr, "                                                ");

   if (scaryguy_on_level) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 3, -1, attr, "Scary Guy on level");

   if (breeder_level) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 4, -1, attr,"Breeder level (closing doors)");

   if (borg_kills_summoner != -1) attr = TERM_WHITE;
   else attr = TERM_SLATE;
   Term_putstr(42, 5, -1, attr,"Summoner very close (AS-Corridor)");

    /* level preparedness */
   attr = TERM_SLATE;
   Term_putstr(42, 6, -1, attr,"Reason for not diving:");
   attr = TERM_WHITE;
   Term_putstr(64, 6, -1, attr,format("%s                              ", borg_prepared(borg_skill[BI_MAXDEPTH]+1)));

   attr = TERM_SLATE;
   Term_putstr(42, 7, -1, attr,"Scumming: not active                          ");
   if (borg_money_scum_amount != 0)
   {
	   attr = TERM_WHITE;
	   Term_putstr(42, 7, -1, attr,format("Scumming: $%d                  ", borg_money_scum_amount));
	   /* Term_putstr(42, 7, -1, attr,format("Scumming:$%d,%s                  ", borg_money_scum_amount, borg_money_scum_item)); */
   }
   attr = TERM_SLATE;
   Term_putstr(42, 8, -1, attr,"Maximal Depth:");
   attr = TERM_WHITE;
   Term_putstr(56, 8, -1, attr,format("%d    ", borg_skill[BI_MAXDEPTH]));

    /* Important endgame information */
   if (borg_skill[BI_MAXDEPTH] >= 50) /* 85 */
   {
       Term_putstr(5, 15, -1,TERM_WHITE, "Important Deep Events:");

       attr = TERM_SLATE;
       Term_putstr(1, 16, -1, attr, "Home *Heal*:        ");
       attr = TERM_WHITE;
       Term_putstr(13, 16, -1, attr, format("%d   ",num_ezheal));

       attr = TERM_SLATE;
       Term_putstr(1, 17, -1, attr, "Home Heal:        ");
       attr = TERM_WHITE;
       Term_putstr(11, 17, -1, attr, format("%d   ",num_heal));

       attr = TERM_SLATE;
       Term_putstr(1, 18, -1, attr, "Home Life:        ");
       attr = TERM_WHITE;
       Term_putstr(11, 18, -1, attr, format("%d   ",num_life));

	   attr = TERM_SLATE;
       Term_putstr(1, 19, -1, attr, "Res_Mana:        ");
       attr = TERM_WHITE;
       Term_putstr(11, 19, -1, attr, format("%d   ",num_mana));

       if (morgoth_on_level)  attr = TERM_BLUE;
       else attr = TERM_SLATE;
       Term_putstr(1, 20, -1, attr, format("Morgoth on Level.  Last seen:%d       ", borg_t - borg_t_morgoth));

       if (borg_morgoth_position)  attr = TERM_BLUE;
       else attr = TERM_SLATE;
       if (borg_needs_new_sea) attr = TERM_WHITE;
       Term_putstr(1, 21, -1, attr, "Sea of Runes.");

       if (borg_ready_morgoth)  attr = TERM_BLUE;
       else attr = TERM_SLATE;
       Term_putstr(1, 22, -1, attr, "Ready for Morgoth.");
   }
   else
   {
       Term_putstr(5, 15, -1,TERM_WHITE, "                        ");

       attr = TERM_SLATE;
       Term_putstr(1, 16, -1, attr, "                    ");
       attr = TERM_WHITE;
       Term_putstr(10, 16, -1, attr, format("       ",num_ezheal));

       attr = TERM_SLATE;
       Term_putstr(1, 17, -1, attr, "                    ");
       attr = TERM_WHITE;
       Term_putstr(10, 17, -1, attr, format("       ",num_life));

	   attr = TERM_SLATE;
       Term_putstr(1, 18, -1, attr, "                    ");
       attr = TERM_WHITE;
       Term_putstr(11, 18, -1, attr, format("       ",num_heal));

       attr = TERM_SLATE;
       Term_putstr(1, 19, -1, attr, "                   ");
       attr = TERM_WHITE;
       Term_putstr(11, 19, -1, attr, format("       ",num_mana));
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
        auto_play = FALSE;
        keep_playing = TRUE;
        cmd = 'z';
    }
    else
    {

#endif /* BABLOS */

    /* Get a "Borg command", or abort */
#ifdef XSCREENSAVER
	if (auto_start_borg == FALSE)
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
        Term_putstr(2, i, -1, TERM_WHITE, "Command 'z' activates the Borg.");
        Term_putstr(42, i++, -1, TERM_WHITE, "Command 'u' updates the Borg.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'x' steps the Borg.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'f' modifies the normal flags.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'c' modifies the cheat flags.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'l' activates a log file.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 's' activates search mode.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'i' displays grid info.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'g' displays grid feature.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'a' displays avoidances.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'k' displays monster info.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 't' displays object info.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '%' displays targetting flow.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '#' displays danger grid.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '_' Regional Fear info.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'p' Borg Power.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '1' change max depth.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '2' level prep info.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '3' Feature of grid.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '!' Time.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '@' Borg LOS.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'w' My Swap Weapon.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'q' Auto stop on level.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'v' Version stamp.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'd' Dump spell info.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'h' Borg_Has function.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command '$' Reload Borg.txt.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'y' Last 75 steps.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'm' money Scum.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command '^' Flow Pathway.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'R' Respawn Borg.");
        Term_putstr(42, i, -1, TERM_WHITE, "Command 'o' Object Flags.");
        Term_putstr(2, i++, -1, TERM_WHITE, "Command 'r' Restock Stores.");

        /* Prompt for key */
        msg("Commands: ");
        msg(NULL);

        /* Restore the screen */
        Term_load();

        /* Done */
        return;
    }


    /* Hack -- force initialization */
    if (!initialized) borg_init_9();

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
	            FREE(borg_required_item[j]); /* externalize the 400 later */
	            FREE(borg_power_item[j]); /* externalize the 400 later */
			}
            FREE(borg_required_item); /* externalize the 400 later */
            FREE(borg_power_item); /* externalize the 400 later */
            FREE(borg_has);
			for (j = 0; j < 1000; j++)
			{
				if (formula[j])
				{
					FREE(formula[j]);
					formula[j] = 0;
				}
			}
		    FREE(n_req);
		    FREE(n_pwr);

    		C_MAKE(borg_required_item, MAX_CLASSES, req_item*);
    		C_MAKE(n_req, MAX_CLASSES, int);
    		C_MAKE(borg_power_item, MAX_CLASSES, power_item*);
    		C_MAKE(n_pwr, MAX_CLASSES, int);

            init_borg_txt_file();
            borg_note("# Ready...");
            break;
        }
        /* Command: Activate */
        case 'z':
        case 'Z':
        {
            /* Activate */
            borg_active = TRUE;

            /* Reset cancel */
            borg_cancel = FALSE;

            /* Step forever */
            borg_step = 0;

            /* need to check all stats */
            my_need_stat_check[0] = TRUE;
            my_need_stat_check[1] = TRUE;
            my_need_stat_check[2] = TRUE;
            my_need_stat_check[3] = TRUE;
            my_need_stat_check[4] = TRUE;
            my_need_stat_check[5] = TRUE;

            /* Allowable Cheat -- Obtain "recall" flag */
            goal_recalling = p_ptr->word_recall * 1000;

		    /* Allowable Cheat -- Obtain "prot_from_evil" flag */
		    borg_prot_from_evil = (p_ptr->timed[TMD_PROTEVIL] ? TRUE : FALSE);
		    /* Allowable Cheat -- Obtain "speed" flag */
		    borg_speed = (p_ptr->timed[TMD_FAST] ? TRUE : FALSE);
		    /* Allowable Cheat -- Obtain "resist" flags */
		    borg_skill[BI_TRACID] = (p_ptr->timed[TMD_OPP_ACID] ? TRUE : FALSE);
		    borg_skill[BI_TRELEC] = (p_ptr->timed[TMD_OPP_ELEC] ? TRUE : FALSE);
		    borg_skill[BI_TRFIRE] = (p_ptr->timed[TMD_OPP_FIRE] ? TRUE : FALSE);
		    borg_skill[BI_TRCOLD] = (p_ptr->timed[TMD_OPP_COLD] ? TRUE : FALSE);
		    borg_skill[BI_TRPOIS] = (p_ptr->timed[TMD_OPP_POIS] ? TRUE : FALSE);
		    borg_bless = (p_ptr->timed[TMD_BLESSED] ? TRUE : FALSE);
		    borg_shield = (p_ptr->timed[TMD_SHIELD] ? TRUE : FALSE);
		    borg_hero = (p_ptr->timed[TMD_HERO] ? TRUE : FALSE);
		    borg_berserk = (p_ptr->timed[TMD_SHERO] ? TRUE : FALSE);
		    if (p_ptr->timed[TMD_SINVIS]) borg_see_inv = 10000;

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
            /* Activate */
            borg_active = TRUE;

            /* Immediate cancel */
            borg_cancel = TRUE;

            /* Step forever */
            borg_step = 0;

            /* Allowable Cheat -- Obtain "recall" flag */
            goal_recalling = p_ptr->word_recall * 1000;

		    /* Allowable Cheat -- Obtain "prot_from_evil" flag */
		    borg_prot_from_evil = (p_ptr->timed[TMD_PROTEVIL] ? TRUE : FALSE);
		    /* Allowable Cheat -- Obtain "speed" flag */
		    borg_speed = (p_ptr->timed[TMD_FAST] ? TRUE : FALSE);
		    /* Allowable Cheat -- Obtain "resist" flags */
		    borg_skill[BI_TRACID] = (p_ptr->timed[TMD_OPP_ACID] ? TRUE : FALSE);
		    borg_skill[BI_TRELEC] = (p_ptr->timed[TMD_OPP_ELEC] ? TRUE : FALSE);
		    borg_skill[BI_TRFIRE] = (p_ptr->timed[TMD_OPP_FIRE] ? TRUE : FALSE);
		    borg_skill[BI_TRCOLD] = (p_ptr->timed[TMD_OPP_COLD] ? TRUE : FALSE);
		    borg_skill[BI_TRPOIS] = (p_ptr->timed[TMD_OPP_POIS] ? TRUE : FALSE);
		    borg_bless = (p_ptr->timed[TMD_BLESSED] ? TRUE : FALSE);
		    borg_shield = (p_ptr->timed[TMD_SHIELD] ? TRUE : FALSE);
		    borg_hero = (p_ptr->timed[TMD_HERO] ? TRUE : FALSE);
		    borg_berserk = (p_ptr->timed[TMD_SHERO] ? TRUE : FALSE);
		    if (p_ptr->timed[TMD_SINVIS]) borg_see_inv = 10000;

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
            /* Activate */
            borg_active = TRUE;

            /* Reset cancel */
            borg_cancel = FALSE;

            /* Step N times */
			borg_step  = (p_ptr->command_arg ? p_ptr->command_arg : 1);

            /* need to check all stats */
            my_need_stat_check[0] = TRUE;
            my_need_stat_check[1] = TRUE;
            my_need_stat_check[2] = TRUE;
            my_need_stat_check[3] = TRUE;
            my_need_stat_check[4] = TRUE;
            my_need_stat_check[5] = TRUE;

		    /* Allowable Cheat -- Obtain "prot_from_evil" flag */
		    borg_prot_from_evil = (p_ptr->timed[TMD_PROTEVIL] ? TRUE : FALSE);
		    /* Allowable Cheat -- Obtain "speed" flag */
		    borg_speed = (p_ptr->timed[TMD_FAST] ? TRUE : FALSE);
		    /* Allowable Cheat -- Obtain "resist" flags */
		    borg_skill[BI_TRACID] = (p_ptr->timed[TMD_OPP_ACID] ? TRUE : FALSE);
		    borg_skill[BI_TRELEC] = (p_ptr->timed[TMD_OPP_ELEC] ? TRUE : FALSE);
		    borg_skill[BI_TRFIRE] = (p_ptr->timed[TMD_OPP_FIRE] ? TRUE : FALSE);
		    borg_skill[BI_TRCOLD] = (p_ptr->timed[TMD_OPP_COLD] ? TRUE : FALSE);
		    borg_skill[BI_TRPOIS] = (p_ptr->timed[TMD_OPP_POIS] ? TRUE : FALSE);
		    borg_bless = (p_ptr->timed[TMD_BLESSED] ? TRUE : FALSE);
		    borg_shield = (p_ptr->timed[TMD_SHIELD] ? TRUE : FALSE);
		    borg_hero = (p_ptr->timed[TMD_HERO] ? TRUE : FALSE);
		    borg_berserk = (p_ptr->timed[TMD_SHERO] ? TRUE : FALSE);
		    if (p_ptr->timed[TMD_SINVIS]) borg_see_inv = 10000;

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
            if (!get_com("Borg command: Toggle Cheat: (d/i/e/s/p)", &cmd))
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
				borg_note(format("Nasty: [%c] Count: %d, limited: %d",borg_nasties[i],
					borg_nasties_count[i],borg_nasties_limit[i]));
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
                strcpy(borg_match, "");

                /* Message */
                msg("Borg Match String de-activated.");
            }
            break;
        }

        /* Command: check Grid "feature" flags */
        case 'g':
        {
            int x, y;

            u16b low, high = 0;

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
                case 'i': low = high = FEAT_INVIS; break;
                case ';': low = high = FEAT_GLYPH; break;
                case ',': low = high = FEAT_OPEN; break;
                case 'x': low = high = FEAT_BROKEN; break;
                case '<': low = high = FEAT_LESS; break;
                case '>': low = high = FEAT_MORE; break;
                case '@': low = FEAT_SHOP_HEAD;
                          high = FEAT_SHOP_TAIL;
                           break;
                case '^': low = FEAT_TRAP_HEAD;
                          high = FEAT_TRAP_TAIL;
                          break;
                case '+': low  = FEAT_DOOR_HEAD;
                          high = FEAT_DOOR_TAIL; break;
                case 's': low = high = FEAT_SECRET; break;
                case ':': low = high = FEAT_RUBBLE; break;
                case 'm': low = high = FEAT_MAGMA; break;
                case 'q': low = high = FEAT_QUARTZ; break;
                case 'r': low = high = FEAT_QUARTZ_H; break;
                case 'k': low = high = FEAT_MAGMA_K; break;
                case '&': low = high = FEAT_QUARTZ_K; break;
                case 'w': low = FEAT_WALL_EXTRA;
                          high = FEAT_WALL_SOLID;
                          break;
                case 'p': low = FEAT_PERM_EXTRA;
                          high = FEAT_PERM_SOLID;
                          break;

                default: low = high = 0x00; break;
            }

            /* Scan map */
            for (y = 1; y <= AUTO_MAX_Y - 1; y++)
            {
                for (x = 1; x <= AUTO_MAX_X - 1; x++)
                {
                    byte a = TERM_RED;

                    borg_grid *ag = &borg_grids[y][x];

                    /* show only those grids */
                    if (!(ag->feat >= low && ag->feat <= high)) continue;

                    /* Color */
                    if (borg_cave_floor_bold(y, x)) a = TERM_YELLOW;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg("Press any key.");
            msg(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

		/* Display Feature of a targetted grid */
		case 'G':
		{
			int y = 1;
			int x = 1;
			s16b ty, tx;

			u16b mask;

			mask = borg_grids[y][x].feat;

			target_get(&tx, &ty);
			y = ty;
			x = tx;

			borg_note(format("Borg's Feat for grid (%d, %d) is %d, game Feat is %d", y,x,mask, cave->feat[y][x]));
			prt_map();
			break;
		}

        /* Command: check "info" flags */
        case 'i':
        {
            int x, y;

            u16b mask;

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
                    byte a = TERM_RED;

                    borg_grid *ag = &borg_grids[y][x];

                    /* Given mask, show only those grids */
                    if (mask && !(ag->info & mask)) continue;

                    /* Given no mask, show unknown grids */
                    if (!mask && (ag->info & BORG_MARK)) continue;

                    /* Color */
                    if (borg_cave_floor_bold(y, x)) a = TERM_YELLOW;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg("Press any key.");
            msg(NULL);

            /* Redraw map */
            prt_map();
            break;
        }


		/* Display Info of a targetted grid */
		case 'I':
		{
			int y = 1;
			int x = 1;
			s16b ty, tx;

			u16b mask;

			mask = borg_grids[y][x].info;

			target_get(&tx, &ty);
			y = ty;
			x = tx;

				if (borg_grids[y][x].info & BORG_MARK) msg("Info for grid (%d, %d) is MARK", y,x);
                if (borg_grids[y][x].info & BORG_GLOW) msg("Info for grid (%d, %d) is GLOW", y,x);
                if (borg_grids[y][x].info & BORG_DARK) msg("Info for grid (%d, %d) is DARK", y,x);
                if (borg_grids[y][x].info & BORG_OKAY)	msg("Info for grid (%d, %d) is OKAY", y,x);
                if (borg_grids[y][x].info & BORG_LIGHT)	msg("Info for grid (%d, %d) is LITE", y,x);
                if (borg_grids[y][x].info & BORG_VIEW)	msg("Info for grid (%d, %d) is VIEW", y,x);
                if (borg_grids[y][x].info & BORG_TEMP)	msg("Info for grid (%d, %d) is TEMP", y,x);
                if (borg_grids[y][x].info & BORG_XTRA)	msg("Info for grid (%d, %d) is XTRA", y,x);
				if (cave->info[y][x] & CAVE_ICKY) msg("Info for grid (%d, %d) is ICKY", y,x);

				borg_note(format("Info for grid (%d, %d) is %d", y,x,cave->info[y][x]));
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
                    byte a = TERM_RED;

                    /* Obtain danger */
                    p = borg_danger(y, x, 1, TRUE, FALSE);

                    /* Skip non-avoidances */
                    if (p < avoidance / 10) continue;

                    /* Use colors for less painful */
                    if (p < avoidance / 2) a = TERM_ORANGE;
                    if (p < avoidance / 4) a = TERM_YELLOW;
                    if (p < avoidance / 6) a = TERM_GREEN;
                    if (p < avoidance / 8) a = TERM_BLUE;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg("(%d,%d of %d,%d) Avoidance value %d.", c_y, c_x, Term->offset_y / PANEL_HGT,Term->offset_x / PANEL_WID,avoidance);
            msg(NULL);

            /* Redraw map */
            prt_map();
            break;
        }


    /* Command: check previous steps */
        case 'y':
        {
            int i;

            /* Scan map */
                    byte a = TERM_RED;
                    /* Check for an existing step */
                    for (i = 0; i < track_step_num; i++)
                    {
                        /* Display */
                        print_rel('*', a, track_step_y[track_step_num-i], track_step_x[track_step_num-i]);
			            msg("(-%d) Steps noted %d,%d", i,track_step_y[track_step_num-i], track_step_x[track_step_num-i]);
						msg(NULL);
                        print_rel('*', TERM_ORANGE, track_step_y[track_step_num-i], track_step_x[track_step_num-i]);
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
                borg_kill *kill = &borg_kills[i];

                /* Still alive */
                if (kill->r_idx)
                {
                    int x = kill->x;
                    int y = kill->y;

                    /* Display */
                    print_rel('*', TERM_RED, y, x);

                    /* Count */
                    n++;
                }
            }

            /* Get keypress */
            msg("There are %d known monsters.", n);
            msg(NULL);

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
                borg_take *take = &borg_takes[i];

                /* Still alive */
                if (take->k_idx)
                {
                    int x = take->x;
                    int y = take->y;

                    /* Display */
                    print_rel('*', TERM_RED, y, x);

                    /* Count */
                    n++;
                }
            }

            /* Get keypress */
            msg("There are %d known objects.", n);
            msg(NULL);

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
			s16b tx, ty;
		    /* Determine "path" */
		    n_x = p_ptr->px;
		    n_y = p_ptr->py;
			target_get(&tx, &ty);
			x = tx;
			y = ty;

			/* Borg's pathway */
			while (1)
			{

                /* Display */
                print_rel('*', TERM_RED, n_y, n_x);

		        if (n_x == x && n_y == y) break;

		        /* Calculate the new location */
        		mmove2(&n_y, &n_x, p_ptr->py, p_ptr->px, y, x);

			}

            msg("Borg's Targetting Path");
            msg(NULL);

		    /* Determine "path" */
		    n_x = p_ptr->px;
		    n_y = p_ptr->py;
			x = tx;
			y = ty;

			/* Real LOS */
      		project(-1, 0, y, x, 1, GF_MISSILE, PROJECT_BEAM);



            msg("Actual Targetting Path");
            msg(NULL);

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
                borg_grid *ag;

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

                        /* Access the grid */
                        ag = &borg_grids[y][x];

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
                        print_rel('*', TERM_RED, y, x);

                        /* Simulate motion */
                        false_y = y;
                        false_x = x;
                    }

                }
                print_rel('*', TERM_YELLOW, borg_flow_y[0], borg_flow_x[0]);
                msg("Probable Flow Path");
                msg(NULL);

                /* Redraw map */
                prt_map();
        break;
        }



        /* Command: debug -- danger of grid */
        case '#':
        {
            int n;
			s16b ty, tx;

			target_get(&tx, &ty);

            /* Turns */
            n = (p_ptr->command_arg ? p_ptr->command_arg : 1);

            /* Danger of grid */
            msg("Danger(%d,%d,%d) is %d",
                        tx, ty, n,
                        borg_danger(ty, tx, n, TRUE, FALSE));
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
                    byte a = TERM_RED;

                    /* Obtain danger */
                    p =  borg_fear_region[y/11][x/11];

                    /* Skip non-fears */
                    if (p < avoidance / 10) continue;

                    /* Use colors = less painful */
                    if (p < avoidance / 2) a = TERM_ORANGE;
                    if (p < avoidance / 4) a = TERM_YELLOW;
                    if (p < avoidance / 6) a = TERM_GREEN;
                    if (p < avoidance / 8) a = TERM_BLUE;

					/* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg("(%d,%d of %d,%d) Regional Fear.", c_y, c_x, Term->offset_y / PANEL_HGT,Term->offset_x / PANEL_WID);
            msg(NULL);

            /* Redraw map */
            prt_map();

            /* Scan map */
            for (y = 1; y <= AUTO_MAX_Y; y++)
            {
                for (x = 1; x <= AUTO_MAX_X; x++)
                {
                    byte a = TERM_BLUE;

                    /* Obtain danger */
                    p =  borg_fear_monsters[y][x];

                    /* Skip non-fears */
                    if (p <= 0) continue;

                    /* Color Defines */
                    if (p == 1)  a = TERM_L_BLUE;

                    /* Color Defines */
                    if (p < avoidance / 20 &&
                        p > 1) a = TERM_BLUE;

                    /* Color Defines */
                    if (p < avoidance / 10 &&
                        p > avoidance / 20) a = TERM_GREEN;

                    /* Color Defines */
                    if (p < avoidance / 4 &&
                        p > avoidance / 10) a = TERM_YELLOW;

                    /* Color Defines */
                    if (p < avoidance / 2 &&
                        p > avoidance / 4) a = TERM_ORANGE;

                    /* Color Defines */
                    if (p > avoidance / 2)  a = TERM_RED;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg("(%d,%d of %d,%d) Monster Fear.", c_y, c_x, Term->offset_y / PANEL_HGT,Term->offset_x / PANEL_WID);
            msg(NULL);

            /* Redraw map */
            prt_map();
            break;
        }

        /* Command: debug -- Power */
        case 'p':
        case 'P':
        {
            s32b p;

            /* Examine the screen */
            borg_update_frame();

            /* Cheat the "equip" screen */
            borg_cheat_equip();

            /* Cheat the "inven" screen */
            borg_cheat_inven();

            /* Examine the screen */
            borg_update();

			/* Examine the inventory */
            borg_object_star_id();
            borg_notice(TRUE);
            /* Evaluate */
            p = borg_power();

            borg_notice_home(NULL, FALSE);

            /* Report it */
            msg("Current Borg Power %ld", p);
            msg("Current Home Power %ld", borg_power_home());

            break;
        }

        /* Command: Show time */
        case '!':
        {
            s32b time = borg_t - borg_began;
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
                    byte a = TERM_RED;

                    /* Obtain danger */
                    if (!borg_los(c_y,c_x, y, x)) continue;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }

            /* Get keypress */
            msg("Borg has Projectable to these places.");
            msg(NULL);

            /* Scan map */
            for (y = w_y; y < w_y + SCREEN_HGT; y++)
            {
                for (x = w_x; x < w_x + SCREEN_WID; x++)
                {
                    byte a = TERM_YELLOW;

                    /* Obtain danger */
                    if (!borg_projectable_dark(c_y, c_x, y, x)) continue;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }
            msg("Borg has Projectable Dark to these places.");
            msg(NULL);

            /* Scan map */
            for (y = w_y; y < w_y + SCREEN_HGT; y++)
            {
                for (x = w_x; x < w_x + SCREEN_WID; x++)
                {
                    byte a = TERM_GREEN;

                    /* Obtain danger */
                    if (!borg_los(c_y, c_x, y, x)) continue;

                    /* Display */
                    print_rel('*', a, y, x);
                }
            }
            msg("Borg has LOS to these places.");
            msg(NULL);
            /* Redraw map */
            prt_map();
            break;
        }
       /*  command: debug -- change max depth */
       case '1':
        {
           int new_borg_skill;
           /* Get the new max depth */
           new_borg_skill = get_quantity("Enter new Max Depth: ", MAX_DEPTH - 1);

           /* Allow user abort */
           if (new_borg_skill >= 0)
           {
               p_ptr->max_depth = new_borg_skill;
               borg_skill[BI_MAXDEPTH] = new_borg_skill;
           }

           break;
       }
       /*  command: debug -- allow borg to stop */
       case 'q':
        {
           int new_borg_stop_dlevel = 127;
           int new_borg_stop_clevel = 51;
           char cmd;

           /* Get the new max depth */
           new_borg_stop_dlevel = get_quantity("Enter new auto-stop dlevel: ", MAX_DEPTH -1);
           new_borg_stop_clevel = get_quantity("Enter new auto-stop clevel: ", 51);
           get_com("Stop when Morgoth Dies? (y or n)? ", &cmd);

           borg_stop_dlevel = new_borg_stop_dlevel;
           borg_stop_clevel = new_borg_stop_clevel;
           if (cmd =='n' || cmd =='N' ) borg_stop_king = FALSE;

           break;
       }

       /* command: money Scum-- allow borg to stop when he gets a certain amount of money*/
       case 'm':
        {
           int new_borg_money_scum_amount = 0;

           /* report current status */
           msg("money Scumming for %d, I need %d more.", borg_money_scum_amount,
                             borg_money_scum_amount - borg_gold);

           /* Get the new amount */
           new_borg_money_scum_amount = get_quantity("Enter new dollar amount for money scumming (0 for no scumming):", borg_money_scum_amount);

           borg_money_scum_amount = new_borg_money_scum_amount;

           break;
       }
        /* Command:  HACK debug -- preparation for level */
        case '2':
        {
          int i=0;

            /* Extract some "hidden" variables */
            borg_cheat_equip();
            borg_cheat_inven();

			/* Examine the screen */
            borg_update_frame();
            borg_update();


            /* Examine the inventory */
            borg_object_star_id();
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);

            /* Dump prep codes */
            for (i = 1; i <= 101; i++)
            {
  			   /* Dump fear code*/
               if ((cptr)NULL != borg_prepared(i)) break;
            }
            borg_slow_return = TRUE;
            msg("Max Level: %d  Prep'd For: %d  Reason: %s", borg_skill[BI_MAXDEPTH], i-1, borg_prepared(i) );
            borg_slow_return = FALSE;
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
			for (i =0; i < 6; i++)
			{
				borg_note(format("stat # %d, is: %d", i, my_stat_cur[i]));
			}
#if 0
			artifact_type *a_ptr;

			int i;
            for (i = 0; i < z_info->a_max; i++)
            {
                 a_ptr = &a_info[i];
				 borg_note(format("(%d) %d, %d (act:%d)",i, a_ptr->name, a_ptr->text, a_ptr->activation));
			}
#endif
            break;
        }

        /* Command: List the swap weapon and armour */
        case 'w':
        case 'W':
        {
            borg_item *item;


            /* Cheat the "equip" screen */
            borg_cheat_equip();
            /* Cheat the "inven" screen */
            borg_cheat_inven();
            /* Examine the inventory */
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);
            /* Check the power */
            borg_power();

            /* Examine the screen */
            borg_update();

			/* Examine the screen */
            borg_update_frame();

            /* note the swap items */
            item = &borg_items[weapon_swap];
            msg("Swap Weapon:  %s, value= %d", item->desc, weapon_swap_value);
            item = &borg_items[armour_swap];
            msg("Swap Armour:  %s, value= %d", item->desc, armour_swap_value);
            break;
        }
        case 'd':
        {
            int ii= 1;

            /* Save the screen */
            Term_save();

            /* Dump the spells */
            if (p_ptr->class->spell_book)
            {

            int i,j;

            for (i = 0; i < 9; i++ )
            {
                /* Clear the screen */
                Term_clear();

                ii = 2;
                Term_putstr(1, ii, -1, TERM_WHITE, "[ Spells ].");
                for (j = 0; j < 9; j++)
                {
                    borg_magic *as = &borg_magics[i][j];
                    cptr legal;
                    int failpercent = 0;

                    if (as->level <99)
                    {
                        if (p_ptr->class->spell_book == TV_PRAYER_BOOK)
                        {
                            legal = (borg_prayer_legal(i, j) ? "Legal" : "Not Legal ");
		                    failpercent = (borg_prayer_fail_rate(i, j));
                        }
                        else
                        {
                            legal = (borg_spell_legal(i, j) ? "legal" : "Not Legal ");
		                    failpercent = (borg_spell_fail_rate(i,j));
                        }
                        Term_putstr(1, ii++, -1, TERM_WHITE, format("%s, %s, attempted %d times, fail rate:%d",as->name, legal, as->times, failpercent));
                    }
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
            char cmd;
            int item, to;

            /* Get a "Borg command", or abort */
            if (!get_com("Dynamic Borg Has What: ((i)nv/(w)orn/(a)rtifact/(s)kill) ", &cmd)) return;

            switch (cmd)
            {
                case 'i':
                case 'I':
                    item = 0;
                    to = z_info->k_max;
                    break;
                case 'w':
                case 'W':
                    item = z_info->k_max;
                    to = z_info->k_max + z_info->k_max;
                    break;
                case 'a':
                case 'A':
                    item = z_info->k_max + z_info->k_max;
                    to = z_info->k_max + z_info->k_max + z_info->a_max;
                    break;
                default:
                    item = z_info->k_max + z_info->k_max + z_info->a_max;
                    to = z_info->k_max + z_info->k_max + z_info->a_max + BI_MAX;
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
            borg_object_star_id();
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);
            for (;item < to; item++)
            {
                switch (cmd)
                {
                    case 'i':
                    case 'I':
                        borg_note(format("Item%03d value= %d.", item, borg_has[item]));
                        break;
                    case 'w':
                    case 'W':
                        borg_note(format("WItem%03d value= %d.", item-z_info->k_max, borg_has[item]));
                        break;
                    case 'a':
                    case 'A':
                        borg_note(format("Artifact%03d value= %d.", item-z_info->k_max-z_info->k_max, borg_has[item]));
                        break;
                    default:
                        borg_note(format("skill %d (%s) value= %d.",item,
                            prefix_pref[item -
                                        z_info->k_max -
                                        z_info->k_max -
                                        z_info->a_max], borg_has[item]));
                        break;
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
            msg("APWBorg Version: %s",borg_engine_date);
            break;
        }
        /* Command: Display all known info on item */
        case 'o':
        case 'O':
        {
			int n =0;

			object_type *item2;

			/* use this item */
            n = (p_ptr->command_arg ? p_ptr->command_arg : 1);

            /* Cheat the "equip" screen */
            borg_cheat_equip();
            /* Cheat the "inven" screen */
            borg_cheat_inven();
            /* Examine the inventory */
            borg_notice(TRUE);
            borg_notice_home(NULL, FALSE);
            /* Check the power */
            borg_power();

			/* Examine the screen */
            borg_update();

            /* Examine the screen */
            borg_update_frame();

			/* Save the screen */
			Term_save();

			/* get the item */
			item2 = &p_ptr->inventory[n];

			/* Display the special screen */
			borg_display_item(item2);

			/* pause for study */
            msg("Borg believes: ");
            msg(NULL);

			/* Restore the screen */
            Term_load();


            break;
        }

		/* Command: Resurrect Borg */
		case 'R':
		{
           char cmd;
			int i =0;

           /* Confirm it */
           get_com("Are you sure you want to Respawn this borg? (y or n)? ", &cmd);

		   if (cmd =='y' || cmd =='Y' )
           {
			   resurrect_borg();
		   }
           break;
       }

		/* Command: Restock the Stores */
		case 'r':
		{
           char cmd;
			int n;

           /* Confirm it */
           get_com("Are you sure you want to Restock the stores? (y or n)? ", &cmd);

           if (cmd =='y' || cmd =='Y' )
           {
				/* Message */
				msg("Updating Shops...");

				/* Maintain each shop (except home) */
				for (n = 0; n < MAX_STORES; n++)
				{
					/* Skip the home */
					if (n == STORE_HOME) continue;

					/* Maintain */
					store_maint(&stores[n]);
				}
		   }

           break;
       }

        case ';':
        {
            int glyph_check;

            byte a = TERM_RED;

	        for (glyph_check = 0; glyph_check < track_glyph_num; glyph_check++)
		    {
					/* Display */
                    print_rel('*', a, track_glyph_y[glyph_check], track_glyph_x[glyph_check]);
					msg("Borg has Glyph (%d)noted.", glyph_check);
		            msg(NULL);
            }

            /* Get keypress */
		}
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

