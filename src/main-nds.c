/**
 * \file main-nds.c
 * \brief Main file for playing on the Nintendo DS
 *
 * Copyright (c) 2010 Nick McConnell
 *
 * Many of the routines are based on (or lifted directly from) brettk's
 * excellent NethackDS: http://frodo.dyn.gno.org/~brettk/NetHackDS
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

#ifdef __3DS__
/* We can't include 3ds.h because utf32_to_utf8 conflicts */
#include <3ds/types.h>
#include <3ds/services/apt.h>
#include <3ds/services/fs.h>
#include <3ds/services/hid.h>
#include <3ds/os.h>
#else
#include <fat.h>
#include <nds.h>
#endif

#include "angband.h"
#include "buildid.h"
#include "init.h"
#include "main.h"
#include "savefile.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-input.h"
#include "ui-prefs.h"
#include "ui-term.h"
#include "ui-init.h"

/* DS includes */
#include "nds/nds-draw.h"
#include "nds/nds-event.h"
#include "nds/nds-keyboard.h"
#include "nds/nds-buttons.h"
#include "nds/nds-screenkeys.h"
#include "nds/nds-slot2-virt.h"

#ifndef __3DS__
#define hidScanInput scanKeys
#endif

#ifdef DEBUG_MEMORY_USAGE

#include <malloc.h>
#include <unistd.h>

/* https://devkitpro.org/viewtopic.php?f=6&t=3057 */

extern uint8_t *fake_heap_end;
extern uint8_t *fake_heap_start;

static int nds_free_memory_bytes(void) {
	struct mallinfo info = mallinfo();
	return info.fordblks + (fake_heap_end - (uint8_t*)sbrk(0));
}

#endif


/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data {
	term t;
};

/*
 * Number of "term_data" structures to support XXX XXX XXX
 *
 * You MUST support at least one "term_data" structure, and the
 * game will currently use up to eight "term_data" structures if
 * they are available.
 *
 * If only one "term_data" structure is supported, then a lot of
 * the things that would normally go into a "term_data" structure
 * could be made into global variables instead.
 */
#define MAX_TERM_DATA 1

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];

/*
 * Color data
 */
static nds_pixel color_data[MAX_COLORS];

/*** Function hooks needed by "Term" ***/

/*
 * Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_nds(term *t)
{
	term_data *td = (term_data *)(t->data);

	/* XXX XXX XXX */
}

/*
 * Nuke an old "term"
 *
 * This function is called when an old "term" is no longer needed.  It should
 * do whatever is needed to clean up before the program exits, such as wiping
 * the screen, restoring the cursor, fixing the font, etc.  Often this function
 * does nothing and lets the operating system clean up when the program quits.
 */
static void Term_nuke_nds(term *t)
{
	term_data *td = (term_data *)(t->data);

	/* XXX XXX XXX */
}

/*
 * Handle a touch on the touch screen.
 */
static void handle_touch(int x, int y, int button, bool press)
{
	/* The co-ordinates are only used in Angband format. */
	nds_pixel_to_square(&x, &y, x, y);

	if (press)
		Term_mousepress(x, y, button);
}

