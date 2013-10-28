/* unit-test.c
 *
 * Framework for unit/regression testing harness
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "unit-test-types.h"
#include "z-util.h"

int verbose = 0;

extern const char *suite_name;
extern struct test tests[];
extern int setup_tests(void **data);
extern int teardown_tests(void **data);

int main(int argc, char *argv[]) {
	void *state;
	int i;
	int passed = 0;
	int total = 0;

	char *s = getenv("VERBOSE");
	if (s && s[0]) {
		verbose = 1;
	} else if (argc > 1 && !strncmp(argv[1], "-v", 2)) {
		verbose = 1;
	}

	if (verbose) {
		printf("%s: starting...\n", suite_name);
		fflush(stdout);
	}

	if (setup_tests(&state)) {
		printf("ERROR: %s setup failed\n", suite_name);
		return 1;
	}

	for (i = 0; tests[i].name; i++) {
		if (verbose) printf("  %-16s  ", tests[i].name);
		fflush(stdout);
		if (tests[i].func(state) == 0) passed++;
		total++;
		fflush(stdout);
	}

	if (teardown_tests(state)) {
		printf("ERROR: %s teardown failed\n", suite_name);
		return 1;
	}

	printf("%s finished: %d/%d passed\n", suite_name, passed, total);
	return 0;
}

int showpass(void) {
	if (verbose) printf("\033[01;32mPassed\033[00m\n");
	return 0;
}
int showfail(void) {
	if (verbose) printf("\033[01;31mFailed\033[00m\n");
	return 1;
}
