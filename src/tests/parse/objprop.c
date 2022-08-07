/* parse/objprop */
/* Exercise parsing used for object_property.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "init.h"
#include "object.h"
#include "obj-init.h"
#include "ui-entry-init.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = object_property_parser.init();
	/* Need by object_property_parser.finish. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (object_property_parser.finish(p)) {
		r = 1;
	}
	object_property_parser.cleanup();
	ui_entry_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct obj_property *prop = (struct obj_property*) parser_priv(p);
	enum parser_error r;

	null(prop);
	r = parser_parse(p, "type:flag");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "subtype:protection");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "id-type:on effect");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "code:PROT_FEAR");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "power:6");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "mult:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "type-mult:helm:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "adjective:ugly");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "neg-adjective:handsome");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg:Your {name} glows.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:testing");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "bindui:test_ui_0:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:test");
	struct obj_property *prop;
	int i;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	require(streq(prop->name, "test"));
	eq(prop->type, 0);
	eq(prop->subtype, 0);
	eq(prop->id_type, 0);
	eq(prop->index, 0);
	eq(prop->power, 0);
	eq(prop->mult, 0);
	for (i = 0; i < TV_MAX; ++i) {
		eq(prop->type_mult[i], 1);
	}
	null(prop->adjective);
	null(prop->neg_adj);
	null(prop->msg);
	null(prop->desc);
	ok;
}

static int test_type0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type:stat");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	eq(prop->type, OBJ_PROPERTY_STAT);
	r = parser_parse(p, "type:mod");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->type, OBJ_PROPERTY_MOD);
	r = parser_parse(p, "type:flag");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->type, OBJ_PROPERTY_FLAG);
	r = parser_parse(p, "type:ignore");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->type, OBJ_PROPERTY_IGNORE);
	r = parser_parse(p, "type:resistance");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->type, OBJ_PROPERTY_RESIST);
	r = parser_parse(p, "type:vulnerability");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->type, OBJ_PROPERTY_VULN);
	r = parser_parse(p, "type:immunity");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->type, OBJ_PROPERTY_IMM);
	ok;
}

static int test_type_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type:xyzzy");

	eq(r, PARSE_ERROR_INVALID_PROPERTY);
	ok;
}

static int test_subtype0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Mark the property as a flag. */
	enum parser_error r = parser_parse(p, "type:flag");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "subtype:sustain");
	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	eq(prop->subtype, OFT_SUST);
	r = parser_parse(p, "subtype:protection");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_PROT);
	r = parser_parse(p, "subtype:misc ability");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_MISC);
	r = parser_parse(p, "subtype:light");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_LIGHT);
	r = parser_parse(p, "subtype:melee");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_MELEE);
	r = parser_parse(p, "subtype:bad");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_BAD);
	r = parser_parse(p, "subtype:dig");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_DIG);
	r = parser_parse(p, "subtype:throw");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->subtype, OFT_THROW);
	ok;
}

static int test_subtype_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "subtype:xyzzy");

	eq(r, PARSE_ERROR_INVALID_SUBTYPE);
	ok;
}

static int test_id_type0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "id-type:on effect");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	eq(prop->id_type, OFID_NORMAL);
	r = parser_parse(p, "id-type:timed");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->id_type, OFID_TIMED);
	r = parser_parse(p, "id-type:on wield");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->id_type, OFID_WIELD);
	ok;
}

static int test_id_type_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "id-type:xyzzy");

	eq(r, PARSE_ERROR_INVALID_ID_TYPE);
	ok;
}

static int test_code0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a type. */
	enum parser_error r = parser_parse(p, "type:stat");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:STR");
	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	eq(prop->index, OBJ_MOD_STR);

	r = parser_parse(p, "type:mod");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:STEALTH");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->index, OBJ_MOD_STEALTH);

	r = parser_parse(p, "type:flag");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:PROT_FEAR");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->index, OF_PROT_FEAR);

	r = parser_parse(p, "type:ignore");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:COLD");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->index, ELEM_COLD);

	r = parser_parse(p, "type:resistance");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:FIRE");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->index, ELEM_FIRE);

	r = parser_parse(p, "type:vulnerability");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:ACID");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->index, ELEM_ACID);

	r = parser_parse(p, "type:immunity");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:ELEC");
	eq(r, PARSE_ERROR_NONE);
	eq(prop->index, ELEM_ELEC);

	ok;
}

