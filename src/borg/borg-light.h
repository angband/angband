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

#ifndef INCLUDED_BORG_LIGHT_H
#define INCLUDED_BORG_LIGHT_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item-use.h"

/*
 * Check to see if the surrounding dungeon should be illuminated, and if
 * it should, do it.
 */
extern bool borg_check_light_only(void);

/*
 * Refuel, call lite, detect traps/doors/walls/evil, etc
 */
extern bool borg_check_light(void);

/*
 * Refuel the light if it is needed
 */
extern enum borg_need borg_maintain_light(void);

/*
 * This will look down a hallway and possibly light it up
 */
extern bool borg_light_beam(bool simulation);

#endif
#endif
