/**
 * \file mon-init.c
 * \brief Monster initialization routines.
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "init.h"
#include "mon-init.h"
#include "mon-lore.h"
#include "mon-msg.h"
#include "mon-power.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "mon-blow-methods.h"
#include "mon-blow-effects.h"
#include "monster.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "parser.h"
#include "player-spell.h"

struct monster_pain *pain_messages;
struct monster_spell *monster_spells;
struct monster_base *rb_info;
struct monster_race *r_info;
const struct monster_race *ref_race = NULL;
struct monster_lore *l_list;

const char *r_info_flags[] =
{
	#define RF(a, b, c) #a,
	#include "list-mon-race-flags.h"
	#undef RF
	NULL
};

const char *r_info_spell_flags[] =
{
	#define RSF(a, b) #a,
	#include "list-mon-spells.h"
	#undef RSF
	NULL
};

static const char *effect_list[] = {
	"NONE",
	#define EFFECT(x, a, b, c, d, e)	#x,
	#include "list-effects.h"
	#undef EFFECT
	"MAX"
};

/**
 * Write the flag lines for a set of flags.
 */
void write_flags(ang_file *fff, const char *intro_text, bitflag *flags,
					   int flag_size, const char *names[])
{
	int flag;
	char buf[1024] = "";
	int pointer = 0;

	/* Write flag name list */
	for (flag = flag_next(flags, flag_size, FLAG_START); flag != FLAG_END;
		 flag = flag_next(flags, flag_size, flag + 1)) {

		/* Write the flags, keeping track of where we are */
		if (strlen(buf)) {
			my_strcat(buf, " | ", sizeof(buf));
			pointer += 3;
		}

		/* If no name, we're past the real flags */
		if (!names[flag]) break;
		my_strcat(buf, names[flag], sizeof(buf));
		pointer += strlen(names[flag]);

		/* Move to a new line if this one is long enough */
		if (pointer >= 60) {
			file_putf(fff, "%s%s\n", intro_text, buf);
			my_strcpy(buf, "", sizeof(buf));
			pointer = 0;
		}
	}

	/* Print remaining flags if any */
	if (pointer)
		file_putf(fff, "%s%s\n", intro_text, buf);
}


/**
 * Parsing functions for monster_spell.txt
 */

