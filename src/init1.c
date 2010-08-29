/*
 * File: init1.c
 * Purpose: Parsing the lib/edit/ files into data structures.
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
#include "object/tvalsval.h"


/*
 * This file is used to initialize various variables and arrays for the
 * Angband game.  Note the use of "fd_read()" and "fd_write()" to bypass
 * the common limitation of "read()" and "write()" to only 32767 bytes
 * at a time.
 *
 * Several of the arrays for Angband are built from "template" files in
 * the "lib/file" directory, from which quick-load binary "image" files
 * are constructed whenever they are not present in the "lib/data"
 * directory, or if those files become obsolete, if we are allowed.
 *
 * Warning -- the "ascii" file parsers use a minor hack to collect the
 * name and text information in a single pass.  Thus, the game will not
 * be able to load any template file with more than 20K of names or 60K
 * of text, even though technically, up to 64K should be legal.
 *
 * The code could actually be removed and placed into a "stand-alone"
 * program, but that feels a little silly, especially considering some
 * of the platforms that we currently support.
 */

#include "effects.h"
#include "monster/constants.h"
#include "init.h"


/*** Helper arrays for parsing ascii template files ***/

/* Use a slightly unusual include method to create effect_list[] */
static const char *effect_list[] =
{
	#define EFFECT(x, y, r, z)    #x,
	#include "list-effects.h"
	#undef EFFECT
};


/*
 * Monster blow methods
 */
static const char *r_info_blow_method[] =
{
	#define RBM(a, b) #a,
	#include "list-blow-methods.h"
	#undef RBM
	NULL
};


/*
 * Monster blow effects
 */
static const char *r_info_blow_effect[] =
{
	#define RBE(a, b) #a,
	#include "list-blow-effects.h"
	#undef RBE
	NULL
};


/*
 * Monster race flags
 */

static const char *r_info_flags[] =
{
	#define RF(a, b) #a,
	#include "list-mon-flags.h"
	#undef RF
	NULL
};


/*
 * Monster (race) spell flags
 */
static const char *r_info_spell_flags[] =
{
	#define RSF(a, b) #a,
	#include "list-mon-spells.h"
	#undef RSF
	NULL
};


/*
 * Object (kind) flags
 */

static const char *k_info_flags[] =
{
	#define OF(a, b) #a,
	#include "list-object-flags.h"
	#undef OF
	NULL
};

/*
 * Player race and class flags
 */

static const char *player_info_flags[] =
{
	#define PF(a, b) #a,
	#include "list-player-flags.h"
	#undef PF
	NULL
};

/*
 * Terrain feature flags
 */
static const char *f_info_flags[] =
{
	"PWALK",
	"PPASS",
	"MWALK",
	"MPASS",
	"LOOK",
	"DIG",
	"DOOR",
	"EXIT_UP",
	"EXIT_DOWN",
	"PERM",
	"TRAP",
	"SHOP",
	"HIDDEN",
	"BORING",
	NULL
};



/*** Initialize from ascii template files ***/


/*
 * Initialize an "*_info" array, by parsing an ascii "template" file
 */
