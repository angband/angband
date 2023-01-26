/* parse/ui_knowledge.c */
/* Exercise parsing used for ui_knowledge.txt. */

#include "unit-test.h"
#include "ui-knowledge.h"

static char dummy_bat[16] = "bat";
static char dummy_lizard[16] = "lizard";
static char dummy_horror[16] = "winged horror";
static struct monster_base dummy_mon_bases[] = {
	{ .name = dummy_bat, .next = NULL },
	{ .name = dummy_lizard, .next = NULL },
	{ .name = dummy_horror, .next = NULL },
};

int setup_tests(void **state) {
	int i;

	*state = ui_knowledge_parser.init();
	/* Supply a minimal set of monster bases so the tests can function. */
	for (i = 0; i < (int) N_ELEMENTS(dummy_mon_bases) - 1; ++i) {
		dummy_mon_bases[i].next = dummy_mon_bases + i + 1;
	}
	rb_info = dummy_mon_bases;
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (ui_knowledge_parser.finish(p)) {
		r = 1;
	}
	ui_knowledge_parser.cleanup();
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ui_knowledge_parse_state *s =
		(struct ui_knowledge_parse_state*) parser_priv(p);
	enum parser_error r;

	notnull(s);
	null(s->categories);
	r = parser_parse(p, "mcat-include-base:bat");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "mcat-include-flag:UNIQUE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_category0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "monster-category:Flying Things");
	struct ui_knowledge_parse_state *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct ui_knowledge_parse_state*) parser_priv(p);
	notnull(s);
	notnull(s->categories);
	notnull(s->categories->name);
	require(streq(s->categories->name, "Flying Things"));
	require(rf_is_empty(s->categories->inc_flags));
	eq(s->categories->n_inc_bases, 0);
	ok;
}

static int test_include_base0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ui_knowledge_parse_state *s =
		(struct ui_knowledge_parse_state*) parser_priv(p);
	enum parser_error r;
	int n_old;

	notnull(s);
	notnull(s->categories);
	n_old = s->categories->n_inc_bases;
	r = parser_parse(p, "mcat-include-base:bat");
	eq(r, PARSE_ERROR_NONE);
	/* Try adding another base. */
	r = parser_parse(p, "mcat-include-base:winged horror");
	eq(r, PARSE_ERROR_NONE);
	eq(s->categories->n_inc_bases, n_old + 2);
	notnull(s->categories->inc_bases);
	notnull(s->categories->inc_bases[s->categories->n_inc_bases - 2]);
	notnull(s->categories->inc_bases[s->categories->n_inc_bases - 2]->name);
	require(streq(
            s->categories->inc_bases[s->categories->n_inc_bases - 2]->name,
            "bat"));
	notnull(s->categories->inc_bases[s->categories->n_inc_bases - 1]);
	notnull(s->categories->inc_bases[s->categories->n_inc_bases - 1]->name);
	require(streq(
            s->categories->inc_bases[s->categories->n_inc_bases - 1]->name,
            "winged horror"));
	ok;
}

static int test_include_base_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "mcat-include-base:XYZZY");

	eq(r, PARSE_ERROR_INVALID_MONSTER_BASE);
	ok;
}

static int test_include_flag0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ui_knowledge_parse_state *s =
		(struct ui_knowledge_parse_state*) parser_priv(p);
	bitflag eflags[RF_SIZE];
	enum parser_error r;

	notnull(s);
	notnull(s->categories);
	rf_wipe(s->categories->inc_flags);
	/* Check that specifying an empty set of flags works. */
	r = parser_parse(p, "mcat-include-flag:");
	eq(r, PARSE_ERROR_NONE);
	require(rf_is_empty(s->categories->inc_flags));
	/* Check that specifying one flag works. */
	r = parser_parse(p, "mcat-include-flag:UNIQUE");
	eq(r, PARSE_ERROR_NONE);
	/* Check that specifying two flags at once works. */
	r = parser_parse(p, "mcat-include-flag:MALE | NEVER_MOVE");
	eq(r, PARSE_ERROR_NONE);
	rf_wipe(eflags);
	rf_on(eflags, RF_UNIQUE);
	rf_on(eflags, RF_MALE);
	rf_on(eflags, RF_NEVER_MOVE);
	require(rf_is_equal(s->categories->inc_flags, eflags));
	ok;
}

static int test_include_flag_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	struct ui_knowledge_parse_state *s =
		(struct ui_knowledge_parse_state*) parser_priv(p);
	bitflag eflags[RF_SIZE];
	enum parser_error r;

	notnull(s);
	notnull(s->categories);
	rf_copy(eflags, s->categories->inc_flags);
	/* Try an invalid flag. */
	r = parser_parse(p, "mcat-include-flag:XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	require(rf_is_equal(s->categories->inc_flags, eflags));
	ok;
}

/*
 * test_missing_record_header0() has to be before test_category0().
 * test_include_base0(), test_include_base_bad0(), test_include_flag0(),
 * and test_include_flag_bad0() have to be after test_category0().
 */
const char *suite_name = "parse/ui_knowledge";
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "category0", test_category0 },
	{ "include_base0", test_include_base0 },
	{ "include_base_bad0", test_include_base_bad0 },
	{ "include_flag0", test_include_flag0 },
	{ "include_flag_bad0", test_include_flag_bad0 },
	{ NULL, NULL }
};
