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
#include "parser.h"


/*** Helper arrays for parsing ascii template files ***/

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





/*
 * Go through the attack types for this monster.
 * We look for the maximum possible maximum damage that this
 * monster can inflict in 10 game turns.
 *
 * We try to scale this based on assumed resists,
 * chance of casting spells and of spells failing,
 * chance of hitting in melee, and particularly speed.
 */


/* Evaluate and adjust a monsters hit points for how easily the monster is damaged */


/*
 * Evaluate the monster power ratings to be stored in r_info.raw
 */

/*
 * Create the slay cache by determining the number of different slay
 * combinations available to ego items
 */

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
	file_putf(fp, "N:%d:%s\n", i, r_ptr->name);

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
	emit_desc(fp, "D:", r_ptr->text);

	file_putf(fp,"\n");

	/* Success */
	return (0);
}

