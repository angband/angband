/**
 * \file main-gcu.c
 * \brief Support for "curses" systems
 *
 * Copyright (c) 1997 Ben Harrison, and others
 * Copyright (c) 2009-2015 Erik Osheim
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
#include "cave.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-prefs.h"

#ifdef USE_GCU
#include "main.h"
#ifdef _WIN32
/*
 * Microsoft puts write() in io.h and marks it with a deprecation warning.
 * Use the equivalent _write() instead.
 */
#include <io.h>
#define PLATFORM_WRITE _write
#else
#define PLATFORM_WRITE write
#endif

/**
 * Avoid 'struct term' name conflict with <curses.h> (via <term.h>) on AIX
 */
#define term System_term

/**
 * Include the proper "header" file
 */
#ifdef USE_NCURSES
# ifdef HAVE_STDbool_H
#  define NCURSES_ENABLE_STDbool_H 0
# endif

/* Mac needs _XOPEN_SOURCE_EXTENDED to expose mvwaddnstr(). */
# define _XOPEN_SOURCE_EXTENDED 1

# include <ncurses.h>
#else
# include <curses.h>
#endif

#include <term.h>

#undef term

/**
 * Use POSIX terminal I/O
 */
#define USE_TPOSIX


/**
 * Hack -- Windows Console mode uses PDCURSES and cannot do any terminal stuff
 * Hack -- Windows needs Sleep(), and I really don't want to pull in all
 *         the Win32 headers for this one function
 */
#if defined(WIN32_CONSOLE_MODE)
# undef USE_TPOSIX
_stdcall void Sleep(int);
#define usleep(v) Sleep(v / 1000)
#endif

/**
 * POSIX stuff
 */
#ifdef USE_TPOSIX
# include <termios.h>
#endif

/**
 * If you have errors relating to curs_set(), comment out the following line
 */
#define USE_CURS_SET

/**
 * If you have errors with any of the functions mentioned below, try
 * uncommenting the line it's mentioned on.
 */
/* #define cbreak() crmode() */
/* #define nonl() */
/* #define nl() */

/**
 * Save the "normal" and "angband" terminal settings
 */

#ifdef USE_TPOSIX

static struct termios  norm_termios;
static struct termios  game_termios;

#endif

/**
 * The TERM environment variable; used for terminal capabilities.
 */
static char *termtype;
static bool loaded_terminfo;

/**
 * Simple rectangle type
 */
struct rect_s
{
    int  x,  y;
    int cx, cy;
};
typedef struct rect_s rect_t, *rect_ptr;

/* Trivial rectangle utility to make code a bit more readable */
static rect_t rect(int x, int y, int cx, int cy)
{
    rect_t r;
    r.x = x;
    r.y = y;
    r.cx = cx;
    r.cy = cy;
    return r;
}

/**
 * Information about a term
 */
typedef struct term_data {
	term t;                 /* All term info */
	rect_t r;
	WINDOW *win;            /* Pointer to the curses window */
} term_data;

/* Max number of windows on screen */
#define MAX_TERM_DATA 6

/* Minimum main term size */
#define MIN_TERM0_LINES 24
#define MIN_TERM0_COLS 80

/* Comfortable subterm size */
#define COMFY_SUBTERM_LINES 5
#define COMFY_SUBTERM_COLS 40

/* Information about our windows */
static term_data data[MAX_TERM_DATA];

/* Number of initialized "term" structures */
static int active = 0;

#ifdef A_COLOR

/**
 * Hack -- define "A_BRIGHT" to be "A_BOLD", because on many
 * machines, "A_BRIGHT" produces ugly "inverse" video.
 */
#ifndef A_BRIGHT
# define A_BRIGHT A_BOLD
#endif

/**
 * Software flag -- we are allowed to use color
 */
static int can_use_color = false;

/**
 * Simple Angband to Curses color conversion table
 */
static int colortable[BASIC_COLORS];

/**
 * Same colors as in the colortable, except fg and bg are both set to the fg value.
 * These pairs are used for drawing solid walls.
 */
static int same_colortable[BASIC_COLORS];

/* Screen info: use one big Term 0, or other subwindows? */
static bool bold_extended = false;
static bool use_default_background = false;
static bool keep_terminal_colors = false;
static int term_count = 1;

/**
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
#define PAIR_WHITE_WHITE 8
#define PAIR_RED_RED 9
#define PAIR_GREEN_GREEN 10
#define PAIR_YELLOW_YELLOW 11
#define PAIR_BLUE_BLUE 12
#define PAIR_MAGENTA_MAGENTA 13
#define PAIR_CYAN_CYAN 14
#define PAIR_BLACK_BLACK 15

#endif

/**
 * Place the "keymap" into its "normal" state
 */
static void keymap_norm(void) {
#ifdef USE_TPOSIX
	(void)tcsetattr(0, TCSAFLUSH, &norm_termios);
#endif
}


/**
 * Place the "keymap" into the "game" state
 */
static void keymap_game(void) {
#ifdef USE_TPOSIX
	/* Set the game's termios settings */
	(void)tcsetattr(0, TCSAFLUSH, &game_termios);
#endif
}


/**
 * Save the normal keymap
 */
static void keymap_norm_prepare(void) {
#ifdef USE_TPOSIX
	/* Restore the normal termios settings */
	tcgetattr(0, &norm_termios);
#endif
}


