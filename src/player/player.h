/* player/player.h - player interface */

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include "guid.h"
#include "object/obj-flag.h"
#include "object/object.h"
#include "option.h"


/*** Game constants ***/

#define PY_MAX_EXP	99999999L	/* Maximum exp */
#define PY_MAX_GOLD	999999999L	/* Maximum gold */
#define PY_MAX_LEVEL	50		/* Maximum level */

/* Player "food" values */
#define PY_FOOD_MAX 	17000	/* Food value (Bloated) */
#define PY_FOOD_FULL	10000	/* Food value (Normal) */
#define PY_FOOD_ALERT	2000	/* Food value (Hungry) */
#define PY_FOOD_WEAK	1000	/* Food value (Weak) */
#define PY_FOOD_FAINT	500	/* Food value (Fainting) */
#define PY_FOOD_STARVE	100	/* Food value (Starving) */

/* Player regeneration constants */
#define PY_REGEN_NORMAL		197	/* Regen factor*2^16 when full */
#define PY_REGEN_WEAK		98	/* Regen factor*2^16 when weak */
#define PY_REGEN_FAINT		33	/* Regen factor*2^16 when fainting */
#define PY_REGEN_HPBASE		1442	/* Min amount hp regen*2^16 */
#define PY_REGEN_MNBASE		524	/* Min amount mana regen*2^16 */

/* Maximum number of player spells */
#define PY_MAX_SPELLS 64

/* Flags for player.spell_flags[] */
#define PY_SPELL_LEARNED    0x01 /* Spell has been learned */
#define PY_SPELL_WORKED     0x02 /* Spell has been successfully tried */
#define PY_SPELL_FORGOTTEN  0x04 /* Spell has been forgotten */

/** Inventory **/

/*
 * Maximum number of "normal" pack slots, and the index of the "overflow"
 * slot, which can hold an item, but only temporarily, since it causes the
 * pack to "overflow", dropping the "last" item onto the ground.  Since this
 * value is used as an actual slot, it must be less than "INVEN_WIELD" (below).
 * Note that "INVEN_PACK" is probably hard-coded by its use in savefiles, and
 * by the fact that the screen can only show 23 items plus a one-line prompt.
 */
#define INVEN_PACK        23

/*
 * Like the previous but takes into account the (variably full quiver).
 */
#define INVEN_MAX_PACK  (INVEN_PACK - p_ptr->quiver_slots)

/*
 * Indexes used for various "equipment" slots (hard-coded by savefiles, etc).
 */
#define INVEN_WIELD	24
#define INVEN_BOW       25
#define INVEN_LEFT      26
#define INVEN_RIGHT     27
#define INVEN_NECK      28
#define INVEN_LIGHT     29
#define INVEN_BODY      30
#define INVEN_OUTER     31
#define INVEN_ARM       32
#define INVEN_HEAD      33
#define INVEN_HANDS     34
#define INVEN_FEET      35

/*
 * Total number of inventory slots (hard-coded).
 */
#define INVEN_TOTAL	36


/* Quiver */
#define QUIVER_START 	37
#define QUIVER_SIZE  	10
#define QUIVER_END   	47

#define ALL_INVEN_TOTAL 47
/* Since no item index can have this value, use it to mean
 * "no object", so that 0 can be a valid index. */
#define NO_OBJECT		(ALL_INVEN_TOTAL+1)


/** Sexes **/

/* Maximum number of player "sex" types (see "table.c", etc) */
#define MAX_SEXES            3

/* Player sex constants (hard-coded by save-files, arrays, etc) */
#define SEX_FEMALE		0
#define SEX_MALE		1
#define SEX_NEUTER		2

/*
 * Timed effects
 */
enum
{
	TMD_FAST = 0,
	TMD_SLOW,
	TMD_BLIND, 
	TMD_PARALYZED,
	TMD_CONFUSED,
	TMD_AFRAID,
	TMD_IMAGE,
	TMD_POISONED,
	TMD_CUT,
	TMD_STUN,
	TMD_PROTEVIL,
	TMD_INVULN,
	TMD_HERO,
	TMD_SHERO,
	TMD_SHIELD,
	TMD_BLESSED,
	TMD_SINVIS,
	TMD_SINFRA,
	TMD_OPP_ACID,
	TMD_OPP_ELEC,
	TMD_OPP_FIRE,
	TMD_OPP_COLD,
	TMD_OPP_POIS,
	TMD_OPP_CONF,
	TMD_AMNESIA,
	TMD_TELEPATHY,
	TMD_STONESKIN,
	TMD_TERROR,
	TMD_SPRINT,
	TMD_BOLD,

