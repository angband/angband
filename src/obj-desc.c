/* File: obj-desc.c */

/* Purpose: handle object descriptions, mostly string handling code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * There is a new metaphor for "store made objects".  Note that now that
 * Angband uses real physical colors for objects on the floor, once a player
 * knows the physical color of something, and what it does, then he can extract
 * an awareness of its identity.  Thus, we no longer try to "hide" the identity
 * of "store bought items" from the player.  So whenever an item is bought from
 * a store, the player becomes "aware" of its identity (as well as knowing the
 * object fully, via "inven_known()".  But when items are listed in the store, before
 * the player is "aware" of its effect, we do not want the "flavors" to appear.
 * This is accomplished by "objdes_store()" below, which is a hack, oh well.
 */


/*
 * XXX XXX Hack -- note that "TERM_MULTI" is now just "TERM_VIOLET"
 * We will have to find a cleaner method for "MULTI_HUED" later.
 * There were only two multi-hued "flavors" (one potion, one food).
 * Plus five multi-hued "base-objects" (3 dragon scales, one blade
 * of chaos, and one something else).
 */
#define TERM_MULTI	TERM_VIOLET


/*
 * Max sizes of the following arrays
 */
#define MAX_COLORS     57       /* Used with potions      */
#define MAX_SHROOM     21       /* Used with mushrooms    */
#define MAX_WOODS      32       /* Used with staffs       */
#define MAX_METALS     32       /* Used with wands/rods   */
#define MAX_ROCKS      42       /* Used with rings        */
#define MAX_AMULETS    16       /* Used with amulets      */
#define MAX_TITLES     45       /* Used with scrolls      */
#define MAX_SYLLABLES 158       /* Used with scrolls      */


/*
 * Color adjectives and colors, for potions.
 * Hack -- The first three are hard-coded for slime mold juice,
 * apple juice, and water, so do not scramble them.
 */

static cptr potion_adj[MAX_COLORS] = {
    "Icky Green","Light Brown","Clear","Azure","Blue",
    "Blue Speckled","Black","Brown","Brown Speckled","Bubbling",
    "Chartreuse","Cloudy","Copper Speckled","Crimson","Cyan",
    "Dark Blue","Dark Green","Dark Red","Gold Speckled","Green",
    "Green Speckled","Grey","Grey Speckled","Hazy","Indigo",
    "Light Blue","Light Green","Magenta","Metallic Blue","Metallic Red",
    "Metallic Green","Metallic Purple","Misty","Orange","Orange Speckled",
    "Pink","Pink Speckled","Puce","Purple","Purple Speckled",
    "Red","Red Speckled","Silver Speckled","Smoky","Tangerine",
    "Violet","Vermilion","White","Yellow", "Purple Speckled",
    "Pungent","Clotted Red","Viscous Pink","Oily Yellow","Gloopy Green",
    "Shimmering","Coagulated Crimson"
};

static byte potion_col[MAX_COLORS] = {
    TERM_GREEN,TERM_L_UMBER,TERM_WHITE,TERM_L_BLUE,TERM_BLUE,
    TERM_BLUE,TERM_D_GRAY,TERM_UMBER,TERM_UMBER,TERM_L_GRAY,
    TERM_L_GREEN,TERM_WHITE,TERM_L_UMBER,TERM_RED,TERM_L_BLUE,
    TERM_BLUE,TERM_GREEN,TERM_RED,TERM_YELLOW,TERM_GREEN,
    TERM_GREEN,TERM_GRAY,TERM_GRAY,TERM_L_GRAY,TERM_VIOLET,
    TERM_L_BLUE,TERM_L_GREEN,TERM_RED,TERM_BLUE,TERM_RED,
    TERM_GREEN,TERM_VIOLET,TERM_L_GRAY,TERM_ORANGE,TERM_ORANGE,
    TERM_L_RED,TERM_L_RED,TERM_VIOLET,TERM_VIOLET,TERM_VIOLET,
    TERM_RED,TERM_RED,TERM_L_GRAY,TERM_D_GRAY,TERM_ORANGE,
    TERM_VIOLET,TERM_RED,TERM_WHITE,TERM_YELLOW,TERM_VIOLET,
    TERM_L_RED,TERM_RED,TERM_L_RED,TERM_YELLOW,TERM_GREEN,
    TERM_MULTI,TERM_RED
};


/*
 * Color adjectives and colors, for mushrooms and molds
 */

static cptr food_adj[MAX_SHROOM] = {
    "Blue","Black","Black Spotted","Brown","Dark Blue",
    "Dark Green","Dark Red","Ecru","Furry","Green",
    "Grey","Light Blue","Light Green","Plaid","Red",
    "Slimy","Tan","White","White Spotted","Wooden",
    "Wrinkled",/*"Yellow","Shaggy","Red Spotted","Pale Blue","Dark Orange"*/
};

static byte food_col[MAX_SHROOM] = {
    TERM_BLUE,TERM_D_GRAY,TERM_D_GRAY,TERM_UMBER,TERM_BLUE,
    TERM_GREEN,TERM_RED,TERM_YELLOW,TERM_L_GRAY,TERM_GREEN,
    TERM_GRAY,TERM_L_BLUE,TERM_L_GREEN,TERM_MULTI,TERM_RED,
    TERM_GRAY,TERM_L_UMBER,TERM_WHITE,TERM_WHITE,TERM_UMBER,
    TERM_UMBER,/*TERM_YELLOW,???,TERM_RED,TERM_L_BLUE,TERM_ORANGE*/
};


/*
 * Wood adjectives and colors, for staffs
 */

static cptr staff_adj[MAX_WOODS] = {
    "Aspen","Balsa","Banyan","Birch","Cedar",
    "Cottonwood","Cypress","Dogwood","Elm","Eucalyptus",
    "Hemlock","Hickory","Ironwood","Locust","Mahogany",
    "Maple","Mulberry","Oak","Pine","Redwood",
    "Rosewood","Spruce","Sycamore","Teak","Walnut",
    "Mistletoe","Hawthorn","Bamboo","Silver","Runed",
    "Golden","Ashen"/*,"Gnarled","Ivory","Decorative","Willow"*/
};

static byte staff_col[MAX_WOODS] = {
    TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_L_UMBER,TERM_L_UMBER,TERM_UMBER,TERM_L_UMBER,TERM_UMBER,
    TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_RED,
    TERM_RED,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_UMBER,
    TERM_GREEN,TERM_L_UMBER,TERM_L_UMBER,TERM_L_GRAY,TERM_UMBER,
    TERM_YELLOW,TERM_GRAY,/*???,???,???,???*/
};


/*
 * Metal adjectives and colors, for wands
 */

static cptr wand_adj[MAX_METALS] = {
    "Aluminum","Cast Iron","Chromium","Copper","Gold",
    "Iron","Magnesium","Molybdenum","Nickel","Rusty",
    "Silver","Steel","Tin","Titanium","Tungsten",
    "Zirconium","Zinc","Aluminum-Plated","Copper-Plated","Gold-Plated",
    "Nickel-Plated","Silver-Plated","Steel-Plated","Tin-Plated","Zinc-Plated",
    "Mithril-Plated","Mithril","Runed","Bronze","Brass",
    "Platinum","Lead"/*,"Lead-Plated","Ivory","Pewter"*/
};

