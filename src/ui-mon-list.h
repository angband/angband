/**
 * \file ui-mon-list.h
 * \brief Monster list UI.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Ben Semmler
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

#ifndef UI_MONSTER_LIST_H
#define UI_MONSTER_LIST_H

#include "angband.h"

void monster_list_show_subwindow(int height, int width);
void monster_list_show_interactive(int height, int width);
void monster_list_force_subwindow_update(void);

#endif /* UI_MONSTER_LIST_H */
