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
	struct monster_race *mr = parser_priv(state);
	string_free(mr->name);
	string_free(mr->text);
	mem_free(mr);
	parser_destroy(state);
	mem_free(z_info);
	return 0;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:Carcharoth, the Jaws of Thirst");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(streq(mr->name, "Carcharoth, the Jaws of Thirst"));
	ok;
}

static int test_base0(void *state) {
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

static int test_color0(void *state) {
	enum parser_error r = parser_parse(state, "color:v");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->d_attr, COLOUR_VIOLET);
	ok;
}

static int test_speed0(void *state) {
	enum parser_error r = parser_parse(state, "speed:7");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->speed, 7);
	ok;
}

static int test_hp0(void *state) {
	enum parser_error r = parser_parse(state, "hit-points:500");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->avg_hp, 500);
	ok;
}

static int test_hearing0(void *state) {
	enum parser_error r = parser_parse(state, "hearing:80");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->hearing, 80);
	ok;
}

static int test_smell0(void *state) {
	enum parser_error r = parser_parse(state, "smell:30");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->smell, 30);
	ok;
}

static int test_ac0(void *state) {
	enum parser_error r = parser_parse(state, "armor-class:22");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->ac, 22);
	ok;
}

static int test_sleep0(void *state) {
	enum parser_error r = parser_parse(state, "sleepiness:3");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->sleep, 3);
	ok;
}

static int test_depth0(void *state) {
	enum parser_error r = parser_parse(state, "depth:42");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->level, 42);
	ok;
}

static int test_rarity0(void *state) {
	enum parser_error r = parser_parse(state, "rarity:11");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->rarity, 11);
	ok;
}

static int test_mexp0(void *state) {
	enum parser_error r = parser_parse(state, "experience:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->mexp, 4);
	ok;
}

/* Without initialization of the blow_methods array, this crashes so not run. */
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

/* Without initialization of the blow_methods array, this crashes so not run. */
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

static int test_flags0(void *state) {
	enum parser_error r = parser_parse(state, "flags:UNIQUE | MALE");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	require(mr->flags);
	ok;
}

static int test_desc0(void *state) {
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

static int test_innate_freq0(void *state) {
	enum parser_error r = parser_parse(state, "innate-freq:10");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->freq_innate, 10);
	ok;
}

static int test_spell_freq0(void *state) {
	enum parser_error r = parser_parse(state, "spell-freq:4");
	struct monster_race *mr;

	eq(r, PARSE_ERROR_NONE);
	mr = parser_priv(state);
	require(mr);
	eq(mr->freq_spell, 25);
	ok;
}

static int test_spells0(void *state) {
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
	{ "speed0", test_speed0 },
	{ "hp0", test_hp0 },
	{ "hearing0", test_hearing0 },
	{ "smell0", test_smell0 },
	{ "ac0", test_ac0 },
	{ "sleep0", test_sleep0 },
	{ "depth0", test_depth0 },
	{ "rarity0", test_rarity0 },
	{ "mexp0", test_mexp0 },
	//{ "blow0", test_blow0 },
	//{ "blow1", test_blow1 },
	{ "flags0", test_flags0 },
	{ "desc0", test_desc0 },
	{ "innate-freq0", test_innate_freq0 },
	{ "spell-freq0", test_spell_freq0 },
	{ "spells0", test_spells0 },
	{ NULL, NULL }
};
