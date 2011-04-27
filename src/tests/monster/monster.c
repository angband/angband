/* monster/monster 
 *
 * Tests for monster/monster?.c
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#include "unit-test.h"
#include "unit-test-data.h"
#include "test-utils.h"

static int setup(void **state) {
	read_edit_files();
	*state = 0;
	return 0;
}

static int teardown(void *state) {
	mem_free(state);
	return 0;
}

static int test_match_monster_bases(void *state) {
	struct monster_base *base;

	/* Scruffy little dog */
	base = (&r_info[3])->base;
	require(match_monster_bases(base, "canine", NULL));
	require(match_monster_bases(base, "zephyr hound", "canine", NULL));
	require(!match_monster_bases(base, "angel", NULL));
	require(!match_monster_bases(base, "lich", "vampire", "wraith", NULL));

	/* Morgoth */
	base = (&r_info[547])->base;
	require(!match_monster_bases(base, "canine", NULL));
	require(!match_monster_bases(base, "lich", "vampire", "wraith", NULL));
	require(match_monster_bases(base, "person", "Morgoth", NULL));
	require(match_monster_bases(base, "Morgoth", NULL));

	ok;
}

static const char *suite_name = "monster/monster";
static struct test tests[] = {
	{ "match_monster_bases", test_match_monster_bases },
	{ NULL, NULL }
};
