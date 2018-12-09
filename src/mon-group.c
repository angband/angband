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
 * Remove a monster from a monster group, deleting the group if it's empty
 */
void monster_remove_from_group(struct chunk *c, struct monster *mon)
{
	struct monster_group *group = c->monster_groups[mon->group_info.index];
	struct mon_group_list_entry *list_entry = group->member_list;

	/* Check if the first entry is the one we want */
	if (list_entry->midx == mon->midx) {
		if (!list_entry->next) {
			/* If it's the only monster, remove the group */
			monster_group_free(c, group);
			c->monster_groups[mon->group_info.index] = NULL;
			return;
		} else {
			/* Otherwise remove the first entry */
			group->member_list = list_entry->next;
			mem_free(list_entry);
			return;
		}
	}

	/* We have to look further down the member list */
	while (list_entry) {
		if (list_entry->next->midx == mon->midx) {
			list_entry->next = list_entry->next->next;
			mem_free(list_entry);
			return;
		}
	}

	/* Shouldn't get here */
	quit("Deleted monster not found in group");
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
 * Add a monster to an existing monster group
 */
void monster_add_to_group(struct chunk *c, struct monster *mon,
						  struct monster_group *group)
{
	struct mon_group_list_entry *list_entry;

	/* Confirm we're adding to the right group */
	assert(mon->group_info.index == group->index);

	/* Make a new list entry and add it to the start of the list */
	list_entry = mem_zalloc(sizeof(struct mon_group_list_entry));
	list_entry->midx = mon->midx;
	list_entry->next = group->member_list;
	group->member_list = list_entry;
}


/**
 * Make a monster group for a single monster
 */
void monster_group_start(struct chunk *c, struct monster *mon)
{
	struct monster_group *group = monster_group_new();

	/* Use the monster's group index */
	int index = mon->group_info.index;
	assert(index);

	/* Put the group in the group list */
	c->monster_groups[index] = group;

	/* Fill out the group */
	group->index = index;
	group->member_list = mem_zalloc(sizeof(struct mon_group_list_entry));
	group->member_list->midx = mon->midx;

	/* Write the index to the monster's group info */
	mon->group_info.index = index;
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
