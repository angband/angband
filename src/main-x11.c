/* File: main-x11.c */

/* Purpose: Support for X11 Angband */

/*
 * Copyright: Feel free to use this, I have no illusions of grandeur
 */

#include "angband.h"


#ifdef USE_X11


#include "z-util.h"

#include "x-metadpy.h"

#include "x-infoclr.h"
#include "x-infofnt.h"
#include "x-infowin.h"

#include "r-infoclr.h"
#include "r-infofnt.h"
#include "r-infowin.h"

infoclr *clr[16];

infoclr *xor;

infofnt *fnt;


#define WINDOW_ROWS 24
#define WINDOW_COLS 80
#define WINDOW_EDGE 1

infowin *window_0;		/* Frame for "main window" */
infowin *window_1;		/* Actual "main window" */



#ifdef GRAPHIC_RECALL

#define RECALL_ROWS 24
#define RECALL_COLS 80
#define RECALL_EDGE 1

infowin *recall_0;		/* Frame for "recall window" */
infowin *recall_1;		/* Actual "recall window" */

#endif

#ifdef GRAPHIC_CHOICE
infowin *choice_0;		/* Frame for "choice window" */
infowin *choice_1;		/* Actual "choice window" */
#endif



#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif /* __MAKEDEPEND__ */



/*
 * Parse an XEvent of type 'Key' into an ascii code
 * Note that 'Shift' is parsed as an 'error'
 *
 * Inputs:
 *	code: Place to store the ascii value
 *	xev:  Pointer to an XEvent
 */
static errr ascii_from_xevent(int *code, XEvent *xev)
{
  XKeyEvent *ev = (XKeyEvent*)(xev);

  char buf[2];
  KeySym ks;

  int i;
  unsigned int ks1;


  /* They hit a bizarre key like shift, extract arrow keys */
  i = XLookupString (ev, buf, 2, &ks, NULL);

  /* Turn into an Int cause 'KeySym's are no good in 'switch' */
  ks1 = (int)(ks);

  /* If i is weird, complain */
  if (i != 0 && i != 1)
  {
    plog_fmt("Weird length (%d) returned by XLookupString", i);
    return (-11);
  }

  /* If no "string" found, must be a weird key */
  if (i == 0) 
  {
    /* Hack -- Arrow keys into numerical directions */
    switch (ks1)
    {
      case XK_Up:    { *code = '8'; return (0); }
      case XK_Down:  { *code = '2'; return (0); }
      case XK_Left:  { *code = '4'; return (0); } 
      case XK_Right: { *code = '6'; return (0); }
    }

    /* I can't decifer it */
    return (-12);
  }

  /* Switch on a few bizarre KeySym codes */
  switch (ks1)
  {
    case XK_Return:    { *code = CURSOR_RET; return (0); }
    case XK_BackSpace: { *code = CURSOR_BS;  return (0); }
    case XK_Delete:    { *code = CURSOR_BS;  return (0); }
    case XK_Tab:       { *code = CURSOR_TAB; return (0); }
    case XK_Escape:    { *code = CURSOR_ESC; return (0); }
  }

  /* Try Normal Ascii Buffer Values */
  if ((buf[0] > 0) && (buf[0] < 127))
  {
    *code = ((int)(buf[0]));
    return (0);
  }

  /* Cannot decifer it */
  return (-1);
}


/*
 * Helper function for CheckEvents()
 */
