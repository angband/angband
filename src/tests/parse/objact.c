/* parse/objact */
/* Exercise parsing used for activation.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "effects.h"
#include "init.h"
#include "player-timed.h"
#include "project.h"
#include "object.h"
#include "obj-init.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = act_parser.init();
	/* Needed by act_parser.finish. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (act_parser.finish(p)) {
		r = 1;
	}
	act_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct activation *a = (struct activation*) parser_priv(p);
	enum parser_error r;

	null(a);
	r = parser_parse(p, "aim:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "power:102");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:10:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:3+$Nd4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:N:PLAYER_LEVEL:/ 8");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg:{name} throws off small green sparks...");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:does nothing, spectacularly");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:DO_NOTHING_SPECTACULAR");
	struct activation *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->name);
	require(streq(a->name, "DO_NOTHING_SPECTACULAR"));
	eq(a->aim, false);
	eq(a->power, 0);
	null(a->effect);
	null(a->message);
	null(a->desc);
	ok;
}

static int test_aim0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "aim:1");
	struct activation *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	eq(a->aim, true);
	/* Check that nonzero values other than one are also true. */
	a->aim = false;
	r = parser_parse(p, "aim:10");
	eq(r, PARSE_ERROR_NONE);
	eq(a->aim, true);
	/* Check that zero means false. */
	r = parser_parse(p, "aim:0");
	eq(r, PARSE_ERROR_NONE);
	eq(a->aim, false);
	ok;
}

static int test_power0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "power:105");
	struct activation *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	eq(a->power, 105);
	ok;
}

static int test_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "effect:LIGHT_LEVEL");
	struct activation *a;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->effect);
	e = a->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_LIGHT_LEVEL);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Check effect with type and subtype. */
	r = parser_parse(p, "effect:CURE:BLIND");
	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->effect);
	e = a->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_CURE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_BLIND);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Check effect with type, subtype, and radius. */
	r = parser_parse(p, "effect:BALL:COLD:3");
	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->effect);
	e = a->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_BALL);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_COLD);
	eq(e->radius, 3);
	eq(e->other, 0);
	null(e->msg);
	/* Check effect with type, subtype, radius, and other. */
	r = parser_parse(p, "effect:SPOT:LIGHT_WEAK:2:10");
	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->effect);
	e = a->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_LIGHT_WEAK);
	eq(e->radius, 2);
	eq(e->other, 10);
	null(e->msg);
	ok;
}

static int test_effect_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check with unrecognized effect. */
	enum parser_error r = parser_parse(p, "effect:XYZZY");

	eq(r, PARSE_ERROR_INVALID_EFFECT);
	/* Check with bad subtype. */
	r = parser_parse(p, "effect:CURE:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect. */
	enum parser_error r = parser_parse(p, "effect:BOLT_OR_BEAM:FIRE");
	struct activation *a;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:10+2d12");
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->effect);
	e = a->effect;
	while (e->next) e = e->next;
	notnull(e->dice);
	eq(dice_test_values(e->dice, 10, 2, 12, 0), true);
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "dice:8+3d6");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 8, 3, 6, 0), true);
	ok;
}

static int test_dice_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect. */
	enum parser_error r = parser_parse(p, "effect:TIMED_INC:OPP_COLD");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:2+1d4+1d6");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_expr0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect with dice. */
	enum parser_error r = parser_parse(p, "effect:BREATH:COLD:0:30");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:10d$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:S:PLAYER_HP:+ 99 / 100");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_expr_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect with dice. */
	enum parser_error r = parser_parse(p, "effect:TIMED_INC:OPP_FIRE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+d$S");
	eq(r, PARSE_ERROR_NONE);
	/* Try an expression with an invalid operations string. */
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:^ 2");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/* Try to bind an expression to a variable that isn't in the dice. */
	r = parser_parse(p, "expr:N:PLAYER_LEVEL:/ 5 + 1");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"msg:${name} become{s} very warm...");
	struct activation *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->message);
	require(streq(a->message, "${name} become{s} very warm..."));
	/* Check that multiple directives are concatenated. */
	r = parser_parse(p, "msg: And shrilly scream{s}...");
	notnull(a->message);
	require(streq(a->message,
		"${name} become{s} very warm... And shrilly scream{s}..."));
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:destroys doors");
	struct activation *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->desc);
	require(streq(a->desc, "destroys doors"));
	/* Check that multiple directives are concatenated. */
	r = parser_parse(p, "desc: and lights the nearby area");
	eq(r, PARSE_ERROR_NONE);
	notnull(a->desc);
	require(streq(a->desc, "destroys doors and lights the nearby area"));
	ok;
}

