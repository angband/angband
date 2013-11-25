/*
 * File: obj-list.h
 * Purpose: Object list UI.
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

#ifndef OBJECT_LIST_H
#define OBJECT_LIST_H

#include "angband.h"

void object_list_init(void);
void object_list_finalize(void);
void object_list_show_subwindow(int height, int width);
void object_list_show_interactive(int height, int width);

#endif /* OBJECT_LIST_H */
