/* trivial/trivial.c */

#include "unit-test.h"

nosetup;
noteardown;

static int test_empty(void *state) {
	return 0;
}

static int test_require(void *state) {
	require(1);
	return 0;
}

static const char *suite_name = "trivial/trivial";
static struct test tests[] = {
	{ "empty", test_empty },
	{ "require", test_require },
	{ NULL, NULL },
};
