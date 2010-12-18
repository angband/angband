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
#include "macro.h"
#include "prefs.h"
#include "squelch.h"


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
		msg_format("Failed to create file %s", new_fname);
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
 * Save autoinscription data to a pref file.
 */
/* XXX should be renamed dump_* */
void autoinsc_dump(ang_file *fff)
{
	int i;
	if (!inscriptions) return;

	file_putf(fff, "# Autoinscription settings\n");
	file_putf(fff, "# B:item kind:inscription\n\n");

	for (i = 0; i < inscriptions_count; i++)
	{
		object_kind *k_ptr = &k_info[inscriptions[i].kind_idx];

		file_putf(fff, "# Autoinscription for %s\n", k_ptr->name);
		file_putf(fff, "B:%d:%s\n\n", inscriptions[i].kind_idx,
		        quark_str(inscriptions[i].inscription_idx));
	}

	file_putf(fff, "\n");
}

/*
 * Save squelch data to a pref file.
 */
void squelch_dump(ang_file *fff)
{
	int i;
	file_putf(fff, "# Squelch settings\n");

	for (i = 1; i < z_info->k_max; i++)
	{
		int tval = k_info[i].tval;
		int sval = k_info[i].sval;
		bool squelch = k_info[i].squelch;

		/* Dump the squelch info */
		if (tval || sval)
			file_putf(fff, "Q:%d:%d:%d:%d\n", i, tval, sval, squelch);
	}

	file_putf(fff, "\n");
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

	autoinsc_dump(fff);
#if 0
	/* Dumping squelch settings caused problems, see #784 */
	squelch_dump(fff);
#endif
}



#ifdef ALLOW_MACROS

/*
 * Append all current macros to the given file
 */
void macro_dump(ang_file *fff)
{
	int i;
	char buf[1024];

	/* Dump them */
	for (i = 0; i < macro__num; i++)
	{
		/* Start the macro */
		file_putf(fff, "# Macro '%d'\n", i);

		/* Extract the macro action */
		ascii_to_text(buf, sizeof(buf), macro__act[i]);
		file_putf(fff, "A:%s\n", buf);

		/* Extract the macro pattern */
		ascii_to_text(buf, sizeof(buf), macro__pat[i]);
		file_putf(fff, "P:%s\n", buf);

		file_putf(fff, "\n");
	}
}



/*
 * Hack -- Append all keymaps to the given file.
 *
 * Hack -- We only append the keymaps for the "active" mode.
 */
void keymap_dump(ang_file *fff)
{
	size_t i;
	int mode;
	char buf[1024];

	if (OPT(rogue_like_commands))
		mode = KEYMAP_MODE_ROGUE;
	else
		mode = KEYMAP_MODE_ORIG;

	for (i = 0; i < N_ELEMENTS(keymap_act[mode]); i++)
	{
		char key[2] = "?";
		const char *act;

		/* Loop up the keymap */
		act = keymap_act[mode][i];

		/* Skip empty keymaps */
		if (!act) continue;

		/* Encode the action */
		ascii_to_text(buf, sizeof(buf), act);

		/* Dump the keymap action */
		file_putf(fff, "A:%s\n", buf);

		/* Convert the key into a string */
		key[0] = i;

		/* Encode the key */
		ascii_to_text(buf, sizeof(buf), key);

		/* Dump the keymap pattern */
		file_putf(fff, "C:%d:%s\n", mode, buf);

		/* Skip a line */
		file_putf(fff, "\n");
	}

}


#endif 



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
		file_putf(fff, "R:%d:0x%02X:0x%02X\n", i, attr, chr);
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
		byte attr = f_ptr->x_attr;
		byte chr = f_ptr->x_char;

		/* Skip non-entries */
		if (!f_ptr->name) continue;

		/* Skip mimic entries -- except invisible trap */
		if ((f_ptr->mimic != i) && (i != FEAT_INVIS)) continue;

		file_putf(fff, "# Terrain: %s\n", f_ptr->name);
		file_putf(fff, "F:%d:0x%02X:0x%02X\n", i, attr, chr);
	}
}

