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
 * XXX XXX Hack -- note that "TERM_MULTI" is now just "TERM_VIOLET"
 * We will have to find a cleaner method for "MULTI_HUED" later.
 * There were only two multi-hued "flavors" (one potion, one food).
 * Plus five multi-hued "base-objects" (3 dragon scales, one blade
 * of chaos, and one something else).  See the SHIMMER_OBJECTS code
 * in "dungeon.c" and the object color extractor in "cave.c".
 */
#define TERM_MULTI	TERM_VIOLET


/*
 * Max sizes of the following arrays
 */
#define MAX_ROCKS      42       /* Used with rings (min 38) */
#define MAX_AMULETS    16       /* Used with amulets (min 13) */
#define MAX_WOODS      32       /* Used with staffs (min 30) */
#define MAX_METALS     32       /* Used with wands/rods (min 29/28) */
#define MAX_COLORS     60       /* Used with potions (min 60) */
#define MAX_SHROOM     20       /* Used with mushrooms (min 20) */
#define MAX_TITLES     50       /* Used with scrolls (min 48) */
#define MAX_SYLLABLES 158       /* Used with scrolls (see below) */


/*
 * Rings (adjectives and colors)
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
    TERM_RED,TERM_WHITE,TERM_RED,TERM_SLATE,TERM_WHITE,
    TERM_GREEN,TERM_L_GREEN,TERM_RED,TERM_L_WHITE,TERM_L_GREEN,
    TERM_UMBER,TERM_BLUE,TERM_GREEN,TERM_WHITE,TERM_L_WHITE,
    TERM_L_RED,TERM_L_WHITE,TERM_WHITE,TERM_L_WHITE,TERM_L_WHITE,
    TERM_L_RED,TERM_RED,TERM_BLUE,TERM_YELLOW,TERM_YELLOW,
    TERM_L_BLUE,TERM_L_UMBER,TERM_WHITE,TERM_L_UMBER,TERM_YELLOW,
    TERM_L_DARK,TERM_L_WHITE,TERM_UMBER,TERM_L_BLUE,TERM_L_DARK,
    TERM_YELLOW,TERM_L_GREEN
};


/*
 * Amulets (adjectives and colors)
 */

static cptr amulet_adj[MAX_AMULETS] = {
    "Amber","Driftwood","Coral","Agate","Ivory",
    "Obsidian","Bone","Brass","Bronze","Pewter",
    "Tortoise Shell","Golden","Azure","Crystal","Silver",
    "Copper"
};

static byte amulet_col[MAX_AMULETS] = {
    TERM_YELLOW,TERM_L_UMBER,TERM_WHITE,TERM_L_WHITE,TERM_WHITE,
    TERM_L_DARK,TERM_WHITE,TERM_L_UMBER,TERM_L_UMBER,TERM_SLATE,
    TERM_UMBER,TERM_YELLOW,TERM_L_BLUE,TERM_WHITE,TERM_L_WHITE,
    TERM_L_UMBER
};


/*
 * Staffs (adjectives and colors)
 */

static cptr staff_adj[MAX_WOODS] = {
    "Aspen","Balsa","Banyan","Birch","Cedar",
    "Cottonwood","Cypress","Dogwood","Elm","Eucalyptus",
    "Hemlock","Hickory","Ironwood","Locust","Mahogany",
    "Maple","Mulberry","Oak","Pine","Redwood",
    "Rosewood","Spruce","Sycamore","Teak","Walnut",
    "Mistletoe","Hawthorn","Bamboo","Silver","Runed",
    "Golden","Ashen"/*,"Gnarled","Ivory","Willow"*/
};

