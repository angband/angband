/* parse/r-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "init.h"
#include "monster.h"

int setup_tests(void **state) {
	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->max_sight = 20;
	*state = init_parse_monster();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	mem_free(z_info);
	return 0;
}

int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:544:Carcharoth, the Jaws of Thirst");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->ridx, 544);
	require(streq(mr->name, "Carcharoth, the Jaws of Thirst"));
	ok;
}

int test_base0(void *state) {
	enum parser_error r;
	struct monster_race *mr;

	rb_info = &test_rb_info;
	r = parser_parse(state, "base:townsfolk");
	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(streq(mr->base->name, "townsfolk"));
	ok;
}

int test_color0(void *state) {
	enum parser_error r = parser_parse(state, "color:v");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->d_attr, COLOUR_VIOLET);
	ok;
}

int test_info0(void *state) {
	enum parser_error r = parser_parse(state, "info:7:500:80:22:3");
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

int test_power0(void *state) {
	enum parser_error r = parser_parse(state, "power:42:11:27:6:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->level, 42);
	eq(mr->rarity, 11);
	eq(mr->power, 27);
	eq(mr->scaled_power, 6);
	eq(mr->mexp, 4);
	ok;
}

int test_blow0(void *state) {
	enum parser_error r = parser_parse(state, "blow:CLAW:FIRE:9d12");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->blow[0].method);
	require(mr->blow[0].effect);
	eq(mr->blow[0].dice.dice, 9);
	eq(mr->blow[0].dice.sides, 12);
	ok;
}

int test_blow1(void *state) {
	enum parser_error r = parser_parse(state, "blow:BITE:FIRE:6d8:0");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->blow[0].next->method);
	require(mr->blow[0].next->effect);
	eq(mr->blow[0].next->dice.dice, 6);
	eq(mr->blow[0].next->dice.sides, 8);
	ok;
}

int test_flags0(void *state) {
	enum parser_error r = parser_parse(state, "flags:UNIQUE | MALE");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->flags);
	ok;
}

int test_desc0(void *state) {
	enum parser_error r = parser_parse(state, "desc:foo bar ");
	enum parser_error s = parser_parse(state, "desc: baz");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	eq(s, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(streq(mr->text, "foo bar  baz"));
	ok;
}

int test_spell_freq0(void *state) {
	enum parser_error r = parser_parse(state, "spell-freq:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->freq_spell, 25);
	eq(mr->freq_innate, 25);
	ok;
}

int test_spells0(void *state) {
	enum parser_error r = parser_parse(state, "spells:BR_DARK | S_HOUND");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->spell_flags);
	ok;
}

const char *suite_name = "parse/r-info";
struct test tests[] = {
	{ "name0", test_name0 },
	{ "color0", test_color0 },
	{ "base0", test_base0 },
	{ "info0", test_info0 },
	{ "power0", test_power0 },
	{ "blow0", test_blow0 },
	{ "blow1", test_blow1 },
	{ "flags0", test_flags0 },
	{ "desc0", test_desc0 },
	{ "spell-freq0", test_spell_freq0 },
	{ "spells0", test_spells0 },
	{ NULL, NULL }
};
