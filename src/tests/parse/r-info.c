/* parse/r-info */

#include "unit-test.h"
#include "init.h"
#include "monster/constants.h"
#include "monster/types.h"
#include "types.h"

static int setup(void **state) {
	*state = init_parse_r();
	return !*state;
}

static int teardown(void *state) {
	parser_destroy(state);
	return 0;
}

static int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:544:Carcharoth, the Jaws of Thirst");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->ridx, 544);
	require(streq(mr->name, "Carcharoth, the Jaws of Thirst"));
	ok;
}

static int test_g0(void *state) {
	enum parser_error r = parser_parse(state, "G:C:v");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->d_attr, TERM_VIOLET);
	eq(mr->d_char, 'C');
	ok;
}

static int test_i0(void *state) {
	enum parser_error r = parser_parse(state, "I:7:500:80:22:3");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->speed, 7);
	eq(mr->avg_hp, 500);
	eq(mr->aaf, 80);
	eq(mr->ac, 22);
	eq(mr->sleep, 3);
	ok;
}

static int test_w0(void *state) {
	enum parser_error r = parser_parse(state, "W:42:11:27:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->level, 42);
	eq(mr->rarity, 11);
	eq(mr->power, 27);
	eq(mr->mexp, 4);
	ok;
}

static int test_b0(void *state) {
	enum parser_error r = parser_parse(state, "B:CLAW:FIRE:9d12");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->blow[0].method);
	require(mr->blow[0].effect);
	eq(mr->blow[0].d_dice, 9);
	eq(mr->blow[0].d_side, 12);
	ok;
}

static int test_b1(void *state) {
	enum parser_error r = parser_parse(state, "B:BITE:FIRE:6d8");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->blow[1].method);
	require(mr->blow[1].effect);
	eq(mr->blow[1].d_dice, 6);
	eq(mr->blow[1].d_side, 8);
	ok;
}

static int test_f0(void *state) {
	enum parser_error r = parser_parse(state, "F:UNIQUE | MALE");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->flags);
	ok;
}

static int test_d0(void *state) {
	enum parser_error r = parser_parse(state, "D:foo bar ");
	enum parser_error s = parser_parse(state, "D: baz");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	eq(s, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(streq(mr->text, "foo bar  baz"));
	ok;
}

static int test_s0(void *state) {
	enum parser_error r = parser_parse(state, "S:1_IN_4 | BR_DARK | S_HOUND");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->freq_spell, 25);
	eq(mr->freq_innate, 25);
	require(mr->spell_flags);
	ok;
}

static const char *suite_name = "parse/r-info";
static struct test tests[] = {
	{ "n0", test_n0 },
	{ "g0", test_g0 },
	{ "i0", test_i0 },
	{ "w0", test_w0 },
	{ "b0", test_b0 },
	{ "b1", test_b1 },
	{ "f0", test_f0 },
	{ "d0", test_d0 },
	{ "s0", test_s0 },
	{ NULL, NULL }
};
