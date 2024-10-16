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

#ifndef INCLUDED_BORG_PREPARED_H
#define INCLUDED_BORG_PREPARED_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

extern int          borg_numb_live_unique;
extern unsigned int borg_first_living_unique;
extern int          borg_depth_hunted_unique;

/*
 * Determine what level the borg is prepared to dive to.
 */
extern const char *borg_prepared(int depth);

/*
 * Determine if the Borg is out of "crucial" supplies.
 */
extern const char *borg_restock(int depth);

#endif
#endif
