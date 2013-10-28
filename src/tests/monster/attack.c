/* monster/attack */

#include "unit-test.h"
#include "unit-test-data.h"

#include "monster/monster.h"

int setup_tests(void **state) {
	struct monster_race *r = &test_r_human;
	struct monster *m = mem_zalloc(sizeof *m);
	m->race = r;
	m->r_idx = r->ridx;
	r_info = r;
	*state = m;

	p_ptr = NULL;
	rand_fix(100);
	return 0;
}

NOTEARDOWN

static int mdam(struct monster *m)
{
	return m->race->blow[0].d_dice;
}

static int take1(struct player *p, struct monster *m, int blow, int eff)
{
	int old, new;
	m->race->blow[0].effect = eff;
	m->race->blow[0].method = blow;
	p->chp = p->mhp;
	old = p->chp;
	testfn_make_attack_normal(m, p);
	new = p->chp;
	p->chp = p->mhp;
	return old - new;
}

static int test_blows(void *state) {
	struct monster *m = state;
	struct player *p = &test_player;
	int delta;

	flags_set(m->race->flags, RF_SIZE, RF_NEVER_BLOW, FLAG_END);
	delta = take1(p, m, RBM_HIT, RBE_HURT);
	flags_clear(m->race->flags, RF_SIZE, RF_NEVER_BLOW, FLAG_END);
	eq(delta, 0);

	delta = take1(p, m, RBM_HIT, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_TOUCH, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_PUNCH, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_KICK, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_CLAW, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_BITE, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_STING, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_BUTT, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_CRUSH, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_ENGULF, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_CRAWL, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_DROOL, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_SPIT, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_GAZE, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_WAIL, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_SPORE, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_BEG, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_INSULT, RBE_HURT);
	eq(delta, mdam(m));
	delta = take1(p, m, RBM_MOAN, RBE_HURT);
	eq(delta, mdam(m));

	ok;
}

static int test_effects(void *state) {
	struct monster *m = state;
	struct player *p = &test_player;
	int delta;

	require(!p->timed[TMD_POISONED]);
	delta = take1(p, m, RBM_HIT, RBE_POISON);
	require(p->timed[TMD_POISONED]);

	delta = take1(p, m, RBM_HIT, RBE_ACID);
	require(delta > 0);
	delta = take1(p, m, RBM_HIT, RBE_ELEC);
	require(delta > 0);
	delta = take1(p, m, RBM_HIT, RBE_FIRE);
	require(delta > 0);
	delta = take1(p, m, RBM_HIT, RBE_COLD);
	require(delta > 0);

	require(!p->timed[TMD_BLIND]);
	delta = take1(p, m, RBM_HIT, RBE_BLIND);
	require(p->timed[TMD_BLIND]);

	ok;
}

const char *suite_name = "monster/attack";
const struct test tests[] = {
	{ "blows", test_blows },
	{ "effects", test_effects },
	{ NULL, NULL },
};
