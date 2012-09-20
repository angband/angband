/* File: term.c */

/* Purpose: generic, efficient, modular, terminal routines */

#include "term.h"

#include "z-virt.h"


/*
 * Some ideas may have been adapted from MacAngband 2.6.1
 *
 * Note that there is a rather large difference between the newer
 * "graphic based" machines and the older "text based" terminals.
 * Luckily, the only significant one for a game like "Angband"
 * is that the older machines have a "hardware cursor".
 *
 * There are also a few "tuning" issues, for example, the older
 * machines traditionally have very "expensive" routines to check
 * for a keypress without actually "waiting" for it.  So these
 * routines must be explicitly "requested".
 *
 * New machines:
 *   movement" across the screen is relatively cheap
 *   changing "colors" may take a little bit of time
 *   characters must be "erased" from the screen
 *   drawing characters may or may not first erase behind them
 *   drawing a character on the cursor will clear the cursor
 *
 * Old machines:
 *   expensive cursor movement
 *   hardware cursor (moves at every "print")
 *   printing characters erases the previous contents
 *   may have "fast" routines for "clear to end of line"
 *   may have "fast" routines for "clear entire screen"
 *   
 *
 * Note that currently, the colors are "semi-hard-coded", that is,
 * while the "term.c" package assumes only that "color zero" is
 * the "background color", but the packages that supply our "support"
 * probably make very distinct assumptions about them.  So the
 * "defines" in "term.h" can be ignored, as long as the "support"
 * files are told how to display each color.
 *
 * Note that we reserve the "zero char" for later developement,
 * and that we assume that "ascii 32" is to be used as the "space",
 * or "invisible" character, and that its color does NOT matter.
 * This will need to be changed if we start supporting "attributes"
 * with "background colors" (ala IBM).
 *
 * Note that we can easily allow the "term_win" structure to be
 * exported, allowing users to create various "user defined"
 * routines, such as "scrolling packages".  These routines would
 * not tie in the the "terminal capabilities", but they would
 * be completely portable...  We should add a "TERM_COLOR" define
 * to remove all the "color" code from the system (to save space).
 *
 * We should teach "Term_fresh()" to be a little bit "smarter".
 * In particular, it might help to "pre-process" things like
 * "clear to the end of the line" and such.  Or maybe this could
 * be done by the "Term_text()" procedures themselves...
 *
 * Be VERY careful about printing in the bottom right hand corner
 * of the screen, as it may induce "scrolling" on older machines.
 *
 * Note that we must receive (significant) external support, from
 * a file such as "main-mac.c" or "main-x11.c" or "main-cur.c".  This
 * support takes the form of four function hooks:
 *
 *   Term_xtra() = Perform various actions
 *   Term_curs() = Draw or Move the cursor
 *   Term_wipe() = Erase (part of) the screen
 *   Term_text() = Place some text on the screen
 *
 * We provide, among other things, the functions Term_keypress()
 * to "react" to keypress events, and Term_redraw() to redraw the
 * entire screen, plus "Term_resize()" to note a new size.
 */



/*
 * A macro to determine if an attr/char pair looks "blank"
 * Note that the "black" attribute is "reserved", and that
 * currently, the "space" character is hard-coded as "blank".
 */
#define BLANK(A,C)	(((A)==0) || ((C)==' '))



/* Next "screen memory slot" to use */
static int mem_depth = 0;

/* Semi-Hack -- The "memory" screens */
static term_win mem_array[MEM_SIZE];



/* The current "term" */
term *Term = NULL;




/*
 * Completely "erase" a "term_win".  Mark as "changed".
 * Note that the "cursor visibility" is turned off,
 * and the cursor is moved to the top left corner.
 */
errr term_win_wipe(term_win *t)
{
    int y, x;

    /* Cursor to the top left, and invisible */
    t->cv = t->cu = t->cx = t->cy = 0;

    /* Scan every row */
    for (y = 0; y < t->h; y++)
    {
	/* Wipe this row */
	for (x = 0; x < t->w; x++)
	{
	    tw_a(t,x,y) = 0;
	    tw_c(t,x,y) = ' ';
	}

	/* This row has changed */
	t->x1[y] = 0;
	t->x2[y] = t->w - 1;
    }

    /* Every row has changed */
    t->y1 = 0;
    t->y2 = t->h - 1;

    /* Success */
    return (0);
}




