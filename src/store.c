/*
 * File: store.c
 * Purpose: Store stocking and UI
 *
 * Copyright (c) 1997 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 2007 Andi Sidwell, who rewrote a fair portion
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
#include "hint.h"
#include "history.h"
#include "init.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-info.h"
#include "obj-make.h"
#include "obj-tval.h"
#include "obj-tvalsval.h"
#include "obj-util.h"
#include "spells.h"
#include "squelch.h"
#include "store.h"
#include "target.h"
#include "z-debug.h"

/*** Constants and definitions ***/

/*
 * Array[MAX_STORES] of stores
 */
struct store *stores;

/*
 * The hints array
 */
struct hint *hints;


/* Some local constants */
#define STORE_OBJ_LEVEL 5       /* Magic Level for normal stores */


/* Return the store instance at the given location */
struct store *store_at(struct chunk *c, int y, int x)
{
	if (square_isshop(c, player->py, player->px))
		return &stores[square_shopnum(cave, player->py, player->px)];

	return NULL;
}


/*
 * Create a new store.
 */
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
		mem_free(store->always_table);
		mem_free(store->normal_table);

		for (o = store->owners; o; o = next) {
			next = o->next;
			string_free(o->name);
			mem_free(o);
		}

		string_free((void *)store->name);
	}
	mem_free(stores);
}


/*** Edit file parsing ***/

/** store.txt **/

static enum parser_error parse_store(struct parser *p) {
	struct store *h = parser_priv(p);
	struct store *s;
	unsigned int idx = parser_getuint(p, "index") - 1;

