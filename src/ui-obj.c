/**
 * \file ui-obj.c
 * \brief lists of objects and object pictures
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
#include "cmd-core.h"
#include "cmds.h"
#include "game-input.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-info.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "store.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-obj.h"
#include "ui-output.h"
#include "ui-prefs.h"

/**
 * Determine if the attr and char should consider the item's flavor
 *
 * Identified scrolls should use their own tile.
 */
static bool use_flavor_glyph(const struct object_kind *kind)
{
	return kind->flavor && !(kind->tval == TV_SCROLL && kind->aware);
}

/**
 * Return the "attr" for a given item kind.
 * Use "flavor" if available.
 * Default to user definitions.
 */
byte object_kind_attr(const struct object_kind *kind)
{
	return use_flavor_glyph(kind) ? flavor_x_attr[kind->flavor->fidx] :
		kind_x_attr[kind->kidx];
}

/**
 * Return the "char" for a given item kind.
 * Use "flavor" if available.
 * Default to user definitions.
 */
wchar_t object_kind_char(const struct object_kind *kind)
{
	return use_flavor_glyph(kind) ? flavor_x_char[kind->flavor->fidx] :
		kind_x_char[kind->kidx];
}

/**
 * Return the "attr" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
byte object_attr(const struct object *o_ptr)
{
	return object_kind_attr(o_ptr->kind);
}

/**
 * Return the "char" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
wchar_t object_char(const struct object *o_ptr)
{
	return object_kind_char(o_ptr->kind);
}

/**
 * Convert a label into an item in the inventory.
 *
 * Return NULL if the label does not indicate a real item.
 */
struct object *label_to_inven(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > z_info->pack_size)) return NULL;

	/* Return the object */
	return player->upkeep->inven[i];
}


/**
 * Convert a label into an item in the equipment.
 *
 * Return NULL if the label does not indicate a real item.
 */
struct object *label_to_equip(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i >= player->body.count))
		return NULL;

	/* Return the object */
	return slot_object(player, i);
}


/**
 * Convert a label into an item in the equipment or quiver.
 *
 * Return NULL if the label does not indicate a real item.
 */
struct object *label_to_quiver(int c)
{
	int i;

	/* Convert */
	i = (islower((unsigned char)c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i >= z_info->quiver_size))
		return NULL;

	/* Return the index */
	return player->upkeep->quiver[i];
}



/**
 * Display a list of objects.  Each object may be prefixed with a label.
 * Used by show_inven(), show_equip(), and show_floor().  Mode flags are
 * documented in object.h
 */
static void show_obj_list(int num_obj, int num_head, char labels[50][80],
						  struct object *objects[50], olist_detail_t mode)
{
	int i, row = 0, col = 0;
	int attr;
	size_t max_len = 0;
	int ex_width = 0, ex_offset, ex_offset_ctr;

	struct object *obj;
	char o_name[50][80];
	char tmp_val[80];
	
	bool in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;
	bool terse = FALSE;

	if (in_term) max_len = 40;
	if (in_term && Term->wid < 40) mode &= ~(OLIST_WEIGHT);

	if (Term->wid < 50) terse = TRUE;

	/* Calculate name offset and max name length */
	for (i = 0; i < num_obj; i++) {
		obj = objects[i];

		/* Null objects are used to skip lines, or display only a label */		
		if (!obj) {
			if ((i < num_head) || !strcmp(labels[i], "In quiver"))
				strnfmt(o_name[i], sizeof(o_name[i]), "");
			else
				strnfmt(o_name[i], sizeof(o_name[i]), "(nothing)");
		} else {
			if (terse)
				object_desc(o_name[i], sizeof(o_name[i]), obj,
							ODESC_PREFIX | ODESC_FULL | ODESC_TERSE);
			else
				object_desc(o_name[i], sizeof(o_name[i]), obj,
							ODESC_PREFIX | ODESC_FULL);
		}

		/* Max length of label + object name */
		max_len = MAX(max_len, strlen(labels[i]) + strlen(o_name[i]));
	}

	/* Take the quiver message into consideration */
	if (mode & OLIST_QUIVER && player->upkeep->quiver[0] != NULL)
		max_len = MAX(max_len, 24);

	/* Width of extra fields */
	if (mode & OLIST_WEIGHT) ex_width += 9;
	if (mode & OLIST_PRICE) ex_width += 9;
	if (mode & OLIST_FAIL) ex_width += 10;

	/* Determine beginning row and column */
	if (in_term) {
		/* Term window */
		row = 0;
		col = 0;
	} else {
		/* Main window */
		row = 1;
		col = Term->wid - 1 - max_len - ex_width;

		if (col < 3) col = 0;
	}

	/* Column offset of the first extra field */
	ex_offset = MIN(max_len, (size_t)(Term->wid - 1 - ex_width - col));

