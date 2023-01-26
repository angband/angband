/* parse/slay */
/* Exercise parsing used for slay.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "init.h"
#include "monster.h"
#include "object.h"
#include "obj-init.h"
#include "z-virt.h"

static char dummy_bat[16] = "bat";
static char dummy_lizard[16] = "lizard";
static struct monster_base dummy_mon_bases[] = {
	{ .name = dummy_bat, .next = NULL },
	{ .name = dummy_lizard, .next = NULL },
};

int setup_tests(void **state) {
	int i;

	*state = slay_parser.init();
	/* Needed by slay_parser.finish. */
	z_info = mem_zalloc(sizeof(*z_info));
	/* Supply a minimal set of monster bases so tests can function. */
	for (i = 0; i < (int) N_ELEMENTS(dummy_mon_bases) - 1; ++i) {
		dummy_mon_bases[i].next = dummy_mon_bases + i + 1;
	}
	rb_info = dummy_mon_bases;
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (slay_parser.finish(p)) {
		r = 1;
	}
	slay_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state)
{
	struct parser *p = (struct parser*) state;
	struct slay *s = (struct slay*) parser_priv(p);
	enum parser_error r;

	null(s);
	r = parser_parse(p, "name:evil creatures");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "race-flag:EVIL");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "base:bat");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "multiplier:5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "o-multiplier:35");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "power:120");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "melee-verb:smite");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "range-verb:pierces");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_code0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "code:EVIL_2");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	require(streq(s->code, "EVIL_2"));
	null(s->name);
	null(s->base);
	null(s->melee_verb);
	null(s->range_verb);
	eq(s->race_flag, 0);
	eq(s->multiplier, 0);
	eq(s->o_multiplier, 0);
	eq(s->power, 0);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:evil creatures");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	require(streq(s->name, "evil creatures"));
	/* Try setting it again to see if memory is leaked. */
	r = parser_parse(p, "name:bad guys");
	eq(r, PARSE_ERROR_NONE);
	require(streq(s->name, "bad guys"));
	ok;
}

static int test_race_flag0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "race-flag:UNIQUE");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	eq(s->race_flag, RF_UNIQUE);
	ok;
}

static int test_multiplier0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "multiplier:5");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	eq(s->multiplier, 5);
	ok;
}

static int test_o_multiplier0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "o-multiplier:35");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	eq(s->o_multiplier, 35);
	ok;
}

static int test_power0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "power:115");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	eq(s->power, 115);
	ok;
}

static int test_melee_verb0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "melee-verb:smite");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	require(streq(s->melee_verb, "smite"));
	/* Try setting it again to see if memory is leaked. */
	r = parser_parse(p, "melee-verb:wreck");
	eq(r, PARSE_ERROR_NONE);
	require(streq(s->melee_verb, "wreck"));
	ok;
}

static int test_range_verb0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "range-verb:pierces");
	struct slay *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	notnull(s);
	require(streq(s->range_verb, "pierces"));
	/* Try setting it again to see if memory is leaked. */
	r = parser_parse(p, "range-verb:smashes");
	eq(r, PARSE_ERROR_NONE);
	require(streq(s->range_verb, "smashes"));
	ok;
}

static int test_combined0(void *state) {
	struct parser *p = (struct parser*) state;
	const char *lines[] = {
		"code:TROLL_3",
		"name:trolls",
		"race-flag:TROLL",
		"multiplier:3",
		"o-multiplier:25",
		"power:101",
		"melee-verb:trash",
		"range-verb:demolishes"
	};
	struct slay *s;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	s = (struct slay*) parser_priv(p);
	notnull(s);
	require(streq(s->code, "TROLL_3"));
	require(streq(s->name, "trolls"));
	eq(s->race_flag, RF_TROLL);
	null(s->base);
	eq(s->multiplier, 3);
	eq(s->o_multiplier, 25);
	eq(s->power, 101);
	require(streq(s->melee_verb, "trash"));
	require(streq(s->range_verb, "demolishes"));
	ok;
}

static int test_base0(void *state) {
	struct parser *p = (struct parser*) state;
	struct slay *s;
	enum parser_error r;

	/*
	 * Start with a fresh slay record to avoid conflicts with the race flag
	 * or monster base already set on a prior record.
	 */
	r = parser_parse(p, "code:BAT_5");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "base:bat");
	eq(r, PARSE_ERROR_NONE);
	s = (struct slay*) parser_priv(p);
	require(streq(s->base, "bat"));
	ok;
}

static int test_base_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	/*
	 * Start with a fresh slay record to avoid conflicts with the race flag
	 * or monster base already set on a prior record.
	 */
	r = parser_parse(p, "code:XYZZY_10");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "base:XYZZY");
	eq(r, PARSE_ERROR_INVALID_MONSTER_BASE);
	r = parser_parse(p, "code:BAT_8");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "base:bat");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying a race flag when a base has been set for the slay should
	 * generate an error.
	 */
	r = parser_parse(p, "race-flag:UNIQUE");
	eq(r, PARSE_ERROR_INVALID_SLAY);
	r = parser_parse(p, "code:BAT_10");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "base:bat");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying more than one base for the same slay should generate
	 * an error.
	 */
	r = parser_parse(p, "base:lizard");
	eq(r, PARSE_ERROR_INVALID_SLAY);
	ok;
}

static int test_race_flag_bad0(void *state)
{
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	/*
	 * Start with a fresh slay record to avoid conflicts with the race flag
	 * or monster base already set on a prior record.
	 */
	r = parser_parse(p, "code:XYZZY_2");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "race-flag:XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "code:BAT_4");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "base:bat");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying a race flag after a base has been set for the slay
	 * should generate an error.
	 */
	r = parser_parse(p, "race-flag:ANIMAL");
	eq(r, PARSE_ERROR_INVALID_SLAY);
	r = parser_parse(p, "code:ORC_3");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "race-flag:ORC");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying more than one race flag for the same slay should
	 * generate an error.
	 */
	r = parser_parse(p, "race-flag:ANIMAL");
	eq(r, PARSE_ERROR_INVALID_SLAY);
	ok;
}

const char *suite_name = "parse/slay";
/*
 * test_missing_record_header0() has to be before test_code0().
 * test_name0(), test_race_flag0(), test_multiplier0(), test_o_multiplier0(),
 * test_power0(), test_melee_verb0(), test_range_verb0() have to be after
 * test_code0().  test_combined0(), test_base0(), test_base_bad0(), and
 * test_race_flag_bad0() have to be after test_name0(), test_race_flag0(),
 * test_multiplier0(), test_o_multiplier0(), test_power0(), test_melee_verb0(),
 * and test_range_verb0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "code0", test_code0 },
	{ "name0", test_name0 },
	{ "race_flag0", test_race_flag0 },
	{ "multiplier0", test_multiplier0 },
	{ "o_multiplier0", test_o_multiplier0 },
	{ "power0", test_power0 },
	{ "melee_verb0", test_melee_verb0 },
	{ "range_verb0", test_range_verb0 },
	{ "combined0", test_combined0 },
	{ "base0", test_base0 },
	{ "base_bad0",  test_base_bad0 },
	{ "race_flag_bad0", test_race_flag_bad0 },
	{ NULL, NULL }
};
