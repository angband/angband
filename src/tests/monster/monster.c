/* monster/monster 
 *
 * Tests for monster/monster?.c
 *
 * Created by: myshkin
 *             26 Apr 2011
 */

#include "mon-util.h"
#include "player-birth.h"
#include "test-utils.h"
#include "unit-test.h"
#include "unit-test-data.h"

int setup_tests(void **state) {
	set_file_paths();
	init_angband();
	*state = 0;
	return 0;
}

int teardown_tests(void *state) {
	mem_free(state);
	return 0;
}

/* Regression test for #1409 */
static int test_match_monster_bases(void *state) {
	struct monster_base *base;

	/* Scruffy little dog */
	base = (&r_info[3])->base;
	require(match_monster_bases(base, "canine", NULL));
	require(match_monster_bases(base, "zephyr hound", "canine", NULL));
	require(!match_monster_bases(base, "angel", NULL));
	require(!match_monster_bases(base, "lich", "vampire", "wraith", NULL));

	/* Morgoth */
	base = (lookup_monster("Morgoth, Lord of Darkness"))->base;
	require(!match_monster_bases(base, "canine", NULL));
	require(!match_monster_bases(base, "lich", "vampire", "wraith", NULL));
	require(match_monster_bases(base, "person", "Morgoth", NULL));
	require(match_monster_bases(base, "Morgoth", NULL));

	ok;
}

static int test_nearby_kin(void *state) {
	struct chunk *c = t_build_arena(20, 20);

	player_make_simple(NULL, NULL, "Tester");

	struct monster *wolf0 = t_add_monster(c, loc(5, 5), "wolf");
	struct monster *wolf1 = t_add_monster(c, loc(4, 5), "wolf");
	struct monster *warg0 = t_add_monster(c, loc(6, 5), "warg");
	struct monster *cat0 = t_add_monster(c, loc(5, 6), "wild cat");
	struct monster *wolf2 = t_add_monster(c, loc(9, 5), "wolf");
	struct monster *warg1 = t_add_monster(c, loc(2, 2), "warg");
	struct monster *wolf3 = t_add_monster(c, loc(15, 5), "wolf");

	/* To start off with, nothing is injured. */
	ptreq(NULL, choose_nearby_injured_kin(c, wolf0));

	/* Injure the nearby wolf, it should get chosen. */
	wolf1->hp -= 1;
	ptreq(wolf1, choose_nearby_injured_kin(c, wolf0));
	wolf1->hp += 1;

	/* Injure the nearby warg (different race, same base), it should get
	 * chosen. */
	warg0->hp -= 1;
	ptreq(warg0, choose_nearby_injured_kin(c, wolf0));
	warg0->hp += 1;

	/* Injure the nearby cat (different base), it should not get chosen. */
	cat0->hp -= 1;
	ptreq(NULL, choose_nearby_injured_kin(c, wolf0));
	cat0->hp += 1;

	/* Injure the distant wolf, it should not get chosen. */
	wolf3->hp -= 1;
	ptreq(NULL, choose_nearby_injured_kin(c, wolf0));
	wolf3->hp += 1;

	/* Injure the wolf at (9,5). It should get chosen at first... */
	wolf2->hp -= 1;
	ptreq(wolf2, choose_nearby_injured_kin(c, wolf0));
	/* but not when LOS is lost. */
	square_set_feat(c, loc(8, 5), FEAT_PERM);
	ptreq(NULL, choose_nearby_injured_kin(c, wolf2));
	square_set_feat(c, loc(8, 5), FEAT_FLOOR);
	wolf2->hp += 1;

	/* Now, injure all the canines and check for nearby kin a few times.
	 * We should get at least a couple of different ones. */
	wolf1->hp -= 1;
	wolf2->hp -= 1;
	warg0->hp -= 1;
	warg1->hp -= 1;

	int seen_w1 = 0;
	int seen_w2 = 0;
	int seen_a0 = 0;
	int seen_a1 = 0;

	for (int i = 0; i < 1000; i++) {
		struct monster *m = choose_nearby_injured_kin(c, wolf0);
		if (m == wolf1)
			seen_w1++;
		else if (m == wolf2)
			seen_w2++;
		else if (m == warg0)
			seen_a0++;
		else if (m == warg1)
			seen_a1++;
	}

	require(seen_w1 > 0);
	require(seen_w2 > 0);
	require(seen_a0 > 0);
	require(seen_a1 > 0);

	ok;
}

const char *suite_name = "monster/monster";
struct test tests[] = {
	{ "match_monster_bases", test_match_monster_bases },
	{ "nearby_kin", test_nearby_kin },
	{ NULL, NULL }
};
