/*
 * File: main-stats.c
 * Purpose: Pseudo-UI for stats generation (borrows heavily from main-test.c)
 *
 * Copyright (c) 2010-11 Robert Au <myshkin+angband@durak.net>
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

#ifdef USE_STATS

#include "birth.h"
#include "buildid.h"
#include "init.h"
#include "object/pval.h"
#include "object/tvalsval.h"
#include "stats/db.h"
#include "stats/structs.h"
#include <stddef.h>
#include <time.h>

#define OBJ_FEEL_MAX	 11
#define MON_FEEL_MAX 	 10
#define LEVEL_MAX 		101
#define TOP_DICE		 21 /* highest catalogued values for wearables */
#define TOP_SIDES		 11
#define TOP_AC			146
#define TOP_PLUS		 56
#define TOP_POWER		999
#define TOP_PVAL		 25
#define RUNS_PER_CHECKPOINT	10000

/* For ref, e_max is 128, a_max is 136, r_max is ~650,
	ORIGIN_STATS is 14, OF_MAX is ~120 */

/* There are 416 kinds, of which about 150-200 are wearable */

static int randarts = 0;
static int no_selling = 0;
static int save = 1;
static u32b num_runs = 1;
static bool quiet = FALSE;
static int nextkey = 0;
static int running_stats = 0;
static char *ANGBAND_DIR_STATS;

static int *consumables_index;
static int *wearables_index;
static int *pval_flags_index;
static int wearable_count = 0;
static int consumable_count = 0;
static int pval_flags_count = 0;

struct wearables_data {
	u32b count;
	u32b dice[TOP_DICE][TOP_SIDES];
	u32b ac[TOP_AC];
	u32b hit[TOP_PLUS];
	u32b dam[TOP_PLUS];
/*	u32b power[TOP_POWER]; not enough memory - add it later in bands */
	u32b *egos;
	u32b flags[OF_MAX];
	u32b *pval_flags[TOP_PVAL];
};

static struct level_data {
	u32b *monsters;
/*  u32b *vaults;  Add these later - requires passing into generate.c
	u32b *pits; */
	u32b obj_feelings[OBJ_FEEL_MAX];
	u32b mon_feelings[MON_FEEL_MAX];
	long long gold[ORIGIN_STATS];
	u32b *artifacts[ORIGIN_STATS];
	u32b *consumables[ORIGIN_STATS];
	struct wearables_data *wearables[ORIGIN_STATS];
} level_data[LEVEL_MAX];

static void create_indices()
{
	int i;

	consumables_index = C_ZNEW(z_info->k_max, int);
	wearables_index = C_ZNEW(z_info->k_max, int);
	pval_flags_index = C_ZNEW(OF_MAX, int);

	for (i = 0; i < z_info->k_max; i++) {

		object_type object_type_body;
		object_type *o_ptr = &object_type_body;
		object_kind *kind = &k_info[i];
		o_ptr->tval = kind->tval;

		if (! kind->name) continue;

		if (wearable_p(o_ptr))
			wearables_index[i] = ++wearable_count;
		else
			consumables_index[i] = ++consumable_count;
	}

	for (i = 0; i < OF_MAX; i++)
		if (flag_uses_pval(i))
			pval_flags_index[i] = ++pval_flags_count;
}

static void alloc_memory()
{
	int i, j, k, l;

	for (i = 0; i < LEVEL_MAX; i++) {
		level_data[i].monsters = C_ZNEW(z_info->r_max, u32b);
/*		level_data[i].vaults = C_ZNEW(z_info->v_max, u32b);
		level_data[i].pits = C_ZNEW(z_info->pit_max, u32b); */

		for (j = 0; j < ORIGIN_STATS; j++) {
			level_data[i].artifacts[j] = C_ZNEW(z_info->a_max, u32b);
			level_data[i].consumables[j] = C_ZNEW(consumable_count + 1, u32b);
			level_data[i].wearables[j]
				= C_ZNEW(wearable_count + 1, struct wearables_data);

			for (k = 0; k < wearable_count + 1; k++) {
				level_data[i].wearables[j][k].egos
					= C_ZNEW(z_info->e_max, u32b);
				for (l = 0; l < TOP_PVAL; l++)
					level_data[i].wearables[j][k].pval_flags[l]
						= C_ZNEW(pval_flags_count + 1, u32b);
			}
		}
	}
}

static void free_stats_memory(void)
{
	int i, j, k, l;
	for (i = 0; i < LEVEL_MAX; i++) {
		mem_free(level_data[i].monsters);
/*		mem_free(level_data[i].vaults);
 		mem_free(level_data[i].pits); */
		for (j = 0; j < ORIGIN_STATS; j++) {
			mem_free(level_data[i].artifacts[j]);
			mem_free(level_data[i].consumables[j]);
			for (k = 0; k < wearable_count + 1; k++) {
				for (l = 0; l < TOP_PVAL; l++) {
					mem_free(level_data[i].wearables[j][k].pval_flags[l]);
				}
				mem_free(level_data[i].wearables[j][k].egos);
			}
			mem_free(level_data[i].wearables[j]);
		}
	}
	mem_free(consumables_index);
	mem_free(wearables_index);
	mem_free(pval_flags_index);
}

/* Copied from birth.c:generate_player() */
static void generate_player_for_stats()
{
	OPT(birth_randarts) = randarts;
	OPT(birth_no_selling) = no_selling;
	OPT(birth_no_stacking) = FALSE;
	OPT(auto_more) = TRUE;

	p_ptr->wizard = 1; /* Set wizard mode on */

	p_ptr->psex = 0;   /* Female  */
	p_ptr->race = races;  /* Human   */
	p_ptr->class = classes; /* Warrior */

	/* Level 1 */
	p_ptr->max_lev = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = p_ptr->race->r_exp + p_ptr->class->c_exp;

	/* Hitdice */
	p_ptr->hitdie = p_ptr->race->r_mhp + p_ptr->class->c_mhp;

	/* Initial hitpoints -- high just to be safe */
	p_ptr->mhp = p_ptr->chp = 2000;

	/* Pre-calculate level 1 hitdice */
	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Set age/height/weight */
	p_ptr->ht = p_ptr->ht_birth = 66;
	p_ptr->wt = p_ptr->wt_birth = 150;
	p_ptr->age = 14;

	/* Set social class and (null) history */
	p_ptr->history = get_history(p_ptr->race->history, &p_ptr->sc);
	p_ptr->sc_birth = p_ptr->sc;
}

