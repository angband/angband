/* File: create.c */

/* Purpose: create a player character */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"


/* Lets do all prototypes correctly.... -CWS */
#ifndef NO_LINT_ARGS
#ifdef __STDC__

static void change_stat(int, int);
static int  monval(int);
static void get_stats(void);
static void set_prev_stats(void);
static int  get_prev_stats(void);
static void get_all_stats(void);
static void put_auto_stats(void);
static void choose_race(void);
static void print_history(void);
static void set_prev_history(void);
static void get_prev_history(void);
static void get_sex(void);
static void get_ahw(void);
static void set_prev_ahw(void);
static void get_prev_ahw(void);
static void get_class(void);
static void get_class_choice(void);
static void get_money(void);

#endif
#endif


typedef struct _previous previous;

struct _previous {
    int16u age;
    int16u wt;
    int16u ht;
    int16  disarm;
    int16u stat[6];
    int16u sc;
    char   history[4][60];
    player_background	bg;
};


/*
 * Previous stats
 */
static previous prev;


/*
 * Generates character's stats			-JWT-	 
 */
static void get_stats()
{
    register int i, tot;
    int dice[18];

    do {
	tot = 0;
	for (i = 0; i < 18; i++) {
	    dice[i] = randint(3 + i % 3); /* Roll 3,4,5 sided dice once each */
	    tot += dice[i];
	}
    }
    while (tot <= 42 || tot >= 54);

    for (i = 0; i < 6; i++) {
	p_ptr->max_stat[i] = 5 + dice[3 * i] + dice[3 * i + 1] +
	    dice[3 * i + 2];
    }
}


/* Returns adjusted stat                                       -JK-  */
/* Algorithm by ...                                            -JWT- */
/* Used by change_stats and auto_roller
 * auto_roll is boolean and states maximum changes
 * should be used rather than random ones
 * to allow specification of higher values to wait for
 */

static int adjust_stat(int stat_value, int16 amount, int auto_roll)
{
    register int i;

    /* OK so auto_roll conditions not needed for negative amounts */
    /* since stat_value is always 15 at least currently!  -JK */
    if (amount < 0) {
	for (i = 0; i > amount; i--) {

	    if (stat_value > 108)
		stat_value--;
	    else if (stat_value > 88)
		stat_value -= ((auto_roll ? 6 : randint(6)) + 2);
	    else if (stat_value > 18) {
		stat_value -= ((auto_roll ? 15 : randint(15)) + 5);
		if (stat_value < 18) stat_value = 18;
	    }
	    else if (stat_value > 3)
		stat_value--;
	}
    }

    else {
	for (i = 0; i < amount; i++) {
	    if (stat_value < 18)
		stat_value++;
	    else if (stat_value < 88)
		stat_value += ((auto_roll ? 15 : randint(15)) + 5);
	    else if (stat_value < 108)
		stat_value += ((auto_roll ? 6 : randint(6)) + 2);
	    else if (stat_value < 118)
		stat_value++;
	}
    }

    return stat_value;
}


/*
 * Changes stats by given amount                                -JWT-   
 */
static void change_stat(int stat, int amount)
{
    p_ptr->max_stat[stat] = adjust_stat(p_ptr->max_stat[stat],
					(int16) amount, FALSE);
}


/*
 * Save the old stats
 */
static void set_prev_stats()
{
    register int        i;

    for (i = 0; i < 6; i++) {
	prev.stat[i] = (int16u)(p_ptr->max_stat[i]);
    }

    return;
}

static int get_prev_stats()
{
    register int        i;

    if (!prev.stat[0]) return 0;

    for (i = 0; i < 6; i++) {
	p_ptr->cur_stat[i] = prev.stat[i];
	p_ptr->max_stat[i] = prev.stat[i];
	p_ptr->use_stat[i] = prev.stat[i];
    }

    p_ptr->ptodam = todam_adj();
    p_ptr->ptohit = tohit_adj();
    p_ptr->pac = toac_adj();

    prev.stat[0] = 0;

    return 1;
}


/*
 * Generate all stats and modify for race.
 * Needed in a separate module so looping of character
 * selection would be allowed     -RGM- 
 *
 * It strikes me that nobody was clearing the players
 * experience points (from past lives).  It looks like
 * some vague accident took care of this...
 */
