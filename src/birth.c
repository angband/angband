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
 * A structure to hold "rolled" information
 */
typedef struct _birther {

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

} birther;



/*
 * The last character displayed
 */
static birther prev;




/*
 * Player background information
 */
typedef struct _hist_type {

  cptr info;			    /* Textual History			*/

  byte roll;			    /* Frequency of this entry		*/
  byte chart;			    /* Chart index			*/
  byte next;			    /* Next chart index			*/
  byte bonus;			    /* Social Class Bonus + 50		*/

} hist_type;


/*
 * Background information (see below)
 *
 * Chart progression by race:
 *   Human/Dunadan -->  1 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53 --> 0
 *   Half-Elf      -->  4 -->  1 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53 --> 0
 *   Elf/High-Elf  -->  7 -->  8 -->  9 --> 54 --> 55 --> 56 --> 0
 *   Hobbit        --> 10 --> 11 -->  3 --> 50 --> 51 --> 52 --> 53 --> 0
 *   Gnome         --> 13 --> 14 -->  3 --> 50 --> 51 --> 52 --> 53 --> 0
 *   Dwarf         --> 16 --> 17 --> 18 --> 57 --> 58 --> 59 --> 60 --> 61 --> 0
 *   Half-Orc      --> 19 --> 20 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53 --> 0
 *   Half-Troll    --> 22 --> 23 --> 62 --> 63 --> 64 --> 65 --> 66 --> 0
 */
