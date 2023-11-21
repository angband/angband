/**
 *
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

#ifndef INCLUDED_BORG_UPDATE_H
#define INCLUDED_BORG_UPDATE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Notice failure
 */
extern bool borg_failure;

/*
 * Hack -- the detection arrays
 */
extern bool borg_detect_wall[6][18];
extern bool borg_detect_trap[6][18];
extern bool borg_detect_door[6][18];
extern bool borg_detect_evil[6][18];
extern bool borg_detect_obj[6][18];

/*
 * Strategy flags -- recalculate things
 */
extern bool borg_do_update_view; /* Recalculate view */
extern bool borg_do_update_lite; /* Recalculate lite */

/*
 * Update state based on current "map"
 */
extern void borg_update(void);

extern void borg_init_update(void);
extern void borg_free_update(void);

#endif
#endif
