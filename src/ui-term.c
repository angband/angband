/**
 * \file ui-term.c
 * \brief A generic, efficient, terminal window package
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
#include "buildid.h"
#include "h-basic.h"
#include "ui-term.h"
#include "z-color.h"
#include "z-util.h"
#include "z-virt.h"

/**
 * This file provides a generic, efficient, terminal window package,
 * which can be used not only on standard terminal environments such
 * as dumb terminals connected to a Unix box, but also in more modern
 * "graphic" environments, such as the Macintosh or Unix/X11.
 *
 * Each "window" works like a standard "dumb terminal", that is, it
 * can display a two dimensional array of grids containing colored
 * textual symbols, plus an optional cursor, and it can be used to
 * get keypress events from the user.
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
 *   drawing characters normally will not erase old ones
 *   drawing a character on the cursor often erases it
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
 *   characters may only be drawn at the cursor location
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
 *
 * This package allows each "grid" in each window to hold an attr/char
 * pair, with each ranging from 0 to 255, and makes very few assumptions
 * about the meaning of any attr/char values.  Normally, we assume that
 * "attr 0" is "black", with the semantics that "black" text should be
 * sent to "Term_wipe()" instead of "Term_text()", but this sematics is
 * modified if either the "always_pict" or the "always_text" flags are
 * set.  We assume that "char 0" is "dangerous", since placing such a
 * "char" in the middle of a string "terminates" the string, and usually
 * we prevent its use.
 *
 * Finally, we use a special attr/char pair, defaulting to "attr 0" and
 * "char 32", also known as "black space", when we "erase" or "clear"
 * any window, but this pair can be redefined to any pair, including
 * the standard "white space", or the bizarre "emptiness" ("attr 0"
 * and "char 0"), as long as various obscure restrictions are met.
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
 * The new formalism includes a "displayed" screen image (old) which
 * is actually seen by the user, a "requested" screen image (scr)
 * which is being prepared for display, a "memorized" screen image
 * (mem) which is used to save and restore screen images, and a
 * "temporary" screen image (tmp) which is currently unused.
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
 *   Term->xtra_hook = Perform extra actions
 *   Term->curs_hook = Draw (or Move) the cursor
 *   Term->bigcurs_hook = Draw (or Move) the big cursor (bigtile mode)
 *   Term->wipe_hook = Draw some blank spaces
 *   Term->text_hook = Draw some text in the window
 *   Term->pict_hook = Draw some attr/chars in the window
 *   Term->dblh_hook = Test if attr/char pair represents a double-height tile
 *
 * The "Term->xtra_hook" hook provides a variety of different functions,
 * based on the first parameter (which should be taken from the various
 * TERM_XTRA_* defines) and the second parameter (which may make sense
 * only for some first parameters).  It is available to the program via
 * the "Term_xtra()" function, though some first parameters are only
 * "legal" when called from inside this package.
 *
 * The "Term->curs_hook" hook provides this package with a simple way
 * to "move" or "draw" the cursor to the grid "x,y", depending on the
 * setting of the "soft_cursor" flag.  Note that the cursor is never
 * redrawn if "nothing" has happened to the screen (even temporarily).
 * This hook is required.
 *
 * The "Term->wipe_hook" hook provides this package with a simple way
 * to "erase", starting at "x,y", the next "n" grids.  This hook assumes
 * that the input is valid.  This hook is required, unless the setting
 * of the "always_pict" or "always_text" flags makes it optional.
 *
 * The "Term->text_hook" hook provides this package with a simple way
 * to "draw", starting at "x,y", the "n" chars contained in "cp", using
 * the attr "a".  This hook assumes that the input is valid, and that
 * "n" is between 1 and 256 inclusive, but it should NOT assume that
 * the contents of "cp" are null-terminated.  This hook is required,
 * unless the setting of the "always_pict" flag makes it optional.
 *
 * The "Term->pict_hook" hook provides this package with a simple way
 * to "draw", starting at "x,y", the "n" attr/char pairs contained in
 * the arrays "ap" and "cp".  This hook assumes that the input is valid,
 * and that "n" is between 1 and 256 inclusive, but it should NOT assume
 * that the contents of "cp" are null-terminated.  This hook is optional,
 * unless the setting of the "always_pict" or "higher_pict" flags make
 * it required.  Note that recently, this hook was changed from taking
 * a int "a" and a char "c" to taking a length "n", an array of ints
 * "ap" and an array of chars "cp".  Old implementations of this hook
 * should now iterate over all "n" attr/char pairs.
 * The two new arrays "tap" and "tcp" can contain the attr/char pairs
 * of the terrain below the values in "ap" and "cp".  These values can
 * be used to implement transparency when using graphics by drawing
 * the terrain values as a background and the "ap", "cp" values in
 * the foreground.
 *
 * The "Term->dblh_hook" hook provides this package a way to query whether
 * an attr/char pair corresponds to a double-height tile when it determines what
 * has changed and needs to be redrawn.  This hook is optional.  Set to NULL,
 * if no pairs correspond to a double-height tile.  Non-NULL values will only
 * be used if either the "always_pict" or "higher_pict" flags are are on.
 * Another, less efficient way to handle double-height tiles is to use
 * Term_mark() to force a position affected by a double-height tile to be
 * redrawn at the next refresh.
 *
 * The game "Angband" uses a set of files called "main-xxx.c", for
 * various "xxx" suffixes.  Most of these contain a function called
 * "init_xxx()", that will prepare the underlying visual system for
 * use with Angband, and then create one or more "term" structures,
 * using flags and hooks appropriate to the given platform, so that
 * the "main()" function can call one (or more) of the "init_xxx()"
 * functions, as appropriate, to prepare the required "term" structs
 * (one for each desired sub-window), and these "init_xxx()" functions
 * are called from a centralized "main()" function in "main.c".  Other
 * "main-xxx.c" systems contain their own "main()" function which, in
 * addition to doing everything needed to initialize the actual program,
 * also does everything that the normal "init_xxx()" functions would do.
 *
 * The game "Angband" defines, in addition to "attr 0", all of the
 * attr codes from 1 to 15, using definitions in "defines.h", and
 * thus the "main-xxx.c" files used by Angband must handle these
 * attr values correctly.  Also, they must handle all other attr
 * values, though they may do so in any way they wish, for example,
 * by always taking every attr code mod 16.  Many of the "main-xxx.c"
 * files use "white space" ("attr 1" / "char 32") to "erase" or "clear"
 * any window, for efficiency.
 *
 * See "main-xxx.c" for a simple skeleton file which can be used to
 * create a "visual system" for a new platform when porting Angband.
 */



/**
 * The array[ANGBAND_TERM_MAX] of window pointers
 */
term *angband_term[ANGBAND_TERM_MAX];


/**
 * The array[ANGBAND_TERM_MAX] of window names (modifiable?)
 *
 * ToDo: Make the names independent of ANGBAND_TERM_MAX.
 */
char angband_term_name[ANGBAND_TERM_MAX][16] =
{
	VERSION_NAME,
	"Term-1",
	"Term-2",
	"Term-3",
	"Term-4",
	"Term-5",
	"Term-6",
	"Term-7"
};

uint32_t window_flag[ANGBAND_TERM_MAX];

int row_top_map[SIDEBAR_MAX] = {1, 4, 1};
int row_bottom_map[SIDEBAR_MAX] = {1, 0, 0};
int col_map[SIDEBAR_MAX] = {13, 0, 0};

/**
 * The current "term"
 */
term *Term = NULL;

/* grumbles */
int log_i = 0;
int log_size = 0;
struct keypress keylog[KEYLOG_SIZE];


/**
 * ------------------------------------------------------------------------
 * Local routines
 * ------------------------------------------------------------------------ */


/**
 * Nuke a term_win (see below)
 */
static errr term_win_nuke(term_win *s)
{
	/* Free the window access arrays */
	mem_free_alt(s->a);
	mem_free_alt(s->c);

	/* Free the window content arrays */
	mem_free_alt(s->va);
	mem_free_alt(s->vc);

	/* Free the terrain access arrays */
	mem_free_alt(s->ta);
	mem_free_alt(s->tc);

	/* Free the terrain content arrays */
	mem_free_alt(s->vta);
	mem_free_alt(s->vtc);

	/* Success */
	return (0);
}


/**
 * Initialize a "term_win" (using the given window size)
 */
static errr term_win_init(term_win *s, int w, int h)
{
	int y;

	/* Make the window access arrays */
	s->a = mem_zalloc_alt(h * sizeof(int*));
	s->c = mem_zalloc_alt(h * sizeof(wchar_t*));

	/* Make the window content arrays */
	s->va = mem_zalloc_alt(h * w * sizeof(int));
	s->vc = mem_zalloc_alt(h * w * sizeof(wchar_t));

	/* Make the terrain access arrays */
	s->ta = mem_zalloc_alt(h * sizeof(int*));
	s->tc = mem_zalloc_alt(h * sizeof(wchar_t*));

	/* Make the terrain content arrays */
	s->vta = mem_zalloc_alt(h * w * sizeof(int));
	s->vtc = mem_zalloc_alt(h * w * sizeof(wchar_t));

	/* Prepare the window access arrays */
	for (y = 0; y < h; y++) {
		s->a[y] = s->va + w * y;
		s->c[y] = s->vc + w * y;

		s->ta[y] = s->vta + w * y;
		s->tc[y] = s->vtc + w * y;
	}

	/* Success */
	return (0);
}


/**
 * Copy a "term_win" from another
 */
static errr term_win_copy(term_win *s, term_win *f, int w, int h)
{
	int x, y;

	/* Copy contents */
	for (y = 0; y < h; y++) {
		int *f_aa = f->a[y];
		wchar_t *f_cc = f->c[y];

		int *s_aa = s->a[y];
		wchar_t *s_cc = s->c[y];

		int *f_taa = f->ta[y];
		wchar_t *f_tcc = f->tc[y];

		int *s_taa = s->ta[y];
		wchar_t *s_tcc = s->tc[y];

		for (x = 0; x < w; x++) {
			*s_aa++ = *f_aa++;
			*s_cc++ = *f_cc++;

			*s_taa++ = *f_taa++;
			*s_tcc++ = *f_tcc++;
		}
	}

	/* Copy cursor */
	s->cnx = f->cnx;
	s->cny = f->cny;
	s->cx = f->cx;
	s->cy = f->cy;
	s->cu = f->cu;
	s->cv = f->cv;

	/* Success */
	return (0);
}



