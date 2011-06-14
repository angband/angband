/*
 * File: obj-ui.c
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
#include "button.h"
#include "tvalsval.h"
#include "cmds.h"

/*
 * Display a list of objects.  Each object may be prefixed with a label.
 * Used by show_inven(), show_equip(), and show_floor().  Mode flags are
 * documented in object.h
 */
static void show_obj_list(int num_obj, int num_head, char labels[50][80],
	object_type *objects[50], olist_detail_t mode)
{
	int i, row = 0, col = 0;
	int attr;
	size_t max_len = 0;
	int ex_width = 0, ex_offset, ex_offset_ctr;

	object_type *o_ptr;
	char o_name[50][80];
	char tmp_val[80];
	
	bool in_term;
	
	in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;

	if (in_term) max_len = 40;

	/* Calculate name offset and max name length */
	for (i = 0; i < num_obj; i++)
	{
		o_ptr = objects[i];

		/* Null objects are used to skip lines, or display only a label */		
		if (!o_ptr || !o_ptr->kind)
		{
			if (i < num_head)
				strnfmt(o_name[i], sizeof(o_name[i]), "");
			else
				strnfmt(o_name[i], sizeof(o_name[i]), "(nothing)");
		}
		else
			object_desc(o_name[i], sizeof(o_name[i]), o_ptr, ODESC_PREFIX | ODESC_FULL);

		/* Max length of label + object name */
		max_len = MAX(max_len, strlen(labels[i]) + strlen(o_name[i]));
	}

	/* Take the quiver message into consideration */
	if (mode & OLIST_QUIVER && p_ptr->quiver_slots > 0)
		max_len = MAX(max_len, 24);

	/* Width of extra fields */
	if (mode & OLIST_WEIGHT) ex_width += 9;
	if (mode & OLIST_PRICE) ex_width += 9;
	if (mode & OLIST_FAIL) ex_width += 10;

	/* Determine beginning row and column */
	if (in_term)
	{
		/* Term window */
		row = 0;
		col = 0;
	}
	else
	{
		/* Main window */
		row = 1;
		col = Term->wid - 1 - max_len - ex_width;

		if (col < 3) col = 0;
	}

	/* Column offset of the first extra field */
	ex_offset = MIN(max_len, (size_t)(Term->wid - 1 - ex_width - col));

	/* Output the list */
	for (i = 0; i < num_obj; i++)
	{
		o_ptr = objects[i];
		
		/* Clear the line */
		prt("", row + i, MAX(col - 2, 0));

		/* If we have no label then we won't display anything */
		if (!strlen(labels[i])) continue;

		/* Print the label */
		put_str(labels[i], row + i, col);

		/* Limit object name */
		if (strlen(labels[i]) + strlen(o_name[i]) > (size_t)ex_offset)
		{
			int truncate = ex_offset - strlen(labels[i]);
			
			if (truncate < 0) truncate = 0;
			if ((size_t)truncate > sizeof(o_name[i]) - 1) truncate = sizeof(o_name[i]) - 1;

			o_name[i][truncate] = '\0';
		}
		
		/* Item kind determines the color of the output */
		if (o_ptr && o_ptr->kind)
			attr = tval_to_attr[o_ptr->tval % N_ELEMENTS(tval_to_attr)];
		else
			attr = TERM_SLATE;

		/* Object name */
		c_put_str(attr, o_name[i], row + i, col + strlen(labels[i]));

		/* If we don't have an object, we can skip the rest of the output */
		if (!(o_ptr && o_ptr->kind)) continue;

		/* Extra fields */
		ex_offset_ctr = ex_offset;
		
		if (mode & OLIST_PRICE)
		{
			int price = price_item(o_ptr, TRUE, o_ptr->number);
			strnfmt(tmp_val, sizeof(tmp_val), "%6d au", price);
			put_str(tmp_val, row + i, col + ex_offset_ctr);
			ex_offset_ctr += 9;
		}

		if (mode & OLIST_FAIL)
		{
			int fail = (9 + get_use_device_chance(o_ptr)) / 10;
			if (object_effect_is_known(o_ptr))
				strnfmt(tmp_val, sizeof(tmp_val), "%4d%% fail", fail);
			else
				my_strcpy(tmp_val, "    ? fail", sizeof(tmp_val));
			put_str(tmp_val, row + i, col + ex_offset_ctr);
			ex_offset_ctr += 10;
		}

		if (mode & OLIST_WEIGHT)
		{
			int weight = o_ptr->weight * o_ptr->number;
			strnfmt(tmp_val, sizeof(tmp_val), "%4d.%1d lb", weight / 10, weight % 10);
			put_str(tmp_val, row + i, col + ex_offset_ctr);
			ex_offset_ctr += 9;
		}
	}

	/* For the inventory: print the quiver count */
	if (mode & OLIST_QUIVER)
	{
		int count, j;

		/* Quiver may take multiple lines */
		for(j = 0; j < p_ptr->quiver_slots; j++, i++)
		{
			const char *fmt = "in Quiver: %d missile%s";
			char letter = index_to_label(in_term ? i - 1 : i);

			/* Number of missiles in this "slot" */
			if (j == p_ptr->quiver_slots - 1 && p_ptr->quiver_remainder > 0)
				count = p_ptr->quiver_remainder;
			else
				count = 99;

			/* Clear the line */
			prt("", row + i, MAX(col - 2, 0));

			/* Print the (disabled) label */
			strnfmt(tmp_val, sizeof(tmp_val), "%c) ", letter);
			c_put_str(TERM_SLATE, tmp_val, row + i, col);

			/* Print the count */
			strnfmt(tmp_val, sizeof(tmp_val), fmt, count, count == 1 ? "" : "s");
			c_put_str(TERM_L_UMBER, tmp_val, row + i, col + 3);
		}
	}

	/* Clear term windows */
	if (in_term)
	{
		for (; i < Term->hgt; i++)
		{
			prt("", row + i, MAX(col - 2, 0));
		}
	}
	
	/* Print a drop shadow for the main window if necessary */
	else if (i > 0 && row + i < 24)
	{
		prt("", row + i, MAX(col - 2, 0));
	}
}


