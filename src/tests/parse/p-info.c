/* parse/p-info */
/* Exercise parsing used for p_race.txt. */

#include "unit-test.h"
#include "init.h"
#include "object.h"
#include "player.h"


int setup_tests(void **state) {
	*state = p_race_parser.init();
	return !*state;
}

int teardown_tests(void *state) {
	struct player_race *pr = parser_priv(state);
	string_free((char *)pr->name);
	mem_free(pr);
	parser_destroy(state);
	return 0;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_race *pr = (struct player_race*) parser_priv(p);
	enum parser_error r;

	null(pr);
	r = parser_parse(p, "stats:0:1:-1:1:-1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-disarm-phys:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-disarm-magic:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-device:3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-save:3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-stealth:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-search:3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-melee:-1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-shoot:5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-throw:5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-dig:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "hitdie:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "exp:120");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "infravision:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "history:4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "age:24:16");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "height:71:8");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "weight:115:25");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "obj-flags:SUST_DEX");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "player-flags:KNOW_MUSHROOM");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "values:RES_LIGHT[1]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
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
	struct parser *p = (struct parser*) state;
	struct player_race *pr = (struct player_race*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[OF_SIZE];

	notnull(pr);
	of_wipe(pr->flags);
	/* Check that specifying no flags works. */
	r = parser_parse(p, "obj-flags:");
	eq(r, PARSE_ERROR_NONE);
	pr = (struct player_race*) parser_priv(p);
	notnull(pr);
	require(of_is_empty(pr->flags));
	/* Try one flag. */
	r = parser_parse(p, "obj-flags:SUST_DEX");
	eq(r, PARSE_ERROR_NONE);
	/* Try multiple flags at once. */
	r = parser_parse(p, "obj-flags:HOLD_LIFE | FREE_ACT");
	eq(r, PARSE_ERROR_NONE);
	of_wipe(eflags);
	of_on(eflags, OF_SUST_DEX);
	of_on(eflags, OF_HOLD_LIFE);
	of_on(eflags, OF_FREE_ACT);
	pr = (struct player_race*) parser_priv(p);
	notnull(pr);
	require(of_is_equal(pr->flags, eflags));
	ok;
}

static int test_obj_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized flag. */
	enum parser_error r = parser_parse(p, "obj-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_play_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_race *pr = (struct player_race*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[PF_SIZE];

	notnull(pr);
	pf_wipe(pr->flags);
	/* Check that specifying no flags works. */
	r = parser_parse(p, "player-flags:");
	eq(r, PARSE_ERROR_NONE);
	pr = (struct player_race*) parser_priv(p);
	notnull(pr);
	require(pf_is_empty(pr->pflags));
	/* Try one flag. */
	r  = parser_parse(p, "player-flags:KNOW_ZAPPER");
	eq(r, PARSE_ERROR_NONE);
	/* Try setting more than one flag at once. */
	r = parser_parse(p, "player-flags:SEE_ORE | KNOW_MUSHROOM");
	eq(r, PARSE_ERROR_NONE);
	pf_wipe(eflags);
	pf_on(eflags, PF_KNOW_ZAPPER);
	pf_on(eflags, PF_SEE_ORE);
	pf_on(eflags, PF_KNOW_MUSHROOM);
	pr = (struct player_race*) parser_priv(p);
	notnull(pr);
	require(pf_is_equal(pr->pflags, eflags));
	ok;
}

static int test_play_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized flag. */
	enum parser_error r = parser_parse(p, "player-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_values0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_race *pr = (struct player_race*) parser_priv(p);
	enum parser_error r;
	int i;

	notnull(pr);
	for (i = 0; i < ELEM_MAX; ++i) {
		pr->el_info[i].res_level = 0;
	}
	/* Try setting one value. */
	r = parser_parse(p, "values:RES_DARK[1]");
	eq(r, PARSE_ERROR_NONE);
	/* Try setting multiple values at once. */
	r = parser_parse(p, "values:RES_FIRE[1] | RES_COLD[-1]");
	eq(r, PARSE_ERROR_NONE);
	pr = (struct player_race*) parser_priv(p);
	notnull(pr);
	for (i = 0; i < ELEM_MAX; ++i) {
		if (i == ELEM_DARK || i == ELEM_FIRE) {
			eq(pr->el_info[i].res_level, 1);
		} else if (i == ELEM_COLD) {
			eq(pr->el_info[i].res_level, -1);
		} else {
			eq(pr->el_info[i].res_level, 0);
		}
	}
	ok;
}

static int test_values_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized value. */
	enum parser_error r = parser_parse(p, "values:XYZZY[2]");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	/* Try an unrecognized element. */
	r = parser_parse(p, "values:RES_XYZZY[3]");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

const char *suite_name = "parse/p-info";
/*
 * test_missing_header_record0() has to be before test_name0().  All others,
 * except test_name0(), have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
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
	{ "object_flags_bad0", test_obj_flags_bad0 },
	{ "player_flags0", test_play_flags0 },
	{ "player_flags_bad0", test_play_flags_bad0 },
	{ "values0", test_values0 },
	{ "values_bad0", test_values_bad0 },
	{ NULL, NULL }
};
