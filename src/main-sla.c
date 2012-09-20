/* File: main-sla.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file makes Angband work with the S-Lang library, available
 * from <URL:http://www.s-lang.org/>.  S-Lang works with OS/2,
 * MSDOS and Unix.  (And with VMS - if the rest of Angband does.)
 *
 *
 * Author: hans@grumbeer.pfalz.de (Hans-Joachim Baader)
 *
 * Most of this code is adapted directly from "main-gcu.c"
 */


#include "angband.h"


#ifdef USE_SLA

#include "main.h"


#include <slang.h>


/*
 * Are we "active"?
 */
static int slang_on = FALSE;


/*
 * Can we use "color"?
 */
static bool can_use_color = FALSE;


/*
 * Angband to S-Lang color conversion table
 */
static int colortable[16];


/*
 * Currently, only a single "term" is supported here
 */
static term term_screen_body;





/*
 * Hack -- see below
 */
static void init_pair(int index, char *foreground, char *background)
{
	SLtt_set_color(index, "", foreground, background);
}





#define A_NORMAL        0
#define A_BOLD          8
#define A_REVERSE       0
#define REVERSE         8

#define COLOR_BLACK     "black"
#define COLOR_BLUE      "blue"
#define COLOR_GREEN     "green"
#define COLOR_CYAN      "cyan"
#define COLOR_RED       "red"
#define COLOR_MAGENTA   "magenta"
#define COLOR_YELLOW    "brown"
#define COLOR_WHITE     "lightgray"
#define COLOR_BBLACK     "gray"
#define COLOR_BBLUE      "brightblue"
#define COLOR_BGREEN     "brightgreen"
#define COLOR_BCYAN      "brightcyan"
#define COLOR_BRED       "brightred"
#define COLOR_BMAGENTA   "brightmagenta"
#define COLOR_BYELLOW    "yellow"
#define COLOR_BWHITE     "white"





static char *color_terminals[] =
{
#ifdef linux
	"console",
#endif
	"linux",
	"xterm-color",
	"color-xterm",
	"xtermc",
	"ansi",
	0
};





/*
 * Stolen from the Midnight Commander
 */
int has_colors(void)
{
	int i;

	char *terminal;


	/* Access the terminal type */
	terminal = getenv("TERM");

	/* Check for colors */
	SLtt_Use_Ansi_Colors = 0;
	if (NULL != getenv ("COLORTERM"))
	{
		SLtt_Use_Ansi_Colors = 1;
	}

	/* We want to allow overriding */
	for (i = 0; color_terminals [i]; i++)
	{
		if (strcmp (color_terminals [i], terminal) == 0)
		{
			SLtt_Use_Ansi_Colors = 1;
		}
	}

	/* Setup emulated colors */
	if (SLtt_Use_Ansi_Colors)
	{
		/*init_pair (REVERSE, "black", "white");*/
	}

#if 0
	/*
	 * This sets up Angband to print characters reversed, but it does not
	 * reverse the background in places where not characters were printed.
	 */
	/* Setup bizarre colors */
	else
	{
		SLtt_set_mono(A_BOLD,    NULL, SLTT_BOLD_MASK);
		SLtt_set_mono(A_REVERSE, NULL, SLTT_REV_MASK);
		SLtt_set_mono(A_BOLD|A_REVERSE, NULL, SLTT_BOLD_MASK | SLTT_REV_MASK);
	}
#endif

	return SLtt_Use_Ansi_Colors;
}





/*
 * Nuke S-Lang
 */
static void Term_nuke_sla(term *t)
{
	/* Unused parameter */
	(void) t;

	if (!slang_on) return;

	/* Show the cursor */
	/* curs_set(1); */

	/* Clear the screen */
	(void)SLsmg_cls();

	/* Refresh */
	SLsmg_refresh();

	/* We are now off */
	slang_on = FALSE;

	/* Shut down */
	SLsmg_reset_smg();
	SLang_reset_tty();
}


/*
 * Init S-Lang
 */
