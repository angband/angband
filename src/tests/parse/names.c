/* parse/names */

#include "unit-test.h"
#include "init.h"

int setup_tests(void **state) {
	*state = init_parse_names();
	return !*state;
}

int teardown_tests(void *state) {
	parser_destroy(state);
	return 0;
}

int test_section0(void *state) {
	errr r = parser_parse(state, "section:1");
	eq(r, 0);
	ok;
}

int test_word0(void *state) {
	errr r = parser_parse(state, "word:foo");
	eq(r, 0);
	r = parser_parse(state, "word:bar");
	eq(r, 0);
	ok;
}

int test_section1(void *state) {
	errr r = parser_parse(state, "section:2");
	eq(r, 0);
	ok;
}

int test_word1(void *state) {
	errr r = parser_parse(state, "word:baz");
	eq(r, 0);
	r = parser_parse(state, "word:quxx");
	eq(r, 0);
	ok;
}

const char *suite_name = "parse/names";
struct test tests[] = {
	{ "section0", test_section0 },
	{ "word0", test_word0 },

	{ "section1", test_section1 },
	{ "word1", test_word1 },

	{ NULL, NULL }
};
