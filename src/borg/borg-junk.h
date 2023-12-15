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

#ifndef INCLUDED_BORG_JUNK_H
#define INCLUDED_BORG_JUNK_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Destroy "junk" items
 */
extern bool borg_crush_junk(void);

/*
 * Destroy something to make a free inventory slot.
 */
extern bool borg_crush_hole(void);

/*
 * Destroy "junk" when slow (in the dungeon).
 */
extern bool borg_crush_slow(void);

/*
 * Examine the quiver and dump any worthless items
 */
extern bool borg_dump_quiver(void);

/*
 * Remove useless equipment.
 */
extern bool borg_remove_stuff(void);

#endif
#endif
