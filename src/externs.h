/* File: externs.h */

/* Purpose: macros, and extern's for functions and global variables */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

/*
 * This file is not realy in very good shape.  What we SHOULD have
 * is a correct list of available variables and functions.  What we
 * actually have seems to work, but does not reflect the current state
 * of the various files.
 *
 * We should NOT have both ANSI and non-ANSI versions in the same file.
 * What we SHOULD have is a quick utility that converts ANSI to non-ANSI,
 * and we should provide a version of the files on which this has been run.
 */


/*****************************************************************************/


/*
 * Here's some functions that've been macroized rather than being called
 * from everywhere.  They're short enough so that inlining them will probably
 * result in a smaller executable, and speed things up, to boot. -CWS
 */


/*
 * Simple integer math functions
 * Should just use the real ones.
 */
#define MY_MAX	MAX
#define MY_MIN	MIN
#define MY_ABS	ABS
#define MY_POM	POM


/*
 * Determines if a map location is fully inside the outer walls
 */
#define in_bounds(y, x) \
   ((((y) > 0) && ((x) > 0) && ((y) < cur_height-1) && ((x) < cur_width-1)) ? \
    (TRUE) : (FALSE))

/*
 * Determines if a map location is on or inside the outer walls
 */
#define in_bounds2(y, x) \
   ((((y) >= 0) && ((x) >= 0) && ((y) < cur_height) && ((x) < cur_width)) ? \
    (TRUE) : (FALSE))


/*
 * Determine if a "legal" grid is a "floor" grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch rubble and closed/secret doors
 */
#define floor_grid_bold(Y,X) \
    ((cave[Y][X].fval < MIN_WALL) && \
     (i_list[cave[Y][X].i_idx].tval < TV_MIN_BLOCK))
    
/*
 * Determine if a "legal" grid is a "clean floor" grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch all normal objects
 */
#define clean_grid_bold(Y,X) \
    ((cave[Y][X].fval < MIN_WALL) && \
     (cave[Y][X].i_idx == 0))
    
/*
 * Determine if a "legal" grid is an "empty floor" grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch all normal monsters (and players)
 */
#define empty_grid_bold(Y,X) \
    ((cave[Y][X].fval < MIN_WALL) && \
     (cave[Y][X].m_idx == 0))
    
/*
 * Determine if a "legal" grid is an "naked floor" grid
 * First test -- catch normal granite/quartz/magma walls
 * Second test -- catch all objects, monsters, and players
 */
#define naked_grid_bold(Y,X) \
    ((cave[Y][X].fval < MIN_WALL) && \
     (cave[Y][X].m_idx == 0) && (cave[Y][X].i_idx == 0))
    
    
    
/*
 * Determines if a map location is currently "on screen" -RAK-
 * Note that "panel_contains(y,x)" always implies "in_bounds2(y,x)".
 */
#define panel_contains(y, x) \
  ((((y) >= panel_row_min) && ((y) <= panel_row_max) && \
    ((x) >= panel_col_min) && ((x) <= panel_col_max)) ? (TRUE) : (FALSE))

/*
 * Determine if a given object is "wearable"
 */
#define wearable_p(T) \
	(((T)->tval >= TV_MIN_WEAR) && ((T)->tval <= TV_MAX_WEAR))

/*
 * Only wearable items can be cursed.
 */
#define cursed_p(T) \
	(wearable_p(T) && ((T)->flags3 & TR3_CURSED))

/*
 * Artifacts use the "name1" field
 */
#define artifact_p(T) \
	((T)->name1 ? TRUE : FALSE)

/*
 * Convert a "speed" into an "energy bonus"
 * Note that speeds below "normal" have an assymptotic effect.
 * Note that all speeds from 0 to 210 are correctly handled.
 */
#define extract_energy(S) \
	(((S) >= 110) ? ((S) - 100) : (((S) >= 70) ? (100/(120-(S))) : 1))



/**** Available macros ****/

/*
 * Generates a random long integer X where O<=X<M.
 * The integer X falls along a uniform distribution.
 * For example, if M is 100, you get "percentile dice"
 */
