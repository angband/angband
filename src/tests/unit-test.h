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

	if (setup(&state)) {
		printf("%s: setup failed\n", suite_name);
		return 1;
	}

	for (i = 0; tests[i].name; i++) {
		int res = tests[i].func(state);
		if (res) {
			printf("%s: test failed: %s\n", suite_name, tests[i].name);
		} else {
			printf("%s: test passed: %s\n", suite_name, tests[i].name);
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