static void get_all_stats(void)
{
    register player_race *rp_ptr;

    register int        j;

    rp_ptr = &race[p_ptr->prace];

    get_stats();
    change_stat(A_STR, rp_ptr->str_adj);
    change_stat(A_INT, rp_ptr->int_adj);
    change_stat(A_WIS, rp_ptr->wis_adj);
    change_stat(A_DEX, rp_ptr->dex_adj);
    change_stat(A_CON, rp_ptr->con_adj);
    change_stat(A_CHR, rp_ptr->chr_adj);

    for (j = 0; j < 6; j++) {
	p_ptr->cur_stat[j] = p_ptr->max_stat[j];
	p_ptr->use_stat[j] = modify_stat(j, p_ptr->mod_stat[j]);
    }

    p_ptr->expfact = rp_ptr->b_exp;
    p_ptr->lev = 1;

    p_ptr->hitdie = rp_ptr->bhitdie;

    p_ptr->srh = rp_ptr->srh;
    p_ptr->fos = rp_ptr->fos;
    p_ptr->stl = rp_ptr->stl;
    p_ptr->save = rp_ptr->bsav;
    p_ptr->see_infra = rp_ptr->infra;

    p_ptr->bth = rp_ptr->bth;
    p_ptr->bthb = rp_ptr->bthb;
    p_ptr->ptoac = 0;
    p_ptr->pac = toac_adj();
    p_ptr->ptodam = todam_adj();
    p_ptr->ptohit = tohit_adj();
}

/*
 * copied from misc2.c, so the display loop would work nicely -cft
 */
static cptr stat_names[] = {
    "STR: ", "INT: ", "WIS: ", "DEX: ", "CON: ", "CHR: "
};


#ifdef AUTOROLLER

/*
 * Used for auto-roller.
 * Just put_stats(), w/o the extra info -CFT 
 */
static void put_auto_stats()
{
    register int i;
    vtype        buf;

    for (i = 0; i < 6; i++) {
	cnv_stat(p_ptr->use_stat[i], buf);
	c_put_str(COLOR_L_GREEN, buf, 2 + i, 66);
	if (p_ptr->max_stat[i] > p_ptr->cur_stat[i]) {
	    cnv_stat(p_ptr->max_stat[i], buf);
	    c_put_str(COLOR_L_GREEN, buf, 2 + i, 73);
	}
    }
}
#endif

/* Allows player to select a race			-JWT-	 */
static void choose_race(void)
{
    register int         j, k;
    int                  l, m, exit_flag;
    char                 s;
    char                 tmp_str[80];

    register player_race *rp_ptr;

    j = 0;
    k = 0;
    l = 2;
    m = 21;

    clear_from(20);
    put_str("Choose a race (? for Help):", 20, 2);
    do {
	(void)sprintf(tmp_str, "%c) %s", k + 'a', race[j].trace);
	put_str(tmp_str, m, l);
	k++;
	l += 15;
	if (l > 70) {
	    l = 2;
	    m++;
	}
	j++;
    }
    while (j < MAX_RACES);

    exit_flag = FALSE;

    do {
	move_cursor(20, 30);
	s = inkey();
	j = s - 'a';
	if ((j < MAX_RACES) && (j >= 0))
	    exit_flag = TRUE;
	else if (s == '?')
	    helpfile(ANGBAND_WELCOME);
	else
	    bell();
    }
    while (!exit_flag);

    rp_ptr = &race[j];
    p_ptr->prace = j;

    c_put_str(COLOR_L_BLUE, rp_ptr->trace, 3, 15);
}


/*
 * Will print the history of a character			-JWT-	 
 */
static void print_history()
{
    register int        i;

    put_str("Character Background", 14, 27);
    for (i = 0; i < 4; i++) {
	prt(history[i], i + 15, 10);
    }
}


static void set_prev_history()
{
    register int	i;

    prev.bg.info = background->info;
    prev.bg.roll = background->roll;
    prev.bg.chart = background->chart;
    prev.bg.next = background->next;
    prev.bg.bonus = background->bonus;

    prev.sc = p_ptr->sc;

    for (i = 0; i < 4; i++) {
	(void)strncpy(prev.history[i], history[i], 60);
    }
}