/*
 * Load a "term_win()" from another
 */
errr term_win_load(term_win *t, term_win *s)
{
    int y, x;

    int w = MIN(t->w, s->w);
    int h = MIN(t->h, s->h);
    
    /* Copy all the data from "s" into "t" */
    for (y = 0; y < h; y++)
    {
	for (x = 0; x < w; x++)
	{
	    tw_a(t,x,y) = tw_a(s,x,y);
	    tw_c(t,x,y) = tw_c(s,x,y);
	}
    }

    /* Load the "cursor state" */
    t->cx = s->cx;
    t->cy = s->cy;
    t->cu = s->cu;
    t->cv = s->cv;

    /* XXX Hack -- prevent cursor errors */
    if (t->cx > t->w - 1) t->cx = t->w - 1;
    if (t->cy > t->h - 1) t->cy = t->h - 1;

    /* Every row may have changed */
    t->y1 = 0;
    if (t->y2 < h - 1) t->y2 = h - 1;

    /* Every col of every row may have changed */
    for (y = 0; y < h; y++)
    {
	t->x1[y] = 0;
	if (t->x2[y] < w - 1) t->x2[y] = w - 1;
    }

    /* Success */
    return (0);
}


/*
 * Resize a term_win -- Hack -- not very efficient.
 */
static errr term_win_resize(term_win *tw, int w, int h)
{
    term_win hack;
    
    /* Ignore non-changes */
    if ((tw->w == w) && (tw->h == h)) return (1);

    /* Hack -- steal the term_win via structure copy */
    hack = (*tw);

    /* Init ourself to the new size */
    term_win_init(tw, w, h);

    /* Recopy the contents */
    term_win_load(tw, &hack);    
    
    /* Hack -- nuke the copy of our old contents */
    term_win_nuke(&hack);

    return (0);
}



/*
 * Nuke a term_win
 */
errr term_win_nuke(term_win *t)
{
    /* Free the arrays */
    C_KILL(t->a, t->w * t->h, byte);
    C_KILL(t->c, t->w * t->h, char);
    C_KILL(t->x1, t->w, byte);
    C_KILL(t->x2, t->w, byte);
    
    return (0);
}


/*
 * Initialize a "term_win" (using the given screen size)
 */
errr term_win_init(term_win *t, int w, int h)
{
    /* Save the size */
    t->w = w;
    t->h = h;

    /* Allocate the main arrays */
    C_MAKE(t->a, w*h, byte);
    C_MAKE(t->c, w*h, char);

    /* Allocate the change arrays */
    C_MAKE(t->x1, w, byte);
    C_MAKE(t->x2, w, byte);

    /* XXX Allocate the "max used col" array */
    /* C_MAKE(t->rm, w, byte); */

    /* Wipe it */
    term_win_wipe(t);

    /* Success */
    return (0);
}




/*
 * Perform the "extra action" of type "n" with value "v".
 * Valid actions are defined as the "TERM_XTRA_*" constants.
 */
errr Term_xtra(int n, int v)
{
    if (!Term->xtra_hook) return (-1);
    return ((*Term->xtra_hook)(n, v));
}

/*
 * Draw a "cursor" at "(x,y)".  Later, "z" may mean something.
 * Note that "Term_what(x,y,&a,&c)" will have "usable" results.
 * The input is assumed to be "valid".
 */
errr Term_curs(int x, int y, int z)
{
    if (!Term->curs_hook) return (-1);
    return ((*Term->curs_hook)(x, y, z));
}

/*
 * Erase a "block" of chars starting at "(x,y)", with size "(w,h)"
 * The input is assumed to be "valid".
 */
errr Term_wipe(int x, int y, int w, int h)
{
    if (!Term->wipe_hook) return (-1);
    return ((*Term->wipe_hook)(x, y, w, h));
}

/*
 * Draw "n" chars from the string "s" using attr "a", at location "(x,y)"
 * The input is assumed to be "valid".  The length is assumed to be positive.
 */
errr Term_text(int x, int y, int n, byte a, cptr s)
{
    if (!Term->text_hook) return (-1);
    return ((*Term->text_hook)(x, y, n, a, s));
}





