/* File: borg-obj.c */

/* Purpose: Object parsing routines for the Borg	-BEN- */

#include "angband.h"


#ifdef ALLOW_BORG

#include "borg.h"

#include "borg-obj.h"



/*
 * See "borg.h" for general information.
 *
 * This file provides general support for various "Borg" files,
 * mostly in the form of various "object parsing" routines.
 */



/*
 * Hack -- single character constants
 */

static const char /* p1 = '(', */ p2 = ')';
static const char c1 = '{' /* , c2 = '}' */;




/*
 * Some variables
 */
 
auto_item *auto_items;		/* Current "inventory" */

auto_shop *auto_shops;		/* Current "shops" */



/*
 * General info
 */
 
byte auto_tval_ammo;	/* Tval of usable ammo */



/*
 * Kind indexes of the relevant books
 */

s16b kind_book[9];



/*
 * Spell info
 */

auto_spell auto_spells[9][9];	/* Spell info, by book/what */




/*
 * Constant "item description parsers" (singles)
 */
static int auto_size_single;		/* Number of "singles" */
static s16b *auto_what_single;		/* Kind indexes for "singles" */
static cptr *auto_text_single;		/* Textual prefixes for "singles" */

/*
 * Constant "item description parsers" (plurals)
 */
static int auto_size_plural;		/* Number of "plurals" */
static s16b *auto_what_plural;		/* Kind index for "plurals" */
static cptr *auto_text_plural;		/* Textual prefixes for "plurals" */

/*
 * Constant "item description parsers" (suffixes)
 */
static int auto_size_artego;		/* Number of "artegos" */
static s16b *auto_what_artego;		/* Indexes for "artegos" */
static cptr *auto_text_artego;		/* Textual prefixes for "artegos" */






/*
 * Extract the "NNN" from "(with NNN turns of lite)", or return (-1)
 */
static int extract_fuel(cptr tail)
{
    /* Require the prefix and suffix */
    if (!prefix(tail, "(with ")) return (-1);
    if (!suffix(tail, " of light)")) return (-1);

    /* Extract and return the "turns of lite" */
    return (atoi(tail + 6));
}


/*
 * Extract the "NNN" from "(NNN charges)", or return (-1)
 */
static int extract_charges(cptr tail)
{
    /* Require the prefix and suffix */
    if (!prefix(tail, "(")) return (-1); /* --(-- */
    if (!suffix(tail, " charge)") && !suffix(tail, " charges)")) return (-1);

    /* Extract and return the "charges" */
    return (atoi(tail + 1));
}



/*
 * Determine if an item kind is "easy" to identify
 * We can assume that the item kind is known.
 */
static bool obvious_kind(int kind)
{
    /* Analyze the "tval" */
    switch (k_list[kind].tval) {
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
            return (TRUE);
    }

    /* Nope */
    return (FALSE);
}



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
    }

    /* No slot available */
    return (-1);
}




/*
 * Analyze an auto_item based on a description and cost
 *
 * We do a simple binary search on the arrays of object base names,
 * relying on the fact that they are sorted in reverse order, and on
 * the fact that efficiency is only important when the parse succeeds
 * (which it always does), and on some facts about "prefixes".
 *
 * Note that we will fail if the "description" was "partial", though
 * we will correctly handle a description with a "partial inscription",
 * so the actual item description must exceed 75 chars for us to fail,
 * though we will only get 60 characters if the item is in the equipment
 * or in a shop, and the "long" description items tend to get worn.  :-)
 */
