/* File: main-gcu.c */

/* Purpose: Allow use of Unix "curses" with Angband -BEN- */


/*
 * To use this file, you must define "USE_GCU" in the Makefile.
 *
 * Hack -- note that "angband.h" is included AFTER the #ifdef test.
 * This was necessary because of annoying "curses.h" silliness.
 *
 * Note that this file is not "intended" to support non-Unix machines,
 * nor is it intended to support VMS or other bizarre setups.
 *
 * Also, this package assumes that the underlying "curses" handles both
 * the "nonl()" and "cbreak()" commands correctly, see the "OPTION" below.
 *
 * This code should work with most versions of "curses" or "ncurses",
 * and the "main-ncu.c" file (and USE_NCU define) are no longer used.
 *
 * See also "USE_CAP" and "main-cap.c" for code that bypasses "curses"
 * and uses the "termcap" information directly, or even bypasses the
 * "termcap" information and sends direct vt100 escape sequences.
 *
 * XXX XXX XXX We do not handle the "TERM_XTRA_FLUSH" action (!)
 *
 * XXX XXX XXX This file provides only a single "term" window.
 *
 * The "init" and "nuke" hooks are built so that only the first init and
 * the last nuke actually do anything, but the other functions are not
 * "correct" for multiple windows.  Minor changes would also be needed
 * to allow the system to handle the "locations" of the various windows.
 *
 * But in theory, it should be possible to allow a 50 line screen to be
 * split into two sub-screens (i.e. "term_screen" and "term_mirror").
 *
 * XXX XXX XXX Consider the use of "savetty()" and "resetty()".
 */

#include "angband.h"


#ifdef USE_GCU


/*
 * Hack -- play games with "bool"
 */
#define bool bool_hack

/*
 * Include the proper "header" file
 */
#ifdef USE_NCURSES
# include <ncurses.h>
#else
# include <curses.h>
#endif

/*
 * Hack -- play games with "bool"
 */
#undef bool


/*
 * Hack -- try to guess which systems use what commands
 * Hack -- allow one of the "USE_Txxxxx" flags to be pre-set.
 * Mega-Hack -- try to guess when "POSIX" is available.
 * If the user defines two of these, we will probably crash.
 */
#if !defined(USE_TPOSIX)
# if !defined(USE_TERMIO) && !defined(USE_TCHARS)
#  if defined(_POSIX_VERSION)
#   define USE_TPOSIX
#  else
#   if defined(USG) || defined(linux) || defined(SOLARIS)
#    define USE_TERMIO
#   else
#    define USE_TCHARS
#   endif
#  endif
# endif
#endif

/*
 * Hack -- Amiga uses "fake curses" and cannot do any of this stuff
 */
#if defined(AMIGA)
# undef USE_TPOSIX
# undef USE_TERMIO
# undef USE_TCHARS
#endif




/*
 * POSIX stuff
 */
#ifdef USE_TPOSIX
# include <sys/ioctl.h>
# include <termios.h>
#endif

/*
 * One version needs this file
 */
#ifdef USE_TERMIO
# include <sys/ioctl.h>
# include <termio.h>
#endif

/*
 * The other needs this file
 */
#ifdef USE_TCHARS
# include <sys/ioctl.h>
# include <sys/resource.h>
# include <sys/param.h>
# include <sys/file.h>
# include <sys/types.h>
#endif


/*
 * Hack -- dis-allow color if requested.
 */
#ifndef USE_COLOR
# undef A_COLOR
#endif


/*
 * XXX XXX Hack -- POSIX uses "O_NONBLOCK" instead of "O_NDELAY"
 *
 * They should both work due to the "(i != 1)" test below.
 */
#ifndef O_NDELAY
# define O_NDELAY O_NONBLOCK
#endif


/*
 * OPTION: some machines lack "cbreak()"
 * On these machines, we use an older definition
 */
/* #define cbreak() crmode() */


/*
 * OPTION: some machines cannot handle "nonl()" and "nl()"
 * On these machines, we can simply ignore those commands.
 */
/* #define nonl() */
/* #define nl() */


/*
 * Save the "normal" and "angband" terminal settings
 */

#ifdef USE_TPOSIX

static struct termios  norm_termios;