/*
 * Display the inventory.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_inven(olist_detail_t mode)
{
	int i, last_slot = -1;
	int diff = weight_remaining();

	object_type *o_ptr;

	int num_obj = 0;
	char labels[50][80];
	object_type *objects[50];

	bool in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;

	/* Include burden for term windows */
	if (in_term)
	{
		strnfmt(labels[num_obj], sizeof(labels[num_obj]),
		        "Burden %d.%d lb (%d.%d lb %s) ",
		        p_ptr->total_weight / 10, p_ptr->total_weight % 10,
		        abs(diff) / 10, abs(diff) % 10,
		        (diff < 0 ? "overweight" : "remaining"));

		objects[num_obj] = NULL;
		num_obj++;
	}

	/* Find the last occupied inventory slot */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		if (o_ptr->kind) last_slot = i;
	}

	/* Build the object list */
	for (i = 0; i <= last_slot; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* Acceptable items get a label */
		if (item_tester_okay(o_ptr))
			strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ", index_to_label(i));

		/* Unacceptable items are still displayed in term windows */
		else if (in_term)
			my_strcpy(labels[num_obj], "   ", sizeof(labels[num_obj]));

		/* Unacceptable items are skipped in the main window */
		else continue;

		/* Save the object */
		objects[num_obj] = o_ptr;
		num_obj++;
	}

	/* Display the object list */
	if (in_term)
		/* Term window starts with a burden header */
		show_obj_list(num_obj, 1, labels, objects, mode);
	else
		show_obj_list(num_obj, 0, labels, objects, mode);
}


