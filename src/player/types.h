#ifndef INCLUDED_PLAYER_TYPES_H
#define INCLUDED_PLAYER_TYPES_H

#include "object/obj-flag.h"
#include "object/object.h"
#include "option.h"
#include "ui-event.h"

typedef struct
{
	s16b speed;		/* Current speed */

	s16b num_blows;		/* Number of blows x100 */
	s16b num_shots;		/* Number of shots */

	byte ammo_mult;		/* Ammo multiplier */
	byte ammo_tval;		/* Ammo variety */

	s16b stat_add[A_MAX];	/* Equipment stat bonuses */
	s16b stat_ind[A_MAX];	/* Indexes into stat tables */
	s16b stat_use[A_MAX];	/* Current modified stats */
	s16b stat_top[A_MAX];	/* Maximal modified stats */

	s16b dis_ac;		/* Known base ac */
	s16b ac;			/* Base ac */

	s16b dis_to_a;		/* Known bonus to ac */
	s16b to_a;			/* Bonus to ac */

	s16b to_h;			/* Bonus to hit */
	s16b dis_to_h;		/* Known bonus to hit */

	s16b to_d;			/* Bonus to dam */
	s16b dis_to_d;		/* Known bonus to dam */

	s16b see_infra;		/* Infravision range */

	s16b skills[SKILL_MAX];	/* Skills */

	u32b noise;			/* Derived from stealth */

	bool heavy_wield;	/* Heavy weapon */
	bool heavy_shoot;	/* Heavy shooter */
	bool icky_wield;	/* Icky weapon */

	bool flags[OF_MAX];	/* Status flags from race and items */
} player_state;


/*
 * Most of the "player" information goes here.
 *
 * This stucture gives us a large collection of player variables.
 *
 * This entire structure is wiped when a new character is born.
 *
 * This structure is more or less laid out so that the information
 * which must be saved in the savefile precedes all the information
 * which can be recomputed as needed.
 */