/*
 * Medium level graphics.  Assumes valid input.
 *
 * Flush a row of the current window
 * Method: collect similar adjacent entries into stripes
 *
 * This routine currently "skips" any locations which "appear"
 * to already contain the desired contents.  This may or may
 * not be the best method, especially when the desired content
 * fits nicely into a "strip" currently under construction...
 *
 * Currently, this function skips right over any characters which
 * are already present on the screen.  I imagine that it would be
 * more efficient to NOT skip such characters all the time, but
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
 * So there are some "screen writes" which never even make it to this function.
 *
 * The old method pre-extracted "spaces" into a "efficient" form of
 * "erasing", but for various reasons, we will simply "print" them, and
 * let the "Term_text()" procedure handle "space extraction".
 * Note especially the frequency of "trailing spaces" in the text which
 * is sent to "Term_text()".  Combined with trailing spaces already on
 * the screen, it should be possible to "optimize" screen clearing.  We
 * will need a new field "max column used" per row.
 *
 * Hack -- we do NOT terminate the threads, since "Term_text()" must be
 * able to handle "non-terminated" text.
 */
static void FlushOutputRow(int y)
{
    register int x;

    /* No chars "pending" in "text" */
    int n = 0;

    /* Pending text starts in the first column */
    int fx = 0;

    /* Pending text is "black" */
    int fa = 0;

    int oa, oc, na, nc;

    bool can_skip;

    /* Max width is 255 */
    char text[256];


    /* Scan the columns marked as "modified" */
    for (x = Term->scr->x1[y]; x <= Term->scr->x2[y]; x++)
    {
	/* See what SEEMS TO BE there */
	oa = tw_a(Term->old,x,y);
	oc = tw_c(Term->old,x,y);

	/* See what SHOULD BE there */
	na = tw_a(Term->scr,x,y);
	nc = tw_c(Term->scr,x,y);

	/* Save the "actual" info mentally */
	tw_a(Term->old,x,y) = na;
	tw_c(Term->old,x,y) = nc;

	/* Hack -- color "bleeds" into holes */
	if (BLANK(oa,oc)) oa = fa, oc = ' ';
	if (BLANK(na,nc)) na = fa, nc = ' ';

	/* Note if no "visual changes" have occured */
	can_skip = ((na == oa) && (nc == oc));

	/* Flush as needed (see above) */
	if (n && (can_skip || (fa != na)))
	{
	    /* Draw the pending chars (if not spaces) */
	    if (fa) Term_text(fx, y, n, fa, text);

	    /* Erase behind the characters */
	    else Term_wipe(fx, y, n, 1);

	    /* Forget the pending thread */
	    n = 0;
	}

	/* Skip all grids which still "look like" they used to */
	if (can_skip) continue;

	/* Start a new thread, if needed */
	if (!n) fx = x, fa = na;

	/* Expand the current thread */
	text[n++] = nc;
    }

    /* Flush any leftover threads */
    if (n)
    {
	/* Draw the pending chars (unless invisible) */
	if (fa) Term_text(fx, y, n, fa, text);

	/* Otherwise just Erase them */
	else Term_wipe(fx, y, n, 1);
    }

    /* This row is all done */
    Term->scr->x1[y] = Term->scr->w;
    Term->scr->x2[y] = 0;
}



/*
 * Check to see if the old cursor will get erased by FlushOutputRow()'s
 */
static void FlushOutputHack()
{
    int cx, cy, oa, oc, na, nc;

    /* Cursor was offscreen, ignore it */
    if (Term->old->cu) return;

    /* Cursor was invisible, ignore it */
    if (!Term->old->cv) return;

    /* Extract the cursor location */
    cy = Term->old->cy;
    cx = Term->old->cx;

    /* Cursor row was unaffected, ignore it */
    if ((Term->scr->y1 > cy) || (Term->scr->y2 < cy)) return;

    /* Cursor column in the Cursor row was unaffected, ignore it */
    if ((Term->scr->x1[cy] > cx) || (Term->scr->x2[cy] < cx)) return;

    /* Access the Term->old contents */
    oa = tw_a(Term->old,cx,cy);
    oc = tw_c(Term->old,cx,cy);

    /* Access the new contents */
    na = tw_a(Term->scr,cx,cy);
    nc = tw_c(Term->scr,cx,cy);

    /* If the contents are the same, cursor may be skipped */
    if ((oa == na) && (oc == nc)) return;

    /* If the contents are both "blank", cursor may be skipped */
    if (BLANK(oa,oc) && BLANK(na,nc)) return;

    /* Hack -- The cursor will be erased, pretend it was invisible */
    Term->old->cv = 0;
}


