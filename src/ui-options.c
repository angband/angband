/**
 * \file ui-options.c
 * \brief Text UI options handling code (everything accessible from '=')
 *
 * Copyright (c) 1997-2000 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 2007 Pete Mack
 * Copyright (c) 2010 Andi Sidwell
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
#include "cmds.h"
#include "game-input.h"
#include "init.h"
#include "obj-desc.h"
#include "obj-ignore.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-calcs.h"
#include "ui-display.h"
#include "ui-help.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-knowledge.h"
#include "ui-menu.h"
#include "ui-options.h"
#include "ui-prefs.h"
#include "ui-target.h"


/**
 * Prompt the user for a filename to save the pref file to.
 */
static bool get_pref_path(const char *what, int row, char *buf, size_t max)
{
	char ftmp[80];
	bool ok;

	screen_save();

	/* Prompt */
	prt(format("%s to a pref file", what), row, 0);
	prt("File: ", row + 2, 0);

	/* Get the filesystem-safe name and append .prf */
	player_safe_name(ftmp, sizeof(ftmp), player->full_name, true);
	my_strcat(ftmp, ".prf", sizeof(ftmp));

	/* Get a filename */
	
	if(!arg_force_name)
		ok = askfor_aux(ftmp, sizeof ftmp, NULL);
	
	else
		ok = get_check(format("Confirm writing to %s? ", ftmp));

	screen_load();

	/* Build the filename */
	if (ok)
		path_build(buf, max, ANGBAND_DIR_USER, ftmp);

	return ok;
}


static void dump_pref_file(void (*dump)(ang_file *), const char *title, int row)
{
	char buf[1024];

	/* Get filename from user */
	if (!get_pref_path(title, row, buf, sizeof(buf)))
		return;

	/* Try to save */
	if (prefs_save(buf, dump, title))
		msg("Saved %s.", strstr(title, " ") + 1);
	else
		msg("Failed to save %s.", strstr(title, " ") + 1);

	event_signal(EVENT_MESSAGE_FLUSH);

	return;
}

static void do_cmd_pref_file_hack(long row);






/**
 * ------------------------------------------------------------------------
 * Options display and setting
 * ------------------------------------------------------------------------ */


/**
 * Displays an option entry.
 */
static void option_toggle_display(struct menu *m, int oid, bool cursor,
		int row, int col, int width)
{
	byte attr = curs_attrs[CURS_KNOWN][cursor != 0];
	bool *options = menu_priv(m);

	c_prt(attr, format("%-45s: %s  (%s)", option_desc(oid),
			options[oid] ? "yes" : "no ", option_name(oid)), row, col);
}

/**
 * Handle keypresses for an option entry.
 */
static bool option_toggle_handle(struct menu *m, const ui_event *event,
		int oid)
{
	bool next = false;

	if (event->type == EVT_SELECT) {
		/* Hack -- birth options can not be toggled after birth */
		/* At birth, m->flags == MN_DBL_TAP. */
		/* After birth, m->flags == MN_NO_TAGS */
		if (!((option_type(oid) == OP_BIRTH) && (m->flags == MN_NO_TAGS))) {
			option_set(option_name(oid), !player->opts.opt[oid]);
		}
	} else if (event->type == EVT_KBRD) {
		if (event->key.code == 'y' || event->key.code == 'Y') {
			option_set(option_name(oid), true);
			next = true;
		} else if (event->key.code == 'n' || event->key.code == 'N') {
			option_set(option_name(oid), false);
			next = true;
		} else if (event->key.code == 't' || event->key.code == 'T') {
			option_set(option_name(oid), !player->opts.opt[oid]);
		} else if (event->key.code == 's' || event->key.code == 'S') {
			char dummy;

			screen_save();
			if (options_save_custom_birth(&player->opts)) {
				get_com("Successfully saved.  Press any key to continue.", &dummy);
			} else {
				get_com("Save failed.  Press any key to continue.", &dummy);
			}
			screen_load();
		/* Only allow restore from custom defaults at birth. */
		} else if ((event->key.code == 'r' || event->key.code == 'R') &&
				m->flags == MN_DBL_TAP) {
			screen_save();
			if (options_restore_custom_birth(&player->opts)) {
				screen_load();
				menu_refresh(m, false);
			} else {
				char dummy;

				get_com("Restore failed.  Press any key to continue.", &dummy);
				screen_load();
			}
		/* Only allow reset to maintainer's defaults at birth. */
		} else if ((event->key.code == 'm' || event->key.code == 'M') &&
				m->flags == MN_DBL_TAP) {
			options_reset_birth(&player->opts);
			menu_refresh(m, false);
		} else if (event->key.code == '?') {
			screen_save();
			show_file(format("option.txt#%s", option_name(oid)), NULL, 0, 0);
			screen_load();
		} else {
			return false;
		}
	} else {
		return false;
	}

	if (next) {
		m->cursor++;
		m->cursor = (m->cursor + m->filter_count) % m->filter_count;
	}

	return true;
}

/**
 * Toggle option menu display and handling functions
 */
static const menu_iter option_toggle_iter = {
	NULL,
	NULL,
	option_toggle_display,
	option_toggle_handle,
	NULL
};


/**
 * Interact with some options
 */
static void option_toggle_menu(const char *name, int page)
{
	int i;
	
	struct menu *m = menu_new(MN_SKIN_SCROLL, &option_toggle_iter);

	/* for all menus */
	m->prompt = "Set option (y/n/t), '?' for information";
	m->cmd_keys = "?YyNnTt";
	m->selections = "abcdefghijklmopqrsuvwxz";
	m->flags = MN_DBL_TAP;

	/* We add 10 onto the page amount to indicate we're at birth */
	if (page == OPT_PAGE_BIRTH) {
		m->prompt = "You can only modify these options at character birth. '?' for information";
		m->cmd_keys = "?";
		m->flags = MN_NO_TAGS;
	} else if (page == OPT_PAGE_BIRTH + 10) {
		m->prompt = "Set option (y/n/t), 's' to save, 'r' to restore, 'm' to reset, '?' for help";
		m->cmd_keys = "?YyNnTtSsRrMm";
		page -= 10;
	}

	/* for this particular menu */
	m->title = name;

	/* Find the number of valid entries */
	for (i = 0; i < OPT_PAGE_PER; i++) {
		if (option_page[page][i] == OPT_none)
			break;
	}

	/* Set the data to the player's options */
	menu_setpriv(m, OPT_MAX, &player->opts.opt);
	menu_set_filter(m, option_page[page], i);
	menu_layout(m, &SCREEN_REGION);

	/* Run the menu */
	screen_save();

	clear_from(0);
	menu_select(m, 0, false);

	screen_load();

	mem_free(m);
}