typedef struct player
{
	s16b py;			/* Player location */
	s16b px;			/* Player location */

	byte psex;			/* Sex index */
	byte oops;			/* Unused */

	const struct player_sex *sex;
	const struct player_race *race;
	const struct player_class *class;

	byte hitdie;		/* Hit dice (sides) */
	byte expfact;		/* Experience factor */

	s16b age;			/* Characters age */
	s16b ht;			/* Height */
	s16b wt;			/* Weight */
	s16b sc;			/* Social Class */

	s32b au;			/* Current Gold */

	s16b max_depth;		/* Max depth */
	s16b depth;			/* Cur depth */

	s16b max_lev;		/* Max level */
	s16b lev;			/* Cur level */

	s32b max_exp;		/* Max experience */
	s32b exp;			/* Cur experience */
	u16b exp_frac;		/* Cur exp frac (times 2^16) */

	s16b mhp;			/* Max hit pts */
	s16b chp;			/* Cur hit pts */
	u16b chp_frac;		/* Cur hit frac (times 2^16) */

	s16b msp;			/* Max mana pts */
	s16b csp;			/* Cur mana pts */
	u16b csp_frac;		/* Cur mana frac (times 2^16) */

	s16b stat_max[A_MAX];	/* Current "maximal" stat values */
	s16b stat_cur[A_MAX];	/* Current "natural" stat values */

	s16b timed[TMD_MAX];	/* Timed effects */

	s16b word_recall;	/* Word of recall counter */

	s16b energy;		/* Current energy */

	s16b food;			/* Current nutrition */

	byte confusing;		/* Glowing hands */
	byte searching;		/* Currently searching */
	byte unignoring;	/* Unignoring */

	byte spell_flags[PY_MAX_SPELLS]; /* Spell flags */

	byte spell_order[PY_MAX_SPELLS];	/* Spell order */

	s16b player_hp[PY_MAX_LEVEL];	/* HP Array */

	char died_from[80];		/* Cause of death */
	char *history;

	u16b total_winner;		/* Total winner */
	u16b panic_save;		/* Panic save */

	u16b noscore;			/* Cheating flags */

	bool is_dead;			/* Player is dead */

	bool wizard;			/* Player is in wizard mode */


	/*** Temporary fields ***/

	bool playing;			/* True if player is playing */

	bool leaving;			/* True if player is leaving */

	bool autosave;          /* True if autosave is pending */

	bool create_up_stair;	/* Create up stair on next level */
	bool create_down_stair;	/* Create down stair on next level */

	s32b total_weight;		/* Total weight being carried */

	s16b inven_cnt;			/* Number of items in inventory */
	s16b equip_cnt;			/* Number of items in equipment */

	s16b health_who;		/* Health bar trackee */

	s16b monster_race_idx;	/* Monster race trackee */

	s16b object_idx;    /* Object trackee */
	s16b object_kind_idx;	/* Object kind trackee */

	s16b energy_use;		/* Energy use this turn */

	s16b resting;			/* Resting counter */
	s16b running;			/* Running counter */
	bool running_withpathfind;      /* Are we using the pathfinder ? */
	bool running_firststep;  /* Is this our first step running? */

	s16b run_cur_dir;		/* Direction we are running */
	s16b run_old_dir;		/* Direction we came from */
	bool run_unused;		/* Unused (padding field) */
	bool run_open_area;		/* Looking for an open area */
	bool run_break_right;	/* Looking for a break (right) */
	bool run_break_left;	/* Looking for a break (left) */

	s16b command_arg;		/* Gives argument of current command 
					   (generally a repeat count) */
	s16b command_wrk;		/* Used by the UI to decide whether
					   to start off showing equipment or
					   inventory listings when offering
					   a choice.  See obj/obj-ui.c*/

	s16b new_spells;		/* Number of spells available */

	bool cumber_armor;	/* Mana draining armor */
	bool cumber_glove;	/* Mana draining gloves */

	s16b cur_light;		/* Radius of light (if any) */

	u32b notice;		/* Bit flags for pending "special" actions to 
				   carry out after the current "action", 
				   such as reordering inventory, squelching, 
				   etc. */
	u32b update;		/* Bit flags for recalculations needed after
				   this "action", such as HP, or visible area */
	u32b redraw;	        /* Bit flags for things that /have/ changed,
				   and just need to be redrawn by the UI,
				   such as HP, Speed, etc.*/

	u32b total_energy;	/* Total energy used (including resting) */
	u32b resting_turn;	/* Number of player turns spent resting */

	/* Generation fields (for quick start) */
	s32b au_birth;          /* Birth gold when option birth_money is false */
	s16b stat_birth[A_MAX]; /* Birth "natural" stat values */
	s16b ht_birth;          /* Birth Height */
	s16b wt_birth;          /* Birth Weight */
	s16b sc_birth;		/* Birth social class */

	/* Variable and calculatable player state */
	player_state	state;

	/* "cached" quiver statistics*/
	u16b quiver_size;
	u16b quiver_slots;
	u16b quiver_remainder;

	struct object *inventory;
} player_type;




/*
 * Player sex info
 */
typedef struct player_sex
{
	const char *title;			/* Type of sex */
	const char *winner;		/* Name of winner */
} player_sex;


/*
 * Player racial info
 */
typedef struct player_race
{
	struct player_race *next;
	const char *name;
	
	unsigned int ridx;

	s16b r_adj[A_MAX];	/* Racial stat bonuses */
	
	s16b r_skills[SKILL_MAX];	/* racial skills */
	
	byte r_mhp;			/* Race hit-dice modifier */
	byte r_exp;			/* Race experience factor */
	
	byte b_age;			/* base age */
	byte m_age;			/* mod age */
	
	byte m_b_ht;		/* base height (males) */
	byte m_m_ht;		/* mod height (males) */
	byte m_b_wt;		/* base weight (males) */
	byte m_m_wt;		/* mod weight (males) */
	
	byte f_b_ht;		/* base height (females) */
	byte f_m_ht;		/* mod height (females) */
	byte f_b_wt;		/* base weight (females) */
	byte f_m_wt;		/* mod weight (females) */
	
	byte infra;			/* Infra-vision	range */
	
	byte choice;		/* Legal class choices */
	struct history_chart *history;
	
	bitflag flags[OF_SIZE];   /* Racial (object) flags */
	bitflag pflags[PF_SIZE];  /* Racial (player) flags */
} player_race;

