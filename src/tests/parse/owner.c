/* parse/owner */

#include "unit-test.h"

#include "store.h"

struct store s0;
struct store s1;

static int setup(void **state) {
	s0.next = &s1;
	s1.next = NULL;
	s0.sidx = 0;
	s1.sidx = 1;
	*state = store_owner_parser_new(&s0);
	return !*state;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:0");

	eq(r, PARSE_ERROR_NONE);
	require(parser_priv(state));
	ok;
}

static int test_s0(void *state) {
	enum parser_error r = parser_parse(state, "S:5000:Foo");

	eq(r, PARSE_ERROR_NONE);
	eq(s0.owners->max_cost, 5000);
	require(streq(s0.owners->name, "Foo"));
	ok;
}

static int test_s1(void *state) {
	enum parser_error r = parser_parse(state, "S:10000:Bar");

	eq(r, PARSE_ERROR_NONE);
	eq(s0.owners->max_cost, 10000);
	require(streq(s0.owners->name, "Bar"));
	ok;
}

static int test_n1(void *state) {
	enum parser_error r = parser_parse(state, "N:1");

	eq(r, PARSE_ERROR_NONE);
	require(parser_priv(state));
	ok;
}

static int test_s2(void *state) {
	enum parser_error r = parser_parse(state, "S:15000:Baz");

	eq(r, PARSE_ERROR_NONE);
	eq(s1.owners->max_cost, 15000);
	require(streq(s1.owners->name, "Baz"));
	ok;
}

static const char *suite_name = "parse/owner";
static struct test tests[] = {
	{ "n0", test_n0 },
	{ "s0", test_s0 },
	{ "s1", test_s1 },
	{ "n1", test_n1 },
	{ "s2", test_s2 },
	{ NULL, NULL }
};
