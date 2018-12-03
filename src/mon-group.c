/**
 * \file mon-group.c
 * \brief Monster group behaviours
 *
 * Copyright (c) 2018 Nick McConnell
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

#include "angband.h"
#include "cave.h"
#include "init.h"
#include "mon-group.h"
#include "monster.h"

/**
 * Allocate a new monster group
 */
struct monster_group *monster_group_new(void)
{
	struct monster_group *group = mem_zalloc(sizeof(struct monster_group));
	return group;
}

/**
 * Free a monster group
 */
void monster_group_free(struct chunk *c, struct monster_group *group)
{
	/* Free the member list */
	while (group->member_list) {
		struct mon_group_list_entry *next = group->member_list->next;
		mem_free(group->member_list);
		group->member_list = next;
	}

	/* Free the heatmap */
	if (group->heatmap.grids) {
		int y;
		for (y = 0; y < c->height; y++) {
			mem_free(group->heatmap.grids[y]);
		}
		mem_free(group->heatmap.grids);
	}

	mem_free(group);
}

/**
 * Get the next available monster group index
 */
int monster_group_index_new(struct chunk *c)
{
	int index;

	for (index = 1; index < z_info->level_monster_max; index++) {
		if (!(c->monster_groups[index])) return index;
	}

	/* Fail, very unlikely */
	return 0;
}

/**
 * Get the index of a monster group
 */
int monster_group_index(struct monster_group *group)
{
	return group->index;
}

/**
 * Get a monster group from its index
 */
struct monster_group *monster_group_by_index(struct chunk *c, int index)
{
	return c->monster_groups[index];
}

/**
 * Change the group record of the index of a monster
 */
bool monster_group_change_index(struct chunk *c, int new, int old)
{
	int index = cave_monster(c, old)->group_info.index;
	struct monster_group *group = monster_group_by_index(c, index);
	struct mon_group_list_entry *entry = group->member_list;

	while (entry) {
		if (entry->midx == old) {
			entry->midx = new;
			return true;
		}
		entry = entry->next;
	}

	return false;
}


/**
 * Get the index of the leader of a monster group
 */
int monster_group_leader_idx(struct monster_group *group)
{
	return group->leader;
}

/**
 * Get the leader of a monster group
 */
struct monster *monster_group_leader(struct chunk *c,
									 struct monster_group *group)
{
	return cave_monster(c, group->leader);
}
