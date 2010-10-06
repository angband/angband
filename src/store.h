#ifndef INCLUDED_STORE_H
#define INCLUDED_STORE_H

#include "object/types.h"

/*** Constants ***/

#define STORE_INVEN_MAX		24    /* Max number of discrete objs in inven */
#define STORE_TURNS		1000  /* Number of turns between turnovers */
#define STORE_SHUFFLE		25    /* 1/Chance (per day) of an owner changing */

/* List of store indices */
enum
{
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

typedef struct owner {
	struct owner *next;
	unsigned int store;
	unsigned int oidx;
	char *owner_name;
	s32b max_cost;		/* Purse limit */
} owner_type;

/*
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
typedef struct store {
	byte owner;				/* Owner index */

	byte stock_num;			/* Stock -- Number of entries */
	s16b stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */

	s16b table_num;     /* Table -- Number of entries */
	s16b table_size;    /* Table -- Total Size of Array */
	s16b *table;        /* Table -- Legal item kinds */
} store_type;

/*** Functions ***/

/* store.c */
void store_init(void);
void store_shuffle(int which);
void store_maint(int which);
s32b price_item(const object_type *o_ptr, bool store_buying, int qty);


#endif /* INCLUDED_STORE_H */
