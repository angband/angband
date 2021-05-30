/**
 * \file main-stats.c
 * \brief Pseudo-UI for stats generation (borrows heavily from main-test.c)
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

#include "buildid.h"
#include "game-world.h"
#include "init.h"
#include "main.h"
#include "mon-make.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-power.h"
#include "obj-randart.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player.h"
#include "player-birth.h"
#include "player-util.h"
#include "project.h"
#include "stats/db.h"
#include "stats/structs.h"
#include "store.h"
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
#define TOP_MOD 		 25
#define RUNS_PER_CHECKPOINT	10000

/* For ref, e_max is 128, a_max is 136, r_max is ~650,
	ORIGIN_STATS is 14, OF_MAX is ~120 */

/**
 * There are (at last count) 416 kinds, of which about 150-200 are wearable
 * - here wearable includes ammo, which behaves similarly in having
 * variable power between individual items
 */

static int randarts = 0;
static int no_selling = 0;
static u32b num_runs = 1;
static bool quiet = false;
static int nextkey = 0;
static int running_stats = 0;
static char *ANGBAND_DIR_STATS;

static int *consumables_index;
static int *wearables_index;
static int wearable_count = 0;
static int consumable_count = 0;

struct wearables_data {
	u32b count;
	u32b dice[TOP_DICE][TOP_SIDES];
	u32b ac[TOP_AC];
	u32b hit[TOP_PLUS];
	u32b dam[TOP_PLUS];
/*	u32b power[TOP_POWER]; not enough memory - add it later in bands */
	u32b *egos;
	u32b flags[OF_MAX];
	u32b *modifiers[TOP_MOD];
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

static void create_indices(void)
{
	int i;

	consumables_index = mem_zalloc(z_info->k_max * sizeof(int));
	wearables_index = mem_zalloc(z_info->k_max * sizeof(int));

	for (i = 0; i < z_info->k_max; i++) {

		struct object object_type_body = { 0 };
		struct object *obj = &object_type_body;
		struct object_kind *kind = &k_info[i];
		obj->tval = kind->tval;

		if (!kind->name) continue;

		if (tval_has_variable_power(obj))
			wearables_index[i] = ++wearable_count;
		else
			consumables_index[i] = ++consumable_count;
	}
}

static void alloc_memory(void)
{
	int i, j, k, l;

	for (i = 0; i < LEVEL_MAX; i++) {
		level_data[i].monsters = mem_zalloc(z_info->r_max * sizeof(u32b));
/*		level_data[i].vaults = mem_zalloc(z_info->v_max * sizeof(u32b));
		level_data[i].pits = mem_zalloc(z_info->pit_max * sizeof(u32b)); */

		for (j = 0; j < ORIGIN_STATS; j++) {
			level_data[i].artifacts[j] = mem_zalloc(z_info->a_max *
													sizeof(u32b));
			level_data[i].consumables[j] = mem_zalloc((consumable_count + 1) *
													  sizeof(u32b));
			level_data[i].wearables[j]
				= mem_zalloc((wearable_count + 1) *
							 sizeof(struct wearables_data));

			for (k = 0; k < wearable_count + 1; k++) {
				level_data[i].wearables[j][k].egos
					= mem_zalloc(z_info->e_max * sizeof(u32b));
				for (l = 0; l < TOP_MOD; l++)
					level_data[i].wearables[j][k].modifiers[l]
						= mem_zalloc((OBJ_MOD_MAX + 1) * sizeof(u32b));
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
				for (l = 0; l < TOP_MOD; l++) {
					mem_free(level_data[i].wearables[j][k].modifiers[l]);
				}
				mem_free(level_data[i].wearables[j][k].egos);
			}
			mem_free(level_data[i].wearables[j]);
		}
	}
	mem_free(consumables_index);
	mem_free(wearables_index);
	string_free(ANGBAND_DIR_STATS);
}

/* Copied from birth.c:generate_player() */
static void generate_player_for_stats(void)
{
	OPT(player, birth_randarts) = randarts;
	OPT(player, birth_no_selling) = no_selling;
	OPT(player, birth_stacking) = true;
	OPT(player, auto_more) = true;

	player->wizard = 1; /* Set wizard mode on */

	player->race = races;  /* Human   */
	player->class = classes; /* Warrior */

	/* Level 1 */
	player->max_lev = player->lev = 1;

	/* Experience factor */
	player->expfact = player->race->r_exp + player->class->c_exp;

	/* Hitdice */
	player->hitdie = player->race->r_mhp + player->class->c_mhp;

	/* Initial hitpoints -- high just to be safe */
	player->mhp = player->chp = 2000;

	/* Pre-calculate level 1 hitdice */
	player->player_hp[0] = player->hitdie;

	/* Set age/height/weight */
	player->ht = player->ht_birth = 66;
	player->wt = player->wt_birth = 150;
	player->age = 14;

	/* Set social class and (null) history */
	player->history = get_history(player->race->history);
}

static void initialize_character(void)
{
	u32b seed;

	if (!quiet) {
		printf(" [I  ]\b\b\b\b\b\b");
		fflush(stdout);
	}

	seed = (time(NULL));
	Rand_quick = false;
	Rand_state_init(seed);

	player_init(player);
	generate_player_for_stats();

	seed_flavor = randint0(0x10000000);
	seed_randart = randint0(0x10000000);

	if (randarts) {
		do_randart(seed_randart, false);
	}

	store_reset();
	flavor_init();
	player->upkeep->playing = true;
	player->upkeep->autosave = false;
	prepare_next_level(&cave, player);
}

static void kill_all_monsters(int level)
{
	int i;

	for (i = cave_monster_max(cave) - 1; i >= 1; i--) {
		struct monster *mon = cave_monster(cave, i);

		level_data[level].monsters[mon->race->ridx]++;

		monster_death(mon, true);

		if (rf_has(mon->race->flags, RF_UNIQUE))
			mon->race->max_num = 0;
	}
}

static void unkill_uniques(void)
{
	int i;

	if (!quiet) {
		printf(" [U  ]\b\b\b\b\b\b");
		fflush(stdout);
	}

	for (i = 0; i < z_info->r_max; i++) {
		struct monster_race *race = &r_info[i];

		if (rf_has(race->flags, RF_UNIQUE))
			race->max_num = 1;
	}
}

static void reset_artifacts(void)
{
	int i;

	if (!quiet) {
		printf(" [R  ]\b\b\b\b\b\b");
		fflush(stdout);
	}

	for (i = 0; i < z_info->a_max; i++)
		a_info[i].created = false;

}

static void log_all_objects(int level)
{
	int x, y, i;

	for (y = 1; y < cave->height - 1; y++) {
		for (x = 1; x < cave->width - 1; x++) {
			struct loc grid = loc(x, y);
			struct object *obj;

			for (obj = square_object(cave, grid); obj; obj = obj->next) {
				/*	u32b o_power = 0; */

/*				o_power = object_power(obj, false, NULL, true); */

				/* Capture gold amounts */
				if (tval_is_money(obj))
					level_data[level].gold[obj->origin] += obj->pval;

				/* Capture artifact drops */
				if (obj->artifact)
					level_data[level].artifacts[obj->origin][obj->artifact->aidx]++;

				/* Capture kind details */
				if (tval_has_variable_power(obj)) {
					struct wearables_data *w
						= &level_data[level].wearables[obj->origin][wearables_index[obj->kind->kidx]];

					w->count++;
					w->dice[MIN(obj->dd, TOP_DICE - 1)][MIN(obj->ds, TOP_SIDES - 1)]++;
					w->ac[MIN(MAX(obj->ac + obj->to_a, 0), TOP_AC - 1)]++;
					w->hit[MIN(MAX(obj->to_h, 0), TOP_PLUS - 1)]++;
					w->dam[MIN(MAX(obj->to_d, 0), TOP_PLUS - 1)]++;

					/* Capture egos */
					if (obj->ego)
						w->egos[obj->ego->eidx]++;
					/* Capture object flags */
					for (i = of_next(obj->flags, FLAG_START); i != FLAG_END;
							i = of_next(obj->flags, i + 1))
						w->flags[i]++;
					/* Capture object modifiers */
					for (i = 0; i < OBJ_MOD_MAX; i++) {
						int p = obj->modifiers[i];
						w->modifiers[MIN(MAX(p, 0), TOP_MOD - 1)][i]++;
					}
				} else
					level_data[level].consumables[obj->origin][consumables_index[obj->kind->kidx]]++;
			}
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
				printf(" [%3d]\b\b\b\b\b\b", level);
				fflush(stdout);
				last = now;
			}
		}

		dungeon_change_level(player, level);
		prepare_next_level(&cave, player);

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
		"%s%sstats", ANGBAND_DIR_USER, PATH_SEP);

