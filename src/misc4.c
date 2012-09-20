/* File: misc4.c */ 

/* Purpose: misc dungeon/player code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke 
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies. 
 */

#include "angband.h"



#define COL_STAT 0

#define ROW_RACE    1
#define ROW_CLASS   2
#define ROW_TITLE   3

#define ROW_LEVEL   (new_screen_layout ? 4 : 12)
#define ROW_EXP     (new_screen_layout ? 5 : 13)

#define ROW_STAT    (new_screen_layout ? 7 : 5)

#define ROW_AC      (new_screen_layout ? 14 : 17)
#define ROW_CURHP   (new_screen_layout ? 15 : 16)
#define ROW_MAXHP   (new_screen_layout ? 16 : 15)
#define ROW_MANA    (new_screen_layout ? 17 : 14)
#define ROW_GOLD    18

#define ROW_WINNER  (new_screen_layout ? 19 : 20)
#define ROW_EQUIPPY (new_screen_layout ? 20 : 4)
#define ROW_CUT     21
#define ROW_STUN    22


static cptr stat_names[] = {
    "STR: ", "INT: ", "WIS: ", "DEX: ", "CON: ", "CHR: "
};

static cptr stat_names_reduced[] = {
    "Str: ", "Int: ", "Wis: ", "Dex: ", "Con: ", "Chr: "
};




/*
 * Print character info in given row, column   -RAK-
 * the longest title is 13 characters, so only pad to 13
 * XXX There has got to be a cleaner way
 */
void prt_field(cptr info, int row, int col)
{
    char tmp[16];
    sprintf(tmp, "%-13.13s", info);
    c_put_str(TERM_L_BLUE, tmp, row, col);
}




/*
 * Converts stat num into a string (six chars plus a space)
 */
void cnv_stat(int my_stat, char *out_val)
{
    register u16b stat = my_stat;
    register int    part1, part2;

    if (stat > 18) {
	part1 = 18;
	part2 = stat - 18;
	if (part2 >= 220)
	    (void)sprintf(out_val, "%2d/*** ", part1);
	else if (part2 >= 100)
	    (void)sprintf(out_val, "%2d/%03d ", part1, part2);
	else
	    (void)sprintf(out_val, " %2d/%02d ", part1, part2);
    }
    else {
	(void)sprintf(out_val, "%6d ", stat);
    }
}


/*
 * Print character stat in given row, column
 */
void prt_stat(int stat)
{
    char tmp[32];

    /* Display "injured" stat */
    if (p_ptr->cur_stat[stat] < p_ptr->max_stat[stat]) {
	put_str(stat_names_reduced[stat], ROW_STAT + stat, COL_STAT);
	cnv_stat(p_ptr->use_stat[stat], tmp);
	c_put_str(TERM_YELLOW, tmp, ROW_STAT + stat, COL_STAT + 6);
    }

    /* Display "healthy" stat */
    else {
	put_str(stat_names[stat], ROW_STAT + stat, COL_STAT);
	cnv_stat(p_ptr->use_stat[stat], tmp);
	c_put_str(TERM_L_GREEN, tmp, ROW_STAT + stat, COL_STAT + 6);
    }
}




/*
 * Adjustment for wisdom/intelligence
 */
int stat_adj(int stat)
{
    register int value;

    value = p_ptr->use_stat[stat];
    if (value > 228) return (20);
    if (value > 218) return (18);
    if (value > 198) return (16);
    if (value > 188) return (15);
    if (value > 178) return (14);
    if (value > 168) return (13);
    if (value > 158) return (12);
    if (value > 148) return (11);
    if (value > 138) return (10);
    if (value > 128) return (9);
    if (value > 118) return (8);
    if (value > 117) return (7);
    if (value > 107) return (6);
    if (value > 87) return (5);
    if (value > 67) return (4);
    if (value > 17) return (3);
    if (value > 14) return (2);
    if (value > 7) return (1);
    return (0);
}


/*
 * Adjustment for charisma
 * Percent decrease or increase in price of goods		
 */
int chr_adj()
{
    register int charisma;

    charisma = p_ptr->use_stat[A_CHR];

    if (charisma > 217) return (80);
    if (charisma > 187) return (86);
    if (charisma > 147) return (88);
    if (charisma > 117) return (90);
    if (charisma > 107) return (92);
    if (charisma > 87) return (94);
    if (charisma > 67) return (96);
    if (charisma > 18) return (98);

    switch (charisma) {
	case 18:
	    return (100);
	case 17:
	    return (101);
	case 16:
	    return (102);
	case 15:
	    return (103);
	case 14:
	    return (104);
	case 13:
	    return (106);
	case 12:
	    return (108);
	case 11:
	    return (110);
	case 10:
	    return (112);
	case 9:
	    return (114);
	case 8:
	    return (116);
	case 7:
	    return (118);
	case 6:
	    return (120);
	case 5:
	    return (122);
	case 4:
	    return (125);
	case 3:
	    return (130);
	default:
	    return (140);
      }
}


/*
 * Returns a character's adjustment to hit points
 */
int con_adj()
{
    register int con;

    con = p_ptr->use_stat[A_CON];

    if (con < 7) return (con - 7);
    if (con < 17) return (0);
    if (con < 18) return (1);
    if (con < 94) return (2);
    if (con < 117) return (3);
    if (con < 119) return (4);
    if (con < 128) return (5);
    if (con < 138) return (6);
    if (con < 158) return (7);
    if (con < 168) return (8);
    if (con < 178) return (9);
    if (con < 188) return (10);
    if (con < 198) return (11);
    if (con < 208) return (12);
    if (con < 228) return (13);
    return (14);
}


/*
 * Determine a "title" for the player
 */
cptr title_string()
{
    cptr p;

    if (p_ptr->lev < 1) {
	p = "Babe in arms";
    }
    else if (p_ptr->lev <= MAX_PLAYER_LEVEL) {
	p = player_title[p_ptr->pclass][p_ptr->lev - 1];
    }
    else if (p_ptr->male) {
	p = "**KING**";
    }
    else {
	p = "**QUEEN**";
    }

    return p;
}


/*
 * Prints title of character				-RAK-	
 */
void prt_title()
{
    prt_field(title_string(), ROW_TITLE, COL_STAT);
}


/*
 * Prints level
 */
void prt_level()
{
    char tmp[32];
    sprintf(tmp, "%6d", p_ptr->lev);
    c_put_str(TERM_L_GREEN, tmp, ROW_LEVEL, COL_STAT + 6);
}

/*
 * Display the experience
 */
static void prt_exp()
{
    char out_val[32];

    (void) sprintf(out_val, "%8ld", (long)p_ptr->exp);

    if (p_ptr->exp == p_ptr->max_exp) {
	c_put_str(TERM_L_GREEN, out_val, ROW_EXP, COL_STAT+4);
    }
    else {
	c_put_str(TERM_YELLOW, out_val, ROW_EXP, COL_STAT+4);
    }
}


/*
 * Prints players current mana points.
 */
void prt_cmana()
{
    char tmp[32];
    byte color;

    sprintf(tmp, "%6d", p_ptr->cmana);

    if (p_ptr->cmana >= p_ptr->mana) {
	color = TERM_L_GREEN;
    }
    else if (p_ptr->cmana > (p_ptr->mana * hitpoint_warn) / 10) {
	color = TERM_YELLOW;
    }
    else {
	color = TERM_RED;
    }

    if (p_ptr->pclass != 0) {
	c_put_str(color, tmp, ROW_MANA, COL_STAT + 6);
    }
}


/*
 * Prints Max hit points
 */
void prt_mhp()
{
    char tmp[32];
    sprintf(tmp, "%6d", p_ptr->mhp);
    c_put_str(TERM_L_GREEN, tmp, ROW_MAXHP, COL_STAT + 6);
}


/*
 * Prints players current hit points
 */
void prt_chp()
{
    char tmp[32];
    byte color;
    sprintf(tmp, "%6d", p_ptr->chp);

    if (p_ptr->chp >= p_ptr->mhp) {
	color = TERM_L_GREEN;
    }
    else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10) {
	color = TERM_YELLOW;
    }
    else {
	color = TERM_RED;
    }

    c_put_str(color, tmp, ROW_CURHP, COL_STAT + 6);
}


/*
 * prints current AC
 */
void prt_pac()
{
    char tmp[32];
    sprintf(tmp, "%6d", p_ptr->dis_ac);
    c_put_str(TERM_L_GREEN, tmp, ROW_AC, COL_STAT + 6);
}


/*
 * Prints current gold
 */
void prt_gold()
{
    char tmp[32];

    sprintf(tmp, "%9ld", (long)p_ptr->au);
    c_put_str(TERM_L_GREEN, tmp, ROW_GOLD, COL_STAT + 3);
}


/*
 * Prints depth in stat area				-RAK-	 
 */
void prt_depth()
{
    char depths[32];

    if (!dun_level) {
	(void)strcpy(depths, "Town");
    }
    else if (depth_in_feet) {
	(void)sprintf(depths, "%d ft", dun_level * 50);
    }
    else {
	(void)sprintf(depths, "Lev %d", dun_level);
    }

    /* Right-Adjust the "depth", but clear old values */
    prt(format("%7s", depths), 23, 70);
}


/*
 * Prints status of hunger				-RAK-	 
 */
void prt_hunger()
{
    if (PY_WEAK & p_ptr->status) {
	c_put_str(TERM_ORANGE, "Weak  ", 23, 0);
    }
    else if (PY_HUNGRY & p_ptr->status) {
	c_put_str(TERM_YELLOW, "Hungry", 23, 0);
    }
    else {
	put_str("      ", 23, 0);
    }
}


/*
 * Prints Blind status					-RAK-	 
 */
void prt_blind(void)
{
    if (PY_BLIND & p_ptr->status) {
	c_put_str(TERM_ORANGE, "Blind", 23, 7);
    }
    else {
	put_str("     ", 23, 7);
    }
}


/*
 * Prints Confusion status				-RAK-	 
 */
