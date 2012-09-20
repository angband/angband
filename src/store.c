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


#undef CTRL
#define CTRL(X) ((X) & 037)


#define MAX_COMMENT_1	6

static cptr comment_1[MAX_COMMENT_1] = {
    "Okay.",
    "Fine.",
    "Accepted!",
    "Agreed!",
    "Done!",
    "Taken!"
};

#define MAX_COMMENT_2A	2

static cptr comment_2a[MAX_COMMENT_2A] = {
    "You try my patience.  %s is final.",
    "My patience grows thin.  %s is final."
};

#define MAX_COMMENT_2B	12

static cptr comment_2b[MAX_COMMENT_2B] = {
    "I can take no less than %s gold pieces.",
    "I will accept no less than %s gold pieces.",
    "Ha!  No less than %s gold pieces.",
    "You knave!  No less than %s gold pieces.",
    "That's a pittance!  I want %s gold pieces.",
    "That's an insult!  I want %s gold pieces.",
    "As if!  How about %s gold pieces?",
    "My arse!  How about %s gold pieces?",
    "May the fleas of 1000 orcs molest you!  Try %s gold pieces.",
    "May your most favourite parts go moldy!  Try %s gold pieces.",
    "May Morgoth find you tasty!  Perhaps %s gold pieces?",
    "Your mother was an Ogre!  Perhaps %s gold pieces?"
};

#define MAX_COMMENT_3A	2

static cptr comment_3a[MAX_COMMENT_3A] = {
    "You try my patience.  %s is final.",
    "My patience grows thin.  %s is final."
};


#define MAX_COMMENT_3B	12

static cptr comment_3b[MAX_COMMENT_3B] = {
    "Perhaps %s gold pieces?",
    "How about %s gold pieces?",
    "I will pay no more than %s gold pieces.",
    "I can afford no more than %s gold pieces.",
    "Be reasonable.  How about %s gold pieces?",
    "I'll buy it as scrap for %s gold pieces.",
    "That is too much!  How about %s gold pieces?",
    "That looks war surplus!  Say %s gold pieces?",
    "Never!  %s is more like it.",
    "That's an insult!  %s is more like it.",
    "%s gold pieces and be thankful for it!",
    "%s gold pieces and not a copper more!"
};

#define MAX_COMMENT_4A	4

static cptr comment_4a[MAX_COMMENT_4A] = {
    "Enough!  You have abused me once too often!",
    "Arghhh!  I have had enough abuse for one day!",
    "That does it!  You shall waste my time no more!",
    "This is getting nowhere!  I'm going to Londis!"
};

#define MAX_COMMENT_4B	4

static cptr comment_4b[MAX_COMMENT_4B] = {
    "Leave my store!",
    "Get out of my sight!",
    "Begone, you scoundrel!",
    "Out, out, out!"
};

#define MAX_COMMENT_5	8

static cptr comment_5[MAX_COMMENT_5] = {
    "Try again.",
    "Ridiculous!",
    "You will have to do better than that!",
    "Do you wish to do business or not?",
    "You've got to be kidding!",
    "You'd better be kidding!",
    "You try my patience.",
    "Hmmm, nice weather we're having."
};

#define MAX_COMMENT_6	4

static cptr comment_6[MAX_COMMENT_6] = {
    "I must have heard you wrong.",
    "I'm sorry, I missed that.",
    "I'm sorry, what was that?",
    "Sorry, what was that again?"
};



/*
 * Successful haggle.
 */
static void say_comment_1(void)
{
    msg_print(comment_1[rand_int(MAX_COMMENT_1)]);
}


/*
 * Continue haggling (player is buying)
 */
static void say_comment_2(s32b offer, s32b asking, int annoyed)
{
    char	tmp_val[80];

    /* Prepare a string to insert */
    sprintf(tmp_val, "%ld", (long)asking);

    /* Final offer */
    if (annoyed > 0) {

        /* Formatted message */
        msg_format(comment_2a[rand_int(MAX_COMMENT_2A)], tmp_val);
    }

    /* Normal offer */
    else {

        /* Formatted message */
        msg_format(comment_2b[rand_int(MAX_COMMENT_2B)], tmp_val);
    }
}


/*
 * Continue haggling (player is selling)
 */
static void say_comment_3(s32b offer, s32b asking, int annoyed)
{
    char	tmp_val[80];

    /* Prepare a string to insert */
    sprintf(tmp_val, "%ld", (long)offer);

    /* Final offer */
    if (annoyed > 0) {

        /* Formatted message */
        msg_format(comment_3a[rand_int(MAX_COMMENT_3A)], tmp_val);
    }

    /* Normal offer */
    else {

        /* Formatted message */
        msg_format(comment_3b[rand_int(MAX_COMMENT_3B)], tmp_val);
    }
}


/*
 * Kick 'da bum out.					-RAK-	
 */
static void say_comment_4(void)
{
    msg_print(comment_4a[rand_int(MAX_COMMENT_4A)]);
    msg_print(comment_4b[rand_int(MAX_COMMENT_4B)]);
}


/*
 * You are insulting me
 */
static void say_comment_5(void)
{
    msg_print(comment_5[rand_int(MAX_COMMENT_5)]);
}


/*
 * That makes no sense.
 */
static void say_comment_6(void)
{
    msg_print(comment_6[rand_int(5)]);
}



/*
 * Messages for reacting to purchase prices.
 */

#define MAX_COMMENT_7A	4

static cptr comment_7a[MAX_COMMENT_7A] = {
    "Arrgghh!",
    "You bastard!",
    "You hear someone sobbing...",
    "The shopkeeper howls in agony!"
};

#define MAX_COMMENT_7B	4

static cptr comment_7b[MAX_COMMENT_7B] = {
    "Damn!",
    "You fiend!",
    "The shopkeeper curses at you.",
    "The shopkeeper glares at you."
};

#define MAX_COMMENT_7C	4

static cptr comment_7c[MAX_COMMENT_7C] = {
    "Cool!",
    "You've made my day!",
    "The shopkeeper giggles.",
    "The shopkeeper laughs loudly."
};

#define MAX_COMMENT_7D	4

static cptr comment_7d[MAX_COMMENT_7D] = {
    "Yipee!",
    "I think I'll retire!",
    "The shopkeeper jumps for joy.",
    "The shopkeeper smiles gleefully."
};

/*
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
    /* Item was worthless, but we bought it */
    if ((value <= 0) && (price > value)) {
        msg_print(comment_7a[rand_int(MAX_COMMENT_7A)]);
    }

    /* Item was cheaper than we thought, and we paid more than necessary */
    else if ((value < guess) && (price > value)) {
        msg_print(comment_7b[rand_int(MAX_COMMENT_7B)]);
    }

    /* Item was a good bargain, and we got away with it */
    else if ((value > guess) && (value < (4 * guess)) && (price < value)) {
        msg_print(comment_7c[rand_int(MAX_COMMENT_7C)]);
    }

    /* Item was a great bargain, and we got away with it */
    else if ((value > guess) && (price < value)) {
        msg_print(comment_7d[rand_int(MAX_COMMENT_7D)]);
    }
}





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
 * Buying and selling adjustments for race combinations.
 * Entry[owner][player] gives the basic "cost inflation".
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
 * Stat Table (CHR) -- payment percentages
 */
