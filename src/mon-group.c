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
#include "mon-make.h"
#include "mon-util.h"
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

	mem_free(group);
}

/**
 * Break a monster group into race-based pieces
 */
static void monster_group_split(struct chunk *c, struct monster_group *group,
								struct monster *leader)
{
	struct mon_group_list_entry *entry = group->member_list;

	/* Keep a list of groups made for easy checking */
	int *temp = mem_zalloc(z_info->level_monster_max * sizeof(int));
	int current = 0;

	/* Go through the monsters in the group */
	for (entry = group->member_list; entry; entry = entry->next) {
		int i;
		struct monster *mon = &c->monsters[entry->midx];

		/* Check all groups to see if they contain a monster of this race */
		for (i = 0; i < current; i++) {
			struct monster_group *new_group = c->monster_groups[temp[i]];

			/* If it's the right group, add the monster and stop checking */
			if (c->monsters[new_group->member_list->midx].race == mon->race) {
				mon->group_info[PRIMARY_GROUP].index = temp[i];
				mon->group_info[PRIMARY_GROUP].role = MON_GROUP_MEMBER;
				monster_add_to_group(c, mon, new_group);
				break;
			}
		}

		/* If the monster's still in the old group, make a new one */
		if (mon->group_info[PRIMARY_GROUP].index == group->index) {
			monster_group_start(c, mon, 0);

			/* Store the new index */
			temp[current++] = mon->group_info[PRIMARY_GROUP].index;
		}
	}
	mem_free(temp);
}


/**
 * Handle the leader of a group being removed
 */
static void monster_group_remove_leader(struct chunk *c, struct monster *leader,
										struct monster_group *group)
{
	struct mon_group_list_entry *list_entry = group->member_list;
	int poss_leader = 0;

	/* Look for another leader */
	while (list_entry) {
		struct monster *mon = cave_monster(c, list_entry->midx);

		if (!mon) {
			list_entry = list_entry->next;
			continue;
		}

		/* Monsters of the same race can take over as leader */
		if ((leader->race == mon->race) && !poss_leader
			&& (mon->group_info[PRIMARY_GROUP].role != MON_GROUP_SUMMON)) {
			poss_leader = mon->midx;
		}

		/* Uniques always take over */
		if (rf_has(mon->race->flags, RF_UNIQUE)) {
			poss_leader = mon->midx;
		}
		list_entry = list_entry->next;
	}

	/* If no new leader, group fractures and old group is removed */
	if (!poss_leader) {
		monster_group_split(c, group, leader);
		c->monster_groups[group->index] = NULL;
		monster_group_free(c, group);
	} else {
		/* If there is a successor, appoint them and finalise changes */
		group->leader = poss_leader;
		list_entry = group->member_list;
		assert(list_entry);
		while (list_entry) {
			struct monster *mon = cave_monster(c, list_entry->midx);

			/* Record the leader */
			if (mon->midx == poss_leader) {
				mon->group_info[PRIMARY_GROUP].role = MON_GROUP_LEADER;
				break;
			}
			list_entry = list_entry->next;
		}
	}
	monster_groups_verify(c);
}

/**
 * Remove a monster from a monster group, deleting the group if it's empty.
 * Deal with removal of the leader.
 */
