#ifndef INCLUDED_STORE_H
#define INCLUDED_STORE_H

#include "cave.h"
#include "object.h"
#include "parser.h"

extern bool store_in_store;


/* List of store indices */
enum {
	STORE_NONE      = -1,
	STORE_GENERAL	= 0,
	STORE_ARMOR	= 1,
	STORE_WEAPON	= 2,
	STORE_TEMPLE	= 3,
	STORE_ALCHEMY	= 4,
	STORE_MAGIC	= 5,
	STORE_B_MARKET	= 6,
	STORE_HOME	= 7,
	MAX_STORES	= 8
};

struct object_buy {
	struct object_buy *next;
	size_t tval;
	size_t flag;
};

struct owner {
	unsigned int oidx;
	struct owner *next;
	char *name;
	s32b max_cost;
};

struct store {
	struct store *next;
	struct owner *owners;
	struct owner *owner;
	unsigned int sidx;
	const char *name;

	byte stock_num;				/* Stock -- Number of entries */
	s16b stock_size;			/* Stock -- Total Size of Array */
	struct object *stock;		/* Stock -- Actual stock items */

	/* Always stock these items */
	size_t always_size;
	size_t always_num;
	object_kind **always_table;

	/* Select a number of these items to stock */
	size_t normal_size;
	size_t normal_num;
	object_kind **normal_table;

	/* Buy these items */
	struct object_buy *buy;

	int turnover;
	int normal_stock_min;
	int normal_stock_max;
};

extern struct store *stores;

struct store *store_at(struct chunk *c, int y, int x);
void store_init(void);
void free_stores(void);
void store_stock_list(struct store *store, struct object **list, int n);
struct object *store_carry(struct store *store, struct object *obj);
void store_reset(void);
void store_shuffle(struct store *store);
void store_maint(struct store *store);
int price_item(struct store *store, const object_type *o_ptr, bool store_buying, int qty);

extern struct owner *store_ownerbyidx(struct store *s, unsigned int idx);

struct parser *init_parse_stores(void);
extern struct parser *store_parser_new(void);
extern struct parser *store_owner_parser_new(struct store *stores);
void textui_store_knowledge(int store);

#endif /* INCLUDED_STORE_H */
