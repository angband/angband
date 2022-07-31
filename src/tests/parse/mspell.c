/* parse/mspell */
/* Exercise the parsing used for monster_spell.txt. */

#include "unit-test.h"
#include "datafile.h"
#include "effects.h"
#include "message.h"
#include "monster.h"
#include "mon-init.h"
#include "parser.h"
#include "player-timed.h"
#include "project.h"
#include "z-color.h"
#include "z-dice.h"

int setup_tests(void **state) {
	*state = mon_spell_parser.init();
	return !*state;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser *) state;

	mon_spell_parser.finish(p);
	mon_spell_parser.cleanup();
	return 0;
}

/*
 * Check that supplying any of the other directives before specifying the name
 * works as expected.
 */
static int test_missing_record0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	null(s);
	r = parser_parse(p, "msgt:TELEPORT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "hit:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:10:15");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "dice:3+1d35");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "expr:D:SPELL_POWER:/ 8 + 1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "power-cutoff:15");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lore:cough up a hairball");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lore-color-base:Orange");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lore-color-resist:Yellow");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lore-color-immune:Light Green");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-vis:{name} cackles.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-invis:Something cackles.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-miss:{name} gestures but stumbles.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "message-save:Something brushes your cheek, but you seem unharmed.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);

	ok;
}

static int test_name_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:XYZZY");

	eq(r, PARSE_ERROR_INVALID_SPELL_NAME);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:BLINK");
	const struct monster_spell *s;

	eq(r, PARSE_ERROR_NONE);
	s = parser_priv(p);
	eq(s->index, RSF_BLINK);
	notnull(s->level);
	eq(s->level->power, 0);
	null(s->level->lore_desc);
	null(s->level->message);
	null(s->level->blind_message);
	null(s->level->miss_message);
	null(s->level->save_message);
	ok;
}

static int test_msgt0(void *state) {
	struct parser *p = (struct parser*) state;
	const struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	notnull(s);
	r = parser_parse(p, "msgt:TELEPORT");
	eq(r, PARSE_ERROR_NONE);
	eq(s->msgt, MSG_TELEPORT);
	ok;
}

static int test_msgt_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	const struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	notnull(s);
	r = parser_parse(p, "msgt:XYZZY");
	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	ok;
}

static int test_hit0(void *state) {
	struct parser *p = (struct parser*) state;
	const struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	notnull(s);
	r = parser_parse(p, "hit:100");
	eq(r, PARSE_ERROR_NONE);
	eq(s->hit, 100);
	ok;
}

/*
 * Check the placing "effect-yx:", "dice:", and "expr:' directives before
 * any "effect:" directives for a spell works as expected:  do nothing and
 * return PARSE_ERROR_NONE.
 */
static int test_misplaced_effect_deps0(void *state) {
	struct parser *p = (struct parser*) state;
	const struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	require(s && !s->effect);
	r = parser_parse(p, "effect-yx:8:7");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:5+1d4");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:D:SPELL_POWER:* 8 + 20");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	const struct monster_spell *s = parser_priv(p);
	enum parser_error r;
	const struct effect* e;

	notnull(s);

	/* Check effect where just the type matters. */
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_NONE);
	e = s->effect;
	notnull(e);
	while (e->next) {
		e = e->next;
	}
	eq(e->index, EF_DAMAGE);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	eq(e->y, 0);
	eq(e->x, 0);
	null(e->dice);
	null(e->msg);

	/* Check effect where there's a type and subtype. */
	r = parser_parse(p, "effect:TIMED_INC:CONFUSED");
	e = e->next;
	notnull(e);
	eq(e->index, EF_TIMED_INC);
	eq(e->subtype, TMD_CONFUSED);
	eq(e->radius, 0);
	eq(e->other, 0);
	eq(e->y, 0);
	eq(e->x, 0);
	null(e->dice);
	null(e->msg);

	/* Check effect with a type, subtype, and radius. */
	r = parser_parse(p, "effect:BALL:ACID:2");
	e = e->next;
	notnull(e);
	eq(e->index, EF_BALL);
	eq(e->subtype, PROJ_ACID);
	eq(e->radius, 2);
	eq(e->y, 0);
	eq(e->x, 0);
	null(e->dice);
	null(e->msg);

	/* Check effect with a type, subtype, radius, and other parameter. */
	r = parser_parse(p, "effect:BREATH:FIRE:10:30");
	e = e->next;
	notnull(e);
	eq(e->index, EF_BREATH);
	eq(e->subtype, PROJ_FIRE);
	eq(e->radius, 10);
	eq(e->other, 30);
	eq(e->y, 0);
	eq(e->x, 0);
	null(e->dice);
	null(e->msg);

	ok;
}

