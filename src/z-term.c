/* File: z-term.c */

/* Purpose: a generic, efficient, terminal window package -BEN- */

#include "z-term.h"

#include "z-virt.h"


/*
 * This file provides a generic, efficient, terminal window package,
 * which can be used not only on standard terminal environments such
 * as dumb terminals connected to a Unix box, but also in more modern
 * "graphic" environments, such as the Macintosh or Unix/X11.
 *
 * Each "window" works like a standard "dumb terminal", that is, it
 * can display colored textual symbols, plus an optional cursor, and
 * it can be used to get keypress events from the user.
 *
 * In fact, this package can simply be used, if desired, to support
 * programs which will look the same on a dumb terminal as they do
 * on a graphic platform such as the Macintosh.
 *
 * This package was designed to help port the game "Angband" to a wide
 * variety of different platforms.  Angband, like many other games in
 * the "rogue-like" heirarchy, requires, at the minimum, the ability
 * to display "colored textual symbols" in a standard 80x24 "window",
 * such as that provided by most dumb terminals, and many old personal
 * computers, and to check for "keypresses" from the user.  The major
 * concerns were thus portability and efficiency, so Angband could be
 * easily ported to many different systems, with minimal effort, and
 * yet would run quickly on each of these systems, no matter what kind
 * of underlying hardware/software support was being used.
 *
 * It is important to understand the differences between the older
 * "dumb terminals" and the newer "graphic interface" machines, since
 * this package was designed to work with both types of systems.
 *
 * New machines:
 *   waiting for a keypress is complex
 *   checking for a keypress is often cheap
 *   changing "colors" may be expensive
 *   the "color" of a "blank" is rarely important
 *   moving the "cursor" is relatively cheap
 *   use a "software" cursor (only moves when requested)
 *   drawing new symbols does not erase old symbols
 *   drawing characters may or may not first erase behind them
 *   drawing a character on the cursor will clear the cursor
 *   may have fast routines for "clear a region"
 *   the bottom right corner is usually not special
 *
 * Old machines:
 *   waiting for a keypress is simple
 *   checking for a keypress is often expensive
 *   changing "colors" is usually cheap
 *   the "color" of a "blank" may be important
 *   moving the "cursor" may be expensive
 *   use a "hardware" cursor (moves during screen updates)
 *   drawing new symbols automatically erases old ones
 *   drawing a character on the cursor will move the cursor
 *   may have fast routines for "clear entire window"
 *   may have fast routines for "clear to end of line"
 *   the bottom right corner is often dangerous
 *
 *
 * This package provides support for multiple windows, each of an
 * arbitrary size (up to 255x255), each with its own set of flags,
 * and its own hooks to handle several low-level procedures which
 * differ from platform to platform.  Then the main program simply
 * creates one or more "term" structures, setting the various flags
 * and hooks in a manner appropriate for the current platform, and
 * then it can use the various "term" structures without worrying
 * about the underlying platform.
 *
 * The game "Angband" uses a set of files called "main-xxx.c", for
 * various "xxx" suffixes.  Most of these contain a function called
 * "init_xxx()", that will prepare the underlying visual system for
 * use with Angband, and then create one or more "term" structures,
 * using flags and hooks appropriate to the given platform, so that
 * the "main()" function can call one (or more) of the "init_xxx()"
 * functions, as appropriate, to prepare the required "term_screen"
 * (and the optional "term_mirror", "term_choice", "term_recall")
 * pointers to "term" structures.  Other "main-xxx.c" systems contain
 * their own "main()" function which, in addition to doing everything
 * needed to initialize the actual program, also does everything that
 * the normal "init_xxx()" functions would do.
 *
 *
 * This package allows each "grid" in each window to hold an attr/char
 * pair, with each ranging from 0 to 255, and makes very few assumptions
 * about the meaning of any attr/char values.  Normally, we assume that
 * "attr 0" is "black", with the semantics that "black" text should be
 * sent to "Term_wipe()" instead of "Term_text()", but this behavior is
 * disabled if either the "always_pict" or the "always_text" flags are
 * set.  Normally, we assume that "char 0" is "illegal", since placing
 * a "char 0" in the middle of a string "terminates" the string, and
 * this behavior cannot be easily disabled.  Finally, we use a special
 * attr/char pair, defaulting to "attr 0" and "char 32", also known as
 * "black space", when we "erase" or "clear" any window, but this pair
 * can be redefined to any pair, including the standard "white space",
 * or the bizarre "emptiness" ("attr 0" and "char 0"), which is the
 * only way to place "char 0" codes in the "window" arrays.
 *
 * The game "Angband" defines, in addition to "attr 0", all of the
 * attr codes from 1 to 15, using definitions in "defines.h", and
 * thus the "main-xxx.c" files used by Angband must handle these
 * attr values correctly.  Also, they must handle all other attr
 * values, though they may do so in any way they wish.  Many of
 * the "main-xxx.c" files use "white space" ("attr 1" / "char 32")
 * to "erase" or "clear" any window, for efficiency.
 *
 *
 * This package provides several functions which allow a program to
 * interact with the "term" structures.  Most of the functions allow
 * the program to "request" certain changes to the current "term",
 * such as moving the cursor, drawing an attr/char pair, erasing a
 * region of grids, hiding the cursor, etc.  Then there is a special
 * function which causes all of the "pending" requests to be performed
 * in an efficient manner.  There is another set of functions which
 * allow the program to query the "requested state" of the current
 * "term", such as asking for the cursor location, or what attr/char
 * is at a given location, etc.  There is another set of functions
 * dealing with "keypress" events, which allows the program to ask if
 * the user has pressed any keys, or to forget any keys the user pressed.
 * There is a pair of functions to allow this package to memorize the
 * contents of the current "term", and to restore these contents at
 * a later time.  There is a special function which allows the program
 * to specify which "term" structure should be the "current" one.  At
 * the lowest level, there is a set of functions which allow a new
 * "term" to be initialized or destroyed, and which allow this package,
 * or a program, to access the special "hooks" defined for the current
 * "term", and a set of functions which those "hooks" can use to inform
 * this package of the results of certain occurances, for example, one
 * such function allows this package to learn about user keypresses,
 * detected by one of the special "hooks".
 *
 * We provide, among other things, the functions "Term_keypress()"
 * to "react" to keypress events, and "Term_redraw()" to redraw the
 * entire window, plus "Term_resize()" to note a new size.
 *
 *
 * Note that the current "term" contains two "window images".  One of
 * these images represents the "requested" contents of the "term", and
 * the other represents the "actual" contents of the "term", at the time
 * of the last performance of pending requests.  This package uses these
 * two images to determine the "minimal" amount of work needed to make
 * the "actual" contents of the "term" match the "requested" contents of
 * the "term".  This method is not perfect, but it often reduces the
 * amount of work needed to perform the pending requests, which thus
 * increases the speed of the program itself.  This package promises
 * that the requested changes will appear to occur either "all at once"
 * or in a "top to bottom" order.  In addition, a "cursor" is maintained,
 * and this cursor is updated along with the actual window contents.
 *
 * Currently, the "Term_fresh()" routine attempts to perform the "minimum"
 * number of physical updates, in terms of total "work" done by the hooks
 * Term_wipe(), Term_text(), and Term_pict(), making use of the fact that
 * adjacent characters of the same color can both be drawn together using
 * the "Term_text()" hook, and that "black" text can often be sent to the
 * "Term_wipe()" hook instead of the "Term_text()" hook, and if something
 * is already displayed in a window, then it is not necessary to display
 * it again.  Unfortunately, this may induce slightly non-optimal results
 * in some cases, in particular, those in which, say, a string of ten
 * characters needs to be written, but the fifth character has already
 * been displayed.  Currently, this will cause the "Term_text()" routine
 * to be called once for each half of the string, instead of once for the
 * whole string, which, on some machines, may be non-optimal behavior.
 *
 *
 * Several "flags" are available in each "term" to allow the underlying
 * visual system (which initializes the "term" structure) to "optimize"
 * the performance of this package for the given system, or to request
 * certain behavior which is helpful/required for the given system.
 *
 * The "soft_cursor" flag indicates the use of a "soft" cursor, which
 * only moves when explicitly requested,and which is "erased" when
 * any characters are drawn on top of it.  This flag is used for all
 * "graphic" systems which handle the cursor by "drawing" it.
 *
 * The "icky_corner" flag indicates that the bottom right "corner"
 * of the windows are "icky", and "printing" anything there may
 * induce "messy" behavior, such as "scrolling".  This flag is used
 * for most old "dumb terminal" systems.
 *
 *
 * The "term" structure contains the following function "hooks":
 *
 *   Term->init_hook = Init the term
 *   Term->nuke_hook = Nuke the term
 *   Term->user_hook = Perform user actions
 *   Term->xtra_hook = Perform extra actions
 *   Term->wipe_hook = Draw some blank spaces
 *   Term->curs_hook = Draw (or Move) the cursor
 *   Term->pict_hook = Draw a picture in the window
 *   Term->text_hook = Draw some text in the window
 *
 * The "Term_user()" and "Term_xtra()" functions provide access to
 * the first two of these hooks for the main program, but note that
 * the behavior of "Term_xtra()" is not always defined when called
 * from outside this file, and the behavior of "Term_user()" is only
 * defined by the application.
 *
 *
 * The new formalism includes a "displayed" screen image (old) which
 * is actually seen by the user, a "requested" screen image (scr)
 * which is being prepared for display, a "memorized" screen image
 * (mem) which is used to save and restore screen images, and a
 * "temporary" screen image (tmp) which is currently unused.
 */






