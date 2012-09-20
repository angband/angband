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


#define MAX_COMMENT_1	14

static cptr comment_1[MAX_COMMENT_1] = {
    "Okay.",
    "Fine.",
    "Accepted!",
    "Agreed!",
    "Done!",
    "Taken!",
    "You drive a hard bargain, but taken.",
    "You'll force me bankrupt, but it's a deal.",
    "Sigh.  I'll take it.",
    "My poor sick children may starve, but done!",
    "Finally!  I accept.",
    "Robbed again.",
    "A pleasure to do business with you!",
    "My spouse will skin me, but accepted."
};

#define MAX_COMMENT_2A	3

static cptr comment_2a[MAX_COMMENT_2A] = {
    "%A2 is my final offer; take it or leave it.",
    "I'll give you no more than %A2.",
    "My patience grows thin.  %A2 is final."
};

#define MAX_COMMENT_2B	16

static cptr comment_2b[MAX_COMMENT_2B] = {
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

#define MAX_COMMENT_3A	3

static cptr comment_3a[MAX_COMMENT_3A] = {
    "I'll pay no more than %A1; take it or leave it.",
    "You'll get no more than %A1 from me.",
    "%A1 and that's final."
};


#define MAX_COMMENT_3B	15

static cptr comment_3b[MAX_COMMENT_3B] = {
    "%A2 for that piece of junk?  No more than %A1.",
    "For %A2 I could own a bundle of those.  Try %A1.",
    "%A2?  NEVER!  %A1 is more like it.",
    "Let's be reasonable...NOT! How about %A1 gold pieces?",
    "%A1 gold for that! That's you, that is!.",
    "%A1 gold pieces and be thankful for it!",
    "%A1 gold pieces and not a copper more.",
    "%A2 gold?  HA!  %A1 is more like it.",
    "Try about %A1 gold.",
    "I wouldn't pay %A2 for your bottom, try %A1.",
    "*CHOKE* For that!?  Let's say %A1.",
    "How about %A1?",
    "That looks war surplus!  Say %A1 gold.",
    "I'll buy it as scrap for %A1.",
    "%A2 is too much, let us say %A1 gold."
};

#define MAX_COMMENT_4A	5

static cptr comment_4a[MAX_COMMENT_4A] = {
    "ENOUGH!  You have abused me once too often!",
    "THAT DOES IT!  You shall waste my time no more!",
    "This is getting nowhere.  I'm going to Londis!",
    "BAHAHAHAHAHA!  No more shall you insult me!",
    "Begone!  I have had enough abuse for one day."
};

#define MAX_COMMENT_4B	5

static cptr comment_4b[MAX_COMMENT_4B] = {
    "Out of my place!",
    "out... Out... OUT!!!",
    "Come back tomorrow.",
    "Leave my place.  Begone!",
    "Come back when thou art richer."
};

#define MAX_COMMENT_5	10

static cptr comment_5[MAX_COMMENT_5] = {
    "You will have to do better than that!",
    "That's an insult!",
    "Do you wish to do business or not?",
    "Hah!  Try again.",
    "Ridiculous!",
    "You've got to be kidding!",
    "You'd better be kidding!",
    "You try my patience.",
    "I don't hear you.",
    "Hmmm, nice weather we're having."
};

#define MAX_COMMENT_6	5

static cptr comment_6[MAX_COMMENT_6] = {
    "I must have heard you wrong.",
    "What was that?",
    "I'm sorry, say that again.",
    "What did you say?",
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
static bool insert_str(char *buf, cptr target, cptr insert)
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
    char	   tmp_val[80];

    /* Prepare a string to insert */
    sprintf(tmp_val, "%ld", (long)number);

    /* Insert it */
    return (insert_str(buf, target, tmp_val));
}


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
    char	comment[160];

    if (annoyed > 0) {
        (void)strcpy(comment, comment_2a[rand_int(MAX_COMMENT_2A)]);
    }
    else {
        (void)strcpy(comment, comment_2b[rand_int(MAX_COMMENT_2B)]);
    }

    insert_lnum(comment, "%A1", offer);
    insert_lnum(comment, "%A2", asking);

    msg_print(comment);
}


