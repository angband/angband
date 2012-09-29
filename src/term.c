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
 */



/*
 * The "hardware screen" is assumed to be this big.
 */
#define SCRN_ROWS	24
#define SCRN_COLS	80


/*
 * A macro to determine if an attr/char pair looks "blank"
 * Note that the "black" attribute is "reserved", and that
 * currently, the "space" character is hard-coded as "blank".
 */
#define BLANK(A,C)	(((A)==0) || ((C)==' '))





/* Do our support routines use a "software cursor"? */
static bool Term_flag_soft_cursor = FALSE;

/* Should we call the "Event Loop" when "bored"? */
static bool Term_flag_scan_events = FALSE;


/* The "actual" window */
static term_win old_body;
static term_win *old = &old_body;

/* The "current" window */
static term_win scr_body;
static term_win *scr = &scr_body;


/* Next "screen memory slot" to use */
static int mem_depth = 0;

/* The "memory" screens */
static term_win mem_array[MEM_SIZE];



/*
 * Basic Init/Nuke hooks (optional) that can be provided by the various
 * "graphic modules" to do "module dependant" startup/shutdown when the
 * Term_init() and Term_nuke() routines are called (see below).
 */
void (*Term_init_hook)(void) = NULL;
void (*Term_nuke_hook)(void) = NULL;

 
/*
 * Hooks, provided by particular "graphic modules"
 * See, for example, "main-mac.c" for a Macintosh Module
 * These hooks are used below to provide the "bodies" of functions.
 */
void (*Term_text_hook)(int x, int y, int n, byte a, cptr s) = NULL;
void (*Term_wipe_hook)(int x, int y, int w, int h) = NULL;
void (*Term_curs_hook)(int x, int y, int z) = NULL;
void (*Term_scan_hook)(int n) = NULL;
void (*Term_xtra_hook)(int n) = NULL;






/*
 * Load a "term_win()" from another
 * Note that the "cursor visibility" is turned off
 * The target "t" should be "as big as" the source "s"
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
 * The target "t" better be as big as the source "s"!
 */