/*
 * The current "term"
 */
term *Term = NULL;




/*** Local routines ***/


/*
 * Nuke a term_win (see below)
 */
static errr term_win_nuke(term_win *s, int w, int h)
{
	/* Free the window access arrays */
	C_KILL(s->a, h, byte*);
	C_KILL(s->c, h, char*);

	/* Free the window content arrays */
	C_KILL(s->va, h * w, byte);
	C_KILL(s->vc, h * w, char);

	/* Success */
	return (0);
}


/*
 * Initialize a "term_win" (using the given window size)
 */
static errr term_win_init(term_win *s, int w, int h)
{
	int y;

	/* Make the window access arrays */
	C_MAKE(s->a, h, byte*);
	C_MAKE(s->c, h, char*);

	/* Make the window content arrays */
	C_MAKE(s->va, h * w, byte);
	C_MAKE(s->vc, h * w, char);

	/* Prepare the window access arrays */
	for (y = 0; y < h; y++)
	{
		s->a[y] = s->va + w * y;
		s->c[y] = s->vc + w * y;
	}

	/* Success */
	return (0);
}


/*
 * Copy a "term_win" from another
 */
static errr term_win_copy(term_win *s, term_win *f, int w, int h)
{
	int x, y;

	/* Copy contents */
	for (y = 0; y < h; y++)
	{
		byte *f_aa = f->a[y];
		char *f_cc = f->c[y];

		byte *s_aa = s->a[y];
		char *s_cc = s->c[y];

		for (x = 0; x < w; x++)
		{
			*s_aa++ = *f_aa++;
			*s_cc++ = *f_cc++;
		}
	}

	/* Copy cursor */
	s->cx = f->cx;
	s->cy = f->cy;
	s->cu = f->cu;
	s->cv = f->cv;

	/* Success */
	return (0);
}



/*** Local routines ***/


/*
 * Mentally draw an attr/char at a given location
 *
 * Called only from "Term_draw()" and "Term_putch()"
 *
 * Assumes given location and values are valid.
 */
static void QueueAttrChar(int x, int y, byte a, char c)
{
	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	int oa = scr_aa[x];
	int oc = scr_cc[x];

	/* Hack -- Ignore non-changes */
	if ((oa == a) && (oc == c)) return;

	/* Save the "literal" information */
	scr_aa[x] = a;
	scr_cc[x] = c;

	/* Check for new min/max row info */
	if (y < Term->y1) Term->y1 = y;
	if (y > Term->y2) Term->y2 = y;

	/* Check for new min/max col info for this row */
	if (x < Term->x1[y]) Term->x1[y] = x;
	if (x > Term->x2[y]) Term->x2[y] = x;
}


/*
 * Mentally draw some attr/chars at a given location
 *
 * Called only from "Term_addstr()"
 *
 * Assumes that (x,y) is a valid location, that the first "n" characters
 * of the string "s" are all valid (non-zero), and that (x+n-1,y) is also
 * a valid location, so the first "n" characters of "s" can all be added
 * starting at (x,y) without causing any illegal operations.
 */
static void QueueAttrChars(int x, int y, int n, byte a, cptr s)
{
	int x1 = -1, x2 = -1;

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* Queue the attr/chars */
	for ( ; n; x++, s++, n--)
	{
		int oa = scr_aa[x];
		int oc = scr_cc[x];

		/* Hack -- Ignore non-changes */
		if ((oa == a) && (oc == *s)) continue;

		/* Save the "literal" information */
		scr_aa[x] = a;
		scr_cc[x] = *s;

		/* Note the "range" of window updates */
		if (x1 < 0) x1 = x;
		x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0)
	{
		/* Check for new min/max row info */
		if (y < Term->y1) Term->y1 = y;
		if (y > Term->y2) Term->y2 = y;

		/* Check for new min/max col info in this row */
		if (x1 < Term->x1[y]) Term->x1[y] = x1;
		if (x2 > Term->x2[y]) Term->x2[y] = x2;
	}
}




/*** External hooks ***/


/*
 * Perform the "user action" of type "n".
 */
errr Term_user(int n)
{
	if (!Term->user_hook) return (-1);
	return ((*Term->user_hook)(n));
}

/*
 * Perform the "extra action" of type "n" with value "v".
 * Valid actions are defined as the "TERM_XTRA_*" constants.
 * Invalid and non-handled actions should return an error code.
 * This function is available for external usage, though some
 * parameters may not make sense unless called from "term.c".
 */
errr Term_xtra(int n, int v)
{
	if (!Term->xtra_hook) return (-1);
	return ((*Term->xtra_hook)(n, v));
}



/*** Fake hooks ***/


/*
 * Hack -- fake hook for "Term_curs()"
 * Place a "cursor" at "(x,y)".
 */
static errr Term_curs_hack(int x, int y)
{
	/* XXX XXX XXX */
	if (x || y) return (-2);

	/* Oops */
	return (-1);
}

/*
 * Hack -- fake hook for "Term_wipe()"
 * Erase "n" characters starting at "(x,y)"
 */
static errr Term_wipe_hack(int x, int y, int n)
{
	/* XXX XXX XXX */
	if (x || y || n) return (-2);

	/* Oops */
	return (-1);
}

/*
 * Hack -- fake hook for "Term_pict()"
 * Draw a "special" attr/char pair at "(x,y)".
 */
