/*
 * effects/chain
 * Test handling of effect chains and the container (RANDOM and SELECT) or
 * logic operation effects (BREAK, SKIP, BREAK_IF, and SKIP_IF) that are used
 * in chains.
 */

#include "unit-test.h"
#include "test-utils.h"
#include "cave.h"
#include "effects.h"
#include "effects-info.h"
#include "game-world.h"
#include "init.h"
#include "player.h"
#include "player-birth.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "source.h"
#include "z-dice.h"

struct simple_effect {
	int t_index, radius, other;
	const char *st_str;
	const char *d_str;
};

int setup_tests(void **state) {
	set_file_paths();
	init_angband();
	/* Set up the player so there's a target available for the effects. */
	if (!player_make_simple(NULL, NULL, "Tester")) {
		cleanup_angband();
		return 1;
	}
	prepare_next_level(&cave, player);
	on_new_level();
	return 0;
}

int teardown_tests(void *state) {
	cleanup_angband();
	return 0;
}

static void restore_to_full_health(void)
{
	player->chp = player->mhp;
	if (player->upkeep) player->upkeep->redraw |= (PR_HP);
}

static struct effect *build_effect_chain(const struct simple_effect *earr,
	int count)
{
	struct effect *prev = NULL;
	int i = count;

	/* Work backwards to make building the linked list easier. */
	while (i > 0) {
		struct effect *curr = mem_zalloc(sizeof(*curr));

		--i;
		curr->next = prev;
		curr->index = earr[i].t_index;
		if (earr[i].d_str) {
			curr->dice = dice_new();
			if (!dice_parse_string(curr->dice, earr[i].d_str)) {
				free_effect(curr);
				return NULL;
			}
		}
		curr->subtype = effect_subtype(curr->index, earr[i].st_str);
		if (curr->subtype == -1) {
			free_effect(curr);
			return NULL;
		}
		curr->radius = earr[i].radius;
		curr->other = earr[i].other;
		prev = curr;
	}
	return prev;
}

static int test_chain1_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require(player->chp == player->mhp - 1);
	ok;
}

static int test_chain2_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_DAMAGE, 0, 0, "NONE", "2" },
		{ EF_HEAL_HP, 0, 0, "NONE", "1" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require(player->chp == player->mhp - 1);
	ok;
}

static int test_chain3_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_DAMAGE, 0, 0, "NONE", "5" },
		{ EF_HEAL_HP, 0, 0, "NONE", "4" },
		{ EF_DAMAGE, 0, 0, "NONE", "2" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require(player->chp == player->mhp - 3);
	ok;
}

/*
 * A RANDOM effect with a negative number of subeffects should do nothing
 * successfully.
 */
static int test_randomneg_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "-4+1d2" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	ok;
}

/* A RANDOM effect with zero subeffects should do nothing successfully.  */
static int test_random0_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "0" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	ok;
}

static int test_random1_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "1" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require(player->chp == player->mhp - 1);
	ok;
}

static int test_random2_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "2" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		player_clear_timed(player, TMD_BOLD, false);
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require((player->chp == player->mhp - 1 || player->timed[TMD_BOLD]) &&
		!(player->chp == player->mhp - 1 && player->timed[TMD_BOLD]));
	ok;
}

/*
 * Check that a RANDOM effect which says it has more subeffects than it does
 * is handled gracefully.
 */
static int test_randomover_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "10" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	ok;
}

/*
 * There's a non-deterministic aspect to this test.  It could fail (about .01%
 * of the time) even if random selection is working correctly.
 */
