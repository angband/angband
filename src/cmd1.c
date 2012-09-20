/* File: cmd1.c */

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
    cptr p;

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
    cptr p;

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
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Hack -- sometimes the tag "@xn" will work as well, where
 * "n" is a char, and "x" is the "current" command_cmd code.
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
        s = strchr(inventory[i].inscrip, '@');

        /* Process all tags */
        while (s) {

            /* Check the normal tags */
            if (s[1] == tag) {

                /* Save the actual inventory ID */
                *com_val = i;

                /* Success */
                return (TRUE);
            }

            /* Check the special tags */
            if ((s[1] == command_cmd) && (s[2] == tag)) {

                /* Save the actual inventory ID */
                *com_val = i;

                /* Success */
                return (TRUE);
            }

            /* Find another '@' */
            s = strchr(s + 1, '@');
        }
    }

    /* No such tag */
    return (FALSE);
}

#endif







/*
 * Describe number of remaining charges.		-RAK-	
 */
void inven_item_charges(int item_val)
{
    int		rem_num;
    char	out_val[80];

    if (inven_known_p(&inventory[item_val])) {
        rem_num = inventory[item_val].pval;
        (void)sprintf(out_val, "You have %d charges remaining.", rem_num);
        msg_print(out_val);
    }
}


/*
 * Describe an inventory item, in terms of its "number"
 * Ex: "You have 5 arrows (+1,+1)." or "You have no more arrows."
 */
void inven_item_describe(int item)
{
    inven_type	*i_ptr;
    char	tmp_str[160];

    i_ptr = &inventory[item];

    /* Get a description */
    objdes(tmp_str, i_ptr, TRUE);

    /* Print a message */
    message("You have ", 0x02);
    message(tmp_str, 0x02);
    message(".", 0);
}




/*
 * Increase the "number" of a given item by a given amount
 * Be sure not to exceed the legal bounds.
 * Note that this can result in an item with zero items
 * Take account of changes to the players weight.
 * Note that item is an index into the inventory.
 */
void inven_item_increase(int item, int num)
{
    int cnt;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &inventory[item];

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
        p_ptr->update |= PU_BONUS;
    }
}


/*
 * Destroy an inventory slot if it has no more items
 * Slides items if necessary, and clears out the hole
 */
void inven_item_optimize(int item)
{
    int i;
    inven_type *i_ptr;

    /* Get the item */
    i_ptr = &inventory[item];

    /* Paranoia -- be sure it exists */
    if (!i_ptr->tval) return;

    /* Only optimize if empty */
    if (i_ptr->number) return;

    /* The item is in the pack */
    if (item < INVEN_WIELD) {

        /* One less item */
        inven_ctr--;

        /* Slide later entries onto us */
        for (i = item; i < inven_ctr; i++) {
            inventory[i] = inventory[i + 1];
        }

        /* Paranoia -- erase the empty slot */
        invwipe(&inventory[inven_ctr]);
    }

    /* The item is being wielded */
    else {

        /* One less item */
        equip_ctr--;

        /* Paranoia -- erase the empty slot */
        invwipe(&inventory[item]);
    }
}



/*
 * Increase the "number" of the item below the player by a given amount
 * Be sure not to exceed the legal bounds.
 * Note that this can result in an item with zero items
 */
void floor_item_increase(int y, int x, int amt)
{
    int num;

    /* Get the item */
    inven_type *i_ptr = &i_list[cave[y][x].i_idx];

    /* Paranoia -- be sure it exists */
    if (!i_ptr->tval) return;

    /* Bounds check */
    num = i_ptr->number + amt;
    if (num > 255) num = 255;
    else if (num < 0) num = 0;

    /* Save the new number */
    i_ptr->number = num;
}


/*
 * Destroy an inventory slot if it has no more items
 * Slides items if necessary, and clears out the hole
 */
void floor_item_optimize(int y, int x)
{
    /* Get the item */
    inven_type *i_ptr = &i_list[cave[y][x].i_idx];

    /* Paranoia -- be sure it exists */
    if (!i_ptr->tval) return;

    /* Optimize if empty */
    if (i_ptr->number == 0) delete_object(y, x);
}




