/**
 * \file player-properties.c 
 * \brief Class and race abilities
 *
 * Copyright (c) 1997-2020 Ben Harrison, James E. Wilson, Robert A. Koeneke,
 * Leon Marrick, Bahman Rabii, Nick McConnell
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
#include "player-properties.h"
#include "game-input.h"

/**
 * ------------------------------------------------------------------------
 * Ability utilities
 * ------------------------------------------------------------------------ */
bool class_has_ability(const struct player_class *class,
					   struct player_ability *ability)
{
	if (streq(ability->type, "player") &&
		pf_has(class->pflags, ability->index)) {
		return true;
	} else if (streq(ability->type, "object") &&
			   of_has(class->flags, ability->index)) {
		return true;
	}
	return false;
}

bool race_has_ability(const struct player_race *race,
					  struct player_ability *ability)
{
	if (streq(ability->type, "player") &&
		pf_has(race->pflags, ability->index)) {
		return true;
	} else if (streq(ability->type, "object") &&
			   of_has(race->flags, ability->index)) {
		return true;
	} else if (streq(ability->type, "element") &&
			   (race->el_info[ability->index].res_level == ability->value)) {
		return true;
	}

	return false;
}


/**
 * Browse known abilities -BR-
 */
static void view_abilities(void)
{
	struct player_ability *ability;
	int num_abilities = 0;
	struct player_ability ability_list[32];

	/* Count the number of class powers we have */
	for (ability = player_abilities; ability; ability = ability->next) {
		if (class_has_ability(player->class, ability)) {
			memcpy(&ability_list[num_abilities], ability,
				   sizeof(struct player_ability));
			ability_list[num_abilities++].group = PLAYER_FLAG_CLASS;
		}
	}

	/* Count the number of race powers we have */
	for (ability = player_abilities; ability; ability = ability->next) {
		if (race_has_ability(player->race, ability)) {
			memcpy(&ability_list[num_abilities], ability,
				   sizeof(struct player_ability));
			ability_list[num_abilities++].group = PLAYER_FLAG_RACE;
		}
	}

	/* View choices until user exits */
	view_ability_menu(ability_list, num_abilities);

	return;
}


/**
 * Interact with abilities -BR-
 */
void do_cmd_abilities(void)
{
	/* View existing abilities */
	view_abilities();

	return;
}
