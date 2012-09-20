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
typedef struct _auto_item auto_item;
typedef struct _auto_shop auto_shop;




/*
 * A structure holding information about an object.  120 bytes.
 *
 * The "iqty" is zero if the object is "missing"
 * The "kind" is zero if the object is "unaware" (or missing)
 * The "able" is zero if the object is "unknown" (or unaware or missing)
 *
 * Note that unaware items will have a "tval" but an invalid "sval".
 */
struct _auto_item {

  char desc[80];	/* Actual Description		*/

  cptr note;		/* Pointer to tail of 'desc'	*/

  s32b cost;		/* Announced cost		*/

  s16b kind;		/* Kind index			*/

  bool okay;		/* True if something		*/  
  bool able;		/* True if item is identified	*/

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
  
  bool junk;		/* Item should be trashed 	*/
  bool cash;		/* Item should be bought/sold	*/
  bool test;		/* Item should be identified	*/
  bool wear;		/* Item should be worn/wielded	*/
};


/*
 * A store
 */
struct _auto_shop {

  int visit;		/* Number of visits */
  int extra;		/* Something unused */

  int page;		/* Current page */
  int more;		/* Number of pages */

  auto_item ware[24];	/* Store contents */
};



/*
 * Spell status values
 */

#define AUTO_SPELL_ICKY		0	/* Spell is illegible */
#define AUTO_SPELL_LOST		1	/* Spell is forgotten */
#define AUTO_SPELL_HIGH		2	/* Spell is high level */
#define AUTO_SPELL_OKAY		3	/* Spell is learnable */
#define AUTO_SPELL_TEST		4	/* Spell is untried */
#define AUTO_SPELL_KNOW		5	/* Spell is known */



/*
 * Forward declare
 */
typedef struct _auto_spell auto_spell;


/*
 * A spell in a book
 */
struct _auto_spell {

    byte status;	/* See AUTO_SPELL_xxx */

    byte index;		/* Actual "spell index" (or 99) */

    byte level;		/* Required level */
    byte power;		/* Required power */
};



/*
 * Some variables
 */

extern auto_item *auto_items;		/* Current "inventory" */

extern auto_shop *auto_shops;		/* Current "shops" */




/*
 * General information
 */
 
extern byte auto_tval_ammo;	/* Tval of usable ammo */


/*
 * Kind indexes of the relevant books
 */

extern s16b kind_book[9];


/*
 * Spell casting information
 */

extern auto_spell auto_spells[9][9];	/* Spell info */




/*
 * Determine which slot an item could be wielded into
 */
extern int borg_wield_slot(auto_item *item);

/*
 * Analyze an item, given a textual description and (optional) cost
 */
extern void borg_item_analyze(auto_item *item, cptr desc, cptr cost);

/*
 * Determine the "value" of an item in a shop-keeper's eyes
 */
extern s32b borg_item_value(auto_item *item);


/*
 * Item checkers
 */
extern bool borg_item_is_armour(auto_item *item);
extern bool borg_item_is_weapon(auto_item *item);


/*
 * Guess how many blows a weapon might get
 */
extern int borg_blows(auto_item *item);


/*
 * Inscribe an object
 */
extern void borg_send_inscribe(int i, cptr str);


/*
 * Find an item slot (by kind)
 */
extern int borg_choose(int k);

/*
 * Perform an action on an item (by kind)
 */
extern bool borg_action(char c, int k);


/*
 * Find an item slot (by tval/sval)
 */
extern int borg_slot(int tval, int sval);

/*
 * Item usage functions (by sval)
 */
extern bool borg_read_scroll(int sval);
extern bool borg_quaff_potion(int sval);
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

/*
 * Prayer functions
 */
extern bool borg_prayer_okay(int book, int what);
extern bool borg_prayer(int book, int what);

/*
 * Study functions
 */
extern bool borg_study_spell_okay(int book, int what);
extern bool borg_study_spell(int book, int what);

/*
 * Study functions
 */
extern bool borg_study_prayer_okay(int book, int what);
extern bool borg_study_prayer(int book, int what);

/*
 * Study functions (general)
 */
extern bool borg_study_any_okay(void);
extern bool borg_study_any(void);



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