void monster_remove_from_groups(struct chunk *c, struct monster *mon)
{
	int i;
	struct monster_group *group;
	struct mon_group_list_entry *list_entry;

	for (i = 0; i < 2; i++) {
		group =	c->monster_groups[mon->group_info[i].index];

		/* Most monsters won't have a second group */
		if (!group) return;
		list_entry = group->member_list;

		/* Check if the first entry is the one we want */
		if (list_entry->midx == mon->midx) {
			if (!list_entry->next) {
				/* If it's the only monster, remove the group */
				monster_group_free(c, group);
				c->monster_groups[mon->group_info[i].index] = NULL;
				continue;
			} else {
				/* Otherwise remove the first entry */
				group->member_list = list_entry->next;
				mem_free(list_entry);
				if (group->leader == mon->midx) {
					monster_group_remove_leader(c, mon, group);
				}
				continue;
			}
		}

		/* Check - necessary? */
		if (list_entry->next == NULL) {
			quit_fmt("Bad group: index=%d, monster=%d",
					 mon->group_info[i].index, mon->midx);
		}

		/* We have to look further down the member list */
		while (list_entry) {
			if (list_entry->next->midx == mon->midx) {
				struct mon_group_list_entry *remove = list_entry->next;
				list_entry->next = list_entry->next->next;
				mem_free(remove);
				if (group->leader == mon->midx) {
					monster_group_remove_leader(c, mon, group);
				}
				break;
			}
			list_entry = list_entry->next;
		}
	}
	monster_groups_verify(c);
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
	assert(mon->group_info[PRIMARY_GROUP].index == group->index);

	/* Make a new list entry and add it to the start of the list */
	list_entry = mem_zalloc(sizeof(struct mon_group_list_entry));
	list_entry->midx = mon->midx;
	list_entry->next = group->member_list;
	group->member_list = list_entry;
}


/**
 * Make a monster group for a single monster
 */
void monster_group_start(struct chunk *c, struct monster *mon, int which)
{
	/* Get a group and a group index */
	struct monster_group *group = monster_group_new();
	int index = monster_group_index_new(c);
	assert(index);

	/* Put the group in the group list */
	c->monster_groups[index] = group;

	/* Fill out the group */
	group->index = index;
	group->leader = mon->midx;
	group->member_list = mem_zalloc(sizeof(struct mon_group_list_entry));
	group->member_list->midx = mon->midx;

	/* Write the index to the monster's group info, make it leader */
	mon->group_info[which].index = index;
	mon->group_info[which].role = MON_GROUP_LEADER;
}

/**
 * Assign a monster to a monster group
 */
void monster_group_assign(struct chunk *c, struct monster *mon,
						  struct monster_group_info *info, bool loading)
{
	int index = info[PRIMARY_GROUP].index;
	struct monster_group *group = monster_group_by_index(c, index);

	if (!loading) {
		/* For newly created monsters, use the group start and add functions */
		if (group) {
			monster_add_to_group(c, mon, group);
		} else {
			monster_group_start(c, mon, 0);
		}
	} else {
		/* For loading from a savefile, build by hand */
		int i;

		for (i = 0; i < GROUP_MAX; i++) {
			struct mon_group_list_entry *entry = mem_zalloc(sizeof(*entry));

			/* Check the index */
			index = info[i].index;
			if (!index) {
				if (i == PRIMARY_GROUP) {
					/* Everything should have a primary group */
					quit_fmt("Monster %d has no group", mon->midx);
				} else {
					/* Plenty of things have no summon group */
					mem_free(entry);
					return;
				}
			}

			/* Fill out the group, creating if necessary */
			group = monster_group_by_index(c, index);
			if (!group) {
				group = monster_group_new();
				group->index = index;
				c->monster_groups[index] = group;
			}
			if (info[i].role == MON_GROUP_LEADER) {
				group->leader = mon->midx;
			}

			/* Add this monster */
			entry->midx = mon->midx;
			entry->next = group->member_list;
			group->member_list = entry;
		}
	}
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
 * Change the group record of the index of a monster (for one or two groups)
 */
bool monster_group_change_index(struct chunk *c, int new, int old)
{
	int index0 = cave_monster(c, old)->group_info[PRIMARY_GROUP].index;
	int index1 = cave_monster(c, old)->group_info[SUMMON_GROUP].index;
	struct monster_group *group0 = monster_group_by_index(c, index0);
	struct monster_group *group1 = monster_group_by_index(c, index1);
	struct mon_group_list_entry *entry = group0->member_list;

	if (group0->leader == old) {
		group0->leader = new;
	}
	while (entry) {
		if (entry->midx == old) {
			entry->midx = new;
			if (!group1) {
				return true;
			}
		}
		entry = entry->next;
	}

	if (group1) {
		if (group1->leader == old) {
			group1->leader = new;
		}
		entry = group1->member_list;
		while (entry) {
			if (entry->midx == old) {
				entry->midx = new;
				return true;
			}
			entry = entry->next;
		}
	}

	return false;
}

/**
 * Get the group of summons of a monster
 */
struct monster_group *summon_group(struct chunk *c, int midx)
{
	struct monster *mon = cave_monster(c, midx);
	int index;