static void react (XEvent *xev)
{
  int flag = 0;

  int x, y, data;

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
      break;
    }

    /* A Motion Event */
    case MotionNotify:
    {
      /* Where is the mouse */
      x = xev->xmotion.x;
      y = xev->xmotion.y;
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
      /* Convert the Data into Ascii Form, or assume unknown */
      if (ascii_from_xevent (&data, xev)) data = -128;

      /* Only accept values that will fit inside a 'char' */
      if ((data < -127) || (data > 127)) data = -128;

      /* Save the mouse location */
      x = xev->xkey.x;
      y = xev->xkey.y;

      /* Enqueue the keypress, if useful */
      if (data > 0) Term_keypress(data);

      break;
    }

    /* An Expose Event */
    case Expose:
    {
      /* Erase and redraw */
      Infowin_wipe();
      Term_redraw();
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
      int wid, hgt;

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

      /* Process "window" */
      if (Infowin == window_0)
      {
        /* Desired inner size */
        wid = WINDOW_COLS * fnt->wid + 2 * WINDOW_EDGE;
        hgt = WINDOW_ROWS * fnt->hgt + 2 * WINDOW_EDGE;

        /* Reset the size */
        if ((Infowin->w != wid) || (Infowin->h != hgt))
        {
          /* Hack -- cancel window resizes */
          Infowin_resize(wid,hgt);
        }
      }
      break;

    }
  }
}



/*
 * Check for events
 */
static void Term_scan_x11(int n)
{
  XEvent xev;

  /* Scan for events */
  while (1)
  {
    /* If we were told to handle 'n' events, see if we have */
    if ((n >= 0) && (n-- == 0)) return;

    /* If we were told to Scan until done, see if we are done */
    if ((n < 0) && !XPending(Metadpy->dpy)) return;

    /* Load an Event (block if needed) */
    XNextEvent(Metadpy->dpy, &xev);

    /* Look up the window */
    if (window_0->win == xev.xany.window)
    {
      Infowin_set(window_0);
      react (&xev);
      Infowin_set(window_1);
    }

    /* Look up the window */
    if (window_1->win == xev.xany.window)
    {
      react (&xev);
    }
  }
}


/*
 * Handle a "special request"
 */
static void Term_xtra_x11(int n)
{
    /* Handle a subset of the legal requests */
    switch (n)
    {
	/* Make a noise */
	case TERM_XTRA_NOISE: Metadpy_do_beep(); break;

	/* Flush the output */
	case TERM_XTRA_FLUSH: Metadpy_update(1,0,0); break;
    }
}



#if 0

void dump(void)
{
  int x, y, len;
  char *str;

  /* Set the Infowin and Infofnt */
  Infowin_set (txt->w->iwin);
  Infofnt_set (txt->fnt);

  /* See how many chars would fit in this window */
  w = Infowin->w / Infofnt->wid;
  h = Infowin->h / Infofnt->hgt;

  /* Save a clean, safe version of these values */
  txt->wid = (w < 1) ? 1 : w;
  txt->hgt = (h < 1) ? 1 : h;

  /* XXX Be sure ind is on the screen */

  /* XXX Now perhaps auto-resize to that exact size */

  x = 0; y = 5;

  /* Get the str and len */
  str = "Hello There My Name is Ben";
  len = strlen(str);

  /* Figure out how many chars we should display */
  if (x + len > 80) len = 80 - x;
}
#endif



/*
 * Erase a number of characters
 */
static void Term_wipe_x11(int x, int y, int w, int h)
{
  int k;

  /* Erase (use black) */
  Infoclr_set (clr[0]);

  /* Erase each row */
  for (k = 0; k < h; ++k)
  {
    /* Mega-Hack -- Erase some space */
    Infofnt_text (x, y+k, "", w, TEXT_FILL | TEXT_GRID | TEXT_J_LT);
  }
}



/*
 * Draw the cursor (XXX by hiliting)
 */
static void Term_curs_x11(int x, int y, int z)
{
  /* Draw the cursor */
  Infoclr_set(xor);

  /* Hilite the cursor character */
  Infofnt_text(x, y, " ", 1, TEXT_GRID | TEXT_J_LT | TEXT_FILL);
}


/*
 * Draw a number of characters
 */
static void Term_text_x11(int x, int y, int n, byte a, cptr s)
{
  /* Hack -- Check stuff */
  if (a == COLOR_BLACK) a = COLOR_RED;

  /* First, erase behind the chars */
  Term_wipe(x, y, n, 1);

  /* Draw the text in Xor */
  Infoclr_set(clr[a]);

  /* Draw the text, left justified, in a grid */
  Infofnt_text(x, y, s, n, TEXT_GRID | TEXT_J_LT);
}