static void Term_init_sla(term *t)
{
	/* Unused parameter */
	(void) t;

	/* Note that we are on */
	slang_on = TRUE;
}


/*
 * Process an event, wait if requested
 */
static errr Term_xtra_sla_event(int v)
{
	/* Do not wait unless requested */
	if (!v && (SLang_input_pending(0) == 0)) return (1);

	/* Get and enqueue the key */
	Term_keypress(SLang_getkey ());

	/* Success */
	return 0;
}



/*
 * Suspend / Resume
 */
static errr Term_xtra_sla_alive(int v)
{
	/* Suspend */
	if (!v)
	{
		/* Oops */
		if (!slang_on) return (1);

		/* We are now off */
		slang_on = FALSE;

		/* Block further signals, so double signals wont confuse us */
		SLsig_block_signals();

		/* Shut down (temporarily) */
		if (SLsmg_suspend_smg() < 0)
			quit("Could not save S-Lang state");
		SLang_reset_tty();

		/* Done with blocking */
		SLsig_unblock_signals();
	}

	/* Resume */
	else
	{
		/* Oops */
		if (slang_on) return (1);

		/* Block further signals, so double signals wont confuse us */
		SLsig_block_signals();

		/* Initialize, check for errors */
		if (SLang_init_tty(-1, TRUE, 0) == -1)
		    quit("S-Lang re-initialization failed");

#ifdef HANDLE_SIGNALS
		/* Allow keyboard generated suspend signal (on Unix, ^Z) */
		SLtty_set_suspend_state(TRUE);
#endif

		/* Restore the and screen and screen management state */
		if (SLsmg_resume_smg() == -1)
		{
		    SLang_reset_tty();
		    quit("Could not get back virtual display memory");
		}

		/* Done with blocking */
		SLsig_unblock_signals();

		/* Note that we are on */
		slang_on = TRUE;
	}

	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_sla(int n, int v)
{
	/* Analyze the request */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE:
		(void)SLsmg_write_char('\007');
		return (0);

		/* Flush the ncurses buffer */
		case TERM_XTRA_FRESH:
		(void)SLsmg_refresh();
		return (0);

		/* Make the cursor invisible or visible */
		case TERM_XTRA_SHAPE:
		/* curs_set(v); */
		return (0);

		/* Handle events */
		case TERM_XTRA_EVENT:
		return (Term_xtra_sla_event(v));

		/* Handle events */
		case TERM_XTRA_FLUSH:
		while (!Term_xtra_sla_event(FALSE));
		return (0);

		/* Suspend/Resume */
		case TERM_XTRA_ALIVE:
		return (Term_xtra_sla_alive(v));

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		(void)SLsmg_cls();
		SLsmg_gotorc(0, 0);
		return (0);

		/* Delay */
		case TERM_XTRA_DELAY:
		if (v > 0) usleep(1000 * v);
		return (0);
	}

	/* Oops */
	return (1);
}




/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_sla(int x, int y)
{
	/* Literally move the cursor */
	SLsmg_gotorc(y, x);

	/* Success */
	return 0;
}


/*
 * Erase some characters
 */
static errr Term_wipe_sla(int x, int y, int n)
{
	int i;

	/* Place the cursor */
	SLsmg_gotorc(y, x);

	/* Dump spaces */
	for (i = 0; i < n; i++) SLsmg_write_char(' ');

	/* Success */
	return 0;
}


/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_sla(int x, int y, int n, byte a, cptr s)
{
	/* Move the cursor */
	SLsmg_gotorc(y, x);

	/* Set the color */
	if (can_use_color) SLsmg_set_color(colortable[a&0x0F]);

	/* Dump the string */
	SLsmg_write_nchars((char *)s, n);

	/* Success */
	return 0;
}


const char help_sla[] = "S-Lang library, for terminal console";


/*
 * Prepare "S-Lang" for use by the file "z-term.c"
 * Installs the "hook" functions defined above
 */
