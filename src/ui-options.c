/*
 * File: ui-options.c
 * Purpose: Text UI options handling code (everything accessible from '=')
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
#include "keymap.h"
#include "squelch.h"
#include "prefs.h"
#include "object/tvalsval.h"
#include "ui-menu.h"
#include "files.h"




static void dump_pref_file(void (*dump)(ang_file *), const char *title, int row)
{
	char ftmp[80];
	char buf[1024];

	screen_save();

	/* Prompt */
	prt(format("%s to a pref file", title), row, 0);
	
	/* Prompt */
	prt("File: ", row + 2, 0);
	
	/* Default filename */
	strnfmt(ftmp, sizeof ftmp, "%s.prf", op_ptr->base_name);
	
	/* Get a filename */
	if (askfor_aux(ftmp, sizeof ftmp, NULL))
	{
		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_USER, ftmp);
	
		prt("", 0, 0);
		if (prefs_save(buf, dump, title))
			msg("Dumped %s", strstr(title, " ") + 1);
		else
			msg("Failed");
	}

	screen_load();

	return;
}

static void do_cmd_pref_file_hack(long row);






/*** Options display and setting ***/



/*** Boolean option menu code ***/

/**
 * Displays an option entry.
 */
static void option_toggle_display(menu_type *m, int oid, bool cursor,
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
static bool option_toggle_handle(menu_type *m, const ui_event *event,
		int oid)
{
	bool next = FALSE;

	if (event->type == EVT_SELECT) {
		option_set(option_name(oid), !op_ptr->opt[oid]);
	} else if (event->type == EVT_KBRD) {
		if (event->key.code == 'y' || event->key.code == 'Y') {
			option_set(option_name(oid), TRUE);
			next = TRUE;
		} else if (event->key.code == 'n' || event->key.code == 'N') {
			option_set(option_name(oid), FALSE);
			next = TRUE;
		} else if (event->key.code == '?') {
			screen_save();
			show_file(format("option.txt#%s", option_name(oid)), NULL, 0, 0);
			screen_load();
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}

	if (next) {
		m->cursor++;
		m->cursor = (m->cursor + m->filter_count) % m->filter_count;
	}

	return TRUE;
}

/** Toggle option menu display and handling functions */
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
	
	menu_type *m = menu_new(MN_SKIN_SCROLL, &option_toggle_iter);

	/* for all menus */
	m->prompt = "Set option (y/n/t), '?' for information";
	m->cmd_keys = "?YyNnTt";
	m->selections = "abcdefghijklmopqrsuvwxz";
	m->flags = MN_DBL_TAP;

	/* We add 10 onto the page amount to indicate we're at birth */
	if (page == OPT_PAGE_BIRTH) {
		m->prompt = "You can only modify these options at character birth.";
		m->flags |= MN_NO_ACTION;
	} else if (page == OPT_PAGE_BIRTH + 10) {
		page -= 10;
	}

	/* for this particular menu */
	m->title = name;

	/* Find the number of valid entries */
	for (i = 0; i < OPT_PAGE_PER; i++) {
		if (option_page[page][i] == OPT_NONE)
			break;
	}

	/* Set the data to the player's options */
	menu_setpriv(m, OPT_MAX, &op_ptr->opt);
	menu_set_filter(m, option_page[page], i);
	menu_layout(m, &SCREEN_REGION);

	/* Run the menu */
	screen_save();

	clear_from(0);
	menu_select(m, 0, FALSE);

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


/*
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
	{
		new_flags[j] = op_ptr->window_flag[j];
	}


	/* Clear screen */
	screen_save();
	clear_from(0);

	/* Interact */
	while (1)
	{
		/* Prompt */
		prt("Window flags (<dir> to move, 't'/Enter to toggle, or ESC)", 0, 0);

		/* Display the windows */
		for (j = 0; j < ANGBAND_TERM_MAX; j++)
		{
			byte a = TERM_WHITE;

			const char *s = angband_term_name[j];

			/* Use color */
			if (j == x) a = TERM_L_BLUE;

			/* Window name, staggered, centered */
			Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
		}

		/* Display the options */
		for (i = 0; i < PW_MAX_FLAGS; i++)
		{
			byte a = TERM_WHITE;

			const char *str = window_flag_desc[i];

			/* Use color */
			if (i == y) a = TERM_L_BLUE;

			/* Unused option */
			if (!str) str = "(Unused option)";

			/* Flag name */
			Term_putstr(0, i + 5, -1, a, str);

			/* Display the windows */
			for (j = 0; j < ANGBAND_TERM_MAX; j++)
			{
				char c = '.';

				a = TERM_WHITE;

				/* Use color */
				if ((i == y) && (j == x)) a = TERM_L_BLUE;

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

		/* Mouse interaction */
		if (ke.type == EVT_MOUSE)
		{
			int choicey = ke.mouse.y - 5;
			int choicex = (ke.mouse.x - 35)/5;

			if ((choicey >= 0) && (choicey < PW_MAX_FLAGS)
				&& (choicex > 0) && (choicex < ANGBAND_TERM_MAX)
				&& !(ke.mouse.x % 5))
			{
				y = choicey;
				x = (ke.mouse.x - 35)/5;
			}
		}

		/* Allow escape */
		else if (ke.type == EVT_KBRD)
		{
			if (ke.key.code == ESCAPE || ke.key.code == 'q')
				break;

			/* Toggle */
			else if (ke.key.code == '5' || ke.key.code == 't' ||
					ke.key.code == '\n' || ke.key.code == '\r')
			{
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
			if (d != 0)
			{
				x = (x + ddx[d] + 8) % ANGBAND_TERM_MAX;
				y = (y + ddy[d] + 16) % PW_MAX_FLAGS;
			}
		}
	}

	/* Notice changes */
	subwindows_set_flags(new_flags, ANGBAND_TERM_MAX);

	screen_load();
}



/*** Interact with keymaps ***/

/*
 * Current (or recent) keymap action
 */
static struct keypress keymap_buffer[KEYMAP_ACTION_MAX];


/*
 * Ask for, and display, a keymap trigger.
 *
 * Returns the trigger input.
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static struct keypress keymap_get_trigger(void)
{
	char tmp[80];
	struct keypress buf[2] = { { 0 }, { 0 } };

	/* Flush */
	flush();

	/* Get a key */
	buf[0] = inkey();

	/* Convert to ascii */
	keypress_to_text(tmp, sizeof(tmp), buf, FALSE);

	/* Hack -- display the trigger */
	Term_addstr(-1, TERM_WHITE, tmp);

	/* Flush */
	flush();

	/* Return trigger */
	return buf[0];
}


/*
 * Macro menu action functions
 */

static void ui_keymap_pref_load(const char *title, int row)
{
	do_cmd_pref_file_hack(16);
}

static void ui_keymap_pref_append(const char *title, int row)
{
	(void)dump_pref_file(keymap_dump, "Dump keymaps", 13);
}

static void ui_keymap_query(const char *title, int row)
{
	char tmp[1024];
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;
	struct keypress c;
	const struct keypress *act;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);
	
	/* Get a keymap trigger & mapping */
	c = keymap_get_trigger();
	act = keymap_find(mode, c);
	
	/* Nothing found */
	if (!act)
	{
		/* Prompt */
		prt("No keymap with that trigger.  Press any key to continue.", 16, 0);
		inkey();
	}
	
	/* Found one */
	else
	{
		/* Analyze the current action */
		keypress_to_text(tmp, sizeof(tmp), act, FALSE);
	
		/* Display the current action */
		prt("Found: ", 15, 0);
		Term_addstr(-1, TERM_WHITE, tmp);

		prt("Press any key to continue.", 17, 0);
		inkey();
	}
}

static void ui_keymap_create(const char *title, int row)
{
	bool done = FALSE;
	size_t n = 0;

	struct keypress c;
	char tmp[1024];
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

	prt(title, 13, 0);
	prt("Key: ", 14, 0);

	c = keymap_get_trigger();
	if (c.code == '$') {
		c_prt(TERM_L_RED, "The '$' key is reserved.", 16, 2);
		prt("Press any key to continue.", 18, 0);
		inkey();
		return;
	}

	/* Get an encoded action, with a default response */
	while (!done) {
		struct keypress kp = {EVT_NONE, 0, 0};

		int color = TERM_WHITE;
		if (n == 0) color = TERM_YELLOW;
		if (n == KEYMAP_ACTION_MAX) color = TERM_L_RED;

		keypress_to_text(tmp, sizeof(tmp), keymap_buffer, FALSE);
		c_prt(color, format("Action: %s", tmp), 15, 0);

		c_prt(TERM_L_BLUE, "  Press '$' when finished.", 17, 0);
		c_prt(TERM_L_BLUE, "  Use 'CTRL-U' to reset.", 18, 0);
		c_prt(TERM_L_BLUE, format("(Maximum keymap length is %d keys.)", KEYMAP_ACTION_MAX), 19, 0);

		kp = inkey();

		if (kp.code == '$') {
			done = TRUE;
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
		keymap_add(mode, c, keymap_buffer, TRUE);
		prt("Keymap added.  Press any key to continue.", 17, 0);
		inkey();
	}
}

static void ui_keymap_remove(const char *title, int row)
{
	struct keypress c;
	int mode = OPT(rogue_like_commands) ? KEYMAP_MODE_ROGUE : KEYMAP_MODE_ORIG;

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

	message_flush();

	clear_from(13);

	/* Show current action */
	prt("Current action (if any) shown below:", 13, 0);
	keypress_to_text(tmp, sizeof(tmp), keymap_buffer, FALSE);
	prt(tmp, 14, 0);
}

static menu_type *keymap_menu;
static menu_action keymap_actions[] =
{
	{ 0, 0, "Load a user pref file",    ui_keymap_pref_load },
	{ 0, 0, "Append keymaps to a file", ui_keymap_pref_append },
	{ 0, 0, "Query a keymap",           ui_keymap_query },
	{ 0, 0, "Create a keymap",          ui_keymap_create },
	{ 0, 0, "Remove a keymap",          ui_keymap_remove },
};

static void do_cmd_keymaps(const char *title, int row)
{
	region loc = {0, 0, 0, 12};

	screen_save();
	clear_from(0);

	if (!keymap_menu)
	{
		keymap_menu = menu_new_action(keymap_actions,
				N_ELEMENTS(keymap_actions));
	
		keymap_menu->title = title;
		keymap_menu->selections = lower_case;
		keymap_menu->browse_hook = keymap_browse_hook;
	}

	menu_layout(keymap_menu, &loc);
	menu_select(keymap_menu, 0, FALSE);

	screen_load();
}



/*** Interact with visuals ***/

static void visuals_pref_load(const char *title, int row)
{
	do_cmd_pref_file_hack(15);
}

#ifdef ALLOW_VISUALS

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

#endif /* ALLOW_VISUALS */

static void visuals_reset(const char *title, int row)
{
	/* Reset */
	reset_visuals(TRUE);

	/* Message */
	prt("", 0, 0);
	msg("Visual attr/char tables reset.");
	message_flush();
}


static menu_type *visual_menu;
static menu_action visual_menu_items [] =
{
	{ 0, 0, "Load a user pref file",   visuals_pref_load },
#ifdef ALLOW_VISUALS
	{ 0, 0, "Dump monster attr/chars", visuals_dump_monsters },
	{ 0, 0, "Dump object attr/chars",  visuals_dump_objects },
	{ 0, 0, "Dump feature attr/chars", visuals_dump_features },
	{ 0, 0, "Dump flavor attr/chars",  visuals_dump_flavors },
#endif /* ALLOW_VISUALS */
	{ 0, 0, "Reset visuals",           visuals_reset },
};


static void visuals_browse_hook(int oid, void *db, const region *loc)
{
	message_flush();
	clear_from(1);
}


/*
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
	menu_select(visual_menu, 0, FALSE);

	screen_load();
}


/*** Interact with colours ***/

#ifdef ALLOW_COLORS

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
	while (1)
	{
		const char *name;
		char index;

		struct keypress cx;

		/* Clear */
		clear_from(10);

		/* Exhibit the normal colors */
		for (i = 0; i < BASIC_COLORS; i++)
		{
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
		Term_putstr(5, 10, -1, TERM_WHITE,
					format("Color = %d, Name = %s, Index = %c", a, name, index));

		/* Label the Current values */
		Term_putstr(5, 12, -1, TERM_WHITE,
				format("K = 0x%02x / R,G,B = 0x%02x,0x%02x,0x%02x",
				   angband_color_table[a][0],
				   angband_color_table[a][1],
				   angband_color_table[a][2],
				   angband_color_table[a][3]));

		/* Prompt */
		Term_putstr(0, 14, -1, TERM_WHITE,
				"Command (n/N/k/K/r/R/g/G/b/B): ");

		/* Get a command */
		cx = inkey();

		/* All done */
		if (cx.code == ESCAPE) break;

		/* Analyze */
		if (cx.code == 'n') a = (byte)(a + 1);
		if (cx.code == 'N') a = (byte)(a - 1);
		if (cx.code == 'k') angband_color_table[a][0] = (byte)(angband_color_table[a][0] + 1);
		if (cx.code == 'K') angband_color_table[a][0] = (byte)(angband_color_table[a][0] - 1);
		if (cx.code == 'r') angband_color_table[a][1] = (byte)(angband_color_table[a][1] + 1);
		if (cx.code == 'R') angband_color_table[a][1] = (byte)(angband_color_table[a][1] - 1);
		if (cx.code == 'g') angband_color_table[a][2] = (byte)(angband_color_table[a][2] + 1);
		if (cx.code == 'G') angband_color_table[a][2] = (byte)(angband_color_table[a][2] - 1);
		if (cx.code == 'b') angband_color_table[a][3] = (byte)(angband_color_table[a][3] + 1);
		if (cx.code == 'B') angband_color_table[a][3] = (byte)(angband_color_table[a][3] - 1);

		/* Hack -- react to changes */
		Term_xtra(TERM_XTRA_REACT, 0);

		/* Hack -- redraw */
		Term_redraw();
	}
}

static void colors_browse_hook(int oid, void *db, const region *loc)
{
	message_flush();
	clear_from(1);
}


static menu_type *color_menu;
static menu_action color_events [] =
{
	{ 0, 0, "Load a user pref file", colors_pref_load },
	{ 0, 0, "Dump colors",           colors_pref_dump },
	{ 0, 0, "Modify colors",         colors_modify }
};

/*
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
	menu_select(color_menu, 0, FALSE);

	screen_load();
}

#endif


/*** Non-complex menu actions ***/

static bool askfor_aux_numbers(char *buf, size_t buflen, size_t *curs, size_t *len, struct keypress keypress, bool firsttime)
{
	switch (keypress.code)
	{
		case ESCAPE:
		case '\n':
		case '\r':
		case ARROW_LEFT:
		case ARROW_RIGHT:
		case 0x7F:
		case '\010':
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
			return askfor_aux_keypress(buf, buflen, curs, len, keypress, firsttime);
	}

	return FALSE;
}


/*
 * Set base delay factor
 */
static void do_cmd_delay(const char *name, int row)
{
	char tmp[4] = "";
	int msec = op_ptr->delay_factor;

	strnfmt(tmp, sizeof(tmp), "%i", op_ptr->delay_factor);

	screen_save();

	/* Prompt */
	prt("Command: Base Delay Factor", 20, 0);

	prt(format("Current base delay factor: %d msec",
			   op_ptr->delay_factor, msec), 22, 0);
	prt("New base delay factor (0-255): ", 21, 0);

	/* Ask for a numeric value */
	if (askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers)) {
		u16b val = (u16b) strtoul(tmp, NULL, 0);
		op_ptr->delay_factor = MIN(val, 255);
	}

	screen_load();
}


/*
 * Set hitpoint warning level
 */
static void do_cmd_hp_warn(const char *name, int row)
{
	bool res;
	char tmp[4] = "";
	u16b warn;

	strnfmt(tmp, sizeof(tmp), "%i", op_ptr->hitpoint_warn);

	screen_save();

	/* Prompt */
	prt("Command: Hitpoint Warning", 20, 0);

	prt(format("Current hitpoint warning: %d (%d%%)",
			   op_ptr->hitpoint_warn, op_ptr->hitpoint_warn * 10), 22, 0);
	prt("New hitpoint warning (0-9): ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res)
	{
		warn = (u16b) strtoul(tmp, NULL, 0);
		
		/* Reset nonsensical warnings */
		if (warn > 9)
			warn = 0;

		op_ptr->hitpoint_warn = warn;
	}

	screen_load();
}


/*
 * Set "lazy-movement" delay
 */
static void do_cmd_lazymove_delay(const char *name, int row)
{
	bool res;
	char tmp[4] = "";

	strnfmt(tmp, sizeof(tmp), "%i", lazymove_delay);

	screen_save();

	/* Prompt */
	prt("Command: Movement Delay Factor", 20, 0);

	prt(format("Current movement delay: %d (%d msec)",
			   lazymove_delay, lazymove_delay * 10), 22, 0);
	prt("New movement delay: ", 21, 0);

	/* Ask the user for a string */
	res = askfor_aux(tmp, sizeof(tmp), askfor_aux_numbers);

	/* Process input */
	if (res)
	{
		lazymove_delay = (u16b) strtoul(tmp, NULL, 0);
	}

	screen_load();
}



/*
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

	screen_save();

	/* Prompt */
	prt("Command: Load a user pref file", row, 0);

	/* Prompt */
	prt("File: ", row + 2, 0);

	/* Default filename */
	strnfmt(ftmp, sizeof ftmp, "%s.prf", op_ptr->base_name);

	/* Ask for a file (or cancel) */
	if (askfor_aux(ftmp, sizeof ftmp, NULL))
	{
		/* Process the given filename */
		if (process_pref_file(ftmp, FALSE, TRUE) == FALSE)
		{
			/* Mention failure */
			prt("", 0, 0);
			msg("Failed to load '%s'!", ftmp);
		}
		else
		{
			/* Mention success */
			prt("", 0, 0);
			msg("Loaded '%s'.", ftmp);
		}
	}

	screen_load();
}


/*
 * Write options to a file.
 */
static void do_dump_options(const char *title, int row)
{
	dump_pref_file(option_dump, "Dump options", 20);
}

/*
 * Load a pref file.
 */
static void options_load_pref_file(const char *n, int row)
{
	do_cmd_pref_file_hack(20);
}







/*** Quality-squelch menu ***/

/* Structure to describe tval/description pairings. */
typedef struct
{
	int tval;
	const char *desc;
} tval_desc;

/* Categories for sval-dependent squelch. */
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
	{ TV_MAGIC_BOOK,	"Magic books" },
	{ TV_PRAYER_BOOK,	"Prayer books" },
	{ TV_SPIKE,			"Spikes" },
	{ TV_LIGHT,			"Lights" },
	{ TV_FLASK,			"Flasks of oil" },
/*	{ TV_DRAG_ARMOR,	"Dragon Mail Armor" }, */
	{ TV_GOLD,			"Money" },
};


/*
 * menu struct for differentiating aware from unaware squelch
 */
typedef struct
{
	object_kind *kind;
	bool aware;
} squelch_choice;

/*
 * Ordering function for squelch choices.
 * Aware comes before unaware, and then sort alphabetically.
 */
static int cmp_squelch(const void *a, const void *b)
{
	char bufa[80];
	char bufb[80];
	const squelch_choice *x = a;
	const squelch_choice *y = b;

	if (!x->aware && y->aware)
		return 1;
	if (x->aware && !y->aware)
		return -1;

	object_kind_name(bufa, sizeof(bufa), x->kind, x->aware);
	object_kind_name(bufb, sizeof(bufb), y->kind, y->aware);

	return strcmp(bufa, bufb);
}

/*
 * Display an entry in the menu.
 */
static void quality_display(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	const char *name = quality_choices[oid].name;

	byte level = squelch_level[oid];
	const char *level_name = quality_values[level].name;

	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);


	c_put_str(attr, format("%-20s : %s", name, level_name), row, col);
}


