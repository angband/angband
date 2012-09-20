/* File: main-cap.c */

/* Purpose: Support for "term.c" using "termcap" calls */

#ifdef USE_CAP

#include "angband.h"


/*
 * This file allows use of the terminal without requiring the
 * "curses" routines.  In fact, if "USE_HARDCODE" is defined,
 * this file will attempt to use various hard-coded "vt100"
 * escape sequences to also avoid the use of the "termcap"
 * routines.  I do not know if this will work on System V.
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
static void term_cl(void)
{
  if (cl) tp (cl);
}

/*
 * Clear to the end of the line
 */
static void term_ce(void)
{
  if (ce) tp(ce);
}


/*
 * Set the cursor visibility
 */
static void curs_set(int vis)
{
  char *v = NULL;

  if (!vis) v = vi;
  else if (vis == 2) v = vs ? vs : ve;
  else v = ve ? ve : vs;

  if (v) tp(v);
}



static void hilite_on(void)
{
  if (so) tp(so);
  else if (md) tp(md);
}

static void hilite_off(void)
{
  if (se) tp(se);
  else if (me) tp(me);
}


/*
 * Restrict scrolling to within these rows
 */
static void term_cs(int y1, int y2)
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
static void term_cm(int x, int y)
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
static void term_move(int x1, int y1, int x2, int y2)
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





/*
 * XXX XXX XXX XXX
 * See "main-gcu.c" for methods to save/restore and activate various
 * key-bindings, and to read a keypress with or without blocking.
 */


/*
 * Remember where the cursor is
 */
static int cx = 0, cy = 0;


/*
 * Actually move the hardware cursor
 */
static void Term_curs(int x, int y, int z)
{
    /* Literally move the cursor */
    term_move(cx, cy, x, y);

    /* Save the cursor */
    cx = x; cy = y;
}


/*
 * Erase a grid of space
 */
static void Term_wipe(int x, int y, int w, int h)
{
    int dx, dy;

    if (!x && !y && (w >= 80) && (h >= 24))
    {
        term_cl();
    }

    else if (w >= 80)
    {
        for (dy = 0; dy < h; ++dy)
        {
            Term_curs(x, y+dy);
            Term_ce();
        }
    }

    else
    {
        for (dy = 0; dy < h; ++dy)
        {
            Term_curs(x, y+dy);
            for (dx = 0; dx < w; ++dx)
            {
                putch(' ');
                cx++;
            }
        }
    }

    /* Hack -- Fix the cursor */
    Term_curs(x, y);
}


/*
 * Place some text on the screen using an attribute
 */
static void Term_text(int x, int y, int n, byte a, cptr s)
{
    int i;

    /* Not too far */
    if (x + n > 80) n = 80 - x;

    /* Move the cursor */
    Term_curs(x, y);

    /* Dump the text */
    for (i = 0; (i < n) && s[i]; i++)
    {
        putch(s[i]);
        cx++;
    }
}


/*
 * Handle a "special request"
 */
static void Term_xtra(int n)
{
    /* Analyze the request */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE: (void)write(1, "\007", 1); break;

        /* Make the cursor invisible */
        case TERM_XTRA_INVIS: curs_set(0); break;

        /* Make the cursor visible */
        case TERM_XTRA_BEVIS: curs_set(1); break;
    }
}



#endif





/*
 * Nuke the term
 */
static errr term_nuke()
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
  rows = 24;
  cols = 80;

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


/*
 * Initialize the Visual System
 */
errr init_cap(void)
{
    /* Oops */
    return (1);
}


#endif


