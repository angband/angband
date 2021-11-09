/* parse/readstore */

#include "unit-test.h"
#include "object.h"
#include "store.h"
#include "init.h"

int setup_tests(void **state) {
	z_info = mem_zalloc(sizeof(struct angband_constants));
	z_info->store_inven_max = 24;
	/* Do the bare minimum so sval and kind lookups work. */
	z_info->k_max = 2;
	z_info->ordinary_kind_max = 2;
	k_info = mem_zalloc(z_info->k_max * sizeof(*k_info));
	k_info[1].tval = 3;
	k_info[1].sval = 5;
	*state = init_parse_stores();
	return !*state;
}

int teardown_tests(void *state) {
	struct store *s = parser_priv(state);
	struct owner *o = s->owners, *o_next;
	while (o) {
		o_next = o->next;
		string_free(o->name);
		mem_free(o);
		o = o_next;
	}
	string_free((char *)s->name);
	mem_free(s->normal_table);
	mem_free(s);
	parser_destroy(state);
	mem_free(k_info);
	mem_free(z_info);
	return 0;
}

static int test_store0(void *state) {
	enum parser_error r = parser_parse(state, "store:1:foobar");
	struct store *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	eq(s->sidx, 0);
	require(streq(s->name, "foobar"));
	ok;
}

static int test_slots0(void *state) {
	enum parser_error r = parser_parse(state, "slots:2:33");
	struct store *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	eq(s->normal_stock_min, 2);
	eq(s->normal_stock_max, 33);
	ok;
}

static int test_owner0(void *state) {
	enum parser_error r = parser_parse(state, "owner:5000:Foo");
	struct store *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	eq(s->owners->max_cost, 5000);
	require(streq(s->owners->name, "Foo"));
	ok;
}

static int test_i0(void *state) {
	enum parser_error r = parser_parse(state, "normal:3:5");
	struct store *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(state);
	require(s);
	require(s->normal_table[0] && s->normal_table[0]->tval == 3
		&& s->normal_table[0]->sval == 5);
	ok;
}

const char *suite_name = "parse/store";
struct test tests[] = {
	{ "store0", test_store0 },
	{ "slots0", test_slots0 },
	{ "owner0", test_owner0 },
	{ "i0", test_i0 },
	{ NULL, NULL }
};
