/**
 * \file player-path.h
 * \brief Pathfinding and running code.
 *
 * Copyright (c) 1988 Christopher J Stuart (running code)
 * Copyright (c) 2004-2007 Christophe Cavalaria, Leon Marrick (pathfinding)
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef PLAYER_PATH_H
#define PLAYER_PATH_H

#include "z-type.h"

struct pfdistances;

struct pfdistances *prepare_pfdistances(struct player *p, struct loc start,
		bool only_known, bool forbid_traps);
int pfdistances_to_turncount(const struct pfdistances *a, struct loc grid);
int pfdistances_to_path(const struct pfdistances *a, struct loc grid,
		int16_t **step_dirs);
void release_pfdistances(struct pfdistances *a);
int path_nearest_known(struct player *p, struct loc start,
		bool (*pred)(struct chunk*, struct loc),
		struct loc *dest_grid, int16_t **step_dirs);
int path_nearest_unknown(struct player *p, struct loc start,
		struct loc *dest_grid, int16_t **step_dirs);
int find_path(struct player *p, struct loc start, struct loc dest,
		int16_t **step_dirs);
int pathfind_direction_to(struct loc from, struct loc to);
void run_step(int dir);

#endif /* !PLAYER_PATH_H */
