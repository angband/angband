/* File: borg3.h */
/* Purpose: Header file for "borg3.c" -BEN- */

#ifndef INCLUDED_BORG3_H
#define INCLUDED_BORG3_H

#include "angband.h"
#include "obj-tval.h"
#include "cave.h"
#include "store.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg3.c".
 */

#include "borg1.h"


extern void apw(void);  /* special function used in testing */

/*
 * Hack -- location of the "Lv Mana Fail" prompt
 */
#define ROW_SPELL   1
#define COL_SPELL   20+35




 /*
  * Forward declare
  */
typedef struct borg_item borg_item;
typedef struct borg_shop borg_shop;

enum {
    BORG_CURSE_UNKNOWN = 0,
    BORG_CURSE_VULNERABILITY = 1,
    BORG_CURSE_TELEPORTATION = 2,
    BORG_CURSE_DULLNESS = 3,
    BORG_CURSE_SICKLINESS = 4,
    BORG_CURSE_ENVELOPING = 5,
    BORG_CURSE_IRRITATION = 6,
    BORG_CURSE_WEAKNESS = 7,
    BORG_CURSE_CLUMSINESS = 8,
    BORG_CURSE_SLOWNESS = 9,
    BORG_CURSE_ANNOYANCE = 10,
    BORG_CURSE_POISON = 11,
    BORG_CURSE_SIREN = 12,
    BORG_CURSE_HALLUCINATION = 13,
    BORG_CURSE_PARALYSIS = 14,
    BORG_CURSE_DRAGON_SUMMON = 15,
    BORG_CURSE_DEMON_SUMMON = 16,
    BORG_CURSE_UNDEAD_SUMMON = 17,
    BORG_CURSE_IMPAIR_MANA_RECOVERY = 18,
    BORG_CURSE_IMPAIR_HITPOINT_RECOVERY = 19,
    BORG_CURSE_COWARDICE = 20,
    BORG_CURSE_STONE = 21,
    BORG_CURSE_ANTI_TELEPORTATION = 22,
    BORG_CURSE_TREACHEROUS_WEAPON = 23,
    BORG_CURSE_BURNING_UP = 24,
    BORG_CURSE_CHILLED_TO_THE_BONE = 25,
    BORG_CURSE_STEELSKIN = 26,
    BORG_CURSE_AIR_SWING = 27,
    BORG_CURSE_MAX = 28,
};

/*
 * A structure holding information about an object.  120 bytes.
 *
 * The "iqty" is zero if the object is "missing"
 * The "kind" is zero if the object is "unaware" (or missing)
 * The "able" is zero if the object is "unknown" (or unaware or missing)
 *
 * Note that unaware items will have a "tval" but an invalid "sval".
 */
struct borg_item
{
    char    desc[80];  /* Actual Description */

    char*   note;      /* Pointer to tail of 'desc' */

    uint32_t kind;      /* Kind index */

    bool    ident;      /* True if item is identified */
    bool    needs_ident; /* True if item needs to be identified (not all items have runes that can be identified) */
    bool    aware;		/* Player is aware of the effects */

    bool    xxxx;      /* Unused */

    uint8_t tval;      /* Item type */
    uint8_t sval;      /* Item sub-type */
    int16_t pval;      /* Item extra-info */

    uint8_t iqty;      /* Number of items */

    int16_t weight;    /* Probable weight */

    uint8_t art_idx;     /* Artifact index (if any) */
    uint8_t ego_idx;     /* Ego-item index (if any) */
    int     activ_idx;   /* Activation index (if any) */
    bool    one_ring;  /* is this the one ring */

    int16_t timeout;   /* Timeout counter */

    int16_t to_h;      /* Bonus to hit */
    int16_t to_d;      /* Bonus to dam */
    int16_t to_a;      /* Bonus to ac */
    int16_t ac;        /* Armor class */
    uint8_t dd;        /* Damage dice */
    uint8_t ds;        /* Damage sides */

