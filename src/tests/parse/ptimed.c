/* parse/ptimed.c */
/* Exercise the parser used for player_timed.txt. */

#include "unit-test.h"
#include "effects.h"
#include "init.h"
#include "message.h"
#include "object.h"
#include "obj-slays.h"
#include "parser.h"
#include "player.h"
#include "player-timed.h"
#include "project.h"
#include "z-color.h"
#include "z-rand.h"
#include "z-virt.h"
#include <ctype.h>

static const char* const test_brands[] = { "ACID_3", "ELEC_2", "POIS_2" };
static const char* const test_slays[] = {
	"ANIMAL_2", "ORC_3", "GIANT_3", "UNDEAD_3"
};

int setup_tests(void **state) {
	struct parser *p = (*player_timed_parser.init)();
	int i;

	if (!p) {
		(void) teardown_tests(p);
		return 1;
	}
	/* Need z_info for food value and number of slays and brands. */
	z_info = mem_zalloc(sizeof(*z_info));
	if (!z_info) {
		(void) teardown_tests(p);
		return 1;
	}
	z_info->food_value = 100;
	/* Set up minimal slay and brand arrays for testing. */
	z_info->brand_max = (int) N_ELEMENTS(test_brands) + 1;
	brands = mem_zalloc(z_info->brand_max * sizeof(*brands));
	if (!brands) {
		(void) teardown_tests(p);
		return 1;
	}
	for (i = 1; i < z_info->brand_max; ++i) {
		brands[i].code = string_make(test_brands[i - 1]);
	}
	z_info->slay_max = (int) N_ELEMENTS(test_slays) + 1;
	slays = mem_zalloc(z_info->slay_max * sizeof(*slays));
	if (!slays) {
		(void) teardown_tests(p);
		return 1;
	}
	for (i = 1; i < z_info->slay_max; ++i) {
		slays[i].code = string_make(test_slays[i - 1]);
	}
	Rand_init();
	*state = p;
	return 0;
}

int teardown_tests(void *state) {
	struct parser *p = (struct parser*) state;
	int i;

	(void) (*player_timed_parser.finish)(p);
	(*player_timed_parser.cleanup)();
	for (i = 1; i < z_info->brand_max; ++i) {
		string_free(brands[i].code);
	}
	mem_free(brands);
	brands = NULL;
	for (i = 1; i < z_info->slay_max; ++i) {
		string_free(slays[i].code);
	}
	mem_free(slays);
	slays = NULL;
	mem_free(z_info);
	z_info = NULL;
	return 0;
}

static void clear_grades(struct timed_effect_data* t) {
	struct timed_grade *g = t->grade;

	while (g) {
		struct timed_grade *tgt = g;

		g = g->next;
		string_free(tgt->name);
		string_free(tgt->up_msg);
		string_free(tgt->down_msg);
		mem_free(tgt);
	}
	t->grade = NULL;
}

static int test_missing_record_header0(void *state)
{
	struct parser *p = (struct parser*) state;
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	enum parser_error r;

	notnull(ps);
	null(ps->t);
	r = parser_parse(p, "desc:haste");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "on-end:You feel yourself slow down.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "on-increase:You are more confused!");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "on-decrease:You feel a little less confused.");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "msgt:SPEED");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "fail:1:FREE_ACT");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p,
		"grade:G:10000:Haste:You feel yourself moving faster!");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "resist:ACID");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "brand:ACID_3");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "slay:ANIMAL_2");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flag-synonym:PROT_FEAR:0");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "on-begin-effect:SCRAMBLE_STATS");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "on-end-effect:UNSCRAMBLE_STATS");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-yx:10:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-dice:2d20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-expr:B:PLAYER_HP:/ 4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:despair");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "flags:NONSTACKING");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "lower-bound:1");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_name0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:FOOD");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	eq(ps->t, timed_effects + TMD_FOOD);
	notnull(ps->t->name);
	require(streq(ps->t->name, "FOOD"));
	null(ps->t->desc);
	null(ps->t->on_end);
	null(ps->t->on_increase);
	null(ps->t->on_decrease);
	eq(ps->t->msgt, 0);
	null(ps->t->fail);
	null(ps->t->on_begin_effect);
	null(ps->t->on_end_effect);
	eq(ps->t->flags, 0);
	eq(ps->t->lower_bound, 0);
	eq(ps->t->oflag_dup, OF_NONE);
	eq(ps->t->oflag_syn, false);
	eq(ps->t->temp_resist, -1);
	eq(ps->t->temp_brand, -1);
	eq(ps->t->temp_slay, -1);
	ok;
}

