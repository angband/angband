/* unit-test.h */

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdlib.h>
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

	if (setup(&state)) {
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

	if (teardown(state)) {
		printf("ERROR: %s teardown failed\n", suite_name);
		return 1;
	}

	printf("  %s finished: %d/%d passed\n", suite_name, passed, total);
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

#define ok return showpass();

#define eq(x,y) \
	if ((x) != (y)) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' == '%s' failed\n", suite_name, \
		           __LINE__, #x, #y); \
			printf("      %s: 0x%016lld\n", #x, (long long)x); \
			printf("      %s: 0x%016lld\n", #y, (long long)y); \
		} \
		return 1; \
	}

#define require(x) \
	if (!(x)) { \
		if (verbose) \
			showfail(); \
			printf("    %s:%d: requirement '%s' failed\n", \
		           suite_name, __LINE__, #x); \
		return 1; \
	}

#if __WORDSIZE == 64
#define ptreq(x,y) \
	if ((x) != (y)) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' == '%s' failed\n", suite_name, \
		           __LINE__, #x, #y); \
			printf("      %s: 0x%016llx\n", #x, (unsigned long long)(x)); \
			printf("      %s: 0x%016llx\n", #y, (unsigned long long)(y)); \
		} \
		return 1; \
	}

#else
#define ptreq(x,y) \
	if ((x) != (y)) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' == '%s' failed\n", suite_name, \
		           __LINE__, #x, #y); \
			printf("      %s: 0x%08lx\n", #x, (unsigned long)(x)); \
			printf("      %s: 0x%08lx\n", #y, (unsigned long)(y)); \
		} \
		return 1; \
	}

#endif

#endif /* !UNIT_TEST_H */
