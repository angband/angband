/* File: borg-ben.c */

/* Purpose: one brain for the Borg (see "borg.c") -BEN- */

#include "angband.h"



#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-map.h"

#include "borg-obj.h"

#include "borg-ext.h"


/*
 * This file implements the "Ben Borg", an "Automatic Angband Player".
 *
 * This file requires the "borg-ext.c", "borg-map.c", "borg-obj.c", and
 * "borg.c" modules, and the "ALLOW_BORG" compilation flag.
 *
 * This code, and the related files, can easily be adapted to create your
 * very own Automatic Angband Player, using your favorite strategies.  Well,
 * maybe not easily, but it can certainly be done.
 *
 * The "borg_ben()" function, which is called when the user hits "^Z", allows
 * the user to interact with the Borg.  The first thing you must do is use
 * the '$' command to "initialize" the Borg.  Assuming you are in wizard mode,
 * this initialization routine grabs the "Term_xtra()" hook of the main "Term",
 * allowing the Borg to notice when the game is asking for keypresses, and to
 * send its own keys to the keypress queue.
 *
 * Once the Borg has been initialized, the user can use the "borg_ben()"
 * function to interact with the borg.  The two most useful commands are
 * probably restart ("z") and resume ("r"), but other commands allow you
 * to ask the Borg questions, or give him instructions.
 *
 * While the Borg is running, it will auto-matically intercept any keys from
 * the user, and will "halt" upon doing so.  Note that this is the only way
 * to halt the Borg, and will produce a "borg_ben()" message.
 *
 * The Borg is only supposed to "know" what is visible on the screen, which
 * it learns by using the "term.c" screen access function "Term_what()", and
 * the cursor location function "Term_locate()", and a hack to access the
 * cursor visibility using "Term_show/hide_cursor()".  It is only allowed to
 * send keypresses to the "Term" like a normal user, by using the standard
 * "Term_keypress()" routine.  It only thinks and sends keys when it catches
 * a request to "Term_xtra(TERM_XTRA_EVENT)", which the "term.c" file uses
 * to wait for a keypress (or other event) from the user.
 *
 * That is, if if were not for the "cheats" currently used (see below).
 *
 *
 * Must be careful of:
 *   Level changing (intentionally or accidentally)
 *   Gold (or objects) embedded in walls (need to tunnel)
 *   Low strength plus embedded gold yields ignored gold
 *   Objects with ',' or '$' symbols (look like monsters)
 *   Mimic monsters (look like objects), and invis monsters
 *   Traps (disarm), Doors (open/bash), Rubble (tunnel)
 *   Stores (enter via movement, exit via escape)
 *   Stores (limited commands, such as no "quaff" command)
 *   Be careful about "destroying" items in quantity
 *   Discard junk before trying to pick up more junk
 *   Be very careful not to run out of food/light
 *   Use "identify" cleverly to obtain knowledge/money
 *   Do not sell junk or cursed/damaged items to any stores
 *   Do not attempt to buy something with insufficient funds
 *   Use the non-optimal stairs if stuck on a level
 *   Use "flow" code for all tasks for consistency
 *   Weird objects ("Mace of Disruption"/"Scythe of Slicing")
 *   Learn spells, and use them when appropriate
 *   Remember that studying prayers is slightly random
 *   Do not try to read scrolls when blind or confused
 *   Do not browse/use/learn spells when blind/confused.
 *   Attempt to heal when "confused", "blind", etc
 *   Attempt to remove fear, if possible, when priest
 *   Occasionally remove both rings (to rearrange them)
 *   Make sure to "wear" items in order of "goodness"
 *   This will allow the "ring removal hack" to work
 *   Notice "failure" when using rod/staff of perceptions
 *   Notice "failure" when attempting spells/prayers
 *   Do not fire at monsters that may have changed location
 *   Search for secret doors if stuck on a level (spastic)
 *   Earthquake (or destruction) of already mapped grids
 *   Collect monsters/objects left behind or seen from afar
 *   Fire missiles at magic mushrooms when afraid
 *   Try not to walk up next to a dangerous monster
 *   Mages should almost never wear gloves
 *   Spell casters need to watch armor weight
 *
 * Should be careful of:
 *   Technically a room can have no exits, requiring digging
 *   Hallucination -- can induce fake "player" symbols
 *   Special screens (including tombstone) with no "walls"
 *   Try to use a shovel/pick to help with tunnelling
 *   Do not "destroy" cursed artifacts (non-warrior types)
 *   If we do destroy a cursed artifact, try and catch it
 *   If wounded, must run away from monsters, then rest
 *   Invisible monsters causing damage while trying to rest
 *   Becoming "afraid" (attacking takes a turn for no effect)
 *   Becoming "blind" (map may be no longer valid)
 *   Running out of light (sort of like being blind)
 *   Be VERY careful not to access illegal locations!
 *   Extremely long object descriptions (clipping inscriptions)
 *   In particular, consider artifact dragon scale mail and such
 *   Attempt to actually walk to the end of all corridors
 *   This will allow the "search" routines to work effectively
 *   Attempt to "track" monsters who flee down corridors
 *   When totally surrounded by monsters, try to escape rooms
 *   Conserve memory space (each grid byte costs about 15K)
 *   Conserve computation time (especially with the flow code)
 *
 * Consider some heuristics:
 *   food is not necessary given "satisfy hunger"
 *   identify is not necessary given the spell
 *   recall is not necessary given the spell
 *
 * Approximate Memory Usage (BORG_GRIDS):
 *   Inventory: 36*120 = 5K
 *   Shop Items: 8*24*120 = 24K
 *
 * Approximate memory usage (without BORG_ROOMS)
 *   All rows of Grids: 70*(200*10) = 140K
 *
 * Approximate memory usage (with BORG_ROOMS)
 *   Rooms: (200*70/8)*24 = 42K
 *   All rows of Grids: 70*(200*6) = 80K
 *
 * Note that there is minimal memory use by static variables compared
 * to the code itself, and compared to allocated memory.
 *
 * Currently, the auto-player "cheats" in a few situations.  Oops.
 *
 * Cheats that are significant, and possibly unavoidable:
 *   Knowledge of when we are being asked for a keypress.
 *   Note that this could be avoided by LONG timeouts/delays
 *
 * Cheats "required" by implementation, but not signifant:
 *   Direct access to the "keypress queue" (sending keys)
 *   Direct access to the cursor visibility (this is silly)
 *   Direct use of the "current screen image" (parsing screen)
 *   Note that this includes distinguishing white/black spaces
 *
 * Cheats that could be "overcome" trivially by ugly code:
 *   Direct modification of the "current options"
 *
 * Cheats that could be avoided by duplicating code:
 *   Direct access to the "r_list" and "k_list" arrays
 *   Direct access to the "v_list" and "ego_name" arrays
 *
 * Cheats that save a lot of "time", but are "optional":
 *   Direct extraction of "panel" info (cheat_panel)
 *   Direct extraction of "inven" info (cheat_inven)
 *   Direct extraction of "equip" info (cheat_equip)
 *   Direct extraction of "spell" info (cheat_spell)
 *
 * Cheats that the Borg would love:
 *   Unique attr/char codes for every monster and object
 *   Ring of See invisible, Ring of Free Action, Helm of Seeing
 *   Use of "x" for mushroom monsters (magic mushrooms!)
 *   Immunity to hallucination, blindness, confusion
 *
 * Simple ways to make the Borg more effective:
 *   Choose a male fighter (maximize strength)
 *   Roll for high strength (this is very important)
 *   Roll for 18 or 18/50 constitution (high hitpoints)
 *   Roll for high dexterity (may yield multiple attacks)
 *   Do not play a human or dunadin (no infravision)
 *   Choose a dwarf (resist blindness)
 *   Choose a high elf (see invisible)
 *   Choose a gnome (free action)
 */



/*
 * Minimum "harmless" food
 */
#define SV_FOOD_MIN_OKAY	SV_FOOD_CURE_POISON


/*
 * Some variables
 */

static int goal_shop = -1;	/* Next shop to visit */
static int goal_item = -1;	/* Next item to buy there */

static int shop_num = 7;	/* Most recent shop index */

static int auto_wearing;	/* Number of "wears" this level */

static int auto_feeling;	/* Number of "turns" to stay here */


static s32b when_recall;	/* When we last did "recall" */

static s32b when_phial;		/* When we last used the Phial */


static bool ignore_hunger;	/* Can ignore hunger */


static int do_add_stat[6];	/* Need to increase the stat */


static int max_level;		/* Maximum level */
static int max_depth;		/* Maximum depth */


static int old_depth = -1;	/* Previous depth */

static int old_chp = -1;	/* Previous hit points */
static int old_csp = -1;	/* Previous spell points */


static bool cheat_equip;	/* Cheat for "equip mode" */

static bool cheat_inven;	/* Cheat for "inven mode" */

static bool cheat_spell;	/* Cheat for "browse mode" */

static bool cheat_panel;	/* Cheat for "panel mode" */


static bool panic_death;	/* Panic before Death */

static bool panic_stuff;	/* Panic before Junking Stuff */



static bool do_inven = TRUE;	/* Acquire "inven" info */
static bool do_equip = TRUE;	/* Acquire "equip" info */

static bool do_spell = TRUE;	/* Acquire "spell" info */

static bool do_panel = TRUE;	/* Acquire "panel" info */

static byte do_spell_aux = 0;	/* Hack -- current book for "do_spell" */



/*
 * Some kinds -- required items
 */
static int kind_food_ration = 21;
static int kind_potion_serious = 240;
static int kind_potion_critical = 241;
static int kind_scroll_recall = 220;
static int kind_scroll_teleport = 186;
static int kind_scroll_identify = 176;
static int kind_flask = 348;
static int kind_torch = 346;
static int kind_lantern = 347;

/*
 * Some kinds -- increase stat potions
 */
static int kind_potion_add_str = 225;
static int kind_potion_add_int = 228;
static int kind_potion_add_wis = 231;
static int kind_potion_add_dex = 251;
static int kind_potion_add_con = 243;
static int kind_potion_add_chr = 234;

/*
 * Some kinds -- restore stat potions
 */
static int kind_potion_fix_str = 227;
static int kind_potion_fix_int = 230;
static int kind_potion_fix_wis = 233;
static int kind_potion_fix_dex = 252;
static int kind_potion_fix_con = 253;
static int kind_potion_fix_chr = 236;

/*
 * Some kinds -- miscellaneous
 */
static int kind_potion_fix_exp = 260;

/*
 * Some kinds -- various enchantment scrolls
 */
static int kind_enchant_to_hit = 173;
static int kind_enchant_to_dam = 174;
static int kind_enchant_to_ac = 175;



/*
 * Hack -- Actual number of each "item kind".
 */
static byte kind_have[MAX_K_IDX];

/*
 * Hack -- Desired number of each "item kind".
 */
static byte kind_need[MAX_K_IDX];





/*
 * Hack -- single character constants
 */
static const char /* p1 = '(', */ p2 = ')';



/*
 * Hack -- make sure we have a good "ANSI" definition for "CTRL()"
 */
#undef CTRL
#define CTRL(C) ((C)&037)








/*
 * Estimate the "power" of an item.
 *
 * Make a heuristic guess at the "power" of an item in "gold"
 *
 * We assume that the given item is "aware" and "known" (or "average").
 * We also assume that the given item is "acceptable" (not worthless).
 *
 * We used to attempt to factor in the effects of "enchanting" the item,
 * but this was a total hack.  A much better method would be to keep a
 * "secondary" weapon somewhere, probably in the pack, and enchant it
 * when possible until we actually have a better item to use.
 *
 * Also, the calling function may wish to factor in the "cost" of
 * the item, to prevent "expensive" upgrades in the black market.
 *
 * Note -- This is used both for "wearing" items and for "buying" them.
 *
 * But the "buying" part is only active for things chosen as necessary
 * by the "borg_good_buy()" function.
 */
