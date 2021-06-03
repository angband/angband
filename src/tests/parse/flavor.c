/* parse/flavor.c */

#include "unit-test.h"

#include "init.h"
#include "obj-tval.h"
#include "obj-properties.h"
#include "object.h"
#include "z-color.h"

int setup_tests(void **state) {
	*state = init_parse_flavor();
	return !*state;
}

int teardown_tests(void *state) {
	struct flavor *f = parser_priv(state);
	string_free(f->text);
	mem_free(f);
	parser_destroy(state);
	return 0;
}

static int test_kind0(void *state) {
	enum parser_error r = parser_parse(state, "kind:light:&");

	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_flavor0(void *state) {
	enum parser_error r = parser_parse(state, "flavor:2:blue:Fishy");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->fidx, 2);
	eq(f->tval, TV_LIGHT);
	eq(f->d_char, L'&');
	eq(f->d_attr, COLOUR_BLUE);
	require(streq(f->text, "Fishy"));
	ok;
}

const char *suite_name = "parse/flavor";
struct test tests[] = {
	{ "kind0", test_kind0 },
	{ "flavor0", test_flavor0 },
	{ NULL, NULL }
};