/**
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


/**
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

const char help_gcu[] = "Text mode, subopts\n"
	"              -B     Use brighter bold characters\n"
	"              -D     Use terminal default background color\n"
	"              -K     Keep terminal's color table when changing colors\n"
	"              -nN    Use N terminals (up to 6)";

/**
 * Usage:
 *
 * angband -mgcu -- [-B] [-D] [-nN]
 *
 *   -B      Use brighter bold characters
 *   -D      Use terminal default background color
 *   -nN     Use N terminals (up to 6)
 */

#ifdef MSYS2_ENCODING_WORKAROUND
/*
 * Override the default character encoding in MSYS2 for better handling in
 * Term_text_gcu() of characters that don't fall in the ASCII range.  See
 * init_sdl2() in main-sdl2.c for more details about the default character
 * encoding in MSYS2.
 */

/**
 * Convert UTF-8 to UTF-16 with each UTF-16 in the native byte order and
 * lossily change any code point that requires a surrogate pair to U+FFFD.
 * Return the total number of code points that would be generated by
 * converting the UTF-8 input.
 *
 * \param dest Points to the buffer in which to store the conversion.  May be
 * NULL.
 * \param src Is a null-terminated UTF-8 sequence.
 * \param n Is the maximum number of code points to store in dest.
 *
 * In case of malformed UTF-8, inserts a U+FFFD in the converted output at the
 * point of the error.
 */
static size_t Term_mbcs_gcu_msys2(wchar_t *dest, const char *src, int n)
{
        size_t nout = (n > 0) ? n : 0;
        size_t count = 0;

        while (1) {
                /*
                 * Default to U+FFFD to indicate an erroneous UTF-8 sequence
                 * that could not be decoded.  Follow "best practice"
                 * recommended by the Unicode 6 standard:  an erroneous
		 * sequence ends as soon as a disallowed byte is encountered.
                 */
                uint32_t decoded = 0xfffd;

                if (((unsigned int) *src & 0x80) == 0) {
                        /*
                         * Encoded as a single byte:  U+0000 to U+007F ->
                         * 0xxxxxxx.
                         */
                        if (*src == 0) {
                                if (dest && count < nout) {
                                        dest[count] = 0;
                                }
                                break;
                        }
                        decoded = *src;
                        ++src;
                } else if (((unsigned int) *src & 0xe0) == 0xc0) {
                        /*
                         * Encoded as two bytes:  U+0080 to U+07FF ->
                         * 110xxxxx 10xxxxxx.
                         */
                        uint32_t part = ((uint32_t) *src & 0x1f) << 6;

                        ++src;
                        /*
                         * Check that the first two bits of the continuation
                         * byte are valid and the encoding is not overlong.
                         */
                        if (((unsigned int) *src & 0xc0) == 0x80
                                        && part > 0x40) {
                                decoded = part + ((uint32_t) *src & 0x3f);
                                ++src;
                        }
                } else if (((unsigned int) *src & 0xf0) == 0xe0) {
                        /*
                         * Encoded as three bytes:  U+0800 to U+FFFF ->
                         * 1110xxxx 10xxxxxx 10xxxxxx.
                         */
                        uint32_t part = ((uint32_t) *src & 0xf) << 12;

                        ++src;
                        if (((unsigned int) *src & 0xc0) == 0x80) {
                                part += ((uint32_t) *src & 0x3f) << 6;
                                ++src;
                                /*
                                 * The second part of the test rejects
                                 * overlong encodings.  The third part
                                 * rejects encodings of U+D800 to U+DFFF,
                                 * reserved for surrogate pairs.
                                 */
                                if (((unsigned int) *src & 0xc0) == 0x80
                                                && part >= 0x800
                                                && (part & 0xf800) != 0xd800) {
                                        decoded = part
                                                + ((uint32_t) *src & 0x3f);
                                        ++src;
                                }
                        }
                } else if (((unsigned int) *src & 0xf8) == 0xf0) {
                        /*
                         * Encoded as four bytes:  U+10000 to U+1FFFFF ->
                         * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
                         */
                        uint32_t part = ((uint32_t) *src & 0x7) << 18;

                        ++src;
                        if (((unsigned int) *src & 0xc0) == 0x80) {
                                part += ((uint32_t) * src & 0x3f) << 12;
                                ++src;
                                /*
                                 * The second part of the test rejects
                                 * overlong encodings.  The third part
                                 * rejects code points beyond U+10FFFF which
                                 * can't be encoded in UTF-16.
                                 */
                                if (((unsigned int) *src & 0xc0) == 0x80
                                                && part >= 0x10000
                                                && (part & 0xff0000)
                                                <= 0x100000) {
                                        part += ((uint32_t) *src & 0x3f) << 6;
                                        ++src;
                                        if (((unsigned int) *src & 0xc0)
                                                        == 0x80) {
                                                decoded = part
                                                        + ((uint32_t) *src
                                                        & 0x3f);
                                                ++src;
                                        }
                                }
                        }
                } else {
                        /*
                         * Either an impossible byte or one that signals the
                         * start of a five byte or longer encoding.
                         */
                        ++src;
                }
                if (dest && count < nout) {
                        if (decoded >= 0x10000) {
                                /*
                                 * Would require a surrogate pair to represent
                                 * accurately.  Substitute U+FFFD instead.
                                 */
                                assert(decoded <= 0x10FFFF);
                                dest[count] = (wchar_t) 0xfffd;
                        } else {
                                /*
                                 * By the decoding logic, the decoded value
                                 * should not match part of surrogate pair.
                                 */
                                assert(decoded < 0xd800 || decoded > 0xdfff);
                                dest[count] = (wchar_t) decoded;
                        }
                }
                ++count;
        }
	return count;
}