errr init_sla(int argc, char **argv)
{
	int err;

	term *t = &term_screen_body;

	/* Unused parameters */
	(void)argc;
	(void)argv;

	/* Block signals, so signals cannot confuse the setup */
	SLsig_block_signals();

	/* Initialize, check for errors */
	err = (SLang_init_tty(-1, TRUE, 0) == -1);

	/* Quit on error */
	if (err) quit("S-Lang initialization failed");

	/* Get terminal info */
	SLtt_get_terminfo();

#ifdef HANDLE_SIGNALS
	/* Allow keyboard generated suspend signal (on Unix, ^Z) */
	SLtty_set_suspend_state(TRUE);

	/* Instead of signal(), use sigaction():SA_RESTART via SLsignal() */
	signal_aux = SLsignal;
#endif

	/* Initialize some more */
	if (SLsmg_init_smg() == -1)
	{
		SLang_reset_tty();
		quit("Could not get virtual display memory");
	}

	/* Check we have enough screen. */
	err = ((SLtt_Screen_Rows < 24) || (SLtt_Screen_Cols < 80));

	/* Quit with message */
	if (err)
	{
		SLsmg_reset_smg();
		SLang_reset_tty();
		quit("S-Lang screen must be at least 80x24");
	}

	/* Now let's go for a little bit of color! */
	err = !has_colors();

	/* Do we have color available? */
	can_use_color = !err;

	/* Init the Color-pairs and set up a translation table */
	/* If the terminal has enough colors */
	/* Color-pair 0 is *always* WHITE on BLACK */

	/* XXX XXX XXX See "main-gcu.c" for proper method */

	/* Only do this on color machines */
	if (can_use_color)
	{
		/* Prepare the color pairs */
		init_pair(1, COLOR_RED,     COLOR_BLACK);
		init_pair(2, COLOR_GREEN,   COLOR_BLACK);
		init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
		init_pair(4, COLOR_BLUE,    COLOR_BLACK);
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6, COLOR_CYAN,    COLOR_BLACK);
		init_pair(7, COLOR_BLACK,   COLOR_BLACK);
		init_pair(9, COLOR_BRED,    COLOR_BLACK);
		init_pair(10, COLOR_BGREEN,   COLOR_BLACK);
		init_pair(11, COLOR_BYELLOW,  COLOR_BLACK);
		init_pair(12, COLOR_BBLUE,    COLOR_BLACK);
		init_pair(13, COLOR_BMAGENTA, COLOR_BLACK);
		init_pair(14, COLOR_BCYAN,    COLOR_BLACK);
		init_pair(15, COLOR_BBLACK,   COLOR_BLACK);

		/* Prepare the color table */
		colortable[0] = 7;       /* Black */
		colortable[1] = 0;       /* White */
		colortable[2] = 6;       /* Grey XXX */
		colortable[3] = 11;      /* Orange XXX */
		colortable[4] = 1;       /* Red */
		colortable[5] = 2;       /* Green */
		colortable[6] = 4;       /* Blue */
		colortable[7] = 3;       /* Brown */
		colortable[8] = 15;      /* Dark-grey XXX */
		colortable[9] = 14;      /* Light-grey XXX */
		colortable[10] = 5;       /* Purple */
		colortable[11] = 11;      /* Yellow */
		colortable[12] = 9;       /* Light Red */
		colortable[13] = 10;      /* Light Green */
		colortable[14] = 12;      /* Light Blue */
		colortable[15] = 3;       /* Light Brown XXX */
	}

	/* Done with blocking */
	SLsig_unblock_signals();

	/* Initialize the term */
	term_init(t, 80, 24, 64);

	/* Stick in some hooks */
	t->nuke_hook = Term_nuke_sla;
	t->init_hook = Term_init_sla;

	/* Stick in some more hooks */
	t->xtra_hook = Term_xtra_sla;
	t->curs_hook = Term_curs_sla;
	t->wipe_hook = Term_wipe_sla;
	t->text_hook = Term_text_sla;

	/* Save the term */
	term_screen = t;

	/* Activate it */
	Term_activate(t);


	/* Success */
	return 0;
}

#endif /* USE_SLA */
