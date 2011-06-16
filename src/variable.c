/*
 * File: variable.c
 * Purpose: Various global variables
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "buildid.h"


/*
 * Hack -- Link a copyright message into the executable
 */
const char *copyright =
	"Copyright (c) 1987-2011 Angband contributors.\n"
	"\n"
	"This work is free software; you can redistribute it and/or modify it\n"
	"under the terms of either:\n"
	"\n"
	"a) the GNU General Public License as published by the Free Software\n"
	"   Foundation, version 2, or\n"
	"\n"
	"b) the Angband licence:\n"
	"   This software may be copied and distributed for educational, research,\n"
	"   and not for profit purposes provided that this copyright and statement\n"
	"   are included in all such copies.  Other copyrights may also apply.\n";

/*
 * Run-time arguments
 */
bool arg_wizard;			/* Command arg -- Request wizard mode */
bool arg_rebalance;			/* Command arg -- Rebalance monsters */
int arg_graphics;			/* Command arg -- Request graphics mode */
bool arg_graphics_nice;			/* Command arg -- Request nice graphics mode */

#ifdef ALLOW_BORG
bool screensaver = FALSE;
#endif /* ALLOW_BORG */

/*
 * Various things
 */

bool character_generated;	/* The character exists */
bool character_dungeon;		/* The character has a dungeon */
bool character_saved;		/* The character was just saved to a savefile */

s16b character_icky;		/* Depth of the game in special mode */
s16b character_xtra;		/* Depth of the game in startup mode */

u32b seed_randart;		/* Hack -- consistent random artifacts */

u32b seed_flavor;		/* Hack -- consistent object colors */
u32b seed_town;			/* Hack -- consistent town layout */

s16b num_repro;			/* Current reproducer count */

char summon_kin_type;		/* Hack -- See summon_specific() */

s32b turn;				/* Current game turn */

int use_graphics;		/* The "graphics" mode is enabled */
bool use_graphics_nice;	        /* The 'nice' "graphics" mode is enabled */
byte tile_width = 1;            /* Tile width in units of font width */
byte tile_height = 1;           /* Tile height in units of font height */

s16b signal_count;		/* Hack -- Count interrupts */

bool msg_flag;			/* Player has pending message */

bool inkey_xtra;		/* See the "inkey()" function */
u32b inkey_scan;		/* See the "inkey()" function */
bool inkey_flag;		/* See the "inkey()" function */

s16b o_max = 1;			/* Number of allocated objects */
s16b o_cnt = 0;			/* Number of live objects */

/*
 * Buffer to hold the current savefile name
 */
char savefile[1024];

/*
 * The array[ANGBAND_TERM_MAX] of window pointers
 */
term *angband_term[ANGBAND_TERM_MAX];


/*
 * The array[ANGBAND_TERM_MAX] of window names (modifiable?)
 *
 * ToDo: Make the names independent of ANGBAND_TERM_MAX.
 */
char angband_term_name[ANGBAND_TERM_MAX][16] =
{
	VERSION_NAME,
	"Term-1",
	"Term-2",
	"Term-3",
	"Term-4",
	"Term-5",
	"Term-6",
	"Term-7"
};

/*
 * Global table of color definitions (mostly zeros)
 */