/**
 * ------------------------------------------------------------------------
 * Public functions operating on all terminals
 * ------------------------------------------------------------------------
 */


/**
 * Redraw all the terminals.
 */
extern errr Term_redraw_all(void)
{
	term *old = Term;
	errr combined = 0;
	int j;

	for (j = 0; j < ANGBAND_TERM_MAX; j++) {
		errr one_result;

		if (!angband_term[j]) continue;
		(void) Term_activate(angband_term[j]);
		one_result = Term_redraw();
		if (!one_result) {
			combined = one_result;
		}
	}
	(void) Term_activate(old);

	return combined;
}

/**
 * ------------------------------------------------------------------------
 * External hooks
 * ------------------------------------------------------------------------ */


/**
 * Execute the "Term->xtra_hook" hook, if available (see above).
 */
errr Term_xtra(int n, int v)
{
	/* Verify the hook */
	if (!Term->xtra_hook) return (-1);

	/* Call the hook */
	return ((*Term->xtra_hook)(n, v));
}

/**
 * ------------------------------------------------------------------------
 * Fake hooks
 * ------------------------------------------------------------------------ */


/**
 * Hack -- fake hook for "Term_curs()" (see above)
 */
static errr Term_curs_hack(int x, int y)
{
	/* Compiler silliness */
	if (x || y) return (-2);

	/* Oops */
	return (-1);
}

/**
 * Hack -- fake hook for "Term_wipe()" (see above)
 */
static errr Term_wipe_hack(int x, int y, int n)
{
	/* Compiler silliness */
	if (x || y || n) return (-2);

	/* Oops */
	return (-1);
}

/**
 * Hack -- fake hook for "Term_text()" (see above)
 */
static errr Term_text_hack(int x, int y, int n, int a, const wchar_t *cp)
{
	/* Compiler silliness */
	if (x || y || n || a || cp) return (-2);

	/* Oops */
	return (-1);
}


/**
 * Hack -- fake hook for "Term_pict()" (see above)
 */
static errr Term_pict_hack(int x, int y, int n, const int *ap,
						   const wchar_t *cp, const int *tap,
						   const wchar_t *tcp)
{
	/* Compiler silliness */
	if (x || y || n || ap || cp || tap || tcp) return (-2);

	/* Oops */
	return (-1);
}


/**
 * ------------------------------------------------------------------------
 * Efficient routines
 * ------------------------------------------------------------------------ */


/**
 * Mentally draw an attr/char at a given location
 *
 * Assumes given location and values are valid.
 */
void Term_queue_char(term *t, int x, int y, int a, wchar_t c, int ta,
					 wchar_t tc)
{
	int *scr_aa = t->scr->a[y];
	wchar_t *scr_cc = t->scr->c[y];

	int oa = scr_aa[x];
	wchar_t oc = scr_cc[x];

	int *scr_taa = t->scr->ta[y];
	wchar_t *scr_tcc = t->scr->tc[y];

	int ota = scr_taa[x];
	wchar_t otc = scr_tcc[x];

	/* Don't change is the terrain value is 0 */
	if (!ta) ta = ota;
	if (!tc) tc = otc;

	/* Hack -- Ignore non-changes */
	if ((oa == a) && (oc == c) && (ota == ta) && (otc == tc)) return;

	/* Save the "literal" information */
	scr_aa[x] = a;
	scr_cc[x] = c;

	scr_taa[x] = ta;
	scr_tcc[x] = tc;

	/* Check for new min/max row info */
	if (y < t->y1) t->y1 = y;
	if (y > t->y2) t->y2 = y;

	/* Check for new min/max col info for this row */
	if (x < t->x1[y]) t->x1[y] = x;
	if (x > t->x2[y]) t->x2[y] = x;

	if (t->dblh_hook) {
		/*
		 * If the previous contents are a double-height tile also
		 * adjust the modified bounds to encompass the position on
		 * the previous row of tiles so it can be included when
		 * redrawing at the next refresh.
		 */
		if (y >= tile_height) {
			int ofg_dbl = (*t->dblh_hook)(oa, oc);
			int obg_dbl = (*t->dblh_hook)(ota, otc);

			if (ofg_dbl || obg_dbl) {
				int yp = y - tile_height;

				if (yp < t->y1) t->y1 = yp;
				if (x < t->x1[yp]) t->x1[yp] = x;
				if (x > t->x2[yp]) t->x2[yp] = x;
			}
		}
		/*
		 * If the next row had a double-height tile, expand the modified
		 * bounds to encompass it as well since at least its upper
		 * half will need to be redrawn for the change here.
		 */
		if (y < t->hgt - tile_height) {
			int yn = y + tile_height;
			int ofg_dbl_nr = (*t->dblh_hook)(
				t->old->a[yn][x], t->old->c[yn][x]);
			int obg_dbl_nr = (*t->dblh_hook)(
				t->old->ta[yn][x], t->old->tc[yn][x]);

			if (ofg_dbl_nr || obg_dbl_nr) {
				if (yn > t->y2) t->y2 = yn;
				if (x < t->x1[yn]) t->x1[yn] = x;
				if (x > t->x2[yn]) t->x2[yn] = x;
			}
		}
	}
}

/**
 * Queue a large-sized tile.
 * \param t Is the terminal to modify.
 * \param x Is the column for the upper left corner of the tile.
 * \param y Is the row for the upper left corner of the tile.
 * \param clipy Is the lower bound for rows that should not be modified when
 * writing the large-sized tile.
 * \param a Is the foreground attribute.
 * \param c Is the foreground character.
 * \param a1 Is the background attribute.
 * \param c1 Is the background character.
 */
void Term_big_queue_char(term *t, int x, int y, int clipy,
	int a, wchar_t c, int a1, wchar_t c1)
{
	int vmax;
	int hor, vert;

	/* Avoid warning */
	(void)c;

	/* Leave space on bottom if requested */
	vmax = (y + tile_height <= clipy) ? tile_height : clipy - y;

	/* No tall skinny tiles */
	if (tile_width > 1) {
	        /* Horizontal first; skip already marked upper left corner */
	        for (hor = 1; hor < tile_width; hor++) {
		        /* Queue dummy character */
			if (a & 0x80)
				Term_queue_char(t, x + hor, y, 255, -1, 0, 0);
			else
				Term_queue_char(t, x + hor, y, COLOUR_WHITE, L' ', a1, c1);
		}

		/* Now vertical */
		for (vert = 1; vert < vmax; vert++) {
			for (hor = 0; hor < tile_width; hor++) {
				/* Queue dummy character */
				if (a & 0x80)
					Term_queue_char(t, x + hor, y + vert, 255, -1, 0, 0);
				else
					Term_queue_char(t, x + hor, y + vert, COLOUR_WHITE, L' ', a1, c1);
			}
		}
	} else {
		/* Only vertical */
		for (vert = 1; vert < vmax; vert++) {
			/* Queue dummy character */
			if (a & 0x80)
				Term_queue_char(t, x, y + vert, 255, -1, 0, 0);
			else
				Term_queue_char(t, x, y + vert, COLOUR_WHITE, L' ', a1, c1);
		}
	}
}

/**
 * Mentally draw some attr/chars at a given location
 *
 * Assumes that (x,y) is a valid location, that the first "n" characters
 * of the string "s" are all valid (non-zero), and that (x+n-1,y) is also
 * a valid location, so the first "n" characters of "s" can all be added
 * starting at (x,y) without causing any illegal operations.
 */
void Term_queue_chars(int x, int y, int n, int a, const wchar_t *s)
{
	int x1 = -1, x2 = -1;

	int *scr_aa = Term->scr->a[y];
	wchar_t *scr_cc = Term->scr->c[y];

	int *scr_taa = Term->scr->ta[y];
	wchar_t *scr_tcc = Term->scr->tc[y];

	/* Queue the attr/chars */
	for ( ; n; x++, s++, n--) {
		int oa = scr_aa[x];
		wchar_t oc = scr_cc[x];

		int ota = scr_taa[x];
		wchar_t otc = scr_tcc[x];

		/* Hack -- Ignore non-changes */
		if ((oa == a) && (oc == *s) && (ota == 0) && (otc == 0)) continue;

		/* Save the "literal" information */
		scr_aa[x] = a;
		scr_cc[x] = *s;

		scr_taa[x] = 0;
		scr_tcc[x] = 0;

		/* Note the "range" of window updates */
		if (x1 < 0) x1 = x;
		x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0) {
		/* Check for new min/max row info */
		if (y < Term->y1) Term->y1 = y;
		if (y > Term->y2) Term->y2 = y;

		/* Check for new min/max col info in this row */
		if (x1 < Term->x1[y]) Term->x1[y] = x1;
		if (x2 > Term->x2[y]) Term->x2[y] = x2;
	}
}



/**
 * ------------------------------------------------------------------------
 * Refresh routines
 * ------------------------------------------------------------------------ */


/**
 * Flush a row of the current window (see "Term_fresh")
 *
 * Display text using "Term_pict()"
 */
static void Term_fresh_row_pict(int y, int x1, int x2)
{
	int x;

	int *old_aa = Term->old->a[y];
	wchar_t *old_cc = Term->old->c[y];

	int *scr_aa = Term->scr->a[y];
	wchar_t *scr_cc = Term->scr->c[y];

	int *old_taa = Term->old->ta[y];
	wchar_t *old_tcc = Term->old->tc[y];

	int *scr_taa = Term->scr->ta[y];
	wchar_t *scr_tcc = Term->scr->tc[y];

	int ota;
	wchar_t otc;

	int nta;
	wchar_t ntc;

	/* Pending length */
	int fn = 0;

	/* Pending start */
	int fx = 0;

	int oa;
	wchar_t oc;

	int na;
	wchar_t nc;

	/* Scan "modified" columns */
	for (x = x1; x <= x2; x++) {
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* See what is desired there */
		na = scr_aa[x];
		nc = scr_cc[x];

		ota = old_taa[x];
		otc = old_tcc[x];

		nta = scr_taa[x];
		ntc = scr_tcc[x];

		/* Handle unchanged grids */
		if ((na == oa) && (nc == oc) && (nta == ota) && (ntc == otc)) {
			/* Flush */
			if (fn) {
				/* Draw pending attr/char pairs */
				(void)((*Term->pict_hook)(fx, y, fn, &scr_aa[fx], &scr_cc[fx],
										  &scr_taa[fx], &scr_tcc[fx]));

				/* Forget */
				fn = 0;
			}

			/* Skip */
			continue;
		}

		/* Save new contents */
		old_aa[x] = na;
		old_cc[x] = nc;

		old_taa[x] = nta;
		old_tcc[x] = ntc;

		/* Restart and Advance */
		if (fn++ == 0) fx = x;
	}

	/* Flush */
	if (fn) {
		/* Draw pending attr/char pairs */
		(void)((*Term->pict_hook)(fx, y, fn, &scr_aa[fx], &scr_cc[fx],
								  &scr_taa[fx], &scr_tcc[fx]));
	}
}


