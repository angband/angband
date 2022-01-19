/*
 * effects/info
 * Test functions from effects-info.c for single effects; tests for chains are
 * in chain.c.
 */

#include "unit-test.h"
#include "test-utils.h"
#include "effects.h"
#include "effects-info.h"
#include "init.h"
#include "z-dice.h"

struct test_effects {
	struct effect *acid_bolt;
	struct effect *fire_arc;
	struct effect *cold_sphere;
	struct effect *lightning_ball;
	struct effect *drain_bolt;
	struct effect *curse_mon;
	struct effect *slow_bolt;
	struct effect *heal;
	struct effect *food;
	struct effect *cure_stun;
	struct effect *inc_fear;
	struct effect *inc_nores_blind;
	struct effect *dec_fast;
	struct effect *detect_gold;
	struct effect *set_value;
	int avgd_acid_bolt;
	int avgd_fire_arc;
	int avgd_cold_sphere;
	int avgd_lightning_ball;
	int avgd_drain_bolt;
	int avgd_curse_mon;
	int avg_set_value;
};

static struct effect *build_effect(int index, const char *st_str,
		const char *d_str, int radius, int other) {
	struct effect *e = mem_zalloc(sizeof(*e));

	e->index = index;
	if (d_str) {
		e->dice = dice_new();
		if (!dice_parse_string(e->dice, d_str)) {
			free_effect(e);
			return NULL;
		}
	}
	e->subtype = effect_subtype(e->index, st_str);
	if (e->subtype == -1) {
		free_effect(e);
		return NULL;
	}
	e->radius = radius;
	e->other = other;
	return e;
}

int teardown_tests(void *state) {
	struct test_effects *te = state;

	if (te) {
		free_effect(te->set_value);
		free_effect(te->detect_gold);
		free_effect(te->dec_fast);
		free_effect(te->inc_nores_blind);
		free_effect(te->inc_fear);
		free_effect(te->cure_stun);
		free_effect(te->food);
		free_effect(te->heal);
		free_effect(te->slow_bolt);
		free_effect(te->curse_mon);
		free_effect(te->drain_bolt);
		free_effect(te->lightning_ball);
		free_effect(te->cold_sphere);
		free_effect(te->fire_arc);
		free_effect(te->acid_bolt);
		mem_free(te);
	}
	cleanup_angband();
	return 0;
}

int setup_tests(void **state) {
	struct test_effects *te;
	bool failed;

	set_file_paths();
	init_angband();

	/* Set up some effects.  Remember expected average damage. */
	failed = false;
	te = mem_zalloc(sizeof(*te));
	te->acid_bolt = build_effect(EF_BOLT, "ACID", "2d8", 0, 0);
	te->avgd_acid_bolt = 9;
	if (!te->acid_bolt) failed = true;
	te->fire_arc = build_effect(EF_ARC, "FIRE", "4+1d5", 0, 0);
	te->avgd_fire_arc = 7;
	if (!te->fire_arc) failed = true;
	te->cold_sphere = build_effect(EF_SPHERE, "COLD", "2+3d1", 5, 0);
	te->avgd_cold_sphere = 5;
	if (!te->cold_sphere) failed = true;
	te->lightning_ball = build_effect(EF_BALL, "ELEC", "5+8d3", 3, 0);
	te->avgd_lightning_ball = 21;
	if (!te->lightning_ball) failed = true;
	te->drain_bolt = build_effect(EF_BOLT_STATUS_DAM, "MON_DRAIN",
		"10", 0, 0);
	te->avgd_drain_bolt = 10;
	if (!te->drain_bolt) failed = true;
	te->curse_mon = build_effect(EF_CURSE, "NONE", "6d4", 0, 0);
	te->avgd_curse_mon = 15;
	if (!te->curse_mon) failed = true;
	te->slow_bolt = build_effect(EF_BOLT_STATUS, "MON_SLOW", "15+1d5", 0, 0);
	if (!te->slow_bolt) failed = true;
	te->heal = build_effect(EF_HEAL_HP, "NONE", "13", 0, 0);
	if (!te->heal) failed = true;
	te->food = build_effect(EF_NOURISH, "INC_BY", "5", 0, 0);
	if (!te->food) failed = true;
	te->cure_stun = build_effect(EF_CURE, "STUN", NULL, 0, 0);
	if (!te->cure_stun) failed = true;
	te->inc_fear = build_effect(EF_TIMED_INC, "AFRAID", "30+1d10", 0, 0);
	if (!te->inc_fear) failed = true;
	te->inc_nores_blind = build_effect(EF_TIMED_INC_NO_RES, "BLIND",
		"40", 0, 0);
	if (!te->inc_nores_blind) failed = true;
	te->dec_fast = build_effect(EF_TIMED_DEC, "FAST", "15", 0, 0);
	if (!te->dec_fast) failed = true;
	te->detect_gold = build_effect(EF_DETECT_GOLD, "NONE", NULL, 0, 0);
	if (!te->detect_gold) failed = true;
	te->set_value = build_effect(EF_SET_VALUE, "NONE", "5+8d10", 0, 0);
	if (!te->set_value) failed = true;
	te->avg_set_value = 49;

	if (failed) {
		teardown_tests(te);
		return 1;
	}

	*state = te;
	return 0;
}