static byte staff_col[MAX_WOODS] = {
    TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_L_UMBER,TERM_L_UMBER,TERM_UMBER,TERM_L_UMBER,TERM_UMBER,
    TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_RED,
    TERM_RED,TERM_L_UMBER,TERM_L_UMBER,TERM_L_UMBER,TERM_UMBER,
    TERM_GREEN,TERM_L_UMBER,TERM_L_UMBER,TERM_L_WHITE,TERM_UMBER,
    TERM_YELLOW,TERM_SLATE,/*???,???,???*/
};


/*
 * Wands (adjectives and colors)
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
    TERM_L_BLUE,TERM_L_DARK,TERM_WHITE,TERM_L_UMBER,TERM_YELLOW,
    TERM_SLATE,TERM_L_WHITE,TERM_L_WHITE,TERM_L_UMBER,TERM_RED,
    TERM_L_WHITE,TERM_L_WHITE,TERM_L_WHITE,TERM_WHITE,TERM_WHITE,
    TERM_L_WHITE,TERM_L_WHITE,TERM_L_BLUE,TERM_L_UMBER,TERM_YELLOW,
    TERM_L_UMBER,TERM_L_WHITE,TERM_L_WHITE,TERM_L_WHITE,TERM_L_WHITE,
    TERM_L_BLUE,TERM_L_BLUE,TERM_UMBER,TERM_L_UMBER,TERM_L_UMBER,
    TERM_WHITE,TERM_SLATE,/*TERM_SLATE,TERM_WHITE,TERM_SLATE*/
};


/*
 * Rods (adjectives and colors).
 * Efficiency -- copied from wand arrays
 */

static cptr rod_adj[MAX_METALS];

static byte rod_col[MAX_METALS];


/*
 * Mushrooms (adjectives and colors)
 */

static cptr food_adj[MAX_SHROOM] = {
    "Blue","Black","Black Spotted","Brown","Dark Blue",
    "Dark Green","Dark Red","Yellow","Furry","Green",
    "Grey","Light Blue","Light Green","Violet","Red",
    "Slimy","Tan","White","White Spotted","Wrinkled",
};

static byte food_col[MAX_SHROOM] = {
    TERM_BLUE,TERM_L_DARK,TERM_L_DARK,TERM_UMBER,TERM_BLUE,
    TERM_GREEN,TERM_RED,TERM_YELLOW,TERM_L_WHITE,TERM_GREEN,
    TERM_SLATE,TERM_L_BLUE,TERM_L_GREEN,TERM_VIOLET,TERM_RED,
    TERM_SLATE,TERM_L_UMBER,TERM_WHITE,TERM_WHITE,TERM_UMBER
};


/*
 * Color adjectives and colors, for potions.
 * Hack -- The first four entries are hard-coded.
 * (water, apple juice, slime mold juice, something)
 */

static cptr potion_adj[MAX_COLORS] = {
    "Clear","Light Brown","Icky Green","xxx",
    "Azure","Blue","Blue Speckled","Black","Brown","Brown Speckled",
    "Bubbling","Chartreuse","Cloudy","Copper Speckled","Crimson","Cyan",
    "Dark Blue","Dark Green","Dark Red","Gold Speckled","Green",
    "Green Speckled","Grey","Grey Speckled","Hazy","Indigo",
    "Light Blue","Light Green","Magenta","Metallic Blue","Metallic Red",
    "Metallic Green","Metallic Purple","Misty","Orange","Orange Speckled",
    "Pink","Pink Speckled","Puce","Purple","Purple Speckled",
    "Red","Red Speckled","Silver Speckled","Smoky","Tangerine",
    "Violet","Vermilion","White","Yellow", "Violet Speckled",
    "Pungent","Clotted Red","Viscous Pink","Oily Yellow","Gloopy Green",
    "Shimmering","Coagulated Crimson","Yellow Speckled","Gold"
};

