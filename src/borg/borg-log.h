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

#ifndef INCLUDED_BORG_LOG_H
#define INCLUDED_BORG_LOG_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

/*
 * write a death to borg-log.txt
 */
extern void borg_log_death(void);

/*
 * write a death to borg.dat
 */
extern void borg_log_death_data(void);

/*
 * Display what the borg is thinking
 */
extern void borg_status(void);

/*
 * Write a file with the current dungeon info
 */
extern void borg_write_map(bool ask);

/*
 * Display the values which the borg believes an item has.
 */
void borg_display_item(struct object *item2, int n);

#endif
#endif
