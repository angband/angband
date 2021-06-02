/* parse/p-info */

#include "unit-test.h"
#include "init.h"
#include "player.h"


int setup_tests(void **state) {
	*state = init_parse_p_race();
	return !*state;
}

int teardown_tests(void *state) {
	struct player_race *pr = parser_priv(state);
	string_free((char *)pr->name);
	mem_free(pr);
	parser_destroy(state);
	return 0;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:Half-Elf");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	require(streq(pr->name, "Half-Elf"));
	ok;
}

static int test_stats0(void *state) {
	enum parser_error r = parser_parse(state, "stats:1:-1:2:-2:3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_adj[STAT_STR], 1);
	eq(pr->r_adj[STAT_INT], -1);
	eq(pr->r_adj[STAT_WIS], 2);
	eq(pr->r_adj[STAT_DEX], -2);
	eq(pr->r_adj[STAT_CON], 3);
	ok;
}

static int test_skill_disarm0(void *state) {
	enum parser_error r = parser_parse(state, "skill-disarm-magic:1");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_DISARM_MAGIC], 1);
	ok;
}

static int test_skill_device0(void *state) {
	enum parser_error r = parser_parse(state, "skill-device:3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_DEVICE], 3);
	ok;
}

static int test_skill_save0(void *state) {
	enum parser_error r = parser_parse(state, "skill-save:5");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_SAVE], 5);
	ok;
}

static int test_skill_stealth0(void *state) {
	enum parser_error r = parser_parse(state, "skill-stealth:7");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_STEALTH], 7);
	ok;
}

static int test_skill_search0(void *state) {
	enum parser_error r = parser_parse(state, "skill-search:9");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_SEARCH], 9);
	ok;
}

static int test_skill_melee0(void *state) {
	enum parser_error r = parser_parse(state, "skill-melee:4");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_TO_HIT_MELEE], 4);
	ok;
}

static int test_skill_shoot0(void *state) {
	enum parser_error r = parser_parse(state, "skill-shoot:6");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_TO_HIT_BOW], 6);
	ok;
}

static int test_skill_throw0(void *state) {
	enum parser_error r = parser_parse(state, "skill-throw:8");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_TO_HIT_THROW], 8);
	ok;
}

static int test_skill_dig0(void *state) {
	enum parser_error r = parser_parse(state, "skill-dig:10");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_DIGGING], 10);
	ok;
}

static int test_hitdie0(void *state) {
	enum parser_error r = parser_parse(state, "hitdie:10");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_mhp, 10);
	ok;
}

static int test_exp0(void *state) {
	enum parser_error r = parser_parse(state, "exp:20");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_exp, 20);
	ok;
}

static int test_infravision0(void *state) {
	enum parser_error r = parser_parse(state, "infravision:80");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->infra, 80);
	ok;
}

static int test_history0(void *state) {
	enum parser_error r = parser_parse(state, "history:0");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	null(pr->history);
	ok;
}

static int test_age0(void *state) {
	enum parser_error r = parser_parse(state, "age:10:3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->b_age, 10);
	eq(pr->m_age, 3);
	ok;
}

static int test_height0(void *state) {
	enum parser_error r = parser_parse(state, "height:10:2");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->base_hgt, 10);
	eq(pr->mod_hgt, 2);
	ok;
}

static int test_weight0(void *state) {
	enum parser_error r = parser_parse(state, "weight:80:10");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->base_wgt, 80);
	eq(pr->mod_wgt, 10);
	ok;
}

static int test_obj_flags0(void *state) {
	enum parser_error r = parser_parse(state, "obj-flags:SUST_DEX");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	require(pr->flags);
	ok;
}

static int test_play_flags0(void *state) {
	enum parser_error r = parser_parse(state, "player-flags:KNOW_ZAPPER");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	require(pr->pflags);
	ok;
}

const char *suite_name = "parse/p-info";
struct test tests[] = {
	{ "name0", test_name0 },
	{ "stats0", test_stats0 },
	{ "skill_disarm0", test_skill_disarm0 },
	{ "skill_device0", test_skill_device0 },
	{ "skill_save0", test_skill_save0 },
	{ "skill_stealth0", test_skill_stealth0 },
	{ "skill_search0", test_skill_search0 },
	{ "skill_melee0", test_skill_melee0 },
	{ "skill_shoot0", test_skill_shoot0 },
	{ "skill_throw0", test_skill_throw0 },
	{ "skill_dig0", test_skill_dig0 },
	{ "hitdie0", test_hitdie0 },
	{ "exp0", test_exp0 },
	{ "infravision0", test_infravision0 },
	{ "history0", test_history0 },
	{ "age0", test_age0 },
	{ "height0", test_height0 },
	{ "weight0", test_weight0 },
	{ "object_flags0", test_obj_flags0 },
	{ "player_flags0", test_play_flags0 },
	{ NULL, NULL }
};
