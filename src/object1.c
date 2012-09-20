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
    for (i = 1; i < MAX_K_IDX; i++) {

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
    for (i = 1; i < MAX_K_IDX; i++) {

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
    for (i = 0; i < MAX_F_IDX; i++) {

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
 * Obtain the "flags" for an item
 */
void inven_flags(inven_type *i_ptr, u32b *f1, u32b *f2, u32b *f3)
{
    inven_kind *k_ptr = &k_info[i_ptr->k_idx];
    
    /* Base object */
    (*f1) = k_ptr->flags1;
    (*f2) = k_ptr->flags2;
    (*f3) = k_ptr->flags3;

    /* Artifact */
    if (i_ptr->name1) {

        artifact_type *a_ptr = &a_info[i_ptr->name1];

        (*f1) = a_ptr->flags1;
        (*f2) = a_ptr->flags2;
        (*f3) = a_ptr->flags3;
    }

    /* Ego-item */
    if (i_ptr->name2) {

        ego_item_type *e_ptr = &e_info[i_ptr->name2];
    
        (*f1) |= e_ptr->flags1;
        (*f2) |= e_ptr->flags2;
        (*f3) |= e_ptr->flags3;
    }

    /* Extra powers */
    switch (i_ptr->xtra1) {
    
        case EGO_XTRA_SUSTAIN:

            /* Choose a sustain */
            switch (i_ptr->xtra2 % 6) {
                case 0: (*f2) |= TR2_SUST_STR; break;
                case 1: (*f2) |= TR2_SUST_INT; break;
                case 2: (*f2) |= TR2_SUST_WIS; break;
                case 3: (*f2) |= TR2_SUST_DEX; break;
                case 4: (*f2) |= TR2_SUST_CON; break;
                case 5: (*f2) |= TR2_SUST_CHR; break;
            }
            
            break;
            
        case EGO_XTRA_POWER:

            /* Choose a power */
            switch (i_ptr->xtra2 % 9) {
                case 0: (*f2) |= TR2_RES_BLIND; break;
                case 1: (*f2) |= TR2_RES_CONF; break;
                case 2: (*f2) |= TR2_RES_SOUND; break;
                case 3: (*f2) |= TR2_RES_SHARDS; break;
                case 4: (*f2) |= TR2_RES_NETHER; break;
                case 5: (*f2) |= TR2_RES_NEXUS; break;
                case 6: (*f2) |= TR2_RES_CHAOS; break;
                case 7: (*f2) |= TR2_RES_DISEN; break;
                case 8: (*f2) |= TR2_RES_POIS; break;
            }
            
            break;
            
        case EGO_XTRA_ABILITY:

            /* Choose an ability */
            switch (i_ptr->xtra2 % 8) {
                case 0: (*f3) |= TR3_FEATHER; break;
                case 1: (*f3) |= TR3_LITE; break;
                case 2: (*f3) |= TR3_SEE_INVIS; break;
                case 3: (*f3) |= TR3_TELEPATHY; break;
                case 4: (*f3) |= TR3_SLOW_DIGEST; break;
                case 5: (*f3) |= TR3_REGEN; break;
                case 6: (*f2) |= TR2_FREE_ACT; break;
                case 7: (*f2) |= TR2_HOLD_LIFE; break;
            }
            
            break;
            
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




/*
 * Determine the "Activation" (if any) for an artifact
 * Return a string, or NULL for "no activation"
 */
cptr item_activation(inven_type *i_ptr)
{
    u32b f1, f2, f3;

    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
    
    /* Require activation ability */
    if (!(f3 & TR3_ACTIVATE)) return (NULL);

    /* Some artifacts can be activated */
    switch (i_ptr->name1) {

        case ART_NARTHANC:
            return "fire bolt (9d8) every 8+d8 turns";
        case ART_NIMTHANC:
            return "frost bolt (6d8) every 7+d7 turns";
        case ART_DETHANC:
            return "lightning bolt (4d8) every 6+d6 turns";
        case ART_RILIA:
            return "stinking cloud (12) every 4+d4 turns";
        case ART_BELANGIL:
            return "frost ball (48) every 5+d5 turns";
        case ART_DAL:
            return "remove fear and cure poison every 5 turns";
        case ART_RINGIL:
            return "frost ball (100) every 300 turns";
        case ART_ANDURIL:
            return "fire ball (72) every 400 turns";
        case ART_FIRESTAR:
            return "large fire ball (72) every 100 turns";
        case ART_FEANOR:
            return "haste self (20+d20 turns) every 200 turns";
        case ART_THEODEN:
            return "drain life (120) every 400 turns";
        case ART_TURMIL:
            return "drain life (90) every 70 turns";
        case ART_CASPANION:
            return "door and trap destruction every 10 turns";
        case ART_AVAVIR:
            return "word of recall every 200 turns";
        case ART_TARATOL:
            return "haste self (20+d20 turns) every 100+d100 turns";
        case ART_ERIRIL:
            return "identify every 10 turns";
        case ART_OLORIN:
            return "probing every 20 turns";
        case ART_EONWE:
            return "mass genocide every 1000 turns";
        case ART_LOTHARANG:
            return "cure wounds (4d7) every 3+d3 turns";
        case ART_CUBRAGOL:
            return "fire branding of bolts every 999 turns";
        case ART_ARUNRUTH:
            return "frost bolt (12d8) every 500 turns";
        case ART_AEGLOS:
            return "frost ball (100) every 500 turns";
        case ART_OROME:
            return "stone to mud every 5 turns";
        case ART_SOULKEEPER:
            return "heal (1000) every 888 turns";
        case ART_BELEGENNON:
            return "phase door every 2 turns";
        case ART_CELEBORN:
            return "genocide every 500 turns";
        case ART_LUTHIEN:
            return "restore life levels every 450 turns";
        case ART_ULMO:
            return "teleport away every 150 turns";
        case ART_COLLUIN:
            return "resistance (20+d20 turns) every 111 turns";
        case ART_HOLCOLLETH:
            return "Sleep II every 55 turns";
        case ART_THINGOL:
            return "recharge item I every 70 turns";
        case ART_COLANNON:
            return "teleport every 45 turns";
        case ART_TOTILA:
            return "confuse monster every 15 turns";
        case ART_CAMMITHRIM:
            return "magic missile (2d6) every 2 turns";
        case ART_PAURHACH:
            return "fire bolt (9d8) every 8+d8 turns";
        case ART_PAURNIMMEN:
            return "frost bolt (6d8) every 7+d7 turns";
        case ART_PAURAEGEN:
            return "lightning bolt (4d8) every 6+d6 turns";
        case ART_PAURNEN:
            return "acid bolt (5d8) every 5+d5 turns";
        case ART_FINGOLFIN:
            return "a magical arrow (150) every 90+d90 turns";
        case ART_HOLHENNETH:
            return "detection every 55+d55 turns";
        case ART_GONDOR:
            return "heal (500) every 500 turns";
        case ART_RAZORBACK:
            return "star ball (150) every 1000 turns";
        case ART_BLADETURNER:
            return "berserk rage, bless, and resistance every 400 turns";
        case ART_GALADRIEL:
            return "illumination every 10+d10 turns";
        case ART_ELENDIL:
            return "magic mapping every 50+d50 turns";
        case ART_THRAIN:
            return "clairvoyance every 100+d100 turns";
        case ART_INGWE:
            return "dispel evil (x5) every 300+d300 turns";
        case ART_CARLAMMAS:
            return "protection from evil every 225+d225 turns";
        case ART_TULKAS:
            return "haste self (75+d75 turns) every 150+d150 turns";
        case ART_NARYA:
            return "large fire ball (120) every 225+d225 turns";
        case ART_NENYA:
            return "large frost ball (200) every 325+d325 turns";
        case ART_VILYA:
            return "large lightning ball (250) every 425+d425 turns";
        case ART_POWER:
            return "bizarre things every 450+d450 turns";
    }


    /* Require dragon scale mail */
    if (i_ptr->tval != TV_DRAG_ARMOR) return (NULL);

    /* Branch on the sub-type */
    switch (i_ptr->sval) {

        case SV_DRAGON_BLUE:
            return "breathe lightning (100) every 450+d450 turns";
        case SV_DRAGON_WHITE:
            return "breathe frost (110) every 450+d450 turns";
        case SV_DRAGON_BLACK:
            return "breathe acid (130) every 450+d450 turns";
        case SV_DRAGON_GREEN:
            return "breathe poison gas (150) every 450+d450 turns";
        case SV_DRAGON_RED:
            return "breathe fire (200) every 450+d450 turns";
        case SV_DRAGON_MULTIHUED:
            return "breathe multi-hued (250) every 225+d225 turns";
        case SV_DRAGON_BRONZE:
            return "breathe confusion (120) every 450+d450 turns";
        case SV_DRAGON_GOLD:
            return "breathe sound (130) every 450+d450 turns";
        case SV_DRAGON_CHAOS:
            return "breathe chaos/disenchant (220) every 300+d300 turns";
        case SV_DRAGON_LAW:
            return "breathe sound/shards (230) every 300+d300 turns";
        case SV_DRAGON_BALANCE:
            return "You breathe balance (250) every 300+d300 turns";
        case SV_DRAGON_SHINING:
            return "breathe light/darkness (200) every 300+d300 turns";
        case SV_DRAGON_POWER:
            return "breathe the elements (300) every 300+d300 turns";
    }


    /* Oops */
    return NULL;
}


/*
 * Describe a "fully identified" item
 */
bool identify_fully_aux(inven_type *i_ptr)
{
    int			i = 0, j, k;

    u32b f1, f2, f3;

    cptr		info[128];


    /* Extract the flags */
    inven_flags(i_ptr, &f1, &f2, &f3);
        

    /* Mega-Hack -- describe activation */
    if (f3 & TR3_ACTIVATE) {
        info[i++] = "It can be activated for...";
        info[i++] = item_activation(i_ptr);
        info[i++] = "...if it is being worn.";
    }


    /* Hack -- describe lite's */
    if (i_ptr->tval == TV_LITE) {

        if (artifact_p(i_ptr)) {
            info[i++] = "It provides light (radius 3) forever.";
        }
        else if (i_ptr->sval == SV_LITE_LANTERN) {
            info[i++] = "It provides light (radius 2) when fueled.";
        }
        else {
            info[i++] = "It provides light (radius 1) when fueled.";
        }
    }


    /* And then describe it fully */

    if (f1 & TR1_STR) {
        info[i++] = "It affects your strength.";
    }
    if (f1 & TR1_INT) {
        info[i++] = "It affects your intelligence.";
    }
    if (f1 & TR1_WIS) {
        info[i++] = "It affects your wisdom.";
    }
    if (f1 & TR1_DEX) {
        info[i++] = "It affects your dexterity.";
    }
    if (f1 & TR1_CON) {
        info[i++] = "It affects your constitution.";
    }
    if (f1 & TR1_CHR) {
        info[i++] = "It affects your charisma.";
    }

    if (f1 & TR1_STEALTH) {
        info[i++] = "It affects your stealth.";
    }
    if (f1 & TR1_SEARCH) {
        info[i++] = "It affects your searching.";
    }
    if (f1 & TR1_INFRA) {
        info[i++] = "It affects your infravision.";
    }
    if (f1 & TR1_TUNNEL) {
        info[i++] = "It affects your ability to tunnel.";
    }
    if (f1 & TR1_SPEED) {
        info[i++] = "It affects your speed.";
    }
    if (f1 & TR1_BLOWS) {
        info[i++] = "It affects your attack speed.";
    }

    if (f1 & TR1_BRAND_ACID) {
        info[i++] = "It does extra damage from acid.";
    }
    if (f1 & TR1_BRAND_ELEC) {
        info[i++] = "It does extra damage from electricity.";
    }
    if (f1 & TR1_BRAND_FIRE) {
        info[i++] = "It does extra damage from fire.";
    }
    if (f1 & TR1_BRAND_COLD) {
        info[i++] = "It does extra damage from frost.";
    }

    if (f1 & TR1_IMPACT) {
        info[i++] = "It can cause earthquakes.";
    }

    if (f1 & TR1_KILL_DRAGON) {
        info[i++] = "It is a great bane of dragons.";
    }
    else if (f1 & TR1_SLAY_DRAGON) {
        info[i++] = "It is especially deadly against dragons.";
    }
    if (f1 & TR1_SLAY_ORC) {
        info[i++] = "It is especially deadly against orcs.";
    }
    if (f1 & TR1_SLAY_TROLL) {
        info[i++] = "It is especially deadly against trolls.";
    }
    if (f1 & TR1_SLAY_GIANT) {
        info[i++] = "It is especially deadly against giants.";
    }
    if (f1 & TR1_SLAY_DEMON) {
        info[i++] = "It strikes at demons with holy wrath.";
    }
    if (f1 & TR1_SLAY_UNDEAD) {
        info[i++] = "It strikes at undead with holy wrath.";
    }
    if (f1 & TR1_SLAY_EVIL) {
        info[i++] = "It fights against evil with holy fury.";
    }
    if (f1 & TR1_SLAY_ANIMAL) {
        info[i++] = "It is especially deadly against natural creatures.";
    }

    if (f2 & TR2_SUST_STR) {
        info[i++] = "It sustains your strength.";
    }
    if (f2 & TR2_SUST_INT) {
        info[i++] = "It sustains your intelligence.";
    }
    if (f2 & TR2_SUST_WIS) {
        info[i++] = "It sustains your wisdom.";
    }
    if (f2 & TR2_SUST_DEX) {
        info[i++] = "It sustains your dexterity.";
    }
    if (f2 & TR2_SUST_CON) {
        info[i++] = "It sustains your constitution.";
    }
    if (f2 & TR2_SUST_CHR) {
        info[i++] = "It sustains your charisma.";
    }

    if (f2 & TR2_IM_ACID) {
        info[i++] = "It provides immunity to acid.";
    }
    if (f2 & TR2_IM_ELEC) {
        info[i++] = "It provides immunity to electricity.";
    }
    if (f2 & TR2_IM_FIRE) {
        info[i++] = "It provides immunity to fire.";
    }
    if (f2 & TR2_IM_COLD) {
        info[i++] = "It provides immunity to cold.";
    }

    if (f2 & TR2_FREE_ACT) {
        info[i++] = "It provides immunity to paralysis.";
    }
    if (f2 & TR2_HOLD_LIFE) {
        info[i++] = "It provides resistance to life draining.";
    }

    if (f2 & TR2_RES_ACID) {
        info[i++] = "It provides resistance to acid.";
    }
    if (f2 & TR2_RES_ELEC) {
        info[i++] = "It provides resistance to electricity.";
    }
    if (f2 & TR2_RES_FIRE) {
        info[i++] = "It provides resistance to fire.";
    }
    if (f2 & TR2_RES_COLD) {
        info[i++] = "It provides resistance to cold.";
    }
    if (f2 & TR2_RES_POIS) {
        info[i++] = "It provides resistance to poison.";
    }

    if (f2 & TR2_RES_LITE) {
        info[i++] = "It provides resistance to light.";
    }
    if (f2 & TR2_RES_DARK) {
        info[i++] = "It provides resistance to dark.";
    }

    if (f2 & TR2_RES_BLIND) {
        info[i++] = "It provides resistance to blindness.";
    }
    if (f2 & TR2_RES_CONF) {
        info[i++] = "It provides resistance to confusion.";
    }
    if (f2 & TR2_RES_SOUND) {
        info[i++] = "It provides resistance to sound.";
    }
    if (f2 & TR2_RES_SHARDS) {
        info[i++] = "It provides resistance to shards.";
    }

    if (f2 & TR2_RES_NETHER) {
        info[i++] = "It provides resistance to nether.";
    }
    if (f2 & TR2_RES_NEXUS) {
        info[i++] = "It provides resistance to nexus.";
    }
    if (f2 & TR2_RES_CHAOS) {
        info[i++] = "It provides resistance to chaos.";
    }
    if (f2 & TR2_RES_DISEN) {
        info[i++] = "It provides resistance to disenchantment.";
    }

    if (f3 & TR3_FEATHER) {
        info[i++] = "It induces feather falling.";
    }
    if (f3 & TR3_LITE) {
        info[i++] = "It provides permanent light.";
    }
    if (f3 & TR3_SEE_INVIS) {
        info[i++] = "It allows you to see invisible monsters.";
    }
    if (f3 & TR3_TELEPATHY) {
        info[i++] = "It gives telepathic powers.";
    }
    if (f3 & TR3_SLOW_DIGEST) {
        info[i++] = "It slows your metabolism.";
    }
    if (f3 & TR3_REGEN) {
        info[i++] = "It speeds your regenerative powers.";
    }

    if (f3 & TR3_XTRA_MIGHT) {
        info[i++] = "It fires missiles with extra might.";
    }
    if (f3 & TR3_XTRA_SHOTS) {
        info[i++] = "It fires missiles excessively fast.";
    }

    if (f3 & TR3_DRAIN_EXP) {
        info[i++] = "It drains experience.";
    }
    if (f3 & TR3_TELEPORT) {
        info[i++] = "It induces random teleportation.";
    }
    if (f3 & TR3_AGGRAVATE) {
        info[i++] = "It aggravates nearby creatures.";
    }

    if (f3 & TR3_BLESSED) {
        info[i++] = "It has been blessed by the gods.";
    }

    if (cursed_p(i_ptr)) {
        if (f3 & TR3_PERMA_CURSE) {
            info[i++] = "It is permanently cursed.";
        }
        else if (f3 & TR3_HEAVY_CURSE) {
            info[i++] = "It is heavily cursed.";
        }
        else {
            info[i++] = "It is cursed.";
        }
    }


    if (f3 & TR3_IGNORE_ACID) {
        info[i++] = "It cannot be harmed by acid.";
    }
    if (f3 & TR3_IGNORE_ELEC) {
        info[i++] = "It cannot be harmed by electricity.";
    }
    if (f3 & TR3_IGNORE_FIRE) {
        info[i++] = "It cannot be harmed by fire.";
    }
    if (f3 & TR3_IGNORE_COLD) {
        info[i++] = "It cannot be harmed by cold.";
    }


    /* No special effects */
    if (!i) return (FALSE);


    /* Save the screen */
    Term_save();

    /* Erase the screen */
    for (k = 1; k < 24; k++) prt("", k, 13);

    /* Label the information */
    prt("     Item Attributes:", 1, 15);

    /* We will print on top of the map (column 13) */
    for (k = 2, j = 0; j < i; j++) {

        /* Show the info */
        prt(info[j], k++, 15);

        /* Every 20 entries (lines 2 to 21), start over */
        if ((k == 22) && (j+1 < i)) {
            prt("-- more --", k, 15);
            inkey();
            for ( ; k > 2; k--) prt("", k, 15);
        }
    }

    /* Wait for it */
    prt("[Press any key to continue]", k, 15);
    inkey();

    /* Restore the screen */
    Term_load();

    /* Gave knowledge */
    return (TRUE);
}



/*
 * Convert an inventory index into a one character label
 * Note that the label does NOT distinguish inven/equip.
 */
s16b index_to_label(int i)
{
    /* Indexes for "inven" are easy */
    if (i < INVEN_WIELD) return (I2A(i));

    /* Indexes for "equip" are offset */
    return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven"
 * Return "-1" if the label does not indicate a real item
 */
s16b label_to_inven(int c)
{
    int i;
    
    /* Convert */
    i = (islower(c) ? A2I(c) : -1);

    /* Verify the index */
    if ((i < 0) || (i > INVEN_PACK)) return (-1);

    /* Empty slots can never be chosen */
    if (!inventory[i].k_idx) return (-1);

    /* Return the index */
    return (i);
}


/*
 * Convert a label into the index of a item in the "equip"
 * Return "-1" if the label does not indicate a real item
 */
s16b label_to_equip(int c)
{
    int i;

    /* Convert */
    i = (islower(c) ? A2I(c) : -1) + INVEN_WIELD;

    /* Verify the index */
    if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

    /* Empty slots can never be chosen */
    if (!inventory[i].k_idx) return (-1);

    /* Return the index */
    return (i);
}



/*
 * Determine which equipment slot (if any) an item likes
 */
s16b wield_slot(inven_type *i_ptr)
{
    /* Slot for equipment */
    switch (i_ptr->tval) {

        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
            return (INVEN_WIELD);

        case TV_BOW:
            return (INVEN_BOW);

        case TV_RING:

            /* Use the right hand first */
            if (!inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);

            /* Use the left hand for swapping (by default) */
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
 * Return a string mentioning how a given item is carried
 */
cptr mention_use(int i)
{
    cptr p;

    /* Examine the location */
    switch (i) {
      case INVEN_WIELD: p = "Wielding"; break;
      case INVEN_BOW:   p = "Shooting"; break;
      case INVEN_LEFT:  p = "On left hand"; break;
      case INVEN_RIGHT: p = "On right hand"; break;
      case INVEN_NECK:  p = "Around neck"; break;
      case INVEN_LITE:  p = "Light source"; break;
      case INVEN_BODY:  p = "On body"; break;
      case INVEN_OUTER: p = "About body"; break;
      case INVEN_ARM:   p = "On arm"; break;
      case INVEN_HEAD:  p = "On head"; break;
      case INVEN_HANDS: p = "On hands"; break;
      case INVEN_FEET:  p = "On feet"; break;
      default:          p = "In pack"; break;
    }

    /* Hack -- Heavy weapon */
    if (i == INVEN_WIELD) {
        inven_type *i_ptr;
        i_ptr = &inventory[i];
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "Just lifting";
        }
    }

    /* Hack -- Heavy bow */
    if (i == INVEN_BOW) {
        inven_type *i_ptr;
        i_ptr = &inventory[i];
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "Just holding";
        }
    }

    /* Return the result */
    return (p);
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
cptr describe_use(int i)
{
    cptr p;

    switch (i) {
      case INVEN_WIELD: p = "attacking monsters with"; break;
      case INVEN_BOW:   p = "shooting missiles with"; break;
      case INVEN_LEFT:  p = "wearing on your left hand"; break;
      case INVEN_RIGHT: p = "wearing on your right hand"; break;
      case INVEN_NECK:  p = "wearing around your neck"; break;
      case INVEN_LITE:  p = "using to light the way"; break;
      case INVEN_BODY:  p = "wearing on your body"; break;
      case INVEN_OUTER: p = "wearing on your back"; break;
      case INVEN_ARM:   p = "wearing on your arm"; break;
      case INVEN_HEAD:  p = "wearing on your head"; break;
      case INVEN_HANDS: p = "wearing on your hands"; break;
      case INVEN_FEET:  p = "wearing on your feet"; break;
      default:          p = "carrying in your pack"; break;
    }

    /* Hack -- Heavy weapon */
    if (i == INVEN_WIELD) {
        inven_type *i_ptr;
        i_ptr = &inventory[i];
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "just lifting";
        }
    }

    /* Hack -- Heavy bow */
    if (i == INVEN_BOW) {
        inven_type *i_ptr;
        i_ptr = &inventory[i];
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "just holding";
        }
    }

    /* Return the result */
    return p;
}





/*
 * Check an item against the item tester info
 */
bool item_tester_okay(inven_type *i_ptr)
{
    /* Hack -- allow listing empty slots */
    if (item_tester_full) return (TRUE);

    /* Require an item */
    if (!i_ptr->k_idx) return (FALSE);

    /* Hack -- ignore "gold" */
    if (i_ptr->tval == TV_GOLD) return (FALSE);

    /* Check the tval */
    if (item_tester_tval) {
        if (!(item_tester_tval == i_ptr->tval)) return (FALSE);
    }

    /* Check the hook */
    if (item_tester_hook) {
        if (!(*item_tester_hook)(i_ptr)) return (FALSE);
    }
    
    /* Assume okay */
    return (TRUE);
}




/*
 * Choice window "shadow" of the "show_inven()" function
 */
void display_inven(void)
{
    register	int i, n, z = 0;

    inven_type *i_ptr;

    byte	attr = TERM_WHITE;

    char	tmp_val[80];

    char	i_name[80];


    /* Find the "final" slot */
    for (i = 0; i < INVEN_PACK; i++) {

        i_ptr = &inventory[i];

        /* Track non-empty slots */
        if (i_ptr->k_idx) z = i + 1;
    }

    /* Display the pack */
    for (i = 0; i < z; i++) {

        /* Examine the item */
        i_ptr = &inventory[i];

        /* Start with an empty "index" */
        tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

        /* Is this item "acceptable"? */
        if (item_tester_okay(i_ptr)) {

            /* Prepare an "index" */
            tmp_val[0] = index_to_label(i);

            /* Bracket the "index" --(-- */
            tmp_val[1] = ')';
        }

        /* Display the index (or blank space) */
        Term_putstr(0, i, 3, TERM_WHITE, tmp_val);

        /* Obtain an item description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Obtain the length of the description */
        n = strlen(i_name);

        /* Get a color */
        if (use_color) attr = tval_to_attr[i_ptr->tval % 128];

        /* Display the entry itself */
        Term_putstr(3, i, n, attr, i_name);

        /* Erase the rest of the line */
        Term_erase(3+n, i, 80, 1);

        /* Display the weight if needed */
        if (show_choose_weight && i_ptr->weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
        }
    }

    /* Erase the rest of the window */
    Term_erase(0, z, 80, 24);

    /* Refresh */
    Term_fresh();
}



/*
 * Choice window "shadow" of the "show_equip()" function
 */
void display_equip(void)
{
    register	int i, n;
    inven_type *i_ptr;
    byte	attr = TERM_WHITE;

    char	tmp_val[80];

    char	i_name[80];


    /* Display the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Examine the item */
        i_ptr = &inventory[i];

        /* Start with an empty "index" */
        tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

        /* Is this item "acceptable"? */
        if (item_tester_okay(i_ptr)) {

            /* Prepare an "index" */
            tmp_val[0] = index_to_label(i);

            /* Bracket the "index" --(-- */
            tmp_val[1] = ')';
        }

        /* Display the index (or blank space) */
        Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);

        /* Obtain an item description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Obtain the length of the description */
        n = strlen(i_name);

        /* Get the color */
        if (use_color) attr = tval_to_attr[i_ptr->tval % 128];

        /* Display the entry itself */
        Term_putstr(3, i - INVEN_WIELD, n, attr, i_name);

        /* Erase the rest of the line */
        Term_erase(3+n, i - INVEN_WIELD, 80, 1);

        /* Display the slot description (if needed) */
        if (show_choose_label) {
            Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
            Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, mention_use(i));
        }

        /* Display the weight (if needed) */
        if (show_choose_weight && i_ptr->weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            int col = (show_choose_label ? 52 : 71);
            sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
        }
    }

    /* Erase the rest of the window */
    Term_erase(0, INVEN_TOTAL - INVEN_WIELD, 80, 24);

    /* Refresh */
    Term_fresh();
}






/*
 * Display the inventory.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven(void)
{
    int		i, j, k, l, z = 0;
    int		col, len, lim;

    inven_type	*i_ptr;

    char	i_name[80];

    char	tmp_val[80];

    int		out_index[23];
    byte	out_color[23];
    char	out_desc[23][80];


    /* Starting column */
    col = command_gap;

    /* Default "max-length" */
    len = 79 - col;

    /* Maximum space allowed for descriptions */
    lim = 79 - 3;

    /* Require space for weight (if needed) */
    if (show_inven_weight) lim -= 9;


    /* Find the "final" slot */
    for (i = 0; i < INVEN_PACK; i++) {

        i_ptr = &inventory[i];

        /* Track non-empty slots */
        if (i_ptr->k_idx) z = i + 1;
    }

    /* Display the inventory */
    for (k = 0, i = 0; i < z; i++) {

        i_ptr = &inventory[i];

        /* Is this item acceptable? */
        if (!item_tester_okay(i_ptr)) continue;

        /* Describe the object */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Hack -- enforce max length */
        i_name[lim] = '\0';

        /* Save the object index, color, and description */
        out_index[k] = i;
        out_color[k] = tval_to_attr[i_ptr->tval % 128];
        (void)strcpy(out_desc[k], i_name);

        /* Find the predicted "line length" */
        l = strlen(out_desc[k]) + 5;

        /* Be sure to account for the weight */
        if (show_inven_weight) l += 9;

        /* Maintain the maximum length */
        if (l > len) len = l;

        /* Advance to next "line" */
        k++;
    }

    /* Find the column to start in */
    col = (len > 76) ? 0 : (79 - len);

    /* Output each entry */
    for (j = 0; j < k; j++) {

        /* Get the index */
        i = out_index[j];

        /* Get the item */
        i_ptr = &inventory[i];

        /* Clear the line */
        prt("", j + 1, col ? col - 2 : col);

        /* Prepare an index --(-- */
        sprintf(tmp_val, "%c)", index_to_label(i));

        /* Clear the line with the (possibly indented) index */
        put_str(tmp_val, j + 1, col);

        /* Display the entry itself */
        c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

        /* Display the weight if needed */
        if (show_inven_weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            (void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            put_str(tmp_val, j + 1, 71);
        }
    }

    /* Make a "shadow" below the list (only if needed) */
    if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

    /* Save the new column */
    command_gap = col;
}



/*
 * Display the equipment.
 */
void show_equip(void)
{
    int			i, j, k, l;
    int			col, len, lim;

    inven_type		*i_ptr;

    char		tmp_val[80];

    char		i_name[80];

    int			out_index[23];
    byte		out_color[23];
    char		out_desc[23][80];


    /* Starting column */
    col = command_gap;
    
    /* Maximal length */
    len = 79 - col;

    /* Maximum space allowed for descriptions */
    lim = 79 - 3;

    /* Require space for labels (if needed) */
    if (show_equip_label) lim -= (14 + 2);
    
    /* Require space for weight (if needed) */
    if (show_equip_weight) lim -= 9;

    /* Scan the equipment list */
    for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        i_ptr = &inventory[i];

        /* Is this item acceptable? */
        if (!item_tester_okay(i_ptr)) continue;

        /* Description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Truncate the description */
        i_name[lim] = 0;

        /* Save the color */
        out_index[k] = i;
        out_color[k] = tval_to_attr[i_ptr->tval % 128];
        (void)strcpy(out_desc[k], i_name);

        /* Extract the maximal length (see below) */
        l = strlen(out_desc[k]) + (2 + 3);
        
        /* Increase length for labels (if needed) */
        if (show_equip_label) l += (14 + 2);

        /* Increase length for weight (if needed) */
        if (show_equip_weight) l += 9;

        /* Maintain the max-length */
        if (l > len) len = l;

        /* Advance the entry */
        k++;
    }

    /* Hack -- Find a column to start in */
    col = (len > 76) ? 0 : (79 - len);

    /* Output each entry */
    for (j = 0; j < k; j++) {

        /* Get the index */
        i = out_index[j];

        /* Get the item */
        i_ptr = &inventory[i];

        /* Clear the line */
        prt("", j + 1, col ? col - 2 : col);

        /* Prepare an index --(-- */
        sprintf(tmp_val, "%c)", index_to_label(i));

        /* Clear the line with the (possibly indented) index */
        put_str(tmp_val, j+1, col);

        /* Use labels */
        if (show_equip_label) {

            /* Mention the use */
            (void)sprintf(tmp_val, "%-14s: ", mention_use(i));
            put_str(tmp_val, j+1, col + 3);

            /* Display the entry itself */
            c_put_str(out_color[j], out_desc[j], j+1, col + 3 + 14 + 2);
        }

        /* No labels */
        else {

            /* Display the entry itself */
            c_put_str(out_color[j], out_desc[j], j+1, col + 3);
        }

        /* Display the weight if needed */
        if (show_equip_weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            (void)sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
            put_str(tmp_val, j+1, 71);
        }
    }

    /* Make a "shadow" below the list (only if needed) */
    if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);

    /* Save the new column */
    command_gap = col;
}