/*
 * Find the minimum and maximum index of items in the pack which
 * match either of the given "tval" codes.  Return TRUE if possible.
 * If no items match the given indexes, set both indexes to "-1".
 */
int find_range(int tval, int *i1, int *i2)
{
    int         i, flag = FALSE;

    /* Assume nothing */
    *i1 = (-1);
    *i2 = (-1);

    /* Check every item in the pack */
    for (i = 0; i < inven_ctr; i++) {

        /* Check the "tval" code */
        if (inventory[i].tval == tval) {

            if (!flag) *i1 = i;
            *i2 = i;
            flag = TRUE;
        }
    }

    /* Any found? */
    return (flag);
}



/*
 * Computes current weight limit.
 */
int weight_limit(void)
{
    s32b weight_cap;

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
    int i;

    /* If there is an empty space, we are fine */
    if (inven_ctr < INVEN_PACK) return (TRUE);

    /* Scan every possible match */
    for (i = 0; i < inven_ctr; i++) {

        /* Get that item */
        inven_type *j_ptr = &inventory[i];

        /* Check if the two items can be combined */
        if (item_similar(j_ptr, i_ptr)) return (TRUE);
    }

    /* And there was no room in the inn... */
    return (FALSE);
}


/* 
 * Hack -- determine the inventory slot at which an item "should" be placed
 */
