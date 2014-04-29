#ifndef INCLUDED_OBJECT_H
#define INCLUDED_OBJECT_H

#include "z-rand.h"
#include "z-quark.h"
#include "z-bitflag.h"
#include "z-dice.h"
#include "obj-properties.h"


/*** Game constants ***/

/*
 * Elements
 */
enum
{
	#define ELEM(a, b, c, d, e, col, f, fh, oh, mh, ph) ELEM_##a,
	#include "list-elements.h"
	#undef ELEM

	ELEM_MAX
};

/*
 * Identify flags
 */
enum
{
	#define ID(a) ID_##a,
	#include "list-identify-flags.h"
	#undef ID
	#define OBJ_MOD(a, b, c, d) ID_##a,
	#include "list-object-modifiers.h"
	#undef OBJ_MOD
	ID_MAX
};

#define ID_SIZE                	FLAG_SIZE(ID_MAX)

#define id_has(f, flag)        	flag_has_dbg(f, ID_SIZE, flag, #f, #flag)
#define id_next(f, flag)       	flag_next(f, ID_SIZE, flag)
#define id_is_empty(f)         	flag_is_empty(f, ID_SIZE)
#define id_is_full(f)          	flag_is_full(f, ID_SIZE)
#define id_is_inter(f1, f2)    	flag_is_inter(f1, f2, ID_SIZE)
#define id_is_subset(f1, f2)   	flag_is_subset(f1, f2, ID_SIZE)
#define id_is_equal(f1, f2)    	flag_is_equal(f1, f2, ID_SIZE)
#define id_on(f, flag)         	flag_on_dbg(f, ID_SIZE, flag, #f, #flag)
#define id_off(f, flag)        	flag_off(f, ID_SIZE, flag)
#define id_wipe(f)             	flag_wipe(f, ID_SIZE)
#define id_setall(f)           	flag_setall(f, ID_SIZE)
#define id_negate(f)           	flag_negate(f, ID_SIZE)
#define id_copy(f1, f2)        	flag_copy(f1, f2, ID_SIZE)
#define id_union(f1, f2)       	flag_union(f1, f2, ID_SIZE)
#define id_comp_union(f1, f2)  	flag_comp_union(f1, f2, ID_SIZE)
#define id_inter(f1, f2)       	flag_inter(f1, f2, ID_SIZE)
#define id_diff(f1, f2)        	flag_diff(f1, f2, ID_SIZE)

/*
 * Refueling constants
 */
#define FUEL_TORCH    5000  /* Maximum amount of fuel in a torch */
#define FUEL_LAMP     15000  /* Maximum amount of fuel in a lantern */
#define DEFAULT_TORCH FUEL_TORCH  /* Default amount of fuel in a torch */
#define DEFAULT_LAMP  (FUEL_LAMP / 2)  /* Default amount of fuel in a lantern */

/* A "stack" of items is limited to 40 items (hard-coded). */
#define MAX_STACK_SIZE 41

/* An item's pval (for charges, amount of gold, etc) is limited to s16b */
#define MAX_PVAL  32767

/*
 * Maximum number of objects allowed in a single dungeon grid.
 *
 * The main-screen has a minimum size of 24 rows, so we can always
 * display 23 objects + 1 header line.
 */
#define MAX_FLOOR_STACK			23


/*** API constants ***/

/* Object origin kinds */

enum {
	ORIGIN_NONE = 0,
	ORIGIN_FLOOR,			/* found on the dungeon floor */
	ORIGIN_DROP,			/* normal monster drops */
	ORIGIN_CHEST,
	ORIGIN_DROP_SPECIAL,	/* from monsters in special rooms */
	ORIGIN_DROP_PIT,		/* from monsters in pits/nests */
	ORIGIN_DROP_VAULT,		/* from monsters in vaults */
	ORIGIN_SPECIAL,			/* on the floor of a special room */
	ORIGIN_PIT,				/* on the floor of a pit/nest */
	ORIGIN_VAULT,			/* on the floor of a vault */
	ORIGIN_LABYRINTH,		/* on the floor of a labyrinth */
	ORIGIN_CAVERN,			/* on the floor of a cavern */
	ORIGIN_RUBBLE,			/* found under rubble */
	ORIGIN_MIXED,			/* stack with mixed origins */
	ORIGIN_STATS,			/* ^ only the above are considered by main-stats */
	ORIGIN_ACQUIRE,			/* called forth by scroll */
	ORIGIN_DROP_BREED,		/* from breeders */
	ORIGIN_DROP_SUMMON,		/* from combat summons */
	ORIGIN_STORE,			/* something you bought */
	ORIGIN_STOLEN,			/* stolen by monster (used only for gold) */
	ORIGIN_BIRTH,			/* objects created at character birth */
	ORIGIN_DROP_UNKNOWN,	/* drops from unseen foes */
	ORIGIN_CHEAT,			/* created by wizard mode */
	ORIGIN_DROP_POLY,		/* from polymorphees */
	ORIGIN_DROP_WIZARD,		/* from wizard mode summons */