static s32b item_power(auto_item *item)
{
    s32b value;

    
    /* Mega-Hack -- Never wear unknowns */
    if (!item->kind) return (0L);


    /* Armour */
    if (borg_item_is_armour(item)) {

        /* Hack -- Base value */
        value = borg_item_value(item);

        /* Reward the total armor */
        value += (item->ac + item->to_a) * 100L;

        /* Hack -- low level spell casters hate heavy armor */
        if ((mb_ptr->spell_book) && (auto_lev < 20)) {

            /* Mega-Hack -- Reward "armor vs weight", depending on level */
            value += (10L * item->ac - item->weight) * (20 - auto_lev);
        }
        
        /* Mages hate (most) gloves */
        if ((mb_ptr->spell_book == TV_MAGIC_BOOK) &&
            (item->tval == TV_GLOVES) &&
            (item->name2 != EGO_FREE_ACTION) &&
            (item->name2 != EGO_AGILITY) &&
            (!(v_list[item->name1].flags2 & TR2_FREE_ACT))) {

            /* Worthless */
            value = 0L;
        }
    }

    /* Weapons */
    else if ((item->tval == TV_HAFTED) ||
             (item->tval == TV_SWORD) ||
             (item->tval == TV_POLEARM) ||
             (item->tval == TV_DIGGING)) {

        int blows = 1;
        
        /* Reward blows (unless fucked) */
        if (auto_wearing < 100) blows = borg_blows(item);
        
        /* Hack -- Base value */
        value = borg_item_value(item);

        /* Favor high "max damage" */
        value += (blows * ((item->dd * item->ds) * 200L));

        /* Favor the bonuses */
        value += (item->to_h * 10L);
        value += (item->to_d * 100L);

        /* Priest weapon penalty for non-blessed edged weapons */
        /* Note that edged weapons add a 25% prayer penalty! */
        if ((auto_class == 2) &&
            ((item->tval == TV_SWORD) || (item->tval == TV_POLEARM)) &&
            (!(v_list[item->name1].flags3 & TR3_BLESSED))) {

            /* Icky */
            value = 0L;
        }

        /* Too heavy */
        if (adj_str_hold[borg_stat_index(A_STR)] < item->weight) value = 0L;
    }

    /* Missile Launchers */
    else if (item->tval == TV_BOW) {

        /* Base value */
        value = borg_item_value(item);

        /* Mega-Hack -- Check the sval */
        value += (item->sval * 500L);

        /* Check the bonuses */
        value += (item->to_h * 10L);
        value += (item->to_d * 100L);

        /* Too heavy */
        if (adj_str_hold[borg_stat_index(A_STR)] < item->weight) value = 0L;
    }

    /* Rings/Amulets */
    else if ((item->tval == TV_AMULET) || 
             (item->tval == TV_RING)) {

        /* Avoid negative bonuses */
        if (item->to_h < 0) return (0L);
        if (item->to_d < 0) return (0L);
        if (item->to_a < 0) return (0L);
        if (item->pval < 0) return (0L);

        /* Base value */
        value = borg_item_value(item);
    }


    /* Flasks */
    else if (item->tval == TV_FLASK) {

        /* Base value */
        value = borg_item_value(item);

        /* Buy flasks for lanterns */
        if (kind_have[item->kind] < 15) value += 5004000L;
    }

    /* Lite */
    else if (item->tval == TV_LITE) {

        /* Base value */
        value = borg_item_value(item);

        /* Analyze the lite */
        switch (item->sval) {

            case SV_LITE_GALADRIEL:
                value += 5009000L;
                break;
                
            case SV_LITE_LANTERN:            
                value += 5004000L;
                break;

            case SV_LITE_TORCH:            
                if (kind_have[item->kind] < 15) value += 5002000L;
                break;
        }
    }

    /* Food */
    else if (item->tval == TV_FOOD) {

        /* Base value */
        value = borg_item_value(item);

        /* Analyze the food */
        switch (item->sval) {

            case SV_FOOD_RATION:

                if (kind_have[item->kind] < 10) value += 5005000L;

                break;
        }
    }


    /* Scrolls */
    else if (item->tval == TV_SCROLL) {

        /* Base value */
        value = borg_item_value(item);

        /* Analyze the scroll */
        switch (item->sval) {

            case SV_SCROLL_PHASE_DOOR:
                if (kind_have[item->kind] < 5) value += 500L;
                break;
                
            case SV_SCROLL_TELEPORT:
                if (kind_have[item->kind] < 5) value += 3000L;
                break;
                
            case SV_SCROLL_IDENTIFY:
                if (kind_have[item->kind] < 15) value += 6000L;
                break;
                
            case SV_SCROLL_WORD_OF_RECALL:
                if (kind_have[item->kind] < 3) value += 9000L;
                break;
        }
    }

    /* Potions */
    else if (item->tval == TV_POTION) {

        /* Base value */
        value = borg_item_value(item);

        /* Analyze the potion */
        switch (item->sval) {
        
            case SV_POTION_RESTORE_EXP:
                if (do_fix_exp) value += 500000L;
                break;
                
            case SV_POTION_RES_STR:
                if (do_fix_stat[A_STR]) value += 10000L;
                break;
                
            case SV_POTION_RES_INT:
                if (do_fix_stat[A_INT]) value += 10000L;
                break;
                
            case SV_POTION_RES_WIS:
                if (do_fix_stat[A_WIS]) value += 10000L;
                break;
                
            case SV_POTION_RES_DEX:
                if (do_fix_stat[A_DEX]) value += 10000L;
                break;
                
            case SV_POTION_RES_CON:
                if (do_fix_stat[A_CON]) value += 10000L;
                break;
                
            case SV_POTION_RES_CHR:
                if (do_fix_stat[A_CHR]) value += 10000L;
                break;
            
            case SV_POTION_INC_STR:
                if (auto_stat[A_STR] < 18+50) value += 200000L;
                break;
                
            case SV_POTION_INC_INT:
                if (auto_stat[A_INT] < 18+50) value += 200000L;
                break;
                
            case SV_POTION_INC_WIS:
                if (auto_stat[A_WIS] < 18+50) value += 200000L;
                break;
                
            case SV_POTION_INC_DEX:
                if (auto_stat[A_DEX] < 18+50) value += 200000L;
                break;
                
            case SV_POTION_INC_CON:
                if (auto_stat[A_CON] < 18+50) value += 200000L;
                break;
                
            case SV_POTION_INC_CHR:
                if (auto_stat[A_CHR] < 18+50) value += 200000L;
                break;
                
            case SV_POTION_CURE_SERIOUS:
                if (kind_have[item->kind] < 5) value += 500L;
                break;
                
            case SV_POTION_CURE_CRITICAL:
                if (kind_have[item->kind] < 5) value += 5000L;
                break;
        }
    }

    /* Readible books */
    else if (item->tval == mb_ptr->spell_book) {

        /* Base value */
        value = borg_item_value(item);

        /* Large bonus for useful books */
        value += 50000L;
    }

    /* Others */
    else {
    
        /* Base value */
        value = borg_item_value(item);
    }


    /* Return the value */
    return (value);
}




/*
 * Determine how much it is "worth" to bring an object back to town
 * This function analyzes a single instance of the given item.
 *
 * Note that we make heavy use of the "borg_item_value()" routine, which
 * is pretty accurate, but is misleading for unaware or unidentified items,
 * and for items with "convenient" side effects, such as missiles or staffs
 * of perceptions, and for items which can easily "stack" with other items.
 *
 * This routine is used to determine how much gold the Borg will get for
 * holding onto an item until he can sell it in a shop.  This is NOT used
 * to determine which item is best to buy or wield/wear, see "item_power()".
 *
 * This function currently assumes that "borg_item_value()" is "correct",
 * which is a large assumption, since certain items have "hidden" cost
 * components, such as weapons of slay undead which also hold life.
 *
 * Hack -- we add "heuristic" bonuses to various items which have
 * "implicit" value to the Borg.  This is cute but a little dangerous.
 */
static s32b item_worth(auto_item *item)
{
    s32b value;


    /* Extract the base value */
    value = borg_item_value(item);

    /* Worthless item */
    if (value <= 0L) return (0L);


    /* Mega-Hack -- fake value for non-aware items */
    if (!item->kind) return (auto_depth * 100 + 50 + value);


    /* Analyze the item */
    switch (item->tval) {
    
        case TV_STAFF:
        
            switch (item->sval) {
            
                case SV_STAFF_IDENTIFY:
                case SV_STAFF_TELEPORTATION:
                    value += item->pval * 200L;
                    break;
            }
    }
    

    /* Return the value */
    return (value);
}



/*
 * Determine if an item can be sold in the current store
 */
static bool borg_good_sell(auto_item *item)
{
    int tval = item->tval;


    /* Hack -- artifacts */
    if (item->name1) {

        /* Save them in the home */
        if (shop_num == 7) return (TRUE);

        /* Never sell them */
        return (FALSE);
    }


    /* Switch on the store */
    switch (shop_num + 1) {

      /* General Store */
      case 1:

        /* Analyze the type */
        switch (tval) {
          case TV_DIGGING:
          case TV_CLOAK:
          case TV_FOOD:
          case TV_FLASK:
          case TV_LITE:
          case TV_SPIKE:
            return (TRUE);
        }
        break;

      /* Armoury */
      case 2:

        /* Analyze the type */
        switch (tval) {
          case TV_BOOTS:
          case TV_GLOVES:
          case TV_HELM:
          case TV_CROWN:
          case TV_SHIELD:
          case TV_SOFT_ARMOR:
          case TV_HARD_ARMOR:
          case TV_DRAG_ARMOR:
            return (TRUE);
        }
        break;

      /* Weapon Shop */
      case 3:

        /* Analyze the type */
        switch (tval) {
          case TV_SHOT:
          case TV_BOLT:
          case TV_ARROW:
          case TV_BOW:
          case TV_HAFTED:
          case TV_POLEARM:
          case TV_SWORD:
            return (TRUE);
        }
        break;

      /* Temple */
      case 4:

        /* Analyze the type */
        switch (tval) {
          case TV_HAFTED:
          case TV_SCROLL:
          case TV_POTION:
          case TV_PRAYER_BOOK:
            return (TRUE);
        }
        break;

      /* Alchemist */
      case 5:

        /* Analyze the type */
        switch (tval) {
          case TV_SCROLL:
          case TV_POTION:
            return (TRUE);
        }
        break;

      /* Magic Shop */
      case 6:

        /* Analyze the type */
        switch (tval) {
          case TV_MAGIC_BOOK:
          case TV_AMULET:
          case TV_RING:
          case TV_STAFF:
          case TV_WAND:
          case TV_ROD:
            return (TRUE);
        }
        break;
    }

    /* Assume not */
    return (FALSE);
}




/*
 * Determine if the Borg is running out of crucial supplies.
 * This routine is used to invoke word of recall (if possible)
 */
static bool borg_restock()
{
    auto_item *item = &auto_items[INVEN_LITE];


    /* Totally out of light */
    if (item->iqty == 0) return (TRUE);

    /* Running out of flasks for lanterns */
    if ((item->kind == kind_lantern) && (kind_have[kind_flask] < 5)) return (TRUE);

    /* Running out of torches */
    if ((item->kind == kind_torch) && (kind_have[kind_torch] < 5)) return (TRUE);


    /* Running out of food */
    if (kind_have[kind_food_ration] < (ignore_hunger ? 0 : 5)) return (TRUE);


    /* Assume happy */
    return (FALSE);
}


/*
 * Determine if an item should be bought
 */
static bool borg_good_buy(auto_item *ware)
{
    int slot;

    auto_item *worn = NULL;


    /* Never buy "weird" stuff */
    if (!ware->kind) return (FALSE);


    /* Determine where the item would be worn */
    slot = borg_wield_slot(ware);

    /* Extract the item currently in that slot */
    if (slot >= 0) worn = &auto_items[slot];


    /* Use the "shopping list" */
    if (kind_need[ware->kind]) {

        /* Check the pile */
        if (kind_have[ware->kind] < kind_need[ware->kind]) {
            return (TRUE);
        }
    }
    

    /* Process Lites */
    if (ware->tval == TV_LITE) {
    
        /* Never buy from the black market */
        if (shop_num == 6) return (FALSE);

        /* Hack -- Artifact lites are the best */
        if (worn->name1) return (FALSE);

        /* Torches */
        if (ware->sval == SV_LITE_TORCH) {

            /* Hack -- Torches are defeated by (fueled) lanterns */
            if (worn->kind == kind_lantern) {
                if (kind_have[kind_flask] >= 10) return (FALSE);
            }

            /* Always have at least 20 torches */
            if (kind_have[kind_torch] < 20) return (TRUE);
        }

        /* Lanterns */
        else if (ware->sval == SV_LITE_LANTERN) {

            /* Never buy a lantern when wielding one already */
            if (worn->kind == kind_lantern) return (FALSE);

            /* Always buy at least 1 lantern */
            if (kind_have[kind_lantern] < 1) return (TRUE);
        }
    }


    /* Missiles */
    else if ((ware->tval == TV_SHOT) ||
             (ware->tval == TV_ARROW) ||
             (ware->tval == TV_BOLT)) {

        /* Never use the black market */
        if (shop_num == 6) return (FALSE);

        /* Never buy expensive missiles */
        if (ware->cost > 50L) return (FALSE);

        /* Never buy incorrect missiles */
        if (ware->tval != auto_tval_ammo) return (FALSE);

        /* Never buy too many missiles */
        if (kind_have[ware->kind] >= 30) return (FALSE);
        
        /* Buy missiles */
        return (TRUE);
    }


    /* Rings, Amulets, Weapons, Bows, Armor */
    else if (slot >= 0) {

        s32b new = item_power(ware);
        s32b old = item_power(worn);
        
        /* Always penalize shopping in the black market */
        if (shop_num == 6) new -= ware->cost;

        /* Penalize the cost of the new item */
        if (ware->cost > auto_gold / 10) new -= ware->cost;

        /* Buy better equipment */
        if (new > old) return (TRUE);
    }


    /* Assume useless */
    return (FALSE);
}


/*
 * Hack -- determine what to do with an item
 *
 * Note that marking an item for sale may result in that item
 * being selected for destruction if the pack is full.
 *
 * This, if the pack is full, and an item is "necessary", we
 * will not mark that item for sale.
 */
