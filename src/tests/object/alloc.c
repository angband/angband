/* object/alloc */

#include "unit-test.h"
#include "unit-test-data.h"

#include "init.h"
#include "object.h"
#include "obj-make.h"
#include "obj-properties.h"
#include <math.h>

struct alloc_test_state {
	int *histogram;
	double *expected;
};

extern struct init_module obj_make_module;


int setup_tests(void **state) {
	struct alloc_test_state *st;

	player = &test_player;

	z_info = mem_zalloc(sizeof(*z_info));
	z_info->k_max = 5;
	/* Won't set up any egos for testing. */
	z_info->e_max = 0;
	z_info->max_obj_depth = 2;
	z_info->great_obj = 20;

	/*
	 * Set up a small number of fake kinds with a mix of the attributes
	 * get_obj_num() uses.
	 */
	k_info = mem_zalloc(sizeof(*k_info) * z_info->k_max);
	k_info[0].alloc_min = 0;
	k_info[0].alloc_max = 1;
	k_info[0].alloc_prob = 80;
	k_info[0].tval = TV_LIGHT;
	k_info[1].alloc_min = 0;
	k_info[1].alloc_max = 6;
	k_info[1].alloc_prob = 40;
	k_info[1].tval = TV_POTION;
	k_info[2].alloc_min = 0;
	k_info[2].alloc_max = 6;
	k_info[2].alloc_prob = 20;
	k_info[2].tval = TV_WAND;
	k_info[3].alloc_min = 0;
	k_info[3].alloc_max = 6;
	k_info[3].alloc_prob = 10;
	k_info[3].tval = TV_POTION;
	kf_on(k_info[3].kind_flags, KF_GOOD);
	k_info[4].alloc_min = 1;
	k_info[4].alloc_max = 6;
	k_info[4].alloc_prob = 5;
	k_info[4].tval = TV_WAND;
	kf_on(k_info[4].kind_flags, KF_GOOD);

	st = mem_alloc(sizeof(*st));
	st->histogram = mem_alloc(z_info->k_max * sizeof(*st->histogram));
	st->expected = mem_alloc(z_info->k_max * sizeof(*st->expected));
	*state = st;

	(*obj_make_module.init)();

	return 0;
}


int teardown_tests(void *state) {
	struct alloc_test_state *st = state;

	(*obj_make_module.cleanup)();
	if (st) {
		mem_free(st->expected);
		mem_free(st->histogram);
		mem_free(st);
	}
	mem_free(k_info);
	mem_free(z_info);
	return 0;
}


/*
 * Compute the G-test statistic, https://en.wikipedia.org/wiki/G-test , given
 * the number of observed occurences, obs, in n different categories with
 * ntotal observations made and the expected probability, ex, of each of those
 * categories.  The distribution for the result is expected to be approximately
 * a chi-squared distribution with the degrees of freedom equal to one less
 * than the number of nonzero elements in ex.
 */
static double compute_gtest_statistic(const int *ob, const double *ex,
	int n, int ntotal)
{
	double result = 0.0;
	int i;

	for (i = 0; i < n; ++i) {
		double oprob;

		/* Zero observations don't contribute. */
		if (!ob[i]) continue;

		/*
		 * There were nonzero observations in a supposedly impossible
		 * category.
		 */
		if (ex[i] == 0.0) return 1e30;

		oprob = ob[i] / (double) ntotal;
		result += oprob * log(oprob / ex[i]);
	}

	return 2.0 * result;
}


/*
 * Returns true if the chance of drawing a value greater than or equal to v
 * from a chi-squared distribution with ndof degrees of freedom is less than
 * .1%
 */
static bool satisfies_chisq_criteria(double v, int ndof)
{
	const double table[] = {
		10.82757, 13.81551, 16.26624, 18.46683, 20.51501
	};

	if (ndof <= 0 || ndof > (int)N_ELEMENTS(table)) {
		return false;
	}
	return v < table[ndof - 1];
}


static int histogram_max(const int* histogram, int nbin)
{
	int result = 0;
	int i;

	for (i = 0; i < nbin; ++i) {
		if (result < histogram[i]) {
			result = histogram[i];
		}
	}
	return result;
}


