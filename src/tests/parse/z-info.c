/* parse/z-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"


int setup_tests(void **state) {
	*state = init_parse_constants();
	return !*state;
}

int teardown_tests(void *state) {
	struct angband_constants *z = parser_priv(state);
	mem_free(z);
	parser_destroy(state);
	return 0;
}

static int test_negative(void *state) {
	errr r = parser_parse(state, "level-max:F:-1");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_badmax(void *state) {
	errr r = parser_parse(state, "level-max:D:1");
	eq(r, PARSE_ERROR_UNDEFINED_DIRECTIVE);
	ok;
}

#define TEST_MAX(l,u) \
	static int test_##l(void *s) { \
		struct angband_constants *m = parser_priv(s); \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "level-max:%s:%d", u, __LINE__); \
		r = parser_parse(s, buf); \
		eq(m->l, __LINE__); \
		eq(r, 0); \
		ok; \
	}

TEST_MAX(level_monster_max, "monsters")

#define TEST_MON(l,u) \
	static int test_##l(void *s) { \
		struct angband_constants *m = parser_priv(s); \
		char buf[64]; \
		errr r; \
		snprintf(buf, sizeof(buf), "mon-gen:%s:%d", u, __LINE__); \
		r = parser_parse(s, buf); \
		eq(m->l, __LINE__); \
		eq(r, 0); \
		ok; \
	}

TEST_MON(alloc_monster_chance, "chance")
TEST_MON(level_monster_min, "level-min")
TEST_MON(town_monsters_day, "town-day")
TEST_MON(town_monsters_night, "town-night")
TEST_MON(repro_monster_max, "repro-max")

const char *suite_name = "parse/z-info";
struct test tests[] = {
	{ "negative", test_negative },
	{ "badmax", test_badmax },
	{ "monsters_max", test_level_monster_max },
	{ "mon_chance", test_alloc_monster_chance },
	{ "monsters_min", test_level_monster_min },
	{ "town_day", test_town_monsters_day },
	{ "town_night", test_town_monsters_night },
	{ "repro_max", test_repro_monster_max },
	{ NULL, NULL }
};
