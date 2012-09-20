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
bool is_a_vowel(int ch)
{
    switch (ch) {

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
    }

    return (FALSE);
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
    int i = c - 'a';

    /* Verify the index */
    if ((i < 0) || (i > INVEN_PACK)) return (-1);

    /* Empty slots can never be chosen */
    if (!inventory[i].k_idx) return (-1);

    /* Return the index */
    return (i);
}


/*
 * Convert a label into the index of a item in the "equip"
 * Return "-1" if the label does not indicate a real item
 */
int label_to_equip(int c)
{
    int i = INVEN_WIELD + (c - 'a');

    /* Speed -- Ignore silly labels */
    if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

    /* Empty slots can never be chosen */
    if (!inventory[i].k_idx) return (-1);

    /* Return the index */
    return (i);
}



/*
 * Determine which equipment slot (if any) an item likes
 */
s16b wield_slot(inven_type *i_ptr)
{
    /* Slot for equipment */
    switch (i_ptr->tval) {

        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
            return (INVEN_WIELD);

        case TV_BOW:
            return (INVEN_BOW);

        case TV_RING:

            /* Use the right hand first */
            if (!inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);

            /* Use the left hand for swapping (by default) */
            return (INVEN_LEFT);

        case TV_AMULET:
            return (INVEN_NECK);

        case TV_LITE:
            return (INVEN_LITE);

        case TV_DRAG_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
            return (INVEN_BODY);

        case TV_CLOAK:
            return (INVEN_OUTER);

        case TV_SHIELD:
            return (INVEN_ARM);

        case TV_CROWN:
        case TV_HELM:
            return (INVEN_HEAD);

        case TV_GLOVES:
            return (INVEN_HANDS);

        case TV_BOOTS:
            return (INVEN_FEET);
    }

    /* No slot available */
    return (-1);
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
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "Just lifting";
        }
    }

    /* Hack -- Heavy bow */
    if (i == INVEN_BOW) {
        inven_type *i_ptr;
        i_ptr = &inventory[i];
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "Just holding";
        }
    }

    /* Return the result */
    return (p);
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
cptr describe_use(int i)
{
    cptr p;

    switch (i) {
      case INVEN_WIELD: p = "attacking monsters with"; break;
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
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "just lifting";
        }
    }

    /* Hack -- Heavy bow */
    if (i == INVEN_BOW) {
        inven_type *i_ptr;
        i_ptr = &inventory[i];
        if (adj_str_hold[p_ptr->stat_ind[A_STR]] < i_ptr->weight / 10) {
            p = "just holding";
        }
    }

    /* Return the result */
    return p;
}



/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" command_cmd code.
 */
