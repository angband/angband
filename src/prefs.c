/*
 * File: prefs.c
 * Purpose: Pref file handling code
 *
 * Copyright (c) 2003 Takeshi Mogami, Robert Ruehlmann
 * Copyright (c) 2007 Pete Mack
 * Copyright (c) 2010 Andi Sidwell
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
#include "keymap.h"
#include "prefs.h"
#include "squelch.h"
#include "spells.h"


/*** Pref file saving code ***/

/*
 * Header and footer marker string for pref file dumps
 */
static const char *dump_separator = "#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#";


/*
 * Remove old lines from pref files
 */
static void remove_old_dump(const char *cur_fname, const char *mark)
{
	bool between_marks = FALSE;
	bool changed = FALSE;

	char buf[1024];

	char start_line[1024];
	char end_line[1024];

	char new_fname[1024];

	ang_file *new_file;
	ang_file *cur_file;


	/* Format up some filenames */
	strnfmt(new_fname, sizeof(new_fname), "%s.new", cur_fname);

	/* Work out what we expect to find */
	strnfmt(start_line, sizeof(start_line), "%s begin %s",
			dump_separator, mark);
	strnfmt(end_line,   sizeof(end_line),   "%s end %s",
			dump_separator, mark);



	/* Open current file */
	cur_file = file_open(cur_fname, MODE_READ, -1);
	if (!cur_file) return;

	/* Open new file */
	new_file = file_open(new_fname, MODE_WRITE, FTYPE_TEXT);
	if (!new_file)
	{
		msg("Failed to create file %s", new_fname);
		return;
	}

	/* Loop for every line */
	while (file_getl(cur_file, buf, sizeof(buf)))
	{
		/* If we find the start line, turn on */
		if (!strcmp(buf, start_line))
		{
			between_marks = TRUE;
		}

		/* If we find the finish line, turn off */
		else if (!strcmp(buf, end_line))
		{
			between_marks = FALSE;
			changed = TRUE;
		}

		if (!between_marks)
		{
			/* Copy orginal line */
			file_putf(new_file, "%s\n", buf);
		}
	}

	/* Close files */
	file_close(cur_file);
	file_close(new_file);

	/* If there are changes, move things around */
	if (changed)
	{
		char old_fname[1024];
		strnfmt(old_fname, sizeof(old_fname), "%s.old", cur_fname);

		if (file_move(cur_fname, old_fname))
		{
			file_move(new_fname, cur_fname);
			file_delete(old_fname);
		}
	}

	/* Otherwise just destroy the new file */
	else
	{
		file_delete(new_fname);
	}
}


/*
 * Output the header of a pref-file dump
 */
static void pref_header(ang_file *fff, const char *mark)
{
	/* Start of dump */
	file_putf(fff, "%s begin %s\n", dump_separator, mark);

	file_putf(fff, "# *Warning!*  The lines below are an automatic dump.\n");
	file_putf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");
}

/*
 * Output the footer of a pref-file dump
 */
static void pref_footer(ang_file *fff, const char *mark)
{
	file_putf(fff, "# *Warning!*  The lines above are an automatic dump.\n");
	file_putf(fff, "# Don't edit them; changes will be deleted and replaced automatically.\n");

	/* End of dump */
	file_putf(fff, "%s end %s\n", dump_separator, mark);
}


/*
 * Write all current options to a user preference file.
 */
