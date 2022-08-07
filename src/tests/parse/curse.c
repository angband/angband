/* parse/curse */
/* Exercise parsing used for curse.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "effects.h"
#include "init.h"
#include "object.h"
#include "obj-init.h"
#include "player.h"
#include "player-timed.h"
#include "project.h"
#include "z-virt.h"

int setup_tests(void **state) {
	*state = curse_parser.init();
	/* Needed by curse_parser.finish. */
	z_info = mem_zalloc(sizeof(*z_info));
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (curse_parser.finish(p)) {
		r = 1;
	}
	curse_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	struct curse *c = (struct curse*) parser_priv(p);
	enum parser_error r;

	null(c);
	r = parser_parse(p, "type:cloak");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "combat:-5:-8:-15");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:TELEPORT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:7:9");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:$B+1d10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:B:WEAPON_DAMAGE:+ 0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msg:Your weapon turns on you!");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "time:50+1d50");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:AGGRAVATE | NO_TELEPORT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "values:SPEED[2] | STEALTH[-10]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "desc:makes your movements quicker but noisier");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "conflict:anti-teleportation");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "conflict-flags:NO_TELEPORT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:dullness");
	struct curse *c;
	int i;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	require(streq(c->name, "dullness"));
	for (i = 0; i < TV_MAX; ++i) {
		eq(c->poss[i], false);
	}
	notnull(c->obj);
	eq(c->obj->to_a, 0);
	eq(c->obj->to_h, 0);
	eq(c->obj->to_d, 0);
	require(of_is_empty(c->obj->flags));
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		eq(c->obj->modifiers[i], 0);
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(c->obj->el_info[i].res_level, 0);
	}
	null(c->obj->effect);
	null(c->obj->effect_msg);
	eq(c->obj->time.base, 0);
	eq(c->obj->time.dice, 0);
	eq(c->obj->time.sides, 0);
	eq(c->obj->time.m_bonus, 0);
	null(c->conflict);
	require(of_is_empty(c->conflict_flags));
	null(c->desc);
	ok;
}

static int test_type0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type:cloak");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->poss);
	eq(c->poss[TV_CLOAK], true);
	ok;
}

static int test_type_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "type:xyzzy");

	eq(r, PARSE_ERROR_UNRECOGNISED_TVAL);
	ok;
}

static int test_combat0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "combat:1:-2:3");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	eq(c->obj->to_h, 1);
	eq(c->obj->to_d, -2);
	eq(c->obj->to_a, 3);
	ok;
}

static int test_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct curse *c;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_DAMAGE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Check effect with type and subtype. */
	r = parser_parse(p, "effect:TIMED_INC:POISONED");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_TIMED_INC);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_POISONED);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Check effect with type, subtype, and radius. */
	r = parser_parse(p, "effect:SPOT:FIRE:3");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_FIRE);
	eq(e->radius, 3);
	eq(e->other, 0);
	null(e->msg);
	/* Check effect with type, subtype, radius, and other. */
	r = parser_parse(p, "effect:SPOT:DARK_WEAK:2:5");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_DARK_WEAK);
	eq(e->radius, 2);
	eq(e->other, 5);
	null(e->msg);
	ok;
}

static int test_effect_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check with unrecognized effect. */
	enum parser_error r = parser_parse(p, "effect:XYZZY");

	eq(r, PARSE_ERROR_INVALID_EFFECT);
	/* Check with bad subtype. */
	r = parser_parse(p, "effect:TIMED_INC:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_effect_yx0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect. */
	enum parser_error r = parser_parse(p, "effect:MAP_AREA");
	struct curse *c;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-yx:6:8");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	eq(e->y, 6);
	eq(e->x, 8);
	ok;
}

static int test_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect. */
	enum parser_error r = parser_parse(p, "effect:TIMED_INC:POISONED");
	struct curse *c;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:8+2d10");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	notnull(e->dice);
	eq(dice_test_values(e->dice, 8, 2, 10, 0), true);
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "dice:-4+4d8");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->dice);
	eq(dice_test_values(e->dice, -4, 4, 8, 0), true);
	ok;
}

static int test_dice_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:a");
	eq(r, PARSE_ERROR_INVALID_DICE);
	r = parser_parse(p, "dice:10+8d-1");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_expr0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect with dice. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+$Dd$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:B:PLAYER_HP:/ 8");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_expr_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up effect with dice. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:10+$Ad6");
	eq(r, PARSE_ERROR_NONE);
	/* Try an expression with an invalid operations string. */
	r = parser_parse(p, "expr:A:PLAYER_LEVEL:% 8");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/* Try to bind an expression to a variable that isn't in the dice. */
	r = parser_parse(p, "expr:B:DUNGEON_LEVEL:+ 0");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msg:Your equipment grabs you!");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect_msg);
	require(streq(c->obj->effect_msg, "Your equipment grabs you!"));
	/* Check that multiple msg directives are concatenated. */
	r = parser_parse(p, "msg: And doesn't let go!");
	eq(r, PARSE_ERROR_NONE);
	notnull(c->obj);
	notnull(c->obj->effect_msg);
	require(streq(c->obj->effect_msg,
		"Your equipment grabs you! And doesn't let go!"));
	ok;
}