void prt_confused(void)
{
    if (PY_CONFUSED & p_ptr->status) {
	c_put_str(TERM_ORANGE, "Confused", 23, 13);
    }
    else {
	put_str("        ", 23, 13);
    }
}


/*
 * Prints Fear status					-RAK-	 
 */
void prt_afraid()
{
    if (PY_FEAR & p_ptr->status) {
	c_put_str(TERM_ORANGE, "Afraid", 23, 22);
    }
    else {
	put_str("      ", 23, 22);
    }
}


/*
 * Prints Poisoned status				-RAK-	 
 */
void prt_poisoned(void)
{
    if (PY_POISONED & p_ptr->status) {
	c_put_str(TERM_L_GREEN, "Poisoned", 23, 29);
    }
    else {
	put_str("        ", 23, 29);
    }
}


/*
 * Prints Searching, Resting, Paralysis, or 'count' status	-RAK-
 * Display is always exactly 10 characters wide (see below)
 */
void prt_state(void)
{
    /* Default to white */
    byte attr = TERM_WHITE;

    /* Default to ten spaces */
    char text[16] = "          ";

    /* Turn off the flag */
    p_ptr->status &= ~PY_REPEAT;

    /* Most important info is paralyzation */
    if (p_ptr->paralysis > 1) {
	attr = TERM_RED;
	strcpy(text, "Paralyzed!");
    }

    /* Then comes resting */
    else if (PY_REST & p_ptr->status) {
	if (p_ptr->rest > 999) {
	    (void)sprintf(text, "Rest %3d00", p_ptr->rest / 100);
	}
	else if (p_ptr->rest > 0) {
	    (void)sprintf(text, "Rest   %3d", p_ptr->rest);
	}
	else if (p_ptr->rest == -1) {
	    (void)sprintf(text, "Rest *****");
	}
	else if (p_ptr->rest == -2) {
	    (void)sprintf(text, "Rest &&&&&");
	}
    }

    /* Then comes repeating */
    else if (command_rep > 0) {

	/* Hack -- we need to redraw this */
	p_ptr->status |= PY_REPEAT;

	/* Hack -- ignore searching info */
	/* if (PY_SEARCH & p_ptr->status) ... */

	if (command_rep > 999) {
	    (void)sprintf(text, "Rep. %3d00", command_rep / 100);
	}
	else {
	    (void)sprintf(text, "Repeat %3d", command_rep);
	}
    }

    else if (PY_SEARCH & p_ptr->status) {
	strcpy(text, "Searching ");
    }

    /* Display the info (or blanks) */
    c_put_str(attr, text, 23, 38);
}


/*
 * Prints the speed of a character.			-CJS-
 */
void prt_speed()
{
    int i = p_ptr->pspeed;
    
    int attr = TERM_WHITE;
    char buf[32] = "";

    /* Visually "undo" the Search Mode Slowdown */
    if (PY_SEARCH & p_ptr->status) i += 10;

    /* Fast */
    if (i > 110) {
	attr = TERM_L_GREEN;
	sprintf(buf, "Fast (+%d)", (i - 110));
    }

    /* Slow */
    else if (i < 110) {
	attr = TERM_L_UMBER;
	sprintf(buf, "Slow (-%d)", (110 - i));
    }

    /* Display the speed */
    c_put_str(attr, format("%-14s", buf), 23, 49);
}


void prt_study()
{
    p_ptr->status &= ~PY_STUDY;

    if (p_ptr->new_spells != 0) {
	put_str("Study", 23, 64);
    }
    else {
	put_str("     ", 23, 64);
    }
}


void prt_cut()
{
    int c = p_ptr->cut;

    if (c > 900)
	c_put_str(TERM_L_RED,"Mortal wound", ROW_CUT, 0);
    else if (c > 300)
	c_put_str(TERM_RED,"Deep gash   ", ROW_CUT, 0);
    else if (c > 200)
	c_put_str(TERM_RED,"Severe cut  ", ROW_CUT, 0);
    else if (c > 45)
	c_put_str(TERM_ORANGE,"Nasty cut   ", ROW_CUT, 0);
    else if (c > 15)
	c_put_str(TERM_ORANGE,"Bad cut     ", ROW_CUT, 0);
    else if (c > 5)
	c_put_str(TERM_YELLOW,"Light cut   ", ROW_CUT, 0);
    else if (c > 0)
	c_put_str(TERM_YELLOW,"Graze       ", ROW_CUT, 0);
    else
	put_str("            ", ROW_CUT, 0);
}



void prt_stun(void)
{
    int s = p_ptr->stun;

    if (!p_ptr->resist_sound) {
	if (s > 100) {
	    c_put_str(TERM_RED, "Knocked out ", ROW_STUN, 0);
	}
	else if (s > 50) {
	    c_put_str(TERM_ORANGE, "Heavy stun  ", ROW_STUN, 0);
	}
	else if (s > 0) {
	    c_put_str(TERM_ORANGE, "Stun        ", ROW_STUN, 0);
	}
	else {
	    put_str("            ", ROW_STUN, 0);
	}
    }
}


/*
 * Prints winner/wizard status on display			-RAK-	
 */
void prt_winner(void)
{
    if (wizard) {
	put_str("Wizard", ROW_WINNER, 0);
    }
    else if (total_winner) {
	put_str("Winner", ROW_WINNER, 0);
    }
    else {
	put_str("       ", ROW_WINNER, 0);
    }
}



/*
 * Display the "equippy chars"
 */
void prt_equippy_chars(void)
{                                        
    int i, j;                              
    inven_type *i_ptr;                     

    byte attr;
    char out_val[2];


    /* Wipe the equippy chars */
    put_str("            ", ROW_EQUIPPY, 0);

    /* Hack -- skip if requested */
    if (!equippy_chars) return;
    

    /* Analyze the pack */
    for (j = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++, j++) {

	/* Get the item */
	i_ptr = &inventory[i];                           

	/* Skip empty slots */
	if (!i_ptr->tval) continue;
	
	/* Extract the color */
	attr = inven_attr(i_ptr);                

	/* Extract the symbol */
	out_val[0] = inven_char(i_ptr);                

	/* Hack -- terminate the string */
	out_val[1] = '\0';

	/* Display the item symbol */
	c_put_str(attr, out_val, ROW_EQUIPPY, j);
    }
}



/*
 * Prints character-screen info				-RAK-	 
 */
void prt_stat_block()
{
    register int          i;

    prt_field(race[p_ptr->prace].trace, ROW_RACE, COL_STAT);
    prt_field(class[p_ptr->pclass].title, ROW_CLASS, COL_STAT);

    prt_field(title_string(), ROW_TITLE, COL_STAT);

    prt_equippy_chars();

    for (i = 0; i < 6; i++) prt_stat(i);

    put_str("Level       ", ROW_LEVEL, COL_STAT);
    prt_level();

    put_str("Exp.        ", ROW_EXP, COL_STAT);
    prt_exp();

    if (p_ptr->pclass != 0) {
	put_str("Mana        ", ROW_MANA, COL_STAT);
	prt_cmana();
    }

    put_str("Max HP      ", ROW_MAXHP, COL_STAT);
    prt_mhp();

    put_str("Cur HP      ", ROW_CURHP, COL_STAT);
    prt_chp();

    put_str("AC          ", ROW_AC, COL_STAT);
    prt_pac();

    put_str("AU          ", ROW_GOLD, COL_STAT);
    prt_gold();

    prt_winner();
    prt_cut();
    prt_stun();
    prt_study();

    if ((PY_HUNGRY | PY_WEAK) & p_ptr->status)
	prt_hunger();
    if (PY_BLIND & p_ptr->status)
	prt_blind();
    if (PY_CONFUSED & p_ptr->status)
	prt_confused();
    if (PY_FEAR & p_ptr->status)
	prt_afraid();
    if (PY_POISONED & p_ptr->status)
	prt_poisoned();
    if ((PY_SEARCH | PY_REST) & p_ptr->status)
	prt_state();

    /* Hack -- always print speed */
    prt_speed();
}


/*
 * Draws entire screen					-RAK-	
 */
void draw_cave(void)
{
    /* Forget everything */
    forget_lite();
    forget_view();
    
    /* Redraw everything */
    clear_screen();
    prt_stat_block();
    prt_map();
    prt_depth();

    /* Verify */    
    update_view();
    update_lite();
    update_monsters();
}




/*
 * Cut the player
 */
void cut_player(int c)
{
    c += p_ptr->cut;
    p_ptr->cut = c;

    if (c > 5000) {
	msg_print("You have been given a mortal wound.");
    }
    else if (c > 900) {
	msg_print("You have been given a deep gash.");
    }
    else if (c > 200) {
	msg_print("You have been given a severe cut.");
    }
    else if (c > 100) {
	msg_print("You have been given a nasty cut.");
    }
    else if (c > 50) {
	msg_print("You have been given a bad cut.");
    }
    else if (c > 10) {
	msg_print("You have been given a light cut.");
    }
    else if (c > 0) {
	msg_print("You have been given a graze.");
    }
}


/*
 * Stun the player
 */
void stun_player(int s)
{
    if (p_ptr->resist_sound) return;
    
    s += p_ptr->stun;
    p_ptr->stun = s;

    if (s > 100) {
	msg_print("You have been knocked out.");
    }
    else if (s > 50) {
	msg_print("You've been heavily stunned.");
    }
    else if (s > 0) {
	msg_print("You've been stunned.");
    }

    /* Hack -- recalculate the bonuses */
    calc_bonuses();
}


/*
 * Modify a stat by a "modifier", return new value
 * Stats go: 3,4,...,17,18,18/10,18/20,...
 */
int modify_stat(int stat, int amount)
{
    register int    loop, i, tmp_stat;

    tmp_stat = p_ptr->cur_stat[stat];

    loop = ABS(amount);

    for (i = 0; i < loop; i++) {
	if (amount > 0) {
	    if (tmp_stat < 18) tmp_stat++;
	    else tmp_stat += 10;
	}
	else {
	    if (tmp_stat > 27) tmp_stat -= 10;
	    else if (tmp_stat > 18) tmp_stat = 18;
	    else if (tmp_stat > 3) tmp_stat--;
	}
    }

    return tmp_stat;
}


