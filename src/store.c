/* File: store.c */

/*
 * Copyright (c) 1997-2005 Andrew Sidwell, Ben Harrison, James E. Wilson,
 *                         Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "script.h"
#include "cmds.h"

/*** Constants and definitions ***/

/* Easy names for the elements of the 'scr_places' arrays. */
enum
{
	LOC_PRICE = 0,
	LOC_OWNER,
	LOC_HEADER,
	LOC_ITEMS_START,
	LOC_ITEMS_END,
	LOC_MORE,
	LOC_HELP_CLEAR,
	LOC_HELP_PROMPT,
	LOC_AU,
	LOC_WEIGHT,

	LOC_MAX
};

/* Places for the various things displayed onscreen */
static unsigned int scr_places_x[LOC_MAX];
static unsigned int scr_places_y[LOC_MAX];


/* State flags */
#define STORE_GOLD_CHANGE      0x01
#define STORE_FRAME_CHANGE     0x02

#define STORE_SHOW_HELP        0x04
#define STORE_KEEP_PROMPT      0x08





/* Compound flag for the initial display of a store */
#define STORE_INIT_CHANGE		(STORE_FRAME_CHANGE | STORE_GOLD_CHANGE)



/* Some local constants */
#define STORE_TURNOVER  9       /* Normal shop turnover, per day */
#define STORE_OBJ_LEVEL 5       /* Magic Level for normal stores */
#define STORE_MIN_KEEP  6       /* Min slots to "always" keep full (>0) */
#define STORE_MAX_KEEP  18      /* Max slots to "always" keep full */



/** Variables to maintain state XXX ***/

/* Current store number */
static int store_current;

/* Flags for the display */
static u16b store_flags;




/*** Utilities ***/

/*
 * Return the owner struct for the given store.
 */
owner_type *store_owner(int st)
{
	store_type *st_ptr = &store[st];
	return &b_info[(st * z_info->b_max) + st_ptr->owner];
}


/* Randomly select one of the entries in an array */
#define ONE_OF(x)	x[rand_int(N_ELEMENTS(x))]



/*** Flavour text stuff ***/

/*
 * Shopkeeper welcome messages.
 *
 * The shopkeeper's name must come first, then the character's name.
 */
static const char *comment_welcome[] =
{
	"",
	"%s nods to you.",
	"%s says hello.",
	"%s: \"See anything you like, adventurer?\"",
	"%s: \"How may I help you, %s?\"",
	"%s: \"Welcome back, %s.\"",
	"%s: \"A pleasure to see you again, %s.\"",
	"%s: \"How may I be of assistance, good %s?\"",
	"%s: \"You do honour to my humble store, noble %s.\"",
	"%s: \"I and my family are entirely at your service, glorious %s.\""
};

/*
 * Messages for reacting to purchase prices.
 */
static const char *comment_worthless[] =
{
	"Arrgghh!",
	"You bastard!",
	"You hear someone sobbing...",
	"The shopkeeper howls in agony!",
	"The shopkeeper wails in anguish!",
	"The shopkeeper beats his head against the counter."
};

static const char *comment_bad[] =
{
	"Damn!",
	"You fiend!",
	"The shopkeeper curses at you.",
	"The shopkeeper glares at you.",
};

static const char *comment_accept[] =
{
	"Okay.",
	"Fine.",
	"Accepted!",
	"Agreed!",
	"Done!",
	"Taken!"
};

static const char *comment_good[] =
{
	"Cool!",
	"You've made my day!",
	"The shopkeeper sniggers.",
	"The shopkeeper giggles.",
	"The shopkeeper laughs loudly."
};

static const char *comment_great[] =
{
	"Yipee!",
	"I think I'll retire!",
	"The shopkeeper jumps for joy.",
	"The shopkeeper smiles gleefully.",
	"Wow.  I'm going to name my new villa in your honour."
};



/*
 * The greeting a shopkeeper gives the character says a lot about his
 * general attitude.
 *
 * Taken and modified from Sangband 1.0.
 */
static void prt_welcome(const owner_type *ot_ptr)
{
	char short_name[20];
	const char *player_name;

	const char *owner_name = &b_name[ot_ptr->owner_name];
	int i = p_ptr->lev / 5;

	/* Only show the message one in four times to stop it being irritating. */
	if (!rand_int(4)) return;

	/* Welcome the character */
	if (i)
	{
		int j;

		/* Extract the first name of the store owner (stop before the first space) */
		for (j = 0; owner_name[j] && owner_name[j] != ' '; j++)
			short_name[j] = owner_name[j];

		/* Truncate the name */
		short_name[j] = '\0';


		/* Get a title for the character */
		if ((i % 2) && rand_int(2)) player_name = c_text + cp_ptr->title[(p_ptr->lev - 1) / 5];
		else if (rand_int(2))       player_name = op_ptr->full_name;
		else                        player_name = (p_ptr->psex == SEX_MALE ? "sir" : "lady");

		/* Balthazar says "Welcome" */
		prt(format(comment_welcome[i], short_name, player_name), 0, 0);
	}
}



/*
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(s32b price, s32b value, s32b guess)
{
	/* Item was worthless, but we bought it */
	if ((value <= 0) && (price > value))
		message(MSG_STORE1, 0, ONE_OF(comment_worthless));

	/* Item was cheaper than we thought, and we paid more than necessary */
	else if ((value < guess) && (price > value))
		message(MSG_STORE2, 0, ONE_OF(comment_bad));

	/* Item was a good bargain, and we got away with it */
	else if ((value > guess) && (value < (4 * guess)) && (price < value))
		message(MSG_STORE3, 0, ONE_OF(comment_good));

	/* Item was a great bargain, and we got away with it */
	else if ((value > guess) && (price < value))
		message(MSG_STORE4, 0, ONE_OF(comment_great));
}




/*** Check if a store will buy an object ***/

/*
 * Determine if the current store will purchase the given object
 *
 * Note that a shop-keeper must refuse to buy "worthless" objects
 */
static bool store_will_buy(int store_num, const object_type *o_ptr)
{
	/* Hack -- The Home is simple */
	if (store_num == STORE_HOME) return (TRUE);

	/* Switch on the store */
	switch (store_num)
	{
		/* General Store */
		case STORE_GENERAL:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SPIKE:
				case TV_SHOT:
				case TV_ARROW:
				case TV_BOLT:
				case TV_DIGGING:
				case TV_CLOAK:
					break;

				case TV_LITE:
					if (artifact_p(o_ptr) || ego_item_p(o_ptr))
						break;

				default:
					return (FALSE);
			}
			break;
		}

		/* Armoury */
		case STORE_ARMOR:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
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
		}

		/* Weapon Shop */
		case STORE_WEAPON:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
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
		}

		/* Temple */
		case STORE_TEMPLE:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_PRAYER_BOOK:
				case TV_SCROLL:
				case TV_POTION:
				case TV_HAFTED:
					break;

				case TV_POLEARM:
				case TV_SWORD:
				{
					/* Known blessed blades are accepted too */
					if (is_blessed(o_ptr) && object_known_p(o_ptr)) break;
				}

				default:
					return (FALSE);
			}
			break;
		}

		/* Alchemist */
		case STORE_ALCHEMY:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
				case TV_SCROLL:
				case TV_POTION:
					break;

				default:
					return (FALSE);
			}
			break;
		}

		/* Magic Shop */
		case STORE_MAGIC:
		{
			/* Analyze the type */
			switch (o_ptr->tval)
			{
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
	}

	/* Ignore "worthless" items XXX XXX XXX */
	if (object_value(o_ptr) <= 0) return (FALSE);

	/* Assume okay */
	return (TRUE);
}