static int test_random_stats(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "2" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_DAMAGE, 0, 0, "NONE", "2" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	int nsim = 1000, i = 0;
	int bins[2] = { 0, 0 };

	if (ec) {
		while (1) {
			bool completed = false, ident = true;

			if (i >= nsim) break;
			restore_to_full_health();
			player_clear_timed(player, TMD_BOLD, false);
			completed = effect_do(ec, source_player(), NULL,
				&ident, true, 0, 0, false, NULL);
			if (!completed) break;
			if (player->mhp - player->chp == 1) {
				++bins[0];
			} else if (player->mhp - player->chp == 2) {
				++bins[1];
			} else {
				break;
			}
			++i;
		}
		free_effect(ec);
	}
	require(i == nsim && (bins[0] + bins[1]) == nsim);
	/*
         * By my calculation of the Chernoff upper bound for the cumulative
	 * distribution function of the binomial distribution,
	 * https://en.wikipedia.org/wiki/Binomial_distribution#Tail_bounds
	 * , with n = 1000 and p = 0.5, 432 is the closest point to where the
	 * upper bound crosses a probability of .0001.
	 */
	require(bins[0] >= 432 && bins[0] <= 568);
	ok;
}

/*
 * In the current implementation, a SELECT effect nested in another RANDOM
 * (or SELECT) effect is treated as a do-nothing effect with no subeffects.
 * This test exercises that.  If the implementation changes to allow proper
 * nesting of SELECT effects, this test will have to be changed.
 */
static int test_nested_random_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_RANDOM, 0, 0, "NONE", "5" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		player_clear_timed(player, TMD_BOLD, false);
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require((player->chp == player->mhp - 1 || player->timed[TMD_BOLD] ||
		(player->chp == player->mhp && !player->timed[TMD_BOLD])) &&
		!(player->chp == player->mhp - 1 && player->timed[TMD_BOLD]));
	ok;
}

/*
 * A SELECT effect with a negative number of subeffects should do nothing
 * successfully.
 */
static int test_selectneg_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_SELECT, 0, 0, "NONE", "-4+1d2" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	ok;
}

/* A SELECT effect with zero subeffects should do nothing successfully.  */
static int test_select0_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_SELECT, 0, 0, "NONE", "0" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	ok;
}

/*
 * A SELECT effect with only one choice should not prompt so this test should
 * run without special steps to avoid the prompt.
 */
static int test_select1_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_SELECT, 0, 0, "NONE", "1" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require(player->chp == player->mhp - 1);
	ok;
}

/*
 * A SELECT with more than one subeffect will prompt so provide a command object
 * with a preselected choice to bypass the prompt.
 */
static int test_select2_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_SELECT, 0, 0, "NONE", "2" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;
	int choice = randint0(2);

	if (ec) {
		struct command cmd;

		restore_to_full_health();
		player_clear_timed(player, TMD_BOLD, false);
		memset(&cmd, 0, sizeof(cmd));
		cmd_set_arg_choice(&cmd, "list_index", choice);
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, &cmd);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require(choice == 0 || choice == 1);
	if (choice == 0) {
		require(player->chp == player->mhp - 1 &&
			!player->timed[TMD_BOLD]);
	} else {
		require(player->chp == player->mhp && player->timed[TMD_BOLD]);
	}
	ok;
}

/*
 * Check that a SELECT effect which says it has more subeffects than it does
 * is handled gracefully.
 */
static int test_selectover_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_SELECT, 0, 0, "NONE", "5" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;
	int choice = randint0(5);

	if (ec) {
		struct command cmd;

		memset(&cmd, 0, sizeof(cmd));
		cmd_set_arg_choice(&cmd, "list_index", choice);
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, &cmd);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	ok;
}

/*
 * In the current implementation, a SELECT effect nested in another SELECT
 * (or RANDOM) effect is treated as a do-nothing effect with no subeffects.
 * This test exercises that.  If the implementation changes to allow proper
 * nesting of SELECT effects, this test will have to be changed.
 */
