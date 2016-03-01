/**
 * \file player.h
 * \brief Player implementation
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2011 elly+angband@leptoquark.net. See COPYING.
 * Copyright (c) 2015 Nick McConnell
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

#ifndef PLAYER_H
#define PLAYER_H

#include "guid.h"
#include "obj-properties.h"
#include "object.h"
#include "option.h"


/**
 * Indexes of the various "stats" (hard-coded by savefiles, etc).
 */
enum
{
	#define STAT(a, b, c, d, e, f, g, h) STAT_##a,
	#include "list-stats.h"
	#undef STAT

	STAT_MAX
};


/**
 * Player race and class flags
 */
enum
{
	#define PF(a,b,c) PF_##a,
	#include "list-player-flags.h"
	#undef PF
	PF_MAX
};

#define PF_SIZE                FLAG_SIZE(PF_MAX)

#define pf_has(f, flag)        flag_has_dbg(f, PF_SIZE, flag, #f, #flag)
#define pf_next(f, flag)       flag_next(f, PF_SIZE, flag)
#define pf_is_empty(f)         flag_is_empty(f, PF_SIZE)
#define pf_is_full(f)          flag_is_full(f, PF_SIZE)
#define pf_is_inter(f1, f2)    flag_is_inter(f1, f2, PF_SIZE)
#define pf_is_subset(f1, f2)   flag_is_subset(f1, f2, PF_SIZE)
#define pf_is_equal(f1, f2)    flag_is_equal(f1, f2, PF_SIZE)
#define pf_on(f, flag)         flag_on_dbg(f, PF_SIZE, flag, #f, #flag)
#define pf_off(f, flag)        flag_off(f, PF_SIZE, flag)
#define pf_wipe(f)             flag_wipe(f, PF_SIZE)
#define pf_setall(f)           flag_setall(f, PF_SIZE)
#define pf_negate(f)           flag_negate(f, PF_SIZE)
#define pf_copy(f1, f2)        flag_copy(f1, f2, PF_SIZE)
#define pf_union(f1, f2)       flag_union(f1, f2, PF_SIZE)
#define pf_comp_union(f1, f2)  flag_comp_union(f1, f2, PF_SIZE)
#define pf_inter(f1, f2)       flag_inter(f1, f2, PF_SIZE)
#define pf_diff(f1, f2)        flag_diff(f1, f2, PF_SIZE)

#define player_has(p, flag)       (pf_has(p->race->pflags, (flag)) || pf_has(p->class->pflags, (flag)))


/**
 * The range of possible indexes into tables based upon stats.
 * Currently things range from 3 to 18/220 = 40.
 */
#define STAT_RANGE 38


/**
 * ------------------------------------------------------------------------
 * Player constants
 * ------------------------------------------------------------------------ */


#define PY_MAX_EXP		99999999L	/* Maximum exp */
#define PY_KNOW_LEVEL	30			/* Level to know all runes */
#define PY_MAX_LEVEL	50			/* Maximum level */

/**
 * Flags for player.spell_flags[]
 */
#define PY_SPELL_LEARNED    0x01 	/* Spell has been learned */
#define PY_SPELL_WORKED     0x02 	/* Spell has been successfully tried */
#define PY_SPELL_FORGOTTEN  0x04 	/* Spell has been forgotten */

#define BTH_PLUS_ADJ    	3 		/* Adjust BTH per plus-to-hit */

/**
 * Player magic realms
 */
enum
{
	#define REALM(a, b, c, d, e, f) REALM_##a,
	#include "list-magic-realms.h"
	#undef REALM
	REALM_MAX
};

/**
 * player noscore flags
 */
#define NOSCORE_WIZARD		0x0002
#define NOSCORE_DEBUG		0x0008
#define NOSCORE_JUMPING     0x0010

/**
 * Skill indexes
 */
enum
{
	SKILL_DISARM,			/* Skill: Disarming */
	SKILL_DEVICE,			/* Skill: Magic Devices */
	SKILL_SAVE,				/* Skill: Saving throw */
	SKILL_STEALTH,			/* Skill: Stealth factor */
	SKILL_SEARCH,			/* Skill: Searching ability */
	SKILL_SEARCH_FREQUENCY,	/* Skill: Searching frequency */
	SKILL_TO_HIT_MELEE,		/* Skill: To hit (normal) */
	SKILL_TO_HIT_BOW,		/* Skill: To hit (shooting) */
	SKILL_TO_HIT_THROW,		/* Skill: To hit (throwing) */
	SKILL_DIGGING,			/* Skill: Digging */