static errr Term_pict_hack(int x, int y, byte a, char c)
{
	/* XXX XXX XXX */
	if (x || y || a || c) return (-2);

	/* Oops */
	return (-1);
}

/*
 * Hack -- fake hook for "Term_text()"
 * Draw "n" chars from the string "s" using attr "a", at location "(x,y)".
 */
static errr Term_text_hack(int x, int y, int n, byte a, cptr s)
{
	/* XXX XXX XXX */
	if (x || y || n || a || s) return (-2);

	/* Oops */
	return (-1);
}



/*** Refresh routines ***/


/*
 * Flush a row of the current window using "Term_text()"
 *
 * Method: collect similar adjacent entries into stripes
 *
 * This routine currently "skips" any locations which "appear"
 * to already contain the desired contents.  This may or may
 * not be the best method, especially when the desired content
 * fits nicely into a "strip" currently under construction...
 *
 * Currently, this function skips right over any characters which
 * are already present on the window.  I imagine that it would be
 * more efficient to NOT skip such characters ALL the time, but
 * only when "useful".
 *
 * Might be better to go ahead and queue them while allowed, but
 * keep a count of the "trailing skipables", then, when time to
 * flush, or when a "non skippable" is found, force a flush if
 * there are too many skippables.  Hmmm...
 *
 * Perhaps an "initialization" stage, where the "text" (and "attr")
 * buffers are "filled" with information, converting "blanks" into
 * a convenient representation, and marking "skips" with "zero chars",
 * and then some "processing" is done to determine which chars to skip.
 *
 * Currently, this function is optimal for systems which prefer to
 * "print a char + move a char + print a char" to "print three chars",
 * and for applications that do a lot of "detailed" color printing.
 *
 * Note that, in QueueAttrChar(s), total "non-changes" are "pre-skipped".
 * But this routine must also handle situations in which the contents of
 * a location are changed, but then changed back to the original value,
 * and situations in which two locations in the same row are changed,
 * but the locations between them are unchanged.
 *
 * Note that "trailing spaces" are assumed to have the color of the
 * text that is currently being used, which in some cases allows the
 * spaces to be drawn somewhat efficiently.
 *
 * Note that special versions of this function (see below) are used if
 * either the "Term->always_pict" or "Term->higher_pict" flags are set.
 *
 * Normally, the "Term_wipe()" function is used only to display "blanks"
 * that were induced by "Term_clear()" or "Term_erase()", and then only
 * if the "attr_blank" and "char_blank" fields have not been redefined
 * to use "white space" instead of the default "black space".  Also,
 * the "Term_wipe()" function is used to display "black" text, though
 * why anyone would draw "black" text is beyond me.  Note that the
 * "Term->always_text" flag will disable the use of the "Term_wipe()"
 * function hook entirely.
 */
static void Term_fresh_row_text_wipe(int y)
{
	int x;

	byte *old_aa = Term->old->a[y];
	char *old_cc = Term->old->c[y];

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* No chars "pending" in "text" */
	int n = 0;

	/* Pending text starts in the first column */
	int fx = 0;

	/* Pending text color is "blank" */
	int fa = Term->attr_blank;

	/* The "old" data */
	int oa, oc;

	/* The "new" data */
	int na, nc;

	/* Max width is 255 */
	char text[256];


	/* Scan the columns marked as "modified" */
	for (x = Term->x1[y]; x <= Term->x2[y]; x++)
	{
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* Save and remember the new contents */
		na = old_aa[x] = scr_aa[x];
		nc = old_cc[x] = scr_cc[x];

		/* Notice unchanged areas */
		if ((na == oa) && (nc == oc))
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				if (fa)
				{
					(void)((*Term->text_hook)(fx, y, n, fa, text));
				}

				/* Hack -- Erase "leading" spaces */
				else
				{
					(void)((*Term->wipe_hook)(fx, y, n));
				}

				/* Forget the pending thread */
				n = 0;
			}

			/* Skip */
			continue;
		}

		/* Notice new color */
		if (fa != na)
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				if (fa)
				{
					(void)((*Term->text_hook)(fx, y, n, fa, text));
				}

				/* Hack -- Erase "leading" spaces */
				else
				{
					(void)((*Term->wipe_hook)(fx, y, n));
				}

				/* Forget the pending thread */
				n = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Start a new thread, if needed */
		if (!n) fx = x;

		/* Expand the current thread */
		text[n++] = nc;
	}

	/* Flush the pending thread, if any */
	if (n)
	{
		/* Terminate the thread */
		text[n] = '\0';

		/* Draw the pending chars */
		if (fa)
		{
			(void)((*Term->text_hook)(fx, y, n, fa, text));
		}

		/* Hack -- Erase fully blank lines */
		else
		{
			(void)((*Term->wipe_hook)(fx, y, n));
		}
	}
}


/*
 * Like "Term_fresh_row_text_wipe" but always use "Term_text()"
 * instead of "Term_wipe()" even for "black" (invisible) text.
 */
static void Term_fresh_row_text_text(int y)
{
	int x;

	byte *old_aa = Term->old->a[y];
	char *old_cc = Term->old->c[y];

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* No chars "pending" in "text" */
	int n = 0;

	/* Pending text starts in the first column */
	int fx = 0;

	/* Pending text color is "blank" */
	int fa = Term->attr_blank;

	/* The "old" data */
	int oa, oc;

	/* The "new" data */
	int na, nc;

	/* Max width is 255 */
	char text[256];


	/* Scan the columns marked as "modified" */
	for (x = Term->x1[y]; x <= Term->x2[y]; x++)
	{
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* Save and remember the new contents */
		na = old_aa[x] = scr_aa[x];
		nc = old_cc[x] = scr_cc[x];

		/* Notice unchanged areas */
		if ((na == oa) && (nc == oc))
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				(void)((*Term->text_hook)(fx, y, n, fa, text));

				/* Forget the pending thread */
				n = 0;
			}

			/* Skip */
			continue;
		}

		/* Notice new color */
		if (fa != na)
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				(void)((*Term->text_hook)(fx, y, n, fa, text));

				/* Forget the pending thread */
				n = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Start a new thread, if needed */
		if (!n) fx = x;

		/* Expand the current thread */
		text[n++] = nc;
	}

	/* Flush the pending thread, if any */
	if (n)
	{
		/* Terminate the thread */
		text[n] = '\0';

		/* Draw the pending chars */
		(void)((*Term->text_hook)(fx, y, n, fa, text));
	}
}


/*
 * As above, but use "Term_pict()" instead of "Term_text()" for
 * any attr/char pairs with the high-bits set.
 */