	/* Output the list */
	for (i = 0; i < num_obj; i++) {
		obj = objects[i];
		
		/* Clear the line */
		prt("", row + i, MAX(col - 2, 0));

		/* If we have no label then we won't display anything */
		if (!strlen(labels[i])) continue;

		/* Print the label */
		put_str(labels[i], row + i, col);

		/* Limit object name */
		if (strlen(labels[i]) + strlen(o_name[i]) > (size_t)ex_offset) {
			int truncate = ex_offset - strlen(labels[i]);
			
			if (truncate < 0) truncate = 0;
			if ((size_t)truncate > sizeof(o_name[i]) - 1)
				truncate = sizeof(o_name[i]) - 1;

			o_name[i][truncate] = '\0';
		}
		
		/* Item kind determines the color of the output */
		if (obj)
			attr = obj->kind->base->attr;
		else
			attr = COLOUR_SLATE;

		/* Object name */
		c_put_str(attr, o_name[i], row + i, col + strlen(labels[i]));

		/* If we don't have an object, we can skip the rest of the output */
		if (!obj) continue;

		/* Extra fields */
		ex_offset_ctr = ex_offset;
		
		if (mode & OLIST_PRICE) {
			struct store *store = store_at(cave, player->py, player->px);
			if (store) {
				int price = price_item(store, obj, TRUE, obj->number);

				strnfmt(tmp_val, sizeof(tmp_val), "%6d au", price);
				put_str(tmp_val, row + i, col + ex_offset_ctr);
				ex_offset_ctr += 9;
			}
		}

		if (mode & OLIST_FAIL && obj_can_fail(obj)) {
			int fail = (9 + get_use_device_chance(obj)) / 10;
			if (object_effect_is_known(obj))
				strnfmt(tmp_val, sizeof(tmp_val), "%4d%% fail", fail);
			else
				my_strcpy(tmp_val, "    ? fail", sizeof(tmp_val));
			put_str(tmp_val, row + i, col + ex_offset_ctr);
			ex_offset_ctr += 10;
		}

		if (mode & OLIST_WEIGHT) {
			int weight = obj->weight * obj->number;
			strnfmt(tmp_val, sizeof(tmp_val), "%4d.%1d lb",
					weight / 10, weight % 10);
			put_str(tmp_val, row + i, col + ex_offset_ctr);
			ex_offset_ctr += 9;
		}
	}

	/* For the inventory: print the quiver count */
	if (mode & OLIST_QUIVER) {
		int count, j;
		int quiver_slots = player->upkeep->quiver_cnt / (z_info->stack_size);

		/* Quiver may take multiple lines */
		for (j = 0; j < quiver_slots; j++, i++) {
			const char *fmt = "in Quiver: %d missile%s";
			char letter = I2A(in_term ? i - 1 : i);

			/* Number of missiles in this "slot" */
			if (j == quiver_slots - 1)
				count = player->upkeep->quiver_cnt % (z_info->stack_size);
			else
				count = z_info->stack_size;

			/* Clear the line */
			prt("", row + i, MAX(col - 2, 0));

			/* Print the (disabled) label */
			strnfmt(tmp_val, sizeof(tmp_val), "%c) ", letter);
			c_put_str(COLOUR_SLATE, tmp_val, row + i, col);

			/* Print the count */
			strnfmt(tmp_val, sizeof(tmp_val), fmt, count,
					count == 1 ? "" : "s");
			c_put_str(COLOUR_L_UMBER, tmp_val, row + i, col + 3);
		}
	}

	/* Clear term windows */
	if (in_term) {
		for (; i < Term->hgt; i++)
			prt("", row + i, MAX(col - 2, 0));
	} else if (i > 0 && row + i < 24) {
		/* Print a drop shadow for the main window if necessary */
		prt("", row + i, MAX(col - 2, 0));
	}
}

/**
 * Display the inventory.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_inven(int mode, item_tester tester)
{
	int i, last_slot = -1;
	int diff = weight_remaining();

	object_type *o_ptr;

	int num_obj = 0;
	char labels[50][80];
	object_type *objects[50];

	bool in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;

	/* Include burden for term windows */
	if (in_term) {
		strnfmt(labels[num_obj], sizeof(labels[num_obj]),
		        "Burden %d.%d lb (%d.%d lb %s) ",
		        player->upkeep->total_weight / 10,
				player->upkeep->total_weight % 10,
		        abs(diff) / 10, abs(diff) % 10,
		        (diff < 0 ? "overweight" : "remaining"));

		objects[num_obj] = NULL;
		num_obj++;
	}

	/* Find the last occupied inventory slot */
	for (i = 0; i < z_info->pack_size; i++)
		if (player->upkeep->inven[i] != NULL) last_slot = i;

	/* Build the object list */
	for (i = 0; i <= last_slot; i++) {
		o_ptr = player->upkeep->inven[i];

		/* Acceptable items get a label */
		if (object_test(tester, o_ptr))
			strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ", I2A(i));

		/* Unacceptable items are still sometimes shown */
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


/**
 * Display the quiver.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_quiver(int mode, item_tester tester)
{
	int i, last_slot = -1;

	struct object *obj;

	int num_obj = 0;
	char labels[50][80];
	struct object *objects[50];

	bool in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;

	/* Find the last occupied quiver slot */
	for (i = 0; i < z_info->quiver_size; i++)
		if (player->upkeep->quiver[i] != NULL) last_slot = i;

	/* Build the object list */
	for (i = 0; i <= last_slot; i++) {
		obj = player->upkeep->quiver[i];

		/* Acceptable items get a label */
		if (object_test(tester, obj))
			strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ", I2A(i));

		/* Unacceptable items are still sometimes shown */
		else if (in_term)
			my_strcpy(labels[num_obj], "   ", sizeof(labels[num_obj]));

		/* Unacceptable items are skipped in the main window */
		else continue;

		/* Save the object */
		objects[num_obj] = obj;
		num_obj++;
	}

	/* Display the object list */
	show_obj_list(num_obj, 0, labels, objects, mode);
}


