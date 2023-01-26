/* parse/a-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "effects.h"
#include "obj-curse.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "init.h"
#include "z-color.h"
#include "z-virt.h"

static char orc_slay_name[16] = "ORC_3";
static char animal_slay_name[16] = "ANIMAL_2";
static struct slay dummy_slays[] = {
	{ .code = NULL },
	{ .code = orc_slay_name },
	{ .code = animal_slay_name },
};
static char cold_brand_name[16] = "COLD_2";
static char acid_brand_name[16] = "ACID_3";
static struct brand dummy_brands[] = {
	{ .code = NULL },
	{ .code = cold_brand_name },
	{ .code = acid_brand_name },
};
static char vuln_curse_name[16] = "vulnerability";
static char tele_curse_name[16] = "teleportation";
static struct curse dummy_curses[] = {
	{ .name = NULL },
	{ .name = vuln_curse_name },
	{ .name = tele_curse_name },
};
static char recall_act_name[16] = "RECALL";
static char clairvoyance_act_name[16] = "CLAIRVOYANCE";
static char haste_act_name[16] = "HASTE";
static struct activation dummy_activations[] = {
	{ .name = NULL, .index = 0 },
	{ .name = recall_act_name, .index = 1 },
	{ .name = clairvoyance_act_name, .index = 2 },
	{ .name = haste_act_name, .index = 3 },
};

int setup_tests(void **state) {
	int n_act, i;

	*state = init_parse_artifact();
	/* Do the bare minimum so kind lookups work. */
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->k_max = 1;
	z_info->ordinary_kind_max = 1;
	k_info = mem_zalloc(z_info->k_max * sizeof(*k_info));
	kb_info = mem_zalloc(TV_MAX * sizeof(*kb_info));
	kb_info[TV_LIGHT].tval = TV_LIGHT;
	/* Do minimal setup to for testing slay, brand, and curse directives. */
	z_info->slay_max = (uint8_t) N_ELEMENTS(dummy_slays);
	slays = dummy_slays;
	z_info->brand_max = (uint8_t) N_ELEMENTS(dummy_brands);
	brands = dummy_brands;
	z_info->curse_max = (uint8_t) N_ELEMENTS(dummy_curses);
	curses = dummy_curses;
	/* Do minimal setup for testing activation directives. */
	n_act = (int) N_ELEMENTS(dummy_activations);
	z_info->act_max = (uint16_t) n_act;
	for (i = 0; i < n_act - 1; ++i) {
		dummy_activations[i].next = dummy_activations + i + 1;
	}
	activations = dummy_activations;
	return !*state;
}

int teardown_tests(void *state) {
	struct artifact *a = parser_priv(state);
	int k;

	string_free(a->name);
	string_free(a->text);
	string_free(a->alt_msg);
	mem_free(a->slays);
	mem_free(a->brands);
	mem_free(a->curses);
	mem_free(a);
	for (k = 1; k < z_info->k_max; ++k) {
		struct object_kind *kind = &k_info[k];

		string_free(kind->name);
		string_free(kind->text);
		string_free(kind->effect_msg);
		string_free(kind->vis_msg);
		mem_free(kind->brands);
		mem_free(kind->slays);
		mem_free(kind->curses);
		free_effect(kind->effect);
	}
	mem_free(k_info);
	mem_free(kb_info);
	mem_free(z_info);
	parser_destroy(state);
	return 0;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "base-object:light:Arkenstone");

	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "graphics:~:y");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "level:50");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "weight:5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "cost:50000");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "alloc:2:50 to 127");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "attack:1d1:0:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "armor:0:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:SEE_INVIS | HOLD_LIFE | NO_FUEL");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "act:CLAIRVOYANCE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "time:50+d50");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg:Your {kind} grow{s} magical spikes...");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "values:LIGHT[4] | RES_LIGHT[1] | RES_DARK[1]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:It is a highly magical McGuffin.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "slay:ANIMAL_2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "brand:ACID_3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "curse:teleportation:8");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	enum parser_error r = parser_parse(state, "name:of Thrain");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	require(streq(a->name, "of Thrain"));
	ok;
}

