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
#ifndef INCLUDED_BORG_ITEM_ANALYZE_H
#define INCLUDED_BORG_ITEM_ANALYZE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-item.h"

/*
 * Analyze an item, given a textual description
 */
extern void borg_item_analyze(
    borg_item *item, const struct object *real_item, char *desc, bool in_store);

/*
 * Check if an item produces a certain effect
 */
extern bool borg_obj_has_effect(uint32_t kind, int index, int subtype);

/*
 * Check if an item power is "random"
 */
extern bool borg_ego_has_random_power(struct ego_item *e_ptr);

#endif
#endif
