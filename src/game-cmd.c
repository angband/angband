/*
 * File: game-cmd.c
 * Purpose: Handles the queueing of game commands.
 *
 * Copyright (c) 2008-9 Antony Sidwell
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
#include "game-cmd.h"
#include "object/object.h"
#include "spells.h"
#include "target.h"

errr (*cmd_get_hook)(cmd_context c, bool wait);

#define CMD_QUEUE_SIZE 20
#define prev_cmd_idx(idx) ((idx + CMD_QUEUE_SIZE - 1) % CMD_QUEUE_SIZE)

static int cmd_head = 0;
static int cmd_tail = 0;
static game_command cmd_queue[CMD_QUEUE_SIZE];

static bool repeat_prev_allowed = FALSE;
static bool repeating = FALSE;

enum cmd_arg_type {
	arg_STRING,
	arg_CHOICE,
	arg_NUMBER,
	arg_ITEM,
	arg_DIRECTION,
	arg_TARGET,
	arg_POINT,
	arg_END
};

/* A simple list of commands and their handling functions. */
static struct
{
	cmd_code cmd;
	enum cmd_arg_type arg_type[3];
	cmd_handler_fn fn;
	bool repeat_allowed;
	int auto_repeat_n;
} game_cmds[] =
{
	{ CMD_LOADFILE, { arg_END }, NULL, FALSE, 0 },
	{ CMD_NEWGAME, { arg_END }, NULL, FALSE, 0 },

	{ CMD_BIRTH_RESET, { arg_END }, NULL, FALSE, 0 },
	{ CMD_CHOOSE_SEX, { arg_CHOICE, arg_END }, NULL, FALSE, 0 },
	{ CMD_CHOOSE_RACE, { arg_CHOICE, arg_END }, NULL, FALSE, 0 },
	{ CMD_CHOOSE_CLASS, { arg_CHOICE, arg_END }, NULL, FALSE, 0 },
	{ CMD_BUY_STAT, { arg_CHOICE, arg_END }, NULL, FALSE, 0 },
	{ CMD_SELL_STAT, { arg_CHOICE, arg_END }, NULL, FALSE, 0 },
	{ CMD_RESET_STATS, { arg_CHOICE, arg_END }, NULL, FALSE, 0 },
	{ CMD_ROLL_STATS, { arg_END }, NULL, FALSE, 0 },
	{ CMD_PREV_STATS, { arg_END }, NULL, FALSE, 0 },
	{ CMD_NAME_CHOICE, { arg_STRING, arg_END }, NULL, FALSE, 0 },
	{ CMD_ACCEPT_CHARACTER, { arg_END }, NULL, FALSE, 0 },

	{ CMD_GO_UP, { arg_END }, do_cmd_go_up, FALSE, 0 },
	{ CMD_GO_DOWN, { arg_END }, do_cmd_go_down, FALSE, 0 },
	{ CMD_SEARCH, { arg_END }, do_cmd_search, TRUE, 10 },
	{ CMD_TOGGLE_SEARCH, { arg_END }, do_cmd_toggle_search, FALSE, 0 },
	{ CMD_WALK, { arg_DIRECTION, arg_END }, do_cmd_walk, TRUE, 0 },
	{ CMD_RUN, { arg_DIRECTION, arg_END }, do_cmd_run, FALSE, 0 },
	{ CMD_JUMP, { arg_DIRECTION, arg_END }, do_cmd_jump, FALSE, 0 },
	{ CMD_OPEN, { arg_DIRECTION, arg_END }, do_cmd_open, TRUE, 99 },
	{ CMD_CLOSE, { arg_DIRECTION, arg_END }, do_cmd_close, TRUE, 99 },
	{ CMD_TUNNEL, { arg_DIRECTION, arg_END }, do_cmd_tunnel, TRUE, 99 },
	{ CMD_HOLD, { arg_END }, do_cmd_hold, TRUE, 0 },
	{ CMD_DISARM, { arg_DIRECTION, arg_END }, do_cmd_disarm, TRUE, 99 },
	{ CMD_BASH, { arg_DIRECTION, arg_END }, do_cmd_bash, TRUE, 99 },
	{ CMD_ALTER, { arg_DIRECTION, arg_END }, do_cmd_alter, TRUE, 99 },
	{ CMD_JAM, { arg_DIRECTION, arg_END }, do_cmd_spike, FALSE, 0 },
	{ CMD_REST, { arg_CHOICE, arg_END }, do_cmd_rest, FALSE, 0 },
	{ CMD_PATHFIND, { arg_POINT, arg_END }, do_cmd_pathfind, FALSE, 0 },
	{ CMD_PICKUP, { arg_ITEM, arg_END }, do_cmd_pickup, FALSE, 0 },
	{ CMD_WIELD, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_wield, FALSE, 0 },
	{ CMD_TAKEOFF, { arg_ITEM, arg_END }, do_cmd_takeoff, FALSE, 0 },
	{ CMD_DROP, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_drop, FALSE, 0 },
	{ CMD_UNINSCRIBE, { arg_ITEM, arg_END }, do_cmd_uninscribe, FALSE, 0 },
	{ CMD_EAT, { arg_ITEM, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_QUAFF, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_USE_ROD, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_USE_STAFF, { arg_ITEM, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_USE_WAND, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_READ_SCROLL, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_ACTIVATE, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_use, FALSE, 0 },
	{ CMD_REFILL, { arg_ITEM, arg_END }, do_cmd_refill, FALSE, 0 },
	{ CMD_FIRE, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_fire, FALSE, 0 },
	{ CMD_THROW, { arg_ITEM, arg_TARGET, arg_END }, do_cmd_throw, FALSE, 0 },
	{ CMD_DESTROY, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_destroy, FALSE, 0 },
	{ CMD_ENTER_STORE, { arg_END }, do_cmd_store, FALSE, 0 },
	{ CMD_INSCRIBE, { arg_ITEM, arg_STRING, arg_END }, do_cmd_inscribe, FALSE, 0 },
	{ CMD_STUDY_SPELL, { arg_CHOICE, arg_END }, do_cmd_study_spell, FALSE, 0 },
	{ CMD_STUDY_BOOK, { arg_ITEM, arg_END }, do_cmd_study_book, FALSE, 0 },
	{ CMD_CAST, { arg_CHOICE, arg_TARGET, arg_END }, do_cmd_cast, FALSE, 0 },
	{ CMD_SELL, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_sell, FALSE, 0 },
	{ CMD_STASH, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_stash, FALSE, 0 },
	{ CMD_BUY, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_buy, FALSE, 0 },
	{ CMD_RETRIEVE, { arg_ITEM, arg_NUMBER, arg_END }, do_cmd_retrieve, FALSE, 0 }
,
	{ CMD_SUICIDE, { arg_END }, do_cmd_suicide, FALSE, 0 },
	{ CMD_SAVE, { arg_END }, do_cmd_save_game, FALSE, 0 },
	{ CMD_QUIT, { arg_END }, do_cmd_quit, FALSE, 0 },
	{ CMD_HELP, { arg_END }, NULL, FALSE, 0 },
	{ CMD_REPEAT, { arg_END }, NULL, FALSE, 0 },
};


/*
 * Insert the given command into the command queue.
 */
errr cmd_insert_s(game_command *cmd)
{
	/* If queue full, return error */
	if (cmd_head + 1 == cmd_tail) return 1;
	if (cmd_head + 1 == CMD_QUEUE_SIZE && cmd_tail == 0) return 1;

	/* Insert command into queue. */
	if (cmd->command != CMD_REPEAT)
	{
		cmd_queue[cmd_head] = *cmd;
	}
	else
	{
		int cmd_prev = cmd_head - 1;

		if (!repeat_prev_allowed) return 1;

		/* If we're repeating a command, we duplicate the previous command 
		   in the next command "slot". */
		if (cmd_prev < 0) cmd_prev = CMD_QUEUE_SIZE - 1;
		
		if (cmd_queue[cmd_prev].command != CMD_NULL)
			cmd_queue[cmd_head] = cmd_queue[cmd_prev];
	}

	/* Advance point in queue, wrapping around at the end */
	cmd_head++;
	if (cmd_head == CMD_QUEUE_SIZE) cmd_head = 0;

	return 0;	
}

/*
 * Get the next game command, with 'wait' indicating whether we
 * are prepared to wait for a command or require a quick return with
 * no command.
 */
errr cmd_get(cmd_context c, game_command *cmd, bool wait)
{
	/* If we're repeating, just pull the last command again. */
	if (repeating)
	{
		*cmd = cmd_queue[prev_cmd_idx(cmd_tail)];
		return 0;
	}

	/* If there are no commands queued, ask the UI for one. */
	if (cmd_head == cmd_tail) 
		cmd_get_hook(c, wait);

	/* If we have a command ready, set it and return success. */
	if (cmd_head != cmd_tail)
	{
		*cmd = cmd_queue[cmd_tail++];
		if (cmd_tail == CMD_QUEUE_SIZE) cmd_tail = 0;

		return 0;
	}

	/* Failure to get a command. */
	return 1;
}

/* Return the index of the given command in the command array. */
static int cmd_idx(cmd_code code)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(game_cmds); i++)
	{
		if (game_cmds[i].cmd == code)
		{
			return i;
		}
	}

	return -1;
}

