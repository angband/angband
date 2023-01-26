/* parse/mbase */
/* Exercise parsing used for monster_base.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "init.h"
#include "monster.h"
#include "mon-init.h"
#include "mon-spell.h"
#include "z-form.h"
#include "z-virt.h"
#include <locale.h>
#include <langinfo.h>

static struct monster_pain dummy_pain_messages[] = {
	{ { NULL, NULL, NULL, NULL, NULL, NULL, NULL }, 0, NULL },
	{
		{
			"shrug[s] off the attack.",
			"grunt[s] with pain.",
			"cr[ies|y] out in pain.",
			"scream[s] in pain.",
			"scream[s] in agony.",
			"writhe[s] in agony.",
			"cr[ies|y] out feebly."
		},
		1, NULL
	},
	{
		{
			"barely notice[s].",
			"flinch[es].",
			"squelch[es].",
			"quiver[s] in pain.",
			"writhe[s] about.",
			"writhe[s] in agony.",
			"jerk[s] limply."
		},
		2, NULL
	}
};

int setup_tests(void **state) {
	int i;

	*state = mon_base_parser.init();
	/* Set up enough pain messages to exercise the pain directive. */
	for (i = (int) N_ELEMENTS(dummy_pain_messages) - 2; i >= 0; --i) {
		dummy_pain_messages[i].next = dummy_pain_messages + i + 1;
	}
	pain_messages = dummy_pain_messages;
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->mp_max = (uint16_t) N_ELEMENTS(dummy_pain_messages);
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (mon_base_parser.finish(p)) {
		r = 1;
	}
	mon_base_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_base *rb = (struct monster_base*) parser_priv(p);
	enum parser_error r;

	null(rb);
	r = parser_parse(p, "glyph:D");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "pain:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:DRAGON | NO_CONF");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:Ancient Dragon/Wyrm");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:ancient dragon");
	struct monster_base *rb;

	eq(r, PARSE_ERROR_NONE);
	rb = (struct monster_base*) parser_priv(p);
	notnull(rb);
	notnull(rb->name);
	require(streq(rb->name, "ancient dragon"));
	null(rb->text);
	eq(rb->d_char, 0);
	null(rb->pain);
	require(rf_is_empty(rb->flags));
	ok;
}

static int test_glyph0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "glyph:D");
	struct monster_base *rb;

	eq(r, PARSE_ERROR_NONE);
	rb = (struct monster_base*) parser_priv(p);
	notnull(rb);
	eq(rb->d_char, L'D');
	if (setlocale(LC_CTYPE, "") && streq(nl_langinfo(CODESET), "UTF-8")) {
		/*
		 * Check that a glyph outside of the ASCII range works.  Using
		 * the Yen sign, U+00A5 or C2 A5 as UTF-8.
		 */
		wchar_t wcs[3];
		int nc;

		r = parser_parse(p, "glyph:¥");
		eq(r, PARSE_ERROR_NONE);
		nc = text_mbstowcs(wcs, "¥", (int) N_ELEMENTS(wcs));
		eq(nc, 1);
		eq(rb->d_char, wcs[0]);
	}
	ok;
}

static int test_pain0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "pain:1");
	struct monster_base *rb;

	eq(r, PARSE_ERROR_NONE);
	rb = (struct monster_base*) parser_priv(p);
	notnull(rb);
	require(rb->pain == dummy_pain_messages + 1);
	ok;
}

static int test_pain_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	char buffer[80];
	enum parser_error r;
	size_t np;

	np = strnfmt(buffer, sizeof(buffer), "pain:%d",
		(int) N_ELEMENTS(dummy_pain_messages));
	require(np < sizeof(buffer));
	r = parser_parse(p, buffer);
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_base *rb = (struct monster_base*) parser_priv(p);
	bitflag expected[RF_SIZE];
	enum parser_error r;

	notnull(rb);
	rf_wipe(rb->flags);
	/* Check that specifying an empty set of flags works. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	require(rf_is_empty(rb->flags));
	/* Try setting one flag. */
	r = parser_parse(p, "flags:UNIQUE");
	eq(r, PARSE_ERROR_NONE);
	/* Try setting more than one flag. */
	r = parser_parse(p, "flags:MALE | UNAWARE");
	eq(r, PARSE_ERROR_NONE);
	rf_wipe(expected);
	rf_on(expected, RF_UNIQUE);
	rf_on(expected, RF_MALE);
	rf_on(expected, RF_UNAWARE);
	require(rf_is_equal(rb->flags, expected));
	ok;
}

static int test_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check that an unknown flag generates an appropriate error. */
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:something");
	struct monster_base *rb;

	eq(r, PARSE_ERROR_NONE);
	rb = (struct monster_base*) parser_priv(p);
	notnull(rb);
	notnull(rb->text);
	require(streq(rb->text, "something"));
	/* Check that another directive appends to the first. */
	r = parser_parse(p, "desc: nasty");
	eq(r, PARSE_ERROR_NONE);
	notnull(rb->text);
	require(streq(rb->text, "something nasty"));
	ok;
}

static int test_combined0(void *state) {
	const char *lines[] = {
		"name:mold",
		"glyph:,",
		"pain:2",
		"flags:NEVER_MOVE",
		"flags:HURT_FIRE | NO_SLEEP",
		"desc:Nasty immobile fungi"
	};
	struct parser *p = (struct parser*) state;
	struct monster_base *rb;
	bitflag eflags[RF_SIZE];
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	rb = (struct monster_base*) parser_priv(p);
	notnull(rb);
	notnull(rb->name);
	require(streq(rb->name, "mold"));
	eq(rb->d_char, L',');
	eq(rb->pain, dummy_pain_messages + 2);
	rf_wipe(eflags);
	rf_on(eflags, RF_NEVER_MOVE);
	rf_on(eflags, RF_HURT_FIRE);
	rf_on(eflags, RF_NO_SLEEP);
	require(rf_is_equal(rb->flags, eflags));
	notnull(rb->text);
	require(streq(rb->text, "Nasty immobile fungi"));
	ok;
}

const char *suite_name = "parse/mbase";
/*
 * test_missing_record_header0() has to be before test_name0() and
 * test_combined0().
 * test_glyph0(), test_pain0(), test_pain_bad0(), test_flags0(),
 * test_flags_bad0() and test_desc0() have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "glyph0", test_glyph0 },
	{ "pain0", test_pain0 },
	{ "pain_bad0", test_pain_bad0 },
	{ "flags0", test_flags0 },
	{ "flags_bad0", test_flags_bad0 },
	{ "desc0", test_desc0 },
	{ "combined0", test_combined0 },
	{ NULL, NULL }
};
