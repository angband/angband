/* z-quark/quark.c */

#include "unit-test.h"
#include "z-quark.h"

static int setup(void **state) {
	quarks_init();
}

static int teardown(void *state) {
	quarks_free();
}

static int test_alloc(void *state) {
	quark_t q1 = quark_add("0-foo");
	quark_t q2 = quark_add("0-bar");
	quark_t q3 = quark_add("0-baz");

	require(quark_str(q1));
	require(quark_str(q2));
	require(quark_str(q3));

	require(!strcmp(quark_str(q1), "0-foo"));
	require(!strcmp(quark_str(q2), "0-bar"));
	require(!strcmp(quark_str(q3), "0-baz"));

	return 0;
}

static int test_dedup(void *state) {
	quark_t q1 = quark_add("1-foo");
	quark_t q2 = quark_add("1-foo");
	quark_t q3 = quark_add("1-bar");

	require(quark_str(q1));
	require(quark_str(q2));
	require(quark_str(q3));

	require(q1 == q2);
	require(quark_str(q1) == quark_str(q2));
	require(q1 != q3);
	require(quark_str(q1) != quark_str(q3));

	return 0;
}

static const char *suite_name = "z-quark/quark";
static struct test tests[] = {
	{ "alloc", test_alloc },
	{ "dedup", test_dedup },
	{ NULL, NULL }
};
