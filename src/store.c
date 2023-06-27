/**
 * \file store.c
 * \brief Store stocking
 *
 * Copyright (c) 1997 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 2007 Andi Sidwell
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
#include "game-world.h"
#include "hint.h"
#include "init.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-spell.h"
#include "store.h"
#include "target.h"
#include "debug.h"


static void store_maint(struct store *s);

/**
 * ------------------------------------------------------------------------
 * Constants and definitions
 * ------------------------------------------------------------------------ */


/**
 * Array[z_info->store_max] of stores
 */
struct store *stores;

/**
 * The hints array
 */
struct hint *hints;


static const char *obj_flags[] = {
	"NONE",
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

/**
 * Return the store instance at the given location
 */
struct store *store_at(struct chunk *c, struct loc grid)
{
	if (square_isshop(c, grid))
		return &stores[square_shopnum(c, grid)];

	return NULL;
}


/**
 * Get rid of stores at cleanup. Gets rid of everything.
 */
static void cleanup_stores(void)
{
	struct owner *o, *o_next;
	struct object_buy *buy, *buy_next;
	int i;

	if (!stores)
		return;

	/* Free the store inventories */
	for (i = 0; i < z_info->store_max; i++) {
		/* Get the store */
		struct store *store = &stores[i];

		/* Free the store inventory */
		object_pile_free(NULL, NULL, store->stock_k);
		object_pile_free(NULL, NULL, store->stock);
		mem_free(store->always_table);
		mem_free(store->normal_table);

		for (o = store->owners; o; o = o_next) {
			o_next = o->next;
			string_free(o->name);
			mem_free(o);
		}

		for (buy = store->buy; buy; buy = buy_next) {
			buy_next = buy->next;
			mem_free(buy);
		}
	}
	mem_free(stores);
}


/**
 * ------------------------------------------------------------------------
 * Edit file parsing
 * ------------------------------------------------------------------------ */


/** store.txt **/

static enum parser_error parse_store(struct parser *p) {
	int feat = lookup_feat_code(parser_getstr(p, "feat"));
	struct store *s;

	if (feat < 0 || !tf_has(f_info[feat].flags, TF_SHOP)) {
		return PARSE_ERROR_INVALID_VALUE;
	}

	assert(f_info[feat].shopnum >= 1
		&& f_info[feat].shopnum <= z_info->store_max);
	s = &stores[f_info[feat].shopnum - 1];
	s->feat = feat;
	s->stock_size = z_info->store_inven_max;
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

	struct object_kind *kind = lookup_kind(tval, sval);
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
	struct object_kind *kind = NULL;

	/* Mostly svals are given, but special handling is needed for books */
	if (parser_hasval(p, "sval")) {
		int sval = lookup_sval(tval, parser_getsym(p, "sval"));
		kind = lookup_kind(tval, sval);
		if (!kind) {
			return PARSE_ERROR_UNRECOGNISED_SVAL;
		}

		/* Expand if necessary */
		if (!s->always_num) {
			s->always_size = 8;
			s->always_table = mem_zalloc(s->always_size * sizeof *s->always_table);
		} else if (s->always_num >= s->always_size) {
			s->always_size += 8;
			s->always_table = mem_realloc(s->always_table, s->always_size * sizeof *s->always_table);
		}

		s->always_table[s->always_num++] = kind;
	} else {
		/* Books */
		struct object_base *book_base = &kb_info[tval];
		int i;

		/* Run across all the books for this type, add the town books */
		for (i = 1; i <= book_base->num_svals; i++) {
			const struct class_book *book = NULL;
			kind = lookup_kind(tval, i);
			book = object_kind_to_book(kind);
			if (!book->dungeon) {
				/* Expand if necessary */
				if (!s->always_num) {
					s->always_size = 8;
					s->always_table = mem_zalloc(s->always_size * sizeof *s->always_table);
				} else if (s->always_num >= s->always_size) {
					s->always_size += 8;
					s->always_table = mem_realloc(s->always_table, s->always_size * sizeof *s->always_table);
				}

				s->always_table[s->always_num++] = kind;
			}
		}
	}

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

static enum parser_error parse_buy(struct parser *p) {
	struct store *s = parser_priv(p);
	struct object_buy *buy;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	buy = mem_zalloc(sizeof(*buy));
	buy->tval = tval_find_idx(parser_getstr(p, "base"));
	buy->next = s->buy;
	s->buy = buy;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_buy_flag(struct parser *p) {
	struct store *s = parser_priv(p);
	int flag;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	flag = lookup_flag(obj_flags, parser_getsym(p, "flag"));

	if (flag == FLAG_END) {
		return PARSE_ERROR_INVALID_FLAG;
	} else {
		struct object_buy *buy = mem_zalloc(sizeof(*buy));

		buy->flag = flag;
		buy->tval = tval_find_idx(parser_getstr(p, "base"));
		buy->next = s->buy;
		s->buy = buy;

		return PARSE_ERROR_NONE;
	}
}

struct parser *init_parse_stores(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "store str feat", parse_store);
	parser_reg(p, "owner uint purse str name", parse_owner);
	parser_reg(p, "slots uint min uint max", parse_slots);
	parser_reg(p, "turnover uint turnover", parse_turnover);
	parser_reg(p, "normal sym tval sym sval", parse_normal);
	parser_reg(p, "always sym tval ?sym sval", parse_always);
	parser_reg(p, "buy str base", parse_buy);
	parser_reg(p, "buy-flag sym flag str base", parse_buy_flag);
	/*
	 * The number of stores is known from terrain.txt so allocate the
	 * store array here and fill in the details when parsing.
	 */
	stores = mem_zalloc(z_info->store_max * sizeof(*stores));
	return p;
}

static errr run_parse_stores(struct parser *p) {
	return parse_file_quit_not_found(p, "store");
}

static errr finish_parse_stores(struct parser *p) {
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


/**
 * ------------------------------------------------------------------------
 * Other init stuff
 * ------------------------------------------------------------------------ */


void store_init(void)
{
	event_signal_message(EVENT_INITSTATUS, 0, "Initializing stores...");
	if (run_parser(&store_parser)) quit("Can't initialize stores");
}

void store_reset(void) {
	int i, j;
	struct store *s;

	for (i = 0; i < z_info->store_max; i++) {
		s = &stores[i];
		s->stock_num = 0;
		store_shuffle(s);
		object_pile_free(NULL, NULL, s->stock_k);
		object_pile_free(NULL, NULL, s->stock);
		s->stock_k = NULL;
		s->stock = NULL;
		if (s->feat == FEAT_HOME)
			continue;
		for (j = 0; j < 10; j++)
			store_maint(s);
	}
}


struct init_module store_module = {
	.name = "store",
	.init = store_init,
	.cleanup = cleanup_stores
};





/**
 * Check if a given item kind is an always-stocked item.
 */
static bool store_is_staple(struct store *s, struct object_kind *k) {
	size_t i;

	assert(s);
	assert(k);

	for (i = 0; i < s->always_num; i++) {
		struct object_kind *l = s->always_table[i];
		if (k == l)
			return true;
	}

	return false;
}

/**
 * Check if a given item kind is an always-stocked or sometimes-stocked item.
 */
static bool store_can_carry(struct store *store, struct object_kind *kind) {
	size_t i;

	for (i = 0; i < store->normal_num; i++) {
		if (store->normal_table[i] == kind)
			return true;
	}

	return store_is_staple(store, kind);
}

/**
 * Check if an object is such that selling it should reduce the stock.
 */
static bool store_sale_should_reduce_stock(struct store *store,
		struct object *obj)
{
	if (obj->artifact || obj->ego) return true;
	if (tval_is_weapon(obj) && (obj->to_h || obj->to_d))
		return true;
	if (tval_is_armor(obj) && obj->to_a) return true;
	return !store_is_staple(store, obj->kind);
}


/**
 * ------------------------------------------------------------------------
 * Utilities
 * ------------------------------------------------------------------------ */


/* Randomly select one of the entries in an array */
#define ONE_OF(x)	x[randint0(N_ELEMENTS(x))]


/**
 * ------------------------------------------------------------------------
 * Flavour text stuff
 * ------------------------------------------------------------------------ */


/**
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






/**
 * Let a shop-keeper React to a purchase
 *
 * We paid "price", it was worth "value", and we thought it was worth "guess"
 */
static void purchase_analyze(int price, int value, int guess)
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




/**
 * ------------------------------------------------------------------------
 * Check if a store will buy an object
 * ------------------------------------------------------------------------ */


/**
 * Determine if the current store will purchase the given object
 *
 * Note that a shop-keeper must refuse to buy "worthless" objects
 */
static bool store_will_buy(struct store *store, const struct object *obj)
{
	struct object_buy *buy;

	/* Home accepts anything */
	if (store->feat == FEAT_HOME) return true;

	/* Ignore apparently worthless items, except no-selling {??} items */
	if (object_value(obj, 1) <= 0 && !(OPT(player, birth_no_selling) &&
									   tval_has_variable_power(obj) &&
									   !object_runes_known(obj))) {
		return false;
	}

	/* No buy list means we buy anything */
	if (!store->buy) return true;

	/* Run through the buy list */
	for (buy = store->buy; buy; buy = buy->next) {
		/* Wrong tval */
		if (buy->tval != obj->tval) continue;

		/* No flag means we're good */
		if (!buy->flag) return true;

		/* OK if the object is known to have the flag */
		if (of_has(obj->flags, buy->flag) &&
			object_flag_is_known(player, obj, buy->flag))
			return true;
	}

	/* Not on the list */
	return false;
}


/**
 * ------------------------------------------------------------------------
 * Basics: pricing, generation, etc.
 * ------------------------------------------------------------------------ */


/**
 * Determine the price of an object (qty one) in a store.
 *
 *  store_buying == true  means the shop is buying, player selling
 *               == false means the shop is selling, player buying
 *
 * This function never lets a shop-keeper lose money in a transaction.
 *
 * The "greed" value should exceed 100 when the player is "buying" the
 * object, and should be less than 100 when the player is "selling" it.
 *
 * Hack -- the black market always charges twice as much as it should.
 */
int price_item(struct store *store, const struct object *obj,
			   bool store_buying, int qty)
{
	int adjust = 100;
	int price;
	struct owner *proprietor;

	if (!store) {
		return 0;
	}

	proprietor = store->owner;

	/* Get the value of the stack of wands, or a single item */
	if (tval_can_have_charges(obj)) {
		if (store_buying) {
			price = MIN(object_value_real(obj, qty),
				object_value(obj, qty));
		} else {
			price = MAX(object_value_real(obj, qty),
				object_value(obj, qty));
		}
	} else {
		if (store_buying) {
			price = MIN(object_value_real(obj, 1),
				object_value(obj, 1));
		} else {
			price = MAX(object_value_real(obj, 1),
				object_value(obj, 1));
		}
	}

	/* Worthless items */
	if (price <= 0) {
		return (store_buying) ? 0 : qty;
	}

	/* The black market is always a worse deal */
	if (store->feat == FEAT_STORE_BLACK)
		adjust = 150;

	/* Shop is buying */
	if (store_buying) {
		/* Set the factor */
		adjust = 100 + (100 - adjust);
		if (adjust > 100) {
			adjust = 100;
		}

		/* Shops now pay 2/3 of true value */
		price = price * 2 / 3;

		/* Black market sucks */
		if (store->feat == FEAT_STORE_BLACK) {
			price = price / 2;
		}

		/* Check for no_selling option */
		if (OPT(player, birth_no_selling)) {
			return 0;
		}
	} else {
		/* Re-evaluate if we're selling */
		if (tval_can_have_charges(obj)) {
			price = object_value_real(obj, qty);
		} else {
			price = object_value_real(obj, 1);
		}

		/* Black market sucks */
		if (store->feat == FEAT_STORE_BLACK) {
			price = price * 2;
		}
	}

	/* Compute the final price (with rounding) */
	price = (price * adjust + 50L) / 100L;

	/* Now convert price to total price for non-wands */
	if (!tval_can_have_charges(obj)) {
		price *= qty;
	}

	/* Now limit the price to the purse limit */
	if (store_buying && (price > proprietor->max_cost * qty)) {
		price = proprietor->max_cost * qty;
	}

	/* Note -- Never become "free" */
	if (price <= 0) {
		return qty;
	}

	/* Return the price */
	return price;
}


/**
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


/**
 * Some cheap objects should be created in piles.
 */
static void mass_produce(struct object *obj)
{
	int size = 1;
	int cost = object_value_real(obj, 1);

	/* Analyze the type */
	switch (obj->tval)
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
		case TV_NATURE_BOOK:
		case TV_SHADOW_BOOK:
		case TV_OTHER_BOOK:
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
			if (obj->ego) break;
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
	obj->number = MIN(size, obj->kind->base->max_stack);
}


/**
 * Sort the store inventory into an ordered array.
 */
void store_stock_list(struct store *store, struct object **list, int n)
{
	bool home = (store->feat != FEAT_HOME);
	int list_num;
	int num = 0;

	for (list_num = 0; list_num < n; list_num++) {
		struct object *current, *first = NULL;
		for (current = store->stock; current; current = current->next) {
			int i;
			bool possible = true;

			/* Skip objects already allocated */
			for (i = 0; i < num; i++)
				if (list[i] == current)
					possible = false;

			/* If still possible, choose the first in order */
			if (!possible)
				continue;
			else if (earlier_object(first, current, home))
				first = current;
		}

		/* Allocate and count the stock */
		list[list_num] = first;
		if (first)
			num++;
	}
}

/**
 * Allow a store object to absorb another object
 */
static void store_object_absorb(struct object *old, struct object *new)
{
	int total = old->number + new->number;

	/* Combine quantity, lose excess items */
	old->number = MIN(total, old->kind->base->max_stack);

	/* If rods are stacking, add the charging timeouts */
	if (tval_can_have_timeout(old))
		old->timeout += new->timeout;

	/* If wands/staves are stacking, combine the charges */
	if (tval_can_have_charges(old))
		old->pval += new->pval;

	object_origin_combine(old, new);

	/* Fully absorbed */
	object_delete(NULL, NULL, &new);
}


/**
 * Check to see if the shop will be carrying too many objects
 *
 * Note that the shop, just like a player, will not accept things
 * it cannot hold.  Before, one could "nuke" objects this way, by
 * adding them to a pile which was already full.
 */
bool store_check_num(struct store *store, const struct object *obj)
{
	struct object *stock_obj;

	/* Free space is always usable */
	if (store->stock_num < store->stock_size) return true;

	/* The "home" acts like the player */
	if (store->feat == FEAT_HOME) {
		for (stock_obj = store->stock; stock_obj; stock_obj = stock_obj->next) {
			/* Can the new object be combined with the old one? */
			if (object_mergeable(stock_obj, obj, OSTACK_PACK))
				return true;
		}
	} else {
		/* Normal stores do special stuff */
		for (stock_obj = store->stock; stock_obj; stock_obj = stock_obj->next) {
			/* Can the new object be combined with the old one? */
			if (object_mergeable(stock_obj, obj, OSTACK_STORE))
				return true;
		}
	}

	/* But there was no room at the inn... */
	return false;
}


/**
 * Add an object to the inventory of the Home.
 *
 * Also note that it may not correctly "adapt" to "knowledge" becoming
 * known: the player may have to pick stuff up and drop it again.
 */
void home_carry(struct object *obj)
{
	struct object *temp_obj;
	struct store *store = &stores[f_info[FEAT_HOME].shopnum - 1];

	/* Check each existing object (try to combine) */
	for (temp_obj = store->stock; temp_obj; temp_obj = temp_obj->next) {
		/* The home acts just like the player */
		if (object_mergeable(temp_obj, obj, OSTACK_PACK)) {
			/* Save the new number of items */
			object_absorb(temp_obj, obj);
			return;
		}
	}

	/* No space? */
	if (store->stock_num >= store->stock_size) return;

	/* Insert the new object */
	pile_insert(&store->stock, obj);
	pile_insert(&store->stock_k, obj->known);
	store->stock_num++;
}


/**
 * Add an object to a real stores inventory.
 *
 * If the object is "worthless", it is thrown away (except in the home).
 *
 * If the object cannot be combined with an object already in the inventory,
 * make a new slot for it, and calculate its "per item" price.  Note that
 * this price will be negative, since the price will not be "fixed" yet.
 * Adding an object to a "fixed" price stack will not change the fixed price.
 *
 * Returns the object inserted (for ease of use) or NULL if it disappears
 */
struct object *store_carry(struct store *store, struct object *obj)
{
	unsigned int i;
	uint32_t value;
	struct object *temp_obj, *known_obj = obj->known;

	struct object_kind *kind = obj->kind;

	/* Evaluate the object */
	if (object_is_carried(player, obj))
		value = object_value(obj, 1);
	else
		value = object_value_real(obj, 1);

	/* Cursed/Worthless items "disappear" when sold */
	if (value <= 0)
		return NULL;

	/* Erase the inscription */
	obj->note = 0;
	known_obj->note = 0;

	/* Some item types require maintenance */
	if (tval_is_light(obj)) {
		if (!of_has(obj->flags, OF_NO_FUEL)) {
			if (of_has(obj->flags, OF_BURNS_OUT))
				obj->timeout = z_info->fuel_torch;

			else if (of_has(obj->flags, OF_TAKES_FUEL))
				obj->timeout = z_info->default_lamp;
		}
	} else if (tval_can_have_timeout(obj)) {
		obj->timeout = 0;
	} else if (tval_is_launcher(obj)) {
		obj->known->pval = obj->pval;
	} else if (tval_can_have_charges(obj)) {
		/* If the store can stock this item kind, we recharge */
		if (store_can_carry(store, obj->kind)) {
			int charges = 0;

			/* Calculate the recharged number of charges */
			for (i = 0; i < obj->number; i++)
				charges += randcalc(kind->charge, 0, RANDOMISE);

			/* Use recharged value only if greater */
			if (charges > obj->pval)
				obj->pval = charges;
		}
	}

	for (temp_obj = store->stock; temp_obj; temp_obj = temp_obj->next) {
		/* Can the existing items be incremented? */
		if (object_mergeable(temp_obj, obj, OSTACK_STORE)) {
			/* Absorb (some of) the object */
			store_object_absorb(temp_obj->known, known_obj);
			obj->known = NULL;
			store_object_absorb(temp_obj, obj);

			/* All done */
			return temp_obj;
		}
	}

	/* No space? */
	if (store->stock_num >= store->stock_size)
		return NULL;

	/* Insert the new object */
	pile_insert(&store->stock, obj);
	pile_insert(&store->stock_k, known_obj);
	store->stock_num++;

	return obj;
}


static void store_delete(struct store *s, struct object *obj, int amt)
{
	struct object *known_obj = obj->known;

	if (obj->number > amt) {
		obj->number -= amt;
		known_obj->number -= amt;
	} else {
		pile_excise(&s->stock, obj);
		object_delete(NULL, NULL, &obj);
		pile_excise(&s->stock_k, known_obj);
		object_delete(NULL, NULL, &known_obj);
		assert(s->stock_num);
		s->stock_num--;
	}
}


/**
 * Find a given object kind in the store.  If fexclude is not NULL, exclude
 * any object, o, for which (*fexclude)(s, o) is true.
 */
static struct object *store_find_kind(struct store *s, struct object_kind *k,
		bool (*fexclude)(struct store *, struct object *)) {
	struct object *obj;

	assert(s);
	assert(k);

	/* Check if it's already in stock */
	for (obj = s->stock; obj; obj = obj->next) {
		if (obj->kind == k && (fexclude == NULL ||
			!((*fexclude)(s, obj)))) return obj;
	}

	return NULL;
}


/**
 * Delete an object from store 'store', or, if it is a stack, perhaps only
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
	int num;
	struct object *obj;

	assert(store->stock_num > 0);

	/* Pick a random slot */
	what = randint0(store->stock_num);

	/* Walk through list until we find our item */
	obj = store->stock;
	while (what--) {
		assert(obj);
		obj = obj->next;
	}

	/* Determine how many objects are in the slot */
	num = obj->number;

	/* Deal with stacks */
	if (num > 1) {
		/* Special behaviour for arrows, bolts &tc. */
		if (tval_is_ammo(obj)) {
			/* 50% of the time, destroy the entire stack */
			if (randint0(100) < 50 || num < 10)
				num = obj->number;

			/* 50% of the time, reduce the size to a multiple of 5 */
			else
				num = randint1(num / 5) * 5 + (num % 5);
		} else {
			/* 50% of the time, destroy a single object */
			if (randint0(100) < 50) num = 1;

			/* 25% of the time, destroy half the objects */
			else if (randint0(100) < 50) num = (num + 1) / 2;

			/* 25% of the time, destroy all objects */
			else num = obj->number;

			/* Hack -- decrement the total charges of staves and wands. */
			if (tval_can_have_charges(obj))
				obj->pval -= num * obj->pval / obj->number;
		}
	}

	assert (num <= obj->number);

	if (obj->artifact) {
		history_lose_artifact(player, obj->artifact);
	}

	/* Delete the item, wholly or in part */
	store_delete(store, obj, num);
}


/**
 * This makes sure that the black market doesn't stock any object that other
 * stores have, unless it is an ego-item or has various bonuses.
 *
 * Based on a suggestion by Lee Vogt <lvogt@cig.mcel.mot.com>.
 */
static bool black_market_ok(const struct object *obj)
{
	int i;

	/* Ego items are always fine */
	if (obj->ego) return true;

	/* Good items are normally fine */
	if (obj->to_a > 2) return true;
	if (obj->to_h > 1) return true;
	if (obj->to_d > 2) return true;

	/* No cheap items */
	if (object_value_real(obj, 1) < 10) return (false);

	/* Check the other stores */
	for (i = 0; i < z_info->store_max; i++) {
		struct object *stock_obj;

		/* Skip home and black market */
		if (stores[i].feat == FEAT_STORE_BLACK
				|| stores[i].feat == FEAT_HOME)
			continue;

		/* Check every object in the store */
		for (stock_obj = stores[i].stock; stock_obj; stock_obj = stock_obj->next) {
			/* Compare object kinds */
			if (obj->kind == stock_obj->kind)
				return false;
		}
	}

	/* Otherwise fine */
	return true;
}



/**
 * Get a choice from the store allocation table, in tables.c
 */
static struct object_kind *store_get_choice(struct store *store)
{
	/* Choose a random entry from the store's table */
	return store->normal_table[randint0(store->normal_num)];
}


/**
 * Creates a random object and gives it to store 'store'
 */
static bool store_create_random(struct store *store)
{
	int tries, level;

	int min_level, max_level;

	/* Decide min/max levels */
	if (store->feat == FEAT_STORE_BLACK) {
		min_level = player->max_depth + 5;
		max_level = player->max_depth + 20;
	} else {
		min_level = 1;
		max_level = z_info->store_magic_level + MAX(player->max_depth - 20, 0);
	}

	if (min_level > 55) min_level = 55;
	if (max_level > 70) max_level = 70;

	/* Consider up to six items */
	for (tries = 0; tries < 6; tries++) {
		struct object_kind *kind;
		struct object *obj, *known_obj;

		/* Work out the level for objects to be generated at */
		level = rand_range(min_level, max_level);

		/* Black Markets have a random object, of a given level */
		if (store->feat == FEAT_STORE_BLACK)
			kind = get_obj_num(level, false, 0);
		else
			kind = store_get_choice(store);

		/*** Pre-generation filters ***/

		/* No chests in stores XXX */
		if (kind->tval == TV_CHEST) continue;

		/*** Generate the item ***/

		/* Create a new object of the chosen kind */
		obj = object_new();
		object_prep(obj, kind, level, RANDOMISE);

		/* Apply some "low-level" magic (no artifacts) */
		apply_magic(obj, level, false, false, false, false);
		assert(!obj->artifact);

		/* Reject if item is 'damaged' (negative combat mods, curses) */
		if ((tval_is_weapon(obj) && ((obj->to_h < 0) || (obj->to_d < 0)))
			|| (tval_is_armor(obj) && (obj->to_a < 0)) || (obj->curses)) {
			object_delete(NULL, NULL, &obj);
			continue;
		}

		/*** Post-generation filters ***/

		/* Make a known object */
		known_obj = object_new();
		obj->known = known_obj;

		/* Know everything the player knows, no origin yet */
		obj->known->notice |= OBJ_NOTICE_ASSESSED;
		object_set_base_known(player, obj);
		obj->known->notice |= OBJ_NOTICE_ASSESSED;
		player_know_object(player, obj);
		obj->origin = ORIGIN_NONE;

		/* Black markets have expensive tastes */
		if ((store->feat == FEAT_STORE_BLACK) && !black_market_ok(obj)) {
			object_delete(NULL, NULL, &known_obj);
			obj->known = NULL;
			object_delete(NULL, NULL, &obj);
			continue;
		}

		/* No "worthless" items */
		if (object_value_real(obj, 1) < 1)  {
			object_delete(NULL, NULL, &known_obj);
			obj->known = NULL;
			object_delete(NULL, NULL, &obj);
			continue;
		}

		/* Mass produce and/or apply discount */
		mass_produce(obj);

		/* Attempt to carry the object */
		if (!store_carry(store, obj)) {
			object_delete(NULL, NULL, &known_obj);
			obj->known = NULL;
			object_delete(NULL, NULL, &obj);
			continue;
		}

		/* Definitely done */
		return true;
	}

	return false;
}


/**
 * Helper function: create an item with the given tval,sval pair, add it to the
 * store st.  Return the item in the inventory.
 */
static struct object *store_create_item(struct store *store,
										struct object_kind *kind)
{
	struct object *obj = object_new();
	struct object *known_obj = object_new();
	struct object *carried;

	/* Create a new object of the chosen kind */
	object_prep(obj, kind, 0, RANDOMISE);
	assert(!obj->artifact);

	/* Know everything the player knows, no origin yet */
	obj->known = known_obj;
	obj->known->notice |= OBJ_NOTICE_ASSESSED;
	object_set_base_known(player, obj);
	obj->known->notice |= OBJ_NOTICE_ASSESSED;
	player_know_object(player, obj);
	obj->origin = ORIGIN_NONE;

	/* Attempt to carry the object */
	carried = store_carry(store, obj);
	if (!carried) {
		object_delete(NULL, NULL, &known_obj);
		obj->known = NULL;
		object_delete(NULL, NULL, &obj);
	}
	return carried;
}

/**
 * Maintain the inventory at the stores.
 */
static void store_maint(struct store *s)
{
	/* Ignore home */
	if (s->feat == FEAT_HOME)
		return;

	/* Destroy crappy black market items */
	if (s->feat == FEAT_STORE_BLACK) {
		struct object *obj = s->stock;
		while (obj) {
			struct object *next = obj->next;
			if (!black_market_ok(obj)) {
				if (obj->artifact) {
					history_lose_artifact(player,
						obj->artifact);
				}
				store_delete(s, obj, obj->number);
			}
			obj = next;
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
			quit_fmt("Unable to (de-)stock %s. Please report this bug",
				(f_info[s->feat].name) ? f_info[s->feat].name :
				format("store %d", f_info[s->feat].shopnum));
	} else {
		/* For the Bookseller, occasionally sell a book */
		if (s->always_num && s->stock_num) {
			int sales = randint1(s->stock_num);
			while (sales--) {
				store_delete_random(s);
			}
		}
	}

	/* Ensure staples are created */
	if (s->always_num) {
		size_t i;
		for (i = 0; i < s->always_num; i++) {
			struct object_kind *kind = s->always_table[i];
			struct object *obj = store_find_kind(s, kind,
				store_sale_should_reduce_stock);

			/* Create the item if it doesn't exist */
			if (!obj) {
				obj = store_create_item(s, kind);
				if (!obj) continue;
			}

			/* Ensure a full stack */
			obj->number = obj->kind->base->max_stack;
			obj->known->number = obj->kind->base->max_stack;
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
			quit_fmt("Unable to (re-)stock %s. Please report this bug",
				(f_info[s->feat].name) ? f_info[s->feat].name :
				format("store %d", f_info[s->feat].shopnum));
	}
}

/**
 * Update the stores on the return to town.
 */
void store_update(void)
{
	if (OPT(player, cheat_xtra)) msg("Updating Shops...");
	while (daycount--) {
		int n;

		/* Maintain each shop (except home) */
		for (n = 0; n < z_info->store_max; n++) {
			/* Skip the home */
			if (stores[n].feat == FEAT_HOME) continue;

			/* Maintain */
			store_maint(&stores[n]);
		}

		/* Sometimes, shuffle the shop-keepers */
		if (one_in_(z_info->store_shuffle)) {
			int *non_home_inds = mem_zalloc(z_info->store_max
				* sizeof(*non_home_inds));
			int n_without_home = 0;

			/* Message */
			if (OPT(player, cheat_xtra)) msg("Shuffling a Shopkeeper...");

			/* Pick a random shop (except home) */
			for (n = 0; n < z_info->store_max; n++) {
				if (stores[n].feat != FEAT_HOME) {
					non_home_inds[n_without_home] = n;
					++n_without_home;
				}
			}
			if (n_without_home > 0) {
				n = randint0(n_without_home);
				/* Then suffle it. */
				store_shuffle(&stores[non_home_inds[n]]);
			}
			mem_free(non_home_inds);
		}
	}
	daycount = 0;
	if (OPT(player, cheat_xtra)) msg("Done.");
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

/**
 * Shuffle one of the stores.
 */
void store_shuffle(struct store *store)
{
	struct owner *o = store->owner;

	while (o == store->owner)
	    o = store_choose_owner(store);

	store->owner = o;
}




/**
 * ------------------------------------------------------------------------
 * Higher-level code
 * ------------------------------------------------------------------------ */


/**
 * Return the quantity of a given item in the pack (include quiver).
 */
int find_inven(const struct object *obj)
{
	int i;
	struct object *gear_obj;
	int num = 0;

	/* Similar slot? */
	for (gear_obj = player->gear; gear_obj; gear_obj = gear_obj->next) {
		/* Check only the inventory and the quiver */
		if (object_is_equipped(player->body, gear_obj))
			continue;

		/* Require identical object types */
		if (obj->kind != gear_obj->kind)
			continue;

		/* Analyze the items */
		switch (obj->tval)
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

			/* Wearables */
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
			case TV_RING:
			case TV_AMULET:
			case TV_LIGHT:
			case TV_BOLT:
			case TV_ARROW:
			case TV_SHOT:
			{
				/* Require identical "bonuses" */
				if (obj->to_h != gear_obj->to_h)
					continue;
				if (obj->to_d != gear_obj->to_d)
					continue;
				if (obj->to_a != gear_obj->to_a)
					continue;

				/* Require identical modifiers */
				for (i = 0; i < OBJ_MOD_MAX; i++)
					if (obj->modifiers[i] != gear_obj->modifiers[i])
						continue;

				/* Require identical "artifact" names */
				if (obj->artifact != gear_obj->artifact)
					continue;

				/* Require identical "ego-item" names */
				if (obj->ego != gear_obj->ego)
					continue;

				/* Lights must have same amount of fuel */
				else if (obj->timeout != gear_obj->timeout &&
						 tval_is_light(obj))
					continue;

				/* Require identical "values" */
				if (obj->ac != gear_obj->ac)
					continue;
				if (obj->dd != gear_obj->dd)
					continue;
				if (obj->ds != gear_obj->ds)
					continue;

				/* Probably okay */
				break;
			}

			/* Various */
			default:
			{
				/* Probably okay */
				break;
			}
		}


		/* Different flags */
		if (!of_is_equal(obj->flags, gear_obj->flags))
			continue;

		/* They match, so add up */
		num += gear_obj->number;
	}

	return num;
}


/**
 * Buy the item with the given index from the current store's inventory.
 */
void do_cmd_buy(struct command *cmd)
{
	int amt;

	struct object *obj, *bought, *known_obj;

	char o_name[80];
	int price;

	struct store *store = store_at(cave, player->grid);

	if (!store) {
		msg("You cannot purchase items when not in a store.");
		return;
	}

	/* Get arguments */
	/* XXX-AS fill this out, split into cmd-store.c */
	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK)
		return;

	if (!pile_contains(store->stock, obj)) {
		msg("You cannot buy that item because it's not in the store.");
		return;
	}

	if (cmd_get_arg_number(cmd, "quantity", &amt) != CMD_OK)
		return;

	/* Get desired object */
	bought = object_new();
	object_copy_amt(bought, obj, amt);

	/* Ensure we have room */
	if (bought->number > inven_carry_num(player, bought)) {
		msg("You cannot carry that many items.");
		object_delete(NULL, NULL, &bought);
		return;
	}

	/* Describe the object (fully) */
	object_desc(o_name, sizeof(o_name), bought, ODESC_PREFIX | ODESC_FULL,
		player);

	/* Extract the price for the entire stack */
	price = price_item(store, bought, false, bought->number);

	if (price > player->au) {
		msg("You cannot afford that purchase.");
		object_delete(NULL, NULL, &bought);
		return;
	}

	/* Spend the money */
	player->au -= price;

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE | PN_IGNORE);

	/* Describe the object (fully) again for the message */
	object_desc(o_name, sizeof(o_name), bought, ODESC_PREFIX | ODESC_FULL,
		player);

	/* Message */
	if (one_in_(3)) msgt(MSG_STORE5, "%s", ONE_OF(comment_accept));
	msg("You bought %s for %d gold.", o_name, price);

	/* Erase the inscription */
	bought->note = 0;

	/* Give it an origin if it doesn't have one */
	if (bought->origin == ORIGIN_NONE)
		bought->origin = ORIGIN_STORE;

	/* Hack - Reduce the number of charges in the original stack */
	if (tval_can_have_charges(obj))
		obj->pval -= bought->pval;

	/* Make a known object */
	known_obj = object_new();
	object_copy(known_obj, obj->known);
	bought->known = known_obj;

	/* Learn flavor, any effect and all the runes */
	object_flavor_aware(player, bought);
	bought->known->effect = bought->effect;
	while (!object_fully_known(bought)) {
		object_learn_unknown_rune(player, bought);
		player_know_object(player, bought);
	}

	/* Give it to the player */
	inven_carry(player, bought, true, true);

	/* Handle stuff */
	handle_stuff(player);

	/* Remove the bought objects from the store if it's not a readily
	 * replaced staple item */
	if (store_sale_should_reduce_stock(store, obj)) {
		/* Reduce or remove the item */
		store_delete(store, obj, amt);

		/* Store is empty */
		if (store->stock_num == 0) {
			int i;

			/* Sometimes shuffle the shopkeeper */
			if (one_in_(z_info->store_shuffle)) {
				/* Shuffle */
				msg("The shopkeeper retires.");
				store_shuffle(store);
			} else
				/* Maintain */
				msg("The shopkeeper brings out some new stock.");

			/* New inventory */
			for (i = 0; i < 10; ++i)
				store_maint(store);
		}
	}

	event_signal(EVENT_STORECHANGED);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/**
 * Retrieve the item with the given index from the home's inventory.
 */
void do_cmd_retrieve(struct command *cmd)
{
	int amt;

	struct object *obj, *known_obj, *picked_item;

	struct store *store = store_at(cave, player->grid);
	if (!store) return;

	if (store->feat != FEAT_HOME) {
		msg("You are not currently at home.");
		return;
	}

	/* Get arguments */
	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK)
		return;

	if (!pile_contains(store->stock, obj)) {
		msg("You cannot retrieve that item because it's not in the home.");
		return;
	}

	if (cmd_get_arg_number(cmd, "quantity", &amt) != CMD_OK)
		return;

	/* Get desired object */
	picked_item = object_new();
	object_copy_amt(picked_item, obj, amt);

	/* Ensure we have room */
	if (picked_item->number > inven_carry_num(player, picked_item)) {
		msg("You cannot carry that many items.");
		object_delete(NULL, NULL, &picked_item);
		return;
	}

	/* Distribute charges of wands, staves, or rods */
	distribute_charges(obj, picked_item, amt);

	/* Make a known object */
	known_obj = object_new();
	object_copy(known_obj, obj->known);
	picked_item->known = known_obj;

	/* Give it to the player */
	inven_carry(player, picked_item, true, true);

	/* Handle stuff */
	handle_stuff(player);
	
	/* Reduce or remove the item */
	store_delete(store, obj, amt);

	event_signal(EVENT_STORECHANGED);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}


/**
 * Determine if the current store will purchase the given object
 */
bool store_will_buy_tester(const struct object *obj)
{
	struct store *store = store_at(cave, player->grid);
	if (!store) return false;

	return store_will_buy(store, obj);
}

/**
 * Sell an item to the current store.
 */
void do_cmd_sell(struct command *cmd)
{
	int amt;
	struct object dummy_item;
	struct store *store = store_at(cave, player->grid);
	int price, dummy, value;
	char o_name[120];
	char label;

	struct object *obj, *sold_item;
	bool none_left = false;

	/* Get arguments */
	/* XXX-AS fill this out, split into cmd-store.c */
	if (cmd_get_arg_item(cmd, "item", &obj) != CMD_OK)
		return;

	if (cmd_get_quantity(cmd, "quantity", &amt, obj->number) != CMD_OK)
		return;

	/* Cannot remove stickied objects */
	if (object_is_equipped(player->body, obj) && !obj_can_takeoff(obj)) {
		msg("Hmmm, it seems to be stuck.");
		return;
	}

	/* Check we are somewhere we can sell the items. */
	if (!store) {
		msg("You cannot sell items when not in a store.");
		return;
	}

	/* Check the store wants the items being sold */
	if (!store_will_buy(store, obj)) {
		msg("I do not wish to purchase this item.");
		return;
	}

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(&dummy_item, obj, amt);

	/* Check if the store has space for the items */
	if (!store_check_num(store, &dummy_item)) {
		object_wipe(&dummy_item);
		msg("I have not the room in my store to keep it.");
		return;
	}

	/* Get the label */
	label = gear_to_label(player, obj);

	price = price_item(store, &dummy_item, true, amt);

	/* Get some money */
	player->au += price;

	/* Update the auto-history if selling an artifact that was previously
	 * un-IDed. (Ouch!) */
	if (obj->artifact)
		history_find_artifact(player, obj->artifact);

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

	/* Get the "apparent" value */
	dummy = object_value(&dummy_item, amt);
	/*
	 * Do not need the dummy any more so release the memory allocated
	 * within it.
	 */
	object_wipe(&dummy_item);

	/* Know flavor of consumables */
	object_flavor_aware(player, obj);
	obj->known->effect = obj->effect;
	while (!object_fully_known(obj)) {
		object_learn_unknown_rune(player, obj);
		player_know_object(player, obj);
	}

	/* Take a proper copy of the now known-about object. */
	sold_item = gear_object_for_use(player, obj, amt, false, &none_left);

	/* Get the "actual" value */
	value = object_value_real(sold_item, amt);

	/* Get the description all over again */
	object_desc(o_name, sizeof(o_name), sold_item,
		ODESC_PREFIX | ODESC_FULL, player);

	/* Describe the result (in message buffer) */
	if (OPT(player, birth_no_selling)) {
		msg("You had %s (%c).", o_name, label);
	} else {
		msg("You sold %s (%c) for %d gold.", o_name, label, price);

		/* Analyze the prices (and comment verbally) */
		purchase_analyze(price, value, dummy);
	}

	/* Autoinscribe if we still have any */
	if (!none_left)
		apply_autoinscription(player, obj);

	/* Set ignore flag */
	player->upkeep->notice |= PN_IGNORE;

	/* Notice if pack items need to be combined or reordered */
	notice_stuff(player);

	/* Handle stuff */
	handle_stuff(player);

	/* The store gets that (known) object */
	if (!store_carry(store, sold_item)) {
		/* The store rejected it; delete. */
		if (sold_item->artifact) {
			history_lose_artifact(player, sold_item->artifact);
		}
		if (sold_item->known) {
			object_delete(NULL, NULL, &sold_item->known);
			sold_item->known = NULL;
		}
		object_delete(NULL, NULL, &sold_item);
	}

	event_signal(EVENT_STORECHANGED);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}

/**
 * Stash an item in the home.
 */
void do_cmd_stash(struct command *cmd)
{
	int amt;
	struct object dummy;
	struct store *store = store_at(cave, player->grid);
	char o_name[120];
	char label;

	struct object *obj, *dropped;
	bool none_left = false;
	bool no_room;

	if (cmd_get_arg_item(cmd, "item", &obj))
		return;

	if (cmd_get_quantity(cmd, "quantity", &amt, obj->number) != CMD_OK)
		return;

	/* Check we are somewhere we can stash items. */
	if (store->feat != FEAT_HOME) {
		msg("You are not in your home.");
		return;
	}

	/* Cannot remove stickied objects */
	if (object_is_equipped(player->body, obj) && !obj_can_takeoff(obj)) {
		msg("Hmmm, it seems to be stuck.");
		return;
	}	

	/* Get a copy of the object representing the number being sold */
	object_copy_amt(&dummy, obj, amt);

	no_room = !store_check_num(store, &dummy);
	/*
	 * Do not need the dummy any more so release the memory allocated
	 * within it.
	 */
	object_wipe(&dummy);
	if (no_room) {
		msg("Your home is full.");
		return;
	}

	/* Get where the object is now */
	label = gear_to_label(player, obj);

	/* Now get the real item */
	dropped = gear_object_for_use(player, obj, amt, false, &none_left);

	/* Describe */
	object_desc(o_name, sizeof(o_name), dropped,
		ODESC_PREFIX | ODESC_FULL, player);

	/* Message */
	msg("You drop %s (%c).", o_name, label);

	/* Handle stuff */
	handle_stuff(player);

	/* Let the home carry it */
	home_carry(dropped);

	event_signal(EVENT_STORECHANGED);
	event_signal(EVENT_INVENTORY);
	event_signal(EVENT_EQUIPMENT);
}
