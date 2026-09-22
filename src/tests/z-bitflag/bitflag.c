/* z-bitflag/bitflag */
/*
 * Exercise routines from z-bitflag.h.
 *
 * The tests for flag_has() rely on knowing implementation details for the
 * bitflag routines.  The remaining tests use flag_has() or other previously
 * tested routines.
 */

#include "unit-test.h"
#include "z-bitflag.h"
#include "z-rand.h"

int setup_tests(void **state)
{
	Rand_init();
	return 0;
}

NOTEARDOWN

/*
 * Turn on n distinct random bits in a bitset and record their FLAG_START-based
 * indices, in ascending order, in p.
 */
static void random_bits(bitflag *f, int size, int n, int *p)
{
	int i;

	assert(n <= FLAG_MAX(size) - FLAG_START);
	flag_wipe(f, size);
	for (i = 0; i < n; ++i) {
		int r = randint0(FLAG_MAX(size) - FLAG_START - i)
			+ FLAG_START, j = 0;

		while (1) {
			if (j == i) {
				p[i] = r;
				break;
			}
			if (r < p[j]) {
				int k;

				for (k = i - 1; k >= j; --k) {
					p[k + 1] = p[k];
				}
				p[j] = r;
				break;
			}
			++r;
			++j;
		}
		(void)flag_on(f, size, r);
	}
}

static int test_has(void *state)
{
	bitflag f[256];
	unsigned int n = sizeof(f) / sizeof(f[0]), i, j;
	bool result, match;

	/* Test as single bytes. */
	for (i = 0; i < n; ++i) {
		f[i] = (uint8_t)i;
		result = flag_has(f + i, 1, FLAG_END);
		require(!result);
		for (j = 0; j < 8; ++j) {
			result = flag_has(f + i, 1, j + FLAG_START);
			match = (result && (i & (1 << j)))
				|| (!result && !(i & (1 << j)));
			if (!match) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: mismatch at "
						"bit %u of %.2x\n", suite_name,
						__LINE__, j, i);
				}
				return 1;
			}
			if (f[i] != i) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: corrupted "
						"pattern %.2x after checking "
						"bit %u\n", suite_name,
						__LINE__, i, j);
				}
				return 1;
			}
		}
	}

	/* Test as one large set. */
	result = flag_has(f, n, FLAG_END);
	require(!result);
	for (i = FLAG_START, j = 0; i < FLAG_MAX(256); ++i) {
		result = flag_has(f, n, i);
		match = (result && (j & (1 << ((i - FLAG_START) % 8))))
			|| (!result && !(j & (1 << ((i - FLAG_START) % 8))));
		if (!match) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: mismatch at bit %u "
					"(bit %u of chunk %u)\n", suite_name,
					__LINE__, i - FLAG_START,
					(i - FLAG_START) % 8, j);
			}
			return 1;
		}
		if (f[j] != j) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: corrupted chunk %u "
					"after checking bit %u\n", suite_name,
					__LINE__, j, i);
			}
			return 1;
		}
		if ((i - FLAG_START) % 8 == 7) {
			++j;
		}
	}

	ok;
}

static int test_wipe(void *state)
{
	bitflag f[5] = { 170, 85, 170, 85, 170 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2;
	int i;

	flag_wipe(f + 1, 0);
	eq(f[0], 170);
	eq(f[1], 85);
	eq(f[2], 170);

	flag_wipe(f + 1, n);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		bool result = flag_has(f + 1, n, i);

		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is on\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f[n + 1], 170);

	ok;
}

