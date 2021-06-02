/* parse/names */

#include "unit-test.h"
#include "init.h"
#include "randname.h"

struct name {
	struct name *next;
	char *str;
};

struct names_parse {
	unsigned int section;
	unsigned int nnames[RANDNAME_NUM_TYPES];
	struct name *names[RANDNAME_NUM_TYPES];
};

int setup_tests(void **state) {
	*state = init_parse_names();
	return !*state;
}

int teardown_tests(void *state) {
	struct names_parse *n = parser_priv(state);
	mem_free(n);
	parser_destroy(state);
	return 0;
}

static int test_section0(void *state) {
	errr r = parser_parse(state, "section:1");
	eq(r, 0);
	ok;
}

static int test_word0(void *state) {
	errr r = parser_parse(state, "word:foo");
	struct names_parse *s = parser_priv(state);
	eq(r, 0);
	string_free(s->names[s->section]->str);
	mem_free(s->names[s->section]);
	r = parser_parse(state, "word:bar");
	s = parser_priv(state);
	eq(r, 0);
	string_free(s->names[s->section]->str);
	mem_free(s->names[s->section]);
	ok;
}

static int test_section1(void *state) {
	errr r = parser_parse(state, "section:2");
	eq(r, 0);
	ok;
}

static int test_word1(void *state) {
	errr r = parser_parse(state, "word:baz");
	struct names_parse *s = parser_priv(state);
	eq(r, 0);
	string_free(s->names[s->section]->str);
	mem_free(s->names[s->section]);
	r = parser_parse(state, "word:quxx");
	s = parser_priv(state);
	eq(r, 0);
	string_free(s->names[s->section]->str);
	mem_free(s->names[s->section]);
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