/*
 * High level graphics.
 * Flush the current screen
 */
static void FlushOutput()
{
    int y;


    /* Speed -- Pre-check for "cursor erase" */
    if (Term->soft_cursor) FlushOutputHack();


    /* Update the "modified rows" */
    for (y = Term->scr->y1; y <= Term->scr->y2; ++y) FlushOutputRow(y);

    /* No rows are invalid */
    Term->scr->y1 = Term->scr->h;
    Term->scr->y2 = 0;


    /* Cursor update -- Graphics machines */
    if (Term->soft_cursor)
    {
	/* XXX Note -- there may be a slight "flicker" of the cursor */
	/* at redraws, if annoying, just surround this whole block with */
	/* a huge "did something about the cursor change" conditional */

	/* Erase the cursor (if it WAS visible) */
	if (!Term->old->cu && Term->old->cv)
	{
	    /* Note that "Term->old" and "new" are the same now */
	    int x = Term->old->cx;
	    int y = Term->old->cy;
	    byte a = tw_a(Term->old, x, y);
	    char c =  tw_c(Term->old, x, y);
	    char buf[2];

	    /* Hack -- process spaces */
	    if (BLANK(a,c)) a = 0, c = ' ';

	    /* Hack -- build a two-char string */
	    buf[0] = c;

	    /* Restore the actual character (unless invisible) */
	    if (a) Term_text(x, y, 1, a, buf);

	    /* Simply Erase the grid */
	    else Term_wipe(x, y, 1, 1);
	}

	/* Redraw the cursor itself (if it IS visible) */
	if (!Term->scr->cu && Term->scr->cv)
	{
	    /* Hack -- call the cursor display routine */
	    Term_curs(Term->scr->cx, Term->scr->cy, 1);
	}
    }

    /* Cursor Update -- Hardware Cursor */
    else
    {    
	/* The cursor is "Useless", attempt to "hide" it */
	if (Term->scr->cu)
	{
	    /* Put the cursor NEAR where it belongs */
	    Term_curs(80, Term->scr->cy, 1);

	    /* Cursor invisible if possible */
	    Term_xtra(TERM_XTRA_INVIS, -999);
	}

	/* The cursor is "invisible", attempt to hide it */
	else if (!Term->scr->cv)
	{
	    /* Put the cursor where it belongs anyway */
	    Term_curs(Term->scr->cx, Term->scr->cy, 1);

	    /* Cursor invisible if possible */
	    Term_xtra(TERM_XTRA_INVIS, -999);
	}

	/* The cursor must be "visible", put it where it belongs */
	else
	{
	    /* Put the cursor where it belongs */
	    Term_curs(Term->scr->cx, Term->scr->cy, 1);

	    /* Make sure we are visible */
	    Term_xtra(TERM_XTRA_BEVIS, -999);
	}
    }


    /* Save the "cursor state" */
    Term->old->cu = Term->scr->cu;
    Term->old->cv = Term->scr->cv;
    Term->old->cx = Term->scr->cx;
    Term->old->cy = Term->scr->cy;
}




/*
 * Mentally draw an attr/char at a given location
 * Assumes location and data is legal.
 */
static void QueueAttrChar(int x, int y, int na, int nc)
{
    int oa = tw_a(Term->scr,x,y);
    int oc = tw_c(Term->scr,x,y);

    /* Hack -- Ignore "non-changes" */
    if ((oa == na) && (oc == nc)) return;

    /* Save the "literal" information */
    tw_a(Term->scr,x,y) = na;
    tw_c(Term->scr,x,y) = nc;

    /* Hack -- ignore "double blanks" */
    if (BLANK(na,nc) && BLANK(oa,oc)) return;

    /* Check for new min/max row info */
    if (y < Term->scr->y1) Term->scr->y1 = y;
    if (y > Term->scr->y2) Term->scr->y2 = y;

    /* Check for new min/max col info for this row */
    if (x < Term->scr->x1[y]) Term->scr->x1[y] = x;
    if (x > Term->scr->x2[y]) Term->scr->x2[y] = x;
}


/*
 * Mentally draw some attr/chars at a given location
 *
 * Called only from "Term_addstr()", see below.
 *
 * Assumes that (x,y) is a valid location, and also that
 * one of (x+n-1,y) or (x+strlen(s)-1,y) is a valid location.
 */