void borg_item_analyze(auto_item *item, cptr desc, cptr cost)
{
    int i, j, m, n;

    char *scan;
    char *tail;

    char temp[128];

    char c1 = '{'; /* c2 = '}' */
    char p1 = '(', p2 = ')';
    char b1 = '[', b2 = ']';


    /* Wipe it */
    WIPE(item, auto_item);


    /* Save the item description */
    strcpy(item->desc, desc);

    /* Extract the item cost */
    item->cost = atol(cost);


    /* Advance to the "inscription" or end of string */
    for (scan = item->desc; *scan && *scan != c1; scan++);

    /* Save a pointer to the inscription */
    item->note = scan;


    /* Do not process empty items */
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
        for (s = desc; *s && (*s != ' '); s++);

        /* Paranoia -- Catch sillyness */
        if (*s != ' ') return;

        /* Extract a quantity */
        item->iqty = atoi(desc);

        /* Skip the quantity and space */
        desc = s + 1;
    }


    /* Paranoia -- catch "broken" descriptions */
    if (!desc[0]) return;

    /* Obtain a copy of the description */
    strcpy(temp, desc);

    /* Advance to the "inscription" or end of string */
    for (scan = temp; *scan && (*scan != c1); scan++);

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

        /* Hack -- guess at the weight */
        switch (item->tval) {

            case TV_SCROLL: item->weight = 5; break;
            case TV_POTION: item->weight = 4; break;
            case TV_STAFF: item->weight = 50; break;
            case TV_WAND: item->weight = 10; break;
            case TV_ROD: item->weight = 15; break;
            case TV_RING: item->weight = 2; break;
            case TV_AMULET: item->weight = 3; break;
            case TV_FOOD: item->weight = 1; break;
        }
        
        /* Done */
        return;
    }


    /* Start at the beginning */
    tail = temp;

    /* Check singular items */
    if (item->iqty == 1) {

        /* Start the search */
        m = 0; n = auto_size_single;

        /* Simple binary search */
        while (m < n - 4) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Found a new minimum */
            if (strcmp(tail, auto_text_single[i]) < 0) {
                m = i;
            }

            /* Found a new maximum */
            else {
                n = i;
            }
        }

        /* Search for a prefix */
        for (i = m; i < auto_size_single; i++) {

            /* Check for proper prefix */
            if (prefix(tail, auto_text_single[i])) break;
        }

        /* Oops.  Bizarre item. */
        if (i >= auto_size_single) {
            borg_oops("Bizarre object!");
            return;
        }

        /* Save the item kind */
        item->kind = auto_what_single[i];

        /* Skip past the base name */
        tail += strlen(auto_text_single[i]);
    }

    /* Check plural items */
    else {

        /* Start the search */
        m = 0; n = auto_size_plural;

        /* Simple binary search */
        while (m < n - 4) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Found a new minimum */
            if (strcmp(tail, auto_text_plural[i]) < 0) {
                m = i;
            }

            /* Found a new maximum */
            else {
                n = i;
            }
        }

        /* Search for a prefix */
        for (i = m; i < auto_size_plural; i++) {

            /* Check for proper prefix */
            if (prefix(tail, auto_text_plural[i])) break;
        }

        /* Oops.  Bizarre item. */
        if (i >= auto_size_plural) {
            borg_oops("Bizarre object!");
            return;
        }

        /* Save the item kind */
        item->kind = auto_what_plural[i];

        /* Skip past the base name */
        tail += strlen(auto_text_plural[i]);
    }


    /* Extract some info */
    item->tval = k_list[item->kind].tval;
    item->sval = k_list[item->kind].sval;

    /* Guess at the weight */
    item->weight = k_list[item->kind].weight;
    

    /* Hack -- check for ego-items and artifacts */
    if ((tail[0] == ' ') &&
        (item->tval >= TV_MIN_WEAR) && (item->tval <= TV_MAX_WEAR)) {

        /* Start the search */
        m = 0; n = auto_size_artego;

        /* XXX XXX XXX Binary search */
        while (m < n - 4) {

            /* Pick a "middle" entry */
            i = (m + n) / 2;

            /* Found a new minimum */
            if (strcmp(tail, auto_text_artego[i]) < 0) {
                m = i;
            }

            /* Found a new maximum */
            else {
                n = i;
            }
        }

        /* XXX XXX XXX Search for a prefix */
        for (i = m; i < m + 12; i++) {

            /* Check for proper prefix */
            if (prefix(tail, auto_text_artego[i])) {

                /* Paranoia -- Item is known */
                item->able = TRUE;

                /* Save the artifact name */
                if (auto_what_artego[i] > 0) {
                    item->name1 = auto_what_artego[i];
                }

                /* Save the ego-item name */
                else {
                    item->name2 = 0 - auto_what_artego[i];
                }

                /* Skip the space and the ego-item name */
                tail += strlen(auto_text_artego[i]);

                /* Done */
                break;
            }
        }


        /* Hack -- examine ego-items */
        if (item->name2) {

            /* XXX XXX Hack -- fix weird "missiles" */
            if ((item->tval == TV_BOLT) ||
                (item->tval == TV_ARROW) ||
                (item->tval == TV_SHOT)) {

                /* Fix missile ego-items */
                if (item->name2 == EGO_FIRE) {
                    item->name2 = EGO_AMMO_FIRE;
                }
                else if (item->name2 == EGO_SLAYING) {
                    item->name2 = EGO_AMMO_SLAYING;
                }
                else if (item->name2 == EGO_SLAY_EVIL) {
                    item->name2 = EGO_AMMO_EVIL;
                }
            }

            /* XXX XXX Hack -- fix weird "robes" */
            if ((item->tval == TV_SOFT_ARMOR) &&
                (item->sval == SV_ROBE)) {

                /* Fix "robes of the magi" */
                if (item->name2 == EGO_MAGI) {
                    item->name2 = EGO_ROBE_MAGI;
                }
            }
        }


        /* Hack -- examine artifacts */
        if (item->name1) {

            /* XXX XXX Hack -- fix "weird" artifacts */
            if ((item->tval != v_list[item->name1].tval) ||
                (item->sval != v_list[item->name1].sval)) {

                /* Save the kind */
                item->kind = lookup_kind(item->tval, item->sval);

                /* Save the tval/sval */
                item->tval = k_list[item->kind].tval;
                item->sval = k_list[item->kind].sval;
            }
            
            /* Extract the weight */
            item->weight = v_list[item->name1].weight;
        }
    }


    /* Mega-Hack -- skip spaces */
    while (tail[0] == ' ') tail++;


    /* XXX XXX Hack -- Chests are too complicated */
    if (item->tval == TV_CHEST) {
        return;
    }


    /* Hack -- Some kinds of objects are always obvious */
    if (obvious_kind(item->kind)) {
        item->able = TRUE;
        return;
    }

    /* Hack -- Examine Wands/Staffs for charges */
    if ((item->tval == TV_WAND) || (item->tval == TV_STAFF)) {
        i = extract_charges(tail);
        if (i >= 0) item->able = TRUE;
        if (item->able) item->pval = i;
        return;
    }

    /* Mega-Hack -- Examine Rods for charging */
    if (item->tval == TV_ROD) {

        /* Rods are always known (if aware) */
        item->able = TRUE;

        /* Mega-Hack -- fake "charges" */
        item->pval = 1;

        /* Mega-Hack -- "charging" means no "charges" */
        if (streq(tail, "(charging)")) item->pval = 0;
    }


    /* Hack -- Extract Lites for Light */
    if (item->tval == TV_LITE) {

        /* Fuels yields known (and fuel) */
        i = extract_fuel(tail);
        if (i >= 0) item->pval = i;
        if (i >= 0) item->able = TRUE;

        /* Hack -- Artifacts have infinite fuel */
        if (item->name1) item->pval = 29999;
        if (item->name1) item->able = TRUE;

        /* Done */
        return;
    }


    /* Wearable stuff */
    if ((item->tval >= TV_MIN_WEAR) && (item->tval <= TV_MAX_WEAR)) {

        bool done = FALSE;

        int d1 = 0, d2 = 0, ac = 0, th = 0, td = 0, ta = 0;


        /* Hack -- examine the "easy know" flag */
        if (k_list[item->kind].flags3 & TR3_EASY_KNOW) {
            item->able = TRUE;
        }


        /* Must have a suffix */
        if (!tail[0]) return;


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
            for (scan = tail; *scan != p2; scan++); scan++;

            /* Hack -- Notice "end of string" */
            if (scan[0] != ' ') done = TRUE;

            /* Terminate the string and advance */
            *scan++ = '\0';

            /* Parse the damage string, or stop XXX */
            if (sscanf(tail, "(%dd%d)", &d1, &d2) != 2) return;

            /* Save the values */
            item->dd = d1; item->ds = d2;

            /* No extra information means not identified */
            if (done) return;

            /* Skip the "damage" info */
            tail = scan;
        }

        /* Parse the "damage" string for bows */
        else if ((tail[0] == p1) &&
                 (item->tval == TV_BOW)) {

            /* First extract the damage string */
            for (scan = tail; *scan != p2; scan++); scan++;

            /* Hack -- Notice "end of string" */
            if (scan[0] != ' ') done = TRUE;

            /* Terminate the string and advance */
            *scan++ = '\0';

            /* Parse the multiplier string, or stop */
            if (sscanf(tail, "(x%d)", &d1) != 1) return;

            /* Hack -- save it in "damage dice" */
            item->dd = d1;

            /* No extra information means not identified */
            if (done) return;

            /* Skip the "damage" info */
            tail = scan;
        }


        /* Parse the "bonus" string */
        if (tail[0] == p1) {

            /* Extract the extra info */
            for (scan = tail; *scan != p2; scan++); scan++;

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
                return;
            }

            /* Nothing left */
            if (done) return;

            /* Skip the "damage bonus" info */
            tail = scan;
        }


        /* Parse the "bonus" string */
        if (tail[0] == b1) {

            /* Extract the extra info */
            for (scan = tail; *scan != b2; scan++); scan++;

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
                item->to_a = ta;
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
                return;
            }

            /* Nothing left */
            if (done) return;

            /* Skip the "armor" data */
            tail = scan;
        }


        /* Parse the final "pval" string, if any */
        if (tail[0] == p1) {

            /* Assume identified */
            item->able = TRUE;

            /* Grab it */
            item->pval = atoi(tail + 1);
        }
    }
}



