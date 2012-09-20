/* File: borg-obj.c */

/* Purpose: Helper file for "borg-ben.c" -BEN- */

#include "angband.h"


#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-obj.h"



/*
 * See "borg-ben.c" for general information.
 *
 * This file helps the Borg analyze "objects" and "shops", and to
 * deal with objects and spells.
 */



/*
 * Hack -- help analyze the magic
 *
 * The comments make a nice way to look up spells/prayers
 */

static byte auto_magic_method[2][9][9] = {


  /*** Spells ***/

  {
    {
        /* Magic for Beginners (sval 0) */
      BORG_MAGIC_AIM	/* "Magic Missile" */,
      BORG_MAGIC_EXT	/* "Detect Monsters" */,
      BORG_MAGIC_NOP	/* "Phase Door" */,
      BORG_MAGIC_NOP	/* "Light Area" */,
      BORG_MAGIC_NOP	/* "Treasure Detection" */,
      BORG_MAGIC_NOP	/* "Cure Light Wounds" */,
      BORG_MAGIC_NOP	/* "Object Detection" */,
      BORG_MAGIC_NOP	/* "Find Hidden Traps/Doors" */,
      BORG_MAGIC_AIM	/* "Stinking Cloud" */
    },
    
    {
        /* Conjurings and Tricks (sval 1) */
      BORG_MAGIC_AIM	/* "Confusion" */,
      BORG_MAGIC_AIM	/* "Lightning Bolt" */,
      BORG_MAGIC_NOP	/* "Trap/Door Destruction" */,
      BORG_MAGIC_AIM	/* "Sleep I" */,
      BORG_MAGIC_NOP	/* "Cure Poison" */,
      BORG_MAGIC_NOP	/* "Teleport Self" */,
      BORG_MAGIC_AIM	/* "Spear of Light" */,
      BORG_MAGIC_AIM	/* "Frost Bolt" */,
      BORG_MAGIC_AIM	/* "Turn Stone to Mud" */
    },
    
    {
        /* Incantations and Illusions (sval 2) */
      BORG_MAGIC_NOP	/* "Satisfy Hunger" */,
      BORG_MAGIC_OBJ	/* "Recharge Item I" */,
      BORG_MAGIC_NOP	/* "Sleep II" */,
      BORG_MAGIC_AIM	/* "Polymorph Other" */,
      BORG_MAGIC_OBJ	/* "Identify" */,
      BORG_MAGIC_NOP	/* "Sleep III" */,
      BORG_MAGIC_AIM	/* "Fire Bolt" */,
      BORG_MAGIC_AIM	/* "Slow Monster" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Sorcery and Evocations (sval 3) */
      BORG_MAGIC_AIM	/* "Frost Ball" */,
      BORG_MAGIC_OBJ	/* "Recharge Item II" */,
      BORG_MAGIC_AIM	/* "Teleport Other" */,
      BORG_MAGIC_NOP	/* "Haste Self" */,
      BORG_MAGIC_AIM	/* "Fire Ball" */,
      BORG_MAGIC_NOP	/* "Word of Destruction" */,
      BORG_MAGIC_WHO	/* "Genocide" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Resistance of Scarabtarices (sval 4) */
      BORG_MAGIC_NOP	/* "Resist Fire" */,
      BORG_MAGIC_NOP	/* "Resist Cold" */,
      BORG_MAGIC_NOP	/* "Resist Acid" */,
      BORG_MAGIC_NOP	/* "Resist Poison" */,
      BORG_MAGIC_NOP	/* "Resistance" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },

    {
        /* Mordenkainen's Escapes (sval 5) */
      BORG_MAGIC_NOP	/* "Door Creation" */,
      BORG_MAGIC_NOP	/* "Stair Creation" */,
      BORG_MAGIC_NOP	/* "Teleport Level" */,
      BORG_MAGIC_NOP	/* "Earthquake" */,
      BORG_MAGIC_NOP	/* "Word of Recall" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Kelek's Grimoire of Power (sval 6) */
      BORG_MAGIC_EXT	/* "Detect Evil" */,
      BORG_MAGIC_NOP	/* "Detect Enchantment" */,
      BORG_MAGIC_OBJ	/* "Recharge Item III" */,
      BORG_MAGIC_WHO	/* "Genocide" */,
      BORG_MAGIC_NOP	/* "Mass Genocide" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Tenser's transformations... (sval 7) */
      BORG_MAGIC_NOP	/* "Heroism" */,
      BORG_MAGIC_NOP	/* "Shield" */,
      BORG_MAGIC_NOP	/* "Berserker" */,
      BORG_MAGIC_NOP	/* "Essence of Speed" */,
      BORG_MAGIC_NOP	/* "Globe of Invulnerability" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Raal's Tome of Destruction (sval 8) */
      BORG_MAGIC_AIM	/* "Acid Bolt" */,
      BORG_MAGIC_AIM	/* "Cloud Kill" */,
      BORG_MAGIC_AIM	/* "Acid Ball" */,
      BORG_MAGIC_AIM	/* "Ice Storm" */,
      BORG_MAGIC_AIM	/* "Meteor Swarm" */,
      BORG_MAGIC_AIM	/* "Mana Storm" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    }
  },


  /*** Prayers ***/
  
  {
    {
        /* Beginners Handbook (sval 0) */
      BORG_MAGIC_EXT	/* "Detect Evil" */,
      BORG_MAGIC_NOP	/* "Cure Light Wounds" */,
      BORG_MAGIC_NOP	/* "Bless" */,
      BORG_MAGIC_NOP	/* "Remove Fear" */,
      BORG_MAGIC_NOP	/* "Call Light" */,
      BORG_MAGIC_NOP	/* "Find Traps" */,
      BORG_MAGIC_NOP	/* "Detect Doors/Stairs" */,
      BORG_MAGIC_NOP	/* "Slow Poison" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Words of Wisdom (sval 1) */
      BORG_MAGIC_AIM	/* "Confuse Creature" */,
      BORG_MAGIC_NOP	/* "Portal" */,
      BORG_MAGIC_NOP	/* "Cure Serious Wounds" */,
      BORG_MAGIC_NOP	/* "Chant" */,
      BORG_MAGIC_NOP	/* "Sanctuary" */,
      BORG_MAGIC_NOP	/* "Satisfy Hunger" */,
      BORG_MAGIC_NOP	/* "Remove Curse" */,
      BORG_MAGIC_NOP	/* "Resist Heat and Cold" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Chants and Blessings (sval 2) */
      BORG_MAGIC_NOP	/* "Neutralize Poison" */,
      BORG_MAGIC_AIM	/* "Orb of Draining" */,
      BORG_MAGIC_NOP	/* "Cure Critical Wounds" */,
      BORG_MAGIC_EXT	/* "Sense Invisible" */,
      BORG_MAGIC_NOP	/* "Protection from Evil" */,
      BORG_MAGIC_NOP	/* "Earthquake" */,
      BORG_MAGIC_NOP	/* "Sense Surroundings" */,
      BORG_MAGIC_NOP	/* "Cure Mortal Wounds" */,
      BORG_MAGIC_NOP	/* "Turn Undead" */
    },
    
    {
        /* Exorcism and Dispelling (sval 3) */
      BORG_MAGIC_NOP	/* "Prayer" */,
      BORG_MAGIC_NOP	/* "Dispel Undead" */,
      BORG_MAGIC_NOP	/* "Heal" */,
      BORG_MAGIC_NOP	/* "Dispel Evil" */,
      BORG_MAGIC_NOP	/* "Glyph of Warding" */,
      BORG_MAGIC_NOP	/* "Holy Word" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Ethereal openings (sval 4) */
      BORG_MAGIC_NOP	/* "Blink" */,
      BORG_MAGIC_NOP	/* "Teleport" */,
      BORG_MAGIC_AIM	/* "Teleport Away" */,
      BORG_MAGIC_NOP	/* "Teleport Level" */,
      BORG_MAGIC_NOP	/* "Word of Recall" */,
      BORG_MAGIC_NOP	/* "Alter Reality" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },

    {
        /* Godly Insights... (sval 5) */
      BORG_MAGIC_EXT	/* "Detect Monsters" */,
      BORG_MAGIC_EXT	/* "Detection" */,
      BORG_MAGIC_OBJ	/* "Perception" */,
      BORG_MAGIC_NOP	/* "Probing" */,
      BORG_MAGIC_NOP	/* "Clairvoyance" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Purifications and Healing (sval 6) */
      BORG_MAGIC_NOP	/* "Cure Serious Wounds" */,
      BORG_MAGIC_NOP	/* "Cure Mortal Wounds" */,
      BORG_MAGIC_NOP	/* "Healing" */,
      BORG_MAGIC_NOP	/* "Restoration" */,
      BORG_MAGIC_NOP	/* "Remembrance" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Holy Infusions (sval 7) */
      BORG_MAGIC_NOP	/* "Unbarring Ways" */,
      BORG_MAGIC_OBJ	/* "Recharging" */,
      BORG_MAGIC_NOP	/* "Dispel Curse" */,
      BORG_MAGIC_OBJ	/* "Enchant Weapon" */,
      BORG_MAGIC_OBJ	/* "Enchant Armour" */,
      BORG_MAGIC_NOP	/* "Elemental Brand" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    },
    
    {
        /* Wrath of God (sval 8) */
      BORG_MAGIC_NOP	/* "Dispel Undead" */,
      BORG_MAGIC_NOP	/* "Dispel Evil" */,
      BORG_MAGIC_NOP	/* "Banishment" */,
      BORG_MAGIC_NOP	/* "Word of Destruction" */,
      BORG_MAGIC_AIM	/* "Annihilation" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */,
      BORG_MAGIC_ICK	/* "(blank)" */
    }
  }
};


/*
 * Some variables
 */

auto_item *auto_items;		/* Current "inventory" */

auto_shop *auto_shops;		/* Current "shops" */



/*
 * Spell info
 */

auto_magic auto_magics[9][9];	/* Spell info, by book/what */




/*
 * Constant "item description parsers" (singles)
 */
static int auto_single_size;		/* Number of "singles" */
static s16b *auto_single_what;		/* Kind indexes for "singles" */
static cptr *auto_single_text;		/* Textual prefixes for "singles" */

/*
 * Constant "item description parsers" (plurals)
 */
static int auto_plural_size;		/* Number of "plurals" */
static s16b *auto_plural_what;		/* Kind index for "plurals" */
static cptr *auto_plural_text;		/* Textual prefixes for "plurals" */

/*
 * Constant "item description parsers" (suffixes)
 */
static int auto_artego_size;		/* Number of "artegos" */
static s16b *auto_artego_what;		/* Indexes for "artegos" */
static cptr *auto_artego_text;		/* Textual prefixes for "artegos" */




/*
 * Return the slot that items of the given type are wielded into
 *
 * Note that "rings" are now automatically wielded into the left hand
 *
 * Returns "-1" if the item cannot (or should not) be wielded
 */
int borg_wield_slot(auto_item *item)
{
    /* Slot for equipment */
    switch (item->tval) {

        case TV_SWORD:
        case TV_POLEARM:
        case TV_HAFTED:
        case TV_DIGGING:
            return (INVEN_WIELD);

        case TV_BOW:
            return (INVEN_BOW);

        case TV_RING:
            return (INVEN_LEFT);

        case TV_AMULET:
            return (INVEN_NECK);

        case TV_LITE:
            return (INVEN_LITE);

        case TV_DRAG_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
            return (INVEN_BODY);

        case TV_CLOAK:
            return (INVEN_OUTER);

        case TV_SHIELD:
            return (INVEN_ARM);

        case TV_CROWN:
        case TV_HELM:
            return (INVEN_HEAD);

        case TV_GLOVES:
            return (INVEN_HANDS);

        case TV_BOOTS:
            return (INVEN_FEET);
    }

    /* No slot available */
    return (-1);
}




/*
 * Determine the "base price" of a known item (see below)
 *
 * This function is adapted from "item_value_known()".
 *
 * This routine is called only by "borg_item_analyze()", which
 * uses this function to guess at the "value" of an item, if it
 * was to be sold to a store, with perfect "charisma" modifiers.
 */
static s32b borg_item_value_known(auto_item *item)
{
    s32b value;


    inven_kind *k_ptr = &k_info[item->kind];
    
    /* Worthless items */
    if (!k_ptr->cost) return (0L);

    /* Extract the base value */
    value = k_ptr->cost;


    /* Hack -- use artifact base costs */
    if (item->name1) {

        artifact_type *a_ptr = &a_info[item->name1];

        /* Worthless artifacts */
        if (!a_ptr->cost) return (0L);

        /* Hack -- use the artifact cost */
        value = a_ptr->cost;
    }

    /* Hack -- add in ego-item bonus cost */
    if (item->name2) {

        ego_item_type *e_ptr = &e_info[item->name2];

        /* Worthless ego-items */
        if (!e_ptr->cost) return (0L);

        /* Hack -- reward the ego-item cost */
        value += e_ptr->cost;
    }


    /* Analyze pval bonus */
    switch (item->tval) {

        /* Wands/Staffs */
        case TV_WAND:
        case TV_STAFF:

            /* Pay extra for charges */
            value += ((value / 20) * item->pval);

            break;

        /* Wearable items */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_LITE:
        case TV_AMULET:
        case TV_RING:

            /* Hack -- Negative "pval" is always bad */
            if (item->pval < 0) return (0L);

            /* No pval */
            if (!item->pval) break;
                
            /* Give credit for stat bonuses */
            if (item->flags1 & TR1_STR) value += (item->pval * 200L);
            if (item->flags1 & TR1_INT) value += (item->pval * 200L);
            if (item->flags1 & TR1_WIS) value += (item->pval * 200L);
            if (item->flags1 & TR1_DEX) value += (item->pval * 200L);
            if (item->flags1 & TR1_CON) value += (item->pval * 200L);
            if (item->flags1 & TR1_CHR) value += (item->pval * 200L);

            /* Give credit for stealth and searching */
            if (item->flags1 & TR1_STEALTH) value += (item->pval * 100L);
            if (item->flags1 & TR1_SEARCH) value += (item->pval * 100L);

            /* Give credit for infra-vision and tunneling */
            if (item->flags1 & TR1_INFRA) value += (item->pval * 50L);
            if (item->flags1 & TR1_TUNNEL) value += (item->pval * 50L);

            /* Give credit for extra attacks */
            if (item->flags1 & TR1_BLOWS) value += (item->pval * 2000L);

            /* Give credit for speed bonus */
            if (item->flags1 & TR1_SPEED) value += (item->pval * 30000L);

            break;
    }


    /* Analyze the item */
    switch (item->tval) {

        /* Rings/Amulets */
        case TV_RING:
        case TV_AMULET:

            /* Hack -- negative bonuses are bad */
            if (item->to_a < 0) return (0L);
            if (item->to_h < 0) return (0L);
            if (item->to_d < 0) return (0L);

            /* Give credit for bonuses */
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

            break;

        /* Armor */
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_CLOAK:
        case TV_CROWN:
        case TV_HELM:
        case TV_SHIELD:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:

            /* Hack -- negative armor bonus */
            if (item->to_a < 0) return (0L);

            /* Give credit for bonuses */
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

            break;

        /* Bows/Weapons */
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_SWORD:
        case TV_POLEARM:

            /* Hack -- negative hit/damage bonuses */
            if (item->to_h + item->to_d < 0) return (0L);

            /* Factor in the bonuses */
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

            /* Hack -- Factor in extra damage dice */
            if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds)) {
                value += (item->dd - k_ptr->dd) * item->ds * 200L;
            }

            break;

        /* Ammo */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:

            /* Hack -- negative hit/damage bonuses */
            if (item->to_h + item->to_d < 0) return (0L);

            /* Factor in the bonuses */
            value += ((item->to_h + item->to_d) * 5L);

            /* Hack -- Factor in extra damage dice */
            if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds)) {
                value += (item->dd - k_ptr->dd) * item->ds * 5L;
            }

            break;
    }


    /* Return the value */
    return (value);
}