static int get_tag(int *com_val, char tag)
{
    int i;
    cptr s;


    /* Check every object */
    for (i = 0; i < INVEN_TOTAL; ++i) {

        inven_type *i_ptr = &inventory[i];

        /* Skip empty objects */
        if (!i_ptr->k_idx) continue;

        /* Skip empty inscriptions */
        if (!i_ptr->note) continue;

        /* Find a '@' */
        s = strchr(quark_str(i_ptr->note), '@');

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






/*
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(int item)
{
    inven_type *i_ptr = &inventory[item];

    /* Require staff/wand */
    if ((i_ptr->tval != TV_STAFF) && (i_ptr->tval != TV_WAND)) return;

    /* Require known item */
    if (!inven_known_p(i_ptr)) return;

    /* Multiple charges */
    if (i_ptr->pval != 1) {

        /* Print a message */
        msg_format("You have %d charges remaining.", i_ptr->pval);
    }

    /* Single charge */
    else {

        /* Print a message */
        msg_format("You have %d charge remaining.", i_ptr->pval);
    }
}


/*
 * Describe an item in the inventory.
 */
void inven_item_describe(int item)
{
    inven_type	*i_ptr = &inventory[item];

    char	i_name[80];
        
    /* Get a description */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Print a message */
    msg_format("You have %s.", i_name);
}


/*
 * Increase the "number" of an item in the inventory
 */
void inven_item_increase(int item, int num)
{
    inven_type *i_ptr = &inventory[item];

    /* Apply */
    num += i_ptr->number;

    /* Bounds check */
    if (num > 255) num = 255;
    else if (num < 0) num = 0;

    /* Un-apply */
    num -= i_ptr->number;

    /* Change the number and weight */
    if (num) {

        /* Add the number */
        i_ptr->number += num;

        /* Add the weight */
        total_weight += (num * i_ptr->weight);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Redraw the choice window */
        p_ptr->redraw |= (PR_CHOOSE);
    }
}


/*
 * Erase an inventory slot if it has no more items
 *
 * Note that the "slide" code works on the equipment too
 */
void inven_item_optimize(int item)
{
    inven_type *i_ptr = &inventory[item];

    /* Only optimize real items */
    if (!i_ptr->k_idx) return;

    /* Only optimize empty items */
    if (i_ptr->number) return;

    /* The item is in the pack */
    if (item < INVEN_WIELD) {

        /* One less item */
        inven_cnt--;
    }

    /* The item is being wielded */
    else {

        /* One less item */
        equip_cnt--;
    }

    /* Reorder the pack */
    if (auto_reorder_pack) {

        int i;
        
        /* Slide everything down */
        for (i = item; i < INVEN_PACK; i++) {

            /* Structure copy */
            inventory[i] = inventory[i+1];
        }
        
        /* Erase the "final" slot */
        invwipe(&inventory[i]);
    }
    
    /* Just wipe the slot */
    else {
    
        /* Erase the empty slot */
        invwipe(&inventory[item]);

        /* Reorder the pack later */
        p_ptr->update |= (PU_REORDER);
    }
    
    /* Redraw choice window */
    p_ptr->redraw |= (PR_CHOOSE);
}


/*
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(int item)
{
    inven_type *i_ptr = &i_list[item];

    /* Require staff/wand */
    if ((i_ptr->tval != TV_STAFF) && (i_ptr->tval != TV_WAND)) return;

    /* Require known item */
    if (!inven_known_p(i_ptr)) return;

    /* Multiple charges */
    if (i_ptr->pval != 1) {

        /* Print a message */
        msg_format("There are %d charges remaining.", i_ptr->pval);
    }

    /* Single charge */
    else {

        /* Print a message */
        msg_format("There is %d charge remaining.", i_ptr->pval);
    }
}



/*
 * Describe an item in the inventory.
 */
void floor_item_describe(int item)
{
    inven_type	*i_ptr = &i_list[item];

    char	i_name[80];

    /* Get a description */
    objdes(i_name, i_ptr, TRUE, 3);

    /* Print a message */
    msg_format("You see %s.", i_name);
}


/*
 * Increase the "number" of an item on the floor
 */
void floor_item_increase(int item, int num)
{
    inven_type *i_ptr = &i_list[item];

    /* Apply */
    num += i_ptr->number;

    /* Bounds check */
    if (num > 255) num = 255;
    else if (num < 0) num = 0;

    /* Un-apply */
    num -= i_ptr->number;
    
    /* Change the number */
    i_ptr->number += num;
}


/*
 * Optimize an item on the floor (destroy "empty" items)
 */
void floor_item_optimize(int item)
{
    inven_type *i_ptr = &i_list[item];

    /* Paranoia -- be sure it exists */
    if (!i_ptr->k_idx) return;

    /* Only optimize empty items */
    if (i_ptr->number) return;
    
    /* Delete it */
    delete_object_idx(item);
}





