/* vars2.c: Global variables, except for cave array...
  cave array is barely small enough to fit into a 64k segment by
  itself.  Put any new variables here...  Leave vars2.c alone, unless
  the 64k segment barrier is overcome, or unless cave[][] gets
  smaller..  -CFT

  Some #ifdef MSDOS type changes have made cave[][] smaller, but it's
  still more than 2/3 of a segment, so I'm going to leave it separate. -CFT
*/

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

#ifdef MSDOS
int8u weapon_heavy = FALSE;
int8u pack_heavy = FALSE;
#else
int weapon_heavy = FALSE;
int pack_heavy = FALSE;
#endif

int16 log_index = -1;		/* Index to log file. (<= 0 means no log) */
vtype died_from;

vtype savefile;			/* The savefile to use. */

#ifdef MSDOS
int16 total_winner = FALSE;
int8u NO_SAVE=FALSE;
int8u character_generated = 0;	/* don't save score until char gen finished */
int8u character_saved = 0;	/* prevents save on kill after save_char() */
#else
int16 total_winner = FALSE;
int NO_SAVE=FALSE;
int character_generated = 0;	/* don't save score until char gen finished */
int character_saved = 0;	/* prevents save on kill after save_char() */
#endif
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
#ifdef MSDOS
int8u death = FALSE;		/* True if died	      */
int8u find_flag;			/* Used in ANGBAND for .(dir) */
int8u free_turn_flag;		/* Used in ANGBAND, do not move creatures  */
#else
int death = FALSE;		/* True if died	      */
int find_flag;			/* Used in ANGBAND for .(dir) */
int free_turn_flag;		/* Used in ANGBAND, do not move creatures  */
#endif
int command_count;		/* Gives repetition of commands. -CJS- */
int32 turn = -1;		/* Cur turn of game    */
#ifdef MSDOS
int8u default_dir = FALSE;	/* Use last direction for repeated command */
int8u wizard = FALSE;		/* Wizard flag	      */
int8u to_be_wizard = FALSE;	/* used during startup, when -w option used */
int16u panic_save = FALSE;	/* this is true if playing from a panic save */
int16u noscore = FALSE;		/* Don't log the game. -CJS- */
int8u rogue_like_commands;	/* set in config.h/main.c */
#else
int default_dir = FALSE;	/* Use last direction for repeated command */
int wizard = FALSE;		/* Wizard flag	      */
int to_be_wizard = FALSE;	/* used during startup, when -w option used */
int16 panic_save = FALSE;	/* this is true if playing from a panic save */
int16 noscore = FALSE;		/* Don't log the game. -CJS- */
int rogue_like_commands;	/* set in config.h/main.c */
#endif
struct unique_mon u_list[MAX_CREATURES]; /* Unique check list... -LVB- */ 


/* options set via the '=' command */
#ifdef MSDOS
int8u find_cut = TRUE;
int8u find_examine = TRUE;
int8u find_bound = FALSE;
int8u find_prself = FALSE;
int8u prompt_carry_flag = FALSE;
int8u show_weight_flag = FALSE;
int8u highlight_seams = FALSE;
int8u find_ignore_doors = FALSE;
int8u sound_beep_flag = FALSE; /* I hate beeps! -CFT */
int8u no_haggle_flag = FALSE; /* for those who find it tedious... -CFT */
#ifdef TC_COLOR
int8u no_color_flag = FALSE; /* for mono monitors -CFT */
#endif
#else
int find_cut = TRUE;
int find_examine = TRUE;
int find_bound = FALSE;
int find_prself = FALSE;
int prompt_carry_flag = FALSE;
int show_weight_flag = FALSE;
int highlight_seams = FALSE;
int find_ignore_doors = FALSE;
#endif

#ifdef MSDOS
char doing_inven = FALSE;	/* Track inventory commands. -CJS- */
int8u screen_change = FALSE;	/* Track screen updates for inven_commands. */
#else
char doing_inven = FALSE;	/* Track inventory commands. -CJS- */
int screen_change = FALSE;	/* Track screen updates for inven_commands. */
#endif

char last_command = ' ';  	/* Memory of previous command. */

/* these used to be in dungeon.c */
#ifdef MSDOS
int8u new_level_flag;		/* Next level when true	 */
int8u search_flag = FALSE;	/* Player is searching	 */
int8u teleport_flag;		/* Handle teleport traps  */
int8u player_light;		/* Player carrying light */
int8u eof_flag = FALSE;		/* Used to signal EOF/HANGUP condition */
int8u light_flag = FALSE;		/* Track if temporary light about player.  */

int8u wait_for_more = FALSE;	/* used when ^C hit during -more- prompt */
int8u closing_flag = FALSE;	/* Used for closing   */
#else
int new_level_flag;		/* Next level when true	 */
int search_flag = FALSE;	/* Player is searching	 */
int teleport_flag;		/* Handle teleport traps  */
int player_light;		/* Player carrying light */
int eof_flag = FALSE;		/* Used to signal EOF/HANGUP condition */
int light_flag = FALSE;		/* Track if temporary light about player.  */

int wait_for_more = FALSE;	/* used when ^C hit during -more- prompt */
int closing_flag = FALSE;	/* Used for closing   */
#endif

/*  Following are calculated from max dungeon sizes		*/
int16 max_panel_rows,max_panel_cols;
int panel_row,panel_col;
int panel_row_min,panel_row_max;
int panel_col_min,panel_col_max;
int panel_col_prt,panel_row_prt;

#ifdef MAC
recall_type *c_recall;
#else
recall_type c_recall[MAX_CREATURES];	/* Monster memories */
#endif

