/* File: variable.c */

/* Purpose: Global variables */

#include "angband.h"


/*
 * Hack -- Link a copyright message into the executable
 */
cptr copyright[5] = {
    "Copyright (c) 1989 James E. Wilson, Robert A. Keoneke",
    "",
    "This software may be copied and distributed for educational, research,",
    "and not for profit purposes provided that this copyright and statement",
    "are included in all such copies."
};


/*
 * Hack -- Link the "version" into the executable
 */
byte version_major = VERSION_MAJOR;
byte version_minor = VERSION_MINOR;
byte version_patch = VERSION_PATCH;
byte version_extra = VERSION_EXTRA;

/*
 * Hack -- Savefile information
 */
u32b sf_xtra;			/* Operating system info */
u32b sf_when;			/* Time when savefile created */
u16b sf_lives;			/* Number of past "lives" with this file */
u16b sf_saves;			/* Number of "saves" during this life */

/*
 * Hack -- Run-time arguments
 */
bool arg_wizard;		/* Command arg -- Enter wizard mode */
bool arg_fiddle;		/* Command arg -- Enter fiddle mode */
bool arg_force_original;	/* Command arg -- Force original keyset */
bool arg_force_roguelike;	/* Command arg -- Force roguelike keyset */

bool character_generated;	/* The character exists */
bool character_dungeon;		/* The character has a dungeon */
bool character_loaded;		/* The character was loaded from the savefile */
bool character_saved;		/* The character was saved to the savefile */

bool character_icky;		/* The game is in an icky full screen mode */
bool character_xtra;		/* The game is in an unused state (unused) */

u32b seed_flavor;		/* Hack -- consistent object colors */
u32b seed_town;			/* Hack -- consistent town layout */

s16b command_cmd;		/* Current "Angband Command" */

s16b command_arg;		/* Gives argument of current command */
s16b command_rep;		/* Gives repetition of current command */
s16b command_dir;		/* Gives direction of current command */

s16b command_see;		/* See "cmd1.c" */
s16b command_wrk;		/* See "cmd1.c" */

s16b command_gap = 50;		/* See "cmd1.c" */

s16b command_new;		/* Command chaining from inven/equip view */

s16b energy_use;		/* Energy use this turn */

s16b choose_default;		/* Default contents of choice window */

bool create_up_stair;		/* Auto-create "up stairs" */
bool create_down_stair;		/* Auto-create "down stairs" */

bool msg_flag;			/* Used in msg_print() for "buffering" */

bool alive;			/* True if game is running */

bool death;			/* True if player has died */

s16b running;			/* Current counter for running, if any */
s16b resting;			/* Current counter for resting, if any */

s16b cur_hgt;			/* Current dungeon height */
s16b cur_wid;			/* Current dungeon width */
s16b dun_level;			/* Current dungeon level */
s16b num_repro;			/* Current reproducer count */
s16b object_level;		/* Current object creation level */
s16b monster_level;		/* Current monster creation level */

s32b turn;			/* Current game turn */
s32b old_turn;			/* Turn when level began (feelings) */

bool wizard;			/* Is the player currently in Wizard mode? */
bool can_be_wizard;		/* Does the player have wizard permissions? */

u16b total_winner;		/* Semi-Hack -- Game has been won */

u16b panic_save;		/* Track some special "conditions" */
u16b noscore;			/* Track various "cheating" conditions */

s16b signal_count = 0;		/* Hack -- Count interupts */

bool inkey_base;		/* See the "inkey()" function */
bool inkey_xtra;		/* See the "inkey()" function */
bool inkey_scan;		/* See the "inkey()" function */
bool inkey_flag;		/* See the "inkey()" function */

s16b coin_type;			/* Hack -- force coin type */
bool opening_chest;		/* Hack -- prevent chest generation */

bool use_graphics;		/* Hack -- Assume no graphics mapping */

bool use_sound;			/* Hack -- Assume no special sounds */

bool scan_monsters;		/* Hack -- optimize multi-hued code, etc */
bool scan_objects;		/* Hack -- optimize multi-hued code, etc */

s16b total_weight;		/* Total weight being carried */

s16b inven_nxt;			/* Hack -- unused */

s16b inven_cnt;			/* Number of items in inventory */
s16b equip_cnt;			/* Number of items in equipment */

s16b i_cnt = 0;			/* Object counter */
s16b m_cnt = 0;			/* Monster counter */

s16b i_nxt = 1;			/* Object free scanner */
s16b m_nxt = 1;			/* Monster free scanner */

s16b i_max = 1;			/* Object heap size */
s16b m_max = 1;			/* Monster heap size */


/* Software options (set via the '=' command).  See "tables.c" */

/* General options */

