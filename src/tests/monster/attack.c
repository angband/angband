/* monster/attack */

#include "unit-test.h"
#include "unit-test-data.h"

#include "mon-attack.h"
#include "mon-lore.h"
#include "monster.h"
#include "option.h"
#include "player-timed.h"
#include "ui-input.h"

int setup_tests(void **state) {
	struct monster_race *r = &test_r_human;
	struct monster *m = mem_zalloc(sizeof *m);
	textui_input_init();
	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->mon_blows_max = 2;
	projections = test_projections;
	m->race = r;
	r_info = r;
	*state = m;

	rand_fix(100);
	return 0;
}

int teardown_tests(void *state) {
	struct monster *m = state;
	mem_free(m);
	mem_free(z_info);
	return 0;
}

static int mdam(struct monster *m)
{
	return m->race->blow[0].dice.dice;
}

static int take1(struct player *p, struct monster *m, struct blow_method *blow,
				 struct blow_effect *eff)
{
	int old, new;
	cave = &test_cave;
	m->race->blow[0].effect = eff;
	m->race->blow[0].method = blow;
	p->chp = p->mhp;
	old = p->chp;
	make_attack_normal(m, p);
	new = p->chp;
	p->chp = p->mhp;
	return old - new;
}

static int test_blows(void *state) {
	struct monster *m = state;
	struct player *p = &test_player;
	int delta;

	p->upkeep = &test_player_upkeep;

	rf_on(m->race->flags, RF_NEVER_BLOW);
	delta = take1(p, m, &test_blow_method, &test_blow_effect_hurt);
	rf_off(m->race->flags, RF_NEVER_BLOW);
	eq(delta, 0);

	delta = take1(p, m, &test_blow_method, &test_blow_effect_hurt);
	eq(delta, mdam(m));

	ok;
}

static int test_effects(void *state) {
	struct monster *m = state;
	struct player *p = &test_player;
	int delta;

	options_init_defaults(&p->opts);
	p->upkeep = &test_player_upkeep;

	//require(!p->timed[TMD_POISONED]);
	//delta = take1(p, m, &test_blow_method, &test_blow_effect_poison);
	//require(p->timed[TMD_POISONED]);

	delta = take1(p, m, &test_blow_method, &test_blow_effect_acid);
	require(delta > 0);
	delta = take1(p, m, &test_blow_method, &test_blow_effect_elec);
	require(delta > 0);
	delta = take1(p, m, &test_blow_method, &test_blow_effect_fire);
	require(delta > 0);
	delta = take1(p, m, &test_blow_method, &test_blow_effect_cold);
	require(delta > 0);

	//require(!p->timed[TMD_BLIND]);
	//delta = take1(p, m, &test_blow_method, &test_blow_effect_blind);
	//require(p->timed[TMD_BLIND]);

	ok;
}

const char *suite_name = "monster/attack";
struct test tests[] = {
	{ "blows", test_blows },
	{ "effects", test_effects },
	{ NULL, NULL },
};
