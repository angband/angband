/* parse/k-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "types.h"

static int setup(void **state) {
	*state = init_parse_k();
	return !*state;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_n0(void *state) {
	errr r = parser_parse(state, "N:3:Test Object Kind");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->kidx, 3);
	require(streq(k->name, "Test Object Kind"));
	ok;
}

static int test_g0(void *state) {
	errr r = parser_parse(state, "G:~:red");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->d_char, '~');
	eq(k->d_attr, TERM_RED);
	ok;
}

static int test_g1(void *state) {
	errr r = parser_parse(state, "G:!:W");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->d_char, '!');
	eq(k->d_attr, TERM_L_WHITE);
	ok;
}

static int test_i0(void *state) {
	errr r = parser_parse(state, "I:4:2:3d6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->tval, 4);
	eq(k->sval, 2);
	eq(k->pval.dice, 3);
	eq(k->pval.sides, 6);
	ok;
}

static int test_i1(void *state) {
	errr r = parser_parse(state, "I:food:2:0");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->tval, TV_FOOD);
	eq(k->sval, 2);
	ok;
}

static int test_w0(void *state) {
	errr r = parser_parse(state, "W:10:0:5:120");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->level, 10);
	eq(k->weight, 5);
	eq(k->cost, 120);
	ok;
}

static int test_a0(void *state) {
	errr r = parser_parse(state, "A:3:4 to 6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->alloc_prob, 3);
	eq(k->alloc_min, 4);
	eq(k->alloc_max, 6);
	ok;
}

static int test_p0(void *state) {
	errr r = parser_parse(state, "P:3:4d8:1d4:2d5:7d6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->ac, 3);
	eq(k->dd, 4);
	eq(k->ds, 8);
	eq(k->to_h.dice, 1);
	eq(k->to_h.sides, 4);
	eq(k->to_d.dice, 2);
	eq(k->to_d.sides, 5);
	eq(k->to_a.dice, 7);
	eq(k->to_a.sides, 6);
	ok;
}

static int test_c0(void *state) {
	errr r = parser_parse(state, "C:2d8");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->charge.dice, 2);
	eq(k->charge.sides, 8);
	ok;
}

static int test_m0(void *state) {
	errr r = parser_parse(state, "M:4:3d6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->gen_mult_prob, 4);
	eq(k->stack_size.dice, 3);
	eq(k->stack_size.sides, 6);
	ok;
}

static int test_f0(void *state) {
	errr r = parser_parse(state, "F:STR");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	require(k->flags);
	ok;
}

static int test_e0(void *state) {
	errr r = parser_parse(state, "E:POISON:4d5");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	require(k->effect);
	eq(k->time.dice, 4);
	eq(k->time.sides, 5);
	ok;
}

static int test_d0(void *state) {
	errr r = parser_parse(state, "D:foo bar");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	require(k->text);
	require(streq(k->text, "foo bar"));
	r = parser_parse(state, "D: baz");
	eq(r, 0);
	eq(k, parser_priv(state));
	require(streq(k->text, "foo bar baz"));
	ok;
}

static const char *suite_name = "parse/k-info";
static struct test tests[] = {
	{ "n0", test_n0 },
	{ "g0", test_g0 },
	{ "g1", test_g1 },
	{ "i0", test_i0 },
	{ "i1", test_i1 },
	{ "w0", test_w0 },
	{ "a0", test_a0 },
	{ "p0", test_p0 },
	{ "c0", test_c0 },
	{ "m0", test_m0 },
	{ "f0", test_f0 },
	{ "e0", test_e0 },
	{ "d0", test_d0 },
	{ NULL, NULL }
};