static int test_setall(void *state)
{
	bitflag f[4] = { 85, 170, 85, 170 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2;
	int i;

	flag_setall(f + 1, 0);
	eq(f[0], 85);
	eq(f[1], 170);
	eq(f[2], 85);

	flag_setall(f + 1, n);
	eq(f[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		bool result = flag_has(f + 1, n, i);

		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is off\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f[n + 1], 170);

	ok;
}

static int test_on(void *state)
{
	bitflag f[6] = { 170, 85, 170, 85, 170, 85 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2, i;

	flag_wipe(f + 1, n);

	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		bool changed = flag_on(f + 1, n, i);
		bool on;
		int j;

		if (!changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: said not changed at "
					"bit %d\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		if (f[0] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: leading guard chunk "
					"changed when bit %d changed\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
		for (j = FLAG_START; j < i; ++j) {
			on = flag_has(f + 1, n, j);
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: changing bit "
						"%d turned bit %d off\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: changing bit %d left "
					"it off\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		for (j = i + 1; j < FLAG_MAX(n); ++j) {
			on = flag_has(f + 1, n, j);
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: changing bit "
						"%d turned bit %d on\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		if (f[n + 1] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: trailing guard chunk "
					"changed when bit %d changed\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}

	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		bool changed = flag_on(f + 1, n, i);
		bool on;
		int j;

		if (changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: said changed at bit "
					"%d\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		if (f[0] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: leading guard chunk "
					"changed when setting bit %d\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
		for (j = FLAG_START; j < i; ++j) {
			on = flag_has(f + 1, n, j);
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: setting bit "
						"%d turned bit %d off\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: setting bit %d "
					"turned it off\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		for (j = i + 1; j < FLAG_MAX(n); ++j) {
			on = flag_has(f + 1, n, j);
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: setting bit %d "
						"turned bit %d off\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		if (f[n + 1] != 85) {
			if (verbose) {
				showfail();
				(void)printf("%s:%d: trailing guard chunk "
					"changed when setting bit %d\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}

	ok;
}

static int test_off(void *state)
{
	bitflag f[6] = { 85, 170, 85, 170, 85, 170 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2, i;

	flag_setall(f + 1, n);

	for (i = FLAG_MAX(n) - 1; i >= FLAG_START; --i) {
		bool changed = flag_off(f + 1, n, i);
		bool on;
		int j;

		if (!changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: said not changed at "
					"bit %d\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		if (f[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: trailing guard chunk "
					"changed when bit %d changed\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
		for (j = FLAG_MAX(n) - 1; j > i; --j) {
			on = flag_has(f + 1, n, j);
			if (on) {
				if (verbose) {
					showfail();
					(void)printf(    "%s:%d: changing bit "
						"%d turned bit %d on\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		on = flag_has(f + 1, n, i);
		if (on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: changing bit %d left "
					"it on\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		for (j = i - 1; j >= FLAG_START; --j) {
			on = flag_has(f + 1, n, j);
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: changing bit "
						"%d turned bit %d off\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		if (f[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: leading guard chunk "
					"changed when bit %d changed\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}

	for (i = FLAG_MAX(n) - 1; i >= FLAG_START; --i) {
		bool changed = flag_off(f + 1, n, i);
		bool on;
		int j;

		if (changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: said changed at bit "
					"%d\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		if (f[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: trailing guard chunk "
					"changed when setting bit %d\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
		for (j = FLAG_MAX(n) - 1; j > i; --j) {
			on = flag_has(f + 1, n, j);
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: setting bit "
						"%d turned bit %d on\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		on = flag_has(f + 1, n, i);
		if (on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: setting bit %d "
					"turned it on\n", suite_name, __LINE__,
					i - FLAG_START);
			}
			return 1;
		}
		for (j = i - 1; j >= FLAG_START; --j) {
			on = flag_has(f + 1, n, j);
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: setting bit "
						"%d turned bit %d on\n",
						suite_name, __LINE__,
						i - FLAG_START, j - FLAG_START);
				}
				return 1;
			}
		}
		if (f[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: leading guard chunk "
					"changed when setting bit %d\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}

	ok;
}

static int test_negate(void *state)
{
	bitflag f[5] = { 170, 85, 170, 85, 170 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2, i, p[12];

	flag_negate(f + 1, 0);
	eq(f[0], 170);
	eq(f[1], 85);
	eq(f[2], 170);

	flag_wipe(f + 1, n);
	flag_negate(f + 1, n);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		bool on = flag_has(f + 1, n, i);

		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: left bit %d off\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f[n + 1], 170);

	flag_negate(f + 1, n);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		bool on = flag_has(f + 1, n, i);

		if (on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: left bit %d on\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f[n + 1], 170);

	random_bits(f + 1, n, (int)(sizeof(p) / sizeof(p[0])), p);
	flag_negate(f + 1, n);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		int j = 0, k;
		bool on = flag_has(f + 1, n, i);

		while (1) {
			if (j == (int)(sizeof(p) / sizeof(p[0]))) {
				if (!on) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: bit "
							"%d is off; all bits "
							"but these should be "
							"on:", suite_name,
							__LINE__,
							i - FLAG_START);
						for (k = 0; k < (int)(sizeof(p)
								/ sizeof(p[0]));
								++k) {
							(void)printf(" %d",
								p[k]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
				break;
			}
			if (i == p[j]) {
				if (on) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: bit "
							"%d is on; all bits "
							"but these should be "
							"on:", suite_name,
							__LINE__,
							i - FLAG_START);
						for (k = 0; k < (int)(sizeof(p)
								/ sizeof(p[0]));
								++k) {
							(void)printf(" %d",
								p[k]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
				break;
			}
			++j;
		}
	}
	eq(f[n + 1], 170);
	flag_negate(f + 1, n);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		int j = 0, k;
		bool on = flag_has(f + 1, n, i);

		while (1) {
			if (j == (int)(sizeof(p) / sizeof(p[0]))) {
				if (on) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: bit "
							"%d is on; all bits "
							"but these should be "
							"off:", suite_name,
							__LINE__,
							i - FLAG_START);
						for (k = 0; k < (int)(sizeof(p)
								/ sizeof(p[0]));
								++k) {
							(void)printf(" %d",
								p[k]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
				break;
			}
			if (i == p[j]) {
				if (!on) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: bit "
							"%d is off; all bits "
							"but these should be "
							"off:", suite_name,
							__LINE__,
							i - FLAG_START);
						for (k = 0; k < (int)(sizeof(p)
								/ sizeof(p[0]));
								++k) {
							(void)printf(" %d",
								p[k]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
				break;
			}
			++j;
		}
	}
	eq(f[n + 1], 170);

	ok;
}

static int test_next(void *state)
{
	bitflag f[4];
	int n = (int)(sizeof(f) / sizeof(f[0])), result, i, p[8];

	flag_wipe(f, n);
	for (i = FLAG_START; i <= FLAG_MAX(n) + 1; ++i) {
		result = flag_next(f, n, i);
		if (result != FLAG_END) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: got %d rather than "
					"FLAG_END (%d)\n", suite_name, __LINE__,
					result, FLAG_END);
			}
			return 1;
		}
	}

	flag_setall(f, n);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_next(f, n, i);
		if (result != i) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: got %d rather than "
					"%d\n", suite_name, __LINE__, result,
					i);
			}
			return 1;
		}
	}
	result = flag_next(f, n, FLAG_MAX(n));
	eq(result, FLAG_END);
	result = flag_next(f, n, FLAG_MAX(n) + 1);
	eq(result, FLAG_END);

	for (i = 1; i <= (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j;

		for (j = 0; j < 8; ++j) {
			int k, kk, expected, b;

			random_bits(f, n, i, p);
			kk = 0;
			expected = p[0];
			for (k = FLAG_START, kk = 0; k < FLAG_MAX(n) + 2; ++k) {
				result = flag_next(f, n, k);
				if (result != expected) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: got "
							"%d rather than %d "
							"with %d bits on:",
							suite_name, __LINE__,
							result, expected,
							i);
						for (b = 0; b < i; ++i) {
							(void)printf(" %d",
								p[k]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
				if (k == expected) {
					if (kk < i - 1) {
						++kk;
						expected = p[kk];
					} else {
						expected = FLAG_END;
					}
				}
			}

			flag_negate(f, n);
			for (k = FLAG_START; k < FLAG_MAX(n) + 2; ++k) {
				expected = k;
				kk = 0;
				while (kk < i - 1 && expected > p[kk]) {
					++kk;
				}
				while (1) {
					if (expected < p[kk]) {
						break;
					}
					if (expected > p[kk]) {
						assert(kk == i - 1);
						if (expected >= FLAG_MAX(n)) {
							expected = FLAG_END;
						}
						break;
					}
					if (p[kk] == FLAG_MAX(n) - 1) {
						expected = FLAG_END;
						break;
					}
					++expected;
					if (kk == i - 1) {
						break;
					}
					++kk;
				}
				result = flag_next(f, n, k);
				if (result != expected) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: got "
							"%d rather than %d "
							"with %d bits set "
							"before negation:",
							suite_name, __LINE__,
							result, expected, i);
						for (b = 0; b < i; ++i) {
							(void)printf(" %d",
								p[k]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
			}
		}
	}

	ok;
}

static int test_count(void *state)
{
	bitflag f[2];
	int n = (int)(sizeof(f) / sizeof(f[0])), i, p[6], result;

	flag_wipe(f, n);
	result = flag_count(f, n);
	eq(result, 0);

	flag_setall(f, n);
	result = flag_count(f, n);
	eq(result, FLAG_MAX(n) - FLAG_START);

	for (i = 1; i <= (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j, k;

		for (j = 0; j < 8; ++j) {
			random_bits(f, n, i, p);
			result = flag_count(f, n);
			if (result != i) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: got %d but "
						"expected %d with these bits "
						"on:", suite_name, __LINE__,
						result, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d\n", p[k]);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_negate(f, n);
			result = flag_count(f, n);
			if (result != FLAG_MAX(n) - FLAG_START - i) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: got %d but "
						"expected %d with these bits "
						"off:", suite_name, __LINE__,
						result, FLAG_MAX(n)
						- FLAG_START - i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d\n", p[k]);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_is_empty(void *state)
{
	bitflag f[3];
	int n = (int)(sizeof(f) / sizeof(f[0])), i, p[9];
	bool result;

	flag_setall(f, n);
	result = flag_is_empty(f, n);
	require(!result);

	flag_wipe(f, n);
	result = flag_is_empty(f, n);
	require(result);

	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		flag_on(f, n, i);
		result = flag_is_empty(f, n);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is on but "
					"flagged as empty\n", suite_name,
					__LINE__, i - FLAG_START);
			}
			return 1;
		}
		flag_off(f, n, i);
	}

	for (i = 2; i <= (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j, k;

		for (j = 0; j < 8; ++j) {
			random_bits(f, n, i, p);
			result = flag_is_empty(f, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: these bits "
						"are on but flagged as empty:",
						suite_name, __LINE__);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			flag_negate(f, n);
			result = flag_is_empty(f, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: all bits "
						"but these are on but flagged "
						"as empty:", suite_name,
						__LINE__);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_is_full(void *state)
{
	bitflag f[2];
	int n = (int)(sizeof(f) / sizeof(f[0])), i, p[6];
	bool result;

	flag_wipe(f, n);
	result = flag_is_full(f, n);
	require(!result);

	flag_setall(f, n);
	result = flag_is_full(f, n);
	require(result);

	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		flag_off(f, n, i);
		result = flag_is_full(f, n);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is off but "
					"flagged as full\n", suite_name,
					__LINE__, i - FLAG_START);
			}
			return 1;
		}
		flag_on(f, n, i);
	}

	for (i = 2; i <= (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j, k;

		for (j = 0; j < 8; ++j) {
			random_bits(f, n, i, p);
			result = flag_is_full(f, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: all bits but "
						"these are off but flagged as "
						"full:", suite_name,
						__LINE__);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			flag_negate(f, n);
			result = flag_is_full(f, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: all bits but "
						"these are on but flagged as "
						"full:", suite_name,
						__LINE__);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_test(void *state)
{
	bitflag f[3];
	int n = (int)(sizeof(f) / sizeof(f[0])), p[3], i;
	bool result;

	flag_wipe(f, n);
	result = flags_test(f, n, FLAG_END);
	require(!result);
	result = flags_test_all(f, n, FLAG_END);
	require(result);
	result = flags_test(f, n, FLAG_START + 2, FLAG_END);
	require(!result);
	result = flags_test_all(f, n, FLAG_START + 2, FLAG_END);
	require(!result);
	result = flags_test(f, n, FLAG_START, FLAG_START + 7, FLAG_MAX(n) - 1,
		FLAG_END);
	require(!result);
	result = flags_test_all(f, n, FLAG_START, FLAG_START + 7,
		FLAG_MAX(n) - 1, FLAG_END);
	require(!result);

	flag_setall(f, n);
	result = flags_test(f, n, FLAG_END);
	require(!result);
	result = flags_test_all(f, n, FLAG_END);
	require(result);
	result = flags_test(f, n, FLAG_START + 6, FLAG_END);
	require(result);
	result = flags_test_all(f, n, FLAG_START + 6, FLAG_END);
	require(result);
	result = flags_test(f, n, FLAG_START + 3, FLAG_START + 9,
		FLAG_MAX(n) - 5, FLAG_MAX(n) - 2, FLAG_END);
	require(result);
	result = flags_test_all(f, n, FLAG_START + 3, FLAG_START + 9,
		FLAG_MAX(n) - 5, FLAG_MAX(n) - 2, FLAG_END);
	require(result);

	for (i = 0; i < 8; ++i) {
		random_bits(f, n, 2, p);
		result = flags_test(f, n, p[0], p[1], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d and %d are on "
					"but flags_test() with those bits is "
					"false\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[1] - FLAG_START);
			}
			return 1;
		}
		result = flags_test_all(f, n, p[0], p[1], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d and %d are on "
					"but flags_test_all() with those bits "
					"is false\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[1] - FLAG_START);
			}
			return 1;
		}
		require(result);

		(void)flag_off(f, n, p[1]);
		result = flags_test(f, n, p[0], p[1], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is on and bit "
					"%d is off but flags_test() with those "
					"bits is false\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[1] - FLAG_START);
			}
			return 1;
		}
		result = flags_test_all(f, n, p[0], p[1], FLAG_END);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is on and bit "
					"%d is off but flags_test_all() with "
					"those bits is true\n", suite_name,
					__LINE__, p[0] - FLAG_START,
					p[1] - FLAG_START);
			}
			return 1;
		}

		(void)flag_off(f, n, p[0]);
		(void)flag_on(f, n, p[1]);
		result = flags_test(f, n, p[0], p[1], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is off and bit "
					"%d is on but flags_test() with those "
					"bits is false\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[1] - FLAG_START);
			}
			return 1;
		}
		result = flags_test_all(f, n, p[0], p[1], FLAG_END);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is off and bit "
					"%d is on but flags_test_all() with "
					"those bits is true\n", suite_name,
					__LINE__, p[0] - FLAG_START,
					p[1] - FLAG_START);
			}
			return 1;
		}

		(void)flag_on(f, n, p[0]);
		flag_negate(f, n);
		result = flags_test(f, n, p[0], p[1], FLAG_END);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d and %d are "
					"off but flags_test() with those "
					"bits is true\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[1] - FLAG_START);
			}
			return 1;
		}
		result = flags_test_all(f, n, p[0], p[1], FLAG_END);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d and %d are "
					"off but flags_test_all() with "
					"those bits is true\n", suite_name,
					__LINE__, p[0] - FLAG_START,
					p[1] - FLAG_START);
			}
			return 1;
		}

		random_bits(f, n, 3, p);
		result = flags_test(f, n, p[0], p[1], p[2], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d, %d, and %d "
					"are on but flags_test() with those "
					"bits is false\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[1] - FLAG_START,
					p[2] - FLAG_START);
			}
			return 1;
		}
		result = flags_test_all(f, n, p[0], p[1], p[2], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d, %d, and %d "
					"are on but flags_test_all() with "
					"those bits is false\n", suite_name,
					__LINE__, p[0] - FLAG_START,
					p[1] - FLAG_START, p[2] - FLAG_START);
			}
			return 1;
		}

		(void)flag_off(f, n, p[1]);
		result = flags_test(f, n, p[0], p[1], p[2], FLAG_END);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d and %d are "
					"on and bit %d is off but "
					"flags_test() with all those bits is "
					"false\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[2] - FLAG_START,
					p[1] - FLAG_START);
			}
			return 1;
		}
		result = flags_test_all(f, n, p[0], p[1], p[2], FLAG_END);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bits %d and %d are "
					"on and bit %d is off but "
					"flags_test_all() with all those bits "
					"is true\n", suite_name, __LINE__,
					p[0] - FLAG_START, p[2] - FLAG_START,
					p[1] - FLAG_START);
			}
			return 1;
		}
	}

	ok;
}

