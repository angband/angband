/*
 * File: stats.c
 * Purpose: Statistics collection on dungeon generation
 *
 * Copyright (c) 2008 Andrew Sidwell
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */
#include "angband.h"
#include "wizard.h"
#include "tvalsval.h"


#ifdef WITH_STATS

/*** Utility ***/

typedef struct
{
	char *key;
	char *value;
} keyv;

keyv results[10];
size_t no_results = 0;

void results_reset(void)
{
	size_t i;
	for (i = 0; i < no_results; i++)
	{
		FREE(results[i].key);
		FREE(results[i].value);
	}

	no_results = 0;
}

void result_add(const char *key, const char *value)
{
	results[no_results].key = string_make(key);
	results[no_results].value = string_make(value);
	no_results++;
}

void results_print_csv_titles(void)
{
	size_t i;
	for (i = 0; i < no_results; i++)
		printf("%s,", results[i].key);

	printf("\n");
}

void results_print_csv(void)
{
	size_t i;
	for (i = 0; i < no_results; i++)
		printf("%s,", results[i].value);

	printf("\n");
}

void results_print_csv_pair(const char *field1, const char *field2)
{
	size_t i;
	for (i = 0; i < no_results; i++)
	{
		if (strcmp(results[i].key, field1) == 0)
		{
			printf("%s,", results[i].value);
			break;
		}
	}

	for (i = 0; i < no_results; i++)
	{
		if (strcmp(results[i].key, field2) == 0)
		{
			printf("%s,", results[i].value);
			break;
		}
	}

	printf("\n");
}



/*** Statsgen ***/

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

	result_add("floor-objs", format("%f", (float) x / TRIES));
	result_add("floor-gold",  format("%f", (float) y / TRIES));
}


float avg_drop_by_level[] =
{
	/* 0   1      2      3      4      5      6      7      8      9   */
	10.7,  11.11, 11.53, 11.94, 12.35, 12.76, 13.18, 13.59, 14,    14.41,
	14.83, 15.24, 15.65, 16.06, 16.48, 16.89, 17.3,  17.71, 18.13, 18.54,
	18.95, 19.36, 19.78, 20.38, 21.2,  22.03, 22.85, 23.68, 24.5,  25.33,
	26.15, 26.98, 27.8,  28.63, 29.45, 30.28, 31.1,  31.93, 32.75, 33.58,
	34.4,  35.45, 37.1,  38.75, 40.4,  42.05, 43.7,  45.35, 47,    48.65,
	50.3,  51.95, 54.2,  57.5,  60.8,  64.1,  77,    93.5,  110,   125
};


double mon_drop;
double mon_gold;

inline static void stats_print_m(void)
{
	float gold_per_drop;

	if ((unsigned)p_ptr->depth >= N_ELEMENTS(avg_drop_by_level))
		gold_per_drop = avg_drop_by_level[N_ELEMENTS(avg_drop_by_level)-1];
	else
		gold_per_drop = avg_drop_by_level[p_ptr->depth];

	result_add("mon-drops", format("%f", mon_drop / TRIES));
	result_add("mon-gold", format("%f", (mon_gold * gold_per_drop) / TRIES));
}

void stats_monster(const monster_type *m_ptr)
{
	u32b f0 = r_info[m_ptr->r_idx].flags[0];
	float prob = 0.0;

	bool gold_ok = (!(f0 & (RF0_ONLY_ITEM)));
	bool item_ok = (!(f0 & (RF0_ONLY_GOLD)));

	if (f0 & RF0_DROP_60)  prob += 0.6/* 0.4*/;
	if (f0 & RF0_DROP_90)  prob += 0.9/* 0.6*/;
	if (f0 & RF0_DROP_1D2) prob += 1.5/* 1.0*/;
	if (f0 & RF0_DROP_2D2) prob += 3.0/* 2.0*/;
	if (f0 & RF0_DROP_3D2) prob += 4.5/* 3.0*/;
	if (f0 & RF0_DROP_4D2) prob += 6.0/* 4.0*/;

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
	static bool first = TRUE;
	size_t i, x, y;

	memset(o_count, 0, sizeof(o_count));
	memset(gold_count, 0, sizeof(gold_count));

	mon_gold = 0.0;
	mon_drop = 0.0;

	results_reset();
	result_add("level", format("%d", p_ptr->depth));


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

	stats_print_o();
	stats_print_m();

	if (first)
	{
		/* printf("level,mon-drops"); */
		results_print_csv_titles();
		first = FALSE;
	}

	/* results_print_csv_pair("level", "mon-drops"); */
	results_print_csv();

	do_cmd_redraw();
}



void stats_collect(void)
{
	int depth;

	for (depth = 0; depth < 101; depth += 5)
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
