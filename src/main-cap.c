/* File: main-cap.c */

/* Purpose: Termcap support for "term.c" */

#include "h-include.h"

#define DEFAULT_ROWS 24
#define DEFAULT_COLS 80


/*
 * This file allows use of the terminal without requiring the
 * "curses" routines.  In fact, if "USE_HARDCODE" is defined,
 * this file will attempt to use various hard-coded "vt100"
 * escape sequences to also avoid the use of the "termcap"
 * routines.  I do not know if this will work on System V.
 *
 * It was originally designed as an alternative to using "main-cur.c"
 * with the game "Angband", so that the "curses" library is not needed.
 *
 * This file will not function on MSDOS, ATARI, or VMS machines.
 */



/* Termcap strings */

/* The "termcap" entry */
static char blob[1024];

/* The "buffer" for extracting strings */
static char area[1024];

/* The current "index" into "area" */
static char *next = area;

/* The terminal name */
static char *term;

/* Pointers into the "area" */
static char *cm;             /* Move cursor */
static char *ch;             /* Move cursor to horizontal location */
static char *cv;             /* Move cursor to vertical location */
static char *ho;             /* Move cursor to top left */
static char *ll;             /* Move cursor to bottom left */
static char *cs;             /* Set scroll area */
static char *cl;             /* Clear screen */
static char *cd;             /* Clear to end of display */
static char *ce;             /* Clear to end of line */
static char *cr;             /* Move to start of line */
static char *so;             /* Turn on standout */
static char *se;             /* Turn off standout */
static char *md;             /* Turn on bold */
static char *me;             /* Turn off bold */
static char *vi;             /* Cursor - invisible */
static char *ve;             /* Cursor - normal */
static char *vs;             /* Cursor - bright */


/* Others */

int rows, cols;              /* Default screen size */


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
void term_cl(void)
{
  if (cl) tp (cl);
}

/*
 * Clear to the end of the line
 */
void term_ce(void)
{
  if (ce) tp(ce);
}


/*
 * Set the cursor visibility
 */
void curs_set(int vis)
{
  char *v = NULL;

  if (!vis) v = vi;
  else if (vis == 2) v = vs ? vs : ve;
  else v = ve ? ve : vs;

  if (v) tp(v);
}



void hilite_on(void)
{
  if (so) tp(so);
  else if (md) tp(md);
}

void hilite_off(void)
{
  if (se) tp(se);
  else if (me) tp(me);
}


/*
 * Restrict scrolling to within these rows
 */
void term_cs(int y1, int y2)
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
void term_cm(int x, int y)
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
 */
void term_move(int x1, int y1, int x2, int y2)
{
  /* Hack -- unknown start location */
  if ((x1 == x2) && (y1 == y2)) term_cm(x2,y2);

  /* Left edge */
  else if (x2 == 0)
  {
    if ((y2 <= 0) && ho) tp(ho);
    else if ((y2 >= rows-1) && ll) tp(ll);
    else if ((y2 == y1) && cr) tp(cr);
#if 0
    else if ((y2 == y1+1) && cr && dn) { tp(cr); tp(dn); }
    else if ((y2 == y1-1) && cr && up) { tp(cr); tp(up); }
#endif
    else term_cm(x2,y2);
  }

#if 0
  /* Up/Down one line */
  else if ((x2 == x1) && (y2 == y1+1) && dn) tp(dn);
  else if ((x2 == x1) && (y2 == y1-1) && up) tp(up);
#endif

  /* Default -- go directly there */
  else term_cm(x2,y2);
}





#if 0

/*** Some special stuff ***/


/*
 * Variables to hold the terminal state
 */
#ifdef USG
static struct termio	save_termio;
#else
static struct ltchars	save_special_chars;
static struct sgttyb	save_ttyb;
static struct tchars	save_tchars;
static int		save_local_chars;
#endif


/*
 * Set the terminal values
 */