static void borg_notice_aux(auto_item *item)
{
    int slot;

    auto_item *worn = NULL;


    /* Clear the flags */
    item->wear = item->cash = item->junk = item->test = FALSE;


    /* Throw away "junk" and "spikes" */
    if ((item_worth(item) <= 0) ||
        (item->tval == TV_SPIKE)) {

        /* This is junk */
        item->junk = TRUE;

        /* Do not sell this */
        item->cash = FALSE;

        /* All done */
        return;
    }


    /* Hack -- Assume we will sell it */
    item->cash = TRUE;


    /* Check the "shopping list" */
    if (kind_need[item->kind]) {

        /* Hack -- Do not sell items we "need" */
        if (kind_have[item->kind] <= kind_need[item->kind]) {
            item->cash = FALSE;
        }
    }
    

    /* Process scrolls */
    if (item->tval == TV_SCROLL) {

        /* Hack -- Identify "interesting" items */
        if (!item->kind && (auto_depth > 5)) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Process potions */
    else if (item->tval == TV_POTION) {

        /* Hack -- Identify "interesting" items */
        if (!item->kind && (auto_depth > 5)) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Process rods */
    else if (item->tval == TV_ROD) {

        /* Always identify rods */
        if (!item->kind) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Process wands/staffs */
    else if ((item->tval == TV_WAND) || (item->tval == TV_STAFF)) {

        /* Identify type and charges */
        if (!item->kind || !item->able) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Mega-Hack -- Junk chests */
    else if (item->tval == TV_CHEST) {

        /* Hack -- Throw it away */
        item->junk = TRUE;
        item->cash = FALSE;
    }


    /* Keep some food */
    else if (item->tval == TV_FOOD) {

        /* Hack -- Identify "interesting" items */
        if (!item->kind) {
            item->test = TRUE;
            item->cash = FALSE;
        }
    }


    /* Keep flasks (unless unnecessary) */
    else if (item->tval == TV_FLASK) {

        /* Hack -- assume we will keep it */
        item->cash = FALSE;

        /* Hack -- Artifact lites need no fuel */
        if (auto_items[INVEN_LITE].name1) item->cash = TRUE;
    }


    /* Hack -- skip "non-wearables" */
    if (item->tval < TV_MIN_WEAR) return;
    if (item->tval > TV_MAX_WEAR) return;


    /* See what slot that item could go in */
    slot = borg_wield_slot(item);

    /* Extract the item currently in that slot */
    if (slot >= 0) worn = &auto_items[slot];


    /* Note -- Assume we will not sell it */
    item->cash = FALSE;


    /* Process rings */
    if (item->tval == TV_RING) {

        /* Hack -- identify items */
        if (!item->kind || !item->able) {
            item->test = TRUE;
        }

        /* Wear most powerful items */
        else if (item_power(item) > item_power(worn)) {
            item->wear = TRUE;
        }

        /* Sell everything else */
        else {
            item->cash = TRUE;
        }

        /* All done */
        return;
    }


    /* Process amulets */
    if (item->tval == TV_AMULET) {

        /* Hack -- identify items */
        if (!item->kind || !item->able) {
            item->test = TRUE;
        }

        /* Wear most powerful items */
        else if (item_power(item) > item_power(worn)) {
            item->wear = TRUE;
        }

        /* Sell everything else */
        else {
            item->cash = TRUE;
        }

        /* All done */
        return;
    }


    /* Process Lite's */
    if (item->tval == TV_LITE) {

        /* Hack -- identify lites */
        if (!item->kind || !item->able) {
            item->test = TRUE;
        }

        /* Hack -- Replace missing lites */
        else if (!worn->iqty) {
            item->wear = TRUE;
        }

        /* Hack -- Sell "empty" Lites */
        else if (!item->pval) {
            item->cash = TRUE;
        }

        /* Hack -- replace empty lites */
        else if (!worn->pval) {
            item->wear = TRUE;
        }

        /* Hack -- Heuristic test */
        else if (item_power(item) > item_power(worn)) {
            item->wear = TRUE;
        }

        /* Hack -- Keep a single artifact lite */
        else if (worn->name1) {
            item->cash = TRUE;
        }

        /* Notice "junky" torches */
        else if (item->kind == kind_torch) {

            /* Lantern plus fuel pre-empts torches */
            if (worn->kind == kind_lantern) {
                if (kind_have[kind_flask] > 10) {
                    item->cash = TRUE;
                }
            }

            /* Sell "excess" torches */
            else if (kind_have[kind_torch] > 30) {
                item->cash = TRUE;
            }
        }

        /* Notice "extra" lanterns */
        else if (item->kind == kind_lantern) {

            /* Already wielding a lantern */
            if (worn->kind == kind_lantern) {
                item->cash = TRUE;
            }

            /* Already carrying a lantern */
            else if (kind_have[kind_lantern] > 1) {
                item->cash = TRUE;
            }
        }

        /* All done */
        return;
    }


    /* Identify "good" items */
    if (!item->able &&
        ((streq(item->note, "{good}")) ||
         (streq(item->note, "{excellent}")) ||
         (streq(item->note, "{terrible}")) ||
         (streq(item->note, "{special}")))) {

        /* Test this */
        item->test = TRUE;

        /* All done */
        return;
    }


    /* Analyze missiles */
    if ((item->tval == TV_SHOT) ||
        (item->tval == TV_ARROW) ||
        (item->tval == TV_BOLT)) {

        /* Sell invalid missiles */
        if (item->tval != auto_tval_ammo) {
            item->cash = TRUE;
        }

        /* Hack -- Sell single unidentified missiles */
        else if ((!item->able) && (item->iqty == 1)) {
            item->cash = TRUE;
        }
        
        /* Hack -- Sell unidentified missiles in town */
        else if ((!item->able) && !auto_depth) {
            item->cash = TRUE;
        }
        
        /* Hack -- Sell large piles */
        else if (item->iqty > 50) {
            item->cash = TRUE;
        }
        
        /* All done */
        return;
    }


    /* Examine non-identified things */
    if (!item->able && !streq(item->note, "{average}")) {

        /* Mega-Hack -- Mage/Ranger -- identify stuff */
        if ((auto_class == 1) || (auto_class == 4)) {

            /* Test it eventually */
            if (rand_int(1000) < auto_lev) item->test = TRUE;
        }
        
        /* Mega-Hack -- Priest -- identify stuff */
        if (auto_class == 2) {

            /* Test it eventually */
            if (rand_int(5000) < auto_lev) item->test = TRUE;
        }
        
        /* All done */
        return;
    }

    
    /* Wear the "best" equipment (unless worthless) */
    if (item_power(item) > item_power(worn)) {

        /* Wear it */
        item->wear = TRUE;
    }

    /* Sell the rest */
    else {

        /* Sell it */
        item->cash = TRUE;
    }
}



/*
 * Examine the inventory
 */
static void borg_notice(void)
{
    int i, n;


    /* Assume we have nothing */
    C_WIPE(&kind_have, MAX_K_IDX, byte);
    
    /* Assume we need nothing */
    C_WIPE(&kind_need, MAX_K_IDX, byte);


    /* Count "amount" of each item kind in pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Obtain quantity */
        n = kind_have[item->kind] + item->iqty;
        
        /* Maintain count (max out at 255) */
        kind_have[item->kind] = ((n < 255) ? n : 255);
    }
        

    /* Assume we need food */
    ignore_hunger = FALSE;
    
    /* Mega-Hack -- no need for food */
    if (borg_spell_okay(2,0) ||
        borg_prayer_okay(1,5)) {
        ignore_hunger = TRUE;
    }


    /* Determine which stats need "boosting" */
    for (i = 0; i < 6; i++) {

        /* Assume maximized */
        do_add_stat[i] = FALSE;

        /* Probably not maximized if below 18/100 */
        if (auto_stat[i] < 18+100) do_add_stat[i] = TRUE;
        
        /* Probably not maximized if not smoothed */
        if ((auto_stat[i] - 18) % 10) do_add_stat[i] = TRUE;
    }
    
    
    /* Collect basic spell books */
    for (i = 0; i < 4; i++) {

        /* Collect two of every spell-book */
        if (kind_book[i]) kind_need[kind_book[i]] = 2;
    }
    
    /* Collect extra spell books */
    for (i = 4; i < 9; i++) {

        /* Collect two of every spell-book */
        if (kind_book[i]) kind_need[kind_book[i]] = 1;
    }
    
    
    /* Choose a missile type */
    if (TRUE) {
    
        /* Access the bow (if any) */
        auto_item *worn = &auto_items[INVEN_BOW];

        /* Default ammo */
        auto_tval_ammo = 0;
        
        /* Proper ammo */
        if (worn->sval == SV_SLING) auto_tval_ammo = TV_SHOT;
        if (worn->sval == SV_SHORT_BOW) auto_tval_ammo = TV_ARROW;
        if (worn->sval == SV_LONG_BOW)  auto_tval_ammo = TV_ARROW;
        if (worn->sval == SV_LIGHT_XBOW) auto_tval_ammo = TV_BOLT;
        if (worn->sval == SV_HEAVY_XBOW) auto_tval_ammo = TV_BOLT;
    }


    /* Collect food */
    kind_need[kind_food_ration] = (ignore_hunger ? 0 : 30);


    /* Collect Flasks for lanterns */
    if (auto_items[INVEN_LITE].kind == kind_lantern) {
        kind_need[kind_flask] = 30;
    }


    /* Collect scrolls of identify */
    kind_need[kind_scroll_identify] = 40;

    /* Collect scrolls of teleport */
    kind_need[kind_scroll_teleport] = 10;

    /* Collect scrolls of recall */
    kind_need[kind_scroll_recall] = 5;


    /* Collect potions of cure critical wounds */
    kind_need[kind_potion_critical] = 20;

    /* Collect potions of cure serious wounds */
    kind_need[kind_potion_serious] = 10;


    /* Increase Stats if Possible */
    if (do_add_stat[A_STR]) {
        kind_need[kind_potion_add_str] = 1;
    }

    /* Increase INT if possible */
    if (do_add_stat[A_INT]) {
        kind_need[kind_potion_add_int] = 1;
    }

    /* Increase WIS if possible */
    if (do_add_stat[A_WIS]) {
        kind_need[kind_potion_add_wis] = 1;
    }

    /* Increase DEX if possible */
    if (do_add_stat[A_DEX]) {
        kind_need[kind_potion_add_dex] = 1;
    }

    /* Increase CON if possible */
    if (do_add_stat[A_CON]) {
        kind_need[kind_potion_add_con] = 1;
    }

    /* Increase CHR if possible */
    if (do_add_stat[A_CHR]) {
        kind_need[kind_potion_add_chr] = 1;
    }


    /* Restore Stats if needed */
    if (do_fix_stat[A_STR]) kind_need[kind_potion_fix_str] = 1;
    if (do_fix_stat[A_INT]) kind_need[kind_potion_fix_int] = 1;
    if (do_fix_stat[A_WIS]) kind_need[kind_potion_fix_wis] = 1;
    if (do_fix_stat[A_DEX]) kind_need[kind_potion_fix_dex] = 1;
    if (do_fix_stat[A_CON]) kind_need[kind_potion_fix_con] = 1;
    if (do_fix_stat[A_CHR]) kind_need[kind_potion_fix_chr] = 1;


    /* Restore experience if needed */
    if (do_fix_exp) kind_need[kind_potion_fix_exp] = 1;


    /* Hack -- enchant all the equipment */
    for (i = INVEN_WIELD; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Skip "unknown" items */
        if (!item->able) continue;
        
        /* Enchant all armour */
        if (borg_item_is_armour(item) && (item->to_a < 8)) {
            kind_need[kind_enchant_to_ac] += (8 - item->to_a);
        }

        /* Enchant all weapons (to damage) */
        if (borg_item_is_weapon(item) && (item->to_d < 8)) {
            kind_need[kind_enchant_to_dam] += (8 - item->to_d);
        }

        /* Enchant all weapons (to hit) */
        if (borg_item_is_weapon(item) && (item->to_h < 8)) {
            kind_need[kind_enchant_to_hit] += (8 - item->to_h);
        }
    }


    /* Analyze the inventory */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Decide what to do with it */
        borg_notice_aux(item);
    }
}


















/*
 * Read a scroll of recall if not in town and allowed
 */
static bool borg_recall(void)
{
    /* Attempt to only read recall once per level */
    if (when_recall + 500 >= c_t) return (FALSE);

    /* Try to read a scroll of recall */
    if (borg_read_scroll(SV_SCROLL_WORD_OF_RECALL)) {

        /* Remember when */
        when_recall = c_t;

        /* Success */
        return (TRUE);
    }

    /* Nothing */
    return (FALSE);
}



/*
 * We are starving -- attempt to eat anything edible
 */
static bool borg_eat_food_any(void)
{
    int i;

    /* Scan the inventory for "normal" food */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD) continue;

        /* Skip "flavored" food */
        if (item->sval < SV_FOOD_MIN_FOOD) continue;

        /* Eat something of that type */
        if (borg_action('E', item->kind)) return (TRUE);
    }

    /* Scan the inventory for "okay" food */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip unknown food */
        if (!item->kind) continue;

        /* Skip non-food */
        if (item->tval != TV_FOOD) continue;

        /* Skip "icky" food */
        if (item->sval < SV_FOOD_MIN_OKAY) continue;
        
        /* Eat something of that type */
        if (borg_action('E', item->kind)) return (TRUE);
    }

    /* Nothing */
    return (FALSE);
}





/*
 * Use things in a useful manner
 * Attempt healing, refueling, eating, etc.
 */