static s16b adj_chr[] = {
    130	/* 3 */,
    125	/* 4 */,
    122	/* 5 */,
    120	/* 6 */,
    118	/* 7 */,
    116	/* 8 */,
    114	/* 9 */,
    112	/* 10 */,
    110	/* 11 */,
    108	/* 12 */,
    106	/* 13 */,
    104	/* 14 */,
    103	/* 15 */,
    102	/* 16 */,
    101	/* 17 */,
    100	/* 18 */,
    99	/* 18/01-18/09 */,
    98	/* 18/10-18/19 */,
    97	/* 18/20-18/29 */,
    96	/* 18/30-18/39 */,
    96	/* 18/40-18/49 */,
    95	/* 18/50-18/59 */,
    94	/* 18/60-18/69 */,
    93	/* 18/70-18/79 */,
    92	/* 18/80-18/89 */,
    91	/* 18/90-18/99 */,
    90	/* 18/100 */,
    90	/* 18/101-18/109 */,
    89	/* 18/110-18/119 */,
    88	/* 18/120-18/129 */,
    87	/* 18/130-18/139 */,
    86	/* 18/140-18/149 */,
    85	/* 18/150-18/159 */,
    84	/* 18/160-18/169 */,
    83	/* 18/170-18/179 */,
    82	/* 18/180-18/189 */,
    81	/* 18/190-18/199 */,
    80	/* 18/200-18/209 */,
    80	/* 18/210-18/219 */,
    80	/* 18/220+ */
};




/*
 * Determine the price of an item (qty one) in a store.
 *
 * This function takes into account the player's charisma, and the
 * shop-keepers friendliness, and the shop-keeper's base greed, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * item, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 *
 * Charisma adjustment runs from 80 to 130
 * Racial adjustment runs from 95 to 130
 *
 * Since greed/charisma/racial adjustments are centered at 100, we need
 * to adjust (by 200) to extract a usable multiplier.  Note that the
 * "greed" value is always something (?).
 */
static s32b price_item(inven_type *i_ptr, int greed, bool flip)
{
    int     factor;
    int     adjust;
    s32b    price;


    /* Get the value of one of the items */
    price = item_value(i_ptr);

    /* Worthless items */
    if (price <= 0) return (0L);


    /* Compute the racial factor */
    factor = rgold_adj[ot_ptr->owner_race][p_ptr->prace];

    /* Add in the charisma factor */
    factor += adj_chr[p_ptr->stat_ind[A_CHR]];


    /* Shop is buying */
    if (flip) {

        /* Adjust for greed */
        adjust = 100 + (300 - (greed + factor));

        /* Never get "silly" */
        if (adjust > 100) adjust = 100;

        /* Mega-Hack -- Black market sucks */
        if (store_num == 6) price = price / 2;
    }

    /* Shop is selling */
    else {

        /* Adjust for greed */
        adjust = 100 + ((greed + factor) - 300);

        /* Never get "silly" */
        if (adjust < 100) adjust = 100;

        /* Mega-Hack -- Black market sucks */
        if (store_num == 6) price = price * 2;
    }

    /* Compute the final price (with rounding) */
    price = (price * adjust + 50L) / 100L;

    /* Note -- Never become "free" */
    if (price <= 0L) return (1L);

    /* Return the price */
    return (price);
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
 * Certain "cheap" objects should be created in "piles"
 * Some objects can be sold at a "discount" (in small piles)
 */
static void mass_produce(inven_type *i_ptr)
{
    int size = 1;
    int discount = 0;
    
    s32b cost = item_value(i_ptr);


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

        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SHIELD:
        case TV_GLOVES:
        case TV_BOOTS:
        case TV_CLOAK:
        case TV_HELM:
        case TV_CROWN:
        case TV_SWORD:
        case TV_POLEARM:
        case TV_HAFTED:
        case TV_DIGGING:
        case TV_BOW:
            if (i_ptr->name2) break;
            if (cost <= 10L) size += mass_roll(3,5);
            if (cost <= 100L) size += mass_roll(3,5);
            break;

        case TV_SPIKE:
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
            if (cost <= 5L) size += mass_roll(5,5);
            if (cost <= 50L) size += mass_roll(5,5);
            if (cost <= 500L) size += mass_roll(5,5);
            break;
    }


    /* Pick a discount */
    if (cost < 5) {
        discount = 0;
    }
    else if (rand_int(25) == 0) {
        discount = 25;
    }
    else if (rand_int(150) == 0) {
        discount = 50;
    }
    else if (rand_int(300) == 0) {
        discount = 75;
    }
    else if (rand_int(500) == 0) {
        discount = 90;
    }


    /* Save the discount */
    i_ptr->discount = discount;
    
    /* Save the total pile size */
    i_ptr->number = size - (size * discount / 100);
}








/*
 * Determine if a store item can "absorb" another item
 *
 * See "item_similar()" for the same function for the "player"
 */
static bool store_item_similar(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Identical items cannot be stacked */
    if (i_ptr == j_ptr) return (0);

    /* Different objects cannot be stacked */
    if (i_ptr->k_idx != j_ptr->k_idx) return (0);

    /* Different charges (etc) cannot be stacked */
    if (i_ptr->pval != j_ptr->pval) return (0);

    /* Require many identical values */
    if (i_ptr->to_h  !=  j_ptr->to_h) return (0);
    if (i_ptr->to_d  !=  j_ptr->to_d) return (0);
    if (i_ptr->to_a  !=  j_ptr->to_a) return (0);

    /* Require identical "artifact" names */
    if (i_ptr->name1 != j_ptr->name1) return (0);

    /* Require identical "ego-item" names */
    if (i_ptr->name2 != j_ptr->name2) return (0);

    /* Hack -- Never stack "powerful" items */
    if (i_ptr->xtra1 || j_ptr->xtra1) return (0);

    /* Hack -- Never stack recharging items */
    if (i_ptr->timeout || j_ptr->timeout) return (0);

    /* Require many identical values */
    if (i_ptr->ac    !=  j_ptr->ac)   return (0);
    if (i_ptr->dd    !=  j_ptr->dd)   return (0);
    if (i_ptr->ds    !=  j_ptr->ds)   return (0);

    /* Hack -- Never stack chests */
    if (i_ptr->tval == TV_CHEST) return (0);

    /* Require matching discounts */
    if (i_ptr->discount != j_ptr->discount) return (0);

    /* They match, so they must be similar */
    return (TRUE);
}


/*
 * Allow a store item to absorb another item
 */
static void store_item_absorb(inven_type *i_ptr, inven_type *j_ptr)
{
    int total = i_ptr->number + j_ptr->number;

    /* Combine quantity, lose excess items */
    i_ptr->number = (total > 99) ? 99 : total;
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
    if (st_ptr->stock_num < st_ptr->stock_size) return TRUE;

    /* The "home" acts like the player */
    if (store_num == 7) {

        /* Check all the items */
        for (i = 0; i < st_ptr->stock_num; i++) {

            /* Get the existing item */
            j_ptr = &st_ptr->stock[i];

            /* Can the new object be combined with the old one? */
            if (item_similar(j_ptr, i_ptr)) return (TRUE);
        }
    }

    /* Normal stores do special stuff */
    else {

        /* Check all the items */
        for (i = 0; i < st_ptr->stock_num; i++) {

            /* Get the existing item */
            j_ptr = &st_ptr->stock[i];

            /* Can the new object be combined with the old one? */
            if (store_item_similar(j_ptr, i_ptr)) return (TRUE);
        }
    }

    /* But there was no room at the inn... */
    return (FALSE);
}




/*
 * Determine if the current store will purchase the given item
 *
 * Note that a shop-keeper must refuse to buy "worthless" items
 */
