/* parse/a-info */

#include "unit-test.h"
#include "unit-test-data.h"
#include "effects.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "init.h"
	
int setup_tests(void **state) {
	*state = init_parse_artifact();
	/* Do the bare minimum so kind lookups work. */
	z_info = mem_zalloc(sizeof(*z_info));
	z_info->k_max = 1;
	z_info->ordinary_kind_max = 1;
	k_info = mem_zalloc(TV_MAX * sizeof(*k_info));
	kb_info = mem_zalloc(TV_MAX * sizeof(*kb_info));
	kb_info[TV_LIGHT].tval = TV_LIGHT;
	return !*state;
}

int teardown_tests(void *state) {
	struct artifact *a = parser_priv(state);
	int k;

	string_free(a->name);
	string_free(a->text);
	string_free(a->alt_msg);
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
	enum parser_error r = parser_parse(state, "weight:8");
	struct artifact *a;
	struct object_kind *k;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->weight, 8);
	k = lookup_kind(a->tval, a->sval);
	noteq(k, NULL);
	if (k->kidx >= z_info->ordinary_kind_max) {
		eq(k->weight, 8);
	}
	ok;
}

static int test_cost0(void *state) {
	enum parser_error r = parser_parse(state, "cost:200");
	struct artifact *a;
	struct object_kind *k;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	eq(a->cost, 200);
	k = lookup_kind(a->tval, a->sval);
	noteq(k, NULL);
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
	enum parser_error r = parser_parse(state, "flags:SEE_INVIS | HOLD_LIFE");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	require(a);
	require(a->flags);
	ok;
}

static int test_values0(void *state) {
	enum parser_error r = parser_parse(state, "values:STR[1] | CON[1]");
	struct artifact *a;

	eq(r, PARSE_ERROR_NONE);
	a = parser_priv(state);
	eq(a->modifiers[0], 1);
	eq(a->modifiers[4], 1);
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

const char *suite_name = "parse/a-info";
struct test tests[] = {
	{ "name0", test_name0 },
	{ "badtval0", test_badtval0 },
	{ "badtval1", test_badtval1 },
	{ "base-object0", test_base_object0 },
	{ "level0", test_level0 },
	{ "weight0", test_weight0 },
	{ "cost0", test_cost0 },
	{ "alloc0", test_alloc0 },
	{ "alloc1", test_alloc1 },
	{ "alloc2", test_alloc2 },
	{ "attack0", test_attack0 },
	{ "armor0", test_armor0 },
	{ "flags0", test_flags0 },
	{ "time0", test_time0 },
	{ "msg0", test_msg0 },
	{ "desc0", test_desc0 },
	{ "values0", test_values0 },
	{ NULL, NULL }
};
