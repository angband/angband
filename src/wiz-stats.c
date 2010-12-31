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
#include "cmds.h"
#include "wizard.h"
#include "object/tvalsval.h"


#ifdef WITH_STATS

/*** Utility ***/

typedef struct
{
	char *key;
	char *value;
} keyv;

static keyv results[10];
static size_t no_results = 0;

static void results_reset(void)
{
	size_t i;
	for (i = 0; i < no_results; i++)
	{
		FREE(results[i].key);
		FREE(results[i].value);
	}

	no_results = 0;
}

static void result_add(const char *key, const char *value)
{
	results[no_results].key = string_make(key);
	results[no_results].value = string_make(value);
	no_results++;
}

static void results_print_csv_titles(void)
{
	size_t i;
	for (i = 0; i < no_results; i++)
		printf("%s,", results[i].key);

	printf("\n");
}

static void results_print_csv(void)
{
	size_t i;
	for (i = 0; i < no_results; i++)
		printf("%s,", results[i].value);

	printf("\n");
}
#if 0
static void results_print_csv_pair(const char *field1, const char *field2)
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
#endif


/*** Statsgen ***/

/*
 * This file does some very simple operations; namely, it iterates over the
 * entire dungeon grid and collects statistics on what monsters, objects, and
 * terrain are being generated.  The results are very useful for balancing.
 */

#define TRIES	5000
static size_t o_count[TRIES];
static size_t gold_count[TRIES];


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


double mon_drop;
double mon_gold;

inline static void stats_print_m(void)
{
	float level_avg = 2*p_ptr->depth + 20;

	result_add("mon-drops", format("%f", mon_drop / TRIES));
	result_add("mon-gold", format("%f", mon_gold * level_avg / TRIES));
}

static void stats_monster(const monster_type *m_ptr)
{
	const monster_race r_ptr = &r_info[m_ptr->r_idx];
	float prob = 0.0;

	bool gold_ok = (!rf_has(r_ptr->flags, RF_ONLY_ITEM));
	bool item_ok = (!rf_has(r_ptr->flags, RF_ONLY_GOLD));

	if (rf_has(r_ptr->flags, RF_DROP_40))  prob += /*0.6*/ 0.4;
	if (rf_has(r_ptr->flags, RF_DROP_60))  prob += /*0.9*/ 0.6;

	if (rf_has(r_ptr->flags, RF_DROP_4)) prob += /*6.0*/ 4.0;
	if (rf_has(r_ptr->flags, RF_DROP_3)) prob += /*4.5*/ 3.0;
	if (rf_has(r_ptr->flags, RF_DROP_2)) prob += /*3.0*/ 2.0;
	if (rf_has(r_ptr->flags, RF_DROP_1)) prob += /*1.5*/ 1.0;

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
static void stats_collect_level(void)
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
		cave_generate(cave, p_ptr);

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
				if (cave->m_idx[y][x])
					stats_monster(&mon_list[cave->m_idx[y][x]]);
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

		stats_collect_level();
	}
}


#if 0
/*
 * Deliberately left unlinked, but useful in e.g. gdb to print the dungeon
 * when debugging, using "call print_dun()".
 */
static void print_dun(void)
{
	int y, x;

	/* Get stats on objects */
	for (y = 1; y < DUNGEON_HGT - 1; y++)
	{
		for (x = 1; x < DUNGEON_WID - 1; x++)
		{
			char feat = 'A' + cave->feat[y][x];
			printf("%c", feat);
		}
		printf("\n");
	}
}
#endif

#else /* WITH_STATS */

void stats_collect(void)
{
	msg("Statistics generation not turned on in this build.");
}

#endif /* WITH_STATS */
