/* player.h - player interface */

#ifndef PLAYER_H
#define PLAYER_H

#include "guid.h"
#include "obj-properties.h"
#include "object.h"
#include "option.h"
#include "player-calcs.h"


/*** Game constants ***/

#define PY_MAX_EXP		99999999L	/* Maximum exp */
#define PY_MAX_LEVEL	50			/* Maximum level */

/* Flags for player.spell_flags[] */
#define PY_SPELL_LEARNED    0x01 	/* Spell has been learned */
#define PY_SPELL_WORKED     0x02 	/* Spell has been successfully tried */
#define PY_SPELL_FORGOTTEN  0x04 	/* Spell has been forgotten */

#define BTH_PLUS_ADJ    	3 		/* Adjust BTH per plus-to-hit */

/** Sexes **/

/* Maximum number of player "sex" types (see "table.c", etc) */
#define MAX_SEXES            3

/* Player sex constants (hard-coded by save-files, arrays, etc) */
#define SEX_FEMALE		0
#define SEX_MALE		1
#define SEX_NEUTER		2

/* Player magic realms */
enum
{
	#define REALM(a, b, c, d, e, f) REALM_##a,
	#include "list-magic-realms.h"
	#undef REALM
	REALM_MAX
};

/* player_type.noscore flags */
#define NOSCORE_WIZARD		0x0002
#define NOSCORE_DEBUG		0x0008
#define NOSCORE_JUMPING     0x0010

extern struct player_body *bodies;


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

	s16b stat_max[STAT_MAX];	/* Current "maximal" stat values */
	s16b stat_cur[STAT_MAX];	/* Current "natural" stat values */

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
	player_state state;
	player_state known_state;

	/* Tracking of various temporary player-related values */
	player_upkeep *upkeep;

	/* Real gear */
	struct object *gear;
	/* Known gear */
	struct object *gear_k;

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

	int r_adj[STAT_MAX];	/* Racial stat bonuses */
	
	int r_skills[SKILL_MAX];	/* racial skills */
	
	int r_mhp;			/* Race hit-dice modifier */
	int r_exp;			/* Race experience factor */
	
	int b_age;			/* base age */
	int m_age;			/* mod age */
	
	int m_b_ht;		/* base height (males) */
	int m_m_ht;		/* mod height (males) */
	int m_b_wt;		/* base weight (males) */
	int m_m_wt;		/* mod weight (males) */
	
	int f_b_ht;		/* base height (females) */
	int f_m_ht;		/* mod height (females) */
	int f_b_wt;		/* base weight (females) */
	int f_m_wt;		/* mod weight (females) */
	
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
	object_kind *kind;
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
typedef struct {
	char *name;
	char *text;

	struct effect *effect;	/**< The spell's effect */

	int sidx;		/**< The index of this spell for this class */
	int bidx;		/**< The index into the player's books array */
	int slevel;	/**< Required level (to learn) */
	int smana;		/**< Required mana (to cast) */
	int sfail;		/**< Base chance of failure */
	int sexp;		/**< Encoded experience bonus */
} class_spell;


/**
 * A structure to hold class-dependent information on spell books.
 */
typedef struct {
	int tval;			/**< Item type of the book */
	int sval;			/**< Item sub-type for book (book number) */
	int realm;			/**< The magic realm of this book */
	int num_spells;	/**< Number of spells in this book */
	class_spell *spells;	/**< Spells in the book*/
} class_book;


/**
 * Information about class magic knowledge
 */
typedef struct {
	int spell_first;		/**< Level of first spell */
	int spell_weight;		/**< Max armour weight to avoid mana penalties */
	struct magic_realm *spell_realm;  		/**< Primary spellcasting realm */
	int num_books;			/**< Number of spellbooks */
	class_book *books;		/**< Details of spellbooks */
	int total_spells;		/**< Number of spells for this class */
} class_magic;


/**
 * Player class info
 */
typedef struct player_class {
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
	
	class_magic magic; /* Magic spells */
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

/* race.c */
extern struct player_race *player_id2race(guid id);

#endif /* !PLAYER_H */
