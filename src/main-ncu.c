/* File: main-ncu.c */

/* Purpose: Actual Unix "ncurses" support for Angband */
/* Author: wiebelt@mathematik.hu-berlin.de (Bernd "Bernardo" Wiebelt) */

#define bool bool_hack
#include "angband.h"
#undef bool


/*
 * This module may very well work with modern implementations of
 * normal "terminfo based curses" as well.  Let us know...
 */

#ifdef USE_NCU



/*
 * OPTION: You may have to use "#include <ncurses.h>" instead
 */
#include <curses.h>  


/*
 * Hack -- cancel color if not desired
 */
#ifndef USE_COLOR
# undef A_COLOR
#endif



/*
 * Are we "active"?  Currently unused.
 */
static int ncurses_on = FALSE;


/*
 * Can we use "color"?
 */
static bool can_use_color = FALSE;


/*
 * Angband to Ncurses color conversion table
 */
static int colortable[16];


/*
 * Currently, only a single "term" is supported here
 */
static term term_screen_body;


/*
 * Hack -- Suspend NCurses
 * See "signals.h" for usage
 */
static void unix_suspend_ncurses(void)
{
    if (!ncurses_on) return;
    
    /* XXX We may want to "undo" the following things */
    /* cbreak(); noecho(); nonl(); nodelay(stdscr, TRUE); */

    /* We are now off */
    ncurses_on = FALSE;

    /* Shut down (temporarily) */
    endwin();
}

/*
 * Restart NCurses
 */
static void unix_continue_ncurses(void)
{
    if (ncurses_on) return;
    
    /* XXX We may want to "redo" the following things */
    /* cbreak(); noecho(); nonl(); nodelay(stdscr, TRUE); */

    /* Fix the screen */
    refresh();
    
    /* Note that we are on */
    ncurses_on = TRUE;
}


/*
 * Nuke NCurses
 */
static void Term_nuke_ncu(term *t)
{
    if (!ncurses_on) return;
    
    /* Show the cursor */
    curs_set(1);
        
    /* Clear the screen */
    touchwin(stdscr);
    (void)clear();

    /* Refresh */
    refresh();
    
    /* We may want to "undo" the following things */
    /* cbreak(); noecho(); nonl(); nodelay(stdscr, TRUE); */

    /* We are now off */
    ncurses_on = FALSE;

    /* Shut down */
    endwin();
}


/*
 * Init NCurses
 */
static void Term_init_ncu(term *t)
{
    /* Prepare to be interactive */
    cbreak(); noecho(); nonl(); nodelay(stdscr, TRUE);

    /* Note that we are on */
    ncurses_on = TRUE;
}



/*
 * Check for events, without blocking
 */
static errr Term_xtra_ncu_check(int v)
{
    int i;

    /* Check for keypresses */
    i = getch();
    
    /* None ready */
    if (i == ERR) return (1);
    if (i == EOF) return (1);
        
    /* Enqueue the key */
    Term_keypress(i);

    /* Success */
    return (0);
}


/*
 * Wait for an event, and handle it.
 */
static errr Term_xtra_ncu_event(int v)
{
    int i;

    /* Wait for it */
    nodelay(stdscr, FALSE);

    /* Get a keypress */
    i = getch();

    /* Do not wait next time */
    nodelay(stdscr, TRUE);

    /* Broken input is special */
    if (i == ERR) exit_game_panic();
    if (i == EOF) exit_game_panic();

    /* Enqueue the keypress */
    Term_keypress(i);

    /* Success */
    return (0);
}
    


/*
 * Change in "activation level"
 */
static errr Term_xtra_ncu_level(int v)
{
    switch (v)
    {
        case TERM_LEVEL_HARD_SHUT: unix_suspend_ncurses(); break;
        case TERM_LEVEL_HARD_OPEN: unix_continue_ncurses(); break;
    }

    return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_ncu(int n, int v)
{
    /* Analyze the request */
    switch (n)
    {
	/* Make a noise */
	case TERM_XTRA_NOISE: (void)write(1, "\007", 1); return (0);

	/* Flush the ncurses buffer */
	case TERM_XTRA_FLUSH: (void)refresh(); return (0);

	/* Make the cursor invisible */
	case TERM_XTRA_INVIS: curs_set(0); return (0);

	/* Make the cursor visible */
	case TERM_XTRA_BEVIS: curs_set(1); return (0);

	/* Suspend ncurses */
	case TERM_XTRA_CHECK: return (Term_xtra_ncu_check(v));

	/* Suspend ncurses */
	case TERM_XTRA_EVENT: return (Term_xtra_ncu_event(v));

	/* Suspend ncurses */
	case TERM_XTRA_LEVEL: return (Term_xtra_ncu_level(v));
    }

    /* Success */
    return (1);
}




/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_ncu(int x, int y, int z)
{
    /* Literally move the cursor */
    move(y,x);

    /* Success */
    return (0);
}


/*
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_ncu(int x, int y, int w, int h)
{
    int dx, dy;

    if (!x && !y && (w >= 80) && (h >= 24))
    {
	touchwin(stdscr);
	(void)clear();
    }

    else if (!x && (h >= 24) && (w >= 80))
    {
	move(y,x);
	clrtobot();
    }

    else if (w >= 80)
    {
	for (dy = 0; dy < h; ++dy)
	{
	    move(y+dy,x);
	    clrtoeol();
	}
    }

    else
    {
	for (dy = 0; dy < h; ++dy)
	{
	    move(y+dy,x);
	    for (dx = 0; dx < w; ++dx) addch(' ');
	}
    }

    /* Hack -- Fix the cursor */
    move(y,x);

    /* Success */
    return (0);
}


