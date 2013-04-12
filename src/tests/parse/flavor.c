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

int test_kind0(void *state) {
	enum parser_error r = parser_parse(state, "kind:light:&");

	eq(r, PARSE_ERROR_NONE);
	ok;
}

int test_flavor0(void *state) {
	enum parser_error r = parser_parse(state, "flavor:2:blue:Fishy");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->fidx, 2);
	eq(f->tval, TV_LIGHT);
	eq(f->d_char, L'&');
	eq(f->d_attr, TERM_BLUE);
	require(streq(f->text, "Fishy"));
	ok;
}

const char *suite_name = "parse/flavor";
struct test tests[] = {
	{ "kind0", test_kind0 },
	{ "flavor0", test_flavor0 },
	{ NULL, NULL }
};