/*
 * Analyze an item given a description and (optional) cost
 *
 * From the description, extract the item identity, and the various
 * bonuses, plus the "aware" and "known" flags (in an encoded state).
 *
 * We do a simple binary search on the arrays of object base names,
 * relying on the fact that they are sorted in reverse order, and on
 * the fact that efficiency is only important when the parse succeeds
 * (which it always does), and on some facts about "prefixes".
 *
 * We use the object kind to guess at the object weight and flags.
 *
 * Note that we will fail if the "description" was "partial", though
 * we will correctly handle a description with a "partial inscription",
 * so the actual item description must exceed 75 chars for us to fail,
 * though we will only get 60 characters if the item is in a shop, but
 * luckily items in shops (usually) do not have inscriptions, and when
 * they do, they are (usually) very short.
 *
 * We also do a simple binary search on the descriptions of items which
 * might be artifacts or ego-items, and use that infomation to update
 * our guesses about the object weight and flags.
 *
 * We also guess at the value of the item, as given by "item_value()".
 */
void borg_item_analyze(auto_item *item, cptr desc)
{
    int i, m, n;

    int d1 = 0;
    int d2 = 0;
    int ac = 0;
    int th = 0;
    int td = 0;
    int ta = 0;

    bool done = FALSE;

    char *scan;
    char *tail;

    char temp[128];


    /* Wipe the item */
    WIPE(item, auto_item);


    /* Save the item description */
    strcpy(item->desc, desc);


    /* Advance to the "inscription" or end of string */
    for (scan = item->desc; *scan && (*scan != c1); scan++) ;

    /* Save a pointer to the inscription */
    item->note = scan;


    /* Empty item */
    if (!desc[0]) return;


    /* Assume singular */
    item->iqty = 1;

    /* Notice prefix "a " */
    if ((desc[0] == 'a') && (desc[1] == ' ')) {

        /* Skip "a " */
        desc += 2;
    }

    /* Notice prefix "a " */
    else if ((desc[0] == 'a') && (desc[1] == 'n') && (desc[2] == ' ')) {

        /* Skip "an " */
        desc += 3;
    }

    /* Notice prefix "The " */
    else if ((desc[0] == 'T') && (desc[1] == 'h') &&
             (desc[2] == 'e') && (desc[3] == ' ')) {

        /* Skip "The " */
        desc += 4;
    }

    /* Notice "numerical" prefixes */
    else if (isdigit(desc[0])) {

        cptr s;

        /* Find the first space */
        for (s = desc; *s && (*s != ' '); s++) ;

        /* Paranoia -- Catch sillyness */
        if (*s != ' ') return;

        /* Extract a quantity */
        item->iqty = atoi(desc);

        /* Skip the quantity and space */
        desc = s + 1;
    }


    /* Paranoia */
    if (!desc[0]) return;


    /* Obtain a copy of the description */
    strcpy(temp, desc);

    /* Advance to the "inscription" or end of string */
    for (scan = temp; *scan && (*scan != c1); scan++) ;

    /* Nuke the space before the inscription */
    if ((scan[0] == c1) && (scan[-1] == ' ')) *--scan = '\0';

    /* Note that "scan" points at the "tail" of "temp" */

    /* Hack -- non-aware, singular, flavored items */
    if (item->iqty == 1) {
        if (prefix(temp, "Scroll titled ")) item->tval = TV_SCROLL;
        else if (streq(scan-7, " Potion")) item->tval = TV_POTION;
        else if (streq(scan-6, " Staff")) item->tval = TV_STAFF;
        else if (streq(scan-5, " Wand")) item->tval = TV_WAND;
        else if (streq(scan-4, " Rod")) item->tval = TV_ROD;
        else if (streq(scan-5, " Ring")) item->tval = TV_RING;
        else if (streq(scan-7, " Amulet")) item->tval = TV_AMULET;
        else if (streq(scan-9, " Mushroom")) item->tval = TV_FOOD;
    }

    /* Hack -- non-aware, plural, flavored items */
    else {
        if (prefix(temp, "Scrolls titled ")) item->tval = TV_SCROLL;
        else if (streq(scan-8, " Potions")) item->tval = TV_POTION;
        else if (streq(scan-7, " Staffs")) item->tval = TV_STAFF;
        else if (streq(scan-6, " Wands")) item->tval = TV_WAND;
        else if (streq(scan-5, " Rods")) item->tval = TV_ROD;
        else if (streq(scan-6, " Rings")) item->tval = TV_RING;
        else if (streq(scan-8, " Amulets")) item->tval = TV_AMULET;
        else if (streq(scan-10, " Mushrooms")) item->tval = TV_FOOD;
    }

    /* Accept non-aware flavored objects */
    if (item->tval) {

        /* Guess at weight and cost */
        switch (item->tval) {
       
            case TV_FOOD:
                item->weight = 1;
                item->value = 5L;
                break;

            case TV_POTION:
                item->weight = 4;
                item->value = 20L;
                break;

            case TV_SCROLL:
                item->weight = 5;
                item->value = 20L;
                break;

            case TV_STAFF:
                item->weight = 50;
                item->value = 70L;
                break;

            case TV_WAND:
                item->weight = 10;
                item->value = 50L;
                break;
                
            case TV_ROD:
                item->weight = 15;
                item->value = 90L;
                break;

            case TV_RING:
                item->weight = 2;
                item->value = 45L;
                break;
                
            case TV_AMULET:
                item->weight = 3;
                item->value = 45L;
                break;
        }

        /* Done */
        return;
    }


    /* Start at the beginning */
    tail = temp;

    /* Check singular items */
    if (item->iqty == 1) {

        /* Start the search */
        m = 0; n = auto_single_size;

        /* Simple binary search */
        while (m < n - 1) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Search to the right (or here) */
            if (strcmp(auto_single_text[i], tail) <= 0) {
                m = i;
            }

            /* Search to the left */
            else {
                n = i;
            }
        }

        /* Search for a prefix */
        for (i = m; i >= 0; i--) {

            /* Check for proper prefix */
            if (prefix(tail, auto_single_text[i])) break;
        }

        /* Oops.  Bizarre item. */
        if (i < 0) {
            borg_oops("bizarre object");
            return;
        }

        /* Save the item kind */
        item->kind = auto_single_what[i];

        /* Skip past the base name */
        tail += strlen(auto_single_text[i]);
    }

    /* Check plural items */
    else {

        /* Start the search */
        m = 0; n = auto_plural_size;

        /* Simple binary search */
        while (m < n - 1) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Search to the right (or here) */
            if (strcmp(auto_plural_text[i], tail) <= 0) {
                m = i;
            }

            /* Search to the left */
            else {
                n = i;
            }
        }

        /* Search for a prefix */
        for (i = m; i >= 0; i--) {

            /* Check for proper prefix */
            if (prefix(tail, auto_plural_text[i])) break;
        }

        /* Oops.  Bizarre item. */
        if (i < 0) {
            borg_oops("bizarre object");
            return;
        }

        /* Save the item kind */
        item->kind = auto_plural_what[i];

        /* Skip past the base name */
        tail += strlen(auto_plural_text[i]);
    }


    /* Extract some info */
    item->tval = k_info[item->kind].tval;
    item->sval = k_info[item->kind].sval;

    /* Guess at the weight */
    item->weight = k_info[item->kind].weight;

    /* Extract the base flags */
    item->flags1 = k_info[item->kind].flags1;
    item->flags2 = k_info[item->kind].flags2;
    item->flags3 = k_info[item->kind].flags3;


    /* Analyze "bonuses" */
    switch (item->tval) {

        /* Basic items */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
        case TV_FLASK:
        case TV_FOOD:
        case TV_POTION:
        case TV_SCROLL:
        case TV_SPIKE:
        case TV_SKELETON:
        case TV_BOTTLE:
        case TV_JUNK:

            /* Always "able" */
            item->able = TRUE;

            break;

        /* Chests */
        case TV_CHEST:

            /* XXX XXX XXX */
            
            /* Require the prefix and suffix */
            if (!prefix(tail, " (")) break;
            if (!suffix(tail, ")")) break;

            /* Assume "able" */
            item->able = TRUE;

            /* Hack -- assume "trapped" */
            item->pval = 63;

            /* Hack -- extract "empty" */
            if (streq(tail, " (empty)")) item->pval = 0;

            break;
                
        /* Wands/Staffs -- charges */
        case TV_WAND:
        case TV_STAFF:

            /* Require the prefix and suffix */
            if (!prefix(tail, " (")) break; /* --(-- */
            if (!suffix(tail, " charge)") && !suffix(tail, " charges)")) break;

            /* Extract the "charges" */
            item->pval = atoi(tail+2);

            /* Assume "able" */
            item->able = TRUE;

            break;

        /* Rods -- charging */
        case TV_ROD:

            /* Always "able" */
            item->able = TRUE;

            /* Mega-Hack -- fake "charges" */
            item->pval = 1;

            /* Mega-Hack -- "charging" means no "charges" */
            if (streq(tail, " (charging)")) item->pval = 0;

            break;

        /* Wearable items */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_LITE:
        case TV_AMULET:
        case TV_RING:

            /* Hack -- handle "easy know" */
            if (k_info[item->kind].flags3 & TR3_EASY_KNOW) {

                /* Always known */
                item->able = TRUE;
            }

            /* No suffix */
            if (tail[0] != ' ') break;

            /* Start the search */
            m = 0; n = auto_artego_size;

            /* Binary search */
            while (m < n - 1) {

                /* Pick a "middle" entry */
                i = (m + n) / 2;

                /* Search to the right (or here) */
                if (strcmp(auto_artego_text[i], tail) <= 0) {
                    m = i;
                }

                /* Search to the left */
                else {
                    n = i;
                }
            }

            /* Search for a prefix */
            for (i = m; i >= 0; i--) {

                /* Check for proper prefix */
                if (prefix(tail, auto_artego_text[i])) {

                    /* Paranoia -- Item is known */
                    item->able = TRUE;

                    /* Save the artifact name */
                    if (auto_artego_what[i] < 256) {
                        item->name1 = auto_artego_what[i];
                    }

                    /* Save the ego-item name */
                    else {
                        item->name2 = auto_artego_what[i] - 256;
                    }

                    /* Skip the space and the ego-item name */
                    tail += strlen(auto_artego_text[i]);

                    /* Done */
                    break;
                }

                /* Hack -- limit the search */
                if (i < m - 12) break;
            }

            /* Hack -- handle Lite's */
            if (item->tval == TV_LITE) {

                /* Hack -- Artifact Lite's */
                if (item->name1) {

                    /* Assume "able" */
                    item->able = TRUE;
                    
                    /* Hack -- huge fuel */
                    item->pval = 29999;

                    break;
                }
                
                /* Require the prefix and suffix */
                if (!prefix(tail, " (with ")) break;
                if (!suffix(tail, " of light)")) break;

                /* Extract "turns of lite" */
                item->pval = atoi(tail+7);

                /* Assume "able" */
                item->able = TRUE;

                break;
            }

            /* Hack -- Skip spaces */
            while (tail[0] == ' ') tail++;
            
            /* No suffix */
            if (!tail[0]) break;

            /* Parse "weapon-style" damage strings */
            if ((tail[0] == p1) &&
                ((item->tval == TV_HAFTED) ||	
                 (item->tval == TV_POLEARM) ||	
                 (item->tval == TV_SWORD) ||	
                 (item->tval == TV_DIGGING) ||	
                 (item->tval == TV_BOLT) ||	
                 (item->tval == TV_ARROW) ||	
                 (item->tval == TV_SHOT))) {

                /* First extract the damage string */
                for (scan = tail; *scan != p2; scan++) ;
                scan++;

                /* Hack -- Notice "end of string" */
                if (scan[0] != ' ') done = TRUE;

                /* Terminate the string and advance */
                *scan++ = '\0';

                /* Parse the damage string, or stop XXX */
                if (sscanf(tail, "(%dd%d)", &d1, &d2) != 2) break;

                /* Save the values */
                item->dd = d1; item->ds = d2;

                /* No extra information means not identified */
                if (done) break;

                /* Skip the "damage" info */
                tail = scan;
            }

            /* Parse the "damage" string for bows */
            else if ((tail[0] == p1) &&
                     (item->tval == TV_BOW)) {

                /* First extract the damage string */
                for (scan = tail; *scan != p2; scan++) ;
                scan++;

                /* Hack -- Notice "end of string" */
                if (scan[0] != ' ') done = TRUE;

                /* Terminate the string and advance */
                *scan++ = '\0';

                /* Parse the multiplier string, or stop */
                if (sscanf(tail, "(x%d)", &d1) != 1) break;

                /* Hack -- save it in "damage dice" */
                item->dd = d1;

                /* No extra information means not identified */
                if (done) break;

                /* Skip the "damage" info */
                tail = scan;
            }


            /* Parse the "bonus" string */
            if (tail[0] == p1) {

                /* Extract the extra info */
                for (scan = tail; *scan != p2; scan++) ;
                scan++;

                /* Hack -- Notice "end of string" */
                if (scan[0] != ' ') done = TRUE;

                /* Terminate the damage, advance */
                *scan++ = '\0';

                /* Parse standard "bonuses" */
                if (sscanf(tail, "(%d,%d)", &th, &td) == 2) {
                    item->to_h = th; item->to_d = td;
                    item->able = TRUE;
                }

                /* XXX XXX Hack -- assume non-final bonuses are "to_hit" */
                else if (!done && sscanf(tail, "(%d)", &th) == 1) {
                    item->to_h = th;
                    item->able = TRUE;
                }

                /* XXX XXX Hack -- assume final bonuses are "pval" codes */
                else if (done) {
                    item->pval = atoi(tail + 1);
                    item->able = TRUE;
                }

                /* Oops */
                else {
                    break;
                }

                /* Nothing left */
                if (done) break;

                /* Skip the "damage bonus" info */
                tail = scan;
            }


            /* Parse the "bonus" string */
            if (tail[0] == b1) {

                /* Extract the extra info */
                for (scan = tail; *scan != b2; scan++) ;
                scan++;

                /* Hack -- Notice "end of string" */
                if (scan[0] != ' ') done = TRUE;

                /* Terminate the armor string, advance */
                *scan++ = '\0';

                /* Parse the armor, and bonus */
                if (sscanf(tail, "[%d,%d]", &ac, &ta) == 2) {
                    item->ac = ac;
                    item->to_a = ta;
                    item->able = TRUE;
                }

                /* Negative armor bonus */
                else if (sscanf(tail, "[-%d]", &ta) == 1) {
                    item->to_a = -ta;
                    item->able = TRUE;
                }

                /* Positive armor bonus */
                else if (sscanf(tail, "[+%d]", &ta) == 1) {
                    item->to_a = ta;
                    item->able = TRUE;
                }

                /* Just base armor */
                else if (sscanf(tail, "[%d]", &ac) == 1) {
                    item->ac = ac;
                }

                /* Oops */
                else {
                    break;
                }

                /* Nothing left */
                if (done) break;

                /* Skip the "armor" data */
                tail = scan;
            }


            /* Parse the final "pval" string, if any */
            if (tail[0] == p1) {

                /* Assume identified */
                item->able = TRUE;

                /* Hack -- Grab it */
                item->pval = atoi(tail + 1);
            }

            break;
    }


    /* Hack -- repair rings of damage */
    if ((item->tval == TV_RING) && (item->sval == SV_RING_DAMAGE)) {

        /* Bonus to dam, not pval */
        item->to_d = item->pval;
        item->pval = 0;
    }
    
    /* Hack -- repair rings of accuracy */
    if ((item->tval == TV_RING) && (item->sval == SV_RING_ACCURACY)) {

        /* Bonus to hit, not pval */
        item->to_h = item->pval;
        item->pval = 0;
    }


    /* XXX XXX XXX Repair various "ego-items" */


    /* Hack -- examine artifacts */
    if (item->name1) {

        /* XXX XXX Hack -- fix "weird" artifacts */
        if ((item->tval != a_info[item->name1].tval) ||
            (item->sval != a_info[item->name1].sval)) {

            /* Save the kind */
            item->kind = lookup_kind(item->tval, item->sval);

            /* Save the tval/sval */
            item->tval = k_info[item->kind].tval;
            item->sval = k_info[item->kind].sval;
        }

        /* Extract the weight */
        item->weight = a_info[item->name1].weight;

        /* Extract the artifact flags */
        item->flags1 = a_info[item->name1].flags1;
        item->flags2 = a_info[item->name1].flags2;
        item->flags3 = a_info[item->name1].flags3;
    }


    /* Hack -- examine ego-items */
    if (item->name2) {

        /* XXX Extract the weight */

        /* Extract the ego-item flags */
        item->flags1 |= e_info[item->name2].flags1;
        item->flags2 |= e_info[item->name2].flags2;
        item->flags3 |= e_info[item->name2].flags3;
    }

    
    /* Known items */
    if (item->able) {

        /* Process various fields */
        item->value = borg_item_value_known(item);
    }

    /* Aware items */
    else {

        /* Aware items can assume template cost */
        item->value = k_info[item->kind].cost;
    }


    /* Parse various "inscriptions" */
    if (item->note[0]) {

        /* Special "discount" */
        if (streq(item->note, "{on sale}")) item->discount = 50;

        /* Standard "discounts" */
        else if (streq(item->note, "{25% off}")) item->discount = 25;
        else if (streq(item->note, "{50% off}")) item->discount = 50;
        else if (streq(item->note, "{75% off}")) item->discount = 75;
        else if (streq(item->note, "{90% off}")) item->discount = 90;

        /* Cursed indicators */
        else if (streq(item->note, "{cursed}")) item->value = 0L;
        else if (streq(item->note, "{broken}")) item->value = 0L;
        else if (streq(item->note, "{terrible}")) item->value = 0L;
        else if (streq(item->note, "{worthless}")) item->value = 0L;

        /* Ignore certain feelings */
        /* "{average}" */
        /* "{blessed}" */
        /* "{good}" */
        /* "{excellent}" */
        /* "{special}" */

        /* Ignore special inscriptions */
        /* "{empty}", "{tried}" */
    }


    /* Apply "discount" if any */
    if (item->discount) item->value -= item->value * item->discount / 100;
}




