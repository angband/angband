/* z-virt/string.c */

#include "unit-test.h"
#include "z-virt.h"

NOSETUP
NOTEARDOWN

int test_string_make(void *state) {
	char *s1 = string_make("foo");
	require(s1);
	require(!strcmp(s1, "foo"));
	string_free(s1);
	ok;
}

int test_string_make_null(void *state) {
	char *s1 = string_make(NULL);
	require(!s1);
	ok;
}

int test_string_free_null(void *state) {
	string_free(NULL);
	ok;
}

int test_string_append(void *state) {
	char *s1 = string_make("foo");
	char *s3 = string_append(s1, "bar");

	require(s3);
	require(!strcmp(s3, "foobar"));

	string_free(s3);
	ok;
}

int test_string_append_null0(void *state) {
	char *r = string_append(NULL, "foo");
	require(r);
	require(!strcmp(r, "foo"));
	string_free(r);
	ok;
}

int test_string_append_null1(void *state) {
	char *s = string_make("bar");
	char *r = string_append(s, NULL);
	require(r);
	require(!strcmp(r, "bar"));
	string_free(r);
	ok;
}

int test_string_append_null2(void *state) {
	char *r = string_append(NULL, NULL);
	require(!r);
	ok;
}

const char *suite_name = "z-virt/string";
struct test tests[] = {
	{ "make", test_string_make },
	{ "make-null", test_string_make_null },
	{ "free-null", test_string_free_null },
	{ "append", test_string_append },
	{ "append-null0", test_string_append_null0 },
	{ "append-null1", test_string_append_null1 },
	{ "append-null2", test_string_append_null2 },
	{ NULL, NULL }
};