/*
 * Display the quality squelch subtypes.
 */
static void quality_subdisplay(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	const char *name = quality_values[oid].name;
	byte attr = (cursor ? TERM_L_BLUE : TERM_WHITE);

	c_put_str(attr, name, row, col);
}


/*
 * Handle keypresses.
 */
static bool quality_action(menu_type *m, const ui_event *event, int oid)
{
	menu_type menu;
	menu_iter menu_f = { NULL, NULL, quality_subdisplay, NULL, NULL };
	region area = { 27, 2, 29, SQUELCH_MAX };
	ui_event evt;
	int count;

	/* Display at the right point */
	area.row += oid;

	/* Save */
	screen_save();

	/* Work out how many options we have */
	count = SQUELCH_MAX;
	if ((oid == TYPE_RING) || (oid == TYPE_AMULET))
		count = area.page_rows = SQUELCH_BAD + 1;

	/* Run menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu_setpriv(&menu, count, quality_values);

	/* Stop menus from going off the bottom of the screen */
	if (area.row + menu.count > Term->hgt - 1)
		area.row += Term->hgt - 1 - area.row - menu.count;

	menu_layout(&menu, &area);

	window_make(area.col - 2, area.row - 1, area.col + area.width + 2, area.row + area.page_rows);

	evt = menu_select(&menu, 0, TRUE);

	/* Set the new value appropriately */
	if (evt.type == EVT_SELECT)
		squelch_level[oid] = menu.cursor;

	/* Load and finish */
	screen_load();
	return TRUE;
}

