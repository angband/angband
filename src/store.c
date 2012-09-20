/* File: store.c */

/* Purpose: store code, updating store inventory, pricing objects */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


/*
 * Note that this file is in somewhat poor shape, mainly due to the
 * complex code currently used for (1) haggling and (2) determining
 * the price of an item.  Stay tuned for changes...
 */


#undef CTRL
#define CTRL(X) ((X) & 037)


static cptr comment1[14] = {
    "Okay. ", "Fine. ", "Accepted! ", "Agreed! ", "Done! ", "Taken! ",
    "You drive a hard bargain, but taken. ",
    "You'll force me bankrupt, but it's a deal. ", "Sigh.  I'll take it. ",
    "My poor sick children may starve, but done! ", "Finally!  I accept. ",
    "Robbed again. ", "A pleasure to do business with you! ",
    "My spouse will skin me, but accepted. "
};

static cptr comment2a[3] = {
    "%A2 is my final offer; take it or leave it.",
    "I'll give you no more than %A2.",
    "My patience grows thin.  %A2 is final."
};

static cptr comment2b[16] = {
    "%A1 for such a fine item?  HA!  No less than %A2.",
    "%A1 is an insult!  Try %A2 gold pieces.",
    "%A1?!?  Go try Londis instead!",
    "Why, I'll take no less than %A2 gold pieces.",
    "Ha!  No less than %A2 gold pieces.",
    "Thou knave!  No less than %A2 gold pieces.",
    "%A1 is far too little, how about %A2?",
    "I paid more than %A1 for it myself, try %A2.",
    "%A1 my arse!  How about %A2 gold pieces?",
    "As scrap this would bring %A1.  Try %A2 in gold.",
    "May the fleas of 1000 orcs molest you.  I want %A2.",
    "My mother you can get for %A1, this costs %A2.",
    "May your most favourite parts go mouldy!  I want %A2 in gold!",
    "Sell this for such a pittance?  Give me %A2 gold.",
    "May Morgoth find you tasty!  %A2 gold pieces?",
    "Your mother was a Troll/Orc/Elf!  %A2 or I'll tell."
};

static cptr comment3a[3] = {
    "I'll pay no more than %A1; take it or leave it.",
    "You'll get no more than %A1 from me.",
    "%A1 and that's final."
};

static cptr comment3b[15] = {
    "%A2 for that piece of junk?  No more than %A1.",
    "For %A2 I could own a bundle of those.  Try %A1.",
    "%A2?  NEVER!  %A1 is more like it.",
    "Let's be reasonable...NOT! How about %A1 gold pieces?",
    "%A1 gold for that! That's you, that is!.",
    "%A1 gold pieces and be thankful for it!",
    "%A1 gold pieces and not a copper more.",
    "%A2 gold?  HA!  %A1 is more like it.", "Try about %A1 gold.",
    "I wouldn't pay %A2 for your bottom, try %A1.",
    "*CHOKE* For that!?  Let's say %A1.", "How about %A1?",
    "That looks war surplus!  Say %A1 gold.",
    "I'll buy it as scrap for %A1.",
    "%A2 is too much, let us say %A1 gold."
};

static cptr comment4a[5] = {
    "ENOUGH!  You have abused me once too often!",
    "THAT DOES IT!  You shall waste my time no more!",
    "This is getting nowhere.  I'm going to Londis!",
    "BAHAHAHAHAHA!  No more shall you insult me!",
    "Begone!  I have had enough abuse for one day."
};

static cptr comment4b[5] = {
    "Out of my place!", "out... Out... OUT!!!", "Come back tomorrow.",
    "Leave my place.  Begone!", "Come back when thou art richer."
};

static cptr comment5[10] = {
    "You will have to do better than that!", "That's an insult!",
    "Do you wish to do business or not?", "Hah!  Try again.",
    "Ridiculous!", "You've got to be kidding!", "You'd better be kidding!",
    "You try my patience.", "I don't hear you.",
    "Hmmm, nice weather we're having."
};

static cptr comment6[5] = {
    "I must have heard you wrong.", "What was that?",
    "I'm sorry, say that again.", "What did you say?",
    "Sorry, what was that again?"
};



/*
 * Comments vary, and can take parameters.	-RAK-
 *
 * %A1 is replaced by offer, %A2 by asking price.
 */


/*
 * Replace the first instance of "target" in "buf" with "insert"
 * If "insert" is NULL, just remove the first instance of "target"
 * In either case, return TRUE if "target" is found.
 *
 * XXX Could be made more efficient, especially in the
 * case where "insert" is smaller than "target".
 */
bool insert_str(char *buf, cptr target, cptr insert)
{
    int   i, len;
    int		   b_len, t_len, i_len;

    /* Attempt to find the target (modify "buf") */
    buf = strstr(buf, target);

    /* No target found */
    if (!buf) return (FALSE);

    /* Be sure we have an insertion string */
    if (!insert) insert = "";

    /* Extract some lengths */
    t_len = strlen(target);
    i_len = strlen(insert);
    b_len = strlen(buf);

    /* How much "movement" do we need? */
    len = i_len - t_len;

    /* We need less space (for insert) */
    if (len < 0) {
        for (i = t_len; i < b_len; ++i) buf[i+len] = buf[i];
    }

    /* We need more space (for insert) */
    else if (len > 0) {
        for (i = b_len-1; i >= t_len; --i) buf[i+len] = buf[i];
    }

    /* If movement occured, we need a new terminator */
    if (len) buf[b_len+len] = '\0';

    /* Now copy the insertion string */
    for (i = 0; i < i_len; ++i) buf[i] = insert[i];

    /* Successful operation */
    return (TRUE);
}


/*
 * Given a buffer, replace the first occurance of the string "target"
 * with the textual form of the long integer "number"
 */
static bool insert_lnum(char *buf, cptr target, s32b number)
{
    char	   insert[32];

    /* Prepare a string to insert */
    sprintf(insert, "%ld", (long)number);

    /* Insert it */
    return (insert_str(buf, target, insert));
}


/*
 * Successful haggle.
 */
static void say_comment1(void)
{
    msg_print(comment1[rand_int(14)]);
}


/*
 * Continue haggling (player is buying)
 */
static void say_comment2(s32b offer, s32b asking, int final)
{
    char	comment[160];

    if (final > 0) {
        (void)strcpy(comment, comment2a[rand_int(3)]);
    }
    else {
        (void)strcpy(comment, comment2b[rand_int(16)]);
    }

    insert_lnum(comment, "%A1", offer);
    insert_lnum(comment, "%A2", asking);

    msg_print(comment);
}


/*
 * Continue haggling (player is selling)
 */
static void say_comment3(s32b offer, s32b asking, int final)
{
    char	comment[160];

    if (final > 0) {
        (void)strcpy(comment, comment3a[rand_int(3)]);
    }
    else {
        (void)strcpy(comment, comment3b[rand_int(15)]);
    }

    insert_lnum(comment, "%A1", offer);
    insert_lnum(comment, "%A2", asking);

    msg_print(comment);
}


/*
 * Kick 'da bum out.					-RAK-	
 */
static void say_comment4(void)
{
    msg_print(comment4a[rand_int(5)]);
    msg_print(comment4b[rand_int(5)]);
}


/*
 * You are insulting me
 */
static void say_comment5(void)
{
    msg_print(comment5[rand_int(10)]);
}


/*
 * That makes no sense.
 */
static void say_comment6(void)
{
    msg_print(comment6[rand_int(5)]);
}






/*
 * Hack -- Kind index objects that may appear in the stores
 *
 * XXX This whole function is really just a giant hack... XXX
 */

static u16b store_choice[MAX_STORES][STORE_CHOICES] = {

        /* General Store */
    { 21, 21, 21, 21, 21, 21, 22, 23, 26, 27,
      84, 87, 123, 123, 123, 223, 224, 345, 345, 345,
      346, 346, 346, 346, 347, 348, 348, 348, 348, 348 },

        /* Armoury */
    { 91, 91, 92, 94, 94, 95, 96, 101, 101, 103,
      103, 104, 105, 106, 107, 107, 108, 109, 109, 111,
      112, 113, 121, 125, 125, 126, 128, 128, 129, 130 },

        /* Weaponsmith */
    { 31, 32, 33, 34, 35, 38, 39, 41, 42, 43,
      44, 45, 46, 49, 59, 62, 63, 64, 65, 66,
      68, 70, 73, 74, 75, 77, 78, 80, 82, 83 },

        /* Temple */
    { 48, 49, 50, 52, 53, 54, 55, 56, 58, 180,
      217, 218, 220, 233, 237, 240, 241, 257, 260, 261,
      262, 334, 334, 334, 335, 335, 335, 336, 336, 337 },

        /* Alchemy shop */
    { 173, 174, 175, 176, 176, 176, 176, 181, 181, 181,
      185, 185, 188, 189, 192, 193, 194, 197, 201, 206,
      210, 220, 220, 220, 227, 230, 233, 236, 252, 253 },

        /* Magic-User store */
    { 137, 142, 153, 164, 167, 168, 277, 278, 279, 282,
      292, 300, 301, 302, 303, 306, 313, 316, 318, 326,
      328, 330, 330, 330, 331, 331, 331, 332, 332, 333 }
};



/*
 * Note the "name" of the store with index N can be found as
 * "k_list[OBJ_STORE_LIST+N].name".  This array is also accessed
 * from the "Death" Routine.
 */

store_type store[MAX_STORES];

/*
 * Store owners have different characteristics for pricing and haggling
 * Note: Store owners should be added in groups, one for each store
 */

static owner_type owners[MAX_OWNERS] = {

{"Rincewind the Chicken  (Human)",
          450,	175,  108,    4,  0, 12},
{"Mauglin the Grumpy     (Dwarf)",
        32000,	200,  112,    4,  5,  5},
{"Arndal Beast-Slayer    (Half-Elf)"  ,
        10000,	185,  110,    5,  1,  8},
{"Ludwig the Humble      (Human)",
         5000,	175,  109,    6,  0, 15},
{"Ga-nat the Greedy      (Gnome)",
        12000,	220,  115,    4,  4,  9},
{"Luthien Starshine      (Elf)"   ,
        32000,	175,  110,    5,  2, 11},
{"Durwin the Shifty      (Human)" ,
        32000,	250,  155,   10,  0,  5},
{"Your home",
            0,  100,  100,    0, 99, 99},

{"Bilbo the Friendly     (Hobbit)",
          300,	170,  108,    5,  3, 15},
{"Darg-Low the Grim      (Human)",
        10000,	190,  111,    4,  0,  9},
{"Oglign Dragon-Slayer   (Dwarf)"  ,
        32000,	195,  112,    4,  5,  8},
{"Gunnar the Paladin     (Human)",
        12000,	185,  110,    5,  0, 23},
{"Mauser the Chemist     (Half-Elf)",
        10000,	190,  111,    5,  1,  8},
{"Buggerby the Great!    (Gnome)",
        20000,	215,  113,    6,  4, 10},
{"Histor the Goblin      (Orc)",
        32000,	250,  160,   10,  6,  5},
{"Your sweet abode",
            0,  100,  100,    0, 99, 99},

{"Lyar-el the Comely     (Elf)",
          600,	165,  107,    6,  2, 18},
{"Decado the Handsome    (Human)",
        25000,  200,  112,    4,  0, 10},
{"Ithyl-Mak the Beastly  (Half-Troll)",
         6000,	210,  115,    6,  7,  8},
{"Delilah the Pure       (Half-Elf)",
        25000,	180,  107,    6,  1, 20},
{"Wizzle the Chaotic     (Hobbit)",
        10000,	190,  110,    6,  3,  8},
{"Inglorian the Mage     (Human?)"   ,
        32000,	200,  110,    7,  0, 10},
{"Drago the Fair?        (Elf)" ,
        32000,	250,  150,   10,  2,  5},
{"Your house",
            0,  100,  100,    0, 99, 99}
};


