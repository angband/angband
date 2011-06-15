/*
 * File: cmd3.c
 * Purpose: Miscellaneous queries
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
#include "cave.h"
#include "cmds.h"
#include "monster/monster.h"
#include "object/inventory.h"
#include "object/tvalsval.h"
#include "object/object.h"
#include "squelch.h"
#include "ui-menu.h"
#include "target.h"

/*
 * Display inventory
 */
void do_cmd_inven(void)
{
	ui_event e;
	int diff = weight_remaining();

	/* Hack -- Start in "inventory" mode */
	p_ptr->command_wrk = (USE_INVEN);

	/* Save screen */
	screen_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the inventory */
	show_inven(OLIST_WEIGHT | OLIST_QUIVER);

	/* Hack -- hide empty slots */
	item_tester_full = FALSE;

	/* Prompt for a command */
	prt(format("(Inventory) Burden %d.%d lb (%d.%d lb %s). Command: ",
		        p_ptr->total_weight / 10, p_ptr->total_weight % 10,
		        abs(diff) / 10, abs(diff) % 10,
		        (diff < 0 ? "overweight" : "remaining")),
	    0, 0);

	/* Get a new command */
	e = inkey_ex();
	if (!(e.type == EVT_KBRD && e.key.code == ESCAPE))
		Term_event_push(&e);

	/* Load screen */
	screen_load();
}


/*
 * Display equipment
 */
void do_cmd_equip(void)
{
	ui_event e;

	/* Hack -- Start in "equipment" mode */
	p_ptr->command_wrk = (USE_EQUIP);

	/* Save screen */
	screen_save();

	/* Hack -- show empty slots */
	item_tester_full = TRUE;

	/* Display the equipment */
	show_equip(OLIST_WEIGHT);

	/* Hack -- undo the hack above */
	item_tester_full = FALSE;

	/* Prompt for a command */
	prt("(Equipment) Command: ", 0, 0);

	/* Get a new command */
	e = inkey_ex();
	if (!(e.type == EVT_KBRD && e.key.code == ESCAPE))
		Term_event_push(&e);

	/* Load screen */
	screen_load();
}

enum
{
	IGNORE_THIS_ITEM,
	UNIGNORE_THIS_ITEM,
	IGNORE_THIS_FLAVOR,
	UNIGNORE_THIS_FLAVOR,
	IGNORE_THIS_QUALITY
};

void textui_cmd_destroy(void)
{
	int item;
	object_type *o_ptr;

	char out_val[160];

	menu_type *m;
	region r;
	int selected;

	/* Get an item */
	const char *q = "Ignore which item? ";
	const char *s = "You have nothing to ignore.";
	if (!get_item(&item, q, s, CMD_DESTROY, USE_INVEN | USE_EQUIP | USE_FLOOR))
		return;

	o_ptr = object_from_item_idx(item);

	m = menu_dynamic_new();
	m->selections = lower_case;

	/* Basic ignore option */
	if (!o_ptr->ignore) {
		menu_dynamic_add(m, "This item only", IGNORE_THIS_ITEM);
	} else {
		menu_dynamic_add(m, "Unignore this item", UNIGNORE_THIS_ITEM);
	}

	/* Flavour-aware squelch */
	if (squelch_tval(o_ptr->tval) &&
			(!o_ptr->artifact || !object_flavor_is_aware(o_ptr))) {
		bool squelched = kind_is_squelched_aware(o_ptr->kind) ||
				kind_is_squelched_unaware(o_ptr->kind);

		char tmp[70];
		object_desc(tmp, sizeof(tmp), o_ptr, ODESC_BASE | ODESC_PLURAL);
		if (!squelched) {
			strnfmt(out_val, sizeof out_val, "All %s", tmp);
			menu_dynamic_add(m, out_val, IGNORE_THIS_FLAVOR);
		} else {
			strnfmt(out_val, sizeof out_val, "Unignore all %s", tmp);
			menu_dynamic_add(m, out_val, UNIGNORE_THIS_FLAVOR);
		}
	}

	/* Quality squelching */
	if (object_was_sensed(o_ptr) || object_was_worn(o_ptr) ||
			object_is_known_not_artifact(o_ptr)) {
		byte value = squelch_level_of(o_ptr);
		int type = squelch_type_of(o_ptr);

		if (object_is_jewelry(o_ptr) &&
					squelch_level_of(o_ptr) != SQUELCH_BAD)
			value = SQUELCH_MAX;

		if (value != SQUELCH_MAX && type != TYPE_MAX) {
			strnfmt(out_val, sizeof out_val, "All %s %s",
					quality_values[value].name, quality_choices[type].name);

			menu_dynamic_add(m, out_val, IGNORE_THIS_QUALITY);
		}
	}

	/* work out display region */
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
		cmd_insert(CMD_DESTROY);
		cmd_set_arg_item(cmd_get_top(), 0, item);
	} else if (selected == UNIGNORE_THIS_ITEM) {
		o_ptr->ignore = FALSE;
	} else if (selected == IGNORE_THIS_FLAVOR) {
		object_squelch_flavor_of(o_ptr);
	} else if (selected == UNIGNORE_THIS_FLAVOR) {
		kind_squelch_clear(o_ptr->kind);
	} else if (selected == IGNORE_THIS_QUALITY) {
		byte value = squelch_level_of(o_ptr);
		int type = squelch_type_of(o_ptr);

		squelch_level[type] = value;
	}

	p_ptr->notice |= PN_SQUELCH;

	menu_dynamic_free(m);
}

