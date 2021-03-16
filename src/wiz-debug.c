/**
 * \file wiz-debug.c
 * \brief Debug mode commands
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "cave.h"
#include "cmds.h"
#include "effects.h"
#include "game-input.h"
#include "grafmode.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "ui-event.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-menu.h"
#include "ui-prefs.h"
#include "wizard.h"


static void proj_display(struct menu *m, int type, bool cursor,
		int row, int col, int wid)
{
	size_t i;

	byte attr = curs_attrs[CURS_KNOWN][(int)cursor];
	const char *proj_name = proj_idx_to_name(type);

	if (type % 2)
		c_prt(attr, ".........................", row, col);
	c_put_str(attr, proj_name, row, col);

	col += 25;

	if (tile_height == 1) {
		for (i = 0; i < BOLT_MAX; i++) {
			if (use_graphics == GRAPHICS_NONE) {
				/* ASCII is simple */
				wchar_t chars[] = L"*|/-\\";

				col += big_pad(col, row, projections[type].color, chars[i]);
			} else {
				col += big_pad(col, row, proj_to_attr[type][i],
							   proj_to_char[type][i]);
			}
		}
	} else {
		prt("Change tile_height to 1 to see graphics.", row, col);
	}
}

static const menu_iter proj_iter = {
	NULL, /* get_tag */
	NULL, /* validity */
	proj_display,
	NULL, /* action */
	NULL /* resize */
};

static void wiz_proj_demo(void)
{
	struct menu *m = menu_new(MN_SKIN_SCROLL, &proj_iter);
	region loc = { 0, 0, 0, 0 };

	menu_setpriv(m, PROJ_MAX, NULL);

	m->title = "PROJ_ types display";
	menu_layout(m, &loc);

	screen_save();
	clear_from(0);
	menu_select(m, 0, false);
	screen_load();
	mem_free(m);
}








/**
 * This is a nice utility function; it determines if a (NULL-terminated)
 * string consists of only digits (starting with a non-zero digit).
 */
static s16b get_idx_from_name(char *s)
{
	char *endptr = NULL;
	long l = strtol(s, &endptr, 10);
	return *endptr == '\0' ? (s16b)l : 0;
}


/** Object creation code **/
static bool choose_artifact = false;

static const region wiz_create_item_area = { 0, 0, 0, 0 };

/**
 * Build an "artifact name" and transfer it into a buffer.
 */
static void get_art_name(char *buf, int max, int a_idx)
{
	struct object *obj, *known_obj;
	struct object_kind *kind;
	struct artifact *art = &a_info[a_idx];

	/* Get object */
	obj = object_new();

	/* Acquire the "kind" index */
	kind = lookup_kind(art->tval, art->sval);

	/* Oops */
	if (!kind)
		return;

	/* Create the base object */
	object_prep(obj, kind, 0, RANDOMISE);

	/* Mark it as an artifact */
	obj->artifact = art;

	/* Make it known to us */
	known_obj = object_new();
	obj->known = known_obj;
	object_copy(known_obj, obj);
	known_obj->notice |= OBJ_NOTICE_IMAGINED;

	/* Create the artifact description */
	object_desc(buf, max, obj, ODESC_SINGULAR | ODESC_SPOIL);

	object_delete(&known_obj);
	obj->known = NULL;
	object_delete(&obj);
}

#define WIZ_CREATE_ALL_MENU_ITEM -9999

/**
 * Artifact or object kind selection
 */