static int test_clear(void *state)
{
	bitflag f[4] = { 85, 0, 0, 170 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2, i;
	bool changed, on;

	flag_setall(f + 1, n);
	changed = flags_clear(f + 1, n, FLAG_END);
	require(!changed);
	eq(f[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flags_clear() with "
					"empty list turned off bit %d\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f[n + 1], 170);

	changed = flags_clear(f + 1, n, FLAG_START, FLAG_MAX(n) - 1, FLAG_END);
	require(changed);
	eq(f[0], 85);
	on = flag_has(f + 1, n, FLAG_START);
	require(!on);
	for (i = FLAG_START + 1; i < FLAG_MAX(n) - 1; ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flag_clear() with %d "
					"and %d turned off bit %d\n",
					suite_name, __LINE__, 0,
					FLAG_MAX(n) - 1 - FLAG_START, i);
			}
			return 1;
		}
	}
	on = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(!on);
	eq(f[n + 1], 170);

	changed = flags_clear(f + 1, n, FLAG_START, FLAG_MAX(n) - 1, FLAG_END);
	require(!changed);
	eq(f[0], 85);
	on = flag_has(f + 1, n, FLAG_START);
	require(!on);
	for (i = FLAG_START + 1; i < FLAG_MAX(n) - 1; ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flag_clear() with "
					"%d and %d turned off bit %d\n",
					suite_name, __LINE__, 0,
					FLAG_MAX(n) - 1 - FLAG_START, i);
			}
			return 1;
		}
	}
	on = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(!on);
	eq(f[n + 1], 170);

	changed = flags_clear(f + 1, n, FLAG_START + 1, FLAG_END);
	require(changed);
	eq(f[0], 85);
	on = flag_has(f + 1, n, FLAG_START);
	require(!on);
	on = flag_has(f + 1, n, FLAG_START + 1);
	require(!on);
	for (i = FLAG_START + 2; i < FLAG_MAX(n) - 1; ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flag_clear() with %d "
					"turned off bit %d\n", suite_name,
					__LINE__, 1, i);
			}
			return 1;
		}
	}
	on = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(!on);
	eq(f[n + 1], 170);

	changed = flags_clear(f + 1, n, FLAG_START + 1, FLAG_END);
	require(!changed);
	eq(f[0], 85);
	on = flag_has(f + 1, n, FLAG_START);
	require(!on);
	on = flag_has(f + 1, n, FLAG_START + 1);
	require(!on);
	for (i = FLAG_START + 2; i < FLAG_MAX(n) - 1; ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flag_clear() with %d "
					"turned off bit %d\n", suite_name,
					__LINE__, 1, i);
			}
			return 1;
		}
	}
	on = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(!on);
	eq(f[n + 1], 170);

	changed = flags_clear(f + 1, n, FLAG_MAX(n) - 3, FLAG_START, FLAG_END);
	require(changed);
	eq(f[0], 85);
	on = flag_has(f + 1, n, FLAG_START);
	require(!on);
	on = flag_has(f + 1, n, FLAG_START + 1);
	require(!on);
	for (i = FLAG_START + 2; i < FLAG_MAX(n) - 3; ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flag_clear() with %d "
					"and %d turned off bit %d\n",
					suite_name, __LINE__,
					FLAG_MAX(n) - 3 - FLAG_START, 0, i);
			}
			return 1;
		}
	}
	on = flag_has(f + 1, n, FLAG_MAX(n) - 3);
	require(!on);
	on = flag_has(f + 1, n, FLAG_MAX(n) - 2);
	require(on);
	on = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(!on);
	eq(f[n + 1], 170);

	changed = flags_clear(f + 1, n, FLAG_MAX(n) - 3, FLAG_START, FLAG_END);
	require(!changed);
	eq(f[0], 85);
	on = flag_has(f + 1, n, FLAG_START);
	require(!on);
	on = flag_has(f + 1, n, FLAG_START + 1);
	require(!on);
	for (i = FLAG_START + 2; i < FLAG_MAX(n) - 3; ++i) {
		on = flag_has(f + 1, n, i);
		if (!on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flag_clear() with %d "
					"and %d turned off bit %d\n",
					suite_name, __LINE__,
					FLAG_MAX(n) - 3 - FLAG_START, 0, i);
			}
			return 1;
		}
	}
	on = flag_has(f + 1, n, FLAG_MAX(n) - 3);
	require(!on);
	on = flag_has(f + 1, n, FLAG_MAX(n) - 2);
	require(on);
	on = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(!on);
	eq(f[n + 1], 170);

	ok;
}

