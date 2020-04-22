/**
 * \file ui-object.c
 * \brief Object lists and selection, and other object-related UI functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
 * Copyright (c) 2015 Nick McConnell
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
#include "cave.h"
#include "cmd-core.h"
#include "cmds.h"
#include "effects.h"
#include "game-input.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-info.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "store.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-menu.h"
#include "ui-object.h"
#include "ui-options.h"
#include "ui-output.h"
#include "ui-prefs.h"

/**
 * ------------------------------------------------------------------------
 * Variables for object display and selection
 * ------------------------------------------------------------------------ */
#define MAX_ITEMS 50

/**
 * Info about a particular object
 */
struct object_menu_data {
	char label[80];
	char equip_label[80];
	struct object *object;
	char o_name[80];
	char key;
};

static struct object_menu_data items[MAX_ITEMS];
static int num_obj;
static int num_head;
static size_t max_len;
static int ex_width;
static int ex_offset;

/**
 * ------------------------------------------------------------------------
 * Display of individual objects in lists or for selection
 * ------------------------------------------------------------------------ */
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
byte object_attr(const struct object *obj)
{
	return object_kind_attr(obj->kind);
}

/**
 * Return the "char" for a given item.
 * Use "flavor" if available.
 * Default to user definitions.
 */
wchar_t object_char(const struct object *obj)
{
	return object_kind_char(obj->kind);
}

/**
 * Display an object.  Each object may be prefixed with a label.
 * Used by show_inven(), show_equip(), show_quiver() and show_floor().
 * Mode flags are documented in object.h
 */
static void show_obj(int obj_num, int row, int col, bool cursor,
					 olist_detail_t mode)
{
	int attr;
	int label_attr = cursor ? COLOUR_L_BLUE : COLOUR_WHITE;
	int ex_offset_ctr;
	char buf[80];
	struct object *obj = items[obj_num].object;
	bool show_label = mode & (OLIST_WINDOW | OLIST_DEATH) ? true : false;
	int label_size = show_label ? strlen(items[obj_num].label) : 0;
	int equip_label_size = strlen(items[obj_num].equip_label);

	/* Clear the line */
	prt("", row + obj_num, MAX(col - 1, 0));

	/* If we have no label then we won't display anything */
	if (!strlen(items[obj_num].label)) return;

	/* Print the label */
	if (show_label)
		c_put_str(label_attr, items[obj_num].label, row + obj_num, col);

	/* Print the equipment label */
	c_put_str(label_attr, items[obj_num].equip_label, row + obj_num,
			  col + label_size);

	/* Limit object name */
	if (label_size + equip_label_size + strlen(items[obj_num].o_name) >
		(size_t)ex_offset) {
		int truncate = ex_offset - label_size - equip_label_size;

		if (truncate < 0) truncate = 0;
		if ((size_t)truncate > sizeof(items[obj_num].o_name) - 1)
			truncate = sizeof(items[obj_num].o_name) - 1;

		items[obj_num].o_name[truncate] = '\0';
	}

	/* Item kind determines the color of the output */
	if (obj) {
		attr = obj->kind->base->attr;

		/* Unreadable books are a special case */
		if (tval_is_book_k(obj->kind) &&
			(player_object_to_book(player, obj) == NULL)) {
			attr = COLOUR_SLATE;
		}
	} else {
		attr = COLOUR_SLATE;
	}

	/* Object name */
	c_put_str(attr, items[obj_num].o_name, row + obj_num,
			  col + label_size + equip_label_size);

	/* If we don't have an object, we can skip the rest of the output */
	if (!obj) return;

	/* Extra fields */
	ex_offset_ctr = ex_offset;

	/* Price */
	if (mode & OLIST_PRICE) {
		struct store *store = store_at(cave, player->grid);
		if (store) {
			int price = price_item(store, obj, true, obj->number);

			strnfmt(buf, sizeof(buf), "%6d au", price);
			put_str(buf, row + obj_num, col + ex_offset_ctr);
			ex_offset_ctr += 9;
		}
	}

	/* Failure chance for magic devices and activations */
	if (mode & OLIST_FAIL && obj_can_fail(obj)) {
		int fail = (9 + get_use_device_chance(obj)) / 10;
		if (object_effect_is_known(obj))
			strnfmt(buf, sizeof(buf), "%4d%% fail", fail);
		else
			my_strcpy(buf, "    ? fail", sizeof(buf));
		put_str(buf, row + obj_num, col + ex_offset_ctr);
		ex_offset_ctr += 10;
	}

	/* Failure chances for recharging an item; see effect_handler_RECHARGE */
	if (mode & OLIST_RECHARGE) {
		int fail = 1000 / recharge_failure_chance(obj, player->upkeep->recharge_pow);
		if (object_effect_is_known(obj))
			strnfmt(buf, sizeof(buf), "%2d.%1d%% fail", fail / 10, fail % 10);
		else
			my_strcpy(buf, "    ? fail", sizeof(buf));
		put_str(buf, row + obj_num, col + ex_offset_ctr);
		ex_offset_ctr += 10;
	}

	/* Weight */
	if (mode & OLIST_WEIGHT) {
		int weight = obj->weight * obj->number;
		strnfmt(buf, sizeof(buf), "%4d.%1d lb", weight / 10, weight % 10);
		put_str(buf, row + obj_num, col + ex_offset_ctr);
	}
}

/**
 * ------------------------------------------------------------------------
 * Display of lists of objects
 * ------------------------------------------------------------------------ */
/**
 * Clear the object list.
 */