void textui_cmd_toggle_ignore(void)
{
	p_ptr->unignoring = !p_ptr->unignoring;
	p_ptr->notice |= PN_SQUELCH;
	do_cmd_redraw();
}

/* Examine an object */
void textui_obj_examine(void)
{
	char header[120];

	textblock *tb;
	region area = { 0, 0, 0, 0 };

	object_type *o_ptr;
	int item;

	/* Select item */
	if (!get_item(&item, "Examine which item?", "You have nothing to examine.",
			CMD_NULL, (USE_EQUIP | USE_INVEN | USE_FLOOR | IS_HARMLESS)))
		return;

	/* Track object for object recall */
	track_object(item);

	/* Display info */
	o_ptr = object_from_item_idx(item);
	tb = object_info(o_ptr, OINFO_NONE);
	object_desc(header, sizeof(header), o_ptr, ODESC_PREFIX | ODESC_FULL);

	textui_textblock_show(tb, area, format("%^s", header));
	textblock_free(tb);
}


/*
 * Target command
 */
void do_cmd_target(void)
{
	if (target_set_interactive(TARGET_KILL, -1, -1))
		msg("Target Selected.");
	else
		msg("Target Aborted.");
}


void do_cmd_target_closest(void)
{
	target_set_closest(TARGET_KILL);
}


/*
 * Look command
 */
void do_cmd_look(void)
{
	/* Look around */
	if (target_set_interactive(TARGET_LOOK, -1, -1))
	{
		msg("Target Selected.");
	}
}



/*
 * Allow the player to examine other sectors on the map
 */
void do_cmd_locate(void)
{
	int dir, y1, x1, y2, x2;

	char tmp_val[80];

	char out_val[160];


	/* Start at current panel */
	y1 = Term->offset_y;
	x1 = Term->offset_x;

	/* Show panels until done */
	while (1)
	{
		/* Get the current panel */
		y2 = Term->offset_y;
		x2 = Term->offset_x;
		
		/* Describe the location */
		if ((y2 == y1) && (x2 == x1))
		{
			tmp_val[0] = '\0';
		}
		else
		{
			strnfmt(tmp_val, sizeof(tmp_val), "%s%s of",
			        ((y2 < y1) ? " north" : (y2 > y1) ? " south" : ""),
			        ((x2 < x1) ? " west" : (x2 > x1) ? " east" : ""));
		}

		/* Prepare to ask which way to look */
		strnfmt(out_val, sizeof(out_val),
		        "Map sector [%d,%d], which is%s your sector.  Direction?",
		        (y2 / PANEL_HGT), (x2 / PANEL_WID), tmp_val);

		/* More detail */
		if (OPT(center_player))
		{
			strnfmt(out_val, sizeof(out_val),
		        	"Map sector [%d(%02d),%d(%02d)], which is%s your sector.  Direction?",
		        	(y2 / PANEL_HGT), (y2 % PANEL_HGT),
		        	(x2 / PANEL_WID), (x2 % PANEL_WID), tmp_val);
		}

		/* Assume no direction */
		dir = 0;

		/* Get a direction */
		while (!dir)
		{
			struct keypress command;

			/* Get a command (or Cancel) */
			if (!get_com(out_val, &command)) break;

			/* Extract direction */
			dir = target_dir(command);

			/* Error */
			if (!dir) bell("Illegal direction for locate!");
		}

		/* No direction */
		if (!dir) break;

		/* Apply the motion */
		change_panel(dir);

		/* Handle stuff */
		handle_stuff(p_ptr);
	}

	/* Verify panel */
	verify_panel();
}

