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

/* included for redrawing code, to prevent warnings */
#include "cmds.h"

#ifdef USE_GCU

#include "main.h"


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
 * The TERM environment variable; used for terminal capabilities.
 */
static char *termtype;
static bool loaded_terminfo;

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

#ifdef A_ALTCHARSET

/* Whether or not to use the special ACS characters */
static int use_alt_charset = 1;

#endif


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

/* Screen info: use one big Term 0, or other subwindows? */
static bool use_big_screen;

/*
 * Background color we should draw with; either BLACK or DEFAULT
 */
static int bg_color = COLOR_BLACK;

/*
 * Lookup table for the "alternate character set".
 *
 * It's worth noting that curses already has this, as an undocumented
 * (but exported) internal variable named acs_map.  I wish I could use
 * it.
 */
static unsigned int acs_table[32] = {
	0, '*', '#', '?', '?', '?', '?', '\'', '+', '?', '?', '+',
	'+', '+', '+', '+', '~', '-', '-', '-', '_', '+', '+', '+',
	'+', '|', '?', '?', '?', '?', '?', '.'
};

#define PAIR_WHITE 0
#define PAIR_RED 1
#define PAIR_GREEN 2
#define PAIR_YELLOW 3
#define PAIR_BLUE 4
#define PAIR_MAGENTA 5
#define PAIR_CYAN 6
#define PAIR_BLACK 7

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


#ifdef A_ALTCHARSET
const char help_gcu[] = "Text mode, subopts -b(ig screen) -a(scii) -g(raphics)";
#else
const char help_gcu[] = "Text mode, subopts -b(ig screen)";
#endif


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
 * For a given term number (i) set the upper left corner (x, y) and the
 * correct dimensions. Terminal layout: 0|2
 *                                      1|3
 */
void get_gcu_term_size(int i, int *rows, int *cols, int *y, int *x)
{
	if (use_big_screen && i == 0)
	{
		*rows = LINES;
		*cols = COLS;
		*y = *x = 0;
	}
	else if (use_big_screen)
	{
		*rows = *cols = *y = *x = 0;
	}
	else if (i == 0)
	{
		*rows = 24;
		*cols = 80;
		*y = *x = 0;
	}
	else if (i == 1)
	{
		*rows = LINES - 25;
		*cols = 80;
		*y = 25;
		*x = 0;
	}
	else if (i == 2)
	{
		*rows = 24;
		*cols = COLS - 81;
		*y = 0;
		*x = 81;
	}
	else if (i == 3)
	{
		*rows = LINES - 25;
		*cols = COLS - 81;
		*y = 25;
		*x = 81;
	}
	else
	{
		*rows = *cols = *y = *x = 0;
	}
}


/*
 * Query ncurses for new screen size and try to resize the GCU terms.
 */
void do_gcu_resize(void)
{
	int i, rows, cols, y, x;
	term *old_t = Term;
	
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* If we're using a big screen, we only care about Term-0 */
		if (use_big_screen && i > 0) break;
		
		/* Activate the current Term */
		Term_activate(&data[i].t);

		/* If we can resize the curses window, then resize the Term */
		get_gcu_term_size(i, &rows, &cols, &y, &x);
		if (wresize(data[i].win, rows, cols) == OK)
			Term_resize(cols, rows);

		/* Activate the old term */
		Term_activate(old_t);
	}
	do_cmd_redraw();
}


/*
 * Process events, with optional wait
 */