	if (dir_create(ANGBAND_DIR_STATS))
		return;
	else
		quit("Couldn't create stats directory!");
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
		flag = of_next(flags, flag + 1)) {
		err = sqlite3_bind_int(flags_stmt, 2, flag);
		if (err) return err;
		STATS_DB_STEP_RESET(flags_stmt)
	}


	return SQLITE_OK;
}

static int stats_dump_artifacts(void)
{
	int err, idx, i;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *mods_stmt;

	strnfmt(sql_buf, 256, "INSERT INTO artifact_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO artifact_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO artifact_mods_map VALUES (?,?,?);");
	err = stats_db_stmt_prep(&mods_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->a_max; idx++) {
		struct artifact *art = &a_info[idx];

		if (!art->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, art->name, 
			strlen(art->name), SQLITE_STATIC);
		if (err) return err;
		err = stats_db_bind_ints(info_stmt, 13, 2, 
			art->tval, art->sval, art->weight,
			art->cost, art->alloc_prob, art->alloc_min,
			art->alloc_max, art->ac, art->dd,
			art->ds, art->to_h, art->to_d, art->to_a);
		STATS_DB_STEP_RESET(info_stmt)

		err = stats_dump_oflags(flags_stmt, idx, art->flags);
		if (err) return err;

		for (i = 0; i < OBJ_MOD_MAX; i++)
		{
			err = stats_db_bind_ints(mods_stmt, 3, 0, idx, i, 
									 art->modifiers[i]);
				if (err) return err;
				STATS_DB_STEP_RESET(mods_stmt)
		}
	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(mods_stmt)

	return SQLITE_OK;
}

static int stats_dump_egos(void)
{
	int err, idx, i;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *mods_stmt; //*type_stmt;

	strnfmt(sql_buf, 256, "INSERT INTO ego_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO ego_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO ego_mods_map VALUES (?,?,?);");
	err = stats_db_stmt_prep(&mods_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->e_max; idx++) {
		struct ego_item *ego = &e_info[idx];

		if (!ego->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, ego->name, 
			strlen(ego->name), SQLITE_STATIC);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 3, ego->to_h); 
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 4, ego->to_d); 
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 5, ego->to_a); 
		if (err) return err;
		err = stats_db_bind_ints(info_stmt, 8, 5, 
			ego->cost, ego->alloc_min, ego->alloc_max,
			ego->alloc_prob, ego->rating, ego->min_to_h, 
			ego->min_to_d, ego->min_to_a);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		err = stats_dump_oflags(flags_stmt, idx, ego->flags);
		if (err) return err;

		for (i = 0; i < OBJ_MOD_MAX; i++) {
			err = stats_db_bind_ints(mods_stmt, 3, 0, idx, i, 
									 ego->min_modifiers[i]);
				if (err) return err;
				STATS_DB_STEP_RESET(mods_stmt)
		}
	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(mods_stmt)

	return SQLITE_OK;
}