static int test_badname0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:XYZZY");

	noteq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_desc0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "desc:nourishment");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t)
	notnull(ps->t->desc);
	require(streq(ps->t->desc, "nourishment"));
	r = parser_parse(p, "desc: (i.e. food)");
	eq(r, PARSE_ERROR_NONE);
	require(streq(ps->t->desc, "nourishment (i.e. food)"));
	ok;
}

static int test_endmsg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"on-end:You no longer feel safe from evil!");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_end);
	require(streq(ps->t->on_end, "You no longer feel safe from evil!"));
	r = parser_parse(p, "on-end:  They'll be after you soon.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(ps->t->on_end, "You no longer feel safe from evil!  "
		"They'll be after you soon."));
	ok;
}

static int test_incmsg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"on-increase:You feel even safer from evil!");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	require(streq(ps->t->on_increase, "You feel even safer from evil!"));
	r = parser_parse(p,
		"on-increase:  And the shadows seem to lighten and shrink.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(ps->t->on_increase, "You feel even safer from evil!  "
		"And the shadows seem to lighten and shrink."));
	ok;
}

static int test_decmsg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p,
		"on-decrease:You feel less safe from evil!");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_decrease);
	require(streq(ps->t->on_decrease, "You feel less safe from evil!"));
	r = parser_parse(p,
		"on-decrease:  And the shadows seem to lengthen and darken.");
	eq(r, PARSE_ERROR_NONE);
	require(streq(ps->t->on_decrease, "You feel less safe from evil!  "
		"And the shadows seem to lengthen and darken."));
	ok;
}

static int test_msgt0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msgt:HUNGRY");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	eq(ps->t->msgt, MSG_HUNGRY);
	ok;
}

static int test_badmsgt0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "msgt:XYZZY");

	eq(r, PARSE_ERROR_INVALID_MESSAGE);
	ok;
}

static int test_fail0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "fail:1:FREE_ACT");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "fail:2:FIRE");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "fail:3:POIS");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "fail:4:NO_MANA");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "fail:5:SLOW");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->fail);
	eq(ps->t->fail->code, TMD_FAIL_FLAG_TIMED_EFFECT);
	eq(ps->t->fail->idx, TMD_SLOW);
	notnull(ps->t->fail->next);
	eq(ps->t->fail->next->code, TMD_FAIL_FLAG_PLAYER);
	eq(ps->t->fail->next->idx, PF_NO_MANA);
	notnull(ps->t->fail->next->next);
	eq(ps->t->fail->next->next->code, TMD_FAIL_FLAG_VULN);
	eq(ps->t->fail->next->next->idx, ELEM_POIS);
	notnull(ps->t->fail->next->next->next);
	eq(ps->t->fail->next->next->next->code, TMD_FAIL_FLAG_RESIST);
	eq(ps->t->fail->next->next->next->idx, ELEM_FIRE);
	notnull(ps->t->fail->next->next->next->next);
	eq(ps->t->fail->next->next->next->next->code, TMD_FAIL_FLAG_OBJECT);
	eq(ps->t->fail->next->next->next->next->idx, OF_FREE_ACT);
	ok;
}

