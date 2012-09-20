/* File: term.c */

/* Purpose: generic, efficient, modular, terminal routines -BEN- */

#include "term.h"

#include "z-virt.h"


/*
 * This file provides a generic interface to one or more "windows"
 * of textual information.  This includes the case of a single
 * "window" such as that provided by a dumb terminal.
 *
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
 *   movement across the screen is relatively cheap
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
 * files are told how to display each color (except color zero).
 * It is also a good idea to reserve color "one" for white.
 *
 * Note that we reserve the "zero char" for string termination.
 * Thus the "zero char" can never be drawn on the screen.
 *
 * The only "reserved" character is "ascii 32" (the "space").
 * Also, we assume that the "color" of a "space" does not matter.
 * This will need to be changed if we start supporting "attributes"
 * with "background colors" (ala IBM).  When the screen is cleared,
 * it is cleared to "spaces" with color "zero" (black).
 *
 * Note that we can easily allow the "term_win" structure to be
 * exported, allowing users to create various "user defined"
 * routines, such as "scrolling packages".  These routines would
 * not tie in the the "terminal capabilities", but they would
 * be completely portable...
 *
 * Currently, the "Term_fresh()" routine performs the *minimum*
 * number of physical updates (via Term_wipe() and Term_text()),
 * making use of the fact that the "color" of a "blank" is not
 * important, and by remembering the current contents of the
 * window.  Unfortunately, this may be slightly non-optimal in
 * some cases, in particular, those in which, say, a string of
 * ten characters needs to be written, but the fifth character
 * is already on the screen.  Currently, this will cause the
 * "Term_text()" routine to be called once for each half of the
 * string, instead of once for the whole string.  It seems that
 * it should be possible to notice at least this situation, which
 * seems pretty common.
 *
 * We should teach "Term_fresh()" to be a little bit "smarter".
 * In particular, it might help to "pre-process" things like
 * "clear to the end of the line" and such.  Or maybe this could
 * be done by the "Term_text()" procedures themselves...  One easy
 * addition would be to have the "term_win" maintain, in addition
 * to the range of changed columns, a range of "erased" columns.
 *
 * Be VERY careful about printing in the bottom right hand corner
 * of the screen, as it may induce "scrolling" on older machines.
 * We should perhaps consider an option to refuse to flush changes
 * to such a location.  Or assume that the "Term_text()" hook checks.
 *
 * Note that we must receive (significant) external support, from
 * a file such as "main-mac.c" or "main-x11.c" or "main-cur.c".  This
 * support takes the form of four function hooks:
 *
 *   Term_xtra() = Perform various actions
 *   Term_curs() = Draw (or Move) the cursor
 *   Term_wipe() = Erase (part of) the screen
 *   Term_text() = Place some text on the screen
 *
 * Only "Term_xtra()" is available for external usage.
 *
 * We provide, among other things, the functions "Term_keypress()"
 * to "react" to keypress events, and "Term_redraw()" to redraw the
 * entire screen, plus "Term_resize()" to note a new size.
 *
 * The current screen image always contains exactly what the user
 * requested, but the system optimizes the actual physical updates
 * to just those which actually *look* different on the screen.
 *
 * In particular, the "color" of a "blank" is not important, and all
 * "black" characters are treated as "blanks".
 *
 * Note that some machines CARE about the color of a blank, for example,
 * on IBM machines, the color of a blank is reflected in the cursor.
 */




/* Next "screen memory slot" to use */
static int mem_depth = 0;

/* Semi-Hack -- The "memory" screens */
static term_win mem_array[MEM_SIZE];



/* The current "term" */
term *Term = NULL;




/*
 * Completely "erase" a "term_win".  Mark as "changed".
 *
 * Note that the "cursor visibility" is turned off,
 * and the cursor is moved to the top left corner.
 */