byte angband_color_table[MAX_COLORS][4] =
{
	{0x00, 0x00, 0x00, 0x00}, /* 0  TERM_DARK */
	{0x00, 0xff, 0xff, 0xff}, /* 1  TERM_WHITE */
	{0x00, 0x80, 0x80, 0x80}, /* 2  TERM_SLATE */
	{0x00, 0xff, 0x80, 0x00}, /* 3  TERM_ORANGE */
	{0x00, 0xc0, 0x00, 0x00}, /* 4  TERM_RED */
	{0x00, 0x00, 0x80, 0x40}, /* 5  TERM_GREEN */
	{0x00, 0x00, 0x40, 0xff}, /* 6  TERM_BLUE */
	{0x00, 0x80, 0x40, 0x00}, /* 7  TERM_UMBER */
	{0x00, 0x60, 0x60, 0x60}, /* 8  TERM_L_DARK */
	{0x00, 0xc0, 0xc0, 0xc0}, /* 9  TERM_L_WHITE */
	{0x00, 0xff, 0x00, 0xff}, /* 10 TERM_L_PURPLE */
	{0x00, 0xff, 0xff, 0x00}, /* 11 TERM_YELLOW */
	{0x00, 0xff, 0x40, 0x40}, /* 12 TERM_L_RED */
	{0x00, 0x00, 0xff, 0x00}, /* 13 TERM_L_GREEN */
	{0x00, 0x00, 0xff, 0xff}, /* 14 TERM_L_BLUE */
	{0x00, 0xc0, 0x80, 0x40}, /* 15 TERM_L_UMBER */
	{0x00, 0x90, 0x00, 0x90}, /* 16 TERM_PURPLE */
	{0x00, 0x90, 0x20, 0xff}, /* 17 TERM_VIOLET */
	{0x00, 0x00, 0xa0, 0xa0}, /* 18 TERM_TEAL */
	{0x00, 0x6c, 0x6c, 0x30}, /* 19 TERM_MUD */
	{0x00, 0xff, 0xff, 0x90}, /* 20 TERM_L_YELLOW */
	{0x00, 0xff, 0x00, 0xa0}, /* 21 TERM_MAGENTA */
	{0x00, 0x20, 0xff, 0xdc}, /* 22 TERM_L_TEAL */
	{0x00, 0xb8, 0xa8, 0xff}, /* 23 TERM_L_VIOLET */
	{0x00, 0xff, 0x80, 0x80}, /* 24 TERM_L_PINK */
	{0x00, 0xb4, 0xb4, 0x00}, /* 25 TERM_MUSTARD */
	{0x00, 0xa0, 0xc0, 0xd0}, /* 26 TERM_BLUE_SLATE */
	{0x00, 0x00, 0xb0, 0xff}, /* 27 TERM_DEEP_L_BLUE */
};

/*
 * Global array of color names and translations.
 */
