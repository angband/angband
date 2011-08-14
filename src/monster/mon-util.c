/*
 * File: mon-util.c
 * Purpose: Monster manipulation utilities.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "monster/mon-make.h"
#include "monster/mon-msg.h"
#include "monster/mon-spell.h"
#include "monster/mon-timed.h"
#include "monster/mon-util.h"
#include "squelch.h"

/**
 * Returns the r_idx of the monster with the given name. If no monster has
 * the exact name given, returns the r_idx of the first monster having the
 * given name as a (case-insensitive) substring.
 *
 * Returns -1 if no match is found.
 */
int lookup_monster(const char *name)
{
	int i;
	int r_idx = -1;
	
	/* Look for it */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Test for equality */
		if (r_ptr->name && streq(name, r_ptr->name))
			return i;
		
		/* Test for close matches */
		if (r_ptr->name && my_stristr(r_ptr->name, name) && r_idx == -1)
			r_idx = i;
	} 

	/* Return our best match */
	return r_idx;
}

/**
 * Return the monster base matching the given name.
 */
monster_base *lookup_monster_base(const char *name)
{
	monster_base *base;

	/* Look for it */
	for (base = rb_info; base; base = base->next) {
		if (streq(name, base->name))
			return base;
	}

	return NULL;
}

/**
 * Return whether the given base matches any of the names given.
 *
 * Accepts a variable-length list of name strings. The list must end with NULL.
 */
bool match_monster_bases(const monster_base *base, ...)
{
	bool ok = FALSE;
	va_list vp;
	char *name;

	va_start(vp, base);
	while (!ok && ((name = va_arg(vp, char *)) != NULL))
		ok = base == lookup_monster_base(name);
	va_end(vp);

	return ok;
}

/**
 * Mega-hack - Fix plural names of monsters
 *
 * Taken from PernAngband via EY, modified to fit NPP monster list
 *
 * Note: It should handle all regular Angband monsters.
 *
 * TODO: Specify monster name plurals in monster.txt instead.
 */
void plural_aux(char *name, size_t max)
{
	int name_len = strlen(name);

	if (strstr(name, " of "))
	{
		char *aider = strstr(name, " of ");
		char dummy[80];
		int i = 0;
		char *ctr = name;

		while (ctr < aider)
		{
			dummy[i] = *ctr;
			ctr++;
			i++;
		}

		if (dummy[i - 1] == 's')
		{
			strcpy (&(dummy[i]), "es");
			i++;
		}
		else
		{
			strcpy (&(dummy[i]), "s");
		}

		strcpy(&(dummy[i + 1]), aider);
		my_strcpy(name, dummy, max);
	}
	else if ((strstr(name, "coins")) || (strstr(name, "gems")))
	{
		char dummy[80];
		strcpy (dummy, "Piles of c");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}

	else if (strstr(name, "Greater Servant of"))
	{
		char dummy[80];
		strcpy (dummy, "Greater Servants of ");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}
	else if (strstr(name, "Lesser Servant of"))
	{
		char dummy[80];
		strcpy (dummy, "Greater Servants of ");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}
	else if (strstr(name, "Servant of"))
	{
		char dummy[80];
		strcpy (dummy, "Servants of ");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}
	else if (strstr(name, "Great Wyrm"))
	{
		char dummy[80];
		strcpy (dummy, "Great Wyrms ");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}
	else if (strstr(name, "Spawn of"))
	{
		char dummy[80];
		strcpy (dummy, "Spawn of ");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}
	else if (strstr(name, "Descendant of"))
	{
		char dummy[80];
		strcpy (dummy, "Descendant of ");
		my_strcat (dummy, &(name[1]), sizeof(dummy));
		my_strcpy (name, dummy, max);
		return;
	}
	else if ((strstr(name, "Manes")) || (name[name_len-1] == 'u') || (strstr(name, "Yeti")) ||
		(streq(&(name[name_len-2]), "ua")) || (streq(&(name[name_len-3]), "nee")) ||
		(streq(&(name[name_len-4]), "idhe")))
	{
		return;
	}
	else if (name[name_len-1] == 'y')
	{
		strcpy(&(name[name_len - 1]), "ies");
	}
	else if (streq(&(name[name_len - 4]), "ouse"))
	{
		strcpy (&(name[name_len - 4]), "ice");
	}
	else if (streq(&(name[name_len - 4]), "lung"))
	{
		strcpy (&(name[name_len - 4]), "lungen");
	}
	else if (streq(&(name[name_len - 3]), "sus"))
	{
		strcpy (&(name[name_len - 3]), "si");
	}
	else if (streq(&(name[name_len - 4]), "star"))
	{
		strcpy (&(name[name_len - 4]), "stari");
	}
	else if (streq(&(name[name_len - 3]), "aia"))
	{
		strcpy (&(name[name_len - 3]), "aiar");
	}
	else if (streq(&(name[name_len - 3]), "inu"))
	{
		strcpy (&(name[name_len - 3]), "inur");
	}
	else if (streq(&(name[name_len - 5]), "culus"))
	{
		strcpy (&(name[name_len - 5]), "culi");
	}
	else if (streq(&(name[name_len - 4]), "sman"))
	{
		strcpy (&(name[name_len - 4]), "smen");
	}
	else if (streq(&(name[name_len - 4]), "lman"))
	{
		strcpy (&(name[name_len - 4]), "lmen");
	}
	else if (streq(&(name[name_len - 2]), "ex"))
	{
		strcpy (&(name[name_len - 2]), "ices");
	}
	else if ((name[name_len - 1] == 'f') && (!streq(&(name[name_len - 2]), "ff")))
	{
		strcpy (&(name[name_len - 1]), "ves");
	}
	else if (((streq(&(name[name_len - 2]), "ch")) || (name[name_len - 1] == 's')) &&
			(!streq(&(name[name_len - 5]), "iarch")))
	{
		strcpy (&(name[name_len]), "es");
	}
	else
	{
		strcpy (&(name[name_len]), "s");
	}
}