static int test_badfail0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "fail:1:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "fail:2:XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "fail:3:XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "fail:4:XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "fail:5:XYZZY");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "fail:6:FIRE");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	r = parser_parse(p, "fail:10:FIRE");
	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_grade0(void *state) {
	struct parser *p = (struct parser*) state;
	struct {
		const char *name;
		const char *up_msg;
		const char *down_msg;
		int *p_py_food;
		int max;
		int color;
	} test_grades[] = {
		{
			"Starving",
			NULL,
			"You are starving!!",
			&PY_FOOD_STARVE,
			1,
			COLOUR_L_RED
		},
		{
			"Faint",
			"You are still faint.",
			"You are getting faint from hunger!",
			&PY_FOOD_FAINT,
			4,
			COLOUR_RED
		},
		{
			"Weak",
			"You are still weak.",
			"You are getting weak from hunger!",
			&PY_FOOD_WEAK,
			8,
			COLOUR_ORANGE
		},
		{
			"Hungry",
			"You are still hungry.",
			"You are getting hungry.",
			&PY_FOOD_HUNGRY,
			15,
			COLOUR_YELLOW
		},
		{
			"Fed",
			"You are no longer hungry.",
			"You are no longer full.",
			&PY_FOOD_FULL,
			90,
			COLOUR_L_GREEN,
		},
		{
			"Full",
			"You are full!",
			NULL,
			&PY_FOOD_MAX,
			100,
			COLOUR_GREEN
		},
		{
			NULL,
			NULL,
			NULL,
			NULL,
			200,
			COLOUR_PURPLE
		},
	};
	char buffer[256], color[32];
	enum parser_error r;
	struct timed_effect_parse_state *ps;
	const struct timed_grade *last_grade;
	int i, scale;

	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	clear_grades(ps->t);

	for (i = 0; i < (int) N_ELEMENTS(test_grades); ++i) {
		/*
		 * Use the one letter code for the color half of the time.
		 * Otherwise, use either the mixed case, all lower case, or
		 * all upper case version of the full name.
		 */
		if (one_in_(2)) {
			(void) strnfmt(color, sizeof(color), "%c",
				color_table[test_grades[i].color].index_char);
		} else {
			int j;

			(void) my_strcpy(color,
				color_table[test_grades[i].color].name,
				sizeof(color));
			if (one_in_(3)) {
				for (j = 0; color[j]; ++j) {
					if (isupper(color[j])) {
						color[j] = tolower(color[j]);
					}
				}
			} else if (one_in_(3)) {
				for (j = 0; color[j]; ++j) {
					if (islower(color[j])) {
						color[j] = toupper(color[j]);
					}
				}
			}
		}
		if (test_grades[i].down_msg) {
			(void) strnfmt(buffer, sizeof(buffer),
				"grade:%s:%d:%s:%s:%s",
				color,
				test_grades[i].max,
				(test_grades[i].name) ? test_grades[i].name : " ",
				(test_grades[i].up_msg) ? test_grades[i].up_msg : " ",
				test_grades[i].down_msg);
		} else if (one_in_(2)) {
			/*
			 * Test that a trailing colon with nothing after it
			 * works for the optional down message.
			 */
			(void) strnfmt(buffer, sizeof(buffer),
				"grade:%s:%d:%s:%s:",
				color,
				test_grades[i].max,
				(test_grades[i].name) ? test_grades[i].name : " ",
				(test_grades[i].up_msg) ? test_grades[i].up_msg : " ");
		} else {
			/*
			 * Test that omitting the down message entirely works.
			 */
			(void) strnfmt(buffer, sizeof(buffer),
				"grade:%s:%d:%s:%s",
				color,
				test_grades[i].max,
				(test_grades[i].name) ? test_grades[i].name : " ",
				(test_grades[i].up_msg) ? test_grades[i].up_msg : " ");
		}

		r = parser_parse(p, buffer);
		eq(r, PARSE_ERROR_NONE);
	}

	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	scale = (ps->t->name && streq(ps->t->name, "FOOD"))
		? z_info->food_value : 1;
	last_grade = ps->t->grade;
	notnull(last_grade);
	/* Skip the zero grade at the start. */
	last_grade = last_grade->next;
	for (i = 0; i < (int) N_ELEMENTS(test_grades); ++i) {
		notnull(last_grade);
		eq(i + 1, last_grade->grade);
		eq(test_grades[i].color, last_grade->color);
		eq(test_grades[i].max * scale, last_grade->max);
		if (test_grades[i].name) {
			require(streq(test_grades[i].name, last_grade->name));
		} else {
			null(last_grade->name);
		}
		if (test_grades[i].up_msg) {
			require(streq(test_grades[i].up_msg,
				last_grade->up_msg));
		} else {
			null(last_grade->up_msg);
		}
		if (test_grades[i].down_msg) {
			require(streq(test_grades[i].down_msg,
				last_grade->down_msg));
		} else {
			null(last_grade->down_msg);
		}
		last_grade = last_grade->next;
		if (test_grades[i].p_py_food) {
			eq(*test_grades[i].p_py_food,
				test_grades[i].max * scale);
		}
	}

	ok;
}