color_type color_table[MAX_COLORS] =
{
	/* full mono vga blind lighter darker highlight metallic misc */
	{'d', "Dark", {0, 0, 0, TERM_DARK, TERM_L_DARK, TERM_DARK,
				   TERM_L_DARK, TERM_L_DARK, TERM_DARK}},

	{'w', "White", {1, 1, 1, TERM_WHITE, TERM_YELLOW, TERM_SLATE,
					TERM_L_BLUE, TERM_YELLOW, TERM_WHITE}},

	{'s', "Slate", {2, 1, 2, TERM_SLATE, TERM_L_WHITE, TERM_L_DARK,
					TERM_L_WHITE, TERM_L_WHITE, TERM_SLATE}},

	{'o', "Orange", {3, 1, 3, TERM_L_WHITE, TERM_YELLOW, TERM_SLATE,
					 TERM_YELLOW, TERM_YELLOW, TERM_ORANGE}},

	{'r', "Red", {4, 1, 4, TERM_SLATE, TERM_L_RED, TERM_SLATE,
				  TERM_L_RED, TERM_L_RED, TERM_RED}},

	{'g', "Green", {5, 1, 5, TERM_SLATE, TERM_L_GREEN, TERM_SLATE,
					TERM_L_GREEN, TERM_L_GREEN, TERM_GREEN}},

	{'b', "Blue", {6, 1, 6, TERM_SLATE, TERM_L_BLUE, TERM_SLATE,
				   TERM_L_BLUE, TERM_L_BLUE, TERM_BLUE}},

	{'u', "Umber", {7, 1, 7, TERM_L_DARK, TERM_L_UMBER, TERM_L_DARK,
					TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER}},

	{'D', "Light Dark", {8, 1, 8, TERM_L_DARK, TERM_SLATE, TERM_L_DARK,
						 TERM_SLATE, TERM_SLATE, TERM_L_DARK}},

	{'W', "Light Slate", {9, 1, 9, TERM_L_WHITE, TERM_WHITE, TERM_SLATE,
						  TERM_WHITE, TERM_WHITE, TERM_SLATE}},

	{'P', "Light Purple", {10, 1, 10, TERM_SLATE, TERM_YELLOW, TERM_SLATE,
						   TERM_YELLOW, TERM_YELLOW, TERM_L_PURPLE}},

	{'y', "Yellow", {11, 1, 11, TERM_L_WHITE, TERM_L_YELLOW, TERM_L_WHITE,
					 TERM_WHITE, TERM_WHITE, TERM_YELLOW}},

	{'R', "Light Red", {12, 1, 12, TERM_L_WHITE, TERM_YELLOW, TERM_RED,
						TERM_YELLOW, TERM_YELLOW, TERM_L_RED}},

	{'G', "Light Green", {13, 1, 13, TERM_L_WHITE, TERM_YELLOW, TERM_GREEN,
						  TERM_YELLOW, TERM_YELLOW, TERM_L_GREEN}},

	{'B', "Light Blue", {14, 1, 14, TERM_L_WHITE, TERM_YELLOW, TERM_BLUE,
						 TERM_YELLOW, TERM_YELLOW, TERM_L_BLUE}},

	{'U', "Light Umber", {15, 1, 15, TERM_L_WHITE, TERM_YELLOW, TERM_UMBER,
						  TERM_YELLOW, TERM_YELLOW, TERM_L_UMBER}},

	/* "new" colors */
	{'p', "Purple", {16, 1, 10,TERM_SLATE, TERM_L_PURPLE, TERM_SLATE,
					 TERM_L_PURPLE, TERM_L_PURPLE, TERM_L_PURPLE}},

	{'v', "Violet", {17, 1, 10,TERM_SLATE, TERM_L_PURPLE, TERM_SLATE,
					 TERM_L_PURPLE, TERM_L_PURPLE, TERM_L_PURPLE}},

	{'t', "Teal", {18, 1, 6, TERM_SLATE, TERM_L_TEAL, TERM_SLATE,
				   TERM_L_TEAL, TERM_L_TEAL, TERM_L_BLUE}},

	{'m', "Mud", {19, 1, 5, TERM_SLATE, TERM_MUSTARD, TERM_SLATE,
				  TERM_MUSTARD, TERM_MUSTARD, TERM_UMBER}},

	{'Y', "Light Yellow", {20, 1, 11, TERM_WHITE, TERM_WHITE, TERM_YELLOW,
						   TERM_WHITE, TERM_WHITE, TERM_L_YELLOW}},

	{'i', "Magenta-Pink", {21, 1, 12, TERM_SLATE, TERM_L_PINK, TERM_RED,
						   TERM_L_PINK, TERM_L_PINK, TERM_L_PURPLE}},

	{'T', "Light Teal", {22, 1, 14, TERM_L_WHITE, TERM_YELLOW, TERM_TEAL,
						 TERM_YELLOW, TERM_YELLOW, TERM_L_BLUE}},

	{'V', "Light Violet", {23, 1, 10, TERM_L_WHITE, TERM_YELLOW, TERM_VIOLET,
						   TERM_YELLOW, TERM_YELLOW, TERM_L_PURPLE}},

	{'I', "Light Pink", {24, 1, 12, TERM_L_WHITE, TERM_YELLOW, TERM_MAGENTA,
						 TERM_YELLOW, TERM_YELLOW, TERM_L_PURPLE}},

	{'M', "Mustard", {25, 1, 11, TERM_SLATE, TERM_YELLOW, TERM_SLATE,
					  TERM_YELLOW, TERM_YELLOW, TERM_YELLOW}},

	{'z', "Blue Slate",  {26, 1, 9, TERM_SLATE, TERM_DEEP_L_BLUE, TERM_SLATE,
						  TERM_DEEP_L_BLUE, TERM_DEEP_L_BLUE, TERM_L_WHITE}},

	{'Z', "Deep Light Blue", {27, 1, 14, TERM_L_WHITE, TERM_L_BLUE, TERM_BLUE_SLATE,
							  TERM_L_BLUE, TERM_L_BLUE, TERM_L_BLUE}},

	/* Rest to be filled in when the game loads */
};



/*
 * Standard sound (and message) names
 */
