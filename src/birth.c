/* File: birth.c */

/* Purpose: create a player character */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"



/*
 * Hold the data from the previous "roll"
 */
static struct {

    s16b age;
    s16b wt;
    s16b ht;
    s16b sc;

    s16b stat[6];

    s32b au;

    char history[4][60];

    player_background bg;

} prev;



/*
 * Is the structure below ready
 */
static int prev_ready = FALSE;





/*
 * Save the current data for later
 */
static void save_prev_data()
{
    register int i;

    prev.age = p_ptr->age;

    prev.wt = p_ptr->wt;
    prev.ht = p_ptr->ht;

    prev.sc = p_ptr->sc;

    prev.au = p_ptr->au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
	prev.stat[i] = p_ptr->max_stat[i];
    }

    for (i = 0; i < 4; i++) {
	(void)strncpy(prev.history[i], history[i], 60);
    }

    prev.bg.info = background->info;
    prev.bg.roll = background->roll;
    prev.bg.chart = background->chart;
    prev.bg.next = background->next;
    prev.bg.bonus = background->bonus;
}


/*
 * Load the previous data
 */
static void load_prev_data()
{
    register int        i;

    for (i = 0; i < 6; i++) {
	p_ptr->cur_stat[i] = prev.stat[i];
	p_ptr->max_stat[i] = prev.stat[i];
	p_ptr->use_stat[i] = prev.stat[i];
    }

    p_ptr->age = prev.age;
    p_ptr->wt = prev.wt;
    p_ptr->ht = prev.ht;

    p_ptr->sc = prev.sc;

    p_ptr->au = prev.au;

    for (i = 0; i < 4; i++) {
	strncpy(history[i], prev.history[i], 60);
    }

    background->info = prev.bg.info;
    background->roll = prev.bg.roll;
    background->chart = prev.bg.chart;
    background->next = prev.bg.next;
    background->bonus = prev.bg.bonus;
}








/*
 * Choose the character's sex				-JWT-	 
 */
static void choose_sex(void)
{
    char        c;

    clear_from(20);
    put_str("m) Male       f) Female", 21, 2);

    while (1) {
	put_str("Choose a sex (? for Help): ", 20, 2);
	move_cursor(20, 29);
	c = inkey();
	if (c == 'm' || c == 'M') {
	    p_ptr->male = TRUE;
	    c_put_str(TERM_L_BLUE, "Male", 3, 15);
	    break;
	}
	else if (c == 'f' || c == 'F') {
	    p_ptr->male = FALSE;
	    c_put_str(TERM_L_BLUE, "Female", 3, 15);
	    break;
	}
	else if (c == '?') {
	    helpfile(ANGBAND_WELCOME);
	}
	else {
	    bell();
	}
    }
}


/*
 * Allows player to select a race			-JWT-	 
 */
static void choose_race(void)
{
    int                  j, k, l, m;
    char                 s, tmp_str[80];

    k = 0;
    l = 2;
    m = 21;

    clear_from(20);

    for (j = 0; j < MAX_RACES; j++) {
	(void)sprintf(tmp_str, "%c) %s", j + 'a', race[j].trace);
	put_str(tmp_str, m, l);
	l += 15;
	if (l > 70) {
	    l = 2;
	    m++;
	}
    }

    while (1) {
	put_str("Choose a race (? for Help): ", 20, 2);
	s = inkey();
	j = s - 'a';
	if ((j < MAX_RACES) && (j >= 0)) {
	    p_ptr->prace = j;
	    c_put_str(TERM_L_BLUE, race[j].trace, 4, 15);
	    break;
	}
	else if (s == '?') {
	    helpfile(ANGBAND_WELCOME);
	}
	else {
	    bell();
	}
    }
}



/*
 * Gets a character class				-JWT-	 
 */
static void choose_class()
{
    int          j, k, l, m;
    int          cl[MAX_CLASS];
    char         tmp_str[80], s;

    /* Clear the display */
    clear_from(20);

    /* Prepare to list */
    k = 0;
    l = 2;
    m = 21;

    /* No legal choices yet */
    for (j = 0; j < MAX_CLASS; j++) cl[j] = 0;

    /* Display the legal choices */
    for (j = 0; j < MAX_CLASS; j++) {
	if (race[p_ptr->prace].rtclass & (1L << j)) {
	    sprintf(tmp_str, "%c) %s", k + 'a', class[j].title);
	    put_str(tmp_str, m, l);
	    cl[k++] = j;
	    l += 15;
	    if (l > 70) {
		l = 2;
		m++;
	    }
	}
    }

    /* Get a class */
    while (1) {
	put_str("Choose a class (? for Help): ", 20, 2);
	s = inkey();
	j = s - 'a';
	if ((j < k) && (j >= 0)) {
	    p_ptr->pclass = cl[j];
	    c_put_str(TERM_L_BLUE, class[p_ptr->pclass].title, 5, 15);
	    clear_from(20);
	    break;
	}
	else if (s == '?') {
	    helpfile(ANGBAND_WELCOME);
	}
	else {
	    bell();
	}
    }
}





