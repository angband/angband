/* parse/store */

#include "unit-test.h"

#include "store.h"

int setup_tests(void **state) {
	*state = store_parser_new();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_s0(void *state) {
	enum parser_error r = parser_parse(state, "S:2:33");
	struct store *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	eq(s->sidx, 1);
	eq(s->table_size, 33);
	ok;
}

/* Causes segfault: lookup_name() requires z_info/k_info */
int test_i0(void *state) {
	enum parser_error r = parser_parse(state, "I:2:3:5");
	struct store *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	require(s->table[0]);
	require(s->table[1]);
	ok;
}

const char *suite_name = "parse/store";
struct test tests[] = {
	{ "s0", test_s0 },
/*	{ "i0", test_i0 }, */
	{ NULL, NULL }
};