static int test_time0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "time:9+10d8");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	eq(c->obj->time.base, 9);
	eq(c->obj->time.dice, 10);
	eq(c->obj->time.sides, 8);
	eq(c->obj->time.m_bonus, 0);
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "flags:AGGRAVATE");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "flags:HATES_FIRE | IGNORE_ACID");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	require(of_has(c->obj->flags, OF_AGGRAVATE));
	require((c->obj->el_info[ELEM_FIRE].flags & EL_INFO_HATES) != 0);
	require((c->obj->el_info[ELEM_ACID].flags & EL_INFO_IGNORE) != 0);
	ok;
}

static int test_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "flags:IGNORE_XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_values0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "values:SPEED[-2]");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "values:STEALTH[4] | RES_ELEC[3]");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	eq(c->obj->modifiers[OBJ_MOD_SPEED], -2);
	eq(c->obj->modifiers[OBJ_MOD_STEALTH], 4);
	eq(c->obj->el_info[ELEM_ELEC].res_level, 3);
	ok;
}

static int test_values_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "values:XYZZY[-8]");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "values:RES_XYZZY[-1]");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:makes you frail");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->desc);
	require(streq(c->desc, "makes you frail"));
	/* Check that multiple desc directives are concatenated. */
	r = parser_parse(p, "desc: and clumsy");
	eq(r, PARSE_ERROR_NONE);
	notnull(c->desc);
	require(streq(c->desc, "makes you frail and clumsy"));
	ok;
}

static int test_conflict0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "conflict:chilled to the bone");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->conflict);
	require(streq(c->conflict, "|chilled to the bone|"));
	/* Check that multiple conflict directives are appended. */
	r = parser_parse(p, "conflict:burning up");
	eq(r, PARSE_ERROR_NONE);
	notnull(c->conflict);
	require(streq(c->conflict, "|chilled to the bone||burning up|"));
	ok;
}

static int test_conflict_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "conflict-flags:AFRAID");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	/* Check that multiple conflict-flags directives add to the flags. */
	r = parser_parse(p, "conflict-flags:PROT_FEAR | NO_TELEPORT");
	eq(r, PARSE_ERROR_NONE);
	require(of_has(c->conflict_flags, OF_AFRAID));
	require(of_has(c->conflict_flags, OF_PROT_FEAR));
	require(of_has(c->conflict_flags, OF_NO_TELEPORT));
	ok;
}

static int test_conflict_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "conflict-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_combined0(void *state)
{
	const char *lines[] = {
		"name:the body is willing but the mind is weak",
		"type:helm",
		"effect:CURE:POISONED",
		"effect:TIMED_DEC:CUT",
		"dice:20",
		"effect:RESTORE_STAT:STR",
		"effect:RESTORE_STAT:CON",
		"effect:DRAIN_MANA",
		"dice:15",
		"effect:TIMED_INC:CONFUSED",
		"dice:20+1d20",
		"time:99+1d100",
		"conflict:sickliness",
		"conflict:poison",
		"conflict-flags:SUST_STR | SUST_CON",
		"msg:Your body feels invigorated while your mind descends into a fog.",
		"desc:periodically strengthens the body while weakening the mind"
	};
	struct parser *p = (struct parser*) state;
	struct curse *c;
	struct effect *e;
	bitflag flags[OF_SIZE];
	int i;

	for (i = 0; i < (int) N_ELEMENTS(lines); ++i) {
		enum parser_error r = parser_parse(p, lines[i]);

		eq(r, PARSE_ERROR_NONE);
	}
	c = (struct curse*) parser_priv(p);
	notnull(c);
	require(streq(c->name, "the body is willing but the mind is weak"));
	notnull(c->poss);
	for (i = 0; i < TV_MAX; ++i) {
		eq(c->poss[i], (i == TV_HELM));
	}
	notnull(c->obj);
	eq(c->obj->to_h, 0);
	eq(c->obj->to_d, 0);
	eq(c->obj->to_a, 0);
	require(of_is_empty(c->obj->flags));
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(c->obj->el_info[i].res_level, 0);
		eq(c->obj->el_info[i].flags, 0);
	}
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		eq(c->obj->modifiers[i], 0);
	}
	eq(c->obj->time.base, 99);
	eq(c->obj->time.dice, 1);
	eq(c->obj->time.sides, 100);
	eq(c->obj->time.m_bonus, 0);
	notnull(c->obj->effect);
	e = c->obj->effect;
	eq(e->index, EF_CURE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_POISONED);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_TIMED_DEC);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 20, 0, 0, 0), true);
	eq(e->subtype, TMD_CUT);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_RESTORE_STAT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, STAT_STR);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_RESTORE_STAT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, STAT_CON);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_DRAIN_MANA);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 15, 0, 0, 0), true);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	notnull(e->next);
	e = e->next;
	eq(e->index, EF_TIMED_INC);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 20, 1, 20, 0), true);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_CONFUSED);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	null(e->next);
	notnull(c->obj->effect_msg);
	require(streq(c->obj->effect_msg, "Your body feels invigorated "
		"while your mind descends into a fog."));
	notnull(c->conflict);
	require(streq(c->conflict, "|sickliness||poison|"));
	of_wipe(flags);
	of_on(flags, OF_SUST_STR);
	of_on(flags, OF_SUST_CON);
	require(of_is_equal(flags, c->conflict_flags));
	notnull(c->desc);
	require(streq(c->desc,
		"periodically strengthens the body while weakening the mind"));
	ok;
}

