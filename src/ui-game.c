/**
 * \file ui-game.c
 * \brief Game management for the traditional text UI
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2015 Nick McConnell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cmds.h"
#include "datafile.h"
#include "game-world.h"
#include "grafmode.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-path.h"
#include "player-util.h"
#include "savefile.h"
#include "target.h"
#include "ui-birth.h"
#include "ui-command.h"
#include "ui-context.h"
#include "ui-death.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-help.h"
#include "ui-init.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-knowledge.h"
#include "ui-map.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-player.h"
#include "ui-prefs.h"
#include "ui-spell.h"
#include "ui-score.h"
#include "ui-signals.h"
#include "ui-store.h"
#include "ui-target.h"


bool arg_wizard;			/* Command arg -- Request wizard mode */

/**
 * Buffer to hold the current savefile name
 */
char savefile[1024];


/**
 * Here are lists of commands, stored in this format so that they can be
 * easily maniuplated for e.g. help displays, or if a port wants to provide a
 * native menu containing a command list.
 *
 * Consider a two-paned layout for the command menus. XXX
 */

/**
 * Item commands
 */
struct cmd_info cmd_item[] =
{
	{ "Inscribe an object", { '{' }, CMD_INSCRIBE, NULL, NULL },
	{ "Uninscribe an object", { '}' }, CMD_UNINSCRIBE, NULL, NULL },
	{ "Wear/wield an item", { 'w' }, CMD_WIELD, NULL, NULL },
	{ "Take off/unwield an item", { 't', 'T'}, CMD_TAKEOFF, NULL, NULL },
	{ "Examine an item", { 'I' }, CMD_NULL, textui_obj_examine, NULL },
	{ "Drop an item", { 'd' }, CMD_DROP, NULL, NULL },
	{ "Fire your missile weapon", { 'f', 't' }, CMD_FIRE, NULL, player_can_fire_prereq },
	{ "Use a staff", { 'u', 'Z' }, CMD_USE_STAFF, NULL, NULL },
	{ "Aim a wand", {'a', 'z'}, CMD_USE_WAND, NULL, NULL },
	{ "Zap a rod", {'z', 'a'}, CMD_USE_ROD, NULL, NULL },
	{ "Activate an object", {'A' }, CMD_ACTIVATE, NULL, NULL },
	{ "Eat some food", { 'E' }, CMD_EAT, NULL, NULL },
	{ "Quaff a potion", { 'q' }, CMD_QUAFF, NULL, NULL },
	{ "Read a scroll", { 'r' }, CMD_READ_SCROLL, NULL, player_can_read_prereq },
	{ "Fuel your light source", { 'F' }, CMD_REFILL, NULL, player_can_refuel_prereq },
	{ "Use an item", { 'U', 'X' }, CMD_USE, NULL, NULL }
};

/**
 * General actions
 */
struct cmd_info cmd_action[] =
{
	{ "Disarm a trap or chest", { 'D' }, CMD_DISARM, NULL, NULL },
	{ "Rest for a while", { 'R' }, CMD_NULL, textui_cmd_rest, NULL },
	{ "Look around", { 'l', 'x' }, CMD_NULL, do_cmd_look, NULL },
	{ "Target monster or location", { '*' }, CMD_NULL, textui_target, NULL },
	{ "Target closest monster", { '\'' }, CMD_NULL, textui_target_closest, NULL },
	{ "Dig a tunnel", { 'T', KTRL('T') }, CMD_TUNNEL, NULL, NULL },
	{ "Go up staircase", {'<' }, CMD_GO_UP, NULL, NULL },
	{ "Go down staircase", { '>' }, CMD_GO_DOWN, NULL, NULL },
	{ "Open a door or a chest", { 'o' }, CMD_OPEN, NULL, NULL },
	{ "Close a door", { 'c' }, CMD_CLOSE, NULL, NULL },
	{ "Fire at nearest target", { 'h', KC_TAB }, CMD_NULL, do_cmd_fire_at_nearest, NULL },
	{ "Throw an item", { 'v' }, CMD_THROW, NULL, NULL },
	{ "Walk into a trap", { 'W', '-' }, CMD_JUMP, NULL, NULL },
};