static byte wand_col[MAX_METALS] = {
    TERM_L_BLUE,TERM_D_GRAY,TERM_WHITE,TERM_L_UMBER,TERM_YELLOW,
    TERM_GRAY,TERM_L_GRAY,TERM_L_GRAY,TERM_L_UMBER,TERM_RED,
    TERM_L_GRAY,TERM_L_GRAY,TERM_L_GRAY,TERM_WHITE,TERM_WHITE,
    TERM_L_GRAY,TERM_L_GRAY,TERM_L_BLUE,TERM_L_UMBER,TERM_YELLOW,
    TERM_L_UMBER,TERM_L_GRAY,TERM_L_GRAY,TERM_L_GRAY,TERM_L_GRAY,
    TERM_L_BLUE,TERM_L_BLUE,TERM_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_WHITE,TERM_GRAY,/*TERM_GRAY,TERM_WHITE,TERM_GRAY*/
};


/*
 * Another copy of the metal adjectives and colors, for rods
 * We do not want rods and wands to be identical, its too easy to "cheat".
 * However, the two arrays start out identical.  They are scrambled later.
 * We could actually copy them directly, but this lets us change the
 * bounds on the arrays, and change the distributions, etc.
 */

static cptr rod_adj[MAX_METALS] = {
    "Aluminum","Cast Iron","Chromium","Copper","Gold",
    "Iron","Magnesium","Molybdenum","Nickel","Rusty",
    "Silver","Steel","Tin","Titanium","Tungsten",
    "Zirconium","Zinc","Aluminum-Plated","Copper-Plated","Gold-Plated",
    "Nickel-Plated","Silver-Plated","Steel-Plated","Tin-Plated","Zinc-Plated",
    "Mithril-Plated","Mithril","Runed","Bronze","Brass",
    "Platinum","Lead"/*,"Lead-Plated","Ivory","Pewter"*/
};

static byte rod_col[MAX_METALS] = {
    TERM_L_BLUE,TERM_D_GRAY,TERM_WHITE,TERM_L_UMBER,TERM_YELLOW,
    TERM_GRAY,TERM_L_GRAY,TERM_L_GRAY,TERM_L_UMBER,TERM_RED,
    TERM_L_GRAY,TERM_L_GRAY,TERM_L_GRAY,TERM_WHITE,TERM_WHITE,
    TERM_L_GRAY,TERM_L_GRAY,TERM_L_BLUE,TERM_L_UMBER,TERM_YELLOW,
    TERM_L_UMBER,TERM_L_GRAY,TERM_L_GRAY,TERM_L_GRAY,TERM_L_GRAY,
    TERM_L_BLUE,TERM_L_BLUE,TERM_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_WHITE,TERM_GRAY,/*TERM_GRAY,TERM_WHITE,TERM_GRAY*/
};


/*
 * Rock adjectives and colors, for rings
 */

static cptr ring_adj[MAX_ROCKS] = {
    "Alexandrite","Amethyst","Aquamarine","Azurite","Beryl",
    "Bloodstone","Calcite","Carnelian","Corundum","Diamond",
    "Emerald","Fluorite","Garnet","Granite","Jade",
    "Jasper","Lapis Lazuli","Malachite","Marble","Moonstone",
    "Onyx","Opal","Pearl","Quartz","Quartzite",
    "Rhodonite","Ruby","Sapphire","Tiger Eye","Topaz",
    "Turquoise","Zircon","Platinum","Bronze","Gold",
    "Obsidian","Silver","Tortoise Shell","Mithril","Jet",
    "Engagement","Adamantite"
};

static byte ring_col[MAX_ROCKS] = {
    TERM_GREEN,TERM_VIOLET,TERM_L_BLUE,TERM_L_BLUE,TERM_L_GREEN,
    TERM_RED,TERM_WHITE,TERM_RED,TERM_GRAY,TERM_WHITE,
    TERM_GREEN,TERM_L_GREEN,TERM_RED,TERM_L_GRAY,TERM_L_GREEN,
    TERM_UMBER,TERM_BLUE,TERM_GREEN,TERM_WHITE,TERM_L_GRAY,
    TERM_L_RED,TERM_L_GRAY,TERM_WHITE,TERM_L_GRAY,TERM_L_GRAY,
    TERM_L_RED,TERM_RED,TERM_BLUE,TERM_YELLOW,TERM_YELLOW,
    TERM_L_BLUE,TERM_L_UMBER,TERM_WHITE,TERM_L_UMBER,TERM_YELLOW,
    TERM_D_GRAY,TERM_L_GRAY,TERM_UMBER,TERM_L_BLUE,TERM_D_GRAY,
    TERM_YELLOW,TERM_L_GREEN
};


/*
 * Amulet adjectives and colors, for amulets
 */

static cptr amulet_adj[MAX_AMULETS] = {
    "Amber","Driftwood","Coral","Agate","Ivory",
    "Obsidian","Bone","Brass","Bronze","Pewter",
    "Tortoise Shell","Golden","Azure","Crystal","Silver",
    "Copper"
};

static byte amulet_col[MAX_AMULETS] = {
    TERM_YELLOW,TERM_L_UMBER,TERM_WHITE,TERM_L_GRAY,TERM_WHITE,
    TERM_D_GRAY,TERM_WHITE,TERM_L_UMBER,TERM_L_UMBER,TERM_GRAY,
    TERM_UMBER,TERM_YELLOW,TERM_L_BLUE,TERM_WHITE,TERM_L_GRAY,
    TERM_L_UMBER
};


/*
 * Syllables for scrolls
 */

static cptr syllables[MAX_SYLLABLES] = {
  "a","ab","ag","aks","ala","an","ankh","app",
  "arg","arze","ash","aus","ban","bar","bat","bek",
  "bie","bin","bit","bjor","blu","bot","bu",
  "byt","comp","con","cos","cre","dalf","dan",
  "den","der","doe","dok","eep","el","eng","er","ere","erk",
  "esh","evs","fa","fid","flit","for","fri","fu","gan",
  "gar","glen","gop","gre","ha","he","hyd","i",
  "ing","ion","ip","ish","it","ite","iv","jo",
  "kho","kli","klis","la","lech","man","mar",
  "me","mi","mic","mik","mon","mung","mur","nag","nej",
  "nelg","nep","ner","nes","nis","nih","nin","o",
  "od","ood","org","orn","ox","oxy","pay","pet",
  "ple","plu","po","pot","prok","re","rea","rhov",
  "ri","ro","rog","rok","rol","sa","san","sat",
  "see","sef","seh","shu","ski","sna","sne","snik",
  "sno","so","sol","sri","sta","sun","ta","tab",
  "tem","ther","ti","tox","trol","tue","turs","u",
  "ulk","um","un","uni","ur","val","viv","vly",
  "vom","wah","wed","werg","wex","whon","wun","x",
  "yerg","yp","zun","tri","blaa"
};


/*
 * Hold the titles of scrolls, ten characters each
 * Also keep an array of scroll colors, all WHITE for now
 */

static char scroll_adj[MAX_TITLES][10];

static byte scroll_col[MAX_TITLES];