static void Term_fresh_row_both_wipe(int y)
{
	int x;

	byte *old_aa = Term->old->a[y];
	char *old_cc = Term->old->c[y];

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* No chars "pending" in "text" */
	int n = 0;

	/* Pending text starts in the first column */
	int fx = 0;

	/* Pending text color is "blank" */
	int fa = Term->attr_blank;

	/* The "old" data */
	int oa, oc;

	/* The "new" data */
	int na, nc;

	/* Max width is 255 */
	char text[256];


	/* Scan the columns marked as "modified" */
	for (x = Term->x1[y]; x <= Term->x2[y]; x++)
	{
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* Save and remember the new contents */
		na = old_aa[x] = scr_aa[x];
		nc = old_cc[x] = scr_cc[x];

		/* Notice unchanged areas */
		if ((na == oa) && (nc == oc))
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				if (fa)
				{
					(void)((*Term->text_hook)(fx, y, n, fa, text));
				}

				/* Hack -- Erase "leading" spaces */
				else
				{
					(void)((*Term->wipe_hook)(fx, y, n));
				}

				/* Forget the pending thread */
				n = 0;
			}

			/* Skip */
			continue;
		}

		/* Use "Term_pict" for "special" data */
		if ((na & 0x80) && (nc & 0x80))
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				if (fa)
				{
					(void)((*Term->text_hook)(fx, y, n, fa, text));
				}

				/* Hack -- Erase "leading" spaces */
				else
				{
					(void)((*Term->wipe_hook)(fx, y, n));
				}

				/* Forget the pending thread */
				n = 0;
			}

			/* Display this special character */
			(void)((*Term->pict_hook)(x, y, na, nc));

			/* Skip */
			continue;
		}

		/* Notice new color */
		if (fa != na)
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				if (fa)
				{
					(void)((*Term->text_hook)(fx, y, n, fa, text));
				}

				/* Hack -- Erase "leading" spaces */
				else
				{
					(void)((*Term->wipe_hook)(fx, y, n));
				}

				/* Forget the pending thread */
				n = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Start a new thread, if needed */
		if (!n) fx = x;

		/* Expand the current thread */
		text[n++] = nc;
	}

	/* Flush the pending thread, if any */
	if (n)
	{
		/* Terminate the thread */
		text[n] = '\0';

		/* Draw the pending chars */
		if (fa)
		{
			(void)((*Term->text_hook)(fx, y, n, fa, text));
		}

		/* Hack -- Erase fully blank lines */
		else
		{
			(void)((*Term->wipe_hook)(fx, y, n));
		}
	}
}


/*
 * Like "Term_fresh_row_both_wipe()", but always use "Term_text()"
 * instead of "Term_wipe()", even for "black" (invisible) text.
 */
static void Term_fresh_row_both_text(int y)
{
	int x;

	byte *old_aa = Term->old->a[y];
	char *old_cc = Term->old->c[y];

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* No chars "pending" in "text" */
	int n = 0;

	/* Pending text starts in the first column */
	int fx = 0;

	/* Pending text color is "blank" */
	int fa = Term->attr_blank;

	/* The "old" data */
	int oa, oc;

	/* The "new" data */
	int na, nc;

	/* Max width is 255 */
	char text[256];


	/* Scan the columns marked as "modified" */
	for (x = Term->x1[y]; x <= Term->x2[y]; x++)
	{
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* Save and remember the new contents */
		na = old_aa[x] = scr_aa[x];
		nc = old_cc[x] = scr_cc[x];

		/* Notice unchanged areas */
		if ((na == oa) && (nc == oc))
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				(void)((*Term->text_hook)(fx, y, n, fa, text));

				/* Forget the pending thread */
				n = 0;
			}

			/* Skip */
			continue;
		}

		/* Use "Term_pict" for "special" data */
		if ((na & 0x80) && (nc & 0x80))
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				(void)((*Term->text_hook)(fx, y, n, fa, text));

				/* Forget the pending thread */
				n = 0;
			}

			/* Display this special character */
			(void)((*Term->pict_hook)(x, y, na, nc));

			/* Skip */
			continue;
		}

		/* Notice new color */
		if (fa != na)
		{
			/* Flush as needed (see above) */
			if (n)
			{
				/* Terminate the thread */
				text[n] = '\0';

				/* Draw the pending chars */
				(void)((*Term->text_hook)(fx, y, n, fa, text));

				/* Forget the pending thread */
				n = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Start a new thread, if needed */
		if (!n) fx = x;

		/* Expand the current thread */
		text[n++] = nc;
	}

	/* Flush the pending thread, if any */
	if (n)
	{
		/* Terminate the thread */
		text[n] = '\0';

		/* Draw the pending chars */
		(void)((*Term->text_hook)(fx, y, n, fa, text));
	}
}


/*
 * Like "Term_fresh_row_text_wipe()", but use "Term_pict()" instead
 * of "Term_text()" or "Term_wipe()" for all "changed" data
 */
static void Term_fresh_row_pict(int y)
{
	int x;

	byte *old_aa = Term->old->a[y];
	char *old_cc = Term->old->c[y];

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* The "old" data */
	int oa, oc;

	/* The "new" data */
	int na, nc;


	/* Scan the columns marked as "modified" */
	for (x = Term->x1[y]; x <= Term->x2[y]; x++)
	{
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* Save and remember the new contents */
		na = old_aa[x] = scr_aa[x];
		nc = old_cc[x] = scr_cc[x];

		/* Ignore unchanged areas */
		if ((na == oa) && (nc == oc)) continue;

		/* Display this special character */
		(void)((*Term->pict_hook)(x, y, na, nc));
	}
}





/*
 * Actually perform all requested changes to the window
 *
 * Note that "Term_xtra(TERM_XTRA_FROSH,y)" will be always be called
 * after any row "y" has been "flushed", unless the "Term->never_frosh"
 * flag is set, and "Term_xtra(TERM_XTRA_FRESH,0)" will be called after
 * all of the rows have been "flushed".
 *
 * Note the use of five different functions to handle the actual flush,
 * depending on the various useful settings of the "Term->always_pict",
 * "Term->higher_pict", and "Term->always_text" flags.  This allows a
 * lot of conditional checks against these flags to be removed, and
 * allows us to support these flags without any speed loss.  It is
 * very possible that this degree of optimization is "overkill".
 *
 * This function does nothing unless the "Term" is "mapped", which allows
 * certain systems to optimize the handling of "closed" windows.
 *
 * On systems with a "soft" cursor, we must explicitly erase the cursor
 * before flushing the output, if needed, to prevent a "jumpy" refresh.
 * The actual method for this is horrible, but there is very little that
 * we can do to simplify it.  XXX XXX XXX
 *
 * On systems with a "hard" cursor, we will "hide" the cursor before
 * flushing the output, if needed, to avoid a "flickery" refresh.  It
 * would be nice to *always* hide the cursor during the refresh, but
 * this might be expensive (and/or ugly) on some machines.
 *
 * Note that if no "black" text is ever drawn, and if "attr_blank" is
 * not "zero", then the "Term_wipe" hook will never be used.
 *
 * The "Term->icky_corner" flag is used to avoid calling "Term_wipe()"
 * or "Term_pict()" or "Term_text()" on the bottom right corner of the
 * window, which might induce "scrolling" or other nasty stuff on old
 * dumb terminals.  This flag is handled very efficiently.  We assume
 * that the "Term_curs()" call will prevent placing the cursor in the
 * corner, if needed, though I doubt such placement is ever a problem.
 * Currently, the use of "Term->icky_corner" and "Term->soft_cursor"
 * together may result in undefined behavior.
 */