/*
 * Process the arguments to cmd_insert, etc, into a command struct and
 * attempt to insert it into the queue.
 */
static errr vcmd_insert_repeated(cmd_code c, int nrepeats, va_list vp)
{
	game_command cmd = {CMD_NULL, 0, {{0}} };
	size_t j = 0;
	int idx = cmd_idx(c);

	if (idx == -1) return 1;

	cmd.command = c;

	for (j = 0; (game_cmds[idx].arg_type[j] != arg_END && 
				 j < N_ELEMENTS(game_cmds[idx].arg_type)); j++)
	{
		switch (game_cmds[idx].arg_type[j])
		{
			case arg_CHOICE:
			{
				cmd.args[j].choice = va_arg(vp, int);
				break;
			}

			case arg_STRING:
			{
				cmd.args[j].string = string_make(va_arg(vp, const char *));
				break;
			}
			
			case arg_DIRECTION:
			case arg_TARGET:
			{
				cmd.args[j].direction = va_arg(vp, int);
				break;
			}
			
			case arg_POINT:
			{
				cmd.args[j].point.y = va_arg(vp, int);
				cmd.args[j].point.x = va_arg(vp, int);
				break;
			}
			
			case arg_ITEM:
			{
				cmd.args[j].item = va_arg(vp, int);
				break;
			}
			
			case arg_NUMBER:
			{
				cmd.args[j].number = va_arg(vp, int);
				break;
			}

			case arg_END:
			{
				break;
			}
		}
	}

	cmd.nrepeats = nrepeats;

	return cmd_insert_s(&cmd);
}