/*
 * Send a command to inscribe item number "i" with the inscription "str".
 */
void borg_send_inscribe(int i, cptr str)
{
    cptr s;

    /* Label it */
    borg_keypress(c1);

    /* Choose from inventory */
    if (i < INVEN_WIELD) {

        /* Choose the item */
        borg_keypress(I2A(i));
    }

    /* Choose from equipment */
    else {

        /* Go to equipment (if necessary) */
        if (auto_items[0].iqty) borg_keypress('/');

        /* Choose the item */
        borg_keypress(I2A(i - INVEN_WIELD));
    }

    /* Send the label */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the inscription */
    borg_keypress('\n');
}




#if 0

/*
 * Count the number of items of the given tval/sval in our possession
 */
int borg_count(int tval, int sval)
{
    int i, n = 0;

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip un-aware items */
        if (!item->kind) continue;

        /* Require correct tval */
        if (item->tval != tval) continue;

        /* Require correct sval */
        if (item->sval != sval) continue;

        /* Count the items */
        n += item->iqty;
    }

    /* Done */
    return (n);
}

#endif


/*
 * Find the slot of an item with the given tval/sval, if available.
 * Given multiple choices, choose the item with the largest "pval".
 * Given multiple choices, choose the smallest available pile.
 */
int borg_slot(int tval, int sval)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip un-aware items */
        if (!item->kind) continue;

        /* Require correct tval */
        if (item->tval != tval) continue;

        /* Require correct sval */
        if (item->sval != sval) continue;

        /* Prefer largest "pval" */
        if ((n >= 0) && (item->pval < auto_items[n].pval)) continue;

        /* Prefer smallest pile */
        if ((n >= 0) && (item->iqty > auto_items[n].iqty)) continue;

        /* Save this item */
        n = i;
    }

    /* Done */
    return (n);
}



