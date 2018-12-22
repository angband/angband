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
#include "mon-predicate.h"

/**
 * Perform simple English pluralization on a monster name.
 */
void plural_aux(char *name, size_t max)
{
	size_t name_len = strlen(name);
	assert(name_len != 0);

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
void get_mon_name(char *buf, size_t buflen,
				  const struct monster_race *race, int num)
{
	assert(race != NULL);

    /* Unique names don't have a number */
	if (rf_has(race->flags, RF_UNIQUE)) {
		strnfmt(buf, buflen, "[U] %s", race->name);
    } else {
	    strnfmt(buf, buflen, "%3d ", num);

	    if (num == 1) {
	        my_strcat(buf, race->name, buflen);
	    } else if (race->plural != NULL) {
	        my_strcat(buf, race->plural, buflen);
	    } else {
	        my_strcat(buf, race->name, buflen);
	        plural_aux(buf, buflen);
	    }
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
	assert(mon != NULL);

	/* Can we see it? (forced, or not hidden + visible) */
	bool seen = (mode & MDESC_SHOW) ||
		(!(mode & MDESC_HIDE) && monster_is_visible(mon));

	/* Sexed pronouns (seen and forced, or unseen and allowed) */
	bool use_pronoun = (seen && (mode & MDESC_PRO_VIS)) ||
			(!seen && (mode & MDESC_PRO_HID));

	/* First, try using pronouns, or describing hidden monsters */
	if (!seen || use_pronoun) {
		const char *choice = "it";

		/* an encoding of the monster "sex" */
		int msex = 0x00;

		/* Extract the gender (if applicable) */
		if (use_pronoun) {
			if (rf_has(mon->race->flags, RF_FEMALE)) {
				msex = 0x20;
			} else if (rf_has(mon->race->flags, RF_MALE)) {
				msex = 0x10;
			}
		}

		/* Brute force: split on the possibilities */
		switch (msex + (mode & 0x07)) {
			/* Neuter */
			case 0x00: choice = "it"; break;
			case 0x01: choice = "it"; break;
			case 0x02: choice = "its"; break;
			case 0x03: choice = "itself"; break;
			case 0x04: choice = "something"; break;
			case 0x05: choice = "something"; break;
			case 0x06: choice = "something's"; break;
			case 0x07: choice = "itself"; break;

			/* Male */
			case 0x10: choice = "he"; break;
			case 0x11: choice = "him"; break;
			case 0x12: choice = "his"; break;
			case 0x13: choice = "himself"; break;
			case 0x14: choice = "someone"; break;
			case 0x15: choice = "someone"; break;
			case 0x16: choice = "someone's"; break;
			case 0x17: choice = "himself"; break;

			/* Female */
			case 0x20: choice = "she"; break;
			case 0x21: choice = "her"; break;
			case 0x22: choice = "her"; break;
			case 0x23: choice = "herself"; break;
			case 0x24: choice = "someone"; break;
			case 0x25: choice = "someone"; break;
			case 0x26: choice = "someone's"; break;
			case 0x27: choice = "herself"; break;
		}

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
		} else {
			if (mode & MDESC_IND_VIS) {
				/* XXX Check plurality for "some" */
				/* Indefinite monsters need an indefinite article */
				my_strcpy(desc, is_a_vowel(mon->race->name[0]) ? "an " : "a ", max);
			} else {
				/* Definite monsters need a definite article */
				my_strcpy(desc, "the ", max);
			}

			my_strcat(desc, mon->race->name, max);
		}

		/* Handle the possessive */
		/* XXX Check for trailing "s" */
		if (mode & MDESC_POSS) {
			my_strcat(desc, "'s", max);
		}

		/* Mention "offscreen" monsters */
		if (!panel_contains(mon->grid.y, mon->grid.x)) {
			my_strcat(desc, " (offscreen)", max);
		}
	}

	if (mode & MDESC_CAPITAL) {
		my_strcap(desc);
	}
}
