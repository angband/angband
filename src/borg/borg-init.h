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

#ifndef INCLUDED_BORG_INIT_H
#define INCLUDED_BORG_INIT_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

extern bool borg_init_failure;

extern bool borg_initialized; /* Hack -- Initialized */
extern bool game_closed; /* Has the game been closed */

/*
 * Initialize borg.txt
 */
extern void borg_init_txt_file(void);

/*
 * Reset the required options when returning from user control
 */
extern void borg_reinit_options(void);

/*
 * Hack -- prepare some stuff based on the player race and class
 */
extern void borg_prepare_race_class_info(void);

/*
 * Initialize the Borg
 */
extern void borg_init(void);

/*
 * Clean up resources allocated for the borg.
 */
extern void borg_free(void);

#endif
#endif