errr term_win_load(term_win *t, term_win *s)
{
    int y, x;
    
    /* Copy all the data from "s" into "t" */
    for (y = 0; y < s->h; y++)
    {
	for (x = 0; x < s->w; x++)
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
    
    /* The "whole" window may have changed */
    t->y1 = 0;
    if (t->y2 < s->h-1) t->y2 = s->h - 1;

    /* Hack -- update the "per-row" info */
    for (y = 0; y < s->h; y++)
    {
	t->x1[y] = 0;
	if (t->x2[y] < s->w-1) t->x2[y] = s->w - 1;
    }
    
    /* Success */
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
 * Query/Set various "modes" (see term.h)
 * A negative "value" means "query only"
 */
int Term_method(int mode, int value)
{
    int res = 0;

    /* Check the old value */    
    switch (mode)
    {
        case TERM_SOFT_CURSOR: res = Term_flag_soft_cursor; break;
        case TERM_SCAN_EVENTS: res = Term_flag_scan_events; break;
    }

    /* Just a query? */
    if (value < 0) return (res);
        
    /* Assign a new value */    
    switch (mode)
    {
        case TERM_SOFT_CURSOR: Term_flag_soft_cursor = value; break;
        case TERM_SCAN_EVENTS: Term_flag_scan_events = value; break;
    }

    /* Return the old value */
    return (res);
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
    for (x = scr->x1[y]; x <= scr->x2[y]; x++)
    {
	/* See what SEEMS TO BE there */
	oa = tw_a(old,x,y);
	oc = tw_c(old,x,y);

	/* See what SHOULD BE there */
	na = tw_a(scr,x,y);
	nc = tw_c(scr,x,y);

	/* Save the "actual" info mentally */
	tw_a(old,x,y) = na;
	tw_c(old,x,y) = nc;

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
    scr->x1[y] = scr->w;
    scr->x2[y] = 0;
}



/*
 * Check to see if the old cursor will get erased by FlushOutputRow()'s
 */
static void FlushOutputHack()
{
    int cx, cy, oa, oc, na, nc;

    /* Cursor was offscreen, ignore it */
    if (old->cu) return;
    
    /* Cursor was invisible, ignore it */
    if (!old->cv) return;

    /* Extract the cursor location */
    cy = old->cy;
    cx = old->cx;

    /* Cursor row was unaffected, ignore it */
    if ((scr->y1 > cy) || (scr->y2 < cy)) return;

    /* Cursor column in the Cursor row was unaffected, ignore it */
    if ((scr->x1[cy] > cx) || (scr->x2[cy] < cx)) return;

    /* Access the old contents */
    oa = tw_a(old,cx,cy);
    oc = tw_c(old,cx,cy);

    /* Access the new contents */
    na = tw_a(scr,cx,cy);
    nc = tw_c(scr,cx,cy);

    /* If the contents are the same, cursor may be skipped */
    if ((oa == na) && (oc == nc)) return;

    /* If the contents are both "blank", cursor may be skipped */
    if (BLANK(oa,oc) && BLANK(na,nc)) return;

    /* Hack -- The cursor will be erased, pretend it was invisible */
    old->cv = 0;
}


/*
 * High level graphics.
 * Flush the current screen
 */
static void FlushOutput()
{
    int y;
    

    /* Speed -- Pre-check for "cursor erase" */
    if (Term_flag_soft_cursor) FlushOutputHack();


    /* Update the "modified rows" */
    for (y = scr->y1; y <= scr->y2; ++y) FlushOutputRow(y);

    /* No rows are invalid */
    scr->y1 = scr->h;
    scr->y2 = 0;


    /* Cursor update -- Graphics machines */
    if (Term_flag_soft_cursor)
    {
        /* XXX Note -- there may be a slight "flicker" of the cursor */
        /* at redraws, if annoying, just surround this whole block with */
        /* a huge "did something about the cursor change" conditional */
        
        /* Erase the cursor (if it WAS visible) */
        if (!old->cu && old->cv)
        {
	    /* Note that "old" and "new" are the same now */
	    int x = old->cx;
	    int y = old->cy;
	    byte a = tw_a(old, x, y);
	    char c =  tw_c(old, x, y);
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
        if (!scr->cu && scr->cv)
        {
	    /* Hack -- call the cursor display routine */
	    Term_curs(scr->cx, scr->cy, 1);
        }
    }
        
    /* Cursor Update -- Hardware Cursor */
    else
    {    
        /* The cursor is "Useless", attempt to "hide" it */
        if (scr->cu)
        {
	    /* Put the cursor NEAR where it belongs */
	    Term_curs(80, scr->cy, 1);
    
	    /* Cursor invisible if possible */
	    Term_xtra(TERM_XTRA_INVIS);
        }
        
        /* The cursor is "invisible", attempt to hide it */
        else if (!scr->cv)
        {
	    /* Put the cursor where it belongs anyway */
	    Term_curs(scr->cx, scr->cy, 1);
    
	    /* Cursor invisible if possible */
	    Term_xtra(TERM_XTRA_INVIS);
        }
    
        /* The cursor must be "visible", put it where it belongs */
        else
        {
	    /* Put the cursor where it belongs */
	    Term_curs(scr->cx, scr->cy, 1);
    
	    /* Make sure we are visible */
	    Term_xtra(TERM_XTRA_BEVIS);
        }
    }


    /* Save the "cursor state" */
    old->cu = scr->cu;
    old->cv = scr->cv;
    old->cx = scr->cx;
    old->cy = scr->cy;
}




/*
 * Mentally draw an attr/char at a given location
 * Assumes location and data is legal.
 */
static void QueueAttrChar(int x, int y, int na, int nc)
{
    int oa = tw_a(scr,x,y);
    int oc = tw_c(scr,x,y);
    
    /* Hack -- Ignore "non-changes" */
    if ((oa == na) && (oc == nc)) return;

    /* Save the "literal" information */
    tw_a(scr,x,y) = na;
    tw_c(scr,x,y) = nc;

    /* Hack -- ignore "double blanks" */
    if (BLANK(na,nc) && BLANK(oa,oc)) return;
    
    /* Check for new min/max row info */
    if (y < scr->y1) scr->y1 = y;
    if (y > scr->y2) scr->y2 = y;

    /* Check for new min/max col info for this row */
    if (x < scr->x1[y]) scr->x1[y] = x;
    if (x > scr->x2[y]) scr->x2[y] = x;
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
	int oa = tw_a(scr,x,y);
	int oc = tw_c(scr,x,y);

	/* Hack -- Ignore "non-changes" */
	if ((oa == na) && (oc == nc)) continue;

	/* Save the "literal" information */
	tw_a(scr,x,y) = na;
	tw_c(scr,x,y) = nc;

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
	if (y < scr->y1) scr->y1 = y;
	if (y > scr->y2) scr->y2 = y;

	/* Check for new min/max col info in this row */
	if (x1 < scr->x1[y]) scr->x1[y] = x1;
	if (x2 > scr->x2[y]) scr->x2[y] = x2;
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
    if (ex >= scr->w) return;
    if (ey >= scr->h) return;

    /* Force legal location */
    if (ex < 0) ex = 0;
    if (ey < 0) ey = 0;

    /* Force legal size */
    if (ex + ew > scr->w) ew = scr->w - ex;
    if (ey + eh > scr->h) eh = scr->h - ey;

    /* Scan every row */
    for (y = ey; y < ey + eh; y++)
    {
	register int x1 = -1, x2;
	
	/* Scan every column */
	for (x = ex; x < ex + ew; x++)
	{
	    int oa = tw_a(scr,x,y);
	    int oc = tw_c(scr,x,y);

	    /* Hack -- Ignore "non-changes" */
	    if ((oa == na) && (oc == nc)) continue;

	    /* Save the "literal" information */
	    tw_a(scr,x,y) = na;
	    tw_c(scr,x,y) = nc;

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
	    if (y < scr->y1) scr->y1 = y;
	    if (y > scr->y2) scr->y2 = y;

	    /* Check for new min/max col info in this row */
	    if (x1 < scr->x1[y]) scr->x1[y] = x1;
	    if (x2 > scr->x2[y]) scr->x2[y] = x2;
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
    /* Save the cursor visibility */
    bool cv = scr->cv;
    
    /* Wipe the screen mentally */
    term_win_wipe(scr);

    /* Restore the cursor visibility */
    scr->cv = cv;

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

    /* Mark the old screen as empty */
    term_win_wipe(old);

    /* Physically erase the whole screen */
    Term_wipe(0, 0, old->w, old->h);

    /* Hack -- mark every row as invalid */
    scr->y1 = 0;
    scr->y2 = scr->h - 1;

    /* Hack -- mark every column of every row as invalid */
    for (y = 0; y < scr->h; y++)
    {
	scr->x1[y] = 0;
	scr->x2[y] = scr->w - 1;
    }
    
    /* And then "flush" the real screen */
    FlushOutput();

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
    if (mem->w != scr->w || mem->h != scr->h)
    {
	/* XXX Free the old data if needed */

	/* Efficiency -- Initialize "mem" as needed */	
	term_win_init(mem, scr->w, scr->h);
    }

    /* Save the current screen data */
    term_win_load(mem, scr);

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

    /* Hack -- require matching sizes */
    if (mem->w != scr->w || mem->h != scr->h) return (-1);

    /* Restore the saved info */
    term_win_load(scr, mem);
    
    /* Success */
    return (0);
}





/*
 * Initialize the Term, using an 80x24 physical screen.
 *
 * Note that we must receive (significant) external support, from
 * a file such as "main-mac.c" or "main-x11.c" or "main-cur.c".  This
 * support takes the form of various external functions:
 *
 *   Term_scan() = Check for events
 *   Term_curs() = Draw / Move the cursor
 *   Term_wipe() = Erase (part of) the screen
 *   Term_text() = Place some text on the screen
 *   Term_xtra() = Low level boring things
 *
 * We provide, among other things, the functions Term_keypress()
 * to "react" to keypress events, and Term_redraw() to redraw the
 * entire screen, plus (later), "Term_resize()" to note a new size.
 *
 * Note that the "graphic hooks" must be ready BEFORE calling "Term_init()".
 */
errr Term_init()
{
    /* Allow a "module dependant" startup function */
    if (Term_init_hook) (*Term_init_hook)();
    
    /* No screens memorized yet.  Note "static init" of mem_array.  */
    mem_depth = 0;

    /* Initialize the default "physical" screen */
    term_win_init(old, SCRN_COLS, SCRN_ROWS);

    /* Initialize the default "current" screen */
    term_win_init(scr, SCRN_COLS, SCRN_ROWS);

    /* Hack -- Start with a visible Cursor */
    scr->cv = 1;

    /* Redraw the screen */
    Term_redraw();

    /* Success */
    return (0);
}


/*
 * Destroy the Term
 * XXX Not implemented
 */
errr Term_nuke()
{
    /* XXX Could do stuff like this */
    /* C_KILL(old->a, old->w * old->h, char); */
    /* C_KILL(old->c, old->w * old->h, char); */
    /* C_KILL(old->x1, old->w, byte); */
    /* C_KILL(old->x1, old->w, byte); */

    /* Allow a "module dependant" shutdown function */
    if (Term_nuke_hook) (*Term_nuke_hook)();
    
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
    Term_xtra(TERM_XTRA_FLUSH);

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
    Term_xtra(TERM_XTRA_NOISE);

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
 * XXX Not implemented.
 */
errr Term_resize(int w, int h)
{
    /* Success */
    return (0);
}




/*
 * Show the cursor
 */
errr Term_show_cursor()
{
    /* Already visible */
    if (scr->cv) return (1);

    /* Remember "visible" */
    scr->cv = 1;

    /* Success */
    return (0);
}


/*
 * Hide the cursor
 */
errr Term_hide_cursor()
{
    /* Already hidden */
    if (!scr->cv) return (1);

    /* Remember "invisible" */
    scr->cv = 0;

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
    if ((x < 0) || (x >= scr->w)) return (-1);
    if ((y < 0) || (y >= scr->h)) return (-1);

    /* Remember the cursor */    
    scr->cx = x;
    scr->cy = y;

    /* The cursor not Useless */
    scr->cu = 0;

    /* Success */
    return (0);
}


/*
 * Extract the current cursor location
 */
errr Term_locate(int *x, int *y)
{
    /* Cannot grab "Useless" cursor */
    if (scr->cu) return (1);
    
    /* Just grab the cursor vars */
    (*x) = scr->cx;
    (*y) = scr->cy;

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
    if ((x < 0) || (x >= scr->w) || (y < 0) || (y > scr->h)) return (-1);
        
    /* Direct access */
    (*a) = tw_a(scr,x,y);
    (*c) = tw_c(scr,x,y);
    
    /* Success */
    return (0);
}


/*
 * At the current location, using an attr, add a char
 */
errr Term_addch(byte a, char c)
{
    /* Cannot use "Useless" cursor */
    if (scr->cu) return (-1);

    /* Add the attr/char to the queue */
    QueueAttrChar(scr->cx, scr->cy, a, c);

    /* Advance the cursor */
    scr->cx++;

    /* Success */
    if (scr->cx < scr->w) return (0);
    
    /* XXX Could handle "wrap" here, such as in: */
    /* if (scr->cx >= scr->w) scr->cx = 0, scr->cy++; */
    /* if (scr->cy >= scr->h) scr->cy = 0; */

    /* Note "Useless" cursor */
    scr->cu = 1;

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
    if (scr->cu) return (-1);

    /* Calculate the length if needed */
    if (n < 0) n = strlen(s);

    /* Nothing to add! */
    if (!n) return (-2);

    /* Notice, and deal with, running out of room */
    if (scr->cx + n >= scr->w) res = n = scr->w - scr->cx;

    /* Add all those chars to the queue */
    QueueAttrChars(scr->cx, scr->cy, n, a, s);

    /* Advance the cursor */
    scr->cx += n;

    /* Hack -- Notice "Useless" cursor */
    if (res) scr->cu = 1;

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
 * Size of Keypress buffer
 */
#define KEY_SIZE 128

/*
 * A Queue of characters, initially empty
 */
static char key_queue[KEY_SIZE];
static int key_head = 0;
static int key_tail = 0;



/*
 * Flush and forget the input
 */
errr Term_flush()
{
    /* Flush event queue */
    Term_scan(-1);

    /* Forget all keypresses */
    key_head = key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Add a keypress to the "queue" (fake event)
 */
errr Term_keypress(int k)
{
    /* Hack -- Refuse to enqueue "nul" */
    if (!k) return (-1);

    /* Store the char, advance the queue */
    key_queue[key_head++] = k;

    /* Circular queue, handle wrap */
    if (key_head == KEY_SIZE) key_head = 0;

    /* Hack -- Catch overflow (forget oldest) */
    if (key_head == key_tail) key_tail++;

    /* Hack -- Overflow may induce circular queue */
    if (key_tail == KEY_SIZE) key_tail = 0;

    /* Success */
    return (0);
}


/*
 * Are there any keypresses ready?
 * Return the key (but do not de-queue it).
 */
int Term_kbhit()
{
    int i;

    /* Scan for events */
    if (Term_flag_scan_events) Term_scan(-1);

    /* If no keys are "obviously ready", Flush all the events */
    if (key_head == key_tail) Term_scan(-1);

    /* If no key is ready, none was hit */
    if (key_head == key_tail) return (0);

    /* Extract the keypress, advance the queue */
    i = key_queue[key_tail++];

    /* Circular queue requires wrap-around */
    if (key_tail == KEY_SIZE) key_tail = 0;

    /* Return the key */
    return (i);
}



/*
 * Wait for a keypress, and return it
 * First flush the output.
 */
int Term_inkey()
{
    int i;

    /* Flush the output */
    Term_fresh();

    /* Scan for events */
    if (Term_flag_scan_events) Term_scan(-1);

    /* While no keypresses ready, wait for an event */
    while (key_head == key_tail) Term_scan(1);

    /* Extract the keypress, advance the queue */
    i = key_queue[key_tail++];

    /* Circular queue requires wrap-around */
    if (key_tail == KEY_SIZE) key_tail = 0;

    /* Return the key */
    return (i);
}




/*
 * Draw "n" chars from the string "s" using attr "a", at location "(x,y)"
 */
void Term_text(int x, int y, int n, byte a, cptr s)
{
    if (Term_text_hook) (*Term_text_hook)(x, y, n, a, s);
}

/*
 * Erase a "block" of chars starting at "(x,y)", with size "(w,h)"
 */
void Term_wipe(int x, int y, int w, int h)
{
    if (Term_wipe_hook) (*Term_wipe_hook)(x, y, w, h);
}

/*
 * Draw a "cursor" at "(x,y)".  Later, "z" may mean something.
 * Note that "Term_grab(x,y,&a,&c)" will have "usable" results.
 */
void Term_curs(int x, int y, int z)
{
    if (Term_curs_hook) (*Term_curs_hook)(x, y, z);
}

/*
 * Scan for up to "n" events, or until done if "n" is negative
 */
void Term_scan(int n)
{
    if (Term_scan_hook) (*Term_scan_hook)(n);
}

/*
 * Do extra thing number "n" (one of the TERM_XTRA_XXX constants)
 */
void Term_xtra(int n)
{
    if (Term_xtra_hook) (*Term_xtra_hook)(n);
}