/**
 * Convert a UTF-16 stored in the native byte order to UTF-8.
 * \param s Points to the buffer where the conversion should be stored.
 * That buffer must have at least Term_wcsz_gcu_msys2() bytes.
 * \param wchar Is the UTF-16 value to convert.
 * \return The returned value is the number of bytes written to s or -1
 * if the UTF-16 value could not be converted.
 *
 * This is a necessary counterpart to Term_mbcs_gcu_msys2():  since we
 * are overriding the default multibyte to wide character conversion, need
 * to override the reverse conversion as well.
 */
static int Term_wctomb_gcu_msys2(char *s, wchar_t wchar)
{
        if ((unsigned int) wchar <= 0x7f) {
                *s = wchar;
                return 1;
        }
        if ((unsigned int) wchar < 0x7ff) {
                *s++ = 0xc0 + (((unsigned int) wchar & 0x7c0) >> 6);
                *s++ = 0x80 + ((unsigned int) wchar & 0x3f);
                return 2;
        }
        /* Refuse to encode a value reserved for surrogate pairs in UTF-16. */
        if ((unsigned int) wchar >= 0xd800 && (unsigned int) wchar <= 0xdfff) {
                return -1;
        }
        *s++ = 0xe0 + (((unsigned int) wchar & 0xf000) >> 12);
        *s++ = 0x80 + (((unsigned int) wchar & 0xfc0) >> 6);
        *s++ = 0x80 + ((unsigned int) wchar & 0x3f);
        return 3;
}

/**
 * Return whether a UTF-16 value is printable.
 *
 * This is a necessary counterpart to Term_mbcs_gcu_msys2() so that
 * screening of wide characters in the core's text_out_to_screen() is
 * consistent with what Term_mbcs_gcu_msys2() does.
 */
static int Term_iswprint_gcu_msys2(wint_t wc)
{
       /*
         * Upcast the UTF-16 value to UTF-32 since the UTF-16 value is either
         * equal to the code point's value or is part of a surrogate pair.
         */
        return utf32_isprint((uint32_t) wc);
}

/**
 * Return the maximum number of bytes needed for a multibyte encoding of a
 * wchar.
 */
static int Term_wcsz_gcu_msys2(void)
{
        /*
         * UTF-8 takes at most 3 bytes to encode a code point that can be
         * encoded with a single 16-bit quantity in UTF-16.
         */
        return 3;
}

#endif /* MSYS2_ENCODING_WORKAROUND */

/**
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

	#if defined(USE_NCURSES) && defined(KEY_MOUSE)
	/* Turn on the mouse. */
	mousemask(ALL_MOUSE_EVENTS, NULL);
	#endif

	/* Erase the window */
	wclear(td->win);

	/* Reset the cursor */
	wmove(td->win, 0, 0);

	/* Flush changes */
	wrefresh(td->win);

	/* Game keymap */
	keymap_game();

#ifdef MSYS2_ENCODING_WORKAROUND
	/*
	 * Override default character encoding on MSYS2; see main-sdl2.c for
	 * more details.
	 */
	text_mbcs_hook = Term_mbcs_gcu_msys2;
	text_wctomb_hook = Term_wctomb_gcu_msys2;
	text_wcsz_hook = Term_wcsz_gcu_msys2;
	text_iswprint_hook = Term_iswprint_gcu_msys2;
#endif /* MSYS2_ENCODING_WORKAROUND */
}


/**
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

/**
 * Helper function for get_gcu_term_size:
 * Given inputs, populates size and start (rows and y, or cols and x)
 * with correct values for a group (column or row) of terms.
 *   term_group_index: the placement of the group, e.g.
 *     top row is 0
 *   term_group_count: the number of groups in this dimension (2 or 3)
 *   window_size:      the number of grids the window has in this dimension
 *   min_term0_size:   the minimum main term size in this dimension 
 *     (80 or 24), also the maximum subterm size
 *   comfy_subterm_size: in balancing among three groups, we first give the
 *     main term its minimum, and then allocate evenly between the other
 *     two subterms until they are both comfy_subterm_size, at which point
 *     we grow the outer subterm until it reaches min_term0_size. (The
 *     middle subterm then grows until min_term0_size, and any further
 *     window space goes to the main term.)
 */
static void balance_dimension(int *size, int *start, int term_group_index,
	int term_group_count, int window_size, int min_term0_size,
	int comfy_subterm_size) {
	/* Convenience variable for clarity.
	 * Note that it is also the number of separator rows/columns */
	int subterm_group_count = term_group_count - 1;

	if (term_group_index == 0) { 
		/* main term */
		*size = MAX(min_term0_size, window_size - subterm_group_count*(min_term0_size + 1)); 
		*start = 0;
	} else if (term_group_index == term_group_count - 1) { 
		/* outer or only subterm */
		if (window_size <= min_term0_size + subterm_group_count*(comfy_subterm_size + 1)) { 
			/* Not enough room for min term0 and all subterms comfy.
			 * Note that we round up here and down for the middle subterm*/
			*size = (window_size - min_term0_size - subterm_group_count) / subterm_group_count;
			if (window_size > min_term0_size + subterm_group_count + *size * subterm_group_count)
				(*size)++;
		} else {
			*size = MIN(min_term0_size, window_size - min_term0_size - comfy_subterm_size*(subterm_group_count - 1) - subterm_group_count);
		}
		*start = window_size - *size;
	} else {
		/* middle subterm */
		if (window_size <= subterm_group_count*(min_term0_size + 1) + comfy_subterm_size) {
			/* Outer subterm(s) not yet full-sized, thus at most comfy */
			*size = MIN(comfy_subterm_size, (window_size - min_term0_size - subterm_group_count) / subterm_group_count);
		} else {
			*size = MIN(min_term0_size, window_size - subterm_group_count*(min_term0_size + 1));
		}
		*start = 1 + MAX(min_term0_size, window_size - subterm_group_count*(min_term0_size + 1));
	}
}

