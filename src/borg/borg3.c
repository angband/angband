/* File: borg3.c */

/* Purpose: Object and Spell routines for the Borg -BEN- */

#include "angband.h"
#include "object/tvalsval.h"
#include "cave.h"

#ifdef ALLOW_BORG
#include "borg1.h"
#include "borg3.h"



/*
 * This file helps the Borg analyze "objects" and "shops", and to
 * deal with objects and spells.
 */



/*
 * Some variables
 */

borg_item *borg_items;      /* Current "inventory" */

borg_shop *borg_shops;      /* Current "shops" */



/*
 * Safety arrays for simulating possible worlds
 */

borg_item *safe_items;      /* Safety "inventory" */
borg_item *safe_home;       /* Safety "home stuff" */

borg_shop *safe_shops;      /* Safety "shops" */


/*
 * Spell info
 */

borg_magic borg_magics[9][9];   /* Spell info, by book/what */


/* Food Names */
static char *food_syllable1[] =
{
    "BBQ ", "Boiled ", "Fresh ", "Frozen ", "Burned ", "Rotten ", "Raw ", "Toasted ", "Broiled ", "Baked ", "Fried ", "Buttered ", "Steamed ", "Gramma's ",
};

/* Food Names */
static char *food_syllable2[] =
{
    "Pizza", "Eggs", "Spam", "Oatmeal", "Chicken", "Bacon", "Peanutbutter", "Roast Beef", "Cheese", "Toast", "Hamburger", "Carrots", "Corn", "Potato", "Pork Chops", "Chinese Takeout", "Cookies",
};

/* Slime Molds */
static char *mold_syllable1[] =
{
    "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar", "B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et", "Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha", "Ib", "Jer", "K", "Ka", "Ked", "L", "Loth"
, "Lar", "Leg", "M", "Mir", "N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T", "Tr", "Th", "V", "Y", "Z", "W", "Wic",
};

static char *mold_syllable2[] =
{
    "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra", "ilma", "il-Ga", "ili", "o", "orfi", "u", "y",
};

static char *mold_syllable3[] =
{
    "bur", "fur", "gan", "gnus", "gnar", "li", "lin", "lir", "mli", "nar", "nus", "rin", "ran", "sin", "sil", "sur",
};


/*
 * Hack -- help analyze the magic
 *
 * The comments yield the "name" of the spell or prayer.
 *
 * Also, the leading letter in the comment indicates how we use the
 * spell or prayer, if at all, using "A" for "attack", "D" for "call
 * light" and "detection", "E" for "escape", "H" for healing, "O" for
 * "object manipulation", and "F" for "terrain feature manipulation",
 * plus "!" for entries that can soon be handled.
 */