static bool store_will_buy(inven_type *i_ptr)
{
    /* Hack -- The Home is simple */
    if (store_num == 7) return (TRUE);

    /* Switch on the store */
    switch (store_num) {

      /* General Store */
      case 0:

        /* Analyze the type */
        switch (i_ptr->tval) {
          case TV_DIGGING:
          case TV_CLOAK:
          case TV_FOOD:
          case TV_FLASK:
          case TV_LITE:
          case TV_SPIKE:
            break;
          default:
            return (FALSE);
        }
        break;

      /* Armoury */
      case 1:

        /* Analyze the type */
        switch (i_ptr->tval) {
          case TV_BOOTS:
          case TV_GLOVES:
          case TV_CROWN:
          case TV_HELM:
          case TV_SHIELD:
          case TV_CLOAK:
          case TV_SOFT_ARMOR:
          case TV_HARD_ARMOR:
          case TV_DRAG_ARMOR:
            break;
          default:
            return (FALSE);
        }
        break;

      /* Weapon Shop */
      case 2:

        /* Analyze the type */
        switch (i_ptr->tval) {
          case TV_SHOT:
          case TV_BOLT:
          case TV_ARROW:
          case TV_BOW:
          case TV_DIGGING:
          case TV_HAFTED:
          case TV_POLEARM:
          case TV_SWORD:
            break;
          default:
            return (FALSE);
        }
        break;

      /* Temple */
      case 3:

        /* Analyze the type */
        switch (i_ptr->tval) {
          case TV_PRAYER_BOOK:
          case TV_SCROLL:
          case TV_POTION:
          case TV_HAFTED:
            break;
          default:
            return (FALSE);
        }
        break;

      /* Alchemist */
      case 4:

        /* Analyze the type */
        switch (i_ptr->tval) {
          case TV_SCROLL:
          case TV_POTION:
            break;
          default:
            return (FALSE);
        }
        break;

      /* Magic Shop */
      case 5:

        /* Analyze the type */
        switch (i_ptr->tval) {
          case TV_MAGIC_BOOK:
          case TV_AMULET:
          case TV_RING:
          case TV_STAFF:
          case TV_WAND:
          case TV_ROD:
          case TV_SCROLL:
          case TV_POTION:
            break;
          default:
            return (FALSE);
        }
        break;
    }

    /* XXX XXX XXX Ignore "worthless" items */
    if (item_value(i_ptr) <= 0) return (FALSE);

    /* Assume okay */
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


    /* Check each existing item (try to combine) */
    for (slot = 0; slot < st_ptr->stock_num; slot++) {

        /* Get the existing item */
        j_ptr = &st_ptr->stock[slot];

        /* The home acts just like the player */
        if (item_similar(j_ptr, i_ptr)) {

            /* Save the new number of items */
            item_absorb(j_ptr, i_ptr);

            /* All done */
            return (slot);
        }
    }

    /* No space? */
    if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


    /* Determine the "value" of the item */
    value = item_value(i_ptr);

    /* Check existing slots to see if we must "slide" */
    for (slot = 0; slot < st_ptr->stock_num; slot++) {

        /* Get that item */
        j_ptr = &st_ptr->stock[slot];

        /* Hack -- readable books always come first */
        if ((i_ptr->tval == mp_ptr->spell_book) &&
            (j_ptr->tval != mp_ptr->spell_book)) break;
        if ((j_ptr->tval == mp_ptr->spell_book) &&
            (i_ptr->tval != mp_ptr->spell_book)) continue;

        /* Objects sort by decreasing type */
        if (i_ptr->tval > j_ptr->tval) break;
        if (i_ptr->tval < j_ptr->tval) continue;

        /* Can happen in the home */
        if (!inven_aware_p(i_ptr)) continue;
        if (!inven_aware_p(j_ptr)) break;

        /* Objects sort by increasing sval */
        if (i_ptr->sval < j_ptr->sval) break;
        if (i_ptr->sval > j_ptr->sval) continue;

        /* Objects in the home can be unknown */
        if (!inven_known_p(i_ptr)) continue;
        if (!inven_known_p(j_ptr)) break;

        /* Objects sort by decreasing value */
        j_value = item_value(j_ptr);
        if (value > j_value) break;
        if (value < j_value) continue;
    }

    /* Slide the others up */
    for (i = st_ptr->stock_num; i > slot; i--) {
        st_ptr->stock[i] = st_ptr->stock[i-1];
    }

    /* More stuff now */
    st_ptr->stock_num++;

    /* Insert the new item */
    st_ptr->stock[slot] = *i_ptr;

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
    int		i, slot;
    s32b	value, j_value;
    inven_type	*j_ptr;


    /* Evaluate the object */
    value = item_value(i_ptr);

    /* Cursed/Worthless items "disappear" when sold */
    if (value <= 0) return (-1);


    /* Erase the inscription */
    i_ptr->note = 0;

    /* Check each existing item (try to combine) */
    for (slot = 0; slot < st_ptr->stock_num; slot++) {

        /* Get the existing item */
        j_ptr = &st_ptr->stock[slot];

        /* Can the existing items be incremented? */
        if (store_item_similar(j_ptr, i_ptr)) {

            /* Hack -- extra items disappear */
            store_item_absorb(j_ptr, i_ptr);

            /* All done */
            return (slot);
        }
    }

    /* No space? */
    if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


    /* Check existing slots to see if we must "slide" */
    for (slot = 0; slot < st_ptr->stock_num; slot++) {

        /* Get that item */
        j_ptr = &st_ptr->stock[slot];

        /* Objects sort by decreasing type */
        if (i_ptr->tval > j_ptr->tval) break;
        if (i_ptr->tval < j_ptr->tval) continue;

        /* Objects sort by increasing sval */
        if (i_ptr->sval < j_ptr->sval) break;
        if (i_ptr->sval > j_ptr->sval) continue;

        /* Evaluate that slot */
        j_value = item_value(j_ptr);

        /* Objects sort by decreasing value */
        if (value > j_value) break;
        if (value < j_value) continue;
    }

    /* Slide the others up */
    for (i = st_ptr->stock_num; i > slot; i--) {
        st_ptr->stock[i] = st_ptr->stock[i-1];
    }

    /* More stuff now */
    st_ptr->stock_num++;

    /* Insert the new item */
    st_ptr->stock[slot] = *i_ptr;

    /* Return the location */
    return (slot);
}


/*
 * Increase, by a given amount, the number of a certain item
 * in a certain store.  This can result in zero items.
 */
