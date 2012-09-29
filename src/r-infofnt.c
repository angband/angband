/* File: r-infofnt.c */

#include "r-infofnt.h"

#include "z-util.h"

#include "x-metadpy.h"

#include "x-infoclr.h"
#include "x-infofnt.h"
#include "x-infowin.h"




/*
 * Standard Text
 */

errr Infofnt_text_std (int x, int y, cptr str, int len, uint mode)
{
  int i;
  int dir, asc, desc;
  XCharStruct ov;


  /*** Do a brief info analysis ***/

  /* Do nothing if the string is null */
  if (!str || !*str) return (-1);

  /* Get the length of the string */
  if (len < 0) len = strlen (str);


  /*** Decide where to place the string ***/

  /* Both TEXT_GRID and TEXT_QUERY makes no sense */
  if ((mode & TEXT_QUERY) && (mode & TEXT_GRID))
  {
    /* Not very graceful */
    core("Infofnt: Impossible Request: QUERY+ GRID\n");
  }


  /* Assume TEXT_QUERY over-rides TEXT_GRID */
  else if (mode & TEXT_QUERY)
  {
    /*** Query the Info - Why? ***/

    /* Get the "size" of the string */
    XTextExtents (Infofnt->info, str, len, &dir, &asc, &desc, &ov);



    /*** Decide where to place the string, vertically ***/

    /* Use y as the baseline location */
    if ((mode & TEXT_J_UP) && (mode & TEXT_J_DN))
    {
      /* Use the standard Baseline */
    }

    /* Use y as the top location */
    else if (mode & TEXT_J_UP)
    {
      y = y + ov.ascent;
    }

    /* Use y as the bottom location */
    else if (mode & TEXT_J_DN)
    {
      y = y - ov.descent;
    }

    /* Center vertically around y */
    else
    {
      y = y + (ov.ascent - ov.descent) / 2;
    }


    /*** Decide where to place the string, horizontally ***/

    /* Same as Centering below */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = x - (ov.width / 2);
    }

    /* Line up with x at left edge (XXX cleanly?) */
    else if (mode & TEXT_J_LT)
    {
      /* x = x + ov.lbearing; */
    }

    /* Line up with x at right edge (XXX cleanly?) */
    else if (mode & TEXT_J_RT)
    {
      x = x - (ov.width);
    }

    /* Default -- Center horizontally (XXX cleanly?) */
    else
    {
      x = x - (ov.width / 2);
    }
  }


  /* Use (row,col) positional info */
  else if (mode & TEXT_GRID)
  {
    /*** Decide where to place the string, vertically ***/

    /* Ignore Vertical Justifications */
    y = (y * Infofnt->hgt) + Infofnt->asc;


    /*** Decide where to place the string, horizontally ***/

    /* Center, allow half-grid column leeway */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = (x * Infofnt->wid) - (len * Infofnt->wid / 2);
    }

    /* Line up with x at left edge of column 'x' */
    else if (mode & TEXT_J_LT)
    {
      x = (x * Infofnt->wid);
    }

    /* Line up with x at right edge of column 'x' */
    else if (mode & TEXT_J_RT)
    {
      x = (x - (len - 1)) * Infofnt->wid;
    }

    /* Center horizontally, snap to grid columns */
    else
    {
      x = (x - (len / 2)) * Infofnt->wid;
    }
  }


  /* Neither TEXT_GRID nor TEXT_QUERY, this is very common */
  else
  {
    /*** Decide where to place the string, vertically ***/

    /* Use y as the baseline location */
    if ((mode & TEXT_J_UP) && (mode & TEXT_J_DN))
    {
      /* Use standard base line */
    }

    /* Use y as the top location */
    else if (mode & TEXT_J_UP)
    {
      y = y + Infofnt->asc;
    }

    /* Use y as the bottom location */
    else if (mode & TEXT_J_DN)
    {
      y = y + Infofnt->asc - Infofnt->hgt;
    }

    /* Center vertically around y */
    else
    {
      y = y + Infofnt->asc - (Infofnt->hgt / 2);
    }


    /*** Decide where to place the string, horizontally ***/

    /* Same as standard centering */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = x - ((len * Infofnt->wid) / 2);
    }

    /* Line up with x at left edge (XXX cleanly?) */
    else if (mode & TEXT_J_LT)
    {
      /* Just use (messy) left edge */
    }

    /* Line up with x at right edge (XXX cleanly?) */
    else if (mode & TEXT_J_RT)
    {
      x = x - (len * Infofnt->wid);
    }

    /* Center horizontally (XXX cleanly?) */
    else
    {
      x = x - ((len * Infofnt->wid) / 2);
    }
  }


  /*** Actually draw 'str' onto the infowin ***/

  /* Be sure the correct font is ready */
  XSetFont (Metadpy->dpy, Infoclr->gc, Infofnt->info->fid);


  /*** Handle the fake mono we can enforce on fonts ***/

  /* Monotize the font */
  if (Infofnt->mono)
  {
    /* Do each character */
    for (i = 0; i < len; ++i)
    {
      /* Perhaps draw it and clear behind it */
      if (mode & TEXT_WIPE)
      {
	/* Note that the Infoclr is set up to contain the Infofnt */
	XDrawImageString (Metadpy->dpy, Infowin->win, Infoclr->gc,
			  x + i * Infofnt->wid + Infofnt->off, y, str + i, 1);
      }

      /* Else draw it without clearing behind it */
      else
      {
	/* Note that the Infoclr is set up to contain the Infofnt */
	XDrawString (Metadpy->dpy, Infowin->win, Infoclr->gc,
		     x + i * Infofnt->wid + Infofnt->off, y, str + i, 1);
      }
    }
  }

  /* Assume monoospaced font */
  else
  {
    /* Perhaps draw it and clear behind it */
    if (mode & TEXT_WIPE)
    {
      /* Note that the Infoclr is set up to contain the Infofnt */
      XDrawImageString (Metadpy->dpy, Infowin->win, Infoclr->gc,
			x, y, str, len);
    }

    /* Else draw it without clearing behind it */
    else
    {
      /* Note that the Infoclr is set up to contain the Infofnt */
      XDrawString (Metadpy->dpy, Infowin->win, Infoclr->gc,
		   x, y, str, len);
    }
  }


  /* Success */
  return (0);
}






