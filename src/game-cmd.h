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
	CMD_FINALIZE_OPTIONS,
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
	CMD_BROWSE_SPELL,
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
	CMD_AUTOPICKUP,
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

enum {
	DIR_UNKNOWN = 0,
	DIR_NW = 1,
	DIR_N = 2,
	DIR_NE = 3,
	DIR_W = 4,
	DIR_TARGET = 5,
	DIR_NONE = 5,
	DIR_E = 6,
	DIR_SW = 7,
	DIR_S = 8,
	DIR_SE = 9,
};

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

enum cmd_arg_type
{
	arg_NONE = 0,
	arg_STRING = 0x01,
	arg_CHOICE = 0x02,
	arg_NUMBER = 0x04,
	arg_ITEM = 0x08,
	arg_DIRECTION = 0x10,
	arg_TARGET = 0x20,
	arg_POINT = 0x40
};

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
	int nrepeats;

	/* Arguments to the command */
	cmd_arg arg[CMD_MAX_ARGS];

	/* Whether an argument was passed or not */
	bool arg_present[CMD_MAX_ARGS];

	/* Types of the arguments passed */
	enum cmd_arg_type arg_type[CMD_MAX_ARGS];
} game_command;


/**
 * Returns the top command on the queue.
 */
game_command *cmd_get_top(void);

/*
 * A function called by the game to get a command from the UI.
 */
extern errr (*cmd_get_hook)(cmd_context c, bool wait);

/* Inserts a command in the queue to be carried out. */
errr cmd_insert_s(game_command *cmd);

/* 
 * Convenience functions.
 * Insert a command with params in the queue to be carried out.
 */
errr cmd_insert_repeated(cmd_code c, int nrepeats);
errr cmd_insert(cmd_code c);

/**
 * Set the args of a command.
 */
void cmd_set_arg_choice(game_command *cmd, int n, int choice);
void cmd_set_arg_string(game_command *cmd, int n, const char *str);
void cmd_set_arg_direction(game_command *cmd, int n, int dir);
void cmd_set_arg_target(game_command *cmd, int n, int target);
void cmd_set_arg_point(game_command *cmd, int n, int x, int y);
void cmd_set_arg_item(game_command *cmd, int n, int item);
void cmd_set_arg_number(game_command *cmd, int n, int num);

/* 
 * Gets the next command from the queue, optionally waiting to allow
 * the UI time to process user input, etc. if wait is TRUE 
 */
errr cmd_get(cmd_context c, game_command **cmd, bool wait);

/* Called by the game engine to get the player's next action. */
void process_command(cmd_context c, bool no_request);

/* Remove any pending repeats from the current command. */
void cmd_cancel_repeat(void);

/* Update the number of repeats pending for the current command. */
void cmd_set_repeat(int nrepeats);

/*
 * Call to disallow the current command from being repeated with the
 * "Repeat last command" command.
 */
void cmd_disable_repeat(void);

/* 
 * Returns the number of repeats left for the current command.
 * i.e. zero if not repeating.
 */
int cmd_get_nrepeats(void);

#endif