void Term_terminal_set()
{

#ifdef USG
    (void)ioctl(0, TCSETA, (char *)&save_termio);
#else
    (void)ioctl(0, TIOCSLTC, (char *)&save_special_chars);
    (void)ioctl(0, TIOCSETP, (char *)&save_ttyb);
    (void)ioctl(0, TIOCSETC, (char *)&save_tchars);
    (void)ioctl(0, TIOCLSET, (char *)&save_local_chars);
#endif

}

/*
 * Get the terminal values
 */
void Term_terminal_get()
{

#ifdef USG
    (void)ioctl(0, TCGETA, (char *)&save_termio);
#else
    (void)ioctl(0, TIOCGLTC, (char *)&save_special_chars);
    (void)ioctl(0, TIOCGETP, (char *)&save_ttyb);
    (void)ioctl(0, TIOCGETC, (char *)&save_tchars);
    (void)ioctl(0, TIOCLGET, (char *)&save_local_chars);
#endif

}


/*
 * Prepare the terminal for a standard "game" environment (see term.c).
 *
 * disable all of the special characters except the suspend char, interrupt
 * char, and the control flow start/stop characters 
 */
void Term_terminal_raw()
{

#ifdef USG
    struct termio  tbuf;
#else
    struct ltchars lbuf;
    struct tchars  buf;
#endif


    /* XXX Enter "cbreak" mode */
#ifndef BSD4_3
    crmode();
#else
    cbreak();
#endif

    /* XXX Perhaps enter "do not translate newline" mode */

#ifdef USG

    /* disable all of the normal special control characters */
    (void)ioctl(0, TCGETA, (char *)&tbuf);
    tbuf.c_cc[VINTR] = (char)3;	   /* control-C */
    tbuf.c_cc[VQUIT] = (char)-1;
    tbuf.c_cc[VERASE] = (char)-1;
    tbuf.c_cc[VKILL] = (char)-1;
    tbuf.c_cc[VEOF] = (char)-1;
    tbuf.c_cc[VEOL] = (char)-1;
    tbuf.c_cc[VEOL2] = (char)-1;
    tbuf.c_cc[VMIN] = 1;	   /* Input should wait for at least 1 char */
    tbuf.c_cc[VTIME] = 0;	   /* no matter how long that takes. */
    (void)ioctl(0, TCSETA, (char *)&tbuf);

#else

    (void)ioctl(0, TIOCGLTC, (char *)&lbuf);
    lbuf.t_suspc = (char)26;	   /* control-Z */
    lbuf.t_dsuspc = (char)-1;
    lbuf.t_rprntc = (char)-1;
    lbuf.t_flushc = (char)-1;
    lbuf.t_werasc = (char)-1;
    lbuf.t_lnextc = (char)-1;
    (void)ioctl(0, TIOCSLTC, (char *)&lbuf);

    (void)ioctl(0, TIOCGETC, (char *)&buf);
    buf.t_intrc = (char)3;	   /* control-C */
    buf.t_quitc = (char)-1;
    buf.t_startc = (char)17;	   /* control-Q */
    buf.t_stopc = (char)19;	   /* control-S */
    buf.t_eofc = (char)-1;
    buf.t_brkc = (char)-1;
    (void)ioctl(0, TIOCSETC, (char *)&buf);

#endif

}


/*
 * Hack -- Suspend Curses
 * See "signals.h" for usage
 */
void unix_suspend_curses(int go)
{

#ifdef USG

/*
 * for USG systems with BSDisms that have SIGTSTP defined,
 * but don't actually implement it.  XXX What?
 */

#else

    static struct sgttyb  tbuf;
    static struct ltchars lcbuf;
    static struct tchars  cbuf;
    static int            lbuf;

    /* Step One */
    if (go)
    {
	(void)ioctl(0, TIOCGETP, (char *)&tbuf);
	(void)ioctl(0, TIOCGETC, (char *)&cbuf);
	(void)ioctl(0, TIOCGLTC, (char *)&lcbuf);
	(void)ioctl(0, TIOCLGET, (char *)&lbuf);

	unix_restore_curses();
    }

    /* Step 2 */
    else
    {
	(void)ioctl(0, TIOCSETP, (char *)&tbuf);
	(void)ioctl(0, TIOCSETC, (char *)&cbuf);
	(void)ioctl(0, TIOCSLTC, (char *)&lcbuf);
	(void)ioctl(0, TIOCLSET, (char *)&lbuf);

	(void)touchwin(curscr);
	(void)wrefresh(curscr);

	/* XXX Enter cbreak() mode */
	cbreak();
    }

#endif

}