/**
 * Edit birth options.
 */
void do_cmd_options_birth(void)
{
	option_toggle_menu("Birth options", OPT_PAGE_BIRTH + 10);
}


/**
 * Modify the "window" options
 */
static void do_cmd_options_win(const char *name, int row)
{
	int i, j, d;
	int y = 0;
	int x = 0;
	ui_event ke;
	u32b new_flags[ANGBAND_TERM_MAX];

	/* Set new flags to the old values */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
		new_flags[j] = window_flag[j];

	/* Clear screen */
	screen_save();
	clear_from(0);

	/* Interact */
	while (1) {
		/* Prompt */
		prt("Window flags (<dir> to move, 't'/Enter to toggle, or ESC)", 0, 0);

		/* Display the windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++) {
			byte a = COLOUR_WHITE;

			const char *s = angband_term_name[j];

			/* Use color */
			if (j == x) a = COLOUR_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < PW_MAX_FLAGS; i++) {
			byte a = COLOUR_WHITE;

			const char *str = window_flag_desc[i];

			/* Use color */
			if (i == y) a = COLOUR_L_BLUE;

			/* Unused option */
			if (!str) str = "(Unused option)";

			/* Flag name */
			Term_putstr(0, i + 5, -1, a, str);

			/* Display the windows */
			for (j = 0; j < ANGBAND_TERM_MAX; j++) {
				char c = '.';

				a = COLOUR_WHITE;

				/* Use color */
				if ((i == y) && (j == x)) a = COLOUR_L_BLUE;

				/* Active flag */
				if (new_flags[j] & (1L << i)) c = 'X';

				/* Flag value */
				Term_putch(35 + j * 5, i + 5, a, c);
			}
		}

		/* Place Cursor */
		Term_gotoxy(35 + x * 5, y + 5);

		/* Get key */
		ke = inkey_ex();

		/* Mouse or keyboard interaction */
		if (ke.type == EVT_MOUSE) {
			int choicey = ke.mouse.y - 5;
			int choicex = (ke.mouse.x - 35)/5;

			if (ke.mouse.button == 2)
				break;

			if ((choicey >= 0) && (choicey < PW_MAX_FLAGS)
				&& (choicex > 0) && (choicex < ANGBAND_TERM_MAX)
				&& !(ke.mouse.x % 5)) {
				if ((choicey == y) && (choicex == x)) {
					/* Toggle flag (off) */
					if (new_flags[x] & (1L << y))
						new_flags[x] &= ~(1L << y);
					/* Toggle flag (on) */
					else
						new_flags[x] |= (1L << y);
				} else {
					y = choicey;
					x = (ke.mouse.x - 35)/5;
				}
			}
		} else if (ke.type == EVT_KBRD) {
			if (ke.key.code == ESCAPE || ke.key.code == 'q')
				break;

			/* Toggle */
			else if (ke.key.code == '5' || ke.key.code == 't' ||
					ke.key.code == KC_ENTER) {
				/* Hack -- ignore the main window */
				if (x == 0)
					bell("Cannot set main window flags!");

				/* Toggle flag (off) */
				else if (new_flags[x] & (1L << y))
					new_flags[x] &= ~(1L << y);

				/* Toggle flag (on) */
				else
					new_flags[x] |= (1L << y);

				/* Continue */
				continue;
			}

			/* Extract direction */
			d = target_dir(ke.key);

			/* Move */
			if (d != 0) {
				x = (x + ddx[d] + 8) % ANGBAND_TERM_MAX;
				y = (y + ddy[d] + 16) % PW_MAX_FLAGS;
			}
		}
	}

	/* Notice changes */
	subwindows_set_flags(new_flags, ANGBAND_TERM_MAX);

	screen_load();
}



/**
 * ------------------------------------------------------------------------
 * Interact with keymaps
 * ------------------------------------------------------------------------ */

/**
 * Current (or recent) keymap action
 */
static struct keypress keymap_buffer[KEYMAP_ACTION_MAX + 1];


/**
 * Ask for, and display, a keymap trigger.
 *
 * Returns the trigger input.
 *
 * Note that both "event_signal(EVENT_INPUT_FLUSH)" calls are extremely
 * important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static struct keypress keymap_get_trigger(void)
{
	char tmp[80];
	struct keypress buf[2] = { KEYPRESS_NULL, KEYPRESS_NULL };

	/* Flush */
	event_signal(EVENT_INPUT_FLUSH);

	/* Get a key */
	buf[0] = inkey();

	/* Convert to ascii */
	keypress_to_text(tmp, sizeof(tmp), buf, false);

	/* Hack -- display the trigger */
	Term_addstr(-1, COLOUR_WHITE, tmp);

	/* Flush */
	event_signal(EVENT_INPUT_FLUSH);

	/* Return trigger */
	return buf[0];
}


/**
 * Keymap menu action functions
 */

static void ui_keymap_pref_load(const char *title, int row)
{
	do_cmd_pref_file_hack(16);
}

static void ui_keymap_pref_append(const char *title, int row)
{
	dump_pref_file(keymap_dump, "Dump keymaps", 13);
}

static void ui_keymap_query(const char *title, int row)
{
	char tmp[1024];
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	struct keypress c;
	const struct keypress *act;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);
	
	/* Get a keymap trigger & mapping */
	c = keymap_get_trigger();
	act = keymap_find(mode, c);
	
	/* Keymap found? */
	if (!act) {
		/* Prompt */
		prt("No keymap with that trigger.  Press any key to continue.", 16, 0);
		inkey();
	} else {
		/* Analyze the current action */
		keypress_to_text(tmp, sizeof(tmp), act, false);
	
		/* Display the current action */
		prt("Found: ", 15, 0);
		Term_addstr(-1, COLOUR_WHITE, tmp);

		prt("Press any key to continue.", 17, 0);
		inkey();
	}
}