errr Term_fresh(void)
{
	int x, y;

	int w = Term->wid;
	int h = Term->hgt;

	term_win *old = Term->old;
	term_win *scr = Term->scr;


	/* Do nothing unless "mapped" */
	if (!Term->mapped_flag) return (1);


	/* Paranoia -- enforce "fake" hooks if needed */
	if (!Term->curs_hook) Term->curs_hook = Term_curs_hack;
	if (!Term->wipe_hook) Term->wipe_hook = Term_wipe_hack;
	if (!Term->pict_hook) Term->pict_hook = Term_pict_hack;
	if (!Term->text_hook) Term->text_hook = Term_text_hack;


	/* Cursor update -- Erase old Cursor */
	if (Term->soft_cursor)
	{
		bool okay = FALSE;

		/* Cursor has moved */
		if (old->cy != scr->cy) okay = TRUE;
		if (old->cx != scr->cx) okay = TRUE;

		/* Cursor is now offscreen/invisible */
		if (scr->cu || !scr->cv) okay = TRUE;

		/* Cursor was already offscreen/invisible */
		if (old->cu || !old->cv) okay = FALSE;

		/* Erase old cursor if it is "wrong" */
		if (okay)
		{
			int tx = old->cx;
			int ty = old->cy;

			byte *old_aa = old->a[ty];
			char *old_cc = old->c[ty];

			byte a = old_aa[tx];
			char c = old_cc[tx];

			/* Hack -- use "Term_pict()" always */
			if (Term->always_pict)
			{
				(void)((*Term->pict_hook)(tx, ty, a, c));
			}

			/* Hack -- use "Term_pict()" sometimes */
			else if (Term->higher_pict && (a & 0x80) && (c & 0x80))
			{
				(void)((*Term->pict_hook)(tx, ty, a, c));
			}

			/* Hack -- restore the actual character */
			else if (a || Term->always_text)
			{
				char buf[2];
				buf[0] = c;
				buf[1] = '\0';
				(void)((*Term->text_hook)(tx, ty, 1, a, buf));
			}

			/* Hack -- erase the grid */
			else
			{
				(void)((*Term->wipe_hook)(tx, ty, 1));
			}
		}
	}

	/* Cursor Update -- Erase old Cursor */
	else
	{
		/* The cursor is useless/invisible, hide it */
		if (scr->cu || !scr->cv)
		{
			/* Make the cursor invisible */
			Term_xtra(TERM_XTRA_SHAPE, 0);
		}
	}


	/* Handle "total erase" */
	if (Term->total_erase)
	{
		byte a = Term->attr_blank;
		char c = Term->char_blank;

		/* Physically erase the entire window */
		Term_xtra(TERM_XTRA_CLEAR, 0);

		/* Hack -- clear all "cursor" data XXX XXX XXX */
		old->cv = old->cu = old->cx = old->cy = 0;

		/* Wipe each row */
		for (y = 0; y < h; y++)
		{
			byte *aa = old->a[y];
			char *cc = old->c[y];

			/* Wipe each column */
			for (x = 0; x < w; x++)
			{
				/* Wipe each grid */
				*aa++ = a;
				*cc++ = c;
			}
		}

		/* Redraw every row */
		Term->y1 = 0;
		Term->y2 = h - 1;

		/* Redraw every column */
		for (y = 0; y < h; y++)
		{
			Term->x1[y] = 0;
			Term->x2[y] = w - 1;
		}
	}


	/* Something to update */
	if (Term->y1 <= Term->y2)
	{
		/* Handle "icky corner" */
		if (Term->icky_corner)
		{
			/* Avoid the corner */
			if (Term->y2 > h - 2)
			{
				/* Avoid the corner */
				if (Term->x2[h - 1] > w - 2)
				{
					/* Avoid the corner */
					Term->x2[h - 1] = w - 2;
				}
			}
		}


		/* Always use "Term_pict()" */
		if (Term->always_pict)
		{
			/* Scan the "modified" rows */
			for (y = Term->y1; y <= Term->y2; ++y)
			{
				/* Flush each "modified" row */
				if (Term->x1[y] <= Term->x2[y])
				{
					/* Flush the row */
					Term_fresh_row_pict(y);
				}

				/* This row is all done */
				Term->x1[y] = w;
				Term->x2[y] = 0;

				/* Hack -- Flush that row (if allowed) */
				if (!Term->never_frosh) Term_xtra(TERM_XTRA_FROSH, y);
			}
		}

		/* Sometimes use "Term_pict()" */
		else if (Term->higher_pict)
		{
			/* Never use "Term_wipe()" */
			if (Term->always_text)
			{
				/* Scan the "modified" rows */
				for (y = Term->y1; y <= Term->y2; ++y)
				{
					/* Flush each "modified" row */
					if (Term->x1[y] <= Term->x2[y])
					{
						/* Flush the row */
						Term_fresh_row_both_text(y);
					}

					/* This row is all done */
					Term->x1[y] = w;
					Term->x2[y] = 0;

					/* Hack -- Flush that row (if allowed) */
					if (!Term->never_frosh) Term_xtra(TERM_XTRA_FROSH, y);
				}
			}

			/* Sometimes use "Term_wipe()" */
			else
			{
				/* Scan the "modified" rows */
				for (y = Term->y1; y <= Term->y2; ++y)
				{
					/* Flush each "modified" row */
					if (Term->x1[y] <= Term->x2[y])
					{
						/* Flush the row */
						Term_fresh_row_both_wipe(y);
					}

					/* This row is all done */
					Term->x1[y] = w;
					Term->x2[y] = 0;

					/* Hack -- Flush that row (if allowed) */
					if (!Term->never_frosh) Term_xtra(TERM_XTRA_FROSH, y);
				}
			}
		}

		/* Never use "Term_pict()" */
		else
		{
			/* Never use "Term_wipe()" */
			if (Term->always_text)
			{
				/* Scan the "modified" rows */
				for (y = Term->y1; y <= Term->y2; ++y)
				{
					/* Flush each "modified" row */
					if (Term->x1[y] <= Term->x2[y])
					{
						/* Flush the row */
						Term_fresh_row_text_text(y);
					}

					/* This row is all done */
					Term->x1[y] = w;
					Term->x2[y] = 0;

					/* Hack -- Flush that row (if allowed) */
					if (!Term->never_frosh) Term_xtra(TERM_XTRA_FROSH, y);
				}
			}

			/* Sometimes use "Term_wipe()" */
			else
			{
				/* Scan the "modified" rows */
				for (y = Term->y1; y <= Term->y2; ++y)
				{
					/* Flush each "modified" row */
					if (Term->x1[y] <= Term->x2[y])
					{
						/* Flush the row */
						Term_fresh_row_text_wipe(y);
					}

					/* This row is all done */
					Term->x1[y] = w;
					Term->x2[y] = 0;

					/* Hack -- Flush that row (if allowed) */
					if (!Term->never_frosh) Term_xtra(TERM_XTRA_FROSH, y);
				}
			}
		}

		/* No rows are invalid */
		Term->y1 = h;
		Term->y2 = 0;
	}


	/* Cursor update -- Show new Cursor */
	if (Term->soft_cursor)
	{
		/* Draw the cursor */
		if (!scr->cu && scr->cv)
		{
			/* Call the cursor display routine */
			(void)((*Term->curs_hook)(scr->cx, scr->cy));
		}
	}

	/* Cursor Update -- Show new Cursor */
	else
	{
		/* The cursor is useless, hide it */
		if (scr->cu)
		{
			/* Paranoia -- Put the cursor NEAR where it belongs */
			(void)((*Term->curs_hook)(w - 1, scr->cy));

			/* Make the cursor invisible */
			/* Term_xtra(TERM_XTRA_SHAPE, 0); */
		}

		/* The cursor is invisible, hide it */
		else if (!scr->cv)
		{
			/* Paranoia -- Put the cursor where it belongs */
			(void)((*Term->curs_hook)(scr->cx, scr->cy));

			/* Make the cursor invisible */
			/* Term_xtra(TERM_XTRA_SHAPE, 0); */
		}

		/* The cursor is visible, display it correctly */
		else
		{
			/* Put the cursor where it belongs */
			(void)((*Term->curs_hook)(scr->cx, scr->cy));

			/* Make the cursor visible */
			Term_xtra(TERM_XTRA_SHAPE, 1);
		}
	}


	/* Save the "cursor state" */
	old->cu = scr->cu;
	old->cv = scr->cv;
	old->cx = scr->cx;
	old->cy = scr->cy;


	/* Forget "total erase" */
	Term->total_erase = FALSE;


	/* Actually flush the output */
	Term_xtra(TERM_XTRA_FRESH, 0);


	/* Success */
	return (0);
}