static int inven_carry_aux(inven_type *i_ptr)
{
    int			i;

    s32b		i_value, j_value;

    inven_type		*j_ptr;


    /* The tval of readible books */
    int read_tval = 0;

    /* Acquire the type value of the books that the player can read, if any */
    if (cp_ptr->spell_stat == A_WIS) read_tval = TV_PRAYER_BOOK;
    else if (cp_ptr->spell_stat == A_INT) read_tval = TV_MAGIC_BOOK;


    /* Get the "value" of the item */
    i_value = item_value(i_ptr);

    /* Scan every occupied slot */
    for (i = 0; i < inven_ctr; i++) {

        /* Get the item already there */
        j_ptr = &inventory[i];

        /* Hack -- readable books always come first */
        if ((i_ptr->tval == read_tval) && (j_ptr->tval != read_tval)) break;
        if ((j_ptr->tval == read_tval) && (i_ptr->tval != read_tval)) continue;

        /* Objects sort by decreasing type */
        if (i_ptr->tval > j_ptr->tval) break;
        if (i_ptr->tval < j_ptr->tval) continue;

        /* Non-aware (flavored) items always come last */
        if (!inven_aware_p(i_ptr)) continue;
        if (!inven_aware_p(j_ptr)) break;

        /* Objects sort by increasing sval */
        if (i_ptr->sval < j_ptr->sval) break;
        if (i_ptr->sval > j_ptr->sval) continue;

        /* Unidentified objects always come last */
        if (!inven_known_p(i_ptr)) continue;
        if (!inven_known_p(j_ptr)) break;

	/* Determine the "value" of the pack item */
        j_value = item_value(j_ptr);

        /* Objects sort by decreasing value */
        if (i_value > j_value) break;
        if (i_value < j_value) continue;
    }

    /* Use that slot */
    return (i);
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
    int         slot, i;


    /* Check all the items in the pack (attempt to combine) */
    for (slot = 0; slot < inven_ctr; slot++) {

        /* Access that inventory item */
        inven_type *j_ptr = &inventory[slot];

        /* Check if the two items can be combined */
        if (item_similar(j_ptr, i_ptr)) {

            /* Add together the item counts */
            j_ptr->number += i_ptr->number;

            /* Hack -- save largest discount */
            j_ptr->discount = MAX(j_ptr->discount, i_ptr->discount);

            /* Hack -- never lose an inscription */
            if (i_ptr->inscrip[0]) inscribe(j_ptr, i_ptr->inscrip);

            /* Increase the weight */
            inven_weight += i_ptr->number * i_ptr->weight;

            /* Remember to recalculate bonuses */
            p_ptr->update |= PU_BONUS;

            /* All done, report where we put it */
            return (slot);
        }
    }


    /* Mega-Hack -- Induce "proper" over-flow */
    if (inven_ctr == INVEN_PACK) {

        /* Use the special slot */
        slot = INVEN_PACK;
    }
    
    /* Normal object insertion */
    else {

        /* Determine where to insert the item */
        slot = inven_carry_aux(i_ptr);
    
        /* Structure slide (make room) */
        for (i = inven_ctr; i > slot; i--) {

	    /* Slide the item */
            inventory[i] = inventory[i-1];
        }
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
    p_ptr->update |= PU_BONUS;

    /* Say where it went */
    return (slot);
}


/*
 * Hack -- make sure the pack is nice and clean
 * Objects can only move towards the "top" of the pack
 * Combine where possible, re-order where necessary
 *
 * Note that if the INVEN_PACK slot is occupied, we must be very
 * careful to preserve the proper "over-flow" semantics (see above).
 */
void combine_pack(void)
{
    int i, j, k;

    inven_type temp;
    
    inven_type *i_ptr, *j_ptr;

    bool	did_combine = FALSE;
    bool	did_reorder = FALSE;
    

    /* Combine the pack (backwards) */
    for (i = inven_ctr-1; i > 0; i--) {

        /* Get the item */
        i_ptr = &inventory[i];

        /* Scan the items above that item */
        for (j = 0; j < i; j++) {

            /* Get the item */
            j_ptr = &inventory[j];

            /* Can we drop "i_ptr" onto "j_ptr"? */
            if (item_similar(j_ptr, i_ptr)) {

		/* Take note */
		did_combine = TRUE;

                /* Add together the item counts */
                j_ptr->number += i_ptr->number;

                /* Hack -- save largest discount */
                j_ptr->discount = MAX(i_ptr->discount, j_ptr->discount);

                /* Hack -- maintain the "best" inscription */
                if (i_ptr->inscrip[0]) inscribe(j_ptr, i_ptr->inscrip);

                /* One object is gone */
                inven_ctr--;

                /* Slide the inventory (via structure copy) */
                for (k = i; k < inven_ctr; k++) {

                    /* Slide the objects */
                    inventory[k] = inventory[k + 1];
                }

                /* Erase the last object */
                invwipe(&inventory[k]);

		/* Combine next item */
		break;
            }
        }
    }
    

    /* Re-order the pack (forwards) */
    for (i = 0; i < inven_ctr; i++) {

        /* Get the item */
        i_ptr = &inventory[i];

	/* Mega-Hack -- allow "proper" over-flow */
	if (i == INVEN_PACK) break;
	
        /* Determine where to insert the item */
        j = inven_carry_aux(i_ptr);

        /* Never move down */
        if (j >= i) continue;

        /* Take note */
        did_reorder = TRUE;
        
        /* Save the moving item */
        temp = inventory[i];
        
        /* Structure slide (make room) */
        for (k = i; k > j; k--) {

            /* Slide the item */
            inventory[k] = inventory[k-1];
        }

	/* Insert the moved item */
	inventory[j] = temp;
    }


    /* Messages */
    if (did_combine) msg_print("You combine similar items in your pack.");
    if (did_reorder) msg_print("You reorganize some items in your pack.");
}






/*
 * Here is a "hook" used during calls to "get_item()" and
 * "show_inven()" and "show_equip()".
 */
bool (*item_tester_hook)(inven_type*) = NULL;







/*
 * Choice window "shadow" of the "show_inven()" function
 * Note that we do our best to re-use any previous contents
 */
void choice_inven(int r1, int r2)
{
    register	int i, n;
    inven_type *i_ptr;
    byte	attr = TERM_WHITE;
    char	tmp_val[160];


    /* In-active */
    if (!use_choice_win || !term_choice) return;


    /* Activate the choice window */
    Term_activate(term_choice);

    /* Display the pack */
    for (i = 0; i < inven_ctr; i++) {

        /* Examine the item */
        i_ptr = &inventory[i];

        /* Start with an empty "index" */
        tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

        /* Is this item "acceptable"? */
        if ((i >= r1) && (i <= r2) &&
            (!item_tester_hook || (*item_tester_hook)(i_ptr))) {

            /* Prepare an "index" */
            tmp_val[0] = index_to_label(i);

            /* Bracket the "index" --(-- */
            tmp_val[1] = ')';
        }

        /* Display the index (or blank space) */
        Term_putstr(0, i, 3, TERM_WHITE, tmp_val);

        /* Obtain an item description */
        objdes(tmp_val, i_ptr, TRUE);

        /* Obtain the length of the description */
        n = strlen(tmp_val);

        /* Get a color */
        if (use_color) attr = tval_to_attr[i_ptr->tval];

        /* Display the entry itself */
        Term_putstr(3, i, n, attr, tmp_val);

        /* Erase the rest of the line */
        Term_erase(3+n, i, 80, i);

        /* Display the weight if needed */
        if (choice_inven_wgt) {
            int wgt = i_ptr->weight * i_ptr->number;
            sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
        }
    }

    /* Erase the rest of the window */
    Term_erase(0, inven_ctr, 80, 24);

    /* Refresh */
    Term_fresh();

    /* Re-activate the main screen */
    Term_activate(term_screen);
}



/*
 * Choice window "shadow" of the "show_equip()" function
 * Note that we do our best to re-use any previous contents
 */
void choice_equip(int r1, int r2)
{
    register	int i, n;
    inven_type *i_ptr;
    byte	attr = TERM_WHITE;
    char	tmp_val[160];


    /* In-active */
    if (!use_choice_win || !term_choice) return;


    /* Activate the choice window */
    Term_activate(term_choice);

    /* Display the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Examine the item */
        i_ptr = &inventory[i];

        /* Start with an empty "index" */
        tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

        /* Is this item "acceptable"? */
        if ((i >= r1) && (i <= r2) && (i_ptr->tval) &&
            (!item_tester_hook || (*item_tester_hook)(i_ptr))) {

            /* Prepare an "index" */
            tmp_val[0] = index_to_label(i);

            /* Bracket the "index" --(-- */
            tmp_val[1] = ')';
        }

        /* Display the index (or blank space) */
        Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);

        /* Obtain an item description */
        objdes(tmp_val, i_ptr, TRUE);

        /* Obtain the length of the description */
        n = strlen(tmp_val);

        /* Get the color */
        if (use_color) attr = tval_to_attr[i_ptr->tval];

        /* Display the entry itself */
        Term_putstr(3, i - INVEN_WIELD, n, attr, tmp_val);

        /* Erase the rest of the line */
        Term_erase(3+n, i - INVEN_WIELD, 80, i - INVEN_WIELD);

        /* Display the slot description (if needed) */
        if (choice_equip_xtra) {
            Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
            Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, mention_use(i));
        }

        /* Display the weight (if needed) */
        if (choice_equip_wgt && i_ptr->weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            int col = choice_equip_xtra ? 52 : 71;
            sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
        }
    }

    /* Erase the rest of the window */
    Term_erase(0, INVEN_TOTAL - INVEN_WIELD, 80, 24);

    /* Refresh */
    Term_fresh();

    /* Re-activate the main screen */
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
    int		i, j, k;
    inven_type	*i_ptr;

    int		len, l, lim;
    char	tmp_val[160];

    int		out_index[23];
    byte	out_color[23];
    char	out_desc[23][80];

    int		weight = show_inven_weight;

    int		col = command_gap;


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
        out_color[k] = tval_to_attr[i_ptr->tval];
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
    int			i, j, k, l, len, lim;

    inven_type		*i_ptr;

    char		tmp_val[160];

    int			out_index[23];
    byte		out_color[23];
    char		out_desc[23][80];

    int			weight = show_equip_weight;

    int			col = command_gap;


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
        out_color[k] = tval_to_attr[i_ptr->tval];
        (void)strcpy(out_desc[k], tmp_val);

        /* Extract the maximal length (see below) */
        l = strlen(out_desc[k]) + 2 + 3 + 14 + 2;
        if (weight) l += 9;

        /* Maintain the max-length */
        if (l > len) len = l;

        /* Advance the entry */
        k++;
    }

    /* Hack -- Find a column to start in */
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
    prt("", j+1, col);

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
 * Let the user select an item, return its "index"
 *
 * The selected item must fall in a slot between "s1" and "s2", and must
 * satisfy the "item_tester_hook()" function, if that hook is set.
 *
 * If a legal item is selected, we save it in "com_val" and return TRUE.
 * If this "legal" item is on the floor, we use a "com_val" of "-1".
 *
 * Otherwise, we set return FALSE, and set "com_val" to:
 *   -1 for "User hit space/escape"
 *   -2 for "No legal items to choose"
 *
 * If the parameter "floor" is TRUE, then "-" is a legal response, and
 * indicates "the item on the floor" (com_val of "-1", return TRUE).
 *
 * Note that "space" is a very important "response" which tells the system
 * to drop into inven/equip mode, as appropriate.  Note that all commands
 * which call "get_item()" must be prepared for this possible result.
 */