const const char *angband_sound_name[MSG_MAX] =
{
	"",
	"hit",
	"miss",
	"flee",
	"drop",
	"kill",
	"level",
	"death",
	"study",
	"teleport",
	"shoot",
	"quaff",
	"zap_rod",
	"walk",
	"tpother",
	"hitwall",
	"eat",
	"store1",
	"store2",
	"store3",
	"store4",
	"dig",
	"opendoor",
	"shutdoor",
	"tplevel",
	"bell",
	"nothing_to_open",
	"lockpick_fail",
	"stairs_down", 
	"hitpoint_warn",
	"act_artifact", 
	"use_staff", 
	"destroy", 
	"mon_hit", 
	"mon_touch", 
	"mon_punch", 
	"mon_kick", 
	"mon_claw", 
	"mon_bite", 
	"mon_sting", 
	"mon_butt", 
	"mon_crush", 
	"mon_engulf", 
	"mon_crawl", 
	"mon_drool", 
	"mon_spit", 
	"mon_gaze", 
	"mon_wail", 
	"mon_spore", 
	"mon_beg", 
	"mon_insult", 
	"mon_moan", 
	"recover", 
	"blind", 
	"confused", 
	"poisoned", 
	"afraid", 
	"paralyzed", 
	"drugged", 
	"speed", 
	"slow", 
	"shield", 
	"blessed", 
	"hero", 
	"berserk", 
	"bold", 
	"prot_evil", 
	"invuln", 
	"see_invis", 
	"infrared", 
	"res_acid", 
	"res_elec", 
	"res_fire", 
	"res_cold", 
	"res_pois", 
	"stun", 
	"cut", 
	"stairs_up", 
	"store_enter", 
	"store_leave", 
	"store_home", 
	"money1", 
	"money2", 
	"money3", 
	"shoot_hit", 
	"store5", 
	"lockpick", 
	"disarm", 
	"identify_bad", 
	"identify_ego", 
	"identify_art", 
	"breathe_elements", 
	"breathe_frost", 
	"breathe_elec", 
	"breathe_acid", 
	"breathe_gas", 
	"breathe_fire", 
	"breathe_confusion", 
	"breathe_disenchant", 
	"breathe_chaos", 
	"breathe_shards", 
	"breathe_sound", 
	"breathe_light", 
	"breathe_dark", 
	"breathe_nether", 
	"breathe_nexus", 
	"breathe_time", 
	"breathe_inertia", 
	"breathe_gravity", 
	"breathe_plasma", 
	"breathe_force", 
	"summon_monster", 
	"summon_angel", 
	"summon_undead", 
	"summon_animal", 
	"summon_spider", 
	"summon_hound", 
	"summon_hydra", 
	"summon_demon", 
	"summon_dragon", 
	"summon_gr_undead", 
	"summon_gr_dragon", 
	"summon_gr_demon", 
	"summon_ringwraith", 
	"summon_unique", 
	"wield", 
	"cursed", 
	"pseudo_id", 
	"hungry", 
	"notice", 
	"ambient_day", 
	"ambient_nite", 
	"ambient_dng1", 
	"ambient_dng2", 
	"ambient_dng3", 
	"ambient_dng4", 
	"ambient_dng5", 
	"mon_create_trap", 
	"mon_shriek", 
	"mon_cast_fear", 
	"hit_good", 
	"hit_great", 
	"hit_superb", 
	"hit_hi_great", 
	"hit_hi_superb", 
	"cast_spell", 
	"pray_prayer",
	"kill_unique",
	"kill_king",
	"drain_stat",
	"multiply"
};

/*
 * Arrays[TEMP_MAX] used for various things
 */
u16b *temp_g;

/*
 * Array[z_info->r_max] of monster lore
 */
monster_lore *l_list;


/*
 * Hack -- Array[MAX_Q_IDX] of quests
 */
quest *q_list;


/*
 * Array[MAX_STORES] of stores
 */
struct store *stores;

/*
 * Flag to override which store is selected if in a knowledge menu
 */
int store_knowledge = STORE_NONE;

/*
 * Array[RANDNAME_NUM_TYPES][num_names] of random names
 */
const char *** name_sections;

/*
 * The size of the "alloc_ego_table"
 */
s16b alloc_ego_size;

/*
 * The array[alloc_ego_size] of entries in the "ego allocator table"
 */
alloc_entry *alloc_ego_table;


/*
 * The size of "alloc_race_table" (at most z_info->r_max)
 */
s16b alloc_race_size;