/*
 * Verify the choice of an item.
 */
static bool verify(cptr prompt, int item)
{
    char	i_name[80];

    char	out_val[160];


    /* Describe */
    objdes(i_name, &inventory[item], TRUE, 3);

    /* Prompt */
    (void)sprintf(out_val, "%s %s? ", prompt, i_name);

    /* Query */
    return (get_check(out_val));
}


/*
 * Hack -- allow user to "prevent" certain choices
 */
static bool get_item_allow(int i)
{
    cptr s;

    inven_type *i_ptr = &inventory[i];

    /* No inscription */
    if (!i_ptr->note) return (TRUE);

    /* Find a '!' */
    s = strchr(quark_str(i_ptr->note), '!');

    /* Process preventions */
    while (s) {

        /* Check the "restriction" */
        if ((s[1] == command_cmd) || (s[1] == '*')) {

            /* Verify the choice */
            if (!verify("Really try", i)) return (FALSE);
        }

        /* Find another '!' */
        s = strchr(s + 1, '!');
    }

    /* Allow it */
    return (TRUE);
}



/*
 * Auxiliary function for "get_item()" -- test an index
 */
static bool get_item_okay(int i)
{
    /* Illegal items */
    if ((i < 0) || (i >= INVEN_TOTAL)) return (FALSE);

    /* Verify the item */
    if (!item_tester_okay(&inventory[i])) return (FALSE);

    /* Assume okay */
    return (TRUE);
}