static int test_badtval0(void *state) {
	enum parser_error r = parser_parse(state, "base-object:badtval:Junk");
	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

static int test_badtval1(void *state) {
	enum parser_error r = parser_parse(state, "base-object:-1:Junk");
	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

static int test_base_object0(void *state) {
	enum parser_error r = parser_parse(state, "base-object:light:Arkenstone");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->tval, TV_LIGHT);
	eq(a->sval, z_info->ordinary_kind_max);
	ok;
}

static int test_graphics0(void *state) {
	struct parser *p = (struct parser*) state;
	struct artifact *a = (struct artifact*) parser_priv(p);
	struct object_kind *k;
	enum parser_error r;
	bool kind_changed;

	notnull(a);
	k = lookup_kind(a->tval, a->sval);
	notnull(k);
	if (!kf_has(k->kind_flags, KF_INSTA_ART)) {
		kf_on(k->kind_flags, KF_INSTA_ART);
		kind_changed = true;
	} else {
		kind_changed = false;
	}
	/* Try with single letter code for color. */
	r = parser_parse(state, "graphics:&:b");
	eq(r, PARSE_ERROR_NONE);
	eq(k->d_char, '&');
	eq(k->d_attr, COLOUR_BLUE);
	/*
	 * Try with the full name for the color (matching is supposed to be
	 * case insensitive).
	 */
	r = parser_parse(state, "graphics:~:Yellow");
	eq(r, PARSE_ERROR_NONE);
	eq(k->d_char, '~');
	eq(k->d_attr, COLOUR_YELLOW);
	r = parser_parse(state, "graphics:+:light green");
	eq(r, PARSE_ERROR_NONE);
	eq(k->d_char, '+');
	eq(k->d_attr, COLOUR_L_GREEN);
	if (kind_changed) {
		kf_off(k->kind_flags, KF_INSTA_ART);
	}
	ok;
}

static int test_graphics_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	struct artifact *a = (struct artifact*) parser_priv(p);
	struct object_kind *k;
	enum parser_error r;
	bool kind_changed;

	notnull(a);
	k = lookup_kind(a->tval, a->sval);
	notnull(k);
	if (kf_has(k->kind_flags, KF_INSTA_ART)) {
		kf_off(k->kind_flags, KF_INSTA_ART);
		kind_changed = true;
	} else {
		kind_changed = false;
	}
	r = parser_parse(state, "graphics:~:y");
	eq(r, PARSE_ERROR_NOT_SPECIAL_ARTIFACT);
	if (kind_changed) {
		kf_on(k->kind_flags, KF_INSTA_ART);
	}
	ok;
}

static int test_level0(void *state) {
	enum parser_error r = parser_parse(state, "level:3");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->level, 3);
	ok;
}

static int test_weight0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "weight:8");
	struct artifact *a;
	struct object_kind *k;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	eq(a->weight, 8);
	k = lookup_kind(a->tval, a->sval);
	notnull(k);
	if (k->kidx >= z_info->ordinary_kind_max) {
		eq(k->weight, 8);
	}
	ok;
}

static int test_cost0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "cost:200");
	struct artifact *a;
	struct object_kind *k;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	eq(a->cost, 200);
	k = lookup_kind(a->tval, a->sval);
	notnull(k);
	if (k->kidx >= z_info->ordinary_kind_max) {
		eq(k->cost, 200);
	}
	ok;
}

static int test_alloc0(void *state) {
	enum parser_error r = parser_parse(state, "alloc:3:5");
	eq(r, PARSE_ERROR_INVALID_ALLOCATION);
	ok;
}

static int test_alloc1(void *state) {
	enum parser_error r = parser_parse(state, "alloc:3:5 to 300");
	eq(r, PARSE_ERROR_OUT_OF_BOUNDS);
	ok;
}

static int test_alloc2(void *state) {
	enum parser_error r = parser_parse(state, "alloc:3:5 to 10");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->alloc_prob, 3);
	eq(a->alloc_min, 5);
	eq(a->alloc_max, 10);
	ok;
}

static int test_attack0(void *state) {
	enum parser_error r = parser_parse(state, "attack:4d5:8:2");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->dd, 4);
	eq(a->ds, 5);
	eq(a->to_h, 8);
	eq(a->to_d, 2);
	ok;
}

static int test_armor0(void *state) {
	enum parser_error r = parser_parse(state, "armor:3:1");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->ac, 3);
	eq(a->to_a, 1);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct artifact *a = (struct artifact*) parser_priv(p);
	bitflag expflags[OF_SIZE];
	enum parser_error r;
	int i;

	/* Wipe the slate. */
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	of_wipe(a->flags);
	for (i = 0; i < ELEM_MAX; ++i) {
		a->el_info[i].flags = 0;
	}
	/* Try nothing at all. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	/* Try two object flags. */
	r = parser_parse(p, "flags:SEE_INVIS | HOLD_LIFE");
	eq(r, PARSE_ERROR_NONE);
	/* Try adding a single element flag. */
	r = parser_parse(p, "flags:HATES_FIRE");
	eq(r, PARSE_ERROR_NONE);
	/* Check that state is correct. */
	of_wipe(expflags);
	of_on(expflags, OF_SEE_INVIS);
	of_on(expflags, OF_HOLD_LIFE);
	require(of_is_equal(a->flags, expflags));
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(a->el_info[i].flags, ((i == ELEM_FIRE) ? EL_INFO_HATES : 0));
	}
	ok;
}

