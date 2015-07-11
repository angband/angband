/**
 * \file ui-mon-lore.h
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

#ifndef UI_MONSTER_LORE_H
#define UI_MONSTER_LORE_H

void lore_title(textblock *tb, const struct monster_race *race);
void lore_description(textblock *tb, const struct monster_race *race,
					  const struct monster_lore *original_lore, bool spoilers);
void lore_show_interactive(const struct monster_race *race,
						   const struct monster_lore *lore);
void lore_show_subwindow(const struct monster_race *race,
						 const struct monster_lore *lore);

#endif /* UI_MONSTER_LORE_H */
