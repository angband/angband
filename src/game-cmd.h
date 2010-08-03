#ifndef INCLUDED_GAME_CMD_H
#define INCLUDED_GAME_CMD_H

/*
 * All valid game commands.  Not all implemented yet.
 */
typedef enum cmd_code
{
	CMD_NULL = 0,	/* A "do nothing" command so that there's something
					   UIs can use as a "no command yet" sentinel. */
	/* 
	 * Splash screen commands 
	 */
	CMD_LOADFILE,
	CMD_NEWGAME,

	/* 
	 * Birth commands 
	 */
	CMD_BIRTH_RESET,
	CMD_CHOOSE_SEX,
	CMD_CHOOSE_RACE,
	CMD_CHOOSE_CLASS,
	CMD_BUY_STAT,
	CMD_SELL_STAT,
	CMD_RESET_STATS,
	CMD_ROLL_STATS,
	CMD_PREV_STATS,
	CMD_NAME_CHOICE,
	CMD_ACCEPT_CHARACTER,

	/* 
	 * The main game commands
	 */
	CMD_GO_UP,
	CMD_GO_DOWN,
	CMD_SEARCH,
	CMD_TOGGLE_SEARCH,
	CMD_WALK,
	CMD_JUMP,
	CMD_PATHFIND,

	CMD_INSCRIBE,
	CMD_UNINSCRIBE,
	CMD_TAKEOFF,
	CMD_WIELD,
	CMD_DROP,
	CMD_STUDY_SPELL,
	CMD_STUDY_BOOK,
	CMD_CAST, /* Casting a spell /or/ praying. */
	CMD_USE_STAFF,
	CMD_USE_WAND,
	CMD_USE_ROD,
	CMD_ACTIVATE,
	CMD_EAT,
	CMD_QUAFF,
	CMD_READ_SCROLL,
	CMD_REFILL,
	CMD_FIRE,
	CMD_THROW,
	CMD_PICKUP,
	CMD_DESTROY,
/*	CMD_SQUELCH_TYPE, -- might be a command, might have another interface */
	CMD_DISARM,
	CMD_REST,
/*	CMD_TARGET, -- possibly should be a UI-level thing */
	CMD_TUNNEL,
	CMD_OPEN,
	CMD_CLOSE,
	CMD_JAM,
	CMD_BASH,
	CMD_RUN,
	CMD_HOLD,
	CMD_ENTER_STORE,
	CMD_ALTER,

    /* Store commands */	
	CMD_SELL,
	CMD_BUY,
	CMD_STASH,
	CMD_RETRIEVE,

	/* Hors categorie Commands */
	CMD_SUICIDE,
	CMD_SAVE,

/*	CMD_OPTIONS, -- probably won't be a command in this sense*/
	CMD_QUIT,
	CMD_HELP,
	CMD_REPEAT
}
cmd_code;

typedef enum cmd_context
{
	CMD_INIT,
	CMD_BIRTH,
	CMD_GAME,
	CMD_STORE,
	CMD_DEATH
} cmd_context;

#define DIR_UNKNOWN 0
#define DIR_TARGET 5


typedef union 
{
	const char *string;
	
	int choice;
	int item;
	int number;
	int direction;
	
	struct 
	{
		int x, y;
	} point;
} cmd_arg;

/* Maximum number of arguments a command needs to take. */
#define CMD_MAX_ARGS 2

/*
 * The game_command type is used to return details of the command the
 * game should carry out.
 *
 * 'command' should always have a valid cmd_code value, the other entries
 * may or may not be significant depending on the command being returned.
 *
 * NOTE: This is prone to change quite a bit while things are shaken out.
 */
typedef struct game_command
{
	/* A valid command code. */
	cmd_code command;

	/* Number of times to attempt to repeat command. */
	int repeat; 

	/* Arguments to the command */
	cmd_arg args[CMD_MAX_ARGS];
} game_command;


/*
 * A function called by the game to get a command from the UI.
 */
extern errr (*cmd_get_hook)(cmd_context c, bool wait);

/* Inserts a command in the queue to be carried out. */
errr cmd_insert_s(game_command *cmd);

/* 
 * Convenience function.
 * Inserts a command with params in the queue to be carried out. 
 */
errr cmd_insert(cmd_code c, ...);

/* Gets the next command from the queue, optionally waiting to allow
   the UI time to process user input, etc. if wait is TRUE */
errr cmd_get(cmd_context c,game_command *cmd, bool wait);

/* Called by the game engine to get the player's next action. */
void process_command(cmd_context c, bool no_request);

void cmd_disable_repeat(void);

#endif
