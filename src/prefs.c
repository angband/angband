/*
 * File: prefs.c
 * Purpose: Pref file handling code
 *
 * Copyright (c) 2003 Takeshi Mogami, Robert Ruehlmann
 * Copyright (c) 2007 Pete Mack
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


/*
 * Header and footer marker string for pref file dumps
 */
static cptr dump_separator = "#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#";


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
void autoinsc_dump(ang_file *fff)
{
	int i;
	if (!inscriptions) return;

	file_putf(fff, "# Autoinscription settings\n");
	file_putf(fff, "# B:item kind:inscription\n\n");

	for (i = 0; i < inscriptions_count; i++)
	{
		object_kind *k_ptr = &k_info[inscriptions[i].kind_idx];

		file_putf(fff, "# Autoinscription for %s\n", k_name + k_ptr->name);
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
	squelch_dump(fff);
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
		cptr act;

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

		file_putf(fff, "# Monster: %s\n", (r_name + r_ptr->name));
		file_putf(fff, "R:%d:0x%02X:0x%02X\n", i, attr, chr);
	}
}

/* Dump objects */
void dump_objects(ang_file *fff)
{
	int i;

	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		byte attr = k_ptr->x_attr;
		byte chr = k_ptr->x_char;

		/* Skip non-entries */
		if (!k_ptr->name) continue;

		file_putf(fff, "# Object: %s\n", (k_name + k_ptr->name));
		file_putf(fff, "K:%d:0x%02X:0x%02X\n", i, attr, chr);
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

		file_putf(fff, "# Terrain: %s\n", (f_name + f_ptr->name));
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

		file_putf(fff, "# Item flavor: %s\n", (flavor_text + x_ptr->text));
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

		cptr name = "unknown";

		/* Skip non-entries */
		if (!kv && !rv && !gv && !bv) continue;

		/* Extract the color name */
		if (i < BASIC_COLORS) name = color_names[i];

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

