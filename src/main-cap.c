/* File: main-cap.c */

/* Purpose: Support for "term.c" using "termcap" calls */

#include "angband.h"


#ifdef USE_CAP


/*
 * This file is a total hack, but is often very helpful.  :-)
 *
 * This file allows use of the terminal without requiring the
 * "curses" routines.  In fact, if "USE_HARDCODE" is defined,
 * this file will attempt to use various hard-coded "vt100"
 * escape sequences to also avoid the use of the "termcap"
 * routines.  I do not know if this will work on System V.
 *
 * This file is intended for use only on those machines which are
 * unable, for whatever reason, to compile the "main-gcu.c" file,
 * but which seem to be able to support the "termcap" library, or
 * which at least seem able to support "vt100" terminals.
 *
 * Large portions of this file were stolen from "main-gcu.c"
 *
 * This file incorrectly handles output to column 80, I think.
 */


/*
 * Require a "system"
 */
#if !defined(USE_TERMCAP) && !defined(USE_HARDCODE)
# define USE_TERMCAP
#endif

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
 * POSIX stuff
 */
#ifdef USE_TPOSIX
# include <sys/ioctl.h>
# include <termios.h>
#endif

/*
 * One version needs these files
 */
#ifdef USE_TERMIO
# include <sys/ioctl.h>
# include <termio.h>
#endif

/*
 * The other needs these files
 */
#ifdef USE_TCHARS
# include <sys/ioctl.h>
# include <sys/resource.h>
# include <sys/param.h>
# include <sys/file.h>
# include <sys/types.h>
#endif


/*
 * XXX XXX Hack -- POSIX uses "O_NONBLOCK" instead of "O_NDELAY"
 *
 * They should both work due to the "(i != 1)" test in the code
 * which checks for the result of the "read()" command.
 */
#ifndef O_NDELAY
# define O_NDELAY O_NONBLOCK
#endif




#ifdef USE_TERMCAP

/*
 * Termcap string information
 */

static char blob[1024];		/* The "termcap" entry */
static char area[1024];		/* The string extraction buffer */
static char *next = area;	/* The current "index" into "area" */
static char *desc;		/* The terminal name */

#endif


/*
 * Pointers into the "area"
 */

static char *cm;	/* Move cursor */
static char *ch;	/* Move cursor to horizontal location */
static char *cv;	/* Move cursor to vertical location */
static char *ho;	/* Move cursor to top left */
static char *ll;	/* Move cursor to bottom left */
static char *cs;	/* Set scroll area */
static char *cl;	/* Clear screen */
static char *cd;	/* Clear to end of display */
static char *ce;	/* Clear to end of line */
static char *cr;	/* Move to start of line */
static char *so;	/* Turn on standout */
static char *se;	/* Turn off standout */
static char *md;	/* Turn on bold */
static char *me;	/* Turn off bold */
static char *vi;	/* Cursor - invisible */
static char *ve;	/* Cursor - normal */
static char *vs;	/* Cursor - bright */


/*
 * State variables
 */

static int rows;	/* Screen size (Y) */
static int cols;	/* Screen size (X) */
static int curx;	/* Cursor location (X) */
static int cury;	/* Cursor location (Y) */
static int curv;	/* Cursor visibility */


/*
 * Extern functions
 */
extern char *getenv();
extern char *tgoto();
extern char *tgetstr();


/*
 * Write some chars to the terminal
 */
static void ewrite(char *str)
{
	int numtowrite, numwritten;

	/* See how much work we have */
	numtowrite = strlen(str);

	/* Write until done */
	while (numtowrite > 0)
	{
		/* Try to write the chars */
		numwritten = write(1, str, numtowrite);

		/* Handle FIFOs and EINTR */
		if (numwritten < 0) numwritten = 0;

		/* See what we completed */
		numtowrite -= numwritten;
		str += numwritten;

		/* Hack -- sleep if not done */
		if (numtowrite > 0) sleep(1);
	}
}



#ifdef USE_TERMCAP

