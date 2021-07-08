/**
 * \file player-properties.h
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

#ifndef PLAYER_PROPS_H
#define PLAYER_PROPS_H

bool class_has_ability(const struct player_class *class,
					   struct player_ability *ability);
bool race_has_ability(const struct player_race *race,
					  struct player_ability *ability);
void do_cmd_abilities(void);

#endif /* !PLAYER_PROPS_H */