/*
 * Certain items have a flavor
 * This function is used only by "flavor_init()"
 */
static bool object_has_flavor(int i)
{
    /* Check for flavor */
    switch (k_list[i].tval) {

        /* The standard "flavored" items */
        case TV_AMULET:
        case TV_RING:
        case TV_STAFF:
        case TV_WAND:
        case TV_SCROLL:
        case TV_POTION:
        case TV_ROD:
            return (TRUE);

        /* Hack -- food SOMETIMES has a flavor */
        case TV_FOOD:
            if (k_list[i].sval < SV_FOOD_MIN_FOOD) return (TRUE);
            return (FALSE);
    }

    /* Assume no flavor */
    return (FALSE);
}


/*
 * Certain items, if aware, are known instantly
 * This function is used only by "flavor_init()"
 */
static bool object_easy_know(int i)
{
    /* Analyze the "tval" */
    switch (k_list[i].tval) {

        /* Spellbooks */
        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
            return (TRUE);

        /* Simple items */
        case TV_FLASK:
        case TV_JUNK:
        case TV_BOTTLE:
        case TV_SKELETON:
        case TV_SPIKE:
            return (TRUE);

        /* All Food, Potions, Scrolls, Rods */
        case TV_FOOD:
        case TV_POTION:
        case TV_SCROLL:
        case TV_ROD:
            return (TRUE);

        /* Some Rings, Amulets, Lites */
        case TV_RING:
        case TV_AMULET:
        case TV_LITE:
            if (k_list[i].flags3 & TR3_EASY_KNOW) return (TRUE);
            return (FALSE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Extract the "default" attr for each object
 * This function is used only by "flavor_init()"
 */
static byte object_k_attr(int i)
{
    /* Flavored items */
    if (x_list[i].has_flavor) {

        /* Extract the indexx */
        int indexx = k_list[i].sval;

        /* Analyze the item */
        switch (k_list[i].tval) {

            case TV_FOOD:
                return (food_col[indexx]);

            case TV_POTION:
                return (potion_col[indexx]);

            case TV_SCROLL:
                return (scroll_col[indexx]);

            case TV_AMULET:
                return (amulet_col[indexx]);

            case TV_RING:
                return (ring_col[indexx]);

            case TV_STAFF:
                return (staff_col[indexx]);

            case TV_WAND:
                return (wand_col[indexx]);

            case TV_ROD:
                return (rod_col[indexx]);
        }
    }

    /* Default attr if legal */
    if (k_list[i].k_attr) return (k_list[i].k_attr);

    /* Default to white */
    return (TERM_WHITE);
}


/*
 * Hack -- prepare the default object attr codes by tval
 */
static byte default_tval_to_attr(int i)
{
    switch (i) {
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
        return (TERM_WHITE);
      case TV_CHEST:
        return (TERM_GRAY);
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
        return (TERM_L_UMBER);
      case TV_LITE:
        return (TERM_YELLOW);
      case TV_SPIKE:
        return (TERM_GRAY);
      case TV_BOW:
        return (TERM_UMBER);
      case TV_DIGGING:
        return (TERM_GRAY);
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
        return (TERM_L_GRAY);
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_CROWN:
      case TV_HELM:
      case TV_SHIELD:
      case TV_CLOAK:
        return (TERM_L_UMBER);
      case TV_SOFT_ARMOR:
      case TV_HARD_ARMOR:
      case TV_DRAG_ARMOR:
        return (TERM_GRAY);
      case TV_AMULET:
        return (TERM_ORANGE);
      case TV_RING:
        return (TERM_RED);
      case TV_STAFF:
        return (TERM_L_UMBER);
      case TV_WAND:
        return (TERM_L_GREEN);
      case TV_ROD:
        return (TERM_L_GRAY);
      case TV_SCROLL:
        return (TERM_WHITE);
      case TV_POTION:
        return (TERM_L_BLUE);
      case TV_FLASK:
        return (TERM_YELLOW);
      case TV_FOOD:
        return (TERM_L_UMBER);
      case TV_MAGIC_BOOK:
        return (TERM_L_RED);
      case TV_PRAYER_BOOK:
        return (TERM_L_GREEN);
    }

    return (TERM_WHITE);
}


/*
 * Prepare the "x_list" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Food, Scrolls, Potions, Wands, Staffs, Rods, Rings, Amulets.
 *
 * Hack -- make sure they stay the same for each saved game
 * This is accomplished by the use of a saved "random seed"
 * Note that this function does not call any others, so the
 * hack is not such a big deal as in town_gen().
 */
void flavor_init(void)
{
    int		h, i, j, k;

    byte	tmp2;

    cptr	tmp1;

    char	string[160];


    /* Hack -- Play games with the R.N.G. */
    set_seed(randes_seed);

    /* The first 3 entries for potions are fixed */
    /* That is, slime mold juice, apple juice, water */
    for (i = 3; i < MAX_COLORS; i++) {
        j = rand_int(MAX_COLORS - 3) + 3;
        tmp1 = potion_adj[i];
        potion_adj[i] = potion_adj[j];
        potion_adj[j] = tmp1;
        tmp2 = potion_col[i];
        potion_col[i] = potion_col[j];
        potion_col[j] = tmp2;
    }

    /* Woods are used for staffs */
    for (i = 0; i < MAX_WOODS; i++) {
        j = rand_int(MAX_WOODS);
        tmp1 = staff_adj[i];
        staff_adj[i] = staff_adj[j];
        staff_adj[j] = tmp1;
        tmp2 = staff_col[i];
        staff_col[i] = staff_col[j];
        staff_col[j] = tmp2;
    }

    /* Wands are made of metal */
    for (i = 0; i < MAX_METALS; i++) {
        j = rand_int(MAX_METALS);
        tmp1 = wand_adj[i];
        wand_adj[i] = wand_adj[j];
        wand_adj[j] = tmp1;
        tmp2 = wand_col[i];
        wand_col[i] = wand_col[j];
        wand_col[j] = tmp2;
    }

    /* Rocks are used for rings */
    for (i = 0; i < MAX_ROCKS; i++) {
        j = rand_int(MAX_ROCKS);
        tmp1 = ring_adj[i];
        ring_adj[i] = ring_adj[j];
        ring_adj[j] = tmp1;
        tmp2 = ring_col[i];
        ring_col[i] = ring_col[j];
        ring_col[j] = tmp2;
    }

    /* Rocks are used for amulets */
    for (i = 0; i < MAX_AMULETS; i++) {
        j = rand_int(MAX_AMULETS);
        tmp1 = amulet_adj[i];
        amulet_adj[i] = amulet_adj[j];
        amulet_adj[j] = tmp1;
        tmp2 = amulet_col[i];
        amulet_col[i] = amulet_col[j];
        amulet_col[j] = tmp2;
    }

    /* Hack -- Molds and Mushrooms (not normal foods) have colors */
    for (i = 0; i < MAX_SHROOM; i++) {
        j = rand_int(MAX_SHROOM);
        tmp1 = food_adj[i];
        food_adj[i] = food_adj[j];
        food_adj[j] = tmp1;
        tmp2 = food_col[i];
        food_col[i] = food_col[j];
        food_col[j] = tmp2;
    }

    /* Hack -- Scrolls have titles, and are always white */
    for (h = 0; h < MAX_TITLES; h++) {
        string[0] = '\0';

        /* Construct a two or three word title */
        k = rand_range(2,3);
        for (i = 0; i < k; i++) {

            /* Add a one or two syllable word */
            for (j = rand_range(1,2); j > 0; j--) {
                (void)strcat(string, syllables[rand_int(MAX_SYLLABLES)]);
            }

            /* Add a space */
            if (i < k - 1) {
                (void)strcat(string, " ");
            }
        }

        /* Hack -- chop off part of the title */
        if (string[8] == ' ') {
            string[8] = '\0';
        }
        else if (string[7] == ' ') {
            string[7] = '\0';
        }
        else {
            string[9] = '\0';
        }

        /* Save the title */
        (void)strcpy(scroll_adj[h], string);

        /* Hack -- all scrolls are white */
        scroll_col[h] = TERM_WHITE;
    }

    /* Rods are made of metal */
    for (i = 0; i < MAX_METALS; i++) {
        j = rand_int(MAX_METALS);
        tmp1 = rod_adj[i];
        rod_adj[i] = rod_adj[j];
        rod_adj[j] = tmp1;
        tmp2 = rod_col[i];
        rod_col[i] = rod_col[j];
        rod_col[j] = tmp2;
    }

    /* Hack -- undo the hack above */
    reset_seed();


    /* Analyze every object */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Check for a "flavor" */
        x_list[i].has_flavor = object_has_flavor(i);

        /* No flavor yields aware */
        if (!x_list[i].has_flavor) x_list[i].aware = TRUE;

        /* Check for "easily known" */
        x_list[i].easy_know = object_easy_know(i);

        /* Extract the "underlying" attr */
        x_list[i].x_attr = x_list[i].k_attr = object_k_attr(i);

        /* Extract the "underlying" char */
        x_list[i].x_char = x_list[i].k_char = k_list[i].k_char;
    }

    /* Default attr/chars for object tvals */
    for (i = 0; i < 128; i++) {

        /* Extract a default attr */
        tval_to_attr[i] = default_tval_to_attr(i);

        /* Hack -- Assume no char is known */
        tval_to_char[i] = ' ';
    }

    /* Hack -- Guess at "correct" values for tval_to_char[] */
    for (i = 0; i < MAX_K_IDX; i++) {

        /* Use the first value we find */
        if (tval_to_char[k_list[i].tval] == ' ') {

            /* Use that value */
            tval_to_char[k_list[i].tval] = x_list[i].k_char;
        }
    }
}






/*
 * Known2 is true when the "attributes" of an object are "known".
 * These include tohit, todam, toac, cost, and pval (charges).
 *
 * Note that "knowing" an object gives you everything that an "awareness"
 * gives you, and much more.  In fact, the player is always "aware" of any
 * item of which he has full "knowledge".
 *
 * But having full knowledge of, say, one "wand of wonder", does not, by
 * itself, give you knowledge, or even awareness, of other "wands of wonder".
 * It happens that most "identify" routines (including "buying from a shop")
 * will make the player "aware" of the object as well as fully "know" it.
 *
 * This routine also removes inscriptions generated by "innate feelings".
 */
void inven_known(inven_type *i_ptr)
{
    /* Remove "inscriptions" created when ID_FELT was set */
    if (i_ptr->ident & ID_FELT) {

        /* Hack -- Remove any inscription we may have made */
        if ((streq(i_ptr->inscrip, "cursed")) ||

            (streq(i_ptr->inscrip, "terrible")) ||
            (streq(i_ptr->inscrip, "worthless")) ||

            (streq(i_ptr->inscrip, "blessed")) ||

            (streq(i_ptr->inscrip, "special")) ||
            (streq(i_ptr->inscrip, "excellent")) ||
            (streq(i_ptr->inscrip, "good")) ||

            (streq(i_ptr->inscrip, "average"))) {

            /* Forget the inscription */
            inscribe(i_ptr, "");
        }
    }

    /* Hack -- notice cursed items */
    else if (cursed_p(i_ptr)) {

        /* Put an initial inscription */
        if (!i_ptr->inscrip[0]) inscribe(i_ptr, "cursed");
    }


    /* Clear the "Felt" info */
    i_ptr->ident &= ~ID_FELT;

    /* Clear the "Worn" info */
    i_ptr->ident &= ~ID_WORN;

    /* Clear the "Empty" info */
    i_ptr->ident &= ~ID_EMPTY;

    /* Now we know all about it */
    i_ptr->ident |= ID_KNOWN;
}





/*
 * The player is now aware of the effects of the given object.
 */
void inven_aware(inven_type *i_ptr)
{
    /* Fully aware of the effects */
    x_list[i_ptr->k_idx].aware = TRUE;
}



/*
 * Something has been "sampled"
 */
void inven_tried(inven_type *i_ptr)
{
    /* Mark it as tried (even if "aware") */
    x_list[i_ptr->k_idx].tried = TRUE;
}





#ifdef USE_COLOR

/*
 * Get a legal "multi-hued" color
 * Does NOT include White, Black, or any Grays.
 * Should it include "Brown" and "Light Brown"?
 */
static byte mh_attr(void)
{
    switch (randint(11)) {
        case 1: return (TERM_RED);
        case 2: return (TERM_BLUE);
        case 3: return (TERM_GREEN);
        case 4: return (TERM_YELLOW);
        case 5: return (TERM_ORANGE);
        case 6: return (TERM_VIOLET);
        case 7: return (TERM_UMBER);
        case 8: return (TERM_L_RED);
        case 9: return (TERM_L_BLUE);
        case 10: return (TERM_L_GREEN);
        case 11: return (TERM_L_UMBER);
    }

    return (TERM_WHITE);
}

#endif




/*
 * Return a color to use for the bolt/ball spells
 */
byte spell_color(int type)
{

#ifdef USE_COLOR

    if (!use_color) return (TERM_WHITE);

    switch (type) {

        case GF_MISSILE:
                return (mh_attr());		/* multihued */
        case GF_ELEC:
                return (TERM_YELLOW);
        case GF_POIS:
                return (TERM_GREEN);
        case GF_ACID:
                return (TERM_GRAY);
        case GF_COLD:
                return (TERM_L_BLUE);
        case GF_FIRE:
                return (TERM_RED);
        case GF_HOLY_ORB:
                return (TERM_D_GRAY);
        case GF_ARROW:
                return (TERM_L_UMBER);
        case GF_PLASMA:
                return (TERM_RED);
        case GF_NETHER:
                return (TERM_VIOLET);
        case GF_WATER:
                return (TERM_BLUE);
        case GF_CHAOS:
                return (mh_attr());		/* multihued */
        case GF_SHARDS:
                return (TERM_L_UMBER);
        case GF_SOUND:
                return (TERM_ORANGE);
        case GF_CONFUSION:
                return (mh_attr());		/* multihued */
        case GF_DISENCHANT:
                return (TERM_VIOLET);
        case GF_NEXUS:
                return (TERM_L_RED);
        case GF_FORCE:
                return (TERM_WHITE);
        case GF_INERTIA:
                return (TERM_L_GRAY);
        case GF_LITE_WEAK:
        case GF_LITE:
                return (TERM_YELLOW);
        case GF_DARK_WEAK:
        case GF_DARK:
                return (TERM_D_GRAY);
        case GF_TIME:
                return (TERM_L_BLUE);
        case GF_GRAVITY:
                return (TERM_GRAY);
        case GF_MANA:
                return (TERM_L_RED);
        case GF_METEOR:
                return (TERM_ORANGE);
        case GF_ICE:
                return (TERM_L_BLUE);
    }

#endif

    /* Standard "color" */
    return (TERM_WHITE);
}












/*
 * Helper function.  Compare the "ident" field of two objects.
 * Determine if the "ident" fields of two items "match".
 */
static bool similar_ident(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Food, Potions, Scrolls, and Rods are "simple" objects */
    if (i_ptr->tval == TV_FOOD) return (1);
    if (i_ptr->tval == TV_POTION) return (1);
    if (i_ptr->tval == TV_SCROLL) return (1);
    if (i_ptr->tval == TV_ROD) return (1);

    /* Require identical "feeling" status */
    if ((i_ptr->ident & ID_FELT) != (j_ptr->ident & ID_FELT)) return (0);

    /* Require identical "worn" status */
    if ((i_ptr->ident & ID_WORN) != (j_ptr->ident & ID_WORN)) return (0);

    /* Require identical "emptiness" */
    if ((i_ptr->ident & ID_EMPTY) != (j_ptr->ident & ID_EMPTY)) return (0);

    /* Allow match if both items are "known" */
    if (inven_known_p(i_ptr) && inven_known_p(j_ptr)) return (1);

    /* Require identical "knowledge" */
    if (inven_known_p(i_ptr) != inven_known_p(j_ptr)) return (0);

    /* Hack -- Allow match on unidentified missiles */
    if (i_ptr->tval == TV_SHOT) return (1);
    if (i_ptr->tval == TV_BOLT) return (1);
    if (i_ptr->tval == TV_ARROW) return (1);

    /* Assume no match */
    return (0);
}




/*
 * Determine if an item can "absorb" a second item
 *
 * No object can absorb itself.  This prevents object replication.
 *
 * When an object absorbs another, the second object loses all
 * of its attributes (except for "number", which gets added in),
 * so it is important to verify that all important attributes match.
 * But the "costs" are averaged, to allow stacking of discounted items.
 *
 * Note that all "problems" associated with "combining" differently
 * priced objects have been removed by never combining such objects.
 *
 * We allow wands (and staffs) to combine if they are known to have
 * equivalent charges.  They are unstacked as they are used.
 * We allow rods to combine when they are fully charged, and again,
 * we unstack them as they are used (see effects.c).
 *
 * We do not allow chests to combine, it would be annoying.
 *
 * We do NOT allow activatable items (artifacts or dragon scale mail)
 * to stack, to keep the "activation" code clean.  Artifacts may stack,
 * but only with another identical artifact (which does not exist).
 * Ego items may stack as long as they have the same ego-item type.
 */
int item_similar(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Identical items cannot be stacked */
    if (i_ptr == j_ptr) return (0);

    /* Different objects cannot be stacked */
    if (i_ptr->k_idx != j_ptr->k_idx) return (0);


    /* Hack -- refuse weapons/armor if requested */
    if (!stack_allow_items &&
        (wearable_p(i_ptr)) &&
        (i_ptr->tval != TV_LITE) &&
        (i_ptr->tval != TV_RING) &&
        (i_ptr->tval != TV_AMULET)) return (0);

    /* Hack -- refuse wands/staffs/rods if requested */
    if (!stack_allow_wands &&
        ((i_ptr->tval == TV_STAFF) ||
         (i_ptr->tval == TV_WAND) ||
         (i_ptr->tval == TV_ROD))) return (0);


    /* Require matching prices */
    if (i_ptr->cost != j_ptr->cost) return (0);

    /* Hack -- Require matching discounts, unless told not to */
    if (!stack_force_costs && (i_ptr->discount != j_ptr->discount)) return (0);


    /* Require identical "pval" codes */
    if (i_ptr->pval != j_ptr->pval) return (0);

    /* Require many identical values */
    if ((i_ptr->tohit     != j_ptr->tohit)     ||
        (i_ptr->todam     != j_ptr->todam)     ||
        (i_ptr->toac      != j_ptr->toac)      ||
        (i_ptr->ac        != j_ptr->ac)        ||
        (i_ptr->dd        != j_ptr->dd)        ||
        (i_ptr->ds        != j_ptr->ds)) {
        return (0);
    }

    /* Require identical flags */
    if ((i_ptr->flags1 != j_ptr->flags1) ||
        (i_ptr->flags2 != j_ptr->flags2) ||
        (i_ptr->flags3 != j_ptr->flags3)) {
        return (0);
    }


    /* Both items must be "fully identified" (see above) */
    if (!similar_ident(i_ptr, j_ptr)) return (0);


    /* Require identical "artifact" names */
    if (i_ptr->name1 != j_ptr->name1) return (0);

    /* Require identical "ego-item" names */
    if (i_ptr->name2 != j_ptr->name2) return (0);


    /* XXX Hack -- never stack "activatable" items */
    if (wearable_p(i_ptr) && (i_ptr->flags3 & TR3_ACTIVATE)) return (0);
    if (wearable_p(j_ptr) && (j_ptr->flags3 & TR3_ACTIVATE)) return (0);


    /* Hack -- Never stack chests */
    if (i_ptr->tval == TV_CHEST) return (0);


    /* No stack can grow bigger than a certain size */
    if (i_ptr->number + j_ptr->number >= MAX_STACK_SIZE) return (0);


    /* Hack -- never "lose" an inscription */
    if (i_ptr->inscrip[0] && j_ptr->inscrip[0] &&
        strcmp(i_ptr->inscrip, j_ptr->inscrip)) return (0);

    /* Hack -- normally require matching "inscriptions" */
    if (!stack_force_notes &&
        strcmp(i_ptr->inscrip, j_ptr->inscrip)) return (0);


    /* Paranoia -- Different types cannot be stacked */
    if (i_ptr->tval != j_ptr->tval) return (0);

    /* Paranoia -- Different sub-types cannot be stacked */
    if (i_ptr->sval != j_ptr->sval) return (0);

    /* Paranoia -- Different weights cannot be stacked */
    if (i_ptr->weight != j_ptr->weight) return (0);

    /* Paranoia -- Timeout should always be zero (see "TR3_ACTIVATE" above) */
    if (i_ptr->timeout || j_ptr->timeout) return (0);


    /* They match, so they must be similar */
    return (TRUE);
}




/*
 * Print a char "c" into a string "t", as if by sprintf(t, "%c", c),
 * and return a pointer to the terminator (t + 1).
 */
static char *objdes_chr(char *t, char c)
{
    /* Copy the char */
    *t++ = c;

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}


/*
 * Print a string "s" into a string "t", as if by strcpy(t, s),
 * and return a pointer to the terminator.
 */
static char *objdes_str(char *t, cptr s)
{
    /* Copy the string */
    while (*s) *t++ = *s++;

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}


/*
 * XXX XXX XXX XXX For some unknown reason, the statement
 * "*t++ = '0' + n;" uses the *original* value of "n" instead
 * of the modified value of "n", if the statement occurs after
 * the "loop" below.  This may be a Think C 6.0 compiler bug.
 * Inserting a line like "dummy = n;" fixes the problem (!).
 */


/*
 * Print an unsigned number "n" into a string "t", as if by
 * sprintf(t, "%u", n), and return a pointer to the terminator.
 */
static char *objdes_num(char *t, uint n)
{
    uint p;

    /* Find "size" of "n" */
    for (p = 1; n >= p * 10; p = p * 10);

    /* Dump each digit */
    while (p >= 1) {

        /* Dump the digit */
        *t++ = '0' + n / p;

        /* Remove the digit */
        n = n % p;

        /* Process next digit */
        p = p / 10;
    }

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}




/*
 * Print an signed number "v" into a string "t", as if by
 * sprintf(t, "%+d", n), and return a pointer to the terminator.
 * Note that we always print a sign, either "+" or "-".
 */
static char *objdes_int(char *t, sint v)
{
    uint p, n;

    /* Negative */
    if (v < 0) {

        /* Take the absolute value */
        n = 0 - v;

        /* Use a "minus" sign */
        *t++ = '-';
    }

    /* Positive (or zero) */
    else {

        /* Use the actual number */
        n = v;

        /* Use a "plus" sign */
        *t++ = '+';
    }

    /* Find "size" of "n" */
    for (p = 1; n >= p * 10; p = p * 10);

    /* Dump each digit */
    while (p >= 1) {

        /* Dump the digit */
        *t++ = '0' + n / p;

        /* Remove the digit */
        n = n % p;

        /* Process next digit */
        p = p / 10;
    }

    /* Terminate */
    *t = '\0';

    /* Result */
    return (t);
}



/*
 * Creates a description of the item "i_ptr", and stores it in "out_val".
 *
 * If "pref" is TRUE, the description is verbose, and has an article
 * (or number, or "no more") prefixed to the description.
 *
 * Note that buf must be large enough to hold the longest possible
 * description.  And the descriptions can get pretty long, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)"
 *
 * Note the use of "objdes_num()" and "objdes_int()" instead of various
 * forms of "sprintf()", which is not very efficient.  Also, the "%+d" code
 * does not work on some machines, though we could always use something like
 * sprintf(buf, "%c%d", POM(val), ABS(val)) for sprintf(buf, "%+d", val).
 *
 * Note that "objdes_store()" forces "plain_descriptions" to appear "true",
 * so that the "flavors" of "un-aware" objects are not shown.
 *
 * Note that ALL ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that ALL artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lite's use the "k_list" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_list" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_list" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lites.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 */
void objdes(char *buf, inven_type *i_ptr, int pref)
{
    cptr		basenm, modstr;
    int			power, indexx;

    bool		aware = FALSE;
    bool		known = FALSE;
    bool		plain = FALSE;

    bool		append_name = FALSE;

    bool		show_weapon = FALSE;
    bool		show_armour = FALSE;

    cptr		s, u;
    char		*t;

    char		p1 = '(', p2 = ')';
    char		b1 = '[', b2 = ']';
    char		c1 = '{', c2 = '}';

    char		tmpstr[160];


    /* See if the object is "aware" */
    if (inven_aware_p(i_ptr)) aware = TRUE;

    /* See if the object is "known" */
    if (inven_known_p(i_ptr)) known = TRUE;

    /* See if we should use skip "flavors" */
    if (aware && plain_descriptions) plain = TRUE;

    /* Hack -- Extract the sub-type "indexx" */
    indexx = i_ptr->sval;


    /* Extract default "base" string */
    basenm = k_list[i_ptr->k_idx].name;

    /* Assume no "modifier" string */
    modstr = "";


    /* Analyze the object */
    switch (i_ptr->tval) {

      /* Some objects are easy to describe */
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
      case TV_SPIKE:
      case TV_FLASK:
      case TV_CHEST:
        break;


      /* Missiles/ Bows/ Weapons */
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
      case TV_BOW:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
      case TV_DIGGING:
        show_weapon = TRUE;
        break;


      /* Armour */
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_CLOAK:
      case TV_CROWN:
      case TV_HELM:
      case TV_SHIELD:
      case TV_SOFT_ARMOR:
      case TV_HARD_ARMOR:
      case TV_DRAG_ARMOR:
        show_armour = TRUE;
        break;


      /* Lites (including a few "Specials") */
      case TV_LITE:

        break;


      /* Amulets (including a few "Specials") */
      case TV_AMULET:

        if (artifact_p(i_ptr) && aware) break;

        modstr = amulet_adj[indexx];
        if (aware) append_name = TRUE;

        basenm = plain ? "& Amulet~" : "& # Amulet~";

        break;


      /* Rings (including a few "Specials") */
      case TV_RING:

        if (artifact_p(i_ptr) && aware) break;

        modstr = ring_adj[indexx];
        if (aware) append_name = TRUE;

        if (!aware && (i_ptr->sval == SV_RING_POWER)) modstr = "Plain Gold";

        basenm = plain ? "& Ring~" : "& # Ring~";

        break;


      case TV_STAFF:
        modstr = staff_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Staff~" : "& # Staff~";
        break;

      case TV_WAND:
        modstr = wand_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Wand~" : "& # Wand~";
        break;

      case TV_ROD:
        modstr = rod_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Rod~" : "& # Rod~";
        break;

      case TV_SCROLL:
        modstr = scroll_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Scroll~" : "& Scroll~ titled \"#\"";
        break;

      case TV_POTION:
        modstr = potion_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Potion~" : "& # Potion~";
        break;

      case TV_FOOD:

        /* Ordinary food is "boring" */
        if (i_ptr->sval >= SV_FOOD_MIN_FOOD) break;

        /* Modifier string */
        modstr = food_adj[indexx];
        if (aware) append_name = TRUE;

        /* The Molds */
        if (i_ptr->sval >= SV_FOOD_MIN_MOLD) {

            basenm = plain ? "& Hairy Mold~" : "& # Hairy Mold~";
        }

        /* The Mushrooms */
        else {

            basenm = plain ? "& Mushroom~" : "& # Mushroom~";
        }

        break;


      /* Magic Books */
      case TV_MAGIC_BOOK:
        modstr = basenm;
        basenm = "& Book~ of Magic Spells #";
        break;

      /* Prayer Books */
      case TV_PRAYER_BOOK:
        modstr = basenm;
        basenm = "& Holy Book~ of Prayers #";
        break;


      /* Things in the dungeon */
      case TV_OPEN_DOOR:
      case TV_CLOSED_DOOR:
      case TV_SECRET_DOOR:
      case TV_RUBBLE:
      case TV_INVIS_TRAP:
      case TV_VIS_TRAP:
      case TV_UP_STAIR:
      case TV_DOWN_STAIR:
        break;


      /* Hack -- Gold/Gems */
      case TV_GOLD:
        strcpy(buf, basenm);
        return;

      /* Mega-Hack -- Store Doors */
      case TV_STORE_DOOR:
        if (pref) {
            strcpy(buf, "the entrance to the ");
        }
        else {
            strcpy(buf, "entrance to the ");
        }
        strcat(buf, basenm);
        return;

      /* Used in the "inventory" routine */
      default:
        strcpy(buf, "(nothing)");
        return;
    }


    /* Start dumping the result */
    t = buf;

    /* The object "expects" a "number" */
    if (basenm[0] == '&') {

        /* Skip the ampersand (and space) */
        s = basenm + 2;

        /* No prefix */
        if (!pref) {
            /* Nothing */
        }

        /* Hack -- None left */
        else if (i_ptr->number <= 0) {
            t = objdes_str(t, "no more ");
        }

        /* Extract the number */
        else if (i_ptr->number > 1) {
            t = objdes_num(t, i_ptr->number);
            t = objdes_chr(t, ' ');
        }

        /* Hack -- The only one of its kind */
        else if (known && artifact_p(i_ptr)) {
            t = objdes_str(t, "The ");
        }

        /* A single one, with a vowel in the modifier */
        else if ((*s == '#') && (is_a_vowel(modstr[0]))) {
            t = objdes_str(t, "an ");
        }

        /* A single one, with a vowel */
        else if (is_a_vowel(*s)) {
            t = objdes_str(t, "an ");
        }

        /* A single one, without a vowel */
        else {
            t = objdes_str(t, "a ");
        }
    }

    /* Hack -- objects that "never" take an article */
    else {

        /* No ampersand */
        s = basenm;

        /* No pref */
        if (!pref) {
            /* Nothing */
        }

        /* Hack -- all gone */
        else if (i_ptr->number <= 0) {
            t = objdes_str(t, "no more ");
        }

        /* Prefix a number if required */
        else if (i_ptr->number > 1) {
            t = objdes_num(t, i_ptr->number);
            t = objdes_chr(t, ' ');
        }

        /* Hack -- The only one of its kind */
        else if (known && artifact_p(i_ptr)) {
            t = objdes_str(t, "The ");
        }

        /* Hack -- single items get no prefix */
        else {
            /* Nothing */
        }
    }

    /* Paranoia -- skip illegal tildes */
    /* while (*s == '~') s++; */

    /* Copy the string */
    for ( ; *s; s++) {

        /* Pluralizer */
        if (*s == '~') {

            /* Add a plural if needed */
            if (i_ptr->number != 1) {

                char k = t[-1];

                /* Hack -- Cutlass-es, Torch-es, Patch-es */
                if ((k == 's') || (k == 'h')) *t++ = 'e';

                /* Add an 's' */
                *t++ = 's';
            }
        }

        /* Modifier */
        else if (*s == '#') {

            /* Insert the modifier */
            for (u = modstr; *u; u++) *t++ = *u;
        }

        /* Normal */
        else {

            /* Copy */
            *t++ = *s;
        }
    }

    /* Terminate */
    *t = '\0';


    /* Append the "kind name" to the "base name" */
    if (append_name) {
        t = objdes_str(t, " of ");
        t = objdes_str(t, k_list[i_ptr->k_idx].name);
    }


    /* Hack -- Append "Artifact" or "Special" names */
    if (known) {

        /* Grab any artifact name */
        if (i_ptr->name1) {
            t = objdes_chr(t, ' ');
            t = objdes_str(t, v_list[i_ptr->name1].name);
        }

        /* Grab any ego-item name */
        else if (i_ptr->name2) {
            t = objdes_chr(t, ' ');
            t = objdes_str(t, ego_names[i_ptr->name2]);
        }
    }


    /* No more details wanted */
    if (!pref) return;


    /* Hack -- Chests must be described in detail */
    if (i_ptr->tval == TV_CHEST) {

        /* Hack -- Empty chests are "obvious" */
        if (!i_ptr->flags1) {
            t = objdes_str(t, " (empty)");
        }

        /* Not searched yet */
        else if (!known) {

            /* Nothing */
        }

        /* Describe the traps, if any */
        else if (i_ptr->flags2 & CH2_TRAP_MASK) {

            /* Describe the traps */
            switch (i_ptr->flags2 & CH2_TRAP_MASK) {
              case CH2_LOSE_STR:
                t = objdes_str(t, " (Poison Needle)");
                break;
              case CH2_POISON:
                t = objdes_str(t, " (Poison Needle)");
                break;
              case CH2_PARALYSED:
                t = objdes_str(t, " (Gas Trap)");
                break;
              case CH2_EXPLODE:
                t = objdes_str(t, " (Explosion Device)");
                break;
              case CH2_SUMMON:
                t = objdes_str(t, " (Summoning Runes)");
                break;
              default:
                t = objdes_str(t, " (Multiple Traps)");
                break;
            }
        }

        /* May be "disarmed" */
        else if (i_ptr->flags2 & CH2_DISARMED) {
            t = objdes_str(t, " (disarmed)");
        }

        /* May be "locked" */
        else if (i_ptr->flags2 & CH2_LOCKED) {
            t = objdes_str(t, " (locked)");
        }

        /* Assume "unlocked" */
        else {
            t = objdes_str(t, " (unlocked)");
        }
    }


    /* Hack -- Extract a few flags */
    if (wearable_p(i_ptr)) {

        /* Display the item like a weapon */
        if (i_ptr->flags3 & TR3_SHOW_MODS) show_weapon = TRUE;

        /* Display the item like a weapon */
        if (i_ptr->tohit && i_ptr->todam) show_weapon = TRUE;

        /* Display the item like armour */
        if (i_ptr->ac) show_armour = TRUE;
    }


    /* Dump base weapon info */
    switch (i_ptr->tval) {

      /* Missiles and Weapons */
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
      case TV_DIGGING:

        /* Append a "damage" string */
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, p1);
        t = objdes_num(t, i_ptr->dd);
        t = objdes_chr(t, 'd');
        t = objdes_num(t, i_ptr->ds);
        t = objdes_chr(t, p2);

        /* All done */
        break;


      /* Bows get a special "damage string" */
      case TV_BOW:

        /* Hack -- Extract the "base power" */
        power = (i_ptr->sval % 10);

        /* Apply the "Extra Might" flag */
        if (i_ptr->flags3 & TR3_XTRA_MIGHT) power++;

        /* Append a special "damage" string */
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, p1);
        t = objdes_chr(t, 'x');
        t = objdes_num(t, power);
        t = objdes_chr(t, p2);

        /* All done */
        break;
    }


    /* Add the weapon bonuses */
    if (known) {

        /* Show the tohit/todam on request */
        if (show_weapon) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, p1);
            t = objdes_int(t, i_ptr->tohit);
            t = objdes_chr(t, ',');
            t = objdes_int(t, i_ptr->todam);
            t = objdes_chr(t, p2);
        }

        /* Show the tohit if needed */
        else if (i_ptr->tohit) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, p1);
            t = objdes_int(t, i_ptr->tohit);
            t = objdes_chr(t, p2);
        }

        /* Show the todam if needed */
        else if (i_ptr->todam) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, p1);
            t = objdes_int(t, i_ptr->todam);
            t = objdes_chr(t, p2);
        }
    }


    /* Add the armor bonuses */
    if (known) {

        /* Show the armor class info */
        if (show_armour) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, b1);
            t = objdes_num(t, i_ptr->ac);
            t = objdes_chr(t, ',');
            t = objdes_int(t, i_ptr->toac);
            t = objdes_chr(t, b2);
        }

        /* No base armor, but does increase armor */
        else if (i_ptr->toac) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, b1);
            t = objdes_int(t, i_ptr->toac);
            t = objdes_chr(t, b2);
        }
    }

    /* Hack -- always show base armor */
    else if (show_armour) {
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, b1);
        t = objdes_num(t, i_ptr->ac);
        t = objdes_chr(t, b2);
    }


    /* Hack -- Wands and Staffs have charges */
    if (known &&
        ((i_ptr->tval == TV_STAFF) ||
         (i_ptr->tval == TV_WAND))) {

        /* Dump " (N charges)" */
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, p1);
        t = objdes_num(t, i_ptr->pval);
        t = objdes_str(t, " charge");
        if (i_ptr->pval != 1) t = objdes_chr(t, 's');
        t = objdes_chr(t, p2);
    }

    /* Hack -- Rods have a "charging" indicator */
    else if (known && (i_ptr->tval == TV_ROD)) {

        /* Hack -- Dump " (charging)" if relevant */
        if (i_ptr->pval) t = objdes_str(t, " (charging)");
    }

    /* Hack -- Process Lanterns/Torches */
    else if ((i_ptr->tval == TV_LITE) && (!artifact_p(i_ptr))) {

        /* Hack -- Turns of light for normal lites */
        t = objdes_str(t, " (with ");
        t = objdes_num(t, i_ptr->pval);
        t = objdes_str(t, " turns of light)");
    }


    /* Dump "pval" flags for wearable items */
    if (known && wearable_p(i_ptr) &&
        (i_ptr->flags1 & TR1_PVAL_MASK)) {

        /* Start the display */
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, p1);

        /* Dump the "pval" itself */
        t = objdes_int(t, i_ptr->pval);

        /* Do not display the "pval" flags */
        if (i_ptr->flags3 & TR3_HIDE_TYPE) {

            /* Nothing */
        }

        /* Speed */
        else if (i_ptr->flags1 & TR1_SPEED) {

            /* Dump " to speed" */
            t = objdes_str(t, " to speed");
        }

        /* Attack speed */
        else if (i_ptr->flags1 & TR1_ATTACK_SPD) {

            /* Add " attack" */
            t = objdes_str(t, " attack");

            /* Add "attacks" */
            if (ABS(i_ptr->pval) != 1) t = objdes_chr(t, 's');
        }

        /* Stealth */
        else if (i_ptr->flags1 & TR1_STEALTH) {

            /* Dump " to stealth" */
            t = objdes_str(t, " to stealth");
        }

        /* Search */
        else if (i_ptr->flags1 & TR1_SEARCH) {

            /* Dump " to searching" */
            t = objdes_str(t, " to searching");
        }

        /* Infravision */
        else if (i_ptr->flags1 & TR1_INFRA) {

            /* Dump " to infravision" */
            t = objdes_str(t, " to infravision");
        }

        /* Tunneling */
        else if (i_ptr->flags1 & TR1_TUNNEL) {

            /* Nothing */
        }

        /* Finish the display */
        t = objdes_chr(t, p2);
    }


    /* Erase tmp_str */
    tmpstr[0] = '\0';

    /* Use the standard inscription if available */
    if (i_ptr->inscrip[0]) {
        strcpy(tmpstr, i_ptr->inscrip);
    }

    /* Mega-Hack -- note empty wands/staffs */
    else if (!known && (i_ptr->ident & ID_EMPTY)) {
        strcpy(tmpstr, "empty");
    }

    /* Note "cursed" if the item is known (and cursed) */
    else if (known && cursed_p(i_ptr)) {
        strcpy(tmpstr, "cursed");
    }

    /* Note "cursed" if any curse has been felt */
    else if ((i_ptr->ident & ID_FELT) && (cursed_p(i_ptr))) {
        strcpy(tmpstr, "cursed");
    }