void do_vblank()
{
#ifdef DEBUG_MEMORY_USAGE
	char mem_usage_str[96];
	snprintf(mem_usage_str, sizeof(mem_usage_str), "Free mem: %d bytes", nds_free_memory_bytes());
	nds_draw_str(0, NDS_SCREEN_LINES * 2 - 1, mem_usage_str, NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
#endif

	nds_video_vblank();

#ifdef __3DS__
	/* Handle home menu, poweroff, etc */
	if (!aptMainLoop()) {
		quit(NULL);
	}
#endif

	hidScanInput();

	/* Handle button inputs */
	nds_btn_vblank();

	/* Handle touchscreen (keyboard) inputs */
	nds_kbd_vblank();

	/* Handle on-screen key inputs */
	nds_scrkey_vblank();
}

/*END JUST MOVED */

/*
 * An event handler XXX XXX XXX
 *
 * You may need an event handler, which can be used by both
 * by the "TERM_XTRA_BORED" and "TERM_XTRA_EVENT" entries in
 * the "Term_xtra_xxx()" function, and also to wait for the
 * user to perform whatever user-interface operation is needed
 * to request the start of a new game or the loading of an old
 * game, both of which should launch the "play_game()" function.
 */
static errr CheckEvents(bool wait)
{
	nds_event e;

	do_vblank();

	if (!wait && !nds_event_ready())
		return (1);

	do {
		e = nds_event_get();

		do_vblank();
	} while (e.type == NDS_EVENT_INVALID);

	switch (e.type) {
	case NDS_EVENT_MOUSE:
		handle_touch(e.mouse.x, e.mouse.y, 1, true);
		break;
	case NDS_EVENT_KEYBOARD:
		Term_keypress(e.keyboard.key, e.keyboard.mods);
		break;
	default:
		nds_logf("Got unknown event type: %d\n", e.type);
		break;
	}

	return (0);
}

static void init_color_data(void)
{
	/* Initialize the "color_data" array */
	for (int i = 0; i < MAX_COLORS; i++) {
#ifdef __3DS__
		color_data[i] = angband_color_table[i][1] << 24 |
		                angband_color_table[i][2] << 16 |
		                angband_color_table[i][3] << 8;
#else
		color_data[i] = RGB15(angband_color_table[i][1] >> 3,
		                      angband_color_table[i][2] >> 3,
		                      angband_color_table[i][3] >> 3) | 0x8000;
#endif
	}
}

/*
 * Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "ui-term.c" package,
 * or by the application itself.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_xtra_nds(int n, int v)
{
	term_data *td = (term_data *)(Term->data);

	/* Analyze */
	switch (n) {
	case TERM_XTRA_EVENT: {
		/*
		 * Process some pending events
		 */
		return (CheckEvents(v));
	}

	case TERM_XTRA_FLUSH: {
		/*
		 * Flush all pending events
		 */
		while (!CheckEvents(false))
			;

		return (0);
	}

	case TERM_XTRA_CLEAR: {
		/*
		 * Clear the entire window
		 */
		int x, y;

		for (y = 0; y < NDS_SCREEN_LINES; y++) {
			for (x = 0; x < NDS_SCREEN_COLS; x++) {
				nds_draw_char(x, y, 0, color_data[COLOUR_DARK], color_data[COLOUR_DARK]);
			}
		}

		return (0);
	}

	case TERM_XTRA_SHAPE: {
		/*
		 * Set the cursor visibility XXX XXX XXX
		 *
		 * This action should change the visibility of the cursor,
		 * if possible, to the requested value (0=off, 1=on)
		 *
		 * This action is optional, but can improve both the
		 * efficiency (and attractiveness) of the program.
		 */

		return (0);
	}

	case TERM_XTRA_FROSH: {
		return (0);
	}

	case TERM_XTRA_FRESH: {
		return (0);
	}

	case TERM_XTRA_NOISE: {
		/*
		 * Make a noise XXX XXX XXX
		 *
		 * This action should produce a "beep" noise.
		 *
		 * This action is optional, but convenient.
		 */

		return (0);
	}

	case TERM_XTRA_BORED: {
		/*
		 * Handle random events when bored
		 */
		return (CheckEvents(0));
	}

	case TERM_XTRA_REACT: {
		/*
		 * React to global changes XXX XXX XXX
		 *
		 * For example, this action can be used to react to
		 * changes in the global "color_table[256][4]" array.
		 *
		 * This action is optional, but can be very useful for
		 * handling "color changes" and the "arg_sound" and/or
		 * "arg_graphics" options.
		 */

		init_color_data();

		return (0);
	}

	case TERM_XTRA_ALIVE: {
		/*
		 * Change the "hard" level XXX XXX XXX
		 *
		 * This action is used if the program changes "aliveness"
		 * by being either "suspended" (v=0) or "resumed" (v=1)
		 * This action is optional, unless the computer uses the
		 * same "physical screen" for multiple programs, in which
		 * case this action should clean up to let other programs
		 * use the screen, or resume from such a cleaned up state.
		 *
		 * This action is currently only used by "main-gcu.c",
		 * on UNIX machines, to allow proper "suspending".
		 */

		return (0);
	}

	case TERM_XTRA_LEVEL: {
		/*
		 * Change the "soft" level XXX XXX XXX
		 *
		 * This action is used when the term window changes "activation"
		 * either by becoming "inactive" (v=0) or "active" (v=1)
		 *
		 * This action can be used to do things like activate the proper
		 * font / drawing mode for the newly active term window.  This
		 * action should NOT change which window has the "focus", which
		 * window is "raised", or anything like that.
		 *
		 * This action is optional if all the other things which depend
		 * on what term is active handle activation themself, or if only
		 * one "term_data" structure is supported by this file.
		 */

		return (0);
	}

	case TERM_XTRA_DELAY: {
		/*
		 * Delay for some milliseconds
		 */
#ifdef __3DS__
		if (v > 0) {
			svcSleepThread(1e6 * v);
		}
#else
		int i;
		for (i = 0; i < ((v + 15) >> 4); i++)
			nds_video_vblank();
#endif

		return (0);
	}
	}

	/* Unknown or Unhandled action */
	return (1);
}