errr term_win_wipe(term_win *t)
{
    int y, x;

    /* Cursor to the top left, and invisible */
    t->cv = t->cu = t->cx = t->cy = 0;

    /* Wipe each row */
    for (y = 0; y < t->h; y++)
    {
        byte *aa = t->a[y];
        char *cc = t->c[y];

        /* Wipe each column */
        for (x = 0; x < t->w; x++)
        {
            *aa++ = 0;
            *cc++ = ' ';
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

    /* Compute Overlap */
    int w = MIN(t->w, s->w);
    int h = MIN(t->h, s->h);

    /* Copy all the data from "s" into "t" */
    for (y = 0; y < h; y++)
    {
        byte *src_aa = s->a[y];
        char *src_cc = s->c[y];

        byte *tgt_aa = t->a[y];
        char *tgt_cc = t->c[y];

        for (x = 0; x < w; x++)
        {
            tgt_aa[x] = src_aa[x];
            tgt_cc[x] = src_cc[x];
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
    /* Free the max-change arrays */
    C_KILL(t->x1, t->h, byte);
    C_KILL(t->x2, t->h, byte);

    /* Free the screen access arrays */
    C_KILL(t->a, t->h, byte*);
    C_KILL(t->c, t->h, char*);

    /* Free the screen arrays */
    C_KILL(t->va, t->h * t->w, byte);
    C_KILL(t->vc, t->h * t->w, char);

    return (0);
}


/*
 * Initialize a "term_win" (using the given screen size)
 */
errr term_win_init(term_win *t, int w, int h)
{
    int y;

    /* Save the size */
    t->w = w;
    t->h = h;

    /* XXX Make the "max used col" array */
    /* C_MAKE(t->rm, h, byte); */

    /* Make the max-change arrays */
    C_MAKE(t->x1, t->h, byte);
    C_MAKE(t->x2, t->h, byte);

    /* Make the screen access arrays */
    C_MAKE(t->a, t->h, byte*);
    C_MAKE(t->c, t->h, char*);

    /* Make the screen arrays */
    C_MAKE(t->va, t->h * t->w, byte);
    C_MAKE(t->vc, t->h * t->w, char);

    /* Prepare the screen access arrays */
    for (y = 0; y < t->h; y++) t->a[y] = t->va + t->w * y;
    for (y = 0; y < t->h; y++) t->c[y] = t->vc + t->w * y;

    /* Wipe it */
    term_win_wipe(t);

    /* Success */
    return (0);
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

/*
 * Place a "cursor" at "(x,y)", with "visibility" of "z".
 * The input is assumed to be "valid".
 */
static errr Term_curs(int x, int y, int z)
{
    if (!Term->curs_hook) return (-1);
    return ((*Term->curs_hook)(x, y, z));
}

/*
 * Erase a "block" of chars starting at "(x,y)", with size "(w,h)"
 * The input is assumed to be "valid".
 */
static errr Term_wipe(int x, int y, int w, int h)
{
    if (!Term->wipe_hook) return (-1);
    return ((*Term->wipe_hook)(x, y, w, h));
}

/*
 * Draw "n" chars from the string "s" using attr "a", at location "(x,y)".
 * The input is assumed to be "valid", that is, not only is the string
 * assumed to be terminated, but the "length" is assumed to be correct,
 * and small enough to fit on the screen at the given location.
 */
static errr Term_text(int x, int y, int n, byte a, cptr s)
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
 * So there are some "screen writes" which never even make it here...
 *
 * The old method pre-extracted "spaces" into a "efficient" form of
 * "erasing", but for various reasons, we will simply "print" them, and
 * let the "Term_text()" procedure handle "space extraction".
 *
 * Note that "trailing spaces" are assumed to have the color of the
 * text that preceded them.  This is probably a mistake.
 *
 * Note especially the frequency of "trailing spaces" in the text which
 * is sent to "Term_text()".  Combined with trailing spaces already on
 * the screen, it should be possible to "optimize" screen clearing.  We
 * will need a new field "max column used" per row.
 */
static void FlushOutputRow(int y)
{
    int x;

    term_win *old = Term->old;
    term_win *scr = Term->scr;

    byte *old_aa = old->a[y];
    char *old_cc = old->c[y];

    byte *scr_aa = scr->a[y];
    char *scr_cc = scr->c[y];

    /* No chars "pending" in "text" */
    int n = 0;

    /* Pending text starts in the first column */
    int fx = 0;

    /* Pending text is "black" */
    int fa = 0;

    int oa, oc, na, nc;

    /* Max width is 255 */
    char text[256];


    /* Scan the columns marked as "modified" */
    for (x = scr->x1[y]; x <= scr->x2[y]; x++)
    {
        /* See what is currently here */
        oa = old_aa[x];
        oc = old_cc[x];

        /* Save and remember the new contents */
        na = old_aa[x] = scr_aa[x];
        nc = old_cc[x] = scr_cc[x];

        /* Hack -- optimize "blanks" */
        if (Term->dark_blanks) {

            /* Optimize the old contents */
            if (oc == ' ') oa = fa;

            /* Optimize the new contents */
            if (nc == ' ') na = fa;
        }

        /* Notice unchanged areas */
        if ((na == oa) && (nc == oc)) {

            /* Flush as needed (see above) */
            if (n)
            {
                /* Terminate the thread */
                text[n] = '\0';

                /* Draw the pending chars */
                if (fa) Term_text(fx, y, n, fa, text);

                /* Hack -- Erase "leading" spaces */
                else Term_wipe(fx, y, n, 1);

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
                if (fa) Term_text(fx, y, n, fa, text);

                /* Hack -- Erase "leading" spaces */
                else Term_wipe(fx, y, n, 1);

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
        if (fa) Term_text(fx, y, n, fa, text);

        /* Hack -- Erase fully blank lines */
        else Term_wipe(fx, y, n, 1);
    }

    /* This row is all done */
    scr->x1[y] = scr->w;
    scr->x2[y] = 0;
}





/*
 * High level graphics.
 * Flush the current screen
 */
static void FlushOutput()
{
    int y;

    term_win *old = Term->old;
    term_win *scr = Term->scr;


    /* Mega-Hack -- allow full erase */
    if (old->erase)
    {
        /* Physically erase the entire window */
        Term_wipe(0, 0, scr->w, scr->h);

        /* Forget the erase */
        old->erase = FALSE;
    }


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
            int x = old->cx;
            int y = old->cy;

            byte *old_aa = old->a[y];
            char *old_cc = old->c[y];

            byte a = old_aa[x];
            char c = old_cc[x];

            char buf[2];

            /* Hack -- build a two-char string */
            buf[0] = c;

            /* Restore the actual character */
            if (a) Term_text(x, y, 1, a, buf);

            /* Simply Erase the grid */
            else Term_wipe(x, y, 1, 1);
        }
    }

    /* Cursor Update -- Erase old Cursor */
    else
    {
        /* Hide useless/invisible cursors */
        if (scr->cu || !scr->cv)
        {
            /* Cursor invisible (if possible) */
            Term_xtra(TERM_XTRA_INVIS, 0);
        }
    }


    /* Update the "modified rows" */
    for (y = scr->y1; y <= scr->y2; ++y) FlushOutputRow(y);

    /* No rows are invalid */
    scr->y1 = scr->h;
    scr->y2 = 0;


    /* Cursor update -- Show new Cursor */
    if (Term->soft_cursor)
    {
        /* Draw the cursor */
        if (!scr->cu && scr->cv)
        {
            /* Call the cursor display routine */
            Term_curs(scr->cx, scr->cy, 1);
        }
    }

    /* Cursor Update -- Show new Cursor */
    else
    {
        /* Hack -- The cursor is "Useless", attempt to "hide" it */
        if (scr->cu)
        {
            /* Paranoia -- Put the cursor NEAR where it belongs */
            Term_curs(scr->w - 1, scr->cy, 0);
        }

        /* The cursor is "invisible", attempt to hide it */
        else if (!scr->cv)
        {
            /* Paranoia -- Put the cursor where it belongs */
            Term_curs(scr->cx, scr->cy, 0);
        }

        /* The cursor must be "visible", put it where it belongs */
        else
        {
            /* Put the cursor where it belongs */
            Term_curs(scr->cx, scr->cy, 1);

            /* Make sure we are visible */
            Term_xtra(TERM_XTRA_BEVIS, 1);
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
    term_win *scr = Term->scr;

    byte *scr_aa = scr->a[y];
    char *scr_cc = scr->c[y];

    int oa = scr_aa[x];
    int oc = scr_cc[x];

    /* Hack -- Ignore "non-changes" */
    if ((oa == na) && (oc == nc)) return;

    /* Save the "literal" information */
    scr_aa[x] = na;
    scr_cc[x] = nc;

    /* Hack -- ignore "double blanks" */
    if (Term->dark_blanks) {

        /* Optimize spaces */
        if ((nc == ' ') && (oc == ' ')) return;
    }

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
    int i, x1 = -1, x2 = -1;

    term_win *scr = Term->scr;

    byte *scr_aa = scr->a[y];
    char *scr_cc = scr->c[y];

    /* Hack -- Avoid icky "unsigned" issues */
    int na = a;
    int nc = *s;

    /* Analyze the new chars */
    for (i = 0; (i < n) && nc; x++, i++, nc = s[i])
    {
        int oa = scr_aa[x];
        int oc = scr_cc[x];

        /* Hack -- Ignore "non-changes" */
        if ((oa == na) && (oc == nc)) continue;

        /* Save the "literal" information */
        scr_aa[x] = na;
        scr_cc[x] = nc;

        /* Hack -- ignore "double blanks" */
        if (Term->dark_blanks) {

            /* Optimize spaces */
            if ((nc == ' ') && (oc == ' ')) continue;
        }

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
 * Clear a block of chars, starting at (x,y), of size (w,h)
 * Move the cursor to (x,y), the top left corner of the block
 */
errr Term_erase(int x, int y, int w, int h)
{
    int xx, yy;

    term_win *scr = Term->scr;

    /* Drop "black spaces" everywhere */
    int na = 0;
    int nc = ' ';

    /* Paranoia -- nothing selected */
    if (w <= 0) return (0);
    if (h <= 0) return (0);

    /* Paranoia -- nothing visible */
    if (x >= scr->w) return (0);
    if (y >= scr->h) return (0);

    /* Force legal location */
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    /* Force legal size */
    if (x + w > scr->w) w = scr->w - x;
    if (y + h > scr->h) h = scr->h - y;

    /* Scan every row */
    for (yy = y; yy < y + h; yy++)
    {
        int x1 = -1, x2 = -1;

        byte *scr_aa = scr->a[yy];
        char *scr_cc = scr->c[yy];

        /* Scan every column */
        for (xx = x; xx < x + w; xx++)
        {
            int oa = scr_aa[xx];
            int oc = scr_cc[xx];

            /* Hack -- Ignore "non-changes" */
            if ((oa == na) && (oc == nc)) continue;

            /* Save the "literal" information */
            scr_aa[xx] = na;
            scr_cc[xx] = nc;

            /* Hack -- ignore "double blanks" */
            if (Term->dark_blanks) {

                /* Optimize spaces */
                if ((nc == ' ') && (oc == ' ')) continue;
            }

            /* Track minumum changed column */
            if (x1 < 0) x1 = xx;

            /* Track maximum changed column */
            x2 = xx;
        }

        /* Expand the "change area" as needed */
        if (x1 >= 0)
        {
            /* Check for new min/max row info */
            if (yy < scr->y1) scr->y1 = yy;
            if (yy > scr->y2) scr->y2 = yy;

            /* Check for new min/max col info in this row */
            if (x1 < scr->x1[yy]) scr->x1[yy] = x1;
            if (x2 > scr->x2[yy]) scr->x2[yy] = x2;
        }
    }

    /* Place the cursor in the erased rectangle */
    Term_gotoxy(x,y);

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
    term_win *old = Term->old;
    term_win *scr = Term->scr;

    /* Hack -- Save the cursor visibility */
    bool cv = scr->cv;

    /* Wipe the screen mentally */
    term_win_wipe(scr);

    /* Hack -- Restore the cursor visibility */
    scr->cv = cv;

    /* Wipe the old screen mentally */
    term_win_wipe(old);

    /* Hack -- Allow full screen wipe */
    old->erase = TRUE;

    /* Success */
    return (0);
}





/*
 * High level graphics.
 * Redraw the whole screen.
 */
errr Term_redraw()
{
    int y;

    term_win *old = Term->old;
    term_win *scr = Term->scr;

    /* Mark the old screen as empty */
    term_win_wipe(old);

    /* Wipe the old screen mentally */
    term_win_wipe(old);

    /* Remember to erase it physically */
    old->erase = TRUE;

    /* Hack -- mark every row as invalid */
    scr->y1 = 0;
    scr->y2 = scr->h - 1;

    /* Hack -- mark every column of every row as invalid */
    for (y = 0; y < scr->h; y++)
    {
        scr->x1[y] = 0;
        scr->x2[y] = scr->w - 1;
    }

    /* Hack -- refresh */
    Term_fresh();

    /* Success */
    return (0);
}





/*
 * Save the current screen contents
 * This may or may not be currently "displayed".
 *
 * Note that up to MEM_SIZE "pending" Term_save()'s are allowed,
 * and only the ones that are used take up more than 32 bytes each.
 */
errr Term_save(void)
{
    term_win *scr = Term->scr;

    term_win *mem;

    /* Get the next memory entry */
    mem = &mem_array[mem_depth];

    /* Hack -- allocate memory screens as needed */
    if (!mem->w || !mem->h)
    {
        /* Efficiency -- Initialize "mem" as needed */	
        term_win_init(mem, scr->w, scr->h);
    }

    /* Hack -- react to changing window sizes */
    if (mem->w != scr->w || mem->h != scr->h)
    {
        /* Resize to the desired size */
        term_win_resize(mem, scr->w, scr->h);
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
    term_win *scr = Term->scr;

    term_win *mem;

    /* Hack -- Handle "errors" (see above) */
    if (mem_depth == 0) mem_depth = MEM_SIZE;

    /* Retreat the depth pointer */
    mem_depth--;

    /* Get the memory screen */
    mem = &mem_array[mem_depth];

    /* Restore the saved info */
    term_win_load(scr, mem);

    /* Success */
    return (0);
}




/*
 * Flush the output
 */
errr Term_fresh()
{
    /* Send the output */
    FlushOutput();

    /* Actually flush the output */
    Term_xtra(TERM_XTRA_FRESH, 0);

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
 *
 * Hack -- not very efficient.
 */
errr Term_resize(int w, int h)
{
    term_win *old = Term->old;
    term_win *scr = Term->scr;

    errr res = 0;

    /* Resize the "current" term_win */
    if (term_win_resize(old, w, h)) res += 1;

    /* Make a new "desired" term_win */
    if (term_win_resize(scr, w, h)) res += 2;

    /* Success */
    return (res);
}




/*
 * Show the cursor
 */
errr Term_show_cursor()
{
    term_win *scr = Term->scr;

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
    term_win *scr = Term->scr;

    /* Already hidden */
    if (!scr->cv) return (1);

    /* Remember "invisible" */
    scr->cv = 0;

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
    term_win *scr = Term->scr;

    /* Verify */
    if ((x < 0) || (x >= scr->w)) return (-1);
    if ((y < 0) || (y >= scr->h)) return (-1);

    /* Remember the cursor */
    scr->cx = x;
    scr->cy = y;

    /* The cursor is not useless */
    scr->cu = 0;

    /* Success */
    return (0);
}


/*
 * Extract the current cursor location
 */
errr Term_locate(int *x, int *y)
{
    term_win *scr = Term->scr;

    /* Access the cursor */
    (*x) = scr->cx;
    (*y) = scr->cy;

    /* Warn about "useless" cursor */
    if (scr->cu) return (1);

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
    term_win *scr = Term->scr;

    /* Verify location */
    if ((x < 0) || (x >= scr->w)) return (-1);
    if ((y < 0) || (y >= scr->h)) return (-1);

    /* Paranoia -- illegal char */
    if (!c) return (-2);

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
    term_win *scr = Term->scr;

    /* Verify location */
    if ((x < 0) || (x >= scr->w)) return (-1);
    if ((y < 0) || (y >= scr->h)) return (-1);

    /* Direct access */
    (*a) = scr->a[y][x];
    (*c) = scr->c[y][x];

    /* Success */
    return (0);
}


/*
 * Using the given attr, add the given char at the cursor.
 */
errr Term_addch(byte a, char c)
{
    term_win *scr = Term->scr;

    /* Cannot use "Useless" cursor */
    if (scr->cu) return (-1);

    /* Paranoia -- no illegal chars */
    if (!c) return (-2);

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
    term_win *scr = Term->scr;

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



/*** Input routines ***/


/*
 * Flush and forget the input
 */
errr Term_flush()
{
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
    /* Hack -- Refuse to enqueue "nul" */
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
    /* Hack -- Refuse to enqueue "nul" */
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

    /* Mega-Hack -- scan for events */
    if (Term->scan_events) {

        /* Process events (do not wait) */
        Term_xtra(TERM_XTRA_EVENT, FALSE);
    }

    /* Wait */
    if (wait) {
    
        /* Process pending events while necessary */
        while (Term->key_head == Term->key_tail) {

            /* Process events (wait for one) */
            Term_xtra(TERM_XTRA_EVENT, TRUE);
        }
    }

    /* Do not Wait */
    else {
    
        /* Process pending events if necessary */
        if (Term->key_head == Term->key_tail) {

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
    if (!t->initialized) {
        if (t->init_hook) (*t->init_hook)(t);
        t->initialized = TRUE;
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
    /* Hack -- Call the special "nuke" hook */
    if (t->initialized) {
        if (t->nuke_hook) (*t->nuke_hook)(t);
        t->initialized = FALSE;
    }

    /* Free the arrays */
    term_win_nuke(t->old);
    term_win_nuke(t->scr);

    /* Free the input queue */
    C_KILL(t->key_queue, t->key_size, char);

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


