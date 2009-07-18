/*
 * File: store.c
 * Purpose: Store stocking and UI
 *
 * Copyright (c) 1997 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 2007 Andrew Sidwell, who rewrote a fair portion
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "cmds.h"
#include "ui-menu.h"
#include "game-event.h"
#include "object/tvalsval.h"


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

/* Flags for the display */
static u16b store_flags;




/*** Utilities ***/

/*
 * Return the owner struct for the given store.
 */
static owner_type *store_owner(int st)
{
	store_type *st_ptr = &store[st];
	return &b_info[(st * z_info->b_max) + st_ptr->owner];
}


/* Randomly select one of the entries in an array */
#define ONE_OF(x)	x[randint0(N_ELEMENTS(x))]



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
	"The shopkeeper glares at you."
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
	/* We go from level 1 - 50  */
	size_t i = ((unsigned)p_ptr->lev - 1) / 5;

	/* Sanity check in case we increase the max level carelessly */
	i = MIN(i, N_ELEMENTS(comment_welcome) - 1);

	/* Only show the message one in four times to stop it being irritating. */
	if (one_in_(4)) return;

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
		if ((i % 2) && randint0(2)) player_name = c_text + cp_ptr->title[(p_ptr->lev - 1) / 5];
		else if (randint0(2))       player_name = op_ptr->full_name;
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
			/* Doesn't buy anything back */
			return (FALSE);
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
	if (object_value(o_ptr, 1, FALSE) <= 0) return (FALSE);

	/* Assume okay */
	return (TRUE);
}


#define STORE_NONE -1

/* Get the current store number, or STORE_NONE if not in a store */
static int current_store()
{
	if ((cave_feat[p_ptr->py][p_ptr->px] >= FEAT_SHOP_HEAD) &&
		(cave_feat[p_ptr->py][p_ptr->px] <= FEAT_SHOP_TAIL))
		return (cave_feat[p_ptr->py][p_ptr->px] - FEAT_SHOP_HEAD);

	return STORE_NONE;
}



/*** Basics: pricing, generation, etc. ***/

/*
 * Determine the price of an object (qty one) in a store.
 *
 *  store_buying == TRUE  means the shop is buying, player selling
 *               == FALSE means the shop is selling, player buying
 *
 * This function takes into account the player's charisma, but
 * never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * object, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 */
