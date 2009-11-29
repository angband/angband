/*
 * File: object1.c
 * Purpose: Mainly object descriptions and generic UI functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "tvalsval.h"


/*
 * Display the inventory.  Called with mode |= OLIST_WINDOW
 * from the inventory term window.
 *
 * Hack -- do not display "trailing" empty slots
 */
void show_inven(olist_detail_t mode)
{
	int i, j, k, l, z = 0;
	int col, row, r_col, len = 0, lim, ex_wid = 0;
	int ammo_count;

	object_type *o_ptr;
	char o_name[80];

	char tmp_val[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];


	/* Default length of the item name column */
	if (mode & OLIST_WINDOW) len = 60;

	/* Maximum space allowed for item descriptions 
	 * Screen width minus the letter label "a) "
	 */
	lim = (Term->wid-1) - 3;

   /* Reserve space for extra fields */
	if (mode & OLIST_PRICE)
		ex_wid += 9;
	if (mode & OLIST_WEIGHT)
		ex_wid += 9;
	lim -= ex_wid;
		
	/* Do not overflow o_name buffer */
	if (lim > 79)
		lim = 79;

	/* Find the "final" slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Track */
		z = i + 1;
	}

	/* Display the inventory */
	for (k = 0, i = 0; i < z; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr))
		{
			if (mode & OLIST_WINDOW)
			{
				/* Display unacceptable items in terminal windows, but
				 * don't save the index for a letter label
				 */
				out_index[k] = -1;
			}
			else
			{
				/* Don't display unacceptable items in main window lists */
				continue;
			}
		}
		else
		{
			/* Acceptable, so save the index */
			out_index[k] = i;
		}

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Find the predicted line length: "a) " + Item description  */
		l = 3 + strlen(o_name);

		/* Adjust length for for extra fields */
		l += ex_wid;

		/* Track the maximum line length */
		if (l > len) len = l;

		/* Save the object description */
		my_strcpy(out_desc[k], o_name, sizeof(out_desc[0]));

		/* Save the inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Advance to next "line" */
		k++;
	}

	/* Find the row and column to start in */
	if (mode & OLIST_WINDOW)
	{
		/* Terminal window - left justified */
		col = 0;
		row = 0;
		r_col = MIN(Term->wid, len + 1);
	}
	else
	{
		/* Main window - right justified */
		col = (Term->wid-1) - len;
		row = 1;
		r_col = Term->wid;
		if (col < 3) col = 0;
	}

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		u32b price;
		int ralign, wgt;
		
		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Clear the line */
		prt("", row + j, col ? col - 2 : col);

		if (i > -1)
		{
			/* Prepare an index --(-- */
			strnfmt(tmp_val, sizeof(tmp_val), "%c)", index_to_label(i));

			/* Clear the line with the (possibly indented) index */
			put_str(tmp_val, row + j, col);
		}

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], row + j, col + 3);

		/* Display extra fields */
		ralign = r_col;
		if (mode & OLIST_WEIGHT)
		{
			ralign -= 9;
			wgt = o_ptr->weight * o_ptr->number;
			strnfmt(tmp_val, sizeof(tmp_val), "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, row + j, ralign);
		}
		if (mode & OLIST_PRICE)
		{
			ralign -= 9;
			price = price_item(o_ptr, TRUE, o_ptr->number);
			strnfmt(tmp_val, sizeof(tmp_val), "%5d au", price);
			put_str(tmp_val, row + j, ralign);
		}
	}

	/* Count the number of missiles in the quiver */
	if (j < 23 && p_ptr->command_cmd == 'i')
	{
		ammo_count = 0;
		for (i=QUIVER_START; i < QUIVER_END; i++)
			if (inventory[i].k_idx)
				ammo_count += inventory[i].number;
	
		/* Clear a row, and then print the number of missiles in the quiver */
		prt("", row + j, col ? col - 2 : col);
		strnfmt(o_name, sizeof(o_name), "(QUIVER - %d missile%s)", ammo_count,
				ammo_count == 1 ? "" : "s");
		c_put_str(TERM_BLUE, o_name, row + j, col + 3);
		j++;
	}

	if (mode & OLIST_WINDOW)
	{
		/* Erase the rest of the window */
		for (i = z; i < Term->hgt; i++)
		{
			/* Erase the line */
			Term_erase(0, i, 255);
		}
	}
	else
	{
		/* Make a "shadow" below the list (only if needed) */
		if (j && (j < 23)) prt("", row + j, col ? col - 2 : col);
	}
}


