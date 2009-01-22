/*
 * File: main-gcu.c
 * Purpose: Support for "curses" systems
 *
 * Copyright (c) 1997 Ben Harrison, and others
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


#ifdef USE_GCU

#include "main.h"

/*
 * Hack -- play games with "bool" and "term"
 */
#undef bool

/* Avoid 'struct term' name conflict with <curses.h> (via <term.h>) on AIX */
#define term System_term


/*
 * Include the proper "header" file
 */
#ifdef USE_NCURSES
# ifdef HAVE_STDBOOL_H
#  define NCURSES_ENABLE_STDBOOL_H 0
# endif

# include <ncurses.h>
#else
# include <curses.h>
#endif

#include <term.h>

#undef term



/*
 * Use POSIX terminal I/O
 */
#define USE_TPOSIX


/* Use ACS characters for walls etc */
/* #define A_ALTCHARSET */


/*
 * Hack -- Windows Console mode uses PDCURSES and cannot do any terminal stuff
 * Hack -- Windows needs Sleep(), and I really don't want to pull in all
 *         the Win32 headers for this one function
 */
#if defined(WIN32_CONSOLE_MODE)
# undef USE_TPOSIX
_stdcall void Sleep(int);
#define usleep(v) Sleep(v / 1000)
#endif

/*
 * POSIX stuff
 */
#ifdef USE_TPOSIX
# include <termios.h>
#endif



/*
 * If you have errors relating to curs_set(), comment out the following line
 */ 
#define USE_CURS_SET


/*
 * If you have errors with any of the functions mentioned below, try
 * uncommenting the line it's mentioned on.
 */
/* #define cbreak() crmode() */
/* #define nonl() */
/* #define nl() */



/*
 * Save the "normal" and "angband" terminal settings
 */

#ifdef USE_TPOSIX

static struct termios  norm_termios;
static struct termios  game_termios;

#endif


/*
 * Information about a term
 */
typedef struct term_data
{
	term t;                 /* All term info */
	WINDOW *win;            /* Pointer to the curses window */
} term_data;

/* Max number of windows on screen */
#define MAX_TERM_DATA 4

/* Information about our windows */
static term_data data[MAX_TERM_DATA];

/* Number of initialized "term" structures */
static int active = 0;



#ifdef A_COLOR

/*
 * Hack -- define "A_BRIGHT" to be "A_BOLD", because on many
 * machines, "A_BRIGHT" produces ugly "inverse" video.
 */
#ifndef A_BRIGHT
# define A_BRIGHT A_BOLD
#endif

/*
 * Software flag -- we are allowed to use color
 */
static int can_use_color = FALSE;

/*
 * Software flag -- we are allowed to change the colors
 */
static int can_fix_color = FALSE;

/*
 * Simple Angband to Curses color conversion table
 */
static int colortable[BASIC_COLORS];

/*
 * Lookup table for the "alternate character set".
 *
 * The unsigned is critical, for instance on systems like Linux
 * where some ACS characters have the high bit.
 */
static unsigned char acs_table[128];

#endif



/*
 * Place the "keymap" into its "normal" state
 */
static void keymap_norm(void)
{

#ifdef USE_TPOSIX

	/* restore the saved values of the special chars */
	(void)tcsetattr(0, TCSAFLUSH, &norm_termios);

#endif


}


/*
 * Place the "keymap" into the "game" state
 */
static void keymap_game(void)
{

#ifdef USE_TPOSIX

	/* restore the saved values of the special chars */
	(void)tcsetattr(0, TCSAFLUSH, &game_termios);

#endif
}


/*
 * Save the normal keymap
 */
static void keymap_norm_prepare(void)
{

#ifdef USE_TPOSIX

	/* Get the normal keymap */
	tcgetattr(0, &norm_termios);

#endif

}


/*
 * Save the keymaps (normal and game)
 */
static void keymap_game_prepare(void)
{

#ifdef USE_TPOSIX

	/* Acquire the current mapping */
	tcgetattr(0, &game_termios);

	/* Force "Ctrl-C" to interupt */
	game_termios.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termios.c_cc[VSUSP] = (char)26;

#ifdef VDSUSP
	/* Hack -- disable "Ctrl-Y" on *BSD */
	game_termios.c_cc[VDSUSP] = (char)-1;
#endif

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

	/* Turn off flow control (enable ^S) */
	game_termios.c_iflag &= ~IXON;

#endif


}




/*
 * Suspend/Resume
 */