/*** Basics: pricing, generation, etc. ***/

/*
 * Determine the price of an object (qty one) in a store.
 *
 * `flip` == TRUE  means the shop is buying, player selling
 *        == FALSE means the shop is selling, player buying
 *
 * This function takes into account the player's charisma, and the
 * shop-keepers friendliness, and the shop-keeper's base greed, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * object, and should be less than 100 when the player is "selling" it.
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
static s32b price_item(const object_type *o_ptr, bool store_buying)
{
	int factor;
	int adjust;
	s32b price;

	owner_type *ot_ptr = store_owner(store_current);

	/* The greed value is always of the current store's owner */
	int greed = ot_ptr->inflate;


	/* Get the value of one of the items */
	price = object_value(o_ptr);

	/* Worthless items */
	if (price <= 0) return (0L);


	/* Compute the racial factor */
	factor = g_info[(ot_ptr->owner_race * z_info->p_max) + p_ptr->prace];

	/* Add in the charisma factor */
	factor += adj_chr_gold[p_ptr->stat_ind[A_CHR]];


	/* Shop is buying */
	if (store_buying)
	{
		/* Adjust for greed */
		adjust = 100 + (300 - (greed + factor));

		/* Never get "silly" */
		if (adjust > 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (store_current == STORE_B_MARKET) price = price / 2;

		/* Now limit the price to the purse limit */
		if (price > ot_ptr->max_cost) price = ot_ptr->max_cost;
	}

	/* Shop is selling */
	else
	{
		/* Adjust for greed */
		adjust = 100 + ((greed + factor) - 300);

		/* Never get "silly" */
		if (adjust < 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (store_current == STORE_B_MARKET) price = price * 2;
	}

	/* Compute the final price (with rounding) */
	price = (price * adjust + 50L) / 100L;

	/* Note -- Never become "free" */
	if (price <= 0L) return (1L);

	/* Return the price */
	return (price);
}


/*
 * Special "mass production" computation.
 */
static int mass_roll(int times, int max)
{
	int i, t = 0;

	assert(max > 1);

	for (i = 0; i < max; i++)
		t += rand_int(max);

	return (t);
}


/*
 * Some cheap objects should be created in piles.
 */
static void mass_produce(object_type *o_ptr)
{
	int size = 1;
	s32b cost = object_value(o_ptr);

	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Food, Flasks, and Lites */
		case TV_FOOD:
		case TV_FLASK:
		case TV_LITE:
		{
			if (cost <= 5L) size += mass_roll(3, 5);
			if (cost <= 20L) size += mass_roll(3, 5);
			break;
		}

		case TV_POTION:
		case TV_SCROLL:
		{
			if (cost <= 60L) size += mass_roll(3, 5);
			if (cost <= 240L) size += mass_roll(1, 5);
			break;
		}

		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			if (cost <= 50L) size += mass_roll(2, 3);
			if (cost <= 500L) size += mass_roll(1, 3);
			break;
		}

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
		{
			if (o_ptr->name2) break;
			if (cost <= 10L) size += mass_roll(3, 5);
			if (cost <= 100L) size += mass_roll(3, 5);
			break;
		}

		case TV_SPIKE:
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			if (cost <= 5L)
				size = rand_die(3) * 20;         /* 20-60 in 20s */
			else if (cost > 5L && cost <= 50L)
				size = rand_die(4) * 10;         /* 10-40 in 10s */
			else if (cost > 50 && cost <= 500L)
				size = rand_die(4) * 5;          /* 5-20 in 5s */
			else
				size = 1;

			break;
		}
	}


	/* Save the total pile size */
	o_ptr->number = size;

	/* Hack -- rods need to increase PVAL if stacked */
	if (o_ptr->tval == TV_ROD)
	{
		o_ptr->pval = o_ptr->number * k_info[o_ptr->k_idx].pval;
	}
}



/*
 * Determine if a store object can "absorb" another object.
 *
 * See "object_similar()" for the same function for the "player".
 *
 * This function can ignore many of the checks done for the player,
 * since stores (but not the home) only get objects under certain
 * restricted circumstances.
 */
static bool store_object_similar(const object_type *o_ptr, const object_type *j_ptr)
{
	/* Hack -- Identical items cannot be stacked */
	if (o_ptr == j_ptr) return (0);

	/* Different objects cannot be stacked */
	if (o_ptr->k_idx != j_ptr->k_idx) return (0);

	/* Different pvals cannot be stacked, except for wands, staves, or rods */
	if ((o_ptr->pval != j_ptr->pval) &&
	    (o_ptr->tval != TV_WAND) &&
	    (o_ptr->tval != TV_STAFF) &&
	    (o_ptr->tval != TV_ROD)) return (0);

	/* Require many identical values */
	if (o_ptr->to_h != j_ptr->to_h) return (0);
	if (o_ptr->to_d != j_ptr->to_d) return (0);
	if (o_ptr->to_a != j_ptr->to_a) return (0);

	/* Require identical "artifact" names */
	if (o_ptr->name1 != j_ptr->name1) return (0);

	/* Require identical "ego-item" names */
	if (o_ptr->name2 != j_ptr->name2) return (0);

	/* Hack -- Never stack "powerful" items */
	if (o_ptr->xtra1 || j_ptr->xtra1) return (0);

	/* Hack -- Never stack recharging items */
	if ((o_ptr->timeout || j_ptr->timeout) && o_ptr->tval != TV_LITE)
		return (0);

	/* Never stack items with different fuel */
	else if ((o_ptr->timeout != j_ptr->timeout) && o_ptr->tval == TV_LITE)
		return (0);

	/* Require many identical values */
	if (o_ptr->ac != j_ptr->ac) return (0);
	if (o_ptr->dd != j_ptr->dd) return (0);
	if (o_ptr->ds != j_ptr->ds) return (0);

	/* Hack -- Never stack chests */
	if (o_ptr->tval == TV_CHEST) return (0);

	/* They match, so they must be similar */
	return (TRUE);
}


/*
 * Allow a store object to absorb another object
 */
static void store_object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Combine quantity, lose excess items */
	o_ptr->number = (total > 99) ? 99 : total;

	/*
	 * Hack -- if rods are stacking, add the pvals (maximum timeouts)
	 * and any charging timeouts together
	 */
	if (o_ptr->tval == TV_ROD)
	{
		o_ptr->pval += j_ptr->pval;
		o_ptr->timeout += j_ptr->timeout;
	}

	/* Hack -- if wands/staves are stacking, combine the charges */
	if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
	{
		o_ptr->pval += j_ptr->pval;
	}
}


/*
 * Check to see if the shop will be carrying too many objects
 *
 * Note that the shop, just like a player, will not accept things
 * it cannot hold.  Before, one could "nuke" objects this way, by
 * adding them to a pile which was already full.
 */
