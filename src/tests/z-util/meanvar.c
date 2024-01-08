/* z-util/meanvar.c */

#include "unit-test.h"
#include "z-util.h"

NOSETUP
NOTEARDOWN

static int test_mean_trivial(void *state)
{
	struct my_rational f;
	int m;

	m = mean(NULL, -10, NULL);
	eq(m, 0);
	m = mean(NULL, -10, &f);
	eq(m, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	m = mean(NULL, 0, NULL);
	eq(m, 0);
	m = mean(NULL, 0, &f);
	eq(m, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	ok;
}

static int test_mean_simple(void *state)
{
	const int case1[1] = { 5 };
	const int case2[4] = { 0, 0, 0, 0 };
	const int case3[3] = { -3, 4, -7 };
	const int case4[5] = { 4, -7, 5, -1, 3 };
	const int case5[6] = { 2, 3, 2, 2, 2, 3 };
	const int case6[4] = { -1, 0, 1, -1 };
	const int case7[5] = { -4, -5, -6, -4, -5 };
	struct my_rational f;
	int m;

	m = mean(case1, (int)N_ELEMENTS(case1), NULL);
	eq(m, 5);
	m = mean(case1, (int)N_ELEMENTS(case1), &f);
	eq(m, 5);
	eq(f.n, 0);
	eq(f.d, 1);
	m = mean(case2, (int)N_ELEMENTS(case2), NULL);
	eq(m, 0);
	m = mean(case2, (int)N_ELEMENTS(case2), &f);
	eq(m, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	m = mean(case3, (int)N_ELEMENTS(case3), NULL);
	eq(m, -2);
	m = mean(case3, (int)N_ELEMENTS(case3), &f);
	eq(m, -2);
	eq(f.n, 0);
	eq(f.d, 1);
	m = mean(case4, (int)N_ELEMENTS(case4), NULL);
	eq(m, 1);
	m = mean(case4, (int)N_ELEMENTS(case4), &f);
	eq(m, 0);
	eq(f.n, 4);
	eq(f.d, 5);
	m = mean(case5, (int)N_ELEMENTS(case5), NULL);
	eq(m, 2);
	m = mean(case5, (int)N_ELEMENTS(case5), &f);
	eq(m, 2);
	eq(f.n, 1);
	eq(f.d, 3);
	m = mean(case6, (int)N_ELEMENTS(case6), NULL);
	eq(m, 0);
	m = mean(case6, (int)N_ELEMENTS(case6), &f);
	eq(m, -1);
	eq(f.n, 3);
	eq(f.d, 4);
	m = mean(case7, (int)N_ELEMENTS(case7), NULL);
	eq(m, -5);
	m = mean(case7, (int)N_ELEMENTS(case7), &f);
	eq(m, -5);
	eq(f.n, 1);
	eq(f.d, 5);
	ok;
}

static int test_mean_overflow(void *state)
{
	/*
	 * Try combinations that would trigger overflow with naive
	 * implementations.
	 */
	const int case1[3] = { INT_MIN, INT_MIN, INT_MIN };
	const int case2[4] = { INT_MAX, INT_MAX, INT_MAX, INT_MAX };
	const int case3[6] = { INT_MAX, INT_MAX, INT_MAX, INT_MIN, INT_MIN,
		INT_MIN };
	struct my_rational f;
	int m;

	m = mean(case1, (int)N_ELEMENTS(case1), NULL);
	eq(m, INT_MIN);
	m = mean(case1, (int)N_ELEMENTS(case1), &f);
	eq(m, INT_MIN);
	eq(f.n, 0);
	eq(f.d, 1);
	m = mean(case2, (int)N_ELEMENTS(case2), NULL);
	eq(m, INT_MAX);
	m = mean(case2, (int)N_ELEMENTS(case2), &f);
	eq(m, INT_MAX);
	eq(f.n, 0);
	eq(f.d, 1);
	m = mean(case3, (int)N_ELEMENTS(case3), NULL);
	if (INT_MIN + INT_MAX < 0) {
		eq(m, (INT_MIN + INT_MAX) / 2
			+ (((INT_MIN + INT_MAX) % 2 != 0) ? -1 : 0));
	} else {
		eq(m, (INT_MIN + INT_MAX) / 2
			+ (((INT_MIN + INT_MAX) % 2 != 0) ? 1 : 0));
	}
	m = mean(case3, (int)N_ELEMENTS(case3), &f);
	if (INT_MIN + INT_MAX < 0) {
		if ((INT_MIN + INT_MAX) % 2 != 0) {
			eq(m, (INT_MIN + INT_MAX) / 2 - 1);
			eq(f.n, 1);
			eq(f.d, 2);
		} else {
			eq(m, (INT_MIN + INT_MAX) / 2);
			eq(f.n, 0);
			eq(f.d, 1);
		}
	} else {
		eq(m, (INT_MIN + INT_MAX) / 2);
		if ((INT_MIN + INT_MAX) % 2 != 0) {
			eq(f.n, 1);
			eq(f.d, 2);
		} else {
			eq(f.n, 0);
			eq(f.d, 1);
		}
	}
	ok;
}

static int test_variance_trivial(void *state)
{
	struct my_rational f;
	int v;

	v = variance(NULL, -8, false, false, NULL);
	eq(v, 0);
	v = variance(NULL, -8, true, false, NULL);
	eq(v, 0);
	v = variance(NULL, -8, false, true, NULL);
	eq(v, 0);
	v = variance(NULL, -8, true, true, NULL);
	eq(v, 0);
	v = variance(NULL, -8, false, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, -8, true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, -8, false, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, -8, true, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 0, false, false, NULL);
	eq(v, 0);
	v = variance(NULL, 0, true, false, NULL);
	eq(v, 0);
	v = variance(NULL, 0, false, true, NULL);
	eq(v, 0);
	v = variance(NULL, 0, true, true, NULL);
	eq(v, 0);
	v = variance(NULL, 0, false, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 0, true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 0, false, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 0, true, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 1, false, false, NULL);
	eq(v, 0);
	v = variance(NULL, 1, true, false, NULL);
	eq(v, 0);
	v = variance(NULL, 1, false, true, NULL);
	eq(v, 0);
	v = variance(NULL, 1, true, true, NULL);
	eq(v, 0);
	v = variance(NULL, 1, false, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 1, true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 1, false, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(NULL, 1, true, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	ok;
}

static int test_variance_simple(void *state)
{
	const int case1[2] = { 3, 4 };
	const int case2[4] = { 0, 0, 0, 0 };
	const int case3[3] = { -3, 4, -7 };
	const int case4[5] = { 4, -7, 5, -1, 3 };
	const int case5[6] = { 2, 3, 2, 2, 2, 3 };
	const int case6[4] = { -1, 0, 1, -1 };
	const int case7[5] = { -4, -5, -6, -4, -5 };
	struct my_rational f;
	int v;

	v = variance(case1, (int)N_ELEMENTS(case1), false, false, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), true, false, NULL);
	eq(v, 1);
	v = variance(case1, (int)N_ELEMENTS(case1), false, true, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), true, true, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), false, false, &f);
	eq(v, 0);
	eq(f.n, 1);
	eq(f.d, 4);
	v = variance(case1, (int)N_ELEMENTS(case1), true, false, &f);
	eq(v, 0);
	eq(f.n, 1);
	eq(f.d, 2);
	v = variance(case1, (int)N_ELEMENTS(case1), false, true, &f);
	eq(v, 0);
	eq(f.n, 1);
	eq(f.d, 8);
	v = variance(case1, (int)N_ELEMENTS(case1), true, true, &f);
	eq(v, 0);
	eq(f.n, 1);
	eq(f.d, 4);
	v = variance(case2, (int)N_ELEMENTS(case2), false, false, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), true, false, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), false, true, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), true, true, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), false, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), true, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case3, (int)N_ELEMENTS(case3), false, false, NULL);
	eq(v, 21);
	v = variance(case3, (int)N_ELEMENTS(case3), true, false, NULL);
	eq(v, 31);
	v = variance(case3, (int)N_ELEMENTS(case3), false, true, NULL);
	eq(v, 7);
	v = variance(case3, (int)N_ELEMENTS(case3), true, true, NULL);
	eq(v, 10);
	v = variance(case3, (int)N_ELEMENTS(case3), false, false, &f);
	eq(v, 20);
	eq(f.n, 2);
	eq(f.d, 3);
	v = variance(case3, (int)N_ELEMENTS(case3), true, false, &f);
	eq(v, 31);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case3, (int)N_ELEMENTS(case3), false, true, &f);
	eq(v, 6);
	eq(f.n, 8);
	eq(f.d, 9);
	v = variance(case3, (int)N_ELEMENTS(case3), true, true, &f);
	eq(v, 10);
	eq(f.n, 1);
	eq(f.d, 3);
	v = variance(case4, (int)N_ELEMENTS(case4), false, false, NULL);
	eq(v, 19);
	v = variance(case4, (int)N_ELEMENTS(case4), true, false, NULL);
	eq(v, 24);
	v = variance(case4, (int)N_ELEMENTS(case4), false, true, NULL);
	eq(v, 4);
	v = variance(case4, (int)N_ELEMENTS(case4), true, true, NULL);
	eq(v, 5);
	v = variance(case4, (int)N_ELEMENTS(case4), false, false, &f);
	eq(v, 19);
	eq(f.n, 9);
	eq(f.d, 25);
	v = variance(case4, (int)N_ELEMENTS(case4), true, false, &f);
	eq(v, 24);
	eq(f.n, 1);
	eq(f.d, 5);
	v = variance(case4, (int)N_ELEMENTS(case4), false, true, &f);
	eq(v, 3);
	eq(f.n, 109);
	eq(f.d, 125);
	v = variance(case4, (int)N_ELEMENTS(case4), true, true, &f);
	eq(v, 4);
	eq(f.n, 21);
	eq(f.d, 25);
	v = variance(case5, (int)N_ELEMENTS(case5), false, false, NULL);
	eq(v, 0);
	v = variance(case5, (int)N_ELEMENTS(case5), true, false, NULL);
	eq(v, 0);
	v = variance(case5, (int)N_ELEMENTS(case5), false, true, NULL);
	eq(v, 0);
	v = variance(case5, (int)N_ELEMENTS(case5), true, true, NULL);
	eq(v, 0);
	v = variance(case5, (int)N_ELEMENTS(case5), false, false, &f);
	eq(v, 0);
	eq(f.n, 2);
	eq(f.d, 9);
	v = variance(case5, (int)N_ELEMENTS(case5), true, false, &f);
	eq(v, 0);
	eq(f.n, 4);
	eq(f.d, 15);
	v = variance(case5, (int)N_ELEMENTS(case5), false, true, &f);
	eq(v, 0);
	eq(f.n, 1);
	eq(f.d, 27);
	v = variance(case5, (int)N_ELEMENTS(case5), true, true, &f);
	eq(v, 0);
	eq(f.n, 2);
	eq(f.d, 45);
	v = variance(case6, (int)N_ELEMENTS(case6), false, false, NULL);
	eq(v, 1);
	v = variance(case6, (int)N_ELEMENTS(case6), true, false, NULL);
	eq(v, 1);
	v = variance(case6, (int)N_ELEMENTS(case6), false, true, NULL);
	eq(v, 0);
	v = variance(case6, (int)N_ELEMENTS(case6), true, true, NULL);
	eq(v, 0);
	v = variance(case6, (int)N_ELEMENTS(case6), false, false, &f);
	eq(v, 0);
	eq(f.n, 11);
	eq(f.d, 16);
	v = variance(case6, (int)N_ELEMENTS(case6), true, false, &f);
	eq(v, 0);
	eq(f.n, 11);
	eq(f.d, 12);
	v = variance(case6, (int)N_ELEMENTS(case6), false, true, &f);
	eq(v, 0);
	eq(f.n, 11);
	eq(f.d, 64);
	v = variance(case6, (int)N_ELEMENTS(case6), true, true, &f);
	eq(v, 0);
	eq(f.n, 11);
	eq(f.d, 48);
	v = variance(case7, (int)N_ELEMENTS(case7), false, false, NULL);
	eq(v, 1);
	v = variance(case7, (int)N_ELEMENTS(case7), true, false, NULL);
	eq(v, 1);
	v = variance(case7, (int)N_ELEMENTS(case7), false, true, NULL);
	eq(v, 0);
	v = variance(case7, (int)N_ELEMENTS(case7), true, true, NULL);
	eq(v, 0);
	v = variance(case7, (int)N_ELEMENTS(case7), false, false, &f);
	eq(v, 0);
	eq(f.n, 14);
	eq(f.d, 25);
	v = variance(case7, (int)N_ELEMENTS(case7), true, false, &f);
	eq(v, 0);
	eq(f.n, 7);
	eq(f.d, 10);
	v = variance(case7, (int)N_ELEMENTS(case7), false, true, &f);
	eq(v, 0);
	eq(f.n, 14);
	eq(f.d, 125);
	v = variance(case7, (int)N_ELEMENTS(case7), true, true, &f);
	eq(v, 0);
	eq(f.n, 7);
	eq(f.d, 50);
	ok;
}