/**
 * Helper function for display monlist.  Prints the number of creatures, followed
 * by either a singular or plural version of the race name as appropriate.
 */
static void get_mon_name(char *output_name, size_t max, 
		const monster_race *r_ptr, int num)
{
	char race_name[80];

	assert(r_ptr);

	my_strcpy(race_name, r_ptr->name, sizeof(race_name));

	/* Unique names don't have a number */
	if (rf_has(r_ptr->flags, RF_UNIQUE))
		my_strcpy(output_name, "[U] ", max);

	/* Normal races*/
	else {
		my_strcpy(output_name, format("%3d ", num), max);

		/* Make it plural, if needed. */
		if (num > 1)
			plural_aux(race_name, sizeof(race_name));
	}

	/* Mix the quantity and the header. */
	my_strcat(output_name, race_name, max);
}


/* 
 * Monster data for the visible monster list 
 */
typedef struct
{
	u16b count;		/* total number of this type visible */
	u16b asleep;		/* number asleep (not in LOS) */
	u16b los;		/* number in LOS */
	u16b los_asleep;	/* number asleep and in LOS */
	byte attr; /* attr to use for drawing */
} monster_vis; 

/*
 * Display visible monsters in a window
 */
void display_monlist(void)
{
	int ii;
	size_t i, j, k;
	int max;
	int line = 1, x = 0;
	int cur_x;
	unsigned total_count = 0, disp_count = 0, type_count = 0, los_count = 0;

	byte attr;

	char m_name[80];
	char buf[80];

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_race *r2_ptr;

	monster_vis *list;

	u16b *order;

	bool in_term = (Term != angband_term[0]);

	/* Hallucination is weird */
	if (p_ptr->timed[TMD_IMAGE]) {
		if (in_term)
			clear_from(0);
		Term_gotoxy(0, 0);
		text_out_to_screen(TERM_ORANGE,
			"Your hallucinations are too wild to see things clearly.");

		return;
	}

	/* Clear the term if in a subwindow, set x otherwise */
	if (in_term) {
		clear_from(0);
		max = Term->hgt - 1;
	}
	else {
		x = 13;
		max = Term->hgt - 2;
	}

	/* Allocate the primary array */
	list = C_ZNEW(z_info->r_max, monster_vis);

	/* Scan the list of monsters on the level */
	for (ii = 1; ii < cave_monster_max(cave); ii++) {
		monster_vis *v;

		m_ptr = cave_monster(cave, ii);
		r_ptr = &r_info[m_ptr->r_idx];

		/* Only consider visible, known monsters */
		if (!m_ptr->ml || m_ptr->unaware) continue;

		/* Take a pointer to this monster visibility entry */
		v = &list[m_ptr->r_idx];

		/* Note each monster type and save its display attr (color) */
		if (!v->count) type_count++;
		if (!v->attr) v->attr = m_ptr->attr ? m_ptr->attr : r_ptr->x_attr;
		
		/* Check for LOS
		 * Hack - we should use (m_ptr->mflag & (MFLAG_VIEW)) here,
		 * but this does not catch monsters detected by ESP which are
		 * targetable, so we cheat and use projectable() instead 
		 */
		if (projectable(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx,
			PROJECT_NONE))
		{
			/* Increment the total number of in-LOS monsters */
			los_count++;

			/* Increment the LOS count for this monster type */
			v->los++;
			
			/* Check if asleep and increment accordingly */
			if (m_ptr->m_timed[MON_TMD_SLEEP]) v->los_asleep++;
		}
		/* Not in LOS so increment if asleep */
		else if (m_ptr->m_timed[MON_TMD_SLEEP]) v->asleep++;

		/* Bump the count for this race, and the total count */
		v->count++;
		total_count++;
	}

	/* Note no visible monsters at all */
	if (!total_count)
	{
		/* Clear display and print note */
		c_prt(TERM_SLATE, "You see no monsters.", 0, 0);
		if (!in_term)
		    Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

		/* Free up memory */
		FREE(list);

		/* Done */
		return;
	}

	/* Allocate the secondary array */
	order = C_ZNEW(type_count, u16b);

	/* Sort, because we cannot rely on monster.txt being ordered */

	/* Populate the ordered array, starting at 1 to ignore @ */
	for (i = 1; i < z_info->r_max; i++)
	{
		/* No monsters of this race are visible */
		if (!list[i].count) continue;

		/* Get the monster info */
		r_ptr = &r_info[i];

		/* Fit this monster into the sorted array */
		for (j = 0; j < type_count; j++)
		{
			/* If we get to the end of the list, put this one in */
			if (!order[j])
			{
				order[j] = i;
				break;
			}

			/* Get the monster info for comparison */
			r2_ptr = &r_info[order[j]];

			/* Monsters are sorted by depth */
			/* Monsters of same depth are sorted by power */
			if ((r_ptr->level > r2_ptr->level) ||
				((r_ptr->level == r2_ptr->level) &&
				(r_ptr->power > r2_ptr->power)))
			{
				/* Move weaker monsters down the array */
				for (k = type_count - 1; k > j; k--)
				{
					order[k] = order[k - 1];
				}

				/* Put current monster in the right place */
				order[j] = i;
				break;
			}
		}
	}

	/* Message for monsters in LOS - even if there are none */
	if (!los_count) prt(format("You can see no monsters."), 0, 0);
	else prt(format("You can see %d monster%s", los_count, (los_count == 1
		? ":" : "s:")), 0, 0);

	/* Print out in-LOS monsters in descending order */
	for (i = 0; (i < type_count) && (line < max); i++)
	{
		/* Skip if there are none of these in LOS */
		if (!list[order[i]].los) continue;

		/* Reset position */
		cur_x = x;

		/* Note that these have been displayed */
		disp_count += list[order[i]].los;

		/* Get monster race and name */
		r_ptr = &r_info[order[i]];
		get_mon_name(m_name, sizeof(m_name), r_ptr, list[order[i]].los);

		/* Display uniques in a special colour */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
			attr = TERM_VIOLET;
		else if (r_ptr->level > p_ptr->depth)
			attr = TERM_RED;
		else
			attr = TERM_WHITE;

		/* Build the monster name */
		if (list[order[i]].los == 1)
			strnfmt(buf, sizeof(buf), (list[order[i]].los_asleep ==
			1 ? "%s (asleep) " : "%s "), m_name);
		else strnfmt(buf, sizeof(buf), (list[order[i]].los_asleep > 0 ?
			"%s (%d asleep) " : "%s"), m_name, list[order[i]].los_asleep);

		/* Display the pict */
		if ((tile_width == 1) && (tile_height == 1)) {
	        Term_putch(cur_x++, line, list[order[i]].attr, r_ptr->x_char);
			Term_putch(cur_x++, line, TERM_WHITE, ' ');
		}

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;

		/* Page wrap */
		if (!in_term && (line == max) && disp_count != total_count) {
			prt("-- more --", line, x);
			anykey();

			/* Clear the screen */
			for (line = 1; line <= max; line++)
				prt("", line, 0);

			/* Reprint Message */
			prt(format("You can see %d monster%s",
				los_count, (los_count > 0 ? (los_count == 1 ?
				":" : "s:") : "s.")), 0, 0);

			/* Reset */
			line = 1;
		}
	}

	/* Message for monsters outside LOS, if there are any */
	if (total_count > los_count) {
		/* Leave a blank line */
		line++;
		
		prt(format("You are aware of %d %smonster%s", 
		(total_count - los_count), (los_count > 0 ? "other " : ""), 
		((total_count - los_count) == 1 ? ":" : "s:")), line++, 0);
	}

	/* Print out non-LOS monsters in descending order */
	for (i = 0; (i < type_count) && (line < max); i++) {
		int out_of_los = list[order[i]].count - list[order[i]].los;

		/* Skip if there are none of these out of LOS */
		if (list[order[i]].count == list[order[i]].los) continue;

		/* Reset position */
		cur_x = x;

		/* Note that these have been displayed */
		disp_count += out_of_los;

		/* Get monster race and name */
		r_ptr = &r_info[order[i]];
		get_mon_name(m_name, sizeof(m_name), r_ptr, out_of_los);

		/* Display uniques in a special colour */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
			attr = TERM_VIOLET;
		else if (r_ptr->level > p_ptr->depth)
			attr = TERM_RED;
		else
			attr = TERM_WHITE;

		/* Build the monster name */
		if (out_of_los == 1)
			strnfmt(buf, sizeof(buf), (list[order[i]].asleep ==
			1 ? "%s (asleep) " : "%s "), m_name);
		else strnfmt(buf, sizeof(buf), (list[order[i]].asleep > 0 ? 
			"%s (%d asleep) " : "%s"), m_name,
			list[order[i]].asleep);

		/* Display the pict */
		if ((tile_width == 1) && (tile_height == 1)) {
	        Term_putch(cur_x++, line, list[order[i]].attr, r_ptr->x_char);
			Term_putch(cur_x++, line, TERM_WHITE, ' ');
		}

		/* Print and bump line counter */
		c_prt(attr, buf, line, cur_x);
		line++;

		/* Page wrap */
		if (!in_term && (line == max) && disp_count != total_count) {
			prt("-- more --", line, x);
			anykey();

			/* Clear the screen */
			for (line = 1; line <= max; line++)
				prt("", line, 0);

			/* Reprint Message */
			prt(format("You are aware of %d %smonster%s",
				(total_count - los_count), (los_count > 0 ?
				"other " : ""), ((total_count - los_count) > 0
				? ((total_count - los_count) == 1 ? ":" : "s:")
				: "s.")), 0, 0);

			/* Reset */
			line = 1;
		}
	}


	/* Print "and others" message if we've run out of space */
	if (disp_count != total_count) {
		strnfmt(buf, sizeof buf, "  ...and %d others.", total_count - disp_count);
		c_prt(TERM_WHITE, buf, line, x);
	}

	/* Otherwise clear a line at the end, for main-term display */
	else
		prt("", line, x);

	if (!in_term)
		Term_addstr(-1, TERM_WHITE, "  (Press any key to continue.)");

	/* Free the arrays */
	FREE(list);
	FREE(order);
}