static bool store_check_num(int st, const object_type *o_ptr)
{
	int i;
	object_type *j_ptr;

	store_type *st_ptr = &store[st];

	/* Free space is always usable */
	if (st_ptr->stock_num < st_ptr->stock_size) return TRUE;

	/* The "home" acts like the player */
	if (st == STORE_HOME)
	{
		/* Check all the objects */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Get the existing object */
			j_ptr = &st_ptr->stock[i];

			/* Can the new object be combined with the old one? */
			if (object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* Normal stores do special stuff */
	else
	{
		/* Check all the objects */
		for (i = 0; i < st_ptr->stock_num; i++)
		{
			/* Get the existing object */
			j_ptr = &st_ptr->stock[i];

			/* Can the new object be combined with the old one? */
			if (store_object_similar(j_ptr, o_ptr)) return (TRUE);
		}
	}

	/* But there was no room at the inn... */
	return (FALSE);
}



/*
 * Add an object to the inventory of the Home.
 *
 * In all cases, return the slot (or -1) where the object was placed.
 *
 * Note that this is a hacked up version of "inven_carry()".
 *
 * Also note that it may not correctly "adapt" to "knowledge" becoming
 * known: the player may have to pick stuff up and drop it again.
 */
static int home_carry(object_type *o_ptr)
{
	int i, slot;
	u32b value, j_value;
	object_type *j_ptr;

	store_type *st_ptr = &store[STORE_HOME];

	/* Check each existing object (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing object */
		j_ptr = &st_ptr->stock[slot];

		/* The home acts just like the player */
		if (object_similar(j_ptr, o_ptr))
		{
			/* Save the new number of items */
			object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


	/* Determine the "value" of the object */
	value = object_value(o_ptr);

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get that object */
		j_ptr = &st_ptr->stock[slot];

		/* Hack -- readable books always come first */
		if ((o_ptr->tval == cp_ptr->spell_book) &&
		    (j_ptr->tval != cp_ptr->spell_book)) break;
		if ((j_ptr->tval == cp_ptr->spell_book) &&
		    (o_ptr->tval != cp_ptr->spell_book)) continue;

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Can happen in the home */
		if (!object_aware_p(o_ptr)) continue;
		if (!object_aware_p(j_ptr)) break;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Objects in the home can be unknown */
		if (!object_known_p(o_ptr)) continue;
		if (!object_known_p(j_ptr)) break;

		/* Objects sort by decreasing value */
		j_value = object_value(j_ptr);
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		/* Hack -- slide the objects */
		object_copy(&st_ptr->stock[i], &st_ptr->stock[i-1]);
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Hack -- Insert the new object */
	object_copy(&st_ptr->stock[slot], o_ptr);

	/* Return the location */
	return (slot);
}


/*
 * Add an object to a real stores inventory.
 *
 * If the object is "worthless", it is thrown away (except in the home).
 *
 * If the object cannot be combined with an object already in the inventory,
 * make a new slot for it, and calculate its "per item" price.  Note that
 * this price will be negative, since the price will not be "fixed" yet.
 * Adding an object to a "fixed" price stack will not change the fixed price.
 *
 * In all cases, return the slot (or -1) where the object was placed
 */
static int store_carry(int st, object_type *o_ptr)
{
	int i, slot;
	u32b value, j_value;
	object_type *j_ptr;

	store_type *st_ptr = &store[st];

	/* Evaluate the object */
	value = object_value(o_ptr);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0) return (-1);

	/* Erase the inscription & pseudo-ID bit */
	o_ptr->note = 0;
	o_ptr->pseudo = 0;

	/* Check each existing object (try to combine) */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get the existing object */
		j_ptr = &st_ptr->stock[slot];

		/* Can the existing items be incremented? */
		if (store_object_similar(j_ptr, o_ptr))
		{
			/* Absorb (some of) the object */
			store_object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (st_ptr->stock_num >= st_ptr->stock_size) return (-1);


	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < st_ptr->stock_num; slot++)
	{
		/* Get that object */
		j_ptr = &st_ptr->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Evaluate that slot */
		j_value = object_value(j_ptr);

		/* Objects sort by decreasing value */
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = st_ptr->stock_num; i > slot; i--)
	{
		/* Hack -- slide the objects */
		object_copy(&st_ptr->stock[i], &st_ptr->stock[i-1]);
	}

	/* More stuff now */
	st_ptr->stock_num++;

	/* Hack -- Insert the new object */
	object_copy(&st_ptr->stock[slot], o_ptr);

	/* Return the location */
	return (slot);
}


/*
 * Increase, by a 'num', the number of an item 'item' in store 'st'.
 * This can result in zero items.
 */
static void store_item_increase(int st, int item, int num)
{
	int cnt;
	object_type *o_ptr;

	store_type *st_ptr = &store[st];

	/* Get the object */
	o_ptr = &st_ptr->stock[item];

	/* Verify the number */
	cnt = o_ptr->number + num;
	if (cnt > 255) cnt = 255;
	else if (cnt < 0) cnt = 0;

	/* Save the new number */
	o_ptr->number = cnt;
}


/*
 * Remove a slot if it is empty, in store 'st'.
 */
static void store_item_optimize(int st, int item)
{
	int j;
	object_type *o_ptr;

	store_type *st_ptr = &store[st];

	/* Get the object */
	o_ptr = &st_ptr->stock[item];

	/* Must exist */
	if (!o_ptr->k_idx) return;

	/* Must have no items */
	if (o_ptr->number) return;

	/* One less object */
	st_ptr->stock_num--;

	/* Slide everyone */
	for (j = item; j < st_ptr->stock_num; j++)
	{
		st_ptr->stock[j] = st_ptr->stock[j + 1];
	}

	/* Nuke the final slot */
	object_wipe(&st_ptr->stock[j]);
}



/*
 * Delete a random object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 *
 * This function is used when store maintainance occurs, and is designed to
 * imitate non-PC purchasers making purchases from the store.
 */
static void store_delete_item(int st)
{
	int what, num;
	object_type *o_ptr;

	store_type *st_ptr = &store[st];

	/* Paranoia */
	if (st_ptr->stock_num <= 0) return;

	/* Pick a random slot */
	what = rand_int(st_ptr->stock_num);

	/* Get the object */
	o_ptr = &st_ptr->stock[what];

	/* Determine how many objects are in the slot */
	num = o_ptr->number;

	/* Deal with stacks */
	if (num > 1)
	{
		/* Special behaviour for arrows, bolts &tc. */
		switch (o_ptr->tval)
		{
			case TV_SPIKE:
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			{
				int cur_num = num;

				/* Sometimes take things to the nearest increment of 5 */
				if (rand_int(100) < 50) break;

				/* Keep things to increments of 5 */
				if (num % 5)
				{
					/* `num` is number of items to remove */
					num = num % 5;
				}
				else
				{
					/* Maybe decrement some more */
					if (rand_int(100) < 75) break;

					/* Decrement by a random factor of 5 */
					num = rand_die(cur_num) * 5;
				}

				break;
			}

			default:
			{
				/* Sometimes destroy a single object */
				if (rand_int(100) < 50) num = 1;

				/* Sometimes destroy half the objects */
				else if (rand_int(100) < 50) num = (num + 1) / 2;


				/* Hack -- decrement the maximum timeouts and total charges of rods and wands. */
				if ((o_ptr->tval == TV_ROD) ||
				    (o_ptr->tval == TV_STAFF) ||
				    (o_ptr->tval == TV_WAND))
				{
					o_ptr->pval -= num * o_ptr->pval / o_ptr->number;
				}
			}
		}

	}

	/* Delete the item */
	store_item_increase(st, what, -num);
	store_item_optimize(st, what);
}



/*
 * This makes sure that the black market doesn't stock any object that other
 * stores have, unless it is an ego-item or has various bonuses.
 *
 * Based on a suggestion by Lee Vogt <lvogt@cig.mcel.mot.com>.
 */
static bool black_market_ok(const object_type *o_ptr)
{
	int i, j;

	/* Ego items are always fine */
	if (ego_item_p(o_ptr)) return (TRUE);

	/* Good items are normally fine */
	if (o_ptr->to_a > 2) return (TRUE);
	if (o_ptr->to_h > 1) return (TRUE);
	if (o_ptr->to_d > 2) return (TRUE);


	/* No cheap items */
	if (object_value(o_ptr) < 10) return (FALSE);

	/* Check the other stores */
	for (i = 0; i < MAX_STORES; i++)
	{
		/* Skip home and black market */
		if (i == STORE_B_MARKET || i == STORE_HOME)
			continue;

		/* Check every object in the store */
		for (j = 0; j < store[i].stock_num; j++)
		{
			object_type *j_ptr = &store[i].stock[j];

			/* Compare object kinds */
			if (o_ptr->k_idx == j_ptr->k_idx)
				return (FALSE);
		}
	}

	/* Otherwise fine */
	return (TRUE);
}


/*
 * Helper function: Find a given tval,sval pair in the store 'num'.
 * Return first occurance of that item, or -1 if not found.
 */
static int store_find(int num, int tval, int sval)
{
	int i, k_idx;

	/* Find the kind */
	k_idx = lookup_kind(tval, sval);

	/* Validate */
	if (!k_idx)
	{
		msg_print("No object from store_find");
		return -1;
	}

	/* Check every object in the store */
	for (i = 0; i < store[num].stock_num; i++)
	{
		/* Compare object kinds */
		if (store[num].stock[i].k_idx == k_idx) return i;
	}

	/* Otherwise fine */
	return -1;
}


/*
 * Find tval, sval from k_idx.
 * XXX Should be in object2.c; is here for low impact.
 */
bool lookup_reverse(s16b k_idx, int *tval, int *sval)
{
	object_kind *k_ptr;

	/* Validate k_idx */
	if ((k_idx < 1) || (k_idx > z_info->k_max))
		return FALSE;

	/* Get pointer */
	k_ptr = &k_info[k_idx];
	*tval = k_ptr->tval;
	*sval = k_ptr->sval;

	/* Done */
	return TRUE;
}

/*
 * Get a choice from the store allocation table, in tables.c
 */
static s16b store_get_choice(int st)
{
	int r;
	store_type *st_ptr = &store[st];

	/* Choose a random entry from the store's table */
	r = rand_int(st_ptr->table_num);

	/* Return it */
	return st_ptr->table[r];
}


/*
 * Creates a random object and gives it to store 'st'
 */
static bool store_create_random(int st)
{
	int k_idx, tries, level, tval, sval;

	object_type *i_ptr;
	object_type object_type_body;

	int min_level, max_level;

	/*
	 * Decide min/max levels 
	 */
	if (st == STORE_B_MARKET)
	{
		min_level = 25;
		max_level = 50;
	}
	else
	{
		min_level = 1;
		max_level = STORE_OBJ_LEVEL;
	}


	/* Consider up to six items */
	for (tries = 0; tries < 6; tries++)
	{
		/* Work out the level for objects to be generated at */
		level = rand_range(min_level, max_level);


		/* Black Markets have a random object, of a given level */
		if (st == STORE_B_MARKET) k_idx = get_obj_num(level);

		/* Normal stores use a big table of choices */
		else k_idx = store_get_choice(st);


		/* Get tval/sval; if not found, item isn't real, so try again */
		if (!lookup_reverse(k_idx, &tval, &sval))
		{
			msg_print("Invalid object index in store_create_random()!");
			continue;
		}


		/*** Pre-generation filters ***/

		/* No chests in stores XXX */
		if (tval == TV_CHEST) continue;


		/*** Generate the item ***/

		/* Get local object */
		i_ptr = &object_type_body;

		/* Create a new object of the chosen kind */
		object_prep(i_ptr, k_idx);

		/* Apply some "low-level" magic (no artifacts) */
		apply_magic(i_ptr, level, FALSE, FALSE, FALSE);
	
		/* The object is "known" and belongs to a store */
		object_known(i_ptr);
		i_ptr->ident |= IDENT_STORE;


		/*** Post-generation filters ***/

		/* Black markets have expensive tastes */
		if ((st == STORE_B_MARKET) && !black_market_ok(i_ptr))
			continue;

		/* No "worthless" items */
		if (object_value(i_ptr) < 1) continue;



		/* Charge lights XXX */
		if (i_ptr->tval == TV_LITE)
		{
			if (i_ptr->sval == SV_LITE_TORCH) i_ptr->timeout = FUEL_TORCH / 2;
			if (i_ptr->sval == SV_LITE_LANTERN) i_ptr->timeout = FUEL_LAMP / 2;
		}

		/* Mass produce and/or apply discount */
		mass_produce(i_ptr);

		/* Attempt to carry the object */
		(void)store_carry(st, i_ptr);

		/* Definitely done */
		return TRUE;
	}

	return FALSE;
}

/*
 * Staple definitions.
 */
typedef enum { MAKE_SINGLE, MAKE_NORMAL, MAKE_MAX } create_mode;

static struct staple_type
{
	int tval, sval;
	create_mode mode;
} staples[] =
{
	{ TV_FOOD, SV_FOOD_RATION, MAKE_NORMAL },
	{ TV_LITE, SV_LITE_TORCH, MAKE_NORMAL },
	{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL, MAKE_NORMAL },
	{ TV_FOOD, SV_FOOD_BISCUIT, MAKE_NORMAL },
	{ TV_FOOD, SV_FOOD_JERKY, MAKE_NORMAL },
	{ TV_FOOD, SV_FOOD_PINT_OF_WINE, MAKE_NORMAL },
	{ TV_LITE, SV_LITE_TORCH, MAKE_NORMAL },
	{ TV_FLASK, 0, MAKE_NORMAL },
	{ TV_SPIKE, 0, MAKE_NORMAL },
	{ TV_SHOT, SV_AMMO_NORMAL, MAKE_MAX },
	{ TV_ARROW, SV_AMMO_NORMAL, MAKE_MAX },
	{ TV_BOLT, SV_AMMO_NORMAL, MAKE_MAX },
	{ TV_DIGGING, SV_SHOVEL, MAKE_SINGLE },
	{ TV_DIGGING, SV_PICK, MAKE_SINGLE },
	{ TV_CLOAK, SV_CLOAK, MAKE_SINGLE }
};


/*
 * Helper function: create an item with the given tval,sval pair, add it to the
 * store st.  Return the slot in the inventory.
 */
static int store_create_item(int st, int tval, int sval, create_mode mode)
{
	object_type object;
	int k_idx;

	/* Resolve tval,sval pair into an index */
	k_idx = lookup_kind(tval, sval);

	/* Validation - do something more substantial here? XXX */
	if (!k_idx)
	{
		msg_print("No object in store_create_item().");
		return -1;
	}

	/* Wipe this object */
	object_wipe(&object);

	/* Create a new object of the chosen kind */
	object_prep(&object, k_idx);

	/* The object is "known" */
	object_known(&object);

	/* Item belongs to a store */
	object.ident |= IDENT_STORE;

	/* Charge lights */
	if (object.tval == TV_LITE)
	{
		if (object.sval == SV_LITE_TORCH)        object.timeout = FUEL_TORCH / 2;
		else if (object.sval == SV_LITE_LANTERN) object.timeout = FUEL_LAMP / 2;
	}

	/* Make according to mode */
	switch (mode)
	{
		case MAKE_SINGLE:
			break;

		case MAKE_NORMAL:
			mass_produce(&object);
			break;

		case MAKE_MAX:
			object.number = 99;
			break;
	}

	/* Attempt to carry the object */
	return store_carry(st, &object);
}



/*
 * Create all staple items.
 *
 * XXX should ensure that entries marked as "max" stay in stock
 */
static void store_create_staples(void)
{
	unsigned i;

	/* Iterate through staples */
	for (i = 0; i < N_ELEMENTS(staples); i++)
	{
		struct staple_type *s = &staples[i];

		/* Look for the item, and if it isn't there, create it */
		if (store_find(STORE_GENERAL, s->tval, s->sval) == -1)
			store_create_item(STORE_GENERAL, s->tval, s->sval, s->mode);
	}
}



/*
 * Maintain the inventory at the stores.
 */
void store_maint(int which)
{
	int j;
	int stock;

	int old_rating = rating;

	store_type *st_ptr;


	/* Ignore home */
	if (which == STORE_HOME) return;

	/* General Store gets special treatment */
	if (which == STORE_GENERAL)
	{
		store_create_staples();
		return;
	}


	/* Activate that store */
	st_ptr = &store[which];


	/* XXX Prune the black market */
	if (which == STORE_B_MARKET)
	{
		/* Destroy crappy black market items */
		for (j = st_ptr->stock_num - 1; j >= 0; j--)
		{
			object_type *o_ptr = &st_ptr->stock[j];

			/* Destroy crappy items */
			if (!black_market_ok(o_ptr))
			{
				/* Destroy the object */
				store_item_increase(which, j, 0 - o_ptr->number);
				store_item_optimize(which, j);
			}
		}
	}

	/*** "Sell" various items */

	/* Sell a few items */
	stock = st_ptr->stock_num;
	stock -= randint(STORE_TURNOVER);

	/* Keep stock between STORE_MAX_KEEP and STORE_MIN_KEEP slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* Destroy objects until only "j" slots are left */
	while (st_ptr->stock_num > stock) store_delete_item(which);


	/*** "Buy in" various items */

	/* Buy a few items */
	stock = st_ptr->stock_num;
	stock += randint(STORE_TURNOVER);

	/* Keep stock between STORE_MAX_KEEP and STORE_MIN_KEEP slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* For the rest, we just choose items randomlyish */
	while (st_ptr->stock_num < stock) store_create_random(which);



	/* Hack -- Restore the rating */
	rating = old_rating;
}


/*
 * Initialize the stores.  Used at birth-time.
 */
void store_init(void)
{
	int n, i;

	store_type *st_ptr;

	/* Initialise all the stores */
	for (n = 0; n < MAX_STORES; n++)
	{
		/* Activate this store */
		st_ptr = &store[n];


		/* Pick an owner */
		st_ptr->owner = (byte)rand_int(z_info->b_max);

		/* Nothing in stock */
		st_ptr->stock_num = 0;

		/* Clear any old items */
		for (i = 0; i < st_ptr->stock_size; i++)
		{
			object_wipe(&st_ptr->stock[i]);
		}


		/* Ignore home */
		if (n == STORE_HOME) continue;

		/* Maintain the shop (ten times) */
		for (i = 0; i < 10; i++) store_maint(n);
	}
}


/*
 * Shuffle one of the stores.
 */
void store_shuffle(int which)
{
	int i;

	store_type *st_ptr = &store[which];

	/* Ignore home */
	if (which == STORE_HOME) return;


	/* Pick a new owner */
	i = st_ptr->owner;

	while (i == st_ptr->owner)
	    i = rand_int(z_info->b_max);

	st_ptr->owner = i;
}



/*** Display code ***/


/*
 * This function sets up screen locations based on the current term size.
 *
 * Current screen layout:
 *  line 0: reserved for messages
 *  line 1: shopkeeper and their purse / item buying price
 *  line 2: empty
 *  line 3: table headers
 *
 *  line 4: Start of items
 *
 * If help is turned off, then the rest of the display goes as:
 *
 *  line (height - 4): end of items
 *  line (height - 3): "more" prompt
 *  line (height - 2): empty
 *  line (height - 1): Help prompt and remaining gold
 *
 * If help is turned on, then the rest of the display goes as:
 *
 *  line (height - 7): end of items
 *  line (height - 6): "more" prompt
 *  line (height - 4): gold remaining
 *  line (height - 3): command help 
 */
static void store_display_recalc(void)
{
	int wid, hgt;
	Term_get_size(&wid, &hgt);

	/* Clip the width at a maximum of 104 (enough room for an 80-char item name) */
	if (wid > 104) wid = 104;

	/* Clip the text_out function at two smaller than the screen width */
	text_out_wrap = wid - 2;


	/* X co-ords first */
	scr_places_x[LOC_PRICE] = wid - 14;
	scr_places_x[LOC_AU] = wid - 26;
	scr_places_x[LOC_OWNER] = wid - 2;
	scr_places_x[LOC_WEIGHT] = wid - 14;

	/* Add space for for prices */
	if (store_current != STORE_HOME)
		scr_places_x[LOC_WEIGHT] -= 10;

	/* Then Y */
	scr_places_y[LOC_OWNER] = 1;
	scr_places_y[LOC_HEADER] = 3;
	scr_places_y[LOC_ITEMS_START] = 4;

	/* If we are displaying help, make the height smaller */
	if (store_flags & (STORE_SHOW_HELP))
		hgt -= 3;

	scr_places_y[LOC_ITEMS_END] = hgt - 4;
	scr_places_y[LOC_MORE] = hgt - 3;
	scr_places_y[LOC_AU] = hgt - 2;



	/* If we're displaying the help, then put it with a line of padding */
	if (!(store_flags & (STORE_SHOW_HELP)))
	{
		hgt -= 2;
	}

	scr_places_y[LOC_HELP_CLEAR] = hgt - 1;
	scr_places_y[LOC_HELP_PROMPT] = hgt;
}

/*
 * Convert a store item index into a one character label
 */
static s16b store_to_label(int i)
{
	/* Assume legal */
	return (I2A(i));
}



/*
 * Redisplay a single store entry
 */
static void store_display_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	object_type *o_ptr;
	s32b x;

	char o_name[80];
	char out_val[160];
	byte colour;

	store_type *st_ptr = &store[store_current];

	(void)menu;
	(void)cursor;
	(void)width;

	/* Get the object */
	o_ptr = &st_ptr->stock[oid];

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, 4);

	/* Display the object */
	c_put_str(tval_to_attr[o_ptr->tval & 0x7F], o_name, row, col);

	/* Show weights */
	colour = curs_attrs[CURS_KNOWN][(int)cursor];
	strnfmt(out_val, sizeof out_val, "%3d.%d lb", o_ptr->weight / 10, o_ptr->weight % 10);
	c_put_str(colour, out_val, row, scr_places_x[LOC_WEIGHT]);

	/* Describe an object (fully) in a store */
	if (store_current != STORE_HOME)
	{
		/* Extract the "minimum" price */
		x = price_item(o_ptr, FALSE);

		/* Make sure the player can afford it */
		if ((int) p_ptr->au < (int) x)
			colour = curs_attrs[CURS_UNKNOWN][(int)cursor];

		/* Actually draw the price */
		if (((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF)) &&
		    (o_ptr->number > 1))
			strnfmt(out_val, sizeof out_val, "%9ld avg", (long)x);
		else
			strnfmt(out_val, sizeof out_val, "%9ld    ", (long)x);

		c_put_str(colour, out_val, row, scr_places_x[LOC_PRICE]);
	}
}