#define rand_int(M) (random() % (M))

/*
 * Generates a random long integer X where A<=X<=B
 * The integer X falls along a uniform distribution.
 * Note: rand_range(0,N-1) == rand_int(N)
 */
#define rand_range(A,B) ((A) + (rand_int(1+(B)-(A))))

/*
 * Generate a random long integer X where A-D<=X<=A+D
 * The integer X falls along a uniform distribution.
 * Note: rand_spread(A,D) == rand_range(A-D,A+D)
 */
#define rand_spread(A,D) ((A) + (rand_int(1+(D)+(D))) - (D))


/*
 * Generate a random long integer X where 1<=X<=M
 * Also, "correctly" handle the case of M<=1
 */
#define randint(M) (((M) <= 1) ? (1) : (rand_int(M) + 1))




/*****************************************************************************/


/*
 * Variable access to the version
 */
extern int cur_version_maj;
extern int cur_version_min;
extern int cur_patch_level;

/*
 * Savefile info 
 */
extern u32b sf_xtra;		/* Operating system info */
extern u32b sf_when;		/* Time when savefile created */
extern u16b sf_lives;		/* Number of past lives with this file */
extern u16b sf_saves;		/* Number of "saves" during this life */






/*
 * Note that "hack_m_idx" is not nearly as horrible a hack as before.
 * Needed because compact_monster() can be called from within
 * process_monsters() via place_monster() and summon_monster() 
 */

extern int hack_m_idx;			/* The "current" monster, if any */

extern int player_uid;			/* The player's uid, or zero */

extern char player_name[32];		/* The player name */

extern char history[4][60];		/* The player history */

extern char savefile[1024];		/* The save file name */

extern char died_from[160];		/* Cause of death */

extern char days[7][29];		/* The load check info */


/*
 * These are options, set with via "do_cmd_options()"
 */

/* Option set 1 */

extern int rogue_like_commands;	/* Pick initial keyset */
extern int quick_messages;		/* Quick messages -CWS */
extern int prompt_carry_flag;		/* Require "g" key to pick up */
extern int carry_query_flag;		/* Prompt for pickup */
extern int always_pickup;		/* Warn about over-flow */
extern int always_throw;		/* Do not prompt for throw */
extern int always_repeat;		/* Always repeat commands */
extern int use_old_target;		/* Use old target somehow */

extern int new_screen_layout;	/* Use the new screen layout */
extern int equippy_chars;	/* do equipment characters -CWS */
extern int depth_in_feet;	/* Display the depth in "feet" */
extern int notice_seams;	/* Highlight mineral seams */

extern int use_color;		/* Use color if possible */
extern int use_recall_win;	/* Use the "recall window" */
extern int use_choice_win;	/* Use the "choice window" */

extern int compress_savefile;	/* Compress the savefile as possible */

extern int hilite_player;	/* Hilite the player */

extern int ring_bell;		/* Ring the bell */


/* Option Set 2 -- Disturbance */

extern int find_cut;		/* Cut corners */
extern int find_examine;	/* Examine corners */
extern int find_prself;		/* Print self */
extern int find_bound;		/* Stop on borders */
extern int find_ignore_doors;	/* Run through doors */
extern int find_ignore_stairs;	/* Run past stairs */

extern int disturb_near;	/* Disturbed by "local" motion */
extern int disturb_move;	/* Disturbed by monster movement */
extern int disturb_enter;	/* Disturbed by monster appearing */
extern int disturb_leave;	/* Disturbed by monster disappearing */

extern int flush_disturb;	/* Flush input on disturbance */
extern int flush_failure;	/* Flush input on any failure */
extern int flush_command;	/* Flush input before every command */
extern int flush_unused;	/* Flush input on some condition */

extern int view_yellow_lite;	/* Use "yellow" for "torch lite" */
extern int view_bright_lite;	/* Use "bright" for (viewable) "perma-lite" */
extern int view_yellow_fast;	/* Ignore "yellow_lite" when running */
extern int view_bright_fast;	/* Ignore "bright_lite" when running */