/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" command_cmd code.
 */
static int get_tag(int *cp, char tag)
{
    int i;
    cptr s;


    /* Check every object */
    for (i = 0; i < INVEN_TOTAL; ++i) {

        inven_type *i_ptr = &inventory[i];

        /* Skip empty objects */
        if (!i_ptr->k_idx) continue;

        /* Skip empty inscriptions */
        if (!i_ptr->note) continue;

        /* Find a '@' */
        s = strchr(quark_str(i_ptr->note), '@');

        /* Process all tags */
        while (s) {

            /* Check the normal tags */
            if (s[1] == tag) {

                /* Save the actual inventory ID */
                *cp = i;

                /* Success */
                return (TRUE);
            }

            /* Check the special tags */
            if ((s[1] == command_cmd) && (s[2] == tag)) {

                /* Save the actual inventory ID */
                *cp = i;

                /* Success */
                return (TRUE);
            }

            /* Find another '@' */
            s = strchr(s + 1, '@');
        }
    }

    /* No such tag */
    return (FALSE);
}



/*
 * Let the user select an item, return its "index"
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.  Note that
 * the equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected, we save it in "cp" and return TRUE.
 * If this "legal" item is on the floor, we use a "cp" equal to zero
 * minus the dungeon index of the item on the floor.
 *
 * Otherwise, we return FALSE, and set "cp" to:
 *   -1 for "User hit space/escape"
 *   -2 for "No legal items to choose"
 *
 * Global "command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "command_see" may be set before calling this function to start
 * out in "browse" mode.  It is cleared before this function returns.
 *
 * Global "command_wrk" is used to choose between equip/inven listings.
 * If it is TRUE then we are viewing inventory, else equipment.
 *
 * Global "command_gap" is used to indent the inven/equip tables, and
 * to provide some consistancy over time.  It shrinks as needed to hold
 * the various tables horizontally, and can only be reset by calling this
 * function with "command_see" being FALSE, or by pressing ESCAPE from
 * this function, or by hitting "escape" while viewing the inven/equip.
 *
 * We always erase the prompt when we are done.
 *
 * Note that "Term_save()" / "Term_load()" blocks must not overlap.
 */