/*
 * Display store (after clearing screen)
 */
static void store_display_frame(void)
{
	char buf[80];

	owner_type *ot_ptr = store_owner(store_current);

	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (store_current == STORE_HOME)
	{
		/* Put the owner name */
		put_str("Your Home", scr_places_y[LOC_OWNER], 1);

		/* Label the object descriptions */
		put_str("Home Inventory", scr_places_y[LOC_HEADER], 1);

		/* Show weight header */
		put_str("Weight", 5, scr_places_x[LOC_WEIGHT] + 2);
	}

	/* Normal stores */
	else
	{
		cptr store_name = (f_name + f_info[FEAT_SHOP_HEAD + store_current].name);
		cptr owner_name = &(b_name[ot_ptr->owner_name]);
		cptr race_name = p_name + p_info[ot_ptr->owner_race].name;

		/* Put the owner name and race */
		strnfmt(buf, sizeof(buf), "%s (%s)", owner_name, race_name);
		put_str(buf, scr_places_y[LOC_OWNER], 1);

		/* Show the max price in the store (above prices) */
		strnfmt(buf, sizeof(buf), "%s (%ld)", store_name, (long)(ot_ptr->max_cost));
		prt(buf, scr_places_y[LOC_OWNER], scr_places_x[LOC_OWNER] - strlen(buf));

		/* Label the object descriptions */
		put_str("Store Inventory", scr_places_y[LOC_HEADER], 1);

		/* Showing weight label */
		put_str("Weight", scr_places_y[LOC_HEADER], scr_places_x[LOC_WEIGHT] + 2);

		/* Label the asking price (in stores) */
		put_str("Price", scr_places_y[LOC_HEADER], scr_places_x[LOC_PRICE] + 4);
	}
}