/* Option set 3 -- Gameplay */

extern int view_pre_compute;	/* Precompute the "view" */
extern int view_xxx_compute;	/* XXX Unused */

extern int view_reduce_view;	/* Reduce "view" radius if running */
extern int view_reduce_lite;	/* Reduce torch lite if running */

extern int view_wall_memory;	/* Map "remembers" walls */
extern int view_xtra_memory;	/* Map "remembers" extra stuff */
extern int view_perma_grids;	/* Map "remembers" perma-lit grids */
extern int view_torch_grids;	/* Map "remembers" torch-lit grids */

extern int flow_by_sound;	/* Monsters track new player location */
extern int flow_by_smell;	/* Monsters track old player location */

extern int no_haggle_flag;	/* Cancel haggling */
extern int shuffle_owners;	/* Shuffle store owners occasionally */

extern int show_inven_weight;	/* Show weights in inven */
extern int show_equip_weight;	/* Show weights in equip */
extern int show_store_weight;	/* Show weights in store */
extern int plain_descriptions;	/* Plain descriptions */

extern int stack_allow_items;	/* Allow weapons and armor and such to stack */
extern int stack_allow_wands;	/* Allow wands and staffs and rods to stack */
extern int stack_force_notes;	/* Force items with different notes to stack */
extern int stack_force_costs;	/* Force items with different costs to stack */



/*
 * More options
 */
extern int delay_spd;		/* 1-9 for delays, or zero */
extern int hitpoint_warn;	/* 1-9 for hitpoint warning, or zero */


/*
 * More flags
 */
extern int in_store_flag;		/* Currently in a store */
extern int peek;			/* should we display additional msgs */
extern int coin_type;			/* Hack -- creeping coin treasure */
extern int opening_chest;		/* Hack -- chest treasure */

/*
 * Mega-Hack -- special treatment of "inkey()" when getting a command
 */
extern int inkey_flag;			/* Hack -- get a command */


/*
 * Term window info
 */
extern term *term_screen;	/* The main screen */
extern term *term_recall;	/* The recall window */
extern term *term_choice;	/* The choice window */


/* The current dungeon level */
/* Was: extern cave_type cave[MAX_HEIGHT][MAX_WIDTH]; */
extern cave_type *cave[MAX_HEIGHT];


/*
 * global flags
 */

extern int LOAD;
extern int closing_flag;		/* Used for closing   */

extern int in_store_flag;		/* Notice when in stores */
extern int good_item_flag;		/* Notice artifact created... */
extern int feeling;			/* level feeling */
extern int rating;			/* level rating */
extern int new_level_flag;		/* Next level when true  */
extern int teleport_flag;		/* Handle teleport traps  */
extern int cur_lite, old_lite;          /* Light radius */
extern int find_flag;			/* Are we running */
extern int free_turn_flag;		/* Is this turn free */

extern char doing_inven;		/* Track inventory commands */
extern int screen_change;		/* Notice disturbing of inventory */

extern int character_generated;		/* Character generation complete */
extern int character_saved;		/* Character has been saved. */
extern int peek;			/* Peek like a wizard */
extern int create_up_stair;
extern int create_down_stair;

extern int command_cmd;			/* Current command */
extern int command_old;			/* Last command */
extern int command_esc;			/* Nothing yet */
extern int command_arg;			/* Argument of current command */
extern int command_rep;			/* Repetition of current command */
extern int command_dir;			/* Direction of current command */

extern int command_xxx;			/* Just in case */
extern int command_wrk;			/* See "moria1.c" */
extern int command_new;			/* Next command to execute */
extern int command_gap;			/* First column to use for "?" */
extern int command_see;			/* See a list of options */

extern s16b noscore;			/* Don't score this game. -CJS- */

extern u32b randes_seed;		/* Hack -- consistent object colors */
extern u32b town_seed;		/* Hack -- consistent town layout */