/*
 * Modify the "max" stat by a "modifier", return new value
 * Stats go: 3,4,...,17,18,18/10,18/20,...
 */
static int modify_max_stat(int stat, int amount)
{
    register int    loop, i;
    register u16b tmp_stat;

    tmp_stat = p_ptr->max_stat[stat];

    loop = ABS(amount);

    for (i = 0; i < loop; i++) {
	if (amount > 0) {
	    if (tmp_stat < 18) tmp_stat++;
	    else tmp_stat += 10;
	}
	else {
	    if (tmp_stat > 27) tmp_stat -= 10;
	    else if (tmp_stat > 18) tmp_stat = 18;
	    else if (tmp_stat > 3) tmp_stat--;
	}
    }

    return tmp_stat;
}



/*
 * Set the value of the stat which is actually used.	 -CJS-
 * Also, make sure to recalculate any changed values.
 */
void set_use_stat(int stat)
{
    /* Calculate the "total" stat value */
    p_ptr->use_stat[stat] = modify_stat(stat, p_ptr->mod_stat[stat]);

    /* Calculate various effects */
    if (stat == A_STR) {
	calc_bonuses();
    }
    else if (stat == A_DEX) {
	calc_bonuses();
    }
    else if (stat == A_CON) {
	calc_hitpoints();
    }
    else if (stat == A_INT && class[p_ptr->pclass].spell == MAGE) {
	calc_spells(A_INT);
	calc_mana(A_INT);
    }
    else if (stat == A_WIS && class[p_ptr->pclass].spell == PRIEST) {
	calc_spells(A_WIS);
	calc_mana(A_WIS);
    }
}


/*
 * Increases a stat by one randomized level		-RAK-	 
 */
int inc_stat(int stat)
{
    register int tmp_stat, gain;

    /* Hmmm -- First, restore the stat */
    res_stat(stat);

    /* Then augment the current/max stat */
    tmp_stat = p_ptr->cur_stat[stat];

    /* Cannot go above 18/100 */
    if (tmp_stat < 118) {
	if (tmp_stat < 18) {	   
	    gain = randint(2);
	    tmp_stat += gain;
	}
	else if (tmp_stat < 116) {
	    /* stat increases by 1/6 to 1/3 of difference from max */
	    gain = ((118 - tmp_stat) / 2 + 3) >> 1;
	    tmp_stat += randint(gain) + gain / 2;
	    if (tmp_stat > 117) tmp_stat = 117;
	}
	else {
	    tmp_stat++;
	}

	p_ptr->cur_stat[stat] = tmp_stat;
	if (tmp_stat > p_ptr->max_stat[stat]) {
	    p_ptr->max_stat[stat] = tmp_stat;
	}
	set_use_stat(stat);
	prt_stat(stat);
	return TRUE;
    }

    return FALSE;
}



/* 
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Amount could be a little higher in extreme cases to mangle very high
 * stats from massive assaults.  -CWS
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm.
 */
int dec_stat(int stat, int amount, int permanent)
{
    int cur, max, loss, same, res = FALSE;


    /* Acquire current value */
    cur = p_ptr->cur_stat[stat];
    max = p_ptr->max_stat[stat];

    /* Note when the values are identical */
    same = (cur == max);
    
    /* Damage "current" value */
    if (cur > 3) {

	/* Handle "low" values */
	if (cur <= 18) {

	    if (amount > 90) cur--;
	    if (amount > 50) cur--;
	    if (amount > 20) cur--;
	    cur--;
	}

	/* Handle "high" values */
	else {

	    /* Hack -- Decrement by a random amount between one-quarter */
	    /* and one-half of the stat bonus times the percentage, with a */
	    /* minimum damage of half the percentage. -CWS */
	    loss = (((cur-18) / 2 + 1) / 2 + 1);
	    loss = ((randint(loss) + loss) * amount) / 100;
	    if (loss < amount/2) loss = amount/2;

	    /* Lose some points */
	    cur = cur - loss;

	    /* Hack -- Only reduce stat to 17 sometimes */
	    if (cur < 18) cur = (amount <= 20) ? 18 : 17;
	}

	/* Prevent illegal values */
	if (cur < 3) cur = 3;

	/* Something happened */
	if (cur != p_ptr->cur_stat[stat]) res = TRUE;
    }
    
    /* Damage "max" value */
    if (permanent && (max > 3)) {

	/* Handle "low" values */
	if (max <= 18) {

	    if (amount > 90) max--;
	    if (amount > 50) max--;
	    if (amount > 20) max--;
	    max--;
	}

	/* Handle "high" values */
	else {

	    /* Hack -- Decrement by a random amount between one-quarter */
	    /* and one-half of the stat bonus times the percentage, with a */
	    /* minimum damage of half the percentage. -CWS */
	    loss = (((max-18) / 2 + 1) / 2 + 1);
	    loss = ((randint(loss) + loss) * amount) / 100;
	    if (loss < amount/2) loss = amount/2;

	    /* Lose some points */
	    max = max - loss;

	    /* Hack -- Only reduce stat to 17 sometimes */
	    if (max < 18) max = (amount <= 20) ? 18 : 17;
	}

	/* Hack -- keep it clean */
	if (same || (max < cur)) max = cur;

	/* Something happened */
	if (max != p_ptr->max_stat[stat]) res = TRUE;
    }

    /* Apply changes */
    if (res) {
    
	/* Actually set the stat to its new value. */
	p_ptr->cur_stat[stat] = cur;
	p_ptr->max_stat[stat] = max;

	/* Update the stat */
	set_use_stat(stat);
	prt_stat(stat);
    }

    /* Done */
    return (res);
}


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference. 
 */
int res_stat(int stat)
{
    /* Restore */
    if (p_ptr->cur_stat[stat] != p_ptr->max_stat[stat]) {
	p_ptr->cur_stat[stat] = p_ptr->max_stat[stat];
	set_use_stat(stat);
	prt_stat(stat);
	return TRUE;
    }

    /* Nothing to restore */
    return FALSE;
}


/*
 * Returns a character's adjustment to hit.		 -JWT-	 
 */
int tohit_adj()
{
    register int total, stat;

    stat = p_ptr->use_stat[A_DEX];
    if      (stat <   4)  total = -3;
    else if (stat <   6)  total = -2;
    else if (stat <   8)  total = -1;
    else if (stat <  16)  total =  0;
    else if (stat <  17)  total =  1;
    else if (stat <  18)  total =  2;
    else if (stat <  69)  total =  3;
    else if (stat < 108)  total =  4; /* 18/51 to 18/89 -CFT */
    else if (stat < 118)  total =  5; /* 18/90 to 18/99 -CFT */
    else if (stat < 128)  total =  6; /* 18/100 to 18/109 -CFT */
    else if (stat < 138)  total =  7;
    else if (stat < 148)  total =  8;
    else if (stat < 158)  total =  9;
    else if (stat < 168)  total = 10;
    else if (stat < 178)  total = 11;
    else if (stat < 188)  total = 12;
    else if (stat < 198)  total = 13;
    else if (stat < 218)  total = 14;
    else if (stat < 228)  total = 15;
    else total = 17;
    stat = p_ptr->use_stat[A_STR];
    if      (stat <   4)  total -= 3;
    else if (stat <   5)  total -= 2;
    else if (stat <   7)  total -= 1;
    else if (stat <  18)  total -= 0;
    else if (stat <  88)  total += 1; /* 18 to 18/69 -CFT */
    else if (stat <  98)  total += 2; /* 18/70 to 18/79 -CFT */
    else if (stat < 108)  total += 3; /* 18/80 to 18/89 -CFT */
    else if (stat < 118)  total += 4; /* 18/90 to 18/99 -CFT */
    else if (stat < 128)  total += 5; /* 18/100 to 18/109 -CFT */
    else if (stat < 138)  total += 6;
    else if (stat < 148)  total += 7;
    else if (stat < 158)  total += 8;
    else if (stat < 168)  total += 9;
    else if (stat < 178)  total +=10;
    else if (stat < 188)  total +=11;
    else if (stat < 198)  total +=12;
    else if (stat < 218)  total +=13;
    else if (stat < 228)  total +=14;
    else total += 16;
    return (total);
}


/*
 * Returns a character's adjustment to armor class	 -JWT-	 
 */
int toac_adj(void)
{
    register int stat;

    stat = p_ptr->use_stat[A_DEX];
    if      (stat <   4)  return(-4);
    else if (stat ==  4)  return(-3);
    else if (stat ==  5)  return(-2);
    else if (stat ==  6)  return(-1);
    else if (stat <  15)  return( 0);
    else if (stat <  18)  return( 1);
    else if (stat <  58)  return( 2); /* 18 to 18/49 -CFT */
    else if (stat <  98)  return( 3); /* 18/50 to 18/79 -CFT */
    else if (stat < 108)  return( 4); /* 18/80 to 18/89 -CFT */
    else if (stat < 118)  return( 5); /* 18/90 to /99 -CFT */
    else if (stat < 128)  return( 6); /* /100 to /109 -CFT */
    else if (stat < 138)  return( 7);
    else if (stat < 148)  return( 8);
    else if (stat < 158)  return( 9);
    else if (stat < 168)  return(10);
    else if (stat < 178)  return(11);
    else if (stat < 188)  return(12);
    else if (stat < 198)  return(13);
    else if (stat < 218)  return(14);
    else if (stat < 228)  return(15);
    else                  return(17);
}


/*
 * Returns a character's adjustment to disarm		 -RAK-	 
 */
int todis_adj(void)
{
    register int stat;

    stat = p_ptr->use_stat[A_DEX];
    if      (stat <=  3)  return(-8);
    else if (stat ==  4)  return(-6);
    else if (stat ==  5)  return(-4);
    else if (stat ==  6)  return(-2);
    else if (stat ==  7)  return(-1);
    else if (stat <  13)  return( 0);
    else if (stat <  16)  return( 1);
    else if (stat <  18)  return( 2);
    else if (stat <  58)  return( 4); /* 18 to 18/49 -CFT */
    else if (stat <  88)  return( 5); /* 18/50 to 18/69 -CFT */
    else if (stat < 108)  return( 6); /* 18/70 to 18/89 -CFT */
    else if (stat < 118)  return( 7); /* 18/90 to 18/99 -CFT */
    else                  return( 8); /* 18/100 and over -CFT */
}


