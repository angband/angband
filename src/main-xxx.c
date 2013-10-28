/*
 * File: main-xxx.c
 * Purpose: Outline how to make a new "main-xxx" file.
 *
 * Copyright (c) 1997 Ben Harrison
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


/*
 * This file helps Angband work on non-existant computers.
 *
 * To use this file, use "Makefile.xxx", which defines USE_XXX.
 *
 *
 * This file is intended to show one way to build a "visual module"
 * for Angband to allow it to work with a new system.  It does not
 * actually work, but if the code near "XXX XXX XXX" comments were
 * replaced with functional code, then it probably would.
 *
 * See "z-term.c" for info on the concept of the "generic terminal",
 * and for more comments about what this file must supply.
 *
 * There are two basic ways to port Angband to a new system.  The
 * first involves modifying the "main-gcu.c" and/or "main-x11.c"
 * files to support some version of "curses" and/or "X11" on your
 * machine, and to compile with the "USE_GCU" and/or "USE_X11"
 * compilation flags defined.  The second involves creating a
 * new "main-xxx.c" file, based on this sample file (or on any
 * existing "main-xxx.c" file), and comes in two flavors, based
 * on whether it contains a "main()" function (as in "main-mac.c"
 * and "main-win.c") or not (as in "main-gcu.c" or "main-x11.c").
 *
 * If the "main-xxx.c" file includes its own "main()" function,
 * then you should NOT link in the "main.c" file, and your "main()"
 * function must process any command line arguments, initialize the
 * "visual system", and call "play_game()" with appropriate arguments.
 *
 * If the "main-xxx.c" file does not include its own "main()"
 * function, then you must add some code to "main.c" which, if
 * the appropriate "USE_XXX" compilation flag is defined, will
 * attempt to call the "init_xxx()" function in the "main-xxx.c"
 * file, which should initialize the "visual system" and return
 * zero if it was successful.  The "main()" function in "main.c"
 * will take care of processing command line arguments and then
 * calling "play_game()" with appropriate arguments.
 *
 * Note that the "util.c" file often contains functions which must
 * be modified in small ways for various platforms, even if you are
 * able to use the existing "main-gcu.c" and/or "main-x11.c" files,
 * in particular, the "file handling" functions may not work on all
 * systems.
 *
 * When you complete a port to a new system, you should email any
 * newly created files, and any changes made to existing files,
 * including "h-config.h", "config.h", and any of the "Makefile"
 * files, to "rr9@thangorodrim.net" for inclusion in the next version.
 *
 * Always choose a "three letter" naming scheme for "main-xxx.c"
 * and "Makefile.xxx" and such for consistency and simplicity.
 *
 *
 * Initial framework (and all code) by Ben Harrison (benh@phial.com).
 */


#ifdef USE_XXX

#include "main.h"

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data
{
	term t;

	/* Other fields if needed XXX XXX XXX */
};



/*
 * Number of "term_data" structures to support XXX XXX XXX
 *
 * You MUST support at least one "term_data" structure, and the
 * game will currently use up to eight "term_data" structures if
 * they are available.
 *
 * Actually, MAX_TERM_DATA is now defined as eight in 'defines.h'.
 *
 * If only one "term_data" structure is supported, then a lot of
 * the things that would normally go into a "term_data" structure
 * could be made into global variables instead.
 */
#define MAX_XXX_TERM 1


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_XXX_TERM];


#if 0

/*
 * Often, it is helpful to create an array of "color data" containing
 * a representation of the "angband_color_table" array in some "local" form.
 *
 * Often, the "Term_xtra(TERM_XTRA_REACT, 0)" hook is used to initialize
 * "color_data" from "angband_color_table".  XXX XXX XXX
 */
static local_color_data_type color_data[MAX_COLORS];

#endif



/*** Function hooks needed by "Term" ***/


/*
 * Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "z-term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_xxx(term *t)
{
	term_data *td = (term_data*)(t->data);

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
static void Term_nuke_xxx(term *t)
{
	term_data *td = (term_data*)(t->data);

	/* XXX XXX XXX */
}



