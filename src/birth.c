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
typedef struct birther {

    s16b age;
    s16b wt;
    s16b ht;
    s16b sc;

    s32b au;

    s16b stat[6];

    char history[4][60];

} birther;



/*
 * The last character displayed
 */
static birther prev;



/*
 * Player background information
 */
typedef struct hist_type {

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
 *   Human/Dunadan -->  1 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Half-Elf      -->  4 -->  1 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Elf/High-Elf  -->  7 -->  8 -->  9 --> 54 --> 55 --> 56
 *   Hobbit        --> 10 --> 11 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Gnome         --> 13 --> 14 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Dwarf         --> 16 --> 17 --> 18 --> 57 --> 58 --> 59 --> 60 --> 61
 *   Half-Orc      --> 19 --> 20 -->  2 -->  3 --> 50 --> 51 --> 52 --> 53
 *   Half-Troll    --> 22 --> 23 --> 62 --> 63 --> 64 --> 65 --> 66
 *
 * XXX XXX XXX This table *must* be correct or drastic errors may occur!
 */
static hist_type bg[] = {

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
 * Current stats
 */
static s16b		stat_use[6];


#ifdef ALLOW_AUTOROLLER

/*
 * Use the autoroller
 */
static bool		autoroll;

/*
 * Requested minimum stats
 */
static s16b		stat_limit[6];

/*
 * Number of times each stat matched
 */
static s32b		stat_match[6];

/*
 * Current "round" in the auto-roller
 */
static s32b		auto_round;

/*
 * Last time the auto-roller stopped
 */
static s32b		last_round;

#endif




/*
 * Save the current data for later
 */
static void save_prev_data()
{
    int i;


    /*** Save the current data ***/

    /* Save the data */
    prev.age = p_ptr->age;
    prev.wt = p_ptr->wt;
    prev.ht = p_ptr->ht;
    prev.sc = p_ptr->sc;
    prev.au = p_ptr->au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
        prev.stat[i] = p_ptr->stat_max[i];
    }

    /* Save the history */
    for (i = 0; i < 4; i++) {
        strcpy(prev.history[i], history[i]);
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

    /* Save the data */
    temp.age = p_ptr->age;
    temp.wt = p_ptr->wt;
    temp.ht = p_ptr->ht;
    temp.sc = p_ptr->sc;
    temp.au = p_ptr->au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
        temp.stat[i] = p_ptr->stat_max[i];
    }

    /* Save the history */
    for (i = 0; i < 4; i++) {
        strcpy(temp.history[i], history[i]);
    }


    /*** Load the previous data ***/

    /* Load the data */
    p_ptr->age = prev.age;    
    p_ptr->wt = prev.wt;
    p_ptr->ht = prev.ht;
    p_ptr->sc = prev.sc;
    p_ptr->au = prev.au;

    /* Load the stats */
    for (i = 0; i < 6; i++) {
        p_ptr->stat_max[i] = prev.stat[i];
        p_ptr->stat_cur[i] = prev.stat[i];
    }

    /* Load the history */
    for (i = 0; i < 4; i++) {
        strcpy(history[i], prev.history[i]);
    }


    /*** Save the current data ***/

    /* Save the data */
    prev.age = temp.age;
    prev.wt = temp.wt;
    prev.ht = temp.ht;
    prev.sc = temp.sc;
    prev.au = temp.au;

    /* Save the stats */
    for (i = 0; i < 6; i++) {
        prev.stat[i] = temp.stat[i];
    }

    /* Save the history */
    for (i = 0; i < 4; i++) {
        strcpy(prev.history[i], temp.history[i]);
    }
}




/*
 * Choose the character's sex				-JWT-	
 */
static void choose_sex(void)
{
    char        c;


    put_str("m) Male", 21, 2);
    put_str("f) Female", 21, 17);

    while (1) {
        put_str("Choose a sex (? for Help, Q to Quit): ", 20, 2);
        c = inkey();
        if (c == 'Q') quit(NULL);
        if ((c == 'm') || (c == 'M')) {
            p_ptr->male = TRUE;
            c_put_str(TERM_L_BLUE, "Male", 3, 15);
            break;
        }
        else if ((c == 'f') || (c == 'F')) {
            p_ptr->male = FALSE;
            c_put_str(TERM_L_BLUE, "Female", 3, 15);
            break;
        }
        else if (c == '?') {
            do_cmd_help("help.hlp");
        }
        else {
            bell();
        }
    }

    clear_from(20);
}


/*
 * Allows player to select a race			-JWT-	
 */
static void choose_race(void)
{
    int                 j, k, l, m;

    char                c;

    char		out_val[160];

    k = 0;
    l = 2;
    m = 21;


    for (j = 0; j < MAX_RACES; j++) {
        p_ptr->prace = j;
        rp_ptr = &race_info[p_ptr->prace];
        (void)sprintf(out_val, "%c) %s", I2A(j), rp_ptr->title);
        put_str(out_val, m, l);
        l += 15;
        if (l > 70) {
            l = 2;
            m++;
        }
    }

    while (1) {
        put_str("Choose a race (? for Help, Q to Quit): ", 20, 2);
        c = inkey();
        if (c == 'Q') quit(NULL);
        j = (islower(c) ? A2I(c) : -1);
        if ((j < MAX_RACES) && (j >= 0)) {
            p_ptr->prace = j;
            rp_ptr = &race_info[p_ptr->prace];
            c_put_str(TERM_L_BLUE, rp_ptr->title, 4, 15);
            break;
        }
        else if (c == '?') {
            do_cmd_help("help.hlp");
        }
        else {
            bell();
        }
    }

    clear_from(20);
}



/*
 * Gets a character class				-JWT-	
 */
static void choose_class()
{
    int          j, k, l, m;
    int          cl[MAX_CLASS];

    char         c;

    char	 out_val[160];


    /* Prepare to list */
    k = 0;
    l = 2;
    m = 21;

    /* No legal choices yet */
    for (j = 0; j < MAX_CLASS; j++) cl[j] = 0;

    /* Display the legal choices */
    for (j = 0; j < MAX_CLASS; j++) {

        /* Verify legality */
        if (rp_ptr->choice & (1L << j)) {

            p_ptr->pclass = j;
            cp_ptr = &class_info[p_ptr->pclass];
            sprintf(out_val, "%c) %s", I2A(k), cp_ptr->title);
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
        put_str("Choose a class (? for Help, Q to Quit): ", 20, 2);
        c = inkey();
        if (c == 'Q') quit(NULL);
        j = (islower(c) ? A2I(c) : -1);
        if ((j < k) && (j >= 0)) {
            p_ptr->pclass = cl[j];
            cp_ptr = &class_info[p_ptr->pclass];
            c_put_str(TERM_L_BLUE, cp_ptr->title, 5, 15);
            break;
        }
        else if (c == '?') {
            do_cmd_help("help.hlp");
        }
        else {
            bell();
        }
    }

    /* Important -- Choose the magic info */
    mp_ptr = &magic_info[p_ptr->pclass];

    clear_from(20);
}


/*
 * Returns adjusted stat -JK-
 * Algorithm by -JWT-
 *
 * auto_roll is boolean and states maximum changes should be used rather
 * than random ones to allow specification of higher values to wait for
 *
 * The "p_ptr->maximize" code is important	-BEN-
 */
static int adjust_stat(int value, s16b amount, int auto_roll)
{
    int i;

    /* Negative amounts */
    if (amount < 0) {

        /* Apply penalty */
        for (i = 0; i < (0 - amount); i++) {
            if (value >= 18+10) {
                value -= 10;
            }
            else if (value > 18) {
                value = 18;
            }
            else if (value > 3) {
                value--;
            }
        }
    }

    /* Positive amounts */
    else if (amount > 0) {

        /* Apply reward */
        for (i = 0; i < amount; i++) {
            if (value < 18) {
                value++;
            }
            else if (p_ptr->maximize) {
                value += 10;
            }
            else if (value < 18+70) {
                value += ((auto_roll ? 15 : randint(15)) + 5);
            }
            else if (value < 18+90) {
                value += ((auto_roll ? 6 : randint(6)) + 2);
            }
            else if (value < 18+100) {
                value++;
            }
        }
    }

    /* Return the result */
    return (value);
}




/*
 * Roll for a characters stats
 *
 * For efficiency, we include a chunk of "calc_bonuses()".
 */
static void get_stats(void)
{
    int		i, j;

    int		bonus;
            
    int		dice[18];


    /* Roll and verify some stats */
    while (TRUE) {

        /* Roll some dice */
        for (j = i = 0; i < 18; i++) {

            /* Roll the dice */
            dice[i] = randint(3 + i % 3);

            /* Collect the maximum */
            j += dice[i];
        }

        /* Verify totals */
        if ((j > 42) && (j < 54)) break;
    }

    /* Acquire the stats */
    for (i = 0; i < 6; i++) {

        /* Extract 5 + 1d3 + 1d4 + 1d5 */
        j = 5 + dice[3*i] + dice[3*i+1] + dice[3*i+2];

        /* Save that value */
        p_ptr->stat_max[i] = j;

        /* Obtain a "bonus" for "race" and "class" */
        bonus = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

        /* Variable stat maxes */
        if (p_ptr->maximize) {

            /* Start fully healed */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i];

            /* Efficiency -- Apply the racial/class bonuses */
            stat_use[i] = modify_stat_value(p_ptr->stat_max[i], bonus);
        }

        /* Fixed stat maxes */
        else {

            /* Apply the bonus to the stat (somewhat randomly) */
            stat_use[i] = adjust_stat(p_ptr->stat_max[i], bonus, FALSE);

            /* Save the resulting stat maximum */
            p_ptr->stat_cur[i] = p_ptr->stat_max[i] = stat_use[i];
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
    p_ptr->expfact = rp_ptr->r_exp + cp_ptr->c_exp;

    /* Hitdice */
    p_ptr->hitdie = rp_ptr->r_mhp + cp_ptr->c_mhp;

    /* Assume base hitpoints (fully healed) */
    p_ptr->chp = p_ptr->mhp = p_ptr->hitdie;


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
            j = randint(p_ptr->hitdie);
            player_hp[i] = player_hp[i-1] + j;
        }

        /* XXX Could also require acceptable "mid-level" hitpoints */

        /* Require "valid" hitpoints at highest level */
        if (player_hp[PY_MAX_LEVEL-1] < min_value) continue;
        if (player_hp[PY_MAX_LEVEL-1] > max_value) continue;

        /* Acceptable */
        break;
    }
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

    /* Starting place */
    switch (p_ptr->prace) {
    
        case RACE_HUMAN:
        case RACE_DUNADAN:
            chart = 1;
            break;

        case RACE_HALF_ELF:
            chart = 4;
            break;
            
        case RACE_ELF:
        case RACE_HIGH_ELF:
            chart = 7;
            break;
            
        case RACE_HOBBIT:
            chart = 10;
            break;
            
        case RACE_GNOME:
            chart = 13;
            break;
            
        case RACE_DWARF:
            chart = 16;
            break;
            
        case RACE_HALF_ORC:
            chart = 19;
            break;
            
        case RACE_HALF_TROLL:
            chart = 22;
            break;
            
        default:
            chart = 0;
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
    for (s = buf; *s == ' '; s++) ;

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
        for (n = 60; ((n > 0) && (s[n-1] != ' ')); n--) ;

        /* Save next location */
        t = s + n;

        /* Wipe trailing spaces */
        while ((n > 0) && (s[n-1] == ' ')) s[--n] = '\0';

        /* Save one line of history */
        strcpy(history[i++], s);

        /* Start next line */
        for (s = t; *s == ' '; s++) ;
    }
}


/*
 * Computes character's age, height, and weight
 */
static void get_ahw(void)
{
    /* Calculate the age */
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
 * Get the player's starting money
 */
static void get_money(void)
{
    int        i, gold;

    /* Social Class determines starting gold */
    gold = (p_ptr->sc * 6) + randint(100) + 300;

    /* Process the stats */
    for (i = 0; i < 6; i++) {

        /* Mega-Hack -- reduce gold for high stats */
        if (stat_use[i] >= 18+50) gold -= 300;
        else if (stat_use[i] >= 18+20) gold -= 200;
        else if (stat_use[i] > 18) gold -= 150;
        else gold -= (stat_use[i] - 8) * 10;
    }

    /* Minimum 100 gold */
    if (gold < 100) gold = 100;

    /* She charmed the banker into it! -CJS- */
    /* She slept with the banker.. :) -GDH-  */
    if (!p_ptr->male) gold += 50;

    /* Save the gold */
    p_ptr->au = gold;
}



#ifdef ALLOW_AUTOROLLER

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
        cnv_stat(stat_use[i], buf);
        c_put_str(TERM_L_GREEN, buf, 2 + i, 66);

        /* Put the percent */
        if (stat_match[i]) {
            p = 1000L * stat_match[i] / auto_round;
            attr = (p < 100) ? TERM_YELLOW : TERM_L_GREEN;
            sprintf(buf, "%3d.%d%%", p/10, p%10);
            c_put_str(attr, buf, 2 + i, 73);
        }

        /* Never happened */
        else {
            c_put_str(TERM_RED, "(NONE)", 2 + i, 73);
        }
    }
}