/*
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(inven_type *i_ptr)
{
    int i;

    /* Empty slot? */
    if (inven_cnt < INVEN_PACK) return (TRUE);

    /* Similar slot? */
    for (i = 0; i < INVEN_PACK; i++) {

        /* Get that item */
        inven_type *j_ptr = &inventory[i];

        /* Check if the two items can be combined */
        if (item_similar(j_ptr, i_ptr)) return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


/*
 * Add an item to the players inventory, and return the slot used.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so (using "item_similar()" and "item_absorb()"), otherwise,
 * the item will be placed into the first available empty slot, unless
 * the "auto_reorder_pack" option is set, in which case the item will
 * be inserted into a reasonable location, to prevent misleading
 * about the reordering of the pack.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined.  This may be tricky.  See "dungeon.c" for info.
 */
s16b inven_carry(inven_type *i_ptr)
{
    int         i, j, k;
    int		n = -1;

    inven_type	*j_ptr;


    /* Check for combining */
    for (j = 0; j < INVEN_PACK; j++) {

        j_ptr = &inventory[j];

        /* Skip empty items */
        if (!j_ptr->k_idx) continue;

        /* Hack -- track last item */
        n = j;

        /* Check if the two items can be combined */
        if (item_similar(j_ptr, i_ptr)) {

            /* Combine the items */
            item_absorb(j_ptr, i_ptr);
            
            /* Increase the weight */
            total_weight += (i_ptr->number * i_ptr->weight);

            /* Redraw choice window (later) */
            p_ptr->redraw |= (PR_CHOOSE);

            /* Recalculate bonuses (later) */
            p_ptr->update |= (PU_BONUS);

            /* All done, report where we put it */
            return (j);
        }
    }


    /* Paranoia */
    if (inven_cnt > INVEN_PACK) return (-1);


    /* Find an empty slot */
    for (j = 0; j <= INVEN_PACK; j++) {

        j_ptr = &inventory[j];

        /* Use it if found */
        if (!j_ptr->k_idx) break;
    }

    /* Use that slot */
    i = j;


    /* Hack -- pre-reorder the pack */
    if (auto_reorder_pack && (i < INVEN_PACK)) {

        s32b		i_value, j_value;

        /* Get the "value" of the item */
        i_value = item_value(i_ptr);

        /* Scan every occupied slot */
        for (j = 0; j < INVEN_PACK; j++) {

            j_ptr = &inventory[j];

            /* Use empty slots */
            if (!j_ptr->k_idx) break;

            /* Hack -- readable books always come first */
            if ((i_ptr->tval == mp_ptr->spell_book) &&
                (j_ptr->tval != mp_ptr->spell_book)) break;
            if ((j_ptr->tval == mp_ptr->spell_book) &&
                (i_ptr->tval != mp_ptr->spell_book)) continue;

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
        i = j;

        /* Structure slide (make room) */
        for (k = n; k >= i; k--) {

            /* Hack -- Slide the item */
            inventory[k+1] = inventory[k];
        }

	/* Paranoia -- Wipe the new slot */
        invwipe(&inventory[i]);
    }


    /* Structure copy to insert the new item */
    inventory[i] = (*i_ptr);

    /* Forget the old location */
    inventory[i].iy = inventory[i].ix = 0;

    /* Increase the weight, prepare to redraw */
    total_weight += (i_ptr->number * i_ptr->weight);

    /* Count the items */
    inven_cnt++;

    /* Redraw choice window (later) */
    p_ptr->redraw |= (PR_CHOOSE);

    /* Recalculate bonuses (later) */
    p_ptr->update |= (PU_BONUS);

    /* Reorder pack */
    p_ptr->update |= (PU_REORDER);

    /* Return the slot */
    return (i);
}






/*
 * Check an item against the item tester info
 */
bool item_tester_okay(inven_type *i_ptr)
{
    /* Hack -- allow listing empty slots */
    if (item_tester_full) return (TRUE);

    /* Require an item */
    if (!i_ptr->k_idx) return (FALSE);

    /* Hack -- ignore "gold" */
    if (i_ptr->tval == TV_GOLD) return (FALSE);

    /* Check the tval */
    if (item_tester_tval && (!(item_tester_tval == i_ptr->tval))) return (FALSE);

    /* Check the hook */
    if (item_tester_hook && (!(*item_tester_hook)(i_ptr))) return (FALSE);

    /* Assume okay */
    return (TRUE);
}




/*
 * Choice window "shadow" of the "show_inven()" function
 */
void display_inven(void)
{
    register	int i, n, z = 0;

    inven_type *i_ptr;

    byte	attr = TERM_WHITE;

    char	tmp_val[80];

    char	i_name[80];


    /* Find the "final" slot */
    for (i = 0; i < INVEN_PACK; i++) {

        i_ptr = &inventory[i];

        /* Track non-empty slots */
        if (i_ptr->k_idx) z = i + 1;
    }

    /* Display the pack */
    for (i = 0; i < z; i++) {

        /* Examine the item */
        i_ptr = &inventory[i];

        /* Start with an empty "index" */
        tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

        /* Is this item "acceptable"? */
        if (item_tester_okay(i_ptr)) {

            /* Prepare an "index" */
            tmp_val[0] = index_to_label(i);

            /* Bracket the "index" --(-- */
            tmp_val[1] = ')';
        }

        /* Display the index (or blank space) */
        Term_putstr(0, i, 3, TERM_WHITE, tmp_val);

        /* Obtain an item description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Obtain the length of the description */
        n = strlen(i_name);

        /* Get a color */
        if (use_color) attr = tval_to_attr[i_ptr->tval];

        /* Display the entry itself */
        Term_putstr(3, i, n, attr, i_name);

        /* Erase the rest of the line */
        Term_erase(3+n, i, 80, 1);

        /* Display the weight if needed */
        if (show_choose_weight && i_ptr->weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            Term_putstr(71, i, -1, TERM_WHITE, tmp_val);
        }
    }

    /* Erase the rest of the window */
    Term_erase(0, z, 80, 24);

    /* Refresh */
    Term_fresh();
}



/*
 * Choice window "shadow" of the "show_equip()" function
 */
void display_equip(void)
{
    register	int i, n;
    inven_type *i_ptr;
    byte	attr = TERM_WHITE;

    char	tmp_val[80];

    char	i_name[80];


    /* Display the equipment */
    for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        /* Examine the item */
        i_ptr = &inventory[i];

        /* Start with an empty "index" */
        tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

        /* Is this item "acceptable"? */
        if (item_tester_okay(i_ptr)) {

            /* Prepare an "index" */
            tmp_val[0] = index_to_label(i);

            /* Bracket the "index" --(-- */
            tmp_val[1] = ')';
        }

        /* Display the index (or blank space) */
        Term_putstr(0, i - INVEN_WIELD, 3, TERM_WHITE, tmp_val);

        /* Obtain an item description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Obtain the length of the description */
        n = strlen(i_name);

        /* Get the color */
        if (use_color) attr = tval_to_attr[i_ptr->tval];

        /* Display the entry itself */
        Term_putstr(3, i - INVEN_WIELD, n, attr, i_name);

        /* Erase the rest of the line */
        Term_erase(3+n, i - INVEN_WIELD, 80, 1);

        /* Display the slot description (if needed) */
        if (show_choose_label) {
            Term_putstr(61, i - INVEN_WIELD, -1, TERM_WHITE, "<--");
            Term_putstr(65, i - INVEN_WIELD, -1, TERM_WHITE, mention_use(i));
        }

        /* Display the weight (if needed) */
        if (show_choose_weight && i_ptr->weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            int col = (show_choose_label ? 52 : 71);
            sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            Term_putstr(col, i - INVEN_WIELD, -1, TERM_WHITE, tmp_val);
        }
    }

    /* Erase the rest of the window */
    Term_erase(0, INVEN_TOTAL - INVEN_WIELD, 80, 24);

    /* Refresh */
    Term_fresh();
}






/*
 * Display the inventory.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven(void)
{
    int		i, j, k, l, z = 0;
    int		col, len, lim;

    inven_type	*i_ptr;

    char	i_name[80];

    char	tmp_val[80];

    int		out_index[23];
    byte	out_color[23];
    char	out_desc[23][80];


    /* Starting column */
    col = command_gap;

    /* Default "max-length" */
    len = 79 - col;

    /* Maximum space allowed for descriptions */
    lim = 79 - 3;

    /* Require space for weight (if needed) */
    if (show_inven_weight) lim -= 9;


    /* Find the "final" slot */
    for (i = 0; i < INVEN_PACK; i++) {

        i_ptr = &inventory[i];

        /* Track non-empty slots */
        if (i_ptr->k_idx) z = i + 1;
    }

    /* Display the inventory */
    for (k = 0, i = 0; i < z; i++) {

        i_ptr = &inventory[i];

        /* Is this item acceptable? */
        if (!item_tester_okay(i_ptr)) continue;

        /* Describe the object */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Hack -- enforce max length */
        i_name[lim] = '\0';

        /* Save the object index, color, and description */
        out_index[k] = i;
        out_color[k] = tval_to_attr[i_ptr->tval];
        (void)strcpy(out_desc[k], i_name);

        /* Find the predicted "line length" */
        l = strlen(out_desc[k]) + 5;

        /* Be sure to account for the weight */
        if (show_inven_weight) l += 9;

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
        if (show_inven_weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            (void)sprintf(tmp_val, "%3d.%1d lb", wgt / 10, wgt % 10);
            put_str(tmp_val, j + 1, 71);
        }
    }

    /* Erase the final line */
    prt("", j + 1, col ? col - 2 : col);

    /* Save the new column */
    command_gap = col;
}