/*
 * Display help.
 */
static void store_display_help(void)
{
	int help_loc = scr_places_y[LOC_HELP_PROMPT];

	/* Clear */
	clear_from(scr_places_y[LOC_HELP_CLEAR]);

	/* Prepare help hooks */
	text_out_hook = text_out_to_screen;
	text_out_indent = 1;
	Term_gotoxy(1, help_loc);

	text_out("Use the ");
	text_out_c(TERM_L_GREEN, "movement keys");
	text_out(" to navigate, or ");
	text_out_c(TERM_L_GREEN, "Space");
	text_out(" to advance to the next page. '");

	if (rogue_like_commands)
		text_out_c(TERM_L_GREEN, "x");
	else
		text_out_c(TERM_L_GREEN, "l");

	text_out("' examines and ");
	text_out_c(TERM_L_GREEN, "Enter");

	if (store_current == STORE_HOME) text_out(" picks up");
	else text_out(" purchases");

	text_out(" the selected item. '");
	
	text_out_c(TERM_L_GREEN, "d");
	if (store_current == STORE_HOME) text_out("' drops");
	else text_out("' sells");

	text_out(" an item from your inventory. ");

	text_out_c(TERM_L_GREEN, "ESC");
	text_out(" exits the building.");

	text_out_indent = 0;
}