static int test_code_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a type. */
	enum parser_error r = parser_parse(p, "type:stat");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	r = parser_parse(p, "type:mod");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	r = parser_parse(p, "type:flag");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	r = parser_parse(p, "type:ignore");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	r = parser_parse(p, "type:resistance");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	r = parser_parse(p, "type:vulnerability");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	r = parser_parse(p, "type:immunity");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:xyzzy");
	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);

	ok;
}

static int test_power0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "power:50");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	eq(prop->power, 50);
	ok;
}

static int test_mult0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "mult:5");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	eq(prop->mult, 5);
	ok;
}

static int test_type_mult0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type-mult:light:3");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	eq(prop->type_mult[TV_LIGHT], 3);
	ok;
}

static int test_type_mult_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type-mult:xyzzy:2");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

static int test_adjective0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "adjective:weak");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	notnull(prop->adjective);
	require(streq(prop->adjective, "weak"));
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "adjective:bland");
	eq(r, PARSE_ERROR_NONE);
	notnull(prop->adjective);
	require(streq(prop->adjective, "bland"));
	ok;
}

static int test_neg_adjective0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "neg-adjective:opulent");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	notnull(prop->neg_adj);
	require(streq(prop->neg_adj, "opulent"));
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "neg-adjective:wretched");
	eq(r, PARSE_ERROR_NONE);
	notnull(prop->neg_adj);
	require(streq(prop->neg_adj, "wretched"));
	ok;
}

static int test_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msg:Your {name} glows.");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	notnull(prop->msg);
	require(streq(prop->msg, "Your {name} glows."));
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "msg:Your {name} shudders.");
	eq(r, PARSE_ERROR_NONE);
	notnull(prop->msg);
	require(streq(prop->msg, "Your {name} shudders."));
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:confusion");
	struct obj_property *prop;

	eq(r, PARSE_ERROR_NONE);
	prop = (struct obj_property*) parser_priv(p);
	notnull(prop);
	notnull(prop->desc);
	require(streq(prop->desc, "confusion"));
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "desc:fear");
	eq(r, PARSE_ERROR_NONE);
	notnull(prop->desc);
	require(streq(prop->desc, "fear"));
	ok;
}

static int test_bindui0(void *state) {
	struct parser *p = (struct parser*) state;
	/*
	 * Try the two parameer version. As of 4.2.4, this works regardless of
	 * whether the mentioned UI element has been configured.
	 */
	enum parser_error r = parser_parse(p, "bindui:test_ui_0:1");

	eq(r, PARSE_ERROR_NONE);
	/* Try the three parameter version. */
	r = parser_parse(p, "bindui:test_ui_1:0:5");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_missing_type0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a property without a type. */
	enum parser_error r = parser_parse(p, "name:test2");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "code:ACID");
	eq(r, PARSE_ERROR_MISSING_OBJ_PROP_TYPE);
	r = parser_parse(p, "bindui:test_ui_0:0");
	eq(r, PARSE_ERROR_MISSING_OBJ_PROP_TYPE);
	ok;
}

const char *suite_name = "parse/objprop";
/*
 * test_missing_record_header0() has to be before test_name0() and
 * test_missing_type0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "type0", test_type0 },
	{ "type_bad0", test_type_bad0 },
	{ "subtype0", test_subtype0 },
	{ "subtype_bad0", test_subtype_bad0 },
	{ "id_type0", test_id_type0 },
	{ "id_type_bad0", test_id_type_bad0 },
	{ "code0", test_code0 },
	{ "code_bad0", test_code_bad0 },
	{ "power0", test_power0 },
	{ "mult0", test_mult0 },
	{ "type_mult0", test_type_mult0 },
	{ "type_mult_bad0", test_type_mult_bad0 },
	{ "adjective0", test_adjective0 },
	{ "neg_adjective0", test_neg_adjective0 },
	{ "msg0", test_msg0 },
	{ "desc0", test_desc0 },
	{ "bindui0", test_bindui0 },
	{ "missing_type0", test_missing_type0 },
	{ NULL, NULL }
};
