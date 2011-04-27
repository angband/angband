/* parse/flavor.c */

#include "unit-test.h"

#include "init.h"
#include "object/tvalsval.h"
#include "object/obj-flag.h"
#include "object/object.h"
#include "z-term.h"

int setup_tests(void **state) {
	*state = init_parse_flavor();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:1:3:5");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->fidx, 1);
	eq(f->tval, 3);
	eq(f->sval, 5);
	ok;
}

int test_n1(void *state) {
	enum parser_error r = parser_parse(state, "N:2:light");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->fidx, 2);
	eq(f->tval, TV_LIGHT);
	eq(f->sval, SV_UNKNOWN);
	ok;
}

int test_g0(void *state) {
	enum parser_error r = parser_parse(state, "G:!:y");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->fidx, 2);
	eq(f->d_char, '!');
	eq(f->d_attr, TERM_YELLOW);
	ok;
}

int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:foo");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(state, "D: bar");
	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(streq(f->text, "foo bar"));
	eq(f->fidx, 2);
	ok;
}

const char *suite_name = "parse/flavor";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "n1", test_n1 },
	{ "g0", test_g0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
