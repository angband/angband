/* z-util/rational.c */

#include "unit-test.h"
#include "z-util.h"

NOSETUP
NOTEARDOWN

static int test_rational_construct(void *state) {
	struct my_rational result = my_rational_construct(0, 1);

	eq(result.n, 0);
	eq(result.d, 1);
	result = my_rational_construct(1, 1);
	eq(result.n, 1);
	eq(result.d, 1);
	result = my_rational_construct(105, 441);
	eq(result.n, 5);
	eq(result.d, 21);
	ok;
}

static int test_rational_to_uint(void *state) {
	struct my_rational arg;
	unsigned int result, remainder;

	arg = my_rational_construct(0, 1);
	result = my_rational_to_uint(&arg, 0, NULL);
	eq(result, 0);
	result = my_rational_to_uint(&arg, 0, &remainder);
	eq(result, 0);
	eq(remainder, 0);
	result = my_rational_to_uint(&arg, 1, NULL);
	eq(result, 0);
	result = my_rational_to_uint(&arg, 1, &remainder);
	eq(result, 0);
	eq(remainder, 0);
	result = my_rational_to_uint(&arg, 100, NULL);
	eq(result, 0);
	result = my_rational_to_uint(&arg, 100, &remainder);
	eq(result, 0);
	eq(remainder, 0);

	arg = my_rational_construct(9, 5);
	result = my_rational_to_uint(&arg, 1, NULL);
	eq(result, 1);
	result = my_rational_to_uint(&arg, 1, &remainder);
	eq(result, 1);
	eq(remainder, 4);
	result = my_rational_to_uint(&arg, 4, NULL);
	eq(result, 7);
	result = my_rational_to_uint(&arg, 4, &remainder);
	eq(result, 7);
	eq(remainder, 1);
	result = my_rational_to_uint(&arg, 17, NULL);
	eq(result, 30);
	result = my_rational_to_uint(&arg, 17, &remainder);
	eq(result, 30);
	eq(remainder, 3);

	arg = my_rational_construct(UINT_MAX - 5, 8);
	result = my_rational_to_uint(&arg, 15, NULL);
	eq(result, UINT_MAX);
	result = my_rational_to_uint(&arg, 15, &remainder);
	eq(result, UINT_MAX);
	eq(remainder, 0);
	arg = my_rational_construct(UINT_MAX - 7, UINT_MAX - 6);
	result = my_rational_to_uint(&arg, 24, NULL);
	eq(result, 23);
	result = my_rational_to_uint(&arg, 24, &remainder);
	eq(result, 23);
	eq(remainder, UINT_MAX - 30);
	arg = my_rational_construct(UINT_MAX, UINT_MAX - 1);
	result = my_rational_to_uint(&arg, UINT_MAX, NULL);
	eq(result, UINT_MAX);
	result = my_rational_to_uint(&arg, UINT_MAX, &remainder);
	eq(result, UINT_MAX);
	eq(remainder, 0);
	arg = my_rational_construct(3, 8);
	result = my_rational_to_uint(&arg, UINT_MAX, NULL);
	eq(result, (3U * (1U << (CHAR_BIT * sizeof(unsigned int) - 3)) - 1U));
	result = my_rational_to_uint(&arg, UINT_MAX, &remainder);
	eq(result, (3U * (1U << (CHAR_BIT * sizeof(unsigned int) - 3)) - 1U));
	eq(remainder, 5);

	ok;
}

static int test_rational_product(void *state) {
	struct my_rational arg1, arg2, result;

	arg1 = my_rational_construct(0, 5);
	arg2 = my_rational_construct(1, 1);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, 0);
	result = my_rational_product(&arg2, &arg1);
	eq(result.n, 0);

	arg1 = my_rational_construct(1, 9);
	arg2 = my_rational_construct(2, 7);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, 2);
	eq(result.d, 63);
	result = my_rational_product(&arg2, &arg1);
	eq(result.n, 2);
	eq(result.d, 63);

	arg1 = my_rational_construct(39, 64);
	arg2 = my_rational_construct(7, 13);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, 21);
	eq(result.d, 64);
	result = my_rational_product(&arg2, &arg1);
	eq(result.n, 21);
	eq(result.d, 64);

	arg1 = my_rational_construct(5, 4);
	arg2 = my_rational_construct(6, 35);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, 3);
	eq(result.d, 14);
	result = my_rational_product(&arg2, &arg1);
	eq(result.n, 3);
	eq(result.d, 14);

	arg1 = my_rational_construct(UINT_MAX - 1, UINT_MAX);
	arg2 = my_rational_construct(UINT_MAX - 1, UINT_MAX - 2);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, 1);
	eq(result.d, 1);
	result = my_rational_product(&arg2, &arg1);
	eq(result.n, 1);
	eq(result.d, 1);

	arg1 = my_rational_construct(1, UINT_MAX);
	arg2 = my_rational_construct(1, UINT_MAX - 1);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, 0);
	eq(result.d, 1);

	arg1 = my_rational_construct(UINT_MAX, 3);
	arg2 = my_rational_construct(UINT_MAX - 1, 7);
	result = my_rational_product(&arg1, &arg2);
	eq(result.n, UINT_MAX);
	eq(result.d, 1);

	ok;
}

static int test_rational_sum(void *state) {
	struct my_rational arg1, arg2, result;

	arg1 = my_rational_construct(0, 7);
	arg2 = my_rational_construct(1, 1);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, 1);
	eq(result.d, 1);
	result = my_rational_sum(&arg2, &arg1);
	eq(result.n, 1);
	eq(result.d, 1);

	arg1 = my_rational_construct(9, 17);
	arg2 = my_rational_construct(3, 5);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, 96);
	eq(result.d, 85);
	result = my_rational_sum(&arg2, &arg1);
	eq(result.n, 96);
	eq(result.d, 85);

	arg1 = my_rational_construct(3, 8);
	arg2 = my_rational_construct(5, 4);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, 13);
	eq(result.d, 8);
	result = my_rational_sum(&arg2, &arg1);
	eq(result.n, 13);
	eq(result.d, 8);

	arg1 = my_rational_construct(3, 14);
	arg2 = my_rational_construct(7, 30);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, 47);
	eq(result.d, 105);
	result = my_rational_sum(&arg2, &arg1);
	eq(result.n, 47);
	eq(result.d, 105);

	arg1 = my_rational_construct(UINT_MAX - 1, UINT_MAX);
	arg2 = my_rational_construct(UINT_MAX - 2, UINT_MAX);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, UINT_MAX - 2);
	eq(result.d, UINT_MAX / 2);

	arg1 = my_rational_construct(1, UINT_MAX);
	arg2 = my_rational_construct(1, UINT_MAX - 1);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, 2);
	eq(result.d, UINT_MAX);

	arg1 = my_rational_construct(UINT_MAX - 1, 1);
	arg2 = my_rational_construct(17, 8);
	result = my_rational_sum(&arg1, &arg2);
	eq(result.n, UINT_MAX);
	eq(result.d, 1);

	ok;
}

const char *suite_name = "z-util/rational";
struct test tests[] = {
	{ "rational_construct", test_rational_construct },
	{ "rational_to_uint", test_rational_to_uint },
	{ "rational_product", test_rational_product },
	{ "rational_sum", test_rational_sum },
	{ NULL, NULL }
};