static hist_type bg[MAX_BACKGROUND] = {

{"You are the illegitimate and unacknowledged child ",		 10, 1, 2, 25},
{"You are the illegitimate but acknowledged child ",		 20, 1, 2, 35},
{"You are one of several children ",				 95, 1, 2, 45},
{"You are the first child ",					100, 1, 2, 50},

{"of a Serf.  ",						 40, 2, 3, 65},
{"of a Yeoman.  ",						 65, 2, 3, 80},
{"of a Townsman.  ",						 80, 2, 3, 90},
{"of a Guildsman.  ",						 90, 2, 3,105},
{"of a Landed Knight.  ",					 96, 2, 3,120},
{"of a Titled Noble.  ",					 99, 2, 3,130},
{"of a Royal Blood Line.  ",					100, 2, 3,140},

{"You are the black sheep of the family.  ",			 20, 3,50, 20},
{"You are a credit to the family.  ",				 80, 3,50, 55},
{"You are a well liked child.  ",				100, 3,50, 60},

{"Your mother was of the Teleri.  ",				 40, 4, 1, 50},
{"Your father was of the Teleri.  ",				 75, 4, 1, 55},
{"Your mother was of the Noldor.  ",			 	 90, 4, 1, 55},
{"Your father was of the Noldor.  ",			 	 95, 4, 1, 60},
{"Your mother was of the Vanyar.  ",				 98, 4, 1, 65},
{"Your father was of the Vanyar.  ",				100, 4, 1, 70},

{"You are one of several children ",				 60, 7, 8, 50},
{"You are the only child ",					100, 7, 8, 55},

{"of a Teleri ",						 75, 8, 9, 50},
{"of a Noldor ",						 95, 8, 9, 55},
{"of a Vanyar ",						100, 8, 9, 60},

{"Ranger.  ",							 40, 9,54, 80},
{"Archer.  ",							 70, 9,54, 90},
{"Warrior.  ",							 87, 9,54,110},
{"Mage.  ",							 95, 9,54,125},
{"Prince.  ",							 99, 9,54,140},
{"King.  ",							100, 9,54,145},

{"You are one of several children of a Hobbit ",		 85,10,11, 45},
{"You are the only child of a Hobbit ",			        100,10,11, 55},

{"Bum.  ",							 20,11, 3, 55},
{"Tavern Owner.  ",						 30,11, 3, 80},
{"Miller.  ",							 40,11, 3, 90},
{"Home Owner.  ",						 50,11, 3,100},
{"Burglar.  ",							 80,11, 3,110},
{"Warrior.  ",							 95,11, 3,115},
{"Mage.  ",							 99,11, 3,125},
{"Clan Elder.  ",						100,11, 3,140},

{"You are one of several children of a Gnome ",			 85,13,14, 45},
{"You are the only child of a Gnome ",				100,13,14, 55},

{"Beggar.  ",							 20,14, 3, 55},
{"Braggart.  ",							 50,14, 3, 70},
{"Prankster.  ",						 75,14, 3, 85},
{"Warrior.  ",							 95,14, 3,100},
{"Mage.  ",							100,14, 3,125},

{"You are one of two children of a Dwarven ",			 25,16,17, 40},
{"You are the only child of a Dwarven ",			100,16,17, 50},

{"Thief.  ",							 10,17,18, 60},
{"Prison Guard.  ",						 25,17,18, 75},
{"Miner.  ",							 75,17,18, 90},
{"Warrior.  ",							 90,17,18,110},
{"Priest.  ",							 99,17,18,130},
{"King.  ",							100,17,18,150},

{"You are the black sheep of the family.  ",			 15,18,57, 10},
{"You are a credit to the family.  ",				 85,18,57, 50},
{"You are a well liked child.  ",				100,18,57, 55},

{"Your mother was an Orc, but it is unacknowledged.  ",		 25,19,20, 25},
{"Your father was an Orc, but it is unacknowledged.  ",		100,19,20, 25},

{"You are the adopted child ",					100,20, 2, 50},

{"Your mother was a Cave-Troll ",				 30,22,23, 20},
{"Your father was a Cave-Troll ",				 60,22,23, 25},
{"Your mother was a Hill-Troll ",				 75,22,23, 30},
{"Your father was a Hill-Troll ",				 90,22,23, 35},
{"Your mother was a Water-Troll ",				 95,22,23, 40},
{"Your father was a Water-Troll ",				100,22,23, 45},

{"Cook.  ",							  5,23,62, 60},
{"Warrior.  ",							 95,23,62, 55},
{"Shaman.  ",							 99,23,62, 65},
{"Clan Chief.  ",						100,23,62, 80},

{"You have dark brown eyes, ",					 20,50,51, 50},
{"You have brown eyes, ",					 60,50,51, 50},
{"You have hazel eyes, ",					 70,50,51, 50},
{"You have green eyes, ",					 80,50,51, 50},
{"You have blue eyes, ",					 90,50,51, 50},
{"You have blue-gray eyes, ",					100,50,51, 50},

{"straight ",							 70,51,52, 50},
{"wavy ",							 90,51,52, 50},
{"curly ",							100,51,52, 50},

{"black hair, ",						 30,52,53, 50},
{"brown hair, ",						 70,52,53, 50},
{"auburn hair, ",						 80,52,53, 50},
{"red hair, ",							 90,52,53, 50},
{"blond hair, ",						100,52,53, 50},

{"and a very dark complexion.",					 10,53, 0, 50},
{"and a dark complexion.",					 30,53, 0, 50},
{"and an average complexion.",					 80,53, 0, 50},
{"and a fair complexion.",					 90,53, 0, 50},
{"and a very fair complexion.",					100,53, 0, 50},

{"You have light grey eyes, ",					 85,54,55, 50},
{"You have light blue eyes, ",					 95,54,55, 50},
{"You have light green eyes, ",					100,54,55, 50},

{"straight ",							 75,55,56, 50},
{"wavy ",							100,55,56, 50},

{"black hair, and a fair complexion.",				 75,56, 0, 50},
{"brown hair, and a fair complexion.",				 85,56, 0, 50},
{"blond hair, and a fair complexion.",				 95,56, 0, 50},
{"silver hair, and a fair complexion.",				100,56, 0, 50},

{"You have dark brown eyes, ",					 99,57,58, 50},
{"You have glowing red eyes, ",					100,57,58, 60},

{"straight ",							 90,58,59, 50},
{"wavy ",							100,58,59, 50},

{"black hair, ",						 75,59,60, 50},
{"brown hair, ",						100,59,60, 50},

{"a one foot beard, ",						 25,60,61, 50},
{"a two foot beard, ",						 60,60,61, 51},
{"a three foot beard, ",					 90,60,61, 53},
{"a four foot beard, ",						100,60,61, 55},

{"and a dark complexion.",					100,61, 0, 50},

{"You have slime green eyes, ",					 60,62,63, 50},
{"You have puke yellow eyes, ",					 85,62,63, 50},
{"You have blue-bloodshot eyes, ",				 99,62,63, 50},
{"You have glowing red eyes, ",					100,62,63, 55},

{"dirty ",							 33,63,64, 50},
{"mangy ",							 66,63,64, 50},
{"oily ",							100,63,64, 50},

{"sea-weed green hair, ",					 33,64,65, 50},
{"bright red hair, ",						 66,64,65, 50},
{"dark purple hair, ",						100,64,65, 50},

{"and green ",							 25,65,66, 50},
{"and blue ",							 50,65,66, 50},
{"and white ",							 75,65,66, 50},
{"and black ",							100,65,66, 50},

{"ulcerous skin.",						 33,66, 0, 50},
{"scabby skin.",						 66,66, 0, 50},
{"leprous skin.",						100,66, 0, 50}

};





