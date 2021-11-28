/* parse/k-info */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "obj-tval.h"


int setup_tests(void **state) {
	*state = init_parse_object();
	/* Do the bare minimum so the sval assignment will work. */
	kb_info = mem_zalloc(TV_MAX * sizeof(*kb_info));
	kb_info[TV_FOOD].tval = TV_FOOD;
	return !*state;
}

int teardown_tests(void *state) {
	struct object_kind *k = parser_priv(state);
	string_free(k->name);
	string_free(k->text);
	mem_free(k);
	mem_free(kb_info);
	parser_destroy(state);
	return 0;
}

static int test_name0(void *state) {
	errr r = parser_parse(state, "name:Test Object Kind");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	require(streq(k->name, "Test Object Kind"));
	ok;
}

static int test_graphics0(void *state) {
	errr r = parser_parse(state, "graphics:~:red");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->d_char, L'~');
	eq(k->d_attr, COLOUR_RED);
	ok;
}

static int test_graphics1(void *state) {
	errr r = parser_parse(state, "graphics:!:W");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->d_char, L'!');
	eq(k->d_attr, COLOUR_L_WHITE);
	ok;
}

static int test_type0(void *state) {
	int previous_sval = kb_info[TV_FOOD].num_svals;
	errr r = parser_parse(state, "type:food");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->tval, TV_FOOD);
	eq(k->sval, previous_sval + 1);
	ok;
}

static int test_level0(void *state) {
	errr r = parser_parse(state, "level:10");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->level, 10);
	ok;
}

static int test_weight0(void *state) {
	errr r = parser_parse(state, "weight:5");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->weight, 5);
	ok;
}

static int test_cost0(void *state) {
	errr r = parser_parse(state, "cost:120");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->cost, 120);
	ok;
}

static int test_alloc0(void *state) {
	errr r = parser_parse(state, "alloc:3:4 to 6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->alloc_prob, 3);
	eq(k->alloc_min, 4);
	eq(k->alloc_max, 6);
	ok;
}

static int test_attack0(void *state) {
	errr r = parser_parse(state, "attack:4d8:1d4:2d5");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->dd, 4);
	eq(k->ds, 8);
	eq(k->to_h.dice, 1);
	eq(k->to_h.sides, 4);
	eq(k->to_d.dice, 2);
	eq(k->to_d.sides, 5);
	ok;
}

static int test_armor0(void *state) {
	errr r = parser_parse(state, "armor:3:7d6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->ac, 3);
	eq(k->to_a.dice, 7);
	eq(k->to_a.sides, 6);
	ok;
}

static int test_charges0(void *state) {
	errr r = parser_parse(state, "charges:2d8");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->charge.dice, 2);
	eq(k->charge.sides, 8);
	ok;
}

static int test_pile0(void *state) {
	errr r = parser_parse(state, "pile:4:3d6");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->gen_mult_prob, 4);
	eq(k->stack_size.dice, 3);
	eq(k->stack_size.sides, 6);
	ok;
}

static int test_flags0(void *state) {
	errr r = parser_parse(state, "flags:EASY_KNOW | FEATHER");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	require(k->flags);
	require(k->kind_flags);
	eq(of_has(k->flags, OF_FEATHER), 1);
	eq(of_has(k->flags, OF_SLOW_DIGEST), 0);
	eq(kf_has(k->kind_flags, KF_EASY_KNOW), 1);
	eq(kf_has(k->kind_flags, KF_INSTA_ART), 0);
	ok;
}

static int test_pval0(void *state) {
	errr r = parser_parse(state, "pval:1+2d3M4");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->pval.base, 1);
	eq(k->pval.dice, 2);
	eq(k->pval.sides, 3);
	eq(k->pval.m_bonus, 4);
	ok;
}

static int test_time0(void *state) {
	errr r = parser_parse(state, "time:4d5");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	eq(k->time.dice, 4);
	eq(k->time.sides, 5);
	ok;
}

static int test_desc0(void *state) {
	errr r = parser_parse(state, "desc:foo bar");
	struct object_kind *k;

	eq(r, 0);
	k = parser_priv(state);
	require(k);
	require(k->text);
	require(streq(k->text, "foo bar"));
	r = parser_parse(state, "desc: baz");
	eq(r, 0);
	ptreq(k, parser_priv(state));
	require(streq(k->text, "foo bar baz"));
	ok;
}

const char *suite_name = "parse/k-info";
struct test tests[] = {
	{ "name0", test_name0 },
	{ "graphics0", test_graphics0 },
	{ "graphics1", test_graphics1 },
	{ "type0", test_type0 },
	{ "level0", test_level0 },
	{ "weight0", test_weight0 },
	{ "cost0", test_cost0 },
	{ "alloc0", test_alloc0 },
	{ "attack0", test_attack0 },
	{ "armor0", test_armor0 },
	{ "charges0", test_charges0 },
	{ "pile0", test_pile0 },
	{ "flags0", test_flags0 },
	{ "time0", test_time0 },
	{ "desc0", test_desc0 },
	{ "pval0", test_pval0 },
	{ NULL, NULL }
};