/*
 * The array[alloc_race_size] of entries in the "race allocator table"
 */
alloc_entry *alloc_race_table;


/*
 * Specify attr/char pairs for visual special effects for project()
 */
byte gf_to_attr[GF_MAX][BOLT_MAX];
char gf_to_char[GF_MAX][BOLT_MAX];


/*
 * Specify color for inventory item text display (by tval)
 * Be sure to use "index & 0x7F" to avoid illegal access
 */
byte tval_to_attr[128];




/*** Player information ***/

/*
 * The player other record (static)
 */
static player_other player_other_body;

/*
 * Pointer to the player other record
 */
player_other *op_ptr = &player_other_body;

/*
 * The player info record (static)
 */
static player_type player_type_body;

/*
 * Pointer to the player info record
 */
player_type *p_ptr = &player_type_body;


/*
 * Structure (not array) of size limits
 */
maxima *z_info;

/*
 * The vault generation arrays
 */
feature_type *f_info;

object_kind *k_info;
object_base *kb_info;

/*
 * The artifact arrays
 */
artifact_type *a_info;

/*
 * The ego-item arrays
 */
ego_item_type *e_info;

/*
 * The monster race arrays
 */
monster_race *r_info;
monster_base *rb_info;
monster_pain *pain_messages;

struct player_race *races;
struct player_class *classes;
struct vault *vaults;
struct object_kind *objkinds;

struct flavor *flavors;

/*
 * The spell arrays
 */
spell_type *s_info;

/*
 * The hints array
 */
struct hint *hints;

/*
 * Array of pit types
 */
struct pit_profile *pit_info;
 
/*
 * Hack -- The special Angband "System Suffix"
 * This variable is used to choose an appropriate "pref-xxx" file
 */
const char *ANGBAND_SYS = "xxx";

/*
 * Hack -- The special Angband "Graphics Suffix"
 * This variable is used to choose an appropriate "graf-xxx" file
 */
const char *ANGBAND_GRAF = "old";

/*
 * Various directories. These are no longer necessarily all subdirs of "lib"
 */
char *ANGBAND_DIR_APEX;
char *ANGBAND_DIR_EDIT;
char *ANGBAND_DIR_FILE;
char *ANGBAND_DIR_HELP;
char *ANGBAND_DIR_INFO;
char *ANGBAND_DIR_SAVE;
char *ANGBAND_DIR_PREF;
char *ANGBAND_DIR_USER;
char *ANGBAND_DIR_XTRA;

/* 
 * Various xtra/ subdirectories.
 */
char *ANGBAND_DIR_XTRA_FONT;
char *ANGBAND_DIR_XTRA_GRAF;
char *ANGBAND_DIR_XTRA_SOUND;
char *ANGBAND_DIR_XTRA_ICON;

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
bool (*item_tester_hook)(const object_type*);



/*
 * Hack -- function hook to restrict "get_mon_num_prep()" function
 */
bool (*get_mon_num_hook)(int r_idx);



/*
 * Hack - the destination file for text_out_to_file.
 */
ang_file *text_out_file = NULL;


/*
 * Hack -- function hook to output (colored) text to the
 * screen or to a file.
 */
void (*text_out_hook)(byte a, const char *str);


/*
 * Hack -- Where to wrap the text when using text_out().  Use the default
 * value (for example the screen width) when 'text_out_wrap' is 0.
 */
int text_out_wrap = 0;


/*
 * Hack -- Indentation for the text when using text_out().
 */
int text_out_indent = 0;

/*
 * Hack -- Padding after wrapping
 */
int text_out_pad = 0;


/*
 * Use transparent tiles
 */
bool use_transparency = FALSE;


/*
 * Sound hook (for playing FX).
 */
void (*sound_hook)(int sound);

/* Delay in centiseconds before moving to allow another keypress */
/* Zero means normal instant movement. */
u16b lazymove_delay = 0;


/* Number of days passed on the current dungeon trip -
  - used for determining store updates on return to town */
u16b daycount = 0;

/*
 * The array used to store stacked monster messages
 */
monster_race_message *mon_msg;

monster_message_history *mon_message_hist;

/*
 * The current size of that array
 */
u16b size_mon_msg;
u16b size_mon_hist;

