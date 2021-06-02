/* parse/a-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "obj-tval.h"
#include "object.h"
#include "init.h"
	
int setup_tests(void **state) {
	*state = init_parse_artifact();
	return !*state;
}

int teardown_tests(void *state) {
	struct artifact *a = parser_priv(state);
	string_free(a->name);
	string_free(a->text);
	string_free(a->alt_msg);
	mem_free(a);
	parser_destroy(state);
	return 0;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:of Thrain");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	require(streq(a->name, "of Thrain"));
	ok;
}

static int test_badtval0(void *state) {
	enum parser_error r = parser_parse(state, "base-object:badtval:Junk");
	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

static int test_badtval1(void *state) {
	enum parser_error r = parser_parse(state, "base-object:-1:Junk");
	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

static int test_base_object0(void *state) {
	enum parser_error r = parser_parse(state, "base-object:light:6");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->tval, TV_LIGHT);
	eq(a->sval, 6);
	ok;
}

static int test_level0(void *state) {
	enum parser_error r = parser_parse(state, "level:3");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->level, 3);
	ok;
}

/* Causes segfault: lookup_kind() requires z_info/k_info() */
int test_weight0(void *state) {
	enum parser_error r = parser_parse(state, "weight:8");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->weight, 8);
	ok;
}

static int test_cost0(void *state) {
	enum parser_error r = parser_parse(state, "cost:200");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->cost, 200);
	ok;
}

static int test_alloc0(void *state) {
	enum parser_error r = parser_parse(state, "alloc:3:5");
	eq(r, PARSE_ERROR_INVALID_ALLOCATION);
	ok;
}

static int test_alloc1(void *state) {
	enum parser_error r = parser_parse(state, "alloc:3:5 to 300");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	ok;
}

static int test_alloc2(void *state) {
	enum parser_error r = parser_parse(state, "alloc:3:5 to 10");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->alloc_prob, 3);
	eq(a->alloc_min, 5);
	eq(a->alloc_max, 10);
	ok;
}

static int test_attack0(void *state) {
	enum parser_error r = parser_parse(state, "attack:4d5:8:2");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->dd, 4);
	eq(a->ds, 5);
	eq(a->to_h, 8);
	eq(a->to_d, 2);
	ok;
}

static int test_armor0(void *state) {
	enum parser_error r = parser_parse(state, "armor:3:1");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->ac, 3);
	eq(a->to_a, 1);
	ok;
}

static int test_flags0(void *state) {
	enum parser_error r = parser_parse(state, "flags:SEE_INVIS | HOLD_LIFE");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	require(a->flags);
	ok;
}

static int test_values0(void *state) {
	enum parser_error r = parser_parse(state, "values:STR[1] | CON[1]");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	eq(a->modifiers[0], 1);
	eq(a->modifiers[4], 1);
	ok;
}

/* Causes segfault: lookup_kind() requires z_info/k_info */
int test_time0(void *state) {
	enum parser_error r = parser_parse(state, "time:20+d30");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->time.base, 20);
	eq(a->time.sides, 30);
	ok;
}

static int test_msg0(void *state) {
	enum parser_error r = parser_parse(state, "msg:foo");
	struct artifact *a;

	eq(r, 0);
	r = parser_parse(state, "msg:bar");
	eq(r, 0);
	a = parser_priv(state);
	require(a);
	require(streq(a->alt_msg, "foobar"));
	ok;
}


static int test_desc0(void *state) {
	enum parser_error r = parser_parse(state, "desc:baz");
	struct artifact *a;

	eq(r, 0);
	r = parser_parse(state, "desc: quxx");
	eq(r, 0);
	a = parser_priv(state);
	require(a);
	require(streq(a->text, "baz quxx"));
	ok;
}

const char *suite_name = "parse/a-info";
struct test tests[] = {
	{ "name0", test_name0 },
	{ "badtval0", test_badtval0 },
	{ "badtval1", test_badtval1 },
	{ "base-object0", test_base_object0 },
	{ "level0", test_level0 },
	//{ "weight0", test_weight0 },
	{ "cost0", test_cost0 },
	{ "alloc0", test_alloc0 },
	{ "alloc1", test_alloc1 },
	{ "alloc2", test_alloc2 },
	{ "attack0", test_attack0 },
	{ "armor0", test_armor0 },
	{ "flags0", test_flags0 },
	//{ "time0", test_time0 },
	{ "msg0", test_msg0 },
	{ "desc0", test_desc0 },
	{ "values0", test_values0 },
	{ NULL, NULL }
};
