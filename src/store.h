
/*
 * A store owner
 */
struct owner_type
{
	uint32_t owner_name;	/* Name (offset) */
	int32_t max_cost;		/* Purse limit */
};




/*
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
struct store_type
{
	uint8_t owner;				/* Owner index */

	uint8_t stock_num;			/* Stock -- Number of entries */
	int16_t stock_size;		/* Stock -- Total Size of Array */
	object_type *stock;		/* Stock -- Actual stock items */

	int16_t table_num;     /* Table -- Number of entries */
	int16_t table_size;    /* Table -- Total Size of Array */
	int16_t *table;        /* Table -- Legal item kinds */
};

