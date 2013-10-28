/* parse/v-info */

#include "unit-test.h"

#include "init.h"
#include "types.h"

int setup_tests(void **state) {
	*state = init_parse_v();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:1:round");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	require(v);
	eq(v->vidx, 1);
	require(streq(v->name, "round"));
	ok;
}

int test_x0(void *state) {
	enum parser_error r = parser_parse(state, "X:6:5:12:20");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	require(v);
	eq(v->typ, 6);
	eq(v->rat, 5);
	eq(v->hgt, 12);
	eq(v->wid, 20);
	ok;
}

int test_d0(void *state) {
	enum parser_error r0 = parser_parse(state, "D:  %%  ");
	enum parser_error r1 = parser_parse(state, "D: %  % ");
	struct vault *v;

	eq(r0, PARSE_ERROR_NONE);
	eq(r1, PARSE_ERROR_NONE);
	v = parser_priv(state);
	require(v);
	require(streq(v->text, "  %%   %  % "));
	ok;
}

const char *suite_name = "parse/v-info";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "x0", test_x0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