static void wiz_create_item_subdisplay(struct menu *m, int oid, bool cursor,
		int row, int col, int width)
{
	int *choices = menu_priv(m);
	int selected = choices[oid];
	char buf[70];

	if (selected == WIZ_CREATE_ALL_MENU_ITEM) {
		/* Super big hack: the special flag should be the last menu item, with
		 * the selected tval stored in the next element. */
		int current_tval = choices[oid + 1];
		char name[70];

		object_base_name(name, sizeof(name), current_tval, true);
		if (choose_artifact)
			strnfmt(buf, sizeof(buf), "All artifact %s", name);
		else
			strnfmt(buf, sizeof(buf), "All %s", name);
	}
	else {
		if (choose_artifact)
			get_art_name(buf, sizeof(buf), selected);
		else
			object_kind_name(buf, sizeof(buf), &k_info[selected], true);
	}

	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

static bool wiz_create_item_subaction(struct menu *m, const ui_event *e, int oid)
{
	int *choices = menu_priv(m);
	int selected = choices[oid];

	if (e->type != EVT_SELECT)
		return true;

	if (selected == WIZ_CREATE_ALL_MENU_ITEM && !choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_ALL_OBJ_FROM_TVAL);
		/* Same hack as in wiz_create_item_subdisplay() to get tval. */
		cmd_set_arg_number(cmdq_peek(), "tval", choices[oid + 1]);
		cmd_set_arg_choice(cmdq_peek(), "choice", 0);
	} else if (selected == WIZ_CREATE_ALL_MENU_ITEM && choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_ALL_ARTIFACT_FROM_TVAL);
		cmd_set_arg_number(cmdq_peek(), "tval", choices[oid + 1]);
	} else if (selected != WIZ_CREATE_ALL_MENU_ITEM && !choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_OBJ);
		cmd_set_arg_number(cmdq_peek(), "index", choices[oid]);
	} else if (selected != WIZ_CREATE_ALL_MENU_ITEM && choose_artifact) {
		cmdq_push(CMD_WIZ_CREATE_ARTIFACT);
		cmd_set_arg_number(cmdq_peek(), "index", choices[oid]);
	}

	return false;
}

static menu_iter wiz_create_item_submenu =
{
	NULL,
	NULL,
	wiz_create_item_subdisplay,
	wiz_create_item_subaction,
	NULL
};

/**
 * Object base kind selection
 */

static void wiz_create_item_display(struct menu *m, int oid, bool cursor,
		int row, int col, int width)
{
	char buf[80];

	if (oid == WIZ_CREATE_ALL_MENU_ITEM) {
		if (choose_artifact)
			my_strcpy(buf, "All artifacts", sizeof(buf));
		else
			my_strcpy(buf, "All objects", sizeof(buf));
	}
	else {
		object_base_name(buf, sizeof(buf), oid, true);
	}

	c_prt(curs_attrs[CURS_KNOWN][0 != cursor], buf, row, col);
}

static bool wiz_create_item_action(struct menu *m, const ui_event *e, int oid)
{
	ui_event ret;
	struct menu *menu;

	char buf[80];
	char title[80];

	int choice[70];
	int num;

	int i;

	if (e->type != EVT_SELECT)
		return true;

	if (oid == WIZ_CREATE_ALL_MENU_ITEM) {
		cmdq_push((choose_artifact) ? CMD_WIZ_CREATE_ALL_ARTIFACT :
			CMD_WIZ_CREATE_ALL_OBJ);
		return false;
	}

	/* Artifacts */
	if (choose_artifact) {
		/* ...We have to search the whole artifact list. */
		for (num = 0, i = 1; (num < 60) && (i < z_info->a_max); i++) {
			struct artifact *art = &a_info[i];

			if (art->tval != oid) continue;

			choice[num++] = i;
		}
	} else {
		/* Regular objects */
		for (num = 0, i = 1; (num < 60) && (i < z_info->k_max); i++) {
			struct object_kind *kind = &k_info[i];

			if (kind->tval != oid || kf_has(kind->kind_flags, KF_INSTA_ART))
				continue;

			choice[num++] = i;
		}
	}

	/* Add a flag for an "All <tval>" item to create all svals of that tval. The
	 * tval is stored (in a super hacky way) beyond the end of the valid menu
	 * items. The menu won't render it, but we can still get to it without 
	 * doing a bunch of work. */
	choice[num++] = WIZ_CREATE_ALL_MENU_ITEM;
	choice[num] = oid;

	screen_save();
	clear_from(0);

	menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_submenu);
	menu->selections = all_letters;

	object_base_name(buf, sizeof(buf), oid, true);
	if (choose_artifact)
		strnfmt(title, sizeof(title), "Which artifact %s? ", buf);
	else
		strnfmt(title, sizeof(title), "What kind of %s?", buf);
	menu->title = title;

	menu_setpriv(menu, num, choice);
	menu_layout(menu, &wiz_create_item_area);
	ret = menu_select(menu, 0, false);

	screen_load();
	mem_free(menu);

	return (ret.type == EVT_ESCAPE);
}