extern s16b dun_level;			/* Cur dungeon level   */
extern s16b object_level;		/* used to generate objects -CWS */
extern int msg_flag;			/* Set with first msg  */
extern int death;			/* True if died	      */
extern u32b turn;			/* Current game turn */
extern u32b old_turn;			/* Last turn dungeon() started */
extern int wizard;			/* Is the player currently a Wizard? */
extern int to_be_wizard;		/* Does the player want to be a wizard? */
extern int can_be_wizard;		/* Can the player ever be a wizard? */
extern s16b panic_save;		/* true if playing from a panic save */



/*
 * Dungeon info
 */
extern s16b cur_height, cur_width;
extern s16b max_panel_rows, max_panel_cols;
extern int panel_row, panel_col;
extern int panel_row_min, panel_row_max;
extern int panel_col_min, panel_col_max;
extern int panel_col_prt, panel_row_prt;

#ifdef TARGET
/* Targetting code, stolen from Morgul -CFT */
extern int target_mode;
extern int target_mon;
extern int target_row;
extern int target_col;
#endif

/* A pointer to the main player record */
extern player_type *p_ptr;

extern player_race race[MAX_RACES];
extern player_class class[MAX_CLASS];
extern player_background background[MAX_BACKGROUND];

extern cptr player_title[MAX_CLASS][MAX_PLAYER_LEVEL];

extern u32b player_exp[MAX_PLAYER_LEVEL];
extern u16b player_hp[MAX_PLAYER_LEVEL];

extern s16b char_row;
extern s16b char_col;

extern s16b class_level_adj[MAX_CLASS][MAX_LEV_ADJ];
extern u16b player_init[MAX_CLASS][3];

extern s16b total_winner;


/*** Spell Information ***/

/* Warriors don't have spells, so there is no entry for them. */
extern spell_type magic_spell[MAX_CLASS-1][63];
extern cptr spell_names[127];
extern u32b spell_learned;	/* Bit field for spells learnt -CJS- */
extern u32b spell_learned2;	/* Bit field for spells learnt -CJS- */
extern u32b spell_worked;	/* Bit field for spells tried -CJS- */
extern u32b spell_worked2;	/* Bit field for spells tried -CJS- */
extern u32b spell_forgotten;	/* Bit field for spells forgotten -JEW- */
extern u32b spell_forgotten2;	/* Bit field for spells forgotten -JEW- */
extern byte spell_order[64];	/* remember order that spells are learned in */
extern u32b spellmasks[MAX_CLASS][2];	/* what spells can classes learn */


/*** Store information ***/

extern store_type store[MAX_STORES];


/*** Miscellaneous Information ***/

extern cptr ego_names[EGO_MAX];		/* Ego-item Names */

extern char ghost_name[128];		/* Writable ghost name */

extern monster_attack a_list[MAX_A_IDX];	/* Monster attacks */

extern byte blows_table[11][12];

extern u16b normal_table[NORMAL_TABLE_SIZE];

extern byte keymap_cmds[128];
extern byte keymap_dirs[128];


/*** Inventory ***/

/* Inventory information */
extern s16b inven_weight;		/* Total carried weight */
extern s16b inven_ctr;			/* Number of obj's in inven */
extern s16b equip_ctr;			/* Number of obj's in equip */

/* Player inventory (inven+equip) */
extern inven_type inventory[INVEN_TOTAL];


/*** Item Information ***/

/* Treasure heap pointer (used with i_list) */
extern s16b i_max;

/* Actual array of all physical objects (on the ground) */
extern inven_type *i_list;

/* The array of object types (parsed from "k_list" file) */
extern inven_kind *k_list;

/* The array of graphic attr/char info for objects */
extern byte *k_attr;
extern char *k_char;

/* The "xtra" array ("extra" object info) */
extern inven_xtra *x_list;

/* The "artifact" template array */
extern inven_very *v_list;

/* Attribute table for inventory */
extern byte tval_to_attr[128];


/*** Monster Information ***/

/* Monster heap pointer (used with m_list) */
extern s16b m_max;

/* Actual array of physical monsters */
extern monster_type *m_list;