#endif


/*
 * Clear all the global "character" data
 */
static void player_wipe()
{
    int i;


    /* Hack -- zero the struct */
    WIPE(p_ptr, player_type);

    /* Wipe the history */
    for (i = 0; i < 4; i++) {
        strcpy(history[i], "");
    }
    

    /* No weight */
    total_weight = 0;

    /* No items */
    inven_cnt = 0;
    equip_cnt = 0;

    /* Clear the inventory */
    for (i = 0; i < INVEN_TOTAL; i++) {
        invwipe(&inventory[i]);
    }


    /* Start with no artifacts made yet */
    for (i = 0; i < MAX_A_IDX; i++) {
        artifact_type *a_ptr = &a_info[i];
        a_ptr->cur_num = 0;
    }


    /* Start with no quests */
    for (i = 0; i < MAX_Q_IDX; i++) {
        q_list[i].level = 0;
    }

    /* Add a special quest */
    q_list[0].level = 99;

    /* Add a second quest */
    q_list[1].level = 100;


    /* Reset the "current" and "maximum" monster populations */
    for (i = 1; i < MAX_R_IDX; i++) {

        monster_race *r_ptr = &r_info[i];

        /* Hack -- Reset the counter */
        r_ptr->cur_num = 0;

        /* Hack -- Reset the max counter */
        r_ptr->max_num = 100;

        /* Hack -- Reset the max counter */
        if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 1;

        /* Clear player kills */
        r_ptr->r_pkills = 0;
    }

    /* Reset the "tried" and "aware" flags for objects */
    for (i=0; i<MAX_K_IDX; i++) {

        inven_kind *k_ptr = &k_info[i];

        /* Never tried one */
        k_ptr->tried = FALSE;

        /* Not aware of hidden effects */
        if (k_ptr->has_flavor) k_ptr->aware = FALSE;
    }


    /* Hack -- Well fed player */
    p_ptr->food = PY_FOOD_FULL - 1;


    /* Wipe the spells */
    spell_learned1 = spell_learned2 = 0L;
    spell_worked1 = spell_worked2 = 0L;
    spell_forgotten1 = spell_forgotten2 = 0L;
    for (i = 0; i < 64; i++) spell_order[i] = 99;


    /* Assume no winning game */
    total_winner = FALSE;

    /* Assume no panic save */
    panic_save = 0;

    /* Assume no cheating */
    noscore = 0;
}