/*
 * Buying and selling adjustments for character race VS store
 * owner race							
 */

static byte rgold_adj[MAX_RACES][MAX_RACES] = {

                        /*Hum, HfE, Elf,  Hal, Gno, Dwa, HfO, HfT, Dun, HiE*/

/*Human		 */	 { 100, 105, 105, 110, 113, 115, 120, 125, 100, 105},
/*Half-Elf	 */	 { 110, 100, 100, 105, 110, 120, 125, 130, 110, 100},
/*Elf		 */	 { 110, 105, 100, 105, 110, 120, 125, 130, 110, 100},
/*Halfling	 */	 { 115, 110, 105,  95, 105, 110, 115, 130, 115, 105},
/*Gnome		 */	 { 115, 115, 110, 105,  95, 110, 115, 130, 115, 110},
/*Dwarf		 */	 { 115, 120, 120, 110, 110,  95, 125, 135, 115, 120},
/*Half-Orc	 */	 { 115, 120, 125, 115, 115, 130, 110, 115, 115, 125},
/*Half-Troll	 */	 { 110, 115, 115, 110, 110, 130, 110, 110, 110, 115},
/*Dunedain 	 */	 { 100, 105, 105, 110, 113, 115, 120, 125, 100, 105},
/*High_Elf	 */	 { 110, 105, 100, 105, 110, 120, 125, 130, 110, 100}

};



/*
 * We store the current "store number" here so everyone can access it
 */
static int store_num = 0;

/*
 * We store the current "store page" here so everyone can access it
 */
static int store_top = 0;

/*
 * We store the current "store pointer" here so everyone can access it
 */
static store_type *st_ptr = NULL;

/*
 * We store the current "owner type" here so everyone can access it
 */
static owner_type *ot_ptr = NULL;

/*
 * Hack -- Last "increment" during haggling
 */
static s32b last_inc = 0L;





/*
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b item_value_base(inven_type *i_ptr)
{
    inven_kind *k_ptr = &k_list[i_ptr->k_idx];

    /* Aware item -- use template cost */
    if (inven_aware_p(i_ptr)) return (k_ptr->cost);

    /* Unknown food is cheap */
    if (i_ptr->tval == TV_FOOD) return (1L);

    /* Unknown Scrolls are cheap */
    if (i_ptr->tval == TV_SCROLL) return (20L);

    /* Unknown Potions are cheap */
    if (i_ptr->tval == TV_POTION) return (20L);

    /* Unknown Rings are cheap */
    if (i_ptr->tval == TV_RING) return (45L);

    /* Unknown Amulets are cheap */
    if (i_ptr->tval == TV_AMULET) return (45L);

    /* Unknown Wands are Cheap */
    if (i_ptr->tval == TV_WAND) return (50L);

    /* Unknown Staffs are Cheap */
    if (i_ptr->tval == TV_STAFF) return (70L);

    /* Unknown Rods are Cheap */
    if (i_ptr->tval == TV_ROD) return (75L);

    /* Hack -- Oops */
    return (0L);
}


/*
 * Return the price of an item including plusses (and charges)
 *
 * This function returns the "value" of ONE of the item's objects.
 *
 * Never charge extra for "unknown" bonuses or properties
 *
 * Hack -- Do not give a "zero" cost to a cursed item unless the
 * player knows that it is cursed, since that would identify them.
 *
 * Wand and staffs get cost for each (known) charge
 *
 * Armor is worth an extra 100 gold per bonus point to armor class.
 *
 * Weapons are worth an extra 100 gold per bonus point (AC,TH,TD).
 *
 * Missiles are only worth 5 gold per bonus point, since they
 * usually appear in groups of 20, and we want the player to get
 * the same amount of cash for any "equivalent" item.
 *
 * Armor with a negative armor bonus is worthless.
 * Weapons with negative hit+damage bonuses are worthless.
 *
 * Note that discounted items stay discounted forever, even if
 * the discount is "forgotten" by the player via memory loss.
 */
s32b item_value(inven_type *i_ptr)
{
    s32b value = i_ptr->cost;


    /* Known worthless items */
    if (inven_known_p(i_ptr) && (i_ptr->cost <= 0L)) return (0L);

    /* Known cursed items */
    if (inven_known_p(i_ptr) && cursed_p(i_ptr)) return (0L);

    /* Hack -- Felt cursed items */
    if ((i_ptr->ident & ID_FELT) && cursed_p(i_ptr)) return (0L);


    /* Unknown items -- acquire a base value */
    if (!inven_known_p(i_ptr)) {

        /* Base value (see above) */
        value = item_value_base(i_ptr);
    }


    /* Wands/Staffs -- Pay extra for identified charges */
    else if ((i_ptr->tval == TV_WAND) ||
             (i_ptr->tval == TV_STAFF)) {

        /* Pay extra for charges */
        value += ((value / 20) * i_ptr->pval);
    }

    /* Rings/Amulets -- pay extra for armor bonuses */
    else if ((i_ptr->tval == TV_RING) ||
             (i_ptr->tval == TV_AMULET)) {

        /* Hack -- negative bonuses are bad */
        if (i_ptr->pval < 0) return (0L);
        if (i_ptr->toac < 0) return (0L);
        if (i_ptr->tohit < 0) return (0L);
        if (i_ptr->todam < 0) return (0L);

        /* Give credit for bonuses */
        value += ((i_ptr->tohit + i_ptr->todam) * 100L);
        value += ((i_ptr->toac + i_ptr->pval) * 100L);
    }

    /* Armour -- pay extra for armor bonuses */
    else if ((i_ptr->tval == TV_BOOTS) ||
             (i_ptr->tval == TV_GLOVES) ||
             (i_ptr->tval == TV_CLOAK) ||
             (i_ptr->tval == TV_CROWN) ||
             (i_ptr->tval == TV_HELM) ||
             (i_ptr->tval == TV_SHIELD) ||
             (i_ptr->tval == TV_SOFT_ARMOR) ||
             (i_ptr->tval == TV_HARD_ARMOR) ||
             (i_ptr->tval == TV_DRAG_ARMOR)) {

        /* Hack -- negative armor bonus */
        if (i_ptr->toac < 0) return (0L);

        /* Give credit for bonuses */
        value += ((i_ptr->tohit + i_ptr->todam) * 100L);
        value += ((i_ptr->toac + i_ptr->pval) * 100L);
    }

    /* Weapons -- pay extra for all three bonuses */
    else if ((i_ptr->tval == TV_DIGGING) ||
             (i_ptr->tval == TV_HAFTED) ||
             (i_ptr->tval == TV_SWORD) ||
             (i_ptr->tval == TV_POLEARM)) {

        /* Hack -- negative hit/damage bonuses */
        if (i_ptr->tohit + i_ptr->todam < 0) return (0L);

        /* Factor in the bonuses */
        value += ((i_ptr->tohit + i_ptr->todam) * 100L);
        value += ((i_ptr->toac + i_ptr->pval) * 100L);
    }

    /* Bows -- pay extra for all three bonuses */
    else if (i_ptr->tval == TV_BOW) {

        /* Hack -- negative hit/damage bonuses */
        if (i_ptr->tohit + i_ptr->todam < 0) return (0L);

        /* Factor in the bonuses */
        value += ((i_ptr->tohit + i_ptr->todam) * 100L);
        value += ((i_ptr->toac + i_ptr->pval) * 100L);
    }

    /* Ammo -- pay extra for all three bonuses */
    else if ((i_ptr->tval == TV_SHOT) ||
             (i_ptr->tval == TV_ARROW) ||
             (i_ptr->tval == TV_BOLT)) {

        /* Hack -- negative hit/damage bonuses */
        if (i_ptr->tohit + i_ptr->todam < 0) return (0L);

        /* Factor in the bonuses */
        value += ((i_ptr->tohit + i_ptr->todam) * 5L);
    }


    /* Apply discount (if any) */
    if (i_ptr->discount) value -= (value * i_ptr->discount / 100L);


    /* Return the final value */
    return (value);
}


/*
 * Hack -- same as above but assume the item as "aware"
 */
static s32b store_item_value(inven_type *i_ptr)
{
    s32b value;
    bool aware;
    aware = x_list[i_ptr->k_idx].aware;
    x_list[i_ptr->k_idx].aware = TRUE;
    value = item_value(i_ptr);
    x_list[i_ptr->k_idx].aware = aware;
    return (value);
}


/*
 * Special "mass production" computation
 */
static int mass_roll(int num, int max)
{
    int i, t = 0;
    for (i = 0; i < num; i++) t += rand_int(max);
    return (t);
}


/*
 * Some objects naturally occur in "piles" of certain sizes
 * All objects can be sold at a discounted rate (in smaller piles)
 * Hack -- never discount "really cheap" items, it is annoying
 */