/*
 * This function "guesses" at the "value" of non-aware items
 */
static s32b borg_item_value_base(auto_item *item)
{
    /* Aware items can assume template cost */
    if (item->kind) return (k_list[item->kind].cost);

    /* Unknown food is cheap */
    if (item->tval == TV_FOOD) return (1L);

    /* Unknown Scrolls are cheap */
    if (item->tval == TV_SCROLL) return (20L);

    /* Unknown Potions are cheap */
    if (item->tval == TV_POTION) return (20L);

    /* Unknown Rings are cheap */
    if (item->tval == TV_RING) return (45L);

    /* Unknown Amulets are cheap */
    if (item->tval == TV_AMULET) return (45L);

    /* Unknown Wands are Cheap */
    if (item->tval == TV_WAND) return (50L);

    /* Unknown Staffs are Cheap */
    if (item->tval == TV_STAFF) return (70L);

    /* Unknown Rods are Cheap */
    if (item->tval == TV_ROD) return (75L);

    /* Hack -- Oops */
    return (0L);
}





/*
 * Determine the base price of a known item (see below)
 */
static s32b borg_item_value_known(auto_item *item)
{
    s32b value;


    /* Extract the base value */
    value = k_list[item->kind].cost;

    /* Known worthless items are worthless */
    if (value <= 0L) return (0L);


    /* Hack -- use artifact base costs */
    if (item->name1) value = v_list[item->name1].cost;

    /* Known worthless items are worthless */
    if (value <= 0L) return (0L);


    /* Hack -- catch worthless ego-items */
    if (item->name2 >= EGO_MIN_WORTHLESS) return (0L);

    /* Hack -- add in ego-item bonus cost */
    if (item->name2) value += ego_item_value[item->name2];


    /* Wands/Staffs -- pay extra for charges */
    if ((item->tval == TV_WAND) ||
        (item->tval == TV_STAFF)) {

        /* Reward charges */
        value += ((value / 20) * item->pval);
    }

    /* Rings/Amulets -- pay extra for bonuses */
    else if ((item->tval == TV_RING) ||
             (item->tval == TV_AMULET)) {

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);

        /* XXX XXX Ring of Speed */
    }

    /* Armour -- pay extra for armor bonuses */
    else if ((item->tval == TV_BOOTS) ||
             (item->tval == TV_GLOVES) ||
             (item->tval == TV_CLOAK) ||
             (item->tval == TV_HELM) ||
             (item->tval == TV_CROWN) ||
             (item->tval == TV_SHIELD) ||
             (item->tval == TV_SOFT_ARMOR) ||
             (item->tval == TV_HARD_ARMOR) ||
             (item->tval == TV_DRAG_ARMOR)) {

        /* Hack -- negative armor bonus */
        if (item->to_a < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);
    }

    /* Weapons -- pay extra for all three bonuses */
    else if ((item->tval == TV_DIGGING) ||
             (item->tval == TV_HAFTED) ||
             (item->tval == TV_SWORD) ||
             (item->tval == TV_POLEARM)) {

        /* Allow negatives to be overcome */
        if (item->to_h + item->to_d < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);
    }

    /* Bows -- pay extra for all three bonuses */
    else if (item->tval == TV_BOW) {

        /* Allow negatives to be overcome */
        if (item->to_h + item->to_d < 0) return (0L);

        /* Reward bonuses */
        value += ((item->to_h + item->to_d) * 100L);
        value += ((item->to_a + item->pval) * 100L);
    }

    /* Ammo -- pay extra for all three bonuses. */
    else if ((item->tval == TV_SHOT) ||
             (item->tval == TV_ARROW) ||
             (item->tval == TV_BOLT)) {

        /* Allow negatives to be overcome */
        if (item->to_h + item->to_d < 0) return (0L);

        /* Reward bonuses (value 1/20) */
        value += ((item->to_h + item->to_d) * 5L);
    }


    /* Return the value */
    return (value);
}