/*
 * Continue haggling (player is selling)
 */
static void say_comment_3(s32b offer, s32b asking, int annoyed)
{
    char	comment[160];

    if (annoyed > 0) {
        (void)strcpy(comment, comment_3a[rand_int(MAX_COMMENT_3A)]);
    }
    else {
        (void)strcpy(comment, comment_3b[rand_int(MAX_COMMENT_3B)]);
    }

    insert_lnum(comment, "%A1", offer);
    insert_lnum(comment, "%A2", asking);

    msg_print(comment);
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
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b item_value_base(inven_type *i_ptr)
{
    inven_kind *k_ptr = &k_list[i_ptr->k_idx];

    /* Aware item -- use template cost */
    if (inven_aware_p(i_ptr)) return (k_ptr->cost);

    /* Analyze the type */
    switch (i_ptr->tval) {

      /* Un-aware Food */
      case TV_FOOD: return (1L);

      /* Un-aware Scrolls */
      case TV_SCROLL: return (20L);

      /* Un-aware Potions */
      case TV_POTION: return (20L);

      /* Un-aware Rings */
      case TV_RING: return (45L);

      /* Un-aware Amulets */
      case TV_AMULET: return (45L);

      /* Un-aware Wands */
      case TV_WAND: return (50L);

      /* Un-aware Staffs */
      case TV_STAFF: return (70L);

      /* Un-aware Rods */
      case TV_ROD: return (75L);
    }

    /* Paranoia -- Oops */
    return (0L);
}


/*
 * Return the "real" price of a "known" item, not including discounts
 *
 * Wand and staffs get cost for each charge
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
 */
static s32b item_value_real(inven_type *i_ptr)
{
    s32b value;
    
    inven_kind *k_ptr = &k_list[i_ptr->k_idx];
    

    /* Base cost */
    value = k_ptr->cost;


    /* Artifact */
    if (i_ptr->name1) {

        /* Use the artifact cost instead */
        value = v_list[i_ptr->name1].cost;
    }
        
    /* Ego-Item */
    else if (i_ptr->name2) {

        /* Reward the ego-item with a bonus */
        value += ego_item_value[i_ptr->name2];
    }
    

    /* Wearable items with a "pval" effect */
    if (wearable_p(i_ptr) && (i_ptr->pval)) {

        /* Negative "pval" is always bad */
        if (i_ptr->pval < 0) return (0L);

        /* Give credit for stat bonuses */
        if (i_ptr->flags1 & TR1_STR) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_INT) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_WIS) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_DEX) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_CON) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_CHR) value += (i_ptr->pval * 100L);

        /* Give credit for other things */
        if (i_ptr->flags1 & TR1_STEALTH) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_SEARCH) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_INFRA) value += (i_ptr->pval * 100L);
        if (i_ptr->flags1 & TR1_TUNNEL) value += (i_ptr->pval * 100L);
        
        /* Give credit for attack speed */
        if (i_ptr->flags1 & TR1_BLOWS) value += (i_ptr->pval * 2000L);

        /* Give credit for speed bonus */
        if (i_ptr->flags1 & TR1_SPEED) value += (i_ptr->pval * 30000L);
    }
    
    
    /* Analyze the item */
    switch (i_ptr->tval) {
       
      /* Wands/Staffs */
      case TV_WAND:
      case TV_STAFF:

        /* Pay extra for charges */
        value += ((value / 20) * i_ptr->pval);

        /* Done */
        break;

      /* Rings/Amulets */
      case TV_RING:
      case TV_AMULET:

        /* Hack -- negative bonuses are bad */
        if (i_ptr->toac < 0) return (0L);
        if (i_ptr->tohit < 0) return (0L);
        if (i_ptr->todam < 0) return (0L);

        /* Give credit for bonuses */
        value += ((i_ptr->tohit + i_ptr->todam + i_ptr->toac) * 100L);

        /* Done */
        break;

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

        /* Hack -- negative armor bonus */
        if (i_ptr->toac < 0) return (0L);

        /* Give credit for bonuses */
        value += ((i_ptr->tohit + i_ptr->todam + i_ptr->toac) * 100L);

        /* Done */
        break;

      /* Weapons */
      case TV_DIGGING:
      case TV_HAFTED:
      case TV_SWORD:
      case TV_POLEARM:

        /* Hack -- negative hit/damage bonuses */
        if (i_ptr->tohit + i_ptr->todam < 0) return (0L);

        /* Factor in the bonuses */
        value += ((i_ptr->tohit + i_ptr->todam + i_ptr->toac) * 100L);
        
        /* Done */
        break;

      /* Bows */
      case TV_BOW:

        /* Hack -- negative hit/damage bonuses */
        if (i_ptr->tohit + i_ptr->todam < 0) return (0L);

        /* Factor in the bonuses */
        value += ((i_ptr->tohit + i_ptr->todam + i_ptr->toac) * 100L);

        /* Done */
        break;

      /* Ammo */
      case TV_SHOT:
      case TV_ARROW:
      case TV_BOLT:

        /* Hack -- negative hit/damage bonuses */
        if (i_ptr->tohit + i_ptr->todam < 0) return (0L);

        /* Factor in the bonuses */
        value += ((i_ptr->tohit + i_ptr->todam) * 5L);
        
        /* Done */
        break;
    }


    /* Return the value */
    return (value);
}


