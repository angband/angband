#ifndef INCLUDED_OBJECT_H
#define INCLUDED_OBJECT_H

#include "z-rand.h"
#include "z-file.h"
#include "z-textblock.h"
#include "z-quark.h"
#include "z-bitflag.h"
#include "game-cmd.h"
#include "cave.h"

struct player;

/*** Constants ***/

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


/**
 * Maximum number of pvals on objects
 *
 * Note: all pvals other than DEFAULT_PVAL are assumed to be associated with
 * flags, and any non-flag uses of pval (e.g. chest quality, gold quantity)
 * are assumed to use DEFAULT_PVAL.
 */
#define MAX_PVALS 3
#define DEFAULT_PVAL 0

/* ID flags */
#define IDENT_SENSE     0x0001  /* Has been "sensed" */
#define IDENT_WORN      0x0002  /* Has been tried on */
#define IDENT_EMPTY     0x0004  /* Is known to be empty */
#define IDENT_KNOWN     0x0008  /* Fully known */
#define IDENT_STORE     0x0010  /* Item is in the inventory of a store */
#define IDENT_ATTACK    0x0020  /* Know combat dice/ac/bonuses */
#define IDENT_DEFENCE   0x0040  /* Know AC/etc bonuses */
#define IDENT_EFFECT    0x0080  /* Know item activation/effect */
/* xxx */
#define IDENT_INDESTRUCT 0x0200 /* Tried to destroy it and failed */
#define IDENT_NAME      0x0400  /* Know the name of ego or artifact if there is one */
#define IDENT_FIRED     0x0800  /* Has been used as a missile */
#define IDENT_NOTART    0x1000  /* Item is known not to be an artifact */
#define IDENT_FAKE      0x2000  /* Item is a fake, for displaying knowledge */

/* Whether to learn egos and flavors with less than complete information */
#define EASY_LEARN 1

/* Maximum number of scroll titles generated */
#define MAX_TITLES     50

/**
 * Modes for object_desc().
 */
typedef enum
{
	ODESC_BASE   = 0x00,   /*!< Only describe the base name */
	ODESC_COMBAT = 0x01,   /*!< Also show combat bonuses */
	ODESC_EXTRA  = 0x02,   /*!< Show charges/inscriptions/pvals */

	ODESC_FULL   = ODESC_COMBAT | ODESC_EXTRA,
	                       /*!< Show entire description */

	ODESC_STORE  = 0x04,   /*!< This is an in-store description */
	ODESC_PLURAL = 0x08,   /*!< Always pluralise */
	ODESC_SINGULAR    = 0x10,    /*!< Always singular */
	ODESC_SPOIL  = 0x20,    /*!< Display regardless of player knowledge */
	ODESC_PREFIX = 0x40   /* */
} odesc_detail_t;


/**
 * Modes for item lists in "show_inven()"  "show_equip()" and "show_floor()"
 */
typedef enum
{
	OLIST_NONE   = 0x00,   /* No options */
   	OLIST_WINDOW = 0x01,   /* Display list in a sub-term (left-align) */
   	OLIST_QUIVER = 0x02,   /* Display quiver lines */
   	OLIST_GOLD   = 0x04,   /* Include gold in the list */
	OLIST_WEIGHT = 0x08,   /* Show item weight */
	OLIST_PRICE  = 0x10,   /* Show item price */
	OLIST_FAIL   = 0x20    /* Show device failure */

} olist_detail_t;


/**
 * Modes for object_info()
 */
typedef enum
{
	OINFO_NONE   = 0x00, /* No options */
	OINFO_TERSE  = 0x01, /* Keep descriptions brief, e.g. for dumps */
	OINFO_SUBJ   = 0x02, /* Describe object from the character's POV */
	OINFO_FULL   = 0x04, /* Treat object as if fully IDd */
	OINFO_DUMMY  = 0x08, /* Object does not exist (e.g. knowledge menu) */
	OINFO_EGO    = 0x10, /* Describe ego random powers */
} oinfo_detail_t;


/**
 * Modes for stacking by object_similar()
 */
typedef enum
{
	OSTACK_NONE    = 0x00, /* No options (this does NOT mean no stacking) */
	OSTACK_STORE   = 0x01, /* Store stacking */
	OSTACK_PACK    = 0x02, /* Inventory and home */
	OSTACK_LIST    = 0x04, /* Object list */
	OSTACK_MONSTER = 0x08, /* Monster carrying objects */
	OSTACK_FLOOR   = 0x10, /* Floor stacking */
	OSTACK_QUIVER  = 0x20  /* Quiver */
} object_stack_t;


/**
 * Pseudo-ID markers.
 */
