/* parse/f-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "init.h"
#include "types.h"

int setup_tests(void **state) {
	*state = init_parse_f();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:3:Test Feature");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(streq(f->name, "Test Feature"));
	eq(f->fidx, 3);
	eq(f->mimic, 3);
	ok;
}

int test_g0(void *state) {
	enum parser_error r = parser_parse(state, "G:::red");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->d_char, ':');
	eq(f->d_attr, TERM_RED);
	ok;
}

int test_m0(void *state) {
	enum parser_error r = parser_parse(state, "M:11");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->mimic, 11);
	ok;
}

int test_p0(void *state) {
	enum parser_error r = parser_parse(state, "P:2");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->priority, 2);
	ok;
}

int test_f0(void *state) {
	enum parser_error r = parser_parse(state, "F:MWALK | LOOK");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(f->flags);
	ok;
}

int test_x0(void *state) {
	enum parser_error r = parser_parse(state, "X:3:5:9:2");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->locked, 3);
	eq(f->jammed, 5);
	eq(f->shopnum, 9);
	eq(f->dig, 2);
	ok;
}

int test_e0(void *state) {
	enum parser_error r = parser_parse(state, "E:TRAP_PIT");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(f->effect);
	ok;
}

const char *suite_name = "parse/f-info";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "g0", test_g0 },
	{ "m0", test_m0 },
	{ "p0", test_p0 },
	{ "f0", test_f0 },
	{ "x0", test_x0 },
	{ "e0", test_e0 },
	{ NULL, NULL }
};