/*
 * For Non GRAPHIC_WINDOW systems, actually MOVE the hardware cursor
 */
void Term_curs(int x, int y, int z)
{
    /* Literally move the cursor */
    move(y,x);
}

/*
 * Erase a grid of space
 */
void Term_wipe(int x, int y, int w, int h)
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
}


/*
 * Place some text on the screen using an attribute
 */
void Term_text(int x, int y, int n, byte a, cptr s)
{
    int i;
    char buf[81];
    if (n > 80) n = 80;
    for (i = 0; i < n && s[i]; ++i) buf[i] = s[i];
    buf[n]=0;
    move(y, x);
    addstr(buf);
}


/*
 * Provides for a timeout on input. Does a non-blocking read, consuming the
 * data if any, and then returns 1 if data was read, zero otherwise. 
 *
 * Porting: 
 *
 * In systems without the select call, but with a sleep for fractional numbers
 * of seconds, one could sleep for the time and then check for input. 
 *
 * In systems which can only sleep for whole number of seconds, you might sleep
 * by writing a lot of nulls to the terminal, and waiting for them to drain,
 * or you might hack a static accumulation of times to wait. When the
 * accumulation reaches a certain point, sleep for a second. There would need
 * to be a way of resetting the count, with a call made for commands like run
 * or rest.
 *
 * Currently, miscrosec is zero, but we could use this in some kbhit()'s.
 */
static int check_input(int microsec)
{
    int result;

#if defined(USG) && !defined(M_XENIX)
    int                 arg;
#else
    struct timeval      tbuf;
#if defined(BSD4_3) || defined(M_XENIX) || defined(linux)
    fd_set              smask;
    fd_set		*no_fds = NULL;
#else
    int                 smask;
    int			*no_fds = NULL;
#endif
#endif


#if defined(USG) && !defined(M_XENIX)

    /*** SysV code (?) ***/

    /* Hack -- mod 128, sleep one sec every 128 turns */
    if (microsec != 0 && (turn & 0x7F) == 0) (void)sleep(1);

    /* Can't check for input, but can do non-blocking read, so...  Ugh! */
    arg = 0;
    arg = fcntl(0, F_GETFL, arg);
    arg |= O_NDELAY;
    (void)fcntl(0, F_SETFL, arg);

    result = getchar();

    arg = 0;
    arg = fcntl(0, F_GETFL, arg);
    arg &= ~O_NDELAY;
    (void)fcntl(0, F_SETFL, arg);

    if (result == EOF) return 0;

#else

    /*** Do a nice clean "select" ***/

    tbuf.tv_sec = 0;
    tbuf.tv_usec = microsec;

#if defined(BSD4_3) || defined(M_XENIX) || defined(linux)
    FD_ZERO(&smask);
    FD_SET(1, &smask);
#else
    smask = 1;
#endif

    /* If we time out, no key ready */
    if (select(1, &smask, no_fds, no_fds, &tbuf) != 1) return (0);

    /* Get a key */
    result = getchar();

    /* See "EOF" handling below */
    if (result == EOF)
    {
	eof_flag++;
	return 0;
    }

#endif

    /* There is a key ready, return it */
    return (result);
}



/*
 * Scan for events.  If "n" is "-1", scan until done.
 * Otherwise, block until "n" events have been handled.
 * XXX Need to choose a behavior if "n" is zero...
 */