static void ui_keymap_create(const char *title, int row)
{
	bool done = false;
	size_t n = 0;

	struct keypress c;
	char tmp[1024];
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);

	c = keymap_get_trigger();
	if (c.code == '$') {
		c_prt(COLOUR_L_RED, "The '$' key is reserved.", 16, 2);
		prt("Press any key to continue.", 18, 0);
		inkey();
		return;
	}

	/* Get an encoded action, with a default response */
	while (!done) {
		struct keypress kp = {EVT_NONE, 0, 0};

		int color = COLOUR_WHITE;
		if (n == 0) color = COLOUR_YELLOW;
		if (n == KEYMAP_ACTION_MAX) color = COLOUR_L_RED;

		keypress_to_text(tmp, sizeof(tmp), keymap_buffer, false);
		c_prt(color, format("Action: %s", tmp), 15, 0);

		c_prt(COLOUR_L_BLUE, "  Press '$' when finished.", 17, 0);
		c_prt(COLOUR_L_BLUE, "  Use 'CTRL-U' to reset.", 18, 0);
		c_prt(COLOUR_L_BLUE, format("(Maximum keymap length is %d keys.)",
									KEYMAP_ACTION_MAX), 19, 0);

		kp = inkey();

		if (kp.code == '$') {
			done = true;
			continue;
		}

		switch (kp.code) {
			case KC_DELETE:
			case KC_BACKSPACE: {
				if (n > 0) {
					n -= 1;
				    keymap_buffer[n].type = 0;
					keymap_buffer[n].code = 0;
					keymap_buffer[n].mods = 0;
				}
				break;
			}

			case KTRL('U'): {
				memset(keymap_buffer, 0, sizeof keymap_buffer);
				n = 0;
				break;
			}

			default: {
				if (n == KEYMAP_ACTION_MAX) continue;

				if (n == 0) {
					memset(keymap_buffer, 0, sizeof keymap_buffer);
				}
				keymap_buffer[n++] = kp;
				break;
			}
		}
	}

	if (c.code && get_check("Save this keymap? ")) {
		keymap_add(mode, c, keymap_buffer, true);
		prt("Keymap added.  Press any key to continue.", 17, 0);
		inkey();
	}
}

static void ui_keymap_remove(const char *title, int row)
{
	struct keypress c;
	int mode = OPT(player, rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);

	c = keymap_get_trigger();

	if (keymap_remove(mode, c))
		prt("Removed.", 16, 0);
	else
		prt("No keymap to remove!", 16, 0);

	/* Prompt */
	prt("Press any key to continue.", 17, 0);
	inkey();
}

static void keymap_browse_hook(int oid, void *db, const region *loc)
{
	char tmp[1024];

	event_signal(EVENT_MESSAGE_FLUSH);

	clear_from(13);

	/* Show current action */
	prt("Current action (if any) shown below:", 13, 0);
	keypress_to_text(tmp, sizeof(tmp), keymap_buffer, false);
	prt(tmp, 14, 0);
}

static struct menu *keymap_menu;
static menu_action keymap_actions[] =
{
	{ 0, 0, "Load a user pref file",    ui_keymap_pref_load },
	{ 0, 0, "Save keymaps to file",     ui_keymap_pref_append },
	{ 0, 0, "Query a keymap",           ui_keymap_query },
	{ 0, 0, "Create a keymap",          ui_keymap_create },
	{ 0, 0, "Remove a keymap",          ui_keymap_remove },
};

static void do_cmd_keymaps(const char *title, int row)
{
	region loc = {0, 0, 0, 12};

	screen_save();
	clear_from(0);

	if (!keymap_menu) {
		keymap_menu = menu_new_action(keymap_actions,
				N_ELEMENTS(keymap_actions));
	
		keymap_menu->title = title;
		keymap_menu->selections = lower_case;
		keymap_menu->browse_hook = keymap_browse_hook;
	}

	menu_layout(keymap_menu, &loc);
	menu_select(keymap_menu, 0, false);

	screen_load();
}



/**
 * ------------------------------------------------------------------------
 * Interact with visuals
 * ------------------------------------------------------------------------ */

static void visuals_pref_load(const char *title, int row)
{
	do_cmd_pref_file_hack(15);
}

static void visuals_dump_monsters(const char *title, int row)
{
	dump_pref_file(dump_monsters, title, 15);
}

static void visuals_dump_objects(const char *title, int row)
{
	dump_pref_file(dump_objects, title, 15);
}

static void visuals_dump_features(const char *title, int row)
{
	dump_pref_file(dump_features, title, 15);
}

static void visuals_dump_flavors(const char *title, int row)
{
	dump_pref_file(dump_flavors, title, 15);
}

static void visuals_reset(const char *title, int row)
{
	/* Reset */
	reset_visuals(true);

	/* Message */
	prt("", 0, 0);
	msg("Visual attr/char tables reset.");
	event_signal(EVENT_MESSAGE_FLUSH);
}


static struct menu *visual_menu;
static menu_action visual_menu_items [] =
{
	{ 0, 0, "Load a user pref file",   visuals_pref_load },
	{ 0, 0, "Save monster attr/chars", visuals_dump_monsters },
	{ 0, 0, "Save object attr/chars",  visuals_dump_objects },
	{ 0, 0, "Save feature attr/chars", visuals_dump_features },
	{ 0, 0, "Save flavor attr/chars",  visuals_dump_flavors },
	{ 0, 0, "Reset visuals",           visuals_reset },
};


static void visuals_browse_hook(int oid, void *db, const region *loc)
{
	event_signal(EVENT_MESSAGE_FLUSH);
	clear_from(1);
}


/**
 * Interact with "visuals"
 */
static void do_cmd_visuals(const char *title, int row)
{
	screen_save();
	clear_from(0);

	if (!visual_menu)
	{
		visual_menu = menu_new_action(visual_menu_items,
				N_ELEMENTS(visual_menu_items));

		visual_menu->title = title;
		visual_menu->selections = lower_case;
		visual_menu->browse_hook = visuals_browse_hook;
		visual_menu->header = "To edit visuals, use the knowledge menu";
	}

	menu_layout(visual_menu, &SCREEN_REGION);
	menu_select(visual_menu, 0, false);

	screen_load();
}


