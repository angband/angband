/* parse/p-info */

#include "unit-test.h"
#include "init.h"
#include "player.h"


int setup_tests(void **state) {
	*state = init_parse_p_race();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:1:Half-Elf");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->ridx, 1);
	require(streq(pr->name, "Half-Elf"));
	ok;
}

int test_stats0(void *state) {
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

int test_skills0(void *state) {
	enum parser_error r = parser_parse(state, "skills:1:3:5:7:9:2:4:6:8:10");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_skills[SKILL_DISARM], 1);
	eq(pr->r_skills[SKILL_DEVICE], 3);
	eq(pr->r_skills[SKILL_SAVE], 5);
	eq(pr->r_skills[SKILL_STEALTH], 7);
	eq(pr->r_skills[SKILL_SEARCH], 9);
	eq(pr->r_skills[SKILL_SEARCH_FREQUENCY], 2);
	eq(pr->r_skills[SKILL_TO_HIT_MELEE], 4);
	eq(pr->r_skills[SKILL_TO_HIT_BOW], 6);
	eq(pr->r_skills[SKILL_TO_HIT_THROW], 8);
	eq(pr->r_skills[SKILL_DIGGING], 10);
	ok;
}

int test_info0(void *state) {
	enum parser_error r = parser_parse(state, "info:10:20:80");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_mhp, 10);
	eq(pr->r_exp, 20);
	eq(pr->infra, 80);
	ok;
}

int test_history0(void *state) {
	enum parser_error r = parser_parse(state, "history:0:10:3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	ptreq(pr->history, NULL);
	eq(pr->b_age, 10);
	eq(pr->m_age, 3);
	ok;
}

int test_height0(void *state) {
	enum parser_error r = parser_parse(state, "height:10:2:11:3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->m_b_ht, 10);
	eq(pr->m_m_ht, 2);
	eq(pr->f_b_ht, 11);
	eq(pr->f_m_ht, 3);
	ok;
}

int test_weight0(void *state) {
	enum parser_error r = parser_parse(state, "weight:80:10:75:7");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->m_b_wt, 80);
	eq(pr->m_m_wt, 10);
	eq(pr->f_b_wt, 75);
	eq(pr->f_m_wt, 7);
	ok;
}

int test_obj_flags0(void *state) {
	enum parser_error r = parser_parse(state, "obj-flags:SUST_DEX");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	require(pr->flags);
	ok;
}

int test_play_flags0(void *state) {
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
	{ "skills0", test_skills0 },
	{ "info0", test_info0 },
	{ "history0", test_history0 },
	{ "height0", test_height0 },
	{ "weight0", test_weight0 },
	{ "object_flags0", test_obj_flags0 },
	{ "player_flags0", test_play_flags0 },
	{ NULL, NULL }
};