/*
 * Decides what parts of the store display to redraw.  Called on terminal
 * resizings and the redraw command.
 */
static void store_redraw(void)
{
	if (store_flags & (STORE_FRAME_CHANGE))
	{
		store_display_frame();

		if (store_flags & STORE_SHOW_HELP)
			store_display_help();
		else
			prt("Press '?' for help.", scr_places_y[LOC_HELP_PROMPT], 1);

		store_flags &= ~(STORE_FRAME_CHANGE);
	}

	if (store_flags & (STORE_GOLD_CHANGE))
	{
		prt(format("Gold Remaining: %9ld", (long)p_ptr->au),
		    scr_places_y[LOC_AU], scr_places_x[LOC_AU]);
		store_flags &= ~(STORE_GOLD_CHANGE);
	}
}


/*** Higher-level code ***/


static bool store_get_check(const char *prompt)
{
	char ch;

	/* Prompt for it */
	prt(prompt, 0, 0);

	/* Get an answer */
	ch = inkey();

	/* Erase the prompt */
	prt("", 0, 0);

	if (ch == ESCAPE) return (FALSE);
	if (strchr("Nn", ch)) return (FALSE);

	/* Success */
	return (TRUE);
}



/*
 * Buy an object from a store
 */
static bool store_purchase(int item)
{
	int amt, item_new;

	store_type *st_ptr = &store[store_current];

	object_type *o_ptr;

	object_type *i_ptr;
	object_type object_type_body;

	char o_name[80];

	s32b price;

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];
	if(item < 0) return FALSE;

	/* Clear all current messages */
	msg_flag = FALSE;
	prt("", 0, 0);

	if (store_current == STORE_HOME)
	{
		amt = o_ptr->number;
	}
	else
	{
		/* Price of one */
		price = price_item(o_ptr, FALSE);

		/* Check if the player can afford any at all */
		if ((u32b)p_ptr->au < (u32b)price)
		{
			/* Tell the user */
			msg_print("You do not have enough gold for this item.");
			store_flags |= STORE_KEEP_PROMPT;

			/* Abort now */
			return FALSE;
		}

		/* Work out how many the player can afford */
		amt = p_ptr->au / price;
		if (amt > o_ptr->number) amt = o_ptr->number;
	}

	/* Get a quantity */
	amt = get_quantity(NULL, amt);

	/* Allow user abort */
	if (amt <= 0) return FALSE;

	/* Get local object */
	i_ptr = &object_type_body;

	/* Get desired object */
	object_copy(i_ptr, o_ptr);

	/*
	 * XXX Stacking
	 * If a rod or wand, allocate total maximum timeouts or charges
	 * between those purchased and left on the shelf.
	 */
	reduce_charges(i_ptr, i_ptr->number - amt);

	/* Modify quantity */
	i_ptr->number = amt;

	/* Ensure we have room */
	if (!inven_carry_okay(i_ptr))
	{
		msg_print("You cannot carry that many items.");
		store_flags |= STORE_KEEP_PROMPT;
		return FALSE;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 3);

	/* Attempt to buy it */
	if (store_current != STORE_HOME)
	{
		u32b price;
		bool response;

		/* Extract the price for the entire stack */
		price = price_item(i_ptr, FALSE) * i_ptr->number;

		screen_save();

		/* Show price */
		prt(format("Price: %d", price), 1, 0);

		/* Confirm purchase */
		response = store_get_check(format("Buy %s? [ESC, any other key to accept]", o_name));
		screen_load();

		/* Negative response, so give up */
		if (!response) return FALSE;


		/* Spend the money */
		p_ptr->au -= price;

		/* Update the display */
		store_flags |= STORE_GOLD_CHANGE;

		/* Buying an object makes you aware of it */
		object_aware(i_ptr);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* The object no longer belongs to the store */
		i_ptr->ident &= ~(IDENT_STORE);

		/* Message */
		if (!rand_int(3)) message(MSG_STORE5, 0, ONE_OF(comment_accept));
		msg_format("You bought %s (%c) for %ld gold.", o_name,
		           store_to_label(item), (long)price);

		/* Erase the inscription */
		i_ptr->note = 0;

		/* Give it to the player */
		item_new = inven_carry(i_ptr);

		/* Message */
		msg_format("You have %s (%c).", o_name, index_to_label(item_new));
		store_flags |= STORE_KEEP_PROMPT;

		/* Now, reduce the original stack's pval */
		if ((o_ptr->tval == TV_ROD) ||
		    (o_ptr->tval == TV_WAND) ||
		    (o_ptr->tval == TV_STAFF))
		{
			o_ptr->pval -= i_ptr->pval;
		}

		/* Handle stuff */
		handle_stuff();

		/* Remove the bought objects from the store */
		store_item_increase(store_current, item, -amt);
		store_item_optimize(store_current, item);

		/* Store is empty */
		if (st_ptr->stock_num == 0)
		{
			int i;

			/* Shuffle */
			if (rand_int(STORE_SHUFFLE) == 0)
			{
				/* Message */
				msg_print("The shopkeeper retires.");

				/* Shuffle the store */
				store_shuffle(store_current);
			}

			/* Maintain */
			else
			{
				/* Message */
				msg_print("The shopkeeper brings out some new stock.");
			}

			/* New inventory */
			for (i = 0; i < 10; ++i)
			{
				/* Maintain the store */
				store_maint(store_current);
			}
		}
	}

	/* Home is much easier */
	else
	{
		/* Distribute charges of wands, staves, or rods */
		distribute_charges(o_ptr, i_ptr, amt);

		/* Give it to the player */
		item_new = inven_carry(i_ptr);

		/* Describe just the result */
		object_desc(o_name, sizeof(o_name), &inventory[item_new], TRUE, 3);

		/* Message */
		msg_format("You have %s (%c).", o_name, index_to_label(item_new));
		store_flags |= STORE_KEEP_PROMPT;

		/* Handle stuff */
		handle_stuff();

		/* Remove the items from the home */
		store_item_increase(store_current, item, -amt);
		store_item_optimize(store_current, item);
	}

	/* Not kicked out */
	return TRUE;
}