static char write_buffer[128];
static char *write_buffer_ptr;

static void output_one(char c)
{
	*write_buffer_ptr++ = c;
}

static void tp(char *s)
{
	/* Dump the string into us */
	write_buffer_ptr = write_buffer;

	/* Write the string with padding */
	tputs (s, 1, output_one);

	/* Finish the string */
	*write_buffer_ptr = '\0';

	/* Dump the recorded buffer */
	ewrite (write_buffer);
}

#endif

#ifdef USE_HARDCODE

static void tp(char *s)
{
	ewrite(s);
}

#endif







/*
 * Clear the screen
 */
static void do_cl(void)
{
	if (cl) tp (cl);
}

/*
 * Clear to the end of the line
 */
static void do_ce(void)
{
	if (ce) tp(ce);
}


/*
 * Set the cursor visibility (0 = invis, 1 = normal, 2 = bright)
 */
static void curs_set(int vis)
{
	char *v = NULL;

	if (!vis)
	{
		v = vi;
	}
	else if (vis > 1)
	{
		v = vs ? vs : ve;
	}
	else
	{
		v = ve ? ve : vs;
	}

	if (v) tp(v);
}



/*
 * Restrict scrolling to within these rows
 */
static void do_cs(int y1, int y2)
{

#ifdef USE_TERMCAP
	if (cs) tp(tgoto(cs, y2, y1));
#endif

#ifdef USE_HARDCODE
	char temp[64];
	sprintf(temp, cs, y1, y2);
	tp (temp);
#endif

}



/*
 * Go to the given screen location directly
 */
static void do_cm(int x, int y)
{

#ifdef USE_TERMCAP
	if (cm) tp(tgoto(cm, x, y));
#endif

#ifdef USE_HARDCODE
	char temp[64];
	sprintf(temp, cm, y+1, x+1);
	tp(temp);
#endif

}


/*
 * Go to the given screen location in a "clever" manner
 *
 * XXX XXX XXX This function could use some work!
 */
static void do_move(int x1, int y1, int x2, int y2)
{
	/* Hack -- unknown start location */
	if ((x1 == x2) && (y1 == y2)) do_cm(x2, y2);

	/* Left edge */
	else if (x2 == 0)
	{
		if ((y2 <= 0) && ho) tp(ho);
		else if ((y2 >= rows-1) && ll) tp(ll);
		else if ((y2 == y1) && cr) tp(cr);
#if 0
		else if ((y2 == y1+1) && cr && dn)
		{ tp(cr); tp(dn); }
		else if ((y2 == y1-1) && cr && up)
		{ tp(cr); tp(up); }
#endif
		else do_cm(x2, y2);
	}

#if 0
	/* Up/Down one line */
	else if ((x2 == x1) && (y2 == y1+1) && dn) tp(dn);
	else if ((x2 == x1) && (y2 == y1-1) && up) tp(up);
#endif

	/* Default -- go directly there */
	else do_cm(x2, y2);
}




/*
 * Help initialize this file (see below)
 */