static void QueueAttrChars(int x, int y, int n, byte a, cptr s)
{
    register int i, x1 = -1, x2;

    /* Avoid icky "unsigned" issues */
    int na = a;
    int nc = *s;

    /* Analyze the new chars */
    for (i = 0; (i < n) && nc; x++, i++, nc = s[i])
    {
	int oa = tw_a(Term->scr,x,y);
	int oc = tw_c(Term->scr,x,y);

	/* Hack -- Ignore "non-changes" */
	if ((oa == na) && (oc == nc)) continue;

	/* Save the "literal" information */
	tw_a(Term->scr,x,y) = na;
	tw_c(Term->scr,x,y) = nc;

	/* Hack -- ignore "double blanks" */
	if (BLANK(na,nc) && BLANK(oa,oc)) continue;

	/* Note the "range" of screen updates */
	if (x1 < 0) x1 = x;
	x2 = x;
    }

    /* Expand the "change area" as needed */
    if (x1 >= 0)
    {
	/* Check for new min/max row info */
	if (y < Term->scr->y1) Term->scr->y1 = y;
	if (y > Term->scr->y2) Term->scr->y2 = y;

	/* Check for new min/max col info in this row */
	if (x1 < Term->scr->x1[y]) Term->scr->x1[y] = x1;
	if (x2 > Term->scr->x2[y]) Term->scr->x2[y] = x2;
    }
}



/*
 * Erase part of the screen (given top left, and size)
 * Note that these changes are NOT flushed immediately
 * This function is only used below.
 */
static void EraseScreen(int ex, int ey, int ew, int eh)
{
    register int x, y;

    /* Drop "black spaces" everywhere */
    int na = 0;
    int nc = ' ';

    /* Paranoia -- nothing selected */
    if (ew <= 0) return;
    if (eh <= 0) return;

    /* Paranoia -- nothing visible */
    if (ex >= Term->scr->w) return;
    if (ey >= Term->scr->h) return;

    /* Force legal location */
    if (ex < 0) ex = 0;
    if (ey < 0) ey = 0;

    /* Force legal size */
    if (ex + ew > Term->scr->w) ew = Term->scr->w - ex;
    if (ey + eh > Term->scr->h) eh = Term->scr->h - ey;

    /* Scan every row */
    for (y = ey; y < ey + eh; y++)
    {
	register int x1 = -1, x2;

	/* Scan every column */
	for (x = ex; x < ex + ew; x++)
	{
	    int oa = tw_a(Term->scr,x,y);
	    int oc = tw_c(Term->scr,x,y);

	    /* Hack -- Ignore "non-changes" */
	    if ((oa == na) && (oc == nc)) continue;

	    /* Save the "literal" information */
	    tw_a(Term->scr,x,y) = na;
	    tw_c(Term->scr,x,y) = nc;

	    /* Hack -- ignore "double blanks" */
	    if (BLANK(na,nc) && BLANK(oa,oc)) continue;

	    /* Note the "range" of screen updates */
	    if (x1 < 0) x1 = x;
	    x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0)
	{
	    /* Check for new min/max row info */
	    if (y < Term->scr->y1) Term->scr->y1 = y;
	    if (y > Term->scr->y2) Term->scr->y2 = y;

	    /* Check for new min/max col info in this row */
	    if (x1 < Term->scr->x1[y]) Term->scr->x1[y] = x1;
	    if (x2 > Term->scr->x2[y]) Term->scr->x2[y] = x2;
	}
    }
}


/*
 * Clear from (x1,y1) to (x2,y2), inclusive, and move to (x1,y1)
 *
 * I do not know how efficient this is expected to be...
 * Nor do I know how efficient the OutputFlush() will be.
 */
errr Term_erase(int x1, int y1, int x2, int y2)
{
    /* We always leave the cursor at the top-left edge */
    Term_gotoxy(x1,y1);

    /* Queue the "erase" for later */
    EraseScreen(x1,y1,1+x2-x1,1+y2-y1);

    /* Success */
    return (0);
}


/*
 * Clear the entire screen, and move to the top left corner
 *
 * After a "clear screen" we are "allowed" to do a total redraw.
 *
 * Note that "term_win_wipe()" does the cursor stuff for us.
 */