/*
 * Determine the approximate "price" of one instance of an item
 * Adapted from "store.c:item_value()" and "object.c:apply_magic()".
 *
 * This routine is used to determine how much gold the Borg would get
 * for selling an item, ignoring "charisma" related issues, and the
 * maximum purchase limits of the various shop-keepers.
 *
 * This function correctly handles artifacts and ego-items, though
 * it will (slightly) undervalue a few ego-items.  This function will
 * not "sufficiently" reward rings/boots of speed, though they are
 * already worth much, much more than any other objects.
 *
 * This function correctly handles "cursed" items, and attempts to
 * apply relevant "discounts" if known.
 *
 * This function is remarkably accurate, considering the complexity...
 */
s32b borg_item_value(auto_item *item)
{
    s32b value;


    /* Non-aware items */
    if (item->kind && item->able) {

        /* Process various fields */
        value = borg_item_value_known(item);
    }

    /* Known items */
    else {

        /* Do what "store.c" does */
        value = borg_item_value_base(item);
    }

    /* Worthless items */
    if (value <= 0L) return (0L);


    /* Parse various "inscriptions" */
    if (item->note[0]) {

        /* Cursed indicators */
        if (streq(item->note, "{cursed}")) return (0L);
        if (streq(item->note, "{broken}")) return (0L);
        if (streq(item->note, "{terrible}")) return (0L);
        if (streq(item->note, "{worthless}")) return (0L);

        /* Ignore certain feelings */
        /* "{average}" */
        /* "{blessed}" */
        /* "{good}" */
        /* "{excellent}" */
        /* "{special}" */

        /* Ignore special inscriptions */
        /* "{empty}", "{tried}" */

        /* Special "discount" */
        if (streq(item->note, "{on sale}")) item->discount = 50;

        /* Standard "discounts" */
        else if (streq(item->note, "{25% off}")) item->discount = 25;
        else if (streq(item->note, "{50% off}")) item->discount = 50;
        else if (streq(item->note, "{75% off}")) item->discount = 75;
        else if (streq(item->note, "{90% off}")) item->discount = 90;
    }


    /* Apply "discount" if any */
    if (item->discount) value -= value * item->discount / 100;


    /* Return the value */
    return (value);
}






/*
 * Hack -- determine if an item is "armor"
 */
bool borg_item_is_armour(auto_item *item)
{
    /* Check for armor */
    if ((item->tval == TV_DRAG_ARMOR) ||
        (item->tval == TV_HARD_ARMOR) ||
        (item->tval == TV_SOFT_ARMOR) ||
        (item->tval == TV_SHIELD) ||
        (item->tval == TV_CROWN) ||
        (item->tval == TV_HELM) ||
        (item->tval == TV_CLOAK) ||
        (item->tval == TV_GLOVES) ||
        (item->tval == TV_BOOTS)) {

        /* Yep */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Hack -- determine if an item is a "weapon"
 */
bool borg_item_is_weapon(auto_item *item)
{
    /* Check for weapon */
    if ((item->tval == TV_SWORD) ||
        (item->tval == TV_HAFTED) ||
        (item->tval == TV_POLEARM) ||
        (item->tval == TV_DIGGING) ||
        (item->tval == TV_BOW) ||
        (item->tval == TV_BOLT) ||
        (item->tval == TV_ARROW) ||
        (item->tval == TV_SHOT)) {

        /* Yep */
        return (TRUE);
    }

    /* Nope */
    return (FALSE);
}




/*
 * Hack -- determine number of blows with a given weapon
 */
int borg_blows(auto_item *item)
{
    int b, str_index, dex_index;

    int num = 0, wgt = 0, mul = 0, div = 0;
        
    /* Analyze the class */
    switch (auto_class) {
        
        /* Warrior */
        case 0: num = 6; wgt = 30; mul = 5; break;

        /* Mage */
        case 1: num = 4; wgt = 40; mul = 2; break;

        /* Priest (was mul = 3.5) */
        case 2: num = 5; wgt = 35; mul = 3; break;

        /* Rogue */
        case 3: num = 5; wgt = 30; mul = 3; break;

        /* Ranger */
        case 4: num = 5; wgt = 35; mul = 4; break;

        /* Paladin */
        case 5: num = 5; wgt = 30; mul = 4; break;
    }

    /* Enforce a minimum "weight" */
    div = ((item->weight < wgt) ? wgt : item->weight);
        
    /* Access the strength vs weight */
    str_index = (adj_str_blow[borg_stat_index(A_STR)] * mul / div);

    /* Maximal value */
    if (str_index > 11) str_index = 11;
        
    /* Index by dexterity */
    dex_index = (adj_dex_blow[borg_stat_index(A_DEX)]);

    /* Maximal value */
    if (dex_index > 11) dex_index = 11;
        
    /* Use the blows table */
    b = blows_table[str_index][dex_index];

    /* Maximal value */
    if (b > num) b = num;

    /* Add in the "bonus blows" */
    if ((item->name2 == EGO_ATTACKS) ||
        (v_list[item->name1].flags1 & TR1_BLOWS)) {

        /* Extra blows */
        b += item->pval;
    }
    
    /* Require at least one blow */
    if (b < 1) b = 1;

    /* Return the blows */
    return (b);
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
        borg_keypress('a' + i);
    }
    
    /* Choose from equipment */
    else {
    
        /* Go to equipment (if necessary) */
        if (auto_items[0].iqty) borg_keypress('/');

        /* Choose the item */
        borg_keypress('a' + i - INVEN_WIELD);
    }

    /* Send the label */
    for (s = str; *s; s++) borg_keypress(*s);

    /* End the inscription */
    borg_keypress('\n');
}



/*
 * Choose the "best" pile of items of the given kind
 * Hack -- prefer the last pile, to use up discounted items
 */
int borg_choose(int k)
{
    int i, n = -1;

    /* Nothing available */
    /* if (kind_have[k] <= 0) return (n); */
    
    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

        /* Require that kind */
        if (item->kind != k) continue;

        /* Save the last "best" index (smallest pile) */
        if ((n < 0) || (item->iqty <= auto_items[n].iqty)) n = i;
    }

    /* Result */
    return (n);
}