bool get_item(int *cp, cptr pmt, bool equip, bool inven, bool floor)
{
    char        n1, n2, which = ' ';

    int		floor_item = 0;

    int		k, i1, i2, e1, e2;
    bool	ver, done, item;

    char	tmp_val[160];
    char	out_val[160];


    /* Paranoia XXX XXX XXX */
    msg_print(NULL);


    /* Not done */
    done = FALSE;

    /* No item selected */
    item = FALSE;

    /* Default to "no item" (see above) */
    *cp = -1;


    /* Paranoia */
    if (!inven && !equip) return (FALSE);


    /* Full inventory */
    i1 = 0;
    i2 = INVEN_PACK - 1;

    /* Forbid inventory */
    if (!inven) i2 = -1;

    /* Restrict inventory indexes */
    while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
    while ((i1 <= i2) && (!get_item_okay(i2))) i2--;
    

    /* Full equipment */
    e1 = INVEN_WIELD;
    e2 = INVEN_TOTAL - 1;

    /* Forbid equipment */
    if (!equip) e2 = -1;
    
    /* Restrict equipment indexes */
    while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
    while ((e1 <= e2) && (!get_item_okay(e2))) e2--;
    

    /* Hack -- Restrict floor usage */
    if (floor) {

        cave_type *c_ptr = &cave[py][px];
    
        inven_type *i_ptr = &i_list[c_ptr->i_idx];
        
        /* Accept the item on the floor if legal */
        if (item_tester_okay(i_ptr)) floor_item = c_ptr->i_idx;
    }


    /* Verify choices */
    if (!floor_item && (i1 > i2) && (e1 > e2)) {

        /* Cancel command_see */
        command_see = FALSE;

        /* Hack -- Nothing to choose */
        *cp = -2;
        
        /* Done */
        done = TRUE;
    }

    /* Analyze choices */
    else {
    
        /* Hack -- Reset display width */
        if (!command_see) command_gap = 50;

        /* Hack -- Start on equipment if requested */
        if (command_see && command_wrk && equip) {
            command_wrk = TRUE;
        }
 
        /* Use inventory if allowed */
        else if (inven) {
            command_wrk = FALSE;
        }

        /* Use equipment if allowed */
        else if (equip) {
            command_wrk = TRUE;
        }

        /* Use inventory for floor */
        else {
            command_wrk = FALSE;
        }
    }


#ifdef GRAPHIC_CHOICE

    /* Save "term_choice" */
    if (term_choice && use_choice_choose) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Save the contents */
        Term_save();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

#ifdef GRAPHIC_MIRROR

    /* Save "term_mirror" */
    if (term_mirror && use_mirror_choose) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Save the contents */
        Term_save();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

    /* Hack -- start out in "display" mode */
    if (command_see) Term_save();


    /* Repeat until done */
    while (!done) {

        /* Inventory screen */
        if (!command_wrk) {

#ifdef GRAPHIC_CHOICE

            /* Display choices in "term_choice" */
            if (term_choice && use_choice_choose) {

                /* Activate the choice window */
                Term_activate(term_choice);

                /* Display the inventory */
                display_inven();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }

#endif

#ifdef GRAPHIC_CHOICE

            /* Display choices in "term_mirror" */
            if (term_mirror && use_mirror_choose) {

                /* Activate the mirror window */
                Term_activate(term_mirror);

                /* Display the inventory */
                display_inven();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }

#endif

            /* Extract the legal requests */
            n1 = I2A(i1);
            n2 = I2A(i2);

            /* Redraw if needed */
            if (command_see) show_inven();
        }

        /* Equipment screen */
        else {

#ifdef GRAPHIC_CHOICE

            /* Display choices in "term_choice" */
            if (term_choice && use_choice_choose) {

                /* Activate the choice window */
                Term_activate(term_choice);

                /* Display the equipment */
                display_equip();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }

#endif

#ifdef GRAPHIC_MIRROR

            /* Display choices in "term_mirror" */
            if (term_mirror && use_mirror_choose) {

                /* Activate the mirror window */
                Term_activate(term_mirror);

                /* Display the equipment */
                display_equip();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }

#endif

            /* Extract the legal requests */
            n1 = I2A(e1 - INVEN_WIELD);
            n2 = I2A(e2 - INVEN_WIELD);

            /* Redraw if needed */
            if (command_see) show_equip();
        }

        /* Viewing inventory */    
        if (!command_wrk) {
        
            /* Begin the prompt */
            sprintf(out_val, "Inven:");
        
            /* Some legal items */
            if (i1 <= i2) {

                /* Build the prompt */
                sprintf(tmp_val, " %c-%c,",
                        index_to_label(i1), index_to_label(i2));

                /* Append */
                strcat(out_val, tmp_val);
            }
            
            /* Indicate ability to "view" */
            if (!command_see) strcat(out_val, " * to see,");

            /* Append */
            if (equip) strcat(out_val, " / for Equip,");
        }
    
        /* Viewing equipment */    
        else {

            /* Begin the prompt */
            sprintf(out_val, "Equip:");
        
            /* Some legal items */
            if (e1 <= e2) {

                /* Build the prompt */
                sprintf(tmp_val, " %c-%c,",
                        index_to_label(e1), index_to_label(e2));

                /* Append */
                strcat(out_val, tmp_val);
            }
            
            /* Indicate ability to "view" */
            if (!command_see) strcat(out_val, " * to see,");
        
            /* Append */
            if (inven) strcat(out_val, " / for Inven,");
        }
        
        /* Indicate legality of the "floor" item */
        if (floor_item) strcat(out_val, " - for floor,");
        
        /* Finish the prompt */
        strcat(out_val, " ESC");

        /* Build the prompt */
        sprintf(tmp_val, "(%s) %s", out_val, pmt);

        /* Show the prompt */	
        prt(tmp_val, 0, 0);


        /* Get a key */
        which = inkey();

        /* Parse it */
        switch (which) {

          case ESCAPE:

            command_gap = 50;
            done = TRUE;
            break;

          case '*':
          case '?':
          case ' ':

            /* Show/hide the list */
            if (!command_see) {
                Term_save();
                command_see = TRUE;
            }
            else {
                Term_load();
                command_see = FALSE;
            }
            break;

          case '/':

            /* Verify legality */
            if (!inven || !equip) {
                bell();
                break;
            }

            /* Fix screen */
            if (command_see) {
                Term_load();
                Term_save();
            }

            /* Switch inven/equip */
            command_wrk = !command_wrk;

            /* Need to redraw */
            break;

          case '-':

            /* Use floor item */
            if (floor_item) {
                (*cp) = 0 - floor_item;
                item = TRUE;
                done = TRUE;
            }
            else {
                bell();
            }
            break;

          case '0':
          case '1': case '2': case '3':
          case '4': case '5': case '6':
          case '7': case '8': case '9':

            /* XXX XXX Look up that tag */
            if (!get_tag(&k, which)) {
                bell();
                break;
            }

            /* Hack -- Verify item */
            if ((k < INVEN_WIELD) ? !inven : !equip) {
                bell();
                break;
            }

            /* Validate the item */
            if (!get_item_okay(k)) {
                bell();
                break;
            }

            /* Allow player to "refuse" certain actions */
            if (!get_item_allow(k)) {
                done = TRUE;
                break;
            }

            /* Use that item */
            (*cp) = k;
            item = TRUE;
            done = TRUE;
            break;

          case '\n':
          case '\r':

            /* Choose "default" inventory item */
            if (!command_wrk) {
                k = ((i1 == i2) ? i1 : -1);
            }

            /* Choose "default" equipment item */
            else {
                k = ((e1 == e2) ? e1 : -1);
            }

            /* Validate the item */
            if (!get_item_okay(k)) {
                bell();
                break;
            }

            /* Allow player to "refuse" certain actions */
            if (!get_item_allow(k)) {
                done = TRUE;
                break;
            }

            /* Accept that choice */
            (*cp) = k;
            item = TRUE;
            done = TRUE;
            break;

          default:

            /* Extract "query" setting */
            ver = isupper(which);
            if (ver) which = tolower(which);

            /* Convert letter to inventory index */
            if (!command_wrk) {
                k = label_to_inven(which);
            }

            /* Convert letter to equipment index */
            else {
                k = label_to_equip(which);
            }

            /* Validate the item */
            if (!get_item_okay(k)) {
                bell();
                break;
            }

            /* Verify, abort if requested */
            if (ver && !verify("Try", k)) {
                done = TRUE;
                break;
            }

            /* Allow player to "refuse" certain actions */
            if (!get_item_allow(k)) {
                done = TRUE;
                break;
            }

            /* Accept that choice */
            (*cp) = k;
            item = TRUE;
            done = TRUE;
            break;
        }
    }


    /* Fix the screen if necessary */
    if (command_see) Term_load();

    /* Hack -- Cancel "display" */
    command_see = FALSE;


#ifdef GRAPHIC_MIRROR

    /* Restore "term_mirror" */
    if (term_mirror && use_mirror_choose) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Load the contents */
        Term_load();

        /* Refresh */
        Term_fresh();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif

#ifdef GRAPHIC_CHOICE

    /* Restore "term_choice" */
    if (term_choice && use_choice_choose) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Load the contents */
        Term_load();

        /* Refresh */
        Term_fresh();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

#endif


    /* Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);


    /* Forget the item_tester_tval restriction */
    item_tester_tval = 0;

    /* Forget the item_tester_hook restriction */
    item_tester_hook = NULL;

    /* Clear the prompt line */
    prt("", 0, 0);

    /* Return TRUE if something was picked */
    return (item);
}