static byte potion_col[MAX_COLORS] = {
    TERM_WHITE,TERM_L_UMBER,TERM_GREEN,0,
    TERM_L_BLUE,TERM_BLUE,TERM_BLUE,TERM_L_DARK,TERM_UMBER,TERM_UMBER,
    TERM_L_WHITE,TERM_L_GREEN,TERM_WHITE,TERM_L_UMBER,TERM_RED,TERM_L_BLUE,
    TERM_BLUE,TERM_GREEN,TERM_RED,TERM_YELLOW,TERM_GREEN,
    TERM_GREEN,TERM_SLATE,TERM_SLATE,TERM_L_WHITE,TERM_VIOLET,
    TERM_L_BLUE,TERM_L_GREEN,TERM_RED,TERM_BLUE,TERM_RED,
    TERM_GREEN,TERM_VIOLET,TERM_L_WHITE,TERM_ORANGE,TERM_ORANGE,
    TERM_L_RED,TERM_L_RED,TERM_VIOLET,TERM_VIOLET,TERM_VIOLET,
    TERM_RED,TERM_RED,TERM_L_WHITE,TERM_L_DARK,TERM_ORANGE,
    TERM_VIOLET,TERM_RED,TERM_WHITE,TERM_YELLOW,TERM_VIOLET,
    TERM_L_RED,TERM_RED,TERM_L_RED,TERM_YELLOW,TERM_GREEN,
    TERM_MULTI,TERM_RED,TERM_YELLOW,TERM_YELLOW
};


/*
 * Syllables for scrolls (must be 1-4 letters each)
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
 * Hold the titles of scrolls, 6 to 14 characters each
 * Also keep an array of scroll colors (always WHITE for now)
 */

static char scroll_adj[MAX_TITLES][16];

static byte scroll_col[MAX_TITLES];






/*
 * Certain items have a flavor
 * This function is used only by "flavor_init()"
 */