/*
 * Display the cursor
 */
static errr Term_curs_nds(int x, int y)
{
	nds_draw_cursor(x, y);

	/* Success */
	return (0);
}

/*
 * Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_nds(int x, int y, int n)
{
	term_data *td = (term_data *)(Term->data);

	int i;

	/* Draw a blank */
	for (i = 0; i < n; i++)
		nds_draw_char(x + i, y, 0, color_data[COLOUR_DARK], color_data[COLOUR_DARK]);

	/* Success */
	return (0);
}

/*
 * Draw some text on the screen
 *
 * This function should actually display an array of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which contains
 * exactly "n" characters and which is NOT null-terminated.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * In color environments, you should activate the color contained
 * in "color_data[a & (MAX_COLORS - 1)]", if needed, before drawing anything.
 *
 * You may ignore the "attribute" if you are only supporting a
 * monochrome environment, since this routine is normally never
 * called to display "black" (invisible) text, including the
 * default "spaces", and all other colors should be drawn in
 * the "normal" color in a monochrome environment.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_nds(int x, int y, int n, int a, const wchar_t *s)
{
	for (int i = 0; i < n; i++) {
		nds_draw_char(x + i, y, s[i], color_data[a & (MAX_COLORS - 1)], color_data[COLOUR_DARK]);
	}

	return (0);
}

/*** Internal Functions ***/

/*
 * Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.  One way to allow some
 * variation is to add fields to the "term_data" structure listing
 * parameters for that window, initialize them in the "init_xxx()"
 * function, and then use them in the code below.
 *
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 */
static void term_data_link(int i)
{
	term_data *td = &data[i];

	term *t = &td->t;

	/* Initialize the term */
	term_init(t, NDS_SCREEN_COLS, NDS_SCREEN_LINES, 256);

	/* Choose "soft" or "hard" cursor XXX XXX XXX */
	/* A "soft" cursor must be explicitly "drawn" by the program */
	/* while a "hard" cursor has some "physical" existance and is */
	/* moved whenever text is drawn on the screen.  See "term.c". */
	t->soft_cursor = true;

	/* Use "Term_pict()" for all attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* td->t->always_pict = true; */

	/* Use "Term_pict()" for some attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* t->higher_pict = true; */

	/* Use "Term_text()" even for "black" text XXX XXX XXX */
	/* See the "Term_text_xxx()" function above. */
	/* t->always_text = true; */

	/* Ignore the "TERM_XTRA_BORED" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	t->never_bored = true;

	/* Ignore the "TERM_XTRA_FROSH" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	/* td->t->never_frosh = true; */

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_nds;
	t->nuke_hook = Term_nuke_nds;

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_nds;
	t->curs_hook = Term_curs_nds;
	t->wipe_hook = Term_wipe_nds;
	t->text_hook = Term_text_nds;

	/* Remember where we came from */
	t->data = (void *)(td);

	/* Activate it */
	Term_activate(t);
}