static byte borg_magic_method[2][9][9] =
{
    /*** Spells ***/

    {
        {
            /* Magic for Beginners (sval 0) */
            BORG_MAGIC_AIM  /* A "Magic Missile" */,
            BORG_MAGIC_EXT  /*   "Detect Monsters" */,
            BORG_MAGIC_NOP  /* E "Phase Door" */,
            BORG_MAGIC_NOP  /* D "Light Area" */,
            BORG_MAGIC_NOP  /*   "Treasure Detection" */,
            BORG_MAGIC_NOP  /* H "Cure Light Wounds" */,
            BORG_MAGIC_NOP  /*   "Find Hidden Traps/Doors" */,
            BORG_MAGIC_AIM  /* D "Stinking Cloud" */,
            BORG_MAGIC_ICK  /* A  */
        },

        {
            /* Conjurings and Tricks (sval 1) */
            BORG_MAGIC_AIM  /*   "Confusion" */,
            BORG_MAGIC_AIM  /* A "Lightning Bolt" */,
            BORG_MAGIC_NOP  /* F "Trap/Door Destruction" */,
            BORG_MAGIC_NOP  /* H "Cure Poison" */,
            BORG_MAGIC_AIM  /*   "Sleep I" */,
            BORG_MAGIC_NOP  /* E "Teleport Self" */,
            BORG_MAGIC_AIM  /* A "Spear of Light" */,
            BORG_MAGIC_AIM  /* A "Frost Bolt" */,
            BORG_MAGIC_AIM  /* A "Wonder" */
        },

        {
            /* Incantations and Illusions (sval 2) */
            BORG_MAGIC_NOP  /* H "Satisfy Hunger" */,
            BORG_MAGIC_OBJ  /* O "Recharge Item I" */,
            BORG_MAGIC_AIM  /*   "Stone to Mud" */,
            BORG_MAGIC_AIM  /*   "Fire Bolt" */,
            BORG_MAGIC_AIM  /* O "Polymorph" */,
            BORG_MAGIC_OBJ  /*   "Identify" */,
            BORG_MAGIC_NOP  /* A "Detect Inv" */,
            BORG_MAGIC_AIM  /*   "Acid Bolt" */,
            BORG_MAGIC_AIM  /* A "Slow Mon"*/
        },

        {
            /* Sorcery and Evocations (sval 3) */
            BORG_MAGIC_AIM  /* A "Frost Ball" */,
            BORG_MAGIC_AIM  /* O "Teleport Other" */,
            BORG_MAGIC_NOP  /*   "Haste Self" */,
            BORG_MAGIC_NOP  /* A "Mass Sleep" */,
            BORG_MAGIC_AIM  /*   "Fire Ball" */,
            BORG_MAGIC_NOP  /*   "Detect Enchant" */,
            BORG_MAGIC_ICK  /*   "(Blank)"*/,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Resistance of Scarabtarices (sval 4) */
            BORG_MAGIC_NOP  /*   "Resist Cold" */,
            BORG_MAGIC_NOP  /*   "Resist Fire" */,
            BORG_MAGIC_NOP  /*   "Resist Pois" */,
            BORG_MAGIC_NOP  /*   "Resistance" */,
            BORG_MAGIC_NOP  /*   "Shield" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Raal's Tome of Destruction (sval 5) */
            BORG_MAGIC_AIM  /* A "Shock Wave" */,
            BORG_MAGIC_AIM  /* A "Explosion" */,
            BORG_MAGIC_AIM  /* A "Cloud Kill" */,
            BORG_MAGIC_AIM  /* A "Acid Kill" */,
            BORG_MAGIC_AIM  /* A "Ice Storm" */,
            BORG_MAGIC_AIM  /* A "Meteor Swarm" */,
            BORG_MAGIC_AIM  /*   "Rift"*/,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Mordenkainen's Escapes (sval 6) */
            BORG_MAGIC_NOP  /*   "Door Creation" */,
            BORG_MAGIC_NOP  /*   "Stair Creation" */,
            BORG_MAGIC_NOP  /*   "Teleport Level" */,
            BORG_MAGIC_NOP  /*   "Word of Recall" */,
            BORG_MAGIC_NOP  /* E "Rune of Protec" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Tenser's transformations... (sval 7) */
            BORG_MAGIC_NOP  /* ! "Heroism" */,
            BORG_MAGIC_NOP  /*   "Berserker" */,
            BORG_MAGIC_OBJ  /* ! "Enchant Armour" */,
            BORG_MAGIC_OBJ  /*   "Enchant Weapon" */,
            BORG_MAGIC_OBJ  /*   "Recharge Item II" */,
            BORG_MAGIC_OBJ  /*   "Elemental Brand" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Kelek's Grimoire of Power (sval 8) */
            BORG_MAGIC_NOP  /*   "Earthquake" */,
            BORG_MAGIC_AIM  /*   "Bedlam" */,
            BORG_MAGIC_AIM  /* O "Rend Sould" */,
            BORG_MAGIC_WHO  /*   "Genocide" */,
            BORG_MAGIC_NOP  /*   "Word of Dest" */,
            BORG_MAGIC_NOP  /*   "Mass Genocide"*/,
            BORG_MAGIC_AIM  /*   "Chaos Strike"*/,
            BORG_MAGIC_AIM  /*   "Mana Storm" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        }

    },


    /*** Prayers ***/

    {
        {
            /* Beginners Handbook (sval 0) */
            BORG_MAGIC_EXT  /*   "Detect Evil" */,
            BORG_MAGIC_NOP  /*   "Cure Light Wounds" */,
            BORG_MAGIC_NOP  /*   "Bless" */,
            BORG_MAGIC_NOP  /* H "Remove Fear" */,
            BORG_MAGIC_NOP  /* D "Call Light" */,
            BORG_MAGIC_NOP  /* D "Detect Doors/Stairs" */,
            BORG_MAGIC_NOP  /*   "Slow Poison" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Words of Wisdom (sval 1) */
            BORG_MAGIC_AIM  /*   "Scare Creature" */,
            BORG_MAGIC_NOP  /* E "Portal" */,
            BORG_MAGIC_NOP  /* H "Cure Serious Wounds" */,
            BORG_MAGIC_NOP  /*   "Chant" */,
            BORG_MAGIC_NOP  /*   "Sanctuary" */,
            BORG_MAGIC_NOP  /* H "Satisfy Hunger" */,
            BORG_MAGIC_NOP  /*   "Remove Curse" */,
            BORG_MAGIC_NOP  /*   "Resist Heat and Cold" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Chants and Blessings (sval 2) */
            BORG_MAGIC_NOP  /* H "Neutralize Poison" */,
            BORG_MAGIC_AIM  /* A "Orb of Draining" */,
            BORG_MAGIC_NOP  /* H "Cure Critical Wounds" */,
            BORG_MAGIC_EXT  /*   "Sense Invisible" */,
            BORG_MAGIC_NOP  /*   "Protection from Evil" */,
            BORG_MAGIC_NOP  /*   "Earthquake" */,
            BORG_MAGIC_NOP  /* D "Sense Surroundings" */,
            BORG_MAGIC_NOP  /* H "Cure Mortal Wounds" */,
            BORG_MAGIC_NOP  /*   "Turn Undead" */
        },

        {
            /* Exorcism and Dispelling (sval 3) */
            BORG_MAGIC_NOP  /*   "Prayer" */,
            BORG_MAGIC_NOP  /* ! "Dispel Undead" */,
            BORG_MAGIC_NOP  /* H "Heal" */,
            BORG_MAGIC_NOP  /* ! "Dispel Evil" */,
            BORG_MAGIC_NOP  /*   "Glyph of Warding" */,
            BORG_MAGIC_NOP  /* ! "Holy Word" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Ethereal openings (sval 4) */
            BORG_MAGIC_NOP  /* E "Blink" */,
            BORG_MAGIC_NOP  /* E "Teleport" */,
            BORG_MAGIC_AIM  /*   "Teleport Away" */,
            BORG_MAGIC_NOP  /*   "Teleport Level" */,
            BORG_MAGIC_NOP  /* E "Word of Recall" */,
            BORG_MAGIC_NOP  /*   "Alter Reality" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Godly Insights... (sval 5) */
            BORG_MAGIC_EXT  /*   "Detect Monsters" */,
            BORG_MAGIC_EXT  /* D "Detection" */,
            BORG_MAGIC_OBJ  /* O "Perception" */,
            BORG_MAGIC_NOP  /*   "Probing" */,
            BORG_MAGIC_NOP  /* D "Clairvoyance" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Purifications and Healing (sval 6) */
            BORG_MAGIC_NOP  /* H "Cure Serious Wounds" */,
            BORG_MAGIC_NOP  /* H "Cure Mortal Wounds" */,
            BORG_MAGIC_NOP  /* H "Healing" */,
            BORG_MAGIC_NOP  /* ! "Restoration" */,
            BORG_MAGIC_NOP  /* ! "Remembrance" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Holy Infusions (sval 7) */
            BORG_MAGIC_NOP  /* F "Unbarring Ways" */,
            BORG_MAGIC_OBJ  /* O "Recharging" */,
            BORG_MAGIC_NOP  /*   "Dispel Curse" */,
            BORG_MAGIC_OBJ  /* O "Enchant Weapon" */,
            BORG_MAGIC_OBJ  /* O "Enchant Armour" */,
            BORG_MAGIC_OBJ  /*   "Elemental Brand" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        },

        {
            /* Wrath of God (sval 8) */
            BORG_MAGIC_NOP  /* ! "Dispel Undead" */,
            BORG_MAGIC_NOP  /* ! "Dispel Evil" */,
            BORG_MAGIC_NOP  /*   "Banishment" */,
            BORG_MAGIC_NOP  /*   "Word of Destruction" */,
            BORG_MAGIC_AIM  /*   "Annihilation" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */,
            BORG_MAGIC_ICK  /*   "(blank)" */
        }
    }
};



/*
 * Hack -- help analyze the magic
 *
 * The comments yield the "name" of the spell or prayer.
 *
 * Also, the leading letter in the comment indicates how we use the
 * spell or prayer, if at all, using "A" for "attack", "D" for "call
 * light" and "detection", "E" for "escape", "H" for healing, "O" for
 * "object manipulation", "F" for "terrain feature manipulation",
 * "X" for "never use this", and "!" for "soon to be handled".
 *
 * The value indicates how much we want to know the spell/prayer.  A
 * rating of zero indicates that the spell/prayer is useless, and should
 * never be learned or used.  A rating from 1 to 49 indicates that the
 * spell/prayer is worth some experience to use once, so we should study
 * (and use) it when we get bored in town.  A rating from 50 to 99 means
 * that the spell/prayer should be learned as soon as possible (and used
 * when bored).
 *
 * XXX XXX XXX Verify ratings.
 */
static byte borg_magic_rating[2][9][9] =
{
    /*** Spells ***/

    {
        {
            /* Magic for Beginners (sval 0) */
            95          /* A "Magic Missile" */,
            85          /*   "Detect Monsters" */,
            75          /* E "Phase Door" */,
            65          /* D "Light Area" */,
            5           /*   "Treasure Detection" */,
            65          /* H   "Object Detection" */,
            85           /*  "Cure Light Wounds" */,
            95          /* D "Find Hidden Traps/Doors" */,
            85          /* A "Stinking Cloud" */
        },

        {
            /* Conjurings and Tricks (sval 1) */
            55           /*   "Confusion" */,
            85          /* A "Lightning Bolt" */,
            55          /* F "Trap/Door Destruction" */,
            65          /* H "Cure Poison" */,
            65          /*   "Sleep I" */,
            95          /* E "Teleport Self" */,
            55          /* A "Spear of Light" */,
            85          /* A "Frost Bolt" */,
            75          /* A "Wonder" */
        },

        {
            /* Incantations and Illusions (sval 2) */
            95          /* H "Satisfy Hunger" */,
            65          /* O "Recharge Item I" */,
            75          /*   "Stone to mud" */,
            75          /*   "Fire Bolt" */,
            55          /* O "PolyMorph" */,
            95          /*   "Identify" */,
            75          /* A "Detect Inv" */,
            75          /*   "Acid Bolt" */,
            55          /*   "Slow Monster" */
        },

        {
            /* Sorcery and Evocations (sval 3) */
            85          /* A "Frost Ball" */,
            75          /* O "Teleport Other" */,
            85          /*   "Haste Self" */,
            65          /*   "Mass Sleep" */,
            75          /* A "Fire Ball" */,
            55          /*   "Detect Enchantment" */,
            0           /*   "blank" */,
            0           /*   "blank" */,
            0           /*   "(blank)" */
        },

        {
            /* Resistance of Scarabtarices (sval 4) */
            65          /*   "Resist Cold" */,
            65          /*   "Resist Fire" */,
            60          /*   "Resist Poison" */,
            70          /*   "Resistance" */,
            75          /*   "Shield" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Raal's Tome of Destruction (sval 5) */
            85          /* A "Shock Wave" */,
            85          /* A "Explosion" */,
            85          /* A "Cloud Kill" */,
            85          /* A "Acid Ball" */,
            85          /* A "Ice Storm" */,
            85          /* A "Meteor Swarm" */,
            85          /*   "Rift" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Mordenkainen's Escapes (sval 6) */
            65          /*   "Door Creation" */,
            5           /*   "Stair Creation" */,
            65          /*   "Teleport Level" */,
            65          /*   "Word of Recall" */,
            55          /* E "Rune of Protection" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Tenser's transformations... (sval 7) */
            75          /* H "Heroism" */,
            75          /*   "Berserker" */,
            75          /* H "Enchant Armor" */,
            75          /*   "Enchant Weapon" */,
            75          /*   "Recharge Item II" */,
            75          /*   "Elemental Brand" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Kelek's Grimoire of Power (sval 8) */
            55          /*   "Earthquake" */,
            5           /*   "Bedlam" */,
            75          /*   "Rend Soul" */,
            75          /*   "Genocide" */,
            55          /*   "Word of Dest" */,
            65          /*   "Mass Genocide" */,
            75          /*   "Chaos Strike" */,
            75          /*   "Mana Storm" */,
            0           /*   "(blank)" */
        }

    },


    /*** Prayers ***/

    {
        {
            /* Beginners Handbook (sval 0) */
            85          /*   "Detect Evil" */,
            55          /* H "Cure Light Wounds" */,
            80          /*   "Bless" */,
            35          /* H "Remove Fear" */,
            35          /* D "Call Light" */,
            75          /* D "Detect Doors/Stairs" */,
            55          /*   "Slow Poison" */,
            0           /*   "(blank)" */,
			0
        },

        {
            /* Words of Wisdom (sval 1) */
            55           /*   "Confuse Creature" */,
            90          /* E "Portal" */,
            55           /* H "Cure Serious Wounds" */,
            55           /*   "Chant" */,
            55           /*   "Sanctuary" */,
            75          /* H "Satisfy Hunger" */,
            35           /*   "Remove Curse" */,
            55           /*   "Resist Heat and Cold" */,
            0           /*   "(blank)" */
        },

        {
            /* Chants and Blessings (sval 2) */
            65          /* H "Neutralize Poison" */,
            95          /* A "Orb of Draining" */,
            55          /* H "Cure Critical Wounds" */,
            65          /*   "Sense Invisible" */,
            65          /*   "Protection from Evil" */,
            55          /*   "Earthquake" */,
            65          /* D "Sense Surroundings" */,
            55          /* H "Cure Mortal Wounds" */,
            55           /*   "Turn Undead" */
        },

        {
            /* Exorcism and Dispelling (sval 3) */
            45          /*   "Prayer" */,
            65          /* ! "Dispel Undead" */,
            55          /* H "Heal" */,
            55          /* ! "Dispel Evil" */,
            55          /*   "Glyph of Warding" */,
            55          /* ! "Holy Word" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Ethereal openings (sval 4) */
            65          /* E "Blink" */,
            65          /* E "Teleport" */,
            55          /*   "Teleport Away" */,
            55          /*   "Teleport Level" */,
            75          /* E "Word of Recall" */,
            65          /*   "Alter Reality" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Godly Insights... (sval 5) */
            55          /*   "Detect Monsters" */,
            65          /* D "Detection" */,
            75          /* O "Perception" */,
            5           /*   "Probing" */,
            65          /* D "Clairvoyance" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Purifications and Healing (sval 6) */
            55          /* H "Cure Serious Wounds" */,
            55          /* H "Cure Mortal Wounds" */,
            55          /* H "Healing" */,
            45          /* ! "Restoration" */,
            45          /* ! "Remembrance" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Holy Infusions (sval 7) */
            50          /* F "Unbarring Ways" */,
            50          /* O "Recharging" */,
            45           /*   "Dispel Curse" */,
            65          /* O "Enchant Weapon" */,
            65          /* O "Enchant Armour" */,
            65          /*   "Elemental Brand" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        },

        {
            /* Wrath of God (sval 8) */
            65           /* ! "Dispel Undead" */,
            65           /* ! "Dispel Evil" */,
            55          /*   "Banishment" */,
            55          /*  "Word of Destruction" */,
            55          /*   "Annihilation" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */,
            0           /*   "(blank)" */
        }
    }
};


/* Cheat the game's index for spells */
static byte borg_magic_index[2][9][9] =
{
    /*** Spells ***/

    {
        {
         /* Magic for Beginners */
            0  /* SPELL_MAGIC_MISSILE */,
            1  /* SPELL_DETECT_MONSTERS */,
            2  /* SPELL_PHASE_DOOR */,
            3  /* SPELL_LIGHT_AREA */,
            6  /* SPELL_TREASURE_DETECTION */,
            5  /* SPELL_CURE_LIGHT_WOUNDS */,
            4  /* SPELL_FIND_TRAPS_DOORS */,
            11 /* SPELL_STINKING_CLOUD */,
			99
         },

        /* Conjurings and Tricks */
        {
            13 /* SPELL_CONFUSE_MONSTER */,
            12 /* SPELL_LIGHTNING_BOLT */,
            19 /* SPELL_TRAP_DOOR_DESTRUCTION */,
            25 /* SPELL_CURE_POISON */,
            14 /* SPELL_SLEEP_MONSTER */,
            30 /* SPELL_TELEPORT_SELF */,
            20 /* SPELL_SPEAR_OF_LIGHT */,
            16 /* SPELL_FROST_BOLT */,
            15 /* SPELL_WONDER */
         },

    /* Incantations and Illusions */
        {
            26 /* SPELL_SATISFY_HUNGER */,
            50 /* SPELL_RECHARGE_ITEM_I */,
            21 /* SPELL_TURN_STONE_TO_MUD */,
            18 /* SPELL_FIRE_BOLT */,
            35 /* SPELL_POLYMORPH_OTHER */,
            8  /* SPELL_IDENTIFY */,
            9  /* SPELL_DETECT_INVISIBLE */,
            17 /* SPELL_ACID_BOLT */,
            31 /* SPELL_SLOW_MONSTER */
         },

    /* Sorcery and Evocations */
        {
            55 /* SPELL_FROST_BALL */,
            32 /* SPELL_TELEPORT_OTHER */,
            29 /* SPELL_HASTE_SELF */,
            39 /* SPELL_MASS_SLEEP */,
            57 /* SPELL_FIRE_BALL */,
            10 /* SPELL_DETECT_ENCHANTMENT */,
            99,
            99,
            99
        },

    /* Resistances of Scarabtarices */
        {
            44 /* SPELL_RESIST_COLD */,
            45 /* SPELL_RESIST_FIRE */,
            46 /* SPELL_RESIST_POISON */,
            47 /* SPELL_RESISTANCE */,
            48 /* SPELL_SHIELD */,
            99,
            99,
            99,
            99
        },

    /* Raal's Tome of Destruction */
        {
            36 /* SPELL_SHOCK_WAVE */,
            37 /* SPELL_EXPLOSION */,
            38 /* SPELL_CLOUD_KILL */,
            56 /* SPELL_ACID_BALL */,
            58 /* SPELL_ICE_STORM */,
            60 /* SPELL_METEOR_SWARM */,
            62 /* SPELL_RIFT */,
            99,
            99
        },

    /* Mordenkainen's Escapes */
        {
            22 /* SPELL_DOOR_CREATION */,
            24 /* SPELL_STAIR_CREATION */,
            33 /* SPELL_TELEPORT_LEVEL */,
            34 /* SPELL_WORD_OF_RECALL */,
            49 /* SPELL_RUNE_OF_PROTECTION */,
            99,
            99,
            99,
            99
         },

    /* Tenser's transformations */
        {
            27 /* SPELL_HEROISM */,
            28 /* SPELL_BERSERKER */,
            51 /* SPELL_ENCHANT_ARMOR */,
            52 /* SPELL_ENCHANT_WEAPON */,
            53 /* SPELL_RECHARGE_ITEM_II */,
            54 /* SPELL_ELEMENTAL_BRAND */,
            99,
            99,
            99
        },

    /* Kelek's Grimoire of Power */
        {
            23 /* SPELL_EARTHQUAKE */,
            40 /* SPELL_BEDLAM */,
            41 /* SPELL_REND_SOUL */,
            59 /* SPELL_GENOCIDE */,
            42 /* SPELL_WORD_OF_DESTRUCTION */,
            61 /* SPELL_MASS_GENOCIDE */,
            43 /* SPELL_CHAOS_STRIKE */,
            63 /* SPELL_MANA_STORM */,
            99
        }

    },

    {
        /*** Priest spell books ***/
        {
            0 /* a PRAYER_DETECT_EVIL */,
            1 /* b PRAYER_CURE_LIGHT_WOUNDS */,
            2 /* c PRAYER_BLESS */,
            3 /* d PRAYER_REMOVE_FEAR */,
            4 /* e PRAYER_CALL_LIGHT */,
            5 /* f PRAYER_FIND_TRAPS */,
            7 /* g PRAYER_SLOW_POISON */,
            99,
			99
        },

        {
            8 /* PRAYER_SCARE_MONSTER */,
            9 /* PRAYER_PORTAL */,
            10/* PRAYER_CURE_SERIOUS_WOUNDS */,
            11 /* PRAYER_CHANT */,
            12 /* PRAYER_SANCTUARY */,
            13 /* PRAYER_SATISFY_HUNGER */,
            14 /* PRAYER_REMOVE_CURSE */,
            15 /* PRAYER_RESIST_HEAT_COLD */,
            99
        },

        {
            16 /* PRAYER_NEUTRALIZE_POISON */,
            17 /* PRAYER_ORB_OF_DRAINING */,
            18 /* PRAYER_CURE_CRITICAL_WOUNDS */,
            19 /* PRAYER_SENSE_INVISIBLE */,
            20 /* PRAYER_PROTECTION_FROM_EVIL */,
            21 /* PRAYER_EARTHQUAKE */,
            22 /* PRAYER_SENSE_SURROUNDINGS */,
            23 /* PRAYER_CURE_MORTAL_WOUNDS */,
            24 /* PRAYER_TURN_UNDEAD */
        },

        {
            25 /* PRAYER_PRAYER */,
            26 /* PRAYER_DISPEL_UNDEAD */,
            27 /* PRAYER_HEAL */,
            28 /* PRAYER_DISPEL_EVIL */,
            29 /* PRAYER_GLYPH_OF_WARDING */,
            30 /* PRAYER_HOLY_WORD */,
            99,
            99,
            99
        },

        {
            52  /* PRAYER_BLINK */,
            53  /* PRAYER_TELEPORT_SELF */,
            54  /* PRAYER_TELEPORT_OTHER */,
            55  /* PRAYER_TELEPORT_LEVEL */,
            56  /* PRAYER_WORD_OF_RECALL */,
            57  /* PRAYER_ALTER_REALITY */,
            99,
            99,
            99
        },

        {
            31 /* PRAYER_DETECT_MONSTERS */,
            32 /* PRAYER_DETECTION */,
            33 /* PRAYER_PERCEPTION */,
            34 /* PRAYER_PROBING */,
            35 /* PRAYER_CLAIRVOYANCE */,
            99,
            99,
            99,
            99
        },

        {
            36  /* PRAYER_CURE_SERIOUS_WOUNDS2 */,
            37  /* PRAYER_CURE_MORTAL_WOUNDS2 */,
            38  /* PRAYER_HEALING */,
            39  /* PRAYER_RESTORATION */,
            40  /* PRAYER_REMEMBRANCE */,
            99,
            99,
            99,
            99
        },

        {
            46  /* PRAYER_UNBARRING_WAYS */,
            47  /* PRAYER_RECHARGING */,
            48  /* PRAYER_DISPEL_CURSE */,
            49  /* PRAYER_ENCHANT_WEAPON */,
            50  /* PRAYER_ENCHANT_ARMOUR */,
            51  /* PRAYER_ELEMENTAL_BRAND */,
            99,
            99,
            99
        },

        {
            41  /* PRAYER_DISPEL_UNDEAD2 */,
            42  /* PRAYER_DISPEL_EVIL2 */,
            43  /* PRAYER_BANISHMENT */,
            44  /* PRAYER_WORD_OF_DESTRUCTION */,
            45  /* PRAYER_ANNIHILATION */,
            99,
            99,
            99,
            99
        }
    }
};

static cptr borg_magic_name[2][9][9] =
{
    /*** Spells ***/

    {
        {
            /* Magic for Beginners (sval 0) */
            "Magic Missile" ,
            "Detect Monsters" ,
            "Phase Door" ,
            "Light Area" ,
            "Treasure Detection" ,
            "Cure Light Wounds" ,
            "Find Traps, Doors & Stairs" ,
            "Stinking Cloud",
			"(Blank)"

        },

        {
            /* Conjurings and Tricks (sval 1) */
            "Confusion" ,
            "Lightning Bolt" ,
            "Trap/Door Destruction" ,
            "Cure Poison" ,
            "Sleep Monster" ,
            "Teleport Self" ,
            "Spear of Light" ,
            "Frost Bolt" ,
            "Wonder"
        },

        {
            /* Incantations and Illusions (sval 2) */
            "Satisfy Hunger" ,
            "Lesser Recharging" ,
            "Turn Stone to Mud" ,
            "Fire Bolt" ,
            "PolyMorph Other" ,
            "Identify" ,
            "Reveal Monsters" ,
            "Acid Bolt" ,
            "Slow Monster"
        },

        {
            /* Sorcery and Evocations (sval 3) */
            "Frost Ball" ,
            "Teleport Other" ,
            "Haste Self" ,
            "Mass Sleep" ,
            "Fire Ball" ,
            "Detect Enchantment" ,
            "0x3x6" ,
            "0x3x7" ,
            "0x3x8"
        },

        {
            /* Resistance of Scarabtarices (sval 4) */
             "Resist Cold" ,
             "Resist Fire" ,
             "Resist Poison" ,
             "Resistance" ,
             "Shield" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Raal's Tome of Destruction (sval 5) */
             "Shock Wave" ,
             "Explosion" ,
             "Cloud Kill" ,
             "Acid Ball" ,
             "Ice Storm" ,
             "Meteor Swarm" ,
             "Rift" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Mordenkainen's Escapes (sval 6) */
             "Door Creation" ,
             "Stair Creation" ,
             "Teleport Level" ,
             "Word of Recall" ,
             "Rune of Protection" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Tenser's transformations... (sval 7) */
             "Heroism" ,
             "Berserker" ,
             "Enchant Armor" ,
             "Enchant Weapon" ,
             "Greater Recharging" ,
             "Elemental Brand" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Kelek's Grimoire of Power (sval 8) */
             "Earthquake" ,
             "Bedlam" ,
             "Rend Soul" ,
             "Banishment" ,
             "Word of Dest" ,
             "Mass Genocide" ,
             "Chaos Strike" ,
             "Mana Storm" ,
             "(blank)"
        }

    },


    /*** Prayers ***/

    {
        {
            /* Beginners Handbook (sval 0) */
             "Detect Evil" ,
             "Cure Light Wounds" ,
             "Bless" ,
             "Remove Fear" ,
             "Call Light" ,
             "Find Traps, Doors & Stairs" ,
             "Slow Poison" ,
             "(blank)",
             "(blank)"
        },

        {
            /* Words of Wisdom (sval 1) */
             "Scare Monster" ,
             "Portal" ,
             "Cure Serious Wounds" ,
             "Chant" ,
             "Sanctuary" ,
             "Satisfy Hunger" ,
             "Remove Curse" ,
             "Resist Heat and Cold" ,
             "(blank)"
        },

        {
            /* Chants and Blessings (sval 2) */
             "Neutralize Poison" ,
             "Orb of Draining" ,
             "Cure Critical Wounds" ,
             "Sense Invisible" ,
             "Protection from Evil" ,
             "Earthquake" ,
             "Sense Surroundings" ,
             "Cure Mortal Wounds" ,
             "Turn Undead"
        },

        {
            /* Exorcism and Dispelling (sval 3) */
             "Prayer" ,
             "Dispel Undead" ,
             "Heal" ,
             "Dispel Evil" ,
             "Glyph of Warding" ,
             "Holy Word" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Ethereal openings (sval 4) */
             "Blink" ,
             "Teleport Self" ,
             "Teleport Other" ,
             "Teleport Level" ,
             "Word of Recall" ,
             "Alter Reality" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Godly Insights... (sval 5) */
             "Detect Monsters" ,
             "Detection" ,
             "Perception" ,
             "Probing" ,
             "Clairvoyance" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Purifications and Healing (sval 6) */
             "Cure Serious Wounds" ,
             "Cure Mortal Wounds" ,
             "Healing" ,
             "Restoration" ,
             "Remembrance" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Holy Infusions (sval 7) */
             "Unbarring Ways" ,
             "Recharging" ,
             "Dispel Curse" ,
             "Enchant Weapon" ,
             "Enchant Armour" ,
             "Elemental Brand" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        },

        {
            /* Wrath of God (sval 8) */
             "Dispel Undead" ,
             "Dispel Evil" ,
             "Banishment" ,
             "Word of Destruction" ,
             "Annihilation" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)" ,
             "(blank)"
        }
    }
};

/*
 * Constant "item description parsers" (singles)
 */
static int borg_single_size;        /* Number of "singles" */
static s16b *borg_single_what;      /* Kind indexes for "singles" */
static cptr *borg_single_text;      /* Textual prefixes for "singles" */

/*
 * Constant "item description parsers" (plurals)
 */
static int borg_plural_size;        /* Number of "plurals" */
static s16b *borg_plural_what;      /* Kind index for "plurals" */
static cptr *borg_plural_text;      /* Textual prefixes for "plurals" */
static cptr *borg_sv_plural_text;   /* Save Textual prefixes for "plurals" (in kind order) */

/*
 * Constant "item description parsers" (suffixes)
 */
static int borg_artego_size;        /* Number of "artegos" */
static s16b *borg_artego_what;      /* Indexes for "artegos" */
static cptr *borg_artego_text;      /* Textual prefixes for "artegos" */
static cptr *borg_sv_art_text;      /* Save textual prefixes for "artifacts" (in kind order) */

/*
 * Return the slot that items of the given type are wielded into
 *
 * Note that "rings" are tough because there are two slots
 *
 * Returns "-1" if the item cannot (or should not) be wielded
 */
int borg_wield_slot(borg_item *item)
{
        switch (borg_class)
           {
            case CLASS_WARRIOR:
            {
                if ((item->tval == TV_SWORD) ||
                (item->tval == TV_POLEARM) ||
                (item->tval == TV_HAFTED) ||
                (item->tval == TV_DIGGING) ) return (INVEN_WIELD);

                if ((item->tval == TV_DRAG_ARMOR) ||
                (item->tval == TV_HARD_ARMOR) ||
                (item->tval == TV_SOFT_ARMOR) ) return (INVEN_BODY);

                if (item->tval == TV_SHIELD) return (INVEN_ARM);

                if ((item->tval == TV_CROWN) ||
                (item->tval == TV_HELM) ) return (INVEN_HEAD);

                if (item->tval == TV_BOW) return (INVEN_BOW);

                if (item->tval == TV_RING) return (INVEN_LEFT);

                if (item->tval == TV_AMULET) return (INVEN_NECK);

                if (item->tval == TV_LIGHT) return (INVEN_LIGHT);

                if (item->tval == TV_CLOAK) return (INVEN_OUTER);

                if (item->tval == TV_GLOVES) return (INVEN_HANDS);

                if (item->tval == TV_BOOTS) return (INVEN_FEET);
            }
                break;

            case CLASS_MAGE:
            {
                if ((item->tval == TV_SWORD) ||
                (item->tval == TV_POLEARM) ||
                (item->tval == TV_HAFTED) ||
                (item->tval == TV_DIGGING) ) return (INVEN_WIELD);

                if ((item->tval == TV_DRAG_ARMOR) ||
                (item->tval == TV_HARD_ARMOR) ||
                (item->tval == TV_SOFT_ARMOR) ) return (INVEN_BODY);

                if (item->tval == TV_SHIELD) return (INVEN_ARM);

                if ((item->tval == TV_CROWN) ||
                (item->tval == TV_HELM) ) return (INVEN_HEAD);

                if (item->tval == TV_BOW) return (INVEN_BOW);

                if (item->tval == TV_RING) return (INVEN_LEFT);

                if (item->tval == TV_AMULET) return (INVEN_NECK);

                if (item->tval == TV_LIGHT) return (INVEN_LIGHT);

                if (item->tval == TV_CLOAK) return (INVEN_OUTER);

                if (item->tval == TV_GLOVES) return (INVEN_HANDS);

                if (item->tval == TV_BOOTS) return (INVEN_FEET);
            }
                break;

            case CLASS_PRIEST:
            {
                if ((item->tval == TV_SWORD) ||
                (item->tval == TV_POLEARM) ||
                (item->tval == TV_HAFTED) ||
                (item->tval == TV_DIGGING) ) return (INVEN_WIELD);

                if ((item->tval == TV_DRAG_ARMOR) ||
                (item->tval == TV_HARD_ARMOR) ||
                (item->tval == TV_SOFT_ARMOR) ) return (INVEN_BODY);

                if (item->tval == TV_SHIELD) return (INVEN_ARM);

                if ((item->tval == TV_CROWN) ||
                (item->tval == TV_HELM) ) return (INVEN_HEAD);

                if (item->tval == TV_BOW) return (INVEN_BOW);

                if (item->tval == TV_RING) return (INVEN_LEFT);

                if (item->tval == TV_AMULET) return (INVEN_NECK);

                if (item->tval == TV_LIGHT) return (INVEN_LIGHT);

                if (item->tval == TV_CLOAK) return (INVEN_OUTER);

                if (item->tval == TV_GLOVES) return (INVEN_HANDS);

                if (item->tval == TV_BOOTS) return (INVEN_FEET);
            }
                break;

            case CLASS_ROGUE:
            {
                if ((item->tval == TV_SWORD) ||
                (item->tval == TV_POLEARM) ||
                (item->tval == TV_HAFTED) ||
                (item->tval == TV_DIGGING) ) return (INVEN_WIELD);

                if ((item->tval == TV_DRAG_ARMOR) ||
                (item->tval == TV_HARD_ARMOR) ||
                (item->tval == TV_SOFT_ARMOR) ) return (INVEN_BODY);

                if (item->tval == TV_SHIELD) return (INVEN_ARM);

                if ((item->tval == TV_CROWN) ||
                (item->tval == TV_HELM) ) return (INVEN_HEAD);

                if (item->tval == TV_BOW) return (INVEN_BOW);

                if (item->tval == TV_RING) return (INVEN_LEFT);

                if (item->tval == TV_AMULET) return (INVEN_NECK);

                if (item->tval == TV_LIGHT) return (INVEN_LIGHT);

                if (item->tval == TV_CLOAK) return (INVEN_OUTER);

                if (item->tval == TV_GLOVES) return (INVEN_HANDS);

                if (item->tval == TV_BOOTS) return (INVEN_FEET);
            }
                break;

            case CLASS_PALADIN:
            {
                if ((item->tval == TV_SWORD) ||
                (item->tval == TV_POLEARM) ||
                (item->tval == TV_HAFTED) ||
                (item->tval == TV_DIGGING) ) return (INVEN_WIELD);

                if ((item->tval == TV_DRAG_ARMOR) ||
                (item->tval == TV_HARD_ARMOR) ||
                (item->tval == TV_SOFT_ARMOR) ) return (INVEN_BODY);

                if (item->tval == TV_SHIELD) return (INVEN_ARM);

                if ((item->tval == TV_CROWN) ||
                (item->tval == TV_HELM) ) return (INVEN_HEAD);

                if (item->tval == TV_BOW) return (INVEN_BOW);

                if (item->tval == TV_RING) return (INVEN_LEFT);

                if (item->tval == TV_AMULET) return (INVEN_NECK);

                if (item->tval == TV_LIGHT) return (INVEN_LIGHT);

                if (item->tval == TV_CLOAK) return (INVEN_OUTER);

                if (item->tval == TV_GLOVES) return (INVEN_HANDS);

                if (item->tval == TV_BOOTS) return (INVEN_FEET);
            }
                break;

            case CLASS_RANGER:
            {
                if ((item->tval == TV_SWORD) ||
                (item->tval == TV_POLEARM) ||
                (item->tval == TV_HAFTED) ||
                (item->tval == TV_DIGGING) ) return (INVEN_WIELD);

                if ((item->tval == TV_DRAG_ARMOR) ||
                (item->tval == TV_HARD_ARMOR) ||
                (item->tval == TV_SOFT_ARMOR) ) return (INVEN_BODY);

                if (item->tval == TV_SHIELD) return (INVEN_ARM);

                if ((item->tval == TV_CROWN) ||
                (item->tval == TV_HELM) ) return (INVEN_HEAD);

                if (item->tval == TV_BOW) return (INVEN_BOW);

                if (item->tval == TV_RING) return (INVEN_LEFT);

                if (item->tval == TV_AMULET) return (INVEN_NECK);

                if (item->tval == TV_LIGHT) return (INVEN_LIGHT);

                if (item->tval == TV_CLOAK) return (INVEN_OUTER);

                if (item->tval == TV_GLOVES) return (INVEN_HANDS);

                if (item->tval == TV_BOOTS) return (INVEN_FEET);
            }
                break;

        }

    /* No slot available */
    return (-1);
}

/*
 * Get the *ID information
 *
 * This function pulls the information from the screen if it is not passed
 * a *real* item.  It is only passed in *real* items if the borg is allowed
 * to 'cheat' for inventory.
 * This function returns TRUE if space needs to be pressed
 */
bool borg_object_star_id_aux(borg_item *borg_item, object_type *real_item)
{
	bitflag f[OF_SIZE];
	int i = OF_SIZE;

    /* the data directly from the real item    */
    object_flags(real_item, f);
	for (i = 0; i < 12 && i < OF_SIZE; i++) borg_item->flags[i] = f[i];

    borg_item->needs_I = FALSE;

    return (FALSE);
}

/*
 * Look for an item that needs to be analysed because it has been *ID*d
 *
 * This will go through inventory and look for items that were just*ID*'d
 * and examine them for their bonuses.
 */
bool borg_object_star_id( void )
{
    int i;

    /* look in inventory and equiptment for something to *id* */
    for (i = 0; i < QUIVER_END; i++) /* or INVEN_TOTAL */
    {

        borg_item *item = &borg_items[i];
		ego_item_type *e_ptr = &e_info[item->name2];

        if (borg_items[i].needs_I)
        {
            /* cheat to get the information. */
            borg_object_star_id_aux( &borg_items[i], &p_ptr->inventory[i]);

            /* inscribe certain objects */
            if (!borg_skill[BI_CDEPTH] &&
                (e_ptr->xtra == OBJECT_XTRA_TYPE_RESIST || e_ptr->xtra == OBJECT_XTRA_TYPE_POWER) &&
                (streq(item->note, "{ }")  || streq(item->note, "")  || strstr(item->note, "uncursed")))
            {

                /* make the inscription */
                borg_keypress('{');

                if (i >= INVEN_WIELD)
                {
                borg_keypress('/');
                borg_keypress(I2A(i - INVEN_WIELD));
                }
                else
                {
                borg_keypress(I2A(i));
                }

                if (of_has(item->flags, OF_SPEED))
                {
                    borg_keypresses("Spd");
                }
                /* slays and immunities */
                if (of_has(item->flags, OF_RES_POIS))
                {
                	borg_keypresses("Poisn");
                }
                if (of_has(item->flags, OF_IM_FIRE))
                {
                	borg_keypresses("IFir");
                }
                if (of_has(item->flags, OF_IM_COLD))
                {
                	borg_keypresses("ICld");
                }
                if (of_has(item->flags, OF_IM_ACID))
                {
                	borg_keypresses("IAcd");
                }
                if (of_has(item->flags, OF_IM_ELEC))
                {
                	borg_keypresses("IElc");
                }
                if (of_has(item->flags, OF_RES_LIGHT))
                {
                    borg_keypresses("Lite");
                }
                if (of_has(item->flags, OF_RES_DARK))
                {
                    borg_keypresses("Dark");
                }
                if (of_has(item->flags, OF_RES_BLIND))
                {
                    borg_keypresses("Blnd");
                }
                if (of_has(item->flags, OF_RES_CONFU))
                {
                    borg_keypresses("Conf");
                }
                if (of_has(item->flags, OF_RES_SOUND))
                {
                    borg_keypresses("Sound");
                }
                if (of_has(item->flags, OF_RES_SHARD))
                {
                    borg_keypresses("Shrd");
                }
                if (of_has(item->flags, OF_RES_NETHR))
                {
                    borg_keypresses("Nthr");
                }
                if (of_has(item->flags, OF_RES_NEXUS))
                {
                    borg_keypresses("Nxs");
                }
                if (of_has(item->flags, OF_RES_CHAOS))
                {
                    borg_keypresses("Chaos");
                }
                if (of_has(item->flags, OF_RES_DISEN))
                {
                    borg_keypresses("Disn");
                }
				/* TR2_activate was removed */
				if ((item->name1 && a_info[item->name1].effect) ||
				    (k_info[item->kind].effect))
				{
                    borg_keypresses("Actv");
                }
                if (of_has(item->flags, OF_TELEPATHY))
                {
                    borg_keypresses("ESP");
                }
                if (of_has(item->flags, OF_HOLD_LIFE))
                {
                    borg_keypresses("HL");
                }
                if (of_has(item->flags, OF_FREE_ACT))
                {
                    borg_keypresses("FA");
                }
                if (of_has(item->flags, OF_SEE_INVIS))
                {
                    borg_keypresses("SInv");
                }

                /* end the inscription */
                borg_keypress('\n');

            }

        }

    }
    return (FALSE);
}



/*
 * Determine the "base price" of a known item (see below)
 *
 * This function is adapted from "object_value_known()".
 *
 * This routine is called only by "borg_item_analyze()", which
 * uses this function to guess at the "value" of an item, if it
 * was to be sold to a store, with perfect "charisma" modifiers.
 */
static s32b borg_object_value_known(borg_item *item)
{
    s32b value;


    object_kind *k_ptr = &k_info[item->kind];

    /* Worthless items */
    if (!k_ptr->cost) return (0L);

    /* Extract the base value */
    value = k_ptr->cost;


    /* Hack -- use artifact base costs */
    if (item->name1)
    {
        artifact_type *a_ptr = &a_info[item->name1];

        /* Worthless artifacts */
        if (!a_ptr->cost) return (0L);

        /* Hack -- use the artifact cost */
        value = a_ptr->cost;
    }

    /* Hack -- add in ego-item bonus cost */
    if (item->name2)
    {
        ego_item_type *e_ptr = &e_info[item->name2];

        /* Worthless ego-items */
        if (!e_ptr->cost) return (0L);

        /* Hack -- reward the ego-item cost */
        value += e_ptr->cost;
    }

    /* Analyze pval bonus */
    switch (item->tval)
    {
        /* Wands/Staffs */
        case TV_WAND:
        case TV_STAFF:
        {
            /* Pay extra for charges */
            value += ((value / 20) * item->pval);

            break;
        }

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
        case TV_LIGHT:
        case TV_AMULET:
        case TV_RING:
        {
            /* Hack -- Negative "pval" is always bad */
            if (item->pval < 0) return (0L);

            /* No pval */
            if (!item->pval) break;

            /* Give credit for stat bonuses */
            if (of_has(item->flags, OF_STR)) value += (item->pval * 200L);
            if (of_has(item->flags, OF_INT)) value += (item->pval * 200L);
            if (of_has(item->flags, OF_WIS)) value += (item->pval * 200L);
            if (of_has(item->flags, OF_DEX)) value += (item->pval * 200L);
            if (of_has(item->flags, OF_CON)) value += (item->pval * 200L);
            if (of_has(item->flags, OF_CHR)) value += (item->pval * 200L);

            /* Give credit for stealth and searching */
            if (of_has(item->flags, OF_STEALTH)) value += (item->pval * 100L);
            if (of_has(item->flags, OF_SEARCH)) value += (item->pval * 100L);

            /* Give credit for infra-vision and tunneling */
            if (of_has(item->flags, OF_INFRA)) value += (item->pval * 50L);
            if (of_has(item->flags, OF_TUNNEL)) value += (item->pval * 50L);

            /* Give credit for extra attacks */
            if (of_has(item->flags, OF_BLOWS)) value += (item->pval * 2000L);

            /* Give credit for speed bonus */
            if (of_has(item->flags, OF_SPEED)) value += (item->pval * 30000L);

            break;
        }
    }


    /* Analyze the item */
    switch (item->tval)
    {
        /* Rings/Amulets */
        case TV_RING:
        case TV_AMULET:
        {
            /* Hack -- negative bonuses are bad */
            if (item->to_a < 0) return (0L);
            if (item->to_h < 0) return (0L);
            if (item->to_d < 0) return (0L);

            /* Give credit for bonuses */
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

            break;
        }

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
        {
            /* Hack -- negative armor bonus */
            if (item->to_a < 0) return (0L);

            /* Give credit for bonuses */
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

            break;
        }

        /* Bows/Weapons */
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_SWORD:
        case TV_POLEARM:
        {
            /* Hack -- negative hit/damage bonuses */
            if (item->to_h + item->to_d < 0) return (0L);

            /* Factor in the bonuses */
            value += ((item->to_h + item->to_d + item->to_a) * 100L);

            /* Hack -- Factor in extra damage dice */
            if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds))
            {
                value += (item->dd - k_ptr->dd) * item->ds * 200L;
            }

            break;
        }

        /* Ammo */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        {
            /* Hack -- negative hit/damage bonuses */
            if (item->to_h + item->to_d < 0) return (0L);

            /* Factor in the bonuses */
            value += ((item->to_h + item->to_d) * 5L);

            /* Hack -- Factor in extra damage dice */
            if ((item->dd > k_ptr->dd) && (item->ds == k_ptr->ds))
            {
                value += (item->dd - k_ptr->dd) * item->ds * 5L;
            }

            break;
        }
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
 * Note the use of a "prefix binary search" on the arrays of object
 * base names, and on the arrays of artifact/ego-item special names.
 *
 * A "prefix binary search" starts out just like a normal binary search,
 * in that it searches a sorted array of entries for a specific entry,
 * using a simple "less than or equal to" comparison.  When it finds
 * an entry, however, instead of simply checking for "equality" of the
 * entry to the key, it must check whether the key is a "prefix" of the
 * entry.  And if any entry can be a prefix of another entry, then it
 * must check whether the key is a "prefix" of any of the entries which
 * precede the "found" entry.  Technically, it only has to check the
 * preceding N entries, where N is the maximum distance between any two
 * entries sharing a "prefix" relation, but note that only in the case
 * of "failure" will the search need to check more than a few entries,
 * even if it scans all the way to the start of the list.
 *
 * We use the object kind to guess at the object weight and flags, and
 * then we use the artifact/ego-item information to update our guesses.
 *
 * We also guess at the value of the item, as given by "object_value()".
 *
 * Note that we will fail if the "description" was "partial", that is,
 * if it was "broken" by the display functions for any reason.  This
 * should only be an issue in "stores", which "chop" the description
 * to a length of about 60 characters, which may be "messy".  Luckily,
 * objects in stores never have important inscriptions, and we should
 * correctly handle objects with "bizarre" inscriptions, or even with
 * "broken" inscriptions, so we should be okay.
 */
void borg_item_analyze(borg_item *item, object_type *real_item, cptr desc)
{
	object_kind *k_ptr;
	bitflag f[OF_SIZE];
	bitflag known_f[OF_SIZE];

    char *scan;
	int i;


    /* Wipe the item */
    WIPE(item, borg_item);

	/* Extract the flags */
	object_flags(real_item, f);
	object_flags_known(real_item, known_f);

	/* Save the item description */
    strcpy(item->desc, desc);

    /* Advance to the "inscription" or end of string */
    for (scan = item->desc; *scan && (*scan != c1); scan++) /* loop */;

    /* Save a pointer to the inscription */
    item->note = scan;

    /* Empty item */
    if (!desc[0]) return;
	if (strstr(desc, "(nothing)")) return;


    /* Assume singular */
    item->iqty = real_item->number;

	/* empty item, leave here */
	if (item->iqty == 0) return;

	/* TVal and SVal */
	item->tval = real_item->tval;

	/* This is only known on some items if not id'd/known */
	item->sval = real_item->sval;

	/* Some Sense but not necessarily real ID
	 * some (Easy Know + Aware) items might sneak in here.
	 */
    item->aware = object_is_known(real_item);
	if (!item->aware)
	{
		if (object_flavor_is_aware(real_item)) item->kind = real_item->kind->kidx;

		/* item has some awareness */
		if (item->kind) item->aware = TRUE;
	}

	/* Item has been ID'd (store, scroll, spell) */
  	if ((real_item->ident & IDENT_KNOWN) ||
		(real_item->ident & IDENT_ATTACK) ||
		/*(real_item->ident & IDENT_FIRED)  || */
		(real_item->ident & IDENT_DEFENCE) ||
  	    (real_item->ident & IDENT_STORE) ||
		(real_item->ident & IDENT_SENSE)) item->ident = TRUE;

	/* Descriptions from PseudoID trump the IDENT_SENSE */
	if (strstr(item->note, "magical") ||
		strstr(item->note, "excellent") ||
		strstr(item->note, "ego") ||
		strstr(item->note, "splendid") ||
		strstr(item->note, "special") ||
		strstr(item->note, "terrible") ||
		strstr(item->note, "indestructible")) item->ident = FALSE;

	/* Item has been *ID*'d (store, scroll, spell) */
  	if ((real_item->ident & IDENT_KNOWN) /*||
  	    (real_item->ident & IDENT_MENTAL)*/) item->fully_identified = TRUE;

    /* Kind index -- Only if partially ID*/
    if (item->aware) item->kind = real_item->kind->kidx;

	/* Some items do not need full ID */
	if (real_item->tval == TV_SCROLL && item->kind) item->ident = TRUE;
	if (item->kind && easy_know(real_item)) item->ident = TRUE;

	/* power value -- Only if ID'd */
  	if (item->ident) item->pval = real_item->pval[DEFAULT_PVAL];

	/* does it have an activation or effect? */
	item->activation = k_info[item->kind].effect;

	/* Rods are considered pval 1 if charged */
  	if (item->tval == TV_ROD)
  	{
		char *buf;

		k_ptr = &k_info[real_item->kind->kidx];

		if (item->iqty == 1 && real_item->timeout) item->pval = 0;
  		else
		{
			if (strstr(item->desc, "charging"))
			{
				cptr s;
				int number;

				/* Assume all are charging */
				item->pval = 0;

				/* Find the first "(" */
				for (s = desc; *s && (*s != '('); s++) /* loop */;

				desc = s + 1;

				if (isdigit(desc[0]))
				{
					cptr ss;

					/* Find the first space */
					for (ss = desc; *ss && (*ss != ' '); ss++) /* loop */;

					/* Paranoia -- Catch sillyness */
					if (*ss != ' ')
					{
						number = item->iqty;
					}
					else
					{
						/* Extract a quantity */
						number = atoi(desc);
					}

					/* Quantity more than number charging */
					if (item->iqty > number)
					{
						item->pval = 1;
					}
				}
			}
  			else item->pval = 1;
		}
	}

	/* Staves and Wands are considered charged unless
	 * they are known to be empty or are {empty}
	 */
	if (item->tval == TV_STAFF || item->tval == TV_WAND)
	{
		/* assume good */
		item->pval =1;

		/* if Known, get correct pval */
		if (item->ident) item->pval = real_item->pval[DEFAULT_PVAL];

		/* Gotta know charges */
		/* if (!object_known_p(real_item)) item->pval = 0; */

		/* if seen {empty} assume pval 0 */
		if (real_item->ident & IDENT_EMPTY) item->pval = 0;
		if (strstr(item->desc, "(0 charges)")) item->pval = 0;
	}

  	/* Weight of item */
  	item->weight = real_item->weight;

	/* Index known if ID'd */
	if (item->ident)
	{
  		/* Artifact Index --Only known if ID'd*/
  		item->name1 = real_item->artifact->aidx;

  		/* Ego Index --Only known if ID'd*/
  		item->name2 = real_item->ego->eidx;
	}

	/* Timeout, must wait for recharge */
  	item->timeout = real_item->timeout;

    /* Modifiers -- Only known if ID'd */
    if (item->ident)
    {
		item->to_h = real_item->to_h;/* Bonus to hit */
    	item->to_d = real_item->to_d;/* Bonus to dam */
    	item->to_a = real_item->to_a;/* Bonus to ac */
	}

	/* Attributes known */
	item->ac = real_item->ac;    /* Armor class */
    item->dd = real_item->dd;    /* Damage dice */
    item->ds = real_item->ds;    /* Damage sides */

    /* Level of item */
    item->level = k_info[item->kind].level;

    /* Extract the base flags -- Kind only given if 'able' */
	for (i = 0; i < 12 && i < OF_SIZE; i++) item->flags[i] = f[i];

    /* Base Cost -- Guess */

    /* Known items */
    if (item->ident)
    {
        /* Process various fields */
        item->value = borg_object_value_known(item);
    }
    /* Aware items */
    else if (item->aware)
    {
        /* Aware items can assume template cost */
        item->value = k_info[item->kind].cost;
    }

	/* No known price on non-aware  item */
    if (!item->aware && !item->value)
    {
        /* Guess at weight and cost */
        switch (item->tval)
        {
			case TV_SKELETON:
				item->value = 0L;
				break;
            case TV_FOOD:
            {
                item->value = 5L;
                break;
            }
            case TV_POTION:
            {
                item->value = 20L;
                break;
            }
            case TV_SCROLL:
            {
                item->value = 20L;
                break;
            }
            case TV_STAFF:
            {
                item->value = 70L;
                break;
            }
            case TV_WAND:
            {
                item->value = 50L;
                break;
            }
            case TV_ROD:
            {
                item->value = 90L;
                break;
            }
            case TV_RING:
            {
                item->value = 45L;
                break;
            }
            case TV_AMULET:
            {
                item->value = 45L;
                break;
            }
        }
	}

    /* Item is cursed */
    if (item->ident)
	{
		item->cursed = cursed_p(real_item->flags);
	}

	/* Hack -- examine artifacts */
    if (item->name1)
    {
        /* XXX XXX Hack -- fix "weird" artifacts */
        if ((item->tval != a_info[item->name1].tval) ||
            (item->sval != a_info[item->name1].sval))
        {
			object_kind *o_ptr;

            /* Save the kind */
            o_ptr = lookup_kind(item->tval, item->sval);
			item->kind = o_ptr->kidx;

            /* Save the tval/sval */
            item->tval = k_info[item->kind].tval;
            item->sval = k_info[item->kind].sval;
        }

        /* Extract the weight */
        item->weight = a_info[item->name1].weight;

    }


    /* Hack -- examine ego-items */
    if (item->name2)
    {
        /* XXX Extract the weight */

    }

    /* Special "discount" */
/*     item->discount = real_item->discount */

    /* Cursed indicators */
    if (strstr(item->note, "cursed")) item->value = 0L;
    else if (strstr(item->note, "{broken")) item->value = 0L;
    else if (strstr(item->note, "{terrible")) item->value = 0L;
    else if (strstr(item->note, "{worthless")) item->value = 0L;
	else if (strstr(item->note, "{strange}")) item->value = 0L;


    /* Ignore certain feelings */
    /* "{average}" */
    /* "{blessed}" */
    /* "{magical}" */
    /* "{excellent}" */
    /* "{special}" */

    /* Ignore special inscriptions */
    /* "{empty}", "{tried}" */




    /* Hack -- repair rings of damage */
    if ((item->tval == TV_RING) && (item->sval == SV_RING_DAMAGE))
    {
        /* Bonus to dam, not pval */
        item->to_d = item->pval;
        item->pval = 0;
    }

    /* Hack -- repair rings of accuracy */
    if ((item->tval == TV_RING) && (item->sval == SV_RING_ACCURACY))
    {
        /* Bonus to hit, not pval */
        item->to_h = item->pval;
        item->pval = 0;
    }


    /* XXX XXX XXX Repair various "ego-items" */

	/* Repair the Planatir of Westerness. The borg thinks
	 * it is an ego item.since it has an ego name (Westerness).
	 */
	if (item->kind == borg_lookup_kind(TV_LIGHT, SV_PLANATIR))
	{
		/* remove the name2 and replace it with correct name1 */
		item->name1 = 7;
		item->name2 = 0;
		/* correct activation and pval */
		item->activation = EFF_CLAIRVOYANCE;
		item->pval = 2;
	}

#if 0
	/* Repair the Shield of the Haradrim. The borg thinks
	 * it is an ego item.since it has an ego name (Haradrim).
	 */
	if (item->kind == 129 && item->name2 == 107 && item->pval == 2)
	{
		/* remove the name2 and replace it with correct name1 */
		item->name1 = 134;
		item->name2 = 0;
		/* correct activation and pval */
		item->activation = EFF_BERSERKER;
		item->pval = 2;
	}
#endif

	/* Repair the Elvenkind items that are not Armour so that
	 * the borg can correctly handle the high resists
	 */
	if (item->name2 == 61 || item->name2 == 21) item->name2 = 9;

    /* Hack -- examine artifacts */
    if (item->name1)
    {
        /* XXX XXX Hack -- fix "weird" artifacts */
        if ((item->tval != a_info[item->name1].tval) ||
            (item->sval != a_info[item->name1].sval))
        {
            /* Save the kind */
            item->kind = borg_lookup_kind(item->tval, item->sval);

            /* Save the tval/sval */
            item->tval = k_info[item->kind].tval;
            item->sval = k_info[item->kind].sval;
        }

        /* Extract the weight */
        item->weight = a_info[item->name1].weight;

     }


    /* Known items */
    if (item->ident)
    {
        /* Process various fields */
        item->value = borg_object_value_known(item);
		item->aware = TRUE;
    }

    /* Aware items */
    else if (item->kind)
    {
        /* Aware items can assume template cost */
        item->value = k_info[item->kind].cost;
		item->aware = TRUE;
    }
	/* Non ID, assume some value */
	else
	{
		item->value = 20L;
		item->aware = FALSE;
	}


    /* Parse various "inscriptions" */
    if (item->note[0])
    {
        /* Special "discount" */
        if (streq(item->note, "{on sale}")) item->discount = 50;

        /* Standard "discounts" */
        else if (streq(item->note, "{25% off}")) item->discount = 25;
        else if (streq(item->note, "{50% off}")) item->discount = 50;
        else if (streq(item->note, "{75% off}")) item->discount = 75;
        else if (streq(item->note, "{90% off}")) item->discount = 90;

        /* Cursed indicators */
        else if (strstr(item->note, "cursed}"))
        {
            /* One Ring is not junk */
            if (item->activation != EFF_BIZARRE)
            {
                item->value = 0L;
            }
            item->cursed = TRUE;
        }

        else if (strstr(item->note, "broken")) item->value = 0L;
        else if (strstr(item->note, "terrible")) item->value = 0L;
        else if (strstr(item->note, "worthless")) item->value = 0L;

        /* Ignore certain feelings */
        /* "{average}" */
        /* "{blessed}" */
        /* "{magical}" */
        /* "{excellent}" */
        /* "{special}" */

        /* Ignore special inscriptions */
        /* "{empty}", "{tried}" */
    }


    /* Apply "discount" if any */
    if (item->discount) item->value -= item->value * item->discount / 100;

	/* Make sure any effect is known if not already known */
	if (!item->activation)
	{
		item->activation = k_info[real_item->kind->kidx].effect;
	}

	/* Modifiers assumed if pseudoID.  This will allow him to trade his cloak +0 in for a cloak {magical} */
    if (strstr(item->note, "magical") && !item->ident)
	{
		int slot = borg_wield_slot(item);

		/* Minimally boost the value so we don't crush it */
		item->value += 5;

		/* Weapons are assumed to have at least +1/+1 */
		if (slot == INVEN_WIELD || slot == INVEN_BOW)
		{
			item->to_h = 1;
    		item->to_d = 1;

			/* In Muchkin_mode, we want to make money so we want to wear stuff to ID it.
			 * Have him believe that the item is high bonus so that he will replace his
			 * current equipment.  There is a chance that the item is cursed and this will
			 * adversely impact his survivability.  He is able to buy ?Remove Curse.
			 */
			if (borg_munchkin_mode)
			{
				item->to_h = 8;
				item->to_d = 8;
			}

		}

		/* armors are assumed to have at least +1 */
    	if (slot > INVEN_BOW)
		{
			item->to_a = 1;

			if (borg_munchkin_mode)
			{
				item->to_a = 8;
			}
		}

	}

	/* Fake modifiers assumed if pseudoID.  This will allow him to trade his cloak +0 in for a cloak {magical}
	 * But if the cloak has been ID-though-use at Cloak [+4] {Special}, then he will be keeping the real +4
	 * instead of the +5 given below
	 */
    if ((strstr(item->note, "ego") ||
        strstr(item->note, "splendid") ||
        strstr(item->note, "excellent") ||
        strstr(item->note, "special"))  && !item->ident)

	{
		int slot = borg_wield_slot(item);

		/* Minimally boost the value so we don't crush it */
		item->value += 25;

		/* Weapons are assumed to have at least +1/+1 */
		if (slot == INVEN_WIELD || slot == INVEN_BOW)
		{
			item->to_h = 5;
    		item->to_d = 5;

			if (borg_munchkin_mode)
			{
				item->to_h = 9;
				item->to_d = 9;
			}
		}

		/* armors are assumed to have at least +1 */
    	if (slot > INVEN_BOW)
		{
			item->to_a = 5;

			if (borg_munchkin_mode)
			{
				item->to_a = 9;
			}
		}

	}

	/* Assume not fully Identified. */
    item->needs_I = TRUE;
    item->fully_identified = FALSE;

	/* Unless its easy_know */
	if (item->kind && easy_know(real_item))
	{
		item->needs_I = FALSE;
		item->fully_identified = TRUE;

	}

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
    if (i < INVEN_WIELD)
    {
        /* Choose the item */
        borg_keypress(I2A(i));
    }

    /* Choose from equipment */
    else
    {
        /* Go to equipment (if necessary) */
        if (borg_items[0].iqty) borg_keypress('/');

        /* Choose the item */
        borg_keypress(I2A(i - INVEN_WIELD));
    }

    /* Send the label */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the inscription */
    borg_keypress('\n');

}




/*
 * Find the slot of an item with the given tval/sval, if available.
 * Given multiple choices, choose the item with the largest "pval".
 * Given multiple choices, choose the smallest available pile.
 */
int borg_slot(int tval, int sval)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Skip un-aware items */
        if (!item->kind) continue;

        /* Require correct tval */
        if (item->tval != tval) continue;

        /* Require correct sval */
        if (item->sval != sval) continue;

        /* Prefer smallest pile */
        if ((n >= 0) && (item->iqty > borg_items[n].iqty)) continue;

		/* Prefer largest "pval" (even if smaller pile)*/
        if ((n >= 0) && (item->pval < borg_items[n].pval) &&
			(item->iqty > borg_items[n].iqty)) continue;

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
    i = borg_slot(TV_LIGHT, SV_LIGHT_TORCH);

    /* None available */
    if (i < 0) return (FALSE);

    /* must first wield before one can refuel */
    if (borg_items[INVEN_LIGHT].sval != SV_LIGHT_TORCH)
    {
        return (FALSE);
    }

    /* Dont bother with empty */
    if (borg_items[i].timeout == 0)
    {
        return (FALSE);
    }

    /* Cant refuel nothing */
    if (borg_items[INVEN_LIGHT].iqty == 0)
    {
        return (FALSE);
    }

    /* Log the message */
    borg_note(format("# Refueling with %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('F');
    borg_keypress(I2A(i));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

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

    /* None available check for lantern */
    if (i < 0)
    {
		i = borg_slot(TV_LIGHT, SV_LIGHT_LANTERN);

		/* It better have some oil left */
	    if (borg_items[i].timeout <= 0) i = -1;
	}

	/* Still none */
	if (i < 0) return (FALSE);

    /* Cant refuel a torch with oil */
    if (borg_items[INVEN_LIGHT].sval != SV_LIGHT_LANTERN)
    {
        return (FALSE);
    }

    /* Log the message */
    borg_note(format("# Refueling with %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('F');
    borg_keypress(I2A(i));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

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
    borg_note(format("# Eating %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(I2A(i));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (TRUE);
}

/*
 * Quaff a potion of cure critical wounds.  This is a special case
 *   for several reasons.
 *   1) it is usually the only healing potion we have on us
 *   2) we should try to conserve a couple for when we really need them
 *   3) if we are burning through them fast we should probably teleport out of
 *      the fight.
 *   4) When it is the only/best way out of danger, drink away
  */
bool borg_quaff_crit( bool no_check )
{
    static s16b when_last_quaff = 0;

    if (no_check)
    {
        if (borg_quaff_potion(SV_POTION_CURE_CRITICAL))
        {
            when_last_quaff = borg_t;
            return (TRUE);
        }
        return (FALSE);
    }

    /* Avoid drinking CCW twice in a row */
    if (when_last_quaff > (borg_t-4) &&
        when_last_quaff <= borg_t  &&
        (randint1(100) < 75))
        return FALSE;

    /* Save the last two for when we really need them */
    if (borg_skill[BI_ACCW] < 2)
        return FALSE;

    if (borg_quaff_potion(SV_POTION_CURE_CRITICAL))
    {
        when_last_quaff = borg_t;
        return (TRUE);
    }
    return (FALSE);
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
    borg_note(format("# Quaffing %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(I2A(i));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (TRUE);
}
/*
 * Hack -- attempt to quaff an unknown potion
 */
bool borg_quaff_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require correct tval */
        if (item->tval != TV_POTION) continue;

        /* Skip aware items */
        if (item->kind) continue;

        /* Save this item */
        n = i;
    }


    /* None available */
    if (n < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Quaffing unknown potion %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(I2A(n));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (TRUE);
}

/*
 * Hack -- attempt to read an unknown scroll
 */
bool borg_read_unknown(void)
{
    int i, n = -1;
    borg_grid *ag = &borg_grids[c_y][c_x];

    /* Scan the pack */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require correct tval */
        if (item->tval != TV_SCROLL) continue;

        /* Skip aware items */
        if (item->kind) continue;

        /* Save this item */
        n = i;
    }


    /* None available */
    if (n < 0) return (FALSE);

    /* Dark */
    if (no_light()) return (FALSE);

    /* Blind or Confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (FALSE);

    /* Log the message */
    borg_note(format("# Reading unknown scroll %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('r');
    borg_keypress(I2A(n));

	/* Incase it is ID scroll, ESCAPE out. */
    borg_keypress(ESCAPE);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to eat an unknown potion.  This is done in emergencies.
 */
bool borg_eat_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require correct tval */
        if (item->tval != TV_FOOD) continue;

        /* Skip aware items */
        if (item->kind) continue;

        /* Save this item */
        n = i;
    }


    /* None available */
    if (n < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Eating unknown mushroom %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(I2A(n));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (TRUE);
}

/*
 * Hack -- attempt to use an unknown staff.  This is done in emergencies.
 */
bool borg_use_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require correct tval */
        if (item->tval != TV_STAFF) continue;

        /* Skip aware items */
        if (item->kind) continue;

        /* Save this item */
        n = i;
    }


    /* None available */
    if (n < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Using unknown Staff %s.", borg_items[n].desc));

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    zap_slot = n;

	/* Perform the action */
    borg_keypress('u');
    borg_keypress(I2A(n));

	/* Incase it is ID staff, ESCAPE out. */
    borg_keypress(ESCAPE);

    /* Success */
    return (TRUE);
}


/*
 * Hack -- attempt to read the given scroll (by sval)
 */
bool borg_read_scroll(int sval)
{
    int i;
    borg_grid *ag = &borg_grids[c_y][c_x];

    /* Dark */
    if (no_light()) return (FALSE);

    /* Blind or Confused or Amnesia*/
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
    	borg_skill[BI_ISFORGET]) return (FALSE);

    /* Look for that scroll */
    i = borg_slot(TV_SCROLL, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("# Reading %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress('r');
    borg_keypress(I2A(i));

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (TRUE);
}

/* Return the relative chance for failure to activate an item.
 * The lower the number, the better the chance of success
 * 200 is 80% of success
 * 600 is 40% chance of success
 */
int borg_activate_failure(int tval, int sval)
{
	int lev;
	int skill;
	int fail;
	int i;

    /* Look for that item */
    i = borg_slot(tval, sval);

    /* None available */
    if (i < 0) return (100);

    /* No charges */
    if (!borg_items[i].pval) return (100);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Yep we got one */
    return (fail);
}

/*
 * Hack -- checks rod (by sval) and
 * make a fail check on it.
 */
bool borg_equips_rod(int sval)
{
    int i, skill, lev;
	int fail;

    /* Look for that staff */
    i = borg_slot(TV_ROD, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* No charges */
    if (!borg_items[i].pval) return (FALSE);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage (at least 1/2 chance of success. */
    if (fail > 500) return (FALSE);

    /* Yep we got one */
    return (TRUE);
}



/*
 * Hack -- attempt to zap the given (charged) rod (by sval)
 */
bool borg_zap_rod(int sval)
{
    int i, lev, fail;
	int skill;

    /* Look for that rod */
    i = borg_slot(TV_ROD, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* Hack -- Still charging */
    if (!borg_items[i].pval) return (FALSE);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage */
    if (sval != SV_ROD_RECALL)
    {
		if (fail > 500) return (FALSE);
	}

    /* Log the message */
    borg_note(format("# Zapping %s.", borg_items[i].desc));

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
    if (!borg_items[i].pval) return (FALSE);

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    zap_slot = i;

    /* Log the message */
    borg_note(format("# Aiming %s.", borg_items[i].desc));

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
    if (!borg_items[i].pval) return (FALSE);

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    zap_slot = i;

    /* Log the message */
    borg_note(format("# Using %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}

/*
 * Hack -- attempt to use the given (charged) staff (by sval) and
 * make a fail check on it.
 */
bool borg_use_staff_fail(int sval)
{
    int i, fail, lev;
	int skill;

    /* Look for that staff */
    i = borg_slot(TV_STAFF, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* No charges */
    if (!borg_items[i].pval) return (FALSE);

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    zap_slot = i;

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500)
    {
        if (sval != SV_STAFF_TELEPORTATION)
        {
            return (FALSE);
        }

        /* We need to give some "desparation attempt to teleport staff" */
        if (!borg_skill[BI_ISCONFUSED] && !borg_skill[BI_ISBLIND]) /* Dark? */
        {
            /* We really have no chance, return false, attempt the scroll */
            if (fail > 500) return (FALSE);
        }
        /* We might have a slight chance, or we cannot not read */
    }


    /* record the address to avoid certain bugs with inscriptions&amnesia */
    zap_slot = i;

    /* Log the message */
    borg_note(format("# Using %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(I2A(i));

    /* Success */
    return (TRUE);
}
/*
 * Hack -- checks staff (by sval) and
 * make a fail check on it.
 */
bool borg_equips_staff_fail(int sval)
{
    int i, fail, lev;
	int skill;

    /* Look for that staff */
    i = borg_slot(TV_STAFF, sval);

    /* None available */
    if (i < 0) return (FALSE);

    /* No charges */
    if (!borg_items[i].pval) return (FALSE);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

	/* If its a Destruction, we only use it in emergencies, attempt it */
	if (sval == SV_STAFF_DESTRUCTION)
	{
		return (TRUE);
	}

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500)
    {
        /* No real chance of success on other types of staffs */
        if (sval != SV_STAFF_TELEPORTATION)
        {
            return (FALSE);
        }

        /* We need to give some "desparation attempt to teleport staff" */
        if (sval == SV_STAFF_TELEPORTATION && !borg_skill[BI_ISCONFUSED])
        {
            /* We really have no chance, return false, attempt the scroll */
            if (fail < 650) return (FALSE);
        }

        /* We might have a slight chance (or its a Destruction), continue on */
    }

    /* Yep we got one */
    return (TRUE);
}



/*
 * Hack -- attempt to use the given artifact (by index)
 */
bool borg_activate_artifact(int activation, int location)
{
    int i;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];
		artifact_type *a_ptr;

		/* Skip non artifacts */
        /* if (!artifact_p(item)) continue; */

		/* Skip artifacts w/o activation */
		/* TR2_activate was removed */
		if (!(item->name1 && a_info[item->name1].effect) &&
		    !(k_info[item->kind].effect)) continue;

		/* get the item */
		a_ptr = &a_info[item->name1];

		/* Skip wrong activation */
		if (a_ptr->effect != activation) continue;

        /* Check charge */
        if (item->timeout) continue;

        /*
         * Random Artifact must be *ID* to know the activation power.
         * The borg will cheat with random artifacts to know if the
         * artifact number is activatable, but artifact names and
         * types will be scrambled.  So he must first *ID* the artifact
         * he must play with the artifact to learn its power, just as
         * he plays with magic to gain experience.  But I am not about
         * to undertake that coding.  He needs to *ID* it anyway to learn
         * of the resists that go with the artifact.
         * Lights dont need *id* just regular id.
         */
        if  ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && (item->activation != EFF_ILLUMINATION &&
              item->activation != EFF_MAPPING &&
              item->activation != EFF_CLAIRVOYANCE) &&
             (!item->fully_identified))
        {
            borg_note(format("# %s must be *ID*'d before activation.", item->desc));
            return (FALSE);
        }

        /* Log the message */
        borg_note(format("# Activating artifact %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(I2A(i - INVEN_WIELD));

        /* Success */
        return (TRUE);
    }

    /* Oops */
    return (FALSE);
}


/*
 * Hack -- check and see if borg is wielding an artifact
 */
bool borg_equips_artifact(int activation, int location)
{
    int i;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];
		artifact_type *a_ptr;

		/* Skip non artifacts */
        /* if (!artifact_p(item)) continue; */

		/* get the item */
		a_ptr = &a_info[item->name1];

		/* Skip artifacts w/o activation */
		/* TR2_activate was removed */
		if (!a_info[item->name1].effect &&
		    !k_info[item->kind].effect) continue;


		/* Skip wrong activation */
		if (a_ptr->effect != activation) continue;

        /* Check charge.  But not on certain ones  Wor, ID, phase, TELEPORT.*/
        /* this is to ensure that his borg_prep code is working ok */
        if ((activation != EFF_RECALL &&
             activation != EFF_IDENTIFY &&
             activation != EFF_TELE_PHASE &&
             activation != EFF_TELE_LONG) &&
           (item->timeout >= 1) ) continue;

        /*
         * Random Artifact must be *ID* to know the activation power.
         * The borg will cheat with random artifacts to know if the
         * artifact number is activatable, but artifact names and
         * types will be scrambled.  So he must first *ID* the artifact
         * he must play with the artifact to learn its power, just as
         * he plays with magic to gain experience.  But I am not about
         * to undertake that coding.  He needs to *ID* it anyway to learn
         * of the resists that go with the artifact.
         * Lights dont need *id* just regular id.
         */
        if  ((op_ptr->opt[OPT_birth_randarts] || op_ptr->opt[OPT_birth_randarts]) && (item->activation != EFF_ILLUMINATION &&
              item->activation != EFF_MAPPING &&
              item->activation != EFF_CLAIRVOYANCE) &&
             (!item->fully_identified))
        {
            borg_note(format("# %s must be *ID*'d before activation.", item->desc));
            return (FALSE);
        }

        /* Success */
        return (TRUE);
    }

    /* I do not have it or it is not charged */
    return (FALSE);
}

/*
 * Check and see if borg is wielding an artifact
 */
bool borg_equips_item(int tval, int sval)
{
    int i;
    int lev, fail;
	int skill;

    /* Check the equipment-- */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip incorrect artifacts */
        if (item->tval != tval) continue;
        if (item->sval != sval) continue;

        /* Check charge. */
        if (item->timeout) continue;

        /* Extract the item level for fail rate check*/
        lev = item->level;

		/* Base chance of success */
		skill = borg_skill[BI_DEV];

		/* Confusion hurts skill */
		if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

		/* High level objects are harder */
		fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

        /* Roll for usage.  Return Fail if greater than 50% fail.  Must beat 500 to succeed. */
        if (fail > 500) continue;

        /* Success */
        return (TRUE);

    }

    /* I guess I dont have it, or it is not ready, or too hard to activate. */
    return (FALSE);
}

/*
 * Attempt to use the given equipment item.
 */
bool borg_activate_item(int tval, int sval, bool target)
{
    int i;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip incorrect items */
        if (item->tval != tval) continue;
        if (item->sval != sval) continue;

        /* Check charge */
        if (item->timeout) return (FALSE);

        /* Log the message */
        borg_note(format("# Activating item %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(I2A(i - INVEN_WIELD));

		/* Some items require a target */
		if (target)
		{
    	    borg_keypress('5');
		}

        /* Success */
        return (TRUE);
    }

    /* Oops */
    return (FALSE);
}

/*
 * Hack -- check and see if borg is wielding a dragon armor and if
 * he will pass a fail check.
 */
bool borg_equips_dragon(int drag_sval)
{
    int lev, fail;
	int skill;
	int numerator;
	int denominator;

       /* Check the equipment */
       borg_item *item = &borg_items[INVEN_BODY];

        /* Skip incorrect armours */
        if (item->tval !=TV_DRAG_ARMOR) return (FALSE);
        if (item->sval != drag_sval) return (FALSE);

        /* Check charge */
        if (item->timeout) return (FALSE);

        /*  Make Sure Mail is IDed */
        if (!item->ident) return (FALSE);


       /* check on fail rate
        * The fail check is automatic for dragon armor.  It is an attack
        * item.  He should not sit around failing 5 or 6 times in a row.
        * he should attempt to activate it, and if he is likely to fail, then
        * eh should look at a different attack option.  We are assuming
        * that the fail rate is about 50%.  So He may still try to activate it
        * and fail.  But he will not even try if he has negative chance or
        * less than twice the USE_DEVICE variable
        */
       /* Extract the item level */
       lev = borg_items[INVEN_BODY].level;

		/* Base chance of success */
		skill = borg_skill[BI_DEV];

		/* Confusion hurts skill */
		if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

		/* High level objects are harder */
		numerator   = (skill - lev) - (141 - 1);
		denominator = (lev - skill) - (100 - 10);

		/* Make sure that we don't divide by zero */
		if (denominator == 0) denominator = numerator > 0 ? 1 : -1;

		fail = (100 * numerator) / denominator;

		/* Roll for usage, but if its a Teleport be generous. */
		if (fail > 500) return (FALSE);

        /* Success */
        return (TRUE);

}

/*
 *  Hack -- attempt to use the given dragon armour
 */
bool borg_activate_dragon(int drag_sval)
{
    /* Check the equipment */

      borg_item *item = &borg_items[INVEN_BODY];

        /* Skip incorrect mails */
        if (item->tval != TV_DRAG_ARMOR) return (FALSE);
        if (item->sval != drag_sval) return (FALSE);

		/* Check charge */
        if (item->timeout) return (FALSE);

        /*  Make Sure Mail is IDed */
        if (!item->ident) return (FALSE);

        /* Log the message */
        borg_note(format("# Activating dragon scale %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(I2A(INVEN_BODY - INVEN_WIELD));

        /* Success */
        return (TRUE);
}

/*
 * Hack -- check and see if borg is wielding a ring and if
 * he will pass a fail check.
 */
bool borg_equips_ring(int ring_sval)
{
    int lev, fail, i;
	int skill;

    for (i = INVEN_LEFT; i < INVEN_RIGHT; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip incorrect armours */
        if (item->tval !=TV_RING) continue;
        if (item->sval != ring_sval) continue;

        /* Check charge */
        if (item->timeout) continue;

        /*  Make Sure is IDed */
        if (!item->ident) continue;

       /* check on fail rate
        */

       /* Extract the item level */
       lev = borg_items[i].level;

		/* Base chance of success */
		skill = borg_skill[BI_DEV];

		/* Confusion hurts skill */
		if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

		/* High level objects are harder */
		fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

		/* Roll for usage, but if its a Teleport be generous. */
		if (fail > 500) continue;

        /* Success */
        return (TRUE);
	}

	return (FALSE);

}

/*
 *  Hack -- attempt to use the given ring
 */
bool borg_activate_ring(int ring_sval)
{

	int i;

    /* Check the equipment */
    for (i = INVEN_LEFT; i < INVEN_RIGHT; i++)
    {
        borg_item *item = &borg_items[i];

        /* Skip incorrect mails */
        if (item->tval != TV_RING) continue;
        if (item->sval != ring_sval) continue;

        /* Check charge */
        if (item->timeout) continue;

        /*  Make Sure item is IDed */
        if (!item->ident) continue;

        /* Log the message */
        borg_note(format("# Activating ring %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(I2A(i - INVEN_WIELD));

        /* Success */
        return (TRUE);
	}

	return (FALSE);
}

/*
 * Determine if borg can cast a given spell (when fully rested)
 */
bool borg_spell_legal(int book, int what)
{
    borg_magic *as = &borg_magics[book][what];

    /* The borg must be able to "cast" spells */
    if (p_ptr->class->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* The book must be possessed */
    if (amt_book[book] <= 0) return (FALSE);

    /* The spell must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The spell must be affordable (when rested) */
    if (as->power > borg_skill[BI_MAXSP]) return (FALSE);

    /* Success */
    return (TRUE);
}

/*
 * Determine if borg can cast a given spell (right now)
 */
bool borg_spell_okay(int book, int what)
{
    int reserve_mana = 0;

    borg_magic *as = &borg_magics[book][what];

    borg_grid *ag = &borg_grids[c_y][c_x];

    /* Dark */
    if (no_light()) return (FALSE);

    /* Define reserve_mana for each class */
    if (borg_class == CLASS_MAGE) reserve_mana = 6;
    if (borg_class == CLASS_RANGER) reserve_mana = 22;
    if (borg_class == CLASS_ROGUE) reserve_mana = 20;

    /* Low level spell casters should not worry about this */
    if (borg_skill[BI_CLEVEL] < 35) reserve_mana = 0;

    /* Require ability (when rested) */
    if (!borg_spell_legal(book, what)) return (FALSE);

    /* Hack -- blind/confused/amnesia */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (FALSE);


    /* The spell must be affordable (now) */
    if (as->power > borg_skill[BI_CURSP]) return (FALSE);

    /* Do not cut into reserve mana (for final teleport) */
    if (borg_skill[BI_CURSP] - as->power < reserve_mana)
    {
        /* Phase spells ok */
        if (book == 0 && what == 2) return (TRUE);

        /* Teleport spells ok */
        if (book == 1 && what == 5) return (TRUE);

        /* Satisfy Hunger OK */
        if (book == 2 && what == 0) return (TRUE);

		/* Magic Missile OK */
        if (book == 0 && what == 0 && borg_skill[BI_CDEPTH] <= 35) return (TRUE);

        /* others are rejected */
        return (FALSE);
    }

    /* Success */
    return (TRUE);
}

/*
 * fail rate on a spell
 */
int borg_spell_fail_rate(int book, int what)
{
    int     chance, minfail;
    borg_magic *as = &borg_magics[book][what];

    /* Access the spell  */
    chance = as->sfail;

    /* Reduce failure rate by "effective" level adjustment */
    chance -= 3 * (borg_skill[BI_CLEVEL] - as->level);

    /* Reduce failure rate by INT/WIS adjustment */
    chance -= (adj_mag_stat[my_stat_ind[A_INT]]);

	/* Fear makes the failrate higher */
	if (borg_skill[BI_ISAFRAID]) chance += 20;

    /* Extract the minimum failure rate */
    minfail = adj_mag_fail[my_stat_ind[A_INT]];

    /* Non mage characters never get too good */
    if (!player_has(PF_ZERO_FAIL))
    {
        if (minfail < 5) minfail = 5;
    }

    /* Minimum failure rate and max */
    if (chance < minfail) chance = minfail;
	if (chance > 50) chance = 50;

    /* Stunning makes spells harder */
    if (borg_skill[BI_ISHEAVYSTUN]) chance += 25;
    if (borg_skill[BI_ISSTUN]) chance += 15;

	/* Amnesia makes it harder */
	if (borg_skill[BI_ISFORGET]) chance *= 2;

    /* Always a 5 percent chance of working */
    if (chance > 95) chance = 95;

    /* Return the chance */
    return (chance);


}

/*
 * same as borg_spell_okay with a fail % check
 */
bool borg_spell_okay_fail(int book, int what, int allow_fail )
{
    if (borg_spell_fail_rate(book, what) > allow_fail)
        return FALSE;
    return borg_spell_okay( book, what );
}

/*
 * Same as borg_spell with a fail % check
 */
bool borg_spell_fail(int book, int what, int allow_fail)
{
    if (borg_spell_fail_rate(book, what) > allow_fail)
        return FALSE;
    return borg_spell( book, what );
}

/*
 * Same as borg_spell_legal with a fail % check
 */
bool borg_spell_legal_fail(int book, int what, int allow_fail)
{
    if (borg_spell_fail_rate(book, what) > allow_fail)
        return FALSE;
    return borg_spell_legal( book, what );
}

/*
 * Attempt to cast a spell
 */
bool borg_spell(int book, int what)
{
    int i;

    borg_magic *as = &borg_magics[book][what];

    /* Require ability (right now) */
    if (!borg_spell_okay(book, what)) return (FALSE);

    /* Look for the book */
    i = borg_book[book];

    /* Paranoia */
    if (i < 0) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Casting %s (%d,%d).", as->name, book, what));

    /* Cast a spell */
    borg_keypress('m');
    borg_keypress(I2A(i));
    borg_keypress(I2A(what));

    /* increment the spell counter */
    as->times ++;

    /* Success */
    return (TRUE);
}


/*
 * Determine if borg can pray a given prayer (when fully rested)
 */
bool borg_prayer_legal(int book, int what)
{
    borg_magic *as = &borg_magics[book][what];

    /* The borg must be able to "pray" prayers */
    if (p_ptr->class->spell_book != TV_PRAYER_BOOK) return (FALSE);

    /* Look for the book */
    if (amt_book[book] <= 0) return (FALSE);

    /* The prayer must be "known" */
    if (as->status < BORG_MAGIC_TEST) return (FALSE);

    /* The prayer must be affordable (when fully rested) */
    if (as->power > borg_skill[BI_MAXSP]) return (FALSE);

    /* Success */
    return (TRUE);
}

/*
 * Determine if borg can pray a given prayer (right now)
 */
bool borg_prayer_okay(int book, int what)
{
    int reserve_mana =0;

    borg_magic *as = &borg_magics[book][what];

    borg_grid *ag = &borg_grids[c_y][c_x];

    /* Dark */
    if (no_light()) return (FALSE);

    /* define reserve_mana */
    if (borg_class == CLASS_PRIEST) reserve_mana = 8;
    if (borg_class == CLASS_PALADIN) reserve_mana = 20;

    /* Low level spell casters should not worry about this */
    if (borg_skill[BI_CLEVEL] < 35) reserve_mana = 0;

    /* Require ability (when rested) */
    if (!borg_prayer_legal(book, what)) return (FALSE);

    /* Hack -- blind/confused/amnesia */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (FALSE);

    /* The prayer must be affordable (right now) */
    if (as->power > borg_skill[BI_CURSP]) return (FALSE);

    /* Do not cut into reserve mana (for final teleport) */
    if (borg_skill[BI_CURSP] - as->power < reserve_mana)
    {
        /* Phase spells ok */
        if (book == 1 && what == 1) return (TRUE);
        if (book == 4 && what == 0) return (TRUE);

        /* Teleport spells ok */
        if (book == 4 && what == 1) return (TRUE);

        /* Satisfy Hunger spells ok */
        if (book == 1 && what == 5) return (TRUE);

		/* Banishment is OK, */
        if (book == 8 && what == 2) return (TRUE);

        /* others are rejected */
        return (FALSE);
    }

    /* Success */
    return (TRUE);
}

int borg_prayer_fail_rate(int book, int what)
{
    int     chance, minfail;
    borg_magic *as = &borg_magics[book][what];

    /* Access the spell  */
    chance = as->sfail;

    /* Reduce failure rate by "effective" level adjustment */
    chance -= 3 * (borg_skill[BI_CLEVEL] - as->level);

    /* Reduce failure rate by INT/WIS adjustment */
    chance -= (adj_mag_stat[my_stat_ind[A_WIS]]);

    /* Extract the minimum failure rate */
    minfail = adj_mag_fail[my_stat_ind[A_WIS]];

    /* Non priest characters never get too good */
    if (!player_has(PF_ZERO_FAIL))
    {
        if (minfail < 5) minfail = 5;
    }

    /*  Hack -- Priest prayer penalty for "edged" weapons  -DGK */
    if (player_has(PF_BLESS_WEAPON))
    {
        borg_item       *item;

        item = &borg_items[INVEN_WIELD];

        /* Penalize non-blessed edged weapons */
        if ((item->tval == TV_SWORD || item->tval == TV_POLEARM) &&
            !of_has(item->flags, OF_BLESSED))
        {
            chance += 25;
        }
    }

	/* Fear makes it more difficult */
	if (borg_skill[BI_ISAFRAID]) chance += 20;

    /* Minimum failure rate */
    if (chance < minfail) chance = minfail;
	if (chance > 50) chance = 50;

    /* Stunning makes spells harder */
    if (borg_skill[BI_ISHEAVYSTUN]) chance += 25;
    if (borg_skill[BI_ISSTUN]) chance += 15;

    /* Always a 5 percent chance of working */
    if (chance > 95) chance = 95;

    /* Return the chance */
    return (chance);


}


/*
 * same as borg_prayer_okay with a fail % check
 */
bool borg_prayer_okay_fail(int book, int what, int allow_fail )
{
    if (borg_prayer_fail_rate(book, what) > allow_fail)
        return FALSE;
    return borg_prayer_okay( book, what );
}

/*
 * Same as borg_prayer with a fail % check
 */
bool borg_prayer_fail(int book, int what, int allow_fail)
{
    if (borg_prayer_fail_rate(book, what) > allow_fail)
        return FALSE;
    return borg_prayer( book, what );
}

/*
 * Same as borg_prayer_legal with a fail % check
 */
bool borg_prayer_legal_fail(int book, int what, int allow_fail)
{
    if (borg_prayer_fail_rate(book, what) > allow_fail)
        return FALSE;
    return borg_prayer_legal( book, what );
}

/*
 * Attempt to pray a prayer
 */
bool borg_prayer(int book, int what)
{
    int i;

    borg_magic *as = &borg_magics[book][what];

    /* Require ability (right now) */
    if (!borg_prayer_okay(book, what)) return (FALSE);

    /* Look for the book */
    i = borg_book[book];

    /* Paranoia */
    if (i < 0) return (FALSE);

    /* Debugging Info */
    borg_note(format("# Praying %s (%d,%d).", as->name, book, what));

    /* Pray a prayer */
    borg_keypress('p');
    borg_keypress(I2A(i));
    borg_keypress(I2A(what));

    /* Because we have no launch message to indicate failure */
    if (book ==3 && what ==4)
    {
        borg_casted_glyph = TRUE;
    }
    else
    {
        borg_casted_glyph = FALSE;
    }

    /* increment the spell counter */
    as->times ++;

    /* Success */
    return (TRUE);
}


/*
 * Inscribe food and Slime Molds
 */
extern bool borg_inscribe_food(void)
{
    int ii;
    char name[80];

    for (ii=0; ii < INVEN_TOTAL; ii++)
    {
        borg_item *item = &borg_items[ii];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require correct tval */
        if (item->tval != TV_FOOD) continue;

        /* skip things already inscribed */
        if (!(streq(item->note, "")) &&
            !(streq(item->note, "{ }"))) continue;

        /* inscribe foods and molds */
        if (item->sval == SV_FOOD_SLIME_MOLD || item->sval == SV_FOOD_RATION)
        {

            if (item->sval == SV_FOOD_RATION)
            {
                /* get a name */
                strcpy(name, food_syllable1[randint0(sizeof(food_syllable1) / sizeof(char*))]);
                strcat(name, food_syllable2[randint0(sizeof(food_syllable2) / sizeof(char*))]);

                borg_send_inscribe(ii, name);
                return (TRUE);
            }

            if (item->sval == SV_FOOD_SLIME_MOLD)
            {
                /* get a name */
                strcpy(name, mold_syllable1[randint0(sizeof(mold_syllable1) / sizeof(char*))]);
                strcat(name, mold_syllable2[randint0(sizeof(mold_syllable2) / sizeof(char*))]);
                strcat(name, mold_syllable3[randint0(sizeof(mold_syllable3) / sizeof(char*))]);

                borg_send_inscribe(ii, name);
                return (TRUE);
            }

        }
    }

    /* all done */
    return (FALSE);
}
/*
 * Send a command to de-inscribe item number "i" .
 */
void borg_send_deinscribe(int i)
{

    /* Ok to inscribe Slime Molds */
    if (borg_items[i].tval == TV_FOOD &&
        borg_items[i].sval == SV_FOOD_SLIME_MOLD) return;

    /* Label it */
    borg_keypress('}');

    /* Choose from inventory */
    if (i < INVEN_WIELD)
    {
        /* Choose the item */
        borg_keypress(I2A(i));
    }

    /* Choose from equipment */
    else
    {
        /* Go to equipment (if necessary) */
        if (borg_items[0].iqty) borg_keypress('/');

        /* Choose the item */
        borg_keypress(I2A(i - INVEN_WIELD));
    }

	/* May ask for a confirmation */
	borg_keypress('y');
	borg_keypress('y');
}

/*
 * Return the owner struct for the given store.
 */
static struct owner *store_owner(int st) {
	return stores[st].owner;
}

/* (This is copied from store.c
 * Determine the price of an object (qty one) in a store.
 *
 *  store_buying == TRUE  means the shop is buying, player selling
 *               == FALSE means the shop is selling, player buying
 *
 * This function takes into account the player's charisma, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * object, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 */
s32b borg_price_item(const object_type *o_ptr, bool store_buying, int qty, int this_store)
{
	int adjust;
	s32b price;
	owner_type *ot_ptr;

	if (this_store == STORE_NONE) return 0L;

	ot_ptr = store_owner(this_store);

	/* Get the value of the stack of wands, or a single item */
	if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
		price = object_value(o_ptr, qty, FALSE);
	else
		price = object_value(o_ptr, 1, FALSE);

	/* Worthless items */
	if (price <= 0) return (0L);


	/* Add in the charisma factor */
	if (this_store == STORE_B_MARKET)
		adjust = 150;
	else
		adjust = adj_chr_gold[p_ptr->state.stat_ind[A_CHR]];


	/* Shop is buying */
	if (store_buying)
	{
		/* Set the factor */
		adjust = 100 + (100 - adjust);
		if (adjust > 100) adjust = 100;

		/* Shops now pay 2/3 of true value */
		price = price * 2 / 3;

		/* Mega-Hack -- Black market sucks */
		if (this_store == STORE_B_MARKET) price = price / 2;

		/* Check for no_selling option */
		if (OPT(birth_no_selling)) return (0L);
	}

	/* Shop is selling */
	else
	{
		/* Fix the factor */
		if (adjust < 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (this_store == STORE_B_MARKET) price = price * 2;
	}

	/* Compute the final price (with rounding) */
	price = (price * adjust + 50L) / 100L;

	/* Now convert price to total price for non-wands */
	if (!(o_ptr->tval == TV_WAND) && !(o_ptr->tval == TV_STAFF))
		price *= qty;

	/* Now limit the price to the purse limit */
	if (store_buying && (price > ot_ptr->max_cost * qty))
		price = ot_ptr->max_cost * qty;

	/* Note -- Never become "free" */
	if (price <= 0L) return (qty);

	/* Return the price */
	return (price);
}

/*
 * Cheat the "equip" screen
 */
void borg_cheat_equip(void)
{
    int i;

    char buf[256];

    /* Extract the equipment */
    for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
    {
		/* skip non items */
		if (!p_ptr->inventory[i].kind->kidx)
		{
			/* Be sure to wipe it from the borg equip */
			WIPE(&borg_items[i], borg_item);
		}

        /* Default to "nothing" */
        buf[0] = '\0';

        /* Describe a real item */
        if (p_ptr->inventory[i].kind->kidx)
        {
            /* Describe it */
            object_desc(buf, sizeof(buf), &p_ptr->inventory[i], ODESC_FULL);
        }

        /* Analyze the item (no price) */
        borg_item_analyze(&borg_items[i], &p_ptr->inventory[i], buf);

        /* get the fully id stuff */
        if (p_ptr->inventory[i].ident & (IDENT_KNOWN | IDENT_EFFECT))
        {
            borg_items[i].fully_identified = TRUE;
            borg_items[i].needs_I = FALSE;
    	}

		/* Uninscribe items with ! inscriptions */
		if (strstr(borg_items[i].desc, "!")) borg_send_deinscribe(i);
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
    borg_cur_wgt = p_ptr->total_weight;

    /* Extract the inventory */
    for (i = 0; i < INVEN_MAX_PACK; i++)
    {
		/* Skip non-items */
		if (!p_ptr->inventory[i].kind->kidx)
		{
			/* Wipe from borg lists */
			WIPE(&borg_items[i], borg_item);
		}

        /* Default to "nothing" */
        buf[0] = '\0';

        /* Describe a real item */
        if (p_ptr->inventory[i].kind->kidx)
        {
            /* Describe it */
            object_desc(buf, sizeof(buf), &p_ptr->inventory[i], ODESC_FULL);
        }

		/* Skip Empty slots */
		if (streq(buf,"(nothing)")) continue;


    	/* Analyze the item (no price) */
    	borg_item_analyze(&borg_items[i], &p_ptr->inventory[i], buf);

    	/* get the fully id stuff */
    	if (p_ptr->inventory[i].ident & (IDENT_KNOWN | IDENT_EFFECT))
    	{
    	    borg_items[i].fully_identified = TRUE;
    	    borg_items[i].needs_I = FALSE;
    	}

    	/* Note changed inventory */
    	borg_do_crush_junk = TRUE;
    	borg_do_crush_hole = TRUE;
    	borg_do_crush_slow = TRUE;

		/* Uninscribe items with ! inscriptions */
		if (strstr(borg_items[i].desc, "!"))
		{borg_send_deinscribe(i);}
	}
}

/*
 * Cheat the "Store" screen
 */
void borg_cheat_store(void)
{
    int i;
	int slot;
    char buf[256];
	int shop_num;

	object_type *j_ptr;

	struct store *st_ptr;

	/* scan each visited store */
	for (shop_num = 0; shop_num < MAX_STORES;  shop_num ++)
	{

#if 0
		/* Skip a shop that has not been visited */
		if (!borg_shops[shop_num].when && shop_num != 7) continue;
#endif
		/* Access the current shop */
		st_ptr = &stores[shop_num];

		/* Clear the Inventory from memory */
		for (i = 0; i < 24; i++)
		{
			/* Wipe the ware */
			WIPE(&borg_shops[shop_num].ware[i], borg_item);
		}

		/* Check each existing object in this store */
		for (slot = 0; slot < 24; slot++)
		{
			/* Get the existing object */
			j_ptr = &st_ptr->stock[slot];

			/* Default to "nothing" */
			buf[0] = '\0';

			/* Describe it */
			object_desc(buf, sizeof(buf), j_ptr, ODESC_FULL);

			/* Skip Empty slots */
			if (streq(buf,"(nothing)")) continue;

			/* Copy the Description to the borg's memory */
			strcpy(borg_shops[shop_num].ware[slot].desc, buf);

			/* Analyze the item */
			borg_item_analyze(&borg_shops[shop_num].ware[slot], &stores[shop_num].stock[slot], buf);

			/*need to be able to analize the home inventory to see if it was */
			/* *fully ID*d. */
			/* This is a BIG CHEAT!  It will be less of a cheat if code is put*/
			/* in place to allow 'I' in stores. */
			if (stores[shop_num].stock[slot].ident & IDENT_KNOWN)
			{
				/* XXX XXX XXX for now, always cheat to get info on items at */
				/*   home. */
				borg_object_star_id_aux( &borg_shops[shop_num].ware[slot],
										 &stores[shop_num].stock[slot]);
				borg_shops[shop_num].ware[slot].fully_identified = TRUE;
			}

			/* hack -- see of store is selling food.  Needed for Money Scumming */
			if (shop_num == 0 &&
				borg_shops[shop_num].ware[slot].tval == TV_FOOD &&
				borg_shops[shop_num].ware[slot].sval == SV_FOOD_RATION)
			{
				borg_food_onsale = borg_shops[shop_num].ware[slot].iqty;
			}

			/* hack -- see of store is selling our fuel. */
			if (shop_num == 0 &&
				borg_shops[shop_num].ware[slot].tval == TV_FLASK &&
				borg_items[INVEN_LIGHT].sval == SV_LIGHT_LANTERN)
			{
				borg_fuel_onsale = borg_shops[shop_num].ware[slot].iqty;
			}

			/* hack -- see of store is selling our fuel. */
			if (shop_num == 0 &&
				borg_shops[shop_num].ware[slot].tval == TV_LIGHT &&
				borg_items[INVEN_LIGHT].sval == SV_LIGHT_TORCH)
			{
				borg_fuel_onsale = borg_shops[shop_num].ware[slot].iqty;
			}

			/* Hack -- Save the declared cost */
			borg_shops[shop_num].ware[slot].cost = borg_price_item(j_ptr, FALSE,1, shop_num);
		}
	}
}

/*
 * Hack -- Cheat the "spell" info
 *
 * Hack -- note the use of the "cheat" field for efficiency
 */
void borg_cheat_spell(int book)
{
    int what;


    /* Can we use spells/prayers? */
    if (!p_ptr->class->spell_book) return;

	/* Process the Books */
/*	for (book = 0; book < 9; book++) */
/*	{ */
		/* Process the spells */
	    for (what = 0; what < 9; what++)
		{
			/* Access the spell */
			borg_magic *as = &borg_magics[book][what];

			/* Skip illegible spells */
			if (as->status == BORG_MAGIC_ICKY) continue;

			/* Note "forgotten" spells */
			if (p_ptr->spell_flags[as->cheat] & PY_SPELL_FORGOTTEN)
			{
	            /* Forgotten */
				as->status = BORG_MAGIC_LOST;
			}

	        /* Note "difficult" spells */
		    else if (borg_skill[BI_CLEVEL] < as->level)
			{
	            /* Unknown */
		        as->status = BORG_MAGIC_HIGH;
			}

			/* Note "Unknown" spells */
	        else if (!(p_ptr->spell_flags[as->cheat] & PY_SPELL_LEARNED))
		    {
			    /* UnKnown */
				as->status = BORG_MAGIC_OKAY;
	        }

		    /* Note "untried" spells */
			else if (!(p_ptr->spell_flags[as->cheat] & PY_SPELL_WORKED))
	        {
		        /* Untried */
			    as->status = BORG_MAGIC_TEST;
			}

	        /* Note "known" spells */
		    else
			{
				/* Known */
	            as->status = BORG_MAGIC_KNOW;
		    }
	    }
/*	} */
}


/*
 * Prepare a book
 */
static void prepare_book_info(void)
{
    int what;

    int book;

    int index;

    /* Reset each spell entry */
    for (book = 0; book < 9; book++)
    {
        for (what = 0; what < 9; what++)
        {
            borg_magic *as = &borg_magics[book][what];

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
    }

    /* Can we use spells/prayers? */
    if (!p_ptr->class->spell_book) return;

    /* define the spell type */
    if (p_ptr->class->spell_book == TV_MAGIC_BOOK)
    {
        /* Mage book */
        index = 0;
    }
    else
    {
        /* Priest */
        index = 1;
    }

    /* Process each existing spell */
    for (book = 0; book < 9; book++)
    {
        for (what = 0; what < 9; what++)
        {
            borg_magic *as = &borg_magics[book][what];

            const magic_type *spell_ptr;

            /* access the game index */
            spell_ptr = &p_ptr->class->spells.info[borg_magic_index[index][book][what]];

            /* Save the spell index */
            as->cheat = borg_magic_index[index][book][what];

            /* Hack -- assume excessive level */
            as->status = BORG_MAGIC_HIGH;

            /* Access the correct "method" */
            as->method = borg_magic_method[index][book][what];

            /* Access the correct "rating" */
            as->rating = borg_magic_rating[index][book][what];

            /* Save the spell name */
            as->name = borg_magic_name[index][book][what];

            /* Skip blank ones */
            if (as->cheat == 99) continue;

            /* Save the spell level */
            as->level = spell_ptr->slevel;
            /* Save the spell mana */
            as->power = spell_ptr->smana;
            /* Save the spell fail ratename */
            as->sfail = spell_ptr->sfail;
        }

    }

}



/*
 * Hack -- prepare some stuff based on the player race and class
 */
void prepare_race_class_info(void)
{
    /* Initialize the various spell arrays by book */
    prepare_book_info();
}


void borg_clear_3(void)
{
    FREE(borg_items);
    FREE(borg_shops);
    FREE(safe_items);
    FREE(safe_home);
    FREE(safe_shops);
    FREE(borg_plural_text);
    FREE(borg_sv_plural_text);
    FREE(borg_plural_what);
    FREE(borg_single_text);
    FREE(borg_single_what);
    FREE(borg_artego_text);
    FREE(borg_sv_art_text);
    FREE(borg_artego_what);
}

/*
 * Initialize this file
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
void borg_init_3(void)
{
    int i, k, n;

    int size;

    s16b what[514];
    cptr text[514];

    char buf[256];


    /*** Item/Ware arrays ***/

    /* Make the inventory array */
    C_MAKE(borg_items, QUIVER_END, borg_item);

    /* Make the stores in the town */
    C_MAKE(borg_shops, 9, borg_shop);


    /*** Item/Ware arrays (simulation) ***/

    /* Make the "safe" inventory array */
    C_MAKE(safe_items, QUIVER_END, borg_item);
    C_MAKE(safe_home,  STORE_INVEN_MAX, borg_item);

    /* Make the "safe" stores in the town */
    C_MAKE(safe_shops, 8, borg_shop);

    /*** Plural Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 1; k < z_info->k_max; k++)
    {
        object_type hack;

        /* Get the kind */
        object_kind *k_ptr = &k_info[k];

        /* Skip "empty" items */
        if (!k_ptr->name) continue;

        /* Skip "gold" objects */
        if (k_ptr->tval == TV_GOLD) continue;

        /* Skip "artifacts" */
        if (of_has(k_ptr->flags, OF_INSTA_ART)) continue;

        /* Hack -- make an item */
        object_prep(&hack, &k_info[k], 10 , MINIMISE);

        /* Describe a "plural" object */
        hack.number = 2;
        object_desc(buf, sizeof(buf), &hack, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;

        size++;

    }

    /* Set the sort hooks */
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort */
    borg_sort(text, what, size);

    C_MAKE(borg_sv_plural_text, z_info->k_max, cptr);
    for (i = 0; i < size; i++) borg_sv_plural_text[what[i]] = text[i];


    /* Save the size */
    borg_plural_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(borg_plural_text, borg_plural_size, cptr);
    C_MAKE(borg_plural_what, borg_plural_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) borg_plural_text[i] = text[i];
    for (i = 0; i < size; i++) borg_plural_what[i] = what[i];


    /*** Singular Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 1; k < z_info->k_max; k++)
    {
        object_type hack;

        /* Get the kind */
        object_kind *k_ptr = &k_info[k];

        /* Skip "empty" items */
        if (!k_ptr->name) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval == TV_GOLD) continue;

        /* Skip "artifacts" */
        if (of_has(k_ptr->flags, OF_INSTA_ART)) continue;

        /* Hack -- make an item */
        object_prep(&hack, &k_info[k], 0, MINIMISE);

        /* Describe a "singular" object */
        hack.number = 1;
        object_desc(buf, sizeof(buf), &hack, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Analyze the "INSTA_ART" items */
    for (i = 1; i < z_info->a_max; i++)
    {
        object_type hack;

        artifact_type *a_ptr = &a_info[i];

        cptr name = (a_ptr->name);

        /* Skip "empty" items */
        if (!a_ptr->name) continue;

		/* Hack-- to handle bug in 301 code.
		 * In the object.txt file, the new artifacts (jewel,
		 * elfstones) have the INSTA_ART flag, but the flag is
		 * not listed in the artifact.txt file.  Because of this,
		 * these 2 items will be skipped over by the borg.
		 * Remove this hack and restore the bypass below when
		 * the bug in the artifact.txt file is fixed.
		 */
        if (!(of_has(a_ptr->flags, OF_INSTA_ART)) &&
            i != 14 && /* elfstone */
            i != 15)  /* Jewel */
        	continue;
#if 0  /* Remove this bypass when bug is fixed in artifact.txt */
        /* Skip non INSTA_ART things */
        if (!(a_ptr->flags[2] & TR2_INSTA_ART)) continue;
#endif
        /* Extract the k"ind" */
        k = borg_lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Hack -- make an item */
        object_prep(&hack, &k_info[k], 10, MINIMISE);

        /* Save the index */
        /* hack.name1 = i; */

        /* Describe a "singular" object */
        hack.number = 1;
        object_desc(buf, sizeof(buf), &hack, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL);

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
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort */
    borg_sort(text, what, size);


    /* Save the size */
    borg_single_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(borg_single_text, borg_single_size, cptr);
    C_MAKE(borg_single_what, borg_single_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) borg_single_text[i] = text[i];
    for (i = 0; i < size; i++) borg_single_what[i] = what[i];


    /*** Artifact and Ego-Item Parsers ***/

    /* No entries yet */
    size = 0;

    /* Collect the "artifact names" */
    for (k = 1; k < z_info->a_max; k++)
    {
        artifact_type *a_ptr = &a_info[k];

        /* Skip non-items */
        if (!a_ptr->name) continue;

        /* Extract a string */
        sprintf(buf, " %s", (a_ptr->name));

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    C_MAKE(borg_sv_art_text, z_info->a_max, cptr);
    for (i = 0; i < size; i++) borg_sv_art_text[what[i]] = text[i];

    /* Collect the "ego-item names" */
    for (k = 1; k < z_info->e_max; k++)
    {
        ego_item_type *e_ptr = &e_info[k];

        /* Skip non-items */
        if (!e_ptr->name) continue;

        /* Extract a string */
        sprintf(buf, " %s", (e_ptr->name));

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k + 256;
        size++;
    }
    /* Set the sort hooks */
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;

    /* Sort */
    borg_sort(text, what, size);

    /* Save the size */
    borg_artego_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(borg_artego_text, borg_artego_size, cptr);
    C_MAKE(borg_artego_what, borg_artego_size, s16b);

    /* Save the entries */
    for (i = 0; i < size; i++) borg_artego_text[i] = text[i];
    for (i = 0; i < size; i++) borg_artego_what[i] = what[i];
}

cptr borg_prt_item(int item)
{
            if (item < z_info->k_max)
            {
                return borg_sv_plural_text[item];
            }
            if (item < z_info->k_max + z_info->k_max)
                return borg_sv_plural_text[item - z_info->k_max];
            if (item < z_info->k_max + z_info->k_max + z_info->a_max)
                return borg_sv_art_text[item - z_info->k_max - z_info->k_max];
            return (prefix_pref[item -
                                z_info->k_max -
                                z_info->k_max -
                                z_info->a_max]);

}



#ifdef MACINTOSH
static int HACK = 0;
#endif
#endif /* ALLOW_BORG */