/*
 * Hack -- refuel a torch
 */
bool borg_refuel_torch(void)
{
    int i;

    /* Look for a torch */
    i = borg_slot(TV_LITE, SV_LITE_TORCH);

    /* None available */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Refueling with %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('F');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}


/*
 * Hack -- refuel a lantern
 */
bool borg_refuel_lantern(void)
{
    int i;

    /* Look for a torch */
    i = borg_slot(TV_FLASK, 0);

    /* None available */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Refueling with %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('F');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}




/*
 * Hack -- attempt to eat the given food (by sval)
 */
bool borg_eat_food(int sval)
{
    int i;

    /* Look for that food */
    i = borg_slot(TV_FOOD, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Eating %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to quaff the given potion (by sval)
 */
bool borg_quaff_potion(int sval)
{
    int i;

    /* Look for that potion */
    i = borg_slot(TV_POTION, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Quaffing %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to read the given scroll (by sval)
 */
bool borg_read_scroll(int sval)
{
    int i;

    /* Blind or Confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for that scroll */
    i = borg_slot(TV_SCROLL, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Reading %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('r');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to zap the given (charged) rod (by sval)
 */
bool borg_zap_rod(int sval)
{
    int i;

    /* Look for that rod */
    i = borg_slot(TV_ROD, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* Hack -- Still charging */
    if (!auto_items[i].pval) return (FALSE);

    /* Log the message */
    borg_note(format("# Zapping %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('z');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to aim the given (charged) wand (by sval)
 */
bool borg_aim_wand(int sval)
{
    int i;

    /* Look for that wand */
    i = borg_slot(TV_WAND, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* No charges */
    if (!auto_items[i].pval) return (FALSE);

    /* Log the message */
    borg_note(format("# Aiming %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('a');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to use the given (charged) staff (by sval)
 */
bool borg_use_staff(int sval)
{
    int i;

    /* Look for that staff */
    i = borg_slot(TV_STAFF, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* No charges */
    if (!auto_items[i].pval) return (FALSE);

    /* Log the message */
    borg_note(format("# Using %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}




/*
 * Find the slot of any copy of the given "book", if possible
 *
 * This function should probably be optimized by a global array, since
 * it is the bottle-neck for many important functions.  XXX XXX XXX
 */
int borg_book(int book)
{
    int i;

    /* Must have a book */
    if (!mb_ptr->spell_book) return (-1);

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Found it */
        if ((item->tval == mb_ptr->spell_book) &&
            (item->sval == book)) return (i);
    }

    /* No luck */
    return (-1);
}


/*
 * Determine if borg can cast a given spell (when rested)
 */
bool borg_spell_okay(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Hack -- blind/confused */
    /* if (do_blind || do_confused) return (FALSE); */

    /* The borg must be able to "cast" spells */
    if (mb_ptr->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The spell must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The spell must be affordable (when rested) */
    if (as->power > auto_msp) return (FALSE);

    /* Success */
    return (TRUE);
}

/*
 * Attempt to cast a spell
 */
bool borg_spell(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* The borg must be able to "cast" spells */
    if (mb_ptr->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The spell must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The spell must be affordable */
    if (as->power > auto_csp) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Casting %s.", as->name));

    /* Cast a spell */
    borg_keypress('m');
    borg_keypress(I2A(i));
    borg_keypress(I2A(what));

    /* Success */
    return (TRUE);
}


/*
 * Attempt to cast a spell, keeping some "safety" mana
 */
bool borg_spell_safe(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* The borg must be able to "cast" spells */
    if (mb_ptr->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The spell must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The spell must be affordable */
    if (as->power > auto_csp) return (FALSE);

    /* The spell must be "safe" */
    if (auto_csp - as->power < auto_msp / 2) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Casting %s.", as->name));

    /* Cast a spell */
    borg_keypress('m');
    borg_keypress(I2A(i));
    borg_keypress(I2A(what));

    /* Success */
    return (TRUE);
}



/*
 * Determine if borg can pray a given prayer
 */
bool borg_prayer_okay(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Hack -- blind/confused */
    /* if (do_blind || do_confused) return (FALSE); */

    /* The borg must be able to "pray" prayers */
    if (mb_ptr->spell_book != TV_PRAYER_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The prayer must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The prayer must be affordable (when rested) */
    if (as->power > auto_msp) return (FALSE);

    /* Success */
    return (TRUE);
}

/*
 * Attempt to pray a prayer
 */
bool borg_prayer(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* The borg must be able to "pray" prayers */
    if (mb_ptr->spell_book != TV_PRAYER_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The prayer must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The prayer must be affordable */
    if (as->power > auto_csp) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Praying %s.", as->name));

    /* Pray a prayer */
    borg_keypress('p');
    borg_keypress(I2A(i));
    borg_keypress(I2A(what));

    /* Success */
    return (TRUE);
}

/*
 * Attempt to pray a prayer, keeping some "safety" mana
 */
bool borg_prayer_safe(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* The borg must be able to "pray" prayers */
    if (mb_ptr->spell_book != TV_PRAYER_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The prayer must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The prayer must be affordable */
    if (as->power > auto_csp) return (FALSE);

    /* The prayer must be "safe" */
    if (auto_csp - as->power < auto_msp / 2) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Praying %s.", as->name));

    /* Pray a prayer */
    borg_keypress('p');
    borg_keypress(I2A(i));
    borg_keypress(I2A(what));

    /* Success */
    return (TRUE);
}


/*
 * Attempt to "study" the given spell
 */
bool borg_study_spell(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Can we use spells? */
    if (mb_ptr->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* Make sure we have the book */
    if (i < 0) return (FALSE);

    /* Skip "non-learnable" spells */
    if (as->status != BORG_MAGIC_OKAY) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Studying Spell %s.", as->name));

    /* Learn the spell */
    borg_keypress('G');

    /* Specify the book */
    borg_keypress(I2A(i));

    /* Specify the spell */
    borg_keypress(I2A(what));

    /* Success */
    return (TRUE);
}


/*
 * Attempt to "study" the given prayer
 */
bool borg_study_prayer(int book, int what)
{
    int i;

    auto_magic *as = &auto_magics[book][what];

    /* Can we use prayers? */
    if (mb_ptr->spell_book != TV_PRAYER_BOOK) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for the book */
    i = borg_book(book);

    /* Make sure we have the book */
    if (i < 0) return (FALSE);

    /* Skip "non-learnable" spells */
    if (as->status != BORG_MAGIC_OKAY) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Studying Prayer %s.", as->name));

    /* Learn the spell */
    borg_keypress('G');

    /* Specify the book */
    borg_keypress(I2A(i));

    /* Specify the prayer (not!) */
    /* borg_keypress(I2A(what)); */

    /* Success */
    return (TRUE);
}



/*
 * Determine if the Borg may "study" new spells/prayers
 */
bool borg_study_okay(void)
{
    int i, book, what;

    /* Can we use spells/prayers? */
    if (!mb_ptr->spell_book) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Check each book */
    for (book = 0; book < 9; book++) {

        /* Look for the book */
        i = borg_book(book);

        /* Make sure we have the book */
        if (i < 0) continue;

        /* Check for spells */
        for (what = 0; what < 9; what++) {

            auto_magic *as = &auto_magics[book][what];

            /* Notice "learnable" spells */
            if (as->status == BORG_MAGIC_OKAY) return (TRUE);
        }
    }

    /* Nope */
    return (FALSE);
}





/*
 * Cheat the "equip" screen
 */
void borg_cheat_equip(void)
{
    int i;

    char buf[256];

    /* Extract the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Default to "nothing" */
        buf[0] = '\0';

        /* Describe a real item */
        if (inventory[i].k_idx) {

            /* Describe it */
            objdes(buf, &inventory[i], TRUE, 3);
        }

        /* Ignore "unchanged" items */
        if (streq(buf, auto_items[i].desc)) continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&auto_items[i], buf);
    }
}


/*
 * Cheat the "inven" screen
 */
void borg_cheat_inven(void)
{
    int i;

    char buf[256];

    /* Extract the current weight */
    auto_cur_wgt = total_weight;

    /* Extract the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        /* Default to "nothing" */
        buf[0] = '\0';

        /* Describe a real item */
        if (inventory[i].k_idx) {

            /* Describe it */
            objdes(buf, &inventory[i], TRUE, 3);
        }

        /* Ignore "unchanged" items */
        if (streq(buf, auto_items[i].desc)) continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&auto_items[i], buf);
    }
}


/*
 * Parse the "equip" screen
 */
void borg_parse_equip(void)
{
    int i;

    int row, col;

    bool done = FALSE;

    byte t_a;

    char buf[160];


    /* Find the column */
    for (col = 0; col < 55; col++) {

        /* Look for first prefix */
        if ((0 == borg_what_text_hack(col, 1, 3, &t_a, buf)) &&
            (buf[0] == I2A(0)) && (buf[1] == p2) && (buf[2] == ' ')) {

            break;
        }
    }

    /* Extract the inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Access the row */
        row = i - INVEN_WIELD;

        /* Attempt to get some text */
        if (!done &&
            (0 == borg_what_text_hack(col, row+1, 3, &t_a, buf)) &&
            (buf[0] == I2A(row)) && (buf[1] == p2) && (buf[2] == ' ') &&
            (0 == borg_what_text(col+19, row+1, -80, &t_a, buf)) &&
            (buf[0] && (buf[0] != ' '))) {

            /* XXX Strip final spaces */
        }

        /* Default to "nothing" */
        else {
            buf[0] = '\0';
            done = TRUE;
        }

        /* Notice empty slots */
        if (streq(buf, "(nothing)")) strcpy(buf, "");

        /* Ignore "unchanged" items */
        if (streq(buf, auto_items[i].desc)) continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&auto_items[i], buf);
    }
}


/*
 * Parse the "inven" screen
 */
void borg_parse_inven(void)
{
    int i;

    int row, col;

    int w1a, w1b, w2a, w2b;
    
    bool done = FALSE;

    byte t_a;

    char buf[160];


    /* Hack -- Parse the current and maximum weight */
    if ((0 == borg_what_text_hack(0, 0, -80, &t_a, buf)) &&
        (sscanf(buf, "Inventory (carrying %d.%d / %d.%d pounds)",
                &w1a, &w1b, &w2a, &w2b) == 4)) {

        /* Save the current weight */
        auto_cur_wgt = w1a * 10 + w1b;
    }


    /* Find the column */
    for (col = 0; col < 55; col++) {

        /* Look for first prefix */
        if ((0 == borg_what_text_hack(col, 1, 3, &t_a, buf)) &&
            (buf[0] == I2A(0)) && (buf[1] == p2) && (buf[2] == ' ')) {

            break;
        }
    }

    /* Extract the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        /* Access the row */
        row = i;

        /* Attempt to get some text */
        if (!done &&
            (0 == borg_what_text_hack(col, row+1, 3, &t_a, buf)) &&
            (buf[0] == I2A(row)) && (buf[1] == p2) && (buf[2] == ' ') &&
            (0 == borg_what_text(col+3, row+1, -80, &t_a, buf)) &&
            (buf[0] && (buf[0] != ' '))) {

            /* XXX Strip final spaces */
        }

        /* Default to "nothing" */
        else {
            buf[0] = '\0';
            done = TRUE;
        }

        /* Notice empty slots */
        if (streq(buf, "(nothing)")) strcpy(buf, "");

        /* Ignore "unchanged" items */
        if (streq(buf, auto_items[i].desc)) continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&auto_items[i], buf);
    }
}





/*
 * Hack -- Cheat the "spell" info (given the book)
 *
 * Hack -- note the use of the "cheat" field for efficiency
 */
void borg_cheat_spell(int book)
{
    int j, what;


    /* Can we use spells/prayers? */
    if (!mb_ptr->spell_book) return;


    /* Process the spells */
    for (what = 0; what < 9; what++) {

        /* Access the spell */
        auto_magic *as = &auto_magics[book][what];

        /* Skip illegible spells */
        if (as->status == BORG_MAGIC_ICKY) continue;

        /* Access the index */
        j = as->cheat;

        /* Note "forgotten" spells */
        if ((j < 32) ?
            ((spell_forgotten1 & (1L << j))) :
            ((spell_forgotten2 & (1L << (j - 32))))) {

            /* Forgotten */
            as->status = BORG_MAGIC_LOST;
        }

        /* Note "difficult" spells */
        else if (auto_level < as->level) {

            /* Unknown */
            as->status = BORG_MAGIC_HIGH;
        }

        /* Note "unknown" spells */
        else if (!((j < 32) ?
                   (spell_learned1 & (1L << j)) :
                   (spell_learned2 & (1L << (j - 32))))) {

            /* Unknown */
            as->status = BORG_MAGIC_OKAY;
        }

        /* Note "untried" spells */
        else if (!((j < 32) ?
                   (spell_worked1 & (1L << j)) :
                   (spell_worked2 & (1L << (j - 32))))) {

            /* Untried */
            as->status = BORG_MAGIC_TEST;
        }

        /* Note "known" spells */
        else {

            /* Known */
            as->status = BORG_MAGIC_KNOW;
        }
    }
}





/*
 * Hack -- Parse the "spell" info (given the book)
 */
void borg_parse_spell(int book)
{
    int what;

    byte t_a;

    char buf[160];


    /* Can we use spells/prayers? */
    if (!mb_ptr->spell_book) return;


    /* Process the spells */
    for (what = 0; what < 9; what++) {

        int row = ROW_SPELL + 1 + what;
        int col = COL_SPELL;

        /* Access the spell */
        auto_magic *as = &auto_magics[book][what];

        /* Skip illegible spells */
        if (as->status == BORG_MAGIC_ICKY) continue;

#if 0
        /* Format: "spell-name...................." at col 20+5 */
        if (0 != borg_what_text_hack(col-30, row, -30, &t_a, buf)) continue;
#endif

        /* Format: "Lv Mana Freq Comment" at col 20+35 */
        if (0 != borg_what_text_hack(col, row, -20, &t_a, buf)) continue;

        /* Note "forgotten" spells */
        if (prefix(buf + 13, "forgott")) {

            /* Forgotten */
            as->status = BORG_MAGIC_LOST;
        }

        /* Note "difficult" spells */
        else if (auto_level < as->level) {

            /* Unknown */
            as->status = BORG_MAGIC_HIGH;
        }

        /* Note "unknown" spells */
        else if (prefix(buf + 13, "unknown")) {

            /* Unknown */
            as->status = BORG_MAGIC_OKAY;
        }

        /* Note "untried" spells */
        else if (prefix(buf + 13, "untried")) {

            /* Untried */
            as->status = BORG_MAGIC_TEST;
        }

        /* Note "known" spells */
        else {

            /* Known */
            as->status = BORG_MAGIC_KNOW;
        }
    }
}




/*
 * Prepare a book
 */
static void prepare_book_info(int book)
{
    int i, what;

    int spell[64], num = 0;


    /* Reset each spell entry */
    for (what = 0; what < 9; what++) {

        auto_magic *as = &auto_magics[book][what];

        /* Assume no name */
        as->name = NULL;

        /* Assume illegible */
        as->status = BORG_MAGIC_ICKY;

        /* Assume illegible */
        as->method = BORG_MAGIC_ICK;

        /* Impossible values */
        as->level = 99;
        as->power = 99;

        /* Impossible value */
        as->cheat = 99;
    }


    /* Can we use spells/prayers? */
    if (!mb_ptr->spell_book) return;


    /* Extract spells */
    for (i = 0; i < 64; i++) {

        /* Check for this spell */
        if ((i < 32) ?
            (spell_flags[mb_ptr->spell_type][book][0] & (1L << i)) :
            (spell_flags[mb_ptr->spell_type][book][1] & (1L << (i - 32)))) {

            /* Collect this spell */
            spell[num++] = i;
        }
    }


    /* Process each existing spell */
    for (what = 0; what < num; what++) {

        auto_magic *as = &auto_magics[book][what];

        magic_type *s_ptr = &mb_ptr->info[spell[what]];

        /* Skip "illegible" spells */
        if (s_ptr->slevel == 99) continue;

        /* Mega-Hack -- Save the spell index */
        as->cheat = spell[what];

        /* Access the "name" */
        as->name = spell_names[mb_ptr->spell_type][spell[what]];

        /* Hack -- assume excessive level */
        as->status = BORG_MAGIC_HIGH;

        /* Access the correct "method" */
        as->method = auto_magic_method[mb_ptr->spell_type][book][what];

        /* Extract the level and power */
        as->level = s_ptr->slevel;
        as->power = s_ptr->smana;
    }
}



/*
 * Hack -- prepare some stuff based on the player race and class
 */
void prepare_race_class_info(void)
{
    int book;

    /* Initialize the various spell arrays by book */
    for (book = 0; book < 9; book++) prepare_book_info(book);
}



/*
 * Sorting hook -- comp function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
static bool ang_sort_comp_hook(vptr u, vptr v, int a, int b)
{
    cptr *text = (cptr*)(u);
    s16b *what = (s16b*)(v);

    int cmp;
    
    /* Compare the two strings */
    cmp = (strcmp(text[a], text[b]));
    
    /* Strictly less */
    if (cmp < 0) return (TRUE);
    
    /* Strictly more */
    if (cmp > 0) return (FALSE);
    
    /* Enforce "stable" sort */
    return (what[a] <= what[b]);
}


/*
 * Sorting hook -- swap function -- see below
 *
 * We use "u" to point to an array of strings, and "v" to point to
 * an array of indexes, and we sort them together by the strings.
 */
static void ang_sort_swap_hook(vptr u, vptr v, int a, int b)
{
    cptr *text = (cptr*)(u);
    s16b *what = (s16b*)(v);

    cptr texttmp;
    s16b whattmp;
    
    /* Swap "text" */
    texttmp = text[a];
    text[a] = text[b];
    text[b] = texttmp;

    /* Swap "what" */
    whattmp = what[a];
    what[a] = what[b];
    what[b] = whattmp;
}



/*
 * Init "borg-obj.c".
 *
 * Note that the Borg will never find Grond/Morgoth, but we
 * prepare the item parsers for them anyway.  Actually, the
 * Borg might get lucky and find some of the special artifacts,
 * so it is always best to prepare for a situation if it does
 * not cost any effort.
 *
 * Note that all six artifact "Rings" will parse as "kind 506"
 * (the first artifact ring) and both artifact "Amulets" will
 * parse as "kind 503" (the first of the two artifact amulets),
 * but as long as we use the "name1" field (and not the "kind"
 * or "sval" fields) we should be okay.
 *
 * We sort the two arrays of items names in reverse order, so that
 * we will catch "mace of disruption" before "mace", "Scythe of
 * Slicing" before "Scythe", and for "Ring of XXX" before "Ring".
 *
 * Note that we do not have to parse "plural artifacts" (!)
 *
 * Hack -- This entire routine is a giant hack, but it works
 */
void borg_obj_init(void)
{
    int i, k, n;

    int size;

    s16b what[512];
    cptr text[512];

    char buf[256];


    /*** Item/Ware arrays ***/

    /* Make the inventory array */
    C_MAKE(auto_items, INVEN_TOTAL, auto_item);

    /* Make the stores in the town */
    C_MAKE(auto_shops, 8, auto_shop);


    /*** Plural Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 1; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_info[k];

        /* Skip "empty" items */
        if (!k_ptr->name) continue;

        /* Skip "gold" objects */
        if (k_ptr->tval == TV_GOLD) continue;

        /* Skip "artifacts" */
        if (k_ptr->flags3 & TR3_INSTA_ART) continue;

        /* Hack -- make an item */
        invcopy(&hack, k);

        /* Describe a "plural" object */
        hack.number = 2;
        objdes_store(buf, &hack, FALSE, 0);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort */
    ang_sort(text, what, size);

    /* Save the size */
    auto_plural_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_plural_text, auto_plural_size, cptr);
    C_MAKE(auto_plural_what, auto_plural_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_plural_text[i] = text[i];
    for (i = 0; i < size; i++) auto_plural_what[i] = what[i];


    /*** Singular Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 1; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_info[k];

        /* Skip "empty" items */
        if (!k_ptr->name) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval == TV_GOLD) continue;

        /* Skip "artifacts" */
        if (k_ptr->flags3 & TR3_INSTA_ART) continue;

        /* Hack -- make an item */
        invcopy(&hack, k);

        /* Describe a "singular" object */
        hack.number = 1;
        objdes_store(buf, &hack, FALSE, 0);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Analyze the "INSTA_ART" items */
    for (i = 1; i < MAX_A_IDX; i++) {

        inven_type hack;

        artifact_type *a_ptr = &a_info[i];

        cptr name = (a_name + a_ptr->name);

        /* Skip "empty" items */
        if (!a_ptr->name) continue;

        /* Skip non INSTA_ART things */
        if (!(a_ptr->flags3 & TR3_INSTA_ART)) continue;

        /* Extract the "kind" */
        k = lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Hack -- make an item */
        invcopy(&hack, k);

        /* Save the index */
        hack.name1 = i;

        /* Describe a "singular" object */
        hack.number = 1;
        objdes_store(buf, &hack, FALSE, 0);

        /* Extract the "suffix" length */
        n = strlen(name) + 1;

        /* Remove the "suffix" */
        buf[strlen(buf) - n] = '\0';

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort */
    ang_sort(text, what, size);

    /* Save the size */
    auto_single_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_single_text, auto_single_size, cptr);
    C_MAKE(auto_single_what, auto_single_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_single_text[i] = text[i];
    for (i = 0; i < size; i++) auto_single_what[i] = what[i];


    /*** Artifact and Ego-Item Parsers ***/

    /* No entries yet */
    size = 0;

    /* Collect the "artifact names" */
    for (k = 1; k < MAX_A_IDX; k++) {

        artifact_type *a_ptr = &a_info[k];

        /* Skip non-items */
        if (!a_ptr->name) continue;

        /* Extract a string */
        sprintf(buf, " %s", (a_name + a_ptr->name));

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Collect the "ego-item names" */
    for (k = 1; k < MAX_E_IDX; k++) {

        ego_item_type *e_ptr = &e_info[k];

        /* Skip non-items */
        if (!e_ptr->name) continue;

        /* Extract a string */
        sprintf(buf, " %s", (e_name + e_ptr->name));

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k + 256;
        size++;
    }

    /* Set the sort hooks */
    ang_sort_comp = ang_sort_comp_hook;
    ang_sort_swap = ang_sort_swap_hook;

    /* Sort */
    ang_sort(text, what, size);

    /* Save the size */
    auto_artego_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_artego_text, auto_artego_size, cptr);
    C_MAKE(auto_artego_what, auto_artego_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_artego_text[i] = text[i];
    for (i = 0; i < size; i++) auto_artego_what[i] = what[i];
}



#else

#ifdef MACINTOSH
static int i = 0;
#endif

#endif