static int test_badgrade0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "name:FAST");

	eq(r, PARSE_ERROR_NONE);
	/* Try with out of bounds values for the grade maximum. */
	r = parser_parse(p, "grade:G:-1:Haste:Grade maximum below zero");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "grade:G:32768:Haste:Grade maximum too larage");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	/* Try with non-increasing values for the grade maximums. */
	r = parser_parse(p, "grade:G:50:Haste:Valid grade");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "grade:R:25:Haste:Grade out of order");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_resist0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "resist:COLD");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	eq(ps->t->temp_resist, ELEM_COLD);
	ok;
}

static int test_badresist0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "resist:XYZZY");

	noteq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_brand0(void *state) {
	struct parser *p = (struct parser*) state;
	char buffer[128];
	enum parser_error r;
	struct timed_effect_parse_state *ps;
	int i;

	for (i = (int) N_ELEMENTS(test_brands) - 1; i >= 0; --i) {
		(void) strnfmt(buffer, sizeof(buffer), "brand:%s",
			 test_brands[i]);
		r = parser_parse(p, buffer);
		eq(r, PARSE_ERROR_NONE);
		ps = (struct timed_effect_parse_state*) parser_priv(p);
		notnull(ps);
		notnull(ps->t);
		require(streq(test_brands[i], brands[ps->t->temp_brand].code));
	}
	ok;
}

static int test_badbrand0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "brand:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_BRAND);
	ok;
}

static int test_slay0(void *state) {
	struct parser *p = (struct parser*) state;
	char buffer[128];
	enum parser_error r;
	struct timed_effect_parse_state *ps;
	int i;

	for (i = (int) N_ELEMENTS(test_slays) - 1; i >= 0; --i) {
		(void) strnfmt(buffer, sizeof(buffer), "slay:%s",
			 test_slays[i]);
		r = parser_parse(p, buffer);
		eq(r, PARSE_ERROR_NONE);
		ps = (struct timed_effect_parse_state*) parser_priv(p);
		notnull(ps);
		notnull(ps->t);
		require(streq(test_slays[i], slays[ps->t->temp_slay].code));
	}
	ok;
}

static int test_badslay0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "slay:XYZZY");

	eq(r, PARSE_ERROR_UNRECOGNISED_SLAY);
	ok;
}

static int test_flagsyn0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "flag-synonym:SUST_STR:0");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	eq(ps->t->oflag_dup, OF_SUST_STR);
	eq(ps->t->oflag_syn, false);
	r = parser_parse(p, "flag-synonym:TELEPATHY:1");
	eq(r, PARSE_ERROR_NONE);
	eq(ps->t->oflag_dup, OF_TELEPATHY);
	eq(ps->t->oflag_syn, true);
	ok;
}

static int test_badflagsyn0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "flag-synonym:XYZZY:0");

	eq(r, PARSE_ERROR_INVALID_OBJ_PROP_CODE);
	ok;
}

static int test_missing_effect0(void *state) {
	struct parser *p = (struct parser*) state;
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	enum parser_error r;

	notnull(ps);
	null(ps->e);
	r = parser_parse(p, "effect-yx:10:20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-dice:2d20");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-expr:B:PLAYER_HP:/ 4");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	r = parser_parse(p, "effect-msg:despair");
	eq(r, PARSE_ERROR_MISSING_RECORD_HEADER);
	ok;
}

static int test_begineffect0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-begin-effect:DAMAGE");
	struct timed_effect_parse_state *ps;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_begin_effect);
	e = ps->t->on_begin_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_DAMAGE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	ptreq(e, ps->e);

	/* Check effect with type and subtype. */
	r = parser_parse(p, "on-begin-effect:CURE:BLIND");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_begin_effect);
	e = ps->t->on_begin_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_CURE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_BLIND);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	ptreq(e, ps->e);

	/* Check effect with type, subtype, and radius. */
	r = parser_parse(p, "on-begin-effect:BALL:COLD:3");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_begin_effect);
	e = ps->t->on_begin_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_BALL);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_COLD);
	eq(e->radius, 3);
	eq(e->other, 0);
	null(e->msg);
	ptreq(e, ps->e);

	/* Check effect with type, subtype, radius, and other. */
	r = parser_parse(p, "on-begin-effect:SPOT:LIGHT_WEAK:2:10");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_begin_effect);
	e = ps->t->on_begin_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_LIGHT_WEAK);
	eq(e->radius, 2);
	eq(e->other, 10);
	null(e->msg);
	ptreq(e, ps->e);

	ok;
}

