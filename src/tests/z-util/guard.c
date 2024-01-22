/* z-util/guard.c */
/* Exercise add_guardi(), sub_guardi(), add_guardi16(), and sub_guardi16(). */

#include "unit-test.h"
#include "z-util.h"

NOSETUP
NOTEARDOWN

static int test_add_guardi(void *state)
{
	eq(add_guardi(0, 0), 0);
	eq(add_guardi(0, 1), 1);
	eq(add_guardi(1, 0), 1);
	eq(add_guardi(0, -1), -1);
	eq(add_guardi(-1, 0), -1);
	eq(add_guardi(3, 5), 8);
	eq(add_guardi(5, 3), 8);
	eq(add_guardi(-4, -7), -11);
	eq(add_guardi(-7, -4), -11);
	eq(add_guardi(6, -2), 4);
	eq(add_guardi(-2, 6), 4);
	eq(add_guardi(-8, 7), -1);
	eq(add_guardi(7, -8), -1);
	eq(add_guardi(INT_MAX, 0), INT_MAX);
	eq(add_guardi(0, INT_MAX), INT_MAX);
	eq(add_guardi(INT_MAX, -5), INT_MAX - 5);
	eq(add_guardi(-5, INT_MAX), INT_MAX - 5);
	eq(add_guardi(INT_MIN, 0), INT_MIN);
	eq(add_guardi(0, INT_MIN), INT_MIN);
	eq(add_guardi(INT_MIN, 7), INT_MIN + 7);
	eq(add_guardi(7, INT_MIN), INT_MIN + 7);
	/* Cases that would overflow. */
	eq(add_guardi(INT_MAX, 1), INT_MAX);
	eq(add_guardi(1, INT_MAX), INT_MAX);
	eq(add_guardi(INT_MAX - 3, 5), INT_MAX);
	eq(add_guardi(5, INT_MAX - 3), INT_MAX);
	eq(add_guardi(INT_MAX, INT_MAX), INT_MAX);
	eq(add_guardi(INT_MIN, -1), INT_MIN);
	eq(add_guardi(-1, INT_MIN), INT_MIN);
	eq(add_guardi(INT_MIN + 6, -8), INT_MIN);
	eq(add_guardi(-8, INT_MIN + 6), INT_MIN);
	eq(add_guardi(INT_MIN, INT_MIN), INT_MIN);
	ok;
}

static int test_sub_guardi(void *state)
{
	eq(sub_guardi(0, 0), 0);
	eq(sub_guardi(0, 1), -1);
	eq(sub_guardi(1, 0), 1);
	eq(sub_guardi(0, -1), 1);
	eq(sub_guardi(-1, 0), -1);
	eq(sub_guardi(3, 5), -2);
	eq(sub_guardi(5, 3), 2);
	eq(sub_guardi(-4, -7), 3);
	eq(sub_guardi(-7, -4), -3);
	eq(sub_guardi(6, -2), 8);
	eq(sub_guardi(-2, 6), -8);
	eq(sub_guardi(-8, 7), -15);
	eq(sub_guardi(7, -8), 15);
	eq(sub_guardi(9, 9), 0);
	eq(sub_guardi(-10, -10), 0);
	eq(sub_guardi(INT_MAX, 0), INT_MAX);
	eq(sub_guardi(0, INT_MAX - 1), -(INT_MAX - 1));
	eq(sub_guardi(INT_MAX, 5), INT_MAX - 5);
	eq(sub_guardi(5, INT_MAX), -(INT_MAX - 5));
	eq(sub_guardi(INT_MIN, 0), INT_MIN);
	eq(sub_guardi(0, INT_MIN + 2), -(INT_MIN + 2));
	eq(sub_guardi(INT_MIN, -7), INT_MIN + 7);
	eq(sub_guardi(-7, INT_MIN), -(INT_MIN + 7));
	eq(sub_guardi(INT_MAX, INT_MAX), 0);
	eq(sub_guardi(INT_MIN, INT_MIN), 0);
	/* Cases that would overflow. */
	eq(sub_guardi(INT_MAX, -1), INT_MAX);
	eq(sub_guardi(INT_MAX - 3, -5), INT_MAX);
	eq(sub_guardi(INT_MAX, INT_MIN), INT_MAX);
	eq(sub_guardi(INT_MIN, 3), INT_MIN);
	eq(sub_guardi(INT_MIN + 6, 8), INT_MIN);
	eq(sub_guardi(INT_MIN, INT_MAX), INT_MIN);
	ok;
}

