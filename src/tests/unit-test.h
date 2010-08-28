/* unit-test.h */

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdio.h>

#define nosetup int setup(void **data) { return 0; }
#define noteardown int teardown(void *data) { return 0; }

static const char *suite_name;

struct test {
	const char *name;
	int (*func)(void *data);
};

static struct test tests[];

static int setup(void **data);
static int teardown(void *data);

int main(void) {
	void *state;
	int i;
	int passed = 0;
	int total = 0;
	int res;

	printf("%s: starting\n", suite_name);
	fflush(stdout);

	if (setup(&state)) {
		printf("%s: setup failed\n", suite_name);
		return 1;
	}

	for (i = 0; tests[i].name; i++) {
		printf("%s: %s... ", suite_name, tests[i].name);
		fflush(stdout);
		res = tests[i].func(state);
		if (res) {
			printf("\033[01;31mFailed\033[00m\n");
			fflush(stdout);
		} else {
			printf("\033[01;32mPassed\033[00m\n");
			fflush(stdout);
			passed++;
		}
		total++;
	}

	if (teardown(&state)) {
		printf("%s: teardown failed\n", suite_name);
		return 1;
	}

	printf("%s: done: %d %d\n", suite_name, passed, total);
}

#define require(x) \
	if (!(x)) { \
		printf("%s:%d: requirement '%s' failed\n", \
		       suite_name, __LINE__, #x); \
		return 1; \
	}

#endif /* !UNIT_TEST_H */
