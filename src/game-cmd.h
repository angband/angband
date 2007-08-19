
/*
 * All valid game commands.  Not all implemented yet.
 */
typedef enum cmd_code
{
	CMD_NULL,	/* A "do nothing" command so that there's something
			   UIs can use as a "no command yet" sentinel. */

	CMD_QUIT,
	CMD_OPTIONS,

	CMD_LOADFILE,
	CMD_NEWGAME
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

	/* Various parameters - which ones are used depends on the command. */
	object_type *object_manipulated;

	const char *string;

	union 
	{
		object_type *object;

		struct
		{
			int x;
			int y;
		} point;
	} target;

	int direction;
} game_command;


/*
 * A function called by the game to get a command from the UI.
 * Just a hook, with the real function supplied by the UI.
 */
extern game_command (*get_game_command)(void);