/*
 * Hack -- perform an action on the "best" pile of items of the given "kind"
 */
bool borg_action(char c, int k)
{
    int i;

    /* Choose a usable item */
    i = borg_choose(k);

    /* Nothing to use */
    if (i < 0) return (FALSE);

    /* Log the message */
    borg_note(format("Action '%c' on item %s.", c, auto_items[i].desc));

    /* Perform the action */
    borg_keypress(c);
    borg_keypress('a' + i);

    /* Success */
    return (TRUE);
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
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];

        /* Notice end of inventory */
        if (!item->iqty) break;

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
    borg_note(format("Reading %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('r');
    borg_keypress('a' + i);

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
    borg_note(format("Quaffing %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('q');
    borg_keypress('a' + i);

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
    borg_note(format("Zapping %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('z');
    borg_keypress('a' + i);

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
    borg_note(format("Aiming %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('a');
    borg_keypress('a' + i);

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
    borg_note(format("Using %s.", auto_items[i].desc));

    /* Perform the action */
    borg_keypress('a');
    borg_keypress('a' + i);

    /* Success */
    return (TRUE);
}




/*
 * Find the slot of the given "usable" book, if available
 * Note that the "size" of the pile is not important (?)
 * Assumes that usable books always appear first in inventory.
 */
int borg_book(int book)
{
    int i;

    /* Must have a book */
    if (!mb_ptr->spell_book) return (-1);

    /* Scan the pack */
    for (i = 0; i < INVEN_PACK; i++) {

        auto_item *item = &auto_items[i];
    
        /* Notice end of usable books */
        if (item->tval != mb_ptr->spell_book) break;

        /* Found it */
        if (item->sval == book) return (i);
    }
    
    /* No luck */
    return (-1);
}


/*
 * Determine if borg can cast a given spell
 */
bool borg_spell_okay(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];

    /* The borg must be able to "cast" spells */
    if (mb_ptr->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);
        
    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The spell must be "known" */
    if (as->status < AUTO_SPELL_TEST) return (FALSE);

    /* The spell must be affordable */
    if (as->power > auto_csp) return (FALSE);
    
    /* Success */
    return (TRUE);
}

/*
 * Attempt to cast a spell
 */
bool borg_spell(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];

    /* The borg must be able to "cast" spells */
    if (mb_ptr->spell_book != TV_MAGIC_BOOK) return (FALSE);

    /* Look for the book */
    i = borg_book(book);
        
    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The spell must be "known" */
    if (as->status < AUTO_SPELL_TEST) return (FALSE);

    /* The spell must be affordable */
    if (as->power > auto_csp) return (FALSE);

    /* Debugging Info */
    borg_note(format("Casting %s.", spell_names[0][as->index]));

    /* Cast a spell */
    borg_keypress('m');
    borg_keypress('a' + i);
    borg_keypress('a' + what);
    
    /* Success */
    return (TRUE);
}



/*
 * Determine if borg can pray a given prayer
 */
bool borg_prayer_okay(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];

    /* The borg must be able to "pray" prayers */
    if (mb_ptr->spell_book != TV_PRAYER_BOOK) return (FALSE);
    
    /* Look for the book */
    i = borg_book(book);
        
    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The prayer must be "known" */
    if (as->status < AUTO_SPELL_TEST) return (FALSE);

    /* The prayer must be affordable */
    if (as->power > auto_csp) return (FALSE);
    
    /* Success */
    return (TRUE);
}

/*
 * Attempt to pray a prayer
 */
bool borg_prayer(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];

    /* The borg must be able to "pray" prayers */
    if (mb_ptr->spell_book != TV_PRAYER_BOOK) return (FALSE);
    
    /* Look for the book */
    i = borg_book(book);
        
    /* The book must be possessed */
    if (i < 0) return (FALSE);

    /* The prayer must be "known" */
    if (as->status < AUTO_SPELL_TEST) return (FALSE);

    /* The prayer must be affordable */
    if (as->power > auto_csp) return (FALSE);

    /* Debugging Info */
    borg_note(format("Praying %s.", spell_names[1][as->index]));

    /* Pray a prayer */
    borg_keypress('p');
    borg_keypress('a' + i);
    borg_keypress('a' + what);
    
    /* Success */
    return (TRUE);
}



/*
 * Determine if the Borg may "study" a given spell
 */
bool borg_study_spell_okay(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];
    
    /* Paranoia -- Warriors never learn spells */
    if (auto_class == 0) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for the book */
    i = borg_book(book);
        
    /* Make sure we have the book */
    if (i < 0) return (FALSE);

    /* Skip "non-learnable" spells */
    if (as->status != AUTO_SPELL_OKAY) return (FALSE);

    /* Sure */
    return (TRUE);
}


/*
 * Attempt to "study" the given spell
 */
bool borg_study_spell(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];
    
    /* Paranoia -- Warriors never learn spells */
    if (auto_class == 0) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for the book */
    i = borg_book(book);
        
    /* Make sure we have the book */
    if (i < 0) return (FALSE);

    /* Skip "non-learnable" spells */
    if (as->status != AUTO_SPELL_OKAY) return (FALSE);

    /* Debugging Info */
    borg_note(format("Studying Spell %s.", spell_names[0][as->index]));

    /* Learn the spell */
    borg_keypress('G');
            
    /* Specify the book */
    borg_keypress('a' + i);
            
    /* Specify the spell */
    borg_keypress('a' + what);
            
    /* Success */
    return (TRUE);
}



/*
 * Determine if the Borg may "study" a given prayer
 */
bool borg_study_prayer_okay(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];
    
    /* Paranoia -- Warriors never learn spells */
    if (auto_class == 0) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for the book */
    i = borg_book(book);
        
    /* Make sure we have the book */
    if (i < 0) return (FALSE);

    /* Skip "non-learnable" spells */
    if (as->status != AUTO_SPELL_OKAY) return (FALSE);

    /* Sure */
    return (TRUE);
}


