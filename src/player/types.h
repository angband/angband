#ifndef INCLUDED_PLAYER_TYPES_H
#define INCLUDED_PLAYER_TYPES_H

#include "object/obj-flag.h"
#include "object/object.h"
#include "option.h"
#include "ui-event.h"

typedef struct
{
	int16_t speed;		/* Current speed */

	int16_t num_blows;		/* Number of blows x100 */
	int16_t num_shots;		/* Number of shots */

	uint8_t ammo_mult;		/* Ammo multiplier */
	uint8_t ammo_tval;		/* Ammo variety */

	int16_t stat_add[A_MAX];	/* Equipment stat bonuses */
	int16_t stat_ind[A_MAX];	/* Indexes into stat tables */
	int16_t stat_use[A_MAX];	/* Current modified stats */
	int16_t stat_top[A_MAX];	/* Maximal modified stats */

	int16_t dis_ac;		/* Known base ac */
	int16_t ac;			/* Base ac */

	int16_t dis_to_a;		/* Known bonus to ac */
	int16_t to_a;			/* Bonus to ac */

	int16_t to_h;			/* Bonus to hit */
	int16_t dis_to_h;		/* Known bonus to hit */

	int16_t to_d;			/* Bonus to dam */
	int16_t dis_to_d;		/* Known bonus to dam */

	int16_t see_infra;		/* Infravision range */

	int16_t skills[SKILL_MAX];	/* Skills */

	uint32_t noise;			/* Derived from stealth */

	bool heavy_wield;	/* Heavy weapon */
	bool heavy_shoot;	/* Heavy shooter */
	bool icky_wield;	/* Icky weapon shooter */

	bitflag flags[OF_SIZE];	/* Status flags from race and items */
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
	int16_t py;			/* Player location */
	int16_t px;			/* Player location */

	uint8_t psex;			/* Sex index */
	uint8_t oops;			/* Unused */

	const struct player_sex *sex;
	const struct player_race *race;
	const struct player_class *class;

	uint8_t hitdie;		/* Hit dice (sides) */
	uint8_t expfact;		/* Experience factor */

	int16_t age;			/* Characters age */
	int16_t ht;			/* Height */
	int16_t wt;			/* Weight */

	int32_t au;			/* Current Gold */

	int16_t max_depth;		/* Max depth */
	int16_t depth;			/* Cur depth */

	int16_t max_lev;		/* Max level */
	int16_t lev;			/* Cur level */

	int32_t max_exp;		/* Max experience */
	int32_t exp;			/* Cur experience */
	uint16_t exp_frac;		/* Cur exp frac (times 2^16) */

	int16_t mhp;			/* Max hit pts */
	int16_t chp;			/* Cur hit pts */
	uint16_t chp_frac;		/* Cur hit frac (times 2^16) */

	int16_t msp;			/* Max mana pts */
	int16_t csp;			/* Cur mana pts */
	uint16_t csp_frac;		/* Cur mana frac (times 2^16) */

	int16_t stat_max[A_MAX];	/* Current "maximal" stat values */
	int16_t stat_cur[A_MAX];	/* Current "natural" stat values */

	int16_t timed[TMD_MAX];	/* Timed effects */

	int16_t word_recall;	/* Word of recall counter */
	int16_t deep_descent;	/* Deep Descent counter */

	int16_t energy;		/* Current energy */

	int16_t food;			/* Current nutrition */

	uint8_t confusing;		/* Glowing hands */
	uint8_t searching;		/* Currently searching */
	uint8_t unignoring;	/* Unignoring */

	uint8_t spell_flags[PY_MAX_SPELLS]; /* Spell flags */

	uint8_t spell_order[PY_MAX_SPELLS];	/* Spell order */

	int16_t player_hp[PY_MAX_LEVEL];	/* HP Array */

	char died_from[80];		/* Cause of death */
	char *history;

	uint16_t total_winner;		/* Total winner */
	uint16_t panic_save;		/* Panic save */

	uint16_t noscore;			/* Cheating flags */

	bool is_dead;			/* Player is dead */

	bool wizard;			/* Player is in wizard mode */


	/*** Temporary fields ***/

	bool playing;			/* True if player is playing */
	bool leaving;			/* True if player is leaving */
	bool autosave;          /* True if autosave is pending */

	bool create_up_stair;	/* Create up stair on next level */
	bool create_down_stair;	/* Create down stair on next level */

	int32_t total_weight;		/* Total weight being carried */

	int16_t inven_cnt;			/* Number of items in inventory */
	int16_t equip_cnt;			/* Number of items in equipment */

	struct monster *health_who;		/* Health bar trackee */

	struct monster_race *monster_race;	/* Monster race trackee */

	int16_t object_idx;    /* Object trackee */
	int16_t object_kind_idx;	/* Object kind trackee */

	int16_t energy_use;		/* Energy use this turn */

	int16_t resting;			/* Resting counter */
	int16_t running;			/* Running counter */
	bool running_withpathfind;      /* Are we using the pathfinder ? */
	bool running_firststep;  /* Is this our first step running? */

	int16_t run_cur_dir;		/* Direction we are running */
	int16_t run_old_dir;		/* Direction we came from */
	bool run_unused;		/* Unused (padding field) */
	bool run_open_area;		/* Looking for an open area */
	bool run_break_right;	/* Looking for a break (right) */
	bool run_break_left;	/* Looking for a break (left) */

	int16_t command_wrk;		/* Used by the UI to decide whether
					   to start off showing equipment or
					   inventory listings when offering
					   a choice.  See obj/obj-ui.c*/

	int16_t new_spells;		/* Number of spells available */

	bool cumber_armor;	/* Mana draining armor */
	bool cumber_glove;	/* Mana draining gloves */

	int16_t cur_light;		/* Radius of light (if any) */

	uint32_t notice;		/* Bit flags for pending "special" actions to 
				   carry out after the current "action", 
				   such as reordering inventory, squelching, 
				   etc. */
	uint32_t update;		/* Bit flags for recalculations needed after
				   this "action", such as HP, or visible area */
	uint32_t redraw;	        /* Bit flags for things that /have/ changed,
				   and just need to be redrawn by the UI,
				   such as HP, Speed, etc.*/

	uint32_t total_energy;	/* Total energy used (including resting) */
	uint32_t resting_turn;	/* Number of player turns spent resting */

	/* Generation fields (for quick start) */
	int32_t au_birth;          /* Birth gold when option birth_money is false */
	int16_t stat_birth[A_MAX]; /* Birth "natural" stat values */
	int16_t ht_birth;          /* Birth Height */
	int16_t wt_birth;          /* Birth Weight */

	/* Variable and calculatable player state */
	player_state	state;

	/* "cached" quiver statistics*/
	uint16_t quiver_size;
	uint16_t quiver_slots;
	uint16_t quiver_remainder;

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
struct player_race
{
	struct player_race *next;
	const char *name;
	
	unsigned int ridx;

	int16_t r_adj[A_MAX];	/* Racial stat bonuses */
	
	int16_t r_skills[SKILL_MAX];	/* racial skills */
	
	uint8_t r_mhp;			/* Race hit-dice modifier */
	uint8_t r_exp;			/* Race experience factor */
	
	uint8_t b_age;			/* base age */
	uint8_t m_age;			/* mod age */
	
	uint8_t m_b_ht;		/* base height (males) */
	uint8_t m_m_ht;		/* mod height (males) */
	uint8_t m_b_wt;		/* base weight (males) */
	uint8_t m_m_wt;		/* mod weight (males) */
	
	uint8_t f_b_ht;		/* base height (females) */
	uint8_t f_m_ht;		/* mod height (females) */
	uint8_t f_b_wt;		/* base weight (females) */
	uint8_t f_m_wt;		/* mod weight (females) */
	
	uint8_t infra;			/* Infra-vision	range */
	
	uint8_t choice;		/* Legal class choices */
	struct history_chart *history;
	
	bitflag flags[OF_SIZE];   /* Racial (object) flags */
	bitflag pflags[PF_SIZE];  /* Racial (player) flags */
};

struct start_item
{
	object_kind *kind;
	uint8_t min;	/* Minimum starting amount */
	uint8_t max;	/* Maximum starting amount */

	struct start_item *next;
};


/*
 * A structure to hold class-dependent information on spells.
 */
typedef struct
{
	uint8_t slevel;		/* Required level (to learn) */
	uint8_t smana;			/* Required mana (to cast) */
	uint8_t sfail;			/* Minimum chance of failure */
	uint8_t sexp;			/* Encoded experience bonus */
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
	
	int16_t c_adj[A_MAX]; /* Class stat modifier */
	
	int16_t c_skills[SKILL_MAX];	/* class skills */
	int16_t x_skills[SKILL_MAX];	/* extra skills */
	
	int16_t c_mhp;        /* Class hit-dice adjustment */
	int16_t c_exp;        /* Class experience factor */
	
	bitflag pflags[PF_SIZE]; /* Class (player) flags */
	
	uint16_t max_attacks;  /* Maximum possible attacks */
	uint16_t min_weight;   /* Minimum weapon weight for calculations */
	uint16_t att_multiply; /* Multiplier for attack calculations */
	
	uint8_t spell_book;   /* Tval of spell books (if any) */
	uint16_t spell_stat;   /* Stat for spells (if any) */
	uint16_t spell_first;  /* Level of first spell */
	uint16_t spell_weight; /* Weight that hurts spells */
	
	uint32_t sense_base;   /* Base pseudo-id value */
	uint16_t sense_div;    /* Pseudo-id divisor */
	
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
	
	uint32_t window_flag[ANGBAND_TERM_MAX];	/* Window flags */
	
	uint8_t hitpoint_warn;		/* Hitpoint warning (0 to 9) */
	
	uint8_t delay_factor;		/* Delay factor (0 to 9) */
	
	uint8_t name_suffix;		/* numeric suffix for player name */
} player_other;



#endif /* INCLUDED_PLAYER_TYPES_H */