#if 0
    /* Hack -- Note "worn" if the object has been worn */
    else if (i_ptr->ident & ID_WORN) {
        strcpy(tmpstr, "tested");
    }
#endif

    /* Note "tried" if the object has been tested unsuccessfully */
    else if (!aware && x_list[i_ptr->k_idx].tried) {
        strcpy(tmpstr, "tried");
    }

    /* Note the discount, if any */
    else if (i_ptr->discount) {
        objdes_num(tmpstr, i_ptr->discount);
        strcat(tmpstr, "% off");
    }

    /* Use the inscription, if any */
    if (tmpstr[0]) {
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, c1);
        t = objdes_str(t, tmpstr);
        t = objdes_chr(t, c2);
    }
}


/*
 * Hack -- describe an item currently in a store's inventory
 * This allows an item to *look* like the player is "aware" of it
 */
void objdes_store(char *buf, inven_type *i_ptr, int mode)
{
    /* Save the actual "plain_descriptions" flag */
    bool hack_plain = plain_descriptions;

    /* Save the "aware" flag */
    bool hack_aware = x_list[i_ptr->k_idx].aware;

    /* Save the "known" flag */
    bool hack_known = (i_ptr->ident & ID_KNOWN) ? TRUE : FALSE;


    /* Force "plain descriptions" of store items */
    plain_descriptions = TRUE;

    /* Set the "known" flag */
    i_ptr->ident |= ID_KNOWN;

    /* Force "aware" for description */
    x_list[i_ptr->k_idx].aware = TRUE;


    /* Describe the object */
    objdes(buf, i_ptr, mode);


    /* Restore the "plain descriptions" option */
    plain_descriptions = hack_plain;

    /* Restore "aware" flag */
    x_list[i_ptr->k_idx].aware = hack_aware;

    /* Clear the known flag */
    if (!hack_known) i_ptr->ident &= ~ID_KNOWN;
}