static int test_effect_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	notnull(s);

	/* Check bad effect name. */
	r = parser_parse(p, "effect:XYZZY");
	eq(r, PARSE_ERROR_INVALID_EFFECT);

	/* Check bad effect subtype. */
	r = parser_parse(p, "effect:CURE:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);

	ok;
}

static int test_effect_yx0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct effect *e;
	enum parser_error r;

	notnull(s);
	e = s->effect;
	notnull(e);
	while (e->next) {
		e = e->next;
	}

	r = parser_parse(p, "effect-yx:5:9");
	eq(r, PARSE_ERROR_NONE);
	eq(e->y, 5);
	eq(e->x, 9);
	ok;
}

static int test_dice0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct { const char *s; int base, ndice, nsides; } test_cases[] = {
		{ "dice:-1", -1, 0, 0 },
		{ "dice:8", 8, 0, 0 },
		{ "dice:d10", 0, 1, 10 },
		{ "dice:-1+d5", -1, 1, 5 },
		{ "dice:3+2d7", 3, 2, 7 },
	};
	int i;

	notnull(s);
	for (i = 0; i < (int) N_ELEMENTS(test_cases); ++i) {
		enum parser_error r = parser_parse(p, "effect:DAMAGE");
		struct effect *e;

		eq(r, PARSE_ERROR_NONE);
		e = s->effect;
		notnull(e);
		while (e->next) {
			e = e->next;
		}
		r = parser_parse(p, test_cases[i].s);
		eq(r, PARSE_ERROR_NONE);
		notnull(e->dice);
		require(dice_test_values(e->dice, test_cases[i].base,
			test_cases[i].ndice, test_cases[i].nsides, 0));
	}
	ok;
}

static int test_dice_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct { const char *s; } test_cases[] = {
		{ "dice:5+d8+d4" },
	};
	int i;

	notnull(s);
	for (i = 0; i < (int) N_ELEMENTS(test_cases); ++i) {
		enum parser_error r = parser_parse(p, "effect:DAMAGE");
		struct effect *e;

		eq(r, PARSE_ERROR_NONE);
		e = s->effect;
		notnull(e);
		while (e->next) {
			e = e->next;
		}
		r = parser_parse(p, test_cases[i].s);
		eq(r, PARSE_ERROR_INVALID_DICE);
	}
	ok;
}

static int test_expr0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct { const char *s; } test_cases[] = {
		{ "expr:B:MAX_SIGHT: " },
		{ "expr:D:SPELL_POWER:/ 10 + 1" },
		{ "expr:S:SPELL_POWER:* 2 + 3" },
	};
	enum parser_error r;
	struct effect *e;
	int i;

	notnull(s);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_NONE);
	e = s->effect;
	notnull(e);
	while (e->next) {
		e = e->next;
	}
	r = parser_parse(p, "dice:$B+$Dd$S");
	eq(r, PARSE_ERROR_NONE);
	for (i = 0; i < (int) N_ELEMENTS(test_cases); ++i) {
		r = parser_parse(p, test_cases[i].s);
		eq(r, PARSE_ERROR_NONE);
	}
	ok;
}

static int test_expr_bad0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	enum parser_error r;

	notnull(s);
	r = parser_parse(p, "effect:DAMAGE");
	eq(r, PARSE_ERROR_NONE);
	/*
	 * Using expr before the effect has dice specified currently does
	 * nothing.
	 */
	r = parser_parse(p, "expr:MAX_SIGHT:B:+ 1");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "dice:$B+$Dd$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "expr:C:SPELL_POWER:* 3 / 2");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	r = parser_parse(p, "expr:B:MAX_SIGHT:- 40000");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	r = parser_parse(p, "expr:D:SPELL_POWER:/ 0");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	r = parser_parse(p, "expr:S:MAX_SIGHT:% 2");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	ok;
}

static int test_cutoff0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	r = parser_parse(p, "power-cutoff:10");
	eq(r, PARSE_ERROR_NONE);
	l = s->level;
	notnull(s);
	while (l->next) {
		l = l->next;
	}
	eq(l->power, 10);
	null(l->lore_desc);
	null(l->message);
	null(l->blind_message);
	null(l->miss_message);
	null(l->save_message);
	ok;
}

static int test_lore_color0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	/*
	 * Test with full name for color, using different capitalizations
	 * since the name matching is case-insensitive.
	 */
	r = parser_parse(p, "lore-color-base:Orange");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr, COLOUR_ORANGE);
	r = parser_parse(p, "lore-color-base:yellow");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr, COLOUR_YELLOW);
	r = parser_parse(p, "lore-color-base:light red");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr, COLOUR_L_RED);
	/* Test with the single character abbreviation for a color. */
	r = parser_parse(p, "lore-color-base:o");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr, COLOUR_ORANGE);
	ok;
}