static errr Term_xtra_gcu_alive(int v)
{
	int x, y;


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
		Term_xtra(TERM_XTRA_SHAPE, 1);

		/* Flush the curses buffer */
		(void)refresh();

		/* Get current cursor position */
		getyx(curscr, y, x);

		/* Move the cursor to bottom right corner */
		mvcur(y, x, LINES - 1, 0);

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


#ifdef USE_NCURSES
const char help_gcu[] = "NCurses, for terminal console, subopts -b(ig screen)";
#else /* USE_NCURSES */
const char help_gcu[] = "Curses, for terminal console, subopts -b(ig screen)";
#endif /* USE_NCURSES */


/*
 * Init the "curses" system
 */
static void Term_init_gcu(term *t)
{
	term_data *td = (term_data *)(t->data);

	/*
	 * This is necessary to keep the first call to getch()
	 * from clearing the screen
	 */
	wrefresh(stdscr);

	/* Count init's, handle first */
	if (active++ != 0) return;

	/* Erase the window */
	(void)wclear(td->win);

	/* Reset the cursor */
	(void)wmove(td->win, 0, 0);

	/* Flush changes */
	(void)wrefresh(td->win);

	/* Game keymap */
	keymap_game();
}


/*
 * Nuke the "curses" system
 */
static void Term_nuke_gcu(term *t)
{
	int x, y;
	term_data *td = (term_data *)(t->data);

	/* Delete this window */
	delwin(td->win);

	/* Count nuke's, handle last */
	if (--active != 0) return;

	/* Hack -- make sure the cursor is visible */
	Term_xtra(TERM_XTRA_SHAPE, 1);

#ifdef A_COLOR
	/* Reset colors to defaults */
	start_color();
#endif

	/* Get current cursor position */
	getyx(curscr, y, x);

	/* Move the cursor to bottom right corner */
	mvcur(y, x, LINES - 1, 0);

	/* Flush the curses buffer */
	(void)refresh();

	/* Exit curses */
	endwin();

	/* Flush the output */
	(void)fflush(stdout);

	/* Normal keymap */
	keymap_norm();
}




/*
 * Process events, with optional wait
 */
static errr Term_xtra_gcu_event(int v)
{
	int i, k;

	/* Wait */
	if (v)
	{
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
	else
	{
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

#ifdef KEY_DOWN
	/* Handle arrow keys */
	switch (i)
	{
		case KEY_DOWN:  i = ARROW_DOWN;  break;
		case KEY_UP:    i = ARROW_UP;    break;
		case KEY_LEFT:  i = ARROW_LEFT;  break;
		case KEY_RIGHT: i = ARROW_RIGHT; break;
		default:
			if (i < KEY_MIN) break;

			/* Mega-Hack -- Fold, spindle, and mutilate
			 * the keys to fit in 7 bits.
			 */

			if (i >= 252) i = KEY_F(63) - (i - 252);
			if (i >= ARROW_DOWN) i += 4;

			i = 128 + (i & 127);
			break;
	}
#endif

	/* Enqueue the keypress */
	Term_keypress(i);

	/* Success */
	return (0);
}


/*
 * React to changes
 */
static errr Term_xtra_gcu_react(void)
{

#ifdef A_COLOR

	int i;

	/* Cannot handle color redefinition */
	if (!can_fix_color) return (0);

	/* Set the colors */
	for (i = 0; i < BASIC_COLORS; i++)
	{
		/* Set one color (note scaling) */
		init_color(i, angband_color_table[i][1] * 1000 / 255,
		              angband_color_table[i][2] * 1000 / 255,
		              angband_color_table[i][3] * 1000 / 255);
	}

#endif

	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_gcu(int n, int v)
{
	term_data *td = (term_data *)(Term->data);

	/* Analyze the request */
	switch (n)
	{
		/* Clear screen */
		case TERM_XTRA_CLEAR:
		touchwin(td->win);
		(void)wclear(td->win);
		return (0);

		/* Make a noise */
		case TERM_XTRA_NOISE:
		if (write(1, "\007", 1) != 1)
			return (1);
		return (0);

		/* Flush the Curses buffer */
		case TERM_XTRA_FRESH:
		(void)wrefresh(td->win);
		return (0);

#ifdef USE_CURS_SET

		/* Change the cursor visibility */
		case TERM_XTRA_SHAPE:
		curs_set(v);
		return (0);

#endif

		/* Suspend/Resume curses */
		case TERM_XTRA_ALIVE:
		return (Term_xtra_gcu_alive(v));

		/* Process events */
		case TERM_XTRA_EVENT:
		return (Term_xtra_gcu_event(v));

		/* Flush events */
		case TERM_XTRA_FLUSH:
		while (!Term_xtra_gcu_event(FALSE));
		return (0);

		/* Delay */
		case TERM_XTRA_DELAY:
		if (v > 0)
			usleep(1000 * v);
		return (0);

		/* React to events */
		case TERM_XTRA_REACT:
		Term_xtra_gcu_react();
		return (0);
	}

	/* Unknown */
	return (1);
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_gcu(int x, int y)
{
	term_data *td = (term_data *)(Term->data);

	/* Literally move the cursor */
	wmove(td->win, y, x);

	/* Success */
	return (0);
}


/*
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_gcu(int x, int y, int n)
{
	term_data *td = (term_data *)(Term->data);

	/* Place cursor */
	wmove(td->win, y, x);

	/* Clear to end of line */
	if (x + n >= td->t.wid)
	{
		wclrtoeol(td->win);
	}

	/* Clear some characters */
	else
	{
		whline(td->win, ' ', n);
	}

	/* Success */
	return (0);
}


/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_gcu(int x, int y, int n, byte a, cptr s)
{
	term_data *td = (term_data *)(Term->data);

#ifdef A_COLOR
	/* Set the color */
	if (can_use_color) wattrset(td->win, colortable[a & (BASIC_COLORS-1)]);
#endif

	/* Move the cursor */
	wmove(td->win, y, x);

	/* Write to screen */
	while (n--) {
		unsigned int c = (unsigned char) *(s++);

		/* Map high-bit characters down using the $TERM-specific
		 * alternate character set.
		 */

#ifdef A_ALTCHARSET
		if (c > 128) c = ((int)acs_table[c - 128]) | A_ALTCHARSET;
#endif

		if ((c & 255) < ' ' || (c & 255) == 127) {
			/* Hack - replace non-ASCII characters to
			 * avoid display glitches in selectors.
			 *
			 * Note that we do this after the ACS mapping,
			 * because the display glitches we are avoiding
			 * are in curses itself.
			 */
			waddch(td->win, '?');
		} else
			waddch(td->win, c);
	}

#if defined(A_COLOR)
	/* Unset the color */
	if (can_use_color) wattrset(td->win, A_NORMAL);
#endif

	/* Success */
	return (0);
}


/*
 * Create a window for the given "term_data" argument.
 *
 * Assumes legal arguments.
 */
static errr term_data_init_gcu(term_data *td, int rows, int cols, int y, int x)
{
	term *t = &td->t;

	/* Create new window */
	td->win = newwin(rows, cols, y, x);

	/* Check for failure */
	if (!td->win)
	{
		/* Error */
		quit("Failed to setup curses window.");
	}

	/* Initialize the term */
	term_init(t, cols, rows, 256);

	/* Avoid bottom right corner */
	t->icky_corner = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Set some hooks */
	t->init_hook = Term_init_gcu;
	t->nuke_hook = Term_nuke_gcu;

	/* Set some more hooks */
	t->text_hook = Term_text_gcu;
	t->wipe_hook = Term_wipe_gcu;
	t->curs_hook = Term_curs_gcu;
	t->xtra_hook = Term_xtra_gcu;

	/* Save the data */
	t->data = td;

	/* Activate it */
	Term_activate(t);

	/* Success */
	return (0);
}


static void hook_quit(cptr str)
{
	/* Unused */
	(void)str;

	/* Exit curses */
	endwin();
}


/*
 * Prepare "curses" for use by the file "z-term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
errr init_gcu(int argc, char **argv)
{
	int i;

	int num_term = MAX_TERM_DATA, next_win = 0;

	bool use_big_screen = FALSE;


	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-b"))
		{
			use_big_screen = TRUE;
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}

	/* Extract the normal keymap */
	keymap_norm_prepare();

	/* We do it like this to prevent a link error with curseses that
	 * lack ESCDELAY.
	 */
	if (!getenv("ESCDELAY"))
		putenv("ESCDELAY=20");

	/* Initialize */
	if (initscr() == NULL) return (-1);

	/* Activate hooks */
	quit_aux = hook_quit;

	/* Require standard size screen */
	if ((LINES < 24) || (COLS < 80))
	{
		quit("Angband needs at least an 80x24 'curses' screen");
	}


#ifdef A_COLOR

	/*** Init the Color-pairs and set up a translation table ***/

	/* Do we have color, and enough color, available? */
	can_use_color = ((start_color() != ERR) && has_colors() &&
	                 (COLORS >= 8) && (COLOR_PAIRS >= 8));

#ifdef HAVE_CAN_CHANGE_COLOR

	/* Can we change colors? */
	can_fix_color = (can_use_color && can_change_color() &&
	                 orig_colors && (COLORS >= 16) && (COLOR_PAIRS > 8));

#endif

	/* Attempt to use customized colors */
	if (can_fix_color)
	{
		/* Prepare the color pairs */
		for (i = 0; i < (BASIC_COLORS / 2); i++)
		{
			/* Reset the color */
			if (init_pair(i + 1, i, 0) == ERR)
			{
				quit("Color pair init failed");
			}

			/* Set up the colormap */
			colortable[i] = (COLOR_PAIR(i + 1) | A_NORMAL);
			colortable[i + (BASIC_COLORS / 2)] = (COLOR_PAIR(i + 1) | A_BRIGHT);
		}

		/* Take account of "gamma correction" XXX XXX XXX */

		/* Prepare the "Angband Colors" */
		Term_xtra_gcu_react();
	}

	/* Attempt to use colors */
	else if (can_use_color)
	{
		/* Color-pair 0 is *always* WHITE on BLACK */

		/* Prepare the color pairs */
		init_pair(1, COLOR_RED,     COLOR_BLACK);
		init_pair(2, COLOR_GREEN,   COLOR_BLACK);
		init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
		init_pair(4, COLOR_BLUE,    COLOR_BLACK);
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6, COLOR_CYAN,    COLOR_BLACK);
		init_pair(7, COLOR_BLACK,   COLOR_BLACK);

		/* Prepare the colors */
		colortable[0] = (COLOR_PAIR(7) | A_NORMAL);	/* Black */
		colortable[1] = (COLOR_PAIR(0) | A_BRIGHT);	/* White */
		colortable[2] = (COLOR_PAIR(0) | A_NORMAL);	/* Grey XXX */
		colortable[3] = (COLOR_PAIR(1) | A_BRIGHT);	/* Orange XXX */
		colortable[4] = (COLOR_PAIR(1) | A_NORMAL);	/* Red */
		colortable[5] = (COLOR_PAIR(2) | A_NORMAL);	/* Green */
		colortable[6] = (COLOR_PAIR(4) | A_NORMAL);	/* Blue */
		colortable[7] = (COLOR_PAIR(3) | A_NORMAL);	/* Umber */
		colortable[8] = (COLOR_PAIR(7) | A_BRIGHT);	/* Dark-grey XXX */
		colortable[9] = (COLOR_PAIR(0) | A_NORMAL);	/* Light-grey XXX */
		colortable[10] = (COLOR_PAIR(5) | A_NORMAL);	/* Purple */
		colortable[11] = (COLOR_PAIR(3) | A_BRIGHT);	/* Yellow */
		colortable[12] = (COLOR_PAIR(5) | A_BRIGHT);	/* Light Red XXX */
		colortable[13] = (COLOR_PAIR(2) | A_BRIGHT);	/* Light Green */
		colortable[14] = (COLOR_PAIR(4) | A_BRIGHT);	/* Light Blue */
		colortable[15] = (COLOR_PAIR(3) | A_NORMAL);	/* Light Umber XXX */
	}

#endif

#ifdef A_ALTCHARSET
	/* Build a quick access table for the "alternate character set". */
	for (i = 0; i < 128; i++)
		acs_table[i] = i;

	for (i = 0; acs_chars && acs_chars[i] && acs_chars[i+1]; i += 2)
	{
		/* Paranoia -- the first element of an ACS mapping should
		 * be a printable ASCII character.
		 */
		if (acs_chars[i] < ' ' || acs_chars[i] > '~')
			continue;

		acs_table[(unsigned)acs_chars[i]] = acs_chars[i+1];
	}
#endif


	/*** Low level preparation ***/

	/* Paranoia -- Assume no waiting */
	nodelay(stdscr, FALSE);

	/* Prepare */
	cbreak();
	noecho();
	nonl();

	keypad(stdscr, TRUE);

	/* Extract the game keymap */
	keymap_game_prepare();


	/*** Now prepare the term(s) ***/

	/* Big screen -- one big term */
	if (use_big_screen)
	{
		/* Create a term */
		term_data_init_gcu(&data[0], LINES, COLS, 0, 0);

		/* Remember the term */
		angband_term[0] = &data[0].t;
	}

	/* No big screen -- create as many term windows as possible */
	else
	{
		/* Create several terms */
		for (i = 0; i < num_term; i++)
		{
			int rows, cols, y, x;

			/* Decide on size and position */
			switch (i)
			{
				/* Upper left */
				case 0:
				{
					rows = 24;
					cols = 80;
					y = x = 0;
					break;
				}

				/* Lower left */
				case 1:
				{
					rows = LINES - 25;
					cols = 80;
					y = 25;
					x = 0;
					break;
				}

				/* Upper right */
				case 2:
				{
					rows = 24;
					cols = COLS - 81;
					y = 0;
					x = 81;
					break;
				}

				/* Lower right */
				case 3:
				{
					rows = LINES - 25;
					cols = COLS - 81;
					y = 25;
					x = 81;
					break;
				}

				/* XXX */
				default:
				{
					rows = cols = y = x = 0;
					break;
				}
			}

			/* Skip non-existant windows */
			if (rows <= 0 || cols <= 0) continue;

			/* Create a term */
			term_data_init_gcu(&data[next_win], rows, cols, y, x);

			/* Remember the term */
			angband_term[next_win] = &data[next_win].t;

			/* One more window */
			next_win++;
		}
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Remember the active screen */
	term_screen = &data[0].t;

	/* Success */
	return (0);
}


#endif /* USE_GCU */