/*
 * Clear an item
 */
void invwipe(inven_type *i_ptr)
{
    /* Copy the empty record */
    (*i_ptr) = i_list[0];
}


/*
 * Make "i_ptr" a "clean" copy of the given "kind" of object
 */
void invcopy(inven_type *i_ptr, int k_idx)
{
    inven_kind *k_ptr = &k_list[k_idx];

    /* Hack -- clear the record */
    (*i_ptr) = i_list[0];
    
    /* Save the kind index */
    i_ptr->k_idx = k_idx;

    /* Quick access to tval/sval */
    i_ptr->tval = k_ptr->tval;
    i_ptr->sval = k_ptr->sval;

    /* Save the default "pval" */
    i_ptr->pval = k_ptr->pval;

    /* Default number and weight */
    i_ptr->number = k_ptr->number;
    i_ptr->weight = k_ptr->weight;

    /* Default magic */
    i_ptr->tohit = k_ptr->tohit;
    i_ptr->todam = k_ptr->todam;
    i_ptr->toac = k_ptr->toac;
    i_ptr->ac = k_ptr->ac;
    i_ptr->dd = k_ptr->dd;
    i_ptr->ds = k_ptr->ds;

    /* Default cost */
    i_ptr->cost = k_ptr->cost;

    /* Default flags */
    i_ptr->flags1 = k_ptr->flags1;
    i_ptr->flags2 = k_ptr->flags2;
    i_ptr->flags3 = k_ptr->flags3;

    /* Wipe the inscription */
    inscribe(i_ptr, "");
}






