/**
 * \file mon-desc.c
 * \brief Monster description
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
#include "game-input.h"
#include "mon-desc.h"

/**
 * Perform simple English pluralization on a monster name.
 */
void plural_aux(char *name, size_t max)
{
	unsigned long name_len = strlen(name);

	if (name[name_len - 1] == 's')
		my_strcat(name, "es", max);
	else
		my_strcat(name, "s", max);
}


/**
 * Helper function for display monlist.  Prints the number of creatures,
 * followed by either a singular or plural version of the race name as
 * appropriate.
 */
void get_mon_name(char *output_name, size_t max,
				  const struct monster_race *race, int num)
{
	assert(race);

    /* Unique names don't have a number */
	if (rf_has(race->flags, RF_UNIQUE)) {
		my_strcpy(output_name, "[U] ", max);
        my_strcat(output_name, race->name, max);
        return;
    }

    my_strcpy(output_name, format("%3d ", num), max);

    if (num == 1) {
        my_strcat(output_name, race->name, max);
        return;
    }

    if (race->plural != NULL) {
        my_strcat(output_name, race->plural, max);
    } else {
        char race_name[80];
		my_strcpy(race_name, race->name, sizeof(race_name));
        plural_aux(race_name, sizeof(race_name));
        my_strcat(output_name, race_name, max);
    }
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
 *  0x100 --> Capitalise monster name
 *
 * Useful Modes:
 *   0x00 --> Full nominative name ("the kobold") or "it"
 *   0x04 --> Full nominative name ("the kobold") or "something"
 *   0x80 --> Banishment resistance name ("the kobold")
 *   0x88 --> Killing name ("a kobold")
 *   0x22 --> Possessive, genderized if visable ("his") or "its"
 *   0x23 --> Reflexive, genderized if visable ("himself") or "itself"
 */
void monster_desc(char *desc, size_t max, const struct monster *mon, int mode)
{
	const char *choice;
	bool seen, use_pronoun;

	assert(mon);


	/* Can we "see" it (forced, or not hidden + visible) */
	seen = ((mode & (MDESC_SHOW)) || (!(mode & (MDESC_HIDE)) &&
									  mflag_has(mon->mflag, MFLAG_VISIBLE)));

	/* Sexed Pronouns (seen and forced, or unseen and allowed) */
	use_pronoun = ((seen && (mode & (MDESC_PRO_VIS))) ||
				   (!seen && (mode & (MDESC_PRO_HID))));


	/* First, try using pronouns, or describing hidden monsters */
	if (!seen || use_pronoun) {
		/* an encoding of the monster "sex" */
		int msex = 0x00;

		/* Extract the gender (if applicable) */
		if (rf_has(mon->race->flags, RF_FEMALE)) msex = 0x20;
		else if (rf_has(mon->race->flags, RF_MALE)) msex = 0x10;

		/* Ignore the gender (if desired) */
		if (!mon || !use_pronoun) msex = 0x00;


		/* Assume simple result */
		choice = "it";

		/* Brute force: split on the possibilities */
		switch (msex + (mode & 0x07)) {
			/* Neuter, or unknown */
			case 0x00: choice = "it"; break;
			case 0x01: choice = "it"; break;
			case 0x02: choice = "its"; break;
			case 0x03: choice = "itself"; break;
			case 0x04: choice = "something"; break;
			case 0x05: choice = "something"; break;
			case 0x06: choice = "something's"; break;
			case 0x07: choice = "itself"; break;

			/* Male (assume human if vague) */
			case 0x10: choice = "he"; break;
			case 0x11: choice = "him"; break;
			case 0x12: choice = "his"; break;
			case 0x13: choice = "himself"; break;
			case 0x14: choice = "someone"; break;
			case 0x15: choice = "someone"; break;
			case 0x16: choice = "someone's"; break;
			case 0x17: choice = "himself"; break;

			/* Female (assume human if vague) */
			case 0x20: choice = "she"; break;
			case 0x21: choice = "her"; break;
			case 0x22: choice = "her"; break;
			case 0x23: choice = "herself"; break;
			case 0x24: choice = "someone"; break;
			case 0x25: choice = "someone"; break;
			case 0x26: choice = "someone's"; break;
			case 0x27: choice = "herself"; break;
		}

		/* Copy the result */
		my_strcpy(desc, choice, max);
	} else if ((mode & MDESC_POSS) && (mode & MDESC_OBJE)) {
		/* The monster is visible, so use its gender */
		if (rf_has(mon->race->flags, RF_FEMALE))
			my_strcpy(desc, "herself", max);
		else if (rf_has(mon->race->flags, RF_MALE))
			my_strcpy(desc, "himself", max);
		else
			my_strcpy(desc, "itself", max);
	} else {
		/* Unique, indefinite or definite */
		if (rf_has(mon->race->flags, RF_UNIQUE)) {
			/* Start with the name (thus nominative and objective) */
			my_strcpy(desc, mon->race->name, max);
		} else if (mode & MDESC_IND_VIS) {
			/* XXX Check plurality for "some" */

			/* Indefinite monsters need an indefinite article */
			my_strcpy(desc, is_a_vowel(mon->race->name[0]) ? "an " : "a ", max);
			my_strcat(desc, mon->race->name, max);
		} else {
			/* Definite monsters need a definite article */
			my_strcpy(desc, "the ", max);
			my_strcat(desc, mon->race->name, max);
		}

		/* Handle the Possessive as a special afterthought */
		if (mode & MDESC_POSS) {
			/* XXX Check for trailing "s" */

			/* Simply append "apostrophe" and "s" */
			my_strcat(desc, "'s", max);
		}

		/* Mention "offscreen" monsters XXX XXX */
		if (!panel_contains(mon->fy, mon->fx)) {
			/* Append special notation */
			my_strcat(desc, " (offscreen)", max);
		}
	}

	if (mode & MDESC_CAPITAL)
		my_strcap(desc);
}
