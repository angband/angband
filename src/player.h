/* player.h - player interface */

#ifndef PLAYER_H
#define PLAYER_H

#include "guid.h"
#include "obj-properties.h"
#include "object.h"
#include "option.h"
#include "player-calcs.h"


/*** Game constants ***/

#define PY_MAX_EXP	99999999L	/* Maximum exp */
#define PY_MAX_GOLD	999999999L	/* Maximum gold */
#define PY_MAX_LEVEL	50		/* Maximum level */

/* Maximum number of player spells */
#define PY_MAX_SPELLS 64

/* Flags for player.spell_flags[] */
#define PY_SPELL_LEARNED    0x01 	/* Spell has been learned */
#define PY_SPELL_WORKED     0x02 	/* Spell has been successfully tried */
#define PY_SPELL_FORGOTTEN  0x04 	/* Spell has been forgotten */

#define BTH_PLUS_ADJ    	3 		/* Adjust BTH per plus-to-hit */

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
#define INVEN_MAX_PACK  (INVEN_PACK - player->upkeep->quiver_slots)

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

#define MAX_GEAR		100


/** Sexes **/

/* Maximum number of player "sex" types (see "table.c", etc) */
#define MAX_SEXES            3

/* Player sex constants (hard-coded by save-files, arrays, etc) */
#define SEX_FEMALE		0
#define SEX_MALE		1
#define SEX_NEUTER		2

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

#define player_has(flag)       (pf_has(player->race->pflags, (flag)) || pf_has(player->class->pflags, (flag)))


/* player_type.noscore flags */
#define NOSCORE_WIZARD		0x0002
#define NOSCORE_DEBUG		0x0008
#define NOSCORE_JUMPING     0x0010

struct equip_slot {
	int type;
	char *name;
	int index;
};

struct player_body {
	int count;
	char *name;
	struct equip_slot *slots;
};


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

	byte spell_flags[PY_MAX_SPELLS]; /* Spell flags */

	byte spell_order[PY_MAX_SPELLS];	/* Spell order */

	s16b player_hp[PY_MAX_LEVEL];	/* HP Array */

	char died_from[80];		/* Cause of death */
	char *history;

	u16b total_winner;		/* Total winner */

	u16b noscore;			/* Cheating flags */

	bool is_dead;			/* Player is dead */

	bool wizard;			/* Player is in wizard mode */

	/* Generation fields (for quick start) */
	s32b au_birth;          /* Birth gold when option birth_money is false */
	s16b stat_birth[A_MAX]; /* Birth "natural" stat values */
	s16b ht_birth;          /* Birth Height */
	s16b wt_birth;          /* Birth Weight */

	/* Variable and calculatable player state */
	player_state state;
	player_state known_state;

	/* Tracking of various temporary player-related values */
	player_upkeep *upkeep;

	struct object *inventory;
	struct object *gear;
	struct player_body body;
} player_type;



/*
 * Player sex info
 */
typedef struct player_sex {
	const char *title;		/* Type of sex */
	const char *winner;		/* Name of winner */
} player_sex;

extern const player_sex sex_info[MAX_SEXES];

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

	struct element_info el_info[ELEM_MAX]; /* Racial resists */
};

extern struct player_race *races;

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

extern struct player_class *classes;

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
	
	byte hitpoint_warn;		/* Hitpoint warning (0 to 9) */
	
	byte delay_factor;		/* Delay factor (0 to 9) */
	
	byte name_suffix;		/* numeric suffix for player name */
} player_other;


/*** Externs ***/

extern const s32b player_exp[PY_MAX_LEVEL];
extern player_other *op_ptr;
extern player_type *player;


/* class.c */
extern struct player_class *player_id2class(guid id);

/* player.c */
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

#endif /* !PLAYER_H */