	SKILL_MAX
};

/* Terrain that the player has a chance of digging through */
enum
{
	DIGGING_RUBBLE = 0,
	DIGGING_MAGMA,
	DIGGING_QUARTZ,
	DIGGING_GRANITE,
	DIGGING_DOORS,
	
	DIGGING_MAX
};

/**
 * ------------------------------------------------------------------------
 * Player structures
 * ------------------------------------------------------------------------ */

/**
 * Structure for the "quests"
 */
struct quest
{
	struct quest *next;
	byte index;
	char *name;
	byte level;					/* Dungeon level */
	struct monster_race *race;	/* Monster race */
	int cur_num;				/* Number killed (unused) */
	int max_num;				/* Number required (unused) */
};

/**
 * Player body info
 */
struct equip_slot {
	struct equip_slot *next;

	u16b type;
	char *name;
	struct object *obj;
};

struct player_body {
	struct player_body *next;
	char *name;
	u16b count;
	struct equip_slot *slots;
};

extern struct player_body *bodies;

/**
 * Player racial info
 */
struct player_race {
	struct player_race *next;
	const char *name;
	
	unsigned int ridx;

	int r_adj[STAT_MAX];	/* Racial stat bonuses */
	
	int r_skills[SKILL_MAX];	/* racial skills */
	
	int r_mhp;			/* Race hit-dice modifier */
	int r_exp;			/* Race experience factor */
	
	int b_age;			/* base age */
	int m_age;			/* mod age */
	
	int base_hgt;		/* base height */
	int mod_hgt;		/* mod height */
	int base_wgt;		/* base weight */
	int mod_wgt;		/* mod weight */
	
	int infra;			/* Infra-vision	range */
	
	int body;			/* Race body */
	struct history_chart *history;
	
	bitflag flags[OF_SIZE];   /* Racial (object) flags */
	bitflag pflags[PF_SIZE];  /* Racial (player) flags */

	struct element_info el_info[ELEM_MAX]; /* Racial resists */
};

extern struct player_race *races;

/**
 * Items the player starts with.  Used in player_class and specified in
 * class.txt.
 */
struct start_item {
	struct object_kind *kind;
	int min;	/* Minimum starting amount */
	int max;	/* Maximum starting amount */

	struct start_item *next;
};


/**
 * Structure for magic realms
 */
struct magic_realm {
	int index;
	int stat;
	const char *verb;
	const char *spell_noun;
	const char *book_noun;
	const char *adjective;
};

extern struct magic_realm realms[REALM_MAX];

/**
 * A structure to hold class-dependent information on spells.
 */
struct class_spell {
	char *name;
	char *text;

	struct effect *effect;	/**< The spell's effect */

	int sidx;		/**< The index of this spell for this class */
	int bidx;		/**< The index into the player's books array */
	int slevel;	/**< Required level (to learn) */
	int smana;		/**< Required mana (to cast) */
	int sfail;		/**< Base chance of failure */
	int sexp;		/**< Encoded experience bonus */
};


/**
 * A structure to hold class-dependent information on spell books.
 */
struct class_book {
	int tval;			/**< Item type of the book */
	int sval;			/**< Item sub-type for book (book number) */
	int realm;			/**< The magic realm of this book */
	int num_spells;	/**< Number of spells in this book */
	struct class_spell *spells;	/**< Spells in the book*/
};


/**
 * Information about class magic knowledge
 */
struct class_magic {
	int spell_first;		/**< Level of first spell */
	int spell_weight;		/**< Max armour weight to avoid mana penalties */
	struct magic_realm *spell_realm;  		/**< Primary spellcasting realm */
	int num_books;			/**< Number of spellbooks */
	struct class_book *books;		/**< Details of spellbooks */
	int total_spells;		/**< Number of spells for this class */
};


/**
 * Player class info
 */
