/* File: borg3.c */

/* Purpose: Object and Spell routines for the Borg -BEN- */

#include "../angband.h"
#include "../cave.h"
#include "../cmd-core.h"
#include "../obj-curse.h"
#include "../obj-desc.h"
#include "../obj-knowledge.h"
#include "../obj-make.h"
#include "../obj-power.h"
#include "../obj-slays.h"
#include "../player-spell.h"

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

borg_item* borg_items;      /* Current "inventory" */

borg_shop* borg_shops;      /* Current "shops" */



/*
 * Safety arrays for simulating possible worlds
 */

borg_item* safe_items;      /* Safety "inventory" */
borg_item* safe_home;       /* Safety "home stuff" */

borg_shop* safe_shops;      /* Safety "shops" */


/*
 * Spell info
 */

borg_magic* borg_magics = NULL;   /* Spell info, individualized for class by spell number */


/* Food Names */
static const char* food_syllable1[] =
{
    "BBQ ", "Boiled ", "Fresh ", "Frozen ", "Burned ", "Rotten ", "Raw ", "Toasted ", "Broiled ", "Baked ", "Fried ", "Buttered ", "Steamed ", "Gramma's ",
};

/* Food Names */
static const char* food_syllable2[] =
{
    "Pizza", "Eggs", "Spam", "Oatmeal", "Chicken", "Bacon", "Peanutbutter", "Roast Beef", "Cheese", "Toast", "Hamburger", "Carrots", "Corn", "Potato", "Pork Chops", "Chinese Takeout", "Cookies",
};

/* Slime Molds */
static const char* mold_syllable1[] =
{
    "Ab", "Ac", "Ad", "Af", "Agr", "Ast", "As", "Al", "Adw", "Adr", "Ar", "B", "Br", "C", "Cr", "Ch", "Cad", "D", "Dr", "Dw", "Ed", "Eth", "Et", "Er", "El", "Eow", "F", "Fr", "G", "Gr", "Gw", "Gal", "Gl", "H", "Ha", "Ib", "Jer", "K", "Ka", "Ked", "L", "Loth"
, "Lar", "Leg", "M", "Mir", "N", "Nyd", "Ol", "Oc", "On", "P", "Pr", "R", "Rh", "S", "Sev", "T", "Tr", "Th", "V", "Y", "Z", "W", "Wic",
};

static const char* mold_syllable2[] =
{
    "a", "adrie", "ara", "e", "ebri", "ele", "ere", "i", "io", "ithra", "ilma", "il-Ga", "ili", "o", "orfi", "u", "y",
};

static const char* mold_syllable3[] =
{
    "bur", "fur", "gan", "gnus", "gnar", "li", "lin", "lir", "mli", "nar", "nus", "rin", "ran", "sin", "sil", "sur",
};


static borg_spell_rating* borg_spell_ratings;
// !FIX !TODO !AJG for now put this in the code.  It should probably end up in borg.txt or a new borg.cfg
// I also gave low ratings to spells that are new since the borg doesn't know when to use them yet.
static borg_spell_rating borg_spell_ratings_MAGE[] =
{
    { "Magic Missile", 95, MAGIC_MISSILE },
    { "Light Room", 65, LIGHT_ROOM },
    { "Find Traps, Doors & Stairs", 85, FIND_TRAPS_DOORS_STAIRS },
    { "Phase Door", 95, PHASE_DOOR },
    { "Electric Arc", 85, ELECTRIC_ARC },
    { "Detect Monsters", 85, DETECT_MONSTERS },
    { "Fire Ball", 75, FIRE_BALL },
    { "Recharging", 65, RECHARGING },
    { "Identify Rune", 95, IDENTIFY_RUNE },
    { "Treasure Detection", 5, TREASURE_DETECTION }, /* borg never uses this */
    { "Frost Bolt", 75, FROST_BOLT },
    { "Reveal Monsters", 85, REVEAL_MONSTERS },
    { "Acid Spray", 75, ACID_SPRAY },
    { "Disable Traps, Destroy Doors", 95, DISABLE_TRAPS_DESTROY_DOORS },
    { "Teleport Self", 95, TELEPORT_SELF },
    { "Teleport Other", 75, TELEPORT_OTHER },
    { "Resistance", 90, RESISTANCE },
    { "Tap Magical Energy", 5, TAP_MAGICAL_ENERGY }, /* need to figure out when to cast this one */
    { "Mana Channel", 95, MANA_CHANNEL },
    { "Door Creation", 65, DOOR_CREATION },
    { "Mana Bolt", 95, MANA_BOLT },
    { "Teleport Level", 65, TELEPORT_LEVEL },
    { "Detection", 95, DETECTION },
    { "Dimension Door", 95, DIMENSION_DOOR },
    { "Thrust Away", 55, THRUST_AWAY },
    { "Shock Wave", 85, SHOCK_WAVE },
    { "Explosion", 85, EXPLOSION },
    { "Banishment", 75, BANISHMENT },
    { "Mass Banishment", 65, MASS_BANISHMENT },
    { "Mana Storm", 75, MANA_STORM }
};
static borg_spell_rating borg_spell_ratings_DRUID[] =
{
    { "Detect Life", 95,  DETECT_LIFE },
    { "Fox Form", 5, FOX_FORM }, // !FIX !TODO !AJG need to know when to cast any of the shapechages
    { "Remove Hunger", 85, REMOVE_HUNGER },
    { "Stinking Cloud", 95, STINKING_CLOUD },
    { "Confuse Monster", 55, CONFUSE_MONSTER },
    { "Slow Monster", 65, SLOW_MONSTER },
    { "Cure Poison", 55, CURE_POISON },
    { "Resist Poison", 60, RESIST_POISON },
    { "Turn Stone to Mud", 80, TURN_STONE_TO_MUD },
    { "Sense Surroundings", 80, SENSE_SURROUNDINGS },
    { "Lightning Strike", 85, LIGHTNING_STRIKE },
    { "Earth Rising", 70, EARTH_RISING },
    { "Trance", 55, TRANCE },
    { "Mass Sleep", 80, MASS_SLEEP },
    { "Become Pukel-man", 5, BECOME_PUKEL_MAN }, // !FIX !TODO !AJG shapechage
    { "Eagle's Flight", 5, EAGLES_FLIGHT }, // !FIX !TODO !AJG shapechage
    { "Bear Form", 5, BEAR_FORM }, // !FIX !TODO !AJG shapechage
    { "Tremor", 80, TREMOR },
    { "Haste Self", 90, HASTE_SELF },
    { "Revitalize", 95, REVITALIZE },
    { "Rapid Regeneration", 55, RAPID_REGENERATION },
    { "Herbal Curing", 90, HERBAL_CURING },
    { "Meteor Swarm", 90, METEOR_SWARM },
    { "Rift", 90, RIFT },
    { "Ice Storm", 85, ICE_STORM },
    { "Volcanic Eruption", 60, VOLCANIC_ERUPTION },
    { "River of Lightning", 90, RIVER_OF_LIGHTNING }
};

