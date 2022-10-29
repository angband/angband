/* parse/body */
/* Exercise parsing used for body.txt. */

#include "unit-test.h"
#include "init.h"
#include "obj-gear.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = body_parser.init();
	/* body_parser.finish() needs z_info. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (body_parser.finish(p)) {
		r = 1;
	}
	body_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	null(parser_priv(p));
	r = parser_parse(p, "slot:WEAPON:weapon");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_body0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "body:Test Body 1");
	struct player_body *b;

	eq(r, PARSE_ERROR_NONE);
	b = (struct player_body*) parser_priv(p);
	notnull(b);
	notnull(b->name);
	require(streq(b->name, "Test Body 1"));
	eq(b->count, 0);
	null(b->slots);
	ok;
}

static int test_slot0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_body *b = (struct player_body*) parser_priv(p);
	struct equip_slot *s;
	enum parser_error r;
	uint16_t old_count;

	notnull(b);
	old_count = b->count;
	r = parser_parse(p, "slot:WEAPON:weapon");
	eq(r, PARSE_ERROR_NONE);
	b = (struct player_body*) parser_priv(p);
	notnull(b);
	eq(b->count, old_count + 1);
	s = b->slots;
	notnull(s);
	while (s->next) s = s->next;
	eq(s->type, EQUIP_WEAPON);
	notnull(s->name);
	require(streq(s->name, "weapon"));
	null(s->obj);
	/* Try adding another slot. */
	r = parser_parse(p, "slot:BOW:shooting");
	eq(r, PARSE_ERROR_NONE);
	b = (struct player_body*) parser_priv(p);
	notnull(b);
	eq(b->count, old_count + 2);
	s = b->slots;
	notnull(s);
	while (s->next) s = s->next;
	eq(s->type, EQUIP_BOW);
	notnull(s->name);
	require(streq(s->name, "shooting"));
	null(s->obj);
	ok;
}

static int test_slot_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try a bad type of slot. */
	enum parser_error r = parser_parse(p, "slot:XYZZY:left nostril");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_complete0(void *state) {
	const char *lines[] = {
		"body:Humanoid",
		"slot:WEAPON:weapon",
		"slot:RING:right hand",
		"slot:RING:left hand",
		"slot:LIGHT:light",
		"slot:BODY_ARMOR:body",
		"slot:CLOAK:back",
		"slot:HAT:head"
	};
	struct parser *p = (struct parser*) state;
	struct player_body *b;
	struct equip_slot *s;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	b = (struct player_body*) parser_priv(p);
	notnull(b);
	require(streq(b->name, "Humanoid"));
	s = b->slots;
	notnull(s);
	eq(s->type, EQUIP_WEAPON);
	notnull(s->name);
	require(streq(s->name, "weapon"));
	null(s->obj);
	s = s->next;
	notnull(s);
	eq(s->type, EQUIP_RING);
	notnull(s->name);
	require(streq(s->name, "right hand"));
	null(s->obj);
	s = s->next;
	notnull(s);
	eq(s->type, EQUIP_RING);
	notnull(s->name);
	require(streq(s->name, "left hand"));
	null(s->obj);
	s = s->next;
	notnull(s);
	eq(s->type, EQUIP_LIGHT);
	notnull(s->name);
	require(streq(s->name, "light"));
	null(s->obj);
	s = s->next;
	notnull(s);
	eq(s->type, EQUIP_BODY_ARMOR);
	notnull(s->name);
	require(streq(s->name, "body"));
	null(s->obj);
	s = s->next;
	notnull(s);
	eq(s->type, EQUIP_CLOAK);
	notnull(s->name);
	require(streq(s->name, "back"));
	null(s->obj);
	s = s->next;
	notnull(s);
	eq(s->type, EQUIP_HAT);
	notnull(s->name);
	require(streq(s->name, "head"));
	null(s->obj);
	s = s->next;
	null(s);
	ok;
}

const char *suite_name = "parse/body";
/*
 * test_missing_record_header0() has to be before test_body0() and
 * test_complete0().
 * test_slot0() and test_slot_bad0() have to be after test_body0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "body0", test_body0 },
	{ "slot0", test_slot0 },
	{ "slot_bad0", test_slot_bad0 },
	{ "complete0", test_complete0 },
	{ NULL, NULL }
};