/*
 * Each player starts out with a few items, given as tval/sval pairs.
 * In addition, he always has some food and a few torches.
 */

static byte player_init[MAX_CLASS][3][2] = {

    {
      /* Warrior */
      { TV_POTION, SV_POTION_BESERK_STRENGTH },
      { TV_SWORD, SV_BROAD_SWORD },
      { TV_HARD_ARMOR, SV_CHAIN_MAIL }
    },

    {
      /* Mage */
      { TV_MAGIC_BOOK, 0 },
      { TV_SWORD, SV_DAGGER },
      { TV_SCROLL, SV_SCROLL_WORD_OF_RECALL }
    },

    {
      /* Priest */
      { TV_PRAYER_BOOK, 0 },
      { TV_HAFTED, SV_MACE },
      { TV_POTION, SV_POTION_HEALING }
    },

    {
      /* Rogue */
      { TV_MAGIC_BOOK, 0 },
      { TV_SWORD, SV_SMALL_SWORD },
      { TV_SOFT_ARMOR, SV_SOFT_LEATHER_ARMOR }
    },

    {
      /* Ranger */
      { TV_MAGIC_BOOK, 0 },
      { TV_SWORD, SV_BROAD_SWORD },
      { TV_BOW, SV_LONG_BOW }
    },

    {
      /* Paladin */
      { TV_PRAYER_BOOK, 0 },
      { TV_SWORD, SV_BROAD_SWORD },
      { TV_SCROLL, SV_SCROLL_PROTECTION_FROM_EVIL }
    }
};



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


    /* Hack -- Give the player some food */
    invcopy(i_ptr, lookup_kind(TV_FOOD, SV_FOOD_RATION));
    i_ptr->number = rand_range(3,7);
    inven_aware(i_ptr);
    inven_known(i_ptr);
    (void)inven_carry(i_ptr);

    /* Hack -- Give the player some torches */
    invcopy(i_ptr, lookup_kind(TV_LITE, SV_LITE_TORCH));
    i_ptr->number = rand_range(3,7);
    i_ptr->pval = rand_range(3,7) * 500;
    inven_known(i_ptr);
    (void)inven_carry(i_ptr);

    /* Hack -- Give the player three useful objects */
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

    bool		flag;
    
    bool		prev_ready;
    bool		use_history;

    char		buf[80];