/* The array of monster races */
extern monster_race *r_list;

/* The array of graphic attr/char info for monsters */
extern byte *r_attr;
extern char *r_char;

/* Monster memories. */
extern monster_lore *l_list;

/* Hack -- The "quests" */
extern quest q_list[QUEST_MAX];


/* 
 * Here is a "hook" used during calls to "get_item()" and "show_inven()".
 */
extern bool (*item_tester_hook)(inven_type*);


/*
 * The FILEPATH's to various files, see "arrays.c"
 */

extern cptr ANGBAND_DIR_FILES;		/* Dir: ascii files  */
extern cptr ANGBAND_DIR_BONES;		/* Dir: ascii bones files */
extern cptr ANGBAND_DIR_SAVE;		/* Dir: binary save files */
extern cptr ANGBAND_DIR_DATA;		/* Dir: system dependant files */

extern cptr ANGBAND_NEWS;		/* News file */
extern cptr ANGBAND_WELCOME;		/* Player generation help */
extern cptr ANGBAND_VERSION;		/* Version information */

extern cptr ANGBAND_WIZ;		/* Acceptable wizard uid's */
extern cptr ANGBAND_HOURS;		/* Hours of operation */
extern cptr ANGBAND_LOAD;		/* Load information */
extern cptr ANGBAND_LOG;		/* Log file of some form */

extern cptr ANGBAND_R_HELP;		/* Roguelike command help */
extern cptr ANGBAND_O_HELP;		/* Original command help */
extern cptr ANGBAND_W_HELP;		/* Wizard command help */

extern cptr ANGBAND_R_LIST;		/* Ascii monster race file */
extern cptr ANGBAND_K_LIST;		/* Ascii item kind file */
extern cptr ANGBAND_V_LIST;		/* Ascii artifact template file */
extern cptr ANGBAND_A_LIST;		/* Ascii attr/char extra file */


/*
 * only extern functions declared here, static functions declared
 * inside the file that defines them.  Duh...
 */


#ifdef __STDC__

/* birth.c */
void player_birth(void);

/* borg.c */
#ifdef AUTO_PLAY
extern void Borg_init(void);
extern void Borg_mode(void);
#endif


/* creature.c */
void update_mon(int);
int movement_rate(int);
int multiply_monster(int);
void update_monsters(void);
void process_monsters(void);

/* death.c */
void init_scorefile(void);
void nuke_scorefile(void);
void display_scores(int, int);
void delete_entry(int);
long total_points(void);
void exit_game(void);
void exit_game_panic(void);

/* desc.c */
void flavor_init(void);
bool flavor_p(inven_type *);
bool inven_aware_p(inven_type *);
void inven_aware(inven_type *);
bool inven_tried_p(inven_type *);
void inven_tried(inven_type *);
bool known2_p(inven_type *);
void known2(inven_type *);
char inven_char(inven_type *);
byte inven_attr(inven_type *);
void inscribe(inven_type *, cptr);
void objdes(char *, inven_type *, int);
void objdes_store(char *, inven_type *, int);
void invcopy(inven_type *, int);
void inven_item_charges(int);
void inven_item_describe(int);

/* command.c */
void process_command(void);
void request_command(void);

/* dungeon.c */
void dungeon(void);

/* effects.c */
void do_cmd_eat_food(void);
void do_cmd_quaff_potion(void);
void do_cmd_read_scroll(void);
void do_cmd_aim_wand(void);
void do_cmd_use_staff(void);
void do_cmd_zap_rod(void);
void do_cmd_activate(void);

/* files.c */
void read_times(void);
void show_news(void);
void helpfile(cptr);
int file_character(cptr);
int mac_file_character(void);

/* generate.c */
void generate_cave(void);

/* cave.c */

int los(int, int, int, int);
int test_lite(int, int);
int no_lite(void);

bool player_has_los(int, int);
bool player_can_see(int, int);

void forget_flow(void);
void update_flow(void);
void forget_lite(void);
void update_lite(void);
void forget_view(void);
void update_view(void);