/**
 * ------------------------------------------------------------------------
 * Interact with colours
 * ------------------------------------------------------------------------ */

static void colors_pref_load(const char *title, int row)
{
	/* Ask for and load a user pref file */
	do_cmd_pref_file_hack(8);
	
	/* XXX should probably be a cleaner way to tell UI about
	 * colour changes - how about doing this in the pref file
	 * loading code too? */
	Term_xtra(TERM_XTRA_REACT, 0);
	Term_redraw();
}

static void colors_pref_dump(const char *title, int row)
{
	dump_pref_file(dump_colors, title, 15);
}

static void colors_modify(const char *title, int row)
{
	int i;

	static byte a = 0;

	/* Prompt */
	prt("Command: Modify colors", 8, 0);

	/* Hack -- query until done */
	while (1) {
		const char *name;
		char index;

		struct keypress cx;

		/* Clear */
		clear_from(10);

		/* Exhibit the normal colors */
		for (i = 0; i < BASIC_COLORS; i++) {
			/* Exhibit this color */
			Term_putstr(i*3, 20, -1, a, "##");

			/* Exhibit character letter */
			Term_putstr(i*3, 21, -1, (byte)i,
						format(" %c", color_table[i].index_char));

			/* Exhibit all colors */
			Term_putstr(i*3, 22, -1, (byte)i, format("%2d", i));
		}

		/* Describe the color */
		name = ((a < BASIC_COLORS) ? color_table[a].name : "undefined");
		index = ((a < BASIC_COLORS) ? color_table[a].index_char : '?');

		/* Describe the color */
		Term_putstr(5, 10, -1, COLOUR_WHITE,
					format("Color = %d, Name = %s, Index = %c",
						   a, name, index));

		/* Label the Current values */
		Term_putstr(5, 12, -1, COLOUR_WHITE,
				format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
				   angband_color_table[a][0],
				   angband_color_table[a][1],
				   angband_color_table[a][2],
				   angband_color_table[a][3]));

		/* Prompt */
		Term_putstr(0, 14, -1, COLOUR_WHITE,
				"Command (n/N/k/K/r/R/g/G/b/B): ");

		/* Get a command */
		cx = inkey();

		/* All done */
		if (cx.code == ESCAPE) break;

		/* Analyze */
		if (cx.code == 'n')
			a = (byte)(a + 1);
		if (cx.code == 'N')
			a = (byte)(a - 1);
		if (cx.code == 'k')
			angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
		if (cx.code == 'K')
			angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
		if (cx.code == 'r')
			angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
		if (cx.code == 'R')
			angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
		if (cx.code == 'g')
			angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
		if (cx.code == 'G')
			angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
		if (cx.code == 'b')
			angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
		if (cx.code == 'B')
			angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

		/* Hack -- react to changes */
		Term_xtra(TERM_XTRA_REACT, 0);

		/* Hack -- redraw */
		Term_redraw();
	}
}

static void colors_browse_hook(int oid, void *db, const region *loc)
{
	event_signal(EVENT_MESSAGE_FLUSH);
	clear_from(1);
}


static struct menu *color_menu;
static menu_action color_events [] =
{
	{ 0, 0, "Load a user pref file", colors_pref_load },
	{ 0, 0, "Dump colors",           colors_pref_dump },
	{ 0, 0, "Modify colors",         colors_modify }
};

/**
 * Interact with "colors"
 */
static void do_cmd_colors(const char *title, int row)
{
	screen_save();
	clear_from(0);

	if (!color_menu)
	{
		color_menu = menu_new_action(color_events,
			N_ELEMENTS(color_events));

		color_menu->title = title;
		color_menu->selections = lower_case;
		color_menu->browse_hook = colors_browse_hook;
	}

	menu_layout(color_menu, &SCREEN_REGION);
	menu_select(color_menu, 0, false);

	screen_load();
}


/**
 * ------------------------------------------------------------------------
 * Non-complex menu actions
 * ------------------------------------------------------------------------ */

static bool askfor_aux_numbers(char *buf, size_t buflen, size_t *curs, size_t *len, struct keypress keypress, bool firsttime)
{
	switch (keypress.code)
	{
		case ESCAPE:
		case KC_ENTER:
		case ARROW_LEFT:
		case ARROW_RIGHT:
		case KC_DELETE:
		case KC_BACKSPACE:
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return askfor_aux_keypress(buf, buflen, curs, len, keypress,
									   firsttime);
	}

	return false;
}


/**
 * Set base delay factor
 */
static void do_cmd_delay(const char *name, int row)
{
	char tmp[4] = "";
	int msec = player->opts.delay_factor;

	strnfmt(tmp, sizeof(tmp), "%i", player->opts.delay_factor);

	screen_save();

	/* Prompt */
	prt("Command: Base Delay Factor", 20, 0);

	prt(format("Current base delay factor: %d msec",
			   player->opts.delay_factor, msec), 22, 0);
	prt("New base delay factor (0-255): ", 21, 0);

	/* Ask for a numeric value */
	if (askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers)) {
		u16b val = (u16b) strtoul(tmp, NULL, 0);
		player->opts.delay_factor = MIN(val, 255);
	}

	screen_load();
}

/**
 * Set sidebar mode
 */
static void do_cmd_sidebar_mode(const char *name, int row)
{
	char tmp[20] = "";	
	const char *names[SIDEBAR_MAX] = {"Left", "Top", "None"};
	struct keypress cx = KEYPRESS_NULL;

	screen_save();

	while (true) {	

		// Get the name
		my_strcpy(tmp, names[SIDEBAR_MODE % SIDEBAR_MAX], sizeof(tmp));

		/* Prompt */
		prt("Command: Sidebar Mode", 20, 0);

		prt("ESC: go back, other: cycle", 22, 0);

		prt(format("Current mode: %s", tmp), 21, 0);

		/* Get a command */
		cx = inkey();

		/* All done */
		if (cx.code == ESCAPE) break;

		// Cycle
		SIDEBAR_MODE = (SIDEBAR_MODE + 1) % SIDEBAR_MAX;
	}

	screen_load();
}