static int test_lore_color_resist0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	/*
	 * Test with full name for color, using different capitalizations
	 * since the name matching is case-insensitive.
	 */
	r = parser_parse(p, "lore-color-resist:Yellow");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_resist, COLOUR_YELLOW);
	r = parser_parse(p, "lore-color-resist:light green");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_resist, COLOUR_L_GREEN);
	r = parser_parse(p, "lore-color-resist:orange");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_resist, COLOUR_ORANGE);
	/* Test with the single character abbreviation for a color. */
	r = parser_parse(p, "lore-color-resist:G");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_resist, COLOUR_L_GREEN);
	ok;
}

static int test_lore_color_immune0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	/*
	 * Test with full name for color, using different capitalizations
	 * since the name matching is case-insensitive.
	 */
	r = parser_parse(p, "lore-color-immune:Light Green");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_immune, COLOUR_L_GREEN);
	r = parser_parse(p, "lore-color-immune:light purple");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_immune, COLOUR_L_PURPLE);
	r = parser_parse(p, "lore-color-immune:white");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_immune, COLOUR_WHITE);
	/* Test with the single character abbreviation for a color. */
	r = parser_parse(p, "lore-color-immune:u");
	eq(r, PARSE_ERROR_NONE);
	eq(l->lore_attr_immune, COLOUR_UMBER);
	ok;
}

static int test_lore0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	r = parser_parse(p, "lore:clean windows");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->lore_desc, "clean windows"));
	r = parser_parse(p, "lore: expertly");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->lore_desc, "clean windows expertly"));
	ok;
}

static int test_message_vis0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	r = parser_parse(p, "message-vis:${name} cackles");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->message, "${name} cackles"));
	r = parser_parse(p, "message-vis: evilly.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->message, "${name} cackles evilly."));
	ok;
}

static int test_message_invis0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	r = parser_parse(p, "message-invis:Something cackles");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->blind_message, "Something cackles"));
	r = parser_parse(p, "message-invis: evilly.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->blind_message, "Something cackles evilly."));
	ok;
}

static int test_message_miss0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	r = parser_parse(p, "message-miss:${name} gestures");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->miss_message, "${name} gestures"));
	r = parser_parse(p, "message-miss: but then stumbles.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->miss_message, "${name} gestures but then stumbles."));
	ok;
}

static int test_message_save0(void *state) {
	struct parser *p = (struct parser*) state;
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	enum parser_error r;

	notnull(s);
	l = s->level;
	notnull(l);
	while (l->next) {
		l = l->next;
	}
	r = parser_parse(p, "message-save:You duck");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->save_message, "You duck"));
	r = parser_parse(p, "message-save: and are shaken but unharmed.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(l->save_message,
		"You duck and are shaken but unharmed."));
	ok;
}

const char *suite_name = "parse/mspell";
/*
 * test_missing_record0() has to be first.  test_name0() has to be before any
 * of the other tests except for test_name_bad0() and test_missing_record0().
 * test_name_bad0() should be before test_name0() and after
 * test_missing_record0() or after any of the other tests that depend on
 * test_name0() (all except test_missing_record0()).
 * test_misplaced_effect_deps0() has to be before test_effect0()
 * and test_effect_bad0().  test_effect_yx0() has to be after test_effect0().
 */
struct test tests[] = {
	{ "missing_record0", test_missing_record0 },
	{ "name_bad0", test_name_bad0 },
	{ "name0", test_name0 },
	{ "msgt0", test_msgt0 },
	{ "msgt_bad0", test_msgt_bad0 },
	{ "hit0", test_hit0 },
	{ "misplaced_effect_deps0", test_misplaced_effect_deps0 },
	{ "effect0", test_effect0 },
	{ "effect_bad0", test_effect_bad0 },
	{ "effect_yx0", test_effect_yx0 },
	{ "dice0", test_dice0 },
	{ "dice_bad0", test_dice_bad0 },
	{ "expr0", test_expr0 },
	{ "expr_bad0", test_expr_bad0 },
	{ "cutoff0", test_cutoff0 },
	{ "lore_color0", test_lore_color0 },
	{ "lore_color_resist0", test_lore_color_resist0 },
	{ "lore_color_immune0", test_lore_color_immune0 },
	{ "lore0", test_lore0 },
	{ "message_vis0", test_message_vis0 },
	{ "message_invis0", test_message_invis0 },
	{ "message_miss0", test_message_miss0 },
	{ "message_save0", test_message_save0 },
	{ NULL, NULL }
};