errr Term_clear()
{
    /* Hack -- Save the cursor visibility */
    bool cv = Term->scr->cv;

    /* Wipe the screen mentally */
    term_win_wipe(Term->scr);

    /* Hack -- Restore the cursor visibility */
    Term->scr->cv = cv;

    /* Success */
    return (0);
}





/*
 * High level graphics.
 * Redraw the whole screen.
 * Hack -- this is probably not the cleanest method.
 */
errr Term_redraw()
{
    int y;

    /* Mark the Term->old screen as empty */
    term_win_wipe(Term->old);

    /* Physically erase the whole screen */
    Term_wipe(0, 0, Term->old->w, Term->old->h);

    /* Hack -- mark every row as invalid */
    Term->scr->y1 = 0;
    Term->scr->y2 = Term->scr->h - 1;

    /* Hack -- mark every column of every row as invalid */
    for (y = 0; y < Term->scr->h; y++)
    {
	Term->scr->x1[y] = 0;
	Term->scr->x2[y] = Term->scr->w - 1;
    }

    /* And then "flush" the real screen */
    FlushOutput();

    /* Success */
    return (0);
}





/*
 * Save the current screen contents
 * This may or may not be currently "displayed".
 *
 * Note that up to 16 "pending" Term_save()'s are allowed.
 * Note that only the ones that are used take up memory.
 */
errr Term_save(void)
{
    term_win *mem;

    /* Get the next memory entry */
    mem = &mem_array[mem_depth];

    /* Hack -- allocate memory screens as needed */
    if (!mem->w || !mem->h)
    {
	/* Efficiency -- Initialize "mem" as needed */	
	term_win_init(mem, Term->scr->w, Term->scr->h);
    }

    /* Hack -- react to changing window sizes */
    if (mem->w != Term->scr->w || mem->h != Term->scr->h)
    {
	/* Resize to the desired size */
	term_win_resize(mem, Term->scr->w, Term->scr->h);
    }

    /* Save the current screen data */
    term_win_load(mem, Term->scr);

    /* Advance the depth pointer */
    mem_depth++;

    /* Hack -- Handle "errors" (better than crashing) */
    if (mem_depth >= MEM_SIZE) mem_depth = 0;

    /* Success */
    return (0);
}


/*
 * Restore screen contents saved above.
 *
 * Note that every "Term_save()" MUST have a matching "Term_load()".
 */
errr Term_load(void)
{
    term_win *mem;

    /* Hack -- Handle "errors" (see above) */
    if (mem_depth == 0) mem_depth = MEM_SIZE;

    /* Retreat the depth pointer */
    mem_depth--;

    /* Get the memory screen */
    mem = &mem_array[mem_depth];

    /* Restore the saved info */
    term_win_load(Term->scr, mem);

    /* Success */
    return (0);
}




/*
 * Flush the output
 */
errr Term_fresh()
{
    /* Flush the output */
    FlushOutput();

    /* And flush the graphics */
    Term_xtra(TERM_XTRA_FLUSH, -999);

    /* Success */
    return (0);
}


/*
 * Make an "alert sound" on the Term
 * XXX Allow for some form of "visual bell"
 */
errr Term_bell()
{
    /* Make a noise */
    Term_xtra(TERM_XTRA_NOISE, -999);

    /* Success */
    return (0);
}




/*
 * Update the Term.
 * XXX Not implemented.
 */
errr Term_update()
{
    /* Success */
    return (0);
}


/*
 * React to a new physical screen size.
 * Hack -- not very efficient.
 */
errr Term_resize(int w, int h)
{
    errr res = 0;
    
    /* Resize the "current" term_win */
    if (term_win_resize(Term->old, w, h)) res += 1;
    
    /* Make a new "desired" term_win */
    if (term_win_resize(Term->scr, w, h)) res += 2;

    /* Success */
    return (res);
}




/*
 * Show the cursor
 */
errr Term_show_cursor()
{
    /* Already visible */
    if (Term->scr->cv) return (1);

    /* Remember "visible" */
    Term->scr->cv = 1;

    /* Success */
    return (0);
}


/*
 * Hide the cursor
 */
errr Term_hide_cursor()
{
    /* Already hidden */
    if (!Term->scr->cv) return (1);

    /* Remember "invisible" */
    Term->scr->cv = 0;

    /* Success */
    return (0);
}