/*
 * Returns a character's adjustment to damage	 
 */
int todam_adj(void)
{
    register int stat;

    stat = p_ptr->use_stat[A_STR];
    if      (stat <   4)  return(-2);
    else if (stat <   5)  return(-1);
    else if (stat <  16)  return( 0);
    else if (stat <  17)  return( 1);
    else if (stat <  18)  return( 2);
    else if (stat <  88)  return( 3); /* 18 to 18/69 -CFT */
    else if (stat <  98)  return( 4); /* 18/70 to 18/79 -CFT */
    else if (stat < 108)  return( 5); /* 18/80 to 18/89 -CFT */
    else if (stat < 118)  return( 5); /* 18/90 to 18/99 -CFT */
    else if (stat < 128)  return( 6); /* 18/100 to /109 -CFT */
    else if (stat < 138)  return( 7);
    else if (stat < 148)  return( 8);
    else if (stat < 158)  return( 9);
    else if (stat < 168)  return(10);
    else if (stat < 178)  return(11);
    else if (stat < 188)  return(12);
    else if (stat < 198)  return(13);
    else if (stat < 218)  return(14);
    else if (stat < 228)  return(16);
    else                  return(20);
}


/*
 * Print long number with header at given row, column
 * Use the color for the number, not the header
 */
static void prt_lnum(cptr header, u32b num, int row, int col, byte color)
{
    int len = strlen(header);
    char out_val[32];
    put_str(header, row, col);
    (void)sprintf(out_val, "%9ld", (long)num);
    c_put_str(color, out_val, row, col + len);
}

/*
 * Print number with header at given row, column
 */
static void prt_num(cptr header, int num, int row, int col, byte color)
{
    int len = strlen(header);
    char out_val[32];
    put_str(header, row, col);
    put_str("   ", row, col + len);
    (void)sprintf(out_val, "%6ld", (long)num);
    c_put_str(color, out_val, row, col + len + 3);
}



/*
 * Prints the following information on the screen.	-JWT-	 
 */
void put_character()
{
    clear_screen();

    put_str("Name        :", 2, 1);
    put_str("Sex         :", 3, 1);
    put_str("Race        :", 4, 1);
    put_str("Class       :", 5, 1);

    c_put_str(TERM_L_BLUE, player_name, 2, 15);
    c_put_str(TERM_L_BLUE, (p_ptr->male ? "Male" : "Female"), 3, 15);
    c_put_str(TERM_L_BLUE, race[p_ptr->prace].trace, 4, 15);
    c_put_str(TERM_L_BLUE, class[p_ptr->pclass].title, 5, 15);
}



/*
 * Prints some information on the screen
 */
void put_stats()
{
    int          i;
    vtype        buf;

    /* Display the stats */
    for (i = 0; i < 6; i++) {

	/* Special treatment of "injured" stats */
	if (p_ptr->cur_stat[i] < p_ptr->max_stat[i]) {

	    /* Use lowercase stat name */
	    put_str(stat_names_reduced[i], 2 + i, 61);

	    /* Display the stat itself */
	    cnv_stat(p_ptr->use_stat[i], buf);
	    c_put_str(TERM_YELLOW, buf, 2 + i, 66);

	    /* Display the maximum stat */
	    cnv_stat(modify_max_stat(i, p_ptr->mod_stat[i]), buf);

	    /* Display the maximum stat */
	    c_put_str(TERM_L_GREEN, buf, 2 + i, 73);
	}

	else {

	    /* Assume uppercase stat name */
	    put_str(stat_names[i], 2 + i, 61);

	    /* Display the normal stat */
	    cnv_stat(p_ptr->use_stat[i], buf);
	    c_put_str(TERM_L_GREEN, buf, 2 + i, 66);
	}
    }
}


/*
 * Used to pass color info around
 */
static byte likert_color;

/*
 * Returns a "rating" of x depending on y
 */
cptr likert(int x, int y)
{
    if ((x/y) < 0) {
	likert_color = TERM_RED;
	return ("Very Bad");
    }

    switch ((x / y)) {
      case 0:
      case 1:
	likert_color = TERM_RED;
	return ("Bad");
      case 2:
	likert_color = TERM_RED;
	return ("Poor");
      case 3:
      case 4:
	likert_color = TERM_YELLOW;
	return ("Fair");
      case 5:
	likert_color = TERM_YELLOW;
	return ("Good");
      case 6:
	likert_color = TERM_YELLOW;
	return ("Very Good");
      case 7:
      case 8:
	likert_color = TERM_L_GREEN;
	return ("Excellent");
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
	likert_color = TERM_L_GREEN;
	return ("Superb");
      case 14:
      case 15:
      case 16:
      case 17:
	likert_color = TERM_L_GREEN;
	return ("Heroic");
      default:
	likert_color = TERM_L_GREEN;
	return ("Legendary");
    }
}


/*
 * Prints age, height, weight, and Social Class
 */
void put_misc1()
{
    prt_num("Age          ", (int)p_ptr->age, 2, 32, TERM_L_BLUE);
    prt_num("Height       ", (int)p_ptr->ht, 3, 32, TERM_L_BLUE);
    prt_num("Weight       ", (int)p_ptr->wt, 4, 32, TERM_L_BLUE);
    prt_num("Social Class ", (int)p_ptr->sc, 5, 32, TERM_L_BLUE);
}


/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 */
void put_misc2()
{
    int show_tohit = p_ptr->dis_th;
    int show_todam = p_ptr->dis_td;

    inven_type *i_ptr = &inventory[INVEN_WIELD];

    /* Hack -- add in weapon info if known */
    if (known2_p(i_ptr)) show_tohit += i_ptr->tohit;
    if (known2_p(i_ptr)) show_tohit += i_ptr->todam;

    /* Dump the bonuses to hit/dam */
    prt_num("+ To Hit    ", show_tohit, 9, 1, TERM_L_BLUE);
    prt_num("+ To Damage ", show_todam, 10, 1, TERM_L_BLUE);

    /* Dump the armor class bonus */
    prt_num("+ To AC     ", p_ptr->dis_tac, 11, 1, TERM_L_BLUE);

    /* Dump the total armor class */
    prt_num("  Total AC  ", p_ptr->dis_ac, 12, 1, TERM_L_BLUE);

    prt_num("Level      ", (int)p_ptr->lev, 9, 28, TERM_L_GREEN);
    prt_lnum("Experience ", p_ptr->exp, 10, 28,
             p_ptr->exp == p_ptr->max_exp ? TERM_L_GREEN : TERM_YELLOW);
    prt_lnum("Max Exp    ", p_ptr->max_exp, 11, 28, TERM_L_GREEN);
    if (p_ptr->lev >= MAX_PLAYER_LEVEL) {
	put_str("Exp to Adv.     ", 12, 28);
	c_put_str(TERM_L_GREEN, "****", 12, 28+16);
    }
    else {
	prt_lnum("Exp to Adv.",
	    (s32b) (player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L),
	    12, 28, TERM_L_GREEN);
    }
    prt_lnum("Gold       ", p_ptr->au, 13, 28, TERM_L_GREEN);
    prt_num("Max Hit Points ", p_ptr->mhp, 9, 52, TERM_L_GREEN);
    if (p_ptr->mhp == p_ptr->chp) {
	prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_L_GREEN);
    }
    else if (p_ptr->chp > (p_ptr->mhp * hitpoint_warn) / 10) {
	prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_YELLOW);
    }
    else {
	prt_num("Cur Hit Points ", p_ptr->chp, 10, 52, TERM_RED);
    }

    prt_num("Max Mana       ", p_ptr->mana, 11, 52, TERM_L_GREEN);
    if (p_ptr->mana == p_ptr->cmana) {
	prt_num("Cur Mana       ", p_ptr->cmana, 12, 52, TERM_L_GREEN);
    }
    else if (p_ptr->cmana > (p_ptr->mana * hitpoint_warn) / 10) {
	prt_num("Cur Mana       ", p_ptr->cmana, 12, 52, TERM_YELLOW);
    }
    else {
	prt_num("Cur Mana       ", p_ptr->cmana, 12, 52, TERM_RED);
    }
}


/*
 * Prints ratings on certain abilities
 *
 * This code is "repeated" elsewhere to "dump" a character sheet.
 */
void put_misc3()
{
    int			tmp;
    int			xbth, xbthb, xfos, xsrh;
    int			xstl, xdis, xsave, xdev;
    char                xinfra[32];
    cptr		desc;

    /* XXX XXX XXX Skill with current weapon */
    tmp = p_ptr->ptohit + inventory[INVEN_WIELD].tohit;
    xbth = p_ptr->bth + tmp * BTH_PLUS_ADJ +
	   (class_level_adj[p_ptr->pclass][CLA_BTH] * p_ptr->lev);

    /* XXX XXX XXX Skill with current bow */
    tmp = p_ptr->ptohit + inventory[INVEN_BOW].tohit;
    xbthb = p_ptr->bthb + tmp * BTH_PLUS_ADJ +
	    (class_level_adj[p_ptr->pclass][CLA_BTHB] * p_ptr->lev);

    /* Basic abilities */
    xfos = 40 - p_ptr->fos;
    if (xfos < 0) xfos = 0;
    xsrh = p_ptr->srh;
    xstl = p_ptr->stl + 1;
    xdis = p_ptr->disarm + 2 * todis_adj() + stat_adj(A_INT) +
	   (class_level_adj[p_ptr->pclass][CLA_DISARM] * p_ptr->lev / 3);
    xsave = p_ptr->save + stat_adj(A_WIS) +
	    (class_level_adj[p_ptr->pclass][CLA_SAVE] * p_ptr->lev / 3);
    xdev = p_ptr->save + stat_adj(A_INT) +
	   (class_level_adj[p_ptr->pclass][CLA_DEVICE] * p_ptr->lev / 3);

    /* Infravision string */
    (void)sprintf(xinfra, "%d feet", p_ptr->see_infra * 10);

    /* Clear it */
    clear_from(14);

    put_str("(Miscellaneous Abilities)", 15, 25);

    put_str("Fighting    :", 16, 1);
    desc=likert(xbth, 12);
    c_put_str(likert_color, desc, 16, 15);

    put_str("Bows/Throw  :", 17, 1);
    desc=likert(xbthb, 12);
    c_put_str(likert_color, desc, 17, 15);

    put_str("Saving Throw:", 18, 1);
    desc=likert(xsave, 6);
    c_put_str(likert_color, desc, 18, 15);

    put_str("Stealth     :", 16, 28);
    desc=likert(xstl, 1);
    c_put_str(likert_color, desc, 16, 42);

    put_str("Disarming   :", 17, 28);
    desc=likert(xdis, 8);
    c_put_str(likert_color, desc, 17, 42);

    put_str("Magic Device:", 18, 28);
    desc=likert(xdev, 6);
    c_put_str(likert_color, desc, 18, 42);

    put_str("Perception  :", 16, 55);
    desc=likert(xfos, 3);
    c_put_str(likert_color, desc, 16, 69);

    put_str("Searching   :", 17, 55);
    desc=likert(xsrh, 6);
    c_put_str(likert_color, desc, 17, 69);

    put_str("Infra-Vision:", 18, 55);
    put_str(xinfra, 18, 69);
}


