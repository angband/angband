/* File: variable.c */

/* Purpose: Global variables */

#include "angband.h"


/*
 * Link a copyright message into the executable
 */
cptr copyright[5] = {
    "Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
    "",
    "This software may be copied and distributed for educational, research,",
    "and not for profit purposes provided that this copyright and statement",
    "are included in all such copies."
};


/*
 * Link the "version" into the executable
 */
int cur_version_maj = CUR_VERSION_MAJ;
int cur_version_min = CUR_VERSION_MIN;
int cur_patch_level = CUR_PATCH_LEVEL;


int cur_lite = 0;		/* Current light radius (zero for none) */
int old_lite = 0;		/* Previous light radius (assume none) */

u16b total_winner = 0;		/* Semi-Hack -- Game has been won */

int character_generated = 0;	/* A character has been generated or loaded */
int character_loaded = 0;	/* The character came from a savefile */
int character_saved = 0;	/* The character has been saved */


u32b randes_seed;		/* Hack -- consistent object colors */
u32b town_seed;			/* Hack -- consistent town layout */

int command_cmd = 0;		/* Current "Angband Command" */
int command_old = 0;		/* Last "Angband Command" completed */
int command_esc = 0;		/* Later -- was current command aborted? */
int command_arg = 0;		/* Gives argument of current command */
int command_rep = 0;		/* Gives repetition of current command */
int command_dir = -1;		/* Gives direction of current command */

int command_wrk = 0;		/* Unused */
int command_see = 0;		/* See "cmd1.c" */
int command_xxx = 0;		/* See "cmd1.c" */
int command_new = 0;		/* Next command to execute */
int command_gap = 0;		/* First column to use for inven/equip list */

int energy_use = 0;		/* Energy use this turn */

int choice_default = 0;		/* Default contents of choice window */

int create_up_stair = FALSE;
int create_down_stair = FALSE;

int death = FALSE;		/* True if player has died */

int find_flag;			/* Number of turns spent running */

int msg_flag;			/* Used in msg_print() for "buffering" */

s16b cur_hgt;			/* Current dungeon height */
s16b cur_wid;			/* Current dungeon width */
s16b dun_level = 0;		/* Current dungeon level */
s16b object_level = 0;		/* Current object creation level */
s16b monster_level = 0;		/* Current monster creation level */

u32b turn = 0;			/* Current game turn */
u32b old_turn = 0;		/* Turn when level began (feelings) */

bool wizard = FALSE;		/* Is the player currently in Wizard mode? */
bool to_be_wizard = FALSE;	/* Is the player about to be a wizard? */
bool can_be_wizard = FALSE;	/* Does the player have wizard permissions? */

u16b panic_save = 0;		/* Track some special "conditions" */
u16b noscore = 0;		/* Track various "cheating" conditions */

bool in_store_flag = FALSE;	/* Don't redisplay light in stores -DGK */
bool inkey_flag = FALSE;	/* Do a special "inkey()" command */

int coin_type = 0;		/* Hack -- force coin type */
int opening_chest = 0;          /* Hack -- prevent chest generation */


/* Inventory info */
s16b inven_ctr = 0;		/* Total different obj's	*/
s16b inven_weight = 0;		/* Cur carried weight	*/
s16b equip_ctr = 0;		/* Cur equipment ctr	*/


/* Basic savefile information */
u32b sf_xtra = 0L;		/* Operating system info */
u32b sf_when = 0L;		/* Time when savefile created */
u16b sf_lives = 0L;		/* Number of past "lives" with this file */
u16b sf_saves = 0L;		/* Number of "saves" during this life */

s16b i_max;			/* Treasure heap size */
s16b m_max;			/* Monster heap size */


/* OPTION: software options (set via the '=' command) */
/* note that the values set here will be the default settings */
/* the default keyset can thus be chosen via "rogue_like_commands" */
/* note that pre-2.7.6 savefiles get these values as well */


/* General options */

int rogue_like_commands = FALSE;	/* Use the rogue-like keyset */
int quick_messages = FALSE;		/* Clear "-more-" with any key */
int other_query_flag = FALSE;		/* Prompt before various actions */
int carry_query_flag = FALSE;		/* Prompt when picking up things */
int always_pickup = TRUE;		/* Pick things up by default */
int always_throw = TRUE;		/* Throw things without asking */
int always_repeat = TRUE;		/* Auto-repeat some commands */
int use_old_target = TRUE;		/* Use old target when possible */

int new_screen_layout = TRUE;	/* Use the new screen layout */
int equippy_chars = TRUE;	/* Show equipment characters -CWS */
int depth_in_feet = TRUE;	/* Display the depth in "feet" */
int notice_seams = TRUE;	/* Highlight mineral seams */