/*
 * Display quality squelch menu.
 */
static void quality_menu(void *unused, const char *also_unused)
{
	menu_type menu;
	menu_iter menu_f = { NULL, NULL, quality_display, quality_action, NULL };
	region area = { 0, 0, 0, 0 };

	/* Save screen */
	screen_save();
	clear_from(0);

	/* Set up the menu */
	menu_init(&menu, MN_SKIN_SCROLL, &menu_f);
	menu.title = "Quality squelch menu";
	menu_setpriv(&menu, TYPE_MAX, quality_values);
	menu_layout(&menu, &area);

	/* Select an entry */
	menu_select(&menu, 0, FALSE);

	/* Load screen */
	screen_load();
	return;
}



/*** Sval-dependent menu ***/

/*
 * Display an entry on the sval menu
 */
static void squelch_sval_menu_display(menu_type *menu, int oid, bool cursor,
		int row, int col, int width)
{
	char buf[80];
	const squelch_choice *choice = menu_priv(menu);

	object_kind *kind = choice[oid].kind;
	bool aware = choice[oid].aware;

	byte attr = curs_attrs[(int)aware][0 != cursor];

	/* Acquire the "name" of object "i" */
	object_kind_name(buf, sizeof(buf), kind, aware);

	/* Print it */
	c_put_str(attr, format("[ ] %s", buf), row, col);
	if ((aware && (kind->squelch & SQUELCH_IF_AWARE)) ||
			(!aware && (kind->squelch & SQUELCH_IF_UNAWARE)))
		c_put_str(TERM_L_RED, "*", row, col + 1);
}