start_over:


    /* Clear old information */
    player_wipe();

    /* Clear some flags */
    use_history = FALSE;
    prev_ready = FALSE;


    /* Clear the screen */
    clear_screen();

    /* Title everything */
    put_str("Name        :", 2, 1);
    put_str("Sex         :", 3, 1);
    put_str("Race        :", 4, 1);
    put_str("Class       :", 5, 1);

    /* Dump the default name */
    c_put_str(TERM_L_BLUE, player_name, 2, 15);


    /* Choose a sex */
    choose_sex();


    /* Choose a race */
    choose_race();


    /* Choose a class */
    choose_class();


    /* Ask about "maximize" mode */
    while (1) {
        put_str("Use 'maximize' mode? (Y/N/S/Q/?) ", 20, 2);
        c = inkey();
        if (c == ESCAPE) break;
        else if (strchr("ynYN", c)) break;
        else if (c == 'S') goto start_over;
        else if (c == 'Q') quit(NULL);
        else if (c == '?') do_cmd_help("help.hlp");
        else bell();
    }

    /* XXX XXX Accept "maximize" mode */
    p_ptr->maximize = ((c == 'Y') || (c == 'y'));

    /* Clear */
    clear_from(20);


    /* Ask about "preserve" mode */
    while (1) {
        put_str("Use 'preserve' mode? (Y/N/S/Q/?) ", 20, 2);
        c = inkey();
        if (c == ESCAPE) break;
        else if (strchr("ynYN", c)) break;
        else if (c == 'S') goto start_over;
        else if (c == 'Q') quit(NULL);
        else if (c == '?') do_cmd_help("help.hlp");
        else bell();
    }

    /* XXX XXX Accept "preserve" mode */
    p_ptr->preserve = ((c == 'Y') || (c == 'y'));

    /* Clear */
    clear_from(20);