/*
 * Attempt to "study" the given prayer
 */
bool borg_study_prayer(int book, int what)
{
    int i;
    
    auto_spell *as = &auto_spells[book][what];
    
    /* Paranoia -- Warriors never learn spells */
    if (auto_class == 0) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Look for the book */
    i = borg_book(book);
        
    /* Make sure we have the book */
    if (i < 0) return (FALSE);

    /* Skip "non-learnable" spells */
    if (as->status != AUTO_SPELL_OKAY) return (FALSE);

    /* Debugging Info */
    borg_note(format("Studying Prayer %s.", spell_names[1][as->index]));

    /* Learn the spell */
    borg_keypress('G');
            
    /* Specify the book */
    borg_keypress('a' + i);
            
    /* Specify the prayer (not!) */
    /* borg_keypress('a' + what); */
            
    /* Success */
    return (TRUE);
}



/*
 * Determine if the Borg may "study" new spells/prayers
 */
bool borg_study_any_okay(void)
{
    int i, book, what;
    
    /* Paranoia -- Warriors never learn spells */
    if (auto_class == 0) return (FALSE);

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
    
            auto_spell *as = &auto_spells[book][what];
    
            /* Notice "learnable" spells */
            if (as->status == AUTO_SPELL_OKAY) return (TRUE);
        }
    }

    /* Nope */
    return (FALSE);
}


/*
 * Attempt to "study" a prayer
 */
bool borg_study_any(void)
{
    int i, book, what;

    /* Paranoia -- Warriors never learn spells */
    if (auto_class == 0) return (FALSE);

    /* Access the "study" flag */
    if (!do_study) return (FALSE);

    /* Hack -- blind/confused */
    if (do_blind || do_confused) return (FALSE);

    /* Check each book (backwards) */
    for (book = 8; book >= 0; book--) {

        /* Look for the book */
        i = borg_book(book);
        
        /* Make sure we have the book */
        if (i < 0) continue;

        /* Check for spells */
        for (what = 0; what < 9; what++) {

            auto_spell *as = &auto_spells[book][what];

            /* Notice "learnable" spells */
            if (as->status != AUTO_SPELL_OKAY) continue;

            /* Study a spell */
            if (mb_ptr->spell_book == TV_MAGIC_BOOK) {

                /* Debugging Info */
                borg_note(format("Studying Spell %s.", spell_names[0][as->index]));

                /* Learn a spell/prayer */
                borg_keypress('G');
            
                /* Specify the book */
                borg_keypress('a' + i);
            
                /* Specify the spell */
                borg_keypress('a' + what);
            }

            /* Study a prayer */
            if (mb_ptr->spell_book == TV_PRAYER_BOOK) {

                /* Debugging Info */
                borg_note(format("Studying Prayer %s.", spell_names[1][as->index]));

                /* Learn a spell/prayer */
                borg_keypress('G');
            
                /* Specify the book */
                borg_keypress('a' + i);
            
                /* Specify the prayer (not!) */
                /* borg_keypress('a' + what); */
            }

            /* Success */
            return (TRUE);
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
        borg_item_analyze(&auto_items[i], buf, "");
    }
}


/*
 * Cheat the "inven" screen
 */
void borg_cheat_inven(void)
{
    int i;
    
    char buf[256];

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
        borg_item_analyze(&auto_items[i], buf, "");
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
            (buf[0] == 'a') && (buf[1] == p2) && (buf[2] == ' ')) {

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
            (buf[0] == 'a' + row) && (buf[1] == p2) && (buf[2] == ' ') &&
            (0 == borg_what_text(col+19, row+1, -80, &t_a, buf)) &&
            (buf[0] && (buf[0] != ' '))) {
            
            /* XXX Strip final spaces */
        }

        /* Default to "nothing" */
        else {
            buf[0] = '\0';
            done = TRUE;
        }

        /* Use the nice "show_empty_slots" flag */
        if (streq(buf, "(nothing)")) strcpy(buf, "");

        /* Ignore "unchanged" items */
        if (streq(buf, auto_items[i].desc)) continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&auto_items[i], buf, "");
    }
}


/*
 * Parse the "inven" screen
 */
