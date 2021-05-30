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
#include "cave.h"
#include "mon-group.h"
#include "mon-spell.h"
#include "mon-util.h"

/**
 * ------------------------------------------------------------------------
 * Permanent monster properties
 * ------------------------------------------------------------------------ */
/**
 * Undead monsters
 */
bool monster_is_undead(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_UNDEAD);
}

/**
 * Nonliving monsters are immune to life drain
 */
bool monster_is_nonliving(const struct monster *mon)
{
	return (monster_is_undead(mon) || rf_has(mon->race->flags, RF_NONLIVING));
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
					  RF_SMASH_WALL, FLAG_END);
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
 * Monster is stupid
 */
bool monster_is_stupid(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_STUPID);
}

/**
 * Monster is (or was) smart
 */
bool monster_is_smart(const struct monster *mon)
{
	if (mon->original_race && rf_has(mon->original_race->flags, RF_SMART)) {
		return true;
	}
	return rf_has(mon->race->flags, RF_SMART);
}

/**
 * Monster is (or was) detectable by telepathy
 *
 * Note that weirdness may result if WEIRD_MIND monsters shapechange
 */
bool monster_is_esp_detectable(const struct monster *mon)
{
	bitflag flags[RF_SIZE];
	rf_copy(flags, mon->race->flags);
	if (mon->original_race) {
		rf_inter(flags, mon->original_race->flags);
	}
	if (rf_has(flags, RF_EMPTY_MIND)) {
		/* Empty mind, no telepathy */
		return false;
	} else if (rf_has(mon->race->flags, RF_WEIRD_MIND)) {
		/* Weird mind, one in ten individuals are detectable */
		if ((mon->midx % 10) != 5) {
			/* Undetectable */
			return false;
		}
	}

	return true;
}

/**
 * Monster has a spirit
 */
bool monster_has_spirit(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_SPIRIT);
}

/**
 * Monster is evil
 */
bool monster_is_evil(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_EVIL);
}

/**
 * Monster can be frightened
 */
bool monster_is_fearful(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_NO_FEAR) ? false : true;
}

/**
 * Monster is powerful
 */
bool monster_is_powerful(const struct monster *mon)
{
	return rf_has(mon->race->flags, RF_POWERFUL);
}

/**
 * Monster has spells
 */
bool monster_has_spells(const struct monster *mon)
{
	return rsf_is_empty(mon->race->spell_flags) ? false : true;
}

/**
 * Monster has damaging breath
 */
bool monster_breathes(const struct monster *mon)
{
	bitflag breaths[RSF_SIZE];
	create_mon_spell_mask(breaths, RST_BREATH, RST_NONE);
	rsf_inter(breaths, mon->race->spell_flags);
	return rsf_is_empty(breaths) ? false : true;
}

/**
 * Monster has innate spells
 */
bool monster_has_innate_spells(const struct monster *mon)
{
	bitflag innate_spells[RSF_SIZE];
	create_mon_spell_mask(innate_spells, RST_INNATE, RST_NONE);
	rsf_inter(innate_spells, mon->race->spell_flags);
	return rsf_is_empty(innate_spells) ? false : true;
}

/**
 * Monster has non-innate spells
 */
bool monster_has_non_innate_spells(const struct monster *mon)
{
	bitflag innate_spells[RSF_SIZE], mon_spells[RSF_SIZE];
	create_mon_spell_mask(innate_spells, RST_INNATE, RST_NONE);
	rsf_copy(mon_spells, mon->race->spell_flags);
	rsf_diff(mon_spells, innate_spells);
	return rsf_is_empty(mon_spells) ? false : true;
}

/**
 * Monster has frequent and good archery attacks
 */
bool monster_loves_archery(const struct monster *mon)
{
	bitflag shooting[RSF_SIZE];
	create_mon_spell_mask(shooting, RST_ARCHERY, RST_NONE);
	rsf_inter(shooting, mon->race->spell_flags);
	if (rsf_is_empty(shooting)) return false;
	return (mon->race->freq_innate < 4) ? true : false;
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

/**
 * Monster can be frightened
 *
 * Note that differing group roles imply a chance of avoiding fear:
 * - bodyguards are fearless
 * - servants have a one in 3 chance
 * - others have a chance depending on the size of the group
 */
bool monster_can_be_scared(const struct monster *mon)
{
	if (rf_has(mon->race->flags, RF_NO_FEAR)) return false;
	switch (mon->group_info[PRIMARY_GROUP].role) {
		case MON_GROUP_BODYGUARD: return false;
		case MON_GROUP_SERVANT:	return one_in_(3) ? false : true;
		default: {
			int count = monster_primary_group_size(cave, mon) - 1;
			while (count--) {
				if (one_in_(20)) return false;
			}
		}
	}
	return true;
}

/**
 * Monster attracted to a decoy, not the player
 */
bool monster_is_decoyed(const struct monster *mon)
{
	struct loc decoy = cave_find_decoy(cave);

	/* No decoy */
	if (loc_is_zero(decoy)) return false;

	/* Monster can't see the decoy */
	if (!los(cave, mon->grid, decoy)) return false;

	return true;
}
