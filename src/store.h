/**
 * \file store.h
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

#ifndef INCLUDED_STORE_H
#define INCLUDED_STORE_H

#include "cave.h"
#include "cmd-core.h"
#include "datafile.h"
#include "object.h"

struct object_buy {
	struct object_buy *next;
	size_t tval;
	size_t flag;
};

struct owner {
	unsigned int oidx;
	struct owner *next;
	char *name;
	int32_t max_cost;
};

struct store {
	struct owner *owners;		/**< linked list of the possible
						owners of the store */
	struct owner *owner;		/**< pointer to the current owner of
						the store */
	int feat;			/**< index for entrance's terrain */

	uint8_t stock_num;		/**< number of stacks in stock */
	int16_t stock_size;		/**< maximum number of stacks the
						store can have in stock */
	struct object *stock;		/**< actual stock items */
	struct object *stock_k;		/**< stock as known by the character */

	size_t always_size;		/**< maximum number of entries that can
						be stored in always_table */
	size_t always_num;		/**< current number of entries in
						always_table */
	struct object_kind **always_table;	/**< array of types of items the store always keeps in stock */

	size_t normal_size;		/**< maximum number of entries that
						can be stored in normal_table */
	size_t normal_num;		/**< current number of entries in
						normal_table */
	struct object_kind **normal_table;	/**< array of types of items
							the store may keep in
							stock */

	struct object_buy *buy;		/**< linked list of types of items the
						store is willing to buy */

	int turnover;			/**< up to this number of stacks are
						removed in store_maint(); if
						zero, the bounds from
						normal_stock_min and
						normal_stock_max are not
						applied */
	int normal_stock_min;		/**< minimum number of stacks that are
						not always stocked items to keep
						in store_maint(); only relevant
						if turnover is not zero */
	int normal_stock_max;		/**< maximum number of stacks that are
						not always stacked items to keep
						in store_maint(); only relevant
						if turnover is not zero */
};

extern struct store *stores;

struct store *store_at(struct chunk *c, struct loc grid);
void store_init(void);
void free_stores(void);
void store_stock_list(struct store *store, struct object **list, int n);
void home_carry(struct object *obj);
struct object *store_carry(struct store *store, struct object *obj,
		bool maintain);
void store_reset(void);
void store_shuffle(struct store *store);
void store_update(void);
int price_item(struct store *store, const struct object *obj,
			   bool store_buying, int qty);

bool store_will_buy_tester(const struct object *obj);
bool store_check_num(struct store *store, const struct object *obj);
int find_inven(const struct object *obj);

extern struct owner *store_ownerbyidx(struct store *s, unsigned int idx);

struct parser *init_parse_stores(void);
extern struct parser *store_parser_new(void);
extern struct parser *store_owner_parser_new(struct store *stores);

extern void do_cmd_sell(struct command *cmd);
extern void do_cmd_stash(struct command *cmd);
extern void do_cmd_buy(struct command *cmd);
extern void do_cmd_retrieve(struct command *cmd);

#endif /* INCLUDED_STORE_H */
