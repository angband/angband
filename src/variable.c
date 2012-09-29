/* File: variable.c */

/* Purpose: Global variables */

#include "angband.h"


/* Link a copyright message into the executable */
cptr copyright[5] = {
    "Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
    "",
    "This software may be copied and distributed for educational, research,",
    "and not for profit purposes provided that this copyright and statement",
    "are included in all such copies."
};


/* Link the "version" into the executable */
int cur_version_maj = CUR_VERSION_MAJ;
int cur_version_min = CUR_VERSION_MIN;
int cur_patch_level = CUR_PATCH_LEVEL;


int hack_m_idx = (-1);		/* XXX Current monster in "process_monsters()" */


int weapon_heavy = FALSE;
int pack_heavy = FALSE;

int cumber_armor = FALSE;
int cumber_glove = FALSE;

int cur_lite = 0;		/* Current light radius (zero for none) */
int old_lite = 1;		/* Previous light radius (assume lite) */

int16 total_winner = FALSE;

int character_generated = 0;	/* don't save score until char gen finished */
int character_saved = 0;	/* prevents save on kill after save_player() */


int LOAD = 0;			/* Hack -- used for CHECK_LOAD */


int32u randes_seed;		/* Hack -- consistent object colors */
int32u town_seed;		/* Hack -- consistent town layout */

int command_cmd = 0;		/* Current "Angband Command" */
int command_old = 0;		/* Last command completed */
int command_esc = 0;		/* Later -- was current command aborted? */
int command_arg = 0;		/* Gives argument of current command -BEN- */
int command_rep = 0;		/* Gives repetition of current command -BEN- */
int command_dir = -1;		/* Gives direction of current command -BEN- */

int create_up_stair = FALSE;
int create_down_stair = FALSE;

int death = FALSE;		/* True if player has died */
int free_turn_flag;		/* Command is "free", do not move creatures */
int find_flag;			/* Number of turns spent running */

int msg_flag;			/* Used in msg_print() for "buffering" */

int16 cur_height;		/* Cur dungeon height */
int16 cur_width;		/* Cur dungeon width  */
int16 dun_level = 0;		/* Cur dungeon level   */
int16 object_level = 0;		/* level for objects to be created -CWS  */
int32u turn = 0;		/* Cur turn of game    */
int32u old_turn = 0;		/* Last feeling message */
int wizard = FALSE;		/* Is the player currently in Wizard mode? */
int to_be_wizard = FALSE;	/* Is the player about to be a wizard? */
int can_be_wizard = FALSE;	/* Does the player have wizard permissions? */
int16 panic_save = FALSE;	/* this is true if playing from a panic save */
int16 noscore = FALSE;		/* Don't log the game. -CJS- */

int in_store_flag = FALSE;	/* Don't redisplay light in stores -DGK */

int coin_type = 0;		/* remember Creeping _xxx_ coin type -CWS */
int opening_chest = 0;          /* don't generate another chest -CWS */


/* Inventory info */
int16 inven_ctr = 0;		/* Total different obj's	*/
int16 inven_weight = 0;		/* Cur carried weight	*/
int16 equip_ctr = 0;		/* Cur equipment ctr	*/


/* Basic savefile information */
int32u sf_xtra = 0L;		/* Operating system info */
int32u sf_when = 0L;		/* Time when savefile created */
int16u sf_lives = 0L;		/* Number of "lives" with this file */
int16u sf_saves = 0L;		/* Number of "saves" during this life */

int16 i_max;			/* Treasure heap size */
int16 m_max;			/* Monster heap size */

int16 mon_tot_mult;		/* Hack -- limit reproduction */



/* OPTION: options set via the '=' command (in order) */
/* note that the values set here will be the default settings */
/* the default keyset can thus be chosen via "rogue_like_commands" */

int rogue_like_commands = FALSE;	/* Pick initial keyset */
int prompt_carry_flag = FALSE;		/* Require "g" key to pick up */
int carry_query_flag = FALSE;		/* Prompt for pickup */
int always_throw = FALSE;		/* Do not prompt for throw */
int always_repeat = FALSE;		/* Always repeat commands */
int quick_messages = TRUE;		/* quick messages -CWS */

int use_color = TRUE;		/* Use color if possible */
int notice_seams = TRUE;	/* Highlight mineral seams */
int ring_bell = TRUE;		/* Ring the bell */
int equippy_chars = TRUE;	/* do equipment characters -CWS */
int new_screen_layout = TRUE;	/* Use the new screen layout */
int depth_in_feet = TRUE;	/* Display the depth in "feet" */
int hilite_player = TRUE;	/* Hilite the player */

int show_inven_weight = TRUE;	/* Show weights in inven */
int show_equip_weight = TRUE;	/* Show weights in equip */
int show_store_weight = TRUE;	/* Show weights in store */
int plain_descriptions = TRUE;	/* Plain descriptions */

int use_recall_win = TRUE;	/* Use the "recall window" */
int use_choice_win = TRUE;	/* Use the "choice window" */

int no_haggle_flag = FALSE;	/* Cancel haggling -CWS */
int shuffle_owners = FALSE;	/* Let store owners shuffle */