/*
 * Used to display the character on the screen.
 */
void display_player()
{
    put_character();
    put_stats();
    put_misc1();
    put_misc2();
    put_misc3();
}




#if defined(_Windows)

/*
 * Hack -- extract a "clean" savefile name from a Windows player name
 * This function takes a pointer to a buffer and modifies it in place.
 */
static void process_name(char *buf)
{
    int k;
    cptr s;
    
    /* Extract useful letters */    
    for (k = 0, s = buf; *s; s++) {
	if (isalpha(*s) || isdigit(*s)) buf[k++] = *s;
	else if (*s == ' ') buf[k++] = '_';
    }

    /* Hack -- max length */
    if (k > 8) k = 8;

    /* Terminate */
    buf[k] = '\0';

    /* Prevent "empty" savefile names */
    if (!buf[0]) strcpy(buf, "PLAYER");
}
    
#endif
    


/*
 * Gets a name for the character, reacting to name changes.
 *
 * Assumes that "display_player()" has just been called
 * XXX Perhaps we should NOT ask for a name (at "birth()") on Unix?
 */
void get_name()
{
    char tmp[32];

    /* Prompt and ask */
    prt("Enter your player's name above [press <RETURN> when finished]", 21, 2);

    /* Ask until happy */
    while (1) {

	/* Get an input, let "Escape" and "Empty" cancel */
	if (get_string(tmp, 2, 15, 15) && tmp[0]) {
	
	    /* Accept old name */
	    strcpy(player_name, tmp);
	}

	/* Hack -- Prevent "empty" names */
	if (!player_name[0]) strcpy(player_name, "PLAYER");

#ifdef SAVEFILE_MUTABLE

#if defined(_Windows)
	/* Hack -- Process the player name */
	process_name(tmp);
#else
	/* Normally, just use the whole name */
	strcpy(tmp, player_name);
#endif

#ifdef SAVEFILE_USE_UID
	/* Rename the savefile, using the player UID and character name */
	(void)sprintf(savefile, "%s%s%d%s",
			ANGBAND_DIR_SAVE, PATH_SEP, player_uid, tmp);
#else
	/* Rename the savefile, using the character name */
	(void)sprintf(savefile, "%s%s%s",
			ANGBAND_DIR_SAVE, PATH_SEP, tmp);
#endif

#endif

	/* All done */
	break;
    }

    /* Pad the name (to clear junk) */
    sprintf(tmp, "%-15.15s", player_name);

    /* Re-Draw the name (in light blue) */
    c_put_str(TERM_L_BLUE, tmp, 2, 15);

    /* Erase the prompt, etc */
    clear_from(20);
}


/*
 * Display character, allow name change or character dump.
 */
void change_name()
{
    register char c;
    register int  flag;
    vtype         temp;

    display_player();

    for (flag = FALSE; !flag; ) {

	prt("<f>ile character description. <c>hange character name.", 21, 2);
	c = inkey();
	switch (c) {

	  case 'c':
	    get_name();
	    flag = TRUE;
	    break;

	  case 'f':
#ifdef MACINTOSH
	    if (mac_file_character()) flag = TRUE;
#else
	    prt("File name:", 0, 0);
	    if (get_string(temp, 0, 10, 60) && temp[0]) {
		if (file_character(temp)) flag = TRUE;
	    }
#endif
	    break;

	  case ESCAPE:
	  case ' ':
	  case '\n':
	  case '\r':
	    flag = TRUE;
	    break;

	  default:
	    bell();
	    break;
	}
    }
}




/*
 * Describe number of remaining charges.		-RAK-	
 */
void inven_item_charges(int item_val)
{
    register int rem_num;
    vtype        out_val;

    if (known2_p(&inventory[item_val])) {
	rem_num = inventory[item_val].pval;
	(void)sprintf(out_val, "You have %d charges remaining.", rem_num);
	msg_print(out_val);
    }
}


/*
 * Describe an inventory item, in terms of its "number"
 * Ex: "You have 5 arrows (+1,+1)." or "You have no more arrows."
 */
void inven_item_describe(int i_idx)
{
    inven_type *i_ptr;
    bigvtype tmp_str;

    i_ptr = &inventory[i_idx];

    /* Get a description */
    objdes(tmp_str, i_ptr, TRUE);

    /* Print a message */
    message("You have ", 0x02);
    message(tmp_str, 0x02);
    message(".", 0);
}




/*
 * Look for things to combine with the given item
 * Return the (possibly changed) index of the item
 */
int combine(int i)
{
    register int         j, k;
    register inven_type *i_ptr, *j_ptr;

    /* Allow function chaining with "ident_spell()" */
    if (i < 0) return (-1);

    /* No combination possible for non-pack items */
    if (i >= inven_ctr) return (i);

    /* Get the "base item" */
    i_ptr = &inventory[i];

    /* Find everything that can combine with us */
    for (j = inven_ctr - 1; j >= 0; j--) {

	/* Get the pack item */
	j_ptr = &inventory[j];

	/* Are they combinable? */
	if (item_similar(i_ptr, j_ptr)) {

	    /* Message */
	    msg_print("You combine similar objects.");

	    /* Add together the item counts */
	    i_ptr->number += j_ptr->number;

	    /* Hack -- maintain the MINIMUM cost */
	    if (i_ptr->cost > j_ptr->cost) i_ptr->cost = j_ptr->cost;
	    
	    /* One less item */
	    inven_ctr--;

	    /* Slide the inventory (via structure copy) */
	    for (k = j; k < inven_ctr; k++) {
		inventory[k] = inventory[k + 1];
	    }

	    /* Notice if the base object has moved "up" */
	    if (i > j) i_ptr = &inventory[--i];

	    /* Erase the last object */
	    invcopy(&inventory[inven_ctr], OBJ_NOTHING);
	}
    }

    /* Return the (possibly new) index */
    return (i);
}


/*
 * Hack -- apply "combine" to every item in the pack
 * Note that objects will only move "up" in the pack
 */
void combine_pack(void)
{
    register int i, j, k;
    inven_type *i_ptr, *j_ptr;

    /* Scan the pack (from the start) */
    for (i = 0; i < inven_ctr; i++) {

	/* Get the item */
	i_ptr = &inventory[i];

	/* Only check items BELOW us */
	for (j = inven_ctr-1; j > i; j--) {

	    /* Get the item */
	    j_ptr = &inventory[j];

	    /* Can we drop "j_ptr" onto "i_ptr"? */
	    if (item_similar(i_ptr, j_ptr)) {

		/* Message */
		msg_print("You combine similar objects.");

		/* Add together the item counts */
		i_ptr->number += j_ptr->number;

		/* Hack -- maintain the MINIMUM cost */
		if (i_ptr->cost > j_ptr->cost) i_ptr->cost = j_ptr->cost;

		/* One fewer object */
		inven_ctr--;

		/* Slide the inventory (via structure copy) */
		for (k = j; k < inven_ctr; k++) {
		    inventory[k] = inventory[k + 1];
		}

		/* Erase the last object */
		invcopy(&inventory[inven_ctr], OBJ_NOTHING);
	    }
	}
    }
}



/*
 * Increase the "number" of a given item by a given amount
 * Be sure not to exceed the legal bounds.
 * Note that this can result in an item with zero items
 * Take account of changes to the players weight.
 * Note that i_idx is an index into the inventory.
 */
void inven_item_increase(int i_idx, int num)
{
    int cnt;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &inventory[i_idx];

    /* Bounds check */
    cnt = i_ptr->number + num;
    if (cnt > 255) cnt = 255;
    else if (cnt < 0) cnt = 0;
    num = cnt - i_ptr->number;

    /* Change the number and weight */
    if (num) {

	/* Add the weight */
	i_ptr->number += num;
	inven_weight += num * i_ptr->weight;
	
	/* Remember to recalculate bonuses */
	p_ptr->status |= PY_STR_WGT;
    }
}


/*
 * Destroy an inventory slot if it has no more items
 * Slides items if necessary, and clears out the hole
 */
void inven_item_optimize(int i_idx)
{
    register int i;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &inventory[i_idx];

    /* Paranoia -- be sure it exists */
    if (i_ptr->tval == TV_NOTHING) return;

    /* Only optimize if empty */
    if (i_ptr->number) return;

    /* The item is in the pack */
    if (i_idx < INVEN_WIELD) {

	/* One less item */
	inven_ctr--;

	/* Slide later entries onto us */
	for (i = i_idx; i < inven_ctr; i++) {
	    inventory[i] = inventory[i + 1];
	}

	/* Paranoia -- erase the empty slot */
	invcopy(&inventory[inven_ctr], OBJ_NOTHING);
    }

    /* The item is being wielded */
    else {

	/* One less item */
	equip_ctr--;

	/* Paranoia -- erase the empty slot */
	invcopy(&inventory[i_idx], OBJ_NOTHING);
    }
}