/*
 * Place the cursor at a given location
 * Note -- "illegal" requests do not move the cursor.
 */
errr Term_gotoxy(int x, int y)
{
    /* Verify */
    if ((x < 0) || (x >= Term->scr->w)) return (-1);
    if ((y < 0) || (y >= Term->scr->h)) return (-1);

    /* Remember the cursor */    
    Term->scr->cx = x;
    Term->scr->cy = y;

    /* The cursor is not useless */
    Term->scr->cu = 0;

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
 * At a given location, draw an attr/char
 * Do not change the cursor position
 * No visual changes until "Term_fresh()".
 */
errr Term_draw(int x, int y, byte a, char c)
{
    /* Queue it for later */
    QueueAttrChar(x, y, a, c);

    /* Success */
    return (0);
}


/*
 * At a given location, determine the "current" attr and char
 * Note that this refers to what will be on the screen after the
 * next call to "Term_fresh()".  It may or may not already be there.
 */
errr Term_what(int x, int y, byte *a, char *c)
{
    /* Verify location */
    if ((x < 0) || (x >= Term->scr->w)) return (-1);
    if ((y < 0) || (y >= Term->scr->h)) return (-1);

    /* Direct access */
    (*a) = tw_a(Term->scr,x,y);
    (*c) = tw_c(Term->scr,x,y);

    /* Success */
    return (0);
}


/*
 * At the current location, using an attr, add a char
 */
errr Term_addch(byte a, char c)
{
    /* Cannot use "Useless" cursor */
    if (Term->scr->cu) return (-1);

    /* Add the attr/char to the queue */
    QueueAttrChar(Term->scr->cx, Term->scr->cy, a, c);

    /* Advance the cursor */
    Term->scr->cx++;

    /* Success */
    if (Term->scr->cx < Term->scr->w) return (0);

    /* XXX Could handle "wrap" here, such as in: */
    /* if (Term->scr->cx >= Term->scr->w) Term->scr->cx = 0, Term->scr->cy++; */
    /* if (Term->scr->cy >= Term->scr->h) Term->scr->cy = 0; */

    /* Note "Useless" cursor */
    Term->scr->cu = 1;

    /* Note "Useless" cursor */
    return (1);
}


/*
 * At the current location, using an attr, add a string
 * We also take a "length", where "-1" means "all of it".
 * We return "-1" if the cursor is already off the screen.
 * We return "N" if we were "only" able to write "N" chars.
 * So a "positive" result implies future "negative" ones.
 */
errr Term_addstr(int n, byte a, cptr s)
{
    errr res = 0;

    /* Cannot use "Useless" cursor */
    if (Term->scr->cu) return (-1);

    /* Calculate the length if needed */
    if (n < 0) n = strlen(s);

    /* Nothing to add! */
    if (!n) return (-2);

    /* Notice, and deal with, running out of room */
    if (Term->scr->cx + n >= Term->scr->w) res = n = Term->scr->w - Term->scr->cx;

    /* Add all those chars to the queue */
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
    if ((res = Term_gotoxy(x, y))) return (res);

    /* Then add the char */
    if ((res = Term_addch(a, c))) return (res);

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
    if ((res = Term_gotoxy(x, y))) return (res);

    /* Then add the string */
    if ((res = Term_addstr(n, a, s))) return (res);

    /* Success */
    return (0);
}



/*** Input routines ***/


/*
 * Flush and forget the input
 */
errr Term_flush()
{
    /* Flush all pending events */
    while (0 == Term_xtra(TERM_XTRA_CHECK, -999));

    /* Forget all keypresses */
    Term->key_head = Term->key_tail = 0;

    /* Forget any pending macro */
    Term->key_macro = NULL;
    
    /* Success */
    return (0);
}



/*
 * Add a keypress to the "queue"
 */
errr Term_keypress(int k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Store the char, advance the queue */
    Term->key_queue[Term->key_head++] = k;

    /* Circular queue, handle wrap */
    if (Term->key_head == Term->key_size) Term->key_head = 0;

    /* Success (unless overflow) */
    if (Term->key_head != Term->key_tail) return (0);

    /* Hack -- Forget the oldest key */
    if (++Term->key_tail == Term->key_size) Term->key_tail = 0;

    /* Problem */
    return (1);
}


/*
 * Add a keypress to the FRONT of the "queue"
 */
errr Term_key_push(int k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Hack -- Overflow may induce circular queue */
    if (Term->key_tail == 0) Term->key_tail = Term->key_size;

    /* Back up, Store the char */
    Term->key_queue[--Term->key_tail] = k;

    /* Success (unless overflow) */
    if (Term->key_head != Term->key_tail) return (0);

    /* Hack -- Forget the most recent key */
    if (Term->key_head-- == 0) Term->key_head = Term->key_size;

    /* Problem */
    return (1);
}




/*
 * Return the pending keypress, if any, or zero.
 * This routine does NOT remove the key from the queue.
 */
int Term_kbhit()
{
    int i;

    /* Hack -- check for events (once) */
    if (Term->scan_events) Term_xtra(TERM_XTRA_CHECK, -999);

    /* Semi-Hack -- Handle pending macros (do not advance) */
    if (Term->key_macro && Term->key_macro[0]) return (*Term->key_macro);
    
    /* Flush the event queue if necessary */
    if (Term->key_head == Term->key_tail) {
	while (0 == Term_xtra(TERM_XTRA_CHECK, -999));
    }
    
    /* If no key is ready, none was hit, return "nothing" */
    if (Term->key_head == Term->key_tail) return (0);

    /* Extract the keypress, do NOT advance the queue */
    i = Term->key_queue[Term->key_tail];

    /* A key is ready */
    return (i);
}



/*
 * Get a keypress from the user.  Parse macros.
 */
int Term_inkey()
{
    int i;

    /* Hack -- check for events (once) */
    if (Term->scan_events) Term_xtra(TERM_XTRA_CHECK, -999);

    /* Semi-Hack -- Handle pending macros (and advance) */
    if (Term->key_macro && Term->key_macro[0]) return (*Term->key_macro++);
    
    /* Wait for a keypress */
    while (Term->key_head == Term->key_tail) Term_xtra(TERM_XTRA_EVENT, -999);
    
    /* Extract the next real keypress, advance the queue */
    i = Term->key_queue[Term->key_tail++];

    /* Circular queue requires wrap-around */
    if (Term->key_tail == Term->key_size) Term->key_tail = 0;

    /* Return the key */
    return (i);
}




/*
 * Activate a new Term (and deactivate the current Term)
 *
 * This function is extremely important, and also somewhat bizarre.
 * It is the only function that should "modify" the value of "Term".
 *
 * To "create" a valid "term", one should do a "term_init()", then set
 * the various flags and hooks, and then do a "Term_activate()".
 */
errr Term_activate(term *t)
{
    /* Already done */
    if (Term == t) return (1);
    
    /* Deactivate the old Term */
    if (Term) Term_xtra(TERM_XTRA_LEVEL, TERM_LEVEL_SOFT_SHUT);

    /* Hack -- Call the special "init" hook */
    if (!t->initialized) {
	if (t->init_hook) (*t->init_hook)(t);
	t->initialized = TRUE;
    }
    
    /* Remember the Term */
    Term = t;

    /* Activate the new Term */
    if (Term) Term_xtra(TERM_XTRA_LEVEL, TERM_LEVEL_SOFT_OPEN);

    /* Success */
    return (0);
}



/*
 * Nuke a term
 */
errr term_nuke(term *t)
{
    /* Hack -- Call the special "nuke" hook */
    if (t->initialized) {
	if (t->nuke_hook) (*t->nuke_hook)(t);
	t->initialized = FALSE;
    }

    /* Free the arrays */
    term_win_nuke(t->old);
    term_win_nuke(t->scr);

    /* Success */
    return (0);
}


/*
 * Initialize a term, using a screen of the given size.
 * Also prepare the "input queue" for "k" keypresses.
 * By default, the cursor should start "invisible".
 */
errr term_init(term *t, int w, int h, int k)
{
    /* Hack -- clear the term */
    WIPE(t, term);
    
    /* Initialize the default "physical" screen */
    MAKE(t->old, term_win);
    term_win_init(t->old, w, h);

    /* Initialize the default "current" screen */
    MAKE(t->scr, term_win);
    term_win_init(t->scr, w, h);

    /* Prepare the input queue */
    t->key_head = t->key_tail = 0;

    /* Determine the input queue size */
    t->key_size = k;

    /* Allocate the input queue */
    C_MAKE(t->key_queue, t->key_size, char);

    /* Success */
    return (0);
}


