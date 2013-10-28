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
#include "object/tvalsval.h"
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

/* A simple list of commands and their handling functions. */
static struct
{
	cmd_code cmd;
	enum cmd_arg_type arg_type[CMD_MAX_ARGS];
	cmd_handler_fn fn;
	bool repeat_allowed;
	int auto_repeat_n;
} game_cmds[] =
{
	{ CMD_LOADFILE, { arg_NONE }, NULL, FALSE, 0 },
	{ CMD_NEWGAME, { arg_NONE }, NULL, FALSE, 0 },

	{ CMD_BIRTH_RESET, { arg_NONE }, NULL, FALSE, 0 },
	{ CMD_CHOOSE_SEX, { arg_CHOICE }, NULL, FALSE, 0 },
	{ CMD_CHOOSE_RACE, { arg_CHOICE }, NULL, FALSE, 0 },
	{ CMD_CHOOSE_CLASS, { arg_CHOICE }, NULL, FALSE, 0 },
 	{ CMD_FINALIZE_OPTIONS, { arg_CHOICE }, NULL, FALSE },
	{ CMD_BUY_STAT, { arg_CHOICE }, NULL, FALSE, 0 },
	{ CMD_SELL_STAT, { arg_CHOICE }, NULL, FALSE, 0 },
	{ CMD_RESET_STATS, { arg_CHOICE }, NULL, FALSE, 0 },
	{ CMD_ROLL_STATS, { arg_NONE }, NULL, FALSE, 0 },
	{ CMD_PREV_STATS, { arg_NONE }, NULL, FALSE, 0 },
	{ CMD_NAME_CHOICE, { arg_STRING }, NULL, FALSE, 0 },
	{ CMD_ACCEPT_CHARACTER, { arg_NONE }, NULL, FALSE, 0 },

	{ CMD_GO_UP, { arg_NONE }, do_cmd_go_up, FALSE, 0 },
	{ CMD_GO_DOWN, { arg_NONE }, do_cmd_go_down, FALSE, 0 },
	{ CMD_SEARCH, { arg_NONE }, do_cmd_search, TRUE, 10 },
	{ CMD_TOGGLE_SEARCH, { arg_NONE }, do_cmd_toggle_search, FALSE, 0 },
	{ CMD_WALK, { arg_DIRECTION }, do_cmd_walk, TRUE, 0 },
	{ CMD_RUN, { arg_DIRECTION }, do_cmd_run, FALSE, 0 },
	{ CMD_JUMP, { arg_DIRECTION }, do_cmd_jump, FALSE, 0 },
	{ CMD_OPEN, { arg_DIRECTION }, do_cmd_open, TRUE, 99 },
	{ CMD_CLOSE, { arg_DIRECTION }, do_cmd_close, TRUE, 99 },
	{ CMD_TUNNEL, { arg_DIRECTION }, do_cmd_tunnel, TRUE, 99 },
	{ CMD_HOLD, { arg_NONE }, do_cmd_hold, TRUE, 0 },
	{ CMD_DISARM, { arg_DIRECTION }, do_cmd_disarm, TRUE, 99 },
	{ CMD_BASH, { arg_DIRECTION }, do_cmd_bash, TRUE, 99 },
	{ CMD_ALTER, { arg_DIRECTION }, do_cmd_alter, TRUE, 99 },
	{ CMD_JAM, { arg_DIRECTION }, do_cmd_spike, FALSE, 0 },
	{ CMD_REST, { arg_CHOICE }, do_cmd_rest, FALSE, 0 },
	{ CMD_PATHFIND, { arg_POINT }, do_cmd_pathfind, FALSE, 0 },
	{ CMD_PICKUP, { arg_ITEM }, do_cmd_pickup, FALSE, 0 },
	{ CMD_AUTOPICKUP, { arg_NONE }, do_cmd_autopickup, FALSE, 0 },
	{ CMD_WIELD, { arg_ITEM, arg_NUMBER }, do_cmd_wield, FALSE, 0 },
	{ CMD_TAKEOFF, { arg_ITEM }, do_cmd_takeoff, FALSE, 0 },
	{ CMD_DROP, { arg_ITEM, arg_NUMBER }, do_cmd_drop, FALSE, 0 },
	{ CMD_UNINSCRIBE, { arg_ITEM }, do_cmd_uninscribe, FALSE, 0 },
	{ CMD_EAT, { arg_ITEM }, do_cmd_use, FALSE, 0 },
	{ CMD_QUAFF, { arg_ITEM, arg_TARGET }, do_cmd_use, FALSE, 0 },
	{ CMD_USE_ROD, { arg_ITEM, arg_TARGET }, do_cmd_use, FALSE, 0 },
	{ CMD_USE_STAFF, { arg_ITEM }, do_cmd_use, FALSE, 0 },
	{ CMD_USE_WAND, { arg_ITEM, arg_TARGET }, do_cmd_use, FALSE, 0 },
	{ CMD_READ_SCROLL, { arg_ITEM, arg_TARGET }, do_cmd_use, FALSE, 0 },
	{ CMD_ACTIVATE, { arg_ITEM, arg_TARGET }, do_cmd_use, FALSE, 0 },
	{ CMD_REFILL, { arg_ITEM }, do_cmd_refill, FALSE, 0 },
	{ CMD_FIRE, { arg_ITEM, arg_TARGET }, do_cmd_fire, FALSE, 0 },
	{ CMD_THROW, { arg_ITEM, arg_TARGET }, do_cmd_throw, FALSE, 0 },
	{ CMD_DESTROY, { arg_ITEM }, do_cmd_destroy, FALSE, 0 },
	{ CMD_ENTER_STORE, { arg_NONE }, do_cmd_store, FALSE, 0 },
	{ CMD_INSCRIBE, { arg_ITEM, arg_STRING }, do_cmd_inscribe, FALSE, 0 },
	{ CMD_STUDY_SPELL, { arg_CHOICE }, do_cmd_study_spell, FALSE, 0 },
	{ CMD_STUDY_BOOK, { arg_ITEM }, do_cmd_study_book, FALSE, 0 },
	{ CMD_CAST, { arg_CHOICE, arg_TARGET }, do_cmd_cast, FALSE, 0 },
	{ CMD_SELL, { arg_ITEM, arg_NUMBER }, do_cmd_sell, FALSE, 0 },
	{ CMD_STASH, { arg_ITEM, arg_NUMBER }, do_cmd_stash, FALSE, 0 },
	{ CMD_BUY, { arg_CHOICE, arg_NUMBER }, do_cmd_buy, FALSE, 0 },
	{ CMD_RETRIEVE, { arg_CHOICE, arg_NUMBER }, do_cmd_retrieve, FALSE, 0 },
	{ CMD_SUICIDE, { arg_NONE }, do_cmd_suicide, FALSE, 0 },
	{ CMD_SAVE, { arg_NONE }, do_cmd_save_game, FALSE, 0 },
	{ CMD_QUIT, { arg_NONE }, do_cmd_quit, FALSE, 0 },
	{ CMD_HELP, { arg_NONE }, NULL, FALSE, 0 },
	{ CMD_REPEAT, { arg_NONE }, NULL, FALSE, 0 },
};

