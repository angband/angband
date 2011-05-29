/* monster/attack */

#include "unit-test.h"
#include "unit-test-data.h"

#include "monster/monster.h"

int setup_tests(void **state) {
	struct monster_race *r = &test_r_human;
	struct monster *m = mem_zalloc(sizeof *m);
	m->race = r;
	m->r_idx = r->ridx;
	r_info = r;
	*state = m;
	return 0;
}

NOTEARDOWN

static int test_attack(void *state) {
	struct monster *m = state;
	struct player *p = &test_player;
	/* testfn_make_attack_normal */
	testfn_make_attack_normal(m, p);
	ok;
}

const char *suite_name = "monster/attack";
const struct test tests[] = {
	{ "attack", test_attack },
	{ NULL, NULL },
};