static const menu_iter wiz_create_item_menu =
{
	NULL,
	NULL,
	wiz_create_item_display,
	wiz_create_item_action,
	NULL
};


/**
 * Choose and create an instance of an artifact or object kind
 */
static void wiz_create_item(bool art)
{
	int tvals[TV_MAX];
	int i, n;

	struct menu *menu = menu_new(MN_SKIN_COLUMNS, &wiz_create_item_menu);

	choose_artifact = art;

	menu->selections = all_letters;
	menu->title = art ? "What kind of artifact?" : "What kind of object?";

	/* Make a list of all tvals for the filter */
	for (i = 0, n = 0; i < TV_MAX; i++) {
		/* Only real object bases */
		if (!kb_info[i].name) continue;

		/* For artifact creation, only include tvals which have an artifact */
		if (art) {
			int j;
			for (j = 1; j < z_info->a_max; j++) {
				struct artifact *art_local = &a_info[j];
				if (art_local->tval == i) break;
			}
			if (j == z_info->a_max) continue;
		}

		tvals[n++] = i;
	}

	tvals[n++] = WIZ_CREATE_ALL_MENU_ITEM;

	screen_save();
	clear_from(0);

	menu_setpriv(menu, TV_MAX, kb_info);
	menu_set_filter(menu, tvals, n);
	menu_layout(menu, &wiz_create_item_area);
	menu_select(menu, 0, false);

	screen_load();
	mem_free(menu);
	
	/* Redraw map */
	player->upkeep->redraw |= (PR_MAP | PR_ITEMLIST);
	handle_stuff(player);

}


/**
 * What happens when you cheat death.  Tsk, tsk.
 */
void wiz_cheat_death(void)
{
	/* Mark social class, reset age, if needed */
	player->age = 1;
	player->noscore |= NOSCORE_WIZARD;

	player->is_dead = false;

	/* Restore hit & spell points */
	player->chp = player->mhp;
	player->chp_frac = 0;
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Healing */
	(void)player_clear_timed(player, TMD_BLIND, true);
	(void)player_clear_timed(player, TMD_CONFUSED, true);
	(void)player_clear_timed(player, TMD_POISONED, true);
	(void)player_clear_timed(player, TMD_AFRAID, true);
	(void)player_clear_timed(player, TMD_PARALYZED, true);
	(void)player_clear_timed(player, TMD_IMAGE, true);
	(void)player_clear_timed(player, TMD_STUN, true);
	(void)player_clear_timed(player, TMD_CUT, true);

	/* Prevent starvation */
	player_set_timed(player, TMD_FOOD, PY_FOOD_MAX - 1, false);

	/* Cancel recall */
	if (player->word_recall)
	{
		/* Message */
		msg("A tension leaves the air around you...");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Hack -- Prevent recall */
		player->word_recall = 0;
	}

	/* Cancel deep descent */
	if (player->deep_descent)
	{
		/* Message */
		msg("The air around you stops swirling...");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Hack -- Prevent recall */
		player->deep_descent = 0;
	}

	/* Note cause of death XXX XXX XXX */
	my_strcpy(player->died_from, "Cheating death", sizeof(player->died_from));

	/* Back to the town */
	dungeon_change_level(player, 0);
}


/**
 * Display the debug commands help file.
 */
static void do_cmd_wiz_help(void) 
{
	char buf[80];
	strnfmt(buf, sizeof(buf), "debug.txt");
	screen_save();
	show_file(buf, NULL, 0, 0);
	screen_load();
}

/**
 * Prompt for an effect and perform it.
 */