/* Item selector type (everything required for get_item()) */
struct item_selector
{
	cmd_code command;
	const char *prompt;
	const char *noop;

	bool (*filter)(const object_type *o_ptr);
	int mode;
};

/** List of requirements for various commands' objects */
struct item_selector item_selector[] =
{
	{ CMD_INSCRIBE, "Inscribe which item? ",
	  "You have nothing to inscribe.",
	  NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS) },

	{ CMD_UNINSCRIBE, "Un-inscribe which item? ",
	  "You have nothing to un-inscribe.",
	  obj_has_inscrip, (USE_EQUIP | USE_INVEN | USE_FLOOR) },

	{ CMD_WIELD, "Wear/wield which item? ",
	  "You have nothing you can wear or wield.",
	  obj_can_wear, (USE_INVEN | USE_FLOOR) },

	{ CMD_TAKEOFF, "Take off which item? ",
	  "You are not wearing anything you can take off.",
	  obj_can_takeoff, USE_EQUIP },

	{ CMD_DROP, "Drop which item? ",
	  "You have nothing to drop.",
	  NULL, (USE_EQUIP | USE_INVEN) },

	{ CMD_FIRE, "Fire which item? ",
	  "You have nothing to fire.",
	  obj_can_fire, (USE_INVEN | USE_EQUIP | USE_FLOOR | QUIVER_TAGS) },

	{ CMD_USE_STAFF, "Use which staff? ",
	  "You have no staff to use.",
	  obj_is_staff, (USE_INVEN | USE_FLOOR | SHOW_FAIL) },

	{ CMD_USE_WAND, "Aim which wand? ",
	  "You have no wand to aim.",
	  obj_is_wand, (USE_INVEN | USE_FLOOR | SHOW_FAIL) },

	{ CMD_USE_ROD, "Zap which rod? ",
	  "You have no charged rods to zap.",
	  obj_is_rod, (USE_INVEN | USE_FLOOR | SHOW_FAIL) },

	{ CMD_ACTIVATE, "Activate which item? ",
	  "You have nothing to activate.",
	  obj_is_activatable, (USE_EQUIP | SHOW_FAIL) },

	{ CMD_EAT, "Eat which item? ",
	  "You have nothing to eat.",
	  obj_is_food, (USE_INVEN | USE_FLOOR) },

	{ CMD_QUAFF, "Quaff which potion? ",
	  "You have no potions to quaff.",
	  obj_is_potion, (USE_INVEN | USE_FLOOR) },

	{ CMD_READ_SCROLL, "Read which scroll? ",
	  "You have no scrolls to read.",
	  obj_is_scroll, (USE_INVEN | USE_FLOOR) },

	{ CMD_REFILL, "Refuel with what fuel source? ",
	  "You have nothing to refuel with.",
	  obj_can_refill, (USE_INVEN | USE_FLOOR) },
};

