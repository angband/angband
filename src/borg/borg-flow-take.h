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

#ifndef INCLUDED_BORG_FLOW_TAKE_H
#define INCLUDED_BORG_FLOW_TAKE_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-flow.h"

/*
 * Object information
 */
typedef struct borg_take borg_take;

struct borg_take {
    struct object_kind *kind; /* Kind */
    bool                known; /* Verified kind */
    bool                seen; /* Assigned motion */
    bool                extra; /* Unused */
    bool                orbed; /* Orb of Draining cast on it */
    uint8_t             x, y; /* Location */
    int16_t             when; /* When last seen */
    int                 value; /* Estimated value of item */
    int                 tval; /* Known tval */
};

/*
 * The object list.  This list is used to "track" objects.
 */
extern int16_t    borg_takes_cnt;
extern int16_t    borg_takes_nxt;
extern borg_take *borg_takes;


/*
 * Helper to get the top non-ignored object
 */
struct object *borg_get_top_object(struct chunk *c, struct loc grid);

/*
 * Delete an old "object" record
 */
extern void borg_delete_take(int i);

/*
 * Attempt to "follow" a missing object
 */
extern void borg_follow_take(int i);

/*
 * Attempt to notice a changing "take"
 */
extern bool observe_take_diff(int y, int x, uint8_t a, wchar_t c);

/*
 * Attempt to "track" a "take" at the given location
 */
extern bool observe_take_move(int y, int x, int d, uint8_t a, wchar_t c);

/*
 * Prepare to "flow" towards objects to "take"
 */
extern bool borg_flow_take(bool viewable, int nearness);

/*
 * Prepare to "flow" towards special objects to "take"
 */
extern bool borg_flow_take_scum(bool viewable, int nearness);

/*
 * Prepare to "flow" towards special objects to "take"
 */
extern bool borg_flow_take_lunal(bool viewable, int nearness);

extern void borg_init_flow_take(void);
extern void borg_free_flow_take(void);

#endif
#endif