void borg_parse_inven(void)
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
            (buf[0] == 'a') && (buf[1] == p2) && (buf[2] == ' ')) {

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
            (buf[0] == 'a' + row) && (buf[1] == p2) && (buf[2] == ' ') &&
            (0 == borg_what_text(col+3, row+1, -80, &t_a, buf)) &&
            (buf[0] && (buf[0] != ' '))) {

            /* XXX Strip final spaces */
        }

        /* Default to "nothing" */
        else {
            buf[0] = '\0';
            done = TRUE;
        }

        /* Use the nice "show_empty_slots" flag */
        /* if (streq(buf, "(nothing)")) strcpy(buf, ""); */

        /* Ignore "unchanged" items */
        if (streq(buf, auto_items[i].desc)) continue;

        /* Analyze the item (no price) */
        borg_item_analyze(&auto_items[i], buf, "");
    }
}







/*
 * Hack -- Cheat the "spell" info (given the book)
 */
void borg_cheat_spell(int book)
{
    int j, what;
    int num = 0, spell[64];

    u32b j1, j2;

    /* Hack -- no spells for warriors */
    if (auto_class == 0) return;
    
    /* Observe spells in that book */
    j1 = spell_flags[mb_ptr->spell_type][book][0];
    j2 = spell_flags[mb_ptr->spell_type][book][1];

    /* Extract spells */
    while (j1) spell[num++] = bit_pos(&j1);
    while (j2) spell[num++] = bit_pos(&j2) + 32;

    /* Process the spells */
    for (what = 0; what < num; what++) {

        /* Access the spell */
        auto_spell *as = &auto_spells[book][what];
        
        /* Skip illegible spells */
        if (as->status == AUTO_SPELL_ICKY) continue;

        /* Access the index */
        j = as->index;
        
        /* Note "forgotten" spells */
        if ((j < 32) ?
            ((spell_forgotten1 & (1L << j))) :
            ((spell_forgotten2 & (1L << (j - 32))))) {

            /* Forgotten */
            as->status = AUTO_SPELL_LOST;
        }

        /* Note "difficult" spells */
        else if (auto_lev < as->level) {

            /* Unknown */
            as->status = AUTO_SPELL_HIGH;
        }
        
        /* Note "unknown" spells */
        else if (!((j < 32) ?
                   (spell_learned1 & (1L << j)) :
                   (spell_learned2 & (1L << (j - 32))))) {

            /* Unknown */
            as->status = AUTO_SPELL_OKAY;
        }

        /* Note "untried" spells */
        else if (!((j < 32) ?
                   (spell_worked1 & (1L << j)) :
                   (spell_worked2 & (1L << (j - 32))))) {

            /* Untried */
            as->status = AUTO_SPELL_TEST;
        }

        /* Note "known" spells */
        else {

            /* Known */
            as->status = AUTO_SPELL_KNOW;
        }
    }
}





/*
 * Hack -- Parse the "spell" info (given the book)
 */
void borg_parse_spell(int book)
{
    int j, what;
    int num = 0, spell[64];

    u32b j1, j2;

    byte t_a;
    
    char buf[160];
   

    /* Hack -- no spells for warriors */
    if (auto_class == 0) return;
    
    /* Observe spells in that book */
    j1 = spell_flags[mb_ptr->spell_type][book][0];
    j2 = spell_flags[mb_ptr->spell_type][book][1];

    /* Extract spells */
    while (j1) spell[num++] = bit_pos(&j1);
    while (j2) spell[num++] = bit_pos(&j2) + 32;

    /* Process the spells */
    for (what = 0; what < num; what++) {

        int row = ROW_SPELL + 1 + what;
        int col = COL_SPELL;
        
        /* Access the spell */
        auto_spell *as = &auto_spells[book][what];
        
        /* Skip illegible spells */
        if (as->status == AUTO_SPELL_ICKY) continue;

        /* Access the index */
        j = as->index;

#if 0
        /* Format: "spell-name...................." at col 20+5 */
        if (0 != borg_what_text_hack(col-30, row, -30, &t_a, buf)) continue;
#endif

        /* Format: "Lv Mana Freq Comment" at col 20+35 */
        if (0 != borg_what_text_hack(col, row, -20, &t_a, buf)) continue;

        /* Note "forgotten" spells */
        if (prefix(buf + 13, "forgott")) {

            /* Forgotten */
            as->status = AUTO_SPELL_LOST;
        }

        /* Note "difficult" spells */
        else if (auto_lev < as->level) {

            /* Unknown */
            as->status = AUTO_SPELL_HIGH;
        }
        
        /* Note "unknown" spells */
        else if (prefix(buf + 13, "unknown")) {

            /* Unknown */
            as->status = AUTO_SPELL_OKAY;
        }

        /* Note "untried" spells */
        else if (prefix(buf + 13, "untried")) {

            /* Untried */
            as->status = AUTO_SPELL_TEST;
        }

        /* Note "known" spells */
        else {

            /* Known */
            as->status = AUTO_SPELL_KNOW;
        }
    }
}




/*
 * Hack -- prepare some stuff based on the player race and class
 */
