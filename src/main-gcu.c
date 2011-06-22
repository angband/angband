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
#include "buildid.h"
#include "cmds.h"

#ifdef USE_GCU
#include "main.h"
#include "files.h"

/* locale junk */
#include "locale.h"
#include "langinfo.h"

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
typedef struct term_data {
	term t;                 /* All term info */
	WINDOW *win;            /* Pointer to the curses window */
} term_data;

/* Max number of windows on screen */
#define MAX_TERM_DATA 4

/* Information about our windows */
static term_data data[MAX_TERM_DATA];

/* Number of initialized "term" structures */
static int active = 0;

#define CTRL_ORE 1
#define CTRL_WALL 2
#define CTRL_ROCK 3

static char ctrl_char[32] = {
	'\0', '*', '#', '%', '?', '?', '?', '\'', '+', '?', '?', '+',
	'+', '+', '+', '+', '~', '-', '-', '-', '_', '+', '+', '+',
	'+', '|', '?', '?', '?', '?', '?', '.'
};

static int ctrl_attr[32] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

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
 * Simple Angband to Curses color conversion table
 */
static int colortable[BASIC_COLORS];

/* Screen info: use one big Term 0, or other subwindows? */
static bool use_big_screen = FALSE;
static bool bold_extended = FALSE;

/*
 * Background color we should draw with; either BLACK or DEFAULT
 */
static int bg_color = COLOR_BLACK;


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
static void keymap_norm(void) {
#ifdef USE_TPOSIX
	(void)tcsetattr(0, TCSAFLUSH, &norm_termios);
#endif
}


/*
 * Place the "keymap" into the "game" state
 */
static void keymap_game(void) {
#ifdef USE_TPOSIX
	/* Set the game's termios settings */
	(void)tcsetattr(0, TCSAFLUSH, &game_termios);
#endif
}


/*
 * Save the normal keymap
 */
static void keymap_norm_prepare(void) {
#ifdef USE_TPOSIX
	/* Restore the normal termios settings */
	tcgetattr(0, &norm_termios);
#endif
}


/*
 * Save the keymaps (normal and game)
 */
