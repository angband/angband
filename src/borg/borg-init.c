/**
 * \file borg-init.c
 * \brief Full initialization of all borg variables
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

#include "borg-init.h"

#ifdef ALLOW_BORG

#include "../player-calcs.h"
#include "../ui-command.h"
#include "../ui-keymap.h"
#include "../ui-output.h"
#include "../ui-prefs.h"

#include "borg-cave.h"
#include "borg-flow-kill.h"
#include "borg-flow-take.h"
#include "borg-flow.h"
#include "borg-formulas.h"
#include "borg-io.h"
#include "borg-item-activation.h"
#include "borg-item-val.h"
#include "borg-item-wear.h"
#include "borg-magic.h"
#include "borg-messages.h"
#include "borg-power.h"
#include "borg-store.h"
#include "borg-update.h"
#include "borg.h"

bool borg_initialized; /* Hack -- Initialized */
bool game_closed; /* Has the game been closed since the borg was
                      initialized */

bool borg_init_failure = false;

/*
 * Borg settings information.
 * NOTE: Must match enum in borg.h
 */
struct borg_setting borg_settings[] = {
    { "borg_verbose", 'b', false },
    { "borg_munchkin_start", 'b', false },
    { "borg_munchkin_level", 'i', 12 }, 
    { "borg_munchkin_depth", 'i', 16 },
    { "borg_worships_damage", 'b', false },
    { "borg_worships_speed", 'b', false },
    { "borg_worships_hp", 'b', false },
    { "borg_worships_mana", 'b', false },
    { "borg_worships_ac", 'b', false },
    { "borg_worships_gold", 'b', false },
    { "borg_plays_risky", 'b', false },
    { "borg_kills_uniques", 'b', false }, 
    { "borg_uses_swaps", 'b', true },
    { "borg_uses_dynamic_calcs", 'b', false },
    { "borg_stop_dlevel", 'i', 128 }, 
    { "borg_stop_clevel", 'i', 51 },
    { "borg_no_deeper", 'i', 127 }, 
    { "borg_stop_king", 'b', true },
    { "borg_cheat_death", 'b', false },
    { "borg_respawn_winners", 'b', false },
    { "borg_respawn_class", 'i', -1 }, 
    { "borg_respawn_race", 'i', -1 },
    { "borg_chest_fail_tolerance", 'i', 7 },
    { "borg_delay_factor", 'i', 0 }, 
    { "borg_money_scum_amount", 'i', 0 },
    { "borg_self_scum", 'b', true }, 
    { "borg_lunal_mode", 'b', false },
    { "borg_self_lunal", 'b', false }, 
    { "borg_enchant_limit", 'i', 12 },
    { "borg_dump_level", 'i', 1 }, 
    { "borg_save_death", 'i', 1 },
    { "borg_stop_on_bell", 'b', false },
    { "borg_allow_strange_opts", 'b', false},
    { "borg_autosave", 'b', false},
    { 0, 0, 0 }};


/*
 * read a line from the config file and parse the setting value, if there is
 * one.
 */