static int test_damages(void *state)
{
	struct test_effects *te = state;

	require(effect_damages(te->acid_bolt));
	require(effect_damages(te->fire_arc));
	require(effect_damages(te->cold_sphere));
	require(effect_damages(te->lightning_ball));
	require(effect_damages(te->drain_bolt));
	require(effect_damages(te->curse_mon));
	require(!effect_damages(te->slow_bolt));
	require(!effect_damages(te->heal));
	require(!effect_damages(te->food));
	require(!effect_damages(te->cure_stun));
	require(!effect_damages(te->inc_fear));
	require(!effect_damages(te->inc_nores_blind));
	require(!effect_damages(te->dec_fast));
	require(!effect_damages(te->detect_gold));
	require(!effect_damages(te->set_value));
	ok;
}

static int test_avg_damage(void *state) {
	struct test_effects *te = state;

	eq(effect_avg_damage(te->acid_bolt, NULL), te->avgd_acid_bolt);
	eq(effect_avg_damage(te->fire_arc, NULL), te->avgd_fire_arc);
	eq(effect_avg_damage(te->cold_sphere, NULL), te->avgd_cold_sphere);
	eq(effect_avg_damage(te->lightning_ball, NULL), te->avgd_lightning_ball);
	eq(effect_avg_damage(te->drain_bolt, NULL), te->avgd_drain_bolt);
	eq(effect_avg_damage(te->curse_mon, NULL), te->avgd_curse_mon);
	eq(effect_avg_damage(te->slow_bolt, NULL), 0);
	eq(effect_avg_damage(te->heal, NULL), 0);
	eq(effect_avg_damage(te->food, NULL), 0);
	eq(effect_avg_damage(te->cure_stun, NULL), 0);
	eq(effect_avg_damage(te->inc_fear, NULL), 0);
	eq(effect_avg_damage(te->inc_nores_blind, NULL), 0);
	eq(effect_avg_damage(te->dec_fast, NULL), 0);
	eq(effect_avg_damage(te->detect_gold, NULL), 0);
	eq(effect_avg_damage(te->lightning_ball, te->set_value->dice), te->avg_set_value);
	ok;
}

static int test_projection(void *state) {
	struct test_effects *te = state;

	require(streq(effect_projection(te->acid_bolt), "acid"));
	require(streq(effect_projection(te->fire_arc), "fire"));
	require(streq(effect_projection(te->cold_sphere), "frost"));
	require(streq(effect_projection(te->lightning_ball), "lightning"));
	require(streq(effect_projection(te->drain_bolt), ""));
	require(streq(effect_projection(te->curse_mon), ""));
	require(streq(effect_projection(te->slow_bolt), ""));
	require(streq(effect_projection(te->heal), ""));
	require(streq(effect_projection(te->food), ""));
	require(streq(effect_projection(te->cure_stun), ""));
	require(streq(effect_projection(te->inc_fear), ""));
	require(streq(effect_projection(te->inc_nores_blind), ""));
	require(streq(effect_projection(te->dec_fast), ""));
	require(streq(effect_projection(te->detect_gold), ""));
	ok;
}

static int test_menu_name(void *state) {
	struct test_effects *te = state;
	char buf[80];
	size_t n;

	n = effect_get_menu_name(buf, sizeof(buf), te->acid_bolt);
	eq(n, strlen(buf));
	require(streq(buf, "cast a bolt of acid"));
	n = effect_get_menu_name(buf, sizeof(buf), te->fire_arc);
	eq(n, strlen(buf));
	require(streq(buf, "produce a cone of fire"));
	n = effect_get_menu_name(buf, sizeof(buf), te->cold_sphere);
	eq(n, strlen(buf));
	require(streq(buf, "project frost"));
	n = effect_get_menu_name(buf, sizeof(buf), te->lightning_ball);
	eq(n, strlen(buf));
	require(streq(buf, "fire a ball of lightning"));
	n = effect_get_menu_name(buf, sizeof(buf), te->drain_bolt);
	eq(n, strlen(buf));
	require(streq(buf, "cast a bolt which damages living monsters"));
	n = effect_get_menu_name(buf, sizeof(buf), te->curse_mon);
	eq(n, strlen(buf));
	require(streq(buf, "curse"));
	n = effect_get_menu_name(buf, sizeof(buf), te->slow_bolt);
	eq(n, strlen(buf));
	require(streq(buf, "cast a bolt which attempts to slow monsters"));
	n = effect_get_menu_name(buf, sizeof(buf), te->heal);
	eq(n, strlen(buf));
	require(streq(buf, "heal self"));
	n = effect_get_menu_name(buf, sizeof(buf), te->food);
	eq(n, strlen(buf));
	require(streq(buf, "feed yourself"));
	n = effect_get_menu_name(buf, sizeof(buf), te->cure_stun);
	eq(n, strlen(buf));
	require(streq(buf, "cure stunning"));
	n = effect_get_menu_name(buf, sizeof(buf), te->inc_fear);
	eq(n, strlen(buf));
	require(streq(buf, "extend fear"));
	n = effect_get_menu_name(buf, sizeof(buf), te->inc_nores_blind);
	eq(n, strlen(buf));
	require(streq(buf, "extend blindness"));
	n = effect_get_menu_name(buf, sizeof(buf), te->dec_fast);
	eq(n, strlen(buf));
	require(streq(buf, "reduce haste"));
	n = effect_get_menu_name(buf, sizeof(buf), te->detect_gold);
	eq(n, strlen(buf));
	require(streq(buf, "detect gold"));
	ok;
}

const char *suite_name = "effects/info";
struct test tests[] = {
	{ "damages", test_damages },
	{ "average damage", test_avg_damage },
	{ "projection", test_projection },
	{ "menu name", test_menu_name },
	{ NULL, NULL }
};
