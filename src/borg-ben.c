/* File: borg-ben.c */

/* Purpose: an "automatic" player -BEN- */

#include "angband.h"



#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-map.h"

#include "borg-obj.h"

#include "borg-ext.h"

#include "borg-aux.h"

#include "borg-ben.h"


/*
 * This file implements the "Ben Borg", an "Automatic Angband Player".
 *
 * This version of the "Ben Borg" is designed for use with Angband 2.7.9v3.
 *
 * The "Ben Borg" can (easily?) be adapted to create your very own "Borg",
 * using your own favorite strategies, and/or heuristic values.
 *
 * Use of the "Ben Borg" requires re-compilation with ALLOW_BORG defined,
 * and with the "borg-ben.c", "borg-aux.c", "borg-ext.c", "borg-map.c",
 * "borg-obj.c", and "borg.c" files linked into the executable.
 *
 * Note that you can only use the Borg if your character has been marked
 * as a "Borg User".  You can do this, if necessary, by responding "y"
 * when asked if you really want to use the Borg.  This will (probably)
 * result in your character being inelligible for the high score list.
 *
 * The "borg_ben()" function, called when the user hits "^Z", allows the
 * user to interact with the Borg.  You do so by typing "Borg Commands",
 * including 'z' to resume (or start running), or 'K' to show monsters,
 * or 'A' to show avoidances, or 'd' to toggle "demo mode", or 'f' to
 * open/shut the "log file", etc.  See "borg_ben()" for complete details.
 *
 * The first time you enter a Borg command, the Borg is initialized.  This
 * consists of three major steps, and requires at least 400K of free memory,
 * if the memory is not available, the game may abort.
 *
 * (1) The "Term_xtra()" hook is stolen, allowing the Borg to interupt
 * whenever the game asks for a keypress, and to supply its own keypresses
 * as if it was the player.  This is a total hack.
 *
 * (2) Some important "state" information is extracted, including the level
 * and race/class of the player, which are needed for initialization of some
 * of the modules.  Also, some "historical" information (killed uniques,
 * maximum dungeon depth, etc) is "stolen" from the game.
 *
 * (3) Various modules are initialized, including "borg-aux.c" (high level
 * control), "borg-ext.c" (low level control), "borg-obj.c" (item analysis),
 * "borg-map.c" (dungeon analysis), and "borg.c" (low level variables).
 *
 * When the Ben Borg is "inactive", it simply passes control through to the
 * standard "Term_xtra()" hook.  When the Ben Borg is "active" (for example,
 * after the 'z' command), it "steals" certain of the "Term_xtra()" actions,
 * allowing it to pretend to be a player.  It also checks for actual user
 * input, and if it detects any keypresses from the user, it will "halt"
 * (become inactive) and strip those key presses.  Eventually, it may be
 * possible to halt more "cleanly", preventing the Borg from entering a
 * "bizarre" state.  Once halted, you can normally "resume" the Ben Borg
 * with the 'z' command.  The Ben Borg will also "halt" if the game
 * attempts to produce a "bell" noise, or if it detects various internal
 * "errors", such as death, or "situations", such as impending death, if
 * the respective "panic" flags are set.
 *
 * The Ben Borg is only supposed to "know" what is visible on the screen,
 * which it learns by using the "term.c" screen access function "Term_what()",
 * and the cursor location function "Term_locate()", and a hack to access the
 * cursor visibility using "Term_show/hide_cursor()".  It is only allowed to
 * send keypresses to the "Term" like a normal user, by using the standard
 * "Term_keypress()" routine.
 *
 * The Ben Borg only thinks and sends keys when it catches calls to
 * "Term_xtra(TERM_XTRA_EVENT, TRUE)", which the "term.c" file uses to wait
 * for a keypress (or other event) from the user.  It also catches calls to
 * "Term_xtra(TERM_XTRA_EVENT, FALSE)", to prevent "keypress sneaking",
 * and calls to "Term_xtra(TERM_XTRA_NOISE, *)" to prevent "noisy" bugs,
 * and calls to "Term_xtra(TERM_XTRA_FLUSH, *)" to simulate actual flushing.
 * The rest of the "Term_xtra()" calls are passed through.
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
 * The Borg is thus allowed to examine the screen directly (usually by
 * an efficient direct access to "Term->scr->a" and "Term->scr->c") and
 * to send keypresses directly (via "Term_keypress()").  The Borg also
 * accesses the cursor location ("Term_locate()") and visibility (via
 * a hack involving "Term_show/hide_cursor()") directly as well.
 *
 * The Borg should not know when the game is ready for a keypress, but it
 * checks this anyway by distinguishing between the "check for keypress"
 * and "wait for keypress" hooks sent by the "term.c" package.  Otherwise
 * it would have to "pause" between turns for some amount of time to ensure
 * that the game was done processing.  It might be possible to notice when
 * the game is ready for input by some other means, but it seems likely that
 * at least some "waiting" would be necessary, unless the terminal emulation
 * program explicitly sends a "ready" sequence when ready for a keypress.
 *
 * Various other "cheats" (mostly optional) are described where they are
 * used, primarily in "borg-ben.c", for example.
 *
 * Note that any "user input" will be ignored, and will cancel the Borg.
 *
 * Note that the "c_t" parameter bears a close resemblance to the number of
 * "player turns" that have gone by.  Except that occasionally, the Borg will
 * do something that he *thinks* will take time but which actually does not
 * (for example, attempting to open a hallucinatory door), and that sometimes,
 * the Borg performs a "repeated" command (rest, open, tunnel, or search),
 * which may actually take longer than a single turn.  This has the effect
 * that the "c_t" variable is slightly lacking in "precision".  Note that
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
 * The Borg assumes that the "p_ptr->maximize" flag is off, and that the
 * "p_ptr->preserve" flag is on, since he cannot actually set those flags.
 * If the "p_ptr->maximize" flag is on, the Borg may not work correctly.
 * If the "p_ptr->preserve" flag is off, the Borg may miss artifacts.
 */
 

