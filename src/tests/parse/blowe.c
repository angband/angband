/* parse/blowe */
/* Exercise parsing used for blow_effects.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "init.h"
#include "mon-blows.h"
#include "mon-init.h"
#include "object.h"
#include "project.h"
#include "z-color.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = eff_parser.init();
	/* eff_parser.finish needs z_info. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (eff_parser.finish(p)) {
		r = 1;
	}
	eff_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct blow_effect *e = (struct blow_effect*) parser_priv(p);
	enum parser_error r;

	null(e);
	r = parser_parse(p, "lore-color-base:Orange");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lore-color-resist:Yellow");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lore-color-immune:Green");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:HURT");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	notnull(e->name);
	require(streq(e->name, "HURT"));
	eq(e->power, 0);
	eq(e->eval, 0);
	null(e->desc);
	eq(e->lore_attr, 0);
	eq(e->lore_attr_resist, 0);
	eq(e->lore_attr_immune, 0);
	null(e->effect_type);
	eq(e->resist, 0);
	eq(e->lash_type, 0);
	ok;
}

static int test_power0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "power:30");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->power, 30);
	ok;
}

static int test_eval0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "eval:10");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->eval, 10);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:attack");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	notnull(e->desc);
	require(streq(e->desc, "attack"));
	/* Check that a second directive appends to the first. */
	r = parser_parse(p, "desc: something");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->desc);
	require(streq(e->desc, "attack something"));
	ok;
}

static int test_lore_color_base0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lore-color-base:Orange");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->lore_attr, COLOUR_ORANGE);
	/* Check that the full name lookup is case insensitive. */
	r = parser_parse(p, "lore-color-base:light red");
	eq(e->lore_attr, COLOUR_L_RED);
	/* Check that a single letter code for the color works. */
	r = parser_parse(p, "lore-color-base:o");
	eq(e->lore_attr, COLOUR_ORANGE);
	ok;
}

static int test_lore_color_resist0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lore-color-resist:Yellow");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->lore_attr_resist, COLOUR_YELLOW);
	/* Check that the full name lookup is case insensitive. */
	r = parser_parse(p, "lore-color-resist:light green");
	eq(e->lore_attr_resist, COLOUR_L_GREEN);
	/* Check that a single letter code for the color works. */
	r = parser_parse(p, "lore-color-resist:y");
	eq(e->lore_attr_resist, COLOUR_YELLOW);
	ok;
}

static int test_lore_color_immune0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lore-color-immune:Light Green");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->lore_attr_immune, COLOUR_L_GREEN);
	/* Check that the full name lookup is case insensitive. */
	r = parser_parse(p, "lore-color-immune:light red");
	eq(e->lore_attr_immune, COLOUR_L_RED);
	/* Check that a single letter code for the color works. */
	r = parser_parse(p, "lore-color-immune:G");
	eq(e->lore_attr_immune, COLOUR_L_GREEN);
	ok;
}

static int test_effect_type0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "effect-type:element");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	require(streq(e->effect_type, "element"));
	ok;
}

static int test_effect_type_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect with an unrecognized effect-type. */
	enum parser_error r = parser_parse(p, "name:TEST_BAD_EFFECT_TYPE0");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-type:XYZZY");
	/*
	 * The unrecognized effect-type is detected when trying to use the
	 * resist directive.
	 */
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "resist:POIS");
	eq(r, PARSE_ERROR_MISSING_BLOW_EFF_TYPE);
	ok;
}

static int test_resist0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect with an effect-type of eleemnt. */
	enum parser_error r = parser_parse(p, "name:POISON");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-type:element");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "resist:POIS");
	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->resist, PROJ_POIS);
	/* Set up a new effect with an effect-type of flag. */
	r = parser_parse(p, "name:BLIND");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-type:flag");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "resist:PROT_BLIND");
	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->resist, OF_PROT_BLIND);
	ok;
}

static int test_resist_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect with an effect-type of eleemnt. */
	enum parser_error r = parser_parse(p, "name:BAD_ELEMENT");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-type:element");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "resist:XYZZY");
	/* Doesn't signal an error, but the resist field is set to -1. */
	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->resist, -1);
	/* Set up a new effect with an effect-type of flag. */
	r = parser_parse(p, "name:BAD_FLAG");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-type:flag");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "resist:XYZZY");
	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->resist, -1);
	ok;
}

static int test_lash_type0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lash-type:FIRE");
	struct blow_effect *e;

	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->lash_type, PROJ_FIRE);
	ok;
}

static int test_lash_type_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lash-type:XYZZY");
	struct blow_effect *e;

	/*
	 * Doesn't flag an error and the lash_type field is set to PROJ_MISSILE.
	 */
	eq(r, PARSE_ERROR_NONE);
	e = (struct blow_effect*) parser_priv(p);
	notnull(e);
	eq(e->lash_type, PROJ_MISSILE);
	ok;
}

const char *suite_name = "parse/blowe";
/*
 * test_missing_record_header0() has to be before test_name0(),
 * test_effect_type_bad0(), test_resist0(), and test_resist_bad0().
 * test_power0(), test_eval0(), test_desc0(), test_lore_color_base0(),
 * test_lore_color_resist0(), test_lore_color_immune0(), test_effect_type0(),
 * test_resist0(), test_lash_type0(), test_lash_type_bad0() have to be after
 * test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "power0", test_power0 },
	{ "eval0", test_eval0 },
	{ "desc0", test_desc0 },
	{ "lore_color_base0", test_lore_color_base0 },
	{ "lore_color_resist0", test_lore_color_resist0 },
	{ "lore_color_immune0", test_lore_color_immune0 },
	{ "effect_type0", test_effect_type0 },
	{ "effect_type_bad0", test_effect_type_bad0 },
	{ "resist0", test_resist0 },
	{ "resist_bad0", test_resist_bad0 },
	{ "lash_type0", test_lash_type0 },
	{ "lash_type_bad0", test_lash_type_bad0 },
	{ NULL, NULL }
};