static int test_set(void *state)
{
	bitflag f[3] = { 170, 85, 170 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2, i;
	bool changed, on;

	flag_wipe(f + 1, n);

	changed = flags_set(f + 1, n, FLAG_END);
	require(!changed);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flags_set() with "
					"empty list turned on bit %d\n",
					suite_name, __LINE__, i);
			}
			return 1;
		}
	}
	eq(f[n + 1], 170);

	changed = flags_set(f + 1, n, FLAG_MAX(n) - 4, FLAG_START, FLAG_END);
	require(changed);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (i == FLAG_START || i == FLAG_MAX(n) - 4) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d left bit %d "
						"off\n", suite_name, __LINE__,
						FLAG_MAX(n) - 4 - FLAG_START, 0,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d turned on bit "
						"%d\n", suite_name, __LINE__,
						FLAG_MAX(n) - 4 - FLAG_START, 0,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f[n + 1], 170);

	changed = flags_set(f + 1, n, FLAG_MAX(n) - 4, FLAG_START, FLAG_END);
	require(!changed);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (i == FLAG_START || i == FLAG_MAX(n) - 4) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d left bit %d "
						"off\n", suite_name, __LINE__,
						FLAG_MAX(n) - 4 - FLAG_START, 0,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d turned on bit "
						"%d\n", suite_name, __LINE__,
						FLAG_MAX(n) - 4 - FLAG_START, 0,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f[n + 1], 170);

	changed = flags_set(f + 1, n, FLAG_START + 1, FLAG_START, FLAG_END);
	require(changed);
	eq(f[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (i == FLAG_START || i == FLAG_START + 1) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d left bit %d "
						"off\n", suite_name, __LINE__,
						1, 0, i - FLAG_START);
				}
				return 1;
			}
		} else if (i == FLAG_MAX(n) - 4) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d turned bit %d "
						"off\n", suite_name, __LINE__,
						1, 0, i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d turned bit %d "
						"on\n", suite_name, __LINE__,
						1, 0, i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f[n + 1], 170);

	changed = flags_set(f + 1, n, FLAG_START + 1, FLAG_START, FLAG_END);
	require(!changed);
	eq(f[0], 170);
	for (i = FLAG_START; i < (int)FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (i == FLAG_START || i == FLAG_START + 1
				|| i == FLAG_MAX(n) - 4) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d turned bit %d "
						"off\n", suite_name, __LINE__,
						1, 0, i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_set() "
						"with %d and %d turned bit %d "
						"on\n", suite_name, __LINE__,
						1, 0, i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f[n + 1], 170);

	ok;
}