static void wipe_obj_list(void)
{
	int i;

	/* Zero the constants */
	num_obj = 0;
	num_head = 0;
	max_len = 0;
	ex_width = 0;
	ex_offset = 0;

	/* Clear the existing contents */
	for (i = 0; i < MAX_ITEMS; i++) {
		my_strcpy(items[i].label, "", sizeof(items[i].label));
		my_strcpy(items[i].equip_label, "", sizeof(items[i].equip_label));
		items[i].object = NULL;
		my_strcpy(items[i].o_name, "", sizeof(items[i].o_name));
		items[i].key = '\0';
	}
}

/**
 * Build the object list.
 */
static void build_obj_list(int last, struct object **list, item_tester tester,
						   olist_detail_t mode)
{
	int i;
	bool gold_ok = (mode & OLIST_GOLD) ? true : false;
	bool in_term = (mode & OLIST_WINDOW) ? true : false;
	bool dead = (mode & OLIST_DEATH) ? true : false;
	bool show_empty = (mode & OLIST_SEMPTY) ? true : false;
	bool equip = list ? false : true;
	bool quiver = list == player->upkeep->quiver ? true : false;

	/* Build the object list */
	for (i = 0; i <= last; i++) {
		char buf[80];
		struct object *obj = equip ? slot_object(player, i) : list[i];

		/* Acceptable items get a label */
		if (object_test(tester, obj) ||	(obj && tval_is_money(obj) && gold_ok))
			strnfmt(items[num_obj].label, sizeof(items[num_obj].label), "%c) ",
					quiver ? I2D(i) : I2A(i));

		/* Unacceptable items are still sometimes shown */
		else if ((!obj && show_empty) || in_term)
			my_strcpy(items[num_obj].label, "   ",
					  sizeof(items[num_obj].label));

		/* Unacceptable items are skipped in the main window */
		else continue;

		/* Show full slot labels for equipment (or quiver in subwindow) */
		if (equip) {
			strnfmt(buf, sizeof(buf), "%-14s: ", equip_mention(player, i));
			my_strcpy(items[num_obj].equip_label, buf,
					  sizeof(items[num_obj].equip_label));
		} else if ((in_term || dead) && quiver) {
			strnfmt(buf, sizeof(buf), "Slot %-9d: ", i);
			my_strcpy(items[num_obj].equip_label, buf,
					  sizeof(items[num_obj].equip_label));
		} else {
			strnfmt(items[num_obj].equip_label,
					sizeof(items[num_obj].equip_label), "");
		}

		/* Save the object */
		items[num_obj].object = obj;
		items[num_obj].key = (items[num_obj].label)[0];
		num_obj++;
	}
}

/**
 * Set object names and get their maximum length.
 * Only makes sense after building the object list.
 */
static void set_obj_names(bool terse)
{
	int i;
	struct object *obj;

	/* Calculate name offset and max name length */
	for (i = 0; i < num_obj; i++) {
		obj = items[i].object;

		/* Null objects are used to skip lines, or display only a label */		
		if (!obj) {
			if ((i < num_head) || !strcmp(items[i].label, "In quiver"))
				strnfmt(items[i].o_name, sizeof(items[i].o_name), "");
			else
				strnfmt(items[i].o_name, sizeof(items[i].o_name), "(nothing)");
		} else {
			if (terse)
				object_desc(items[i].o_name, sizeof(items[i].o_name), obj,
							ODESC_PREFIX | ODESC_FULL | ODESC_TERSE);
			else
				object_desc(items[i].o_name, sizeof(items[i].o_name), obj,
							ODESC_PREFIX | ODESC_FULL);
		}

		/* Max length of label + object name */
		max_len = MAX(max_len,
					  strlen(items[i].label) + strlen(items[i].equip_label) +
					  strlen(items[i].o_name));
	}
}

/**
 * Display a list of objects.  Each object may be prefixed with a label.
 * Used by show_inven(), show_equip(), and show_floor().  Mode flags are
 * documented in object.h
 */