void option_dump(ang_file *fff)
{
	int i, j;

	/* Dump options (skip cheat, adult, score) */
	for (i = 0; i < OPT_CHEAT; i++)
	{
		const char *name = option_name(i);
		if (!name) continue;

		/* Comment */
		file_putf(fff, "# Option '%s'\n", option_desc(i));

		/* Dump the option */
		if (op_ptr->opt[i])
			file_putf(fff, "Y:%s\n", name);
		else
			file_putf(fff, "X:%s\n", name);

		/* Skip a line */
		file_putf(fff, "\n");
	}

	/* Dump window flags */
	for (i = 1; i < ANGBAND_TERM_MAX; i++)
	{
		/* Require a real window */
		if (!angband_term[i]) continue;

		/* Check each flag */
		for (j = 0; j < (int)N_ELEMENTS(window_flag_desc); j++)
		{
			/* Require a real flag */
			if (!window_flag_desc[j]) continue;

			/* Comment */
			file_putf(fff, "# Window '%s', Flag '%s'\n",
				angband_term_name[i], window_flag_desc[j]);

			/* Dump the flag */
			if (op_ptr->window_flag[i] & (1L << j))
				file_putf(fff, "W:%d:%d:1\n", i, j);
			else
				file_putf(fff, "W:%d:%d:0\n", i, j);

			/* Skip a line */
			file_putf(fff, "\n");
		}
	}

	keymap_dump(fff);
}




/* Dump monsters */
void dump_monsters(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];
		byte attr = r_ptr->x_attr;
		byte chr = r_ptr->x_char;

		/* Skip non-entries */
		if (!r_ptr->name) continue;

		file_putf(fff, "# Monster: %s\n", r_ptr->name);
		file_putf(fff, "R:%d:%d:%d\n", i, attr, chr);
	}
}

/* Dump objects */
void dump_objects(ang_file *fff)
{
	int i;

	file_putf(fff, "# Objects\n");

	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		const char *name = k_ptr->name;

		if (!name) continue;
		if (name[0] == '&' && name[1] == ' ')
			name += 2;

		file_putf(fff, "K:%s:%s:%d:%d\n", tval_find_name(k_ptr->tval),
				name, k_ptr->x_attr, k_ptr->x_char);
	}
}

/* Dump features */
void dump_features(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];
		size_t j;

		/* Skip non-entries */
		if (!f_ptr->name) continue;

		/* Skip mimic entries -- except invisible trap */
		if ((f_ptr->mimic != i) && (i != FEAT_INVIS)) continue;

		file_putf(fff, "# Terrain: %s\n", f_ptr->name);
		for (j = 0; j < FEAT_LIGHTING_MAX; j++)
		{
			byte attr = f_ptr->x_attr[j];
			byte chr = f_ptr->x_char[j];

			const char *light = NULL;
			if (j == FEAT_LIGHTING_BRIGHT)
				light = "bright";
			else if (j == FEAT_LIGHTING_LIT)
				light = "lit";
			else if (j == FEAT_LIGHTING_DARK)
				light = "dark";

			assert(light);

			file_putf(fff, "F:%d:%s:%d:%d\n", i, light, attr, chr);
		}
	}
}

/* Dump flavors */
void dump_flavors(ang_file *fff)
{
	struct flavor *f;

	for (f = flavors; f; f = f->next) {
		byte attr = f->x_attr;
		byte chr = f->x_char;

		file_putf(fff, "# Item flavor: %s\n", f->text);
		file_putf(fff, "L:%d:%d:%d\n\n", f->fidx, attr, chr);
	}
}

/* Dump colors */
void dump_colors(ang_file *fff)
{
	int i;

	for (i = 0; i < MAX_COLORS; i++)
	{
		int kv = angband_color_table[i][0];
		int rv = angband_color_table[i][1];
		int gv = angband_color_table[i][2];
		int bv = angband_color_table[i][3];

		const char *name = "unknown";

		/* Skip non-entries */
		if (!kv && !rv && !gv && !bv) continue;

		/* Extract the color name */
		if (i < BASIC_COLORS) name = color_table[i].name;

		file_putf(fff, "# Color: %s\n", name);
		file_putf(fff, "V:%d:%d:%d:%d:%d\n\n", i, kv, rv, gv, bv);
	}
}




/**
 * Save a set of preferences to file, overwriting any old preferences with the
 * same title.
 *
 * \param path is the filename to dump to
 * \param dump is a pointer to the function that does the writing to file
 * \param title is the name of this set of preferences
 *
 * \returns TRUE on success, FALSE otherwise.
 */