bool rogue_like_commands;	/* Use the rogue-like keyset */
bool quick_messages;		/* Clear "-more-" with any key */
bool other_query_flag;		/* Prompt before various actions */
bool carry_query_flag;		/* Prompt when picking up things */
bool always_pickup;		/* Pick things up by default */
bool always_throw;		/* Throw things without asking */
bool always_repeat;		/* Auto-repeat some commands */
bool use_old_target;		/* Use old target when possible */

bool show_equip_label;		/* Shop labels in equipment list */
bool equippy_chars;		/* Show equippy characters */
bool depth_in_feet;		/* Display the depth in "feet" */
bool notice_seams;		/* Highlight mineral seams */

bool use_color;			/* Use color if possible */

bool compress_savefile;		/* Compress the savefile as possible */

bool hilite_player;		/* Hilite the player */

bool ring_bell;			/* Ring the bell */

bool view_yellow_lite;		/* Use "yellow" for "torch lite" */
bool view_bright_lite;		/* Use "bright" for (viewable) "perma-lite" */


/* Option Set 2 -- Disturbance */

bool find_ignore_stairs;	/* Run past stairs */
bool find_ignore_doors;		/* Run through doors */
bool find_cut;			/* Cut corners */
bool find_examine;		/* Examine corners */

bool disturb_near;		/* Disturbed by "local" motion */
bool disturb_move;		/* Disturbed by monster movement */
bool disturb_enter;		/* Disturbed by monster appearing */
bool disturb_leave;		/* Disturbed by monster disappearing */

bool disturb_panel;		/* Disturbed by map panel changing */
bool disturb_other;		/* Disturbed by various things happening */

bool flush_command;		/* Flush input before every command */
bool flush_disturb;		/* Flush input on disturbance */
bool flush_failure;		/* Flush input on any failure */

bool fresh_before;		/* Flush output before normal commands */
bool fresh_after;		/* Flush output after normal commands */
bool fresh_find;		/* Flush output while running */

bool filch_message;		/* Flush messages before new messages */
bool filch_disturb;		/* Flush messages before disturbances */

bool alert_hitpoint;		/* Alert user to critical hitpoints */
bool alert_failure;		/* Alert user to various failures */


/* Gameplay options */

bool scum_always;		/* Auto-scum for good levels (always) */
bool scum_sometimes;		/* Auto-scum for good levels (sometimes) */

bool dungeon_align;		/* Generate dungeons with align rooms */
bool dungeon_stair;		/* Generate dungeons with connected stairs */

bool view_perma_grids;		/* Map "remembers" perma-lit grids */
bool view_torch_grids;		/* Map "remembers" torch-lit grids */

bool flow_by_sound;		/* Monsters track new player location */
bool flow_by_smell;		/* Monsters track old player location */

bool track_follow;		/* Monsters follow the player */
bool track_target;		/* Monsters target the player */

bool smart_learn;		/* Monsters learn from their mistakes */
bool smart_cheat;		/* Monsters exploit player weaknesses */

bool no_haggle_flag;		/* Cancel haggling */
bool shuffle_owners;		/* Shuffle store owners occasionally */

bool show_spell_info;		/* Show extra spell info */
bool show_health_bar;		/* Show monster health bar */

bool show_inven_weight;		/* Show weights in inven */
bool show_equip_weight;		/* Show weights in equip */
bool show_store_weight;		/* Show weights in store */
bool plain_descriptions;	/* Plain descriptions */

bool stack_allow_items;		/* Allow weapons and armor and such to stack */
bool stack_allow_wands;		/* Allow wands and staffs and rods to stack */
bool stack_force_notes;		/* Force items with different notes to stack */
bool stack_force_costs;		/* Force items with different costs to stack */

bool auto_combine_pack;		/* Automatically combine items in the pack */
bool auto_reorder_pack;		/* Automatically reorder items in the pack */


/* Efficiency options */

bool view_reduce_lite;		/* Reduce torch lite if running */
bool view_reduce_lite_town;	/* Reduce torch lite if running (in town) */
bool view_reduce_view;		/* Reduce "view" radius if running */
bool view_reduce_view_town;	/* Reduce "view" radius if running (in town) */

bool optimize_running;		/* Optimize various things when running */
bool optimize_resting;		/* Optimize various things when resting */
bool optimize_display;		/* Optimize various things (visual display) */
bool optimize_various;		/* Optimize various things (message recall) */


/* Special options */

bool use_mirror_recent;		/* Use "mirror" window -- recent monsters */

bool use_mirror_normal;		/* Use "mirror" window -- current stuff */
bool use_mirror_choose;		/* Use "mirror" window -- show "choices" */
bool use_mirror_spells;		/* Use "mirror" window -- show "spells" */