static int test_nested_select_execute(void *state) {
	struct simple_effect ea[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_SELECT, 0, 0, "NONE", "5" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	bool completed = false, ident = true;

	if (ec) {
		restore_to_full_health();
		player_clear_timed(player, TMD_BOLD, false);
		completed = effect_do(ec, source_player(), NULL, &ident, true,
			0, 0, false, NULL);
		free_effect(ec);
	}
	noteq(ec, NULL);
	require(completed);
	require(ident);
	require((player->chp == player->mhp - 1 || player->timed[TMD_BOLD] ||
		(player->chp == player->mhp && !player->timed[TMD_BOLD])) &&
		!(player->chp == player->mhp - 1 && player->timed[TMD_BOLD]));
	ok;
}

static int test_random_select_damages(void *state)
{
	struct simple_effect ea1[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_HEAL_HP, 0, 0, "NONE", "5" },
		{ EF_BOLT, 0, 0, "ACID", "1" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct simple_effect ea2[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_HEAL_HP, 0, 0, "NONE", "5" },
		{ EF_TIMED_INC, 0, 0, "FAST", "8" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct effect *ec1 = build_effect_chain(ea1, (int)N_ELEMENTS(ea1));
	struct effect *ec2 = build_effect_chain(ea2, (int)N_ELEMENTS(ea2));
	const bool expected[4] = { true, true, false, false };
	bool results[4] = { false, false, false, false };

	if (ec1) {
		results[0] = effect_damages(ec1);
		ec1->index = EF_SELECT;
		results[1] = effect_damages(ec1);
		free_effect(ec1);
	}
	if (ec2) {
		results[2] = effect_damages(ec2);
		ec2->index = EF_SELECT;
		results[3] = effect_damages(ec2);
		free_effect(ec2);
	}
	require(ec1 && ec2);
	require(results[0] == expected[0] && results[1] == expected[1] &&
		results[2] == expected[2] && results[3] == expected[3]);
	ok;
}

static int test_random_select_avg_damage(void *state)
{
	struct simple_effect ea1[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_HEAL_HP, 0, 0, "NONE", "5" },
		{ EF_BOLT, 0, 0, "ACID", "3d5" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
	};
	struct simple_effect ea2[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_BOLT, 0, 0, "FIRE", "1d7" },
		{ EF_ARC, 0, 0, "COLD", "3+1d3" },
		{ EF_BOLT_OR_BEAM, 0, 0, "POIS", "6" },
	};
	struct effect *ec1 = build_effect_chain(ea1, (int)N_ELEMENTS(ea1));
	struct effect *ec2 = build_effect_chain(ea2, (int)N_ELEMENTS(ea2));
	const int expected[4] = { 3, 3, 5, 5 };
	int results[4] = { -1, -1, -1, -1 };

	if (ec1) {
		results[0] = effect_avg_damage(ec1);
		ec1->index = EF_SELECT;
		results[1] = effect_avg_damage(ec1);
		free_effect(ec1);
	}
	if (ec2) {
		results[2] = effect_avg_damage(ec2);
		ec2->index = EF_SELECT;
		results[3] = effect_avg_damage(ec2);
		free_effect(ec2);
	}
	require(results[0] == expected[0] && results[1] == expected[1] &&
		results[2] == expected[2] && results[3] == expected[3]);
	ok;
}

static int test_random_select_projection(void *state)
{
	struct simple_effect ea1[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_BALL, 3, 0, "ACID", "5" },
		{ EF_BOLT, 0, 0, "ACID", "1" },
		{ EF_ARC, 0, 0, "ACID", "10" },
	};
	struct simple_effect ea2[] = {
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_BOLT, 0, 0, "FIRE", "5" },
		{ EF_BREATH, 0, 0, "ELEC", "8" },
		{ EF_LASH, 0, 0, "POIS", "10" },
	};
	struct effect *ec1 = build_effect_chain(ea1, (int)N_ELEMENTS(ea1));
	struct effect *ec2 = build_effect_chain(ea2, (int)N_ELEMENTS(ea2));
	const char *expected[4] = { "acid", "acid", "", "" };
	bool results[4] = { false, false, false, false };
	const char* result;

	if (ec1) {
		result = effect_projection(ec1);
		results[0] = streq(result, expected[0]);
		ec1->index = EF_SELECT;
		result = effect_projection(ec1);
		results[1] = streq(result, expected[1]);
		free_effect(ec1);
	}
	if (ec2) {
		result = effect_projection(ec2);
		results[2] = streq(result, expected[2]);
		ec2->index = EF_SELECT;
		result = effect_projection(ec2);
		results[3] = streq(result, expected[3]);
		free_effect(ec2);
	}
	require(results[0] && results[1] && results[2] && results[3]);
	ok;
}

