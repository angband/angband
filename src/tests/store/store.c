/*
 * store/store.c
 *
 *  Created on: 22 Apr 2011
 *      Author: noz
 */


#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"

#include "init.h"
#include "store.h"

int setup_tests(void **state) {
	read_edit_files();
	*state = 0;
	return 0;
}

int teardown_tests(void *state) {
	return 0;
}

static int number_distinct_items(int which)
{
	struct store *s = &stores[which];
	unsigned int i;
	u32b *kinds = mem_zalloc(sizeof(u32b) * s->table_num);
	u32b kind;
	int number_kinds = 0;

	/* Loop over available stock entries, counting unique kinds */
	for (i = 0; i < s->table_num; i++)
	{
		unsigned int j;

		kind = s->table[i]->kidx;
		/* Loop over existing found kinds, and skip out early if it's
		 * been seen already */
		for (j = 0; j < i; j++)
		{
			if (kinds[j] == kind)
				break;
		}
		/* If we've run off the end of the loop, we've not seen it before,
		 * so count it */
		if (j == i) {
			kinds[j] = kind;
			number_kinds++;
		}
	}
	return number_kinds;
}

/*
 * Check that each shop stocks enough types of items to be able to fill up
 * if the RNG chooses the largest possible stock (STORE_MAX_KEEP).
 */
int test_enough_armor(void *state) {
	require(number_distinct_items(STORE_ARMOR) >= STORE_MAX_KEEP);
	ok;
}
int test_enough_weapons(void *state) {
	require(number_distinct_items(STORE_WEAPON) >= STORE_MAX_KEEP);
	ok;
}
int test_enough_temple(void *state) {
	require(number_distinct_items(STORE_TEMPLE) >= STORE_MAX_KEEP);
	ok;
}
int test_enough_alchemy(void *state) {
	require(number_distinct_items(STORE_ALCHEMY) >= STORE_MAX_KEEP);
	ok;
}
int test_enough_magic(void *state) {
	require(number_distinct_items(STORE_MAGIC) >= STORE_MAX_KEEP);
	ok;
}

const char *suite_name = "store/store";
struct test tests[] = {
	{ "Enough items in Armoury", test_enough_armor },
	{ "Enough items in Weaponsmith", test_enough_weapons },
	{ "Enough items in Temple", test_enough_temple },
	{ "Enough items in Alchemists", test_enough_alchemy },
	{ "Enough items in Magicians", test_enough_magic },
	{ NULL, NULL }
};