/*
 * Display the equipment.  Called with mode |= OLIST_WINDOW
 * from the equipment term window.
 */
void show_equip(olist_detail_t mode)
{
	int i, j, k, l, b = 0;
	int col, row, r_col, len = 0, lim, ex_wid = 0;
	int x, y, last = 0;

	object_type *o_ptr;
	char o_name[80];

	char tmp_val[80];

	int out_index[24];
	byte out_color[24];
	char out_desc[24][80];


	/* Default length of the item name column */
	if (mode & OLIST_WINDOW) len = 60;

	/* Maximum space allowed for descriptions 
	 * Screen width minus the letter label "a) "
	 */
	lim = (Term->wid-1) - 3;

   /* Reserve space for extra fields */
	if (mode & OLIST_PRICE)
		ex_wid += 9;
	if (mode & OLIST_WEIGHT)
		ex_wid += 9;
	lim -= ex_wid;
		
	/* Reserve space for labels: "Wielded       " + ": " */
	if (OPT(show_labels)) lim -= (14 + 2);

	/* Do not overflow o_name buffer */
	if (lim > 79)
		lim = 79;

	/* Scan the equipment list */
	for (k = 0, i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Is this item acceptable? */
		if (!item_tester_okay(o_ptr))
		{
			if (mode & OLIST_WINDOW)
			{
				/* Display unacceptable items in terminal windows, but
				 * don't save the index for a letter label
				 */
				out_index[k] = -1;
			}
			else
			{
				/* Don't display unacceptable items in main window lists */
				continue;
			}
		}
		else
		{
			/* Acceptable, so save the index */
			out_index[k] = i;
		}

		/* Save the last slot that should be displayed */
		if (i < INVEN_TOTAL - 1 || o_ptr->k_idx) last = k;

		/* Description */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Truncate the description */
		o_name[lim] = 0;

		/* Find the predicted line length: "a) " + Item description */
		l = 3 + strlen(o_name);

		/* Increase length for labels (if needed) */
		if (OPT(show_labels)) l += (14 + 2);

		/* Adjust length for extra fields */
		l += ex_wid;

		/* Track the maximum line length */
		if (l > len) len = l;

		/* Save the description */
		my_strcpy(out_desc[k], o_name, sizeof(out_desc[0]));

		/* Save the inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Advance the entry */
		k++;
	}

	/* Find the row and column to start in */
	if (mode & OLIST_WINDOW)
	{
		/* Terminal window - left justified */
		col = 0;
		row = 0;
		r_col = MIN(Term->wid, len + 1);
	}
	else
	{
		/* Main window - right justified */
		col = (Term->wid-1) - len;
		row = 1;
		r_col = Term->wid;
		if (col < 3) col = 0;
	}

   /* Track the slot label */
   b = INVEN_WIELD;

	/* Output each entry */
	for (j = 0; j <= last; j++, b++)
	{
		u32b price;
		int ralign, wgt;

		/* Get the index */
		i = out_index[j];

		/* Get the item */
		o_ptr = &inventory[i];

		/* Display row */
		y = row + j;

		/* Clear the line */
		prt("", y, col ? col - 2 : col);

		/* There is an empty line between regular equipment and the quiver */
		if (j + INVEN_WIELD == INVEN_TOTAL) continue;

		/* Prepare an index --(-- */
		if (i > -1)
		{
			/* Track the slot label */
			b = i;
			
			strnfmt(tmp_val, sizeof(tmp_val), "%c)", index_to_label(i));

			/* Clear the line with the (possibly indented) index */
			put_str(tmp_val, y, col);
		}

		/* Display column */
		x = col + 3;

		/* Use labels */
		if (OPT(show_labels))
		{
			/* Mention the use */
			strnfmt(tmp_val, sizeof(tmp_val), "%-14s: ", mention_use(b));
			put_str(tmp_val, y, x);
			x += 14 + 2;
		}

		/* Only "quiver labels" */
		else if (i >= QUIVER_START)
		{
			strnfmt(tmp_val, sizeof(tmp_val), "[f%d] ", i - QUIVER_START);
			put_str(tmp_val, y, x);
			x += 4 + 1;
		}

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], y, x);


		/* Display extra fields */
		ralign = r_col;
		if (mode & OLIST_WEIGHT)
		{
			ralign -= 9;
			wgt = o_ptr->weight * o_ptr->number;
			strnfmt(tmp_val, sizeof(tmp_val), "%3d.%1d lb", wgt / 10, wgt % 10);
			put_str(tmp_val, y, ralign);
		}
		if (mode & OLIST_PRICE)
		{
			ralign -= 9;
			price = price_item(o_ptr, TRUE, o_ptr->number);
			strnfmt(tmp_val, sizeof(tmp_val), "%5d au", price);
			put_str(tmp_val, y, ralign);
		}
	}

	if (mode & OLIST_WINDOW)
	{
		/* Erase the rest of the window */
		for (i = k; i < Term->hgt; i++)
		{
			/* Clear that line */
			Term_erase(0, i, 255);
		}
	}
	else
	{
		/* Make a "shadow" below the list (only if needed) */
		if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
	}
}


