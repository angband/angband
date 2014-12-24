/* parse/f-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "init.h"


int setup_tests(void **state) {
	*state = init_parse_feat();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:3:Test Feature");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(streq(f->name, "Test Feature"));
	eq(f->fidx, 3);
	eq(f->mimic, 3);
	ok;
}

int test_graphics0(void *state) {
	enum parser_error r = parser_parse(state, "graphics:::red");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->d_char, L':');
	eq(f->d_attr, COLOUR_RED);
	ok;
}

int test_mimic0(void *state) {
	enum parser_error r = parser_parse(state, "mimic:11");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->mimic, 11);
	ok;
}

int test_priority0(void *state) {
	enum parser_error r = parser_parse(state, "priority:2");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->priority, 2);
	ok;
}

int test_flags0(void *state) {
	enum parser_error r = parser_parse(state, "flags:LOS | PERMANENT | DOWNSTAIR");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	require(f->flags);
	ok;
}

int test_info0(void *state) {
	enum parser_error r = parser_parse(state, "info:9:2");
	struct feature *f;

	eq(r, PARSE_ERROR_NONE);
	f = parser_priv(state);
	require(f);
	eq(f->shopnum, 9);
	eq(f->dig, 2);
	ok;
}

const char *suite_name = "parse/f-info";
struct test tests[] = {
	{ "name0", test_name0 },
	{ "graphics0", test_graphics0 },
	{ "mimic0", test_mimic0 },
	{ "priority0", test_priority0 },
	{ "flags0", test_flags0 },
	{ "info0", test_info0 },
	{ NULL, NULL }
};