#ifdef ALLOW_AUTOROLLER

    /* Ask about "auto-roller" mode */
    while (1) {
        put_str("Use the Auto-Roller? (Y/N/S/Q/?) ", 20, 2);
        c = inkey();
        if (c == ESCAPE) break;
        else if (strchr("ynYN", c)) break;
        else if (c == 'S') goto start_over;
        else if (c == 'Q') quit(NULL);
        else if (c == '?') do_cmd_help("help.hlp");
        else bell();
    }

    /* Prepare the autoroller */
    autoroll = ((c == 'Y') || (c == 'y'));

    /* Clear */
    clear_from(20);


    /* Initialize autoroller */
    if (autoroll) {

        int mval[6];

        char inp[80];


        /* Clear fields */
        auto_round = 0L;
        last_round = 0L;

        /* Prompt for the minimum stats */
        clear_from(15);
        put_str("Enter minimum attribute for: ", 15, 2);

        /* Output the maximum stats */
        for (i = 0; i < 6; i++) {

            int k, m;

            /* Reset the "success" counter */
            stat_match[i] = 0;

            /* Race/Class bonus */
            k = rp_ptr->r_adj[i] + cp_ptr->c_adj[i];

            /* Obtain the "maximal" stat */
            m = adjust_stat(17, k, TRUE);

            /* Save the maximum */
            mval[i] = m;

            /* Extract a textual format */
            /* cnv_stat(m, inp); */

            /* Above 18 */
            if (m > 18) {
                sprintf(inp, "(Max of 18/%02d):", (m - 18));
            }

            /* From 3 to 18 */
            else {
                sprintf(inp, "(Max of %2d):", m);
            }

            /* Prepare a prompt */
            sprintf(buf, "%-5s%-20s", stat_names[i], inp);

            /* Dump the prompt */
            put_str(buf, 16 + i, 5);
        }

        /* Input the minimum stats */
        for (i = 0; i < 6; i++) {

            int v;

            /* Get a minimum stat */
            while (TRUE) {

                char *s;

                /* Move the cursor */
                put_str("", 16 + i, 30);

                /* Get a response (or escape) */
                if (!askfor(inp, 8)) inp[0] = '\0';

                /* Hack -- add a fake slash */
                strcat(inp, "/");

                /* Hack -- look for the "slash" */
                s = strchr(inp, '/');

                /* Hack -- Nuke the slash */
                *s++ = '\0';

                /* Hack -- Extract an input */
                v = atoi(inp) + atoi(s);

                /* Break on valid input */
                if (v <= mval[i]) break;
            }

            /* Save the minimum stat */
            stat_limit[i] = (v > 0) ? v : 0;
        }

        /* Dump results */
        Term_fresh();
    }

