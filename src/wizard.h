/**
 * \file wizard.h
 * \brief Debug mode commands, stats collection, spoiler generation
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef INCLUDED_WIZARD_H
#define INCLUDED_WIZARD_H

#include "cave.h"

/* For stat_grid_counter() */
struct chunk;
struct grid_counter_pred {
	square_predicate pred;
	/* Hold the number of grids that match pred and are in vaults. */
	int in_vault_count;
	/*
	 * Hold the number of grids that match pred and are in rooms (but not
	 * in vaults).
	 */
	int in_room_count;
	/*
	 * Hold the number of grids that match pred and are neither in vaults
	 * nor rooms.
	 */
	int in_other_count;
};
struct neighbor_counter_pred {
	square_predicate pred;
	square_predicate neigh;
	int vault_histogram[9];
	int room_histogram[9];
	int other_histogram[9];
};

/* For stat_grid_counter_simple() */
struct grid_counts {
	int floor;
	int upstair;
	int downstair;
	int trap;
	int lava;
	int impass_rubble;
	int pass_rubble;
	int magma_treasure;
	int quartz_treasure;
	int open_door;
	int closed_door;
	int broken_door;
	int secret_door;
	int traversable_neighbor_histogram[9];
};

/* wiz-debug.c */
void wiz_cheat_death(void);

/* wiz-stats.c */
bool stats_are_enabled(void);
void stats_collect(int nsim, int simtype);
void disconnect_stats(int nsim, bool stop_on_disconnect);
void pit_stats(int nsim, int pittype, int depth);
void stat_grid_counter(struct chunk *c, struct grid_counter_pred *gpreds,
	int n_gpred, struct neighbor_counter_pred *npreds, int n_npred);
void stat_grid_counter_simple(struct chunk *c, struct grid_counts counts[3]);

/* wiz-spoil.c */
void spoil_artifact(const char *fname);
void spoil_mon_desc(const char *fname);
void spoil_mon_info(const char *fname);
void spoil_obj_desc(const char *fname);

#endif /* !INCLUDED_WIZARD_H */