static void mass_produce(inven_type *i_ptr)
{
    int size = i_ptr->number;

    s32b cost = store_item_value(i_ptr);


    /* Analyze the type */
    switch (i_ptr->tval) {

        /* Food, Flasks, and Lites */
        case TV_FOOD:
        case TV_FLASK:
        case TV_LITE:
            if (cost <= 5L) size += mass_roll(3,5);
            if (cost <= 20L) size += mass_roll(3,5);
            break;

        case TV_POTION:
        case TV_SCROLL:
            if (cost <= 60L) size += mass_roll(3,5);
            if (cost <= 240L) size += mass_roll(1,5);
            break;

        case TV_MAGIC_BOOK:
        case TV_PRAYER_BOOK:
            if (cost <= 50L) size += mass_roll(2,3);
            if (cost <= 500L) size += mass_roll(1,3);
            break;

        case TV_SPIKE:
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
            size += mass_roll(5,5);
            if (cost <= 5L) size += mass_roll(5,5);
            if (cost <= 50L) size += mass_roll(5,5);
            break;

        case TV_SWORD:
        case TV_POLEARM:
        case TV_HAFTED:
        case TV_DIGGING:
            if (i_ptr->name2) break;
            if (cost <= 10L) size += mass_roll(3,5);
            if (cost <= 100L) size += mass_roll(3,5);
            break;

        case TV_BOW:
            if (i_ptr->name2) break;
            if (cost <= 10L) size += mass_roll(3,5);
            if (cost <= 100L) size += mass_roll(3,5);
            break;
    }

    /* Paranoia -- Never mass-produce expensive items */
    if (cost > 1000L) size = 1;

    /* Save the size */
    i_ptr->number = size;


    /* Perhaps apply a permanent discount */
    if (cost < 4) {
        i_ptr->discount = 0;
    }
    else if (rand_int(30) == 0) {
        i_ptr->discount = 25;
    }
    else if (rand_int(150) == 0) {
        i_ptr->discount = 50;
    }
    else if (rand_int(300) == 0) {
        i_ptr->discount = 75;
    }
    else if (rand_int(500) == 0) {
        i_ptr->discount = 90;
    }

    /* Handle discount */
    if (i_ptr->discount) {

        /* Reduce the pile size */
        i_ptr->number -= (i_ptr->number * i_ptr->discount / 100);
    }
}






/*
 * Asking price for a single item
 */
static s32b sell_price(s32b *max_sell, s32b *min_sell, inven_type *i_ptr)
{
    s32b      i;

    /* Clear the max/min sell values */
    *max_sell = *min_sell = 0;

    /* Get the item value, inflate it per object */
    i = store_item_value(i_ptr);

    /* Cursed/Worthless/Damaged items are worth nothing */
    if (i <= 0) return (0);

    /* Get the "basic value" */
    i = i * rgold_adj[ot_ptr->owner_race][p_ptr->prace] / 100L;

    /* Nothing becomes free */
    if (i < 1) i = 1;

    /* Extract min/max sell values */
    *max_sell = i * ot_ptr->max_inflate / 100L;
    *min_sell = i * ot_ptr->min_inflate / 100L;

    /* Black market is always over-priced */
    if (store_num == 6) {
        (*max_sell) *= 2;
        (*min_sell) *= 2;
    }

    /* Paranoia */
    if (*min_sell > *max_sell) *min_sell = *max_sell;

    /* Paranoia -- nothing becomes free */
    if (*min_sell < 1) *min_sell = 1;
    if (*max_sell < 1) *max_sell = 1;

    /* Return the price */
    return (i);
}




/*
 * Determine if a store item can "absorb" another item
 * See "item_similar()" for the same function for the "player"
 */