/*
 * Display the equipment.
 */
void show_equip(void)
{
    int			i, j, k, l;
    int			col, len, lim;

    inven_type		*i_ptr;

    char		tmp_val[80];

    char		i_name[80];

    int			out_index[23];
    byte		out_color[23];
    char		out_desc[23][80];


    /* Starting column */
    col = command_gap;
    
    /* Maximal length */
    len = 79 - col;

    /* Maximum space allowed for descriptions */
    lim = 79 - 3;

    /* Require space for labels (if needed) */
    if (show_equip_label) lim -= (14 + 2);
    
    /* Require space for weight (if needed) */
    if (show_equip_weight) lim -= 9;

    /* Scan the equipment list */
    for (k = 0, i = INVEN_WIELD; i < INVEN_TOTAL; i++) {

        i_ptr = &inventory[i];

        /* Is this item acceptable? */
        if (!item_tester_okay(i_ptr)) continue;

        /* Description */
        objdes(i_name, i_ptr, TRUE, 3);

        /* Truncate the description */
        i_name[lim] = 0;

        /* Save the color */
        out_index[k] = i;
        out_color[k] = tval_to_attr[i_ptr->tval];
        (void)strcpy(out_desc[k], i_name);

        /* Extract the maximal length (see below) */
        l = strlen(out_desc[k]) + (2 + 3);
        
        /* Increase length for labels (if needed) */
        if (show_equip_label) l += (14 + 2);

        /* Increase length for weight (if needed) */
        if (show_equip_weight) l += 9;

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

        /* Use labels */
        if (show_equip_label) {

            /* Mention the use */
            (void)sprintf(tmp_val, "%-14s: ", mention_use(i));
            put_str(tmp_val, j+1, col + 3);

            /* Display the entry itself */
            c_put_str(out_color[j], out_desc[j], j+1, col + 3 + 14 + 2);
        }

        /* No labels */
        else {

            /* Display the entry itself */
            c_put_str(out_color[j], out_desc[j], j+1, col + 3);
        }

        /* Display the weight if needed */
        if (show_equip_weight) {
            int wgt = i_ptr->weight * i_ptr->number;
            (void)sprintf(tmp_val, "%3d.%d lb", wgt / 10, wgt % 10);
            put_str(tmp_val, j+1, 71);
        }
    }

    /* Make a shadow below the list (if possible) */
    prt("", j+1, col);

    /* Save the new column */
    command_gap = col;
}