static bool borg_proc_setting(
    int setting, const char *setting_string, char type, const char *line)
{
    bool negative = false;

    /* skip leading space */
    while (*line == ' ')
        line++;
    if (!prefix_i(line, setting_string))
        return false;

    line += strlen(setting_string);

    /* accept either */
    /* value true or */
    /* value=true or */
    /* value=1 or */
    /* value y */
    while (*line) {
        char ch = *line;
        if (ch == ' ' || ch == '=') {
            line++;
            continue;
        }

        if (type == 'b') {
            if (ch == 'T' || ch == 't' || ch == '1' || ch == 'Y' || ch == 'y')
                borg_cfg[setting] = true;
            else
                borg_cfg[setting] = false;

            break;
        }
        if (type == 'i') {
            if (ch == '-') {
                negative = true;
                line++;
                continue;
            }
            if (isdigit(ch)) {
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
 * Release resources allocated by borg_init_txt_file().
 */
static void borg_free_txt_file(void)
{
    borg_trait_free();
    borg_free_formulas();

    mem_free(borg_cfg);
    borg_cfg = NULL;
}

/*
 * Initialize borg.txt
 */
bool borg_init_txt_file(void)
{
    ang_file *fp;

    bool formulas_found = false;
    bool old_formulas_found = false;
    int i;
    bool warning = false;


    if (borg_active)
        borg_free_txt_file();

    borg_trait_init();

    /* a couple of spot checks on settings definitions */
    if (!streq(borg_settings[BORG_MUNCHKIN_LEVEL].setting_string,
            "borg_munchkin_level")
        || !streq(borg_settings[BORG_RESPAWN_RACE].setting_string,
            "borg_respawn_race")) {
        msg("borg settings structures not correct.  aborting. ");
        borg_init_failure = true;
    }

    char buf[1024];
    path_build(buf, 1024, ANGBAND_DIR_USER, "borg.txt");

    /* Open the file */
    fp = file_open(buf, MODE_READ, -1);

    /* No file, use defaults*/
    if (!fp) {
        /* Complain */
        msg("*****WARNING***** You do not have a proper BORG.TXT file!");
        msg("writing borg.txt to the \\user\\ subdirectory with default "
            "values");
        msg("Which is: %s", buf);
        event_signal(EVENT_MESSAGE_FLUSH);

        fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);
        if (!fp) {
            borg_warning("*****WARNING***** unable to write default BORG.TXT file!");
            return true;
        }
        file_putf(fp, "# BORG.txt default settings \n");
        file_putf(
            fp, "# A more descriptive version of this file is delivered. \n");
        file_putf(fp, "# Check your original install for borg.txt and \n");
        file_putf(fp, "# replace this one with the delivered version. \n\n");

        for (i = 0; i < BORG_MAX_SETTINGS; i++) {
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
    while (file_getl(fp, buf, sizeof(buf) - 1)) {
        /* Skip comments and blank lines */
        if (!buf[0] || (buf[0] == '#'))
            continue;

        /* Chop the buffer */
        buf[sizeof(buf) - 1] = '\0';

        for (i = 0; i < BORG_MAX_SETTINGS; i++) {
            if (borg_proc_setting(i, borg_settings[i].setting_string,
                    borg_settings[i].setting_type, buf))
                break;
        }
        if (i != BORG_MAX_SETTINGS)
            continue;

        /* other settings */
        if (prefix_i(buf, "REQ")) {
            old_formulas_found = true;
            continue;
        }
        if (prefix_i(buf, "FORMULA")) {
            old_formulas_found = true;
            continue;
        }
        if (prefix_i(buf, "CND")) {
            old_formulas_found = true;
            continue;
        }
        if (prefix_i(buf, "POWER")) {
            old_formulas_found = true;
            continue;
        }

        if (prefix_i(buf, "[BEGIN FORMULA SECTION]")) {
            formulas_found = true;
            borg_load_formulas(fp);
        }
    }

    /* Close it */
    file_close(fp);

    if (borg_cfg[BORG_USES_DYNAMIC_CALCS]) {
        warning = true;
        if (old_formulas_found) {
            borg_warning(
                "** Borg's dynamic calculations enabled but old formulas "
                "found in borg.txt.");
            borg_warning("** formulas disabled ** ");
            borg_cfg[BORG_USES_DYNAMIC_CALCS] = false;
        } else if (!formulas_found) {
            borg_warning(
                "** Borg's dynamic calculations enabled but no formulas "
                "found in borg.txt.");
            borg_warning("** formulas disabled ** ");
            borg_cfg[BORG_USES_DYNAMIC_CALCS] = false;
        } else {
            borg_warning(
                "Borg's dynamic calculations enabled.  You may see some "
                "performance loss (~20 percent).");
        }

        /* Hack -- flush it */
        Term_fresh();
    }

    /* lunal mode is a default rather than a setting */
    borg.lunal_mode = borg_cfg[BORG_LUNAL_MODE];

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

    if (borg_cfg[BORG_RESPAWN_RACE] >= MAX_RACES
        || borg_cfg[BORG_RESPAWN_RACE] < -1)
        borg_cfg[BORG_RESPAWN_RACE] = 0;

    if (borg_cfg[BORG_RESPAWN_CLASS] >= MAX_CLASSES
        || borg_cfg[BORG_RESPAWN_CLASS] < -1)
        borg_cfg[BORG_RESPAWN_CLASS] = 0;

    /* make sure it continues to run if reset */
    if (borg_cfg[BORG_RESPAWN_WINNERS])
        borg_cfg[BORG_STOP_KING] = false;

    /* Success */
    return warning;
}

/*
 * Reset the required options when returning from user control
 */
void borg_reinit_options(void)
{
    /* Save current key mode */
    key_mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE
                                                : KEYMAP_MODE_ORIG;

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
static void borg_leave_game(
    game_event_type ev_type, game_event_data *ev_data, void *user)
{
    assert(ev_type == EVENT_LEAVE_GAME && ev_data == NULL && user == NULL);
    game_closed = true;
}

/*
 * Hack -- prepare some stuff based on the player race and class
 */
void borg_prepare_race_class_info(void)
{
    /* Initialize the various spell arrays by book */
    borg_prepare_book_info();
}

/*
 * Initialize the Borg
 */
void borg_init(void)
{
    uint8_t *memory_test;
    bool    warning_given;

    /*** Hack -- verify system ***/
    /* Redraw everything */
    do_cmd_redraw();

    /* Message */
    borg_note("Initializing the Borg... (memory)");
    borg_init_failure = false;

    /* Hack -- flush it */
    Term_fresh();

    /* Mega-Hack -- verify memory */
    memory_test = mem_zalloc(400 * 1024L * sizeof(uint8_t));
    mem_free(memory_test);
    memory_test = NULL;

    /* Prapare a local random number seed */
    if (!borg_rand_local)
        borg_rand_local = randint1(0x10000000);

    borg.player = player; /* HACK work around msvc issue */

    /*** Hack -- initialize borg.ini options ***/

    /* Message */
    borg_note("Initializing the Borg... (borg.txt)");
    warning_given = borg_init_txt_file();

    /*** Hack -- initialize game options ***/

    /* Message */
    borg_note("Initializing the Borg... (options)");

    /* Hack -- flush it */
    Term_fresh();

    /* Make sure it rolls up a new guy at death */
    if (screensaver || borg_cfg[BORG_CHEAT_DEATH]) {
        /* We need the borg to keep playing after he dies */
        option_set("cheat_live", true);
    }

#ifndef ALLOW_BORG_GRAPHICS
    if (!borg_graphics) {
        int i, j;
        /* Reset the # and % -- Scan the features */
        for (i = 1; i < FEAT_MAX; i++) {
            struct feature *f_ptr = &f_info[i];
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
    if (use_graphics) {
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

    /*** Various ***/

    /* Message */
    borg_note("Initializing the Borg... (various)");

    /* Hack -- flush it */
    Term_fresh();

    /*** Cheat / Panic ***/

    /* more cheating */
    borg_cheat_death = false;

    /* set the continuous play mode if the game cheat death is on */
    if (OPT(player, cheat_live))
        borg_cheat_death = true;

    /*** Initialize ***/

    borg_init_messages();

    /* Initialize */
    borg_init_io();
    borg_init_item_val();
    borg_init_item_activation();

    /*** Grids ***/
    borg_init_cave();
    borg_init_flow();

    borg_init_item_wear();
    borg_init_flow_take();
    borg_init_flow_kill();

    borg_init_item();
    borg_init_store();
    /*** Object/Monster tracking ***/
    borg_init_update();

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
    borg_note("Initializing the Borg... done.");

    /* Reset the clock */
    borg_t = 10;

    /* note: I would check if player_id2class returns null but it */
    /* never does, even on a bad class */
    if (!streq(player_id2class(CLASS_WARRIOR)->name, "Warrior")
        || !streq(player_id2class(CLASS_MAGE)->name, "Mage")
        || !streq(player_id2class(CLASS_DRUID)->name, "Druid")
        || !streq(player_id2class(CLASS_PRIEST)->name, "Priest")
        || !streq(player_id2class(CLASS_NECROMANCER)->name, "Necromancer")
        || !streq(player_id2class(CLASS_PALADIN)->name, "Paladin")
        || !streq(player_id2class(CLASS_ROGUE)->name, "Rogue")
        || !streq(player_id2class(CLASS_RANGER)->name, "Ranger")
        || !streq(player_id2class(CLASS_BLACKGUARD)->name, "Blackguard")) {
        borg_note("**STARTUP FAILURE** classes do not match");
        borg_init_failure = true;
    }

    /* Don't allow the user to do stupid things unless they ask to */
    if (!borg_cfg[BORG_ALLOW_STRANGE_OPTS]) {
        if (OPT(player, birth_force_descend)) {
            borg_note("**STARTUP FAILURE** must allow up stairs");
            borg_note("** birth option failure **");
            borg_init_failure = true;
        }

        if (!OPT(player, birth_connect_stairs)) {
            borg_note("**STARTUP FAILURE** must connect stairs");
            borg_note("** birth option failure **");
            borg_init_failure = true;
        }

        if (OPT(player, birth_no_recall)) {
            borg_note("**STARTUP FAILURE** must allow recall");
            borg_note("** birth option failure **");
            borg_init_failure = true;
        }

        if (OPT(player, birth_percent_damage)) {
            borg_note("**STARTUP FAILURE** strange damage calculation");
            borg_note("** birth option failure **");
            borg_init_failure = true;
        }
    }

    /* Official message */
    if (!borg_init_failure && !warning_given)
        borg_note("# Ready...");

    /* Now it is ready */
    borg_initialized = true;
}

/*
 * Clean up resources allocated for the borg.
 */
void borg_free(void)
{
    /* Undo the allocations in reverse order from what borg_init() does. */
    event_remove_handler(EVENT_LEAVE_GAME, borg_leave_game, NULL);
    borg_free_update();
    borg_free_item();
    borg_free_store();

    borg_free_flow_kill();
    borg_free_flow_take();
    borg_free_item_wear();

    borg_free_flow();
    borg_free_cave();
    borg_free_io();

    borg_free_messages();
    borg_free_txt_file();
}

#endif
