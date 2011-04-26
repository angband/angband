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

static int randarts = 0;
static u32b num_runs = 1;
static int verbose = 0;
static int nextkey = 0;
static int running_stats = 0;
static char *ANGBAND_DIR_STATS;

static ang_file *obj_fp, *mon_fp, *ainfo_fp, *rinfo_fp;

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
	char m_name[80];
	bool fear;

	for (i = cave_monster_max(cave) - 1; i >= 1; i--)
	{
		const monster_type *m_ptr = cave_monster(cave, i);
		char *offscreen_ptr;

		monster_desc(m_name, sizeof(m_name), m_ptr, 0x88);

		/* Remove " (offscreen)" from description */
		offscreen_ptr = strstr(m_name, " (offscreen)");
		if (offscreen_ptr)
		{
			*offscreen_ptr = '\0';
		}

		file_putf(mon_fp, "%d|%d|%s\n",
			level,
			m_ptr->r_idx,
			m_name);	

		mon_take_hit(i, m_ptr->hp + 1, &fear, NULL);
	}

}

static void print_all_objects(void)
{
	int x, y;
	char o_name[256];

	for (y = 1; y < DUNGEON_HGT - 1; y++)
	{
		for (x = 1; x < DUNGEON_WID - 1; x++)
		{
			object_type *o_ptr = get_first_object(y, x);
			u16b o_origin_xtra;
			u32b o_power = 0;
			char o_flags[OF_SIZE];
			u16b j;

			if (o_ptr) do
			{
				/* Mark object as fully known */
				object_notice_everything(o_ptr);
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL | ODESC_SPOIL);

				/* Report where floor items were found, 
				 * notably vaults (CAVE_ICKY) */
				if (o_ptr->origin == ORIGIN_FLOOR)
				{
					o_origin_xtra = cave->info[y][x] &
						(CAVE_ICKY | CAVE_ROOM);
				} 
				else 
				{
					o_origin_xtra = o_ptr->origin_xtra;
				}

				o_flags[OF_SIZE] = 0;
				for (j = 0; j < OF_SIZE; j++)
				{
					o_flags[j] = hexsym[o_ptr->flags[j] % 16];
				}

				o_power = object_power(o_ptr, 0, NULL, 1);


				file_putf(obj_fp, 
					"%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%s|%d|%s\n",
					o_ptr->tval,
					o_ptr->sval,
					o_ptr->pval[DEFAULT_PVAL],
					o_ptr->number,
					o_ptr->origin,
					o_ptr->origin_depth,
					o_origin_xtra,
					o_ptr->to_h,
					o_ptr->to_d,
					o_ptr->to_a,
					o_ptr->ac,
					o_ptr->dd,
					o_ptr->ds,
					o_ptr->weight,
					o_flags,
					o_power,
					o_name);
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
	file_putf(obj_fp, "tval|sval|pval|name1|name2|number|origin|origin_depth|origin_xtra|to_h|to_d|to_a|ac|dd|ds|weight|flags|power|name\n");
	file_putf(mon_fp, "level|r_idx|name\n");
	file_putf(ainfo_fp, "aidx|tval|sval|pval|to_h|to_d|to_a|ac|dd|ds|weight|flags|level|alloc_prob|alloc_min|alloc_max|effect|name\n");
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
	unsigned int i, j;
	char flags[OF_SIZE];

	flags[OF_SIZE] = 0;

	for (i = 0; i < z_info->a_max; i++)
	{
		artifact_type *a_ptr = &a_info[i];	

		/* Don't print anything for "empty" artifacts */
		if (!a_ptr->name) continue;

		for (j = 0; j < OF_SIZE; j++)
		{
			flags[j] = hexsym[a_ptr->flags[j] % 16];
		}

		file_putf(ainfo_fp,
			"%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%s|%d|%d|%d|%d|%d|%s\n",
			a_ptr->aidx,
			a_ptr->tval,
			a_ptr->sval,
			a_ptr->pval,
			a_ptr->to_h,
			a_ptr->to_d,
			a_ptr->to_a,
			a_ptr->ac,
			a_ptr->dd,
			a_ptr->ds,
			a_ptr->weight,
			a_ptr->cost,
			flags,
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

	for (level = 1; level < 100; level++) 
	{
		dungeon_change_level(level);
		cave_generate(cave, p_ptr);
		kill_all_monsters(level);
		print_all_objects();
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

	prep_output_dir();

	for (run = 0; run < num_runs; run++)
	{
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
