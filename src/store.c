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
#include "cave.h"
#include "cmds.h"
#include "game-event.h"
#include "history.h"
#include "init.h"
#include "object/inventory.h"
#include "object/tvalsval.h"
#include "object/object.h"
#include "spells.h"
#include "squelch.h"
#include "target.h"
#include "textui.h"
#include "ui-menu.h"
#include "z-debug.h"

/*** Constants and definitions ***/

/* Easy names for the elements of the 'scr_places' arrays. */
enum
{
	LOC_PRICE = 0,
	LOC_OWNER,
	LOC_HEADER,
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





/* Compound flag for the initial display of a store */
#define STORE_INIT_CHANGE		(STORE_FRAME_CHANGE | STORE_GOLD_CHANGE)



/* Some local constants */
#define STORE_TURNOVER  9       /* Normal shop turnover, per day */
#define STORE_OBJ_LEVEL 5       /* Magic Level for normal stores */



/** Variables to maintain state XXX ***/

/* Flags for the display */
static u16b store_flags;




/*** Utilities ***/


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

static const char *comment_hint[] =
{
/*	"%s tells you soberly: \"%s\".",
	"(%s) There's a saying round here, \"%s\".",
	"%s offers to tell you a secret next time you're about."*/
	"\"%s\""
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
	{ TV_LIGHT, SV_LIGHT_TORCH, MAKE_NORMAL },
	{ TV_SCROLL, SV_SCROLL_WORD_OF_RECALL, MAKE_NORMAL },
	{ TV_SCROLL, SV_SCROLL_PHASE_DOOR, MAKE_NORMAL },
	{ TV_FLASK, 0, MAKE_NORMAL },
	{ TV_SPIKE, 0, MAKE_NORMAL },
	{ TV_SHOT, SV_AMMO_NORMAL, MAKE_MAX },
	{ TV_ARROW, SV_AMMO_NORMAL, MAKE_MAX },
	{ TV_BOLT, SV_AMMO_NORMAL, MAKE_MAX },
	{ TV_DIGGING, SV_SHOVEL, MAKE_SINGLE },
	{ TV_DIGGING, SV_PICK, MAKE_SINGLE },
	{ TV_CLOAK, SV_CLOAK, MAKE_SINGLE }
};

static struct store *store_new(int idx) {
	struct store *s = mem_zalloc(sizeof *s);
	s->sidx = idx;
	s->stock = mem_zalloc(sizeof(*s->stock) * STORE_INVEN_MAX);
	s->stock_size = STORE_INVEN_MAX;
	return s;
}

/*
 * Get rid of stores at cleanup. Gets rid of everything.
 */
void free_stores(void)
{
	struct owner *o;
	struct owner *next;
	int i;

	/* Free the store inventories */
	for (i = 0; i < MAX_STORES; i++)
	{
		/* Get the store */
		struct store *store = &stores[i];

		/* Free the store inventory */
		mem_free(store->stock);
		mem_free(store->table);

		for (o = store->owners; o; o = next) {
			next = o->next;
			string_free(o->name);
			mem_free(o);
		}
	}
	mem_free(stores);
}

static enum parser_error parse_s(struct parser *p) {
	struct store *h = parser_priv(p);
	struct store *s;
	unsigned int idx = parser_getuint(p, "index") - 1;
	unsigned int slots = parser_getuint(p, "slots");

	if (idx < STORE_ARMOR || idx > STORE_MAGIC)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	s = store_new(parser_getuint(p, "index") - 1);
	s->table = mem_zalloc(sizeof(*s->table) * slots);
	s->table_size = slots;
	s->next = h;
	parser_setpriv(p, s);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_i(struct parser *p) {
	struct store *s = parser_priv(p);
	unsigned int slots = parser_getuint(p, "slots");
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	int sval = lookup_sval(tval, parser_getsym(p, "sval"));
	object_kind *kind = lookup_kind(tval, sval);

	if (!kind)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	if (s->table_num + slots > s->table_size)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	while (slots--) {
		s->table[s->table_num++] = kind;
	}
	/* XXX: get rid of this table_size/table_num/indexing thing. It's
	 * stupid. Dynamically allocate. */
	return PARSE_ERROR_NONE;
}

testonly struct parser *store_parser_new(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "S uint index uint slots", parse_s);
	parser_reg(p, "I uint slots sym tval sym sval", parse_i);
	return p;
}

struct owner_parser_state {
	struct store *stores;
	struct store *cur;
};

static enum parser_error parse_own_n(struct parser *p) {
	struct owner_parser_state *s = parser_priv(p);
	unsigned int index = parser_getuint(p, "index");
	struct store *st;

	for (st = s->stores; st; st = st->next) {
		if (st->sidx == index) {
			s->cur = st;
			break;
		}
	}

	return st ? PARSE_ERROR_NONE : PARSE_ERROR_OUT_OF_BOUNDS;
}

static enum parser_error parse_own_s(struct parser *p) {
	struct owner_parser_state *s = parser_priv(p);
	unsigned int maxcost = parser_getuint(p, "maxcost");
	char *name = string_make(parser_getstr(p, "name"));
	struct owner *o;

