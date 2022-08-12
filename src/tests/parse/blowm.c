/* parse/blowm */
/* Exercise parsing used for blow_methods.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "init.h"
#include "message.h"
#include "mon-blows.h"
#include "mon-init.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = meth_parser.init();
	/* meth_parser.finish needs z_info. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (meth_parser.finish(p)) {
		r = 1;
	}
	meth_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct blow_method *m = (struct blow_method*) parser_priv(p);
	enum parser_error r;

	null(m);
	r = parser_parse(p, "act:hits {target}");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:HIT");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	require(streq(m->name, "HIT"));
	eq(m->cut, false);
	eq(m->stun, false);
	eq(m->miss, false);
	eq(m->phys, false);
	eq(m->msgt, 0);
	null(m->messages);
	eq(m->num_messages, 0);
	null(m->desc);
	ok;
}

static int test_cut0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "cut:10");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	eq(m->cut, true);
	r = parser_parse(p, "cut:0");
	eq(r, PARSE_ERROR_NONE);
	eq(m->cut, false);
	ok;
}

static int test_stun0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "stun:2");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	eq(m->stun, true);
	r = parser_parse(p, "stun:0");
	eq(r, PARSE_ERROR_NONE);
	eq(m->stun, false);
	ok;
}

static int test_miss0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "miss:1");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	eq(m->miss, true);
	r = parser_parse(p, "miss:0");
	eq(r, PARSE_ERROR_NONE);
	eq(m->miss, false);
	ok;
}

static int test_phys0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "phys:5");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	eq(m->phys, true);
	r = parser_parse(p, "phys:0");
	eq(r, PARSE_ERROR_NONE);
	eq(m->phys, false);
	ok;
}

static int test_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msg:MON_HIT");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	eq(m->msgt, MSG_MON_HIT);
	ok;
}

static int test_msg_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msg:XYZZY");

	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	ok;
}

static int test_act0(void *state) {
	struct parser *p = (struct parser*) state;
	struct blow_method *m = (struct blow_method*) parser_priv(p);
	enum parser_error r;
	int old_count;

	notnull(m);
	old_count = m->num_messages;
	r = parser_parse(p, "act:hits {target}");
	eq(r, PARSE_ERROR_NONE);
	notnull(m->messages);
	notnull(m->messages->act_msg);
	require(streq(m->messages->act_msg, "hits {target}"));
	eq(m->num_messages, old_count + 1);
	r = parser_parse(p, "act:slaps {target}");
	eq(r, PARSE_ERROR_NONE);
	/* Check that a second message gets added as a separate entry. */
	require(streq(m->messages->act_msg, "slaps {target}"));
	eq(m->num_messages, old_count + 2);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:hit");
	struct blow_method *m;

	eq(r, PARSE_ERROR_NONE);
	m = (struct blow_method*) parser_priv(p);
	notnull(m);
	notnull(m->desc);
	require(streq(m->desc, "hit"));
	/* Check that a second directive appends to the first. */
	r = parser_parse(p, "desc: something");
	eq(r, PARSE_ERROR_NONE);
	notnull(m->desc);
	require(streq(m->desc, "hit something"));
	ok;
}

const char *suite_name = "parse/blowm";
/*
 * test_missing_record_header0() has to be before test_name0().
 * test_cut0(), test_stun0(), test_miss0(), test_phys0(), test_msg0,
 * test_msg_bad0(), and test_act0 have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "cut0", test_cut0 },
	{ "stun0", test_stun0 },
	{ "miss0", test_miss0 },
	{ "phys0", test_phys0 },
	{ "msg0", test_msg0 },
	{ "msg_bad0", test_msg_bad0 },
	{ "act0", test_act0 },
	{ "desc0", test_desc0 },
	{ NULL, NULL }
};