static int cmp_mexp(const void *a, const void *b)
{
	u16b ia = *(const u16b *)a;
	u16b ib = *(const u16b *)b;
	if (r_info[ia].mexp < r_info[ib].mexp)
		return -1;
	if (r_info[ia].mexp > r_info[ib].mexp)
		return 1;
	return (a < b ? -1 : (a > b ? 1 : 0));
}

static int cmp_level(const void *a, const void *b)
{
	u16b ia = *(const u16b *)a;
	u16b ib = *(const u16b *)b;
	if (r_info[ia].level < r_info[ib].level)
		return -1;
	if (r_info[ia].level > r_info[ib].level)
		return 1;
	return cmp_mexp(a, b);
}

static int cmp_tkill(const void *a, const void *b)
{
	u16b ia = *(const u16b *)a;
	u16b ib = *(const u16b *)b;
	if (l_list[ia].tkills < l_list[ib].tkills)
		return -1;
	if (l_list[ia].tkills > l_list[ib].tkills)
		return 1;
	return cmp_level(a, b);
}

static int cmp_pkill(const void *a, const void *b)
{
	u16b ia = *(const u16b *)a;
	u16b ib = *(const u16b *)b;
	if (l_list[ia].pkills < l_list[ib].pkills)
		return -1;
	if (l_list[ia].pkills > l_list[ib].pkills)
		return 1;
	return cmp_tkill(a, b);
}

int cmp_monsters(const void *a, const void *b)
{
	return cmp_level(a, b);
}

/*
 * Search the monster, item, and feature types to find the
 * meaning for the given symbol.
 *
 * Note: We currently search items first, then features, then
 * monsters, and we return the first hit for a symbol.
 * This is to prevent mimics and lurkers from matching
 * a symbol instead of the item or feature it is mimicking.
 *
 * Todo: concatenate all matches into buf. This will be much
 * easier once we can loop through item tvals instead of items
 * (see note below.)
 *
 * Todo: Should this take the user's pref files into account?
 */
static void lookup_symbol(struct keypress sym, char *buf, size_t max)
{
	int i;
	monster_base *race;

	/* Look through items */
	/* Note: We currently look through all items, and grab the tval when we find a match.
	It would make more sense to loop through tvals, but then we need to associate
	a display character with each tval. */
	for (i = 1; i < z_info->k_max; i++) {
		if (k_info[i].d_char == (char)sym.code) {
			strnfmt(buf, max, "%c - %s.", (char)sym.code, tval_find_name(k_info[i].tval));
			return;
		}
	}

	/* Look through features */
	/* Note: We need a better way of doing this. Currently '#' matches secret door,
	and '^' matches trap door (instead of the more generic "trap"). */
	for (i = 1; i < z_info->f_max; i++) {
		if (f_info[i].d_char == (char)sym.code) {
			strnfmt(buf, max, "%c - %s.", (char)sym.code, f_info[i].name);
			return;
		}
	}
	
	/* Look through monster templates */
	for (race = rb_info; race; race = race->next){
		if ((char)sym.code == race->d_char) {
			strnfmt(buf, max, "%c - %s.", (char)sym.code, race->text);
			return;
		}
	}

	/* No matches */
	strnfmt(buf, max, "%c - %s.", (char)sym.code, "Unknown Symbol");
	
	return;
}

/*
 * Identify a character, allow recall of monsters
 *
 * Several "special" responses recall "multiple" monsters:
 *   ^A (all monsters)
 *   ^U (all unique monsters)
 *   ^N (all non-unique monsters)
 *
 * The responses may be sorted in several ways, see below.
 *
 * Note that the player ghosts are ignored, since they do not exist.
 */
