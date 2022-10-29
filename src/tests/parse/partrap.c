/* parse/partrap */
/* Exercise parsing used for trap.txt. */

#include "unit-test.h"

struct chunk;
#include "effects.h"
#include "init.h"
#include "object.h"
#include "player.h"
#include "player-timed.h"
#include "project.h"
#include "trap.h"
#include "z-color.h"

int setup_tests(void **state) {
	*state = trap_parser.init();
	/* trap_parser.finish() needs z_info. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (trap_parser.finish(p)) {
		r = 1;
	}
	trap_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_header_record0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	null(parser_priv(p));
	r = parser_parse(p, "graphics:;:G");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "appear:2:20:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "visibility:70");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:TRAP | FLOOR | PIT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:11:22");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:4d$S");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:S:DUNGEON_LEVEL:/ 2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-xtra:TIMED_INC:CUT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx-xtra:13:25");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice-xtra:8d$S");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr-xtra:S:DUNGEON_LEVEL:/ 25 + 3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "save:FEATHER");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:A hole dug to snare the unwary.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg:You fall into a pit!");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg-good:You float gently to the bottom of "
		"the pit.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg-bad:A small dart hits you!");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg-xtra:You are impaled!");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:test trap:test trap 1");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->name);
	require(streq(t->name, "test trap"));
	null(t->text);
	notnull(t->desc);
	require(streq(t->desc, "test trap 1"));
	null(t->msg);
	null(t->msg_good);
	null(t->msg_bad);
	null(t->msg_xtra);
	eq(t->d_attr, 0);
	eq(t->d_char, 0);
	eq(t->rarity, 0);
	eq(t->min_depth, 0);
	eq(t->max_num, 0);
	eq(t->power.base, 0);
	eq(t->power.dice, 0);
	eq(t->power.sides, 0);
	eq(t->power.m_bonus, 0);
	require(trf_is_empty(t->flags));
	require(of_is_empty(t->save_flags));
	null(t->effect);
	null(t->effect_xtra);
	ok;
}

static int test_graphics0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with a full name for the color. */
	enum parser_error r = parser_parse(p, "graphics:^:Red");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	eq(t->d_attr, COLOUR_RED);
	eq(t->d_char, L'^');
	/* Check that matching the color's full name is case-insensitive. */
	r = parser_parse(p, "graphics:%:light green");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	eq(t->d_attr, COLOUR_L_GREEN);
	eq(t->d_char, L'%');
	/* Try with a single letter code for the color. */
	r = parser_parse(p, "graphics:_:s");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	eq(t->d_attr, COLOUR_SLATE);
	eq(t->d_char, L'_');
	ok;
}

static int test_appear0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "appear:2:20:1");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	eq(t->rarity, 2);
	eq(t->min_depth, 20);
	eq(t->max_num, 1);
	ok;
}

static int test_visibility0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "visibility:5+2d10M50");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	eq(t->power.base, 5);
	eq(t->power.dice, 2);
	eq(t->power.sides, 10);
	eq(t->power.m_bonus, 50);
	ok;
}

static int test_visibility_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "visibility:1d6+1d10");

	eq(r, PARSE_ERROR_NOT_RANDOM);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct trap_kind *t = (struct trap_kind*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[TRF_SIZE];

	notnull(t);
	trf_wipe(t->flags);
	/* Try with no flags. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	require(trf_is_empty(t->flags));
	/* Try with a single flag. */
	r = parser_parse(p, "flags:TRAP");
	eq(r, PARSE_ERROR_NONE);
	/* Try with multiple flags at once. */
	r = parser_parse(p, "flags:FLOOR | VISIBLE");
	eq(r, PARSE_ERROR_NONE);
	trf_wipe(eflags);
	trf_on(eflags, TRF_TRAP);
	trf_on(eflags, TRF_FLOOR);
	trf_on(eflags, TRF_VISIBLE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	require(trf_is_equal(t->flags, eflags));
	ok;
}

static int test_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an unrecognized flag. */
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_missing_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	struct trap_kind *t = (struct trap_kind*) parser_priv(p);
	enum parser_error r;

	notnull(t);
	null(t->effect);
	/*
	 * Specifying effect details without and effect should not signal an
	 * error and leave the trap unmodified.
	 */
	r = parser_parse(p, "effect-yx:11:23");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:4+5d$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:S:DUNGEON_LEVEL:/ 10 + 2");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an effect that has no subtype, radius or other. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct trap_kind *t;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_DAMAGE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Try with an effect that has a subtype but no radius or other. */
	r = parser_parse(p, "effect:TIMED_INC:SLOW");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_TIMED_INC);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_SLOW);
	eq(e->radius, 0);
	eq(e->other, 0);
	/* Try with an effect that has a subtype and radius but no other. */
	r = parser_parse(p, "effect:SPOT:FIRE:1");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_FIRE);
	eq(e->radius, 1);
	eq(e->other, 0);
	/* Try with an effect that has a subtype, radius, and other. */
	r = parser_parse(p, "effect:SPOT:LIGHT_WEAK:2:10");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_LIGHT_WEAK);
	eq(e->radius, 2);
	eq(e->other, 10);
	ok;
}