s32b price_item(const object_type *o_ptr, bool store_buying, int qty)
{
	int adjust;
	s32b price;
	int this_store = current_store();
	owner_type *ot_ptr;

	if (this_store == STORE_NONE) return 0L;

	ot_ptr = store_owner(this_store);


	/* Get the value of the stack of wands, or a single item */
	if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
		price = object_value(o_ptr, qty, TRUE);
	else
		price = object_value(o_ptr, 1, TRUE);

	/* Worthless items */
	if (price <= 0) return (0L);


	/* Add in the charisma factor */
	if (this_store == STORE_B_MARKET)
		adjust = 150;
	else
		adjust = adj_chr_gold[p_ptr->state.stat_ind[A_CHR]];


	/* Shop is buying */
	if (store_buying)
	{
		/* Set the factor */
		adjust = 100 + (100 - adjust);
		if (adjust > 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (this_store == STORE_B_MARKET) price = price / 2;
	}

	/* Shop is selling */
	else
	{
		/* Fix the factor */
		if (adjust < 100) adjust = 100;

		/* Mega-Hack -- Black market sucks */
		if (this_store == STORE_B_MARKET) price = price * 2;
	}

	/* Compute the final price (with rounding) */
	price = (price * adjust + 50L) / 100L;

	/* Now convert price to total price for non-wands */
	if (!(o_ptr->tval == TV_WAND) && !(o_ptr->tval == TV_STAFF))
		price *= qty;

	/* Now limit the price to the purse limit */
	if (store_buying && (price > ot_ptr->max_cost * qty))
		price = ot_ptr->max_cost * qty;

	/* Note -- Never become "free" */
	if (price <= 0L) return (qty);

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

	for (i = 0; i < times; i++)
		t += randint0(max);

	return (t);
}


/*
 * Some cheap objects should be created in piles.
 */
static void mass_produce(object_type *o_ptr)
{
	int size = 1;
	s32b cost = object_value(o_ptr, 1, TRUE);

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
				size = randint1(3) * 20;         /* 20-60 in 20s */
			else if (cost > 5L && cost <= 50L)
				size = randint1(4) * 10;         /* 10-40 in 10s */
			else if (cost > 50 && cost <= 500L)
				size = randint1(4) * 5;          /* 5-20 in 5s */
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

	/* Different flags */
	if (o_ptr->flags[0] != j_ptr->flags[0] ||
		o_ptr->flags[1] != j_ptr->flags[1] ||
		o_ptr->flags[2] != j_ptr->flags[2]) return FALSE;

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

	if ((o_ptr->origin != j_ptr->origin) ||
	    (o_ptr->origin_depth != j_ptr->origin_depth) ||
	    (o_ptr->origin_xtra != j_ptr->origin_xtra))
	{
		int act = 2;

		if ((o_ptr->origin == ORIGIN_DROP) && (o_ptr->origin == j_ptr->origin))
		{
			monster_race *r_ptr = &r_info[o_ptr->origin_xtra];
			monster_race *s_ptr = &r_info[j_ptr->origin_xtra];

			bool r_uniq = (r_ptr->flags[0] & RF0_UNIQUE) ? TRUE : FALSE;
			bool s_uniq = (s_ptr->flags[0] & RF0_UNIQUE) ? TRUE : FALSE;

			if (r_uniq && !s_uniq) act = 0;
			else if (s_uniq && !r_uniq) act = 1;
			else act = 2;
		}

		switch (act)
		{
			/* Overwrite with j_ptr */
			case 1:
			{
				o_ptr->origin = j_ptr->origin;
				o_ptr->origin_depth = j_ptr->origin_depth;
				o_ptr->origin_xtra = j_ptr->origin_xtra;
			}

			/* Set as "mixed" */
			case 2:
			{
				o_ptr->origin = ORIGIN_MIXED;
			}
		}
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
	value = object_value(o_ptr, 1, TRUE);

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
		j_value = object_value(j_ptr, 1, TRUE);
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
	value = object_value(o_ptr, 1, TRUE);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0) return (-1);

	/* Erase the inscription & pseudo-ID bit */
	o_ptr->note = 0;

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
		j_value = object_value(j_ptr, 1, TRUE);

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
	what = randint0(st_ptr->stock_num);

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
				if (randint0(100) < 50) break;

				/* Keep things to increments of 5 */
				if (num % 5)
				{
					/* `num` is number of items to remove */
					num = num % 5;
				}
				else
				{
					/* Maybe decrement some more */
					if (randint0(100) < 75) break;

					/* Decrement by a random factor of 5 */
					num = randint1(cur_num) * 5;
				}

				break;
			}

			default:
			{
				/* Sometimes destroy a single object */
				if (randint0(100) < 50) num = 1;

				/* Sometimes destroy half the objects */
				else if (randint0(100) < 50) num = (num + 1) / 2;


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

	/* Is the item an artifact? Mark it as lost if the player has it in history list */
	if (artifact_p(o_ptr))
		history_lose_artifact(o_ptr->name1);

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
	if (object_value(o_ptr, 1, TRUE) < 10) return (FALSE);

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
 * Get a choice from the store allocation table, in tables.c
 */
static s16b store_get_choice(int st)
{
	int r;
	store_type *st_ptr = &store[st];

	/* Choose a random entry from the store's table */
	r = randint0(st_ptr->table_num);

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
		min_level = p_ptr->max_depth + 5;
		max_level = p_ptr->max_depth + 20;
	}
	else
	{
		min_level = 1;
		max_level = STORE_OBJ_LEVEL + MAX(p_ptr->max_depth - 20, 0);
	}

	if (min_level > 55) min_level = 55;
	if (max_level > 70) max_level = 70;

	/* Consider up to six items */
	for (tries = 0; tries < 6; tries++)
	{
		/* Work out the level for objects to be generated at */
		level = rand_range(min_level, max_level);


		/* Black Markets have a random object, of a given level */
		if (st == STORE_B_MARKET) k_idx = get_obj_num(level, FALSE);

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

		/* Reject if item is 'damaged' (i.e. negative mods) */
		switch (i_ptr->tval)
		{
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOW:
			case TV_SHOT:
			case TV_ARROW:
			case TV_BOLT:
			{
				if ((i_ptr->to_h < 0) || (i_ptr->to_d < 0)) 
					continue;
			}

			case TV_DRAG_ARMOR:
			case TV_HARD_ARMOR:
			case TV_SOFT_ARMOR:
			case TV_SHIELD:
			case TV_HELM:
			case TV_CROWN:
			case TV_CLOAK:
			case TV_GLOVES:
			case TV_BOOTS:
			{
				if (i_ptr->to_a < 0) continue;
			}

			default:
			{
				/* nothing to do */
			}
		}

		/* The object is "known" and belongs to a store */
		object_known(i_ptr);
		i_ptr->ident |= IDENT_STORE;
		i_ptr->origin = ORIGIN_STORE;


		/*** Post-generation filters ***/

		/* Black markets have expensive tastes */
		if ((st == STORE_B_MARKET) && !black_market_ok(i_ptr))
			continue;

		/* No "worthless" items */
		if (object_value(i_ptr, 1, TRUE) < 1) continue;



		/* Charge lights XXX */
		if (i_ptr->tval == TV_LITE)
		{
			if (i_ptr->sval == SV_LITE_TORCH)
				i_ptr->timeout = FUEL_TORCH;
			if (i_ptr->sval == SV_LITE_LANTERN)
				i_ptr->timeout = FUEL_LAMP / 2;
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
	{ TV_SCROLL, SV_SCROLL_PHASE_DOOR, MAKE_NORMAL },
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
static int store_create_item(int st, int tval, int sval)
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
	object.origin = ORIGIN_STORE;

	/* Charge lights */
	if (object.tval == TV_LITE)
	{
		if (object.sval == SV_LITE_TORCH)
			object.timeout = FUEL_TORCH;
		else if (object.sval == SV_LITE_LANTERN)
			object.timeout = FUEL_LAMP / 2;
	}

	/* Attempt to carry the object */
	return store_carry(st, &object);
}



/*
 * Create all staple items.
 */
static void store_create_staples(void)
{
	unsigned i;

	/* Iterate through staples */
	for (i = 0; i < N_ELEMENTS(staples); i++)
	{
		struct staple_type *staple = &staples[i];
		object_type *o_ptr;

		int idx = store_find(STORE_GENERAL, staple->tval, staple->sval);

		/* Look for the item, and if it isn't there, create it */
		if (idx == -1)
			idx = store_create_item(STORE_GENERAL,
					staple->tval, staple->sval);

		o_ptr = &store[STORE_GENERAL].stock[idx];

		/* Stock appropriate amounts */
		switch (staple->mode)
		{
			case MAKE_SINGLE:
				o_ptr->number = 1;
				break;

			case MAKE_NORMAL:
				mass_produce(o_ptr);
				break;

			case MAKE_MAX:
				o_ptr->number = 99;
				break;
		}
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
	stock -= randint1(STORE_TURNOVER);

	/* Keep stock between STORE_MAX_KEEP and STORE_MIN_KEEP slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* Destroy objects until only "j" slots are left */
	while (st_ptr->stock_num > stock) store_delete_item(which);


	/*** "Buy in" various items */

	/* Buy a few items */
	stock = st_ptr->stock_num;
	stock += randint1(STORE_TURNOVER);

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
		st_ptr->owner = (byte)randint0(z_info->b_max);

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
	    i = randint0(z_info->b_max);

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
	if (current_store() != STORE_HOME)
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
 * Redisplay a single store entry
 */
static void store_display_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	object_type *o_ptr;
	s32b x;
	odesc_detail_t desc;

	char o_name[80];
	char out_val[160];
	byte colour;

	int this_store = current_store(); 
	store_type *st_ptr = &store[this_store];

	(void)menu;
	(void)cursor;
	(void)width;

	/* Get the object */
	o_ptr = &st_ptr->stock[oid];

	/* Describe the object - preserving insriptions in the home */
	if (this_store == STORE_HOME) desc = ODESC_FULL;
	else desc = ODESC_FULL | ODESC_STORE;
	object_desc(o_name, sizeof(o_name), o_ptr, TRUE, desc);

	/* Display the object */
	c_put_str(tval_to_attr[o_ptr->tval & 0x7F], o_name, row, col);

	/* Show weights */
	colour = curs_attrs[CURS_KNOWN][(int)cursor];
	strnfmt(out_val, sizeof out_val, "%3d.%d lb", o_ptr->weight / 10, o_ptr->weight % 10);
	c_put_str(colour, out_val, row, scr_places_x[LOC_WEIGHT]);

	/* Describe an object (fully) in a store */
	if (this_store != STORE_HOME)
	{
		/* Extract the "minimum" price */
		x = price_item(o_ptr, FALSE, 1);

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
	int this_store = current_store();

	owner_type *ot_ptr = store_owner(this_store);

	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (this_store == STORE_HOME)
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
		const char *store_name = (f_name + f_info[FEAT_SHOP_HEAD + this_store].name);
		const char *owner_name = &b_name[ot_ptr->owner_name];

		/* Put the owner name */
		put_str(owner_name, scr_places_y[LOC_OWNER], 1);

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

	if (OPT(rogue_like_commands))
		text_out_c(TERM_L_GREEN, "x");
	else
		text_out_c(TERM_L_GREEN, "l");

	text_out("' examines and '");
	text_out_c(TERM_L_GREEN, "p");

	if (current_store() == STORE_HOME) text_out("' picks up");
	else text_out("' purchases");

	text_out(" the selected item. '");

	text_out_c(TERM_L_GREEN, "d");
	if (current_store() == STORE_HOME) text_out("' drops");
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
 * Return the quantity of a given item in the pack.
 */
static int find_inven(const object_type *o_ptr)
{
	int j;
	int num = 0;

	/* Similar slot? */
	for (j = 0; j < INVEN_PACK; j++)
	{
		object_type *j_ptr = &inventory[j];

		/* Require identical object types */
		if (!j_ptr->k_idx || o_ptr->k_idx != j_ptr->k_idx) continue;

		/* Analyze the items */
		switch (o_ptr->tval)
		{
			/* Chests */
			case TV_CHEST:
			{
				/* Never okay */
				return 0;
			}

			/* Food and Potions and Scrolls */
			case TV_FOOD:
			case TV_POTION:
			case TV_SCROLL:
			{
				/* Assume okay */
				break;
			}

			/* Staves and Wands */
			case TV_STAFF:
			case TV_WAND:
			{
				/* Assume okay */
				break;
			}

			/* Rods */
			case TV_ROD:
			{
				/* Assume okay */
				break;
			}

			/* Weapons and Armor */
			case TV_BOW:
			case TV_DIGGING:
			case TV_HAFTED:
			case TV_POLEARM:
			case TV_SWORD:
			case TV_BOOTS:
			case TV_GLOVES:
			case TV_HELM:
			case TV_CROWN:
			case TV_SHIELD:
			case TV_CLOAK:
			case TV_SOFT_ARMOR:
			case TV_HARD_ARMOR:
			case TV_DRAG_ARMOR:
			{
				/* Fall through */
			}

			/* Rings, Amulets, Lites */
			case TV_RING:
			case TV_AMULET:
			case TV_LITE:
			{
				/* Require both items to be known */
				if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) continue;

				/* Fall through */
			}

			/* Missiles */
			case TV_BOLT:
			case TV_ARROW:
			case TV_SHOT:
			{
				/* Require identical knowledge of both items */
				if (object_known_p(o_ptr) != object_known_p(j_ptr)) continue;

				/* Require identical "bonuses" */
				if (o_ptr->to_h != j_ptr->to_h) continue;
				if (o_ptr->to_d != j_ptr->to_d) continue;
				if (o_ptr->to_a != j_ptr->to_a) continue;

				/* Require identical "pval" code */
				if (o_ptr->pval != j_ptr->pval) continue;

				/* Require identical "artifact" names */
				if (o_ptr->name1 != j_ptr->name1) continue;

				/* Require identical "ego-item" names */
				if (o_ptr->name2 != j_ptr->name2) continue;

				/* Lites must have same amount of fuel */
				else if (o_ptr->timeout != j_ptr->timeout && o_ptr->tval == TV_LITE)
					continue;

				/* Require identical "values" */
				if (o_ptr->ac != j_ptr->ac) continue;
				if (o_ptr->dd != j_ptr->dd) continue;
				if (o_ptr->ds != j_ptr->ds) continue;

				/* Probably okay */
				break;
			}

			/* Various */
			default:
			{
				/* Require knowledge */
				if (!object_known_p(o_ptr) || !object_known_p(j_ptr)) continue;

				/* Probably okay */
				break;
			}
		}


		/* Different flags */
		if (o_ptr->flags[0] != j_ptr->flags[0] ||
			o_ptr->flags[1] != j_ptr->flags[1] ||
			o_ptr->flags[2] != j_ptr->flags[2])
			continue;

		/* They match, so add up */
		num += j_ptr->number;
	}

	return num;
}


/*
 * Buy the item with the given index from the current store's inventory.
 */
void do_cmd_buy(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	int amt = args[1].number;

	object_type *o_ptr;	
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	char o_name[80];
	int price, item_new;

	store_type *st_ptr;
	int this_store = current_store();

	if (this_store == STORE_NONE)
	{
		msg_print("You cannot purchase items when not in a store.");
		return;
	}

	st_ptr = &store[this_store];

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];

	/* Get desired object */
	object_copy_amt(i_ptr, o_ptr, amt);

	/* Ensure we have room */
	if (!inven_carry_okay(i_ptr))
	{
		msg_print("You cannot carry that many items.");
		return;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), i_ptr, TRUE, ODESC_FULL);

	/* Extract the price for the entire stack */
	price = price_item(i_ptr, FALSE, i_ptr->number);

	if (price > p_ptr->au)
	{
		msg_print("You cannot afford that purchase.");
		return;
	}

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
	if (one_in_(3)) message(MSG_STORE5, 0, ONE_OF(comment_accept));
	msg_format("You bought %s for %ld gold.", o_name, (long)price);

	/* Erase the inscription */
	i_ptr->note = 0;

	/* Give it to the player */
	item_new = inven_carry(i_ptr);

	/* Message */
	object_desc(o_name, sizeof(o_name), &inventory[item_new], TRUE, ODESC_FULL);
	msg_format("You have %s (%c).", o_name, index_to_label(item_new));

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
	store_item_increase(this_store, item, -amt);
	store_item_optimize(this_store, item);

	/* Store is empty */
	if (st_ptr->stock_num == 0)
	{
		int i;

		/* Shuffle */
		if (one_in_(STORE_SHUFFLE))
		{
			/* Message */
			msg_print("The shopkeeper retires.");

			/* Shuffle the store */
			store_shuffle(this_store);
			store_flags |= STORE_FRAME_CHANGE;
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
			store_maint(this_store);
		}
	}

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Retrieve the item with the given index from the home's inventory.
 */
void do_cmd_retrieve(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	int amt = args[1].number;

	object_type *o_ptr;	
	object_type picked_item;
	char o_name[80];
	int item_new;

	store_type *st_ptr;

	if (current_store() != STORE_HOME)
	{
		msg_print("You are not currently at home.");
		return;
	}

	st_ptr = &store[STORE_HOME];

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];

	/* Get desired object */
	object_copy_amt(&picked_item, o_ptr, amt);

	/* Ensure we have room */
	if (!inven_carry_okay(&picked_item))
	{
		msg_print("You cannot carry that many items.");
		return;
	}

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(o_ptr, &picked_item, amt);
	
	/* Give it to the player */
	item_new = inven_carry(&picked_item);

	/* Describe just the result */
	object_desc(o_name, sizeof(o_name), &inventory[item_new], TRUE, ODESC_FULL);
	
	/* Message */
	msg_format("You have %s (%c).", o_name, index_to_label(item_new));
	
	/* Handle stuff */
	handle_stuff();
	
	/* Remove the items from the home */
	store_item_increase(STORE_HOME, item, -amt);
	store_item_optimize(STORE_HOME, item);
	
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Buy an object from a store
 */
static bool store_purchase(int item)
{
	int amt, num;

	object_type *o_ptr;

	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	char o_name[80];

	s32b price;

	int this_store = current_store();

	store_type *st_ptr;

	if (this_store == STORE_NONE)
	{
		msg_print("You cannot purchase items when not in a store.");
		return FALSE;
	}

	st_ptr = &store[this_store];

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];
	if (item < 0) return FALSE;

	/* Clear all current messages */
	msg_flag = FALSE;
	prt("", 0, 0);

	if (this_store == STORE_HOME)
	{
		amt = o_ptr->number;
	}
	else
	{
		/* Price of one */
		price = price_item(o_ptr, FALSE, 1);

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
		
		/* Double check for wands/staves */
		if ((p_ptr->au >= price_item(o_ptr, FALSE, amt+1)) && (amt < o_ptr->number))
			amt++;

	}

	/* Find the number of this item in the inventory */
	if (!object_aware_p(o_ptr))
		num = 0;
	else
		num = find_inven(o_ptr);

	strnfmt(o_name, sizeof o_name, "%s how many%s? (max %d) ",
	        (this_store == STORE_HOME) ? "Take" : "Buy",
	        num ? format(" (you have %d)", num) : "", amt);

	/* Get a quantity */
	amt = get_quantity(o_name, amt);

	/* Allow user abort */
	if (amt <= 0) return FALSE;

	/* Get desired object */
	object_copy_amt(i_ptr, o_ptr, amt);

	/* Ensure we have room */
	if (!inven_carry_okay(i_ptr))
	{
		msg_print("You cannot carry that many items.");
		store_flags |= STORE_KEEP_PROMPT;
		return FALSE;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), i_ptr, TRUE, ODESC_FULL);

	/* Attempt to buy it */
	if (this_store != STORE_HOME)
	{
		u32b price;
		bool response;

		/* Extract the price for the entire stack */
		price = price_item(i_ptr, FALSE, i_ptr->number);

		screen_save();

		/* Show price */
		prt(format("Price: %d", price), 1, 0);

		/* Confirm purchase */
		response = store_get_check(format("Buy %s? [ESC, any other key to accept]", o_name));
		screen_load();

		/* Negative response, so give up */
		if (!response) return FALSE;

		cmd_insert(CMD_BUY, item, amt);
		store_flags |= STORE_KEEP_PROMPT;
	}

	/* Home is much easier */
	else
	{
		cmd_insert(CMD_RETRIEVE, item, amt);
		store_flags |= STORE_KEEP_PROMPT;
	}

	/* Not kicked out */
	return TRUE;
}



/*
 * Determine if the current store will purchase the given object
 */
static bool store_will_buy_tester(const object_type *o_ptr)
{
	int this_store = current_store();
	
	if (this_store == STORE_NONE) return FALSE;

	return store_will_buy(this_store, o_ptr);
}

/*
 * Sell an item to the current store.
 */
void do_cmd_sell(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	int amt = args[1].number;
	object_type sold_item;
	int price, dummy, value;
	char o_name[120];
	
	/* Get the item */
	object_type *o_ptr = object_from_item_idx(item);
	
	/* Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		msg_print("Hmmm, it seems to be cursed.");
		return;
	}	
	
	/* Check we are somewhere we can sell the items. */
	if (current_store() == STORE_NONE)
	{
		msg_print("You cannot sell items when not in a store.");
		return;
	}
	
	/* Check the store wants the items being sold */
	if (!store_will_buy(current_store(), o_ptr))
	{
		msg_print("I do not wish to purchase this item.");
		return;
	}
	
	/* Get a copy of the object representing the number being sold */
	object_copy_amt(&sold_item, o_ptr, amt);
	
	/* Check if the store has space for the items */
	if (!store_check_num(current_store(), &sold_item))
	{
		msg_print("I have not the room in my store to keep it.");
		return;
	}
	
	price = price_item(&sold_item, TRUE, amt);
	
	/* Get some money */
	p_ptr->au += price;
	
	/* Update the display */
	store_flags |= STORE_GOLD_CHANGE;
	
	/* Identify original object */
	object_aware(o_ptr);
	object_known(o_ptr);
	
	/* Update the auto-history if selling an artifact that was previously un-IDed. (Ouch!) */
	if (artifact_p(o_ptr))
		history_add_artifact(o_ptr->name1, TRUE);
	
	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	
	/* Redraw stuff */
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
	
	/* The object belongs to the store now */
	sold_item.ident |= IDENT_STORE;
	
	/* Get the "apparent" value */
	dummy = object_value(&sold_item, amt, TRUE);
	
	/* Take a new copy of the now known-about object. */
	object_copy_amt(&sold_item, o_ptr, amt);
	   
	/*
	* Hack -- Allocate charges between those wands, staves, or rods
	* sold and retained, unless all are being sold.
	 */
	distribute_charges(o_ptr, &sold_item, amt);
	
	/* Get the "actual" value */
	value = object_value(&sold_item, amt, TRUE);
	
	/* Get the description all over again */
	object_desc(o_name, sizeof(o_name), &sold_item, TRUE, ODESC_FULL);
	
	/* Describe the result (in message buffer) */
	msg_format("You sold %s (%c) for %ld gold.",
			   o_name, index_to_label(item), (long)price);
	
	/* Analyze the prices (and comment verbally) */
	purchase_analyze(price, value, dummy);
	
	/* Set squelch flag */
	p_ptr->notice |= PN_SQUELCH;
	
	/* Take the object from the player */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);
	
	/* Handle stuff */
	handle_stuff();
	
	/* The store gets that (known) object */
	store_carry(current_store(), &sold_item);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Stash an item in the home.
 */
void do_cmd_stash(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	int amt = args[1].number;
	object_type dropped_item;
	object_type *o_ptr = object_from_item_idx(item);
	char o_name[120];

	/* Check we are somewhere we can stash items. */
	if (current_store() != STORE_HOME)
	{
		msg_print("You are not in your home.");
		return;
	}

	/* Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr))
	{
		msg_print("Hmmm, it seems to be cursed.");
		return;
	}	

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(&dropped_item, o_ptr, amt);

	if (!store_check_num(STORE_HOME, &dropped_item))
	{
		msg_print("Your home is full.");
		return;
	}

	/* Distribute charges of wands/staves/rods */
	distribute_charges(o_ptr, &dropped_item, amt);
	
	/* Describe */
	msg_format("You drop %s (%c).", o_name, index_to_label(item));
	
	/* Take it from the players inventory */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);
	
	/* Handle stuff */
	handle_stuff();
	
	/* Let the home carry it */
	home_carry(&dropped_item);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Sell an object, or drop if it we're in the home.
 */
static void store_sell(void)
{
	int amt;
	int item;

	object_type *o_ptr;
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	char o_name[120];


	const char *reject = "You have nothing that I want. ";
	const char *prompt = "Sell which item? ";

	int this_store = current_store();

	if (this_store == STORE_NONE)
	{
		msg_print("You cannot sell items when not in a store.");
		return;
	}

	/* Clear all current messages */
	msg_flag = FALSE;
	prt("", 0, 0);

	if (this_store == STORE_HOME)
		prompt = "Drop which item? ";
	else
		item_tester_hook = store_will_buy_tester;

	/* Get an item */
	p_ptr->command_wrk = USE_INVEN;
	p_ptr->command_cmd = 'd';
	if (!get_item(&item, prompt, reject, (USE_EQUIP | USE_INVEN | USE_FLOOR)))
	{
		store_flags |= STORE_KEEP_PROMPT;
		return;
	}

	/* Get the item */
	o_ptr = object_from_item_idx(item);

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

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(i_ptr, object_from_item_idx(item), amt);

	if (!store_check_num(this_store, i_ptr))
	{
		store_flags |= STORE_KEEP_PROMPT;

		if (this_store == STORE_HOME)
			msg_print("Your home is full.");

		else
			msg_print("I have not the room in my store to keep it.");

		return;
	}

	/* Get a full description */
	object_desc(o_name, sizeof(o_name), i_ptr, TRUE, ODESC_FULL);

	/* Real store */
	if (this_store != STORE_HOME)
	{
		/* Extract the value of the items */
		u32b price = price_item(i_ptr, TRUE, amt);

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

		cmd_insert(CMD_SELL, item, amt);

		store_flags |= STORE_KEEP_PROMPT;
	}

	/* Player is at home */
	else
	{
		cmd_insert(CMD_STASH, item, amt);
	}
}


/*
 * Examine an item in a store
 */
static void store_examine(int item)
{
	store_type *st_ptr = &store[current_store()];
	object_type *o_ptr;
	bool info_known;

	if (item < 0) return;

	/* Get the actual object */
	o_ptr = &st_ptr->stock[item];

	/* Describe it fully */
	Term_erase(0, 0, 255);
	Term_gotoxy(0, 0);

	text_out_hook = text_out_to_screen;
	screen_save();

	object_info_header(o_ptr);

	/* Show full info in most stores, but normal info in player home */
	info_known = object_info(o_ptr,
			(current_store() != STORE_HOME) ? TRUE : FALSE);

	if (!info_known)
		text_out("\n\nThis item does not seem to possess any special abilities.");

	text_out_c(TERM_L_BLUE, "\n\n[Press any key to continue]\n");
	(void)anykey();

	screen_load();

	/* Hack -- Browse book, then prompt for a command */
	if (o_ptr->tval == cp_ptr->spell_book)
	{
		/* Call the aux function */
		do_cmd_browse_aux(o_ptr, item);
	}
}


/*
 * Flee the store when it overflows.
 */
static bool store_overflow(void)
{
	int item = INVEN_PACK;

	object_type *o_ptr = &inventory[item];

	/* Flee from the store */
	if (current_store() != STORE_HOME)
	{
		/* Leave */
		msg_print("Your pack is so full that you flee the store...");
		return TRUE;
	}

	/* Flee from the home */
	else if (!store_check_num(current_store(), o_ptr))
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
		object_desc(o_name, sizeof(o_name), i_ptr, TRUE, ODESC_FULL);

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
	bool redraw = FALSE;
	bool command_processed = FALSE;

	/* Parse the command */
	switch (cmd)
	{
		/* Leave */
		case ESCAPE:
		{
			command_processed = TRUE;
			break;
		}

		/* Sell */
		case 's':
		case 'd':
		{
			store_sell();
			command_processed = TRUE;
			break;
		}

		/* Buy */
		case 'p':
		case 'g':
		{
			/* On successful purchase, redraw */
			command_processed = store_purchase(oid);
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
			command_processed = TRUE;
			break;
		}



		/*** Inventory Commands ***/

		/* Wear/wield equipment */
		case 'w':
		{
			textui_cmd_wield();
			redraw = TRUE;
			
			break;
		}

		/* Take off equipment */
		case 'T':
		case 't':
		{
			textui_cmd_takeoff();
			redraw = TRUE;
			
			break;
		}

		/* Destroy an item */
		case KTRL('D'):
		case 'k':
		{
			textui_cmd_destroy();
			redraw = TRUE;
			
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
			textui_cmd_inscribe();
			redraw = TRUE;
			
			break;
		}

		/* Uninscribe an object */
		case '}':
		{
			textui_cmd_uninscribe();
			redraw = TRUE;
			
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

			command_processed = TRUE;
			break;
		}

		/*** System Commands ***/

		/* Interact with options */
		case '=':
		{
			do_cmd_options();
			redraw = TRUE;
			
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

	/* Let the game handle any core commands (equipping, etc) */
	process_command(CMD_STORE, TRUE);

	if (redraw)
	{
		event_signal(EVENT_INVENTORY);
		event_signal(EVENT_EQUIPMENT);
	}
	
	return command_processed;
}


/*
 * Enter a store, and interact with it.
 */
void do_cmd_store(cmd_code code, cmd_arg args[])
{
	bool leave = FALSE;

	/* Take note of the store number from the terrain feature */
	int this_store = current_store();

	/* Verify that there is a store */
	if (this_store == STORE_NONE)
	{
		msg_print("You see no store here.");
		return;
	}

	/* Check if we can enter the store */
	if (OPT(adult_no_stores))
	{
		msg_print("The doors are locked.");
		return;
	}

	/* Shut down the normal game view - it won't be updated - and start
	   up the store state. */
	event_signal(EVENT_LEAVE_GAME);
	event_signal(EVENT_ENTER_STORE);

	/* Forget the view */
	forget_view();

	/* Reset the command variables */
	p_ptr->command_arg = 0;
	p_ptr->command_rep = 0;
	p_ptr->command_new = 0;




	/*** Display ***/

	/* Save current screen (ie. dungeon) */
	screen_save();

	/*** Inventory display ***/
	{

	static region items_region = { 1, 4, -1, -1 };
	static const menu_iter store_menu = { NULL, NULL, store_display_entry, store_process_command };
	const menu_iter *cur_menu = &store_menu;

	menu_type menu;
	ui_event_data evt = EVENT_EMPTY;
	int cursor = 0;

	store_type *st_ptr = &store[this_store];

	/* Wipe the menu and set it up */
	WIPE(&menu, menu);
	menu.flags = MN_DBL_TAP;

	/* Calculate the positions of things and redraw */
	store_flags = STORE_INIT_CHANGE;
	store_display_recalc();
	store_redraw();

	/* Say a friendly hello. */
	if (this_store != STORE_HOME) 
		prt_welcome(store_owner(this_store));

	/* Loop */
	while (!leave)
	{
		/* As many rows in the menus as there are items in the store */
		menu.count = st_ptr->stock_num;

		/* Roguelike */
		if (OPT(rogue_like_commands))
		{
			/* These two can't intersect! */
			menu.cmd_keys = "\n\x04\x10\r?={}~CEIPTdegilpswx\x8B\x8C"; /* \x10 = ^p , \x04 = ^D */
			menu.selections = "abcfmnoqrtuvyz13456790ABDFGH";
		}

		/* Original */
		else
		{
			/* These two can't intersect! */
			menu.cmd_keys = "\n\x010\r?={}~CEIbdegiklpstw\x8B\x8C"; /* \x10 = ^p */
			menu.selections = "acfhmnoqruvxyz13456790ABDFGH";
		}

		/* Keep the cursor in range of the stock */
		if (cursor < 0 || cursor >= menu.count)
			cursor = menu.count - 1;

		items_region.page_rows = scr_places_y[LOC_MORE] - scr_places_y[LOC_ITEMS_START];

		/* Init the menu structure */
		menu_init(&menu, MN_SKIN_SCROLL, cur_menu, &items_region);

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
				/* Unset so that the prompt is cleared next time */
				store_flags &= ~STORE_KEEP_PROMPT;
			}
			else
			{
				/* Clear the prompt */
				prt("", 0, 0);
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

		msg_flag = FALSE;
	}

	}

	/* Switch back to the normal game view. */
	event_signal(EVENT_LEAVE_STORE);
	event_signal(EVENT_ENTER_GAME);

	/* Take a turn */
	p_ptr->energy_use = 100;


	/* Hack -- Cancel automatic command */
	p_ptr->command_new = 0;


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
}