/*
 * Determine if the current store will purchase the given object
 */
static bool store_will_buy_tester(const object_type *o_ptr)
{
	return store_will_buy(store_current, o_ptr);
}


/*
 * Sell an object, or drop if it we're in the home.
 */
static void store_sell(void)
{
	int amt;
	int item;

	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	char o_name[120];


	const char *reject = "You have nothing that I want. ";
	const char *prompt = "Sell which item? ";

	/* Clear all current messages */
	msg_flag = FALSE;
	prt("", 0, 0);

	if (store_current == STORE_HOME)
		prompt = "Drop which item? ";
	else
		item_tester_hook = store_will_buy_tester;

	/* Get an item */
	if (!get_item(&item, prompt, reject, (USE_EQUIP | USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
		o_ptr = &inventory[item];
 	else
		o_ptr = &o_list[0 - item];

	/* Hack -- Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		/* Oops */
		msg_print("Hmmm, it seems to be cursed.");
		store_flags |= STORE_KEEP_PROMPT;

		/* Nope */
		return;
	}

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);

	/* Allow user abort */
	if (amt <= 0) return;


	/* Get local object */
	i_ptr = &object_type_body;

	/* Get a copy of the object */
	object_copy(i_ptr, o_ptr);

	/* Modify quantity */
	i_ptr->number = amt;


	/*
	 * XXX Stacking
	 * If a rod, wand, or staff, allocate total maximum timeouts or charges
	 * to those being sold.
	 */
	if ((o_ptr->tval == TV_ROD) ||
	    (o_ptr->tval == TV_WAND) ||
	    (o_ptr->tval == TV_STAFF))
	{
		i_ptr->pval = o_ptr->pval * amt / o_ptr->number;
	}

	/* Get a full description */
	object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 3);


	/* Is there room in the store (or the home?) */
	if (!store_check_num(store_current, i_ptr))
	{
		store_flags |= STORE_KEEP_PROMPT;

		if (store_current == STORE_HOME)
			msg_print("Your home is full.");

		else
			msg_print("I have not the room in my store to keep it.");

		return;
	}


	/* Real store */
	if (store_current != STORE_HOME)
	{
		u32b price, dummy, value;

		/* Extract the value of the items */
		price = price_item(i_ptr, TRUE) * amt;

		screen_save();

		/* Show price */
		prt(format("Price: %d", price), 1, 0);

		/* Confirm sale */
		if (!store_get_check(format("Sell %s? [ESC, any other key to accept]", o_name)))
		{
			screen_load();
			return;
		}

		screen_load();

		/* Get some money */
		p_ptr->au += price;

		/* Update the display */
		store_flags |= STORE_GOLD_CHANGE;

		/* Identify original object */
		object_aware(o_ptr);
		object_known(o_ptr);

		/* Combine / Reorder the pack (later) */
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER_0 | PW_PLAYER_1);
		p_ptr->redraw |= (PR_EQUIPPY);

		/* The object belongs to the store now */
		i_ptr->ident |= IDENT_STORE;

		/* Get the "apparent" value */
		dummy = object_value(i_ptr) * i_ptr->number;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Get a copy of the object */
		object_copy(i_ptr, o_ptr);

		/* Modify quantity */
		i_ptr->number = amt;

		/*
		 * Hack -- Allocate charges between those wands, staves, or rods
		 * sold and retained, unless all are being sold.
		 */
		distribute_charges(o_ptr, i_ptr, amt);

		/* Get the "actual" value */
		value = object_value(i_ptr) * i_ptr->number;

		/* Get the description all over again */
		object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 3);

		/* Describe the result (in message buffer) */
		msg_format("You sold %s (%c) for %ld gold.",
		           o_name, index_to_label(item), (long)price);
		store_flags |= STORE_KEEP_PROMPT;

		/* Analyze the prices (and comment verbally) */
		purchase_analyze(price, value, dummy);

		/* Set squelch flag */
		p_ptr->notice = PN_SQUELCH;

		/* Take the object from the player */
		inven_item_increase(item, -amt);
		inven_item_optimize(item);

		/* Handle stuff */
		handle_stuff();

		/* The store gets that (known) object */
		store_carry(store_current, i_ptr);
	}

	/* Player is at home */
	else
	{
		/* Distribute charges of wands/staves/rods */
		distribute_charges(o_ptr, i_ptr, amt);

		/* Describe */
		msg_format("You drop %s (%c).", o_name, index_to_label(item));
		store_flags |= STORE_KEEP_PROMPT;

		/* Take it from the players inventory */
		inven_item_increase(item, -amt);
		inven_item_optimize(item);

		/* Handle stuff */
		handle_stuff();

		/* Let the home carry it */
		home_carry(i_ptr);
	}
}


/*
 * Examine an item in a store
 */
static void store_examine(int item)
{
	store_type *st_ptr = &store[store_current];
	object_type *o_ptr;

	if(item < 0) return;

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];

	/* Describe it fully */
	Term_erase(0, 0, 255);
	Term_gotoxy(0, 0);
	object_info_screen(o_ptr);
}


/*
 * Flee the store when it overflows.
 */
bool store_overflow(void)
{
	int item = INVEN_PACK;

	object_type *o_ptr = &inventory[item];

	/* Flee from the store */
	if (store_current != STORE_HOME)
	{
		/* Leave */
		msg_print("Your pack is so full that you flee the store...");
		return TRUE;
	}

	/* Flee from the home */
	else if (!store_check_num(store_current, o_ptr))
	{
		/* Leave */
		msg_print("Your pack is so full that you flee your home...");
		return TRUE;
	}

	/* Drop items into the home */
	else
	{
		object_type *i_ptr;
		object_type object_type_body;

		char o_name[80];


		/* Give a message */
		msg_print("Your pack overflows!");
		store_flags |= STORE_KEEP_PROMPT;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Grab a copy of the object */
		object_copy(i_ptr, o_ptr);

		/* Describe it */
		object_desc(o_name, sizeof(o_name), i_ptr, TRUE, 3);

		/* Message */
		msg_format("You drop %s (%c).", o_name, index_to_label(item));

		/* Remove it from the players inventory */
		inven_item_increase(item, -255);
		inven_item_describe(item);
		inven_item_optimize(item);

		/* Handle stuff */
		handle_stuff();

		/* Let the home carry it */
		home_carry(i_ptr);
	}

	return FALSE;
}


/*
 * Process a command in a store
 *
 * Note that we must allow the use of a few "special" commands in the stores
 * which are not allowed in the dungeon, and we must disable some commands
 * which are allowed in the dungeon but not in the stores, to prevent chaos.
 */
