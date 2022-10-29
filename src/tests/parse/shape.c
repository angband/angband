/* parse/shape */
/* Exercise the parsing used for shape.txt. */

#include "unit-test.h"
#include "effects.h"
#include "init.h"
#include "object.h"
#include "player.h"
#include "player-timed.h"
#include "project.h"
#include "z-virt.h"

int setup_tests(void **state) {
	/* Parsing needs z_info. */
	z_info = mem_zalloc(sizeof(*z_info));
	*state = shape_parser.init();
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int r = 0;

	if (shape_parser.finish(p)) {
		r = 1;
	}
	shape_parser.cleanup();
	mem_free(z_info);
	return r;
}

static int test_missing_record_header0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r;

	null(parser_priv(p));
	r = parser_parse(p, "combat:-3:-4:2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-disarm-phys:-5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-disarm-magic:-10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-save:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-stealth:13");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-search:5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-melee:10");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-throw:-5");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "skill-dig:25");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "obj-flags:FEATHER | FREE_ACT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "player-flags:STEAL");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "values:SPEED[3] | STEALTH[3] | INFRA[5]");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:11:23");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:1d$S");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:S:PLAYER_LEVEL:+ 0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:turning into a bat");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "blow:bite");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	int eidx = z_info->shape_max;
	struct player_shape *s;
	enum parser_error r;
	int i;

	r = parser_parse(p, "name:fox");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	notnull(s->name);
	require(streq(s->name, "fox"));
	eq(s->sidx, eidx);
	eq(s->to_a, 0);
	eq(s->to_h, 0);
	eq(s->to_d, 0);
	for (i = 0; i < SKILL_MAX; ++i) {
		eq(s->skills[i], 0);
	}
	require(of_is_empty(s->flags));
	require(pf_is_empty(s->pflags));
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		eq(s->modifiers[i], 0);
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		eq(s->el_info[i].res_level, 0);
		eq(s->el_info[i].flags, 0);
	}
	null(s->effect);
	null(s->blows);
	eq(s->num_blows, 0);
	ok;
}

static int test_combat0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "combat:5:2:-2");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->to_a, -2);
	eq(s->to_h, 5);
	eq(s->to_d, 2);
	ok;
}

static int test_disarm_phys0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-disarm-phys:-5");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_DISARM_PHYS], -5);
	ok;
}

static int test_disarm_magic0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-disarm-magic:-10");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_DISARM_MAGIC], -10);
	ok;
}

static int test_save0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-save:20");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_SAVE], 20);
	ok;
}

static int test_stealth0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-stealth:-7");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_STEALTH], -7);
	ok;
}

static int test_search0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-search:12");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_SEARCH], 12);
	ok;
}

static int test_melee0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-melee:9");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_TO_HIT_MELEE], 9);
	ok;
}

static int test_throw0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-throw:3");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_TO_HIT_THROW], 3);
	ok;
}

static int test_dig0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "skill-dig:8");
	struct player_shape *s;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->skills[SKILL_DIGGING], 8);
	ok;
}

static int test_obj_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_shape *s = (struct player_shape*) parser_priv(p);
	bitflag eflags[OF_SIZE];
	enum parser_error r;

	notnull(s);
	of_wipe(s->flags);
	/* Try with no flags specified. */
	r = parser_parse(p, "obj-flags:");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	require(of_is_empty(s->flags));
	/* Try with one flag. */
	r = parser_parse(p, "obj-flags:FEATHER");
	eq(r, PARSE_ERROR_NONE);
	/* Try with two flags at once. */
	r = parser_parse(p, "obj-flags:FREE_ACT | REGEN");
	eq(r, PARSE_ERROR_NONE);
	of_wipe(eflags);
	of_on(eflags, OF_FEATHER);
	of_on(eflags, OF_FREE_ACT);
	of_on(eflags, OF_REGEN);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	require(of_is_equal(s->flags, eflags));
	ok;
}

