/* File: borg3.h */
/* Purpose: Header file for "borg3.c" -BEN- */

#ifndef INCLUDED_BORG3_H
#define INCLUDED_BORG3_H

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"

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

extern bool (*borg_sort_comp)(void* u, void* v, int a, int b);
extern void (*borg_sort_swap)(void* u, void* v, int a, int b);
extern bool borg_sort_comp_hook(void* u, void* v, int a, int b);
extern void borg_sort_swap_hook(void* u, void* v, int a, int b);
extern void borg_sort_aux(void* u, void* v, int p, int q);
extern void borg_sort(void* u, void* v, int n);

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
    char desc[80];  /* Actual Description */

    cptr note;      /* Pointer to tail of 'desc' */

    s16b kind;      /* Kind index */

    bool ident;      /* True if item is identified */
	bool aware;		/* Player is aware of the effects */

    bool fully_identified; /* True if item is fully identified (AJG) */

    bool needs_I;   /* True if item needs to be 'I'd (AJG) */

    bool xxxx;      /* Unused */

    byte tval;      /* Item type */
    byte sval;      /* Item sub-type */
    s16b pval;      /* Item extra-info */

    byte discount;  /* Discount */

    byte iqty;      /* Number of items */

    s16b weight;    /* Probable weight */

    byte name1;     /* Artifact index (if any) */
    byte name2;     /* Ego-item index (if any) */
	byte activation; /* Artifact activation and effects*/

    s16b timeout;   /* Timeout counter */

    s16b to_h;      /* Bonus to hit */
    s16b to_d;      /* Bonus to dam */
    s16b to_a;      /* Bonus to ac */
    s16b ac;        /* Armor class */
    byte dd;        /* Damage dice */
    byte ds;        /* Damage sides */

    byte level;     /* Level  */

    s32b cost;      /* Cost (in stores) */

    s32b value;     /* Value (estimated) */

    bool cursed;    /* Item is cursed */
	bitflag flags[OF_SIZE];		/**< Flags */
};


/*
 * A store
 */
struct borg_shop
{
/*    s16b when; */      /* Time stamp */

    s16b xtra;      /* Something unused */

    s16b page;      /* Current page */
    s16b more;      /* Number of pages */

    borg_item ware[24]; /* Store contents */
};



/*
 * Spell method values
 */

#define BORG_MAGIC_ICK      0   /* Spell is illegible */
#define BORG_MAGIC_NOP      1   /* Spell takes no arguments */
#define BORG_MAGIC_EXT      2   /* Spell has "detection" effects */
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


/*
 * Forward declare
 */
typedef struct borg_magic borg_magic;


/*
 * A spell/prayer in a book
 */
struct borg_magic
{
    cptr name;      /* Textual name */

    byte status;    /* Status (see above) */

    byte method;    /* Method (see above) */

    byte rating;    /* Usefulness */

    byte level;     /* Required level */

    byte power;     /* Required power */

    byte sfail;     /* Minimum chance of failure */

    byte cheat;     /* Actual "spell index" (or 99) */

    s32b times;     /* Times this spell was cast */
};



/*
 * Some variables
 */

extern borg_item *borg_items;       /* Current "inventory" */

extern borg_shop *borg_shops;       /* Current "shops" */


/*
 * Safety arrays for simulating possible worlds
 */

extern borg_item *safe_items;       /* Safety "inventory" */
extern borg_item *safe_home;        /* Safety "home" */

extern borg_shop *safe_shops;       /* Safety "shops" */


/*
 * Spell casting information
 */

extern borg_magic borg_magics[9][9];    /* Spell info */




/*
 * Determine which slot an item could be wielded into
 */
extern int borg_wield_slot(borg_item *item);

/*
 * Analyze an item, given a textual description
 */
extern void borg_item_analyze(borg_item *item, object_type *real_item, cptr desc);


/* look for a *id*'d item */
extern bool borg_object_star_id( void );

/* look for a *id*'d item */
extern bool borg_object_star_id_aux(borg_item *borg_item, object_type *real_item);

/*
 * Inscribe an object
 */
extern void borg_send_inscribe(int i, cptr str);
extern void borg_send_deinscribe(int i);

/*
 * Count the items of a given tval/sval
 */
extern int borg_count(int tval, int sval);

/*
 * Find an item with a given tval/sval
 */
extern int borg_slot(int tval, int sval);

/*
 * Item usage functions
 */
extern bool borg_refuel_torch(void);
extern bool borg_refuel_lantern(void);

/*
 * Item usage functions (by sval)
 */
extern bool borg_eat_food(int sval);
extern bool borg_quaff_crit( bool no_check );
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
extern bool borg_activate_artifact(int activation, int location); /*  */
extern bool borg_equips_artifact(int activation, int location);  /*  */
extern bool borg_activate_dragon(int drag_sval); /*  */
extern bool borg_equips_dragon(int drag_sval);  /*  */
extern bool borg_activate_item(int tval, int sval, bool target);
extern bool borg_equips_item(int tval, int sval);
extern bool borg_activate_ring(int ring_sval); /*  */
extern bool borg_equips_ring(int ring_sval);  /*  */


/*
 * Spell functions
 */
extern bool borg_spell_legal(int book, int what);
extern bool borg_spell_okay(int book, int what);
extern bool borg_spell(int book, int what);
extern bool borg_spell_fail(int book, int what, int allow_fail);
extern bool borg_spell_okay_fail(int book, int what, int allow_fail );
extern bool borg_spell_legal_fail(int book, int what, int allow_fail );
extern int borg_spell_fail_rate(int book, int what);
extern int borg_prayer_fail_rate(int book, int what);

/*
 * Prayer functions
 */
extern bool borg_prayer_legal(int book, int what);
extern bool borg_prayer_okay(int book, int what);
extern bool borg_prayer(int book, int what);
extern bool borg_prayer_fail(int book, int what, int allow_fail);
extern bool borg_prayer_okay_fail(int book, int what, int allow_fail );
extern bool borg_prayer_legal_fail(int book, int what, int allow_fail );



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
extern void prepare_race_class_info(void);

extern void borg_clear_3(void);

/*
 * Initialize this file
 */
extern void borg_init_3(void);


#endif

#endif