static void initialize_character(void)
{
	u32b seed;

	if (!quiet) {
		printf("[I  ]\b\b\b\b\b");
		fflush(stdout);
	}

	seed = (time(NULL));
	Rand_quick = FALSE;
	Rand_state_init(seed);

	player_init(p_ptr);
	generate_player_for_stats();

	seed_flavor = randint0(0x10000000);
	seed_town = randint0(0x10000000);
	seed_randart = randint0(0x10000000);

	if (randarts)
	{
		do_randart(seed_randart, TRUE);
	}

	store_reset();
	flavor_init();
	p_ptr->playing = TRUE;
	p_ptr->autosave = FALSE;
	cave_generate(cave, p_ptr);
}

static void dispose_character(void)
{
	mem_free(p_ptr->history);
}

static void kill_all_monsters(int level)
{
	int i;

	for (i = cave_monster_max(cave) - 1; i >= 1; i--) {
		const monster_type *m_ptr = cave_monster(cave, i);
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		level_data[level].monsters[m_ptr->r_idx]++;

		monster_death(i, TRUE);

		if (rf_has(r_ptr->flags, RF_UNIQUE))
			r_ptr->max_num = 0;
	}
}

static void unkill_uniques(void)
{
	int i;

	if (!quiet) {
		printf("[U  ]\b\b\b\b\b");
		fflush(stdout);
	}

	for (i = 0; i < z_info->r_max; i++) {
		monster_race *r_ptr = &r_info[i];

		if (rf_has(r_ptr->flags, RF_UNIQUE))
			r_ptr->max_num = 1;
	}
}

static void reset_artifacts(void)
{
	int i;

	if (!quiet) {
		printf("[R  ]\b\b\b\b\b");
		fflush(stdout);
	}

	for (i = 0; i < z_info->a_max; i++)
		a_info[i].created = FALSE;

}

static void log_all_objects(int level)
{
	int x, y, i;

	for (y = 1; y < DUNGEON_HGT - 1; y++) {
		for (x = 1; x < DUNGEON_WID - 1; x++) {
			object_type *o_ptr = get_first_object(y, x);

			if (o_ptr) do {
			/*	u32b o_power = 0; */

				/* Mark object as fully known */
				object_notice_everything(o_ptr);

/*				o_power = object_power(o_ptr, FALSE, NULL, TRUE); */

				/* Capture gold amounts */
				if (o_ptr->tval == TV_GOLD)
					level_data[level].gold[o_ptr->origin] += o_ptr->pval[DEFAULT_PVAL];

				/* Capture artifact drops */
				if (o_ptr->artifact)
					level_data[level].artifacts[o_ptr->origin][o_ptr->artifact->aidx]++;

				/* Capture kind details */
				if (wearable_p(o_ptr)) {
					struct wearables_data *w
						= &level_data[level].wearables[o_ptr->origin][wearables_index[o_ptr->kind->kidx]];

					w->count++;
					w->dice[MIN(o_ptr->dd, TOP_DICE - 1)][MIN(o_ptr->ds, TOP_SIDES - 1)]++;
					w->ac[MIN(MAX(o_ptr->ac + o_ptr->to_a, 0), TOP_AC - 1)]++;
					w->hit[MIN(MAX(o_ptr->to_h, 0), TOP_PLUS - 1)]++;
					w->dam[MIN(MAX(o_ptr->to_d, 0), TOP_PLUS - 1)]++;

					/* Capture egos */
					if (o_ptr->ego)
						w->egos[o_ptr->ego->eidx]++;
					/* Capture object flags */
					for (i = of_next(o_ptr->flags, FLAG_START); i != FLAG_END;
							i = of_next(o_ptr->flags, i + 1)) {
						w->flags[i]++;
						if (flag_uses_pval(i)) {
							int p = o_ptr->pval[which_pval(o_ptr, i)];
							w->pval_flags[MIN(MAX(p, 0), TOP_PVAL - 1)][pval_flags_index[i]]++;
						}
					}
				} else
					level_data[level].consumables[o_ptr->origin][consumables_index[o_ptr->kind->kidx]]++;
			}
			while ((o_ptr = get_next_object(o_ptr)));
		}
	}
}

static void descend_dungeon(void)
{
	int level;
	u16b obj_f, mon_f;

	clock_t last = 0;

	clock_t wait = CLOCKS_PER_SEC / 5;

	for (level = 1; level < LEVEL_MAX; level++)
	{
		if (!quiet) {
			clock_t now = clock();
			if (now - last > wait) {
				printf("[%3d]\b\b\b\b\b", level);
				fflush(stdout);
				last = now;
			}
		}

		dungeon_change_level(level);
		cave_generate(cave, p_ptr);

		/* Store level feelings */
		obj_f = cave->feeling / 10;
		mon_f = cave->feeling - (10 * obj_f);
		level_data[level].obj_feelings[MIN(obj_f, OBJ_FEEL_MAX - 1)]++;
		level_data[level].mon_feelings[MIN(mon_f, MON_FEEL_MAX - 1)]++;

		kill_all_monsters(level);
		log_all_objects(level);
	}
}

static void prep_output_dir(void)
{
	size_t size = strlen(ANGBAND_DIR_USER) + strlen(PATH_SEP) + 6;
	ANGBAND_DIR_STATS = mem_alloc(size);
	strnfmt(ANGBAND_DIR_STATS, size,
		"%s%sstats", ANGBAND_DIR_USER,PATH_SEP);

	if (dir_create(ANGBAND_DIR_STATS))
	{
		return;
	}
	else
	{
		quit("Couldn't create stats directory!");
	}
}

/**
 * Caller is responsible for prepping and finalizing flags_stmt, which
 * should have two parameters. 
 */
static int stats_dump_oflags(sqlite3_stmt *flags_stmt, int idx, 
	bitflag flags[OF_SIZE]) 
{
	int err, flag;

	err = sqlite3_bind_int(flags_stmt, 1, idx);
	if (err) return err;
	for (flag = of_next(flags, FLAG_START); flag != FLAG_END;
		flag = of_next(flags, flag + 1))
	{
		err = sqlite3_bind_int(flags_stmt, 2, flag);
		if (err) return err;
		STATS_DB_STEP_RESET(flags_stmt)
	}


	return SQLITE_OK;
}

