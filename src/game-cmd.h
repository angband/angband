

/*
 * All valid game commands.  Not all implemented yet.
 */
typedef enum cmd_code
{
	CMD_NULL,	/* A "do nothing" command so that there's something
			   UIs can use as a "no command yet" sentinel. */

	CMD_QUIT,
	CMD_OPTIONS,

	/* Splash screen commands */
	CMD_LOADFILE,
	CMD_NEWGAME,

	/* Birth commands */
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
	
	CMD_HELP
} cmd_code;

typedef enum cmd_context
{
	CMD_INIT,
	CMD_BIRTH,
	CMD_MAP,
	CMD_STORE,
	CMD_DEATH
} cmd_context;


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

	/* 
	 * Probably handle "special" repetitions like '*' as negative 
	 * ones, as classically we limit repeats to 9999 anyway 
	 */
	int repeat; 

	union 
	{
		const char *string;
		int choice;

		int direction;

		struct 
		{
			int x;
			int y;
		} point;

		struct
		{
			int idx;
			int minimum;
		} stat_min;

		struct
		{
			object_type *object_used;
			union 
			{
				object_type *object;
				
				struct
				{
					int x;
					int y;
				} point;
				
				int direction;
			} target;
		} object;

		int stat_limits[A_MAX];
	} params;
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


