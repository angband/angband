/* player/pscore.c */
/* Exercise functions in score.c and score-util.c */

#include "unit-test.h"
#include "unit-test-data.h"
#include "game-world.h"
#include "player.h"
#include "score.h"
#include "z-file.h"
#include "z-rand.h"
#include "z-util.h"
#include <limits.h>
#include <string.h>
#include <time.h>


static struct player my_p;

int setup_tests(void **state) {
	/* Set up enough of a struct player to work with build_score(). */
	my_p.max_exp = 1234;
	my_p.au = 567;
	my_p.lev = 4;
	my_p.depth = 3;
	my_p.max_lev = 6;
	my_p.max_depth = 5;
	my_strcpy(my_p.died_from, "a grue", sizeof(my_p.died_from));
	my_p.race = &test_race;
	my_p.class = &test_class;

	/* build_score() also uses these globals. */
	turn = 890;
	player_uid = 10;

	/*
	 * One test randomly uses random indices when corrupting or reordering
	 * score records.
	 */
	Rand_init();

	return 0;
}

NOTEARDOWN

/*
 * Test highscore_valid() with score records that should be valid.
 */
static int test_highscore_valid0(void *state) {
	struct high_score score;
	time_t now;

	/* An empty score record should be valid. */
	(void)memset(&score, 0, sizeof(score));
	require(highscore_valid(&score));

	/* The result of build_score() should be valid. */
	build_score(&score, &my_p, my_p.died_from, NULL);
	require(highscore_valid(&score));
	build_score(&score, &my_p, "nobody (yet)", NULL);
	require(highscore_valid(&score));
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	require(highscore_valid(&score));

	ok;
}

/*
 * Test highscore_valid() with score records that should be invalid.
 */
