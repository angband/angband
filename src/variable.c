/* variable.c: Global variables */

char *copyright[5] = {
"Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
"",
"This software may be copied and distributed for educational, research, and",
"not for profit purposes provided that this copyright and statement are",
"included in all such copies."};

#include "constant.h"
#include "config.h"
#include "types.h"

/* a horrible hack: needed because compact_monster() can be called from
   creatures() via summon_monster() and place_monster() */
int hack_monptr = -1;

int weapon_heavy = FALSE;
int pack_heavy = FALSE;
int16 log_index = -1;		/* Index to log file. (<= 0 means no log) */
vtype died_from;

vtype savefile;			/* The savefile to use. */

int16 total_winner = FALSE;
int NO_SAVE=FALSE;
int character_generated = 0;	/* don't save score until char gen finished */
int character_saved = 0;	/* prevents save on kill after save_char() */
int highscore_fd;		/* File descriptor to high score file */
int LOAD=0;
int32u randes_seed;		/* for restarting randes_state */
int32u town_seed;		/* for restarting town_seed */
int16 cur_height,cur_width;	/* Cur dungeon size    */
int16 dun_level = 0;		/* Cur dungeon level   */
int16 missile_ctr = 0;		/* Counter for missiles */
int msg_flag;			/* Set with first msg  */
vtype old_msg[MAX_SAVE_MSG];	/* Last message	      */
int16 last_msg = 0;		/* Where last is held */
int death = FALSE;		/* True if died	      */
int find_flag;			/* Used in ANGBAND for .(dir) */
int free_turn_flag;		/* Used in ANGBAND, do not move creatures  */
int command_count;		/* Gives repetition of commands. -CJS- */
int default_dir = FALSE;	/* Use last direction for repeated command */
int32 turn = -1;		/* Cur turn of game    */
int wizard = FALSE;		/* Wizard flag	      */
int to_be_wizard = FALSE;	/* used during startup, when -w option used */
int16 panic_save = FALSE;	/* this is true if playing from a panic save */
int16 noscore = FALSE;		/* Don't log the game. -CJS- */

struct unique_mon u_list[MAX_CREATURES]; /* Unique check list... -LVB- */ 

int rogue_like_commands;	/* set in config.h/main.c */

/* options set via the '=' command */
int find_cut = TRUE;
int find_examine = TRUE;
int find_bound = FALSE;
int find_prself = FALSE;
int prompt_carry_flag = FALSE;
int show_weight_flag = FALSE;
int highlight_seams = FALSE;
int find_ignore_doors = FALSE;

char doing_inven = FALSE;	/* Track inventory commands. -CJS- */
int screen_change = FALSE;	/* Track screen updates for inven_commands. */
char last_command = ' ';  	/* Memory of previous command. */

/* these used to be in dungeon.c */
int new_level_flag;		/* Next level when true	 */
int search_flag = FALSE;	/* Player is searching	 */
int teleport_flag;		/* Handle teleport traps  */
int player_light;		/* Player carrying light */
int eof_flag = FALSE;		/* Used to signal EOF/HANGUP condition */
int light_flag = FALSE;		/* Track if temporary light about player.  */

int wait_for_more = FALSE;	/* used when ^C hit during -more- prompt */
int closing_flag = FALSE;	/* Used for closing   */

/*  Following are calculated from max dungeon sizes		*/
int16 max_panel_rows,max_panel_cols;
int panel_row,panel_col;
int panel_row_min,panel_row_max;
int panel_col_min,panel_col_max;
int panel_col_prt,panel_row_prt;

#ifdef MAC
cave_type (*cave)[MAX_WIDTH];
#else
cave_type cave[MAX_HEIGHT][MAX_WIDTH];
#endif

#ifdef MAC
recall_type *c_recall;
#else
recall_type c_recall[MAX_CREATURES];	/* Monster memories */
#endif