/* Dump flavors */
void dump_flavors(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->flavor_max; i++)
	{
		flavor_type *x_ptr = &flavor_info[i];
		byte attr = x_ptr->x_attr;
		byte chr = x_ptr->x_char;

		file_putf(fff, "# Item flavor: %s\n", x_ptr->text);
		file_putf(fff, "L:%d:0x%02X:0x%02X\n\n", i, attr, chr);
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
		file_putf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n", i, kv, rv, gv, bv);
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

	/* Remove old macros */
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

/* Forward declare */
static struct parser *init_parse_prefs(void);


/**
 * Private data for pref file parsing.
 */
struct prefs_data
{
	bool bypass;
	char macro_buffer[1024];
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
	(void)process_pref_file(file, TRUE);

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
				v = rp_ptr->name;
			else if (streq(b+1, "CLASS"))
				v = cp_ptr->name;
			else if (streq(b+1, "PLAYER"))
				v = op_ptr->base_name;
			else if (streq(b+1, "VERSION"))
				v = VERSION_STRING;
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

	string_free(str);

	/* Set flag */
	d->bypass = streq(v, "0");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_k(struct parser *p)
{
	int tvi, svi, idx;
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

	idx = lookup_kind(tvi, svi);
	if (idx < 0)
		return PARSE_ERROR_UNRECOGNISED_SVAL;

	kind = &k_info[idx];
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

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx >= z_info->f_max)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	feature = &f_info[idx];
	feature->x_attr = (byte)parser_getint(p, "attr");
	feature->x_char = (char)parser_getint(p, "char");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_s(struct parser *p)
{
	size_t idx;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx >= N_ELEMENTS(misc_to_attr))
		return PARSE_ERROR_OUT_OF_BOUNDS;

	misc_to_attr[idx] = (byte)parser_getint(p, "attr");
	misc_to_char[idx] = (char)parser_getint(p, "char");

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_l(struct parser *p)
{
	int idx;
	flavor_type *flavor;

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	idx = parser_getuint(p, "idx");
	if (idx >= z_info->flavor_max)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	flavor = &flavor_info[idx];
	flavor->x_attr = (byte)parser_getint(p, "attr");
	flavor->x_char = (char)parser_getint(p, "char");

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
		int tvi, svi, idx;

		tvi = tval_find_idx(parser_getsym(p, "n"));
		if (tvi < 0)
			return PARSE_ERROR_UNRECOGNISED_TVAL;
	
		svi = lookup_sval(tvi, parser_getsym(p, "sval"));
		if (svi < 0)
			return PARSE_ERROR_UNRECOGNISED_SVAL;

		idx = lookup_kind(tvi, svi);
		if (idx < 0)
			return PARSE_ERROR_UNRECOGNISED_SVAL;

		kind = &k_info[idx];
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
	text_to_ascii(d->macro_buffer, sizeof(d->macro_buffer), act);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_p(struct parser *p)
{
	char tmp[1024];

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	text_to_ascii(tmp, sizeof(tmp), parser_getstr(p, "key"));
	macro_add(tmp, d->macro_buffer);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_c(struct parser *p)
{
	int mode;
	byte j;
	char tmp[1024];

	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	mode = parser_getint(p, "mode");
	if (mode < 0 || mode >= KEYMAP_MODES)
		return PARSE_ERROR_OUT_OF_BOUNDS;

	text_to_ascii(tmp, sizeof(tmp), parser_getstr(p, "key"));
	if (!tmp[0] || tmp[1])
		return PARSE_ERROR_FIELD_TOO_LONG;

	j = (byte)tmp[0];

	string_free(keymap_act[mode][j]);
	keymap_act[mode][j] = string_make(d->macro_buffer);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_prefs_t(struct parser *p)
{
	struct prefs_data *d = parser_priv(p);
	assert(d != NULL);
	if (d->bypass) return PARSE_ERROR_NONE;

	/* set macro trigger names and a template */
	/* Process "T:<template>:<modifier chr>:<modifier name>:..." */
	if (parser_hasval(p, "n4"))
	{
		const char *template = parser_getsym(p, "n1");
		const char *chars = parser_getsym(p, "n2");
		const char *name0 = parser_getsym(p, "n3");
		const char *namelist = parser_getstr(p, "n4");

		char *modifiers;
		char *t;
		const char *names[MAX_MACRO_MOD];
		int i = 1, j;

		/* Free existing macro triggers and trigger template */
		macro_trigger_free();

		/* Clear template? */
		if (template[0] == '\0')
			return PARSE_ERROR_NONE;

		/* Tokenise last field... */
		modifiers = string_make(namelist);

		/* first token is name0 */
		names[0] = name0;

		t = strtok(modifiers, ":");
		while (t) {
			names[i++] = t;
			t = strtok(NULL, ":");
		}

		/* The number of modifiers must equal the number of names */
		if (strlen(chars) != (size_t) i)
		{
			string_free(modifiers);
			return (strlen(chars) > (size_t) i) ?
					PARSE_ERROR_TOO_FEW_ENTRIES : PARSE_ERROR_TOO_MANY_ENTRIES;
		}

		/* OK, now copy the data across */
		macro_template = string_make(template);
		macro_modifier_chr = string_make(chars);
		for (j = 0; j < i; j++)
			macro_modifier_name[j] = string_make(names[j]);

		string_free(modifiers);
	}

	/* Macro trigger */
	/* Process "T:<trigger>:<keycode>:<shift-keycode>" */
	else
	{
		const char *trigger = parser_getsym(p, "n1");
		const char *kc = parser_getsym(p, "n2");
		const char *shift_kc = NULL;

		char *buf;
		const char *s;
		char *t;

		if (parser_hasval(p, "n3"))
			shift_kc = parser_getsym(p, "n3");

		if (max_macrotrigger >= MAX_MACRO_TRIGGER)
			return PARSE_ERROR_TOO_MANY_ENTRIES;

		/* Buffer for the trigger name */
		buf = C_ZNEW(strlen(trigger) + 1, char);

		/* Simulate strcpy() and skip the '\' escape character */
		s = trigger;
		t = buf;

		while (*s)
		{
			if ('\\' == *s) s++;
			*t++ = *s++;
		}

		/* Terminate the trigger name */
		*t = '\0';

		/* Store the trigger name */
		macro_trigger_name[max_macrotrigger] = string_make(buf);

		/* Free the buffer */
		FREE(buf);

		/* Normal keycode */
		macro_trigger_keycode[0][max_macrotrigger] = string_make(kc);
		if (shift_kc)
			macro_trigger_keycode[1][max_macrotrigger] = string_make(shift_kc);
		else
			macro_trigger_keycode[1][max_macrotrigger] = string_make(kc);

		/* Count triggers */
		max_macrotrigger++;
	}

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
			op_ptr->window_flag[window] |= (1L << flag);
		else
			op_ptr->window_flag[window] &= ~(1L << flag);
	}

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


static struct parser *init_parse_prefs(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, mem_zalloc(sizeof(struct prefs_data)));
	parser_reg(p, "% str file", parse_prefs_load);
	parser_reg(p, "? str expr", parse_prefs_expr);
	parser_reg(p, "K sym tval sym sval int attr int char", parse_prefs_k);
	parser_reg(p, "R uint idx int attr int char", parse_prefs_r);
	parser_reg(p, "F uint idx int attr int char", parse_prefs_f);
	parser_reg(p, "S uint idx int attr int char", parse_prefs_s);
	parser_reg(p, "L uint idx int attr int char", parse_prefs_l);
	parser_reg(p, "E sym tval int attr", parse_prefs_e);
	parser_reg(p, "Q sym idx sym n ?sym sval ?sym flag", parse_prefs_q);
		/* XXX should be split into two kinds of line */
	parser_reg(p, "B uint idx str text", parse_prefs_b);
		/* XXX idx should be {tval,sval} pair! */
	parser_reg(p, "A str act", parse_prefs_a);
	parser_reg(p, "P str key", parse_prefs_p);
	parser_reg(p, "C int mode str key", parse_prefs_c);
	parser_reg(p, "T sym n1 sym n2 ?sym n3 ?str n4", parse_prefs_t);
		/* XXX should be two separate codes again */
	parser_reg(p, "M int type sym attr", parse_prefs_m);
	parser_reg(p, "V uint idx int k int r int g int b", parse_prefs_v);
	parser_reg(p, "W int window uint flag uint value", parse_prefs_w);
	parser_reg(p, "X str option", parse_prefs_x);
	parser_reg(p, "Y str option", parse_prefs_y);

	return p;
}


errr process_pref_file_command(const char *s)
{
	struct parser *p = init_parse_prefs();
	errr e = parser_parse(p, s);
	parser_destroy(p);
	return e;
}


static void print_error(const char *name, struct parser *p) {
	struct parser_state s;
	parser_getstate(p, &s);
	msg_format("Parse error in %s line %d column %d: %s: %s", name,
	           s.line, s.col, s.msg, parser_error_str[s.error]);
	message_flush();
}


/*
 * Process the user pref file with the given name.
 * "quiet" means "don't complain about not finding the file.
 *
 * Returns TRUE if everything worked OK, false otherwise
 */
bool process_pref_file(const char *name, bool quiet)
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
			msg_format("Cannot open '%s'.", buf);
	}
	else
	{
		char line[1024];

		p = init_parse_prefs();

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

		file_close(f);
		parser_destroy(p);
	}

	/* Result */
	return e == PARSE_ERROR_NONE;
}
