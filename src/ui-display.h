/**
 * \file ui-display.h
 * \brief Handles the setting up updating, and cleaning up of the game display.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007 Antony Sidwell
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

#ifndef INCLUDED_UI_DISPLAY_H
#define INCLUDED_UI_DISPLAY_H

#include "angband.h"
#include "cmd-core.h"

extern const char *stat_names[STAT_MAX];
extern const char *stat_names_reduced[STAT_MAX];
extern const char *window_flag_desc[32];

uint8_t monster_health_attr(void);
void cnv_stat(int val, char *out_val, size_t out_len);
void allow_animations(void);
void disallow_animations(void);
void idle_update(void);
void toggle_inven_equip(void);
void subwindows_set_flags(uint32_t *new_flags, size_t n_subwindows);
void init_display(void);

#endif /* INCLUDED_UI_DISPLAY_H */