struct start_item
{
	object_kind *kind;
	byte min;	/* Minimum starting amount */
	byte max;	/* Maximum starting amount */

	struct start_item *next;
};


/*
 * A structure to hold class-dependent information on spells.
 */
typedef struct
{
	byte slevel;		/* Required level (to learn) */
	byte smana;			/* Required mana (to cast) */
	byte sfail;			/* Minimum chance of failure */
	byte sexp;			/* Encoded experience bonus */
} magic_type;


/*
 * Information about the player's "magic"
 *
 * Note that a player with a "spell_book" of "zero" is illiterate.
 */
typedef struct
{
	magic_type info[PY_MAX_SPELLS];	/* The available spells */
} player_magic;


/*
 * Player class info
 */
typedef struct player_class
{
	struct player_class *next;
	const char *name;
	unsigned int cidx;
	
	const char *title[10];    /* Titles - offset */
	
	s16b c_adj[A_MAX]; /* Class stat modifier */
	
	s16b c_skills[SKILL_MAX];	/* class skills */
	s16b x_skills[SKILL_MAX];	/* extra skills */
	
	s16b c_mhp;        /* Class hit-dice adjustment */
	s16b c_exp;        /* Class experience factor */
	
	bitflag pflags[PF_SIZE]; /* Class (player) flags */
	
	u16b max_attacks;  /* Maximum possible attacks */
	u16b min_weight;   /* Minimum weapon weight for calculations */
	u16b att_multiply; /* Multiplier for attack calculations */
	
	byte spell_book;   /* Tval of spell books (if any) */
	u16b spell_stat;   /* Stat for spells (if any) */
	u16b spell_first;  /* Level of first spell */
	u16b spell_weight; /* Weight that hurts spells */
	
	u32b sense_base;   /* Base pseudo-id value */
	u16b sense_div;    /* Pseudo-id divisor */
	
	struct start_item *start_items; /**< The starting inventory */
	
	player_magic spells; /* Magic spells */
} player_class;

/* Histories are a graph of charts; each chart contains a set of individual
 * entries for that chart, and each entry contains a text description and a
 * successor chart to move history generation to.
 * For example:
 * 	chart 1 {
 * 		entry {
 * 			desc "You are the illegitimate and unacknowledged child";
 * 			next 2;
 * 		};
 * 		entry {
 * 			desc "You are the illegitimate but acknowledged child";
 * 			next 2;
 * 		};
 * 		entry {
 * 			desc "You are one of several children";
 * 			next 3;
 * 		};
 * 	};
 *
 * History generation works by walking the graph from the starting chart for
 * each race, picking a random entry (with weighted probability) each time.
 */
struct history_entry {
	struct history_entry *next;
	struct history_chart *succ;
	int isucc;
	int roll;
	int bonus;
	char *text;
};

struct history_chart {
	struct history_chart *next;
	struct history_entry *entries;
	unsigned int idx;
};

/*
 * Some more player information
 *
 * This information is retained across player lives
 */
typedef struct
{
	char full_name[32];		/* Full name */
	char base_name[32];		/* Base name */
	
	bool opt[OPT_MAX];		/* Options */
	
	u32b window_flag[ANGBAND_TERM_MAX];	/* Window flags */
	
	byte hitpoint_warn;		/* Hitpoint warning (0 to 9) */
	
	byte delay_factor;		/* Delay factor (0 to 9) */
	
	byte name_suffix;		/* numeric suffix for player name */
} player_other;



#endif /* INCLUDED_PLAYER_TYPES_H */