static void show_obj_list(olist_detail_t mode)
{
	int i, row = 0, col = 0;
	char tmp_val[80];

	bool in_term = (mode & OLIST_WINDOW) ? true : false;
	bool terse = false;

	/* Initialize */
	max_len = 0;
	ex_width = 0;
	ex_offset = 0;

	if (in_term) max_len = 40;
	if (in_term && Term->wid < 40) mode &= ~(OLIST_WEIGHT);

	if (Term->wid < 50) terse = true;

	/* Set the names and get the max length */
	set_obj_names(terse);

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
	for (i = 0; i < num_obj; i++)
		show_obj(i, row, col, false, mode);

	/* For the inventory: print the quiver count */
	if (mode & OLIST_QUIVER) {
		int count, j;
		int quiver_slots = (player->upkeep->quiver_cnt + z_info->quiver_slot_size - 1) / z_info->quiver_slot_size;

		/* Quiver may take multiple lines */
		for (j = 0; j < quiver_slots; j++, i++) {
			const char *fmt = "in Quiver: %d missile%s";
			char letter = I2A(in_term ? i - 1 : i);

			/* Number of missiles in this "slot" */
			if (j == quiver_slots - 1)
				count = player->upkeep->quiver_cnt - (z_info->quiver_slot_size * (quiver_slots - 1));
			else
				count = z_info->quiver_slot_size;

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
	int diff = weight_remaining(player);

	bool in_term = (mode & OLIST_WINDOW) ? true : false;

	/* Intialize */
	wipe_obj_list();

	/* Include burden for term windows */
	if (in_term) {
		strnfmt(items[num_obj].label, sizeof(items[num_obj].label),
		        "Burden %d.%d lb (%d.%d lb %s) ",
		        player->upkeep->total_weight / 10,
				player->upkeep->total_weight % 10,
		        abs(diff) / 10, abs(diff) % 10,
		        (diff < 0 ? "overweight" : "remaining"));

		items[num_obj].object = NULL;
		num_obj++;
	}

	/* Find the last occupied inventory slot */
	for (i = 0; i < z_info->pack_size; i++)
		if (player->upkeep->inven[i] != NULL) last_slot = i;

	/* Build the object list */
	build_obj_list(last_slot, player->upkeep->inven, tester, mode);

	/* Term window starts with a burden header */
	num_head = in_term ? 1 : 0;

	/* Display the object list */
	show_obj_list(mode);
}


/**
 * Display the quiver.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_quiver(int mode, item_tester tester)
{
	int i, last_slot = -1;

	/* Intialize */
	wipe_obj_list();

	/* Find the last occupied quiver slot */
	for (i = 0; i < z_info->quiver_size; i++)
		if (player->upkeep->quiver[i] != NULL) last_slot = i;

	/* Build the object list */
	build_obj_list(last_slot, player->upkeep->quiver, tester, mode);

	/* Display the object list */
	num_head = 0;
	show_obj_list(mode);
}


/**
 * Display the equipment.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_equip(int mode, item_tester tester)
{
	int i;
	bool in_term = (mode & OLIST_WINDOW) ? true : false;

	/* Intialize */
	wipe_obj_list();

	/* Build the object list */
	build_obj_list(player->body.count - 1, NULL, tester, mode);

	/* Show the quiver in subwindows */
	if (in_term) {
		int last_slot = -1;

		strnfmt(items[num_obj].label, sizeof(items[num_obj].label),
				"In quiver");
		items[num_obj].object = NULL;
		num_obj++;

		/* Find the last occupied quiver slot */
		for (i = 0; i < z_info->quiver_size; i++)
			if (player->upkeep->quiver[i] != NULL) last_slot = i;

		/* Extend the object list */
		build_obj_list(last_slot, player->upkeep->quiver, tester, mode);
	}

	/* Display the object list */
	num_head = 0;
	show_obj_list(mode);
}


/**
 * Display the floor.  Builds a list of objects and passes them
 * off to show_obj_list() for display.  Mode flags documented in
 * object.h
 */
void show_floor(struct object **floor_list, int floor_num, int mode,
				item_tester tester)
{
	/* Intialize */
	wipe_obj_list();

	if (floor_num > z_info->floor_size)
		floor_num = z_info->floor_size;

	/* Build the object list */
	build_obj_list(floor_num - 1, floor_list, tester, mode);

	/* Display the object list */
	num_head = 0;
	show_obj_list(mode);
}


/**
 * ------------------------------------------------------------------------
 * Variables for object selection
 * ------------------------------------------------------------------------ */

static item_tester tester_m;
static region area = { 20, 1, -1, -2 };
static struct object *selection;
static const char *prompt;
static char header[80];
static int i1, i2;
static int e1, e2;
static int q1, q2;
static int f1, f2;
static int throwing_num;
static struct object **floor_list;
static struct object **throwing_list;
static olist_detail_t olist_mode = 0;
static int item_mode;
static cmd_code item_cmd;
static bool newmenu = false;
static bool allow_all = false;

/**
 * ------------------------------------------------------------------------
 * Object selection utilities
 * ------------------------------------------------------------------------ */

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
		char prompt_buf[1024];

		const char *verb = cmd_verb(cmd);
		if (!verb)
			verb = "do that with";

		strnfmt(prompt_buf, sizeof(prompt_buf), "Really %s", verb);

		/* Prompt for confirmation n times */
		while (n--) {
			if (!verify_object(prompt_buf, (struct object *) obj))
				return (false);
		}
	}

	/* Allow it */
	return (true);
}



/**
 * Find the first object in the object list with the given "tag".  The object
 * list needs to be built before this function is called.
 *
 * A "tag" is a char "n" appearing as "@n" anywhere in the
 * inscription of an object.
 *
 * Also, the tag "@xn" will work as well, where "n" is a tag-char,
 * and "x" is the action that tag will work for.
 */
static bool get_tag(struct object **tagged_obj, char tag, cmd_code cmd,
				   bool quiver_tags)
{
	int i;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	/* (f)ire is handled differently from all others, due to the quiver */
	if (quiver_tags) {
		i = tag - '0';
		if (player->upkeep->quiver[i]) {
			*tagged_obj = player->upkeep->quiver[i];
			return true;
		}
	}

	/* Check every object in the object list */
	for (i = 0; i < num_obj; i++) {
		const char *s;
		struct object *obj = items[i].object;

		/* Skip non-objects */
		if (!obj) continue;

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
				return true;
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
				return true;
			}

			/* Find another '@' */
			s = strchr(s + 1, '@');
		}
	}

	/* No such tag */
	return false;
}


/**
 * ------------------------------------------------------------------------
 * Object selection menu
 * ------------------------------------------------------------------------ */

/**
 * Make the correct header for the selection menu
 */