/**
 * Set hitpoint warning level
 */
static void do_cmd_hp_warn(const char *name, int row)
{
	bool res;
	char tmp[4] = "";
	byte warn;

	strnfmt(tmp, sizeof(tmp), "%i", player->opts.hitpoint_warn);

	screen_save();

	/* Prompt */
	prt("Command: Hitpoint Warning", 20, 0);

	prt(format("Current hitpoint warning: %d (%d%%)",
			   player->opts.hitpoint_warn, player->opts.hitpoint_warn * 10), 22, 0);
	prt("New hitpoint warning (0-9): ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res) {
		warn = (byte) strtoul(tmp, NULL, 0);
		
		/* Reset nonsensical warnings */
		if (warn > 9)
			warn = 0;

		player->opts.hitpoint_warn = warn;
	}

	screen_load();
}


/**
 * Set "lazy-movement" delay
 */
static void do_cmd_lazymove_delay(const char *name, int row)
{
	bool res;
	char tmp[4] = "";

	strnfmt(tmp, sizeof(tmp), "%i", player->opts.lazymove_delay);

	screen_save();

	/* Prompt */
	prt("Command: Movement Delay Factor", 20, 0);

	prt(format("Current movement delay: %d (%d msec)",
			   player->opts.lazymove_delay, player->opts.lazymove_delay * 10), 22, 0);
	prt("New movement delay: ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res) {
		u16b delay = (u16b)strtoul(tmp, NULL, 0);
		player->opts.lazymove_delay = MIN(delay, 255);
	}

	screen_load();
}



/**
 * Ask for a "user pref file" and process it.
 *
 * This function should only be used by standard interaction commands,
 * in which a standard "Command:" prompt is present on the given row.
 *
 * Allow absolute file names?  XXX XXX XXX
 */
