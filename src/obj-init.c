/**
 * \file obj-init.c
 * \brief Various game initialization routines
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
 *
 * This file is used to initialize various variables and arrays for objects
 * in the Angband game.
 *
 * Several of the arrays for Angband are built from data files in the
 * "lib/gamedata" directory.
 */


#include "angband.h"
#include "buildid.h"
#include "datafile.h"
#include "effects.h"
#include "init.h"
#include "mon-util.h"
#include "obj-curse.h"
#include "obj-ignore.h"
#include "obj-list.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-randart.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "option.h"
#include "player-spell.h"
#include "project.h"
#include "ui-entry.h"

static const char *mon_race_flags[] =
{
	#define RF(a, b, c) #a,
	#include "list-mon-race-flags.h"
	#undef RF
	NULL
};

static const char *obj_flags[] = {
	"NONE",
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

static const char *obj_mods[] = {
	#define STAT(a) #a,
	#include "list-stats.h"
	#undef STAT
	#define OBJ_MOD(a) #a,
	#include "list-object-modifiers.h"
	#undef OBJ_MOD
	NULL
};

static const char *kind_flags[] = {
	#define KF(a, b) #a,
	#include "list-kind-flags.h"
	#undef KF
	NULL
};

static const char *element_names[] = {
	#define ELEM(a) #a,
	#include "list-elements.h"
	#undef ELEM
	NULL
};

static bool grab_element_flag(struct element_info *info, const char *flag_name)
{
	char *under = strchr(flag_name, '_');
	size_t i;

	if (!under) {
		return false;
	}

	/* Ignore or hate */
	for (i = 0; i < ELEM_MAX; i++) {
		if (streq(under + 1, element_names[i])) {
			if (!strncmp(flag_name, "IGNORE", under - flag_name)) {
				info[i].flags |= EL_INFO_IGNORE;
				return true;
			}
			if (!strncmp(flag_name, "HATES", under - flag_name)) {
				info[i].flags |= EL_INFO_HATES;
				return true;
			}
		}
	}
	return false;
}

static enum parser_error write_dummy_object_record(struct artifact *art, const char *name)
{
	struct object_kind *temp, *dummy;
	int i;
	char mod_name[100];

	/* Extend by 1 and realloc */
	z_info->k_max += 1;
	temp = mem_realloc(k_info, (z_info->k_max + 1) * sizeof(*temp));

	/* Copy if no errors */
	if (!temp) {
		return PARSE_ERROR_INTERNAL;
	}
	k_info = temp;
	/* Use the (second) last entry for the dummy */
	dummy = &k_info[z_info->k_max - 1];
	memset(dummy, 0, sizeof(*dummy));

	/* Copy the tval, base and level */
	dummy->tval = art->tval;
	dummy->base = &kb_info[dummy->tval];

	/* Make the name and index */
	strnfmt(mod_name, sizeof(mod_name), "& %s~", name);
	dummy->name = string_make(mod_name);
	dummy->kidx = z_info->k_max - 1;
	dummy->level = art->level;

	/* Increase the sval count for this tval, set the new one to the max */
	for (i = 0; i < TV_MAX; i++) {
		if (kb_info[i].tval == dummy->tval) {
			kb_info[i].num_svals++;
			dummy->sval = kb_info[i].num_svals;
			break;
		}
	}
	if (i == TV_MAX) return PARSE_ERROR_INTERNAL;

	/* Copy the sval to the artifact info */
	art->sval = dummy->sval;

	/* Give the object default colours (these should be overwritten) */
	dummy->d_char = '*';
	dummy->d_attr = COLOUR_RED;

	/* Inherit the flags and element information of the tval */
	of_copy(dummy->flags, kb_info[i].flags);
	kf_copy(dummy->kind_flags, kb_info[i].kind_flags);
	(void)memcpy(dummy->el_info, kb_info[i].el_info,
		sizeof(dummy->el_info[0]) * ELEM_MAX);

	/* Register this as an INSTA_ART object */
	kf_on(dummy->kind_flags, KF_INSTA_ART);

	return PARSE_ERROR_NONE;
}

/**
 * Fill in curse object info now that curse_object_kind is defined
 */
static void write_curse_kinds(void)
{
	int i;
	int sval =  lookup_sval(tval_find_idx("none"), "<curse object>");

	for (i = 1; i < z_info->curse_max; i++) {
		struct curse *curse = &curses[i];
		curse->obj->kind = curse_object_kind;
		curse->obj->sval = sval;
		/*
		 * Tolerate an already allocated known version:  restarting
		 * without exiting and redoing the artifacts in
		 * do_cmd_accept_character().
		 */
		if (! curse->obj->known) {
			curse->obj->known = object_new();
		}
		curse->obj->known->kind = curse_object_kind;
		curses[i].obj->known->sval = sval;
		/* Mark it as touched so it can be fully known. */
		curse->obj->known->notice |= OBJ_NOTICE_ASSESSED;
	}
}

static struct activation *findact(const char *act_name) {
	struct activation *act = &activations[1];
	while (act) {
		if (streq(act->name, act_name)) {
			break;
		}
		act = act->next;
	}
	return act;
}

/**
 * ------------------------------------------------------------------------
 * Initialize projections
 * ------------------------------------------------------------------------ */

static enum parser_error parse_projection_code(struct parser *p) {
	const char *code = parser_getstr(p, "code");
	struct projection *h = parser_priv(p);
	int index = h ? h->index + 1 : 0;
	struct projection *projection = mem_zalloc(sizeof *projection);

	parser_setpriv(p, projection);
	projection->next = h;
	projection->index = index;
	if ((index < ELEM_MAX) && !streq(code, element_names[index])) {
		return PARSE_ERROR_ELEMENT_NAME_MISMATCH;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(projection->name);
	projection->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_type(struct parser *p) {
	const char *type = parser_getstr(p, "type");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(projection->type);
	projection->type = string_make(type);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_desc(struct parser *p) {
	const char *desc = parser_getstr(p, "desc");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(projection->desc);
	projection->desc = string_make(desc);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_player_desc(struct parser *p) {
	const char *desc = parser_getstr(p, "desc");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(projection->player_desc);
	projection->player_desc = string_make(desc);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_blind_desc(struct parser *p) {
	const char *desc = parser_getstr(p, "desc");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(projection->blind_desc);
	projection->blind_desc = string_make(desc);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_lash_desc(struct parser *p) {
	const char *desc = parser_getstr(p, "desc");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(projection->lash_desc);
	projection->lash_desc = string_make(desc);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_numerator(struct parser *p) {
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	projection->numerator = parser_getuint(p, "num");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_denominator(struct parser *p) {
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	projection->denominator = parser_getrand(p, "denom");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_divisor(struct parser *p) {
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	projection->divisor = parser_getuint(p, "div");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_damage_cap(struct parser *p) {
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	projection->damage_cap = parser_getuint(p, "cap");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_message_type(struct parser *p)
{
	int msg_index;
	const char *type;
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	type = parser_getsym(p, "type");
	msg_index = message_lookup_by_name(type);
	if (msg_index < 0) {
		return PARSE_ERROR_INVALID_MESSAGE;
	}
	projection->msgt = msg_index;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_obvious(struct parser *p) {
	int obvious = parser_getuint(p, "answer");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	projection->obvious = (obvious == 1) ? true : false;;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_wake(struct parser *p) {
	int wake = parser_getuint(p, "answer");
	struct projection *projection = parser_priv(p);

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	projection->wake = (wake == 1) ? true : false;;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_projection_color(struct parser *p) {
	struct projection *projection = parser_priv(p);
	const char *color;

	if (!projection) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	color = parser_getsym(p, "color");
	if (strlen(color) > 1) {
		projection->color = color_text_to_attr(color);
	} else {
		projection->color = color_char_to_attr(color[0]);
	}
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_projection(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "code str code", parse_projection_code);
	parser_reg(p, "name str name", parse_projection_name);
	parser_reg(p, "type str type", parse_projection_type);
	parser_reg(p, "desc str desc", parse_projection_desc);
	parser_reg(p, "player-desc str desc", parse_projection_player_desc);
	parser_reg(p, "blind-desc str desc", parse_projection_blind_desc);
	parser_reg(p, "lash-desc str desc", parse_projection_lash_desc);
	parser_reg(p, "numerator uint num", parse_projection_numerator);
	parser_reg(p, "denominator rand denom", parse_projection_denominator);
	parser_reg(p, "divisor uint div", parse_projection_divisor);
	parser_reg(p, "damage-cap uint cap", parse_projection_damage_cap);
	parser_reg(p, "msgt sym type", parse_projection_message_type);
	parser_reg(p, "obvious uint answer", parse_projection_obvious);
	parser_reg(p, "wake uint answer", parse_projection_wake);
	parser_reg(p, "color sym color", parse_projection_color);
	return p;
}

static errr run_parse_projection(struct parser *p) {
	return parse_file_quit_not_found(p, "projection");
}

static errr finish_parse_projection(struct parser *p) {
	struct projection *projection, *next = NULL;
	int element_count = 0, count = 0;

	/* Count the entries */
	z_info->projection_max = 0;
	projection = parser_priv(p);
	while (projection) {
		z_info->projection_max++;
		if (projection->type && streq(projection->type, "element")) {
			element_count++;
		}
		projection = projection->next;
	}

	if (element_count + 1 < (int) N_ELEMENTS(element_names)) {
		quit_fmt("Too few elements in projection.txt!");
	} else if (element_count + 1 > (int) N_ELEMENTS(element_names)) {
		quit_fmt("Too many elements in projection.txt!");
	}

	/* Allocate the direct access list and copy the data to it */
	projections = mem_zalloc((z_info->projection_max) * sizeof(*projection));
	count = z_info->projection_max - 1;
	for (projection = parser_priv(p); projection; projection = next, count--) {
		memcpy(&projections[count], projection, sizeof(*projection));
		next = projection->next;
		mem_free(projection);
	}

	parser_destroy(p);
	return 0;
}

static void cleanup_projection(void)
{
	int idx;
	for (idx = 0; idx < z_info->projection_max; idx++) {
		string_free(projections[idx].name);
		string_free(projections[idx].type);
		string_free(projections[idx].desc);
		string_free(projections[idx].lash_desc);
		string_free(projections[idx].player_desc);
		string_free(projections[idx].blind_desc);
	}
	mem_free(projections);
}

struct file_parser projection_parser = {
	"projection",
	init_parse_projection,
	run_parse_projection,
	finish_parse_projection,
	cleanup_projection
};

/**
 * ------------------------------------------------------------------------
 * Initialize object bases
 * ------------------------------------------------------------------------ */

struct kb_parsedata {
	struct object_base defaults;
	struct object_base *kb;
};

static enum parser_error parse_object_base_defaults(struct parser *p) {
	const char *label;
	int value;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);
	label = parser_getsym(p, "label");
	value = parser_getint(p, "value");
	if (streq(label, "break-chance")) {
		d->defaults.break_perc = value;
	} else if (streq(label, "max-stack")) {
		d->defaults.max_stack = value;
	} else {
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_name(struct parser *p) {
	struct object_base *kb;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);
	kb = mem_alloc(sizeof *kb);
	memcpy(kb, &d->defaults, sizeof(*kb));
	kb->next = d->kb;
	d->kb = kb;

	kb->tval = tval_find_idx(parser_getsym(p, "tval"));
	if (kb->tval == -1) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	if (parser_hasval(p, "name")) {
		kb->name = string_make(parser_getstr(p, "name"));
	}
	kb->num_svals = 0;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_graphics(struct parser *p) {
	struct object_base *kb;
	const char *color;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);
	kb = d->kb;
	if (!kb) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	color = parser_getsym(p, "color");
	if (strlen(color) > 1) {
		kb->attr = color_text_to_attr(color);
	} else {
		kb->attr = color_char_to_attr(color[0]);
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_break(struct parser *p) {
	struct object_base *kb;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);
	kb = d->kb;
	if (!kb) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	kb->break_perc = parser_getint(p, "breakage");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_max_stack(struct parser *p) {

	struct kb_parsedata *d = parser_priv(p);
	struct object_base *kb;

	assert(d);
	kb = d->kb;
	if (!kb) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	kb->max_stack = parser_getint(p, "size");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_base_flags(struct parser *p) {
	struct object_base *kb;
	char *s, *t;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);
	kb = d->kb;
	if (!kb) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(kb->flags, OF_SIZE, obj_flags, t)) {
			found = true;
		}
		if (!grab_flag(kb->kind_flags, KF_SIZE, kind_flags, t)) {
			found = true;
		}
		if (grab_element_flag(kb->el_info, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}
	string_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

struct parser *init_parse_object_base(void) {
	struct parser *p = parser_new();

	struct kb_parsedata *d = mem_zalloc(sizeof(*d));
	parser_setpriv(p, d);

	parser_reg(p, "default sym label int value", parse_object_base_defaults);
	parser_reg(p, "name sym tval ?str name", parse_object_base_name);
	parser_reg(p, "graphics sym color", parse_object_base_graphics);
	parser_reg(p, "break int breakage", parse_object_base_break);
	parser_reg(p, "max-stack int size", parse_object_base_max_stack);
	parser_reg(p, "flags str flags", parse_object_base_flags);
	return p;
}

static errr run_parse_object_base(struct parser *p) {
	return parse_file_quit_not_found(p, "object_base");
}

static errr finish_parse_object_base(struct parser *p) {
	struct object_base *kb;
	struct object_base *next = NULL;
	struct kb_parsedata *d = parser_priv(p);

	assert(d);

	kb_info = mem_zalloc(TV_MAX * sizeof(*kb_info));

	for (kb = d->kb; kb; kb = next) {
		if (kb->tval < TV_MAX && kb->tval >= 0) {
			memcpy(&kb_info[kb->tval], kb, sizeof(*kb));
		} else {
			string_free(kb->name);
		}
		next = kb->next;
		mem_free(kb);
	}

	mem_free(d);
	parser_destroy(p);
	return 0;
}

static void cleanup_object_base(void)
{
	int idx;
	for (idx = 0; idx < TV_MAX; idx++) {
		string_free(kb_info[idx].name);
	}
	mem_free(kb_info);
}

struct file_parser object_base_parser = {
	"object_base",
	init_parse_object_base,
	run_parse_object_base,
	finish_parse_object_base,
	cleanup_object_base
};



/**
 * ------------------------------------------------------------------------
 * Initialize object slays
 * ------------------------------------------------------------------------ */

static enum parser_error parse_slay_code(struct parser *p) {
	const char *code = parser_getstr(p, "code");
	struct slay *h = parser_priv(p);
	struct slay *slay = mem_zalloc(sizeof *slay);

	slay->next = h;
	parser_setpriv(p, slay);
	slay->code = string_make(code);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(slay->name);
	slay->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_race_flag(struct parser *p) {
	int flag;
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	flag = lookup_flag(mon_race_flags, parser_getsym(p, "flag"));
	if (flag == FLAG_END) {
		return PARSE_ERROR_INVALID_FLAG;
	}
	/* Flag or base, not both nor multiple race flags */
	if (slay->race_flag || slay->base) {
		return PARSE_ERROR_INVALID_SLAY;
	}
	slay->race_flag = flag;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_base(struct parser *p) {
	const char *base_name = parser_getsym(p, "base");
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (lookup_monster_base(base_name) == NULL) {
		return PARSE_ERROR_INVALID_MONSTER_BASE;
	}
	/* Flag or base, not both nor multiple bases */
	if (slay->race_flag || slay->base) {
		return PARSE_ERROR_INVALID_SLAY;
	}
	slay->base = string_make(base_name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_multiplier(struct parser *p) {
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	slay->multiplier = parser_getuint(p, "multiplier");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_o_multiplier(struct parser *p) {
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	slay->o_multiplier = parser_getuint(p, "multiplier");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_power(struct parser *p) {
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	slay->power = parser_getuint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_melee_verb(struct parser *p) {
	const char *verb = parser_getstr(p, "verb");
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(slay->melee_verb);
	slay->melee_verb = string_make(verb);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_slay_range_verb(struct parser *p) {
	const char *verb = parser_getstr(p, "verb");
	struct slay *slay = parser_priv(p);

	if (!slay) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(slay->range_verb);
	slay->range_verb = string_make(verb);
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_slay(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "code str code", parse_slay_code);
	parser_reg(p, "name str name", parse_slay_name);
	parser_reg(p, "race-flag sym flag", parse_slay_race_flag);
	parser_reg(p, "base sym base", parse_slay_base);
	parser_reg(p, "multiplier uint multiplier", parse_slay_multiplier);
	parser_reg(p, "o-multiplier uint multiplier", parse_slay_o_multiplier);
	parser_reg(p, "power uint power", parse_slay_power);
	parser_reg(p, "melee-verb str verb", parse_slay_melee_verb);
	parser_reg(p, "range-verb str verb", parse_slay_range_verb);
	return p;
}

static errr run_parse_slay(struct parser *p) {
	return parse_file_quit_not_found(p, "slay");
}

static errr finish_parse_slay(struct parser *p) {
	struct slay *slay, *next = NULL;
	int count = 1;
	errr result = PARSE_ERROR_NONE;

	/* Count the entries */
	z_info->slay_max = 0;
	slay = parser_priv(p);
	while (slay) {
		if (z_info->slay_max >= 254) {
			result = PARSE_ERROR_TOO_MANY_ENTRIES;
			break;
		}
		z_info->slay_max++;
		slay = slay->next;
	}

	/* Allocate the direct access list and copy the data to it */
	slays = mem_zalloc((z_info->slay_max + 1) * sizeof(*slay));
	for (slay = parser_priv(p); slay; slay = next, count++) {
		next = slay->next;
		if (count <= z_info->slay_max) {
			memcpy(&slays[count], slay, sizeof(*slay));
			slays[count].next = NULL;
		}
		mem_free(slay);
	}
	z_info->slay_max += 1;

	parser_destroy(p);
	return result;
}

static void cleanup_slay(void)
{
	int idx;
	for (idx = 0; idx < z_info->slay_max; idx++) {
		string_free(slays[idx].code);
		string_free(slays[idx].name);
		string_free(slays[idx].base);
		string_free(slays[idx].melee_verb);
		string_free(slays[idx].range_verb);
	}
	mem_free(slays);
}

struct file_parser slay_parser = {
	"slay",
	init_parse_slay,
	run_parse_slay,
	finish_parse_slay,
	cleanup_slay
};

/**
 * ------------------------------------------------------------------------
 * Initialize object brands
 * ------------------------------------------------------------------------ */

static enum parser_error parse_brand_code(struct parser *p) {
	const char *code = parser_getstr(p, "code");
	struct brand *h = parser_priv(p);
	struct brand *brand = mem_zalloc(sizeof *brand);

	brand->next = h;
	parser_setpriv(p, brand);
	brand->code = string_make(code);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(brand->name);
	brand->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_verb(struct parser *p) {
	const char *verb = parser_getstr(p, "verb");
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(brand->verb);
	brand->verb = string_make(verb);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_multiplier(struct parser *p) {
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	brand->multiplier = parser_getuint(p, "multiplier");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_o_multiplier(struct parser *p) {
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	brand->o_multiplier = parser_getuint(p, "multiplier");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_power(struct parser *p) {
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	brand->power = parser_getuint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_resist_flag(struct parser *p) {
	int flag;
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	flag = lookup_flag(mon_race_flags, parser_getsym(p, "flag"));
	if (flag == FLAG_END) {
		return PARSE_ERROR_INVALID_FLAG;
	}
	brand->resist_flag = flag;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_brand_vuln_flag(struct parser *p) {
	int flag;
	struct brand *brand = parser_priv(p);

	if (!brand) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	flag = lookup_flag(mon_race_flags, parser_getsym(p, "flag"));
	if (flag == FLAG_END) {
		return PARSE_ERROR_INVALID_FLAG;
	}
	brand->vuln_flag = flag;
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_brand(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "code str code", parse_brand_code);
	parser_reg(p, "name str name", parse_brand_name);
	parser_reg(p, "verb str verb", parse_brand_verb);
	parser_reg(p, "multiplier uint multiplier", parse_brand_multiplier);
	parser_reg(p, "o-multiplier uint multiplier", parse_brand_o_multiplier);
	parser_reg(p, "power uint power", parse_brand_power);
	parser_reg(p, "resist-flag sym flag", parse_brand_resist_flag);
	parser_reg(p, "vuln-flag sym flag", parse_brand_vuln_flag);
	return p;
}

static errr run_parse_brand(struct parser *p) {
	return parse_file_quit_not_found(p, "brand");
}

static errr finish_parse_brand(struct parser *p) {
	struct brand *brand, *next = NULL;
	int count = 1;
	errr result = PARSE_ERROR_NONE;

	/* Count the entries */
	z_info->brand_max = 0;
	brand = parser_priv(p);
	while (brand) {
		if (z_info->brand_max >= 254) {
			result = PARSE_ERROR_TOO_MANY_ENTRIES;
			break;
		}
		z_info->brand_max++;
		brand = brand->next;
	}

	/* Allocate the direct access list and copy the data to it */
	brands = mem_zalloc((z_info->brand_max + 1) * sizeof(*brand));
	for (brand = parser_priv(p); brand; brand = next, count++) {
		next = brand->next;
		if (count <= z_info->brand_max) {
			memcpy(&brands[count], brand, sizeof(*brand));
			brands[count].next = NULL;
		}
		mem_free(brand);
	}
	z_info->brand_max += 1;

	parser_destroy(p);
	return result;
}

static void cleanup_brand(void)
{
	int idx;
	for (idx = 0; idx < z_info->brand_max; idx++) {
		string_free(brands[idx].code);
		string_free(brands[idx].name);
		string_free(brands[idx].verb);
	}
	mem_free(brands);
}

struct file_parser brand_parser = {
	"brand",
	init_parse_brand,
	run_parse_brand,
	finish_parse_brand,
	cleanup_brand
};



/**
 * ------------------------------------------------------------------------
 * Initialize object curses
 * ------------------------------------------------------------------------ */

static enum parser_error parse_curse_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct curse *h = parser_priv(p);

	struct curse *curse = mem_zalloc(sizeof *curse);
	curse->obj = mem_zalloc(sizeof(struct object));
	curse->next = h;
	parser_setpriv(p, curse);
	curse->name = string_make(name);
	curse->poss = mem_zalloc(TV_MAX * sizeof(bool));

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_type(struct parser *p) {
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	struct curse *curse = parser_priv(p);

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if ((tval < 0) || (tval >= TV_MAX)) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	curse->poss[tval] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_combat(struct parser *p) {
	struct curse *curse = parser_priv(p);

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	curse->obj->to_h = parser_getint(p, "to-h");
	curse->obj->to_d = parser_getint(p, "to-d");
	curse->obj->to_a = parser_getint(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_flags(struct parser *p) {
	struct curse *curse = parser_priv(p);
	char *s, *t;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(curse->obj->flags, OF_SIZE, obj_flags, t)) {
			found = true;
		}
		if (grab_element_flag(curse->obj->el_info, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}
	string_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_values(struct parser *p) {
	struct curse *curse = parser_priv(p);
	char *s, *t;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		int value = 0;
		int index = 0;
		bool found = false;
		if (!grab_index_and_int(&value, &index, obj_mods, "", t)) {
			found = true;
			curse->obj->modifiers[index] = value;
		}
		if (!grab_index_and_int(&value, &index, element_names, "RES_", t)) {
			found = true;
			curse->obj->el_info[index].res_level = value;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}

	string_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_effect(struct parser *p) {
	struct curse *curse = parser_priv(p);
	struct effect *effect, *new_effect;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Go to the next vacant effect and set it to the new one  */
	new_effect = mem_zalloc(sizeof(*new_effect));
	if (curse->obj->effect) {
		effect = curse->obj->effect;
		while (effect->next) effect = effect->next;
		effect->next = new_effect;
	} else {
		curse->obj->effect = new_effect;
	}
	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_curse_effect_yx(struct parser *p) {
	struct curse *curse = parser_priv(p);
	struct effect *effect;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = curse->obj->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_dice(struct parser *p) {
	struct curse *curse = parser_priv(p);
	struct effect *effect;
	dice_t *dice;
	const char *string;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = curse->obj->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	/* Go to the correct effect */
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

static enum parser_error parse_curse_expr(struct parser *p) {
	struct curse *curse = parser_priv(p);
	expression_t *expression;
	expression_base_value_f function;
	struct effect *effect;
	const char *name;
	const char *base;
	const char *expr;
	enum parser_error result;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = curse->obj->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	/* Go to the correct effect */
	while (effect->next) effect = effect->next;
	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL) {
		return PARSE_ERROR_NONE;
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
	} else if (dice_bind_expression(effect->dice, name, expression) < 0) {
		result = PARSE_ERROR_UNBOUND_EXPRESSION;
	} else {
		result = PARSE_ERROR_NONE;
	}
	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);
	return result;
}

static enum parser_error parse_curse_msg(struct parser *p) {
	struct curse *curse = parser_priv(p);

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	curse->obj->effect_msg = string_append(curse->obj->effect_msg,
		parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_time(struct parser *p) {
	struct curse *curse = parser_priv(p);

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	curse->obj->time = parser_getrand(p, "time");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_desc(struct parser *p) {
	struct curse *curse = parser_priv(p);

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	curse->desc = string_append(curse->desc, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_conflict(struct parser *p) {
	struct curse *curse = parser_priv(p);

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	curse->conflict = string_append(curse->conflict, "|");
	curse->conflict = string_append(curse->conflict, parser_getstr(p, "conf"));
	curse->conflict = string_append(curse->conflict, "|");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_curse_conflict_flags(struct parser *p) {
	struct curse *curse = parser_priv(p);
	char *s, *t;

	if (!curse) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(curse->conflict_flags, OF_SIZE, obj_flags, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}
	string_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static struct parser *init_parse_curse(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_curse_name);
	parser_reg(p, "type sym tval", parse_curse_type);
	parser_reg(p, "combat int to-h int to-d int to-a", parse_curse_combat);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_curse_effect);
	parser_reg(p, "effect-yx int y int x", parse_curse_effect_yx);
	parser_reg(p, "dice str dice", parse_curse_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_curse_expr);
	parser_reg(p, "msg str text", parse_curse_msg);
	parser_reg(p, "time rand time", parse_curse_time);
	parser_reg(p, "flags str flags", parse_curse_flags);
	parser_reg(p, "values str values", parse_curse_values);
	parser_reg(p, "desc str desc", parse_curse_desc);
	parser_reg(p, "conflict str conf", parse_curse_conflict);
	parser_reg(p, "conflict-flags str flags", parse_curse_conflict_flags);
	return p;
}

static errr run_parse_curse(struct parser *p) {
	return parse_file_quit_not_found(p, "curse");
}

static errr finish_parse_curse(struct parser *p) {
	struct curse *curse, *next = NULL;
	int count = 1;
	errr result = PARSE_ERROR_NONE;

	/* Count the entries */
	z_info->curse_max = 0;
	curse = parser_priv(p);
	while (curse) {
		if (z_info->curse_max >= 254) {
			result = PARSE_ERROR_TOO_MANY_ENTRIES;
			break;
		}
		z_info->curse_max++;
		curse = curse->next;
	}

	/* Allocate the direct access list and copy the data to it */
	curses = mem_zalloc((z_info->curse_max + 1) * sizeof(*curse));
	for (curse = parser_priv(p); curse; curse = next, count++) {
		next = curse->next;
		if (count <= z_info->curse_max) {
			memcpy(&curses[count], curse, sizeof(*curse));
			curses[count].next = NULL;
		}
		mem_free(curse);
	}
	z_info->curse_max += 1;

	parser_destroy(p);
	return result;
}

static void cleanup_curse(void)
{
	int idx;
	for (idx = 0; idx < z_info->curse_max; idx++) {
		string_free(curses[idx].name);
		string_free(curses[idx].conflict);
		mem_free(curses[idx].desc);
		if (curses[idx].obj) {
			free_effect(curses[idx].obj->effect);
			mem_free(curses[idx].obj->effect_msg);
			if (curses[idx].obj->known) {
				object_free(curses[idx].obj->known);
			}
			mem_free(curses[idx].obj);
		}
		mem_free(curses[idx].poss);
	}
	mem_free(curses);
}

struct file_parser curse_parser = {
	"curse",
	init_parse_curse,
	run_parse_curse,
	finish_parse_curse,
	cleanup_curse
};

/**
 * ------------------------------------------------------------------------
 * Initialize activations
 * ------------------------------------------------------------------------ */

static enum parser_error parse_act_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct activation *h = parser_priv(p);

	struct activation *act = mem_zalloc(sizeof *act);
	act->next = h;
	parser_setpriv(p, act);
	act->name = string_make(name);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_aim(struct parser *p) {
	struct activation *act = parser_priv(p);
	int val;

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	val = parser_getuint(p, "aim");
	act->aim = val ? true : false;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_power(struct parser *p) {
	struct activation *act = parser_priv(p);

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	act->power = parser_getuint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_effect(struct parser *p) {
	struct activation *act = parser_priv(p);
	struct effect *effect, *new_effect;

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Go to the next vacant effect and set it to the new one  */
	new_effect = mem_zalloc(sizeof(*new_effect));
	if (act->effect) {
		effect = act->effect;
		while (effect->next) effect = effect->next;
		effect->next = new_effect;
	} else {
		act->effect = new_effect;
	}
	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_act_effect_yx(struct parser *p) {
	struct activation *act = parser_priv(p);
	struct effect *effect;

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = act->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_act_dice(struct parser *p) {
	struct activation *act = parser_priv(p);
	struct effect *effect;
	dice_t *dice;
	const char *string;

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = act->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	/* Go to the correct effect */
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

static enum parser_error parse_act_expr(struct parser *p) {
	struct activation *act = parser_priv(p);
	expression_t *expression;
	expression_base_value_f function;
	struct effect *effect;
	const char *name;
	const char *base;
	const char *expr;
	enum parser_error result;

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = act->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	/* Go to the correct effect */
	while (effect->next) effect = effect->next;
	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL) {
		return PARSE_ERROR_NONE;
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
	} else if (dice_bind_expression(effect->dice, name, expression) < 0) {
		result = PARSE_ERROR_UNBOUND_EXPRESSION;
	} else {
		result = PARSE_ERROR_NONE;
	}
	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);
	return result;
}

static enum parser_error parse_act_msg(struct parser *p) {
	struct activation *act = parser_priv(p);

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	act->message = string_append(act->message, parser_getstr(p, "msg"));
	return PARSE_ERROR_NONE;
}


static enum parser_error parse_act_desc(struct parser *p) {
	struct activation *act = parser_priv(p);

	if (!act) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	act->desc = string_append(act->desc, parser_getstr(p, "desc"));
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_act(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_act_name);
	parser_reg(p, "aim uint aim", parse_act_aim);
	parser_reg(p, "power uint power", parse_act_power);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_act_effect);
	parser_reg(p, "effect-yx int y int x", parse_act_effect_yx);
	parser_reg(p, "dice str dice", parse_act_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_act_expr);
	parser_reg(p, "msg str msg", parse_act_msg);
	parser_reg(p, "desc str desc", parse_act_desc);
	return p;
}

static errr run_parse_act(struct parser *p) {
	return parse_file_quit_not_found(p, "activation");
}

static errr finish_parse_act(struct parser *p) {
	struct activation *act, *next = NULL;
	int count = 1;

	/* Count the entries */
	z_info->act_max = 0;
	act = parser_priv(p);
	while (act) {
		z_info->act_max++;
		act = act->next;
	}

	/* Allocate the direct access list and copy the data to it */
	activations = mem_zalloc((z_info->act_max + 1) * sizeof(*act));
	for (act = parser_priv(p); act; act = next, count++) {
		memcpy(&activations[count], act, sizeof(*act));
		activations[count].index = count;
		next = act->next;
		if (next) {
			activations[count].next = &activations[count + 1];
		} else {
			activations[count].next = NULL;
		}
		mem_free(act);
	}
	z_info->act_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_act(void)
{
	int idx;
	for (idx = 0; idx < z_info->act_max; idx++) {
		string_free(activations[idx].name);
		mem_free(activations[idx].desc);
		mem_free(activations[idx].message);
		free_effect(activations[idx].effect);
	}
	mem_free(activations);
}

struct file_parser act_parser = {
	"activation",
	init_parse_act,
	run_parse_act,
	finish_parse_act,
	cleanup_act
};

/**
 * ------------------------------------------------------------------------
 * Initialize objects
 * ------------------------------------------------------------------------ */

/* Generic object kinds */
struct object_kind *unknown_item_kind;
struct object_kind *unknown_gold_kind;
struct object_kind *pile_kind;
struct object_kind *curse_object_kind;

static enum parser_error parse_object_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct object_kind *h = parser_priv(p);

	struct object_kind *k = mem_zalloc(sizeof *k);
	k->next = h;
	parser_setpriv(p, k);
	k->name = string_make(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_graphics(struct parser *p) {
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->d_char = glyph;
	if (strlen(color) > 1) {
		k->d_attr = color_text_to_attr(color);
	} else {
		k->d_attr = color_char_to_attr(color[0]);
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_type(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	int tval;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	k->tval = tval;
	k->base = &kb_info[k->tval];
	k->base->num_svals++;
	k->sval = k->base->num_svals;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_level(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->level = parser_getint(p, "level");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_weight(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->weight = parser_getint(p, "weight");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_cost(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->cost = parser_getint(p, "cost");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_alloc(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->alloc_prob = parser_getint(p, "common");
	if (grab_int_range(&amin, &amax, tmp, "to")) {
		return PARSE_ERROR_INVALID_ALLOCATION;
	}
	k->alloc_min = amin;
	k->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_attack(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->dd = hd.dice;
	k->ds = hd.sides;
	k->to_h = parser_getrand(p, "to-h");
	k->to_d = parser_getrand(p, "to-d");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_armor(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->ac = parser_getint(p, "ac");
	k->to_a = parser_getrand(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_charges(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->charge = parser_getrand(p, "charges");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_pile(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->gen_mult_prob = parser_getint(p, "prob");
	k->stack_size = parser_getrand(p, "stack");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_flags(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	char *s, *t;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(k->flags, OF_SIZE, obj_flags, t)) {
			found = true;
		}
		if (!grab_flag(k->kind_flags, KF_SIZE, kind_flags, t)) {
			found = true;
		}
		if (grab_element_flag(k->el_info, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}
	string_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_object_power(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->power = parser_getint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_effect(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct effect *effect, *new_effect;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Go to the next vacant effect and set it to the new one  */
	new_effect = mem_zalloc(sizeof(*new_effect));
	if (k->effect) {
		effect = k->effect;
		while (effect->next) effect = effect->next;
		effect->next = new_effect;
	} else {
		k->effect = new_effect;
	}
	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_object_effect_yx(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct effect *effect;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = k->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;
	effect->y = parser_getint(p, "y");
	effect->x = parser_getint(p, "x");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_dice(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	dice_t *dice;
	struct effect *effect;
	const char *string;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = k->effect;
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

static enum parser_error parse_object_expr(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	struct effect *effect;
	expression_t *expression;
	expression_base_value_f function;
	const char *name;
	const char *base;
	const char *expr;
	enum parser_error result;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* If there is no effect, assume that this is human and not parser error. */
	effect = k->effect;
	if (effect == NULL) {
		return PARSE_ERROR_NONE;
	}
	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL) {
		return PARSE_ERROR_NONE;
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
	} else if (dice_bind_expression(effect->dice, name, expression) < 0) {
		result = PARSE_ERROR_UNBOUND_EXPRESSION;
	} else {
		result = PARSE_ERROR_NONE;
	}
	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);
	return result;
}

static enum parser_error parse_object_msg(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->effect_msg = string_append(k->effect_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_vis_msg(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->vis_msg = string_append(k->vis_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_time(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->time = parser_getrand(p, "time");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_desc(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->text = string_append(k->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_pval(struct parser *p) {
	struct object_kind *k = parser_priv(p);

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k->pval = parser_getrand(p, "pval");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_values(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	char *s, *t;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		int value = 0;
		int index = 0;
		bool found = false;
		if (!grab_rand_value(k->modifiers, obj_mods, t)) {
			found = true;
		}
		if (!grab_index_and_int(&value, &index, element_names, "RES_", t)) {
			found = true;
			k->el_info[index].res_level = value;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}

	string_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_object_slay(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	const char *s = parser_getstr(p, "code");
	int i;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->slay_max; i++) {
		if (streq(s, slays[i].code)) break;
	}
	if (i == z_info->slay_max) {
		return PARSE_ERROR_UNRECOGNISED_SLAY;
	}
	if (!k->slays) {
		k->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	}
	k->slays[i] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_brand(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	const char *s = parser_getstr(p, "code");
	int i;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->brand_max; i++) {
		if (streq(s, brands[i].code)) break;
	}
	if (i == z_info->brand_max) {
		return PARSE_ERROR_UNRECOGNISED_BRAND;
	}
	if (!k->brands) {
		k->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	}
	k->brands[i] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_curse(struct parser *p) {
	struct object_kind *k = parser_priv(p);
	const char *s = parser_getsym(p, "name");
	int power = parser_getint(p, "power");
	int i;

	if (!k) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->curse_max; i++) {
		if (streq(s, curses[i].name)) break;
	}
	if (i == z_info->curse_max) {
		return PARSE_ERROR_UNRECOGNISED_CURSE;
	}
	/* Only add if it has power. */
	if (power > 0) {
		if (!k->curses) {
			k->curses = mem_zalloc(z_info->curse_max * sizeof(int));
		}
		k->curses[i] = power;
	}
	return PARSE_ERROR_NONE;
}


struct parser *init_parse_object(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_object_name);
	parser_reg(p, "type sym tval", parse_object_type);
	parser_reg(p, "graphics char glyph sym color", parse_object_graphics);
	parser_reg(p, "level int level", parse_object_level);
	parser_reg(p, "weight int weight", parse_object_weight);
	parser_reg(p, "cost int cost", parse_object_cost);
	parser_reg(p, "alloc int common str minmax", parse_object_alloc);
	parser_reg(p, "attack rand hd rand to-h rand to-d", parse_object_attack);
	parser_reg(p, "armor int ac rand to-a", parse_object_armor);
	parser_reg(p, "charges rand charges", parse_object_charges);
	parser_reg(p, "pile int prob rand stack", parse_object_pile);
	parser_reg(p, "flags str flags", parse_object_flags);
	parser_reg(p, "power int power", parse_object_power);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_object_effect);
	parser_reg(p, "effect-yx int y int x", parse_object_effect_yx);
	parser_reg(p, "dice str dice", parse_object_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_object_expr);
	parser_reg(p, "msg str text", parse_object_msg);
	parser_reg(p, "vis-msg str text", parse_object_vis_msg);
	parser_reg(p, "time rand time", parse_object_time);
	parser_reg(p, "pval rand pval", parse_object_pval);
	parser_reg(p, "values str values", parse_object_values);
	parser_reg(p, "desc str text", parse_object_desc);
	parser_reg(p, "slay str code", parse_object_slay);
	parser_reg(p, "brand str code", parse_object_brand);
	parser_reg(p, "curse sym name int power", parse_object_curse);
	return p;
}

static errr run_parse_object(struct parser *p) {
	return parse_file_quit_not_found(p, "object");
}

static errr finish_parse_object(struct parser *p) {
	struct object_kind *k, *next = NULL;
	int kidx;

	/* scan the list for the max id */
	z_info->k_max = 0;
	k = parser_priv(p);
	while (k) {
		z_info->k_max++;
		k = k->next;
	}

	/* allocate the direct access list and copy the data to it */
	k_info = mem_zalloc((z_info->k_max + 1) * sizeof(*k));
	kidx = z_info->k_max - 1;
	for (k = parser_priv(p); k; k = next, kidx--) {
		assert(kidx >= 0);

		memcpy(&k_info[kidx], k, sizeof(*k));
		k_info[kidx].kidx = kidx;

		/* Add base kind flags to kind kind flags */
		kf_union(k_info[kidx].kind_flags, kb_info[k->tval].kind_flags);

		next = k->next;
		if (kidx < z_info->k_max - 1) {
			k_info[kidx].next = &k_info[kidx + 1];
		} else {
			k_info[kidx].next = NULL;
		}
		mem_free(k);
	}
	z_info->k_max += 1;
	z_info->ordinary_kind_max = z_info->k_max;

	parser_destroy(p);
	return 0;
}

static void cleanup_object(void)
{
	int idx;
	for (idx = 0; idx < z_info->k_max; idx++) {
		struct object_kind *kind = &k_info[idx];
		string_free(kind->name);
		string_free(kind->text);
		string_free(kind->effect_msg);
		string_free(kind->vis_msg);
		mem_free(kind->brands);
		mem_free(kind->slays);
		mem_free(kind->curses);
		free_effect(kind->effect);
	}
	mem_free(k_info);
}

struct file_parser object_parser = {
	"object",
	init_parse_object,
	run_parse_object,
	finish_parse_object,
	cleanup_object
};

/**
 * ------------------------------------------------------------------------
 * Initialize ego items
 * ------------------------------------------------------------------------ */

static enum parser_error parse_ego_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct ego_item *h = parser_priv(p);

	struct ego_item *e = mem_zalloc(sizeof *e);
	e->next = h;
	parser_setpriv(p, e);
	e->name = string_make(name);

	/* Set all min-combat values to no minimum */
	e->min_to_h = NO_MINIMUM;
	e->min_to_d = NO_MINIMUM;
	e->min_to_a = NO_MINIMUM;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_info(struct parser *p) {
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->cost = parser_getint(p, "cost");
	e->rating = parser_getint(p, "rating");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_alloc(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->alloc_prob = parser_getint(p, "common");
	if (grab_int_range(&amin, &amax, tmp, "to")) {
		return PARSE_ERROR_INVALID_ALLOCATION;
	}
	if (amin > 255 || amax > 255 || amin < 0 || amax < 0) {
		return PARSE_ERROR_OUT_OF_BOUNDS;
	}
	e->alloc_min = amin;
	e->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_type(struct parser *p) {
	struct poss_item *poss;
	int i;
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	bool found_one_kind = false;
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (tval < 0) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	/* Find all the right object kinds */
	for (i = 0; i < z_info->k_max; i++) {
		if (k_info[i].tval != tval) continue;
		poss = mem_zalloc(sizeof(struct poss_item));
		poss->kidx = i;
		poss->next = e->poss_items;
		e->poss_items = poss;
		found_one_kind = true;
	}

	if (!found_one_kind) {
		return PARSE_ERROR_NO_KIND_FOR_EGO_TYPE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_item(struct parser *p) {
	struct poss_item *poss;
	int tval = tval_find_idx(parser_getsym(p, "tval"));
	int sval = lookup_sval(tval, parser_getsym(p, "sval"));
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (tval < 0) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	if (sval < 0) {
		return PARSE_ERROR_UNRECOGNISED_SVAL;
	}
	poss = mem_zalloc(sizeof(struct poss_item));
	poss->kidx = lookup_kind(tval, sval)->kidx;
	poss->next = e->poss_items;
	e->poss_items = poss;
	return (poss->kidx <= 0) ?
		PARSE_ERROR_INVALID_ITEM_NUMBER : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_combat(struct parser *p) {
	struct random th = parser_getrand(p, "th");
	struct random td = parser_getrand(p, "td");
	struct random ta = parser_getrand(p, "ta");
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->to_h = th;
	e->to_d = td;
	e->to_a = ta;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_min(struct parser *p) {
	int th = parser_getint(p, "th");
	int td = parser_getint(p, "td");
	int ta = parser_getint(p, "ta");
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->min_to_h = th;
	e->min_to_d = td;
	e->min_to_a = ta;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_act(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	const char *name = parser_getstr(p, "name");

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->activation = findact(name);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_time(struct parser *p) {
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->time = parser_getrand(p, "time");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_flags(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *flags;
	char *t;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "flags")) {
		return PARSE_ERROR_NONE;
	}
	flags = string_make(parser_getstr(p, "flags"));
	t = strtok(flags, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(e->flags, OF_SIZE, obj_flags, t)) {
			found = true;
		}
		if (!grab_flag(e->kind_flags, KF_SIZE, kind_flags, t)) {
			found = true;
		}
		if (grab_element_flag(e->el_info, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}
	string_free(flags);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_flags_off(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *flags;
	char *t;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "flags")) {
		return PARSE_ERROR_NONE;
	}
	flags = string_make(parser_getstr(p, "flags"));
	t = strtok(flags, " |");
	while (t) {
		if (grab_flag(e->flags_off, OF_SIZE, obj_flags, t)) {
			string_free(flags);
			return PARSE_ERROR_INVALID_FLAG;
		}
		t = strtok(NULL, " |");
	}
	string_free(flags);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_values(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *s, *t;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "values")) {
		return PARSE_ERROR_MISSING_FIELD;
	}
	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");

	while (t) {
		bool found = false;
		int value = 0;
		int index = 0;
		if (!grab_rand_value(e->modifiers, obj_mods, t)) {
			found = true;
		}
		if (!grab_index_and_int(&value, &index, element_names, "RES_", t)) {
			found = true;
			e->el_info[index].res_level = value;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}

	string_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_min_val(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	char *s, *t;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "min_values")) {
		return PARSE_ERROR_MISSING_FIELD;
	}

	s = string_make(parser_getstr(p, "min_values"));
	t = strtok(s, " |");

	while (t) {
		bool found = false;
		if (!grab_int_value(e->min_modifiers, obj_mods, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}

	string_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_desc(struct parser *p) {
	struct ego_item *e = parser_priv(p);

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	e->text = string_append(e->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_slay(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	const char *s = parser_getstr(p, "code");
	int i;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->slay_max; i++) {
		if (streq(s, slays[i].code)) break;
	}
	if (i == z_info->slay_max) {
		return PARSE_ERROR_UNRECOGNISED_SLAY;
	}
	if (!e->slays) {
		e->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	}
	e->slays[i] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_brand(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	const char *s = parser_getstr(p, "code");
	int i;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->brand_max; i++) {
		if (streq(s, brands[i].code)) break;
	}
	if (i == z_info->brand_max) {
		return PARSE_ERROR_UNRECOGNISED_BRAND;
	}
	if (!e->brands) {
		e->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	}
	e->brands[i] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_ego_curse(struct parser *p) {
	struct ego_item *e = parser_priv(p);
	int power = parser_getint(p, "power");
	const char *s = parser_getsym(p, "name");
	int i;

	if (!e) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->curse_max; i++) {
		if (streq(s, curses[i].name)) break;
	}
	if (i == z_info->curse_max) {
		return PARSE_ERROR_UNRECOGNISED_CURSE;
	}
	/* Only add if it has power. */
	if (power > 0) {
		if (!e->curses) {
			e->curses = mem_zalloc(z_info->curse_max * sizeof(int));
		}
		e->curses[i] = power;
	}
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_ego(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_ego_name);
	parser_reg(p, "info int cost int rating", parse_ego_info);
	parser_reg(p, "alloc int common str minmax", parse_ego_alloc);
	parser_reg(p, "type sym tval", parse_ego_type);
	parser_reg(p, "item sym tval sym sval", parse_ego_item);
	parser_reg(p, "combat rand th rand td rand ta", parse_ego_combat);
	parser_reg(p, "min-combat int th int td int ta", parse_ego_min);
	parser_reg(p, "act str name", parse_ego_act);
	parser_reg(p, "time rand time", parse_ego_time);
	parser_reg(p, "flags ?str flags", parse_ego_flags);
	parser_reg(p, "flags-off ?str flags", parse_ego_flags_off);
	parser_reg(p, "values str values", parse_ego_values);
	parser_reg(p, "min-values str min_values", parse_ego_min_val);
	parser_reg(p, "desc str text", parse_ego_desc);
	parser_reg(p, "slay str code", parse_ego_slay);
	parser_reg(p, "brand str code", parse_ego_brand);
	parser_reg(p, "curse sym name int power", parse_ego_curse);
	return p;
}

static errr run_parse_ego(struct parser *p) {
	return parse_file_quit_not_found(p, "ego_item");
}

static errr finish_parse_ego(struct parser *p) {
	struct ego_item *e, *n;
	int eidx;

	/* Scan the list for the max id */
	z_info->e_max = 0;
	e = parser_priv(p);
	while (e) {
		z_info->e_max++;
		e = e->next;
	}

	/* Allocate the direct access list and copy the data to it */
	e_info = mem_zalloc((z_info->e_max + 1) * sizeof(*e));
	eidx = z_info->e_max - 1;
	for (e = parser_priv(p); e; e = n, eidx--) {
		assert(eidx >= 0);

		memcpy(&e_info[eidx], e, sizeof(*e));
		e_info[eidx].eidx = eidx;
		n = e->next;
		if (eidx < z_info->e_max - 1) {
			e_info[eidx].next = &e_info[eidx + 1];
		} else {
			e_info[eidx].next = NULL;
		}
		mem_free(e);
	}
	z_info->e_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_ego(void)
{
	int idx;
	for (idx = 0; idx < z_info->e_max; idx++) {
		struct ego_item *ego = &e_info[idx];
		struct poss_item *poss;

		string_free(ego->name);
		string_free(ego->text);
		mem_free(ego->brands);
		mem_free(ego->slays);
		mem_free(ego->curses);

		poss = ego->poss_items;
		while (poss) {
			struct poss_item *next = poss->next;
			mem_free(poss);
			poss = next;
		}
	}
	mem_free(e_info);
}

struct file_parser ego_parser = {
	"ego_item",
	init_parse_ego,
	run_parse_ego,
	finish_parse_ego,
	cleanup_ego
};

/**
 * ------------------------------------------------------------------------
 * Initialize artifacts
 * ------------------------------------------------------------------------ */

static enum parser_error parse_artifact_name(struct parser *p) {
	size_t i;
	const char *name = parser_getstr(p, "name");
	struct artifact *h = parser_priv(p);

	struct artifact *a = mem_zalloc(sizeof *a);
	a->next = h;
	parser_setpriv(p, a);
	a->name = string_make(name);

	/* Ignore all base elements */
	for (i = ELEM_BASE_MIN; i < ELEM_HIGH_MIN; i++) {
		a->el_info[i].flags |= EL_INFO_IGNORE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_base_object(struct parser *p) {
	struct artifact *a = parser_priv(p);
	int tval, sval;
	const char *sval_name;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	tval = tval_find_idx(parser_getsym(p, "tval"));
	if (tval < 0) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	a->tval = tval;

	sval_name = parser_getsym(p, "sval");
	sval = lookup_sval(a->tval, sval_name);
	if (sval < 0) {
		return write_dummy_object_record(a, sval_name);
	}
	a->sval = sval;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_graphics(struct parser *p) {
	wchar_t glyph = parser_getchar(p, "glyph");
	const char *color = parser_getsym(p, "color");
	struct artifact *a = parser_priv(p);
	struct object_kind *k;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k = lookup_kind(a->tval, a->sval);
	assert(k);
	if (!kf_has(k->kind_flags, KF_INSTA_ART)) {
		return PARSE_ERROR_NOT_SPECIAL_ARTIFACT;
	}
	k->d_char = glyph;
	if (strlen(color) > 1) {
		k->d_attr = color_text_to_attr(color);
	} else {
		k->d_attr = color_char_to_attr(color[0]);
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_level(struct parser *p) {
	struct artifact *a = parser_priv(p);

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	a->level = parser_getint(p, "level");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_weight(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct object_kind *k;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k = lookup_kind(a->tval, a->sval);
	assert(k);
	a->weight = parser_getint(p, "weight");
	/* Set kind weight for special artifacts */
	if (k->kidx >= z_info->ordinary_kind_max) {
		k->weight = a->weight;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_cost(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct object_kind *k;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	k = lookup_kind(a->tval, a->sval);
	assert(k);
	a->cost = parser_getint(p, "cost");
	/* Set kind cost for special artifacts */
	if (k->kidx >= z_info->ordinary_kind_max) {
		k->cost = a->cost;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_alloc(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *tmp = parser_getstr(p, "minmax");
	int amin, amax;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	a->alloc_prob = parser_getint(p, "common");
	if (grab_int_range(&amin, &amax, tmp, "to")) {
		return PARSE_ERROR_INVALID_ALLOCATION;
	}
	if (amin > 255 || amax > 255 || amin < 0 || amax < 0) {
		return PARSE_ERROR_OUT_OF_BOUNDS;
	}
	a->alloc_min = amin;
	a->alloc_max = amax;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_attack(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct random hd = parser_getrand(p, "hd");

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	a->dd = hd.dice;
	a->ds = hd.sides;
	a->to_h = parser_getint(p, "to-h");
	a->to_d = parser_getint(p, "to-d");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_armor(struct parser *p) {
	struct artifact *a = parser_priv(p);

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	a->ac = parser_getint(p, "ac");
	a->to_a = parser_getint(p, "to-a");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_flags(struct parser *p) {
	struct artifact *a = parser_priv(p);
	char *s, *t;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!parser_hasval(p, "flags")) {
		return PARSE_ERROR_NONE;
	}
	s = string_make(parser_getstr(p, "flags"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		if (!grab_flag(a->flags, OF_SIZE, obj_flags, t)) {
			found = true;
		}
		if (grab_element_flag(a->el_info, t)) {
			found = true;
		}
		if (!found) {
			break;
		}
		t = strtok(NULL, " |");
	}
	string_free(s);
	return t ? PARSE_ERROR_INVALID_FLAG : PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_act(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct object_kind *k;
	const char *name = parser_getstr(p, "name");

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Special light activations are a property of the base object */
	k = lookup_kind(a->tval, a->sval);
	if ((a->tval == TV_LIGHT) && (k->kidx >= z_info->ordinary_kind_max)) {
		k->activation = findact(name);
	} else {
		a->activation = findact(name);
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_time(struct parser *p) {
	struct artifact *a = parser_priv(p);
	struct object_kind *k;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	/* Special light activations are a property of the base object */
	k = lookup_kind(a->tval, a->sval);
	if ((a->tval == TV_LIGHT) && (k->kidx >= z_info->ordinary_kind_max)) {
		k->time = parser_getrand(p, "time");
	} else {
		a->time = parser_getrand(p, "time");
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_msg(struct parser *p) {
	struct artifact *a = parser_priv(p);

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	a->alt_msg = string_append(a->alt_msg, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_values(struct parser *p) {
	struct artifact *a = parser_priv(p);
	char *s, *t;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}

	s = string_make(parser_getstr(p, "values"));
	t = strtok(s, " |");
	while (t) {
		bool found = false;
		int value = 0;
		int index = 0;
		if (!grab_int_value(a->modifiers, obj_mods, t)) {
			found = true;
		}
		if (!grab_index_and_int(&value, &index, element_names, "RES_", t)) {
			found = true;
			a->el_info[index].res_level = value;
		}
		if (!found) {
			break;
		}

		t = strtok(NULL, " |");
	}

	string_free(s);
	return t ? PARSE_ERROR_INVALID_VALUE : PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_desc(struct parser *p) {
	struct artifact *a = parser_priv(p);

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	a->text = string_append(a->text, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_slay(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *s = parser_getstr(p, "code");
	int i;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->slay_max; i++) {
		if (streq(s, slays[i].code)) break;
	}
	if (i == z_info->slay_max) {
		return PARSE_ERROR_UNRECOGNISED_SLAY;
	}
	if (!a->slays) {
		a->slays = mem_zalloc(z_info->slay_max * sizeof(bool));
	}
	a->slays[i] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_brand(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *s = parser_getstr(p, "code");
	int i;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->brand_max; i++) {
		if (streq(s, brands[i].code)) break;
	}
	if (i == z_info->brand_max) {
		return PARSE_ERROR_UNRECOGNISED_BRAND;
	}
	if (!a->brands) {
		a->brands = mem_zalloc(z_info->brand_max * sizeof(bool));
	}
	a->brands[i] = true;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_artifact_curse(struct parser *p) {
	struct artifact *a = parser_priv(p);
	const char *s = parser_getsym(p, "name");
	int power = parser_getint(p, "power");
	int i;

	if (!a) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	for (i = 1; i < z_info->curse_max; i++) {
		if (streq(s, curses[i].name)) break;
	}
	if (i == z_info->curse_max) {
		return PARSE_ERROR_UNRECOGNISED_CURSE;
	}
	/* Only add if it has power. */
	if (power > 0) {
		if (!a->curses) {
			a->curses = mem_zalloc(z_info->curse_max * sizeof(int));
		}
		a->curses[i] = power;
	}
	return PARSE_ERROR_NONE;
}

struct parser *init_parse_artifact(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_artifact_name);
	parser_reg(p, "base-object sym tval sym sval", parse_artifact_base_object);
	parser_reg(p, "graphics char glyph sym color", parse_artifact_graphics);
	parser_reg(p, "level int level", parse_artifact_level);
	parser_reg(p, "weight int weight", parse_artifact_weight);
	parser_reg(p, "cost int cost", parse_artifact_cost);
	parser_reg(p, "alloc int common str minmax", parse_artifact_alloc);
	parser_reg(p, "attack rand hd int to-h int to-d", parse_artifact_attack);
	parser_reg(p, "armor int ac int to-a", parse_artifact_armor);
	parser_reg(p, "flags ?str flags", parse_artifact_flags);
	parser_reg(p, "act str name", parse_artifact_act);
	parser_reg(p, "time rand time", parse_artifact_time);
	parser_reg(p, "msg str text", parse_artifact_msg);
	parser_reg(p, "values str values", parse_artifact_values);
	parser_reg(p, "desc str text", parse_artifact_desc);
	parser_reg(p, "slay str code", parse_artifact_slay);
	parser_reg(p, "brand str code", parse_artifact_brand);
	parser_reg(p, "curse sym name int power", parse_artifact_curse);
	return p;
}

static errr run_parse_artifact(struct parser *p) {
	return parse_file_quit_not_found(p, "artifact");
}

static errr finish_parse_artifact(struct parser *p) {
	struct artifact *a, *n;
	int none, aidx;

	/* Scan the list for the max id */
	z_info->a_max = 0;
	a = parser_priv(p);
	while (a) {
		z_info->a_max++;
		a = a->next;
	}

	/* Allocate the direct access list and copy the data to it */
	a_info = mem_zalloc((z_info->a_max + 1) * sizeof(*a));
	aup_info = mem_zalloc((z_info->a_max + 1) * sizeof(*aup_info));
	aidx = z_info->a_max;
	for (a = parser_priv(p); a; a = n, aidx--) {
		assert(aidx > 0);

		memcpy(&a_info[aidx], a, sizeof(*a));
		a_info[aidx].aidx = aidx;
		n = a->next;
		a_info[aidx].next = (aidx < z_info->a_max) ?
			&a_info[aidx + 1] : NULL;
		mem_free(a);

		aup_info[aidx].aidx = aidx;
	}
	z_info->a_max += 1;

	/* Now we're done with object kinds, deal with object-like things */
	none = tval_find_idx("none");
	unknown_item_kind = lookup_kind(none, lookup_sval(none, "<unknown item>"));
	unknown_gold_kind = lookup_kind(none,
									lookup_sval(none, "<unknown treasure>"));
	pile_kind = lookup_kind(none, lookup_sval(none, "<pile>"));
	curse_object_kind = lookup_kind(none, lookup_sval(none, "<curse object>"));
	write_curse_kinds();
	parser_destroy(p);
	return 0;
}

static void cleanup_artifact(void)
{
	int idx;
	for (idx = 0; idx < z_info->a_max; idx++) {
		struct artifact *art = &a_info[idx];
		string_free(art->name);
		string_free(art->alt_msg);
		string_free(art->text);
		mem_free(art->brands);
		mem_free(art->slays);
		mem_free(art->curses);
	}
	mem_free(a_info);
	mem_free(aup_info);
}

struct file_parser artifact_parser = {
	"artifact",
	init_parse_artifact,
	run_parse_artifact,
	finish_parse_artifact,
	cleanup_artifact
};

/**
 * ------------------------------------------------------------------------
 * Initialize random artifacts
 * This mostly uses the artifact functions
 * ------------------------------------------------------------------------ */
static errr run_parse_randart(struct parser *p) {
	return parse_file_quit_not_found(p, "randart");
}

static errr finish_parse_randart(struct parser *p) {
	struct artifact *a, *n;
	int aidx;

	/* Scan the list for the max id */
	z_info->a_max = 0;
	a = parser_priv(p);
	while (a) {
		z_info->a_max++;
		a = a->next;
	}

	/* Allocate the direct access list and copy the data to it */
	a_info = mem_zalloc((z_info->a_max + 1) * sizeof(*a));
	aup_info = mem_zalloc((z_info->a_max + 1) * sizeof(*aup_info));
	aidx = z_info->a_max;
	for (a = parser_priv(p); a; a = n, aidx--) {
		assert(aidx > 0);

		memcpy(&a_info[aidx], a, sizeof(*a));
		a_info[aidx].aidx = aidx;
		n = a->next;
		a_info[aidx].next = (aidx < z_info->a_max) ?
			&a_info[aidx + 1] : NULL;
		mem_free(a);

		aup_info[aidx].aidx = aidx;
	}
	z_info->a_max += 1;

	parser_destroy(p);
	return 0;
}

struct file_parser randart_parser = {
	"randart",
	init_parse_artifact,
	run_parse_randart,
	finish_parse_randart,
	cleanup_artifact
};

/**
 * ------------------------------------------------------------------------
 * Initialize object properties
 * ------------------------------------------------------------------------ */

static enum parser_error parse_object_property_name(struct parser *p) {
	const char *name = parser_getstr(p, "name");
	struct obj_property *h = parser_priv(p);
	struct obj_property *prop = mem_zalloc(sizeof *prop);
	int i;

	prop->next = h;
	parser_setpriv(p, prop);
	prop->name = string_make(name);

	/* Set all the type multipliers to the default of 1 */
	for (i = 0; i < TV_MAX; i++) {
		prop->type_mult[i] = 1;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_type(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *name = parser_getstr(p, "type");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (streq(name, "stat")) {
		prop->type = OBJ_PROPERTY_STAT;
	} else if (streq(name, "mod")) {
		prop->type = OBJ_PROPERTY_MOD;
	} else if (streq(name, "flag")) {
		prop->type = OBJ_PROPERTY_FLAG;
	} else if (streq(name, "ignore")) {
		prop->type = OBJ_PROPERTY_IGNORE;
	} else if (streq(name, "resistance")) {
		prop->type = OBJ_PROPERTY_RESIST;
	} else if (streq(name, "vulnerability")) {
		prop->type = OBJ_PROPERTY_VULN;
	} else if (streq(name, "immunity")) {
		prop->type = OBJ_PROPERTY_IMM;
	} else {
		return PARSE_ERROR_INVALID_PROPERTY;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_subtype(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *name = parser_getstr(p, "subtype");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (streq(name, "sustain")) {
		prop->subtype = OFT_SUST;
	} else if (streq(name, "protection")) {
		prop->subtype = OFT_PROT;
	} else if (streq(name, "misc ability")) {
		prop->subtype = OFT_MISC;
	} else if (streq(name, "light")) {
		prop->subtype = OFT_LIGHT;
	} else if (streq(name, "melee")) {
		prop->subtype = OFT_MELEE;
	} else if (streq(name, "bad")) {
		prop->subtype = OFT_BAD;
	} else if (streq(name, "dig")) {
		prop->subtype = OFT_DIG;
	} else if (streq(name, "throw")) {
		prop->subtype = OFT_THROW;
	} else {
		return PARSE_ERROR_INVALID_SUBTYPE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_id_type(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *name = parser_getstr(p, "id");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (streq(name, "on effect")) {
		prop->id_type = OFID_NORMAL;
	} else if (streq(name, "timed")) {
		prop->id_type = OFID_TIMED;
	} else if (streq(name, "on wield")) {
		prop->id_type = OFID_WIELD;
	} else {
		return PARSE_ERROR_INVALID_ID_TYPE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_code(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *code = parser_getstr(p, "code");
	int index = -1;

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!prop->type) {
		return PARSE_ERROR_MISSING_OBJ_PROP_TYPE;
	}
	if (prop->type == OBJ_PROPERTY_STAT) {
		index = code_index_in_array(obj_mods, code);
	} else if (prop->type == OBJ_PROPERTY_MOD) {
		index = code_index_in_array(obj_mods, code);
	} else if (prop->type == OBJ_PROPERTY_FLAG) {
		index = code_index_in_array(obj_flags, code);
	} else if (prop->type == OBJ_PROPERTY_IGNORE) {
		index = code_index_in_array(element_names, code);
	} else if (prop->type == OBJ_PROPERTY_RESIST) {
		index = code_index_in_array(element_names, code);
	} else if (prop->type == OBJ_PROPERTY_VULN) {
		index = code_index_in_array(element_names, code);
	} else if (prop->type == OBJ_PROPERTY_IMM) {
		index = code_index_in_array(element_names, code);
	}
	if (index >= 0) {
		prop->index = index;
	} else {
		return PARSE_ERROR_INVALID_OBJ_PROP_CODE;
	}
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_power(struct parser *p) {
	struct obj_property *prop = parser_priv(p);

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	prop->power = parser_getint(p, "power");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_mult(struct parser *p) {
	struct obj_property *prop = parser_priv(p);

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	prop->mult = parser_getint(p, "mult");
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_type_mult(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	int tval, mult;

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	tval = tval_find_idx(parser_getsym(p, "type"));
	if (tval < 0) {
		return PARSE_ERROR_UNRECOGNISED_TVAL;
	}
	mult = parser_getint(p, "mult");
	prop->type_mult[tval] = mult;
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_adjective(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *adj = parser_getstr(p, "adj");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(prop->adjective);
	prop->adjective = string_make(adj);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_neg_adj(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *adj = parser_getstr(p, "neg_adj");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(prop->neg_adj);
	prop->neg_adj = string_make(adj);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_msg(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *msg = parser_getstr(p, "msg");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(prop->msg);
	prop->msg = string_make(msg);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_desc(struct parser *p) {
	struct obj_property *prop = parser_priv(p);
	const char *desc = parser_getstr(p, "desc");

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	string_free(prop->desc);
	prop->desc = string_make(desc);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_object_property_bindui(struct parser* p) {
	struct obj_property *prop = parser_priv(p);
	const char *uiname = parser_getsym(p, "ui");
	bool isaux = (parser_getint(p, "aux") != 0);
	bool have_val = parser_hasval(p, "uival");
	int val = (have_val) ? parser_getint(p, "uival") : 0;

	if (!prop) {
		return PARSE_ERROR_MISSING_RECORD_HEADER;
	}
	if (!prop->type) {
		return PARSE_ERROR_MISSING_OBJ_PROP_TYPE;
	}
	(void) bind_object_property_to_ui_entry_by_name(uiname, prop->type,
		prop->index, val, have_val, isaux);
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_object_property(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_object_property_name);
	parser_reg(p, "code str code", parse_object_property_code);
	parser_reg(p, "type str type", parse_object_property_type);
	parser_reg(p, "subtype str subtype", parse_object_property_subtype);
	parser_reg(p, "id-type str id", parse_object_property_id_type);
	parser_reg(p, "power int power", parse_object_property_power);
	parser_reg(p, "mult int mult", parse_object_property_mult);
	parser_reg(p, "type-mult sym type int mult", parse_object_property_type_mult);
	parser_reg(p, "adjective str adj", parse_object_property_adjective);
	parser_reg(p, "neg-adjective str neg_adj", parse_object_property_neg_adj);
	parser_reg(p, "msg str msg", parse_object_property_msg);
	parser_reg(p, "desc str desc", parse_object_property_desc);
	parser_reg(p, "bindui sym ui int aux ?int uival", parse_object_property_bindui);
	return p;
}

static errr run_parse_object_property(struct parser *p) {
	return parse_file_quit_not_found(p, "object_property");
}

static errr finish_parse_object_property(struct parser *p) {
	struct obj_property *prop, *n;
	int idx;

	/* Scan the list for the max id */
	z_info->property_max = 0;
	prop = parser_priv(p);
	while (prop) {
		z_info->property_max++;
		prop = prop->next;
	}

	/* Allocate the direct access list and copy the data to it */
	obj_properties = mem_zalloc((z_info->property_max + 1) * sizeof(*prop));
	idx = z_info->property_max;
	for (prop = parser_priv(p); prop; prop = n, idx--) {
		assert(idx > 0);

		memcpy(&obj_properties[idx], prop, sizeof(*prop));
		n = prop->next;

		mem_free(prop);
	}
	z_info->property_max += 1;

	parser_destroy(p);
	return 0;
}

static void cleanup_object_property(void)
{
	int idx;
	for (idx = 0; idx < z_info->property_max; idx++) {
		struct obj_property *prop = &obj_properties[idx];

		string_free(prop->name);
		string_free(prop->adjective);
		string_free(prop->neg_adj);
		string_free(prop->msg);
		string_free(prop->desc);
	}
	mem_free(obj_properties);
}

struct file_parser object_property_parser = {
	"object_property",
	init_parse_object_property,
	run_parse_object_property,
	finish_parse_object_property,
	cleanup_object_property
};