static void menu_header(void)
{
	char tmp_val[75];
	char out_val[75];

	bool use_inven = ((item_mode & USE_INVEN) ? true : false);
	bool use_equip = ((item_mode & USE_EQUIP) ? true : false);
	bool use_quiver = ((item_mode & USE_QUIVER) ? true : false);
	bool allow_floor = ((f1 <= f2) || allow_all);

	/* Viewing inventory */
	if (player->upkeep->command_wrk == USE_INVEN) {
		/* Begin the header */
		strnfmt(out_val, sizeof(out_val), "Inven:");

		/* List choices */
		if (i1 <= i2) {
			/* Build the header */
			strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(i1), I2A(i2));

			/* Append */
			my_strcat(out_val, tmp_val, sizeof(out_val));
		}

		/* Indicate legality of equipment */
		if (use_equip)
			my_strcat(out_val, " / for Equip,", sizeof(out_val));

		/* Indicate legality of quiver */
		if (use_quiver)
			my_strcat(out_val, " | for Quiver,", sizeof(out_val));

		/* Indicate legality of the "floor" */
		if (allow_floor)
			my_strcat(out_val, " - for floor,", sizeof(out_val));
	}

	/* Viewing equipment */
	else if (player->upkeep->command_wrk == USE_EQUIP) {
		/* Begin the header */
		strnfmt(out_val, sizeof(out_val), "Equip:");

		/* List choices */
		if (e1 <= e2) {
			/* Build the header */
			strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(e1), I2A(e2));

			/* Append */
			my_strcat(out_val, tmp_val, sizeof(out_val));
		}

		/* Indicate legality of inventory */
		if (use_inven)
			my_strcat(out_val, " / for Inven,", sizeof(out_val));

		/* Indicate legality of quiver */
		if (use_quiver)
			my_strcat(out_val, " | for Quiver,", sizeof(out_val));

		/* Indicate legality of the "floor" */
		if (allow_floor)
			my_strcat(out_val, " - for floor,", sizeof(out_val));
	}

	/* Viewing quiver */
	else if (player->upkeep->command_wrk == USE_QUIVER) {
		/* Begin the header */
		strnfmt(out_val, sizeof(out_val), "Quiver:");

		/* List choices */
		if (q1 <= q2) {
			/* Build the header */
			strnfmt(tmp_val, sizeof(tmp_val), " %d-%d,", q1, q2);

			/* Append */
			my_strcat(out_val, tmp_val, sizeof(out_val));
		}

		/* Indicate legality of inventory or equipment */
		if (use_inven)
			my_strcat(out_val, " / for Inven,", sizeof(out_val));
		else if (use_equip)
			my_strcat(out_val, " / for Equip,", sizeof(out_val));

		/* Indicate legality of the "floor" */
		if (allow_floor)
			my_strcat(out_val, " - for floor,", sizeof(out_val));
	}

	/* Viewing throwing */
	else if (player->upkeep->command_wrk == SHOW_THROWING) {
		/* Begin the header */
		strnfmt(out_val, sizeof(out_val), "Throwing items:");

		/* List choices */
		if (throwing_num) {
			/* Build the header */
			strnfmt(tmp_val, sizeof(tmp_val),  " a-%c,", I2A(throwing_num - 1));

			/* Append */
			my_strcat(out_val, tmp_val, sizeof(out_val));
		}

		/* Indicate legality of inventory */
		if (use_inven)
			my_strcat(out_val, " / for Inven,", sizeof(out_val));

		/* Indicate legality of quiver */
		if (use_quiver)
			my_strcat(out_val, " | for Quiver,", sizeof(out_val));

		/* Indicate legality of the "floor" */
		if (allow_floor)
			my_strcat(out_val, " - for floor,", sizeof(out_val));
	}

	/* Viewing floor */
	else {
		/* Begin the header */
		strnfmt(out_val, sizeof(out_val), "Floor:");

		/* List choices */
		if (f1 <= f2) {
			/* Build the header */
			strnfmt(tmp_val, sizeof(tmp_val), " %c-%c,", I2A(f1), I2A(f2));

			/* Append */
			my_strcat(out_val, tmp_val, sizeof(out_val));
		}

		/* Indicate legality of inventory or equipment */
		if (use_inven)
			my_strcat(out_val, " / for Inven,", sizeof(out_val));
		else if (use_equip)
			my_strcat(out_val, " / for Equip,", sizeof(out_val));

		/* Indicate legality of quiver */
		if (use_quiver)
			my_strcat(out_val, " | for Quiver,", sizeof(out_val));
	}

	/* Finish the header */
	my_strcat(out_val, " ESC", sizeof(out_val));

	/* Build the header */
	strnfmt(header, sizeof(header), "(%s)", out_val);
}


/**
 * Get an item tag
 */
char get_item_tag(struct menu *menu, int oid)
{
	struct object_menu_data *choice = menu_priv(menu);

	return choice[oid].key;
}

/**
 * Determine if an item is a valid choice
 */
int get_item_validity(struct menu *menu, int oid)
{
	struct object_menu_data *choice = menu_priv(menu);

	return (choice[oid].object != NULL) ? 1 : 0;
}

/**
 * Display an entry on the item menu
 */
void get_item_display(struct menu *menu, int oid, bool cursor, int row,
					  int col, int width)
{
	/* Print it */
	show_obj(oid, row - oid, col, cursor, olist_mode);
}

/**
 * Deal with events on the get_item menu
 */
bool get_item_action(struct menu *menu, const ui_event *event, int oid)
{
	struct object_menu_data *choice = menu_priv(menu);
	char key = event->key.code;
	bool is_harmless = item_mode & IS_HARMLESS ? true : false;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	if (event->type == EVT_SELECT) {
		if (get_item_allow(choice[oid].object, cmd_lookup_key(item_cmd, mode),
						   item_cmd, is_harmless))
			selection = choice[oid].object;
	}

	if (event->type == EVT_KBRD) {
		if (key == '/') {
			/* Toggle if allowed */
			if (((item_mode & USE_INVEN) || allow_all)
				&& (player->upkeep->command_wrk != USE_INVEN)) {
				player->upkeep->command_wrk = USE_INVEN;
				newmenu = true;
			} else if (((item_mode & USE_EQUIP) || allow_all) &&
					   (player->upkeep->command_wrk != USE_EQUIP)) {
				player->upkeep->command_wrk = USE_EQUIP;
				newmenu = true;
			} else {
				bell("Cannot switch item selector!");
			}
		}

		else if (key == '|') {
			/* No toggle allowed */
			if ((q1 > q2) && !allow_all){
				bell("Cannot select quiver!");
			} else {
				/* Toggle to quiver */
				player->upkeep->command_wrk = (USE_QUIVER);
				newmenu = true;
			}
		}

		else if (key == '-') {
			/* No toggle allowed */
			if ((f1 > f2) && !allow_all) {
				bell("Cannot select floor!");
			} else {
				/* Toggle to floor */
				player->upkeep->command_wrk = (USE_FLOOR);
				newmenu = true;
			}
		}
	}

	return false;
}