game_command *cmd_get_top(void)
{
	return &cmd_queue[prev_cmd_idx(cmd_head)];
}


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
errr cmd_get(cmd_context c, game_command **cmd, bool wait)
{
	/* If we're repeating, just pull the last command again. */
	if (repeating)
	{
		*cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];
		return 0;
	}

	/* If there are no commands queued, ask the UI for one. */
	if (cmd_head == cmd_tail) 
		cmd_get_hook(c, wait);

	/* If we have a command ready, set it and return success. */
	if (cmd_head != cmd_tail)
	{
		*cmd = &cmd_queue[cmd_tail++];
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
			return i;
	}

	return -1;
}

void cmd_set_arg_choice(game_command *cmd, int n, int choice)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_CHOICE);

	cmd->arg[n].choice = choice;
	cmd->arg_type[n] = arg_CHOICE;
	cmd->arg_present[n] = TRUE;
}

void cmd_set_arg_string(game_command *cmd, int n, const char *str)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_STRING);

	cmd->arg[n].string = string_make(str);
	cmd->arg_type[n] = arg_STRING;
	cmd->arg_present[n] = TRUE;
}

void cmd_set_arg_direction(game_command *cmd, int n, int dir)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_DIRECTION);

	cmd->arg[n].direction = dir;
	cmd->arg_type[n] = arg_DIRECTION;
	cmd->arg_present[n] = TRUE;
}

void cmd_set_arg_target(game_command *cmd, int n, int target)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_TARGET);

	cmd->arg[n].direction = target;
	cmd->arg_type[n] = arg_TARGET;
	cmd->arg_present[n] = TRUE;
}

void cmd_set_arg_point(game_command *cmd, int n, int x, int y)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_POINT);

	cmd->arg[n].point.x = x;
	cmd->arg[n].point.y = y;
	cmd->arg_type[n] = arg_POINT;
	cmd->arg_present[n] = TRUE;
}

void cmd_set_arg_item(game_command *cmd, int n, int item)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_ITEM);

	cmd->arg[n].item = item;
	cmd->arg_type[n] = arg_ITEM;
	cmd->arg_present[n] = TRUE;
}

void cmd_set_arg_number(game_command *cmd, int n, int num)
{
	int idx = cmd_idx(cmd->command);

	assert(n <= CMD_MAX_ARGS);
	assert(game_cmds[idx].arg_type[n] & arg_NUMBER);

	cmd->arg[n].number = num;
	cmd->arg_type[n] = arg_NUMBER;
	cmd->arg_present[n] = TRUE;
}

/*
 * Inserts a command in the queue to be carried out, with the given
 * number of repeats.
 */
