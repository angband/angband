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
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "grafmode.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "obj-knowledge.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-path.h"
#include "player-properties.h"
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
#include "ui-menu.h"
#include "ui-object.h"
#include "ui-output.h"
#include "ui-player.h"
#include "ui-prefs.h"
#include "ui-spell.h"
#include "ui-score.h"
#include "ui-signals.h"
#include "ui-spoil.h"
#include "ui-store.h"
#include "ui-target.h"
#include "ui-wizard.h"
#include "z-file.h"
#include "z-util.h"
#include "z-virt.h"


struct savefile_getter_impl {
	ang_dir *d;
	struct savefile_details details;
#ifdef SETGID
	char uid_c[10];
#endif
	bool have_details;
	bool have_savedir;
};


bool arg_wizard;			/* Command arg -- Request wizard mode */

/**
 * Buffer to hold the current savefile name
 */
char savefile[1024];

/**
 * Buffer to hold the name of the panic save corresponding to savefile.  Only
 * set and used as necessary (in start_game() and handle_signal_abort()).  Use
 * static storage to avoid complications in the signal handler (i.e. limited
 * stack space or the possibility of a full or corrupted heap).
 */
char panicfile[1024];

/**
 * Set by the front end to perform necessary actions when restarting after death
 * without exiting.  May be NULL.
 */
void (*reinit_hook)(void) = NULL;


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
	{ "Inscribe an object", { '{' }, CMD_INSCRIBE, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Uninscribe an object", { '}' }, CMD_UNINSCRIBE, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Wear/wield an item", { 'w' }, CMD_WIELD, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Take off/unwield an item", { 't', 'T'}, CMD_TAKEOFF, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Examine an item", { 'I' }, CMD_NULL, textui_obj_examine, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Drop an item", { 'd' }, CMD_DROP, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Fire your missile weapon", { 'f', 't' }, CMD_FIRE, NULL, player_can_fire_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Use a staff", { 'u', 'Z' }, CMD_USE_STAFF, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Aim a wand", {'a', 'z'}, CMD_USE_WAND, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Zap a rod", {'z', 'a'}, CMD_USE_ROD, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Activate an object", {'A' }, CMD_ACTIVATE, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Eat some food", { 'E' }, CMD_EAT, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Quaff a potion", { 'q' }, CMD_QUAFF, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Read a scroll", { 'r' }, CMD_READ_SCROLL, NULL, player_can_read_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Fuel your light source", { 'F' }, CMD_REFILL, NULL, player_can_refuel_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Use an item", { 'U', 'X' }, CMD_USE, NULL, NULL, 0, NULL, NULL, NULL, 0 }
};

/**
 * General actions
 */
struct cmd_info cmd_action[] =
{
	{ "Disarm a trap or chest", { 'D' }, CMD_DISARM, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Rest for a while", { 'R' }, CMD_NULL, textui_cmd_rest, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Look around", { 'l', 'x' }, CMD_NULL, do_cmd_look, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Target monster or location", { '*' }, CMD_NULL, textui_target, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Target closest monster", { '\'' }, CMD_NULL, textui_target_closest, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Dig a tunnel", { 'T', KTRL('T') }, CMD_TUNNEL, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Go up staircase", {'<' }, CMD_GO_UP, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Go down staircase", { '>' }, CMD_GO_DOWN, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Open a door or a chest", { 'o' }, CMD_OPEN, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Close a door", { 'c' }, CMD_CLOSE, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Fire at nearest target", { 'h', KC_TAB }, CMD_NULL, do_cmd_fire_at_nearest, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Throw an item", { 'v' }, CMD_THROW, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Walk into a trap", { 'W', '-' }, CMD_JUMP, NULL, NULL, 0, NULL, NULL, NULL, 0 },
};

/**
 * Item management commands
 */
struct cmd_info cmd_item_manage[] =
{
	{ "Display equipment listing", { 'e' }, CMD_NULL, do_cmd_equip, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Display inventory listing", { 'i' }, CMD_NULL, do_cmd_inven, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Display quiver listing", { '|' }, CMD_NULL, do_cmd_quiver, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Pick up objects", { 'g' }, CMD_PICKUP, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Ignore an item", { 'k', KTRL('D') }, CMD_IGNORE, textui_cmd_ignore, NULL, 0, NULL, NULL, NULL, 0 },
};