	ORIGIN_MAX
};

#define ORIGIN_SIZE FLAG_SIZE(ORIGIN_MAX)
#define ORIGIN_BYTES 4 /* savefile bytes - room for 32 origin types */


/* Maximum number of scroll titles generated */
#define MAX_TITLES	 50

/*
 * Some constants used in randart generation and power calculation
 * - thresholds for limiting to_hit, to_dam and to_ac
 * - fudge factor for rescaling ammo cost
 * (a stack of this many equals a weapon of the same damage output)
 */
#define INHIBIT_POWER		20000
#define INHIBIT_BLOWS		3
#define INHIBIT_MIGHT		4
#define INHIBIT_SHOTS		3
#define HIGH_TO_AC			26
#define VERYHIGH_TO_AC		36
#define INHIBIT_AC			56
#define HIGH_TO_HIT			16
#define VERYHIGH_TO_HIT		26
#define HIGH_TO_DAM			16
#define VERYHIGH_TO_DAM		26
#define AMMO_RESCALER		20 /* this value is also used for torches */

#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/* Values for struct object->marked */
enum {
	MARK_UNAWARE = 0,
	MARK_AWARE = 1,
	MARK_SEEN = 2
};



/*** Macros ***/

/*** Structures ***/

/*
 * And here's the structure for the "fixed" spell information
 */
typedef struct spell {
	struct spell *next;
	unsigned int sidx;
	char *name;
	char *text;

	byte realm;			/* 0 = mage; 1 = priest */
	byte tval;			/* Item type for book this spell is in */
	byte sval;			/* Item sub-type for book (= book number) */
	byte snum;			/* Position of spell within book */

	byte spell_index;	/* Index into player_magic array */
	dice_t *dice;		/* Value information from spell file */
	int params[3];		/* Extra parameters to be passed to the handler */
} spell_type;

/*
 * The spell arrays
 */
extern spell_type *s_info;


enum spell_param_project_type_e {
	SPELL_PROJECT_NONE = 0,
	SPELL_PROJECT_BOLT,
	SPELL_PROJECT_BEAM,
	SPELL_PROJECT_BOLT_OR_BEAM,
	SPELL_PROJECT_BALL,
};


/* Brand type */
struct brand {
	char *name;
	int element;
	int multiplier;
	bool known;
	struct brand *next;
};

/* Slay type */
struct slay {
	char *name;
	int race_flag;
	int multiplier;
	bool known;
	struct slay *next;
};

enum {
	EL_INFO_KNOWN = 0x01,
	EL_INFO_HATES = 0x02,
	EL_INFO_IGNORE = 0x04,
	EL_INFO_NOTABLE = 0x08,
};

/* Element info type */
struct element_info {
	s16b res_level;
	bitflag flags;
};

/**
 * Information about object types, like rods, wands, etc.
 */
typedef struct object_base
{
	char *name;

	int tval;
	struct object_base *next;

	int attr;

	bitflag flags[OF_SIZE];
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */
	struct element_info el_info[ELEM_MAX];

	int break_perc;
} object_base;

extern object_base *kb_info;

/**
 * Information about object kinds, including player knowledge.
 *
 * TODO: split out the user-changeable bits into a separate struct so this
 * one can be read-only.
 */
