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
#ifndef INCLUDED_BORG_ITEM_ID_H
#define INCLUDED_BORG_ITEM_ID_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item.h"

/* look for a *id*'d item */
extern bool borg_object_fully_id(void);

/* look for a *id*'d item */
extern bool borg_object_fully_id_aux(
    borg_item *borg_item, struct object *real_item);

/*
 * The code currently inscribes items with {??} if they have unknown powers.
 */
extern bool borg_item_note_needs_id(const borg_item *item);

#endif
#endif
