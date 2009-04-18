
#include "angband.h"
#include "game-cmd.h"

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

/* Inserts a command in the queue to be carried out. */
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
		case CMD_OPTIONS:
		case CMD_LOADFILE:
		case CMD_NEWGAME:
		case CMD_ROLL_STATS:
		case CMD_PREV_STATS:
		case CMD_ACCEPT_CHARACTER:
		case CMD_HELP:
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
		{
			cmd.params.choice = va_arg(vp, int);
			break;
		}

		/* These take a string argument. */
		case CMD_NAME_CHOICE:
		{
			cmd.params.string = va_arg(vp, const char *);
			break;
		}
	}

	/* End the Varargs Stuff */
	va_end(vp);

	return cmd_insert_s(&cmd);
}