bool prefs_save(const char *path, void (*dump)(ang_file *), const char *title)
{
	ang_file *fff;

	/* Remove old keymaps */
	remove_old_dump(path, title);

	fff = file_open(path, MODE_APPEND, FTYPE_TEXT);
	if (!fff) return FALSE;

	/* Append the header */
	pref_header(fff, title);
	file_putf(fff, "\n\n");
	file_putf(fff, "# %s definitions\n\n", strstr(title, " "));
	
	dump(fff);

	file_putf(fff, "\n\n\n");
	pref_footer(fff, title);
	file_close(fff);

	return TRUE;
}






/*** Pref file parser ***/


/**
 * Private data for pref file parsing.
 */
struct prefs_data
{
	bool bypass;
	struct keypress keymap_buffer[KEYMAP_ACTION_MAX];
	bool user;
	bool loaded_window_flag[ANGBAND_TERM_MAX];
	u32b window_flags[ANGBAND_TERM_MAX];
};


/**
 * Load another file.
 */
static enum parser_error parse_prefs_load(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);
	const char *file;

	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	file = parser_getstr(p, "file");
	(void)process_pref_file(file, TRUE, d->user);

	return PARSE_ERROR_NONE;
}

/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
static const char *process_pref_file_expr(char **sp, char *fp)
{
	const char *v;

	char *b;
	char *s;

	char f = ' ';

	/* Initial */
	s = (*sp);

	/* Skip spaces */
	while (isspace((unsigned char)*s)) s++;

	/* Save start */
	b = s;

	/* Default */
	v = "?o?o?";

	/* Analyze */
	if (*s == '[')
	{
		const char *p;
		const char *t;

		/* Skip [ */
		s++;

		/* First */
		t = process_pref_file_expr(&s, &f);

		/* Oops */
		if (!*t)
		{
			/* Nothing */
		}

		/* Function: IOR */
		else if (streq(t, "IOR"))
		{
			v = "0";
			while (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "1";
			}
		}

		/* Function: AND */
		else if (streq(t, "AND"))
		{
			v = "1";
			while (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && streq(t, "0")) v = "0";
			}
		}

		/* Function: NOT */
		else if (streq(t, "NOT"))
		{
			v = "1";
			while (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(t, "0")) v = "0";
			}
		}

		/* Function: EQU */
		else if (streq(t, "EQU"))
		{
			v = "1";
			if (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != ']'))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && !streq(p, t)) v = "0";
			}
		}

		/* Function: LEQ */
		else if (streq(t, "LEQ"))
		{
			v = "1";
			if (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != ']'))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) >= 0)) v = "0";
			}
		}

		/* Function: GEQ */
		else if (streq(t, "GEQ"))
		{
			v = "1";
			if (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
			}
			while (*s && (f != ']'))
			{
				p = t;
				t = process_pref_file_expr(&s, &f);
				if (*t && (strcmp(p, t) <= 0)) v = "0";
			}
		}

		/* Oops */
		else
		{
			while (*s && (f != ']'))
			{
				t = process_pref_file_expr(&s, &f);
			}
		}

		/* Verify ending */
		if (f != ']') v = "?x?x?";

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';
	}

	/* Other */
	else
	{
		/* Accept all printables except spaces and brackets */
		while (isprint((unsigned char)*s) && !strchr(" []", *s)) ++s;

		/* Extract final and Terminate */
		if ((f = *s) != '\0') *s++ = '\0';

		/* Variable */
		if (*b == '$')
		{
			if (streq(b+1, "SYS"))
				v = ANGBAND_SYS;
			else if (streq(b+1, "GRAF"))
				v = ANGBAND_GRAF;
			else if (streq(b+1, "RACE"))
				v = p_ptr->race->name;
			else if (streq(b+1, "CLASS"))
				v = p_ptr->class->name;
			else if (streq(b+1, "PLAYER"))
				v = op_ptr->base_name;
		}

		/* Constant */
		else
		{
			v = b;
		}
	}

	/* Save */
	(*fp) = f;
	(*sp) = s;

	return v;
}