/*
 * Deal with events on the sval menu
 */
static bool squelch_sval_menu_action(menu_type *m, const ui_event *event,
		int oid)
{
	const squelch_choice *choice = menu_priv(m);

	if (event->type == EVT_SELECT)
	{
		object_kind *kind = choice[oid].kind;

		/* Toggle the appropriate flag */
		if (choice[oid].aware)
			kind->squelch ^= SQUELCH_IF_AWARE;
		else
			kind->squelch ^= SQUELCH_IF_UNAWARE;

		p_ptr->notice |= PN_SQUELCH;
		return TRUE;
	}

	return FALSE;
}

static const menu_iter squelch_sval_menu =
{
	NULL,
	NULL,
	squelch_sval_menu_display,
	squelch_sval_menu_action,
	NULL,
};


/**
 * Collect all tvals in the big squelch_choice array
 */
static int squelch_collect_kind(int tval, squelch_choice **ch)
{
	squelch_choice *choice;
	int num = 0;

	int i;

	/* Create the array, with entries both for aware and unaware squelch */
	choice = mem_alloc(2 * z_info->k_max * sizeof *choice);

	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!k_ptr->name || k_ptr->tval != tval)
			continue;

		if (!k_ptr->aware)
		{
			/* can unaware squelch anything */
			choice[num].kind = k_ptr;
			choice[num++].aware = FALSE;
		}

		if ((k_ptr->everseen && !of_has(k_ptr->flags, OF_INSTA_ART)) || k_ptr->tval == TV_GOLD)
		{
			/* Do not display the artifact base kinds in this list */
			/* aware squelch requires everseen */
			/* do not require awareness for aware squelch, so people can set at game start */
			choice[num].kind = k_ptr;
			choice[num++].aware = TRUE;
		}
	}

	if (num == 0)
		mem_free(choice);
	else
		*ch = choice;

	return num;
}

