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
#include "mon-group.h"
#include "monster.h"

struct monster_group *monster_group_new(void)
{
	struct monster_group *group = mem_zalloc(sizeof(struct monster_group));
	return group;
}

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