static struct termios  game_termios;

#endif

#ifdef USE_TERMIO

static struct termio  norm_termio;

static struct termio  game_termio;

#endif

#ifdef USE_TCHARS

static struct ltchars norm_special_chars;
static struct sgttyb  norm_ttyb;
static struct tchars  norm_tchars;
static int            norm_local_chars;

static struct ltchars game_special_chars;
static struct sgttyb  game_ttyb;
static struct tchars  game_tchars;
static int            game_local_chars;

#endif



/*
 * Hack -- Number of initialized "term" structures
 */
static int active = 0;

 
/*
 * The main screen information
 */
static term term_screen_body;


#ifdef A_COLOR

/*
 * Software flag -- we are allowed to use color
 */
static bool can_use_color = FALSE;

/*
 * Software flag -- we are allowed to change the colors
 */
static bool can_fix_color = FALSE;

/*
 * Simple Angband to Curses color conversion table
 */
static int colortable[16];

#endif



/*
 * Place the "keymap" into its "normal" state
 */
static void keymap_norm()
{

#ifdef USE_TPOSIX

    /* restore the saved values of the special chars */
    (void)tcsetattr(0, TCSAFLUSH, &norm_termios);

#endif

#ifdef USE_TERMIO

    /* restore the saved values of the special chars */
    (void)ioctl(0, TCSETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

    /* restore the saved values of the special chars */
    (void)ioctl(0, TIOCSLTC, (char *)&norm_special_chars);
    (void)ioctl(0, TIOCSETP, (char *)&norm_ttyb);
    (void)ioctl(0, TIOCSETC, (char *)&norm_tchars);
    (void)ioctl(0, TIOCLSET, (char *)&norm_local_chars);

#endif

}


/*
 * Place the "keymap" into the "game" state
 */
static void keymap_game()
{

#ifdef USE_TPOSIX

    /* restore the saved values of the special chars */
    (void)tcsetattr(0, TCSAFLUSH, &game_termios);

#endif

#ifdef USE_TERMIO

    /* restore the saved values of the special chars */
    (void)ioctl(0, TCSETA, (char *)&game_termio);

#endif

#ifdef USE_TCHARS

    /* restore the saved values of the special chars */
    (void)ioctl(0, TIOCSLTC, (char *)&game_special_chars);
    (void)ioctl(0, TIOCSETP, (char *)&game_ttyb);
    (void)ioctl(0, TIOCSETC, (char *)&game_tchars);
    (void)ioctl(0, TIOCLSET, (char *)&game_local_chars);

#endif

}


/*
 * Save the normal keymap
 */
static void keymap_norm_prepare()
{

#ifdef USE_TPOSIX

    /* Get the normal keymap */
    tcgetattr(0, &norm_termios);

#endif

#ifdef USE_TERMIO

    /* Get the normal keymap */
    (void)ioctl(0, TCGETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

    /* Get the normal keymap */
    (void)ioctl(0, TIOCGETP, (char *)&norm_ttyb);
    (void)ioctl(0, TIOCGLTC, (char *)&norm_special_chars);
    (void)ioctl(0, TIOCGETC, (char *)&norm_tchars);
    (void)ioctl(0, TIOCLGET, (char *)&norm_local_chars);

#endif

}


/*
 * Save the keymaps (normal and game)
 */
static void keymap_game_prepare()
{

#ifdef USE_TPOSIX

    /* Acquire the current mapping */
    tcgetattr(0, &game_termios);

    /* Force "CTRL-C" to interupt */
    game_termios.c_cc[VINTR] = (char)3;

    /* Force "CTRL-Z" to suspend */
    game_termios.c_cc[VSUSP] = (char)26;

    /* Hack -- Leave "VSTART/VSTOP" alone */

    /* Disable the standard control characters */
    game_termios.c_cc[VQUIT] = (char)-1;
    game_termios.c_cc[VERASE] = (char)-1;
    game_termios.c_cc[VKILL] = (char)-1;
    game_termios.c_cc[VEOF] = (char)-1;
    game_termios.c_cc[VEOL] = (char)-1;

    /* Normally, block until a character is read */
    game_termios.c_cc[VMIN] = 1;
    game_termios.c_cc[VTIME] = 0;

#endif

#ifdef USE_TERMIO

    /* Acquire the current mapping */
    (void)ioctl(0, TCGETA, (char *)&game_termio);

    /* Force "CTRL-C" to interupt */
    game_termio.c_cc[VINTR] = (char)3;

    /* Force "CTRL-Z" to suspend */
    game_termio.c_cc[VSUSP] = (char)26;

    /* Hack -- Leave "VSTART/VSTOP" alone */

    /* Disable the standard control characters */
    game_termio.c_cc[VQUIT] = (char)-1;
    game_termio.c_cc[VERASE] = (char)-1;
    game_termio.c_cc[VKILL] = (char)-1;
    game_termio.c_cc[VEOF] = (char)-1;
    game_termio.c_cc[VEOL] = (char)-1;

#if 0
    /* Disable the non-posix control characters */
    game_termio.c_cc[VEOL2] = (char)-1;
    game_termio.c_cc[VSWTCH] = (char)-1;
    game_termio.c_cc[VDSUSP] = (char)-1;
    game_termio.c_cc[VREPRINT] = (char)-1;
    game_termio.c_cc[VDISCARD] = (char)-1;
    game_termio.c_cc[VWERASE] = (char)-1;
    game_termio.c_cc[VLNEXT] = (char)-1;
    game_termio.c_cc[VSTATUS] = (char)-1;
#endif

    /* Normally, block until a character is read */
    game_termio.c_cc[VMIN] = 1;
    game_termio.c_cc[VTIME] = 0;

#endif

#ifdef USE_TCHARS

    /* Get the default game characters */
    (void)ioctl(0, TIOCGETP, (char *)&game_ttyb);
    (void)ioctl(0, TIOCGLTC, (char *)&game_special_chars);
    (void)ioctl(0, TIOCGETC, (char *)&game_tchars);
    (void)ioctl(0, TIOCLGET, (char *)&game_local_chars);

    /* Force suspend (^Z) */
    game_special_chars.t_suspc = (char)26;

    /* Cancel some things */
    game_special_chars.t_dsuspc = (char)-1;
    game_special_chars.t_rprntc = (char)-1;
    game_special_chars.t_flushc = (char)-1;
    game_special_chars.t_werasc = (char)-1;
    game_special_chars.t_lnextc = (char)-1;

    /* Force interupt (^C) */
    game_tchars.t_intrc = (char)3;

    /* Force start/stop (^Q, ^S) */
    game_tchars.t_startc = (char)17;
    game_tchars.t_stopc = (char)19;

    /* Cancel some things */
    game_tchars.t_quitc = (char)-1;
    game_tchars.t_eofc = (char)-1;
    game_tchars.t_brkc = (char)-1;

#endif

}




/*
 * Suspend/Resume
 */
static errr Term_xtra_gcu_alive(int v)
{
    /* Suspend */
    if (!v)
    {
        /* Go to normal keymap mode */
        keymap_norm();

        /* Restore modes */
        nocbreak();
        echo();
        nl();

        /* Hack -- make sure the cursor is visible */
        Term_xtra(TERM_XTRA_BEVIS, -999);

        /* Flush the curses buffer */
        (void)refresh();

#ifdef SPECIAL_BSD
        /* this moves curses to bottom right corner */
        mvcur(curscr->cury, curscr->curx, LINES - 1, 0);
#else
        /* this moves curses to bottom right corner */
        mvcur(curscr->_cury, curscr->_curx, LINES - 1, 0);
#endif

        /* Exit curses */
        endwin();

        /* Flush the output */
        (void)fflush(stdout);
    }

    /* Resume */
    else
    {
        /* Refresh */
        /* (void)touchwin(curscr); */
        /* (void)wrefresh(curscr); */

        /* Restore the settings */
        cbreak();
        noecho();
        nonl();

        /* Go to angband keymap mode */
        keymap_game();
    }

    /* Success */
    return (0);
}




/*
 * Init the "curses" system
 */
static void Term_init_gcu(term *t)
{
    /* Count init's, handle first */
    if (active++ != 0) return;
    
    /* Erase the screen */
    (void)clear();

    /* Reset the cursor */
    (void)move(0, 0);

    /* Flush changes */
    (void)refresh();

    /* Game keymap */
    keymap_game();
}


/*
 * Nuke the "curses" system
 */
static void Term_nuke_gcu(term *t)
{
    /* Count nuke's, handle last */
    if (--active != 0) return;

    /* Hack -- make sure the cursor is visible */
    Term_xtra(TERM_XTRA_BEVIS, -999);

#ifdef SPECIAL_BSD
    /* This moves curses to bottom right corner */
    mvcur(curscr->cury, curscr->curx, LINES - 1, 0);
#else
    /* This moves curses to bottom right corner */
    mvcur(curscr->_cury, curscr->_curx, LINES - 1, 0);
#endif

    /* Flush the curses buffer */
    (void)refresh();

    /* Exit curses */
    endwin();

    /* Flush the output */
    (void)fflush(stdout);

    /* Normal keymap */
    keymap_norm();
}




#ifdef USE_GETCH

/*
 * Process events, with optional wait
 */
static errr Term_xtra_gcu_event(int v)
{
    int i, k;

    /* Wait */
    if (v) {

        /* Paranoia -- Wait for it */
        nodelay(stdscr, FALSE);

        /* Get a keypress */
        i = getch();

        /* Mega-Hack -- allow graceful "suspend" */
        for (k = 0; (k < 10) && (i == ERR); k++) i = getch();

        /* Broken input is special */
        if (i == ERR) exit_game_panic();
        if (i == EOF) exit_game_panic();
    }
    
    /* Do not wait */
    else {

        /* Do not wait for it */
        nodelay(stdscr, TRUE);

        /* Check for keypresses */
        i = getch();

        /* Wait for it next time */
        nodelay(stdscr, FALSE);

        /* None ready */
        if (i == ERR) return (1);
        if (i == EOF) return (1);
    }

    /* Enqueue the keypress */
    Term_keypress(i);

    /* Success */
    return (0);
}

#else	/* USE_GETCH */

/*
 * Process events (with optional wait)
 */
static errr Term_xtra_gcu_event(int v)
{
    int i, k;
    
    char buf[2];

    /* Wait */
    if (v) {
    
        /* Wait for one byte */
        i = read(0, buf, 1);

        /* Hack -- Handle bizarre "errors" */
        if ((i <= 0) && (errno != EINTR)) exit_game_panic();
    }

    /* Do not wait */
    else {

        /* Get the current flags for stdin */
        k = fcntl(0, F_GETFL, 0);

        /* Oops */
        if (k < 0) return (1);

        /* Tell stdin not to block */
        if (fcntl(0, F_SETFL, k | O_NDELAY) < 0) return (1);

        /* Read one byte, if possible */
        i = read(0, buf, 1);

        /* Replace the flags for stdin */
        if (fcntl(0, F_SETFL, k)) return (1);
    }

    /* Ignore "invalid" keys */
    if ((i != 1) || (!buf[0])) return (1);

    /* Enqueue the keypress */
    Term_keypress(buf[0]);
    
    /* Success */
    return (0);
}

#endif	/* USE_GETCH */



/*
 * Handle a "special request"
 */
static errr Term_xtra_gcu(int n, int v)
{
    /* Analyze the request */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE: (void)write(1, "\007", 1); return (0);

        /* Flush the Curses buffer */
        case TERM_XTRA_FRESH: (void)refresh(); return (0);

#ifdef USE_CURS_SET

        /* Make the cursor invisible */
        case TERM_XTRA_INVIS: curs_set(0); return (0);

        /* Make the cursor visible */
        case TERM_XTRA_BEVIS: curs_set(1); return (0);

#endif

        /* Suspend/Resume curses */
        case TERM_XTRA_ALIVE: return (Term_xtra_gcu_alive(v));

        /* Process events */
        case TERM_XTRA_EVENT: return (Term_xtra_gcu_event(v));
    }

    return (1);
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_gcu(int x, int y, int z)
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
static errr Term_wipe_gcu(int x, int y, int w, int h)
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
static errr Term_text_gcu(int x, int y, int n, byte a, cptr s)
{
    /* Move the cursor and dump the string */
    move(y, x);

#ifdef A_COLOR
    /* Set the color */
    if (can_use_color) attrset(colortable[a & 0x0F]);
#endif

    /* Add the text */
    addstr(s);

    /* Success */
    return (0);
}




/*
 * Prepare "curses" for use by the file "term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
errr init_gcu(void)
{
    int i, y, x, err;

    term *t = &term_screen_body;


    /* Extract the normal keymap */
    keymap_norm_prepare();


#if defined(USG)
    /* Initialize for USG Unix */
    if (initscr() == NULL) return (-1);
#else
    /* Initialize for others systems */
    if (initscr() == (WINDOW*)ERR) return (-1);
#endif


    /* Hack -- Require large screen, or Quit with message */
    err = ((LINES < 24) || (COLS < 80));
    if (err) quit("Angband needs an 80x24 'curses' screen");

#if 0
    /* Hack -- Check the tabs */
    (void)move(0, 0);
    for (i = 1; i < 10; i++)
    {
        (void)addch('\t');
        getyx(stdscr, y, x);
        if (y != 0 || x != i * 8) quit("Angband needs eight space tab-stops");
    }
#endif

#ifdef A_COLOR

    /*** Init the Color-pairs and set up a translation table ***/

    /* Now let's go for a little bit of color! */
    err = (start_color() == ERR);

    /* Do we have color, and enough color, available? */
    can_use_color = (!err && has_colors() &&
                     (COLORS >= 8) && (COLOR_PAIRS >= 8));

#ifdef REDEFINE_COLORS
    /* Can we change colors? */
    can_fix_color = (can_use_color && can_change_color() &&
                     (COLOR_PAIRS >= 16));
#endif

    /* Init the Color-pairs and set up a translation table */
    /* If the terminal has enough colors */
    /* Color-pair 0 is *always* WHITE on BLACK */

    /* Only do this if we can use it */
    if (can_use_color && !can_fix_color) {

        /* Color-pair 0 is *always* WHITE on BLACK */

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

    /* Attempt to change the colors if allowed */
    if (can_use_color && can_fix_color) {

        /* Prepare the color pairs */
        for (i = 0; i < 16; i++) {
            init_pair(i, i, i);
            colortable[i] = (COLOR_PAIR(i) | A_NORMAL);
        }

        /* XXX XXX XXX See new color definitions */

        /* XXX XXX XXX Take account of "gamma correction" */

        /* Prepare the "Angband Colors" */
        init_color (0,     0,    0,    0);      /* Black */
        init_color (1,  1000, 1000, 1000);      /* White */
        init_color (2,   500,  500,  500);      /* Grey */
        init_color (3,  1000,  400,   20);      /* Orange */
        init_color (4,   900,   50,   50);      /* Red */
        init_color (5,     0,  400,   50);      /* Green */
        init_color (6,     0,    0,  900);      /* Blue */
        init_color (7,   400,  200,   20);      /* Brown */
        init_color (8,   250,  250,  250);      /* Dark-grey */
        init_color (9,   750,  750,  750);      /* Light-grey XXX */
        init_color (10,  250,    0,  750);      /* Purple */
        init_color (11,  900,  900,   50);      /* Yellow */
        init_color (12,  900,   50,   50);      /* Light Red */
        init_color (13,   50,  900,   50);      /* Light Green */
        init_color (14,   50,  600,  950);      /* Light Blue */
        init_color (15,  600,  400,  250);      /* Light Brown XXX */
    }

#endif


    /*** Low level preparation ***/

    /* Paranoia -- Assume no waiting */
    nodelay(stdscr, FALSE);

    /* Prepare */
    cbreak();
    noecho();
    nonl();

    /* Extract the game keymap */
    keymap_game_prepare();


    /*** Now prepare the term ***/

    /* Initialize the term */
    term_init(t, 80, 24, 64);

    /* Hack -- shutdown hook */
    t->init_hook = Term_init_gcu;
    t->nuke_hook = Term_nuke_gcu;

    /* Stick in some hooks */
    t->text_hook = Term_text_gcu;
    t->wipe_hook = Term_wipe_gcu;
    t->curs_hook = Term_curs_gcu;
    t->xtra_hook = Term_xtra_gcu;

    /* Save the term */
    term_screen = t;

    /* Activate it */
    Term_activate(term_screen);


    /* Success */
    return (0);
}


#endif /* USE_GCU */