static void keymap_game_prepare(void) {
#ifdef USE_TPOSIX
	/* Save the current termios settings */
	tcgetattr(0, &game_termios);

	/* Force "Ctrl-C" to interupt */
	game_termios.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termios.c_cc[VSUSP] = (char)26;

#ifdef VDSUSP
	/* Hack -- disable "Ctrl-Y" on *BSD */
	game_termios.c_cc[VDSUSP] = (char)-1;
#endif

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
static errr Term_xtra_gcu_alive(int v) {
	if (!v) {
		/* Suspend */
		int x, y;

		/* Go to normal keymap mode */
		keymap_norm();

		/* Restore modes */
		nocbreak();
		echo();
		nl();

		/* Hack -- make sure the cursor is visible */
		Term_xtra(TERM_XTRA_SHAPE, 1);

		/* Flush the curses buffer */
		refresh();

		/* Get current cursor position */
		getyx(stdscr, y, x);

		/* Move the cursor to bottom right corner */
		mvcur(y, x, LINES - 1, 0);

		/* Exit curses */
		endwin();

		/* Flush the output */
		fflush(stdout);

	} else {
		/* Resume */

		/* Restore the settings */
		cbreak();
		noecho();
		nonl();

		/* Go to angband keymap mode */
		keymap_game();
	}

	/* Success */
	return 0;
}

const char help_gcu[] = "Text mode, subopts -b(ig screen) -a(scii) -B(old)";

/*
 * Init the "curses" system
 */
static void Term_init_gcu(term *t) {
	term_data *td = (term_data *)(t->data);

	/*
	 * This is necessary to keep the first call to getch()
	 * from clearing the screen
	 */
	wrefresh(stdscr);

	/* Count init's, handle first */
	if (active++ != 0) return;

	/* Erase the window */
	wclear(td->win);

	/* Reset the cursor */
	wmove(td->win, 0, 0);

	/* Flush changes */
	wrefresh(td->win);

	/* Game keymap */
	keymap_game();
}


/*
 * Nuke the "curses" system
 */
static void Term_nuke_gcu(term *t) {
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
	getyx(stdscr, y, x);

	/* Move the cursor to bottom right corner */
	mvcur(y, x, LINES - 1, 0);

	/* Flush the curses buffer */
	refresh();

	/* Exit curses */
	endwin();

	/* Flush the output */
	fflush(stdout);

	/* Normal keymap */
	keymap_norm();
}


/*
 * For a given term number (i) set the upper left corner (x, y) and the
 * correct dimensions. Terminal layout: 0|2
 *                                      1|3
 */
void get_gcu_term_size(int i, int *rows, int *cols, int *y, int *x) {
	if (use_big_screen && i == 0) {
		*rows = LINES;
		*cols = COLS;
		*y = *x = 0;
	} else if (use_big_screen) {
		*rows = *cols = *y = *x = 0;
	} else if (i == 0) {
		*rows = 24;
		*cols = 80;
		*y = *x = 0;
	} else if (i == 1) {
		*rows = LINES - 25;
		*cols = 80;
		*y = 25;
		*x = 0;
	} else if (i == 2) {
		*rows = 24;
		*cols = COLS - 81;
		*y = 0;
		*x = 81;
	} else if (i == 3) {
		*rows = LINES - 25;
		*cols = COLS - 81;
		*y = 25;
		*x = 81;
	} else {
		*rows = *cols = *y = *x = 0;
	}
}


/*
 * Query ncurses for new screen size and try to resize the GCU terms.
 */
void do_gcu_resize(void) {
	int i, rows, cols, y, x;
	term *old_t = Term;
	
	for (i = 0; i < MAX_TERM_DATA; i++) {
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
static errr Term_xtra_gcu_event(int v) {
	int i, j, k;

	if (v) {
		/* Wait for a keypress; use halfdelay(1) so if the user takes more */
		/* than 0.2 seconds we get a chance to do updates. */
		halfdelay(2);
		i = getch();
		while (i == ERR) {
			i = getch();
			idle_update();
		}
		cbreak();
	} else {
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
	if (i == KEY_RESIZE) {
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
	if (i == 27) { /* ESC */
		nodelay(stdscr, TRUE);
		j = getch();
		switch (j) {
			case 'O': {
				k = getch();
				switch (k) {
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
	switch (i) {
		case KEY_DOWN:  i = ARROW_DOWN;  break;
		case KEY_UP:    i = ARROW_UP;    break;
		case KEY_LEFT:  i = ARROW_LEFT;  break;
		case KEY_RIGHT: i = ARROW_RIGHT; break;

		/* keypad keys */
		case 0xFC: i = '0'; break;
		case 0xFD: i = '.'; break;
		case 0xC0: i = '\b'; break;
		case 0xDF: i = '1'; break;
		case 0xF5: i = '3'; break;
		case 0xE9: i = '5'; break;
		case 0xC1: i = '7'; break;
		case 0xF4: i = '9'; break;

		/* try to compensate for inadequate terminfo */
		case 263: i = '\b'; break;

		default: {
			if (i < KEY_MIN) break;

			/* Mega-Hack -- Fold, spindle, and mutilate
			 * the keys to fit in 7 bits.
			 */

			if (i >= 252) i = KEY_F(63) - (i - 252);
			if (i >= ARROW_DOWN) i += 4;

			i = 128 + (i & 127);
			break;
		}
	}
#endif

	/* Enqueue the keypress */
	Term_keypress(i, 0);

	/* Success */
	return (0);
}

int scale_color(int i, int j, int scale) {
	return (angband_color_table[i][j] * (scale - 1) + 127) / 255;
}

int create_color(int i, int scale) {
	int r = scale_color(i, 1, scale);
	int g = scale_color(i, 2, scale);
	int b = scale_color(i, 3, scale);
	int rgb = 16 + scale * scale * r + scale * g + b;

	/* In the case of white and black we need to use the ANSI colors */
	if (r == g && g == b) {
		if (b == 0) rgb = 0;
		if (b == scale) rgb = 15;
	}

	return rgb;
}


/*
 * React to changes
 */
static errr Term_xtra_gcu_react(void) {

#ifdef A_COLOR
	if (COLORS == 256 || COLORS == 88) {
		/* If we have more than 16 colors, find the best matches. These numbers
		 * correspond to xterm/rxvt's builtin color numbers--they do not
		 * correspond to curses' constants OR with curses' color pairs.
		 *
		 * XTerm has 216 (6*6*6) RGB colors, with each RGB setting 0-5.
		 * RXVT has 64 (4*4*4) RGB colors, with each RGB setting 0-3.
		 *
		 * Both also have the basic 16 ANSI colors, plus some extra grayscale
		 * colors which we do not use.
		 */
		int i;
		int scale = COLORS == 256 ? 6 : 4;

		for (i = 0; i < BASIC_COLORS; i++) {
			int fg = create_color(i, scale);
			init_pair(i + 1, fg, bg_color);
			if (bold_extended)
				colortable[i] = COLOR_PAIR(i + 1) | A_BRIGHT;
			else
				colortable[i] = COLOR_PAIR(i + 1);
		}
	}
#endif

	return 0;
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_gcu(int n, int v) {
	term_data *td = (term_data *)(Term->data);

	/* Analyze the request */
	switch (n) {
		/* Clear screen */
		case TERM_XTRA_CLEAR: touchwin(td->win); wclear(td->win); return 0;

		/* Make a noise */
		case TERM_XTRA_NOISE: write(1, "\007", 1); return 0;

		/* Flush the Curses buffer */
		case TERM_XTRA_FRESH: wrefresh(td->win); return 0;

#ifdef USE_CURS_SET
		/* Change the cursor visibility */
		case TERM_XTRA_SHAPE: curs_set(v); return 0;
#endif

		/* Suspend/Resume curses */
		case TERM_XTRA_ALIVE: return Term_xtra_gcu_alive(v);

		/* Process events */
		case TERM_XTRA_EVENT: return Term_xtra_gcu_event(v);

		/* Flush events */
		case TERM_XTRA_FLUSH: while (!Term_xtra_gcu_event(FALSE)); return 0;

		/* Delay */
		case TERM_XTRA_DELAY: if (v > 0) usleep(1000 * v); return 0;

		/* React to events */
		case TERM_XTRA_REACT: Term_xtra_gcu_react(); return 0;
	}

	/* Unknown event */
	return 1;
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_gcu(int x, int y) {
	term_data *td = (term_data *)(Term->data);
	wmove(td->win, y, x);
	return 0;
}


/*
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_gcu(int x, int y, int n) {
	term_data *td = (term_data *)(Term->data);

	wmove(td->win, y, x);

	if (x + n >= td->t.wid)
		/* Clear to end of line */
		wclrtoeol(td->win);
	else
		/* Clear some characters */
		whline(td->win, ' ', n);

	return 0;
}

/*
 * Since GCU currently only supports Latin-1 extended chracters, we only
 * install this hook if we're not using UTF-8.
 * Given a position in the ISO Latin-1 character set, return the correct
 * character on this system. Currently
 */
 static byte Term_xchar_gcu(byte c) {
	return c;
}


/* Hack - replace non-ASCII characters to
 * avoid display glitches in selectors.
 *
 * Note that we do this after the ACS mapping,
 * because the display glitches we are avoiding
 * are in curses itself.
 */
char filter_char(char c) {
	if (c < ' ' || c >= 127)
		return '?';
	else
		return c;
}


/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_gcu(int x, int y, int n, byte a, const char *s) {
	term_data *td = (term_data *)(Term->data);

#ifdef A_COLOR
	/* Set the color */
	if (can_use_color) (void)wattrset(td->win, colortable[a & 255]);
#endif

	/* Move the cursor */
	wmove(td->win, y, x);

	/* Write to screen */
	while (n--) {
		unsigned char c = *(s++);

		if (c < 32) {
			wattron(td->win, ctrl_attr[c]);
			waddch(td->win, filter_char(ctrl_char[c]));
			wattroff(td->win, ctrl_attr[c]);
		} else {
			waddch(td->win, filter_char(c));
		}
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
static errr term_data_init_gcu(term_data *td, int rows, int cols, int y, int x) {
	term *t = &td->t;

	/* Create new window */
	td->win = newwin(rows, cols, y, x);

	/* Check for failure */
	if (!td->win)
		quit("Failed to setup curses window.");

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

	/* only if the locale supports Latin-1 will we enable xchar_hook */
	if (setlocale(LC_CTYPE, "")) {
		/* the Latin-1 codeset is ISO-8859-1 */
		if (strcmp(nl_langinfo(CODESET), "ISO-8859-1") == 0)
			t->xchar_hook = Term_xchar_gcu;
	}

	/* Save the data */
	t->data = td;

	/* Activate it */
	Term_activate(t);

	/* Success */
	return (0);
}

static void hook_quit(const char *str) {
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
errr init_gcu(int argc, char **argv) {
	int i;
	int rows, cols, y, x;
	int next_win = 0;
	bool graphics = TRUE;

	/* Initialize info about terminal capabilities */
	termtype = getenv("TERM");
	loaded_terminfo = termtype && tgetent(0, termtype) == 1;

	/* Parse args */
	for (i = 1; i < argc; i++) {
		if (prefix(argv[i], "-b"))
			use_big_screen = TRUE;
		else if (prefix(argv[i], "-B"))
			bold_extended = TRUE;
		else if (prefix(argv[i], "-a"))
			graphics = FALSE;
		else
			plog_fmt("Ignoring option: %s", argv[i]);
	}

	if (graphics) {
		ctrl_char[CTRL_WALL] = ' ';
		ctrl_attr[CTRL_ORE] = A_REVERSE;
		ctrl_attr[CTRL_WALL] = A_REVERSE;
		ctrl_attr[CTRL_ROCK] = A_REVERSE;
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
	if (LINES < 24 || COLS < 80)
		quit("Angband needs at least an 80x24 'curses' screen");

#ifdef A_COLOR
	/* Do we have color, and enough color, available? */
	can_use_color = ((start_color() != ERR) && has_colors() &&
					 (COLORS >= 8) && (COLOR_PAIRS >= 8));

#ifdef HAVE_USE_DEFAULT_COLORS
	/* Should we use curses' "default color" */
	if (use_default_colors() == OK) bg_color = -1;
#endif

	/* Attempt to use colors */
	if (can_use_color) {
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
		colortable[TERM_ORANGE]   = (COLOR_PAIR(PAIR_YELLOW) | A_BRIGHT);
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
	for (i = 0; i < MAX_TERM_DATA; i++) {
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
