/**
 * \file ui-visuals.h
 * \brief Appearance for screen elements.
 *
 * Copyright (c) 2016 Ben Semmler
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

#ifndef UI_VISUALS_H
#define UI_VISUALS_H

extern struct init_module ui_visuals_module;

uint8_t visuals_cycler_get_attr_for_frame(const char *group_name,
									   const char *cycle_name,
									   size_t const frame);
void visuals_cycler_set_cycle_for_race(struct monster_race const *race,
									   const char *group_name,
									   const char *cycle_name);
uint8_t visuals_cycler_get_attr_for_race(struct monster_race const *race,
									  size_t const frame);
uint8_t visuals_flicker_get_attr_for_frame(uint8_t const selection_attr,
										size_t const frame);

#endif /* UI_VISUALS_H */
