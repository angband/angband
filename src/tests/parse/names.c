/* parse/names */

#include "unit-test.h"
#include "init.h"

int setup_tests(void **state) {
	*state = init_parse_names();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	errr r = parser_parse(state, "N:1");
	eq(r, 0);
	ok;
}

int test_d0(void *state) {
	errr r = parser_parse(state, "D:foo");
	eq(r, 0);
	r = parser_parse(state, "D:bar");
	eq(r, 0);
	ok;
}

int test_n1(void *state) {
	errr r = parser_parse(state, "N:2");
	eq(r, 0);
	ok;
}

int test_d1(void *state) {
	errr r = parser_parse(state, "D:baz");
	eq(r, 0);
	r = parser_parse(state, "D:quxx");
	eq(r, 0);
	ok;
}

const char *suite_name = "parse/names";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "d0", test_d0 },

	{ "n1", test_n1 },
	{ "d1", test_d1 },

	{ NULL, NULL }
};