static void do_cmd_pref_file_hack(long row)
{
	char ftmp[80];
	bool ok;

	screen_save();

	/* Prompt */
	prt("Command: Load a user pref file", row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Get the filesystem-safe name and append .prf */
	player_safe_name(ftmp, sizeof(ftmp), player->full_name, true);
	my_strcat(ftmp, ".prf", sizeof(ftmp));

	if(!arg_force_name)
		ok = askfor_aux(ftmp, sizeof ftmp, NULL);
	else
		ok = get_check(format("Confirm loading %s? ", ftmp));
	
	/* Ask for a file (or cancel) */
	if(ok) {
		/* Process the given filename */
		if (process_pref_file(ftmp, false, true) == false) {
			/* Mention failure */
			prt("", 0, 0);
			msg("Failed to load '%s'!", ftmp);
		} else {
			/* Mention success */
			prt("", 0, 0);
			msg("Loaded '%s'.", ftmp);
		}
	}

	screen_load();
}

 
 
/**
 * Write options to a file.
 */
static void do_dump_options(const char *title, int row) {
	dump_pref_file(option_dump, "Dump window settings", 20);
}

/**
 * Write autoinscriptions to a file.
 */
static void do_dump_autoinsc(const char *title, int row) {
	dump_pref_file(dump_autoinscriptions, "Dump autoinscriptions", 20);
}

/**
 * Write character screen customizations to a file.
 */
static void do_dump_charscreen_opt(const char *title, int row) {
	dump_pref_file(dump_ui_entry_renderers, "Dump char screen options", 20);
}

/**
 * Load a pref file.
 */
static void options_load_pref_file(const char *n, int row)
{
	do_cmd_pref_file_hack(20);
}



/**
 * ------------------------------------------------------------------------
 * Ego item ignore menu
 * ------------------------------------------------------------------------ */

#define EGO_MENU_HELPTEXT \
"{light green}Movement keys{/} scroll the list\n{light red}ESC{/} returns to the previous menu\n{light blue}Enter{/} toggles the current setting."

/**
 * Skip common prefixes in ego-item names.
 */
static const char *strip_ego_name(const char *name)
{
	if (prefix(name, "of the "))
		return name + 7;
	if (prefix(name, "of "))
		return name + 3;
	return name;
}


/**
 * Display an ego-item type on the screen.
 */
int ego_item_name(char *buf, size_t buf_size, struct ego_desc *desc)
{
	size_t i;
	int end;
	size_t prefix_size;
	const char *long_name;

	struct ego_item *ego = &e_info[desc->e_idx];

	/* Find the ignore type */
	for (i = 0; i < N_ELEMENTS(quality_choices); i++)
		if (desc->itype == i) break;

	if (i == N_ELEMENTS(quality_choices)) return 0;

	/* Initialize the buffer */
	end = my_strcat(buf, "[ ] ", buf_size);

	/* Append the name */
	end += my_strcat(buf, quality_choices[i].name, buf_size);

	/* Append an extra space */
	end += my_strcat(buf, " ", buf_size);

	/* Get the full ego-item name */
	long_name = ego->name;

	/* Get the length of the common prefix, if any */
	prefix_size = (desc->short_name - long_name);

	/* Found a prefix? */
	if (prefix_size > 0) {
		char prefix[100];

		/* Get a copy of the prefix */
		my_strcpy(prefix, long_name, prefix_size + 1);

		/* Append the prefix */
		end += my_strcat(buf, prefix, buf_size);
	}
	/* Set the name to the right length */
	return end;
}

/**
 * Utility function used for sorting an array of ego-item indices by
 * ego-item name.
 */
static int ego_comp_func(const void *a_ptr, const void *b_ptr)
{
	const struct ego_desc *a = a_ptr;
	const struct ego_desc *b = b_ptr;

	/* Note the removal of common prefixes */
	return (strcmp(a->short_name, b->short_name));
}

/**
 * Display an entry on the sval menu
 */
static void ego_display(struct menu * menu, int oid, bool cursor, int row,
						int col, int width)
{
	char buf[80] = "";
	struct ego_desc *choice = (struct ego_desc *) menu->menu_data;
	bool ignored = ego_is_ignored(choice[oid].e_idx, choice[oid].itype);

	byte attr = (cursor ? COLOUR_L_BLUE : COLOUR_WHITE);
	byte sq_attr = (ignored ? COLOUR_L_RED : COLOUR_L_GREEN);

	/* Acquire the "name" of object "i" */
	(void) ego_item_name(buf, sizeof(buf), &choice[oid]);

	/* Print it */
	c_put_str(attr, format("%s", buf), row, col);

	/* Show ignore mark, if any */
	if (ignored)
		c_put_str(COLOUR_L_RED, "*", row, col + 1);

	/* Show the stripped ego-item name using another colour */
	c_put_str(sq_attr, choice[oid].short_name, row, col + strlen(buf));
}

/**
 * Deal with events on the sval menu
 */
static bool ego_action(struct menu * menu, const ui_event * event, int oid)
{
	struct ego_desc *choice = menu->menu_data;

	/* Toggle */
	if (event->type == EVT_SELECT) {
		ego_ignore_toggle(choice[oid].e_idx, choice[oid].itype);

		return true;
	}

	return false;
}

/**
 * Display list of ego items to be ignored.
 */
static void ego_menu(const char *unused, int also_unused)
{
	int max_num = 0;
	struct ego_item *ego;
	struct ego_desc *choice;

	struct menu menu;
	menu_iter menu_f = { 0, 0, ego_display, ego_action, 0 };
	region area = { 1, 5, -1, -1 };
	int cursor = 0;

	int i;

	/* Create the array */
	choice = mem_zalloc(z_info->e_max * ITYPE_MAX * sizeof(struct ego_desc));

	/* Get the valid ego-items */
	for (i = 0; i < z_info->e_max; i++) {
		int itype;
		ego = &e_info[i];

		/* Only valid known ego-items allowed */
		if (!ego->name || !ego->everseen)
			continue;

		/* Find appropriate ignore types */
		for (itype = ITYPE_NONE + 1; itype < ITYPE_MAX; itype++)
			if (ego_has_ignore_type(ego, itype)) {

				/* Fill in the details */
				choice[max_num].e_idx = i;
				choice[max_num].itype = itype;
				choice[max_num].short_name = strip_ego_name(ego->name);

				++max_num;
			}
	}

	/* Quickly sort the array by ego-item name */
	qsort(choice, max_num, sizeof(choice[0]), ego_comp_func);

	/* Return here if there are no objects */
	if (!max_num) {
		mem_free(choice);
		return;
	}


	/* Save the screen and clear it */
	screen_save();
	clear_from(0);

	/* Help text */
	prt("Ego item ignore menu", 0, 0);

	/* Output to the screen */
	text_out_hook = text_out_to_screen;

	/* Indent output */
	text_out_indent = 1;
	text_out_wrap = 79;
	Term_gotoxy(1, 1);

	/* Display some helpful information */
	text_out_e(EGO_MENU_HELPTEXT);

	text_out_indent = 0;

	/* Set up the menu */
	memset(&menu, 0, sizeof(menu));
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu_setpriv(&menu, max_num, choice);
	menu_layout(&menu, &area);

	/* Select an entry */
	(void) menu_select(&menu, cursor, false);

	/* Free memory */
	mem_free(choice);

	/* Load screen */
	screen_load();

	return;
}


/**
 * ------------------------------------------------------------------------
 * Quality ignore menu
 * ------------------------------------------------------------------------ */

/**
 * Menu struct for differentiating aware from unaware ignore
 */
typedef struct
{
	struct object_kind *kind;
	bool aware;
} ignore_choice;

/**
 * Ordering function for ignore choices.
 * Aware comes before unaware, and then sort alphabetically.
 */
static int cmp_ignore(const void *a, const void *b)
{
	char bufa[80];
	char bufb[80];
	const ignore_choice *x = a;
	const ignore_choice *y = b;

	if (!x->aware && y->aware)
		return 1;
	if (x->aware && !y->aware)
		return -1;

	object_kind_name(bufa, sizeof(bufa), x->kind, x->aware);
	object_kind_name(bufb, sizeof(bufb), y->kind, y->aware);

	return strcmp(bufa, bufb);
}

/**
 * Determine if an item is a valid choice
 */
int quality_validity(struct menu *menu, int oid)
{
	return oid ? 1 : 0;
}

/**
 * Display an entry in the menu.
 */
static void quality_display(struct menu *menu, int oid, bool cursor, int row,
							int col, int width)
{
	/* Note: the order of the values in quality_choices do not align with the
	 * ignore_type_t enum order. - fix? NRM*/
	const char *name = quality_choices[oid].name;

	byte level = ignore_level[oid];
	const char *level_name = quality_values[level].name;

	byte attr = (cursor ? COLOUR_L_BLUE : COLOUR_WHITE);

	if (oid)
		c_put_str(attr, format("%-30s : %s", name, level_name), row, col);
}


/**
 * Display the quality ignore subtypes.
 */
static void quality_subdisplay(struct menu *menu, int oid, bool cursor, int row,
							   int col, int width)
{
	const char *name = quality_values[oid].name;
	byte attr = (cursor ? COLOUR_L_BLUE : COLOUR_WHITE);

	c_put_str(attr, name, row, col);
}


/**
 * Handle keypresses.
 */
static bool quality_action(struct menu *m, const ui_event *event, int oid)
{
	struct menu menu;
	menu_iter menu_f = { NULL, NULL, quality_subdisplay, NULL, NULL };
	region area = { 37, 2, 29, IGNORE_MAX };
	ui_event evt;
	int count;

	/* Display at the right point */
	area.row += oid;

	/* Save */
	screen_save();

	/* Work out how many options we have */
	count = IGNORE_MAX;
	if ((oid == ITYPE_RING) || (oid == ITYPE_AMULET))
		count = area.page_rows = IGNORE_BAD + 1;

	/* Run menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu_setpriv(&menu, count, quality_values);

	/* Stop menus from going off the bottom of the screen */
	if (area.row + menu.count > Term->hgt - 1)
		area.row += Term->hgt - 1 - area.row - menu.count;

	menu_layout(&menu, &area);

	window_make(area.col - 2, area.row - 1, area.col + area.width + 2,
				area.row + area.page_rows);

	evt = menu_select(&menu, 0, true);

	/* Set the new value appropriately */
	if (evt.type == EVT_SELECT)
		ignore_level[oid] = menu.cursor;

	/* Load and finish */
	screen_load();
	return true;
}

/**
 * Display quality ignore menu.
 */
static void quality_menu(void *unused, const char *also_unused)
{
	struct menu menu;
	menu_iter menu_f = { NULL, quality_validity, quality_display,
						 quality_action, NULL };
	region area = { 0, 0, 0, 0 };

	/* Save screen */
	screen_save();
	clear_from(0);

	/* Set up the menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu.title = "Quality ignore menu";
	menu_setpriv(&menu, ITYPE_MAX, quality_values);
	menu_layout(&menu, &area);

	/* Select an entry */
	menu_select(&menu, 0, false);

	/* Load screen */
	screen_load();
	return;
}



/**
 * ------------------------------------------------------------------------
 * Sval ignore menu
 * ------------------------------------------------------------------------ */

/**
 * Structure to describe tval/description pairings.
 */
typedef struct
{
	int tval;
	const char *desc;
} tval_desc;

/**
 * Categories for sval-dependent ignore.
 */
static tval_desc sval_dependent[] =
{
	{ TV_STAFF,			"Staffs" },
	{ TV_WAND,			"Wands" },
	{ TV_ROD,			"Rods" },
	{ TV_SCROLL,		"Scrolls" },
	{ TV_POTION,		"Potions" },
	{ TV_RING,			"Rings" },
	{ TV_AMULET,		"Amulets" },
	{ TV_FOOD,			"Food" },
	{ TV_MUSHROOM,		"Mushrooms" },
	{ TV_MAGIC_BOOK,	"Magic books" },
	{ TV_PRAYER_BOOK,	"Prayer books" },
	{ TV_NATURE_BOOK,	"Nature books" },
	{ TV_SHADOW_BOOK,	"Shadow books" },
	{ TV_OTHER_BOOK,	"Mystery books" },
	{ TV_LIGHT,			"Lights" },
	{ TV_FLASK,			"Flasks of oil" },
	{ TV_GOLD,			"Money" },
};


/**
 * Determines whether a tval is eligible for sval-ignore.
 */
bool ignore_tval(int tval)
{
	size_t i;

	/* Only ignore if the tval's allowed */
	for (i = 0; i < N_ELEMENTS(sval_dependent); i++) {
		if (kb_info[tval].num_svals == 0) continue;
		if (tval == sval_dependent[i].tval)
			return true;
	}

	return false;
}


/**
 * Display an entry on the sval menu
 */
static void ignore_sval_menu_display(struct menu *menu, int oid, bool cursor,
									 int row, int col, int width)
{
	char buf[80];
	const ignore_choice *choice = menu_priv(menu);

	struct object_kind *kind = choice[oid].kind;
	bool aware = choice[oid].aware;

	byte attr = curs_attrs[(int)aware][0 != cursor];

	/* Acquire the "name" of object "i" */
	object_kind_name(buf, sizeof(buf), kind, aware);

	/* Print it */
	c_put_str(attr, format("[ ] %s", buf), row, col);
	if ((aware && (kind->ignore & IGNORE_IF_AWARE)) ||
			(!aware && (kind->ignore & IGNORE_IF_UNAWARE)))
		c_put_str(COLOUR_L_RED, "*", row, col + 1);
}


/**
 * Deal with events on the sval menu
 */
static bool ignore_sval_menu_action(struct menu *m, const ui_event *event,
									int oid)
{
	const ignore_choice *choice = menu_priv(m);

	if (event->type == EVT_SELECT ||
			(event->type == EVT_KBRD && tolower(event->key.code) == 't')) {
		struct object_kind *kind = choice[oid].kind;

		/* Toggle the appropriate flag */
		if (choice[oid].aware)
			kind->ignore ^= IGNORE_IF_AWARE;
		else
			kind->ignore ^= IGNORE_IF_UNAWARE;

		player->upkeep->notice |= PN_IGNORE;
		return true;
	}

	return false;
}

static const menu_iter ignore_sval_menu =
{
	NULL,
	NULL,
	ignore_sval_menu_display,
	ignore_sval_menu_action,
	NULL,
};


/**
 * Collect all tvals in the big ignore_choice array
 */
static int ignore_collect_kind(int tval, ignore_choice **ch)
{
	ignore_choice *choice;
	int num = 0;

	int i;

	/* Create the array, with entries both for aware and unaware ignore */
	choice = mem_alloc(2 * z_info->k_max * sizeof *choice);

	for (i = 1; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!kind->name || kind->tval != tval)
			continue;

		if (!kind->aware) {
			/* can unaware ignore anything */
			choice[num].kind = kind;
			choice[num++].aware = false;
		}

		if ((kind->everseen && !kf_has(kind->kind_flags, KF_INSTA_ART)) || 
			tval_is_money_k(kind)) {
			/* Do not display the artifact base kinds in this list 
			 * aware ignore requires everseen 
			 * do not require awareness for aware ignore, so people can set 
			 * at game start */
			choice[num].kind = kind;
			choice[num++].aware = true;
		}
	}

	if (num == 0)
		mem_free(choice);
	else
		*ch = choice;

	return num;
}

