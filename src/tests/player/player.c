/* player/player */

#include "unit-test.h"
#include "unit-test-data.h"

#include "player/player.h"

static int setup(void **state) {
	struct player *p = mem_zalloc(sizeof *p);
	player_init(p);
	*state = p;
	return 0;
}

static int teardown(void *state) {
	mem_free(state);
	return 0;
}

static int test_stat_inc(void *state) {
	struct player *p = state;
	int v;

	p->stat_cur[A_STR] = 18 + 101;
	v = player_stat_inc(p, A_STR);
	require(!v);
	p->stat_cur[A_STR] = 15;
	player_stat_inc(p, A_STR);
	eq(p->stat_cur[A_STR], 16);
	player_stat_inc(p, A_STR);
	eq(p->stat_cur[A_STR], 17);
	player_stat_inc(p, A_STR);
	eq(p->stat_cur[A_STR], 18);
	player_stat_inc(p, A_STR);
	require(p->stat_cur[A_STR] > 18);
	ok;
}

static int test_stat_dec(void *state) {
	struct player *p = state;
	int v;

	p->stat_cur[A_STR] = 3;
	p->stat_max[A_STR] = 3;
	v = player_stat_dec(p, A_STR, TRUE);
	require(!v);
	p->stat_cur[A_STR] = 15;
	p->stat_max[A_STR] = 15;
	player_stat_dec(p, A_STR, FALSE);
	eq(p->stat_cur[A_STR], 14);
	eq(p->stat_max[A_STR], 15);
	player_stat_dec(p, A_STR, TRUE);
	eq(p->stat_cur[A_STR], 13);
	eq(p->stat_max[A_STR], 14);
	ok;
}

static const char *suite_name = "player/player";
static struct test tests[] = {
	{ "stat-inc", test_stat_inc },
	{ "stat-dec", test_stat_dec },
	{ NULL, NULL }
};