void do_cmd_wiz_effect(void)
{
	char name[80] = "";
	char dice[80] = "0";
	int index = -1;
	int p1 = 0, p2 = 0, p3 = 0;
	int y = 0, x = 0;
	bool ident = false;

	/* Avoid the prompt getting in the way */
	screen_save();

	/* Get the name */
	if (get_string("Do which effect: ", name, sizeof(name))) {
		/* See if an effect index was entered */
		index = get_idx_from_name(name);

		/* If not, find the effect with that name */
		if (index <= EF_NONE || index >= EF_MAX)
			index = effect_lookup(name);

		/* Failed */
		if (index <= EF_NONE || index >= EF_MAX) {
			msg("No effect found.");
			return;
		}
	}

	/* Get the dice */
	if (! get_string("Enter damage dice (eg 1+2d6M2): ", dice,
			sizeof(dice))) {
		my_strcpy(dice, "0", sizeof(dice));
	}

	/* Get the effect subtype */
	if (get_string("Enter name or number for effect subtype: ", name,
			sizeof(name))) {
		/* See if an effect parameter was entered */
		p1 = effect_subtype(index, name);
		if (p1 == -1) p1 = 0;
	}

	/* Get the parameters */
	p2 = get_quantity("Enter second parameter (radius): ", 100);
	p3 = get_quantity("Enter third parameter (other): ", 100);
	y = get_quantity("Enter y parameter: ", 100);
	x = get_quantity("Enter x parameter: ", 100);

	/* Reload the screen */
	screen_load();

	effect_simple(index, source_player(), dice, p1, p2, p3, y, x, &ident);

	if (ident)
		msg("Identified!");
}

/**
 * Main switch for processing debug commands.  This is a step back in time to
 * how all commands used to be processed
 */