static int test_badbegineffect0(void *state) {
	struct parser *p = (struct parser*) state;

	/* Check with unrecognized effect. */
	enum parser_error r = parser_parse(p, "on-begin-effect:XYZZY");

	eq(r, PARSE_ERROR_INVALID_EFFECT);
	/* Check with bad subtype. */
	r = parser_parse(p, "on-begin-effect:CURE:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_endeffect0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-end-effect:DAMAGE");
	struct timed_effect_parse_state *ps;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_end_effect);
	e = ps->t->on_end_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_DAMAGE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, 0);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	ptreq(e, ps->e);

	/* Check effect with type and subtype. */
	r = parser_parse(p, "on-end-effect:CURE:BLIND");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_end_effect);
	e = ps->t->on_end_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_CURE);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, TMD_BLIND);
	eq(e->radius, 0);
	eq(e->other, 0);
	null(e->msg);
	ptreq(e, ps->e);

	/* Check effect with type, subtype, and radius. */
	r = parser_parse(p, "on-end-effect:BALL:COLD:3");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_end_effect);
	e = ps->t->on_end_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_BALL);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_COLD);
	eq(e->radius, 3);
	eq(e->other, 0);
	null(e->msg);
	ptreq(e, ps->e);

	/* Check effect with type, subtype, radius, and other. */
	r = parser_parse(p, "on-end-effect:SPOT:LIGHT_WEAK:2:10");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_end_effect);
	e = ps->t->on_end_effect;
	while (e->next) e = e->next;
	eq(e->index, EF_SPOT);
	null(e->dice);
	eq(e->y, 0);
	eq(e->x, 0);
	eq(e->subtype, PROJ_LIGHT_WEAK);
	eq(e->radius, 2);
	eq(e->other, 10);
	null(e->msg);
	ptreq(e, ps->e);

	ok;
}

static int test_badendeffect0(void *state) {
	struct parser *p = (struct parser*) state;
	/* Check with unrecognized effect. */
	enum parser_error r = parser_parse(p, "on-begin-effect:XYZZY");

	eq(r, PARSE_ERROR_INVALID_EFFECT);
	/* Check with bad subtype. */
	r = parser_parse(p, "on-begin-effect:CURE:XYZZY");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

static int test_effectyx0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-begin-effect:DAMAGE");
	struct timed_effect_parse_state *ps;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-yx:10:20");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_begin_effect);
	e = ps->t->on_begin_effect;
	while (e->next) e = e->next;
	eq(e->y, 10);
	eq(e->x, 20);
	ok;
}

static int test_effectdice0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-end-effect:DAMAGE");
	struct timed_effect_parse_state *ps;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-dice:5+2d20");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_end_effect);
	e = ps->t->on_end_effect;
	while (e->next) e = e->next;
	notnull(e->dice);
	eq(dice_test_values(e->dice, 5, 2, 20, 0), true);
	/* Try setting again to see if memory is leaked. */
	r = parser_parse(p, "effect-dice:3+4d5");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->dice);
	eq(dice_test_values(e->dice, 3, 4, 5, 0), true);
	ok;
}

static int test_badeffectdice0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-begin-effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-dice:2+1d3+1d4");
	eq(r, PARSE_ERROR_INVALID_DICE);
	ok;
}

static int test_effectexpr0(void* state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-begin-effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-dice:4d$S");
	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-expr:S:DUNGEON_LEVEL:+ 19 / 20");
	eq(r, PARSE_ERROR_NONE);
	ok;
}

static int test_badeffectexpr0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-end-effect:DAMAGE");

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-dice:$B+d$S");
	eq(r, PARSE_ERROR_NONE);
	/* Try an expression with an invalid operations string. */
	r = parser_parse(p, "effect-expr:B:PLAYER_LEVEL:^ 2");
	eq(r, PARSE_ERROR_BAD_EXPRESSION_STRING);
	/* Try to bind an expression to a variable that isn't in the dice. */
	r = parser_parse(p, "effect-expr:N:PLAYER_LEVEL:/ 5 + 1");
	eq(r, PARSE_ERROR_UNBOUND_EXPRESSION);
	ok;
}