	TMD_MAX
};

/*
 * Skill indexes
 */
enum
{
	SKILL_DISARM,			/* Skill: Disarming */
	SKILL_DEVICE,			/* Skill: Magic Devices */
	SKILL_SAVE,			/* Skill: Saving throw */
	SKILL_STEALTH,			/* Skill: Stealth factor */
	SKILL_SEARCH,			/* Skill: Searching ability */
	SKILL_SEARCH_FREQUENCY,		/* Skill: Searching frequency */
	SKILL_TO_HIT_MELEE,		/* Skill: To hit (normal) */
	SKILL_TO_HIT_BOW,		/* Skill: To hit (shooting) */
	SKILL_TO_HIT_THROW,		/* Skill: To hit (throwing) */
	SKILL_DIGGING,			/* Skill: Digging */

	SKILL_MAX
};

/*
 * Indexes of the various "stats" (hard-coded by savefiles, etc).
 */
enum
{
	A_STR = 0,
	A_INT,
	A_WIS,
	A_DEX,
	A_CON,

	A_MAX
};


/*
 * Player race and class flags
 */
enum
{
	#define PF(a,b) PF_##a,
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

#define player_has(flag)       (pf_has(p_ptr->race->pflags, (flag)) || pf_has(p_ptr->class->pflags, (flag)))


/* player_type.noscore flags */
#define NOSCORE_WIZARD		0x0002
#define NOSCORE_DEBUG		0x0008
#define NOSCORE_BORG		0x0010


/*** Structures ***/

/*
 * All the variable state that changes when you put on/take off equipment.
 */
typedef struct player_state {
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
typedef struct player {
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
	s16b deep_descent;	/* Deep Descent counter */

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

	struct monster *health_who;		/* Health bar trackee */

	struct monster_race *monster_race;	/* Monster race trackee */

	s16b object_idx;    /* Object trackee */
	struct object_kind *object_kind;	/* Object kind trackee */

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
typedef struct player_sex {
	const char *title;		/* Type of sex */
	const char *winner;		/* Name of winner */
} player_sex;


/*
 * Player racial info
 */
struct player_race {
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
};


/*
 * Items the player starts with.  Used in player_class and specified in
 * p_class.txt.
 */
struct start_item {
	object_kind *kind;
	byte min;	/* Minimum starting amount */
	byte max;	/* Maximum starting amount */

	struct start_item *next;
};


/*
 * A structure to hold class-dependent information on spells.
 */
typedef struct {
	byte slevel;		/* Required level (to learn) */
	byte smana;			/* Required mana (to cast) */
	byte sfail;			/* Minimum chance of failure */
	byte sexp;			/* Encoded experience bonus */
} magic_type;


/*
 * Information about the player's magic knowledge
 */
typedef struct {
	magic_type info[PY_MAX_SPELLS];	/* The available spells */
} player_magic;


/*
 * Player class info
 */
typedef struct player_class {
	struct player_class *next;
	const char *name;
	unsigned int cidx;
	
	const char *title[10];    /* Titles */
	
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
	
	struct start_item *start_items; /* Starting inventory */
	
	player_magic spells; /* Magic spells */
} player_class;

/*
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

/*
 * Some more player information
 * This information is retained across player lives
 *
 * XXX - options.c should store most of this, and struct player the rest
 */
typedef struct {
	char full_name[32];		/* Full name */
	
	bool opt[OPT_MAX];		/* Options */
	
	u32b window_flag[ANGBAND_TERM_MAX];	/* Window flags */
	
	byte hitpoint_warn;		/* Hitpoint warning (0 to 9) */
	
	byte delay_factor;		/* Delay factor (0 to 9) */
	
	byte name_suffix;		/* numeric suffix for player name */
} player_other;


/*** Externs ***/

/*
 * The range of possible indexes into tables based upon stats.
 * Currently things range from 3 to 18/220 = 40.
 */
#define STAT_RANGE 38

/* calcs.c */
extern const byte adj_str_blow[STAT_RANGE];
extern const byte adj_dex_safe[STAT_RANGE];
extern const byte adj_con_fix[STAT_RANGE];
extern const byte adj_str_hold[STAT_RANGE];
#ifdef ALLOW_BORG
extern const byte adj_int_dev[STAT_RANGE];
extern const byte adj_wis_sav[STAT_RANGE];
extern const byte adj_dex_dis[STAT_RANGE];
extern const byte adj_int_dis[STAT_RANGE];
extern const byte adj_dex_ta[STAT_RANGE];
extern const byte adj_str_td[STAT_RANGE];
extern const byte adj_dex_th[STAT_RANGE];
extern const byte adj_str_th[STAT_RANGE];
extern const byte adj_str_wgt[STAT_RANGE];
extern const byte adj_str_dig[STAT_RANGE];
extern const byte adj_dex_blow[STAT_RANGE];
extern const int adj_con_mhp[STAT_RANGE];
extern const int adj_mag_study[STAT_RANGE];
extern const int adj_mag_mana[STAT_RANGE];
extern const byte blows_table[12][12];
#endif

void calc_bonuses(object_type inventory[], player_state *state, bool id_only);
int calc_blows(const object_type *o_ptr, player_state *state, int extra_blows);
void notice_stuff(struct player *p);
void update_stuff(struct player *p);
void redraw_stuff(struct player *p);
void handle_stuff(struct player *p);
int weight_remaining(void);

/* class.c */
extern struct player_class *player_id2class(guid id);

/* player.c */

/** XXX These do not belong here **/
extern void health_track(struct player *p, struct monster *m_ptr);
extern void monster_race_track(struct monster_race *race);
extern void track_object(int item);
extern void track_object_kind(struct object_kind *kind);
extern bool tracked_object_is(int item);



extern bool player_stat_inc(struct player *p, int stat);
extern bool player_stat_dec(struct player *p, int stat, bool permanent);
extern void player_exp_gain(struct player *p, s32b amount);
extern void player_exp_lose(struct player *p, s32b amount, bool permanent);

extern byte player_hp_attr(struct player *p);
extern byte player_sp_attr(struct player *p);

extern bool player_restore_mana(struct player *p, int amt);

extern const char *player_safe_name(struct player *p, bool strip_suffix);

/* race.c */
extern struct player_race *player_id2race(guid id);

/* spell.c */
#ifdef ALLOW_BORG
extern const byte adj_mag_fail[STAT_RANGE];
extern const int adj_mag_stat[STAT_RANGE];
#endif
int spell_collect_from_book(const object_type *o_ptr, int *spells);
int spell_book_count_spells(const object_type *o_ptr, bool (*tester)(int spell));
bool spell_okay_list(bool (*spell_test)(int spell), const int spells[], int n_spells);
bool spell_okay_to_cast(int spell);
bool spell_okay_to_study(int spell);
bool spell_okay_to_browse(int spell);
bool spell_in_book(int spell, int book);
s16b spell_chance(int spell);
void spell_learn(int spell);
bool spell_cast(int spell, int dir);

/* timed.c */
bool player_set_timed(struct player *p, int idx, int v, bool notify);
bool player_inc_timed(struct player *p, int idx, int v, bool notify, bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify);
bool player_clear_timed(struct player *p, int idx, bool notify);
bool player_set_food(struct player *p, int v);

/* util.c */
s16b modify_stat_value(int value, int amount);
bool player_can_cast(struct player *p, bool show_msg);
bool player_can_study(struct player *p, bool show_msg);
bool player_can_read(struct player *p, bool show_msg);
bool player_can_fire(struct player *p, bool show_msg);
bool player_can_refuel(struct player *p, bool show_msg);
bool player_can_cast_prereq(void);
bool player_can_study_prereq(void);
bool player_can_read_prereq(void);
bool player_can_fire_prereq(void);
bool player_can_refuel_prereq(void);
bool player_book_has_unlearned_spells(struct player *p);
bool player_confuse_dir(struct player *p, int *dir, bool too);
bool player_resting_is_special(s16b count);
bool player_is_resting(struct player *p);
s16b player_resting_count(struct player *p);
void player_resting_set_count(struct player *p, s16b count);
void player_resting_cancel(struct player *p);
bool player_resting_can_regenerate(struct player *p);
void player_resting_step_turn(struct player *p);
void player_resting_complete_special(struct player *p);

#endif /* !PLAYER_PLAYER_H */