static int test_effect_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an unrecognized effect. */
	enum parser_error r = parser_parse(p, "effect:XYZZY");

	eq(r, PARSE_ERROR_INVALID_EFFECT);
	/* Try with a recognized effect but an unrecognized subtype. */
	r = parser_parse(p, "effect:TIMED_INC:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_effect_yx0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct trap_kind *t;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-yx:11:23");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->y, 11);
	eq(e->x, 23);
	ok;
}

static int test_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct trap_kind *t;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:5+2d8M30");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->dice);
	require(dice_test_values(e->dice, 5, 2, 8, 30));
	ok;
}

static int test_dice_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:1d8+7");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_missing_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying an expression without dice should not flag an error and
	 * not modify the trap.
	 */
	r = parser_parse(p, "expr:S:DUNGEON_LEVEL:/ 5 + 2");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->effect);
	null(t->effect->dice);
	ok;
}

static int test_expr0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect with dice. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+5d$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:B:DUNGEON_LEVEL:/ 10 + 8");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_expr_bad0(void *state) {
	struct  parser *p = (struct parser*) state;
	/* Set up a new effect with dice. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+10d$S");
	/* Try an expression with invalid operations. */
	r = parser_parse(p, "expr:S:DUNGEON_LEVEL:^ 2");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/* Try to bind the expression to a variable that is not in the dice. */
	r = parser_parse(p, "expr:M:DUNGEON_LEVEL:/ 8 + 1");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_missing_effect_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	struct trap_kind *t = (struct trap_kind*) parser_priv(p);
	enum parser_error r;

	notnull(t);
	null(t->effect_xtra);
	/*
	 * Specifying effect details without and effect should not signal an
	 * error and leave the trap unmodified.
	 */
	r = parser_parse(p, "effect-yx-xtra:7:15");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice-xtra:$B+5d4");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr-xtra:B:DUNGEON_LEVEL:/ 10 + 8");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_effect_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an effect that has no subtype, radius or other. */
	enum parser_error r = parser_parse(p, "effect-xtra:WAKE");
	struct trap_kind *t;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect_xtra;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_WAKE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Try with an effect that has a subtype but no radius or other. */
	r = parser_parse(p, "effect-xtra:DRAIN_STAT:STR");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect_xtra;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_DRAIN_STAT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, STAT_STR);
	eq(e->radius, 0);
	eq(e->other, 0);
	/* Try with an effect that has a subtype and radius but no other. */
	r = parser_parse(p, "effect-xtra:SPOT:ACID:1");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect_xtra;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_ACID);
	eq(e->radius, 1);
	eq(e->other, 0);
	/* Try with an effect that has a subtype, radius, and other. */
	r = parser_parse(p, "effect-xtra:SPHERE:FIRE:4:5");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect_xtra;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_SPHERE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_FIRE);
	eq(e->radius, 4);
	eq(e->other, 5);
	ok;
}

static int test_effect_xtra_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an unrecognized effect. */
	enum parser_error r = parser_parse(p, "effect-xtra:XYZZY");

	eq(r, PARSE_ERROR_INVALID_EFFECT);
	/* Try with a recognized effect but an unrecognized subtype. */
	r = parser_parse(p, "effect-xtra:BOLT:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_effect_yx_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect-xtra:DAMAGE");
	struct trap_kind *t;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-yx-xtra:13:27");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect_xtra;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->y, 13);
	eq(e->x, 27);
	ok;
}

static int test_dice_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect-xtra:DAMAGE");
	struct trap_kind *t;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice-xtra:10+5d6");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	e = t->effect_xtra;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->dice);
	require(dice_test_values(e->dice, 10, 5, 6, 0));
	ok;
}

static int test_dice_xtra_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect-xtra:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice-xtra:1d6+1d8+1d12");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_missing_dice_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect. */
	enum parser_error r = parser_parse(p, "effect-xtra:DAMAGE");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying an expression without dice should not flag an error and
	 * not modify the trap.
	 */
	r = parser_parse(p, "expr-xtra:B:DUNGEON_LEVEL:* 2");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->effect_xtra);
	null(t->effect_xtra->dice);
	ok;
}