/*
 * Display a list of the items on the floor at the given location.  -TNB-
 */
void show_floor(const int *floor_list, int floor_num, bool gold)
{
	int i, j, k, l;
	int col, len, lim;

	object_type *o_ptr;

	char o_name[80];

	char tmp_val[80];

	int out_index[MAX_FLOOR_STACK];
	byte out_color[MAX_FLOOR_STACK];
	char out_desc[MAX_FLOOR_STACK][80];


	/* Default length */
	len = 79 - 50;

	/* Maximum space allowed for descriptions */
	lim = 79 - 3;

	/* Require space for weight */
	lim -= 9;

	/* Limit displayed floor items to 23 (screen limits) */
	if (floor_num > MAX_FLOOR_STACK) floor_num = MAX_FLOOR_STACK;

	/* Display the floor */
	for (k = 0, i = 0; i < floor_num; i++)
	{
		o_ptr = &o_list[floor_list[i]];

		/* Optionally, show gold */
		if ((o_ptr->tval != TV_GOLD) || (!gold))
		{
			/* Is this item acceptable?  (always rejects gold) */
			if (!item_tester_okay(o_ptr)) continue;
		}

		/* Describe the object */
		object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Hack -- enforce max length */
		o_name[lim] = '\0';

		/* Save the index */
		out_index[k] = i;

		/* Get inventory color */
		out_color[k] = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];

		/* Save the object description */
		my_strcpy(out_desc[k], o_name, sizeof(out_desc[0]));

		/* Find the predicted "line length" */
		l = strlen(out_desc[k]) + 5;

		/* Be sure to account for the weight */
		l += 9;

		/* Maintain the maximum length */
		if (l > len) len = l;

		/* Advance to next "line" */
		k++;
	}

	/* Find the column to start in */
	col = (len > 76) ? 0 : (79 - len);

	/* Output each entry */
	for (j = 0; j < k; j++)
	{
		int wgt;

		/* Get the index */
		i = floor_list[out_index[j]];

		/* Get the item */
		o_ptr = &o_list[i];

		/* Clear the line */
		prt("", j + 1, col ? col - 2 : col);

		/* Prepare an index --(-- */
		strnfmt(tmp_val, sizeof(tmp_val), "%c)", index_to_label(out_index[j]));

		/* Clear the line with the (possibly indented) index */
		put_str(tmp_val, j + 1, col);

		/* Display the entry itself */
		c_put_str(out_color[j], out_desc[j], j + 1, col + 3);

		/* Display the weight if needed */
		wgt = o_ptr->weight * o_ptr->number;
		strnfmt(tmp_val, sizeof(tmp_val), "%3d.%1d lb", wgt / 10, wgt % 10);
		put_str(tmp_val, j + 1, 71);
	}

	/* Make a "shadow" below the list (only if needed) */
	if (j && (j < 23)) prt("", j + 1, col ? col - 2 : col);
}



/*
 * Verify the choice of an item.
 *
 * The item can be negative to mean "item on floor".
 */