static enum parser_error parse_prefs_expr(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);

	const char *v;
	char *str;
	char *expr;
	char f;

	assert(d != NULL);

	/* XXX this can be avoided with a rewrite of process_pref_file_expr */
	str = expr = string_make(parser_getstr(p, "expr"));

	/* Parse the expr */
	v = process_pref_file_expr(&expr, &f);

	/* Set flag */
	d->bypass = streq(v, "0");

	string_free(str);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_k(struct parser *p)
{
	int tvi, svi;
	object_kind *kind;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	tvi = tval_find_idx(parser_getsym(p, "tval"));
	if (tvi < 0)
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	svi = lookup_sval(tvi, parser_getsym(p, "sval"));
	if (svi < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	kind = lookup_kind(tvi, svi);
	if (!kind)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	kind->x_attr = (byte)parser_getint(p, "attr");
	kind->x_char = (char)parser_getint(p, "char");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_r(struct parser *p)
{
	int idx;
	monster_race *monster;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx >= z_info->r_max)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	monster = &r_info[idx];
	monster->x_attr = (byte)parser_getint(p, "attr");
	monster->x_char = (char)parser_getint(p, "char");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_f(struct parser *p)
{
	int idx;
	feature_type *feature;

	const char *lighting;
	int light_idx;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx >= z_info->f_max)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	lighting = parser_getsym(p, "lighting");
	if (streq(lighting, "bright"))
		light_idx = FEAT_LIGHTING_BRIGHT;
	else if (streq(lighting, "lit"))
		light_idx = FEAT_LIGHTING_LIT;
	else if (streq(lighting, "dark"))
		light_idx = FEAT_LIGHTING_DARK;
	else if (streq(lighting, "all"))
		light_idx = FEAT_LIGHTING_MAX;
	else
		return PARSE_ERROR_GENERIC; /* xxx fixme */

	if (light_idx < FEAT_LIGHTING_MAX)
	{
		feature = &f_info[idx];
		feature->x_attr[light_idx] = (byte)parser_getint(p, "attr");
		feature->x_char[light_idx] = (char)parser_getint(p, "char");
	}
	else
	{
		for (light_idx = 0; light_idx < FEAT_LIGHTING_MAX; light_idx++)
		{
			feature = &f_info[idx];
			feature->x_attr[light_idx] = (byte)parser_getint(p, "attr");
			feature->x_char[light_idx] = (char)parser_getint(p, "char");
		}
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_gf(struct parser *p)
{
	bool types[GF_MAX] = { 0 };
	const char *direction;
	int motion;

	char *s, *t;

	size_t i;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	/* Parse the type, which is a | seperated list of GF_ constants */
	s = string_make(parser_getsym(p, "type"));
	t = strtok(s, "| ");
	while (t) {
		if (streq(t, "*")) {
			memset(types, TRUE, sizeof types);
		} else {
			int idx = gf_name_to_idx(t);
			if (idx == -1)
				return PARSE_ERROR_INVALID_VALUE;

			types[idx] = TRUE;
		}

		t = strtok(NULL, "| ");
	}

	string_free(s);

	direction = parser_getsym(p, "direction");
	if (streq(direction, "static"))
		motion = BOLT_NO_MOTION;
	else if (streq(direction, "0"))
		motion = BOLT_0;
	else if (streq(direction, "45"))
		motion = BOLT_45;
	else if (streq(direction, "90"))
		motion = BOLT_90;
	else if (streq(direction, "135"))
		motion = BOLT_135;
	else
		return PARSE_ERROR_INVALID_VALUE;

	for (i = 0; i < GF_MAX; i++) {
		if (!types[i]) continue;

		gf_to_attr[i][motion] = (byte)parser_getuint(p, "attr");
		gf_to_char[i][motion] = (char)parser_getuint(p, "char");
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_l(struct parser *p)
{
	unsigned int idx;
	struct flavor *flavor;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	for (flavor = flavors; flavor; flavor = flavor->next)
		if (flavor->fidx == idx)
			break;

	if (flavor) {
		flavor->x_attr = (byte)parser_getint(p, "attr");
		flavor->x_char = (char)parser_getint(p, "char");
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_e(struct parser *p)
{
	int tvi, a;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	tvi = tval_find_idx(parser_getsym(p, "tval"));
	if (tvi < 0 || tvi >= (long)N_ELEMENTS(tval_to_attr))
		return PARSE_ERROR_UNRECOGNISED_TVAL;

	a = parser_getint(p, "attr");
	if (a) tval_to_attr[tvi] = (byte) a;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_q(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	if (parser_hasval(p, "sval") && parser_hasval(p, "flag"))
	{
		object_kind *kind;
		int tvi, svi;

		tvi = tval_find_idx(parser_getsym(p, "n"));
		if (tvi < 0)
			return PARSE_ERROR_UNRECOGNISED_TVAL;
	
		svi = lookup_sval(tvi, parser_getsym(p, "sval"));
		if (svi < 0)
			return PARSE_ERROR_UNRECOGNISED_SVAL;

		kind = lookup_kind(tvi, svi);
		if (!kind)
			return PARSE_ERROR_UNRECOGNISED_SVAL;

		kind->squelch = parser_getint(p, "flag");
	}
	else
	{
		int idx = parser_getint(p, "idx");
		int level = parser_getint(p, "n");

		squelch_level[idx] = level;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_b(struct parser *p)
{
	int idx;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx > z_info->k_max)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	add_autoinscription(idx, parser_getstr(p, "text"));

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_a(struct parser *p)
{
	const char *act;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	act = parser_getstr(p, "act");
	keypress_from_text(d->keymap_buffer, N_ELEMENTS(d->keymap_buffer), act);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_c(struct parser *p)
{
	int mode;
	struct keypress tmp[2];

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	mode = parser_getint(p, "mode");
	if (mode < 0 || mode >= KEYMAP_MODE_MAX)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	keypress_from_text(tmp, N_ELEMENTS(tmp), parser_getstr(p, "key"));
	if (tmp[0].type != EVT_KBRD || tmp[1].type != EVT_NONE)
		return PARSE_ERROR_FIELD_TOO_LONG;

	keymap_add(mode, tmp[0], d->keymap_buffer, d->user);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_m(struct parser *p)
{
	int a, type;
	const char *attr;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	type = parser_getint(p, "type");
	attr = parser_getsym(p, "attr");

	if (strlen(attr) > 1)
		a = color_text_to_attr(attr);
	else
		a = color_char_to_attr(attr[0]);

	if (a < 0)
		return PARSE_ERROR_INVALID_COLOR;

	message_color_define((u16b)type, (byte)a);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_v(struct parser *p)
{
	int idx;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx > MAX_COLORS)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	angband_color_table[idx][0] = parser_getint(p, "k");
	angband_color_table[idx][1] = parser_getint(p, "r");
	angband_color_table[idx][2] = parser_getint(p, "g");
	angband_color_table[idx][3] = parser_getint(p, "b");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_w(struct parser *p)
{
	int window;
	size_t flag;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	window = parser_getint(p, "window");
	if (window <= 0 || window >= ANGBAND_TERM_MAX)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	flag = parser_getuint(p, "flag");
	if (flag >= N_ELEMENTS(window_flag_desc))
		return PARSE_ERROR_OUT_OF_BOUNDS;

	if (window_flag_desc[flag])
	{
		int value = parser_getuint(p, "value");
		if (value)
			d->window_flags[window] |= (1L << flag);
		else
			d->window_flags[window] &= ~(1L << flag);
	}

	d->loaded_window_flag[window] = TRUE;

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_x(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	/* XXX check for valid option */
	option_set(parser_getstr(p, "option"), FALSE);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_y(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	option_set(parser_getstr(p, "option"), TRUE);

	return PARSE_ERROR_NONE;
}


static struct parser *init_parse_prefs(bool user)
{
	struct parser *p = parser_new();
	struct prefs_data *pd = mem_zalloc(sizeof *pd);
	int i;

	parser_setpriv(p, pd);
	pd->user = user;
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		pd->loaded_window_flag[i] = FALSE;
	}

	parser_reg(p, "% str file", parse_prefs_load);
	parser_reg(p, "? str expr", parse_prefs_expr);
	parser_reg(p, "K sym tval sym sval int attr int char", parse_prefs_k);
	parser_reg(p, "R uint idx int attr int char", parse_prefs_r);
	parser_reg(p, "F uint idx sym lighting int attr int char", parse_prefs_f);
	parser_reg(p, "GF sym type sym direction uint attr uint char", parse_prefs_gf);
	parser_reg(p, "L uint idx int attr int char", parse_prefs_l);
	parser_reg(p, "E sym tval int attr", parse_prefs_e);
	parser_reg(p, "Q sym idx sym n ?sym sval ?sym flag", parse_prefs_q);
		/* XXX should be split into two kinds of line */
	parser_reg(p, "B uint idx str text", parse_prefs_b);
		/* XXX idx should be {tval,sval} pair! */
	parser_reg(p, "A str act", parse_prefs_a);
	parser_reg(p, "C int mode str key", parse_prefs_c);
	parser_reg(p, "M int type sym attr", parse_prefs_m);
	parser_reg(p, "V uint idx int k int r int g int b", parse_prefs_v);
	parser_reg(p, "W int window uint flag uint value", parse_prefs_w);
	parser_reg(p, "X str option", parse_prefs_x);
	parser_reg(p, "Y str option", parse_prefs_y);

	return p;
}

errr finish_parse_prefs(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);
	int i;

	/* Update sub-windows based on the newly read-in prefs.
	 *
	 * The op_ptr->window_flag[] array cannot be updated directly during
	 * parsing since the changes between the existing flags and the new
	 * are used to set/unset the event handlers that update the windows.
	 *
	 * Build a complete set to pass to subwindows_set_flags() by loading
	 * any that weren't read in by the parser from the existing set.
	 */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		if (!d->loaded_window_flag[i])
			d->window_flags[i] = op_ptr->window_flag[i];
	}
	subwindows_set_flags(d->window_flags, ANGBAND_TERM_MAX);

	return PARSE_ERROR_NONE;
}

errr process_pref_file_command(const char *s)
{
	struct parser *p = init_parse_prefs(TRUE);
	errr e = parser_parse(p, s);
	mem_free(parser_priv(p));
	parser_destroy(p);
	return e;
}


static void print_error(const char *name, struct parser *p) {
	struct parser_state s;
	parser_getstate(p, &s);
	msg("Parse error in %s line %d column %d: %s: %s", name,
	           s.line, s.col, s.msg, parser_error_str[s.error]);
	message_flush();
}


/*
 * Process the user pref file with the given name.
 * "quiet" means "don't complain about not finding the file.
 *
 * 'user' should be TRUE if the pref file loaded is user-specific and not
 * a game default.
 *
 * Returns TRUE if everything worked OK, false otherwise
 */
bool process_pref_file(const char *name, bool quiet, bool user)
{
	char buf[1024];

	ang_file *f;
	struct parser *p;
	errr e = 0;

	int line_no = 0;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_PREF, name);
	if (!file_exists(buf))
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);

	f = file_open(buf, MODE_READ, -1);
	if (!f)
	{
		if (!quiet)
			msg("Cannot open '%s'.", buf);
	}
	else
	{
		char line[1024];

		p = init_parse_prefs(user);
		while (file_getl(f, line, sizeof line))
		{
			line_no++;

			e = parser_parse(p, line);
			if (e != PARSE_ERROR_NONE)
			{
				print_error(buf, p);
				break;
			}
		}
		finish_parse_prefs(p);

		file_close(f);
		mem_free(parser_priv(p));
		parser_destroy(p);
	}

	/* Result */
	return e == PARSE_ERROR_NONE;
}