/**
 * Display list of svals to be ignored.
 */
static bool sval_menu(int tval, const char *desc)
{
	struct menu *menu;
	region area = { 1, 2, -1, -1 };

	ignore_choice *choices;

	int n_choices = ignore_collect_kind(tval, &choices);
	if (!n_choices)
		return false;

	/* Sort by name in ignore menus except for categories of items that are
	 * aware from the start */
	switch (tval)
	{
		case TV_LIGHT:
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_NATURE_BOOK:
		case TV_SHADOW_BOOK:
		case TV_OTHER_BOOK:
		case TV_DRAG_ARMOR:
		case TV_GOLD:
			/* leave sorted by sval */
			break;

		default:
			/* sort by name */
			sort(choices, n_choices, sizeof(*choices), cmp_ignore);
	}


	/* Save the screen and clear it */
	screen_save();
	clear_from(0);

	/* Help text */
	prt(format("Ignore the following %s:", desc), 0, 0);

	/* Run menu */
	menu = menu_new(MN_SKIN_COLUMNS, &ignore_sval_menu);
	menu_setpriv(menu, n_choices, choices);
	menu->cmd_keys = "Tt";
	menu_layout(menu, &area);
	menu_set_cursor_x_offset(menu, 1); /* Place cursor in brackets. */
	menu_select(menu, 0, false);

	/* Free memory */
	mem_free(menu);
	mem_free(choices);

	/* Load screen */
	screen_load();
	return true;
}


