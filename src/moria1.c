/* File: moria1.c */

/* Purpose: player inventory (and related commands) */

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
static void inven_screen(int);
static void inven_takeoff(int, int);
static void inven_drop(int, int);
#endif
#endif






/*
 * Check a char for "vowel-hood"
 */
int is_a_vowel(int ch)
{
    switch (ch & 127) {
      case 'a':
      case 'e':
      case 'i':
      case 'o':
      case 'u':
      case 'A':
      case 'E':
      case 'I':
      case 'O':
      case 'U':
	return (TRUE);
      default:
	return (FALSE);
    }
}


/*
 * Convert an inventory index into a one character label
 * Note that the label does NOT distinguish inven/equip.
 */
int index_to_label(int i)
{
    int j, k;

    /* Indexes for "inven" are easy */
    if (i < INVEN_WIELD) return ('a' + i);

    /* Indexes for "equip" are more subtle */
    for (k = 0, j = INVEN_WIELD; j < i; ++j) {
	inven_type *i_ptr = &inventory[j];
	if (i_ptr->tval == TV_NOTHING) continue;
	k++;
    }

    /* Return the "equip" label */
    return ('a' + k);
}

/*
 * Convert a label into the index of an item in the "inven"
 * Return "-1" if the label does not indicate a real item
 */
int label_to_inven(int c)
{
    int i = c - 'a';

    /* Verify the index */
    if ((i < 0) || (i >= inven_ctr)) return (-1);

    /* Return the index */
    return (i);
}

/*
 * Convert a label into the index of a item in the "equip"
 * Return "-1" if the label does not indicate a real item
 */
int label_to_equip(int c)
{
    int i, k = (c - 'a');

    /* Speed -- Ignore silly labels */
    if (k < 0) return (-1);

    /* Scan the "equip" list */
    for (i = INVEN_WIELD; i <= INVEN_AUX; ++i) {
	inven_type *i_ptr = &inventory[i];
	if (i_ptr->tval == TV_NOTHING) continue;
	if (k-- == 0) return (i);
    }

    /* No object found */
    return (-1);
}


/*
 * Return a string mentioning how a given item is carried
 */
cptr mention_use(int i)
{
    register cptr p;

    /* Examine the location */
    switch (i) {
      case INVEN_WIELD: p = "Wielding"; break;
      case INVEN_HEAD:  p = "On head"; break;
      case INVEN_NECK:  p = "Around neck"; break;
      case INVEN_BODY:  p = "On body"; break;
      case INVEN_ARM:   p = "On arm"; break;
      case INVEN_HANDS: p = "On hands"; break;
      case INVEN_RIGHT: p = "On right hand"; break;
      case INVEN_LEFT:  p = "On left hand"; break;
      case INVEN_FEET:  p = "On feet"; break;
      case INVEN_OUTER: p = "About body"; break;
      case INVEN_LITE:  p = "Light source"; break;
      case INVEN_AUX:   p = "Spare weapon"; break;
      default:          p = "In pack"; break;
    }

    /* Hack -- Heavy weapon */
    if (i == INVEN_WIELD) {
	inven_type *i_ptr;
	i_ptr = &inventory[i];
	if (p_ptr->use_stat[A_STR] * 15 < i_ptr->weight) {
	    p = "Just lifting";
	}
    }

    /* Return the result */
    return (p);
}


/*
 * Return a string describing how a given item is carried. -CJS- 
 */
cptr describe_use(int i)
{
    register cptr p;

    switch (i) {
      case INVEN_WIELD: p = "wielding"; break;
      case INVEN_HEAD:  p = "wearing on your head"; break;
      case INVEN_NECK:  p = "wearing around your neck"; break;
      case INVEN_BODY:  p = "wearing on your body"; break;
      case INVEN_ARM:   p = "wearing on your arm"; break;
      case INVEN_HANDS: p = "wearing on your hands"; break;
      case INVEN_RIGHT: p = "wearing on your right hand"; break;
      case INVEN_LEFT:  p = "wearing on your left hand"; break;
      case INVEN_FEET:  p = "wearing on your feet"; break;
      case INVEN_OUTER: p = "wearing about your body"; break;
      case INVEN_LITE:  p = "using to light the way"; break;
      case INVEN_AUX:   p = "holding ready by your side"; break;
      default:          p = "carrying in your pack"; break;
    }

    /* Hack -- Heavy weapon */
    if (i == INVEN_WIELD) {
	inven_type *i_ptr;
	i_ptr = &inventory[i];
	if (p_ptr->use_stat[A_STR] * 15 < i_ptr->weight) {
	    p = "barely lifting";
	}
    }

    /* Return the result */
    return p;
}



/*** Inventory "Tagging" mechanism ***/

#ifdef TAGGER

/*
 * Find the "first" inventory object with the given "tag".
 * A "tag" is a char N appearing as "@N" in an objects inscription.
 */
int get_tag(int *com_val, char tag)
{
    int i;
    cptr s;

    /* Check every object -- XXX Teach it to skip XXX */
    for (i = 0; i < INVEN_ARRAY_SIZE; ++i) {

	/* Find a '@' */
	s = strchr (inventory[i].inscrip, '@');

	/* Did it work? */
	if (s) {

	    /* Check the tag */
	    if (s[1] == tag) {

		/* Save the actual inventory ID */
		*com_val = i;

		/* Success */
		return (TRUE);
	    }
	}
    }

    /* No such tag */
    return (FALSE);
}

#endif





/* 
 * Okay, now its time to GENERICALLY drop in the "choice" window.
 * We will start with just supporting the "inventory" choices, and
 * then add "spell selection" later.
 */


/*
 * Hack -- for printing on top of the "term", keep track of the
 * column to "start" at.  This will range from 0 (full screen)
 * to about 60 (very "short" choices).  This does NOT affect
 * the "graphic choice window", which always starts at the left.
 *
 * Actually, memorize the "needed width" and indent accordingly.
 */
static int choice_wid = 20;

/*
 * Hack -- Keep track of which lines have been cleared
 */
static int choice_row = 0;


/*
 * Hooks for the GRAPHIC_RECALL functions
 */
void (*choice_fresh_hook)(void) = NULL;
void (*choice_clear_hook)(void) = NULL;
void (*choice_putstr_hook)(int, int, int, byte, cptr) = NULL;


/*
 * A simple "choice fresh" function
 */
void choice_fresh(void)
{
    /* Hook -- if allowed */
    if (use_choice_win) {
	if (choice_fresh_hook) (*choice_fresh_hook)();
	return;
    }
    
    /* Use the "Term" */
    Term_fresh();
}


/*
 * A simple "choice clear" function
 */
void choice_clear(void)
{
    /* Hook -- if allowed */
    if (use_choice_win) {
	if (choice_clear_hook) (*choice_clear_hook)();
	return;
    }
    
    /* Hack -- no rows used yet */
    choice_row = 0;

    /* Hack -- assume small width */
    choice_wid = 20;
}


/*
 * A simple "putstr()" function, for the "choice" window
 * Note that the "on-screen" version starts in line one.
 */
void choice_putstr(int x, int y, int n, byte a, cptr s)
{
    int y1 = 1;
    int x1 = (80 - choice_wid);

    /* Hook -- if allowed */
    if (use_choice_win) {
	if (choice_putstr_hook) (*choice_putstr_hook)(x, y, n, a, s);
	return;
    }

    /* Paranoia -- Legalize the left edge */
    if (x1 < 0) x1 = 0;
    
    /* Be sure we have cleared up to the line below us */
    while (choice_row < y + 2) {
	Term_erase((x1 < 2) ? 0 : (x1 - 2), choice_row + y1,
		   80-1, choice_row + y1);
	choice_row++;
    }
    
    /* Send it to the Term, offset appropriately */
    Term_putstr(x1 + x, y1 + y, n, a, s);
}