static int test_expr_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up a new effect with dice. */
	enum parser_error r = parser_parse(p, "effect-xtra:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice-xtra:$B+5d$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr-xtra:S:DUNGEON_LEVEL:/ 20 + 4");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_expr_xtra_bad0(void *state) {
	struct  parser *p = (struct parser*) state;
	/* Set up a new effect with dice. */
	enum parser_error r = parser_parse(p, "effect-xtra:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice-xtra:$B+4d$S");
	/* Try an expression with invalid operations. */
	r = parser_parse(p, "expr-xtra:B:DUNGEON_LEVEL:% 9");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/* Try to bind the expression to a variable that is not in the dice. */
	r = parser_parse(p, "expr-xtra:T:DUNGEON_LEVEL:+ 1");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_save0(void *state) {
	struct parser *p = (struct parser*) state;
	struct trap_kind *t = (struct trap_kind*) parser_priv(p);
	enum parser_error r;
	bitflag eflags[OF_SIZE];

	notnull(t);
	of_wipe(t->save_flags);
	/* Try with a single flag. */
	r = parser_parse(p, "save:FEATHER");
	eq(r, PARSE_ERROR_NONE);
	/* Try with multiple flags at once. */
	r = parser_parse(p, "save:FREE_ACT | HOLD_LIFE");
	eq(r, PARSE_ERROR_NONE);
	of_wipe(eflags);
	of_on(eflags, OF_FEATHER);
	of_on(eflags, OF_FREE_ACT);
	of_on(eflags, OF_HOLD_LIFE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	require(of_is_equal(t->save_flags, eflags));
	ok;
}

static int test_save_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an unrecognized flag. */
	enum parser_error r = parser_parse(p, "save:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:This weakened ceiling "
		"beam threatens to collapse at any moment.");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->desc);
	require(streq(t->text, "This weakened ceiling beam threatens to "
		"collapse at any moment."));
	/* Check that a second directive is appended to the first. */
	r = parser_parse(p, "desc:  You would prefer not to be nearby when "
		"it does.");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->desc);
	require(streq(t->text, "This weakened ceiling beam threatens to "
		"collapse at any moment.  You would prefer not to be nearby "
		"when it does."));
	ok;
}

static int test_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"msg:Blades whirl around you, slicing your skin!");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg);
	require(streq(t->msg, "Blades whirl around you, slicing your skin!"));
	/* Check that a second directive is appended to the first. */
	r = parser_parse(p,
		"msg:  The air is filled with a fine mist of blood.");
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg);
	require(streq(t->msg, "Blades whirl around you, slicing your "
		"skin!  The air is filled with a fine mist of blood."));
	ok;
}

static int test_msg_good0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"msg-good:You manage to spring to the side.");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg_good);
	require(streq(t->msg_good, "You manage to spring to the side."));
	/* Check that a second directive is appended to the first. */
	r = parser_parse(p,
		"msg-good:  The floor is not as lucky and shatters.");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg_good);
	require(streq(t->msg_good, "You manage to spring to the side.  The "
		"floor is not as lucky and shatters."));
	ok;
}

static int test_msg_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msg-bad:A small dart hits you!");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg_bad);
	require(streq(t->msg_bad, "A small dart hits you!"));
	/* Check that a second directive is appended to the first. */
	r = parser_parse(p,
		"msg-bad:  Numbness spreads from where it pricked you.");
	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg_bad);
	require(streq(t->msg_bad, "A small dart hits you!  Numbness spreads "
		"from where it pricked you."));
	ok;
}

static int test_msg_xtra0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msg-xtra:You are impaled!");
	struct trap_kind *t;

	eq(r, PARSE_ERROR_NONE);
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg_xtra);
	require(streq(t->msg_xtra, "You are impaled!"));
	/* Check that a second directive is appended to the first. */
	r = parser_parse(p, "msg-xtra:  And begin to bleed profusely.");
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->msg_xtra);
	require(streq(t->msg_xtra, "You are impaled!  And begin to bleed "
		"profusely."));
	ok;
}

