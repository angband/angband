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
#include "birth.h"
#include "buildid.h"
#include "init.h"
#include "object/pval.h"
#include "object/tvalsval.h"

#define OBJ_FEEL_MAX	 10
#define MON_FEEL_MAX 	  9
#define LEVEL_MAX 		100

static int randarts = 0;
static u32b num_runs = 1;
static int verbose = 0;
static int nextkey = 0;
static int running_stats = 0;
static char *ANGBAND_DIR_STATS;

static ang_file *obj_fp, *mon_fp, *ainfo_fp, *rinfo_fp;

static struct level_data {
	u32b **kinds;
	u32b ***egos;
	u32b **arts;
	u32b ***flags;
	u32b *monsters;
/*  u32b *vaults;  Add these later - requires passing into generate.c
	u32b *pits; */
	u32b obj_feeling[OBJ_FEEL_MAX];
	u32b mon_feeling[MON_FEEL_MAX];
} level_data[100];

static void alloc_memory()
{
	int i;

	for (i = 0; i < LEVEL_MAX; i++) {
		level_data[i].kinds[ORIGIN_MAX] = C_ZNEW(z_info->k_max, u32b);
		level_data[i].egos[ORIGIN_MAX][z_info->k_max]
			= C_ZNEW(z_info->e_max, u32b);
		level_data[i].arts[ORIGIN_MAX] = C_ZNEW(z_info->a_max, u32b);
		level_data[i].flags[OF_MAX][MAX_PVAL] = C_ZNEW(z_info->k_max, u32b);
		level_data[i].monsters = C_ZNEW(z_info->r_max, u32b);
/*		level_data[i].vaults = C_ZNEW(z_info->v_max, u32b);
		level_data[i].pits = C_ZNEW(z_info->pit_max, u32b); */
	}
}

/* Copied from birth.c:generate_player() */
static void generate_player_for_stats()
{
	OPT(birth_randarts) = randarts;
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

	store_init();
	flavor_init();
	p_ptr->playing = TRUE;
	p_ptr->autosave = FALSE;
	cave_generate(cave, p_ptr);
}

static void kill_all_monsters(int level)
{
	int i;

	for (i = cave_monster_max(cave) - 1; i >= 1; i--) {
		const monster_type *m_ptr = cave_monster(cave, i);

		level_data[level].monsters[m_ptr->r_idx]++;

		monster_death(i, TRUE);
	}
}

/*
 * Insert into out_str a hex representation of the bitflag flags[].
 */
static void flag2hex(const bitflag flags[], char *out_str)
{
	unsigned int i;

	out_str[2*OF_SIZE] = 0;

	for (i = 0; i < OF_SIZE; i++)
		strnfmt(out_str + 2 * i, 2 * OF_SIZE - 2 * i + 1, "%02x", flags[i]);
}

/*
 * Insert into pvals a comma-separated list of pvals in pval[],
 * and insert into pval_flags a comma-separated list of hex
 * representations of the bitflags in pval_flags[].
 */

static void dump_pvals(char *pval_string, char *pval_flag_string,
	s16b pval[], bitflag pval_flags[][OF_SIZE], byte num_pvals)
{
	unsigned int i;
	size_t pval_end = 0;
	size_t pval_flag_end = 0;
	char buf[64];

	if (num_pvals <= 0) return;

	for (i = 0; (i + 1) < num_pvals; i++)
	{
		strnfcat(pval_string, 20, &pval_end, "%d,", pval[i]);
		flag2hex(pval_flags[i], buf);
		strnfcat(pval_flag_string, 128, &pval_flag_end, "%s,", buf);
	}

	strnfcat(pval_string, 20, &pval_end, "%d", pval[num_pvals - 1]);
	flag2hex(pval_flags[num_pvals - 1], buf);
	strnfcat(pval_flag_string, 128, &pval_flag_end, "%s", buf);
}