/*
 * Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "z-term.c" package,
 * or by the application itself.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "z-term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_xtra_xxx(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	/* Analyze */
	switch (n)
	{
		case TERM_XTRA_EVENT:
		{
			/*
			 * Process some pending events XXX XXX XXX
			 *
			 * Wait for at least one event if "v" is non-zero
			 * otherwise, if no events are ready, return at once.
			 * When "keypress" events are encountered, the "ascii"
			 * value corresponding to the key should be sent to the
			 * "Term_keypress()" function.  Certain "bizarre" keys,
			 * such as function keys or arrow keys, may send special
			 * sequences of characters, such as control-underscore,
			 * plus letters corresponding to modifier keys, plus an
			 * underscore, plus carriage return, which can be used by
			 * the main program for "macro" triggers.  This action
			 * should handle as many events as is efficiently possible
			 * but is only required to handle a single event, and then
			 * only if one is ready or "v" is true.
			 *
			 * This action is required.
			 */

			return (0);
		}

		case TERM_XTRA_FLUSH:
		{
			/*
			 * Flush all pending events XXX XXX XXX
			 *
			 * This action should handle all events waiting on the
			 * queue, optionally discarding all "keypress" events,
			 * since they will be discarded anyway in "z-term.c".
			 *
			 * This action is required, but may not be "essential".
			 */

			return (0);
		}

		case TERM_XTRA_CLEAR:
		{
			/*
			 * Clear the entire window XXX XXX XXX
			 *
			 * This action should clear the entire window, and redraw
			 * any "borders" or other "graphic" aspects of the window.
			 *
			 * This action is required.
			 */

			return (0);
		}

		case TERM_XTRA_SHAPE:
		{
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

		case TERM_XTRA_FROSH:
		{
			/*
			 * Flush a row of output XXX XXX XXX
			 *
			 * This action should make sure that row "v" of the "output"
			 * to the window will actually appear on the window.
			 *
			 * This action is optional, assuming that "Term_text_xxx()"
			 * (and similar functions) draw directly to the screen, or
			 * that the "TERM_XTRA_FRESH" entry below takes care of any
			 * necessary flushing issues.
			 */

			return (0);
		}

		case TERM_XTRA_FRESH:
		{
			/*
			 * Flush output XXX XXX XXX
			 *
			 * This action should make sure that all "output" to the
			 * window will actually appear on the window.
			 *
			 * This action is optional, assuming that "Term_text_xxx()"
			 * (and similar functions) draw directly to the screen, or
			 * that the "TERM_XTRA_FROSH" entry above takes care of any
			 * necessary flushing issues.
			 */

			return (0);
		}

		case TERM_XTRA_NOISE:
		{
			/*
			 * Make a noise XXX XXX XXX
			 *
			 * This action should produce a "beep" noise.
			 *
			 * This action is optional, but convenient.
			 */

			return (0);
		}

		case TERM_XTRA_BORED:
		{
			/*
			 * Handle random events when bored XXX XXX XXX
			 *
			 * This action is optional, and normally not important
			 */

			return (0);
		}

		case TERM_XTRA_REACT:
		{
			/*
			 * React to global changes XXX XXX XXX
			 *
			 * For example, this action can be used to react to
			 * changes in the global "angband_color_table[MAX_COLORS][4]" array.
			 *
			 * This action is optional, but can be very useful for
			 * handling "color changes" and the "arg_sound" and/or
			 * "arg_graphics" options.
			 */

			return (0);
		}

		case TERM_XTRA_ALIVE:
		{
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

		case TERM_XTRA_LEVEL:
		{
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

		case TERM_XTRA_DELAY:
		{
			/*
			 * Delay for some milliseconds XXX XXX XXX
			 *
			 * This action is useful for proper "timing" of certain
			 * visual effects, such as breath attacks.
			 *
			 * This action is optional, but may be required by this file,
			 * especially if special "macro sequences" must be supported.
			 */

			return (0);
		}
	}

	/* Unknown or Unhandled action */
	return (1);
}


/*
 * Display the cursor
 *
 * This routine should display the cursor at the given location
 * (x,y) in some manner.  On some machines this involves actually
 * moving the physical cursor, on others it involves drawing a fake
 * cursor in some form of graphics mode.  Note the "soft_cursor"
 * flag which tells "z-term.c" to treat the "cursor" as a "visual"
 * thing and not as a "hardware" cursor.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You may use the "Term_what(x, y, &a, &c)" function, if needed,
 * to determine what attr/char should be "under" the new cursor,
 * for "inverting" purposes or whatever.
 */
static errr Term_curs_xxx(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

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
static errr Term_wipe_xxx(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}

/*
 * Given a position in the ISO Latin-1 character set, return
 * the correct character on this system.
 */
 static byte Term_xchar_xxx(byte c)
{
 	/* The xxx port uses the Latin-1 standard */
 	return (c);
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
 * in "color_data[a & BASIC_COLORS]", if needed, before drawing anything.
 *
 * You may ignore the "attribute" if you are only supporting a
 * monochrome environment, since this routine is normally never
 * called to display "black" (invisible) text, including the
 * default "spaces", and all other colors should be drawn in
 * the "normal" color in a monochrome environment.
 *
 * Note that if you have changed the "attr_blank" to something
 * which is not black, then this function must be able to draw
 * the resulting "blank" correctly.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_xxx(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
	return (0);
}


/*
 * Draw some attr/char pairs on the screen
 *
 * This routine should display the given "n" attr/char pairs at
 * the given location (x,y).  This function is only used if one
 * of the flags "always_pict" or "higher_pict" is defined.
 *
 * You must be sure that the attr/char pairs, when displayed, will
 * erase anything (including any visual cursor) that used to be at
 * the given location.  On many machines this is automatic, but on
 * others, you must first call "Term_wipe_xxx(x, y, 1)".
 *
 * With the "higher_pict" flag, this function can be used to allow
 * the display of "pseudo-graphic" pictures, for example, by using
 * the attr/char pair as an encoded index into a pixmap of special
 * "pictures".
 *
 * With the "always_pict" flag, this function can be used to force
 * every attr/char pair to be drawn by this function, which can be
 * very useful if this file can optimize its own display calls.
 *
 * This function is often associated with the "arg_graphics" flag.
 *
 * This function is only used if one of the "higher_pict" and/or
 * "always_pict" flags are set.
 */
static errr Term_pict_xxx(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);

	/* XXX XXX XXX */

	/* Success */
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
	term_init(t, 80, 24, 256);

	/* Choose "soft" or "hard" cursor XXX XXX XXX */
	/* A "soft" cursor must be explicitly "drawn" by the program */
	/* while a "hard" cursor has some "physical" existance and is */
	/* moved whenever text is drawn on the screen.  See "z-term.c". */
	/* t->soft_cursor = TRUE; */

	/* Avoid the "corner" of the window XXX XXX XXX */
	/* t->icky_corner = TRUE; */

	/* Use "Term_pict()" for all attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* t->always_pict = TRUE; */

	/* Use "Term_pict()" for some attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* t->higher_pict = TRUE; */

	/* Use "Term_text()" even for "black" text XXX XXX XXX */
	/* See the "Term_text_xxx()" function above. */
	/* t->always_text = TRUE; */

	/* Ignore the "TERM_XTRA_BORED" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	/* t->never_bored = TRUE; */

	/* Ignore the "TERM_XTRA_FROSH" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	/* t->never_frosh = TRUE; */

	/* Erase with "white space" XXX XXX XXX */
	/* t->attr_blank = TERM_WHITE; */
	/* t->char_blank = ' '; */

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_xxx;
	t->nuke_hook = Term_nuke_xxx;

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_xxx;
	t->curs_hook = Term_curs_xxx;
	t->wipe_hook = Term_wipe_xxx;
	t->text_hook = Term_text_xxx;
	t->pict_hook = Term_pict_xxx;
	t->xchar_hook = Term_xchar_xxx;

	/* Remember where we came from */
	t->data = td;

	/* Activate it */
	Term_activate(t);

	/* Global pointer */
	angband_term[i] = t;
}


/*
 * Help message.
 *   1st line = max 68 chars.
 *   Start next lines with 11 spaces
 */
const char help_xxx[] = "Describe XXX, subopts -describe suboptions here";


/*
 * Initialization function
 */
errr init_xxx(int argc, char **argv)
{
	int i;

	/* Initialize globals XXX XXX XXX */

	/* Initialize "term_data" structures XXX XXX XXX */

	/* Initialize the "color_data" array XXX XXX XXX */

	/* Create windows (backwards!) */
	for (i = MAX_XXX_TERM; i-- > 0; )
	{
		/* Link */
		term_data_link(i);
	}

	/* Success */
	return (0);
}


#ifdef INTERNAL_MAIN


/*
 * Some special machines need their own "main()" function, which they
 * can provide here, making sure NOT to compile the "main.c" file.
 *
 * These systems usually have some form of "event loop", run forever
 * as the last step of "main()", which handles things like menus and
 * window movement, and calls "play_game(FALSE)" to load a game after
 * initializing "savefile" to a filename, or "play_game(TRUE)" to make
 * a new game.  The event loop would also be triggered by "Term_xtra()"
 * (the TERM_XTRA_EVENT action), in which case the event loop would not
 * actually "loop", but would run once and return.
 */


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
static bool CheckEvents(bool wait)
{
	/* XXX XXX XXX */

	return (0);
}



/*
 * Make a sound.
 *
 * This action should produce sound number "v", where the
 * "name" of that sound is "sound_names[v]".
 *
 * This action is optional, and not very important.
 */
static void xxx_sound(int v)
{
	return;
}


/*
 * Init some stuff
 *
 * This function is used to keep the "path" variable off the stack.
 */
static void init_stuff(void)
{
	char path[1024];

	/* Prepare the path XXX XXX XXX */
	/* This must in some way prepare the "path" variable */
	/* so that it points at the "lib" directory.  Every */
	/* machine handles this in a different way... */
	my_strcpy(path, "XXX XXX XXX", sizeof(path));

	/* Prepare the filepaths */
	init_file_paths(path, path, path);

	/* Set up sound hook */
	sound_hook = xxx_sound;
}


/*
 * Main function
 *
 * This function must do a lot of stuff.
 */
int main(int argc, char *argv[])
{
	/* Initialize the machine itself XXX XXX XXX */

	/* Process command line arguments XXX XXX XXX */

	/* Initialize the windows */
	if (init_xxx(argc, argv) != 0) quit("Oops!");

	/* XXX XXX XXX */
	ANGBAND_SYS = "xxx";

	/* Initialize some stuff */
	init_stuff();

	/* Initialize */
	init_angband();

	/* Allow auto-startup XXX XXX XXX */

	/* Event loop forever XXX XXX XXX */
	while (TRUE) CheckEvents(TRUE);
}


#endif /* INTERNAL_MAIN */


#endif /* USE_XXX */
