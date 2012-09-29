/* File: desc.c */

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
 * an awareness of its identity.  Thus, we stop trying to "hide" the identity
 * of "store bought items" from the player.  So whenever an item is bought from
 * a store, the player becomes "aware" of its identity (as well as knowing the
 * object fully, via "known2()".  But when items are listed in the store, before
 * the player has actually bought one, we do not want the "flavors" to appear.
 *
 * If an item is in a store, it is guaranteed to be "known".  So perhaps it
 * should no longer be possible to see an item as "a red potion of death",
 * since once the potion type is known, all red potions are known.  Actually,
 * the "objdes_store()"....
 *
 * Summary: the "store_made" flag is now implicit in calls to the special
 * "objdes_store()" function.  And once an item is bought from a store, the
 * user instantly becomes aware of its identity.  This will yield combining
 * when necessary.
 *
 * Also note that the "aware" predicate returns TRUE if the object is known.
 */


/*
 * Hack -- note that "COLOR_MULTI" is now just "COLOR_VIOLET"
 * We will have to find a cleaner method for "MULTI_HUED" later.
 * There were only two multi-hued "flavors" (one potion, one food).
 * Plus five multi-hued "base-objects" (3 dragon scales, one blade
 * of chaos, and one something else).
 */
#define COLOR_MULTI	COLOR_VIOLET


/*
 * Color adjectives and colors, for potions.
 * Hack -- The first three are hard-coded for slime mold juice,
 * apple juice, and water, so do not scramble them.
 */

static cptr potion_adj[MAX_COLORS] = {
    "Icky Green", "Light Brown", "Clear","Azure","Blue",
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

static int8u potion_col[MAX_COLORS] = {
    COLOR_GREEN,COLOR_L_BROWN,COLOR_WHITE,COLOR_L_BLUE,COLOR_BLUE,
    COLOR_BLUE,COLOR_D_GRAY,COLOR_BROWN,COLOR_BROWN,COLOR_L_GRAY,
    COLOR_RED,COLOR_WHITE,COLOR_L_BROWN,COLOR_RED,COLOR_L_BLUE,
    COLOR_BLUE,COLOR_GREEN,COLOR_RED,COLOR_YELLOW,COLOR_GREEN,
    COLOR_GREEN,COLOR_GRAY,COLOR_GRAY,COLOR_L_GRAY,COLOR_VIOLET,
    COLOR_L_BLUE,COLOR_L_GREEN,COLOR_RED,COLOR_BLUE,COLOR_RED,
    COLOR_GREEN,COLOR_VIOLET,COLOR_L_GRAY,COLOR_ORANGE,COLOR_ORANGE,
    COLOR_L_RED,COLOR_L_RED,COLOR_VIOLET,COLOR_VIOLET,COLOR_VIOLET,
    COLOR_RED,COLOR_RED,COLOR_L_GRAY,COLOR_D_GRAY,COLOR_ORANGE,
    COLOR_VIOLET,COLOR_RED,COLOR_WHITE,COLOR_YELLOW,COLOR_VIOLET,
    COLOR_L_RED,COLOR_RED,COLOR_L_RED,COLOR_YELLOW,COLOR_GREEN,
    COLOR_MULTI,COLOR_RED
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

static int8u food_col[MAX_SHROOM] = {
    COLOR_BLUE,COLOR_D_GRAY,COLOR_D_GRAY,COLOR_BROWN,COLOR_BLUE,
    COLOR_GREEN,COLOR_RED,COLOR_YELLOW,COLOR_L_GRAY,COLOR_GREEN,
    COLOR_GRAY,COLOR_L_BLUE,COLOR_L_GREEN,COLOR_MULTI,COLOR_RED,
    COLOR_GRAY,COLOR_L_BROWN,COLOR_WHITE,COLOR_WHITE,COLOR_BROWN,
    COLOR_BROWN,/*COLOR_YELLOW,???,COLOR_RED,COLOR_L_BLUE,COLOR_ORANGE*/
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

static int8u staff_col[MAX_WOODS] = {
    COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,
    COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,
    COLOR_L_BROWN,COLOR_L_BROWN,COLOR_BROWN,COLOR_L_BROWN,COLOR_BROWN,
    COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_RED,
    COLOR_RED,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_BROWN,
    COLOR_GREEN,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_L_GRAY,COLOR_BROWN,
    COLOR_YELLOW,COLOR_GRAY,/*???,???,???,???*/
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

static int8u wand_col[MAX_METALS] = {
    COLOR_L_BLUE,COLOR_D_GRAY,COLOR_WHITE,COLOR_L_BROWN,COLOR_YELLOW,
    COLOR_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_BROWN,COLOR_RED,
    COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_WHITE,COLOR_WHITE,
    COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_BLUE,COLOR_L_BROWN,COLOR_YELLOW,
    COLOR_L_BROWN,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,
    COLOR_L_BLUE,COLOR_L_BLUE,COLOR_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,
    COLOR_WHITE,COLOR_GRAY,/*COLOR_GRAY,COLOR_WHITE,COLOR_GRAY*/
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

static int8u rod_col[MAX_METALS] = {
    COLOR_L_BLUE,COLOR_D_GRAY,COLOR_WHITE,COLOR_L_BROWN,COLOR_YELLOW,
    COLOR_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_BROWN,COLOR_RED,
    COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_WHITE,COLOR_WHITE,
    COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_BLUE,COLOR_L_BROWN,COLOR_YELLOW,
    COLOR_L_BROWN,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,COLOR_L_GRAY,
    COLOR_L_BLUE,COLOR_L_BLUE,COLOR_BROWN,COLOR_L_BROWN,COLOR_L_BROWN,
    COLOR_WHITE,COLOR_GRAY,/*COLOR_GRAY,COLOR_WHITE,COLOR_GRAY*/
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

static int8u ring_col[MAX_ROCKS] = {
    COLOR_GREEN,COLOR_VIOLET,COLOR_L_BLUE,COLOR_L_BLUE,COLOR_L_GREEN,
    COLOR_RED,COLOR_WHITE,COLOR_RED,COLOR_GRAY,COLOR_WHITE,
    COLOR_GREEN,COLOR_L_GREEN,COLOR_RED,COLOR_L_GRAY,COLOR_L_GREEN,
    COLOR_BROWN,COLOR_BLUE,COLOR_GREEN,COLOR_WHITE,COLOR_L_GRAY,
    COLOR_L_RED,COLOR_L_GRAY,COLOR_WHITE,COLOR_L_GRAY,COLOR_L_GRAY,
    COLOR_L_RED,COLOR_RED,COLOR_BLUE,COLOR_YELLOW,COLOR_YELLOW,
    COLOR_L_BLUE,COLOR_L_BROWN,COLOR_WHITE,COLOR_L_BROWN,COLOR_YELLOW,
    COLOR_D_GRAY,COLOR_L_GRAY,COLOR_BROWN,COLOR_L_BLUE,COLOR_D_GRAY,
    COLOR_YELLOW,COLOR_L_GREEN
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

static int8u amulet_col[MAX_AMULETS] = {
    COLOR_YELLOW,COLOR_L_BROWN,COLOR_WHITE,COLOR_L_GRAY,COLOR_WHITE,
    COLOR_D_GRAY,COLOR_WHITE,COLOR_L_BROWN,COLOR_L_BROWN,COLOR_GRAY,
    COLOR_BROWN,COLOR_YELLOW,COLOR_L_BLUE,COLOR_WHITE,COLOR_L_GRAY,
    COLOR_L_BROWN
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

static int8u scroll_col[MAX_TITLES];



/*
 * The "color"/"metal"/"type" of an item is its "flavor".
 *
 * Initialize descriptions for the "colored" objects, including:
 * Food, Scrolls, Potions, Wands, Staffs, Rods, Rings, Amulets.
 *
 * Hack -- make sure they stay the same for each saved game
 *
 * Note that this function does not call any others, so the
 * hack is not such a big deal as in town_gen().
 *
 * We do rods last so we don't "hurt" old save files
 */
void flavor_init(void)
{
    register int        h, i, j, k;
    register cptr		tmp1;
    register int8u		tmp2;
    vtype               string;


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
	scroll_col[h] = COLOR_WHITE;
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
}






/*
 * Return "TRUE" is the given item has a "flavor"
 */
bool flavor_p(inven_type *i_ptr)
{
    switch (i_ptr->tval) {

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
	if (i_ptr->sval < SV_FOOD_MIN_FOOD) return (TRUE);
    }

    /* No flavor */
    return (FALSE);
}



/*
 * Certain items get "known" very easily
 */
static bool known2_aux(inven_type *i_ptr)
{
    /* Analyze the "tval" */
    switch (i_ptr->tval) {

	/* Food */
	case TV_FOOD:

	    /* Some food is "always" known */
	    if (i_ptr->sval >= SV_FOOD_MIN_FOOD) return (TRUE);

	    /* XXX Fall through */

	/* Potions, Scrolls, Rods (plus some Food) */
	case TV_POTION:
	case TV_SCROLL:
	case TV_ROD:

	    /* The player must be "aware" of the item's effects */
	    if (!x_list[i_ptr->k_idx].aware) return (FALSE);

	    /* Assume knowledge */
	    return (TRUE);
	    
	/* Rings, Amulets */
	case TV_RING:
	case TV_AMULET:

	    /* Must be "aware" of the object's effect */
	    if (!x_list[i_ptr->k_idx].aware) return (FALSE);

	    /* XXX Fall through */
	    
	/* Lites (plus Rings and Amulets).  Could also do weapons/armor */
	case TV_LITE:

	    /* Check the "EASY_KNOW" flag for wearables */
	    if (i_ptr->flags3 & TR3_EASY_KNOW) return (TRUE);

	    /* Assume unknown */
	    return (FALSE);
	    
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
    }

    /* Nope */
    return (FALSE);
}


/*
 * Is a given item "fully identified"?
 */
bool known2_p(inven_type *i_ptr)
{
    /* Some items get "tagged" as known */
    if (i_ptr->ident & ID_KNOWN) return (TRUE);

    /* Hack -- auto-know certain items */
    if (known2_aux(i_ptr)) {

	/* Be ready for it next time */
	i_ptr->ident |= ID_KNOWN;

	/* This item is known */
	return (TRUE);
    }

    /* Assume not known */
    return (FALSE);    
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
void known2(inven_type *i_ptr)
{
    /* Remove "inscriptions" created when ID_FELT was set */
    if (i_ptr->ident & ID_FELT) {

	/* Hack -- Remove any inscription we may have made */
	if ((streq(i_ptr->inscrip, "cursed")) ||
	    (streq(i_ptr->inscrip, "blessed")) ||

	    (streq(i_ptr->inscrip, "terrible")) ||
	    (streq(i_ptr->inscrip, "special")) ||

	    (streq(i_ptr->inscrip, "worthless")) ||
	    (streq(i_ptr->inscrip, "excellent")) ||

	    (streq(i_ptr->inscrip, "average"))) {

	    /* Forget the inscription */
	    inscribe(i_ptr, "");
	}
    }

    /* Hack -- notice cursed items */
    else if (cursed_p(i_ptr)) {

	/* Put an initial inscription */
	inscribe(i_ptr, "cursed");
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
 * Is the player "aware" of the "flavor" of the given object?
 * The player is always "aware" of objects with no "flavor".
 * The player is "aware" of any object which he fully "knows".
 *
 * Thus, the only things the player can be "unaware of" are Potions, Scrolls,
 * Food, Amulets, Rings, Staffs, Wands, and Rods, which have unknown effects.
 */
bool inven_aware_p(inven_type *i_ptr)
{
    /* Hack -- player always knows "bland" objects */
    if (!flavor_p(i_ptr)) return (TRUE);

    /* Hack -- "known" induces "aware" */
    if (known2_p(i_ptr)) return (TRUE);
    
    /* Check the "x_list" */
    return (x_list[i_ptr->k_idx].aware);
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
 * Has the player "tried" a given object?
 */
bool inven_tried_p(inven_type *i_ptr)
{
    /* Hack -- "aware" cancels "tried" */
    if (inven_aware_p(i_ptr)) return (FALSE);
    
    /* Check the "x_list" */
    return (x_list[i_ptr->k_idx].tried);
}


/*
 * Something has been "sampled"
 */
void inven_tried(inven_type *i_ptr)
{
    /* Mark it as tried (even if "aware") */
    x_list[i_ptr->k_idx].tried = TRUE;
}





/*
 * Return the "char" for a given item
 */
char inven_char(inven_type *i_ptr)
{
    char i_char;
    
    /* Hack -- allow "graphic pictures" */
    if (inven_aware_p(i_ptr)) {

	/* Get the "user" symbol */    
        i_char = x_list[i_ptr->k_idx].x_char;

	/* Use it if possible */
	if (i_char) return (i_char);
    }

    /* Get the "user" symbol */    
    i_char = k_list[i_ptr->k_idx].i_char;

    /* Use it if possible */
    if (i_char) return (i_char);
    
    /* Oops */
    return (' ');
}




/*
 * Return the "attr" corresponding to an object.
 *
 * Un-aware objects MUST use the "flavor" attribute.
 *
 * Else, attempt to use the "user-defined" attribute.
 * Else, attempt to use the "kind-defined" attribute.
 * Else, use WHITE.
 */
byte inven_attr(inven_type *i_ptr)
{

#ifdef USE_COLOR

    int indexx;
    byte i_char;


    /* Allow "special" colors if "aware" */
    if (inven_aware_p(i_ptr)) {
    
	/* Extract the "kind" color */
	i_char = x_list[i_ptr->k_idx].x_attr;

	/* Use the color of the "kind" if given */
	if (i_char) return (i_char);
	
	/* Extract the "kind" color */
	i_char = k_list[i_ptr->k_idx].i_attr;

	/* Use the color of the "kind" if given */
	if (i_char) return (i_char);
    }


    /* Obtain the item "index" */
    indexx = i_ptr->sval & ITEM_SUBVAL_MASK;

    /* Analyze the item */
    switch (i_ptr->tval) {

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

#endif

    /* Nothing else found */
    return (COLOR_WHITE);
}









/*
 * Helper function.  Compare the "ident" field of two objects.
 */
static bool similar_ident(inven_type *i_ptr, inven_type *j_ptr)
{
    /* XXX Hack -- no longer possible (???) */
    if (inven_aware_p(i_ptr) != inven_aware_p(j_ptr)) return (0);

    /* XXX Hack -- force identical "ident" flag sets */
    if (i_ptr->ident != j_ptr->ident) return (0);

    /* Food, Potions, Scrolls, and Rods are "simple" objects */
    if (i_ptr->tval == TV_FOOD) return (1);
    if (i_ptr->tval == TV_POTION) return (1);
    if (i_ptr->tval == TV_SCROLL) return (1);
    if (i_ptr->tval == TV_ROD) return (1);

    /* XXX Mega-Hack -- missiles do not have to be identified */
    if (i_ptr->tval == TV_SHOT) return (1);
    if (i_ptr->tval == TV_BOLT) return (1);
    if (i_ptr->tval == TV_ARROW) return (1);

    /* Normally, both items must be fully "known" to stack */
    if (!known2_p(i_ptr) || !known2_p(j_ptr)) return (0);

    /* Allow match */
    return (1);
}




/*
 * Determine if an item can "absorb" a second item
 *
 * No object can absorb itself.  This prevents object replication.
 *
 * When an object absorbs another, the second object loses all
 * of its attributes (except for "number", which gets added in),
 * so it is important to verify that all important attributes match.
 *
 * These fields are ignored: k_idx, ix, iy, scost.
 *
 * Note that all "problems" associated with "combining" differently
 * priced objects have been removed by never combining such objects.
 *
 * XXX Currently, we allow identical unidentified "stackables" to
 * combine.  This includes "arrows", which is a major hack.
 *
 * We allow wands (and staffs) to combine if they are known to have
 * equivalent charges.  They are unstacked as they are used.
 *
 * We allow rods to combine when they are fully charged, and again,
 * we unstack them as they are used.
 *
 * We do not allow chests to combine, it would be annoying.
 *
 * We do NOT allow artifacts or ego-items or dragon scale mail to
 * combine, since the "activation" code would become very messy.
 */
int item_similar(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Identical items cannot be stacked */
    if (i_ptr == j_ptr) return (0);

    /* Different objects cannot be stacked */
    if (i_ptr->k_idx != j_ptr->k_idx) return (0);


    /* Different charges (etc) cannot be stacked */
    if (i_ptr->pval != j_ptr->pval) return (0);

    /* Require matching prices (prevents "value" loss) */
    if (i_ptr->cost != j_ptr->cost) return (0);


    /* Require many identical values */
    if ((i_ptr->tohit     != j_ptr->tohit)     ||
	(i_ptr->todam     != j_ptr->todam)     ||
	(i_ptr->toac      != j_ptr->toac)      ||
	(i_ptr->ac        != j_ptr->ac)        ||
	(i_ptr->damage[0] != j_ptr->damage[0]) ||
	(i_ptr->damage[1] != j_ptr->damage[1]) ||
	(i_ptr->flags1    != j_ptr->flags1)    ||
	(i_ptr->flags2    != j_ptr->flags2)    ||
	(i_ptr->flags3    != j_ptr->flags3)) {
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


    /* Require matching "inscriptions" (various reasons for this) */
    if (strcmp(i_ptr->inscrip, j_ptr->inscrip)) return (0);


    /* Paranoia -- Different types cannot be stacked */
    if (i_ptr->tval != j_ptr->tval) return (0);

    /* Paranoia -- Different sub-types cannot be stacked */
    if (i_ptr->sval != j_ptr->sval) return (0);

    /* Paranoia -- Could possibly have objects with "broken" weights */
    if (i_ptr->weight != j_ptr->weight) return (0);

    /* Paranoia -- Timeout should always be zero (see "TR3_ACTIVATE" above) */
    if (i_ptr->timeout || j_ptr->timeout) return (0);


    /* They match, so they must be similar */
    return (TRUE);
}










/*
 * defines for pval_use, determine how the pval field is printed 
 */

#define IGNORED     0		/* ignore the pval field */
#define Z_PLUSSES   1		/* show pval as "(+x)" */
#define PLUSSES     2		/* show pval as "(+x)", unless zero */
#define FLAGS       3		/* show pval as "(+x to yyy)", or "(+x)" */
#define CHARGES     5		/* show pval as "(x charges)" */
#define CHARGING    6		/* show pval as "(charging)", unless zero */
#define USE_LITE    7		/* show pval as "with x turns of light" */


/*
 * Creates a description of the item "i_ptr", and stores it in "out_val".
 *
 * If "pref" is TRUE, the description is verbose, and has an article (or number,
 * or "no more") prefixed to the description.
 *
 * Note that out_val must be large enough to hold more than 80 characters
 * (is this true?), but it seems that 160 chars will always be enough
 *
 * Originally used sprintf(buf, "%+d", val), but several machines don't
 * support it, so use sprintf(buf, "%c%d", MY_POM(val), MY_ABS(val))
 *
 * Note that "objdes_store()" forces "plain_descriptions" to appear "true",
 * so that the "flavors" of "un-aware" objects are not shown.
 *
 * Note that ALL ego-items (when known) append an "Artifact Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that ALL artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known".
 *
 * Special Lite's use the "k_list" base-name (Phial, Star, or Arkenstone).
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_list" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_list" name.
 * In all three cases above, the "artifact name" is appended if the object
 * is "known".  Some of the Special Rings/Amulets are thus "EASY_KNOW".
 * Note that not all of them can be, since the "plusses" are hidden.  But
 * it might be worth it to mark them as "EASY_KNOW" anyway, for convenience.
 *
 * There is extra processing for "The One Ring", though I disapprove...
 */
void objdes(char *out_val, inven_type *i_ptr, int pref)
{
    register cptr basenm, modstr;
    bool aware, append_name;
    int power, indexx, pval_use;
    vtype                tmp_str, damstr;
    bigvtype             tmp_val;


    /* Is the player "aware" of the object? */
    aware = (inven_aware_p(i_ptr));

    /* Hack -- Extract the sub-type "indexx" */
    indexx = i_ptr->sval & ITEM_SUBVAL_MASK;

    /* Extract the (default) "base name" */
    basenm = k_list[i_ptr->k_idx].name;

    /* Assume we will NOT append the "kind" name */
    append_name = FALSE;

    /* Assume no modifier string */
    modstr = NULL;

    /* Start with no damage string */
    damstr[0] = '\0';

    /* Assume no display of "pval" */
    pval_use = IGNORED;


    /* Analyze the object */
    switch (i_ptr->tval) {

      /* Some objects are easy to describe */
      case TV_SKELETON:
      case TV_BOTTLE:
      case TV_JUNK:
      case TV_SPIKE:
      case TV_FLASK:
	break;

      /* Hack -- Chests must be described in detail */
      case TV_CHEST:

	/* Empty chests are "obvious" */
	if (!i_ptr->flags1) {
	    strcpy(damstr, " (empty)");
	    break;
	}

	/* Not searched yet */
	if (!known2_p(i_ptr)) break;

	/* Describe the traps */
	switch (i_ptr->flags2 & CH2_TRAP_MASK) {
	    case CH2_LOSE_STR:  strcpy(damstr, " (Poison Needle)"); break;
	    case CH2_POISON:    strcpy(damstr, " (Poison Needle)"); break;
	    case CH2_PARALYSED: strcpy(damstr, " (Gas Trap)"); break;
	    case CH2_EXPLODE:   strcpy(damstr, " (Explosion Device)"); break;
	    case CH2_SUMMON:    strcpy(damstr, " (Summoning Runes)"); break;
	    default:            strcpy(damstr, " (Multiple Traps)"); break;
	}

	/* Described a trap above */
	if (i_ptr->flags2 & CH2_TRAP_MASK) break;

	/* Already disarmed */
	if (i_ptr->flags2 & CH2_DISARMED) {
	    strcpy(damstr, " (disarmed)");
	    break;
	}

	/* Already unlocked */
	if (!(i_ptr->flags2 & CH2_LOCKED)) {
	    strcpy(damstr, " (unlocked)");
	    break;
	}

	/* Assume it must be locked */
	strcpy(damstr, " (locked)");
	break;

      /* Weapons (and missiles) have a damage string, and flags */
      case TV_SHOT:
      case TV_BOLT:
      case TV_ARROW:
      case TV_HAFTED:
      case TV_POLEARM:
      case TV_SWORD:
	pval_use = FLAGS;
	(void)sprintf(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
	break;

      /* Hack -- Display "Shovel (+0)" */
      case TV_DIGGING:
	pval_use = Z_PLUSSES;
	(void)sprintf(damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
	break;

      /* Bows get a special "damage string" */
      case TV_BOW:

	/* Extract the "base power" from the bow type */
	switch (i_ptr->sval) {
	  case SV_SLING: power = 2; break;
	  case SV_SHORT_BOW: power = 2; break;
	  case SV_LONG_BOW: power = 3; break;
	  case SV_LIGHT_XBOW: power = 3; break;
	  case SV_HEAVY_XBOW: power = 4; break;
	  default: power = 0; break;
	}

	/* Apply the "Extra Might" flag */
	if (i_ptr->flags3 & TR3_XTRA_MIGHT) power++;

	/* Build the damage string */
	sprintf(damstr, " (x%d)", power);

	/* Show flags, if any */
	pval_use = FLAGS;

	/* All done */
	break;


      /* Armour uses flags */
      case TV_BOOTS:
      case TV_GLOVES:
      case TV_CLOAK:
      case TV_HELM:
      case TV_SHIELD:
      case TV_SOFT_ARMOR:
      case TV_HARD_ARMOR:
      case TV_DRAG_ARMOR:
	pval_use = FLAGS;
	break;


      /* Lites (including a few "Specials") */
      case TV_LITE:

	pval_use = USE_LITE;

	/* Special Lites do NOT show "turns of light" */
	if (artifact_p(i_ptr)) {
	    pval_use = IGNORED;
	}

	break;


      /* Amulets (including a few "Specials") */
      case TV_AMULET:

	pval_use = FLAGS;

	if (!aware) {
	    basenm = "& %s Amulet~";
	    modstr = amulet_adj[indexx];
	}
	else if (artifact_p(i_ptr)) {
	    /* Use default values */
	}
	else if (!plain_descriptions) {
	    basenm = "& %s Amulet~";
	    modstr = amulet_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Amulet~";
	    append_name = TRUE;
	}
	break;


      /* Rings (including a few "Specials") */
      case TV_RING:

	pval_use = PLUSSES;

	if (!aware) {
	    basenm = "& %s Ring~";
	    modstr = ring_adj[indexx];
	    if (i_ptr->sval == SV_RING_POWER) modstr = "Plain Gold";
	}
	else if (artifact_p(i_ptr)) {
	    /* Use default values */
	}
	else if (!plain_descriptions) {
	    basenm = "& %s Ring~";
	    modstr = ring_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Ring~";
	    append_name = TRUE;
	}
	break;

      case TV_STAFF:
	pval_use = CHARGES;
	if (!aware) {
	    basenm = "& %s Staff~";
	    modstr = staff_adj[indexx];
	}
	else if (!plain_descriptions) {
	    basenm = "& %s Staff~";
	    modstr = staff_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Staff~";
	    append_name = TRUE;
	}
	break;

      case TV_WAND:
	pval_use = CHARGES;
	if (!aware) {
	    basenm = "& %s Wand~";
	    modstr = wand_adj[indexx];
	}
	else if (!plain_descriptions) {
	    basenm = "& %s Wand~";
	    modstr = wand_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Wand~";
	    append_name = TRUE;
	}
	break;

      case TV_ROD:
	pval_use = CHARGING;
	if (!aware) {
	    basenm = "& %s Rod~";
	    modstr = rod_adj[indexx];
	}
	else if (!plain_descriptions) {
	    basenm = "& %s Rod~";
	    modstr = rod_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Rod~";
	    append_name = TRUE;
	}
	break;

      case TV_SCROLL:
	if (!aware) {
	    basenm = "& Scroll~ titled \"%s\"";
	    modstr = scroll_adj[indexx];
	}
	else if (!plain_descriptions) {
	    basenm = "& Scroll~ titled \"%s\"";
	    modstr = scroll_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Scroll~";
	    append_name = TRUE;
	}
	break;

      case TV_POTION:
	if (!aware) {
	    basenm = "& %s Potion~";
	    modstr = potion_adj[indexx];
	}
	else if (!plain_descriptions) {
	    basenm = "& %s Potion~";
	    modstr = potion_adj[indexx];
	    append_name = TRUE;
	}
	else {
	    basenm = "& Potion~";
	    append_name = TRUE;
	}
	break;

      case TV_FOOD:

	/* Ordinary food is "boring" */
	if (i_ptr->sval >= SV_FOOD_MIN_FOOD) break;

	/* The Molds */
	if (i_ptr->sval >= SV_FOOD_MIN_MOLD) {
	    if (!aware) {
		basenm = "& Hairy %s Mold~";
		modstr = food_adj[indexx];
	    }
	    else if (!plain_descriptions) {
		basenm = "& Hairy %s Mold~";
		modstr = food_adj[indexx];
		append_name = TRUE;
	    }
	    else {
		basenm = "& Hairy Mold~";
		append_name = TRUE;
	    }
	}

	/* The Mushrooms */
	else {

	    if (!aware) {
		basenm = "& %s Mushroom~";
		modstr = food_adj[indexx];
	    }
	    else if (!plain_descriptions) {
		basenm = "& %s Mushroom~";
		modstr = food_adj[indexx];
		append_name = TRUE;
	    }
	    else {
		basenm = "& Mushroom~";
		append_name = TRUE;
	    }
	}
	break;


      /* Magic Books */
      case TV_MAGIC_BOOK:
	modstr = basenm;
	basenm = "& Book~ of Magic Spells %s";
	break;

      /* Prayer Books */
      case TV_PRAYER_BOOK:
	modstr = basenm;
	basenm = "& Holy Book~ of Prayers %s";
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


      /* Gold */
      case TV_GOLD:
	strcpy(out_val, basenm);
	return;

      case TV_STORE_DOOR:
	sprintf(out_val, "the entrance to the %s", basenm);
	return;

      default:
	sprintf(out_val, "*** error in objdes(%d) ***", i_ptr->k_idx);
	return;
    }


    /* Insert the modifier */
    if (modstr) {
	sprintf(tmp_val, basenm, modstr);
    }
    else {
	strcpy(tmp_val, basenm);
    }


    /* Append the "kind name" to the "base name" */
    if (append_name) {
	(void)strcat(tmp_val, " of ");
	(void)strcat(tmp_val, k_list[i_ptr->k_idx].name);
    }


    /* Attempt to pluralize somewhat correctly */
    if (i_ptr->number != 1) {
	insert_str(tmp_val, "s~", "ses");
	insert_str(tmp_val, "x~", "xes");
	insert_str(tmp_val, "sh~", "shes");
	insert_str(tmp_val, "ch~", "ches");
	insert_str(tmp_val, "~", "s");
    }

    /* Delete the plural, if any */
    else {
	insert_str(tmp_val, "~", NULL);
    }


    /* Hack -- No prefixes requested */
    if (!pref) {

	cptr skip = tmp_val;

	/* Delete the count symbol */
	if (skip[0] == '&') skip += 2;

	/* Use the name (without the "&") */
	(void)strcpy(out_val, skip);

	/* Short answer... */
	return;
    }


    /* Hack -- Append "Artifact" or "Special" names */
    if (known2_p(i_ptr)) {

	/* Hack -- grab the artifact name */
	if (i_ptr->name1) {
	    (void)strcat(tmp_val, " ");
	    (void)strcat(tmp_val, v_list[i_ptr->name1].name);
	}

	/* Otherwise, grab the "ego-item" name */
	else if (i_ptr->name2) {
	    (void)strcat(tmp_val, " ");
	    (void)strcat(tmp_val, ego_names[i_ptr->name2]);
	}
    }

    /* Append the "damage info", if any */
    if (damstr[0]) {
	(void)strcat(tmp_val, damstr);
    }


    /* We know it, describe it */	
    if (known2_p(i_ptr)) {

	char *tail = tmp_val + strlen(tmp_val);

	/* Add the tohit/todam */
	if (wearable_p(i_ptr) && (i_ptr->flags3 & TR3_SHOW_MODS)) {
	    (void)sprintf(tail, " (%c%d,%c%d)",
			  MY_POM(i_ptr->tohit), MY_ABS(i_ptr->tohit),
			  MY_POM(i_ptr->todam), MY_ABS(i_ptr->todam));
	}

	/* Add the tohit */
	else if (i_ptr->tohit) {
	    (void)sprintf(tail, " (%c%d)",
			  MY_POM(i_ptr->tohit), MY_ABS(i_ptr->tohit));
	}

	/* Add the todam */
	else if (i_ptr->todam) {
	    (void)sprintf(tail, " (%c%d)",
			  MY_POM(i_ptr->todam), MY_ABS(i_ptr->todam));
	}
    }


    /* Add in the "armor class info", base and magic */
    /* Crowns have a zero base AC, so make a special test for them. */
    if (i_ptr->ac != 0 || (i_ptr->tval == TV_HELM)) {
	char *tail;
	cptr b1 = "[", b2 = "]";            
	tail = tmp_val + strlen(tmp_val);
	(void)sprintf(tail, " %s%d", b1, i_ptr->ac);
	if (known2_p(i_ptr)) {
	    tail = tail + strlen(tail);
	    (void)sprintf(tail, ",%c%d",
			  MY_POM(i_ptr->toac), MY_ABS(i_ptr->toac));
	}
	tail = tail + strlen(tail);
	(void)strcpy(tail, b2);
    }

    /* No base armor, but does increase armor */
    else if (i_ptr->toac && known2_p(i_ptr)) {
	char *tail = tmp_val + strlen(tmp_val);
	(void)sprintf(tail, " [%c%d]",
		      MY_POM(i_ptr->toac), MY_ABS(i_ptr->toac));
    }


    /* Unknown things cannot display the charge */
    if (!known2_p(i_ptr)) pval_use = IGNORED;

    /* Hack -- some objects "Hide" the "Type" */
    if (wearable_p(i_ptr) && (i_ptr->flags3 & TR3_HIDE_TYPE)) {

	/* Do not show the "type", just the bonus */
	if (pval_use == FLAGS) pval_use = PLUSSES;
    }


    /* Erase tmp_str */
    tmp_str[0] = '\0';

    /* Nothing to add */
    if (pval_use == IGNORED) {
	/* Nothing */
    }

    /* Hack -- Boring Shovels */
    else if (pval_use == Z_PLUSSES) {
	(void)sprintf(tmp_str, "%c%d",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }

    /* Torches and Lanterns have predictable life */
    else if (pval_use == USE_LITE) {
	(void)sprintf(tmp_str, "with %d turns of light", i_ptr->pval);
    }

    /* Wands and Staffs have charges */
    else if (pval_use == CHARGES) {
	    (void)sprintf(tmp_str, "%d charge%s",
			  i_ptr->pval, (i_ptr->pval == 1 ? "" : "s"));
    }

    /* Nothing to declare */
    else if (i_ptr->pval == 0) {
	/* Nothing */
    }

    /* Rods, if not charged yet */
    else if (pval_use == CHARGING) {
	(void)strcpy(tmp_str, "charging");
    }

    /* Boring objects */        
    else if (pval_use == PLUSSES) {
	(void)sprintf(tmp_str, "%c%d",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }

    /* Hack -- Everything else is a flag */
    else if (pval_use != FLAGS) {
	/* Nothing */
    }

    /* Speed */
    else if ((i_ptr->flags1 & TR1_SPEED) &&
	     (i_ptr->name2 != EGO_SPEED)) {
	(void)sprintf(tmp_str, "%c%d to speed",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }

    /* Search (Hack -- display redundant info?) */
    else if (i_ptr->flags1 & TR1_SEARCH) {
	/* && (i_ptr->name2 != EGO_SEARCH) */
	(void)sprintf(tmp_str, "%c%d to searching",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }

    /* Stealth */
    else if ((i_ptr->flags1 & TR1_STEALTH) &&
	     (i_ptr->name2 != EGO_STEALTH)) {
	(void)sprintf(tmp_str, "%c%d to stealth",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }

    /* Infravision */
    else if ((i_ptr->flags1 & TR1_INFRA) &&
	     (i_ptr->name2 != EGO_INFRAVISION)) {
	(void)sprintf(tmp_str, "%c%d to infravision",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }

    /* Attack speed */
    else if (i_ptr->flags1 & TR1_ATTACK_SPD) {
	(void)sprintf(tmp_str, "%c%d attack%s",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval),
		      ((MY_ABS(i_ptr->pval) == 1) ? "" : "s"));
    }

    /* Default to Boring Plusses */
    else {
	(void)sprintf(tmp_str, "%c%d",
		      MY_POM(i_ptr->pval), MY_ABS(i_ptr->pval));
    }


    /* Extract the extra info, if any */
    if (tmp_str[0]) {
	char *tail = tmp_val + strlen(tmp_val);
	(void)sprintf(tail, " (%s)", tmp_str);
    }


    /* The object "expects" a "number" */
    if (tmp_val[0] == '&') {

	/* Hack -- None left */
	if (i_ptr->number < 1) {
	    (void)sprintf(out_val, "%s%s", "no more", &tmp_val[1]);
	}

	/* Extract the number */
	else if (i_ptr->number > 1) {
	    (void)sprintf(out_val, "%d%s", (int)i_ptr->number, &tmp_val[1]);
	}

	/* Hack -- The only one of its kind */
	else if (known2_p(i_ptr) && artifact_p(i_ptr)) {
	    (void)sprintf(out_val, "The%s", &tmp_val[1]);
	}

	/* A single one, with a vowel */
	else if (is_a_vowel(tmp_val[2])) {
	    (void)sprintf(out_val, "an%s", &tmp_val[1]);
	}

	/* A single one, without a vowel */
	else {
	    (void)sprintf(out_val, "a%s", &tmp_val[1]);
	}
    }

    /* Hack -- objects that never take an article */
    else {

	/* Hack -- all gone */
	if (i_ptr->number < 1) {
	    (void)sprintf(out_val, "no more %s", tmp_val);
	}

	/* Prefix a number if required */
	else if (i_ptr->number > 1) {
	    (void)sprintf(out_val, "%d %s", (int)i_ptr->number, tmp_val);
	}

	/* Hack -- The only one of its kind */
	else if (known2_p(i_ptr) && artifact_p(i_ptr)) {
	    (void)sprintf(out_val, "The %s", tmp_val);
	}

	/* Hack -- single items get no prefix */
	else {
	    (void)strcpy(out_val, tmp_val);
	}
    }


    /* Start with the user's inscription */
    strcpy(tmp_str, i_ptr->inscrip);


    /* Hack -- create a "fake" inscription */
    if (!tmp_str[0]) {

	/* If the item is "known", only inscribe curses */
	if (known2_p(i_ptr)) {
	    if (cursed_p(i_ptr)) (void)strcat(tmp_str, "cursed");
	}

	/* Note "cursed" if any curse has been felt */
	else if ((i_ptr->ident & ID_FELT) && (cursed_p(i_ptr))) {
	    (void)strcat(tmp_str, "cursed");
	}

	/* Note "worn" if the object has been worn */
	else if (i_ptr->ident & ID_WORN) {
	    (void)strcat(tmp_str, "tested");
	}

	/* Note "tried" if the object has been tested */
	else if (inven_tried_p(i_ptr)) {
	    (void)strcat(tmp_str, "tried");
	}
    }


    /* Hack -- note empty wands/staffs */
    if (!known2_p(i_ptr) && (i_ptr->ident & ID_EMPTY)) {
	if (tmp_str[0]) strcat(tmp_str, " ");
	(void)strcat(tmp_str, "empty");
    }


    /* If we created an inscription, append it */
    if (tmp_str[0]) {
	char *tail = out_val + strlen(out_val);
	(void)sprintf(tail, " {%s}", tmp_str);
    }
}


/*
 * Hack -- describe an item currently in a store's inventory
 */
void objdes_store(char *buf, inven_type *i_ptr, int mode)
{
    /* Save the "known" flag */
    bool hack_known = i_ptr->ident & ID_KNOWN;
    
    /* Save the actual "plain_descriptions" flag */
    bool hack_plain = plain_descriptions;

    /* Force plain descriptions of store items */
    plain_descriptions = TRUE;

    /* Force "known" */
    i_ptr->ident |= ID_KNOWN;
    
    /* Describe the (known) object, with no "adjectives" */
    objdes(buf, i_ptr, mode);

    /* Replace the global option */
    plain_descriptions = hack_plain;      

    /* Restore the "known" flag */
    if (!hack_known) i_ptr->ident &= ~ID_KNOWN;
}




/*
 * Make "i_ptr" a "clean" copy of the given "kind" of object
 */
void invcopy(inven_type *i_ptr, int k_idx)
{
    register inven_kind *k_ptr;

    /* Get the object template */
    k_ptr = &k_list[k_idx];

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
    i_ptr->damage[0] = k_ptr->damage[0];
    i_ptr->damage[1] = k_ptr->damage[1];

    /* Default cost and flags */
    i_ptr->cost = k_ptr->cost;
    i_ptr->flags1 = k_ptr->flags1;
    i_ptr->flags2 = k_ptr->flags2;
    i_ptr->flags3 = k_ptr->flags3;

    /* Wipe the inscription */
    inscribe(i_ptr, "");

    /* No artufact name */
    i_ptr->name1 = 0;

    /* No special name */
    i_ptr->name2 = 0;

    /* No ident info yet */
    i_ptr->ident = 0;

    /* No location yet */
    i_ptr->ix = i_ptr->iy = 0;

    /* No store cost yet */
    i_ptr->scost = 0;

    /* Fully "primed" */
    i_ptr->timeout = 0;
}






