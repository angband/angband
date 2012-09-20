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


int cur_lite = 0;		/* Current light radius (zero for none) */
int old_lite = 1;		/* Previous light radius (assume lite) */

s16b total_winner = FALSE;	/* Semi-Hack -- Game has been won */

int character_generated = 0;	/* A character has been generated */
int character_saved = 0;	/* The character has been saved */


int LOAD = 0;			/* Hack -- used for CHECK_LOAD */


u32b randes_seed;		/* Hack -- consistent object colors */
u32b town_seed;		/* Hack -- consistent town layout */

int command_cmd = 0;		/* Current "Angband Command" */
int command_old = 0;		/* Last "Angband Command" completed */
int command_esc = 0;		/* Later -- was current command aborted? */
int command_arg = 0;		/* Gives argument of current command */
int command_rep = 0;		/* Gives repetition of current command */
int command_dir = -1;		/* Gives direction of current command */

int command_wrk = 0;		/* See "moria1.c" */
int command_see = 0;		/* See "moria1.c" */
int command_xxx = 0;		/* See "moria1.c" */
int command_new = 0;		/* Next command to execute */
int command_gap = 0;		/* First column to use for inven/equip list */

int create_up_stair = FALSE;
int create_down_stair = FALSE;

int death = FALSE;		/* True if player has died */
int free_turn_flag;		/* Command is "free", do not move creatures */
int find_flag;			/* Number of turns spent running */

int msg_flag;			/* Used in msg_print() for "buffering" */

s16b cur_height;		/* Cur dungeon height */
s16b cur_width;			/* Cur dungeon width */
s16b dun_level = 0;		/* Cur dungeon level */
s16b object_level = 0;		/* Cur object creation level */

u32b turn = 0;			/* Cur turn of game    */
u32b old_turn = 0;		/* Last feeling message */

int wizard = FALSE;		/* Is the player currently in Wizard mode? */
int to_be_wizard = FALSE;	/* Is the player about to be a wizard? */
int can_be_wizard = FALSE;	/* Does the player have wizard permissions? */

s16b panic_save = FALSE;	/* this is true if playing from a panic save */
s16b noscore = FALSE;		/* Don't log the game. -CJS- */

int in_store_flag = FALSE;	/* Don't redisplay light in stores -DGK */
int inkey_flag = FALSE;		/* Do a special "inkey()" command */

int coin_type = 0;		/* Hack -- force coin type */
int opening_chest = 0;          /* Hack -- prevent chest generation */


/* Inventory info */
s16b inven_ctr = 0;		/* Total different obj's	*/
s16b inven_weight = 0;		/* Cur carried weight	*/
s16b equip_ctr = 0;		/* Cur equipment ctr	*/


/* Basic savefile information */
u32b sf_xtra = 0L;		/* Operating system info */
u32b sf_when = 0L;		/* Time when savefile created */
u16b sf_lives = 0L;		/* Number of "lives" with this file */
u16b sf_saves = 0L;		/* Number of "saves" during this life */

s16b i_max;			/* Treasure heap size */
s16b m_max;			/* Monster heap size */


/* OPTION: options set via the '=' command */
/* note that the values set here will be the default settings */
/* the default keyset can thus be chosen via "rogue_like_commands" */

/* Option set 1 */

int rogue_like_commands = FALSE;	/* Pick initial keyset */
int quick_messages = FALSE;		/* Quick messages -CWS */
int prompt_carry_flag = FALSE;		/* Require "g" key to pick up */
int carry_query_flag = FALSE;		/* Prompt for pickup */
int always_pickup = FALSE;		/* Warn about over-flow */
int always_throw = FALSE;		/* Do not prompt for throw */
int always_repeat = FALSE;		/* Always repeat commands */
int use_old_target = FALSE;		/* Use old target somehow */

int new_screen_layout = TRUE;	/* Use the new screen layout */
int equippy_chars = TRUE;	/* do equipment characters -CWS */
int depth_in_feet = TRUE;	/* Display the depth in "feet" */
int notice_seams = TRUE;	/* Highlight mineral seams */

int use_color = TRUE;		/* Use color if possible */
int use_recall_win = TRUE;	/* Use the "recall window" */
int use_choice_win = TRUE;	/* Use the "choice window" */

int compress_savefile = TRUE;	/* Compress the savefile as possible */

int hilite_player = TRUE;	/* Hilite the player */

int ring_bell = TRUE;		/* Ring the bell */


/* Option Set 2 -- Disturbance */

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

int flush_disturb = TRUE;	/* Flush input on disturbance */
int flush_failure = TRUE;	/* Flush input on any failure */
int flush_command = FALSE;	/* Flush input before every command */
int flush_unused = FALSE;	/* Flush input on some condition */

int view_yellow_lite = TRUE;	/* Use "yellow" for "torch lite" */
int view_bright_lite = TRUE;	/* Use "bright" for (viewable) "perma-lite" */
int view_yellow_fast = FALSE;	/* Ignore "yellow_lite" when running */
int view_bright_fast = FALSE;	/* Ignore "bright_lite" when running */