/**
 * Builds a string describing a monster in some way.
 *
 * We can correctly describe monsters based on their visibility.
 * We can force all monsters to be treated as visible or invisible.
 * We can build nominatives, objectives, possessives, or reflexives.
 * We can selectively pronominalize hidden, visible, or all monsters.
 * We can use definite or indefinite descriptions for hidden monsters.
 * We can use definite or indefinite descriptions for visible monsters.
 *
 * Pronominalization involves the gender whenever possible and allowed,
 * so that by cleverly requesting pronominalization / visibility, you
 * can get messages like "You hit someone.  She screams in agony!".
 *
 * Reflexives are acquired by requesting Objective plus Possessive.
 *
 * I am assuming that no monster name is more than 65 characters long,
 * so that "char desc[80];" is sufficiently large for any result, even
 * when the "offscreen" notation is added.
 *
 * Note that the "possessive" for certain unique monsters will look
 * really silly, as in "Morgoth, King of Darkness's".  We should
 * perhaps add a flag to "remove" any "descriptives" in the name.
 *
 * Note that "offscreen" monsters will get a special "(offscreen)"
 * notation in their name if they are visible but offscreen.  This
 * may look silly with possessives, as in "the rat's (offscreen)".
 * Perhaps the "offscreen" descriptor should be abbreviated.
 *
 * Mode Flags:
 *   0x01 --> Objective (or Reflexive)
 *   0x02 --> Possessive (or Reflexive)
 *   0x04 --> Use indefinites for hidden monsters ("something")
 *   0x08 --> Use indefinites for visible monsters ("a kobold")
 *   0x10 --> Pronominalize hidden monsters
 *   0x20 --> Pronominalize visible monsters
 *   0x40 --> Assume the monster is hidden
 *   0x80 --> Assume the monster is visible
 *
 * Useful Modes:
 *   0x00 --> Full nominative name ("the kobold") or "it"
 *   0x04 --> Full nominative name ("the kobold") or "something"
 *   0x80 --> Banishment resistance name ("the kobold")
 *   0x88 --> Killing name ("a kobold")
 *   0x22 --> Possessive, genderized if visable ("his") or "its"
 *   0x23 --> Reflexive, genderized if visable ("himself") or "itself"
 */