/**
 * Show quiver missiles in full inventory
 */
static void item_menu_browser(int oid, void *data, const region *local_area)
{
	char tmp_val[80];
	int count, j, i = num_obj;
	int quiver_slots = (player->upkeep->quiver_cnt + z_info->quiver_slot_size - 1)
		/ z_info->quiver_slot_size;

	/* Set up to output below the menu */
	text_out_hook = text_out_to_screen;
	text_out_wrap = 0;
	text_out_indent = local_area->col - 1;
	text_out_pad = 1;
	prt("", local_area->row + local_area->page_rows, MAX(0, local_area->col - 1));
	Term_gotoxy(local_area->col, local_area->row + local_area->page_rows);

	/* If we're printing pack slots the quiver takes up */
	if (olist_mode & OLIST_QUIVER && player->upkeep->command_wrk == USE_INVEN) {
		/* Quiver may take multiple lines */
		for (j = 0; j < quiver_slots; j++, i++) {
			const char *fmt = "in Quiver: %d missile%s\n";
			char letter = I2A(i);

			/* Number of missiles in this "slot" */
			if (j == quiver_slots - 1)
				count = player->upkeep->quiver_cnt - (z_info->quiver_slot_size *
													  (quiver_slots - 1));
			else
				count = z_info->quiver_slot_size;

			/* Print the (disabled) label */
			strnfmt(tmp_val, sizeof(tmp_val), "%c) ", letter);
			text_out_c(COLOUR_SLATE, tmp_val, local_area->row + i, local_area->col);

			/* Print the count */
			strnfmt(tmp_val, sizeof(tmp_val), fmt, count,
					count == 1 ? "" : "s");
			text_out_c(COLOUR_L_UMBER, tmp_val, local_area->row + i, local_area->col + 3);
		}
	}

	/* Always print a blank line */
	prt("", local_area->row + i, MAX(0, local_area->col - 1));

	/* Blank out whole tiles */
	while ((tile_height > 1) && ((local_area->row + i) % tile_height != 0)) {
		i++;
		prt("", local_area->row + i, MAX(0, local_area->col - 1));
	}

	text_out_pad = 0;
	text_out_indent = 0;
}

/**
 * Display list items to choose from
 */
struct object *item_menu(cmd_code cmd, int prompt_size, int mode)
{
	menu_iter menu_f = { get_item_tag, get_item_validity, get_item_display,
						 get_item_action, 0 };
	struct menu *m = menu_new(MN_SKIN_OBJECT, &menu_f);
	ui_event evt = { 0 };
	int ex_offset_ctr = 0;
	int row, inscrip;
	struct object *obj = NULL;

	/* Set up the menu */
	menu_setpriv(m, num_obj, items);
	if (player->upkeep->command_wrk == USE_QUIVER)
		m->selections = "0123456789";
	else
		m->selections = lower_case;
	m->switch_keys = "/|-";
	m->flags = (MN_PVT_TAGS | MN_INSCRIP_TAGS);
	m->browse_hook = item_menu_browser;

	/* Get inscriptions */
	m->inscriptions = mem_zalloc(10 * sizeof(char));
	for (inscrip = 0; inscrip < 10; inscrip++) {
		/* Look up the tag */
		if (get_tag(&obj, (char)inscrip + '0', item_cmd,
					item_mode & QUIVER_TAGS)) {
			int i;
			for (i = 0; i < num_obj; i++)
				if (items[i].object == obj)
						break;

			if (i < num_obj)
				m->inscriptions[inscrip] = get_item_tag(m, i);
		}
	}

	/* Set up the item list variables */
	selection = NULL;
	set_obj_names(false);

	if (mode & OLIST_QUIVER && player->upkeep->quiver[0] != NULL)
		max_len = MAX(max_len, 24);

	if (olist_mode & OLIST_WEIGHT) {
		ex_width += 9;
		ex_offset_ctr += 9;
	}
	if (olist_mode & OLIST_PRICE) {
		ex_width += 9;
		ex_offset_ctr += 9;
	}
	if (olist_mode & OLIST_FAIL) {
		ex_width += 10;
		ex_offset_ctr += 10;
	}

	/* Set up the menu region */
	area.page_rows = m->count;
	area.row = 1;
	area.col = MIN(Term->wid - 1 - (int) max_len - ex_width, prompt_size - 2);
	if (area.col <= 3)
		area.col = 0;
	ex_offset = MIN(max_len, (size_t)(Term->wid - 1 - ex_width - area.col));
	while (strlen(header) < max_len + ex_width + ex_offset_ctr) {
		my_strcat(header, " ", sizeof(header));
		if (strlen(header) > sizeof(header) - 2) break;
	}
	area.width = MAX(max_len, strlen(header));

	for (row = area.row; row < area.row + area.page_rows; row++)
		prt("", row, MAX(0, area.col - 1));

	menu_layout(m, &area);

	/* Choose */
	evt = menu_select(m, 0, true);

	/* Clean up */
	mem_free(m->inscriptions);
	mem_free(m);