/*
 * Computes current weight limit.
 */
int weight_limit(void)
{
    register s32b weight_cap;

    /* Factor in strength */
    weight_cap = (long)p_ptr->use_stat[A_STR] * (long)PLAYER_WEIGHT_CAP;

    /* Hack -- large players can carry more */
    weight_cap += (long)p_ptr->wt;

    /* Nobody can carry more than 300 pounds */
    if (weight_cap > 3000L) weight_cap = 3000L;

    /* Return the result */
    return ((int)weight_cap);
}



/*
 * Will "inven_carry(i_ptr)" succeed without inducing pack overflow?
 */
int inven_check_num(inven_type *i_ptr)
{
    register int i;

    /* If there is an empty space, we are fine */
    if (inven_ctr < INVEN_PACK) return TRUE;

    /* Scan every possible match */
    for (i = 0; i < inven_ctr; i++) {

	/* Get that item */
	inven_type *j_ptr = &inventory[i];

	/* Check if the two items can be combined */
	if (item_similar(j_ptr, i_ptr)) return TRUE;
    }

    /* And there was no room in the inn... */
    return FALSE;
}

/*
 * Add an item to the players inventory, and return the slot used.
 *
 * This function can be used to "over-fill" the player's pack, but note
 * that this is not recommended, and will induce an "automatic drop" of
 * one of the items.  This function thus never "fails", but it can be
 * result in an "overflow" situation requiring immediate attention.
 *
 * When an "overflow" situation is about to occur, the new item is always
 * placed into the "icky" slot where it will be automatically dropped by
 * the overflow routine in "dungeon.c".
 *
 * Forget where on the floor the item used to be (if anywhere).
 *
 * Items will sort into place, with mage spellbooks coming first for mages,
 * rangers, and rogues.  Also, this will make Tenser's book sort after all
 * the mage books except Raals, instead of in the middle of them (which
 * always seemed strange to somebody).
 *
 * We should use the same "stacking" logic as "inven_check_num()" above.
 *
 * Note the stacking code below now allows groupable objects to combine.
 * See item_similar() for more information.  This also prevents the
 * "reselling discounted item" problems from previous versions.
 *
 * The sorting order gives away the "goodness" of "Special Lites"
 * but not of any of the food, amulets, rings, potions, staffs, etc.
 */
int inven_carry(inven_type *i_ptr)
{
    register int         slot, i;
    s32b		 i_value, j_value;
    register inven_type *j_ptr;


    /* The tval of readible books */
    int read_tval = TV_NOTHING;

    /* Acquire the type value of the books that the player can read, if any */
    if (class[p_ptr->pclass].spell == PRIEST) read_tval = TV_PRAYER_BOOK;
    else if (class[p_ptr->pclass].spell == MAGE) read_tval = TV_MAGIC_BOOK;


    /* Check all the items in the pack (attempt to combine) */
    for (slot = 0; slot < inven_ctr; slot++) {

	/* Access that inventory item */
	j_ptr = &inventory[slot];

	/* Check if the two items can be combined */
	if (item_similar(j_ptr, i_ptr)) {

	    /* Add together the item counts */
	    j_ptr->number += i_ptr->number;

	    /* Hack -- maintain the MINIMUM cost */
	    if (j_ptr->cost > i_ptr->cost) j_ptr->cost = i_ptr->cost;

	    /* Increase the weight */
	    inven_weight += i_ptr->number * i_ptr->weight;

	    /* Remember to recalculate bonuses */
	    p_ptr->status |= PY_STR_WGT;

	    /* All done, report where we put it */
	    return slot;
	}
    }


    /* Mega-Hack -- use "icky" slot when about to "overflow" */
    if (inven_ctr == INVEN_PACK) {

	/* Use the "icky" slot */
	slot = INVEN_PACK;

	/* Structure copy to insert the new item */
	inventory[slot] = *i_ptr;

	/* Forget the "location" if any */
	inventory[slot].iy = inventory[slot].ix = 0;

	/* One more item present now */
	inven_ctr++;

	/* Increase the weight, prepare to redraw */
	inven_weight += i_ptr->number * i_ptr->weight;

	/* Remember to re-calculate bonuses */
	p_ptr->status |= PY_STR_WGT;

	/* Say where it went */
	return slot;
    }
    
    
    /* Get the "value" of the item being carried */
    i_value = item_value(i_ptr);

    /* Scan every slot (default to the empty one) */
    for (slot = 0; slot < inven_ctr; slot++) {

	/* Get the item already there */
	j_ptr = &inventory[slot];

	/* Hack -- readable books always come first */
	if ((i_ptr->tval == read_tval) && (j_ptr->tval != read_tval)) break;
	if ((j_ptr->tval == read_tval) && (i_ptr->tval != read_tval)) continue;

	/* Objects sort by decreasing type */
	if (i_ptr->tval > j_ptr->tval) break;
	if (i_ptr->tval < j_ptr->tval) continue;

	/* Flavored items with Unknown effects come last */
	if (!inven_aware_p(i_ptr)) continue;
	if (!inven_aware_p(j_ptr)) break;

	/* Objects sort by increasing sval */
	if (i_ptr->sval < j_ptr->sval) break;
	if (i_ptr->sval > j_ptr->sval) continue;

	/* Unidentified objects come last */
	if (!known2_p(i_ptr)) continue;
	if (!known2_p(j_ptr)) break;

	/* Objects sort by decreasing value */
	j_value = item_value(j_ptr);
	if (i_value > j_value) break;
	if (i_value < j_value) continue;
    }

    /* Structure slide (make room) */
    for (i = inven_ctr; i > slot; i--) {
	inventory[i] = inventory[i-1];
    }

    /* Structure copy to insert the new item */
    inventory[slot] = *i_ptr;

    /* Forget the old location */
    inventory[slot].iy = inventory[slot].ix = 0;

    /* One more item present now */
    inven_ctr++;

    /* Increase the weight, prepare to redraw */
    inven_weight += i_ptr->number * i_ptr->weight;

    /* Remember to re-calculate bonuses */
    p_ptr->status |= PY_STR_WGT;

    /* Say where it went */
    return slot;
}


/*
 * Returns spell chance of failure for spell		-RAK-	 
 */
int spell_chance(int spell)
{
    register spell_type *s_ptr;
    register int         chance;
    register int         stat;
    int                  minfail;

    s_ptr = &magic_spell[p_ptr->pclass - 1][spell];
    chance = s_ptr->sfail - 3 * (p_ptr->lev - s_ptr->slevel);
    stat = (class[p_ptr->pclass].spell == MAGE) ? A_INT : A_WIS;
    chance -= 3 * (stat_adj(stat) - 1);
    if (s_ptr->smana > p_ptr->cmana) {
	chance += 5 * (s_ptr->smana - p_ptr->cmana);
    }
    
    switch (stat_adj(stat)) {
      case 0:
	minfail = 50;
	break;
      case 1:
	minfail = 12;
	break;			   /* 8-14 stat */
      case 2:
	minfail = 8;
	break;			   /* 15-17 stat */
      case 3:
	minfail = 5;
	break;			   /* 18-18/49 stat */
      case 4:
	minfail = 4;
	break;			   /* 18/50-18/69 */
      case 5:
	minfail = 4;
	break;			   /* 18/70-18/89 */
      case 6:
	minfail = 3;
	break;			   /* 18/90-18/99 */
      case 7:
	minfail = 3;
	break;			   /* 18/100 */
      case 8:
      case 9:
      case 10:
	minfail = 2;
	break;			   /* 18/101 - /130 */
      case 11:
      case 12:
	minfail = 2;
	break;			   /* /131 - /150 */
      case 13:
      case 14:
	minfail = 1;
	break;			   /* /151 - /170 */
      case 15:
      case 16:
	minfail = 1;
	break;			   /* /171 - /200 */
      default:
	minfail = 0;
	break;			   /* > 18/200 */
    }

     /* only mages/priests can get best chances... */
    if ((minfail < 5) && (p_ptr->pclass != 1) && (p_ptr->pclass != 2)) {
	minfail = 5;
    }

    /* Big prayer penalty for edged weapons  -DGK */
    /* XXX So, like, switch weapons and then pray? */
    if (p_ptr->pclass == 2) {
	register inven_type *i_ptr;
	i_ptr = &inventory[INVEN_WIELD];
	if ((i_ptr->tval == TV_SWORD) || (i_ptr->tval == TV_POLEARM)) {
	    if (!(i_ptr->flags3 & TR3_BLESSED)) {
		chance += 25;
	    }
	}
    }

    if (chance > 95) return (95);

    if (chance < minfail) return (minfail);

    return chance;
}


/*
 * Print list of spells					-RAK-
 *
 * if (nonconsec < 0) spells numbered consecutively from
 * 'a' to 'a'+num, for example, when learning spells.
 *
 * Otherwise, spells numbered by offset from nonconsec, such as when
 * asking which spell to cast 
 */