static enum parser_error parse_mon_spell_name(struct parser *p) {
	struct monster_spell *h = parser_priv(p);
	struct monster_spell *s = mem_zalloc(sizeof *s);
	const char *name = parser_getstr(p, "name");
	int index;
	s->next = h;
	if (grab_name("monster spell", name, r_info_spell_flags, N_ELEMENTS(r_info_spell_flags), &index))
		return PARSE_ERROR_INVALID_SPELL_NAME;
	s->index = index;
	parser_setpriv(p, s);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_mon_spell_message_type(struct parser *p)
{
	int msg_index;
	const char *type;
	struct monster_spell *s = parser_priv(p);
	assert(s);

	type = parser_getsym(p, "type");

	msg_index = message_lookup_by_name(type);

	if (msg_index < 0)
		return PARSE_ERROR_INVALID_MESSAGE;

	s->msgt = msg_index;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	assert(s);

	s->message = string_append(s->message, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_blind_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	assert(s);

	s->blind_message = string_append(s->blind_message,
									 parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_miss_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	assert(s);

	s->miss_message = string_append(s->miss_message, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_save_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	assert(s);

	s->save_message = string_append(s->save_message, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_lore_desc(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	assert(s);

	s->lore_desc = string_append(s->lore_desc, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_hit(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	assert(s);
	s->hit = parser_getuint(p, "hit");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_effect(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));
	const char *type;
	int val;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (s->effect) {
		effect = s->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		s->effect = new_effect;

	if (grab_name("effect", parser_getsym(p, "eff"), effect_list,
				  N_ELEMENTS(effect_list), &val))
		return PARSE_ERROR_INVALID_EFFECT;
	new_effect->index = val;

	if (parser_hasval(p, "type")) {
		type = parser_getsym(p, "type");

		if (type == NULL)
			return PARSE_ERROR_UNRECOGNISED_PARAMETER;

		/* Check for a value */
	val = effect_param(new_effect->index, type);
		if (val < 0)
			return PARSE_ERROR_INVALID_VALUE;
		else
			new_effect->params[0] = val;
	}

	if (parser_hasval(p, "xtra"))
		new_effect->params[1] = parser_getint(p, "xtra");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_param(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect = s->effect;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;
	effect->params[1] = parser_getint(p, "p2");

	if (parser_hasval(p, "p3"))
		effect->params[2] = parser_getint(p, "p3");

	return PARSE_ERROR_NONE;
}


static enum parser_error parse_mon_spell_dice(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	dice_t *dice = NULL;
	struct effect *effect = s->effect;
	const char *string = NULL;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_expr(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect = s->effect;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!s)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL)
		return PARSE_ERROR_NONE;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = spell_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_power(struct parser *p) {
	struct monster_spell *s = parser_priv(p);

	s->power = parser_getrand(p, "power");

	return PARSE_ERROR_NONE;
}

struct parser *init_parse_mon_spell(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_mon_spell_name);
	parser_reg(p, "msgt sym type", parse_mon_spell_message_type);
	parser_reg(p, "message-vis str text", parse_mon_spell_message);
	parser_reg(p, "message-invis str text", parse_mon_spell_blind_message);
	parser_reg(p, "message-miss str text", parse_mon_spell_miss_message);
	parser_reg(p, "message-save str text", parse_mon_spell_save_message);
	parser_reg(p, "lore str text", parse_mon_spell_lore_desc);
	parser_reg(p, "hit uint hit", parse_mon_spell_hit);
	parser_reg(p, "effect sym eff ?sym type ?int xtra", parse_mon_spell_effect);
	parser_reg(p, "param int p2 ?int p3", parse_mon_spell_param);
	parser_reg(p, "dice str dice", parse_mon_spell_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_mon_spell_expr);
	parser_reg(p, "power rand power", parse_mon_spell_power);
	return p;
}

static errr run_parse_mon_spell(struct parser *p) {
	return parse_file(p, "monster_spell");
}

static errr finish_parse_mon_spell(struct parser *p) {
	monster_spells = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_mon_spell(void)
{
	struct monster_spell *rs = monster_spells;
	struct monster_spell *next;

	while (rs) {
		next = rs->next;
		free_effect(rs->effect);
		string_free(rs->message);
		string_free(rs->blind_message);
		if (rs->miss_message)
			string_free(rs->miss_message);
		if (rs->save_message)
			string_free(rs->save_message);
		string_free(rs->lore_desc);
		mem_free(rs);
		rs = next;
	}
}

struct file_parser mon_spell_parser = {
	"monster_spell",
	init_parse_mon_spell,
	run_parse_mon_spell,
	finish_parse_mon_spell,
	cleanup_mon_spell
};

/**
 * Parsing functions for monster_base.txt
 */
static enum parser_error parse_mon_base_name(struct parser *p) {
	struct monster_base *h = parser_priv(p);
	struct monster_base *rb = mem_zalloc(sizeof *rb);
	rb->next = h;
	rb->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, rb);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_base_glyph(struct parser *p) {
	struct monster_base *rb = parser_priv(p);

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	rb->d_char = parser_getchar(p, "glyph");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_base_pain(struct parser *p) {
	struct monster_base *rb = parser_priv(p);
	int pain_idx;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pain_idx = parser_getuint(p, "pain");
	if (pain_idx >= z_info->mp_max)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	rb->pain = &pain_messages[pain_idx];

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_base_flags(struct parser *p) {
	struct monster_base *rb = parser_priv(p);
	char *flags;
	char *s;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(rb->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			quit_fmt("bad f-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_base_spells(struct parser *p) {
	struct monster_base *rb = parser_priv(p);
	char *flags;
	char *s;

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(rb->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			quit_fmt("bad s-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_mon_base_desc(struct parser *p) {
	struct monster_base *rb = parser_priv(p);

	if (!rb)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	rb->text = string_append(rb->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}


static struct parser *init_parse_mon_base(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name str name", parse_mon_base_name);
	parser_reg(p, "glyph char glyph", parse_mon_base_glyph);
	parser_reg(p, "pain uint pain", parse_mon_base_pain);
	parser_reg(p, "flags ?str flags", parse_mon_base_flags);
	parser_reg(p, "spells ?str spells", parse_mon_base_spells);
	parser_reg(p, "desc str desc", parse_mon_base_desc);
	return p;
}

static errr run_parse_mon_base(struct parser *p) {
	return parse_file(p, "monster_base");
}

static errr finish_parse_mon_base(struct parser *p) {
	rb_info = parser_priv(p);
	parser_destroy(p);
	return 0;
}

static void cleanup_mon_base(void)
{
	struct monster_base *rb, *next;

	rb = rb_info;
	while (rb) {
		next = rb->next;
		string_free(rb->text);
		string_free(rb->name);
		mem_free(rb);
		rb = next;
	}
}

struct file_parser mon_base_parser = {
	"monster_base",
	init_parse_mon_base,
	run_parse_mon_base,
	finish_parse_mon_base,
	cleanup_mon_base
};


/**
 * Parsing functions for monster.txt
 */
static enum parser_error parse_monster_name(struct parser *p) {
	struct monster_race *h = parser_priv(p);
	struct monster_race *r = mem_zalloc(sizeof *r);
	r->next = h;
	r->ridx = parser_getuint(p, "index");
	r->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, r);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_base(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	r->base = lookup_monster_base(parser_getsym(p, "base"));
	if (r->base == NULL)
		/* Todo: make new error for this */
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	/* The template sets the default display character */
	r->d_char = r->base->d_char;

	/* Give the monster its default flags */
	rf_union(r->flags, r->base->flags);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_glyph(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	/* If the display character is specified, it overrides any template */
	r->d_char = parser_getchar(p, "glyph");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_color(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	const char *color;
	int attr;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
		color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	r->d_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_info(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->speed = parser_getint(p, "speed");
	r->avg_hp = parser_getint(p, "hp");
	/* Area of action assumes max_sight is 20, so we adjust in case it isn't */
	r->aaf = parser_getint(p, "aaf") * 20 / z_info->max_sight;
	r->ac = parser_getint(p, "ac");
	r->sleep = parser_getint(p, "sleep");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_power(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->level = parser_getint(p, "level");
	r->rarity = parser_getint(p, "rarity");
	r->power = parser_getint(p, "power");
	r->scaled_power = parser_getint(p, "scaled");
	r->mexp = parser_getint(p, "mexp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_blow(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_blow *b = r->blow;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the last valid blow, then allocate a new one */
	if (!b) {
		r->blow = mem_zalloc(sizeof(struct monster_blow));
		b = r->blow;
	} else {
		while (b->next)
			b = b->next;
		b->next = mem_zalloc(sizeof(struct monster_blow));
		b = b->next;
	}

	/* Now read the data */
	b->method = blow_method_name_to_idx(parser_getsym(p, "method"));
	if (!monster_blow_method_is_valid(b->method))
		return PARSE_ERROR_UNRECOGNISED_BLOW;
	if (parser_hasval(p, "effect")) {
		b->effect = blow_effect_name_to_idx(parser_getsym(p, "effect"));
		if (!monster_blow_effect_is_valid(b->effect))
			return PARSE_ERROR_INVALID_EFFECT;
	}
	if (parser_hasval(p, "damage"))
		b->dice = parser_getrand(p, "damage");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_flags(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			quit_fmt("bad f2-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_flags_off(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (remove_flag(r->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			quit_fmt("bad mf-flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_desc(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->text = string_append(r->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_spell_freq(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	int pct;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	pct = parser_getint(p, "freq");
	if (pct < 1 || pct > 100)
		return PARSE_ERROR_INVALID_SPELL_FREQ;
	r->freq_spell = 100 / pct;
	r->freq_innate = r->freq_spell;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_spells(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;
	int ret = PARSE_ERROR_NONE;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			quit_fmt("bad spell flag: %s", s);
			ret = PARSE_ERROR_INVALID_FLAG;
			break;
		}
		s = strtok(NULL, " |");
	}

	/* Add the "base monster" flags to the monster */
	if (r->base)
		rsf_union(r->spell_flags, r->base->spell_flags);

	mem_free(flags);
	return ret;
}

static enum parser_error parse_monster_drop(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_drop *d;
	struct object_kind *k;
	int tval, sval;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	if (parser_getuint(p, "min") > 99 || parser_getuint(p, "max") > 99)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;

	k = lookup_kind(tval, sval);
	if (!k)
		return PARSE_ERROR_UNRECOGNISED_SVAL;
		
	d = mem_zalloc(sizeof *d);
	d->kind = k;
	d->percent_chance = parser_getuint(p, "chance");
	d->min = parser_getuint(p, "min");
	d->max = parser_getuint(p, "max");
	d->next = r->drops;
	r->drops = d;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_drop_artifact(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_drop *d;
	int art;
	struct artifact *a;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	art = lookup_artifact_name(parser_getstr(p, "name"));
	if (art < 0)
		return PARSE_ERROR_NO_ARTIFACT_NAME;
	a = &a_info[art];

	d = mem_zalloc(sizeof *d);
	d->artifact = a;
	d->min = 1;
	d->max = 1;
	d->percent_chance = 100;
	d->next = r->drops;
	r->drops = d;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_friends(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_friends *f;
	struct random number;
	
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f = mem_zalloc(sizeof *f);
	number = parser_getrand(p, "number");
	f->number_dice = number.dice;
	f->number_side = number.sides;
	f->percent_chance = parser_getuint(p, "chance");
	f->name = string_make(parser_getstr(p, "name"));
	f->next = r->friends;
	r->friends = f;
	
	return PARSE_ERROR_NONE;
}			

static enum parser_error parse_monster_friends_base(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_friends_base *f;
	struct random number;
	
	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f = mem_zalloc(sizeof *f);
	number = parser_getrand(p, "number");
	f->number_dice = number.dice;
	f->number_side = number.sides;
	f->percent_chance = parser_getuint(p, "chance");
	f->base = lookup_monster_base(parser_getstr(p, "name"));
	if (!f->base) return PARSE_ERROR_UNRECOGNISED_TVAL;

	f->next = r->friends_base;
	r->friends_base = f;
	
	return PARSE_ERROR_NONE;
}		

static enum parser_error parse_monster_mimic(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_mimic *m;
	int tval, sval;
	struct object_kind *kind;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	kind = lookup_kind(tval, sval);
	if (!kind)
		return PARSE_ERROR_NO_KIND_FOUND;
	m = mem_zalloc(sizeof *m);
	m->kind = kind;
	m->next = r->mimic_kinds;
	r->mimic_kinds = m;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_plural(struct parser *p)
{
	struct monster_race *r = parser_priv(p);

	if (r == NULL)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	if (parser_hasval(p, "plural")) {
		const char *plural = parser_getstr(p, "plural");

		if (strlen(plural) > 0)
			r->plural = string_make(plural);
		else
			r->plural = NULL;
	}

	return PARSE_ERROR_NONE;
}

struct parser *init_parse_monster(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name uint index str name", parse_monster_name);
	parser_reg(p, "plural ?str plural", parse_monster_plural);
	parser_reg(p, "base sym base", parse_monster_base);
	parser_reg(p, "glyph char glyph", parse_monster_glyph);
	parser_reg(p, "color sym color", parse_monster_color);
	parser_reg(p, "info int speed int hp int aaf int ac int sleep", parse_monster_info);
	parser_reg(p, "power int level int rarity int power int scaled int mexp", parse_monster_power);
	parser_reg(p, "blow sym method ?sym effect ?rand damage", parse_monster_blow);
	parser_reg(p, "flags ?str flags", parse_monster_flags);
	parser_reg(p, "flags-off ?str flags", parse_monster_flags_off);
	parser_reg(p, "desc str desc", parse_monster_desc);
	parser_reg(p, "spell-freq int freq", parse_monster_spell_freq);
	parser_reg(p, "spells str spells", parse_monster_spells);
	parser_reg(p, "drop sym tval sym sval uint chance uint min uint max", parse_monster_drop);
	parser_reg(p, "drop-artifact str name", parse_monster_drop_artifact);
	parser_reg(p, "friends uint chance rand number str name", parse_monster_friends);
	parser_reg(p, "friends-base uint chance rand number str name", parse_monster_friends_base);
	parser_reg(p, "mimic sym tval sym sval", parse_monster_mimic);
	return p;
}

static errr run_parse_monster(struct parser *p) {
	return parse_file(p, "monster");
}

static errr finish_parse_monster(struct parser *p) {
	struct monster_race *r, *n;
	size_t i;

	/* Scan the list for the max id and max blows */
	z_info->r_max = 0;
	z_info->mon_blows_max = 0;
	r = parser_priv(p);
	while (r) {
		int max_blows = 0;
		struct monster_blow *b = r->blow;
		if (r->ridx > z_info->r_max)
			z_info->r_max = r->ridx;
		while (b) {
			b = b->next;
			max_blows++;
		}
		if (max_blows > z_info->mon_blows_max)
			z_info->mon_blows_max = max_blows;
		r = r->next;
	}

	/* Allocate the direct access list and copy the race records to it */
	r_info = mem_zalloc((z_info->r_max + 1) * sizeof(*r));
	for (r = parser_priv(p); r; r = n) {
		struct monster_blow *b_new;

		/* Main record */
		memcpy(&r_info[r->ridx], r, sizeof(*r));
		n = r->next;
		if (n)
			r_info[r->ridx].next = &r_info[n->ridx];
		else
			r_info[r->ridx].next = NULL;

		/* Blows */
		b_new = mem_zalloc(z_info->mon_blows_max * sizeof(*b_new));
		if (r->blow) {
			struct monster_blow *b_temp, *b_old = r->blow;

			/* Allocate space and copy */
			for (i = 0; i < z_info->mon_blows_max; i++) {
				memcpy(&b_new[i], b_old, sizeof(*b_old));
				b_old = b_old->next;
				if (!b_old) break;
			}

			/* Make next point correctly */
			for (i = 0; i < z_info->mon_blows_max; i++)
				if (b_new[i].next)
					b_new[i].next = &b_new[i + 1];

			/* Tidy up */
			b_old = r->blow;
			b_temp = b_old;
			while (b_temp) {
				b_temp = b_old->next;
				mem_free(b_old);
				b_old = b_temp;
			}
		}
		r_info[r->ridx].blow = b_new;

		mem_free(r);
	}
	z_info->r_max += 1;

	/* Convert friend names into race pointers */
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_race *r = &r_info[i];
		struct monster_friends *f;
		for (f = r->friends; f; f = f->next) {
			if (!my_stricmp(f->name, "same"))
				f->race = r;
			else
				f->race = lookup_monster(f->name);

			if (!f->race)
				quit_fmt("Couldn't find friend named '%s' for monster '%s'",
						 f->name, r->name);

			string_free(f->name);
		}
	}

	/* Allocate space for the monster lore */
	l_list = mem_zalloc(z_info->r_max * sizeof(struct monster_lore));
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_lore *l = &l_list[i];
		l->blows = mem_zalloc(z_info->mon_blows_max * sizeof(struct monster_blow));
		l->blow_known = mem_zalloc(z_info->mon_blows_max * sizeof(bool));
	}

	/* Write new monster.txt file if requested */
	if (arg_power || arg_rebalance)
		eval_monster_power(r_info);

	parser_destroy(p);
	return 0;
}

static void cleanup_monster(void)
{
	int ridx;

	for (ridx = 0; ridx < z_info->r_max; ridx++) {
		struct monster_race *r = &r_info[ridx];
		struct monster_drop *d;
		struct monster_friends *f;
		struct monster_friends_base *fb;
		struct monster_mimic *m;

		d = r->drops;
		while (d) {
			struct monster_drop *dn = d->next;
			mem_free(d);
			d = dn;
		}
		f = r->friends;
		while (f) {
			struct monster_friends *fn = f->next;
			mem_free(f);
			f = fn;
		}
		fb = r->friends_base;
		while (fb) {
			struct monster_friends_base *fbn = fb->next;
			mem_free(fb);
			fb = fbn;
		}		
		m = r->mimic_kinds;
		while (m) {
			struct monster_mimic *mn = m->next;
			mem_free(m);
			m = mn;
		}
		string_free(r->plural);
		string_free(r->text);
		string_free(r->name);
		mem_free(r->blow);
	}

	mem_free(r_info);
}

struct file_parser monster_parser = {
	"monster",
	init_parse_monster,
	run_parse_monster,
	finish_parse_monster,
	cleanup_monster
};

/* Parsing functions for lore.txt */
static enum parser_error parse_lore_name(struct parser *p) {
	int index = parser_getuint(p, "index");
	struct monster_lore *l = &l_list[index];

	parser_setpriv(p, l);
	l->ridx = index;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_base(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_base *base = lookup_monster_base(parser_getsym(p, "base"));

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (base == NULL)
		/* Todo: make new error for this */
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	/* Know everything */
	l->all_known = TRUE;
	rf_setall(l->flags);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_counts(struct parser *p) {
	struct monster_lore *l = parser_priv(p);

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	l->sights = parser_getint(p, "sights");
	l->deaths = parser_getint(p, "deaths");
	l->tkills = parser_getint(p, "tkills");
	l->wake = parser_getint(p, "wake");
	l->ignore = parser_getint(p, "ignore");
	l->cast_innate = parser_getint(p, "innate");
	l->cast_spell = parser_getint(p, "spell");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_blow(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	int method, effect = 0, seen = 0, index = 0;
	struct random dam;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Read in all the data */
	method = blow_method_name_to_idx(parser_getsym(p, "method"));
	if (!monster_blow_method_is_valid(method))
		return PARSE_ERROR_UNRECOGNISED_BLOW;
	if (parser_hasval(p, "effect")) {
		effect = blow_effect_name_to_idx(parser_getsym(p, "effect"));
		if (!monster_blow_effect_is_valid(effect))
			return PARSE_ERROR_INVALID_EFFECT;
	}
	if (parser_hasval(p, "damage"))
		dam = parser_getrand(p, "damage");
	if (parser_hasval(p, "seen"))
		seen = parser_getint(p, "seen");
	if (parser_hasval(p, "index"))
		index = parser_getint(p, "index");
	if (index >= z_info->mon_blows_max)
		return PARSE_ERROR_TOO_MANY_ENTRIES;

	/* Interpret */
	if (seen) {
		struct monster_blow *b = &l->blows[index];
		b->method = method;
		b->effect = effect;
		b->dice = dam;
		b->times_seen = seen;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_flags(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	char *flags;
	char *s;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(l->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			quit_fmt("bad lore flag: %s", s);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_spells(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	char *flags;
	char *s;
	int ret = PARSE_ERROR_NONE;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(l->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			quit_fmt("bad lore spell flag: %s", s);
			ret = PARSE_ERROR_INVALID_FLAG;
			break;
		}
		s = strtok(NULL, " |");
	}

	mem_free(flags);
	return ret;
}

static enum parser_error parse_lore_drop(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_drop *d;
	struct object_kind *k;
	int tval, sval;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	if (parser_getuint(p, "min") > 99 || parser_getuint(p, "max") > 99)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;

	k = lookup_kind(tval, sval);
	if (!k)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	d = mem_zalloc(sizeof *d);
	d->kind = k;
	d->percent_chance = parser_getuint(p, "chance");
	d->min = parser_getuint(p, "min");
	d->max = parser_getuint(p, "max");
	d->next = l->drops;
	l->drops = d;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_drop_artifact(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_drop *d;
	int art;
	struct artifact *a;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	art = lookup_artifact_name(parser_getstr(p, "name"));
	if (art < 0)
		return PARSE_ERROR_NO_ARTIFACT_NAME;
	a = &a_info[art];

	d = mem_zalloc(sizeof *d);
	d->artifact = a;
	d->min = 1;
	d->max = 1;
	d->percent_chance = 100;
	d->next = l->drops;
	l->drops = d;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_friends(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_friends *f;
	struct random number;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f = mem_zalloc(sizeof *f);
	number = parser_getrand(p, "number");
	f->number_dice = number.dice;
	f->number_side = number.sides;
	f->percent_chance = parser_getuint(p, "chance");
	f->name = string_make(parser_getstr(p, "name"));
	f->next = l->friends;
	l->friends = f;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_friends_base(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_friends_base *f;
	struct random number;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	f = mem_zalloc(sizeof *f);
	number = parser_getrand(p, "number");
	f->number_dice = number.dice;
	f->number_side = number.sides;
	f->percent_chance = parser_getuint(p, "chance");
	f->base = lookup_monster_base(parser_getstr(p, "name"));
	if (!f->base) return PARSE_ERROR_UNRECOGNISED_TVAL;

	f->next = l->friends_base;
	l->friends_base = f;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_mimic(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_mimic *m;
	int tval, sval;
	struct object_kind *kind;

	if (!l)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	sval = lookup_sval(tval, parser_getsym(p, "sval"));
	if (sval < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	kind = lookup_kind(tval, sval);
	if (!kind)
		return PARSE_ERROR_NO_KIND_FOUND;
	m = mem_zalloc(sizeof *m);
	m->kind = kind;
	m->next = l->mimic_kinds;
	l->mimic_kinds = m;
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_lore(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name uint index str name", parse_lore_name);
	parser_reg(p, "plural ?str plural", ignored);
	parser_reg(p, "base sym base", parse_lore_base);
	parser_reg(p, "glyph char glyph", ignored);
	parser_reg(p, "color sym color", ignored);
	parser_reg(p, "info int speed int hp int aaf int ac int sleep", ignored);
	parser_reg(p, "power int level int rarity int power int scaled int mexp", ignored);
	parser_reg(p, "counts int sights int deaths int tkills int wake int ignore int innate int spell", parse_lore_counts);
	parser_reg(p, "blow sym method ?sym effect ?rand damage ?int seen ?int index", parse_lore_blow);
	parser_reg(p, "flags ?str flags", parse_lore_flags);
	parser_reg(p, "flags-off ?str flags", ignored);
	parser_reg(p, "desc str desc", ignored);
	parser_reg(p, "spell-freq int freq", ignored);
	parser_reg(p, "spells str spells", parse_lore_spells);
	parser_reg(p, "drop sym tval sym sval uint chance uint min uint max", parse_lore_drop);
	parser_reg(p, "drop-artifact str name", parse_lore_drop_artifact);
	parser_reg(p, "friends uint chance rand number str name", parse_lore_friends);
	parser_reg(p, "friends-base uint chance rand number str name", parse_lore_friends_base);
	parser_reg(p, "mimic sym tval sym sval", parse_lore_mimic);
	return p;
}

static errr run_parse_lore(struct parser *p) {
	/* Failure is always an option */
	if (parse_file(p, "lore")) {
		event_signal_message(EVENT_INITSTATUS, 0, "No monster lore file found");
	}
	return PARSE_ERROR_NONE;
}

static errr finish_parse_lore(struct parser *p) {
	size_t i;

	/* Processing */
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_lore *l = &l_list[i];
		struct monster_race *r = &r_info[i];
		struct monster_friends *f;
		int j;

		//if (!l->sights) continue;

		/* Base flag knowledge */
		if (r->base) {
			rf_union(l->flags, r->base->flags);
			rsf_union(l->spell_flags, r->base->spell_flags);
		}

		/* Remove blows data for non-blows */
		for (j = 0; j < z_info->mon_blows_max; j++) {
			if (!r->blow) break;
			if (!(r->blow[j].effect || r->blow[j].method))
				l->blows[j].times_seen = 0;
		}

	   /* Convert friend names into race pointers - failure leaves NULL race */
		for (f = l->friends; f; f = f->next) {
			if (!my_stricmp(f->name, "same"))
				f->race = r;
			else
				f->race = lookup_monster(f->name);

			string_free(f->name);
		}

		/* update any derived values */
		lore_update(r, l);
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_lore(void)
{
	int ridx;

	for (ridx = 0; ridx < z_info->r_max; ridx++) {
		struct monster_lore *l = &l_list[ridx];
		struct monster_drop *d;
		struct monster_friends *f;
		struct monster_friends_base *fb;
		struct monster_mimic *m;

		d = l->drops;
		while (d) {
			struct monster_drop *dn = d->next;
			mem_free(d);
			d = dn;
		}
		f = l->friends;
		while (f) {
			struct monster_friends *fn = f->next;
			mem_free(f);
			f = fn;
		}
		fb = l->friends_base;
		while (fb) {
			struct monster_friends_base *fbn = fb->next;
			mem_free(fb);
			fb = fbn;
		}
		m = l->mimic_kinds;
		while (m) {
			struct monster_mimic *mn = m->next;
			mem_free(m);
			m = mn;
		}
		mem_free(l->blows);
		mem_free(l->blow_known);
	}

	mem_free(l_list);
}

struct file_parser lore_parser = {
	"lore",
	init_parse_lore,
	run_parse_lore,
	finish_parse_lore,
	cleanup_lore
};