	/* Deal with menu switch */
	if (evt.type == EVT_SWITCH && !newmenu) {
		bool left = evt.key.code == ARROW_LEFT;

		if (player->upkeep->command_wrk == USE_EQUIP) {
			if (left) {
				if (f1 <= f2) player->upkeep->command_wrk = USE_FLOOR;
				else if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
				else if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
			} else {
				if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
				else if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
				else if (f1 <= f2) player->upkeep->command_wrk = USE_FLOOR;
			}
		} else if (player->upkeep->command_wrk == USE_INVEN) {
			if (left) {
				if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
				else if (f1 <= f2) player->upkeep->command_wrk = USE_FLOOR;
				else if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
			} else {
				if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
				else if (f1 <= f2) player->upkeep->command_wrk = USE_FLOOR;
				else if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
			}
		} else if (player->upkeep->command_wrk == USE_QUIVER) {
			if (left) {
				if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
				else if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
				else if (f1 <= f2) player->upkeep->command_wrk = USE_FLOOR;
			} else {
				if (f1 <= f2) player->upkeep->command_wrk = USE_FLOOR;
				else if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
				else if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
			}
		} else if (player->upkeep->command_wrk == USE_FLOOR) {
			if (left) {
				if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
				else if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
				else if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
			} else {
				if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
				else if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
				else if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
			}
		} else if (player->upkeep->command_wrk == SHOW_THROWING) {
			if (left) {
				if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
				else if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
				else if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
			} else {
				if (e1 <= e2) player->upkeep->command_wrk = USE_EQUIP;
				else if (i1 <= i2) player->upkeep->command_wrk = USE_INVEN;
				else if (q1 <= q2) player->upkeep->command_wrk = USE_QUIVER;
			}
		}

		newmenu = true;
	}

	/* Result */
	return selection;
}



/**
 * Let the user select an object, save its address
 *
 * Return true only if an acceptable item was chosen by the user.
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
 * If a legal item is selected , we save it in "choice" and return true.
 *
 * If no item is available, we do nothing to "choice", and we display a
 * warning message, using "str" if available, and return false.
 *
 * If no item is selected, we do nothing to "choice", and return false.
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
	bool use_inven = ((mode & USE_INVEN) ? true : false);
	bool use_equip = ((mode & USE_EQUIP) ? true : false);
	bool use_quiver = ((mode & USE_QUIVER) ? true : false);
	bool use_floor = ((mode & USE_FLOOR) ? true : false);
	bool quiver_tags = ((mode & QUIVER_TAGS) ? true : false);
	bool show_throwing = ((mode & SHOW_THROWING) ? true : false);

	bool allow_inven = false;
	bool allow_equip = false;
	bool allow_quiver = false;
	bool allow_floor = false;

	bool toggle = false;

	int floor_max = z_info->floor_size;
	int floor_num;

	int throwing_max = z_info->pack_size + z_info->quiver_size +
		z_info->floor_size;

	floor_list = mem_zalloc(floor_max * sizeof(*floor_list));
	throwing_list = mem_zalloc(throwing_max * sizeof(*floor_list));
	olist_mode = 0;
	item_mode = mode;
	item_cmd = cmd;
	tester_m = tester;
	prompt = pmt;
	allow_all = str ? false : true;

	/* Object list display modes */
	if (mode & SHOW_FAIL)
		olist_mode |= OLIST_FAIL;
	else
		olist_mode |= OLIST_WEIGHT;

	if (mode & SHOW_PRICES)
		olist_mode |= OLIST_PRICE;

	if (mode & SHOW_EMPTY)
		olist_mode |= OLIST_SEMPTY;

	if (mode & SHOW_QUIVER)
		olist_mode |= OLIST_QUIVER;

	if (mode & SHOW_RECHARGE)
		olist_mode |= OLIST_RECHARGE;

	/* Paranoia XXX XXX XXX */
	event_signal(EVENT_MESSAGE_FLUSH);

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
	if ((i1 <= i2) || allow_all)
		allow_inven = true;
	else if (item_mode & USE_INVEN)
		item_mode -= USE_INVEN;

	/* Full equipment */
	e1 = 0;
	e2 = player->body.count - 1;

	/* Forbid equipment */
	if (!use_equip) e2 = -1;

	/* Restrict equipment indexes unless starting with no command */
	if ((cmd != CMD_NULL) || (tester != NULL)) {
		while ((e1 <= e2) && (!object_test(tester, slot_object(player, e1))))
			e1++;
		while ((e1 <= e2) && (!object_test(tester, slot_object(player, e2))))
			e2--;
	}

	/* Accept equipment */
	if ((e1 <= e2) || allow_all)
		allow_equip = true;
	else if (item_mode & USE_EQUIP)
		item_mode -= USE_EQUIP;

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
	if ((q1 <= q2) || allow_all)
		allow_quiver = true;
	else if (item_mode & USE_QUIVER)
		item_mode -= USE_QUIVER;

	/* Scan all non-gold objects in the grid */
	floor_num = scan_floor(floor_list, floor_max,
						   OFLOOR_TEST | OFLOOR_SENSE | OFLOOR_VISIBLE, tester);

	/* Full floor */
	f1 = 0;
	f2 = floor_num - 1;

	/* Forbid floor */
	if (!use_floor) f2 = -1;

	/* Restrict floor indexes */
	while ((f1 <= f2) && (!object_test(tester, floor_list[f1]))) f1++;
	while ((f1 <= f2) && (!object_test(tester, floor_list[f2]))) f2--;

	/* Accept floor */
	if ((f1 <= f2) || allow_all)
		allow_floor = true;
	else if (item_mode & USE_FLOOR)
		item_mode -= USE_FLOOR;

	/* Scan all throwing objects in reach */
	throwing_num = scan_items(throwing_list, throwing_max,
							  USE_INVEN | USE_QUIVER | USE_FLOOR,
							  obj_is_throwing);

	/* Require at least one legal choice */
	if (allow_inven || allow_equip || allow_quiver || allow_floor) {
		/* Use throwing menu if at all possible */
		if (show_throwing && throwing_num) {
			player->upkeep->command_wrk = SHOW_THROWING;

			/* Start where requested if possible */
		} else if ((player->upkeep->command_wrk == USE_EQUIP) && allow_equip)
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

		while (true) {
			int j;
			int ni = 0;
			int ne = 0;

			/* If inven or equip is on the main screen, and only one of them
			 * is slated for a subwindow, we should show the opposite there */
			for (j = 0; j < ANGBAND_TERM_MAX; j++) {
				/* Unused */
				if (!angband_term[j]) continue;

				/* Count windows displaying inven */
				if (window_flag[j] & (PW_INVEN)) ni++;

				/* Count windows displaying equip */
				if (window_flag[j] & (PW_EQUIP)) ne++;
			}

			/* Are we in the situation where toggling makes sense? */
			if ((ni && !ne) || (!ni && ne)) {
				if (player->upkeep->command_wrk == USE_EQUIP) {
					if ((ne && !toggle) || (ni && toggle)) {
						/* Main screen is equipment, so is subwindow */
						toggle_inven_equip();
						toggle = !toggle;
					}
				} else if (player->upkeep->command_wrk == USE_INVEN) {
					if ((ni && !toggle) || (ne && toggle)) {
						/* Main screen is inventory, so is subwindow */
						toggle_inven_equip();
						toggle = !toggle;
					}
				} else {
					/* Quiver or floor, go back to the original */
					if (toggle) {
						toggle_inven_equip();
						toggle = !toggle;
					}
				}
			}

			/* Redraw */
			player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

			/* Redraw windows */
			redraw_stuff(player);

			/* Save screen */
			screen_save();

			/* Build object list */
			wipe_obj_list();
			if (player->upkeep->command_wrk == USE_INVEN)
				build_obj_list(i2, player->upkeep->inven, tester_m, olist_mode);
			else if (player->upkeep->command_wrk == USE_EQUIP)
				build_obj_list(e2, NULL, tester_m, olist_mode);
			else if (player->upkeep->command_wrk == USE_QUIVER)
				build_obj_list(q2, player->upkeep->quiver, tester_m,olist_mode);
			else if (player->upkeep->command_wrk == USE_FLOOR)
				build_obj_list(f2, floor_list, tester_m, olist_mode);
			else if (player->upkeep->command_wrk == SHOW_THROWING)
				build_obj_list(throwing_num, throwing_list, tester_m,
							   olist_mode);

			/* Show the prompt */
			menu_header();
			if (pmt) {
				prt(pmt, 0, 0);
				prt(header, 0, strlen(pmt) + 1);
			}

			/* No menu change request */
			newmenu = false;

			/* Get an item choice */
			*choice = item_menu(cmd, MAX(pmt ? strlen(pmt) : 0, 15), mode);

			/* Fix the screen */
			screen_load();

			/* Update */
			player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
			redraw_stuff(player);

			/* Clear the prompt line */
			prt("", 0, 0);

			/* We have a selection, or are backing out */
			if (*choice || !newmenu) {
				if (toggle) toggle_inven_equip();
				break;
			}
		}
	} else {
		/* Warning if needed */
		if (str) msg("%s", str);
		*choice = NULL;
	}

	/* Clean up */
	player->upkeep->command_wrk = 0;
	mem_free(throwing_list);
	mem_free(floor_list);

	/* Result */
	return (*choice != NULL) ? true : false;
}


