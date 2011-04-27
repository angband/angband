/* player/birth */

#include "unit-test.h"
#include "unit-test-data.h"
#include "birth.h"

int setup_tests(void **state) {
	struct player *p = mem_alloc(sizeof *p);
	player_init(p);
	*state = p;
	return 0;
}

int teardown_tests(void *state) {
	mem_free(state);
	return 0;
}

int test_generate0(void *state) {
	struct player *p = state;
	player_generate(p, &test_sex, &test_race, &test_class);
	eq(p->lev, 1);
	ptreq(p->sex, &test_sex);
	ptreq(p->race, &test_race);
	ptreq(p->class, &test_class);
	ok;
}

const char *suite_name = "player/birth";
struct test tests[] = {
	{ "generate0", test_generate0 },
	{ NULL, NULL }
};
