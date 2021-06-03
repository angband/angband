/* parse/v-info */

#include "unit-test.h"

#include "init.h"
#include "cave.h"
#include "generate.h"


int setup_tests(void **state) {
	*state = init_parse_vault();
	return !*state;
}

int teardown_tests(void *state) {
	struct vault *v = parser_priv(state);
	string_free(v->name);
	string_free(v->text);
	string_free(v->typ);
	mem_free(v);
	parser_destroy(state);
	return 0;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:round");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	require(v);
	require(streq(v->name, "round"));
	ok;
}

static int test_typ0(void *state) {
	enum parser_error r = parser_parse(state, "type:Lesser vault");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	require(v);
	require(streq(v->typ, "Lesser vault"));
	ok;
}

static int test_rat0(void *state) {
	enum parser_error r = parser_parse(state, "rating:5");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	eq(v->rat, 5);
	require(v);
	ok;
}

static int test_hgt0(void *state) {
	enum parser_error r = parser_parse(state, "rows:12");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	eq(v->hgt, 12);
	require(v);
	ok;
}

static int test_wid0(void *state) {
	enum parser_error r = parser_parse(state, "columns:6");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	eq(v->wid, 6);
	require(v);
	ok;
}

static int test_min_lev0(void *state) {
	enum parser_error r = parser_parse(state, "min-depth:15");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	eq(v->min_lev, 15);
	require(v);
	ok;
}

static int test_max_lev0(void *state) {
	enum parser_error r = parser_parse(state, "max-depth:25");
	struct vault *v;

	eq(r, PARSE_ERROR_NONE);
	v = parser_priv(state);
	eq(v->max_lev, 25);
	require(v);
	ok;
}

static int test_d0(void *state) {
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
	{ "name0", test_name0 },
	{ "typ0", test_typ0 },
	{ "rat0", test_rat0 },
	{ "hgt0", test_hgt0 },
	{ "wid0", test_wid0 },
	{ "min_lev0", test_min_lev0 },
	{ "max_lev0", test_max_lev0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