void get_debug_command(void)
{
	char cmd;

	/* Get a "debug command" */
	if (!get_com("Debug Command: ", &cmd)) return;

	/* Analyze the command */
	switch (cmd)
	{
		/* Ignore */
		case ' ':
		{
			break;
		}

		/* Hack -- Generate Spoilers */
		case '"':
		{
			do_cmd_spoilers();
			break;
		}

		/* Hack -- Help */
		case '?':
		{
			do_cmd_wiz_help();
			break;
		}

		/* Cure all maladies */
		case 'a':
			cmdq_push(CMD_WIZ_CURE_ALL);
			break;

		/* Make the player powerful */
		case 'A':
			cmdq_push(CMD_WIZ_ADVANCE);
			break;

		/* Teleport to target */
		case 'b':
			cmdq_push(CMD_WIZ_TELEPORT_TO);
			break;

		/* Create any object */
		case 'c':
		{
			wiz_create_item(false);
			break;
		}

		/* Create an artifact */
		case 'C':
		{
			wiz_create_item(true);
			break;
		}

		/* Detect everything */
		case 'd':
			cmdq_push(CMD_WIZ_DETECT_ALL_LOCAL);
			break;

		/* Test for disconnected dungeon */
		case 'D':
			cmdq_push(CMD_WIZ_COLLECT_DISCONNECT_STATS);
			break;

		/* Edit character */
		case 'e':
			cmdq_push(CMD_WIZ_EDIT_PLAYER_START);
			break;

		/* Perform an effect. */
		case 'E':
		{
			do_cmd_wiz_effect();
			break;
		}

		case 'f':
			cmdq_push(CMD_WIZ_COLLECT_OBJ_MON_STATS);
			break;

		case 'F':
			cmdq_push(CMD_WIZ_QUERY_FEATURE);
			break;

		/* Good Objects */
		case 'g':
			cmdq_push(CMD_WIZ_ACQUIRE);
			cmd_set_arg_choice(cmdq_peek(), "choice", 0);
			break;

		/* GF demo */
		case 'G':
		{
			wiz_proj_demo();
			break;
		}

		/* Hitpoint rerating */
		case 'h':
			cmdq_push(CMD_WIZ_RERATE);
			break;

		/* Hit all monsters in LOS */
		case 'H':
			cmdq_push(CMD_WIZ_HIT_ALL_LOS);
			break;

		/* Go up or down in the dungeon */
		case 'j':
			cmdq_push(CMD_WIZ_JUMP_LEVEL);
			break;

		/* Learn about objects */
		case 'l':
			cmdq_push(CMD_WIZ_LEARN_OBJECT_KINDS);
			cmd_set_arg_number(cmdq_peek(), "level", 100);
			break;

		/* Work out what the player is typing */
		case 'L':
			cmdq_push(CMD_WIZ_DISPLAY_KEYLOG);
			break;

		/* Magic Mapping */
		case 'm':
			cmdq_push(CMD_WIZ_MAGIC_MAP);
			break;

		/* Dump a map of the current level as HTML. */
		case 'M':
			cmdq_push(CMD_WIZ_DUMP_LEVEL_MAP);
			break;

		/* Summon Named Monster */
		case 'n':
			cmdq_push(CMD_WIZ_SUMMON_NAMED);
			break;

		/* Object playing routines */
		case 'o':
			cmdq_push(CMD_WIZ_PLAY_ITEM);
			break;

		/* Phase Door */
		case 'p':
			cmdq_push(CMD_WIZ_TELEPORT_RANDOM);
			cmd_set_arg_number(cmdq_peek(), "range", 10);
			break;

		/* Monster pit stats */
		case 'P':
			cmdq_push(CMD_WIZ_COLLECT_PIT_STATS);
			break;

		/* Query the dungeon */
		case 'q':
			cmdq_push(CMD_WIZ_QUERY_SQUARE_FLAG);
			break;

		/* Get full recall for a monster */
		case 'r':
			cmdq_push(CMD_WIZ_RECALL_MONSTER);
			break;

		/* Summon Random Monster(s) */
		case 's':
			cmdq_push(CMD_WIZ_SUMMON_RANDOM);
			break;

		/* Collect stats (S) */
		case 'S':
			cmdq_push(CMD_WIZ_COLLECT_OBJ_MON_STATS);
			break;

		/* Teleport */
		case 't':
			cmdq_push(CMD_WIZ_TELEPORT_RANDOM);
			cmd_set_arg_number(cmdq_peek(), "range", 100);
			break;

		/* Create a trap */
		case 'T':
			cmdq_push(CMD_WIZ_CREATE_TRAP);
			break;

		/* Un-hide all monsters */
		case 'u':
			cmdq_push(CMD_WIZ_DETECT_ALL_MONSTERS);
			break;

		/* Very Good Objects */
		case 'v':
			cmdq_push(CMD_WIZ_ACQUIRE);
			cmd_set_arg_choice(cmdq_peek(), "choice", 1);
			break;

		case 'V':
			cmdq_push(CMD_WIZ_CREATE_ALL_OBJ_FROM_TVAL);
			cmd_set_arg_choice(cmdq_peek(), "choice", 1);
			break;

		/* Wizard Light the Level */
		case 'w':
			cmdq_push(CMD_WIZ_WIZARD_LIGHT);
			break;

		/* Wipe recall for a monster */
		case 'W':
			cmdq_push(CMD_WIZ_WIPE_RECALL);
			break;

		/* Increase Experience */
		case 'x':
			cmdq_push(CMD_WIZ_INCREASE_EXP);
			break;

		/* Quit the game, don't save */
		case 'X':
			if (get_check("Really quit without saving? ")) {
				cmdq_push(CMD_WIZ_QUIT_NO_SAVE);
			}
			break;

		/* Zap Monsters (Banishment) */
		case 'z':
			cmdq_push(CMD_WIZ_BANISH);
			break;

		/* Hack */
		case '_':
			cmdq_push(CMD_WIZ_PEEK_NOISE_SCENT);
			break;

		/* Use push_object() on a selected grid. */
		case '>':
			cmdq_push(CMD_WIZ_PUSH_OBJECT);
			break;

		/* Oops */
		default:
			msg("That is not a valid debug command.");
			break;
	}
}