static int stats_dump_artifacts(void)
{
	int err, idx, i, flag;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *pval_flags_stmt;

	strnfmt(sql_buf, 256, "INSERT INTO artifact_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO artifact_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO artifact_pval_flags_map VALUES (?,?,?);");
	err = stats_db_stmt_prep(&pval_flags_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->a_max; idx++)
	{
		artifact_type *a_ptr = &a_info[idx];

		if (!a_ptr->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, a_ptr->name, 
			strlen(a_ptr->name), SQLITE_STATIC);
		if (err) return err;
		err = stats_db_bind_ints(info_stmt, 14, 2, 
			a_ptr->tval, a_ptr->sval, a_ptr->weight,
			a_ptr->cost, a_ptr->alloc_prob, a_ptr->alloc_min,
			a_ptr->alloc_max, a_ptr->ac, a_ptr->dd,
			a_ptr->ds, a_ptr->to_h, a_ptr->to_d,
			a_ptr->to_a, a_ptr->effect);
		STATS_DB_STEP_RESET(info_stmt)

		err = stats_dump_oflags(flags_stmt, idx, a_ptr->flags);
		if (err) return err;

		for (i = 0; i < a_ptr->num_pvals; i++)
		{
			for (flag = of_next(a_ptr->pval_flags[i], FLAG_START);
				flag != FLAG_END;
				flag = of_next(a_ptr->pval_flags[i], flag + 1))
			{
				err = stats_db_bind_ints(pval_flags_stmt, 3, 0, 
					idx, flag, a_ptr->pval[i]);
				if (err) return err;
				STATS_DB_STEP_RESET(pval_flags_stmt)
			}
		}
	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(pval_flags_stmt)

	return SQLITE_OK;
}

static int stats_dump_egos(void)
{
	int err, idx, flag, i;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *pval_flags_stmt, *type_stmt;

	strnfmt(sql_buf, 256, "INSERT INTO ego_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO ego_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO ego_pval_flags_map VALUES (?,?,?,?);");
	err = stats_db_stmt_prep(&pval_flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO ego_type_map VALUES (?,?,?,?);");
	err = stats_db_stmt_prep(&type_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->e_max; idx++)
	{
		ego_item_type *e_ptr = &e_info[idx];

		if (!e_ptr->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, e_ptr->name, 
			strlen(e_ptr->name), SQLITE_STATIC);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 3, e_ptr->to_h); 
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 4, e_ptr->to_d); 
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 5, e_ptr->to_a); 
		if (err) return err;
		err = stats_db_bind_ints(info_stmt, 9, 5, 
			e_ptr->cost, e_ptr->level, e_ptr->rarity,
			e_ptr->rating, e_ptr->num_pvals, e_ptr->min_to_h, 
			e_ptr->min_to_d, e_ptr->min_to_a, e_ptr->xtra);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		err = stats_dump_oflags(flags_stmt, idx, e_ptr->flags);
		if (err) return err;

		for (i = 0; i < e_ptr->num_pvals; i++)
		{
			for (flag = of_next(e_ptr->pval_flags[i], FLAG_START);
				flag != FLAG_END;
				flag = of_next(e_ptr->pval_flags[i], flag + 1))
			{
				err = stats_db_bind_ints(pval_flags_stmt, 3, 0, 
					idx, flag, e_ptr->min_pval[i]);
				if (err) return err;
				err = stats_db_bind_rv(pval_flags_stmt, 4,
					e_ptr->pval[i]);
				if (err) return err;
				STATS_DB_STEP_RESET(pval_flags_stmt)
			}
		}

		for (i = 0; i < EGO_TVALS_MAX; i++)
		{
			err = stats_db_bind_ints(type_stmt, 4, 0,
				idx, e_ptr->tval[i], e_ptr->min_sval[i], 
				e_ptr->max_sval[i]);
			if (err) return err;
			STATS_DB_STEP_RESET(type_stmt)
		}

	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(pval_flags_stmt)
	STATS_DB_FINALIZE(type_stmt)

	return SQLITE_OK;
}