void lite_room(int, int);
void unlite_room(int, int);

void move_cursor_relative(int, int);

char inven_char(inven_type *i_ptr);
byte inven_attr(inven_type *i_ptr);
byte inven_attr_by_tval(inven_type *i_ptr);

byte spell_color(int type);

void mh_forget(void);
void mh_cycle(void);
void mh_print(char, byte, int, int, int);
void mh_print_rel(char, byte, int, int, int);

void lite_spot(int, int);
void lite_monster(monster_type*);

void screen_map(void);


/* io.c */

void bell(void);

void move_cursor(int, int);

void text_to_ascii(char *buf, cptr str);
void ascii_to_text(char *buf, cptr str);

void macro(cptr, cptr);
void macro_dump(cptr);

void flush(void);

char inkey(void);

uint message_num(void);
uint message_len(uint);
cptr message_str(uint);
void message_add(cptr, int);
void message_new(cptr, int);

void msg_more(int);
void msg_print(cptr);
void message(cptr, int);

void erase_line(int, int);
void clear_screen(void);
void clear_from(int);

void c_put_str(byte, cptr, int, int);
void put_str(cptr, int, int);

void c_prt(byte, cptr, int, int);
void prt(cptr, int, int);

void save_screen(void);
void restore_screen(void);

int get_check(cptr);
int get_com(cptr, char *);
int get_string(char *, int, int, int);
int askfor(char *, int);
void pause_line(int);


/* magic.c */
int door_creation(void);
void stair_creation(void);
void pray(void);
void cast(void);

/* arrays.c */
void get_file_paths(void);
void init_some_arrays(void);
void keymap_init(void);

/* main.c */
void play_game(void);
void play_game_mac(int);

#ifdef _Windows
/* main-win.c */
char *get_lib_path(void);
#endif

/* misc1.c */
void init_seeds(void);
void set_seed(u32b);
void reset_seed(void);
int check_time(void);
int randnor(int, int);
int bit_pos(u32b *);
void panel_bounds(void);
int get_panel(int, int, int);
int damroll(int, int);
int pdamroll(byte *);
void prt_map(void);
void add_food(int);

/* misc2.c */
void delete_monster_idx(int);
void delete_monster(int, int);
void tighten_m_list(void);
void wipe_m_list(void);
int m_pop(void);
int max_hp(byte *);
int place_monster(int, int, int, int);
int place_win_monster(void);
void place_group(int, int, int, int);
int get_mons_num(int);
int poly_r_idx(int);
void alloc_monster(int, int, int);
int summon_monster(int * ,int *, int);
int summon_undead(int *, int *);
int summon_demon(int, int *, int *);
int summon_dragon(int *, int *);
int summon_wraith(int *, int *);
int summon_reptile(int *, int *);
int summon_spider(int *, int *);
int summon_angel(int *, int *);
int summon_ant(int *, int *);
int summon_unique(int *, int *);
int summon_jabberwock(int *, int *);
int summon_gundead(int *, int *);
int summon_ancientd(int *, int *);
int summon_hound(int *, int *);
int get_nmons_num(int);
int distance(int, int, int, int);