/*
 * Display list of svals to be squelched.
 */
static bool sval_menu(int tval, const char *desc)
{
	menu_type *menu;
	region area = { 1, 2, -1, -1 };

	squelch_choice *choices;

	int n_choices = squelch_collect_kind(tval, &choices);
	if (!n_choices)
		return FALSE;

	/* sort by name in squelch menus except for categories of items that are aware from the start */
	switch (tval)
	{
		case TV_LIGHT:
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_DRAG_ARMOR:
		case TV_GOLD:
			/* leave sorted by sval */
			break;

		default:
			/* sort by name */
			sort(choices, n_choices, sizeof(*choices), cmp_squelch);
	}


	/* Save the screen and clear it */
	screen_save();
	clear_from(0);

	/* Help text */
	prt(format("Squelch the following %s:", desc), 0, 0);

	/* Run menu */
	menu = menu_new(MN_SKIN_COLUMNS, &squelch_sval_menu);
	menu_setpriv(menu, n_choices, choices);
	menu_layout(menu, &area);
	menu_select(menu, 0, FALSE);

	/* Free memory */
	FREE(choices);

	/* Load screen */
	screen_load();
	return TRUE;
}


/* Returns TRUE if there's anything to display a menu of */
static bool seen_tval(int tval)
{
	int i;

	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip empty objects, unseen objects, and incorrect tvals */
		if (!k_ptr->name) continue;
		if (!k_ptr->everseen) continue;
		if (k_ptr->tval != tval) continue;

		 return TRUE;
	}


	return FALSE;
}