    uint8_t level;     /* Level  */

    int32_t cost;      /* Cost (in stores) */

    int32_t value;     /* Value (estimated) */

    bool    cursed;     /* Item is cursed */
    bool    uncursable; /* Item can be uncursed */
    bool    curses[BORG_CURSE_MAX];

    bitflag flags[OF_SIZE];	/**< Object flags */
    int16_t modifiers[OBJ_MOD_MAX];	/**< Object modifiers*/
    struct element_info el_info[ELEM_MAX];	/**< Object element info */
    bool    brands[254];			/**< Flag absence/presence of each brand */
    /* HACK this should be dynamic but we don't know when borg_item's go away */
    int     slays[RF_MAX];			/**< power of slays based on race flag */
};


/*
 * A store
 */
struct borg_shop
{
    /*    int16_t when; */      /* Time stamp */

    int16_t     xtra;      /* Something unused */

    int16_t     page;      /* Current page */
    int16_t     more;      /* Number of pages */

    borg_item   ware[24]; /* Store contents */
};



/*
 * Spell method values
 */

#define BORG_MAGIC_ICK      0   /* Spell is illegible */
#define BORG_MAGIC_NOP      1   /* Spell takes no arguments */
#define BORG_MAGIC_EXT      2   /* Spell has DETECTION effects */
#define BORG_MAGIC_AIM      3   /* Spell requires a direction */
#define BORG_MAGIC_OBJ      4   /* Spell requires a pack object */
#define BORG_MAGIC_WHO      5   /* Spell requires a monster symbol */


 /*
  * Spell status values
  */

#define BORG_MAGIC_ICKY     0   /* Spell is illegible */
#define BORG_MAGIC_LOST     1   /* Spell is forgotten */
#define BORG_MAGIC_HIGH     2   /* Spell is high level */
#define BORG_MAGIC_OKAY     3   /* Spell is learnable */
#define BORG_MAGIC_TEST     4   /* Spell is untried */
#define BORG_MAGIC_KNOW     5   /* Spell is known */