/* misc3.c */
int i_pop(void);
void delete_object_idx(int);
void delete_object(int, int);
void wipe_i_list(void);
int magik(int);
bool make_artifact(inven_type *);
int m_bonus(int, int, int);
void apply_magic(inven_type *, int, bool, bool, bool);
bool valid_grid(int, int);
void place_trap(int, int);
void place_rubble(int, int);
void place_gold(int, int);
int get_obj_num(int,int);
void place_object(int, int);
void random_object(int, int, int);
void cnv_stat(int, char *);
void prt_stat(int);
void prt_field(cptr, int, int);
int stat_adj(int);
int chr_adj(void);
int con_adj(void);
cptr title_string(void);
void prt_title(void);
void prt_level(void);
void prt_cmana(void);
void prt_mhp(void);
void prt_chp(void);
void prt_pac(void);
void prt_gold(void);
void prt_depth(void);
void prt_hunger(void);
void prt_blind(void);
void prt_confused(void);
void prt_afraid(void);
void prt_poisoned(void);
void prt_state(void);
void prt_speed(void);
void prt_study(void);
void prt_winner(void);
int modify_stat(int, int);
void set_use_stat(int);
int inc_stat(int);
int dec_stat(int,int,int);
int res_stat(int);
int tohit_adj(void);
int toac_adj(void);
int todis_adj(void);
int todam_adj(void);
void prt_stat_block(void);
void draw_cave(void);
void put_character(void);
void put_stats(void);
cptr likert(int, int);
void put_misc1(void);
void put_misc2(void);
void put_misc3(void);
void display_player(void);
void get_name(void);
void change_name(void);
int item_similar(inven_type *, inven_type *);
int combine(int);
void combine_pack(void);
void inven_item_increase(int, int);
void inven_item_optimize(int);
int inven_check_num(inven_type *);
int inven_carry(inven_type *);
int weight_limit(void);
int inven_check_weight(inven_type *);
void check_strength(void);
int spell_chance(int);
void print_spells(int *, int, int, int);
int get_spell(int *, int, int *, int *, cptr, int);
void calc_spells(int);
void gain_spells(void);
void calc_mana(int);
void prt_experience(void);
void calc_hitpoints(void);
bool insert_str(char *, cptr, cptr);
int enter_wiz_mode(void);
int attack_blows(int);
int tot_dam(inven_type *, int, int);
int critical_blow(int, int, int, int);
int mmove(int, int *, int *);
int player_saves(void);
int find_range(int, int, int *, int *);
void teleport(int);
void check_view(void);
void place_good(int, int, bool);
int place_ghost(void);
void prt_cut(void);
void prt_stun(void);
void special_random_object(int, int, int);
void cut_player(int);
void stun_player(int);
void prt_equippy_chars(void);
int get_coin_type(monster_race *);

/* moria1.c */
void choice_inven(int, int);
void choice_equip(int, int);
void choice_spell(int*, int, int);
void move_rec(int, int, int, int);
void calc_bonuses(void);
void show_inven(int, int);
void show_equip(int, int);
int verify(cptr, int);
void inven_command(int);
int get_item(int *, cptr, int, int);
int test_hit(int, int, int, int, int);
void take_hit(int, cptr);
void change_trap(int, int);

/* moria2.c */
int is_quest(int);
int ruin_stat(int);
void hit_trap(int, int);
int cast_spell(cptr ,int, int *, int *);
void monster_death(monster_type *, bool, bool);
bool mon_take_hit(int, int, bool);
void py_attack(int, int);
void shoot(int,int);
void py_bash(int, int);
void drop_near(inven_type*, int, int, int);

/* moria3.c */
int twall(int, int, int, int);
void do_cmd_look(void);
void do_cmd_locate(void);
void do_cmd_view_map(void);
void do_cmd_rest(void);
void do_cmd_view_map(void);
void do_cmd_open(void);
void do_cmd_close(void);
void do_cmd_tunnel(void);
void do_cmd_disarm(void);
void do_cmd_bash(void);
void do_cmd_spike(void);
void do_cmd_fire(void);
void do_cmd_walk(int);
void do_cmd_stay(int);
void do_cmd_run(void);
void do_cmd_search(void);
void do_cmd_rest(void);
void do_cmd_feeling(void);
void do_cmd_inscribe(void);
void do_cmd_check_artifacts(void);
void do_cmd_check_uniques(void);

/* moria4.c */
int is_a_vowel(int);
int index_to_label(int);
int label_to_inven(int);
int label_to_equip(int);
cptr describe_use(int);
cptr mention_use(int);
void monster_desc(char *, monster_type*, int);
int get_tag(int*,char);
void target_update(void);
int target_at(int, int);
int target_okay(void);
int target_set(void);
void mmove2(int *, int *, int, int, int, int);
void confuse_dir(int *, int);
int get_a_dir(cptr, int *, int);
int get_dir(cptr, int *);
int get_dir_c(cptr, int *);
void disturb(int, int);
void search_on(void);
void search_off(void);
void search(int, int, int);
void rest_off(void);
void carry(int, int, int);
void move_player(int, int);
void find_step(void);
void find_init(void);
void end_find(void);