static void store_item_increase(int item, int num)
{
    int         cnt;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &st_ptr->stock[item];

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
static void store_item_optimize(int item)
{
    int         j;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &st_ptr->stock[item];

    /* Must exist */
    if (!i_ptr->k_idx) return;

    /* Must have no items */
    if (i_ptr->number) return;

    /* One less item */
    st_ptr->stock_num--;

    /* Slide everyone */
    for (j = item; j < st_ptr->stock_num; j++) {
        st_ptr->stock[j] = st_ptr->stock[j + 1];
    }

    /* Nuke the final slot */
    invwipe(&st_ptr->stock[st_ptr->stock_num]);
}


/*
 * This function will keep 'crap' out of the black market.
 * Crap is defined as any item that is "available" elsewhere
 * Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
 */
static bool black_market_crap(inven_type *i_ptr)
{
    int		i, j;

    /* Ego items are never crap */
    if (i_ptr->name2) return (FALSE);

    /* Good items are never crap */
    if (i_ptr->to_a > 0) return (FALSE);
    if (i_ptr->to_h > 0) return (FALSE);
    if (i_ptr->to_d > 0) return (FALSE);

    /* Check the other "normal" stores */
    for (i = 0; i < 6; i++) {

        /* Check every item in the store */
        for (j = 0; j < store[i].stock_num; j++) {

            inven_type *j_ptr = &store[i].stock[j];

            /* Duplicate item "type", assume crappy */
            if (i_ptr->k_idx == j_ptr->k_idx) return (TRUE);
        }
    }

    /* Assume okay */
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
    what = rand_int(st_ptr->stock_num);

    /* Determine how many items are here */
    num = st_ptr->stock[what].number;

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
    inven_type		tmp_obj;
    inven_type		*i_ptr = &tmp_obj;


    /* Paranoia -- no room left */
    if (st_ptr->stock_num >= st_ptr->stock_size) return;


    /* Hack -- consider up to four items */
    for (tries = 0; tries < 4; tries++) {

        /* Black Market */
        if (store_num == 6) {

            /* Pick a level for object/magic */
            level = 25 + rand_int(25);

            /* Random item (usually of given level) */
            i = get_obj_num(level);
        }

        /* Normal Store */
        else {

            /* Hack -- Pick an item to sell */
            i = st_ptr->table[rand_int(st_ptr->table_num)];

            /* Hack -- fake level for apply_magic() */
            level = rand_range(1, STORE_OBJ_LEVEL);
        }


        /* Create a new object of the chosen kind */
        invcopy(i_ptr, i);

        /* Apply some "low-level" magic (no artifacts) */
        apply_magic(i_ptr, level, FALSE, FALSE, FALSE);

        /* Hack -- Charge lite's */
        if (i_ptr->tval == TV_LITE) {
            if (i_ptr->sval == SV_LITE_TORCH) i_ptr->pval = FUEL_TORCH / 2;
            if (i_ptr->sval == SV_LITE_LANTERN) i_ptr->pval = FUEL_LAMP / 2;
        }


        /* The item is "known" */
        inven_known(i_ptr);

        /* Mega-Hack -- no chests in stores */
        if (i_ptr->tval == TV_CHEST) continue;

        /* Prune the black market */
        if (store_num == 6) {

            /* Hack -- No "crappy" items */
            if (black_market_crap(i_ptr)) continue;

            /* Hack -- No "cheap" items */
            if (item_value(i_ptr) < 10) continue;

            /* No "worthless" items */
            /* if (item_value(i_ptr) <= 0) continue; */
        }

        /* Prune normal stores */
        else {

            /* No "worthless" items */
            if (item_value(i_ptr) <= 0) continue;
        }


        /* Mass produce and/or Apply discount */
        mass_produce(i_ptr);

        /* Attempt to carry the (known) item */
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
 * Update the bargain info
 */
static void updatebargain(s32b price, s32b minprice)
{
    /* Allow haggling to be turned off */
    if (no_haggle_flag) return;

    /* Cheap items are "boring" */
    if (minprice < 10L) return;

    /* Count the successful haggles */
    if (price == minprice) {

        /* Just count the good haggles */
        if (st_ptr->good_buy < MAX_SHORT) {
            st_ptr->good_buy++;
        }
    }

    /* Count the failed haggles */
    else {

        /* Just count the bad haggles */
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
    /* Clear */
    clear_from(21);

    /* Display the legal commands */
    prt("You may:", 21, 0);

    /* Basic commands */
    prt(" ESC) Exit from Building.", 22, 0);

    /* Browse if necessary */
    if (st_ptr->stock_num > 12) {
        prt(" SPACE) Next page of stock", 23, 0);
    }
    
    /* Home commands */
    if (store_num == 7) {
        prt(" g) Get an item.", 22, 40);
        prt(" d) Drop an item.", 23, 40);
    }

    /* Shop commands */
    else {
        prt(" p) Purchase an item.", 22, 40);
        prt(" s) Sell an item.", 23, 40);
    }
}


/*
 * Displays the set of commands				-RAK-	
 */
static void haggle_commands(void)
{
    prt("Specify an offer (N) or an increment (+N or -N).", 21, 0);
    prt("RETURN) Repeat last offer or increment.", 22, 0);
    prt("ESC) Quit Haggling.", 23, 0);
}



/*
 * Re-displays a single store entry
 */
static void display_entry(int pos)
{
    int			i;
    inven_type		*i_ptr;
    s32b		x;

    char		i_name[80];
    char		out_val[160];


    int maxwid = 75;

    /* Get the item */
    i_ptr = &st_ptr->stock[pos];

    /* Get the "offset" */
    i = (pos % 12);

    /* Label it, clear the line --(-- */
    (void)sprintf(out_val, "%c) ", 'a' + i);
    prt(out_val, i+6, 0);

    /* Describe an item in the home */
    if (store_num == 7) {

        maxwid = 75;

        /* Leave room for weights, if necessary -DRS- */
        if (show_inven_weight) maxwid -= 10;

        /* Describe the object */
        objdes(i_name, i_ptr, TRUE, 3);
        i_name[maxwid] = '\0';
        c_put_str(tval_to_attr[i_ptr->tval], i_name, i+6, 3);

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

        /* Must leave room for the "price" */
        maxwid = 65;

        /* Leave room for weights, if necessary -DRS- */
        if (show_store_weight) maxwid -= 7;

        /* Describe the object (fully) */
        objdes_store(i_name, i_ptr, TRUE, 3);
        i_name[maxwid] = '\0';
        c_put_str(tval_to_attr[i_ptr->tval], i_name, i+6, 3);

        /* Show weights, if turned on -DRS- */
        if (show_store_weight) {

            /* Only show the weight of an individual item */
            int wgt = i_ptr->weight;
            (void)sprintf(out_val, "%3d.%d", wgt / 10, wgt % 10);
            put_str(out_val, i+6, 61);
        }

        /* Display a "fixed" cost */
        if (i_ptr->ident & ID_FIXED) {

            /* Extract the "minimum" price */
            x = price_item(i_ptr, ot_ptr->min_inflate, FALSE);

            /* Actually draw the price (not fixed) */
            (void)sprintf(out_val, "%9ld F", (long)x);
            put_str(out_val, i+6, 68);
        }

        /* Display a "taxed" cost */
        else if (no_haggle_flag) {

            /* Extract the "minimum" price */
            x = price_item(i_ptr, ot_ptr->min_inflate, FALSE);

            /* Hack -- Apply Sales Tax if needed */
            if (!noneedtobargain(x)) x += x / 10;

            /* Actually draw the price (with tax) */
            (void)sprintf(out_val, "%9ld  ", (long)x);
            put_str(out_val, i+6, 68);
        }

        /* Display a "haggle" cost */
        else {

            /* Extrect the "maximum" price */
            x = price_item(i_ptr, ot_ptr->max_inflate, FALSE);

            /* Actually draw the price (not fixed) */
            (void)sprintf(out_val, "%9ld  ", (long)x);
            put_str(out_val, i+6, 68);
        }
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
        if (store_top + k >= st_ptr->stock_num) break;

        /* Display that line */
        display_entry(store_top + k);
    }

    /* Erase the extra lines and the "more" prompt */
    for (i = k; i < 13; i++) prt("", i + 6, 0);

    /* Assume "no current page" */
    put_str("        ", 5, 20);

    /* Visual reminder of "more items" */
    if (st_ptr->stock_num > 12) {

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
    char buf[80];


    /* Erase the screen */
    clear_screen();

    /* The "Home" is special */
    if (store_num == 7) {

        /* Put the owner name */
        put_str("Your Home", 3, 30);

        /* Label the item descriptions */
        put_str("Item Description", 5, 3);

        /* If showing weights, show label */
        if (show_inven_weight) {
            put_str("Weight", 5, 70);
        }
    }

    /* Normal stores */
    else {

        cptr store_name = (f_name + f_info[0x08 + store_num].name);
        cptr owner_name = (ot_ptr->owner_name);
        cptr race_name = race_info[ot_ptr->owner_race].title;

        /* Put the owner name and race */
        sprintf(buf, "%s (%s)", owner_name, race_name);
        put_str(buf, 3, 10);

        /* Show the max price in the store (above prices) */
        sprintf(buf, "%s (%ld)", store_name, (long)(ot_ptr->max_cost));
        prt(buf, 3, 50);

        /* Label the item descriptions */
        put_str("Item Description", 5, 3);

        /* If showing weights, show label */
        if (show_store_weight) {
            put_str("Weight", 5, 60);
        }

        /* Label the asking price (in stores) */
        put_str("Price", 5, 72);

        /* Display the current gold */
        store_prt_gold();
    }

    /* Draw in the inventory */
    display_inventory();

    /* Hack -- Display the commands */
    display_commands();
}



/*
 * Get the ID of a store item and return its value	-RAK-	
 */
static int get_stock(int *com_val, cptr pmt, int i, int j)
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

    /* Clear the prompt */
    prt("", 0, 0);

    return (command != ESCAPE);
}


/*
 * Increase the insult counter and get angry if too many -RAK-	
 */
static int increase_insults(void)
{
    /* Increase insults */
    st_ptr->insult_cur++;

    /* Become insulted */
    if (st_ptr->insult_cur > ot_ptr->insult_max) {

        /* Complain */
        say_comment_4();

        /* Reset insults */
        st_ptr->insult_cur = 0;
        st_ptr->good_buy = 0;
        st_ptr->bad_buy = 0;

        /* Open tomorrow */
        st_ptr->store_open = turn + 25000 + randint(25000);

        /* Closed */
        return (TRUE);
    }

    /* Not closed */
    return (FALSE);
}


/*
 * Decrease insults					-RAK-	
 */
static void decrease_insults(void)
{
    /* Decrease insults */
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
    say_comment_5();

    /* Still okay */
    return (FALSE);
}


/*
 * Mega-Hack -- Enable "increments"
 */
static bool allow_inc = FALSE;

/*
 * Mega-Hack -- Last "increment" during haggling
 */
static s32b last_inc = 0L;


/*
 * Get a haggle
 */
static int get_haggle(cptr pmt, s32b *poffer, s32b price, int final)
{
    s32b		i;

    cptr		p;

    char                buf[128];
    char		out_val[160];


    /* Clear old increment if necessary */
    if (!allow_inc) last_inc = 0L;

    /* Final offer, or no increment */
    if (final || !allow_inc || !last_inc) {
        sprintf(buf, "%s [accept] ", pmt);
    }

    /* Old (negative) increment, and not final */
    else if (last_inc < 0) {
        sprintf(buf, "%s [-%ld] ", pmt, (long)(ABS(last_inc)));
    }

    /* Old (positive) increment, and not final */
    else {
        sprintf(buf, "%s [+%ld] ", pmt, (long)(ABS(last_inc)));
    }

    /* Ask until done */
    while (TRUE) {

        /* Prompt for a string, handle abort */
        prt(buf, 0, 0);
        if (!askfor(out_val, 32)) {
            prt("", 0, 0);
            return (FALSE);
        }

        /* Skip leading spaces */
        for (p = out_val; *p == ' '; p++) ;

        /* Return accepts default */
        if (*p == '\0') {

            /* Accept current price */
            if (final || !allow_inc || !last_inc) {
                *poffer = price;
                last_inc = 0L;
            }

            /* Use previous increment again */
            else {
                *poffer += last_inc;
            }

            /* Done */
            break;
        }

        /* Extract a number */
        i = atol(p);

        /* Handle "incremental" number */
        if ((*p == '+' || *p == '-')) {

            /* Allow increments */
            if (allow_inc) {

                /* Use the given "increment" */
                *poffer += i;
                last_inc = i;
                break;
            }
        }

        /* Handle normal number */
        else {

            /* Use the given "number" */
            *poffer = i;
            last_inc = 0L;
            break;
        }

        /* Warning */
        msg_print("Invalid response.");
    }

    /* Success */
    return (TRUE);
}


/*
 * Receive an offer (from the player)
 *
 * Return TRUE if offer is NOT okay
 */
static bool receive_offer(cptr pmt, s32b *poffer,
                          s32b last_offer, int factor,
                          s32b price, int final)
{
    /* Haggle till done */
    while (TRUE) {

        /* Get a haggle (or cancel) */
        if (!get_haggle(pmt, poffer, price, final)) return (TRUE);

        /* Acceptable offer */
        if (((*poffer) * factor) >= (last_offer * factor)) break;

        /* Insult, and check for kicked out */
        if (haggle_insults()) return (TRUE);

        /* Reject offer (correctly) */
        (*poffer) = last_offer;
    }

    /* Success */
    return (FALSE);
}


/*
 * Haggling routine					-RAK-	
 *
 * Return TRUE if purchase is NOT successful
 */
static bool purchase_haggle(inven_type *i_ptr, s32b *price)
{
    s32b               cur_ask, final_ask;
    s32b               last_offer, offer;
    s32b               x1, x2, x3;
    s32b               min_per, max_per;
    int                flag, loop_flag, noneed;
    int                annoyed = 0, final = FALSE;

    bool		cancel = FALSE;

    cptr		pmt = "Asking";

    char		out_val[160];


    *price = 0;


    /* Extract the starting offer and the final offer */
    cur_ask = price_item(i_ptr, ot_ptr->max_inflate, FALSE);
    final_ask = price_item(i_ptr, ot_ptr->min_inflate, FALSE);

    /* Determine if haggling is necessary */
    noneed = noneedtobargain(final_ask);

    /* Go right to final price if player has bargained well */
    if (noneed || no_haggle_flag) {

        /* No need to haggle */
        if (noneed) {

            /* Message summary */
            msg_print("You eventually agree upon the price.");
            msg_print(NULL);
        }

        /* No haggle option */
        else {

            /* Message summary */
            msg_print("You quickly agree upon the price.");
            msg_print(NULL);

            /* Apply Sales Tax */
            final_ask += final_ask / 10;
        }

        /* Final price */
        cur_ask = final_ask;

        /* Go to final offer */
        pmt = "Final Offer";
        final = TRUE;
    }


    /* Haggle for the whole pile */
    cur_ask *= i_ptr->number;
    final_ask *= i_ptr->number;


    /* List legal commands */
    haggle_commands();

    /* Haggle parameters */
    min_per = ot_ptr->haggle_per;
    max_per = min_per * 3;

    /* Mega-Hack -- artificial "last offer" value */
    last_offer = item_value(i_ptr) * i_ptr->number;
    last_offer = last_offer * (200 - (int)(ot_ptr->max_inflate)) / 100L;
    if (last_offer <= 0) last_offer = 1;

    /* No offer yet */
    offer = 0;

    /* No incremental haggling yet */
    allow_inc = FALSE;

    /* Haggle until done */
    for (flag = FALSE; !flag; ) {

        loop_flag = TRUE;

        while (!flag && loop_flag) {

            (void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
            put_str(out_val, 1, 0);
            cancel = receive_offer("What do you offer? ",
                                     &offer, last_offer, 1, cur_ask, final);

            if (cancel) {
                flag = TRUE;
            }
            else if (offer > cur_ask) {
                say_comment_6();
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

        if (!flag) {
            x1 = 100 * (offer - last_offer) / (cur_ask - last_offer);
            if (x1 < min_per) {
                if (haggle_insults()) {
                    flag = TRUE;
                    cancel = TRUE;
                }
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

            /* Too little */
            if (cur_ask < final_ask) {
                final = TRUE;
                cur_ask = final_ask;
                pmt = "Final Offer";
                annoyed++;
                if (annoyed > 3) {
                    (void)(increase_insults());
                    cancel = TRUE;
                    flag = TRUE;
                }
            }
            else if (offer >= cur_ask) {
                flag = TRUE;
                *price = offer;
            }

            if (!flag) {
                last_offer = offer;
                allow_inc = TRUE;
                prt("", 1, 0);
                (void)sprintf(out_val, "Your last offer: %ld",
                              (long)last_offer);
                put_str(out_val, 1, 39);
                say_comment_2(last_offer, cur_ask, annoyed);
            }
        }
    }

    /* Cancel */
    if (cancel) return (TRUE);

    /* Update bargaining info */
    updatebargain(*price, final_ask);

    /* Do not cancel */
    return (FALSE);
}


/*
 * Haggling routine					-RAK-	
 *
 * Return TRUE if purchase is NOT successful
 */
static bool sell_haggle(inven_type *i_ptr, s32b *price)
{
    s32b               purse, cur_ask, final_ask;
    s32b               last_offer = 0, offer = 0;
    s32b               x1, x2, x3;
    s32b               min_per, max_per;

    int			flag, loop_flag, noneed;
    int			annoyed = 0, final = FALSE;

    bool		cancel = FALSE;

    cptr		pmt = "Offer";

    char		out_val[160];


    *price = 0;


    /* Obtain the starting offer and the final offer */
    cur_ask = price_item(i_ptr, ot_ptr->max_inflate, TRUE);
    final_ask = price_item(i_ptr, ot_ptr->min_inflate, TRUE);

    /* Determine if haggling is necessary */
    noneed = noneedtobargain(final_ask);

    /* Get the owner's payout limit */
    purse = (s32b)(ot_ptr->max_cost);

    /* Go right to final price if player has bargained well */
    if (noneed || no_haggle_flag || (final_ask >= purse)) {

        /* No reason to haggle */
        if (final_ask >= purse) {

            /* Message */
            msg_print("You instantly agree upon the price.");
            msg_print(NULL);

            /* Offer full purse */
            final_ask = purse;
        }
        
        /* No need to haggle */
        else if (noneed) {

            /* Message */
            msg_print("You eventually agree upon the price.");
            msg_print(NULL);
        }

        /* No haggle option */
        else {

            /* Message summary */
            msg_print("You quickly agree upon the price.");
            msg_print(NULL);

            /* Apply Sales Tax */
            final_ask -= final_ask / 10;
        }

        /* Final price */
        cur_ask = final_ask;

        /* Final offer */
        final = TRUE;
        pmt = "Final Offer";
    }

    /* Haggle for the whole pile */
    cur_ask *= i_ptr->number;
    final_ask *= i_ptr->number;


    /* List legal commands */
    haggle_commands();

    /* Haggling parameters */
    min_per = ot_ptr->haggle_per;
    max_per = min_per * 3;

    /* Mega-Hack -- artificial "last offer" value */
    last_offer = item_value(i_ptr) * i_ptr->number;
    last_offer = last_offer * ot_ptr->max_inflate / 100L;

    /* No offer yet */
    offer = 0;

    /* No incremental haggling yet */
    allow_inc = FALSE;

    /* Haggle */
    for (flag = FALSE; !flag; ) {

        do {

            loop_flag = TRUE;

            (void)sprintf(out_val, "%s :  %ld", pmt, (long)cur_ask);
            put_str(out_val, 1, 0);
            cancel = receive_offer("What price do you ask? ",
                                 &offer, last_offer, -1, cur_ask, final);

            if (cancel) {
                flag = TRUE;
            }
            else if (offer < cur_ask) {
                say_comment_6();
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
        while (!flag && loop_flag);

        if (!flag) {
            x1 = 100 * (last_offer - offer) / (last_offer - cur_ask);
            if (x1 < min_per) {
                if (haggle_insults()) {
                    flag = TRUE;
                    cancel = TRUE;
                }
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
                annoyed++;
                if (annoyed > 3) {
                    flag = TRUE;
                    (void)(increase_insults());
                }
            }
            else if (offer <= cur_ask) {
                flag = TRUE;
                *price = offer;
            }

            if (!flag) {
                last_offer = offer;
                allow_inc = TRUE;
                prt("", 1, 0);
                (void)sprintf(out_val,
                             "Your last bid %ld", (long)last_offer);
                put_str(out_val, 1, 39);
                say_comment_3(cur_ask, last_offer, annoyed);
            }
        }
    }

    /* Cancel */
    if (cancel) return (TRUE);

    /* Update bargaining info */
    updatebargain(*price, final_ask);

    /* Do not cancel */
    return (FALSE);
}





/*
 * Buy an item from a store				-RAK-	
 */
static void store_purchase(void)
{
    int			i, amt, choice;
    int			item, item_new;

    s32b		price, best;

    inven_type		sell_obj;
    inven_type		*i_ptr;

    char		i_name[80];

    char		out_val[160];


    /* Empty? */
    if (st_ptr->stock_num <= 0) {
        if (store_num == 7) msg_print("Your home is empty.");
        else msg_print("I am currently out of stock.");
        return;
    }


    /* Find the number of objects on this and following pages */
    i = (st_ptr->stock_num - store_top);

    /* And then restrict it to the current page */
    if (i > 12) i = 12;

    /* Prompt */
    sprintf(out_val, "Which item %s? ",
            (store_num == 7) ? "do you want to take" : "are you interested in");

    /* Get the item number to be bought */
    if (!get_stock(&item, out_val, 0, i-1)) return;

    /* Get the actual index */
    item = item + store_top;

    /* Get the actual item */
    i_ptr = &st_ptr->stock[item];

    /* Assume the player wants just one of them */
    amt = 1;

    /* Hack -- get a "sample" object */
    sell_obj = *i_ptr;
    sell_obj.number = amt;

    /* Hack -- require room in pack */
    if (!inven_carry_okay(&sell_obj)) {
        msg_print("You cannot carry that many different items.");
        return;
    }

    /* Determine the "best" price (per item) */
    best = price_item(&sell_obj, ot_ptr->min_inflate, FALSE);

    /* Find out how many the player wants */
    if (i_ptr->number > 1) {

        /* Hack -- note cost of "fixed" items */
        if ((store_num != 7) && (i_ptr->ident & ID_FIXED)) {
            msg_format("That costs %ld gold per item.", (long)(best));
        }

        /* Get a quantity */
        amt = get_quantity(NULL, i_ptr->number);

        /* Allow user abort */
        if (amt <= 0) return;
    }

    /* Create the object to be sold (structure copy) */
    sell_obj = *i_ptr;
    sell_obj.number = amt;

    /* Hack -- require room in pack */
    if (!inven_carry_okay(&sell_obj)) {
        msg_print("You cannot carry that many items.");
        return;
    }

    /* Attempt to buy it */
    if (store_num != 7) {

        /* Fixed price, quick buy */
        if (i_ptr->ident & ID_FIXED) {

            /* Assume accept */
            choice = 0;

            /* Go directly to the "best" deal */
            price = (best * sell_obj.number);
        }

        /* Haggle for it */
        else {

            /* Describe the object (fully) */
            objdes_store(i_name, &sell_obj, TRUE, 3);

            /* Message */
            msg_format("Buying %s (%c).", i_name, item + 'a');

            /* Haggle for a final price */
            choice = purchase_haggle(&sell_obj, &price);

            /* Hack -- Got kicked out */
            if (st_ptr->store_open >= turn) return;
        }


        /* Player wants it */
        if (choice == 0) {

            /* Fix the item price (if "correctly" haggled) */
            if (price == (best * sell_obj.number)) i_ptr->ident |= ID_FIXED;

            /* Player can afford it */
            if (p_ptr->au >= price) {

                /* Thanks! */
                say_comment_1();

                /* Be happy */
                decrease_insults();

                /* Spend the money */
                p_ptr->au -= price;
                store_prt_gold();

                /* Note how many slots the store used to have */
                i = st_ptr->stock_num;

                /* Remove the bought items from the store */
                store_item_increase(item, -amt);
                store_item_optimize(item);

                /* Hack -- buying an item makes you aware of it */
                inven_aware(&sell_obj);

                /* Hack -- clear the "fixed" flag from the item */
                sell_obj.ident &= ~ID_FIXED;

                /* Describe the transaction */
                objdes(i_name, &sell_obj, TRUE, 3);

                /* Message */
                msg_format("You bought %s for %ld gold.", i_name, (long)price);

                /* Let the player carry it (as if he picked it up) */
                item_new = inven_carry(&sell_obj);

                /* Describe the final result */
                objdes(i_name, &inventory[item_new], TRUE, 3);

                /* Message */
                msg_format("You have %s (%c).", i_name, index_to_label(item_new));

                /* Recalculate bonuses */
                p_ptr->update |= (PU_BONUS);

                /* Handle stuff */
                handle_stuff();

                /* Item is still here */
                if (i == st_ptr->stock_num) {

                    /* Redraw the item */
                    display_entry(item);
                }

                /* The item is gone */
                else {

                    /* Nothing left */
                    if (st_ptr->stock_num == 0) store_top = 0;

                    /* Nothing left on that screen */
                    else if (store_top >= st_ptr->stock_num) store_top -= 12;

                    /* Redraw everything */
                    display_inventory();
                }
            }

            /* Player cannot afford it */
            else {

                /* Simple message (no insult) */
                msg_print("You do not have enough gold.");
            }
        }
    }

    /* Home is much easier */
    else {

        /* Carry the item */
        item_new = inven_carry(&sell_obj);

        /* Take note if we take the last one */
        i = st_ptr->stock_num;

        /* Remove the items from the home */
        store_item_increase(item, -amt);
        store_item_optimize(item);

        /* Describe just the result */
        objdes(i_name, &inventory[item_new], TRUE, 3);

        /* Message */
        msg_format("You have %s (%c).", i_name, index_to_label(item_new));

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Handle stuff */
        handle_stuff();

        /* Hack -- Item is still here */
        if (i == st_ptr->stock_num) {

            /* Redraw the item */
            display_entry(item);
        }

        /* The item is gone */
        else {

            /* Nothing left */
            if (st_ptr->stock_num == 0) store_top = 0;

            /* Nothing left on that screen */
            else if (store_top >= st_ptr->stock_num) store_top -= 12;

            /* Redraw everything */
            display_inventory();
        }
    }

    /* Not kicked out */
    return;
}


/*
 * Sell an item to the store (or home)
 */
static void store_sell(void)
{
    int			choice;
    int			item, item_pos;
    int			amt;

    s32b		price, value, dummy;

    inven_type		sold_obj;
    inven_type		*i_ptr;

    cptr		pmt = "Sell which item? ";

    char		i_name[80];


    /* Prepare a prompt */
    if (store_num == 7) pmt = "Drop which item? ";

    /* Only allow items the store will buy */
    item_tester_hook = store_will_buy;

    /* Get an item (from inven) */
    if (!get_item(&item, pmt, FALSE, TRUE, FALSE)) {
        if (item == -2) msg_print("You have nothing that I want.");
        return;
    }

    /* Get the item (in the pack) */
    if (item >= 0) {
        i_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else {
        i_ptr = &i_list[0 - item];
    }


    /* Assume one item */
    amt = 1;

    /* Find out how many the player wants (letter means "all") */
    if (i_ptr->number > 1) {

        /* Get a quantity */
        amt = get_quantity(NULL, i_ptr->number);
        
        /* Allow user abort */
        if (amt <= 0) return;
    }

    /* Create the object to be sold (structure copy) */
    sold_obj = *i_ptr;
    sold_obj.number = amt;

    /* Get a full description */
    objdes(i_name, &sold_obj, TRUE, 3);

    /* Remove any inscription for stores */
    if (store_num != 7) sold_obj.note = 0;

    /* Is there room in the store (or the home?) */
    if (!store_check_num(&sold_obj)) {
        if (store_num == 7) msg_print("Your home is full.");
        else msg_print("I have not the room in my store to keep it.");
        return;
    }


    /* Real store */
    if (store_num != 7) {

        /* Describe the transaction */
        msg_format("Selling %s (%c).", i_name, index_to_label(item));

        /* Haggle for it */
        choice = sell_haggle(&sold_obj, &price);

        /* Kicked out */
        if (st_ptr->store_open >= turn) return;

        /* Sold... */
        if (choice == 0) {

            say_comment_1();
            decrease_insults();

            /* Get some money */
            p_ptr->au += price;
            store_prt_gold();

            /* Get the inventory item */
            i_ptr = &inventory[item];

            /* Get the "apparent" value */
            dummy = item_value(&sold_obj) * sold_obj.number;

            /* Become "aware" of the item */
            inven_aware(i_ptr);

            /* Know the item fully */
            inven_known(i_ptr);

            /* Re-Create the now-identified object that was sold */
            sold_obj = *i_ptr;
            sold_obj.number = amt;

            /* Get the "actual" value */
            value = item_value(&sold_obj) * sold_obj.number;

            /* Get the description all over again */
            objdes(i_name, &sold_obj, TRUE, 3);

            /* Describe the result (in message buffer) */
            msg_format("You sold %s for %ld gold.", i_name, (long)price);

            /* Analyze the prices (and comment verbally) */
            purchase_analyze(price, value, dummy);

            /* Take the item from the player, describe the result */
            inven_item_increase(item, -amt);
            inven_item_describe(item);
            inven_item_optimize(item);

            /* Recalculate bonuses */
            p_ptr->update |= (PU_BONUS);

            /* Combine pack */
            p_ptr->update |= (PU_COMBINE | PU_REORDER);

            /* Handle stuff */
            handle_stuff();

            /* The store gets that (known) item */
            item_pos = store_carry(&sold_obj);

            /* Re-display if item is now in store */
            if (item_pos >= 0) {
                store_top = (item_pos / 12) * 12;
                display_inventory();
            }
        }
    }

    /* Player is at home */
    else {

        /* Describe */
        msg_format("You drop %s.", i_name);

        /* Take it from the players inventory */
        inven_item_increase(item, -amt);
        inven_item_describe(item);
        inven_item_optimize(item);

        /* Let the store (home) carry it */
        item_pos = home_carry(&sold_obj);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Handle stuff */
        handle_stuff();

        if (item_pos >= 0) {
            store_top = (item_pos / 12) * 12;
            display_inventory();
        }
    }
}



/*
 * Hack -- set this to leave the store
 */
static bool leave_store = FALSE;


/*
 * Process a command in a store
 *
 * Mega-Hack -- allow some "normal" commands using one or both of the
 * original/roguelike keys, or even completely unrelated keys.
 *
 * Note that the "keymaps" are not used.
 *
 * Note the two-step process to allow "fake" use of "command_cmd".
 */
static void store_process_command(void)
{
    char cmd;


    /* Hack -- Automatic commands */
    if (command_new) {

        /* Do the same command again */
        cmd = command_new;

        /* Forget the command */
        command_new = 0;

        /* Flush messages */
        msg_print(NULL);
        
        /* Wipe top line */
        prt("", 0, 0);
    }

    /* Get a new command */
    else {

        /* Assume player has read his messages */
        msg_flag = FALSE;

        /* Cursor to the prompt location */
        move_cursor(21, 9);

        /* Get a command */
        cmd = inkey();
    }


    /* Translate the command */
    switch (cmd) {

        /* Special commands */
        case ESCAPE:
        case ' ':
        case CTRL('R'):
            command_cmd = cmd;
            break;

        /* Purchase (Get) */
        case 'p':
            command_cmd = 'g';
            break;

        /* Sell (Drop) */
        case 's':
            command_cmd = 'd';
            break;

        /* Get (not buy) */
        case 'g':
            command_cmd = ((store_num == 7) ? 'g' : '`');
            break;

        /* Drop (not sell) */
        case 'd':
            command_cmd = ((store_num == 7) ? 'd' : '`');
            break;

        /* Wear an item */
        case 'W': case 'w': case '[':
            command_cmd = '[';
            break;

        /* Take off an item */
        case 'T': case 't': case ']':
            command_cmd = ']';
            break;

        /* Browse a Book */
        case 'b': case 'P':
            command_cmd = 'b';
            break;

        /* Normal commands */
        case CTRL('P'):
        case '?':
        case 'C':
        case ':':
        case '@':
        case '!':
        case '&':
        case '{':
        case '}':
        case '(':
        case ')':
        case 'e':
        case 'i':
        case 'k':
        case CTRL('E'):
        case CTRL('I'):
        case '\n':
        case '\r':
            command_cmd = cmd;
            break;

        /* Illegal commands */
        default:
            command_cmd = '`';
            break;
    }


    /* Parse the command */
    switch (command_cmd) {

        case ESCAPE:
            leave_store = TRUE;
            break;

        case ' ':
            if (st_ptr->stock_num <= 12) {
                msg_print("Entire inventory is shown.");
            }
            else {
                store_top += 12;
                if (store_top >= st_ptr->stock_num) store_top = 0;
                display_inventory();
            }
            break;

        case CTRL('R'):
            display_store();
            Term_redraw();
            break;

        case 'g':
            store_purchase(); break;

        case 'd':
            store_sell(); break;

        case '\n':
        case '\r':
        case CTRL('I'):
            break;

        case 'e':
            do_cmd_equip(); break;

        case 'i':
            do_cmd_inven(); break;

        case '[':
            do_cmd_wield(); break;

        case ']':
            do_cmd_takeoff(); break;

        case 'k':
            do_cmd_destroy(); break;

        case '?':
            do_cmd_help("help.hlp"); break;

        case 'C':
            do_cmd_change_name(); break;

        case ':':
            do_cmd_note(); break;	

        case '@':
            do_cmd_macro(FALSE); break;

        case '!':
            do_cmd_macro(TRUE); break;

        case '&':
            do_cmd_keymap(); break;

        case 'b':
            do_cmd_browse(); break;

        case '{':
            do_cmd_inscribe(); break;

        case '}':
            do_cmd_uninscribe(); break;

        case CTRL('P'):
            do_cmd_messages(); break;


        /*** Handle "choice window" ***/

        /* Hack -- toggle choice window */
        case CTRL('E'):

            /* Hack -- flip the current status */
            choose_default = !choose_default;

            /* Redraw choice window */
            p_ptr->redraw |= (PR_CHOOSE);

            break;


#ifndef ANGBAND_LITE

        /* Dump screen */
        case '(':
            do_cmd_dump(FALSE); break;

        /* Dump screen (with colors) */
        case ')':
            do_cmd_dump(TRUE); break;

#endif


        /* Hack -- Unknown command */
        default:
            bell();
            break;
    }
}


/*
 * Enter a store, and interact with it.
 *
 * Note that "s" and "p" now work in the Home.
 */
void store_enter(int which)
{
    int                  tmp_chr;


    /* Check the "locked doors" */
    if (store[which].store_open >= turn) {
        msg_print("The doors are locked.");
        return;
    }


    /* Hack -- Character is in "icky" mode */
    character_icky = TRUE;


    /* No automatic command */
    command_new = 0;


    /* Save the store number */
    store_num = which;

    /* Save the store and owner pointers */
    st_ptr = &store[store_num];
    ot_ptr = &owners[store_num][st_ptr->owner];


    /* Start at the beginning */
    store_top = 0;

    /* Display the store */
    display_store();

    /* Do not leave */
    leave_store = FALSE;

    /* Interact with player */
    while (!leave_store) {

        /* Hack -- Clear line 1 */
        prt("", 1, 0);

        /* Display the legal commands */
        display_commands();

        /* Hack -- Check the charisma */
        tmp_chr = p_ptr->stat_use[A_CHR];

        /* Process the command */
        store_process_command();

        /* Hack -- Character is still in "icky" mode */
        character_icky = TRUE;

        /* Handle stuff */
        handle_stuff();

        /* Hack -- Redisplay store prices if charisma changes */
        if (tmp_chr != p_ptr->stat_use[A_CHR]) display_inventory();

        /* Mega-Hack -- handle pack overflow */
        if (inventory[INVEN_PACK].k_idx) {

            /* Flee from the store */
            if (store_num != 7) {
                msg_print("Your pack is so full that you flee the store...");
                msg_print(NULL);
                leave_store = TRUE;
            }

            /* The home is too full */
            else if (!store_check_num(&inventory[INVEN_PACK])) {
                msg_print("Your pack is so full that you flee your home...");
                msg_print(NULL);
                leave_store = TRUE;
            }

            /* Hack -- Drop items into the home */
            else {

                int item_pos;

                inven_type sold_obj;

                char i_name[80];


                /* Grab a copy of the item */
                sold_obj = inventory[INVEN_PACK];

                /* Give a message */
                msg_print("Your pack overflows!");

                /* Describe it */
                objdes(i_name, &sold_obj, TRUE, 3);

                /* Message */
                msg_format("You drop %s.", i_name);

                /* Remove it from the players inventory */
                inven_item_increase(INVEN_PACK, -999);
                inven_item_describe(INVEN_PACK);
                inven_item_optimize(INVEN_PACK);

                /* Recalculate bonuses */
                p_ptr->update |= (PU_BONUS);

                /* Handle stuff */
                handle_stuff();

                /* Let the store (home) carry it */
                item_pos = home_carry(&sold_obj);

                /* Redraw the home */
                if (item_pos >= 0) {
                    store_top = (item_pos / 12) * 12;
                    display_inventory();
                }
            }
        }


        /* Hack -- get kicked out of the store */
        if (st_ptr->store_open >= turn) leave_store = TRUE;
    }


    /* Forget the store number, etc */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;


    /* Hack -- Character is no longer in "icky" mode */
    character_icky = FALSE;


    /* Hack -- Cancel automatic command */
    command_new = 0;

    /* Hack -- Cancel "see" mode */
    command_see = FALSE;


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE);
    p_ptr->update |= (PU_MONSTERS);

    /* Redraw stuff */
    p_ptr->redraw |= (PR_WIPE | PR_MAP | PR_BASIC | PR_EXTRA);
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
        st_ptr->owner = rand_int(MAX_OWNERS);
    }

    /* Activate the new owner */
    ot_ptr = &owners[store_num][st_ptr->owner];

    /* Reset the owner data */
    st_ptr->insult_cur = 0;
    st_ptr->store_open = 0;
    st_ptr->good_buy = 0;
    st_ptr->bad_buy = 0;

    /* Hack -- discount all the items */
    for (i = 0; i < st_ptr->stock_num; i++) {

        inven_type *i_ptr;

        /* Get the item */
        i_ptr = &st_ptr->stock[i];

        /* Hack -- Sell all old items for "half price" */
        i_ptr->discount = 50;

        /* Hack -- Items are no longer "fixed price" */
        i_ptr->ident &= ~ID_FIXED;

        /* Mega-Hack -- Note that the item is "on sale" */
        i_ptr->note = quark_add("on sale");
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

    int		old_rating = rating;


    /* Maintain every store (except the home) */
    for (i = 0; i < (MAX_STORES - 1); i++) {

        /* Save the store index */
        store_num = i;

        /* Activate that store */
        st_ptr = &store[store_num];

        /* Activate the new owner */
        ot_ptr = &owners[store_num][st_ptr->owner];


        /* Store keeper forgives the player */
        st_ptr->insult_cur = 0;


        /* Mega-Hack -- prune the black market */
        if (store_num == 6) {

            /* Destroy crappy black market items */
            for (j = st_ptr->stock_num - 1; j >= 0; j--) {

                inven_type *i_ptr = &st_ptr->stock[j];

                /* Destroy crappy items */
                if (black_market_crap(i_ptr)) {

                    /* Destroy the item */
                    store_item_increase(j, 0 - i_ptr->number);
                    store_item_optimize(j);
                }
            }
        }


        /* Choose the number of slots to keep */
        j = st_ptr->stock_num;

        /* Sell a few items */
        j = j - randint(STORE_TURNOVER);

        /* Never keep more than "STORE_MAX_KEEP" slots */
        if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

        /* Always "keep" at least "STORE_MIN_KEEP" items */
        if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

        /* Hack -- prevent "underflow" */
        if (j < 0) j = 0;

        /* Destroy objects until only "j" slots are left */
        while (st_ptr->stock_num > j) store_delete();


        /* Choose the number of slots to fill */
        j = st_ptr->stock_num;

        /* Buy some more items */
        j = j + randint(STORE_TURNOVER);

        /* Never keep more than "STORE_MAX_KEEP" slots */
        if (j > STORE_MAX_KEEP) j = STORE_MAX_KEEP;

        /* Always "keep" at least "STORE_MIN_KEEP" items */
        if (j < STORE_MIN_KEEP) j = STORE_MIN_KEEP;

        /* Hack -- prevent "overflow" */
        if (j >= st_ptr->stock_size) j = st_ptr->stock_size - 1;

        /* Acquire some new items */
        while (st_ptr->stock_num < j) store_create();
    }


    /* Hack -- Restore the rating */
    rating = old_rating;


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
        st_ptr->owner = rand_int(MAX_OWNERS);

        /* Activate the new owner */
        ot_ptr = &owners[store_num][st_ptr->owner];


        /* Initialize the store */
        st_ptr->store_open = 0;
        st_ptr->insult_cur = 0;
        st_ptr->good_buy = 0;
        st_ptr->bad_buy = 0;

        /* Nothing in stock */
        st_ptr->stock_num = 0;

        /* Clear any old items */
        for (k = 0; k < st_ptr->stock_size; k++) {
            invwipe(&st_ptr->stock[k]);
        }
    }


    /* Turn it all off */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;
}