bool use_recall_recent;		/* Use "recall" window -- recent monsters */

bool use_choice_normal;		/* Use "choice" window -- current stuff */
bool use_choice_choose;		/* Use "choice" window -- show "choices" */
bool use_choice_spells;		/* Use "choice" window -- show "spells" */

bool show_choose_info;		/* Show info in windows when "choosing" */
bool show_choose_prompt;	/* Show prompt in windows when "choosing" */
bool show_choose_weight;	/* Show weights in windows when "choosing" */
bool show_choose_label;		/* Show labels in windows when "choosing"  */

bool recall_show_desc;		/* Show monster descriptions when "recalling" */
bool recall_show_kill;		/* Show monster kill info when "recalling" */


/* Cheating options */

bool cheat_peek;		/* Cheat -- note object creation */
bool cheat_hear;		/* Cheat -- note monster creation */
bool cheat_room;		/* Cheat -- note dungeon creation */
bool cheat_xtra;		/* Cheat -- note something else */
bool cheat_know;		/* Cheat -- complete monster recall */
bool cheat_live;		/* Cheat -- allow death avoidance */


s16b hitpoint_warn;		/* Hitpoint warning (0 to 9) */

s16b delay_spd;			/* Delay factor (0 to 9) */


term *term_screen;		/* The screen window */
term *term_mirror;		/* The mirror window */
term *term_recall;		/* The recall window */
term *term_choice;		/* The choice window */


s16b feeling;			/* Most recent feeling */
s16b rating;			/* Level's current rating */

bool good_item_flag;		/* True if "Artifact" on this level */

bool new_level_flag;		/* Start a new level */

bool teleport_flag;		/* Teleport required */
s16b teleport_dist;		/* Teleport distance */
s16b teleport_to_y;		/* Teleport location (Y) */
s16b teleport_to_x;		/* Teleport location (X) */

bool closing_flag;		/* Dungeon is closing */


/*
 * Dungeon size info
 */
s16b max_panel_rows, max_panel_cols;
s16b panel_row, panel_col;
s16b panel_row_min, panel_row_max;
s16b panel_col_min, panel_col_max;
s16b panel_col_prt, panel_row_prt;

/* Player location in dungeon */
s16b py;
s16b px;

/* Targetting variables */
s16b target_who;
s16b target_col;
s16b target_row;

/* Health bar variable -DRS- */
s16b health_who;



/*
 * The player's UID and GID
 */
int player_uid = 0;
int player_euid = 0;
int player_egid = 0;

/* Current player's character name */
char player_name[32];

/* Stripped version of "player_name" */
char player_base[32];

/* What killed the player */
char died_from[80];

/* Hack -- Textual "history" for the Player */
char history[4][60];

/* Buffer to hold the current savefile name */
char savefile[1024];


/* Was: cave_type cave[MAX_HGT][MAX_WID]; */
cave_type *cave[MAX_HGT];


/* Array of grids lit by player lite (see "cave.c") [LITE_MAX] */
s16b lite_n;
byte *lite_y;
byte *lite_x;

/* Array of grids viewable to the player (see "cave.c") [VIEW_MAX] */
s16b view_n;
byte *view_y;
byte *view_x;

/* Array of grids for use by various functions (see "cave.c") [TEMP_MAX] */
s16b temp_n;
byte *temp_y;
byte *temp_x;


/*
 * Number of active macros.
 */
s16b macro__num;

/*
 * Array of macro patterns [MACRO_MAX]
 */
cptr *macro__pat;

/*
 * Array of macro actions [MACRO_MAX]
 */
cptr *macro__act;

/*
 * Array of macro types [MACRO_MAX]
 */
bool *macro__cmd;


/*
 * The number of quarks
 */
s16b quark__num;

/*
 * The pointers to the quarks [QUARK_MAX]
 */
cptr *quark__str;


/*
 * The next "free" index to use
 */
u16b message__next;

/*
 * The index of the oldest message (none yet)
 */
u16b message__last;

/*
 * The next "free" offset
 */
u16b message__head;

/*
 * The offset to the oldest used char (none yet)
 */
u16b message__tail;

/*
 * The array of offsets, by index [MESSAGE_MAX]
 */
u16b *message__ptr;

/*
 * The array of chars, by offset [MESSAGE_BUF]
 */
char *message__buf;


/* Hack -- Quest array */
quest q_list[MAX_Q_IDX];


/* The stores [MAX_STORES] */
store_type *store;

/* The player's inventory [INVEN_TOTAL] */
inven_type *inventory;


/* The array of dungeon monsters [MAX_M_IDX] */
monster_type *m_list;

/* The array of dungeon items [MAX_I_IDX] */
inven_type *i_list;