static int test_iterate1(void *state)
{
	struct simple_effect ea[] = {
		{ EF_DAMAGE, 0, 0, "NONE", "0" },
		{ EF_RANDOM, 0, 0, "NONE", "3" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_HEAL_HP, 0, 0, "NONE", "2" },
		{ EF_TIMED_INC, 0, 0, "BOLD", "10" },
		{ EF_DAMAGE, 0, 0, "NONE", "1" },
		{ EF_SELECT, 0, 0, "NONE", "2" },
		{ EF_TIMED_INC, 0, 0, "STUN", "5" },
		{ EF_TIMED_INC, 0, 0, "FAST", "5" },
		{ EF_HEAL_HP, 0, 0, "NONE", "2" },
	};
	struct effect *ec = build_effect_chain(ea, (int)N_ELEMENTS(ea));
	/* This is a flat array for the effect addresses in the chain. */
	int namax = 10, nacurr = 0;
	struct effect **a = mem_alloc(namax * sizeof(*a));
	/* This is a flat array for the effect address from iteration. */
	int naimax = 10, naicurr = 0;
	struct effect **ai = mem_alloc(naimax * sizeof(*ai));
	/* These as are the expected results as indices into a. */
	const int expected[] = { 0, 1, 5, 6, 9 };
	int i, nmatch;

	if (ec) {
		struct effect *e = ec;

		/* Build up a. */
		while (e) {
			if (nacurr >= namax) {
				namax *= 2;
				a = mem_realloc(a, namax * sizeof(*a));
			}
			a[nacurr] = e;
			++nacurr;
			e = e->next;
		}

		/* Get the iteration results. */
		e = ec;
		while (e) {
			if (naicurr >= naimax) {
				naimax *= 2;
				ai = mem_realloc(ai, naimax * sizeof(*ai));
			}
			ai[naicurr] = e;
			++naicurr;
			e = effect_next(e);
		}
		free_effect(ec);
	}
	for (i = 0, nmatch = 0;
			i < naicurr && i < (int)N_ELEMENTS(expected); ++i) {
		if (ai[i] == a[expected[i]]) {
			++nmatch;
		}
	}
	mem_free(ai);
	mem_free(a);
	require(nmatch == naicurr && nmatch == (int)N_ELEMENTS(expected));
	ok;
}

const char *suite_name = "effects/chain";
struct test tests[] = {
	{ "chain1_execute", test_chain1_execute },
	{ "chain2_execute", test_chain2_execute },
	{ "chain3_execute", test_chain3_execute },
	{ "randomneg_execute", test_randomneg_execute },
	{ "random0_execute", test_random0_execute },
	{ "random1_execute", test_random1_execute },
	{ "random2_execute", test_random2_execute },
	{ "randomover_execute", test_randomover_execute },
	{ "nested_random_execute", test_nested_random_execute },
	{ "test_random_stats", test_random_stats },
	{ "selectneg_execute", test_selectneg_execute },
	{ "select0_execute", test_select0_execute },
	{ "select1_execute", test_select1_execute },
	{ "select2_execute", test_select2_execute },
	{ "selectover_execute", test_selectover_execute },
	{ "nested_select_execute", test_nested_select_execute },
	{ "random_select_damages", test_random_select_damages },
	{ "random_select_avg_damage", test_random_select_avg_damage },
	{ "random_select_projection", test_random_select_projection },
	{ "iterate1", test_iterate1 },
	{ NULL, NULL }
};