/* Extra options on the "item options" menu */
struct
{
	char tag;
	const char *name;
	void (*action)(); /* this is a nasty hack */
} extra_item_options[] = {
	{ 'Q', "Quality squelching options", quality_menu },
	{ '{', "Autoinscription setup", textui_browse_object_knowledge },
};

static char tag_options_item(menu_type *menu, int oid)
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

static int valid_options_item(menu_type *menu, int oid)
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

static void display_options_item(menu_type *menu, int oid, bool cursor, int row, int col, int width)
{
	size_t line = (size_t) oid;

	/* First section of menu - the svals */
	if (line < N_ELEMENTS(sval_dependent))
	{
		bool known = seen_tval(sval_dependent[line].tval);
		byte attr = curs_attrs[known ? CURS_KNOWN: CURS_UNKNOWN][(int)cursor];

		c_prt(attr, sval_dependent[line].desc, row, col);
	}
	/* Second section - the "extra options" */
	else
	{
		byte attr = curs_attrs[CURS_KNOWN][(int)cursor];

		line = line - N_ELEMENTS(sval_dependent) - 1;

		if (line < N_ELEMENTS(extra_item_options))
			c_prt(attr, extra_item_options[line].name, row, col);
	}
}

static bool handle_options_item(menu_type *menu, const ui_event *event, int oid)
{
	if (event->type == EVT_SELECT)
	{
		if ((size_t) oid < N_ELEMENTS(sval_dependent))
		{
			sval_menu(sval_dependent[oid].tval, sval_dependent[oid].desc);
		}
		else
		{
			oid = oid - (int)N_ELEMENTS(sval_dependent) - 1;
			assert((size_t) oid < N_ELEMENTS(extra_item_options));
			extra_item_options[oid].action();
		}

		return TRUE;
	}

	return FALSE;
}