/*** Output routines ***/


/*
 * Set the cursor visibility
 */
errr Term_set_cursor(int v)
{
	/* Already done */
	if (Term->scr->cv == v) return (1);

	/* Change */
	Term->scr->cv = v;

	/* Success */
	return (0);
}


/*
 * Place the cursor at a given location
 *
 * Note -- "illegal" requests do not move the cursor.
 */
errr Term_gotoxy(int x, int y)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify */
	if ((x < 0) || (x >= w)) return (-1);
	if ((y < 0) || (y >= h)) return (-1);

	/* Remember the cursor */
	Term->scr->cx = x;
	Term->scr->cy = y;

	/* The cursor is not useless */
	Term->scr->cu = 0;

	/* Success */
	return (0);
}


/*
 * At a given location, place an attr/char
 * Do not change the cursor position
 * No visual changes until "Term_fresh()".
 */
errr Term_draw(int x, int y, byte a, char c)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify location */
	if ((x < 0) || (x >= w)) return (-1);
	if ((y < 0) || (y >= h)) return (-1);

	/* Paranoia -- illegal char */
	if (!c) return (-2);

	/* Queue it for later */
	QueueAttrChar(x, y, a, c);

	/* Success */
	return (0);
}


/*
 * Using the given attr, add the given char at the cursor.
 *
 * We return "-2" if the character is "illegal". XXX XXX
 *
 * We return "-1" if the cursor is currently unusable.
 *
 * We queue the given attr/char for display at the current
 * cursor location, and advance the cursor to the right,
 * marking it as unuable and returning "1" if it leaves
 * the screen, and otherwise returning "0".
 *
 * So when this function, or the following one, return a
 * positive value, future calls to either function will
 * return negative ones.
 */
errr Term_addch(byte a, char c)
{
	int w = Term->wid;

	/* Handle "unusable" cursor */
	if (Term->scr->cu) return (-1);

	/* Paranoia -- no illegal chars */
	if (!c) return (-2);

	/* Queue the given character for display */
	QueueAttrChar(Term->scr->cx, Term->scr->cy, a, c);

	/* Advance the cursor */
	Term->scr->cx++;

	/* Success */
	if (Term->scr->cx < w) return (0);

	/* Note "Useless" cursor */
	Term->scr->cu = 1;

	/* Note "Useless" cursor */
	return (1);
}


/*
 * At the current location, using an attr, add a string
 *
 * We also take a length "n", using negative values to imply
 * the largest possible value, and then we use the minimum of
 * this length and the "actual" length of the string as the
 * actual number of characters to attempt to display, never
 * displaying more characters than will actually fit, since
 * we do NOT attempt to "wrap" the cursor at the screen edge.
 *
 * We return "-1" if the cursor is currently unusable.
 * We return "N" if we were "only" able to write "N" chars,
 * even if all of the given characters fit on the screen,
 * and mark the cursor as unusable for future attempts.
 *
 * So when this function, or the preceding one, return a
 * positive value, future calls to either function will
 * return negative ones.
 */
errr Term_addstr(int n, byte a, cptr s)
{
	int k;

	int w = Term->wid;

	errr res = 0;

	/* Handle "unusable" cursor */
	if (Term->scr->cu) return (-1);

	/* Obtain maximal length */
	k = (n < 0) ? (w + 1) : n;

	/* Obtain the usable string length */
	for (n = 0; (n < k) && s[n]; n++) /* loop */;

	/* React to reaching the edge of the screen */
	if (Term->scr->cx + n >= w) res = n = w - Term->scr->cx;

	/* Queue the first "n" characters for display */
	QueueAttrChars(Term->scr->cx, Term->scr->cy, n, a, s);

	/* Advance the cursor */
	Term->scr->cx += n;

	/* Hack -- Notice "Useless" cursor */
	if (res) Term->scr->cu = 1;

	/* Success (usually) */
	return (res);
}


/*
 * Move to a location and, using an attr, add a char
 */
errr Term_putch(int x, int y, byte a, char c)
{
	errr res;

	/* Move first */
	if ((res = Term_gotoxy(x, y)) != 0) return (res);

	/* Then add the char */
	if ((res = Term_addch(a, c)) != 0) return (res);

	/* Success */
	return (0);
}


/*
 * Move to a location and, using an attr, add a string
 */
errr Term_putstr(int x, int y, int n, byte a, cptr s)
{
	errr res;

	/* Move first */
	if ((res = Term_gotoxy(x, y)) != 0) return (res);

	/* Then add the string */
	if ((res = Term_addstr(n, a, s)) != 0) return (res);

	/* Success */
	return (0);
}



/*
 * Place cursor at (x,y), and clear the next "n" chars
 */
errr Term_erase(int x, int y, int n)
{
	int i;

	int w = Term->wid;
	/* int h = Term->hgt; */

	int x1 = -1;
	int x2 = -1;

	int na = Term->attr_blank;
	int nc = Term->char_blank;

	byte *scr_aa;
	char *scr_cc;


	/* Place cursor */
	if (Term_gotoxy(x, y)) return (-1);

	/* Force legal size */
	if (x + n > w) n = w - x;

	/* Fast access */
	scr_aa = Term->scr->a[y];
	scr_cc = Term->scr->c[y];

	/* Scan every column */
	for (i = 0; i < n; i++, x++)
	{
		int oa = scr_aa[x];
		int oc = scr_cc[x];

		/* Hack -- Ignore "non-changes" */
		if ((oa == na) && (oc == nc)) continue;

		/* Save the "literal" information */
		scr_aa[x] = na;
		scr_cc[x] = nc;

		/* Track minumum changed column */
		if (x1 < 0) x1 = x;

		/* Track maximum changed column */
		x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0)
	{
		/* Check for new min/max row info */
		if (y < Term->y1) Term->y1 = y;
		if (y > Term->y2) Term->y2 = y;

		/* Check for new min/max col info in this row */
		if (x1 < Term->x1[y]) Term->x1[y] = x1;
		if (x2 > Term->x2[y]) Term->x2[y] = x2;
	}

	/* Success */
	return (0);
}


/*
 * Clear the entire window, and move to the top left corner
 *
 * Note the use of the special "total_erase" code
 */
