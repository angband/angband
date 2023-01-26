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
#include "datafile.h"
#include "effects.h"
#include "generate.h"
#include "init.h"
#include "mon-init.h"
#include "mon-lore.h"
#include "mon-msg.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "mon-blows.h"
#include "monster.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-spell.h"
#include "project.h"
#include "ui-visuals.h"

struct blow_method *blow_methods;
struct blow_effect *blow_effects;
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

static const char *obj_flags[] =
{
	"NONE",
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
    ""
};

/**
 * Return the index of a flag from its name.
 */
static int flag_index_by_name(const char *name)
{
	size_t i;
	for (i = 0; i < N_ELEMENTS(obj_flags); i++) {
		if (streq(name, obj_flags[i])) {
			return i;
		}
	}

	return -1;
}

/**
 * ------------------------------------------------------------------------
 * Initialize monster blow methods
 * ------------------------------------------------------------------------ */

static struct blow_method *findmeth(const char *meth_name) {
	struct blow_method *meth = &blow_methods[1];
	while (meth) {
		if (streq(meth->name, meth_name))
			break;
		meth = meth->next;
	}
	return meth;
}

static enum parser_error parse_meth_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct blow_method *h = parser_priv(p);

	struct blow_method *meth = mem_zalloc(sizeof *meth);
	meth->next = h;
	parser_setpriv(p, meth);
	meth->name = string_make(name);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_cut(struct parser *p) {
	struct blow_method *meth = parser_priv(p);
	int val;
	assert(meth);

	val = parser_getuint(p, "cut");
	meth->cut = val ? true : false;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_stun(struct parser *p) {
	struct blow_method *meth = parser_priv(p);
	int val;
	assert(meth);

	val = parser_getuint(p, "stun");
	meth->stun = val ? true : false;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_miss(struct parser *p) {
	struct blow_method *meth = parser_priv(p);
	int val;
	assert(meth);

	val = parser_getuint(p, "miss");
	meth->miss = val ? true : false;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_phys(struct parser *p) {
	struct blow_method *meth = parser_priv(p);
	int val;
	assert(meth);

	val = parser_getuint(p, "phys");
	meth->phys = val ? true : false;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_message_type(struct parser *p)
{
	int msg_index;
	const char *type;
	struct blow_method *meth = parser_priv(p);
	assert(meth);

	if (parser_hasval(p, "msg")) {
		type = parser_getstr(p, "msg");

		msg_index = message_lookup_by_name(type);

		if (msg_index < 0)
			return PARSE_ERROR_INVALID_MESSAGE;

		meth->msgt = msg_index;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_act_msg(struct parser *p) {
	const char *message = parser_getstr(p, "act");
	struct blow_method *meth = parser_priv(p);
	struct blow_message *msg;

	if (!meth) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	msg = mem_zalloc(sizeof(*msg));
	msg->act_msg = string_make(message);
	msg->next = meth->messages;
	meth->messages = msg;
	meth->num_messages++;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_meth_desc(struct parser *p) {
	struct blow_method *meth = parser_priv(p);
	assert(meth);

	meth->desc = string_append(meth->desc, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_meth(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_meth_name);
	parser_reg(p, "cut uint cut", parse_meth_cut);
	parser_reg(p, "stun uint stun", parse_meth_stun);
	parser_reg(p, "miss uint miss", parse_meth_miss);
	parser_reg(p, "phys uint phys", parse_meth_phys);
	parser_reg(p, "msg ?str msg", parse_meth_message_type);
	parser_reg(p, "act str act", parse_meth_act_msg);
	parser_reg(p, "desc str desc", parse_meth_desc);
	return p;
}

static errr run_parse_meth(struct parser *p) {
	return parse_file_quit_not_found(p, "blow_methods");
}

static errr finish_parse_meth(struct parser *p) {
	struct blow_method *meth, *next = NULL;
	int count = 1;

	/* Count the entries */
	z_info->blow_methods_max = 0;
	meth = parser_priv(p);
	while (meth) {
		z_info->blow_methods_max++;
		meth = meth->next;
	}

	/* Allocate the direct access list and copy the data to it */
	blow_methods = mem_zalloc((z_info->blow_methods_max + 1) * sizeof(*meth));
	for (meth = parser_priv(p); meth; meth = next, count++) {
		memcpy(&blow_methods[count], meth, sizeof(*meth));
		next = meth->next;
		if (next)
			blow_methods[count].next = &blow_methods[count + 1];
		else
			blow_methods[count].next = NULL;

		mem_free(meth);
	}
	z_info->blow_methods_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_meth(void)
{
	struct blow_method *meth = &blow_methods[1];

	while (meth) {
		struct blow_message *msg = meth->messages;
		string_free(meth->desc);
		while (msg) {
			struct blow_message *next = msg->next;
			string_free(msg->act_msg);
			mem_free(msg);
			msg = next;
		}
		string_free(meth->name);
		meth = meth->next;
	}
	mem_free(blow_methods);
}

struct file_parser meth_parser = {
	"blow_methods",
	init_parse_meth,
	run_parse_meth,
	finish_parse_meth,
	cleanup_meth
};


/**
 * ------------------------------------------------------------------------
 * Initialize monster blow effects
 * ------------------------------------------------------------------------ */

static struct blow_effect *findeff(const char *eff_name) {
	struct blow_effect *eff = &blow_effects[0];
	while (eff) {
		if (streq(eff->name, eff_name))
			break;
		eff = eff->next;
	}
	return eff;
}

static enum parser_error parse_eff_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct blow_effect *h = parser_priv(p);

	struct blow_effect *eff = mem_zalloc(sizeof *eff);
	eff->next = h;
	parser_setpriv(p, eff);
	eff->name = string_make(name);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_power(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	assert(eff);
	eff->power = parser_getint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_eval(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	assert(eff);
	eff->eval = parser_getint(p, "eval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_desc(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	assert(eff);

	eff->desc = string_append(eff->desc, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_lore_color(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	const char *color;
	int attr;

	if (!eff)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	eff->lore_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_lore_color_resist(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	const char *color;
	int attr;

	if (!eff)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	eff->lore_attr_resist = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_lore_color_immune(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	const char *color;
	int attr;

	if (!eff)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
	eff->lore_attr_immune = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_effect_type(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	assert(eff);

	eff->effect_type = string_make(parser_getstr(p, "type"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_resist(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	const char *resist = parser_getstr(p, "resist");
	assert(eff);

	if (streq(eff->effect_type, "element")) {
		eff->resist = proj_name_to_idx(resist);
	} else if (streq(eff->effect_type, "flag")) {
		eff->resist = flag_index_by_name(resist);
	} else {
		return PARSE_ERROR_MISSING_BLOW_EFF_TYPE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_eff_lash_type(struct parser *p) {
	struct blow_effect *eff = parser_priv(p);
	int type;
	assert(eff);

	type = proj_name_to_idx(parser_getstr(p, "type"));
	eff->lash_type = type >= 0 ? type : PROJ_MISSILE;
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_eff(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_eff_name);
	parser_reg(p, "power int power", parse_eff_power);
	parser_reg(p, "eval int eval", parse_eff_eval);
	parser_reg(p, "desc str desc", parse_eff_desc);
	parser_reg(p, "lore-color-base sym color", parse_eff_lore_color);
	parser_reg(p, "lore-color-resist sym color", parse_eff_lore_color_resist);
	parser_reg(p, "lore-color-immune sym color", parse_eff_lore_color_immune);
	parser_reg(p, "effect-type str type", parse_eff_effect_type);
	parser_reg(p, "resist str resist", parse_eff_resist);
	parser_reg(p, "lash-type str type", parse_eff_lash_type);
	return p;
}

static errr run_parse_eff(struct parser *p) {
	return parse_file_quit_not_found(p, "blow_effects");
}

static errr finish_parse_eff(struct parser *p) {
	struct blow_effect *eff, *next = NULL;
	int count;

	/* Count the entries */
	z_info->blow_effects_max = 0;
	eff = parser_priv(p);
	while (eff) {
		z_info->blow_effects_max++;
		eff = eff->next;
	}

	/* Allocate the direct access list and copy the data to it */
	count = z_info->blow_effects_max - 1;
	blow_effects = mem_zalloc((z_info->blow_effects_max + 1) * sizeof(*eff));
	for (eff = parser_priv(p); eff; eff = next, count--) {
		memcpy(&blow_effects[count], eff, sizeof(*eff));
		next = eff->next;
		if (count < z_info->blow_effects_max - 1)
			blow_effects[count].next = &blow_effects[count + 1];
		else
			blow_effects[count].next = NULL;

		mem_free(eff);
	}
	z_info->blow_effects_max++;

	parser_destroy(p);
	return 0;
}

static void cleanup_eff(void)
{
	struct blow_effect *eff = blow_effects;
	struct blow_effect *next;

	while (eff) {
		next = eff->next;
		string_free(eff->effect_type);
		string_free(eff->desc);
		string_free(eff->name);
		eff = next;
	}
	mem_free(blow_effects);
}

struct file_parser eff_parser = {
	"blow_effects",
	init_parse_eff,
	run_parse_eff,
	finish_parse_eff,
	cleanup_eff
};

/**
 * ------------------------------------------------------------------------
 * Initialize monster pain messages
 * ------------------------------------------------------------------------ */

static enum parser_error parse_pain_type(struct parser *p) {
	struct monster_pain *h = parser_priv(p);
	struct monster_pain *mp = mem_zalloc(sizeof *mp);
	mp->next = h;
	mp->pain_idx = parser_getuint(p, "index");
	parser_setpriv(p, mp);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pain_message(struct parser *p) {
	struct monster_pain *mp = parser_priv(p);
	int i;

	if (!mp)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	for (i = 0; i < 7; i++)
		if (!mp->messages[i])
			break;
	if (i == 7)
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	mp->messages[i] = string_make(parser_getstr(p, "message"));
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_pain(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "type uint index", parse_pain_type);
	parser_reg(p, "message str message", parse_pain_message);
	return p;
}

static errr run_parse_pain(struct parser *p) {
	return parse_file_quit_not_found(p, "pain");
}

static errr finish_parse_pain(struct parser *p) {
	struct monster_pain *mp, *n;
		
	/* scan the list for the max id */
	z_info->mp_max = 0;
	mp = parser_priv(p);
	while (mp) {
		if (mp->pain_idx > z_info->mp_max)
			z_info->mp_max = mp->pain_idx;
		mp = mp->next;
	}

	/* allocate the direct access list and copy the data to it */
	pain_messages = mem_zalloc((z_info->mp_max + 1) * sizeof(*mp));
	for (mp = parser_priv(p); mp; mp = n) {
		memcpy(&pain_messages[mp->pain_idx], mp, sizeof(*mp));
		n = mp->next;
		if (n)
			pain_messages[mp->pain_idx].next = &pain_messages[n->pain_idx];
		else
			pain_messages[mp->pain_idx].next = NULL;
		mem_free(mp);
	}
	z_info->mp_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_pain(void)
{
	int idx, i;
	for (idx = 0; idx < z_info->mp_max; idx++) {
		for (i = 0; i < 7; i++) {
			string_free((char *)pain_messages[idx].messages[i]);
		}
	}
	mem_free(pain_messages);
}

struct file_parser pain_parser = {
	"pain messages",
	init_parse_pain,
	run_parse_pain,
	finish_parse_pain,
	cleanup_pain
};


/**
 * ------------------------------------------------------------------------
 * Initialize monster spells
 * ------------------------------------------------------------------------ */

static enum parser_error parse_mon_spell_name(struct parser *p) {
	struct monster_spell *h = parser_priv(p);
	const char *name = parser_getstr(p, "name");
	struct monster_spell *s;
	int index;

	if (grab_name("monster spell", name, r_info_spell_flags,
			N_ELEMENTS(r_info_spell_flags) - 1, &index)) {
		return PARSE_ERROR_INVALID_SPELL_NAME;
	}
	s = mem_zalloc(sizeof(*s));
	s->next = h;
	s->index = index;
	s->level = mem_zalloc(sizeof(*(s->level)));
	parser_setpriv(p, s);
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_mon_spell_message_type(struct parser *p)
{
	struct monster_spell *s = parser_priv(p);
	const char *type = parser_getsym(p, "type");
	int msg_index = message_lookup_by_name(type);

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (msg_index < 0) {
		return PARSE_ERROR_INVALID_MESSAGE;
	}
	s->msgt = msg_index;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_hit(struct parser *p) {
	struct monster_spell *s = parser_priv(p);

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s->hit = parser_getuint(p, "hit");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_effect(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect, *new_effect;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	new_effect = mem_zalloc(sizeof(*new_effect));
	/* Go to the next vacant effect and set it to the new one  */
	if (s->effect) {
		effect = s->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else {
		s->effect = new_effect;
	}

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_mon_spell_effect_yx(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	/* If there is no effect, assume that this is human and not parser error. */
	effect = s->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;

	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_dice(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect;
	dice_t *dice;
	const char *string;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	/* If there is no effect, assume that this is human and not parser error. */
	effect = s->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;

	dice = dice_new();
	if (dice == NULL) {
		return PARSE_ERROR_INVALID_DICE;
	}
	string = parser_getstr(p, "dice");
	if (dice_parse_string(dice, string)) {
		dice_free(effect->dice);
		effect->dice = dice;
	} else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_expr(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct effect *effect;
	expression_t *expression;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	/* If there is no effect, assume that this is human and not parser error. */
	effect = s->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL) {
		return PARSE_ERROR_NONE;
	}

	expression = expression_new();
	if (expression == NULL) {
		return PARSE_ERROR_INVALID_EXPRESSION;
	}
	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	function = effect_value_base_by_name(base);
	expression_set_base_value(expression, function);
	if (expression_add_operations_string(expression, expr) < 0) {
		expression_free(expression);
		return PARSE_ERROR_BAD_EXPRESSION_STRING;
	}
	if (dice_bind_expression(effect->dice, name, expression) < 0) {
		expression_free(expression);
		return PARSE_ERROR_UNBOUND_EXPRESSION;
	}

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_power_cutoff(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l, *new;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	new = mem_zalloc(sizeof(*new));
	new->power = parser_getint(p, "power");
	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->next = new;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_lore_desc(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->lore_desc = string_append(l->lore_desc, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_lore_color(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	const char *color;
	int attr;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	color = parser_getsym(p, "color");
	if (strlen(color) > 1) {
		attr = color_text_to_attr(color);
	} else {
		attr = color_char_to_attr(color[0]);
	}
	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}
	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->lore_attr = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_lore_color_resist(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	const char *color;
	int attr;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	color = parser_getsym(p, "color");
	if (strlen(color) > 1) {
		attr = color_text_to_attr(color);
	} else {
		attr = color_char_to_attr(color[0]);
	}
	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}
	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->lore_attr_resist = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_lore_color_immune(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;
	const char *color;
	int attr;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	color = parser_getsym(p, "color");
	if (strlen(color) > 1) {
		attr = color_text_to_attr(color);
	} else {
		attr = color_char_to_attr(color[0]);
	}
	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}
	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->lore_attr_immune = attr;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->message = string_append(l->message, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_blind_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->blind_message = string_append(l->blind_message,parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_miss_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->miss_message = string_append(l->miss_message, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_mon_spell_save_message(struct parser *p) {
	struct monster_spell *s = parser_priv(p);
	struct monster_spell_level *l;

	if (!s) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	l = s->level;
	while (l->next) {
		l = l->next;
	}
	l->save_message = string_append(l->save_message, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_mon_spell(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_mon_spell_name);
	parser_reg(p, "msgt sym type", parse_mon_spell_message_type);
	parser_reg(p, "hit uint hit", parse_mon_spell_hit);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_mon_spell_effect);
	parser_reg(p, "effect-yx int y int x", parse_mon_spell_effect_yx);
	parser_reg(p, "dice str dice", parse_mon_spell_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_mon_spell_expr);
	parser_reg(p, "power-cutoff int power", parse_mon_spell_power_cutoff);
	parser_reg(p, "lore str text", parse_mon_spell_lore_desc);
	parser_reg(p, "lore-color-base sym color", parse_mon_spell_lore_color);
	parser_reg(p, "lore-color-resist sym color", parse_mon_spell_lore_color_resist);
	parser_reg(p, "lore-color-immune sym color", parse_mon_spell_lore_color_immune);
	parser_reg(p, "message-vis str text", parse_mon_spell_message);
	parser_reg(p, "message-invis str text", parse_mon_spell_blind_message);
	parser_reg(p, "message-miss str text", parse_mon_spell_miss_message);
	parser_reg(p, "message-save str text", parse_mon_spell_save_message);
	return p;
}

static errr run_parse_mon_spell(struct parser *p) {
	return parse_file_quit_not_found(p, "monster_spell");
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
	struct monster_spell_level *level;

	while (rs) {
		next = rs->next;
		level = rs->level;
		free_effect(rs->effect);
		while (level) {
			struct monster_spell_level *next_level = level->next;
			string_free(level->lore_desc);
			string_free(level->message);
			string_free(level->blind_message);
			string_free(level->miss_message);
			string_free(level->save_message);
			mem_free(level);
			level = next_level;
		}
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
 * ------------------------------------------------------------------------
 * Initialize monster bases
 * ------------------------------------------------------------------------ */

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
			plog(format("bad monster base flag: %s", s));
			string_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	string_free(flags);
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
	parser_reg(p, "desc str desc", parse_mon_base_desc);
	return p;
}

static errr run_parse_mon_base(struct parser *p) {
	return parse_file_quit_not_found(p, "monster_base");
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
 * ------------------------------------------------------------------------
 * Initialize monsters
 * ------------------------------------------------------------------------ */

static enum parser_error parse_monster_name(struct parser *p) {
	struct monster_race *h = parser_priv(p);
	struct monster_race *r = mem_zalloc(sizeof *r);
	r->next = h;
	r->name = string_make(parser_getstr(p, "name"));
	parser_setpriv(p, r);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_base(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	r->base = lookup_monster_base(parser_getsym(p, "base"));
	if (r->base == NULL)
		return PARSE_ERROR_INVALID_MONSTER_BASE;

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

static enum parser_error parse_monster_speed(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->speed = parser_getint(p, "speed");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_hit_points(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->avg_hp = parser_getint(p, "hp");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_light(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->light = parser_getint(p, "light");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_hearing(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	/* Assumes max_sight is 20, so we adjust in case it isn't */
	r->hearing = parser_getint(p, "hearing") * z_info->max_sight / 20;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_smell(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	/* Assumes max_sight is 20, so we adjust in case it isn't */
	r->smell = parser_getint(p, "smell") * z_info->max_sight / 20;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_armor_class(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->ac = parser_getint(p, "ac");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_sleepiness(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->sleep = parser_getint(p, "sleep");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_depth(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->level = parser_getint(p, "level");

	/* Level is default spell power */
	r->spell_power = r->level;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_rarity(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->rarity = parser_getint(p, "rarity");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_experience(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
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
	b->method = findmeth(parser_getsym(p, "method"));
	if (!b->method)
		return PARSE_ERROR_UNRECOGNISED_BLOW;
	if (parser_hasval(p, "effect")) {
		b->effect = findeff(parser_getsym(p, "effect"));
		if (!b->effect)
			return PARSE_ERROR_INVALID_EFFECT;
	} else {
		b->effect = findeff("NONE");
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
			plog(format("bad monster race flag: %s", s));
			string_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	string_free(flags);
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
			plog(format("bad monster race flag: %s", s));
			string_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}

	string_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_desc(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->text = string_append(r->text, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_innate_freq(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	int pct;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	pct = parser_getint(p, "freq");
	if (pct < 1 || pct > 100)
		return PARSE_ERROR_INVALID_SPELL_FREQ;
	r->freq_innate = 100 / pct;
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
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_spell_power(struct parser *p) {
	struct monster_race *r = parser_priv(p);

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	r->spell_power = parser_getuint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_spells(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	char *flags;
	char *s;
	int ret = PARSE_ERROR_NONE;
	bitflag current_flags[RSF_SIZE], test_flags[RSF_SIZE];

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(r->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			plog(format("bad monster spell flag: %s", s));
			ret = PARSE_ERROR_INVALID_FLAG;
			break;
		}
		s = strtok(NULL, " |");
	}

	/* Make sure innate frequency is set if necessary */
	create_mon_spell_mask(current_flags, RST_INNATE, RST_NONE);
	rsf_inter(current_flags, r->spell_flags);
	if (!rsf_is_empty(current_flags) && !r->freq_innate) {
		/* Set frequency to the lowest found value */
		r->freq_innate = 4;
	}

	/* Make sure non-innate frequency is set if necessary */
	rsf_copy(current_flags, r->spell_flags);
	create_mon_spell_mask(test_flags, RST_BREATH, RST_INNATE, RST_NONE);
	rsf_diff(current_flags, test_flags);
	if (!rsf_is_empty(current_flags) && !r->freq_spell) {
		/* Set frequency to the lowest found value */
		r->freq_spell = 4;
	}

	string_free(flags);
	return ret;
}

static void add_alternate_spell_message(struct monster_race *r,
		int s_idx, enum monster_altmsg_type msg_type, const char *msg)
{
	struct monster_altmsg *alt = mem_alloc(sizeof(*alt));

	alt->next = r->spell_msgs;
	r->spell_msgs = alt;
	if (msg == NULL || contains_only_spaces(msg)) {
		alt->message = string_make("");
	} else {
		alt->message = string_make(msg);
	}
	alt->msg_type = msg_type;
	assert(s_idx >= 0 && s_idx <= 0xFFFF);
	alt->index = (uint16_t) s_idx;
}

static enum parser_error parse_monster_msg_vis(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	const char *spell = parser_getsym(p, "spell");
	const char *msg = (parser_hasval(p, "message")) ?
		parser_getstr(p, "message") : NULL;
	int s_idx;

	if (!r) return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (grab_name("monster spell", spell, r_info_spell_flags,
			N_ELEMENTS(r_info_spell_flags) - 1, &s_idx))
			return PARSE_ERROR_INVALID_SPELL_NAME;
	add_alternate_spell_message(r, s_idx, MON_ALTMSG_SEEN, msg);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_msg_invis(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	const char *spell = parser_getsym(p, "spell");
	const char *msg = (parser_hasval(p, "message")) ?
		parser_getstr(p, "message") : NULL;
	int s_idx;

	if (!r) return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (grab_name("monster spell", spell, r_info_spell_flags,
			N_ELEMENTS(r_info_spell_flags) - 1, &s_idx))
			return PARSE_ERROR_INVALID_SPELL_NAME;
	add_alternate_spell_message(r, s_idx, MON_ALTMSG_UNSEEN, msg);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_monster_msg_miss(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	const char *spell = parser_getsym(p, "spell");
	const char *msg = (parser_hasval(p, "message")) ?
		parser_getstr(p, "message") : NULL;
	int s_idx;

	if (!r) return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (grab_name("monster spell", spell, r_info_spell_flags,
			N_ELEMENTS(r_info_spell_flags) - 1, &s_idx))
			return PARSE_ERROR_INVALID_SPELL_NAME;
	add_alternate_spell_message(r, s_idx, MON_ALTMSG_MISS, msg);

	return PARSE_ERROR_NONE;
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

static enum parser_error parse_monster_drop_base(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_drop *d;
	int tval;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	if (parser_getuint(p, "min") > 99 || parser_getuint(p, "max") > 99)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;

	d = mem_zalloc(sizeof *d);
	d->tval = tval;
	d->percent_chance = parser_getuint(p, "chance");
	d->min = parser_getuint(p, "min");
	d->max = parser_getuint(p, "max");
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
	if (parser_hasval(p, "role")) {
		const char *role_name = parser_getsym(p, "role");
		if (streq(role_name, "servant")) {
			f->role = MON_GROUP_SERVANT;
		} else if (streq(role_name, "bodyguard")) {
			f->role = MON_GROUP_BODYGUARD;
		} else {
			mem_free(f);
			return PARSE_ERROR_INVALID_MONSTER_ROLE;
		}
	} else {
		f->role = MON_GROUP_MEMBER;
	}
	f->name = string_make(parser_getsym(p, "name"));
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
	f->base = lookup_monster_base(parser_getsym(p, "name"));
	if (!f->base) {
		mem_free(f);
		return PARSE_ERROR_INVALID_MONSTER_BASE;
	}
	if (parser_hasval(p, "role")) {
		const char *role_name = parser_getsym(p, "role");
		if (streq(role_name, "servant")) {
			f->role = MON_GROUP_SERVANT;
		} else if (streq(role_name, "bodyguard")) {
			f->role = MON_GROUP_BODYGUARD;
		} else {
			mem_free(f);
			return PARSE_ERROR_INVALID_MONSTER_ROLE;
		}
	} else {
		f->role = MON_GROUP_MEMBER;
	}

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

static enum parser_error parse_monster_shape(struct parser *p) {
	struct monster_race *r = parser_priv(p);
	struct monster_shape *s;

	if (!r)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	s = mem_zalloc(sizeof *s);
	s->name = string_make(parser_getstr(p, "name"));
	s->base = lookup_monster_base(s->name);
	s->next = r->shapes;
	r->shapes = s;
	r->num_shapes++;

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

static enum parser_error parse_monster_color_cycle(struct parser *p)
{
	struct monster_race *r = parser_priv(p);
	const char *group = parser_getsym(p, "group");
	const char *cycle = parser_getsym(p, "cycle");

	if (r == NULL)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	if (group == NULL || strlen(group) == 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (cycle == NULL || strlen(cycle) == 0)
		return PARSE_ERROR_INVALID_VALUE;

	visuals_cycler_set_cycle_for_race(r, group, cycle);

	return PARSE_ERROR_NONE;
}

struct parser *init_parse_monster(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name str name", parse_monster_name);
	parser_reg(p, "plural ?str plural", parse_monster_plural);
	parser_reg(p, "base sym base", parse_monster_base);
	parser_reg(p, "glyph char glyph", parse_monster_glyph);
	parser_reg(p, "color sym color", parse_monster_color);
	parser_reg(p, "speed int speed", parse_monster_speed);
	parser_reg(p, "hit-points int hp", parse_monster_hit_points);
	parser_reg(p, "light int light", parse_monster_light);
	parser_reg(p, "hearing int hearing", parse_monster_hearing);
	parser_reg(p, "smell int smell", parse_monster_smell);
	parser_reg(p, "armor-class int ac", parse_monster_armor_class);
	parser_reg(p, "sleepiness int sleep", parse_monster_sleepiness);
	parser_reg(p, "depth int level", parse_monster_depth);
	parser_reg(p, "rarity int rarity", parse_monster_rarity);
	parser_reg(p, "experience int mexp", parse_monster_experience);
	parser_reg(p, "blow sym method ?sym effect ?rand damage", parse_monster_blow);
	parser_reg(p, "flags ?str flags", parse_monster_flags);
	parser_reg(p, "flags-off ?str flags", parse_monster_flags_off);
	parser_reg(p, "desc str desc", parse_monster_desc);
	parser_reg(p, "innate-freq int freq", parse_monster_innate_freq);
	parser_reg(p, "spell-freq int freq", parse_monster_spell_freq);
	parser_reg(p, "spell-power uint power", parse_monster_spell_power);
	parser_reg(p, "spells str spells", parse_monster_spells);
	parser_reg(p, "message-vis sym spell ?str message", parse_monster_msg_vis);
	parser_reg(p, "message-invis sym spell ?str message", parse_monster_msg_invis);
	parser_reg(p, "message-miss sym spell ?str message", parse_monster_msg_miss);
	parser_reg(p, "drop sym tval sym sval uint chance uint min uint max", parse_monster_drop);
	parser_reg(p, "drop-base sym tval uint chance uint min uint max", parse_monster_drop_base);
	parser_reg(p, "friends uint chance rand number sym name ?sym role", parse_monster_friends);
	parser_reg(p, "friends-base uint chance rand number sym name ?sym role", parse_monster_friends_base);
	parser_reg(p, "mimic sym tval sym sval", parse_monster_mimic);
	parser_reg(p, "shape str name", parse_monster_shape);
	parser_reg(p, "color-cycle sym group sym cycle", parse_monster_color_cycle);
	return p;
}

static errr run_parse_monster(struct parser *p) {
	return parse_file_quit_not_found(p, "monster");
}

static errr finish_parse_monster(struct parser *p) {
	struct monster_race *r, *n;
	size_t i;
	int ridx;

	/* Scan the list for the max id and max blows */
	z_info->r_max = 0;
	z_info->mon_blows_max = 0;
	r = parser_priv(p);
	while (r) {
		int max_blows = 0;
		struct monster_blow *b = r->blow;
		z_info->r_max++;
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
	ridx = z_info->r_max - 1;
	for (r = parser_priv(p); r; r = n, ridx--) {
		struct monster_blow *b_new;

		assert(ridx >= 0);

		/* Main record */
		memcpy(&r_info[ridx], r, sizeof(*r));
		r_info[ridx].ridx = ridx;
		n = r->next;
		if (ridx < z_info->r_max - 1)
			r_info[ridx].next = &r_info[ridx + 1];
		else
			r_info[ridx].next = NULL;

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
		r_info[ridx].blow = b_new;

		mem_free(r);
	}
	z_info->r_max += 1;

	/* Convert friend and shape names into race pointers */
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];
		struct monster_friends *f;
		struct monster_shape *s;
		for (f = race->friends; f; f = f->next) {
			if (!my_stricmp(f->name, "same")) {
				f->race = race;
			} else {
				f->race = lookup_monster(f->name);
			}
			if (!f->race) {
				quit_fmt("Couldn't find friend named '%s' for monster '%s'",
						 f->name, race->name);
			}
			string_free(f->name);
		}
		for (s = race->shapes; s; s = s->next) {
			if (!s->base) {
				s->race = lookup_monster(s->name);
				if (!s->race) {
					quit_fmt("Couldn't find shape named '%s' for monster '%s'",
							 s->name, race->name);
				}
			}
			string_free(s->name);
		}
	}

	/* Allocate space for the monster lore */
	l_list = mem_zalloc(z_info->r_max * sizeof(struct monster_lore));
	for (i = 0; i < z_info->r_max; i++) {
		struct monster_lore *l = &l_list[i];
		l->blows = mem_zalloc(z_info->mon_blows_max * sizeof(struct monster_blow));
		l->blow_known = mem_zalloc(z_info->mon_blows_max * sizeof(bool));
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_monster(void)
{
	int ridx;

	for (ridx = 0; ridx < z_info->r_max; ridx++) {
		struct monster_race *r = &r_info[ridx];
		struct monster_altmsg *am;
		struct monster_drop *d;
		struct monster_friends *f;
		struct monster_friends_base *fb;
		struct monster_mimic *m;
		struct monster_shape *s;

		am = r->spell_msgs;
		while (am) {
			struct monster_altmsg *amn = am->next;

			string_free(am->message);
			mem_free(am);
			am = amn;
		}
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
		s = r->shapes;
		while (s) {
			struct monster_shape *sn = s->next;
			mem_free(s);
			s = sn;
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

/**
 * ------------------------------------------------------------------------
 * Initialize monster pits
 * ------------------------------------------------------------------------ */

static enum parser_error parse_pit_name(struct parser *p) {
	struct pit_profile *h = parser_priv(p);
	struct pit_profile *pit = mem_zalloc(sizeof *pit);
	pit->next = h;
	pit->name = string_make(parser_getstr(p, "name"));
	pit->colors = NULL;
	pit->forbidden_monsters = NULL;
	parser_setpriv(p, pit);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_room(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pit->room_type = parser_getuint(p, "type");
	return PARSE_ERROR_NONE;
}
static enum parser_error parse_pit_alloc(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pit->rarity = parser_getuint(p, "rarity");
	pit->ave = parser_getuint(p, "level");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_obj_rarity(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	pit->obj_rarity = parser_getuint(p, "obj_rarity");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_mon_base(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	struct pit_monster_profile *bases;
	struct monster_base *base = lookup_monster_base(parser_getsym(p, "base"));

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	else if (!base)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	bases = mem_zalloc(sizeof *bases);
	bases->base = base;
	bases->next = pit->bases;
	pit->bases = bases;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_mon_ban(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	struct pit_forbidden_monster *monsters;
	struct monster_race *r = lookup_monster(parser_getsym(p, "race"));

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	monsters = mem_zalloc(sizeof *monsters);
	monsters->race = r;
	monsters->next = pit->forbidden_monsters;
	pit->forbidden_monsters = monsters;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_color(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	struct pit_color_profile *colors;
	const char *color;
	int attr;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	color = parser_getsym(p, "color");
	if (strlen(color) > 1)
		attr = color_text_to_attr(color);
	else
		attr = color_char_to_attr(color[0]);
	if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;

	colors = mem_zalloc(sizeof *colors);
	colors->color = attr;
	colors->next = pit->colors;
	pit->colors = colors;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_flags_req(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_flags_ban(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->forbidden_flags, RF_SIZE, r_info_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_innate_freq(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	int pct;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	pct = parser_getint(p, "freq");
	if (pct < 1 || pct > 100)
		return PARSE_ERROR_INVALID_SPELL_FREQ;
	pit->freq_innate = 100 / pct;
	return PARSE_ERROR_NONE;
}
static enum parser_error parse_pit_spell_req(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_pit_spell_ban(struct parser *p) {
	struct pit_profile *pit = parser_priv(p);
	char *flags;
	char *s;

	if (!pit)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	if (!parser_hasval(p, "spells"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		if (grab_flag(pit->forbidden_spell_flags, RSF_SIZE, r_info_spell_flags, s)) {
			mem_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		s = strtok(NULL, " |");
	}
	
	mem_free(flags);
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_pit(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name str name", parse_pit_name);
	parser_reg(p, "room uint type", parse_pit_room);
	parser_reg(p, "alloc uint rarity uint level", parse_pit_alloc);
	parser_reg(p, "obj-rarity uint obj_rarity", parse_pit_obj_rarity);
	parser_reg(p, "mon-base sym base", parse_pit_mon_base);
	parser_reg(p, "mon-ban sym race", parse_pit_mon_ban);
	parser_reg(p, "color sym color", parse_pit_color);
	parser_reg(p, "flags-req ?str flags", parse_pit_flags_req);
	parser_reg(p, "flags-ban ?str flags", parse_pit_flags_ban);
	parser_reg(p, "innate-freq int freq", parse_pit_innate_freq);
	parser_reg(p, "spell-req ?str spells", parse_pit_spell_req);
	parser_reg(p, "spell-ban ?str spells", parse_pit_spell_ban);
	return p;
}

static errr run_parse_pit(struct parser *p) {
	return parse_file_quit_not_found(p, "pit");
}
 
static errr finish_parse_pit(struct parser *p) {
	struct pit_profile *pit, *n;
	int pit_idx;
		
	/* Scan the list for the max id */
	z_info->pit_max = 0;
	pit = parser_priv(p);
	while (pit) {
		z_info->pit_max++;
		pit = pit->next;
	}

	/* Allocate the direct access list and copy the data to it */
	pit_info = mem_zalloc((z_info->pit_max + 1) * sizeof(*pit));
	pit_idx = z_info->pit_max - 1;
	for (pit = parser_priv(p); pit; pit = n, pit_idx--) {
		assert(pit_idx >= 0);

		memcpy(&pit_info[pit_idx], pit, sizeof(*pit));
		pit_info[pit_idx].pit_idx = pit_idx;
		n = pit->next;
		if (pit_idx < z_info->pit_max - 1)
			pit_info[pit_idx].next = &pit_info[pit_idx + 1];
		else
			pit_info[pit_idx].next = NULL;

		mem_free(pit);
	}
	z_info->pit_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_pits(void)
{
	int idx;
	
	for (idx = 0; idx < z_info->pit_max; idx++) {
		struct pit_profile *pit = &pit_info[idx];
		struct pit_color_profile *c, *cn;
		struct pit_forbidden_monster *m, *mn;
		struct pit_monster_profile *b, *bn;
		
		c = pit->colors;
		while (c) {
			cn = c->next;
			mem_free(c);
			c = cn;
		}
		m = pit->forbidden_monsters;
		while (m) {
			mn = m->next;
			mem_free(m);
			m = mn;
		}
		b = pit->bases;
		while (b) {
			bn = b->next;
			mem_free(b);
			b = bn;
		}
		string_free((char *)pit_info[idx].name);
		
	}
	mem_free(pit_info);
}

struct file_parser pit_parser = {
	"pits",
	init_parse_pit,
	run_parse_pit,
	finish_parse_pit,
	cleanup_pits
};


/**
 * ------------------------------------------------------------------------
 * Initialize monster lore
 * ------------------------------------------------------------------------ */

static enum parser_error parse_lore_name(struct parser *p) {
	struct monster_race *race = lookup_monster(parser_getstr(p, "name"));
	struct monster_lore *l = NULL;

	/* Only set lore if we have a race, to allow for non-existent monsters */
	if (race) {
		l = &l_list[race->ridx];
		l->ridx = race->ridx;
	}
	parser_setpriv(p, l);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_base(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_base *base = lookup_monster_base(parser_getsym(p, "base"));

	if (!l)
		return PARSE_ERROR_NONE;
	if (base == NULL)
		return PARSE_ERROR_INVALID_MONSTER_BASE;

	/* Know everything */
	l->all_known = true;
	rf_setall(l->flags);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_counts(struct parser *p) {
	struct monster_lore *l = parser_priv(p);

	if (!l)
		return PARSE_ERROR_NONE;
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
	struct blow_method *method = NULL;
	struct blow_effect *effect = NULL;
	int seen = 0, index = 0;
	struct random dam = { 0, 0, 0, 0 };

	if (!l)
		return PARSE_ERROR_NONE;

	/* Read in all the data */
	method = findmeth(parser_getsym(p, "method"));
	if (!method)
		return PARSE_ERROR_UNRECOGNISED_BLOW;
	if (parser_hasval(p, "effect")) {
		effect = findeff(parser_getsym(p, "effect"));
		if (!effect)
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
		return PARSE_ERROR_NONE;
	if (!parser_hasval(p, "flags"))
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "flags"));
	s = strtok(flags, " |");
	while (s) {
		(void) grab_flag(l->flags, RF_SIZE, r_info_flags, s);
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
		return PARSE_ERROR_NONE;
	flags = string_make(parser_getstr(p, "spells"));
	s = strtok(flags, " |");
	while (s) {
		(void) grab_flag(l->spell_flags, RSF_SIZE, r_info_spell_flags, s);
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
		return PARSE_ERROR_NONE;
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

static enum parser_error parse_lore_drop_base(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_drop *d;
	int tval;

	if (!l)
		return PARSE_ERROR_NONE;
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	if (parser_getuint(p, "min") > 99 || parser_getuint(p, "max") > 99)
		return PARSE_ERROR_INVALID_ITEM_NUMBER;

	d = mem_zalloc(sizeof *d);
	d->tval = tval;
	d->percent_chance = parser_getuint(p, "chance");
	d->min = parser_getuint(p, "min");
	d->max = parser_getuint(p, "max");
	d->next = l->drops;
	l->drops = d;
	return PARSE_ERROR_NONE;
}
static enum parser_error parse_lore_friends(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_friends *f;
	struct random number;

	if (!l)
		return PARSE_ERROR_NONE;
	f = mem_zalloc(sizeof *f);
	number = parser_getrand(p, "number");
	f->number_dice = number.dice;
	f->number_side = number.sides;
	f->percent_chance = parser_getuint(p, "chance");
	f->name = string_make(parser_getsym(p, "name"));
	if (parser_hasval(p, "role")) {
		const char *role_name = parser_getsym(p, "role");
		if (streq(role_name, "servant")) {
			f->role = MON_GROUP_SERVANT;
		} else if (streq(role_name, "bodyguard")) {
			f->role = MON_GROUP_BODYGUARD;
		} else {
			return PARSE_ERROR_INVALID_MONSTER_ROLE;
		}
	} else {
		f->role = MON_GROUP_MEMBER;
	}
	f->next = l->friends;
	l->friends = f;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_lore_friends_base(struct parser *p) {
	struct monster_lore *l = parser_priv(p);
	struct monster_friends_base *f;
	struct random number;

	if (!l)
		return PARSE_ERROR_NONE;
	f = mem_zalloc(sizeof *f);
	number = parser_getrand(p, "number");
	f->number_dice = number.dice;
	f->number_side = number.sides;
	f->percent_chance = parser_getuint(p, "chance");
	f->base = lookup_monster_base(parser_getsym(p, "name"));
	if (!f->base) return PARSE_ERROR_UNRECOGNISED_TVAL;
	if (parser_hasval(p, "role")) {
		const char *role_name = parser_getsym(p, "role");
		if (streq(role_name, "servant")) {
			f->role = MON_GROUP_SERVANT;
		} else if (streq(role_name, "bodyguard")) {
			f->role = MON_GROUP_BODYGUARD;
		} else {
			return PARSE_ERROR_INVALID_MONSTER_ROLE;
		}
	} else {
		f->role = MON_GROUP_MEMBER;
	}

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
		return PARSE_ERROR_NONE;
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

static struct parser *init_parse_lore(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name str name", parse_lore_name);
	parser_reg(p, "plural ?str plural", ignored);
	parser_reg(p, "base sym base", parse_lore_base);
	parser_reg(p, "glyph char glyph", ignored);
	parser_reg(p, "color sym color", ignored);
	parser_reg(p, "speed int speed", ignored);
	parser_reg(p, "hit-points int hp", ignored);
	parser_reg(p, "light int light", ignored);
	parser_reg(p, "hearing int hearing", ignored);
	parser_reg(p, "smell int smell", ignored);
	parser_reg(p, "armor-class int ac", ignored);
	parser_reg(p, "sleepiness int sleep", ignored);
	parser_reg(p, "depth int level", ignored);
	parser_reg(p, "rarity int rarity", ignored);
	parser_reg(p, "experience int mexp", ignored);
	parser_reg(p, "counts int sights int deaths int tkills int wake int ignore int innate int spell", parse_lore_counts);
	parser_reg(p, "blow sym method ?sym effect ?rand damage ?int seen ?int index", parse_lore_blow);
	parser_reg(p, "flags ?str flags", parse_lore_flags);
	parser_reg(p, "flags-off ?str flags", ignored);
	parser_reg(p, "desc str desc", ignored);
	parser_reg(p, "innate-freq int freq", ignored);
	parser_reg(p, "spell-freq int freq", ignored);
	parser_reg(p, "spell-power uint power", ignored);
	parser_reg(p, "spells str spells", parse_lore_spells);
	parser_reg(p, "message-vis sym spell ?str message", ignored);
	parser_reg(p, "message-invis sym spell ?str message", ignored);
	parser_reg(p, "message-miss sym spell ?str message", ignored);
	parser_reg(p, "drop sym tval sym sval uint chance uint min uint max", parse_lore_drop);
	parser_reg(p, "drop-base sym tval uint chance uint min uint max", parse_lore_drop_base);
	parser_reg(p, "drop-artifact str name", ignored);
	parser_reg(p, "friends uint chance rand number sym name ?sym role", parse_lore_friends);
	parser_reg(p, "friends-base uint chance rand number sym name ?sym role", parse_lore_friends_base);
	parser_reg(p, "mimic sym tval sym sval", parse_lore_mimic);
	parser_reg(p, "shape str name", ignored);
	parser_reg(p, "color-cycle sym group sym cycle", ignored);
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

		/* Base flag knowledge */
		if (r->base) {
			rf_union(l->flags, r->base->flags);
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

