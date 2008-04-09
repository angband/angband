/* File: maid-x11.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file defines some "XImage" manipulation functions for X11.
 *
 * Original code by Desvignes Sebastien (desvigne@solar12.eerie.fr).
 *
 * BMP format support by Denis Eropkin (denis@dream.homepage.ru).
 *
 * Major fixes and cleanup by Ben Harrison (benh@phial.com).
 *
 * This file is designed to be "included" by "main-x11.c",
 * which will have already "included" several relevant header files.
 */

#include "angband.h"

#if defined(USE_X11) || defined(USE_XAW) || defined(USE_XPJ) || defined(USE_GTK)

#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif /* __MAKEDEPEND__ */

/* Include our headers */
#include "maid-x11.h"


#ifdef SUPPORT_GAMMA
static bool gamma_table_ready = FALSE;
static int gamma_val = 0;
#endif /* SUPPORT_GAMMA */


/*
 * Hack -- Convert an RGB value to an X11 Pixel, or die.
 */
u32b create_pixel(Display *dpy, byte red, byte green, byte blue)
{
	Colormap cmap = DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy));

	XColor xcolour;

#ifdef SUPPORT_GAMMA

	if (!gamma_table_ready)
	{
		cptr str = getenv("ANGBAND_X11_GAMMA");
		if (str != NULL) gamma_val = atoi(str);

		gamma_table_ready = TRUE;

		/* Only need to build the table if gamma exists */
		if (gamma_val) build_gamma_table(gamma_val);
	}

	/* Hack -- Gamma Correction */
	if (gamma_val > 0)
	{
		red = gamma_table[red];
		green = gamma_table[green];
		blue = gamma_table[blue];
	}

#endif /* SUPPORT_GAMMA */

	/* Build the color */

	xcolour.red = red * 255;
	xcolour.green = green * 255;
	xcolour.blue = blue * 255;
	xcolour.flags = DoRed | DoGreen | DoBlue;

	/* Attempt to Allocate the Parsed color */
	if (!(XAllocColor(dpy, cmap, &xcolour)))
	{
		quit_fmt("Couldn't allocate bitmap color #%04x%04x%04x\n",
		         xcolour.red, xcolour.green, xcolour.blue);
	}

	return (xcolour.pixel);
}


/*
 * Get the name of the default font to use for the term.
 */
cptr get_default_font(int term_num)
{
	cptr font;

	char buf[80];

	/* Window specific font name */
	strnfmt(buf, sizeof(buf), "ANGBAND_X11_FONT_%d", term_num);

	/* Check environment for that font */
	font = getenv(buf);

	/* Check environment for "base" font */
	if (!font) font = getenv("ANGBAND_X11_FONT");

	/* No environment variables, use default font */
	if (!font)
	{
		switch (term_num)
		{
			case 0:
			{
				font = DEFAULT_X11_FONT_0;
			}
			break;
			case 1:
			{
				font = DEFAULT_X11_FONT_1;
			}
			break;
			case 2:
			{
				font = DEFAULT_X11_FONT_2;
			}
			break;
			case 3:
			{
				font = DEFAULT_X11_FONT_3;
			}
			break;
			case 4:
			{
				font = DEFAULT_X11_FONT_4;
			}
			break;
			case 5:
			{
				font = DEFAULT_X11_FONT_5;
			}
			break;
			case 6:
			{
				font = DEFAULT_X11_FONT_6;
			}
			break;
			case 7:
			{
				font = DEFAULT_X11_FONT_7;
			}
			break;
			default:
			{
				font = DEFAULT_X11_FONT;
			}
		}
	}

	return (font);
}


#endif /* USE_X11 || USE_XAW || USE_XPJ || USE_GTK */

