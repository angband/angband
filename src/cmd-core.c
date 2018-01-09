/**
 * \file cmd-core.c
 * \brief Handles the queueing of game commands.
 *
 * Copyright (c) 2008-9 Antony Sidwell
 * Copyright (c) 2014 Andi Sidwell
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
#include "cmd-core.h"
#include "game-input.h"
#include "obj-chest.h"
#include "obj-desc.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-util.h"
#include "store.h"
#include "target.h"

errr (*cmd_get_hook)(cmd_context c);

/**
 * ------------------------------------------------------------------------
 * A simple list of commands and their handling functions.
 * ------------------------------------------------------------------------ */
struct command_info
{
	cmd_code cmd;
	const char *verb;
	cmd_handler_fn fn;
	bool repeat_allowed;
	int auto_repeat_n;
};

static const struct command_info game_cmds[] =
{
	{ CMD_LOADFILE, "load a savefile", NULL, false, 0 },
	{ CMD_NEWGAME, "start a new game", NULL, false, 0 },

	{ CMD_BIRTH_INIT, "start the character birth process", do_cmd_birth_init, false, 0 },
	{ CMD_BIRTH_RESET, "go back to the beginning", do_cmd_birth_reset, false, 0 },
	{ CMD_CHOOSE_RACE, "select race", do_cmd_choose_race, false, 0 },
	{ CMD_CHOOSE_CLASS, "select class", do_cmd_choose_class, false, 0 },
	{ CMD_BUY_STAT, "buy points in a stat", do_cmd_buy_stat, false, 0 },
	{ CMD_SELL_STAT, "sell points in a stat", do_cmd_sell_stat, false, 0 },
	{ CMD_RESET_STATS, "reset stats", do_cmd_reset_stats, false, 0 },
	{ CMD_ROLL_STATS, "roll new stats", do_cmd_roll_stats, false, 0 },
	{ CMD_PREV_STATS, "use previously rolled stats", do_cmd_prev_stats, false, 0 },
	{ CMD_NAME_CHOICE, "choose name", do_cmd_choose_name, false, 0 },
	{ CMD_HISTORY_CHOICE, "write history", do_cmd_choose_history, false, 0 },
	{ CMD_ACCEPT_CHARACTER, "accept character", do_cmd_accept_character, false, 0 },

	{ CMD_GO_UP, "go up stairs", do_cmd_go_up, false, 0 },
	{ CMD_GO_DOWN, "go down stairs", do_cmd_go_down, false, 0 },
	{ CMD_WALK, "walk", do_cmd_walk, true, 0 },
	{ CMD_RUN, "run", do_cmd_run, true, 0 },
	{ CMD_JUMP, "jump", do_cmd_jump, false, 0 },
	{ CMD_OPEN, "open", do_cmd_open, true, 99 },
	{ CMD_CLOSE, "close", do_cmd_close, true, 99 },
	{ CMD_TUNNEL, "tunnel", do_cmd_tunnel, true, 99 },
	{ CMD_HOLD, "stay still", do_cmd_hold, true, 0 },
	{ CMD_DISARM, "disarm", do_cmd_disarm, true, 99 },
	{ CMD_ALTER, "alter", do_cmd_alter, true, 99 },
	{ CMD_REST, "rest", do_cmd_rest, false, 0 },
	{ CMD_SLEEP, "sleep", do_cmd_sleep, false, 0 },
	{ CMD_PATHFIND, "walk", do_cmd_pathfind, false, 0 },
	{ CMD_PICKUP, "pickup", do_cmd_pickup, false, 0 },
	{ CMD_AUTOPICKUP, "autopickup", do_cmd_autopickup, false, 0 },
	{ CMD_WIELD, "wear or wield", do_cmd_wield, false, 0 },
	{ CMD_TAKEOFF, "take off", do_cmd_takeoff, false, 0 },
	{ CMD_DROP, "drop", do_cmd_drop, false, 0 },
	{ CMD_UNINSCRIBE, "un-inscribe", do_cmd_uninscribe, false, 0 },
	{ CMD_AUTOINSCRIBE, "autoinscribe", do_cmd_autoinscribe, false, 0 },
	{ CMD_EAT, "eat", do_cmd_eat_food, false, 0 },
	{ CMD_QUAFF, "quaff", do_cmd_quaff_potion, false, 0 },
	{ CMD_USE_ROD, "zap", do_cmd_zap_rod, false, 0 },
	{ CMD_USE_STAFF, "use", do_cmd_use_staff, false, 0 },
	{ CMD_USE_WAND, "aim", do_cmd_aim_wand, false, 0 },
	{ CMD_READ_SCROLL, "read", do_cmd_read_scroll, false, 0 },
	{ CMD_ACTIVATE, "activate", do_cmd_activate, false, 0 },
	{ CMD_REFILL, "refuel with", do_cmd_refill, false, 0 },
	{ CMD_FIRE, "fire", do_cmd_fire, false, 0 },
	{ CMD_THROW, "throw", do_cmd_throw, false, 0 },
	{ CMD_INSCRIBE, "inscribe", do_cmd_inscribe, false, 0 },
	{ CMD_STUDY, "study", do_cmd_study, false, 0 },
	{ CMD_CAST, "cast", do_cmd_cast, false, 0 },
	{ CMD_SELL, "sell", do_cmd_sell, false, 0 },
	{ CMD_STASH, "stash", do_cmd_stash, false, 0 },
	{ CMD_BUY, "buy", do_cmd_buy, false, 0 },
	{ CMD_RETRIEVE, "retrieve", do_cmd_retrieve, false, 0 },
	{ CMD_USE, "use", do_cmd_use, false, 0 },
	{ CMD_SUICIDE, "kill character", do_cmd_suicide, false, 0 },
	{ CMD_HELP, "help", NULL, false, 0 },
	{ CMD_REPEAT, "repeat", NULL, false, 0 },
};

