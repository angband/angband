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

#ifndef INCLUDED_BORG_THINK_H
#define INCLUDED_BORG_THINK_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * Current shop index
 */
extern int16_t shop_num;

/*
 * Strategy flags -- examine the world
 */
extern bool borg_do_inven; /* Acquire "inven" info */
extern bool borg_do_equip; /* Acquire "equip" info */
extern bool borg_do_panel; /* Acquire "panel" info */
extern bool borg_do_frame; /* Acquire "frame" info */
extern bool borg_do_spell; /* Acquire "spell" info */

/*
 * Abort the Borg, noting the reason
 */
extern void borg_oops(const char *what);

/*
 * Think about the world and perform an action
 */
extern bool borg_think(void);

#endif
#endif