/*
 * Returns adjusted stat -JK-
 * Algorithm by -JWT-
 *
 * Used by change_stats and auto_roller
 *
 * auto_roll is boolean and states maximum changes should be used rather
 * than random ones to allow specification of higher values to wait for
 */
static int adjust_stat(int stat_value, s16b amount, int auto_roll)
{
    register int i;

    /* Negative amounts (unused) */
    if (amount < 0) {
	for (i = 0; i > amount; i--) {

	    if (stat_value > 108) {
		stat_value--;
	    }
	    else if (stat_value > 88) {
		stat_value -= ((auto_roll ? 6 : randint(6)) + 2);
	    }
	    else if (stat_value > 18) {
		stat_value -= ((auto_roll ? 15 : randint(15)) + 5);
		if (stat_value < 18) stat_value = 18;
	    }
	    else if (stat_value > 3) {
		stat_value--;
	    }
	}
    }

    /* Positive amounts */
    else {
	for (i = 0; i < amount; i++) {
	    if (stat_value < 18) {
		stat_value++;
	    }
	    else if (stat_value < 88) {
		stat_value += ((auto_roll ? 15 : randint(15)) + 5);
	    }
	    else if (stat_value < 108) {
		stat_value += ((auto_roll ? 6 : randint(6)) + 2);
	    }
	    else if (stat_value < 118) {
		stat_value++;
	    }
	}
    }

    /* Return the result */
    return stat_value;
}


/*
 * Changes stats by given amount
 */
static void change_stat(int stat, int amount)
{
    int max = p_ptr->max_stat[stat];
    int tmp = adjust_stat(max, amount, FALSE);
    p_ptr->max_stat[stat] = tmp;
}



/*
 * Generate all stats and modify for race.
 * Needed in a separate module so looping of character
 * selection would be allowed     -RGM- 
 */