typedef enum
{
	INSCRIP_NULL = 0,            /*!< No pseudo-ID status */
	INSCRIP_STRANGE = 1,         /*!< Item that has mixed combat bonuses */
	INSCRIP_AVERAGE = 2,         /*!< Item with no interesting features */
	INSCRIP_MAGICAL = 3,         /*!< Item with combat bonuses */
	INSCRIP_SPLENDID = 4,        /*!< Obviously good item */
	INSCRIP_EXCELLENT = 5,       /*!< Ego-item */
	INSCRIP_SPECIAL = 6,         /*!< Artifact */
	INSCRIP_UNKNOWN = 7,

	INSCRIP_MAX                  /*!< Maximum number of pseudo-ID markers */
} obj_pseudo_t;


/*
 * Some constants used in randart generation and power calculation
 * - thresholds for limiting to_hit, to_dam and to_ac
 * - fudge factor for rescaling ammo cost
 * (a stack of this many equals a weapon of the same damage output)
 */
#define INHIBIT_POWER       20000
#define HIGH_TO_AC             26
#define VERYHIGH_TO_AC         36
#define INHIBIT_AC             56
#define HIGH_TO_HIT            16
#define VERYHIGH_TO_HIT        26
#define HIGH_TO_DAM            16
#define VERYHIGH_TO_DAM        26
#define AMMO_RESCALER          20 /* this value is also used for torches */

#define sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))


/*** Macros ***/

/*
 * Determine if the attr and char should consider the item's flavor
 *
 * Identified scrolls should use their own tile.
 */
#define use_flavor_glyph(kind) \
    ((kind)->flavor && \
     !((kind)->tval == TV_SCROLL && (kind)->aware))

/*
 * Return the "attr" for a given item kind.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define object_kind_attr(kind) \
    (use_flavor_glyph((kind)) ? \
     ((kind)->flavor->x_attr) : \
     ((kind)->x_attr))

/*
 * Return the "char" for a given item kind.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define object_kind_char(kind) \
    (use_flavor_glyph(kind) ? \
     ((kind)->flavor->x_char) : \
     ((kind)->x_char))

/*
 * Return the "attr" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define object_attr(T) \
    (object_kind_attr((T)->kind))

/*
 * Return the "char" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
#define object_char(T) \
    (object_kind_char((T)->kind))

/*
 * Rings and Amulets
 */
#define object_is_jewelry(T) \
    (((T)->tval == TV_RING) || ((T)->tval == TV_AMULET))


/*** Structures ***/

/**
 * Information about object types, like rods, wands, etc.
 */
typedef struct object_base
{
	char *name;

	int tval;
	struct object_base *next;

	bitflag flags[OF_SIZE];

	int break_perc;
} object_base;


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

	byte tval;         /**< General object type (see TV_ macros) */
	byte sval;         /**< Object sub-type (see SV_ macros) */
	random_value pval[MAX_PVALS]; /**< Power for any flags which need it */
	byte num_pvals;	   /**< Number of pvals in use on this item */

	random_value to_h; /**< Bonus to hit */
	random_value to_d; /**< Bonus to damage */
	random_value to_a; /**< Bonus to armor */
	s16b ac;           /**< Base armor */

	byte dd;           /**< Damage dice */
	byte ds;           /**< Damage sides */
	s16b weight;       /**< Weight, in 1/10lbs */

	s32b cost;         /**< Object base cost */

	bitflag flags[OF_SIZE];			/**< Flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */

	byte d_attr;       /**< Default object attribute */
	char d_char;       /**< Default object character */

	byte alloc_prob;   /**< Allocation: commonness */
	byte alloc_min;    /**< Highest normal dungeon level */
	byte alloc_max;    /**< Lowest normal dungeon level */
	byte level;        /**< Level (difficulty of activation) */

	u16b effect;         /**< Effect this item produces (effects.c) */
	random_value time;   /**< Recharge time (rods/activation) */
	random_value charge; /**< Number of charges (staves/wands) */

	byte gen_mult_prob;      /**< Probability of generating more than one */
	random_value stack_size; /**< Number to generate */

	struct flavor *flavor;         /**< Special object flavor (or zero) */


	/** Game-dependent **/

	byte x_attr;   /**< Desired object attribute (set by user/pref file) */
	char x_char;   /**< Desired object character (set by user/pref file) */

	/** Also saved in savefile **/

	quark_t note; /**< Autoinscription quark number */

	bool aware;    /**< Set if player is aware of the kind's effects */
	bool tried;    /**< Set if kind has been tried */

	byte squelch;  /**< Squelch settings */
	bool everseen; /**< Set if kind has ever been seen (to despoilify squelch menus) */

	struct spell *spells;
} object_kind;



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

	byte tval;    /**< General artifact type (see TV_ macros) */
	byte sval;    /**< Artifact sub-type (see SV_ macros) */
	s16b pval[MAX_PVALS];    /**< Power for any flags which need it */
	byte num_pvals;/**< Number of pvals in use on this item */

	s16b to_h;    /**< Bonus to hit */
	s16b to_d;    /**< Bonus to damage */
	s16b to_a;    /**< Bonus to armor */
	s16b ac;      /**< Base armor */

	byte dd;      /**< Base damage dice */
	byte ds;      /**< Base damage sides */

	s16b weight;  /**< Weight in 1/10lbs */

	s32b cost;    /**< Artifact (pseudo-)worth */

	bitflag flags[OF_SIZE];		/**< Flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */

	byte level;   /** Difficulty level for activation */
	byte rarity;  /** Unused */
	byte alloc_prob; /** Chance of being generated (i.e. rarity) */
	byte alloc_min;  /** Minimum depth (can appear earlier) */
	byte alloc_max;  /** Maximum depth (will NEVER appear deeper) */

	bool created;	/**< Whether this artifact has been created */
	bool seen;	/**< Whether this artifact has been seen this game */
	bool everseen;	/**< Whether this artifact has ever been seen  */

	u16b effect;     /**< Artifact activation (see effects.c) */
	char *effect_msg;

	random_value time;  /**< Recharge time (if appropriate) */

} artifact_type;


