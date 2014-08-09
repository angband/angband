/* monster/monster 
 *
 * Tests for monster/monster?.c
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#include "unit-test.h"
#include "unit-test-data.h"
#include "monster/mon-util.h"

NOSETUP
NOTEARDOWN

/* Regression test for #1409 */
int test_match_monster_bases(void *state) {
	struct monster_base *base;
	rb_info = &test_rb_angel;

	/* Scruffy little dog */
	base = test_r_littledog.base;
	require(match_monster_bases(base, "canine", NULL));
	require(match_monster_bases(base, "zephyr hound", "canine", NULL));
	require(!match_monster_bases(base, "angel", NULL));
	require(!match_monster_bases(base, "lich", "vampire", "wraith", NULL));

	/* Human */
	base = test_r_human.base;
	require(!match_monster_bases(base, "canine", NULL));
	require(!match_monster_bases(base, "lich", "vampire", "wraith", NULL));
	require(match_monster_bases(base, "person", "townsfolk", NULL));
	require(match_monster_bases(base, "townsfolk", NULL));

	ok;
}

const char *suite_name = "monster/monster";
struct test tests[] = {
	{ "match_monster_bases", test_match_monster_bases },
	{ NULL, NULL }
};