int get_item(int *com_val, cptr pmt, int s1, int s2, bool floor)
{
    char        n1, n2, which = ' ';

    int		k, i1, i2, e1, e2;
    bool	ver, done, item;
    bool	allow_inven, allow_equip;

    char	out_val[160];


    /* Not done */
    done = FALSE;

    /* No item selected */
    item = FALSE;

    /* Default to "no item" (see above) */
    *com_val = -1;


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
    if (command_see && !command_xxx && allow_equip) {
        command_xxx = FALSE;
    }

    /* Use inventory if allowed */
    else if (allow_inven) {
        command_xxx = TRUE;
    }

    /* Use equipment */
    else if (allow_equip) {
        command_xxx = FALSE;
    }

    /* Use inventory (by default) for floor */
    else if (floor) {
        command_xxx = TRUE;
    }

    /* Nothing to choose from */
    else {

        /* Go back to inven/equip mode */
        if (command_see) {
            command_new = command_xxx ? 'i' : 'e';
            command_see = FALSE;
        }

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
                          "(%s: %c-%c, / for %s,%s%s or ESC) %s",
                          (command_xxx ? "Inven" : "Equip"), n1, n2,
                          (command_xxx ? "Equip" : "Inven"),
                          (command_see ? "" : " * to see,"),
                          (floor ? " -," : ""), pmt);
        }
        else if (allow_inven) {
            (void)sprintf(out_val,
                          "(Items %c-%c,%s%s ESC to exit) %s", n1, n2,
                          (command_see ? "" : " * to see,"),
                          (floor ? " - for floor," : ""), pmt);
        }
        else if (allow_equip) {
            (void)sprintf(out_val,
                          "(Items %c-%c,%s%s ESC to exit) %s", n1, n2,
                          (command_see ? "" : " * to see,"),
                          (floor ? " - for floor," : ""), pmt);
        }
        else {
            (void)sprintf(out_val,
                          "(- for floor, or ESC to exit) %s", pmt);
        }


        /* Show the prompt */	
        prt(out_val, 0, 0);


        /* Get a key */
        which = inkey();

        /* Hack -- handle "return" */
        if ((which == '\n') || (which == '\r')) {

            /* Hack -- Assume "default" response */
            if (n1 == n2) which = n1;
        }

        /* Parse it */
        switch (which) {

          /* Cancel */
          case ESCAPE:
            done = TRUE;
            break;

          /* Hack -- see below */
          case ' ':
            if (!command_see) {
                save_screen();
                command_see = TRUE;
            }
            else {
                command_new = command_xxx ? 'i' : 'e';
                done = TRUE;
            }
            break;

          /* Show a list of options */	
          case '?':
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
                for (k = n1 - 'a'; k <= n2 - 'a'; k++) prt("", k+1, 0);
            }

            /* Switch "pages" */
            command_xxx = (command_xxx ? FALSE : TRUE);

            /* Need to redraw */
            break;

          case '-':
            if (floor) {
                item = TRUE;
                done = TRUE;
            }
            else {
                bell();
            }
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
                energy_use = 0;
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

    /* Cancel "display" on "Escape" */
    if (which == ESCAPE) command_see = FALSE;

    /* Forget the tester hook */
    item_tester_hook = NULL;

    /* Clear messages (?) */
    msg_print(NULL);

    /* XXX XXX Hack -- update choice window */
    if (!choice_default) choice_inven(0, inven_ctr - 1);
    else choice_equip(INVEN_WIELD, INVEN_TOTAL - 1);


    /* Return TRUE if something was picked */
    return (item);
}