static int test_variance_overflow(void *state)
{
	const int case1[3] = { INT_MIN, INT_MIN, INT_MIN };
	const int case2[4] = { INT_MAX, INT_MAX, INT_MAX, INT_MAX };
	const int case3[6] = { INT_MIN, INT_MAX, INT_MIN, INT_MAX, INT_MIN,
		INT_MAX };
	const int case4[6] = {
		1 << ((sizeof(int) * CHAR_BIT) / 2 - 1),
		-(1 << ((sizeof(int) * CHAR_BIT) / 2 - 1)),
		1 << ((sizeof(int) * CHAR_BIT) / 2 - 1),
		-(1 << ((sizeof(int) * CHAR_BIT) / 2 - 1)),
		1 << ((sizeof(int) * CHAR_BIT) / 2 - 1),
		-(1 << ((sizeof(int) * CHAR_BIT) / 2 - 1)),
	};
	struct my_rational f, fexp_ff, fexp_ft, fexp_tf, fexp_tt, fpart;
	int v, vexp_ff, vexp_ft, vexp_tf, vexp_tt;

	v = variance(case1, (int)N_ELEMENTS(case1), false, false, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), true, false, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), false, true, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), true, true, NULL);
	eq(v, 0);
	v = variance(case1, (int)N_ELEMENTS(case1), false, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case1, (int)N_ELEMENTS(case1), true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case1, (int)N_ELEMENTS(case1), false, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case1, (int)N_ELEMENTS(case1), true, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), false, false, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), true, false, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), false, true, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), true, true, NULL);
	eq(v, 0);
	v = variance(case2, (int)N_ELEMENTS(case2), false, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), true, false, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), false, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case2, (int)N_ELEMENTS(case2), true, true, &f);
	eq(v, 0);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case3, (int)N_ELEMENTS(case3), false, false, NULL);
	eq(v, INT_MAX);
	v = variance(case3, (int)N_ELEMENTS(case3), true, false, NULL);
	eq(v, INT_MAX);
	v = variance(case3, (int)N_ELEMENTS(case3), false, true, NULL);
	eq(v, INT_MAX);
	v = variance(case3, (int)N_ELEMENTS(case3), true, true, NULL);
	eq(v, INT_MAX);
	v = variance(case3, (int)N_ELEMENTS(case3), false, false, &f);
	eq(v, INT_MAX);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case3, (int)N_ELEMENTS(case3), true, false, &f);
	eq(v, INT_MAX);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case3, (int)N_ELEMENTS(case3), false, true, &f);
	eq(v, INT_MAX);
	eq(f.n, 0);
	eq(f.d, 1);
	v = variance(case3, (int)N_ELEMENTS(case3), true, true, &f);
	eq(v, INT_MAX);
	eq(f.n, 0);
	eq(f.d, 1);
	vexp_ff = case4[0] * case4[0];
	fexp_ff = my_rational_construct(0, 1);
	vexp_tf = ((case4[0] * case4[0]) / ((int)N_ELEMENTS(case4) - 1))
		* (int)N_ELEMENTS(case4)
		+ (((case4[0] * case4[0]) % ((int)N_ELEMENTS(case4) - 1))
		* (int)N_ELEMENTS(case4)) / ((int)N_ELEMENTS(case4) - 1);
	fexp_tf = my_rational_construct((((case4[0] * case4[0])
		% ((int)N_ELEMENTS(case4) - 1)) * (int)N_ELEMENTS(case4))
		% ((int)N_ELEMENTS(case4) - 1), (int)N_ELEMENTS(case4) - 1);
	vexp_ft = vexp_ff / (int)N_ELEMENTS(case4);
	fpart = my_rational_construct(1, (int)N_ELEMENTS(case4));
	fexp_ft = my_rational_product(&fexp_ff, &fpart);
	fpart = my_rational_construct(vexp_ff % (int)N_ELEMENTS(case4),
		(int)N_ELEMENTS(case4));
	fexp_ft = my_rational_sum(&fexp_ft, &fpart);
	vexp_tt = vexp_tf / (int)N_ELEMENTS(case4);
	fpart = my_rational_construct(1, (int)N_ELEMENTS(case4));
	fexp_tt = my_rational_product(&fexp_tf, &fpart);
	fpart = my_rational_construct(vexp_tf % (int)N_ELEMENTS(case4),
		(int)N_ELEMENTS(case4));
	fexp_tt = my_rational_sum(&fexp_tt, &fpart);
	v = variance(case4, (int)N_ELEMENTS(case4), false, false, NULL);
	eq(v, vexp_ff + ((fexp_ff.n >= (fexp_ff.d + 1) / 2) ? 1 : 0));
	v = variance(case4, (int)N_ELEMENTS(case4), true, false, NULL);
	eq(v, vexp_tf + ((fexp_tf.n >= (fexp_tf.d + 1) / 2) ? 1 : 0));
	v = variance(case4, (int)N_ELEMENTS(case4), false, true, NULL);
	eq(v, vexp_ft + ((fexp_ft.n >= (fexp_ft.d + 1) / 2) ? 1 : 0));
	v = variance(case4, (int)N_ELEMENTS(case4), true, true, NULL);
	eq(v, vexp_tt + ((fexp_tt.n >= (fexp_tt.d + 1) / 2) ? 1 : 0));
	v = variance(case4, (int)N_ELEMENTS(case4), false, false, &f);
	eq(v, vexp_ff);
	eq(f.n, fexp_ff.n);
	eq(f.d, fexp_ff.d);
	v = variance(case4, (int)N_ELEMENTS(case4), true, false, &f);
	eq(v, vexp_tf);
	eq(f.n, fexp_tf.n);
	eq(f.d, fexp_tf.d);
	v = variance(case4, (int)N_ELEMENTS(case4), false, true, &f);
	eq(v, vexp_ft);
	eq(f.n, fexp_ft.n);
	eq(f.d, fexp_ft.d);
	v = variance(case4, (int)N_ELEMENTS(case4), true, true, &f);
	eq(v, vexp_tt);
	eq(f.n, fexp_tt.n);
	eq(f.d, fexp_tt.d);
	ok;
}

const char *suite_name = "z-util/meanvar";
struct test tests[] = {
	{ "mean trivial", test_mean_trivial },
	{ "mean simple", test_mean_simple },
	{ "mean overflow", test_mean_overflow },
	{ "variance trivial", test_variance_trivial },
	{ "variance simple", test_variance_simple },
	{ "variance overflow", test_variance_overflow },
	{ NULL, NULL }
};
