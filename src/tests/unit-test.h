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
static int verbose = 0;

int main(int argc, char *argv[]) {
	void *state;
	int i;
	int passed = 0;
	int total = 0;
	int res;

	if (argc > 1 && !strcmp(argv[1], "-v")) {
		verbose = 1;
	}

	if (verbose) {
		printf("%s: starting...\n", suite_name);
		fflush(stdout);
	}

	if (setup(&state)) {
		printf("%s: setup failed\n", suite_name);
		return 1;
	}

	for (i = 0; tests[i].name; i++) {
		if (verbose) printf("%s: %s... ", suite_name, tests[i].name);
		fflush(stdout);
		res = tests[i].func(state);
		if (res) {
			if (verbose) {
				printf("\033[01;31mFailed\033[00m\n");
				fflush(stdout);
			}
		} else {
			if (verbose) {
				printf("\033[01;32mPassed\033[00m\n");
				fflush(stdout);
			}
			passed++;
		}
		total++;
	}

	if (teardown(state)) {
		printf("%s: teardown failed\n", suite_name);
		return 1;
	}

	printf("%s: done: %d %d\n", suite_name, passed, total);
	return 0;
}

#define require(x) \
	if (!(x)) { \
		if (verbose) \
			printf("%s:%d: requirement '%s' failed\n", \
		           suite_name, __LINE__, #x); \
		return 1; \
	}

#define requireeq(x,y) \
	if ((x) != (y)) { \
		if (verbose) { \
			printf("%s:%d: requirement '%s' == '%s' failed\n", suite_name, \
		           __LINE__, #x, #y); \
			printf("  %s: %llx\n", #x, (long long)x); \
			printf("  %s: %llx\n", #y, (long long)y); \
		} \
		return 1; \
	}

#define ok return 0

#endif /* !UNIT_TEST_H */
