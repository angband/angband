/* parse/p-info */

#include "unit-test.h"
#include "init.h"
#include "player/types.h"
#include "types.h"

int setup_tests(void **state) {
	*state = init_parse_p();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_n0(void *state) {
	enum parser_error r = parser_parse(state, "N:1:Half-Elf");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->ridx, 1);
	require(streq(pr->name, "Half-Elf"));
	ok;
}

int test_s0(void *state) {
	enum parser_error r = parser_parse(state, "S:1:-1:2:-2:3:-3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_adj[A_STR], 1);
	eq(pr->r_adj[A_INT], -1);
	eq(pr->r_adj[A_WIS], 2);
	eq(pr->r_adj[A_DEX], -2);
	eq(pr->r_adj[A_CON], 3);
	eq(pr->r_adj[A_CHR], -3);
	ok;
}

int test_r0(void *state) {
	enum parser_error r = parser_parse(state, "R:1:3:5:7:9:2:4:6:8:10");
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

int test_x0(void *state) {
	enum parser_error r = parser_parse(state, "X:10:20:80");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->r_mhp, 10);
	eq(pr->r_exp, 20);
	eq(pr->infra, 80);
	ok;
}

int test_i0(void *state) {
	enum parser_error r = parser_parse(state, "I:0:10:3");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	ptreq(pr->history, NULL);
	eq(pr->b_age, 10);
	eq(pr->m_age, 3);
	ok;
}

int test_h0(void *state) {
	enum parser_error r = parser_parse(state, "H:10:2:11:3");
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

int test_w0(void *state) {
	enum parser_error r = parser_parse(state, "W:80:10:75:7");
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

int test_f0(void *state) {
	enum parser_error r = parser_parse(state, "F:SUST_DEX");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	require(pr->flags);
	ok;
}

int test_y0(void *state) {
	enum parser_error r = parser_parse(state, "Y:KNOW_ZAPPER");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	require(pr->pflags);
	ok;
}

int test_c0(void *state) {
	enum parser_error r = parser_parse(state, "C:1|3|5");
	struct player_race *pr;

	eq(r, PARSE_ERROR_NONE);
	pr = parser_priv(state);
	require(pr);
	eq(pr->choice, (1 << 5) | (1 << 3) | (1 << 1));
	ok;
}

const char *suite_name = "parse/p-info";
struct test tests[] = {
	{ "n0", test_n0 },
	{ "s0", test_s0 },
	{ "r0", test_r0 },
	{ "x0", test_x0 },
	{ "i0", test_i0 },
	{ "h0", test_h0 },
	{ "w0", test_w0 },
	{ "f0", test_f0 },
	{ "y0", test_y0 },
	{ "c0", test_c0 },
	{ NULL, NULL }
};
