/* parse/a-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "object/tvalsval.h"
#include "init.h"
#include "types.h"

int setup_tests(void **state) {
	*state = init_parse_a();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:3:of Thrain");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->aidx, 3);
	require(streq(a->name, "of Thrain"));
	ok;
}

int test_badtval0(void *state) {
	enum parser_error r = parser_parse(state, "I:badtval:6:3");
	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

int test_badtval1(void *state) {
	enum parser_error r = parser_parse(state, "I:-1:6:3");
	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

/* Causes segfault: lookup_sval() requires z_info/k_info */
int test_badsval(void *state) {
	errr r = parser_parse(state, "I:light:badsval:3");
	eq(r, PARSE_ERROR_UNRECOGNISED_SVAL);
	ok;
}

int test_badsval1(void *state) {
	enum parser_error r = parser_parse(state, "I:light:-2:3");
	eq(r, PARSE_ERROR_UNRECOGNISED_SVAL);
	ok;
}

int test_i0(void *state) {
	enum parser_error r = parser_parse(state, "I:light:6");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->tval, TV_LIGHT);
	eq(a->sval, 6);
	ok;
}

int test_w0(void *state) {
	enum parser_error r = parser_parse(state, "W:3:5:8:200");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->level, 3);
	eq(a->rarity, 5);
	eq(a->weight, 8);
	eq(a->cost, 200);
	ok;
}

int test_a0(void *state) {
	enum parser_error r = parser_parse(state, "A:3:5");
	eq(r, PARSE_ERROR_GENERIC);
	ok;
}

int test_a1(void *state) {
	enum parser_error r = parser_parse(state, "A:3:5 to 300");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	ok;
}

int test_a2(void *state) {
	enum parser_error r = parser_parse(state, "A:3:5 to 10");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->alloc_prob, 3);
	eq(a->alloc_min, 5);
	eq(a->alloc_max, 10);
	ok;
}

int test_p0(void *state) {
	enum parser_error r = parser_parse(state, "P:3:4d5:8:2:1");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->ac, 3);
	eq(a->dd, 4);
	eq(a->ds, 5);
	eq(a->to_h, 8);
	eq(a->to_d, 2);
	eq(a->to_a, 1);
	ok;
}

int test_f0(void *state) {
	enum parser_error r = parser_parse(state, "F:SEE_INVIS | HOLD_LIFE");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	require(a->flags);
	ok;
}

int test_l0(void *state) {
	enum parser_error r = parser_parse(state, "L:17:STR | CON");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	eq(a->pval[0], 17);
	require(a->pval_flags[0]);
	ok;
}

int test_e0(void *state) {
	enum parser_error r = parser_parse(state, "E:DETECT_ALL:20+d30");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	require(a->effect);
	eq(a->time.base, 20);
	eq(a->time.sides, 30);
	ok;
}

int test_m0(void *state) {
	enum parser_error r = parser_parse(state, "M:foo");
	struct artifact *a;

	eq(r, 0);
	r = parser_parse(state, "M:bar");
	eq(r, 0);
	a = parser_priv(state);
	require(a);
	require(streq(a->effect_msg, "foobar"));
	ok;
}

int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:baz");
	struct artifact *a;

	eq(r, 0);
	r = parser_parse(state, "D: quxx");
	eq(r, 0);
	a = parser_priv(state);
	require(a);
	require(streq(a->text, "baz quxx"));
	ok;
}

const char *suite_name = "parse/a-info";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "badtval0", test_badtval0 },
	{ "badtval1", test_badtval1 },
/*	{ "badsval0", test_badsval0 }, */
	{ "badsval1", test_badsval1 },
	{ "i0", test_i0 },
	{ "w0", test_w0 },
	{ "a0", test_a0 },
	{ "a1", test_a1 },
	{ "a2", test_a2 },
	{ "p0", test_p0 },
	{ "f0", test_f0 },
	{ "e0", test_e0 },
	{ "m0", test_m0 },
	{ "d0", test_d0 },
	{ "l0", test_l0 },
	{ NULL, NULL }
};