const char *cmd_verb(cmd_code cmd)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(game_cmds); i++) {
		if (game_cmds[i].cmd == cmd)
			return game_cmds[i].verb;
	}
	return NULL;
}

/**
 * Return the index of the given command in the command array.
 */
static int cmd_idx(cmd_code code)
{
	size_t i;

	for (i = 0; i < N_ELEMENTS(game_cmds); i++)
		if (game_cmds[i].cmd == code)
			return i;

	return CMD_ARG_NOT_PRESENT;
}


/**
 * ------------------------------------------------------------------------
 * The command queue.
 * ------------------------------------------------------------------------ */

#define CMD_QUEUE_SIZE 20
#define prev_cmd_idx(idx) ((idx + CMD_QUEUE_SIZE - 1) % CMD_QUEUE_SIZE)

static int cmd_head = 0;
static int cmd_tail = 0;
static struct command cmd_queue[CMD_QUEUE_SIZE];

static bool repeat_prev_allowed = false;
static bool repeating = false;

struct command *cmdq_peek(void)
{
	return &cmd_queue[prev_cmd_idx(cmd_head)];
}


/**
 * Insert the given command into the command queue.
 */
errr cmdq_push_copy(struct command *cmd)
{
	/* If queue full, return error */
	if (cmd_head + 1 == cmd_tail) return 1;
	if (cmd_head + 1 == CMD_QUEUE_SIZE && cmd_tail == 0) return 1;

	/* Insert command into queue. */
	if (cmd->code != CMD_REPEAT) {
		cmd_queue[cmd_head] = *cmd;
	} else {
		int cmd_prev = cmd_head - 1;

		if (!repeat_prev_allowed) return 1;

		/* If we're repeating a command, we duplicate the previous command 
		   in the next command "slot". */
		if (cmd_prev < 0) cmd_prev = CMD_QUEUE_SIZE - 1;
		
		if (cmd_queue[cmd_prev].code != CMD_NULL)
			cmd_queue[cmd_head] = cmd_queue[cmd_prev];
	}

	/* Advance point in queue, wrapping around at the end */
	cmd_head++;
	if (cmd_head == CMD_QUEUE_SIZE) cmd_head = 0;

	return 0;	
}

/**
 * Process a game command from the UI or the command queue and carry out
 * whatever actions go along with it.
 */
void process_command(cmd_context ctx, struct command *cmd)
{
	int oldrepeats = cmd->nrepeats;
	int idx = cmd_idx(cmd->code);

	/* Reset so that when selecting items, we look in the default location */
	player->upkeep->command_wrk = 0;

	if (idx == -1) return;

	/* Command repetition */
	if (game_cmds[idx].repeat_allowed) {
		/* Auto-repeat only if there isn't already a repeat length. */
		if (game_cmds[idx].auto_repeat_n > 0 && cmd->nrepeats == 0)
			cmd_set_repeat(game_cmds[idx].auto_repeat_n);
	} else {
		cmd->nrepeats = 0;
		repeating = false;
	}

	/* The command gets to unset this if it isn't appropriate for
	 * the user to repeat it. */
	repeat_prev_allowed = true;

	cmd->context = ctx;

	/* Actually execute the command function */
	if (game_cmds[idx].fn)
		game_cmds[idx].fn(cmd);

	/* If the command hasn't changed nrepeats, count this execution. */
	if (cmd->nrepeats > 0 && oldrepeats == cmd_get_nrepeats())
		cmd_set_repeat(oldrepeats - 1);
}