void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode)
{
	const char *res;

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	const char *name = r_ptr->name;

	bool seen, pron;


	/* Can we "see" it (forced, or not hidden + visible) */
	seen = ((mode & (0x80)) || (!(mode & (0x40)) && m_ptr->ml));

	/* Sexed Pronouns (seen and forced, or unseen and allowed) */
	pron = ((seen && (mode & (0x20))) || (!seen && (mode & (0x10))));


	/* First, try using pronouns, or describing hidden monsters */
	if (!seen || pron)
	{
		/* an encoding of the monster "sex" */
		int kind = 0x00;

		/* Extract the gender (if applicable) */
		if (rf_has(r_ptr->flags, RF_FEMALE)) kind = 0x20;
		else if (rf_has(r_ptr->flags, RF_MALE)) kind = 0x10;

		/* Ignore the gender (if desired) */
		if (!m_ptr || !pron) kind = 0x00;


		/* Assume simple result */
		res = "it";

		/* Brute force: split on the possibilities */
		switch (kind + (mode & 0x07))
		{
			/* Neuter, or unknown */
			case 0x00: res = "it"; break;
			case 0x01: res = "it"; break;
			case 0x02: res = "its"; break;
			case 0x03: res = "itself"; break;
			case 0x04: res = "something"; break;
			case 0x05: res = "something"; break;
			case 0x06: res = "something's"; break;
			case 0x07: res = "itself"; break;

			/* Male (assume human if vague) */
			case 0x10: res = "he"; break;
			case 0x11: res = "him"; break;
			case 0x12: res = "his"; break;
			case 0x13: res = "himself"; break;
			case 0x14: res = "someone"; break;
			case 0x15: res = "someone"; break;
			case 0x16: res = "someone's"; break;
			case 0x17: res = "himself"; break;

			/* Female (assume human if vague) */
			case 0x20: res = "she"; break;
			case 0x21: res = "her"; break;
			case 0x22: res = "her"; break;
			case 0x23: res = "herself"; break;
			case 0x24: res = "someone"; break;
			case 0x25: res = "someone"; break;
			case 0x26: res = "someone's"; break;
			case 0x27: res = "herself"; break;
		}

		/* Copy the result */
		my_strcpy(desc, res, max);
	}


	/* Handle visible monsters, "reflexive" request */
	else if ((mode & 0x02) && (mode & 0x01))
	{
		/* The monster is visible, so use its gender */
		if (rf_has(r_ptr->flags, RF_FEMALE)) my_strcpy(desc, "herself", max);
		else if (rf_has(r_ptr->flags, RF_MALE)) my_strcpy(desc, "himself", max);
		else my_strcpy(desc, "itself", max);
	}


	/* Handle all other visible monster requests */
	else
	{
		/* It could be a Unique */
		if (rf_has(r_ptr->flags, RF_UNIQUE))
		{
			/* Start with the name (thus nominative and objective) */
			my_strcpy(desc, name, max);
		}

		/* It could be an indefinite monster */
		else if (mode & 0x08)
		{
			/* XXX Check plurality for "some" */

			/* Indefinite monsters need an indefinite article */
			my_strcpy(desc, is_a_vowel(name[0]) ? "an " : "a ", max);
			my_strcat(desc, name, max);
		}

		/* It could be a normal, definite, monster */
		else
		{
			/* Definite monsters need a definite article */
			my_strcpy(desc, "the ", max);
			my_strcat(desc, name, max);
		}

		/* Handle the Possessive as a special afterthought */
		if (mode & 0x02)
		{
			/* XXX Check for trailing "s" */

			/* Simply append "apostrophe" and "s" */
			my_strcat(desc, "'s", max);
		}

		/* Mention "offscreen" monsters XXX XXX */
		if (!panel_contains(m_ptr->fy, m_ptr->fx))
		{
			/* Append special notation */
			my_strcat(desc, " (offscreen)", max);
		}
	}
}