void print_spells(int *spell, int num, int comment, int nonconsec)
{
    register int         i, j;
    register spell_type *s_ptr;
    int                  col, offset;
    cptr		 p;
    char                 spell_char;
    vtype                out_val;

    col = (comment ? 22 : 31);

    offset = (class[p_ptr->pclass].spell == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
    erase_line(1, col);
    put_str("Name", 1, col + 5);
    put_str("Lv Mana Fail", 1, col + 35);

    /* only show the first 22 choices */
    if (num > 22) num = 22;

    for (i = 0; i < num; i++) {
	j = spell[i];
	s_ptr = &magic_spell[p_ptr->pclass - 1][j];
	if (comment == FALSE) {
	    p = "";
	}
	else if (j >= 32 ?
		 ((spell_forgotten2 & (1L << (j - 32))) != 0) :
		 ((spell_forgotten & (1L << j)) != 0)) {
	    p = " forgotten";
	}
	else if (j >= 32 ?
		 ((spell_learned2 & (1L << (j - 32))) == 0) :
		 ((spell_learned & (1L << j)) == 0)) {
	    p = " unknown";
	}
	else if (j >= 32 ?
		 ((spell_worked2 & (1L << (j - 32))) == 0) :
		 ((spell_worked & (1L << j)) == 0)) {
	    p = " untried";
	}
	else {
	    p = "";
	}

        /* determine whether or not to leave holes in character choices */
	if (nonconsec == -1) {
	    spell_char = 'a' + i;
	}
	else {
	    spell_char = 'a' + j - nonconsec;
	}

	(void)sprintf(out_val, "  %c) %-30s%2d %4d %3d%%%s", spell_char,
		      spell_names[j + offset], s_ptr->slevel, s_ptr->smana,
		      spell_chance(j), p);
	prt(out_val, 2 + i, col);
    }
}


/*
 * Let the player pick a spell from a book.
 * Attempt to maintain some spell naming consistency
 */
int get_spell(int *spell, int num, int *sn, int *sc, cptr prompt, int first_spell)
{
    register spell_type *s_ptr;
    int                  flag, redraw, offset, i;
    char                 choice;
    vtype                out_str, tmp_str;

    *sn = (-1);
    flag = FALSE;
    (void)sprintf(out_str, "(Spells %c-%c, *=List, <ESCAPE>=exit) %s",
	   spell[0] + 'a' - first_spell, spell[num - 1] + 'a' - first_spell,
		  prompt);
    redraw = FALSE;
    offset = (class[p_ptr->pclass].spell == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
    
    /* Update the "choice" window */
    choice_spell(spell, num, first_spell);

    /* Get a spell from the user */
    while (!flag && get_com(out_str, &choice)) {

	if (isupper((int)choice)) {

	    *sn = choice - 'A' + first_spell;

	    /* verify that this is in spell[], at most 22 entries in spell[] */
	    for (i = 0; i < num; i++) {
		if (*sn == spell[i]) break;
	    }
	    if (i == num) {
		*sn = (-2);
	    }
	    else {
		s_ptr = &magic_spell[p_ptr->pclass - 1][*sn];
		(void)sprintf(tmp_str, "Cast %s (%d mana, %d%% fail)?",
			      spell_names[*sn + offset], s_ptr->smana,
			      spell_chance(*sn));
		if (get_check(tmp_str)) {
		    flag = TRUE;
		}
		else {
		    *sn = (-1);
		}
	    }
	}

	else if (islower((int)choice)) {

	    *sn = choice - 'a' + first_spell;

	    /* verify that this is in spell[], at most 22 entries in spell[] */
	    for (i = 0; i < num; i++) {
		if (*sn == spell[i]) break;
	    }
	    if (i == num) {
		*sn = (-2);
	    }
	    else {
		flag = TRUE;
	    }
	}
	else if (choice == '*') {
	    /* only do this drawing once */
	    if (!redraw) {
		save_screen();
		redraw = TRUE;
		print_spells(spell, num, FALSE, first_spell);
	    }
	}
	else if (isalpha((int)choice)) {
	    *sn = (-2);
	}
	else {
	    *sn = (-1);
	    bell();
	}

	if (*sn == -2) {
	    sprintf(tmp_str, "You don't know that %s.",
		(class[p_ptr->pclass].spell == MAGE) ? "spell" : "prayer");
	    msg_print(tmp_str);
	}
    }
    
    if (redraw) restore_screen();

    erase_line(MSG_LINE, 0);
    if (flag) *sc = spell_chance(*sn);

    /* Fix the choice window */
    choice_inven(0, inven_ctr - 1);

    /* Return TRUE if choice made */
    return (flag);
}



/*
 * Increases hit points and level			-RAK-	 
 */
static void gain_level(void)
{
    vtype               out_val;

    p_ptr->lev++;
    (void)sprintf(out_val, "Welcome to level %d.", (int)p_ptr->lev);
    msg_print(out_val);
    calc_hitpoints();
    prt_level();
    prt_title();

    if (class[p_ptr->pclass].spell == MAGE) {
	calc_spells(A_INT);
	calc_mana(A_INT);
    }
    else if (class[p_ptr->pclass].spell == PRIEST) {
	calc_spells(A_WIS);
	calc_mana(A_WIS);
    }
}


/*
 * Prints experience					-RAK-	 
 */
void prt_experience()
{
    if (p_ptr->exp > MAX_EXP) p_ptr->exp = MAX_EXP;

    if (p_ptr->lev < MAX_PLAYER_LEVEL) {

	while ((player_exp[p_ptr->lev-1] * p_ptr->expfact / 100L)
	       <= p_ptr->exp && (p_ptr->lev < MAX_PLAYER_LEVEL)) {

	    gain_level();

	    /* Level was actually gained, not restored */
	    if (p_ptr->exp > p_ptr->max_exp) {

		/* Age the player.  The "300" is arbitrary, but chosen to */
		/* make human ages work.  The other racial age adjustments */
		/* were based on this "300" as well. -CFT */
		p_ptr->age += randint((u16b)class[p_ptr->pclass].age_adj *
				       (u16b)race[p_ptr->prace].m_age)/300L;
	    }
	}
    }

    if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

    prt_exp();
}



/*
 * Calculate the players hit points
 */
void calc_hitpoints()
{
    register int          hitpoints;
    register s32b        value;

    /* Calculate hitpoints */
    hitpoints = player_hp[p_ptr->lev - 1] + (con_adj() * p_ptr->lev);

    /* Hack -- minimum hitpoints */
    if (hitpoints < (p_ptr->lev + 1)) {
	hitpoints = p_ptr->lev + 1;
    }

    /* Factor in the hero / superhero values */
    if (p_ptr->hero > 0) hitpoints += 10;
    if (p_ptr->shero > 0) hitpoints += 30;

    /* mhp can equal zero while character is being created */
    if (p_ptr->mhp && (hitpoints != p_ptr->mhp)) {

        /* change current hit points proportionately to change of mhp */
        /* divide first to avoid overflow, little loss of accuracy */
	value = (((long)p_ptr->chp << 16) + p_ptr->chp_frac) / p_ptr->mhp;
	value = value * hitpoints;
	p_ptr->chp = value >> 16;
	p_ptr->chp_frac = value & 0xFFFF;

	/* Save the new max-hitpoints */
	p_ptr->mhp = hitpoints;

	/* Remember to redisplay the hitpoints */
	p_ptr->status |= PY_HP;
    }
}


/*
 * Replace the first instance of "target" in "buf" with "insert"
 * If "insert" is NULL, just remove the first instance of "target"
 * In either case, return TRUE if "target" is found.
 *
 * XXX Could be made more efficient, especially in the
 * case where "insert" is smaller than "target".
 */
bool insert_str(char *buf, cptr target, cptr insert)
{
    register int   i, len;
    int		   b_len, t_len, i_len;

    /* Attempt to find the target (modify "buf") */
    buf = strstr(buf, target);

    /* No target found */
    if (!buf) return (FALSE);

    /* Be sure we have an insertion string */
    if (!insert) insert = "";

    /* Extract some lengths */
    t_len = strlen(target);
    i_len = strlen(insert);
    b_len = strlen(buf);

    /* How much "movement" do we need? */
    len = i_len - t_len;

    /* We need less space (for insert) */
    if (len < 0) {
	for (i = t_len; i < b_len; ++i) buf[i+len] = buf[i];
    }

    /* We need more space (for insert) */
    else if (len > 0) {
	for (i = b_len-1; i >= t_len; --i) buf[i+len] = buf[i];
    }

    /* If movement occured, we need a new terminator */
    if (len) buf[b_len+len] = '\0';

    /* Now copy the insertion string */
    for (i = 0; i < i_len; ++i) buf[i] = insert[i];

    /* Successful operation */
    return (TRUE);
}



/*
 * Lets certain players enter wizard mode after a disclaimer...	-JEW-
 */
int enter_wiz_mode(void)
{
    register int answer = FALSE;

    if (!can_be_wizard) return FALSE;

    if (!noscore) {
	msg_print("Wizard mode is for debugging and experimenting.");
	msg_print("The game will not be scored if you enter wizard mode.");
	answer = get_check("Are you sure you want to enter wizard mode?");
    }

    if (noscore || answer) {
	noscore |= 0x2;
	wizard = TRUE;
	return (TRUE);
    }

    return (FALSE);
}


/*
 * Weapon weight VS strength and dexterity		-RAK-	
 */
int attack_blows(int weight)
{
    int i, adj_weight, str_index, dex_index;

    /* Number of blows */
    int b = 0;
    
    /* Start with the strength/dexterity */
    int s = p_ptr->use_stat[A_STR];
    int d = p_ptr->use_stat[A_DEX];

    /* Normal weapons */
    if (weight <= s * 15) {

	/* Access the dexterity */
	if (d < 10) dex_index = 0;
	else if (d < 19) dex_index = 1;
	else if (d < 68) dex_index = 2;
	else if (d < 108) dex_index = 3;
	else if (d < 118) dex_index = 4;
	else if (d < 119) dex_index = 5;
	else if (d < 128) dex_index = 6;
	else if (d < 138) dex_index = 7;
	else if (d < 148) dex_index = 8;
	else if (d < 158) dex_index = 9;
	else if (d < 168) dex_index = 10;
	else dex_index = 11;

	/* new class-based weight penalties -CWS */
	switch (p_ptr->pclass) {
	  case 0:				/* Warriors */
	    adj_weight = ((s * 10) / ((weight < 30) ? 30 : weight));
	    break;
	  case 1:				/* Mages */
	    adj_weight = ((s * 4) / ((weight < 40) ? 40 : weight));
	    break;
	  case 2:				/* Priests */
	    adj_weight = ((s * 7) / ((weight < 35) ? 35 : weight));
	    break;
	  case 3:				/* Rogues */
	    adj_weight = ((s * 6) / ((weight < 30) ? 30 : weight));
	    break;
	  case 4:				/* Rangers */
	    adj_weight = ((s * 8) / ((weight < 35) ? 35 : weight));
	    break;
	  default:			/* Paladins */
	    adj_weight = ((s * 8) / ((weight < 30) ? 30 : weight));
	    break;
	}

	/* Access the strength vs weight */
	if (adj_weight < 2) str_index = 0;
	else if (adj_weight < 3) str_index = 1;
	else if (adj_weight < 4) str_index = 2;
	else if (adj_weight < 6) str_index = 3;
	else if (adj_weight < 8) str_index = 4;
	else if (adj_weight < 10) str_index = 5;
	else if (adj_weight < 13) str_index = 6;
	else if (adj_weight < 15) str_index = 7;
	else if (adj_weight < 18) str_index = 8;
	else if (adj_weight < 20) str_index = 9;
	else str_index = 10;

	/* Use the blows table */
	b = (int)blows_table[str_index][dex_index];

	/* Non-warrior attack penalty */
	if (p_ptr->pclass != 0) {
	    if (b > 5) b = 5;
	}
	
	/* Mage attack penalty */
	if (p_ptr->pclass == 1) {
	    if (b > 4) b = 4;
	}

	/* Hack -- Equipment of Extra Attacks */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
	    if (inventory[i].flags1 & TR1_ATTACK_SPD) {
		b += inventory[i].pval;
	    }
	}
    }

    /* Always get at least one attack */
    if (b < 1) b = 1;

    /* Return the attack count */
    return (b);
}


/*
 * Extract the "total damage" from a given object hitting a given monster.
 *
 * Note that "flasks of oil" do NOT do fire damage, although they
 * certainly could be made to do so.
 *
 * XXX XXX Mega-Hack -- "Earthquake Brand" is dealt with here.
 */
int tot_dam(inven_type *i_ptr, int tdam, int r_idx)
{
    /* Only "missiles" and "weapons" can use these flags */
    if ((i_ptr->tval == TV_SHOT) ||
	(i_ptr->tval == TV_ARROW) ||
	(i_ptr->tval == TV_BOLT) ||
	(i_ptr->tval == TV_HAFTED) ||
	(i_ptr->tval == TV_POLEARM) ||
	(i_ptr->tval == TV_SWORD) ||
	(i_ptr->tval == TV_DIGGING)) {

	int mult = 1;

	monster_race	*r_ptr = &r_list[r_idx];
	monster_lore	*l_ptr = &l_list[r_idx];

	/* Execute Dragon */
	if ((i_ptr->flags1 & TR1_KILL_DRAGON) &&
	    (r_ptr->cflags2 & MF2_DRAGON)) {

	    if (mult < 5) mult = 5;
	    l_ptr->r_cflags2 |= MF2_DRAGON;
	}

	/* Slay Dragon  */
	if ((i_ptr->flags1 & TR1_SLAY_DRAGON) &&
	    (r_ptr->cflags2 & MF2_DRAGON)) {

	    if (mult < 3) mult = 3;
	    l_ptr->r_cflags2 |= MF2_DRAGON;
	}

	/* Slay Undead */
	if ((i_ptr->flags1 & TR1_SLAY_UNDEAD) &&
	    (r_ptr->cflags2 & MF2_UNDEAD)) {

	    if (mult < 3) mult = 3;
	    l_ptr->r_cflags2 |= MF2_UNDEAD;
	}

	/* Slay Orc */
	if ((i_ptr->flags1 & TR1_SLAY_ORC) &&
	    (r_ptr->cflags2 & MF2_ORC)) {

	    if (mult < 3) mult = 3;
	    l_ptr->r_cflags2 |= MF2_ORC;
	}

	/* Slay Troll */
	if ((i_ptr->flags1 & TR1_SLAY_TROLL) &&
	    (r_ptr->cflags2 & MF2_TROLL)) {

	    if (mult < 3) mult = 3;
	    l_ptr->r_cflags2 |= MF2_TROLL;
	}

	/* Slay Giant */
	if ((i_ptr->flags1 & TR1_SLAY_GIANT) &&
	    (r_ptr->cflags2 & MF2_GIANT)) {

	    if (mult < 3) mult = 3;
	    l_ptr->r_cflags2 |= MF2_GIANT;
	}

	/* Slay Demon */
	if ((i_ptr->flags1 & TR1_SLAY_DEMON) &&
	    (r_ptr->cflags2 & MF2_DEMON)) {

	    if (mult < 3) mult = 3;
	    l_ptr->r_cflags2 |= MF2_DEMON;
	}

	/* Slay Evil */
	if ((i_ptr->flags1 & TR1_SLAY_EVIL) &&
	    (r_ptr->cflags2 & MF2_EVIL)) {

	    if (mult < 2) mult = 2;
	    l_ptr->r_cflags2 |= MF2_EVIL;
	}

	/* Slay Animal */
	if ((i_ptr->flags1 & TR1_SLAY_ANIMAL) &&
	    (r_ptr->cflags2 & MF2_ANIMAL)) {

	    if (mult < 2) mult = 2;
	    l_ptr->r_cflags2 |= MF2_ANIMAL;
	}


	/* Lightning Brand */
	if (i_ptr->flags1 & TR1_BRAND_ELEC) {

	    /* Notice immunity */
	    if (r_ptr->cflags2 & MF2_IM_ELEC) {
		l_ptr->r_cflags2 |= MF2_IM_ELEC;
	    }

	    /* Otherwise, take the damage */
	    else {
		if (mult < 5) mult = 5;
	    }
	}

	/* Frost Brand */
	if (i_ptr->flags1 & TR1_BRAND_COLD) {

	    /* Notice immunity */
	    if (r_ptr->cflags2 & MF2_IM_COLD) {
		l_ptr->r_cflags2 |= MF2_IM_COLD;
	    }

	    /* Otherwise, take the damage */
	    else {
		if (mult < 3) mult = 3;
	    }
	}

	/* Flame Tongue */
	if (i_ptr->flags1 & TR1_BRAND_FIRE) {

	    /* Notice immunity */
	    if (r_ptr->cflags2 & MF2_IM_FIRE) {
		l_ptr->r_cflags2 |= MF2_IM_FIRE;
	    }

	    /* Otherwise, take the damage */
	    else {
		if (mult < 3) mult = 3;
	    }
	}


	/* Apply the damage multiplier */
	tdam *= mult;
    }


    /* Return the total damage */
    return (tdam);
}


/*
 * Critical hits, Nasty way to die.			-RAK-	 
 */
int critical_blow(int weight, int plus, int dam, int attack_type)
{
    register int critical;

    critical = dam;

/* Weight of weapon, plusses to hit, and character level all */
/* contribute to the chance of a critical	             */
    if (randint(5000) <= (int)(weight + 5 * plus
			     + (class_level_adj[p_ptr->pclass][attack_type]
				* p_ptr->lev))) {
	weight += randint(650);
	if (weight < 400) {
	    critical = 2 * dam + 5;
	    msg_print("It was a good hit!");
	} else if (weight < 700) {
	    critical = 2 * dam + 10;
	    msg_print("It was an excellent hit!");
	} else if (weight < 900) {
	    critical = 3 * dam + 15;
	    msg_print("It was a superb hit!");
	} else if (weight < 1300) {
	    critical = 3 * dam + 20;
	    msg_print("It was a *GREAT* hit!");
	} else {
	    critical = ((7 * dam) / 2) + 25;
	    msg_print("It was a *SUPERB* hit!");
	}
    }
    return (critical);
}

/*
 * Saving throws for player character.		-RAK-	 
 */
int player_saves(void)
{
    int t1 = class_level_adj[p_ptr->pclass][CLA_SAVE];
    int t2 = p_ptr->save + stat_adj(A_WIS);
    int t3 = t2 + (t1 * p_ptr->lev / 3);
    return (randint(100) <= t3);
}


/*
 * Finds range of item in inventory list		-RAK-	 
 */
int find_range(int item1, int item2, int *j, int *k)
{
    register int         i;
    register inven_type *i_ptr;
    int                  flag;

    i = 0;
    *j = (-1);
    *k = (-1);
    flag = FALSE;
    i_ptr = &inventory[0];
    while (i < inven_ctr) {
	if (!flag) {
	    if ((i_ptr->tval == item1) || (i_ptr->tval == item2)) {
		flag = TRUE;
		*j = i;
	    }
	}
	else {
	    if ((i_ptr->tval != item1) && (i_ptr->tval != item2)) {
		*k = i - 1;
		break;
	    }
	}
	i++;
	i_ptr++;
    }
    if (flag && (*k == -1)) {
	*k = inven_ctr - 1;
    }

    return (flag);
}


/*
 * Teleport the player to a new location, up to "dis" units away.
 * If no such spaces are readily available, the distance may increase.
 * We will try very hard to move the player at least a quarter that distance.
 */
void teleport(int dis)
{
    int x, y, d, count, hack, look, min;

    /* Look until done */
    for (min = dis / 2, look = TRUE; look; min = min / 2) {

	/* Try several locations from "min" to "dis" units away */
	for (count = 0; count < 1000; count++) {

	    /* Pick a (possibly illegal) location */
	    for (hack = 0; hack < 10000; hack++) {
		y = rand_spread(char_row, dis);
		x = rand_spread(char_col, dis);
		d = distance(char_row, char_col, y, x);
		if ((d >= min) && (d <= dis)) break;
	    }
	    
	    /* Ignore illegal locations */
	    if (!in_bounds(y, x)) continue;

	    /* Require "naked" floor space */
	    if (!naked_grid_bold(y, x)) continue;

	    /* No teleporting into vaults and such */
	    if (cave[y][x].fval == VAULT_FLOOR) continue;

	    /* This grid looks good */
	    look = FALSE;
	    break;
	}

	/* Increase the distance if possible */
	if (dis < 200) dis = dis * 2;
    }


    /* Move the player */
    move_rec(char_row, char_col, y, x);

    /* Check for new panel (redraw map) */
    (void)get_panel(char_row, char_col, FALSE);

    /* Update the view/lite */
    update_view();
    update_lite();

    /* Update the monsters */
    update_monsters();


    /* Check the view */
    check_view();


    /* Hack -- The teleport is dealt with */
    teleport_flag = FALSE;
}