/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_ncu(int x, int y, int n, byte a, cptr s)
{
    int i;
    char buf[81];

    /* Hack -- force "termination" of the text */
    if (n > 80) n = 80;
    for (i = 0; (i < n) && s[i]; ++i) buf[i] = s[i];
    buf[n]=0;

    /* Move the cursor */
    move(y, x);

#ifdef A_COLOR
    /* Set the color */
    if (can_use_color) attrset(colortable[a]);
#endif

    /* Dump the string */
    addstr(buf);

    /* Success */
    return (0);
}


/*
 * Prepare "ncurses" for use by the file "term.c"
 * Installs the "hook" functions defined above
 */
errr init_ncu(void)
{
    int err;
    
    term *t = &term_screen_body;


    /* Initialize, check for errors */
    err = (initscr() == NULL);

    /* Quit on error */
    if (err) quit("ncurses initialization failed");

    /* Check we have enough screen. */
    err = ((LINES < 24) || (COLS < 80));

    /* Quit with message */
    if (err) quit("ncurses screen must be at least 80x24");

#ifdef A_COLOR

    /* Now let's go for a little bit of color! */
    err = (start_color() == ERR); 

    /* Do we have color, and enough color, available? */
    can_use_color = (!err && has_colors && (COLORS >= 8) && (COLOR_PAIRS >= 8));

    /* Init the Color-pairs and set up a translation table */
    /* If the terminal has enough colors */
    /* Color-pair 0 is *always* WHITE on BLACK */

    /* Only do this on color machines */
    if (can_use_color) {

	/* Prepare the color pairs */
	init_pair (1, COLOR_RED,     COLOR_BLACK);
	init_pair (2, COLOR_GREEN,   COLOR_BLACK);
	init_pair (3, COLOR_YELLOW,  COLOR_BLACK);
	init_pair (4, COLOR_BLUE,    COLOR_BLACK);
	init_pair (5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair (6, COLOR_CYAN,    COLOR_BLACK);
	init_pair (7, COLOR_BLACK,   COLOR_BLACK);

	/* Prepare the "Angband Colors" -- Bright white is too bright */
	colortable[ 0] = (COLOR_PAIR(7) | A_NORMAL);      /* Black */
	colortable[ 1] = (COLOR_PAIR(0) | A_NORMAL);      /* White */
	colortable[ 2] = (COLOR_PAIR(6) | A_NORMAL);      /* Grey XXX */
	colortable[ 3] = (COLOR_PAIR(3) | A_STANDOUT);    /* Orange XXX */
	colortable[ 4] = (COLOR_PAIR(1) | A_NORMAL);      /* Red */
	colortable[ 5] = (COLOR_PAIR(2) | A_NORMAL);      /* Green */
	colortable[ 6] = (COLOR_PAIR(4) | A_NORMAL);      /* Blue */
	colortable[ 7] = (COLOR_PAIR(3) | A_NORMAL);      /* Brown */
	colortable[ 8] = (COLOR_PAIR(7) | A_STANDOUT);    /* Dark-grey XXX */
	colortable[ 9] = (COLOR_PAIR(6) | A_STANDOUT);    /* Light-grey XXX */
	colortable[10] = (COLOR_PAIR(5) | A_NORMAL);      /* Purple */
	colortable[11] = (COLOR_PAIR(3) | A_STANDOUT);    /* Yellow */
	colortable[12] = (COLOR_PAIR(1) | A_STANDOUT);    /* Light Red */
	colortable[13] = (COLOR_PAIR(2) | A_STANDOUT);    /* Light Green */
	colortable[14] = (COLOR_PAIR(4) | A_STANDOUT);    /* Light Blue */
	colortable[15] = (COLOR_PAIR(3) | A_NORMAL);      /* Light Brown XXX */
    }

#endif


    /* Initialize the term */
    term_init(t, 80, 24, 64);

    /* Stick in some hooks */
    t->nuke_hook = Term_nuke_ncu;
    t->init_hook = Term_init_ncu;

    /* Stick in some more hooks */
    t->xtra_hook = Term_xtra_ncu;
    t->curs_hook = Term_curs_ncu;
    t->wipe_hook = Term_wipe_ncu;
    t->text_hook = Term_text_ncu;

    /* Extra data -- unused */
    /* t->data = NULL; */

    /* Save the term */
    term_screen = t;
    
    /* Activate it */
    Term_activate(t);


    /* Success */
    return (0);
}

#endif /* USE_NCU */