static int test_combined0(void *state) {
	const char *lines[] = {
		"name:PRISMATIC_SPRAY",
		"aim:1",
		"power:4",
		"effect:LINE:LIGHT_WEAK",
		"effect:BOLT:FIRE",
		"dice:1d10",
		"effect:BOLT:COLD",
		"dice:1d8",
		"effect:BOLT:ACID",
		"dice:1d6",
		"effect:BOLT:ELEC",
		"dice:1d4",
		"msg:Five shimmering spheres emerge from your ${kind} and streak to the target.",
		"desc:shoots five different elemental bolts"
	};
	struct parser *p = (struct parser*) state;
	struct activation *a;
	struct effect *e;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	a = (struct activation*) parser_priv(p);
	notnull(a);
	require(streq(a->name, "PRISMATIC_SPRAY"));
	eq(a->aim, true);
	eq(a->power, 4);
	notnull(a->effect);
	e = a->effect;
	eq(e->index, EF_LINE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_LIGHT_WEAK);
	eq(e->radius, 0);
	eq(e->other, 0);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_BOLT);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 0, 1, 10, 0), true);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_FIRE);
	eq(e->radius, 0);
	eq(e->other, 0);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_BOLT);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 0, 1, 8, 0), true);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_COLD);
	eq(e->radius, 0);
	eq(e->other, 0);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_BOLT);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 0, 1, 6, 0), true);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_ACID);
	eq(e->radius, 0);
	eq(e->other, 0);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_BOLT);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 0, 1, 4, 0), true);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_ELEC);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->next);
	notnull(a->message);
	require(streq(a->message, "Five shimmering spheres emerge from your ${kind} and streak to the target."));
	notnull(a->desc);
	require(streq(a->desc, "shoots five different elemental bolts"));
	ok;
}

static int test_missing_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up activation without an effect. */
	enum parser_error r = parser_parse(p, "name:COLD_8");
	struct activation *a;

	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	null(a->effect);
	r = parser_parse(p, "effect-yx:15:23");
	/*
	 * Using effect-yx without a preceding effect should do nothing and
	 * not flag an error.
	 */
	eq(r, PARSE_ERROR_NONE);
	null(a->effect);
	/*
	 * Using dice without a preceding effect should do nothing and not
	 * flag an error.
	 */
	r = parser_parse(p, "dice:d$S");
	eq(r, PARSE_ERROR_NONE);
	null(a->effect);
	/*
	 * Using expr without a preceding effect should do nothing and not
	 * flag an error.
	 */
	r = parser_parse(p, "expr:S:PLAYER_LEVEL:/ 10 + 4");
	eq(r, PARSE_ERROR_NONE);
	null(a->effect);
	ok;
}

static int test_missing_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up activation with an effect but no dice. */
	enum parser_error r = parser_parse(p, "name:CLARITY_2");
	struct activation *a;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect:TIMED_INC:OPP_CONF");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying an expression without preceding dice should do nothing
	 * and not flag an error.
	 */
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:* 2");
	eq(r, PARSE_ERROR_NONE);
	a = (struct activation*) parser_priv(p);
	notnull(a);
	notnull(a->effect);
	e = a->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_TIMED_INC);
	eq(e->subtype, TMD_OPP_CONF);
	null(e->dice);
	ok;
}

const char *suite_name = "parse/objact";
/*
 * test_missing_record_header0() has to be before test_name0(),
 * test_combined0(), test_missing_effect0(), and test_missing_dice0().
 * test_aim0(), test_power0(), test_effect0(), test_effect_bad0(),
 * test_dice0(), test_dice_bad0(), test_expr0(), test_expr_bad0(), test_msg0(),
 * and test_desc0() have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "aim0", test_aim0 },
	{ "power0", test_power0 },
	{ "effect0", test_effect0 },
	{ "effect_bad0", test_effect_bad0 },
	{ "dice0", test_dice0 },
	{ "dice_bad0", test_dice_bad0 },
	{ "expr0", test_expr0 },
	{ "expr_bad0", test_expr_bad0 },
	{ "msg0", test_msg0 },
	{ "desc0", test_desc0 },
	{ "combined0", test_combined0 },
	{ "missing_effect0", test_missing_effect0 },
	{ "missing_dice0", test_missing_dice0 },
	{ NULL, NULL }
};