static borg_spell_rating borg_spell_ratings_PRIEST[] =
{
    { "Call Light", 65, CALL_LIGHT },
    { "Detect Evil", 85, DETECT_EVIL },
    { "Minor Healing", 65, MINOR_HEALING },
    { "Bless", 85, BLESS },
    { "Sense Invisible", 75, SENSE_INVISIBLE },
    { "Heroism", 75, HEROISM },
    { "Orb of Draining", 95, ORB_OF_DRAINING },
    { "Spear of Light", 75, SPEAR_OF_LIGHT },
    { "Dispel Undead", 65, DISPEL_UNDEAD },
    { "Dispel Evil", 65, DISPEL_EVIL },
    { "Protection from Evil", 85, PROTECTION_FROM_EVIL },
    { "Remove Curse", 85, REMOVE_CURSE },
    { "Portal", 85, PORTAL },
    { "Remembrance", 75, REMEMBRANCE },
    { "Word of Recall", 95, WORD_OF_RECALL },
    { "Healing", 95, HEALING },
    { "Restoration", 75, RESTORATION },
    { "Clairvoyance", 85, CLAIRVOYANCE },
    { "Enchant Weapon", 75, ENCHANT_WEAPON },
    { "Enchant Armour", 75, ENCHANT_ARMOUR },
    { "Smite Evil", 75, SMITE_EVIL },
    { "Glyph of Warding", 95, GLYPH_OF_WARDING },
    { "Demon Bane", 85, DEMON_BANE },
    { "Banish Evil", 85, BANISH_EVIL },
    { "Word of Destruction", 75, WORD_OF_DESTRUCTION },
    { "Holy Word", 85, HOLY_WORD },
    { "Spear of Orom\xC3\xab", 85, SPEAR_OF_OROME }, /* "Spear of Oromë" */
    { "Light of Manw\xC3\xab", 85, LIGHT_OF_MANWE } /* "Light of Manwë"*/
};
static borg_spell_rating borg_spell_ratings_NECROMANCER[] =
{
    { "Nether Bolt", 95, NETHER_BOLT },
    { "Sense Invisible", 85, SENSE_INVISIBLE },
    { "Create Darkness", 5, CREATE_DARKNESS }, /* not sure this is borg happy */
    { "Bat Form", 5, BAT_FORM }, // !FIX !TODO !AJG shapechage
    { "Read Minds", 85, READ_MINDS },
    { "Tap Unlife", 85, TAP_UNLIFE },
    { "Crush", 95, CRUSH },
    { "Sleep Evil", 85, SLEEP_EVIL },
    { "Shadow Shift", 95, SHADOW_SHIFT },
    { "Disenchant", 25, DISENCHANT },
    { "Frighten", 85, FRIGHTEN },
    { "Vampire Strike", 75, VAMPIRE_STRIKE },
    { "Dispel Life", 65, DISPEL_LIFE },
    { "Dark Spear", 65, DARK_SPEAR },
    { "Warg Form", 5, WARG_FORM }, // !FIX !TODO !AJG shapechage
    { "Banish Spirits", 65, BANISH_SPIRITS },
    { "Annihilate", 95, ANNIHILATE },
    { "Grond's Blow", 85, GRONDS_BLOW },
    { "Unleash Chaos", 85, UNLEASH_CHAOS },
    { "Fume of Mordor", 75, FUME_OF_MORDOR },
    { "Storm of Darkness", 65, STORM_OF_DARKNESS },
    { "Power Sacrifice", 5, POWER_SACRIFICE },  /* not sure if this is borg happy. */
    { "Zone of Unmagic", 5, ZONE_OF_UNMAGIC },  // !FIX !TODO !AJG defense?  not sure how to code. 
    { "Vampire Form", 5, VAMPIRE_FORM }, // !FIX !TODO !AJG shapechage
    { "Curse", 65, CURSE },
    { "Command", 5, COMMAND } // !FIX !TODO !AJG defense?  not sure how to code. 
};
static borg_spell_rating borg_spell_ratings_PALADIN[] =
{
    { "Bless", 95, BLESS },
    { "Detect Evil", 85, DETECT_EVIL },
    { "Call Light", 85, CALL_LIGHT },
    { "Minor Healing", 95, MINOR_HEALING },
    { "Sense Invisible", 65, SENSE_INVISIBLE },
    { "Heroism", 85, HEROISM },
    { "Protection from Evil", 85, PROTECTION_FROM_EVIL },
    { "Remove Curse", 65, REMOVE_CURSE },
    { "Word of Recall", 95, WORD_OF_RECALL },
    { "Healing", 95, HEALING },
    { "Clairvoyance", 85, CLAIRVOYANCE },
    { "Smite Evil", 55, SMITE_EVIL },
    { "Demon Bane", 55, DEMON_BANE },
    { "Enchant Weapon", 75, ENCHANT_WEAPON },
    { "Enchant Armour", 85, ENCHANT_ARMOUR },
    { "Single Combat", 95, SINGLE_COMBAT } // !FIX !TODO !AJG defense?  not sure how to code.
};
static borg_spell_rating borg_spell_ratings_ROGUE[] =
{
    { "Detect Monsters", 85, DETECT_MONSTERS },
    { "Phase Door", 95, PHASE_DOOR },
    { "Object Detection", 55, OBJECT_DETECTION },
    { "Detect Stairs", 55, DETECT_STAIRS },
    { "Recharging", 85, RECHARGING },
    { "Reveal Monsters", 85, REVEAL_MONSTERS },
    { "Teleport Self", 95, TELEPORT_SELF },
    { "Hit and Run", 15, HIT_AND_RUN }, // !FIX !TODO !AJG not sure how to code this
    { "Teleport Other", 85, TELEPORT_OTHER },
    { "Teleport Level", 75, TELEPORT_LEVEL }
};
static borg_spell_rating borg_spell_ratings_RANGER[] =
{
    { "Remove Hunger", 95, REMOVE_HUNGER },
    { "Detect Life", 85, DETECT_LIFE },
    { "Herbal Curing", 95, HERBAL_CURING },
    { "Resist Poison", 85, RESIST_POISON },
    { "Turn Stone to Mud", 85, TURN_STONE_TO_MUD },
    { "Sense Surroundings", 75, SENSE_SURROUNDINGS },
    { "Cover Tracks", 25, COVER_TRACKS }, // !FIX !TODO !AJG prep?
    { "Create Arrows", 85, CREATE_ARROWS }, // !FIX !TODO !AJG 
    { "Haste Self", 95, HASTE_SELF },
    { "Decoy", 5, DECOY }, // !FIX !TODO !AJG not sure what to do with this
    { "Brand Ammunition", 95, BRAND_AMMUNITION }
};
static borg_spell_rating borg_spell_ratings_BLACKGUARD[] =
{
    { "Seek Battle", 55, SEEK_BATTLE },
    { "Berserk Strength", 95, BERSERK_STRENGTH },
    { "Whirlwind Attack", 85, WHIRLWIND_ATTACK },
    { "Shatter Stone", 95, SHATTER_STONE },
    { "Leap into Battle", 65, LEAP_INTO_BATTLE },
    { "Grim Purpose", 65, GRIM_PURPOSE },
    { "Maim Foe", 75, MAIM_FOE },
    { "Howl of the Damned", 55, HOWL_OF_THE_DAMNED },
    { "Relentless Taunting", 5, RELENTLESS_TAUNTING }, /* seems to dangerous for borg right now */
    { "Venom", 55, VENOM },
    { "Werewolf Form", 5, WEREWOLF_FORM }, // !FIX !TODO !AJG shapechage
    { "Bloodlust", 5, BLOODLUST }, /* seems to dangerous for borg right now */
    { "Unholy Reprieve", 95, UNHOLY_REPRIEVE },
    { "Forceful Blow", 5, FORCEFUL_BLOW }, // !FIX !TODO !AJG need to code this 
    { "Quake", 95, QUAKE }
};


/*
 * Constant "item description parsers" (singles)
 */
static int      borg_single_size;        /* Number of "singles" */
static int16_t* borg_single_what;      /* Kind indexes for "singles" */
static char**   borg_single_text;      /* Textual prefixes for "singles" */

/*
 * Constant "item description parsers" (plurals)
 */
static int      borg_plural_size;        /* Number of "plurals" */
static int16_t* borg_plural_what;      /* Kind index for "plurals" */
static char**   borg_plural_text;      /* Textual prefixes for "plurals" */
static char**   borg_sv_plural_text;   /* Save Textual prefixes for "plurals" (in kind order) */

/*
 * Constant "item description parsers" (suffixes)
 */
static int      borg_artego_size;        /* Number of "artegos" */
static int16_t* borg_artego_what;      /* Indexes for "artegos" */
static char**   borg_artego_text;      /* Textual prefixes for "artegos" */
static char**   borg_sv_art_text;      /* Save textual prefixes for "artifacts" (in kind order) */

/*
 * Return the slot that items of the given type are wielded into
 * XXX this just duplicates Angband's version and should use that instead
 *
 * Returns "-1" if the item cannot be wielded
 */
int borg_wield_slot(const borg_item* item)
{
    switch (item->tval) {
    case TV_SWORD:
    case TV_POLEARM:
    case TV_HAFTED:
    case TV_DIGGING: return INVEN_WIELD;

    case TV_DRAG_ARMOR:
    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR: return INVEN_BODY;

    case TV_SHIELD: return INVEN_ARM;

    case TV_CROWN:
    case TV_HELM: return INVEN_HEAD;

    case TV_BOW: return INVEN_BOW;
    case TV_RING: return INVEN_LEFT;
    case TV_AMULET: return INVEN_NECK;
    case TV_LIGHT: return INVEN_LIGHT;
    case TV_CLOAK: return INVEN_OUTER;
    case TV_GLOVES: return INVEN_HANDS;
    case TV_BOOTS: return INVEN_FEET;
    }

    /* No slot available */
    return -1;
}

/*
 * Get the ID information
 *
 * This function pulls the information from the screen if it is not passed
 * a *real* item.  It is only passed in *real* items if the borg is allowed
 * to 'cheat' for inventory.
 * This function returns true if space needs to be pressed
 */
bool borg_object_fully_id_aux(borg_item* item, struct object* real_item)
{
    bitflag f[OF_SIZE];
    bitflag i = OF_SIZE;

    /* the data directly from the real item    */
    object_flags(real_item, f);
    for (i = 0; i < 12 && i < OF_SIZE; i++) item->flags[i] = f[i];

    return (false);
}

const char* borg_get_note(const borg_item* item)
{
    if (item->note)
        return item->note;
    return "";
}

/*
 * The code currently inscribes items with {??} if they have unknown powers.
 *
 * This helper keeps the check issolated in case this ever changes
 */
bool borg_item_note_needs_id(const borg_item* item)
{
    if (item->ident)
        return false;

    /* save a string check */
    if (item->needs_ident)
        return true;

    return strstr(borg_get_note(item), "{??}");
}

/*
 * Look for an item that needs to be analysed because it has been IDd
 *
 * This will go through inventory and look for items that were just ID'd
 * and examine them for their bonuses.
 */