/*
 * Display the equipment.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_equip(olist_detail_t mode)
{
	int i, last_slot = 0;

	object_type *o_ptr;

   int num_obj = 0;
   char labels[50][80];
   object_type *objects[50];

	char tmp_val[80];

   bool in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;

	/* Find the last equipment slot to display */
	for (i = INVEN_WIELD; i < ALL_INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		if (i < INVEN_TOTAL || o_ptr->kind) last_slot = i;
	}

	/* Build the object list */
	for (i = INVEN_WIELD; i <= last_slot; i++)
	{
		o_ptr = &p_ptr->inventory[i];

		/* May need a blank line to separate the quiver */
		if (i == INVEN_TOTAL)
		{
			int j;
			bool need_spacer = FALSE;
			
			/* Scan the rest of the items for acceptable entries */
			for (j = i; j < last_slot; j++)
			{
				o_ptr = &p_ptr->inventory[j];
				if (item_tester_okay(o_ptr)) need_spacer = TRUE;
			}

			/* Add a spacer between equipment and quiver */
			if (num_obj > 0 && need_spacer)
			{
				my_strcpy(labels[num_obj], "", sizeof(labels[num_obj]));
				objects[num_obj] = NULL;
				num_obj++;
			}

			continue;
		}

		/* Acceptable items get a label */
		if (item_tester_okay(o_ptr))
			strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ", index_to_label(i));

		/* Unacceptable items are still displayed in term windows */
		else if (in_term)
			my_strcpy(labels[num_obj], "   ", sizeof(labels[num_obj]));

		/* Unacceptable items are skipped in the main window */
		else continue;

		/* Show full slot labels */
		strnfmt(tmp_val, sizeof(tmp_val), "%-14s: ", mention_use(i));
		my_strcat(labels[num_obj], tmp_val, sizeof(labels[num_obj]));

		/* Save the object */
		objects[num_obj] = o_ptr;
		num_obj++;
	}

	/* Display the object list */
	show_obj_list(num_obj, 0, labels, objects, mode);
}


/*
 * Display the floor.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_floor(const int *floor_list, int floor_num, olist_detail_t mode)
{
	int i;

	object_type *o_ptr;

   int num_obj = 0;
   char labels[50][80];
   object_type *objects[50];

	if (floor_num > MAX_FLOOR_STACK) floor_num = MAX_FLOOR_STACK;

	/* Build the object list */
	for (i = 0; i < floor_num; i++)
	{
		o_ptr = object_byid(floor_list[i]);

		/* Tester always skips gold. When gold should be displayed,
		 * only test items that are not gold.
		 */
		if ((o_ptr->tval != TV_GOLD || !(mode & OLIST_GOLD)) &&
		    !item_tester_okay(o_ptr))
			continue;

		strnfmt(labels[num_obj], sizeof(labels[num_obj]),
		        "%c) ", index_to_label(i));

		/* Save the object */
		objects[num_obj] = o_ptr;
		num_obj++;
	}

	/* Display the object list */
	show_obj_list(num_obj, 0, labels, objects, mode);
}


/*
 * Verify the choice of an item.
 *
 * The item can be negative to mean "item on floor".
 */
