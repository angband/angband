/**
 * \file mon-predicate.c
 * \brief Monster predicates
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2017 Nick McConnell
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
#include "mon-util.h"

/**
 * ------------------------------------------------------------------------
 * Permanent monster properties
 * ------------------------------------------------------------------------ */
/**
 * Nonliving monsters are immune to life drain
 */
bool monster_is_nonliving(const struct monster *mon)
{
	return flags_test(mon->race->flags, RF_SIZE, RF_DEMON, RF_UNDEAD,
					  RF_NONLIVING, FLAG_END);
}

/**
 * Living monsters
 */
bool monster_is_living(const struct monster *mon)
{
	return !monster_is_nonliving(mon);
}

/**
 * Nonliving and stupid monsters are destroyed rather than dying
 */
bool monster_is_destroyed(const struct monster *mon)
{
	return (monster_is_nonliving(mon) || rf_has(mon->race->flags, RF_STUPID));
}


/**
 * Monster can pass through walls
 */
bool monster_passes_walls(const struct monster *mon)
{
	return flags_test(mon->race->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL,
					  FLAG_END);
}

/**
 * Monster is invisible
 */
bool monster_is_invisible(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_INVISIBLE);
}

/**
 * Monster is visible, in principle
 */
bool monster_is_not_invisible(const struct monster *mon)
{
	return !rf_has(mon->race->flags, RF_INVISIBLE);
}

/**
 * Monster is unique
 */
bool monster_is_unique(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_UNIQUE);
}

/**
 * Monster is 
 */
bool monster_is_stupid(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_STUPID);
}

/**
 * Monster is smart
 */
bool monster_is_smart(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_SMART);
}

/**
 * Monster is evil
 */
bool monster_is_evil(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_EVIL);
}

/**
 * Monster is powerful
 */
bool monster_is_powerful(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_POWERFUL);
}


/**
 * ------------------------------------------------------------------------
 * Temporary monster properties
 * ------------------------------------------------------------------------ */
/**
 * Monster is in the player's field of view
 */
bool monster_is_in_view(const struct monster *mon)
{
	return mflag_has(mon->mflag, MFLAG_VIEW);
}

/**
 * Monster is visible to the player
 */
bool monster_is_visible(const struct monster *mon)
{
	return mflag_has(mon->mflag, MFLAG_VISIBLE);
}

/**
 * Player doesn't recognise the monster as a monster
 */
bool monster_is_camouflaged(const struct monster *mon)
{
	return mflag_has(mon->mflag, MFLAG_CAMOUFLAGE);
}

/**
 * Monster is recognisably a monster to the player
 */
bool monster_is_obvious(const struct monster *mon)
{
	return monster_is_visible(mon) && !monster_is_camouflaged(mon);
}

/**
 * Monster is currently mimicking an item
 */
bool monster_is_mimicking(const struct monster *mon)
{
	return mflag_has(mon->mflag, MFLAG_CAMOUFLAGE) && mon->mimicked_obj;
}


