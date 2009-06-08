
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
	cmd_queue[cmd_head] = *cmd;

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
} game_cmds[] =
{
	{ CMD_GO_UP, do_cmd_go_up },
	{ CMD_GO_DOWN, do_cmd_go_down },
	{ CMD_SEARCH, do_cmd_search },
	{ CMD_TOGGLE_SEARCH, do_cmd_toggle_search },
	{ CMD_WALK, do_cmd_walk },
	{ CMD_RUN, do_cmd_run },
	{ CMD_JUMP, do_cmd_jump },
	{ CMD_OPEN, do_cmd_open },
	{ CMD_CLOSE, do_cmd_close },
	{ CMD_TUNNEL, do_cmd_tunnel },
	{ CMD_HOLD, do_cmd_hold },
	{ CMD_DISARM, do_cmd_disarm },
	{ CMD_BASH, do_cmd_bash },
	{ CMD_ALTER, do_cmd_alter },
	{ CMD_JAM, do_cmd_spike },
	{ CMD_REST, do_cmd_rest },
	{ CMD_PATHFIND, do_cmd_pathfind },
	{ CMD_PICKUP, do_cmd_pickup },
	{ CMD_SUICIDE, do_cmd_suicide },
	{ CMD_SAVE, do_cmd_save_game },
	{ CMD_QUIT, do_cmd_quit },
	{ CMD_WIELD, do_cmd_wield },
	{ CMD_TAKEOFF, do_cmd_takeoff },
	{ CMD_DROP, do_cmd_drop },
	{ CMD_UNINSCRIBE, do_cmd_uninscribe },
	{ CMD_EAT, do_cmd_use },
	{ CMD_QUAFF, do_cmd_use },
	{ CMD_USE_ROD, do_cmd_use },
	{ CMD_USE_STAFF, do_cmd_use },
	{ CMD_USE_WAND, do_cmd_use },
	{ CMD_READ_SCROLL, do_cmd_use },
	{ CMD_ACTIVATE, do_cmd_use },
	{ CMD_REFILL, do_cmd_refill },
	{ CMD_FIRE, do_cmd_fire },
	{ CMD_THROW, do_cmd_throw },
	{ CMD_DESTROY, do_cmd_destroy },
	{ CMD_ENTER_STORE, do_cmd_store },
	{ CMD_INSCRIBE, do_cmd_inscribe },
	{ CMD_STUDY_SPELL, do_cmd_study_spell },
	{ CMD_STUDY_BOOK, do_cmd_study_book },
	{ CMD_CAST, do_cmd_cast },
	{ CMD_SELL, do_cmd_sell },
	{ CMD_STASH, do_cmd_stash },
	{ CMD_BUY, do_cmd_buy },
	{ CMD_RETRIEVE, do_cmd_retrieve },
};


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
				game_cmds[i].fn(cmd.command, cmd.args);
		}
	}
}