typedef struct object_kind
{
	char *name;
	char *text;

	object_base *base;

	struct object_kind *next;
	u32b kidx;

	byte tval;					/**< General object type (see TV_ macros) */
	byte sval;					/**< Object sub-type (see SV_ macros) */

	random_value pval;			/* Item extra-parameter */

	random_value to_h;			/**< Bonus to hit */
	random_value to_d;			/**< Bonus to damage */
	random_value to_a;			/**< Bonus to armor */
	s16b ac;					/**< Base armor */

	byte dd;					/**< Damage dice */
	byte ds;					/**< Damage sides */
	s16b weight;				/**< Weight, in 1/10lbs */

	s32b cost;					/**< Object base cost */

	bitflag flags[OF_SIZE];					/**< Flags */
	bitflag kind_flags[KF_SIZE];			/**< Kind flags */

	random_value modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	byte d_attr;			/**< Default object attribute */
	wchar_t d_char;			/**< Default object character */

	int alloc_prob;			/**< Allocation: commonness */
	byte alloc_min;			/**< Highest normal dungeon level */
	byte alloc_max;			/**< Lowest normal dungeon level */
	byte level;				/**< Level (difficulty of activation) */

	u16b effect;			/**< Effect this item produces (effects.c) */
	random_value time;		/**< Recharge time (rods/activation) */
	random_value charge;	/**< Number of charges (staves/wands) */

	byte gen_mult_prob;		/**< Probability of generating more than one */
	random_value stack_size;/**< Number to generate */

	struct flavor *flavor;	/**< Special object flavor (or zero) */


	/** Game-dependent **/

	byte x_attr;	/**< Desired object attribute (set by user/pref file) */
	wchar_t x_char;	/**< Desired object character (set by user/pref file) */

	/** Also saved in savefile **/

	quark_t note; 	/**< Autoinscription quark number */

	bool aware;		/**< Set if player is aware of the kind's effects */
	bool tried;		/**< Set if kind has been tried */

	byte squelch;  	/**< Squelch settings */
	bool everseen; 	/**< Set if kind has ever been seen (to despoilify squelch menus) */

	struct spell *spells;
} object_kind;

extern object_kind *k_info;

/*** Important artifact indexes (see "lib/edit/artifact.txt") ***/

#define ART_POWER			13
#define ART_MORGOTH			34
#define ART_GROND			111

/*
 * Hack -- first "normal" artifact in the artifact list.  All of
 * the artifacts with indexes from 1 to 15 are "special" (lights,
 * rings, amulets), and the ones from 16 to 127 are "normal".
 */
#define ART_MIN_NORMAL		16

/**
 * Information about artifacts.
 *
 * Note that ::cur_num is written to the savefile.
 *
 * TODO: Fix this max_num/cur_num crap and just have a big boolean array of
 * which artifacts have been created and haven't, so this can become read-only.
 */
typedef struct artifact
{
	char *name;
	char *text;

	u32b aidx;

	struct artifact *next;

	byte tval;		/**< General artifact type (see TV_ macros) */
	byte sval;		/**< Artifact sub-type (see SV_ macros) */

	s16b to_h;		/**< Bonus to hit */
	s16b to_d;		/**< Bonus to damage */
	s16b to_a;		/**< Bonus to armor */
	s16b ac;		/**< Base armor */

	byte dd;		/**< Base damage dice */
	byte ds;		/**< Base damage sides */

	s16b weight;	/**< Weight in 1/10lbs */

	s32b cost;		/**< Artifact (pseudo-)worth */

	bitflag flags[OF_SIZE];			/**< Flags */
	bitflag kind_flags[KF_SIZE];	/**< Kind flags */

	int modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	byte level;			/** Difficulty level for activation */

	int alloc_prob;		/** Chance of being generated (i.e. rarity) */
	byte alloc_min;		/** Minimum depth (can appear earlier) */
	byte alloc_max;		/** Maximum depth (will NEVER appear deeper) */

	bool created;		/**< Whether this artifact has been created */
	bool seen;			/**< Whether this artifact has been seen this game */
	bool everseen;		/**< Whether this artifact has ever been seen  */

	u16b effect;		/**< Artifact activation (see effects.c) */
	char *effect_msg;

	random_value time;	/**< Recharge time (if appropriate) */
} artifact_type;

/*
 * The artifact arrays
 */
extern artifact_type *a_info;


/*
 * Number of tval/min-sval/max-sval slots per ego item
 */
#define EGO_TVALS_MAX 3

/*
 * Information about "ego-items".
 */