/**
 * For a given term number (i) set the upper left corner (x, y) and the
 * correct dimensions. Remember to leave one row and column between
 * subterms.
 */
static void get_gcu_term_size(int i, int *rows, int *cols, int *y, int *x) {
	bool is_wide = (10 * LINES < 3 * COLS);
	int term_rows = 1;
	int term_cols = 1;
	int term_row_index = 0;
	int term_col_index = 0;

	assert(i < term_count);

	/* For sufficiently small windows, we can only use one term.
	 * Each additional row/column of terms requires at least two lines
	 * for the separators. If everything is as square as possible, 
	 * the 3rd, 7th, 13th, etc. terms add to the short dimension, while
	 * the 2nd, 5th, 10th, etc. terms add to the long dimension.
	 * However, three terms are the special case of 1x3 or 3x1.
	 */
	if (is_wide) {
		while (term_rows*(term_rows + 1) < term_count) term_rows++;
		while (term_cols*term_cols < term_count) term_cols++;
		if (term_count == 3) {
			term_rows = 1;
			term_cols = 3;
		}
		term_col_index = i % term_cols;
		term_row_index = (int)(i / term_cols);
	} else { /* !is_wide */ 
		while (term_rows*term_rows < term_count) term_rows++;
		while (term_cols*(term_cols + 1) < term_count) term_cols++;
		if (term_count == 3) {
			term_rows = 3;
			term_cols = 1;
		}
		term_col_index = (int)(i / term_rows);
		term_row_index = i % term_rows;
	}

	if (LINES < MIN_TERM0_LINES + 2 * (term_rows - 1) ||
		COLS < MIN_TERM0_COLS + 2 * (term_cols - 1)) {
		term_rows = term_cols = term_count = 1;
		if (i != 0) {
			*rows = *cols = *y = *x = 0;
		}
		term_col_index = term_row_index = 0;
	}
		
	balance_dimension(cols, x, term_col_index, term_cols, COLS, MIN_TERM0_COLS, COMFY_SUBTERM_COLS);
	balance_dimension(rows, y, term_row_index, term_rows, LINES, MIN_TERM0_LINES, COMFY_SUBTERM_LINES);
}


/**
 * Query ncurses for new screen size and try to resize the GCU terms.
 */