static const menu_iter options_item_iter =
{
	tag_options_item,
	valid_options_item,
	display_options_item,
	handle_options_item,
	NULL
};


/*
 * Display and handle the main squelching menu.
 */
void do_cmd_options_item(const char *title, int row)
{
	menu_type menu;

	menu_init(&menu, MN_SKIN_SCROLL, &options_item_iter);
	menu_setpriv(&menu, N_ELEMENTS(sval_dependent) + N_ELEMENTS(extra_item_options) + 1, NULL);

	menu.title = title;
	menu_layout(&menu, &SCREEN_REGION);

	screen_save();
	clear_from(0);
	menu_select(&menu, 0, FALSE);
	screen_load();

	p_ptr->notice |= PN_SQUELCH;

	return;
}



/*** Main menu definitions and display ***/

static menu_type *option_menu;
static menu_action option_actions[] = 
{
	{ 0, 'a', "Interface and display options", option_toggle_menu },
	{ 0, 'e', "Warning and disturbance options", option_toggle_menu },
	{ 0, 'f', "Birth (difficulty) options", option_toggle_menu },
	{ 0, 'g', "Cheat options", option_toggle_menu },
	{0, 0, 0, 0}, /* Load and append */
	{ 0, 'w', "Subwindow display settings", do_cmd_options_win },
	{ 0, 's', "Item squelch settings", do_cmd_options_item },
	{ 0, 'd', "Set base delay factor", do_cmd_delay },
	{ 0, 'h', "Set hitpoint warning", do_cmd_hp_warn },
	{ 0, 'i', "Set movement delay", do_cmd_lazymove_delay },
	{ 0, 'l', "Load a user pref file", options_load_pref_file },
	{ 0, 'o', "Save options", do_dump_options }, 
	{0, 0, 0, 0}, /* Interact with */	
	{ 0, 'm', "Interact with keymaps (advanced)", do_cmd_keymaps },
	{ 0, 'v', "Interact with visuals (advanced)", do_cmd_visuals },

#ifdef ALLOW_COLORS
	{ 0, 'c', "Interact with colours (advanced)", do_cmd_colors },
#endif /* ALLOW_COLORS */
};


/*
 * Display the options main menu.
 */
void do_cmd_options(void)
{
	if (!option_menu)
	{
		/* Main option menu */
		option_menu = menu_new_action(option_actions,
				N_ELEMENTS(option_actions));

		option_menu->title = "Options Menu";
		option_menu->flags = MN_CASELESS_TAGS;
	}

	screen_save();
	clear_from(0);

	menu_layout(option_menu, &SCREEN_REGION);
	menu_select(option_menu, 0, FALSE);

	screen_load();
}










/*
 * Determines whether a tval is eligible for sval-squelch.
 */
bool squelch_tval(int tval)
{
	size_t i;

	/* Only squelch if the tval's allowed */
	for (i = 0; i < N_ELEMENTS(sval_dependent); i++)
	{
		if (tval == sval_dependent[i].tval)
			return TRUE;
	}

	return FALSE;
}