bool verify_item(const char *prompt, int item)
{
	char o_name[80];

	char out_val[160];

	object_type *o_ptr;

	/* Inventory */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Floor */
	else
	{
		o_ptr = object_byid(0 - item);
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
static bool get_item_allow(int item, unsigned char ch, bool is_harmless)
{
	object_type *o_ptr;
	char verify_inscrip[] = "!*";

	unsigned n;

	/* Inventory or floor */
	if (item >= 0)
		o_ptr = &p_ptr->inventory[item];
	else
		o_ptr = object_byid(0 - item);

	/* Check for a "prevention" inscription */
	verify_inscrip[1] = ch;

	/* Find both sets of inscriptions, add together, and prompt that number of times */
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
 * and "x" is the action that tag will work for.
 */
static int get_tag(int *cp, char tag, cmd_code cmd, bool quiver_tags)
{
	int i;
	const char *s;

	/* (f)ire is handled differently from all others, due to the quiver */
	if (quiver_tags)
	{
		i = QUIVER_START + tag - '0';
		if (p_ptr->inventory[i].kind)
		{
			*cp = i;
			return (TRUE);
		}
		return (FALSE);
	}

	/* Check every object */
	for (i = 0; i < ALL_INVEN_TOTAL; ++i)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

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
			if ((cmd_lookup(s[1]) == cmd) && (s[2] == tag))
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
bool get_item(int *cp, const char *pmt, const char *str, cmd_code cmd, int mode)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	unsigned char cmdkey = cmd_lookup_key(cmd);

	struct keypress which;

	int j, k;

	int i1, i2;
	int e1, e2;
	int f1, f2;

	bool done, item;

	bool oops = FALSE;

	bool use_inven = ((mode & USE_INVEN) ? TRUE : FALSE);
	bool use_equip = ((mode & USE_EQUIP) ? TRUE : FALSE);
	bool use_floor = ((mode & USE_FLOOR) ? TRUE : FALSE);
	bool use_quiver = ((mode & QUIVER_TAGS) ? TRUE : FALSE);
	bool is_harmless = ((mode & IS_HARMLESS) ? TRUE : FALSE);
	bool quiver_tags = ((mode & QUIVER_TAGS) ? TRUE : FALSE);

	olist_detail_t olist_mode = 0;

	bool allow_inven = FALSE;
	bool allow_equip = FALSE;
	bool allow_floor = FALSE;

	bool toggle = FALSE;

	char tmp_val[160];
	char out_val[160];

	int floor_list[MAX_FLOOR_STACK];
	int floor_num;

	bool show_list = TRUE;


	/* Object list display modes */
	if (mode & SHOW_FAIL)
		olist_mode |= (OLIST_FAIL);
	else
		olist_mode |= (OLIST_WEIGHT);
	if (mode & SHOW_PRICES)
		olist_mode |= (OLIST_PRICE);

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

		/* If we are using the quiver then start on equipment */
		else if (use_quiver)
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
		redraw_stuff(p_ptr);

		/* Viewing inventory */
		if (p_ptr->command_wrk == USE_INVEN)
		{
			/* Redraw if needed */
			if (show_list) show_inven(olist_mode);

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
		}

		/* Viewing equipment */
		else if (p_ptr->command_wrk == USE_EQUIP)
		{
			/* Redraw if needed */
			if (show_list) show_equip(olist_mode);

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
			if (show_list) show_floor(floor_list, floor_num, olist_mode);

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
		}

		redraw_stuff(p_ptr);

		/* Finish the prompt */
		my_strcat(out_val, " ESC", sizeof(out_val));

		/* Build the prompt */
		strnfmt(tmp_val, sizeof(tmp_val), "(%s) %s", out_val, pmt);

		/* Show the prompt */
		prt(tmp_val, 0, 0);


		/* Get a key */
		which = inkey();

		/* Parse it */
		switch (which.code)
		{
			case ESCAPE:
			{
				done = TRUE;
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
						if (!get_item_allow(k, cmdkey, is_harmless))
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
					if (!get_item_allow(k, cmdkey, is_harmless)) continue;

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
				if (!get_tag(&k, which.code, cmd, quiver_tags))
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
				if (!get_item_allow(k, cmdkey, is_harmless))
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
				else if (quiver_tags)
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
				if (!get_item_allow(k, cmdkey, is_harmless))
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

			default:
			{
				bool verify;

				/* Note verify */
				verify = (isupper((unsigned char)which.code) ? TRUE : FALSE);

				/* Lowercase */
				which.code = tolower((unsigned char)which.code);

				/* Convert letter to inventory index */
				if (p_ptr->command_wrk == USE_INVEN)
				{
					k = label_to_inven(which.code);

					if (k < 0)
					{
						bell("Illegal object choice (inven)!");
						break;
					}
				}

				/* Convert letter to equipment index */
				else if (p_ptr->command_wrk == USE_EQUIP)
				{
					k = label_to_equip(which.code);

					if (k < 0)
					{
						bell("Illegal object choice (equip)!");
						break;
					}
				}

				/* Convert letter to floor index */
				else
				{
					k = (islower((unsigned char)which.code) ? A2I((unsigned char)which.code) : -1);

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
				if (!get_item_allow(k, cmdkey, is_harmless))
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
	redraw_stuff(p_ptr);
 
	/* Forget the item_tester_tval restriction */
	item_tester_tval = 0;

	/* Forget the item_tester_hook restriction */
	item_tester_hook = NULL;


	/* Toggle again if needed */
	if (toggle) toggle_inven_equip();

	/* Update */
	p_ptr->redraw |= (PR_INVEN | PR_EQUIP);
	redraw_stuff(p_ptr);


	/* Clear the prompt line */
	prt("", 0, 0);

	/* Warning if needed */
	if (oops && str) msg("%s", str);

	/* Result */
	return (item);
}