static int test_highscore_valid1(void *state) {
	struct high_score score;


	/* An empty score which has a non-empty string should be invalid. */
	(void)memset(&score, 0, sizeof(score));
	score.pts[0] = '1';
	require(!highscore_valid(&score));
	score.pts[0] = '\0';
	score.gold[0] = '2';
	require(!highscore_valid(&score));
	score.gold[0] = '\0';
	score.turns[0] = '3';
	require(!highscore_valid(&score));
	score.turns[0] = '\0';
	score.day[0] = 'a';
	require(!highscore_valid(&score));
	score.day[0] = '\0';
	score.who[0] = 'b';
	require(!highscore_valid(&score));
	score.who[0] = '\0';
	score.uid[0] = '4';
	require(!highscore_valid(&score));
	score.uid[0] = '\0';
	score.p_r[0] = '5';
	require(!highscore_valid(&score));
	score.p_r[0] = '\0';
	score.p_c[0] = '6';
	require(!highscore_valid(&score));
	score.p_c[0] = '\0';
	score.cur_lev[0] = '7';
	require(!highscore_valid(&score));
	score.cur_lev[0] = '\0';
	score.cur_dun[0] = '8';
	require(!highscore_valid(&score));
	score.cur_dun[0] = '\0';
	score.max_lev[0] = '9';
	require(!highscore_valid(&score));
	score.max_lev[0] = '\0';
	score.max_dun[0] = '0';
	require(!highscore_valid(&score));
	score.max_dun[0] = '\0';
	score.how[0] = 'c';

	/*
	 * A non-empty score with a string that is not null-terminated should be
	 * invalid.
	 */
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.what, ' ', sizeof(score.what));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.pts, ' ', sizeof(score.pts));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.gold, ' ', sizeof(score.gold));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.turns, ' ', sizeof(score.turns));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.day, ' ', sizeof(score.day));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.who, ' ', sizeof(score.who));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.uid, ' ', sizeof(score.uid));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.p_r, ' ', sizeof(score.p_r));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.p_c, ' ', sizeof(score.p_c));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.cur_lev, ' ', sizeof(score.cur_lev));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.cur_dun, ' ', sizeof(score.cur_dun));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.max_lev, ' ', sizeof(score.max_lev));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.max_dun, ' ', sizeof(score.max_dun));
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)memset(score.how, ' ', sizeof(score.how));
	require(!highscore_valid(&score));

	/*
	 * A non-empty score that has a number field which is not parseable
	 * should be invalid.
	 */
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.pts[0] = '\0';
	require(!highscore_valid(&score));
	score.pts[0] = 'a';
	require(!highscore_valid(&score));
	score.pts[0] = '1';
	score.pts[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.gold[0] = '\0';
	require(!highscore_valid(&score));
	score.gold[0] = 'a';
	require(!highscore_valid(&score));
	score.gold[0] = '1';
	score.gold[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.turns[0] = '\0';
	require(!highscore_valid(&score));
	score.turns[0] = 'a';
	require(!highscore_valid(&score));
	score.turns[0] = '1';
	score.turns[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.uid[0] = '\0';
	require(!highscore_valid(&score));
	score.uid[0] = 'a';
	require(!highscore_valid(&score));
	score.uid[0] = '1';
	score.uid[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.p_r[0] = '\0';
	require(!highscore_valid(&score));
	score.p_r[0] = 'a';
	require(!highscore_valid(&score));
	score.p_r[0] = '1';
	score.p_r[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.p_c[0] = '\0';
	require(!highscore_valid(&score));
	score.p_c[0] = 'a';
	require(!highscore_valid(&score));
	score.p_c[0] = '1';
	score.p_c[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.cur_lev[0] = '\0';
	require(!highscore_valid(&score));
	score.cur_lev[0] = 'a';
	require(!highscore_valid(&score));
	score.cur_lev[0] = '1';
	score.cur_lev[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.cur_dun[0] = '\0';
	require(!highscore_valid(&score));
	score.cur_dun[0] = 'a';
	require(!highscore_valid(&score));
	score.cur_dun[0] = '1';
	score.cur_dun[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.max_lev[0] = '\0';
	require(!highscore_valid(&score));
	score.max_lev[0] = 'a';
	require(!highscore_valid(&score));
	score.max_lev[0] = '1';
	score.max_lev[1] = 'a';
	require(!highscore_valid(&score));
	build_score(&score, &my_p, my_p.died_from, NULL);
	score.max_dun[0] = '\0';
	require(!highscore_valid(&score));
	score.max_dun[0] = 'a';
	require(!highscore_valid(&score));
	score.max_dun[0] = '1';
	score.max_dun[1] = 'a';
	require(!highscore_valid(&score));

	ok;
}

static int test_highscore_where0(void *state) {
	struct high_score scores[5], score;
	int32_t old_exp;
	int16_t old_depth;
	time_t now;

	(void)memset(scores, 0, sizeof(scores));

	/*
	 * If all scores are empty, a new non-empty score should go in the
	 * first slot.
	 */
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	eq(highscore_where(&score, scores, N_ELEMENTS(scores)), 0);
	(void)memcpy(scores, &score, sizeof(score));

	/*
	 * A score with fewer points and the same winning status goes after a
	 * another score with more points.
	 */
	old_exp = my_p.max_exp--;
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	my_p.max_exp = old_exp;
	eq(highscore_where(&score, scores, N_ELEMENTS(scores)), 1);
	(void)memcpy(scores + 1, &score, sizeof(score));

	/* A winning score goes before one that did not win. */
	now = time(NULL);
	build_score(&score, &my_p, WINNING_HOW, &now);
	eq(highscore_where(&score, scores, N_ELEMENTS(scores)), 0);
	(void)memmove(scores + 1, scores, 2 * sizeof(score));
	(void)memcpy(scores, &score, sizeof(score));

	/*
	 * A score with more points and the same winning status goes before a
	 * another score with less points.
	 */
	old_exp = my_p.max_exp++;
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	my_p.max_exp = old_exp;
	eq(highscore_where(&score, scores, N_ELEMENTS(scores)), 1);
	(void)memmove(scores + 2, scores + 1, 2 * sizeof(score));
	(void)memcpy(scores + 1, &score, sizeof(score));

	/*
	 * If the same winning status and same points, a newly added entry
	 * goes first.
	 */
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	eq(highscore_where(&score, scores, N_ELEMENTS(scores)), 2);
	(void)memmove(scores + 3, scores + 2, 2 * sizeof(score));
	(void)memcpy(scores + 2, &score, sizeof(score));

	/*
	 * When the scores array is full with non-empty entries, a new score
	 * that would not otherwise be included goes in the last slot.
	 */
	old_depth = my_p.max_depth;
	my_p.max_depth -= 2;
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	my_p.max_depth = old_depth;
	eq(highscore_where(&score, scores, N_ELEMENTS(scores)), 4);

	ok;
}

static int test_highscore_add0(void *state) {
	struct high_score scurr[5], sadded[6];
	int32_t old_exp;
	int16_t old_depth;
	time_t now;

	(void)memset(scurr, 0, sizeof(scurr));

	/*
	 * If all scores are empty, a new non-empty score should go in the
	 * first slot.
	 */
	now = time(NULL);
	build_score(&sadded[0], &my_p, WINNING_HOW, &now);
	eq(highscore_add(&sadded[0], scurr, N_ELEMENTS(scurr)), 0);
	eq(memcmp(&scurr[0], &sadded[0], sizeof(scurr[0])), 0);

	/*
	 * A score with more points and the same winning status goes before a
	 * another score with less points.
	 */
	old_exp = my_p.max_exp++;
	now = time(NULL);
	build_score(&sadded[1], &my_p, WINNING_HOW, &now);
	my_p.max_exp = old_exp;
	eq(highscore_add(&sadded[1], scurr, N_ELEMENTS(scurr)), 0);
	eq(memcmp(&scurr[0], &sadded[1], sizeof(scurr[0])), 0);
	eq(memcmp(&scurr[1], &sadded[0], sizeof(scurr[1])), 0);

	/* A score that did not win goes after any that did. */
	now = time(NULL);
	build_score(&sadded[2], &my_p, my_p.died_from, &now);
	eq(highscore_add(&sadded[2], scurr, N_ELEMENTS(scurr)), 2);
	eq(memcmp(&scurr[0], &sadded[1], sizeof(scurr[0])), 0);
	eq(memcmp(&scurr[1], &sadded[0], sizeof(scurr[0])), 0);
	eq(memcmp(&scurr[2], &sadded[2], sizeof(scurr[2])), 0);

	/*
	 * A score with less points and the same winning status goes after a
	 * another score with more points.
	 */
	old_exp = my_p.max_exp--;
	now = time(NULL);
	build_score(&sadded[3], &my_p, WINNING_HOW, &now);
	my_p.max_exp = old_exp;
	eq(highscore_add(&sadded[3], scurr, N_ELEMENTS(scurr)), 2);
	eq(memcmp(&scurr[0], &sadded[1], sizeof(scurr[0])), 0);
	eq(memcmp(&scurr[1], &sadded[0], sizeof(scurr[1])), 0);
	eq(memcmp(&scurr[2], &sadded[3], sizeof(scurr[2])), 0);
	eq(memcmp(&scurr[3], &sadded[2], sizeof(scurr[3])), 0);

	/*
	 * If the same winning status and same points, a newly added entry
	 * goes first.
	 */
	now = time(NULL);
	build_score(&sadded[4], &my_p, WINNING_HOW, &now);
	eq(highscore_add(&sadded[4], scurr, N_ELEMENTS(scurr)), 1);
	eq(memcmp(&scurr[0], &sadded[1], sizeof(scurr[0])), 0);
	eq(memcmp(&scurr[1], &sadded[4], sizeof(scurr[1])), 0);
	eq(memcmp(&scurr[2], &sadded[0], sizeof(scurr[2])), 0);
	eq(memcmp(&scurr[3], &sadded[3], sizeof(scurr[3])), 0);
	eq(memcmp(&scurr[4], &sadded[2], sizeof(scurr[4])), 0);

	/*
	 * When the scores array is full with non-empty entries, a new score
	 * that would not otherwise be included goes in the last slot.
	 */
	old_depth = my_p.max_depth;
	my_p.max_depth -= 2;
	now = time(NULL);
	build_score(&sadded[5], &my_p, my_p.died_from, &now);
	my_p.max_depth = old_depth;
	eq(highscore_add(&sadded[5], scurr, N_ELEMENTS(scurr)), 4);
	eq(memcmp(&scurr[0], &sadded[1], sizeof(scurr[0])), 0);
	eq(memcmp(&scurr[1], &sadded[4], sizeof(scurr[1])), 0);
	eq(memcmp(&scurr[2], &sadded[0], sizeof(scurr[2])), 0);
	eq(memcmp(&scurr[3], &sadded[3], sizeof(scurr[3])), 0);
	eq(memcmp(&scurr[4], &sadded[5], sizeof(scurr[4])), 0);

	ok;
}

/*
 * Check cases where highscore_regularize() should not change the array of
 * scores.
 */
static int test_highscore_regularize0(void *state)
{
	struct high_score scores[6], scores_good[6], score;
	int32_t old_exp;
	int16_t old_depth;
	time_t now;

	/*
	 * A scores array filled with empty entries should not trigger any
	 * changes.
	 */
	(void)memset(scores_good, 0, sizeof(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	/*
	 * Arrays with one or more non-empty elements (contents of each
	 * element constructed by build_score() and then added with
	 * highscore_add()) with zero or more empty elements at the end
	 * should not trigger any changes.
	 */
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	old_exp = my_p.max_exp;
	my_p.max_exp += 10;
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	my_p.max_exp -= 5;
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	my_p.max_exp = old_exp;
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	now = time(NULL);
	build_score(&score, &my_p, WINNING_HOW, &now);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	my_p.max_exp += 20;
	now = time(NULL);
	build_score(&score, &my_p, WINNING_HOW, &now);
	my_p.max_exp = old_exp;
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	now = time(NULL);
	build_score(&score, &my_p, my_p.died_from, &now);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	old_depth = my_p.max_depth;
	my_p.max_depth -= 3;
	build_score(&score, &my_p, my_p.died_from, &now);
	my_p.max_depth = old_depth;
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	require(!highscore_regularize(scores, N_ELEMENTS(scores)));

	ok;
}

/*
 * Check cases where highscore_regularize() should change the array of scores.
 */
static int test_highscore_regularize1(void *state)
{
	struct high_score scores[6], scores_good[6], score;
	int32_t old_exp;
	int i;

	/*
	 * An otherwise empty score with some field that is not empty should
	 * be converted to an empty one.
	 */
	(void)memset(scores_good, 0, sizeof(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	for (i = 0; i < (int)N_ELEMENTS(scores); ++i) {
		scores[i].pts[0] = '3';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].gold[0] = '1';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].turns[0] = '2';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		(void)memset(scores[i].day, ' ', sizeof(scores[i].day));
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].who[0] = 'a';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].uid[0] = '7';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].p_r[0] = '8';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].p_c[0] = '6';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].cur_lev[0] = '5';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].cur_dun[0] = '4';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].max_lev[0] = '9';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].max_dun[0] = '0';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
		scores[i].how[0] = 'c';
		require(highscore_regularize(scores, N_ELEMENTS(scores)));
		eq(memcmp(scores, scores_good, sizeof(scores)), 0);
	}

	/*
	 * Try corruption of entries or reordering of them when there is
	 * at least one non-empty entry.
	 */
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	(void)memset(scores[rand_range(1, (int)(N_ELEMENTS(scores) - 1))].p_r,
		' ', sizeof(scores[0].p_r));
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	scores[0].max_dun[0] = 'a';
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores + 1, scores_good + 1, sizeof(scores)
		- sizeof(scores[0])), 0);
	eq(memcmp(scores, scores + 1, sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	score = scores[0];
	i = rand_range(1, (int)(N_ELEMENTS(scores) - 1));
	scores[0] = scores[i];
	scores[i] = score;
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	build_score(&score, &my_p, WINNING_HOW, NULL);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	scores[rand_range(2, (int)(N_ELEMENTS(scores) - 1))].who[0] = 'a';
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	(void)memset(scores[0].turns, ' ', sizeof(scores[0].turns));
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores + 2, scores_good + 2, sizeof(scores)
		- 2 * sizeof(scores[0])), 0);
	eq(memcmp(scores, scores_good + 1, sizeof(scores[0])), 0);
	eq(memcmp(scores + 1, scores + 2, sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	scores[1].gold[0] = '+';
	scores[1].gold[1] = 'a';
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores[0])), 0);
	eq(memcmp(scores + 1, scores_good + 2, sizeof(scores[0])), 0);
	eq(memcmp(scores + 2, scores_good + 2, sizeof(scores)
		- 2 * sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	(void)memset(scores[0].cur_dun, ' ', sizeof(scores[0].cur_dun));
	scores[1].max_lev[0] = '\0';
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good + 2, sizeof(scores[0])), 0);
	eq(memcmp(scores + 1, scores_good + 2, sizeof(scores[0])), 0);
	eq(memcmp(scores + 2, scores_good + 2, sizeof(scores)
		- 2 * sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	score = scores[0];
	i = rand_range(2, (int)(N_ELEMENTS(scores) - 1));
	scores[0] = scores[i];
	scores[i] = score;
	score = scores[1];
	i = rand_range(2, (int)(N_ELEMENTS(scores) - 1));
	scores[1] = scores[i];
	scores[i] = score;
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	old_exp = my_p.max_exp;
	my_p.max_exp -= 22;
	build_score(&score, &my_p, my_p.died_from, NULL);
	my_p.max_exp = old_exp;
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	(void)memcpy(scores, scores_good, sizeof(scores));
	(void)memset(scores[rand_range(3, (int)(N_ELEMENTS(scores) - 1))].how,
		' ', sizeof(scores[0].how));
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	scores[0].max_dun[0] = 'c';
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good + 1, sizeof(scores)
		- sizeof(scores[0])), 0);
	eq(memcmp(scores + (N_ELEMENTS(scores) - 1), scores_good + 3,
		sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	scores[1].cur_lev[0] = '\0';
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores[0])), 0);
	eq(memcmp(scores + 1, scores_good + 2, sizeof(scores)
		- 2 * sizeof(scores[0])), 0);
	eq(memcmp(scores + (N_ELEMENTS(scores) - 1), scores_good + 3,
		sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	(void)memset(scores[2].day, ' ', sizeof(scores[2].day));
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, 2 * sizeof(scores[0])), 0);
	eq(memcmp(scores + 2, scores_good + 3, sizeof(scores[0])), 0);
	eq(memcmp(scores + 3, scores_good + 3, sizeof(scores)
		- 3 * sizeof(scores[0])), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	score = scores[0];
	i = rand_range(3, (int)(N_ELEMENTS(scores) - 1));
	scores[0] = scores[i];
	scores[i] = score;
	score = scores[1];
	i = rand_range(3, (int)(N_ELEMENTS(scores) - 1));
	scores[1] = scores[i];
	scores[i] = score;
	score = scores[2];
	i = rand_range(3, (int)(N_ELEMENTS(scores) - 1));
	scores[2] = scores[i];
	scores[i] = score;
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	old_exp = my_p.max_exp;
	my_p.max_exp += 30;
	build_score(&score, &my_p, WINNING_HOW, NULL);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	build_score(&score, &my_p, my_p.died_from, NULL);
	(void)highscore_add(&score, scores_good, N_ELEMENTS(scores_good));
	my_p.max_exp = old_exp;
	(void)memcpy(scores, scores_good, sizeof(scores));
	i = rand_range(0, (int)(N_ELEMENTS(scores) - 3));
	scores[i].p_r[0] = '\0';
	(void)memset(scores[i + 1].turns, ' ', sizeof(scores[i + 1].turns));
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	if (i > 0) {
		eq(memcmp(scores, scores_good, i * sizeof(scores[0])), 0);
	}
	eq(memcmp(scores + i, scores_good + i + 2,
		(N_ELEMENTS(scores) - (i + 2)) * sizeof(scores[0])), 0);
	(void)memset(&score, 0, sizeof(score));
	eq(memcmp(scores + (N_ELEMENTS(scores) - 2), &score, sizeof(score)), 0);
	eq(memcmp(scores + (N_ELEMENTS(scores) - 1), &score, sizeof(score)), 0);

	(void)memcpy(scores, scores_good, sizeof(scores));
	score = scores[0];
	scores[0] = scores[5];
	scores[5] = score;
	score = scores[1];
	scores[1] = scores[4];
	scores[4] = score;
	score = scores[2];
	scores[2] = scores[3];
	scores[3] = score;
	require(highscore_regularize(scores, N_ELEMENTS(scores)));
	eq(memcmp(scores, scores_good, sizeof(scores)), 0);

	ok;
}

const char *suite_name = "player/pscore";

struct test tests[] = {
	{ "highscore_valid0", test_highscore_valid0 },
	{ "highscore_valid1", test_highscore_valid1 },
	{ "highscore_where0", test_highscore_where0 },
	{ "highscore_add0", test_highscore_add0 },
	{ "highscore_regularize0", test_highscore_regularize0 },
	{ "highscore_regularize1", test_highscore_regularize1 },
	{ NULL, NULL }
};
