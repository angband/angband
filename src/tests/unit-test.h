/* unit-test.h */

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include "unit-test-types.h"
#include "z-util.h"

#define TEST

extern int verbose;
extern int forcepath;

extern int showpass(void);
extern int showfail(void);

/* Forward declaration for string provided by the test case but expected by
 * unit-test.c and the macros declared here.
 */
extern const char *suite_name;

/* Forward declaration for the test case array provided by the test case but
 * expected by unit-test.c.
 */
extern struct test tests[];

/* Provided by the test case and called by unit-test.c.  If a test case
 * does not need setup or teardown use the NOSETUP or NOTEARDOWN macros
 * in the test case to provide the functions unit-test.c wants.
 */
extern int setup_tests(void **data);
extern int teardown_tests(void *data);
#define NOSETUP int setup_tests(void **data) { return 0; }
#define NOTEARDOWN int teardown_tests(void *data) { return 0; }

#define ok return showpass();

#define eq(x,y) \
	if ((x) != (y)) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' == '%s' failed\n", suite_name, \
		           __LINE__, #x, #y); \
			printf("      %s: %16lld\n", #x, (long long)(x)); \
			printf("      %s: %16lld\n", #y, (long long)(y)); \
		} \
		return 1; \
	}

#define noteq(x,y) \
	if ((x) == (y)) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' != '%s' failed\n", suite_name, \
		           __LINE__, #x, #y); \
			printf("      %s: %16lld\n", #x, (long long)(x)); \
			printf("      %s: %16lld\n", #y, (long long)(y)); \
		} \
		return 1; \
	}

#define require(x) \
	do { \
		if (!(x)) { \
			if (verbose) { \
				showfail(); \
				printf("    %s:%d: requirement '%s' failed\n", \
			           suite_name, __LINE__, #x); \
			} \
			return 1; \
		} \
	} while (0)

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

#define null(x) \
	if ((x) != 0) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' == NULL failed\n", suite_name, \
		           __LINE__, #x); \
			printf("      %s: 0x%016llx\n", #x, (unsigned long long)(x)); \
		} \
		return 1; \
	}

#define notnull(x) \
	if ((x) == 0) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' != NULL failed\n", suite_name, \
		           __LINE__, #x); \
			printf("      %s: 0x%016llx\n", #x, (unsigned long long)(x)); \
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
			printf("      %s: %p\n", #x, (void*)(x)); \
			printf("      %s: %p\n", #y, (void*)(y)); \
		} \
		return 1; \
	}

#define null(x) \
	if ((x) != 0) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' == NULL failed\n", suite_name, \
		           __LINE__, #x); \
			printf("      %s: %p\n", #x, (void*)(x)); \
		} \
		return 1; \
	}

#define notnull(x) \
	if ((x) == 0) { \
		if (verbose) { \
			showfail(); \
			printf("    %s:%d: requirement '%s' != NULL failed\n", suite_name, \
		           __LINE__, #x); \
			printf("      %s: %p\n", #x, (void*)(x)); \
		} \
		return 1; \
	}

#endif

/*
 * Test cases that use set_file_paths() will use TEST_DEFAULT_PATH for each
 * of the path arguments to init_file_paths() if TEST_DEFAULT_PATH is set
 * and, when the test is run, it was not run with the -f command line option
 * and the FORCE_PATH environment variable is not set or is empty.  If
 * TEST_DEFAULT_PATH is not set or the test case is run with the -f command
 * line option or the FORCE_PATH environment variable is set to a non-empty
 * string, the paths passed to init_file_paths() will be the same as the game
 * uses:  DEFAULT_CONFIG_PATH, DEFAULT_LIB_PATH, and DEFAULT_DATA_PATH.
 *
 * If TEST_OVERRIDE_PATHS is set and TEST_DEFAULT_PATH is not set, use a path
 * which assumes that the test case is run with a working directory set to the
 * top level directory of a distribution.  That is typically useful for builds
 * with the Windows front end or Unix builds where the data files will be
 * installed outside of the distribution directory.
 */
#ifdef TEST_OVERRIDE_PATHS
#ifndef TEST_DEFAULT_PATH
#define TEST_DEFAULT_PATH "." PATH_SEP "lib" PATH_SEP
#endif /* !TEST_DEFAULT_PATH */
#endif /* TEST_OVERRIDE_PATHS */

#endif /* !UNIT_TEST_H */