/*
 * Save the current data for later
 */
static void save_prev_data()
{
    int i;


    /*** Save the current data ***/
    
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
}


/*
 * Load the previous data
 */
static void load_prev_data()
{
    int        i;

    birther	temp;
    

    /*** Save the current data ***/
    
    temp.age = p_ptr->age;

    temp.wt = p_ptr->wt;
    temp.ht = p_ptr->ht;

    temp.sc = p_ptr->sc;

    temp.au = p_ptr->au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
        temp.max_stat[i] = p_ptr->max_stat[i];
        temp.cur_stat[i] = p_ptr->cur_stat[i];
        temp.mod_stat[i] = p_ptr->mod_stat[i];
        temp.use_stat[i] = p_ptr->use_stat[i];
    }

    for (i = 0; i < 4; i++) {
        (void)strncpy(temp.history[i], history[i], 60);
    }


    /*** Load the previous data ***/
    
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


    /*** Save the current data ***/
    
    prev.age = temp.age;

    prev.wt = temp.wt;
    prev.ht = temp.ht;

    prev.sc = temp.sc;

    prev.au = temp.au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
        prev.max_stat[i] = temp.max_stat[i];
        prev.cur_stat[i] = temp.cur_stat[i];
        prev.mod_stat[i] = temp.mod_stat[i];
        prev.use_stat[i] = temp.use_stat[i];
    }

    for (i = 0; i < 4; i++) {
        (void)strncpy(prev.history[i], temp.history[i], 60);
    }
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
    int                 j, k, l, m;

    char                s;

    char		out_val[160];

    k = 0;
    l = 2;
    m = 21;

    clear_from(20);

    for (j = 0; j < MAX_RACES; j++) {
        p_ptr->prace = j;
        rp_ptr = &race_info[p_ptr->prace];
        (void)sprintf(out_val, "%c) %s", j + 'a', rp_ptr->trace);
        put_str(out_val, m, l);
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

    char         s;
    
    char	 out_val[160];

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
            sprintf(out_val, "%c) %s", k + 'a', cp_ptr->title);
            put_str(out_val, m, l);
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

    /* Important -- Choose the magic info */
    mp_ptr = &magic_info[p_ptr->pclass];
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

    /* Negative amounts */
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
            else if (stat_value < 18+70) {
                stat_value += ((auto_roll ? 15 : randint(15)) + 5);
            }
            else if (stat_value < 18+90) {
                stat_value += ((auto_roll ? 6 : randint(6)) + 2);
            }
            else if (stat_value < 18+100) {
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
static void get_stats(void)
{
    int		i, j;

    int		dice[18];


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
            p_ptr->mod_stat[i] = rp_ptr->radj[i] + cp_ptr->c_adj[i];

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
            change_stat(i, cp_ptr->c_adj[i]);

            /* No bonuses */
            p_ptr->mod_stat[i] = 0;

            /* Start out fully healed */
            p_ptr->cur_stat[i] = p_ptr->max_stat[i];

            /* Use the current stats */
            p_ptr->use_stat[i] = p_ptr->max_stat[i];
        }
    }
}