int use_color = TRUE;		/* Use color if possible */

int compress_savefile = TRUE;	/* Compress the savefile as possible */

int hilite_player = TRUE;	/* Hilite the player */

int ring_bell = TRUE;		/* Ring the bell */

int view_yellow_lite = TRUE;	/* Use "yellow" for "torch lite" */
int view_bright_lite = TRUE;	/* Use "bright" for (viewable) "perma-lite" */


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

int flush_command = FALSE;	/* Flush input before every command */
int flush_disturb = TRUE;	/* Flush input on disturbance */
int flush_failure = TRUE;	/* Flush input on any failure */

int fresh_before = TRUE;	/* Flush output before normal commands */
int fresh_after = TRUE;		/* Flush output after normal commands */
int fresh_find = TRUE;		/* Flush output while running */


/* Gameplay options */

int dungeon_align = TRUE;	/* Align rooms to dungeon panels */
int dungeon_other = TRUE;	/* Do something when making levels */

int view_reduce_view = FALSE;	/* Reduce "view" radius if running */
int view_reduce_lite = FALSE;	/* Reduce torch lite if running */

int view_wall_memory = TRUE;	/* Map "remembers" walls */
int view_xtra_memory = TRUE;	/* Map "remembers" extra stuff */
int view_perma_grids = TRUE;	/* Map "remembers" perma-lit grids */
int view_torch_grids = FALSE;	/* Map "remembers" torch-lit grids */

int flow_by_sound = FALSE;	/* Monsters track new player location */
int flow_by_smell = FALSE;	/* Monsters track old player location */

int track_follow = FALSE;	/* Monsters follow the player */
int track_target = FALSE;	/* Monsters target the player */

int smart_learn = FALSE;	/* Monsters learn from their mistakes */
int smart_cheat = FALSE;	/* Monsters exploit player weaknesses */

int no_haggle_flag = FALSE;	/* Cancel haggling */
int shuffle_owners = FALSE;	/* Shuffle store owners occasionally */

int show_spell_info = FALSE;	/* Show extra spell info */
int show_health_bar = FALSE;	/* Show monster health bar */

int show_inven_weight = TRUE;	/* Show weights in inven */
int show_equip_weight = TRUE;	/* Show weights in equip */
int show_store_weight = TRUE;	/* Show weights in store */
int plain_descriptions = TRUE;	/* Plain descriptions */

int stack_allow_items = TRUE;	/* Allow weapons and armor and such to stack */
int stack_allow_wands = TRUE;	/* Allow wands and staffs and rods to stack */
int stack_force_notes = FALSE;	/* Force items with different notes to stack */
int stack_force_costs = FALSE;	/* Force items with different costs to stack */

int begin_maximize = FALSE;	/* Begin in "maximize" mode */
int begin_preserve = FALSE;	/* Begin in "preserve" mode */


/* Special options */

int use_screen_win = TRUE;	/* Use the "screen window" */
int use_recall_win = TRUE;	/* Use the "recall window" */
int use_choice_win = TRUE;	/* Use the "choice window" */

int recall_show_desc = TRUE;	/* Show monster descriptions in recall */
int recall_show_kill = TRUE;	/* Show monster kill info in recall */

int choice_inven_wgt = FALSE;	/* Show choice window inven weights */
int choice_equip_wgt = FALSE;	/* Show choice window equip weights */

int choice_inven_xtra = FALSE;	/* Show choice window inven extra info */
int choice_equip_xtra = FALSE;	/* Show choice window equip extra info */


/* Cheating options */

int cheat_peek = FALSE;		/* Cheat -- note object creation */
int cheat_hear = FALSE;		/* Cheat -- note monster creation */
int cheat_room = FALSE;		/* Cheat -- note dungeon creation */
int cheat_xtra = FALSE;		/* Cheat -- note something else */
int cheat_know = FALSE;		/* Cheat -- complete monster recall */
int cheat_live = FALSE;		/* Cheat -- allow death avoidance */


/* Misc options */

int hitpoint_warn = 1;		/* Hitpoint warning (0 to 9) */

int delay_spd = 1;		/* Delay factor (0 to 9) */



term *term_screen = NULL;	/* The main screen */
term *term_recall = NULL;	/* The recall window */
term *term_choice = NULL;	/* The choice window */


int feeling = 0;		/* Most recent feeling */
int rating = 0;			/* Level's current rating */
int good_item_flag = FALSE;	/* True if "Artifact" on this level */

char doing_inven = 0;		/* Hack -- track inventory commands */
int screen_change = FALSE;	/* Hack -- disturb inventory commands */

int new_level_flag;		/* Start a new level */