static int test_complete0(void *state) {
	const char *lines[] = {
		"name:dart trap:slow dart",
		"graphics:^:r",
		"appear:1:2:0",
		"visibility:M50",
		"flags:TRAP | FLOOR | SAVE_ARMOR",
		"effect:DAMAGE",
		"dice:1d4",
		"effect:TIMED_INC_NO_RES:SLOW",
		"dice:20+1d20",
		"desc:A trap which shoots slowing darts.",
		"msg-good:A small dart barely misses you.",
		"msg-bad:A small dart hits you!"
	};
	struct parser *p = (struct parser*) state;
	bitflag tflags[TRF_SIZE];
	struct trap_kind *t;
	struct effect *e;
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	t = (struct trap_kind*) parser_priv(p);
	notnull(t);
	notnull(t->name);
	require(streq(t->name, "dart trap"));
	notnull(t->desc);
	require(streq(t->desc, "slow dart"));
	null(t->msg);
	notnull(t->msg_good);
	require(streq(t->msg_good, "A small dart barely misses you."));
	notnull(t->msg_bad);
	require(streq(t->msg_bad, "A small dart hits you!"));
	null(t->msg_xtra);
	eq(t->d_attr, COLOUR_RED);
	eq(t->d_char, L'^');
	eq(t->rarity, 1);
	eq(t->min_depth, 2);
	eq(t->max_num, 0);
	eq(t->power.base, 0);
	eq(t->power.dice, 0);
	eq(t->power.sides, 0);
	eq(t->power.m_bonus, 50);
	trf_wipe(tflags);
	trf_on(tflags, TRF_TRAP);
	trf_on(tflags, TRF_FLOOR);
	trf_on(tflags, TRF_SAVE_ARMOR);
	require(trf_is_equal(t->flags, tflags));
	require(of_is_empty(t->save_flags));
	e = t->effect;
	notnull(e);
	eq(e->index, EF_DAMAGE);
	notnull(e->dice);
	require(dice_test_values(e->dice, 0, 1, 4, 0));
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	e = e->next;
	notnull(e);
	eq(e->index, EF_TIMED_INC_NO_RES);
	notnull(e->dice);
	require(dice_test_values(e->dice, 20, 1, 20, 0));
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_SLOW);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	e = e->next;
	null(e);
	null(t->effect_xtra);
	ok;
}

const char *suite_name = "parse/partrap";
/*
 * test_missing_header_record0() has to be before test_name0() and
 * test_complete0().
 * test_missing_effect0() has to be after test_name0() and before
 * test_effect0(), test_effect_bad0(), test_effect_yx0(), test_dice0(),
 * test_dice_bad0(), test_missing_dice0(), test_expr0(), test_expr_bad0(),
 * and test_complete0().
 * test_missing_effect_xtra0() has to be after test_name0() and before
 * test_effect_xtra0(), test_effect_xtra_bad0(), test_effect_yx_xtra0(),
 * test_dice_xtra0(), test_dice_xtra_bad0(), test_missing_dice_xtra0(),
 * test_expr_xtra0(), test_expr_xtra_bad0(), and test_complete0().
 * test_graphics0(), test_appear0(), test_visibility0(),
 * test_visibility_bad0(), test_flags0(), test_flags_bad0(), test_effect0(),
 * test_effect_bad0(), test_effect_yx0(), test_dice0(), test_dice_bad0(),
 * test_missing_dice0(), test_expr0(), test_expr_bad0(), test_effect_xtra0(),
 * test_effect_xtra_bad0(), test_effect_yx_xtra0(), test_dice_xtra0(),
 * test_dice_xtra_bad0(), test_expr_xtra0(), test_expr_xtra_bad0(),
 * test_save0(), and test_save_bad0() have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_header_record0", test_missing_header_record0 },
	{ "name0", test_name0 },
	{ "graphics0", test_graphics0 },
	{ "appear0", test_appear0 },
	{ "visibility0", test_visibility0 },
	{ "visibility_bad0", test_visibility_bad0 },
	{ "flags0", test_flags0 },
	{ "flags_bad0", test_flags_bad0 },
	{ "missing_effect0", test_missing_effect0 },
	{ "effect0", test_effect0 },
	{ "effect_bad0", test_effect_bad0 },
	{ "effect_yx0", test_effect_yx0 },
	{ "dice0", test_dice0 },
	{ "dice_bad0", test_dice_bad0 },
	{ "missing_dice0", test_missing_dice0 },
	{ "expr0", test_expr0 },
	{ "expr_bad0", test_expr_bad0 },
	{ "missing_effect_xtra0", test_missing_effect_xtra0 },
	{ "effect_xtra0", test_effect_xtra0 },
	{ "effect_xtra_bad0", test_effect_xtra_bad0 },
	{ "effect_yx_xtra0", test_effect_yx_xtra0 },
	{ "dice_xtra0", test_dice_xtra0 },
	{ "dice_xtra_bad0", test_dice_xtra_bad0 },
	{ "missing_dice_xtra0", test_missing_dice_xtra0 },
	{ "expr_xtra0", test_expr_xtra0 },
	{ "expr_xtra_bad0", test_expr_xtra_bad0 },
	{ "save0", test_save0 },
	{ "save_bad0", test_save_bad0 },
	{ "desc0", test_desc0 },
	{ "msg0", test_msg0 },
	{ "msg_good0", test_msg_good0 },
	{ "msg_bad0", test_msg_bad0 },
	{ "msg_xtra0", test_msg_xtra0 },
	{ "complete0", test_complete0 },
	{ NULL, NULL }
};