/*
 * Verify the choice of an item.
 */
static bool verify(cptr prompt, int item)
{
    char	i_name[80];

    char	out_val[160];


    /* Describe */
    objdes(i_name, &inventory[item], TRUE, 3);

    /* Prompt */
    (void)sprintf(out_val, "%s %s? ", prompt, i_name);

    /* Query */
    return (get_check(out_val));
}


/*
 * Hack -- allow user to "prevent" certain choices
 */
static bool get_item_allow(int i)
{
    cptr s;

    inven_type *i_ptr = &inventory[i];

    /* No inscription */
    if (!i_ptr->note) return (TRUE);

    /* Find a '!' */
    s = strchr(quark_str(i_ptr->note), '!');

    /* Process preventions */
    while (s) {

        /* Check the "restriction" */
        if ((s[1] == command_cmd) || (s[1] == '*')) {

            /* Verify the choice */
            if (!verify("Really Try", i)) return (FALSE);
        }

        /* Find another '!' */
        s = strchr(s + 1, '!');
    }

    /* Allow it */
    return (TRUE);
}



/*
 * Auxiliary function for "get_item()" -- test an index
 */
static bool get_item_okay(int i)
{
    /* Illegal items */
    if ((i < 0) || (i >= INVEN_TOTAL)) return (FALSE);

    /* Verify the item */
    if (!item_tester_okay(&inventory[i])) return (FALSE);

    /* Assume okay */
    return (TRUE);
}