static void get_prev_history(void)
{
    register int        i;

    background->info = prev.bg.info;
    background->roll = prev.bg.roll;
    background->chart = prev.bg.chart;
    background->next = prev.bg.next;
    background->bonus = prev.bg.bonus;

    p_ptr->sc = prev.sc;

    for (i = 0; i < 4; i++) {
	strncpy(history[i], prev.history[i], 60);
    }
}


/* Get the racial history, determines social class	-RAK-	 */
/* Assumptions:	Each race has init history beginning at
 * (race-1)*3+1
 * All history parts are in ascending order
 */

static void get_history(void)
{
    int                      hist_idx, cur_idx, test_roll, flag;
    register int             start_pos, end_pos, cur_len;
    int                      line_ctr, new_start = 0, social_class;
    char                     history_block[240];
    player_background		*bp_ptr;

    /* Get a block of history text				 */
    if (p_ptr->prace == 8)
	hist_idx = 1;
    else if (p_ptr->prace > 8)
	hist_idx = 2 * 3 + 1;
    else
	hist_idx = p_ptr->prace * 3 + 1;

    history_block[0] = '\0';
    social_class = randint(4);
    cur_idx = 0;
    do {
	flag = FALSE;
	do {
	    if (background[cur_idx].chart == hist_idx) {
		test_roll = randint(100);
		while (test_roll > background[cur_idx].roll)
		    cur_idx++;
		bp_ptr = &background[cur_idx];
		(void)strcat(history_block, bp_ptr->info);
		social_class += bp_ptr->bonus - 50;
		if (hist_idx > bp_ptr->next) cur_idx = 0;
		hist_idx = bp_ptr->next;
		flag = TRUE;
	    }
	    else {
		cur_idx++;
	    }
	}
	while (!flag);
    }
    while (hist_idx >= 1);

/* clear the previous history strings */
    for (hist_idx = 0; hist_idx < 4; hist_idx++) {
	history[hist_idx][0] = '\0';
    }

/* Process block of history text for pretty output	 */
    start_pos = 0;
    end_pos = strlen(history_block) - 1;
    line_ctr = 0;
    flag = FALSE;
    while (history_block[end_pos] == ' ')
	end_pos--;
    do {
	while (history_block[start_pos] == ' ')
	    start_pos++;
	cur_len = end_pos - start_pos + 1;
	if (cur_len > 60) {
	    cur_len = 60;
	    while (history_block[start_pos + cur_len - 1] != ' ')
		cur_len--;
	    new_start = start_pos + cur_len;
	    while (history_block[start_pos + cur_len - 1] == ' ')
		cur_len--;
	}
	else {
	    flag = TRUE;
	}

	(void)strncpy(history[line_ctr],
		&history_block[start_pos], cur_len);
	history[line_ctr][cur_len] = '\0';
	line_ctr++;
	start_pos = new_start;
    }
    while (!flag);

/* Compute social class for player			 */
    if (social_class > 100)
	social_class = 100;
    else if (social_class < 1)
	social_class = 1;
    p_ptr->sc = social_class;
}


/*
 * Gets the character's sex				-JWT-	 
 */
static void get_sex(void)
{
    register int        exit_flag;
    char                c;

    exit_flag = FALSE;
    clear_from(20);
    put_str("Choose a sex (? for Help):", 20, 2);
    put_str("m) Male       f) Female", 21, 2);
    do {
	move_cursor(20, 29);
    /* speed not important here */
	c = inkey();
	if (c == 'f' || c == 'F') {
	    p_ptr->male = FALSE;
	    c_put_str(COLOR_L_BLUE, "Female", 4, 15);
	    exit_flag = TRUE;
	}
	else if (c == 'm' || c == 'M') {
	    p_ptr->male = TRUE;
	    c_put_str(COLOR_L_BLUE, "Male", 4, 15);
	    exit_flag = TRUE;
	}
	else if (c == '?')
	    helpfile(ANGBAND_WELCOME);
	else
	    bell();
    }
    while (!exit_flag);
}