/* recall.c */
int roff_recall(int);
int bool_roff_recall(int);
void lore_do_probe(monster_type *m_ptr);
void lore_treasure(monster_type *m_ptr, int num_item, int num_gold);
void do_cmd_query_symbol(void);

/* save.c */
int save_player(void);
int _save_player(char *);
int load_player(int *);

/* signals.c */
void signals_ignore_tstp(void);
void signals_handle_tstp(void);
void signals_init(void);
void ignore_signals(void);
void default_signals(void);
void restore_signals(void);

/* spells.c */
void fire_dam(int, cptr);
void cold_dam(int, cptr);
void elec_dam(int, cptr);
void acid_dam(int, cptr);
void poison_gas(int, cptr);
bool project(int, int, int, int, int, int, int);
bool apply_disenchant(int);
int sleep_monsters1(int, int);
int detect_treasure(void);
int detect_magic(void);
int detect_object(void);
int detect_trap(void);
int detect_sdoor(void);
int detect_invisible(void);
int lite_area(int, int, int, int);
int unlite_area(int, int);
void map_area(void);
bool ident_floor(void);
int ident_spell(void);
int identify_fully(void);
void identify_pack(void);
int aggravate_monster(int);
int trap_creation(void);
int door_creation(void);
void stair_creation(void);
int td_destroy(void);
int detect_monsters(void);
void lite_line(int, int, int);
void starlite(int, int);
int disarm_all(int, int, int);
void bolt(int, int, int);
void breath(int, int, int);
void fire_bolt(int, int, int, int, int);
void fire_ball(int, int, int, int, int, int);
int recharge(int);
int heal_monster(int, int, int);
int drain_life(int, int, int, int);
int speed_monster(int, int, int);
int slow_monster(int, int, int);
int confuse_monster(int, int, int, int);
int sleep_monster(int, int, int);
int wall_to_mud(int, int, int);
int td_destroy2(int, int, int);
int poly_monster(int, int, int);
int build_wall(int, int, int);
int clone_monster(int, int, int);
void teleport_away(int, int);
void teleport_to(int, int);
int teleport_monster(int, int, int);
int mass_genocide(int);
int genocide(int);
int speed_monsters(void);
int slow_monsters(void);
int sleep_monsters2(void);
int mass_poly(void);
int detect_evil(void);
int hp_player(int);
int cure_confusion(void);
int cure_blindness(void);
int cure_poison(void);
int remove_fear(void);
void earthquake(void);
int protect_evil(void);
void satisfy_hunger(void);
int dispel_creature(u32b, int);
int turn_undead(void);
void warding_glyph(void);
void lose_exp(s32b);
int slow_poison(void);
void bless(int);
void detect_inv2(int);
void destroy_area(int, int);
int enchant(inven_type *, int, int);
void elemental_brand(void);
int remove_curse(void);
int restore_level(void);
void self_knowledge(void);
int probing(void);
int detection(void);
void starball(int,int);
void spell_hit_monster(monster_type *, int, int *, int, int *, int *, int);
void wiz_lite(void);
void wiz_dark(void);
int lose_all_info(void);
void tele_level(void);
void identify_pack(void);
int fear_monster(int, int, int, int);
int banish_evil(int);
int remove_all_curse(void);
cptr pain_message(int, int);
int line_spell(int, int, int, int, int);

/* store.c */
s32b item_value(inven_type *i_ptr);
void store_init(void);
void store_maint(void);
void store_shuffle(void);
void enter_store(int);


/* util.c */
#ifndef HAS_USLEEP
int usleep(unsigned long);
#endif
void delay(int);
void user_name(char *buf, int id);
int my_topen(cptr, int, int);
FILE *my_tfopen(cptr, cptr);


/* wizard.c */
int do_wiz_command(void);


#endif      /* __STDC__ */