bool borg_object_fully_id(void)
{
    int i;

    /* look in inventory and equiptment for something to *id* */
    for (i = 0; i < QUIVER_END; i++) /* or INVEN_TOTAL */
    {

        borg_item* item = &borg_items[i];
        struct ego_item* e_ptr = &e_info[item->ego_idx];

        /* inscribe certain objects */
        const char* note = borg_get_note(item);
        if (borg_skill[BI_CDEPTH] && item->tval >= TV_BOW && item->tval <= TV_RING &&
            (borg_ego_has_random_power(e_ptr) || item->art_idx) &&
            (streq(note, "{ }") || streq(note, "") || strstr(note, "uncursed")))
        {

            /* make the inscription */
            borg_keypress('{');

            if (i >= INVEN_WIELD)
            {
                borg_keypress('/');
                borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
            }
            else
            {
                borg_keypress(all_letters_nohjkl[i]);
            }

            /* make the inscription */
            if (item->modifiers[OBJ_MOD_SPEED])
            {
                borg_keypresses("Spd");
            }
            /* slays and immunities */
            if (item->el_info[ELEM_POIS].res_level > 0)
            {
                borg_keypresses("Poisn");
            }
            if (item->el_info[ELEM_FIRE].res_level == 3)
            {
                borg_keypresses("IFir");
            }
            if (item->el_info[ELEM_COLD].res_level == 3)
            {
                borg_keypresses("ICld");
            }
            if (item->el_info[ELEM_ACID].res_level == 3)
            {
                borg_keypresses("IAcd");
            }
            if (item->el_info[ELEM_ELEC].res_level == 3)
            {
                borg_keypresses("IElc");
            }
            if (item->el_info[ELEM_LIGHT].res_level > 0)
            {
                borg_keypresses("Lite");
            }
            if (item->el_info[ELEM_DARK].res_level > 0)
            {
                borg_keypresses("Dark");
            }
            if (of_has(item->flags, OF_PROT_BLIND))
            {
                borg_keypresses("Blnd");
            }
            if (of_has(item->flags, OF_PROT_CONF))
            {
                borg_keypresses("Conf");
            }
            if (item->el_info[ELEM_SOUND].res_level > 0)
            {
                borg_keypresses("Sound");
            }
            if (item->el_info[ELEM_SHARD].res_level > 0)
            {
                borg_keypresses("Shrd");
            }
            if (item->el_info[ELEM_NETHER].res_level > 0)
            {
                borg_keypresses("Nthr");
            }
            if (item->el_info[ELEM_NEXUS].res_level > 0)
            {
                borg_keypresses("Nxs");
            }
            if (item->el_info[ELEM_CHAOS].res_level > 0)
            {
                borg_keypresses("Chaos");
            }
            if (item->el_info[ELEM_DISEN].res_level > 0)
            {
                borg_keypresses("Disn");
            }
            /* TR2_activate was removed */
            if (item->activ_idx)
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
            borg_keypress(KC_ENTER);

        }
    }
    return true;
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
static int32_t borg_object_value_known(borg_item* item)
{
    int32_t value;


    struct object_kind* k_ptr = &k_info[item->kind];

    /* Worthless items */
    if (!k_ptr->cost) return (0L);

    /* Extract the base value */
    value = k_ptr->cost;


    /* Hack -- use artifact base costs */
    if (item->art_idx)
    {
        struct artifact* a_ptr = &a_info[item->art_idx];

        /* Worthless artifacts */
        if (!a_ptr->cost) return (0L);

        /* Hack -- use the artifact cost */
        value = a_ptr->cost;
    }

    /* Hack -- add in ego-item bonus cost */
    if (item->ego_idx)
    {
        struct ego_item* e_ptr = &e_info[item->ego_idx];

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
        if (of_has(item->flags, STAT_STR)) value += (item->pval * 200L);
        if (of_has(item->flags, STAT_INT)) value += (item->pval * 200L);
        if (of_has(item->flags, STAT_WIS)) value += (item->pval * 200L);
        if (of_has(item->flags, STAT_DEX)) value += (item->pval * 200L);
        if (of_has(item->flags, STAT_CON)) value += (item->pval * 200L);

        /* Give credit for stealth and searching */
        value += (item->modifiers[OBJ_MOD_STEALTH] * 100L);
        value += (item->modifiers[OBJ_MOD_SEARCH] * 100L);

        /* Give credit for infra-vision and tunneling */
        value += (item->modifiers[OBJ_MOD_INFRA] * 50L);
        value += (item->modifiers[OBJ_MOD_TUNNEL] * 50L);

        /* Give credit for extra attacks */
        if (item->modifiers[OBJ_MOD_BLOWS])
        {
            if (item->modifiers[OBJ_MOD_BLOWS] > MAX_BLOWS)
            {
                value += (MAX_BLOWS * 2000L);
            }
            else
            {
                value += (item->modifiers[OBJ_MOD_BLOWS] * 2000L);
            }
        }
        value += (item->modifiers[OBJ_MOD_SHOTS] * 2000L);

        /* Give credit for speed bonus */
        value += (item->modifiers[OBJ_MOD_SPEED] * 30000L);

        /* Give credit for glowing bonus */
        value += (item->modifiers[OBJ_MOD_LIGHT] * 100L);

        /* Give credit for might */
        value += (item->modifiers[OBJ_MOD_MIGHT] * 100L);

        /* Give credit for moves */
        value += (item->modifiers[OBJ_MOD_MOVES] * 100L);

        /* Give credit for damage reduction */
        value += (item->modifiers[OBJ_MOD_DAM_RED] * 200L);

        break;
    }
    }


    /* Analyze the item */
    switch (item->tval)
    {
        /* Rings/Amulets */
    case TV_RING:
        /* HACK special case */
        if (item->sval == sv_ring_dog)
            return (0L);

        /* Fall through */
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
        /* ignore low to_hit on armor for now since the base armor is marked */
        /* and that should be built into the value */
        if (item->to_h < 0 && item->to_h > -5)
            value += ((item->to_d + item->to_a) * 100L);
        else
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
 * Convert from the object slays structure to a basic multiplier per race
 */
static void borg_set_slays(borg_item* item, const struct object* o)
{
    int i;
    for (i = 0; i < z_info->slay_max; i++)
        if (o->slays[i])
            item->slays[slays[i].race_flag] = slays[i].multiplier;
}

/*
 * Convert from the dynmaic curses to the set the borg knows
 */
static void borg_set_curses(borg_item* item, const struct object* o)
{
    int i;
    bool* item_curses = item->curses;
    item->uncursable = true;
    for (i = 0; i < z_info->curse_max; i++)
    {
        struct curse* c = &curses[i];
        if (o->curses[i].power > 0)
        {
            item->cursed = true;

            if (o->curses[i].power == 100)
                item->uncursable = false;

            if (streq(c->name, "vulnerability"))
                item_curses[BORG_CURSE_VULNERABILITY] = true;
            else if (streq(c->name, "teleportation"))
                item_curses[BORG_CURSE_TELEPORTATION] = true;
            else if (streq(c->name, "dullness"))
                item_curses[BORG_CURSE_DULLNESS] = true;
            else if (streq(c->name, "sickliness"))
                item_curses[BORG_CURSE_SICKLINESS] = true;
            else if (streq(c->name, "enveloping"))
                item_curses[BORG_CURSE_ENVELOPING] = true;
            else if (streq(c->name, "irritation"))
                item_curses[BORG_CURSE_IRRITATION] = true;
            else if (streq(c->name, "weakness"))
                item_curses[BORG_CURSE_WEAKNESS] = true;
            else if (streq(c->name, "clumsiness"))
                item_curses[BORG_CURSE_CLUMSINESS] = true;
            else if (streq(c->name, "slowness"))
                item_curses[BORG_CURSE_SLOWNESS] = true;
            else if (streq(c->name, "annoyance"))
                item_curses[BORG_CURSE_ANNOYANCE] = true;
            else if (streq(c->name, "poison"))
                item_curses[BORG_CURSE_POISON] = true;
            else if (streq(c->name, "siren"))
                item_curses[BORG_CURSE_SIREN] = true;
            else if (streq(c->name, "hallucination"))
                item_curses[BORG_CURSE_HALLUCINATION] = true;
            else if (streq(c->name, "paralysis"))
                item_curses[BORG_CURSE_PARALYSIS] = true;
            else if (streq(c->name, "dragon summon"))
                item_curses[BORG_CURSE_DRAGON_SUMMON] = true;
            else if (streq(c->name, "demon summon"))
                item_curses[BORG_CURSE_DEMON_SUMMON] = true;
            else if (streq(c->name, "undead summon"))
                item_curses[BORG_CURSE_UNDEAD_SUMMON] = true;
            else if (streq(c->name, "impair mana recovery"))
                item_curses[BORG_CURSE_IMPAIR_MANA_RECOVERY] = true;
            else if (streq(c->name, "impair hitpoint recovery"))
                item_curses[BORG_CURSE_IMPAIR_HITPOINT_RECOVERY] = true;
            else if (streq(c->name, "cowardice"))
                item_curses[BORG_CURSE_COWARDICE] = true;
            else if (streq(c->name, "stone"))
                item_curses[BORG_CURSE_STONE] = true;
            else if (streq(c->name, "anti-teleportation"))
                item_curses[BORG_CURSE_ANTI_TELEPORTATION] = true;
            else if (streq(c->name, "treacherous weapon"))
                item_curses[BORG_CURSE_TREACHEROUS_WEAPON] = true;
            else if (streq(c->name, "burning up"))
                item_curses[BORG_CURSE_BURNING_UP] = true;
            else if (streq(c->name, "chilled to the bone"))
                item_curses[BORG_CURSE_CHILLED_TO_THE_BONE] = true;
            else if (streq(c->name, "steelskin"))
                item_curses[BORG_CURSE_STEELSKIN] = true;
            else if (streq(c->name, "air swing"))
                item_curses[BORG_CURSE_AIR_SWING] = true;
            else
                item_curses[BORG_CURSE_UNKNOWN] = true;
        }
    }

    /* this is to catch any items we removed all the curses from */
    if (!item->cursed)
        item->uncursable = false;
}

/*
 * Analyze an item, also given its name
 *
 * This cheats all the information, and maybe is getting information
 * that the player doesn't always get.  The best way to fix this is to
 * refactor the main game code to get it to make a 'fake' object that
 * contains only known info and copy from that.
 */
void borg_item_analyze(borg_item* item, const struct object* real_item,
    char* desc, bool in_store)
{
    char* scan;
    int i;
    const struct object* o;

    /* Wipe the item */
    memset(item, 0, sizeof(borg_item));

    /* Non-item */
    if (!real_item->kind || !real_item->number)
        return;

    /* see if the object is fully identified.  If it is, use the base object */
    item->ident = object_fully_known(real_item);
    if (item->ident)
        o = real_item;
    else
    {
        /* this needs to be a good pointer or object_flags_know will crash */
        if (!real_item->known)
            o = real_item;
        else
            o = real_item->known;
    }
    item->needs_ident = !object_runes_known(real_item);

    /* Extract data from the game */
    object_flags_known(real_item, item->flags);

    /* Save the item description */
    my_strcpy(item->desc, desc, sizeof item->desc);

    /* Advance to the "inscription" or end of string and save */
    for (scan = item->desc; *scan && (*scan != '{'); scan++) /* loop */;
    item->note = scan;

    /* Get various info */
    item->tval = real_item->tval;
    item->sval = real_item->sval;
    item->iqty = real_item->number;
    item->weight = real_item->weight;
    item->timeout = real_item->timeout;
    item->level = real_item->kind->level;
    item->aware = object_flavor_is_aware(real_item);

    /* get info from the known part of the object */
    item->ac = o->ac;
    item->dd = o->dd;
    item->ds = o->ds;
    item->to_h = o->to_h;
    item->to_d = o->to_d;
    item->to_a = o->to_a;

    /* copy modifiers that are known */
    for (i = 0; i < OBJ_MOD_MAX; i++)
        item->modifiers[i] = o->modifiers[i];

    for (i = 0; i < ELEM_MAX; i++)
    {
        item->el_info[i].res_level = o->el_info[i].res_level;
        item->el_info[i].flags = o->el_info[i].flags;
    }

    if (o->curses != NULL)
        borg_set_curses(item, o);

    if (o->slays)
        borg_set_slays(item, o);

    if (o->brands)
    {
        for (i = 0; i < z_info->brand_max; i++)
            item->brands[i] = o->brands[i];
    }

    /* check if we know this is the one ring */
    /* HACK we assume The One Ring is the only artifact that does BIZARRE */
    if (o->activation)
    {
        if (o->activation->index == act_bizarre)
            item->one_ring = true;
        item->activ_idx = o->activation->index;
    }

    /* default the pval */
    if (item->ident)
        item->pval = o->pval;

    /* Rods are considered pval 1 if charged */
    if (item->tval == TV_ROD) {
        /* XXX There should be an obj_rod_charging() function for this logic */
        /* This was ripped from object/obj-desc.c */
        if (item->iqty == 1) {
            item->pval = real_item->timeout ? 0 : 1;
        }
        else {
            int power;
            int time_base = randcalc(real_item->kind->time, 0, MINIMISE);
            if (!time_base) time_base = 1;

            /*
             * Find out how many rods are charging, by dividing
             * current timeout by each rod's maximum timeout.
             * Ensure that any remainder is rounded up.  Display
             * very discharged stacks as merely fully discharged.
             */
            power = (real_item->timeout + (time_base - 1)) / time_base;
            item->pval = (power < item->iqty) ? 1 : 0;
        }
    }
    else if (item->tval == TV_STAFF || item->tval == TV_WAND)
    {
        /* Staffs & wands considered charged unless they are known empty */

        /* Assume good */
        item->pval = 1;

        /* if Known, get correct pval */
        if (item->ident) item->pval = real_item->pval;

        /* if seen {empty} assume pval 0 */
        if (!item->aware && !o->pval) item->pval = 0;
        if (strstr(borg_get_note(item), "empty")) item->pval = 0;
    }

    /* Kind index -- Only if partially ID or this is a store object */
    if (item->aware || in_store)
        item->kind = o->kind->kidx;

    if (o->artifact)
        item->art_idx = o->artifact->aidx;

    if (o->ego)
        item->ego_idx = o->ego->eidx;

    /* Notice values */
    if (item->ident) {
        item->value = borg_object_value_known(item);
    }
    else if (item->aware) {
        item->value = o->kind->cost;
    }
    else {
        /* Guess at value */
        switch (item->tval) {
        case TV_FOOD:     item->value = 5L; break;
        case TV_POTION:   item->value = 20L; break;
        case TV_SCROLL:   item->value = 20L; break;
        case TV_STAFF:    item->value = 70L; break;
        case TV_WAND:     item->value = 50L; break;
        case TV_ROD:      item->value = 90L; break;
        case TV_RING:
        case TV_AMULET:   item->value = 45L; break;
        default:          item->value = 20L; break;
        }
    }

    /* If it's not The One Ring, then it's worthless if cursed */
    if (item->cursed && !item->one_ring)
        item->value = 0L;
}

extern struct borg_spell_messages* spell_msgs;
extern struct borg_spell_messages* spell_invis_msgs;



/*
 * Send a command to inscribe item number "i" with the inscription "str".
 */
void borg_send_inscribe(int i, char* str)
{
    char* s;

    /* Label it */
    borg_keypress('{');

    /* Choose from inventory */
    if (i < INVEN_WIELD)
    {
        /* Choose the item */
        borg_keypress(all_letters_nohjkl[i]);
    }

    /* Choose from equipment */
    else
    {
        /* Go to equipment (if necessary) */
        if (borg_items[0].iqty) borg_keypress('/');

        /* Choose the item */
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
    }

    /* Send the label */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the inscription */
    borg_keypress(KC_ENTER);

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
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

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

enum borg_need borg_maintain_light(void)
{
    int i;
    borg_item* current_light = &borg_items[INVEN_LIGHT];

    if (of_has(current_light->flags, OF_NO_FUEL))
        return BORG_NO_NEED;

    /*  current torch */
    if (current_light->tval == TV_LIGHT)
    {
        if (current_light->sval == sv_light_torch)
        {
            if (current_light->timeout > 250)
            {
                return BORG_NO_NEED;
            }
            else
            {
                /* Look for another torch */
                i = borg_slot(TV_LIGHT, sv_light_torch);
                if (i < 0)
                    return BORG_UNMET_NEED;

                /* Torches automatically disappear when they get to 0 turns
                 * so we don't need to actively swap them out */
                return BORG_NO_NEED;
            }
        }

        /* Refuel current lantern */
        if (current_light->sval == sv_light_lantern)
        {
            /* Refuel the lantern if needed */
            if (borg_items[INVEN_LIGHT].timeout < 1000)
            {
                if (borg_refuel_lantern())
                    return BORG_MET_NEED;

                return BORG_UNMET_NEED;
            }
        }
        return BORG_NO_NEED;
    }
    else
    {
        i = borg_slot(TV_LIGHT, sv_light_lantern);
        if (i < 0)
        {
            i = borg_slot(TV_LIGHT, sv_light_torch);
        }

        if (i < 0)
        {
            return BORG_UNMET_NEED;
        }
        else {
            borg_keypress('w');
            borg_keypress(all_letters_nohjkl[i]);
            return BORG_MET_NEED;
        }
    }
}

/*
 * Hack -- refuel a lantern
 */
bool borg_refuel_lantern(void)
{
    int i;

    /* Look for a flask of oil */
    i = borg_slot(TV_FLASK, sv_flask_oil);

    /* None available check for lantern */
    if (i < 0)
    {
        i = borg_slot(TV_LIGHT, sv_light_lantern);

        /* It better have some oil left */
        if (i >= 0 && borg_items[i].timeout <= 0) i = -1;
    }

    /* Still none */
    if (i < 0) return (false);

    /* Cant refuel a torch with oil */
    if (borg_items[INVEN_LIGHT].sval != sv_light_lantern)
    {
        return (false);
    }

    /* Log the message */
    borg_note(format("# Refueling with %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('F');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
}

/*
 * Helper to see if an object does a certain thing 
 * 
 * kind     - kind index
 * index    - index of the effect
 * subtype  - subtype of the effect 
 */
bool borg_obj_has_effect(uint32_t kind, int index, int subtype)
{
    struct effect* ke = k_info[kind].effect;
    while (ke)
    {
        if (ke->index == index && (ke->subtype == subtype || subtype == -1))
            return true;
        ke = ke->next;
    }
    return false;
}


/*
 * Hack -- attempt to eat the given food (by sval)
 */
bool borg_eat_food(int tval, int sval)
{
    int i;

    /* Look for that food */
    i = borg_slot(tval, sval);

    /* None available */
    if (i < 0) return (false);

    /* Log the message */
    borg_note(format("# Eating %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
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
bool borg_quaff_crit(bool no_check)
{
    static int16_t when_last_quaff = 0;

    if (no_check)
    {
        if (borg_quaff_potion(sv_potion_cure_critical))
        {
            when_last_quaff = borg_t;
            return (true);
        }
        return (false);
    }

    /* Avoid drinking CCW twice in a row */
    if (when_last_quaff > (borg_t - 4) &&
        when_last_quaff <= borg_t &&
        (randint1(100) < 75))
        return false;

    /* Save the last two for when we really need them */
    if (borg_skill[BI_ACCW] < 2)
        return false;

    if (borg_quaff_potion(sv_potion_cure_critical))
    {
        when_last_quaff = borg_t;
        return (true);
    }
    return (false);
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
    if (i < 0) return (false);

    /* Log the message */
    borg_note(format("# Quaffing %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
}
/*
 * Hack -- attempt to quaff an unknown potion
 */
bool borg_quaff_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

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
    if (n < 0) return (false);

    /* Log the message */
    borg_note(format("# Quaffing unknown potion %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress(all_letters_nohjkl[n]);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
}

/*
 * Hack -- attempt to read an unknown scroll
 */
bool borg_read_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

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
    if (n < 0) return (false);

    /* Dark */
    if (no_light(player)) return (false);

    /* Blind or Confused */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (false);

    /* Log the message */
    borg_note(format("# Reading unknown scroll %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('r');
    borg_keypress(all_letters_nohjkl[n]);

    /* Incase it is ID scroll, ESCAPE out. */
    borg_keypress(ESCAPE);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
}


/*
 * Hack -- attempt to eat an unknown potion.  This is done in emergencies.
 */
bool borg_eat_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

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
    if (n < 0) return (false);

    /* Log the message */
    borg_note(format("# Eating unknown mushroom %s.", borg_items[n].desc));

    /* Perform the action */
    borg_keypress('E');
    borg_keypress(all_letters_nohjkl[n]);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
}

/*
 * Hack -- attempt to use an unknown staff.  This is done in emergencies.
 */
bool borg_use_unknown(void)
{
    int i, n = -1;

    /* Scan the pack */
    for (i = 0; i < z_info->pack_size; i++)
    {
        borg_item* item = &borg_items[i];

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
    if (n < 0) return (false);

    /* Log the message */
    borg_note(format("# Using unknown Staff %s.", borg_items[n].desc));

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = n;

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(all_letters_nohjkl[n]);

    /* Incase it is ID staff, ESCAPE out. */
    borg_keypress(ESCAPE);

    /* Success */
    return (true);
}


/*
 * Hack -- attempt to read the given scroll (by sval)
 */
bool borg_read_scroll(int sval)
{
    int i;

    /* Dark */
    if (no_light(player)) return (false);

    /* Blind or Confused or Amnesia*/
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED] ||
        borg_skill[BI_ISFORGET]) return (false);

    /* Look for that scroll */
    i = borg_slot(TV_SCROLL, sval);

    /* None available */
    if (i < 0) return (false);

    /* Log the message */
    borg_note(format("# Reading %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress(ESCAPE);
    borg_keypress(ESCAPE);
    borg_keypress('r');
    borg_keypress(all_letters_nohjkl[i]);

    /* Hack -- Clear "shop" goals */
    goal_shop = goal_ware = goal_item = -1;

    /* Success */
    return (true);
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

    /* no known activation */
    if (!borg_items[i].activ_idx) return (100);

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
    if (i < 0) return (false);

    /* No charges */
    if (!borg_items[i].pval) return (false);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage (at least 1/2 chance of success. */
    if (fail > 500) return (false);

    /* Yep we got one */
    return (true);
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
    if (i < 0) return (false);

    /* Hack -- Still charging */
    if (!borg_items[i].pval) return (false);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* Roll for usage */
    if (sval != sv_rod_recall)
    {
        if (fail > 500) return (false);
    }

    /* Log the message */
    borg_note(format("# Zapping %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('z');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return (true);
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
    if (i < 0) return (false);

    /* No charges */
    if (!borg_items[i].pval) return (false);

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = i;

    /* Log the message */
    borg_note(format("# Aiming %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('a');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return (true);
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
    if (i < 0) return (false);

    /* No charges */
    if (!borg_items[i].pval) return (false);

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = i;

    /* Log the message */
    borg_note(format("# Using %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return (true);
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
    if (i < 0) return (false);

    /* No charges */
    if (!borg_items[i].pval) return (false);

    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = i;

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
        if (sval != sv_staff_teleportation)
        {
            return (false);
        }

        /* We need to give some "desparation attempt to teleport staff" */
        if (!borg_skill[BI_ISCONFUSED] && !borg_skill[BI_ISBLIND]) /* Dark? */
        {
            /* We really have no chance, return false, attempt the scroll */
            if (fail > 500) return (false);
        }
        /* We might have a slight chance, or we cannot not read */
    }


    /* record the address to avoid certain bugs with inscriptions&amnesia */
    borg_zap_slot = i;

    /* Log the message */
    borg_note(format("# Using %s.", borg_items[i].desc));

    /* Perform the action */
    borg_keypress('u');
    borg_keypress(all_letters_nohjkl[i]);

    /* Success */
    return (true);
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
    if (i < 0) return (false);

    /* No charges */
    if (!borg_items[i].pval) return (false);

    /* Extract the item level */
    lev = (borg_items[i].level);

    /* Base chance of success */
    skill = borg_skill[BI_DEV];

    /* Confusion hurts skill */
    if (borg_skill[BI_ISCONFUSED]) skill = skill * 75 / 100;

    /* High level objects are harder */
    fail = 100 * ((skill - lev) - (141 - 1)) / ((lev - skill) - (100 - 10));

    /* If its a Destruction, we only use it in emergencies, attempt it */
    if (sval == sv_staff_destruction)
    {
        return (true);
    }

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500)
    {
        /* No real chance of success on other types of staffs */
        if (sval != sv_staff_teleportation)
        {
            return (false);
        }

        /* We need to give some "desparation attempt to teleport staff" */
        if (sval == sv_staff_teleportation && !borg_skill[BI_ISCONFUSED])
        {
            /* We really have no chance, return false, attempt the scroll */
            if (fail < 650) return (false);
        }

        /* We might have a slight chance (or its a Destruction), continue on */
    }

    /* Yep we got one */
    return (true);
}



/*
 * Attempt to use the given artifact
 */
bool borg_activate_item(int activation)
{
    int i;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item* item = &borg_items[i];

        /* Skip wrong activation*/
        if (item->activ_idx != activation)
            continue;

        /* Check charge */
        if (item->timeout) continue;

        /* Log the message */
        borg_note(format("# Activating item %s.", item->desc));

        /* Perform the action */
        borg_keypress('A');
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);

        /* Success */
        return (true);
    }

    /* Oops */
    return (false);
}


/*
 * Hack -- check and see if borg is wielding an item with this activation
 */
bool borg_equips_item(int activation, bool check_charge)
{
    int i;

    /* Check the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
    {
        borg_item* item = &borg_items[i];

        /* Skip wrong activation */
        if (item->activ_idx != activation) continue;

        /* Check charge.  */
        if (check_charge && (item->timeout >= 1)) continue;

        /* Success */
        return (true);
    }

    /* I do not have it or it is not charged */
    return (false);
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
    borg_item* item = &borg_items[INVEN_BODY];

    /* Skip incorrect armours */
    if (item->tval != TV_DRAG_ARMOR) return (false);
    if (item->sval != drag_sval) return (false);

    /* Check charge */
    if (item->timeout) return (false);

    /*  Make Sure Mail is IDed */
    if (!item->ident) return (false);


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
    numerator = (skill - lev) - (141 - 1);
    denominator = (lev - skill) - (100 - 10);

    /* Make sure that we don't divide by zero */
    if (denominator == 0) denominator = numerator > 0 ? 1 : -1;

    fail = (100 * numerator) / denominator;

    /* Roll for usage, but if its a Teleport be generous. */
    if (fail > 500) return (false);

    /* Success */
    return (true);

}

/*
 *  Hack -- attempt to use the given dragon armour
 */
bool borg_activate_dragon(int drag_sval)
{
    /* Check the equipment */

    borg_item* item = &borg_items[INVEN_BODY];

    /* Skip incorrect mails */
    if (item->tval != TV_DRAG_ARMOR) return (false);
    if (item->sval != drag_sval) return (false);

    /* Check charge */
    if (item->timeout) return (false);

    /*  Make Sure Mail is IDed */
    if (!item->ident) return (false);

    /* Log the message */
    borg_note(format("# Activating dragon scale %s.", item->desc));

    /* Perform the action */
    borg_keypress('A');
    borg_keypress(all_letters_nohjkl[INVEN_BODY - INVEN_WIELD]);

    /* Success */
    return (true);
}

/*
 * Hack -- check and see if borg is wielding a ring and if
 * he will pass a fail check.
 */
bool borg_equips_ring(int ring_sval)
{
    int lev, fail, i;
    int skill;

    for (i = INVEN_RIGHT; i < INVEN_LEFT; i++)
    {
        borg_item* item = &borg_items[i];

        /* Skip incorrect armours */
        if (item->tval != TV_RING) continue;
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
        return (true);
    }

    return (false);

}

/*
 *  Hack -- attempt to use the given ring
 */
bool borg_activate_ring(int ring_sval)
{

    int i;

    /* Check the equipment */
    for (i = INVEN_RIGHT; i < INVEN_LEFT; i++)
    {
        borg_item* item = &borg_items[i];

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
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);

        /* Success */
        return (true);
    }

    return (false);
}

/*
 * get the stat used for casting spells
 *
 * *HACK* assumes the first spell determins the relm thus stat for all spells
*/
int borg_spell_stat(void)
{
    if (player->class->magic.total_spells)
    {
        struct class_spell* spell = &(player->class->magic.books[0].spells[0]);
        if (spell != NULL)
        {
            return spell->realm->stat;
        }
    }

    return -1;
}

int borg_get_book_num(int sval)
{
    if (!player->class->magic.total_spells)
        return -1;

    for (int book_num = 0; book_num < player->class->magic.num_books; book_num++)
    {
        if (player->class->magic.books[book_num].sval == sval)
            return book_num;
    }
    return -1;
}

borg_magic* borg_get_spell_entry(int book, int what)
{
    int entry_in_book = 0;

    for (int spell_num = 0; spell_num < player->class->magic.total_spells; spell_num++)
    {
        if (borg_magics[spell_num].book == book)
        {
            if (entry_in_book == what)
                return &borg_magics[spell_num];
            entry_in_book++;
        }
    }
    return NULL;
}

static int borg_get_spell_number(const enum borg_spells spell)
{
    /* The borg must be able to "cast" spells */
    if (borg_magics == NULL) return -1;

    for (int spell_num = 0; spell_num < player->class->magic.total_spells; spell_num++)
    {
        if (borg_magics[spell_num].spell_enum == spell)
            return spell_num;
    }

    return -1;
}

int borg_get_spell_power(const enum borg_spells spell)
{
    int spell_num = borg_get_spell_number(spell);
    if (spell_num < 0) return -1;

    borg_magic* as = &borg_magics[spell_num];

    return as->power;
}


/*
 * Determine if borg can cast a given spell (when fully rested)
 */
bool borg_spell_legal(const enum borg_spells spell)
{
    int spell_num = borg_get_spell_number(spell);
    if (spell_num < 0) return false;

    borg_magic* as = &borg_magics[spell_num];

    /* The book must be possessed */
    if (borg_book[as->book] < 0) return (false);

    /* The spell must be "known" */
    if (borg_magics[spell_num].status < BORG_MAGIC_TEST) return (false);

    /* The spell must be affordable (when rested) */
    if (borg_magics[spell_num].power > borg_skill[BI_MAXSP]) return (false);

    /* Success */
    return (true);
}

static bool borg_spell_has_effect(int spell_num, uint16_t effect)
{
    const struct class_spell* cspell = spell_by_index(player, spell_num);
    struct effect* eff = cspell->effect;
    while (eff != NULL)
    {
        if (eff->index == effect)
            return true;
        eff = eff->next;
    }
    return false;
}

/*
 * Determine if borg can cast a given spell (right now)
 */
bool borg_spell_okay(const enum borg_spells spell)
{
    int reserve_mana = 0;

    int spell_num = borg_get_spell_number(spell);
    if (spell_num < 0) return false;

    borg_magic* as = &borg_magics[spell_num];

    /* Dark */
    if (no_light(player)) return (false);

    /* Define reserve_mana for each class */
    if (borg_class == CLASS_MAGE) reserve_mana = 6;
    if (borg_class == CLASS_RANGER) reserve_mana = 22;
    if (borg_class == CLASS_ROGUE) reserve_mana = 20;
    if (borg_class == CLASS_NECROMANCER) reserve_mana = 10;
    if (borg_class == CLASS_PRIEST) reserve_mana = 8;
    if (borg_class == CLASS_PALADIN) reserve_mana = 20;
    if (borg_class == CLASS_BLACKGUARD) reserve_mana = 0;

    /* Low level spell casters should not worry about this */
    if (borg_skill[BI_CLEVEL] < 35) reserve_mana = 0;

    /* Require ability (when rested) */
    if (!borg_spell_legal(spell)) return (false);

    /* Hack -- blind/confused/amnesia */
    if (borg_skill[BI_ISBLIND] || borg_skill[BI_ISCONFUSED]) return (false);


    /* The spell must be affordable (now) */
    if (as->power > borg_skill[BI_CURSP]) return (false);

    /* Do not cut into reserve mana (for final teleport) */
    if (borg_skill[BI_CURSP] - as->power < reserve_mana)
    {
        /* nourishing spells okay */
        if (borg_spell_has_effect(spell_num, EF_NOURISH)) return (true);

        /* okay to run away */
        if (borg_spell_has_effect(spell_num, EF_TELEPORT)) return (true);

        /* Magic Missile OK */
        if (MAGIC_MISSILE == spell && borg_skill[BI_CDEPTH] <= 35) return (true);

        /* others are rejected */
        return (false);
    }

    /* Success */
    return (true);
}

/*
 * fail rate on a spell
 */
int borg_spell_fail_rate(const enum borg_spells spell)
{
    int     chance, minfail;

    int spell_num = borg_get_spell_number(spell);
    if (spell_num < 0) return 100;

    borg_magic* as = &borg_magics[spell_num];

    /* Access the spell  */
    chance = as->sfail;

    /* Reduce failure rate by "effective" level adjustment */
    chance -= 3 * (borg_skill[BI_CLEVEL] - as->level);

    /* Reduce failure rate by stat adjustment */
    chance -= borg_skill[BI_FAIL1];

    /* Fear makes the failrate higher */
    if (borg_skill[BI_ISAFRAID]) chance += 20;

    /* Extract the minimum failure rate */
    minfail = borg_skill[BI_FAIL2];

    /* Non mage characters never get too good */
    if (!player_has(player, PF_ZERO_FAIL))
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
bool borg_spell_okay_fail(const enum borg_spells spell, int allow_fail)
{
    if (borg_spell_fail_rate(spell) > allow_fail)
        return false;
    return borg_spell_okay(spell);
}

/*
 * Same as borg_spell with a fail % check
 */
bool borg_spell_fail(const enum borg_spells spell, int allow_fail)
{
    if (borg_spell_fail_rate(spell) > allow_fail)
        return false;
    return borg_spell(spell);
}

/*
 * Same as borg_spell_legal with a fail % check
 */
bool borg_spell_legal_fail(const enum borg_spells spell, int allow_fail)
{
    if (borg_spell_fail_rate(spell) > allow_fail)
        return false;
    return borg_spell_legal(spell);
}

/*
 * Attempt to cast a spell
 */
bool borg_spell(const enum borg_spells spell)
{
    int i;

    int spell_num = borg_get_spell_number(spell);
    if (spell_num < 0) return false;

    borg_magic* as = &borg_magics[spell_num];

    /* Require ability (right now) */
    if (!borg_spell_okay(spell)) return (false);

    /* Look for the book */
    i = borg_book[as->book];

    /* Paranoia */
    if (i < 0) return (false);

    /* Debugging Info */
    borg_note(format("# Casting %s (%d,%d).", as->name, i, as->book_offset));

    /* Cast a spell */
    borg_keypress('m');
    borg_keypress(all_letters_nohjkl[i]);
    borg_keypress(all_letters_nohjkl[as->book_offset]);

    /* increment the spell counter */
    as->times++;

    /* Success */
    return (true);
}

/*
 * Inscribe food and Slime Molds
 */
extern bool borg_inscribe_food(void)
{
    int ii;
    char name[80];

    for (ii = 0; ii < INVEN_TOTAL; ii++)
    {
        borg_item* item = &borg_items[ii];

        /* Skip empty items */
        if (!item->iqty) continue;

        /* Require correct tval */
        if (item->tval != TV_FOOD) continue;

        /* skip things already inscribed */
        const char* note = borg_get_note(item);
        if (!(streq(note, "")) && !(streq(note, "{ }"))) continue;

        /* inscribe foods and molds */
        if (item->sval == sv_food_slime_mold || item->sval == sv_food_ration)
        {

            if (item->sval == sv_food_ration)
            {
                /* get a name */
                my_strcpy(name, food_syllable1[randint0(sizeof(food_syllable1) / sizeof(char*))], sizeof(name));
                my_strcat(name, food_syllable2[randint0(sizeof(food_syllable2) / sizeof(char*))], sizeof(name));

                borg_send_inscribe(ii, name);
                return (true);
            }

            if (item->sval == sv_food_slime_mold)
            {
                /* get a name */
                my_strcpy(name, mold_syllable1[randint0(sizeof(mold_syllable1) / sizeof(char*))], sizeof(name));
                my_strcat(name, mold_syllable2[randint0(sizeof(mold_syllable2) / sizeof(char*))], sizeof(name));
                my_strcat(name, mold_syllable3[randint0(sizeof(mold_syllable3) / sizeof(char*))], sizeof(name));

                borg_send_inscribe(ii, name);
                return (true);
            }

        }
    }

    /* all done */
    return (false);
}
/*
 * Send a command to de-inscribe item number "i" .
 */
void borg_send_deinscribe(int i)
{

    /* Ok to inscribe Slime Molds */
    if (borg_items[i].tval == TV_FOOD &&
        borg_items[i].sval == sv_food_slime_mold) return;

    /* Label it */
    borg_keypress('}');

    /* Choose from inventory */
    if (i < INVEN_WIELD)
    {
        /* Choose the item */
        borg_keypress(all_letters_nohjkl[i]);
    }

    /* Choose from equipment */
    else
    {
        /* Go to equipment (if necessary) */
        if (borg_items[0].iqty) borg_keypress('/');

        /* Choose the item */
        borg_keypress(all_letters_nohjkl[i - INVEN_WIELD]);
    }

    /* May ask for a confirmation */
    borg_keypress('y');
    borg_keypress('y');
}

/* (This is copied from store.c
 * Determine the price of an object (qty one) in a store.
 *
 *  store_buying == true  means the shop is buying, player selling
 *               == false means the shop is selling, player buying
 *
 * This function takes into account the player's charisma, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * object, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 */
static int32_t borg_price_item(const struct object* obj, bool store_buying, int qty, int this_store)
{
    int adjust = 100;
    int price;
    struct owner* proprietor;

    if (this_store == f_info[FEAT_HOME].shopnum - 1) {
        return 0;
    }

    proprietor = stores[this_store].owner;

    /* Get the value of the stack of wands, or a single item */
    if (tval_can_have_charges(obj)) {
        price = MIN(object_value_real(obj, qty), object_value(obj, qty));
    }
    else {
        price = MIN(object_value_real(obj, 1), object_value(obj, 1));
    }

    /* Worthless items */
    if (price <= 0) {
        return 0;
    }

    /* The black market is always a worse deal */
    if (this_store == f_info[FEAT_STORE_BLACK].shopnum - 1)
        adjust = 150;

    /* Shop is buying */
    if (store_buying) {
        /* Set the factor */
        adjust = 100 + (100 - adjust);
        if (adjust > 100) {
            adjust = 100;
        }

        /* Shops now pay 2/3 of true value */
        price = price * 2 / 3;

        /* Black market sucks */
        if (this_store == f_info[FEAT_STORE_BLACK].shopnum - 1) {
            price = price / 2;
        }

        /* Check for no_selling option */
        if (OPT(player, birth_no_selling)) {
            return 0;
        }
    }
    else {
        /* Re-evaluate if we're selling */
        if (tval_can_have_charges(obj)) {
            price = object_value_real(obj, qty);
        }
        else {
            price = object_value_real(obj, 1);
        }

        /* Black market sucks */
        if (this_store == f_info[FEAT_STORE_BLACK].shopnum - 1) {
            price = price * 2;
        }
    }

    /* Compute the final price (with rounding) */
    price = (price * adjust + 50L) / 100L;

    /* Now convert price to total price for non-wands */
    if (!tval_can_have_charges(obj)) {
        price *= qty;
    }

    /* Now limit the price to the purse limit */
    if (store_buying && (price > proprietor->max_cost * qty)) {
        price = proprietor->max_cost * qty;
    }

    /* Note -- Never become "free" */
    if (price <= 0) {
        return qty;
    }

    /* Return the price */
    return price;
}

/*
 * Cheat the quiver part of equipment
 */
static void borg_cheat_quiver(void)
{
    char buf[256];

    /* Extract the quiver */
    for (int j = 0, i = QUIVER_START; j < z_info->quiver_size; i++, j++)
    {
        memset(&borg_items[i], 0, sizeof(borg_item));

        struct object* obj = player->upkeep->quiver[j];
        if (obj)
        {
            /* Default to "nothing" */
            buf[0] = '\0';

            /* skip non items */
            if (!obj->kind)
                continue;

            /* Default to "nothing" */
            buf[0] = '\0';

            /* Describe a real item */
            if (obj->kind)
            {
                /* Describe it */
                object_desc(buf, sizeof(buf), obj, ODESC_FULL, player);

                /* Analyze the item (no price) */
                borg_item_analyze(&borg_items[i], obj, buf, false);

                /* Uninscribe items with ! inscriptions */
                if (strstr(borg_items[i].desc, "!")) borg_send_deinscribe(i);
            }
        }
    }
}

/*
 * Cheat the "equip" screen
 */
void borg_cheat_equip(void)
{
    char buf[256];

    /* Extract the equipment */
    int count = player->body.count + z_info->pack_size;
    for (int j = 0, i = z_info->pack_size; i < count; i++, j++)
    {
        memset(&borg_items[i], 0, sizeof(borg_item));

        struct object* obj = player->body.slots[j].obj;
        if (obj)
        {
            /* Default to "nothing" */
            buf[0] = '\0';

            /* skip non items */
            if (!obj->kind)
                continue;

            /* Default to "nothing" */
            buf[0] = '\0';

            /* Describe a real item */
            if (obj->kind)
            {
                /* Describe it */
                object_desc(buf, sizeof(buf), obj, ODESC_FULL, player);

                /* Analyze the item (no price) */
                borg_item_analyze(&borg_items[i], obj, buf, false);

                /* Uninscribe items with ! inscriptions */
                if (strstr(borg_items[i].desc, "!")) borg_send_deinscribe(i);
            }
        }
    }
    borg_cheat_quiver();
}

/*
 * Cheat the "inven" screen
 */
void borg_cheat_inven(void)
{
    int i;

    char buf[256];

    /* Extract the current weight */
    borg_cur_wgt = player->upkeep->total_weight;

    /* Extract the inventory */
    for (i = 0; i < z_info->pack_size; i++)
    {
        struct object* obj = player->upkeep->inven[i];
        memset(&borg_items[i], 0, sizeof(borg_item));

        /* Skip non-items */
        if (!obj || !obj->kind) continue;

        /* Default to "nothing" */
        buf[0] = '\0';

        /* Describe it */
        object_desc(buf, sizeof(buf), obj, ODESC_FULL, player);

        /* Skip Empty slots */
        if (streq(buf, "(nothing)")) continue;


        /* Analyze the item (no price) */
        borg_item_analyze(&borg_items[i], obj, buf, false);

        /* Note changed inventory */
        borg_do_crush_junk = true;
        borg_do_crush_hole = true;
        borg_do_crush_slow = true;

        /* Uninscribe items with ! inscriptions */
        if (strstr(borg_items[i].desc, "!"))
        {
            borg_send_deinscribe(i);
        }
    }
}

/*
 * Cheat the "Store" screen
 */
void borg_cheat_store(void)
{
    int slot, i;
    int store_num;
    struct object* o_ptr;
    struct object** list = mem_zalloc(sizeof(struct object*) * z_info->store_inven_max);

    /* Scan each store */
    for (store_num = 0; store_num < z_info->store_max; store_num++)
    {
        struct store* st_ptr = &stores[store_num];

        /* Clear the Inventory from memory */
        for (i = 0; i < z_info->store_inven_max; i++)
        {
            /* Wipe the ware */
            memset(&borg_shops[store_num].ware[i], 0, sizeof(borg_item));
        }

        store_stock_list(st_ptr, list, z_info->store_inven_max);

        /* Check each existing object in this store */
        for (slot = 0; slot < z_info->store_inven_max && list[slot]; slot++)
        {
            o_ptr = list[slot];
            borg_item* b_item = &borg_shops[store_num].ware[slot];
            char buf[120];

            /* Describe the item */
            object_desc(buf, sizeof(buf), o_ptr, ODESC_FULL | ODESC_STORE, player);
            if (streq(buf, "(nothing)")) break;

            /* Analyze the item */
            borg_item_analyze(b_item, o_ptr, buf, store_num == 7 ? false : true);

            /* Check if the general store has certain items */
            if (store_num == 0)
            {
                /* Food -- needed for money scumming */
                if (b_item->tval == TV_FOOD && b_item->sval == sv_food_ration)
                    borg_food_onsale = b_item->iqty;

                /* Fuel for lanterns */
                if (b_item->tval == TV_FLASK &&
                    borg_items[INVEN_LIGHT].sval == sv_light_lantern)
                    borg_fuel_onsale = b_item->iqty;

                /* Fuel for lanterns */
                if (b_item->tval == TV_LIGHT &&
                    borg_items[INVEN_LIGHT].sval == sv_light_torch)
                    borg_fuel_onsale = b_item->iqty;
            }

            /* Hack -- Save the declared cost */
            b_item->cost = borg_price_item(o_ptr, false, 1, store_num);
        }
    }
    mem_free(list);
}

/*
 * helper to find an empty slot
 */
int borg_first_empty_inventory_slot(void)
{
    int i;

    for (i = PACK_SLOTS-1; i > 0; i--)
        if (borg_items[i].iqty)
        {
            if ((i + 1) < PACK_SLOTS)
                return i + 1;
            break;
        }

    return -1;
}

/*
 * Hack -- Cheat the "spell" info
 */
void borg_cheat_spell(int book_num)
{
    struct class_book* book = &player->class->magic.books[book_num];
    for (int spell_num = 0; spell_num < book->num_spells; spell_num++)
    {
        struct class_spell* cspell = &book->spells[spell_num];
        borg_magic* as = &borg_magics[cspell->sidx];

        /* Note "forgotten" spells */
        if (player->spell_flags[cspell->sidx] & PY_SPELL_FORGOTTEN)
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
        else if (!(player->spell_flags[cspell->sidx] & PY_SPELL_LEARNED))
        {
            /* UnKnown */
            as->status = BORG_MAGIC_OKAY;
        }

        /* Note "untried" spells */
        else if (!(player->spell_flags[cspell->sidx] & PY_SPELL_WORKED))
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
}

/*
 * Get the offset in the book this spell is so you can cast it (book) (offset)
 */
static int borg_get_book_offset(int index)
{
    int book = 0, count = 0;
    const struct class_magic* magic = &player->class->magic;

    /* Check index validity */
    if (index < 0 || index >= magic->total_spells)
        return -1;

    /* Find the book, count the spells in previous books */
    while (count + magic->books[book].num_spells - 1 < index)
        count += magic->books[book++].num_spells;

    /* Find the spell */
    return index - count;
}

/*
 * initialize the spell data
*/
static void borg_init_spell(borg_magic* spells, int spell_num)
{
    borg_magic* spell = &spells[spell_num];
    const struct class_spell* cspell = spell_by_index(player, spell_num);
    if (strcmp(cspell->name, borg_spell_ratings[spell_num].name))
    {
        borg_note(format("**STARTUP FAILURE** spell definition missmatch. <%s> not the same as <%s>", cspell->name, borg_spell_ratings[spell_num].name));
        borg_init_failure = true;
        return;
    }
    spell->rating = borg_spell_ratings[spell_num].rating;
    spell->name = borg_spell_ratings[spell_num].name;
    spell->spell_enum = borg_spell_ratings[spell_num].spell_enum;
    spell->level = cspell->slevel;
    spell->book_offset = borg_get_book_offset(cspell->sidx);
    spell->effect_index = cspell->effect->index;
    spell->power = cspell->smana;
    spell->sfail = cspell->sfail;
    spell->status = spell_okay_to_cast(player, spell_num);
    spell->times = 0;
    spell->book = cspell->bidx;
}


/*
 * Prepare a book
 */
static void prepare_book_info(void)
{
    switch (player->class->cidx)
    {
    case CLASS_MAGE:
    borg_spell_ratings = borg_spell_ratings_MAGE;
    break;
    case CLASS_DRUID:
    borg_spell_ratings = borg_spell_ratings_DRUID;
    break;
    case CLASS_PRIEST:
    borg_spell_ratings = borg_spell_ratings_PRIEST;
    break;
    case CLASS_NECROMANCER:
    borg_spell_ratings = borg_spell_ratings_NECROMANCER;
    break;
    case CLASS_PALADIN:
    borg_spell_ratings = borg_spell_ratings_PALADIN;
    break;
    case CLASS_ROGUE:
    borg_spell_ratings = borg_spell_ratings_ROGUE;
    break;
    case CLASS_RANGER:
    borg_spell_ratings = borg_spell_ratings_RANGER;
    break;
    case CLASS_BLACKGUARD:
    borg_spell_ratings = borg_spell_ratings_BLACKGUARD;
    break;
    default:
    borg_spell_ratings = NULL;
    return;
    }

    if (borg_magics)
        mem_free(borg_magics);

    borg_magics = mem_zalloc(player->class->magic.total_spells * sizeof(borg_magic));

    for (int spell = 0; spell < player->class->magic.total_spells; spell++)
    {
        borg_init_spell(borg_magics, spell);
    }
}



/*
 * Hack -- prepare some stuff based on the player race and class
 */
void borg_prepare_race_class_info(void)
{
    /* Initialize the various spell arrays by book */
    prepare_book_info();
}

/*
 * Initialize this file
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

    int16_t what[514];
    char* text[514];

    char buf[256];


    /*** Item/Ware arrays ***/

    /* Make the inventory array */
    borg_items = mem_zalloc(QUIVER_END * sizeof(borg_item));

    /* Make the stores in the town */
    borg_shops = mem_zalloc(9 * sizeof(borg_shop));


    /*** Item/Ware arrays (simulation) ***/

    /* Make the "safe" inventory array */
    safe_items = mem_zalloc(QUIVER_END * sizeof(borg_item));
    safe_home = mem_zalloc(z_info->store_inven_max * sizeof(borg_item));

    /* Make the "safe" stores in the town */
    safe_shops = mem_zalloc(8 * sizeof(borg_shop));

    /*** Plural Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 1; k < z_info->k_max; k++)
    {
        struct object hack;

        /* Get the kind */
        struct object_kind* k_ptr = &k_info[k];

        /* Skip "empty" items */
        if (!k_ptr->name) continue;

        /* Skip "gold" objects */
        if (k_ptr->tval == TV_GOLD) continue;

        /* Skip "artifacts" */
        if (kf_has(k_ptr->kind_flags, KF_INSTA_ART)) continue;

        /* Hack -- make an item */
        object_prep(&hack, &k_info[k], 10, MINIMISE);

        /* Describe a "plural" object */
        hack.number = 2;
        object_desc(buf, sizeof(buf), &hack, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL, player);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;

        size++;

        mem_free(hack.brands);
        mem_free(hack.slays);
        mem_free(hack.curses);
    }

    /* Set the sort hooks */
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort */
    borg_sort(text, what, size);

    borg_sv_plural_text = mem_zalloc(z_info->k_max * sizeof(char*));
    for (i = 0; i < size; i++) borg_sv_plural_text[what[i]] = text[i];


    /* Save the size */
    borg_plural_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    borg_plural_text = mem_zalloc(borg_plural_size * sizeof(char*));
    borg_plural_what = mem_zalloc(borg_plural_size * sizeof(int16_t));

    /* Save the entries */
    for (i = 0; i < size; i++) borg_plural_text[i] = text[i];
    for (i = 0; i < size; i++) borg_plural_what[i] = what[i];


    /*** Singular Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 1; k < z_info->k_max; k++)
    {
        struct object hack;

        /* Get the kind */
        struct object_kind* k_ptr = &k_info[k];

        /* Skip "empty" items */
        if (!k_ptr->name) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval == TV_GOLD) continue;

        /* Skip "artifacts" */
        if (kf_has(k_ptr->kind_flags, KF_INSTA_ART)) continue;

        /* Hack -- make an item */
        object_prep(&hack, &k_info[k], 0, MINIMISE);

        /* Describe a "singular" object */
        hack.number = 1;
        object_desc(buf, sizeof(buf), &hack, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL, player);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;

        mem_free(hack.brands);
        mem_free(hack.slays);
        mem_free(hack.curses);
    }

    /* Analyze the "INSTA_ART" items */
    for (i = 1; i < z_info->a_max; i++)
    {
        struct object hack;

        struct artifact* a_ptr = &a_info[i];

        char* name = (a_ptr->name);

        /* Skip "empty" items */
        if (!a_ptr->name) continue;

        /* Extract the k"ind" */
        k = borg_lookup_kind(a_ptr->tval, a_ptr->sval);

        /* Hack -- make an item */
        object_prep(&hack, &k_info[k], 10, MINIMISE);

        if (!hack.known) {
        	mem_free(hack.brands);
        	mem_free(hack.slays);
        	mem_free(hack.curses);
		continue;
	}

        /* Save the index */
        /* hack.name1 = i; */

        /* Describe a "singular" object */
        hack.number = 1;
        object_desc(buf, sizeof(buf), &hack, ODESC_PREFIX | ODESC_FULL | ODESC_SPOIL, player);

        /* Extract the "suffix" length */
        n = strlen(name) + 1;

        /* Remove the "suffix" */
        buf[strlen(buf) - n] = '\0';

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;

        mem_free(hack.brands);
        mem_free(hack.slays);
        mem_free(hack.curses);
    }

    /* Set the sort hooks */
    borg_sort_comp = borg_sort_comp_hook;
    borg_sort_swap = borg_sort_swap_hook;
    /* Sort */
    borg_sort(text, what, size);


    /* Save the size */
    borg_single_size = size;

    /* Allocate the "item parsing arrays" (plurals) */
    borg_single_text = mem_zalloc(borg_single_size * sizeof(char*));
    borg_single_what = mem_zalloc(borg_single_size * sizeof(int16_t));

    /* Save the entries */
    for (i = 0; i < size; i++) borg_single_text[i] = text[i];
    for (i = 0; i < size; i++) borg_single_what[i] = what[i];


    /*** Artifact and Ego-Item Parsers ***/

    /* No entries yet */
    size = 0;

    /* Collect the "artifact names" */
    for (k = 1; k < z_info->a_max; k++)
    {
        struct artifact* a_ptr = &a_info[k];

        /* Skip non-items */
        if (!a_ptr->name) continue;

        /* Extract a string */
        strnfmt(buf, sizeof(buf), " %s", (a_ptr->name));

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    borg_sv_art_text = mem_zalloc(z_info->a_max * sizeof(char*));
    for (i = 0; i < size; i++) borg_sv_art_text[what[i]] = text[i];

    /* Collect the "ego-item names" */
    for (k = 1; k < z_info->e_max; k++)
    {
        struct ego_item* e_ptr = &e_info[k];

        /* Skip non-items */
        if (!e_ptr->name) continue;

        /* Extract a string */
        strnfmt(buf, sizeof(buf), " %s", (e_ptr->name));

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
    borg_artego_text = mem_zalloc(borg_artego_size * sizeof(char*));
    borg_artego_what = mem_zalloc(borg_artego_size * sizeof(int16_t));

    /* Save the entries */
    for (i = 0; i < size; i++) borg_artego_text[i] = text[i];
    for (i = 0; i < size; i++) borg_artego_what[i] = what[i];
}

/*
 * Release resources allocated by borg_init_3().
 */
void borg_clean_3(void)
{
    int i;

    mem_free(borg_artego_what);
    borg_artego_what = NULL;
    if (borg_artego_text) {
        for (i = 0; i < borg_artego_size; ++i) {
            string_free(borg_artego_text[i]);
        }
        mem_free(borg_artego_text);
        borg_artego_text = NULL;
    }
    borg_artego_size = 0;
    mem_free(borg_sv_art_text);
    borg_sv_art_text = NULL;
    mem_free(borg_single_what);
    borg_single_what = NULL;
    if (borg_single_text) {
        for (i = 0; i < borg_single_size; ++i) {
            string_free(borg_single_text[i]);
        }
        mem_free(borg_single_text);
        borg_single_text = NULL;
    }
    borg_single_size = 0;
    mem_free(borg_plural_what);
    borg_plural_what = NULL;
    if (borg_plural_text) {
        for (i = 0; i < borg_plural_size; ++i) {
            string_free(borg_plural_text[i]);
        }
        mem_free(borg_plural_text);
        borg_plural_text = NULL;
    }
    borg_plural_size = 0;
    mem_free(borg_sv_plural_text);
    borg_sv_plural_text = NULL;
    mem_free(safe_shops);
    safe_shops = NULL;
    mem_free(safe_home);
    safe_home = NULL;
    mem_free(safe_items);
    safe_items = NULL;
    mem_free(borg_shops);
    borg_shops = NULL;
    mem_free(borg_items);
    borg_items = NULL;
}

const char* borg_prt_item(int item)
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