static void get_all_stats(void)
{
    register int        i, j;
    int                 min_value, max_value;

    int dice[18];

    player_race		*rp_ptr = &race[p_ptr->prace];
    player_class	*cp_ptr = &class[p_ptr->pclass];


    /* Roll and verify some stats */
    while (TRUE) {

	/* Roll some dice */
	for (j = i = 0; i < 18; i++) {
	    dice[i] = randint(3 + i % 3);
	    j += dice[i];
	}

	/* Verify totals */
	if (j > 42 && j < 54) break;
    }

    /* Each stat is 5 + 1d3 + 1d4 + 1d5 */
    for (i = 0; i < 6; i++) {
	j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];
	p_ptr->max_stat[i] = j;
    }
    
    
    /* Modify the stats for "race" */
    change_stat(A_STR, rp_ptr->str_adj);
    change_stat(A_INT, rp_ptr->int_adj);
    change_stat(A_WIS, rp_ptr->wis_adj);
    change_stat(A_DEX, rp_ptr->dex_adj);
    change_stat(A_CON, rp_ptr->con_adj);
    change_stat(A_CHR, rp_ptr->chr_adj);

    /* Analyze the stats */
    for (j = 0; j < 6; j++) {
	p_ptr->cur_stat[j] = p_ptr->max_stat[j];
	p_ptr->use_stat[j] = modify_stat(j, p_ptr->mod_stat[j]);
    }


    /* Modify the stats for "class" */
    change_stat(A_STR, cp_ptr->madj_str);
    change_stat(A_INT, cp_ptr->madj_int);
    change_stat(A_WIS, cp_ptr->madj_wis);
    change_stat(A_DEX, cp_ptr->madj_dex);
    change_stat(A_CON, cp_ptr->madj_con);
    change_stat(A_CHR, cp_ptr->madj_chr);

    /* Analyze the stats */
    for (i = 0; i < 6; i++) {
	p_ptr->cur_stat[i] = p_ptr->max_stat[i];
	p_ptr->use_stat[i] = p_ptr->max_stat[i];
    }


    /* Experience factor XXX Why here? */
    p_ptr->expfact = rp_ptr->b_exp + cp_ptr->m_exp;

    /* Hitdice XXX Why here? */
    p_ptr->hitdie = rp_ptr->bhitdie + cp_ptr->adj_hd;


    /* Minimum hitpoints at highest level */
    min_value = (MAX_PLAYER_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
    min_value += MAX_PLAYER_LEVEL;

    /* Maximum hitpoints at highest level */
    max_value = (MAX_PLAYER_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
    max_value += MAX_PLAYER_LEVEL;

    /* Pre-calculate level 1 hitdice */
    player_hp[0] = p_ptr->hitdie;

    /* Roll out the hitpoints */
    while (TRUE) {

	/* Roll the hitpoint values */
	for (i = 1; i < MAX_PLAYER_LEVEL; i++) {
	    j = randint((int)p_ptr->hitdie);
	    player_hp[i] = player_hp[i-1] + j;
	}

	/* XXX Could also require acceptable "mid-level" hitpoints */
	
	/* Require "valid" hitpoints at highest level */
	if (player_hp[MAX_PLAYER_LEVEL-1] < min_value) continue;
	if (player_hp[MAX_PLAYER_LEVEL-1] > max_value) continue;

	/* Acceptable */
	break;
    }


    /* Hack -- calculate the bonuses */
    calc_bonuses();

    /* Hack -- Reset starting hitpoints */
    p_ptr->mhp = p_ptr->hitdie + con_adj();

    /* Hack -- Restart as Fully healed */
    p_ptr->chp = p_ptr->mhp;
    p_ptr->chp_frac = 0;
}


#ifdef AUTOROLLER

/*
 * Display stat values, subset of "put_stats()"
 */
static void birth_put_stats()
{
    register int i;
    vtype        buf;

    /* Put the stats */
    for (i = 0; i < 6; i++) {
	cnv_stat(p_ptr->use_stat[i], buf);
	c_put_str(TERM_L_GREEN, buf, 2 + i, 66);
    }
}

#endif


/*
 * Will print the history of a character			-JWT-	 
 */
static void put_history()
{
    register int        i;

    clear_from(15);
    
    put_str("Character Background", 15, 27);
    for (i = 0; i < 4; i++) {
	put_str(history[i], i + 16, 10);
    }
}


/*
 * Get the racial history, determines social class	-RAK-
 * Assumptions:	Each race has init history beginning at
 * (race-1)*3+1.  All history parts are in ascending order
 */

static void get_history(void)
{
    int                      hist_idx, cur_idx, test_roll, flag;
    register int             start_pos, end_pos, cur_len;
    int                      line_ctr, new_start = 0, social_class;
    char                     history_block[240];
    player_background		*bp_ptr;

    /* Special race */
    if (p_ptr->prace == 8) {
	hist_idx = 1;
    }

    /* Special race */
    else if (p_ptr->prace > 8) {
	hist_idx = 2 * 3 + 1;
    }

    /* Normal races */
    else {
	hist_idx = p_ptr->prace * 3 + 1;
    }

    history_block[0] = '\0';
    social_class = randint(4);
    cur_idx = 0;

    /* Process the history */
    while (hist_idx >= 1) {
	for (flag = FALSE; !flag; ) {
	    if (background[cur_idx].chart == hist_idx) {
		test_roll = randint(100);
		while (test_roll > background[cur_idx].roll) cur_idx++;
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
    }

    /* clear the previous history strings */
    for (hist_idx = 0; hist_idx < 4; hist_idx++) {
	history[hist_idx][0] = '\0';
    }

    /* Process block of history text for pretty output	 */
    start_pos = 0;
    end_pos = strlen(history_block) - 1;
    line_ctr = 0;
    flag = FALSE;
    while (history_block[end_pos] == ' ') end_pos--;

    for (flag = FALSE; !flag; ) {
	while (history_block[start_pos] == ' ') start_pos++;
	cur_len = end_pos - start_pos + 1;
	if (cur_len > 60) {
	    cur_len = 60;
	    while (history_block[start_pos + cur_len - 1] != ' ') cur_len--;
	    new_start = start_pos + cur_len;
	    while (history_block[start_pos + cur_len - 1] == ' ') cur_len--;
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

    /* Verify social class */
    if (social_class > 100) social_class = 100;
    else if (social_class < 1) social_class = 1;

    /* Save the social class */
    p_ptr->sc = social_class;
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(void)
{
    register int        i;

    i = p_ptr->prace;
    
    /* Calculate the starting age */
    p_ptr->age = race[i].b_age + randint((int)race[i].m_age);

    /* Calculate the height/weight for males */
    if (p_ptr->male) {
	p_ptr->ht = randnor((int)race[i].m_b_ht, (int)race[i].m_m_ht);
	p_ptr->wt = randnor((int)race[i].m_b_wt, (int)race[i].m_m_wt);
    }

    /* Calculate the height/weight for females */
    else {
	p_ptr->ht = randnor((int)race[i].f_b_ht, (int)race[i].f_m_ht);
	p_ptr->wt = randnor((int)race[i].f_b_wt, (int)race[i].f_m_wt);
    }
}




/*
 * Given a stat value, return a monetary value,
 * which affects the amount of gold a player has. 
 */
static int monval(int i)
{
    return (5 * (i - 10));
}

/*
 * Get the player's starting money
 */
static void get_money(void)
{
    register int        gold;

    /* Social Class is very important */
    gold = p_ptr->sc * 6 + randint(25) + 325;

    /* Stat adj */
    gold -= monval(p_ptr->max_stat[A_STR]);
    gold -= monval(p_ptr->max_stat[A_INT]);
    gold -= monval(p_ptr->max_stat[A_WIS]);
    gold -= monval(p_ptr->max_stat[A_CON]);
    gold -= monval(p_ptr->max_stat[A_DEX]);

    /* Charisma adj */
    gold += monval(p_ptr->max_stat[A_CHR]);

    /* Minimum 80 gold */
    if (gold < 80) gold = 80;

    /* She charmed the banker into it! -CJS- */
    /* She slept with the banker.. :) -GDH-  */
    if (!p_ptr->male) gold += 50;

    /* Save the gold */
    p_ptr->au = gold;
}


/*
 * Clear all the global "character" data
 */
static void player_wipe()
{
    int i;


    /* Hack -- zero the struct */
    WIPE(p_ptr, player_type);


    /* No items */
    inven_ctr = equip_ctr = 0;

    /* No weight */
    inven_weight = 0;

    /* Clear the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {
	invcopy(&inventory[i], OBJ_NOTHING);
    }


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

	/* Assume we can have 100 of them alive at the same time */
	l_list[i].max_num = 100;

	/* But we can only have 1 unique at a time */
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


    /* Start at level one */    
    p_ptr->lev = 1;


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
    char		c;
    bool		use_history;

    player_class	*cp_ptr;
    player_race		*rp_ptr;

#ifdef AUTOROLLER

    register int	i;
    u32b		auto_round = 0;
    u32b		last_round = 0;
    int			stat[6];
    int			autoroll = 0;
    int			msstat = 0;
    int			stat_idx = 0;

    static cptr stat_name[6] = {
	"Strength", "Intelligence", "Wisdom",
	"Dexterity", "Constitution", "Charisma"
    };

    char		inp[60];

#endif


    /* Clear old information */
    player_wipe();


    /* Clear the screen */
    clear_screen();

    /* Title everything */
    put_str("Name        :", 2, 1);
    put_str("Sex         :", 3, 1);
    put_str("Race        :", 4, 1);
    put_str("Class       :", 5, 1);

    /* Choose a sex */
    choose_sex();

    /* Choose a race */
    choose_race();

    /* Choose a class */
    choose_class();

    /* Access the race/class */
    cp_ptr = &class[p_ptr->pclass];
    rp_ptr = &race[p_ptr->prace];


#ifdef AUTOROLLER

/*
 * This auto-roller stolen from a post on rec.games.moria, which I belive was
 * taken from druid moria 5.something.  If this intrudes on someone's
 * copyright, take it out, and someone let me know -CFT 
 */

    /* Prompt for it */
    put_str("Do you want to use automatic rolling? (? for Help) ", 20, 2);

    /* Process help requests until a choice is made */
    while (1) {
	move_cursor(20, 52);
	c = inkey();
	if (c == '?') helpfile(ANGBAND_WELCOME);
	else if (strchr("ynYN", c)) break;
    }


    /* Prepare the autoroller */
    if ((c == 'Y') || (c == 'y')) {

	autoroll = TRUE;

	clear_from(15);
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

	/* Clear the old data */
	clear_from(9);

#ifdef AUTOROLLER

	/* Autoroller needs some feedback */
	if (autoroll) {

	    /* Note when we started */
	    last_round = auto_round;

	    /* Re-caption the screen */
	    put_character();
	    put_stats();
	    
	    /* Indicate the state */
	    put_str("Auto-rolling round #",21,2);
	    put_str("Hit any key to stop.",21,40);
	}

	/* Otherwise just get a character */
	else {
	    get_all_stats();
	}


	/* Start of AUTOROLLing loop */
	while (autoroll) {

	    /* Get a new character */
	    get_all_stats();

	    /* Advance the round */
	    auto_round++;

#if defined(NICE)

	    /* See how many rolls have gone by */
	    i = (auto_round - last_round);
	    
	    /* Update the display occasionally */
	    if ((i < 20) ||
		((i < 100) && (auto_round % 10 == 0)) ||
		((auto_round % 100 == 0))) {

		/* Dump data every few rounds */
		birth_put_stats();
		sprintf(inp, "%9lu.", (long)auto_round);
		put_str(inp, 21, 22);
	    }

#if 0	    
	    /* Disallow picky rolling */
	    if (i > 9000) {
		autoroll = FALSE;
		msg_print("You are too picky.  Autoroll cancelled.");
		msg_print(NULL);
		break;
	    }
#endif

	    /* Wait 1/10 second per roll */
	    delay(10);

#else

	    /* Dump data every round */
	    birth_put_stats();
	    sprintf(inp, "%9lu.", (long)auto_round);
	    put_str(inp, 21, 22);

#endif

	    /* Make sure they see everything */
	    Term_fresh();

	    /* Let the auto-roller get stopped by keypresses */
	    if (Term_kbhit()) {
		flush();
		break;
	    }

	    /* Break if "happy" */
	    if ((stat[A_STR] <= p_ptr->cur_stat[A_STR]) &&
		(stat[A_INT] <= p_ptr->cur_stat[A_INT]) &&
		(stat[A_WIS] <= p_ptr->cur_stat[A_WIS]) &&
		(stat[A_DEX] <= p_ptr->cur_stat[A_DEX]) &&
		(stat[A_CON] <= p_ptr->cur_stat[A_CON]) &&
		(stat[A_CHR] <= p_ptr->cur_stat[A_CHR])) break;
	}

#else

	/* No autoroller */
	get_all_stats();

#endif

	/* Calculate stuff */
	get_history();
	get_ahw();
	get_money();


	/* Start with the "Misc" data */
	use_history = FALSE;

	/* Input loop */
	while (1) {

	    /* Calculate all the bonuses */
	    calc_bonuses();

	    /* Display the player */
	    display_player();

	    /* Hack -- display history instead */
	    if (use_history) put_history();
	    
	    /* Prepare a prompt */
	    Term_putstr(2, 21, -1, TERM_WHITE, "Hit ");
	    if (prev_ready) Term_addstr(-1, TERM_WHITE, "'P' for Previous, ");
	    if (use_history) Term_addstr(-1, TERM_WHITE, "'H' for Misc., ");
	    else Term_addstr(-1, TERM_WHITE, "'H' for History, ");
	    Term_addstr(-1, TERM_WHITE, "'R' to Reroll, or ESCAPE to Accept: ");

	    /* Prompt and get a command */
	    c = inkey();

	    /* Escape accepts the roll */
	    if (c == ESCAPE) break;
	    
	    /* Space (or 'R') rerolls */
	    if ((c == ' ') || (c == 'R') || (c == 'r')) break;

	    /* Go back to last character with 'P' */
	    if (prev_ready && ((c == 'P') || (c == 'p'))) {
		prev_ready = FALSE;
		load_prev_data();		    
		continue;
	    }
	    
	    /* Print the History/MiscData */
	    if ((c == 'H') || (c == 'h')) {
		use_history = !use_history;
		continue;
	    }

	    /* Warning */
	    bell();
	}

	/* Are we done? */
	if (c == ESCAPE) break;	

	/* Save this for the "previous" character */
	save_prev_data();

	/* Remember that he exists */
	prev_ready = TRUE;
    }


    /* Clear from line 20 down */
    clear_from(20);
    
    /* Get a name, recolor it, prepare savefile */
    get_name();

    /* Prompt for it */    
    prt("[Press any key to continue, or Q to commit suicide]", 23, 15);
    Term_fresh();

#if !defined(MACINTOSH) && !defined(MSDOS)
    /* Hack -- Pause for a moment */
    (void)delay(1000);
#endif

    /* Get a key */
    c = inkey();

    /* Allow Total Suicide */
    if (c == 'Q') quit(NULL);
}