enum borg_spells
{
    MAGIC_MISSILE,
    LIGHT_ROOM,
    FIND_TRAPS_DOORS_STAIRS,
    PHASE_DOOR,
    ELECTRIC_ARC,
    DETECT_MONSTERS,
    FIRE_BALL,
    RECHARGING,
    IDENTIFY_RUNE,
    TREASURE_DETECTION,
    FROST_BOLT,
    REVEAL_MONSTERS,
    ACID_SPRAY,
    DISABLE_TRAPS_DESTROY_DOORS,
    TELEPORT_SELF,
    TELEPORT_OTHER,
    RESISTANCE,
    TAP_MAGICAL_ENERGY,
    MANA_CHANNEL,
    DOOR_CREATION,
    MANA_BOLT,
    TELEPORT_LEVEL,
    DETECTION,
    DIMENSION_DOOR,
    THRUST_AWAY,
    SHOCK_WAVE,
    EXPLOSION,
    BANISHMENT,
    MASS_BANISHMENT,
    MANA_STORM,
    DETECT_LIFE,
    FOX_FORM,
    REMOVE_HUNGER,
    STINKING_CLOUD,
    CONFUSE_MONSTER,
    SLOW_MONSTER,
    CURE_POISON,
    RESIST_POISON,
    TURN_STONE_TO_MUD,
    SENSE_SURROUNDINGS,
    LIGHTNING_STRIKE,
    EARTH_RISING,
    TRANCE,
    MASS_SLEEP,
    BECOME_PUKEL_MAN,
    EAGLES_FLIGHT,
    BEAR_FORM,
    TREMOR,
    HASTE_SELF,
    REVITALIZE,
    RAPID_REGENERATION,
    HERBAL_CURING,
    METEOR_SWARM,
    RIFT,
    ICE_STORM,
    VOLCANIC_ERUPTION,
    RIVER_OF_LIGHTNING,
    CALL_LIGHT,
    DETECT_EVIL,
    MINOR_HEALING,
    BLESS,
    SENSE_INVISIBLE,
    HEROISM,
    ORB_OF_DRAINING,
    SPEAR_OF_LIGHT,
    DISPEL_UNDEAD,
    DISPEL_EVIL,
    PROTECTION_FROM_EVIL,
    REMOVE_CURSE,
    PORTAL,
    REMEMBRANCE,
    WORD_OF_RECALL,
    HEALING,
    RESTORATION,
    CLAIRVOYANCE,
    ENCHANT_WEAPON,
    ENCHANT_ARMOUR,
    SMITE_EVIL,
    GLYPH_OF_WARDING,
    DEMON_BANE,
    BANISH_EVIL,
    WORD_OF_DESTRUCTION,
    HOLY_WORD,
    SPEAR_OF_OROME,
    LIGHT_OF_MANWE,
    NETHER_BOLT,
    CREATE_DARKNESS,
    BAT_FORM,
    READ_MINDS,
    TAP_UNLIFE,
    CRUSH,
    SLEEP_EVIL,
    SHADOW_SHIFT,
    DISENCHANT,
    FRIGHTEN,
    VAMPIRE_STRIKE,
    DISPEL_LIFE,
    DARK_SPEAR,
    WARG_FORM,
    BANISH_SPIRITS,
    ANNIHILATE,
    GRONDS_BLOW,
    UNLEASH_CHAOS,
    FUME_OF_MORDOR,
    STORM_OF_DARKNESS,
    POWER_SACRIFICE,
    ZONE_OF_UNMAGIC,
    VAMPIRE_FORM,
    CURSE,
    COMMAND,
    SINGLE_COMBAT,
    OBJECT_DETECTION,
    DETECT_STAIRS,
    HIT_AND_RUN,
    COVER_TRACKS,
    CREATE_ARROWS,
    DECOY,
    BRAND_AMMUNITION,
    SEEK_BATTLE,
    BERSERK_STRENGTH,
    WHIRLWIND_ATTACK,
    SHATTER_STONE,
    LEAP_INTO_BATTLE,
    GRIM_PURPOSE,
    MAIM_FOE,
    HOWL_OF_THE_DAMNED,
    RELENTLESS_TAUNTING,
    VENOM,
    WEREWOLF_FORM,
    BLOODLUST,
    UNHOLY_REPRIEVE,
    FORCEFUL_BLOW,
    QUAKE
};

/*
 * The borgs "usefulness" rating of each spell.
 */
typedef struct borg_spell_rating borg_spell_rating;
struct borg_spell_rating
{
    const char*      name;       /* Textual name */
    uint8_t          rating;     /* Usefulness */
    enum borg_spells spell_enum; /* an enum for quick lookup */
};

/*
 * Forward declare
 */
typedef struct borg_magic borg_magic;


/*
 * A spell/prayer in a book
 */
struct borg_magic
{
    const char* name;      /* Textual name */
    uint8_t     status;    /* Status (see above) */
    uint16_t    effect_index; /* effect index */
    uint8_t     rating;    /* Usefulness */
    uint8_t     level;     /* Required level */
    uint8_t     power;     /* Required power */
    uint8_t     sfail;     /* Minimum chance of failure */
    int         book_offset;  /* offset of this spell in the book it is in */
    int         book;      /* book index */
    int32_t     times;     /* Times this spell was cast */
    enum borg_spells spell_enum;
};



/*
 * Some variables
 */

extern borg_item* borg_items;       /* Current "inventory" */
extern borg_shop* borg_shops;       /* Current "shops" */


/*
 * Safety arrays for simulating possible worlds
 */

extern borg_item* safe_items;       /* Safety "inventory" */
extern borg_item* safe_home;        /* Safety "home" */

extern borg_shop* safe_shops;       /* Safety "shops" */