void prepare_race_class_info(void)
{
    int book, what;
        

    /* Initialize the spell arrays */
    for (book = 0; book < 9; book++) {

        int spell[64], num = 0;
        
        u32b j1, j2;
        
        
        /* Reset each spell entry */
        for (what = 0; what < 9; what++) {

            auto_spell *as = &auto_spells[book][what];
            
            /* Assume spell is icky */
            as->status = AUTO_SPELL_ICKY;

            /* Impossible values */
            as->index = 99;

            /* Impossible values */
            as->level = 99;
            as->power = 99;
        }


        /* No spells for warriors */
        if (auto_class == 0) continue;
        
        
        /* Observe spells in that book */
        j1 = spell_flags[mb_ptr->spell_type][book][0];
        j2 = spell_flags[mb_ptr->spell_type][book][1];

        /* Extract spells into an array */
        while (j1) spell[num++] = bit_pos(&j1);
        while (j2) spell[num++] = bit_pos(&j2) + 32;

        /* Process each spell */
        for (what = 0; what < num; what++) {

            auto_spell *as = &auto_spells[book][what];
            
            magic_type *s_ptr = &mb_ptr->info[spell[what]];
            
            /* Skip "illegible" spells */
            if (s_ptr->slevel == 99) continue;

            /* Default to "spell is hard to learn" */
            as->status = AUTO_SPELL_HIGH;

            /* Save the spell index */
            as->index = spell[what];

            /* Extract the level and power */
            as->level = s_ptr->slevel;
            as->power = s_ptr->smana;
        }


        /* Obtain the "kind" index for the book */
        kind_book[book] = lookup_kind(mb_ptr->spell_book, book);
    }
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
 */
void borg_obj_init(void)
{
    int i, j, k, n;

    int size;

    sint what[512];
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
    for (k = 0; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_list[k];

        /* Skip non-items */
        if (!k_ptr->name) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval >= TV_GOLD) continue;

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

    /* Sort entries (in reverse order) by text */
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {

            int i1 = j;
            int i2 = j + 1;

            s16b k1 = what[i1];
            s16b k2 = what[i2];

            cptr t1 = text[i1];
            cptr t2 = text[i2];

            /* Enforce (reverse) order */
            if (strcmp(t1, t2) < 0) {

                /* Swap "kind" */
                what[i1] = k2;
                what[i2] = k1;

                /* Swap "text" */
                text[i1] = t2;
                text[i2] = t1;
            }
        }
    }

    /* Save the size */
    auto_size_plural = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_what_plural, auto_size_plural, s16b);
    C_MAKE(auto_text_plural, auto_size_plural, cptr);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_text_plural[i] = text[i];
    for (i = 0; i < size; i++) auto_what_plural[i] = what[i];


    /*** Singular Object Templates ***/

    /* Start with no objects */
    size = 0;

    /* Analyze some "item kinds" */
    for (k = 0; k < MAX_K_IDX; k++) {

        inven_type hack;

        /* Get the kind */
        inven_kind *k_ptr = &k_list[k];

        /* Skip non-items */
        if (!k_ptr->name) continue;

        /* Skip "dungeon terrain" objects */
        if (k_ptr->tval >= TV_GOLD) continue;

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
    for (i = 1; i < ART_MAX; i++) {

        inven_type hack;

        inven_very *v_ptr = &v_list[i];
        
        /* Skip non-items */
        if (!v_ptr->name) continue;

        /* Skip non INSTA_ART things */
        if (!(v_ptr->flags3 & TR3_INSTA_ART)) continue;

        /* Extract the "kind" */
        k = lookup_kind(v_ptr->tval, v_ptr->sval);
        
        /* Hack -- make an item */
        invcopy(&hack, k);

        /* Save the index */
        hack.name1 = i;

        /* Describe a "singular" object */
        hack.number = 1;
        objdes_store(buf, &hack, FALSE, 0);

        /* Extract the "suffix" length */
        n = strlen(v_ptr->name) + 1;

        /* Remove the "suffix" */
        buf[strlen(buf) - n] = '\0';

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Sort entries (in reverse order) by text */
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {

            int i1 = j;
            int i2 = j + 1;

            s16b k1 = what[i1];
            s16b k2 = what[i2];

            cptr t1 = text[i1];
            cptr t2 = text[i2];

            /* Enforce (reverse) order */
            if (strcmp(t1, t2) < 0) {

                /* Swap "kind" */
                what[i1] = k2;
                what[i2] = k1;

                /* Swap "text" */
                text[i1] = t2;
                text[i2] = t1;
            }
        }
    }

    /* Save the size */
    auto_size_single = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_what_single, auto_size_single, s16b);
    C_MAKE(auto_text_single, auto_size_single, cptr);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_text_single[i] = text[i];
    for (i = 0; i < size; i++) auto_what_single[i] = what[i];


    /*** Artifact and Ego-Item Parsers ***/

    /* No entries yet */
    size = 0;

    /* Collect the "artifact names" */
    for (k = 1; k < ART_MAX; k++) {

        /* Skip non-items */
        if (!v_list[k].name) continue;

        /* Extract a string */
        sprintf(buf, " %s", v_list[k].name);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = k;
        size++;
    }

    /* Collect the "ego-item names" */
    for (k = 1; k < EGO_MAX; k++) {

        /* Skip non-items */
        if (!ego_item_names[k]) continue;

        /* Extract a string */
        sprintf(buf, " %s", ego_item_names[k]);

        /* Save an entry */
        text[size] = string_make(buf);
        what[size] = 0 - k;
        size++;
    }

    /* Sort entries (in reverse order) by text */
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - 1; j++) {

            int i1 = j;
            int i2 = j + 1;

            s16b k1 = what[i1];
            s16b k2 = what[i2];

            cptr t1 = text[i1];
            cptr t2 = text[i2];

            /* Enforce (reverse) order */
            if (strcmp(t1, t2) < 0) {

                /* Swap "kind" */
                what[i1] = k2;
                what[i2] = k1;

                /* Swap "text" */
                text[i1] = t2;
                text[i2] = t1;
            }
        }
    }

    /* Save the size */
    auto_size_artego = size;

    /* Allocate the "item parsing arrays" (plurals) */
    C_MAKE(auto_what_artego, auto_size_artego, s16b);
    C_MAKE(auto_text_artego, auto_size_artego, cptr);

    /* Save the entries */
    for (i = 0; i < size; i++) auto_text_artego[i] = text[i];
    for (i = 0; i < size; i++) auto_what_artego[i] = what[i];
}




#endif