errr init_info_txt(ang_file *fp, char *buf, header *head,
                   parse_info_txt_func parse_info_txt_line)
{
	errr err;

	/* Not ready yet */
	bool okay = FALSE;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = 0;


	/* Prepare the "fake" stuff */
	head->name_size = 0;
	head->text_size = 0;

	/* Parse */
	while (file_getl(fp, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (PARSE_ERROR_MISSING_COLON);


		/* Hack -- Process 'V' for "Version" */
		if (buf[0] == 'V')
		{
			int v1, v2, v3;

			/* Scan for the values */
			if ((3 != sscanf(buf+2, "%d.%d.%d", &v1, &v2, &v3)) ||
				(v1 != head->v_major) ||
				(v2 != head->v_minor) ||
				(v3 != head->v_patch))
			{
#if 0
				return (PARSE_ERROR_OBSOLETE_FILE);
#endif
			}

			/* Okay to proceed */
			okay = TRUE;

			/* Continue */
			continue;
		}

		/* No version yet */
		if (!okay) return (PARSE_ERROR_OBSOLETE_FILE);

		/* Parse the line */
		if ((err = (*parse_info_txt_line)(buf, head)) != 0)
			return (err);
	}


	/* Complete the "name" and "text" sizes */
	if (head->name_size) head->name_size++;
	if (head->text_size) head->text_size++;


	/* No version yet */
	if (!okay) return (PARSE_ERROR_OBSOLETE_FILE);


	/* Success */
	return (0);
}


/*
 * Add a text to the text-storage and store offset to it.
 *
 * Returns FALSE when there isn't enough space available to store
 * the text.
 */
static bool add_text(u32b *offset, header *head, cptr buf)
{
	size_t len = strlen(buf);

	/* Hack -- Verify space */
	if (head->text_size + len + 8 > z_info->fake_text_size)
		return (FALSE);

	/* New text? */
	if (*offset == 0)
	{
		/* Advance and save the text index */
		*offset = ++head->text_size;
	}

	/* Append chars to the text */
	my_strcpy(head->text_ptr + head->text_size, buf, len + 1);

	/* Advance the index */
	head->text_size += len;

	/* Success */
	return (TRUE);
}


/*
 * Add a name to the name-storage and return an offset to it.
 *
 * Returns 0 when there isn't enough space available to store
 * the name.
 */
static u32b add_name(header *head, cptr buf)
{
	u32b index;
	size_t len = strlen(buf);

	/* Hack -- Verify space */
	if (head->name_size + len + 8 > z_info->fake_name_size)
		return (0);

	/* Advance and save the name index */
	index = ++head->name_size;

	/* Append chars to the names */
	my_strcpy(head->name_ptr + head->name_size, buf, len + 1);

	/* Advance the index */
	head->name_size += len;
	
	/* Return the name index */
	return (index);
}


/*
 * Grab one flag from a textual string
 */
static errr grab_one_flag(u32b *flags, cptr names[], cptr what)
{
	int i;

	/* Check flags */
	for (i = 0; i < 32 && names[i]; i++)
	{
		if (streq(what, names[i]))
		{
			*flags |= (1L << i);
			return (0);
		}
	}

	return (-1);
}


/*
 * This function reads a value of the form "1+2d3M4"
 *
 * - The 1 is a constant base value.
 * - The 2d3 is a dice roll, to be applied with damroll().  The number of dice
 *   is optional, and if not present, is assumed to be 1.
 * - The M4 is a magic bonus, to be applied with m_bonus().
 *
 * All parts of the random value are optional.  The value is read into a
 * "random_value" struct and may be used with the randcalc() function to
 * determine the minimum, maximum, average, or randomised result of the roll.
 */
static bool parse_random_value(const char *str, random_value *bonus)
{
	bool negative = FALSE;

	char buffer[50];
	int i = 0, b, dn, ds, mb;
	
	const char end_chr = '|';
	char eov;

	/* Entire value may be negated */
	if (str[0] == '-')
	{
		negative = TRUE;
		i++;
	}

	/* Make a working copy of the string */
	my_strcpy(buffer, &str[i], N_ELEMENTS(buffer) - 2);

	/* Check for invalid negative numbers */
	if (NULL != strstr(buffer, "-"))	return FALSE;

	/*
	 * Add a sentinal value at the end of the string.
	 * Used by scanf to make sure there's no text after the final conversion.
	 */
	buffer[strlen(buffer) + 1] = '\0';
	buffer[strlen(buffer)] = end_chr;

	/* Scan the value, apply defaults for unspecified components */
	if (5 == sscanf(buffer, "%d+%dd%dM%d%c", &b, &dn, &ds, &mb, &eov) && eov == end_chr)
	{
		/* No defaults */
	}
	else if (4 == sscanf(buffer, "%d+d%dM%d%c", &b, &ds, &mb, &eov) && eov == end_chr)
	{
		dn = 1;
	}
	else if (3 == sscanf(buffer, "%d+M%d%c", &b, &mb, &eov) && eov == end_chr)
	{
		dn = 0; ds = 0;
	}
	else if (4 == sscanf(buffer, "%d+%dd%d%c", &b, &dn, &ds, &eov) && eov == end_chr)
	{
		mb = 0;
	}
	else if (3 == sscanf(buffer, "%d+d%d%c", &b, &ds, &eov) && eov == end_chr)
	{
		dn = 1; mb = 0;
	}
	else if (4 == sscanf(buffer, "%dd%dM%d%c", &dn, &ds, &mb, &eov) && eov == end_chr)
	{
		b = 0;
	}
	else if (3 == sscanf(buffer, "d%dM%d%c", &ds, &mb, &eov) && eov == end_chr)
	{
		b = 0; dn = 1;
	}
	else if (2 == sscanf(buffer, "M%d%c", &mb, &eov) && eov == end_chr)
	{
		b = 0; dn = 0; ds = 0;
	}
	else if (3 == sscanf(buffer, "%dd%d%c", &dn, &ds, &eov) && eov == end_chr)
	{
		b = 0; mb = 0;
	}
	else if (2 == sscanf(buffer, "d%d%c", &ds, &eov) && eov == end_chr)
	{
		b = 0; dn = 1; mb = 0;
	}
	else if (2 == sscanf(buffer, "%d%c", &b, &eov) && eov == end_chr)
	{
		dn = 0; ds = 0; mb = 0;
	}
	else
	{
		return FALSE;
	}

	/* Assign the values */
	bonus->base = b;
	bonus->dice = dn;
	bonus->sides = ds;
	bonus->m_bonus = mb;

	/*
	 * Handle negation (the random components are always positive, so the base
	 * must be adjusted as necessary).
	 */
	if (negative)
	{
		bonus->base *= -1;
		bonus->base -= bonus->m_bonus;
		bonus->base -= bonus->dice * (bonus->sides + 1);
	}

	return TRUE;
}


/*
 * Figure out what index an activation should have
 */
static u32b grab_one_effect(const char *what)
{
	size_t i;

	/* Scan activations */
	for (i = 0; i < N_ELEMENTS(effect_list); i++)
	{
		if (streq(what, effect_list[i]))
			return i;
	}

	/* Oops */
	msg_format("Unknown effect '%s'.", what);

	/* Error */
	return 0;
}


/**
 * Initialise the store stocking lists.
 */
errr init_store_txt(ang_file *fp, char *buf)
{
	int i;

	int store_num = -1;
	store_type *st_ptr;

	error_idx = -1;
	error_line = 0;


	/* Allocate the stores */
	store = C_ZNEW(MAX_STORES, store_type);
	for (i = 0; i < MAX_STORES; i++)
	{
		st_ptr = &store[i];

		/* Assume stock */
		st_ptr->stock_size = STORE_INVEN_MAX;
		st_ptr->stock = C_ZNEW(st_ptr->stock_size, object_type);
	}

	st_ptr = NULL;


	while (file_getl(fp, buf, 1024))
	{
		error_line++;

		if (!buf[0] || buf[0] == '#')
			continue;

		else if (buf[0] == 'S')
		{
			int num, slots;

			/* Make sure all the previous slots have been filled */
			if (st_ptr)
			{
				if (st_ptr->table_num != st_ptr->table_size)
				{
					msg_format("Store %d has too few entries (read %d, expected %d).", error_idx, st_ptr->table_num, st_ptr->table_size);
					return PARSE_ERROR_TOO_FEW_ENTRIES;
				}
			}


			if (2 != sscanf(buf, "S:%d:%d", &num, &slots))
				return PARSE_ERROR_GENERIC;

			if (num < 2 || num > 6)
				return PARSE_ERROR_GENERIC;

			error_idx = num;

			/* Account for 0-based indexing */
			num--;
			store_num = num;

			/* Set up this store */
			st_ptr = &store[num];
			st_ptr->table_size = slots;
			st_ptr->table = C_ZNEW(st_ptr->table_size, s16b);
		}

		else if (buf[0] == 'I')
		{
			int slots, tval;
			int k_idx;

			char *tval_s;
			char *sval_s;

			if (store_num == -1 || !st_ptr)
				return PARSE_ERROR_GENERIC;

			if (1 != sscanf(buf, "I:%d:", &slots))
				return PARSE_ERROR_GENERIC;

			if (st_ptr->table_num + slots > st_ptr->table_size)
				return PARSE_ERROR_TOO_MANY_ENTRIES;

			/* Find the beginning of the tval field */
			tval_s = strchr(buf+2, ':');
			if (!tval_s) return PARSE_ERROR_MISSING_COLON;
			*tval_s++ = '\0';
			if (!*tval_s) return PARSE_ERROR_MISSING_FIELD;

			/* Now find the beginning of the sval field */
			sval_s = strchr(tval_s, ':');
			if (!sval_s) return PARSE_ERROR_MISSING_COLON;
			*sval_s++ = '\0';
			if (!*sval_s) return PARSE_ERROR_MISSING_FIELD;

			/* Now convert the tval into its numeric equivalent */
			tval = tval_find_idx(tval_s);
			if (tval == -1) return PARSE_ERROR_UNRECOGNISED_TVAL;

			k_idx = lookup_name(tval, sval_s);
			if (!k_idx) return PARSE_ERROR_UNRECOGNISED_SVAL;

			while (slots--)
				st_ptr->table[st_ptr->table_num++] = k_idx;
		}

		else
		{
			return PARSE_ERROR_UNDEFINED_DIRECTIVE;
		}
	}

	/* No errors */
	return 0;
}


/**
 * Initialise the random name fragments
 */
errr init_names_txt(ang_file *fp, char *buf)
{
	int i, name_section;
	u32b num_names[RANDNAME_NUM_TYPES];
	char temp[1024];
	int counter = 0;
	error_line = 0;
	memset(num_names, 0, RANDNAME_NUM_TYPES * sizeof(u32b));

	/* 
	 * Go through the file and count the total number of names in each name 
	 * section
	 */
	while (file_getl(fp, buf, 1024))
	{
		error_line++;

		if (!buf[0] || '#' == buf[0])
			continue;

		else if ('N' == buf[0])
		{
			if (1 != sscanf(buf, "N:%d", &name_section))
				return PARSE_ERROR_GENERIC;

			if (name_section > RANDNAME_NUM_TYPES)
				return PARSE_ERROR_GENERIC;
		}

		else if ('D' == buf[0])
		{
			num_names[name_section]++;
		}

		else
		{
			return PARSE_ERROR_UNDEFINED_DIRECTIVE;
		}
	}

	/* Go back to the start of the file */
	file_seek(fp, 0);

	/* Allocate some memory for pointers to sections */
	name_sections = C_ZNEW(RANDNAME_NUM_TYPES, cptr*);

	/* Allocate more memory for pointers to names */
	/* Add a null element at the end of each array (see randname.c) */
	for (i = 0; i < RANDNAME_NUM_TYPES; i++)
	{
		if (num_names[i])
		{
			name_sections[i] = C_ZNEW(num_names[i] + 1, cptr);
		}
	} 
	
	/* 
	 * Go through the file again and read each name into the relevant array 
	 */
	while (file_getl(fp, buf, 1024))
	{
		error_line++;

		if (!buf[0] || '#' == buf[0])
			continue;

		else if ('N' == buf[0])
		{
			if (1 != sscanf(buf, "N:%d", &name_section))
				return PARSE_ERROR_GENERIC;

			if (name_section > RANDNAME_NUM_TYPES)
				return PARSE_ERROR_GENERIC;

			counter = 0;
		}

		else if ('D' == buf[0])
		{
			if (1 != sscanf(buf, "D:%s", temp))
				return PARSE_ERROR_GENERIC;

			name_sections[name_section][counter++] = string_make(temp);
		}

		else
		{
			return PARSE_ERROR_UNDEFINED_DIRECTIVE;
		}
	}

	/* No errors */
	return 0;
}


/*
 * Initialize the "z_info" structure, by parsing an ascii "template" file
 */
errr parse_z_info(char *buf, header *head)
{
	maxima *z_info;
	int max;

	if (!buf || !head)
		return PARSE_ERROR_INTERNAL;
	
	z_info = head->info_ptr;

	if (buf[0] != 'M')
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;
	if (!buf[2] || !buf[4])
		return PARSE_ERROR_MISSING_FIELD;
	if (buf[1] != ':' || buf[3] != ':')
		return PARSE_ERROR_MISSING_COLON;

	if (1 != sscanf(buf + 4, "%d", &max))
		return PARSE_ERROR_NOT_NUMBER;
	if (max < 0)
		return PARSE_ERROR_INVALID_VALUE;

	if (buf[2] == 'F')
		z_info->f_max = max;
	else if (buf[2] == 'K')
		z_info->k_max = max;
	else if (buf[2] == 'A')
		z_info->a_max = max;
	else if (buf[2] == 'E')
		z_info->e_max = max;
	else if (buf[2] == 'R')
		z_info->r_max = max;
	else if (buf[2] == 'V')
		z_info->v_max = max;
	else if (buf[2] == 'P')
		z_info->p_max = max;
	else if (buf[2] == 'C')
		z_info->c_max = max;
	else if (buf[2] == 'H')
		z_info->h_max = max;
	else if (buf[2] == 'B')
		z_info->b_max = max;
	else if (buf[2] == 'S')
		z_info->s_max = max;
	else if (buf[2] == 'O')
		z_info->o_max = max;
	else if (buf[2] == 'M')
		z_info->m_max = max;
	else if (buf[2] == 'L')
		z_info->flavor_max = max;
	else if (buf[2] == 'N')
		z_info->fake_name_size = max;
	else if (buf[2] == 'T')
		z_info->fake_text_size = max;
	else
		return PARSE_ERROR_UNDEFINED_DIRECTIVE;

	return 0;
}


/*
 * Initialize the "v_info" array, by parsing an ascii "template" file
 */
errr parse_v_info(char *buf, header *head)
{
	int i;

	char *s;

	/* Current entry */
	static vault_type *v_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		v_ptr = (vault_type*)head->info_ptr + i;

		/* Store the name */
		if ((v_ptr->name = add_name(head, s)) == 0)
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		/* There better be a current v_ptr */
		if (!v_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Get the text */
		s = buf+2;

		/* Store the text */
		if (!add_text(&v_ptr->text, head, s))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	/* Process 'X' for "Extra info" (one line only) */
	else if (buf[0] == 'X')
	{
		int typ, rat, hgt, wid;

		/* There better be a current v_ptr */
		if (!v_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%d",
			            &typ, &rat, &hgt, &wid)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		v_ptr->typ = typ;
		v_ptr->rat = rat;
		v_ptr->hgt = hgt;
		v_ptr->wid = wid;

		/* Check for maximum vault sizes */
		if ((v_ptr->typ == 6) && ((v_ptr->wid > 33) || (v_ptr->hgt > 22)))
			return (PARSE_ERROR_VAULT_TOO_BIG);

		if ((v_ptr->typ == 7) && ((v_ptr->wid > 66) || (v_ptr->hgt > 44)))
			return (PARSE_ERROR_VAULT_TOO_BIG);
	}
	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}



/*
 * Initialize the "f_info" array, by parsing an ascii "template" file
 */
errr parse_f_info(char *buf, header *head)
{
	int i;

	char *s;
	char *t;

	/* Current entry */
	static feature_type *f_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		f_ptr = (feature_type*)head->info_ptr + i;

		/* Store the name */
		if ((f_ptr->name = add_name(head, s)) == 0)
			return (PARSE_ERROR_OUT_OF_MEMORY);

		/* Default "mimic" */
		f_ptr->mimic = i;
	}

	/* Process 'M' for "Mimic" (one line only) */
	else if (buf[0] == 'M')
	{
		int mimic;

		/* There better be a current f_ptr */
		if (!f_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (1 != sscanf(buf+2, "%d", &mimic))
			return (PARSE_ERROR_NOT_NUMBER);

		/* Save the values */
		f_ptr->mimic = mimic;
	}

	/* Process 'P' for "Priority" */
	else if (buf[0] == 'P')
	{
		int priority;

		/* There better be a current f_ptr */
		if (!f_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (1 != sscanf(buf+2, "%d", &priority))
			return (PARSE_ERROR_NOT_NUMBER);

		/* Save the values */
		f_ptr->priority = priority;
	}

	/* Process 'G' for "Graphics" (one line only) */
	else if (buf[0] == 'G')
	{
		char d_char;
		int d_attr;

		/* There better be a current f_ptr */
		if (!f_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Paranoia */
		if (!buf[2]) return (PARSE_ERROR_GENERIC);
		if (!buf[3]) return (PARSE_ERROR_GENERIC);
		if (!buf[4]) return (PARSE_ERROR_GENERIC);

		/* Extract d_char */
		d_char = buf[2];

		/* If we have a longer string than expected ... */
		if (buf[5])
		{
			/* Advance "buf" on by 4 */
			buf += 4;

			/* Extract the colour */
			d_attr = color_text_to_attr(buf);
		}
		else
		{
			/* Extract the attr */
			d_attr = color_char_to_attr(buf[4]);
		}

		/* Paranoia */
		if (d_attr < 0) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		f_ptr->d_attr = d_attr;
		f_ptr->d_char = d_char;
	}

	/* Process 'F' for flags */
	else if (buf[0] == 'F')
	{
		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Parse this entry */
			if (0 != grab_one_flag(&f_ptr->flags, f_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}

	/* Process 'E' for effect */
	else if (buf[0] == 'E')
	{
		f_ptr->effect = grab_one_effect(buf + 2);
		if (!f_ptr->effect)
			return PARSE_ERROR_GENERIC;
	}

	/* Process 'X' for extra */
	else if (buf[0] == 'X')
	{
		int locked, jammed, shopnum, dig;

		if (4 != sscanf(buf + 2, "%d:%d:%d:%d",
				&locked, &jammed, &shopnum, &dig))
			return PARSE_ERROR_NOT_NUMBER;

		f_ptr->locked = locked;
		f_ptr->jammed = jammed;
		f_ptr->shopnum = shopnum;
		f_ptr->dig = dig;
	}

	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}


/*
 * Find a flag's index from a textual string.
 */
static int lookup_flag(const char **flag_table, const char *flag_name)
{
	int i = FLAG_START;

	while (flag_table[i] && !streq(flag_table[i], flag_name))
		i++;

	/* End of table reached without match */
	if (!flag_table[i]) i = FLAG_END;

	return i;
}


/*
 * Grab one flag from a string, and set that flag in a bit field
 */
static errr grab_flag(bitflag *flags, const size_t size, const char **flag_table, const char *flag_name)
{
	int flag = lookup_flag(flag_table, flag_name);

	if (flag == FLAG_END) return PARSE_ERROR_INVALID_FLAG;

	flag_on(flags, size, flag);

	return 0;
}



/*
 * Initialize the "k_info" array, by parsing an ascii "template" file
 */
errr parse_k_info(char *buf, header *head)
{
	int i;

	char *s = NULL, *t;

	/* Current entry */
	static object_kind *k_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		k_ptr = (object_kind*)head->info_ptr + i;
		k_ptr->kidx = i;
		k_ptr->name = string_make(s);

		/* Success (return early) */
		return (0);
	}

	/* There better be a current k_ptr */
	if (!k_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

	/* Process 'G' for "Graphics" (one line only) */
	if (buf[0] == 'G')
	{
		char d_char;
		int d_attr;

		/* Paranoia */
		if (!buf[2]) return (PARSE_ERROR_GENERIC);
		if (!buf[3]) return (PARSE_ERROR_GENERIC);
		if (!buf[4]) return (PARSE_ERROR_GENERIC);

		/* Extract d_char */
		d_char = buf[2];

		/* If we have a longer string than expected ... */
		if (buf[5])
		{
			/* Advance "buf" on by 4 */
			buf += 4;

			/* Extract the colour */
			d_attr = color_text_to_attr(buf);
		}
		else
		{
			/* Extract the attr */
			d_attr = color_char_to_attr(buf[4]);
		}

		/* Paranoia */
		if (d_attr < 0) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		k_ptr->d_attr = d_attr;
		k_ptr->d_char = d_char;
	}

	/* Process 'I' for "Info" (one line only) */
	else if (buf[0] == 'I')
	{
		int tval, sval;
		char pval[50];

		/* Scan for the values */
		if (3 != sscanf(buf+2, "%d:%d:%s",
			            &tval, &sval, pval)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		k_ptr->tval = tval;
		k_ptr->sval = sval;
		if (!parse_random_value(pval, &k_ptr->pval)) return PARSE_ERROR_INVALID_VALUE;
	}

	/* Process 'W' for "More Info" (one line only) */
	else if (buf[0] == 'W')
	{
		int level, extra, wgt;
		long cost;

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
			            &level, &extra, &wgt, &cost)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		k_ptr->level = level;
		k_ptr->weight = wgt;
		k_ptr->cost = cost;
	}

	/* Process 'A' for "Allocation" (one line only) */
	else if (buf[0] == 'A')
	{
		int common, min, max;

		/* Format is "A:<common>:<min> to <max>" */
		if (3 != sscanf(buf+2, "%d:%d to %d", &common, &min, &max))
			return (PARSE_ERROR_GENERIC);


		/* Limit to size a byte */
		if (common < 0 || common > 255) return (PARSE_ERROR_GENERIC);
		if (min < 0 || min > 255) return (PARSE_ERROR_GENERIC);
		if (max < 0 || max > 255) return (PARSE_ERROR_GENERIC);


		/* Set up data */
		k_ptr->alloc_prob = common;
		k_ptr->alloc_min = min;
		k_ptr->alloc_max = max;
	}

	/* Hack -- Process 'P' for "power" and such */
	else if (buf[0] == 'P')
	{
		int ac, hd1, hd2;
		char *th, *td, *ta;
		char fields[150];

		/* Scan for the values "P:0:0d0:0:0:0" */
		if (4 != sscanf(buf+2, "%d:%dd%d:%s",
			            &ac, &hd1, &hd2, fields)) return (PARSE_ERROR_GENERIC);

		/* Tokenize the last three fields */
		if(!(th = strtok(fields, ":"))) return PARSE_ERROR_GENERIC;
		if(!(td = strtok(NULL, ":"))) return PARSE_ERROR_GENERIC;
		if(!(ta = strtok(NULL, ":"))) return PARSE_ERROR_GENERIC;

		k_ptr->ac = ac;
		k_ptr->dd = hd1;
		k_ptr->ds = hd2;
		if (!parse_random_value(th, &k_ptr->to_h)) return PARSE_ERROR_INVALID_VALUE;
		if (!parse_random_value(td, &k_ptr->to_d)) return PARSE_ERROR_INVALID_VALUE;
		if (!parse_random_value(ta, &k_ptr->to_a)) return PARSE_ERROR_INVALID_VALUE;
	}

	/* Hack -- Process 'C' for "charges" */
	else if (buf[0] == 'C')
	{
		if(!parse_random_value(buf+2, &k_ptr->charge)) return PARSE_ERROR_INVALID_VALUE;
	}

	/* Process 'M' for "Multiple quantity" (one line only) */
	else if (buf[0] == 'M')
	{
		int prob;
		char stack[50];

		/* Scan for the values */
		if (2 != sscanf(buf+2, "%d:%s", &prob, stack))
			return (PARSE_ERROR_GENERIC);

		/* Save the values */
		k_ptr->gen_mult_prob = prob;
		if (!parse_random_value(stack, &k_ptr->stack_size)) return PARSE_ERROR_INVALID_VALUE;
	}

	/* Hack -- Process 'F' for flags */
	else if (buf[0] == 'F')
	{
		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while (*t == ' ' || *t == '|') t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(k_ptr->flags, OF_SIZE, k_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}

	/* Process 'E' for effect */
	else if (buf[0] == 'E')
	{
		/* Find the colon after the name, nuke it and advance */
		s = strchr(buf + 2, ':');
		if (s) *s++ = '\0';

		/* Get the activation */
		k_ptr->effect = grab_one_effect(buf + 2);
		if (!k_ptr->effect) return (PARSE_ERROR_GENERIC);

		/* Get the timeout, if supplied */
		if (s && !parse_random_value(s, &k_ptr->time))
			return (PARSE_ERROR_INVALID_VALUE);
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		k_ptr->text = string_append(k_ptr->text, buf + 2);
	}

	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}



/*
 * Initialize the "a_info" array, by parsing an ascii "template" file
 */
errr parse_a_info(char *buf, header *head)
{
	int i;

	char *s, *t;

	/* Current entry */
	static artifact_type *a_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		a_ptr = (artifact_type*)head->info_ptr + i;

		/* Store the name */
		if ((a_ptr->name = add_name(head, s)) == 0)
			return (PARSE_ERROR_OUT_OF_MEMORY);

		/* Ignore everything */
		flags_set(a_ptr->flags, OF_SIZE, OF_IGNORE_MASK, FLAG_END);

		/* Return early */
		return (0);
	}

	/* There better be a current a_ptr */
	if (!a_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

	/* Process 'I' for "Info" (one line only) */
	else if (buf[0] == 'I')
	{
		char *tval_s, *sval_s, *pval_s;
		int tval, sval, pval;

		/* Find the beginning of the tval field */
		tval_s = strchr(buf, ':');
		if (!tval_s) return PARSE_ERROR_MISSING_COLON;
		*tval_s++ = '\0';
		if (!*tval_s) return PARSE_ERROR_MISSING_FIELD;

		/* Now find the beginning of the sval field */
		sval_s = strchr(tval_s, ':');
		if (!sval_s) return PARSE_ERROR_MISSING_COLON;
		*sval_s++ = '\0';
		if (!*sval_s) return PARSE_ERROR_MISSING_FIELD;

		/* Now find the beginning of the pval field */
		pval_s = strchr(sval_s, ':');
		if (!pval_s) return PARSE_ERROR_MISSING_COLON;
		*pval_s++ = '\0';
		if (!*pval_s) return PARSE_ERROR_MISSING_FIELD;

		/* Now convert the tval into its numeric equivalent */
		if (1 != sscanf(tval_s, "%d", &tval))
		{
			tval = tval_find_idx(tval_s);
			if (tval == -1) return PARSE_ERROR_UNRECOGNISED_TVAL;
		}

		/* Now find the sval */
		if (1 != sscanf(sval_s, "%d", &sval))
		{
			sval = lookup_sval(tval, sval_s);
			if (sval == -1) return PARSE_ERROR_UNRECOGNISED_SVAL;
		}

		/* Now extract the pval */
		if (1 != sscanf(pval_s, "%d", &pval))
			return PARSE_ERROR_NOT_NUMBER;

		/* Save the values */
		a_ptr->tval = tval;
		a_ptr->sval = sval;
		a_ptr->pval = pval;
	}

	/* Process 'W' for "More Info" (one line only) */
	else if (buf[0] == 'W')
	{
		int level, rarity, wgt;
		long cost;

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
			            &level, &rarity, &wgt, &cost)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		a_ptr->level = level;
		a_ptr->rarity = rarity;
		a_ptr->weight = wgt;
		a_ptr->cost = cost;
	}

	/* Process 'A' for "Allocation" (one line only) */
	else if (buf[0] == 'A')
	{
		int common, min, max;

		/* Format is "A:<common>:<min> to <max>" */
		if (3 != sscanf(buf+2, "%d:%d to %d", &common, &min, &max))
			return (PARSE_ERROR_GENERIC);


		/* Limit to size a byte */
		if (common < 0 || common > 255) return (PARSE_ERROR_GENERIC);
		if (min < 0 || min > 255) return (PARSE_ERROR_GENERIC);
		if (max < 0 || max > 255) return (PARSE_ERROR_GENERIC);


		/* Set up data */
		a_ptr->alloc_prob = common;
		a_ptr->alloc_min = min;
		a_ptr->alloc_max = max;
	}

	/* Process 'P' for "power" and such */
	else if (buf[0] == 'P')
	{
		int ac, hd1, hd2, th, td, ta;

		/* Scan for the values */
		if (6 != sscanf(buf+2, "%d:%dd%d:%d:%d:%d",
			            &ac, &hd1, &hd2, &th, &td, &ta)) return (PARSE_ERROR_GENERIC);

		a_ptr->ac = ac;
		a_ptr->dd = hd1;
		a_ptr->ds = hd2;
		a_ptr->to_h = th;
		a_ptr->to_d = td;
		a_ptr->to_a = ta;
	}

	/* Process 'F' for flags */
	else if (buf[0] == 'F')
	{
		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(a_ptr->flags, OF_SIZE, k_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}

	/* Process 'E' for Effect (Activation) & time */
	else if (buf[0] == 'E')
	{
		/* Find the colon after the name */
		s = strchr(buf + 2, ':');
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the activation */
		a_ptr->effect = grab_one_effect(buf + 2);
		if (!a_ptr->effect) return (PARSE_ERROR_GENERIC);

		/* Scan for the values */
		if (!parse_random_value(s, &a_ptr->time))
			return (PARSE_ERROR_INVALID_VALUE);
	}

	/* Process 'M' for "Effect message" */
	else if (buf[0] == 'M')
	{
		/* Store the text */
		if (!add_text(&a_ptr->effect_msg, head, buf+2))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		/* Store the text */
		if (!add_text(&a_ptr->text, head, buf+2))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}



/*
 * Initialize the "e_info" array, by parsing an ascii "template" file
 */
errr parse_e_info(char *buf, header *head)
{
	int i;

	char *s, *t;

	/* Current entry */
	static ego_item_type *e_ptr = NULL;

	static int cur_t = 0;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		e_ptr = (ego_item_type*)head->info_ptr + i;

		/* Store the name */
		if ((e_ptr->name = add_name(head, s)) == 0)
			return (PARSE_ERROR_OUT_OF_MEMORY);

		/* Start with the first of the tval indices */
		cur_t = 0;
	}

	/* Process 'W' for "More Info" (one line only) */
	else if (buf[0] == 'W')
	{
		int level, rarity, pad2;
		long cost;

		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
			            &level, &rarity, &pad2, &cost)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		e_ptr->level = level;
		e_ptr->rarity = rarity;
		/* e_ptr->weight = wgt; */
		e_ptr->cost = cost;
	}

	/* Process 'X' for "Xtra" (one line only) */
	else if (buf[0] == 'X')
	{
		int rating, xtra;

		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (2 != sscanf(buf+2, "%d:%d", &rating, &xtra))
			return (PARSE_ERROR_GENERIC);

		/* Save the values */
		e_ptr->rating = rating;
		e_ptr->xtra = xtra;
	}

	/* Process 'T' for "Types allowed" (up to three lines) */
	else if (buf[0] == 'T')
	{
		int tval, sval1, sval2;

		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (3 != sscanf(buf+2, "%d:%d:%d",
			            &tval, &sval1, &sval2)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		e_ptr->tval[cur_t] = (byte)tval;
		e_ptr->min_sval[cur_t] = (byte)sval1;
		e_ptr->max_sval[cur_t] = (byte)sval2;

		/* Increase counter for 'possible tval' index */
		cur_t++;

		/* Allow only a limited number of T: lines */
		if (cur_t > EGO_TVALS_MAX) return (PARSE_ERROR_GENERIC);
	}

	/* Hack -- Process 'C' for "creation" */
	else if (buf[0] == 'C')
	{
		char *th, *td, *ta, *pv;
		char fields[200];

		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (1 != sscanf(buf+2, "%s", fields)) return (PARSE_ERROR_GENERIC);

		if(!(th = strtok(fields, ":"))) return PARSE_ERROR_GENERIC;
		if(!(td = strtok(NULL, ":"))) return PARSE_ERROR_GENERIC;
		if(!(ta = strtok(NULL, ":"))) return PARSE_ERROR_GENERIC;
		if(!(pv = strtok(NULL, ":"))) return PARSE_ERROR_GENERIC;
		
		if(!parse_random_value(th, &e_ptr->to_h)) return PARSE_ERROR_INVALID_VALUE;
		if(!parse_random_value(td, &e_ptr->to_d)) return PARSE_ERROR_INVALID_VALUE;
		if(!parse_random_value(ta, &e_ptr->to_a)) return PARSE_ERROR_INVALID_VALUE;
		if(!parse_random_value(pv, &e_ptr->pval)) return PARSE_ERROR_INVALID_VALUE;
	}

	/* Process 'M' for "minimum values" */
	else if (buf[0] == 'M')
	{
		int th, td, ta, pv;

		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%d",
			            &th, &td, &ta, &pv)) return (PARSE_ERROR_GENERIC);

		e_ptr->min_to_h = th;
		e_ptr->min_to_d = td;
		e_ptr->min_to_a = ta;
		e_ptr->min_pval = pv;
	}

	/* Hack -- Process 'F' for flags */
	else if (buf[0] == 'F')
	{
		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(e_ptr->flags, OF_SIZE, k_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		/* There better be a current e_ptr */
		if (!e_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Get the text */
		s = buf+2;

		/* Store the text */
		if (!add_text(&e_ptr->text, head, s))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}



/*
 * Initialize the "r_info" array, by parsing an ascii "template" file
 */
errr parse_r_info(char *buf, header *head)
{
	int i;

	char *s, *t;

	/* Current entry */
	static monster_race *r_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		r_ptr = (monster_race*)head->info_ptr + i;

		/* Store the name */
		if ((r_ptr->name = add_name(head, s)) == 0)
			return (PARSE_ERROR_OUT_OF_MEMORY);

		return 0;
	}

	/* There better be a current r_ptr */
	if (!r_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

	/* Process 'D' for "Description" */
	if (buf[0] == 'D')
	{
		/* Store the text */
		if (!add_text(&(r_ptr->text), head, buf+2))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	/* Process 'G' for "Graphics" (one line only) */
	else if (buf[0] == 'G')
	{
		char d_char;
		int d_attr;

		/* Paranoia */
		if (!buf[2]) return (PARSE_ERROR_GENERIC);
		if (!buf[3]) return (PARSE_ERROR_GENERIC);
		if (!buf[4]) return (PARSE_ERROR_GENERIC);

		/* Extract d_char */
		d_char = buf[2];

		/* If we have a longer string than expected ... */
		if (buf[5])
		{
			/* Advance "buf" on by 4 */
			buf += 4;

			/* Extract the colour */
			d_attr = color_text_to_attr(buf);
		}
		else
		{
			/* Extract the attr */
			d_attr = color_char_to_attr(buf[4]);
		}

		/* Paranoia */
		if (d_attr < 0) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		r_ptr->d_attr = d_attr;
		r_ptr->d_char = d_char;
	}

	/* Process 'I' for "Info" (one line only) */
	else if (buf[0] == 'I')
	{
		int spd, lif, aaf, ac, slp;

		/* Scan for the other values */
		if (5 != sscanf(buf+2, "%d:%d:%d:%d:%d",
			            &spd, &lif, &aaf, &ac, &slp)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		r_ptr->speed = spd;
		r_ptr->avg_hp = lif;
		r_ptr->aaf = aaf;
		r_ptr->ac = ac;
		r_ptr->sleep = slp;
	}

	/* Process 'W' for "More Info" (one line only) */
	else if (buf[0] == 'W')
	{
		int lev, rar, pad;
		long exp;

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%ld",
			            &lev, &rar, &pad, &exp)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		r_ptr->level = lev;
		r_ptr->rarity = rar;
		r_ptr->power = pad;
		r_ptr->mexp = exp;
	}

	/* Process 'B' for "Blows" */
	else if (buf[0] == 'B')
	{
		int n1, n2;

		/* Find the next empty blow slot (if any) */
		for (i = 0; i < MONSTER_BLOW_MAX; i++) if (!r_ptr->blow[i].method) break;

		/* Oops, no more slots */
		if (i == MONSTER_BLOW_MAX)
			return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Analyze the first field */
		for (s = t = buf+2; *t && (*t != ':'); t++) /* loop */;

		/* Terminate the field (if necessary) */
		if (*t == ':') *t++ = '\0';

		/* Analyze the method */
		for (n1 = 0; r_info_blow_method[n1]; n1++)
		{
			if (streq(s, r_info_blow_method[n1])) break;
		}

		/* Invalid method */
		if (!r_info_blow_method[n1]) return (PARSE_ERROR_UNRECOGNISED_BLOW);

		/* Analyze the second field */
		for (s = t; *t && (*t != ':'); t++) /* loop */;

		/* Terminate the field (if necessary) */
		if (*t == ':') *t++ = '\0';

		/* Analyze effect */
		for (n2 = 0; r_info_blow_effect[n2]; n2++)
		{
			if (streq(s, "")) break;
			if (streq(s, r_info_blow_effect[n2])) break;
		}

		/* Invalid effect */
		if (!r_info_blow_effect[n2]) return (PARSE_ERROR_UNRECOGNISED_BLOW);

		/* Analyze the third field */
		for (s = t; *t && (*t != 'd'); t++) /* loop */;

		/* Terminate the field (if necessary) */
		if (*t == 'd') *t++ = '\0';

		/* Save the method */
		r_ptr->blow[i].method = n1;

		/* Save the effect */
		r_ptr->blow[i].effect = n2;

		/* Extract the damage dice and sides */
		r_ptr->blow[i].d_dice = atoi(s);
		r_ptr->blow[i].d_side = atoi(t);
	}

	/* Process 'F' for "Basic Flags" (multiple lines) */
	else if (buf[0] == 'F')
	{
		/* Parse every entry */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while (*t == ' ' || *t == '|') t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(r_ptr->flags, RF_SIZE, r_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}

	/* Process 'S' for "Spell Flags" (multiple lines) */
	else if (buf[0] == 'S')
	{
		/* Parse every entry */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* XXX Hack -- Read spell frequency */
			if (1 == sscanf(s, "1_IN_%d", &i))
			{
				/* Sanity check */
				if ((i < 1) || (i > 100))
					return (PARSE_ERROR_INVALID_SPELL_FREQ);

				/* Extract a "frequency" */
				r_ptr->freq_spell = r_ptr->freq_innate = 100 / i;

				/* Start at next entry */
				s = t;

				/* Continue */
				continue;
			}

			/* Parse this entry */
			if (0 != grab_flag(r_ptr->spell_flags, RSF_SIZE, r_info_spell_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}
	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}


	/* Success */
	return (0);
}


/*
 * Helper function for reading a list of skills
 */
static bool parse_skills(s16b *skills_array, const char *buf)
{
	int dis, dev, sav, stl, srh, fos, thn, thb, throw, dig;

	/* Scan for the values */
	if (SKILL_MAX != sscanf(buf, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
			&dis, &dev, &sav, &stl, &srh,
			&fos, &thn, &thb, &throw, &dig))
		return FALSE;

	/* Save the values */
	skills_array[SKILL_DISARM] = dis;
	skills_array[SKILL_DEVICE] = dev;
	skills_array[SKILL_SAVE] = sav;
	skills_array[SKILL_STEALTH] = stl;
	skills_array[SKILL_SEARCH] = srh;
	skills_array[SKILL_SEARCH_FREQUENCY] = fos;
	skills_array[SKILL_TO_HIT_MELEE] = thn;
	skills_array[SKILL_TO_HIT_BOW] = thb;
	skills_array[SKILL_TO_HIT_THROW] = throw;
	skills_array[SKILL_DIGGING] = dig;

	return TRUE;
}


/*
 * Initialize the "p_info" array, by parsing an ascii "template" file
 */
errr parse_p_info(char *buf, header *head)
{
	int i, j;

	char *s, *t;

	/* Current entry */
	static player_race *pr_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_MISSING_FIELD);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		pr_ptr = (player_race*)head->info_ptr + i;

		/* Store the name */
		pr_ptr->name = string_make(s);
	}

	/* Process 'S' for "Stats" (one line only) */
	else if (buf[0] == 'S')
	{
		int adj;

		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Start the string */
		s = buf+1;

		/* For each stat */
		for (j = 0; j < A_MAX; j++)
		{
			/* Find the colon before the subindex */
			s = strchr(s, ':');

			/* Verify that colon */
			if (!s) return (PARSE_ERROR_MISSING_COLON);

			/* Nuke the colon, advance to the subindex */
			*s++ = '\0';

			/* Get the value */
			adj = atoi(s);

			/* Save the value */
			pr_ptr->r_adj[j] = adj;

			/* Next... */
			continue;
		}
	}

	/* Process 'R' for "Racial Skills" (one line only) */
	else if (buf[0] == 'R')
	{
		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Verify text */
		if (!buf[1] || !buf[2]) return (PARSE_ERROR_MISSING_FIELD);

		/* Scan and save the values */
		if (!parse_skills(pr_ptr->r_skills, buf+2)) return (PARSE_ERROR_GENERIC);
	}

	/* Process 'X' for "Extra Info" (one line only) */
	else if (buf[0] == 'X')
	{
		int mhp, exp, infra;

		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (3 != sscanf(buf+2, "%d:%d:%d",
			            &mhp, &exp, &infra)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		pr_ptr->r_mhp = mhp;
		pr_ptr->r_exp = exp;
		pr_ptr->infra = infra;
	}

	/* Hack -- Process 'I' for "info" and such */
	else if (buf[0] == 'I')
	{
		int hist, b_age, m_age;

		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (3 != sscanf(buf+2, "%d:%d:%d",
			            &hist, &b_age, &m_age)) return (PARSE_ERROR_GENERIC);

		pr_ptr->hist = hist;
		pr_ptr->b_age = b_age;
		pr_ptr->m_age = m_age;
	}

	/* Hack -- Process 'H' for "Height" */
	else if (buf[0] == 'H')
	{
		int m_b_ht, m_m_ht, f_b_ht, f_m_ht;

		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%d",
			            &m_b_ht, &m_m_ht, &f_b_ht, &f_m_ht)) return (PARSE_ERROR_GENERIC);

		pr_ptr->m_b_ht = m_b_ht;
		pr_ptr->m_m_ht = m_m_ht;
		pr_ptr->f_b_ht = f_b_ht;
		pr_ptr->f_m_ht = f_m_ht;
	}

	/* Hack -- Process 'W' for "Weight" */
	else if (buf[0] == 'W')
	{
		int m_b_wt, m_m_wt, f_b_wt, f_m_wt;

		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%d",
			            &m_b_wt, &m_m_wt, &f_b_wt, &f_m_wt)) return (PARSE_ERROR_GENERIC);

		pr_ptr->m_b_wt = m_b_wt;
		pr_ptr->m_m_wt = m_m_wt;
		pr_ptr->f_b_wt = f_b_wt;
		pr_ptr->f_m_wt = f_m_wt;
	}

	/* Hack -- Process 'F' for flags */
	else if (buf[0] == 'F')
	{
		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(pr_ptr->flags, OF_SIZE, k_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}

	/* Hack -- Process 'Y' for new race-only flags */
	else if (buf[0] == 'Y')
	{
		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(pr_ptr->pflags, PF_SIZE, player_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}
	
	/* Hack -- Process 'C' for class choices */
	else if (buf[0] == 'C')
	{
		/* There better be a current pr_ptr */
		if (!pr_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Hack - Parse this entry */
			pr_ptr->choice |= (1 << atoi(s));

			/* Start the next entry */
			s = t;
		}
	}
	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}


/*
 * Initialize the "c_info" array, by parsing an ascii "template" file
 */
errr parse_c_info(char *buf, header *head)
{
	int i, j;

	char *s, *t;

	/* Current entry */
	static player_class *pc_ptr = NULL;

	static int cur_title = 0;
	static int cur_equip = 0;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_MISSING_FIELD);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		pc_ptr = (player_class*)head->info_ptr + i;

		/* Store the name */
		pc_ptr->name = string_make(s);

		/* No titles and equipment yet */
		cur_title = 0;
		cur_equip = 0;
	}

	/* Process 'S' for "Stats" (one line only) */
	else if (buf[0] == 'S')
	{
		int adj;

		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Start the string */
		s = buf+1;

		/* For each stat */
		for (j = 0; j < A_MAX; j++)
		{
			/* Find the colon before the subindex */
			s = strchr(s, ':');

			/* Verify that colon */
			if (!s) return (PARSE_ERROR_MISSING_COLON);

			/* Nuke the colon, advance to the subindex */
			*s++ = '\0';

			/* Get the value */
			adj = atoi(s);

			/* Save the value */
			pc_ptr->c_adj[j] = adj;

			/* Next... */
			continue;
		}
	}

	/* Process 'C' for "Class Skills" (one line only) */
	else if (buf[0] == 'C')
	{
		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Verify text */
		if (!buf[1] || !buf[2]) return (PARSE_ERROR_MISSING_FIELD);

		/* Scan and save the values */
		if (!parse_skills(pc_ptr->c_skills, buf+2)) return (PARSE_ERROR_GENERIC);
	}

	/* Process 'X' for "Extra Skills" (one line only) */
	else if (buf[0] == 'X')
	{
		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Verify text */
		if (!buf[1] || !buf[2]) return (PARSE_ERROR_MISSING_FIELD);

		/* Scan and save the values */
		if (!parse_skills(pc_ptr->x_skills, buf+2)) return (PARSE_ERROR_GENERIC);
	}

	/* Process 'I' for "Info" (one line only) */
	else if (buf[0] == 'I')
	{
		int mhp, exp, sense_div;
		long sense_base;

		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%ld:%d",
			            &mhp, &exp, &sense_base, &sense_div))
			return (PARSE_ERROR_GENERIC);

		/* Save the values */
		pc_ptr->c_mhp = mhp;
		pc_ptr->c_exp = exp;
		pc_ptr->sense_base = sense_base;
		pc_ptr->sense_div = sense_div;
	}

	/* Process 'A' for "Attack Info" (one line only) */
	else if (buf[0] == 'A')
	{
		int max_attacks, min_weight, att_multiply;

		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (3 != sscanf(buf+2, "%d:%d:%d",
			            &max_attacks, &min_weight, &att_multiply))
			return (PARSE_ERROR_GENERIC);

		/* Save the values */
		pc_ptr->max_attacks = max_attacks;
		pc_ptr->min_weight = min_weight;
		pc_ptr->att_multiply = att_multiply;
	}

	/* Process 'M' for "Magic Info" (one line only) */
	else if (buf[0] == 'M')
	{
		int spell_book, spell_stat, spell_first, spell_weight;

		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (4 != sscanf(buf+2, "%d:%d:%d:%d",
		                &spell_book, &spell_stat,
		                &spell_first, &spell_weight))
			return (PARSE_ERROR_GENERIC);

		/* Save the values */
		pc_ptr->spell_book = spell_book;
		pc_ptr->spell_stat = spell_stat;
		pc_ptr->spell_first = spell_first;
		pc_ptr->spell_weight = spell_weight;
	}

	/* Process 'B' for "Spell/Prayer book info" */
	else if (buf[0] == 'B')
	{
		int spell, level, mana, fail, exp;
		player_magic *mp_ptr;
		magic_type *spell_ptr;

		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (5 != sscanf(buf+2, "%d:%d:%d:%d:%d",
		                &spell, &level, &mana, &fail, &exp))
			return (PARSE_ERROR_GENERIC);

		/* Validate the spell index */
		if ((spell >= PY_MAX_SPELLS) || (spell < 0))
			return (PARSE_ERROR_OUT_OF_BOUNDS);

		mp_ptr = &pc_ptr->spells;
		spell_ptr = &mp_ptr->info[spell];

		/* Save the values */
		spell_ptr->slevel = level;
		spell_ptr->smana = mana;
		spell_ptr->sfail = fail;
		spell_ptr->sexp = exp;
	}

	/* Process 'T' for "Titles" */
	else if (buf[0] == 'T')
	{
		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Get the text */
		s = buf+2;

		/* Store the text */
		pc_ptr->title[cur_title] = string_make(s);

		/* Next title */
		cur_title++;

		/* Limit number of titles */
		if (cur_title > PY_MAX_LEVEL / 5)
			return (PARSE_ERROR_TOO_MANY_ENTRIES);
	}

	/* Process 'E' for "Starting Equipment" */
	else if (buf[0] == 'E')
	{
		char *tval_s, *sval_s, *end_s;
		int tval, sval;
		int min, max;

		start_item *e_ptr;

		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Access the item */
		e_ptr = &pc_ptr->start_items[cur_equip];

		/* Find the beginning of the tval field */
		tval_s = strchr(buf, ':');
		if (!tval_s) return PARSE_ERROR_MISSING_COLON;
		*tval_s++ = '\0';
		if (!*tval_s) return PARSE_ERROR_MISSING_FIELD;

		/* Now find the beginning of the sval field */
		sval_s = strchr(tval_s, ':');
		if (!sval_s) return PARSE_ERROR_MISSING_COLON;
		*sval_s++ = '\0';
		if (!*sval_s) return PARSE_ERROR_MISSING_FIELD;

		/* Now find the beginning of the pval field */
		end_s = strchr(sval_s, ':');
		if (!end_s) return PARSE_ERROR_MISSING_COLON;
		*end_s++ = '\0';
		if (!*end_s) return PARSE_ERROR_MISSING_FIELD;

		/* Now convert the tval into its numeric equivalent */
		if (1 != sscanf(tval_s, "%d", &tval))
		{
			tval = tval_find_idx(tval_s);
			if (tval == -1) return PARSE_ERROR_UNRECOGNISED_TVAL;
		}

		/* Now find the sval */
		if (1 != sscanf(sval_s, "%d", &sval))
		{
			sval = lookup_sval(tval, sval_s);
			if (sval == -1) return PARSE_ERROR_UNRECOGNISED_SVAL;
		}


		/* Scan for the values */
		if (2 != sscanf(end_s, "%d:%d", &min, &max)) return (PARSE_ERROR_GENERIC);

		if ((min < 0) || (max < 0) || (min > 99) || (max > 99))
			return (PARSE_ERROR_INVALID_ITEM_NUMBER);

		/* Save the values */
		e_ptr->kind = objkind_get(tval, sval);
		e_ptr->min = min;
		e_ptr->max = max;

		/* Next item */
		cur_equip++;

		/* Limit number of starting items */
		if (cur_equip > MAX_START_ITEMS)
			return (PARSE_ERROR_TOO_MANY_ENTRIES);
	}

	/* Process 'F' for flags */
	else if (buf[0] == 'F')
	{
		/* There better be a current pc_ptr */
		if (!pc_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Parse every entry textually */
		for (s = buf + 2; *s; )
		{
			/* Find the end of this entry */
			for (t = s; *t && (*t != ' ') && (*t != '|'); ++t) /* loop */;

			/* Nuke and skip any dividers */
			if (*t)
			{
				*t++ = '\0';
				while ((*t == ' ') || (*t == '|')) t++;
			}

			/* Parse this entry */
			if (0 != grab_flag(pc_ptr->pflags, PF_SIZE, player_info_flags, s))
				return (PARSE_ERROR_INVALID_FLAG);

			/* Start the next entry */
			s = t;
		}
	}
	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}



/*
 * Initialize the "h_info" array, by parsing an ascii "template" file
 */
errr parse_h_info(char *buf, header *head)
{
	int i;

	char *s;

	/* Current entry */
	static hist_type *h_ptr = NULL;


	/* Process 'N' for "New/Number" */
	if (buf[0] == 'N')
	{
		int prv, nxt, prc, soc;

		/* Hack - get the index */
		i = error_idx + 1;

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		h_ptr = (hist_type*)head->info_ptr + i;

		/* Scan for the values */
		if (4 != sscanf(buf, "N:%d:%d:%d:%d",
			            &prv, &nxt, &prc, &soc)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		h_ptr->chart = prv;
		h_ptr->next = nxt;
		h_ptr->roll = prc;
		h_ptr->bonus = soc;
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		/* There better be a current h_ptr */
		if (!h_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Get the text */
		s = buf+2;

		/* Store the text */
		if (!add_text(&h_ptr->text, head, s))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}
	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}




/*
 * Initialize the "b_info" array, by parsing an ascii "template" file
 */
errr parse_b_info(char *buf, header *head)
{
	static int shop_idx = 0;
	static int owner_idx = 0;

	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Confirm the colon */
		if (buf[1] != ':') return PARSE_ERROR_MISSING_COLON;

		/* Get the index */
		shop_idx = atoi(buf+2);
		owner_idx = 0;

		return 0;
	}

	/* Process 'S' for "Owner" */
	else if (buf[0] == 'S')
	{
		owner_type *ot_ptr;
		char *s;
		int purse;

		if (owner_idx >= z_info->b_max)
			return PARSE_ERROR_TOO_MANY_ENTRIES;
		if ((shop_idx * z_info->b_max) + owner_idx >= head->info_num)
			return PARSE_ERROR_TOO_MANY_ENTRIES;

		ot_ptr = (owner_type *)head->info_ptr + (shop_idx * z_info->b_max) + owner_idx;
		if (!ot_ptr) return PARSE_ERROR_GENERIC;

		/* Extract the purse */
		if (1 != sscanf(buf+2, "%d", &purse)) return PARSE_ERROR_GENERIC;
		ot_ptr->max_cost = purse;

		s = strchr(buf+2, ':');
		if (!s || s[1] == 0) return PARSE_ERROR_GENERIC;

		ot_ptr->owner_name = add_name(head, s+1);
		if (!ot_ptr->owner_name)
			return PARSE_ERROR_OUT_OF_MEMORY;

		owner_idx++;
		return 0;
	}

	/* Oops */
	return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
}




/*
 * Initialize the "flavor_info" array, by parsing an ascii "template" file
 */
errr parse_flavor_info(char *buf, header *head)
{
	int i;
	
	/* Current entry */
	static flavor_type *flavor_ptr;


	/* Process 'N' for "Number" */
	if (buf[0] == 'N')
	{
		int tval, sval;
		int result;

		/* Scan the value */
		result = sscanf(buf, "N:%d:%d:%d", &i, &tval, &sval);

		/* Either two or three values */
		if ((result != 2) && (result != 3)) return (PARSE_ERROR_GENERIC);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		flavor_ptr = (flavor_type*)head->info_ptr + i;

		/* Save the tval */
		flavor_ptr->tval = (byte)tval;

		/* Save the sval */
		if (result == 2)
		{
			/* Megahack - unknown sval */
			flavor_ptr->sval = SV_UNKNOWN;
		}
		else
			flavor_ptr->sval = (byte)sval;
	}

	/* Process 'G' for "Graphics" */
	else if (buf[0] == 'G')
	{
		char d_char;
		int d_attr;

		/* There better be a current flavor_ptr */
		if (!flavor_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Paranoia */
		if (!buf[2]) return (PARSE_ERROR_MISSING_FIELD);
		if (!buf[3]) return (PARSE_ERROR_GENERIC);
		if (!buf[4]) return (PARSE_ERROR_GENERIC);

		/* Extract d_char */
		d_char = buf[2];

		/* If we have a longer string than expected ... */
		if (buf[5])
		{
			/* Advance "buf" on by 4 */
			buf += 4;

			/* Extract the colour */
			d_attr = color_text_to_attr(buf);
		}
		else
		{
			/* Extract the attr */
			d_attr = color_char_to_attr(buf[4]);
		}

		/* Paranoia */
		if (d_attr < 0) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		flavor_ptr->d_attr = d_attr;
		flavor_ptr->d_char = d_char;
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		/* There better be a current flavor_ptr */
		if (!flavor_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Paranoia */
		if (!buf[1]) return (PARSE_ERROR_MISSING_FIELD);
		if (!buf[2]) return (PARSE_ERROR_MISSING_FIELD);

		/* Store the text */
		if (!add_text(&flavor_ptr->text, head, buf + 2))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}


/*
 * Initialize the "s_info" array, by parsing an ascii "template" file
 */
errr parse_s_info(char *buf, header *head)
{
	int i;

	char *s;

	/* Current entry */
	static spell_type *s_ptr = NULL;


	/* Process 'N' for "New/Number/Name" */
	if (buf[0] == 'N')
	{
		/* Find the colon before the name */
		s = strchr(buf+2, ':');

		/* Verify that colon */
		if (!s) return (PARSE_ERROR_MISSING_COLON);

		/* Nuke the colon, advance to the name */
		*s++ = '\0';

		/* Paranoia -- require a name */
		if (!*s) return (PARSE_ERROR_GENERIC);

		/* Get the index */
		i = atoi(buf+2);

		/* Verify information */
		if (i <= error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

		/* Verify information */
		if (i >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

		/* Save the index */
		error_idx = i;

		/* Point at the "info" */
		s_ptr = (spell_type*)head->info_ptr + i;

		/* Store the name */
		if ((s_ptr->name = add_name(head, s)) == 0)
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	/* Process 'I' for "Info" (one line only) */
	else if (buf[0] == 'I')
	{
		int tval, sval, snum;

		/* There better be a current s_ptr */
		if (!s_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Scan for the values */
		if (3 != sscanf(buf+2, "%d:%d:%d",
			            &tval, &sval, &snum)) return (PARSE_ERROR_GENERIC);

		/* Save the values */
		s_ptr->tval = tval;
		s_ptr->sval = sval;
		s_ptr->snum = snum;

		/* Hack -- Use tval to calculate realm and spell index */
		s_ptr->realm = tval - TV_MAGIC_BOOK;
		s_ptr->spell_index = error_idx - (s_ptr->realm * PY_MAX_SPELLS);
	}

	/* Process 'D' for "Description" */
	else if (buf[0] == 'D')
	{
		/* There better be a current s_ptr */
		if (!s_ptr) return (PARSE_ERROR_MISSING_RECORD_HEADER);

		/* Get the text */
		s = buf+2;

		/* Store the text */
		if (!add_text(&s_ptr->text, head, s))
			return (PARSE_ERROR_OUT_OF_MEMORY);
	}

	else
	{
		/* Oops */
		return (PARSE_ERROR_UNDEFINED_DIRECTIVE);
	}

	/* Success */
	return (0);
}


/*
 * Initialise the info
 */
errr eval_info(eval_info_post_func eval_info_process, header *head)
{
	int err;

	/* Process the info */
	err = (*eval_info_process)(head);

	return(err);
}


static long eval_blow_effect(int effect, int atk_dam, int rlev)
{
	switch (effect)
	{
		/*other bad effects - minor*/
		case RBE_EAT_GOLD:
		case RBE_EAT_ITEM:
		case RBE_EAT_FOOD:
		case RBE_EAT_LIGHT:
		case RBE_LOSE_CHR:
		{
			atk_dam += 5;
			break;
		}
		/*other bad effects - poison / disease */
		case RBE_POISON:
		{
			atk_dam *= 5;
			atk_dam /= 4;
			atk_dam += rlev;
			break;
		}
		/*other bad effects - elements / sustains*/
		case RBE_TERRIFY:
		case RBE_ELEC:
		case RBE_COLD:
		case RBE_FIRE:
		{
			atk_dam += 10;
			break;
		}
		/*other bad effects - elements / major*/
		case RBE_ACID:
		case RBE_BLIND:
		case RBE_CONFUSE:
		case RBE_LOSE_STR:
		case RBE_LOSE_INT:
		case RBE_LOSE_WIS:
		case RBE_LOSE_DEX:
		case RBE_HALLU:
		{
			atk_dam += 20;
			break;
		}
		/*other bad effects - major*/
		case RBE_UN_BONUS:
		case RBE_UN_POWER:
		case RBE_LOSE_CON:
		{
			atk_dam += 30;
			break;
		}
		/*other bad effects - major*/
		case RBE_PARALYZE:
		case RBE_LOSE_ALL:
		{
			atk_dam += 40;
			break;
		}
		/* Experience draining attacks */
		case RBE_EXP_10:
		case RBE_EXP_20:
		{
			/* change inspired by Eddie because exp is infinite */
			atk_dam += 5;
			break;
		}
		case RBE_EXP_40:
		case RBE_EXP_80:
		{
			/* as above */
			atk_dam += 10;
			break;
		}
		/*Earthquakes*/
		case RBE_SHATTER:
		{
			atk_dam += 300;
			break;
		}
		/*nothing special*/
		default: break;
	}

	return (atk_dam);
}



/*
 * Go through the attack types for this monster.
 * We look for the maximum possible maximum damage that this
 * monster can inflict in 10 game turns.
 *
 * We try to scale this based on assumed resists,
 * chance of casting spells and of spells failing,
 * chance of hitting in melee, and particularly speed.
 */

static long eval_max_dam(monster_race *r_ptr)
{
	int hp, rlev, i;
	int melee_dam, atk_dam, spell_dam, breath_dam;
	int dam = 1;

	/*clear the counters*/
	melee_dam = breath_dam = atk_dam = spell_dam = 0;

	/* Evaluate average HP for this monster */
	hp = r_ptr->avg_hp;

	/* Extract the monster level, force 1 for town monsters */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Assume single resist for the elemental attacks */
	if (rsf_has(r_ptr->spell_flags, RSF_BR_ACID))
	{
		breath_dam = (RES_ACID_ADJ(MIN(BR_ACID_MAX,
			(hp / BR_ACID_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 20;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_ELEC))
	{
		breath_dam = (RES_ELEC_ADJ(MIN(BR_ELEC_MAX, 
			(hp / BR_ELEC_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 10;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_FIRE))
	{
		breath_dam = (RES_FIRE_ADJ(MIN(BR_FIRE_MAX,
			(hp / BR_FIRE_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 10;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_COLD))
	{
		breath_dam = (RES_COLD_ADJ(MIN(BR_COLD_MAX,
			(hp / BR_COLD_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 10;
	}
	/* Same for poison, but lower damage cap */
	if (rsf_has(r_ptr->spell_flags, RSF_BR_POIS))
	{
		breath_dam = (RES_POIS_ADJ(MIN(BR_POIS_MAX,
			(hp / BR_POIS_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = (breath_dam * 5 / 4)
			+ rlev;
	}
	/*
	 * Same formula for the high resist attacks
	 * (remember, we are assuming maximum resisted damage
	 * so we *minimise* the resistance)
	 * See also: melee2.c, spells1.c, constants.h
	 */
	if (rsf_has(r_ptr->spell_flags, RSF_BR_NETH))
	{
		breath_dam = (RES_NETH_ADJ(MIN(BR_NETH_MAX,
			(hp / BR_NETH_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 2000
			/ (rlev + 1);
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_CHAO))
	{
		breath_dam = (RES_CHAO_ADJ(MIN(BR_CHAO_MAX,
			(hp / BR_CHAO_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 2000
			/ (rlev + 1);
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_DISE))
	{
		breath_dam = (RES_DISE_ADJ(MIN(BR_DISE_MAX,
			(hp / BR_DISE_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 50;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_SHAR))
	{
		breath_dam = (RES_SHAR_ADJ(MIN(BR_SHAR_MAX,
			(hp / BR_SHAR_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = (breath_dam * 5 / 4)
			+ 5;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_LIGHT))
	{
		breath_dam = (RES_LIGHT_ADJ(MIN(BR_LIGHT_MAX,
			(hp / BR_LIGHT_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 10;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_DARK))
	{
		breath_dam = (RES_DARK_ADJ(MIN(BR_DARK_MAX,
			(hp / BR_DARK_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 10;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_CONF))
	{
		breath_dam = (RES_CONF_ADJ(MIN(BR_CONF_MAX,
			(hp / BR_CONF_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 20;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_SOUN))
	{
		breath_dam = (RES_SOUN_ADJ(MIN(BR_SOUN_MAX,
			(hp / BR_SOUN_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 20;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_NEXU))
	{
		breath_dam = (RES_NEXU_ADJ(MIN(BR_NEXU_MAX,
			(hp / BR_NEXU_DIVISOR)), MINIMISE));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 20;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_TIME))
	{
		breath_dam = MIN(BR_TIME_MAX, (hp / BR_TIME_DIVISOR));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 2000 / (rlev + 1);
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_INER))
	{
		breath_dam = MIN(BR_INER_MAX, (hp / BR_INER_DIVISOR));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 30;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_GRAV))
	{
		breath_dam = MIN(BR_GRAV_MAX, (hp / BR_GRAV_DIVISOR));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 30;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_PLAS))
	{
		breath_dam = MIN(BR_PLAS_MAX, (hp / BR_PLAS_DIVISOR));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 30;
	}
	if (rsf_has(r_ptr->spell_flags, RSF_BR_WALL))
	{
		breath_dam = MIN(BR_FORC_MAX, (hp / BR_FORC_DIVISOR));
		if (spell_dam < breath_dam) spell_dam = breath_dam + 30;
	}
	/* Handle the attack spells, again assuming minimised single resists for max damage */
	if (rsf_has(r_ptr->spell_flags, RSF_BA_ACID) && spell_dam < (RES_ACID_ADJ(BA_ACID_DMG(rlev, MAXIMISE), MINIMISE) + 20))
		spell_dam = (RES_ACID_ADJ(BA_ACID_DMG(rlev, MAXIMISE), MINIMISE) + 20);
	if (rsf_has(r_ptr->spell_flags, RSF_BA_ELEC) && spell_dam < (RES_ELEC_ADJ(BA_ELEC_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_ELEC_ADJ(BA_ELEC_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	if (rsf_has(r_ptr->spell_flags, RSF_BA_FIRE) && spell_dam < (RES_FIRE_ADJ(BA_FIRE_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_FIRE_ADJ(BA_FIRE_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	if (rsf_has(r_ptr->spell_flags, RSF_BA_COLD) && spell_dam < (RES_COLD_ADJ(BA_COLD_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_COLD_ADJ(BA_COLD_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	if (rsf_has(r_ptr->spell_flags, RSF_BA_POIS) && spell_dam < 8)
		spell_dam = 8;
	if (rsf_has(r_ptr->spell_flags, RSF_BA_NETH) && spell_dam < (RES_NETH_ADJ(BA_NETH_DMG(rlev, MAXIMISE), MINIMISE) + 2000 / (rlev + 1)))
		spell_dam = (RES_NETH_ADJ(BA_NETH_DMG(rlev, MAXIMISE), MINIMISE) + 2000 / (rlev + 1));
	if (rsf_has(r_ptr->spell_flags, RSF_BA_WATE) && spell_dam < (BA_WATE_DMG(rlev, MAXIMISE) + 20))
		spell_dam = (BA_WATE_DMG(rlev, MAXIMISE) + 20);
	if (rsf_has(r_ptr->spell_flags, RSF_BA_MANA) && spell_dam < (BA_MANA_DMG(rlev, MAXIMISE) + 100))
		spell_dam = (BA_MANA_DMG(rlev, MAXIMISE) + 100);
	if (rsf_has(r_ptr->spell_flags, RSF_BA_DARK) && spell_dam < (RES_DARK_ADJ(BA_DARK_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_DARK_ADJ(BA_DARK_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	/* Small annoyance value */
	if (rsf_has(r_ptr->spell_flags, RSF_DRAIN_MANA) && spell_dam < 5)
		spell_dam = 5;
	/* For all attack forms the player can save against, spell_damage is halved */
	if (rsf_has(r_ptr->spell_flags, RSF_MIND_BLAST) && spell_dam < (MIND_BLAST_DMG(rlev, MAXIMISE) / 2))
		spell_dam = (MIND_BLAST_DMG(rlev, MAXIMISE) / 2);
	if (rsf_has(r_ptr->spell_flags, RSF_BRAIN_SMASH) && spell_dam < (BRAIN_SMASH_DMG(rlev, MAXIMISE) / 2))
		spell_dam = (BRAIN_SMASH_DMG(rlev, MAXIMISE) / 2);
	if (rsf_has(r_ptr->spell_flags, RSF_CAUSE_1) && spell_dam < (CAUSE1_DMG(rlev, MAXIMISE) / 2))
		spell_dam = (CAUSE1_DMG(rlev, MAXIMISE) / 2);
	if (rsf_has(r_ptr->spell_flags, RSF_CAUSE_2) && spell_dam < (CAUSE2_DMG(rlev, MAXIMISE) / 2))
		spell_dam = (CAUSE2_DMG(rlev, MAXIMISE) / 2);
	if (rsf_has(r_ptr->spell_flags, RSF_CAUSE_3) && spell_dam < (CAUSE3_DMG(rlev, MAXIMISE) / 2))
		spell_dam = (CAUSE3_DMG(rlev, MAXIMISE) / 2);
	if (rsf_has(r_ptr->spell_flags, RSF_CAUSE_4) && spell_dam < (CAUSE4_DMG(rlev, MAXIMISE) / 2))
		spell_dam = (CAUSE4_DMG(rlev, MAXIMISE) / 2);
	if (rsf_has(r_ptr->spell_flags, RSF_BO_ACID) && spell_dam < (RES_ACID_ADJ(BO_ACID_DMG(rlev, MAXIMISE), MINIMISE) + 20))
		spell_dam = (RES_ACID_ADJ(BO_ACID_DMG(rlev, MAXIMISE), MINIMISE) + 20);
	if (rsf_has(r_ptr->spell_flags, RSF_BO_ELEC) && spell_dam < (RES_ELEC_ADJ(BO_ELEC_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_ELEC_ADJ(BO_ELEC_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	if (rsf_has(r_ptr->spell_flags, RSF_BO_FIRE) && spell_dam < (RES_FIRE_ADJ(BO_FIRE_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_FIRE_ADJ(BO_FIRE_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	if (rsf_has(r_ptr->spell_flags, RSF_BO_COLD) && spell_dam < (RES_COLD_ADJ(BO_COLD_DMG(rlev, MAXIMISE), MINIMISE) + 10))
		spell_dam = (RES_COLD_ADJ(BO_COLD_DMG(rlev, MAXIMISE), MINIMISE) + 10);
	if (rsf_has(r_ptr->spell_flags, RSF_BO_NETH) && spell_dam < (RES_NETH_ADJ(BO_NETH_DMG(rlev, MAXIMISE), MINIMISE) + 2000 / (rlev + 1)))
		spell_dam = (RES_NETH_ADJ(BO_NETH_DMG(rlev, MAXIMISE), MINIMISE) + 2000 / (rlev + 1));
	if (rsf_has(r_ptr->spell_flags, RSF_BO_WATE) && spell_dam < (BO_WATE_DMG(rlev, MAXIMISE) + 20))
		spell_dam = (BO_WATE_DMG(rlev, MAXIMISE) + 20);
	if (rsf_has(r_ptr->spell_flags, RSF_BO_MANA) && spell_dam < (BO_MANA_DMG(rlev, MAXIMISE)))
		spell_dam = (BO_MANA_DMG(rlev, MAXIMISE));
	if (rsf_has(r_ptr->spell_flags, RSF_BO_PLAS) && spell_dam < (BO_PLAS_DMG(rlev, MAXIMISE)))
		spell_dam = (BO_PLAS_DMG(rlev, MAXIMISE));
	if (rsf_has(r_ptr->spell_flags, RSF_BO_ICEE) && spell_dam < (RES_COLD_ADJ(BO_ICEE_DMG(rlev, MAXIMISE), MINIMISE)))
		spell_dam = (RES_COLD_ADJ(BO_ICEE_DMG(rlev, MAXIMISE), MINIMISE));
	if (rsf_has(r_ptr->spell_flags, RSF_MISSILE) && spell_dam < (MISSILE_DMG(rlev, MAXIMISE)))
		spell_dam = (MISSILE_DMG(rlev, MAXIMISE));
	/* Small annoyance value */
	if (rsf_has(r_ptr->spell_flags, RSF_SCARE) && spell_dam < 5)
		spell_dam = 5;
	/* Somewhat higher annoyance values */
	if (rsf_has(r_ptr->spell_flags, RSF_BLIND) && spell_dam < 10)
		spell_dam = 8;
	if (rsf_has(r_ptr->spell_flags, RSF_CONF) && spell_dam < 10)
		spell_dam = 10;
	/* A little more dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_SLOW) && spell_dam < 15)
		spell_dam = 15;
	/* Quite dangerous at an early level */
	if (rsf_has(r_ptr->spell_flags, RSF_HOLD) && spell_dam < 25)
		spell_dam = 25;
	/* Arbitrary values along similar lines from here on */
	if (rsf_has(r_ptr->spell_flags, RSF_HASTE) && spell_dam < 70)
		spell_dam = 70;
	if (rsf_has(r_ptr->spell_flags, RSF_HEAL) && spell_dam < 30)
		spell_dam = 30;
	if (rsf_has(r_ptr->spell_flags, RSF_BLINK) && spell_dam < 5)
		spell_dam = 15;
	if (rsf_has(r_ptr->spell_flags, RSF_TELE_TO) && spell_dam < 25)
		spell_dam = 25;
	if (rsf_has(r_ptr->spell_flags, RSF_TELE_AWAY) && spell_dam < 25)
		spell_dam = 25;
	if (rsf_has(r_ptr->spell_flags, RSF_TELE_LEVEL) && spell_dam < 40)
		spell_dam = 25;
	if (rsf_has(r_ptr->spell_flags, RSF_DARKNESS) && spell_dam < 5)
		spell_dam = 6;
	if (rsf_has(r_ptr->spell_flags, RSF_TRAPS) && spell_dam < 10)
		spell_dam = 5;
	if (rsf_has(r_ptr->spell_flags, RSF_FORGET) && spell_dam < 25)
		spell_dam = 5;
	/* All summons are assigned arbitrary values */
	/* Summon kin is more dangerous at deeper levels */
	if (rsf_has(r_ptr->spell_flags, RSF_S_KIN) && spell_dam < rlev * 2)
		spell_dam = rlev * 2;
	/* Dangerous! */
	if (rsf_has(r_ptr->spell_flags, RSF_S_HI_DEMON) && spell_dam < 250)
		spell_dam = 250;
	/* Somewhat dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_MONSTER) && spell_dam < 40)
		spell_dam = 40;
	/* More dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_MONSTERS) && spell_dam < 80)
		spell_dam = 80;
	/* Mostly just annoying */
	if (rsf_has(r_ptr->spell_flags, RSF_S_ANIMAL) && spell_dam < 30)
		spell_dam = 30;
	if (rsf_has(r_ptr->spell_flags, RSF_S_SPIDER) && spell_dam < 20)
		spell_dam = 20;
	/* Can be quite dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_HOUND) && spell_dam < 100)
		spell_dam = 100;
	/* Dangerous! */
	if (rsf_has(r_ptr->spell_flags, RSF_S_HYDRA) && spell_dam < 150)
		spell_dam = 150;
	/* Can be quite dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_ANGEL) && spell_dam < 150)
		spell_dam = 150;
	/* All of these more dangerous at higher levels */
	if (rsf_has(r_ptr->spell_flags, RSF_S_DEMON) && spell_dam < (rlev * 3) / 2)
		spell_dam = (rlev * 3) / 2;
	if (rsf_has(r_ptr->spell_flags, RSF_S_UNDEAD) && spell_dam < (rlev * 3) / 2)
		spell_dam = (rlev * 3) / 2;
	if (rsf_has(r_ptr->spell_flags, RSF_S_DRAGON) && spell_dam < (rlev * 3) / 2)
		spell_dam = (rlev * 3) / 2;
	/* Extremely dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_HI_UNDEAD) && spell_dam < 400)
		spell_dam = 400;
	/* Extremely dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_HI_DRAGON) && spell_dam < 400)
		spell_dam = 400;
	/* Extremely dangerous */
	if (rsf_has(r_ptr->spell_flags, RSF_S_WRAITH) && spell_dam < 450)
		spell_dam = 450;
	/* Most dangerous summon */
	if (rsf_has(r_ptr->spell_flags, RSF_S_UNIQUE) && spell_dam < 500)
		spell_dam = 500;

	/* Hack - Apply over 10 rounds */
	spell_dam *= 10;

	/* Scale for frequency and availability of mana / ammo */
	if (spell_dam)
	{
		int freq = r_ptr->freq_spell;

			/* Hack -- always get 1 shot */
			if (freq < 10) freq = 10;

			/* Adjust for frequency */
			spell_dam = spell_dam * freq / 100;
	}

	/* Check attacks */
	for (i = 0; i < 4; i++)
	{
		/* Extract the attack infomation */
		int effect = r_ptr->blow[i].effect;
		int method = r_ptr->blow[i].method;
		int d_dice = r_ptr->blow[i].d_dice;
		int d_side = r_ptr->blow[i].d_side;

		/* Hack -- no more attacks */
		if (!method) continue;

		/* Assume maximum damage*/
		atk_dam = eval_blow_effect(effect, d_dice * d_side, r_ptr->level);

		switch (method)
		{
				/*stun definitely most dangerous*/
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_BUTT:
				case RBM_CRUSH:
				{
					atk_dam *= 4;
					atk_dam /= 3;
					break;
				}
				/*cut*/
				case RBM_CLAW:
				case RBM_BITE:
				{
					atk_dam *= 7;
					atk_dam /= 5;
					break;
				}
				default: 
				{
					break;
				}
			}

			/* Normal melee attack */
			if (!rf_has(r_ptr->flags, RF_NEVER_BLOW))
			{
				/* Keep a running total */
				melee_dam += atk_dam;
			}
	}

		/* 
		 * Apply damage over 10 rounds. We assume that the monster has to make contact first.
		 * Hack - speed has more impact on melee as has to stay in contact with player.
		 * Hack - this is except for pass wall and kill wall monsters which can always get to the player.
		 * Hack - use different values for huge monsters as they strike out to range 2.
		 */
		if (flags_test(r_ptr->flags, RF_SIZE, RF_KILL_WALL, RF_PASS_WALL, FLAG_END))
				melee_dam *= 10;
		else
		{
			melee_dam = melee_dam * 3 + melee_dam * extract_energy[r_ptr->speed + (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)] / 7;
		}

		/*
		 * Scale based on attack accuracy. We make a massive number of assumptions here and just use monster level.
		 */
		melee_dam = melee_dam * MIN(45 + rlev * 3, 95) / 100;

		/* Hack -- Monsters that multiply ignore the following reductions */
		if (!rf_has(r_ptr->flags, RF_MULTIPLY))
		{
			/*Reduce damamge potential for monsters that move randomly */
			if (flags_test(r_ptr->flags, RF_SIZE, RF_RAND_25, RF_RAND_50, FLAG_END))
			{
				int reduce = 100;

				if (rf_has(r_ptr->flags, RF_RAND_25)) reduce -= 25;
				if (rf_has(r_ptr->flags, RF_RAND_50)) reduce -= 50;

				/*even moving randomly one in 8 times will hit the player*/
				reduce += (100 - reduce) / 8;

				/* adjust the melee damage*/
				melee_dam = (melee_dam * reduce) / 100;
			}

			/*monsters who can't move aren't nearly as much of a combat threat*/
			if (rf_has(r_ptr->flags, RF_NEVER_MOVE))
			{
				if (rsf_has(r_ptr->spell_flags, RSF_TELE_TO) ||
				    rsf_has(r_ptr->spell_flags, RSF_BLINK))
				{
					/* Scale for frequency */
					melee_dam = melee_dam / 5 + 4 * melee_dam * r_ptr->freq_spell / 500;

					/* Incorporate spell failure chance */
					if (!rf_has(r_ptr->flags, RF_STUPID)) melee_dam = melee_dam / 5 + 4 * melee_dam * MIN(75 + (rlev + 3) / 4, 100) / 500;
				}
				else if (rf_has(r_ptr->flags, RF_INVISIBLE)) melee_dam /= 3;
				else melee_dam /= 5;
			}
		}

		/* But keep at a minimum */
		if (melee_dam < 1) melee_dam = 1;

	/*
	 * Get the max damage attack
	 */

	if (dam < spell_dam) dam = spell_dam;
	if (dam < melee_dam) dam = melee_dam;

	r_ptr->highest_threat = dam;

	/*
	 * Adjust for speed.  Monster at speed 120 will do double damage,
	 * monster at speed 100 will do half, etc.  Bonus for monsters who can haste self.
	 */
	dam = (dam * extract_energy[r_ptr->speed + (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)]) / 10;

	/*
	 * Adjust threat for speed -- multipliers are more threatening.
	 */
	if (rf_has(r_ptr->flags, RF_MULTIPLY))
		r_ptr->highest_threat = (r_ptr->highest_threat * extract_energy[r_ptr->speed + (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)]) / 5;

	/*
	 * Adjust threat for friends.
	 */
	if (rf_has(r_ptr->flags, RF_FRIENDS))
		r_ptr->highest_threat *= 2;
	else if (rf_has(r_ptr->flags, RF_FRIEND))
		r_ptr->highest_threat = r_ptr->highest_threat * 3 / 2;
		
	/*but deep in a minimum*/
	if (dam < 1) dam  = 1;

	/* We're done */
	return (dam);
}

/* Evaluate and adjust a monsters hit points for how easily the monster is damaged */
static long eval_hp_adjust(monster_race *r_ptr)
{
	long hp;
	int resists = 1;
	int hide_bonus = 0;

	/* Get the monster base hitpoints */
	hp = r_ptr->avg_hp;

	/* Never moves with no ranged attacks - high hit points count for less */
	if (rf_has(r_ptr->flags, RF_NEVER_MOVE) && !(r_ptr->freq_innate || r_ptr->freq_spell))
	{
		hp /= 2;
		if (hp < 1) hp = 1;
	}

	/* Just assume healers have more staying power */
	if (rsf_has(r_ptr->spell_flags, RSF_HEAL)) hp = (hp * 6) / 5;

	/* Miscellaneous improvements */
	if (rf_has(r_ptr->flags, RF_REGENERATE)) {hp *= 10; hp /= 9;}
	if (rf_has(r_ptr->flags, RF_PASS_WALL)) {hp *= 3; hp /= 2;}

	/* Calculate hide bonus */
	if (rf_has(r_ptr->flags, RF_EMPTY_MIND)) hide_bonus += 2;
	else
	{
		if (rf_has(r_ptr->flags, RF_COLD_BLOOD)) hide_bonus += 1;
		if (rf_has(r_ptr->flags, RF_WEIRD_MIND)) hide_bonus += 1;
	}

	/* Invisibility */
	if (rf_has(r_ptr->flags, RF_INVISIBLE))
	{
		hp = (hp * (r_ptr->level + hide_bonus + 1)) / MAX(1, r_ptr->level);
	}

	/* Monsters that can teleport are a hassle, and can easily run away */
	if (flags_test(r_ptr->spell_flags, RSF_SIZE, RSF_TPORT, RSF_TELE_AWAY,
	    RSF_TELE_LEVEL, FLAG_END))
		hp = (hp * 6) / 5;

	/*
 	 * Monsters that multiply are tougher to kill
	 */
	if (rf_has(r_ptr->flags, RF_MULTIPLY)) hp *= 2;

	/* Monsters with resistances are harder to kill.
	   Therefore effective slays / brands against them are worth more. */
	if (rf_has(r_ptr->flags, RF_IM_ACID)) resists += 2;
	if (rf_has(r_ptr->flags, RF_IM_FIRE)) resists += 2;
	if (rf_has(r_ptr->flags, RF_IM_COLD)) resists += 2;
	if (rf_has(r_ptr->flags, RF_IM_ELEC)) resists += 2;
	if (rf_has(r_ptr->flags, RF_IM_POIS)) resists += 2;

	/* Bonus for multiple basic resists and weapon resists */
	if (resists >= 12) resists *= 6;
	else if (resists >= 10) resists *= 4;
	else if (resists >= 8) resists *= 3;
	else if (resists >= 6) resists *= 2;

	/* If quite resistant, reduce resists by defense holes */
	if (resists >= 6)
	{
		if (rf_has(r_ptr->flags, RF_HURT_ROCK)) resists -= 1;
		if (!rf_has(r_ptr->flags, RF_NO_SLEEP)) resists -= 3;
		if (!rf_has(r_ptr->flags, RF_NO_FEAR)) resists -= 2;
		if (!rf_has(r_ptr->flags, RF_NO_CONF)) resists -= 2;
		if (!rf_has(r_ptr->flags, RF_NO_STUN)) resists -= 1;

		if (resists < 5) resists = 5;
	}

	/* If quite resistant, bonus for high resists */
	if (resists >= 3)
	{
		if (rf_has(r_ptr->flags, RF_IM_WATER)) resists += 1;
		if (rf_has(r_ptr->flags, RF_RES_NETH)) resists += 1;
		if (rf_has(r_ptr->flags, RF_RES_NEXUS)) resists += 1;
		if (rf_has(r_ptr->flags, RF_RES_DISE)) resists += 1;
	}

	/* Scale resists */
	resists = resists * 25;

	/* Monster resistances */
	if (resists < (r_ptr->ac + resists) / 3)
	{
		hp += (hp * resists) / (150 + r_ptr->level); 	
	}
	else
	{
		hp += (hp * (r_ptr->ac + resists) / 3) / (150 + r_ptr->level); 			
	}

	/*boundry control*/
	if (hp < 1) hp = 1;

	return (hp);

}


/*
 * Evaluate the monster power ratings to be stored in r_info.raw
 */
errr eval_r_power(header *head)
{
	int i, j;
	byte lvl;
	long hp, av_hp, av_dam;
	long tot_hp[MAX_DEPTH];
	long dam;
	long *power;
	long tot_dam[MAX_DEPTH];
	long mon_count[MAX_DEPTH];
	monster_race *r_ptr = NULL;

	int iteration;

	/* If we came here from the .raw file, the monster power data is already done */
	/* Hack - use Morgy (#547) as the test case */
	r_ptr = (monster_race*)head->info_ptr + 547;
	if (r_ptr->power)
	{
	     /*	msg_print("Monster power array already filled - returning."); */
		return 0;
	}

	/* Allocate space for power */
	power = C_ZNEW(z_info->r_max, long);


for (iteration = 0; iteration < 3; iteration ++)
{

	/* Reset the sum of all monster power values */
	tot_mon_power = 0;

	/* Make sure all arrays start at zero */
	for (i = 0; i < MAX_DEPTH; i++)
	{
		tot_hp[i] = 0;
		tot_dam[i] = 0;
		mon_count[i] = 0;
	}

	/*
	 * Go through r_info and evaluate power ratings & flows.
	 */
	for (i = 0; i < z_info->r_max; i++)
	{
		/* Point at the "info" */
		r_ptr = (monster_race*)head->info_ptr + i;

		/*** Evaluate power ratings ***/

		/* Set the current level */
		lvl = r_ptr->level;

		/* Maximum damage this monster can do in 10 game turns */
		dam = eval_max_dam(r_ptr);

		/* Adjust hit points based on resistances */
		hp = eval_hp_adjust(r_ptr);

		/* Hack -- set exp */
		if (lvl == 0) r_ptr->mexp = 0L;
		else
		{
			/* Compute depths of non-unique monsters */
			if (!rf_has(r_ptr->flags, RF_UNIQUE))
			{
				long mexp = (hp * dam) / 25;
				long threat = r_ptr->highest_threat;

				/* Compute level algorithmically */
				for (j = 1; (mexp > j + 4) || (threat > j + 5); mexp -= j * j, threat -= (j + 4), j++);

				/* Set level */
				lvl = MIN(( j > 250 ? 90 + (j - 250) / 20 : 	/* Level 90 and above */
						(j > 130 ? 70 + (j - 130) / 6 :	/* Level 70 and above */
						(j > 40 ? 40 + (j - 40) / 3 :	/* Level 40 and above */
						j))), 99);

				/* Set level */
				if (arg_rebalance)
					r_ptr->level = lvl;
			}

			if (arg_rebalance)
			{
				/* Hack -- for Ungoliant */
				if (hp > 10000) r_ptr->mexp = (hp / 25) * (dam / lvl);
				else r_ptr->mexp = (hp * dam) / (lvl * 25);

				/* Round to 2 significant figures */
				if (r_ptr->mexp > 100)
				{
					if (r_ptr->mexp < 1000) { r_ptr->mexp = (r_ptr->mexp + 5) / 10; r_ptr->mexp *= 10; }
					else if (r_ptr->mexp < 10000) { r_ptr->mexp = (r_ptr->mexp + 50) / 100; r_ptr->mexp *= 100; }
					else if (r_ptr->mexp < 100000) { r_ptr->mexp = (r_ptr->mexp + 500) / 1000; r_ptr->mexp *= 1000; }
					else if (r_ptr->mexp < 1000000) { r_ptr->mexp = (r_ptr->mexp + 5000) / 10000; r_ptr->mexp *= 10000; }
					else if (r_ptr->mexp < 10000000) { r_ptr->mexp = (r_ptr->mexp + 50000) / 100000; r_ptr->mexp *= 100000; }
				}
			}
		}

		/* If we're rebalancing, this is a nop, if not, we restore the orig value */
		lvl = r_ptr->level;
		if ((lvl) && (r_ptr->mexp < 1L)) r_ptr->mexp = 1L;

		/*
		 * Hack - We have to use an adjustment factor to prevent overflow.
		 */
		if (lvl >= 90)
		{
			hp /= 1000;
			dam /= 1000;
		}
		else if (lvl >= 65)
		{
			hp /= 100;
			dam /= 100;
		}
		else if (lvl >= 40)
		{
			hp /= 10;
			dam /= 10;
		}

		/* Define the power rating */
		power[i] = hp * dam;

		/* Adjust for group monsters.  Average in-level group size is 5 */
		if (rf_has(r_ptr->flags, RF_UNIQUE)) ;

		else if (rf_has(r_ptr->flags, RF_FRIEND)) power[i] *= 2;

		else if (rf_has(r_ptr->flags, RF_FRIENDS)) power[i] *= 5;

		/* Adjust for multiplying monsters. This is modified by the speed,
                 * as fast multipliers are much worse than slow ones. We also adjust for
		 * ability to bypass walls or doors.
                 */
		if (rf_has(r_ptr->flags, RF_MULTIPLY))
		{
			if (flags_test(r_ptr->flags, RF_SIZE, RF_KILL_WALL, RF_PASS_WALL, FLAG_END))
				power[i] = MAX(power[i], power[i] * extract_energy[r_ptr->speed
					+ (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)]);
			else if (flags_test(r_ptr->flags, RF_SIZE, RF_OPEN_DOOR, RF_BASH_DOOR, FLAG_END))
				power[i] = MAX(power[i], power[i] *  extract_energy[r_ptr->speed
					+ (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)] * 3 / 2);
			else
				power[i] = MAX(power[i], power[i] * extract_energy[r_ptr->speed
					+ (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)] / 2);
		}

		/*
		 * Update the running totals - these will be used as divisors later
		 * Total HP / dam / count for everything up to the current level
		 */
		for (j = lvl; j < (lvl == 0 ? lvl + 1: MAX_DEPTH); j++)
		{
			int count = 10;

			/*
			 * Uniques don't count towards monster power on the level.
			 */
			if (rf_has(r_ptr->flags, RF_UNIQUE)) continue;

			/*
			 * Specifically placed monsters don't count towards monster power on the level.
			 */
			if (!(r_ptr->rarity)) continue;

			/*
			 * Hack -- provide adjustment factor to prevent overflow
			 */
			if ((j == 90) && (r_ptr->level < 90))
			{
				hp /= 10;
				dam /= 10;
			}

			if ((j == 65) && (r_ptr->level < 65))
			{
				hp /= 10;
				dam /= 10;
			}

			if ((j == 40) && (r_ptr->level < 40))
			{
				hp /= 10;
				dam /= 10;
			}

			/*
			 * Hack - if it's a group monster or multiplying monster, add several to the count
			 * so that the averages don't get thrown off
			 */

			if (rf_has(r_ptr->flags, RF_FRIEND)) count = 20;
			else if (rf_has(r_ptr->flags, RF_FRIENDS)) count = 50;

			if (rf_has(r_ptr->flags, RF_MULTIPLY))
			{
				if (flags_test(r_ptr->flags, RF_SIZE, RF_KILL_WALL, RF_PASS_WALL, FLAG_END))
					count = MAX(1, extract_energy[r_ptr->speed
						+ (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)]) * count;
				else if (flags_test(r_ptr->flags, RF_SIZE, RF_OPEN_DOOR, RF_BASH_DOOR, FLAG_END))
					count = MAX(1, extract_energy[r_ptr->speed
						+ (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)] * 3 / 2) * count;
				else
					count = MAX(1, extract_energy[r_ptr->speed
						+ (rsf_has(r_ptr->spell_flags, RSF_HASTE) ? 5 : 0)] / 2) * count;
			}

			/*
			 * Very rare monsters count less towards total monster power on the level.
			 */
			if (r_ptr->rarity > count)
			{
				hp = hp * count / r_ptr->rarity;
				dam = dam * count / r_ptr->rarity;

				count = r_ptr->rarity;
			}

			tot_hp[j] += hp;
			tot_dam[j] += dam;

			mon_count[j] += count / r_ptr->rarity;
		}

	}

	/* Apply divisors now */
	for (i = 0; i < z_info->r_max; i++)
	{
		int new_power;

		/* Point at the "info" */
		r_ptr = (monster_race*)head->info_ptr + i;

		/* Extract level */
		lvl = r_ptr->level;

		/* Paranoia */
		if (tot_hp[lvl] != 0 && tot_dam[lvl] != 0)
		{
			/* Divide by average HP and av damage for all in-level monsters */
			/* Note we have factored in the above 'adjustment factor' */
			av_hp = tot_hp[lvl] * 10 / mon_count[lvl];
			av_dam = tot_dam[lvl] * 10 / mon_count[lvl];

			/* XXX Justifiable paranoia - avoid divide by zero errors */
			if (av_hp > 0) power[i] = power[i] / av_hp;
			if (av_dam > 0) power[i] = power[i] / av_dam;

			/* Assign monster power */
			r_ptr->power = (s16b)power[i];

			/* Never less than 1 */
			if (r_ptr->power < 1) r_ptr->power = 1;

			/* Get power */
			new_power = r_ptr->power;

			/* Compute rarity algorithmically */
			for (j = 1; new_power > j; new_power -= j * j, j++);

			/* Set rarity */
			if (arg_rebalance)
				r_ptr->rarity = j;
		}
	}

}

	/* Free power array */
	FREE(power);

	/* Success */
	return(0);
}

/*
 * Create the slay cache by determining the number of different slay
 * combinations available to ego items
 */
errr eval_e_slays(header *head)
{
	int i;
	int j;
	int count = 0;
	bitflag cacheme[OF_SIZE];
	bitflag slay_mask[OF_SIZE];
	bitflag **dupcheck;
	ego_item_type *e_ptr;

	/* Build the slay mask */
	flags_init(slay_mask, OF_SIZE, OF_ALL_SLAY_MASK, FLAG_END);

	/* Calculate necessary size of slay_cache */
	dupcheck = C_ZNEW(z_info->e_max, bitflag *);
	
	for (i = 0; i < z_info->e_max; i++)
	{
		dupcheck[i] = C_ZNEW(OF_SIZE, bitflag);
		e_ptr = (ego_item_type*)head->info_ptr + i;

		/* Find the slay flags on this ego */		
		of_copy(cacheme, e_ptr->flags);
		of_inter(cacheme, slay_mask);

		/* Only consider non-empty combinations of slay flags */
		if (!of_is_empty(cacheme))
		{
			/* Skip previously scanned combinations */
			for (j = 0; j < i; j++)
			{
				if (of_is_equal(cacheme, dupcheck[j]))
					continue;
			}

			/* msg_print("Found a new slay combo on an ego item"); */
			count++;
			of_copy(dupcheck[i], cacheme);
		}
	}

	/* Allocate slay_cache with an extra empty element for an iteration stop */
	slay_cache = C_ZNEW((count + 1), flag_cache);
	count = 0;

	/* Populate the slay_cache */
	for (i = 0; i < z_info->e_max; i++)
	{
		if (!of_is_empty(dupcheck[i]))
		{
			of_copy(slay_cache[count].flags, dupcheck[i]);
			slay_cache[count].value = 0;
			count++;
			/*msg_print("Cached a slay combination");*/
		}
	}

	for (i = 0; i < z_info->e_max; i++)
		FREE(dupcheck[i]);
	FREE(dupcheck);

	/* Success */
	return 0;
}

/*
 * Emit a "template" file.
 *
 * This allows us to modify an existing template file by parsing it
 * in and then modifying the data structures.
 *
 * We parse the previous "template" file to allow us to include comments.
 */
errr emit_info_txt(ang_file *fp, ang_file *template, char *buf, header *head,
   emit_info_txt_index_func emit_info_txt_index, emit_info_txt_always_func emit_info_txt_always)
{
	errr err;

	/* Not ready yet */
	bool okay = FALSE;
	bool comment = FALSE;
	int blanklines = 0;

	/* Just before the first record */
	error_idx = -1;

	/* Just before the first line */
	error_line = 0;

	/* Parse */
	while (file_getl(template, buf, 1024))
	{
		/* Advance the line number */
		error_line++;

		/* Skip blank lines */
		if (!buf[0])
		{
			if (comment) blanklines++;
			continue;
		}

		/* Emit a comment line */
		if (buf[0] == '#')
		{
			/* Skip comments created by emission process */
			if ((buf[1] == '$') && (buf[2] == '#')) continue;

			while (blanklines--) file_put(fp, "\n");
			file_putf(fp, "%s\n", buf);
			comment = TRUE;
			blanklines = 0;
			continue;
		}

		/* Verify correct "colon" format */
		if (buf[1] != ':') return (PARSE_ERROR_GENERIC);

		/* Hack -- Process 'V' for "Version" */
		if (buf[0] == 'V')
		{
			if (comment) file_putf(fp,"\n");
			comment = FALSE;

			/* Output the version number */
			file_putf(fp, "\nV:%d.%d.%d\n\n", head->v_major, head->v_minor, head->v_patch);

			/* Okay to proceed */
			okay = TRUE;

			/* Continue */
			continue;
		}

		/* No version yet */
		if (!okay) return (PARSE_ERROR_OBSOLETE_FILE);

		/* Hack -- Process 'N' for "Name" */
		if ((emit_info_txt_index) && (buf[0] == 'N'))
		{
			int idx;

			idx = atoi(buf + 2);

			/* Verify index order */
			if (idx < ++error_idx) return (PARSE_ERROR_NON_SEQUENTIAL_RECORDS);

			/* Verify information */
			if (idx >= head->info_num) return (PARSE_ERROR_TOO_MANY_ENTRIES);

			if (comment) file_putf(fp,"\n");
			comment = FALSE;
			blanklines = 0;

			while (error_idx < idx)
			{
				file_putf(fp,"### %d - Unused ###\n\n",error_idx++);	
			}

			if ((err = (emit_info_txt_index(fp, head, idx))) != 0)
				return (err);
		}

		/* Ignore anything else and continue */
		continue;
	}

	/* No version yet */
	if (!okay) return (PARSE_ERROR_OBSOLETE_FILE);

	if ((emit_info_txt_always) && ((err = (emit_info_txt_always(fp, head))) != 0))
				return (err);

	/* Success */
	return (0);
}


/*
 * Emit one textual string based on a bitflag set.
 */
static errr emit_flags(ang_file *fp, cptr intro_text, bitflag *flags, const size_t size, const char **names)
{
	const int max_flags = FLAG_MAX(size);
	int i;
	bool intro = TRUE;
	int len = 0;

	/* Check flags */
	for (i = FLAG_START; i < max_flags; i++)
	{
		if (flag_has(flags, size, i))
		{
			/* Newline needed */
			if (len + strlen(names[i]) > 75)
			{
					file_putf(fp,"\n");
					len = 0;
					intro = TRUE;
			}
			
			/* Introduction needed */
			if (intro)
			{
				file_putf(fp, intro_text);
				len += strlen(intro_text);
				intro = FALSE;
			}
			else
			{
				file_putf(fp," ");
				len++;
			}
			
			/* Output flag */
			file_putf(fp, "%s |", names[i]);
			len += strlen(names[i]) + 2;
		}
	}

	/* Something output */
	if (!intro) file_putf(fp, "\n");

	return (0);
}


/*
 * Emit description to file.
 * 
 * TODO: Consider merging with text_out_to_file in util.c,
 * where most of this came from.
 */
static errr emit_desc(ang_file *fp, cptr intro_text, cptr text)
{	
	/* Current position on the line */
	int pos = 0;

	/* Wrap width */
	int wrap = 75 - strlen(intro_text);

	/* Current location within "str" */
	cptr s = text;
	
	/* Process the string */
	while (*s)
	{
		char ch;
		int n = 0;
		int len = wrap - pos;
		int l_space = 0;

		/* If we are at the start of the line... */
		if (pos == 0)
		{
			file_putf(fp, intro_text);
		}

		/* Find length of line up to next newline or end-of-string */
		while ((n < len) && !((s[n] == '\n') || (s[n] == '\0')))
		{
			/* Mark the most recent space in the string */
			if (s[n] == ' ') l_space = n;

			/* Increment */
			n++;
		}

		/* If we have encountered no spaces */
		if ((l_space == 0) && (n == len))
		{
			/* If we are at the start of a new line */
			if (pos == 0)
			{
				len = n;
			}
			else
			{
				/* Begin a new line */
				file_putf(fp, "\n");

				/* Reset */
				pos = 0;

				continue;
			}
		}
		else
		{
			/* Wrap at the newline */
			if ((s[n] == '\n') || (s[n] == '\0')) len = n;

			/* Wrap at the last space */
			else len = l_space;
		}

		/* Write that line to file */
		for (n = 0; n < len; n++)
		{
			/* Ensure the character is printable */
			ch = (isprint(s[n]) ? s[n] : ' ');

			/* Write out the character */
			file_putf(fp, "%c", ch);

			/* Increment */
			pos++;
		}

		/* Move 's' past the stuff we've written */
		s += len;

		/* Skip newlines */
		if (*s == '\n') s++;

		/* Begin a new line */
		file_put(fp, "\n");

		/* If we are at the end of the string, end */
		if (*s == '\0') return (0);

		/* Reset */
		pos = 0;
	}

	/* We are done */
	return (0);
}

/*
 * Emit the "r_info" array into an ascii "template" file
 */
errr emit_r_info_index(ang_file *fp, header *head, int i)
{
	int n;

	/* Current entry */
	monster_race *r_ptr = (monster_race *)head->info_ptr + i;


	/* Output 'N' for "New/Number/Name" */
	file_putf(fp, "N:%d:%s\n", i,head->name_ptr + r_ptr->name);

	/* Output 'G' for "Graphics" (one line only) */
	file_putf(fp, "G:%c:%c\n",r_ptr->d_char,color_table[r_ptr->d_attr].index_char);

	/* Output 'I' for "Info" (one line only) */
	file_putf(fp, "I:%d:%d:%d:%d:%d\n",r_ptr->speed,r_ptr->avg_hp,r_ptr->aaf,r_ptr->ac,r_ptr->sleep);

	/* Output 'W' for "More Info" (one line only) */
	file_putf(fp, "W:%d:%d:%d:%d\n",r_ptr->level, r_ptr->rarity, 0, r_ptr->mexp);

	/* Output 'B' for "Blows" (up to four lines) */
	for (n = 0; n < 4; n++)
	{
		/* End of blows */
		if (!r_ptr->blow[n].method) break;

		/* Output blow method */
		file_putf(fp, "B:%s", r_info_blow_method[r_ptr->blow[n].method]);

		/* Output blow effect */
		if (r_ptr->blow[n].effect)
		{
			file_putf(fp, ":%s", r_info_blow_effect[r_ptr->blow[n].effect]);

			/* Output blow damage if required */
			if ((r_ptr->blow[n].d_dice) && (r_ptr->blow[n].d_side))
			{
				file_putf(fp, ":%dd%d", r_ptr->blow[n].d_dice, r_ptr->blow[n].d_side);
			}
		}

		/* End line */
		file_putf(fp, "\n");
	}

	/* Output 'F' for "Flags" */
	emit_flags(fp, "F:", r_ptr->flags, RF_SIZE, r_info_flags);

	/* Output 'S' for "Spell Flags" (multiple lines) */
	emit_flags(fp, "S:", r_ptr->spell_flags, RSF_SIZE, r_info_spell_flags);

	/* Output 'S' for spell frequency in unwieldy format */
	/* TODO: use this routine to output M:freq_innate:freq_spell or similar to allow these to be
	 * specified properly. 'M' is for magic. Could be extended with :spell_power:mana for 4GAI.
	 *
	 * XXX Need to check for rounding errors here.
	 */
	if (r_ptr->freq_innate) file_putf(fp, "S:1_IN_%d\n",100/r_ptr->freq_innate);

	/* Output 'D' for "Description" */
	emit_desc(fp, "D:", head->text_ptr + r_ptr->text);

	file_putf(fp,"\n");

	/* Success */
	return (0);
}

