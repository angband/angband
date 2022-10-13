/* z-file/filename-index.c */

#include "unit-test.h"
#include "z-file.h"

NOSETUP
NOTEARDOWN

static int test_empty(void *state)
{
	eq(path_filename_index(""), 0);
	ok;
}

static int test_no_separator(void *state)
{
	eq(path_filename_index("abcde"), 0);
	ok;
}

static int test_separator(void *state)
{
	struct { const char *path; size_t expected; } cases[] = {
#ifdef WINDOWS
		{ "C:\\", 3 },
		{ "C:\\Windows", 3 },
		{ "C:\\Windows\\temp", 11 },
		{ "C:\\Windows\\temp\\", 16 },
#else
		{ "/", 1 },
		{ "/var", 1 },
		{ "/var/", 5 },
		{ "/var/tmp", 5 },
#endif
	};
	int i;

	for (i = 0; i < (int) N_ELEMENTS(cases); ++i) {
		eq(path_filename_index(cases[i].path), cases[i].expected);
	}
	ok;
}

const char *suite_name = "z-file/filename-index";
struct test tests[] = {
	{ "empty", test_empty },
	{ "no_separator", test_no_separator },
	{ "separator", test_separator },
	{ NULL, NULL }
};
