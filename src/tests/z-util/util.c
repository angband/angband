/* z-quark/quark.c */

#include "unit-test.h"
#include "z-quark.h"

int setup_tests(void **state) {
	return 0;
}

int teardown_tests(void *state) {
	return 0;
}

int test_alloc(void *state) {
	char buffer[64];

	/* Check it functions at all */
	strcpy(buffer, "1234567890");
	utf8_clipto(buffer, 5);
	require(strcmp(buffer, "12345") == 0);

	/* Extremely low input */
	strcpy(buffer, "Test");
	utf8_clipto(buffer, 0);
	require(buffer[0] == 0);

	/* Overly high input */
	strcpy(buffer, "Test");
	utf8_clipto(buffer, 10);
	require(strcmp(buffer, "Test") == 0);

	/* Non-ASCII clipping */
	strcpy(buffer, "Lómin");
	utf8_clipto(buffer, 2);
	require(strcmp(buffer, "Ló") == 0);

	strcpy(buffer, "åéïø");
	utf8_clipto(buffer, 3);
	require(strcmp(buffer, "åéï") == 0);

	ok;
}

const char *suite_name = "z-util/util";
struct test tests[] = {
	{ "utf8_clipto", test_alloc },
	{ NULL, NULL }
};