errr init_cap_aux(void)
{

#ifdef USE_TERMCAP

	/* Get the terminal name (if possible) */
	desc = getenv("TERM");
	if (!desc) return (1);

	/* Get the terminal info */
	if (tgetent(blob, desc) != 1) return (2);

	/* Get the (initial) columns and rows, or default */
	if ((cols = tgetnum("co")) == -1) cols = 80;
	if ((rows = tgetnum("li")) == -1) rows = 24;

	/* Find out how to move the cursor to a given location */
	cm = tgetstr("cm", &next);
	if (!cm) return (10);

	/* Find out how to move the cursor to a given position */
	ch = tgetstr("ch", &next);
	cv = tgetstr("cv", &next);

	/* Find out how to "home" the screen */
	ho = tgetstr("ho", &next);

	/* Find out how to "last-line" the screen */
	ll = tgetstr("ll", &next);

	/* Find out how to do a "carriage return" */
	cr = tgetstr("cr", &next);
	if (!cr) cr = "\r";

	/* Find out how to clear the screen */
	cl = tgetstr("cl", &next);
	if (!cl) return (11);

	/* Find out how to clear to the end of display */
	cd = tgetstr("cd", &next);

	/* Find out how to clear to the end of the line */
	ce = tgetstr("ce", &next);

	/* Find out how to scroll (set the scroll region) */
	cs = tgetstr("cs", &next);

	/* Find out how to hilite */
	so = tgetstr("so", &next);
	se = tgetstr("se", &next);
	if (!so || !se) so = se = NULL;

	/* Find out how to bold */
	md = tgetstr("md", &next);
	me = tgetstr("me", &next);
	if (!md || !me) md = me = NULL;

	/* Check the cursor visibility stuff */
	vi = tgetstr("vi", &next);
	vs = tgetstr("vs", &next);
	ve = tgetstr("ve", &next);

#endif

#ifdef USE_HARDCODE

	/* Assume some defualt information */
	rows = 24;
	cols = 80;

	/* Clear screen */
	cl = "\033[2J\033[H";	/* --]--]-- */

	/* Clear to end of line */
	ce = "\033[K";	/* --]-- */

	/* Hilite on/off */
	so = "\033[7m";	/* --]-- */
	se = "\033[m";	/* --]-- */

	/* Scroll region */
	cs = "\033[%d;%dr";	/* --]-- */

	/* Move cursor */
	cm = "\033[%d;%dH";	/* --]-- */

#endif

	/* Success */
	return (0);
}







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

static struct sgttyb  norm_ttyb;
static struct tchars  norm_tchars;
static struct ltchars norm_ltchars;
static int            norm_local_chars;

static struct sgttyb  game_ttyb;
static struct tchars  game_tchars;
static struct ltchars game_ltchars;
static int            game_local_chars;

#endif



/*
 * Are we active?  Not really needed.
 */
static int active = FALSE;


/*
 * The main screen (no sub-screens)
 */
static term term_screen_body;



/*
 * Place the "keymap" into its "normal" state
 */
static void keymap_norm(void)
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
	(void)ioctl(0, TIOCSETP, (char *)&norm_ttyb);
	(void)ioctl(0, TIOCSETC, (char *)&norm_tchars);
	(void)ioctl(0, TIOCSLTC, (char *)&norm_ltchars);
	(void)ioctl(0, TIOCLSET, (char *)&norm_local_chars);

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

#ifdef USE_TERMIO

	/* restore the saved values of the special chars */
	(void)ioctl(0, TCSETA, (char *)&game_termio);

#endif

#ifdef USE_TCHARS

	/* restore the saved values of the special chars */
	(void)ioctl(0, TIOCSETP, (char *)&game_ttyb);
	(void)ioctl(0, TIOCSETC, (char *)&game_tchars);
	(void)ioctl(0, TIOCSLTC, (char *)&game_ltchars);
	(void)ioctl(0, TIOCLSET, (char *)&game_local_chars);

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

