/* parse/z-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"


int setup_tests(void **state) {
	*state = init_parse_z();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_negative(void *state) {
	errr r = parser_parse(state, "level-max:F:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

int test_badmax(void *state) {
	errr r = parser_parse(state, "level-max:D:1");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

#define TEST_MAX(l,u) \
	int test_##l(void *s) { \
		struct angband_constants *m = parser_priv(s); \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "level-max:%s:%d", u, __LINE__); \
		r = parser_parse(s, buf); \
		eq(m->l, __LINE__); \
		eq(r, 0); \
		ok; \
	}

TEST_MAX(level_object_max, "objects")
TEST_MAX(level_monster_max, "monsters")
TEST_MAX(level_trap_max, "traps")

const char *suite_name = "parse/z-info";
struct test tests[] = {
	{ "negative", test_negative },
	{ "badmax", test_badmax },
	{ "objects_max", test_level_object_max },
	{ "monsters_max", test_level_monster_max },
	{ "traps_max", test_level_trap_max },
	{ NULL, NULL }
};