static void do_gcu_resize(void) {
	int i, rows, cols, y, x;
	term *old_t = Term;
	
	for (i = 0; i < term_count; i++) {
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


/**
 * Process events, with optional wait
 */
static errr Term_xtra_gcu_event(int v) {
	int i, j, k, mods=0;

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
		nodelay(stdscr, true);

		/* Check for keypresses */
		i = getch();

		/* Wait for it next time */
		nodelay(stdscr, false);

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

	#if defined(USE_NCURSES) && defined(KEY_MOUSE)
	if (i == KEY_MOUSE) {
		MEVENT m;
		if (getmouse(&m) != OK) return (0);

		int b = 0;
		if (m.bstate & BUTTON1_CLICKED) b = 1;
		else if (m.bstate & BUTTON2_CLICKED) b = 2;
		else if (m.bstate & BUTTON3_CLICKED) b = 3;
		else if (m.bstate & BUTTON4_CLICKED) b = 4;
		if (m.bstate & BUTTON_SHIFT) b |= (KC_MOD_SHIFT << 4);
		if (m.bstate & BUTTON_CTRL) b |= (KC_MOD_CONTROL << 4);
		if (m.bstate & BUTTON_ALT) b |= (KC_MOD_ALT << 4);

		if (b != 0) Term_mousepress(m.x, m.y, b);
		return (0);
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
		nodelay(stdscr, true);
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
		nodelay(stdscr, false);
	}

#ifdef KEY_DOWN
	/* Handle arrow keys */
	switch (i) {
		case KEY_DOWN:  i = ARROW_DOWN;  break;
		case KEY_UP:    i = ARROW_UP;    break;
		case KEY_LEFT:  i = ARROW_LEFT;  break;
		case KEY_RIGHT: i = ARROW_RIGHT; break;
		case KEY_DC:    i = KC_DELETE; break;
		case KEY_BACKSPACE: i = KC_BACKSPACE; break;
		case KEY_ENTER: i = KC_ENTER; mods |= KC_MOD_KEYPAD; break;
		case 9:         i = KC_TAB; break;
		case 13:        i = KC_ENTER; break;
		case 27:        i = ESCAPE; break;

		/* keypad keys */
		case 0xFC: i = '0'; break;
		case 0xFD: i = '.'; break;
		case 0xC0: i = '\b'; break;
		case 0xDF: i = '1'; break;
		case 0xF5: i = '3'; break;
		case 0xE9: i = '5'; break;
		case 0xC1: i = '7'; break;
		case 0xF4: i = '9'; break;

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
	Term_keypress(i, mods);

	/* Success */
	return (0);
}

static int scale_color(int i, int j, int scale) {
	return (angband_color_table[i][j] * (scale - 1) + 127) / 255;
}

static int create_color(int i, int scale) {
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


/**
 * Adjust the color tables if there's more than 16 available.
 */
static void handle_extended_color_tables(void) {
#ifdef A_COLOR
	if (COLORS == 256 || COLORS == 88) {
		int isbold = bold_extended ?  A_BRIGHT : A_NORMAL;
		int i;

		if (keep_terminal_colors) {
			/*
			 * If we have more than 16 colors, find the best
			 * matches in the terminal's default color table.
			 * These numbers correspond to xterm/rxvt's builtin
			 * color numbers--they do not correspond to curses'
			 * constants OR with curses' color pairs.
			 *
			 * XTerm has 216 (6*6*6) RGB colors, with each RGB
			 * setting 0-5.
			 * RXVT has 64 (4*4*4) RGB colors, with each RGB
			 * setting 0-3.
			 *
			 * Both also have the basic 16 ANSI colors, plus some
			 * extra grayscale colors which we do not use.
			 */
			int scale = COLORS == 256 ? 6 : 4;

			bg_color = create_color(COLOUR_DARK, scale);
			for (i = 0; i < BASIC_COLORS; i++) {
				int fg = create_color(i, scale);
				init_pair(i + 1, fg, bg_color);
				colortable[i] = COLOR_PAIR(i + 1) | isbold;
				init_pair(BASIC_COLORS + i, fg, fg);
				same_colortable[i] =
					COLOR_PAIR(BASIC_COLORS + i) | isbold;
			}
		} else {
			bg_color = 0;
			for (i = 0; i < BASIC_COLORS; i++) {
				/*
				 * Scale components to a range of 0 - 1000 per
				 * init_color()'s documentation.
				 */
				init_color(i,
					(angband_color_table[i][1] * 1001) / 256,
					(angband_color_table[i][2] * 1001) / 256,
					(angband_color_table[i][3] * 1001) / 256);
				init_pair(i + 1, i, bg_color);
				colortable[i] = COLOR_PAIR(i + 1) | isbold;
				init_pair(BASIC_COLORS + i, i, i);
				same_colortable[i] =
					COLOR_PAIR(BASIC_COLORS + i) | isbold;
			}
		}

		for (i = 0; i < term_count; ++i) {
			if (data[i].win) {
				wbkgdset(data[i].win, ' ' |
					colortable[COLOUR_DARK]);
			}
		}
		if (data[0].win) {
			/*
			 * Adjust the background color on the standard screen
			 * as well so separators between the terminals have
			 * the same background as the rest.
			 */
			chtype term0_bkg = getbkgd(data[0].win);

			if (getbkgd(stdscr) != term0_bkg) {
				wbkgd(stdscr, term0_bkg);
				wrefresh(stdscr);
			}
		}
	}
#endif
}


/**
 * Handle a "special request"
 */
static errr Term_xtra_gcu(int n, int v) {
	term_data *td = (term_data *)(Term->data);

	/* Analyze the request */
	switch (n) {
		/* Clear screen */
		case TERM_XTRA_CLEAR: touchwin(td->win); wclear(td->win); return 0;

		/* Make a noise */
		case TERM_XTRA_NOISE: PLATFORM_WRITE(1, "\007", 1); return 0;

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
		case TERM_XTRA_FLUSH: while (!Term_xtra_gcu_event(false)); return 0;

		/* Delay */
		case TERM_XTRA_DELAY: if (v > 0) usleep(1000 * v); return 0;

		/* React to events */
		case TERM_XTRA_REACT: handle_extended_color_tables(); return 0;
	}

	/* Unknown event */
	return 1;
}


/**
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_gcu(int x, int y) {
	term_data *td = (term_data *)(Term->data);
	wmove(td->win, y, x);
	return 0;
}


/**
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_gcu(int x, int y, int n) {
	term_data *td = (term_data *)(Term->data);

	wmove(td->win, y, x);

	if (x + n >= td->t.wid) {
		/* Clear to end of line */
		wclrtoeol(td->win);
	} else {
		/* Clear some characters */
		if (can_use_color) {
			wattrset(td->win, colortable[COLOUR_DARK] | A_NORMAL);
		}
		whline(td->win, ' ', n);
		if (can_use_color) {
			wattrset(td->win, A_NORMAL);
		}
	}

	return 0;
}


/**
 * Place some text on the screen using an attribute
 */
static errr Term_text_gcu(int x, int y, int n, int a, const wchar_t *s) {
	term_data *td = (term_data *)(Term->data);

#ifdef A_COLOR
	if (can_use_color) {

		/* the lower 7 bits of the attribute indicate the fg/bg */
		int attr = a & 127;

		/* the high bit of the attribute indicates a reversed fg/bg */
		bool reversed = a > 127;

		int color;

		/* Set bg and fg to the same color when drawing solid walls */
		if (a / MAX_COLORS == BG_SAME) {
			color = same_colortable[attr];
		} else {
			color = colortable[attr];
		}

		/* the following check for A_BRIGHT is to avoid #1813 */
		int mode;
		if (reversed && (color & A_BRIGHT))
			mode = (color & ~A_BRIGHT) | A_BLINK | A_REVERSE;
		else if (reversed)
			mode = color | A_REVERSE;
		else
			mode = color | A_NORMAL;

		wattrset(td->win, mode);
		mvwaddnwstr(td->win, y, x, s, n);
		wattrset(td->win, A_NORMAL);
		return 0;
	}
#endif

	mvwaddnwstr(td->win, y, x, s, n);
	return 0;
}

/**
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
		quit("Failed to setup curses window.");

	/* Initialize the term */
	term_init(t, cols, rows, 256);

	/* Avoid bottom right corner */
	t->icky_corner = true;

	/* Differentiate between BS/^h, Tab/^i, etc. */
	t->complex_input = true;

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

/**
 * Simple helper
 */ 
static errr term_data_init(term_data *td)
{
	return term_data_init_gcu(td, td->r.cy, td->r.cx, td->r.y, td->r.x);
}


/* Parse 27,15,*x30 up to the 'x'. * gets converted to a big number
   Parse 32,* until the end. Return count of numbers parsed */
static int _parse_size_list(const char *arg, int sizes[], int max)
{
    int i = 0;
    const char *start = arg;
    const char *stop = arg;

    for (;;)
    {
        if (!*stop || !isdigit(*stop))
        {
            if (i >= max) break;
            if (*start == '*')
                sizes[i] = 255;
            else
            {
                /* rely on atoi("23,34,*") -> 23
                   otherwise, copy [start, stop) into a new buffer first.*/
                sizes[i] = atoi(start);
            }
            i++;
            if (!*stop || *stop != ',') break;

            stop++;
            start = stop;
        }
        else
            stop++;
    }
    return i;
}


static void hook_quit(const char *str) {
	int i;

	for (i = 0; i < term_count; i++) {
		if (angband_term[i]) {
			term_nuke(angband_term[i]);
		}
	}
	endwin();
}

/**
 * Prepare "curses" for use by the file "ui-term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
errr init_gcu(int argc, char **argv) {
	int i;

	/* Initialize info about terminal capabilities */
	termtype = getenv("TERM");
	loaded_terminfo = termtype && tgetent(0, termtype) == 1;

	/* Parse args */
	for (i = 1; i < argc; i++) {
		if (prefix(argv[i], "-B")) {
			bold_extended = true;
		} else if (prefix(argv[i], "-n")) {
			term_count = atoi(&argv[i][2]);
			if (term_count > MAX_TERM_DATA) term_count = MAX_TERM_DATA;
			else if (term_count < 1) term_count = 1;
		} else if (prefix(argv[i], "-D")) {
			use_default_background = true;
		} else if (streq(argv[i], "-K")) {
			keep_terminal_colors = true;
		}
	}

	/* Extract the normal keymap */
	keymap_norm_prepare();

	/* We do it like this to prevent a link error with curseses that
	 * lack ESCDELAY. */
	if (!getenv("ESCDELAY")) {
#if _POSIX_C_SOURCE < 200112L
		static char escdelbuf[80] = "ESCDELAY=20";
		putenv(escdelbuf);
#else
		setenv("ESCDELAY", "20", 1);
#endif
	}

	/* Initialize */
	if (initscr() == NULL) return (-1);

	/* Activate hooks */
	quit_aux = hook_quit;

	/* Require standard size screen */
	if (LINES < MIN_TERM0_LINES || COLS < MIN_TERM0_COLS) 
		quit("Angband needs at least an 80x24 'curses' screen");

#ifdef A_COLOR
	/* Do we have color, and enough color, available? */
	can_use_color = ((start_color() != ERR) && has_colors() &&
					 (COLORS >= 8) && (COLOR_PAIRS >= 8));
	if (!can_change_color()) keep_terminal_colors = true;

#ifdef HAVE_USE_DEFAULT_COLORS
	/* Should we use curses' "default color" */
	if (use_default_background && use_default_colors() == OK) bg_color = -1;
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

		/* These pairs are used for drawing solid walls */
		init_pair(PAIR_WHITE_WHITE, COLOR_WHITE, COLOR_WHITE);
		init_pair(PAIR_RED_RED, COLOR_RED, COLOR_RED);
		init_pair(PAIR_GREEN_GREEN, COLOR_GREEN, COLOR_GREEN);
		init_pair(PAIR_YELLOW_YELLOW, COLOR_YELLOW, COLOR_YELLOW);
		init_pair(PAIR_BLUE_BLUE, COLOR_BLUE, COLOR_BLUE);
		init_pair(PAIR_MAGENTA_MAGENTA, COLOR_MAGENTA, COLOR_MAGENTA);
		init_pair(PAIR_CYAN_CYAN, COLOR_CYAN, COLOR_CYAN);
		init_pair(PAIR_BLACK_BLACK, COLOR_BLACK, COLOR_BLACK);

		/* Prepare the colors */
		colortable[COLOUR_DARK]     = (COLOR_PAIR(PAIR_BLACK));
		colortable[COLOUR_WHITE]    = (COLOR_PAIR(PAIR_WHITE) | A_BRIGHT);
		colortable[COLOUR_SLATE]    = (COLOR_PAIR(PAIR_WHITE));
		colortable[COLOUR_ORANGE]   = (COLOR_PAIR(PAIR_YELLOW) | A_BRIGHT);
		colortable[COLOUR_RED]      = (COLOR_PAIR(PAIR_RED));
		colortable[COLOUR_GREEN]    = (COLOR_PAIR(PAIR_GREEN));
		colortable[COLOUR_BLUE]     = (COLOR_PAIR(PAIR_BLUE));
		colortable[COLOUR_UMBER]    = (COLOR_PAIR(PAIR_YELLOW));
		colortable[COLOUR_L_DARK]   = (COLOR_PAIR(PAIR_BLACK) | A_BRIGHT);
		colortable[COLOUR_L_WHITE]  = (COLOR_PAIR(PAIR_WHITE));
		colortable[COLOUR_L_PURPLE] = (COLOR_PAIR(PAIR_MAGENTA));
		colortable[COLOUR_YELLOW]   = (COLOR_PAIR(PAIR_YELLOW) | A_BRIGHT);
		colortable[COLOUR_L_RED]    = (COLOR_PAIR(PAIR_MAGENTA) | A_BRIGHT);
		colortable[COLOUR_L_GREEN]  = (COLOR_PAIR(PAIR_GREEN) | A_BRIGHT);
		colortable[COLOUR_L_BLUE]   = (COLOR_PAIR(PAIR_BLUE) | A_BRIGHT);
		colortable[COLOUR_L_UMBER]  = (COLOR_PAIR(PAIR_YELLOW));

		colortable[COLOUR_PURPLE]      = (COLOR_PAIR(PAIR_MAGENTA));
		colortable[COLOUR_VIOLET]      = (COLOR_PAIR(PAIR_MAGENTA));
		colortable[COLOUR_TEAL]        = (COLOR_PAIR(PAIR_CYAN));
		colortable[COLOUR_MUD]         = (COLOR_PAIR(PAIR_YELLOW));
		colortable[COLOUR_L_YELLOW]    = (COLOR_PAIR(PAIR_YELLOW | A_BRIGHT));
		colortable[COLOUR_MAGENTA]     = (COLOR_PAIR(PAIR_MAGENTA | A_BRIGHT));
		colortable[COLOUR_L_TEAL]      = (COLOR_PAIR(PAIR_CYAN) | A_BRIGHT);
		colortable[COLOUR_L_VIOLET]    = (COLOR_PAIR(PAIR_MAGENTA) | A_BRIGHT);
		colortable[COLOUR_L_PINK]      = (COLOR_PAIR(PAIR_MAGENTA) | A_BRIGHT);
		colortable[COLOUR_MUSTARD]     = (COLOR_PAIR(PAIR_YELLOW));
		colortable[COLOUR_BLUE_SLATE]  = (COLOR_PAIR(PAIR_BLUE));
		colortable[COLOUR_DEEP_L_BLUE] = (COLOR_PAIR(PAIR_BLUE));

		same_colortable[COLOUR_DARK]     = (COLOR_PAIR(PAIR_BLACK_BLACK));
		same_colortable[COLOUR_WHITE]    = (COLOR_PAIR(PAIR_WHITE_WHITE) | A_BRIGHT);
		same_colortable[COLOUR_SLATE]    = (COLOR_PAIR(PAIR_WHITE_WHITE));
		same_colortable[COLOUR_ORANGE]   = (COLOR_PAIR(PAIR_YELLOW_YELLOW) | A_BRIGHT);
		same_colortable[COLOUR_RED]      = (COLOR_PAIR(PAIR_RED_RED));
		same_colortable[COLOUR_GREEN]    = (COLOR_PAIR(PAIR_GREEN_GREEN));
		same_colortable[COLOUR_BLUE]     = (COLOR_PAIR(PAIR_BLUE_BLUE));
		same_colortable[COLOUR_UMBER]    = (COLOR_PAIR(PAIR_YELLOW_YELLOW));
		same_colortable[COLOUR_L_DARK]   = (COLOR_PAIR(PAIR_BLACK_BLACK) | A_BRIGHT);
		same_colortable[COLOUR_L_WHITE]  = (COLOR_PAIR(PAIR_WHITE_WHITE));
		same_colortable[COLOUR_L_PURPLE] = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA));
		same_colortable[COLOUR_YELLOW]   = (COLOR_PAIR(PAIR_YELLOW_YELLOW) | A_BRIGHT);
		same_colortable[COLOUR_L_RED]    = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA) | A_BRIGHT);
		same_colortable[COLOUR_L_GREEN]  = (COLOR_PAIR(PAIR_GREEN_GREEN) | A_BRIGHT);
		same_colortable[COLOUR_L_BLUE]   = (COLOR_PAIR(PAIR_BLUE_BLUE) | A_BRIGHT);
		same_colortable[COLOUR_L_UMBER]  = (COLOR_PAIR(PAIR_YELLOW_YELLOW));

		same_colortable[COLOUR_PURPLE]      = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA));
		same_colortable[COLOUR_VIOLET]      = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA));
		same_colortable[COLOUR_TEAL]        = (COLOR_PAIR(PAIR_CYAN_CYAN));
		same_colortable[COLOUR_MUD]         = (COLOR_PAIR(PAIR_YELLOW_YELLOW));
		same_colortable[COLOUR_L_YELLOW]    = (COLOR_PAIR(PAIR_YELLOW_YELLOW) | A_BRIGHT);
		same_colortable[COLOUR_MAGENTA]     = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA) | A_BRIGHT);
		same_colortable[COLOUR_L_TEAL]      = (COLOR_PAIR(PAIR_CYAN_CYAN) | A_BRIGHT);
		same_colortable[COLOUR_L_VIOLET]    = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA) | A_BRIGHT);
		same_colortable[COLOUR_L_PINK]      = (COLOR_PAIR(PAIR_MAGENTA_MAGENTA) | A_BRIGHT);
		same_colortable[COLOUR_MUSTARD]     = (COLOR_PAIR(PAIR_YELLOW));
		same_colortable[COLOUR_BLUE_SLATE]  = (COLOR_PAIR(PAIR_BLUE));
		same_colortable[COLOUR_DEEP_L_BLUE] = (COLOR_PAIR(PAIR_BLUE));
		handle_extended_color_tables();
	}