/*
 * Move an item from equipment list to pack
 * Note that only one item at a time can be wielded per slot.
 * Note that taking off an item when "full" will cause that item
 * to fall to the ground.
 */
static void inven_takeoff(int item_val, int amt)
{
    int			posn;

    inven_type		*i_ptr;
    inven_type		tmp_obj;

    cptr		act;

    char		out_val[160];
    char		prt2[160];


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
    char	out_str[160];
    char	object[160];

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

    cptr act;

    char		prt1[160];
    char		prt2[160];


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
    drop_near(&tmp_obj, 0, py, px);

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

        case TV_CROWN:
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

            /* Use the right hand first */
            if (!inventory[INVEN_RIGHT].tval) return (INVEN_RIGHT);

            /* And then the left hand */
            if (!inventory[INVEN_LEFT].tval) return (INVEN_LEFT);

            /* Use the left hand for swapping (by default) */
            if (!other_query_flag) return (INVEN_LEFT);

            /* Hack -- ask for a hand */
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
                    if (!verify("Replace", slot)) break;
                    return (slot);
                }

                bell();
            }

            /* Hack -- no selection */
            return (-1);
    }

    /* Weird request */
    msg_print("You can't wear/wield that item!");

    /* No slot available */
    return (-1);
}





/*
 * Hack -- new interface to the "inventory" commands
 *
 * Basically, the "inventory" commands are just normal commands now.
 *
 * Each of the inventory commands, if possible, sets "command_new" in
 * such a way as to enable the command to automatically repeat itself.
 *
 * Unfortunately, the "error" and "status" messages induce screen
 * refresh in between calls to "show_inven()", so we do not have as
 * "nice" of an interface as I would like.  But it is much simpler now.
 */