/* Size of the alloc_kind_table */
s16b alloc_kind_size;

/* Index into the alloc_kind_table */
s16b *alloc_kind_index;

/* Actual alloc_kind_table itself */
kind_entry *alloc_kind_table;

/* Size of the alloc_race_table */
s16b alloc_race_size;

/* Index into the alloc_race_table */
s16b *alloc_race_index;

/* Actual alloc_race_table itself */
race_entry *alloc_race_table;


/*
 * Specify attr/char pairs for inventory items (by tval)
 */
byte tval_to_attr[128];
char tval_to_char[128];

/*
 * Hack -- simple keymap method, see "arrays.c" and "commands.c".
 */
byte keymap_cmds[256];
byte keymap_dirs[256];



/*
 * Player information
 */

static player_type p_body;	/* Static player info record */

player_type *p_ptr = &p_body;	/* Pointer to the player info */

player_race *rp_ptr;	/* Pointer to the player race info */
player_class *cp_ptr;	/* Pointer to the player class info */

player_magic *mp_ptr;	/* Pointer to the player magic info */


/*
 * More spell info
 */
u32b spell_learned1;	/* bit mask of spells learned */
u32b spell_learned2;	/* bit mask of spells learned */
u32b spell_worked1;	/* bit mask of spells tried and worked */
u32b spell_worked2;	/* bit mask of spells tried and worked */
u32b spell_forgotten1;	/* bit mask of spells learned but forgotten */
u32b spell_forgotten2;	/* bit mask of spells learned but forgotten */
byte spell_order[64];	/* order spells learned/remembered/forgotten */


/*
 * Calculated base hp values for player at each level,
 * store them so that drain life + restore life does not
 * affect hit points.  Also prevents shameless use of backup
 * savefiles for hitpoint acquirement.
 */
s16b player_hp[PY_MAX_LEVEL];


/*
 * The vault generation arrays
 */
header *v_head;
vault_type *v_info;
char *v_name;
char *v_text;

/*
 * The terrain feature arrays
 */
header *f_head;
feature_type *f_info;
char *f_name;
char *f_text;

/*
 * The object kind arrays
 */
header *k_head;
inven_kind *k_info;
char *k_name;
char *k_text;

/*
 * The artifact arrays
 */
header *a_head;
artifact_type *a_info;
char *a_name;
char *a_text;

/*
 * The ego-item arrays
 */
header *e_head;
ego_item_type *e_info;
char *e_name;
char *e_text;


/*
 * The monster race arrays
 */
header *r_head;
monster_race *r_info;
char *r_name;
char *r_text;


/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
cptr ANGBAND_SYS = "xxx";

/*
 * Path name: The main "lib" directory
 * This variable is not actually used anywhere in the code
 */
cptr ANGBAND_DIR;

/*
 * High score files (binary)
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_APEX;

/*
 * Bone files for player ghosts (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_BONE;

/*
 * Binary image files for the "*_info" arrays (binary)
 * These files are not portable between platforms
 */
cptr ANGBAND_DIR_DATA;

/*
 * Textual template files for the "*_info" arrays (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_EDIT;

/*
 * Various extra files (ascii)
 * These files may be portable between platforms
 */
cptr ANGBAND_DIR_FILE;

/*
 * Help files (normal) for the online help (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_HELP;

/*
 * Help files (spoilers) for the online help (ascii)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_INFO;

/*
 * Savefiles for current characters (binary)
 * These files are portable between platforms
 */
cptr ANGBAND_DIR_SAVE;

/*
 * User "preference" files (ascii)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_USER;

/*
 * Various extra files (binary)
 * These files are rarely portable between platforms
 */
cptr ANGBAND_DIR_XTRA;


/*
 * Total Hack -- allow all items to be listed (even empty ones)
 * This is only used by "do_cmd_inven_e()" and is cleared there.
 */
bool item_tester_full;


/*
 * Here is a "pseudo-hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
byte item_tester_tval;


/*
 * Here is a "hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()", and the choice window routines.
 */
bool (*item_tester_hook)(inven_type*);



/*
 * Use the "simple" LCRNG
 */
bool Rand_quick = TRUE;


/*
 * Current "value" of the "simple" RNG
 */
u32b Rand_value;


/*
 * Current "index" for the "complex" RNG
 */
u16b Rand_place;

/*
 * Current "state" table for the "complex" RNG
 */
u32b Rand_state[RAND_DEG];



/*
 * Hack -- function hook to check "validity" of given race
 */
bool (*get_mon_num_hook)(int r_idx);



/*
 * Hack -- function hook to check "validity" of given kind
 */
bool (*get_obj_num_hook)(int k_idx);