static int test_add_guardi16(void *state)
{
	eq(add_guardi16(0, 0), 0);
	eq(add_guardi16(0, 1), 1);
	eq(add_guardi16(1, 0), 1);
	eq(add_guardi16(0, -1), -1);
	eq(add_guardi16(-1, 0), -1);
	eq(add_guardi16(3, 5), 8);
	eq(add_guardi16(5, 3), 8);
	eq(add_guardi16(-4, -7), -11);
	eq(add_guardi16(-7, -4), -11);
	eq(add_guardi16(6, -2), 4);
	eq(add_guardi16(-2, 6), 4);
	eq(add_guardi16(-8, 7), -1);
	eq(add_guardi16(7, -8), -1);
	eq(add_guardi16(32767, 0), 32767);
	eq(add_guardi16(0, 32767), 32767);
	eq(add_guardi16(32767, -5), 32762);
	eq(add_guardi16(-5, 32767), 32762);
	eq(add_guardi16(-32768, 0), -32768);
	eq(add_guardi16(0, -32768), -32768);
	eq(add_guardi16(-32768, 7), -32761);
	eq(add_guardi16(7, -32768), -32761);
	/* Cases that would overflow. */
	eq(add_guardi16(32767, 1), 32767);
	eq(add_guardi16(1, 32767), 32767);
	eq(add_guardi16(32764, 5), 32767);
	eq(add_guardi16(5, 32764), 32767);
	eq(add_guardi16(32767, 32767), 32767);
	eq(add_guardi16(-32768, -1), -32768);
	eq(add_guardi16(-1, -32768), -32768);
	eq(add_guardi16(-32762, -8), -32768);
	eq(add_guardi16(-8, -32762), -32768);
	eq(add_guardi16(-32768, -32768), -32768);
	ok;
}

static int test_sub_guardi16(void *state)
{
	eq(sub_guardi16(0, 0), 0);
	eq(sub_guardi16(0, 1), -1);
	eq(sub_guardi16(1, 0), 1);
	eq(sub_guardi16(0, -1), 1);
	eq(sub_guardi16(-1, 0), -1);
	eq(sub_guardi16(3, 5), -2);
	eq(sub_guardi16(5, 3), 2);
	eq(sub_guardi16(-4, -7), 3);
	eq(sub_guardi16(-7, -4), -3);
	eq(sub_guardi16(6, -2), 8);
	eq(sub_guardi16(-2, 6), -8);
	eq(sub_guardi16(-8, 7), -15);
	eq(sub_guardi16(7, -8), 15);
	eq(sub_guardi16(9, 9), 0);
	eq(sub_guardi16(-10, -10), 0);
	eq(sub_guardi16(32767, 0), 32767);
	eq(sub_guardi16(0, 32766), -32766);
	eq(sub_guardi16(32767, 5), 32762);
	eq(sub_guardi16(5, 32767), -32762);
	eq(sub_guardi16(-32768, 0), -32768);
	eq(sub_guardi16(0, -32766), 32766);
	eq(sub_guardi16(-32768, -7), -32761);
	eq(sub_guardi16(-7, -32768), 32761);
	eq(sub_guardi16(32767, 32767), 0);
	eq(sub_guardi16(-32768, -32768), 0);
	/* Cases that would overflow. */
	eq(sub_guardi16(32767, -1), 32767);
	eq(sub_guardi16(32764, -5), 32767);
	eq(sub_guardi16(32767, -32768), 32767);
	eq(sub_guardi16(-32768, 3), -32768);
	eq(sub_guardi16(-32762, 8), -32768);
	eq(sub_guardi16(-32768, 32767), -32768);
	ok;
}

const char *suite_name = "z-util/guard";
struct test tests[] = {
	{ "add_guardi", test_add_guardi },
	{ "sub_guardi", test_sub_guardi },
	{ "add_guardi16", test_add_guardi16 },
	{ "sub_guardi16", test_sub_guardi16 },
	{ NULL, NULL }
};