/* Option set 3 -- Gameplay */

int view_pre_compute = TRUE;	/* Precompute the "view" */
int view_xxx_compute = TRUE;	/* XXX Unused */

int view_reduce_view = FALSE;	/* Reduce "view" radius if running */
int view_reduce_lite = FALSE;	/* Reduce torch lite if running */

int view_wall_memory = TRUE;	/* Map "remembers" walls */
int view_xtra_memory = TRUE;	/* Map "remembers" extra stuff */
int view_perma_grids = TRUE;	/* Map "remembers" perma-lit grids */
int view_torch_grids = FALSE;	/* Map "remembers" torch-lit grids */

int flow_by_sound = FALSE;	/* Monsters track new player location */
int flow_by_smell = FALSE;	/* Monsters track old player location */

int no_haggle_flag = FALSE;	/* Cancel haggling */
int shuffle_owners = FALSE;	/* Shuffle store owners occasionally */

int show_inven_weight = TRUE;	/* Show weights in inven */
int show_equip_weight = TRUE;	/* Show weights in equip */
int show_store_weight = TRUE;	/* Show weights in store */
int plain_descriptions = TRUE;	/* Plain descriptions */

int stack_allow_items = TRUE;	/* Allow weapons and armor and such to stack */
int stack_allow_wands = TRUE;	/* Allow wands and staffs and rods to stack */
int stack_force_notes = FALSE;	/* Force items with different notes to stack */
int stack_force_costs = FALSE;	/* Force items with different costs to stack */





int hitpoint_warn = 1;		/* Hitpoint warning (0 to 9) */
int delay_spd = 1;		/* Delay factor (0 to 9) */

term *term_screen = NULL;	/* The main screen */
term *term_recall = NULL;	/* The recall window */
term *term_choice = NULL;	/* The choice window */


int peek = FALSE;		/* Let user "see" internal stuff */

int feeling = 0;		/* Most recent feeling */
int rating = 0;			/* Level's current rating */
int good_item_flag = FALSE;	/* True if "Artifact" on this level */

char doing_inven = 0;		/* Hack -- track inventory commands */
int screen_change = FALSE;	/* Hack -- disturb inventory commands */

int new_level_flag;		/* Start a new level */
int teleport_flag;		/* Hack -- handle teleport traps */

int closing_flag = FALSE;	/* Dungeon is closing */

/*  Following are calculated from max dungeon sizes		*/
s16b max_panel_rows, max_panel_cols;
int panel_row, panel_col;
int panel_row_min, panel_row_max;
int panel_col_min, panel_col_max;
int panel_col_prt, panel_row_prt;

/* Player location in dungeon */
s16b char_row;
s16b char_col;

#ifdef TARGET
/* Targetting information, this code stolen from Morgul -CFT */
int target_mode = FALSE;
int target_col;
int target_row;
int target_mon;
#endif




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

/* The player's inventory (24 pack items, 12 equipment items) */
inven_type inventory[INVEN_TOTAL];



/* The array of dungeon monsters [MAX_M_IDX] */
monster_type *m_list;

/* The array of monster races [MAX_R_IDX] */
monster_race *r_list;

/* The arrays of monster attr/char codes [MAX_R_IDX] */
byte *r_attr;
char *r_char;

/* The array of monster "memory" [MAX_R_IDX] */
monster_lore *l_list;

/* Hack -- Quest array */
quest q_list[QUEST_MAX];


/* The array of dungeon items [MAX_I_IDX] */
inven_type *i_list;

/* The array of object types [MAX_K_IDX] */
inven_kind *k_list;

/* The array of "artifact" information [MAX_V_IDX] */
inven_very *v_list;

/* The arrays of object attr/char codes [MAX_K_IDX] */
byte *k_attr;
char *k_char;

/* Extra item "memory" [MAX_K_IDX] */ 
inven_xtra *x_list;


/* Hack -- Attribute table for inventory */
byte tval_to_attr[128];


/*
 * Hack -- simple keymap method, see "arrays.c" and "commands.c".
 */
byte keymap_cmds[128];
byte keymap_dirs[128];



static player_type p_body;	/* Static player info record */
player_type *p_ptr = &p_body;	/* Pointer to the player info */

u32b spell_learned = 0;       /* bit mask of spells learned */
u32b spell_learned2 = 0;      /* bit mask of spells learned */
u32b spell_worked = 0;        /* bit mask of spells tried and worked */
u32b spell_worked2 = 0;       /* bit mask of spells tried and worked */
u32b spell_forgotten = 0;     /* bit mask of spells learned but forgotten */
u32b spell_forgotten2 = 0;    /* bit mask of spells learned but forgotten */
byte spell_order[64];          /* order spells learned/remembered/forgotten */

/*
 * Calculated base hp values for player at each level,
 * store them so that drain life + restore life does not
 * affect hit points.  Also prevents shameless use of backup
 * savefiles for hitpoint acquirement.
 */
u16b player_hp[MAX_PLAYER_LEVEL];