static int test_obj_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized flag. */
	enum parser_error r = parser_parse(p, "obj-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_player_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_shape *s = (struct player_shape*) parser_priv(p);
	bitflag eflags[PF_SIZE];
	enum parser_error r;

	notnull(s);
	pf_wipe(s->pflags);
	/* Try with no flags specified. */
	r = parser_parse(p, "player-flags:");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	require(pf_is_empty(s->pflags));
	/* Try with one flag. */
	r = parser_parse(p, "player-flags:STEAL");
	eq(r, PARSE_ERROR_NONE);
	/* Try with two flags at once. */
	r = parser_parse(p, "player-flags:SEE_ORE | ROCK");
	eq(r, PARSE_ERROR_NONE);
	pf_wipe(eflags);
	pf_on(eflags, PF_STEAL);
	pf_on(eflags, PF_SEE_ORE);
	pf_on(eflags, PF_ROCK);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	require(pf_is_equal(s->pflags, eflags));
	ok;
}

static int test_player_flags_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try with an invalid player flag. */;
	enum parser_error r = parser_parse(p, "player-flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_values0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_shape *s = (struct player_shape*) parser_priv(p);
	enum parser_error r;
	int i;

	notnull(s);
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		s->modifiers[i] = 0;
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		s->el_info[i].res_level = 0;
		s->el_info[i].flags = 0;
	}
	/* Try with one modifier. */
	r = parser_parse(p, "values:STR[-2]");
	eq(r, PARSE_ERROR_NONE);
	/* Try with a resistand and a modifier at once. */
	r = parser_parse(p, "values:RES_POIS[3] | STEALTH[4]");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	for (i = 0; i < OBJ_MOD_MAX; ++i) {
		if (i == OBJ_MOD_STR) {
			eq(s->modifiers[i], -2);
		} else if (i == OBJ_MOD_STEALTH) {
			eq(s->modifiers[i], 4);
		} else {
			eq(s->modifiers[i], 0);
		}
	}
	for (i = 0; i < ELEM_MAX; ++i) {
		if (i == ELEM_POIS) {
			eq(s->el_info[i].res_level, 3);
		} else {
			eq(s->el_info[i].res_level, 0);
		}
		eq(s->el_info[i].flags, 0);
	}
	ok;
}

static int test_values_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an unrecognized modifier. */
	enum parser_error r = parser_parse(p, "values:XYZZY[5]");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	/* Try an unrecognized element. */
	r = parser_parse(p, "values:RES_XYZZY[1]");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_missing_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_shape *s = (struct player_shape*) parser_priv(p);
	enum parser_error r;

	notnull(s);
	null(s->effect);
	/*
	 * Trying to modify an effect that isn't present shouldn't return
	 * an error.
	 */
	r = parser_parse(p, "effect-yx:7:15");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	null(s->effect);
	r = parser_parse(p, "dice:7+2d$S");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	null(s->effect);
	r = parser_parse(p, "expr:S:PLAYER_LEVEL:* 2");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	null(s->effect);
	r = parser_parse(p, "effect-msg:turning into a vampire");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	null(s->effect);
	ok;
}

static int test_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Try an effect without a subtype, radius or other. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct player_shape *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
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
	/* Try an effect with a subtype but no radius or other. */
	r = parser_parse(p, "effect:CURE:POISONED");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_CURE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_POISONED);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	/* Try an effect with a subtype and radius but no other. */
	r = parser_parse(p, "effect:SPOT:LIGHT_WEAK:2");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_LIGHT_WEAK);
	eq(e->radius, 2);
	eq(e->other, 0);
	null(e->msg);
	/* Try an effect with a subtype, radius, and other. */
	r = parser_parse(p, "effect:TIMED_INC:SHERO:0:5");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->index, EF_TIMED_INC);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_SHERO);
	eq(e->radius, 0);
	eq(e->other, 5);
	null(e->msg);
	ok;
}

static int test_effect_yx0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct player_shape *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-yx:7:15");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	eq(e->y, 7);
	eq(e->x, 15);
	ok;
}

static int test_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct player_shape *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:5+6d7");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->dice);
	require(dice_test_values(e->dice, 5, 6, 7, 0));
	ok;
}

static int test_dice_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:2d8+2d10-3");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_missing_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct player_shape *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	/*
	 * Binding an expression when there is no dice should not return an
	 * error.
	 */
	r = parser_parse(p, "expr:S:PLAYER_LEVEL:/ 5 + 2");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	null(e->dice);
	ok;
}