static void compute_obj_num_expected(int level, bool good, int tval,
	double *ex, int *nnonzero)
{
	double pboost = 1.0 / z_info->great_obj;
	int i;

	for (i = 0; i < z_info->k_max; ++i) {
		ex[i] = 0.0;
	}

	/*
	 * Add up the probabilities from being at a boosted level or
	 * at the specified level.  To reduce round-off error, do the
	 * boosted levels first since those probabilities are likely to be
	 * smaller.
	 */
	for (i = z_info->max_obj_depth; i >= 0; --i) {
		double mult;
		int total = 0, boosted_level, j;

		if (i == 0) {
			mult = 1.0 - pboost;
			boosted_level = level;
		} else {
			mult = pboost / z_info->max_obj_depth;
			boosted_level = 1 +
				(level * z_info->max_obj_depth / i);
			if (boosted_level > z_info->max_obj_depth) {
				boosted_level = z_info->max_obj_depth;
			}
		}

		for (j = 0; j < z_info->k_max; ++j) {
			if (good && !kf_has(k_info[j].kind_flags, KF_GOOD)) continue;
			if (tval != 0 && k_info[j].tval != tval) continue;
			if (k_info[j].alloc_min > boosted_level ||
				k_info[j].alloc_max < boosted_level) continue;
			total += k_info[j].alloc_prob;
		}
		if (!total) continue;

		for (j = 0; j < z_info->k_max; ++j) {
			if (good && !kf_has(k_info[j].kind_flags, KF_GOOD)) continue;
			if (tval != 0 && k_info[j].tval != tval) continue;
			if (k_info[j].alloc_min > boosted_level ||
				k_info[j].alloc_max < boosted_level) continue;
			ex[j] += mult * (double) k_info[j].alloc_prob / total;
		}
	}

	*nnonzero = 0;
	for (i = 0; i < z_info->k_max; ++i) {
		if (ex[i] > 0.0) {
			++*nnonzero;
		}
	}
}


/*
 * There is a non-deterministic element to this test.  The threshold for
 * failure is unlikey to be met (expected to be .1% for each of the 7 tests
 * below that can get more than one kind of object) but can happen even if
 * get_obj_num() performs correctly.
 */
int test_get_obj_num_basic(void *state) {
	struct {
		int level, tval;
		bool good;
	} tests[] = {
		{ 0, 0, false },
		{ 1, 0, false },
		{ 2, 0, false },
		{ 1, 0, true },
		{ 0, TV_POTION, false },
		{ 1, TV_POTION, false },
		{ 2, TV_POTION, false },
		{ 1, TV_POTION, true }
	};
	int ntrials, i;
	struct alloc_test_state *st = state;

	/* Check for requesting a tval that can't be satisfied. */
	null(get_obj_num(1, false, TV_CHEST));

	/*
	 * Set the number of trials so the expected number of times an object
	 * will appear in the trials is large enough that the assumptions
	 * for the G-test should be satisfied.
	 */
	ntrials = 0;
	for (i = 0; i < z_info->k_max; ++i) {
		ntrials += k_info[i].alloc_prob;
	}
	ntrials *= 30;

	for (i = 0; i < (int)N_ELEMENTS(tests); ++i) {
		int j, nnonzero;

		for (j = 0; j < z_info->k_max; ++j) {
			st->histogram[j] = 0;
		}
		for (j = 0; j < ntrials; ++j) {
			const struct object_kind *kind =
				get_obj_num(tests[i].level, tests[i].good,
					tests[i].tval);
			int k;

			/*
			 * The tests have been configured so an object kind
			 * should always be found.
			 */
			notnull(kind);

			k = 0;
			while (1) {
				require(k < z_info->k_max);
				if (kind == k_info + k) {
					/*
					 * Check that the minimum level is met.
					 * Do not check the maximum level
					 * because of the possibility of level
					 * boosting.
					 */
					require(tests[i].level >= kind->alloc_min);
					if (tests[i].good) {
						require(kf_has(kind->kind_flags, KF_GOOD));
					}
					if (tests[i].tval != 0) {
						eq(kind->tval, tests[i].tval);
					}
					++st->histogram[k];
					break;
				}
				++k;
			}
		}

		compute_obj_num_expected(tests[i].level, tests[i].good,
			tests[i].tval, st->expected, &nnonzero);
		require(nnonzero >= 1);
		if (nnonzero == 1) {
			require(histogram_max(st->histogram, z_info->k_max) == ntrials);
		} else {
			double gtest = compute_gtest_statistic(st->histogram,
				st->expected, z_info->k_max, ntrials);

			require(satisfies_chisq_criteria(gtest, nnonzero - 1));
		}
	}

	ok;
}


const char *suite_name = "object/alloc";
struct test tests[] = {
	{ "get_obj_num_basic", test_get_obj_num_basic },
	{ NULL, NULL }
};