static bool borg_use_things(void)
{
    auto_item *item = &auto_items[INVEN_LITE];


    /* Panic -- avoid possible death */
    if (panic_death &&
       (auto_chp < 100) &&
       (auto_chp < auto_mhp / 4)) {

        borg_oops("Almost dead.");
        return (TRUE);
    }


    /* Hack -- attempt to escape */
    if ((auto_chp < auto_mhp / 2) && (rand_int(4) == 0)) {
        if ((kind_have[kind_potion_critical] < 5) &&
            auto_depth && borg_recall()) return (TRUE);
    }


    /* Hack -- heal when wounded */
    if ((auto_chp < auto_mhp / 2) && (rand_int(4) == 0)) {
        if (borg_quaff_potion(SV_POTION_CURE_CRITICAL) ||
            borg_quaff_potion(SV_POTION_CURE_SERIOUS)) {
            return (TRUE);
        }
    }

    /* Hack -- heal when blind/confused */
    if ((do_blind || do_confused) && (rand_int(100) < 25)) {
        if (borg_quaff_potion(SV_POTION_CURE_SERIOUS) ||
            borg_quaff_potion(SV_POTION_CURE_CRITICAL)) {
            return (TRUE);
        }
    }

    /* Hack -- cure fear when afraid */
    if (do_afraid && (rand_int(100) < 25)) {
        if (borg_prayer(0,3) ||
            borg_quaff_potion(SV_POTION_BOLDNESS)) {
            return (TRUE);
        }
    }    

    /* Eat or abort */
    if (do_weak) {

        /* Attempt to satisfy hunger */
        if (borg_action('E', kind_food_ration) ||
            borg_eat_food_any() ||
            borg_spell(2,0) ||
            borg_prayer(1,5)) {

            return (TRUE);
        }

        /* Hack -- Do NOT starve to death */
        if (panic_death) {

            borg_oops("Starving.");
            return (TRUE);
        }
        
        /* Mega-Hack -- Flee to town for food */
        if (auto_depth && borg_recall()) return (TRUE);
    }


    /* Refuel current torch */
    if ((item->kind == kind_torch) && (item->pval < 1000)) {

        /* Try to refuel the torch */
        if (borg_action('F', kind_torch)) return (TRUE);
    }


    /* Refuel current lantern */
    if ((item->kind == kind_lantern) && (item->pval < 5000)) {

        /* Try to refill the lantern */
        if (borg_action('F', kind_flask)) return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}


/*
 * Use things in a useful, but non-essential, manner
 */
static bool borg_use_others(void)
{
    int i;


    /* Quaff experience restoration potion */
    if (do_fix_exp && borg_quaff_potion(SV_POTION_RESTORE_EXP)) return (TRUE);


    /* Use some items right away */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Process "force" items */
        switch (item->tval) {

            case TV_POTION:

                /* Check the scroll */
                switch (item->sval) {
                
                    case SV_POTION_ENLIGHTENMENT:

                        /* Never quaff these in town */
                        if (!auto_depth) break;
                        
                    case SV_POTION_INC_STR:
                    case SV_POTION_INC_INT:
                    case SV_POTION_INC_WIS:
                    case SV_POTION_INC_DEX:
                    case SV_POTION_INC_CON:
                    case SV_POTION_INC_CHR:
                    case SV_POTION_AUGMENTATION:
                    case SV_POTION_EXPERIENCE:

                        /* Try quaffing the potion */
                        if (borg_quaff_potion(item->sval)) return (TRUE);
                }
                
                break;

            case TV_SCROLL:
            
                /* Hack -- check Blind/Confused */
                if (do_blind || do_confused) break;

                /* Check the scroll */
                switch (item->sval) {
                
                    case SV_SCROLL_MAPPING:
                    case SV_SCROLL_DETECT_TRAP:
                    case SV_SCROLL_DETECT_DOOR:
                    case SV_SCROLL_ACQUIREMENT:
                    case SV_SCROLL_STAR_ACQUIREMENT:
                    case SV_SCROLL_PROTECTION_FROM_EVIL:

                        /* Never read these in town */
                        if (!auto_depth) break;
                        
                        /* Try reading the scroll */
                        if (borg_read_scroll(item->sval)) return (TRUE);
                }

                break;
        }
    }


    /* Quaff stat restoration potions */
    if (do_fix_stat[A_STR] && borg_quaff_potion(SV_POTION_RES_STR)) return (TRUE);
    if (do_fix_stat[A_INT] && borg_quaff_potion(SV_POTION_RES_INT)) return (TRUE);
    if (do_fix_stat[A_WIS] && borg_quaff_potion(SV_POTION_RES_WIS)) return (TRUE);
    if (do_fix_stat[A_DEX] && borg_quaff_potion(SV_POTION_RES_DEX)) return (TRUE);
    if (do_fix_stat[A_CON] && borg_quaff_potion(SV_POTION_RES_CON)) return (TRUE);
    if (do_fix_stat[A_CHR] && borg_quaff_potion(SV_POTION_RES_CHR)) return (TRUE);


    /* Eat food */
    if (do_hungry) {

        /* Attempt to satisfy hunger */
        if (borg_spell(2,0) ||
            borg_prayer(1,5) ||
            borg_action('E', kind_food_ration)) {
            
            return (TRUE);
        }
    }
    

    /* Study spells/prayers */
    if (borg_study_any_okay()) {

        /* Mages */
        if (mb_ptr->spell_book == TV_MAGIC_BOOK) {
        
            /* Hack -- try for magic missile */
            if (borg_study_spell(0,0)) return (TRUE);
        
            /* Hack -- try for teleport */
            if (borg_study_spell(1,5)) return (TRUE);

            /* Hack -- try for satisfy hunger */
            if (borg_study_spell(2,0)) return (TRUE);

            /* Hack -- try for identify */
            if (borg_study_spell(2,4)) return (TRUE);
        
            /* Hack -- try for phase door */
            if (borg_study_spell(0,2)) return (TRUE);

            /* Hack -- try for call light */
            if (borg_study_spell(0,3)) return (TRUE);
        }
        
        /* Priests */
        if (mb_ptr->spell_book == TV_PRAYER_BOOK) {

            /* Hack -- try for perceptions */
            if (borg_study_prayer(5,2)) return (TRUE);

            /* Hack -- try for portal */
            if (borg_study_prayer(1,1)) return (TRUE);

            /* Hack -- try for satisfy hunger */
            if (borg_study_prayer(1,5)) return (TRUE);

            /* Hack -- try for remove fear */
            if (borg_study_prayer(0,3)) return (TRUE);

            /* Hack -- try for call light */
            if (borg_study_prayer(0,4)) return (TRUE);
        }

        /* Attempt to study anything */
        if (borg_study_any()) return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}




/*
 * Hack -- check a location for "dark room"
 */
static bool borg_light_room_aux(int x, int y)
{
    auto_grid *ag;
    
    int x1, y1, x2, y2;
    
    /* Illegal location */
    if (!grid_legal(x,y)) return (FALSE);

    /* Get grid */
    ag = grid(x,y);
    
    /* Location must be dark */
    if (ag->info & BORG_OKAY) return (FALSE);

    /* Location must be on panel */
    if (!(ag->info & BORG_HERE)) return (FALSE);

    /* Location must be in view */
    if (!(ag->info & BORG_VIEW)) return (FALSE);

    /* Build the rectangle */
    x1 = MIN(c_x, x);
    x2 = MAX(c_x, x);
    y1 = MIN(c_y, y);
    y2 = MAX(c_y, y);
    
    /* Scan the rectangle */
    for (y = y1; y <= y2; y++) {
        for (x = x1; x <= x2; x++) {

            /* Get grid */
            ag = grid(x,y);
    
            /* Location must not be a wall */
            if (ag->info & BORG_WALL) return (FALSE);
        }
    }

    /* Okay */
    return (TRUE);
}


/*
 * Light up the room (if possible and useful)
 */
static bool borg_light_room(void)
{
    bool okay = FALSE;
    
    auto_item *item = &auto_items[INVEN_LITE];
    
    
    /* Never in town */
    if (!auto_depth) return (FALSE);
    
    /* Never when blind or hallucinating */
    if (do_blind || do_image) return (FALSE);
     
    /* Paranoia -- no infinite loops */
    if (rand_int(100) < 50) return (FALSE);


    /* Check for "need light" */
    if (item->name1) {

        /* Check "Artifact Corners" */
        if (borg_light_room_aux(c_x + 3, c_y + 2) ||
            borg_light_room_aux(c_x + 3, c_y - 2) ||
            borg_light_room_aux(c_x - 3, c_y + 2) ||
            borg_light_room_aux(c_x - 3, c_y - 2) ||
            borg_light_room_aux(c_x + 2, c_y + 3) ||
            borg_light_room_aux(c_x + 2, c_y - 3) ||
            borg_light_room_aux(c_x - 2, c_y + 3) ||
            borg_light_room_aux(c_x - 2, c_y - 3)) {

            /* Go for it */
            okay = TRUE;
        }
    }
    
    /* Check for "need light" */
    else {

        /* Check "Lantern Corners" */
        if (borg_light_room_aux(c_x + 2, c_y + 2) ||
            borg_light_room_aux(c_x + 2, c_y - 2) ||
            borg_light_room_aux(c_x - 2, c_y + 2) ||
            borg_light_room_aux(c_x - 2, c_y - 2)) {

            /* Go for it */
            okay = TRUE;
        }
    }
    

    /* Not okay */
    if (!okay) return (FALSE);


    /* Mega-Hack -- activate the Phial */
    if (item->name1 == ART_GALADRIEL) {

        /* Hack -- Allow for recharge */
        if (when_phial + 50 < c_t) {

            /* Note the time */
            when_phial = c_t;
            
            /* Note */
            borg_note("Activating the Phial.");
            
            /* Activate the light */
            borg_keypress('A');
            borg_keypress('f');

            /* Success */
            return (TRUE);
        }
    }


    /* Go for it */
    if (borg_spell(0,3) ||
        borg_prayer(0,4) ||
        borg_zap_rod(SV_ROD_ILLUMINATION) ||
        borg_use_staff(SV_STAFF_LITE) ||
        borg_read_scroll(SV_SCROLL_LIGHT)) {

        /* Note */
        borg_note("Illuminating the room.");

        /* Success */
        return (TRUE);
    }


    /* No light */
    return (FALSE);
}




/*
 * Enchant weapons
 */
static bool borg_enchant_1(void)
{
    int i, b, a;

    int num_weapon = 0;

    
    /* Hack -- blind/confused -- no scrolls */
    if (do_blind || do_confused) return (FALSE);


    /* No enchantment scrolls */
    i = 0;

    /* Count enchantment scrolls */
    i += kind_have[kind_enchant_to_hit];
    i += kind_have[kind_enchant_to_dam];

    /* Add in "enchant" spell */
    /* XXX XXX XXX prayer_okay() */
    
    /* No enchantment scrolls */
    if (!i) return (FALSE);


    /* Mega-Hack -- count weapons in pack (see below) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Count weapons */
        if (borg_item_is_weapon(item)) num_weapon++;
    }


    /* Look for a weapon that needs enchanting */
    for (b = 0, a = 99, i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->able) continue;
        
        /* Find the least enchanted item */
        if (item->to_h > a) continue;

        /* Save the info */
        b = i; a = item->to_h;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 8) && (borg_action('r', kind_enchant_to_hit))) {

        /* Choose from equipment */
        if (num_weapon) borg_keypress('/');

        /* Choose that item */
        borg_keypress('a' + b - INVEN_WIELD);

        /* Success */
        return (TRUE);
    }


    /* Look for a weapon that needs enchanting */
    for (b = 0, a = 99, i = INVEN_WIELD; i <= INVEN_BOW; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->able) continue;
        
        /* Find the least enchanted item */
        if (item->to_d > a) continue;

        /* Save the info */
        b = i; a = item->to_d;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 8) && (borg_action('r', kind_enchant_to_dam))) {

        /* Choose from equipment */
        if (num_weapon) borg_keypress('/');

        /* Choose that item */
        borg_keypress('a' + b - INVEN_WIELD);

        /* Success */
        return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}


/*
 * Enchant armor
 */
static bool borg_enchant_2(void)
{
    int i, b, a;

    int num_armour = 0;

    
    /* Hack -- blind/confused -- no scrolls */
    if (do_blind || do_confused) return (FALSE);


    /* No enchantment scrolls */
    i = 0;

    /* Count enchantment scrolls */
    i += kind_have[kind_enchant_to_ac];

    /* Add in "enchant" spell */
    /* XXX XXX XXX prayer_okay() */
    
    /* No enchantment scrolls */
    if (!i) return (FALSE);


    /* Mega-Hack -- count armour/weapons in pack (see below) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Count armour */
        if (borg_item_is_armour(item)) num_armour++;
    }


    /* Look for armor that needs enchanting */
    for (b = 0, a = 99, i = INVEN_BODY; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;

        /* Skip non-identified items */
        if (!item->able) continue;
        
        /* Find the least enchanted item */
        if (item->to_a > a) continue;

        /* Save the info */
        b = i; a = item->to_a;
    }

    /* Enchant that item if "possible" */
    if (b && (a < 8) && (borg_action('r', kind_enchant_to_ac))) {

        /* Choose from equipment */
        if (num_armour) borg_keypress('/');

        /* Choose that item */
        borg_keypress('a' + b - INVEN_WIELD);

        /* Success */
        return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}



/*
 * Destroy everything we know we don't want
 */