/*
 * Information about "ego-items".
 */
typedef struct ego_item
{
	struct ego_item *next;

	char *name;
	char *text;

	u32b eidx;

	s32b cost;			/* Ego-item "cost" */

	bitflag flags[OF_SIZE];		/**< Flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */

	byte level;			/* Minimum level */
	byte rarity;			/* Object rarity */
	byte rating;			/* Level rating boost */

	byte tval[EGO_TVALS_MAX]; 	/* Legal tval */
	byte min_sval[EGO_TVALS_MAX];	/* Minimum legal sval */
	byte max_sval[EGO_TVALS_MAX];	/* Maximum legal sval */

	random_value to_h;     		/* Extra to-hit bonus */
	random_value to_d; 		/* Extra to-dam bonus */
	random_value to_a; 		/* Extra to-ac bonus */
	random_value pval[MAX_PVALS]; 	/* Extra pval bonus */
	byte num_pvals;			/* Number of pvals used */

	byte min_to_h;			/* Minimum to-hit value */
	byte min_to_d;			/* Minimum to-dam value */
	byte min_to_a;			/* Minimum to-ac value */
	byte min_pval[MAX_PVALS];	/* Minimum pval */

	byte xtra;			/* Extra sustain/resist/power */

	bool everseen;			/* Do not spoil squelch menus */
} ego_item_type;



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

	s16b pval[MAX_PVALS];		/* Item extra-parameter */
	byte num_pvals;			/* Number of pvals in use */

	s16b weight;			/* Item weight */

	bitflag flags[OF_SIZE];		/**< Flags */
	bitflag known_flags[OF_SIZE];	/**< Player-known flags */
	bitflag pval_flags[MAX_PVALS][OF_SIZE];	/**< pval flags */
	u16b ident;			/* Special flags */

	s16b ac;			/* Normal AC */
	s16b to_a;			/* Plusses to AC */
	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */

	byte dd, ds;		/* Damage dice/sides */

	s16b timeout;		/* Timeout Counter */

	byte number;		/* Number of items */
	byte marked;		/* Object is marked */
	bool ignore;		/* Object is ignored */

	s16b next_o_idx;	/* Next object in stack (if any) */
	s16b held_m_idx;	/* Monster holding us (if any) */

	byte origin;        /* How this item was found */
	byte origin_depth;  /* What depth the item was found at */
	u16b origin_xtra;   /* Extra information about origin */

	quark_t note; /* Inscription index */
} object_type;

typedef struct flavor {
	char *text;
	struct flavor *next;
	unsigned int fidx;

	byte tval;      /* Associated object type */
	byte sval;      /* Associated object sub-type */

	byte d_attr;    /* Default flavor attribute */
	char d_char;    /* Default flavor character */

	byte x_attr;    /* Desired flavor attribute */
	char x_char;    /* Desired flavor character */
} flavor_type;


/*** Functions ***/

/* identify.c */
extern s32b object_last_wield;