/**
 * Item management commands
 */
struct cmd_info cmd_item_manage[] =
{
	{ "Display equipment listing", { 'e' }, CMD_NULL, do_cmd_equip, NULL },
	{ "Display inventory listing", { 'i' }, CMD_NULL, do_cmd_inven, NULL },
	{ "Display quiver listing", { '|' }, CMD_NULL, do_cmd_quiver, NULL },
	{ "Pick up objects", { 'g' }, CMD_PICKUP, NULL, NULL },
	{ "Ignore an item", { 'k', KTRL('D') }, CMD_IGNORE, textui_cmd_ignore, NULL },
};

/**
 * Information access commands
 */
struct cmd_info cmd_info[] =
{
	{ "Browse a book", { 'b', 'P' }, CMD_BROWSE_SPELL, textui_spell_browse, NULL },
	{ "Gain new spells", { 'G' }, CMD_STUDY, NULL, player_can_study_prereq },
	{ "Cast a spell", { 'm' }, CMD_CAST, NULL, player_can_cast_prereq },
	{ "Cast a spell", { 'p' }, CMD_CAST, NULL, player_can_cast_prereq },
	{ "Full dungeon map", { 'M' }, CMD_NULL, do_cmd_view_map, NULL },
	{ "Toggle ignoring of items", { 'K', 'O' }, CMD_NULL, textui_cmd_toggle_ignore, NULL },
	{ "Display visible item list", { ']' }, CMD_NULL, do_cmd_itemlist, NULL },
	{ "Display visible monster list", { '[' }, CMD_NULL, do_cmd_monlist, NULL },
	{ "Locate player on map", { 'L', 'W' }, CMD_NULL, do_cmd_locate, NULL },
	{ "Help", { '?' }, CMD_NULL, do_cmd_help, NULL },
	{ "Identify symbol", { '/' }, CMD_NULL, do_cmd_query_symbol, NULL },
	{ "Character description", { 'C' }, CMD_NULL, do_cmd_change_name, NULL },
	{ "Check knowledge", { '~' }, CMD_NULL, textui_browse_knowledge, NULL },
	{ "Repeat level feeling", { KTRL('F') }, CMD_NULL, do_cmd_feeling, NULL },
	{ "Show previous message", { KTRL('O') }, CMD_NULL, do_cmd_message_one, NULL },
	{ "Show previous messages", { KTRL('P') }, CMD_NULL, do_cmd_messages, NULL }
};

/**
 * Utility/assorted commands
 */
struct cmd_info cmd_util[] =
{
	{ "Interact with options", { '=' }, CMD_NULL, do_cmd_xxx_options, NULL },

	{ "Save and don't quit", { KTRL('S') }, CMD_NULL, save_game, NULL },
	{ "Save and quit", { KTRL('X') }, CMD_NULL, textui_quit, NULL },
	{ "Kill character and quit", { 'Q' }, CMD_NULL, textui_cmd_suicide, NULL },
	{ "Redraw the screen", { KTRL('R') }, CMD_NULL, do_cmd_redraw, NULL },

	{ "Save \"screen dump\"", { ')' }, CMD_NULL, do_cmd_save_screen, NULL }
};

/**
 * Commands that shouldn't be shown to the user
 */