/**
 * ------------------------------------------------------------------------
 * Object recall
 * ------------------------------------------------------------------------ */


/**
 * This draws the Object Recall subwindow when displaying a particular object
 * (e.g. a helmet in the backpack, or a scroll on the ground)
 */
void display_object_recall(struct object *obj)
{
	char header_buf[120];

	textblock *tb = object_info(obj, OINFO_NONE);
	object_desc(header_buf, sizeof(header_buf), obj, ODESC_PREFIX | ODESC_FULL);

	clear_from(0);
	textui_textblock_place(tb, SCREEN_REGION, header_buf);
	textblock_free(tb);
}


/**
 * This draws the Object Recall subwindow when displaying a recalled item kind
 * (e.g. a generic ring of acid or a generic blade of chaos)
 */
void display_object_kind_recall(struct object_kind *kind)
{
	struct object object = OBJECT_NULL, known_obj = OBJECT_NULL;
	object_prep(&object, kind, 0, EXTREMIFY);
	object.known = &known_obj;

	display_object_recall(&object);
}

/**
 * Display object recall modally and wait for a keypress.
 *
 * This is set up for use in look mode (see target_set_interactive_aux()).
 *
 * \param obj is the object to be described.
 */
void display_object_recall_interactive(struct object *obj)
{
	char header_buf[120];
	textblock *tb;

	event_signal(EVENT_MESSAGE_FLUSH);

	tb = object_info(obj, OINFO_NONE);
	object_desc(header_buf, sizeof(header_buf), obj, ODESC_PREFIX | ODESC_FULL);
	textui_textblock_show(tb, SCREEN_REGION, header_buf);
	textblock_free(tb);
}

/**
 * Examine an object
 */