/*
 * We currently handle:
 *   Level changing (intentionally or accidentally)
 *   Embedded objects (gold) that must be extracted
 *   Ignore embedded objects if too "weak" to extract
 *   Traps (disarm), Doors (open/bash), Rubble (tunnel)
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
 *   Use the "danger" code to avoid potential death
 *   Handle "Mace of Disruption", "Scythe of Slicing", etc
 *   Learn spells, and use them when appropriate
 *   Remember that studying prayers is slightly random
 *   Do not try to read scrolls when blind or confused
 *   Do not study/use spells/prayers when blind/confused
 *   Attempt to heal when "confused", "blind", etc
 *   Attempt to remove fear and poison when possible
 *   Analyze potential equipment in proper context
 *   Priests should avoid edged weapons (spell failure)
 *   Mages should avoid most gloves (lose mana)
 *   Non-warriors should avoid heavy armor (lose mana)
 *   Swap rings in shops to allow use of "tight" ring slot
 *   Remove items which do not contribute to total fitness
 *   Wear/Remove/Sell/Buy items in most optimal order
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
 *   Search for secret doors if stuck on a level (spastic)
 *   React intelligently to changes in the wall structure
 *   Do not recalculate "flow" information unless required
 *   Collect monsters/objects left behind or seen from afar
 *   Try to avoid danger (both actively and passively)
 *   Keep in mind that word of recall is a delayed action
 *   Keep track of charging items (rods and artifacts)
 *   Be VERY careful not to access illegal locations!
 *   Do not rest next to dangerous (or immobile) monsters
 *   Recall into dungeon if prepared for resulting depth
 *   Do not attempt to destroy cursed ("terrible") artifacts
 *   Attempted destruction will yield "terrible" inscription
 *   Use "maximum" level and depth to prevent "thrashing"
 *   Use "maximum" hp's and sp's when checking "potentials"
 *   Attempt to recognize large groups of "disguised" monsters
 *
 * We ignore:
 *   Running out of light can (fatally) confuse the Borg
 *   Running out of food can kill you, try not to starve
 *   Long object descriptions may have clipped inscriptions
 *
 * We need to handle:
 *   Appearance of "similar" monsters (jackals + grip)
 *   Trappers and other monsters that look like floors
 *   Mimics, which look like potions, scrolls, or rings
 *   Management of discounted spell-books and other items
 *   Hallucination (induces fake objects and monsters)
 *   Special screens (including tombstone) with no "walls"
 *   Appearance of the '@' symbol on "special" screens
 *   Technically a room can have no exits, requiring digging
 *   Try to use a shovel/pick to help with tunnelling
 *   If wounded, must run away from monsters, then rest
 *   Invisible monsters causing damage while trying to rest
 *   Becoming "afraid" (attacking takes a turn for no effect)
 *   When blind, monster and object info may be invalid
 *   Attempt to "track" monsters who flee down corridors
 *   When totally surrounded by monsters, try to escape rooms
 *   Conserve memory space (each grid byte costs about 15K)
 *   Conserve computation time (especially with the flow code)
 *   Note -- nutrition can come from food, scrolls, or spells
 *   Note -- recall can come from scrolls, rods, or spells
 *   Note -- identify can come from scrolls, rods, staffs, or spells
 *   Beware of firing missiles at an "object" thought to be a "monster"
 *   Take account of "combinations" of possible equipment
 *   Stockpile items in the Home, and use those stockpiles
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
 * Cheats that can be avoided by toggling a switch:
 *   Direct extraction of "panel" info (cheat_panel)
 *   Direct extraction of "inven" info (cheat_inven)
 *   Direct extraction of "equip" info (cheat_equip)
 *   Direct extraction of "spell" info (cheat_spell)
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

static bool initialized;	/* Hack -- Initialized */


static bool cheat_equip;	/* Cheat for "equip mode" */

static bool cheat_inven;	/* Cheat for "inven mode" */

static bool cheat_spell;	/* Cheat for "browse mode" */

static bool cheat_panel;	/* Cheat for "panel mode" */


static bool do_inven = TRUE;	/* Acquire "inven" info */
static bool do_equip = TRUE;	/* Acquire "equip" info */

static bool do_panel = TRUE;	/* Acquire "panel" info */

static bool do_frame = TRUE;	/* Acquire "frame" info */

static bool do_spell = TRUE;	/* Acquire "spell" info */

static byte do_spell_aux = 0;	/* Hack -- current book for "do_spell" */


static bool do_browse = 0;	/* Hack -- browse the current store */

static int old_store = 0;	/* Hack -- identity of old store */

static int old_more = 0;	/* Hack -- pages in old store */



/*
 * Mega-Hack -- extract some "hidden" variables
 */
static void borg_hidden(void)
{
    int i;

    int stat_add[6];


    /* Clear "stat_add[]" */
    for (i = 0; i < 6; i++) stat_add[i] = 0;

    /* Scan the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Affect stats */
        if (item->flags1 & TR1_STR) stat_add[A_STR] += item->pval;
        if (item->flags1 & TR1_INT) stat_add[A_INT] += item->pval;
        if (item->flags1 & TR1_WIS) stat_add[A_WIS] += item->pval;
        if (item->flags1 & TR1_DEX) stat_add[A_DEX] += item->pval;
        if (item->flags1 & TR1_CON) stat_add[A_CON] += item->pval;
        if (item->flags1 & TR1_CHR) stat_add[A_CHR] += item->pval;
    }

    /* Mega-Hack -- Guess at "my_stat_cur[]" */
    for (i = 0; i < 6; i++) {

        int value;

        /* Hack -- reverse the known bonus */
        value = modify_stat_value(auto_stat[i], -stat_add[i]);

        /* Hack -- save the maximum/current stats */
        my_stat_max[i] = my_stat_cur[i] = value;
    }
}



/*
 * Think about the world and perform an action
 *
 * Check inventory/equipment once per "turn"
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
 */