/*
 * Spell casting information
 */

extern borg_magic* borg_magics;    /* Spell info */




/*
 * Determine which slot an item could be wielded into
 */
extern int borg_wield_slot(const borg_item* item);

/*
 * Analyze an item, given a textual description
 */
extern void borg_item_analyze(borg_item* item, const struct object* real_item, char* desc, bool in_store);


/* look for a *id*'d item */
extern bool borg_object_fully_id(void);

/* look for a *id*'d item */
extern bool borg_object_fully_id_aux(borg_item* borg_item, struct object* real_item);

/*
 * Inscribe an object
 */
extern void borg_send_inscribe(int i, char* str);
extern void borg_send_deinscribe(int i);

/*
 * Find an item with a given tval/sval
 */
extern int borg_slot(int tval, int sval);

/*
 * Item usage functions
 */
enum borg_need {
    BORG_NO_NEED,
    BORG_MET_NEED,
    BORG_UNMET_NEED,
};
extern enum borg_need borg_maintain_light(void);
extern bool borg_refuel_lantern(void);

/*
 * Item usage functions
 */
extern bool borg_obj_has_effect(uint32_t kind, int index, int subtype);
extern bool borg_eat_food(int tval, int sval);
extern bool borg_quaff_crit(bool no_check);
extern bool borg_quaff_potion(int sval);
extern bool borg_eat_unknown(void);
extern bool borg_use_unknown(void);
extern bool borg_quaff_unknown(void);
extern bool borg_read_unknown(void);
extern bool borg_read_scroll(int sval);
extern bool borg_equips_rod(int sval);
extern bool borg_zap_rod(int sval);
extern bool borg_aim_wand(int sval);
extern bool borg_use_staff(int sval);
extern bool borg_use_staff_fail(int sval);
extern bool borg_equips_staff_fail(int sval);
extern bool borg_inscribe_food(void);

extern int borg_activate_failure(int tval, int sval);

/*
 * Artifact usage function (by index)
 */
extern bool borg_activate_item(int activation); /*  */
extern bool borg_equips_item(int activation, bool check_charge);  /*  */
extern bool borg_activate_dragon(int drag_sval); /*  */
extern bool borg_equips_dragon(int drag_sval);  /*  */
extern bool borg_activate_ring(int ring_sval); /*  */
extern bool borg_equips_ring(int ring_sval);  /*  */


/*
 * Spell functions
 */
extern int  borg_spell_stat(void);
extern bool borg_spell_legal(const enum borg_spells spell);
extern bool borg_spell_okay(const enum borg_spells spell);
extern int  borg_get_spell_power(const enum borg_spells spell);
extern int borg_get_book_num(int sval);
extern borg_magic* borg_get_spell_entry(int book, int what);
extern bool borg_spell(const enum borg_spells spell);
extern bool borg_spell_fail(const enum borg_spells spell, int allow_fail);
extern bool borg_spell_okay_fail(const enum borg_spells spell, int allow_fail);
extern bool borg_spell_legal_fail(const enum borg_spells spell, int allow_fail);
extern int  borg_spell_fail_rate(const enum borg_spells spell);


/*
 * Cheat/Parse the "equip" and "inven" screens.
 */
extern void borg_cheat_equip(void);
extern void borg_cheat_inven(void);
extern void borg_cheat_store(void);

/*
 * Cheat/Parse the "spell" screen
 */
extern void borg_cheat_spell(int book);

/*
 * Hack -- prepare stuff based on the race/class
 */
extern void borg_prepare_race_class_info(void);

/*
 * check special note for item needs id
 */
extern const char* borg_get_note(const borg_item* item);
extern bool borg_item_note_needs_id(const borg_item* item);

/*
 * helper to find the first empty slot
 */
extern int borg_first_empty_inventory_slot(void);

/*
 * Initialize this file
 */
extern void borg_init_3(void);
extern void borg_clean_3(void);


#endif

#endif