struct cmd_info cmd_hidden[] =
{
	{ "Take notes", { ':' }, CMD_NULL, do_cmd_note, NULL },
	{ "Version info", { 'V' }, CMD_NULL, do_cmd_version, NULL },
	{ "Load a single pref line", { '"' }, CMD_NULL, do_cmd_pref, NULL },
	{ "Toggle windows", { KTRL('E') }, CMD_NULL, toggle_inven_equip, NULL }, /* XXX */
	{ "Alter a grid", { '+' }, CMD_ALTER, NULL, NULL },
	{ "Steal from a monster", { 's' }, CMD_STEAL, NULL, NULL },
	{ "Walk", { ';' }, CMD_WALK, NULL, NULL },
	{ "Start running", { '.', ',' }, CMD_RUN, NULL, NULL },
	{ "Stand still", { ',', '.' }, CMD_HOLD, NULL, NULL },
	{ "Center map", { KTRL('L'), '@' }, CMD_NULL, do_cmd_center_map, NULL },
	{ "Toggle wizard mode", { KTRL('W') }, CMD_NULL, do_cmd_wizard, NULL },
	{ "Repeat previous command", { 'n', KTRL('V') }, CMD_REPEAT, NULL, NULL },
	{ "Do autopickup", { KTRL('G') }, CMD_AUTOPICKUP, NULL, NULL },
	{ "Debug mode commands", { KTRL('A') }, CMD_NULL, textui_cmd_debug, NULL },
};

/**
 * List of command lists */
struct command_list cmds_all[] =
{
	{ "Items",           cmd_item,        N_ELEMENTS(cmd_item) },
	{ "Action commands", cmd_action,      N_ELEMENTS(cmd_action) },
	{ "Manage items",    cmd_item_manage, N_ELEMENTS(cmd_item_manage) },
	{ "Information",     cmd_info,        N_ELEMENTS(cmd_info) },
	{ "Utility",         cmd_util,        N_ELEMENTS(cmd_util) },
	{ "Hidden",          cmd_hidden,      N_ELEMENTS(cmd_hidden) },
	{ NULL,              NULL,            0 }
};



/*** Exported functions ***/

#define KEYMAP_MAX 2

/* List indexed by char */
static struct cmd_info *converted_list[KEYMAP_MAX][UCHAR_MAX+1];


/**
 * Initialise the command list.
 */
void cmd_init(void)
{
	size_t i, j;

	memset(converted_list, 0, sizeof(converted_list));

	/* Go through all generic commands (-1 for NULL end entry) */
	for (j = 0; j < N_ELEMENTS(cmds_all) - 1; j++)
	{
		struct cmd_info *commands = cmds_all[j].list;

		/* Fill everything in */
		for (i = 0; i < cmds_all[j].len; i++) {
			/* If a roguelike key isn't set, use default */
			if (!commands[i].key[1])
				commands[i].key[1] = commands[i].key[0];

			converted_list[0][commands[i].key[0]] = &commands[i];
			converted_list[1][commands[i].key[1]] = &commands[i];
		}
	}
}

unsigned char cmd_lookup_key(cmd_code lookup_cmd, int mode)
{
	unsigned int i;

	assert(mode == KEYMAP_MODE_ROGUE || mode == KEYMAP_MODE_ORIG);

	for (i = 0; i < N_ELEMENTS(converted_list[mode]); i++) {
		struct cmd_info *cmd = converted_list[mode][i];

		if (cmd && cmd->cmd == lookup_cmd)
			return cmd->key[mode];
	}

	return 0;
}

unsigned char cmd_lookup_key_unktrl(cmd_code lookup_cmd, int mode)
{
	unsigned char c = cmd_lookup_key(lookup_cmd, mode);

	if (c < 0x20)
		c = UN_KTRL(c);

	return c;
}

cmd_code cmd_lookup(unsigned char key, int mode)
{
	assert(mode == KEYMAP_MODE_ROGUE || mode == KEYMAP_MODE_ORIG);

	if (!converted_list[mode][key])
		return CMD_NULL;

	return converted_list[mode][key]->cmd;
}


/**
 * Parse and execute the current command
 * Give "Warning" on illegal commands.
 */