bool object_is_known(const object_type *o_ptr);
bool object_is_known_artifact(const object_type *o_ptr);
bool object_is_known_cursed(const object_type *o_ptr);
bool object_is_known_blessed(const object_type *o_ptr);
bool object_is_known_not_artifact(const object_type *o_ptr);
bool object_is_not_known_consistently(const object_type *o_ptr);
bool object_was_worn(const object_type *o_ptr);
bool object_was_fired(const object_type *o_ptr);
bool object_was_sensed(const object_type *o_ptr);
bool object_flavor_is_aware(const object_type *o_ptr);
bool object_flavor_was_tried(const object_type *o_ptr);
bool object_effect_is_known(const object_type *o_ptr);
bool object_ego_is_visible(const object_type *o_ptr);
bool object_attack_plusses_are_visible(const object_type *o_ptr);
bool object_defence_plusses_are_visible(const object_type *o_ptr);
bool object_flag_is_known(const object_type *o_ptr, int flag);
bool object_high_resist_is_possible(const object_type *o_ptr);
void object_flavor_aware(object_type *o_ptr);
void object_flavor_tried(object_type *o_ptr);
void object_notice_everything(object_type *o_ptr);
void object_notice_indestructible(object_type *o_ptr);
void object_notice_ego(object_type *o_ptr);
void object_notice_sensing(object_type *o_ptr);
void object_sense_artifact(object_type *o_ptr);
void object_notice_effect(object_type *o_ptr);
void object_notice_attack_plusses(object_type *o_ptr);
bool object_notice_flag(object_type *o_ptr, int flag);
bool object_notice_flags(object_type *o_ptr, bitflag flags[OF_SIZE]);
bool object_notice_curses(object_type *o_ptr);
void object_notice_on_defend(struct player *p);
void object_notice_on_wield(object_type *o_ptr);
void object_notice_on_firing(object_type *o_ptr);
void wieldeds_notice_flag(struct player *p, int flag);
void wieldeds_notice_on_attack(void);
void object_repair_knowledge(object_type *o_ptr);
bool object_FA_would_be_obvious(const object_type *o_ptr);
obj_pseudo_t object_pseudo(const object_type *o_ptr);
void sense_inventory(void);
bool easy_know(const object_type *o_ptr);
bool object_check_for_ident(object_type *o_ptr);
bool object_name_is_visible(const object_type *o_ptr);
void object_know_all_flags(object_type *o_ptr);

/* obj-desc.c */
void object_base_name(char *buf, size_t max, int tval, bool plural);
void object_kind_name(char *buf, size_t max, const object_kind *kind, bool easy_know);
size_t object_desc(char *buf, size_t max, const object_type *o_ptr, odesc_detail_t mode);

/* obj-info.c */
textblock *object_info(const object_type *o_ptr, oinfo_detail_t mode);
textblock *object_info_ego(struct ego_item *ego);
void object_info_spoil(ang_file *f, const object_type *o_ptr, int wrap);
void object_info_chardump(ang_file *f, const object_type *o_ptr, int indent, int wrap);

/* obj-make.c */
void free_obj_alloc(void);
bool init_obj_alloc(void);
object_kind *get_obj_num(int level, bool good);
void object_prep(object_type *o_ptr, struct object_kind *kind, int lev, aspect rand_aspect);
s16b apply_magic(object_type *o_ptr, int lev, bool okay, bool good, bool great);
s32b make_object(struct cave *c, object_type *j_ptr, int lev, bool good, bool great);
void make_gold(object_type *j_ptr, int lev, int coin_type);
void copy_artifact_data(object_type *o_ptr, const artifact_type *a_ptr);
void ego_apply_magic(object_type *o_ptr, int level);
void ego_min_pvals(object_type *o_ptr);

/* obj-ui.c */
void show_inven(olist_detail_t mode);
void show_equip(olist_detail_t mode);
void show_floor(const int *floor_list, int floor_num, olist_detail_t mode);
bool verify_item(const char *prompt, int item);
bool get_item(int *cp, const char *pmt, const char *str, cmd_code cmd, int mode);

/* obj-util.c */
struct object_kind *objkind_get(int tval, int sval);
struct object_kind *objkind_byid(int kidx);
void flavor_init(void);
void reset_visuals(bool load_prefs);
void object_flags(const object_type *o_ptr, bitflag flags[OF_SIZE]);
void object_flags_known(const object_type *o_ptr, bitflag flags[OF_SIZE]);
char index_to_label(int i);
s16b label_to_inven(int c);
s16b label_to_equip(int c);
bool wearable_p(const object_type *o_ptr);
s16b wield_slot(const object_type *o_ptr);
bool slot_can_wield_item(int slot, const object_type *o_ptr);
const char *mention_use(int slot);
const char *describe_use(int i);
bool item_tester_okay(const object_type *o_ptr);
int scan_floor(int *items, int max_size, int y, int x, int mode);
void excise_object_idx(int o_idx);
void delete_object_idx(int o_idx);
void delete_object(int y, int x);
void compact_objects(int size);
void wipe_o_list(struct cave *c);
s16b o_pop(void);
object_type *get_first_object(int y, int x);
object_type *get_next_object(const object_type *o_ptr);
bool is_blessed(const object_type *o_ptr);
s32b object_value(const object_type *o_ptr, int qty, int verbose);
s32b object_value_real(const object_type *o_ptr, int qty, int verbose,
    bool known);