static void log_all_objects(int level)
{
	int x, y, i;

	for (y = 1; y < DUNGEON_HGT - 1; y++) {
		for (x = 1; x < DUNGEON_WID - 1; x++) {
			object_type *o_ptr = get_first_object(y, x);

			if (o_ptr) do {
				u32b o_power = 0;

				/* Mark object as fully known */
				object_notice_everything(o_ptr);

				o_power = object_power(o_ptr, FALSE, NULL, TRUE);

				/* Capture kind / ego / artifact origins */
				level_data[level].kinds[o_ptr->origin][o_ptr->kind->kidx]++;
				if (o_ptr->artifact)
					level_data[level].arts[o_ptr->origin][o_ptr->artifact->aidx]++;
				else if (o_ptr->ego)
					level_data[level].egos[o_ptr->origin][o_ptr->kind->kidx][o_ptr->ego->eidx]++;

				/* Capture gold amounts */
				if (o_ptr->tval == TV_GOLD)
					level_data[level].flags[0][o_ptr->pval[0]][o_ptr->kind->kidx]++;
				/* Capture object flags */
				else
					for (i = of_next(o_ptr->flags, FLAG_START); i != FLAG_END;
							i = of_next(o_ptr->flags, i + 1))
						level_data[level].flags[i][flag_uses_pval(i) ? o_ptr->pval[which_pval(o_ptr, i)] : 0][o_ptr->kind->kidx]++;
			}
			while ((o_ptr = get_next_object(o_ptr)));
		}
	}
}

static void open_output_files(u32b run)
{
	char buf[1024];
	char run_dir[1024];

	strnfmt(run_dir, 1024, "%s%s%010d", ANGBAND_DIR_STATS, PATH_SEP, run);

	if (!dir_create(run_dir))
	{
		quit("Couldn't create stats run directory!");
	}

	path_build(buf, sizeof(buf), run_dir, "objects.txt");
	obj_fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	path_build(buf, sizeof(buf), run_dir, "monsters.txt");
	mon_fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	path_build(buf, sizeof(buf), run_dir, "ainfo.txt");
	ainfo_fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);
	path_build(buf, sizeof(buf), run_dir, "rinfo.txt");
	rinfo_fp = file_open(buf, MODE_WRITE, FTYPE_TEXT);

	/* Print headers */
	file_putf(obj_fp, "tval|sval|pvals|name1|name2|number|origin|origin_depth|origin_xtra|to_h|to_d|to_a|ac|dd|ds|weight|flags|pval_flags|power|name\n");
	file_putf(mon_fp, "level|r_idx|name\n");
	file_putf(ainfo_fp, "aidx|tval|sval|pvals|to_h|to_d|to_a|ac|dd|ds|weight|flags|pval_flags|level|alloc_prob|alloc_min|alloc_max|effect|name\n");
	file_putf(rinfo_fp, "ridx|level|rarity|d_char|name\n");
}

static void close_output_files(void)
{
	file_close(obj_fp);
	file_close(mon_fp);
	file_close(ainfo_fp);
	file_close(rinfo_fp);
}

static void dump_ainfo(void)
{
	unsigned int i;

	for (i = 0; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];
		char a_flags[128] = "";
		char a_pvals[20] = "";
		char a_pval_flags[128] = "";

		/* Don't print anything for "empty" artifacts */
		if (!a_ptr->name) continue;

		flag2hex(a_ptr->flags, a_flags);

		if (a_ptr->num_pvals > 0)
			dump_pvals(a_pvals, a_pval_flags, a_ptr->pval,
				a_ptr->pval_flags, a_ptr->num_pvals);

		file_putf(ainfo_fp,
			"%d|%d|%d|%s|%d|%d|%d|%d|%d|%d|%d|%d|%s|%s|%d|%d|%d|%d|%d|%s\n",
			a_ptr->aidx,
			a_ptr->tval,
			a_ptr->sval,
			a_pvals,
			a_ptr->to_h,
			a_ptr->to_d,
			a_ptr->to_a,
			a_ptr->ac,
			a_ptr->dd,
			a_ptr->ds,
			a_ptr->weight,
			a_ptr->cost,
			a_flags,
			a_pval_flags,
			a_ptr->level,
			a_ptr->alloc_prob,
			a_ptr->alloc_min,
			a_ptr->alloc_max,
			a_ptr->effect,
			a_ptr->name);
	}
}