/**
 * Flush a row of the current window when checking for double-height tiles
 * (see "Term_fresh")
 *
 * Display text using "Term_pict()"
 */
static void Term_fresh_row_pict_dblh(int y, int x1, int x2, int *pr_drw)
{
	int x;

	int *old_aa = Term->old->a[y];
	wchar_t *old_cc = Term->old->c[y];

	const int *scr_aa = Term->scr->a[y];
	const wchar_t *scr_cc = Term->scr->c[y];

	int *old_taa = Term->old->ta[y];
	wchar_t *old_tcc = Term->old->tc[y];

	const int *scr_taa = Term->scr->ta[y];
	const wchar_t *scr_tcc = Term->scr->tc[y];

	const int *scr_aa_nr;
	const wchar_t *scr_cc_nr;
	const int *scr_taa_nr;
	const wchar_t *scr_tcc_nr;
	const int *old_aa_nr;
	const wchar_t *old_cc_nr;
	const int *old_taa_nr;
	const wchar_t *old_tcc_nr;

	/* Pending length */
	int fn = 0;

	/* Pending start */
	int fx = 0;

	if (y < Term->hgt - tile_height) {
		scr_aa_nr = Term->scr->a[y + tile_height];
		scr_cc_nr = Term->scr->c[y + tile_height];
		scr_taa_nr = Term->scr->ta[y + tile_height];
		scr_tcc_nr = Term->scr->tc[y + tile_height];
		old_aa_nr = Term->old->a[y + tile_height];
		old_cc_nr = Term->old->c[y + tile_height];
		old_taa_nr = Term->old->ta[y + tile_height];
		old_tcc_nr = Term->old->tc[y + tile_height];
	} else {
		/*
		 * Can't examine the next row of tiles because it would be
		 * out of bounds.  To avoid writing much the same code but
		 * with the checks on the next row skipped, fake it so the
		 * next row looks unmodified.
		 */
		scr_aa_nr = scr_aa;
		scr_cc_nr = scr_cc;
		scr_taa_nr = scr_taa;
		scr_tcc_nr = scr_tcc;
		old_aa_nr = scr_aa_nr;
		old_cc_nr = scr_cc_nr;
		old_taa_nr = scr_taa_nr;
		old_tcc_nr = scr_tcc_nr;
	}

	/*
	 * For unmodified columns at the start, set flags so processing of the
	 * next row knows they were not redrawn.
	 */
	for (x = 0; x < x1; x++) {
		pr_drw[x] = 0;
	}

	/* Scan "modified" columns */
	for (x = x1; x <= x2; x++) {
		/* See what is currently here. */
		int oa = old_aa[x];
		wchar_t oc = old_cc[x];
		int ota = old_taa[x];
		wchar_t otc = old_tcc[x];

		/* See what is desired here. */
		int na = scr_aa[x];
		wchar_t nc = scr_cc[x];
		int nta = scr_taa[x];
		wchar_t ntc = scr_tcc[x];

		int draw;

		if (na == oa && nc == oc && nta == ota && ntc == otc) {
			/*
			 * That element did not change.  If it is double-height
			 * and the previous row was drawn will have to redraw
			 * to get the upper half of this one drawn correctly.
			 */
			if (pr_drw[x] &&
					((*Term->dblh_hook)(na, nc) ||
					(*Term->dblh_hook)(nta, ntc))) {
				draw = 1;
			} else {
				/*
				 * If the next row had double-height tiles and
				 * those have changed, also have to redraw
				 * (either to clear what was there if now gone
				 * or to get the correct backdrop for the new
				 * double-height tile there now).
				 */
				/* See what is in the next row. */
				int oa_nr = old_aa_nr[x];
				wchar_t oc_nr = old_cc_nr[x];
				int ota_nr = old_taa_nr[x];
				wchar_t otc_nr = old_tcc_nr[x];

				/* See what is desired in the next row. */
				int na_nr = scr_aa_nr[x];
				wchar_t nc_nr = scr_cc_nr[x];
				int nta_nr = scr_taa_nr[x];
				wchar_t ntc_nr = scr_tcc_nr[x];

				if (((*Term->dblh_hook)(oa_nr, oc_nr) ||
						(*Term->dblh_hook)(ota_nr, otc_nr)) &&
						(na_nr != oa_nr ||
						nc_nr != oc_nr ||
						nta_nr != ota_nr ||
						ntc_nr != otc_nr)) {
					draw = 1;
				} else {
					draw = 0;
				}
			}

			/* Remember if this element was redrawn or not. */
			pr_drw[x] = draw;
		} else {
			draw = 1;
			/* Remember that this element was redrawn. */
			pr_drw[x] = 1;
		}

		/* Handle grids that don't have to be drawn. */
		if (!draw) {
			/* Flush */
			if (fn) {
				/* Draw pending attr/char pairs */
				(void)((*Term->pict_hook)(fx, y, fn,
					&scr_aa[fx], &scr_cc[fx], &scr_taa[fx],
					&scr_tcc[fx]));

				/* Forget */
				fn = 0;
			}

			/* Skip */
			continue;
		}

		/* Save new contents */
		old_aa[x] = na;
		old_cc[x] = nc;

		old_taa[x] = nta;
		old_tcc[x] = ntc;

		/* Restart and Advance */
		if (fn++ == 0) fx = x;
	}

	/* Flush */
	if (fn) {
		/* Draw pending attr/char pairs */
		(void)((*Term->pict_hook)(fx, y, fn, &scr_aa[fx], &scr_cc[fx],
			&scr_taa[fx], &scr_tcc[fx]));
	}

	/*
	 * For unmodified columns at the end, set flags so processing of the
	 * next row knows they weren't redrawn.
	 */
	for (x = x2 + 1; x < Term->wid; x++) {
		pr_drw[x] = 0;
	}
}


/**
 * Helper function for Term_fresh_row_both() and Term_fresh_row_both_dblh():
 * check padding of big tile for changes.
 * \param t Is the terminal to check.
 * \param y Is the row coordinate for the upper left corner of the big tile.
 * \param x Is the column coordinate for the upper left corner of the big tile.
 * \return Returns a nonzero value if there is a change in one or more grids
 * whose desired contents are padding for the big tile.
 */
static int is_padding_changed(term *t, int y, int x)
{
	int xsl = MIN(x + tile_width, t->wid);
	int ysl = MIN(y + tile_height, t->hgt);
	int xs, ys;

	for (xs = x + 1; xs < xsl; ++xs) {
		if (t->scr->a[y][xs] == 255 &&
				(t->scr->a[y][xs] != t->old->a[y][xs] ||
				t->scr->c[y][xs] != t->old->c[y][xs] ||
				t->scr->ta[y][xs] != t->old->ta[y][xs] ||
				t->scr->tc[y][xs] != t->old->tc[y][xs])) {
			return 1;
		}
	}
	for (ys = y + 1; ys < ysl; ++ys) {
		for (xs = x; xs < xsl; ++xs) {
			if (t->scr->a[ys][xs] == 255 &&
					(t->scr->a[ys][xs] != t->old->a[ys][xs] ||
					t->scr->c[ys][xs] != t->old->c[ys][xs] ||
					t->scr->ta[ys][xs] != t->old->ta[ys][xs] ||
					t->scr->tc[ys][xs] != t->old->tc[ys][xs])) {
				return 1;
			}
		}
	}
	return 0;
}


/**
 * Flush a row of the current window (see "Term_fresh")
 *
 * Display text using "Term_text()" and "Term_wipe()",
 * but use "Term_pict()" for high-bit attr/char pairs
 */