static errr Term_xtra_gcu_event(int v)
{
	int i, j, k;

	/* Wait */
	if (v)
	{
		/* Get a keypress; we use halfdelay(1) so if the user takes more */
		/* than 0.1 seconds we get a chance to do updates. */
		halfdelay(1);
		i = getch();
		while (i == ERR)
		{
			i = getch();
			idle_update();
		}
		cbreak();

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

	/* Not sure if this is portable to non-ncurses platforms */
	#ifdef USE_NCURSES
	if (i == KEY_RESIZE)
	{
		/* wait until we go one second (10 deci-seconds) before actually
		 * doing the resizing. users often end up triggering multiple
		 * KEY_RESIZE events while changing window size. */
		halfdelay(10);
		do {
			i = getch();
		} while (i == KEY_RESIZE);
		cbreak();
		do_gcu_resize();
		if (i == ERR) return (1);
	}
	#endif

	/* uncomment to debug keycode issues */
	#if 0
	printw("key %d", i);
	wrefresh(stdscr);
	#endif

	/* This might be a bad idea, but...
	 *
	 * Here we try to second-guess ncurses. In some cases, keypad() mode will
	 * fail to translate multi-byte escape sequences into things like number-
	 * pad actions, function keys, etc. So we can hardcode a small list of some
	 * of the most common sequences here, just in case.
	 *
	 * Notice that we turn nodelay() on. This means, that we won't accidentally
	 * interpret sequences as valid unless all the bytes are immediately
	 * available; this seems like an acceptable risk to fix problems associated
	 * with various terminal emulators (I'm looking at you PuTTY).
	 */
	if (i == 27) /* ESC */
	{
		nodelay(stdscr, TRUE);
		j = getch();
		switch (j)
		{
			case 'O':
			{
				k = getch();
				switch (k)
				{
					/* PuTTY number pad */
					case 'q': i = '1'; break;
					case 'r': i = '2'; break;
					case 's': i = '3'; break;
					case 't': i = '4'; break;
					case 'u': i = '5'; break;
					case 'v': i = '6'; break;
					case 'w': i = '7'; break;
					case 'x': i = '8'; break;
					case 'y': i = '9'; break;

					/* no match */
					case ERR: break;
					default: ungetch(k); ungetch(j);
				}
				break;
			}

			/* no match */
			case ERR: break;
			default: ungetch(j);
		}
		nodelay(stdscr, FALSE);
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


void set_256color_table(int i, int fg)
{
	init_pair(i + 1, fg, bg_color);
	colortable[i] = COLOR_PAIR(i + 1) | A_BRIGHT;
}


/*
 * React to changes
 */
static errr Term_xtra_gcu_react(void)
{

#ifdef A_COLOR

	int i;

	if (can_fix_color)
	{
		/* Initialize the curses colors to our own RGB definitions */
		for (i = 0; i < BASIC_COLORS; i++)
		{
			/* Set one color (note scaling) */
			init_color(i, angband_color_table[i][1] * 1000 / 255,
						  angband_color_table[i][2] * 1000 / 255,
						  angband_color_table[i][3] * 1000 / 255);
		}
	}
	else if (COLORS == 256)
	{
		/* If we have 256 colors, find the best matches. These numbers
		 * correspond to xterm's builtin color numbers--they do not correspond
		 * to curses' constants OR with curses' color pairs.
		 *
		 * XTerm has 216 (6*6*6) colors, with each RGB setting 0-5. So, to find
		 * the "closest" match, I just multiply Angband's RGB setting by 5 and
		 * then divide by 255. I used true rounding because using the floor
		 * produces too dark a feel, and using the ceiling kind of washes all
		 * the colors out.
		 */
		set_256color_table(TERM_DARK, 0);
		set_256color_table(TERM_WHITE, 15);
		set_256color_table(TERM_SLATE, 145);
		set_256color_table(TERM_ORANGE, 214);
		set_256color_table(TERM_RED, 160);
		set_256color_table(TERM_GREEN, 35);
		set_256color_table(TERM_BLUE, 27);
		set_256color_table(TERM_UMBER, 130);
		set_256color_table(TERM_L_DARK, 102);
		set_256color_table(TERM_L_WHITE, 188);
		set_256color_table(TERM_L_PURPLE, 201);
		set_256color_table(TERM_YELLOW, 226);
		set_256color_table(TERM_L_RED, 203);
		set_256color_table(TERM_L_GREEN, 46);
		set_256color_table(TERM_L_BLUE, 51);
		set_256color_table(TERM_L_UMBER, 179);
		set_256color_table(TERM_PURPLE, 127);
		set_256color_table(TERM_VIOLET, 135);
		set_256color_table(TERM_TEAL, 37);
		set_256color_table(TERM_MUD, 101);
		set_256color_table(TERM_L_YELLOW, 229);
		set_256color_table(TERM_MAGENTA, 199);
		set_256color_table(TERM_L_TEAL, 86);
		set_256color_table(TERM_L_VIOLET, 183);
		set_256color_table(TERM_L_PINK, 217);
		set_256color_table(TERM_MUSTARD, 184);
		set_256color_table(TERM_BLUE_SLATE, 152);
		set_256color_table(TERM_DEEP_L_BLUE, 39);
	}
	/* TODO: figure out how 88-color terminals (e.g. urxvt) work */

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

	if (x + n >= td->t.wid)
		/* Clear to end of line */
		wclrtoeol(td->win);
	else
		/* Clear some characters */
		whline(td->win, ' ', n);

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
	if (can_use_color) wattrset(td->win, colortable[a & 255]);
#endif

	/* Move the cursor */
	wmove(td->win, y, x);

	/* Write to screen */
	while (n--) {
		unsigned int c = (unsigned char) *(s++);

#ifdef A_ALTCHARSET
		/* Map high-bit characters down using the $TERM-specific
		 * alternate character set. */
		if (c < 32) c = acs_table[c];
#endif

		if ((c & 255) < ' ' || (c & 255) == 127)
			/* Hack - replace non-ASCII characters to
			 * avoid display glitches in selectors.
			 *
			 * Note that we do this after the ACS mapping,
			 * because the display glitches we are avoiding
			 * are in curses itself.
			 */
			waddch(td->win, '?');
		else
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
	int rows, cols, y, x;
	int next_win = 0;

	/* Initialize info about terminal capabilities */
	termtype = getenv("TERM");
	loaded_terminfo = termtype && tgetent(0, termtype) == 1;

	/* Let's learn about our terminal */
	use_alt_charset = loaded_terminfo && tgetstr("acs_chars", NULL);

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-b"))
			use_big_screen = TRUE;

#ifdef A_ALTCHARSET
		else if (prefix(argv[i], "-a"))
			use_alt_charset = 0;
		else if (prefix(argv[i], "-g"))
			use_alt_charset = 1;
#endif

		else
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

#ifdef HAVE_USE_DEFAULT_COLORS

	/* Should we use curses' "default color" */
	if (use_default_colors() == OK)
		bg_color = -1;

#endif

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
			if (init_pair(i + 1, i, bg_color) == ERR)
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
		/* Prepare the color pairs */
		/* PAIR_WHITE (pair 0) is *always* WHITE on BLACK */
		init_pair(PAIR_RED, COLOR_RED, bg_color);
		init_pair(PAIR_GREEN, COLOR_GREEN, bg_color);
		init_pair(PAIR_YELLOW, COLOR_YELLOW, bg_color);
		init_pair(PAIR_BLUE, COLOR_BLUE, bg_color);
		init_pair(PAIR_MAGENTA, COLOR_MAGENTA, bg_color);
		init_pair(PAIR_CYAN, COLOR_CYAN, bg_color);
		init_pair(PAIR_BLACK, COLOR_BLACK, bg_color);

		/* Prepare the colors */
		colortable[TERM_DARK]     = (COLOR_PAIR(PAIR_BLACK));
		colortable[TERM_WHITE]    = (COLOR_PAIR(PAIR_WHITE) | A_BRIGHT);
		colortable[TERM_SLATE]    = (COLOR_PAIR(PAIR_WHITE));
		colortable[TERM_ORANGE]   = (COLOR_PAIR(PAIR_RED) | A_BRIGHT);
		colortable[TERM_RED]      = (COLOR_PAIR(PAIR_RED));
		colortable[TERM_GREEN]    = (COLOR_PAIR(PAIR_GREEN));
		colortable[TERM_BLUE]     = (COLOR_PAIR(PAIR_BLUE));
		colortable[TERM_UMBER]    = (COLOR_PAIR(PAIR_YELLOW));
		colortable[TERM_L_DARK]   = (COLOR_PAIR(PAIR_BLACK) | A_BRIGHT);
		colortable[TERM_L_WHITE]  = (COLOR_PAIR(PAIR_WHITE));
		colortable[TERM_L_PURPLE] = (COLOR_PAIR(PAIR_MAGENTA));
		colortable[TERM_YELLOW]   = (COLOR_PAIR(PAIR_YELLOW) | A_BRIGHT);
		colortable[TERM_L_RED]    = (COLOR_PAIR(PAIR_MAGENTA) | A_BRIGHT);
		colortable[TERM_L_GREEN]  = (COLOR_PAIR(PAIR_GREEN) | A_BRIGHT);
		colortable[TERM_L_BLUE]   = (COLOR_PAIR(PAIR_BLUE) | A_BRIGHT);
		colortable[TERM_L_UMBER]  = (COLOR_PAIR(PAIR_YELLOW));

		colortable[TERM_PURPLE]      = (COLOR_PAIR(PAIR_MAGENTA));
		colortable[TERM_VIOLET]      = (COLOR_PAIR(PAIR_MAGENTA));
		colortable[TERM_TEAL]        = (COLOR_PAIR(PAIR_CYAN));
		colortable[TERM_MUD]         = (COLOR_PAIR(PAIR_YELLOW));
		colortable[TERM_L_YELLOW]    = (COLOR_PAIR(PAIR_YELLOW | A_BRIGHT));
		colortable[TERM_MAGENTA]     = (COLOR_PAIR(PAIR_MAGENTA | A_BRIGHT));
		colortable[TERM_L_TEAL]      = (COLOR_PAIR(PAIR_CYAN | A_BRIGHT));
		colortable[TERM_L_VIOLET]    = (COLOR_PAIR(PAIR_MAGENTA | A_BRIGHT));
		colortable[TERM_L_PINK]      = (COLOR_PAIR(PAIR_MAGENTA | A_BRIGHT));
		colortable[TERM_MUSTARD]     = (COLOR_PAIR(PAIR_YELLOW));
		colortable[TERM_BLUE_SLATE]  = (COLOR_PAIR(PAIR_BLUE));
		colortable[TERM_DEEP_L_BLUE] = (COLOR_PAIR(PAIR_BLUE));
	}

#endif

#ifdef A_ALTCHARSET
	/* Build a quick access table for the "alternate character set". */
	if (use_alt_charset)
	{
		acs_table[1] = ACS_DIAMOND;    acs_table[16] = ACS_S1;
		acs_table[2] = ACS_CKBOARD;    acs_table[18] = ACS_HLINE;
		acs_table[7] = ACS_DEGREE;     acs_table[20] = ACS_S9;
		acs_table[8] = ACS_PLMINUS;    acs_table[21] = ACS_LTEE;
		acs_table[11] = ACS_LRCORNER;  acs_table[22] = ACS_RTEE;
		acs_table[12] = ACS_URCORNER;  acs_table[23] = ACS_BTEE;
		acs_table[13] = ACS_ULCORNER;  acs_table[24] = ACS_TTEE;
		acs_table[14] = ACS_LLCORNER;  acs_table[25] = ACS_VLINE;
		acs_table[15] = ACS_PLUS;      acs_table[31] = ACS_BULLET;
	}
#endif


	/*** Low level preparation ***/

	/* Paranoia -- Assume no waiting */
	nodelay(stdscr, FALSE);

	/* Prepare */
	cbreak();
	noecho();
	nonl();

	/* Tell curses to rewrite escape sequences to KEY_UP and friends */
	keypad(stdscr, TRUE);

	/* Extract the game keymap */
	keymap_game_prepare();

	/*** Now prepare the term(s) ***/
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (use_big_screen && i > 0) break;

		/* Get the terminal dimensions; if the user asked for a big screen
		 * then we'll put the whole screen in term 0; otherwise we'll divide
		 * it amongst the available terms */
		get_gcu_term_size(i, &rows, &cols, &y, &x);
		
		/* Skip non-existant windows */
		if (rows <= 0 || cols <= 0) continue;
		
		/* Create a term */
		term_data_init_gcu(&data[next_win], rows, cols, y, x);
		
		/* Remember the term */
		angband_term[next_win] = &data[next_win].t;
		
		/* One more window */
		next_win++;
	}

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Remember the active screen */
	term_screen = &data[0].t;

	/* Success */
	return (0);
}


#endif /* USE_GCU */
