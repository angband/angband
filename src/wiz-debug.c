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
#include "cmds.h"
#include "game-input.h"
#include "player-timed.h"
#include "player-util.h"
#include "ui-help.h"
#include "ui-output.h"
#include "ui-spoil.h"
#include "ui-wizard.h"
#include "wizard.h"


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
			break;

		/* Hack -- Generate Spoilers */
		case '"':
			do_cmd_spoilers();
			break;

		/* Hack -- Help */
		case '?':
			do_cmd_wiz_help();
			break;

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
			wiz_create_item(false);
			break;

		/* Create an artifact */
		case 'C':
			wiz_create_item(true);
			break;

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
			cmdq_push(CMD_WIZ_PERFORM_EFFECT);
			break;

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
			wiz_proj_demo();
			break;

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
