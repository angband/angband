/* File: borg-obj.h */

/* Purpose: Header file for "borg-obj.c" -BEN- */

#ifndef INCLUDED_BORG_OBJ_H
#define INCLUDED_BORG_OBJ_H

#include "angband.h"

#ifdef ALLOW_BORG

/*
 * This file provides support for "borg-obj.c".
 */

#include "borg.h"




/*
 * Hack -- location of the "Lv Mana Fail" prompt
 */
#define ROW_SPELL	1
#define COL_SPELL	20+35




/*
 * Forward declare
 */
typedef struct auto_item auto_item;
typedef struct auto_shop auto_shop;




/*
 * A structure holding information about an object.  120 bytes.
 *
 * The "iqty" is zero if the object is "missing"
 * The "kind" is zero if the object is "unaware" (or missing)
 * The "able" is zero if the object is "unknown" (or unaware or missing)
 *
 * Note that unaware items will have a "tval" but an invalid "sval".
 */
struct auto_item {

  char desc[80];	/* Actual Description		*/

  cptr note;		/* Pointer to tail of 'desc'	*/

  s16b kind;		/* Kind index			*/

  bool able;		/* True if item is identified	*/

  bool xxxx;		/* Unused			*/

  byte tval;		/* Item type			*/
  byte sval;		/* Item sub-type		*/
  s16b pval;		/* Item extra-info		*/

  byte discount;	/* Discount			*/
  
  byte iqty;		/* Number of items		*/

  s16b weight;		/* Probable weight		*/

  byte name1;		/* Artifact index (if any)	*/
  byte name2;		/* Ego-item index (if any)	*/

  s16b timeout;		/* Unused			*/
  
  s16b to_h;		/* Bonus to hit			*/
  s16b to_d;		/* Bonus to dam			*/
  s16b to_a;		/* Bonus to ac			*/
  s16b ac;		/* Armor class			*/
  byte dd;		/* Damage dice			*/
  byte ds;		/* Damage sides			*/

  s16b unused;		/* Unused			*/
  
  s32b cost;		/* Cost (in stores)		*/

  s32b value;		/* Value (estimated)		*/

  u32b flags1;		/* Extracted item flags (set 1)	*/
  u32b flags2;		/* Extracted item flags	(set 2)	*/
  u32b flags3;		/* Extracted item flags	(set 3)	*/
};


/*
 * A store
 */
struct auto_shop {

  s16b when;		/* Time stamp */

  s16b xtra;		/* Something unused */

  s16b page;		/* Current page */
  s16b more;		/* Number of pages */

  auto_item ware[24];	/* Store contents */
};


/*
 * Spell method values
 */

#define BORG_MAGIC_ICK		0	/* Spell is illegible */
#define BORG_MAGIC_NOP		1	/* Spell takes no arguments */
#define BORG_MAGIC_EXT		2	/* Spell has "detection" effects */
#define BORG_MAGIC_AIM		3	/* Spell requires a direction */
#define BORG_MAGIC_OBJ		4	/* Spell requires a pack object */
#define BORG_MAGIC_WHO		5	/* Spell requires a monster symbol */


/*
 * Spell status values
 */

#define BORG_MAGIC_ICKY		0	/* Spell is illegible */
#define BORG_MAGIC_LOST		1	/* Spell is forgotten */
#define BORG_MAGIC_HIGH		2	/* Spell is high level */
#define BORG_MAGIC_OKAY		3	/* Spell is learnable */
#define BORG_MAGIC_TEST		4	/* Spell is untried */
#define BORG_MAGIC_KNOW		5	/* Spell is known */



/*
 * Forward declare
 */
typedef struct auto_magic auto_magic;


/*
 * A spell/prayer in a book
 */
struct auto_magic {

    cptr name;		/* Textual spell name */

    byte status;	/* See BORG_MAGIC_xxx */

    byte method;	/* See BORG_MAGIC_xxx */

    byte level;		/* Required level */
    byte power;		/* Required power */

    byte cheat;		/* Actual "spell index" (or 99) */
};



/*
 * Some variables
 */

extern auto_item *auto_items;		/* Current "inventory" */

extern auto_shop *auto_shops;		/* Current "shops" */



/*
 * Spell casting information
 */

extern auto_magic auto_magics[9][9];	/* Spell info */




/*
 * Determine which slot an item could be wielded into
 */
extern int borg_wield_slot(auto_item *item);

/*
 * Analyze an item, given a textual description
 */
extern void borg_item_analyze(auto_item *item, cptr desc);


/*
 * Inscribe an object
 */
extern void borg_send_inscribe(int i, cptr str);


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
extern bool borg_quaff_potion(int sval);
extern bool borg_read_scroll(int sval);
extern bool borg_zap_rod(int sval);
extern bool borg_aim_wand(int sval);
extern bool borg_use_staff(int sval);


/*
 * Book functions
 */
extern int borg_book(int book);

/*
 * Spell functions
 */
extern bool borg_spell_okay(int book, int what);
extern bool borg_spell(int book, int what);
extern bool borg_spell_safe(int book, int what);

/*
 * Prayer functions
 */
extern bool borg_prayer_okay(int book, int what);
extern bool borg_prayer(int book, int what);
extern bool borg_prayer_safe(int book, int what);

/*
 * Study functions
 */
extern bool borg_study_spell(int book, int what);
extern bool borg_study_prayer(int book, int what);
extern bool borg_study_okay(void);



/*
 * Cheat/Parse the "equip" and "inven" screens.
 */
extern void borg_cheat_equip(void);
extern void borg_cheat_inven(void);
extern void borg_parse_equip(void);
extern void borg_parse_inven(void);

/*
 * Cheat/Parse the "spell" screen
 */
extern void borg_cheat_spell(int book);
extern void borg_parse_spell(int book);

/*
 * Hack -- prepare stuff based on the race/class
 */
extern void prepare_race_class_info(void);

/*
 * Init this file
 */
extern void borg_obj_init(void);


#endif

#endif

