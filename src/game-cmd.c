
#include "angband.h"
#include "game-cmd.h"
#include "object/object.h"
#include "cmds.h"

errr (*cmd_get_hook)(cmd_context c, bool wait);

#define CMD_QUEUE_SIZE 20

static int cmd_head = 0;
static int cmd_tail = 0;
static game_command cmd_queue[CMD_QUEUE_SIZE];

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
		/* If we're repeating a command, we duplicate the previous command 
		   in the next command "slot". */
		int cmd_prev = cmd_head - 1;
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

/* 
 * Inserts a command in the queue to be carried out. 
 */
errr cmd_insert(cmd_code c, ...)
{
	game_command cmd = {0};
	va_list vp;

	/* Begin the Varargs Stuff */
	va_start(vp, c);

	cmd.command = c;

	switch (c)
	{
		/* These commands take no arguments. */
		case CMD_NULL:
		case CMD_QUIT:
		case CMD_SAVE:
		case CMD_SUICIDE:
		case CMD_LOADFILE:
		case CMD_NEWGAME:
		case CMD_ROLL_STATS:
		case CMD_PREV_STATS:
		case CMD_ACCEPT_CHARACTER:
		case CMD_HELP:
		case CMD_GO_UP:
		case CMD_GO_DOWN:
		case CMD_SEARCH:
		case CMD_TOGGLE_SEARCH:
		case CMD_HOLD:
		case CMD_PICKUP:
		case CMD_ENTER_STORE:
		case CMD_REPEAT:
		{
			break;
		}

		/* These take one integer argument - a "choice" */
		case CMD_BIRTH_RESET:
		case CMD_CHOOSE_SEX:
		case CMD_CHOOSE_RACE:
		case CMD_CHOOSE_CLASS:
		case CMD_BUY_STAT:
		case CMD_SELL_STAT:
		case CMD_RESET_STATS:
		case CMD_REST:
		case CMD_STUDY_SPELL:
		{
			cmd.args[0].choice = va_arg(vp, int);
			break;
		}

		/* These take a string argument. */
		case CMD_NAME_CHOICE:
		{
			/* Take a copy, it'll last longer. */
			/* XXX Should we free this automatically when the slot
			   in the queue gets reused, or just continue to let
			   the command processor worry about it? */
			cmd.args[0].string = string_make(va_arg(vp, const char *));
			break;
		}

		/* These take a direction as an argument. */
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
			cmd.args[0].direction = va_arg(vp, int);

			/* Direction hasn't been specified, so we ask for one. */
			if (cmd.args[0].direction == DIR_UNKNOWN)
			{
				/* 
				 * If no direction supplied, abort the command. 
				 * XXX Eventually replace the get_rep_dir call
				 * with something more generalised.
				 */
				if (!get_rep_dir(&cmd.args[0].direction))
					return 1;
			}
			
			break;
		}

		/* These take a point (y, x) on the map as an argument. */
		case CMD_PATHFIND:
		{
			cmd.args[0].point.y = va_arg(vp, int);
			cmd.args[0].point.x = va_arg(vp, int);
			break;			
		}

		/* These take an item number. */
		case CMD_UNINSCRIBE:
		case CMD_WIELD:
		case CMD_TAKEOFF:
		case CMD_REFILL:
		case CMD_STUDY_BOOK:
		/* Note that if the effects change for the following, they 
		   might need to take a target as well, as below. */
		case CMD_USE_STAFF:
		case CMD_EAT:
		{
			cmd.args[0].item = va_arg(vp, int);
			break;
		}
		  
		/* 
		 * These take an item number and a  "target" as arguments, 
		 * though a target isn't always actually needed, so we'll 
		 * only prompt for it via callback
		 * if the item being used needs it.
		 */
		case CMD_USE_WAND:
		case CMD_USE_ROD:
		case CMD_QUAFF:
		case CMD_ACTIVATE:
		case CMD_READ_SCROLL:
		case CMD_FIRE:
		case CMD_THROW:
		{
			cmd.args[0].item = va_arg(vp, int);
			cmd.args[1].direction = va_arg(vp, int);

			if (cmd.args[1].direction == DIR_UNKNOWN && 
				obj_needs_aim(object_from_item_idx(cmd.args[0].choice)))
			{
				if (!get_aim_dir(&cmd.args[1].direction))
					return 1;
			}

			break;
		}

		/* This takes a choice and a direction. */
		case CMD_CAST:
		{
			cmd.args[0].choice = va_arg(vp, int);
			cmd.args[1].direction = va_arg(vp, int);

			if (cmd.args[1].direction == DIR_UNKNOWN && 
				spell_needs_aim(cp_ptr->spell_book, cmd.args[0].choice))
			{
				if (!get_aim_dir(&cmd.args[1].direction))
					return 1;
			}

			break;
		}

		/* These take an item number and a number of those items to process. */
		case CMD_DROP:
		case CMD_DESTROY:
		case CMD_SELL:
		case CMD_BUY:
		case CMD_STASH:
		case CMD_RETRIEVE:
		{
			/* TODO: Number should probably be replaced by 'repeat'ing */
			cmd.args[0].item = va_arg(vp, int);
			cmd.args[1].number = va_arg(vp, int);
			break;
		}

		/* Takes an item number and a string to inscribe. */
		case CMD_INSCRIBE:
		{
			cmd.args[0].item = va_arg(vp, int);
			cmd.args[1].string = string_make(va_arg(vp, const char *));
			break;
		}
	}

	/* End the Varargs Stuff */
	va_end(vp);

	return cmd_insert_s(&cmd);
}