void textui_process_command(void)
{
	int count = 0;
	bool done = true;
	ui_event e = textui_get_command(&count);
	struct cmd_info *cmd = NULL;
	unsigned char key = '\0';
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	switch (e.type) {
		case EVT_RESIZE: do_cmd_redraw(); return;
		case EVT_MOUSE: textui_process_click(e); return;
		case EVT_BUTTON:
		case EVT_KBRD: done = textui_process_key(e.key, &key, count); break;
		default: ;
	}

	/* Null command */
	if (!key && done)
		return;

	if (key == KC_ENTER) {
		/* Use command menus */
		cmd = textui_action_menu_choose();
	} else {
		/* Command key */
		cmd = converted_list[mode][key];
	}

	if (cmd && done) {
		/* Confirm for worn equipment inscriptions, check command prereqs */
		if (!key_confirm_command(key) || (cmd->prereq && !cmd->prereq()))
			cmd = NULL;

		/* Split on type of command */
		if (cmd && cmd->hook)
			/* UI command */
			cmd->hook();
		else if (cmd && cmd->cmd)
			/* Game command */
			cmdq_push_repeat(cmd->cmd, count);
	} else
		/* Error */
		do_cmd_unknown();
}

errr textui_get_cmd(cmd_context context)
{
	if (context == CTX_GAME)
		textui_process_command();

	/* If we've reached here, we haven't got a command. */
	return 1;
}


/**
 * Allow for user abort during repeated commands, running and resting.
 *
 * This will only check during every 128th game turn while resting.
 */
void check_for_player_interrupt(game_event_type type, game_event_data *data,
								void *user)
{
	/* Check for "player abort" */
	if (player->upkeep->running ||
	    cmd_get_nrepeats() > 0 ||
	    (player_is_resting(player) && !(turn & 0x7F))) {
		ui_event e;

		/* Do not wait */
		inkey_scan = SCAN_INSTANT;

		/* Check for a key */
		e = inkey_ex();
		if (e.type != EVT_NONE) {
			/* Flush and disturb */
			event_signal(EVENT_INPUT_FLUSH);
			disturb(player, 0);
			msg("Cancelled.");
		}
	}
}

void pre_turn_refresh(void)
{
	term *old = Term;
	int j;
	if (character_dungeon) {
		/* Redraw map */
		player->upkeep->redraw |= (PR_MAP | PR_STATE);
		player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);
		handle_stuff(player);

		if (OPT(player, show_target) && target_sighted()) {
			struct loc target;
			target_get(&target);
			move_cursor_relative(target.y, target.x);
		} else {
			move_cursor_relative(player->grid.y, player->grid.x);
		}

		for (j = 0; j < ANGBAND_TERM_MAX; j++) {
			if (!angband_term[j]) continue;

			Term_activate(angband_term[j]);
			Term_fresh();
		}
	}
	Term_activate(old);
}

/**
 * Start actually playing a game, either by loading a savefile or creating
 * a new character
 */
static void start_game(bool new_game)
{
	/* Player will be resuscitated if living in the savefile */
	player->is_dead = true;

	/* Try loading */
	if (file_exists(savefile) && !savefile_load(savefile, arg_wizard))
		quit("Broken savefile");

	/* No living character loaded */
	if (player->is_dead || new_game) {
		character_generated = false;
		textui_do_birth();
	}

	/* Tell the UI we've started. */
	event_signal(EVENT_LEAVE_INIT);
	event_signal(EVENT_ENTER_GAME);
	event_signal(EVENT_ENTER_WORLD);

	/* Save not required yet. */
	player->upkeep->autosave = false;

	/* Enter the level, generating a new one if needed */
	if (!character_dungeon) {
		prepare_next_level(&cave, player);
	}
	on_new_level();
}

/**
 * Play Angband
 */