/**
 * Get the next game command from the queue and process it.
 */
bool cmdq_pop(cmd_context c)
{
	struct command *cmd;

	/* If we're repeating, just pull the last command again. */
	if (repeating) {
		cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];
	} else if (cmd_head != cmd_tail) {
		/* If we have a command ready, set it. */
		cmd = &cmd_queue[cmd_tail++];
		if (cmd_tail == CMD_QUEUE_SIZE)
			cmd_tail = 0;
	} else {
		/* Failure to get a command. */
		return false;
	}

	/* Now process it */
	process_command(c, cmd);
	return true;
}

/**
 * Inserts a command in the queue to be carried out, with the given
 * number of repeats.
 */
errr cmdq_push_repeat(cmd_code c, int nrepeats)
{
	struct command cmd = {
		.context = CMD_INIT,
		.code = CMD_NULL,
		.nrepeats = 0,
		.arg = { { 0 } }
	};

	if (cmd_idx(c) == -1)
		return 1;

	cmd.code = c;
	cmd.nrepeats = nrepeats;

	return cmdq_push_copy(&cmd);
}

/**
 * Inserts a command in the queue to be carried out. 
 */
errr cmdq_push(cmd_code c)
{
	return cmdq_push_repeat(c, 0);
}


/**
 * Shorthand to execute all commands in the queue right now, no waiting
 * for input.
 */
void cmdq_execute(cmd_context ctx)
{
	while (cmdq_pop(ctx)) ;
}

/**
 * ------------------------------------------------------------------------
 * Handling of repeated commands
 * ------------------------------------------------------------------------ */

/**
 * Remove any pending repeats from the current command.
 */
void cmd_cancel_repeat(void)
{
	struct command *cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];

	if (cmd->nrepeats || repeating) {
		/* Cancel */
		cmd->nrepeats = 0;
		repeating = false;

		/* Redraw the state (later) */
		player->upkeep->redraw |= (PR_STATE);
	}
}

/**
 * Update the number of repeats pending for the current command.
 */
void cmd_set_repeat(int nrepeats)
{
	struct command *cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];

	cmd->nrepeats = nrepeats;
	if (nrepeats) repeating = true;
	else repeating = false;

	/* Redraw the state (later) */
	player->upkeep->redraw |= (PR_STATE);
}

/**
 * Return the number of repeats pending for the current command.
 */
int cmd_get_nrepeats(void)
{
	struct command *cmd = &cmd_queue[prev_cmd_idx(cmd_tail)];
	return cmd->nrepeats;
}

/**
 * Do not allow the current command to be repeated by the user using the
 * "repeat last command" command.
 */
void cmd_disable_repeat(void)
{
	repeat_prev_allowed = false;
}

/**
 * ------------------------------------------------------------------------
 * Argument setting/getting generics
 * ------------------------------------------------------------------------ */

/**
 * Set an argument of name 'arg' to data 'data'
 */
static void cmd_set_arg(struct command *cmd, const char *name,
						enum cmd_arg_type type, union cmd_arg_data data)
{
	size_t i;

	int first_empty = -1;
	int idx = -1;

	assert(name);
	assert(name[0]);

	/* Find an arg that either... */
	for (i = 0; i < CMD_MAX_ARGS; i++) {
		struct cmd_arg *arg = &cmd->arg[i];
		if (!arg->name[0] && first_empty == -1)
			first_empty = i;

		if (streq(arg->name, name)) {
			idx = i;
			break;
		}
	}

	assert(first_empty != -1 || idx != -1);

	if (idx == -1)
		idx = first_empty;

	cmd->arg[idx].type = type;
	cmd->arg[idx].data = data;
	my_strcpy(cmd->arg[idx].name, name, sizeof cmd->arg[0].name);
}

/**
 * Get an argument with name 'arg'
 */
static int cmd_get_arg(struct command *cmd, const char *arg,
					   enum cmd_arg_type type, union cmd_arg_data *data)
{
	size_t i;

	for (i = 0; i < CMD_MAX_ARGS; i++) {
		if (streq(cmd->arg[i].name, arg)) {
			if (cmd->arg[i].type != type)
				return CMD_ARG_WRONG_TYPE;

			*data = cmd->arg[i].data;
			return CMD_OK;
		}
	}

	return CMD_ARG_NOT_PRESENT;
 }

 