/*
 * Display inventory
 */
void do_cmd_inven_i(void)
{
    char prt1[160];

    /* Free command */
    energy_use = 0;

    /* Reset display */
    if (!command_see) command_gap = 50;

    /* Enter "display" mode */
    command_see = TRUE;

    /* Save the screen */
    save_screen();

    /* Note that we are in "inventory" mode */
    command_xxx = TRUE;

    /* Display the inventory */
    show_inven(0, inven_ctr - 1);

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
void do_cmd_inven_e(void)
{
    char prt1[160];

    /* Free command */
    energy_use = 0;

    /* Reset display */
    if (!command_see) command_gap = 50;

    /* Enter "display" mode */
    command_see = TRUE;

    /* Save the screen */
    save_screen();

    /* Note that we are in "equipment" mode */
    command_xxx = FALSE;

    /* Display the equipment */
    show_equip(INVEN_WIELD, INVEN_TOTAL-1);

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
 * Wield or wear a single item from the pack or floor
 */
void do_cmd_inven_w(void)
{
    int item, slot;
    bool floor, okay;
    inven_type tmp_obj;
    inven_type *i_ptr;
    cptr location;
    char prt1[160];
    char prt2[160];


    /* Assume this will be free */
    energy_use = 0;


    /* Access the item on the floor */
    i_ptr = &i_list[cave[py][px].i_idx];

    /* Check for use of the floor */
    floor = item_tester_hook_wear(i_ptr);


    /* Restrict the choices */
    item_tester_hook = item_tester_hook_wear;

    /* Get an item to wear or wield (if possible) */
    if (get_item(&item, "Wear/Wield which item? ", 0, inven_ctr - 1, floor)) {

        /* Assume okay */
        okay = TRUE;

        /* Access the item */
        if (item >= 0) i_ptr = &inventory[item];

        /* Check the slot */
        slot = wield_slot(i_ptr);

        /* No such slot (arrows and such) */
        if (slot < 0) okay = FALSE;

        /* Prevent wielding into a cursed slot */
        if (okay && (cursed_p(&inventory[slot]))) {

            objdes(prt1, &inventory[slot], FALSE);
            message("The ", 0x02);
            message(prt1, 0x02);
            message(" you are ", 0x02);
            message(describe_use(slot), 0x02);
            message(" appears to be cursed.", 0x04);
            okay = FALSE;
        }

        /* XXX XXX XXX Hack -- Verify potential overflow */
        if (okay && (inven_ctr >= INVEN_PACK) &&
            ((item < 0) || (i_ptr->number > 1))) {

            /* Verify with the player */
            if (other_query_flag &&
                !get_check("Your pack may overflow.  Continue?")) okay = FALSE;
        }

        /* OK. Wear it. */
        if (okay) {

            energy_use = 100;

            /* Get a copy of the object to wield */
            tmp_obj = *i_ptr;
            tmp_obj.number = 1;

            /* Decrease the item (from the pack) */
            if (item >= 0) {
                inven_item_increase(item, -1);
                inven_item_optimize(item);
            }

            /* Decrease the item (from the floor) */
            else {
                floor_item_increase(py, px, -1);
                floor_item_optimize(py, px);
            }

            /* Access the wield slot */
            i_ptr = &inventory[slot];

            /* Take off the "entire" item if one is there */
            if (inventory[slot].tval) inven_takeoff(slot, 255);

            /*** Could make procedure "inven_wield()" ***/

            /* Wear the new stuff */
            *i_ptr = tmp_obj;

            /* Increase the weight */
            inven_weight += i_ptr->weight;

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
                if (!i_ptr->inscrip[0]) inscribe(i_ptr, "cursed");
            }

            /* Update stuff */
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

            /* Redraw stuff */
            p_ptr->redraw |= (PR_EQUIPPY);

            /* Handle stuff */
            handle_stuff(!in_store_flag);
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
void do_cmd_inven_t(void)
{
    int item;

    /* Assume this will be free */
    energy_use = 0;

    /* XXX XXX XXX Hack -- verify potential overflow */
    if (inven_ctr >= INVEN_PACK) {

        /* Verify with the player */
        if (other_query_flag &&
            !get_check("Your pack may overflow.  Continue?")) return;
    }

    /* Get an item to take off */
    if (get_item(&item, "Take off which item? ", INVEN_WIELD, INVEN_TOTAL-1, FALSE)) {

        inven_type *i_ptr = &inventory[item];

        /* Not gonna happen */
        if (cursed_p(i_ptr)) {
            msg_print("Hmmm, it seems to be cursed.");
        }

        /* Item selected? */
        else {

            /* This turn is not free */
            energy_use = 100;

            /* Take off the "entire" item */
            inven_takeoff(item, 255);

            /* Update stuff */
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
            
            /* Redraw equippy chars */
            p_ptr->redraw |= (PR_EQUIPPY);

            /* Handle stuff */
            handle_stuff(!in_store_flag);
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
void do_cmd_inven_d(void)
{
    int item, amt;

    /* Assume this will be free */
    energy_use = 0;

    /* Get an item to take off */
    if (get_item(&item, "Drop which item? ", 0, INVEN_TOTAL-1, FALSE)) {

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
            sprintf(prt2, "Quantity (1-%d): ", i_ptr->number);
            prt(prt2, 0, 0);
            sprintf(prt1, "%d", amt);
            if (!askfor_aux(prt1, 3)) item = -1;

            /* Parse result */
            if (item >= 0) {
            
                /* Extract a number */
                amt = atoi(prt1);

                /* A letter means "all" */
                if (isalpha(prt1[0])) amt = 99;

                /* Keep the entry valid */
                if (amt > i_ptr->number) amt = i_ptr->number;

                /* Allow bizarre "abort" */
                if (amt <= 0) item = -1;
            }
        }

        /* Mega-Hack -- verify "dangerous" drops */
        if ((item >= 0) && (cave[py][px].i_idx)) {

            /* XXX XXX Verify with the player */
            if (other_query_flag &&
                !get_check("The item may disappear.  Continue?")) item = -1;
        }

        /* Actually drop */
        if (item >= 0) {

            /* This turn is not free */
            energy_use = 100;

            /* Drop (some of) the item */
            inven_drop(item, amt);

            /* Update stuff */
            p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
            
            /* Redraw equippy chars */
            p_ptr->redraw |= (PR_EQUIPPY);

            /* Handle stuff */
            handle_stuff(!in_store_flag);
        }

        /* Again, if displaying choices */
        if (command_see) command_new = 'd';
    }

    /* Nothing to drop */
    else if (item == -2) {
        msg_print("You have nothing to drop.");
    }
}