#ifdef USE_TERMIO

	/* Get the normal keymap */
	(void)ioctl(0, TCGETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

	/* Get the normal keymap */
	(void)ioctl(0, TIOCGETP, (char *)&norm_ttyb);
	(void)ioctl(0, TIOCGETC, (char *)&norm_tchars);
	(void)ioctl(0, TIOCGLTC, (char *)&norm_ltchars);
	(void)ioctl(0, TIOCLGET, (char *)&norm_local_chars);

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

	/* Hack -- Turn off "echo" and "canonical" mode */
	game_termios.c_lflag &= ~(ECHO | ICANON);

#endif

#ifdef USE_TERMIO

	/* Acquire the current mapping */
	(void)ioctl(0, TCGETA, (char *)&game_termio);

	/* Force "Ctrl-C" to interupt */
	game_termio.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
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

	/* Hack -- Turn off "echo" and "canonical" mode */
	game_termio.c_lflag &= ~(ECHO | ICANON);

#endif

#ifdef USE_TCHARS

	/* Get the default game characters */
	(void)ioctl(0, TIOCGETP, (char *)&game_ttyb);
	(void)ioctl(0, TIOCGETC, (char *)&game_tchars);
	(void)ioctl(0, TIOCGLTC, (char *)&game_ltchars);
	(void)ioctl(0, TIOCLGET, (char *)&game_local_chars);

	/* Force interupt (^C) */
	game_tchars.t_intrc = (char)3;

	/* Force start/stop (^Q, ^S) */
	game_tchars.t_startc = (char)17;
	game_tchars.t_stopc = (char)19;

	/* Cancel some things */
	game_tchars.t_quitc = (char)-1;
	game_tchars.t_eofc = (char)-1;
	game_tchars.t_brkc = (char)-1;

	/* Force suspend (^Z) */
	game_ltchars.t_suspc = (char)26;

	/* Cancel some things */
	game_ltchars.t_dsuspc = (char)-1;
	game_ltchars.t_rprntc = (char)-1;
	game_ltchars.t_flushc = (char)-1;
	game_ltchars.t_werasc = (char)-1;
	game_ltchars.t_lnextc = (char)-1;

	/* XXX XXX XXX XXX Verify this before use */
	/* Hack -- Turn off "echo" and "canonical" mode */
	/* game_termios.c_lflag &= ~(ECHO | ICANON); */
	game_ttyb.flag &= ~(ECHO | ICANON);

#endif

}








/*
 * Suspend/Resume
 */
static errr Term_xtra_cap_alive(int v)
{
	/* Suspend */
	if (!v)
	{
		if (!active) return (1);

		/* Hack -- make sure the cursor is visible */
		curs_set(1);

		/* Move to bottom right */
		do_move(0, rows - 1, 0, rows - 1);

		/* Go to normal keymap mode */
		keymap_norm();

		/* No longer active */
		active = FALSE;
	}

	/* Resume */
	else
	{
		if (active) return (1);

		/* Hack -- restore the cursor location */
		do_move(curx, cury, curx, cury);

		/* Hack -- restore the cursor visibility */
		curs_set(curv);

		/* Go to angband keymap mode */
		keymap_game();

		/* Now we are active */
		active = TRUE;
	}

	/* Success */
	return (0);
}



/*
 * Process an event
 */
static errr Term_xtra_cap_event(int v)
{
	int i, arg;
	char buf[2];

	/* Wait */
	if (v)
	{
		/* Wait for one byte */
		i = read(0, buf, 1);

		/* Hack -- Handle "errors" */
		if ((i <= 0) && (errno != EINTR)) exit_game_panic();
	}

	/* Do not wait */
	else
	{
		/* Get the current flags for stdin */
		if ((arg = fcntl(0, F_GETFL, 0)) < 1) return (1);

		/* Tell stdin not to block */
		if (fcntl(0, F_SETFL, arg | O_NDELAY) < 0) return (1);

		/* Read one byte, if possible */
		i = read(0, buf, 1);

		/* Replace the flags for stdin */
		if (fcntl(0, F_SETFL, arg)) return (1);
	}

	/* No keys ready */
	if ((i != 1) || (!buf[0])) return (1);

	/* Enqueue the keypress */
	Term_keypress(buf[0]);

	/* Success */
	return (0);
}




/*
 * Actually move the hardware cursor
 */
static errr Term_curs_cap(int x, int y)
{
	/* Literally move the cursor */
	do_move(curx, cury, x, y);

	/* Save the cursor location */
	curx = x;
	cury = y;

	/* Success */
	return (0);
}


/*
 * Erase a grid of space
 *
 * XXX XXX XXX Note that we will never be asked to clear the
 * bottom line all the way to the bottom right edge, since we
 * have set the "avoid the bottom right corner" flag.
 */