bool verify_item(cptr prompt, int item)
{
	char o_name[80];

	char out_val[160];

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

	/* Prompt */
	strnfmt(out_val, sizeof(out_val), "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


/*
 * Hack -- allow user to "prevent" certain choices.
 *
 * The item can be negative to mean "item on floor".
 */
static bool get_item_allow(int item, bool is_harmless)
{
	object_type *o_ptr;
	char verify_inscrip[] = "!*";

	unsigned n;

	/* Inventory or floor */
	if (item >= 0)
		o_ptr = &inventory[item];
	else
		o_ptr = &o_list[0 - item];

	/* Check for a "prevention" inscription */
	verify_inscrip[1] = p_ptr->command_cmd;

	/* Find both sets of inscriptions, add togther, and prompt that number of times */
	n = check_for_inscrip(o_ptr, verify_inscrip);

	if (!is_harmless)
		n += check_for_inscrip(o_ptr, "!*");

	while (n--)
	{
		if (!verify_item("Really try", item))
			return (FALSE);
	}

	/* Allow it */
	return (TRUE);
}



/*
 * Find the "first" inventory object with the given "tag".
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the "current" p_ptr->command_cmd code.
 */
static int get_tag(int *cp, char tag)
{
	int i;
	cptr s;

	/* (f)ire is handled differently from all others, due to the quiver */
	if (p_ptr->command_cmd == 'f')
	{
		i = QUIVER_START + tag - '0';
		if (inventory[i].k_idx)
		{
			*cp = i;
			return (TRUE);
		}
		return (FALSE);
	}

	/* Check every object */
	for (i = 0; i < ALL_INVEN_TOTAL; ++i)
	{
		object_type *o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Skip empty inscriptions */
		if (!o_ptr->note) continue;

		/* Find a '@' */
		s = strchr(quark_str(o_ptr->note), '@');

		/* Process all tags */
		while (s)
		{
			/* Check the normal tags */
			if (s[1] == tag)
			{
				/* Save the actual inventory ID */
				*cp = i;

				/* Success */
				return (TRUE);
			}

			/* Check the special tags */
			if ((s[1] == p_ptr->command_cmd) && (s[2] == tag))
			{
				/* Save the actual inventory ID */
				*cp = i;

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
 * Let the user select an item, save its "index"
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The selected item must satisfy the "item_tester_hook()" function,
 * if that hook is set, and the "item_tester_tval", if that value is set.
 *
 * All "item_tester" restrictions are cleared before this function returns.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * The equipment or inventory are displayed (even if no acceptable
 * items are in that location) if the proper flag was given.
 *
 * If there are no acceptable items available anywhere, and "str" is
 * not NULL, then it will be used as the text of a warning message
 * before the function returns.
 *
 * Note that the user must press "-" to specify the item on the floor,
 * and there is no way to "examine" the item on the floor, while the
 * use of "capital" letters will "examine" an inventory/equipment item,
 * and prompt for its use.
 *
 * If a legal item is selected from the inventory, we save it in "cp"
 * directly (0 to 35), and return TRUE.
 *
 * If a legal item is selected from the floor, we save it in "cp" as
 * a negative (-1 to -511), and return TRUE.
 *
 * If no item is available, we do nothing to "cp", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "cp", and return FALSE.
 *
 * Global "p_ptr->command_new" is used when viewing the inventory or equipment
 * to allow the user to enter a command while viewing those screens, and
 * also to induce "auto-enter" of stores, and other such stuff.
 *
 * Global "p_ptr->command_wrk" is used to choose between equip/inven/floor
 * listings.  It is equal to USE_INVEN or USE_EQUIP or USE_FLOOR, except
 * when this function is first called, when it is equal to zero, which will
 * cause it to be set to USE_INVEN.
 *
 * We always erase the prompt when we are done, leaving a blank line,
 * or a warning message, if appropriate, if no items are available.
 *
 * Note that only "acceptable" floor objects get indexes, so between two
 * commands, the indexes of floor objects may change.  XXX XXX XXX
 */
bool get_item(int *cp, cptr pmt, cptr str, int mode)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	ui_event_data which;

	int j, k;

	int i1, i2;
	int e1, e2;
	int f1, f2;

	bool done, item;

	bool oops = FALSE;

	bool use_inven = ((mode & USE_INVEN) ? TRUE : FALSE);
	bool use_equip = ((mode & USE_EQUIP) ? TRUE : FALSE);
	bool use_floor = ((mode & USE_FLOOR) ? TRUE : FALSE);
	bool can_squelch = ((mode & CAN_SQUELCH) ? TRUE : FALSE);
	bool is_harmless = ((mode & IS_HARMLESS) ? TRUE : FALSE);

	bool allow_inven = FALSE;
	bool allow_equip = FALSE;
	bool allow_floor = FALSE;

	bool toggle = FALSE;

	char tmp_val[160];
	char out_val[160];

	int floor_list[MAX_FLOOR_STACK];
	int floor_num;

	bool show_list = OPT(show_lists) ? TRUE : FALSE;


	/* Paranoia XXX XXX XXX */
	message_flush();


	/* Not done */
	done = FALSE;

	/* No item selected */
	item = FALSE;


	/* Full inventory */
	i1 = 0;
	i2 = INVEN_PACK - 1;

	/* Forbid inventory */
	if (!use_inven) i2 = -1;

	/* Restrict inventory indexes */
	while ((i1 <= i2) && (!get_item_okay(i1))) i1++;
	while ((i1 <= i2) && (!get_item_okay(i2))) i2--;

	/* Accept inventory */
	if (i1 <= i2) allow_inven = TRUE;


	/* Full equipment */
	e1 = INVEN_WIELD;
	e2 = ALL_INVEN_TOTAL - 1;

	/* Forbid equipment */
	if (!use_equip) e2 = -1;

	/* Restrict equipment indexes */
	while ((e1 <= e2) && (!get_item_okay(e1))) e1++;
	while ((e1 <= e2) && (!get_item_okay(e2))) e2--;

	/* Accept equipment */
	if (e1 <= e2) allow_equip = TRUE;


	/* Scan all non-gold objects in the grid */
	floor_num = scan_floor(floor_list, N_ELEMENTS(floor_list), py, px, 0x03);

	/* Full floor */
	f1 = 0;
	f2 = floor_num - 1;

	/* Forbid floor */
	if (!use_floor) f2 = -1;

	/* Restrict floor indexes */
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f1]))) f1++;
	while ((f1 <= f2) && (!get_item_okay(0 - floor_list[f2]))) f2--;

	/* Accept floor */
	if (f1 <= f2) allow_floor = TRUE;


	/* Require at least one legal choice */
	if (!allow_inven && !allow_equip && !allow_floor)
	{
		/* Oops */
		oops = TRUE;
		done = TRUE;
	}

	/* Analyze choices */
	else
	{
		/* Hack -- Start on equipment if requested */
		if ((p_ptr->command_wrk == USE_EQUIP) && use_equip)
			p_ptr->command_wrk = USE_EQUIP;

		/* Use inventory if allowed */
		else if (use_inven)
			p_ptr->command_wrk = USE_INVEN;

		/* Use equipment if allowed */
		else if (use_equip)
			p_ptr->command_wrk = USE_EQUIP;

		/* Use floor if allowed */
		else if (use_floor)
			p_ptr->command_wrk = USE_FLOOR;

		/* Hack -- Use (empty) inventory */
		else
			p_ptr->command_wrk = USE_INVEN;
	}


	/* Start out in "display" mode */
	if (show_list)
	{
		/* Save screen */
		screen_save();
	}


	/* Repeat until done */
	while (!done)
	{
		int ni = 0;
		int ne = 0;

		/* Scan windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++)
		{
			/* Unused */
			if (!angband_term[j]) continue;

			/* Count windows displaying inven */
			if (op_ptr->window_flag[j] & (PW_INVEN)) ni++;

			/* Count windows displaying equip */
			if (op_ptr->window_flag[j] & (PW_EQUIP)) ne++;
		}

		/* Toggle if needed */
		if (((p_ptr->command_wrk == USE_EQUIP) && ni && !ne) ||
		    ((p_ptr->command_wrk == USE_INVEN) && !ni && ne))
		{
			/* Toggle */
			toggle_inven_equip();

			/* Track toggles */
			toggle = !toggle;
		}

		/* Redraw */
		p_ptr->redraw |= (PR_INVEN | PR_EQUIP);

		/* Redraw windows */
		redraw_stuff();

		/* Viewing inventory */
		if (p_ptr->command_wrk == USE_INVEN)
		{
			/* Redraw if needed */
			if (show_list) show_inven(((mode & SHOW_PRICES) ? OLIST_PRICE | OLIST_WEIGHT : OLIST_WEIGHT));

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Inven:");

			/* List choices */
			if (i1 <= i2)
			{
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,",
				        index_to_label(i1), index_to_label(i2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
			{
				my_strcat(out_val, " * to see,", sizeof(out_val));
				button_add("[*]", '*');
			}

			/* Indicate legality of "toggle" */
			if (use_equip)
			{
				my_strcat(out_val, " / for Equip,", sizeof(out_val));
				button_add("[/]", '/');
			}

			/* Indicate legality of the "floor" */
			if (allow_floor)
			{
				my_strcat(out_val, " - for floor,", sizeof(out_val));
				button_add("[-]", '-');
			}

			/* Indicate that squelched items can be selected */
			if (can_squelch)
			{
				my_strcat(out_val, " ! for squelched,", sizeof(out_val));
				button_add("[!]", '!');
			}
		}

		/* Viewing equipment */
		else if (p_ptr->command_wrk == USE_EQUIP)
		{
			/* Redraw if needed */
			if (show_list) show_equip(((mode & SHOW_PRICES) ? OLIST_PRICE | OLIST_WEIGHT : OLIST_WEIGHT));

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Equip:");

			/* List choices */
			if (e1 <= e2)
			{
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,",
				        index_to_label(e1), index_to_label(e2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
			{
				my_strcat(out_val, " * to see,", sizeof(out_val));
				button_add("[*]", '*');
			}

			/* Indicate legality of "toggle" */
			if (use_inven)
			{
				my_strcat(out_val, " / for Inven,", sizeof(out_val));
				button_add("[/]", '/');
			}

			/* Indicate legality of the "floor" */
			if (allow_floor)
			{
				my_strcat(out_val, " - for floor,", sizeof(out_val));
				button_add("[!]", '!');
			}
		}

		/* Viewing floor */
		else
		{
			/* Redraw if needed */
			if (show_list) show_floor(floor_list, floor_num, FALSE);

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Floor:");

			/* List choices */
			if (f1 <= f2)
			{
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(f1), I2A(f2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
			{
				my_strcat(out_val, " * to see,", sizeof(out_val));
				button_add("[*]", '*');
			}

			/* Append */
			if (use_inven)
			{
				my_strcat(out_val, " / for Inven,", sizeof(out_val));
				button_add("[/]", '/');
			}

			/* Append */
			else if (use_equip)
			{
				my_strcat(out_val, " / for Equip,", sizeof(out_val));
				button_add("[/]", '/');
			}

			/* Indicate that squelched items can be selected */
			if (can_squelch)
			{
				my_strcat(out_val, " ! for squelched,", sizeof(out_val));
				button_add("[!]", '!');
			}
		}

		redraw_stuff();

		/* Finish the prompt */
		my_strcat(out_val, " ESC", sizeof(out_val));

		/* Build the prompt */
		strnfmt(tmp_val, sizeof(tmp_val), "(%s) %s", out_val, pmt);

		/* Show the prompt */
		prt(tmp_val, 0, 0);


		/* Get a key */
		which = inkey_ex();

		/* Parse it */
		switch (which.key)
		{
			case ESCAPE:
			{
				done = TRUE;
				break;
			}

			case '*':
			case '?':
			case ' ':
			{
				if (!OPT(show_lists))
				{
					/* Hide the list */
					if (show_list)
					{
						/* Flip flag */
						show_list = FALSE;

						/* Load screen */
						screen_load();
					}

					/* Show the list */
					else
					{
						/* Save screen */
						screen_save();

						/* Flip flag */
						show_list = TRUE;
					}
				}

				break;
			}

			case '/':
			{
				/* Toggle to inventory */
				if (use_inven && (p_ptr->command_wrk != USE_INVEN))
				{
					p_ptr->command_wrk = USE_INVEN;
				}

				/* Toggle to equipment */
				else if (use_equip && (p_ptr->command_wrk != USE_EQUIP))
				{
					p_ptr->command_wrk = USE_EQUIP;
				}

				/* No toggle allowed */
				else
				{
					bell("Cannot switch item selector!");
					break;
				}


				/* Hack -- Fix screen */
				if (show_list)
				{
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				/* Need to redraw */
				break;
			}

			case '-':
			{
				/* Paranoia */
				if (!allow_floor)
				{
					bell("Cannot select floor!");
					break;
				}

				/* There is only one item */
				if (floor_num == 1)
				{
					/* Auto-select */
					if (p_ptr->command_wrk == (USE_FLOOR))
					{
						/* Special index */
						k = 0 - floor_list[0];

						/* Allow player to "refuse" certain actions */
						if (!get_item_allow(k, is_harmless))
						{
							done = TRUE;
							break;
						}

						/* Accept that choice */
						(*cp) = k;
						item = TRUE;
						done = TRUE;

						break;
					}
				}

				/* Hack -- Fix screen */
				if (show_list)
				{
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				p_ptr->command_wrk = (USE_FLOOR);

#if 0
				/* Check each legal object */
				for (i = 0; i < floor_num; ++i)
				{
					/* Special index */
					k = 0 - floor_list[i];

					/* Skip non-okay objects */
					if (!get_item_okay(k)) continue;

					/* Allow player to "refuse" certain actions */
					if (!get_item_allow(k)) continue;

					/* Accept that choice */
					(*cp) = k;
					item = TRUE;
					done = TRUE;
					break;
				}
#endif

				break;
			}

			case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			{
				/* Look up the tag */
				if (!get_tag(&k, which.key))
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Hack -- Validate the item */
				if ((k < INVEN_WIELD) ? !allow_inven : !allow_equip)
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k, is_harmless))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '\n':
			case '\r':
			{
				/* Choose "default" inventory item */
				if (p_ptr->command_wrk == USE_INVEN)
				{
					if (i1 != i2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = i1;
				}

				/* Choose the "default" slot (0) of the quiver */
				else if(p_ptr->command_cmd == 'f')
					k = e1;

				/* Choose "default" equipment item */
				else if (p_ptr->command_wrk == USE_EQUIP)
				{
					if (e1 != e2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = e1;
				}

				/* Choose "default" floor item */
				else
				{
					if (f1 != f2)
					{
						bell("Illegal object choice (default)!");
						break;
					}

					k = 0 - floor_list[f1];
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (default)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k, is_harmless))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}

			case '!':
			{
				/* Try squelched items */
				if (can_squelch)
				{
					(*cp) = ALL_SQUELCHED;
					item = TRUE;
					done = TRUE;
					break;
				}

				/* Just fall through */
			}

			default:
			{
				bool verify;

				/* Note verify */
				verify = (isupper((unsigned char)which.key) ? TRUE : FALSE);

				/* Lowercase */
				which.key = tolower((unsigned char)which.key);

				/* Convert letter to inventory index */
				if (p_ptr->command_wrk == USE_INVEN)
				{
					k = label_to_inven(which.key);

					if (k < 0)
					{
						bell("Illegal object choice (inven)!");
						break;
					}
				}

				/* Convert letter to equipment index */
				else if (p_ptr->command_wrk == USE_EQUIP)
				{
					k = label_to_equip(which.key);

					if (k < 0)
					{
						bell("Illegal object choice (equip)!");
						break;
					}
				}

				/* Convert letter to floor index */
				else
				{
					k = (islower((unsigned char)which.key) ? A2I(which.key) : -1);

					if (k < 0 || k >= floor_num)
					{
						bell("Illegal object choice (floor)!");
						break;
					}

					/* Special index */
					k = 0 - floor_list[k];
				}

				/* Validate the item */
				if (!get_item_okay(k))
				{
					bell("Illegal object choice (normal)!");
					break;
				}

				/* Verify the item */
				if (verify && !verify_item("Try", k))
				{
					done = TRUE;
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(k, is_harmless))
				{
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*cp) = k;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}


	/* Fix the screen if necessary */
	if (show_list)
	{
		/* Load screen */
		screen_load();

		/* Hack -- Cancel "display" */
		show_list = FALSE;
	}


	/* Kill buttons */
	button_kill('*');
	button_kill('/');
	button_kill('-');
	button_kill('!');
	redraw_stuff();
 
	/* Forget the item_tester_tval restriction */
	item_tester_tval = 0;

	/* Forget the item_tester_hook restriction */
	item_tester_hook = NULL;


	/* Toggle again if needed */
	if (toggle) toggle_inven_equip();

	/* Update */
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
	redraw_stuff();


	/* Clear the prompt line */
	prt("", 0, 0);

	/* Warning if needed */
	if (oops && str) msg_print(str);

	/* Result */
	return (item);
}