void Term_scan(int n)
{
    int i;

    /* Scan events until done */
    if (n < 0) {

	/* Enqueue keys while available */
	while ((i = check_input(0))) Term_keypress(i);

	/* All done */
	return;
    }	


    /* Scan events until 'n' are processed */
    while (1) {

	/* Decrease counter, return when done */
	if (n-- == 0) return;

	/* Get a keypress */
	i = getchar();

	/* Broken input is special */
	if (i == EOF) break;

	/* Enqueue the keypress */
	Term_keypress(i);
    }


    /*** Handle death of standard input ***/

    /* Count the interupts */
    eof_flag++;

    /* avoid infinite loops while trying to call */
    /* inkey() for a -more- prompt.  */
    msg_flag = FALSE;

    /* Just exit if nothing important is here */
    if (!character_generated || character_saved) exit_game();

    /* Yeah, I'd say that's pretty disturbing */
    disturb(1, 0);

    /* just in case, to make sure that the process eventually dies */
    if (eof_flag > 100)
    {
	panic_save = 1;
	(void)strcpy(died_from, "(end of input: panic saved)");
	if (save_player()) main_exit();
	(void)strcpy(died_from, "panic: unexpected eof");
	death = TRUE;
	exit_game();
    }

    /* Hack -- pretend "Escape" was pressed */
    Term_keypress(ESCAPE);
}


/*
 * Handle a "special request"
 */
void Term_xtra(int n)
{
    /* Analyze the request */
    switch (n)
    {
	/* Make a noise */
	case TERM_XTRA_NOISE: (void)write(1, "\007", 1); break;

	/* Flush the Curses buffer */
	case TERM_XTRA_FLUSH: (void)refresh(); break;

#ifdef SYS_V

	/* XXX Make the cursor invisible */
	case TERM_XTRA_INVIS: curs_set(0); break;

	/* XXX Make the cursor visible */
	case TERM_XTRA_BEVIS: curs_set(1); break;

#endif

    }
}



#endif





/*
 * Nuke the term
 */
errr term_nuke()
{
  term_cm(0,23);
  printf("\n");

  /* Hack -- flush */
  (void)fflush(stdout);
}


#ifdef USE_TERMCAP

/*
 * Init the term
 */
errr term_init ()
{
  /* Get the terminal name (if possible) */
  term = getenv("TERM");
  if (!term) return (1);

  /* Get the terminal info */
  if (tgetent(blob, term) != 1) return (2);

  /* Get the (initial) columns and rows, or default */
  if ((cols = tgetnum("co")) == -1) cols = DEFAULT_COLS;
  if ((rows = tgetnum("li")) == -1) rows = DEFAULT_ROWS;

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

  /* Clear the screen */
  if (cl) tp(cl);

  /* Success */
  return (0);
}

#endif

#ifdef USE_HARDCODE

/*
 * Note that all the strings default to NULL
 */
int term_init ()
{
  /* Assume some defualt information */
  rows = DEFAULT_LINES;
  cols = DEFAULT_COLUMNS;

  /* Clear screen */
  cl = "\033[2J\033[H";

  /* Clear to end of line */
  ce = "\033[K";

  /* Hilite on/off */
  so = "\033[7m";
  se = "\033[m";

  /* Scroll region */
  cs = "\033[%d;%dr";

  /* Move cursor */
  cm = "\033[%d;%dH";

  /* Do NOT buffer stdout */
  setbuf(stdout, NULL);

  /* Clear the screen */
  if (cl) tp(cl);

  /* Success */
  return (0);
}

#endif




/*
 * This could be used for an "attribute" parser
 */
void hack(int x, int y, int a, cptr s)
{
  term_xy(x,y);
  if (a) hilite_on();
  fputs(s,stdout);
  if (a) hilite_off();
}


/*
 * Test it
 */
int main(int argc, char *argv[])
{
  int i, x, y;

  /* Init the term */
  if (term_init()) exit(-1);

  /* Test it */
  term_cm(10,5);
  printf("A");
  sleep(5);
  hack(39,23,0,"\n");
  sleep(5);
  curs_set(1);

  /* Nuke the term */
  if (term_nuke()) exit(-1);
}


