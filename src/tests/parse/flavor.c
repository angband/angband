/* parse/flavor.c */

#include "unit-test.h"

#include "init.h"
#include "obj-tval.h"
#include "obj-properties.h"
#include "object.h"
#include "z-color.h"

static char dummy_light_1[16] = "Test Light 1";
static char dummy_light_2[16] = "Test Light 2";
static char dummy_art_light_1[24] = "Test Art. Light 1";
static char dummy_art_light_2[24] = "Test Art. Light 2";
struct object_kind dummy_kinds[] = {
	{ .name = NULL, .kidx = 0, .tval = 0, .sval = 0 },
	{ .name = dummy_light_1, .kidx = 1, .tval = TV_LIGHT, .sval = 1 },
	{ .name = dummy_light_2, .kidx = 2, .tval = TV_LIGHT, .sval = 2 },
	{ .name = dummy_art_light_1, .kidx = 3, .tval = TV_LIGHT, .sval = 3 },
	{ .name = dummy_art_light_2, .kidx = 4, .tval = TV_LIGHT, .sval = 4 }
};

int setup_tests(void **state) {
	*state = flavor_parser.init();
	/*
	 * Do minimal setup so sval lookups work for the tests of fixed flavors.
	 */
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->k_max = (uint16_t) N_ELEMENTS(dummy_kinds);
	z_info->ordinary_kind_max = z_info->k_max - 2;
	k_info = dummy_kinds;
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	struct flavor *f = (struct flavor*) parser_priv(p);

	while (f) {
		struct flavor *fn = f->next;

		string_free(f->text);
		mem_free(f);
		f = fn;
	}
	parser_destroy(p);
	mem_free(z_info);
	return 0;
}

static int test_kind0(void *state) {
	enum parser_error r = parser_parse(state, "kind:light:&");

	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_flavor0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "flavor:2:blue:Fishy");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = (struct flavor*) parser_priv(p);
	notnull(f);
	eq(f->fidx, 2);
	eq(f->tval, TV_LIGHT);
	eq(f->sval, SV_UNKNOWN);
	eq(f->d_char, L'&');
	eq(f->d_attr, COLOUR_BLUE);
	notnull(f->text);
	require(streq(f->text, "Fishy"));
	/* Check without a description and using a single letter color code. */
	r = parser_parse(p, "flavor:3:G");
	eq(r, PARSE_ERROR_NONE);
	f = (struct flavor*) parser_priv(p);
	notnull(f);
	eq(f->fidx, 3);
	eq(f->tval, TV_LIGHT);
	eq(f->sval, SV_UNKNOWN);
	eq(f->d_char, L'&');
	eq(f->d_attr, COLOUR_L_GREEN);
	null(f->text);
	ok;
}

static int test_fixed0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"fixed:4:Test Art. Light 1:Light Red:Smelly");
	struct flavor *f;

	eq(r, PARSE_ERROR_NONE);
	f = (struct flavor*) parser_priv(p);
	notnull(f);
	eq(f->fidx, 4);
	eq(f->tval, TV_LIGHT);
	eq(f->sval, 3);
	eq(f->d_char, L'&');
	eq(f->d_attr, COLOUR_L_RED);
	notnull(f->text);
	require(streq(f->text, "Smelly"));
	/* Check without a description and using a single letter color code. */
	r = parser_parse(p, "fixed:5:Test Art. Light 2:g");
	eq(r, PARSE_ERROR_NONE);
	f = (struct flavor*) parser_priv(p);
	notnull(f);
	eq(f->fidx, 5);
	eq(f->tval, TV_LIGHT);
	eq(f->sval, 4);
	eq(f->d_char, L'&');
	eq(f->d_attr, COLOUR_GREEN);
	null(f->text);
	ok;
}

static int test_kind_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an invalid tval. */
	enum parser_error r = parser_parse(p, "kind:xyzzy:'");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

const char *suite_name = "parse/flavor";
/*
 * test_flavor0() and test_fixed0() have to be after test_kind0().  Run
 * test_kind_bad0() to avoid potential effects on the other tests.
 */
struct test tests[] = {
	{ "kind0", test_kind0 },
	{ "flavor0", test_flavor0 },
	{ "fixed0", test_fixed0 },
	{ "kind_bad0", test_kind_bad0 },
	{ NULL, NULL }
};