/**
 * Information access commands
 */
struct cmd_info cmd_info[] =
{
	{ "Browse a book", { 'b', 'P' }, CMD_BROWSE_SPELL, textui_spell_browse, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Gain new spells", { 'G' }, CMD_STUDY, NULL, player_can_study_prereq, 0, NULL, NULL, NULL, 0 },
	{ "View abilities", { 'S' }, CMD_NULL, do_cmd_abilities, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Cast a spell", { 'm' }, CMD_CAST, NULL, player_can_cast_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Full dungeon map", { 'M' }, CMD_NULL, do_cmd_view_map, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Toggle ignoring of items", { 'K', 'O' }, CMD_NULL, textui_cmd_toggle_ignore, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Display visible item list", { ']' }, CMD_NULL, do_cmd_itemlist, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Display visible monster list", { '[' }, CMD_NULL, do_cmd_monlist, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Locate player on map", { 'L', 'W' }, CMD_NULL, do_cmd_locate, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Help", { '?' }, CMD_NULL, do_cmd_help, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Identify symbol", { '/' }, CMD_NULL, do_cmd_query_symbol, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Character description", { 'C' }, CMD_NULL, do_cmd_change_name, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Check knowledge", { '~' }, CMD_NULL, textui_browse_knowledge, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Repeat level feeling", { KTRL('F') }, CMD_NULL, do_cmd_feeling, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Show previous message", { KTRL('O') }, CMD_NULL, do_cmd_message_one, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Show previous messages", { KTRL('P') }, CMD_NULL, do_cmd_messages, NULL, 0, NULL, NULL, NULL, 0 }
};

/**
 * Utility/assorted commands
 */
struct cmd_info cmd_util[] =
{
	{ "Interact with options", { '=' }, CMD_NULL, do_cmd_xxx_options, NULL, 0, NULL, NULL, NULL, 0 },

	{ "Save and don't quit", { KTRL('S') }, CMD_NULL, save_game, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Save and quit", { KTRL('X') }, CMD_NULL, textui_quit, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Kill character and quit", { 'Q' }, CMD_NULL, textui_cmd_suicide, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Redraw the screen", { KTRL('R') }, CMD_NULL, do_cmd_redraw, NULL, 0, NULL, NULL, NULL, 0 },

	{ "Save \"screen dump\"", { ')' }, CMD_NULL, do_cmd_save_screen, NULL, 0, NULL, NULL, NULL, 0 }
};

/**
 * Commands that shouldn't be shown to the user
 */
struct cmd_info cmd_hidden[] =
{
	{ "Take notes", { ':' }, CMD_NULL, do_cmd_note, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Version info", { 'V' }, CMD_NULL, do_cmd_version, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Load a single pref line", { '"' }, CMD_NULL, do_cmd_pref, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Toggle windows", { KTRL('E') }, CMD_NULL, toggle_inven_equip, NULL, 0, NULL, NULL, NULL, 0 }, /* XXX */
	{ "Alter a grid", { '+' }, CMD_ALTER, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Steal from a monster", { 's' }, CMD_STEAL, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Walk", { ';' }, CMD_WALK, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Start running", { '.', ',' }, CMD_RUN, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Stand still", { ',', '.' }, CMD_HOLD, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Center map", { KTRL('L'), '@' }, CMD_NULL, do_cmd_center_map, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Toggle wizard mode", { KTRL('W') }, CMD_NULL, do_cmd_wizard, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Repeat previous command", { 'n', KTRL('V') }, CMD_REPEAT, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Do autopickup", { KTRL('G') }, CMD_AUTOPICKUP, NULL, NULL, 0, NULL, NULL, NULL, 0 },
	{ "Debug mode commands", { KTRL('A') }, CMD_NULL, NULL, NULL, 1, "Debug Command: ", "That is not a valid debug command.", "Debug", -1 },
};

/**
 * Debug mode command categories; placeholders for the Enter menu system
 */
struct cmd_info cmd_debug[] =
{
	{ "Items", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgObj", -1 },
	{ "Player", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgPlayer", -1 },
	{ "Teleport", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgTele", -1 },
	{ "Effects", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgEffects", -1 },
	{ "Summon", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgSummon", -1 },
	{ "Files", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgFiles", -1 },
	{ "Statistics", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgStat", -1 },
	{ "Query", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgQuery", -1 },
	{ "Miscellaneous", { '\0' }, CMD_NULL, NULL, NULL, 0, NULL, NULL, "DbgMisc", -1 },
};

struct cmd_info cmd_debug_obj[] =
{
	{ "Create an object", { 'c' }, CMD_NULL, wiz_create_nonartifact, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Create an artifact", { 'C' }, CMD_NULL, wiz_create_artifact, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Create all from tval", { 'V' }, CMD_NULL, wiz_create_all_for_tval, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Acquire good", { 'g' }, CMD_NULL, wiz_acquire_good, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Acquire great", { 'v' }, CMD_NULL, wiz_acquire_great, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Play with item", { 'o' }, CMD_WIZ_PLAY_ITEM, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_player[] =
{
	{ "Cure everything", { 'a' }, CMD_WIZ_CURE_ALL, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Make powerful", { 'A' }, CMD_WIZ_ADVANCE, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Increase experience", { 'x' }, CMD_WIZ_INCREASE_EXP, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Rerate hitpoints", { 'h' }, CMD_WIZ_RERATE, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Edit player", { 'e' }, CMD_WIZ_EDIT_PLAYER_START, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Learn object kinds", { 'l' }, CMD_NULL, wiz_learn_all_object_kinds, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Recall monster", { 'r' }, CMD_WIZ_RECALL_MONSTER, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Erase monster recall", { 'W' }, CMD_WIZ_WIPE_RECALL, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_tele[] =
{
	{ "To location", { 'b' }, CMD_WIZ_TELEPORT_TO, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Random near", { 'p' }, CMD_NULL, wiz_phase_door, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Random far", { 't' }, CMD_NULL, wiz_teleport, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Jump to a level", { 'j' }, CMD_WIZ_JUMP_LEVEL, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_effects[] =
{
	{ "Detect all nearby", { 'd' }, CMD_WIZ_DETECT_ALL_LOCAL, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Detect all monsters", { 'u' }, CMD_WIZ_DETECT_ALL_MONSTERS, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Map local area", { 'm' }, CMD_WIZ_MAGIC_MAP, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Hit all in LOS", { 'H' }, CMD_WIZ_HIT_ALL_LOS, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Perform an effect", { 'E' }, CMD_WIZ_PERFORM_EFFECT, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Graphics demo", { 'G' }, CMD_NULL, wiz_proj_demo, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_summon[] =
{
	{ "Summon specific", { 'n' }, CMD_WIZ_SUMMON_NAMED, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Summon random", { 's' }, CMD_WIZ_SUMMON_RANDOM, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_files[] =
{
	{ "Create spoilers", { '"' }, CMD_NULL, do_cmd_spoilers, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Write map", { 'M' }, CMD_WIZ_DUMP_LEVEL_MAP, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_stats[] =
{
	{ "Objects and monsters", { 'S' }, CMD_WIZ_COLLECT_OBJ_MON_STATS, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Pits", { 'P' }, CMD_WIZ_COLLECT_PIT_STATS, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Disconnected levels", { 'D' }, CMD_WIZ_COLLECT_DISCONNECT_STATS, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Obj/mon alternate key", { 'f' }, CMD_WIZ_COLLECT_OBJ_MON_STATS, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_query[] =
{
	{ "Feature", { 'F' }, CMD_WIZ_QUERY_FEATURE, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Square flag", { 'q' }, CMD_WIZ_QUERY_SQUARE_FLAG, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Noise and scent", { '_' }, CMD_WIZ_PEEK_NOISE_SCENT, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Keystroke log", { 'L' }, CMD_WIZ_DISPLAY_KEYLOG, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

struct cmd_info cmd_debug_misc[] =
{
	{ "Wizard light level", { 'w' }, CMD_WIZ_WIZARD_LIGHT, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Create a trap", { 'T' }, CMD_WIZ_CREATE_TRAP, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Banish nearby monsters", { 'z' }, CMD_WIZ_BANISH, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Push objects from square", { '>' }, CMD_WIZ_PUSH_OBJECT, NULL, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
	{ "Quit without saving", { 'X' }, CMD_NULL, wiz_confirm_quit_no_save, player_can_debug_prereq, 0, NULL, NULL, NULL, 0 },
};

/**
 * List of command lists; because of the implementation in ui-context.c all
 * entries with menu_level == 0 should appear first; hardwired geometry in
 * ui-context.c limits the maximum nesting level to 2
 */
struct command_list cmds_all[] =
{
	{ "Items",           cmd_item,        N_ELEMENTS(cmd_item), 0, 0 },
	{ "Action commands", cmd_action,      N_ELEMENTS(cmd_action), 0, 0 },
	{ "Manage items",    cmd_item_manage, N_ELEMENTS(cmd_item_manage), 0, false },
	{ "Information",     cmd_info,        N_ELEMENTS(cmd_info), 0, 0 },
	{ "Utility",         cmd_util,        N_ELEMENTS(cmd_util), 0, 0 },
	{ "Hidden",          cmd_hidden,      N_ELEMENTS(cmd_hidden), 0, 0 },
	/*
	 * This is nested below "Hidden"->"Debug mode commands" and only
	 * contains categories.
	 */
	{ "Debug", cmd_debug, N_ELEMENTS(cmd_debug), 1, -1 },
	/* These are nested in "Debug"; names have to match with cmd_debug. */
	{ "DbgObj", cmd_debug_obj, N_ELEMENTS(cmd_debug_obj), 2, 1 },
	{ "DbgPlayer", cmd_debug_player, N_ELEMENTS(cmd_debug_player), 2, 1 },
	{ "DbgTele", cmd_debug_tele, N_ELEMENTS(cmd_debug_tele), 2, 1 },
	{ "DbgEffects", cmd_debug_effects, N_ELEMENTS(cmd_debug_effects), 2, 1 },
	{ "DbgSummon", cmd_debug_summon, N_ELEMENTS(cmd_debug_summon), 2, 1 },
	{ "DbgFiles", cmd_debug_files, N_ELEMENTS(cmd_debug_files), 2, 1 },
	{ "DbgStat", cmd_debug_stats, N_ELEMENTS(cmd_debug_stats), 2, 1 },
	{ "DbgQuery", cmd_debug_query, N_ELEMENTS(cmd_debug_query), 2, 1 },
	{ "DbgMisc", cmd_debug_misc, N_ELEMENTS(cmd_debug_misc), 2, 1 },
	{ NULL,              NULL,            0, 0, false }
};



/*** Exported functions ***/

#define KEYMAP_MAX 2

/* List of directly accessible commands indexed by char */
static struct cmd_info *converted_list[KEYMAP_MAX][UCHAR_MAX+1];

/*
 * Lists of nested commands; each list is also indexed by char but there's no
 * distinction between original/roguelike keys
 */
static int n_nested = 0;
static struct cmd_info ***nested_lists = NULL;

/**
 * Initialise the command list.
 */
void cmd_init(void)
{
	size_t i, j;

	memset(converted_list, 0, sizeof(converted_list));

	/* Set up storage for the nested command lists */
	if (nested_lists != NULL) {
		assert(n_nested >= 0);
		for (j = 0; j < (size_t)n_nested; j++) {
			mem_free(nested_lists[j]);
		}
		nested_lists = NULL;
	}
	n_nested = 0;
	for (j = 0; j < N_ELEMENTS(cmds_all) - 1; j++) {
		n_nested = MAX(n_nested, cmds_all[j].keymap);
	}
	if (n_nested > 0) {
		nested_lists = mem_zalloc(n_nested * sizeof(*nested_lists));
		for (j = 0; j < (size_t)n_nested; j++) {
			nested_lists[j] = mem_zalloc((UCHAR_MAX + 1) *
				sizeof(*(nested_lists[j])));
		}
	}

	/* Go through all generic commands (-1 for NULL end entry) */
	for (j = 0; j < N_ELEMENTS(cmds_all) - 1; j++)
	{
		struct cmd_info *commands = cmds_all[j].list;

		/* Fill everything in */
		if (cmds_all[j].keymap == 0) {
			for (i = 0; i < cmds_all[j].len; i++) {
				/* If a roguelike key isn't set, use default */
				if (!commands[i].key[1])
					commands[i].key[1] = commands[i].key[0];

				/* Skip entries that don't have a valid key. */
				if (!commands[i].key[0] || !commands[i].key[1])
					continue;

				converted_list[0][commands[i].key[0]] =
					&commands[i];
				converted_list[1][commands[i].key[1]] =
					&commands[i];
			}
		} else if (cmds_all[j].keymap > 0) {
			int kidx = cmds_all[j].keymap - 1;

			assert(kidx < n_nested);
			for (i = 0; i < cmds_all[j].len; i++) {
				/*
				 * Nested commands don't go through a keymap;
				 * use the default for the roguelike key.
				 */
				commands[i].key[1] = commands[i].key[0];

				/*
				 * Check for duplicated keys in the same
				 * command set.
				 */
				assert(!nested_lists[kidx][commands[i].key[0]]);

				nested_lists[kidx][commands[i].key[0]] =
					&commands[i];
			}
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

	/*
	 * Because UN_KTRL('ctrl-d') (i.e. rogue-like ignore command) gives 'd'
	 * which is the drop command in both keysets, use UN_KTRL_CAP().
	 */
	if (c < 0x20)
		c = UN_KTRL_CAP(c);

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
 * Return the index into cmds_all for the given name or -2 if not found.
 */
size_t cmd_list_lookup_by_name(const char *name)
{
	size_t i = 0;

	while (1) {
		if (i >= (int) N_ELEMENTS(cmds_all)) {
			/*
			 * Return a negative value other than -1 to prevent
			 * future lookups for the same name by ui-context.c.
			 * Those lookups are guaranteed to fail since the
			 * names in cmds_all don't change.
			 */
			return -2;
		}
		if (streq(cmds_all[i].name, name)) {
			return i;
		}
		++i;
	}
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
		if (cmd->cmd || cmd->hook) {
			/* Confirm for worn equipment inscriptions. */
			if (!key_confirm_command(key)) cmd = NULL;
		} else {
			/*
			 * It refers to nested commands.  Get the nested
			 * command.  Those aren't subject to keymaps and
			 * inherit the count.
			 */
			while (cmd && !cmd->cmd && !cmd->hook) {
				char nestkey;

				if (cmd->nested_keymap > 0 &&
						cmd->nested_keymap <= n_nested &&
						cmd->nested_prompt) {
					if (get_com(cmd->nested_prompt, &nestkey)) {
						const char* em =
							cmd->nested_error;

						cmd = nested_lists[cmd->nested_keymap - 1][(unsigned char) nestkey];
						if (!cmd) {
							msg("%s", em ? em : "That is not a valid nested command.");
						}
					} else {
						cmd = NULL;
					}
				} else {
					cmd = NULL;
				}
			}
		}

		/* Check prereqs. */
		if (cmd && cmd->prereq && !cmd->prereq()) cmd = NULL;

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
			disturb(player);
			msg("Cancelled.");
		}
	}
}

static void pre_turn_refresh(void)
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
static bool start_game(bool new_game)
{
	const char *loadpath = savefile;
	bool exists;

	/* Player will be resuscitated if living in the savefile */
	player->is_dead = true;

	/* Try loading */
	savefile_get_panic_name(panicfile, sizeof(panicfile), loadpath);
	safe_setuid_grab();
	exists = loadpath[0] && file_exists(panicfile);
	safe_setuid_drop();
	if (exists) {
		bool newer;

		safe_setuid_grab();
		newer = file_newer(panicfile, loadpath);
		safe_setuid_drop();
		if (newer) {
			if (get_check("A panic save exists.  Use it? ")) {
				loadpath = panicfile;
			}
		} else {
			/* Remove the out-of-date panic save. */
			safe_setuid_grab();
			file_delete(panicfile);
			safe_setuid_drop();
		}
	}
	safe_setuid_grab();
	exists = file_exists(loadpath);
	safe_setuid_drop();
	if (exists && !savefile_load(loadpath, arg_wizard)) {
		return false;
	}

	/* No living character loaded */
	if (player->is_dead || new_game) {
		character_generated = false;
		textui_do_birth();
	} else {
		/*
		 * Bring the stock curse objects up-to-date with what the
		 * player knows.
		 */
		update_player_object_knowledge(player);
	}

	/* Tell the UI we've started. */
	event_signal(EVENT_LEAVE_INIT);
	event_signal(EVENT_ENTER_GAME);
	event_signal(EVENT_ENTER_WORLD);

	/* Save not required yet. */
	player->upkeep->autosave = false;

	/* Enter the level, generating a new one if needed */
	if (!character_dungeon) {
		prepare_next_level(player);
	}
	on_new_level();

	return true;
}

/**
 * Help select_savefile():  clean up the array of strings
 */
static void cleanup_savefile_selection_strings(char **entries, int count)
{
	int i;

	for (i = 0; i < count; ++i) {
		string_free(entries[i]);
	}
	mem_free(entries);
}

/**
 * Help play_game():  implement the savefile selection menu.
 *
 * \param retry flags that this is a repeated call because the savefile selected
 * by an earlier one could not be loaded.
 * \param new_game points to a boolean.  The pointed to value at entry is not
 * used.  At exit *new_game will be true if the user selected to start a new
 * game.  Otherwise, it will be false.
 */
static void select_savefile(bool retry, bool *new_game)
{
	/* Build the list of selections. */
	savefile_getter g = NULL;
	/*
	 * Leave the first entry for selecting a new game.  Will fill in the
	 * label later.
	 */
	int count = 1, allocated = 16;
	char **entries = mem_zalloc(allocated * sizeof(*entries));
	char **names = mem_zalloc(allocated * sizeof(*names));
	int default_entry = 0;
	struct region m_region = { 0, 3, 0, 0 };
	bool allow_new_game = true;
	bool failed;
	struct menu *m;
	ui_event selection;

	while (got_savefile(&g)) {
		const struct savefile_details *details =
			get_savefile_details(g);

		assert(details);
		if (count == allocated) {
			allocated *= 2;
			entries = mem_realloc(entries,
				allocated * sizeof(*entries));
			names = mem_realloc(names, allocated * sizeof(*names));
		}
		if (details->desc) {
			entries[count] = string_make(format("Use %s: %s",
				details->fnam + details->foff, details->desc));
		} else {
			entries[count] = string_make(format("Use %s",
				details->fnam + details->foff));
		}
		names[count] = string_make(details->fnam);
		if (suffix(savefile, details->fnam)) {
			/*
			 * Matches what's in savefile; put it second in the
			 * the list and mark it as the default entry.  If
			 * not forcing the name, clear savefile and arg_name
			 * so the new game option won't be set up to overwrite
			 * an existing savefile.
			 */
			if (count != 1) {
				char *hold_entry = entries[count];
				char *hold_name = names[count];
				int i;

				for (i = count; i > 1; --i) {
					entries[i] = entries[i - 1];
					names[i] = names[i - 1];
				}
				entries[1] = hold_entry;
				names[1] = hold_name;
			}
			default_entry = 1;
			if (!arg_force_name) {
				savefile[0] = '\0';
				arg_name[0] = '\0';
			}
		}
		++count;
	}
	if (got_savefile_dir(g)) {
		assert(allocated > 0 && !entries[0]);
		if (default_entry && arg_force_name) {
			/*
			 * Name set by front end is already in use and names
			 * are forced so don't allow the new game option.
			 */
			int i;

			assert(!entries[0] && !names[0]);
			for (i = 0; i < count - 1; ++i) {
				entries[i] = entries[i + 1];
				names[i] = names[i + 1];
			}
			--default_entry;
			--count;
			allow_new_game = false;
		} else {
			entries[0] = string_make("New game");
		}
		failed = false;
	} else {
		failed = true;
	}
	cleanup_savefile_getter(g);
	if (failed) {
		cleanup_savefile_selection_strings(names, count);
		cleanup_savefile_selection_strings(entries, count);
		quit("Cannot open the savefile directory");
	}

	m = menu_new(MN_SKIN_SCROLL, menu_find_iter(MN_ITER_STRINGS));
	menu_setpriv(m, count, entries);
	menu_layout(m, &m_region);
	m->cursor = default_entry;
	m->flags |= MN_DBL_TAP;

	screen_save();
	prt("Select the save to use (movement keys and enter or mouse) or quit",
		0, 0);
	prt("(escape or second mouse button).", 1, 0);
	prt((retry) ? "The previously selected savefile was unusable." : "",
		2, 0);
	selection = menu_select(m, 0, false);
	screen_load();

	if (selection.type == EVT_SELECT) {
		if (m->cursor == 0 && allow_new_game) {
			*new_game = true;
		} else {
			*new_game = false;
			assert(m->cursor > 0 && m->cursor < count);
			path_build(savefile, sizeof(savefile),
				ANGBAND_DIR_SAVE, names[m->cursor]);
		}
	}

	menu_free(m);
	cleanup_savefile_selection_strings(names, count);
	cleanup_savefile_selection_strings(entries, count);

	if (selection.type == EVT_ESCAPE) {
		quit(NULL);
	}
}

/**
 * Play Angband
 */
void play_game(enum game_mode_type mode)
{
	while (1) {
		play_again = false;

		/* Load a savefile or birth a character, or both */
		switch (mode) {
		case GAME_LOAD:
		case GAME_NEW:
			if (!start_game(mode == GAME_NEW)) {
				quit("Broken savefile");
			}
			break;

		case GAME_SELECT:
			{
				bool new_game = false, retry = false;

				while (1) {
					select_savefile(retry, &new_game);
					if (start_game(new_game)) {
						break;
					}
					retry = true;
				}
			}
			break;

		default:
			quit("Invalid game mode in play_game()");
			break;
		}

		/* Get commands from the user, then process the game world
		 * until the command queue is empty and a new player command
		 * is needed */
		while (!player->is_dead && player->upkeep->playing) {
			pre_turn_refresh();
			cmd_get_hook(CTX_GAME);
			run_game_loop();
		}

		/* Close game on death or quitting */
		close_game(true);

		if (!play_again) break;

		cleanup_angband();
		init_display();
		init_angband();
		if (reinit_hook != NULL) {
			(*reinit_hook)();
		}
		textui_init();
		if (mode == GAME_LOAD) {
			mode = GAME_NEW;
		}
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
	set_archive_user_prefix(path);
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
 * Test whether savefile_set_name() generates a name that's already in use.
 */
bool savefile_name_already_used(const char *fname, bool make_safe,
		bool strip_suffix)
{
	char *hold = string_make(savefile);
	bool result;

	savefile_set_name(fname, make_safe, strip_suffix);
	result = file_exists(savefile);
	my_strcpy(savefile, hold, sizeof(savefile));
	string_free(hold);
	return result;
}

/**
 * Save the game.
 */
void save_game(void)
{
	(void) save_game_checked();
}

/**
 * Save the game.
 *
 * \return whether the save was successful.
 */
bool save_game_checked(void)
{
	char path[1024];
	bool result;

	/* Disturb the player */
	disturb(player);

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
	if (savefile_save(savefile)) {
		prt("Saving game... done.", 0, 0);
		result = true;
	} else {
		prt("Saving game... failed!", 0, 0);
		result = false;
	}

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

	return result;
}


/**
 * Close up the current game (player may or may not be dead).
 *
 * \param prompt_failed_save If true, prompt the user to retry if saving fails.
 * Otherwise, no prompt is issued.
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(bool prompt_failed_save)
{
	bool prompting = true;

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
		while (prompting && !savefile_save(savefile)) {
			if (!prompt_failed_save
					|| !get_check("Saving failed.  Try again? ")) {
				prompting = false;
				msg("death save failed!");
				event_signal(EVENT_MESSAGE_FLUSH);
			}
		}
	} else {
		/* Save the game */
		while (prompting && !save_game_checked()) {
			if (!prompt_failed_save
					|| !get_check("Saving failed.  Try again? ")) {
				prompting = false;
			}
		}

		if (Term->mapped_flag) {
			struct keypress ch;

			prt("Press Return (or Escape).", 0, 40);
			ch = inkey();
			if (ch.code != ESCAPE)
				predict_score(false);
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


/**
 * Enumerate savefiles in the savefile directory that are available to the
 * current player.
 *
 * \param pg points to the state for the enumeration.  If *pg is NULL, the
 * enumeration will start from scratch.  After enumerating, *pg should be
 * passed to cleanup_savefile_getter() to release any allocated resources.
 * \return true if another savefile useful for the player was found.  In
 * that case calling get_savefile_details() on *pg will return a non-NULL
 * result.  Otherwise, return false.
 */
bool got_savefile(savefile_getter *pg)
{
	char fname[256];

	if (*pg == NULL) {
		/* Initialize the enumeration. */
		*pg = mem_zalloc(sizeof(**pg));
		/* Need enhanced privileges to read from the save directory. */
		safe_setuid_grab();
		(*pg)->d = my_dopen(ANGBAND_DIR_SAVE);
		safe_setuid_drop();
		if (!(*pg)->d) {
			return false;
		}
		(*pg)->have_savedir = true;
		/*
		 * Set up the user-specific prefix.  Mimics savefile_set_name().
		 */
#ifdef SETGID
		strnfmt((*pg)->uid_c, sizeof((*pg)->uid_c), "%d.", player_uid);
		(*pg)->details.foff = strlen((*pg)->uid_c);
#else
		(*pg)->details.foff = 0;
#endif
	} else {
		if (!(*pg)->d) {
			assert(!(*pg)->have_details);
			return false;
		}
	}

	while (1) {
		char path[1024];
		const char *desc;
		bool no_entry;

		/*
		 * Also need elevated privileges for the file attribute queries
		 * in my_dread().
		 */
		safe_setuid_grab();
		no_entry = !my_dread((*pg)->d, fname, sizeof(fname));
		safe_setuid_drop();
		if (no_entry) {
			break;
		}

#ifdef SETGID
		/* Check that the savefile name begins with the user's ID. */
		if (!prefix(fname, (*pg)->uid_c)) {
			continue;
		}
#endif

		path_build(path, sizeof(path), ANGBAND_DIR_SAVE, fname);
		desc = savefile_get_description(path);
		string_free((*pg)->details.fnam);
		(*pg)->details.fnam = string_make(fname);
		string_free((*pg)->details.desc);
		(*pg)->details.desc = string_make(desc);
		(*pg)->have_details = true;
		return true;
	}

	my_dclose((*pg)->d);
	(*pg)->d = NULL;
	(*pg)->have_details = false;

	return false;
}


/**
 * Return whether the savefile directory was at all readable.
 */
bool got_savefile_dir(const savefile_getter g)
{
	return g && g->have_savedir;
}


/**
 * Return the details for a savefile enumerated by a prior call to
 * got_savefile().
 *
 * \return is NULL if the prior call to get_savefile() failed.  Otherwise,
 * returns a non-null pointer with the details about the enumerated savefile.
 */
const struct savefile_details *get_savefile_details(const savefile_getter g)
{
	return (g && g->have_details) ? &g->details : NULL;
}


/**
 * Cleanup resources allocated by got_savefile().
 */
void cleanup_savefile_getter(savefile_getter g)
{
	if (g) {
		string_free(g->details.desc);
		string_free(g->details.fnam);
		if (g->d) {
			my_dclose(g->d);
		}
		mem_free(g);
	}
}