errr Term_clear(void)
{
	int x, y;

	int w = Term->wid;
	int h = Term->hgt;

	byte a = Term->attr_blank;
	char c = Term->char_blank;

	/* Cursor usable */
	Term->scr->cu = 0;

	/* Cursor to the top left */
	Term->scr->cx = Term->scr->cy = 0;

	/* Wipe each row */
	for (y = 0; y < h; y++)
	{
		byte *scr_aa = Term->scr->a[y];
		char *scr_cc = Term->scr->c[y];

		/* Wipe each column */
		for (x = 0; x < w; x++)
		{
			scr_aa[x] = a;
			scr_cc[x] = c;
		}

		/* This row has changed */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Every row has changed */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Force "total erase" */
	Term->total_erase = TRUE;

	/* Success */
	return (0);
}





/*
 * Redraw (and refresh) the whole window.
 */
errr Term_redraw(void)
{
	/* Force "total erase" */
	Term->total_erase = TRUE;

	/* Hack -- Refresh */
	Term_fresh();

	/* Success */
	return (0);
}





/*** Access routines ***/


/*
 * Extract the cursor visibility
 */
errr Term_get_cursor(int *v)
{
	/* Extract visibility */
	(*v) = Term->scr->cv;

	/* Success */
	return (0);
}


/*
 * Extract the current window size
 */
errr Term_get_size(int *w, int *h)
{
	/* Access the cursor */
	(*w) = Term->wid;
	(*h) = Term->hgt;

	/* Success */
	return (0);
}


/*
 * Extract the current cursor location
 */
errr Term_locate(int *x, int *y)
{
	/* Access the cursor */
	(*x) = Term->scr->cx;
	(*y) = Term->scr->cy;

	/* Warn about "useless" cursor */
	if (Term->scr->cu) return (1);

	/* Success */
	return (0);
}


/*
 * At a given location, determine the "current" attr and char
 * Note that this refers to what will be on the window after the
 * next call to "Term_fresh()".  It may or may not already be there.
 */
errr Term_what(int x, int y, byte *a, char *c)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify location */
	if ((x < 0) || (x >= w)) return (-1);
	if ((y < 0) || (y >= h)) return (-1);

	/* Direct access */
	(*a) = Term->scr->a[y][x];
	(*c) = Term->scr->c[y][x];

	/* Success */
	return (0);
}



/*** Input routines ***/


/*
 * XXX XXX XXX Mega-Hack -- special "Term_inkey_hook" hook
 *
 * This special function hook basically acts as a replacement function
 * for both "Term_flush()" and "Term_inkey()", depending on whether the
 * first parameter is NULL or not, and is currently used only to allow
 * the "Borg" to bypass the "standard" keypress routines, and instead
 * use its own internal algorithms to "generate" keypress values.
 *
 * Note that this function hook can do anything it wants, including
 * in most cases simply doing exactly what is normally done by the
 * functions below, and in other cases, doing some of the same things
 * to check for "interuption" by the user, and replacing the rest of
 * the code with specialized "think about the world" code.
 *
 * Similar results could be obtained by "stealing" the "Term->xtra_hook"
 * hook, and doing special things for "TERM_XTRA_EVENT", "TERM_XTRA_FLUSH",
 * and "TERM_XTRA_BORED", but this method is a lot "cleaner", and gives
 * much better results when "profiling" the code.
 * 
 * See the "Borg" documentation for more details.
 */
errr (*Term_inkey_hook)(char *ch, bool wait, bool take) = NULL;


/*
 * Flush and forget the input
 */
errr Term_flush(void)
{
	/* XXX XXX XXX */
	if (Term_inkey_hook)
	{
		/* Special "Borg" hook (flush keys) */
		return ((*Term_inkey_hook)(NULL, 0, 0));
	}

	/* Hack -- Flush all events */
	Term_xtra(TERM_XTRA_FLUSH, 0);

	/* Forget all keypresses */
	Term->key_head = Term->key_tail = 0;

	/* Success */
	return (0);
}



/*
 * Add a keypress to the "queue"
 */
errr Term_keypress(int k)
{
	/* Hack -- Refuse to enqueue non-keys */
	if (!k) return (-1);

	/* Store the char, advance the queue */
	Term->key_queue[Term->key_head++] = k;

	/* Circular queue, handle wrap */
	if (Term->key_head == Term->key_size) Term->key_head = 0;

	/* Success (unless overflow) */
	if (Term->key_head != Term->key_tail) return (0);

#if 0
	/* Hack -- Forget the oldest key */
	if (++Term->key_tail == Term->key_size) Term->key_tail = 0;
#endif

	/* Problem */
	return (1);
}


/*
 * Add a keypress to the FRONT of the "queue"
 */
errr Term_key_push(int k)
{
	/* Hack -- Refuse to enqueue non-keys */
	if (!k) return (-1);

	/* Hack -- Overflow may induce circular queue */
	if (Term->key_tail == 0) Term->key_tail = Term->key_size;

	/* Back up, Store the char */
	Term->key_queue[--Term->key_tail] = k;

	/* Success (unless overflow) */
	if (Term->key_head != Term->key_tail) return (0);

#if 0
	/* Hack -- Forget the oldest key */
	if (++Term->key_tail == Term->key_size) Term->key_tail = 0;
#endif

	/* Problem */
	return (1);
}





/*
 * Check for a pending keypress on the key queue.
 *
 * Store the keypress, if any, in "ch", and return "0".
 * Otherwise store "zero" in "ch", and return "1".
 *
 * Wait for a keypress if "wait" is true.
 *
 * Remove the keypress if "take" is true.
 */
errr Term_inkey(char *ch, bool wait, bool take)
{
	/* Assume no key */
	(*ch) = '\0';

	/* XXX XXX XXX */
	if (Term_inkey_hook)
	{
		/* Special "Borg" hook (generate keys) */
		return ((*Term_inkey_hook)(ch, wait, take));
	}

	/* Hack -- get bored */
	if (!Term->never_bored)
	{
		/* Process random events */
		Term_xtra(TERM_XTRA_BORED, 0);
	}

	/* Wait */
	if (wait)
	{
		/* Process pending events while necessary */
		while (Term->key_head == Term->key_tail)
		{
			/* Process events (wait for one) */
			Term_xtra(TERM_XTRA_EVENT, TRUE);
		}
	}

	/* Do not Wait */
	else
	{
		/* Process pending events if necessary */
		if (Term->key_head == Term->key_tail)
		{
			/* Process events (do not wait) */
			Term_xtra(TERM_XTRA_EVENT, FALSE);
		}
	}

	/* No keys are ready */
	if (Term->key_head == Term->key_tail) return (1);

	/* Extract the next keypress */
	(*ch) = Term->key_queue[Term->key_tail];

	/* If requested, advance the queue, wrap around if necessary */
	if (take && (++Term->key_tail == Term->key_size)) Term->key_tail = 0;

	/* Success */
	return (0);
}



/*** Extra routines ***/


/*
 * Save the "requested" screen into the "memorized" screen
 *
 * Every "Term_save()" should match exactly one "Term_load()"
 */
errr Term_save(void)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Grab */
	term_win_copy(Term->mem, Term->scr, w, h);

	/* Success */
	return (0);
}


/*
 * Restore the "requested" contents (see above).
 *
 * Every "Term_save()" should match exactly one "Term_load()"
 */