/* 
 * Displays inventory items from r1 to r2	-RAK-
 * If "weight" is set, the item weights will be displayed also
 *
 * Designed to keep the display as far to the right as possible.  -CJS-
 * The parameter col gives a column at which to start, but if the display does
 * not fit, it may be moved left.  The return value is the left edge used. 
 */
int show_inven(int r1, int r2, int weight, int col, int (*test)(int))
{
    register int i, j, k;
    register inven_type	*i_ptr;

    int          len, l, lim;
    bigvtype     tmp_val;

    int		 out_index[23];
    int8u	 out_color[23];
    char	 out_desc[23][80];


    /* Default "max-length" */
    len = 79 - col;

    /* Maximum space allowed for descriptions */
    lim = weight ? 68 : 76;

    /* Extract up to 23 lines of information */
    for (k = 0, i = r1; (k < 23) && (i <= r2); i++) {

	i_ptr = &inventory[i];

	/* Is this item acceptable? */
	if (!test || ((*test)(i_ptr->tval))) {

	    /* Describe the object, enforce max length */
	    objdes(tmp_val, i_ptr, TRUE);
	    tmp_val[lim] = '\0';

	    /* Save the object index, color, and description */
	    out_index[k] = i;
	    out_color[k] = inven_attr_by_tval(i_ptr);
	    (void)strcpy(out_desc[k], tmp_val);

	    /* Find the predicted "line length" */
	    l = strlen(out_desc[k]) + 5;

	    /* Be sure to account for the weight */
	    if (weight) l += 9;

	    /* Maintain the maximum length */
	    if (l > len) len = l;

	    /* Advance to next "line" */
	    k++;
	}
    }

    /* Find the column to start in */
    col = 79 - len;
    if (col < 0) col = 0;

    /* Clear it */
    choice_clear();
    
    /* Set the "choice" column (if new minimum) */
    choice_wid = len;
    

    /* Output each entry */
    for (j = 0; j < k; j++) {

	/* Get the index */
	i = out_index[j];

	/* Get the item */
	i_ptr = &inventory[i];

	/* Prepare an "index" --(-- */
	sprintf(tmp_val, "%c)", index_to_label(i));

	/* Clear the line with the (possibly indented) index */
	choice_putstr(0, j, -1, COLOR_WHITE, tmp_val);

	/* Display the entry itself */
	choice_putstr(3, j, -1, out_color[j], out_desc[j]);

	/* Display the weight if needed */
	if (weight) {
	    int wgt = i_ptr->weight * i_ptr->number;
	    (void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
	    choice_putstr(len-9, j, -1, COLOR_WHITE, tmp_val);
	}
    }

    /* Fresh */
    choice_fresh();

    /* Return the first column used */
    return col;
}



/*
 * Displays (all) equipment items    -RAK-
 * Keep display as far right as possible. -CJS-
 */
int show_equip(int weight, int col)
{
    register int         i, j, k;
    register inven_type *i_ptr;
    int                  x1, x2, l, len, lim;

    bigvtype             tmp_val;

    int			 out_index[23];
    int8u		 out_color[23];
    char                 out_desc[23][80];


    len = 79 - col;
    lim = weight ? 52 : 60;

    /* Scan the equipment list */
    for (k = 0, i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) {

	i_ptr = &inventory[i];

	if (i_ptr->tval != TV_NOTHING) {

	    /* Build a truncated object description */
	    objdes(tmp_val, i_ptr, TRUE);
	    tmp_val[lim] = 0;

	    /* Save the color */
	    out_index[k] = i;
	    out_color[k] = inven_attr(i_ptr);
	    (void)strcpy(out_desc[k], tmp_val);

	    /* Extract the maximal length (see below) */
	    l = strlen(out_desc[k]) + 2 + 3 + 14 + 2;
	    if (weight) l += 9;

	    /* Maintain the max-length */
	    if (l > len) len = l;

	    /* Advance the entry */
	    k++;
	}
    }

    col = 79 - len;
    if (col < 0) col = 0;

    /* Determine where the "use" gets printed */
    x1 = (col ? (col+5) : 3);

    /* Determine where the "item" gets printed */
    x2 = x1 + 14 + 2;

    /* Output each entry */
    for (j = 0; j < k; j++) {

	/* Get the index */
	i = out_index[j];

	/* Get the item */
	i_ptr = &inventory[i];

	/* If using the entire line, no need to indent */
	if (col == 0) {

	    /* Prepare a (non-indented) index --(-- */
	    sprintf(tmp_val, "%c)", index_to_label(i));
	}

	/* Indent to make a two space border */
	else {

	    /* Prepare an (indented) index --(-- */
	    sprintf(tmp_val, "  %c)", index_to_label(i));
	}

	/* Clear the line with the (possibly indented) index */
	prt(tmp_val,j+1,col);

	/* Mention the use */
	(void)sprintf(tmp_val, "%-14s: ", mention_use(i));
	put_str(tmp_val, j+1, x1);

	/* Display the entry itself */
	c_put_str(out_color[j], out_desc[j], j+1, x2);

	/* Display the weight if needed */
	if (weight) {
	    int wgt = i_ptr->weight * i_ptr->number;
	    (void)sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
	    put_str(tmp_val, j+1, 71);
	}
    }

    /* Erase the line below the list */
    erase_line(j+1,col);

    /* Return the first column used */
    return col;
}


/*
 * All inventory commands (wear, exchange, take off, drop, inventory and
 * equipment) are handled in an alternative command input mode, which accepts
 * any of the inventory commands. 
 *
 * It is intended that this function be called several times in succession, as
 * some commands take up a turn, and the rest of moria must proceed in the
 * interim. A global variable is provided, doing_inven, which is normally
 * zero; however if on return from inven_command it is expected that
 * inven_command should be called *again*, (being still in inventory command
 * input mode), then doing_inven is set to the inventory command character
 * which should be used in the next call to inven_command. 
 *
 * As long as the screen is not flushed bwteen calls, the term.c functions
 * are smart enough to not have to "blink" the screen.  If anything happens
 * to disturb() the player, the screen will be drawn, and the player will
 * be prompted to see if we should continue. This allows the player to see
 * any changes that take place on the screen during inventory command input. 
 *
 * The global variable, screen_change, is cleared by inven_command, and set
 * when "inventory" is perhaps not a wise thing to be doing.  This used to
 * be done at every screen refresh, but now it is only done by "disturb()".
 * The old method was a major hack.
 *
 * The display of inventory items is kept to the right of the screen to
 * minimize the work done to restore the screen afterwards.	-CJS- 
 */

/* Inventory command screen states. */
#define BLANK_SCR	0	/* Nothing there yet */
#define EQUIP_SCR	1	/* Equipment listing */
#define INVEN_SCR	2	/* Inventory listing */
#define WEAR_SCR	3	/* Inventory (wearable) */

/*
 * Keep track of the state of the inventory screen. 
 *
 * Note that we use "scr_saved" to allow minimal use of the save/restore
 * screen functions, and that we never "leave" inven_command() without
 * restoring the screen and unsetting scr_saved.
 */
static int scr_state, scr_left, scr_base;
static int wear_low, wear_high;


/*
 * Draw the inventory screen.
 */
static void inven_screen(int new_scr)
{
    static int scr_saved = FALSE;

    register int line = 0;


    /* Hack -- ignore "non-changes" */
    if (new_scr == scr_state) return;
    
    
    /* Hack -- allow easy "restore" */
    if (new_scr == BLANK_SCR) {

	/* Restore screen saved below */
        if (scr_saved) restore_screen();

	/* No screen "saved" */
	scr_saved = FALSE;

	/* No window displayed */
	scr_state = BLANK_SCR;

	/* Hack -- no lines */
	line = 0;
    }


    /* Enter a new mode.  Save the screen first. */
    else {

	/* Save the screen */
	if (!scr_saved) {
	    scr_saved = TRUE;
	    save_screen();
	}

	/* Save the screen type */
	scr_state = new_scr;

	/* Draw the new screen */
	switch (new_scr) {

	  case INVEN_SCR:
	    scr_left = show_inven(0, inven_ctr - 1,
				  show_inven_weight, scr_left, 0);
	    line = inven_ctr;
	    break;

	  case WEAR_SCR:
	    scr_left = show_inven(wear_low, wear_high,
				  show_inven_weight, scr_left, 0);
	    line = wear_high - wear_low + 1;
	    break;

	  case EQUIP_SCR:
	    scr_left = show_equip(show_equip_weight, scr_left);
	    line = equip_ctr;
	    break;
	}

	/* Erase as needed */
	if (line >= scr_base) {
	    scr_base = line + 1;
	    erase_line(scr_base, scr_left);
	}
	else {
	    while (++line <= scr_base) {
		erase_line(line, scr_left);
	    }
	}
    }
}


/*
 * Redraw the "choice" window contents
 */
void choice_again()
{
    /* Nothing yet */
}



#if 0 

/*
 * Display some help
 */
help()
{
    if (scr_left > 52) scr_left = 52;
    prt("  e  : list used equipment", 1, scr_left);
    prt("  i  : inventory of pack", 2, scr_left);
    prt("  t  : take off item", 3, scr_left);
    prt("  w  : wear or wield object", 4, scr_left);
    prt("  x  : exchange weapons", 5, scr_left);
    prt("  d  : drop object", 6, scr_left);
    prt("  ESC: exit", 7, scr_left);
    line = 7;
}

#endif















/*
 * Let the user select an item, return its "index"  -RAK-
 *
 * If a pair of 'i' and 'j' are given lying entirely inside the "pack",
 * then the choice will be restricted to those items.  Otherwise, there
 * are no useful restrictions, and "full" is set.  Calling this function
 * with, say, i=1 and j=24, will have unpredictable effects!
 *
 * XXX We should be able to use this function for doing "activations",
 * though it may need to be modified slightly.
 */
int get_item(int *com_val, cptr pmt, int i, int j, int (*test)(int))
{
    vtype        out_val;
    char         which;
    register int test_flag, item;
    int          full, i_scr, redraw;

    item = FALSE;
    redraw = FALSE;
    *com_val = 0;

    i_scr = 1;
    if (j >= INVEN_WIELD) {
	full = TRUE;
	if (inven_ctr == 0) {
	    i_scr = 0;
	    j = equip_ctr - 1;
	}
	else {
	    j = inven_ctr - 1;
	}
    }
    else {
	full = FALSE;
    }

    /* Only run if the player has stuff */
    if (inven_ctr > 0 || (full && equip_ctr > 0)) {

	/* Repeat until "i_scr" is set to "-1" */
	while (i_scr >= 0) {

	    /* Redraw as appropriate */
	    if (redraw) {
		if (i_scr > 0) {
		    (void)show_inven(i, j, FALSE, 80, test);
		}
		else {
		    (void)show_equip(FALSE, 80);
		}
	    }

	    /* Prepare the prompt */
	    if (full) {
		(void)sprintf(out_val,
			      "(%s: %c-%c,%s / for %s, or ESC) %s",
			     (i_scr > 0 ? "Inven" : "Equip"), i + 'a', j + 'a',
			      (redraw ? "" : " * to see,"),
			      (i_scr > 0 ? "Equip" : "Inven"), pmt);
	    }
	    else {
		(void)sprintf(out_val,
			"(Items %c-%c,%s ESC to exit) %s", i + 'a', j + 'a',
			      (redraw ? "" : " * for inventory list,"), pmt);
	    }

	    /* Show the prompt */	    
	    prt(out_val, 0, 0);

	    /* Repeat until "test_flag" gets set */
	    for (test_flag = FALSE; !test_flag; ) {

		/* Get a key, and parse it */
		which = inkey();
		switch (which) {

		  case ESCAPE:
		    test_flag = TRUE;
		    free_turn_flag = TRUE;
		    i_scr = (-1);
		    break;

		  case '/':
		    if (full) {
			if (i_scr > 0) {
			    if (equip_ctr == 0) {
				prt("But you're not using anything", 0, 0);
				c_put_str(COLOR_L_BLUE,"-more-", 0, 30);
				(void)inkey();
			    }
			    else {
				i_scr = 0;
				test_flag = TRUE;
				if (redraw) {
				    j = equip_ctr;
				    while (j < inven_ctr) {
					j++;
					erase_line(j, 0);
				    }
				}
				j = equip_ctr - 1;
			    }
			    prt(out_val, 0, 0);
			}
			else {
			    if (inven_ctr == 0) {
				prt("But you're not carrying anything", 0, 0);
				c_put_str(COLOR_L_BLUE,"-more-", 0, 33);
				(void)inkey();
			    }
			    else {
				i_scr = 1;
				test_flag = TRUE;
				if (redraw) {
				    j = inven_ctr;
				    while (j < equip_ctr) {
					j++;
					erase_line(j, 0);
				    }
				}
				j = inven_ctr - 1;
			    }
			}

		    }
		    break;

		  case '*':
		    if (!redraw) {
			test_flag = TRUE;
			save_screen();
			redraw = TRUE;
		    }
		    break;

#ifdef TAGGER
		  case '0':
		  case '1': case '2': case '3':
		  case '4': case '5': case '6':
		  case '7': case '8': case '9':

		    /* Look up that tag */
		    if (get_tag(com_val, which) &&
			(full || ((i <= *com_val) && (*com_val <= j)))) {

			/* Stop asking for keys */
			test_flag = TRUE;

			/* An item has been found */
			item = TRUE;

			/* Forget what screen we were on */
			i_scr = (-1);
		    }

		    /* Illegal tag */
		    else {
			bell();
		    }

		    break;

#endif

		  default:

		    /* Letter (with query) */
		    if (isupper((int)which)) {
			*com_val = which - 'A';
		    }

		    /* Letter (without query) */
		    else if (islower(which)) {
			*com_val = which - 'a';
		    }

		    /* Illegal entry */
		    else {
			*com_val = j + 1;
		    }

		    /* Verify the entry */
		    if ((*com_val >= i) && (*com_val <= j)) {
			if (i_scr == 0) {
			    i = 21;
			    j = *com_val;
			    do {
				while (inventory[++i].tval == TV_NOTHING);
				j--;
			    }
			    while (j >= 0);
			    *com_val = i;
			}
			if (isupper((int)which) && !verify("Try", *com_val)) {
			    test_flag = TRUE;
			    free_turn_flag = TRUE;
			    i_scr = (-1);
			    break;
			}
			test_flag = TRUE;
			item = TRUE;
			i_scr = (-1);
		    }
		    else {
			bell();
		    }

		    break;
		}
	    }   
	}

	/* Fix the screen if necessary */
	if (redraw) restore_screen();

	/* Erase all messages */
	erase_line(MSG_LINE, 0);
    }

    else {
	prt("You are not carrying anything.", 0, 0);
    }

    /* Return TRUE if something was picked */
    return (item);
}







/*
 * Player bonuses					-RAK-
 *
 * When an item is worn or taken off, this re-adjusts the player
 * bonuses.  Factor=1 : wear; Factor=-1 : removed  
 *
 * Only calculates properties with cumulative effect.  Properties that depend
 * on everything being worn are recalculated by calc_bonuses() -CJS - 
 */
void py_bonuses(inven_type *i_ptr, int factor)
{
    register int amount;

    /* Note that rings and such encode their "power" via "pval" */
    amount = i_ptr->pval * factor;

    /* Boost the stats */
    if (i_ptr->flags1 & TR1_STR) bst_stat(A_STR, amount);
    if (i_ptr->flags1 & TR1_INT) bst_stat(A_INT, amount);
    if (i_ptr->flags1 & TR1_WIS) bst_stat(A_WIS, amount);
    if (i_ptr->flags1 & TR1_DEX) bst_stat(A_DEX, amount);
    if (i_ptr->flags1 & TR1_CON) bst_stat(A_CON, amount);
    if (i_ptr->flags1 & TR1_CHR) bst_stat(A_CHR, amount);

    if (TR1_SEARCH & i_ptr->flags1) {
	p_ptr->srh += amount;
	p_ptr->fos -= amount;
    }
    if (TR1_STEALTH & i_ptr->flags1) {
	p_ptr->stl += amount;
    }
    if (TR1_INFRA & i_ptr->flags1) {
	p_ptr->see_infra += amount;
    }

    /* Hack -- Ignore "duplicate" effects of TWO rings of speed */
    if (TR1_SPEED & i_ptr->flags1) {

	/* Check for duplicate rings */
	if ((i_ptr->tval == TV_RING) &&
	    (i_ptr->sval == SV_RING_SPEED) &&
	    (i_ptr->pval > 0) &&
	    (inventory[INVEN_RIGHT].tval == TV_RING) &&
	    (inventory[INVEN_RIGHT].sval == SV_RING_SPEED) &&
	    (inventory[INVEN_RIGHT].pval > 0) &&
	    (inventory[INVEN_LEFT].tval == TV_RING) &&
	    (inventory[INVEN_LEFT].sval == SV_RING_SPEED) &&
	    (inventory[INVEN_LEFT].pval > 0)) {

	    int p1a = inventory[INVEN_LEFT].pval;
	    int p1b = inventory[INVEN_RIGHT].pval;

	    /* Duplicate rings have no effect */
	    amount = 0;

	    /* XXX Let the "best" ring take effect */
	    if (i_ptr->pval > MIN(p1a,p1b)) {
		amount = factor * (MAX(p1a,p1b)-MIN(p1a,p1b));
	    }
	}

	/* Apply the change */
	change_speed(-amount);
    }
}



/*
 * Recalculate the effect of all the stuff we use.   -CJS-
 * Also initialise (or reapply) race intrinsics    SM
 */
void calc_bonuses()
{
    register int32u        item_flags1, item_flags2, item_flags3;

    register inven_type   *i_ptr;
    register int           i;

    int                    old_dis_ac;


    /* Undo the old modifications to food digestion */
    if (p_ptr->slow_digest) p_ptr->food_digested++;
    if (p_ptr->regenerate) p_ptr->food_digested -= 3;

    /* Clear all the flags */
    p_ptr->see_inv = FALSE;
    p_ptr->teleport = FALSE;
    p_ptr->free_act = FALSE;
    p_ptr->slow_digest = FALSE;
    p_ptr->aggravate = FALSE;
    p_ptr->sustain_str = FALSE;
    p_ptr->sustain_int = FALSE;
    p_ptr->sustain_wis = FALSE;
    p_ptr->sustain_con = FALSE;
    p_ptr->sustain_dex = FALSE;
    p_ptr->sustain_chr = FALSE;
    p_ptr->resist_fire = FALSE;
    p_ptr->resist_acid = FALSE;
    p_ptr->resist_cold = FALSE;
    p_ptr->regenerate = FALSE;
    p_ptr->resist_elec = FALSE;
    p_ptr->ffall = FALSE;
    p_ptr->resist_pois = FALSE;
    p_ptr->hold_life = FALSE;
    p_ptr->telepathy = FALSE;
    p_ptr->immune_fire = FALSE;
    p_ptr->immune_acid = FALSE;
    p_ptr->immune_pois = FALSE;
    p_ptr->immune_cold = FALSE;
    p_ptr->immune_elec = FALSE;
    p_ptr->lite = FALSE;
    p_ptr->resist_conf = FALSE;
    p_ptr->resist_sound = FALSE;
    p_ptr->resist_lite = FALSE;
    p_ptr->resist_dark = FALSE;
    p_ptr->resist_chaos = FALSE;
    p_ptr->resist_disen = FALSE;
    p_ptr->resist_shards = FALSE;
    p_ptr->resist_nexus = FALSE;
    p_ptr->resist_blind = FALSE;
    p_ptr->resist_nether = FALSE;
    p_ptr->resist_fear = FALSE;

    /* Race based special abilities */
    if (p_ptr->prace == 2) p_ptr->resist_lite = TRUE;
    if (p_ptr->prace == 3) p_ptr->sustain_dex = TRUE;
    if (p_ptr->prace == 4) p_ptr->free_act = TRUE;
    if (p_ptr->prace == 5) p_ptr->resist_blind = TRUE;
    if (p_ptr->prace == 6) p_ptr->resist_dark = TRUE;
    if (p_ptr->prace == 7) p_ptr->sustain_str = TRUE;
    if (p_ptr->prace == 8) p_ptr->sustain_con = TRUE;
    if (p_ptr->prace == 9) p_ptr->ffall = TRUE;
    if (p_ptr->prace == 9) p_ptr->see_inv = TRUE;

    old_dis_ac = p_ptr->dis_ac;
    p_ptr->ptohit = tohit_adj();   /* Real To Hit   */
    p_ptr->ptodam = todam_adj();   /* Real To Dam   */
    p_ptr->ptoac = toac_adj();	   /* Real To AC    */
    p_ptr->pac = 0;		   /* Real AC	     */
    p_ptr->dis_th = p_ptr->ptohit; /* Display To Hit	    */
    p_ptr->dis_td = p_ptr->ptodam; /* Display To Dam	    */
    p_ptr->dis_ac = 0;		   /* Display AC		 */
    p_ptr->dis_tac = p_ptr->ptoac; /* Display To AC	    */


    /* Scan the usable inventory (Do NOT check INVEN_AUX) */
    for (i = INVEN_WIELD; i <= INVEN_LITE; i++) {

	i_ptr = &inventory[i];
	if (i_ptr->tval == TV_NOTHING) continue;

	/* XXX Cursed objects do not assist armor class? */
	/* if (!cursed_p(i_ptr)) { ... }   They do now */

	p_ptr->pac += i_ptr->ac;
	p_ptr->dis_ac += i_ptr->ac;


	/* Apply the real bonuses */
	p_ptr->ptoac += i_ptr->toac;
	p_ptr->ptohit += i_ptr->tohit;
	p_ptr->ptodam += i_ptr->todam;

	/* Apply the mental bonuses, if known */
	if (known2_p(i_ptr)) {
	    p_ptr->dis_tac += i_ptr->toac;
	    p_ptr->dis_th += i_ptr->tohit;
	    p_ptr->dis_td += i_ptr->todam;
	}
    }


    /* Re-Check the "wielded weapon" */
    i_ptr = &inventory[INVEN_WIELD];

    /* XXX Hack -- Give very special treatment to the "plusses" */
    /* on bows, since they do NOT help the player club monsters */
    /* with the bow, but are instead applied when firing missiles */
    /* Since they were applied above, remove them down here */
    if (i_ptr->tval == TV_BOW) {

	/* Undo the real bonuses */
	p_ptr->ptohit -= i_ptr->tohit;
	p_ptr->ptodam -= i_ptr->todam;

	/* Undo the mental bonuses, if known */
	if (known2_p(i_ptr)) {

	    p_ptr->dis_th -= i_ptr->tohit;
	    p_ptr->dis_td -= i_ptr->todam;
	}
    }

    /* Priest weapon penalty for edged weapons */
    if ((p_ptr->pclass == 2) &&
	(i_ptr->tval == TV_SWORD || i_ptr->tval == TV_POLEARM)) {

	/* Non-blessed blades are bad for priests */
	if (!(i_ptr->flags3 & TR3_BLESSED)) {

	    /* Reduce the real bonuses */
	    p_ptr->ptohit -= 2;
	    p_ptr->ptodam -= 2;

	    /* Reduce the mental bonuses */
	    p_ptr->dis_th -= 2;
	    p_ptr->dis_td -= 2;
	}

	/* Hack -- If blessed, but unknown, decrease the mental bonuses */
	else if (!known2_p(i_ptr)) {

	    /* Reduce the mental bonuses */
	    p_ptr->dis_th -= 2;
	    p_ptr->dis_td -= 2;
	}
    }

    /* Weapon WAS too heavy, all better now */
    if (weapon_heavy) {
	p_ptr->dis_th += (p_ptr->use_stat[A_STR] * 15 - i_ptr->weight);
    }

    /* Undo the effects of "stun" */
    if (p_ptr->stun > 50) {
	p_ptr->ptohit -= 20;
	p_ptr->dis_th -= 20;
	p_ptr->ptodam -= 20;
	p_ptr->dis_td -= 20;
    }
    else if (p_ptr->stun > 0) {
	p_ptr->ptohit -= 5;
	p_ptr->dis_th -= 5;
	p_ptr->ptodam -= 5;
	p_ptr->dis_td -= 5;
    }

    /* Add in temporary spell increases */
    /* these changed from pac to ptoac, since mana now affected by */
    /* high pac (to simulate encumberence), and these really should */
    /* be magical bonuses anyway -CFT */

    if (p_ptr->invuln > 0) {
	p_ptr->ptoac += 100;
	p_ptr->dis_tac += 100;
    }

    /* Temporary blessing */
    if (p_ptr->blessed > 0) {
	p_ptr->ptoac += 5;
	p_ptr->dis_tac += 5;
	p_ptr->ptohit += 10;
	p_ptr->dis_th += 10;
    }

    /* Temprory shield */
    if (p_ptr->shield > 0) {
	p_ptr->ptoac += 50;
	p_ptr->dis_tac += 50;
    }

    /* Temporary "Hero" */
    if (p_ptr->hero > 0) {
	p_ptr->ptohit += 12;
	p_ptr->dis_th += 12;
    }

    /* Temporary "Beserk" */
    if (p_ptr->shero > 0) {
	p_ptr->ptohit += 24;
	p_ptr->dis_th += 24;
	p_ptr->ptoac -= 10;	   /* berserk, so not being careful... -CFT */
	p_ptr->dis_tac -= 10;
    }

    /* Temporary see invisible */
    if (p_ptr->detect_inv > 0) {
	p_ptr->see_inv = TRUE;
    }

    /* moved from above, so it will show ac adjustments from spells... -CFT */
    p_ptr->dis_ac += p_ptr->dis_tac;

    /* Be sure to show armor changes when we update status */
    p_ptr->status |= PY_ARMOR;

    /* Check the item flags */
    item_flags1 = item_flags2 = item_flags3 = 0L;
    for (i = INVEN_WIELD; i <= INVEN_LITE; i++) {
	i_ptr = &inventory[i];
	item_flags1 |= i_ptr->flags1;
	item_flags2 |= i_ptr->flags2;
	item_flags3 |= i_ptr->flags3;
    }

    /* Process the item flags */
    if (TR2_RES_FIRE & item_flags2) p_ptr->resist_fire = TRUE;
    if (TR2_RES_ACID & item_flags2) p_ptr->resist_acid = TRUE;
    if (TR2_RES_COLD & item_flags2) p_ptr->resist_cold = TRUE;
    if (TR2_RES_POIS & item_flags2) p_ptr->resist_pois = TRUE;
    if (TR2_HOLD_LIFE & item_flags2) p_ptr->hold_life = TRUE;
    if (TR2_IM_FIRE & item_flags2) p_ptr->immune_fire = TRUE;
    if (TR2_IM_ACID & item_flags2) p_ptr->immune_acid = TRUE;
    if (TR2_IM_COLD & item_flags2) p_ptr->immune_cold = TRUE;
    if (TR2_IM_ELEC & item_flags2) p_ptr->immune_elec = TRUE;
    if (TR2_IM_POIS & item_flags2) p_ptr->immune_pois = TRUE;
    if (TR2_FREE_ACT & item_flags2) p_ptr->free_act = TRUE;
    if (TR2_RES_ELEC & item_flags2) p_ptr->resist_elec = TRUE;
    if (TR2_RES_CONF & item_flags2) p_ptr->resist_conf = TRUE;
    if (TR2_RES_SOUND & item_flags2) p_ptr->resist_sound = TRUE;
    if (TR2_RES_LITE & item_flags2) p_ptr->resist_lite = TRUE;
    if (TR2_RES_DARK & item_flags2) p_ptr->resist_dark = TRUE;
    if (TR2_RES_CHAOS & item_flags2) p_ptr->resist_chaos = TRUE;
    if (TR2_RES_DISEN & item_flags2) p_ptr->resist_disen = TRUE;
    if (TR2_RES_SHARDS & item_flags2) p_ptr->resist_shards = TRUE;
    if (TR2_RES_NEXUS & item_flags2) p_ptr->resist_nexus = TRUE;
    if (TR2_RES_BLIND & item_flags2) p_ptr->resist_blind = TRUE;
    if (TR2_RES_NETHER & item_flags2) p_ptr->resist_nether = TRUE;

    if (TR3_SLOW_DIGEST & item_flags3) p_ptr->slow_digest = TRUE;
    if (TR3_AGGRAVATE & item_flags3) p_ptr->aggravate = TRUE;
    if (TR3_TELEPORT & item_flags3) p_ptr->teleport = TRUE;
    if (TR3_REGEN & item_flags3) p_ptr->regenerate = TRUE;
    if (TR3_TELEPATHY & item_flags3) p_ptr->telepathy = TRUE;
    if (TR3_LITE & item_flags3) p_ptr->lite = TRUE;
    if (TR3_SEE_INVIS & item_flags3) p_ptr->see_inv = TRUE;
    if (TR3_FEATHER & item_flags3) p_ptr->ffall = TRUE;

    /* New method for sustaining stats */
    if (item_flags2 & TR2_SUST_STR) p_ptr->sustain_str = TRUE;
    if (item_flags2 & TR2_SUST_INT) p_ptr->sustain_int = TRUE;
    if (item_flags2 & TR2_SUST_WIS) p_ptr->sustain_wis = TRUE;
    if (item_flags2 & TR2_SUST_DEX) p_ptr->sustain_dex = TRUE;
    if (item_flags2 & TR2_SUST_CON) p_ptr->sustain_con = TRUE;
    if (item_flags2 & TR2_SUST_CHR) p_ptr->sustain_chr = TRUE;



    /* Slow digestion takes less food */
    if (p_ptr->slow_digest) p_ptr->food_digested--;

    /* Regeneration takes food */
    if (p_ptr->regenerate) p_ptr->food_digested += 3;

    /* Recalculate the mana */
    if (class[p_ptr->pclass].spell == MAGE) {
	calc_mana(A_INT);
    }
    else if (class[p_ptr->pclass].spell == PRIEST) {
	calc_mana(A_WIS);
    }
}



/*
 * Move item from equipment list to backpack
 * Currently, only one item at a time can be wielded per slot.
 */
static void inven_takeoff(int item_val, int amt)
{
    register inven_type *i_ptr;
    int			posn;
    bigvtype            out_val, prt2;
    inven_type		tmp_obj;

    /* What we are doing with the object */
    cptr act;


    /* Get the item to take off */
    i_ptr = &inventory[item_val];

    /* Paranoia */
    if (amt <= 0) return;

    /* Verify */
    if (amt > i_ptr->number) amt = i_ptr->number;

    /* Make a copy to carry */
    tmp_obj = *i_ptr;
    tmp_obj.number = amt;

    /* What are we "doing" with the object */
    if (amt < i_ptr->number) {
	act = "Took off ";
    }
    else if (item_val == INVEN_WIELD || item_val == INVEN_AUX) {
	act = "Was wielding ";
    }
    else if (item_val == INVEN_LITE) {
	act = "Light source was ";
    }
    else {
	act = "Was wearing ";
    }

    /* Carry the object, saving the slot it went in */
    posn = inven_carry(&tmp_obj);

    /* Describe the result */
    objdes(prt2, i_ptr, TRUE);
    (void)sprintf(out_val, "%s%s. (%c)", act, prt2, 'a' + posn);
    msg_print(out_val);

    /* Delete (part of) it */
    inven_item_increase(item_val, -amt);
    inven_item_optimize(item_val);
}


/*
 * Used to verify if this really is the item we wish to wear or read.
 */
int verify(cptr prompt, int item)
{
    bigvtype out_str, object;

    objdes(object, &inventory[item], TRUE);
    (void)sprintf(out_str, "%s %s? ", prompt, object);
    return (get_check(out_str));
}



/*
 * Drops (some of) an item from inventory to current location
 */
static void inven_drop(int item_val, int amt)
{
    register inven_type *i_ptr;
    int                  i;
    vtype                prt2;
    bigvtype             prt1;

    /* What we are doing with the object */
    cptr act;

    /* Access the slot to be dropped */
    i_ptr = &inventory[item_val];

    /* Error check */
    if (amt <= 0) return;

    /* Not too many */
    if (amt > i_ptr->number) amt = i_ptr->number;

    /* Nothing done? */
    if (amt <= 0) return;

    /* What are we "doing" with the object */
    if (amt < i_ptr->number) {
	act = "Dropped ";
    }
    else if (item_val == INVEN_WIELD || item_val == INVEN_AUX) {
	act = "Was wielding ";
    }
    else if (item_val == INVEN_LITE) {
	act = "Light source was ";
    }
    else if (item_val >= INVEN_WIELD) {
	act = "Was wearing ";
    }
    else {
	act = "Dropped ";
    }

    /* Paranoia -- Delete anything already here */
    delete_object(char_row, char_col);

    /* Make a new dungeon object */
    i = i_pop();
    i_list[i] = *i_ptr;
    i_list[i].number = amt;
    i_list[i].iy = char_row;
    i_list[i].ix = char_col;
    cave[char_row][char_col].i_idx = i;

    /* Message */
    objdes(prt1, &i_list[i], TRUE);
    (void)sprintf(prt2, "%s%s.", act, prt1);
    msg_print(prt2);

    /* Decrease the item, optimize. */
    inven_item_increase(item_val, -amt);
    inven_item_describe(item_val);
    inven_item_optimize(item_val);
}


/*
 * Given an item, find a slot to wield it in
 */
static int wield_slot(inven_type *i_ptr)
{
    /* Slot for equipment */
    switch (i_ptr->tval) {

	case TV_SHOT: case TV_BOLT: case TV_ARROW:
	case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
	case TV_BOW: case TV_DIGGING:
	    return (INVEN_WIELD);

	case TV_LITE:
	    return (INVEN_LITE);

	case TV_BOOTS:
	    return (INVEN_FEET);

	case TV_GLOVES:
	    return (INVEN_HANDS);

	case TV_CLOAK:
	    return (INVEN_OUTER);

	case TV_HELM:
	    return (INVEN_HEAD);

	case TV_SHIELD:
	    return (INVEN_ARM);

	case TV_DRAG_ARMOR:
	case TV_HARD_ARMOR:
	case TV_SOFT_ARMOR:
	    return (INVEN_BODY);

	case TV_AMULET:
	    return (INVEN_NECK);

	case TV_RING:

	    if (inventory[INVEN_RIGHT].tval== TV_NOTHING) return (INVEN_RIGHT);
	    if (inventory[INVEN_LEFT].tval == TV_NOTHING) return (INVEN_LEFT);

	    while (1) {

		char query;
		int slot = -1;

		get_com("Put ring on which hand (l/r/L/R)?", &query);

		if (query == ESCAPE) return (-1);

		if (query == 'l') return (INVEN_LEFT);
		if (query == 'r') return (INVEN_RIGHT);

		if (query == 'L') slot = INVEN_LEFT;
		if (query == 'R') slot = INVEN_RIGHT;

		if (slot >= 0) {
		    if (!verify("Replace", slot)) return (-1);
		    return (slot);
		}

		bell();
	    }
    }

    /* Weird request */
    msg_print("You can't wear/wield that item!");

    /* No slot available */
    return (-1);
}


/*
 * This does all the work. -- see dungeon.c for sample calls
 * But all of the "display" is done by the "choice" routines.
 */
void inven_command(int command)
{
    register int         i, slot = -1, item = -1;
    int                  tmp, selecting, from, to;
    int			 amt, chk;
    const char          *prompt, *swap, *disp, *string;
    char                 which;
    bigvtype             prt1, prt2;
    register inven_type *i_ptr;
    inven_type           tmp_obj;
    int			 changed_lite;

    static int		 last_scr = BLANK_SCR;
    

    /* Assume this will be free */
    free_turn_flag = TRUE;


    /* Hack -- clear the top line (flushes old messages) */
    prt("", 0, 0);


    /* Take up where we left off after a previous inventory command. -CJS- */
    if (doing_inven) {

	/* If something has happened, we need to be sure the user wants */
	/* to continue (unless the command is ' '), and if he does, we */
	/* must redraw our screens (hopefully made efficient in term.c) */
	/* Note that only "disturb()" sets screen_change.  Note also that */
	/* we always do a full redraw, but usually, it is pretty efficient. */
	
	/* Hack -- (see disturb()) -- allow user to stop now */
	if (screen_change) {

	    if (command == ' ' ||
		!get_check("Continue with inventory command?")) {
		doing_inven = FALSE;
		return;
	    }

	    scr_left = 50;
	    scr_base = 0;
	}

	/* Re-enter the old "mode" */
	scr_state = BLANK_SCR;
	inven_screen(last_scr);
    }

    else {

	/* Default left edge */
	scr_left = 50;

	/* No lines used yet */
	scr_base = 0;

	/* Hack -- see below */
	scr_state = BLANK_SCR;
    }


    /* Hack -- Notice if the lite "changes" */
    changed_lite = FALSE;


    /* Process commands (starting with the given one) */
    while (command != ESCAPE) {

	/* Allow use of uppercase commands */
	if (isupper(command)) command = tolower(command);

	/* Assume no "list of options" displayed yet */
	selecting = FALSE;

	/* Parse the command */
	switch (command) {

	  /* View inventory */        
	  case 'i':
	    if (inven_ctr == 0) {
		msg_print("You are not carrying anything.");
	    }
	    else {
		inven_screen(INVEN_SCR);
	    }
	    break;

	  /* View equipment */            
	  case 'e':
	    if (equip_ctr == 0) {
		msg_print("You are not using any equipment.");
	    }
	    else {
		inven_screen(EQUIP_SCR);
	    }
	    break;

	  /* Take something off */
	  case 't':
	    if (equip_ctr == 0) {
		msg_print("You are not using any equipment.");
	    }
	    else if (inven_ctr >= INVEN_WIELD && !doing_inven) {
		msg_print("You will have to drop something first.");
	    }
	    else {
		if (scr_state != BLANK_SCR) {
		    inven_screen(EQUIP_SCR);
		}
		selecting = TRUE;
	    }
	    break;

	  /* Drop something (not in shop) */            
	  case 'd':
	    if (!inven_ctr && !equip_ctr) {
		msg_print("But you're not carrying anything.");
	    }
	    else if (cave[char_row][char_col].i_idx != 0) {
		msg_print("There's no room to drop anything here.");
	    }
	    else {

		selecting = TRUE;
		if ((scr_state == EQUIP_SCR && equip_ctr) || !inven_ctr) {

		    /* Show a screen if needed */
		    if (scr_state != BLANK_SCR) inven_screen(EQUIP_SCR);

		    /* Use "remove" instead of "drop" */
		    command = 'r';
		}

		/* Show a screen if needed */
		else if (scr_state != BLANK_SCR) inven_screen(INVEN_SCR);
	    }
	    break;

	  /* Wear/wield */
	  case 'w':

	    /* Assume nothing is wearable */
	    wear_low = 99;
	    wear_high = -1;

	    /* Determine what can be worn */
	    for (i = 0; i < inven_ctr; i++) {
		if (wearable_p(&inventory[i])) {
		    if (wear_low > i) wear_low = i;
		    if (wear_high < i) wear_high = i;
		}
	    }

	    if (wear_low > wear_high) {
		msg_print("You have nothing to wear or wield.");
	    }
	    else {

		/* Note the efficiency here */
		if (scr_state != BLANK_SCR && scr_state != INVEN_SCR) {
		    inven_screen(WEAR_SCR);
		}

		/* Select something to wear */
		selecting = TRUE;
	    }
	    break;

	  /* Exchange main and aux weapons */            
	  case 'x':
	    if (inventory[INVEN_WIELD].tval == TV_NOTHING &&
		inventory[INVEN_AUX].tval == TV_NOTHING) {
		msg_print("But you are wielding no weapons.");
	    }
	    else if (cursed_p(&inventory[INVEN_WIELD])) {
		objdes(prt1, &inventory[INVEN_WIELD], FALSE);
		(void)sprintf(prt2,
		     "The %s you are wielding appears to be cursed.", prt1);
		msg_print(prt2);
	    }
	    else {
		free_turn_flag = FALSE;

		/* Subtract bonuses */
		py_bonuses(&inventory[INVEN_WIELD], -1);

		/* Swap the items */
		tmp_obj = inventory[INVEN_AUX];
		inventory[INVEN_AUX] = inventory[INVEN_WIELD];
		inventory[INVEN_WIELD] = tmp_obj;

		/* Add bonuses    */
		py_bonuses(&inventory[INVEN_WIELD], 1);

		/* Redraw "equip" screen if needed */
		if (scr_state == EQUIP_SCR) {
		    scr_left = show_equip(show_equip_weight, scr_left);
		}

		/* Take note of primary weapon */
		if (inventory[INVEN_WIELD].tval != TV_NOTHING) {
		    objdes(prt2, &inventory[INVEN_WIELD], TRUE);
		    message("Primary weapon: ", 0x02);
		    message(prt2, 0);
		}
		else {
		    msg_print("No primary weapon.");
		}

		/* Check the strength */
		check_strength();
	    }
	    break;

	  /* Dummy command to return again to main prompt. */
	  case ' ':
	    break;

	  default:
	    /* Nonsense command */
	    bell();
	    break;
	}

	/* Clear the doing_inven flag here, instead of at beginning, */
	/* so that can use it to control when messages above appear. */
	doing_inven = 0;


	/* Start with an illegal choice */
	which = '\0';

	/* Keep looking for objects to drop/wear/take off/throw off */
	while (selecting && free_turn_flag) {

	    /* Set up vars: to,from,prompt,swap */
	    swap = "";
	    if (command == 'w') {
		from = wear_low;
		to = wear_high;
		prompt = "Wear/Wield";
	    }
	    else {
		from = 0;
		if (command == 'd') {
		    to = inven_ctr - 1;
		    prompt = "Drop";
		    if (equip_ctr) swap = ", / for Equip";
		}
		else {
		    to = equip_ctr - 1;
		    if (command == 't') {
			prompt = "Take off";
		    }
		    /* command == 'r' */
		    else {
			prompt = "Throw off";
			if (inven_ctr) swap = ", / for Inven";
		    }
		}
	    }

	    /* When everything is gone */
	    if (from > to) {
		selecting = FALSE;
	    }

	    else {

		/* Assume display already done */
		disp = "";

		/* Offer the '*' option if needed */
		if (scr_state == BLANK_SCR) disp = ", * to list";

		/* Build a prompt */
		(void)sprintf(prt1,
		   "(%c-%c%s%s, space to break, ESC to exit) %s which one?",
			      from + 'a', to + 'a', disp, swap, prompt);

		/* Start without requiring a check */
		chk = FALSE;

		/* Abort everything. */
		if (!get_com(prt1, &which)) which = ESCAPE;

#ifdef TAGGER
		/* Apply the "Tagger" */
		if (which >= '0' && which <= '9') {

		    /* We are using the "inven" */
		    if ((command == 'w' || command == 'd')) {
			/* Look up the item tag in the "inven" */
			if (get_tag(&item, which) && (item < INVEN_WIELD)) {
			    which = index_to_label(item);
			}
		    }

		    /* We are using the "inven" */
		    else {
			/* XXX Look up the item tag in the "equip" */
			if (get_tag(&item, which) && (item >= INVEN_WIELD)) {
			    which = index_to_label(item);
			}
		    }
		}
#endif

		/* Handle "ESCAPE" or Abort */
		if (which == ESCAPE) {
		    selecting = FALSE;
		}

		/* Draw the screen and maybe exit to main prompt. */
		else if (which == ' ' || which == '*') {
		    if (command == 't' || command == 'r') {
			inven_screen(EQUIP_SCR);
		    }
		    else if (command == 'w' && scr_state != INVEN_SCR) {
			inven_screen(WEAR_SCR);
		    }
		    else {
			inven_screen(INVEN_SCR);
		    }
		    if (which == ' ') {
			selecting = FALSE;
		    }
		}

		/* Swap screens (for drop/remove) */
		else if (which == '/' && swap[0]) {

		    /* Swap drop/throw commands */
		    if (command == 'd') command = 'r';
		    else command = 'd';

		    /* Swap inven/equip screens */
		    if (scr_state == EQUIP_SCR) inven_screen(INVEN_SCR);
		    else if (scr_state == INVEN_SCR) inven_screen(EQUIP_SCR);
		}

		/* Hack -- Illegal item */
		else if (((which < from + 'a') || (which > to + 'a')) &&
			 ((which < from + 'A') || (which > to + 'A'))) {
		    bell();
		}

		/* Found an item */
		else {

		    /* Analyze the character */
		    chk = isupper(which);

		    /* Lowercase it if needed */
		    if (chk) which = tolower(which);

		    /* Default to item in "inven" */
		    item = label_to_inven(which);

		    /* Taking off must come from the equipment list */
		    if (command == 'r' || command == 't') {

			/* Re-extract the proper index */
			item = label_to_equip(which);

			if (item < 0) {
			    item = (-1);
			}
			else if (chk && !verify(prompt, item)) {
			    item = (-1);
			}
			else if (cursed_p(&inventory[item])) {
			    msg_print("Hmmm, it seems to be cursed.");
			    item = (-1);
			}
			else if (command == 't' &&
				 !inven_check_num(&inventory[item])) {

			    /* XXX Hack -- try to drop it instead */
			    if (cave[char_row][char_col].i_idx != 0) {
				msg_print("You can't carry or drop it.");
				item = (-1);
			    }
			    else {
				command = 'r';
				if (!get_check(
				    "You can't carry it.  Drop it?")) {
				    item = (-1);
				}
			    }
			}

			/* Item selected? */
			if (item >= 0) {

			    /* This turn is not free */
			    free_turn_flag = FALSE;

			    /* Taking off a light */
			    if (item == INVEN_LITE) changed_lite = TRUE;

			    /* Throw off an item */
			    if (command == 'r') {
				/* Throw off the "entire" item */
				inven_drop(item, 255);
				selecting = FALSE;
			    }

			    /* Take off an item */
			    else {
				/* Take off the "entire" item */
				inven_takeoff(item, 255);
			    }

			    check_strength();
			}
		    }

		    /* Wearing. Go to a bit of trouble over */
		    /* replacing existing equipment. */
		    else if (command == 'w') {

			/* Hack */
			amt = 0;

			/* Allow user to cancel */
			if (chk && !verify(prompt, item)) {
			    item = (-1);
			}
			else {
			    slot = wield_slot(&inventory[item]);
			}

			/* Prevent wielding into a cursed slot */
			if (item >= 0 && (cursed_p(&inventory[slot]))) {

			    objdes(prt1, &inventory[slot], FALSE);
			    message("The ", 0x02);
			    message(prt1, 0x02);
			    message(" you are ", 0x02);
			    message(describe_use(slot), 0x02);
			    message(" appears to be cursed.", 0x04);
			    item = (-1);
			}

			/* Determine how many items to wield */
			if (item >= 0) {

			    /* XXX Wield a "single" item */
			    amt = 1;
			}

			/* Make sure there will be room to un-wield */
			if (item >= 0 &&
			    (inventory[slot].tval != TV_NOTHING) &&
			    (inven_ctr >= INVEN_WIELD) &&
			    (inventory[item].number > amt)) {

			    int okay;

			    /* Hack -- See if there WILL be space for it */
			    /* Used for torches and arrows and daggers? */
			    inventory[item].number -= amt;
			    okay = inven_check_num(&inventory[slot]);
			    inventory[item].number += amt;

			    /* Problem? */
			    if (!okay) {
				msg_print(
				  "You will have to drop something first.");
				item = (-1);
			    }
			}

			/* OK. Wear it. */
			if (item >= 0) {

			    free_turn_flag = FALSE;

			    /* Wielding a new light */
			    if (slot == INVEN_LITE) changed_lite = TRUE;

			    /* Access the item to be wielded */
			    i_ptr = &inventory[item];

			    /* Get a copy of the object to wield */
			    tmp_obj = *i_ptr;
			    tmp_obj.number = amt;

			    /* Note how many things we have */
			    tmp = inven_ctr;

			    /* Decrease the items, delete if needed */
			    inven_item_increase(item, -amt);
			    inven_item_optimize(item);

			    /* Take note if a slot became empty */
			    if (tmp != inven_ctr) wear_high--;

			    /* Access the wield slot */
			    i_ptr = &inventory[slot];

			    /* Remove any item being worn in the slot */
			    if (i_ptr->tval != TV_NOTHING) {

				/* Remember how many things we had */
				tmp = inven_ctr;

				/* Take off the "entire" item */
				inven_takeoff(slot, 255);

				/* Take note if a slot got used */
				if (tmp != inven_ctr) wear_high++;
			    }

			    /*** Could make procedure "inven_wield()" ***/

			    /* Wear the new stuff */
			    *i_ptr = tmp_obj;

			    /* Increase the weight */
			    inven_weight += i_ptr->weight * amt;

			    /* Increment the equip counter by hand */
			    equip_ctr++;

			    /* Re-calculate bonuses (never wield into "AUX") */
			    py_bonuses(i_ptr, 1);

			    if (slot == INVEN_WIELD) {
				string = "You are wielding";
			    }
			    else if (slot == INVEN_LITE) {
				string = "Your light source is";
			    }
			    else {
				string = "You are wearing";
			    }

			    objdes(prt2, i_ptr, TRUE);
			    (void)sprintf(prt1, "%s %s. (%c)",
					  string, prt2, index_to_label(slot));
			    msg_print(prt1);

			    /* Check the strength */
			    check_strength();

			    /* Cursed! */
			    if (cursed_p(i_ptr)) {
				msg_print("Oops! It feels deathly cold!");
				i_ptr->ident |= ID_FELT;
				inscribe(i_ptr, "cursed");
			    }
			}
		    }

		    /* command == 'd' */
		    else {

			/* Get the item */
			i_ptr = &inventory[item];

			/* Assume one item */
			amt = 1;

			/* Use uppercase to verify catagory */
			/* Uppercase ignored if multiple objects */
			if (chk && !verify(prompt, item)) {
			    item = (-1);
			}

			/* Determine how many items to drop */
			if ((item >= 0) && (i_ptr->number > 1)) {

			    /* Get the quantity */
			    sprintf(prt2, "Quantity (1-%d) [%d]: ",
				    i_ptr->number, amt);
			    prt(prt2, 0, 0);
			    if (!askfor(prt1, 3)) item = -1;

			    /* Non-default choice */
			    if ((item >= 0) && prt1[0]) {
				amt = atoi(prt1);
				if (amt > i_ptr->number) amt = i_ptr->number;
				else if (amt <= 0) item = -1;
			    }
			}


			/* Actually drop */
			if (item >= 0) {

			    /* Player turn */
			    free_turn_flag = FALSE;

			    /* Drop the items */
			    inven_drop(item, amt);

			    /* Check the strength */
			    check_strength();
			}

			selecting = FALSE;
		    }

		    /* Simple "drop" yields all done */
		    if (free_turn_flag == FALSE && scr_state == BLANK_SCR) {
			selecting = FALSE;
		    }
		}
	    }
	}

	/* Escape during item choice --> Escape from command */
	if (which == ESCAPE || scr_state == BLANK_SCR) {
	    command = ESCAPE;
	}

	/* Hack -- use free_turn_flag to notice something */	
	else if (!free_turn_flag) {

	    /* Save state for recovery (see dungeon.c) */
	    if (selecting) {
		/* Remember what they are doing */
		doing_inven = command;
	    }
	    else {
		/* A dummy command (see above) to recover screen. */
		doing_inven = ' ';
	    }

	    /* Assume nothing important will happen */
	    screen_change = FALSE;

	    /* And "break" from the main loop */
	    command = ESCAPE;
	}

	/* Put an appropriate header. */
	else {

	    if (scr_state == INVEN_SCR) {
		if (!show_equip_weight || !inven_ctr) {
		    (void)sprintf(prt1, "You are carrying %d.%d pounds. In your pack there is %s",
				inven_weight / 10, inven_weight % 10,
				(inven_ctr == 0 ? "nothing." : "-"));
		}
		else {
		    (void)sprintf(prt1, "You are carrying %d.%d pounds. Your capacity is %d.%d pounds. %s",
				inven_weight / 10, inven_weight % 10,
				weight_limit() / 10, weight_limit() % 10,
				"In your pack is -");
		}
		prt(prt1, 0, 0);
	    }

	    else if (scr_state == WEAR_SCR) {
		if (wear_high < wear_low)
		    prt("You have nothing you could wield.", 0, 0);
		else
		    prt("You could wield -", 0, 0);
	    }

	    else if (scr_state == EQUIP_SCR) {
		if (equip_ctr == 0)
		    prt("You are not using anything.", 0, 0);
		else
		    prt("You are using -", 0, 0);
	    }

	    else {
		prt("Allowed commands:", 0, 0);
	    }

	    erase_line(scr_base, scr_left);
	    put_str("e/i/t/w/x/d/?/ESC:", scr_base, 60);
	    command = inkey();
	    erase_line(scr_base, scr_left);
	}
    }


    /* Hack -- save the "mode" for later */
    last_scr = scr_state;

    /* Hack -- restore the screen */
    inven_screen(BLANK_SCR);


    /* If lite changed, update it */
    if (changed_lite) update_lite();


    /* Something new may be being worn */
    calc_bonuses();

    /* If we ain't in a store, do the equippy chars -DGK */
    if (!in_store_flag) prt_equippy_chars();

    /* Paranoia -- clear weight when all objects gone */
    if (!inven_ctr && !equip_ctr) inven_weight = 0;
}