/*
 * Let the user select an item, return its "index"
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.  Note that
 * the equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected, we save it in "com_val" and return TRUE.
 * If this "legal" item is on the floor, we use a "com_val" equal to zero
 * minus the dungeon index of the item on the floor.
 *
 * Otherwise, we return FALSE, and set "com_val" to:
 *   -1 for "User hit space/escape"
 *   -2 for "No legal items to choose"
 *
 * Global "command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens.
 *
 * Global "command_see" may be set before calling this function to start
 * out in "browse" mode.  It is cleared before this function returns.
 *
 * Global "command_wrk" is used to choose between equip/inven listings.
 * If it is TRUE then we are viewing inventory, else equipment.
 *
 * Global "command_gap" is used to indent the inven/equip tables, and
 * to provide some consistancy over time.  It shrinks as needed to hold
 * the various tables horizontally, and can only be reset by calling this
 * function with "command_see" being FALSE, or by pressing ESCAPE from
 * this function, or by hitting "escape" while viewing the inven/equip.
 *
 * We always erase the prompt when we are done.
 *
 * Note that "Term_save()" / "Term_load()" blocks must not overlap.
 */
bool get_item(int *com_val, cptr pmt, bool equip, bool inven, bool floor)
{
    char        n1, n2, which = ' ';

    int		floor_item = 0;

    int		k, i1, i2, e1, e2;
    bool	ver, done, item;

    char	tmp_val[160];
    char	out_val[160];


    /* Not done */
    done = FALSE;

    /* No item selected */
    item = FALSE;

    /* Default to "no item" (see above) */
    *com_val = -1;


    /* Paranoia */
    if (!inven && !equip) return (FALSE);


    /* Full inventory */
    i1 = 0;
    i2 = INVEN_PACK - 1;

    /* Forbid inventory */
    if (!inven) i2 = -1;

    /* Restrict inventory indexes */
    while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
    while ((i1 <= i2) && (!get_item_okay(i2))) i2--;
    

    /* Full equipment */
    e1 = INVEN_WIELD;
    e2 = INVEN_TOTAL - 1;

    /* Forbid equipment */
    if (!equip) e2 = -1;
    
    /* Restrict equipment indexes */
    while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
    while ((e1 <= e2) && (!get_item_okay(e2))) e2--;
    

    /* Hack -- Restrict floor usage */
    if (floor) {

        cave_type *c_ptr = &cave[py][px];
    
        inven_type *i_ptr = &i_list[c_ptr->i_idx];
        
        /* Accept the item on the floor if legal */
        if (item_tester_okay(i_ptr)) floor_item = c_ptr->i_idx;
    }


    /* Verify choices */
    if (!floor_item && (i1 > i2) && (e1 > e2)) {

        /* Cancel command_see */
        command_see = FALSE;

        /* Hack -- Nothing to choose */
        *com_val = -2;
        
        /* Done */
        done = TRUE;
    }

    /* Analyze choices */
    else {
    
        /* Hack -- Reset display width */
        if (!command_see) command_gap = 50;

        /* Hack -- Start on equipment if requested */
        if (command_see && command_wrk && equip) {
            command_wrk = TRUE;
        }
 
        /* Use inventory if allowed */
        else if (inven) {
            command_wrk = FALSE;
        }

        /* Use equipment if allowed */
        else if (equip) {
            command_wrk = TRUE;
        }

        /* Use inventory for floor */
        else {
            command_wrk = FALSE;
        }
    }


    /* Clear old messages */
    msg_print(NULL);


    /* Save "term_choice" */
    if (term_choice && use_choice_choose) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Save the contents */
        Term_save();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

    /* Save "term_mirror" */
    if (term_mirror && use_mirror_choose) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Save the contents */
        Term_save();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }


    /* Hack -- start out in "display" mode */
    if (command_see) Term_save();


    /* Repeat until done */
    while (!done) {

        /* Inventory screen */
        if (!command_wrk) {

            /* Extract the legal requests */
            n1 = 'a' + i1;
            n2 = 'a' + i2;

            /* Redraw if needed */
            if (command_see) show_inven();

            /* Display choices in "term_choice" */
            if (term_choice && use_choice_choose) {

                /* Activate the choice window */
                Term_activate(term_choice);

                /* Display the inventory */
                display_inven();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }

            /* Display choices in "term_mirror" */
            if (term_mirror && use_mirror_choose) {

                /* Activate the mirror window */
                Term_activate(term_mirror);

                /* Display the inventory */
                display_inven();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }
        }

        /* Equipment screen */
        else {

            /* Extract the legal requests */
            n1 = 'a' + e1 - INVEN_WIELD;
            n2 = 'a' + e2 - INVEN_WIELD;

            /* Redraw if needed */
            if (command_see) show_equip();

            /* Display choices in "term_choice" */
            if (term_choice && use_choice_choose) {

                /* Activate the choice window */
                Term_activate(term_choice);

                /* Display the equipment */
                display_equip();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }

            /* Display choices in "term_mirror" */
            if (term_mirror && use_mirror_choose) {

                /* Activate the mirror window */
                Term_activate(term_mirror);

                /* Display the equipment */
                display_equip();

                /* Re-activate the main screen */
                Term_activate(term_screen);
            }
        }

        /* Viewing inventory */    
        if (!command_wrk) {
        
            /* Begin the prompt */
            sprintf(out_val, "Inven:");
        
            /* Some legal items */
            if (i1 <= i2) {

                /* Build the prompt */
                sprintf(tmp_val, " %c-%c,",
                        index_to_label(i1), index_to_label(i2));

                /* Append */
                strcat(out_val, tmp_val);
            }
            
            /* Indicate ability to "view" */
            if (!command_see) strcat(out_val, " * to see,");

            /* Append */
            if (equip) strcat(out_val, " / for Equip,");
        }
    
        /* Viewing equipment */    
        else {

            /* Begin the prompt */
            sprintf(out_val, "Equip:");
        
            /* Some legal items */
            if (e1 <= e2) {

                /* Build the prompt */
                sprintf(tmp_val, " %c-%c,",
                        index_to_label(e1), index_to_label(e2));

                /* Append */
                strcat(out_val, tmp_val);
            }
            
            /* Indicate ability to "view" */
            if (!command_see) strcat(out_val, " * to see,");
        
            /* Append */
            if (inven) strcat(out_val, " / for Inven,");
        }
        
        /* Indicate legality of the "floor" item */
        if (floor_item) strcat(out_val, " - for floor,");
        
        /* Finish the prompt */
        strcat(out_val, " ESC");

        /* Build the prompt */
        sprintf(tmp_val, "(%s) %s", out_val, pmt);

        /* Show the prompt */	
        prt(tmp_val, 0, 0);


        /* Get a key */
        which = inkey();

        /* Parse it */
        switch (which) {

          case ESCAPE:

            command_gap = 50;
            done = TRUE;
            break;

          case '*':
          case '?':
          case ' ':

            /* Show/hide the list */
            if (!command_see) {
                Term_save();
                command_see = TRUE;
            }
            else {
                Term_load();
                command_see = FALSE;
            }
            break;

          case '/':

            /* Hack -- no "changing pages" allowed */
            if (!inven || !equip) {
                bell();
                break;
            }

            /* Hack -- Erase old info */
            if (command_see) {
                for (k = n1 - 'a'; k <= n2 - 'a'; k++) prt("", k+1, 0);
            }

            /* Switch inven/equip */
            command_wrk = !command_wrk;

            /* Need to redraw */
            break;

          case '-':

            /* Use floor item */
            if (floor_item) {
                (*com_val) = 0 - floor_item;
                item = TRUE;
                done = TRUE;
            }
            else {
                bell();
            }
            break;

          case '0':
          case '1': case '2': case '3':
          case '4': case '5': case '6':
          case '7': case '8': case '9':

            /* XXX XXX Look up that tag */
            if (!get_tag(&k, which)) {
                bell();
                break;
            }

            /* Hack -- Verify item */
            if ((k < INVEN_WIELD) ? !inven : !equip) {
                bell();
                break;
            }

            /* Validate the item */
            if (!get_item_okay(k)) {
                bell();
                break;
            }

            /* Allow player to "refuse" certain actions */
            if (!get_item_allow(k)) {
                done = TRUE;
                break;
            }

            /* Use that item */
            (*com_val) = k;
            item = TRUE;
            done = TRUE;
            break;

          case '\n':
          case '\r':

            /* Choose "default" inventory item */
            if (!command_wrk) {
                k = ((i1 == i2) ? i1 : -1);
            }

            /* Choose "default" equipment item */
            else {
                k = ((e1 == e2) ? e1 : -1);
            }

            /* Validate the item */
            if (!get_item_okay(k)) {
                bell();
                break;
            }

            /* Allow player to "refuse" certain actions */
            if (!get_item_allow(k)) {
                done = TRUE;
                break;
            }

            /* Accept that choice */
            (*com_val) = k;
            item = TRUE;
            done = TRUE;
            break;

          default:

            /* Extract "query" setting */
            ver = isupper(which);
            if (ver) which = tolower(which);

            /* Convert letter to inventory index */
            if (!command_wrk) {
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
                done = TRUE;
                break;
            }

            /* Allow player to "refuse" certain actions */
            if (!get_item_allow(k)) {
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
    if (command_see) Term_load();

    /* Hack -- Cancel "display" */
    command_see = FALSE;


    /* Restore "term_mirror" */
    if (term_mirror && use_mirror_choose) {

        /* Activate the mirror window */
        Term_activate(term_mirror);

        /* Load the contents */
        Term_load();

        /* Refresh */
        Term_fresh();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }

    /* Restore "term_choice" */
    if (term_choice && use_choice_choose) {

        /* Activate the choice window */
        Term_activate(term_choice);

        /* Load the contents */
        Term_load();

        /* Refresh */
        Term_fresh();

        /* Re-activate the main screen */
        Term_activate(term_screen);
    }


    /* Mega-Hack -- Redraw the choice window */
    p_ptr->redraw |= (PR_CHOOSE);


    /* Forget the item_tester_tval restriction */
    item_tester_tval = 0;

    /* Forget the item_tester_hook restriction */
    item_tester_hook = NULL;

    /* Clear the prompt line */
    prt("", 0, 0);

    /* Return TRUE if something was picked */
    return (item);
}