static bool store_process_command(char cmd, void *db, int oid)
{
	bool equip_toggle = FALSE;

	/* Parse the command */
	switch (cmd)
	{
		/* Leave */
		case ESCAPE:
		{
			return TRUE;
			break;
		}

		/* Sell */
		case 's':
		case 'd':
		{
			store_sell();
			return TRUE;
		}

		/* Buy */
		case '\xff':
		case '\n':
		case '\r':
		{
			/* On successful purchase, redraw */
			if (store_purchase(oid))
				return TRUE;

			break;
		}

		/* Examine */
		case 'l':
		case 'x':
		{
			store_examine(oid);
			break;
		}


		/* Redraw */
		case KTRL('R'):
		{
			Term_clear();
			store_flags |= (STORE_FRAME_CHANGE | STORE_GOLD_CHANGE);
			return TRUE;

			break;
		}



		/*** Inventory Commands ***/

		/* Wear/wield equipment */
		case 'w':
		{
			do_cmd_wield();
			break;
		}

		/* Take off equipment */
		case 'T':
		case 't':
		{
			do_cmd_takeoff();
			break;
		}

		/* Destroy an item */
		case KTRL('D'):
		case 'k':
		{
			do_cmd_destroy();
			break;
		}

		/* Equipment list */
		case 'e':
		{
			equip_toggle = TRUE;
		}

		/* Inventory list */
		case 'i':
		{
			/* Display the right thing until the user escapes */
			do
			{
				if (equip_toggle) do_cmd_equip();
				else do_cmd_inven();

				/* Toggle the toggle */
				equip_toggle = !equip_toggle;

			} while (p_ptr->command_new == '/' || p_ptr->command_new == 'e' ||
			         p_ptr->command_new == 'i');

			/* Legal inventory commands are drop, inspect */
			if (!strchr("dsI", p_ptr->command_new))
				p_ptr->command_new = 0;

			break;
		}


		/*** Various commands ***/

		/* Identify an object */
		case 'I':
		{
			do_cmd_observe();
			break;
		}

		/* Hack -- toggle windows */
		case KTRL('E'):
		{
			toggle_inven_equip();
			break;
		}



		/*** Use various objects ***/

		/* Browse a book */
		case 'P':
		case 'b':
		{
			do_cmd_browse();
			break;
		}

		/* Inscribe an object */
		case '{':
		{
			do_cmd_inscribe();
			break;
		}

		/* Uninscribe an object */
		case '}':
		{
			do_cmd_uninscribe();
			break;
		}


		/*** Help and Such ***/

		/* Character description */
		case 'C':
		{
			do_cmd_change_name();
			break;
		}

		case '?':
		{
			/* Toggle help */
			if (store_flags & STORE_SHOW_HELP)
				store_flags &= ~(STORE_SHOW_HELP);
			else
				store_flags |= STORE_SHOW_HELP;

			/* Redisplay */
			store_flags |= STORE_INIT_CHANGE;

			return TRUE;
		}

		/*** System Commands ***/

		/* Interact with options */
		case '=':
		{
			do_cmd_options();
			break;
		}


		/*** Misc Commands ***/

		/* Show previous messages */
		case KTRL('P'):
		{
			do_cmd_messages();
			break;
		}

		/* Check knowledge */
		case '~':
		case '|':
		{
			do_cmd_knowledge();
			break;
		}

		/* Save "screen dump" */
		case ')':
		{
			do_cmd_save_screen();
			break;
		}
	}

	return TRUE;
}



/*
 * Enter a store, and interact with it.
 */
void do_cmd_store(void)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	bool leave = FALSE;

	/* Verify that there is a store */
	if (!((cave_feat[py][px] >= FEAT_SHOP_HEAD) &&
	      (cave_feat[py][px] <= FEAT_SHOP_TAIL)))
	{
		msg_print("You see no store here.");
		return;
	}


	/* Check if we can enter the store */
	if (adult_no_stores)
	{
		msg_print("The doors are locked.");
		return;
	}


	/* Forget the view */
	forget_view();

	/* Reset the command variables */
	p_ptr->command_arg = 0;
	p_ptr->command_rep = 0;
	p_ptr->command_new = 0;


	/*** Set up state ***/

	/* XXX Take note of the store number from the terrain feature */
	store_current = (cave_feat[py][px] - FEAT_SHOP_HEAD);



	/*** Display ***/

	/* Save current screen (ie. dungeon) */
	screen_save();

	/*** Inventory display ***/
	{

	static region items_region = { 1, 4, -1, -1 };
	static const menu_iter store_menu = { 0, 0, 0, store_display_entry, store_process_command };
	const menu_iter *cur_menu = &store_menu;

	menu_type menu;
	event_type evt;
	int cursor = 0;

	store_type *st_ptr = &store[store_current];

	/* Wipe the menu and set it up */
	WIPE(&menu, menu);
	menu.flags = MN_DBL_TAP | MN_PAGE;

	/* Calculate the positions of things and redraw */
	store_flags = STORE_INIT_CHANGE;
	store_display_recalc();
	store_redraw();

	/* Say a friendly hello. */
	if (store_current != STORE_HOME) prt_welcome(store_owner(store_current));

	/* Loop */
	while (!leave)
	{
		/* As many rows in the menus as there are items in the store */
		menu.count = st_ptr->stock_num;

		/* Roguelike */
		if (rogue_like_commands)
		{
			/* These two can't intersect! */
			menu.cmd_keys = "\n\x04\x10\r?=CPdeEiIsTwx\x8B\x8Chl"; /* \x10 = ^p , \x04 = ^D */
			menu.selections = "abcfghmnopqruvyz1234567890";
		}

		/* Original */
		else
		{
			/* These two can't intersect! */
			menu.cmd_keys = "\n\x010\r?=CbdeEiIklstw\x8B\x8C"; /* \x10 = ^p */
			menu.selections = "acfghmnopqruvxyz13456790";
		}

		/* Keep the cursor in range of the stock */
		if (cursor < 0 || cursor >= menu.count)
			cursor = menu.count - 1;

		items_region.page_rows = scr_places_y[LOC_MORE] - scr_places_y[LOC_ITEMS_START];

		/* Init the menu structure */
		menu_init2(&menu, find_menu_skin(MN_SCROLL), cur_menu, &items_region);

		if (menu.count > items_region.page_rows)
			menu.prompt = "  -more-";
		else
			menu.prompt = NULL;

		menu_layout(&menu, &menu.boundary);

		evt.type = EVT_MOVE;
		/* Get a selection/action */
		while (evt.type == EVT_MOVE)
		{
			evt = menu_select(&menu, &cursor, EVT_MOVE);
			if (store_flags & STORE_KEEP_PROMPT)
			{
				store_flags &= ~STORE_KEEP_PROMPT;
			}
			else
			{
				/* Clear the prompt, and mark it as read (i.e. no -more-
				   prompt will be issued when messages are flushed. */
				prt("", 0, 0);
				msg_flag = FALSE;
			}
		}

		if (evt.key == ESCAPE || evt.type == EVT_BACK)
		{
			leave = TRUE;
		}
		else if (evt.type == EVT_RESIZE)
		{
			/* Resize event */
			store_display_recalc();
			store_redraw();
		}
		else
		{
			/* Display the store */
			store_display_recalc();
			store_redraw();

			/* Notice and handle stuff */
			notice_stuff();
			handle_stuff();

			/* XXX Pack Overflow */
			if (inventory[INVEN_PACK].k_idx)
				leave = store_overflow();
		}
	}

	}

	/* Take a turn */
	p_ptr->energy_use = 100;


	/* Hack -- Cancel automatic command */
	p_ptr->command_new = 0;

	/* Hack -- Cancel "see" mode */
	p_ptr->command_see = FALSE;


	/* Flush messages XXX XXX XXX */
	message_flush();


	/* Load the screen */
	screen_load();


	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw entire screen */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}