static int test_init(void *state)
{
	bitflag f[3] = { 85, 170, 85 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2, i;
	bool on;

	flag_wipe(f + 1, n);
	flags_init(f + 1, n, FLAG_END);
	eq(f[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (on) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: flags_init() with "
					"empty list set bit %d\n", suite_name,
					__LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f[n + 1], 85);

	flags_init(f + 1, n, FLAG_START + 1, FLAG_MAX(n) - 3, FLAG_END);
	eq(f[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (i == FLAG_START + 1 || i == FLAG_MAX(n) - 3) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_init() "
						"with %d and %d left bit %d "
						"off\n", suite_name, __LINE__,
						1, FLAG_MAX(n) - 3 - FLAG_START,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_init() "
						"with %d and %d turned bit %d "
						"on\n", suite_name, __LINE__,
						1, FLAG_MAX(n) - 3 - FLAG_START,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f[n + 1], 85);

	flags_init(f + 1, n, FLAG_START + 7, FLAG_END);
	eq(f[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on = flag_has(f + 1, n, i);
		if (i == FLAG_START + 7) {
			if (!on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flaqs_init() "
						"with %d left bit %d off\n",
						suite_name, __LINE__, 7,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (on) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flags_init() "
						"with %d turned bit %d on\n",
						suite_name, __LINE__, 7,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f[n + 1], 85);

	ok;
}

static int test_copy(void *state)
{
	bitflag f0[7] = { 170, 85, 170, 85, 170, 85, 170 }, f1[5];
	int n = (int)(sizeof(f0) / sizeof(f0[0])) - 2, p[8], i;
	bool on0, on1, result;

	flag_wipe(f1, n);
	flag_copy(f0 + 1, f1, 0);
	eq(f0[0], 170);
	eq(f0[1], 85);
	eq(f0[2], 170);

	flag_copy(f0 + 1, f1, n);
	eq(f0[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on0 = flag_has(f0 + 1, n, i);
		if (on0) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is on\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f0[n + 1], 170);
	result = flag_is_empty(f1, n);
	require(result);

	flag_setall(f1, n);
	flag_copy(f0 + 1, f1, n);
	eq(f0[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		on0 = flag_has(f0 + 1, n, i);
		if (!on0) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: bit %d is off\n",
					suite_name, __LINE__, i - FLAG_START);
			}
			return 1;
		}
	}
	eq(f0[n + 1], 170);
	result = flag_is_full(f1, n);
	require(result);

	for (i = 1; i < (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j;

		for (j = 0; j < 8; ++j) {
			int k, kk;

			random_bits(f1, n, j, p);
			flag_copy(f0 + 1, f1, n);
			eq(f0[0], 170);
			for (k = FLAG_START; k < FLAG_MAX(n); ++k) {
				on0 = flag_has(f0 + 1, n, k);
				on1 = flag_has(f1, n, k);
				if (on0 != on1) {
					if (verbose) {
						showfail();
						(void)printf("    %s:%d: "
							"mismatch at bit %d; "
							"source has %d random "
							"bits on:",
							suite_name, __LINE__,
							k - FLAG_START, i);
						for (kk = 0; kk < i; ++kk) {
							(void)printf(" %d",
								p[kk]
								- FLAG_START);
						}
						(void)printf("\n");
					}
					return 1;
				}
			}
			eq(f0[n + 1], 170);
		}
	}

	ok;
}

static int test_is_equal(void *state)
{
	bitflag f0[5], f1[5];
	int n = (int)(sizeof(f0)/ sizeof(f0[0])), p[10], i;
	bool result;

	flag_wipe(f0, n);
	flag_wipe(f1, n);
	result = flag_is_equal(f0, f1, n);
	require(result);
	result = flag_is_equal(f1, f0, n);
	require(result);

	flag_setall(f1, n);
	result = flag_is_equal(f0, f1, n);
	require(!result);
	result = flag_is_equal(f1, f0, n);
	require(!result);

	for (i = 1; i < (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j;

		for (j = 0; j < 8; ++j) {
			int k, kk;

			random_bits(f1, n, i, p);

			flag_copy(f0, f1, n);
			result = flag_is_equal(f0, f1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: not equal "
						" with %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++i) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			require(result);
			result = flag_is_equal(f1, f0, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: not equal "
						" with %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++i) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			k = randint0(i);
			flag_off(f0, n, p[k]);
			result = flag_is_equal(f0, f1, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: equal with "
						"mismatch at bit %d and %d "
						"bits on in the second set:",
						suite_name, __LINE__,
						p[k] - FLAG_START, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f1, f0, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: equal with "
						"mismatch at bit %d and %d "
						"bits on in the first set:",
						suite_name, __LINE__,
						p[k] - FLAG_START, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_wipe(f0, n);
			result = flag_is_equal(f0, f1, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: equal with "
						"set 1 empty and set 2 having "
						"%d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f1, f0, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: equal with "
						"set 2 empty and set 1 having "
						"%d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_setall(f0, n);
			result = flag_is_equal(f0, f1, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: equal with "
						"set 1 full and set 2 having "
						"%d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f1, f0, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: equal with "
						"set 2 full and set 1 having "
						"%d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_is_inter(void *state)
{
	bitflag f0[3], f1[3];
	int n = (int)(sizeof(f0) / sizeof(f0[0])), p[5], i;
	bool result;

	flag_wipe(f0, n);
	flag_wipe(f1, n);
	result = flag_is_inter(f0, f1, 0);
	require(!result);
	result = flag_is_inter(f0, f1, n);
	require(!result);

	flag_setall(f0, n);
	result = flag_is_inter(f0, f1, 0);
	require(!result);
	result = flag_is_inter(f0, f1, n);
	require(!result);
	result = flag_is_inter(f1, f0, n);
	require(!result);

	flag_setall(f1, n);
	result = flag_is_inter(f0, f1, 0);
	require(!result);
	result = flag_is_inter(f0, f1, n);
	require(result);
	result = flag_is_inter(f1, f0, n);
	require(result);

	for (i = 1; i < (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j;

		for (j = 0; j < 8; ++j) {
			int k, kk;

			random_bits(f0, n, i, p);
			flag_wipe(f1, n);
			result = flag_is_inter(f0, f1, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"between empty set and set "
						"with %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_inter(f1, f0, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"between empty set and set "
						"with %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_setall(f1, n);
			result = flag_is_inter(f0, f1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: no "
						"intersection between full set "
						"and set with %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_inter(f1, f0, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: no "
						"intersection between full set "
						"and set with %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_copy(f1, f0, n);
			flag_negate(f1, n);
			result = flag_is_inter(f0, f1, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"set with %d bits set and its "
						"negation:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_inter(f1, f0, n);
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"set with %d bits set and its "
						"negation:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_copy(f1, f0, n);
			k = randint0(i);
			(void)flag_off(f1, n, p[k]);
			result = flag_is_inter(f0, f1, n);
			if (i == 1 && result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"between empty set and set "
						"with %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			} else if (i != 1 && !result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: no "
						"intersection between sets "
						"with mismatch at bit %d "
						"and %d bits on in the first "
						"set:", suite_name, __LINE__,
						p[k] - FLAG_START, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_inter(f1, f0, n);
			if (i == 1 && result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"between empty set and set "
						"with %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			} else if (i != 1 && !result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: no "
						"intersection between sets "
						"with mismatch at bit %d "
						"and %d bits on in the first "
						"set:", suite_name, __LINE__,
						p[k] - FLAG_START, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d\n",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_is_subset(void *state)
{
	bitflag f0[4], f1[4];
	int n = (int)(sizeof(f0) / sizeof(f0[0])), p[7], i;
	bool result;

	/*
	 * With an empty set as the second set, flag_is_subset() returns true.
	 */
	flag_wipe(f0, n);
	flag_wipe(f1, n);
	result = flag_is_subset(f0, f1, n);
	require(result);
	result = flag_is_subset(f1, f0, n);
	require(result);

	flag_setall(f0, n);
	result = flag_is_subset(f0, f1, n);
	require(result);
	result = flag_is_subset(f1, f0, n);
	require(!result);

	flag_setall(f1, n);
	result = flag_is_subset(f0, f1, n);
	require(result);
	result = flag_is_subset(f1, f0, n);
	require(result);

	i = FLAG_START + randint0(FLAG_MAX(n) - FLAG_START);
	(void)flag_off(f1, n, i);
	result = flag_is_subset(f0, f1, n);
	require(result);
	result = flag_is_subset(f1, f0, n);
	require(!result);

	flags_init(f0, n, FLAG_START, FLAG_START + 2, FLAG_END);
	flags_init(f1, n, FLAG_START + 1, FLAG_START + 3, FLAG_END);
	result = flag_is_subset(f0, f1, n);
	require(!result);
	result = flag_is_subset(f1, f0, n);
	require(!result);

	for (i = 3; i < (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j, k, j0;

		random_bits(f0, n, i, p);

		flag_wipe(f1, n);
		result = flag_is_subset(f0, f1, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: empty set is not a "
					"subset of a set with %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_subset(f1, f0, n);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set with %d bits on "
					"is subset of the empty set: ",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}

		flag_setall(f1, n);
		result = flag_is_subset(f0, f1, n);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set with all bits on "
					"is subset of set with %d bits on: ",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_subset(f1, f0, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set with %d bits on "
					"is not a subset of set with all bits "
					"on: ", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		require(result);

		flag_copy(f1, f0, n);
		j = randint0(i);
		(void)flag_off(f1, n, p[j]);
		result = flag_is_subset(f0, f1, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set which only "
					"differs in having %d bit off is not "
					"a subset of set with %d bits on: ",
					suite_name, __LINE__, p[j] - FLAG_START,
					i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_subset(f1, f0, n);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set which has one "
					"more bit (%d) on is a subset of a "
					"set with %d bits on: ", suite_name,
					__LINE__, p[j] - FLAG_START, i - 1);
				for (k = 0; k < i; ++k) {
					if (k == j) continue;
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}

		j0 = j;
		j = randint0(i);
		(void)flag_off(f1, n, p[j]);
		result = flag_is_subset(f0, f1, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set which only "
					"differs in having two bits (%d and "
					"%d) off is not a subset of set with "
					"%d bits on: ", suite_name, __LINE__,
					p[j0] - FLAG_START, p[j] - FLAG_START,
					i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_subset(f1, f0, n);
		if (result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: set which has two "
					"more bits (%d and %d) on is a subset "
					"of a set with %d bits on: ",
					suite_name, __LINE__,
					p[j0] - FLAG_START, p[j] - FLAG_START,
					i - ((j == j0) ? 1 : 2));
				for (k = 0; k < i; ++k) {
					if (k == j0 || k == j) continue;
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
	}

	ok;
}

static int test_union(void *state)
{
	bitflag f0[4] = { 85, 170, 85, 170 }, f1[4] = { 85, 170, 85, 170 },
		f2[4] = { 85, 170 };
	int n = (int)(sizeof(f0) / sizeof(f0[0])) - 2, p[6], i;
	bool changed, result;

	flag_wipe(f0 + 1, n);
	flag_setall(f1 + 1, n);

	changed = flag_union(f1 + 1, f0 + 1, n);
	require(!changed);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 170);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 170);

	changed = flag_union(f0 + 1, f1 + 1, 0);
	require(!changed);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 170);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 170);

	changed = flag_union(f0 + 1, f1 + 1, n);
	require(changed);
	eq(f0[0], 85);
	result = flag_is_full(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 170);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 170);

	for (i = 1; i <= (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int k;

		random_bits(f2, n, i, p);

		flag_wipe(f0 + 1, n);
		flag_copy(f1 + 1, f2, n);
		changed = flag_union(f1 + 1, f0 + 1, n);
		if (changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union with empty "
					"set changes set with %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union with empty set "
					"corrupted leading guard chunk of set "
					"with %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_equal(f1 + 1, f2, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union with empty set "
					"changed set with %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union with empty set "
					"corrupted trailing guard chunk of set "
					"with %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of source set; "
					"destination set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_empty(f0 + 1, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union changed source "
					"set; destination set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of source set; "
					"destination set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}

		changed = flag_union(f0 + 1, f1 + 1, n);
		if (!changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: empty set not changed "
					"by union with set which has %d bits "
					"on:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of destination; "
					"source set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_equal(f0 + 1, f2, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union of empty set "
					"with a set which has %d bits on is "
					"not equal to the source set:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of destination; "
					"source set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of source set "
					"with %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_equal(f1 + 1, f2, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union changed source "
					"set with %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of source set "
					"with %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}

		flag_copy(f0 + 1, f2, n);
		flag_copy(f1 + 1, f2, n);
		flag_negate(f1 + 1, n);

		changed = flag_union(f0 + 1, f1 + 1, n);
		if (!changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union of set with %d "
					"bits on and its negation did not "
					"change the set:", suite_name, __LINE__,
					i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of destination; "
					"source set was negation of set with "
					"%d bits on:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_full(f0 + 1, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union of set with %d "
					"bits on with its negation is not "
					"full:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of destination; "
					"source set was negation of set with "
					"%d bits on:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of source set "
					"which is negation of set with %d "
					"bits on:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		flag_negate(f1 + 1, n);
		result = flag_is_equal(f1 + 1, f2, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union changed "
					"source set; source set was negation "
					"of a set with %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of source set "
					"which is negation of set with %d "
					"bits on:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}

		flag_copy(f0 + 1, f2, n);
		flag_negate(f0 + 1, n);
		flag_copy(f1 + 1, f2, n);
		changed = flag_union(f0 + 1, f1 + 1, n);
		if (!changed) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union of set with %d "
					"bits on and its negation did not "
					"change the negated set:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of destination; "
					"source set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_full(f0 + 1, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union of set with %d "
					"bits on with its negation is not "
					"full:", suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f0[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of destination; "
					"source set had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[0] != 85) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"leading guard chunk of source set "
					"has %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		result = flag_is_equal(f1 + 1, f2, n);
		if (!result) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union changed "
					"source set; source had %d bits on:",
					suite_name, __LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
		if (f1[n + 1] != 170) {
			if (verbose) {
				showfail();
				(void)printf("    %s:%d: union corrupted "
					"trailing guard chunk of source set "
					"has %d bits on:", suite_name,
					__LINE__, i);
				for (k = 0; k < i; ++k) {
					(void)printf(" %d", p[k] - FLAG_START);
				}
				(void)printf("\n");
			}
			return 1;
		}
	}

	flags_init(f2, n, FLAG_START + 1, FLAG_START + 9, FLAG_END);
	flag_copy(f0 + 1, f2, n);
	flags_init(f1 + 1, n, FLAG_START, FLAG_START + 9, FLAG_END);
	changed = flag_union(f0 + 1, f1 + 1, n);
	require(changed);
	result = flag_is_subset(f0 + 1, f2, n);
	require(result);
	result = flag_is_subset(f0 + 1, f1 + 1, n);
	require(result);
	eq(f0[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_has(f0 + 1, n, i);
		if (i == FLAG_START || i == FLAG_START + 1
				|| i == FLAG_START + 9) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"off\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"on\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f0[n + 1], 170);
	eq(f1[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_has(f1 + 1, n, i);
		if (i == FLAG_START || i == FLAG_START + 9) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"off\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"on\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f1[n + 1], 170);

	flag_copy(f0 + 1, f2, n);
	changed = flag_union(f1 + 1, f0 + 1, n);
	require(changed);
	result = flag_is_subset(f1 + 1, f2, n);
	require(result);
	result = flag_is_subset(f1 + 1, f0 + 1, n);
	require(result);
	eq(f1[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_has(f1 + 1, n, i);
		if (i == FLAG_START || i == FLAG_START + 1
				|| i == FLAG_START + 9) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"off\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"on\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f1[n + 1], 170);
	eq(f0[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_has(f0 + 1, n, i);
		if (i == FLAG_START + 1 || i == FLAG_START + 9) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"off\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: bit %d is "
						"on\n", suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f0[n + 1], 170);

	ok;
}

static int test_inter(void *state)
{
	bitflag f0[8] = { 170, 85, 170, 85, 170, 85, 170, 85 },
		f1[8] = { 170, 85, 170, 85, 170, 85, 170, 85 };
	int n = (int)(sizeof(f0) / sizeof(f0[0])) - 2, p[12], i;
	bool changed, result;

	flag_wipe(f0 + 1, n);
	flag_wipe(f1 + 1, n);

	changed = flag_inter(f0 + 1, f1 + 1, 0);
	require(!changed);
	eq(f0[0], 170);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 170);
	result = flag_is_empty(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	changed = flag_inter(f0 + 1, f1 + 1, n);
	require(!changed);
	eq(f0[0], 170);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 170);
	result = flag_is_empty(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	flag_setall(f1 + 1, n);
	changed = flag_inter(f0 + 1, f1 + 1, 0);
	require(!changed);
	eq(f0[0], 170);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 170);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);
	changed = flag_inter(f0 + 1, f1 + 1, n);
	require(!changed);
	eq(f0[0], 170);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 170);
	result = flag_is_full(f1 + 1, n);
	require(result);

	changed = flag_inter(f1 + 1, f0 + 1, 0);
	require(!changed);
	eq(f1[0], 170);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);
	eq(f0[0], 170);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	changed = flag_inter(f1 + 1, f0 + 1, n);
	require(changed);
	eq(f1[0], 170);
	result = flag_is_empty(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);
	eq(f0[0], 170);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);

	flags_init(f0 + 1, n, FLAG_START + 4, FLAG_START + 10, FLAG_MAX(n) - 7,
		FLAG_MAX(n) - 1, FLAG_END);
	flags_init(f1 + 1, n, FLAG_START + 10, (int)FLAG_MAX(n) - 7,
		FLAG_MAX(n) - 2, FLAG_END);
	changed = flag_inter(f0 + 1, f1 + 1, n);
	require(changed);
	result = flag_is_subset(f1 + 1, f0 + 1, n);
	require(result);
	eq(f0[0], 170);
	eq(f1[0], 170);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_has(f0 + 1, n, i);
		if (i == FLAG_START + 10 || i == FLAG_MAX(n) - 7) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: bit %d is off\n",
						suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: bit %d is on\n",
						suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}

		result = flag_has(f1 + 1, n, i);
		if (i == FLAG_START + 10 || i == FLAG_MAX(n) - 7
				|| i == FLAG_MAX(n) - 2) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: bit %d is off\n",
						suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: bit %d is on\n",
						suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f0[n + 1], 85);
	eq(f1[n + 1], 85);

	for (i = 1; i < (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j, k;

		for (j = 0; j < 8; ++j) {
			random_bits(f1 + 1, n, i, p);

			flag_wipe(f0 + 1, n);
			changed = flag_inter(f0 + 1, f1 + 1, n);
			if (changed) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: empty set "
						"changed when intersected with "
						"set that has %d bits on:",
						suite_name, __LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[0] != 170) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"corrupted destination's "
						"leading guard chunk; source "
						"set has %d bits on:",
						suite_name, __LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_empty(f0 + 1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("   %s:%d: empty set no "
						"longer empty after "
						"intersection with set that "
						"has %d bits on:", suite_name,
						__LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"corrupted destination's "
						"trailing guard chunk; source "
						"set has %d bits on:",
						suite_name, __LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_setall(f0 + 1, n);
			changed = flag_inter(f0 + 1, f1 + 1, n);
			if (!changed) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: full set "
						"did not change when "
						"intersected with set that has "
						"%d bits on:", suite_name,
						__LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[0] != 170) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"corrupted destination's "
						"leading guard chunk; source "
						"set has %d bits on:",
						suite_name, __LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f0 + 1, f1 + 1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("   %s:%d: full set "
						"intersected with a set that "
						"has %d bits on is not equal "
						"to that set:", suite_name,
						__LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: intersection "
						"corrupted destination's "
						"trailing guard chunk; source "
						"set has %d bits on:",
						suite_name, __LINE__, i);
					for (k = 0; k < i; ++k) {
						(void)printf(" %d",
							p[k] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_diff(void *state)
{
	bitflag f0[5] = { 85, 170, 85, 170, 85 },
		f1[5] = { 85, 170, 85, 170, 85 }, f2[3];
	int n = (int)(sizeof(f0) / sizeof(f0[0])) - 2, p[7], i;
	bool changed, result;

	flag_wipe(f0 + 1, n);
	flag_wipe(f1 + 1, n);

	changed = flag_diff(f0 + 1, f1 + 1, 0);
	require(!changed);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 85);
	result = flag_is_empty(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	changed = flag_diff(f0 + 1, f1 + 1, n);
	require(!changed);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 85);
	result = flag_is_empty(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	flag_setall(f1 + 1, n);
	changed = flag_diff(f0 + 1, f1 + 1, 0);
	require(!changed);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	changed = flag_diff(f0 + 1, f1 + 1, n);
	require(!changed);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	changed = flag_diff(f1 + 1, f0 + 1, 0);
	require(!changed);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);

	changed = flag_diff(f1 + 1, f0 + 1, n);
	require(!changed);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);

	flag_setall(f0 + 1, n);
	changed = flag_diff(f0 + 1, f1 + 1, 0);
	require(!changed);
	eq(f0[0], 85);
	result = flag_is_full(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	changed = flag_diff(f0 + 1, f1 + 1, n);
	require(changed);
	eq(f0[0], 85);
	result = flag_is_empty(f0 + 1, n);
	require(result);
	eq(f0[n + 1], 85);
	eq(f1[0], 85);
	result = flag_is_full(f1 + 1, n);
	require(result);
	eq(f1[n + 1], 85);

	flags_init(f0 + 1, n, FLAG_START + 1, FLAG_START + 5, FLAG_START + 6,
		FLAG_MAX(n) - 4, FLAG_MAX(n) - 1, FLAG_END);
	flags_init(f1 + 1, n, FLAG_START + 5, FLAG_MAX(n) - 1, FLAG_END);
	changed = flag_diff(f0 + 1, f1 + 1, n);
	require(changed);
	eq(f0[0], 85);
	for (i = FLAG_START; i < FLAG_MAX(n); ++i) {
		result = flag_has(f0 + 1, n, i);
		if (i == FLAG_START + 1 || i == FLAG_START + 6
				|| i == FLAG_MAX(n) - 4) {
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: bit %d is off\n",
						suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		} else {
			if (result) {
				if (verbose) {
					showfail();
					(void)printf("%s:%d: bit %d is on\n",
						suite_name, __LINE__,
						i - FLAG_START);
				}
				return 1;
			}
		}
	}
	eq(f0[n + 1], 85);

	for (i = 2; i < (int)(sizeof(p) / sizeof(p[0])); ++i) {
		int j;

		for (j = 0; j < 8; ++j) {
			int k, kk;

			random_bits(f2, n, i, p);
			flag_copy(f1 + 1, f2, n);
			flag_wipe(f0 + 1, n);

			changed = flag_diff(f0 + 1, f1 + 1, n);
			if (changed) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"changed empty set when other "
						"set had %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[0] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted leading guard "
						"chunk of destination; other "
						"set had %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_empty(f0 + 1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"changed empty destination "
						"set; other set had %d bits "
						"on:", suite_name, __LINE__, i);
				}
				return 1;
			}
			if (f0[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted trailing guard "
						"chunk of destination; other "
						"set had %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f1[0] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted leading guard "
						"chunk of other set which had "
						"had %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f1 + 1, f2, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"changed other set with %d "
						"bits set:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f1[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted trailing guard "
						"chunk of other set which had "
						"had %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_setall(f0 + 1, n);
			changed = flag_diff(f0 + 1, f1 + 1, n);
			if (!changed) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"did not change full set when "
						"other set had %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			flag_negate(f0 + 1, n);
			if (f0[0] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted leading guard "
						"chunk of destination; other "
						"set had %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f0 + 1, f1 + 1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"between full set and a set "
						"with %d bits on did not yield "
						"the negation of that second "
						"set:", suite_name, __LINE__,
						i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted trailing guard "
						"chunk of destination; other "
						"set had %d bits on:",
						suite_name, __LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f1[0] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted leading guard "
						"chunk of other set which had "
						"had %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_is_equal(f1 + 1, f2, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"changed other set with %d "
						"bits set:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f1[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted trailing guard "
						"chunk of other set which had "
						"had %d bits on:", suite_name,
						__LINE__, i);
					for (kk = 0; kk < i; ++kk) {
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}

			flag_copy(f0 + 1, f2, n);
			k = randint0(i);
			flag_off(f1 + 1, n, p[k]);
			changed = flag_diff(f0 + 1, f1 + 1, n);
			if (!changed) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"did not change set with one "
						"more bit on (%d) than the "
						"other set with these bits on:",
						suite_name, __LINE__,
						p[k] - FLAG_START);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[0] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted leading guard "
						"chunk of destination; other "
						"set had %d bits on:",
						suite_name, __LINE__, i - 1);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_has(f0 + 1, n, p[k]);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"using set with one "
						"more bit on (%d) than the "
						"other set did not leave that "
						"bit on; the bits on in both "
						"sets were:", suite_name,
						__LINE__, p[k] - FLAG_START);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_off(f0 + 1, n, p[k]);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: could not "
						"turn bit %d off when it was "
						"on\n", suite_name, __LINE__,
						p[k] - FLAG_START);
				}
				return 1;
			}
			result = flag_is_empty(f0 + 1, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"using set with one "
						"more bit on (%d) than the "
						"other set left more than that "
						"bit on; the bits on in both "
						"sets were:", suite_name,
						__LINE__, p[k] - FLAG_START);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f0[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted trailing guard "
						"chunk of destination; other "
						"set had %d bits on:",
						suite_name, __LINE__, i - 1);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f1[0] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted leading guard "
						"chunk of other set which had "
						"had %d bits on:", suite_name,
						__LINE__, i - 1);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			result = flag_on(f1 + 1, n, p[k]);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: could not "
						"turn bit %d on when it "
						"should be off\n", suite_name,
						__LINE__, p[k] - FLAG_START);
				}
				return 1;
			}
			result = flag_is_equal(f1 + 1, f2, n);
			if (!result) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"changed other set with %d "
						"bits set:", suite_name,
						__LINE__, i - 1);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
			if (f1[n + 1] != 85) {
				if (verbose) {
					showfail();
					(void)printf("    %s:%d: flag_diff() "
						"corrupted trailing guard "
						"chunk of other set which had "
						"had %d bits on:", suite_name,
						__LINE__, i - 1);
					for (kk = 0; kk < i; ++kk) {
						if (kk == k) continue;
						(void)printf(" %d",
							p[kk] - FLAG_START);
					}
					(void)printf("\n");
				}
				return 1;
			}
		}
	}

	ok;
}

static int test_mask(void *state)
{
	bitflag f[4] = { 170, 85, 170, 85 };
	int n = (int)(sizeof(f) / sizeof(f[0])) - 2;
	bool changed, result;

	flag_wipe(f + 1, n);

	changed = flags_mask(f + 1, n, FLAG_END);
	require(!changed);
	eq(f[0], 170);
	result = flag_is_empty(f + 1, n);
	require(result);
	eq(f[n + 1], 85);

	changed = flags_mask(f + 1, n, FLAG_START + 3, FLAG_START + 7,
		FLAG_MAX(n) - 2, FLAG_END);
	require(!changed);
	eq(f[0], 170);
	result = flag_is_empty(f + 1, n);
	require(result);
	eq(f[n + 1], 85);

	flag_setall(f + 1, n);
	changed = flags_mask(f + 1, n, FLAG_END);
	require(changed);
	eq(f[0], 170);
	result = flag_is_empty(f + 1, n);
	require(result);
	eq(f[n + 1], 85);

	flag_setall(f + 1, n);
	changed = flags_mask(f + 1, n, FLAG_START + 3, FLAG_START + 7,
		FLAG_MAX(n) - 2, FLAG_END);
	require(changed);
	eq(f[0], 170);
	result = flag_has(f + 1, n, FLAG_START + 3);
	require(result);
	result = flag_has(f + 1, n, FLAG_START + 7);
	require(result);
	result = flag_has(f + 1, n, FLAG_MAX(n) - 2);
	require(result);
	changed = flags_clear(f + 1, n, FLAG_START + 3, FLAG_START + 7,
		FLAG_MAX(n) - 2, FLAG_END);
	require(changed);
	result = flag_is_empty(f + 1, n);
	require(result);
	eq(f[n + 1], 85);

	flags_init(f + 1, n, FLAG_START + 2, FLAG_START + 6, FLAG_MAX(n) - 4,
		FLAG_MAX(n) - 1, FLAG_END);
	changed = flags_mask(f + 1, n, FLAG_START + 6, FLAG_MAX(n) - 1,
		FLAG_END);
	require(changed);
	eq(f[0], 170);
	result = flag_has(f + 1, n, FLAG_START + 6);
	require(result);
	result = flag_has(f + 1, n, FLAG_MAX(n) - 1);
	require(result);
	changed = flags_clear(f + 1, n, FLAG_START + 6, FLAG_MAX(n) - 1,
		FLAG_END);
	require(changed);
	result = flag_is_empty(f + 1, n);
	require(result);
	eq(f[n + 1], 85);

	ok;
}

const char *suite_name = "z-bitflag/bitflag";
struct test tests[] = {
	{ "flag_has", test_has },
	{ "flag_wipe", test_wipe },
	{ "flag_setall", test_setall },
	{ "flag_on", test_on },
	{ "flag_off", test_off },
	{ "flag_negate", test_negate },
	{ "flag_next", test_next },
	{ "flag_count", test_count },
	{ "flag_is_empty", test_is_empty },
	{ "flag_is_full", test_is_full },
	{ "flags_test and flags_test_all", test_test },
	{ "flags_clear", test_clear },
	{ "flags_set", test_set },
	{ "flags_init", test_init },
	{ "flag_copy", test_copy },
	{ "flag_is_equal", test_is_equal },
	{ "flag_is_inter", test_is_inter },
	{ "flag_is_subset", test_is_subset },
	{ "flag_union", test_union },
	{ "flag_inter", test_inter },
	{ "flag_diff", test_diff },
	{ "flags_mask", test_mask },
	{ NULL, NULL },
};