/**
 * This function updates the monster record of the given monster
 *
 * This involves extracting the distance to the player (if requested),
 * and then checking for visibility (natural, infravision, see-invis,
 * telepathy), updating the monster visibility flag, redrawing (or
 * erasing) the monster when its visibility changes, and taking note
 * of any interesting monster flags (cold-blooded, invisible, etc).
 *
 * Note the new "mflag" field which encodes several monster state flags,
 * including "view" for when the monster is currently in line of sight,
 * and "mark" for when the monster is currently visible via detection.
 *
 * The only monster fields that are changed here are "cdis" (the
 * distance from the player), "ml" (visible to the player), and
 * "mflag" (to maintain the "MFLAG_VIEW" flag).
 *
 * Note the special "update_monsters()" function which can be used to
 * call this function once for every monster.
 *
 * Note the "full" flag which requests that the "cdis" field be updated;
 * this is only needed when the monster (or the player) has moved.
 *
 * Every time a monster moves, we must call this function for that
 * monster, and update the distance, and the visibility.  Every time
 * the player moves, we must call this function for every monster, and
 * update the distance, and the visibility.  Whenever the player "state"
 * changes in certain ways ("blindness", "infravision", "telepathy",
 * and "see invisible"), we must call this function for every monster,
 * and update the visibility.
 *
 * Routines that change the "illumination" of a grid must also call this
 * function for any monster in that grid, since the "visibility" of some
 * monsters may be based on the illumination of their grid.
 *
 * Note that this function is called once per monster every time the
 * player moves.  When the player is running, this function is one
 * of the primary bottlenecks, along with "update_view()" and the
 * "process_monsters()" code, so efficiency is important.
 *
 * Note the optimized "inline" version of the "distance()" function.
 *
 * A monster is "visible" to the player if (1) it has been detected
 * by the player, (2) it is close to the player and the player has
 * telepathy, or (3) it is close to the player, and in line of sight
 * of the player, and it is "illuminated" by some combination of
 * infravision, torch light, or permanent light (invisible monsters
 * are only affected by "light" if the player can see invisible).
 *
 * Monsters which are not on the current panel may be "visible" to
 * the player, and their descriptions will include an "offscreen"
 * reference.  Currently, offscreen monsters cannot be targeted
 * or viewed directly, but old targets will remain set.  XXX XXX
 *
 * The player can choose to be disturbed by several things, including
 * "OPT(disturb_move)" (monster which is viewable moves in some way), and
 * "OPT(disturb_near)" (monster which is "easily" viewable moves in some
 * way).  Note that "moves" includes "appears" and "disappears".
 */