static bool object_has_flavor(int i)
{
    inven_kind *k_ptr = &k_info[i];

    /* Check for flavor */
    switch (k_ptr->tval) {

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
            if (k_ptr->sval < SV_FOOD_MIN_FOOD) return (TRUE);
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
    inven_kind *k_ptr = &k_info[i];

    /* Analyze the "tval" */
    switch (k_ptr->tval) {

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
            if (k_ptr->flags3 & TR3_EASY_KNOW) return (TRUE);
            return (FALSE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Hack -- prepare the default object attr codes by tval
 */
static byte default_tval_to_attr(int tval)
{
    switch (tval) {
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
        return (TERM_WHITE);
      case TV_CHEST:
        return (TERM_SLATE);
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
        return (TERM_L_UMBER);
      case TV_LITE:
        return (TERM_YELLOW);
      case TV_SPIKE:
        return (TERM_SLATE);
      case TV_BOW:
        return (TERM_UMBER);
      case TV_DIGGING:
        return (TERM_SLATE);
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
        return (TERM_L_WHITE);
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
        return (TERM_SLATE);
      case TV_AMULET:
        return (TERM_ORANGE);
      case TV_RING:
        return (TERM_RED);
      case TV_STAFF:
        return (TERM_L_UMBER);
      case TV_WAND:
        return (TERM_L_GREEN);
      case TV_ROD:
        return (TERM_L_WHITE);
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
 * Hack -- prepare the default object char codes by tval
 */
static byte default_tval_to_char(int tval)
{
    int i;

    /* Hack -- Guess at "correct" values for tval_to_char[] */
    for (i = 0; i < MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Use the first value we find */
        if (k_ptr->tval == tval) return (k_ptr->k_char);
    }

    /* Default to space */
    return (' ');
}



/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 14 letters.  The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Duplicate titles are avoided by requiring that no two scrolls share
 * the same first three letters (not the most or least efficient method).
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 *
 * Note that the "hacked seed" provides an RNG with alternating parity!
 */
void flavor_init(void)
{
    int		i, j;

    byte	temp_col;

    cptr	temp_adj;


    /* Hack -- Use the "simple" RNG */
    Rand_quick = TRUE;
    
    /* Hack -- Induce consistant flavors */
    Rand_value = seed_flavor;


    /* Efficiency -- Rods/Wands share initial array */
    for (i = 0; i < MAX_METALS; i++) {
        rod_adj[i] = wand_adj[i];
        rod_col[i] = wand_col[i];
    }


    /* Rings have "ring colors" */
    for (i = 0; i < MAX_ROCKS; i++) {
        j = rand_int(MAX_ROCKS);
        temp_adj = ring_adj[i];
        ring_adj[i] = ring_adj[j];
        ring_adj[j] = temp_adj;
        temp_col = ring_col[i];
        ring_col[i] = ring_col[j];
        ring_col[j] = temp_col;
    }

    /* Amulets have "amulet colors" */
    for (i = 0; i < MAX_AMULETS; i++) {
        j = rand_int(MAX_AMULETS);
        temp_adj = amulet_adj[i];
        amulet_adj[i] = amulet_adj[j];
        amulet_adj[j] = temp_adj;
        temp_col = amulet_col[i];
        amulet_col[i] = amulet_col[j];
        amulet_col[j] = temp_col;
    }

    /* Staffs */
    for (i = 0; i < MAX_WOODS; i++) {
        j = rand_int(MAX_WOODS);
        temp_adj = staff_adj[i];
        staff_adj[i] = staff_adj[j];
        staff_adj[j] = temp_adj;
        temp_col = staff_col[i];
        staff_col[i] = staff_col[j];
        staff_col[j] = temp_col;
    }

    /* Wands */
    for (i = 0; i < MAX_METALS; i++) {
        j = rand_int(MAX_METALS);
        temp_adj = wand_adj[i];
        wand_adj[i] = wand_adj[j];
        wand_adj[j] = temp_adj;
        temp_col = wand_col[i];
        wand_col[i] = wand_col[j];
        wand_col[j] = temp_col;
    }

    /* Rods */
    for (i = 0; i < MAX_METALS; i++) {
        j = rand_int(MAX_METALS);
        temp_adj = rod_adj[i];
        rod_adj[i] = rod_adj[j];
        rod_adj[j] = temp_adj;
        temp_col = rod_col[i];
        rod_col[i] = rod_col[j];
        rod_col[j] = temp_col;
    }

    /* Foods (Mushrooms) */
    for (i = 0; i < MAX_SHROOM; i++) {
        j = rand_int(MAX_SHROOM);
        temp_adj = food_adj[i];
        food_adj[i] = food_adj[j];
        food_adj[j] = temp_adj;
        temp_col = food_col[i];
        food_col[i] = food_col[j];
        food_col[j] = temp_col;
    }

    /* Potions (The first 4 entries for potions are fixed) */
    for (i = 4; i < MAX_COLORS; i++) {
        j = rand_int(MAX_COLORS - 4) + 4;
        temp_adj = potion_adj[i];
        potion_adj[i] = potion_adj[j];
        potion_adj[j] = temp_adj;
        temp_col = potion_col[i];
        potion_col[i] = potion_col[j];
        potion_col[j] = temp_col;
    }

    /* Scrolls (random titles, always white) */
    for (i = 0; i < MAX_TITLES; i++) {

        bool icky = TRUE;

        char buf[80];

        /* Get a new title */
        while (icky) {

            /* Assume not icky */
            icky = FALSE;

            /* Start a new title */
            buf[0] = '\0';

            /* Collect words until done */
            while (1) {

                int q, s;

                char tmp[80];

                /* Start a new word */
                tmp[0] = '\0';

                /* Choose one or two syllables */
                s = ((rand_int(100) < 30) ? 1 : 2);

                /* Add a one or two syllable word */
                for (q = 0; q < s; q++) {

                    /* Add the syllable */
                    strcat(tmp, syllables[rand_int(MAX_SYLLABLES)]);
                }

                /* Stop before getting too long */
                if (strlen(buf) + 1 + strlen(tmp) > 15) break;

                /* Add a space */
                strcat(buf, " ");

                /* Add the word */
                strcat(buf, tmp);
            }

            /* Check for "duplicate" scroll titles */
            for (j = 0; j < i; j++) {

                /* Ignore "different" prefixes */
                if (scroll_adj[j][0] != buf[1]) continue;
                if (scroll_adj[j][1] != buf[2]) continue;
                if (scroll_adj[j][2] != buf[3]) continue;

                /* Assume icky */
                icky = TRUE;

                /* Stop looking */
                break;
            }
        }

        /* Save the title */
        strcpy(scroll_adj[i], buf+1);

        /* All scrolls are white */
        scroll_col[i] = TERM_WHITE;
    }

    /* Hack -- Use the "complex" RNG */
    Rand_quick = FALSE;

    /* Analyze every object */
    for (i = 0; i < MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Skip "empty" objects */
        if (!k_ptr->name) continue;

        /* Check for a "flavor" */
        k_ptr->has_flavor = object_has_flavor(i);

        /* No flavor yields aware */
        if (!k_ptr->has_flavor) k_ptr->aware = TRUE;

        /* Check for "easily known" */
        k_ptr->easy_know = object_easy_know(i);
    }
}




/*
 * Extract the "default" attr for each object
 * This function is used only by "flavor_init()"
 */
static byte object_k_attr(int i)
{
    inven_kind *k_ptr = &k_info[i];

    /* Flavored items */
    if (k_ptr->has_flavor) {

        /* Extract the indexx */
        int indexx = k_ptr->sval;

        /* Analyze the item */
        switch (k_ptr->tval) {

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
    if (k_ptr->k_attr) return (k_ptr->k_attr);

    /* Default to white */
    return (TERM_WHITE);
}


/*
 * Extract the "default" char for each object
 * This function is used only by "flavor_init()"
 */
static byte object_k_char(int i)
{
    inven_kind *k_ptr = &k_info[i];

    return (k_ptr->k_char);
}


/*
 * Reset the "visual" lists
 *
 * This is useful for switching on/off the "use_graphics" flag.
 */
void reset_visuals(void)
{
    int i;

    /* Extract some info about terrain features */
    for (i = 0; i < 64; i++) {

        feature_type *f_ptr = &f_info[i];

        /* Assume we will use the underlying values */
        f_ptr->z_attr = f_ptr->f_attr;
        f_ptr->z_char = f_ptr->f_char;
    }

    /* Extract some info about objects */
    for (i = 0; i < MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Extract the "underlying" attr */
        k_ptr->i_attr = object_k_attr(i);

        /* Extract the "underlying" char */
        k_ptr->i_char = object_k_char(i);

        /* Assume we will use the underlying values */
        k_ptr->x_attr = k_ptr->i_attr;
        k_ptr->x_char = k_ptr->i_char;
    }

    /* Extract some info about monsters */
    for (i = 0; i < MAX_R_IDX; i++) {

        /* Extract the "underlying" attr */
        r_info[i].l_attr = r_info[i].r_attr;

        /* Extract the "underlying" char */
        r_info[i].l_char = r_info[i].r_char;
    }
    
    /* Extract attr/chars for equippy items (by tval) */
    for (i = 0; i < 128; i++) {

        /* Extract a default attr */
        tval_to_attr[i] = default_tval_to_attr(i);

        /* Extract a default char */
        tval_to_char[i] = default_tval_to_char(i);
    }
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
 * XXX XXX XXX Compiler Bug Warning (Macintosh Think C 6.0)
 *
 * For some unknown reason, the statement "*t++ = '0' + n;"
 * (if used below) uses the *original* value of "n" instead
 * of the modified value of "n", if the statement occurs after
 * the "loop" below, unless you insert the line "dummy = n;"!
 * A similar fatal bug was recently removed from "generate.c".
 */


/*
 * Print an unsigned number "n" into a string "t", as if by
 * sprintf(t, "%u", n), and return a pointer to the terminator.
 */
static char *objdes_num(char *t, uint n)
{
    uint p;

    /* Find "size" of "n" */
    for (p = 1; n >= p * 10; p = p * 10) ;

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
    for (p = 1; n >= p * 10; p = p * 10) ;

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
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The global option "plain_descriptions" affects the "flavor" detail.
 *
 * The given "buf" must be 80 chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under 79 chars (plus a terminator).
 *
 * Note the use of "objdes_num()" and "objdes_int()" as hyper-efficient,
 * portable, versions of some common "sprintf()" commands.
 *
 * Note that "objdes_store()" forces "plain_descriptions" to appear "true",
 * so that the "flavors" of "un-aware" objects are not shown.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lite's use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lites.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 *
 * If "pref" then a "numeric" prefix will be pre-pended.
 *
 * Mode:
 *   0 -- The Cloak of Death
 *   1 -- The Cloak of Death [1,+3]
 *   2 -- The Cloak of Death [1,+3] (+2 to Stealth)
 *   3 -- The Cloak of Death [1,+3] (+2 to Stealth) {nifty}
 */
void objdes(char *buf, inven_type *i_ptr, int pref, int mode)
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

    char		tmp_val[160];

    u32b		f1, f2, f3;

    inven_kind		*k_ptr = &k_info[i_ptr->k_idx];


    /* Extract some flags */
    inven_flags(i_ptr, &f1, &f2, &f3);


    /* See if the object is "aware" */
    if (inven_aware_p(i_ptr)) aware = TRUE;

    /* See if the object is "known" */
    if (inven_known_p(i_ptr)) known = TRUE;

    /* See if we should use skip "flavors" */
    if (aware && plain_descriptions) plain = TRUE;

    /* Hack -- Extract the sub-type "indexx" */
    indexx = i_ptr->sval;

    /* Extract default "base" string */
    basenm = (k_name + k_ptr->name);

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

        /* Known artifacts */
        if (artifact_p(i_ptr) && aware) break;

        /* Color the object */
        modstr = amulet_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Amulet~" : "& # Amulet~";
        break;


      /* Rings (including a few "Specials") */
      case TV_RING:

        /* Known artifacts */
        if (artifact_p(i_ptr) && aware) break;

        /* Color the object */
        modstr = ring_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Ring~" : "& # Ring~";

        /* Hack -- The One Ring */
        if (!aware && (i_ptr->sval == SV_RING_POWER)) modstr = "Plain Gold";

        break;


      case TV_STAFF:

        /* Color the object */
        modstr = staff_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Staff~" : "& # Staff~";
        break;

      case TV_WAND:

        /* Color the object */
        modstr = wand_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Wand~" : "& # Wand~";
        break;

      case TV_ROD:

        /* Color the object */
        modstr = rod_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Rod~" : "& # Rod~";
        break;

      case TV_SCROLL:

        /* Color the object */
        modstr = scroll_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Scroll~" : "& Scroll~ titled \"#\"";
        break;

      case TV_POTION:

        /* Color the object */
        modstr = potion_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Potion~" : "& # Potion~";
        break;

      case TV_FOOD:

        /* Ordinary food is "boring" */
        if (i_ptr->sval >= SV_FOOD_MIN_FOOD) break;

        /* Color the object */
        modstr = food_adj[indexx];
        if (aware) append_name = TRUE;
        basenm = plain ? "& Mushroom~" : "& # Mushroom~";
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

      /* Hack -- Gold/Gems */
      case TV_GOLD:
        strcpy(buf, basenm);
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

                /* Hack -- Cutlass-es, Torch-es, Patch-es, Ax-es */
                if ((k == 's') || (k == 'h') || (k == 'x')) *t++ = 'e';

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
        t = objdes_str(t, (k_name + k_ptr->name));
    }


    /* Hack -- Append "Artifact" or "Special" names */
    if (known) {

        /* Grab any artifact name */
        if (i_ptr->name1) {

            artifact_type *a_ptr = &a_info[i_ptr->name1];

            t = objdes_chr(t, ' ');
            t = objdes_str(t, (a_name + a_ptr->name));
        }

        /* Grab any ego-item name */
        else if (i_ptr->name2) {

            ego_item_type *e_ptr = &e_info[i_ptr->name2];

            t = objdes_chr(t, ' ');
            t = objdes_str(t, (e_name + e_ptr->name));
        }
    }


    /* No more details wanted */
    if (mode < 1) return;


    /* Hack -- Chests must be described in detail */
    if (i_ptr->tval == TV_CHEST) {

        /* Not searched yet */
        if (!known) {
            /* Nothing */
        }

        /* May be "empty" */
        else if (!i_ptr->pval) {
            t = objdes_str(t, " (empty)");
        }

        /* May be "disarmed" */
        else if (i_ptr->pval < 0) {
            if (chest_traps[i_ptr->pval]) {
                t = objdes_str(t, " (disarmed)");
            }
            else {
                t = objdes_str(t, " (unlocked)");
            }
        }

        /* Describe the traps, if any */
        else {

            /* Describe the traps */
            switch (chest_traps[i_ptr->pval]) {
              case 0:
                t = objdes_str(t, " (Locked)");
                break;
              case CHEST_LOSE_STR:
                t = objdes_str(t, " (Poison Needle)");
                break;
              case CHEST_LOSE_CON:
                t = objdes_str(t, " (Poison Needle)");
                break;
              case CHEST_POISON:
                t = objdes_str(t, " (Gas Trap)");
                break;
              case CHEST_PARALYZE:
                t = objdes_str(t, " (Gas Trap)");
                break;
              case CHEST_EXPLODE:
                t = objdes_str(t, " (Explosion Device)");
                break;
              case CHEST_SUMMON:
                t = objdes_str(t, " (Summoning Runes)");
                break;
              default:
                t = objdes_str(t, " (Multiple Traps)");
                break;
            }
        }
    }


    /* Display the item like a weapon */
    if (f3 & TR3_SHOW_MODS) show_weapon = TRUE;

    /* Display the item like a weapon */
    if (i_ptr->to_h && i_ptr->to_d) show_weapon = TRUE;

    /* Display the item like armour */
    if (i_ptr->ac) show_armour = TRUE;


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

        /* Mega-Hack -- Extract the "base power" */
        power = (i_ptr->sval % 10);

        /* Apply the "Extra Might" flag */
        if (f3 & TR3_XTRA_MIGHT) power++;

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
            t = objdes_int(t, i_ptr->to_h);
            t = objdes_chr(t, ',');
            t = objdes_int(t, i_ptr->to_d);
            t = objdes_chr(t, p2);
        }

        /* Show the tohit if needed */
        else if (i_ptr->to_h) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, p1);
            t = objdes_int(t, i_ptr->to_h);
            t = objdes_chr(t, p2);
        }

        /* Show the todam if needed */
        else if (i_ptr->to_d) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, p1);
            t = objdes_int(t, i_ptr->to_d);
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
            t = objdes_int(t, i_ptr->to_a);
            t = objdes_chr(t, b2);
        }

        /* No base armor, but does increase armor */
        else if (i_ptr->to_a) {
            t = objdes_chr(t, ' ');
            t = objdes_chr(t, b1);
            t = objdes_int(t, i_ptr->to_a);
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


    /* No more details wanted */
    if (mode < 2) return;


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
    if (known && (f1 & TR1_PVAL_MASK)) {

        /* Start the display */
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, p1);

        /* Dump the "pval" itself */
        t = objdes_int(t, i_ptr->pval);

        /* Do not display the "pval" flags */
        if (f3 & TR3_HIDE_TYPE) {

            /* Nothing */
        }

        /* Speed */
        else if (f1 & TR1_SPEED) {

            /* Dump " to speed" */
            t = objdes_str(t, " to speed");
        }

        /* Attack speed */
        else if (f1 & TR1_BLOWS) {

            /* Add " attack" */
            t = objdes_str(t, " attack");

            /* Add "attacks" */
            if (ABS(i_ptr->pval) != 1) t = objdes_chr(t, 's');
        }

        /* Stealth */
        else if (f1 & TR1_STEALTH) {

            /* Dump " to stealth" */
            t = objdes_str(t, " to stealth");
        }

        /* Search */
        else if (f1 & TR1_SEARCH) {

            /* Dump " to searching" */
            t = objdes_str(t, " to searching");
        }

        /* Infravision */
        else if (f1 & TR1_INFRA) {

            /* Dump " to infravision" */
            t = objdes_str(t, " to infravision");
        }

        /* Tunneling */
        else if (f1 & TR1_TUNNEL) {

            /* Nothing */
        }

        /* Finish the display */
        t = objdes_chr(t, p2);
    }


    /* No more details wanted */
    if (mode < 3) return;


    /* No inscription yet */
    tmp_val[0] = '\0';

    /* Use the standard inscription if available */
    if (i_ptr->note) {
        strcpy(tmp_val, quark_str(i_ptr->note));
    }

    /* Note "cursed" if the item is known to be cursed */
    else if (cursed_p(i_ptr) && (known || (i_ptr->ident & ID_SENSE))) {
        strcpy(tmp_val, "cursed");
    }

    /* Mega-Hack -- note empty wands/staffs */
    else if (!known && (i_ptr->ident & ID_EMPTY)) {
        strcpy(tmp_val, "empty");
    }

    /* Note "tried" if the object has been tested unsuccessfully */
    else if (!aware && inven_tried_p(i_ptr)) {
        strcpy(tmp_val, "tried");
    }

    /* Note the discount, if any */
    else if (i_ptr->discount) {
        objdes_num(tmp_val, i_ptr->discount);
        strcat(tmp_val, "% off");
    }

    /* Append the inscription, if any */
    if (tmp_val[0]) {

        int n;

        /* Hack -- How much so far */
        n = (t - buf);

        /* Paranoia -- do not be stupid */
        if (n > 75) n = 75;

        /* Hack -- shrink the inscription */
        tmp_val[75 - n] = '\0';

        /* Append the inscription */
        t = objdes_chr(t, ' ');
        t = objdes_chr(t, c1);
        t = objdes_str(t, tmp_val);
        t = objdes_chr(t, c2);
    }
}


/*
 * Hack -- describe an item currently in a store's inventory
 * This allows an item to *look* like the player is "aware" of it
 */
void objdes_store(char *buf, inven_type *i_ptr, int pref, int mode)
{
    /* Save the actual "plain_descriptions" flag */
    bool hack_plain = plain_descriptions;

    /* Save the "aware" flag */
    bool hack_aware = k_info[i_ptr->k_idx].aware;

    /* Save the "known" flag */
    bool hack_known = (i_ptr->ident & ID_KNOWN) ? TRUE : FALSE;


    /* Force "plain descriptions" of store items */
    plain_descriptions = TRUE;

    /* Set the "known" flag */
    i_ptr->ident |= ID_KNOWN;

    /* Force "aware" for description */
    k_info[i_ptr->k_idx].aware = TRUE;


    /* Describe the object */
    objdes(buf, i_ptr, pref, mode);


    /* Restore the "plain descriptions" option */
    plain_descriptions = hack_plain;

    /* Restore "aware" flag */
    k_info[i_ptr->k_idx].aware = hack_aware;

    /* Clear the known flag */
    if (!hack_known) i_ptr->ident &= ~ID_KNOWN;
}