/**
 * ------------------------------------------------------------------------
 * 'Choice' type
 * ------------------------------------------------------------------------ */

/**
 * XXX This type is a hack. The only places that use this are:
 * - resting
 * - birth choices
 * - store items
 * - spells
 *
 * Each of these should have its own type, which will allow for proper
 * validity checking of data.
 */

/**
 * Set arg 'n' to 'choice'
 */
void cmd_set_arg_choice(struct command *cmd, const char *arg, int choice)
{
	union cmd_arg_data data;
	data.choice = choice;
	cmd_set_arg(cmd, arg, arg_CHOICE, data);
}

/**
 * Retrive an argument 'n' if it's a choice
 */
int cmd_get_arg_choice(struct command *cmd, const char *arg, int *choice)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_CHOICE, &data)) == CMD_OK)
		*choice = data.choice;

	return err;
}


/**
 * Get a spell from the user, trying the command first but then prompting
 */
int cmd_get_spell(struct command *cmd, const char *arg, int *spell,
				  const char *verb, item_tester book_filter, const char *error,
				  bool (*spell_filter)(int spell))
{
	struct object *book;

	/* See if we've been provided with this one */
	if (cmd_get_arg_choice(cmd, arg, spell) == CMD_OK) {
		/* Ensure it passes the filter */
		if (!spell_filter || spell_filter(*spell) == true)
			return CMD_OK;
	}

	/* See if we've been given a book to look at */
	if (cmd_get_arg_item(cmd, "book", &book) == CMD_OK)
		*spell = get_spell_from_book(verb, book, error, spell_filter);
	else
		*spell = get_spell(verb, book_filter, cmd->code, error, spell_filter);

	if (*spell >= 0) {
		cmd_set_arg_choice(cmd, arg, *spell);
		return CMD_OK;
	}

	return CMD_ARG_ABORTED;
}

/**
 * ------------------------------------------------------------------------
 * Strings
 * ------------------------------------------------------------------------ */

/**
 * Set arg 'n' to given string
 */
void cmd_set_arg_string(struct command *cmd, const char *arg, const char *str)
{
	union cmd_arg_data data;
	data.string = string_make(str);
	cmd_set_arg(cmd, arg, arg_STRING, data);
}

/**
 * Retrieve arg 'n' if a string
 */
int cmd_get_arg_string(struct command *cmd, const char *arg, const char **str)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_STRING, &data)) == CMD_OK)
		*str = data.string;

	return err;
}

/**
 * Get a string, first from the command or failing that prompt the user
 */
int cmd_get_string(struct command *cmd, const char *arg, const char **str,
				   const char *initial, const char *title, const char *prompt)
{
	char tmp[80] = "";

	if (cmd_get_arg_string(cmd, arg, str) == CMD_OK)
		return CMD_OK;

	/* Introduce */
	msg("%s", title);
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Prompt properly */
	if (initial)
		my_strcpy(tmp, initial, sizeof tmp);

	if (get_string(prompt, tmp, sizeof tmp)) {
		cmd_set_arg_string(cmd, arg, tmp);
		if (cmd_get_arg_string(cmd, arg, str) == CMD_OK)
			return CMD_OK;
	}

	return CMD_ARG_ABORTED;
}

/**
 * ------------------------------------------------------------------------
 * Directions
 * ------------------------------------------------------------------------ */

/**
 * Set arg 'n' to given direction
 */
void cmd_set_arg_direction(struct command *cmd, const char *arg, int dir)
{
	union cmd_arg_data data;
	data.direction = dir;
	cmd_set_arg(cmd, arg, arg_DIRECTION, data);
}

/**
 * Retrieve arg 'n' if a direction
 */
int cmd_get_arg_direction(struct command *cmd, const char *arg, int *dir)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_DIRECTION, &data)) == CMD_OK)
		*dir = data.direction;

	return err;
}

/**
 * Get a direction, first from command or prompt otherwise
 */
int cmd_get_direction(struct command *cmd, const char *arg, int *dir,
					  bool allow_5)
{
	if (cmd_get_arg_direction(cmd, arg, dir) == CMD_OK) {
		/* Validity check */
		if (dir != DIR_UNKNOWN)
			return CMD_OK;
	}

	/* We need to do extra work */
	if (get_rep_dir(dir, allow_5)) {
		cmd_set_arg_direction(cmd, arg, *dir);
		return CMD_OK;
	}

	cmd_cancel_repeat();
	return CMD_ARG_ABORTED;
}

/**
 * ------------------------------------------------------------------------
 * Targets
 * ------------------------------------------------------------------------ */