void do_cmd_query_symbol(void)
{
	int i, n, r_idx;
	char buf[128];

	struct keypress sym;
	struct keypress query;

	bool all = FALSE;
	bool uniq = FALSE;
	bool norm = FALSE;

	bool recall = FALSE;

	u16b *who;

	/* Get a character, or abort */
	if (!get_com("Enter character to be identified, or control+[ANU]: ", &sym))
		return;

	/* Describe */
	if (sym.code == KTRL('A'))
	{
		all = TRUE;
		my_strcpy(buf, "Full monster list.", sizeof(buf));
	}
	else if (sym.code == KTRL('U'))
	{
		all = uniq = TRUE;
		my_strcpy(buf, "Unique monster list.", sizeof(buf));
	}
	else if (sym.code == KTRL('N'))
	{
		all = norm = TRUE;
		my_strcpy(buf, "Non-unique monster list.", sizeof(buf));
	}
	else
	{
		lookup_symbol(sym, buf, sizeof(buf));
	}

	/* Display the result */
	prt(buf, 0, 0);

	/* Allocate the "who" array */
	who = C_ZNEW(z_info->r_max, u16b);

	/* Collect matching monsters */
	for (n = 0, i = 1; i < z_info->r_max - 1; i++)
	{
		monster_race *r_ptr = &r_info[i];
		monster_lore *l_ptr = &l_list[i];

		/* Nothing to recall */
		if (!OPT(cheat_know) && !l_ptr->sights) continue;

		/* Require non-unique monsters if needed */
		if (norm && rf_has(r_ptr->flags, RF_UNIQUE)) continue;

		/* Require unique monsters if needed */
		if (uniq && !rf_has(r_ptr->flags, RF_UNIQUE)) continue;

		/* Collect "appropriate" monsters */
		if (all || (r_ptr->d_char == (char)sym.code)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n)
	{
		/* XXX XXX Free the "who" array */
		FREE(who);

		return;
	}

	/* Buttons */
	button_add("[y]", 'y');
	button_add("[k]", 'k');
	/* Don't collide with the repeat button */
	button_add("[n]", 'q'); 
	redraw_stuff(p_ptr);

	/* Prompt */
	put_str("Recall details? (y/k/n): ", 0, 40);

	/* Query */
	query = inkey();

	/* Restore */
	prt(buf, 0, 0);

	/* Buttons */
	button_kill('y');
	button_kill('k');
	button_kill('q');
	redraw_stuff(p_ptr);

	/* Interpret the response */
	if (query.code == 'k')
	{
		/* Sort by kills (and level) */
		sort(who, n, sizeof(*who), cmp_pkill);
	}
	else if (query.code == 'y' || query.code == 'p')
	{
		/* Sort by level; accept 'p' as legacy */
		sort(who, n, sizeof(*who), cmp_level);
	}
	else
	{
		/* Any unsupported response is "nope, no history please" */
	
		/* XXX XXX Free the "who" array */
		FREE(who);

		return;
	}

	/* Start at the end */
	i = n - 1;

	/* Button */
	button_add("[r]", 'r');
	button_add("[-]", '-');
	button_add("[+]", '+');
	redraw_stuff(p_ptr);

	/* Scan the monster memory */
	while (1)
	{
		/* Extract a race */
		r_idx = who[i];

		/* Hack -- Auto-recall */
		monster_race_track(r_idx);

		/* Hack -- Handle stuff */
		handle_stuff(p_ptr);

		/* Hack -- Begin the prompt */
		roff_top(r_idx);

		/* Hack -- Complete the prompt */
		Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");

		/* Interact */
		while (1)
		{
			/* Recall */
			if (recall)
			{
				/* Save screen */
				screen_save();

				/* Recall on screen */
				screen_roff(who[i]);

				/* Hack -- Complete the prompt (again) */
				Term_addstr(-1, TERM_WHITE, " [(r)ecall, ESC]");
			}

			/* Command */
			query = inkey();

			/* Unrecall */
			if (recall)
			{
				/* Load screen */
				screen_load();
			}

			/* Normal commands */
			if (query.code != 'r') break;

			/* Toggle recall */
			recall = !recall;
		}

		/* Stop scanning */
		if (query.code == ESCAPE) break;

		/* Move to "prev" monster */
		if (query.code == '-')
		{
			if (++i == n)
				i = 0;
		}

		/* Move to "next" monster */
		else
		{
			if (i-- == 0)
				i = n - 1;
		}
	}

	/* Button */
	button_kill('r');
	button_kill('-');
	button_kill('+');
	redraw_stuff(p_ptr);

	/* Re-display the identity */
	prt(buf, 0, 0);

	/* Free the "who" array */
	FREE(who);
}

/* Centers the map on the player */
void do_cmd_center_map(void)
{
	center_panel();
}