/* 
 * Inserts a command in the queue to be carried out. 
 */
errr cmd_insert(cmd_code c, ...)
{
	va_list vp;
	errr retcode = 0;

	va_start(vp, c);
	retcode = vcmd_insert_repeated(c, 0, vp);
	va_end(vp);

	return retcode;
}

/*
 * Inserts a command in the queue to be carried out, with the given
 * number of repeats.
 */
errr cmd_insert_repeated(cmd_code c, int nrepeats, ...)
{
	va_list vp;
	errr retcode = 0;

	va_start(vp, nrepeats);
	retcode = vcmd_insert_repeated(c, nrepeats, vp);
	va_end(vp);

	return retcode;
}


/* 
 * Request a game command from the uI and carry out whatever actions
 * go along with it.
 */
void process_command(cmd_context ctx, bool no_request)
{
	int idx;
	game_command cmd;

	/* If we've got a command to process, do it. */
	if (cmd_get(ctx, &cmd, !no_request) == 0)
	{
		idx = cmd_idx(cmd.command);

		if (idx == -1) return;

		/* Do some sanity checking on those arguments that might have 
		   been declared as "unknown", such as directions and targets. */
		switch (cmd.command)
		{
			case CMD_WALK:
			case CMD_RUN:
			case CMD_JUMP:
			case CMD_OPEN:
			case CMD_CLOSE:
			case CMD_TUNNEL:
			case CMD_DISARM:
			case CMD_BASH:
			case CMD_ALTER:
			case CMD_JAM:
			{
				/* Direction hasn't been specified, so we ask for one. */
				if (cmd.args[0].direction == DIR_UNKNOWN)
				{
					if (!get_rep_dir(&cmd.args[0].direction))
						return;
				}
				
				break;
			}
			
			
			/* 
			 * These take an item number and a  "target" as arguments, 
			 * though a target isn't always actually needed, so we'll 
			 * only prompt for it via callback if the item being used needs it.
			 */
			case CMD_USE_WAND:
			case CMD_USE_ROD:
			case CMD_QUAFF:
			case CMD_ACTIVATE:
			case CMD_READ_SCROLL:
			case CMD_FIRE:
			case CMD_THROW:
			{
				bool get_target = FALSE;

				if (cmd.command == CMD_FIRE ||
					cmd.command == CMD_THROW ||
					obj_needs_aim(object_from_item_idx(cmd.args[0].choice)))
				{
					if (cmd.args[1].direction == DIR_UNKNOWN)
						get_target = TRUE;

					if (cmd.args[1].direction == DIR_TARGET && !target_okay())
						get_target = TRUE;
				}

				if (get_target && !get_aim_dir(&cmd.args[1].direction))
						return;

				break;
			}
			
			/* This takes a choice and a direction. */
			case CMD_CAST:
			{
				bool get_target = FALSE;

				if (spell_needs_aim(cp_ptr->spell_book, cmd.args[0].choice))
				{
					if (cmd.args[1].direction == DIR_UNKNOWN)
						get_target = TRUE;

					if (cmd.args[1].direction == DIR_TARGET && !target_okay())
						get_target = TRUE;
				}

				if (get_target && !get_aim_dir(&cmd.args[1].direction))
						return;
				
				break;
			}

			default: 
			{
                /* I can see the point of the compiler warning, but still... */
				break;
			}
		}

		/* Command repetition */
		if (game_cmds[idx].repeat_allowed)
		{
			/* Auto-repeat only if there isn't already a repeat length. */
			if (game_cmds[idx].auto_repeat_n > 0 && cmd.nrepeats == 0)
			{
				cmd.nrepeats = game_cmds[idx].auto_repeat_n;
				cmd_set_repeat(cmd.nrepeats);
			}
		}
		else
		{
			cmd.nrepeats = 0; repeating = FALSE;
		}

		/* 
		 * The command gets to unset this if it isn't appropriate for
		 * the user to repeat it.
		 */
		repeat_prev_allowed = TRUE;

		if (game_cmds[idx].fn)
			game_cmds[idx].fn(cmd.command, cmd.args);

		/* If the command hasn't changed nrepeats, count this execution. */
		if (cmd.nrepeats > 0 && cmd.nrepeats == cmd_get_nrepeats())
			cmd_set_repeat(cmd.nrepeats - 1);
	}
}

/* 
 * Remove any pending repeats from the current command. 
 */
void cmd_cancel_repeat(void)
{
	game_command *cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];

	if (cmd->nrepeats || repeating)
	{
		/* Cancel */
		cmd->nrepeats = 0;
		repeating = FALSE;
		
		/* Redraw the state (later) */
		p_ptr->redraw |= (PR_STATE);
	}
}

/* 
 * Update the number of repeats pending for the current command. 
 */
void cmd_set_repeat(int nrepeats)
{
	game_command *cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];

	cmd->nrepeats = nrepeats;
	if (nrepeats) repeating = TRUE;
	else repeating = FALSE;

	/* Redraw the state (later) */
	p_ptr->redraw |= (PR_STATE);
}

/* 
 * Return the number of repeats pending for the current command. 
 */
int cmd_get_nrepeats(void)
{
	game_command *cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];
	return cmd->nrepeats;
}

/*
 * Do not allow the current command to be repeated by the user using the
 * "repeat last command" command.
 */
void cmd_disable_repeat(void)
{
	repeat_prev_allowed = FALSE;
}