void textui_obj_examine(void)
{
	char header_buf[120];

	textblock *tb;
	region local_area = { 0, 0, 0, 0 };

	struct object *obj;

	/* Select item */
	if (!get_item(&obj, "Examine which item?", "You have nothing to examine.",
			CMD_NULL, NULL, (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR | IS_HARMLESS)))
		return;

	/* Track object for object recall */
	track_object(player->upkeep, obj);

	/* Display info */
	tb = object_info(obj, OINFO_NONE);
	object_desc(header_buf, sizeof(header_buf), obj,
			ODESC_PREFIX | ODESC_FULL | ODESC_CAPITAL);

	textui_textblock_show(tb, local_area, header_buf);
	textblock_free(tb);
}


/**
 * ------------------------------------------------------------------------
 * Object ignore interface
 * ------------------------------------------------------------------------ */

enum {
	IGNORE_THIS_ITEM,
	UNIGNORE_THIS_ITEM,
	IGNORE_THIS_FLAVOR,
	UNIGNORE_THIS_FLAVOR,
	IGNORE_THIS_EGO,
	UNIGNORE_THIS_EGO,
	IGNORE_THIS_QUALITY
};

void textui_cmd_ignore_menu(struct object *obj)
{
	char out_val[160];

	struct menu *m;
	region r;
	int selected;
	byte value;
	int type;

	if (!obj)
		return;

	m = menu_dynamic_new();
	m->selections = lower_case;

	/* Basic ignore option */
	if (!(obj->known->notice & OBJ_NOTICE_IGNORE)) {
		menu_dynamic_add(m, "This item only", IGNORE_THIS_ITEM);
	} else {
		menu_dynamic_add(m, "Unignore this item", UNIGNORE_THIS_ITEM);
	}

	/* Flavour-aware ignore */
	if (ignore_tval(obj->tval) &&
			(!obj->artifact || !object_flavor_is_aware(obj))) {
		bool ignored = kind_is_ignored_aware(obj->kind) ||
				kind_is_ignored_unaware(obj->kind);

		char tmp[70];
		object_desc(tmp, sizeof(tmp), obj,
					ODESC_NOEGO | ODESC_BASE | ODESC_PLURAL);
		if (!ignored) {
			strnfmt(out_val, sizeof out_val, "All %s", tmp);
			menu_dynamic_add(m, out_val, IGNORE_THIS_FLAVOR);
		} else {
			strnfmt(out_val, sizeof out_val, "Unignore all %s", tmp);
			menu_dynamic_add(m, out_val, UNIGNORE_THIS_FLAVOR);
		}
	}

	/* Ego ignoring */
	if (obj->known->ego) {
		struct ego_desc choice;
		struct ego_item *ego = obj->ego;
		char tmp[80] = "";

		choice.e_idx = ego->eidx;
		choice.itype = ignore_type_of(obj);
		choice.short_name = "";
		(void) ego_item_name(tmp, sizeof(tmp), &choice);
		if (!ego_is_ignored(choice.e_idx, choice.itype)) {
			strnfmt(out_val, sizeof out_val, "All %s", tmp + 4);
			menu_dynamic_add(m, out_val, IGNORE_THIS_EGO);
		} else {
			strnfmt(out_val, sizeof out_val, "Unignore all %s", tmp + 4);
			menu_dynamic_add(m, out_val, UNIGNORE_THIS_EGO);
		}
	}

	/* Quality ignoring */
	value = ignore_level_of(obj);
	type = ignore_type_of(obj);

	if (tval_is_jewelry(obj) &&	ignore_level_of(obj) != IGNORE_BAD)
		value = IGNORE_MAX;

	if (value != IGNORE_MAX && type != ITYPE_MAX) {
		strnfmt(out_val, sizeof out_val, "All %s %s",
				quality_values[value].name, ignore_name_for_type(type));

		menu_dynamic_add(m, out_val, IGNORE_THIS_QUALITY);
	}

	/* Work out display region */
	r.width = menu_dynamic_longest_entry(m) + 3 + 2; /* +3 for tag, 2 for pad */
	r.col = 80 - r.width;
	r.row = 1;
	r.page_rows = m->count;

	screen_save();
	menu_layout(m, &r);
	region_erase_bordered(&r);

	prt("(Enter to select, ESC) Ignore:", 0, 0);
	selected = menu_dynamic_select(m);

	screen_load();

	if (selected == IGNORE_THIS_ITEM) {
		obj->known->notice |= OBJ_NOTICE_IGNORE;
	} else if (selected == UNIGNORE_THIS_ITEM) {
		obj->known->notice &= ~(OBJ_NOTICE_IGNORE);
	} else if (selected == IGNORE_THIS_FLAVOR) {
		object_ignore_flavor_of(obj);
	} else if (selected == UNIGNORE_THIS_FLAVOR) {
		kind_ignore_clear(obj->kind);
	} else if (selected == IGNORE_THIS_EGO) {
		ego_ignore(obj);
	} else if (selected == UNIGNORE_THIS_EGO) {
		ego_ignore_clear(obj);
	} else if (selected == IGNORE_THIS_QUALITY) {
		byte ignore_value = ignore_level_of(obj);
		int ignore_type = ignore_type_of(obj);

		ignore_level[ignore_type] = ignore_value;
	}

	player->upkeep->notice |= PN_IGNORE;

	menu_dynamic_free(m);
}

void textui_cmd_ignore(void)
{
	struct object *obj;

	/* Get an item */
	const char *q = "Ignore which item? ";
	const char *s = "You have nothing to ignore.";
	if (!get_item(&obj, q, s, CMD_IGNORE, NULL,
				  USE_INVEN | USE_QUIVER | USE_EQUIP | USE_FLOOR))
		return;

	textui_cmd_ignore_menu(obj);
}

void textui_cmd_toggle_ignore(void)
{
	player->unignoring = !player->unignoring;
	player->upkeep->notice |= PN_IGNORE;
	do_cmd_redraw();
}

