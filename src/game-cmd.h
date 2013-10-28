
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
	CMD_BIRTH_BACK,
	CMD_BIRTH_RESTART,
	CMD_BIRTH_CHOICE,
	CMD_BUY_STAT,
	CMD_SELL_STAT,
	CMD_AUTOROLL,
	CMD_ROLL,
	CMD_PREV_STATS,
	CMD_ACCEPT_STATS,
	CMD_NAME_CHOICE,
	CMD_ACCEPT_CHARACTER
} cmd_code;


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
 * Just a hook, with the real function supplied by the UI.
 */
extern game_command (*get_game_command)(void);