static void dump_rinfo(void)
{
	unsigned int i;

	for (i = 0; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Don't print anything for "empty" artifacts */
		if (!r_ptr->name) continue;

	        /* ridx|level|rarity|d_char|name */
		file_putf(rinfo_fp,
			"%d|%d|%d|%c|%s\n",
			r_ptr->ridx,
			r_ptr->level,
			r_ptr->rarity,
			r_ptr->d_char,
			r_ptr->name);
	}
}

static void descend_dungeon(void)
{
	int level;
	u16b obj_f = cave->feeling / 10;
	u16b mon_f = cave->feeling - (10 * obj_f);

	for (level = 1; level < 100; level++)
	{
		dungeon_change_level(level);
		cave_generate(cave, p_ptr);

		/* Store level feelings */
		level_data[level].obj_feeling[MIN(obj_f, OBJ_FEEL_MAX)]++;
		level_data[level].mon_feeling[MIN(mon_f, MON_FEEL_MAX)]++;

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

static errr run_stats(void)
{
	u32b run;
	artifact_type *a_info_save;
	unsigned int i;

	prep_output_dir();
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

	for (run = 0; run < num_runs; run++)
	{
		if (randarts)
		{
			for (i = 0; i < z_info->a_max; i++)
			{
				memcpy(&a_info[i], &a_info_save[i], sizeof(artifact_type));
			}
		}

		initialize_character();
		open_output_files(run);
		dump_ainfo();
		dump_rinfo();
		descend_dungeon();
		close_output_files();
	}
	cleanup_angband();
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
	if (verbose) printf("term-init %s %s\n", VERSION_NAME, VERSION_STRING);
}

static void term_nuke_stats(term *t) {
	if (verbose) printf("term-end\n");
}

static errr term_xtra_clear(int v) {
	if (verbose) printf("term-xtra-clear %d\n", v);
	return 0;
}

static errr term_xtra_noise(int v) {
	if (verbose) printf("term-xtra-noise %d\n", v);
	return 0;
}

static errr term_xtra_fresh(int v) {
	if (verbose) printf("term-xtra-fresh %d\n", v);
	return 0;
}

static errr term_xtra_shape(int v) {
	if (verbose) printf("term-xtra-shape %d\n", v);
	return 0;
}

static errr term_xtra_alive(int v) {
	if (verbose) printf("term-xtra-alive %d\n", v);
	return 0;
}

static errr term_xtra_event(int v) {
	if (verbose) printf("term-xtra-event %d\n", v);
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
	if (verbose) printf("term-xtra-flush %d\n", v);
	return 0;
}

static errr term_xtra_delay(int v) {
	if (verbose) printf("term-xtra-delay %d\n", v);
	return 0;
}

static errr term_xtra_react(int v) {
	if (verbose) printf("term-xtra-react\n");
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
	if (verbose) printf("term-xtra-unknown %d %d\n", n, v);
	return 0;
}

static errr term_curs_stats(int x, int y) {
	if (verbose) printf("term-curs %d %d\n", x, y);
	return 0;
}

static errr term_wipe_stats(int x, int y, int n) {
	if (verbose) printf("term-wipe %d %d %d\n", x, y, n);
	return 0;
}

static errr term_text_stats(int x, int y, int n, byte a, const char *s) {
	if (verbose) printf("term-text %d %d %d %02x %s\n", x, y, n, a, s);
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

const char help_stats[] = "Stats mode, subopts -r(andarts)";

/* 
 * Usage:
 *
 * angband -mstats -- [-r] [-nNNNN]
 *
 *   -r      Turn on randarts
 *   -nNNNN  Make NNNN runs through the dungeon (default: 1)
 */ 

errr init_stats(int argc, char *argv[]) {
	int i;

	/* Skip over argv[0] */
	for (i = 1; i < argc; i++) {
		if (streq(argv[i], "-r")) {
			randarts = 1;
			continue;
		}
		if (prefix(argv[i], "-n")) {
			num_runs = atoi(&argv[i][2]);
			continue;
		}
		printf("init-stats: bad argument '%s'\n", argv[i]);
	}

	term_data_link(0);
	return 0;
}