static bool borg_crush_junk(void)
{
    int i;


    /* Quaff harmless "junk" potions/scrolls */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Only "discard" junk */
        if (!item->junk) continue;

        /* Process "allow" items */
        switch (item->tval) {
        
            case TV_POTION:

                /* Check the potion */
                switch (item->sval) {
                
                    case SV_POTION_WATER:
                    case SV_POTION_APPLE_JUICE:
                    case SV_POTION_SLIME_MOLD:
                    case SV_POTION_CURE_LIGHT:
                    case SV_POTION_CURE_SERIOUS:
                    case SV_POTION_CURE_CRITICAL:
                    case SV_POTION_HEALING:
                    case SV_POTION_STAR_HEALING:
                    case SV_POTION_LIFE:
                    case SV_POTION_RES_STR:
                    case SV_POTION_RES_INT:
                    case SV_POTION_RES_WIS:
                    case SV_POTION_RES_DEX:
                    case SV_POTION_RES_CON:
                    case SV_POTION_RES_CHR:
                    case SV_POTION_RESTORE_EXP:
                    case SV_POTION_RESTORE_MANA:
                    case SV_POTION_HEROISM:
                    case SV_POTION_BESERK_STRENGTH:
                    case SV_POTION_RESIST_HEAT:
                    case SV_POTION_RESIST_COLD:
                    case SV_POTION_INFRAVISION:
                    case SV_POTION_DETECT_INVIS:
                    case SV_POTION_SLOW_POISON:
                    case SV_POTION_CURE_POISON:
                    case SV_POTION_SPEED:

                        /* Try quaffing the potion */
                        if (borg_quaff_potion(item->sval)) return (TRUE);
                }
                
                break;

            case TV_SCROLL:
            
                /* Check the scroll */
                switch (item->sval) {

                    case SV_SCROLL_REMOVE_CURSE:
                    case SV_SCROLL_LIGHT:
                    case SV_SCROLL_MONSTER_CONFUSION:
                    case SV_SCROLL_RUNE_OF_PROTECTION:
                    case SV_SCROLL_STAR_REMOVE_CURSE:
                    case SV_SCROLL_DETECT_GOLD:
                    case SV_SCROLL_DETECT_ITEM:
                    case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
                    case SV_SCROLL_SATISFY_HUNGER:
                    case SV_SCROLL_DISPEL_UNDEAD:
                    case SV_SCROLL_BLESSING:
                    case SV_SCROLL_HOLY_CHANT:
                    case SV_SCROLL_HOLY_PRAYER:

                        /* Try reading the scroll */
                        if (borg_read_scroll(item->sval)) return (TRUE);
                }

                break;

            case TV_FOOD:
            
                /* Check the scroll */
                switch (item->sval) {

                    case SV_FOOD_CURE_POISON:
                    case SV_FOOD_CURE_BLINDNESS:
                    case SV_FOOD_CURE_PARANOIA:
                    case SV_FOOD_CURE_CONFUSION:
                    case SV_FOOD_CURE_SERIOUS:
                    case SV_FOOD_RESTORE_STR:
                    case SV_FOOD_RESTORE_CON:
                    case SV_FOOD_RESTORING:
                    case SV_FOOD_BISCUIT:
                    case SV_FOOD_JERKY:
                    case SV_FOOD_RATION:
                    case SV_FOOD_SLIME_MOLD:
                    case SV_FOOD_WAYBREAD:
                    case SV_FOOD_PINT_OF_ALE:
                    case SV_FOOD_PINT_OF_WINE:

                        /* Try eating the food (unless Bloated) */
                        if (!do_full && borg_action('E', item->kind)) return (TRUE);
                }

                break;
        }
    }


    /* Throw away "junk" */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Throw junk */
        if (item->junk) {

            /* Throw away junk */
            borg_note(format("Destroying junk (%s).", item->desc));

            /* Destroy that item */
            borg_keypress('k');
            borg_keypress('a' + i);

            /* Hack -- destroy a single item */
            if (item->iqty > 1) borg_keypress('\n');

            /* Mega-Hack -- verify destruction */
            borg_keypress('y');

            /* Did something */
            return (TRUE);
        }
    }


    /* Nothing to destroy */
    return (FALSE);
}



/*
 * Make sure we have at least one free inventory slot.
 *
 * Note that we assume that a "full pack" has all slots filled.
 *
 * This routine can only "fail" if the "panic_stuff" option is set,
 * in which case, we will refuse to destroy "unknown" and "essential"
 * items, even though they might be useless.
 */
static bool borg_free_space(void)
{
    int i, k;

    s32b cost, limit;

    auto_item *junk;


    /* We have plenty of space */
    if (!auto_items[INVEN_PACK-1].iqty) return (FALSE);


    /*** Pass one -- cheapest sellable item ***/
    
    /* Nothing to junk yet */
    junk = NULL;

    /* Nothing found yet */
    limit = 999999999L;
    
    /* Find something to trash */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-sell items */
        if (!item->cash) continue;

        /* Evaluate the slot */
        cost = item_worth(item) * item->iqty;

        /* Skip expensive items */
        if (junk && (cost > limit)) continue;

        /* Track cheapest item */
        junk = item;
        limit = cost;
    }

    /* Throw something away */
    if (junk) {

        /* Debug */
        borg_note(format("Junking %ld gold (sellable item).", limit));

        /* Hack -- Mark it as junk */
        junk->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_crush_junk()) return (TRUE);
    }


    /* Mega-Hack -- try to give "feelings" a chance */
    if (rand_int(100) != 0) return (FALSE);


    /*** Pass two -- cheapest "duplicate" item ***/
    
    /* Nothing to junk yet */
    junk = NULL;
    
    /* Nothing found yet */
    limit = 999999999L;
    
    /* Find something to trash (pass two) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Find the smallest pile of these items */
        k = borg_choose(item->kind);

        /* Only destroy "duplicate" piles */
        if (i == k) continue;

        /* Evaluate the slot */
        cost = item_worth(item) * item->iqty;

        /* Skip expensive items */
        if (junk && (cost > limit)) continue;

        /* Track cheapest item */
        junk = item;
        limit = cost;
    }

    /* Throw something away */
    if (junk) {

        /* Debug */
        borg_note(format("Junking %ld gold (duplicate item).", limit));

        /* Hack -- Mark it as junk */
        junk->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_crush_junk()) return (TRUE);
    }


    /*** Hack -- allow user to help out ***/
    
    /* Oops */
    if (panic_stuff) {

        borg_oops("Too much stuff.");
        return (FALSE);
    }


    /*** Pass three -- cheapest "unknown" item ***/
    
    /* Nothing to junk yet */
    junk = NULL;

    /* Nothing found yet */
    limit = 999999999L;
    
    /* Find something to trash */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Hack -- Skip identified items */
        if (item->able) continue;

        /* Evaluate the slot */
        cost = item_worth(item) * item->iqty;

        /* Skip expensive items */
        if (junk && (cost > limit)) continue;

        /* Track cheapest item */
        junk = item;
        limit = cost;
    }

    /* Throw something away */
    if (junk) {

        /* Debug */
        borg_note(format("Junking %ld+ gold in panic!", limit));

        /* Hack -- Mark it as junk */
        junk->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_crush_junk()) return (TRUE);
    }


    /*** Pass four -- cheapest item ***/

    /* Nothing to junk yet */
    junk = NULL;

    /* Nothing found yet */
    limit = 999999999L;
    
    /* Find something to trash */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Evaluate the slot */
        cost = item_worth(item) * item->iqty;

        /* Skip expensive items */
        if (junk && (cost > limit)) continue;

        /* Track cheapest item */
        junk = item;
        limit = cost;
    }

    /* Throw something away */
    if (junk) {

        /* Debug */
        borg_note(format("Junking %ld gold in desperation!", limit));

        /* Hack -- Mark it as junk */
        junk->junk = TRUE;

        /* Try to throw away the junk */
        if (borg_crush_junk()) return (TRUE);
    }


    /* Never happens */
    return (FALSE);
}


/*
 * Count the number of items worth "selling"
 * This determines the choice of stairs.
 */
static int borg_count_sell(void)
{
    int i, k = 0;
    s32b greed, worth;

    /* Calculate "greed" factor */
    greed = auto_gold / 100L;

    /* Throw away "junk" */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Skip non-items */
        if (!item->iqty) continue;	

        /* Skip "junk" items */
        if (item->junk) continue;

        /* Skip cheap "known" items */
        if (item->kind && item->cash && item->able) {

            /* Guess at the item value */
            worth = item_worth(item) * item->iqty;

            /* Do not bother with "cheap" items */
            if (worth < greed) continue;
        }

        /* Skip cheap "average" items */
        if (item->kind && item->cash && streq(item->note, "{average}")) {

            /* Guess at the item value */
            worth = item_worth(item) * item->iqty;

            /* Do not bother with "cheap" items */
            if (worth < greed) continue;
        }

        /* Count remaining items */
        k++;
    }


    /* Count them */
    return (k);
}




/*
 * Identify items if possible
 *
 * Note that "borg_parse()" will "cancel" the identification if it
 * detects a "You failed..." message.  This is VERY important!!!
 * Otherwise the "identify" might induce bizarre actions by sending
 * the "index" of an item as a command.
 */
static bool borg_test_stuff(void)
{
    int i;


    /* Look for an item to identify (equipment) */
    for (i = INVEN_WIELD; i <= INVEN_FEET; i++) {

        auto_item *item = &auto_items[i];
        
        /* Skip missing items */
        if (!item->iqty) continue;

        /* Skip known items */
        if (item->able) continue;

        /* Use a Spell/Prayer/Rod/Staff/Scroll of Identify */
        if (borg_spell(2,4) ||
            borg_prayer(5,2) ||
            borg_zap_rod(SV_ROD_IDENTIFY) ||
            borg_use_staff(SV_STAFF_IDENTIFY) ||
            borg_read_scroll(SV_SCROLL_IDENTIFY)) {

            /* Log -- may be cancelled */
            borg_note(format("Identifying %s.", item->desc));

            /* Hack -- Select the equipment */
            borg_keypress('/');
            
            /* Identify the item */
            borg_keypress('a' + i - INVEN_WIELD);

            /* Success */
            return (TRUE);
        }
    }


    /* Look for an item to identify (inventory) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];
        
        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip known items */
        if (item->able) continue;
        
        /* Skip tested items */
        if (!item->test) continue;

        /* Use a Spell/Prayer/Rod/Staff/Scroll of Identify */
        if (borg_spell(2,4) ||
            borg_prayer(5,2) ||
            borg_zap_rod(SV_ROD_IDENTIFY) ||
            borg_use_staff(SV_STAFF_IDENTIFY) ||
            borg_read_scroll(SV_SCROLL_IDENTIFY)) {

            /* Log -- may be cancelled */
            borg_note(format("Identifying %s.", item->desc));

            /* Identify the item */
            borg_keypress('a' + i);

            /* Success */
            return (TRUE);
        }
    }


    /* Nothing to do */
    return (FALSE);
}




/*
 * Maintain a "useful" inventory
 */
static bool borg_wear_stuff(void)
{
    int i, b_i = -1;

    s32b p, b_p = 0L;

    /* Wear stuff (top down) */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Skip "unknown" items */
        if (!item->wear) continue;

        /* Acquire the "power" */
        p = item_power(item);

        /* Skip bad items */
        if ((b_i >= 0) && (p < b_p)) continue;

        /* Maintain the best */
        b_i = i; b_p = p;
    }

    /* Wear the "best" item */
    if (b_i >= 0) {

        auto_item *item = &auto_items[b_i];

        /* Count the wearings */
        auto_wearing++;
        
        /* Log */
        borg_note(format("Wearing %s.", item->desc));

        /* Wear it */
        borg_keypress('w');
        borg_keypress('a' + b_i);

        /* Did something */
        return (TRUE);
    }


    /* Nothing to do */
    return (FALSE);
}









/*
 * Help the Borg decide if it should go back to the town
 *
 * This function analyzes the inventory for "essential" items
 * It could also verify adequate "resistance" and other stuff
 * If "bored" is true then we have fully explored the current level
 *
 * Proper use of stores requires "waiting" for them to "restock"
 * Note that this "waiting" must be done in the dungeon (level 1)
 *
 * Hack -- we also use this function to verify legality of using
 * word of recall in the town, using the "max_depth" flag (below)
 */
static bool borg_leave_level_aux(int depth, bool bored)
{
    auto_item *item = &auto_items[INVEN_LITE];



    /*** Essential Items ***/

    /* Require Lite and/or fuel */
    if (item->iqty == 0) return (TRUE);
    if ((item->kind == kind_torch) && (kind_have[kind_torch] < 10)) return (TRUE);
    if ((item->kind == kind_lantern) && (kind_have[kind_flask] < 10)) return (TRUE);

    /* Require Food Rations (usually) */
    if (kind_have[kind_food_ration] < (ignore_hunger ? 0 : 10)) return (TRUE);


    /*** Useful Items ***/

    /* Scrolls of Identify */
    if (kind_have[kind_scroll_identify] < 5) return (TRUE);
    if (bored && (kind_have[kind_scroll_identify] < 10)) return (TRUE);

    /* Scrolls of Word of Recall */
    if ((depth >= 2) && (kind_have[kind_scroll_recall] < 1)) return (TRUE);
    if ((depth >= 5) && (kind_have[kind_scroll_recall] < 2)) return (TRUE);
    if ((depth >= 5) && bored && (kind_have[kind_scroll_recall] < 4)) return (TRUE);

    /* Potions of Cure Serious Wounds */
    if ((depth >= 3) && (kind_have[kind_potion_serious] < 2)) return (TRUE);
    if ((depth >= 3) && bored && (kind_have[kind_potion_serious] < 5)) return (TRUE);

    /* Potions of Cure Critical Wounds */
    if ((depth >= 5) && (kind_have[kind_potion_critical] < 5)) return (TRUE);
    if ((depth >= 5) && bored && (kind_have[kind_potion_critical] < 10)) return (TRUE);


    /* Stay here */
    return (FALSE);
}



/*
 * Leave the level if necessary (or bored)
 */
