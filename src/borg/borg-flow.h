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

#ifndef INCLUDED_BORG_FLOW_H
#define INCLUDED_BORG_FLOW_H

/*
 * must be included before ALLOW_BORG to avoid empty compilation unit
 */
#include "../angband.h"

#ifdef ALLOW_BORG

#include "borg-cave.h"

/*
 * Threshold where the borg will attempt to dig things
 */
#define BORG_DIG      13
#define BORG_DIG_HARD 40

/*
 * track where some things are
 */
struct borg_track {
    int16_t num;
    int16_t size;
    int    *x;
    int    *y;
};

/*
 * Forward declare
 */
typedef struct borg_data borg_data;

/*
 * Hack -- one byte of info per grid
 *
 * We use a structure to encapsulate the data into a "typed" form.
 */
struct borg_data {
    uint8_t data[AUTO_MAX_Y][AUTO_MAX_X];
};

/*
 * Number of grids in the "flow" array
 */
#define AUTO_FLOW_MAX 1536

/*
 * Maintain a set of grids (flow calculations)
 */
extern int16_t borg_flow_n;
extern uint8_t borg_flow_y[AUTO_FLOW_MAX];
extern uint8_t borg_flow_x[AUTO_FLOW_MAX];

/*
 * Hack -- use "flow" array as a queue
 */

extern int flow_head;
extern int flow_tail;

/*
 * Some variables
 */
extern borg_data *borg_data_flow; /* Current "flow" data */
extern borg_data *borg_data_cost; /* Current "cost" data */
extern borg_data *borg_data_hard; /* Constant "hard" data */
extern borg_data *borg_data_know; /* Current "know" flags */
extern borg_data *borg_data_icky; /* Current "icky" flags */

/*
 * Number of grids in the "temp" array
 */
#define AUTO_TEMP_MAX 9000

/*
 * Maintain a set of grids (scanning arrays)
 */
extern int16_t borg_temp_n;
extern uint8_t borg_temp_y[AUTO_TEMP_MAX];
extern uint8_t borg_temp_x[AUTO_TEMP_MAX];

/*
 * Track Steps
 */
extern struct borg_track track_step;

/*
 * Track closed doors which I have closed
 */
extern struct borg_track track_door;

/*
 * Track closed doors which started closed
 */
extern struct borg_track track_closed;

extern bool borg_desperate;
extern bool vault_on_level;

/*
 * Anti-Summon
 */
extern int  borg_t_antisummon;
extern bool borg_as_position;
extern bool borg_digging;
extern bool my_need_alter;
extern bool my_no_alter;
extern bool my_need_redraw;

/*
 * Current danger thresh-hold
 */
extern int16_t avoidance;

/*
 * Search grids
 */
extern const int16_t borg_ddx_ddd[24];
extern const int16_t borg_ddy_ddd[24];

/*
 * Check if the borg can dig.
 */
extern bool borg_can_dig(bool check_fail, uint8_t feat);

/*
 * Clear the "flow" information
 */
extern void borg_flow_clear(void);

/*
 * Spread a "flow" from the "destination" grids outwards
 */
extern void borg_flow_spread(int depth, bool optimize, bool avoid,
    bool tunneling, int stair_idx, bool sneak);

/*
 * Enqueue a fresh (legal) starting grid, if it is safe
 */
extern void borg_flow_enqueue_grid(int y, int x);

/*
 * Commit the current "flow"
 */
extern bool borg_flow_commit(const char *who, int why);

/*
 * Attempt to take an optimal step towards the current goal location
 */
extern bool borg_flow_old(int why);

extern void borg_init_track(struct borg_track *track, int size);
extern void borg_free_track(struct borg_track *track);

extern void borg_init_flow(void);
extern void borg_free_flow(void);

#endif
#endif
