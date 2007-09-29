/*
 * File: stats.c
 * Purpose: Statistics collection on dungeon generation
 *
 * Copyright (c) 2007 Angband Contributors
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
#include "wizard.h"


#ifdef WITH_STATS

/*
 * This file does some very simple operations; namely, it iterates over the
 * entire dungeon grid and collects statistics on what monsters, objects, and
 * terrain are being generated.  The results are very useful for balancing.
 */

#define TRIES	10000
size_t o_count[TRIES];
size_t gold_count[TRIES];


inline static void stats_print_o(void)
{
	int i;
	u64b x = 0, y = 0;

	for (i = 0; i < TRIES; i++)
		x += o_count[i];

	for (i = 0; i < TRIES; i++)
		y += gold_count[i];

	printf("floor,%f\n", (float) x / TRIES);
	printf("gold,%f\n", (float) y / TRIES);
}




double mon_drop;
double mon_gold;

inline static void stats_print_m(void)
{
	printf("drops,%f\n", mon_drop / TRIES);
	printf("goldd,%f\n", mon_gold / TRIES);
}

void stats_monster(const monster_type *m_ptr)
{
	u32b f1 = r_info[m_ptr->r_idx].flags1;
	float prob = 0.0;

	bool gold_ok = (!(f1 & (RF1_ONLY_ITEM)));
	bool item_ok = (!(f1 & (RF1_ONLY_GOLD)));

	if (f1 & RF1_DROP_60)  prob +=  0.6;
	if (f1 & RF1_DROP_90)  prob +=  0.9;
	if (f1 & RF1_DROP_1D2) prob +=  1.5;
	if (f1 & RF1_DROP_2D2) prob +=  3.0;
	if (f1 & RF1_DROP_3D2) prob +=  4.5;
	if (f1 & RF1_DROP_4D2) prob +=  6.0;

	if (gold_ok && item_ok)
	{
		mon_gold += ((float) prob) / 2;
		mon_drop += ((float) prob) / 2;
	}
	else if (gold_ok && !item_ok)
		mon_gold += prob;
	else
		mon_drop += prob;
}







/*
 * This is the entry point for generation statistics.
 */
void stats_collect2(void)
{
	size_t i, x, y;

	memset(o_count, 0, sizeof(o_count));
	memset(gold_count, 0, sizeof(gold_count));

	mon_gold = 0.0;
	mon_drop = 0.0;

	for (i = 0; i < TRIES; i++)
	{
		generate_cave();

		/* Get stats on objects */
		for (y = 1; y < DUNGEON_HGT - 1; y++)
		{
			for (x = 1; x < DUNGEON_WID - 1; x++)
			{
				const object_type *obj = get_first_object(y, x);

				if (obj) do
				{
					if (obj->tval == TV_GOLD) gold_count[i] += obj->pval;
					else o_count[i]++;
				}
				while ((obj = get_next_object(obj)));
			}
		}

		/* Get stats on monsters */
		for (y = 1; y < DUNGEON_HGT - 1; y++)
		{
			for (x = 1; x < DUNGEON_WID - 1; x++)
			{
				if (cave_m_idx[y][x])
					stats_monster(&mon_list[cave_m_idx[y][x]]);
			}
		}
	}

	printf("level,%d\n", p_ptr->depth);
	stats_print_o();
	stats_print_m();
	printf("\n");

	do_cmd_redraw();
}



void stats_collect(void)
{
	int depth;

	for (depth = 40; depth < 100; depth += 5)
	{
		p_ptr->depth = depth;
		if (p_ptr->depth == 0) p_ptr->depth = 1;

		stats_collect2();
	}
}


/*
 * Deliberately left unlinked, but useful in e.g. gdb to print the dungeon
 * when debugging, using "call print_dun()".
 */
void print_dun(void)
{
	int y, x;

	/* Get stats on objects */
	for (y = 1; y < DUNGEON_HGT - 1; y++)
	{
		for (x = 1; x < DUNGEON_WID - 1; x++)
		{
			char feat = 'A' + cave_feat[y][x];
			printf("%c", feat);
		}
		printf("\n");
	}
}

#else /* WITH_STATS */

void stats_collect(void)
{
	msg_print("Statistics generation not turned on in this build.");
}

#endif /* WITH_STATS */
