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
    /* Indexes for "inven" are easy */
    if (i < INVEN_WIELD) return ('a' + i);

    /* Equipment always has a "constant" location */
    return ('a' + (i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven"
 * Return "-1" if the label does not indicate a real item
 */
int label_to_inven(int c)
{
    int k = c - 'a';

    /* Verify the index */
    if ((k < 0) || (k >= inven_ctr)) return (-1);

    /* Return the index */
    return (k);
}


/*
 * Convert a label into the index of a item in the "equip"
 * Return "-1" if the label does not indicate a real item
 */
int label_to_equip(int c)
{
    int k = INVEN_WIELD + (c - 'a');

    /* Speed -- Ignore silly labels */
    if (k < INVEN_WIELD) return (-1);
    if (k >= INVEN_TOTAL) return (-1);

    /* Empty slots can never be chosen */
    if (!inventory[k].tval) return (-1);

    /* Accept it */
    return (k);
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
      case INVEN_BOW:   p = "Shooting"; break;
      case INVEN_LEFT:  p = "On left hand"; break;
      case INVEN_RIGHT: p = "On right hand"; break;
      case INVEN_NECK:  p = "Around neck"; break;
      case INVEN_LITE:  p = "Light source"; break;
      case INVEN_BODY:  p = "On body"; break;
      case INVEN_OUTER: p = "About body"; break;
      case INVEN_ARM:   p = "On arm"; break;
      case INVEN_HEAD:  p = "On head"; break;
      case INVEN_HANDS: p = "On hands"; break;
      case INVEN_FEET:  p = "On feet"; break;
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
      case INVEN_BOW:   p = "shooting missiles with"; break;
      case INVEN_LEFT:  p = "wearing on your left hand"; break;
      case INVEN_RIGHT: p = "wearing on your right hand"; break;
      case INVEN_NECK:  p = "wearing around your neck"; break;
      case INVEN_LITE:  p = "using to light the way"; break;
      case INVEN_BODY:  p = "wearing on your body"; break;
      case INVEN_OUTER: p = "wearing on your back"; break;
      case INVEN_ARM:   p = "wearing on your arm"; break;
      case INVEN_HEAD:  p = "wearing on your head"; break;
      case INVEN_HANDS: p = "wearing on your hands"; break;
      case INVEN_FEET:  p = "wearing on your feet"; break;
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



#ifdef ALLOW_TAGS

/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char N appearing as "@N" anywhere in the
 * inscription of an object.
 */
int get_tag(int *com_val, char tag)
{
    int i;
    cptr s;

    /* Check every object */
    for (i = 0; i < INVEN_TOTAL; ++i) {

	/* Skip empty objects */
	if (!inventory[i].tval) continue;

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
 * Here is a "hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()".
 */
bool (*item_tester_hook)(inven_type*) = NULL;







/* 
 * Choice window "shadow" of the "show_inven()" function
 */
void choice_inven(int r1, int r2)
{
    register int i;
    bigvtype     tmp_val;


    /* In-active */
    if (!use_choice_win || !term_choice) return;

    /* Activate the choice window */
    Term_activate(term_choice);
    
    /* Clear it */
    Term_clear();

    /* Extract up to 23 lines of information */
    for (i = 0; i < inven_ctr; i++) {

	int row = i;
	
	inven_type *i_ptr = &inventory[i];

	/* Is this item "acceptable"? */
	if ((i >= r1) && (i <= r2) &&
	    (!item_tester_hook || (*item_tester_hook)(i_ptr))) {

	    /* Prepare an "index" --(-- */
	    sprintf(tmp_val, "%c)", index_to_label(i));
 
	    /* Display the index */
	    Term_putstr(0, row, 2, TERM_WHITE, tmp_val);
	}

	/* Display the entry itself */
	objdes(tmp_val, i_ptr, TRUE);
	Term_putstr(3, row, -1, inven_attr_by_tval(i_ptr), tmp_val);
    }
    
    /* Refresh */
    Term_fresh();

    /* Activate the main screen */
    Term_activate(term_screen);
}



/* 
 * Choice window "shadow" of the "show_equip()" function
 */
void choice_equip(int r1, int r2)
{
    register int i;
    bigvtype     tmp_val;


    /* In-active */
    if (!use_choice_win || !term_choice) return;

    /* Activate the choice window */
    Term_activate(term_choice);
    
    /* Clear it */
    Term_clear();

    /* Show the items */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

	int row = i - INVEN_WIELD;

	inven_type *i_ptr = &inventory[i];

	/* Is this item "acceptable"? */
	if ((i >= r1) && (i <= r2) && (i_ptr->tval) &&
	    (!item_tester_hook || (*item_tester_hook)(i_ptr))) {

	    /* Prepare an "index" --(-- */
	    sprintf(tmp_val, "%c)", index_to_label(i));
 
	    /* Display the index */
	    Term_putstr(0, row, 2, TERM_WHITE, tmp_val);
	}

	/* Display the entry itself */
	objdes(tmp_val, i_ptr, TRUE);
	Term_putstr(3, row, -1, inven_attr_by_tval(i_ptr), tmp_val);

	/* Mention the use */
	(void)sprintf(tmp_val, " <-- %s", mention_use(i));
	Term_putstr(60, row, -1, TERM_WHITE, tmp_val);
    }
    
    /* Refresh */
    Term_fresh();

    /* Activate the main screen */
    Term_activate(term_screen);
}




/*
 * Hack -- Print a list of spells in the choice window.
 * See "get_spell()" for basic algorithm.
 * We should show "empty" choices as well.
 *
 * XXX XXX XXX XXX This function probably needs some work.
 */
void choice_spell(int *spell, int num, int first)
{
    register int         i, j;
    register spell_type *s_ptr;
    int                  offset;
    cptr		 p;
    char                 spell_char;
    vtype                out_val;


    /* In-active */
    if (!use_choice_win || !term_choice) return;

    /* Activate the choice window */
    Term_activate(term_choice);
    
    /* Clear it */
    Term_clear();


    offset = (class[p_ptr->pclass].spell == MAGE ? SPELL_OFFSET : PRAYER_OFFSET);

#if 0
    erase_line(1, 0);
    put_str("Name", 1, 3);
    put_str("Lv Mana Fail", 1, 35);
#endif

    /* Paranoia -- only show the first 22 choices */
    if (num > 22) num = 22;

    /* Process the spells.  Hack -- skip unknown spells. */
    for (i = 0; i < num; i++) {

	/* Access the list of spells */
	j = spell[i];

	/* Look up that spell */
	s_ptr = &magic_spell[p_ptr->pclass - 1][j];

	/* Describe the spell "state" */
	if (j >= 32 ?
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

	/* Label the spell */
	spell_char = 'a' + j - first;

	/* Make a line of info --(-- */
	(void)sprintf(out_val, "%c) %-30s  %2d %4d %3d%%%s", spell_char,
		      spell_names[j + offset], s_ptr->slevel, s_ptr->smana,
		      spell_chance(j), p);
	Term_putstr(0, i, -1, TERM_WHITE, out_val);
    }

    /* Refresh */
    Term_fresh();

    /* Activate the main screen */
    Term_activate(term_screen);
}







/*
 * Hack -- save the number of rows output by show_inven/equip()
 */
static int show_rows = 0;


/* 
 * Displays inventory items from r1 to r2	-RAK-
 * If "weight" is set, the item weights will be displayed also
 *
 * Designed to keep the display as far to the right as possible.  -CJS-
 *
 * The parameter col gives a column at which to start, but if the display does
 * not fit, it may be moved left.  The return value is the left edge used. 
 */
void show_inven(int r1, int r2)
{
    register int i, j, k;
    register inven_type	*i_ptr;

    int          len, l, lim;
    bigvtype     tmp_val;

    int		 out_index[23];
    byte	 out_color[23];
    char	 out_desc[23][80];

    int weight = show_inven_weight;

    int col = command_gap;
    

    /* Default "max-length" */
    len = 79 - col;

    /* Maximum space allowed for descriptions */
    lim = weight ? 68 : 76;

    /* Extract up to 23 lines of information */
    for (k = 0, i = r1; (k < 23) && (i <= r2); i++) {

	i_ptr = &inventory[i];

	/* Is this item acceptable? */
	if (item_tester_hook && (!(*item_tester_hook)(i_ptr))) continue;

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

    /* Find the column to start in */
    col = (len > 76) ? 0 : (79 - len);

    /* Output each entry */
    for (j = 0; j < k; j++) {

	/* Get the index */
	i = out_index[j];

	/* Get the item */
	i_ptr = &inventory[i];

	/* Clear the line */
	prt("", j + 1, col ? col - 2 : col);

	/* Prepare an index --(-- */
	sprintf(tmp_val, "%c)", index_to_label(i));

	/* Clear the line with the (possibly indented) index */
	put_str(tmp_val, j + 1, col);

	/* Display the entry itself */
	c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

	/* Display the weight if needed */
	if (weight) {
	    int wgt = i_ptr->weight * i_ptr->number;
	    (void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
	    put_str(tmp_val, j + 1, 71);
	}
    }

    /* Erase the final line */
    prt("", j + 1, col ? col - 2 : col);

    /* Save the "rows" */
    show_rows = k;
    
    /* Save the new column */
    command_gap = col;
}



/*
 * Displays (all) equipment items    -RAK-
 * Keep display as far right as possible. -CJS-
 */
void show_equip(int s1, int s2)
{
    register int         i, j, k;
    register inven_type *i_ptr;
    int                  l, len, lim;

    bigvtype             tmp_val;

    int			 out_index[23];
    byte		 out_color[23];
    char                 out_desc[23][80];

    int weight = show_equip_weight;

    int col = command_gap;
    
    len = 79 - col;
    lim = weight ? 52 : 60;

    /* Scan the equipment list */
    for (k = 0, i = s1; i <= s2; i++) {

	i_ptr = &inventory[i];

	/* Sometimes, skip empty equipment slots */
	if (!i_ptr->tval) {
	    if (item_tester_hook) continue;
	    if (s1 > INVEN_WIELD) continue;
	    if (s2 < INVEN_TOTAL-1) continue;
	}
	
	/* Is this item acceptable? */
	if (item_tester_hook && (!(*item_tester_hook)(i_ptr))) continue;

	/* Build a truncated object description */
	objdes(tmp_val, i_ptr, TRUE);
	tmp_val[lim] = 0;

	/* Save the color */
	out_index[k] = i;
	out_color[k] = inven_attr_by_tval(i_ptr);
	(void)strcpy(out_desc[k], tmp_val);

	/* Extract the maximal length (see below) */
	l = strlen(out_desc[k]) + 2 + 3 + 14 + 2;
	if (weight) l += 9;

	/* Maintain the max-length */
	if (l > len) len = l;

	/* Advance the entry */
	k++;
    }

    /* Find a column to start in */
    col = (len > 76) ? 0 : (79 - len);

    /* Output each entry */
    for (j = 0; j < k; j++) {

	/* Get the index */
	i = out_index[j];

	/* Get the item */
	i_ptr = &inventory[i];

	/* Clear the line */
	prt("", j + 1, col ? col - 2 : col);
	
	/* Prepare an index --(-- */
	sprintf(tmp_val, "%c)", index_to_label(i));

	/* Clear the line with the (possibly indented) index */
	put_str(tmp_val, j+1, col);

	/* Mention the use */
	(void)sprintf(tmp_val, "%-14s: ", mention_use(i));
	put_str(tmp_val, j+1, col + 3);

	/* Display the entry itself */
	c_put_str(out_color[j], out_desc[j], j+1, col + 19);

	/* Display the weight if needed */
	if (weight) {
	    int wgt = i_ptr->weight * i_ptr->number;
	    (void)sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
	    put_str(tmp_val, j+1, 71);
	}
    }

    /* Save the "rows" */
    show_rows = k;
    
    /* Make a shadow below the list (if possible) */
    erase_line(j+1, col);

    /* Save the new column */
    command_gap = col;
}






/*
 * Auxiliary function for "get_item()" -- test an index
 */
static bool get_item_okay(int i)
{
    if ((i < 0) || (i >= INVEN_TOTAL)) return (FALSE);
    if (!inventory[i].tval) return (FALSE);
    if (!item_tester_hook) return (TRUE);
    if ((*item_tester_hook)(&inventory[i])) return (TRUE);
    return (FALSE);
}


/*
 * Let the user select an item, return its "index"  -RAK-
 *
 * The selected item must fall in a slot between "s1" and "s2", and must
 * satisfy the "item_tester_hook()" function, if that hook is set.
 *
 * If a legal item is selected, we save it in "com_val" and return TRUE.
 * Otherwise, we set "com_val" to "-1" and return FALSE.
 *
 * If there *are* no legal items, we return (FALSE) and set "com_val" to "-2".
 *
 * Note that "space" is a very important "response" which tells the system
 * to drop into inven/equip mode and get a new command, in some cases.
 */
int get_item(int *com_val, cptr pmt, int s1, int s2)
{
    char        n1, n2, which = ' ';
    int		k, i1, i2, e1, e2;
    bool	ver, done, item;
    bool	allow_inven, allow_equip;
    vtype       out_val;


    /* Not done */    
    done = FALSE;

    /* No item selected */
    item = FALSE;

    /* Default to "no item" */
    *com_val = -1;


    /* Hack -- see below */
    if (!command_wrk) command_see = FALSE;


    /* Determine which "pages" are allowed */
    allow_inven = (s1 < INVEN_WIELD);
    allow_equip = (s2 >= INVEN_WIELD);
    

    /* Start with "default" indexes */
    i1 = 0, i2 = inven_ctr - 1;
    e1 = INVEN_WIELD, e2 = INVEN_TOTAL-1;

    /* Allow "restrictions" on inventory/equipment */
    if (s1 > i1) i1 = s1;
    if ((s2 < INVEN_WIELD) && (s2 < i2)) i2 = s2;
    if ((s1 >= INVEN_WIELD) && (s1 > e1)) e1 = s1;
    if (s2 < e2) e2 = s2;

    /* Restrict indexes (see above) */
    while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
    while ((i1 <= i2) && (!get_item_okay(i2))) i2--;
    while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
    while ((e1 <= e2) && (!get_item_okay(e2))) e2--;
    
    /* Notice when a "page" is "empty" */
    if (allow_inven && (i1 > i2)) allow_inven = FALSE;
    if (allow_equip && (e1 > e2)) allow_equip = FALSE;
    

    /* Reset display width */
    if (!command_see) command_gap = 50;
    

    /* Hack -- Start on equipment if requested */
    if (command_wrk && command_see && !command_xxx && allow_equip) {
	n1 = 'a' + e1 - INVEN_WIELD;
	n2 = 'a' + e2 - INVEN_WIELD;
    }

    /* Use inventory if allowed */
    else if (allow_inven) {
	command_xxx = TRUE;
	n1 = 'a' + i1;
	n2 = 'a' + i2;
    }

    /* Use equipment */
    else if (allow_equip) {
	command_xxx = FALSE;
	n1 = 'a' + e1 - INVEN_WIELD;
	n2 = 'a' + e2 - INVEN_WIELD;
    }

    /* Nothing to choose from */
    else {

	/* Go back to inven/equip mode */
	if (command_wrk && command_see) {
	    command_new = command_xxx ? 'i' : 'e';
	}

	/* Hack -- cancel "see" */
	command_see = FALSE;
	
	/* Do not try to select */
	done = TRUE;
	
	/* Nothing to choose */
	*com_val = -2;
    }


    /* Hack -- start out in "display" mode */
    if (command_see) save_screen();
    
    
    /* Repeat until done */
    while (!done) {

	/* Inventory screen */
	if (command_xxx) {
	
	    /* Extract the legal requests */
	    n1 = 'a' + i1;
	    n2 = 'a' + i2;

	    /* Redraw if needed */
	    if (command_see) show_inven(i1, i2);
	    
	    /* Choice window */
	    choice_inven(i1, i2);
	}

	/* Equipment screen */
	else {

	    /* Extract the legal requests */
	    n1 = 'a' + e1 - INVEN_WIELD;
	    n2 = 'a' + e2 - INVEN_WIELD;

	    /* Redraw if needed */
	    if (command_see) show_equip(e1, e2);
	    
	    /* Choice window */
	    choice_equip(e1, e2);
	}

	/* Prepare the prompt */
	if (allow_inven && allow_equip) {
	    (void)sprintf(out_val,
			  "(%s: %c-%c,%s / for %s, or ESC) %s",
			  (command_xxx ? "Inven" : "Equip"), n1, n2,
			  (command_see ? "" : " * to see,"),
			  (command_xxx ? "Equip" : "Inven"), pmt);
	}
	else if (allow_inven) {
	    (void)sprintf(out_val,
			  "(Items %c-%c,%s ESC to exit) %s", n1, n2,
			  (command_see ? "" : " * for inventory list,"), pmt);
	}
	else {
	    (void)sprintf(out_val,
			  "(Items %c-%c,%s ESC to exit) %s", n1, n2,
			  (command_see ? "" : " * for equipment list,"), pmt);
	}

	/* Show the prompt */	    
	prt(out_val, 0, 0);


	/* Get a key */
	which = inkey();

	/* Parse it */
	switch (which) {

	  /* Cancel */
	  case ESCAPE:
	    done = TRUE;
	    break;

	  /* Hack -- see below */
	  case ' ':
	    if (command_wrk && command_see) {
		command_new = command_xxx ? 'i' : 'e';
		done = TRUE;
	    }
	    break;

	  /* Show a list of options */	    
	  case '*':
	    if (!command_see) {
	        save_screen();
		command_see = TRUE;
	    }
	    break;

	  case '/':

	    /* Hack -- no "changing pages" allowed */
	    if (!allow_inven || !allow_equip) {
		bell();
		break;
	    }
	    
	    /* Hack -- Erase old info */
	    if (command_see) {
		for (k = n1 - 'a'; k <= n2 - 'a'; k++) erase_line(k+1,0);
	    }

	    /* Switch "pages" */
	    command_xxx = (command_xxx ? FALSE : TRUE);

	    /* Need to redraw */
	    break;

#ifdef ALLOW_TAGS
	  case '0':
	  case '1': case '2': case '3':
	  case '4': case '5': case '6':
	  case '7': case '8': case '9':

	    /* XXX Look up that tag */
	    if (!get_tag(&k, which)) {
	        bell();
	        break;
	    }
	    
	    /* Tag was on the inventory */
	    if (k < INVEN_WIELD) {
		if ((k < i1) || (k > i2)) {
		    bell();
		    break;
		}
	    }

	    /* Tag was in the equipment */	    
	    else {
		if ((k < e1) || (k > e2)) {
		    bell();
		    break;
		}
	    }
	    
	    /* Validate the item */
	    if (!get_item_okay(k)) {
		bell();
		break;
	    }
	    
	    /* Use that item */
	    (*com_val) = k;
	    item = TRUE;
	    done = TRUE;
	    break;

#endif

	  default:

	    /* Extract "query" setting */
	    ver = isupper(which);
	    if (ver) which = tolower(which);

	    /* Require legal entry */
	    if ((which < n1) || (which > n2)) {
		bell();
		break;
	    }
		    
	    /* Convert letter to inventory index */
	    if (command_xxx) {
		k = label_to_inven(which);
	    }

	    /* Convert letter to equipment index */
	    else {
		k = label_to_equip(which);
	    }

	    /* Validate the item */
	    if (!get_item_okay(k)) {
		bell();
		break;
	    }
	    	    		    
	    /* Verify, abort if requested */
	    if (ver && !verify("Try", k)) {
	        which = ESCAPE;
		free_turn_flag = TRUE;
		done = TRUE;
		break;
	    }

	    /* Accept that choice */
	    (*com_val) = k;
	    item = TRUE;
	    done = TRUE;
	    break;
	}
    }


    /* Fix the screen if necessary */
    if (command_see) restore_screen();

    /* Hack -- see below */
    if (!command_wrk) command_see = FALSE;

    /* Cancel "display" on "Escape" */
    if (which == ESCAPE) command_see = FALSE;

    /* Hack -- forget the mode */
    command_wrk = FALSE;
    
    /* Forget the tester hook */
    item_tester_hook = NULL;

    /* Erase the prompt (if any) */
    erase_line(MSG_LINE, 0);

    /* Hack -- Restore the choice window (if needed) */
    choice_inven(0, inven_ctr-1);
    
    /* Return TRUE if something was picked */
    return (item);
}



/*
 * Current weapon is not priestly
 */
static int notlike = FALSE;

/*
 * Current weapon is too heavy
 */
static int heavy_weapon = FALSE;

/*
 * Current bow is too heavy
 */
static int heavy_bow = FALSE;



/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.  See also calc_mana/hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not really very
 * painful (4/5 the speed of a normal kobold).
 *
 * Note that the "bow slot" does *not* contribute to the to hit/damage
 * bonuses, since that would be silly.  Also, the normal weapon should
 * not really contribute to those values either, if you think about it,
 * but that would make it harder to penalize "heavy" or "icky" weapons.
 */
void calc_bonuses()
{
    u32b		item_flags1, item_flags2, item_flags3;

    int			i, j, old_dis_ac, old_mod[6];

    inven_type		*i_ptr;

    player_race		*rp_ptr = &race[p_ptr->prace];
    player_class	*cp_ptr = &class[p_ptr->pclass];


    /* Hack -- we have calculated the bonuses */
    p_ptr->status &= ~PY_STR_WGT;


    /* Clear all the flags */
    p_ptr->see_inv = FALSE;
    p_ptr->teleport = FALSE;
    p_ptr->free_act = FALSE;
    p_ptr->slow_digest = FALSE;
    p_ptr->aggravate = FALSE;
    p_ptr->regenerate = FALSE;
    p_ptr->ffall = FALSE;
    p_ptr->hold_life = FALSE;
    p_ptr->telepathy = FALSE;
    p_ptr->lite = FALSE;
    p_ptr->sustain_str = FALSE;
    p_ptr->sustain_int = FALSE;
    p_ptr->sustain_wis = FALSE;
    p_ptr->sustain_con = FALSE;
    p_ptr->sustain_dex = FALSE;
    p_ptr->sustain_chr = FALSE;
    p_ptr->resist_fire = FALSE;
    p_ptr->resist_acid = FALSE;
    p_ptr->resist_cold = FALSE;
    p_ptr->resist_elec = FALSE;
    p_ptr->resist_pois = FALSE;
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
    p_ptr->immune_fire = FALSE;
    p_ptr->immune_acid = FALSE;
    p_ptr->immune_pois = FALSE;
    p_ptr->immune_cold = FALSE;
    p_ptr->immune_elec = FALSE;


    /* Save the old armor class */
    old_dis_ac = p_ptr->dis_ac;

    /* Clear the stat modifiers */
    for (i = 0; i < 6; i++) {
	old_mod[i] = p_ptr->mod_stat[i];
	p_ptr->mod_stat[i] = 0;
    }
    

    /* Base searching */
    p_ptr->srh = rp_ptr->srh + cp_ptr->msrh;
    p_ptr->fos = rp_ptr->fos + cp_ptr->mfos;

    /* Base stealth */
    p_ptr->stl = rp_ptr->stl + cp_ptr->mstl;

    /* Base infravision (no class modifier) */
    p_ptr->see_infra = rp_ptr->infra;

    /* Base saving throw */
    p_ptr->save = rp_ptr->bsav + cp_ptr->msav;
    
    /* Base disarming */
    p_ptr->disarm = rp_ptr->b_dis + cp_ptr->mdis;

    /* Base to Hit */
    p_ptr->bth = rp_ptr->bth + cp_ptr->mbth;

    /* Base to Hit with a Bow */
    p_ptr->bthb = rp_ptr->bthb + cp_ptr->mbthb;

    /* Displayed/Real Bonuses */    
    p_ptr->dis_th = p_ptr->ptohit = 0;
    p_ptr->dis_td = p_ptr->ptodam = 0;
    p_ptr->dis_tac = p_ptr->ptoac = 0;
    
    /* Displayed/Real armor class */
    p_ptr->dis_ac = p_ptr->pac = 0;


    /* Start with "normal" digestion */
    p_ptr->food_digested = 2;

    /* Start with "normal" speed */
    p_ptr->pspeed = 110;


    /* Extract the current weight */
    j = inven_weight;
    
    /* Extract the "weight limit" */
    i = weight_limit();
    
    /* XXX XXX Hack -- Apply "encumbrance" */
    if (j > i/2) p_ptr->pspeed -= ((j - (i/2)) / (i / 10));



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


    /* Scan the usable inventory */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

	i_ptr = &inventory[i];

	/* Skip missing items */
	if (!i_ptr->tval) continue;

	/* Affect stats */
	if (i_ptr->flags1 & TR1_STR) p_ptr->mod_stat[A_STR] += i_ptr->pval;
	if (i_ptr->flags1 & TR1_INT) p_ptr->mod_stat[A_INT] += i_ptr->pval;
	if (i_ptr->flags1 & TR1_WIS) p_ptr->mod_stat[A_WIS] += i_ptr->pval;
	if (i_ptr->flags1 & TR1_DEX) p_ptr->mod_stat[A_DEX] += i_ptr->pval;
	if (i_ptr->flags1 & TR1_CON) p_ptr->mod_stat[A_CON] += i_ptr->pval;
	if (i_ptr->flags1 & TR1_CHR) p_ptr->mod_stat[A_CHR] += i_ptr->pval;
	
	/* Affect searching */
	if (i_ptr->flags1 & TR1_SEARCH) p_ptr->srh += i_ptr->pval;
	
	/* Also affect frequency of search */
	if (i_ptr->flags1 & TR1_SEARCH) p_ptr->fos -= i_ptr->pval;

	/* Affect stealth */
	if (i_ptr->flags1 & TR1_STEALTH) p_ptr->stl += i_ptr->pval;

	/* Affect infravision */
	if (i_ptr->flags1 & TR1_INFRA) p_ptr->see_infra += i_ptr->pval;
	
	/* Affect speed */
	if (i_ptr->flags1 & TR1_SPEED) p_ptr->pspeed += i_ptr->pval;

	/* Modify the base armor class */
	p_ptr->pac += i_ptr->ac;
	
	/* The base armor class is always known */
	p_ptr->dis_ac += i_ptr->ac;

	/* Apply the bonuses to armor class */
	p_ptr->ptoac += i_ptr->toac;

	/* Apply the mental bonuses to armor class, if known */
	if (known2_p(i_ptr)) p_ptr->dis_tac += i_ptr->toac;

	/* Hack -- do not apply "weapon" bonuses */
	if (i == INVEN_WIELD) continue;
	
	/* Hack -- do not apply "bow" bonuses */
	if (i == INVEN_BOW) continue;
	
	/* Apply the bonuses to hit/damage */
	p_ptr->ptohit += i_ptr->tohit;
	p_ptr->ptodam += i_ptr->todam;

	/* Apply the mental bonuses tp hit/damage, if known */
	if (known2_p(i_ptr)) p_ptr->dis_th += i_ptr->tohit;
	if (known2_p(i_ptr)) p_ptr->dis_td += i_ptr->todam;
    }


    /* Calculate the "total" stat values */
    for (i = 0; i < 6; i++) {

	/* Ignore non-changes */
	if (old_mod[i] == p_ptr->mod_stat[i]) continue;
	
	/* Save the new value for the stat */
	p_ptr->use_stat[i] = modify_stat(i, p_ptr->mod_stat[i]);

	/* Redisplay the stat later */
	p_ptr->status |= (PY_STR << i);
    }
    

    /* Apply temporary "stun" */
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

    /* Temporary infravision boost */
    if (p_ptr->tim_infra > 0) {
	p_ptr->see_infra++;
    }
    
    /* Temporary fast */
    if (p_ptr->fast > 0) {
	p_ptr->pspeed += 10;
    }
    
    /* Temporary slow */
    if (p_ptr->slow > 0) {
	p_ptr->pspeed -= 10;
    }
    
    /* This must be done AFTER the stuff above */
    p_ptr->dis_ac += p_ptr->dis_tac;

    /* Hack -- always redraw armor */
    p_ptr->status |= PY_ARMOR;


    /* See how much speed came from left ring */
    i_ptr = &inventory[INVEN_LEFT];
    i = (i_ptr->tval && (i_ptr->flags1 & TR1_SPEED)) ? i_ptr->pval : 0;
        
    /* See how much speed came from left ring */
    i_ptr = &inventory[INVEN_RIGHT];
    j = (i_ptr->tval && (i_ptr->flags1 & TR1_SPEED)) ? i_ptr->pval : 0;

    /* Mega-Hack -- prevent extreme speed bonus from double rings */
    if ((i > 0) && (j > 0) && (i + j > 15)) {

	/* XXX XXX Reduce the speed bonus */
	p_ptr->pspeed -= (i + j - MAX(15, MAX(i, j)));
    }


    /* Searching slows the player down */
    if (p_ptr->searching) p_ptr->pspeed -= 10;

    /* Hack -- redisplay the speed */ 
    p_ptr->status |= PY_SPEED;


    /* Check the item flags */
    item_flags1 = item_flags2 = item_flags3 = 0L;
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
	i_ptr = &inventory[i];
	item_flags1 |= i_ptr->flags1;
	item_flags2 |= i_ptr->flags2;
	item_flags3 |= i_ptr->flags3;
    }

    /* Process the item flags */
    if (TR3_SLOW_DIGEST & item_flags3) p_ptr->slow_digest = TRUE;
    if (TR3_AGGRAVATE & item_flags3) p_ptr->aggravate = TRUE;
    if (TR3_TELEPORT & item_flags3) p_ptr->teleport = TRUE;
    if (TR3_REGEN & item_flags3) p_ptr->regenerate = TRUE;
    if (TR3_TELEPATHY & item_flags3) p_ptr->telepathy = TRUE;
    if (TR3_LITE & item_flags3) p_ptr->lite = TRUE;
    if (TR3_SEE_INVIS & item_flags3) p_ptr->see_inv = TRUE;
    if (TR3_FEATHER & item_flags3) p_ptr->ffall = TRUE;
    if (TR2_FREE_ACT & item_flags2) p_ptr->free_act = TRUE;
    if (TR2_HOLD_LIFE & item_flags2) p_ptr->hold_life = TRUE;
    
    /* Immunity and resistance */
    if (TR2_IM_FIRE & item_flags2) p_ptr->immune_fire = TRUE;
    if (TR2_IM_ACID & item_flags2) p_ptr->immune_acid = TRUE;
    if (TR2_IM_COLD & item_flags2) p_ptr->immune_cold = TRUE;
    if (TR2_IM_ELEC & item_flags2) p_ptr->immune_elec = TRUE;
    if (TR2_IM_POIS & item_flags2) p_ptr->immune_pois = TRUE;
    if (TR2_RES_ACID & item_flags2) p_ptr->resist_acid = TRUE;
    if (TR2_RES_ELEC & item_flags2) p_ptr->resist_elec = TRUE;
    if (TR2_RES_FIRE & item_flags2) p_ptr->resist_fire = TRUE;
    if (TR2_RES_COLD & item_flags2) p_ptr->resist_cold = TRUE;
    if (TR2_RES_POIS & item_flags2) p_ptr->resist_pois = TRUE;
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

    /* New method for sustaining stats */
    if (item_flags2 & TR2_SUST_STR) p_ptr->sustain_str = TRUE;
    if (item_flags2 & TR2_SUST_INT) p_ptr->sustain_int = TRUE;
    if (item_flags2 & TR2_SUST_WIS) p_ptr->sustain_wis = TRUE;
    if (item_flags2 & TR2_SUST_DEX) p_ptr->sustain_dex = TRUE;
    if (item_flags2 & TR2_SUST_CON) p_ptr->sustain_con = TRUE;
    if (item_flags2 & TR2_SUST_CHR) p_ptr->sustain_chr = TRUE;


    /* Regeneration takes more food */
    if (p_ptr->regenerate) p_ptr->food_digested += 3;

    /* Slow digestion takes less food */
    if (p_ptr->slow_digest) p_ptr->food_digested--;

    /* Resting/Searching takes less food */
    if (p_ptr->rest || p_ptr->searching) p_ptr->food_digested--;

       
    /* Actual Modifier Bonuses */
    p_ptr->ptohit += tohit_adj();
    p_ptr->ptodam += todam_adj();
    p_ptr->ptoac += toac_adj();

    /* Displayed Modifier Bonuses */
    p_ptr->dis_th += tohit_adj();
    p_ptr->dis_td += todam_adj();
    p_ptr->dis_tac += toac_adj();


    /* Hack -- Recalculate hitpoints */
    calc_hitpoints();


    /* Hack -- Recalculate the spells/mana */
    if (class[p_ptr->pclass].spell == MAGE) {
	calc_spells(A_INT);
	calc_mana(A_INT);
    }
    else if (class[p_ptr->pclass].spell == PRIEST) {
	calc_spells(A_WIS);
	calc_mana(A_WIS);
    }


    /* Examine the "main weapon" */
    i_ptr = &inventory[INVEN_WIELD];


    /* Priest weapon penalty for non-blessed edged weapons */
    if ((p_ptr->pclass == 2) &&
	((i_ptr->tval == TV_SWORD) || (i_ptr->tval == TV_POLEARM)) &&
	(!(i_ptr->flags3 & TR3_BLESSED))) {

	/* Reduce the real bonuses */
	p_ptr->ptohit -= 2;
	p_ptr->ptodam -= 2;

	/* Reduce the mental bonuses */
	p_ptr->dis_th -= 2;
	p_ptr->dis_td -= 2;

	/* Notice icky weapons */
	if (!notlike) {
	    msg_print("You do not feel comfortable with your weapon.");
	    notlike = TRUE;
	}
    }

    /* Check priest for newly comfortable weapon */
    else if (notlike) {
	notlike = FALSE;
	if (i_ptr->tval) {
	    msg_print("You feel comfortable with your weapon.");
	}
	else {
	    msg_print("You feel more comfortable after removing your weapon.");
	}
    }


    /* It is hard to hit with a heavy weapon */
    if (p_ptr->use_stat[A_STR] * 15 < i_ptr->weight) {

	/* Hard to wield a heavy weapon */
	p_ptr->ptohit += (p_ptr->use_stat[A_STR] * 15 - i_ptr->weight);
	p_ptr->dis_th += (p_ptr->use_stat[A_STR] * 15 - i_ptr->weight);

	/* Notice Heavy Weapon */
	if (!heavy_weapon) {
	    msg_print("You have trouble wielding such a heavy weapon.");
	    heavy_weapon = TRUE;
	}
    }

    /* Notice disappearance of Heavy Weapon */
    else if (heavy_weapon) {
	heavy_weapon = FALSE;
	if (i_ptr->tval) {
	    msg_print("You have no trouble wielding your weapon.");
	}
	else {
	    msg_print("You feel relieved to put down your heavy weapon.");
	}
    }


#if 0

    /* Examine the "current bow" */
    i_ptr = &inventory[INVEN_BOW];

    /* It is hard to carry a heavy bow */
    if (p_ptr->use_stat[A_STR] * 15 < i_ptr->weight) {

	/* Hard to wield a heavy bow */
	p_ptr->ptohit += (p_ptr->use_stat[A_STR] * 15 - i_ptr->weight);
	p_ptr->dis_th += (p_ptr->use_stat[A_STR] * 15 - i_ptr->weight);

	/* Notice Heavy Bow */
	if (!heavy_bow) {
	    msg_print("You have trouble wielding such a heavy bow.");
	    heavy_bow = TRUE;
	}
    }

    /* Notice disappearance of Heavy Bow */
    else if (heavy_bow) {
	heavy_bow = FALSE;
	if (i_ptr->tval) {
	    msg_print("You have no trouble wielding your bow.");
	}
	else {
	    msg_print("You feel relieved to put down your heavy bow.");
	}
    }

#endif


    /* XXX XXX Check monsters if infravision/see-invis changes */
    /* XXX XXX But never do anything in a store */
    /* if (FALSE) update_monsters(); */
}



/*
 * Move an item from equipment list to pack
 * Note that only one item at a time can be wielded per slot.
 * Note that taking off an item when "full" will cause that item
 * to fall to the ground.
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
    else if (item_val == INVEN_WIELD) {
	act = "Was wielding ";
    }
    else if (item_val == INVEN_BOW) {
	act = "Was shooting with ";
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
    (void)sprintf(out_val, "%s%s. (%c)", act, prt2, index_to_label(posn));
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
 * Drops (some of) an item from inventory to "near" the current location
 */
static void inven_drop(int item_val, int amt)
{
    inven_type		*i_ptr;
    inven_type		 tmp_obj;
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

    /* Make a "fake" object */
    tmp_obj = *i_ptr;
    tmp_obj.number = amt;

    /* What are we "doing" with the object */
    if (amt < i_ptr->number) {
	act = "Dropped ";
    }
    else if (item_val == INVEN_WIELD) {
	act = "Was wielding ";
    }
    else if (item_val == INVEN_BOW) {
	act = "Was shooting with ";
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

    /* Message */
    objdes(prt1, &tmp_obj, TRUE);
    (void)sprintf(prt2, "%s%s.", act, prt1);
    msg_print(prt2);

    /* Drop it (carefully) near the player */
    drop_near(&tmp_obj, 0, char_row, char_col);

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
	    return (-1);
	    
	case TV_BOW:
	    return (INVEN_BOW);

	case TV_DIGGING: case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
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

	    if (!inventory[INVEN_RIGHT].tval) return (INVEN_RIGHT);
	    if (!inventory[INVEN_LEFT].tval) return (INVEN_LEFT);

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
 * Hack -- new interface to the "inventory" commands
 *
 * Basically, each call to this function will process some input,
 * and then return a new command to use next time.  Thus, "wear"
 * will return "w" if there is more wearing to be done, and "t"
 * will return "t" if there is more taking odd to be done.  Also,
 * "wear" will return "i" if there is nothing else to wear, but
 * we were in "selection" mode.
 *
 * Then, if, at the "what now" prompt (inventory), any key except
 * escape will return that key to the main program as the "do this"
 * command, so, for example, one can do "wear" + "space" + "inscribe"
 * and end up in "inscribe mode", with the list pre-displayed.
 *
 * This will require each command to save the screen before executing,
 * and restore it when done, but it should be worth it.  Note that all
 * commands that work through "get_item()" will already do this.
 *
 * Problem -- screen flush on "errors" (like "wear" with nothing to wear).
 *
 * Problem -- multiple commands with intervening screen flush.  Hmmm.
 * We could probably cheat and go back to the old "flush()" methods...
 *
 * Note that all commands are now "separate", that is, "i" for inventory
 * will be "inven_command_i()" and "w" for "wear" will be "inven_command_w()".
 * We may lose some screen prettiness, but fuck it.
 *
 * Note that we really do not want to "restore screen" until the
 * error messages and such have been sent...
 */
 
/*
 * Display inventory
 */
static void do_cmd_inven_i(void)
{
    char prt1[160];
    
    /* Assume this will be free */
    free_turn_flag = TRUE;

    /* Reset display */
    if (!command_see) command_gap = 50;
    
    /* Enter "display" mode */
    command_see = TRUE;
    
    /* Save the screen */
    save_screen();

    /* Note that we are in "equipment" mode */
    command_xxx = TRUE;
    
    /* Display the inventory (lines 1 to 22) */
    show_inven(0, inven_ctr - 1);

    /* Hack -- Choice window */
    choice_inven(0, inven_ctr-1);

    /* Build a prompt */
    sprintf(prt1, "Inventory (carrying %d.%d / %d.%d pounds). Command: ",
		inven_weight / 10, inven_weight % 10,
		weight_limit() / 10, weight_limit() % 10);

    /* Get a command */
    prt(prt1, 0, 0);
    command_new = inkey();

    /* Hack -- "ignore" space */
    if (command_new == ' ') {
	command_new = 'i';
    }

    /* Hack -- Process "Escape" */
    if (command_new == ESCAPE) {
	command_see = FALSE;		
	command_new = 0;
    }
    
    /* Restore the screen */
    restore_screen();
}


/*
 * Display equipment
 */
static void do_cmd_inven_e(void)
{
    char prt1[160];

    /* Assume this will be free */
    free_turn_flag = TRUE;

    /* Reset display */
    if (!command_see) command_gap = 50;
    
    /* Enter "display" mode */
    command_see = TRUE;
    
    /* Save the screen */
    save_screen();

    /* Note that we are in "equipment" mode */
    command_xxx = FALSE;
    
    /* Display the equipment (lines 1 to 12) */
    show_equip(INVEN_WIELD, INVEN_TOTAL-1);

    /* Choice window */
    choice_equip(INVEN_WIELD, INVEN_TOTAL-1);

    /* Build a prompt */
    sprintf(prt1, "Equipment (carrying %d.%d / %d.%d pounds). Command: ",
		inven_weight / 10, inven_weight % 10,
		weight_limit() / 10, weight_limit() % 10);

    /* Get a command */
    prt(prt1, 0, 0);
    command_new = inkey();

    /* Hack -- "ignore" space */
    if (command_new == ' ') {
	command_new = 'e';
    }

    /* Hack -- Process "Escape" */
    if (command_new == ESCAPE) {
	command_see = FALSE;		
	command_new = 0;
    }
    
    /* Restore the screen */
    restore_screen();
}


/*
 * The "wearable" tester
 */
static bool item_tester_hook_wear(inven_type *i_ptr)
{
    if (!wearable_p(i_ptr)) return (FALSE);
    if (i_ptr->tval == TV_BOLT) return (FALSE);
    if (i_ptr->tval == TV_ARROW) return (FALSE);
    if (i_ptr->tval == TV_SHOT) return (FALSE);
    return (TRUE);
}


/*
 * Wield or wear an item
 */
static void do_cmd_inven_w(void)
{
    int item, slot, amt;
    inven_type tmp_obj;
    inven_type *i_ptr;
    cptr location;
    char prt1[160];
    char prt2[160];


    /* Assume this will be free */
    free_turn_flag = TRUE;
    
    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Restrict the choices */
    item_tester_hook = item_tester_hook_wear;
    
    /* Get an item to wear or wield (if possible) */    
    if (get_item(&item, "Wear/Wield which item? ", 0, inven_ctr - 1)) {

	/* Assume wield one item */
	amt = 1;

	/* Check the slot */
	slot = wield_slot(&inventory[item]);

	/* No such slot (arrows and such) */
	if (slot < 0) item = -1;

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

	/* XXX XXX XXX Hack -- Verify potential overflow */
	if ((item >= 0) &&
	    (inven_ctr >= INVEN_PACK) &&
	    (inventory[item].number > amt)) {

	    /* Verify with the player */
	    if (!get_check("Your pack might overflow.  Continue?")) item = -1;
	}

	/* OK. Wear it. */
	if (item >= 0) {

	    free_turn_flag = FALSE;

	    /* Access the item to be wielded */
	    i_ptr = &inventory[item];

	    /* Get a copy of the object to wield */
	    tmp_obj = *i_ptr;
	    tmp_obj.number = amt;

	    /* Decrease the items, delete if needed */
	    inven_item_increase(item, -amt);
	    inven_item_optimize(item);

	    /* Access the wield slot */
	    i_ptr = &inventory[slot];

	    /* Take off the "entire" item if one is there */
	    if (inventory[slot].tval) inven_takeoff(slot, 255);

	    /*** Could make procedure "inven_wield()" ***/

	    /* Wear the new stuff */
	    *i_ptr = tmp_obj;

	    /* Increase the weight */
	    inven_weight += i_ptr->weight * amt;

	    /* Increment the equip counter by hand */
	    equip_ctr++;

	    /* Where is the item now */
	    if (slot == INVEN_WIELD) {
		location = "You are wielding";
	    }
	    else if (slot == INVEN_BOW) {
		location = "You are shooting with";
	    }
	    else if (slot == INVEN_LITE) {
		location = "Your light source is";
	    }
	    else {
		location = "You are wearing";
	    }

	    /* Describe the result */
	    objdes(prt2, i_ptr, TRUE);
	    (void)sprintf(prt1, "%s %s. (%c)",
			  location, prt2, index_to_label(slot));
	    msg_print(prt1);

	    /* Cursed! */
	    if (cursed_p(i_ptr)) {
		msg_print("Oops! It feels deathly cold!");
		i_ptr->ident |= ID_FELT;
		inscribe(i_ptr, "cursed");
	    }

	    /* Re-calculate bonuses */
	    calc_bonuses();

	    /* If we are not in a store, do the equippy chars -DGK */
	    if (!in_store_flag) prt_equippy_chars();
	}
    
	/* Repeat the "wear" command, if displaying choices */
	if (command_see) command_new = '[';	/* --]-- */
    }

    /* Hack -- nothing to wield */
    else if (item == -2) {
	msg_print("You have nothing you can wear or wield.");
	if (command_see) command_new = command_xxx ? 'i' : 'e';
    }
}


/*
 * Take off an item
 */
static void do_cmd_inven_t(void)
{
    int item;

    /* Assume this will be free */
    free_turn_flag = TRUE;

    /* XXX XXX XXX Hack -- verify potential overflow */
    if (inven_ctr >= INVEN_PACK) {

    	/* Verify with the player */
	if (!get_check("Your pack might overflow.  Continue?")) return;
    }
    
    /* Hack -- allow auto-see */
    command_wrk = TRUE;

    /* Get an item to take off */    
    if (get_item(&item, "Take off which item? ", INVEN_WIELD, INVEN_TOTAL-1)) {

	inven_type *i_ptr = &inventory[item];
	
	/* Not gonna happen */
	if (cursed_p(i_ptr)) {
	    msg_print("Hmmm, it seems to be cursed.");
	}

	/* Item selected? */
	else {

	    /* This turn is not free */
	    free_turn_flag = FALSE;

	    /* Take off the "entire" item */
	    inven_takeoff(item, 255);

	    /* Re-calculate bonuses */
	    calc_bonuses();

	    /* If we are not in a store, do the equippy chars -DGK */
	    if (!in_store_flag) prt_equippy_chars();
	}

	/* Repeat the "take off" command, if displaying choices --[-- */
	if (command_see) command_new = ']';
    }

    /* Nothing to take off */
    else if (item == -2) {
	msg_print("You are not wearing anything to take off.");
	if (command_see) command_new = 'i';
    }
}


/*
 * Drop an item
 */
static void do_cmd_inven_d(void)
{
    int item, amt;

    /* Assume this will be free */
    free_turn_flag = TRUE;

    /* Hack -- allow auto-see */
    command_wrk = TRUE;
    
    /* Get an item to take off */
    if (get_item(&item, "Drop which item? ", 0, INVEN_TOTAL-1)) {

	/* Get the item */
	inven_type *i_ptr = &inventory[item];

        /* Assume one item */
	amt = 1;
			
	/* Not gonna happen */
	if ((item >= INVEN_WIELD) && cursed_p(i_ptr)) {
	    msg_print("Hmmm, it seems to be cursed.");
	    item = -1;
	}

	/* See how many items */
	if ((item >= 0) && (i_ptr->number > 1)) {

	    char prt2[80];
	    char prt1[80];
	    
	    /* Get the quantity */
	    sprintf(prt2, "Quantity (1-%d) [%d]: ", i_ptr->number, amt);
	    prt(prt2, 0, 0);
	    if (!askfor(prt1, 3)) item = -1;

	    /* Non-default choice (letter means "drop all") */
	    if ((item >= 0) && prt1[0]) {
		amt = atoi(prt1);
		if (isalpha(prt1[0])) amt = 99;
		if (amt > i_ptr->number) amt = i_ptr->number;
		else if (amt <= 0) item = -1;
	    }
	}

#if 0
	/* Hack -- no room to drop here */
	if (cave[char_row][char_col].i_idx) {
	    cptr pmt = "Are you sure you want to drop something here?";
	    if (!get_check(pmt)) item = -1;
	}
#endif

	/* Actually drop */
	if (item >= 0) {

	    /* This turn is not free */
	    free_turn_flag = FALSE;

	    /* Drop (some of) the item */
	    inven_drop(item, amt);

	    /* Re-calculate bonuses */
	    calc_bonuses();

	    /* If we are not in a store, do the equippy chars -DGK */
	    if (!in_store_flag) prt_equippy_chars();
	}

	/* Again, if displaying choices */
	if (command_see) command_new = 'd';
    }

    /* Nothing to drop */
    else if (item == -2) {
	msg_print("You have nothing to drop.");
    }    
}


#if 0

/*
 * Hack -- swap weapons
 */
static void do_cmd_inven_x(void)
{
    inven_type tmp_obj;
    char prt1[160];
    char prt2[160];

    /* Assume this will be free */
    free_turn_flag = TRUE;

    /* Nothing to swap */ 
    if (!inventory[INVEN_WIELD].tval && !inventory[INVEN_AUX].tval) {
	msg_print("But you are wielding no weapons.");
    }

    /* Cursed weapon */
    else if (cursed_p(&inventory[INVEN_WIELD])) {
	objdes(prt1, &inventory[INVEN_WIELD], FALSE);
	(void)sprintf(prt2, "The %s you are wielding appears to be cursed.", prt1);
	msg_print(prt2);
    }

    /* Swap */
    else {

	/* Take a turn */
	free_turn_flag = FALSE;

	/* Swap the items */
	tmp_obj = inventory[INVEN_AUX];
	inventory[INVEN_AUX] = inventory[INVEN_WIELD];
	inventory[INVEN_WIELD] = tmp_obj;

	/* Take note of primary weapon */
	if (inventory[INVEN_WIELD].tval) {
	    objdes(prt2, &inventory[INVEN_WIELD], TRUE);
	    message("Primary weapon: ", 0x02);
	    message(prt2, 0);
	}
	else {
	    msg_print("No primary weapon.");
	}

	/* Re-calculate bonuses */
	calc_bonuses();
    }
    
    /* Hack -- Display mode */
    if (command_see) command_new = command_xxx ? 'i' : 'e';
}

#endif


/*
 * Hack -- let a single function have six effects.
 */
void inven_command(int command)
{
    /* Branch to the appropriate command */
    if (command == 'i') do_cmd_inven_i();
    else if (command == 'e') do_cmd_inven_e();
    else if (command == 'w') do_cmd_inven_w();
    else if (command == 't') do_cmd_inven_t();
    else if (command == 'd') do_cmd_inven_d();

#if 0
    else if (command == 'x') do_cmd_inven_x();
#endif

}