static void borg_think(void)
{
    int i;

    byte t_a;

    char buf[128];


    /*** Process inventory/equipment and Browse Books ***/

    /* Cheat */
    if (cheat_equip && do_equip) {

        /* Only do it once */
        do_equip = FALSE;

        /* Cheat the "equip" screen */
        borg_cheat_equip();

        /* Done */
        return;
    }

    /* Cheat */
    if (cheat_inven && do_inven) {

        /* Only do it once */
        do_inven = FALSE;

        /* Cheat the "inven" screen */
        borg_cheat_inven();

        /* Done */
        return;
    }


    /* Parse "equip" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
       (streq(buf, "Equipment "))) {

        /* Parse the "equip" screen */
        borg_parse_equip();

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return;
    }


    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
       (streq(buf, "Inventory "))) {

        /* Parse the "inven" screen */
        borg_parse_inven();

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return;
    }


    /* Check "equip" */
    if (do_equip) {

        /* Only do it once */
        do_equip = FALSE;

        /* Enter "equip" mode */
        borg_keypress('e');

        /* Done */
        return;
    }

    /* Check "inven" */
    if (do_inven) {

        /* Only do it once */
        do_inven = FALSE;

        /* Enter "inven" mode */
        borg_keypress('i');

        /* Done */
        return;
    }


    /*** Handle stores ***/

    /* Hack -- Check for being in a store */
    if ((0 == borg_what_text(3, 5, 16, &t_a, buf)) &&
       (streq(buf, "Item Description"))) {

        /* Assume the Home */
        shop_num = 7;

        /* Extract the "store" name */
        if (0 == borg_what_text(50, 3, -20, &t_a, buf)) {

            int i;

            /* Check the store names */
            for (i = 0; i < 7; i++) {
                cptr name = (f_name + f_info[0x08+i].name);
                if (prefix(buf, name)) shop_num = i;
            }
        }

        /* Hack -- reset page/more */
        auto_shops[shop_num].page = 0;
        auto_shops[shop_num].more = 0;

        /* React to new stores */
        if (old_store != shop_num) {

            /* Clear all the items */
            for (i = 0; i < 24; i++) {

                /* XXX Wipe the ware */
                WIPE(&auto_shops[shop_num].ware[i], auto_item);
            }

            /* Save the store */
            old_store = shop_num;
        }


        /* Extract the "page", if any */
        if ((0 == borg_what_text(20, 5, 8, &t_a, buf)) &&
            (prefix(buf, "(Page "))) /* --)-- */ {

            /* Take note of the page */
            auto_shops[shop_num].more = 1;
            auto_shops[shop_num].page = (buf[6] - '0') - 1;
        }

        /* React to disappearing pages */
        if (old_more != auto_shops[shop_num].more) {

            /* Clear the second page */
            for (i = 12; i < 24; i++) {

                /* XXX Wipe the ware */
                WIPE(&auto_shops[shop_num].ware[i], auto_item);
            }

            /* Save the new one */
            old_more = auto_shops[shop_num].more;
        }

        /* Extract the current gold (unless in home) */
        if (0 == borg_what_text(68, 19, -9, &t_a, buf)) {

            /* Save the gold, if valid */
            if (buf[0]) auto_gold = atol(buf);
        }

        /* Parse the store (or home) inventory */
        for (i = 0; i < 12; i++) {

            int n;
            
            char desc[80];
            char cost[10];

            /* Default to "empty" */
            desc[0] = '\0';
            cost[0] = '\0';

            /* Verify "intro" to the item */
            if ((0 == borg_what_text(0, i + 6, 3, &t_a, buf)) &&
                (buf[0] == I2A(i)) && (buf[1] == p2) && (buf[2] == ' ')) {

                /* Extract the item description */
                if (0 != borg_what_text(3, i + 6, -65, &t_a, desc)) {
                    desc[0] = '\0';
                }

                /* XXX XXX Make sure trailing spaces get stripped */

                /* Extract the item cost in stores */
                if (shop_num != 7) {
                    if (0 != borg_what_text(68, i + 6, -9, &t_a, cost)) {
                        cost[0] = '\0';
                    }
                }
            }

            /* Extract actual index */
            n = auto_shops[shop_num].page * 12 + i;

            /* Ignore "unchanged" descriptions */
            if (streq(desc, auto_shops[shop_num].ware[n].desc)) continue;

            /* Analyze the item */
            borg_item_analyze(&auto_shops[shop_num].ware[n], desc);

            /* Hack -- Save the declared cost */
            auto_shops[shop_num].ware[n].cost = atol(cost);
        }

        /* Hack -- browse as needed */
        if (auto_shops[shop_num].more && do_browse) {

            /* Check next page */
            borg_keypress(' ');

            /* Done browsing */
            do_browse = FALSE;

            /* Done */
            return;
        }

        /* Hack -- recheck inventory/equipment */
        do_inven = do_equip = TRUE;

        /* Hack -- browse again later */
        do_browse = TRUE;

        /* Think until done */
        (void)borg_think_store();

        /* Done */
        return;
    }



    /*** Determine panel ***/

    /* Hack -- cheat */
    if (cheat_panel) {

        /* Cheat */
        w_y = panel_row * (SCREEN_HGT / 2);
        w_x = panel_col * (SCREEN_WID / 2);

        /* Done */
        do_panel = FALSE;
    }

    /* Hack -- Check for "sector" mode */
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf)) &&
       (prefix(buf, "Map sector "))) {

        /* Hack -- get the panel info */
        w_y = (buf[12] - '0') * (SCREEN_HGT / 2);
        w_x = (buf[14] - '0') * (SCREEN_WID / 2);

        /* Leave panel mode */
        borg_keypress(ESCAPE);

        /* Done */
        return;
    }

    /* Check equipment */
    if (do_panel) {

        /* Only do it once */
        do_panel = FALSE;

        /* Enter "panel" mode */
        borg_keypress('L');

        /* Done */
        return;
    }


    /*** Analyze the Frame ***/

    /* Analyze the frame */
    if (do_frame) {

        /* Only once */
        do_frame = FALSE;

        /* Analyze the "frame" */
        borg_update_frame();
    }


    /*** Handle browsing ***/

    /* Hack -- Warriors never browse */
    if (auto_class == 0) do_spell = FALSE;

    /* Hack -- Blind or Confused prevents browsing */
    if (do_blind || do_confused) do_spell = FALSE;

    /* Hack -- Stop doing spells when done */
    if (do_spell_aux > 8) do_spell = FALSE;

    /* Cheat */
    if (cheat_spell && do_spell) {

        /* Look for the book */
        i = borg_book(do_spell_aux);

        /* Cheat the "spell" screens (all of them) */
        if (i >= 0) {

            /* Cheat that page */
            borg_cheat_spell(do_spell_aux);
        }

        /* Advance to the next book */
        do_spell_aux++;

        /* Done */
        return;
    }

    /* Check for "browse" mode */
    if ((0 == borg_what_text(COL_SPELL, ROW_SPELL, -12, &t_a, buf)) &&
        (streq(buf, "Lv Mana Fail"))) {

        /* Parse the "spell" screen */
        borg_parse_spell(do_spell_aux);

        /* Leave that mode */
        borg_keypress(ESCAPE);

        /* Advance to the next book */
        do_spell_aux++;

        /* Done */
        return;
    }

    /* Check "spells" */
    if (do_spell) {

        /* Look for the book */
        i = borg_book(do_spell_aux);

        /* Enter the "spell" screen */
        if (i >= 0) {

            /* Enter "browse" mode */
            borg_keypress('b');

            /* Pick the next book */
            borg_keypress(I2A(i));
        }

        /* Otherwise, advance */
        else {

            /* Advance to the next book */
            do_spell_aux++;
        }

        /* Done */
        return;
    }


    /*** Re-activate Tests ***/

    /* Check equip again later */
    do_equip = TRUE;

    /* Check inven again later */
    do_inven = TRUE;

    /* Check panel again later */
    do_panel = TRUE;

    /* Check frame again later */
    do_frame = TRUE;

    /* Check spells again later */
    do_spell = TRUE;

    /* Hack -- Start the books over */
    do_spell_aux = 0;


    /*** Analyze status ***/

    /* Track best level */
    if (auto_level > auto_max_level) auto_max_level = auto_level;
    if (auto_depth > auto_max_depth) auto_max_depth = auto_depth;


    /*** Think about it ***/

    /* Increment the clock */
    c_t++;

    /* Examine the screen */
    borg_update();

    /* Extract some "hidden" variables */
    borg_hidden();

    /* Do something */
    (void)borg_think_dungeon();
}