/*
 * Computes character's age, height, and weight		-JWT-	 
 */
static void get_ahw(void)
{
    register int        i;

    i = p_ptr->prace;
    p_ptr->age = race[i].b_age + randint((int)race[i].m_age);
    if (p_ptr->male) {
	p_ptr->ht = randnor((int)race[i].m_b_ht, (int)race[i].m_m_ht);
	p_ptr->wt = randnor((int)race[i].m_b_wt, (int)race[i].m_m_wt);
    }
    else {
	p_ptr->ht = randnor((int)race[i].f_b_ht, (int)race[i].f_m_ht);
	p_ptr->wt = randnor((int)race[i].f_b_wt, (int)race[i].f_m_wt);
    }
    p_ptr->disarm += race[i].b_dis;
}


static void set_prev_ahw(void)
{
    prev.age = p_ptr->age;
    prev.wt = p_ptr->wt;
    prev.ht = p_ptr->ht;
    prev.disarm = p_ptr->disarm;

    return;
}


static void get_prev_ahw(void)
{
    p_ptr->age = prev.age;
    p_ptr->wt = prev.wt;
    p_ptr->ht = prev.ht;
    p_ptr->disarm = prev.disarm;
    prev.age = prev.wt = prev.ht = prev.disarm = 0;
}


/*
 * Gets a character class				-JWT-	 
 */
static void get_class(void)
{
    register int        i;
    int                 min_value, max_value;
    int                 percent;
    char                buf[50];

    player_class		*cp_ptr = &class[p_ptr->pclass];

    change_stat(A_STR, cp_ptr->madj_str);
    change_stat(A_INT, cp_ptr->madj_int);
    change_stat(A_WIS, cp_ptr->madj_wis);
    change_stat(A_DEX, cp_ptr->madj_dex);
    change_stat(A_CON, cp_ptr->madj_con);
    change_stat(A_CHR, cp_ptr->madj_chr);

    for (i = 0; i < 6; i++) {
	p_ptr->cur_stat[i] = p_ptr->max_stat[i];
	p_ptr->use_stat[i] = p_ptr->max_stat[i];
    }
    p_ptr->ptodam = todam_adj();           /* Real values		 */
    p_ptr->ptohit = tohit_adj();
    p_ptr->ptoac = toac_adj();
    p_ptr->pac = 0;
    p_ptr->dis_td = p_ptr->ptodam;	/* Displayed values	 */
    p_ptr->dis_th = p_ptr->ptohit;
    p_ptr->dis_tac = p_ptr->ptoac;
    p_ptr->dis_ac = p_ptr->pac + p_ptr->dis_tac;

/* now set misc stats, do this after setting stats because of con_adj() for
 * hitpoints 
 */

    p_ptr->hitdie += cp_ptr->adj_hd;
    p_ptr->mhp = con_adj() + p_ptr->hitdie;
    p_ptr->chp = p_ptr->mhp;
    p_ptr->chp_frac = 0;

/* initialize hit_points array: put bounds on total possible hp,
 * only succeed if it is within 1/8 of average value
 */
    min_value = (MAX_PLAYER_LEVEL * 3 * (p_ptr->hitdie - 1)) / 8 +
	MAX_PLAYER_LEVEL;
    max_value = (MAX_PLAYER_LEVEL * 5 * (p_ptr->hitdie - 1)) / 8 +
	MAX_PLAYER_LEVEL;

    player_hp[0] = p_ptr->hitdie;
    do {
	for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
	    player_hp[i] = randint((int)p_ptr->hitdie);
	    player_hp[i] += player_hp[i - 1];
	}
    }
    while ((player_hp[MAX_PLAYER_LEVEL - 1] < min_value) ||
	   (player_hp[MAX_PLAYER_LEVEL - 1] > max_value));

    if (peek) {
	percent = (int)(((long)player_hp[MAX_PLAYER_LEVEL - 1] * 200L) /
		(p_ptr->hitdie + ((MAX_PLAYER_LEVEL - 1) * p_ptr->hitdie)));
	sprintf(buf, "%d%% Life Rating", percent);
	msg_print(buf);
    }
    p_ptr->bth += cp_ptr->mbth;
    p_ptr->bthb += cp_ptr->mbthb;   /* RAK */
    p_ptr->srh += cp_ptr->msrh;
    p_ptr->disarm = cp_ptr->mdis + todis_adj();
    p_ptr->fos += cp_ptr->mfos;
    p_ptr->stl += cp_ptr->mstl;
    p_ptr->save += cp_ptr->msav;
    p_ptr->expfact += cp_ptr->m_exp;
}


