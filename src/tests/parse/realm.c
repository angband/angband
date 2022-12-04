/* parse/realm */
/* Exercise parsing used for realm.txt. */

#include "unit-test.h"
#include "init.h"
#include "player.h"

int setup_tests(void **state) {
	*state = realm_parser.init();
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (realm_parser.finish(p)) {
		r = 1;
	}
	realm_parser.cleanup();
	return r;
}

static int test_missing_record_header0(void *state)
{
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	null(parser_priv(p));
	r = parser_parse(p, "stat:STR");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "verb:perform");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "spell-noun:feat of strength");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "book-noun:exercise manual");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:calisthenics");
	struct magic_realm *realm;

	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->name);
	require(streq(realm->name, "calisthenics"));
	eq(realm->stat, 0);
	null(realm->verb);
	null(realm->spell_noun);
	null(realm->book_noun);
	ok;
}

static int test_stat0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "stat:STR");
	struct magic_realm *realm;

	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	eq(realm->stat, STAT_STR);
	ok;
}

static int test_stat_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized stat. */
	enum parser_error r = parser_parse(p, "stat:XYZZY");

	eq(r, PARSE_ERROR_INVALID_SPELL_STAT);
	ok;
}

static int test_verb0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "verb:perform");
	struct magic_realm *realm;

	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->verb);
	require(streq(realm->verb, "perform"));
	/*
	 * Try setting again to see that the prior value is replaced without
	 * a memory leak.
	 */
	r = parser_parse(p, "verb:recite");
	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->verb);
	require(streq(realm->verb, "recite"));
	ok;
}

static int test_spell_noun0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "spell-noun:feat of strength");
	struct magic_realm *realm;

	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->spell_noun);
	require(streq(realm->spell_noun, "feat of strength"));
	/*
	 * Try setting again to see that the prior value is replaced without
	 * a memory leak.
	 */
	r = parser_parse(p, "spell-noun:prayer");
	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->spell_noun);
	require(streq(realm->spell_noun, "prayer"));
	ok;
}

static int test_book_noun0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "book-noun:exercise manual");
	struct magic_realm *realm;

	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->book_noun);
	require(streq(realm->book_noun, "exercise manual"));
	/*
	 * Try setting again to see that the prior value is replaced without
	 * a memory leak.
	 */
	r = parser_parse(p, "book-noun:hymnal");
	eq(r, PARSE_ERROR_NONE);
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->book_noun);
	require(streq(realm->book_noun, "hymnal"));
	ok;
}

static int test_complete0(void *state) {
	const char *lines[] = {
		"name:arcane",
		"stat:INT",
		"verb:cast",
		"spell-noun:spell",
		"book-noun:magic book"
	};
	struct parser *p = (struct parser*) state;
	int i;
	struct magic_realm *realm;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	realm = (struct magic_realm*) parser_priv(p);
	notnull(realm);
	notnull(realm->name);
	require(streq(realm->name, "arcane"));
	eq(realm->stat, STAT_INT);
	notnull(realm->verb);
	require(streq(realm->verb, "cast"));
	notnull(realm->spell_noun);
	require(streq(realm->spell_noun, "spell"));
	notnull(realm->book_noun);
	require(streq(realm->book_noun, "magic book"));
	ok;
}

const char *suite_name = "parse/realm";
/*
 * test_missing_record_header0() has to be before test_name0() and
 * test_complete0().
 * test_stat0(), test_stat_bad0(), test_verb0(), test_spell_noun0(), and
 * test_book_noun0() have to after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "stat0", test_stat0 },
	{ "stat_bad0", test_stat_bad0 },
	{ "verb0", test_verb0 },
	{ "spell_noun0", test_spell_noun0 },
	{ "book_noun0", test_book_noun0 },
	{ "complete0", test_complete0 },
	{ NULL, NULL }
};