static errr Term_wipe_cap(int x, int y, int n)
{
	int dx;

	/* Place the cursor */
	Term_curs_cap(x, y);

	/* Wipe to end of line */
	if (x + n >= 80)
	{
		do_ce();
	}

	/* Wipe region */
	else
	{
		for (dx = 0; dx < n; ++dx)
		{
			putc(' ', stdout);
			curx++;
		}
	}

	/* Success */
	return (0);
}


/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_cap(int x, int y, int n, byte a, cptr s)
{
	int i;

	/* Move the cursor */
	Term_curs_cap(x, y);

	/* Dump the text, advance the cursor */
	for (i = 0; s[i]; i++)
	{
		/* Dump the char */
		putc(s[i], stdout);

		/* Advance cursor 'X', and wrap */
		if (++curx >= cols)
		{
			/* Reset cursor 'X' */
			curx = 0;

			/* Hack -- Advance cursor 'Y', and wrap */
			if (++cury == rows) cury = 0;
		}
	}

	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_cap(int n, int v)
{
	/* Analyze the request */
	switch (n)
	{
		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		do_cl();
		do_move(0, 0, 0, 0);
		return (0);

		/* Make a noise */
		case TERM_XTRA_NOISE:
		(void)write(1, "\007", 1);
		return (0);

		/* Change the cursor visibility */
		case TERM_XTRA_SHAPE:
		curv = v;
		curs_set(v);
		return (0);

		/* Suspend/Resume */
		case TERM_XTRA_ALIVE:
		return (Term_xtra_cap_alive(v));

		/* Process events */
		case TERM_XTRA_EVENT:
		return (Term_xtra_cap_event(v));

		/* Flush events */
		case TERM_XTRA_FLUSH:
		while (!Term_xtra_cap_event(FALSE));
		return (0);

		/* Delay */
		case TERM_XTRA_DELAY:
		usleep(1000 * v);
		return (0);
	}

	/* Not parsed */
	return (1);
}




/*
 * Init a "term" for this file
 */
static void Term_init_cap(term *t)
{
	if (active) return;

	/* Assume cursor at top left */
	curx = 0;
	cury = 0;

	/* Assume visible cursor */
	curv = 1;

	/* Clear the screen */
	do_cl();

	/* Hack -- visible cursor */
	curs_set(1);

	/* Assume active */
	active = TRUE;
}


/*
 * Nuke a "term" for this file
 */
static void Term_nuke_cap(term *t)
{
	if (!active) return;

	/* Hack -- make sure the cursor is visible */
	curs_set(1);

	/* Move to bottom right */
	do_move(0, rows - 1, 0, rows - 1);

	/* Normal keymap */
	keymap_norm();

	/* No longer active */
	active = FALSE;
}










/*
 * Prepare this file for Angband usage
 */
errr init_cap(void)
{
	term *t = &term_screen_body;


	/*** Initialize ***/

	/* Initialize the screen */
	if (init_cap_aux()) return (-1);

	/* Hack -- Require large screen, or Quit with message */
	if ((rows < 24) || (cols < 80)) quit("Screen too small!");


	/*** Prepare to play ***/

	/* Extract the normal keymap */
	keymap_norm_prepare();

	/* Extract the game keymap */
	keymap_game_prepare();

	/* Hack -- activate the game keymap */
	keymap_game();

	/* Hack -- Do NOT buffer stdout */
	setbuf(stdout, NULL);


	/*** Now prepare the term ***/

	/* Initialize the term */
	term_init(t, 80, 24, 256);

	/* Avoid the bottom right corner */
	t->icky_corner = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Set some hooks */
	t->init_hook = Term_init_cap;
	t->nuke_hook = Term_nuke_cap;

	/* Set some more hooks */
	t->text_hook = Term_text_cap;
	t->wipe_hook = Term_wipe_cap;
	t->curs_hook = Term_curs_cap;
	t->xtra_hook = Term_xtra_cap;

	/* Save the term */
	term_screen = t;

	/* Activate it */
	Term_activate(term_screen);

	/* Success */
	return (0);
}


#endif /* USE_CAP */