static int test_effectmsg0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "on-begin-effect:DAMAGE");
	struct timed_effect_parse_state *ps;
	struct effect *e;

	eq(r, PARSE_ERROR_NONE);
	r = parser_parse(p, "effect-msg:despair");
	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	notnull(ps->t->on_begin_effect);
	e = ps->t->on_begin_effect;
	while (e->next) e = e->next;
	notnull(e->msg);
	require(streq(e->msg, "despair"));
	/* Check that multiple directives are concatenated. */
	r = parser_parse(p, "effect-msg: and loneliness");
	eq(r, PARSE_ERROR_NONE);
	notnull(e->msg);
	require(streq(e->msg, "despair and loneliness"));
	ok;
}

static int test_flags0(void *state) {
	struct parser *p = (struct parser*) state;
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	enum parser_error r;

	notnull(ps);
	notnull(ps->t);
	ps->t->flags = 0;

	/* Check that specifying no flags works. */
	r = parser_parse(p, "flags:");
	eq(r, PARSE_ERROR_NONE);
	eq(ps->t->flags, 0);

	r = parser_parse(p, "flags:NONSTACKING");
	eq(r, PARSE_ERROR_NONE);
	eq(ps->t->flags, TMD_FLAG_NONSTACKING);

	ok;
}

static int test_badflags0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "flags:XYZZY");

	eq(r, PARSE_ERROR_INVALID_FLAG);
	ok;
}

static int test_lowerbound0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lower-bound:10");
	struct timed_effect_parse_state *ps;

	eq(r, PARSE_ERROR_NONE);
	ps = (struct timed_effect_parse_state*) parser_priv(p);
	notnull(ps);
	notnull(ps->t);
	eq(ps->t->lower_bound, 10);
	ok;
}

static int test_badlowerbound0(void *state) {
	struct parser *p = (struct parser*) state;
	enum parser_error r = parser_parse(p, "lower-bound:-1");

	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "lower-bound:-10");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "lower-bound:32768");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	r = parser_parse(p, "lower-bound:65535");
	eq(r, PARSE_ERROR_INVALID_VALUE);
	ok;
}

const char *suite_name = "parse/ptimed";

/*
 * test_missing_record_header0() has to be before test_name0().
 * test_name0() has to be before any of the other tests besides
 * test_missing_effect0() has to be before test_begineffect0(),
 * test_badbegineffect0(), test_endeffect0(), test_badendeffect0(),
 * test_effectyx0(), test_effectdice0(), test_badeffectdice0(),
 * test_effectexpr0(), test_badeffectexpr0(), and test_effectmsg0().
 * test_missing_record_header0() and  test_badname0().
 */
struct test tests[] = {
	{ "missing_record_header0", test_missing_record_header0 },
	{ "name0", test_name0 },
	{ "badname0", test_badname0 },
	{ "desc0", test_desc0 },
	{ "endmsg0", test_endmsg0 },
	{ "incmsg0", test_incmsg0 },
	{ "decmsg0", test_decmsg0 },
	{ "msgt0", test_msgt0 },
	{ "badmsgt0", test_badmsgt0 },
	{ "fail0", test_fail0 },
	{ "badfail0", test_badfail0 },
	{ "grade0", test_grade0 },
	{ "badgrad0", test_badgrade0 },
	{ "resist0", test_resist0 },
	{ "badresist0", test_badresist0 },
	{ "brand0", test_brand0 },
	{ "badbrand0", test_badbrand0 },
	{ "slay0", test_slay0 },
	{ "badslay0", test_badslay0 },
	{ "flagsyn0", test_flagsyn0 },
	{ "badflagsyn0", test_badflagsyn0 },
	{ "missing_effect0", test_missing_effect0 },
	{ "begineffect0", test_begineffect0 },
	{ "badbegineffect0", test_badbegineffect0 },
	{ "endeffect0", test_endeffect0 },
	{ "badendeffect0", test_badendeffect0 },
	{ "effectyx0", test_effectyx0 },
	{ "effectdice0", test_effectdice0 },
	{ "badeffectdice0", test_badeffectdice0 },
	{ "effectexpr0", test_effectexpr0 },
	{ "badeffectexpr0", test_badeffectexpr0 },
	{ "effectmsg0", test_effectmsg0 },
	{ "flags0", test_flags0 },
	{ "badflags0", test_badflags0 },
	{ "lowerbound0", test_lowerbound0 },
	{ "badlowerbound0", test_badlowerbound0 },
	{ NULL, NULL }
};
