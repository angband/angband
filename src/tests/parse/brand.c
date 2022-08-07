/* parse/brand */
/* Exercise parsing used for brand.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "init.h"
#include "monster.h"
#include "object.h"
#include "obj-init.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = brand_parser.init();
	/* Needed by brand_parser.finish. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (brand_parser.finish(p)) {
		r = 1;
	}
	brand_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct brand *b = (struct brand*) parser_priv(p);
	enum parser_error r;

	null(b);
	r = parser_parse(p, "name:acid");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "verb:dissolve");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "multiplier:3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "o-multiplier:25");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "power:161");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "resist-flag:IM_ACID");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "vuln-flag:HURT_FIRE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_code0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "code:FIRE_3");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	require(streq(b->code, "FIRE_3"));
	null(b->name);
	null(b->verb);
	eq(b->multiplier, 0);
	eq(b->o_multiplier, 0);
	eq(b->power, 0);
	eq(b->resist_flag, 0);
	eq(b->vuln_flag, 0);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:fire");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	require(streq(b->name, "fire"));
	/* Try setting it again to see if memory is leaked. */
	r = parser_parse(p, "name:flame");
	eq(r, PARSE_ERROR_NONE);
	require(streq(b->name, "flame"));
	ok;
}

static int test_verb0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "verb:burn");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	require(streq(b->verb, "burn"));
	/* Try setting it again to see if memory is leaked. */
	r = parser_parse(p, "verb:incinerate");
	eq(r, PARSE_ERROR_NONE);
	require(streq(b->verb, "incinerate"));
	ok;
}

static int test_multiplier0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "multiplier:3");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	eq(b->multiplier, 3);
	ok;
}

static int test_o_multiplier0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "o-multiplier:25");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	eq(b->o_multiplier, 25);
	ok;
}

static int test_resist_flag0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "resist-flag:IM_FIRE");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	eq(b->resist_flag, RF_IM_FIRE);
	ok;
}

static int test_resist_flag_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "resist-flag:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_vuln_flag0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "vuln-flag:HURT_FIRE");
	struct brand *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct brand*) parser_priv(p);
	notnull(b);
	eq(b->vuln_flag, RF_HURT_FIRE);
	ok;
}

static int test_vuln_flag_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "vuln-flag:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_combined0(void *state) {
	struct parser *p = (struct parser*) state;
	const char *lines[] = {
		"code:COLD_2",
		"name:cold",
		"verb:chill",
		"multiplier:2",
		"o-multiplier:15",
		"power:109",
		"resist-flag:IM_COLD",
		"vuln-flag:HURT_COLD"
	};
	struct brand *b;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	b = (struct brand*) parser_priv(p);
	require(streq(b->code, "COLD_2"));
	require(streq(b->name, "cold"));
	require(streq(b->verb, "chill"));
	eq(b->multiplier, 2);
	eq(b->o_multiplier, 15);
	eq(b->resist_flag, RF_IM_COLD);
	eq(b->vuln_flag, RF_HURT_COLD);
	ok;
}

const char *suite_name = "parse/brand";
/*
 * test_missing_record_header0() has to be before test_code0().
 * test_name0(), test_verb0(), test_multiplier0(), test_o_multiplier0(),
 * test_resist_flag0(), test_resist_flag_bad0(), test_vuln_flag0(), and
 * test_vuln_flag_bad0() have to be after test_code0().
 * test_combined0() has to be after test_name0(), test_verb0(),
 * test_multiplier0(), test_o_multiplier0(), test_resist_flag0(), and
 * test_vuln_flag0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "code0", test_code0 },
	{ "name0", test_name0 },
	{ "verb0", test_verb0 },
	{ "multiplier0", test_multiplier0 },
	{ "o_multiplier0", test_o_multiplier0 },
	{ "resist_flag0", test_resist_flag0 },
	{ "resist_flag_bad0", test_resist_flag_bad0 },
	{ "vuln_flag0", test_vuln_flag0 },
	{ "vuln_flag_bad0", test_vuln_flag_bad0 },
	{ "combined0", test_combined0 },
	{ NULL, NULL }
};