/*
 * Gets a character class				-JWT-	 
 */
static void get_class_choice()
{
    register int i, j;
    int          k, l, m;
    int          cl[MAX_CLASS], exit_flag;
    char         tmp_str[80], s;
    int32u       mask;

    for (j = 0; j < MAX_CLASS; j++) cl[j] = 0;

    i = p_ptr->prace;
    j = 0;
    k = 0;
    l = 2;
    m = 21;
    mask = 0x1;
    clear_from(20);
    put_str("Choose a class (? for Help):", 20, 2);
    do {
	if (race[i].rtclass & mask) {
	    (void)sprintf(tmp_str, "%c) %s", k + 'a', class[j].title);
	    put_str(tmp_str, m, l);
	    cl[k] = j;
	    l += 15;
	    if (l > 70) {
		l = 2;
		m++;
	    }
	    k++;
	}
	j++;
	mask <<= 1;
    }
    while (j < MAX_CLASS);
    p_ptr->pclass = 0;
    exit_flag = FALSE;
    do {
	move_cursor(20, 31);
	s = inkey();
	j = s - 'a';
	if ((j < k) && (j >= 0)) {
	    p_ptr->pclass = cl[j];
	    c_put_str(COLOR_L_BLUE, class[p_ptr->pclass].title, 5, 15);
	    clear_from(20);
	    exit_flag = TRUE;
	}
	else if (s == '?')
	    helpfile(ANGBAND_WELCOME);
	else
	    bell();
    } while (!exit_flag);
}


/*
 * Given a stat value, return a monetary value, which affects the amount of
 * gold a player has. 
 */
static int monval(int i)
{
    return (5 * ((int)i - 10));
}


static void get_money(void)
{
    register int        gold;
    register int16	*ip;

    /* Index into the stats */
    ip = p_ptr->max_stat;

    /* Social Class adj */
    gold = p_ptr->sc * 6 + randint(25) + 325;

    /* Stat adj */
    gold -= monval(ip[A_STR]);
    gold -= monval(ip[A_INT]);
    gold -= monval(ip[A_WIS]);
    gold -= monval(ip[A_CON]);
    gold -= monval(ip[A_DEX]);

    /* Charisma adj	 */
    gold += monval(ip[A_CHR]);

    /* She charmed the banker into it! -CJS- */
    /* She slept with the banker.. :) -GDH-  */
    if (!p_ptr->male) gold += 50;

    /* Minimum 80 gold */
    if (gold < 80) gold = 80;

    /* Save the gold (minimum 80) */
    p_ptr->au = gold;
}



/*** ---------- M A I N  for Character Creation Routine ---------- ***/


/*
 * Clear all the global "character" data
 */
static void player_wipe()
{
    int i;


    /* Hack -- zero the struct */
    WIPE(p_ptr, player_type);


    /* Start with no artifacts made yet */
    for (i = 0; i < ART_MAX; i++) v_list[i].cur_num = 0;


    /* Start with no quests */
    for (i = 0; i < QUEST_MAX; i++) q_list[i].level = 0;

    /* Add a special quest */
    q_list[0].level = 99;

    /* Add a second quest */
    q_list[1].level = 100;


    /* Reset the "current" and "maximum" monster populations */
    for (i=0; i<MAX_R_IDX; i++) {

	/* None alive right now */
	l_list[i].cur_num = 0;

	/* Hack -- Assume we can have 255 of them */
	l_list[i].max_num = 255;

	/* But we can only have 1 unique */
	if (r_list[i].cflags2 & MF2_UNIQUE) l_list[i].max_num = 1;
    }

    /* Reset the "tried" and "aware" flags for objects */
    for (i=0; i<MAX_K_IDX; i++) {

	/* Not aware of the effects */
	x_list[i].aware = FALSE;

	/* Never even tried one */
	x_list[i].tried = FALSE;
    }        


    /* Well fed player */
    p_ptr->food = 7500;
    p_ptr->food_digested = 2;


    /* Wipe the spells */
    spell_learned = spell_learned2 = 0L;
    spell_worked = spell_worked2 = 0L;
    spell_forgotten = spell_forgotten2 = 0L;
    for (i = 0; i < 64; i++) spell_order[i] = 99;

    /* Hack -- reset seeds */
    town_seed = random();
    randes_seed = random();
}



