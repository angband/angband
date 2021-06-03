/* parse/e-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "obj-tval.h"
#include "object.h"
#include "init.h"


int setup_tests(void **state) {
	*state = init_parse_ego();
	return !*state;
}

int teardown_tests(void *state) {
	struct ego_item *e = parser_priv(state);
	string_free(e->name);
	string_free(e->text);
	mem_free(e);
	parser_destroy(state);
	return 0;
}

static int test_order(void *state) {
	enum parser_error r = parser_parse(state, "info:4");
	eq(r, PARSE_ERROR_MISSING_FIELD);
	ok;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:of Resist Lightning");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	require(streq(e->name, "of Resist Lightning"));
	ok;
}

static int test_info0(void *state) {
	enum parser_error r = parser_parse(state, "info:6:8");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->cost, 6);
	eq(e->rating, 8);
	ok;
}

static int test_combat0(void *state) {
	enum parser_error r = parser_parse(state, "combat:1d2:3d4:5d6");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->to_h.dice, 1);
	eq(e->to_h.sides, 2);
	eq(e->to_d.dice, 3);
	eq(e->to_d.sides, 4);
	eq(e->to_a.dice, 5);
	eq(e->to_a.sides, 6);
	ok;
}

static int test_min0(void *state) {
	enum parser_error r = parser_parse(state, "min-combat:10:13:4");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->min_to_h, 10);
	eq(e->min_to_d, 13);
	eq(e->min_to_a, 4);
	ok;
}

static int test_flags0(void *state) {
	enum parser_error r = parser_parse(state, "flags:SEE_INVIS");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	require(e->flags);
	ok;
}

static int test_desc0(void *state) {
	enum parser_error r = parser_parse(state, "desc:foo");
	struct ego_item *e;
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(state, "desc: bar");
	eq(r, PARSE_ERROR_NONE);

	e = parser_priv(state);
	require(e);
	require(streq(e->text, "foo bar"));
	ok;
}

const char *suite_name = "parse/e-info";
struct test tests[] = {
	{ "order", test_order },
	{ "name0", test_name0 },
	{ "info0", test_info0 },
	{ "combat0", test_combat0 },
	{ "min_combat0", test_min0 },
	{ "flags0", test_flags0 },
	{ "desc0", test_desc0 },
	{ NULL, NULL }
};