int teleport_flag;		/* Teleport required */
int teleport_dist;		/* Teleport distance */
int teleport_to_y;		/* Teleport location (Y) */
int teleport_to_x;		/* Teleport location (X) */

int closing_flag = FALSE;	/* Dungeon is closing */

/*  Following are calculated from max dungeon sizes		*/
s16b max_panel_rows, max_panel_cols;
int panel_row, panel_col;
int panel_row_min, panel_row_max;
int panel_col_min, panel_col_max;
int panel_col_prt, panel_row_prt;

/* Player location in dungeon */
s16b py;			/* Player location (Y) */
s16b px;			/* Player location (X) */

/* Targetting variables */
int target_who = 0;
int target_col = 0;
int target_row = 0;

/* Health bar variable -DRS- */
int health_who = 0;



/*
 * The player's UID and GID
 */
int player_uid = 0;
int player_euid = 0;
int player_egid = 0;

/* Current player's character name */
char player_name[32];

/* What killed the player */
char died_from[80];

/* Hack -- Textual "history" for the Player */
char history[4][60];

/* Buffer to hold the current savefile name */
char savefile[1024];

/* Was: cave_type cave[MAX_HGT][MAX_WID]; */
cave_type *cave[MAX_HGT];


/* Buffer to hold the name of the ghost */
char ghost_name[128];

/* The player's inventory (24 pack items, 12 equipment items) */
inven_type inventory[INVEN_TOTAL];



/* The array of dungeon monsters [MAX_M_IDX] */
monster_type *m_list;

/* The array of monster races [MAX_R_IDX] */
monster_race *r_list;

/* The array of monster "memory" [MAX_R_IDX] */
monster_lore *l_list;

/* Hack -- Quest array */
quest q_list[MAX_Q_IDX];


/* The array of dungeon items [MAX_I_IDX] */
inven_type *i_list;

/* The array of object types [MAX_K_IDX] */
inven_kind *k_list;

/* The array of "artifact" information [MAX_V_IDX] */
inven_very *v_list;

/* Extra item "memory" [MAX_K_IDX] */
inven_xtra *x_list;


/*
 * Default attr/char values by tval
 */
byte tval_to_attr[128];
char tval_to_char[128];


/*
 * Hack -- simple keymap method, see "arrays.c" and "commands.c".
 */
byte keymap_cmds[256];
byte keymap_dirs[256];



static player_type p_body;	/* Static player info record */
player_type *p_ptr = &p_body;	/* Pointer to the player info */

player_class *cp_ptr = NULL;	/* Pointer to the player class info */
player_race *rp_ptr = NULL;	/* Pointer to the player race info */


/*
 * More spell info
 */
u32b spell_learned1 = 0;       /* bit mask of spells learned */
u32b spell_learned2 = 0;      /* bit mask of spells learned */
u32b spell_worked1 = 0;        /* bit mask of spells tried and worked */
u32b spell_worked2 = 0;       /* bit mask of spells tried and worked */
u32b spell_forgotten1 = 0;     /* bit mask of spells learned but forgotten */
u32b spell_forgotten2 = 0;    /* bit mask of spells learned but forgotten */
byte spell_order[64];          /* order spells learned/remembered/forgotten */


/*
 * Calculated base hp values for player at each level,
 * store them so that drain life + restore life does not
 * affect hit points.  Also prevents shameless use of backup
 * savefiles for hitpoint acquirement.
 */
s16b player_hp[MAX_PLAYER_LEVEL];


/*
 * Global array for looping through the legal "keypad directions"
 */
int ddd[9] =  {2, 8, 6, 4, 3, 1, 9, 7, 5};

/*
 * Global arrays for converting "keypad direction" into offsets
 */
int ddx[10] = {0, -1, 0, 1, -1, 0, 1, -1, 0, 1};
int ddy[10] = {0, 1, 1, 1, 0, 0, 0, -1, -1, -1};


/*
 * The ANGBAND_xxx filepath "constants".
 */

cptr ANGBAND_SYS = "xxx";		/* System Suffix */

cptr ANGBAND_DIR = NULL;		/* Dir: main directory */

cptr ANGBAND_DIR_FILE = NULL;		/* Dir: ascii (misc) files */
cptr ANGBAND_DIR_HELP = NULL;		/* Dir: ascii (help) files */
cptr ANGBAND_DIR_BONE = NULL;		/* Dir: ascii (bones) files */
cptr ANGBAND_DIR_DATA = NULL;		/* Dir: binary (misc) files */
cptr ANGBAND_DIR_SAVE = NULL;		/* Dir: binary (save) files */
cptr ANGBAND_DIR_PREF = NULL;		/* Dir: ascii (pref) files */

cptr ANGBAND_NEWS = NULL;		/* Hack -- Main News File */


