/**
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband License":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef INCLUDED_BORG_CAVE_LIGHT_H
#define INCLUDED_BORG_CAVE_LIGHT_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Maximum size of the "lite" array
 */
#define AUTO_LIGHT_MAX 1536

/*
 * Maintain a set of grids (liteable grids)
 */

extern int16_t borg_light_n;
extern uint8_t borg_light_y[AUTO_LIGHT_MAX];
extern uint8_t borg_light_x[AUTO_LIGHT_MAX];

/*
 * Maintain a set of glow grids (liteable grids)
 */

extern int16_t borg_glow_n;
extern uint8_t borg_glow_y[AUTO_LIGHT_MAX];
extern uint8_t borg_glow_x[AUTO_LIGHT_MAX];

/*
 * Update the "lite"
 */
extern void borg_update_light(void);

#endif
#endif