/*
 * Return the price of an item including plusses (and charges)
 *
 * This function returns the "value" of the given item (qty one)
 *
 * Never notice "unknown" bonuses or properties, including "curses",
 * since that would give the player information he did not have.
 *
 * Note that discounted items stay discounted forever, even if
 * the discount is "forgotten" by the player via memory loss.
 */
s32b item_value(inven_type *i_ptr)
{
    s32b value;


    /* Unknown items -- acquire a base value */
    if (inven_known_p(i_ptr)) {

        /* Broken items -- worthless */
        if (broken_p(i_ptr)) return (0L);
            
        /* Cursed items -- worthless */
        if (cursed_p(i_ptr)) return (0L);
            
        /* Real value (see above) */
        value = item_value_real(i_ptr);
    }

    /* Known items -- acquire the actual value */
    else {

        /* Hack -- Felt broken items */
        if ((i_ptr->ident & ID_SENSE) && broken_p(i_ptr)) return (0L);

        /* Hack -- Felt cursed items */
        if ((i_ptr->ident & ID_SENSE) && cursed_p(i_ptr)) return (0L);

        /* Base value (see above) */
        value = item_value_base(i_ptr);
    }


    /* Apply discount (if any) */
    if (i_ptr->discount) value -= (value * i_ptr->discount / 100L);


    /* Return the final value */
    return (value);
}



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
    int     adjust;
    s32b    price;


    /* Get the value of the item (qty one) */
    price = item_value(i_ptr);

    /* Worthless items */
    if (price <= 0) return (0L);

    
    /* Compute the racial adjustment */
    adjust = rgold_adj[ot_ptr->owner_race][p_ptr->prace];

    /* Add in the charisma factor */
    adjust += adj_chr[stat_index(A_CHR)];
    

    /* Shop is buying */
    if (flip) {

        /* Adjust for greed */
        adjust = 100 + (300 - (greed + adjust));

        /* Never get "silly" */
        if (adjust > 100) adjust = 100;
    }

    /* Shop is selling */
    else {

        /* Adjust for greed */
        adjust = 100 + ((greed + adjust) - 300);

        /* Never get "silly" */
        if (adjust < 100) adjust = 100;
    }

    /* Compute the final price (with rounding) */
    price = (price * adjust + 50L) / 100L;

    /* Mega-Hack -- Black Market costs extra */
    if ((greed > 100) && (store_num == 6)) price = price * 2L;
    
    /* Never become "free" */
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
 * Some objects naturally occur in "piles" of certain sizes
 * All objects can be sold at a discounted rate (in smaller piles)
 * Hack -- never discount "really cheap" items, it is annoying
 */