	if (idx > STORE_HOME)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	s = store_new(parser_getuint(p, "index") - 1);
	s->name = string_make(parser_getstr(p, "name"));
	s->next = h;
	parser_setpriv(p, s);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slots(struct parser *p) {
	struct store *s = parser_priv(p);
	s->normal_stock_min = parser_getuint(p, "min");
	s->normal_stock_max = parser_getuint(p, "max");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_turnover(struct parser *p) {
	struct store *s = parser_priv(p);
	s->turnover = parser_getuint(p, "turnover");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_normal(struct parser *p) {
	struct store *s = parser_priv(p);
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	int sval = lookup_sval(tval, parser_getsym(p, "sval"));

	object_kind *kind = lookup_kind(tval, sval);
	if (!kind)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	/* Expand if necessary */
	if (!s->normal_num) {
		s->normal_size = 16;
		s->normal_table = mem_zalloc(s->normal_size * sizeof *s->normal_table);
	} else if (s->normal_num >= s->normal_size) {
		s->normal_size += 8; 
		s->normal_table = mem_realloc(s->normal_table, s->normal_size * sizeof *s->normal_table);
	}

	s->normal_table[s->normal_num++] = kind;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_always(struct parser *p) {
	struct store *s = parser_priv(p);
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	int sval = lookup_sval(tval, parser_getsym(p, "sval"));

	object_kind *kind = lookup_kind(tval, sval);
	if (!kind)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	/* Expand if necessary */
	if (!s->always_num) {
		s->always_size = 8;
		s->always_table = mem_zalloc(s->always_size * sizeof *s->always_table);
	} else if (s->always_num >= s->always_size) {
		s->always_size += 8; 
		s->always_table = mem_realloc(s->always_table, s->always_size * sizeof *s->always_table);
	}

	s->always_table[s->always_num++] = kind;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_owner(struct parser *p) {
	struct store *s = parser_priv(p);
	unsigned int maxcost = parser_getuint(p, "purse");
	char *name = string_make(parser_getstr(p, "name"));
	struct owner *o;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	o = mem_zalloc(sizeof *o);
	o->oidx = (s->owners ? s->owners->oidx + 1 : 0);
	o->next = s->owners;
	o->name = name;
	o->max_cost = maxcost;
	s->owners = o;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_stores(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "store uint index str name", parse_store);
	parser_reg(p, "owner uint purse str name", parse_owner);
	parser_reg(p, "slots uint min uint max", parse_slots);
	parser_reg(p, "turnover uint turnover", parse_turnover);
	parser_reg(p, "normal sym tval sym sval", parse_normal);
	parser_reg(p, "always sym tval sym sval", parse_always);
	return p;
}

static errr run_parse_stores(struct parser *p) {
	return parse_file(p, "store");
}

static errr finish_parse_stores(struct parser *p) {
	stores = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static struct file_parser store_parser = {
	"store",
	init_parse_stores,
	run_parse_stores,
	finish_parse_stores,
	NULL
};


/*** Other init stuff ***/

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
	event_signal_string(EVENT_INITSTATUS, "Initialising stores...");
	if (run_parser(&store_parser)) quit("Can't initialise stores");
	stores = flatten_stores(stores);
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
		for (j = 0; j < 10; j++) store_maint(s);
	}
}




/**
 * Check if a given item kind is an always-stocked item.
 */
static bool store_is_staple(struct store *s, object_kind *k) {
	size_t i;

	assert(s);
	assert(k);

	for (i = 0; i < s->always_num; i++) {
		object_kind *l = s->always_table[i];
		if (k == l)
			return TRUE;
	}

	return FALSE;
}

/**
 * Check if a given item kind is an always-stocked or sometimes-stocked item.
 */
static bool store_can_carry(struct store *store, struct object_kind *kind) {
	size_t i;

	for (i = 0; i < store->normal_num; i++) {
		if (store->normal_table[i] == kind)
			return TRUE;
	}

	return store_is_staple(store, kind);
}




/*** Utilities ***/

/* Randomly select one of the entries in an array */
#define ONE_OF(x)	x[randint0(N_ELEMENTS(x))]


/*** Flavour text stuff ***/

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
			/* Analyze the types */
			switch (o_ptr->tval)
			{
				case TV_LIGHT:
				case TV_FOOD:
				case TV_MUSHROOM:
				case TV_FLASK:
				case TV_DIGGING:
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

	/* Ignore "worthless" items */
	if (object_value(o_ptr, 1, FALSE) <= 0) return (FALSE);

	/* Assume okay */
	return (TRUE);
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
int price_item(struct store *store, const object_type *o_ptr, bool store_buying, int qty)
{
	int adjust;
	int price;
	owner_type *ot_ptr;

	if (!store) return 0L;

	ot_ptr = store->owner;


	/* Get the value of the stack of wands, or a single item */
	if (tval_can_have_charges(o_ptr))
		price = object_value(o_ptr, qty, FALSE);
	else
		price = object_value(o_ptr, 1, FALSE);

	/* Worthless items */
	if (price <= 0) return (0L);


	/* Add in the charisma factor */
	if (store->sidx == STORE_B_MARKET)
		adjust = 150;
	else
		adjust = 100;


	/* Shop is buying */
	if (store_buying)
	{
		/* Set the factor */
		adjust = 100 + (100 - adjust);
		if (adjust > 100) adjust = 100;

		/* Shops now pay 2/3 of true value */
		price = price * 2 / 3;

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
	if (!tval_can_have_charges(o_ptr))
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
		case TV_MUSHROOM:
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

		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		{
			if (cost <= 5L)
				size = randint1(2) * 20;         /* 20-40 in 20s */
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
	o_ptr->number = (total >= MAX_STACK_SIZE) ? MAX_STACK_SIZE - 1 : total;

	/* Hack -- if rods are stacking, add the charging timeouts */
	if (tval_can_have_timeout(o_ptr))
		o_ptr->timeout += j_ptr->timeout;

	/* Hack -- if wands/staves are stacking, combine the charges */
	if (tval_can_have_charges(o_ptr))
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
bool store_check_num(struct store *store, const object_type *o_ptr)
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
		if ((o_ptr->tval == player->class->spell_book) &&
		    (j_ptr->tval != player->class->spell_book)) break;
		if ((j_ptr->tval == player->class->spell_book) &&
		    (o_ptr->tval != player->class->spell_book)) continue;

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
	if (tval_is_light(o_ptr)) {
		if (!of_has(o_ptr->flags, OF_NO_FUEL))
		{
			if (o_ptr->sval == SV_LIGHT_TORCH)
				o_ptr->timeout = DEFAULT_TORCH;

			else if (o_ptr->sval == SV_LIGHT_LANTERN)
				o_ptr->timeout = DEFAULT_LAMP;
		}
	}
	else if (tval_can_have_timeout(o_ptr)) {
		o_ptr->timeout = 0;
	}
	else if (tval_can_have_charges(o_ptr)) {
		/* If the store can stock this item kind, we recharge */
		if (store_can_carry(store, o_ptr->kind)) {
			int charges = 0;

			/* Calculate the recharged number of charges */
			for (i = 0; i < o_ptr->number; i++)
				charges += randcalc(kind->charge, 0, RANDOMISE);

			/* Use recharged value only if greater */
			if (charges > o_ptr->pval)
				o_ptr->pval = charges;
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
		if (tval_is_ammo(o_ptr)) {
			/* 50% of the time, destroy the entire stack */
			if (randint0(100) < 50 || num < 10)
				num = o_ptr->number;

			/* 50% of the time, reduce the size to a multiple of 5 */
			else
				num = randint1(num / 5) * 5 + (num % 5);
		}
		else {
			/* 50% of the time, destroy a single object */
			if (randint0(100) < 50) num = 1;

			/* 25% of the time, destroy half the objects */
			else if (randint0(100) < 50) num = (num + 1) / 2;

			/* 25% of the time, destroy all objects */
			else num = o_ptr->number;

			/* Hack -- decrement the total charges of staves and wands. */
			if (tval_can_have_charges(o_ptr))
			{
				o_ptr->pval -= num * o_ptr->pval / o_ptr->number;
			}
		}
	}

	if (o_ptr->artifact)
		history_lose_artifact(o_ptr->artifact);

	/* Delete the item */
	store_item_increase(store, what, -num);
	store_item_optimize(store, what);
}

/**
 * Find a given object kind in the store.
 */
static object_type *store_find_kind(struct store *s, object_kind *k) {
	int slot;

	assert(s);
	assert(k);

	/* Check if it's already in stock */
	for (slot = 0; slot < s->stock_num; slot++) {
		if (s->stock[slot].kind == k && !s->stock[slot].ego) {
			return &s->stock[slot];
		}
	}

	return NULL;
}


/*
 * Delete a random object from store 'st', or, if it is a stack, perhaps only
 * partially delete it.
 *
 * This function is used when store maintainance occurs, and is designed to
 * imitate non-PC purchasers making purchases from the store.
 *
 * The reason this doesn't check for "staple" items and refuse to
 * delete them is that a store could conceviably have two stacks of a
 * single staple item, in which case, you could have a store which had
 * more stacks than staple items, but all stacks are staple items.
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
	return store->normal_table[randint0(store->normal_num)];
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
		min_level = player->max_depth + 5;
		max_level = player->max_depth + 20;
	} else {
		min_level = 1;
		max_level = STORE_OBJ_LEVEL + MAX(player->max_depth - 20, 0);
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
			kind = get_obj_num(level, FALSE, 0);
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
		apply_magic(i_ptr, level, FALSE, FALSE, FALSE, FALSE);

		/* Reject if item is 'damaged' (i.e. negative mods) */
		if (tval_is_weapon(i_ptr)) {
			if ((i_ptr->to_h < 0) || (i_ptr->to_d < 0))
				continue;
		}
		else if (tval_is_armor(i_ptr)) {
			if (i_ptr->to_a < 0) continue;
		}

		/* The object is "known" and belongs to a store */
		object_know_all_but_flavor(i_ptr);
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
	object_know_all_but_flavor(&object);
	object.origin = ORIGIN_STORE;

	/* Attempt to carry the object */
	return store_carry(store, &object);
}


/*
 * Maintain the inventory at the stores.
 */
void store_maint(struct store *s)
{
	int j;

	/* Ignore home */
	if (s->sidx == STORE_HOME)
		return;

	/* Destroy crappy black market items */
	if (s->sidx == STORE_B_MARKET) {
		for (j = s->stock_num - 1; j >= 0; j--) {
			object_type *o_ptr = &s->stock[j];
			if (!black_market_ok(o_ptr)) {
				store_item_increase(s, j, 0 - o_ptr->number);
				store_item_optimize(s, j);
			}
		}
	}

	/* We want to make sure stores have staple items. If there's
	 * turnover, we also want to delete a few items, and add a few
	 * items.
	 *
	 * If we create staple items, then delete items, then create new
	 * items, we are stuck with one of three choices:
	 * 1. We can risk deleting staple items, and not having any left.
	 * 2. We can refuse to delete staple items, and risk having that
	 * become an infinite loop.
	 * 3. We can do a ton of extra bookkeeping to make sure we delete
	 * staple items only if there's duplicates of them.
	 *
	 * What if we change the order? First sell a handful of random items,
	 * then create any missing staples, then create new items. This
	 * has two tests for s->turnover, but simplifies everything else
	 * dramatically.
	 */

	if (s->turnover) {
		int restock_attempts = 100000;
		int stock = s->stock_num - randint1(s->turnover);

		/* We'll end up adding staples for sure, maybe plus other
		 * items. It's fine if we sell out completely, though, if
		 * turnover is high. The cap doesn't include always_num,
		 * because otherwise the addition of missing staples could
		 * put us over (if the store was full of player-sold loot).
		 */
		int min = 0;
		int max = s->normal_stock_max;

		if (stock < min) stock = min;
		if (stock > max) stock = max;

		/* Destroy random objects until only "stock" slots are left */
		while (s->stock_num > stock && --restock_attempts)
			store_delete_random(s);

		if (!restock_attempts)
			quit_fmt("Unable to (de-)stock store %d. Please report this bug", s->sidx + 1);
	}

	/* Ensure staples are created */
	if (s->always_num) {
		size_t i;
		for (i = 0; i < s->always_num; i++) {
			object_kind *k = s->always_table[i];
			object_type *o = store_find_kind(s, k);
			if (o) {
				/* ensure a full stack */
				o->number = MAX_STACK_SIZE - 1;
			} else {
				/* Now create the item */
				int slot = store_create_item(s, k);
				struct object *o = &s->stock[slot];
				o->number = MAX_STACK_SIZE - 1;
			}
		}
	}

	if (s->turnover) {
		int restock_attempts = 100000;
		int stock = s->stock_num + randint1(s->turnover);

		/* Now that the staples exist, we want to add more
		 * items, at least enough to get us to normal_stock_min
		 * items that aren't necessarily staples.
		 */

		int min = s->normal_stock_min + s->always_num;
		int max = s->normal_stock_max + s->always_num;

		/* Buy a few items */

		/* Keep stock between specified min and max slots */
		if (stock > max) stock = max;
		if (stock < min) stock = min;

		/* For the rest, we just choose items randomlyish */
		/* The (huge) restock_attempts will only go to zero (otherwise
		 * infinite loop) if stores don't have enough items they can stock! */
		while (s->stock_num < stock && --restock_attempts)
			store_create_random(s);


		if (!restock_attempts)
			quit_fmt("Unable to (re-)stock store %d. Please report this bug", s->sidx + 1);
	}
}

/** Owner stuff **/

struct owner *store_ownerbyidx(struct store *s, unsigned int idx) {
	struct owner *o;
	for (o = s->owners; o; o = o->next) {
		if (o->oidx == idx)
			return o;
	}

	quit_fmt("Bad call to store_ownerbyidx: idx is %d\n", idx);
	return 0; /* Needed to avoid Windows compiler warning */
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




/*** Higher-level code ***/

/*
 * Return the quantity of a given item in the pack (include quiver).
 */
int find_inven(const object_type *o_ptr)
{
	int i, j;
	int num = 0;

	/* Similar slot? */
	for (j = 0; j < player->max_gear; j++)
	{
		object_type *j_ptr = &player->gear[j];

		/* Check only the inventory and the quiver */
		if (item_is_equipped(player, j)) continue;

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
			case TV_MUSHROOM:
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

				/* Require identical modifiers */
				for (i = 0; i < OBJ_MOD_MAX; i++)
					if (o_ptr->modifiers[i] != j_ptr->modifiers[i])
						continue;

				/* Require identical "artifact" names */
				if (o_ptr->artifact != j_ptr->artifact) continue;

				/* Require identical "ego-item" names */
				if (o_ptr->ego != j_ptr->ego) continue;

				/* Lights must have same amount of fuel */
				else if (o_ptr->timeout != j_ptr->timeout && tval_is_light(o_ptr))
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
void do_cmd_buy(struct command *cmd)
{
	int item;
	int amt;

	object_type *o_ptr;	
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	char o_name[80];
	int price, item_new;

	struct store *store = store_at(cave, player->py, player->px);

	if (!store) {
		msg("You cannot purchase items when not in a store.");
		return;
	}

	/* Get arguments */
	/* XXX-AS fill this out, split into cmd-store.c */
	if (cmd_get_arg_choice(cmd, "item", &item) != CMD_OK)
		return;

	if (cmd_get_arg_number(cmd, "quantity", &amt) != CMD_OK)
		return;


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
	price = price_item(store, i_ptr, FALSE, i_ptr->number);

	if (price > player->au)
	{
		msg("You cannot afford that purchase.");
		return;
	}

	/* Spend the money */
	player->au -= price;

	/* Completely ID objects on buy */
	object_flavor_aware(i_ptr);

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE | PN_SQUELCH);

	/* Message */
	if (one_in_(3)) msgt(MSG_STORE5, "%s", ONE_OF(comment_accept));
	msg("You bought %s for %ld gold.", o_name, (long)price);

	/* Erase the inscription */
	i_ptr->note = 0;

	/* Give it to the player */
	item_new = inven_carry(player, i_ptr);

	/* Message */
	object_desc(o_name, sizeof(o_name), &player->gear[item_new],
				ODESC_PREFIX | ODESC_FULL);
	msg("You have %s (%c).", o_name, index_to_label(item_new));

	/* Hack - Reduce the number of charges in the original stack */
	if (tval_can_have_charges(o_ptr))
	{
		o_ptr->pval -= i_ptr->pval;
	}

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Remove the bought objects from the store if it's not a staple */
	if (!store_is_staple(store, store->stock[item].kind)) {
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
	}

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Retrieve the item with the given index from the home's inventory.
 */
void do_cmd_retrieve(struct command *cmd)
{
	int item, amt;

	object_type *o_ptr;	
	object_type picked_item;
	char o_name[80];
	int item_new;

	struct store *store = store_at(cave, player->py, player->px);

	if (store->sidx != STORE_HOME) {
		msg("You are not currently at home.");
		return;
	}

	/* Get arguments */
	/* XXX-AS fill this out, split into cmd-store.c */
	if (cmd_get_arg_choice(cmd, "item", &item) != CMD_OK)
		return;

	if (cmd_get_arg_number(cmd, "quantity", &amt) != CMD_OK)
		return;

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
	item_new = inven_carry(player, &picked_item);

	/* Describe just the result */
	object_desc(o_name, sizeof(o_name), &player->gear[item_new],
				ODESC_PREFIX | ODESC_FULL);
	
	/* Message */
	msg("You have %s (%c).", o_name, index_to_label(item_new));
	
	/* Handle stuff */
	handle_stuff(player->upkeep);
	
	/* Remove the items from the home */
	store_item_increase(store, item, -amt);
	store_item_optimize(store, item);
	
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/*
 * Determine if the current store will purchase the given object
 */
bool store_will_buy_tester(const object_type *o_ptr)
{
	struct store *store = store_at(cave, player->py, player->px);
	if (!store) return FALSE;

	if (OPT(birth_no_selling)) {
		if (tval_can_have_charges(o_ptr)) {
			if (!store_can_carry(store, o_ptr->kind) && object_is_known(o_ptr))
				return FALSE;
		}
		else {
			if (object_is_known(o_ptr))
				return FALSE;
		}
	}

	return store_will_buy(store, o_ptr);
}

/*
 * Sell an item to the current store.
 */
void do_cmd_sell(struct command *cmd)
{
	int item, amt;
	object_type sold_item;
	struct store *store = store_at(cave, player->py, player->px);
	int price, dummy, value;
	char o_name[120];

	object_type *o_ptr;

	/* Get arguments */
	/* XXX-AS fill this out, split into cmd-store.c */
	if (cmd_get_arg_item(cmd, "item", &item) != CMD_OK)
		return;

	if (cmd_get_arg_number(cmd, "quantity", &amt) != CMD_OK)
		return;

	o_ptr = object_from_item_idx(item);

	/* Cannot remove cursed objects */
	if (item_is_equipped(player, item) && cursed_p(o_ptr->flags)) {
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

	price = price_item(store, &sold_item, TRUE, amt);

	/* Get some money */
	player->au += price;

	/* Update the auto-history if selling an artifact that was previously un-IDed. (Ouch!) */
	if (o_ptr->artifact)
		history_add_artifact(o_ptr->artifact, TRUE, TRUE);

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Get the "apparent" value */
	dummy = object_value(&sold_item, amt, FALSE);

	/* Identify original object */
	object_notice_everything(o_ptr);

	/* Take a new copy of the now known-about object. */
	object_copy_amt(&sold_item, o_ptr, amt);

	/*
	* Hack -- Allocate charges between those wands, staves, or rods
	* sold and retained, unless all are being sold.
	 */
	distribute_charges(o_ptr, &sold_item, amt);

	/* Get the "actual" value */
	value = object_value(&sold_item, amt, FALSE);

	/* Get the description all over again */
	object_desc(o_name, sizeof(o_name), &sold_item, ODESC_PREFIX | ODESC_FULL);

	/* Describe the result (in message buffer) */
	if (OPT(birth_no_selling)) {
		msg("You had %s (%c).", o_name, index_to_label(item));
	} else {
		msg("You sold %s (%c) for %ld gold.", o_name, index_to_label(item), (long)price);

		/* Analyze the prices (and comment verbally) */
		purchase_analyze(price, value, dummy);
	}

	/* Set squelch flag */
	player->upkeep->notice |= PN_SQUELCH;

	/* Take the object from the player */
	inven_item_increase(item, -amt);
	inven_item_optimize(item);

	/* Notice if pack items need to be combined or reordered */
	notice_stuff(player->upkeep);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* The store gets that (known) object */
	store_carry(store, &sold_item);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/*
 * Stash an item in the home.
 */
void do_cmd_stash(struct command *cmd)
{
	int amt;
	object_type dropped_item;
	struct store *store = store_at(cave, player->py, player->px);
	char o_name[120];

	int item;
	object_type *o_ptr;

	if (cmd_get_arg_item(cmd, "item", &item))
		return;

	o_ptr = object_from_item_idx(item);

	if (cmd_get_quantity(cmd, "quantity", &amt, o_ptr->number) != CMD_OK)
		return;

	/* Check we are somewhere we can stash items. */
	if (store->sidx != STORE_HOME)
	{
		msg("You are not in your home.");
		return;
	}

	/* Cannot remove cursed objects */
	if (item_is_equipped(player, item) && cursed_p(o_ptr->flags))
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
	handle_stuff(player->upkeep);
	
	/* Let the home carry it */
	home_carry(&dropped_item);

	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}
