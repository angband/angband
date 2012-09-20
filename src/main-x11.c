/* File: main-x11.c */

/* Purpose: Support for X11 Angband */

/*
 * Copyright: Feel free to use this, I have no illusions of grandeur
 */

#include "angband.h"


#ifdef USE_X11


#include "xtra-x11.h"


#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif /* __MAKEDEPEND__ */


#ifndef IsModifierKey

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 */

#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#define IsCursorKey(keysym) \
  (((unsigned)(keysym) >= XK_Home)     && ((unsigned)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_F1)     && ((unsigned)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_F1)       && ((unsigned)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
  (((unsigned)(keysym) >= XK_Select)   && ((unsigned)(keysym) <  XK_KP_Space))

#define IsModifierKey(keysym) \
  (((unsigned)(keysym) >= XK_Shift_L)  && ((unsigned)(keysym) <= XK_Hyper_R))

#endif



/*
 * Hack -- cursor color
 */
static infoclr *xor;

/*
 * Color table
 */
static infoclr *clr[16];


/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * A structure for each "term"
 */
struct _term_data {

  term t;

  infofnt *fnt;

  infowin *outer;
  infowin *inner;
};


/*
 * The three term_data's
 */
static term_data screen;
static term_data recall;
static term_data choice;


/*
 * Process a keypress event
 */
static void react_keypress(XEvent *xev)
{
  int i, n, mc, ms, mo, mx;

  uint ks1;

  XKeyEvent *ev = (XKeyEvent*)(xev);

  KeySym ks;

  char buf[128];


  /* Check for "normal" keypresses */
  n = XLookupString(ev, buf, 125, &ks, NULL);

  /* Hack -- Ignore "modifier keys" */
  if (IsModifierKey(ks)) return;

  /* Normal keys */
  if (n)
  {
    buf[n] = '\0';
    for (i = 0; buf[i]; i++) Term_keypress(buf[i]);
    return;
  }


  /* Extract four "modifier flags" */
  mc = (ev->state & ControlMask) ? TRUE : FALSE;
  ms = (ev->state & ShiftMask) ? TRUE : FALSE;
  mo = (ev->state & Mod1Mask) ? TRUE : FALSE;
  mx = (ev->state & Mod2Mask) ? TRUE : FALSE;

  /* Hack -- convert into an unsigned int */
  ks1 = (uint)(ks);

  /* Hack -- Build a buffer */
  sprintf(buf, "%c%s%s%s%s%u%c", 31,
	      mc ? "C" : "", ms ? "S" : "",
	      mo ? "O" : "", mx ? "X" : "",
	      ks1, 13);

  /* Handle a few special KeySym codes */
  switch (ks1)
  {
    case XK_Return:
      strcpy(buf, "\n"); break;

    case XK_Tab:
      strcpy(buf, "\t"); break;

    case XK_BackSpace: 
      strcpy(buf, "\010"); break;

    case XK_Delete: 
      strcpy(buf, "\010"); break;

    case XK_Escape:
      sprintf(buf, "%c", ESCAPE); break;

    case XK_Up:
      sprintf(buf, "%c%d", 30, 8); break;

    case XK_Down:
      sprintf(buf, "%c%d", 30, 2); break;

    case XK_Left:
      sprintf(buf, "%c%d", 30, 4); break;

    case XK_Right:
      sprintf(buf, "%c%d", 30, 6); break;
  }

  /* Enqueue the "fake" string */
  for (i = 0; buf[i]; i++) Term_keypress(buf[i]);
}




/*
 * Process an event (or just check for one)
 */
static errr CheckEvent(bool check)
{
  term_data *old_td = (term_data*)(Term->data);

  XEvent xev_body, *xev = &xev_body;

  term_data *td = NULL;
  infowin *iwin = NULL;

  int flag = 0;

  int x, y, data;


  /* No events ready, and told to just check */
  if (check && !XPending(Metadpy->dpy)) return (1);

  /* Load the Event */
  XNextEvent(Metadpy->dpy, xev);


  /* Main screen, inner window */
  if (xev->xany.window == screen.inner->win)
  {
    td = &screen;
    iwin = td->inner;
  }

  /* Main screen, outer window */
  else if (xev->xany.window == screen.outer->win)
  {
    td = &screen;
    iwin = td->outer;
  }


  /* Recall window, inner window */
  else if (xev->xany.window == recall.inner->win)
  {
    td = &recall;
    iwin = td->inner;
  }

  /* Recall Window, outer window */
  else if (xev->xany.window == recall.outer->win)
  {
    td = &recall;
    iwin = td->outer;
  }


  /* Choice window, inner window */
  else if (xev->xany.window == choice.inner->win)
  {
    td = &choice;
    iwin = td->inner;
  }

  /* Choice Window, outer window */
  else if (xev->xany.window == choice.outer->win)
  {
    td = &choice;
    iwin = td->outer;
  }


  /* Unknown window */
  if (!td || !iwin) return (0);


  /* Hack -- activate the Term */
  Term_activate(&td->t);

  /* Hack -- activate the window */
  Infowin_set(iwin);


  /* Switch on the Type */
  switch (xev->type)
  {
    /* A Button Press Event */
    case ButtonPress:
    {
      /* Set flag, then fall through */
      flag = 1;
    }

    /* A Button Release (or ButtonPress) Event */
    case ButtonRelease:
    {
      /* Which button is involved */
      if      (xev->xbutton.button == Button1) data = 1;
      else if (xev->xbutton.button == Button2) data = 2;
      else if (xev->xbutton.button == Button3) data = 3;
      else if (xev->xbutton.button == Button4) data = 4;
      else if (xev->xbutton.button == Button5) data = 5;

      /* Where is the mouse */
      x = xev->xbutton.x;
      y = xev->xbutton.y;

      /* XXX Handle */

      break;
    }

    /* An Enter Event */
    case EnterNotify:
    {
      /* Note the Enter, Fall into 'Leave' */
      flag = 1;
    }

    /* A Leave (or Enter) Event */
    case LeaveNotify:
    {
      /* Where is the mouse */
      x = xev->xcrossing.x;
      y = xev->xcrossing.y;

      /* XXX Handle */

      break;
    }

    /* A Motion Event */
    case MotionNotify:
    {
      /* Where is the mouse */
      x = xev->xmotion.x;
      y = xev->xmotion.y;

      /* XXX Handle */

      break;
    }

    /* A KeyRelease */
    case KeyRelease:
    {
      /* Nothing */
      break;
    }

    /* A KeyPress */
    case KeyPress:
    {
      /* Save the mouse location */
      x = xev->xkey.x;
      y = xev->xkey.y;

      /* Hack -- use "old" term */
      Term_activate(&old_td->t);

      /* Process the key */
      react_keypress(xev);

      break;
    }

    /* An Expose Event */
    case Expose:
    {
      /* Clear the window */
      Infowin_wipe();

      /* Redraw if allowed */
      if (iwin == td->inner) Term_redraw();

      break;
    }

    /* A Mapping Event */
    case MapNotify:
    {
      Infowin->mapped = 1;
      break;
    }

    /* An UnMap Event */
    case UnmapNotify:
    {
      /* Save the mapped-ness */
      Infowin->mapped = 0;
      break;
    }

    /* A Move AND/OR Resize Event */
    case ConfigureNotify:
    {
      int x1, y1, w1, h1;
      int cols, rows, wid, hgt;

      /* Save the Old information */
      x1 = Infowin->x;
      y1 = Infowin->y;
      w1 = Infowin->w;
      h1 = Infowin->h;

      /* Save the new Window Parms */
      Infowin->x = xev->xconfigure.x;
      Infowin->y = xev->xconfigure.y;
      Infowin->w = xev->xconfigure.width;
      Infowin->h = xev->xconfigure.height;

      /* Detemine "proper" number of rows/cols */
      cols = ((Infowin->w - 2) / td->fnt->wid);
      rows = ((Infowin->h - 2) / td->fnt->hgt);

      /* Hack -- do not allow resize of main screen */
      if (td == &screen) cols = 80;
      if (td == &screen) rows = 24;

      /* Hack -- minimal size */
      if (cols < 1) cols = 1;
      if (rows < 1) rows = 1;

      /* Desired size of "outer" window */
      wid = cols * td->fnt->wid;
      hgt = rows * td->fnt->hgt;

      /* Resize the windows if any "change" is needed */
      if ((Infowin->w != wid + 2) || (Infowin->h != hgt + 2))
      {
        Infowin_set(td->outer);
	Infowin_resize(wid + 2, hgt + 2);
        Infowin_set(td->inner);
        Infowin_resize(wid, hgt);
      }

      break;
    }
  }


  /* Hack -- Activate the old term */
  Term_activate(&old_td->t);

  /* Hack -- Activate the proper "inner" window */
  Infowin_set(old_td->inner);


  /* XXX XXX Hack -- map/unmap as needed */


  /* Success */
  return (0);
}







/*
 * Handle "activation" of a term
 */
static errr Term_xtra_x11_level(int v)
{
  term_data *td = (term_data*)(Term->data);

  /* Only handle "activate" */
  if (v != TERM_LEVEL_SOFT_OPEN) return (1);

  /* Activate the "inner" window */
  Infowin_set(td->inner);

  /* Activate the "inner" font */
  Infofnt_set(td->fnt);

  /* Success */
  return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_x11(int n, int v)
{
    /* Handle a subset of the legal requests */
    switch (n)
    {
	/* Make a noise */
	case TERM_XTRA_NOISE: Metadpy_do_beep(); return (0);

	/* Flush the output */
	case TERM_XTRA_FLUSH: Metadpy_update(1,0,0); return (0);
	
	/* Check for a single event */
	case TERM_XTRA_CHECK: return (CheckEvent(TRUE));
	
	/* Wait for a single event */
	case TERM_XTRA_EVENT: return (CheckEvent(FALSE));
	
	/* Handle change in the "level" */
	case TERM_XTRA_LEVEL: return (Term_xtra_x11_level(v));
    }

    /* Unknown */
    return (1);
}



/*
 * Erase a number of characters
 */
static errr Term_wipe_x11(int x, int y, int w, int h)
{
  int k;

  /* Erase (use black) */
  Infoclr_set(clr[0]);

  /* Hack -- Erase each row */
  for (k = 0; k < h; ++k)
  {
    /* Mega-Hack -- Erase some space */
    Infofnt_text (x, y+k, "", w, TEXT_FILL | TEXT_GRID | TEXT_J_LT);
  }
  
  /* Success */
  return (0);
}



/*
 * Draw the cursor (XXX by hiliting)
 */
static errr Term_curs_x11(int x, int y, int z)
{
  /* Draw the cursor */
  Infoclr_set(xor);

  /* Hilite the cursor character */
  Infofnt_text(x, y, " ", 1, TEXT_GRID | TEXT_J_LT | TEXT_FILL);
  
  /* Success */
  return (0);
}


/*
 * Draw a number of characters
 */
static errr Term_text_x11(int x, int y, int n, byte a, cptr s)
{
  /* First, erase behind the chars */
  Term_wipe(x, y, n, 1);

  /* Draw the text in Xor */
  Infoclr_set(clr[a]);

  /* Draw the text, left justified, in a grid */
  Infofnt_text(x, y, s, n, TEXT_GRID | TEXT_J_LT);
  
  /* Success */
  return (0);
}



/*
 * Initialize a term_data
 */
static errr term_data_init(term_data *td, int w, int h, cptr name, cptr font)
{
  term *t = &td->t;

  int wid, hgt;

  /* Prepare the standard font */
  MAKE(td->fnt, infofnt);
  Infofnt_set(td->fnt);
  Infofnt_init_data(font);

  /* Extract the font sizes, add a border */
  wid = w * td->fnt->wid;
  hgt = h * td->fnt->hgt;

  /* Create a top-window (border 5) */
  MAKE(td->outer, infowin);
  Infowin_set(td->outer);
  Infowin_init_top(0, 0, wid + 2, hgt + 2, 1, Metadpy->fg, Metadpy->bg);
  Infowin_set_mask(StructureNotifyMask | KeyPressMask);
  Infowin_set_name(name);
  Infowin_map();

  /* Create a sub-window for playing field */
  MAKE(td->inner, infowin);
  Infowin_set(td->inner);
  Infowin_init_std(td->outer, 1, 1, wid, hgt, 0);
  Infowin_set_mask(ExposureMask);
  Infowin_map();

  /* Initialize the term (full size) */
  term_init(t, 80, 24, 64);

  /* Hooks */
  t->xtra_hook = Term_xtra_x11;
  t->curs_hook = Term_curs_x11;
  t->wipe_hook = Term_wipe_x11;
  t->text_hook = Term_text_x11;

  /* We are not a dumb terminal */
  t->soft_cursor = TRUE;
  t->scan_events = TRUE;

  /* Save the data */
  t->data = td;
    
  /* Activate (important) */
  Term_activate(t);
  
  /* Success */
  return (0);
}


/*
 * Names of the 16 colors
 *   Black, White, Slate, Orange,    Red, Green, Blue, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Green, L-Blue, L-Umber
 *
 * Colors courtesy of: Torbj|rn Lindgren <tl@ae.chalmers.se>
 *
 * These colors may no longer be valid...
 */
static cptr color_name[16] = {
      "black",        /* BLACK 15 */
      "white",        /* WHITE 0 */
      "#d7d7d7",      /* GRAY 13 */
      "#ff9200",      /* ORANGE 2 */
      "#ff0000",      /* RED 3 */
      "#00cd00",      /* GREEN 9 */
      "#0000fe",      /* BLUE 6 */
      "#c86400",      /* BROWN 10 */
      "#a3a3a3",      /* DARKGRAY 14 */
      "#ebebeb",      /* LIGHTGRAY 12 */
      "#a500ff",      /* PURPLE 5 */
      "#fffd00",      /* YELLOW 1 */
      "#ff00bc",      /* PINK 4 */
      "#00ff00",      /* LIGHTGREEN 8 */
      "#00c8ff",      /* LIGHTBLUE 7 */
      "#ffcc80",      /* LIGHTBROWN 11 */
};


/*
 * Initialization function for an "X11" module to Angband
 */
errr init_x11(void)
{
  int i;

  cptr fnt_name;

  cptr dpy_name = "";


  /* Init the Metadpy if possible */
  if (Metadpy_init_name(dpy_name)) return (-1);


  /* Prepare color "xor" (for cursor) */
  MAKE(xor, infoclr);
  Infoclr_set (xor);
  Infoclr_init_ccn ("fg", "bg", "xor", 0);

  /* Prepare the colors (including "black") */
  for (i = 0; i < 16; ++i)
  {
    cptr cname = color_name[0];
    MAKE(clr[i], infoclr);
    Infoclr_set (clr[i]);
    if (Metadpy->color) cname = color_name[i];
    else if (i) cname = color_name[1];
    Infoclr_init_ccn (cname, "bg", "cpy", 0);
  }


  /* Attempt to get a font name from the environment */
  fnt_name = getenv("ANGBAND_X11_FONT");

  /* No environment variable, use the default */
  if (!fnt_name) fnt_name = DEFAULT_X11_FONT;


  /* Initialize the recall window */
  term_data_init(&recall, 80, 6, "Recall", fnt_name);
  term_recall = Term;

  /* Initialize the choice window */
  term_data_init(&choice, 80, 22, "Choice", fnt_name);
  term_choice = Term;

  /* Initialize the screen */
  term_data_init(&screen, 80, 24, "Angband", fnt_name);
  term_screen = Term;


  /* Success */
  return (0);
}

#endif