/**
 * Display the equipment.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_equip(int mode, item_tester tester)
{
	int i;

	struct object *obj;

	int num_obj = 0;
	char labels[50][80];
	struct object *objects[50];

	char tmp_val[80];

	bool in_term = (mode & OLIST_WINDOW) ? TRUE : FALSE;
	bool show_empty = (mode & OLIST_SEMPTY) ? TRUE : FALSE;

	/* Build the object list */
	for (i = 0; i < player->body.count; i++) {
		obj = slot_object(player, i);

		/* Acceptable items get a label */
		if (object_test(tester, obj))
			strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ", I2A(i));

		/* Unacceptable items are still sometimes shown */
		else if ((!obj && show_empty) || in_term)
			my_strcpy(labels[num_obj], "   ", sizeof(labels[num_obj]));

		/* Unacceptable items are skipped in the main window */
		else continue;

		/* Show full slot labels */
		strnfmt(tmp_val, sizeof(tmp_val), "%-14s: ", equip_mention(player, i));
		my_strcap(tmp_val);
		my_strcat(labels[num_obj], tmp_val, sizeof(labels[num_obj]));

		/* Save the object */
		objects[num_obj] = obj;
		num_obj++;
	}

	/* Show the quiver in subwindows */
	if (in_term) {
		int last_slot = -1;

		strnfmt(labels[num_obj], sizeof(labels[num_obj]), "In quiver");
		objects[num_obj] = NULL;
		num_obj++;

		/* Find the last occupied quiver slot */
		for (i = 0; i < z_info->quiver_size; i++)
			if (player->upkeep->quiver[i] != NULL) last_slot = i;

		/* Extend the object list */
		for (i = 0; i <= last_slot; i++) {
			obj = player->upkeep->quiver[i];

			/* Acceptable items get a label */
			if (object_test(tester, obj))
				strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ",
						I2A(i));

			/* Unacceptable items are still sometimes shown */
			else if (in_term)
				my_strcpy(labels[num_obj], "   ", sizeof(labels[num_obj]));

			/* Unacceptable items are skipped in the main window */
			else continue;

			/* Show full slot labels */
			strnfmt(tmp_val, sizeof(tmp_val), "Slot %-9d: ", i);
			my_strcat(labels[num_obj], tmp_val, sizeof(labels[num_obj]));

			/* Save the object */
			objects[num_obj] = obj;
			num_obj++;
		}
	}

	/* Display the object list */
	show_obj_list(num_obj, 0, labels, objects, mode);
}


/**
 * Display the floor.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_floor(struct object **floor_list, int floor_num, int mode, item_tester tester)
{
	int i;

	struct object *obj;

	int num_obj = 0;
	char labels[50][80];
	struct object *objects[50];

	if (floor_num > z_info->floor_size)
		floor_num = z_info->floor_size;

	/* Build the object list */
	for (i = 0; i < floor_num; i++) {
		obj = (struct object *) floor_list[i];

		/* Tester always skips gold. When gold should be displayed,
		 * only test items that are not gold.
		 */
		if ((!tval_is_money(obj) || !(mode & OLIST_GOLD)) &&
		    !object_test(tester, obj))
			continue;

		strnfmt(labels[num_obj], sizeof(labels[num_obj]), "%c) ", I2A(i));

		/* Save the object */
		objects[num_obj] = obj;
		num_obj++;
	}

	/* Display the object list */
	show_obj_list(num_obj, 0, labels, objects, mode);
}


/**
 * Prevent certain choices depending on the inscriptions on the item.
 *
 * The item can be negative to mean "item on floor".
 */
bool get_item_allow(const struct object *obj, unsigned char ch, cmd_code cmd,
					bool is_harmless)
{
	char verify_inscrip[] = "!*";

	unsigned n;

	/* Hack - Only shift the command key if it actually needs to be shifted. */
	if (ch < 0x20)
		ch = UN_KTRL(ch);

	/* The inscription to look for */
	verify_inscrip[1] = ch;

	/* Look for the inscription */
	n = check_for_inscrip(obj, verify_inscrip);

	/* Also look for for the inscription '!*' */
	if (!is_harmless)
		n += check_for_inscrip(obj, "!*");

	/* Choose string for the prompt */
	if (n) {
		char prompt[1024];

		const char *verb = cmd_verb(cmd);
		if (!verb)
			verb = "do that with";

		strnfmt(prompt, sizeof(prompt), "Really %s", verb);

		/* Prompt for confirmation n times */
		while (n--) {
			if (!verify_object(prompt, (struct object *) obj))
				return (FALSE);
		}
	}

	/* Allow it */
	return (TRUE);
}