static int stats_dump_objects(void)
{
	int err, idx, i, flag;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *pval_flags_stmt;

	strnfmt(sql_buf, 256, "INSERT INTO object_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO object_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO object_pval_flags_map VALUES (?,?,?);");
	err = stats_db_stmt_prep(&pval_flags_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->k_max; idx++)
	{
		object_kind *k_ptr = &k_info[idx];

		if (!k_ptr->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, k_ptr->name, 
			strlen(k_ptr->name), SQLITE_STATIC);
		if (err) return err;
		err = stats_db_bind_ints(info_stmt, 14, 2, 
			k_ptr->tval, k_ptr->sval, k_ptr->level, k_ptr->weight,
			k_ptr->cost, k_ptr->ac, k_ptr->dd, k_ptr->ds,
			k_ptr->alloc_prob, k_ptr->alloc_min,
			k_ptr->alloc_max, k_ptr->effect,
			k_ptr->gen_mult_prob, k_ptr->stack_size);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 17, k_ptr->to_h);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 18, k_ptr->to_d);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 19, k_ptr->to_a);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 20, k_ptr->charge);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 21, k_ptr->time);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		err = stats_dump_oflags(flags_stmt, idx, k_ptr->flags);
		if (err) return err;

		for (i = 0; i < k_ptr->num_pvals; i++)
		{
			for (flag = of_next(k_ptr->pval_flags[i], FLAG_START);
				flag != FLAG_END;
				flag = of_next(k_ptr->pval_flags[i], flag + 1))
			{
				err = stats_db_bind_ints(pval_flags_stmt, 2, 0, 
					idx, flag);
				if (err) return err;
				err = stats_db_bind_rv(pval_flags_stmt, 3,
					k_ptr->pval[i]);
				if (err) return err;
				STATS_DB_STEP_RESET(pval_flags_stmt)
			}
		}
	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(pval_flags_stmt)

	/* Handle object_base */
	strnfmt(sql_buf, 256, "INSERT INTO object_base_info VALUES (?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO object_base_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	idx = 0;
	for (idx = 0; idx < TV_MAX; idx++)
	{
		object_base *kb_ptr = &kb_info[idx];

		if (!kb_ptr->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, kb_ptr->name,
			strlen(kb_ptr->name), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		for (flag = of_next(kb_ptr->flags, FLAG_START);
			flag != FLAG_END;
			flag = of_next(kb_ptr->flags, flag + 1))
		{
			err = stats_db_bind_ints(flags_stmt, 2, 0,
				idx, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(flags_stmt)
		}
	}
	
	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)

	return SQLITE_OK;
}

static int stats_dump_monsters(void)
{
	int err, idx, flag;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *spell_flags_stmt;
	monster_base *rb_ptr;

	strnfmt(sql_buf, 256, "INSERT INTO monster_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO monster_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO monster_spell_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&spell_flags_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->r_max; idx++)
	{
		monster_race *r_ptr = &r_info[idx];

		/* Skip empty entries */
		if (!r_ptr->name) continue;

		err = stats_db_bind_ints(info_stmt, 10, 0, idx,
			r_ptr->ac, r_ptr->sleep, r_ptr->speed, r_ptr->mexp,
			r_ptr->avg_hp, r_ptr->freq_innate, r_ptr->freq_spell,
			r_ptr->level, r_ptr->rarity);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 11, r_ptr->name,
			strlen(r_ptr->name), SQLITE_STATIC);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 12, r_ptr->base->name,
			strlen(r_ptr->base->name), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		for (flag = rf_next(r_ptr->flags, FLAG_START);
			flag != FLAG_END;
			flag = rf_next(r_ptr->flags, flag + 1))
		{
			err = stats_db_bind_ints(flags_stmt, 2, 0, idx, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(flags_stmt)
		}

		for (flag = rsf_next(r_ptr->spell_flags, FLAG_START);
			flag != FLAG_END;
			flag = rsf_next(r_ptr->spell_flags, flag + 1))
		{
			err = stats_db_bind_ints(spell_flags_stmt, 2, 0, 
				idx, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(spell_flags_stmt)
		}
	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(spell_flags_stmt)

	/* Handle monster bases */
	strnfmt(sql_buf, 256, "INSERT INTO monster_base_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO monster_base_spell_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&spell_flags_stmt, sql_buf);
	if (err) return err;

	for (rb_ptr = rb_info, idx = 0; rb_ptr; rb_ptr = rb_ptr->next, idx++)
	{
		for (flag = rf_next(rb_ptr->flags, FLAG_START);
			flag != FLAG_END;
			flag = rf_next(rb_ptr->flags, flag + 1))
		{
			err = sqlite3_bind_text(flags_stmt, 1, rb_ptr->name,
				strlen(rb_ptr->name), SQLITE_STATIC);
			if (err) return err;
			err = sqlite3_bind_int(flags_stmt, 2, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(flags_stmt)
		}

		for (flag = rsf_next(rb_ptr->spell_flags, FLAG_START);
			flag != FLAG_END;
			flag = rsf_next(rb_ptr->spell_flags, flag + 1))
		{
			err = sqlite3_bind_text(spell_flags_stmt, 1, 
				rb_ptr->name, strlen(rb_ptr->name), 
				SQLITE_STATIC);
			if (err) return err;
			err = sqlite3_bind_int(spell_flags_stmt, 2, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(spell_flags_stmt)
		}
		
	}

	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(spell_flags_stmt)

	return SQLITE_OK;
}

static int stats_dump_lists(void)
{
	int err, idx;
	char sql_buf[256];
	sqlite3_stmt *sql_stmt;

	/* Note: these lists are sometimes different from the ones the core
	 * game uses, insofar as we put the name of the flag in a
	 * description field. */
	info_entry effects[] =
	{
		#define EFFECT(x, y, r, z)    { EF_##x, y, r, #x },
		#include "list-effects.h"
		#undef EFFECT
	};

	char *r_info_flags[] =
	{
		#define RF(a, b) #a,
		#include "monster/list-mon-flags.h"
		#undef RF
		NULL
	};

	struct mon_spell mon_spell_table[] =
	{
		#define RSF(a, b, c, d, e, f, g, h, i, j, k, l, m) \
			{ RSF_##a, b, #a, d, e, f, g, h, i, j, k, l, m },
		#define RV(b, x, y, m) {b, x, y, m}
		#include "monster/list-mon-spells.h"
		#undef RV
		#undef RSF
	};

	struct object_flag object_flag_table[] =
	{
		#define OF(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s) \
			{ OF_##a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, #a },
		#include "object/list-object-flags.h"
		#undef OF
	};

	struct slay slay_table[] =
	{
		#define SLAY(a, b, c, d, e, f, g, h, i, j) \
			{ SL_##a, b, c, d, e, f, g, h, #a, j},
		#include "object/list-slays.h"
		#undef SLAY
	};

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO effects_list VALUES(?,?,?,?);");
	if (err) return err;

	for (idx = 1; idx < EF_MAX; idx++)
	{
		if (! effects[idx].desc) continue;

		err = stats_db_bind_ints(sql_stmt, 3, 0, idx, 
			effects[idx].aim, effects[idx].power);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 4, effects[idx].desc,
			strlen(effects[idx].desc), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO monster_flags_list VALUES(?,?);");
	if (err) return err;

	for (idx = 0; r_info_flags[idx] != NULL; idx++)
	{
		err = sqlite3_bind_int(sql_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 2, r_info_flags[idx],
			strlen(r_info_flags[idx]), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO monster_spell_flags_list VALUES(?,?,?,?);");
	if (err) return err;

	for (idx = 1; idx < RSF_MAX; idx++)
	{
		if (! mon_spell_table[idx].desc) continue;

		err = stats_db_bind_ints(sql_stmt, 3, 0, idx, 
			mon_spell_table[idx].cap, mon_spell_table[idx].div);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 4, mon_spell_table[idx].desc,
			strlen(mon_spell_table[idx].desc), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO object_flags_list VALUES(?,?,?,?,?,?);");
	if (err) return err;

	for (idx = 1; idx < OF_MAX; idx++)
	{
		struct object_flag *of_ptr = &object_flag_table[idx];
		if (! of_ptr->message) continue;

		err = stats_db_bind_ints(sql_stmt, 5, 0, idx, 
			of_ptr->pval, of_ptr->type, of_ptr->power,
			of_ptr->pval_mult);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 6, of_ptr->message,
			strlen(of_ptr->message), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO object_slays_list VALUES(?,?,?,?,?,?);");
	if (err) return err;

	for (idx = 1; idx < SL_MAX; idx++)
	{
		struct slay *s_ptr = &slay_table[idx];
		if (! s_ptr->desc) continue;

		err = stats_db_bind_ints(sql_stmt, 5, 0, idx, 
			s_ptr->object_flag, s_ptr->monster_flag, 
			s_ptr->resist_flag, s_ptr->mult);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 6, s_ptr->desc,
			strlen(s_ptr->desc), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	/* Hack, until we refactor origin kinds into a header */
	#define STATS_ORIGIN(idx,name) \
		strnfmt(sql_buf, 256, "INSERT INTO origin_flags_list VALUES(%d,'%s');", idx, #name); \
		err = stats_db_exec(sql_buf);\
		if (err) return err;

	STATS_ORIGIN(0,NONE)
	STATS_ORIGIN(1,FLOOR)
	STATS_ORIGIN(2,DROP)
	STATS_ORIGIN(3,CHEST)
	STATS_ORIGIN(4,DROP_SPECIAL)
	STATS_ORIGIN(5,DROP_PIT)
	STATS_ORIGIN(6,DROP_VAULT)
	STATS_ORIGIN(7,SPECIAL)
	STATS_ORIGIN(8,PIT)
	STATS_ORIGIN(9,VAULT)
	STATS_ORIGIN(10,LABYRINTH)
	STATS_ORIGIN(11,CAVERN)
	STATS_ORIGIN(12,RUBBLE)
	STATS_ORIGIN(13,MIXED)
	#undef STATS_ORIGIN

	return SQLITE_OK;
}

static int stats_dump_info(void)
{
	int err;
	char sql_buf[256];

	/* Wrap entire write into a transaction */
	err = stats_db_exec("BEGIN TRANSACTION;");
	if (err) return err;

	/* Metadata */
	strnfmt(sql_buf, 256, "INSERT INTO metadata VALUES('version','%s');",
		buildver);
	err = stats_db_exec(sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO metadata VALUES('randarts',%d);",
		randarts);
	err = stats_db_exec(sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO metadata VALUES('no_selling',%d);",
		no_selling);
	err = stats_db_exec(sql_buf);
	if (err) return err;

	err = stats_dump_artifacts();
	if (err) return err;

	err = stats_dump_egos();
	if (err) return err;

	err = stats_dump_monsters();
	if (err) return err;

	err = stats_dump_objects();
	if (err) return err;

	err = stats_dump_lists();
	if (err) return err;

	/* Commit transaction */
	return stats_db_exec("COMMIT;");
}

/**
 * Open the database connection and create the database tables.
 * All count tables will contain a level column (INTEGER) and a
 * count column (INTEGER). Some tables will include other INTEGER columns
 * for object origin, feeling, attributes, indices, or object flags.
 * Note that random_value types are stored as either A+BdC+Md or A.
 * Tables:
 *     metadata -- key-value pairs describing the stats run
 *     artifact_info -- dump of artifact.txt
 *     artifact_flags_map -- map between artifacts and object flags
 *     artifact_pval_flags_map -- map between artifacts and pval flags, with pvals
 *     ego_info -- dump of ego_item.txt
 *     ego_flags_map -- map between egos and object flags
 *     ego_pval_flags_map -- map between egos and pval flags, with pvals and minima
 *     ego_type_map -- map between egos and tvals/svals
 *     monster_base_flags_map -- map between monster bases and monster flags
 *     monster_base_spell_flags_map -- map between monster bases and monster spell flags
 *     monster_info -- dump of monsters.txt
 *     monster_flags_map -- map between monsters and monster flags
 *     monster_spell_flags_map -- map between monsters and monster spell flags
 *     object_base_info -- dump of object_base.txt
 *     object_base_flags_map -- map between object templates and object flags
 *     object_info -- dump of objects.txt
 *     object_flags_map -- map between artifacts and object flags
 *     object_pval_flags_map -- map between artifacts and pval flags, with pvals
 *     effects_list -- dump of list-effects.h
 *     monster_flags_list -- dump of list-mon-flags.h
 *     monster_spell_flags_list -- dump of list-mon-spells.h
 *     object_flags_list -- dump of list-object-flags.h
 *     object_slays_list -- dump of list-object-slays.h
 *     origin_flags_list -- dump of origin enum
 * Count tables:
 *     monsters
 *     obj_feelings
 *     mon_feelings
 *     gold
 *     artifacts
 *     consumables
 *     wearables_count
 *     wearables_dice
 *     wearables_ac
 *     wearables_hit
 *     wearables_dam
 *     wearables_egos
 *     wearables_flags
 *     wearables_pval_flags
 */
static bool stats_prep_db(void)
{
	bool status;
	int err;

	/* Open the database connection */
	status = stats_db_open();
	if (!status) return status;

	/* Create some tables */
	err = stats_db_exec("CREATE TABLE metadata(field TEXT UNIQUE NOT NULL, value TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE artifact_info(idx INT PRIMARY KEY, name TEXT, tval INT, sval INT, weight INT, cost INT, alloc_prob INT, alloc_min INT, alloc_max INT, ac INT, dd INT, ds INT, to_h INT, to_d INT, to_a INT, effect INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE artifact_flags_map(a_idx INT, o_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE artifact_pval_flags_map(a_idx INT, pval_flag INT, pval INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_info(idx INT PRIMARY KEY, name TEXT, to_h TEXT, to_d TEXT, to_a TEXT, cost INT, level INT, rarity INT, rating INT, num_pvals INT, min_to_h INT, min_to_d INT, min_to_a INT, xtra INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_flags_map(e_idx INT, o_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_pval_flags_map(e_idx INT, pval_flag INT, min_pval INT, pval TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_type_map(e_idx INT, tval INT, min_sval INT, max_sval INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_base_flags_map(rb_idx INT, r_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_base_spell_flags_map(rb_idx INT, rs_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_info(idx INT PRIMARY KEY, ac INT, sleep INT, speed INT, mexp INT, hp INT, freq_innate INT, freq_spell INT, level INT, rarity INT, name TEXT, base TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_flags_map(r_idx INT, r_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_spell_flags_map(r_idx INT, rs_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_base_info(idx INT PRIMARY KEY, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_base_flags_map(kb_idx INT, o_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_info(idx INT PRIMARY KEY, name TEXT, tval INT, sval INT, level INT, weight INT, cost INT, ac INT, dd INT, ds INT, alloc_prob INT, alloc_min INT, alloc_max INT, effect INT, gen_mult_prob INT, stack_size INT, to_h TEXT, to_d TEXT, to_a TEXT, charge TEXT, recharge_time TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_flags_map(k_idx INT, o_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_pval_flags_map(k_idx INT, pval_flag INT, pval TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE effects_list(idx INT PRIMARY KEY, aim INT, rating INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_flags_list(idx INT PRIMARY KEY, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_spell_flags_list(idx INT PRIMARY KEY, cap INT, div INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_flags_list(idx INT PRIMARY KEY, pval INT, type INT, power INT, pval_mult INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_slays_list(idx INT PRIMARY KEY, object_flag INT, monster_flag INT, resist_flag INT, mult INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE origin_flags_list(idx INT PRIMARY KEY, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monsters(level INT, count INT, k_idx INT, UNIQUE (level, k_idx) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE obj_feelings(level INT, count INT, feeling INT, UNIQUE (level, feeling) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE mon_feelings(level INT, count INT, feeling INT, UNIQUE (level, feeling) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE gold(level INT, count INT, origin INT, UNIQUE (level, origin) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE artifacts(level INT, count INT, a_idx INT, origin INT, UNIQUE (level, a_idx, origin) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE consumables(level INT, count INT, k_idx INT, origin INT, UNIQUE (level, k_idx, origin) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_count(level INT, count INT, k_idx INT, origin INT, UNIQUE (level, k_idx, origin) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_dice(level INT, count INT, k_idx INT, origin INT, dd INT, ds INT, UNIQUE (level, k_idx, origin, dd, ds) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_ac(level INT, count INT, k_idx INT, origin INT, ac INT, UNIQUE (level, k_idx, origin, ac) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_hit(level INT, count INT, k_idx INT, origin INT, to_h INT, UNIQUE (level, k_idx, origin, to_h) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_dam(level INT, count INT, k_idx INT, origin INT, to_d INT, UNIQUE (level, k_idx, origin, to_d) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_egos(level INT, count INT, k_idx INT, origin INT, e_idx INT, UNIQUE (level, k_idx, origin, e_idx) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_flags(level INT, count INT, k_idx INT, origin INT, of_idx INT, UNIQUE (level, k_idx, origin, of_idx) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE wearables_pval_flags(level INT, count INT, k_idx INT, origin INT, pval INT, of_idx INT, UNIQUE (level, k_idx, origin, of_idx) ON CONFLICT REPLACE);");
	if (err) return false;

	err = stats_dump_info();
	if (err) return false;

	return true;
}

/**
 * Find the offset of the given member of the level_data struct. Not elegant.
 */
static int stats_level_data_offsetof(const char *member)
{
	if (streq(member, "monsters"))
		return offsetof(struct level_data, monsters);
	else if (streq(member, "obj_feelings"))
		return offsetof(struct level_data, obj_feelings);
	else if (streq(member, "mon_feelings"))
		return offsetof(struct level_data, mon_feelings);
	else if (streq(member, "gold"))
		return offsetof(struct level_data, gold);
	else if (streq(member, "artifacts"))
		return offsetof(struct level_data, artifacts);
	else if (streq(member, "consumables"))
		return offsetof(struct level_data, consumables);
	else if (streq(member, "wearables"))
		return offsetof(struct level_data, wearables);
		
	/* We should not get to this point. */
	assert(0);
}

/**
 * Find the offset of the given member of the wearables_data struct. Not 
 * elegant.
 */
static int stats_wearables_data_offsetof(const char *member)
{
	if (streq(member, "count"))
		return offsetof(struct wearables_data, count);
	else if (streq(member, "dice"))
		return offsetof(struct wearables_data, dice);
	else if (streq(member, "ac"))
		return offsetof(struct wearables_data, ac);
	else if (streq(member, "hit"))
		return offsetof(struct wearables_data, hit);
	else if (streq(member, "dam"))
		return offsetof(struct wearables_data, dam);
	else if (streq(member, "egos"))
		return offsetof(struct wearables_data, egos);
	else if (streq(member, "flags"))
		return offsetof(struct wearables_data, flags);
	else if (streq(member, "pval_flags"))
		return offsetof(struct wearables_data, pval_flags);
		
	/* We should not get to this point. */
	assert(0);
}

/**
 * Given a pointer to a dynamically allocated array and a value, look up
 * the index with that value; e.g. given wearables_index[k_idx], return k_idx.
 * If not found, return -1 * value.
 */

static int stats_lookup_index(const int *index, int max_idx, int value)
{
	int idx;

	for (idx = 0; idx < max_idx; idx ++) {
		if (index[idx] == value) return idx;
	}

	return -1 * value;
}

static int stats_write_db_level_data(const char *table, int max_idx)
{
	char sql_buf[256];
	sqlite3_stmt *sql_stmt;
	int err, level, i, offset;

	strnfmt(sql_buf, 256, "INSERT INTO %s VALUES(?,?,?);", table);
	err = stats_db_stmt_prep(&sql_stmt, sql_buf);
	if (err) return err;

	offset = stats_level_data_offsetof(table);

	for (level = 1; level < LEVEL_MAX; level++)
	{
		for (i = 0; i < max_idx; i++)
		{
			/* This arcane expression finds the value of 
			 * level_data[level].<table>[i] */
			u32b count;
			if (streq(table, "gold"))
			{
				count = *((long long *)((byte *)&level_data[level] + offset) + i);
			}
			else
			{
				count = *((u32b *)((byte *)&level_data[level] + offset) + i);
			}
			if (!count) continue;

			err = stats_db_bind_ints(sql_stmt, 3, 0,
				level, count, i);
			if (err) return err;

			STATS_DB_STEP_RESET(sql_stmt)
		}
	}

	return sqlite3_finalize(sql_stmt);
}

static int stats_write_db_level_data_items(const char *table, int max_idx, 
	bool translate_consumables)
{
	char sql_buf[256];
	sqlite3_stmt *sql_stmt;
	int err, level, origin, i, offset;

	strnfmt(sql_buf, 256, "INSERT INTO %s VALUES(?,?,?,?);", table);
	err = stats_db_stmt_prep(&sql_stmt, sql_buf);
	if (err) return err;

	offset = stats_level_data_offsetof(table);

	for (level = 1; level < LEVEL_MAX; level++)
	{
		for (origin = 0; origin < ORIGIN_STATS; origin++)
		{
			for (i = 0; i < max_idx; i++)
			{
				/* This arcane expression finds the value of 
				 * level_data[level].<table>[origin][i] */
				u32b count = ((u32b **)((byte *)&level_data[level] + offset))[origin][i];
				if (!count) continue;

				err = stats_db_bind_ints(sql_stmt, 4, 0,
					level, count, 
					translate_consumables ? stats_lookup_index(consumables_index, z_info->k_max, i) : i, origin);
				if (err) return err;

				STATS_DB_STEP_RESET(sql_stmt)
			}
		}
	}

	return sqlite3_finalize(sql_stmt);
}

static int stats_write_db_wearables_count(void)
{
	sqlite3_stmt *sql_stmt;
	int err, level, origin, k_idx, idx;

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO wearables_count VALUES(?,?,?,?);");
	if (err) return err;

	for (level = 1; level < LEVEL_MAX; level++)
	{
		for (origin = 0; origin < ORIGIN_STATS; origin++)
		{
			for (idx = 0; idx < wearable_count + 1; idx++)
			{
				u32b count = level_data[level].wearables[origin][idx].count;
				/* Skip if object did not appear */
				if (!count) continue;

				k_idx = stats_lookup_index(wearables_index, 
					z_info->k_max, idx);

				/* Skip if pile */
				if (! k_idx) continue;

				err = stats_db_bind_ints(sql_stmt, 4, 0,
					level, count, k_idx, origin);
				if (err) return err;

				STATS_DB_STEP_RESET(sql_stmt)
			}
		}
	}

	return sqlite3_finalize(sql_stmt);
}

/**
 * Unfortunately, the arcane expression used to find the value of an array 
 * member of a struct differs depending on whether the member is declared
 * as an array or as a pointer. Pass in true if the member is an array, and
 * false if the member is a pointer.
 */
static int stats_write_db_wearables_array(const char *field, int max_val, bool array_p)
{
	char sql_buf[256];
	sqlite3_stmt *sql_stmt;
	int err, level, origin, idx, k_idx, i, offset;

	strnfmt(sql_buf, 256, "INSERT INTO wearables_%s VALUES(?,?,?,?,?);", field);
	err = stats_db_stmt_prep(&sql_stmt, sql_buf);
	if (err) return err;

	offset = stats_wearables_data_offsetof(field);

	for (level = 1; level < LEVEL_MAX; level++)
	{
		for (origin = 0; origin < ORIGIN_STATS; origin++)
		{
			for (idx = 0; idx < wearable_count + 1; idx++)
			{
				k_idx = stats_lookup_index(wearables_index, 
					z_info->k_max, idx);

				/* Skip if pile */
				if (! k_idx) continue;

				for (i = 0; i < max_val; i++)
				{
					/* This arcane expression finds the value of
					 * level_data[level].wearables[origin][idx].<field>[i] */
					u32b count;
					if (array_p)
					{
						count = ((u32b *)((byte *)&level_data[level].wearables[origin][idx] + offset))[i];
					}
					else
					{
						count = ((u32b *)*((u32b **)((byte *)&level_data[level].wearables[origin][idx] + offset)))[i];
					}
					if (!count) continue;

					err = stats_db_bind_ints(sql_stmt, 5, 0,
						level, count, k_idx, origin, i);
					if (err) return err;

					STATS_DB_STEP_RESET(sql_stmt)
				}
			}
		}
	}

	return sqlite3_finalize(sql_stmt);
}

/**
 * Unfortunately, the arcane expression used to find the value of an array 
 * member of a struct differs depending on whether the member is declared
 * as an array or as a pointer. Pass in true if the member is an array, and
 * false if the member is a pointer.
 */
static int stats_write_db_wearables_2d_array(const char *field, 
	int max_val1, int max_val2, bool array_p, bool translate_pval_flags)
{
	char sql_buf[256];
	sqlite3_stmt *sql_stmt;
	int err, level, origin, idx, k_idx, i, j, offset;

	strnfmt(sql_buf, 256, "INSERT INTO wearables_%s VALUES(?,?,?,?,?,?);", field);
	err = stats_db_stmt_prep(&sql_stmt, sql_buf);
	if (err) return err;

	offset = stats_wearables_data_offsetof(field);

	for (level = 1; level < LEVEL_MAX; level++)
	{
		for (origin = 0; origin < ORIGIN_STATS; origin++)
		{
			for (idx = 0; idx < wearable_count + 1; idx++)
			{
				k_idx = stats_lookup_index(wearables_index, 
					z_info->k_max, idx);

				/* Skip if pile */
				if (! k_idx) continue;

				for (i = 0; i < max_val1; i++)
				{
					for (j = 0; j < max_val2; j++)
					{
						/* This arcane expression finds the value of
				 		* level_data[level].wearables[origin][idx].<field>[i][j] */
						u32b count;
						int real_j = translate_pval_flags ? stats_lookup_index(pval_flags_index, OF_MAX, j) : j; 

						if (i == 0 && real_j == 0) continue;

						if (array_p)
						{
							count = ((u32b *)((byte *)&level_data[level].wearables[origin][idx] + offset))[i * max_val2 + j];
						}
						else
						{
							count = *(*((u32b **)((byte *)&level_data[level].wearables[origin][idx] + offset) + i) + j);
						}
						if (!count) continue;

						err = stats_db_bind_ints(sql_stmt, 6, 0,
							level, count, k_idx, origin, 
							i, real_j);
						if (err) return err;

						STATS_DB_STEP_RESET(sql_stmt)
					}
				}
			}
		}
	}

	return sqlite3_finalize(sql_stmt);
}

static int stats_write_db(u32b run)
{
	char sql_buf[256];
	int err;

	/* Wrap entire write into a transaction */
	err = stats_db_exec("BEGIN TRANSACTION;");
	if (err) return err;

	strnfmt(sql_buf, 256, 
		"INSERT OR REPLACE INTO metadata VALUES('runs', %d);", run);
	err = stats_db_exec(sql_buf);
	if (err) return err;

	err = stats_write_db_level_data("monsters", z_info->r_max);
	if (err) return err;

	err = stats_write_db_level_data("obj_feelings", OBJ_FEEL_MAX);
	if (err) return err;

	err = stats_write_db_level_data("mon_feelings", MON_FEEL_MAX);
	if (err) return err;

	err = stats_write_db_level_data("gold", ORIGIN_STATS);
	if (err) return err;

	err = stats_write_db_level_data_items("artifacts", z_info->a_max, 
		false);
	if (err) return err;

	err = stats_write_db_level_data_items("consumables", 
		consumable_count + 1, true);
	if (err) return err;

	err = stats_write_db_wearables_count();
	if (err) return err;

	err = stats_write_db_wearables_2d_array("dice", TOP_DICE, TOP_SIDES, true, false);
	if (err) return err;

	err = stats_write_db_wearables_array("ac", TOP_AC, true);
	if (err) return err;

	err = stats_write_db_wearables_array("hit", TOP_PLUS, true);
	if (err) return err;

	err = stats_write_db_wearables_array("dam", TOP_PLUS, true);
	if (err) return err;

	err = stats_write_db_wearables_array("egos", z_info->e_max, false);
	if (err) return err;

	err = stats_write_db_wearables_array("flags", OF_MAX, true);
	if (err) return err;

	err = stats_write_db_wearables_2d_array("pval_flags", TOP_PVAL, pval_flags_count + 1, false, true);
	if (err) return err;

	/* Commit transaction */
	err = stats_db_exec("COMMIT;");
	if (err) return err;

	return SQLITE_OK;
}

/**
 * Call with the number of runs that have been completed.
 */

void progress_bar(u32b run, time_t start) {
	u32b i;
	u32b n = (run * 40) / num_runs;
	u32b p10 = ((long long)run * 1000) / num_runs;

	time_t delta = time(NULL) - start;
	u32b togo = num_runs - run;
	u32b expect = delta ? ((long long)run * (long long)togo) / delta 
		: 0;

	int h = expect / 3600;
	int m = (expect % 3600) / 60;
	int s = expect % 60;

	printf("\r|");
	for (i = 0; i < n; i++) printf("*");
	for (i = 0; i < 40 - n; i++) printf(" ");
	printf("| %d/%d (%5.1f%%) %3d:%02d:%02d ", run, num_runs, p10/10.0, h, m, s);
	fflush(stdout);
}


static errr run_stats(void)
{
	u32b run;
	artifact_type *a_info_save;
	unsigned int i;
	int err;
	bool status; 

	time_t start;

	prep_output_dir();
	create_indices();
	alloc_memory();
	if (randarts)
	{
		a_info_save = mem_zalloc(z_info->a_max * sizeof(artifact_type));
		for (i = 0; i < z_info->a_max; i++)
		{
			if (!a_info[i].name) continue;

			memcpy(&a_info_save[i], &a_info[i], sizeof(artifact_type));
		}
	}

	if (save) {
		if (!quiet) printf("Creating the database and dumping info...\n");
		status = stats_prep_db();
		if (!status) quit("Couldn't prepare database!");
	}

	if (!quiet) {
		printf("Beginning %d runs...\n", num_runs);
		fflush(stdout);
	}

	start = time(NULL);
	for (run = 1; run <= num_runs; run++)
	{
		if (!quiet) progress_bar(run - 1, start);

		if (randarts)
		{
			for (i = 0; i < z_info->a_max; i++)
			{
				memcpy(&a_info[i], &a_info_save[i], sizeof(artifact_type));
			}
		}

		initialize_character();
		unkill_uniques();
		reset_artifacts();
		descend_dungeon();

		/* Checkpoint every so many runs */
		if (save && run % RUNS_PER_CHECKPOINT == 0)
		{
			err = stats_write_db(run);
			if (err)
			{
				stats_db_close();
				quit_fmt("Problems writing to database!  sqlite3 errno %d.", err);
			}
		}
		dispose_character();
	}

	if (!quiet) {
		progress_bar(num_runs, start);
		printf("\nSaving the data...\n");
		fflush(stdout);
	}

	if (save) {
		err = stats_write_db(run);
		stats_db_close();
		if (err) quit_fmt("Problems writing to database!  sqlite3 errno %d.", err);
	}
	free_stats_memory();
	mem_free(ANGBAND_DIR_STATS);
	cleanup_angband();
	if (!quiet) printf("Done!\n");
	quit(NULL);
	exit(0);
}

typedef struct term_data term_data;
struct term_data {
	term t;
};

static term_data td;
typedef struct {
	int key;
	errr (*func)(int v);
} term_xtra_func;

static void term_init_stats(term *t) {
	return;
}

static void term_nuke_stats(term *t) {
	return;
}

static errr term_xtra_clear(int v) {
	return 0;
}

static errr term_xtra_noise(int v) {
	return 0;
}

static errr term_xtra_fresh(int v) {
	return 0;
}

static errr term_xtra_shape(int v) {
	return 0;
}

static errr term_xtra_alive(int v) {
	return 0;
}

static errr term_xtra_event(int v) {
	if (nextkey) {
		Term_keypress(nextkey, 0);
		nextkey = 0;
	}
	if (running_stats) {
		return 0;
	}
	running_stats = 1;
	return run_stats();
}

static errr term_xtra_flush(int v) {
	return 0;
}

static errr term_xtra_delay(int v) {
	return 0;
}

static errr term_xtra_react(int v) {
	return 0;
}

static term_xtra_func xtras[] = {
	{ TERM_XTRA_CLEAR, term_xtra_clear },
	{ TERM_XTRA_NOISE, term_xtra_noise },
	{ TERM_XTRA_FRESH, term_xtra_fresh },
	{ TERM_XTRA_SHAPE, term_xtra_shape },
	{ TERM_XTRA_ALIVE, term_xtra_alive },
	{ TERM_XTRA_EVENT, term_xtra_event },
	{ TERM_XTRA_FLUSH, term_xtra_flush },
	{ TERM_XTRA_DELAY, term_xtra_delay },
	{ TERM_XTRA_REACT, term_xtra_react },
	{ 0, NULL },
};

static errr term_xtra_stats(int n, int v) {
	int i;
	for (i = 0; xtras[i].func; i++) {
		if (xtras[i].key == n) {
			return xtras[i].func(v);
		}
	}
	return 0;
}

static errr term_curs_stats(int x, int y) {
	return 0;
}

static errr term_wipe_stats(int x, int y, int n) {
	return 0;
}

static errr term_text_stats(int x, int y, int n, byte a, const char *s) {
	return 0;
}

static void term_data_link(int i) {
	term *t = &td.t;

	term_init(t, 80, 24, 256);

	/* Ignore some actions for efficiency and safety */
	t->never_bored = TRUE;
	t->never_frosh = TRUE;

	t->init_hook = term_init_stats;
	t->nuke_hook = term_nuke_stats;

	t->xtra_hook = term_xtra_stats;
	t->curs_hook = term_curs_stats;
	t->wipe_hook = term_wipe_stats;
	t->text_hook = term_text_stats;

	t->data = &td;

	Term_activate(t);

	angband_term[i] = t;
}

const char help_stats[] = "Stats mode, subopts -q(uiet) -r(andarts) -n(# of runs) -s(no selling) -x(dont save data)";

/*
 * Usage:
 *
 * angband -mstats -- [-q] [-r] [-nNNNN] [-s] [-x]
 *
 *
 *   -q      Quiet mode (turn off progress messages)
 *   -r      Turn on randarts
 *   -nNNNN  Make NNNN runs through the dungeon (default: 1)
 *   -s      Turn on no-selling
 *   -x      Dont save (testing only!)
 */

errr init_stats(int argc, char *argv[]) {
	int i;

	/* Skip over argv[0] */
	for (i = 1; i < argc; i++) {
		if (streq(argv[i], "-r")) {
			randarts = 1;
			continue;
		}
		if (streq(argv[i], "-q")) {
			quiet = TRUE;
			continue;
		}
		if (prefix(argv[i], "-n")) {
			num_runs = atoi(&argv[i][2]);
			continue;
		}
		if (prefix(argv[i], "-s")) {
			no_selling = 1;
			continue;
		}
		if (prefix(argv[i], "-x")) {
			save = 0;
			continue;
		}
		printf("init-stats: bad argument '%s'\n", argv[i]);
	}

	term_data_link(0);
	return 0;
}

#endif /* USE_STATS */