/*
 * Roll for some info that the auto-roller ignores
 */
static void get_extra(void)
{
    int		i, j, min_value, max_value;


    /* Level one (never zero!) */
    p_ptr->lev = 1;

    /* Experience factor */
    p_ptr->expfact = rp_ptr->b_exp + cp_ptr->c_exp;

    /* Hitdice */
    p_ptr->hitdie = rp_ptr->bhitdie + cp_ptr->c_mhp;


    /* Minimum hitpoints at highest level */
    min_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 3) / 8;
    min_value += PY_MAX_LEVEL;

    /* Maximum hitpoints at highest level */
    max_value = (PY_MAX_LEVEL * (p_ptr->hitdie - 1) * 5) / 8;
    max_value += PY_MAX_LEVEL;

    /* Pre-calculate level 1 hitdice */
    player_hp[0] = p_ptr->hitdie;

    /* Roll out the hitpoints */
    while (TRUE) {

        /* Roll the hitpoint values */
        for (i = 1; i < PY_MAX_LEVEL; i++) {
            j = randint((int)p_ptr->hitdie);
            player_hp[i] = player_hp[i-1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (player_hp[PY_MAX_LEVEL-1] < min_value) continue;
        if (player_hp[PY_MAX_LEVEL-1] > max_value) continue;

        /* Acceptable */
        break;
    }


    /* Hack -- Prevent crash in "calc_hitpoints()" */
    p_ptr->mhp = 1;
    
    /* Hack -- calculate the bonuses and hitpoints */
    p_ptr->update |= (PU_BONUS | PU_HP);

    /* Handle stuff */
    handle_stuff();


    /* Start out fully healed */
    p_ptr->chp = p_ptr->mhp;
    p_ptr->chp_frac = 0;
}


/*
 * Get the racial history, and social class, using the "history charts".
 */
static void get_history(void)
{
    int		i, n, chart, roll, social_class;

    char	*s, *t;
    
    char	buf[240];



    /* Clear the previous history strings */
    for (i = 0; i < 4; i++) history[i][0] = '\0';


    /* Clear the history text */
    buf[0] = '\0';

    /* Initial social class */
    social_class = randint(4);

    /* Dunadan -- Same as Human */
    if (p_ptr->prace == 8) {
        chart = 0 * 3 + 1;
    }

    /* High Elf -- Same as Elf */
    else if (p_ptr->prace == 9) {
        chart = 2 * 3 + 1;
    }

    /* Normal races -- Start at given chart */
    else {
        chart = p_ptr->prace * 3 + 1;
    }


    /* Process the history */
    while (chart) {

        /* Start over */
        i = 0;
        
        /* Roll for nobility */            
        roll = randint(100);

        /* Access the proper entry in the table */
        while ((chart != bg[i].chart) || (roll > bg[i].roll)) i++;

        /* Acquire the textual history */
        (void)strcat(buf, bg[i].info);

        /* Add in the social class */
        social_class += (int)(bg[i].bonus) - 50;

        /* Enter the next chart */
        chart = bg[i].next;
    }



    /* Verify social class */
    if (social_class > 100) social_class = 100;
    else if (social_class < 1) social_class = 1;

    /* Save the social class */
    p_ptr->sc = social_class;


    /* Skip leading spaces */
    for (s = buf; *s == ' '; s++);
    
    /* Get apparent length */
    n = strlen(s);

    /* Kill trailing spaces */
    while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';


    /* Start at first line */
    i = 0;

    /* Collect the history */
    while (TRUE) {

        /* Extract remaining length */
        n = strlen(s);

        /* All done */
        if (n < 60) {

            /* Save one line of history */
            strcpy(history[i++], s);

            /* All done */
            break;
        }

        /* Find a reasonable break-point */
        for (n = 60; ((n > 0) && (s[n-1] != ' ')); n--);

        /* Save next location */
        t = s + n;

        /* Wipe trailing spaces */
        while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';

        /* Save one line of history */
        strcpy(history[i++], s);

        /* Start next line */
        for (s = t; *s == ' '; s++);
    }
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
 * Hack -- Given an "actual stat value", return a monetary value,
 * which affects the amount of gold a player has (see below).
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

        /* Never tried one */
        x_list[i].tried = FALSE;

        /* Not aware of hidden effects */
        if (x_list[i].has_flavor) x_list[i].aware = FALSE;
    }


    /* Well fed player */
    p_ptr->food = PY_FOOD_FULL - 1;


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
}