static void Term_fresh_row_both(int y, int x1, int x2)
{
	int x;

	int *old_aa = Term->old->a[y];
	wchar_t *old_cc = Term->old->c[y];
	int *scr_aa = Term->scr->a[y];
	wchar_t *scr_cc = Term->scr->c[y];

	int *old_taa = Term->old->ta[y];
	wchar_t *old_tcc = Term->old->tc[y];
	int *scr_taa = Term->scr->ta[y];
	wchar_t *scr_tcc = Term->scr->tc[y];

	int ota;
	wchar_t otc;
	int nta;
	wchar_t ntc;

	/* The "always_text" flag */
	int always_text = Term->always_text;

	/* Pending length */
	int fn = 0;

	/* Pending start */
	int fx = 0;

	/* Pending attr */
	int fa = COLOUR_WHITE;

	int oa;
	wchar_t oc;

	int na;
	wchar_t nc;

	/* Scan "modified" columns */
	for (x = x1; x <= x2; x++) {
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* See what is desired there */
		na = scr_aa[x];
		nc = scr_cc[x];

		ota = old_taa[x];
		otc = old_tcc[x];

		nta = scr_taa[x];
		ntc = scr_tcc[x];

		/* Handle unchanged grids */
		if ((na == oa) && (nc == oc) && (nta == ota) && (ntc == otc)) {
			int draw;

			/*
			 * If (x,y) is the upper left corner of a big tile,
			 * check for change in its padded area.
			 */
			if ((na & 0x80) && na != 255) {
				draw = is_padding_changed(Term, y, x);
			} else {
				draw = 0;
			}

			/* Flush */
			if (fn) {
				/* Draw pending chars (normal or black) */
				if (fa || always_text)
					(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
				else
					(void)((*Term->wipe_hook)(fx, y, fn));

				/* Forget */
				fn = 0;
			}

			if (draw) {
				/*
				 * Since a change occurred in the padded area,
				 * redraw the whole tile even though the upper
				 * left is unchanged.
				 */
				(void)((*Term->pict_hook)(x, y, 1, &na, &nc,
					&nta, &ntc));
			}

			/* Skip */
			continue;
		}

		/* Save new contents */
		old_aa[x] = na;
		old_cc[x] = nc;
		old_taa[x] = nta;
		old_tcc[x] = ntc;

		/* Handle high-bit attr/chars */
		if ((na & 0x80)) {
			/* Flush */
			if (fn) {
				/* Draw pending chars (normal or black) */
				if (fa || always_text)
					(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
				else
					(void)((*Term->wipe_hook)(fx, y, fn));

				/* Forget */
				fn = 0;
			}

			/* 2nd byte of bigtile */
			if (na == 255) continue;

			/* Hack -- Draw the special attr/char pair */
			(void)((*Term->pict_hook)(x, y, 1, &na, &nc, &nta, &ntc));

			/* Skip */
			continue;
		}

		/* Notice new color */
		if (fa != na) {
			/* Flush */
			if (fn) {
				/* Draw the pending chars, erase leading spaces */
				if (fa || always_text)
					(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
				else
					(void)((*Term->wipe_hook)(fx, y, fn));

				/* Forget */
				fn = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Restart and Advance */
		if (fn++ == 0) fx = x;
	}

	/* Flush */
	if (fn) {
		/* Draw pending chars (normal or black) */
		if (fa || always_text)
			(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
		else
			(void)((*Term->wipe_hook)(fx, y, fn));
	}
}


/**
 * Flush a row of the current window when checking for double-height tiles
 * (see "Term_fresh")
 *
 * Display text using "Term_text()" and "Term_wipe()",
 * but use "Term_pict()" for high-bit attr/char pairs
 */
static void Term_fresh_row_both_dblh(int y, int x1, int x2, int *pr_drw)
{
	int x;

	int *old_aa = Term->old->a[y];
	wchar_t *old_cc = Term->old->c[y];
	const int *scr_aa = Term->scr->a[y];
	const wchar_t *scr_cc = Term->scr->c[y];

	int *old_taa = Term->old->ta[y];
	wchar_t *old_tcc = Term->old->tc[y];
	const int *scr_taa = Term->scr->ta[y];
	const wchar_t *scr_tcc = Term->scr->tc[y];

	const int *scr_aa_nr;
	const wchar_t *scr_cc_nr;
	const int *scr_taa_nr;
	const wchar_t *scr_tcc_nr;
	const int *old_aa_nr;
	const wchar_t *old_cc_nr;
	const int *old_taa_nr;
	const wchar_t *old_tcc_nr;

	/* The "always_text" flag */
	int always_text = Term->always_text;

	/* Pending length */
	int fn = 0;

	/* Pending start */
	int fx = 0;

	/* Pending attr */
	int fa = COLOUR_WHITE;

	if (y < Term->hgt - tile_height) {
		scr_aa_nr = Term->scr->a[y + tile_height];
		scr_cc_nr = Term->scr->c[y + tile_height];
		scr_taa_nr = Term->scr->ta[y + tile_height];
		scr_tcc_nr = Term->scr->tc[y + tile_height];
		old_aa_nr = Term->old->a[y + tile_height];
		old_cc_nr = Term->old->c[y + tile_height];
		old_taa_nr = Term->old->ta[y + tile_height];
		old_tcc_nr = Term->old->tc[y + tile_height];
	} else {
		/*
		 * Can't examine the next row of tiles because it would be
		 * out of bounds.  To avoid writing much the same code but
		 * with the checks on the next row skipped, fake it so the
		 * next row looks unmodified.
		 */
		scr_aa_nr = scr_aa;
		scr_cc_nr = scr_cc;
		scr_taa_nr = scr_taa;
		scr_tcc_nr = scr_tcc;
		old_aa_nr = scr_aa_nr;
		old_cc_nr = scr_cc_nr;
		old_taa_nr = scr_taa_nr;
		old_tcc_nr = scr_tcc_nr;
	}

	/*
	 * For unmodified columns at the start, set flags so processing of the
	 * next row knows they weren't redrawn.
	 */
	for (x = 0; x < x1; x++) {
		pr_drw[x] = 0;
	}

	/* Scan "modified" columns */
	for (x = x1; x <= x2; x++) {
		/* See what is currently here. */
		int oa = old_aa[x];
		wchar_t oc = old_cc[x];
		int ota = old_taa[x];
		wchar_t otc = old_tcc[x];

		/* See what is desired here. */
		int na = scr_aa[x];
		wchar_t nc = scr_cc[x];
		int nta = scr_taa[x];
		wchar_t ntc = scr_tcc[x];

		int draw;

		if (na == oa && nc == oc && nta == ota && ntc == otc) {
			/*
			 * That element did not change.  If it is double-height
			 * and the previous row was drawn, still have to redraw
			 * to get the upper half of this one drawn correctly.
			 */
			if (pr_drw[x] &&
					((*Term->dblh_hook)(na, nc) ||
					(*Term->dblh_hook)(nta, ntc))) {
				draw = 1;
			} else {
				/*
				 * If the next row had double-height tiles and
				 * those have changed, also have to redraw
				 * (either to clear what was where if now gone
				 * or to get the correct background for the new
				 * double-height tile there now).
				 */
				/* See what is in the next row. */
				int oa_nr = old_aa_nr[x];
				wchar_t oc_nr = old_cc_nr[x];
				int ota_nr = old_taa_nr[x];
				wchar_t otc_nr = old_tcc_nr[x];

				/* See what is desired in the next row. */
				int na_nr = scr_aa_nr[x];
				wchar_t nc_nr = scr_cc_nr[x];
				int nta_nr = scr_taa_nr[x];
				wchar_t ntc_nr = scr_tcc_nr[x];

				if (((*Term->dblh_hook)(oa_nr, oc_nr) ||
						(*Term->dblh_hook)(ota_nr, otc_nr)) &&
						(na_nr != oa_nr ||
						nc_nr != oc_nr ||
						nta_nr != ota_nr ||
						ntc_nr != otc_nr)) {
					draw = 1;
				} else {
					/*
					 * If (x,y) is the upper left corner of
					 * a big tile, check for change in its
					 * padded area.
					 */
					if ((na & 0x80) && na != 255) {
						draw = is_padding_changed(
							Term, y, x);
					} else {
						draw = 0;
					}
				}
			}

			/* Remember if this element was redrawn or not. */
			pr_drw[x] = draw;
		} else {
			draw = 1;
			/* Remember that this element was redrawn. */
			pr_drw[x] = 1;
		}

		/* Handle grids that don't have to be drawn. */
		if (!draw) {
			/* Flush */
			if (fn) {
				/* Draw pending chars (normal or black) */
				if (fa || always_text) {
					(void)((*Term->text_hook)(fx, y, fn, fa,
						&scr_cc[fx]));
				} else {
					(void)((*Term->wipe_hook)(fx, y, fn));
				}
				/* Forget */
				fn = 0;
			}

			/* Skip */
			continue;
		}

		/* Save new contents */
		old_aa[x] = na;
		old_cc[x] = nc;
		old_taa[x] = nta;
		old_tcc[x] = ntc;

		/* Handle high-bit attr/chars */
		if ((na & 0x80)) {
			/* Flush */
			if (fn) {
				/* Draw pending chars (normal or black) */
				if (fa || always_text) {
					(void)((*Term->text_hook)(fx, y, fn, fa,
						&scr_cc[fx]));
				} else {
					(void)((*Term->wipe_hook)(fx, y, fn));
				}
				/* Forget */
				fn = 0;
			}

			/* Skip padding element for big tiles. */
			if (na == 255) continue;

			/* Hack -- Draw the special attr/char pair */
			(void)((*Term->pict_hook)(x, y, 1, &na, &nc, &nta,
				&ntc));

			/* Skip */
			continue;
		}

		/* Notice new color */
		if (fa != na) {
			/* Flush */
			if (fn) {
				/*
				 * Draw the pending chars, erase leading spaces
				 */
				if (fa || always_text) {
					(void)((*Term->text_hook)(fx, y, fn, fa,
						&scr_cc[fx]));
				} else {
					(void)((*Term->wipe_hook)(fx, y, fn));
				}
				/* Forget */
				fn = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Restart and Advance */
		if (fn++ == 0) fx = x;
	}

	/* Flush */
	if (fn) {
		/* Draw pending chars (normal or black) */
		if (fa || always_text) {
			(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
		} else {
			(void)((*Term->wipe_hook)(fx, y, fn));
		}
	}

	/*
	 * For unmodified columns at end, set flags so processing of the next
	 * knows they weren't redrawn.
	 */
	for (x = x2 + 1; x < Term->wid; x++) {
		pr_drw[x] = 0;
	}
}


/**
 * Flush a row of the current window (see "Term_fresh")
 *
 * Display text using "Term_text()" and "Term_wipe()"
 */
static void Term_fresh_row_text(int y, int x1, int x2)
{
	int x;

	int *old_aa = Term->old->a[y];
	wchar_t *old_cc = Term->old->c[y];

	int *scr_aa = Term->scr->a[y];
	wchar_t *scr_cc = Term->scr->c[y];

	/* The "always_text" flag */
	int always_text = Term->always_text;

	/* Pending length */
	int fn = 0;

	/* Pending start */
	int fx = 0;

	/* Pending attr */
	int fa = COLOUR_WHITE;

	int oa;
	wchar_t oc;

	int na;
	wchar_t nc;


	/* Scan "modified" columns */
	for (x = x1; x <= x2; x++) {
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* See what is desired there */
		na = scr_aa[x];
		nc = scr_cc[x];

		/* Handle unchanged grids */
		if ((na == oa) && (nc == oc)) {
			/* Flush */
			if (fn) 	{
				/* Draw pending chars (normal or black) */
				if (fa || always_text)
					(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
				else
					(void)((*Term->wipe_hook)(fx, y, fn));

				/* Forget */
				fn = 0;
			}

			/* Skip */
			continue;
		}

		/* Save new contents */
		old_aa[x] = na;
		old_cc[x] = nc;

		/* Notice new color */
		if (fa != na) {
			/* Flush */
			if (fn) {
				/* Draw the pending chars, erase leading spaces */
				if (fa || always_text)
					(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
				else
					(void)((*Term->wipe_hook)(fx, y, fn));

				/* Forget */
				fn = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Restart and Advance */
		if (fn++ == 0) fx = x;
	}

	/* Flush */
	if (fn) {
		/* Draw pending chars (normal or black) */
		if (fa || always_text)
			(void)((*Term->text_hook)(fx, y, fn, fa, &scr_cc[fx]));
		else
			(void)((*Term->wipe_hook)(fx, y, fn));
	}
}

/**
 * Mark a spot as needing refresh (see "Term_fresh")
 */
errr Term_mark(int x, int y)
{
	int *old_aa = Term->old->a[y];
	wchar_t *old_cc = Term->old->c[y];
	int *old_taa = Term->old->ta[y];
	wchar_t *old_tcc = Term->old->tc[y];

	/*
	 * using 0x80 as the blank attribute and an impossible value for
	 * the blank char is ok since this function is only called by tile
	 * functions, but ideally there should be a test to use the blank text
	 * attr/char pair
	 */
	old_aa[x] = 0x80; 
	old_cc[x] = 0;
	old_taa[x] = 0x80;
	old_tcc[x] = 0;

	/* Update bounds for modified region. */
	if (y < Term->y1) Term->y1 = y;
	if (y > Term->y2) Term->y2 = y;
	if (x < Term->x1[y]) Term->x1[y] = x;
	if (x > Term->x2[y]) Term->x2[y] = x;

	return (0);
}

uint8_t tile_width = 1;            /* Tile width in units of font width */
uint8_t tile_height = 1;           /* Tile height in units of font height */

/**
 * Helper variables for large cursor
 */
bool bigcurs = false;
bool smlcurs = true;


/**
 * Actually perform all requested changes to the window
 *
 * If absolutely nothing has changed, not even temporarily, or if the
 * current "Term" is not mapped, then this function will return 1 and
 * do absolutely nothing.
 *
 * Note that when "soft_cursor" is true, we erase the cursor (if needed)
 * whenever anything has changed, and redraw it (if needed) after all of
 * the screen updates are complete.  This will induce a small amount of
 * "cursor flicker" but only when the screen has been updated.  If the
 * screen is updated and then restored, you may still get this flicker.
 *
 * When "soft_cursor" is not true, we make the cursor invisible before
 * doing anything else if it is supposed to be invisible by the time we
 * are done, and we make it visible after moving it to its final location
 * after all of the screen updates are complete.
 *
 * Note that "Term_xtra(TERM_XTRA_CLEAR,0)" must erase the entire screen,
 * including the cursor, if needed, and may place the cursor anywhere.
 *
 * Note that "Term_xtra(TERM_XTRA_FROSH,y)" will be always be called
 * after any row "y" has been "flushed", unless the "Term->never_frosh"
 * flag is set, and "Term_xtra(TERM_XTRA_FRESH,0)" will be called after
 * all of the rows have been "flushed".
 *
 * Note the use of three different functions to handle the actual flush,
 * based on the settings of the "Term->always_pict" and "Term->higher_pict"
 * flags (see below).
 *
 * The three helper functions (above) work by collecting similar adjacent
 * grids into stripes, and then sending each stripe to "Term->pict_hook",
 * "Term->text_hook", or "Term->wipe_hook", based on the settings of the
 * "Term->always_pict" and "Term->higher_pict" flags, which select which
 * of the helper functions to call to flush each row.
 *
 * The helper functions currently "skip" any grids which already contain
 * the desired contents.  This may or may not be the best method, especially
 * when the desired content fits nicely into the current stripe.  For example,
 * it might be better to go ahead and queue them while allowed, but keep a
 * count of the "trailing skipables", then, when time to flush, or when a
 * "non skippable" is found, force a flush if there are too many skippables.
 *
 * Perhaps an "initialization" stage, where the "text" (and "attr")
 * buffers are "filled" with information, converting "blanks" into
 * a convenient representation, and marking "skips" with "zero chars",
 * and then some "processing" is done to determine which chars to skip.
 *
 * Currently, the helper functions are optimal for systems which prefer
 * to "print a char + move a char + print a char" to "print three chars",
 * and for applications that do a lot of "detailed" color printing.
 *
 * In the two "queue" functions, total "non-changes" are "pre-skipped".
 * The helper functions must also handle situations in which the contents
 * of a grid are changed, but then changed back to the original value,
 * and situations in which two grids in the same row are changed, but
 * the grids between them are unchanged.
 *
 * If the "Term->always_pict" flag is set, then "Term_fresh_row_pict()"
 * will be used instead of "Term_fresh_row_text()".  This allows all the
 * modified grids to be collected into stripes of attr/char pairs, which
 * are then sent to the "Term->pict_hook" hook, which can draw these pairs
 * in whatever way it would like.
 *
 * If the "Term->higher_pict" flag is set, then "Term_fresh_row_both()"
 * will be used instead of "Term_fresh_row_text()".  This allows all the
 * "special" attr/char pairs (in which both the attr and char have the
 * high-bit set) to be sent (one pair at a time) to the "Term->pict_hook"
 * hook, which can draw these pairs in whatever way it would like.
 *
 * Normally, the "Term_wipe()" function is used only to display "blanks" that
 * were induced by "Term_clear()" or "Term_erase()". Actually, the
 * "Term_wipe()" function is used to display all "black" text, such as the
 * default "spaces" created by "Term_clear()" and "Term_erase()".
 *
 * Note that the "Term->always_text" flag will disable the use of the
 * "Term_wipe()" function hook entirely, and force all text, even text
 * drawn in the color "black", to be explicitly drawn.  This is useful
 * for machines which implement "Term_wipe()" by just drawing spaces.
 *
 * Note that the "Term->always_pict" flag will disable the use of the
 * "Term_wipe()" function entirely, and force everything, even text
 * drawn in the attr "black", to be explicitly drawn.
 *
 * This function does nothing unless the "Term" is "mapped", which allows
 * certain systems to optimize the handling of "closed" windows.
 *
 * On systems with a "soft" cursor, we must explicitly erase the cursor
 * before flushing the output, if needed, to prevent a "jumpy" refresh.
 * The actual method for this is horrible, but there is very little that
 * we can do to simplify it efficiently.  XXX XXX XXX
 *
 * On systems with a "hard" cursor, we will "hide" the cursor before
 * flushing the output, if needed, to avoid a "flickery" refresh.  It
 * would be nice to *always* hide the cursor during the refresh, but
 * this might be expensive (and/or ugly) on some machines.
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

	int y1 = Term->y1;
	int y2 = Term->y2;

	term_win *old = Term->old;
	term_win *scr = Term->scr;


	/* Do nothing unless "mapped" */
	if (!Term->mapped_flag) return (1);


	/* Trivial Refresh */
	if ((y1 > y2) &&
	    (scr->cu == old->cu) &&
	    (scr->cv == old->cv) &&
	    (scr->cx == old->cx) &&
	    (scr->cy == old->cy) &&
	    (scr->cnx == old->cnx) &&
	    (scr->cny == old->cny) &&
	    !(Term->total_erase)) {
		/* Nothing */
		return (1);
	}


	/* Paranoia -- use "fake" hooks to prevent core dumps */
	if (!Term->curs_hook) Term->curs_hook = Term_curs_hack;
	if (!Term->bigcurs_hook) Term->bigcurs_hook = Term->curs_hook;
	if (!Term->wipe_hook) Term->wipe_hook = Term_wipe_hack;
	if (!Term->text_hook) Term->text_hook = Term_text_hack;
	if (!Term->pict_hook) Term->pict_hook = Term_pict_hack;


	/* Handle "total erase" */
	if (Term->total_erase) {
		/* Physically erase the entire window */
		Term_xtra(TERM_XTRA_CLEAR, 0);

		/* Hack -- clear all "cursor" data */
		old->cv = old->cu = false;
		old->cx = old->cy = 0;
		old->cnx = old->cny = 1;

		/* Wipe each row */
		for (y = 0; y < h; y++) {
			int *aa = old->a[y];
			wchar_t *cc = old->c[y];
			int *taa = old->ta[y];
			wchar_t *tcc = old->tc[y];

			/* Wipe each column */
			for (x = 0; x < w; x++) {
				/* Wipe each grid */
				*aa++ = COLOUR_WHITE;
				*cc++ = ' ';

				*taa++ = COLOUR_WHITE;
				*tcc++ = ' ';
			}
		}

		/* Redraw every row */
		Term->y1 = y1 = 0;
		Term->y2 = y2 = h - 1;

		/* Redraw every column */
		for (y = 0; y < h; y++) {
			Term->x1[y] = 0;
			Term->x2[y] = w - 1;
		}

		/* Forget "total erase" */
		Term->total_erase = false;
	}


	/* Cursor update -- Erase old Cursor */
	if (Term->soft_cursor) {
		/* Cursor was visible */
		if (!old->cu && old->cv) {
		        /*
		         * Fake a change at the old cursor position so that
		         * position will be redrawn along with any other
			 * changes.
			 */
			int mty = MAX(old->cy,
				MIN(old->cy + old->cny - 1, h - 1));
			int mtx = MAX(old->cx,
				MIN(old->cx + old->cnx - 1, w - 1));
			int ty;

			for (ty = old->cy; ty <= mty; ++ty) {
				int tx;

				for (tx = old->cx; tx <= mtx; ++tx) {
					old->c[ty][tx] = ~scr->c[ty][tx];
				}
				if (Term->x1[ty] > old->cx) {
					Term->x1[ty] = old->cx;
				}
				if (Term->x2[ty] < mtx) {
					Term->x2[ty] = mtx;
				}
			}
			if (y1 > old->cy) {
			    y1 = old->cy;
			}
			if (y2 < mty) {
			    y2 = mty;
			}
		}
	} else {
		/* Cursor will be invisible */
		if (scr->cu || !scr->cv)
			Term_xtra(TERM_XTRA_SHAPE, 0);
	}


	/* Something to update */
	if (y1 <= y2) {
		int **pr_drw;
		int ipr;

		if (Term->dblh_hook && (Term->always_pict ||
				Term->higher_pict)) {
			/*
			 * Have to track whether each location in the previous
			 * tile_height rows was redrawn.  First dimension in
			 * pr_drw will be treated circularly so there's no
			 * need for copying or swapping pointers.
			 */
			pr_drw = mem_alloc(tile_height * sizeof(*pr_drw));
			for (y = 0; y < tile_height; ++y) {
				pr_drw[y] = mem_zalloc(w * sizeof(**pr_drw));
			}
		} else {
			pr_drw = NULL;
		}

		/* Handle "icky corner" */
		if ((Term->icky_corner) && (y2 >= h - 1) && (Term->x2[h - 1] > w - 2))
			Term->x2[h - 1] = w - 2;


		/*
		 * Make the stored y bounds for the modified region empty.
		 * Do so before drawing so that Term_mark() calls from within
		 * the drawing hooks will adjust the bounds on the modified
		 * region for the next update.
		 */
		Term->y1 = h;
		Term->y2 = 0;

		/* Scan the "modified" rows */
		ipr = 0;
		for (y = y1; y <= y2; ++y) {
			int x1 = Term->x1[y];
			int x2 = Term->x2[y];

			/* Flush each "modified" row */
			if (x1 <= x2) {
				/*
				 * As above, set the bounds for the modified
				 * region to be empty before drawing.
				 */
				Term->x1[y] = w;
				Term->x2[y] = 0;

				/* Use "Term_pict()" - always, sometimes or never */
				if (Term->always_pict) {
					/* Flush the row */
					if (Term->dblh_hook) {
						Term_fresh_row_pict_dblh(
							y, x1, x2, pr_drw[ipr]);
						ipr = (ipr + 1) % tile_height;
					} else {
						Term_fresh_row_pict(y, x1, x2);
					}
				} else if (Term->higher_pict) {
					/* Flush the row */
					if (Term->dblh_hook) {
						Term_fresh_row_both_dblh(
							y, x1, x2, pr_drw[ipr]);
						ipr = (ipr + 1) % tile_height;
					} else {
						Term_fresh_row_both(y, x1, x2);
					}
				} else {
					/* Flush the row */
					Term_fresh_row_text(y, x1, x2);
				}
				/* Hack -- Flush that row (if allowed) */
				if (!Term->never_frosh) Term_xtra(TERM_XTRA_FROSH, y);
			} else if (pr_drw) {
				/*
				 * Remember that nothing was redrawn on that
				 * row.
				 */
				for (x = 0; x < w; ++x) {
					pr_drw[ipr][x] = 0;
				}
				ipr = (ipr + 1) % tile_height;
			}
		}

		if (pr_drw) {
			for (y = 0; y < tile_height; ++y) {
				mem_free(pr_drw[y]);
			}
			mem_free(pr_drw);
		}
	}


	/* Cursor update -- Show new Cursor */
	if (Term->soft_cursor) {
		/* Draw the (large or small) cursor */
		if (!scr->cu && scr->cv) {
			if ((((tile_width > 1)||(tile_height > 1)) &&
			     (!smlcurs) && (Term->saved == 0) && (scr->cy > 0))
			    || bigcurs) {
				(void)((*Term->bigcurs_hook)(scr->cx, scr->cy));
				scr->cnx = tile_width;
				scr->cny = tile_height;
			} else {
				(void)((*Term->curs_hook)(scr->cx, scr->cy));
				scr->cnx = 1;
				scr->cny = 1;
			}
		} else {
			scr->cnx = 1;
			scr->cny = 1;
		}
	} else {
		/* The cursor is useless or invisible ignore it, otherwise display */
		if (scr->cu) {
			/* Paranoia -- Put the cursor NEAR where it belongs */
			(void)((*Term->curs_hook)(w - 1, scr->cy));
		} else if (!scr->cv) {
			/* Paranoia -- Put the cursor where it belongs */
			(void)((*Term->curs_hook)(scr->cx, scr->cy));
		} else {
			/* Put the cursor where it belongs */
			(void)((*Term->curs_hook)(scr->cx, scr->cy));

			/* Make the cursor visible */
			Term_xtra(TERM_XTRA_SHAPE, 1);
		}

		scr->cnx = 1;
		scr->cny = 1;
	}

	/* Save the "cursor state" */
	old->cu = scr->cu;
	old->cv = scr->cv;
	old->cx = scr->cx;
	old->cy = scr->cy;
	old->cnx = scr->cnx;
	old->cny = scr->cny;

	/* Actually flush the output */
	Term_xtra(TERM_XTRA_FRESH, 0);

	/* Success */
	return (0);
}



/**
 * ------------------------------------------------------------------------
 * Output routines
 * ------------------------------------------------------------------------ */


/**
 * Set the cursor visibility
 */
errr Term_set_cursor(bool v)
{
	/* Already done */
	if (Term->scr->cv == v) return (1);

	/* Change */
	Term->scr->cv = v;

	/* Success */
	return (0);
}


/**
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


/**
 * At a given location, place an attr/char
 * Do not change the cursor position
 * No visual changes until "Term_fresh()".
 */
errr Term_draw(int x, int y, int a, wchar_t c)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify location */
	if ((x < 0) || (x >= w)) return (-1);
	if ((y < 0) || (y >= h)) return (-1);

	/* Paranoia -- illegal char */
	if (!c) return (-2);

	/* Queue it for later */
	Term_queue_char(Term, x, y, a, c, 0, 0);

	/* Success */
	return (0);
}


/**
 * Using the given attr, add the given char at the cursor.
 *
 * We return "-2" if the character is "illegal". XXX XXX
 *
 * We return "-1" if the cursor is currently unusable.
 *
 * We queue the given attr/char for display at the current
 * cursor location, and advance the cursor to the right,
 * marking it as unusable and returning "1" if it leaves
 * the screen, and otherwise returning "0".
 *
 * So when this function, or the following one, return a
 * positive value, future calls to either function will
 * return negative ones.
 */
errr Term_addch(int a, wchar_t c)
{
	int w = Term->wid;

	/* Handle "unusable" cursor */
	if (Term->scr->cu) return (-1);

	/* Paranoia -- no illegal chars */
	if (!c) return (-2);

	/* Queue the given character for display */
	Term_queue_char(Term, Term->scr->cx, Term->scr->cy, a, c, 0, 0);

	/* Advance the cursor */
	Term->scr->cx++;

	/* Success */
	if (Term->scr->cx < w) return (0);

	/* Note "Useless" cursor */
	Term->scr->cu = 1;

	/* Note "Useless" cursor */
	return (1);
}


/**
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
errr Term_addstr(int n, int a, const char *buf)
{
	int k;

	int w = Term->wid;

	errr res = 0;

	wchar_t s[1024];

	/* Copy to a rewriteable string */
 	text_mbstowcs(s, buf, 1024);

	/* Handle "unusable" cursor */
	if (Term->scr->cu) return (-1);

	/* Obtain maximal length */
	k = (n < 0) ? (w + 1) : n;

	/* Obtain the usable string length */
	for (n = 0; (n < k) && s[n]; n++) /* loop */;

	/* React to reaching the edge of the screen */
	if (Term->scr->cx + n >= w) res = n = w - Term->scr->cx;

	/* Queue the first "n" characters for display */
	Term_queue_chars(Term->scr->cx, Term->scr->cy, n, a, s);

	/* Advance the cursor */
	Term->scr->cx += n;

	/* Hack -- Notice "Useless" cursor */
	if (res) Term->scr->cu = 1;

	/* Success (usually) */
	return (res);
}


/**
 * Move to a location and, using an attr, add a char
 */
errr Term_putch(int x, int y, int a, wchar_t c)
{
	errr res;

	/* Move first */
	if ((res = Term_gotoxy(x, y)) != 0) return (res);

	/* Then add the char */
	if ((res = Term_addch(a, c)) != 0) return (res);

	/* Success */
	return (0);
}


/**
 * Move to a location and, using an attr, add a big tile
 */
void Term_big_putch(int x, int y, int a, wchar_t c)
{
	int hor, vert;

	/* Avoid warning */
	(void)c;

	/* No tall skinny tiles */
	if (tile_width > 1) {
		/* Horizontal first */
		for (hor = 0; hor < tile_width; hor++) {
			/* Queue dummy character */
			if (hor != 0) {
				if (a & 0x80)
					Term_putch(x + hor, y, 255, -1);
				else
					Term_putch(x + hor, y, COLOUR_WHITE, L' ');
			}

			/* Now vertical */
			for (vert = 1; vert < tile_height; vert++) {
				/* Queue dummy character */
				if (a & 0x80)
					Term_putch(x + hor, y + vert, 255, -1);
				else
					Term_putch(x + hor, y + vert, COLOUR_WHITE, L' ');
			}
		}
	} else {
		/* Only vertical */
		for (vert = 1; vert < tile_height; vert++) {
			/* Queue dummy character */
			if (a & 0x80)
				Term_putch(x, y + vert, 255, -1);
			else
				Term_putch(x, y + vert, COLOUR_WHITE, L' ');
		}
	}
}


/**
 * Move to a location and, using an attr, add a string
 */
errr Term_putstr(int x, int y, int n, int a, const char *s)
{
	errr res;

	if (!Term)
		return 0;

	/* Move first */
	if ((res = Term_gotoxy(x, y)) != 0) return (res);

	/* Then add the string */
	if ((res = Term_addstr(n, a, s)) != 0) return (res);

	/* Success */
	return (0);
}



/**
 * Place cursor at (x,y), and clear the next "n" chars
 */
errr Term_erase(int x, int y, int n)
{
	int i;

	int w = Term->wid;
	/* int h = Term->hgt; */

	int x1 = -1;
	int x2 = -1;

	int *scr_aa;
	wchar_t *scr_cc;

	int *scr_taa;
	wchar_t *scr_tcc;

	/* Place cursor */
	if (Term_gotoxy(x, y)) return (-1);

	/* Force legal size */
	if (x + n > w) n = w - x;

	/* Fast access */
	scr_aa = Term->scr->a[y];
	scr_cc = Term->scr->c[y];

	scr_taa = Term->scr->ta[y];
	scr_tcc = Term->scr->tc[y];

	/* Scan every column */
	for (i = 0; i < n; i++, x++) {
		int oa = scr_aa[x];
		wchar_t oc = scr_cc[x];

		/* Hack -- Ignore "non-changes" */
		if ((oa == COLOUR_WHITE) && (oc == ' ')) continue;

		/* Save the "literal" information */
		scr_aa[x] = COLOUR_WHITE;
		scr_cc[x] = ' ';

		scr_taa[x] = 0;
		scr_tcc[x] = 0;

		/* Track minimum changed column */
		if (x1 < 0) x1 = x;

		/* Track maximum changed column */
		x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0) {
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


/**
 * Clear the entire window, and move to the top left corner
 *
 * Note the use of the special "total_erase" code
 */
errr Term_clear(void)
{
	int x, y;

	int w = Term->wid;
	int h = Term->hgt;

	/* Cursor usable */
	Term->scr->cu = 0;

	/* Cursor to the top left */
	Term->scr->cx = Term->scr->cy = 0;

	/* Wipe each row */
	for (y = 0; y < h; y++) {
		int *scr_aa = Term->scr->a[y];
		wchar_t *scr_cc = Term->scr->c[y];
		int *scr_taa = Term->scr->ta[y];
		wchar_t *scr_tcc = Term->scr->tc[y];

		/* Wipe each column */
		for (x = 0; x < w; x++) {
			scr_aa[x] = COLOUR_WHITE;
			scr_cc[x] = ' ';

			scr_taa[x] = 0;
			scr_tcc[x] = 0;
		}

		/* This row has changed */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Every row has changed */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Force "total erase" */
	Term->total_erase = true;

	/* Success */
	return (0);
}





/**
 * Redraw (and refresh) the whole window.
 */
errr Term_redraw(void)
{
	/* Force "total erase" */
	Term->total_erase = true;

	/* Hack -- Refresh */
	Term_fresh();

	/* Success */
	return (0);
}


/**
 * Redraw part of a window.
 */
errr Term_redraw_section(int x1, int y1, int x2, int y2)
{
	int i, j;

	wchar_t *c_ptr;

	/* Bounds checking */
	if (y2 >= Term->hgt) y2 = Term->hgt - 1;
	if (x2 >= Term->wid) x2 = Term->wid - 1;
	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;


	/* Set y limits */
	Term->y1 = y1;
	Term->y2 = y2;

	/* Set the x limits */
	for (i = Term->y1; i <= Term->y2; i++) {
		if ((x1 > 0) && (Term->old->a[i][x1] == 255))
			x1--;

		Term->x1[i] = x1;
		Term->x2[i] = x2;

		c_ptr = Term->old->c[i];

		/* Clear the section so it is redrawn */
		for (j = x1; j <= x2; j++) {
			/* Hack - set the old character to "none" */
			c_ptr[j] = 0;
		}
	}

	/* Hack -- Refresh */
	Term_fresh();

	/* Success */
	return (0);
}





/**
 * ------------------------------------------------------------------------
 * Access routines
 * ------------------------------------------------------------------------ */


/**
 * Extract the cursor visibility
 */
errr Term_get_cursor(bool *v)
{
	/* Extract visibility */
	(*v) = Term->scr->cv;

	/* Success */
	return (0);
}


/**
 * Extract the current window size
 */
errr Term_get_size(int *w, int *h)
{
	*w = Term ? Term->wid : 80;
	*h = Term ? Term->hgt : 24;
	return 0;
}


/**
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


/**
 * At a given location, determine the "current" attr and char
 * Note that this refers to what will be on the window after the
 * next call to "Term_fresh()".  It may or may not already be there.
 */
errr Term_what(int x, int y, int *a, wchar_t *c)
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



/**
 * ------------------------------------------------------------------------
 * Input routines
 * ------------------------------------------------------------------------ */


/**
 * Flush and forget the input
 */
errr Term_flush(void)
{
	if (!Term)
		return 0;

	/* Hack -- Flush all events */
	Term_xtra(TERM_XTRA_FLUSH, 0);

	/* Forget all keypresses */
	Term->key_head = Term->key_tail = 0;

	/* Success */
	return (0);
}


/**
 * sketchy keylogging pt. 2
 */
static void log_keypress(ui_event e)
{
	if (e.type != EVT_KBRD) return;
	if (!e.key.code) return;

	keylog[log_i] = e.key;
	if (log_size < KEYLOG_SIZE) log_size++;
	log_i = (log_i + 1) % KEYLOG_SIZE;
}


/**
 * Add a keypress to the "queue"
 */
errr Term_keypress(keycode_t k, uint8_t mods)
{
	/* Hack -- Refuse to enqueue non-keys */
	if (!k) return (-1);

	if(!Term->complex_input) {
		switch (k)
		{
			case '\r':
			case '\n':
			  	k = KC_ENTER;
			  	break;
			case 8:
			  	k = KC_BACKSPACE;
			  	break;
			case 9:
			  	k = KC_TAB;
			  	break;
			case 27:
			  	k = ESCAPE;
			  	break;
		}
	}

	/* Store the char, advance the queue */
	Term->key_queue[Term->key_head].type = EVT_KBRD;
	Term->key_queue[Term->key_head].key.code = k;
	Term->key_queue[Term->key_head].key.mods = mods;
	Term->key_head++;

	/* Circular queue, handle wrap */
	if (Term->key_head == Term->key_size) Term->key_head = 0;

	/* Success (unless overflow) */
	if (Term->key_head != Term->key_tail) return (0);

	/* Problem */
	return (1);
}

/**
 * Add a mouse event to the "queue"
 */
errr Term_mousepress(int x, int y, char button)/*, uint8_t mods);*/
{
	/* Store the char, advance the queue */
	Term->key_queue[Term->key_head].type = EVT_MOUSE;
	Term->key_queue[Term->key_head].mouse.x = x;
	Term->key_queue[Term->key_head].mouse.y = y;
	/* XXX for now I encode the mods into the button number, so I would
	 * not have to worry about the other platforms, when all platforms set
	 * mods, this code should be replaced with :
	 * Term->key_queue[Term->key_head].mouse.button = button;
	 * Term->key_queue[Term->key_head].mouse.mods = mods;
	 */
	Term->key_queue[Term->key_head].mouse.button = (button & 0x0F);
	Term->key_queue[Term->key_head].mouse.mods = ((button & 0xF0)>>4);

	Term->key_head++;

	/* Circular queue, handle wrap */
	if (Term->key_head == Term->key_size) Term->key_head = 0;

	/* Success (unless overflow) */
	if (Term->key_head != Term->key_tail) return (0);
  
	/* Problem */
	return (1);
}


/**
 * Add a keypress to the FRONT of the "queue"
 */
errr Term_key_push(int k)
{
	ui_event ke;

	if (!k) return (-1);

	ke.type = EVT_KBRD;
	ke.key.code = k;
	ke.key.mods = 0;

	return Term_event_push(&ke);
}

errr Term_event_push(const ui_event *ke)
{
	/* Hack -- Refuse to enqueue non-keys */
	if (!ke) return (-1);

	/* Hack -- Overflow may induce circular queue */
	if (Term->key_tail == 0) Term->key_tail = Term->key_size;

	/* Back up, Store the char */
	/* Store the char, advance the queue */
	Term->key_queue[--Term->key_tail] = *ke;

	/* Success (unless overflow) */
	if (Term->key_head != Term->key_tail) return (0);

	/* Problem */
	return (1);
}





/**
 * Check for a pending keypress on the key queue.
 *
 * Store the keypress, if any, in "ch", and return "0".
 * Otherwise store "zero" in "ch", and return "1".
 *
 * Wait for a keypress if "wait" is true.
 *
 * Remove the keypress if "take" is true.
 */
errr Term_inkey(ui_event *ch, bool wait, bool take)
{
	/* Assume no key */
	memset(ch, 0, sizeof *ch);

	/* Hack -- get bored */
	if (!Term->never_bored)
		/* Process random events */
		Term_xtra(TERM_XTRA_BORED, 0);

	/* Wait or not */
	if (wait)
		/* Process pending events while necessary */
		while (Term->key_head == Term->key_tail)
			/* Process events (wait for one) */
			Term_xtra(TERM_XTRA_EVENT, true);
	else
		/* Process pending events if necessary */
		if (Term->key_head == Term->key_tail)
			/* Process events (do not wait) */
			Term_xtra(TERM_XTRA_EVENT, false);

	/* No keys are ready */
	if (Term->key_head == Term->key_tail) return (1);

	/* Extract the next keypress */
	(*ch) = Term->key_queue[Term->key_tail];

	/* sketchy key loggin */
	log_keypress(*ch);

	/* If requested, advance the queue, wrap around if necessary */
	if (take && (++Term->key_tail == Term->key_size)) Term->key_tail = 0;

	/* Success */
	return (0);
}



/**
 * ------------------------------------------------------------------------
 * Extra routines
 * ------------------------------------------------------------------------ */

/**
 * Save the "requested" screen into the "memorized" screen
 *
 * Every "Term_save()" should match exactly one "Term_load()" or
 * "Term_load_all()"
 */
errr Term_save(void)
{
	int w = Term->wid;
	int h = Term->hgt;

	term_win *mem;

	/* Allocate window */
	mem = mem_zalloc(sizeof(term_win));

	/* Initialize window */
	term_win_init(mem, w, h);

	/* Grab */
	term_win_copy(mem, Term->scr, w, h);

	/* Front of the queue */
	mem->next = Term->mem;
	Term->mem = mem;

	/* One more saved */
	Term->saved++;

	/* Success */
	return (0);
}


/**
 * Restore the "requested" contents (see above).
 *
 * Every "Term_save()" should match exactly one "Term_load()" or
 * "Term_load_all()"
 */
errr Term_load(void)
{
	int y;

	int w = Term->wid;
	int h = Term->hgt;

	term_win *tmp;

	/* Pop off window from the list */
	if (Term->mem) {
		/* Save pointer to old mem */
		tmp = Term->mem;

		/* Forget it */
		Term->mem = Term->mem->next;

		/* Load */
		term_win_copy(Term->scr, tmp, w, h);

		/* Free the old window */
		(void)term_win_nuke(tmp);

		/* Kill it */
		mem_free(tmp);
	}

	/* Assume change */
	for (y = 0; y < h; y++) {
		/* Assume change */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* One less saved */
	Term->saved--;

	/* Success */
	return (0);
}


/**
 * Restore the "requested" contents (see above).  Differs from Term_load() in
 * that all the previous saves are replayed from earliest to latest with a
 * redraw for each.  That is useful for accurately restoring the display if
 * the saves include partially overwritten big tiles.
 *
 * Every "Term_save()" should match exactly one "Term_load()" or
 * "Term_load_all()"
 */
errr Term_load_all(void)
{
	int w = Term->wid;
	int h = Term->hgt;
	struct reversed_save {
		term_win *saved_mem; struct reversed_save *next;
	} *reversed_list = NULL;
	struct term_win *cursor;

	for (cursor = Term->mem; cursor; cursor = cursor->next) {
		struct reversed_save *new_head = mem_alloc(sizeof(*new_head));

		new_head->saved_mem = cursor;
		new_head->next = reversed_list;
		reversed_list = new_head;
	}

	while (reversed_list) {
		struct reversed_save *tgt = reversed_list;
		int y;

		reversed_list = reversed_list->next;
		term_win_copy(Term->scr, tgt->saved_mem, w, h);
		mem_free(tgt);

		/* Assume change */
		for (y = 0; y < h; y++) {
			/* Assume change */
			Term->x1[y] = 0;
			Term->x2[y] = w - 1;
		}
		Term->y1 = 0;
		Term->y2 = h - 1;

		/* Force a redraw with those contents. */
		Term_fresh();
	}

	/* Drop the most recent save. */
	if (Term->mem) {
		cursor = Term->mem;
		Term->mem = Term->mem->next;
		term_win_nuke(cursor);
		mem_free(cursor);
	}
	Term->saved--;

	/* Success */
	return (0);
}


/**
 * React to a new physical window size.
 */
errr Term_resize(int w, int h)
{
	int i;

	int wid, hgt;

	int *hold_x1;
	int *hold_x2;

	term_win *hold_old;
	term_win *hold_scr;
	term_win *hold_mem;
	term_win **hold_mem_dest;
	term_win *hold_tmp;

	ui_event evt = EVENT_EMPTY;
	evt.type = EVT_RESIZE;

	/* Resizing is forbidden */
	if (Term->fixed_shape) return (-1);

	/* Ignore illegal changes */
	if ((w < 1) || (h < 1)) return (-1);

	/* Ignore non-changes */
	if ((Term->wid == w) && (Term->hgt == h)) return (1);

	/* Minimum dimensions */
	wid = MIN(Term->wid, w);
	hgt = MIN(Term->hgt, h);

	/* Save scanners */
	hold_x1 = Term->x1;
	hold_x2 = Term->x2;

	/* Save old window */
	hold_old = Term->old;

	/* Save old window */
	hold_scr = Term->scr;

	/* Save old window */
	hold_mem = Term->mem;

	/* Save old window */
	hold_tmp = Term->tmp;

	/* Create new scanners */
	Term->x1 = mem_zalloc(h * sizeof(int));
	Term->x2 = mem_zalloc(h * sizeof(int));

	/* Create new window */
	Term->old = mem_zalloc(sizeof(term_win));

	/* Initialize new window */
	term_win_init(Term->old, w, h);

	/* Save the contents */
	term_win_copy(Term->old, hold_old, wid, hgt);

	/* Create new window */
	Term->scr = mem_zalloc(sizeof(term_win));

	/* Initialize new window */
	term_win_init(Term->scr, w, h);

	/* Save the contents */
	term_win_copy(Term->scr, hold_scr, wid, hgt);

	/* If needed */
	hold_mem_dest = &Term->mem;
	while (hold_mem != 0) {
		term_win* trash;

		/* Create new window */
		*hold_mem_dest = mem_zalloc(sizeof(term_win));

		/* Initialize new window */
		term_win_init(*hold_mem_dest, w, h);

		/* Save the contents */
		term_win_copy(*hold_mem_dest, hold_mem, wid, hgt);

		trash = hold_mem;
		hold_mem = hold_mem->next;

		if ((*hold_mem_dest)->cx >= w) (*hold_mem_dest)->cu = 1;
		if ((*hold_mem_dest)->cy >= h) (*hold_mem_dest)->cu = 1;

		hold_mem_dest = &((*hold_mem_dest)->next);

		term_win_nuke(trash);
		mem_free(trash);
	}

	/* If needed */
	if (hold_tmp) {
		/* Create new window */
		Term->tmp = mem_zalloc(sizeof(term_win));

		/* Initialize new window */
		term_win_init(Term->tmp, w, h);

		/* Save the contents */
		term_win_copy(Term->tmp, hold_tmp, wid, hgt);
	}

	/* Free some arrays */
	mem_free(hold_x1);
	mem_free(hold_x2);

	/* Nuke */
	term_win_nuke(hold_old);

	/* Kill */
	mem_free(hold_old);

	/* Illegal cursor */
	if (Term->old->cx >= w) Term->old->cu = 1;
	if (Term->old->cy >= h) Term->old->cu = 1;

	/* Nuke */
	term_win_nuke(hold_scr);

	/* Kill */
	mem_free(hold_scr);

	/* Illegal cursor */
	if (Term->scr->cx >= w) Term->scr->cu = 1;
	if (Term->scr->cy >= h) Term->scr->cu = 1;

	/* If needed */
	if (hold_tmp) {
		/* Nuke */
		term_win_nuke(hold_tmp);

		/* Kill */
		mem_free(hold_tmp);

		/* Illegal cursor */
		if (Term->tmp->cx >= w) Term->tmp->cu = 1;
		if (Term->tmp->cy >= h) Term->tmp->cu = 1;
	}

	/* Save new size */
	Term->wid = w;
	Term->hgt = h;

	/* Force "total erase" */
	Term->total_erase = true;

	/* Assume change */
	for (i = 0; i < h; i++) {
		/* Assume change */
		Term->x1[i] = 0;
		Term->x2[i] = w - 1;
	}

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Push a resize event onto the stack */
	Term_event_push(&evt);

	/* Success */
	return (0);
}



/**
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
	if (t && !t->active_flag) {
		/* Call the "init" hook */
		if (t->init_hook) (*t->init_hook)(t);

		/* Remember */
		t->active_flag = true;

		/* Assume mapped */
		t->mapped_flag = true;
	}

	/* Remember the Term */
	Term = t;

	/* Activate the new Term */
	if (Term) Term_xtra(TERM_XTRA_LEVEL, 1);

	/* Success */
	return (0);
}



/**
 * Nuke a term
 */
errr term_nuke(term *t)
{
	/* Hack -- Call the special "nuke" hook */
	if (t->active_flag) {
		/* Call the "nuke" hook */
		if (t->nuke_hook) (*t->nuke_hook)(t);

		/* Remember */
		t->active_flag = false;

		/* Assume not mapped */
		t->mapped_flag = false;
	}


	/* Nuke "displayed" */
	term_win_nuke(t->old);

	/* Kill "displayed" */
	mem_free(t->old);

	/* Nuke "requested" */
	term_win_nuke(t->scr);

	/* Kill "requested" */
	mem_free(t->scr);

	/* If needed */
	if (t->mem) {
		/* Nuke "memorized" */
		term_win_nuke(t->mem);

		/* Kill "memorized" */
		mem_free(t->mem);
	}

	/* If needed */
	if (t->tmp) {
		/* Nuke "temporary" */
		term_win_nuke(t->tmp);

		/* Kill "temporary" */
		mem_free(t->tmp);
	}

	/* Free some arrays */
	mem_free(t->x1);
	mem_free(t->x2);

	/* Free the input queue */
	mem_free(t->key_queue);

	/* Success */
	return (0);
}


/**
 * Initialize a term, using a window of the given size.
 * Also prepare the "input queue" for "k" keypresses
 * By default, the cursor starts out "invisible"
 * By default, we "erase" using "black spaces"
 */
errr term_init(term *t, int w, int h, int k)
{
	int y;

	/* Wipe it */
	memset(t, 0, sizeof(term));

	/* Prepare the input queue */
	t->key_head = t->key_tail = 0;

	/* Determine the input queue size */
	t->key_size = k;

	/* Allocate the input queue */
	t->key_queue = mem_zalloc(t->key_size * sizeof(ui_event));

	/* Save the size */
	t->wid = w;
	t->hgt = h;

	/* Allocate change arrays */
	t->x1 = mem_zalloc(h * sizeof(int));
	t->x2 = mem_zalloc(h * sizeof(int));


	/* Allocate "displayed" */
	t->old = mem_zalloc(sizeof(term_win));

	/* Initialize "displayed" */
	term_win_init(t->old, w, h);


	/* Allocate "requested" */
	t->scr = mem_zalloc(sizeof(term_win));

	/* Initialize "requested" */
	term_win_init(t->scr, w, h);

	/* Assume change */
	for (y = 0; y < h; y++) {
		/* Assume change */
		t->x1[y] = 0;
		t->x2[y] = w - 1;
	}

	/* Assume change */
	t->y1 = 0;
	t->y2 = h - 1;

	/* Force "total erase" */
	t->total_erase = true;

	/* No saves yet */
	t->saved = 0;

	t->sidebar_mode = SIDEBAR_LEFT;

	/* Success */
	return (0);
}

/**
 * Emit a 'graphical' symbol and a padding character if appropriate
 */
int big_pad(int col, int row, uint8_t a, wchar_t c)
{
	Term_putch(col, row, a, c);

	if ((tile_width > 1) || (tile_height > 1))
		Term_big_putch(col, row, a, c);

	return tile_width;
}

/**
 * For the given terminal, return the first row where tiles may be rendered.
 * \param t Is the terminal to be queried.
 */
int Term_get_first_tile_row(term *t)
{
	int result;

	if (t == angband_term[0]) {
		/*
		 * In the main window, there's no tiles in the top bar, does
		 * not account for the case where the main window is used as
		 * the target for display_map() or displays tiles in the
		 * knowledge menus.
		 */
		result = ROW_MAP;
	} else {
		/* In other windows, have to check the flags. */
		int i = 1;

		while (1) {
			if (i >= ANGBAND_TERM_MAX) {
				/*
				 * Don't know the flags.  Err on the side of
				 * drawing too few tiles.
				 */
				result = 1;
				break;
			}
			if (t == angband_term[i]) {
				if (window_flag[i] & PW_OVERHEAD) {
					/*
					 * All rows are valid targets for
					 * tiles.
					 */
					result = 0;
				} else {
					/*
					 * It's presumably a minimap view where
					 * the first row has a non-tile border.
					 */
					result = 1;
				}
				break;
			}
			++i;
		}
	}
	return result;
}
