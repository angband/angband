#ifndef INCLUDED_STORE_H
#define INCLUDED_STORE_H

#include "object/types.h"
#include "parser.h"

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
	s32b max_cost;
} owner_type;

typedef struct store {
	struct store *next;
	byte owner;				/* Owner index */
	unsigned int sidx;

	byte stock_num;			/* Stock -- Number of entries */
	s16b stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */

	unsigned int table_num;     /* Table -- Number of entries */
	unsigned int table_size;    /* Table -- Total Size of Array */
	s16b *table;        /* Table -- Legal item kinds */
} store_type;

void store_init(void);
void store_shuffle(int which);
void store_maint(int which);
s32b price_item(const object_type *o_ptr, bool store_buying, int qty);

extern struct parser *store_parser_new(void);

#endif /* INCLUDED_STORE_H */