static int store_item_similar(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Identical items cannot be stacked */
    if (i_ptr == j_ptr) return (0);

    /* Different objects cannot be stacked */
    if (i_ptr->k_idx != j_ptr->k_idx) return (0);


    /* Require matching prices */
    if (i_ptr->cost != j_ptr->cost) return (0);

    /* Require matching discounts */
    if (i_ptr->discount != j_ptr->discount) return (0);


    /* Different charges (etc) cannot be stacked */
    if (i_ptr->pval != j_ptr->pval) return (0);

    /* Require many identical values */
    if ((i_ptr->tohit     != j_ptr->tohit)     ||
        (i_ptr->todam     != j_ptr->todam)     ||
        (i_ptr->toac      != j_ptr->toac)      ||
        (i_ptr->ac        != j_ptr->ac)        ||
        (i_ptr->dd        != j_ptr->dd)        ||
        (i_ptr->ds        != j_ptr->ds)        ||
        (i_ptr->flags1    != j_ptr->flags1)    ||
        (i_ptr->flags2    != j_ptr->flags2)    ||
        (i_ptr->flags3    != j_ptr->flags3)) {
        return (0);
    }


    /* Require identical "artifact" names */
    if (i_ptr->name1 != j_ptr->name1) return (0);

    /* Require identical "ego-item" names */
    if (i_ptr->name2 != j_ptr->name2) return (0);


    /* XXX Hack -- never stack "activatable" items */
    if (wearable_p(i_ptr) && (i_ptr->flags3 & TR3_ACTIVATE)) return (0);
    if (wearable_p(j_ptr) && (j_ptr->flags3 & TR3_ACTIVATE)) return (0);


    /* Hack -- Never stack chests */
    if (i_ptr->tval == TV_CHEST) return (0);


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
 * Check to see if the shop will be carrying too many objects	-RAK-	
 * Note that the shop, just like a player, will not accept things
 * it cannot hold.  Before, one could "nuke" potions this way.
 */
static bool store_check_num(inven_type *i_ptr)
{
    int        i;
    inven_type *j_ptr;

    /* Free space is always usable */
    if (st_ptr->store_ctr < STORE_INVEN_MAX) return TRUE;

    /* The "home" acts like the player */
    if (store_num == 7) {

        /* Check all the items */
        for (i = 0; i < st_ptr->store_ctr; i++) {

            /* Get the existing item */
            j_ptr = &st_ptr->store_item[i];

            /* Can the new object be combined with the old one? */
            if (item_similar(j_ptr, i_ptr)) return (TRUE);
        }
    }

    /* Normal stores do special stuff */
    else {

        /* Check all the items */
        for (i = 0; i < st_ptr->store_ctr; i++) {

            /* Get the existing item */
            j_ptr = &st_ptr->store_item[i];

            /* Can the new object be combined with the old one? */
            if (store_item_similar(j_ptr, i_ptr)) return (TRUE);
        }
    }

    /* But there was no room at the inn... */
    return (FALSE);
}




/*
 * Determine if the current store will purchase the given item (by tval)
 */
static bool store_will_buy(inven_type *i_ptr)
{
    int tval = i_ptr->tval;

    /* The Home accepts anything */
    if (store_num == 7) return (TRUE);

    /* Switch on the store */
    switch (store_num) {

      /* General Store */
      case 0:

        /* Analyze the type */
        switch (tval) {
          case TV_DIGGING:
          case TV_CLOAK:
          case TV_FOOD:
          case TV_FLASK:
          case TV_LITE:
          case TV_SPIKE:
            return (TRUE);
          default:
            return (FALSE);
        }

      /* Armoury */
      case 1:

        /* Analyze the type */
        switch (tval) {
          case TV_BOOTS:
          case TV_GLOVES:
          case TV_CROWN:
          case TV_HELM:
          case TV_SHIELD:
          case TV_CLOAK:
          case TV_SOFT_ARMOR:
          case TV_HARD_ARMOR:
          case TV_DRAG_ARMOR:
            return (TRUE);
          default:
            return (FALSE);
        }

      /* Weapon Shop */
      case 2:

        /* Analyze the type */
        switch (tval) {
          case TV_SHOT:
          case TV_BOLT:
          case TV_ARROW:
          case TV_BOW:
          case TV_DIGGING:
          case TV_HAFTED:
          case TV_POLEARM:
          case TV_SWORD:
            return (TRUE);
          default:
            return (FALSE);
        }

      /* Temple */
      case 3:

        /* Analyze the type */
        switch (tval) {
          case TV_PRAYER_BOOK:
          case TV_SCROLL:
          case TV_POTION:
          case TV_HAFTED:
            return (TRUE);
          case TV_SWORD:
          case TV_POLEARM:
            if (i_ptr->flags3 & TR3_BLESSED) return (TRUE);
          default:
            return (FALSE);
        }

      /* Alchemist */
      case 4:

        /* Analyze the type */
        switch (tval) {
          case TV_SCROLL:
          case TV_POTION:
            return (TRUE);
          default:
            return (FALSE);
        }

      /* Magic Shop */
      case 5:

        /* Analyze the type */
        switch (tval) {
          case TV_MAGIC_BOOK:
          case TV_AMULET:
          case TV_RING:
          case TV_STAFF:
          case TV_WAND:
          case TV_ROD:
          case TV_SCROLL:
          case TV_POTION:
            return (TRUE);
          default:
            return (FALSE);
        }
    }

    /* Black Market buys everything */
    return (TRUE);
}



/*
 * Add the item "i_ptr" to the inventory of the "Home"
 *
 * In all cases, return the slot (or -1) where the object was placed
 *
 * Note that this is a hacked up version of "inven_carry()".
 *
 * Also note that it may not correctly "adapt" to "knowledge" bacoming
 * known, the player may have to pick stuff up and drop it again.
 */
static int home_carry(inven_type *i_ptr)
{
    int                 slot;
    s32b               value, j_value;
    int		i;
    inven_type *j_ptr;


    /* The tval of readible books */
    int read_tval = 0;

    /* Acquire the type value of the books that the player can read, if any */
    if (cp_ptr->spell_stat == A_WIS) read_tval = TV_PRAYER_BOOK;
    else if (cp_ptr->spell_stat == A_INT) read_tval = TV_MAGIC_BOOK;


    /* Check each existing item (try to combine) */
    for (slot = 0; slot < st_ptr->store_ctr; slot++) {

        /* Get the existing item */
        j_ptr = &st_ptr->store_item[slot];

        /* The home acts just like the player */
        if (item_similar(j_ptr, i_ptr)) {

            /* Save the new number of items */
            j_ptr->number += i_ptr->number;

            /* Hack -- save largest discount */
            j_ptr->discount = MAX(j_ptr->discount, i_ptr->discount);

            /* Never lose an inscription */
            if (i_ptr->inscrip[0]) inscribe(j_ptr, i_ptr->inscrip);

            /* All done */
            return (slot);
        }
    }

    /* No space? */
    if (st_ptr->store_ctr >= STORE_INVEN_MAX) return (-1);


    /* Determine the "value" of the item */
    value = item_value(i_ptr);

    /* Check existing slots to see if we must "slide" */
    for (slot = 0; slot < st_ptr->store_ctr; slot++) {

        /* Get that item */
        j_ptr = &st_ptr->store_item[slot];

        /* Hack -- readable books always come first */
        if ((i_ptr->tval == read_tval) && (j_ptr->tval != read_tval)) break;
        if ((j_ptr->tval == read_tval) && (i_ptr->tval != read_tval)) continue;

        /* Objects sort by decreasing type */
        if (i_ptr->tval > j_ptr->tval) break;
        if (i_ptr->tval < j_ptr->tval) continue;

        /* Can happen in the home */
        if (!(x_list[i_ptr->k_idx].aware)) continue;
        if (!(x_list[j_ptr->k_idx].aware)) break;

        /* Objects sort by increasing sval */
        if (i_ptr->sval < j_ptr->sval) break;
        if (i_ptr->sval > j_ptr->sval) continue;

        /* Can happen in the home */
        if (!inven_known_p(i_ptr)) continue;
        if (!inven_known_p(j_ptr)) break;

        /* Objects sort by decreasing value */
        j_value = item_value(j_ptr);
        if (value > j_value) break;
        if (value < j_value) continue;
    }

    /* Slide the others up */
    for (i = st_ptr->store_ctr; i > slot; i--) {
        st_ptr->store_item[i] = st_ptr->store_item[i-1];
    }

    /* More stuff now */
    st_ptr->store_ctr++;

    /* Insert the new item */
    st_ptr->store_item[slot] = *i_ptr;

    /* Return the location */
    return (slot);
}


/*
 * Add the item "i_ptr" to a real stores inventory.
 *
 * If the item is "worthless", it is thrown away (except in the home).
 *
 * If the item cannot be combined with an object already in the inventory,
 * make a new slot for it, and calculate its "per item" price.  Note that
 * this price will be negative, since the price will not be "fixed" yet.
 * Adding an item to a "fixed" price stack will not change the fixed price.
 *
 * In all cases, return the slot (or -1) where the object was placed
 */
static int store_carry(inven_type *i_ptr)
{
    int                 slot;
    s32b               value, j_value, scost = 0L;
    int		i;
    inven_type *j_ptr;

    s32b               icost, dummy;


    /* Determine the "value" of the item */
    value = store_item_value(i_ptr);

    /* Cursed/Worthless items "disappear" when sold */
    if (value <= 0) return (-1);

    /* Get a good "initial selling price" */
    dummy = sell_price(&icost, &dummy, i_ptr);

    /* Hack -- Save the "store cost" (negative means not "fixed" yet */
    scost = -icost;


    /* Check each existing item (try to combine) */
    for (slot = 0; slot < st_ptr->store_ctr; slot++) {

        /* Get the existing item */
        j_ptr = &st_ptr->store_item[slot];

        /* Can the existing items be incremented? */
        if (store_item_similar(j_ptr, i_ptr)) {

            int total = j_ptr->number + i_ptr->number;

            /* Hack -- extra items disappear */
            j_ptr->number = (total > 99) ? 99 : total;

            /* All done */
            return (slot);
        }
    }

    /* No space? */
    if (st_ptr->store_ctr >= STORE_INVEN_MAX) return (-1);


    /* Check existing slots to see if we must "slide" */
    for (slot = 0; slot < st_ptr->store_ctr; slot++) {

        /* Get that item */
        j_ptr = &st_ptr->store_item[slot];

        /* Objects sort by decreasing type */
        if (i_ptr->tval > j_ptr->tval) break;
        if (i_ptr->tval < j_ptr->tval) continue;

        /* Objects sort by increasing sval */
        if (i_ptr->sval < j_ptr->sval) break;
        if (i_ptr->sval > j_ptr->sval) continue;

        /* Objects sort by decreasing value */
        j_value = store_item_value(j_ptr);
        if (value > j_value) break;
        if (value < j_value) continue;
    }

    /* Slide the others up */
    for (i = st_ptr->store_ctr; i > slot; i--) {
        st_ptr->store_item[i] = st_ptr->store_item[i-1];
    }

    /* More stuff now */
    st_ptr->store_ctr++;

    /* Insert the new item */
    st_ptr->store_item[slot] = *i_ptr;

    /* Save the "scost" */
    st_ptr->store_item[slot].scost = scost;

    /* Return the location */
    return (slot);
}


/*
 * Increase, by a given amount, the number of a certain item
 * in a certain store.  This can result in zero items.
 */
static void store_item_increase(int item_val, int num)
{
    int         cnt;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &st_ptr->store_item[item_val];

    /* Verify the number */
    cnt = i_ptr->number + num;
    if (cnt > 255) cnt = 255;
    else if (cnt < 0) cnt = 0;
    num = cnt - i_ptr->number;

    /* Save the new number */
    i_ptr->number += num;
}


/*
 * Remove a slot if it is empty
 */
static void store_item_optimize(int item_val)
{
    int         j;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &st_ptr->store_item[item_val];

    /* Must exist */
    if (!i_ptr->tval) return;

    /* Must have no items */
    if (i_ptr->number) return;

    /* One less item */
    st_ptr->store_ctr--;

    /* Slide everyone */
    for (j = item_val; j < st_ptr->store_ctr; j++) {
        st_ptr->store_item[j] = st_ptr->store_item[j + 1];
    }

    /* Nuke the final slot */
    invwipe(&st_ptr->store_item[st_ptr->store_ctr]);
}


/*
 * This function will keep 'crap' out of the black market.
 *
 * Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
 *
 * Basically, this function returns TRUE is an item is "crap".
 *
 * XXX XXX Hack -- this function uses some hard-codes "kind" indexes.
 */
static bool black_market_crap(inven_type *i_ptr)
{
    int		i, j;

    /* Ego items are never crap */
    if (i_ptr->name2) return (FALSE);

    /* Good items are never crap */
    if (i_ptr->toac > 0) return (FALSE);
    if (i_ptr->tohit > 0) return (FALSE);
    if (i_ptr->todam > 0) return (FALSE);

    /* Mega-Hack -- keep good shovels and picks */
    if (i_ptr->tval == TV_DIGGING) {
        if (i_ptr->pval > k_list[i_ptr->k_idx].pval) return (FALSE);
    }

    /* Check the other "normal" stores */
    for (i = 0; i < 6; i++) {

        /* Check every item */
        for (j = 0; j < store[i].store_ctr; j++) {

            /* Access the store item */
            inven_type *j_ptr = &store[i].store_item[j];

            /* Duplicate item, assume crappy */
            if (i_ptr->k_idx == j_ptr->k_idx) return (TRUE);
        }
    }

    /* Amulets of Adornment are crap */
    if (i_ptr->k_idx == 169) return (TRUE);

    /* Potions of Apple Juice & Water are crap. */
    if (i_ptr->k_idx == 223) return (TRUE);
    if (i_ptr->k_idx == 224) return (TRUE);

    /* Normal crowns are crap */
    if (i_ptr->k_idx == 98) return (TRUE);
    if (i_ptr->k_idx == 99) return (TRUE);
    if (i_ptr->k_idx == 100) return (TRUE);

    /* Normal rags are crap */
    if (i_ptr->k_idx == 102) return (TRUE);

    /* Assume useful */
    return (FALSE);
}


/*
 * Attempt to delete (some of) a random item from the store
 * Hack -- we attempt to "maintain" piles of items when possible.
 */
static void store_delete(void)
{
    int what, num;

    /* Pick a random slot */
    what = rand_int(st_ptr->store_ctr);

    /* Determine how many items are here */
    num = st_ptr->store_item[what].number;

    /* Hack -- sometimes, only destroy half the items */
    if (rand_int(100) < 50) num = (num + 1) / 2;

    /* Hack -- sometimes, only destroy a single item */
    if (rand_int(100) < 50) num = 1;

    /* Actually destroy (part of) the item */
    store_item_increase(what, -num);
    store_item_optimize(what);
}


/*
 * Creates a random item and gives it to a store
 * This algorithm needs to be rethought.  A lot.
 * Currently, "normal" stores use a pre-built array.
 *
 * Note -- the "level" given to "obj_get_num()" is a "favored"
 * level, that is, there is a much higher chance of getting
 * items with a level approaching that of the given level...
 *
 * Should we check for "permission" to have the given item?
 */
static void store_create(void)
{
    int			i, tries, level;
    inven_type		*i_ptr;
    inven_type		tmp_obj;

    /* Start a new object */
    i_ptr = &tmp_obj;

    /* Hack -- consider up to four items */
    for (tries = 0; tries < 4; tries++) {

        /* Black Market */
        if (store_num == 6) {

            /* Pick a level for object/magic */
            level = 25 + rand_int(25);

            /* Random item (usually of given level) */
            i = get_obj_num(level, FALSE);
        }

        /* Normal Store */
        else {

            /* Mega-Hack -- Get a random object kind */
            i = store_choice[store_num][rand_int(STORE_CHOICES)];

            /* Hack -- fake level for apply_magic() */
            level = rand_range(1, STORE_OBJ_LEVEL);
        }

        /* Create a new object of the chosen kind */
        invcopy(i_ptr, i);

        /* Apply some "low-level" magic (no artifacts) */
        apply_magic(i_ptr, level, FALSE, FALSE, FALSE);

        /* Hack -- Storebought chests will not be wonderful */
        if (i_ptr->tval == TV_CHEST) {
            if (i_ptr->pval > 20) i_ptr->pval = 20;
        }

        /* Hack -- General Store lites have "clean" amounts of light */
        if ((store_num == 0) && (i_ptr->tval == TV_LITE)) {
            if (i_ptr->sval == SV_LITE_TORCH) i_ptr->pval = FUEL_TORCH / 2;
            if (i_ptr->sval == SV_LITE_LANTERN) i_ptr->pval = FUEL_LAMP / 2;
        }

        /* Mark it as pre-known */
        inven_known(i_ptr);

        /* Skip "worthless" items */
        if (store_item_value(i_ptr) <= 0) continue;

        /* Hack -- prune the black market */
        if ((store_num == 6) && (black_market_crap(i_ptr))) continue;

        /* Mass produce and/or Apply discount */
        mass_produce(i_ptr);

        /* Paranoia -- make sure there is room */
        if (!store_check_num(i_ptr)) continue;

        /* Carry the item */
        (void)store_carry(i_ptr);

        /* Definitely done */
        break;
    }
}



/*
 * Eliminate need to bargain if player has haggled well in the past
 */
static bool noneedtobargain(s32b minprice)
{
    s32b good = st_ptr->good_buy;
    s32b bad = st_ptr->bad_buy;

    /* Cheap items are "boring" */
    if (minprice < 10L) return (TRUE);
    
    /* Perfect haggling */
    if (good == MAX_SHORT) return (TRUE);

    /* Reward good haggles, punish bad haggles, notice price */
    if (good > ((3 * bad) + (5 + (minprice/50)))) return (TRUE);

    /* Return the flag */
    return (FALSE);
}


/*
 * update the bargain info					-DJB-
 */
static void updatebargain(s32b price, s32b minprice)
{
    /* Allow haggling to be turned off */
    if (no_haggle_flag) return;

    /* Cheap items are "boring" */
    if (minprice < 10L) return;

    /* Count the successful haggles */
    if (price == minprice) {
        if (st_ptr->good_buy < MAX_SHORT) {
            st_ptr->good_buy++;
        }
    }

    /* Count the failed haggles */
    else {
        if (st_ptr->bad_buy < MAX_SHORT) {
            st_ptr->bad_buy++;
        }
    }
}



/*
 * Displays the set of commands
 */
static void display_commands(void)
{
    /* Display the legal commands */
    prt("You may:                                 SPACE) Next page.   ", 21, 0);
    prt(" ESCAPE) Exit from Building.             p) Purchase an item.", 22, 0);
    prt(" i/e/t/w/x) Check Inventory.             s) Sell an item.    ", 23, 0);

    /* Modify the commands for the "home" */
    if (store_num == 7) {
        prt(" SPACE) Next page.   ", 21, 40);
        prt(" g) Get an item.     ", 22, 40);
        prt(" d) Drop an item.    ", 23, 40);
    }

    /* Remove "browse" command if necessary */
    if (st_ptr->store_ctr <= 12) prt("", 21, 40);
}


/*
 * Displays the set of commands				-RAK-	
 */
static void haggle_commands(int typ)
{
    if (typ == -1) {
        prt("Specify an asking-price in gold pieces.", 21, 0);
    }
    else {
        prt("Specify an offer in gold pieces.", 21, 0);
    }

    prt("ESC) Quit Haggling.", 22, 0);

    prt("", 23, 0);
}



/*
 * Re-displays a single store entry
 */
static void display_entry(int pos)
{
    int			i;
    inven_type		*i_ptr;
    char		out_val[160];


    int maxwid = 75;

    /* Get the item */
    i_ptr = &st_ptr->store_item[pos];

    /* Get the "offset" */
    i = (pos % 12);

    /* Label it, clear the line */
    (void)sprintf(out_val, "%c) ", 'a' + i);
    prt(out_val, i+6, 0);

    /* Describe an item in the home */
    if (store_num == 7) {

        maxwid = 75;

        /* Leave room for weights, if necessary -DRS- */
        if (show_inven_weight) maxwid -= 10;

        /* Describe the object */
        objdes(out_val, i_ptr, TRUE);
        out_val[maxwid] = '\0';
        c_put_str(tval_to_attr[i_ptr->tval], out_val, i+6, 3);

        /* Show weights, if turned on -DRS- */
        if (show_inven_weight) {
            /* Only show the weight of an individual item */
            int wgt = i_ptr->weight;
            (void)sprintf(out_val, "%3d.%d lb", wgt / 10, wgt % 10);
            put_str(out_val, i+6, 68);
        }
    }

    /* Describe an item (fully) in a store */
    else {

        s32b x = i_ptr->scost;

        /* Must leave room for the "price" */
        maxwid = 65;

        /* Leave room for weights, if necessary -DRS- */
        if (show_store_weight) maxwid -= 7;

        /* Describe the object */
        objdes_store(out_val, i_ptr, TRUE);
        out_val[maxwid] = '\0';
        c_put_str(tval_to_attr[i_ptr->tval], out_val, i+6, 3);

        /* Show weights, if turned on -DRS- */
        if (show_store_weight) {
            /* Only show the weight of an individual item */
            int wgt = i_ptr->weight;
            (void)sprintf(out_val, "%3d.%d", wgt / 10, wgt % 10);
            put_str(out_val, i+6, 61);
        }

        /* Guess at the price */
        if (x < 0) {
            x = -x * chr_adj() / 100L;
            if (x <= 0) x = 1;
        }

        /* Actually draw the price */
        (void)sprintf(out_val, "%9ld", (long)x);
        put_str(out_val, i+6, 68);

        /* Hack -- Label "Fixed" Prices */
        if (i_ptr->scost > 0) put_str(" F", i+6, 77);
    }
}


/*
 * Displays a store's inventory			-RAK-	
 * All prices are listed as "per individual object".  -BEN-
 */
static void display_inventory()
{
    int i, k;

    /* Display the next 12 items */
    for (k = 0; k < 12; k++) {

        /* Do not display "dead" items */
        if (store_top + k >= st_ptr->store_ctr) break;

        /* Display that line */
        display_entry(store_top + k);
    }

    /* Erase the extra lines and the "more" prompt */
    for (i = k; i < 13; i++) prt("", i + 6, 0);

    /* Assume "no current page" */
    put_str("        ", 5, 20);

    /* Visual reminder of "more items" */
    if (st_ptr->store_ctr > 12) {

        /* Show "more" reminder (after the last item) */
        prt("-more-", k + 6, 3);

        /* Indicate the "current page" */
        put_str(format("(Page %d)", store_top/12 + 1), 5, 20);
    }
}


/*
 * Displays players gold					-RAK-	
 */
static void store_prt_gold(void)
{
    char out_val[64];

    prt("Gold Remaining: ", 19, 53);

    sprintf(out_val, "%9ld", (long)p_ptr->au);
    prt(out_val, 19, 68);
}


/*
 * Displays store (after clearing screen)		-RAK-	
 */
static void display_store(void)
{
    /* Erase the screen */
    clear_screen();

    /* The "Home" is special */
    if (store_num == 7) {

        /* Put the owner name */
        put_str("Your Home", 3, 30);

        /* If showing weights, show label, too  -DRS- */
        if (show_inven_weight) {
            put_str("Weight", 5, 70);
        }
    }

    /* Normal stores */
    else {

        /* Put the owner name */
        put_str(owners[st_ptr->owner].owner_name, 3, 10);

        /* And then put the store name */
        put_str(k_list[OBJ_STORE_LIST+store_num].name, 3, 50);

        /* Label the asking price (in stores) */
        put_str("              Price", 5, 58);

        /* If showing weights, show label, too  -DRS- */
        if (show_store_weight) {
            put_str("Weight", 5, 60);
        }

        /* Display the current gold */
        store_prt_gold();
    }

    /* Label the item descriptions */
    put_str("Item Description", 5, 3);

    /* Draw in the inventory */
    display_inventory();

    /* Hack -- Display the commands */
    display_commands();
}



/*
 * Get the ID of a store item and return it's value	-RAK-	
 */
static int get_store_item(int *com_val, cptr pmt, int i, int j)
{
    char	command;

    char	out_val[160];


    /* Assume failure */
    *com_val = (-1);

    /* Build the prompt */
    (void)sprintf(out_val, "(Items %c-%c, ESC to exit) %s",
                  i + 'a', j + 'a', pmt);

    /* Ask until done */
    while (TRUE) {

        /* Escape */
        if (!get_com(out_val, &command)) break;

        /* Legal responses */
        if (command >= i+'a' && command <= j+'a') {
            *com_val = command - 'a';
            break;
        }

        /* Oops */
        bell();
    }

    prt("", 0, 0);

    return (command != ESCAPE);
}


/*
 * Increase the insult counter and get angry if too many -RAK-	
 */
static int increase_insults(void)
{
    st_ptr->insult_cur++;

    if (st_ptr->insult_cur > owners[st_ptr->owner].insult_max) {
        say_comment4();
        st_ptr->insult_cur = 0;
        st_ptr->good_buy = 0;
        st_ptr->bad_buy = 0;
        st_ptr->store_open = turn + 25000 + randint(25000);
        return (TRUE);
    }

    return (FALSE);
}


/*
 * Decrease insults					-RAK-	
 */
static void decrease_insults(void)
{
    if (st_ptr->insult_cur) st_ptr->insult_cur--;
}


/*
 * Have insulted while haggling				-RAK-	
 */
static int haggle_insults(void)
{
    /* Increase insults */
    if (increase_insults()) return (TRUE);

    /* Display and flush insult */
    say_comment5();
    msg_print(NULL);

    /* Still okay */
    return (FALSE);
}


/*
 * Get a haggle
 */
static int get_haggle(cptr pmt, s32b *poffer, int num, s32b price, int final)
{
    int			inc;

    s32b		i;

    char		*p;

    char                buf[128];
    char		out_val[160];


    if (last_inc && !final) {
        (void)sprintf(buf, "%s [%c%ld] ", pmt,
                        POM(last_inc), (long)ABS(last_inc));
    }
    else {
        (void)sprintf(buf, "%s [accept] ", pmt);
    }

    /* Keep asking until player quits, or answers */
    for (i = 0; !i; ) {

        /* Prompt for a string, handle abort */
        prt(buf, 0, 0);
        if (!askfor(out_val, 32)) {
            prt("", 0, 0);
            return (FALSE);
        }

        /* Skip leading spaces */
        for (p = out_val; *p == ' '; p++);

        /* Check for "incremental" */
        inc = ((*p == '+' || *p == '-'));

        /* Extract a number */
        i = atol(p);

        /* incremental haggling must be initiated */
        if (inc && !num) {
            msg_print("You haven't even made your first offer yet!");
            continue;
        }

        /* Allow an Increment of zero */
        if (inc && !i) break;

        /* Return accepts default */
        else if (!i) {

            /* Accept final price */
            if (final || !last_inc) i = price, inc = FALSE;

            /* Use previous increment again */
            else i = last_inc, inc = TRUE;
        }
    }

    /* The player indicated a valid number */
    if (inc) {
        *poffer += i;
        last_inc = i;
    }
    else {
        *poffer = i;
        last_inc = 0;
    }

    /* Success */
    return (TRUE);
}


/*
 * Receive an offer (from the player)
 */
static int receive_offer(cptr pmt, s32b *poffer,
                         s32b last_offer, int num, int factor,
                         s32b price, int final)
{
    int flag, receive;

    receive = 0;

    for (flag = FALSE; !flag; ) {

        if (get_haggle(pmt, poffer, num, price, final)) {

            if ((*poffer) * factor >= last_offer * factor) {
                flag = TRUE;
            }
            else if (haggle_insults()) {
                receive = 2;
                flag = TRUE;
            }
            else {
                /* offer rejected, reset offer so that */
                /* incremental haggling works correctly */
                (*poffer) = last_offer;
            }
        }
        else {
            receive = 1;
            flag = TRUE;
        }
    }

    return (receive);
}


/*
 * Haggling routine					-RAK-	
 */
static int purchase_haggle(inven_type *i_ptr, s32b *price)
{
    s32b               max_sell, min_sell, max_buy;
    s32b               cost, cur_ask, final_ask, min_offer;
    s32b               last_offer, offer;
    s32b               x1, x2, x3;
    s32b               min_per, max_per;
    int                flag, loop_flag, noneed;
    int                purchase, num, final_flag, final = FALSE;

    cptr		pmt;

    char		out_val[160];


    *price = 0;
    purchase = 0;
    final_flag = 0;


    /* Determine the cost of a single items */
    cost = sell_price(&max_sell, &min_sell, i_ptr);

    /* Extract a maximum sell price */
    max_sell = max_sell * chr_adj() / 100L;
    if (max_sell <= 0) max_sell = 1;

    /* Extract a minimum sell price */
    min_sell = min_sell * chr_adj() / 100L;
    if (min_sell <= 0) min_sell = 1;

    /* Determine if bargaining is necessary */
    noneed = noneedtobargain(min_sell);

    /* Scale prices by quantity */
    cost     *= i_ptr->number;
    max_sell *= i_ptr->number;
    min_sell *= i_ptr->number;

    /* XXX This appears to be a hack.  See sell_price(). */
    /* cast max_inflate to signed so that subtraction works correctly */
    max_buy = cost * (200L - (s32b)(ot_ptr->max_inflate)) / 100L;
    if (max_buy <= 0) max_buy = 1;

    min_per = ot_ptr->haggle_per;
    max_per = min_per * 3;

    haggle_commands(1);

    cur_ask = max_sell;
    final_ask = min_sell;
    min_offer = max_buy;
    last_offer = min_offer;
    offer = 0;

    /* this prevents incremental haggling on first try */
    num = 0;
    pmt = "Asking";

    /* Go right to final price if player has bargained well */
    if (noneed || no_haggle_flag) {

        /* Message summary */
        message("After a long bargaining session, ", 0x02);
        message("you agree upon the price.", 0);
        msg_print(NULL);

        /* Sales Tax.  Apply 10% penalty if haggling disabled */
        if (inven_known_p(i_ptr)) final_ask += final_ask / 10;

        /* Go to final offer */
        pmt = "Final Offer";
        cur_ask = final_ask;
        final = TRUE;
    }

    /* Reset increment */
    last_inc = 0L;

    for (flag = FALSE; !flag; ) {

        do {
            loop_flag = TRUE;
            (void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
            put_str(out_val, 1, 0);
            purchase = receive_offer("What do you offer? ",
                                     &offer, last_offer, num,
                                     1, cur_ask, final);
            if (purchase != 0) {
                flag = TRUE;
            }
            else {
                if (offer > cur_ask) {
                    say_comment6();
                    /* rejected, reset offer for incremental haggling */
                    offer = last_offer;
                }
                else if (offer == cur_ask) {
                    flag = TRUE;
                    *price = offer;
                }
                else {
                    loop_flag = FALSE;
                }
            }
        }
        while (!flag && loop_flag);

        if (!flag) {
            x1 = 100 * (offer - last_offer) / (cur_ask - last_offer);
            if (x1 < min_per) {
                flag = haggle_insults();
                if (flag) purchase = 2;
            }
            else if (x1 > max_per) {
                x1 = x1 * 3 / 4;
                if (x1 < max_per) x1 = max_per;
            }
            x2 = rand_range(x1-2, x1+2);
            x3 = ((cur_ask - offer) * x2 / 100L) + 1;
            /* don't let the price go up */
            if (x3 < 0) x3 = 0;
            cur_ask -= x3;

            if (cur_ask < final_ask) {
                final = TRUE;
                cur_ask = final_ask;
                pmt = "Final Offer";
                final_flag++;
                if (final_flag > 3) {
                    if (increase_insults()) purchase = 2;
                    else purchase = 1;
                    flag = TRUE;
                }
            }
            else if (offer >= cur_ask) {
                flag = TRUE;
                *price = offer;
            }
            if (!flag) {
                last_offer = offer;
                num++;	   /* enable incremental haggling */
                prt("", 1, 0);
                (void)sprintf(out_val, "Your last offer: %ld",
                              (long)last_offer);
                put_str(out_val, 1, 39);
                say_comment2(last_offer, cur_ask, final_flag);
            }
        }
    }

    /* Update bargaining info */
    if (purchase == 0) updatebargain(*price, final_ask);

    return (purchase);
}


/*
 * Haggling routine					-RAK-	
 */
static int sell_haggle(inven_type *i_ptr, s32b *price)
{
    s32b               max_sell = 0, max_buy = 0, min_buy = 0;
    s32b               cost = 0, cur_ask = 0, final_ask = 0, min_offer = 0;
    s32b               last_offer = 0, offer = 0;
    s32b               max_gold = 0;
    s32b               x1, x2, x3, tmp;
    s32b               min_per, max_per;
    int                flag, loop_flag, noneed;
    int                 sell, num, final_flag, final = FALSE;

    cptr		pmt;

    char		out_val[160];


    /* Get the value of a single item */
    cost = item_value(i_ptr);

    sell = 0;
    *price = 0;
    final_flag = 0;

    /* Instantly react to worthless items */
    if (cost <= 0) return (3);

    /* XXX XXX See "sell_price()" */

    cost = cost * (200L - chr_adj()) / 100L;

    tmp = (200L - rgold_adj[ot_ptr->owner_race][p_ptr->prace]);
    cost = cost * tmp / 100L;
    if (cost < 1) cost = 1;

    /* Scale by quantity, and inflate at the same time */
    max_sell = i_ptr->number * cost * ot_ptr->max_inflate / 100L;

    /* cast max_inflate to signed so that subtraction works correctly */
    max_buy = cost * (200L - (s32b)(ot_ptr->max_inflate)) / 100L;
    if (max_buy < 1) max_buy = 1;

    min_buy = cost * (200L - (s32b)(ot_ptr->min_inflate)) / 100L;
    if (min_buy < 1) min_buy = 1;

    /* Paranoia */
    if (min_buy < max_buy) min_buy = max_buy;

    /* Determine if haggling is necessary */
    noneed = noneedtobargain(min_buy);

    /* Scale by quantity */
    cost    *= i_ptr->number;
    min_buy *= i_ptr->number;
    max_buy *= i_ptr->number;

    min_per = ot_ptr->haggle_per;
    max_per = min_per * 3;
    max_gold = ot_ptr->max_cost;

    haggle_commands(-1);

    if (max_buy > max_gold) {

        final_flag = 1;
        final = TRUE;
        pmt = "Final Offer";
        cur_ask = max_gold;
        final_ask = max_gold;
        message("I am sorry, ", 0x02);
        message("but I have not the money to afford such a fine item.", 0);
    }

    else {

        cur_ask = max_buy;
        final_ask = min_buy;
        if (final_ask > max_gold) final_ask = max_gold;
        pmt = "Offer";

        /* go right to final price if player has bargained well */
        if (noneed || no_haggle_flag) {

            message("After a long bargaining session, ", 0x02);
            message("you agree upon the price.", 0);
            msg_print(NULL);

            /* Sales tax.  10% if you have cancelled haggling */
            /* XXX XXX Should use "item_value()" not "i_ptr->cost" */
            if (inven_known_p(i_ptr)) final_ask -= final_ask / 10;

            cur_ask = final_ask;

            /* Not bargaining, say so */
            pmt = "Final Offer";
            final = TRUE;
        }
    }


    /* XXX This code is supposed to make "cheating" harder */
    /* XXX XXX XXX Does anyone know what this code does? */
    if (final_ask > cost) {
        if (cost < 10) {
            final_ask = cost;
        }
        else if (cost < 200) {
            final_ask = cost - randint(2);
        }
        else {
            final_ask = cost - randint(cost / 100);
        }
    }

    min_offer = max_sell;
    last_offer = min_offer;
    offer = 0;

    /* this prevents incremental haggling on first try */
    num = 0;

    /* Reset increment */
    last_inc = 0L;

    if (cur_ask < 1) cur_ask = 1;

    for (flag = FALSE; !flag; ) {

        do {
            loop_flag = TRUE;
            (void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
            put_str(out_val, 1, 0);
            sell = receive_offer("What price do you ask? ",
                                 &offer, last_offer, num,
                                 -1, cur_ask, final);
            if (sell != 0) {
                flag = TRUE;
            }
            else {
                if (offer < cur_ask) {
                    say_comment6();
                    /* rejected, reset offer for incremental haggling */
                    offer = last_offer;
                }
                else if (offer == cur_ask) {
                    flag = TRUE;
                    *price = offer;
                }
                else {
                    loop_flag = FALSE;
                }
            }
        }
        while (!flag && loop_flag);

        if (!flag) {
            x1 = 100 * (last_offer - offer) / (last_offer - cur_ask);
            if (x1 < min_per) {
                flag = haggle_insults();
                if (flag) sell = 2;
            }
            else if (x1 > max_per) {
                x1 = x1 * 3 / 4;
                if (x1 < max_per) x1 = max_per;
            }
            x2 = rand_range(x1-2, x1+2);
            x3 = ((offer - cur_ask) * x2 / 100L) + 1;
            /* don't let the price go down */
            if (x3 < 0) x3 = 0;
            cur_ask += x3;
            if (cur_ask > final_ask) {
                cur_ask = final_ask;
                final = TRUE;
                pmt = "Final Offer";
                final_flag++;
                if (final_flag > 3) {
                    flag = TRUE;
                    sell = 1;
                    if (increase_insults()) sell = 2;
                }
            }
            else if (offer <= cur_ask) {
                flag = TRUE;
                *price = offer;
            }
            if (!flag) {
                last_offer = offer;
                num++;   /* enable incremental haggling */
                prt("", 1, 0);
                (void)sprintf(out_val,
                             "Your last bid %ld", (long)last_offer);
                put_str(out_val, 1, 39);
                say_comment3(cur_ask, last_offer, final_flag);
            }
        }
    }

    /* update bargaining info */
    if (sell == 0) updatebargain(*price, final_ask);

    return (sell);
}





/*
 * Buy an item from a store				-RAK-	
 */
static int store_purchase(void)
{
    int			i, amt, choice;
    int			item_val, item_new, purchase;

    s32b		price;

    inven_type		sell_obj;
    inven_type		*i_ptr;

    char		out_val[160];
    char		tmp_str[160];


    purchase = FALSE;

    /* Empty? */
    if (st_ptr->store_ctr <= 0) {
        if (store_num == 7) msg_print("Your home is empty.");
        else msg_print("I am currently out of stock.");
        return (FALSE);
    }


    /* Find the number of objects on this and following pages */
    i = (st_ptr->store_ctr - store_top);

    /* And then restrict it to the current page */
    if (i > 12) i = 12;

    /* Prompt */
    sprintf(out_val, "Which item %s? ",
            (store_num == 7) ? "do you want to take" : "are you interested in");

    /* Get the item number to be bought */
    if (!get_store_item(&item_val, out_val, 0, i-1)) return (FALSE);

    /* Get the actual index */
    item_val = item_val + store_top;

    /* Get the actual item */
    i_ptr = &st_ptr->store_item[item_val];

    /* Assume the player wants just one of them */
    amt = 1;

    /* Hack -- get a "sample" object */
    sell_obj = *i_ptr;
    sell_obj.number = amt;

    /* Hack -- require room in pack */
    if (!inven_check_num(&sell_obj)) {
        prt("You cannot carry that many different items.", 0, 0);
        return (FALSE);
    }

    /* Find out how many the player wants */
    if (i_ptr->number > 1) {

        /* Hack -- note cost of "fixed" items */
        if ((store_num != 7) && (i_ptr->scost > 0)) {
            sprintf(out_val, "That costs %ld gold per item.",
                    (long)i_ptr->scost);
            msg_print(out_val);
        }

        /* Find out how many to buy (letter means "all") */
        sprintf(out_val, "Quantity (1-%d): ", i_ptr->number);
        prt(out_val, 0, 0);
        sprintf(tmp_str, "%d", amt);
        if (!askfor_aux(tmp_str, 3)) return (FALSE);
        
        /* Parse it (a letter means "all") */
        amt = atoi(tmp_str);
        if (isalpha(tmp_str[0])) amt = 99;
        if (amt > i_ptr->number) amt = i_ptr->number;
        if (amt <= 0) return (FALSE);
    }

    /* Create the object to be sold (structure copy) */
    sell_obj = *i_ptr;
    sell_obj.number = amt;

    /* Hack -- require room in pack */
    if (!inven_check_num(&sell_obj)) {
        prt("You cannot carry that many items.", 0, 0);
        return (FALSE);
    }

    /* Attempt to buy it */
    if (store_num != 7) {

        /* Fixed price, quick buy */
        if (sell_obj.scost > 0) {

            /* Assume accept */
            choice = 0;

            /* Hack -- price it by hand */
            price = sell_obj.scost * sell_obj.number;
        }

        /* Haggle for it */
        else {

            objdes_store(tmp_str, &sell_obj, TRUE);	
            (void)sprintf(out_val, "Buying %s (%c)",
                          tmp_str, item_val + 'a');
            message(out_val, 0);

            /* Haggle for a final price */
            choice = purchase_haggle(&sell_obj, &price);
        }

        /* Player wants it */
        if (choice == 0) {

            /* Player can afford it */
            if (p_ptr->au >= price) {

                say_comment1();
                decrease_insults();

                /* Spend the money */
                p_ptr->au -= price;
                store_prt_gold();

                /* Fix the item price if necessary */
                if (i_ptr->scost < 0) {

                    /* Extract, and "memorize", the "per-item" price. */
                    /* XXX Hack -- round down.  Perhaps we should only */
                    /* fix the price if a single item is bought, or if */
                    /* the rounded down price is the normal price.  Or */
                    /* maybe we should round ALL prices up, so that you */
                    /* SAVE money buying in bulk. XXX XXX XXX */
                    sell_obj.scost = price / sell_obj.number;

                    /* Paranoia -- never become free.  Is this needed? */
                    if (sell_obj.scost < 1) sell_obj.scost = 1;

                    /* Fix the price for the remaining objects */
                    i_ptr->scost = sell_obj.scost;
                }

                /* Hack -- only objects in the store have an "scost" */
                sell_obj.scost = 0;

                /* Note how many slots the store used to have */
                i = st_ptr->store_ctr;

                /* Remove the bought items from the store */
                store_item_increase(item_val, -amt);
                store_item_optimize(item_val);

                /* Hack -- buying an item makes you aware of it */
                inven_aware(&sell_obj);

                /* Describe the transaction */
                objdes(tmp_str, &sell_obj, TRUE);	
                (void)sprintf(out_val, "You bought %s for %ld gold.",
                              tmp_str, (long)price);
                message(out_val, 0);

                /* Let the player carry it (as if he picked it up) */
                item_new = inven_carry(&sell_obj);

                /* Describe the final result */
                objdes(tmp_str, &inventory[item_new], TRUE);
                (void)sprintf(out_val, "You have %s (%c)",
                              tmp_str, index_to_label(item_new));
                message(out_val, 0);

                /* Recalculate bonuses */
                p_ptr->update |= (PU_BONUS);
    
                /* Handle non-visual stuff */
                handle_stuff(FALSE);

                /* Item is still here */
                if (i == st_ptr->store_ctr) {

                    /* Redraw the item */
                    display_entry(item_val);
                }

                /* The item is gone */
                else {

                    /* Nothing left */
                    if (st_ptr->store_ctr == 0) store_top = 0;

                    /* Nothing left on that screen */
                    else if (store_top >= st_ptr->store_ctr) store_top -= 12;

                    /* Redraw everything */
                    display_inventory();
                }
            }

            else {

                if (increase_insults()) {
                    purchase = TRUE;
                }
                else {
                    say_comment1();
                    msg_print("Liar!  You have not the gold!");
                }
            }
        }

        else if (choice == 2) {
            purchase = TRUE;
        }
    }

    /* Home is much easier */
    else {

        /* Carry the item */
        item_new = inven_carry(&sell_obj);

        /* Take note if we take the last one */
        i = st_ptr->store_ctr;

        /* Remove one item from the home */
        store_item_increase(item_val, -amt);
        store_item_optimize(item_val);

        /* Describe just the result */
        objdes(tmp_str, &inventory[item_new], TRUE);
        (void)sprintf(out_val, "You have %s (%c)",
                      tmp_str, index_to_label(item_new));
        prt(out_val, 0, 0);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
    
        /* Handle non-visual stuff */
        handle_stuff(FALSE);

        /* Item is still here */
        if (i == st_ptr->store_ctr) {

            /* Redraw the item */
            display_entry(item_val);
        }

        /* The item is gone */
        else {

            /* Nothing left */
            if (st_ptr->store_ctr == 0) store_top = 0;

            /* Nothing left on that screen */
            else if (store_top >= st_ptr->store_ctr) store_top -= 12;

            /* Redraw everything */
            display_inventory();
        }
    }

    /* Return the result */
    return (purchase);
}


/*
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
    /* Item was worthless, but we bought it */
    if ((value <= 0) && (price > value)) {
        switch (randint(4)) {
          case 1:
            msg_print("You hear a shriek!");
            break;
          case 2:
            msg_print("You bastard!");
            break;
          case 3:
            msg_print("You hear sobs coming from the back of the store...");
            break;
          case 4:
            msg_print("Arrgghh!!!!");
            break;
        }
    }

    /* Item was cheaper than we thought, and we paid more than necessary */
    else if ((value < guess) && (price > value)) {
        switch (randint(3)) {
          case 1:
            msg_print("You hear someone swearing...");
            break;
          case 2:
            msg_print("You hear mumbled curses...");
            break;
          case 3:
            msg_print("The shopkeeper glares at you.");
            break;
        }
    }

    /* Item was a great bargain, and we got away with it */
    else if ((value > (4 * guess)) && (price < value)) {
        switch (randint(4)) {
          case 1:
            msg_print("You hear someone jumping for joy!");
            break;
          case 2:
            msg_print("Yipee!");
            break;
          case 3:
            msg_print("I think I'll retire!");
            break;
          case 4:
            msg_print("The shopkeeper smiles gleefully!");
            break;
        }
    }

    /* Item was a good bargain, and we got away with it */
    else if ((value > guess) && (price < value)) {
        switch (randint(4)) {
          case 1:
            msg_print("You hear someone giggling");
            break;
          case 2:
            msg_print("You've made my day!");
            break;
          case 3:
            msg_print("What a fool!");
            break;
          case 4:
            msg_print("The shopkeeper laughs loudly!");
            break;
        }
    }
}


/*
 * Sell an item to the store	-RAK-	
 */
static int store_sell(void)
{
    int			sell, choice;
    int			item_val, item_pos;
    int			test, i1, i2, amt;

    s32b		price, value, dummy;

    inven_type		sold_obj;
    inven_type		*i_ptr;

    char		out_val[160];
    char		tmp_str[160];


    sell = FALSE;

    /* Analyze the inventory */
    test = i1 = i2 = 0;

    /* Find the first, last, and number of sellable items */
    for (item_val = 0; item_val < inven_ctr; item_val++) {
        i_ptr = &inventory[item_val];
        if (store_will_buy(i_ptr)) {
            if (!test) i1 = item_val;
            i2 = item_val;
            test++;
        }
    }

    /* Check for stuff */
    if (inven_ctr < 1) {
        msg_print("You aren't carrying anything.");
        return (FALSE);
    }

    /* Check for stuff */
    if (test <= 0) {
        msg_print("You have nothing that I want.");
        return (FALSE);
    }

    /* Prepare a prompt */
    sprintf(out_val, "%s which item? ", (store_num == 7) ? "Drop" : "Sell");

    /* Only allow items the store will buy */
    item_tester_hook = store_will_buy;

    /* Semi-Hack -- Get an item */
    if (!get_item(&item_val, out_val, i1, i2, FALSE)) return (FALSE);

    /* Cancel auto-see */
    command_see = FALSE;

    /* Get the actual item */
    i_ptr = &inventory[item_val];

    /* Be sure the shop-keeper will buy those */
    if (!store_will_buy(i_ptr)) {
        msg_print("I do not buy such items.");
        return (FALSE);
    }

    /* Verify inscribed objects if so instructed */
    if ((store_num != 7) && i_ptr->inscrip[0]) {
        cptr pmt = "Really sell that inscribed item?";
        if (other_query_flag && !get_check(pmt)) return (FALSE);
    }

    /* Assume the player wants only one item */
    amt = 1;

    /* Find out how many the player wants (letter means "all") */
    if (i_ptr->number > 1) {

        /* Get a quantity */
        sprintf(out_val, "Quantity (1-%d): ", i_ptr->number);
        prt(out_val, 0, 0);
        sprintf(tmp_str, "%d", amt);
        if (!askfor_aux(tmp_str, 3)) return (FALSE);

        /* Parse it (a letter means "all") */
        amt = atoi(tmp_str);
        if (isalpha(tmp_str[0])) amt = 99;
        if (amt > i_ptr->number) amt = i_ptr->number;
        else if (amt <= 0) return (FALSE);
    }

    /* Create the object to be sold (structure copy) */
    sold_obj = *i_ptr;
    sold_obj.number = amt;

    /* Remove any inscription for stores */
    if (store_num != 7) inscribe(&sold_obj, "");

    /* Is there room in the store (or the home?) */
    if (!store_check_num(&sold_obj)) {
        if (store_num == 7) msg_print("Your home is full.");
        else msg_print("I have not the room in my store to keep it.");
        return (FALSE);
    }


    /* Real store */
    if (store_num != 7) {

        /* Describe the transaction */
        objdes(tmp_str, &sold_obj, TRUE);	
        (void)sprintf(out_val, "Selling %s (%c).",
                      tmp_str, index_to_label(item_val));
        msg_print(out_val);

        /* Haggle for it */
        choice = sell_haggle(&sold_obj, &price);

        /* Sold... */
        if (choice == 0) {

            say_comment1();
            decrease_insults();

            /* Get some money */
            p_ptr->au += price;
            store_prt_gold();

            /* Get the inventory item */
            i_ptr = &inventory[item_val];

            /* Get the "apparent value" */
            dummy = item_value(&sold_obj) * sold_obj.number;

            /* Become "aware" of the item */
            inven_aware(i_ptr);

            /* Know the item fully */
            inven_known(i_ptr);

            /* Re-Create the now-identified object that was sold */
            sold_obj = *i_ptr;
            sold_obj.number = amt;

            /* Remove the object inscription again */
            inscribe(&sold_obj, "");

            /* Get the "actual value" */
            value = item_value(&sold_obj) * sold_obj.number;

            /* Get the description all over again */
            objdes(tmp_str, &sold_obj, TRUE);

            /* Describe the result (in message buffer) */
            (void)sprintf(out_val, "You sold %s for %ld gold.",
                          tmp_str, (long)price);
            msg_print(out_val);

            /* Analyze the prices (and comment verbally) */
            purchase_analyze(price, value, dummy);

            /* Take the item from the player, describe the result */
            inven_item_increase(item_val, -amt);
            inven_item_describe(item_val);
            inven_item_optimize(item_val);

            /* The store gets that item */
            item_pos = store_carry(&sold_obj);

            /* Recalculate bonuses */
            p_ptr->update |= (PU_BONUS);
    
            /* Handle non-visual stuff */
            handle_stuff(FALSE);

            /* Re-display if item is now in store */
            if (item_pos >= 0) {
                store_top = (item_pos / 12) * 12;
                display_inventory();
            }
        }

        else if (choice == 2) {
            sell = TRUE;
        }

        else if (choice == 3) {
            msg_print("How dare you!");
            msg_print("I will not buy that!");
            sell = increase_insults();
        }
    }

    /* Player is at home */
    else {

        /* Describe */
        objdes(tmp_str, &sold_obj, TRUE);	
        (void)sprintf(out_val, "You drop %s.", tmp_str);
        msg_print(out_val);

        /* Take it from the players inventory */
        inven_item_increase(item_val, -amt);
        inven_item_describe(item_val);
        inven_item_optimize(item_val);

        /* Let the store (home) carry it */
        item_pos = home_carry(&sold_obj);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
    
        /* Handle non-visual stuff */
        handle_stuff(FALSE);

        if (item_pos >= 0) {
            store_top = (item_pos / 12) * 12;
            display_inventory();
        }
    }


    /* Return result */
    return (sell);
}



/*
 * Hack -- set this to leave the store
 */
static bool leave_store = FALSE;


/*
 * Process a command in a store
 *
 * Hack -- allow some "normal" commands using one or both of the
 * original/roguelike keys, or even completely unrelated keys.
 */
static void process_command_store(char cmd)
{
    /* Parse the command */
    switch (cmd) {

        /*** Store commands ***/

        case '\n':
        case '\r':
            break;

        case ESCAPE:
            leave_store = TRUE;
            break;

        case CTRL('R'):
            display_store();
            Term_redraw();
            break;

        case ' ':
            if (st_ptr->store_ctr <= 12) {
                msg_print("Entire inventory is shown.");
            }
            else {
                store_top += 12;
                if (store_top >= st_ptr->store_ctr) store_top = 0;
                display_inventory();
            }
            break;

        case 'p':
            leave_store = store_purchase();
            break;

        case 's':
            leave_store = store_sell();
            break;

        case 'g':
            if (store_num != 7) bell();
            else leave_store = store_purchase();
            break;

        case 'd':
            if (store_num != 7) bell();
            else leave_store = store_sell();
            break;


        /*** Inventory commands ***/

        case 'e':
            do_cmd_inven_e();
            break;

        case 'i':
            do_cmd_inven_i();
            break;

        case 'W': case 'w': case '[':
            do_cmd_inven_w();
            break;

        case 'T': case 't': case ']':
            do_cmd_inven_t();
            break;


        /*** Help and Such ***/

        /* Help */
        case '?':
            do_cmd_help("help.hlp"); break;

        /* Character Description */
        case 'C':
            do_cmd_change_name(); break;


        /*** Extra commands ***/

        case ':':
            do_cmd_note(); break;	

        case '@':
            do_cmd_macro(FALSE); break;

        case '!':
            do_cmd_macro(TRUE); break;

        case '&':
            do_cmd_keymap(); break;


        /*** Other Commands ***/

        /* Peruse a Book */
        case 'b': case 'P':
            do_cmd_browse(); break;

        /* Inscribe an object */
        case '{':
            do_cmd_inscribe(); break;

        /* Un-Inscribe an object */
        case '}':
            do_cmd_uninscribe(); break;

        /* Previous message(s). */
        case CTRL('P'):
            do_cmd_messages(); break;


        /*** Handle "choice window" ***/

        /* Hack -- toggle choice window */
        case CTRL('E'):

            /* Hack -- flip the current status */
            choice_default = !choice_default;

            /* XXX XXX Hack -- update choice window */
            if (!choice_default) choice_inven(0, inven_ctr - 1);
            else choice_equip(INVEN_WIELD, INVEN_TOTAL - 1);

            break;


        /* Hack -- Unknown command */
        default:
            bell();
            break;
    }


    /* XXX XXX XXX Save the command */
    /* command_old = command_cmd; */
}


/*
 * Enter a store, and interact with it.
 *
 * Note that "s" and "p" now work in the Home.
 */
void enter_store(int which)
{
    int                  tmp_chr;
    char                 command;


    /* Check the "locked doors" */
    if (store[which].store_open >= turn) {
        msg_print("The doors are locked.");
        return;
    }


    /* Hack -- note we are in a store */
    in_store_flag = TRUE;

    /* No automatic command */
    command_new = 0;


    /* Save the store number */
    store_num = which;

    /* Save the store and owner pointers */
    st_ptr = &store[store_num];
    ot_ptr = &owners[st_ptr->owner];


    /* Start at the beginning */
    store_top = 0;

    /* Display the store */
    display_store();

    /* Do not leave */
    leave_store = FALSE;

    /* Interact with player */
    while (!leave_store) {

        /* XXX XXX Hack -- update choice window */
        if (!choice_default) choice_inven(0, inven_ctr - 1);
        else choice_equip(INVEN_WIELD, INVEN_TOTAL - 1);	

        /* Hack -- Clear line 1 */
        prt("", 1, 0);

        /* Display the legal commands */
        display_commands();

        /* Assume player has read his messages */
        msg_flag = FALSE;

        /* Hack -- Automatic commands */
        if (command_new) {

            /* Do the same command again */
            command = command_new;

            /* Important -- Forget the command */
            command_new = 0;

            /* Hack -- Flush messages */
            prt("", 0, 0);
        }

        /* Get a new command */
        else {

            /* Cursor to the prompt location */
            move_cursor(21, 9);

            /* Get a command */
            command = inkey();
        }

        /* Check the charisma */
        tmp_chr = p_ptr->use_stat[A_CHR];

        /* Process the command */
        process_command_store(command);

        /* Hack -- Redisplay store prices if charisma changes */
        if (tmp_chr != p_ptr->use_stat[A_CHR]) display_inventory();

        /* XXX XXX XXX Mega-Hack -- handle pack overflow */
        if (inventory[INVEN_PACK].tval) {
            msg_print("Your pack is so full that you flee the store.");
            leave_store = TRUE;
        }
    }


    /* Forget the store number, etc */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;


    /* Hack -- turn off the flag */
    in_store_flag = FALSE;

    /* Hack -- use full energy */
    energy_use = 100;


    /* No automatic command */
    command_new = 0;

    /* Cancel "see" mode */
    command_see = FALSE;


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
    p_ptr->update |= (PU_DISTANCE);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_CAVE);
}



/*
 * Shuffle one of the stores.
 */
void store_shuffle(void)
{
    int i, j, n;

    /* Pick a real store to shuffle */
    n = rand_int(MAX_STORES - 1);

    /* Save the store index */
    store_num = n;

    /* Activate that store */
    st_ptr = &store[store_num];

    /* Pick a new owner */
    for (j = st_ptr->owner; j == st_ptr->owner; ) {
        st_ptr->owner = MAX_STORES * rand_int(MAX_OWNERS / MAX_STORES) + n;
    }

    /* Activate the new owner */
    ot_ptr = &owners[st_ptr->owner];

    /* Reset the owner data */
    st_ptr->insult_cur = 0;
    st_ptr->store_open = 0;
    st_ptr->good_buy = 0;
    st_ptr->bad_buy = 0;

    /* Hack -- discount all the items */
    for (i = 0; i < st_ptr->store_ctr; i++) {

        s32b icost, dummy;
        inven_type *i_ptr;

        /* Get the item */
        i_ptr = &st_ptr->store_item[i];

        /* Hack -- Sell all old items for "half price" */
        i_ptr->discount = 50;

        /* Hack -- See "store_carry" */
        dummy = sell_price(&icost, &dummy, i_ptr);

        /* Hack -- Save the "store cost" (not "fixed" yet) */
        i_ptr->scost = -icost;

        /* Mega-Hack -- Note that the item is "on sale" */
        inscribe(i_ptr, "on sale");
    }

    /* Turn it all off */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;
}


/*
 * Maintain the inventory at the stores.
 */
void store_maint(void)
{
    int         i, j;


    /* Maintain every store (except the home) */
    for (i = 0; i < (MAX_STORES - 1); i++) {

        /* Save the store index */
        store_num = i;

        /* Activate that store */
        st_ptr = &store[store_num];

        /* Activate the new owner */
        ot_ptr = &owners[st_ptr->owner];


        /* Store keeper forgives the player */
        st_ptr->insult_cur = 0;


        /* Hack -- prune the black market */
        if (store_num == 6) {

            /* Destroy crappy black market items */
            for (j = st_ptr->store_ctr - 1; j >= 0; j--) {

                inven_type *i_ptr = &st_ptr->store_item[j];

                /* Destroy crappy items */
                if (black_market_crap(i_ptr)) {

                    /* Destroy the item */
                    store_item_increase(j, 0 - i_ptr->number);
                    store_item_optimize(j);
                }
            }
        }


        /* Choose the number of slots to keep */
        j = st_ptr->store_ctr;

        /* Sell a few items */
        j = j - randint(STORE_TURNOVER);

        /* Never keep more than "STORE_MAX_KEEP" slots */
        if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

        /* Always "keep" at least "STORE_MIN_KEEP" items */
        if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

        /* Hack -- prevent "underflow" */
        if (j < 0) j = 0;

        /* Destroy objects until only "j" slots are left */
        while (st_ptr->store_ctr > j) store_delete();


        /* Choose the number of slots to fill */
        j = st_ptr->store_ctr;

        /* Buy some more items */
        j = j + randint(STORE_TURNOVER);

        /* Never keep more than "STORE_MAX_KEEP" slots */
        if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

        /* Always "keep" at least "STORE_MIN_KEEP" items */
        if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

        /* Hack -- prevent "overflow" */
        if (j >= STORE_INVEN_MAX) j = STORE_INVEN_MAX - 1;

        /* Acquire some new items */
        while (st_ptr->store_ctr < j) store_create();
    }


    /* Turn it all off */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;
}


/*
 * Initialize the stores
 */
void store_init(void)
{
    int         j, k;


    /* Build each store */
    for (j = 0; j < MAX_STORES; j++) {

        /* Save the store index */
        store_num = j;

        /* Activate that store */
        st_ptr = &store[store_num];


        /* Pick an owner */
        st_ptr->owner = MAX_STORES * rand_int(MAX_OWNERS / MAX_STORES) + j;

        /* Activate the new owner */
        ot_ptr = &owners[st_ptr->owner];


        /* Initialize the store */
        st_ptr->insult_cur = 0;
        st_ptr->store_open = 0;
        st_ptr->store_ctr = 0;
        st_ptr->good_buy = 0;
        st_ptr->bad_buy = 0;

        /* No items yet */
        for (k = 0; k < STORE_INVEN_MAX; k++) {
            invwipe(&st_ptr->store_item[k]);
        }
    }


    /* Turn it all off */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;
}

