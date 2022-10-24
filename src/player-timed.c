/**
 * \file player-timed.c
 * \brief Timed effects handling
 *
 * Copyright (c) 1997 Ben Harrison
 * Copyright (c) 2007 Andi Sidwell
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "datafile.h"
#include "effects.h"
#include "init.h"
#include "mon-util.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-properties.h"
#include "obj-slays.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"

int PY_FOOD_MAX;
int PY_FOOD_FULL;
int PY_FOOD_HUNGRY;
int PY_FOOD_WEAK;
int PY_FOOD_FAINT;
int PY_FOOD_STARVE;

/**
 * ------------------------------------------------------------------------
 * Parsing functions for player_timed.txt
 * ------------------------------------------------------------------------ */

const char *list_player_flag_names[] = {
	#define PF(a) #a,
	#include "list-player-flags.h"
	#undef PF
	NULL
};

struct timed_effect_data timed_effects[TMD_MAX] = {
	#define TMD(a, b, c)	{ #a, b, c, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, 0, 0, OF_NONE, false, -1, -1, -1 },
	#include "list-player-timed.h"
	#undef TMD
};

const char *obj_flags[] = {
	"NONE",
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

int timed_name_to_idx(const char *name)
{
    for (size_t i = 0; i < N_ELEMENTS(timed_effects); i++) {
        if (my_stricmp(name, timed_effects[i].name) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * List of timed effect names
 */
static const char *list_timed_effect_names[] = {
	#define TMD(a, b, c) #a,
	#include "list-player-timed.h"
	#undef TMD
	"MAX",
};

static enum parser_error parse_player_timed_name(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *name = parser_getstr(p, "name");
	int index;

	if (grab_name("timed effect",
			name,
			list_timed_effect_names,
			N_ELEMENTS(list_timed_effect_names),
			&index)) {
		/* XXX not a desctiptive error */
		return PARSE_ERROR_INVALID_SPELL_NAME;
	}

	assert(ps);
	ps->t = &timed_effects[index];
	ps->e = NULL;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_desc(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->t->desc = string_append(ps->t->desc, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_end_message(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->t->on_end = string_append(ps->t->on_end, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_increase_message(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->t->on_increase = string_append(ps->t->on_increase,
		parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_decrease_message(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->t->on_decrease = string_append(ps->t->on_decrease,
		parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_message_type(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->t->msgt = message_lookup_by_name(parser_getsym(p, "type"));
	return ps->t->msgt < 0 ?
		PARSE_ERROR_INVALID_MESSAGE : PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_fail(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *name = parser_getstr(p, "flag");
	struct timed_failure *f;
	int code, idx;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	code = parser_getuint(p, "code");
	switch (code) {
	case TMD_FAIL_FLAG_OBJECT:
		idx = lookup_flag(list_obj_flag_names, name);
		if (idx == FLAG_END) {
			return PARSE_ERROR_INVALID_FLAG;
		}
		break;

	case TMD_FAIL_FLAG_PLAYER:
		idx = lookup_flag(list_player_flag_names, name);
		if (idx == FLAG_END) {
			return PARSE_ERROR_INVALID_FLAG;
		}
		break;

	case TMD_FAIL_FLAG_RESIST:
	case TMD_FAIL_FLAG_VULN:
		idx = 0;
		while (list_element_names[idx]
				&& !streq(list_element_names[idx], name)) {
			idx++;
		}
		if (idx == ELEM_MAX) {
			return PARSE_ERROR_INVALID_FLAG;
		}
		break;

	case TMD_FAIL_FLAG_TIMED_EFFECT:
		if (grab_name("timed effect", name,
				list_timed_effect_names,
				N_ELEMENTS(list_timed_effect_names),
				&idx)) {
			return PARSE_ERROR_INVALID_FLAG;
		}
		break;

	default:
		return PARSE_ERROR_INVALID_FLAG;
	}

	f = mem_alloc(sizeof(*f));
	f->next = ps->t->fail;
	f->code = code;
	f->idx= idx;
	ps->t->fail = f;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_grade(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *color = parser_getsym(p, "color");
	int grade_max = parser_getint(p, "max");
	struct timed_grade *current, *l;
	int attr, food_scl;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/*
	 * The maximum has to be greater than zero so it doesn't interfere
	 * with the implicit "off" grade which has a maximum of 0.  Because
	 * a timed effect duration is stored as an int16_t in struct player,
	 * also guarantee that the maximum is compatible with that.
	 */
	food_scl = (streq(ps->t->name, "FOOD")) ? (int) z_info->food_value : 1;
	if (grade_max <= 0 || grade_max > 32767 / food_scl) {
		return PARSE_ERROR_INVALID_VALUE;
	}

	/* Make a zero grade structure if there isn't one */
	current = ps->t->grade;
	if (!current) {
		ps->t->grade = mem_zalloc(sizeof(struct timed_grade));
		current = ps->t->grade;
	}

	/* Move to the highest grade so far */
	while (current->next) {
		current = current->next;
		/* Enforce that the grades appear in ascending order. */
		if (grade_max * food_scl <= current->max) {
			return PARSE_ERROR_INVALID_VALUE;
		}
	}

	/* Add the new one */
	l = mem_zalloc(sizeof(*l));
	current->next = l;
	l->grade = current->grade + 1;

	if (strlen(color) > 1) {
		attr = color_text_to_attr(color);
	} else {
		attr = color_char_to_attr(color[0]);
	}
	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}
	l->color = attr;

	l->max = grade_max;
	l->name = string_make(parser_getsym(p, "name"));

	/* Name may be a dummy (eg hunger)*/
	if (strlen(l->name) == 1) {
		string_free(l->name);
		l->name = NULL;
	}

	l->up_msg = string_make(parser_getsym(p, "up_msg"));

	/* Message may be a dummy */
	if (strlen(l->up_msg) == 1) {
		string_free(l->up_msg);
		l->up_msg = NULL;
	}

	if (parser_hasval(p, "down_msg")) {
		l->down_msg = string_make(parser_getsym(p, "down_msg"));
	}

	/* Set food constants and deal with percentages */
	if (food_scl != 1) {
		l->max *= food_scl;
		if (l->name) {
			if (streq(l->name, "Starving")) {
				PY_FOOD_STARVE = l->max;
			} else if (streq(l->name, "Faint")) {
				PY_FOOD_FAINT = l->max;
			} else if (streq(l->name, "Weak")) {
				PY_FOOD_WEAK = l->max;
			} else if (streq(l->name, "Hungry")) {
				PY_FOOD_HUNGRY = l->max;
			} else if (streq(l->name, "Fed")) {
				PY_FOOD_FULL = l->max;
			} else if (streq(l->name, "Full")) {
				PY_FOOD_MAX = l->max;
			}
		}
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_resist(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *name = parser_getsym(p, "elem");
	int idx = (name) ? proj_name_to_idx(name) : -1;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (idx < 0 || idx >= ELEM_MAX) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	ps->t->temp_resist = idx;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_brand(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *name = parser_getsym(p, "name");
	int idx = z_info->brand_max;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (name) {
		for (idx = 1; idx < z_info->brand_max; ++idx) {
			if (streq(name, brands[idx].code)) break;
		}
	}
	if (idx == z_info->brand_max) {
		return PARSE_ERROR_UNRECOGNISED_BRAND;
	}
	ps->t->temp_brand = idx;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_slay(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *name = parser_getsym(p, "name");
	int idx = z_info->slay_max;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (name) {
		for (idx = 1; idx < z_info->slay_max; ++idx) {
			if (streq(name, slays[idx].code)) break;
		}
	}
	if (idx == z_info->slay_max) {
		return PARSE_ERROR_UNRECOGNISED_SLAY;
	}
	ps->t->temp_slay = idx;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_flag_synonym(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	const char *code = parser_getsym(p, "code");
	bool is_exact = parser_getint(p, "exact") != 0;
	int idx = (code) ? code_index_in_array(obj_flags, code) : OF_NONE;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (idx <= OF_NONE) {
		return PARSE_ERROR_INVALID_OBJ_PROP_CODE;
	}
	ps->t->oflag_dup = idx;
	ps->t->oflag_syn = is_exact;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_on_begin_effect(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	struct effect *effect;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Go to the next vacant effect and set it to the new one. */
	ps->e = mem_zalloc(sizeof(*effect));
	if (ps->t->on_begin_effect) {
		effect = ps->t->on_begin_effect;
		while (effect->next) effect = effect->next;
		effect->next = ps->e;
	} else {
		ps->t->on_begin_effect = ps->e;
	}
	/* Fill in the details. */
	return grab_effect_data(p, ps->e);
}

static enum parser_error parse_player_timed_on_end_effect(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	struct effect *effect;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Go to the next vacant effect and set it to the new one */
	ps->e = mem_zalloc(sizeof(*effect));
	if (ps->t->on_end_effect) {
		effect = ps->t->on_end_effect;
		while (effect->next) effect = effect->next;
		effect->next = ps->e;
	} else {
		ps->t->on_end_effect = ps->e;
	}
	/* Fill in the detail */
	return grab_effect_data(p, ps->e);
}

static enum parser_error parse_player_timed_effect_yx(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->e->y = parser_getint(p, "y");
	ps->e->x = parser_getint(p, "x");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_effect_dice(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	dice_t *dice;
	const char *string;

	assert(ps);
	if (!ps->e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	dice = dice_new();
	if (dice == NULL) {
		return PARSE_ERROR_INVALID_DICE;
	}
	string = parser_getstr(p, "dice");
	if (dice_parse_string(dice, string)) {
		dice_free(ps->e->dice);
		ps->e->dice = dice;
	} else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_effect_expr(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	expression_t *expression;
	expression_base_value_f function;
	const char *name, *base, *expr;
	enum parser_error result;

	assert(ps);
	if (!ps->e || !ps->e->dice) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();
	if (expression == NULL) {
		return PARSE_ERROR_INVALID_EXPRESSION;
	}
	function = effect_value_base_by_name(base);
	expression_set_base_value(expression, function);
	if (expression_add_operations_string(expression, expr) < 0) {
		result = PARSE_ERROR_BAD_EXPRESSION_STRING;
	} else if (dice_bind_expression(ps->e->dice, name, expression) < 0) {
		result = PARSE_ERROR_UNBOUND_EXPRESSION;
	} else {
		result = PARSE_ERROR_NONE;
	}
	/*
	 * The dice object makes a deep copy of the expression, so we can
	 * free it.
	 */
	expression_free(expression);
	return result;
}

static enum parser_error parse_player_timed_effect_msg(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	assert(ps);
	if (!ps->e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	ps->e->msg = string_append(ps->e->msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_effect_flags(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	char *flags, *s;

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "flags")) {
		return PARSE_ERROR_NONE;
	}
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (streq(s, "NONSTACKING")) {
			ps->t->flags |= TMD_FLAG_NONSTACKING;
		} else {
			string_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	string_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_effect_lower_bound(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);
	int bound = parser_getint(p, "bound");

	assert(ps);
	if (!ps->t) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/*
	 * Don't allow negative lower bounds (breaks the logic for testing
	 * whether a timed effect is active).  Also, since int16_t is used to
	 * store a timed effect's duration in struct player, don't allow
	 * lower bounds that aren't compatible with that.
	 */
	if (bound < 0 || bound > 32767) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	ps->t->lower_bound = bound;
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_player_timed(void)
{
	struct parser *p = parser_new();
	struct timed_effect_parse_state *ps = mem_zalloc(sizeof(*ps));

	parser_setpriv(p, ps);
	parser_reg(p, "name str name", parse_player_timed_name);
	parser_reg(p, "desc str text", parse_player_timed_desc);
	parser_reg(p, "on-end str text", parse_player_timed_end_message);
	parser_reg(p, "on-increase str text", parse_player_timed_increase_message);
	parser_reg(p, "on-decrease str text", parse_player_timed_decrease_message);
	parser_reg(p, "msgt sym type", parse_player_timed_message_type);
	parser_reg(p, "fail uint code str flag", parse_player_timed_fail);
	parser_reg(p, "grade sym color int max sym name sym up_msg ?sym down_msg", parse_player_timed_grade);
	parser_reg(p, "resist sym elem", parse_player_timed_resist);
	parser_reg(p, "brand sym name", parse_player_timed_brand);
	parser_reg(p, "slay sym name", parse_player_timed_slay);
	parser_reg(p, "flag-synonym sym code int exact",
		parse_player_timed_flag_synonym);
	parser_reg(p,
		"on-begin-effect sym eff ?sym type ?int radius ?int other",
		parse_player_timed_on_begin_effect);
	parser_reg(p,
		"on-end-effect sym eff ?sym type ?int radius ?int other",
		parse_player_timed_on_end_effect);
	parser_reg(p, "effect-yx int y int x", parse_player_timed_effect_yx);
	parser_reg(p, "effect-dice str dice", parse_player_timed_effect_dice);
	parser_reg(p, "effect-expr sym name sym base str expr",
		parse_player_timed_effect_expr);
	parser_reg(p, "effect-msg str text", parse_player_timed_effect_msg);
	parser_reg(p, "flags ?str flags", parse_player_timed_effect_flags);
	parser_reg(p, "lower-bound int bound",
		parse_player_timed_effect_lower_bound);
	return p;
}

static errr run_parse_player_timed(struct parser *p)
{
	return parse_file_quit_not_found(p, "player_timed");
}

static errr finish_parse_player_timed(struct parser *p)
{
	struct timed_effect_parse_state *ps =
		(struct timed_effect_parse_state*) parser_priv(p);

	mem_free(ps);
	parser_destroy(p);
	return 0;
}

static void cleanup_player_timed(void)
{
	/*
	 * Besides cleaning up any dynamically allocated resources, revert
	 * any fields set during parsing to their default values so the
	 * timed_effects array is back to where it started after its static
	 * initialization.
	 */
	for (size_t i = 0; i < TMD_MAX; i++) {
		struct timed_effect_data *effect = &timed_effects[i];
		struct timed_failure *f = effect->fail;
		struct timed_grade *grade = effect->grade;

		while (f) {
			struct timed_failure *next = f->next;

			mem_free(f);
			f = next;
		}
		effect->fail = NULL;

		while (grade) {
			struct timed_grade *next = grade->next;
			string_free(grade->name);
			string_free(grade->up_msg);
			string_free(grade->down_msg);
			mem_free(grade);
			grade = next;
		}
		effect->grade = NULL;

		string_free(effect->desc);
		effect->desc = NULL;

		string_free(effect->on_end);
		effect->on_end = NULL;
		string_free(effect->on_increase);
		effect->on_increase = NULL;
		string_free(effect->on_decrease);
		effect->on_decrease = NULL;
		effect->msgt = 0;

		free_effect(effect->on_begin_effect);
		effect->on_begin_effect = NULL;
		free_effect(effect->on_end_effect);
		effect->on_end_effect = NULL;

		effect->flags = 0;
		effect->lower_bound = 0;
		effect->oflag_dup = OF_NONE;
		effect->oflag_syn = false;
		effect->temp_resist = -1;
		effect->temp_brand = -1;
		effect->temp_slay = -1;
	}
}

struct file_parser player_timed_parser = {
	"player timed effects",
	init_parse_player_timed,
	run_parse_player_timed,
	finish_parse_player_timed,
	cleanup_player_timed
};


/**
 * Return true if the player timed effect matches the given string
 */
bool player_timed_grade_eq(struct player *p, int idx, const char *match)
{
	if (p->timed[idx]) {
		struct timed_grade *grade = timed_effects[idx].grade;
		while (p->timed[idx] > grade->max) {
			grade = grade->next;
		}
		if (grade->name && streq(grade->name, match)) return true;
	}

	return false;
}

static bool player_of_has_not_timed(struct player *p, int flag)
{
    bitflag collect_f[OF_SIZE], f[OF_SIZE];
    int i;

    player_flags(p, collect_f);

    for (i = 0; i < p->body.count; i++) {
        struct object *obj = slot_object(p, i);

        if (!obj) continue;
        object_flags(obj, f);
        of_union(collect_f, f);
    }

    return of_has(collect_f, flag);
}

/**
 * ------------------------------------------------------------------------
 * Setting, increasing, decreasing and clearing timed effects
 * ------------------------------------------------------------------------ */
/**
 * Set a timed effect.
 *
 * \param p is the player to affect.
 * \param idx is the index, greater than equal to zero and less than TMD_MAX,
 * for the effect.
 * \param notify, if true, allows for messages, updates to the user interface,
 * and player disturbance if setting the effect doesn't duplicate an effect
 * already present.  If false, prevents messages, updates to the user interface,
 * and player disturbance unless setting the effect increases the effect's
 * gradation or decreases the effect's gradation when the effect has messages
 * for the gradations that lapse.
 * \param can_disturb, if true, allows for setting the effect to disturb the
 * player.
 * \return whether setting the effect caused the player to be notified.
 */
bool player_set_timed(struct player *p, int idx, int v, bool notify,
		bool can_disturb)
{
	assert(idx >= 0);
	assert(idx < TMD_MAX);

	struct timed_effect_data *effect = &timed_effects[idx];
	struct timed_grade *new_grade = effect->grade;
	struct timed_grade *current_grade = effect->grade;
	struct object *weapon = equipped_item_by_slot_name(p, "weapon");

	/* Lower bound */
	v = MAX(v, effect->lower_bound);

	/* No change */
	if (p->timed[idx] == v) {
		return false;
	}

	/* Find the grade we will be going to, and the current one */
	while (v > new_grade->max) {
		new_grade = new_grade->next;
		if (!new_grade->next) break;
	}
	while (p->timed[idx] > current_grade->max) {
		current_grade = current_grade->next;
		if (!current_grade->next) break;
	}

	/* Upper bound */
	if (v > new_grade->max) {
		if (p->timed[idx] == new_grade->max) {
			/*
			 * No change:  tried to exceed the maximum possible and
			 * already there
			 */
			return false;
		}
		v = new_grade->max;
	}

	/* Don't mention effects which already match the known player state. */
	if (timed_effects[idx].temp_resist != -1 &&
			p->obj_k->el_info[timed_effects[idx].temp_resist].res_level &&
			player_is_immune(p, timed_effects[idx].temp_resist)) {
		notify = false;
	}
	if (timed_effects[idx].oflag_syn &&
			timed_effects[idx].oflag_dup != OF_NONE &&
			of_has(p->obj_k->flags, timed_effects[idx].oflag_dup) &&
			player_of_has_not_timed(p, timed_effects[idx].oflag_dup)) {
		notify = false;
	}

	/* Always mention going up a grade, otherwise on request */
	if (new_grade->grade > current_grade->grade) {
		print_custom_message(weapon, new_grade->up_msg,
			effect->msgt, p);
		notify = true;
	} else if ((new_grade->grade < current_grade->grade) &&
			(new_grade->down_msg)) {
		print_custom_message(weapon, new_grade->down_msg,
			effect->msgt, p);
		notify = true;
	} else if (notify) {
		if (v == 0) {
			/* Finishing */
			print_custom_message(weapon, effect->on_end,
				MSG_RECOVER, p);
		} else if (p->timed[idx] > v && effect->on_decrease) {
			/* Decrementing */
			print_custom_message(weapon, effect->on_decrease,
				effect->msgt, p);
		} else if (v > p->timed[idx] && effect->on_increase) {
			/* Incrementing */
			print_custom_message(weapon, effect->on_increase,
				effect->msgt, p);
		}
	}

	/*
	 * Dispatch effects for transitions.  Use source_player() as the
	 * origin if can_disturb is not true; otherwise, use source_none().
	 * That way any TIMED_INC or TIMED_INC_NO_RES effects in the effect
	 * chains will honor can_disturb.
	 */
	if (v > 0 && !p->timed[idx]) {
		/* The effect starts. */
		if (effect->on_begin_effect) {
			bool ident = false;

			(void) effect_do(effect->on_begin_effect,
				can_disturb ? source_none() : source_player(),
				NULL, &ident, true, 0, 0, 0, NULL);
		}
	} else if (v == 0) {
		/* The effect lapses. */
		if (effect->on_end_effect) {
			bool ident = false;

			(void) effect_do(effect->on_end_effect,
				can_disturb ? source_none() : source_player(),
				NULL, &ident, true, 0, 0, 0, NULL);
		}
	}

	/* Use the value */
	p->timed[idx] = v;

	if (notify) {
		/* Disturb */
		if (can_disturb) {
			disturb(p);
		}

		/* Update the visuals, as appropriate. */
		p->upkeep->update |= effect->flag_update;
		p->upkeep->redraw |= (PR_STATUS | effect->flag_redraw);

		/* Handle stuff */
		handle_stuff(p);
	}

	return notify;
}

/**
 * Check whether a timed effect will affect the player
 *
 * \param p is the player to check.
 * \param idx is the index, greater than equal to zero and less than TMD_MAX,
 * for the effect.
 * \param lore, if true, modifies the check so it is appropriate for filling
 * in details of monster recall.
 * \return whether the player can be affected by the effect.
 */
bool player_inc_check(struct player *p, int idx, bool lore)
{
	struct timed_effect_data *effect = &timed_effects[idx];
	struct timed_failure *f = effect->fail;

	while (f) {
		switch (f->code) {
		case TMD_FAIL_FLAG_OBJECT:
			/* Effect is inhibited by an object flag */
			if (lore) {
				if (of_has(p->known_state.flags, f->idx)) {
					return false;
				}
			} else {
				/*
				 * If the effect is from a monster action,
				 * extra stuff happens.
				 */
				struct monster *mon = cave->mon_current > 0 ?
					cave_monster(cave, cave->mon_current) :
					NULL;

				equip_learn_flag(p, f->idx);
				if (mon) {
					update_smart_learn(mon, p, f->idx,
						0, -1);
				}
				if (player_of_has(p, f->idx)) {
					if (mon) {
						msg("You resist the effect!");
					}
					return false;
				}
			}
			break;

		case TMD_FAIL_FLAG_RESIST:
			/* Effect is inhibited by a resist */
			assert(f->idx >= 0 && f->idx < ELEM_MAX);
			if (lore) {
				if (p->known_state.el_info[f->idx].res_level > 0) {
					return false;
				}
			} else {
				equip_learn_element(p, f->idx);
				if (p->state.el_info[f->idx].res_level > 0) {
					return false;
				}
			}
			break;

		case TMD_FAIL_FLAG_VULN:
			/*
			 * Effect is inhibited by a vulnerability
			 * the asymmetry with resists is OK for now - NRM
			 */
			assert(f->idx >= 0 && f->idx < ELEM_MAX);
			if (lore) {
				if (p->known_state.el_info[f->idx].res_level < 0) {
					return false;
				}
			} else {
				equip_learn_element(p, f->idx);
				if (p->state.el_info[f->idx].res_level < 0) {
					return false;
				}
			}
			break;

		case TMD_FAIL_FLAG_PLAYER:
			/* Effect is inhibited by a player flag */
			if (lore) {
				if (pf_has(p->known_state.pflags, f->idx)) {
					return false;
				}
			} else {
				if (player_has(p, f->idx)) {
					return false;
				}
			}
			break;

		case TMD_FAIL_FLAG_TIMED_EFFECT:
			/*
			 * Effect is inhibited by a timed effect.  If timed
			 * effect is active, it is known to the player, so
			 * there's no difference between whether this is
			 * solely a lore check or not.
			 */
			assert(f->idx >= 0 && f->idx < TMD_MAX);
			if (p->timed[f->idx]) {
				return false;
			}
			break;

		default:
			/* Should never happen. */
			assert(0);
			break;
		}

		f = f->next;
	}

	/* Nothing prevents this effect from incrementing. */
	return true;
}

/**
 * Increase the timed effect `idx` by `v`.
 *
 * \param p is the player to affect.
 * \param idx is the index, greater than equal to zero and less than TMD_MAX,
 * for the effect.
 * \param notify, if true, allows for messages, updates to the user interface,
 * and player disturbance if increasing the duration doesn't duplicate an effect
 * already present.  If false, prevents messages, updates to the user interface,
 * and player disturbance unless increasing the duration increases the effect's
 * gradation or decreases the effect's gradation when the effect has messages
 * for the gradations that lapse.
 * \param can_disturb, if true, allows for setting the effect to disturb the
 * player.
 * \param check, if true, allows for the player to resist the effect if
 * player_inc_check(p, idx, false) is true.
 * \return whether increasing the duration caused the player to be notified.
 */
bool player_inc_timed(struct player *p, int idx, int v, bool notify,
		bool can_disturb, bool check)
{
	assert(idx >= 0);
	assert(idx < TMD_MAX);

	if (check == false || player_inc_check(p, idx, false) == true) {
		if ((timed_effects[idx].flags & TMD_FLAG_NONSTACKING)
				&& p->timed[idx] > 0) {
			/*
			 * Block the increase if the effect is nonstacking and
			 * already active.
			 */
			return false;
		} else {
			return player_set_timed(p,
					idx,
					p->timed[idx] + v,
					notify, can_disturb);
		}
	}

	return false;
}

/**
 * Decrease the timed effect `idx` by `v`.
 *
 * \param p is the player to affect.
 * \param idx is the index, greater than equal to zero and less than TMD_MAX,
 * for the effect.
 * \param v is the amount to subtract from the effect's duration.
 * \param notify, if true or v is greater than or equal to the effect's current
 * duration, allows for messages, updates to the user interface, and player
 * disturbance.  If false and v is less than the effect's current duration,
 * prevents messages, updates to the user interface, and player disturbance
 * unless the change to the duration increases the effect's gradation or
 * decreases the effect's gradation when the effect has messages for the
 * gradations that lapse.
 * \param can_disturb, if true, allows for the change to the duration to disturb
 * the player.
 * \return whether changing the duration caused the player to be notified.
 */
bool player_dec_timed(struct player *p, int idx, int v, bool notify,
		bool can_disturb)
{
	int new_value;
	assert(idx >= 0);
	assert(idx < TMD_MAX);
	new_value = p->timed[idx] - v;

	/* Obey `notify` if not finishing; if finishing, always notify */
	if (new_value > 0) {
		return player_set_timed(p, idx, new_value, notify, can_disturb);
	}
	return player_set_timed(p, idx, new_value, true, can_disturb);
}

/**
 * Clear the timed effect `idx`.
 *
 * \param p is the player to affect.
 * \param idx is the index, greater than equal to zero and less than TMD_MAX,
 * for the effect.
 * \param notify, if true, allows for messages, updates to the user interface,
 * and player disturbance if clearing the effect doesn't duplicate an effect
 * already present.  If false, prevents messages, updates to the user interface,
 * and player disturbance unless clearing the effect decreases the effect's
 * gradation and the effect has messages for the gradations that lapse.
 * \param can_disturb, if true, allows for setting the effect to disturb the
 * player.
 * \return whether clearing the effect caused the player to be notified.
 */
bool player_clear_timed(struct player *p, int idx, bool notify,
		bool can_disturb)
{
	assert(idx >= 0);
	assert(idx < TMD_MAX);

	return player_set_timed(p, idx, 0, notify, can_disturb);
}