/* A simple list of commands and their handling functions. */
static struct
{
	cmd_code cmd;
	cmd_handler_fn fn;
	bool repeat_allowed;
} game_cmds[] =
{
	{ CMD_GO_UP, do_cmd_go_up, FALSE },
	{ CMD_GO_DOWN, do_cmd_go_down, FALSE },
	{ CMD_SEARCH, do_cmd_search, TRUE },
	{ CMD_TOGGLE_SEARCH, do_cmd_toggle_search, FALSE },
	{ CMD_WALK, do_cmd_walk, TRUE },
	{ CMD_RUN, do_cmd_run, FALSE },
	{ CMD_JUMP, do_cmd_jump, FALSE },
	{ CMD_OPEN, do_cmd_open, TRUE },
	{ CMD_CLOSE, do_cmd_close, TRUE },
	{ CMD_TUNNEL, do_cmd_tunnel, TRUE },
	{ CMD_HOLD, do_cmd_hold, TRUE },
	{ CMD_DISARM, do_cmd_disarm, TRUE },
	{ CMD_BASH, do_cmd_bash, TRUE },
	{ CMD_ALTER, do_cmd_alter, TRUE },
	{ CMD_JAM, do_cmd_spike, FALSE },
	{ CMD_REST, do_cmd_rest, FALSE },
	{ CMD_PATHFIND, do_cmd_pathfind, FALSE },
	{ CMD_PICKUP, do_cmd_pickup, FALSE },
	{ CMD_SUICIDE, do_cmd_suicide, FALSE },
	{ CMD_SAVE, do_cmd_save_game, FALSE },
	{ CMD_QUIT, do_cmd_quit, FALSE },
	{ CMD_WIELD, do_cmd_wield, FALSE },
	{ CMD_TAKEOFF, do_cmd_takeoff, FALSE },
	{ CMD_DROP, do_cmd_drop, FALSE },
	{ CMD_UNINSCRIBE, do_cmd_uninscribe, FALSE },
	{ CMD_EAT, do_cmd_use, FALSE },
	{ CMD_QUAFF, do_cmd_use, FALSE },
	{ CMD_USE_ROD, do_cmd_use, FALSE },
	{ CMD_USE_STAFF, do_cmd_use, FALSE },
	{ CMD_USE_WAND, do_cmd_use, FALSE },
	{ CMD_READ_SCROLL, do_cmd_use, FALSE },
	{ CMD_ACTIVATE, do_cmd_use, FALSE },
	{ CMD_REFILL, do_cmd_refill, FALSE },
	{ CMD_FIRE, do_cmd_fire, FALSE },
	{ CMD_THROW, do_cmd_throw, FALSE },
	{ CMD_DESTROY, do_cmd_destroy, FALSE },
	{ CMD_ENTER_STORE, do_cmd_store, FALSE },
	{ CMD_INSCRIBE, do_cmd_inscribe, FALSE },
	{ CMD_STUDY_SPELL, do_cmd_study_spell, FALSE },
	{ CMD_STUDY_BOOK, do_cmd_study_book, FALSE },
	{ CMD_CAST, do_cmd_cast, FALSE },
	{ CMD_SELL, do_cmd_sell, FALSE },
	{ CMD_STASH, do_cmd_stash, FALSE },
	{ CMD_BUY, do_cmd_buy, FALSE },
	{ CMD_RETRIEVE, do_cmd_retrieve, FALSE },
};

/*
 * Mark a command as "allowed to be repeated".
 *
 * When a command is executed, the user has the option to request that
 * it be repeated by the UI setting p_ptr->command_arg.  If the command
 * permits repetition, then it calls this function to set 
 * p_ptr->command_rep to make it repeat until an interruption.
 */
static void allow_repeated_command(void)
{
	if (p_ptr->command_arg)
	{
		/* Set repeat count */
		p_ptr->command_rep = p_ptr->command_arg - 1;
		
		/* Redraw the state */
		p_ptr->redraw |= (PR_STATE);
		
		/* Cancel the arg */
		p_ptr->command_arg = 0;
	}
}

/* 
 * Request a game command from the uI and carry out whatever actions
 * go along with it.
 */
void process_command(cmd_context ctx, bool no_request)
{
	int i;
	game_command cmd;

	/* If we've got a command to process, do it. */
	if (cmd_get(ctx, &cmd, !no_request) == 0)
	{
		/* Obviously inefficient - can tune later if needed. */
		for (i = 0; i < N_ELEMENTS(game_cmds); i++)
		{
			if (game_cmds[i].cmd == cmd.command)
			{
				if (game_cmds[i].repeat_allowed)
					allow_repeated_command();
				
				game_cmds[i].fn(cmd.command, cmd.args);
			}
		}
	}
}


