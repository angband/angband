/**
 * \file mon-summon.c
 * \brief Monster summoning
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
#include "mon-make.h"
#include "mon-summon.h"
#include "mon-util.h"

/**
 * The "type" of the current "summon specific"
 */
static int summon_specific_type = 0;

/**
 * The kin base for S_KIN
 */
struct monster_base *kin_base;

static struct summon_details {
	const char *name;
	int message_type;
	bool unique_allowed;
	const char *base1;
	const char *base2;
	const char *base3;
	int race_flag;
	const char *description;
} summon_info[] = {
	#define S(a, b, c, d, e, f, g, h) { #a, b, c, d, e, f, g, h },
	#include "list-summon-types.h"
	#undef S
};

int summon_name_to_idx(const char *name)
{
    int i;
    for (i = 0; !streq(summon_info[i].name, "MAX"); i++) {
        if (streq(name, summon_info[i].name))
            return i;
    }

    return -1;
}

const char *summon_desc(int type)
{
	if (type < 0 || type >= S_MAX)
		return 0;

	return summon_info[type].description;
}

/**
 * Decide if a monster race is "okay" to summon.
 *
 * Compares the given monster to the monster type specified by
 * summon_specific_type. Returns TRUE if the monster is eligible to
 * be summoned, FALSE otherwise. 
 */
static bool summon_specific_okay(struct monster_race *race)
{
	struct summon_details *info = &summon_info[summon_specific_type];
	bool unique = rf_has(race->flags, RF_UNIQUE);

	/* Forbid uniques? */
	if (!info->unique_allowed && unique)
		return FALSE;

	/* A valid base and no match means disallowed */
	if (info->base1 && !match_monster_bases(race->base, info->base1,
											info->base2, info->base3, NULL))
		return FALSE;

	/* A valid race flag and no match means disallowed */
	if (info->race_flag && !rf_has(race->flags, info->race_flag))
		return FALSE;

	/* Special case - summon kin */
	if (summon_specific_type == S_KIN)
		return (!unique && race->base == kin_base);

	/* If we made it here, we're fine */
	return TRUE;
}

/**
 * The message type for a particular summon
 */
int summon_message_type(int summon_type)
{
	return summon_info[summon_type].message_type;
}

/**
 * Check to see if you can call the monster
 */
bool can_call_monster(int y, int x, struct monster *mon)
{
	int oy, ox;

	/* Skip dead monsters */
	if (!mon->race) return (FALSE);

	/* Only consider callable monsters */
	if (!summon_specific_okay(mon->race)) return (FALSE);

	/* Extract monster location */
	oy = mon->fy;
	ox = mon->fx;

	/* Make sure the summoned monster is not in LOS of the summoner */
	if (los(cave, y, x, oy, ox)) return (FALSE);

	return (TRUE);
}


/**
 * Calls a monster from the level and moves it to the desired spot
 */
int call_monster(int y, int x)
{
	int i, mon_count, choice;
	int oy, ox;
	int *mon_indices;
	struct monster *mon;

	mon_count = 0;

	for (i = 1; i < cave_monster_max(cave); i++) {
		mon = cave_monster(cave, i);

		/* Figure out how many good monsters there are */
		if (can_call_monster(y, x, mon)) mon_count++;
	}

	/* There were no good monsters on the level */
	if (mon_count == 0) return (0);

	/* Make the array */
	mon_indices = mem_zalloc(mon_count * sizeof(int));

	/* Reset mon_count */
	mon_count = 0;

	/* Now go through a second time and store the indices */
	for (i = 1; i < cave_monster_max(cave); i++) {
		mon = cave_monster(cave, i);
		
		/* Save the values of the good monster */
		if (can_call_monster(y, x, mon)){
			mon_indices[mon_count] = i;
			mon_count++;
		}
	}

	/* Pick one */
	choice = randint0(mon_count - 1);

	/* Get the lucky monster */
	mon = cave_monster(cave, mon_indices[choice]);
	mem_free(mon_indices);

	/* Extract monster location */
	oy = mon->fy;
	ox = mon->fx;

	/* Swap the moster */
	monster_swap(oy, ox, y, x);

	/* Wake it up */
	mon_clear_timed(mon, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);

	/* Set it's energy to 0 */
	mon->energy = 0;

	return (mon->race->level);
}


/**
 * Places a monster (of the specified "type") near the given
 * location.  Return TRUE iff a monster was actually summoned.
 *
 * We will attempt to place the monster up to 10 times before giving up.
 *
 * Note: S_UNIQUE and S_WRAITH will summon Uniques
 * Note: S_ANY, S_HI_UNDEAD, S_HI_DEMON and S_HI_DRAGON may summon Uniques
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
int summon_specific(int y1, int x1, int lev, int type, bool delay, bool call)
{
	int i, x = 0, y = 0;

	struct monster *mon;
	struct monster_race *race;

	/* Look for a location, allow up to 4 squares away */
	for (i = 0; i < 60; ++i) {
		/* Pick a distance */
		int d = (i / 15) + 1;

		/* Pick a location */
		scatter(cave, &y, &x, y1, x1, d, TRUE);

		/* Require "empty" floor grid */
		if (!square_isempty(cave, y, x)) continue;

		/* Hack -- no summon on glyph of warding */
		if (square_iswarded(cave, y, x)) continue;

		/* Okay */
		break;
	}

	/* Failure */
	if (i == 60) return (0);

	/* Save the "summon" type */
	summon_specific_type = type;

	/* Use the new calling scheme if requested */
	if (call && (type != S_UNIQUE) && (type != S_WRAITH)) {
		return (call_monster(y, x));
	}

	/* Prepare allocation table */
	get_mon_num_prep(summon_specific_okay);

	/* Pick a monster, using the level calculation */
	race = get_mon_num((player->depth + lev) / 2 + 5);

	/* Prepare allocation table */
	get_mon_num_prep(NULL);

	/* Handle failure */
	if (!race) return (0);

	/* Attempt to place the monster (awake, don't allow groups) */
	if (!place_new_monster(cave, y, x, race, FALSE, FALSE, ORIGIN_DROP_SUMMON))
		return (0);

	/* Success, return the level of the monster */
	mon = square_monster(cave, y, x);

	/* If delay, try to let the player act before the summoned monsters,
	 * including slowing down faster monsters for one turn */
	if (delay) {
		mon->energy = 0;
		if (mon->race->speed > player->state.speed)
			mon_inc_timed(mon, MON_TMD_SLOW, 1,
				MON_TMD_FLG_NOMESSAGE, FALSE);
	}

	return (mon->race->level);
}