/**
 * Returns true if there's anything to display a menu of
 */
static bool seen_tval(int tval)
{
	int i;

	for (i = 1; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!kind->name) continue;
		if (!kind->everseen) continue;
		if (kind->tval != tval) continue;

		 return true;
	}


	return false;
}


/**
 * Extra options on the "item options" menu
 */
static struct
{
	char tag;
	const char *name;
	void (*action)(); /* this is a nasty hack */
} extra_item_options[] = {
	{ 'Q', "Quality ignoring options", quality_menu },
	{ 'E', "Ego ignoring options", ego_menu},
	{ '{', "Autoinscription setup", textui_browse_object_knowledge },
};

static char tag_options_item(struct menu *menu, int oid)
{
	size_t line = (size_t) oid;

	if (line < N_ELEMENTS(sval_dependent))
		return I2A(oid);

	/* Separator - blank line. */
	if (line == N_ELEMENTS(sval_dependent))
		return 0;

	line = line - N_ELEMENTS(sval_dependent) - 1;

	if (line < N_ELEMENTS(extra_item_options))
		return extra_item_options[line].tag;

	return 0;
}

static int valid_options_item(struct menu *menu, int oid)
{
	size_t line = (size_t) oid;

	if (line < N_ELEMENTS(sval_dependent))
		return 1;

	/* Separator - blank line. */
	if (line == N_ELEMENTS(sval_dependent))
		return 0;

	line = line - N_ELEMENTS(sval_dependent) - 1;

	if (line < N_ELEMENTS(extra_item_options))
		return 1;

	return 0;
}

static void display_options_item(struct menu *menu, int oid, bool cursor,
								 int row, int col, int width)
{
	size_t line = (size_t) oid;

	/* Most of the menu is svals, with a small "extra options" section below */
	if (line < N_ELEMENTS(sval_dependent)) {
		bool known = seen_tval(sval_dependent[line].tval);
		byte attr = curs_attrs[known ? CURS_KNOWN: CURS_UNKNOWN][(int)cursor];

		c_prt(attr, sval_dependent[line].desc, row, col);
	} else {
		byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

		line = line - N_ELEMENTS(sval_dependent) - 1;

		if (line < N_ELEMENTS(extra_item_options))
			c_prt(attr, extra_item_options[line].name, row, col);
	}
}

static bool handle_options_item(struct menu *menu, const ui_event *event,
								int oid)
{
	if (event->type == EVT_SELECT) {
		if ((size_t) oid < N_ELEMENTS(sval_dependent))
		{
			sval_menu(sval_dependent[oid].tval, sval_dependent[oid].desc);
		} else {
			oid = oid - (int)N_ELEMENTS(sval_dependent) - 1;
			assert((size_t) oid < N_ELEMENTS(extra_item_options));
			extra_item_options[oid].action();
		}

		return true;
	}

	return false;
}


static const menu_iter options_item_iter =
{
	tag_options_item,
	valid_options_item,
	display_options_item,
	handle_options_item,
	NULL
};


/**
 * Display and handle the main ignoring menu.
 */
void do_cmd_options_item(const char *title, int row)
{
	struct menu menu;

	menu_init(&menu, MN_SKIN_SCROLL, &options_item_iter);
	menu_setpriv(&menu, N_ELEMENTS(sval_dependent) +
				 N_ELEMENTS(extra_item_options) + 1, NULL);

	menu.title = title;
	menu_layout(&menu, &SCREEN_REGION);

	screen_save();
	clear_from(0);
	menu_select(&menu, 0, false);
	screen_load();

	player->upkeep->notice |= PN_IGNORE;

	return;
}



/**
 * ------------------------------------------------------------------------
 * Main menu definitions and display
 * ------------------------------------------------------------------------ */

static struct menu *option_menu;
static menu_action option_actions[] = 
{
	{ 0, 'a', "User interface options", option_toggle_menu },
	{ 0, 'b', "Birth (difficulty) options", option_toggle_menu },
	{ 0, 'x', "Cheat options", option_toggle_menu },
	{ 0, 'w', "Subwindow setup", do_cmd_options_win },
	{ 0, 'i', "Item ignoring setup", do_cmd_options_item },
	{ 0, '{', "Auto-inscription setup", textui_browse_object_knowledge },
	{ 0, 0, NULL, NULL },
	{ 0, 'd', "Set base delay factor", do_cmd_delay },
	{ 0, 'h', "Set hitpoint warning", do_cmd_hp_warn },
	{ 0, 'm', "Set movement delay", do_cmd_lazymove_delay },
	{ 0, 'o', "Set sidebar mode", do_cmd_sidebar_mode },
	{ 0, 0, NULL, NULL },
	{ 0, 's', "Save subwindow setup to pref file", do_dump_options },
	{ 0, 't', "Save autoinscriptions to pref file", do_dump_autoinsc },
	{ 0, 'u', "Save char screen options to pref file", do_dump_charscreen_opt },
	{ 0, 0, NULL, NULL },
	{ 0, 'l', "Load a user pref file", options_load_pref_file },
	{ 0, 'k', "Edit keymaps (advanced)", do_cmd_keymaps },
	{ 0, 'c', "Edit colours (advanced)", do_cmd_colors },
	{ 0, 'v', "Save visuals (advanced)", do_cmd_visuals },
};


/**
 * Display the options main menu.
 */
void do_cmd_options(void)
{
	if (!option_menu) {
		/* Main option menu */
		option_menu = menu_new_action(option_actions,
				N_ELEMENTS(option_actions));

		option_menu->title = "Options Menu";
		option_menu->flags = MN_CASELESS_TAGS;
	}

	screen_save();
	clear_from(0);

	menu_layout(option_menu, &SCREEN_REGION);
	menu_select(option_menu, 0, false);

	screen_load();
}

void cleanup_options(void)
{
	if (keymap_menu) menu_free(keymap_menu);
	if (visual_menu) menu_free(visual_menu);
	if (color_menu) menu_free(color_menu);
	if (option_menu) menu_free(option_menu);
}