#endif

	/* Paranoia -- Assume no waiting */
	nodelay(stdscr, false);

	/* Prepare */
	cbreak();
	noecho();
	nonl();
	raw();

	/* Tell curses to rewrite escape sequences to KEY_UP and friends */
	keypad(stdscr, true);

	/* Extract the game keymap */
	keymap_game_prepare();

	/* Now prepare the term(s) */
	if (term_count > 1) 
	{	
		int rows, cols, y, x;
		int next_win = 0;
		for (i = 0; i < term_count; i++) {
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
	}
	else
/* Parse Args and Prepare the Terminals. Rectangles are specified
      as Width x Height, right? The game will allow you to have two
      strips of extra terminals, one on the right and one on the bottom.
      The map terminal will than fit in as big as possible in the remaining
      space.

      Examples:
        angband -mgcu -- -right 30x27,* -bottom *x7 will layout as

        Term-0: Map (COLS-30)x(LINES-7) | Term-1: 30x27
        --------------------------------|----------------------
        <----Term-3: (COLS-30)x7------->| Term-2: 30x(LINES-27)

        composband -mgcu -- -bottom *x7 -right 30x27,* will layout as

        Term-0: Map (COLS-30)x(LINES-7) | Term-2: 30x27
                                        |------------------------------
                                        | Term-3: 30x(LINES-27)
        ---------------------------------------------------------------
        <----------Term-1: (COLS)x7----------------------------------->

        Notice the effect on the bottom terminal by specifying its argument
        second or first. Notice the sequence numbers for the various terminals
        as you will have to blindly configure them in the window setup screen.

        EDIT: Added support for -left and -top.
    */
    {
        rect_t remaining = rect(0, 0, COLS, LINES);
        int    spacer_cx = 1;
        int    spacer_cy = 1;
        int    next_term = 1;
        int    term_ct = 1;

        for (i = 1; i < argc; i++)
        {
            if (streq(argv[i], "-spacer"))
            {
                i++;
                if (i >= argc)
                    quit("Missing size specifier for -spacer");
                sscanf(argv[i], "%dx%d", &spacer_cx, &spacer_cy);
            }
            else if (streq(argv[i], "-right") || streq(argv[i], "-left"))
            {
                const char *arg, *tmp;
                bool left = streq(argv[i], "-left");
                int  cx, cys[MAX_TERM_DATA] = {0}, ct, j, x, y;

                i++;
                if (i >= argc)
                    quit(format("Missing size specifier for -%s", left ? "left" : "right"));

                arg = argv[i];
                tmp = strchr(arg, 'x');
                if (!tmp)
                    quit(format("Expected something like -%s 60x27,* for two %s hand terminals of 60 columns, the first 27 lines and the second whatever is left.", left ? "left" : "right", left ? "left" : "right"));
                cx = atoi(arg);
                remaining.cx -= cx;
                if (left)
                {
                    x = remaining.x;
                    y = remaining.y;
                    remaining.x += cx;
                }
                else
                {
                    x = remaining.x + remaining.cx;
                    y = remaining.y;
                }
                remaining.cx -= spacer_cx;
                if (left)
                    remaining.x += spacer_cx;
                
                tmp++;
                ct = _parse_size_list(tmp, cys, MAX_TERM_DATA);
                for (j = 0; j < ct; j++)
                {
                    int cy = cys[j];
                    if (y + cy > remaining.y + remaining.cy)
                        cy = remaining.y + remaining.cy - y;
                    if (next_term >= MAX_TERM_DATA)
                        quit(format("Too many terminals. Only %d are allowed.", MAX_TERM_DATA));
                    if (cy <= 0)
                    {
                        quit(format("Out of bounds in -%s: %d is too large (%d rows max for this strip)", 
                            left ? "left" : "right", cys[j], remaining.cy));
                    }
                    data[next_term++].r = rect(x, y, cx, cy);
                    y += cy + spacer_cy;
                    term_ct++;
                }
            }
            else if (streq(argv[i], "-top") || streq(argv[i], "-bottom"))
            {
                const char *arg, *tmp;
                bool top = streq(argv[i], "-top");
                int  cy, cxs[MAX_TERM_DATA] = {0}, ct, j, x, y;

                i++;
                if (i >= argc)
                    quit(format("Missing size specifier for -%s", top ? "top" : "bottom"));

                arg = argv[i];
                tmp = strchr(arg, 'x');
                if (!tmp)
                    quit(format("Expected something like -%s *x7 for a single %s terminal of 7 lines using as many columns as are available.", top ? "top" : "bottom", top ? "top" : "bottom"));
                tmp++;
                cy = atoi(tmp);
                ct = _parse_size_list(arg, cxs, MAX_TERM_DATA);

                remaining.cy -= cy;
                if (top)
                {
                    x = remaining.x;
                    y = remaining.y;
                    remaining.y += cy;
                }
                else
                {
                    x = remaining.x;
                    y = remaining.y + remaining.cy;
                }
                remaining.cy -= spacer_cy;
                if (top)
                    remaining.y += spacer_cy;
                
                tmp++;
                for (j = 0; j < ct; j++)
                {
                    int cx = cxs[j];
                    if (x + cx > remaining.x + remaining.cx)
                        cx = remaining.x + remaining.cx - x;
                    if (next_term >= MAX_TERM_DATA)
                        quit(format("Too many terminals. Only %d are allowed.", MAX_TERM_DATA));
                    if (cx <= 0)
                    {
                        quit(format("Out of bounds in -%s: %d is too large (%d cols max for this strip)", 
                            top ? "top" : "bottom", cxs[j], remaining.cx));
                    }
                    data[next_term++].r = rect(x, y, cx, cy);
                    x += cx + spacer_cx;
                    term_ct++;
                }
            }
        }

        /* Map Terminal */
        if (remaining.cx < MIN_TERM0_COLS || remaining.cy < MIN_TERM0_LINES)
            quit(format("Failed: angband needs an %dx%d map screen, not %dx%d", MIN_TERM0_COLS, MIN_TERM0_LINES, remaining.cx, remaining.cy));
        data[0].r = remaining;
        term_data_init(&data[0]);
        angband_term[0] = Term;

        /* Child Terminals */
        for (next_term = 1; next_term < term_ct; next_term++)
        {
            term_data_init(&data[next_term]);
            angband_term[next_term] = Term;
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