#endif


    /* Actually Generate */
    while (TRUE) {

#ifdef ALLOW_AUTOROLLER

        /* Autoroller needs some feedback */
        if (autoroll) {

            clear_screen();

            put_str("Name        :", 2, 1);
            put_str("Sex         :", 3, 1);
            put_str("Race        :", 4, 1);
            put_str("Class       :", 5, 1);

            c_put_str(TERM_L_BLUE, player_name, 2, 15);
            c_put_str(TERM_L_BLUE, (p_ptr->male ? "Male" : "Female"), 3, 15);
            c_put_str(TERM_L_BLUE, rp_ptr->title, 4, 15);
            c_put_str(TERM_L_BLUE, cp_ptr->title, 5, 15);

            /* Label stats */
            put_str("STR:", 2 + A_STR, 61);
            put_str("INT:", 2 + A_INT, 61);
            put_str("WIS:", 2 + A_WIS, 61);
            put_str("DEX:", 2 + A_DEX, 61);
            put_str("CON:", 2 + A_CON, 61);
            put_str("CHR:", 2 + A_CHR, 61);

            /* Note when we started */
            last_round = auto_round;

            /* Indicate the state */
            put_str("(Hit ESC to abort)", 11, 61);

            /* Label count */
            put_str("Round:", 9, 61);
        }

        /* Otherwise just get a character */
        else {

            /* Get a new character */
            get_stats();
        }


        /* Start of AUTOROLLing loop */
        while (autoroll) {

            bool accept = TRUE;

            /* Get a new character */
            get_stats();

            /* Advance the round */
            auto_round++;

            /* Hack -- Prevent overflow */
            if (auto_round >= 1000000L) break;

            /* Check and count acceptable stats */
            for (i = 0; i < 6; i++) {

                /* This stat is okay */
                if (stat_use[i] >= stat_limit[i]) {
                    stat_match[i]++;
                }

                /* This stat is not okay */
                else {
                    accept = FALSE;
                }
            }

            /* Break if "happy" */
            if (accept) break;

            /* Take note every 10 rolls */
            flag = (!(auto_round % 10L));
            
            /* Update display occasionally */
            if (flag || (auto_round < last_round + 100)) {

                /* Dump data */
                birth_put_stats();

                /* Dump round */
                put_str(format("%6ld", auto_round), 9, 73);

                /* Make sure they see everything */
                Term_fresh();

                /* Delay 1/10 second */
                if (flag) delay(100);

                /* Do not wait for a key */
                inkey_scan = TRUE;

                /* Check for a keypress */
                if (inkey()) break;
            }
        }

        /* Flush input */
        flush();

#else

        /* No autoroller */
        get_stats();

#endif

        /* Start with the "Misc" data */
        use_history = FALSE;

        /* Roll for base hitpoints */
        get_extra();

        /* Roll for age/height/weight */
        get_ahw();

        /* Roll for social class */
        get_history();

        /* Roll for gold */
        get_money();

        /* Input loop */
        while (TRUE) {

            /* Calculate the bonuses and hitpoints */
            p_ptr->update |= (PU_BONUS | PU_HP);

            /* Handle stuff */
            handle_stuff();

            /* Display the player */
            display_player(use_history);

            /* Prepare a prompt (must squeeze everything in) */
            Term_putstr(2, 21, -1, TERM_WHITE, "Hit ");
            if (prev_ready) Term_addstr(-1, TERM_WHITE, "'P' for Previous, ");
            else Term_addstr(-1, TERM_WHITE, "'S' to start over, ");
            if (use_history) Term_addstr(-1, TERM_WHITE, "'H' for Misc., ");
            else Term_addstr(-1, TERM_WHITE, "'H' for History, ");
            Term_addstr(-1, TERM_WHITE, "'R' to Reroll, or ESC to Accept: ");

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

            /* Hack -- help */
            if (c == '?') {
                do_cmd_help("help.hlp");
                continue;
            }

            /* Hack -- start over */
            if (c == 'S') goto start_over;

            /* Hack -- quit */
            if (c == 'Q') quit(NULL);
 
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
    prt("[ESC to continue, 'S' to start over, 'Q' to suicide]", 23, 10);

    /* Get a key */
    c = inkey();

    /* Hack -- Start over */
    if (c == 'S') goto start_over;

    /* Allow Total Suicide */
    if (c == 'Q') quit(NULL);


    /* Note player birth in the message recall */
    message_add(" ");
    message_add("  ");
    message_add("====================");
    message_add("  ");
    message_add(" ");


    /* Hack -- outfit the player */
    player_outfit();

    /* Init the stores */
    store_init();

    /* Maintain the stores (ten times) */
    for (i = 0; i < 10; i++) store_maint();
}