bool object_similar(const object_type *o_ptr, const object_type *j_ptr,
	object_stack_t mode);
void object_absorb(object_type *o_ptr, const object_type *j_ptr);
void object_wipe(object_type *o_ptr);
void object_copy(object_type *o_ptr, const object_type *j_ptr);
void object_copy_amt(object_type *dst, object_type *src, int amt);
void object_split(struct object *dest, struct object *src, int amt);
s16b floor_carry(struct cave *c, int y, int x, object_type *j_ptr);
void drop_near(struct cave *c, object_type *j_ptr, int chance, int y, int x,
	bool verbose);
void push_object(int y, int x);
void acquirement(int y1, int x1, int level, int num, bool great);
void inven_item_charges(int item);
void inven_item_describe(int item);
void inven_item_increase(int item, int num);
void save_quiver_size(struct player *p);
void inven_item_optimize(int item);
void floor_item_charges(int item);
void floor_item_describe(int item);
void floor_item_increase(int item, int num);
void floor_item_optimize(int item);
bool inven_carry_okay(const object_type *o_ptr);
bool inven_stack_okay(const object_type *o_ptr);
s16b inven_takeoff(int item, int amt);
void inven_drop(int item, int amt);
void combine_pack(void);
void reorder_pack(void);
void open_quiver_slot(int slot);
void sort_quiver(void);
int get_use_device_chance(const object_type *o_ptr);
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt);
void reduce_charges(object_type *o_ptr, int amt);
int number_charging(const object_type *o_ptr);
bool recharge_timeout(object_type *o_ptr);
unsigned check_for_inscrip(const object_type *o_ptr, const char *inscrip);
object_kind *lookup_kind(int tval, int sval);
int lookup_name(int tval, const char *name);
int lookup_artifact_name(const char *name);
int lookup_sval(int tval, const char *name);
int tval_find_idx(const char *name);
const char *tval_find_name(int tval);
bool obj_is_staff(const object_type *o_ptr);
bool obj_is_wand(const object_type *o_ptr);
bool obj_is_rod(const object_type *o_ptr);
bool obj_is_potion(const object_type *o_ptr);
bool obj_is_scroll(const object_type *o_ptr);
bool obj_is_food(const object_type *o_ptr);
bool obj_is_light(const object_type *o_ptr);
bool obj_is_ring(const object_type *o_ptr);
bool obj_is_ammo(const object_type *o_ptr);
bool obj_has_charges(const object_type *o_ptr);
bool obj_can_zap(const object_type *o_ptr);
bool obj_is_activatable(const object_type *o_ptr);
bool obj_can_activate(const object_type *o_ptr);
bool obj_can_refill(const object_type *o_ptr);
bool obj_can_browse(const object_type *o_ptr);
bool obj_can_cast_from(const object_type *o_ptr);
bool obj_can_study(const object_type *o_ptr);
bool obj_can_takeoff(const object_type *o_ptr);
bool obj_can_wear(const object_type *o_ptr);
bool obj_can_fire(const object_type *o_ptr);
bool obj_has_inscrip(const object_type *o_ptr);
u16b object_effect(const object_type *o_ptr);
object_type *object_from_item_idx(int item);
bool obj_needs_aim(object_type *o_ptr);
bool get_item_okay(int item);
int scan_items(int *item_list, size_t item_list_max, int mode);
bool item_is_available(int item, bool (*tester)(const object_type *), int mode);
extern void display_itemlist(void);
extern void display_object_idx_recall(s16b o_idx);
extern void display_object_kind_recall(s16b k_idx);

bool pack_is_full(void);
bool pack_is_overfull(void);
void pack_overflow(void);

extern struct object *object_byid(s16b oidx);
extern void objects_init(void);
extern void objects_destroy(void);

/* obj-power.c and randart.c */
s32b object_power(const object_type *o_ptr, int verbose, ang_file *log_file, bool known);
char *artifact_gen_name(struct artifact *a, const char ***wordlist);

#endif /* !INCLUDED_OBJECT_H */