static int test_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized flag. */
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	/* Try an unrecognized element. */
	r = parser_parse(p, "flags:HATES_XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "flags:IGNORE_XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_act0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "act:CLAIRVOYANCE");
	struct artifact *a;
	struct object_kind *k;
	struct activation *act;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	k = lookup_kind(a->tval, a->sval);
	if (a->tval == TV_LIGHT && k->kidx >= z_info->ordinary_kind_max) {
		notnull(k->activation);
		act = k->activation;
	} else {
		notnull(a->activation);
		act = a->activation;
	}
	notnull(act->name);
	require(streq(act->name, "CLAIRVOYANCE"));
	ok;
}

static int test_values0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "values:STR[1] | CON[1]");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	eq(a->modifiers[0], 1);
	eq(a->modifiers[4], 1);
	/* Check a resistance. */
	r = parser_parse(p, "values:RES_ACID[-1]");
	eq(r, PARSE_ERROR_NONE);
	eq(a->el_info[ELEM_ACID].res_level, -1);
	ok;
}

static int test_values_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized object modifier. */
	enum parser_error r = parser_parse(p, "values:XYZZY[-4]");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	/* Try an unrecognized element. */
	r = parser_parse(p, "values:RES_XYZZY[1]");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_time0(void *state) {
	enum parser_error r = parser_parse(state, "time:20+d30");
	struct artifact *a;
	struct object_kind *k;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	k = lookup_kind(a->tval, a->sval);
	noteq(k, NULL);
	if (k->kidx >= z_info->ordinary_kind_max) {
		eq(k->time.base, 20);
		eq(k->time.sides, 30);
	} else {
		eq(a->time.base, 20);
		eq(a->time.sides, 30);
	}
	ok;
}

static int test_msg0(void *state) {
	enum parser_error r = parser_parse(state, "msg:foo");
	struct artifact *a;

	eq(r, 0);
	r = parser_parse(state, "msg:bar");
	eq(r, 0);
	a = parser_priv(state);
	require(a);
	require(streq(a->alt_msg, "foobar"));
	ok;
}


static int test_desc0(void *state) {
	enum parser_error r = parser_parse(state, "desc:baz");
	struct artifact *a;

	eq(r, 0);
	r = parser_parse(state, "desc: quxx");
	eq(r, 0);
	a = parser_priv(state);
	require(a);
	require(streq(a->text, "baz quxx"));
	ok;
}

static int test_slay0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "slay:ANIMAL_2");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	notnull(a->slays);
	eq(a->slays[0], false);
	eq(a->slays[1], false);
	eq(a->slays[2], true);
	ok;
}

static int test_slay_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "slay:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_SLAY);
	ok;
}

static int test_brand0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "brand:ACID_3");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	notnull(a->brands);
	eq(a->brands[0], false);
	eq(a->brands[1], false);
	eq(a->brands[2], true);
	ok;
}

static int test_brand_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "brand:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_BRAND);
	ok;
}

static int test_curse0(void *state) {
	struct parser* p = (struct parser*) state;
	/*
	 * Check that adding a curse with a non-positive power has no effect
	 * and does not flag an error.
	 */
	enum parser_error r = parser_parse(p, "curse:vulnerability:0");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct artifact*) parser_priv(p);
	notnull(a);
	null(a->curses);
	r = parser_parse(p, "curse:vulnerability:-7");
	eq(r, PARSE_ERROR_NONE);
	null(a->curses);
	/* Check that adding with a positive power does have an effect. */
	r = parser_parse(p, "curse:teleportation:15");
	eq(r, PARSE_ERROR_NONE);
	eq(a->curses[0], 0);
	eq(a->curses[1], 0);
	eq(a->curses[2], 15);
	ok;
}

static int test_curse_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "curse:xyzzy:25");

	eq(r, PARSE_ERROR_UNRECOGNISED_CURSE);
	ok;
}

const char *suite_name = "parse/a-info";
/* test_missing_record_header0() has to be before test_name0(). */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "badtval0", test_badtval0 },
	{ "badtval1", test_badtval1 },
	{ "base-object0", test_base_object0 },
	{ "graphics0", test_graphics0 },
	{ "graphics_bad0", test_graphics_bad0 },
	{ "level0", test_level0 },
	{ "weight0", test_weight0 },
	{ "cost0", test_cost0 },
	{ "alloc0", test_alloc0 },
	{ "alloc1", test_alloc1 },
	{ "alloc2", test_alloc2 },
	{ "attack0", test_attack0 },
	{ "armor0", test_armor0 },
	{ "flags0", test_flags0 },
	{ "flags_bad0", test_flags_bad0 },
	{ "act0", test_act0 },
	{ "time0", test_time0 },
	{ "msg0", test_msg0 },
	{ "desc0", test_desc0 },
	{ "values0", test_values0 },
	{ "values_bad0", test_values_bad0 },
	{ "slay0", test_slay0 },
	{ "slay_bad0", test_slay_bad0 },
	{ "brand0", test_brand0 },
	{ "brand_bad0", test_brand_bad0 },
	{ "curse0", test_curse0 },
	{ "curse_bad0", test_curse_bad0 },
	{ NULL, NULL }
};
