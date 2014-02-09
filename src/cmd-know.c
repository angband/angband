/*
 * File: cmd4.c
 * Purpose: Various kinds of browsing functions.
 *
 * Copyright (c) 1997-2007 Robert A. Koeneke, James E. Wilson, Ben Harrison,
 * Eytan Zweig, Andrew Doull, Pete Mack.
 * Copyright (c) 2004 DarkGod (HTML dump code)
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
#include "cmds.h"
#include "files.h"
#include "history.h"
#include "object/tvalsval.h"
#include "monster/mon-lore.h"
#include "monster/mon-list.h"
#include "monster/mon-util.h"
#include "object/obj-list.h"
#include "option.h"
#include "prefs.h"
#include "squelch.h"
#include "target.h"
#include "ui-menu.h"
#include "ui.h"



#define INFO_SCREENS 2 /* Number of screens in character info mode */



/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This command is also used to "instantiate" the results of the user
 * selecting various things, such as graphics mode, so it must call
 * the "TERM_XTRA_REACT" hook before redrawing the windows.
 *
 */
void do_cmd_redraw(void)
{
	int j;

	term *old = Term;


	/* Low level flush */
	Term_flush();

	/* Reset "inkey()" */
	flush();

	if (character_dungeon)
		verify_panel();


	/* Hack -- React to changes */
	Term_xtra(TERM_XTRA_REACT, 0);


	/* Combine and Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);


	/* Update torch */
	p_ptr->update |= (PU_TORCH);

	/* Update stuff */
	p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Fully update the visuals */
	p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Redraw everything */
	p_ptr->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN | PR_EQUIP |
	                  PR_MESSAGE | PR_MONSTER | PR_OBJECT |
					  PR_MONLIST | PR_ITEMLIST);

	/* Clear screen */
	Term_clear();

	/* Hack -- update */
	handle_stuff(p_ptr);

	/* Place the cursor on the player */
	if (0 != character_dungeon)
		move_cursor_relative(p_ptr->px, p_ptr->py);


	/* Redraw every window */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		if (!angband_term[j]) continue;

		Term_activate(angband_term[j]);
		Term_redraw();
		Term_fresh();
		Term_activate(old);
	}
}