/*
 * Initialization function
 */
errr init_nds(void)
{
	/* Initialize globals */

	/* Initialize "term_data" structures */

	int i;
	bool none = true;

	term_data *td;

	/* Main window */
	td = &data[0];
	memset(td, 0, sizeof(term_data));

	init_color_data();

	/* Create windows (backwards!) */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--) {
		/* Link */
		term_data_link(i);
		none = false;

		/* Set global pointer */
		angband_term[0] = Term;
	}

	if (none)
		return (1);

	/* Success */
	return (0);
}

/*
 * Initialize file path information
 *
 * This function is used to keep the "path" variable off the stack.
 */
static void init_files(void)
{
	char path[1024];

	/* Prepare the path */
	strcpy(path, "/angband/lib/");

	/* Pass the paths to the game */
	init_file_paths(path, path, path);

	/* Set the savefile path to a well-known value */
	strcpy(savefile, "/angband/lib/save/PLAYER");

	/* Create all the missing required directories */
	create_needed_dirs();
}

/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(const char *str)
{
	/* Warning */
	if (str) {
		nds_raw_print(str);
	}
}

void nds_exit(int code)
{
	/* If we exited gracefully, just shut down */
	if (!code) {
		return;
	}

	/* Lock up so that the user can see potential errors */
	while(1) {
#ifdef __3DS__
		if (!aptMainLoop())
			break;
#endif

		nds_video_vblank();
	}
}

/*
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(const char *str)
{
	/* Give a warning */
	if (str) {
		nds_log(str);
	}

	/* Bail */
	nds_exit(str ? 1 : 0);
}

/*
 * Main function
 *
 * This function must do a lot of stuff.
 */
int main(int argc, char *argv[])
{
#ifdef __3DS__
	osSetSpeedupEnable(1);
#endif

	nds_video_init();

	nds_video_vblank();

	if (!nds_event_init()) {
		nds_log("\nFailed to initialize event queue\nCannot continue.\n");

		nds_exit(1);
		return 1;
	}

	nds_video_vblank();

#ifndef __3DS__
	mem_init_alt();

	if (!fatInitDefault()) {
		nds_log("\nError initializing FAT drivers.\n");
		nds_log("Make sure the game is patched with the correct DLDI.\n");
		nds_log(" (see https://www.chishm.com/DLDI/ for more info).\n");
		nds_log("\n\nUnable to access filesystem.\nCannot continue.\n");

		nds_exit(1);

		return 1;
	}
#endif

	nds_video_vblank();

	if (!nds_kbd_init()) {
		nds_log("\nError loading keyboard graphics.\nCannot continue.\n");

		nds_exit(1);
		return 1;
	}

	nds_btn_init();

	nds_scrkey_init();

	/* Activate hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;

	/* Initialize the windows */
	if (init_nds())
		quit("No terminals initialized!");

	/* XXX XXX XXX */
	ANGBAND_SYS = "nds";

	/* Set up file paths */
	init_files();

	/* Set command hook */
	cmd_get_hook = textui_get_cmd;

	/* Initialize */
	init_display();
	init_angband();
	textui_init();

	/* Wait for response */
	pause_line(Term);

	/* Play the game */
	play_game(GAME_LOAD);

	/* Free resources */
	textui_cleanup();
	cleanup_angband();

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

#ifndef __3DS__
double sqrt(double x) {
	return f32tofloat(sqrtf32(floattof32(x)));
}
#endif
