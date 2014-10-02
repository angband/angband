/**
 * \file mon-summon.c
 * \brief Monster summoning

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

/*
 * Hack -- the "type" of the current "summon specific"
 */
static int summon_specific_type = 0;

/*
 * Hack - the kin type for S_KIN
 */
wchar_t summon_kin_type;


/**
 * Hack -- help decide if a monster race is "okay" to summon.
 *
 * Compares the given monster to the monster type specified by
 * summon_specific_type. Returns TRUE if the monster is eligible to
 * be summoned, FALSE otherwise. 
 */
static bool summon_specific_okay(monster_race *race)
{
	bool unique = rf_has(race->flags, RF_UNIQUE);
	bool scary = flags_test(race->flags, RF_SIZE, RF_UNIQUE, FLAG_END);

	/* Check our requirements */
	switch (summon_specific_type) {
	case S_ANY: return TRUE;
	case S_ANIMAL: return !unique && rf_has(race->flags, RF_ANIMAL);
	case S_SPIDER: return !unique && match_monster_bases(race->base, "spider", NULL);
	case S_HOUND: return !unique && match_monster_bases(race->base, "canine", "zephyr hound", NULL);
	case S_HYDRA: return !unique && match_monster_bases(race->base, "hydra", NULL);
	case S_AINU: return !scary && match_monster_bases(race->base, "ainu", NULL);
	case S_DEMON: return !scary && rf_has(race->flags, RF_DEMON);
	case S_UNDEAD: return !scary && rf_has(race->flags, RF_UNDEAD);
	case S_DRAGON: return !scary && rf_has(race->flags, RF_DRAGON);
	case S_KIN: return !unique && race->d_char == summon_kin_type;
	case S_HI_UNDEAD: return match_monster_bases(race->base, "lich", "vampire", "wraith", NULL);
	case S_HI_DRAGON: return match_monster_bases(race->base, "ancient dragon", NULL);
	case S_HI_DEMON: return match_monster_bases(race->base, "major demon", NULL);
	case S_WRAITH: return unique && match_monster_bases(race->base, "wraith", NULL);
	case S_UNIQUE: return unique;
	case S_MONSTER: return !scary;
	case S_MONSTERS: return !unique;

	default: return TRUE;
	}
}

int summon_message_type(int summon_type)
{
	switch (summon_type) {
	case S_ANY: return MSG_SUM_MONSTER;
	case S_ANIMAL: return MSG_SUM_ANIMAL;
	case S_SPIDER: return MSG_SUM_SPIDER;
	case S_HOUND: return MSG_SUM_HOUND;
	case S_HYDRA: return MSG_SUM_HYDRA;
	case S_AINU: return MSG_SUM_AINU;
	case S_DEMON: return MSG_SUM_DEMON;
	case S_UNDEAD: return MSG_SUM_UNDEAD;
	case S_DRAGON: return MSG_SUM_DRAGON;
	case S_HI_DEMON: return MSG_SUM_HI_DEMON;
	case S_HI_UNDEAD: return MSG_SUM_HI_UNDEAD;
	case S_HI_DRAGON: return MSG_SUM_HI_DRAGON;
	case S_WRAITH: return MSG_SUM_WRAITH;
	case S_UNIQUE: return MSG_SUM_UNIQUE;
	default: return MSG_SUM_MONSTER;
	}
}

/* Check to see if you can call the monster */
bool can_call_monster(int y, int x, monster_type *m_ptr)
{
	int oy, ox;

	/* Skip dead monsters */
	if (!m_ptr->race) return (FALSE);
	
	/* Only consider callable monsters */
	if (!summon_specific_okay(m_ptr->race)) return (FALSE);
	
	/* Extract monster location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;
	
	/* Make sure the summoned monster is not in LOS of the summoner */
	if (los(cave, y, x, oy, ox)) return (FALSE);
	
	return (TRUE);
}


/** Calls a monster from the level and moves it to the desired spot
*/
int call_monster(int y, int x)
{
	int i, mon_count, choice;
	int oy, ox;
	int *mon_indices;
	monster_type *m_ptr;

	mon_count = 0;
	
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		m_ptr = cave_monster(cave, i);
		
		/* Figure out how many good mosnters there are */
		if (can_call_monster(y, x, m_ptr)) mon_count++;
	}
	
	/* There were no good monsters on the level */
	if (mon_count == 0) return (0);
  	
	/* Make the array */
	mon_indices = C_ZNEW(mon_count, int);
	
	/* reset mon_count */
	mon_count = 0;
	
	/* Now go through a second time and store the indices */
	for (i = 1; i < cave_monster_max(cave); i++)
	{
		m_ptr = cave_monster(cave, i);
		
		/* Save the values of the good monster */
		if (can_call_monster(y, x, m_ptr)){
			mon_indices[mon_count] = i;
			mon_count++;
		}
	}
	
	/* Pick one */
	choice = randint0(mon_count - 1);
	
	/* Get the lucky monster */
	m_ptr = cave_monster(cave, mon_indices[choice]);
	FREE(mon_indices);
	
	/* Extract monster location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;
	
	/* Swap the moster */
	monster_swap(oy, ox, y, x);
	
	/* wake it up */
	mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);
		
	/* Set it's energy to 0 */
	m_ptr->energy = 0;
		
	return (m_ptr->race->level);

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
	int i, x = 0, y = 0;

	monster_type *m_ptr;
	monster_race *race;

	/* Look for a location, allow up to 4 squares away */
	for (i = 0; i < 60; ++i)
	{
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
#ifdef FIZZIX_SUMMON	
	/* Use the new calling scheme 
	  Hack - use delay to determine if it's a normal summon or a trap summon */
	if ((type != S_UNIQUE) && (type != S_WRAITH)  && !delay ){
		return (call_monster(y, x));
	}
#endif
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
	m_ptr = square_monster(cave, y, x);
	
	/* If delay, try to let the player act before the summoned monsters,
	 * including slowing down faster monsters for one turn */
	if (delay) {
		m_ptr->energy = 0;
		if (m_ptr->race->speed > player->state.speed)
			mon_inc_timed(m_ptr, MON_TMD_SLOW, 1,
				MON_TMD_FLG_NOMESSAGE, FALSE);
	}

	return (m_ptr->race->level);
}