	if (!s->cur)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	o = mem_zalloc(sizeof *o);
	o->oidx = (s->cur->owners ? s->cur->owners->oidx + 1 : 0);
	o->next = s->cur->owners;
	o->name = name;
	o->max_cost = maxcost;
	s->cur->owners = o;
	return PARSE_ERROR_NONE;
}

testonly struct parser *store_owner_parser_new(struct store *stores) {
	struct parser *p = parser_new();
	struct owner_parser_state *s = mem_zalloc(sizeof *s);
	s->stores = stores;
	s->cur = NULL;
	parser_setpriv(p, s);
	parser_reg(p, "V sym version", ignored);
	parser_reg(p, "N uint index", parse_own_n);
	parser_reg(p, "S uint maxcost str name", parse_own_s);
	return p;
}

/*
 * The greeting a shopkeeper gives the character says a lot about his
 * general attitude.
 *
 * Taken and modified from Sangband 1.0.
 */
static void prt_welcome(const owner_type *ot_ptr)
{
	char short_name[20];
	const char *owner_name = ot_ptr->name;

	int j;

	if (one_in_(2))
		return;

	/* Extract the first name of the store owner (stop before the first space) */
	for (j = 0; owner_name[j] && owner_name[j] != ' '; j++)
		short_name[j] = owner_name[j];

	/* Truncate the name */
	short_name[j] = '\0';

	if (one_in_(3)) {
		size_t i = randint0(N_ELEMENTS(comment_hint));
		msg(comment_hint[i], random_hint());
	} else if (p_ptr->lev > 5) {
		const char *player_name;

		/* We go from level 1 - 50  */
		size_t i = ((unsigned)p_ptr->lev - 1) / 5;
		i = MIN(i, N_ELEMENTS(comment_welcome) - 1);

		/* Get a title for the character */
		if ((i % 2) && randint0(2)) player_name = p_ptr->class->title[(p_ptr->lev - 1) / 5];
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
		msgt(MSG_STORE1, "%s", ONE_OF(comment_worthless));

	/* Item was cheaper than we thought, and we paid more than necessary */
	else if ((value < guess) && (price > value))
		msgt(MSG_STORE2, "%s", ONE_OF(comment_bad));

	/* Item was a good bargain, and we got away with it */
	else if ((value > guess) && (value < (4 * guess)) && (price < value))
		msgt(MSG_STORE3, "%s", ONE_OF(comment_good));

	/* Item was a great bargain, and we got away with it */
	else if ((value > guess) && (price < value))
		msgt(MSG_STORE4, "%s", ONE_OF(comment_great));
}




/*** Check if a store will buy an object ***/

/*
 * Determine if the current store will purchase the given object
 *
 * Note that a shop-keeper must refuse to buy "worthless" objects
 */
static bool store_will_buy(struct store *store, const object_type *o_ptr)
{
	/* Switch on the store */
	switch (store->sidx)
	{
		/* General Store */
		case STORE_GENERAL:
		{
			size_t i;
			bool accept = FALSE;

			/* Accept lights and food */
			if (o_ptr->tval == TV_LIGHT || o_ptr->tval == TV_FOOD)
			    accept = TRUE;

			/* Accept staples */
			for (i = 0; !accept && i < N_ELEMENTS(staples); i++)
			{
				if (staples[i].tval == o_ptr->tval &&
				    staples[i].sval == o_ptr->sval &&
				    object_is_known(o_ptr))
					accept = TRUE;
			}

			if (!accept) return FALSE;
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
				case TV_DIGGING:
				{
					/* Known blessed blades are accepted too */
					if (object_is_known_blessed(o_ptr)) break;
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

		/* Home */
		case STORE_HOME:
		{
			return TRUE;
		}
	}

	/* Ignore "worthless" items XXX XXX XXX */
	if (object_value(o_ptr, 1, FALSE) <= 0) return (FALSE);

	/* Assume okay */
	return (TRUE);
}


/* Get the current store or NULL if there isn't one */
static struct store *current_store(void)
{
	int n = STORE_NONE;

	/* If we're displaying store knowledge whilst not in a store,
	 * override the value returned
	 */
	if (store_knowledge != STORE_NONE)
		n = store_knowledge;

	else if ((cave->feat[p_ptr->py][p_ptr->px] >= FEAT_SHOP_HEAD) &&
			(cave->feat[p_ptr->py][p_ptr->px] <= FEAT_SHOP_TAIL))
		n = cave->feat[p_ptr->py][p_ptr->px] - FEAT_SHOP_HEAD;

	if (n != STORE_NONE)
		return &stores[n];
	else
		return NULL;
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
	struct store *store = current_store();
	owner_type *ot_ptr;

	if (!store) return 0L;

	ot_ptr = store->owner;


	/* Get the value of the stack of wands, or a single item */
	if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
		price = object_value(o_ptr, qty, FALSE);
	else
		price = object_value(o_ptr, 1, FALSE);

	/* Worthless items */
	if (price <= 0) return (0L);


	/* Add in the charisma factor */
	if (store->sidx == STORE_B_MARKET)
		adjust = 150;
	else
		adjust = adj_chr_gold[p_ptr->state.stat_ind[A_CHR]];


	/* Shop is buying */
	if (store_buying)
	{
		/* Set the factor */
		adjust = 100 + (100 - adjust);
		if (adjust > 100) adjust = 100;

		/* Shops now pay 1/3 of true value */
		price = price / 3;

		/* Black market sucks */
		if (store->sidx == STORE_B_MARKET)
			price = price / 2;

		/* Check for no_selling option */
		if (OPT(birth_no_selling)) return (0L);
	}

	/* Shop is selling */
	else
	{
		/* Fix the factor */
		if (adjust < 100) adjust = 100;

		/* Black market sucks */
		if (store->sidx == STORE_B_MARKET)
			price = price * 2;
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
	s32b cost = object_value(o_ptr, 1, FALSE);

	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Food, Flasks, and Lights */
		case TV_FOOD:
		case TV_FLASK:
		case TV_LIGHT:
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
			if (o_ptr->ego) break;
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
}


/*
 * Allow a store object to absorb another object
 */
static void store_object_absorb(object_type *o_ptr, object_type *j_ptr)
{
	int total = o_ptr->number + j_ptr->number;

	/* Combine quantity, lose excess items */
	o_ptr->number = (total > 99) ? 99 : total;

	/* Hack -- if rods are stacking, add the charging timeouts */
	if (o_ptr->tval == TV_ROD)
		o_ptr->timeout += j_ptr->timeout;

	/* Hack -- if wands/staves are stacking, combine the charges */
	if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_STAFF))
	{
		o_ptr->pval[DEFAULT_PVAL] += j_ptr->pval[DEFAULT_PVAL];
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

			bool r_uniq = rf_has(r_ptr->flags, RF_UNIQUE) ? TRUE : FALSE;
			bool s_uniq = rf_has(s_ptr->flags, RF_UNIQUE) ? TRUE : FALSE;

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
static bool store_check_num(struct store *store, const object_type *o_ptr)
{
	int i;
	object_type *j_ptr;

	/* Free space is always usable */
	if (store->stock_num < store->stock_size) return TRUE;

	/* The "home" acts like the player */
	if (store->sidx == STORE_HOME)
	{
		/* Check all the objects */
		for (i = 0; i < store->stock_num; i++)
		{
			/* Get the existing object */
			j_ptr = &store->stock[i];

			/* Can the new object be combined with the old one? */
			if (object_similar(j_ptr, o_ptr, OSTACK_PACK))
				return (TRUE);
		}
	}

	/* Normal stores do special stuff */
	else
	{
		/* Check all the objects */
		for (i = 0; i < store->stock_num; i++)
		{
			/* Get the existing object */
			j_ptr = &store->stock[i];

			/* Can the new object be combined with the old one? */
			if (object_similar(j_ptr, o_ptr, OSTACK_STORE))
				return (TRUE);
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

	struct store *store = &stores[STORE_HOME];

	/* Check each existing object (try to combine) */
	for (slot = 0; slot < store->stock_num; slot++)
	{
		/* Get the existing object */
		j_ptr = &store->stock[slot];

		/* The home acts just like the player */
		if (object_similar(j_ptr, o_ptr, OSTACK_PACK))
		{
			/* Save the new number of items */
			object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (store->stock_num >= store->stock_size) return (-1);

	/* Determine the "value" of the object */
	value = object_value(o_ptr, 1, FALSE);

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < store->stock_num; slot++)
	{
		/* Get that object */
		j_ptr = &store->stock[slot];

		/* Hack -- readable books always come first */
		if ((o_ptr->tval == p_ptr->class->spell_book) &&
		    (j_ptr->tval != p_ptr->class->spell_book)) break;
		if ((j_ptr->tval == p_ptr->class->spell_book) &&
		    (o_ptr->tval != p_ptr->class->spell_book)) continue;

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Can happen in the home */
		if (!object_flavor_is_aware(o_ptr)) continue;
		if (!object_flavor_is_aware(j_ptr)) break;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Objects in the home can be unknown */
		if (!object_is_known(o_ptr)) continue;
		if (!object_is_known(j_ptr)) break;

		/* Objects sort by decreasing value */
		j_value = object_value(j_ptr, 1, FALSE);
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = store->stock_num; i > slot; i--)
	{
		/* Hack -- slide the objects */
		object_copy(&store->stock[i], &store->stock[i-1]);
	}

	/* More stuff now */
	store->stock_num++;

	/* Hack -- Insert the new object */
	object_copy(&store->stock[slot], o_ptr);

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
static int store_carry(struct store *store, object_type *o_ptr)
{
	unsigned int i;
	unsigned int slot;
	u32b value, j_value;
	object_type *j_ptr;

	object_kind *kind = o_ptr->kind;

	/* Evaluate the object */
	value = object_value(o_ptr, 1, FALSE);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0) return (-1);

	/* Erase the inscription & pseudo-ID bit */
	o_ptr->note = 0;

	/* Some item types require maintenance */
	switch (o_ptr->tval)
	{
		/* Refuel lights to the standard amount */
		case TV_LIGHT:
		{
			bitflag f[OF_SIZE];
			object_flags(o_ptr, f);

			if (!of_has(f, OF_NO_FUEL))
			{
				if (o_ptr->sval == SV_LIGHT_TORCH)
					o_ptr->timeout = DEFAULT_TORCH;

				else if (o_ptr->sval == SV_LIGHT_LANTERN)
					o_ptr->timeout = DEFAULT_LAMP;
			}

			break;
		}

		/* Recharge rods */
		case TV_ROD:
		{
			o_ptr->timeout = 0;
			break;
		}

		/* Possibly recharge wands and staves */
		case TV_STAFF:
		case TV_WAND:
		{
			bool recharge = FALSE;

			/* Recharge without fail if the store normally carries that type */
			for (i = 0; i < store->table_num; i++)
			{
				if (store->table[i] == o_ptr->kind)
					recharge = TRUE;
			}

			if (recharge)
			{
				int charges = 0;

				/* Calculate the recharged number of charges */
				for (i = 0; i < o_ptr->number; i++)
					charges += randcalc(kind->charge, 0, RANDOMISE);

				/* Use recharged value only if greater */
				if (charges > o_ptr->pval[DEFAULT_PVAL])
					o_ptr->pval[DEFAULT_PVAL] = charges;
			}

			break;
		}
	}

	/* Check each existing object (try to combine) */
	for (slot = 0; slot < store->stock_num; slot++)
	{
		/* Get the existing object */
		j_ptr = &store->stock[slot];

		/* Can the existing items be incremented? */
		if (object_similar(j_ptr, o_ptr, OSTACK_STORE))
		{
			/* Absorb (some of) the object */
			store_object_absorb(j_ptr, o_ptr);

			/* All done */
			return (slot);
		}
	}

	/* No space? */
	if (store->stock_num >= store->stock_size) {
		return (-1);
	}

	/* Check existing slots to see if we must "slide" */
	for (slot = 0; slot < store->stock_num; slot++)
	{
		/* Get that object */
		j_ptr = &store->stock[slot];

		/* Objects sort by decreasing type */
		if (o_ptr->tval > j_ptr->tval) break;
		if (o_ptr->tval < j_ptr->tval) continue;

		/* Objects sort by increasing sval */
		if (o_ptr->sval < j_ptr->sval) break;
		if (o_ptr->sval > j_ptr->sval) continue;

		/* Evaluate that slot */
		j_value = object_value(j_ptr, 1, FALSE);

		/* Objects sort by decreasing value */
		if (value > j_value) break;
		if (value < j_value) continue;
	}

	/* Slide the others up */
	for (i = store->stock_num; i > slot; i--)
	{
		/* Hack -- slide the objects */
		object_copy(&store->stock[i], &store->stock[i-1]);
	}

	/* More stuff now */
	store->stock_num++;

	/* Hack -- Insert the new object */
	object_copy(&store->stock[slot], o_ptr);

	/* Return the location */
	return (slot);
}


/*
 * Increase, by a 'num', the number of an item 'item' in store 'st'.
 * This can result in zero items.
 */
static void store_item_increase(struct store *store, int item, int num)
{
	int cnt;
	object_type *o_ptr;

	/* Get the object */
	o_ptr = &store->stock[item];

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
static void store_item_optimize(struct store *store, int item)
{
	int j;
	object_type *o_ptr;

	/* Get the object */
	o_ptr = &store->stock[item];

	/* Must exist */
	if (!o_ptr->kind) return;

	/* Must have no items */
	if (o_ptr->number) return;

	/* One less object */
	store->stock_num--;

	/* Slide everyone */
	for (j = item; j < store->stock_num; j++)
	{
		store->stock[j] = store->stock[j + 1];
	}

	/* Nuke the final slot */
	object_wipe(&store->stock[j]);
}



/*
 * Delete an object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 */
static void store_delete_index(struct store *store, int what)
{
	int num;
	object_type *o_ptr;

	/* Paranoia */
	if (store->stock_num <= 0) return;

	/* Get the object */
	o_ptr = &store->stock[what];

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
				/* 50% of the time, destroy the entire stack */
				if (randint0(100) < 50 || num < 10)
					num = o_ptr->number;

				/* 50% of the time, reduce the size to a multiple of 5 */
				else
					num = randint1(num / 5) * 5 + (num % 5);

				break;
			}

			default:
			{
				/* 50% of the time, destroy a single object */
				if (randint0(100) < 50) num = 1;

				/* 25% of the time, destroy half the objects */
				else if (randint0(100) < 50) num = (num + 1) / 2;
				
				/* 25% of the time, destroy all objects */
				else num = o_ptr->number;

				/* Hack -- decrement the total charges of staves and wands. */
				if (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND)
				{
					o_ptr->pval[DEFAULT_PVAL] -= num * o_ptr->pval[DEFAULT_PVAL] / o_ptr->number;
				}
			}
		}

	}

	if (o_ptr->artifact)
		history_lose_artifact(o_ptr->artifact);

	/* Delete the item */
	store_item_increase(store, what, -num);
	store_item_optimize(store, what);
}



/*
 * Delete a random object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 *
 * This function is used when store maintainance occurs, and is designed to
 * imitate non-PC purchasers making purchases from the store.
 */
static void store_delete_random(struct store *store)
{
	int what;

	if (store->stock_num <= 0) return;

	/* Pick a random slot */
	what = randint0(store->stock_num);

	store_delete_index(store, what);
}



/*
 * Delete a percentage of a store's inventory
 */
static void store_prune(struct store *store, int chance_in_1000)
{
	int i;

	for (i = 0; i < store->stock_num; i++) {
		if (randint0(1000) < chance_in_1000)
			store_delete_index(store, i);
	}
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
	if (o_ptr->ego) return (TRUE);

	/* Good items are normally fine */
	if (o_ptr->to_a > 2) return (TRUE);
	if (o_ptr->to_h > 1) return (TRUE);
	if (o_ptr->to_d > 2) return (TRUE);


	/* No cheap items */
	if (object_value(o_ptr, 1, FALSE) < 10) return (FALSE);

	/* Check the other stores */
	for (i = 0; i < MAX_STORES; i++)
	{
		/* Skip home and black market */
		if (i == STORE_B_MARKET || i == STORE_HOME)
			continue;

		/* Check every object in the store */
		for (j = 0; j < stores[i].stock_num; j++)
		{
			object_type *j_ptr = &stores[i].stock[j];

			/* Compare object kinds */
			if (o_ptr->kind == j_ptr->kind)
				return (FALSE);
		}
	}

	/* Otherwise fine */
	return (TRUE);
}



/*
 * Get a choice from the store allocation table, in tables.c
 */
static object_kind *store_get_choice(struct store *store)
{
	/* Choose a random entry from the store's table */
	int r = randint0(store->table_num);

	/* Return it */
	return store->table[r];
}


/*
 * Creates a random object and gives it to store 'st'
 */
static bool store_create_random(struct store *store)
{
	int tries, level;

	object_type *i_ptr;
	object_type object_type_body;

	int min_level, max_level;

	/* Decide min/max levels */
	if (store->sidx == STORE_B_MARKET) {
		min_level = p_ptr->max_depth + 5;
		max_level = p_ptr->max_depth + 20;
	} else {
		min_level = 1;
		max_level = STORE_OBJ_LEVEL + MAX(p_ptr->max_depth - 20, 0);
	}

	if (min_level > 55) min_level = 55;
	if (max_level > 70) max_level = 70;

	/* Consider up to six items */
	for (tries = 0; tries < 6; tries++)
	{
		object_kind *kind;

		/* Work out the level for objects to be generated at */
		level = rand_range(min_level, max_level);


		/* Black Markets have a random object, of a given level */
		if (store->sidx == STORE_B_MARKET)
			kind = get_obj_num(level, FALSE);
		else
			kind = store_get_choice(store);


		/*** Pre-generation filters ***/

		/* No chests in stores XXX */
		if (kind->tval == TV_CHEST) continue;


		/*** Generate the item ***/

		/* Get local object */
		i_ptr = &object_type_body;

		/* Create a new object of the chosen kind */
		object_prep(i_ptr, kind, level, RANDOMISE);

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
		i_ptr->ident |= IDENT_STORE;
		i_ptr->origin = ORIGIN_STORE;


		/*** Post-generation filters ***/

		/* Black markets have expensive tastes */
		if ((store->sidx == STORE_B_MARKET) && !black_market_ok(i_ptr))
			continue;

		/* No "worthless" items */
		if (object_value(i_ptr, 1, FALSE) < 1) continue;

		/* Mass produce and/or apply discount */
		mass_produce(i_ptr);

		/* Attempt to carry the object */
		(void)store_carry(store, i_ptr);

		/* Definitely done */
		return TRUE;
	}

	return FALSE;
}


/*
 * Helper function: create an item with the given tval,sval pair, add it to the
 * store st.  Return the slot in the inventory.
 */
static int store_create_item(struct store *store, object_kind *kind)
{
	object_type object = { 0 };

	/* Create a new object of the chosen kind */
	object_prep(&object, kind, 0, RANDOMISE);

	/* Item belongs to a store */
	object.ident |= IDENT_STORE;
	object.origin = ORIGIN_STORE;

	/* Attempt to carry the object */
	return store_carry(store, &object);
}


/*
 * Create all staple items.
 */
static void store_create_staples(void)
{
	struct store *store = &stores[STORE_GENERAL];
	size_t i;

	/* Make sure there's enough room for staples */
	while (store->stock_num >= STORE_INVEN_MAX - N_ELEMENTS(staples))
		store_delete_random(store);

	/* Iterate through staples */
	for (i = 0; i < N_ELEMENTS(staples); i++)
	{
		struct staple_type *staple = &staples[i];

		/* Create the staple and combine it into the store inventory */
		int idx = store_create_item(store,
				lookup_kind(staple->tval, staple->sval));
		object_type *o_ptr = &store->stock[idx];

		assert(o_ptr);

		/* Tweak the quantities */
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
void store_maint(struct store *store)
{
	int j;
	unsigned int stock;
	int restock_attempts = 100000;

	/* Ignore home */
	if (store->sidx == STORE_HOME)
		return;

	/* General Store gets special treatment */
	if (store->sidx == STORE_GENERAL) {
		/* Sell off 30% of the inventory */
		store_prune(store, 300);
		store_create_staples();
		return;
	}

	/* Prune the black market */
	if (store->sidx == STORE_B_MARKET)
	{
		/* Destroy crappy black market items */
		for (j = store->stock_num - 1; j >= 0; j--)
		{
			object_type *o_ptr = &store->stock[j];

			/* Destroy crappy items */
			if (!black_market_ok(o_ptr))
			{
				/* Destroy the object */
				store_item_increase(store, j, 0 - o_ptr->number);
				store_item_optimize(store, j);
			}
		}
	}

	/*** "Sell" various items */

	/* Sell a few items */
	stock = store->stock_num;
	stock -= randint1(STORE_TURNOVER);

	/* Keep stock between specified min and max slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* Destroy objects until only "j" slots are left */
	while (store->stock_num > stock) store_delete_random(store);


	/*** "Buy in" various items */

	/* Buy a few items */
	stock = store->stock_num;
	stock += randint1(STORE_TURNOVER);

	/* Keep stock between specified min and max slots */
	if (stock > STORE_MAX_KEEP) stock = STORE_MAX_KEEP;
	if (stock < STORE_MIN_KEEP) stock = STORE_MIN_KEEP;

	/* For the rest, we just choose items randomlyish */
	/* The (huge) restock_attempts will only go to zero (otherwise
	 * infinite loop) if stores don't have enough items they can stock! */
	while (store->stock_num < stock && --restock_attempts)
		store_create_random(store);

	if (!restock_attempts)
		quit_fmt("Unable to (re-)stock store %d. Please report this bug", store->sidx);
}

struct owner *store_ownerbyidx(struct store *s, unsigned int idx) {
	struct owner *o;
	for (o = s->owners; o; o = o->next) {
		if (o->oidx == idx)
			return o;
	}

	notreached;
}

static struct owner *store_choose_owner(struct store *s) {
	struct owner *o;
	unsigned int n = 0;

	for (o = s->owners; o; o = o->next) {
		n++;
	}

	n = randint0(n);
	return store_ownerbyidx(s, n);
}

static struct store *parse_stores(void) {
	struct parser *p = store_parser_new();
	struct store *stores;
	/* XXX ignored */
	parse_file(p, "store");
	stores = parser_priv(p);
	parser_destroy(p);
	return stores;
}

static struct store *add_builtin_stores(struct store *stores) {
	struct store *s0, *s1, *s2;

	s0 = store_new(STORE_GENERAL);
	s1 = store_new(STORE_B_MARKET);
	s2 = store_new(STORE_HOME);

	s0->next = stores;
	s1->next = s0;
	s2->next = s1;

	return s2;
}

static void parse_owners(struct store *stores) {
	struct parser *p = store_owner_parser_new(stores);
	parse_file(p, "shop_own");
	mem_free(parser_priv(p));
	parser_destroy(p);
}

static struct store *flatten_stores(struct store *store_list) {
	struct store *s;
	struct store *stores = mem_zalloc(MAX_STORES * sizeof(*stores));

	for (s = store_list; s; s = s->next) {
		/* XXX bounds-check */
		memcpy(&stores[s->sidx], s, sizeof(*s));
	}

	while (store_list) {
		s = store_list->next;
		/* No need to free the sub-allocated memory, as this is passed on
		 * to the array of stores */
		mem_free(store_list);
		store_list = s;
	}

	return stores;
}

void store_init(void)
{
	struct store *store_list;

	store_list = parse_stores();
	store_list = add_builtin_stores(store_list);
	parse_owners(store_list);
	stores = flatten_stores(store_list);
}

void store_reset(void) {
	int i, j;
	struct store *s;

	for (i = 0; i < MAX_STORES; i++) {
		s = &stores[i];
		s->stock_num = 0;
		store_shuffle(s);
		for (j = 0; j < s->stock_size; j++)
			object_wipe(&s->stock[j]);
		if (i == STORE_HOME)
			continue;
		for (j = 0; j < 10; j++)
			store_maint(s);
	}
}

/*
 * Shuffle one of the stores.
 */
void store_shuffle(struct store *store)
{
	struct owner *o = store->owner;

	while (o == store->owner)
	    o = store_choose_owner(store);

	store->owner = o;
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
static void store_display_recalc(menu_type *m)
{
	int wid, hgt;
	region loc;

	struct store *store = current_store();

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
	if (store->sidx != STORE_HOME)
		scr_places_x[LOC_WEIGHT] -= 10;

	/* Then Y */
	scr_places_y[LOC_OWNER] = 1;
	scr_places_y[LOC_HEADER] = 3;

	/* If we are displaying help, make the height smaller */
	if (store_flags & (STORE_SHOW_HELP))
		hgt -= 3;

	scr_places_y[LOC_MORE] = hgt - 3;
	scr_places_y[LOC_AU] = hgt - 1;

	loc = m->boundary;

	/* If we're displaying the help, then put it with a line of padding */
	if (store_flags & (STORE_SHOW_HELP))
	{
		scr_places_y[LOC_HELP_CLEAR] = hgt - 1;
		scr_places_y[LOC_HELP_PROMPT] = hgt;
		loc.page_rows = -5;
	}
	else
	{
		scr_places_y[LOC_HELP_CLEAR] = hgt - 2;
		scr_places_y[LOC_HELP_PROMPT] = hgt - 1;
		loc.page_rows = -2;
	}

	menu_layout(m, &loc);
}


/*
 * Redisplay a single store entry
 */
static void store_display_entry(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	object_type *o_ptr;
	s32b x;
	odesc_detail_t desc = ODESC_PREFIX;

	char o_name[80];
	char out_val[160];
	byte colour;

	struct store *store = current_store();

	assert(store);

	/* Get the object */
	o_ptr = &store->stock[oid];

	/* Describe the object - preserving insriptions in the home */
	if (store->sidx == STORE_HOME) desc = ODESC_FULL;
	else desc = ODESC_FULL | ODESC_STORE;
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | desc);

	/* Display the object */
	c_put_str(tval_to_attr[o_ptr->tval & 0x7F], o_name, row, col);

	/* Show weights */
	colour = curs_attrs[CURS_KNOWN][(int)cursor];
	strnfmt(out_val, sizeof out_val, "%3d.%d lb", o_ptr->weight / 10, o_ptr->weight % 10);
	c_put_str(colour, out_val, row, scr_places_x[LOC_WEIGHT]);

	/* Describe an object (fully) in a store */
	if (store->sidx != STORE_HOME)
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
	struct store *store = current_store();
	owner_type *ot_ptr = store->owner;

	/* Clear screen */
	Term_clear();

	/* The "Home" is special */
	if (store->sidx == STORE_HOME)
	{
		/* Put the owner name */
		put_str("Your Home", scr_places_y[LOC_OWNER], 1);

		/* Label the object descriptions */
		put_str("Home Inventory", scr_places_y[LOC_HEADER], 1);

		/* Show weight header */
		put_str("Weight", scr_places_y[LOC_HEADER], scr_places_x[LOC_WEIGHT] + 2);
	}

	/* Normal stores */
	else
	{
		const char *store_name = f_info[FEAT_SHOP_HEAD + store->sidx].name;
		const char *owner_name = ot_ptr->name;

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
	struct store *store = current_store();
	bool is_home = (store->sidx == STORE_HOME) ? TRUE : FALSE;

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

	text_out("' examines");
	if (store_knowledge == STORE_NONE)
	{
		text_out(" and '");
		text_out_c(TERM_L_GREEN, "p");

		if (is_home) text_out("' picks up");
		else text_out("' purchases");
	}
	text_out(" the selected item. '");

	if (store_knowledge == STORE_NONE)
	{
		text_out_c(TERM_L_GREEN, "d");
		if (is_home) text_out("' drops");
		else text_out("' sells");
	}
	else
	{
		text_out_c(TERM_L_GREEN, "I");
		text_out("' inspects");
	}
	text_out(" an item from your inventory. ");

	text_out_c(TERM_L_GREEN, "ESC");
	if (store_knowledge == STORE_NONE)
	{
		text_out(" exits the building.");
	}
	else
	{
		text_out(" exits this screen.");
	}

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
	struct keypress ch;

	/* Prompt for it */
	prt(prompt, 0, 0);

	/* Get an answer */
	ch = inkey();

	/* Erase the prompt */
	prt("", 0, 0);

	if (ch.code == ESCAPE) return (FALSE);
	if (strchr("Nn", ch.code)) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Return the quantity of a given item in the pack (include quiver).
 */
static int find_inven(const object_type *o_ptr)
{
	int i, j;
	int num = 0;

	/* Similar slot? */
	for (j = 0; j < QUIVER_END; j++)
	{
		object_type *j_ptr = &p_ptr->inventory[j];

		/* Check only the inventory and the quiver */
		if (j >= INVEN_WIELD && j < QUIVER_START) continue;

		/* Require identical object types */
		if (o_ptr->kind != j_ptr->kind) continue;

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

			/* Rings, Amulets, Lights */
			case TV_RING:
			case TV_AMULET:
			case TV_LIGHT:
			{
				/* Require both items to be known */
				if (!object_is_known(o_ptr) || !object_is_known(j_ptr)) continue;

				/* Fall through */
			}

			/* Missiles */
			case TV_BOLT:
			case TV_ARROW:
			case TV_SHOT:
			{
				/* Require identical knowledge of both items */
				if (object_is_known(o_ptr) != object_is_known(j_ptr)) continue;

				/* Require identical "bonuses" */
				if (o_ptr->to_h != j_ptr->to_h) continue;
				if (o_ptr->to_d != j_ptr->to_d) continue;
				if (o_ptr->to_a != j_ptr->to_a) continue;

				/* Require identical "pval" codes */
				for (i = 0; i < MAX_PVALS; i++)
					if (o_ptr->pval[i] != j_ptr->pval[i])
						continue;

				if (o_ptr->num_pvals != j_ptr->num_pvals)
					continue;

				/* Require identical "artifact" names */
				if (o_ptr->artifact != j_ptr->artifact) continue;

				/* Require identical "ego-item" names */
				if (o_ptr->ego != j_ptr->ego) continue;

				/* Lights must have same amount of fuel */
				else if (o_ptr->timeout != j_ptr->timeout && o_ptr->tval == TV_LIGHT)
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
				if (!object_is_known(o_ptr) || !object_is_known(j_ptr)) continue;

				/* Probably okay */
				break;
			}
		}


		/* Different flags */
		if (!of_is_equal(o_ptr->flags, j_ptr->flags))
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

	struct store *store = current_store();

	if (!store) {
		msg("You cannot purchase items when not in a store.");
		return;
	}

	/* Get the actual object */
	o_ptr = &store->stock[item];

	/* Get desired object */
	object_copy_amt(i_ptr, o_ptr, amt);

	/* Ensure we have room */
	if (!inven_carry_okay(i_ptr))
	{
		msg("You cannot carry that many items.");
		return;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Extract the price for the entire stack */
	price = price_item(i_ptr, FALSE, i_ptr->number);

	if (price > p_ptr->au)
	{
		msg("You cannot afford that purchase.");
		return;
	}

	/* Spend the money */
	p_ptr->au -= price;

	/* Update the display */
	store_flags |= STORE_GOLD_CHANGE;

	/* ID objects on buy */
	object_notice_everything(i_ptr);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER | PN_SQUELCH);

	/* The object no longer belongs to the store */
	i_ptr->ident &= ~(IDENT_STORE);

	/* Message */
	if (one_in_(3)) msgt(MSG_STORE5, "%s", ONE_OF(comment_accept));
	msg("You bought %s for %ld gold.", o_name, (long)price);

	/* Erase the inscription */
	i_ptr->note = 0;

	/* Give it to the player */
	item_new = inven_carry(p_ptr, i_ptr);

	/* Message */
	object_desc(o_name, sizeof(o_name), &p_ptr->inventory[item_new],
				ODESC_PREFIX | ODESC_FULL);
	msg("You have %s (%c).", o_name, index_to_label(item_new));

	/* Hack - Reduce the number of charges in the original stack */
	if (o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF)
	{
		o_ptr->pval[DEFAULT_PVAL] -= i_ptr->pval[DEFAULT_PVAL];
	}

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Remove the bought objects from the store */
	store_item_increase(store, item, -amt);
	store_item_optimize(store, item);

	/* Store is empty */
	if (store->stock_num == 0)
	{
		int i;

		/* Shuffle */
		if (one_in_(STORE_SHUFFLE))
		{
			/* Message */
			msg("The shopkeeper retires.");

			/* Shuffle the store */
			store_shuffle(store);
			store_flags |= STORE_FRAME_CHANGE;
		}

		/* Maintain */
		else
		{
			/* Message */
			msg("The shopkeeper brings out some new stock.");
		}

		/* New inventory */
		for (i = 0; i < 10; ++i)
			store_maint(store);
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

	struct store *store = current_store();

	if (store->sidx != STORE_HOME) {
		msg("You are not currently at home.");
		return;
	}

	/* Get the actual object */
	o_ptr = &store->stock[item];

	/* Get desired object */
	object_copy_amt(&picked_item, o_ptr, amt);

	/* Ensure we have room */
	if (!inven_carry_okay(&picked_item))
	{
		msg("You cannot carry that many items.");
		return;
	}

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(o_ptr, &picked_item, amt);
	
	/* Give it to the player */
	item_new = inven_carry(p_ptr, &picked_item);

	/* Describe just the result */
	object_desc(o_name, sizeof(o_name), &p_ptr->inventory[item_new],
				ODESC_PREFIX | ODESC_FULL);
	
	/* Message */
	msg("You have %s (%c).", o_name, index_to_label(item_new));
	
	/* Handle stuff */
	handle_stuff(p_ptr);
	
	/* Remove the items from the home */
	store_item_increase(store, item, -amt);
	store_item_optimize(store, item);
	
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

	struct store *store = current_store();

	if (!store) {
		msg("You cannot purchase items when not in a store.");
		return FALSE;
	}

	/* Get the actual object */
	o_ptr = &store->stock[item];
	if (item < 0) return FALSE;

	/* Clear all current messages */
	msg_flag = FALSE;
	prt("", 0, 0);

	if (store->sidx == STORE_HOME) {
		amt = o_ptr->number;
	} else {
		/* Price of one */
		price = price_item(o_ptr, FALSE, 1);

		/* Check if the player can afford any at all */
		if ((u32b)p_ptr->au < (u32b)price)
		{
			/* Tell the user */
			msg("You do not have enough gold for this item.");

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
	if (!object_flavor_is_aware(o_ptr))
		num = 0;
	else
		num = find_inven(o_ptr);

	strnfmt(o_name, sizeof o_name, "%s how many%s? (max %d) ",
	        (store->sidx == STORE_HOME) ? "Take" : "Buy",
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
		msg("You cannot carry that many items.");
		return FALSE;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Attempt to buy it */
	if (store->sidx != STORE_HOME)
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

		cmd_insert(CMD_BUY);
		cmd_set_arg_choice(cmd_get_top(), 0, item);
		cmd_set_arg_number(cmd_get_top(), 1, amt);
	}

	/* Home is much easier */
	else
	{
		cmd_insert(CMD_RETRIEVE);
		cmd_set_arg_choice(cmd_get_top(), 0, item);
		cmd_set_arg_number(cmd_get_top(), 1, amt);
	}

	/* Not kicked out */
	return TRUE;
}



/*
 * Determine if the current store will purchase the given object
 */
static bool store_will_buy_tester(const object_type *o_ptr)
{
	struct store *store = current_store();
	if (store)
		return store_will_buy(store, o_ptr);

	return FALSE;
}

/*
 * Sell an item to the current store.
 */
void do_cmd_sell(cmd_code code, cmd_arg args[])
{
	int item = args[0].item;
	int amt = args[1].number;
	object_type sold_item;
	struct store *store = current_store();
	int price, dummy, value;
	char o_name[120];

	/* Get the item */
	object_type *o_ptr = object_from_item_idx(item);

	/* Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr->flags)) {
		msg("Hmmm, it seems to be cursed.");
		return;
	}

	/* Check we are somewhere we can sell the items. */
	if (!store) {
		msg("You cannot sell items when not in a store.");
		return;
	}

	/* Check the store wants the items being sold */
	if (!store_will_buy(store, o_ptr)) {
		msg("I do not wish to purchase this item.");
		return;
	}

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(&sold_item, o_ptr, amt);

	/* Check if the store has space for the items */
	if (!store_check_num(store, &sold_item))
	{
		msg("I have not the room in my store to keep it.");
		return;
	}

	price = price_item(&sold_item, TRUE, amt);

	/* Get some money */
	p_ptr->au += price;

	/* Update the display */
	store_flags |= STORE_GOLD_CHANGE;

	/* Update the auto-history if selling an artifact that was previously un-IDed. (Ouch!) */
	if (o_ptr->artifact)
		history_add_artifact(o_ptr->artifact, TRUE, TRUE);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER | PN_SORT_QUIVER);

	/* Redraw stuff */
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);

	/* Get the "apparent" value */
	dummy = object_value(&sold_item, amt, FALSE);
/*	msg("Dummy is %d", dummy); */

	/* Identify original object */
	object_notice_everything(o_ptr);

	/* Take a new copy of the now known-about object. */
	object_copy_amt(&sold_item, o_ptr, amt);

	/* The item belongs to the store now */
	sold_item.ident |= IDENT_STORE;

	/*
	* Hack -- Allocate charges between those wands, staves, or rods
	* sold and retained, unless all are being sold.
	 */
	distribute_charges(o_ptr, &sold_item, amt);

	/* Get the "actual" value */
	value = object_value(&sold_item, amt, FALSE);
/*	msg("Value is %d", value); */

	/* Get the description all over again */
	object_desc(o_name, sizeof(o_name), &sold_item, ODESC_PREFIX | ODESC_FULL);

	/* Describe the result (in message buffer) */
	msg("You sold %s (%c) for %ld gold.",
		o_name, index_to_label(item), (long)price);

	/* Analyze the prices (and comment verbally) */
	purchase_analyze(price, value, dummy);

	/* Set squelch flag */
	p_ptr->notice |= PN_SQUELCH;

	/* Take the object from the player */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* The store gets that (known) object */
	store_carry(store, &sold_item);

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
	struct store *store = current_store();
	char o_name[120];

	/* Check we are somewhere we can stash items. */
	if (store->sidx != STORE_HOME)
	{
		msg("You are not in your home.");
		return;
	}

	/* Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr->flags))
	{
		msg("Hmmm, it seems to be cursed.");
		return;
	}	

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(&dropped_item, o_ptr, amt);

	if (!store_check_num(store, &dropped_item))
	{
		msg("Your home is full.");
		return;
	}

	/* Distribute charges of wands/staves/rods */
	distribute_charges(o_ptr, &dropped_item, amt);
	
	/* Describe */
	object_desc(o_name, sizeof(o_name), &dropped_item, ODESC_PREFIX | ODESC_FULL);

	/* Message */
	msg("You drop %s (%c).", o_name, index_to_label(item));
	
	/* Take it from the players inventory */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);
	
	/* Handle stuff */
	handle_stuff(p_ptr);
	
	/* Let the home carry it */
	home_carry(&dropped_item);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Sell an object, or drop if it we're in the home.
 */
static bool store_sell(void)
{
	int amt;
	int item;
	int get_mode = USE_EQUIP | USE_INVEN | USE_FLOOR;

	object_type *o_ptr;
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	char o_name[120];


	const char *reject = "You have nothing that I want. ";
	const char *prompt = "Sell which item? ";

	struct store *store = current_store();

	if (!store) {
		msg("You cannot sell items when not in a store.");
		return FALSE;
	}

	/* Clear all current messages */
	msg_flag = FALSE;
	prt("", 0, 0);

	if (store->sidx == STORE_HOME) {
		prompt = "Drop which item? ";
	} else {
		item_tester_hook = store_will_buy_tester;
		get_mode |= SHOW_PRICES;
	}

	/* Get an item */
	p_ptr->command_wrk = USE_INVEN;

	if (!get_item(&item, prompt, reject, CMD_DROP, get_mode))
		return FALSE;

	/* Get the item */
	o_ptr = object_from_item_idx(item);

	/* Hack -- Cannot remove cursed objects */
	if ((item >= INVEN_WIELD) && cursed_p(o_ptr->flags))
	{
		/* Oops */
		msg("Hmmm, it seems to be cursed.");

		/* Nope */
		return FALSE;
	}

	/* Get a quantity */
	amt = get_quantity(NULL, o_ptr->number);

	/* Allow user abort */
	if (amt <= 0) return FALSE;

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(i_ptr, object_from_item_idx(item), amt);

	if (!store_check_num(store, i_ptr))
	{
		if (store->sidx == STORE_HOME)
			msg("Your home is full.");

		else
			msg("I have not the room in my store to keep it.");

		return FALSE;
	}

	/* Get a full description */
	object_desc(o_name, sizeof(o_name), i_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Real store */
	if (store->sidx != STORE_HOME)
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
			return FALSE;
		}

		screen_load();

		cmd_insert(CMD_SELL);
		cmd_set_arg_item(cmd_get_top(), 0, item);
		cmd_set_arg_number(cmd_get_top(), 1, amt);
	}

	/* Player is at home */
	else
	{
		cmd_insert(CMD_STASH);
		cmd_set_arg_item(cmd_get_top(), 0, item);
		cmd_set_arg_number(cmd_get_top(), 1, amt);
	}

	return TRUE;
}


/*
 * Examine an item in a store
 */
static void store_examine(int item)
{
	struct store *store = current_store();
	object_type *o_ptr;

	char header[120];

	textblock *tb;
	region area = { 0, 0, 0, 0 };

	if (item < 0) return;

	/* Get the actual object */
	o_ptr = &store->stock[item];

	/* Show full info in most stores, but normal info in player home */
	tb = object_info(o_ptr, (store->sidx != STORE_HOME) ? OINFO_FULL : OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

	textui_textblock_show(tb, area, header);
	textblock_free(tb);

	/* Hack -- Browse book, then prompt for a command */
	if (o_ptr->tval == p_ptr->class->spell_book)
		textui_book_browse(o_ptr);
}


static void store_menu_set_selections(menu_type *menu, bool knowledge_menu)
{
	if (knowledge_menu)
	{
		if (OPT(rogue_like_commands))
		{
			/* These two can't intersect! */
			menu->cmd_keys = "?Ieilx";
			menu->selections = "abcdfghjkmnopqrstuvwyz134567";
		}
		/* Original */
		else
		{
			/* These two can't intersect! */
			menu->cmd_keys = "?Ieil";
			menu->selections = "abcdfghjkmnopqrstuvwxyz13456";
		}
	}
	else
	{
		/* Roguelike */
		if (OPT(rogue_like_commands))
		{
			/* These two can't intersect! */
			menu->cmd_keys = "\x04\x05\x10?={}~CEIPTdegilpswx"; /* \x10 = ^p , \x04 = ^D, \x05 = ^E */
			menu->selections = "abcfmnoqrtuvyz13456790ABDFGH";
		}

		/* Original */
		else
		{
			/* These two can't intersect! */
			menu->cmd_keys = "\x05\x010?={}~CEIbdegiklpstwx"; /* \x05 = ^E, \x10 = ^p */
			menu->selections = "acfhjmnoqruvyz13456790ABDFGH";
		}
	}
}

static void store_menu_recalc(menu_type *m)
{
	struct store *store = current_store();
	menu_setpriv(m, store->stock_num, store->stock);
}

/*
 * Process a command in a store
 *
 * Note that we must allow the use of a few "special" commands in the stores
 * which are not allowed in the dungeon, and we must disable some commands
 * which are allowed in the dungeon but not in the stores, to prevent chaos.
 */
static bool store_process_command_key(struct keypress kp)
{
	int cmd = 0;

	/* Process the keycode */
	switch (kp.code) {
		case 'T': /* roguelike */
		case 't': cmd = CMD_TAKEOFF; break;

		case KTRL('D'): /* roguelike */
		case 'k': textui_cmd_destroy(); break;

		case 'P': /* roguelike */
		case 'b': textui_spell_browse(); break;

		case '~': textui_browse_knowledge(); break;
		case 'I': textui_obj_examine(); break;
		case 'w': cmd = CMD_WIELD; break;
		case '{': cmd = CMD_INSCRIBE; break;
		case '}': cmd = CMD_UNINSCRIBE; break;

		case 'e': do_cmd_equip(); break;
		case 'i': do_cmd_inven(); break;
		case KTRL('E'): toggle_inven_equip(); break;
		case 'C': do_cmd_change_name(); break;
		case KTRL('P'): do_cmd_messages(); break;
		case ')': do_cmd_save_screen(); break;

		default: return FALSE;
	}

	if (cmd)
		cmd_insert_repeated(cmd, 0);

	return TRUE;
}


/*
 *
 */
static bool store_menu_handle(menu_type *m, const ui_event *event, int oid)
{
	bool processed = TRUE;

	if (event->type == EVT_SELECT)
	{
		/* Nothing for now, except "handle" the event */
		return TRUE;
		/* In future, maybe we want a display a list of what you can do. */
	}
	else if (event->type == EVT_KBRD)
	{
		bool storechange = FALSE;

		switch (event->key.code) {
			case 's':
			case 'd': storechange = store_sell(); break;
			case 'p':
			case 'g': storechange = store_purchase(oid); break;
			case 'l':
			case 'x': store_examine(oid); break;

			/* XXX redraw functionality should be another menu_iter handler */
			case KTRL('R'): {
				Term_clear();
				store_flags |= (STORE_FRAME_CHANGE | STORE_GOLD_CHANGE);
				break;
			}

			case '?': {
				/* Toggle help */
				if (store_flags & STORE_SHOW_HELP)
					store_flags &= ~(STORE_SHOW_HELP);
				else
					store_flags |= STORE_SHOW_HELP;

				/* Redisplay */
				store_flags |= STORE_INIT_CHANGE;
				break;
			}

			case '=': {
				do_cmd_options();
				store_menu_set_selections(m, FALSE);
				break;
			}

			default:
				processed = store_process_command_key(event->key);
		}

		/* Let the game handle any core commands (equipping, etc) */
		process_command(CMD_STORE, TRUE);

		if (storechange)
			store_menu_recalc(m);

		if (processed) {
			event_signal(EVENT_INVENTORY);
			event_signal(EVENT_EQUIPMENT);
		}

		/* Notice and handle stuff */
		notice_stuff(p_ptr);
		handle_stuff(p_ptr);

		/* Display the store */
		store_display_recalc(m);
		store_menu_recalc(m);
		store_redraw();

		return processed;
	}

	return FALSE;
}

static region store_menu_region = { 1, 4, -1, -2 };
static const menu_iter store_menu =
{
	NULL,
	NULL,
	store_display_entry,
	store_menu_handle,
	NULL
};

static const menu_iter store_know_menu =
{
	NULL,
	NULL,
	store_display_entry,
	NULL,
	NULL
};


/*
 * Display contents of a store from knowledge menu
 *
 * The only allowed actions are 'I' to inspect an item
 */
void do_cmd_store_knowledge(void)
{
	menu_type menu;

	screen_save();
	clear_from(0);

	/* Init the menu structure */
	menu_init(&menu, MN_SKIN_SCROLL, &store_menu);
	menu_layout(&menu, &store_menu_region);

	/* Calculate the positions of things and redraw */
	store_menu_set_selections(&menu, TRUE);
	store_flags = STORE_INIT_CHANGE;
	store_display_recalc(&menu);
	store_menu_recalc(&menu);
	store_redraw();

	menu_select(&menu, 0, FALSE);

	/* Flush messages XXX XXX XXX */
	message_flush();

	screen_load();
}




/*
 * Enter a store, and interact with it.
 */
void do_cmd_store(cmd_code code, cmd_arg args[])
{
	/* Take note of the store number from the terrain feature */
	struct store *store = current_store();
	menu_type menu;

	/* Verify that there is a store */
	if (!store) {
		msg("You see no store here.");
		return;
	}

	/* Check if we can enter the store */
	if (OPT(birth_no_stores)) {
		msg("The doors are locked.");
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



	/*** Display ***/

	/* Save current screen (ie. dungeon) */
	screen_save();


	/*** Inventory display ***/

	/* Wipe the menu and set it up */
	menu_init(&menu, MN_SKIN_SCROLL, &store_menu);
	menu_layout(&menu, &store_menu_region);

	store_menu_set_selections(&menu, FALSE);
	store_flags = STORE_INIT_CHANGE;
	store_display_recalc(&menu);
	store_menu_recalc(&menu);
	store_redraw();

	/* Say a friendly hello. */
	if (store->sidx != STORE_HOME)
		prt_welcome(store->owner);

	msg_flag = FALSE;
	menu_select(&menu, 0, FALSE);
	msg_flag = FALSE;

	/* Switch back to the normal game view. */
	event_signal(EVENT_LEAVE_STORE);
	event_signal(EVENT_ENTER_GAME);

	/* Take a turn */
	p_ptr->energy_use = 100;


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

