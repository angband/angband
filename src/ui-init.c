/**
 * \file ui-init.c
 * \brief UI initialistion
 *
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
 *
 * This file is used to initialize various variables and arrays for the
 * Angband game.
 *
 * Several of the arrays for Angband are built from data files in the
 * "lib/gamedata" directory.
 */


#include "angband.h"
#include "game-input.h"
#include "game-event.h"
#include "init.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-init.h"
#include "ui-input.h"
#include "ui-keymap.h"
#include "ui-knowledge.h"
#include "ui-options.h"
#include "ui-output.h"
#include "ui-prefs.h"
#include "ui-term.h"

/**
 * Initialise the UI
 */
void textui_init(void)
{
	u32b default_window_flag[ANGBAND_TERM_MAX];

	if (!play_again) {
		/* Initialize graphics info and basic pref data */
		event_signal_message(EVENT_INITSTATUS, 0, "Loading basic pref file...");
		(void)process_pref_file("pref.prf", false, false);

		/* Sneakily init command list */
		cmd_init();

		/* Initialize knowledge things */
		textui_knowledge_init();

		/* Initialize input hooks */
		textui_input_init();

		/* Initialize visual prefs */
		textui_prefs_init();

		/* Hack -- Increase "icky" depth */
		screen_save_depth++;

		/* Verify main term */
		if (!term_screen)
			quit("Main window does not exist");

		/* Make sure main term is active */
		Term_activate(term_screen);

		/* Verify minimum size */
		if ((Term->hgt < 24) || (Term->wid < 80))
			plog("Main window is too small - please make it bigger.");

		/* Hack -- Turn off the cursor */
		(void)Term_set_cursor(false);

		/* Update terminals for preference changes. */
		(void) Term_xtra(TERM_XTRA_REACT, 0);
		(void) Term_redraw_all();
	}

	/* initialize window options that will be overridden by the savefile */
	memset(window_flag, 0, sizeof(u32b)*ANGBAND_TERM_MAX);
	memset(default_window_flag, 0, sizeof default_window_flag);
	if (ANGBAND_TERM_MAX > 1) default_window_flag[1] = (PW_MESSAGE);
	if (ANGBAND_TERM_MAX > 2) default_window_flag[2] = (PW_INVEN);
	if (ANGBAND_TERM_MAX > 3) default_window_flag[3] = (PW_MONLIST);
	if (ANGBAND_TERM_MAX > 4) default_window_flag[4] = (PW_ITEMLIST);
	if (ANGBAND_TERM_MAX > 5) default_window_flag[5] = (PW_MONSTER | PW_OBJECT);
	if (ANGBAND_TERM_MAX > 6) default_window_flag[6] = (PW_OVERHEAD);
	if (ANGBAND_TERM_MAX > 7) default_window_flag[7] = (PW_PLAYER_2);

	/* Set up the subwindows */
	subwindows_set_flags(default_window_flag, ANGBAND_TERM_MAX);

	/* Done */
	event_signal_message(EVENT_INITSTATUS, 0, "Initialization complete");
}


/**
 * Clean up UI
 */
void textui_cleanup(void)
{
	/* Cleanup any options menus */
	cleanup_options();

	keymap_free();
	textui_prefs_free();
	textui_knowledge_cleanup();
}
