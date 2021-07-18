/**
 * \file ui-target.h
 * \brief UI for targetting code
 *
 * Copyright (c) 1997-2014 Angband contributors
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


#ifndef UI_TARGET_H
#define UI_TARGET_H

#include "ui-event.h"

/**
 * Convert a "key event" into a "location" (Y)
 */
#define KEY_GRID_Y(K) \
  ((int) (((K.mouse.y - ROW_MAP) / tile_height) + Term->offset_y))

/**
 * Convert a "key event" into a "location" (X)
 */
#define KEY_GRID_X(K) \
	((int) (((K.mouse.x - COL_MAP) / tile_width) + Term->offset_x))


/**
 * Height of the help screen; any higher than 4 will overlap the health
 * bar which we want to keep in targeting mode.
 */
#define HELP_HEIGHT 3

/**
 * Size of the array that is used for object names during targeting.
 */
#define TARGET_OUT_VAL_SIZE 256

int target_dir(struct keypress ch);
int target_dir_allow(struct keypress ch, bool allow_5);
void target_display_help(bool monster, bool object, bool free);
void textui_target(void);
void textui_target_closest(void);
bool target_set_interactive(int mode, int x, int y);

#endif /* UI_TARGET_H */