struct player_class {
	struct player_class *next;
	const char *name;
	unsigned int cidx;
	
	const char *title[10];    /* Titles */
	
	int c_adj[STAT_MAX]; /* Class stat modifier */
	
	int c_skills[SKILL_MAX];	/* class skills */
	int x_skills[SKILL_MAX];	/* extra skills */
	
	int c_mhp;        /* Class hit-dice adjustment */
	int c_exp;        /* Class experience factor */
	
	bitflag pflags[PF_SIZE]; /* Class (player) flags */
	
	int max_attacks;  /* Maximum possible attacks */
	int min_weight;   /* Minimum weapon weight for calculations */
	int att_multiply; /* Multiplier for attack calculations */
	
	int sense_base;   /* Base pseudo-id value */
	int sense_div;    /* Pseudo-id divisor */
	
	struct start_item *start_items; /* Starting inventory */
	
	struct class_magic magic; /* Magic spells */
};

extern struct player_class *classes;

/**
 * Histories are a graph of charts; each chart contains a set of individual
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

/**
 * Some more player information
 * This information is retained across player lives
 *
 * XXX - options.c should store most of this, and struct player the rest
 */
typedef struct {
	char full_name[32];		/* Full name */
	
	bool opt[OPT_MAX];		/* Options */
	
	byte hitpoint_warn;		/* Hitpoint warning (0 to 9) */
	u16b lazymove_delay;	/* Delay in cs before moving to allow another key */
	byte delay_factor;		/* Delay factor (0 to 9) */
	
	byte name_suffix;		/* numeric suffix for player name */
} player_other;


/**
 * All the variable state that changes when you put on/take off equipment.
 * Player flags are not currently variable, but useful here so monsters can
 * learn them.
 */
struct player_state {
	s16b speed;		/* Current speed */

	s16b num_blows;		/* Number of blows x100 */
	s16b num_shots;		/* Number of shots */

	byte ammo_mult;		/* Ammo multiplier */
	byte ammo_tval;		/* Ammo variety */

	s16b stat_add[STAT_MAX];	/* Equipment stat bonuses */
	s16b stat_ind[STAT_MAX];	/* Indexes into stat tables */
	s16b stat_use[STAT_MAX];	/* Current modified stats */
	s16b stat_top[STAT_MAX];	/* Maximal modified stats */

	s16b ac;			/* Base ac */
	s16b to_a;			/* Bonus to ac */
	s16b to_h;			/* Bonus to hit */
	s16b to_d;			/* Bonus to dam */

	s16b see_infra;		/* Infravision range */

	s16b cur_light;		/* Radius of light (if any) */

	s16b skills[SKILL_MAX];	/* Skills */

	int noise;			/* Derived from stealth */

	bool heavy_wield;	/* Heavy weapon */
	bool heavy_shoot;	/* Heavy shooter */
	bool icky_wield;	/* Icky weapon shooter */

	bool cumber_armor;	/* Mana draining armor */
	bool cumber_glove;	/* Mana draining gloves */

	bitflag flags[OF_SIZE];	/* Status flags from race and items */
	bitflag pflags[PF_SIZE];	/* Player intrinsic flags */
	struct element_info el_info[ELEM_MAX]; /* Resists from race and items */
};

/**
 * Temporary, derived, player-related variables used during play but not saved
 *
 * Some of these probably should go to the UI
 */
struct player_upkeep {
	bool playing;			/* True if player is playing */
	bool autosave;			/* True if autosave is pending */
	bool generate_level;	/* True if level needs regenerating */
	bool only_partial;		/* True if only partial updates are needed */
	bool dropping;			/* True if auto-drop is in progress */

	int energy_use;			/* Energy use this turn */
	int new_spells;			/* Number of spells available */

	struct monster *health_who;			/* Health bar trackee */
	struct monster_race *monster_race;	/* Monster race trackee */
	struct object *object;				/* Object trackee */
	struct object_kind *object_kind;	/* Object kind trackee */

	u32b notice;		/* Bit flags for pending actions such as 
						 * reordering inventory, ignoring, etc. */
	u32b update;		/* Bit flags for recalculations needed 
						 * such as HP, or visible area */
	u32b redraw;	    /* Bit flags for things that /have/ changed,
						 * and just need to be redrawn by the UI,
						 * such as HP, Speed, etc.*/