/**
 * XXX Should this be unified with the arg_DIRECTION type?
 *
 * XXX Should we abolish DIR_TARGET and instead pass a struct target which
 * contains all relevant info?
 */

/**
 * Set arg 'n' to target
 */
void cmd_set_arg_target(struct command *cmd, const char *arg, int target)
{
	union cmd_arg_data data;
	data.direction = target;
	cmd_set_arg(cmd, arg, arg_TARGET, data);
}

/**
 * Retrieve arg 'n' if it's a target
 */
int cmd_get_arg_target(struct command *cmd, const char *arg, int *target)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_TARGET, &data)) == CMD_OK)
		*target = data.direction;

	return err;
}

/**
 * Get a target, first from command or prompt otherwise
 */
int cmd_get_target(struct command *cmd, const char *arg, int *target)
{
	if (cmd_get_arg_target(cmd, arg, target) == CMD_OK) {
		if (*target != DIR_UNKNOWN &&
				(*target != DIR_TARGET || target_okay()))
			return CMD_OK;
	}

	if (get_aim_dir(target)) {
		cmd_set_arg_target(cmd, arg, *target);
		return CMD_OK;
	}

	return CMD_ARG_ABORTED;
}

/**
 * ------------------------------------------------------------------------
 * Points (presently unused)
 * ------------------------------------------------------------------------ */

/*
 * XXX Use struct loc instead
 */

/**
 * Set argument 'n' to point x,y
 */
void cmd_set_arg_point(struct command *cmd, const char *arg, int x, int y)
{
	union cmd_arg_data data;
	data.point = loc(x, y);
	cmd_set_arg(cmd, arg, arg_POINT, data);
}

/**
 * Retrieve argument 'n' if it's a point
 */
int cmd_get_arg_point(struct command *cmd, const char *arg, int *x, int *y)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_POINT, &data)) == CMD_OK) {
		*x = data.point.x;
		*y = data.point.y;
	}

	return err;
}

/**
 * ------------------------------------------------------------------------
 * Item arguments
 * ------------------------------------------------------------------------ */

/**
 * Set argument 'n' to 'item'
 */
void cmd_set_arg_item(struct command *cmd, const char *arg, struct object *obj)
{
	union cmd_arg_data data;
	data.obj = obj;
	cmd_set_arg(cmd, arg, arg_ITEM, data);
}

/**
 * Retrieve argument 'n' as an item
 */
int cmd_get_arg_item(struct command *cmd, const char *arg, struct object **obj)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_ITEM, &data)) == CMD_OK)
		*obj = data.obj;

	return err;
}

/**
 * Get an item, first from the command or try the UI otherwise
 */
int cmd_get_item(struct command *cmd, const char *arg, struct object **obj,
				 const char *prompt, const char *reject, item_tester filter,
				 int mode)
{
	if (cmd_get_arg_item(cmd, arg, obj) == CMD_OK)
		return CMD_OK;

	/* Shapechanged players can only access the floor */
	if (player_is_shapechanged(player)) {
		mode &= ~(USE_EQUIP | USE_INVEN | USE_QUIVER);
	}

	if (get_item(obj, prompt, reject, cmd->code, filter, mode)) {
		cmd_set_arg_item(cmd, arg, *obj);
		return CMD_OK;
	}

	return CMD_ARG_ABORTED;
}

/**
 * ------------------------------------------------------------------------
 * Numbers, quantities
 * ------------------------------------------------------------------------ */

/**
 * Set argument 'n' to 'number'
 */
void cmd_set_arg_number(struct command *cmd, const char *arg, int amt)
{
	union cmd_arg_data data;
	data.number = amt;
	cmd_set_arg(cmd, arg, arg_NUMBER, data);
}

/**
 * Get argument 'n' as a number
 */
int cmd_get_arg_number(struct command *cmd, const char *arg, int *amt)
{
	union cmd_arg_data data;
	int err;

	if ((err = cmd_get_arg(cmd, arg, arg_NUMBER, &data)) == CMD_OK)
		*amt = data.number;

	return err;
}

/**
 * Get argument 'n' as a number; failing that, prompt for input
 */
int cmd_get_quantity(struct command *cmd, const char *arg, int *amt, int max)
{
	if (cmd_get_arg_number(cmd, arg, amt) == CMD_OK)
		return CMD_OK;

	*amt = get_quantity(NULL, max);
	if (*amt > 0) {
		cmd_set_arg_number(cmd, arg, *amt);
		return CMD_OK;
	}

	return CMD_ARG_ABORTED;
}