void update_mon(int m_idx, bool full)
{
	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;

	int d;

	/* Current location */
	int fy, fx;

	/* Seen at all */
	bool flag = FALSE;

	/* Seen by vision */
	bool easy = FALSE;

	assert(m_idx > 0);
	m_ptr = cave_monster(cave, m_idx);
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];
	
	fy = m_ptr->fy;
	fx = m_ptr->fx;

	/* Compute distance */
	if (full) {
		int py = p_ptr->py;
		int px = p_ptr->px;

		/* Distance components */
		int dy = (py > fy) ? (py - fy) : (fy - py);
		int dx = (px > fx) ? (px - fx) : (fx - px);

		/* Approximate distance */
		d = (dy > dx) ? (dy + (dx>>1)) : (dx + (dy>>1));

		/* Restrict distance */
		if (d > 255) d = 255;

		/* Save the distance */
		m_ptr->cdis = d;
	}

	/* Extract distance */
	else {
		/* Extract the distance */
		d = m_ptr->cdis;
	}

	/* Detected */
	if (m_ptr->mflag & (MFLAG_MARK)) flag = TRUE;

	/* Nearby */
	if (d <= MAX_SIGHT) {
		/* Basic telepathy */
		if (check_state(p_ptr, OF_TELEPATHY, p_ptr->state.flags)) {
			/* Empty mind, no telepathy */
			if (rf_has(r_ptr->flags, RF_EMPTY_MIND))
			{
				/* Nothing! */
			}

			/* Weird mind, occasional telepathy */
			else if (rf_has(r_ptr->flags, RF_WEIRD_MIND)) {
				/* One in ten individuals are detectable */
				if ((m_idx % 10) == 5) {
					/* Detectable */
					flag = TRUE;

					/* Check for LOS so that MFLAG_VIEW is set later */
					if (player_has_los_bold(fy, fx)) easy = TRUE;
				}
			}

			/* Normal mind, allow telepathy */
			else {
				/* Detectable */
				flag = TRUE;

				/* Check for LOS to that MFLAG_VIEW is set later */
				if (player_has_los_bold(fy, fx)) easy = TRUE;
			}
		}

		/* Normal line of sight and player is not blind */
		if (player_has_los_bold(fy, fx) && !p_ptr->timed[TMD_BLIND]) {
			/* Use "infravision" */
			if (d <= p_ptr->state.see_infra) {
				/* Learn about warm/cold blood */
				rf_on(l_ptr->flags, RF_COLD_BLOOD);

				/* Handle "warm blooded" monsters */
				if (!rf_has(r_ptr->flags, RF_COLD_BLOOD)) {
					/* Easy to see */
					easy = flag = TRUE;
				}
			}

			/* See if the monster is emitting light */
			/*if (rf_has(r_ptr->flags, RF_HAS_LIGHT)) easy = flag = TRUE;*/

			/* Use "illumination" */
			if (player_can_see_bold(fy, fx)) {
				/* Learn it emits light */
				rf_on(l_ptr->flags, RF_HAS_LIGHT);

				/* Learn about invisibility */
				rf_on(l_ptr->flags, RF_INVISIBLE);

				/* Handle "invisible" monsters */
				if (rf_has(r_ptr->flags, RF_INVISIBLE)) {
					/* See invisible */
					if (check_state(p_ptr, OF_SEE_INVIS, p_ptr->state.flags))
					{
						/* Easy to see */
						easy = flag = TRUE;
					}
				}

				/* Handle "normal" monsters */
				else {
					/* Easy to see */
					easy = flag = TRUE;
				}
			}
		}
	}

	/* If a mimic looks like a squelched item, it's not seen */
	if (is_mimicking(m_idx)) {
		object_type *o_ptr = object_byid(m_ptr->mimicked_o_idx);
		if (squelch_item_ok(o_ptr))
			easy = flag = FALSE;
	}
	
	/* The monster is now visible */
	if (flag) {
		/* Learn about the monster's mind */
		if (check_state(p_ptr, OF_TELEPATHY, p_ptr->state.flags))
			flags_set(l_ptr->flags, RF_SIZE, RF_EMPTY_MIND, RF_WEIRD_MIND,
					RF_SMART, RF_STUPID, FLAG_END);

		/* It was previously unseen */
		if (!m_ptr->ml) {
			/* Mark as visible */
			m_ptr->ml = TRUE;

			/* Draw the monster */
			cave_light_spot(cave, fy, fx);

			/* Update health bar as needed */
			if (p_ptr->health_who == m_idx)
				p_ptr->redraw |= (PR_HEALTH);

			/* Hack -- Count "fresh" sightings */
			if (l_ptr->sights < MAX_SHORT)
				l_ptr->sights++;

			/* Disturb on appearance */
			if (OPT(disturb_move))
				disturb(p_ptr, 1, 0);

			/* Window stuff */
			p_ptr->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not visible */
	else {
		/* It was previously seen */
		if (m_ptr->ml) {
			/* Treat mimics differently */
			if (!m_ptr->mimicked_o_idx || 
					squelch_item_ok(object_byid(m_ptr->mimicked_o_idx)))
			{
				/* Mark as not visible */
				m_ptr->ml = FALSE;

				/* Erase the monster */
				cave_light_spot(cave, fy, fx);

				/* Update health bar as needed */
				if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

				/* Disturb on disappearance */
				if (OPT(disturb_move)) disturb(p_ptr, 1, 0);

				/* Window stuff */
				p_ptr->redraw |= PR_MONLIST;
			}
		}
	}


	/* The monster is now easily visible */
	if (easy) {
		/* Change */
		if (!(m_ptr->mflag & (MFLAG_VIEW))) {
			/* Mark as easily visible */
			m_ptr->mflag |= (MFLAG_VIEW);

			/* Disturb on appearance */
			if (OPT(disturb_near)) disturb(p_ptr, 1, 0);

			/* Re-draw monster window */
			p_ptr->redraw |= PR_MONLIST;
		}
	}

	/* The monster is not easily visible */
	else {
		/* Change */
		if (m_ptr->mflag & (MFLAG_VIEW)) {
			/* Mark as not easily visible */
			m_ptr->mflag &= ~(MFLAG_VIEW);

			/* Disturb on disappearance */
			if (OPT(disturb_near)) disturb(p_ptr, 1, 0);

			/* Re-draw monster list window */
			p_ptr->redraw |= PR_MONLIST;
		}
	}
}




/**
 * Updates all the (non-dead) monsters via update_mon().
 */
void update_monsters(bool full)
{
	int i;

	/* Update each (live) monster */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Update the monster */
		update_mon(i, full);
	}
}


/**
 * Add the given object to the given monster's inventory.
 *
 * Returns the o_idx of the new object, or 0 if the object is
 * not successfully added.
 */
s16b monster_carry(struct monster *m_ptr, object_type *j_ptr)
{
	s16b o_idx;

	s16b this_o_idx, next_o_idx = 0;

	/* Scan objects already being held for combination */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = object_byid(this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Check for combination */
		if (object_similar(o_ptr, j_ptr, OSTACK_MONSTER)) {
			/* Combine the items */
			object_absorb(o_ptr, j_ptr);

			/* Result */
			return (this_o_idx);
		}
	}


	/* Make an object */
	o_idx = o_pop();

	/* Success */
	if (o_idx) {
		object_type *o_ptr;

		/* Get new object */
		o_ptr = object_byid(o_idx);

		/* Copy object */
		object_copy(o_ptr, j_ptr);

		/* Forget mark */
		o_ptr->marked = FALSE;

		/* Forget location */
		o_ptr->iy = o_ptr->ix = 0;

		/* Link the object to the monster */
		o_ptr->held_m_idx = m_ptr->midx;

		/* Link the object to the pile */
		o_ptr->next_o_idx = m_ptr->hold_o_idx;

		/* Link the monster to the object */
		m_ptr->hold_o_idx = o_idx;
	}

	/* Result */
	return (o_idx);
}

/**
 * Swap the players/monsters (if any) at two locations.
 */