static int test_missing_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up curse with no effect. */
	enum parser_error r = parser_parse(p, "name:stinky breath");
	struct curse *c;

	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	/*
	 * Using effect-yx without a preceding effect should do nothing and
	 * not flag an error.
	 */
	r = parser_parse(p, "effect-yx:5:10");
	eq(r, PARSE_ERROR_NONE);
	notnull(c->obj);
	null(c->obj->effect);
	/*
	 * Using dice without a preceding effect should do nothing and not
	 * flag an error.
	 */
	r = parser_parse(p, "dice:$B+2d4");
	eq(r, PARSE_ERROR_NONE);
	notnull(c->obj);
	null(c->obj->effect);
	/*
	 * Using expr without a preceding effect should do nothing and not
	 * flag an error.
	 */
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:* 3 - 2");
	eq(r, PARSE_ERROR_NONE);
	notnull(c->obj);
	null(c->obj->effect);
	ok;
}

static int test_missing_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up curse with an effect but no dice. */
	enum parser_error r = parser_parse(p, "name:clumsy feet");
	struct curse *c;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Specifying an expression without preceding dice should do nothing
	 * and not flag an error.
	 */
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:+ 0");
	eq(r, PARSE_ERROR_NONE);
	c = (struct curse*) parser_priv(p);
	notnull(c);
	notnull(c->obj);
	notnull(c->obj->effect);
	e = c->obj->effect;
	while (e->next) e = e->next;
	eq(e->index, EF_DAMAGE);
	null(e->dice);
	ok;
}

const char *suite_name = "parse/curse";
/*
 * test_missing_record_header0() has to be before test_name0(),
 * test_combined0(), test_missing_effect0(), and test_missing_dice0().
 * test_type0(), test_type_bad0(), test_combat0(), test_effect0(),
 * test_effect_bad0(), test_effect_yx0(), test_dice0(), test_dice_bad0(),
 * test_expr0(), test_expr_bad0(), test_msg0(), test_time0(), test_flags0(),
 * test_flags_bad0(), test_values0(), test_values_bad0(), test_desc(),
 * test_conflict0(), test_conflict_flags0(), and test_conflict_flags_bad0()
 * have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "type0", test_type0 },
	{ "type_bad0", test_type_bad0 },
	{ "combat0", test_combat0 },
	{ "effect0", test_effect0 },
	{ "effect_bad0", test_effect_bad0 },
	{ "effect_yx0", test_effect_yx0 },
	{ "dice0", test_dice0 },
	{ "dice_bad0", test_dice_bad0 },
	{ "expr0", test_expr0 },
	{ "expr_bad0", test_expr_bad0 },
	{ "msg0", test_msg0 },
	{ "time0", test_time0 },
	{ "flags0", test_flags0 },
	{ "flags_bad0", test_flags_bad0 },
	{ "values0", test_values0 },
	{ "values_bad0", test_values_bad0 },
	{ "desc0", test_desc0 },
	{ "conflict0", test_conflict0 },
	{ "conflict_flags0", test_conflict_flags0 },
	{ "conflict_flags_bad0", test_conflict_flags_bad0 },
	{ "combined0", test_combined0 },
	{ "missing_effect", test_missing_effect0 },
	{ "missing_dice", test_missing_dice0 },
	{ NULL, NULL }
};
