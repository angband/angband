/**
 * \file ui-mon-lore.c
 * \brief Monster memory UI
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
#include "init.h"
#include "mon-lore.h"
#include "ui-mon-lore.h"
#include "ui-output.h"
#include "ui-prefs.h"
#include "ui-term.h"
#include "z-textblock.h"

/**
 * Place a monster recall title into a textblock.
 *
 * If graphics are turned on, this appends the title with the appropriate tile.
 * Note: if the title is the only thing in the textblock, make sure to append a
 * newline so that the textui stuff works properly. 
 *
 * \param tb is the textblock we are placing the title into.
 * \param race is the monster race we are describing.
 */
void lore_title(textblock *tb, const struct monster_race *race)
{
	byte standard_attr, optional_attr;
	wchar_t standard_char, optional_char;

	assert(race);

	/* Get the chars */
	standard_char = race->d_char;
	optional_char = monster_x_char[race->ridx];

	/* Get the attrs */
	standard_attr = race->d_attr;
	optional_attr = monster_x_attr[race->ridx];

	/* A title (use "The" for non-uniques) */
	if (!rf_has(race->flags, RF_UNIQUE))
		textblock_append(tb, "The ");
	else if (OPT(player, purple_uniques)) {
		standard_attr = COLOUR_VIOLET;
		if (!(optional_attr & 0x80))
			optional_attr = COLOUR_VIOLET;
	}

	/* Dump the name and then append standard attr/char info */
	textblock_append(tb, "%s", race->name);

	textblock_append(tb, " ('");
	textblock_append_pict(tb, standard_attr, standard_char);
	textblock_append(tb, "')");

	if (((optional_attr != standard_attr) || (optional_char != standard_char))
		&& (tile_width == 1) && (tile_height == 1)) {
		/* Append the "optional" attr/char info */
		textblock_append(tb, " ('");
		textblock_append_pict(tb, optional_attr, optional_char);
		textblock_append(tb, "')");
	}
}

/**
 * Place a full monster recall description (with title) into a textblock, with
 * or without spoilers.
 *
 * \param tb is the textblock we are placing the description into.
 * \param race is the monster race we are describing.
 * \param original_lore is the known information about the monster race.
 * \param spoilers indicates what information is used; `true` will display full
 *        information without subjective information and monster flavor,
 *        while `false` only shows what the player knows.
 */
void lore_description(textblock *tb, const struct monster_race *race,
					  const struct monster_lore *original_lore, bool spoilers)
{
	struct monster_lore mutable_lore;
	struct monster_lore *lore = &mutable_lore;
	bitflag known_flags[RF_SIZE];

	assert(tb && race && original_lore);

	/* Hack -- create a copy of the monster-memory that we can modify */
	memcpy(lore, original_lore, sizeof(struct monster_lore));

	/* Now get the known monster flags */
	monster_flags_known(race, lore, known_flags);

	/* Spoilers -- know everything */
	if (spoilers)
		cheat_monster_lore(race, lore);

	/* Appending the title here simplifies code in the callers. It also causes
	 * a crash when generating spoilers (we don't need titles for them anwyay)*/
	if (!spoilers) {
		lore_title(tb, race);
		textblock_append(tb, "\n");
	}

	/* Show kills of monster vs. player(s) */
	if (!spoilers)
		lore_append_kills(tb, race, lore, known_flags);

	lore_append_flavor(tb, race);

	/* Describe the monster type, speed, life, and armor */
	lore_append_movement(tb, race, lore, known_flags);

	if (!spoilers)
		lore_append_toughness(tb, race, lore, known_flags);

	/* Describe the experience and item reward when killed */
	if (!spoilers)
		lore_append_exp(tb, race, lore, known_flags);

	lore_append_drop(tb, race, lore, known_flags);

	/* Describe the special properties of the monster */
	lore_append_abilities(tb, race, lore, known_flags);
	lore_append_awareness(tb, race, lore, known_flags);
	lore_append_friends(tb, race, lore, known_flags);

	/* Describe the spells, spell-like abilities and melee attacks */
	lore_append_spells(tb, race, lore, known_flags);
	lore_append_attack(tb, race, lore, known_flags);

	/* Do we know everything */
	if (lore_is_fully_known(race))
		textblock_append(tb, "You know everything about this monster.");

	/* Notice "Quest" monsters */
	if (rf_has(race->flags, RF_QUESTOR))
		textblock_append(tb, "You feel an intense desire to kill this monster...  ");

	textblock_append(tb, "\n");
}

/**
 * Display monster recall modally and wait for a keypress.
 *
 * This is intended to be called when the main window is active (hence the
 * message flushing).
 *
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 */
void lore_show_interactive(const struct monster_race *race,
						   const struct monster_lore *lore)
{
	textblock *tb;
	assert(race && lore);

	event_signal(EVENT_MESSAGE_FLUSH);

	tb = textblock_new();
	lore_description(tb, race, lore, false);
	textui_textblock_show(tb, SCREEN_REGION, NULL);
	textblock_free(tb);
}

/**
 * Display monster recall statically.
 *
 * This is intended to be called in a subwindow, since it clears the entire
 * window before drawing, and has no interactivity.
 *
 * \param race is the monster race we are describing.
 * \param lore is the known information about the monster race.
 */
void lore_show_subwindow(const struct monster_race *race,
						 const struct monster_lore *lore)
{
	int y;
	textblock *tb;

	assert(race && lore);

	/* Erase the window, since textui_textblock_place() only clears what it
	 * needs */
	for (y = 0; y < Term->hgt; y++)
		Term_erase(0, y, 255);

	tb = textblock_new();
	lore_description(tb, race, lore, false);
	textui_textblock_place(tb, SCREEN_REGION, NULL);
	textblock_free(tb);
}