void monster_swap(int y1, int x1, int y2, int x2)
{
	int m1, m2;

	monster_type *m_ptr;

	monster_race *r_ptr;

	/* Monsters */
	m1 = cave->m_idx[y1][x1];
	m2 = cave->m_idx[y2][x2];

	/* Update grids */
	cave->m_idx[y1][x1] = m2;
	cave->m_idx[y2][x2] = m1;

	/* Monster 1 */
	if (m1 > 0) {
		m_ptr = cave_monster(cave, m1);

		/* Move monster */
		m_ptr->fy = y2;
		m_ptr->fx = x2;

		/* Update monster */
		update_mon(m1, TRUE);

		/* Radiate light? */
		r_ptr = &r_info[m_ptr->r_idx];
		if (rf_has(r_ptr->flags, RF_HAS_LIGHT)) p_ptr->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Player 1 */
	else if (m1 < 0) {
		/* Move player */
		p_ptr->py = y2;
		p_ptr->px = x2;

		/* Update the trap detection status */
		p_ptr->redraw |= (PR_DTRAP);

		/* Update the panel */
		p_ptr->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_UPDATE_FLOW);

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Monster 2 */
	if (m2 > 0) {
		m_ptr = cave_monster(cave, m2);

		/* Move monster */
		m_ptr->fy = y1;
		m_ptr->fx = x1;

		/* Update monster */
		update_mon(m2, TRUE);

		/* Radiate light? */
		r_ptr = &r_info[m_ptr->r_idx];
		if (rf_has(r_ptr->flags, RF_HAS_LIGHT)) p_ptr->update |= PU_UPDATE_VIEW;

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Player 2 */
	else if (m2 < 0) {
		/* Move player */
		p_ptr->py = y1;
		p_ptr->px = x1;

		/* Update the trap detection status */
		p_ptr->redraw |= (PR_DTRAP);

		/* Update the panel */
		p_ptr->update |= (PU_PANEL);

		/* Update the visuals (and monster distances) */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_DISTANCE);

		/* Update the flow */
		p_ptr->update |= (PU_UPDATE_FLOW);

		/* Redraw monster list */
		p_ptr->redraw |= (PR_MONLIST);
	}

	/* Redraw */
	cave_light_spot(cave, y1, x1);
	cave_light_spot(cave, y2, x2);
}

/*
 * Hack -- the "type" of the current "summon specific"
 */
static int summon_specific_type = 0;


/**
 * Hack -- help decide if a monster race is "okay" to summon.
 *
 * Compares the given monster to the monster type specified by
 * summon_specific_type. Returns TRUE if the monster is eligible to
 * be summoned, FALSE otherwise. 
 */
static bool summon_specific_okay(int r_idx)
{
	const monster_race *r_ptr;
	const bitflag *flags;
	const struct monster_base *base;
	
	bool unique, scary;

	assert(r_idx > 0);
	r_ptr = &r_info[r_idx];

	flags = r_ptr->flags;
	base = r_ptr->base;
	
	unique = rf_has(flags, RF_UNIQUE);
	scary = flags_test(flags, RF_SIZE, RF_UNIQUE, RF_FRIEND, RF_FRIENDS,
			RF_ESCORT, RF_ESCORTS, FLAG_END);

	/* Check our requirements */
	switch (summon_specific_type)
	{
		case S_ANIMAL: return !unique && rf_has(flags, RF_ANIMAL);
		case S_SPIDER: return !unique && match_monster_bases(base, "spider", NULL);
		case S_HOUND: return !unique && match_monster_bases(base, "canine", "zephyr hound", NULL);
		case S_HYDRA: return !unique && match_monster_bases(base, "hydra", NULL);
		case S_ANGEL: return !scary && match_monster_bases(base, "angel", NULL);
		case S_DEMON: return !scary && rf_has(flags, RF_DEMON);
		case S_UNDEAD: return !scary && rf_has(flags, RF_UNDEAD);
		case S_DRAGON: return !scary && rf_has(flags, RF_DRAGON);
		case S_KIN: return !unique && r_ptr->d_char == summon_kin_type;
		case S_HI_UNDEAD: return match_monster_bases(base, "lich", "vampire", "wraith", NULL);
		case S_HI_DRAGON: return match_monster_bases(base, "ancient dragon", NULL);
		case S_HI_DEMON: return match_monster_bases(base, "major demon", NULL);
		case S_WRAITH: return unique && match_monster_bases(base, "wraith", NULL);
		case S_UNIQUE: return unique;
		case S_MONSTER: return !scary;
		case S_MONSTERS: return !unique;

		default: return TRUE;
	}
}


/**
 * Places a monster (of the specified "type") near the given
 * location.  Return TRUE iff a monster was actually summoned.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: S_UNIQUE and S_WRAITH (XXX) will summon Uniques
 * Note: S_HI_UNDEAD and S_HI_DRAGON may summon Uniques
 * Note: None of the other summon codes will ever summon Uniques.
 *
 * This function has been changed.  We now take the "monster level"
 * of the summoning monster as a parameter, and use that, along with
 * the current dungeon level, to help determine the level of the
 * desired monster.  Note that this is an upper bound, and also
 * tends to "prefer" monsters of that level.  Currently, we use
 * the average of the dungeon and monster levels, and then add
 * five to allow slight increases in monster power.
 *
 * Note that we use the new "monster allocation table" creation code
 * to restrict the "get_mon_num()" function to the set of "legal"
 * monsters, making this function much faster and more reliable.
 *
 * Note that this function may not succeed, though this is very rare.
 */
int summon_specific(int y1, int x1, int lev, int type, int delay)
{
	int i, x = 0, y = 0, r_idx;
	int temp = 1;

	monster_type *m_ptr;
	monster_race *r_ptr;

	/* Look for a location, allow up to 4 squares away */
	for (i = 0; i < 60; ++i)
	{
		/* Pick a distance */
		int d = (i / 15) + 1;

		/* Pick a location */
		scatter(&y, &x, y1, x1, d, 0);

		/* Require "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Hack -- no summon on glyph of warding */
		if (cave->feat[y][x] == FEAT_GLYPH) continue;

		/* Okay */
		break;
	}

	/* Failure */
	if (i == 20) return (0);

	/* Save the "summon" type */
	summon_specific_type = type;

	/* Require "okay" monsters */
	get_mon_num_hook = summon_specific_okay;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Pick a monster, using the level calculation */
	r_idx = get_mon_num((p_ptr->depth + lev) / 2 + 5);

	/* Remove restriction */
	get_mon_num_hook = NULL;

	/* Prepare allocation table */
	get_mon_num_prep();

	/* Handle failure */
	if (!r_idx) return (0);

	/* Attempt to place the monster (awake, don't allow groups) */
	if (!place_new_monster(cave, y, x, r_idx, FALSE, FALSE, ORIGIN_DROP_SUMMON))
		return (0);

	/* If delay, try to let the player act before the summoned monsters. */
	/* NOTE: should really be -100, but energy is currently 0-255. */
	if (delay)
		cave_monster(cave, cave->m_idx[y][x])->energy = 0;

	/* Success, return the level of the monster */
	m_ptr = cave_monster(cave, cave->m_idx[y][x]);
	r_ptr = &r_info[m_ptr->r_idx];

	/* Monsters that normally come with FRIENDS are weaker */
	if (rf_has(r_ptr->flags, RF_FRIENDS))
		temp = 5;

	return (r_ptr->level / temp);
}

/**
 * Lets the given monster attempt to reproduce.
 *
 * Note that "reproduction" REQUIRES empty space.
 *
 * Returns TRUE if the monster successfully reproduced.
 */
bool multiply_monster(int m_idx)
{
	const monster_type *m_ptr;

	int i, y, x;

	bool result = FALSE;

	assert(m_idx > 0);
	m_ptr = cave_monster(cave, m_idx);

	/* Try up to 18 times */
	for (i = 0; i < 18; i++) {
		int d = 1;

		/* Pick a location */
		scatter(&y, &x, m_ptr->fy, m_ptr->fx, d, 0);

		/* Require an "empty" floor grid */
		if (!cave_empty_bold(y, x)) continue;

		/* Create a new monster (awake, no groups) */
		result = place_new_monster(cave, y, x, m_ptr->r_idx, FALSE, FALSE,
			ORIGIN_DROP_BREED);

		/* Done */
		break;
	}

	/* Result */
	return (result);
}


/**
 * Make player fully aware of the given mimic.
 *
 * When a player becomes aware of a mimic, we update the monster memory
 * and delete the "fake item" that the monster was mimicking.
 */
void become_aware(int m_idx)
{
	monster_type *m_ptr;
	const monster_race *r_ptr;
	monster_lore *l_ptr;

	assert(m_idx > 0);
	m_ptr = cave_monster(cave, m_idx);
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_list[m_ptr->r_idx];

	if (m_ptr->unaware) {
		m_ptr->unaware = FALSE;

		/* Learn about mimicry */
		if (rf_has(r_ptr->flags, RF_UNAWARE))
			rf_on(l_ptr->flags, RF_UNAWARE);

		/* Delete any false items */
		if (m_ptr->mimicked_o_idx > 0) {
			object_type *o_ptr = object_byid(m_ptr->mimicked_o_idx);
			char o_name[80];
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

			/* Print a message */
			msg("The %s was really a monster!", o_name);

			/* Clear the mimicry */
			o_ptr->mimicking_m_idx = 0;

			/* Give the object to the monster if appropriate */
			if (rf_has(r_ptr->flags, RF_MIMIC_INV)) {
				object_type *i_ptr;
				object_type object_type_body;
				
				/* Get local object */
				i_ptr = &object_type_body;

				/* Obtain local object */
				object_copy(i_ptr, o_ptr);

				/* Carry the object */
				monster_carry(m_ptr, i_ptr);
			}
				
			/* Delete the mimicked object */
			delete_object_idx(m_ptr->mimicked_o_idx);
			m_ptr->mimicked_o_idx = 0;
		}
		
		/* Update monster and item lists */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
		p_ptr->redraw |= (PR_MONLIST | PR_ITEMLIST);
	}
}

/**
 * Returns TRUE if the given monster is currently mimicking an item.
 */
bool is_mimicking(int m_idx)
{
	const monster_type *m_ptr;
	
	assert(m_idx > 0);
	m_ptr = cave_monster(cave, m_idx);	

	return (m_ptr->unaware && m_ptr->mimicked_o_idx);
}


/**
 * The given monster learns about an "observed" resistance or other player
 * state property, or lack of it.
 */
void update_smart_learn(struct monster *m, struct player *p, int flag)
{
	monster_race *r_ptr = &r_info[m->r_idx];

	/* Sanity check */
	if (!flag) return;

	/* anything a monster might learn, the player should learn */
	wieldeds_notice_flag(p, flag);

	/* Not allowed to learn */
	if (!OPT(birth_ai_learn)) return;

	/* Too stupid to learn anything */
	if (rf_has(r_ptr->flags, RF_STUPID)) return;

	/* Not intelligent, only learn sometimes */
	if (!rf_has(r_ptr->flags, RF_SMART) && one_in_(2)) return;

	/* Analyze the knowledge; fail very rarely */
	if (check_state(p, flag, p->state.flags) && !one_in_(100))
		of_on(m->known_pflags, flag);
	else
		of_off(m->known_pflags, flag);
}