/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
	ui_event ke;
	int mode = 0;

	const char *p;

	bool more = TRUE;

	/* Prompt */
	p = "['c' to change name, 'f' to file, 'h' to change mode, or ESC]";

	/* Save screen */
	screen_save();

	/* Forever */
	while (more)
	{
		/* Display the player */
		display_player(mode);

		/* Prompt */
		Term_putstr(2, 23, -1, TERM_WHITE, p);

		/* Query */
		ke = inkey_ex();

		if ((ke.type == EVT_KBRD)||(ke.type == EVT_BUTTON)) {
			switch (ke.key.code) {
				case ESCAPE: more = FALSE; break;
				case 'c': {
					char namebuf[32] = "";

					/* Set player name */
					if (get_name(namebuf, sizeof namebuf))
						my_strcpy(op_ptr->full_name, namebuf,
								  sizeof(op_ptr->full_name));

					break;
				}

				case 'f': {
					char buf[1024];
					char fname[80];

					strnfmt(fname, sizeof fname, "%s.txt", player_safe_name(p_ptr, FALSE));

					if (get_file(fname, buf, sizeof buf))
					{
						if (file_character(buf, FALSE) != 0)
							msg("Character dump failed!");
						else
							msg("Character dump successful.");
					}
					break;
				}
				
				case 'h':
				case ARROW_LEFT:
				case ' ':
					mode = (mode + 1) % INFO_SCREENS;
					break;

				case 'l':
				case ARROW_RIGHT:
					mode = (mode - 1) % INFO_SCREENS;
					break;
			}
		} else if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				/* Flip through the screens */			
				mode = (mode + 1) % INFO_SCREENS;
			} else
			if (ke.mouse.button == 2) {
				/* exit the screen */
				more = FALSE;
			} else
			{
				/* Flip backwards through the screens */			
				mode = (mode - 1) % INFO_SCREENS;
			}
		}

		/* Flush messages */
		message_flush();
	}

	/* Load screen */
	screen_load();
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
	/* Recall one message XXX XXX XXX */
	c_prt(message_color(0), format( "> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 *
 * Note that messages may be longer than 80 characters, but they are
 * displayed using "infinite" length, with a special sub-command to
 * "slide" the virtual display to the left or right.
 *
 * Attempt to only highlight the matching portions of the string.
 */
void do_cmd_messages(void)
{
	ui_event ke;

	bool more = TRUE;

	int i, j, n, q;
	int wid, hgt;

	char shower[80] = "";



	/* Total messages */
	n = messages_num();

	/* Start on first message */
	i = 0;

	/* Start at leftmost edge */
	q = 0;

	/* Get size */
	Term_get_size(&wid, &hgt);

	/* Save screen */
	screen_save();

	/* Process requests until done */
	while (more)
	{
		/* Clear screen */
		Term_clear();

		/* Dump messages */
		for (j = 0; (j < hgt - 4) && (i + j < n); j++)
		{
			const char *msg;
			const char *str = message_str(i + j);
			byte attr = message_color(i + j);
			u16b count = message_count(i + j);

			if (count == 1)
				msg = str;
			else
				msg = format("%s <%dx>", str, count);

			/* Apply horizontal scroll */
			msg = ((int)strlen(msg) >= q) ? (msg + q) : "";

			/* Dump the messages, bottom to top */
			Term_putstr(0, hgt - 3 - j, -1, attr, msg);

			/* Highlight "shower" */
			if (shower[0])
			{
				str = msg;

				/* Display matches */
				while ((str = my_stristr(str, shower)) != NULL)
				{
					int len = strlen(shower);

					/* Display the match */
					Term_putstr(str-msg, hgt - 3 - j, len, TERM_YELLOW, str);

					/* Advance */
					str += len;
				}
			}
		}

		/* Display header */
		prt(format("Message recall (%d-%d of %d), offset %d", i, i + j - 1, n, q), 0, 0);

		/* Display prompt (not very informative) */
		if (shower[0])
			prt("[Movement keys to navigate, '-' for next, '=' to find]", hgt - 1, 0);
		else
			prt("[Movement keys to navigate, '=' to find, or ESCAPE to exit]", hgt - 1, 0);
			

		/* Get a command */
		ke = inkey_ex();


		/* Scroll forwards or backwards using mouse clicks */
		if (ke.type == EVT_MOUSE) {
			if (ke.mouse.button == 1) {
				if (ke.mouse.y <= hgt / 2) {
					/* Go older if legal */
					if (i + 20 < n)
						i += 20;
				} else {
					/* Go newer */
					i = (i >= 20) ? (i - 20) : 0;
				}
			} else
			if (ke.mouse.button == 2) {
				more = FALSE;
			}
		} else if (ke.type == EVT_KBRD) {
			switch (ke.key.code) {
				case ESCAPE: {
					more = FALSE;
					break;
				}

				case '=': {
					/* Get the string to find */
					prt("Find: ", hgt - 1, 0);
					if (!askfor_aux(shower, sizeof shower, NULL)) continue;
		
					/* Set to find */
					ke.key.code = '-';
					break;
				}

				case ARROW_LEFT:
				case '4':
					q = (q >= wid / 2) ? (q - wid / 2) : 0;
					break;

				case ARROW_RIGHT:
				case '6':
					q = q + wid / 2;
					break;

				case ARROW_UP:
				case '8':
					if (i + 1 < n) i += 1;
					break;

				case ARROW_DOWN:
				case '2':
				case KC_ENTER:
					i = (i >= 1) ? (i - 1) : 0;
					break;

				case KC_PGUP:
				case 'p':
				case ' ':
					if (i + 20 < n) i += 20;
					break;

				case KC_PGDOWN:
				case 'n':
					i = (i >= 20) ? (i - 20) : 0;
					break;
			}
		}

		/* Find the next item */
		if (ke.key.code == '-' && shower[0])
		{
			s16b z;

			/* Scan messages */
			for (z = i + 1; z < n; z++)
			{
				/* Search for it */
				if (my_stristr(message_str(z), shower))
				{
					/* New location */
					i = z;

					/* Done */
					break;
				}
			}
		}
	}

	/* Load screen */
	screen_load();
}



#define GET_ITEM_PARAMS \
 	(USE_EQUIP | USE_INVEN | USE_FLOOR | SHOW_QUIVER | SHOW_EMPTY | IS_HARMLESS)
 
/*
 * Display inventory
 */
void do_cmd_inven(void)
{
	int item;
	int ret = 3;
	int diff = weight_remaining();

	if (!p_ptr->inventory[0].kind) {
		msg("You have nothing in your inventory.");
		return;
	}

	/* Hack -- Start in "inventory" mode */
	p_ptr->command_wrk = (USE_INVEN);

	/* Loop this menu until an object context menu says differently */
	while (ret == 3) {
		/* Save screen */
		screen_save();

		/* Prompt for a command */
		prt(format("(Inventory) Burden %d.%d lb (%d.%d lb %s). Select Item: ",
			        p_ptr->total_weight / 10, p_ptr->total_weight % 10,
			        abs(diff) / 10, abs(diff) % 10,
			        (diff < 0 ? "overweight" : "remaining")),
				0, 0);

		/* Get an item to use a context command on (Display the inventory) */
		if (get_item(&item, NULL, NULL, CMD_NULL, GET_ITEM_PARAMS)) {
			object_type *o_ptr;

			/* Load screen */
			screen_load();

			o_ptr = object_from_item_idx(item);

			if (o_ptr && o_ptr->kind) {
				/* Track the object kind */
				track_object(item);

				while ((ret = context_menu_object(o_ptr, item)) == 2);
			}
		} else {
			/* Load screen */
			screen_load();

			ret = -1;
		}
	}
}


/*
 * Display equipment
 */
void do_cmd_equip(void)
{
	int item;
	int ret = 3;

	/* Check inventory, since get_item() will default to inventory list when equipment is empty. */
	if (!p_ptr->inventory[0].kind) {
		msg("You are not wielding or wearing anything.");
		return;
	}

	/* Hack -- Start in "inventory" mode */
	p_ptr->command_wrk = (USE_EQUIP);

	/* Loop this menu until an object context menu says differently */
	while (ret == 3) {
		/* Save screen */
		screen_save();

		/* Get an item to use a context command on (Display the inventory) */
		if (get_item(&item, "Select Item:", NULL, CMD_NULL, GET_ITEM_PARAMS)) {
			object_type *o_ptr;

			/* Load screen */
			screen_load();

			o_ptr = object_from_item_idx(item);

			if (o_ptr && o_ptr->kind) {
				/* Track the object kind */
				track_object(item);

				while ((ret = context_menu_object(o_ptr, item)) == 2);
			}
		} else {
			/* Load screen */
			screen_load();

			ret = -1;
		}
	}
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
		if (char_matches_key(k_info[i].d_char, sym.code)) {
			strnfmt(buf, max, "%c - %s.", (char)sym.code, tval_find_name(k_info[i].tval));
			return;
		}
	}

	/* Look through features */
	/* Note: We need a better way of doing this. Currently '#' matches secret door,
	and '^' matches trap door (instead of the more generic "trap"). */
	for (i = 1; i < z_info->f_max; i++) {
		if (char_matches_key(f_info[i].d_char, sym.code)) {
			strnfmt(buf, max, "%c - %s.", (char)sym.code, f_info[i].name);
			return;
		}
	}
	
	/* Look through monster templates */
	for (race = rb_info; race; race = race->next){
		if (char_matches_key(race->d_char, sym.code)) {
			strnfmt(buf, max, "%c - %s.", (char)sym.code, race->text);
			return;
		}
	}

	/* No matches */
        if (isprint((char)sym.code)) {
	    strnfmt(buf, max, "%c - Unknown Symbol.", (char)sym.code);
        } else {
	    strnfmt(buf, max, "? - Unknown Symbol.");
        }
	
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
	int i, n;
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
		if (all || char_matches_key(r_ptr->d_char, sym.code)) who[n++] = i;
	}

	/* Nothing to recall */
	if (!n)
	{
		/* XXX XXX Free the "who" array */
		FREE(who);

		return;
	}

	/* Prompt */
	put_str("Recall details? (y/k/n): ", 0, 40);

	/* Query */
	query = inkey();

	/* Restore */
	prt(buf, 0, 0);

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

	/* Scan the monster memory */
	while (1)
	{
		textblock *tb;

		/* Extract a race */
		int r_idx = who[i];
		monster_race *r_ptr = &r_info[r_idx];
		monster_lore *l_ptr = &l_list[r_idx];

		/* Hack -- Auto-recall */
		monster_race_track(r_ptr);

		/* Hack -- Handle stuff */
		handle_stuff(p_ptr);

		tb = textblock_new();
		lore_title(tb, r_ptr);
		textblock_append(tb, " [(r)ecall, ESC]\n"); /* Line break is needed for proper display */
		textui_textblock_place(tb, SCREEN_REGION, NULL);
		textblock_free(tb);

		/* Interact */
		while (1)
		{
			/* Ignore keys during recall presentation, otherwise, the 'r' key acts like a toggle and instead of a one-off command */
			if (recall)
				lore_show_interactive(r_ptr, l_ptr);
			else
				query = inkey();

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



/*
 * Display the main-screen monster list.
 */
void do_cmd_monlist(void)
{
	/* Save the screen and display the list */
	screen_save();

    monster_list_show_interactive(Term->hgt, Term->wid);

	/* Return */
	screen_load();
}


/*
 * Display the main-screen item list.
 */
void do_cmd_itemlist(void)
{
	/* Save the screen and display the list */
	screen_save();

    object_list_show_interactive(Term->hgt, Term->wid);

	/* Return */
	screen_load();
}
