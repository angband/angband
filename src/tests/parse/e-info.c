/* parse/e-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "object/tvalsval.h"
#include "init.h"
#include "types.h"

int setup_tests(void **state) {
	*state = init_parse_e();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_order(void *state) {
	enum parser_error r = parser_parse(state, "X:3:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:5:of Resist Lightning");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->eidx, 5);
	require(streq(e->name, "of Resist Lightning"));
	ok;
}

int test_w0(void *state) {
	enum parser_error r = parser_parse(state, "W:2:4:6:8");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->level, 2);
	eq(e->rarity, 4);
	eq(e->cost, 8);
	return PARSE_ERROR_NONE;
}

int test_x0(void *state) {
	enum parser_error r = parser_parse(state, "X:5:1");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->rating, 5);
	eq(e->xtra, 1);
	ok;
}

int test_t0(void *state) {
	enum parser_error r = parser_parse(state, "T:22:2:13");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->tval[0], 22);
	eq(e->min_sval[0], 2);
	eq(e->max_sval[0], 13);
	ok;
}

/* Broken: lookup_sval() requires k_info, z_info */
int test_t1(void *state) {
	enum parser_error r = parser_parse(state, "T:sword:dagger:scimitar");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->tval[1], TV_SWORD);
	eq(e->min_sval[1], SV_DAGGER);
	eq(e->max_sval[1], SV_SCIMITAR);
	ok;
}

int test_c0(void *state) {
	enum parser_error r = parser_parse(state, "C:1d2:3d4:5d6");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->to_h.dice, 1);
	eq(e->to_h.sides, 2);
	eq(e->to_d.dice, 3);
	eq(e->to_d.sides, 4);
	eq(e->to_a.dice, 5);
	eq(e->to_a.sides, 6);
	ok;
}

int test_l0(void *state) {
	enum parser_error r = parser_parse(state, "L:1+2d3M4:5:STR | INT");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->pval[0].base, 1);
	eq(e->pval[0].dice, 2);
	eq(e->pval[0].sides, 3);
	eq(e->pval[0].m_bonus, 4);
	eq(e->min_pval[0], 5);
	require(e->pval_flags[0]);
	ok;
}

int test_m0(void *state) {
	enum parser_error r = parser_parse(state, "M:10:13:4");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	eq(e->min_to_h, 10);
	eq(e->min_to_d, 13);
	eq(e->min_to_a, 4);
	ok;
}

int test_f0(void *state) {
	enum parser_error r = parser_parse(state, "F:SEE_INVIS");
	struct ego_item *e;

	eq(r, PARSE_ERROR_NONE);
	e = parser_priv(state);
	require(e);
	require(e->flags);
	ok;
}

int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:foo");
	struct ego_item *e;
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(state, "D: bar");
	eq(r, PARSE_ERROR_NONE);

	e = parser_priv(state);
	require(e);
	require(streq(e->text, "foo bar"));
	ok;
}

const char *suite_name = "parse/e-info";
struct test tests[] = {
	{ "order", test_order },
	{ "n0", test_n0 },
	{ "w0", test_w0 },
	{ "x0", test_x0 },
	{ "t0", test_t0 },
	/* { "t1", test_t1 }, */
	{ "c0", test_c0 },
	{ "m0", test_m0 },
	{ "f0", test_f0 },
	{ "d0", test_d0 },
	{ "l0", test_l0 },
	{ NULL, NULL }
};