errr Term_load(void)
{
	int y;

	int w = Term->wid;
	int h = Term->hgt;

	/* Load */
	term_win_copy(Term->scr, Term->mem, w, h);

	/* Assume change */
	for (y = 0; y < h; y++)
	{
		/* Assume change */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Success */
	return (0);
}




/*
 * React to a new physical window size.
 */
errr Term_resize(int w, int h)
{
	int i;

	int wid, hgt;

	byte *hold_x1;
	byte *hold_x2;

	term_win *hold_old;
	term_win *hold_scr;
	term_win *hold_mem;

#if 0

	term_win *hold_tmp;

#endif


	/* Ignore illegal changes */
	if ((w < 1) || (h < 1)) return (1);

	/* Ignore non-changes */
	if ((Term->wid == w) && (Term->hgt == h)) return (1);

	/* Minimum dimensions */
	wid = MIN(Term->wid, w);
	hgt = MIN(Term->hgt, h);


	/*** Save ***/

	/* Save scanners */
	hold_x1 = Term->x1;
	hold_x2 = Term->x2;

	/* Save old window */
	hold_old = Term->old;

	/* Save old window */
	hold_scr = Term->scr;

	/* Save old window */
	hold_mem = Term->mem;

#if 0

	/* Save old window */
	hold_tmp = Term->tmp;

#endif


	/*** Make new ***/

	/* Create new scanners */
	C_MAKE(Term->x1, h, byte);
	C_MAKE(Term->x2, h, byte);


	/* Create new window */
	MAKE(Term->old, term_win);

	/* Initialize new window */
	term_win_init(Term->old, w, h);

	/* Save the contents */
	term_win_copy(Term->old, hold_old, wid, hgt);


	/* Create new window */
	MAKE(Term->scr, term_win);

	/* Initialize new window */
	term_win_init(Term->scr, w, h);

	/* Save the contents */
	term_win_copy(Term->scr, hold_scr, wid, hgt);


	/* Create new window */
	MAKE(Term->mem, term_win);

	/* Initialize new window */
	term_win_init(Term->mem, w, h);

	/* Save the contents */
	term_win_copy(Term->mem, hold_mem, wid, hgt);


#if 0

	/* Create new window */
	MAKE(Term->tmp, term_win);

	/* Initialize new window */
	term_win_init(Term->tmp, w, h);

	/* Save the contents */
	term_win_copy(Term->tmp, hold_tmp, wid, hgt);

#endif


	/*** Kill old ***/

	/* Free some arrays */
	C_KILL(hold_x1, Term->hgt, byte);
	C_KILL(hold_x2, Term->hgt, byte);


	/* Nuke */
	term_win_nuke(hold_old, Term->wid, Term->hgt);

	/* Kill */
	KILL(hold_old, term_win);


	/* Nuke */
	term_win_nuke(hold_scr, Term->wid, Term->hgt);

	/* Kill */
	KILL(hold_scr, term_win);


	/* Nuke */
	term_win_nuke(hold_mem, Term->wid, Term->hgt);

	/* Kill */
	KILL(hold_mem, term_win);


#if 0

	/* Nuke */
	term_win_nuke(hold_tmp, Term->wid, Term->hgt);

	/* Kill */
	KILL(hold_tmp, term_win);

#endif


	/*** Verify cursor ***/

	/* Illegal cursor */
	if (Term->old->cx >= w) Term->old->cu = 1;
	if (Term->old->cy >= h) Term->old->cu = 1;

	/* Illegal cursor */
	if (Term->scr->cx >= w) Term->scr->cu = 1;
	if (Term->scr->cy >= h) Term->scr->cu = 1;

	/* Illegal cursor */
	if (Term->mem->cx >= w) Term->mem->cu = 1;
	if (Term->mem->cy >= h) Term->mem->cu = 1;

#if 0

	/* Illegal cursor */
	if (Term->tmp->cx >= w) Term->tmp->cu = 1;
	if (Term->tmp->cy >= h) Term->tmp->cu = 1;

#endif


	/* Save new size */
	Term->wid = w;
	Term->hgt = h;


	/* Force "total erase" */
	Term->total_erase = TRUE;

	/* Assume change */
	for (i = 0; i < h; i++)
	{
		/* Assume change */
		Term->x1[i] = 0;
		Term->x2[i] = w - 1;
	}

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;


	/* Success */
	return (0);
}



/*
 * Activate a new Term (and deactivate the current Term)
 *
 * This function is extremely important, and also somewhat bizarre.
 * It is the only function that should "modify" the value of "Term".
 *
 * To "create" a valid "term", one should do "term_init(t)", then
 * set the various flags and hooks, and then do "Term_activate(t)".
 */
errr Term_activate(term *t)
{
	/* Hack -- already done */
	if (Term == t) return (1);

	/* Deactivate the old Term */
	if (Term) Term_xtra(TERM_XTRA_LEVEL, 0);

	/* Hack -- Call the special "init" hook */
	if (t && !t->active_flag)
	{
		/* Call the "init" hook */
		if (t->init_hook) (*t->init_hook)(t);

		/* Remember */
		t->active_flag = TRUE;

		/* Assume mapped */
		t->mapped_flag = TRUE;
	}

	/* Remember the Term */
	Term = t;

	/* Activate the new Term */
	if (Term) Term_xtra(TERM_XTRA_LEVEL, 1);

	/* Success */
	return (0);
}



/*
 * Nuke a term
 */
errr term_nuke(term *t)
{
	int w = t->wid;
	int h = t->hgt;


	/* Hack -- Call the special "nuke" hook */
	if (t->active_flag)
	{
		/* Call the "nuke" hook */
		if (t->nuke_hook) (*t->nuke_hook)(t);

		/* Remember */
		t->active_flag = FALSE;

		/* Assume not mapped */
		t->mapped_flag = FALSE;
	}


	/* Nuke "displayed" */
	term_win_nuke(t->old, w, h);

	/* Kill "displayed" */
	KILL(t->old, term_win);

	/* Nuke "requested" */
	term_win_nuke(t->scr, w, h);

	/* Kill "requested" */
	KILL(t->scr, term_win);

	/* Nuke "memorized" */
	term_win_nuke(t->mem, w, h);

	/* Kill "memorized" */
	KILL(t->mem, term_win);

#if 0

	/* Nuke "temporary" */
	term_win_nuke(t->tmp, w, h);

	/* Kill "temporary" */
	KILL(t->tmp, term_win);

#endif


	/* Free some arrays */
	C_KILL(t->x1, h, byte);
	C_KILL(t->x2, h, byte);

	/* Free the input queue */
	C_KILL(t->key_queue, t->key_size, char);

	/* Success */
	return (0);
}


/*
 * Initialize a term, using a window of the given size.
 * Also prepare the "input queue" for "k" keypresses
 * By default, the cursor starts out "invisible"
 * By default, we "erase" using "black spaces"
 */
errr term_init(term *t, int w, int h, int k)
{
	int y;


	/* Wipe it */
	WIPE(t, term);


	/* Prepare the input queue */
	t->key_head = t->key_tail = 0;

	/* Determine the input queue size */
	t->key_size = k;

	/* Allocate the input queue */
	C_MAKE(t->key_queue, t->key_size, char);


	/* Save the size */
	t->wid = w;
	t->hgt = h;

	/* Allocate change arrays */
	C_MAKE(t->x1, h, byte);
	C_MAKE(t->x2, h, byte);


	/* Allocate "displayed" */
	MAKE(t->old, term_win);

	/* Initialize "displayed" */
	term_win_init(t->old, w, h);


	/* Allocate "requested" */
	MAKE(t->scr, term_win);

	/* Initialize "requested" */
	term_win_init(t->scr, w, h);


	/* Allocate "memorized" */
	MAKE(t->mem, term_win);

	/* Initialize "memorized" */
	term_win_init(t->mem, w, h);


#if 0

	/* Allocate "temporary" */
	MAKE(t->tmp, term_win);

	/* Initialize "temporary" */
	term_win_init(t->tmp, w, h);

#endif

	/* Assume change */
	for (y = 0; y < h; y++)
	{
		/* Assume change */
		t->x1[y] = 0;
		t->x2[y] = w - 1;
	}

	/* Assume change */
	t->y1 = 0;
	t->y2 = h - 1;

	/* Force "total erase" */
	t->total_erase = TRUE;


	/* Default "blank" */
	t->attr_blank = 0;
	t->char_blank = ' ';


	/* Success */
	return (0);
}