static int test_expr0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect with dice. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+4d$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:* 2");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_expr_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect with dice. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+4d$S");
	eq(r, PARSE_ERROR_NONE);
	/* Try an expression with invalid operations. */
	r = parser_parse(p, "expr:B:PLAYER_LEVEL:% 5");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/* Try binding to a variable that isn't in the dice. */
	r = parser_parse(p, "expr:N:PLAYER_LEVEL:/ 5 + 1");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_effect_msg0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Set up an effect. */
	enum parser_error r = parser_parse(p, "effect:DAMAGE");
	struct player_shape *s;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-msg:turning into");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->msg);
	require(streq(e->msg, "turning into"));
	/* Check that a second directive appends to the first. */
	r = parser_parse(p, "effect-msg: a bat");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) e = e->next;
	notnull(e->msg);
	require(streq(e->msg, "turning into a bat"));
	ok;
}

static int test_blow0(void *state) {
	struct parser *p = (struct parser*) state;
	struct player_shape *s = (struct player_shape*) parser_priv(p);
	struct player_blow *b;
	enum parser_error r;
	int old_count, i;

	notnull(s);
	old_count = s->num_blows;
	require(old_count >= 0);
	r = parser_parse(p, "blow:bite");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "blow:bite");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "blow:sting");
	eq(r, PARSE_ERROR_NONE);
	s = (struct player_shape*) parser_priv(p);
	notnull(s);
	eq(s->num_blows, old_count + 3);
	b = s->blows;
	notnull(b);
	notnull(b->name);
	require(streq(b->name, "sting"));
	b = b->next;
	notnull(b);
	notnull(b->name);
	require(streq(b->name, "bite"));
	b = b->next;
	notnull(b);
	notnull(b->name);
	require(streq(b->name, "bite"));
	b = b->next;
	for (i = 0; i < old_count; ++i) {
		notnull(b);
		b = b->next;
	}
	null(b);
	ok;
}

const char *suite_name = "parser/shape";
/*
 * test_missing_record_header0() has to be before test_name0().
 * test_missing_effect0() has to be after test_name0() and before
 * test_effect0(), test_effect_yx0, test_dice0(), test_bad_dice0(),
 * test_expr0(), test_bad_expr0(), and test_effect_msg0().
 * test_combat0(), test_disarm_phys0(), test_disarm_magic0(), test_save0(),
 * test_stealth0(), test_search0(), test_melee0(), test_throw0(), test_dig0(),
 * test_obj_flags0(), test_obj_flags_bad0(), test_player_flags0(),
 * test_player_flags_bad0(), test_values0(), test_values_bad0(),
 * test_effect0(), test_effect_yx0(), test_dice0(), test_dice_bad0(),
 * test_missing_dice0(), test_expr0(), test_expr_bad0(), test_effect_msg0(),
 * and test_blow0() have to be after test_name0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "combat0", test_combat0 },
	{ "disarm_phys0", test_disarm_phys0 },
	{ "disarm_magic0", test_disarm_magic0 },
	{ "save0", test_save0 },
	{ "stealth0", test_stealth0 },
	{ "search0", test_search0 },
	{ "melee0", test_melee0 },
	{ "throw0", test_throw0 },
	{ "dig0", test_dig0 },
	{ "obj_flags0", test_obj_flags0 },
	{ "obj_flags_bad0", test_obj_flags_bad0 },
	{ "player_flags0", test_player_flags0 },
	{ "player_flags_bad0", test_player_flags_bad0 },
	{ "values0", test_values0 },
	{ "values_bad0", test_values_bad0 },
	{ "missing_effect0", test_missing_effect0 },
	{ "effect0", test_effect0 },
	{ "effect_yx0", test_effect_yx0 },
	{ "dice0", test_dice0 },
	{ "dice_bad0", test_dice_bad0 },
	{ "missing_dice0", test_missing_dice0 },
	{ "expr0", test_expr0 },
	{ "expr_bad0", test_expr_bad0 },
	{ "effect_msg0", test_effect_msg0 },
	{ "blow0", test_blow0 },
	{ NULL, NULL }
};