	int command_wrk;		/* Used by the UI to decide whether
							 * to start off showing equipment or
							 * inventory listings when offering
							 * a choice.  See obj-ui.c */

	bool create_up_stair;		/* Create up stair on next level */
	bool create_down_stair;		/* Create down stair on next level */

	int resting;				/* Resting counter */

	int running;				/* Running counter */
	bool running_withpathfind;	/* Are we using the pathfinder ? */
	bool running_firststep;		/* Is this our first step running? */

	struct object **quiver;		/* Quiver objects */
	struct object **inven;		/* Inventory objects */
	int total_weight;			/* Total weight being carried */
	int inven_cnt;				/* Number of items in inventory */
	int equip_cnt;				/* Number of items in equipment */
	int quiver_cnt;				/* Number of items in the quiver */
};


/**
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
struct player {
	s16b py;			/* Player location */
	s16b px;			/* Player location */

	const struct player_race *race;
	const struct player_class *class;

	byte hitdie;		/* Hit dice (sides) */
	byte expfact;		/* Experience factor */

	s16b age;			/* Characters age */
	s16b ht;			/* Height */
	s16b wt;			/* Weight */

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

	s16b stat_max[STAT_MAX];	/* Current "maximal" stat values */
	s16b stat_cur[STAT_MAX];	/* Current "natural" stat values */
	s16b stat_map[STAT_MAX];	/* Tracks remapped stats from temp stat swap */

	s16b *timed;		/* Timed effects */

	s16b word_recall;	/* Word of recall counter */
	s16b deep_descent;	/* Deep Descent counter */

	s16b energy;		/* Current energy */
	u32b total_energy;	/* Total energy used (including resting) */
	u32b resting_turn;	/* Number of player turns spent resting */

	s16b food;			/* Current nutrition */

	byte confusing;		/* Glowing hands */
	byte searching;		/* Currently searching */
	byte unignoring;	/* Unignoring */

	byte *spell_flags; /* Spell flags */
	byte *spell_order;	/* Spell order */

	s16b player_hp[PY_MAX_LEVEL];	/* HP Array */

	char died_from[80];		/* Cause of death */
	char *history;			/* Player history */
	struct quest *quests;	/* Quest history */
	u16b total_winner;		/* Total winner */

	u16b noscore;			/* Cheating flags */

	bool is_dead;			/* Player is dead */

	bool wizard;			/* Player is in wizard mode */

	/* Generation fields (for quick start) */
	s32b au_birth;          /* Birth gold when option birth_money is false */
	s16b stat_birth[STAT_MAX]; /* Birth "natural" stat values */
	s16b ht_birth;          /* Birth Height */
	s16b wt_birth;          /* Birth Weight */

	/* Variable and calculatable player state */
	struct player_state state;
	struct player_state known_state;

	/* Tracking of various temporary player-related values */
	struct player_upkeep *upkeep;

	/* Real gear */
	struct object *gear;
	/* Known gear */
	struct object *gear_k;
	/* Object knowledge ("runes") */
	struct object *obj_k;

	struct player_body body;
};


/**
 * ------------------------------------------------------------------------
 * Externs
 * ------------------------------------------------------------------------ */

extern const s32b player_exp[PY_MAX_LEVEL];
extern player_other *op_ptr;
extern struct player *player;


/* player-class.c */
extern struct player_class *player_id2class(guid id);

/* player.c */
int stat_name_to_idx(const char *name);
const char *stat_idx_to_name(int type);
extern bool player_stat_inc(struct player *p, int stat);
extern bool player_stat_dec(struct player *p, int stat, bool permanent);
extern void player_exp_gain(struct player *p, s32b amount);
extern void player_exp_lose(struct player *p, s32b amount, bool permanent);
extern void player_flags(struct player *p, bitflag f[OF_SIZE]);

extern byte player_hp_attr(struct player *p);
extern byte player_sp_attr(struct player *p);

extern bool player_restore_mana(struct player *p, int amt);

extern const char *player_safe_name(struct player *p, bool strip_suffix);

/* player-race.c */
extern struct player_race *player_id2race(guid id);

#endif /* !PLAYER_H */