/*
 * Init players with some belongings
 *
 * Having an item makes the player "aware" of its purpose.
 */
static void player_outfit()
{
    int		i, tv, sv;
    inven_type	inven_init;
    inven_type	*i_ptr = &inven_init;


    /* Give the player some food */
    invcopy(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
    i_ptr->number = rand_range(3,7);
    inven_aware(i_ptr);
    inven_known(i_ptr);
    (void)inven_carry(i_ptr);

    /* Give the player some torches */
    invcopy(i_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
    i_ptr->number = rand_range(3,7);
    i_ptr->pval = rand_range(3,7) * 500;
    inven_known(i_ptr);
    (void)inven_carry(i_ptr);

    /* Give the player three useful objects */
    for (i = 0; i < 3; i++) {
        tv = player_init[p_ptr->pclass][i][0];
        sv = player_init[p_ptr->pclass][i][1];
        invcopy(i_ptr, lookup_kind(tv, sv));
        inven_aware(i_ptr);
        inven_known(i_ptr);
        (void)inven_carry(i_ptr);
    }
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
    int			i;
    
    char		c;

    bool		prev_ready;
    bool		use_history;

#ifdef ALLOW_AUTOROLLER

    int			k, m;

    char		inp[80];
    char		buf[80];

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
        put_str("Use the Auto-Roller? (Y/N/S/?) ", 20, 2);
        c = inkey();
        if (c == ESCAPE) break;
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
            k = rp_ptr->radj[i] + cp_ptr->c_adj[i];
            m = adjust_stat(17, k, TRUE);

            /* Extract a textual format */
            cnv_stat(m, inp);
            
            /* Above 18 */
            if (m > 18) {
                sprintf(inp, "(Max of 18/%02d):", (m - 18));
            }
            
            /* From 3 to 18 */
            else {
                sprintf(inp, "(Max of %2d):", m);
            }

            /* Prepare a prompt */
            sprintf(buf, "%-15s%-20s", stat_name[i], inp);

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
            clear_screen();
            put_character();
            put_stats();

            /* Indicate the state */
            put_str("Auto-rolling round #", 21, 2);
            put_str("Hit any key to stop.", 21, 40);

            /* Describe the percentages */
            put_str("The percentages shown above are not independent!", 23, 2);
        }

        /* Otherwise just get a character */
        else {
            get_stats();
        }


        /* Start of AUTOROLLing loop */
        while (autoroll) {

            bool accept = TRUE;

            /* Get a new character */
            get_stats();

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

                /* Do not wait for a key */
                inkey_scan = TRUE;

                /* Check for a keypress */
                if (inkey()) {

                    /* Flush all keys */
                    flush();

                    /* Stop rolling */
                    break;
                }
            }

            /* Break if "happy" */
            if (accept) break;
        }

#else

        /* No autoroller */
        get_stats();

#endif

        /* Roll for some stuff */
        get_extra();
        get_history();
        get_ahw();
        get_money();


        /* Start with the "Misc" data */
        use_history = FALSE;

        /* Input loop */
        while (1) {

            /* Hack -- calculate the bonuses and hitpoints */
            p_ptr->update |= (PU_BONUS | PU_HP);

            /* Handle stuff */
            handle_stuff();

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

        /* Note that a previous roll exists */
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

    
    /* Hack -- outfit the player */
    player_outfit();

    /* Init the stores */
    store_init();

    /* Maintain the stores (ten times) */
    for (i = 0; i < 10; i++) store_maint();
}




