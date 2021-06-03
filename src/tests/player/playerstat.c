/* player/playerstat */

#include "unit-test.h"
#include "unit-test-data.h"

#include "player-birth.h"
#include "player.h"

int setup_tests(void **state) {
	struct player *p = mem_zalloc(sizeof *p);
	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->pack_size = 23;
	z_info->quiver_size = 10;
	player_init(p);
	*state = p;
	return 0;
}

int teardown_tests(void *state) {
	struct player *p = state;
	mem_free(z_info);
	mem_free(p->upkeep->inven);
	mem_free(p->upkeep->quiver);
	mem_free(p->upkeep);
	mem_free(p->timed);
	mem_free(p->obj_k->brands);
	mem_free(p->obj_k->slays);
	mem_free(p->obj_k->curses);
	mem_free(p->obj_k);
	mem_free(state);
	return 0;
}

static int test_stat_inc(void *state) {
	struct player *p = state;
	int v;

	p->stat_cur[STAT_STR] = 18 + 101;
	v = player_stat_inc(p, STAT_STR);
	require(!v);
	p->stat_cur[STAT_STR] = 15;
	player_stat_inc(p, STAT_STR);
	eq(p->stat_cur[STAT_STR], 16);
	player_stat_inc(p, STAT_STR);
	eq(p->stat_cur[STAT_STR], 17);
	player_stat_inc(p, STAT_STR);
	eq(p->stat_cur[STAT_STR], 18);
	player_stat_inc(p, STAT_STR);
	require(p->stat_cur[STAT_STR] > 18);
	ok;
}

static int test_stat_dec(void *state) {
	struct player *p = state;
	int v;

	p->stat_cur[STAT_STR] = 3;
	p->stat_max[STAT_STR] = 3;
	v = player_stat_dec(p, STAT_STR, true);
	require(!v);
	p->stat_cur[STAT_STR] = 15;
	p->stat_max[STAT_STR] = 15;
	player_stat_dec(p, STAT_STR, false);
	eq(p->stat_cur[STAT_STR], 14);
	eq(p->stat_max[STAT_STR], 15);
	player_stat_dec(p, STAT_STR, true);
	eq(p->stat_cur[STAT_STR], 13);
	eq(p->stat_max[STAT_STR], 14);
        p->stat_cur[STAT_STR] = 18+13;
	p->stat_max[STAT_STR] = 18+13;
	player_stat_dec(p, STAT_STR, false);
	eq(p->stat_cur[STAT_STR], 18+03);
	eq(p->stat_max[STAT_STR], 18+13);
	p->stat_max[STAT_STR] = 18+03;
	player_stat_dec(p, STAT_STR, true);
	eq(p->stat_cur[STAT_STR], 18);
	eq(p->stat_max[STAT_STR], 18);
	ok;
}

const char *suite_name = "player/player";
struct test tests[] = {
	{ "stat-inc", test_stat_inc },
	{ "stat-dec", test_stat_dec },
	{ NULL, NULL }
};