int view_pre_compute = FALSE;	/* Precompute the "view" */
int view_reduce_view = TRUE;	/* Reduce "view" radius if running */
int view_reduce_lite = TRUE;	/* Reduce torch lite if running */

int view_yellow_lite = FALSE;	/* Use "yellow" for "torch lite" */
int view_bright_lite = FALSE;	/* Use "bright" for (viewable) "perma-lite" */
int view_yellow_fast = FALSE;	/* Ignore "yellow_lite" when running */
int view_bright_fast = FALSE;	/* Ignore "bright_lite" when running */

int view_perma_grids = TRUE;	/* Map "remembers" perma-lit grids */
int view_torch_grids = FALSE;	/* Map "remembers" torch-lit grids */

int compress_savefile = TRUE;	/* Compress the savefile as possible */


int find_cut = TRUE;		/* Cut corners */
int find_examine = TRUE;	/* Examine corners */
int find_prself = TRUE;		/* Print self */
int find_bound = TRUE;		/* Stop on borders */
int find_ignore_doors = TRUE;	/* Run through doors */
int find_ignore_stairs = TRUE;	/* Run past stairs */

int disturb_near = TRUE;	/* Disturbed by "local" motion */
int disturb_move = TRUE;	/* Disturbed by monster movement */
int disturb_enter = TRUE;	/* Disturbed by monster appearing */
int disturb_leave = TRUE;	/* Disturbed by monster disappearing */

int hitpoint_warn = 1;		/* Warn at 10% of hit points */
int delay_spd = 1;		/* Delay speed of one */


int peek = FALSE;		/* Let user "see" internal stuff */

int feeling = 0;		/* Most recent feeling */
int rating = 0;			/* Level's current rating */
int good_item_flag = FALSE;	/* True if "Artifact" on this level */

char doing_inven = 0;		/* Hack -- track inventory commands */
int screen_change = FALSE;	/* Hack -- disturb inventory commands */

int new_level_flag;		/* Start a new level */
int teleport_flag;		/* Teleport the player */
int eof_flag = FALSE;		/* Hack -- catch some signals */

int wait_for_more = FALSE;	/* Hack -- react to signals in "more" */
int closing_flag = FALSE;	/* Dungeon is closing */

/*  Following are calculated from max dungeon sizes		*/
int16 max_panel_rows, max_panel_cols;
int panel_row, panel_col;
int panel_row_min, panel_row_max;
int panel_col_min, panel_col_max;
int panel_col_prt, panel_row_prt;

/* Player location in dungeon */
int16 char_row;
int16 char_col;

#ifdef TARGET
/* Targetting information, this code stolen from Morgul -CFT */
int target_mode = FALSE;
int target_col;
int target_row;
int target_mon;
#endif



/*
 * Hack -- allow consistant creation of "WINNER" artifacts
 */
bool permit_grond = TRUE;
bool permit_morgoth = TRUE;



/* Current player's user id */
int player_uid = 0;

/* Current player's character name */
char player_name[32];

/* What killed the player */
vtype died_from;

/* Hack -- Textual "history" for the Player */
char history[4][60];

/* Buffer to hold the current savefile name */
char savefile[1024];

/* Was: cave_type cave[MAX_HEIGHT][MAX_WIDTH]; */
cave_type *cave[MAX_HEIGHT];


/* Buffer to hold the name of the ghost */
char ghost_name[128];

/* The player's inventory */
inven_type inventory[INVEN_ARRAY_SIZE];



/* Was: monster_type m_list[MAX_M_IDX] */
monster_type *m_list;

/* The array of monster races (see arrays.c) */
/* Was: monster_race r_list[MAX_R_IDX]; */
monster_race *r_list;

/* Was: monster_lore l_list[MAX_R_IDX]; */
monster_lore *l_list;

/* Hack -- Quest array */
quest q_list[QUEST_MAX];


/* Was: inven_type i_list[MAX_I_IDX]; */
inven_type *i_list;

/* The array of object types (see arays.c) */
/* Was: inven_kind k_list[MAX_K_IDX]; */
inven_kind *k_list;

/* Extra item "memory" */ 
inven_xtra *x_list;



static player_type p_body;	/* Static player info record */

player_type *p_ptr = &p_body;	/* Pointer to the player info */

int32u spell_learned = 0;       /* bit mask of spells learned */
int32u spell_learned2 = 0;      /* bit mask of spells learned */
int32u spell_worked = 0;        /* bit mask of spells tried and worked */
int32u spell_worked2 = 0;       /* bit mask of spells tried and worked */
int32u spell_forgotten = 0;     /* bit mask of spells learned but forgotten */
int32u spell_forgotten2 = 0;    /* bit mask of spells learned but forgotten */
int8u spell_order[64];          /* order spells learned/remembered/forgotten */

/*
 * Calculated base hp values for player at each level,
 * store them so that drain life + restore life does not
 * affect hit points.  Also prevents shameless use of backup
 * savefiles for hitpoint acquirement.
 */
int16u player_hp[MAX_PLAYER_LEVEL];