errr cmd_insert_repeated(cmd_code c, int nrepeats)
{
	game_command cmd = { 0 };

	if (cmd_idx(c) == -1)
		return 1;

	cmd.command = c;
	cmd.nrepeats = nrepeats;

	return cmd_insert_s(&cmd);
}

/* 
 * Inserts a command in the queue to be carried out. 
 */
errr cmd_insert(cmd_code c)
{
	return cmd_insert_repeated(c, 0);
}


/* 
 * Request a game command from the uI and carry out whatever actions
 * go along with it.
 */
void process_command(cmd_context ctx, bool no_request)
{
	game_command *cmd;

	/* Reset so that when selecting items, we look in the default location */
	p_ptr->command_wrk = 0;

	/* If we've got a command to process, do it. */
	if (cmd_get(ctx, &cmd, !no_request) == 0)
	{
		int oldrepeats = cmd->nrepeats;
		int idx = cmd_idx(cmd->command);
		size_t i;

		if (idx == -1) return;

		for (i = 0; i < N_ELEMENTS(item_selector); i++)
		{
			struct item_selector *is = &item_selector[i];

			if (is->command != cmd->command)
				continue;

			if (!cmd->arg_present[0])
			{
				int item;

				item_tester_hook = is->filter;
				if (!get_item(&item, is->prompt, is->noop, cmd->command, is->mode))
					return;

				cmd_set_arg_item(cmd, 0, item);
			}
		}

		/* XXX avoid dead objects from being re-used on repeat.
		 * this needs to be expanded into a general safety-check
		 * on args */
		if ((game_cmds[idx].arg_type[0] == arg_ITEM) && cmd->arg_present[0]) {
			object_type *o_ptr = object_from_item_idx(cmd->arg[0].item);
			if (!o_ptr->kind)
				return;
		}

		/* Do some sanity checking on those arguments that might have 
		   been declared as "unknown", such as directions and targets. */
		switch (cmd->command)
		{
			case CMD_INSCRIBE:
			{
				char o_name[80];
				char tmp[80] = "";

				object_type *o_ptr = object_from_item_idx(cmd->arg[0].item);
			
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);
				msg("Inscribing %s.", o_name);
				message_flush();
			
				/* Use old inscription */
				if (o_ptr->note)
					strnfmt(tmp, sizeof(tmp), "%s", quark_str(o_ptr->note));
			
				/* Get a new inscription (possibly empty) */
				if (!get_string("Inscription: ", tmp, sizeof(tmp)))
					return;

				cmd_set_arg_string(cmd, 1, tmp);
				break;
			}

			case CMD_OPEN:
			{
				if (OPT(easy_open) && (!cmd->arg_present[0] ||
						cmd->arg[0].direction == DIR_UNKNOWN))
				{
					int y, x;
					int n_closed_doors, n_locked_chests;
			
					n_closed_doors = count_feats(&y, &x, cave_iscloseddoor, FALSE);
					n_locked_chests = count_chests(&y, &x, FALSE);
			
					if (n_closed_doors + n_locked_chests == 1)
						cmd_set_arg_direction(cmd, 0, coords_to_dir(y, x));
				}

				goto get_dir;
			}

			case CMD_CLOSE:
			{
				if (OPT(easy_open) && (!cmd->arg_present[0] ||
						cmd->arg[0].direction == DIR_UNKNOWN))
				{
					int y, x;
			
					/* Count open doors */
					if (count_feats(&y, &x, cave_isopendoor, FALSE) == 1)
						cmd_set_arg_direction(cmd, 0, coords_to_dir(y, x));
				}

				goto get_dir;
			}

			case CMD_DISARM:
			{
				if (OPT(easy_open) && (!cmd->arg_present[0] ||
						cmd->arg[0].direction == DIR_UNKNOWN))
				{
					int y, x;
					int n_visible_traps, n_trapped_chests;
			
					n_visible_traps = count_feats(&y, &x, cave_isknowntrap, TRUE);
					n_trapped_chests = count_chests(&y, &x, TRUE);

					if (n_visible_traps + n_trapped_chests == 1)
						cmd_set_arg_direction(cmd, 0, coords_to_dir(y, x));
				}

				goto get_dir;
			}

			case CMD_TUNNEL:
			case CMD_WALK:
			case CMD_RUN:
			case CMD_JUMP:
			case CMD_BASH:
			case CMD_ALTER:
			case CMD_JAM:
			{
			get_dir:

				/* Direction hasn't been specified, so we ask for one. */
				if (!cmd->arg_present[0] ||
						cmd->arg[0].direction == DIR_UNKNOWN)
				{
					int dir;
					if (!get_rep_dir(&dir))
						return;

					cmd_set_arg_direction(cmd, 0, dir);
				}
				
				break;
			}

			case CMD_DROP:
			{
				if (!cmd->arg_present[1])
				{
					object_type *o_ptr = object_from_item_idx(cmd->arg[0].item);
					int amt = get_quantity(NULL, o_ptr->number);
					if (amt <= 0)
						return;

					cmd_set_arg_number(cmd, 1, amt);
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
				object_type *o_ptr = object_from_item_idx(cmd->arg[0].choice);

				/* If we couldn't resolve the item, then abort this */
				if (!o_ptr->kind) break;

				/* Thrown objects always need an aim, others might, depending
				 * on the object */
				if (obj_needs_aim(o_ptr) || cmd->command == CMD_THROW)
				{
					if (!cmd->arg_present[1])
						get_target = TRUE;

					if (cmd->arg[1].direction == DIR_UNKNOWN)
						get_target = TRUE;

					if (cmd->arg[1].direction == DIR_TARGET && !target_okay())
						get_target = TRUE;
				}

				if (get_target && !get_aim_dir(&cmd->arg[1].direction))
						return;

				player_confuse_dir(p_ptr, &cmd->arg[1].direction, FALSE);
				cmd->arg_present[1] = TRUE;

				break;
			}
			
			/* This takes a choice and a direction. */
			case CMD_CAST:
			{
				bool get_target = FALSE;

				if (spell_needs_aim(p_ptr->class->spell_book, cmd->arg[0].choice))
				{
					if (!cmd->arg_present[1])
						get_target = TRUE;

					if (cmd->arg[1].direction == DIR_UNKNOWN)
						get_target = TRUE;

					if (cmd->arg[1].direction == DIR_TARGET && !target_okay())
						get_target = TRUE;
				}

				if (get_target && !get_aim_dir(&cmd->arg[1].direction))
						return;

				player_confuse_dir(p_ptr, &cmd->arg[1].direction, FALSE);
				cmd->arg_present[1] = TRUE;
				
				break;
			}

			case CMD_WIELD:
			{
				object_type *o_ptr = object_from_item_idx(cmd->arg[0].choice);
				int slot = wield_slot(o_ptr);
			
				/* Usually if the slot is taken we'll just replace the item in the slot,
				 * but in some cases we need to ask the user which slot they actually
				 * want to replace */
				if (p_ptr->inventory[slot].kind)
				{
					if (o_ptr->tval == TV_RING)
					{
						const char *q = "Replace which ring? ";
						const char *s = "Error in obj_wield, please report";
						item_tester_hook = obj_is_ring;
						if (!get_item(&slot, q, s, CMD_WIELD, USE_EQUIP)) return;
					}
			
					if (obj_is_ammo(o_ptr) && !object_similar(&p_ptr->inventory[slot],
						o_ptr, OSTACK_QUIVER))
					{
						const char *q = "Replace which ammunition? ";
						const char *s = "Error in obj_wield, please report";
						item_tester_hook = obj_is_ammo;
						if (!get_item(&slot, q, s, CMD_WIELD, USE_EQUIP)) return;
					}
				}

				/* Set relevant slot */
				cmd_set_arg_number(cmd, 1, slot);

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
			if (game_cmds[idx].auto_repeat_n > 0 && cmd->nrepeats == 0)
				cmd_set_repeat(game_cmds[idx].auto_repeat_n);
		}
		else
		{
			cmd->nrepeats = 0;
			repeating = FALSE;
		}

		/* 
		 * The command gets to unset this if it isn't appropriate for
		 * the user to repeat it.
		 */
		repeat_prev_allowed = TRUE;

		if (game_cmds[idx].fn)
			game_cmds[idx].fn(cmd->command, cmd->arg);

		/* If the command hasn't changed nrepeats, count this execution. */
		if (cmd->nrepeats > 0 && oldrepeats == cmd_get_nrepeats())
			cmd_set_repeat(oldrepeats - 1);
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