/**
 * Find the "first" inventory object with the given "tag" - now first in the
 * gear array, which is really arbitrary - NRM.
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the action that tag will work for.
 */
static int get_tag(struct object **tagged_obj, char tag, cmd_code cmd,
				   bool quiver_tags)
{
	int i;
	struct object *obj;
	const char *s;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	/* (f)ire is handled differently from all others, due to the quiver */
	if (quiver_tags) {
		i = tag - '0';
		if (player->upkeep->quiver[i]) {
			*tagged_obj = player->upkeep->quiver[i];
			return TRUE;
		}
		return FALSE;
	}

	/* Check every object */
	for (obj = player->gear; obj; obj = obj->next) {
		/* Skip non-objects */
		assert(obj->kind);

		/* Skip empty inscriptions */
		if (!obj->note) continue;

		/* Find a '@' */
		s = strchr(quark_str(obj->note), '@');

		/* Process all tags */
		while (s) {
			unsigned char cmdkey;

			/* Check the normal tags */
			if (s[1] == tag) {
				/* Save the actual object */
				*tagged_obj = obj;

				/* Success */
				return TRUE;
			}

			cmdkey = cmd_lookup_key(cmd, mode);

			/* Hack - Only shift the command key if it actually needs to be. */
			if (cmdkey < 0x20)
				cmdkey = UN_KTRL(cmdkey);

			/* Check the special tags */
			if ((s[1] == cmdkey) && (s[2] == tag)) {
				/* Save the actual inventory ID */
				*tagged_obj = obj;

				/* Success */
				return TRUE;
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return FALSE;
}



/**
 * Let the user select an object, save its address
 *
 * Return TRUE only if an acceptable item was chosen by the user.
 *
 * The user is allowed to choose acceptable items from the equipment,
 * inventory, quiver, or floor, respectively, if the proper flag was given,
 * and there are any acceptable items in that location.
 *
 * The equipment, inventory or quiver are displayed (even if no acceptable
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
 * If a legal item is selected , we save it in "obj" and return TRUE.
 *
 * If no item is available, we do nothing to "obj", and we display a
 * warning message, using "str" if available, and return FALSE.
 *
 * If no item is selected, we do nothing to "obj", and return FALSE.
 *
 * Global "player->upkeep->command_wrk" is used to choose between
 * equip/inven/quiver/floor listings.  It is equal to USE_INVEN or USE_EQUIP or
 * USE_QUIVER or USE_FLOOR, except when this function is first called, when it
 * is equal to zero, which will cause it to be set to USE_INVEN.
 *
 * We always erase the prompt when we are done, leaving a blank line,
 * or a warning message, if appropriate, if no items are available.
 *
 * Note that only "acceptable" floor objects get indexes, so between two
 * commands, the indexes of floor objects may change.  XXX XXX XXX
 */
bool textui_get_item(struct object **choice, const char *pmt, const char *str,
					 cmd_code cmd, item_tester tester, int mode)
{
	int py = player->py;
	int px = player->px;
	unsigned char cmdkey = cmd_lookup_key(cmd,
			OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG);

	ui_event press;

	int j;
	struct object *obj;

	int i1, i2;
	int e1, e2;
	int f1, f2;
	int q1, q2;

	bool done, item;

	bool oops = FALSE;

	bool use_inven = ((mode & USE_INVEN) ? TRUE : FALSE);
	bool use_equip = ((mode & USE_EQUIP) ? TRUE : FALSE);
	bool use_quiver = ((mode & USE_QUIVER) ? TRUE : FALSE);
	bool use_floor = ((mode & USE_FLOOR) ? TRUE : FALSE);
	bool is_harmless = ((mode & IS_HARMLESS) ? TRUE : FALSE);
	bool quiver_tags = ((mode & QUIVER_TAGS) ? TRUE : FALSE);

	int olist_mode = 0;

	bool allow_inven = FALSE;
	bool allow_equip = FALSE;
	bool allow_quiver = FALSE;
	bool allow_floor = FALSE;

	bool toggle = FALSE;

	char tmp_val[160];
	char out_val[160];

	int floor_max = z_info->floor_size;
	struct object **floor_list = mem_zalloc(floor_max * sizeof(*floor_list));
	int floor_num;

	bool show_list = TRUE;

	/* Hack - Only shift the command key if it actually needs to be shifted. */
	if (cmdkey < 0x20)
		cmdkey = UN_KTRL(cmdkey);

	/* Object list display modes */
	if (mode & SHOW_FAIL)
		olist_mode |= OLIST_FAIL;
	else
		olist_mode |= OLIST_WEIGHT;

	if (mode & SHOW_PRICES)
		olist_mode |= OLIST_PRICE;

	if (mode & SHOW_EMPTY)
		olist_mode |= OLIST_SEMPTY;

	/* Paranoia XXX XXX XXX */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Not done */
	done = FALSE;

	/* No item selected */
	item = FALSE;

	/* Full inventory */
	i1 = 0;
	i2 = z_info->pack_size - 1;

	/* Forbid inventory */
	if (!use_inven) i2 = -1;

	/* Restrict inventory indexes */
	while ((i1 <= i2) && (!object_test(tester, player->upkeep->inven[i1])))
		i1++;
	while ((i1 <= i2) && (!object_test(tester, player->upkeep->inven[i2])))
		i2--;

	/* Accept inventory */
	if (i1 <= i2) allow_inven = TRUE;

	/* Full equipment */
	e1 = 0;
	e2 = player->body.count - 1;

	/* Forbid equipment */
	if (!use_equip) e2 = -1;

	/* Restrict equipment indexes */
	while ((e1 <= e2) && (!object_test(tester, slot_object(player, e1)))) e1++;
	while ((e1 <= e2) && (!object_test(tester, slot_object(player, e2)))) e2--;

	/* Accept equipment */
	if (e1 <= e2) allow_equip = TRUE;

	/* Restrict quiver indexes */
	q1 = 0;
	q2 = z_info->quiver_size - 1;

	/* Forbid quiver */
	if (!use_quiver) q2 = -1;

	/* Restrict quiver indexes */
	while ((q1 <= q2) && (!object_test(tester, player->upkeep->quiver[q1])))
		q1++;
	while ((q1 <= q2) && (!object_test(tester, player->upkeep->quiver[q2])))
		q2--;

	/* Accept quiver */
	if (q1 <= q2) allow_quiver = TRUE;

	/* Scan all non-gold objects in the grid */
	floor_num = scan_floor(floor_list, floor_max, py, px, 0x0B, tester);

	/* Full floor */
	f1 = 0;
	f2 = floor_num - 1;

	/* Forbid floor */
	if (!use_floor) f2 = -1;

	/* Restrict floor indexes */
	while ((f1 <= f2) && (!object_test(tester, floor_list[f1]))) f1++;
	while ((f1 <= f2) && (!object_test(tester, floor_list[f2]))) f2--;

	/* Accept floor */
	if (f1 <= f2) allow_floor = TRUE;

	/* Require at least one legal choice */
	if (!allow_inven && !allow_equip && !allow_quiver && !allow_floor) {
		/* Oops */
		oops = TRUE;
		done = TRUE;
	} else {
		/* Start where requested if possible */
		if ((player->upkeep->command_wrk == USE_EQUIP) && allow_equip)
			player->upkeep->command_wrk = USE_EQUIP;
		else if ((player->upkeep->command_wrk == USE_INVEN) && allow_inven)
			player->upkeep->command_wrk = USE_INVEN;
		else if ((player->upkeep->command_wrk == USE_QUIVER) && allow_quiver)
			player->upkeep->command_wrk = USE_QUIVER;
		else if ((player->upkeep->command_wrk == USE_FLOOR) && allow_floor)
			player->upkeep->command_wrk = USE_FLOOR;

		/* If we are obviously using the quiver then start on quiver */
		else if (quiver_tags && allow_quiver)
			player->upkeep->command_wrk = USE_QUIVER;

		/* Otherwise choose whatever is allowed */
		else if (use_inven && allow_inven)
			player->upkeep->command_wrk = USE_INVEN;
		else if (use_equip && allow_equip)
			player->upkeep->command_wrk = USE_EQUIP;
		else if (use_quiver && allow_quiver)
			player->upkeep->command_wrk = USE_QUIVER;
		else if (use_floor && allow_floor)
			player->upkeep->command_wrk = USE_FLOOR;

		/* If nothing to choose, use (empty) inventory */
		else
			player->upkeep->command_wrk = USE_INVEN;
	}


	/* Start out in "display" mode */
	if (show_list)
		/* Save screen */
		screen_save();


	/* Repeat until done */
	while (!done) {
		int ni = 0;
		int ne = 0;

		/* Scan windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++) {
			/* Unused */
			if (!angband_term[j]) continue;

			/* Count windows displaying inven */
			if (window_flag[j] & (PW_INVEN)) ni++;

			/* Count windows displaying equip */
			if (window_flag[j] & (PW_EQUIP)) ne++;
		}

		/* Toggle if needed */
		if (((player->upkeep->command_wrk == USE_EQUIP) && ni && !ne) ||
		    ((player->upkeep->command_wrk == USE_INVEN) && !ni && ne)) {
			/* Toggle */
			toggle_inven_equip();

			/* Track toggles */
			toggle = !toggle;
		}

		/* Redraw */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

		/* Redraw windows */
		redraw_stuff(player->upkeep);

		/* Viewing inventory */
		if (player->upkeep->command_wrk == USE_INVEN) {
			int nmode = olist_mode;

			/* Show the quiver counts in certain cases, like the 'i' command */
			if (mode & SHOW_QUIVER)
				nmode |= OLIST_QUIVER;

			/* Redraw if needed */
			if (show_list)
				show_inven(nmode, tester);

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Inven:");

			/* List choices */
			if (i1 <= i2) {
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(i1), I2A(i2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
				my_strcat(out_val, " * to see,", sizeof(out_val));

			/* Indicate legality of equipment */
			if (use_equip)
				my_strcat(out_val, " / for Equip,", sizeof(out_val));

			/* Indicate legality of quiver */
			if (use_quiver)
				my_strcat(out_val, " . for Quiver,", sizeof(out_val));

			/* Indicate legality of the "floor" */
			if (allow_floor)
				my_strcat(out_val, " - for floor,", sizeof(out_val));
		}

		/* Viewing equipment */
		else if (player->upkeep->command_wrk == USE_EQUIP) {
			/* Redraw if needed */
			if (show_list)
				show_equip(olist_mode, tester);

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Equip:");

			/* List choices */
			if (e1 <= e2) {
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(e1), I2A(e2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
				my_strcat(out_val, " * to see,", sizeof(out_val));

			/* Indicate legality of inventory */
			if (use_inven)
				my_strcat(out_val, " / for Inven,", sizeof(out_val));

			/* Indicate legality of quiver */
			if (use_quiver)
				my_strcat(out_val, " . for Quiver,", sizeof(out_val));

			/* Indicate legality of the "floor" */
			if (allow_floor)
				my_strcat(out_val, " - for floor,", sizeof(out_val));
		}

		/* Viewing quiver */
		else if (player->upkeep->command_wrk == USE_QUIVER) {
			/* Redraw if needed */
			if (show_list)
				show_quiver(olist_mode, tester);

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Quiver:");

			/* List choices */
			if (q1 <= q2) {
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(q1), I2A(q2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
				my_strcat(out_val, " * to see,", sizeof(out_val));

			/* Indicate legality of inventory */
			if (use_inven)
				my_strcat(out_val, " / for Inven,", sizeof(out_val));

			/* Indicate legality of the "floor" */
			if (allow_floor)
				my_strcat(out_val, " - for floor,", sizeof(out_val));
		}

		/* Viewing floor */
		else {
			/* Redraw if needed */
			if (show_list)
				show_floor(floor_list, floor_num, olist_mode, tester);

			/* Begin the prompt */
			strnfmt(out_val, sizeof(out_val), "Floor:");

			/* List choices */
			if (f1 <= f2) {
				/* Build the prompt */
				strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(f1), I2A(f2));

				/* Append */
				my_strcat(out_val, tmp_val, sizeof(out_val));
			}

			/* Indicate ability to "view" */
			if (!show_list)
				my_strcat(out_val, " * to see,", sizeof(out_val));

			/* Append */
			if (use_inven)
				my_strcat(out_val, " / for Inven,", sizeof(out_val));

			/* Append */
			else if (use_equip)
				my_strcat(out_val, " / for Equip,", sizeof(out_val));

			/* Indicate legality of quiver */
			if (use_quiver)
				my_strcat(out_val, " . for Quiver,", sizeof(out_val));

		}

		/* Finish the prompt */
		my_strcat(out_val, " ESC", sizeof(out_val));

		/* if we have a prompt header, show the part that we just built */
		if (pmt) {
			/* Build the prompt */
			strnfmt(tmp_val, sizeof(tmp_val), "(%s) %s", out_val, pmt);

			/* Show the prompt */
			prt(tmp_val, 0, 0);
		}

		/* Get a key */
		press = inkey_m();

		/* Parse it */
		if (press.type == EVT_MOUSE) {
			if (press.mouse.button == 2) {
				done = TRUE;
			} else if (press.mouse.button == 1) {
				obj = NULL;
				if (player->upkeep->command_wrk == USE_INVEN) {
					if (press.mouse.y == 0) {
						if (allow_equip) {
							player->upkeep->command_wrk = USE_EQUIP;
						} else if (allow_quiver) {
							player->upkeep->command_wrk = USE_QUIVER;
						} else if (allow_floor) {
							player->upkeep->command_wrk = USE_FLOOR;
						}
					} else if ((press.mouse.y <= i2 - i1 + 1) ) {
						/* Get the item index, allowing for skipped indices */
						for (j = i1; j <= i2; j++) {
							struct object *test = player->upkeep->inven[j];
							if (object_test(tester, test)) {
								if (press.mouse.y == 1) {
									obj = test;
									break;
								}
								press.mouse.y--;
							}
						}
					}
				} else if (player->upkeep->command_wrk == USE_EQUIP) {
					if (press.mouse.y == 0) {
						if (allow_quiver) {
							player->upkeep->command_wrk = USE_QUIVER;
						} else if (allow_floor) {
							player->upkeep->command_wrk = USE_FLOOR;
						} else if (allow_inven) {
							player->upkeep->command_wrk = USE_INVEN;
						}
					} else if (press.mouse.y <= e2 - e1 + 1) {
						if (olist_mode & OLIST_SEMPTY) {
							/* If we are showing empties, just set the object
							 * (empty objects will just keep the loop going) */
							obj = label_to_equip('a' + e1 + press.mouse.y - 1);
						} else {
							/* get the item index, allow for skipped indices */
							for (j = e1; j <= e2; j++) {
								struct object *test = slot_object(player, j);
								if (object_test(tester, test)) {
									if (press.mouse.y == 1) {
										obj = test;
										break;
									}
									press.mouse.y--;
								}
							}
						}
					}
				} else if (player->upkeep->command_wrk == USE_QUIVER) {
					if (press.mouse.y == 0) {
						if (allow_floor) {
							player->upkeep->command_wrk = USE_FLOOR;
						} else if (allow_inven) {
							player->upkeep->command_wrk = USE_INVEN;
						} else if (allow_equip) {
							player->upkeep->command_wrk = USE_EQUIP;
						}
					} else if (press.mouse.y <= q2 - q1 + 1) {
						/* get the item index, allow for skipped indices */
						for (j = q1; j <= q2; j++) {
							struct object *test = player->upkeep->quiver[j];
							if (object_test(tester, test)) {
								if (press.mouse.y == 1) {
									obj = test;
									break;
								}
								press.mouse.y--;
							}
						}
					}
				} else if (player->upkeep->command_wrk == USE_FLOOR) {
					if (press.mouse.y == 0) {
						if (allow_inven) {
							player->upkeep->command_wrk = USE_INVEN;
						} else if (allow_equip) {
							player->upkeep->command_wrk = USE_EQUIP;
						} else if (allow_quiver) {
							player->upkeep->command_wrk = USE_QUIVER;
						}
					} else if ((press.mouse.y <= floor_num)
							   && (press.mouse.y >= 1)) {
						/* Get the item index, allowing for skipped indices */
						for (j = f1; j <= f2; j++) {
							struct object *test = floor_list[j];
							if (object_test(tester, test)) {
								if (press.mouse.y == 1) {
									obj = test;
									break;
								}
								press.mouse.y--;
							}
						}
					}
				}
				if (obj) {
					/* Validate the item */
					if (!object_test(tester, obj)) {
						bell("Illegal object choice (normal)!");
					}

					/* Allow player to "refuse" certain actions */
					if (!get_item_allow(obj, cmdkey, cmd, is_harmless))
						done = TRUE;

					/* Accept that choice */
					(*choice) = obj;
					item = TRUE;
					done = TRUE;
				} else if (press.mouse.y == 0) {
					/* Hack -- Fix screen */
					if (show_list) {
						/* Load screen */
						screen_load();

						/* Save screen */
						screen_save();
					}
				}
			}
		} else
		switch (press.key.code)
		{
			case ESCAPE:
			case ' ':
			{
				done = TRUE;
				break;
			}

			case '/':
			{
				/* Toggle to inventory */
				if (use_inven && (player->upkeep->command_wrk != USE_INVEN))
					player->upkeep->command_wrk = USE_INVEN;

				/* Toggle to equipment */
				else if (use_equip &&
						 (player->upkeep->command_wrk != USE_EQUIP))
					player->upkeep->command_wrk = USE_EQUIP;

				/* No toggle allowed */
				else {
					bell("Cannot switch item selector!");
					break;
				}


				/* Hack -- Fix screen */
				if (show_list) {
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				/* Need to redraw */
				break;
			}

			case '.':
			{
				/* Paranoia */
				if (!allow_quiver) {
					bell("Cannot select quiver!");
					break;
				}

				/* Hack -- Fix screen */
				if (show_list) {
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				player->upkeep->command_wrk = (USE_QUIVER);

				break;
			}

			case '-':
			{
				/* Paranoia */
				if (!allow_floor) {
					bell("Cannot select floor!");
					break;
				}

				/* There is only one item */
				if (floor_num == 1) {
					/* Auto-select */
					if (player->upkeep->command_wrk == (USE_FLOOR)) {
						/* Special index */
						obj = floor_list[0];

						/* Allow player to "refuse" certain actions */
						if (!get_item_allow(obj, cmdkey, cmd, is_harmless)) {
							done = TRUE;
							break;
						}

						/* Accept that choice */
						(*choice) = obj;
						item = TRUE;
						done = TRUE;

						break;
					}
				}

				/* Hack -- Fix screen */
				if (show_list) {
					/* Load screen */
					screen_load();

					/* Save screen */
					screen_save();
				}

				player->upkeep->command_wrk = (USE_FLOOR);

				break;
			}

			case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
			{
				/* Look up the tag */
				if (!get_tag(&obj, press.key.code, cmd, quiver_tags)) {
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Validate the item */
				if (!object_test(tester, obj)) {
					bell("Illegal object choice (tag)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(obj, cmdkey, cmd, is_harmless)) {
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*choice) = obj;
				item = TRUE;
				done = TRUE;
				break;
			}

			case KC_ENTER:
			{
				/* Choose "default" inventory item */
				if (player->upkeep->command_wrk == USE_INVEN) {
					if (i1 != i2) {
						bell("Illegal object choice (default)!");
						break;
					}

					obj = player->upkeep->inven[i1];
				}

				/* Choose the "default" slot (0) of the quiver */
				else if (quiver_tags)
					obj = player->upkeep->quiver[q1];

				/* Choose "default" equipment item */
				else if (player->upkeep->command_wrk == USE_EQUIP) {
					if (e1 != e2) {
						bell("Illegal object choice (default)!");
						break;
					}

					obj = slot_object(player, e1);
				}

				/* Choose "default" quiver item */
				else if (player->upkeep->command_wrk == USE_QUIVER) {
					if (q1 != q2) {
						bell("Illegal object choice (default)!");
						break;
					}

					obj = player->upkeep->quiver[q1];
				}

				/* Choose "default" floor item */
				else {
					if (f1 != f2) {
						bell("Illegal object choice (default)!");
						break;
					}

					obj = floor_list[f1];
				}

				/* Validate the item */
				if (!object_test(tester, obj)) {
					bell("Illegal object choice (default)!");
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(obj, cmdkey, cmd, is_harmless)) {
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*choice) = obj;
				item = TRUE;
				done = TRUE;
				break;
			}

			default:
			{
				bool verify;

				/* Note verify */
				verify = (isupper((unsigned char)press.key.code) ? TRUE : FALSE);

				/* Lowercase */
				press.key.code = tolower((unsigned char)press.key.code);

				/* Convert letter to inventory object */
				if (player->upkeep->command_wrk == USE_INVEN) {
					obj = label_to_inven(press.key.code);

					if (!obj) {
						bell("Illegal object choice (inven)!");
						break;
					}
				}

				/* Convert letter to equipment object */
				else if (player->upkeep->command_wrk == USE_EQUIP) {
					obj = label_to_equip(press.key.code);

					if (!obj) {
						bell("Illegal object choice (equip)!");
						break;
					}
				}

				/* Convert letter to quiver object */
				else if (player->upkeep->command_wrk == USE_QUIVER) {
					obj = label_to_quiver(press.key.code);

					if (!obj) {
						bell("Illegal object choice (quiver)!");
						break;
					}
				}

				/* Convert letter to floor object */
				else {
					int k = (islower((unsigned char)press.key.code) ?
							 A2I((unsigned char)press.key.code) : -1);

					if (k < 0 || k >= floor_num) {
						bell("Illegal object choice (floor)!");
						break;
					}

					obj = floor_list[k];
				}

				/* Validate the item */
				if (!object_test(tester, obj)) {
					bell("Illegal object choice (normal)!");
					break;
				}

				/* Verify the item */
				if (verify && !verify_object("Try", obj)) {
					done = TRUE;
					break;
				}

				/* Allow player to "refuse" certain actions */
				if (!get_item_allow(obj, cmdkey, cmd, is_harmless)) {
					done = TRUE;
					break;
				}

				/* Accept that choice */
				(*choice) = obj;
				item = TRUE;
				done = TRUE;
				break;
			}
		}
	}


	/* Fix the screen if necessary */
	if (show_list)
		screen_load();

	/* Toggle again if needed */
	if (toggle) toggle_inven_equip();

	/* Update */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	redraw_stuff(player->upkeep);


	/* Clear the prompt line */
	prt("", 0, 0);

	/* Warning if needed */
	if (oops && str) msg("%s", str);

	mem_free(floor_list);

	/* Result */
	return (item);
}



/**
 * This draws the Object Recall subwindow when displaying a particular object
 * (e.g. a helmet in the backpack, or a scroll on the ground)
 */
void display_object_recall(struct object *o_ptr)
{
	char header[120];

	textblock *tb = object_info(o_ptr, OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

	clear_from(0);
	textui_textblock_place(tb, SCREEN_REGION, header);
	textblock_free(tb);
}


/**
 * This draws the Object Recall subwindow when displaying a recalled item kind
 * (e.g. a generic ring of acid or a generic blade of chaos)
 */
void display_object_kind_recall(struct object_kind *kind)
{
	struct object object = { 0 };
	object_prep(&object, kind, 0, EXTREMIFY);
	if (kind->aware)
		object_notice_everything(&object);

	display_object_recall(&object);
}

/**
 * Display object recall modally and wait for a keypress.
 *
 * This is set up for use in look mode (see target_set_interactive_aux()).
 *
 * \param o_ptr is the object to be described.
 */
void display_object_recall_interactive(struct object *o_ptr)
{
	char header[120];
	textblock *tb;

	event_signal(EVENT_MESSAGE_FLUSH);

	tb = object_info(o_ptr, OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);
	textui_textblock_show(tb, SCREEN_REGION, header);
	textblock_free(tb);
}