void play_game(bool new_game)
{
	play_again = false;

	/* Load a savefile or birth a character, or both */
	start_game(new_game);

	/* Get commands from the user, then process the game world until the
	 * command queue is empty and a new player command is needed */
	while (!player->is_dead && player->upkeep->playing) {
		pre_turn_refresh();
		cmd_get_hook(CTX_GAME);
		run_game_loop();
	}

	/* Close game on death or quitting */
	close_game();

	if (play_again) {
		cleanup_angband();
		init_display();
		init_angband();
		textui_init();
		play_game(true);
	}
}

/**
 * Set the savefile name.
 */
void savefile_set_name(const char *fname, bool make_safe, bool strip_suffix)
{
	char path[128];
	size_t pathlen = sizeof path;
	size_t off = 0;

#if defined(SETGID)
	/*
	 * On SETGID systems, we prefix the filename with the user's UID so we
	 * know whose is whose.
	 */

	strnfmt(path, pathlen, "%d.", player_uid);
	off = strlen(path);
	pathlen -= off;
#endif

	if (make_safe) {
		player_safe_name(path + off, pathlen, fname, strip_suffix);
	} else {
		my_strcpy(path + off, fname, pathlen);
	}

	/* Save the path */
	path_build(savefile, sizeof(savefile), ANGBAND_DIR_SAVE, path);
}

/**
 * Save the game
 */
void save_game(void)
{
	char path[1024];

	/* Disturb the player */
	disturb(player, 1);

	/* Clear messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Handle stuff */
	handle_stuff(player);

	/* Message */
	prt("Saving game...", 0, 0);

	/* Refresh */
	Term_fresh();

	/* The player is not dead */
	my_strcpy(player->died_from, "(saved)", sizeof(player->died_from));

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Save the player */
	if (savefile_save(savefile))
		prt("Saving game... done.", 0, 0);
	else
		prt("Saving game... failed!", 0, 0);

	/* Refresh */
	Term_fresh();

	/* Allow suspend again */
	signals_handle_tstp();

	/* Save the window prefs */
	path_build(path, sizeof(path), ANGBAND_DIR_USER, "window.prf");
	if (!prefs_save(path, option_dump, "Dump window settings"))
		prt("Failed to save subwindow preferences", 0, 0);

	/* Refresh */
	Term_fresh();

	/* Save monster memory to user directory */
	if (!lore_save("lore.txt")) {
		msg("lore save failed!");
		event_signal(EVENT_MESSAGE_FLUSH);
	}

	/* Refresh */
	Term_fresh();

	/* Note that the player is not dead */
	my_strcpy(player->died_from, "(alive and well)", sizeof(player->died_from));
}



/**
 * Close up the current game (player may or may not be dead)
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(void)
{
	/* Tell the UI we're done with the world */
	event_signal(EVENT_LEAVE_WORLD);

	/* Handle stuff */
	handle_stuff(player);

	/* Flush the messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Flush the input */
	event_signal(EVENT_INPUT_FLUSH);

	/* No suspending now */
	signals_ignore_tstp();

	/* Hack -- Increase "icky" depth */
	screen_save_depth++;

	/* Deal with the randarts file */
	if (OPT(player, birth_randarts)) {
		deactivate_randart_file();
	}

	/* Handle death or life */
	if (player->is_dead) {
		death_knowledge(player);
		death_screen();

		/* Save dead player */
		if (!savefile_save(savefile)) {
			msg("death save failed!");
			event_signal(EVENT_MESSAGE_FLUSH);
		}
	} else {
		/* Save the game */
		save_game();

		if (Term->mapped_flag) {
			struct keypress ch;

			prt("Press Return (or Escape).", 0, 40);
			ch = inkey();
			if (ch.code != ESCAPE)
				predict_score();
		}
	}

	/* Wipe the monster list */
	wipe_mon_list(cave, player);

	/* Hack -- Decrease "icky" depth */
	screen_save_depth--;

	/* Tell the UI we're done with the game state */
	event_signal(EVENT_LEAVE_GAME);

	/* Allow suspending now */
	signals_handle_tstp();
}
