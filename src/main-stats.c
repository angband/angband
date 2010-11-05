/** @file main-stats.c
 *  @brief Pseudo-UI for stats generation (borrows heavily from main-test.c)
 *  @author Robert Au <myshkin+angband@durak.net>
 */

#include "angband.h"
#include "buildid.h"

static int randarts = 0;
static int verbose = 0;
static int nextkey = 0;
static int running_stats = 0;

/* Copied from birth.c:generate_player() */
static void generate_player_for_stats()
{
	OPT(birth_randarts) = randarts;
	OPT(birth_no_stacking) = FALSE;
	OPT(quick_messages) = TRUE;
	OPT(auto_more) = TRUE;

	p_ptr->wizard = 1; /* Set wizard mode on */

	p_ptr->psex = 0;   /* Female  */
	p_ptr->race = races;  /* Human   */
	p_ptr->class = classes; /* Warrior */

	sp_ptr = &sex_info[p_ptr->psex];
	rp_ptr = p_ptr->race;
	cp_ptr = p_ptr->class;
	mp_ptr = &cp_ptr->spells;

	/* Level 1 */
	p_ptr->max_lev = p_ptr->lev = 1;

	/* Experience factor */
	p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

	/* Hitdice */
	p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

	/* Initial hitpoints -- high just to be safe */
	p_ptr->mhp = p_ptr->chp = 2000;

	/* Pre-calculate level 1 hitdice */
	p_ptr->player_hp[0] = p_ptr->hitdie;

	/* Set age/height/weight */
	p_ptr->ht = p_ptr->ht_birth = 66;
	p_ptr->wt = p_ptr->wt_birth = 150;
	p_ptr->age = 14;

	/* Set social class and (null) history */
	p_ptr->history[0] = '\0';
	p_ptr->sc = p_ptr->sc_birth = 50;
}

static void initialize_character(void) 
{
	u32b seed;

	seed = (time(NULL));
	Rand_quick = FALSE;
	Rand_state_init(seed);

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
	generate_cave();
}

static void kill_all_monsters(void)
{
	int i;
	char m_name[80];

	for (i = mon_max - 1; i >= 1; i--)
	{
		const monster_type *m_ptr = &mon_list[i];
		char *offscreen_ptr;

		monster_desc(m_name, sizeof(m_name), m_ptr, 0x80);

		/* Remove " (offscreen)" from description */
		offscreen_ptr = strstr(m_name, " (offscreen)");
		if (offscreen_ptr)
		{
			*offscreen_ptr = '\0';
		}

		printf("M|%d|%s\n",
			m_ptr->r_idx,
			m_name);	

		monster_death(i);
	}

}

static void print_all_objects(void)
{
	int x, y;
	char o_name[256];

	/* Get stats on objects */
	for (y = 1; y < DUNGEON_HGT - 1; y++)
	{
		for (x = 1; x < DUNGEON_WID - 1; x++)
		{
			const object_type *o_ptr = get_first_object(y, x);
			u16b o_origin_xtra;

			if (o_ptr) do
			{
				/* Mark object as fully known */
				object_notice_everything(o_ptr);
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL | ODESC_SPOIL);

				/* Report where floor items were found, 
				 * notably vaults (CAVE_ICKY) */
				if (o_ptr->origin == ORIGIN_FLOOR)
				{
					o_origin_xtra = cave_info[y][x] &
						(CAVE_ICKY | CAVE_ROOM);
				} 
				else 
				{
					o_origin_xtra = o_ptr->origin_xtra;
				}
				printf("%d|%d|%d|%d|%d|%d|%d|%s\n",
					o_ptr->tval,
					o_ptr->sval,
					o_ptr->pval[DEFAULT_PVAL],
					o_ptr->number,
					o_ptr->origin,
					o_ptr->origin_depth,
					o_origin_xtra,
					o_name);
			}
			while ((o_ptr = get_next_object(o_ptr)));
		}
	}
}

static void descend_dungeon(void) 
{
	int level;

	for (level = 1; level < 100; level++) 
	{
		dungeon_change_level(level);
		generate_cave();
		kill_all_monsters();
		print_all_objects();
	}
}

static errr run_stats(void) {
	initialize_character();
	descend_dungeon();

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

static errr term_user_stats(int n) {
	if (verbose) printf("term-user %d\n", n);
	return 0;
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

errr init_stats(int argc, char *argv[]) {
	int i;

	/* Skip over argv[0] */
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-r")) {
			randarts = 1;
			continue;
		}
		printf("init-stats: bad argument '%s'\n", argv[i]);
	}

	term_data_link(0);
	return 0;
}