/*
 * Names of the 16 colors
 *   Black, White, Slate, Orange,    Red, Green, Blue, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Green, L-Blue, L-Umber
 *
 * On the machine I was testing on, these colors are all too dark.
 */
static cptr color_name[16] = {
	"black",	/* BLACK 15 */
	"white",	/* WHITE 0 */
	"#808080",	/* GRAY 13 */
	"#FF6402",	/* ORANGE 2 */
	"#E00806",	/* RED 3 */
	"#006210",	/* GREEN 9 */
	"#0000D0",	/* BLUE 6 */
	"#562C06",	/* BROWN 10 */
	"#404040",	/* DARKGRAY 14 */
	"#C0C0C0",	/* LIGHTGRAY 12 */
	"#4600A3",	/* PURPLE 5 */
	"#F8F004",	/* YELLOW 1 */
	"#F20884",	/* PINK 4 */
	"#20B814",	/* LIGHTGREEN 8 */
	"#02A0E8",	/* LIGHTBLUE 7 */
	"#907039",	/* LIGHTBROWN 11 */
};



/*
 * Initialization function for an "X11" module to Angband
 */
errr init_x11(cptr dpy_name, cptr fnt_name)
{
  int wid, hgt;

  int i;

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


  /* Prepare the font */
  MAKE(fnt, infofnt);
  Infofnt_set (fnt);
  Infofnt_init_data (fnt_name);


  /* Extract the font sizes, add a border */
  wid = WINDOW_COLS * fnt->wid + 2 * WINDOW_EDGE;
  hgt = WINDOW_ROWS * fnt->hgt + 2 * WINDOW_EDGE;

  /* Create a top-window (border 5) */
  MAKE(window_0, infowin);
  Infowin_set (window_0);
  Infowin_init_top (0, 0, wid, hgt, 5, Metadpy->fg, Metadpy->bg);
  Infowin_set_mask (StructureNotifyMask | KeyPressMask);
  Infowin_set_name("Angband");
  Infowin_map();

  /* Extract the font sizes, no border */
  wid = WINDOW_COLS * fnt->wid;
  hgt = WINDOW_ROWS * fnt->hgt;

  /* Create a sub-window for playing field */
  MAKE(window_1, infowin);
  Infowin_set (window_1);
  Infowin_init_std (window_0, WINDOW_EDGE, WINDOW_EDGE, wid, hgt, 0);
  Infowin_set_mask (ExposureMask);
  Infowin_map();



#ifdef GRAPHIC_RECALL

  /* Extract the font sizes, add a border */
  wid = RECALL_COLS * fnt->wid + 2 * RECALL_EDGE;
  hgt = RECALL_ROWS * fnt->hgt + 2 * RECALL_EDGE;

  /* Create a top-window (border 5) */
  MAKE(recall_0, infowin);
  Infowin_set (recall_0);
  Infowin_init_top (0, 0, wid, hgt, 5, Metadpy->fg, Metadpy->bg);
  Infowin_set_mask (StructureNotifyMask | KeyPressMask);
  Infowin_set_name("Angband");
  Infowin_map();

  /* Extract the font sizes, no border */
  wid = RECALL_COLS * fnt->wid;
  hgt = RECALL_ROWS * fnt->hgt;

  /* Create a sub-window for playing field */
  MAKE(recall_1, infowin);
  Infowin_set (recall_1);
  Infowin_init_std (recall_0, RECALL_EDGE, RECALL_EDGE, wid, hgt, 0);
  Infowin_set_mask (ExposureMask);
  Infowin_map();

#endif


  /* Add in the hooks */
  Term_text_hook = Term_text_x11;
  Term_wipe_hook = Term_wipe_x11;
  Term_curs_hook = Term_curs_x11;
  Term_scan_hook = Term_scan_x11;
  Term_xtra_hook = Term_xtra_x11;

  /* We support the latest technology... */
  Term_method(TERM_SOFT_CURSOR, TRUE);
  Term_method(TERM_SCAN_EVENTS, TRUE);

  /* Success */
  return (0);
}

#endif