/*
 * Create a character.  Then wait for a moment.
 *
 * The delay may be reduced, but is recommended to keep players
 * from continuously rolling up characters, which can be VERY
 * expensive CPU wise.
 *
 * Note that we may be called with "junk" leftover in the various
 * fields, so we must be sure to clear them first.
 */
void player_birth()
{
    register char       c;

#ifdef AUTOROLLER

    register int	i;
    int32u		auto_round = 0;
    int32u		last_round = 0;
    int			stat[6];
    int			autoroll = 0;
    int			msstat = 0;    /* Max autoroll w/ look for -SAC */
    player_class	*cp_ptr;
    player_race		*rp_ptr;
    int			stat_idx = 0;

    static cptr stat_name[6] = {
	"Strength", "Intelligence", "Wisdom",
	"Dexterity", "Constitution", "Charisma"
    };

    char		inp[60];

#endif

    /* Do not use "previous" data until it exists */
    int previous_exists = 0;


    /* Clear old information */
    player_wipe();


    /* Get the basic choices */
    put_character();
    choose_race();
    get_sex();
    get_class_choice();


#ifdef AUTOROLLER

/*
 * This auto-roller stolen from a post on rec.games.moria, which I belive was
 * taken from druid moria 5.something.  If this intrudes on someone's
 * copyright, take it out, and someone let me know -CFT 
 */

    /* allow multiple key entry, so they can ask for help and */
    /* still get back to this menu... -CFT */
    put_str("Do you want to use automatic rolling? (? for Help) ", 20, 2);
    while (1) {
	move_cursor(20, 52);
	c = inkey();
	if (c == '?') helpfile(ANGBAND_WELCOME);
	else if (strchr("ynYN", c)) break;
    }


    /* Prepare the autoroller */
    if ((c == 'Y') || (c == 'y')) {
	autoroll = 1;
	clear_from(15);
	cp_ptr = &class[p_ptr->pclass];
	rp_ptr = &race[p_ptr->prace];
	put_str("Enter minimum attribute for: ", 15, 2);

	/* Check the stats */
	for (i = 0; i < 6; i++) {

	    switch (i) {
	      case 0:
		stat_idx = A_STR;
		msstat = cp_ptr->madj_str + rp_ptr->str_adj;
		break;
	      case 1:
		stat_idx = A_INT;
		msstat = cp_ptr->madj_int + rp_ptr->int_adj;
		break;
	      case 2:
		stat_idx = A_WIS;
		msstat = cp_ptr->madj_wis + rp_ptr->wis_adj;
		break;
	      case 3:
		stat_idx = A_DEX;
		msstat = cp_ptr->madj_dex + rp_ptr->dex_adj;
		break;
	      case 4:
		stat_idx = A_CON;
		msstat = cp_ptr->madj_con + rp_ptr->con_adj;
		break;
	      case 5:
		stat_idx = A_CHR;
		msstat = cp_ptr->madj_chr + rp_ptr->chr_adj;
		break;
	    }

	    msstat = adjust_stat(17, msstat, TRUE);
	    clear_from(16 + i);
	    sprintf(inp, "%-12s (Max of %2d): ", stat_name[i], msstat);
	    put_str(inp, 16 + i, 5);

	    while (1) {

		/* Treat "Escape" like "Return" */
		if (!get_string(inp, 16 + i, 32, 3)) inp[0] = '\0';

		/* Parse the input */
		stat[stat_idx] = atoi(inp);

		/* Use negative numbers to avoid "max stat" setting */
		if (stat[stat_idx] < 0) {
		    stat[stat_idx] = (-stat[stat_idx]);
		    if (stat[stat_idx] > msstat) msstat = stat[stat_idx];
		}

		/* Enforce minimum of "3" (handle "return") */
		if (stat[stat_idx] < 3) stat[stat_idx] = 3;

		/* Break on valid input */
		if (stat[stat_idx] <= msstat) break;
	    }
	}

	/* Dump results */
	Term_fresh();
    }

#endif				   /* AUTOROLLER - main setup code */


    /* Actually Generate */
    while (1) {

	clear_from(9);

#ifdef AUTOROLLER

	if (autoroll) {
	    last_round = auto_round;
	    for (i = 2; i < 6; i++)
		erase_line(i, 30);
		for (i = 0; i < 6; i++) {
		    put_str(stat_names[i], 2 + i, 61);
		}
		put_str("Auto-rolling round #",20,2);
		put_str("Hit any key to stop.",22,2);
	}

	else {
	    get_all_stats();
	    get_class();
	}


	/* Start of AUTOROLLing loop */
	while (autoroll) {

	    get_all_stats();
	    get_class();

	    put_auto_stats();
	    auto_round++;
	    sprintf(inp, "%9lu.", (long)auto_round);
	    put_str(inp, 20, 22);

#if defined(NICE)
	    /* Wait 1/10 second */
	    delay(100);

	    /* When nice, force occasional quit */
	    if (auto_round > last_round + 9999) {
		autoroll = FALSE;
		msg_print("You are too picky.  Autoroll cancelled.");
		msg_print(NULL);
		break;
	    }
#endif

	    /* Make sure they see everything */
	    Term_fresh();

	    /* Let the auto-roller get stopped by keypresses */
	    if (kbhit()) {
		flush();
		break;
	    }

	    /* Break if happy */
	    if ((stat[A_STR] <= p_ptr->cur_stat[A_STR]) &&
		(stat[A_INT] <= p_ptr->cur_stat[A_INT]) &&
		(stat[A_WIS] <= p_ptr->cur_stat[A_WIS]) &&
		(stat[A_DEX] <= p_ptr->cur_stat[A_DEX]) &&
		(stat[A_CON] <= p_ptr->cur_stat[A_CON]) &&
		(stat[A_CHR] <= p_ptr->cur_stat[A_CHR])) break;
	}

#else

	get_all_stats();
	get_class();

#endif				   /* AUTOROLLER main looping section */

	/* Get and Display some player stats */
	get_history();
	get_ahw();
	calc_bonuses();
	print_history();
	put_misc1();
	put_stats();
	clear_from(20);

	/* Input loop */
	while (1) {

	    if (previous_exists) {
		prt("Hit space: Reroll, ^P: Previous or ESC: Accept: ", 20, 2);
	    }
	    else {
		prt("Hit space: Reroll, or ESC: Accept: ", 20, 2);
	    }

	    c = inkey();

	    if ((c == ESCAPE) || (c == ' ')) break;

	    if ((previous_exists) && (c == CTRL('P'))) {

		previous_exists = FALSE;

		if (get_prev_stats()) {
		    get_prev_history();
		    get_prev_ahw();
		    calc_bonuses();
		    print_history();
		    put_misc1();
		    put_stats();
		    clear_from(20);
		}

		continue;
	    }

	    /* Warn */
	    bell();
	}

	/* Are we done? (hit ESCAPE) */
	if (c != ' ') break;	

	/* Save this for the "previous" character */
	set_prev_stats();
	set_prev_history();
	set_prev_ahw();

	/* Remember that he exists */
	previous_exists = TRUE;
    }


    /* Calculate the money */
    get_money();

    /* Display the character */
    put_stats();
    put_misc2();
    put_misc3();

    /* Get a name, recolor it, prepare savefile */
    get_name();

#if !defined(MACINTOSH) && !defined(MSDOS)
    /* Pause for a moment */
    (void)sleep(1);
#endif

    prt("[Press any key to continue, or Q to commit suicide]", 23, 15);
    c = inkey();
    erase_line(23, 0);
    Term_fresh();

    /* Allow Suicide */
    if (c == 'Q') quit(NULL);
}