typedef struct ego_item
{
	struct ego_item *next;

	char *name;
	char *text;

	u32b eidx;

	s32b cost;						/* Ego-item "cost" */

	bitflag flags[OF_SIZE];			/**< Flags */
	bitflag kind_flags[KF_SIZE];	/**< Kind flags */

	random_value modifiers[OBJ_MOD_MAX];
	int min_modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	byte level;				/* Minimum level */
	byte rarity;			/* Object rarity */
	byte rating;			/* Level rating boost */
	int alloc_prob; 		/** Chance of being generated (i.e. rarity) */
	byte alloc_min;			/** Minimum depth (can appear earlier) */
	byte alloc_max;			/** Maximum depth (will NEVER appear deeper) */

	byte tval[EGO_TVALS_MAX];		/* Legal tval */
	byte min_sval[EGO_TVALS_MAX];	/* Minimum legal sval */
	byte max_sval[EGO_TVALS_MAX];	/* Maximum legal sval */

	random_value to_h;		/* Extra to-hit bonus */
	random_value to_d;		/* Extra to-dam bonus */
	random_value to_a;		/* Extra to-ac bonus */

	byte min_to_h;			/* Minimum to-hit value */
	byte min_to_d;			/* Minimum to-dam value */
	byte min_to_a;			/* Minimum to-ac value */

	bool everseen;			/* Do not spoil squelch menus */
} ego_item_type;

/*
 * The ego-item arrays
 */
extern ego_item_type *e_info;


/*
 * Object information, for a specific object.
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that "object" records are "copied" on a fairly regular basis,
 * and care must be taken when handling such objects.
 *
 * Note that "object flags" must now be derived from the object kind,
 * the artifact and ego-item indexes, and the two "xtra" fields.
 *
 * Each cave grid points to one (or zero) objects via the "o_idx"
 * field (above).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a "stack" of objects in the same grid.
 *
 * Each monster points to one (or zero) objects via the "hold_o_idx"
 * field (below).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a pile of objects held by the monster.
 *
 * The "held_m_idx" field is used to indicate which monster, if any,
 * is holding the object.  Objects being held have "ix=0" and "iy=0".
 */
typedef struct object
{
	struct object_kind *kind;
	struct ego_item *ego;
	struct artifact *artifact;

	byte iy;			/* Y-position on map, or zero */
	byte ix;			/* X-position on map, or zero */

	byte tval;			/* Item type (from kind) */
	byte sval;			/* Item sub-type (from kind) */

	s16b pval;			/* Item extra-parameter */

	s16b weight;		/* Item weight */

	bitflag flags[OF_SIZE];			/**< Flags */
	bitflag known_flags[OF_SIZE];	/**< Player-known flags */
	u16b ident;						/* Special flags */
	bitflag id_flags[ID_SIZE];		/**< Object property ID flags */

	s16b modifiers[OBJ_MOD_MAX];
	struct element_info el_info[ELEM_MAX];

	struct brand *brands;
	struct slay *slays;

	s16b ac;			/* Normal AC */
	s16b to_a;			/* Plusses to AC */
	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */

	byte dd, ds;		/* Damage dice/sides */

	s16b timeout;		/* Timeout Counter */

	byte number;		/* Number of items */
	byte marked;		/* Object is marked */
	byte ignore;		/* Object is ignored */

	s16b next_o_idx;	/* Next object in stack (if any) */
	s16b held_m_idx;	/* Monster holding us (if any) */
	s16b mimicking_m_idx; /* Monster mimicking us (if any) */

	byte origin;		/* How this item was found */
	byte origin_depth;  /* What depth the item was found at */
	u16b origin_xtra;   /* Extra information about origin */

	quark_t note; 		/* Inscription index */
} object_type;

struct flavor
{
	char *text;
	struct flavor *next;
	unsigned int fidx;

	byte tval;	  /* Associated object type */
	byte sval;	  /* Associated object sub-type */

	byte d_attr;	/* Default flavor attribute */
	wchar_t d_char;	/* Default flavor character */

	byte x_attr;	/* Desired flavor attribute */
	wchar_t x_char;	/* Desired flavor character */
};

extern struct flavor *flavors;


typedef bool (*item_tester)(const struct object *);


/*** Functions ***/

/* obj-power.c and randart.c */
s32b object_power(const object_type *o_ptr, int verbose, ang_file *log_file, bool known);
char *artifact_gen_name(struct artifact *a, const char ***wordlist);
errr do_randart(u32b randart_seed, bool full);

#endif /* !INCLUDED_OBJECT_H */