/*
 * Painting where text would be (assume TEXT_GRID)
 */

errr Infofnt_text_non (int x, int y, cptr str, int len, uint mode)
{
  int w, h;


  /*** Do a brief info analysis ***/

  /* If 'str' is NULL, fill all the way to the Infowin edge */
  if (!str)
  {
    /*** Find the X and W dimensions ***/

    /* Fill entire row */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      w = Infowin->w;
      x = 0;
    }

    /* Line up with x at left edge of column 'x', fill to right edge */
    else if (mode & TEXT_J_LT)
    {
      x = x * Infofnt->wid;
      w = Infowin->w - x;
    }

    /* Line up with x at right edge of column 'x', fill to left edge */
    else if (mode & TEXT_J_RT)
    {
      w = x * Infofnt->wid;
      x = 0;
    }

    /* Fill the entire Row */
    else
    {
      w = Infowin->w;
      x = 0;
    }
  }


  /* Otherwise, perform like (TEXT_GRID + not TEXT_FILL) above */
  else
  {
    /*** Find the width ***/

    /* Negative length is a flag to count the characters in str */
    if (len < 0) len = strlen (str);

    /* The total width will be 'len' chars * standard width */
    w = len * Infofnt->wid;


    /*** Find the X dimensions ***/

    /* Center horizontally, allow half-frid column leeway */
    if ((mode & TEXT_J_LT) && (mode & TEXT_J_RT))
    {
      x = (x * Infofnt->wid) - (w / 2);
    }

    /* Line up with x at left edge of column 'x' */
    else if (mode & TEXT_J_LT)
    {
      x = x * Infofnt->wid;
    }

    /* Line up with x at right edge of column 'x' */
    else if (mode & TEXT_J_RT)
    {
      x = (x - (len - 1)) * Infofnt->wid;
    }

    /* Center horizontally, snap to grid. */
    else
    {
      x = (x - (len / 2)) * Infofnt->wid;
    }
  }


  /*** Find other dimensions ***/

  /* Simply do 'Infofnt->hgt' (a single row) high */
  h = Infofnt->hgt;

  /* Simply do "at top" in row 'y' */
  y = y * h;


  /*** Actually 'paint' the area ***/

  /* Just do a Fill Rectangle */
  XFillRectangle (Metadpy->dpy, Infowin->win, Infoclr->gc, x, y, w, h);

  /* Success */
  return (0);
}



/*
 * Draw something resembling text somewhere with various options
 *
 * Globals:
 *   Metadpy: The base metadpy to draw on
 *   Infowin: The base infowin to draw on
 *   Infofnt: The base infofnt to draw with
 *   Infoclr: The base infoclr to draw with
 *
 * Inputs:
 *   x, y: The (x,y) location (in some representation)
 *   str:  The string to use, if any (else NULL)
 *   len:  The length of str, if known (else -1)
 *   mode: The mode (see header file) to employ
 *
 * Position:
 *   The (x,y) can be a pixel location, or a (row,col) position
 *   The text can be centered, or anchored to the left or right
 *   The text can be vertically centered, or anchored weirdly
 *   Vertical anchors include top, bottom, and baseline
 *   The default is to be centered horizontally and vertically
 *   The size of the letters can be accepted, or queried
 *   Text can be drawn, wiped, or painted
 *
 * Note:
 *   When in the 'fill_text' routine, if 'str' is NULL, then
 *   assume that the "string" extends forever towards various
 *   edges (both if neither RIGHT nor LEFT justified)
 */

errr Infofnt_text (int x, int y, cptr str, int len, uint mode)
{
  int i;

  /* Branch on mode */
  i = ((mode & TEXT_FILL) ?
       (Infofnt_text_non (x, y, str, len, mode)) :
       (Infofnt_text_std (x, y, str, len, mode)));

  /* Return result */
  return (i);
}