static bool borg_leave_level(bool bored)
{
    int k, g = 0;

    int depth = (bored ? (auto_depth+1) : auto_depth);
    
    
    /* Cancel stair requests */
    stair_less = stair_more = FALSE;


    /* Hack -- already leaving via "recall" */
    if (when_recall + 100 >= c_t) return (FALSE);


    /* Count sellable items */
    k = borg_count_sell();


    /* Power-dive */
    if (auto_depth && (auto_lev > auto_depth * 2)) g = 1;

    /* Dive when bored */
    if (auto_depth && bored && (c_t > auto_shock + 1000L)) g = 1;

    /* Mega-Hack -- Do not leave a level until "bored" */
    if (c_t < auto_began + auto_feeling) g = 0;

    /* Do not dive when "full" of items */
    if (g && (k >= 18)) g = 0;

    /* Hack -- do not dive when drained */
    if (do_fix_exp) g = 0;


    /* Return to town (immediately) if we are worried */
    if (borg_leave_level_aux(depth, bored)) goal_rising = TRUE;
    
    /* Return to town to sell stuff */
    if (bored && (k >= 18)) goal_rising = TRUE;

    /* Hack -- return to town to restore experience */
    if (bored && do_fix_exp) goal_rising = TRUE;

    /* Return to town when level drained */
    if (do_fix_lev) goal_rising = TRUE;
    
    /* Never rise from town */
    if (!auto_depth) goal_rising = FALSE;

    /* Return to town */
    if (goal_rising) g = -1;

    /* Mega-Hack -- allow "stair scumming" at 50 feet */
    if ((auto_depth == 1) && !bored && (c_t < auto_began + 200L)) {
        if (g < 0) g = 0;
    }


    /* Bored and in town, go down */
    if (bored && !auto_depth) {

        /* Mega-Hack -- Recall into dungeon */
        if (max_depth && (max_depth >= 5) &&
            (kind_have[kind_scroll_recall] > 4) &&
            !borg_leave_level_aux(max_depth, TRUE)) {

            /* Note */
            borg_note("Considering recall into dungeon...");
            
            /* Give it a shot */
            if (borg_recall()) return (TRUE);
        }
            
        /* Go down */
        g = 1;
    }


    /* Use random stairs when really bored */
    if (auto_depth && bored && (c_t - auto_shock > 3000L)) {

        /* Note */
        borg_note("Choosing random stairs.");
        
        /* Use random stairs */
        g = (rand_int(100) < 50) ? -1 : 1;
    }


    /* Go Up */
    if (g < 0) {

        /* Hack -- use scroll of recall if able */
        if (goal_rising && (auto_depth > 4) &&
            (kind_have[kind_scroll_recall] > 3)) {
            if (borg_recall()) return (TRUE);
        }

        /* Attempt to use stairs */
        if (count_less && borg_flow_symbol('<')) {
            if (goal_rising) borg_note("Returning to town.");
            stair_less = TRUE;
            if (borg_play_old_goal()) return (TRUE);
        }

        /* Cannot find any stairs, try word of recall */
        if (goal_rising && bored && auto_depth) {
            if (borg_recall()) return (TRUE);
        }
    }


    /* Go Down */
    if (g > 0) {

        /* Attempt to use those stairs */
        if (count_more && borg_flow_symbol('>')) {
            borg_note("Diving into the dungeon.");
            stair_more = TRUE;
            if (borg_play_old_goal()) return (TRUE);
        }
    }


    /* Failure */
    return (FALSE);
}





/*
 * Hack -- find something to purchase in the stores
 */
static bool borg_think_store_buy_aux(void)
{
    int k, n;
    s32b p;

    int b_k = -1, b_n = -1;
    s32b b_p = 0L;


    /* XXX Hack -- notice "full" inventory */
    if (auto_items[INVEN_PACK-1].iqty) return (FALSE);


    /* Visit everyone before buying anything */
    for (k = 0; k < 8; k++) {

        /* Make sure we visited that shop */
        if (!auto_shops[k].visit) return (FALSE);
    }


    /* Check each store */
    for (k = 0; k < 8; k++) {
    
        auto_shop *shop = &auto_shops[k];

        /* Buy stuff */
        for (n = 0; n < 24; n++) {

            auto_item *ware = &shop->ware[n];

            /* Notice end of shop inventory */
            if (!ware->iqty) break;

            /* We must have "sufficient" cash */
            if (auto_gold < ware->cost) continue;

            /* Only buy useful stuff */
            if (!borg_good_buy(ware)) continue;

            /* Extract the worth of this item */
            p = item_power(ware);

            /* Hack -- notice the "best" thing */
            if ((b_n >= 0) && (p < b_p)) continue;

            /* Save the item and worth */
            b_k = k; b_n = n; b_p = p;
        }
    }


    /* Nothing to buy */
    if (b_k < 0) return (FALSE);
    
    
    /* Hack -- save this goal */
    goal_shop = b_k;
    goal_item = b_n;
    
    /* Success */
    return (TRUE);
}



/*
 * Buy items from the current store
 */
static bool borg_think_store_buy(void)
{
    /* See if there is anything good */
    if (!borg_think_store_buy_aux()) return (FALSE);
    
    /* Buy something if possible */
    if (goal_shop == shop_num) {

        auto_shop *shop = &auto_shops[shop_num];
        auto_item *ware = &shop->ware[goal_item];

        /* Minor Hack -- Go to the correct page */
        if ((goal_item / 12) != auto_shops[shop_num].page) borg_keypress(' ');

        /* Log */
        borg_note(format("Buying %s.", auto_shops[shop_num].ware[goal_item].desc));

        /* Buy an item */
        borg_keypress('p');

        /* Buy the desired item */
        borg_keypress('a' + (goal_item % 12));

        /* Hack -- Buy a single item */
        if (ware->iqty > 1) borg_keypress('\n');

        /* Mega-Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');

        /* Hack -- Note visit index */
        if (last_visit < shop->visit) last_visit = shop->visit;

        /* Success */
        return (TRUE);
    }


    /* Nothing to buy */
    return (FALSE);
}


/*
 * Sell items to the current store
 */
static bool borg_think_store_sell(void)
{
    int i, p;

    int b_n = -1, b_p = 0L;


    /* XXX Hack -- notice "full" store */
    if (auto_shops[shop_num].ware[23].iqty) return (FALSE);


    /* Sell stuff */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Consider "cash" items */
        if (!item->cash) continue;

        /* Do not try to make "bad" sales */
        if (!borg_good_sell(item)) continue;

        /* Extract the worth of this item */
        p = item_worth(item);

        /* Hack -- notice the "best" thing */
        if ((b_n >= 0) && (p < b_p)) continue;

        /* Save the item and worth */
        b_n = i; b_p = p;
    }

    /* Sell the most expensive item (if any) */
    if (b_n >= 0) {

        auto_item *item = &auto_items[b_n];

        /* Log */
        borg_note(format("Selling %s.", auto_items[b_n].desc));

        /* Sell that item */
        borg_keypress('s');
        borg_keypress('a' + b_n);

        /* Hack -- Sell a single item */
        if (item->iqty > 1) borg_keypress('\n');

        /* Hack -- Accept the price */
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');
        borg_keypress('\n');

        /* Success */
        return (TRUE);
    }


    /* Assume not */
    return (FALSE);
}


/*
 * Deal with being in a store
 */
static bool borg_think_store(void)
{
    int i, n;

    byte t_a;

    char desc[80];
    char cost[10];

    char buf[256];


    /* Hack -- make sure both pages are seen */
    static int browse = TRUE;

    /* What store did I *think* I was in */
    static int old_store = 0;

    /* How many pages did I *think* there were */
    static int old_more = 0;


    /* Hack -- reset page/more */
    auto_shops[shop_num].page = 0;
    auto_shops[shop_num].more = 0;


    /* React to new stores */
    if (old_store != shop_num) {

        /* Clear all the items */
        for (i = 0; i < 24; i++) {

            /* XXX Wipe the ware */
            WIPE(&auto_shops[shop_num].ware[i], auto_item);
        }

        /* Save the store */
        old_store = shop_num;
    }


    /* Extract the "page", if any */
    if ((0 == borg_what_text(20, 5, 8, &t_a, buf)) &&
        (prefix(buf, "(Page "))) /* --)-- */ {

        /* Take note of the page */
        auto_shops[shop_num].more = 1;
        auto_shops[shop_num].page = (buf[6] - '0') - 1;
    }

    /* React to disappearing pages */
    if (old_more != auto_shops[shop_num].more) {

        /* Clear the second page */
        for (i = 12; i < 24; i++) {

            /* XXX Wipe the ware */
            WIPE(&auto_shops[shop_num].ware[i], auto_item);
        }

        /* Save the new one */
        old_more = auto_shops[shop_num].more;
    }


    /* Extract the current gold (unless in home) */
    if (0 == borg_what_text(68, 19, -9, &t_a, buf)) {

        /* Ignore this "field" in the home */
        if (shop_num != 7) auto_gold = atol(buf);
    }


    /* Parse the store (or home) inventory */
    for (i = 0; i < 12; i++) {

        /* Default to "empty" */
        desc[0] = '\0';
        cost[0] = '\0';

        /* Verify "intro" to the item */
        if ((0 == borg_what_text(0, i + 6, 3, &t_a, buf)) &&
            (buf[0] == 'a' + i) && (buf[1] == p2) && (buf[2] == ' ')) {

            /* Extract the item description */
            if (0 != borg_what_text(3, i + 6, -65, &t_a, desc)) desc[0] = '\0';

            /* XXX Make sure trailing spaces get stripped */

            /* Extract the item cost */
            if (0 != borg_what_text(68, i + 6, -9, &t_a, cost)) cost[0] = '\0';

            /* Hack -- forget the cost in the home */
            if (shop_num == 7) cost[0] = '\0';
        }

        /* Extract actual index */
        n = auto_shops[shop_num].page * 12 + i;

        /* Ignore "unchanged" descriptions */
        if (streq(desc, auto_shops[shop_num].ware[n].desc)) continue;

        /* Analyze it (including the cost) */
        borg_item_analyze(&auto_shops[shop_num].ware[n], desc, cost);
    }


    /* Hack -- browse as needed */
    if (auto_shops[shop_num].more && browse) {
        borg_keypress(' ');
        browse = FALSE;
        return (TRUE);
    }

    /* Hack -- must browse later */
    browse = TRUE;


    /* Mega-Hack */
    if (rand_int(1000) == 0) {

        bool done = FALSE;

        /* Take off the left ring */
        if (auto_items[INVEN_LEFT].iqty) {
            borg_keypress('t');
            borg_keypress('c');
            done = TRUE;
        }

        /* Take off the right ring */
        if (auto_items[INVEN_RIGHT].iqty) {
            borg_keypress('t');
            borg_keypress('d');
            done = TRUE;
        }

        /* Success */
        if (done) return (TRUE);
    }


    /* Examine the inventory */
    borg_notice();

    /* Wear things */
    if (borg_wear_stuff()) return (TRUE);

    /* Try to sell stuff */
    if (borg_think_store_sell()) return (TRUE);

    /* Try to buy stuff */
    if (borg_think_store_buy()) return (TRUE);


    /* Count the successful visits */
    auto_shops[shop_num].visit++;


    /* Leave the store */
    borg_keypress(ESCAPE);

    /* Done */
    return (TRUE);
}






/*
 * Decide whether to "inspect" a grid
 * Assume the given grid is NOT a wall
 */
static bool borg_inspect(void)
{
    int		i, wall = 0, supp = 0, diag = 0;

    char	cc[8];


    /* Hack -- Never inspect the town */
    if (auto_depth == 0) return (FALSE);

    /* Hack -- This grid is fully inspected */
    if (grid(c_x,c_y)->xtra > 20) return (FALSE);

    /* Tweak -- only search occasionally */
    if (rand_int(50) != 0) return (FALSE);


    /* Examine adjacent grids */
    for (i = 0; i < 8; i++) {

        /* Extract the location */
        int xx = c_x + ddx[ddd[i]];
        int yy = c_y + ddy[ddd[i]];

        /* Obtain the grid */
        auto_grid *ag = grid(xx, yy);

        /* Require knowledge */
        if (ag->o_c == ' ') return (FALSE);

        /* Extract the symbol */
        cc[i] = ag->o_c;
    }


    /* Count possible door locations */
    for (i = 0; i < 4; i++) if (cc[i] == '#') wall++;

    /* Hack -- no possible doors */
    if (!wall) return (FALSE);


    /* Count supporting evidence for secret doors */
    for (i = 0; i < 4; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) supp++;
        else if ((cc[i] == '+') || (cc[i] == '\'')) supp++;
    }

    /* Count supporting evidence for secret doors */
    for (i = 4; i < 8; i++) {
        if ((cc[i] == '#') || (cc[i] == '%')) diag++;
    }

    /* Hack -- Examine "suspicious" walls */
    if (((diag >= 4) && (supp >= 3)) ||
        ((diag >= 4) && (rand_int(30) == 0)) ||
        ((diag >= 2) && (rand_int(900) == 0))) {

        /* Take note */
        borg_note("Searching...");

        /* Remember the search */
        if (grid(c_x,c_y)->xtra < 100) grid(c_x,c_y)->xtra += 9;

        /* Search a little */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }


    /* Assume no suspicions */
    return (FALSE);
}


/*
 * Go shopping in the town (do stores in order)
 */
static bool borg_flow_shop(void)
{
    int i, n = -1, v = 99;

    /* Must be in town */
    if (auto_depth) return (FALSE);

    /* Visit the oldest shop */
    for (i = 0; i < 8; i++) {

        /* Skip recent shops */
        if ((n >= 0) && (auto_shops[i].visit >= v)) continue;

        /* Save this shop */
        n = i; v = auto_shops[i].visit;
    }

    /* Plenty of visits, time to stop */
    if (v >= last_visit + 2) return (FALSE);

    /* Hack -- think about buying */
    if (borg_think_store_buy_aux()) {

        /* Message */
        borg_note(format("Shopping for '%s' at '%s'.",
                         auto_shops[goal_shop].ware[goal_item].desc,
                         k_list[OBJ_STORE_LIST+goal_shop].name));

        /* Prefer that shop */
        n = goal_shop;
    }

    /* Try and visit it */
    if (borg_flow_symbol('1' + n)) return (TRUE);
    
    /* Failure */
    return (FALSE);
}