static int stats_dump_objects(void)
{
	int err, idx, i, flag;
	char sql_buf[256];
	sqlite3_stmt *info_stmt, *flags_stmt, *mods_stmt;

	strnfmt(sql_buf, 256, "INSERT INTO object_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO object_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO object_mods_map VALUES (?,?,?);");
	err = stats_db_stmt_prep(&mods_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->k_max; idx++) {
		struct object_kind *kind = &k_info[idx];

		if (!kind->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, kind->name,
			strlen(kind->name), SQLITE_STATIC);
		if (err) return err;
		err = stats_db_bind_ints(info_stmt, 13, 2,
			kind->tval, kind->sval, kind->level, kind->weight,
			kind->cost, kind->ac, kind->dd, kind->ds,
			kind->alloc_prob, kind->alloc_min,
			kind->alloc_max, kind->gen_mult_prob, kind->stack_size);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 17, kind->to_h);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 18, kind->to_d);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 19, kind->to_a);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 20, kind->charge);
		if (err) return err;
		err = stats_db_bind_rv(info_stmt, 21, kind->time);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		err = stats_dump_oflags(flags_stmt, idx, kind->flags);
		if (err) return err;

		for (i = 0; i < OBJ_MOD_MAX; i++) {
			err = stats_db_bind_ints(mods_stmt, 2, 0, idx, i);
				if (err) return err;
				err = stats_db_bind_rv(mods_stmt, 3, kind->modifiers[i]);
				if (err) return err;
				STATS_DB_STEP_RESET(mods_stmt)
		}
	}

	STATS_DB_FINALIZE(info_stmt)
	STATS_DB_FINALIZE(flags_stmt)
	STATS_DB_FINALIZE(mods_stmt)

	/* Handle object_base */
	strnfmt(sql_buf, 256, "INSERT INTO object_base_info VALUES (?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO object_base_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	idx = 0;
	for (idx = 0; idx < TV_MAX; idx++) {
		struct object_base *base = &kb_info[idx];

		if (!base->name) continue;

		err = sqlite3_bind_int(info_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 2, base->name,
			strlen(base->name), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		for (flag = of_next(base->flags, FLAG_START);
			flag != FLAG_END;
			flag = of_next(base->flags, flag + 1)) {
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
	struct monster_base *base;

	strnfmt(sql_buf, 256, "INSERT INTO monster_info VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
	err = stats_db_stmt_prep(&info_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO monster_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&flags_stmt, sql_buf);
	if (err) return err;

	strnfmt(sql_buf, 256, "INSERT INTO monster_spell_flags_map VALUES (?,?);");
	err = stats_db_stmt_prep(&spell_flags_stmt, sql_buf);
	if (err) return err;

	for (idx = 0; idx < z_info->r_max; idx++) {
		struct monster_race *race = &r_info[idx];

		/* Skip empty entries */
		if (!race->name) continue;

		err = stats_db_bind_ints(info_stmt, 10, 0, idx,
			race->ac, race->sleep, race->speed, race->mexp,
			race->avg_hp, race->freq_innate, race->freq_spell,
			race->level, race->rarity);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 11, race->name,
			strlen(race->name), SQLITE_STATIC);
		if (err) return err;
		err = sqlite3_bind_text(info_stmt, 12, race->base->name,
			strlen(race->base->name), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(info_stmt)

		for (flag = rf_next(race->flags, FLAG_START);
			flag != FLAG_END;
			flag = rf_next(race->flags, flag + 1))
		{
			err = stats_db_bind_ints(flags_stmt, 2, 0, idx, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(flags_stmt)
		}

		for (flag = rsf_next(race->spell_flags, FLAG_START);
			flag != FLAG_END;
			flag = rsf_next(race->spell_flags, flag + 1))
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

	for (base = rb_info, idx = 0; base; base = base->next, idx++) {
		for (flag = rf_next(base->flags, FLAG_START);
			flag != FLAG_END;
			flag = rf_next(base->flags, flag + 1)) {
			err = sqlite3_bind_text(flags_stmt, 1, base->name,
				strlen(base->name), SQLITE_STATIC);
			if (err) return err;
			err = sqlite3_bind_int(flags_stmt, 2, flag);
			if (err) return err;
			STATS_DB_STEP_RESET(flags_stmt)
		}

		for (flag = rsf_next(base->spell_flags, FLAG_START);
			flag != FLAG_END;
			flag = rsf_next(base->spell_flags, flag + 1)) {
			err = sqlite3_bind_text(spell_flags_stmt, 1, 
				base->name, strlen(base->name), 
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
		{ EF_NONE, false, NULL },
		#define F(x) effect_handler_##x
		#define EFFECT(x, a, b, c, d, e, f)    { EF_##x, a, #x },
		#include "list-effects.h"
		#undef EFFECT
		#undef F
		{ EF_MAX, false, NULL }
	};

	const char *r_info_flags[] =
	{
		#define RF(a, b, c) #a,
		#include "list-mon-race-flags.h"
		#undef RF
		NULL
	};

	/** Really want elements (at least) here - NRM **/

	const char *object_flag_names[] =
	{
		"NONE",
		#define OF(a, b) #a,
		#include "list-object-flags.h"
		#undef OF
	};

	const char *object_mods[] =
	{
		#define STAT(a) #a,
        #include "list-stats.h"
        #undef STAT
        #define OBJ_MOD(a) #a,
        #include "list-object-modifiers.h"
        #undef OBJ_MOD
		NULL
	};

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO effects_list VALUES(?,?,?,?);");
	if (err) return err;

	for (idx = 1; idx < EF_MAX; idx++) {
		if (! effects[idx].desc) continue;

		err = stats_db_bind_ints(sql_stmt, 2, 0, idx, 
			effects[idx].aim);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 2, effects[idx].desc,
			strlen(effects[idx].desc), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO monster_flags_list VALUES(?,?);");
	if (err) return err;

	for (idx = 0; r_info_flags[idx] != NULL; idx++) {
		err = sqlite3_bind_int(sql_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 2, r_info_flags[idx],
			strlen(r_info_flags[idx]), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO object_flags_list VALUES(?,?);");
	if (err) return err;

	for (idx = 0; idx < OF_MAX; idx++) {
		err = stats_db_bind_ints(sql_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 2, object_flag_names[idx],
			strlen(object_flag_names[idx]), SQLITE_STATIC);
		if (err) return err;
		STATS_DB_STEP_RESET(sql_stmt)
	}

	STATS_DB_FINALIZE(sql_stmt)

	err = stats_db_stmt_prep(&sql_stmt, 
		"INSERT INTO object_mods_list VALUES(?,?);");
	if (err) return err;

	for (idx = 0; object_mods[idx] != NULL; idx++) {
		err = stats_db_bind_ints(sql_stmt, 1, idx);
		if (err) return err;
		err = sqlite3_bind_text(sql_stmt, 2, object_mods[idx],
			strlen(object_mods[idx]), SQLITE_STATIC);
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
 *     artifact_mods_map -- map between artifacts and modifiers
 *     ego_info -- dump of ego_item.txt
 *     ego_flags_map -- map between egos and object flags
 *     ego_mods_map -- map between egos and modifiers, with minima
 *     ego_type_map -- map between egos and tvals/svals
 *     monster_base_flags_map -- map between monster bases and monster flags
 *     monster_base_spell_flags_map -- map between monster bases and monster spell flags
 *     monster_info -- dump of monsters.txt
 *     monster_flags_map -- map between monsters and monster flags
 *     monster_spell_flags_map -- map between monsters and monster spell flags
 *     object_base_info -- dump of object_base.txt
 *     object_base_flags_map -- map between object templates and object flags
 *     object_info -- dump of objects.txt
 *     object_flags_map -- map between objects and object flags
 *     object_mods_map -- map between objects and modifiers
 *     effects_list -- dump of list-effects.h
 *     monster_flags_list -- dump of list-mon-race-flags.h
 *     object_flags_list -- dump of list-object-flags.h
 *     object_mods_list -- dump of list-object-modifiers.h
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
 *     wearables_mods
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

	err = stats_db_exec("CREATE TABLE artifact_mods_map(a_idx INT, modifier_index INT, modifier INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_info(idx INT PRIMARY KEY, name TEXT, to_h TEXT, to_d TEXT, to_a TEXT, cost INT, level INT, rarity INT, rating INT, min_to_h INT, min_to_d INT, min_to_a INT, xtra INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_flags_map(e_idx INT, o_flag INT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE ego_mods_map(e_idx INT, modifier_index INT, min_modifier INT);");
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

	err = stats_db_exec("CREATE TABLE object_mods_map(k_idx INT, modifier_index INT, modifier TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE effects_list(idx INT PRIMARY KEY, aim INT, rating INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_flags_list(idx INT PRIMARY KEY, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE monster_spell_flags_list(idx INT PRIMARY KEY, cap INT, div INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_flags_list(idx INT PRIMARY KEY, type INT, power INT, name TEXT);");
	if (err) return false;

	err = stats_db_exec("CREATE TABLE object_mods_list(idx INT PRIMARY KEY, type INT, power INT, mod_mult INT, name TEXT);");
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

	err = stats_db_exec("CREATE TABLE wearables_mods(level INT, count INT, k_idx INT, origin INT, mod INT, mod_idx INT, UNIQUE (level, k_idx, origin, mod, mod_idx) ON CONFLICT REPLACE);");
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
	else if (streq(member, "mods"))
		return offsetof(struct wearables_data, modifiers);
		
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
		for (i = 0; i < max_idx; i++) {
	/* This arcane expression finds the value of 
			 * level_data[level].<table>[i] */
			u32b count;
			if (streq(table, "gold"))
				count = *((long long *)((byte *)&level_data[level] + offset) + i);
			else
				count = *((u32b *)((byte *)&level_data[level] + offset) + i);

			if (!count) continue;

			err = stats_db_bind_ints(sql_stmt, 3, 0,
				level, count, i);
			if (err) return err;

			STATS_DB_STEP_RESET(sql_stmt)
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
		for (origin = 0; origin < ORIGIN_STATS; origin++)
			for (i = 0; i < max_idx; i++) {
				/* This arcane expression finds the value of 
				 * level_data[level].<table>[origin][i] */
				u32b count = ((u32b **)((byte *)&level_data[level] + offset))[origin][i];
				if (!count) continue;
				
				err = stats_db_bind_ints(sql_stmt, 4, 0, level, count, translate_consumables ? stats_lookup_index(consumables_index, z_info->k_max, i) : i, origin);
				if (err) return err;

				STATS_DB_STEP_RESET(sql_stmt)
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
		for (origin = 0; origin < ORIGIN_STATS; origin++)
			for (idx = 0; idx < wearable_count + 1; idx++) {
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
		for (origin = 0; origin < ORIGIN_STATS; origin++)
			for (idx = 0; idx < wearable_count + 1; idx++) {
				k_idx = stats_lookup_index(wearables_index, 
					z_info->k_max, idx);

				/* Skip if pile */
				if (! k_idx) continue;

				for (i = 0; i < max_val; i++) {
					/* This arcane expression finds the value of
					 * level_data[level].wearables[origin][idx].<field>[i] */
					u32b count;
					if (array_p)
						count = ((u32b *)((byte *)&level_data[level].wearables[origin][idx] + offset))[i];
					else
						count = ((u32b *)*((u32b **)((byte *)&level_data[level].wearables[origin][idx] + offset)))[i];

					if (!count) continue;

					err = stats_db_bind_ints(sql_stmt, 5, 0,
						level, count, k_idx, origin, i);
					if (err) return err;

					STATS_DB_STEP_RESET(sql_stmt)
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
	int max_val1, int max_val2, bool array_p)
{
	char sql_buf[256];
	sqlite3_stmt *sql_stmt;
	int err, level, origin, idx, k_idx, i, j, offset;

	strnfmt(sql_buf, 256, "INSERT INTO wearables_%s VALUES(?,?,?,?,?,?);",
			field);
	err = stats_db_stmt_prep(&sql_stmt, sql_buf);
	if (err) return err;

	offset = stats_wearables_data_offsetof(field);

	for (level = 1; level < LEVEL_MAX; level++)
		for (origin = 0; origin < ORIGIN_STATS; origin++)
			for (idx = 0; idx < wearable_count + 1; idx++) {
				k_idx = stats_lookup_index(wearables_index, 
					z_info->k_max, idx);

				/* Skip if pile */
				if (! k_idx) continue;

				for (i = 0; i < max_val1; i++)
					for (j = 0; j < max_val2; j++) {
						/* This arcane expression finds the value of
				 		* level_data[level].wearables[origin][idx].<field>[i][j]
						*/
						u32b count;

						if (i == 0 && j == 0) continue;

						if (array_p)
							count = ((u32b *)((byte *)&level_data[level].wearables[origin][idx] + offset))[i * max_val2 + j];
						else
							count = *(*((u32b **)((byte *)&level_data[level].wearables[origin][idx] + offset) + i) + j);

						if (!count) continue;

						err = stats_db_bind_ints(sql_stmt, 6, 0,
							level, count, k_idx, origin, 
							i, j);
						if (err) return err;

						STATS_DB_STEP_RESET(sql_stmt)
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

	err = stats_write_db_wearables_2d_array("dice", TOP_DICE, TOP_SIDES, true);
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

	err = stats_write_db_wearables_2d_array("mods", TOP_MOD, OBJ_MOD_MAX + 1,
											false);
	if (err) return err;

	/* Commit transaction */
	err = stats_db_exec("COMMIT;");
	if (err) return err;

	return SQLITE_OK;
}

/**
 * Call with the number of runs that have been completed.
 */

#define STATS_PROGRESS_BAR_LEN 30

static void progress_bar(u32b run, time_t start) {
	u32b i;
	u32b n = (run * STATS_PROGRESS_BAR_LEN) / num_runs;
	u32b p10 = ((long long)run * 1000) / num_runs;

	time_t delta = time(NULL) - start;
	u32b togo = num_runs - run;
	u32b expect = delta ? ((long long)delta * (long long)togo) / run 
		: 0;

	int h = expect / 3600;
	int m = (expect % 3600) / 60;
	int s = expect % 60;

	printf("\r|");
	for (i = 0; i < n; i++) printf("*");
	for (i = 0; i < STATS_PROGRESS_BAR_LEN - n; i++) printf(" ");
	printf("| %d/%d (%5.1f%%) %3d:%02d:%02d ", run, num_runs, p10/10.0, h, m,
		   s);
	fflush(stdout);
}

/**
 * Clean up memory after each run. Should only affect character and
 * dungeon structs allocated during normal initialization, not persistent 
 * data like *_info.
 */

static void stats_cleanup_angband_run(void)
{
	if (player->history) mem_free(player->history);
}

static errr run_stats(void)
{
	u32b run;
	struct artifact *a_info_save;
	unsigned int i;
	int err;
	bool status; 

	time_t start;

	prep_output_dir();
	create_indices();
	alloc_memory();
	if (randarts) {
		a_info_save = mem_zalloc(z_info->a_max * sizeof(struct artifact));
		for (i = 0; i < z_info->a_max; i++) {
			if (!a_info[i].name) continue;

			memcpy(&a_info_save[i], &a_info[i], sizeof(struct artifact));
		}
	}

	if (!quiet) printf("Creating the database and dumping info...\n");
	status = stats_prep_db();
	if (!status) quit("Couldn't prepare database!");

	if (!quiet) {
		printf("Beginning %d runs...\n", num_runs);
		fflush(stdout);
	}

	start = time(NULL);
	for (run = 1; run <= num_runs; run++) {
		if (!quiet) progress_bar(run - 1, start);

		if (randarts)
			for (i = 0; i < z_info->a_max; i++)
				memcpy(&a_info[i], &a_info_save[i], sizeof(struct artifact));

		initialize_character();
		unkill_uniques();
		reset_artifacts();
		descend_dungeon();
		stats_cleanup_angband_run();

		/* Checkpoint every so many runs */
		if (run % RUNS_PER_CHECKPOINT == 0) {
			err = stats_write_db(run);
			if (err) {
				stats_db_close();
				quit_fmt("Problems writing to database!  sqlite3 errno %d.",
						 err);
			}
		}

		if (quiet && run % 1000 == 0) {
			printf("Finished %d runs.\n", run);
			fflush(stdout);
		}
	}

	if (!quiet) {
		progress_bar(num_runs, start);
		printf("\nSaving the data...\n");
		fflush(stdout);
	}

	err = stats_write_db(run);
	stats_db_close();
	if (err) quit_fmt("Problems writing to database!  sqlite3 errno %d.", err);

	if (randarts)
		mem_free(a_info_save);
	free_stats_memory();
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

static errr term_text_stats(int x, int y, int n, int a, const wchar_t *s) {
	return 0;
}

static void term_data_link(int i) {
	term *t = &td.t;

	term_init(t, 80, 24, 256);

	/* Ignore some actions for efficiency and safety */
	t->never_bored = true;
	t->never_frosh = true;

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

const char help_stats[] = "Stats mode, subopts -q(uiet) -r(andarts) -n(# of runs) -s(no selling)";

/**
 * Usage:
 *
 * angband -mstats -- [-q] [-r] [-nNNNN] [-s]
 *
 *   -q      Quiet mode (turn off progress messages)
 *   -r      Turn on randarts
 *   -nNNNN  Make NNNN runs through the dungeon (default: 1)
 *   -s      Turn on no-selling
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
			quiet = true;
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
		printf("init-stats: bad argument '%s'\n", argv[i]);
	}

	term_data_link(0);
	return 0;
}

#endif /* USE_STATS */