static void mass_produce(inven_type *i_ptr)
{
    int size = i_ptr->number;

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
 * Determine if a store item can "absorb" another item
 * See "item_similar()" for the same function for the "player"
 */
static int store_item_similar(inven_type *i_ptr, inven_type *j_ptr)
{
    /* Hack -- Identical items cannot be stacked */
    if (i_ptr == j_ptr) return (0);

    /* Different objects cannot be stacked */
    if (i_ptr->k_idx != j_ptr->k_idx) return (0);


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


    /* Paranoia -- Make sure the weight's are different */
    if (i_ptr->weight != j_ptr->weight) return (0);

    /* Paranoia -- Make sure the timeout's are different */
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

        /* Evaluate that slot */
        j_value = item_value(j_ptr);

        /* Objects sort by decreasing value */
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
    if (!i_ptr->k_idx) return;

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
 * Crap is defined as any item that is "available" elsewhere
 * Based on a suggestion by "Lee Vogt" <lvogt@cig.mcel.mot.com>
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


    /* Check the other "normal" stores */
    for (i = 0; i < 6; i++) {

        /* Check every item */
        for (j = 0; j < store[i].store_ctr; j++) {

            /* Access the store item */
            inven_type *j_ptr = &store[i].store_item[j];

            /* Duplicate item "type", assume crappy */
            if (i_ptr->k_idx == j_ptr->k_idx) return (TRUE);
        }
    }


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
    inven_type		tmp_obj;
    inven_type		*i_ptr = &tmp_obj;


    /* Paranoia -- no room left */
    if (st_ptr->store_ctr >= STORE_INVEN_MAX) return;


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

            /* Pick an item to sell */
            i = store_choice_kind[store_num][rand_int(STORE_CHOICES)];

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

        /* Hack -- no chests in stores */
        if (i_ptr->tval == TV_CHEST) continue;
        
        /* Prune the black market */
        if (store_num == 6) {
        
            /* Hack -- No cheap items */
            if (item_value(i_ptr) <= 2) continue;
        
            /* Hack -- No duplicate items */
            if (black_market_crap(i_ptr)) continue;
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

    char		i_name[80];
    char		out_val[160];


    int maxwid = 75;

    /* Get the item */
    i_ptr = &st_ptr->store_item[pos];

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
            s32b x = price_item(i_ptr, ot_ptr->min_inflate, FALSE);

            /* Actually draw the price (not fixed) */
            (void)sprintf(out_val, "%9ld F", (long)x);
            put_str(out_val, i+6, 68);
        }

        /* Display a "haggle" cost */
        else {
        
            /* Extrect the "maximum" price */
            s32b x = price_item(i_ptr, ot_ptr->max_inflate, FALSE);

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

        /* Put the owner name and race */
        sprintf(buf, "%s (%s)",
                ot_ptr->owner_name,
                race_info[ot_ptr->owner_race].trace);
        put_str(buf, 3, 10);
        
        /* Show the max price in the store (above prices) */
        sprintf(buf, "%s (%ld)",
                k_list[OBJ_STORE_LIST+store_num].name,
                (long)(ot_ptr->max_cost));
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

    /* Clear the prompt */
    prt("", 0, 0);

    return (command != ESCAPE);
}


/*
 * Increase the insult counter and get angry if too many -RAK-	
 */
static int increase_insults(void)
{
    st_ptr->insult_cur++;

    if (st_ptr->insult_cur > ot_ptr->insult_max) {
        say_comment_4();
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
        for (p = out_val; *p == ' '; p++);

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
    s32b               cost, cur_ask, final_ask;
    s32b               last_offer, offer;
    s32b               x1, x2, x3;
    s32b               min_per, max_per;
    int                flag, loop_flag, noneed;
    int                annoyed = 0, final = FALSE;

    bool		cancel = FALSE;
    
    cptr		pmt = "Asking";

    char		out_val[160];


    *price = 0;

        
    /* Extract actual cost */
    cost = item_value(i_ptr) * i_ptr->number;

    /* Extract the starting offer */
    cur_ask = price_item(i_ptr, ot_ptr->max_inflate, FALSE) * i_ptr->number;

    /* Extract the final offer */
    final_ask = price_item(i_ptr, ot_ptr->min_inflate, FALSE) * i_ptr->number;

    /* Determine if bargaining is necessary */
    noneed = noneedtobargain(final_ask / i_ptr->number);

    /* List legal commands */
    haggle_commands();

    /* Haggle parameters */
    min_per = ot_ptr->haggle_per;
    max_per = min_per * 3;

    /* Mega-Hack -- artificial "last offer" value */
    last_offer = cost * (200 - (int)(ot_ptr->max_inflate)) / 100L;
    if (last_offer <= 0) last_offer = 1;

    offer = 0;

    /* No incremental haggling yet */
    allow_inc = FALSE;
    
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

            /* Tax of 10% if you have disabled haggling */
            final_ask += final_ask / 10;
        }
        
        /* Go to final offer */
        pmt = "Final Offer";
        cur_ask = final_ask;
        final = TRUE;
    }

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
    s32b               purse, cost, cur_ask, final_ask;
    s32b               last_offer = 0, offer = 0;
    s32b               x1, x2, x3;
    s32b               min_per, max_per;

    int			flag, loop_flag, noneed;
    int			annoyed = 0, final = FALSE;

    bool		cancel = FALSE;

    cptr		pmt = "Offer";

    char		out_val[160];


    *price = 0;


    /* Get the maximum amount the owner will spend */
    purse = (s32b)(ot_ptr->max_cost) * i_ptr->number;
    
    /* Get the value of a single item */
    cost = item_value(i_ptr) * i_ptr->number;

    /* Extract the opening bid */
    cur_ask = price_item(i_ptr, ot_ptr->max_inflate, TRUE) * i_ptr->number;

    /* Extract the final offer */
    final_ask = price_item(i_ptr, ot_ptr->min_inflate, TRUE) * i_ptr->number;

    /* Determine if haggling is necessary */
    noneed = noneedtobargain(final_ask / i_ptr->number);

    /* Haggling parameters */
    min_per = ot_ptr->haggle_per;
    max_per = min_per * 3;

    /* List legal commands */
    haggle_commands();

    /* Item is too expensive, do not bother to haggle */
    if (cur_ask > purse) {

        /* Final offer */
        final = TRUE;
        pmt = "Final Offer";

        /* Start and Stop at the limit */
        cur_ask = final_ask = purse;

        /* Explain the situation */
        msg_format("I am sorry, but I have not %s.",
                   "the money to afford such a fine item");
    }

    /* Haggle */
    else {

        /* Never offer more than we have to give */
        if (final_ask > purse) final_ask = purse;

        /* Go right to final price if player has bargained well */
        if (noneed || no_haggle_flag) {

            /* No need to haggle */
            if (noneed) {

                /* Message */
                msg_print("You eventually agree upon the price.");
                msg_print(NULL);
            }
            
            /* No haggle option */
            else {
            
                /* Message summary */
                msg_print("You quickly agree upon the price.");
                msg_print(NULL);

                /* Tax of 10% if you have disabled haggling */
                final_ask -= final_ask / 10;
            }
            
            /* Final price */
            cur_ask = final_ask;

            /* Final offer */
            final = TRUE;
            pmt = "Final Offer";
        }
    }


    /* Mega-Hack -- artificial "last offer" value */
    last_offer = cost * ot_ptr->max_inflate / 100L;
    
    offer = 0;

    /* No incremental haggling yet */
    allow_inc = FALSE;

    if (cur_ask < 1) cur_ask = 1;

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
static int store_purchase(void)
{
    int			i, amt, choice;
    int			item_val, item_new;

    s32b		price, best;

    inven_type		sell_obj;
    inven_type		*i_ptr;

    char		i_name[80];

    char		out_val[160];


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

    /* Determine the "best" price (per item) */
    best = price_item(&sell_obj, ot_ptr->min_inflate, FALSE);

    /* Find out how many the player wants */
    if (i_ptr->number > 1) {

        /* Hack -- note cost of "fixed" items */
        if ((store_num != 7) && (i_ptr->ident & ID_FIXED)) {
            msg_format("That costs %ld gold per item.", (long)(best));
        }

        /* Find out how many to buy (letter means "all") */
        sprintf(out_val, "Quantity (1-%d): ", i_ptr->number);
        prt(out_val, 0, 0);
        sprintf(out_val, "%d", amt);
        if (!askfor_aux(out_val, 3)) return (FALSE);
        
        /* Parse it (a letter means "all") */
        amt = atoi(out_val);
        if (isalpha(out_val[0])) amt = 99;
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
            msg_format("Buying %s (%c)", i_name, item_val + 'a');

            /* Haggle for a final price */
            choice = purchase_haggle(&sell_obj, &price);

            /* Hack -- Got kicked out */
            if (st_ptr->store_open >= turn) return (TRUE);
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
                i = st_ptr->store_ctr;

                /* Remove the bought items from the store */
                store_item_increase(item_val, -amt);
                store_item_optimize(item_val);

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
        i = st_ptr->store_ctr;

        /* Remove one item from the home */
        store_item_increase(item_val, -amt);
        store_item_optimize(item_val);

        /* Describe just the result */
        objdes(i_name, &inventory[item_new], TRUE, 3);

        /* Message */
        msg_format("You have %s (%c).", i_name, index_to_label(item_new));

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);
    
        /* Handle stuff */
        handle_stuff();

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

    /* Not kicked out */
    return (FALSE);
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
static void store_sell(void)
{
    int			choice;
    int			item_val, item_pos;
    int			test, i, amt;

    s32b		price, value, dummy;

    inven_type		sold_obj;
    inven_type		*i_ptr;

    char		i_name[80];

    char		out_val[160];


    /* Check for stuff */
    if (inven_ctr <= 0) {
        msg_print("You aren't carrying anything.");
        return;
    }

    /* Count sellable items */
    for (test = i = 0; i < inven_ctr; i++) {
        if (store_will_buy(&inventory[i])) test++;
    }

    /* Prepare a prompt */
    sprintf(out_val, "%s which item? ", (store_num == 7) ? "Drop" : "Sell");

    /* Only allow items the store will buy */
    item_tester_hook = store_will_buy;

    /* Semi-Hack -- Get an item */
    if (!get_item(&item_val, out_val, 0, inven_ctr, FALSE)) {
        if (item_val == -2) msg_print("You have nothing that I want.");
        return;
    }

    /* Get the actual item */
    i_ptr = &inventory[item_val];

    /* Be sure the shop-keeper will buy those */
    if (!store_will_buy(i_ptr)) {
        msg_print("I do not buy such items.");
        return;
    }


    /* Assume the player wants only one item */
    amt = 1;

    /* Find out how many the player wants (letter means "all") */
    if (i_ptr->number > 1) {

        /* Get a quantity */
        sprintf(out_val, "Quantity (1-%d): ", i_ptr->number);
        prt(out_val, 0, 0);
        sprintf(out_val, "%d", amt);
        if (!askfor_aux(out_val, 3)) return;

        /* Parse it (a letter means "all") */
        amt = atoi(out_val);
        if (isalpha(out_val[0])) amt = 99;
        if (amt > i_ptr->number) amt = i_ptr->number;
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
        msg_format("Selling %s (%c).", i_name, index_to_label(item_val));

        /* Refuse to purchase crap */
        if (item_value(&sold_obj) <= 0) {

            /* Act insulted */
            msg_print("How dare you!");
            msg_print("I will not buy that!");

            /* Insult some more */
            (void)increase_insults();

            /* Do not haggle */
            return;
        }

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
            i_ptr = &inventory[item_val];

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
            inven_item_increase(item_val, -amt);
            inven_item_describe(item_val);
            inven_item_optimize(item_val);

            /* Recalculate bonuses */
            p_ptr->update |= (PU_BONUS);
    
            /* Handle stuff */
            handle_stuff();

            /* The store gets that (known) item */
            item_pos = store_carry(&sold_obj);

            /* Re-display if item is now in store */
            if (item_pos >= 0) {
                store_top = (item_pos / 12) * 12;
                display_inventory();
            }

            /* Recombine the pack */
            combine_pack();
        }
    }

    /* Player is at home */
    else {

        /* Describe */
        msg_format("You drop %s.", i_name);

        /* Take it from the players inventory */
        inven_item_increase(item_val, -amt);
        inven_item_describe(item_val);
        inven_item_optimize(item_val);

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

        /* Hack -- Flush messages */
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
            if (st_ptr->store_ctr <= 12) {
                msg_print("Entire inventory is shown.");
            }
            else {
                store_top += 12;
                if (store_top >= st_ptr->store_ctr) store_top = 0;
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
            choice_default = !choice_default;

            /* Redraw choice window */
            p_ptr->redraw |= (PR_CHOICE);

            break;


        /* Hack -- Unknown command */
        default:
            bell();
            break;
    }


    /* Mega-Hack -- Save the command */
    command_old = command_cmd;
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


    /* Character is in "icky" mode */
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

        /* Check the charisma */
        tmp_chr = p_ptr->use_stat[A_CHR];

        /* Process the command */
        store_process_command();

        /* Character is still in "icky" mode */
        character_icky = TRUE;

        /* Handle stuff */
        handle_stuff();

        /* Hack -- Redisplay store prices if charisma changes */
        if (tmp_chr != p_ptr->use_stat[A_CHR]) display_inventory();

        /* XXX XXX XXX Mega-Hack -- handle pack overflow */
        if (inventory[INVEN_PACK].k_idx) {

            /* Flee from the store */
            if (store_num != 7) {
                msg_print("Your pack is so full that you flee the store!");
                msg_print(NULL);
                leave_store = TRUE;
            }
            
            /* The home is too full */
            else if (!store_check_num(&inventory[INVEN_PACK])) {
                msg_print("Your pack is so full that you flee your home!");
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


    /* Character is no longer in "icky" mode */
    character_icky = FALSE;
    
    /* Hack -- use full energy */
    energy_use = 100;


    /* Hack -- Cancel automatic command */
    command_new = 0;

    /* Hack -- Cancel "see" mode */
    command_see = FALSE;


    /* Update stuff */
    p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);
    p_ptr->update |= (PU_DISTANCE);
    
    /* Redraw stuff */
    p_ptr->redraw |= (PR_CAVE);
}



/*
 * Add an item to a store.  Used when parsing savefiles.
 */
void store_acquire(int which, inven_type *i_ptr)
{
    /* Save the store index */
    store_num = which;

    /* Activate that store */
    st_ptr = &store[store_num];

    /* Activate the new owner */
    ot_ptr = &owners[store_num][st_ptr->owner];

    /* Real store */
    if (store_num != 7) {

        /* The item must be known */    
        inven_known(i_ptr);
        
        /* Carry it */
        (void)store_carry(i_ptr);
    }
    
    /* Player Home */
    else {

        /* Carry it */
        (void)home_carry(i_ptr);
    }
    
    /* Turn it all off */
    store_num = 0;
    st_ptr = NULL;
    ot_ptr = NULL;
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
    for (i = 0; i < st_ptr->store_ctr; i++) {

        inven_type *i_ptr;

        /* Get the item */
        i_ptr = &st_ptr->store_item[i];

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