/*
 * Perform an action in the dungeon
 *
 * Return TRUE if a "meaningful" action was performed
 * Otherwise, return FALSE so we will be called again
 *
 * Strategy:
 *   Make sure we are happy with our "status" (see above)
 *   Attack and kill visible monsters, if near enough
 *   Open doors, disarm traps, tunnel through rubble
 *   Pick up (or tunnel to) gold and useful objects
 *   Explore "interesting" grids, to expand the map
 *   Explore the dungeon and revisit old grids
 */
static bool borg_think_dungeon(void)
{
    /* Increase the "time" */
    c_t++;

    /* Analyze the "frame" */
    borg_update_frame();

    /* Track best level */
    if (auto_lev > max_level) max_level = auto_lev;
    if (auto_depth > max_depth) max_depth = auto_depth;
    
        
    /* Hack -- react to new depth */
    if (old_depth != auto_depth) {

        int i;

        /* Clear "recall stamp" */
        when_recall = 0L;

        /* Clear "wearing counter" */
        auto_wearing = 0;

        /* Clear "feeling power" in town */
        if (!auto_depth) auto_feeling = 0L;

        /* Hack -- clear "shops" */
        for (i = 0; i < 8; i++) {

            /* No visit yet */
            auto_shops[i].visit = 0;
        }
        
        /* Wipe the "ext" info */
        borg_ext_wipe();
    }
    
    /* Examine the screen */
    borg_update();

    /* Hack -- notice hallucination */
    if (do_image) {
        borg_oops("Hallucinating (?)");
        return (TRUE);
    }


    /*** Deal with important stuff ***/
    
    /* Analyze the inventory */
    borg_notice();

    /* Wear things that need to be worn */
    if (borg_wear_stuff()) return (TRUE);

    /* Use things */
    if (borg_use_things()) return (TRUE);

    /* Flee to the town when low on crucial supplies */
    if (borg_restock() && auto_depth && borg_recall()) return (TRUE);

    /* Hack -- attempt to flee the level after a long time */
    if ((c_t - auto_began > 50000L) && borg_recall()) return (TRUE);


    /*** Deal with monsters ***/

    /* Apply a tiny bit of chaos */
    if (rand_int(1000) == 0) {
        borg_keypress('0' + randint(9));
        return (TRUE);
    }

    /* Try not to get surrounded by monsters */
    if (borg_caution()) return (TRUE);

    /* Attack neighboring monsters */
    if (borg_attack()) return (TRUE);

    /* Fire at nearby monsters */
    if (borg_play_fire()) return (TRUE);

    /* Continue flowing towards monsters */
    if (goal == GOAL_KILL) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Hack -- Apply a tiny amount of chaos */
    if (rand_int(1000) == 0) {
        borg_keypress('0' + randint(9));
        return (TRUE);
    }

    /* Rest occasionally if damaged */
    if (((auto_chp < auto_mhp) || (auto_csp < auto_csp)) &&
        (rand_int(100) < 50)) {

        /* Take note */
        borg_note("Resting...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }

    /* Go find a monster */
    if (borg_flow_kill()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Deal with inventory objects ***/

    /* Use other things */
    if (borg_use_others()) return (TRUE);

    /* Illuminate the room */
    if (borg_light_room()) return (TRUE);
    
    /* Identify unknown things */
    if (borg_test_stuff()) return (TRUE);

    /* Enchant things */
    if (borg_enchant_1()) return (TRUE);
    if (borg_enchant_2()) return (TRUE);

    /* XXX Recharge things */
    
    /* Throw away junk */
    if (borg_crush_junk()) return (TRUE);

    /* Acquire free space */
    if (borg_free_space()) return (TRUE);


    /*** Flow towards objects ***/

    /* Hack -- beware of blindness and confusion */
    if ((do_blind || do_confused) && (rand_int(100) < 50)) {

        /* Take note */
        borg_note("Resting...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }
    
    /* Continue flowing to objects */
    if (goal == GOAL_TAKE) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Rest occasionally */
    if (((auto_chp < auto_mhp) || (auto_csp < auto_msp)) &&
        (rand_int(100) < 50)) {

        /* Take note */
        borg_note("Resting...");

        /* Rest until done */
        borg_keypress('R');
        borg_keypress('&');
        borg_keypress('\n');

        /* Done */
        return (TRUE);
    }

    /* Start a new one */
    if (borg_flow_take()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Exploration ***/

    /* Hack -- Search intersections for doors */
    if (borg_inspect()) return (TRUE);

    /* Continue flowing (explore) */
    if (goal == GOAL_DARK) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Continue flowing (see below) */
    if (goal == GOAL_XTRA) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Try grids that are farther away ***/

    /* Chase old monsters */
    if (borg_flow_kill_any()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Chase old objects */
    if (borg_flow_take_any()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Wander around ***/

    /* Hack -- occasionally update the "free room" list */
    /* if (rand_int(100) == 0) borg_free_room_update(); */

    /* Leave the level (if needed) */
    if (borg_leave_level(FALSE)) return (TRUE);

    /* Explore nearby interesting grids */
    if (borg_flow_dark()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Explore interesting grids */
    if (borg_flow_explore()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Hack -- Visit the shops */
    if (borg_flow_shop()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Leave the level (if possible) */
    if (borg_leave_level(TRUE)) return (TRUE);

    /* Search for secret doors */
    if (borg_flow_spastic()) {
        if (borg_play_old_goal()) return (TRUE);
    }

    /* Re-visit old rooms */
    if (borg_flow_revisit()) {
        if (borg_play_old_goal()) return (TRUE);
    }


    /*** Oops ***/

    /* This is a bad thing */
    borg_note("Twitchy!");

    /* Try searching */
    if (rand_int(10) == 0) {

        /* Take note */
        borg_note("Searching...");

        /* Remember the search */
        if (grid(c_x,c_y)->xtra < 100) grid(c_x,c_y)->xtra += 9;

        /* Search */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('s');

        /* Success */
        return (TRUE);
    }

    /* Mega-Hack -- Occasional tunnel */
    if (rand_int(100) == 0) {

        /* Tunnel a lot */
        borg_keypress('0');
        borg_keypress('9');
        borg_keypress('9');
        borg_keypress('T');

        /* In a legal direction */
        borg_keypress('0' + ddd[rand_int(8)]);
    }

    /* Move (or tunnel) */
    borg_keypress('0' + randint(9));

    /* We did something */
    return (TRUE);
}








/*
 * Think about the world and perform an action
 *
 * Check inventory/equipment once per "turn"
 *
 * Process "store" and other modes when necessary
 *
 * Note that the non-cheating "inventory" and "equipment" parsers
 * will get confused by a "weird" situation involving an ant ("a")
 * on line one of the screen, near the left, next to a shield, of
 * the same color, and using --(-- the ")" symbol, directly to the
 * right of the ant.  This is very rare, but perhaps not completely
 * impossible.  I ignore this situation.  :-)
 */
static void borg_think(void)
{
    int i;

    byte t_a;

    char buf[128];


    /*** Process inventory/equipment and Browse Books ***/

    /* Cheat */
    if (cheat_equip && do_equip) {

        /* Only do it once */
        do_equip = FALSE;

        /* Cheat the "equip" screen */
        borg_cheat_equip();
        
        /* Done */
        return;
    }

    /* Cheat */
    if (cheat_inven && do_inven) {

        /* Only do it once */
        do_inven = FALSE;

        /* Cheat the "inven" screen */
        borg_cheat_inven();
        
        /* Done */
        return;
    }


    /* Parse "equip" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
       (streq(buf, "Equipment "))) {

        /* Parse the "equip" screen */
        borg_parse_equip();

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return;
    }


    /* Parse "inven" mode */
    if ((0 == borg_what_text(0, 0, 10, &t_a, buf)) &&
       (streq(buf, "Inventory "))) {

        /* Parse the "inven" screen */
        borg_parse_inven();

        /* Leave this mode */
        borg_keypress(ESCAPE);

        /* Done */
        return;
    }


    /* Check "equip" */
    if (do_equip) {

        /* Only do it once */
        do_equip = FALSE;

        /* Enter "equip" mode */
        borg_keypress('e');

        /* Done */
        return;
    }

    /* Check "inven" */
    if (do_inven) {

        /* Only do it once */
        do_inven = FALSE;

        /* Enter "inven" mode */
        borg_keypress('i');

        /* Done */
        return;
    }


    /*** Handle stores ***/

    /* Hack -- Check for being in a store */
    if ((0 == borg_what_text(3, 5, 16, &t_a, buf)) &&
       (streq(buf, "Item Description"))) {

        /* Assume the Home */
        shop_num = 7;

        /* Extract the "store" name */
        if (0 == borg_what_text(50, 3, -20, &t_a, buf)) {

            int i;

            /* Check the store names */
            for (i = 0; i < 7; i++) {
                cptr name = k_list[OBJ_STORE_LIST+i].name;
                if (prefix(buf, name)) shop_num = i;
            }
        }

#if 0
        /* Note the store */
        borg_note(format("Inside store '%d' (%s)", shop_num + 1,
                         k_list[OBJ_STORE_LIST+shop_num].name));
#endif

        /* Mega-Hack -- Check inventory/equipment again later */
        do_inven = do_equip = TRUE;

        /* Process the store */
        while (auto_active) {

            /* Think */
            if (borg_think_store()) break;
        }
        
        /* Done */
        return;
    }



    /*** Handle browsing ***/
    
    /* Hack -- Warriors never browse */
    if (auto_class == 0) do_spell = FALSE;
    
    /* Hack -- Blind or Confused prevents browsing */
    if (do_blind || do_confused) do_spell = FALSE;

    /* Hack -- Stop doing spells when done */
    if (do_spell_aux > 8) do_spell = FALSE;

    /* Cheat */
    if (cheat_spell && do_spell) {

        /* Look for the book */
        i = borg_book(do_spell_aux);
        
        /* Cheat the "spell" screens (all of them) */
        if (i >= 0) {
            
            /* Cheat that page */
            borg_cheat_spell(do_spell_aux);
        }
            
        /* Advance to the next book */
        do_spell_aux++;
        
        /* Done */
        return;
    }

    /* Check for "browse" mode */
    if ((0 == borg_what_text(COL_SPELL, ROW_SPELL, -12, &t_a, buf)) &&
        (streq(buf,"Lv Mana Fail"))) {

        /* Parse the "spell" screen */
        borg_parse_spell(do_spell_aux);

        /* Leave that mode */
        borg_keypress(ESCAPE);

        /* Advance to the next book */
        do_spell_aux++;

        /* Done */
        return;
    }

    /* Check "spells" */
    if (do_spell) {

        /* Look for the book */
        i = borg_book(do_spell_aux);
        
        /* Enter the "spell" screen */
        if (i >= 0) {

            /* Enter "browse" mode */
            borg_keypress('b');

            /* Pick the next book */
            borg_keypress('a' + i);
        }

        /* Otherwise, advance */
        else {

            /* Advance to the next book */
            do_spell_aux++;
        }

        /* Done */
        return;
    }


    /*** Determine panel ***/

    /* Hack -- cheat */
    if (cheat_panel) {

        /* Cheat */
        w_y = panel_row * (SCREEN_HGT / 2);
        w_x = panel_col * (SCREEN_WID / 2);

        /* Done */
        do_panel = FALSE;
    }

    /* Hack -- Check for "sector" mode */
    if ((0 == borg_what_text(0, 0, 16, &t_a, buf)) &&
       (prefix(buf, "Map sector "))) {

        /* Hack -- get the panel info */
        w_y = (buf[12] - '0') * (SCREEN_HGT / 2);
        w_x = (buf[14] - '0') * (SCREEN_WID / 2);

        /* Leave panel mode */
        borg_keypress(ESCAPE);

        /* Done */
        return;
    }

    /* Check equipment */
    if (do_panel) {
    
        /* Only do it once */
        do_panel = FALSE;

        /* Enter "panel" mode */
        borg_keypress('L');

        /* Done */
        return;
    }


    /*** Re-activate Tests ***/

    /* Check equip again later */
    do_equip = TRUE;

    /* Check inven again later */
    do_inven = TRUE;

    /* Check spells again later */
    do_spell = TRUE;
    
    /* Check panel again later */
    do_panel = TRUE;

    /* Hack -- Start the books over */
    do_spell_aux = 0;
    

    /*** Actually Do Something ***/

    /* Repeat until done */
    while (auto_active) {

        /* Do domething */
        if (borg_think_dungeon()) break;
    }
        
    /* Save the depth */
    old_depth = auto_depth;

    /* Save the hit points */
    old_chp = auto_chp;

    /* Save the spell points */
    old_csp = auto_csp;
}



/*
 * Hack -- Parse a message from the world
 *
 * Note that detecting failures is EXTREMELY important, to prevent
 * bizarre situations after failing to use a staff of perceptions,
 * which would otherwise go ahead and send the "item index" which
 * might be a legal command (such as "a" for "aim").
 *
 * Currently, nothing else must be parsed, though some interesting
 * possibilities suggest themselves... :-)
 *
 * We should extract the "identity" of monsters, to help over-ride
 * the default "monster identity" routines in "borg-ext.c".  These
 * can also be used to notice the presence of invisible monsters.
 */
static void borg_parse(cptr msg)
{
    /* Log (if needed) */
    if (auto_fff) borg_info(format("Msg <%s>", msg));


    /* Detect various "failures" */
    if (prefix(msg, "You failed ")) {

        /* Flush our key-buffer */
        borg_flush();
        return;
    }


    /* Killed somebody */
    if (prefix(msg, "You have killed ") || prefix(msg, "You have slain ")) {

        /* Note kills */
        borg_note(msg);
        return;
    }


    /* Hit somebody */
    if (prefix(msg, "You hit ")) {

        /* Ignore teleport trap */
        if (prefix(msg, "You hit a teleport")) return;
        
        /* Note hits */
        borg_note(msg);
        return;
    }

    /* Miss somebody */
    if (prefix(msg, "You miss ")) {

        /* Note misses */
        borg_note(msg);
        return;
    }


    /* Something happens to the player */
    if (suffix(msg, " you.")) {

        /* Ignore arrow traps */
        if (prefix(msg, "An arrow ")) return;

        /* Ignore dart traps */
        if (prefix(msg, "A small dart ")) return;
        
        /* Ignore darkness */
        if (suffix(msg, " surrounds you.")) return;
        
        /* Ignore gravity */
        if (suffix(msg, " warps around you.")) return;
        
        /* Ignore remove curse */
        if (suffix(msg, " watching over you.")) return;
        
        /* Ignore "stares at" */
        if (suffix(msg, " stares at you.")) return;
        
        /* Ignore "gestures at" */
        if (suffix(msg, " gestures at you.")) return;
        
        /* Ignore shop-keeper */
        if (suffix(msg, " glares at you.")) return;
        
        /* Ignore shop-keeper */
        if (suffix(msg, " hear you.")) return;
        
        /* Miss */
        if (suffix(msg, " misses you.")) {
        
            /* Note misses */
            borg_note(msg);
        }

        /* Hit */
        else {

            /* Note hits */
            borg_note(msg);
        }
        
        return;
    }


#if 0
    /* Hack -- detect word of recall */
    if (prefix(msg, "The air about you becomes charged.")) {

        /* Note the time */
        when_recall = c_t;
        return;
    }
#endif

    /* Hack -- detect word of recall */
    if (prefix(msg, "A tension leaves ")) {

        /* Hack -- Oops */
        when_recall = 0L;
        return;
    }

    /* Word of recall fizzles out or is cancelled */
    if (prefix(msg, "You feel tension leave ")) {

        /* Note the time */
        when_recall = 0L;
        return;
    }

    /* Word of recall kicks in */
    if (prefix(msg, "You feel yourself yanked ")) {

        /* Note the time */
        when_recall = 0L;
        return;
    }


    /* Feelings */
    if (prefix(msg, "You feel there is something special")) {

        /* Hang around */
        auto_feeling = 2000L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You have a superb feeling")) {

        /* Hang around */
        auto_feeling = 2000L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You have an excellent feeling")) {

        /* Hang around */
        auto_feeling = 1500L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You have a very good feeling")) {

        /* Hang around */
        auto_feeling = 1000L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You have a good feeling")) {

        /* Hang around */
        auto_feeling = 500L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You feel strangely lucky")) {

        /* Stair scum */
        auto_feeling = 200L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You feel your luck is turning")) {

        /* Stair scum */
        auto_feeling = 200L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "You like the look of this place")) {

        /* Stair scum */
        auto_feeling = 200L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "This level can't be all bad")) {

        /* Stair scum */
        auto_feeling = 200L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "What a boring place")) {

        /* Stair scum */
        auto_feeling = 200L;
        return;
    }

    /* Feelings */
    if (prefix(msg, "Looks like any other level")) {

        /* Stair scum */
        auto_feeling = 200L;
        return;
    }
}







/*
 * Initialize the "Ben Borg"
 */
static void borg_ben_init(void)
{
    byte *test;


    /* Message */
    msg_print("Initializing the Borg...");

    /* Hack -- flush it */
    Term_fresh();

    /* Mega-Hack -- verify memory */
    C_MAKE(test, 400 * 1024L, byte);
    C_FREE(test, 400 * 1024L, byte);


    /*** Initialize ***/

    /* Init "borg-ext.c" */
    borg_ext_init();

    /* Hook in my functions */
    borg_hook_think = borg_think;

    /* Hook in my functions */
    borg_hook_parse = borg_parse;

    /* Turn off the recall window */
    use_recall_win = FALSE;


    /*** Cheat / Panic ***/
    
    /* Mega-Hack -- Cheat a lot */
    cheat_inven = TRUE;
    cheat_equip = TRUE;

    /* Mega-Hack -- Cheat a lot */
    cheat_spell = TRUE;

    /* Mega-Hack -- Cheat a lot */
    cheat_panel = TRUE;

    /* Mega-Hack -- Panic a lot */
    panic_death = TRUE;
    panic_stuff = TRUE;


    /*** All done ***/
     
    /* Done initialization */
    msg_print("done.");

    /* Now it is ready */
    auto_ready = TRUE;
}



/*
 * Hack -- declare the "borg_ben()" function
 */
extern void borg_ben(void);


/*
 * Hack -- interact with the "Ben Borg".
 */
void borg_ben(void)
{
    char cmd;


    /* Mega-Hack -- Require wizard mode to initialize */
    if (!auto_ready && !wizard) {

        /* Warning */
        msg_print("You must be in wizard mode to initialize the Borg.");

        /* Ignore */
        return;
    }
    
    
    /* Get a "Borg command" */
    if (!get_com("Borg command: ", &cmd)) return;


    /* Require initialization */
    if (!auto_ready) {

        /* Allow initialization */
        if (cmd == '$') borg_ben_init();

        /* Otherwise be annoyed */
        else msg_print("You must initialize the Borg.");

        /* Done */
        return;
    }
    


    /* Command: Resume */
    if (cmd == 'r') {

        /* Activate (see below) */
        auto_active = TRUE;
    }

    /* Command: Restart -- use an "illegal" level */
    else if (cmd == 'z') {

        /* Activate (see below) */
        auto_active = TRUE;

        /* Hack -- restart the level */
        old_depth = -1;
    }


    /* Start a new log file */
    else if (cmd == 'f') {

        char buf[1024];

        /* Close the log file */
        if (auto_fff) fclose(auto_fff);

        /* Make a new log file name */
        prt("Borg Log: ", 0, 0);

        /* Hack -- drop permissions */
        safe_setuid_drop();

        /* Get the name and open the log file */
        if (askfor(buf, 70)) auto_fff = fopen(buf, "w");

        /* Hack -- grab permissions */
        safe_setuid_grab();

        /* Failure */
        if (!auto_fff) msg_print("Cannot open that file.");
    }


    /* Command: toggle some "cheat" flags */
    else if (cmd == CTRL('i')) {
        cheat_inven = !cheat_inven;
        msg_format("Borg -- cheat_inven is now %d.", cheat_inven);
    }
    else if (cmd == CTRL('e')) {
        cheat_equip = !cheat_equip;
        msg_format("Borg -- cheat_equip is now %d.", cheat_equip);
    }
    else if (cmd == CTRL('s')) {
        cheat_spell = !cheat_spell;
        msg_format("Borg -- cheat_spell is now %d.", cheat_spell);
    }
    else if (cmd == CTRL('p')) {
        cheat_panel = !cheat_panel;
        msg_format("Borg -- cheat_panel is now %d.", cheat_panel);
    }


    /* Command: toggle "panic_death" */
    else if (cmd == 'd') {
        panic_death = !panic_death;
        msg_format("Borg -- panic_death is now %d.", panic_death);
    }

    /* Command: toggle "panic_stuff" */
    else if (cmd == 'j') {
        panic_stuff = !panic_stuff;
        msg_format("Borg -- panic_stuff is now %d.", panic_stuff);
    }


#if 0

    /* Command: Show all Rooms (Containers) */
    else if (cmd == 's') {

        int k, x, y, xx, yy;

        int w_y = panel_row * (SCREEN_HGT / 2);
        int w_x = panel_col * (SCREEN_WID / 2);

        auto_room *ar;

        /* Examine all the rooms */
        for (y = w_y; y < w_y + SCREEN_HGT; y++) {
            for (x = w_x; x < w_x + SCREEN_WID; x++) {

                /* Scan the rooms */
                for (ar = room(1,x,y); ar; ar = room(0,0,0)) {

                    /* Skip done rooms */
                    if ((ar->y1 < y) && (y > w_y)) continue;
                    if ((ar->x1 < x) && (x > w_x)) continue;

                    /* Hack -- hilite the room -- count draws */
                    for (yy = ar->y1; yy <= ar->y2; yy++) {	
                        for (xx = ar->x1; xx <= ar->x2; xx++) {	
                            byte a = TERM_RED;
                            char c = '*';
                            if (grid(xx,yy)->info & BORG_VIEW) a = TERM_YELLOW;
                            if (grid(xx,yy)->o_c == ' ') c = '?';
                            print_rel(c, a, yy, xx);
                        }
                    }

                    /* Describe and wait */
                    Term_putstr(0, 0, -1, TERM_WHITE,
                                format("Room %d (%dx%d).", ar->self,
                                (1 + ar->x2 - ar->x1), (1 + ar->y2 - ar->y1)));

                    /* Get a key */
                    k = inkey();

                    /* Hack -- hilite the room -- count draws */
                    for (yy = ar->y1; yy <= ar->y2; yy++) {	
                        for (xx = ar->x1; xx <= ar->x2; xx++) {	
                            lite_spot(yy, xx);
                        }
                    }

                    /* Erase the prompt */
                    Term_erase(0, 0, 80-1, 0);

                    /* Flush the erase */
                    Term_fresh();

                    /* Leave the outer loop */
                    if (k == ESCAPE) x = y = 999;

                    /* Leave this loop */
                    if (k == ESCAPE) break;
                }
            }
        }
    }

    /* Command: Rooms (Containers) */
    else if (cmd == 'c') {

        int n, w, h;

        auto_room *ar;

        int used = 0, n_1x1 = 0, n_1xN = 0, n_Nx1 = 0;
        int n_2x2 = 0, n_2xN = 0, n_Nx2 = 0, n_NxN = 0;

        /* Examine all the rooms */
        for (n = 0; n < AUTO_ROOMS; n++) {

            /* Access the n'th room */
            ar = &auto_rooms[n];

            /* Skip "dead" rooms */
            if (ar->free) continue;

            /* Count the "used" rooms */
            used++;

            /* Extract the "size" */
            w = 1 + ar->x2 - ar->x1;
            h = 1 + ar->y2 - ar->y1;

            /* Count the "singles" */
            if ((w == 1) && (h == 1)) n_1x1++;
            else if (w == 1) n_1xN++;
            else if (h == 1) n_Nx1++;
            else if ((w == 2) && (h == 2)) n_2x2++;
            else if (w == 2) n_2xN++;
            else if (h == 2) n_Nx2++;
            else n_NxN++;
        }


        /* Display some info */	
        msg_format("Rooms: %d/%d used.", used, auto_room_max);
        msg_format("Corridors: 1xN (%d), Nx1 (%d)", n_1xN, n_Nx1);
        msg_format("Thickies: 2x2 (%d), 2xN (%d), Nx2 (%d)",
                         n_2x2, n_2xN, n_Nx2);
        msg_format("Singles: %d.  Normals: %d.", n_1x1, n_NxN);
        msg_print(NULL);
    }

    /* Command: Grid Info */
    else if (cmd == 'g') {

        int x, y, n;

        auto_grid *ag;
        auto_room *ar;

        int tg = 0;
        int tr = 0;

        int cc[8];

        /* Count the crossing factors */
        cc[0] = cc[1] = cc[2] = cc[3] = cc[4] = cc[5] = cc[6] = cc[7] = 0;

        /* Total explored grids */
        for (y = 0; y < AUTO_MAX_Y; y++) {
            for (x = 0; x < AUTO_MAX_X; x++) {

                /* Get the grid */
                ag = grid(x, y);

                /* Skip unknown grids */
                if (ag->o_c == ' ') continue;

                /* Count them */
                tg++;

                /* Count the rooms this grid is in */
                for (n = 0, ar = room(1,x,y); ar && (n<7); ar = room(0,0,0)) n++;

                /* Hack -- Mention some locations */
                if ((n > 1) && !cc[n]) {
                    msg_format("The grid at %d,%d is in %d rooms.", x, y, n);
                }

                /* Count them */
                if (n) tr++;

                /* Count the number of grids in that many rooms */
                cc[n]++;
            }
        }

        /* Display some info */	
        msg_format("Grids: %d known, %d in rooms.", tg, tr);
        msg_format("Roomies: %d, %d, %d, %d, %d, %d, %d, %d.",
                   cc[0], cc[1], cc[2], cc[3],
                   cc[4], cc[5], cc[6], cc[7]);
        msg_print(NULL);
    }

#endif

    /* Oops */
    else {

        /* Message */
        msg_print("That is not a legal Borg command.");
    }
    

    /* Mega-Hack -- Activate Borg */
    if (auto_active) {

        /* Mega-Hack -- Turn off wizard mode */
        wizard = FALSE;

        /* Mega-Hack -- Redraw Everything */
        do_cmd_redraw();

        /* Mega-Hack -- Flush the borg key-queue */
        borg_flush();

        /* Mega-Hack -- make sure we are okay */
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);
        borg_keypress(ESCAPE);

        /* Mega-Hack -- ask for a feeling */
        borg_keypress('^');
        borg_keypress('F');
    }
}





#endif