/*
 * Hack -- methods of killing a monster (order not important).
 *
 * See "mon_take_hit()" for details.
 */
static cptr prefix_kill[] = {

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
static cptr suffix_died[] = {

    " dies.",
    " is destroyed.",
    " dissolves!",
    " shrivels away in the light!",
    NULL
};


/*
 * Hack -- methods of hitting the player (order not important).
 *
 * The "insult", "moan", and "begs you for money" messages are ignored.
 *
 * See "make_attack_normal()" for details.
 */
static cptr suffix_hit_by[] = {

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
static cptr suffix_spell[] = {

    " makes a high pitched shriek.",		/* RF4_SHRIEK */
    " does something.",				/* RF4_XXX2X4 */
    " does something.",				/* RF4_XXX3X4 */
    " does something.",				/* RF4_XXX4X4 */
    " fires an arrow.",				/* RF4_ARROW_1 */
    " fires an arrow!",				/* RF4_ARROW_2 */
    " fires a missile.",			/* RF4_ARROW_3 */
    " fires a missile!",			/* RF4_ARROW_4 */
    " breathes acid.",				/* RF4_BR_ACID */
    " breathes lightning.",			/* RF4_BR_ELEC */
    " breathes fire.",				/* RF4_BR_FIRE */
    " breathes frost.",				/* RF4_BR_COLD */
    " breathes gas.",				/* RF4_BR_POIS */
    " breathes nether.",			/* RF4_BR_NETH */
    " breathes light.",				/* RF4_BR_LITE */
    " breathes darkness.",			/* RF4_BR_DARK */
    " breathes confusion.",			/* RF4_BR_CONF */
    " breathes sound.",				/* RF4_BR_SOUN */
    " breathes chaos.",				/* RF4_BR_CHAO */
    " breathes disenchantment.",		/* RF4_BR_DISE */
    " breathes nexus.",				/* RF4_BR_NEXU */
    " breathes time.",				/* RF4_BR_TIME */
    " breathes inertia.",			/* RF4_BR_INER */
    " breathes gravity.",			/* RF4_BR_GRAV */
    " breathes shards.",			/* RF4_BR_SHAR */
    " breathes plasma.",			/* RF4_BR_PLAS */
    " breathes force.",				/* RF4_BR_WALL */
    " does something.",				/* RF4_BR_MANA */
    " does something.",				/* RF4_XXX5X4 */
    " does something.",				/* RF4_XXX6X4 */
    " does something.",				/* RF4_XXX7X4 */
    " does something.",				/* RF4_XXX8X4 */
    " casts an acid ball.",			/* RF5_BA_ACID */
    " casts a lightning ball.",			/* RF5_BA_ELEC */
    " casts a fire ball.",			/* RF5_BA_FIRE */
    " casts a frost ball.",			/* RF5_BA_COLD */
    " casts a stinking cloud.",			/* RF5_BA_POIS */
    " casts a nether ball.",			/* RF5_BA_NETH */
    " gestures fluidly.",			/* RF5_BA_WATE */
    " invokes a mana storm.",			/* RF5_BA_MANA */
    " invokes a darkness storm.",		/* RF5_BA_DARK */
    " draws psychic energy from you!",		/* RF5_DRAIN_MANA */
    " gazes deep into your eyes.",		/* RF5_MIND_BLAST */
    " looks deep into your eyes.",		/* RF5_BRAIN_SMASH */
    " points at you and curses.",		/* RF5_CAUSE_1 */
    " points at you and curses horribly.",	/* RF5_CAUSE_2 */
    " points at you, incanting terribly!",	/* RF5_CAUSE_3 */
    " points at you, screaming the word DIE!",	/* RF5_CAUSE_4 */
    " casts a acid bolt.",			/* RF5_BO_ACID */
    " casts a lightning bolt.",			/* RF5_BO_ELEC */
    " casts a fire bolt.",			/* RF5_BO_FIRE */
    " casts a frost bolt.",			/* RF5_BO_COLD */
    " does something.",				/* RF5_BO_POIS */
    " casts a nether bolt.",			/* RF5_BO_NETH */
    " casts a water bolt.",			/* RF5_BO_WATE */
    " casts a mana bolt.",			/* RF5_BO_MANA */
    " casts a plasma bolt.",			/* RF5_BO_PLAS */
    " casts an ice bolt.",			/* RF5_BO_ICEE */
    " casts a magic missile.",			/* RF5_MISSILE */
    " casts a fearful illusion.",		/* RF5_SCARE */
    " casts a spell, burning your eyes!",	/* RF5_BLIND */
    " creates a mesmerising illusion.",		/* RF5_CONF */
    " drains power from your muscles!",		/* RF5_SLOW */
    " stares deep into your eyes!",		/* RF5_HOLD */
    " concentrates on XXX body.",		/* RF6_HASTE */
    " does something.",				/* RF6_XXX1X6 */
    " concentrates on XXX wounds.",		/* RF6_HEAL */
    " does something.",				/* RF6_XXX2X6 */
    " blinks away.",				/* RF6_BLINK */
    " teleports away.",				/* RF6_TPORT */
    " does something.",				/* RF6_XXX3X6 */
    " does something.",				/* RF6_XXX4X6 */
    " commands you to return.",			/* RF6_TELE_TO */
    " teleports you away.",			/* RF6_TELE_AWAY */
    " gestures at your feet.",			/* RF6_TELE_LEVEL */
    " does something.",				/* RF6_XXX5 */
    " gestures in shadow.",			/* RF6_DARKNESS */
    " casts a spell and cackles evilly.",	/* RF6_TRAPS */
    " tries to blank your mind.",		/* RF6_FORGET */
    " does something.",				/* RF6_XXX6X6 */
    " does something.",				/* RF6_XXX7X6 */
    " does something.",				/* RF6_XXX8X6 */
    " magically summons help!",			/* RF6_S_MONSTER */
    " magically summons monsters!",		/* RF6_S_MONSTERS */
    " magically summons ants.",			/* RF6_S_ANT */
    " magically summons spiders.",		/* RF6_S_SPIDER */
    " magically summons hounds.",		/* RF6_S_HOUND */
    " magically summons hydras.",		/* RF6_S_HYDRA */
    " magically summons an angel!",		/* RF6_S_ANGEL */
    " magically summons a hellish adversary!",	/* RF6_S_DEMON */
    " magically summons an undead adversary!",	/* RF6_S_UNDEAD */
    " magically summons a dragon!",		/* RF6_S_DRAGON */
    " magically summons greater undead!",	/* RF6_S_HI_UNDEAD */
    " magically summons ancient dragons!",	/* RF6_S_HI_DRAGON */
    " magically summons mighty undead opponents!",	/* RF6_S_WRAITH */
    " magically summons special opponents!",		/* RF6_S_UNIQUE */
    NULL
};



#if 0
    /* XXX XXX XXX */
    msg_format("%^s looks healthier.", m_name);
    msg_format("%^s looks REALLY healthy!", m_name);
#endif



/*
 * Hack -- Spontaneous level feelings (order important).
 *
 * See "do_cmd_feeling()" for details.
 */
static cptr prefix_feeling[] = {

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
 * otherwise by a failure message.
 *
 * Note that certain other messages may contain useful information,
 * and so they are "analyzed" and sent to "borg_react()", which just
 * queues the messages for later analysis in the proper context.
 *
 * Along with the actual message, we send a formatted buffer, with
 * a leading "opcode", and colon-separated fields, with all monster
 * names (including "it" and "the kobold") capitalized.
 *
 * XXX XXX XXX Several message strings take a "possessive" of the form
 * "his" or "her" or "its".  These strings are all represented by the
 * encoded form "XXX" in the various match strings.  Unfortunately,
 * the encode form is never decoded, so the Borg currently ignores
 * messages about several spells (heal self and haste self).
 */
static void borg_parse(cptr msg)
{
    int i, tmp, len;

    char buf[256];

    /* Extract the length */
    len = strlen(msg);


    /* Log (if needed) */
    if (auto_fff) borg_info(format("& Msg <%s>", msg));


    /* Hack -- Notice death */
    if (prefix(msg, "You die.")) {

        /* Oops */
        borg_oops("death");
        return;
    }


    /* Hack -- Notice "failure" */
    if (prefix(msg, "You failed ") ||
        prefix(msg, "It whines, glows ")) {

        /* Flush our key-buffer */
        borg_flush();
        return;
    }


    /* Ignore teleport trap */
    if (prefix(msg, "You hit a teleport")) return;
    
    /* Ignore arrow traps */
    if (prefix(msg, "An arrow ")) return;

    /* Ignore dart traps */
    if (prefix(msg, "A small dart ")) return;


    /* Hit somebody */
    if (prefix(msg, "You hit ")) {
        tmp = strlen("You hit ");
        strnfmt(buf, 4 + 1 + len - (tmp + 1), "HIT:%^s", msg + tmp);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody */
    if (prefix(msg, "You miss ")) {
        tmp = strlen("You miss ");
        strnfmt(buf, 5 + 1 + len - (tmp + 1), "MISS:%^s", msg + tmp);
        borg_react(msg, buf);
        return;
    }

    /* Miss somebody (because of fear) */
    if (prefix(msg, "You are too afraid to attack ")) {
        tmp = strlen("You are too afraid to attack ");
        strnfmt(buf, 5 + 1 + len - (tmp + 1), "MISS:%^s", msg + tmp);
        borg_react(msg, buf);
        return;
    }


    /* "You have killed it." (etc) */
    for (i = 0; prefix_kill[i]; i++) {

        /* "You have killed it." (etc) */
        if (prefix(msg, prefix_kill[i])) {
            tmp = strlen(prefix_kill[i]);
            strnfmt(buf, 5 + 1 + len - (tmp + 1), "KILL:%^s", msg + tmp);
            borg_react(msg, buf);
            return;
        }
    }


    /* "It dies." (etc) */
    for (i = 0; suffix_died[i]; i++) {

        /* "It dies." (etc) */
        if (suffix(msg, suffix_died[i])) {
            tmp = strlen(suffix_died[i]);
            strnfmt(buf, 5 + 1 + len - tmp, "DIED:%^s", msg);
            borg_react(msg, buf);
            return;
        }
    }


    /* "It misses you." */
    if (suffix(msg, " misses you.")) {
        tmp = strlen(" misses you.");
        strnfmt(buf, 8 + 1 + len - tmp, "MISS_BY:%^s", msg);
        borg_react(msg, buf);
        return;
    }

    /* "It hits you." (etc) */
    for (i = 0; suffix_hit_by[i]; i++) {

        /* "It hits you." (etc) */
        if (suffix(msg, suffix_hit_by[i])) {
            tmp = strlen(suffix_hit_by[i]);
            strnfmt(buf, 7 + 1 + len - tmp, "HIT_BY:%^s", msg);
            borg_react(msg, buf);
            return;
        }
    }


    /* "It casts a spell." (etc) */
    for (i = 0; suffix_spell[i]; i++) {

        /* "It casts a spell." (etc) */
        if (suffix(msg, suffix_spell[i])) {
            tmp = strlen(suffix_spell[i]);
            strnfmt(buf, 6 + 1 + len - tmp, "SPELL:%^s", msg);
            borg_react(msg, buf);
            return;
        }
    }


    /* State -- Asleep */
    if (suffix(msg, " falls asleep!")) {
        tmp = strlen(" falls asleep!");
        strnfmt(buf, 12 + 1 + len - tmp, "STATE:SLEEP:%^s", msg);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Asleep */
    if (suffix(msg, " wakes up.")) {
        tmp = strlen(" wakes up.");
        strnfmt(buf, 12 + 1 + len - tmp, "STATE:AWAKE:%^s", msg);
        borg_react(msg, buf);
        return;
    }

    /* State -- Afraid */
    if (suffix(msg, " flees in terror!")) {
        tmp = strlen(" flees in terror!");
        strnfmt(buf, 11 + 1 + len - tmp, "STATE:FEAR:%^s", msg);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers his courage.")) {
        tmp = strlen(" recovers his courage.");
        strnfmt(buf, 11 + 1 + len - tmp, "STATE:BOLD:%^s", msg);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers her courage.")) {
        tmp = strlen(" recovers her courage.");
        strnfmt(buf, 11 + 1 + len - tmp, "STATE:BOLD:%^s", msg);
        borg_react(msg, buf);
        return;
    }

    /* State -- Not Afraid */
    if (suffix(msg, " recovers its courage.")) {
        tmp = strlen(" recovers its courage.");
        strnfmt(buf, 11 + 1 + len - tmp, "STATE:BOLD:%^s", msg);
        borg_react(msg, buf);
        return;
    }


    /* Word of Recall -- Ignition */
    if (prefix(msg, "The air about you becomes ")) {

        /* Initiate recall */
        goal_recalling = TRUE;
        return;
    }

    /* Word of Recall -- Lift off */
    if (prefix(msg, "You feel yourself yanked ")) {

        /* Recall complete */
        goal_recalling = FALSE;
        return;
    }

    /* Word of Recall -- Cancelled */
    if (prefix(msg, "A tension leaves ")) {

        /* Hack -- Oops */
        goal_recalling = FALSE;
        return;
    }


    /* Feelings about the level */
    for (i = 0; prefix_feeling[i]; i++) {
    
        /* "You feel..." (etc) */
        if (prefix(msg, prefix_feeling[i])) {
            strnfmt(buf, 256, "FEELING:%d", i);
            borg_react(msg, buf);
            return;
        }
    }
}







/*
 * Hook -- The normal "Term_xtra()" hook
 */
static errr (*Term_xtra_hook_old)(int n, int v) = NULL;



/*
 * Bypass the standard "Term_xtra()" hook with this replacement
 *
 * This function allows the Borg to "steal" the "key checker"
 *
 * Note the use of the "old" hook for parsing non-input related requests,
 * and also, in combination with a special "bypass", for checking for
 * interuption by the user.  This whole function is a complete hack,
 * made necessary by the framework in which we are working.
 *
 * XXX XXX XXX We should probably attempt to handle "broken" messages,
 * in which long messages are "broken" into pieces, and all but the
 * first message are "indented" by, um, two spaces or something.
 *
 * Note the complete hack that allows the Borg to run in "demo"
 * mode, which allows the Borg to cheat death 100 times, and stamps
 * the number of cheats in the player "age" field.  XXX XXX XXX
 */
static errr Term_xtra_borg(int n, int v)
{
    /* Mega-Hack -- The Borg notices "TERM_XTRA_NOISE" */
    if (auto_active && (n == TERM_XTRA_NOISE)) {

        /* Halt the Borg */
        borg_oops("bell");

        /* Success */
        return (0);
    }


    /* Mega-Hack -- The Borg notices "TERM_XTRA_FLUSH" */
    if (auto_active && (n == TERM_XTRA_FLUSH)) {

        /* Hack -- flush the key buffer */
        borg_flush();

        /* Success */
        return (0);
    }


    /* Hack -- The Borg intercepts "TERM_XTRA_EVENT" */
    while (auto_active && (n == TERM_XTRA_EVENT)) {

        errr err;

        char ch;
                
        int i, x, y;

        bool visible;

        byte t_a;
        char buf[128];


        /* Open Bypass */
        Term->xtra_hook = Term_xtra_hook_old;

        /* Hack -- Check for keypress */
        err = Term_inkey(&ch, FALSE, FALSE);

        /* Shut Bypass */
        Term->xtra_hook = Term_xtra_borg;

        /* User Abort */
        if (!err) {

            /* Stop the Borg */
            borg_oops("user abort");

            /* Flush input */
            flush();

            /* Success */
            return (0);
        }


        /* Only "think" when "waiting" */
        if (!v) return (0);
        

        /* XXX XXX XXX Mega-Hack -- Check the "cursor state" */

        /* Note that the cursor visibility determines whether the */
        /* game is asking us for a "command" or for some other key. */
        /* XXX XXX This requires that "hilite_player" be FALSE. */

        /* We also make use of the "filch_message" option flag */
        /* to make sure each message is on a line by itself XXX */
        
        /* Hack -- Extract the cursor visibility */
        visible = (!Term_hide_cursor());
        if (visible) Term_show_cursor();


        /* XXX XXX XXX Mega-Hack -- Catch "Die? [y/n]" messages */

        /* Hack -- cheat death */
        if (visible &&
            (0 == Term_locate(&x, &y)) && (y == 0) && (x >= 4) &&
            (0 == borg_what_text(0, 0, 4, &t_a, buf)) &&
            (prefix(buf, "Die?"))) {

            /* Take note */
            borg_note("# Cheating death...");

            /* Demo mode */
            if (!p_ptr->sc) {

                /* Increase "age" */
                if (++p_ptr->age >= 100) {

                    /* End the demo */
                    borg_oops("final death");
                }
            }

            /* Normal mode */
            else {

                /* Oops */
                borg_oops("normal death");
            }
            
            /* Cheat death */
            Term_keypress('n');

            /* Success */
            return (0);
        }


        /* XXX XXX XXX Mega-Hack -- Catch "-more-" messages */

        /* If the cursor is visible... */
        /* And the cursor is on the top line... */
        /* And there is text before the cursor... */
        /* And that text is "-more-" */
        if (visible &&
            (0 == Term_locate(&x, &y)) && (y == 0) && (x >= 6) &&
            (0 == borg_what_text(x-6, y, 6, &t_a, buf)) &&
            (streq(buf, "-more-"))) {

            /* Get the message */
            if (0 == borg_what_text(0, 0, -80, &t_a, buf)) {

                /* Parse it */
                borg_parse(buf);
            }

            /* Hack -- Clear the message */
            if (auto_active) Term_keypress(' ');

            /* Done */
            return (0);
        }


        /* XXX XXX XXX Mega-Hack -- catch normal messages */

        /* If the cursor is NOT visible... */
        /* And there is text on the first line... */
        if (!visible &&
            (0 == borg_what_text(0, 0, 3, &t_a, buf)) &&
            (t_a == TERM_WHITE) && buf[0] &&
             ((buf[0] != ' ') || (buf[2] != ' '))) {

            /* Get the message */
            if (0 == borg_what_text(0, 0, -80, &t_a, buf)) {

                /* Parse */
                borg_parse(buf);
            }

            /* Hack -- Clear the message */
            if (auto_active) Term_keypress(' ');

            /* Done */
            return (0);
        }


        /* Take the next keypress, if any */
        if ((i = borg_inkey()) != 0) {

            /* Enqueue the keypress */
            Term_keypress(i);

            /* Success */
            return (0);
        }


        /* Think */
        borg_think();
    }


    /* Hack -- "ignore" most actions */
    return ((*Term_xtra_hook_old)(n, v));
}



/*
 * Hack -- interact with the "Ben Borg".
 */
void borg_ben(void)
{
    char cmd;


    /* Paranoia */
    auto_active = FALSE;
    

    /* Verify use of the Borg */
    if (!(noscore & 0x0010)) {
    
        /* Mention effects */
        msg_print("The Borg is for debugging and experimenting.");
        msg_print("The game will not be scored if you use the Borg.");

        /* Verify request */
        if (!get_check("Are you sure you want to use the Borg? ")) return;

        /* Mark player as a Borg */
        noscore |= 0x0010;
    }


    /* Get a "Borg command", or abort */
    if (!get_com("Borg command: ", &cmd)) return;


    /* Hack -- Cancel wizard mode */
    if (wizard) {

        /* Mega-Hack -- Turn off wizard mode */
        wizard = FALSE;

        /* Mega-Hack -- Redraw Everything */
        do_cmd_redraw();
    }


    /* Hack -- force initialization */
    if (!initialized) borg_ben_init();


    /* Command: Nothing */
    if (cmd == '$') {

        /* Nothing */
    }


    /* Command: Activate */
    else if (cmd == 'z') {

        /* Activate (see below) */
        auto_active = TRUE;

        /* Mega-Hack -- Flush the borg key-queue */
        borg_flush();

        /* Mega-Hack -- make sure we are okay */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Mega-Hack -- ask for the feeling */
        borg_keypresses("^F");
    }


    /* Command: enter "demo" mode */
    else if (cmd == 'd') {

        /* Mark as cheating */
        if (p_ptr->sc) {

            /* Cheat death */
            p_ptr->sc = 0;

            /* Nuke the height */
            /* p_ptr->ht = 0; */

            /* Nuke the weight */
            /* p_ptr->wt = 0; */

            /* Reset the age XXX XXX XXX */
            p_ptr->age = 0;
            
            /* Set the "cheat death" flag XXX XXX XXX */
            cheat_live = TRUE;

            /* Mark the use of "cheat death" XXX XXX XXX */
            if (cheat_live) noscore |= 0x2000;
        }
        
        /* Note */
        msg_format("Borg -- demo mode.");
    }

    
    /* Command: toggle "cheat" flags */
    else if (cmd == 'c') {
    
        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Toggle Cheat: ", &cmd)) return;

        /* Toggle */
        if (cmd == 'i') {
            cheat_inven = !cheat_inven;
            msg_format("Borg -- cheat_inven is now %d.", cheat_inven);
        }
        else if (cmd == 'e') {
            cheat_equip = !cheat_equip;
            msg_format("Borg -- cheat_equip is now %d.", cheat_equip);
        }
        else if (cmd == 's') {
            cheat_spell = !cheat_spell;
            msg_format("Borg -- cheat_spell is now %d.", cheat_spell);
        }
        else if (cmd == 'p') {
            cheat_panel = !cheat_panel;
            msg_format("Borg -- cheat_panel is now %d.", cheat_panel);
        }
    }


    /* Command: toggle "panic" flags */
    else if (cmd == 'p') {
    
        /* Get a "Borg command", or abort */
        if (!get_com("Borg command: Toggle Panic: ", &cmd)) return;

        /* Command: toggle "panic_death" */
        if (cmd == 'd') {
            panic_death = !panic_death;
            msg_format("Borg -- panic_death is now %d.", panic_death);
        }

        /* Command: toggle "panic_stuff" */
        else if (cmd == 'k') {
            panic_stuff = !panic_stuff;
            msg_format("Borg -- panic_stuff is now %d.", panic_stuff);
        }

        /* Command: toggle "panic_power" */
        else if (cmd == 'p') {
            panic_power = !panic_power;
            msg_format("Borg -- panic_power is now %d.", panic_power);
        }
    }


    /* Start a new log file */
    else if (cmd == 'f') {

        char buf[1024];

        /* Close the log file */
        if (auto_fff) my_fclose(auto_fff);

        /* Make a new log file name */
        prt("Borg Log: ", 0, 0);

        /* Hack -- drop permissions */
        safe_setuid_drop();

        /* XXX XXX XXX Get the name and open the log file */
        if (askfor(buf, 70)) auto_fff = my_fopen(buf, "w");

        /* Hack -- grab permissions */
        safe_setuid_grab();

        /* Failure */
        if (!auto_fff) msg_print("Cannot open that file.");
    }


    /* Command: Show "kill" grids */
    else if (cmd == 'K') {

        int i, k = 0;

        /* Scan the kill list */
        for (i = 0; i < auto_kills_nxt; i++) {

            auto_kill *kill = &auto_kills[i];

            /* Show "live" monsters */
            if (kill->r_idx) {

                int x = kill->x;
                int y = kill->y;
                
                byte a = TERM_RED;
                char c = '*';

                /* Display the avoidance */
                print_rel(c, a, y, x);

                /* Count */
                k++;
           }
       }

       /* Message */
       msg_format("There are %d known monsters.", k);
       msg_print(NULL);
       
       /* Hack -- Redraw */
       do_cmd_redraw();
    }


    /* Command: Show "take" grids */
    else if (cmd == 'T') {

        int i, k = 0;

        /* Scan the take list */
        for (i = 0; i < auto_takes_nxt; i++) {

            auto_take *take = &auto_takes[i];

            /* Show "live" objects */
            if (take->k_idx) {

                int x = take->x;
                int y = take->y;
                
                byte a = TERM_RED;
                char c = '*';

                /* Display the avoidance */
                print_rel(c, a, y, x);

                /* Count */
                k++;
           }
       }

       /* Message */
       msg_format("There are %d known objects.", k);
       msg_print(NULL);
       
       /* Hack -- Redraw */
       do_cmd_redraw();
    }


#if 0

    /* Command: Show all Rooms (Containers) */
    else if (cmd == 'S') {

        int k, x, y, xx, yy;

        int w_y = panel_row * (SCREEN_HGT / 2);
        int w_x = panel_col * (SCREEN_WID / 2);

        auto_room *ar;

        /* Examine all the rooms */
        for (y = w_y; y < w_y + SCREEN_HGT; y++) {
            for (x = w_x; x < w_x + SCREEN_WID; x++) {

                /* Scan the rooms */
                for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

                    /* Skip done rooms */
                    if ((ar->y1 < y) && (y > w_y)) continue;
                    if ((ar->x1 < x) && (x > w_x)) continue;

                    /* Hack -- hilite the room -- count draws */
                    for (yy = ar->y1; yy <= ar->y2; yy++) {	
                        for (xx = ar->x1; xx <= ar->x2; xx++) {	
                            byte a = TERM_RED;
                            char c = '*';
                            if (grid(xx,yy)->info & BORG_VIEW) a = TERM_YELLOW;
                            if (grid(xx,yy)->o_c == ' ') c = '?';
                            print_rel(c, a, yy, xx);
                        }
                    }

                    /* Describe and wait */
                    Term_putstr(0, 0, -1, TERM_WHITE,
                                format("Room %d (%dx%d).", ar->self,
                                (1 + ar->x2 - ar->x1), (1 + ar->y2 - ar->y1)));

                    /* Get a key */
                    k = inkey();

                    /* Hack -- hilite the room -- count draws */
                    for (yy = ar->y1; yy <= ar->y2; yy++) {	
                        for (xx = ar->x1; xx <= ar->x2; xx++) {	
                            lite_spot(yy, xx);
                        }
                    }

                    /* Erase the prompt */
                    Term_erase(0, 0, 80, 1);

                    /* Flush the erase */
                    Term_fresh();

                    /* Leave the outer loop */
                    if (k == ESCAPE) x = y = 999;

                    /* Leave this loop */
                    if (k == ESCAPE) break;
                }
            }
        }
    }

    /* Command: Rooms (Containers) */
    else if (cmd == 'C') {

        int n, w, h;

        auto_room *ar;

        int used = 0, n_1x1 = 0, n_1xN = 0, n_Nx1 = 0;
        int n_2x2 = 0, n_2xN = 0, n_Nx2 = 0, n_NxN = 0;

        /* Examine all the rooms */
        for (n = 0; n < AUTO_ROOMS; n++) {

            /* Access the n'th room */
            ar = &auto_rooms[n];

            /* Skip "dead" rooms */
            if (ar->free) continue;

            /* Count the "used" rooms */
            used++;

            /* Extract the "size" */
            w = 1 + ar->x2 - ar->x1;
            h = 1 + ar->y2 - ar->y1;

            /* Count the "singles" */
            if ((w == 1) && (h == 1)) n_1x1++;
            else if (w == 1) n_1xN++;
            else if (h == 1) n_Nx1++;
            else if ((w == 2) && (h == 2)) n_2x2++;
            else if (w == 2) n_2xN++;
            else if (h == 2) n_Nx2++;
            else n_NxN++;
        }


        /* Display some info */	
        msg_format("Rooms: %d/%d used.", used, auto_room_max);
        msg_format("Corridors: 1xN (%d), Nx1 (%d)", n_1xN, n_Nx1);
        msg_format("Thickies: 2x2 (%d), 2xN (%d), Nx2 (%d)",
                         n_2x2, n_2xN, n_Nx2);
        msg_format("Singles: %d.  Normals: %d.", n_1x1, n_NxN);
        msg_print(NULL);
    }

    /* Command: Grid Info */
    else if (cmd == 'G') {

        int x, y, n;

        auto_room *ar;

        int tg = 0;
        int tr = 0;

        int cc[8];

        /* Count the crossing factors */
        cc[0] = cc[1] = cc[2] = cc[3] = cc[4] = cc[5] = cc[6] = cc[7] = 0;

        /* Total explored grids */
        for (y = 0; y < AUTO_MAX_Y; y++) {
            for (x = 0; x < AUTO_MAX_X; x++) {

                /* Get the grid */
                auto_grid *ag = grid(x, y);

                /* Skip unknown grids */
                if (ag->o_c == ' ') continue;

                /* Count them */
                tg++;

                /* No rooms yet */
                n = 0;

                /* Count the rooms this grid is in */
                for (ar = room(1,x,y); ar && (n<7); ar = room(0,0,0)) n++;

                /* Hack -- Mention some locations */
                if ((n > 1) && !cc[n]) {
                    msg_format("The grid at %d,%d is in %d rooms.", x, y, n);
                }

                /* Count them */
                if (n) tr++;

                /* Count the number of grids in that many rooms */
                cc[n]++;
            }
        }

        /* Display some info */	
        msg_format("Grids: %d known, %d in rooms.", tg, tr);
        msg_format("Roomies: %d, %d, %d, %d, %d, %d, %d, %d.",
                   cc[0], cc[1], cc[2], cc[3],
                   cc[4], cc[5], cc[6], cc[7]);
        msg_print(NULL);
    }

#endif

    /* Oops */
    else {

        /* Message */
        msg_print("That is not a legal Borg command.");
    }
}



/*
 * Initialize the "Ben Borg"
 */
void borg_ben_init(void)
{
    byte *test;


    /* Clear messages */
    msg_print(NULL);
    
    /* Message */
    prt("Initializing the Borg... (memory)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();

    /* Mega-Hack -- verify memory */
    C_MAKE(test, 400 * 1024L, byte);
    C_KILL(test, 400 * 1024L, byte);


    /*** Hack -- initialize game options ***/

    /* Message */
    prt("Initializing the Borg... (options)", 0, 0);

    /* Hack -- flush it */
    Term_fresh();

    /* We use the original keypress codes */
    rogue_like_commands = FALSE;

    /* We pick up items when we step on them */
    always_pickup = TRUE;

    /* We require explicit target request */
    use_old_target = FALSE;

    /* We do NOT verify throwing */
    always_throw = TRUE;

    /* We do NOT query any commands */
    carry_query_flag = FALSE;
    other_query_flag = FALSE;

    /* We do NOT auto-repeat any commands */
    always_repeat = FALSE;

    /* We do not know how to haggle */
    no_haggle_flag = TRUE;

    /* We need as much information as possible */
    fresh_before = TRUE;
    fresh_after = TRUE;

    /* We need to see each message by itself */
    filch_message = TRUE;    

    /* We do not want extra confusing info */
    plain_descriptions = TRUE;

    /* We need maximal space for descriptions */
    show_inven_weight = FALSE;
    show_equip_weight = FALSE;
    show_store_weight = FALSE;
    show_equip_label = FALSE;
    
    /* We use color to identify monsters and objects */
    use_color = TRUE;

    /* We do not want to be confused by lighting effects */
    view_bright_lite = FALSE;
    view_yellow_lite = FALSE;

    /* We need to see the actual dungeon level */
    depth_in_feet = FALSE;

    /* We may use the health bar later */
    show_health_bar = TRUE;

    /* We do not want to get confused by extra spell info */
    show_spell_info = FALSE;

    /* We do not want to get confused by equippy chars */
    equippy_chars = FALSE;

    /* Allow items to stack */
    stack_allow_items = TRUE;
    
    /* Allow wands to stack */
    stack_allow_wands = TRUE;
    
    /* Do not ignore discounts */
    stack_force_costs = FALSE;
    
    /* Ignore inscriptions */
    stack_force_notes = TRUE;

    /* Ignore annoying hitpoint warnings */
    hitpoint_warn = 0;

    /* Hack -- notice "command" mode */
    hilite_player = FALSE;

    /* Hack -- reset visuals */
    reset_visuals();

    /* Hack -- Redraw */
    do_cmd_redraw();


    /*** Various ***/
    
    /* Message */
    prt("Initializing the Borg...", 0, 0);

    /* Hack -- flush it */
    Term_fresh();


    /*** Cheat / Panic ***/

    /* Mega-Hack -- Cheat a lot */
    cheat_inven = TRUE;
    cheat_equip = TRUE;

    /* Mega-Hack -- Cheat a lot */
    cheat_spell = TRUE;

    /* Mega-Hack -- Cheat a lot */
    cheat_panel = TRUE;


    /*** Insert special hook function ***/

    /* Remember the "normal" event scanner */
    Term_xtra_hook_old = Term->xtra_hook;

    /* Cheat -- drop a hook into the "event scanner" */
    Term->xtra_hook = Term_xtra_borg;


    /*** Initialize ***/

    /* Init "borg.c" */
    borg_init();

    /* Init "borg-map.c" */
    borg_map_init();

    /* Init "borg-obj.c" */
    borg_obj_init();

    /* Init "borg-ext.c" */
    borg_ext_init();

    /* Init "borg-aux.c" */
    borg_aux_init();


    /*** All done ***/

    /* Done initialization */
    prt("Initializing the Borg... done.", 0, 0);

    /* Official message */
    borg_note("# Ready...");

    /* Now it is ready */
    initialized = TRUE;
}


#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

