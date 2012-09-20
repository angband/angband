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

    s16b max_stat[6];
    s16b cur_stat[6];
    s16b mod_stat[6];
    s16b use_stat[6];

    s32b au;

    char history[4][60];

    player_background bg;

} prev;





/*
 * Save the current data for later
 */
static void save_prev_data()
{
    int i;

    prev.age = p_ptr->age;

    prev.wt = p_ptr->wt;
    prev.ht = p_ptr->ht;

    prev.sc = p_ptr->sc;

    prev.au = p_ptr->au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
        prev.max_stat[i] = p_ptr->max_stat[i];
        prev.cur_stat[i] = p_ptr->cur_stat[i];
        prev.mod_stat[i] = p_ptr->mod_stat[i];
        prev.use_stat[i] = p_ptr->use_stat[i];
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
    int        i;

    for (i = 0; i < 6; i++) {
        p_ptr->max_stat[i] = prev.max_stat[i];
        p_ptr->cur_stat[i] = prev.cur_stat[i];
        p_ptr->mod_stat[i] = prev.mod_stat[i];
        p_ptr->use_stat[i] = prev.use_stat[i];
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
        else if (c == '=') {
            do_cmd_options();
        }
        else if (c == '?') {
            do_cmd_help("help.hlp");
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
        p_ptr->prace = j;
        rp_ptr = &race_info[p_ptr->prace];
        (void)sprintf(tmp_str, "%c) %s", j + 'a', rp_ptr->trace);
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
            rp_ptr = &race_info[p_ptr->prace];
            c_put_str(TERM_L_BLUE, rp_ptr->trace, 4, 15);
            break;
        }
        else if (s == '=') {
            do_cmd_options();
        }
        else if (s == '?') {
            do_cmd_help("help.hlp");
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
        if (rp_ptr->rtclass & (1L << j)) {
            p_ptr->pclass = j;
            cp_ptr = &class_info[p_ptr->pclass];
            sprintf(tmp_str, "%c) %s", k + 'a', cp_ptr->title);
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
            cp_ptr = &class_info[p_ptr->pclass];
            c_put_str(TERM_L_BLUE, cp_ptr->title, 5, 15);
            clear_from(20);
            break;
        }
        else if (s == '=') {
            do_cmd_options();
        }
        else if (s == '?') {
            do_cmd_help("help.hlp");
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
    int i;

    /* Negative amounts (unused) */
    if (amount < 0) {
        for (i = 0; i > amount; i--) {

            if (p_ptr->maximize && stat_value > 18) {
                stat_value -= 10;
                if (stat_value < 18) stat_value = 18;
            }
            else if (stat_value > 108) {
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
            else if (p_ptr->maximize) {
                stat_value += 10;
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
 * Roll for a characters stats
 */
static void get_all_stats(void)
{
    int        i, j;
    int                 min_value, max_value;

    int dice[18];


    /* Roll and verify some stats */
    while (TRUE) {

        /* Roll some dice */
        for (j = i = 0; i < 18; i++) {
            dice[i] = randint(3 + i % 3);
            j += dice[i];
        }

        /* Verify totals */
        if ((j > 42) && (j < 54)) break;
    }

    /* Each stat is 5 + 1d3 + 1d4 + 1d5 */
    for (i = 0; i < 6; i++) {
        j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];
        p_ptr->max_stat[i] = j;
    }


    /* Variable stat maxes */
    if (p_ptr->maximize) {

        /* Hack -- Acquire stats */
        for (i = 0; i < 6; i++) {

            /* Start out maximized */
            p_ptr->cur_stat[i] = p_ptr->max_stat[i];

            /* Modify the stats for "race" and "class" */
            p_ptr->mod_stat[i] = rp_ptr->radj[i] + cp_ptr->cadj[i];

            /* Apply the racial/class bonuses */
            p_ptr->use_stat[i] = modify_stat(i, p_ptr->mod_stat[i]);
        }
    }

    /* Fixed stat maxes */
    else {

        /* Apply the racial modifiers */
        for (i = 0; i < 6; i++) {

            /* Modify the stats for "race" */
            change_stat(i, rp_ptr->radj[i]);

            /* Modify the stats for "class" */
            change_stat(i, cp_ptr->cadj[i]);

            /* No bonuses */
            p_ptr->mod_stat[i] = 0;

            /* Start out fully healed */
            p_ptr->cur_stat[i] = p_ptr->max_stat[i];

            /* Use the current stats */
            p_ptr->use_stat[i] = p_ptr->max_stat[i];
        }
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


    /* Hack -- calculate the bonuses and hitpoints */
    p_ptr->update |= (PU_BONUS | PU_HP);

    /* Handle (non-visual) stuff */
    handle_stuff(FALSE);


    /* Hack -- Reset starting hitpoints */
    p_ptr->mhp = p_ptr->hitdie + con_adj();

    /* Start out fully healed */
    p_ptr->chp = p_ptr->mhp;
    p_ptr->chp_frac = 0;
}


#ifdef ALLOW_AUTOROLLER

static int		autoroll;
static int		stat_limit[6];
static u32b		stat_match[6];
static u32b		auto_round;
static u32b		last_round;



static cptr stat_name[6] = {
    "Strength", "Intelligence", "Wisdom",
    "Dexterity", "Constitution", "Charisma"
};


/*
 * Display stat values, subset of "put_stats()"
 */
static void birth_put_stats()
{
    int		i, p;
    byte	attr;

    char	buf[80];


    /* Put the stats (and percents) */
    for (i = 0; i < 6; i++) {

        /* Put the stat */
        cnv_stat(p_ptr->use_stat[i], buf);
        c_put_str(TERM_L_GREEN, buf, 2 + i, 66);

        /* Put the percent */
        if (stat_match[i]) {
            p = 1000 * stat_match[i] / auto_round;
            attr = (p < 100) ? TERM_YELLOW : TERM_L_GREEN;
            sprintf(buf, "%3d.%d%%", p/10, p%10);
            c_put_str(attr, buf, 2 + i, 73);
        }
        else {
            c_put_str(TERM_RED, "(NONE)", 2 + i, 73);
        }
    }
}

#endif


/*
 * Will print the history of a character			-JWT-	
 */
static void put_history()
{
    int        i;

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
    int		hist_idx, cur_idx, test_roll, flag;
    int		start_pos, end_pos, cur_len;
    int		line_ctr, new_start = 0, social_class;

    char	history_block[240];

    player_background	*bp_ptr;


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
    /* Calculate the starting age */
    p_ptr->age = rp_ptr->b_age + randint(rp_ptr->m_age);

    /* Calculate the height/weight for males */
    if (p_ptr->male) {
        p_ptr->ht = randnor(rp_ptr->m_b_ht, rp_ptr->m_m_ht);
        p_ptr->wt = randnor(rp_ptr->m_b_wt, rp_ptr->m_m_wt);
    }

    /* Calculate the height/weight for females */
    else {
        p_ptr->ht = randnor(rp_ptr->f_b_ht, rp_ptr->f_m_ht);
        p_ptr->wt = randnor(rp_ptr->f_b_wt, rp_ptr->f_m_wt);
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
    int        gold;

    /* Social Class is very important */
    gold = p_ptr->sc * 6 + randint(25) + 325;

    /* Stat adj */
    gold -= monval(p_ptr->use_stat[A_STR]);
    gold -= monval(p_ptr->use_stat[A_INT]);
    gold -= monval(p_ptr->use_stat[A_WIS]);
    gold -= monval(p_ptr->use_stat[A_CON]);
    gold -= monval(p_ptr->use_stat[A_DEX]);

    /* Charisma adj */
    gold += monval(p_ptr->use_stat[A_CHR]);

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
        invwipe(&inventory[i]);
    }


    /* Start with no artifacts made yet */
    for (i = 0; i < ART_MAX; i++) v_list[i].cur_num = 0;


    /* Start with no quests */
    for (i = 0; i < MAX_Q_IDX; i++) q_list[i].level = 0;

    /* Add a special quest */
    q_list[0].level = 99;

    /* Add a second quest */
    q_list[1].level = 100;


    /* Reset the "current" and "maximum" monster populations */
    for (i = 1; i < MAX_R_IDX; i++) {

        monster_race *r_ptr = &r_list[i];
        monster_lore *l_ptr = &l_list[i];

        /* None alive right now */
        l_ptr->cur_num = 0;

        /* Assume we can have 100 of them alive at the same time */
        l_ptr->max_num = 100;

        /* But we can only have 1 unique at a time */
        if (r_ptr->rflags1 & RF1_UNIQUE) l_ptr->max_num = 1;

        /* Clear player kills */
        l_ptr->pkills = 0;
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
    spell_learned1 = spell_learned2 = 0L;
    spell_worked1 = spell_worked2 = 0L;
    spell_forgotten1 = spell_forgotten2 = 0L;
    for (i = 0; i < 64; i++) spell_order[i] = 99;


    /* The player has not won yet */
    total_winner = FALSE;

    /* Reset the "state" flags */
    panic_save = 0;
    noscore = 0;

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

    bool		prev_ready;
    bool		use_history;

#ifdef ALLOW_AUTOROLLER

    int	i, k, m;

    char		inp[160];
    char		buf[160];

#endif


start_over:

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


    /* Clear some flags */
    use_history = prev_ready = FALSE;


    /* Extract the "maximize" and "preserve" flags */
    p_ptr->maximize = begin_maximize;
    p_ptr->preserve = begin_preserve;


#ifdef ALLOW_AUTOROLLER

    /* Clear fields */
    autoroll = 0;
    auto_round = 0;
    last_round = 0;

    /* Process help requests until a choice is made */
    while (1) {
        put_str("Use the Auto-Roller? (? for Help) ", 20, 2);
        c = inkey();
        if (strchr("ynYN", c)) break;
        if (c == 'S') goto start_over;
        if (c == '?') do_cmd_help("help.hlp");
    }


    /* Prepare the autoroller */
    if ((c == 'Y') || (c == 'y')) {

        autoroll = TRUE;

        clear_from(15);
        put_str("Enter minimum attribute for: ", 15, 2);

        /* Check the stats */
        for (i = 0; i < 6; i++) {

            /* Reset the "success" counter */
            stat_match[i] = 0;

            /* Obtain the maximal stat */
            k = cp_ptr->cadj[i] + rp_ptr->radj[i];
            m = adjust_stat(17, k, TRUE);

            /* Prepare a prompt */
            sprintf(buf, "%-12s (Max of %2d): ", stat_name[i], m);

            /* Get a minimum stat */
            while (TRUE) {

                char *s;

                /* Prompt */
                put_str(buf, 16 + i, 5);

                /* Get a response (or escape) */
                if (!askfor(inp, 8)) inp[0] = '\0';

                /* Hack -- add a fake slash */
                strcat(inp, "/");

                /* Hack -- look for the "slash" */
                s = strchr(inp, '/');

                /* Hack -- Nuke the slash */
                *s++ = '\0';

                /* Extract an input */
                k = atoi(inp) + atoi(s);

                /* Break on valid input */
                if (k <= m) break;
            }

            /* Save the minimum stat */
            stat_limit[i] = (k > 0) ? k : 0;
        }

        /* Dump results */
        Term_fresh();
    }

#endif


    /* Actually Generate */
    while (1) {

        /* Clear the old data */
        clear_from(9);

#ifdef ALLOW_AUTOROLLER

        /* Autoroller needs some feedback */
        if (autoroll) {

            /* Note when we started */
            last_round = auto_round;

            /* Re-caption the screen */
            put_character();
            put_stats();

            /* Indicate the state */
            put_str("Auto-rolling round #", 21, 2);
            put_str("Hit any key to stop.", 21, 40);
        }

        /* Otherwise just get a character */
        else {
            get_all_stats();
        }


        /* Start of AUTOROLLing loop */
        while (autoroll) {

            bool accept = TRUE;

            /* Get a new character */
            get_all_stats();

            /* Advance the round */
            auto_round++;

            /* Check and count acceptable stats */
            for (i = 0; i < 6; i++) {

                /* This stat is okay */
                if (p_ptr->use_stat[i] >= stat_limit[i]) {
                    stat_match[i]++;
                }

                /* This stat is not okay */
                else {
                    accept = FALSE;
                }
            }

#ifdef SET_UID

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

            /* Wait 1/100 second per roll */
            delay(10);

#else

            /* Dump data every round */
            birth_put_stats();
            sprintf(inp, "%9lu.", (long)auto_round);
            put_str(inp, 21, 22);

#endif

            /* Make sure they see everything */
            Term_fresh();

            /* Allow user interuption (occasionally) */
            if (!(auto_round % 10)) {
                if (Term_kbhit()) {
                    Term_flush();
                    break;
                }
            }

            /* Break if "happy" */
            if (accept) break;
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

            /* Hack -- calculate the bonuses and hitpoints */
            p_ptr->update |= (PU_BONUS | PU_HP);

            /* Handle (non-visual) stuff */
            handle_stuff(FALSE);

            /* Display the player */
            display_player();

            /* Display history instead */
            if (use_history) put_history();

            /* Prepare a prompt (must squeeze everything in) */
            Term_putstr(2, 21, -1, TERM_WHITE, "Hit ");
            if (prev_ready) Term_addstr(-1, TERM_WHITE, "'P' for Previous, ");
            else Term_addstr(-1, TERM_WHITE, "'S' to start over, ");
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

            /* Hack -- start over */
            if (c == 'S') goto start_over;

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
    prt("[Escape to continue, 'Q' to suicide, 'S' to start over]", 23, 10);

    /* Get a key */
    c = inkey();

    /* Allow Total Suicide */
    if (c == 'Q') quit(NULL);

    /* Hack -- Start over */
    if (c == 'S') goto start_over;
}


