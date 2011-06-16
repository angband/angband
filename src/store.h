#ifndef INCLUDED_STORE_H
#define INCLUDED_STORE_H

#include "object/obj-flag.h"
#include "object/object.h"
#include "parser.h"

#define STORE_INVEN_MAX		24    /* Max number of discrete objs in inven */
#define STORE_TURNS		1000  /* Number of turns between turnovers */
#define STORE_SHUFFLE		25    /* 1/Chance (per day) of an owner changing */
#define STORE_MIN_KEEP  6       /* Min slots to "always" keep full (>0) */
#define STORE_MAX_KEEP  18      /* Max slots to "always" keep full (<STORE_INVEN_MAX) */

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
	unsigned int oidx;
	struct owner *next;
	char *name;
	s32b max_cost;
} owner_type;

struct store {
	struct store *next;
	struct owner *owners;
	struct owner *owner;
	unsigned int sidx;

	byte stock_num;			/* Stock -- Number of entries */
	s16b stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */

	unsigned int table_num;     /* Table -- Number of entries */
	unsigned int table_size;    /* Table -- Total Size of Array */
	object_kind **table;        /* Table -- Legal item kinds */
};

void store_init(void);
void free_stores(void);
void store_reset(void);
void store_shuffle(struct store *store);
void store_maint(struct store *store);
s32b price_item(const object_type *o_ptr, bool store_buying, int qty);

extern struct owner *store_ownerbyidx(struct store *s, unsigned int idx);

#ifdef TEST
extern struct parser *store_parser_new(void);
extern struct parser *store_owner_parser_new(struct store *stores);
#endif

#endif /* INCLUDED_STORE_H */