	if (!mon) return NULL;

	/* If the monster is leader of its primary group, return that group */
	if (mon->group_info[PRIMARY_GROUP].role == MON_GROUP_LEADER) {
		index = mon->group_info[PRIMARY_GROUP].index;
	} else {
		/* Get the (distinct) summon group */
		index = mon->group_info[SUMMON_GROUP].index;

		/* Make a group if there isn't one already */
		if (!index) {
			monster_group_start(c, mon, 1);
			index = mon->group_info[SUMMON_GROUP].index;
		}
	}

	return monster_group_by_index(c, index);
}


/**
 * Monster who is aware of the player tries to let its group know
 */
void monster_group_rouse(struct chunk *c, struct monster *mon)
{
	int index = mon->group_info[PRIMARY_GROUP].index;
	struct monster_group *group = c->monster_groups[index];
	struct mon_group_list_entry *entry = group->member_list;

	/* Not aware means don't rouse */
	if (!mflag_has(mon->mflag, MFLAG_AWARE)) return;

	while (entry) {
		struct monster *friend = &c->monsters[entry->midx];
		struct loc fgrid = friend->grid;
		if (friend->m_timed[MON_TMD_SLEEP] && monster_can_see(c, mon, fgrid)) {
			int dist = distance(mon->grid, fgrid);

			/* Closer means more likely to be roused */
			if (one_in_(dist * 20)) {
				monster_wake(friend, true, 50);
			}
		}
		entry = entry->next;
	}
}

/**
 * Get the size of a monster's primary group
 */
int monster_primary_group_size(struct chunk *c, const struct monster *mon)
{
	int count = 0;
	int index = mon->group_info[PRIMARY_GROUP].index;
	struct monster_group *group = c->monster_groups[index];
	struct mon_group_list_entry *entry = group->member_list;

	while (entry) {
		count++;
		entry = entry->next;
	}
	return count;
}

/**
 * Find a group monster which is tracking
 */
struct monster *group_monster_tracking(struct chunk *c,
									   const struct monster *mon)
{
	int index = mon->group_info[PRIMARY_GROUP].index;
	struct monster_group *group = c->monster_groups[index];
	struct mon_group_list_entry *entry = group->member_list;

	while (entry) {
		struct monster *tracker = cave_monster(c, entry->midx);
		if (mflag_has(tracker->mflag, MFLAG_TRACKING)) return tracker;
		entry = entry->next;
	}

	return NULL;
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
struct monster *monster_group_leader(struct chunk *c, struct monster *mon)
{
	int index = mon->group_info[PRIMARY_GROUP].index;
	struct monster_group *group = c->monster_groups[index];
	return cave_monster(c, group->leader);
}

/**
 * Verify the integrity of all the monster groups
 */
void monster_groups_verify(struct chunk *c)
{
	int i;

	for (i = 0; i < z_info->level_monster_max; i++) {
		if (c->monster_groups[i]) {
			struct monster_group *group = c->monster_groups[i];
			struct mon_group_list_entry *entry = group->member_list;
			while (entry) {
				struct monster *mon = cave_monster(c, entry->midx);
				struct monster_group_info *info = mon->group_info;
				if (info[PRIMARY_GROUP].index != i) {
					if (info[SUMMON_GROUP].index) {
						if (info[SUMMON_GROUP].index != i) {
							quit_fmt("Bad group index: group: %d, monster: %d",
									 i, info[SUMMON_GROUP].index);
						}
						if (info[SUMMON_GROUP].role != MON_GROUP_LEADER) {
							quit_fmt("Bad monster role: group: %d, monster: %d",
									 i, info[SUMMON_GROUP].index);
						}
					} else {
						quit_fmt("Bad group index: group: %d, monster: %d",
								 i, info[PRIMARY_GROUP].index);
					}
				}
				entry = entry->next;
			}
		}
	}
}
