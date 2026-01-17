/**
 * \file main-sdl.c
 * \brief Angband SDL port 
 *
 * Copyright (c) 2007 Ben Harrison, Gregory Velichansky, Eric Stevens,
 * Leon Marrick, Iain McFall, and others
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
#include "game-input.h"
#include "game-world.h"
#include "init.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-output.h"
#include "ui-prefs.h"

/**
 * Comments and suggestions are welcome. The UI probably needs some
 * adjustment, and I need comments from you.
 * perhaps also something like "Angband 3.0.8 by Andi Sidwell and others;
 * SDL port by Iain McFall an others, please see the accompanying documentation
 * for credits" or something
 */

/**
 * This file helps Angband work with at least some computers capable of running
 * SDL, a set of simple and quite popular game development libraries that work
 * on many different operating systems, including Windows, most flavours of
 * UNIX or similar, and Mac OS X.  It requires a 32-bit (or higher) machine
 * capable of displaying at least 640x480 in 256 colors.  A Pentium or better
 * is strongly recommended (for speed reasons treated more fully below).
 *
 * To use this file, use an appropriate "Makefile" or "Project File", install
 * the required libraries (described below), make sure that "USE_SDL" is
 * defined somewhere, and obtain various extra files (described below).  If
 * you are new to all this, read "makefile.sdl".
 *
 * This port uses the following libraries:  SDL (v1.2+) and  SDL_ttf.
 * All are available as source code, pre-compiled libs for developers,
 * and libs (or dlls) for players from www.libsdl.org
 *
 *
 * Other files used by this port:
 * - The game must have a collection of bitmap .fon files in /lib/fonts.
 *
 * - It also needs some .png graphics files in /lib/tiles, 
 *   such as "16x16.png" and "16x16m.bmp".
 *
 * - The "lib/customize/pref-sdl.prf" file contains keymaps, macro definitions,
 *   and/or color redefinitions.
 * - The "lib/customize/font-sdl.prf" contains attr/char mappings for use with
 *   the normal "*.fon" font files in the "lib/fonts/" directory.
 *
 *
 *
 * "Term" framework by Ben Harrison (benh@phial.com).
 *
 * Original Sangband SDL port and the "intrface" module by Leon Marrick
 * (www.runegold.org/sangband).
 *
 * Additional helpful ideas by:
 * 2001 Gregory Velichansky <hmaon@bumba.net>, creator of the first Angband SDL
 * port.
 * 2006 Eric Stevens <sdltome@gmail.com>, main author of the TOME SDL port.
 */

/**
 * Comments on using SDL with Angband:
 *
 * The good news:
 * - SDL is cross-platform.  Really.  No joke.  If this port doesn't work on
 *   your system, it probably isn't SDL's fault.
 * - SDL is relatively easy to use, allowing you you to cobble up feature-rich
 *   apps that look half-way decent without too much fuss and bother.  It's
 *   wonderful for prototyping.
 * - SDL does most of what *band developers are likely to need, plus a whole
 *   lot more we haven't realized we want yet.
 * - SDL is a cleanly written open-source API; it is much less painful to get
 *   the straight word then it is with most other libraries.  Also, the SDL
 *   community offers active discussion boards, solid documentation, and
 *   numerous code examples relating to almost any question you might have
 *   occasion to ask.
 *
 * The bad news:
 * - SDL can be tedious to install.  Each individual library is straightforward
 *   enough, but *band development work requires several, the number growing as
 *   you get more sophisticated.
 * - SDL (as a stand-alone lib, without the assistance of OpenGL) can be very
 *   sluggish if you aren't careful.  It is poor at detecting, let alone making
 *   fullest use of, available video hardware, which cripples speed.  So,
 *   getting half-way decent performance in a game like Angband takes some
 *   skill and vast amounts of effort.  Speed - the lack thereof - is this
 *   port's biggest problem.  More comments below.
 * - SDL is not a complete game development library (although the add-ons help
 *   tremendously).  Much-needed functionality - text output, blit stretching,
 *   and video refresh synching being three examples - were either missing
 *   altogether or covered by functions that proved too slow or delicate for
 *   production use.  I ended up having to spend at least as much time, and
 *   write at least as much low-level code, as I did using the Win32 API.
 * - SDL, like Allegro but to a lesser extent, is falling behind current tech-
 *   nology.  Development progresses, but obsolescence looms, especially on
 *   Windows machines.
 */

#ifdef USE_SDL

#include "main.h"
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

/* SDL flags used for the main window surface */
static Uint32 vflags = SDL_ANYFORMAT;

/* Current screen dimensions */
static int screen_w = 800;
static int screen_h = 600;

/* Fullscreen dimensions */
static int full_w;
static int full_h;

/* Want fullscreen? */
static bool fullscreen = false;

static int overdraw = 0;
static int overdraw_max = 0;

static char *sdl_settings_file;

/**
 * One if the player requests an exit and the game is not at a command prompt;
 * any non-zero value other than one when ready to save the game at exit but
 * the game may request additional input; zero in all other cases
 */
static int quit_when_ready = 0;

/* Default point size for scalable fonts */
#define DEFAULT_POINT_SIZE (10)
/* Minimum allowed point size for scalable fonts */
#define MIN_POINT_SIZE (4)
/* Maximum allowed point size for scalable fonts */
#define MAX_POINT_SIZE (64)

#define MAX_FONTS 40
char *FontList[MAX_FONTS];
static int num_fonts = 0;

/* Holds details about requested properties for a terminal's font. */
typedef struct term_font term_font;
struct term_font
{
	const char *name;	/* final component of path if one of the
					preset fonts; full path if not a
					preset font */
	char *alloc_name;	/* same as name if dynamically allocated;
					otherwise, it is NULL */
	int size;	/* requested point size for the file; zero for
				bitmapped fonts */
	bool preset;	/* true if this is a font included in the lib/fonts
				directory for the game */
	bool bitmapped;	/* true if this is a bitmapped (.fon; case-insensitive)
				font that can't be scaled */
};

/**
 * Used as 'system' font.
 */
static const term_font default_term_font = { "6x10x.fon", NULL, 0, true, true };

/**
 * Used by the 'Point Size' and 'Font Browser' panels to accumulate
 * information about a new requested font.
 */
static term_font new_font = { NULL, NULL, 0, false, false };

/**
 * A font structure
 * Note that the data is only valid for a surface with matching
 * values for pitch & bpp. If a surface is resized the data _must_ be
 * recalculated.
 */
typedef struct sdl_Font sdl_Font;
struct sdl_Font
{
	int width;		/* The dimensions of this font (in pixels)*/
	int height;

	char name[32];          /* Label in menu used to select the font */

	Uint16 pitch;		/* Pitch of the surface this font is made for */
	Uint8 bpp;		/* Bytes per pixel of the surface */
	Uint8 something;	/* Padding */

	const term_font *req;	/* Backreference to the request for this font */
	int *data;		/* The data */
	TTF_Font *sdl_font;	/* The native font */
};

static sdl_Font SystemFont;

/*
 * Window information
 * Each window has its own surface and coordinates
 */
typedef struct term_window term_window;
struct term_window
{
	term term_data;

	SDL_Surface *surface;	/* The surface for this window */
	SDL_Surface *tiles;		/* The appropriately sized tiles for this window */
	SDL_Surface *onebyone;	/* The appropriately sized tiles for this window */
	uint8_t Term_idx;	/* Index of term that relates to this */

	int top;				/* Window Coordinates on the main screen */
	int left;

	int keys;				/* Size of keypress storage */

	sdl_Font font;			/* Font info */
	term_font req_font;		/* Requested font */
	int rows;			/* Dimension in tiles */
	int cols;

	int border;				/* Border width */
	int title_height;		/* Height of title bar */

	int width;				/* Dimension in pixels ==
							 * tile_wid * cols + 2 x borde r*/
	int height;

	int tile_wid;			/* Size in pixels of a char */
	int tile_hgt;

	bool visible;			/* Can we see this window? */

	SDL_Rect uRect;			/* The part that needs to be updated */
};




typedef struct mouse_info mouse_info;
struct mouse_info
{
	int left;			/* Is it pressed? */
	int right;

	int leftx;			/* _IF_ left button is pressed these */
	int lefty;			/* show where it was pressed */

	int rightx;
	int righty;

	int x;				/* Current position of mouse */
	int y;

};

#define WINDOW_DRAW (SDL_USEREVENT + 1)

/**
 * The basic angband text colours in an sdl friendly form
 */
static SDL_Color text_colours[MAX_COLORS];

SDL_Color back_colour;		/* Background colour */
Uint32 back_pixel_colour;
/* Default color for button captions */
SDL_Color DefaultCapColour = { 0, 0, 0, 0 };

typedef struct sdl_ButtonBank sdl_ButtonBank;
typedef struct sdl_Button sdl_Button;
typedef struct sdl_Window sdl_Window;

typedef void (*button_press_func)(sdl_Button *sender);
struct sdl_Button
{
	SDL_Rect pos;				/* Position & Size */
	bool selected;				/* Selected? */
	bool visible;				/* Visible? */
	button_press_func activate;	/* A function to call when pressed */
	sdl_ButtonBank *owner;		/* Which bank is this in? */
	char caption[50];			/* Text for this button */
	SDL_Color unsel_colour;		/* Button unselected colour */
	SDL_Color sel_colour;		/* Selected colour*/
	SDL_Color cap_colour;		/* Caption colour */
	void *data;					/* Something */
	int tag;					/* Something */

};

struct sdl_ButtonBank
{
	sdl_Button *buttons;		/* A collection of buttons */
	bool *used;					/* What buttons are available? */
	sdl_Window *window;			/* The window that these buttons are on */
	bool need_update;
};

/**
 * Other 'windows' (basically a surface with a position and buttons on it)
 * Currently used for the top status bar and popup windows
 */
typedef void (*sdl_WindowCustomDraw)(sdl_Window *window);
struct sdl_Window
{
	int top;				/* Position on main window */
	int left;

	int width;				/* Dimensions */
	int height;

	bool visible;			/* Visible? */

	SDL_Surface *surface;	/* SDL surface info */

	sdl_ButtonBank buttons;		/* Buttons */

	sdl_Font font;			/* Font */

	SDL_Surface *owner;		/* Who shall I display on */

	sdl_WindowCustomDraw draw_extra; /* Stuff to draw on the surface */
	bool need_update;
};




/**
 * The main surface of the application
 */
static SDL_Surface *AppWin;

/**
 * The status bar
 */
static sdl_Window StatusBar;

/**
 * The Popup window
 */
static sdl_Window PopUp;
static bool popped;

/**
 * Term windows
 */
static term_window windows[ANGBAND_TERM_MAX];
static int Zorder[ANGBAND_TERM_MAX];

/**
 * Keep track of the mouse status
 */
static mouse_info mouse;

/**
 * The number pad consists of 10 keys, each with an SDL identifier
 */
#define is_numpad(k) \
((k == SDLK_KP0) || (k == SDLK_KP1) || (k == SDLK_KP2) || (k == SDLK_KP3) || \
 (k == SDLK_KP4) || (k == SDLK_KP5) || (k == SDLK_KP6) || \
 (k == SDLK_KP7) || (k == SDLK_KP8) || (k == SDLK_KP9) || (k == SDLK_KP_ENTER))

static int SnapRange = 5;	/* Window snap range (pixels) */
static int StatusHeight;	/* The height in pixels of the status bar */
static int SelectedTerm;	/* Current selected Term */

static int AboutSelect;		/* About button */
static int TermSelect;		/* Term selector button */
static int FontSelect;		/* Font selector button */
static int VisibleSelect;	/* Hide/unhide window button*/
static int MoreSelect;		/* Other options button */
static int QuitSelect;		/* Quit button */

/* For saving the icon for the About Box */
static SDL_Surface *mratt = NULL;

/*
 * Unselected colour used on the 'About', 'More', 'Point Size', and
 * 'Font Browser' panels; also used to highlight the currently selected font
 * in the font menu
 */
static SDL_Color AltUnselColour = { 160, 60, 60, 0 };
/*
 * Selected colour used on the 'More', 'Point Size', and 'Font Browser' panels
 */
SDL_Color AltSelColour = { 210, 110, 110, 0 };
/*
 * Used to highlight the currently selected font in the font menu and
 * 'Font Browser' panel
 */
SDL_Color AltCapColour = { 95, 95, 195, 0 };

/* Buttons on the 'More' panel */
static int MoreOK;			/* Accept changes */
static int MoreFullscreen;	/* Fullscreen toggle button */
static int MoreSnapPlus;	/* Increase snap range */
static int MoreSnapMinus;	/* Decrease snap range */

/* Buttons on the 'Point Size' panel */
static int PointSizeBigDec;	/* decrease point size by 10 */
static int PointSizeDec;	/* decrease point size by 1 */
static int PointSizeInc;	/* increase point size by 1 */
static int PointSizeBigInc;	/* increase point size by 10 */
static int PointSizeOk;		/* accept current point size */
static int PointSizeCancel;	/* cancel point size selection */
/* Width of the border box about the 'Point Size' panel */
#define POINT_SIZE_BORDER (5)
/* Width of margin within the border box about the 'Point Size' panel */
#define POINT_SIZE_MARGIN (2)

/*
 * Number of directories and files shown at a time in the 'Font Browser' panel;
 * should be at least 3 so that there's vertical space for the scrolling
 * controls; if making this larger than 15, you'll likely have to increase
 * MAX_BUTTONS as well
 */
#define FONT_BROWSER_PAGE_ENTRIES (15)
/*
 * Number of characters to show for each directory in the 'Font Browser' panel
 */
#define FONT_BROWSER_DIR_LENGTH (15)
/* Number of characters to show for each file in the 'Font Browser' panel */
#define FONT_BROWSER_FILE_LENGTH (25)
/* Buttons on the 'Font Browser' panel */
static int FontBrowserDirUp;
static int FontBrowserDirectories[FONT_BROWSER_PAGE_ENTRIES];
static int FontBrowserDirPageBefore;
static int FontBrowserDirPageAfter;
static int FontBrowserDirPageDummy;
static int FontBrowserFiles[FONT_BROWSER_PAGE_ENTRIES];
static int FontBrowserFilePageBefore;
static int FontBrowserFilePageAfter;
static int FontBrowserFilePageDummy;
static int FontBrowserPtSizeBigDec;
static int FontBrowserPtSizeDec;
static int FontBrowserPtSizeInc;
static int FontBrowserPtSizeBigInc;
static int FontBrowserOk;
static int FontBrowserRefresh;
static int FontBrowserCancel;
/* Height of the preview part of the 'Font Browser' panel */
#define FONT_BROWSER_PREVIEW_HEIGHT (80)
/* Width of the border around the 'Font Browser' panel */
#define FONT_BROWSER_BORDER (5)
/* Width of margin within the border box around the 'Font Browser' panel */
#define FONT_BROWSER_MARGIN (2)
/*
 * Width of the border around the directory and file subpanels of the
 * 'Font Browser' panel
 */
#define FONT_BROWSER_SUB_BORDER (3)
/*
 * Width of margin within the border box aroudn the directory and file
 * subpanels of the 'Font Browser' panel
 */
#define FONT_BROWSER_SUB_MARGIN (1)
/*
 * Width of space to separate the scroll controls in the 'Font Browser' panel
 * and the subpanel they affect.
 */
#define FONT_BROWSER_HOR_SPACE (2)
/*
 * In the 'Font Browser' panel, the height of space to separate the point size
 * control from file and directory subpanels and the preview area from the
 * point size control
 */
#define FONT_BROWSER_VER_SPACE (4)

/*
 * Current directory being browsed by the 'Font Browser' panel; if not NULL,
 * should end with a path separator
 */
static char *FontBrowserCurDir = NULL;
/*
 * The length of the root portion (the part that shouldn't be backed over
 * when going up the directory tree) of FontBrowerCurDir
 */
static size_t FontBrowserRootSz = 0;
/*
 * Last directory browsed by the previous instance of the 'Font Browser' panel
 */
static char *FontBrowserLastDir = NULL;
/*
 * Value for the length of the root portion of FontBrowserLastDir
 */
static size_t FontBrowserLastRootSz = 0;
/*
 * Array of the unabbreviated directory names in the current directory being
 * browsed by the 'Font Browser' panel
 */
static char **FontBrowserDirEntries = NULL;
/*
 * Number of directories in the directory currently being browsed by the
 * 'Font Browser'
 */
static size_t FontBrowserDirCount = 0;
/* Number of entries allocated in the FontBrowserDirEntries array */
static size_t FontBrowserDirAlloc = 0;
/*
 * Current page (each with FONT_BROWSER_PAGE_ENTRIES) of directories viewed
 * by the 'Font Browser' panel
 */
static size_t FontBrowserDirPage = 0;
/*
 * Array of the unabbreviated fixed-width font files in the current directory
 * being browsed by the 'Font Browser' panel
 */
static char **FontBrowserFileEntries = NULL;
/*
 * Number of usable files in the directory currently being browsed by the
 * 'Font Browser'
 */
static size_t FontBrowserFileCount = 0;
/* Number of entries allocated in the FontBrowserFileEntries array */
static size_t FontBrowserFileAlloc = 0;
/*
 * Current page (each with FONT_BROWSER_PAGE_ENTRIES) of files viewed
 * by the 'Font Browser' panel
 */
static size_t FontBrowserFilePage = 0;
/*
 * Currently selected file entry in the 'Font Browser' panel or (size_t) -1
 * if there isn't a current selection
 */
static size_t FontBrowserFileCur = (size_t) -1;
/* Font data used when rendering the preview in the 'Font Browser' panel */
static sdl_Font *FontBrowserPreviewFont = NULL;

static bool Moving;				/* Moving a window */
static bool Sizing;				/* Sizing a window */
static SDL_Rect SizingSpot;		/* Rect to descibe the sizing area */
static bool Sizingshow = false;	/* Is the resize thingy displayed? */
static SDL_Rect SizingRect;		/* Rect to describe the current resize window */

#include "grafmode.h"

static SDL_Surface *GfxSurface = NULL;	/* A surface for the graphics */

static int MoreWidthPlus;	/* Increase tile width */
static int MoreWidthMinus;	/* Decrease tile width */
static int MoreHeightPlus;	/* Increase tile height */
static int MoreHeightMinus;	/* Decrease tile height */
static int *GfxButtons;	/* Graphics mode buttons */
static int SelectedGfx;				/* Current selected gfx */

static bool SimpleConfirm(const char *msg, const char *label1,
		const char *label2, bool first_default);
static int sdl_ModalEventLoop(void);

/**
 * Verify if the given path refers to a font file that can be used.
 */
static bool is_font_file(const char *path)
{
	bool result = false;
	TTF_Font *font = TTF_OpenFont(path, 1);

	if (font) {
		if (TTF_FontFaceIsFixedWidth(font)) {
			result = true;
		}
		TTF_CloseFont(font);
	}
	return result;
}

/**
 * Fill a buffer with the short name for a font.
 */
static void get_font_short_name(char *buf, size_t bufsz, const term_font *font)
{
	if (font->bitmapped) {
		my_strcpy(buf, font->name + path_filename_index(font->name),
			bufsz);
	} else {
		strnfmt(buf, bufsz, "%dpt %s", font->size, font->name +
			path_filename_index(font->name));
	}
}

/**
 * Fill in an SDL_Rect structure.
 * Note it also returns the value adjusted
 */
static SDL_Rect *RECT(int x, int y, int w, int h, SDL_Rect *rect)
{	
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;

	return rect;
}

/**
 * Is a point(x, y) in a rectangle?
 */
static bool point_in(SDL_Rect *rect, int x, int y)
{
	if (x < rect->x) return (false);
	if (y < rect->y) return (false);
	if (x >= rect->x + rect->w) return (false);
	if (y >= rect->y + rect->h) return (false);

	/* Must be inside */
	return (true);
}

/**
 * Draw an outline box
 * Given the top, left, width & height
 */
static void sdl_DrawBox(SDL_Surface *surface, SDL_Rect *rect, SDL_Color colour, int width)
{
	SDL_Rect rc;
	int left = rect->x;
	int right = rect->x + rect->w - width;
	int top = rect->y;
	int bottom = rect->y + rect->h - width;
	Uint32 pixel_colour = SDL_MapRGB(surface->format, colour.r, colour.g, colour.b);

	/* Top left -> Top Right */
	RECT(left, top, rect->w, width, &rc);
	SDL_FillRect(surface, &rc, pixel_colour);

	/* Bottom left -> Bottom Right */
	RECT(left, bottom, rect->w, width, &rc);
	SDL_FillRect(surface, &rc, pixel_colour);

	/* Top left -> Bottom left */
	RECT(left, top, width, rect->h, &rc);
	SDL_FillRect(surface, &rc, pixel_colour);

	/* Top right -> Bottom right */
	RECT(right, top, width, rect->h, &rc);
	SDL_FillRect(surface, &rc, pixel_colour);
}

/**
 * Get the width and height of a given font file
 */
static errr sdl_CheckFont(const term_font *req_font, int *width, int *height)
{
	TTF_Font *ttf_font;
	errr result;

	if (req_font->preset) {
		char buf[1024];

		/* Build the path */
		path_build(buf, sizeof(buf), ANGBAND_DIR_FONTS, req_font->name);
		/* Attempt to load it */
		ttf_font = TTF_OpenFont(buf, req_font->size);
	} else {
		/* Attempt to load it */
		ttf_font = TTF_OpenFont(req_font->name, req_font->size);
	}

	/* Bugger */
	if (!ttf_font) return (-1);

	/* Get the size */
	if (!TTF_FontFaceIsFixedWidth(ttf_font)
			|| TTF_SizeText(ttf_font, "M", width, height)) {
		result = -1;
	} else {
		result = 0;
	}

	/* Finished with the font */
	TTF_CloseFont(ttf_font);

	return result;
}

/**
 * The sdl_Font routines
 */

/**
 * Free any memory assigned by Create()
 */
static void sdl_FontFree(sdl_Font *font)
{
	/* Finished with the font */
	TTF_CloseFont(font->sdl_font);
	font->sdl_font = NULL;
}


/**
 * Create new font data with font fontname, optimizing the data
 * for the surface given
 */
static errr sdl_FontCreate(sdl_Font *font, const term_font *req_font,
		SDL_Surface *surface)
{
	TTF_Font *ttf_font;

	if (req_font->preset) {
		char buf[1024];

		/* Build the path */
		path_build(buf, sizeof(buf), ANGBAND_DIR_FONTS, req_font->name);
		/* Attempt to load it */
		ttf_font = TTF_OpenFont(buf, req_font->size);
	} else {
		ttf_font = TTF_OpenFont(req_font->name, req_font->size);
	}

	/* Bugger */
	if (!ttf_font) return -1;

	/* Get the size */
	if (TTF_SizeText(ttf_font, "M", &font->width, &font->height)) {
		TTF_CloseFont(ttf_font);
		return -1;
	}

	/* Fill in some of the font struct */
	get_font_short_name(font->name, sizeof(font->name), req_font);
	font->req = req_font;
	font->pitch = surface->pitch;
	font->bpp = surface->format->BytesPerPixel;
	font->sdl_font = ttf_font;

	/* Success */
	return 0;
}




/**
 * Draw some text onto a surface, allowing shaded backgrounds
 * The surface is first checked to see if it is compatible with
 * this font, if it isn't the font will be 're-precalculated'
 *
 * You can, I suppose, use one font on many surfaces, but it is
 * definitely not recommended. One font per surface is good enough.
 */
static errr sdl_mapFontDraw(sdl_Font *font, SDL_Surface *surface,
							SDL_Color colour, SDL_Color bg, int x, int y,
							int n , const char *s)
{
	Uint8 bpp = surface->format->BytesPerPixel;
	Uint16 pitch = surface->pitch;

	SDL_Rect rc;
	SDL_Surface *text;

	if ((bpp != font->bpp) || (pitch != font->pitch))
		sdl_FontCreate(font, font->req, surface);

	/* Lock the window surface (if necessary) */
	if (SDL_MUSTLOCK(surface))
		if (SDL_LockSurface(surface) < 0)
			return (-1);

	RECT(x, y, n * font->width, font->height, &rc);
	text = TTF_RenderUTF8_Shaded(font->sdl_font, s, colour, bg);
	if (text) {
	    SDL_BlitSurface(text, NULL, surface, &rc);
	    SDL_FreeSurface(text);
	}

	/* Unlock the surface */
	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

	/* Success */
	return (0);
}

/**
 * Draw some text onto a surface
 * The surface is first checked to see if it is compatible with
 * this font, if it isn't the font will be 're-precalculated'
 *
 * You can, I suppose, use one font on many surfaces, but it is
 * definitely not recommended. One font per surface is good enough.
 */
static errr sdl_FontDraw(sdl_Font *font, SDL_Surface *surface, SDL_Color colour,
						 int x, int y, int n , const char *s)
{
	Uint8 bpp = surface->format->BytesPerPixel;
	Uint16 pitch = surface->pitch;

	SDL_Rect rc;
	SDL_Surface *text;

	if ((bpp != font->bpp) || (pitch != font->pitch))
		sdl_FontCreate(font, font->req, surface);

	/* Lock the window surface (if necessary) */
	if (SDL_MUSTLOCK(surface))
		if (SDL_LockSurface(surface) < 0)
			return (-1);

	RECT(x, y, n * font->width, font->height, &rc);
	text = TTF_RenderUTF8_Solid(font->sdl_font, s, colour);
	if (text) {
		SDL_BlitSurface(text, NULL, surface, &rc);
		SDL_FreeSurface(text);
	}

	/* Unlock the surface */
	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

	/* Success */
	return (0);
}



/**
 * Draw a button on the screen
 */
static void sdl_ButtonDraw(sdl_Button *button)
{
	SDL_Surface *surface = button->owner->window->surface;
	sdl_Font *font = &button->owner->window->font;

	SDL_Color colour = button->selected ?
		button->sel_colour : button->unsel_colour;

	if (!button->visible) return;

	SDL_FillRect(surface, &button->pos, SDL_MapRGB(surface->format, colour.r,
												   colour.g, colour.b));

	if (strlen(button->caption)) {
		size_t len = strlen(button->caption);

		unsigned max = button->pos.w / font->width;
		int n = MIN(len, max);
		int l = n * font->width / 2;
		int x = button->pos.x + ((button->pos.w) / 2) - l;
		
		sdl_FontDraw(font, surface, button->cap_colour,
					 x, button->pos.y + 1, n, button->caption);
	}
}

/**
 * Adjust the position of a button
 */
static void sdl_ButtonMove(sdl_Button *button, int x, int y)
{
	button->pos.x = x;
	button->pos.y = y;
	button->owner->need_update = true;
}

/**
 * Adjust the size of a button
 */
static void sdl_ButtonSize(sdl_Button *button, int w, int h)
{
	button->pos.w = w;
	button->pos.h = h;
	button->owner->need_update = true;
}

/**
 * Set the caption
 */
static void sdl_ButtonCaption(sdl_Button *button, const char *s)
{
	my_strcpy(button->caption, s, sizeof(button->caption));
	button->owner->need_update = true;
}

/**
 * Set the visibility of a button
 */
static void sdl_ButtonVisible(sdl_Button *button, bool visible)
{
	if (button->visible != visible) {
		button->visible = visible;
		
		button->owner->need_update = true;
	}
}


/**
 * Maximum amount of buttons in a bank
 */
#define MAX_BUTTONS 50

/**
 * ------------------------------------------------------------------------
 * The button_bank package
 * ------------------------------------------------------------------------ */

/**
 * Initialize it
 */
static void sdl_ButtonBankInit(sdl_ButtonBank *bank, sdl_Window *window)
{
	bank->window = window;
	bank->buttons = mem_zalloc(MAX_BUTTONS * sizeof(sdl_Button));
	bank->used = mem_zalloc(MAX_BUTTONS * sizeof(bool));
	bank->need_update = true;
}

/**
 * Clear the bank
 */
static void sdl_ButtonBankFree(sdl_ButtonBank *bank)
{
	mem_free(bank->buttons);
	mem_free(bank->used);
}

/**
 * Draw all the buttons on the screen
 */
static void sdl_ButtonBankDrawAll(sdl_ButtonBank *bank)
{
	int i;

	for (i = 0; i < MAX_BUTTONS; i++) {
		sdl_Button *button = &bank->buttons[i];
		
		if (!bank->used[i]) continue;
		if (!button->visible) continue;
		
		sdl_ButtonDraw(button);
	}
	bank->need_update = false;
}

/**
 * Get a new button index
 */
static int sdl_ButtonBankNew(sdl_ButtonBank *bank)
{
	int i = 0;
	sdl_Button *new_button;

	while (bank->used[i] && (i < MAX_BUTTONS)) i++;

	if (i == MAX_BUTTONS)
		/* Bugger! */
		return (-1);

	/* Get the button */
	new_button = &bank->buttons[i];

	/* Mark the button as used */
	bank->used[i] = true;

	/* Clear it */
	memset(new_button, 0, sizeof(sdl_Button));

	/* Mark it as mine */
	new_button->owner = bank;

	/* Default colours */
	new_button->unsel_colour.r = 160;
	new_button->unsel_colour.g = 160;
	new_button->unsel_colour.b = 60;
	new_button->sel_colour.r = 210;
	new_button->sel_colour.g = 210;
	new_button->sel_colour.b = 110;
	new_button->cap_colour = DefaultCapColour;

	/* Success */
	return (i);
}

/**
 * Retrieve button 'idx' or NULL
 */
static sdl_Button *sdl_ButtonBankGet(sdl_ButtonBank *bank, int idx)
{
	/* Check the index */
	if ((idx < 0) || (idx >= MAX_BUTTONS)) return (NULL);
	if (!bank->used[idx]) return (NULL);

	/* Return it */
	return &bank->buttons[idx];
}

#if 0
/**
 * Remove a Button by its index
 */
static void sdl_ButtonBankRemove(sdl_ButtonBank *bank, int idx)
{
	sdl_Button *button;

	/* Check the index */
	if ((idx < 0) || (idx >= MAX_BUTTONS)) return;
	if (!bank->used[idx]) return;

	/* Grab it */
	button = &bank->buttons[idx];

	/* Hide it */
	button->visible = false;

	/* Draw it */
	bank->need_update = true;

	/* Forget */
	bank->used[idx] = false;
}

#endif

/**
 * Examine and respond to mouse presses
 * Return if we 'handled' the click
 */
static bool sdl_ButtonBankMouseDown(sdl_ButtonBank *bank, int x, int y)
{
	int i;

	/* Check every button */
	for (i = 0; i < MAX_BUTTONS; i++) {
		sdl_Button *button = &bank->buttons[i];
		
		/* Discard some */
		if (!bank->used[i]) continue;
		if (!button->visible) continue;
		
		/* Check the coordinates */
		if (point_in(&button->pos, x, y)) {
			button->selected = true;
			
			/* Draw it */
			bank->need_update = true;
			
			return (true);
		}
	}
	return (false);
}

/**
 * Respond to a mouse button release
 */
static bool sdl_ButtonBankMouseUp(sdl_ButtonBank *bank, int x, int y)
{
	int i;

	/* Check every button */
	for (i = 0; i < MAX_BUTTONS; i++) {
		sdl_Button *button = &bank->buttons[i];
		
		/* Discard some */
		if (!bank->used[i]) continue;
		if (!button->visible) continue;
		
		/* Check the coordinates */
		if (point_in(&button->pos, x, y)) {
			/* Has this button been 'selected'? */
			if (button->selected) {
				/*
				 * Do these before performing the callback (the
				 * button is no longer selected and needs to
				 * be redrawn) since the callback could remove
				 * the button.
				 */
				button->selected = false;
				bank->need_update = true;
				
				/* Activate the button (usually) */
				if (button->activate) (*button->activate)(button);

				return (true);
			}
		} else {
			/* This button was 'selected' but the release of the */
			/* mouse button was outside the area of this button */
			if (button->selected) {
				/* Now not selected */
				button->selected = false;
				
				/* Draw it */
				bank->need_update = true;
			}
		}
	}

	return (false);
}

/**
 * sdl_Window functions
 */
static void sdl_WindowFree(sdl_Window* window)
{
	if (window->surface) {
		SDL_FreeSurface(window->surface);
		sdl_ButtonBankFree(&window->buttons);
		sdl_FontFree(&window->font);
		memset(window, 0, sizeof(sdl_Window));
	}
}

/**
 * Initialize a window
 */
static void sdl_WindowInit(sdl_Window* window, int w, int h, SDL_Surface *owner,
		const term_font *req_font)
{
	sdl_WindowFree(window);
	window->owner = owner;
	window->width = w;
	window->height = h;
	window->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
										   owner->format->BitsPerPixel,
										   owner->format->Rmask,
										   owner->format->Gmask,
										   owner->format->Bmask,
										   owner->format->Amask);
	sdl_ButtonBankInit(&window->buttons, window);
	sdl_FontCreate(&window->font, req_font, window->surface);
	window->visible = true;
	window->need_update = true;
}


static void sdl_WindowBlit(sdl_Window* window)
{
	SDL_Rect rc;

	if (!window->visible) return;

	RECT(window->left, window->top, window->width, window->height, &rc);

	SDL_BlitSurface(window->surface, NULL, window->owner, &rc);
	SDL_UpdateRects(window->owner, 1, &rc);
}

static void sdl_WindowText(sdl_Window* window, SDL_Color c, int x, int y,
						   const char *s)
{
	sdl_FontDraw(&window->font, window->surface, c, x, y, strlen(s), s);
}

static void sdl_WindowUpdate(sdl_Window* window)
{
	if ((window->need_update || window->buttons.need_update) &&
		(window->visible)) {
		SDL_Event Event;
		
		SDL_FillRect(window->surface, NULL, back_pixel_colour);
		
		if (window->draw_extra) (*window->draw_extra)(window);
		
		sdl_ButtonBankDrawAll(&window->buttons);
		
		window->need_update = false;
		
		memset(&Event, 0, sizeof(SDL_Event));
		
		Event.type = WINDOW_DRAW;
		
		Event.user.data1 = (void*)window;
		
		SDL_PushEvent(&Event);
	}
}



static void term_windowFree(term_window* win)
{
	if (win->surface) {
		SDL_FreeSurface(win->surface);
		win->surface = NULL;

		/* Invalidate the gfx surface */
		if (win->tiles) {
			SDL_FreeSurface(win->tiles);
			win->tiles = NULL;
		}
		if (win->onebyone) {
			SDL_FreeSurface(win->onebyone);
			win->onebyone = NULL;
		}
		term_nuke(&win->term_data);
	}

	sdl_FontFree(&win->font);
}

static errr save_prefs(void);
static void hook_quit(const char *str)
{
	int i;

	save_prefs();

	string_free(sdl_settings_file);

	/* Free the surfaces of the windows */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		term_windowFree(&windows[i]);
		string_free(windows[i].req_font.alloc_name);
	}

	/* Free the graphics surface */
	if (GfxSurface) SDL_FreeSurface(GfxSurface);

	close_graphics_modes();
	if (GfxButtons) mem_free(GfxButtons);

	/* Free the 'System font' */
	sdl_FontFree(&SystemFont);

	/* Free the statusbar window */
	sdl_WindowFree(&StatusBar);

	/* free the popup window */
	sdl_WindowFree(&PopUp);

	/* Free the main surface */
	SDL_FreeSurface(AppWin);

	/* Shut down the font library */
	TTF_Quit(); 

	/* Shut down SDL */
	SDL_Quit();

	for (i = 0; i < MAX_FONTS; i++)
		string_free(FontList[i]);
}

/**
 * Respond to user interface events which either request an immediate
 * exit (forced is true) or a possible exit subject to requesting more
 * input from the user(forced is false).
 */
static void handle_quit(bool forced)
{
	if (character_generated) {
		/*
		 * Want to be at a command prompt so the game's state is ready
		 * to save.  If not at a command prompt and not forcing an
		 * exit, mark as ready to quit:  Term_extra_sdl_event() will
		 * use that to either call back to here when it is safe to
		 * save or send escapes to the game to satisfy its requests
		 * for input.
		 */
		if (!inkey_flag && !forced) {
			quit_when_ready = 1;
			return;
		}

		/* Drop pending messages. */
		msg_flag = false;
		quit_when_ready = 2;
		/*
		 * If not forcing an exit, allow the player to abort the
		 * exit if there is trouble saving the game.
		 */
		if (!forced && !save_game_checked()
				&& SimpleConfirm("Saving failed.  Really quit?",
				NULL, NULL, false) == 1) {
			quit_when_ready = 0;
			return;
		}
		close_game(false);
	}

	save_prefs();
	quit(NULL);
}

static void BringToTop(void)
{
	int i, idx;

	for (idx = 0; idx < ANGBAND_TERM_MAX; idx++)
		if (Zorder[idx] == SelectedTerm) break;

	if (idx == ANGBAND_TERM_MAX) return;

	for (i = idx; i < ANGBAND_TERM_MAX - 1; i++)
		Zorder[i] = Zorder[i + 1];

	Zorder[ANGBAND_TERM_MAX - 1] = SelectedTerm;
}


	
/**
 * Validate a file
 */
static void validate_file(const char *s)
{
	if (!file_exists(s))
		quit_fmt("cannot find required file:\n%s", s);
}

/**
 * Guess whether a file represents a bitmapped font based on the file name's
 * extension.
 *
 * Only include formats that are handled by FreeType (and therefore SDL_TTF).
 */
static bool is_bitmapped_font(const char *file_name)
{
	/*
	 * Microsoft executable only containing font resources; those can
	 * be stroked fonts, but here assume they are bitmapped as that is
	 * most common; ignore case in the file extension as both .fon and
	 * .FON are found
	 */
	if (suffix_i(file_name, ".fon")) {
		return true;
	}
	/*
	 * Portable Compiled Format (PCF) used by X11; accept either
	 * uncompressed or gzipped compressed versions
	 */
	if (suffix(file_name, ".pcf.gz") || suffix(file_name, ".pcf")) {
		return true;
	}
	/* Glyph Bitmap Distribution Format */
	if (suffix(file_name, ".bdf")) {
		return true;
	}
	return false;
}

/**
 * Find a window that is under the points x,y on
 * the main screen
 */

static int sdl_LocateWin(int x, int y)
{
	int i;

	for (i = ANGBAND_TERM_MAX - 1; i >= 0; i--) {
		term_window *win = &windows[Zorder[i]];
		SDL_Rect rc;
		
		if (!win->visible) continue;
		if (!point_in(RECT(win->left, win->top, win->width, win->height, &rc),
					  x, y))
			continue;
		
		return (Zorder[i]);
	}

	return (-1);
}

static void draw_statusbar(sdl_Window *window)
{
	char buf[128];
	term_window *win = &windows[SelectedTerm];
	int fw = window->font.width;
	int x = 1;
	sdl_Button *button;

	SDL_Rect rc;

	SDL_Color c = {160, 160, 60, 0};

	RECT(0, StatusBar.height - 1, StatusBar.width, 1, &rc);
	SDL_FillRect(StatusBar.surface, &rc, SDL_MapRGB(StatusBar.surface->format,
													c.r, c.g, c.b));

	button = sdl_ButtonBankGet(&StatusBar.buttons, AboutSelect);
	x += button->pos.w + 20;

	sdl_WindowText(&StatusBar, c, x, 1, "Term:");
	x += 5 * fw;

	button = sdl_ButtonBankGet(&StatusBar.buttons, TermSelect);
	button->pos.x = x;
	x += button->pos.w + 10;

	strnfmt(buf, sizeof(buf), "(%dx%d)", win->cols, win->rows);
	sdl_WindowText(&StatusBar, c, x, 1, buf);
	x += strlen(buf) * fw + 20;

	sdl_WindowText(&StatusBar, c, x, 1, "Visible:");
	x += 8 * fw;

	button = sdl_ButtonBankGet(&StatusBar.buttons, VisibleSelect);
	button->pos.x = x;
	x += button->pos.w + 20;

	button = sdl_ButtonBankGet(&StatusBar.buttons, FontSelect);
	if (button->visible) sdl_WindowText(&StatusBar, c, x, 1, "Font:");
	x += 5 * fw;

	
	button->pos.x = x;
	x += button->pos.w + 20;

	button = sdl_ButtonBankGet(&StatusBar.buttons, MoreSelect);
	button->pos.x = x;

	x += button->pos.w + 20;

	
}



static void sdl_BlitWin(term_window *win)
{
	SDL_Rect rc;

	if (!win->surface) return;
	if (!win->visible) return;
	if (win->uRect.x == -1) return;

	/* Select the area to be updated */
	RECT(win->left + win->uRect.x, win->top + win->uRect.y, win->uRect.w,
		 win->uRect.h, &rc);

	SDL_BlitSurface(win->surface, &win->uRect, AppWin, &rc);
	SDL_UpdateRects(AppWin, 1, &rc);

	/* Mark the update as complete */
	win->uRect.x = -1;
}

static void sdl_BlitAll(void)
{
	SDL_Rect rc;
	sdl_Window *window = &StatusBar;
	int i;
	SDL_Color colour = {160, 160, 60, 0};
	/* int32 ccolour = SDL_MapRGB(AppWin->format, 160, 40, 40); */
	SDL_FillRect(AppWin, NULL, back_pixel_colour);

	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		term_window *win = &windows[Zorder[i]];
		
		if (!win->surface) continue;
		if (!win->visible) continue;
		
		RECT(win->left, win->top, win->width, win->height, &rc);
		SDL_BlitSurface(win->surface, NULL, AppWin, &rc);
		
		if (Zorder[i] == SelectedTerm) {
			SizingSpot.w = 10;
			SizingSpot.h = 10;
			SizingSpot.x = win->left + win->width - 10;
			SizingSpot.y = win->top + win->height - 10;
			/* SDL_FillRect(AppWin, &SizingSpot, ccolour); */
			
			if (Sizing) {
				int width = 2;
				int grabsize = 10;
				rc = SizingRect;
				SizingSpot.w = grabsize;
				SizingSpot.h = grabsize;
				SizingSpot.x = SizingRect.x + SizingRect.w - grabsize;
				SizingSpot.y = SizingRect.y + SizingRect.h - grabsize;
				sdl_DrawBox(AppWin, &rc, colour, width);
			}
		}
	}

	RECT(window->left, window->top, window->width, window->height, &rc);

	SDL_BlitSurface(window->surface, NULL, AppWin, &rc);

	SDL_UpdateRect(AppWin, 0, 0, AppWin->w, AppWin->h);

	
}

static void RemovePopUp(void)
{
	PopUp.visible = false;
	popped = false;
	sdl_BlitAll();
}

static void QuitActivate(sdl_Button *sender)
{
	handle_quit(false);
}

static void SetStatusButtons(void)
{
	term_window *win = &windows[SelectedTerm];
	sdl_Button *button = sdl_ButtonBankGet(&StatusBar.buttons, TermSelect);
	sdl_Button *fontbutton = sdl_ButtonBankGet(&StatusBar.buttons, FontSelect);
	sdl_Button *visbutton = sdl_ButtonBankGet(&StatusBar.buttons,
											  VisibleSelect);

	sdl_ButtonCaption(button, angband_term_name[SelectedTerm]);

	if (!win->visible) {
		sdl_ButtonVisible(fontbutton, false);
		sdl_ButtonCaption(visbutton, "No");
	} else {
		sdl_ButtonVisible(fontbutton, true);
		sdl_ButtonCaption(fontbutton, win->font.name);
		sdl_ButtonCaption(visbutton, "Yes");
	}
}

static void TermFocus(int idx)
{
	if (SelectedTerm == idx) return;

	SelectedTerm = idx;

	BringToTop();

	SetStatusButtons();

	sdl_BlitAll();
}	

struct simple_confirm_data {
	const char *msg;
	int result;
};

static void SimpleConfirmHandlePress(sdl_Button *sender)
{
	struct simple_confirm_data *sc_data =
		(struct simple_confirm_data*)sender->data;

	sc_data->result = sender->tag;
	RemovePopUp();
}

static void SimpleConfirmDraw(sdl_Window *win)
{
	struct simple_confirm_data *sc_data = (struct simple_confirm_data*)
		win->buttons.buttons[0].data;
	size_t len = strlen(sc_data->msg);
	SDL_Rect rc;

	RECT(0, 0, win->width, win->height, &rc);
	SDL_FillRect(win->surface, &win->surface->clip_rect,
		SDL_MapRGB(win->surface->format, 255, 255, 255));
	sdl_DrawBox(win->surface, &win->surface->clip_rect, AltUnselColour, 5);
	sdl_WindowText(win, AltUnselColour,
		(win->width - ((len > 128) ? 128 : (int)len) * win->font.width)
		/ 2, 7, format("%.128s", sc_data->msg));
}

/**
 * Present a modal dialog with two choices.
 *
 * \param msg is the message to display in the confirmation dialog.
 * \param label1 is the label for the first button.  If NULL, use "Yes"
 * as the label.
 * \param label2 is the label for the second button.  If NULL, use "No"
 * as the label.
 * \param first_default indicates whether the first button is the default.
 * \return 0 if the player selected the first button, 1 if the player
 * selected the second button, 2 if the dialog was dismissed by SDL_QUIT, or
 * 3 if the dialog was dismissed by an error from SDL_WaitEvent().
 */
static bool SimpleConfirm(const char *msg, const char *label1,
		const char *label2, bool first_default)
{
	struct simple_confirm_data sc_data;
	size_t len = strlen(msg);
	int width = ((len > 128) ? 128 : len) * StatusBar.font.width + 14;
	int height = 4 * StatusBar.font.height + 14;
	int check_width, button_width, button_height, next, result;

	if (!label1) {
		label1 = "Yes";
		button_width = 3;
	} else {
		len = strlen(label1);
		button_width = (len >= 50) ? 49 : (int)len;
	}
	if (!label2) {
		label2 = "No";
		check_width = 2;
	} else {
		len = strlen(label2);
		check_width = (len >= 50) ? 49 : (int)len;
	}
	if (button_width < check_width) {
		button_width = check_width;
	}
	button_width = button_width * StatusBar.font.width + 4;
	button_height = StatusBar.font.height + 4;
	check_width = 3 * button_width + button_width / 2 + 14;
	if (width < check_width) {
		width = check_width;
	}

	sc_data.msg = msg;
	sc_data.result = (first_default) ? 0 : 1;
	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = (AppWin->w - width) / 2;
	PopUp.top = (AppWin->h - height) / 2;
	next = sdl_ButtonBankNew(&PopUp.buttons);
	PopUp.buttons.buttons[next].pos.x =
		(PopUp.width - (5 * button_width / 2)) / 2;
	PopUp.buttons.buttons[next].pos.y = PopUp.height - button_height - 10;
	PopUp.buttons.buttons[next].pos.w = button_width;
	PopUp.buttons.buttons[next].pos.h = button_height;
	PopUp.buttons.buttons[next].visible = true;
	PopUp.buttons.buttons[next].activate = SimpleConfirmHandlePress;
	(void)my_strcpy(PopUp.buttons.buttons[next].caption, label1,
		sizeof(PopUp.buttons.buttons[next].caption));
	PopUp.buttons.buttons[next].data = &sc_data;
	PopUp.buttons.buttons[next].tag = 0;
	next = sdl_ButtonBankNew(&PopUp.buttons);
	PopUp.buttons.buttons[next].pos.x = PopUp.width
		- (button_width + (PopUp.width - (5 * button_width / 2)) / 2);
	PopUp.buttons.buttons[next].pos.y = PopUp.height - button_height - 10;
	PopUp.buttons.buttons[next].pos.w = button_width;
	PopUp.buttons.buttons[next].pos.h = button_height;
	PopUp.buttons.buttons[next].visible = true;
	PopUp.buttons.buttons[next].activate = SimpleConfirmHandlePress;
	(void)my_strcpy(PopUp.buttons.buttons[next].caption, label2,
		sizeof(PopUp.buttons.buttons[next].caption));
	PopUp.buttons.buttons[next].data = &sc_data;
	PopUp.buttons.buttons[next].tag = 1;
	PopUp.draw_extra = SimpleConfirmDraw;

	popped = true;
	switch (sdl_ModalEventLoop()) {
	case 0:
		result = sc_data.result;
		break;

	case 1:
		result = 2;
		break;

	case 2:
		result = 3;
		break;

	default:
		assert(0);
		break;
	}
	return result;
}

static void AboutDraw(sdl_Window *win)
{
	SDL_Rect rc;
	SDL_Rect icon;
	const char *copyright_eol;

	RECT(0, 0, win->width, win->height, &rc);

	/* Draw a nice box */
	SDL_FillRect(win->surface, &win->surface->clip_rect,
				 SDL_MapRGB(win->surface->format, 255, 255, 255));
	sdl_DrawBox(win->surface, &win->surface->clip_rect, AltUnselColour, 5);
	if (mratt) {
		RECT((win->width - mratt->w) / 2, 5, mratt->w, mratt->h, &icon);
		SDL_BlitSurface(mratt, NULL, win->surface, &icon);
	}
	sdl_WindowText(win, AltUnselColour, 20, 150,
		format("You are playing %s", buildid));
	copyright_eol = SDL_strstr(copyright, "\n");
	if (copyright_eol) {
		char *line = SDL_malloc((size_t)(copyright_eol
			- copyright) + 1);

		(void)SDL_strlcpy(line, copyright,
			(size_t)(copyright_eol - copyright) + 1);
		sdl_WindowText(win, AltUnselColour, 20, 160, line);
		SDL_free(line);
	} else {
		sdl_WindowText(win, AltUnselColour, 20, 160, copyright);
	}
	sdl_WindowText(win, AltUnselColour, 20, 170,
		"See http://www.rephial.org");
}

static void AboutActivate(sdl_Button *sender)
{
	int width = 350;
	int height = 210;

	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = (AppWin->w / 2) - width / 2;
	PopUp.top = (AppWin->h / 2) - height / 2;
	PopUp.draw_extra = AboutDraw;

	popped = true;
}

static void SelectTerm(sdl_Button *sender)
{
	RemovePopUp();

	TermFocus(sender->tag);
}

static void TermActivate(sdl_Button *sender)
{
	int i, maxl = 0;
	int width, height = ANGBAND_TERM_MAX * (StatusBar.font.height + 1);

	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		int l = strlen(angband_term_name[i]);
		if (l > maxl) maxl = l;
	}

	width = maxl * StatusBar.font.width + 20;

	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = sender->pos.x;
	PopUp.top = sender->pos.y;

	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		int h = PopUp.font.height;
		int b = sdl_ButtonBankNew(&PopUp.buttons);
		sdl_Button *button = sdl_ButtonBankGet(&PopUp.buttons, b);
		sdl_ButtonSize(button, width - 2 , h);
		sdl_ButtonMove(button, 1, i * (h + 1));
		sdl_ButtonCaption(button, angband_term_name[i]);
		sdl_ButtonVisible(button, true);
		button->tag = i;
		button->activate = SelectTerm;
	}
	popped = true;
}

static void ResizeWin(term_window* win, int w, int h);
static void term_data_link_sdl(term_window *win);

static void VisibleActivate(sdl_Button *sender)
{
	term_window *window = &windows[SelectedTerm];

	if (SelectedTerm == 0) return;

	if (window->visible) {
		window->visible = false;
		term_windowFree(window);
		angband_term[SelectedTerm] = NULL;
	} else {
		window->visible = true;
		ResizeWin(window, window->width, window->height);
	}

	SetStatusButtons();
	sdl_BlitAll();
}

static void HelpWindowFontChange(term_window *window)
{
	int w, h;

	if (sdl_CheckFont(&window->req_font, &w, &h)) {
		quit_fmt("could not use the requested font %s",
			window->req_font.name);
	}

	/* Invalidate the gfx surface */
	if (window->tiles) {
		SDL_FreeSurface(window->tiles);
		window->tiles = NULL;
	}

	ResizeWin(window, (w * window->cols) + (2 * window->border),
		(h * window->rows) + window->border + window->title_height);

	SetStatusButtons();
}

static void SelectPresetBitmappedFont(sdl_Button *sender)
{
	term_window *window = &windows[SelectedTerm];

	sdl_FontFree(&window->font);
	string_free(window->req_font.alloc_name);
	window->req_font.alloc_name = string_make(sender->caption);
	window->req_font.name = window->req_font.alloc_name;
	window->req_font.size = 0;
	window->req_font.preset = true;
	window->req_font.bitmapped = true;
	HelpWindowFontChange(window);

	RemovePopUp();
}

static void ChangePointSize(sdl_Button *sender)
{
	sdl_Button *button;

	if ((new_font.size == MIN_POINT_SIZE && sender->tag < 0)
			|| (new_font.size == MAX_POINT_SIZE
			&& sender->tag > 0)) {
		/* Size does not change. */
		return;
	}

	new_font.size = MAX(MIN_POINT_SIZE, MIN(MAX_POINT_SIZE,
		new_font.size + sender->tag));

	/* Change colors on the buttons. */
	if (new_font.size > MIN_POINT_SIZE) {
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeBigDec);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeDec);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeBigDec);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeDec);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	if (new_font.size < MAX_POINT_SIZE) {
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeInc);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeBigInc);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeInc);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeBigInc);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	PopUp.need_update = true;
}

static void AcceptPointSize(sdl_Button *sender)
{
	term_window *window = &windows[SelectedTerm];

	sdl_FontFree(&window->font);
	string_free(window->req_font.alloc_name);
	window->req_font.alloc_name = string_make(new_font.name);
	window->req_font.name = window->req_font.alloc_name;
	window->req_font.size = new_font.size;
	window->req_font.preset = new_font.preset;
	assert(!new_font.bitmapped);
	window->req_font.bitmapped = false;
	HelpWindowFontChange(window);

	RemovePopUp();
}

static void CancelPointSize(sdl_Button *sender)
{
	string_free(new_font.alloc_name);
	new_font.name = NULL;
	new_font.alloc_name = NULL;
	new_font.size = 0;
	new_font.preset = false;
	new_font.bitmapped = false;
	RemovePopUp();
}

static void DrawPointSize(sdl_Window *win)
{
	SDL_Rect rc;

	RECT(0, 0, win->width, win->height, &rc);
	sdl_DrawBox(win->surface, &rc, AltUnselColour, POINT_SIZE_BORDER);

	sdl_WindowText(win, AltUnselColour, win->width / 2
		- 5 * PopUp.font.width, POINT_SIZE_BORDER + POINT_SIZE_MARGIN,
		"Point Size");
	sdl_WindowText(win, AltUnselColour, 12 * PopUp.font.width
		+ POINT_SIZE_BORDER + POINT_SIZE_MARGIN,
		POINT_SIZE_BORDER + POINT_SIZE_MARGIN + PopUp.font.height + 6,
		format("%d pt", new_font.size));
}

/*
 * For this panel, buttons are enabled or disabled based on the state of the
 * point size selection.  Don't use sdl_ButtonVisible() to do that because
 * clicks in invisible buttons aren't handled and end up dismissing the panel.
 * That seems likely to frustrate uses, so alter the coloring of the buttons
 * instead while leaving the "visible" field of the button true.
 */
static void ActivatePointSize(sdl_Button *sender)
{
	int left = sender->pos.x + sender->owner->window->left;
	int top = sender->pos.y + sender->owner->window->top;
	int height = 4 * (PopUp.font.height + 2) + 4 + 2 * (POINT_SIZE_BORDER
		+ POINT_SIZE_MARGIN);
	int width = ((int) strlen(format("%d pt", MAX_POINT_SIZE)) + 24)
		* PopUp.font.width + 2 * (POINT_SIZE_BORDER
		+ POINT_SIZE_MARGIN);
	sdl_Button *button;

	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = left;
	PopUp.top = top;
	PopUp.draw_extra = DrawPointSize;

	PointSizeBigDec = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeBigDec);
	if (new_font.size > MIN_POINT_SIZE) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "--");
	sdl_ButtonMove(button, POINT_SIZE_BORDER + POINT_SIZE_MARGIN,
		POINT_SIZE_BORDER + POINT_SIZE_MARGIN + PopUp.font.height + 6);
	button->tag = -10;
	button->activate = ChangePointSize;

	PointSizeDec = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeDec);
	if (new_font.size > MIN_POINT_SIZE) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, " -");
	sdl_ButtonMove(button, 6 * PopUp.font.width + POINT_SIZE_BORDER
		+ POINT_SIZE_MARGIN, POINT_SIZE_BORDER + POINT_SIZE_MARGIN
		+ PopUp.font.height + 6);
	button->tag = -1;
	button->activate = ChangePointSize;

	PointSizeInc = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeInc);
	if (new_font.size < MAX_POINT_SIZE) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, " +");
	sdl_ButtonMove(button, width - 10 * PopUp.font.width
		- POINT_SIZE_BORDER - POINT_SIZE_MARGIN, POINT_SIZE_BORDER
		+ POINT_SIZE_MARGIN + PopUp.font.height + 6);
	button->tag = 1;
	button->activate = ChangePointSize;

	PointSizeBigInc = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeBigInc);
	if (new_font.size < MAX_POINT_SIZE) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "++");
	sdl_ButtonMove(button, width - 4 * PopUp.font.width
		- POINT_SIZE_BORDER - POINT_SIZE_MARGIN, POINT_SIZE_BORDER
		+ POINT_SIZE_MARGIN + PopUp.font.height + 6);
	button->tag = 10;
	button->activate = ChangePointSize;

	PointSizeOk = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeOk);
	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 8 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "OK");
	sdl_ButtonMove(button, width / 2 - 10 * PopUp.font.width,
		height - PopUp.font.height - 2 - POINT_SIZE_BORDER
		- POINT_SIZE_MARGIN);
	button->activate = AcceptPointSize;

	PointSizeCancel = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, PointSizeCancel);
	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 8 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "Cancel");
	sdl_ButtonMove(button, width / 2 + 2 * PopUp.font.width,
		height - PopUp.font.height - 2 - POINT_SIZE_BORDER
		- POINT_SIZE_MARGIN);
	button->activate = CancelPointSize;

	popped = true;
}

static void SelectPresetScalableFont(sdl_Button *sender)
{
	term_window *window = &windows[SelectedTerm];

	RemovePopUp();
	string_free(new_font.alloc_name);
	new_font.alloc_name = string_make(sender->caption);
	new_font.name = new_font.alloc_name;
	new_font.size = (window->req_font.size > 0) ?
		window->req_font.size : DEFAULT_POINT_SIZE;
	new_font.preset = true;
	new_font.bitmapped = false;
	ActivatePointSize(sender);
}

static void AlterNonPresetFontSize(sdl_Button *sender)
{
	term_window *window = &windows[SelectedTerm];

	assert(!window->req_font.preset);
	RemovePopUp();
	if (!window->req_font.bitmapped) {
		assert(window->req_font.size >= MIN_POINT_SIZE
			&& window->req_font.size <= MAX_POINT_SIZE);
		string_free(new_font.alloc_name);
		new_font.alloc_name = string_make(window->req_font.name);
		new_font.name = new_font.alloc_name;
		new_font.size = window->req_font.size;
		new_font.preset = false;
		new_font.bitmapped = false;
		ActivatePointSize(sender);
	}
}

static void HelpFontBrowserClose(void)
{
	size_t i;

	RemovePopUp();

	/* Remember the directory where the browser was. */
	string_free(FontBrowserLastDir);
	FontBrowserLastDir = FontBrowserCurDir;
	FontBrowserLastRootSz = FontBrowserRootSz;
	FontBrowserCurDir = NULL;
	FontBrowserRootSz = 0;

	/* Clear the other state that doesn't need to be retained. */
	for (i = 0; i < FontBrowserDirCount; ++i) {
		string_free(FontBrowserDirEntries[i]);
	}
	mem_free(FontBrowserDirEntries);
	FontBrowserDirEntries = NULL;
	FontBrowserDirCount = 0;
	FontBrowserDirAlloc = 0;
	FontBrowserDirPage = 0;
	for (i = 0; i < FontBrowserFileCount; ++i) {
		string_free(FontBrowserFileEntries[i]);
	}
	mem_free(FontBrowserFileEntries);
	FontBrowserFileEntries = NULL;
	FontBrowserFileCount = 0;
	FontBrowserFileAlloc = 0;
	FontBrowserFilePage = 0;
	FontBrowserFileCur = (size_t) -1;
	if (FontBrowserPreviewFont) {
		sdl_FontFree(FontBrowserPreviewFont);
		mem_free(FontBrowserPreviewFont);
		FontBrowserPreviewFont = NULL;
	}
	string_free(new_font.alloc_name);
	new_font.name = NULL;
	new_font.alloc_name = NULL;
	new_font.size = 0;
	new_font.preset = false;
	new_font.bitmapped = false;
}

static void AcceptFontBrowser(sdl_Button *sender)
{
	if (FontBrowserPreviewFont) {
		term_window *window = &windows[SelectedTerm];

		sdl_FontFree(&window->font);
		string_free(window->req_font.alloc_name);
		assert(new_font.name);
		window->req_font.alloc_name = string_make(new_font.name);
		window->req_font.name = window->req_font.alloc_name;
		window->req_font.size = new_font.size;
		window->req_font.preset = new_font.preset;
		window->req_font.bitmapped = new_font.bitmapped;
		HelpWindowFontChange(window);
	}
	HelpFontBrowserClose();
}

/**
 * Name sorting function
 */
static int cmp_name(const void *f1, const void *f2)
{
	const char *name1 = *(const char**)f1;
	const char *name2 = *(const char**)f2;

	return strcmp(name1, name2);
}

static void ChangeDirPageFontBrowser(sdl_Button *sender)
{
	/*
	 * Each page overlaps by one entry with the previous; thus the
	 * use of FONT_BROWSER_PAGE_ENTRIES - 1 in the lines below.
	 * If the count is greater than zero, max_page satisfies
	 * max_page * (FONT_BROWSER_PAGE_ENTRIES - 1) < count and
	 * max_page * (FONT_BROWSER_PAGE_ENTRIES - 1)
	 * + FONT_BROWSER_PAGE_ENTRIES >= count.
	 */
	size_t max_page = (FontBrowserDirCount > 0) ?
		(FontBrowserDirCount - 1) / (FONT_BROWSER_PAGE_ENTRIES - 1) : 0;
	sdl_Button *button;
	size_t page_start, i;

	if ((FontBrowserDirPage == 0 && sender->tag < 0) ||
			(FontBrowserDirPage == max_page && sender->tag > 0)) {
		/* Scrolling that direction does nothing. */
		return;
	}

	FontBrowserDirPage = MAX(0, MIN(max_page, FontBrowserDirPage
		+ sender->tag));

	/* Redo the captions and colours of the directory buttons. */
	page_start = FontBrowserDirPage * (FONT_BROWSER_PAGE_ENTRIES - 1);
	for (i = 0; i < FONT_BROWSER_PAGE_ENTRIES; ++i) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserDirectories[i]);
		if (i + page_start < FontBrowserDirCount) {
			/*
			 * Truncate displayed name to at most
			 * FONT_BROWSER_DIR_LENGTH Unicode code points.
			 */
			char *caption;

			assert(FontBrowserDirEntries
				&& FontBrowserDirEntries[i + page_start]);
			caption = string_make(
				FontBrowserDirEntries[i + page_start]);
			utf8_clipto(caption, FONT_BROWSER_DIR_LENGTH);
			sdl_ButtonCaption(button, caption);
			string_free(caption);
			button->unsel_colour = AltUnselColour;
			button->sel_colour = AltSelColour;
			button->cap_colour = DefaultCapColour;
		} else {
			sdl_ButtonCaption(button, "");
			button->unsel_colour = back_colour;
			button->sel_colour = back_colour;
			button->cap_colour = back_colour;
		}
	}

	/* Update the colors of the scroll controls if relevant. */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirPageBefore);
	if (FontBrowserDirPage > 0) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirPageAfter);
	if (FontBrowserDirPage < max_page) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}

	PopUp.need_update = true;
}

static void ChangeFilePageFontBrowser(sdl_Button *sender)
{
	/*
	 * Each page overlaps by one entry with the previous; thus the
	 * use of FONT_BROWSER_PAGE_ENTRIES - 1 in the lines below.
	 * If the count is greater than zero, max_page satisfies
	 * max_page * (FONT_BROWSER_PAGE_ENTRIES - 1) < count and
	 * max_page * (FONT_BROWSER_PAGE_ENTRIES - 1)
	 * + FONT_BROWSER_PAGE_ENTRIES >= count.
	 */
	size_t max_page = (FontBrowserFileCount > 0) ? (FontBrowserFileCount
		- 1) / (FONT_BROWSER_PAGE_ENTRIES - 1) : 0;
	sdl_Button *button;
	size_t page_start, i;

	if ((FontBrowserFilePage == 0 && sender->tag < 0) ||
			(FontBrowserFilePage == max_page && sender->tag > 0)) {
		/* Scrolling that direction does nothing. */
		return;
	}

	FontBrowserFilePage = MAX(0, MIN(max_page, FontBrowserFilePage
		+ sender->tag));

	/* Redo the captions and colours of the file buttons. */
	page_start = FontBrowserFilePage * (FONT_BROWSER_PAGE_ENTRIES - 1);
	for (i = 0; i < FONT_BROWSER_PAGE_ENTRIES; ++i) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserFiles[i]);
		if (i + page_start < FontBrowserFileCount) {
			/*
			 * Truncate displayed name to at most
			 * FONT_BROWSER_FILE_LENGTH characters.
			 */
			char *caption;

			assert(FontBrowserFileEntries &&
				FontBrowserFileEntries[i + page_start]);
			caption = string_make(
				FontBrowserFileEntries[i + page_start]);
			utf8_clipto(caption, FONT_BROWSER_FILE_LENGTH);
			sdl_ButtonCaption(button, caption);
			string_free(caption);
			button->unsel_colour = AltUnselColour;
			button->sel_colour = AltSelColour;
			if (i + page_start == FontBrowserFileCur) {
				button->cap_colour = AltCapColour;
			} else {
				button->cap_colour = DefaultCapColour;
			}
		} else {
			sdl_ButtonCaption(button, "");
			button->unsel_colour = back_colour;
			button->sel_colour = back_colour;
			button->cap_colour = back_colour;
		}
	}

	/* Change the colors on the scroll controls if relevant. */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserFilePageBefore);
	if (FontBrowserFilePage > 0) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserFilePageAfter);
	if (FontBrowserFilePage < max_page) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}

	PopUp.need_update = true;
}

static void RefreshFontBrowser(sdl_Button *sender)
{
	char *oldcur;
	ang_dir *dir;
	size_t i;
	char file_part[1024], full_path[1024];

	/*
	 * If there is a currently selected file, remember it so, if it
	 * is still present, it can be used as the current selection.
	 */
	if (FontBrowserFileCur != (size_t) -1) {
		assert(FontBrowserFileEntries
			&& FontBrowserFileCur < FontBrowserFileCount
			&& FontBrowserFileEntries[FontBrowserFileCur]);
		oldcur = FontBrowserFileEntries[FontBrowserFileCur];
		FontBrowserFileEntries[FontBrowserFileCur] = NULL;
		FontBrowserFileCur = (size_t) -1;
	} else {
		oldcur = NULL;
	}

	/* Clear the old entries in the lists. */
	for (i = 0; i < FontBrowserDirCount; ++i) {
		string_free(FontBrowserDirEntries[i]);
		FontBrowserDirEntries[i] = NULL;
	}
	FontBrowserDirCount = 0;
	FontBrowserDirPage = 0;
	for (i = 0; i < FontBrowserFileCount; ++i) {
		string_free(FontBrowserFileEntries[i]);
		FontBrowserFileEntries[i] = NULL;
	}
	FontBrowserFileCount = 0;
	FontBrowserFilePage = 0;

	/*
	 * Build up the lists of directories and fixed-width font files
	 * present.
	 */
	assert(FontBrowserCurDir);
	dir = my_dopen(FontBrowserCurDir);
	if (!dir) {
		size_t sz1, sz2;
		int nresult;

		/* Try ANGBAND_DIR_FONTS instead. */
		dir = my_dopen(ANGBAND_DIR_FONTS);
		if (dir == NULL) {
			quit_fmt("could not read the directories %s and %s",
				FontBrowserCurDir, ANGBAND_DIR_FONTS);
		}
		mem_free(FontBrowserCurDir);
		/*
		 * Normalize the path (make it absolute with no relative
		 * parts and no redundant path separators).  That simplifies
		 * how the font browser works up the directory tree.
		 */
		sz1 = 1024;
		FontBrowserCurDir = mem_alloc(sz1);
		nresult = path_normalize(FontBrowserCurDir, sz1,
			ANGBAND_DIR_FONTS, true, &sz2, &FontBrowserRootSz);
		if (nresult == 1) {
			/* Try again with a buffer that should hold it. */
			assert(sz2 > sz1);
			FontBrowserCurDir = mem_realloc(FontBrowserCurDir, sz2);
			nresult = path_normalize(FontBrowserCurDir, sz2,
				ANGBAND_DIR_FONTS, true, NULL,
				&FontBrowserRootSz);
		}
		if (nresult != 0) {
			/* Could not normalize it. */
			quit_fmt("could not normalize %s", ANGBAND_DIR_FONTS);
		}
		dir = my_dopen(FontBrowserCurDir);
		if (!dir) {
			quit_fmt("could not open the directory, %s",
				FontBrowserCurDir);
		}
	}
	alter_ang_dir_only_files(dir, false);
	while (my_dread(dir, file_part, sizeof(file_part))) {
		path_build(full_path, sizeof(full_path), FontBrowserCurDir,
			file_part);
		if (dir_exists(full_path)) {
			/*
			 * It's a directory.  If it's readable and not "."
			 * nor "..", put it in the directory list.
			 */
			if (!streq(file_part, ".") && !streq(file_part, "..")) {
				ang_dir *dir2 = my_dopen(full_path);

				if (dir2) {
					my_dclose(dir2);
					if (FontBrowserDirCount
							== FontBrowserDirAlloc) {
						if (FontBrowserDirAlloc
								> ((size_t) -1) / (2 * sizeof(char*))) {
							/*
							 * There's too many
							 * entries to represent
							 * in memory.
							 */
							continue;
						}
						FontBrowserDirAlloc =
							(FontBrowserDirAlloc == 0) ?
							16 : 2 * FontBrowserDirAlloc;
						FontBrowserDirEntries =
							mem_realloc(FontBrowserDirEntries,
							FontBrowserDirAlloc * sizeof(char*));
					}
					FontBrowserDirEntries[FontBrowserDirCount] =
						string_make(file_part);
					++FontBrowserDirCount;
				}
			}
		} else if (is_font_file(full_path)) {
			/* Put it in the file list. */
			if (FontBrowserFileCount == FontBrowserFileAlloc) {
				if (FontBrowserFileAlloc
						> ((size_t) -1) / (2 * sizeof(char*))) {
					/*
					 * There's too many entries to represent
					 * in memory.
					 */
					continue;
				}
				FontBrowserFileAlloc =
					(FontBrowserFileAlloc == 0) ?
					16 : 2 * FontBrowserFileAlloc;
				FontBrowserFileEntries = mem_realloc(
					FontBrowserFileEntries,
					FontBrowserFileAlloc * sizeof(char*));
			}
			FontBrowserFileEntries[FontBrowserFileCount] =
				string_make(file_part);
			++FontBrowserFileCount;
		}
	}
	my_dclose(dir);

	/* Sort the entries. */
	if (FontBrowserDirCount > 0) {
		sort(FontBrowserDirEntries, FontBrowserDirCount,
			sizeof(FontBrowserDirEntries[0]), cmp_name);
	}
	if (FontBrowserFileCount > 0) {
		sort(FontBrowserFileEntries, FontBrowserFileCount,
			sizeof(FontBrowserFileEntries[0]), cmp_name);
	}

	/* Update what's shown. */
	if (oldcur) {
		i = 0;
		while (1) {
			if (i == FontBrowserFileCount) {
				sdl_Button *button;

				/*
				 * Didn't find the old selection.  Clear
				 * new_font and grey out the point size
				 * controls.
				 */
				string_free(new_font.alloc_name);
				new_font.name = NULL;
				new_font.alloc_name = NULL;
				new_font.size = 0;
				new_font.preset = false;
				new_font.bitmapped = false;
				if (FontBrowserPreviewFont) {
					sdl_FontFree(FontBrowserPreviewFont);
					mem_free(FontBrowserPreviewFont);
					FontBrowserPreviewFont = NULL;
				}
				button = sdl_ButtonBankGet(&PopUp.buttons,
					FontBrowserPtSizeBigDec);
				button->unsel_colour = back_colour;
				button->sel_colour = back_colour;
				button->cap_colour = back_colour;
				button = sdl_ButtonBankGet(&PopUp.buttons,
					FontBrowserPtSizeDec);
				button->unsel_colour = back_colour;
				button->sel_colour = back_colour;
				button->cap_colour = back_colour;
				button = sdl_ButtonBankGet(&PopUp.buttons,
					FontBrowserPtSizeInc);
				button->unsel_colour = back_colour;
				button->sel_colour = back_colour;
				button->cap_colour = back_colour;
				button = sdl_ButtonBankGet(&PopUp.buttons,
					FontBrowserPtSizeBigInc);
				button->unsel_colour = back_colour;
				button->sel_colour = back_colour;
				button->cap_colour = back_colour;
				PopUp.need_update = true;
				break;
			}
			if (streq(FontBrowserFileEntries[i], oldcur)) {
				/*
				 * Found the old selection.  Mark it and
				 * make sure it's in the current page
				 * displayed.  The details in new_font,
				 * FontBrowserPreviewFont, and the status
				 * of the point size controls have not changed
				 * so they're left as they are.
				 */
				FontBrowserFileCur = i;
				FontBrowserFilePage =
					i / (FONT_BROWSER_PAGE_ENTRIES - 1);
				break;
			}
			++i;
		}
		string_free(oldcur);
	}
	ChangeDirPageFontBrowser(sender);
	ChangeFilePageFontBrowser(sender);
}

static void CancelFontBrowser(sdl_Button *sender)
{
	HelpFontBrowserClose();
}

static void GoUpFontBrowser(sdl_Button *sender)
{
	size_t idx = strlen(FontBrowserCurDir);
	sdl_Button *button;

	assert(idx >= 1 && FontBrowserCurDir[idx - 1] == PATH_SEPC);
	if (idx <= FontBrowserRootSz) {
		/* Can't go up any further. */
		return;
	}

	/* Forget the font that was being previewed. */
	FontBrowserFileCur = (size_t) -1;
	if (FontBrowserPreviewFont) {
		sdl_FontFree(FontBrowserPreviewFont);
		mem_free(FontBrowserPreviewFont);
		FontBrowserPreviewFont = NULL;
	}
	string_free(new_font.alloc_name);
	new_font.name = NULL;
	new_font.alloc_name = NULL;
	new_font.size = 0;
	new_font.preset = false;
	new_font.bitmapped = false;

	/*
	 * Set up the path to the new directory.  First strip off the trailing
	 * path separator.
	 */
	FontBrowserCurDir[idx - 1] = '\0';
	/*
	 * Then drop the last path component, leaving a trailing path
	 * separator.
	 */
	idx = path_filename_index(FontBrowserCurDir);
	assert(idx >= FontBrowserRootSz);
	FontBrowserCurDir[idx] = '\0';

	/* Can a path element be removed and still leave something? */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirUp);
	if (idx > FontBrowserRootSz) {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}

	/*
	 * Because there's no longer a selected font, grey out the point size
	 * controls.
	 */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigDec);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeDec);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeInc);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigInc);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;

	/* Refresh the lists of files and directories. */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserRefresh);
	RefreshFontBrowser(button);

	PopUp.need_update = true;
}

static void SelectDirFontBrowser(sdl_Button *sender)
{
	sdl_Button *button;
	char *full_path;
	size_t sz, page_start = FontBrowserDirPage
		* (FONT_BROWSER_PAGE_ENTRIES - 1);

	assert(sender->tag >= 0 && sender->tag < FONT_BROWSER_PAGE_ENTRIES);
	if (page_start + sender->tag >= FontBrowserDirCount) {
		/* Directory entry is past the end; do nothing. */
		return;
	}

	/* Forget the font that was being previewed. */
	FontBrowserFileCur = (size_t) -1;
	if (FontBrowserPreviewFont) {
		sdl_FontFree(FontBrowserPreviewFont);
		mem_free(FontBrowserPreviewFont);
		FontBrowserPreviewFont = NULL;
	}
	string_free(new_font.alloc_name);
	new_font.name = NULL;
	new_font.alloc_name = NULL;
	new_font.size = 0;
	new_font.preset = false;
	new_font.bitmapped = false;

	/* Set up the path to the new directory. */
	sz = strlen(FontBrowserCurDir);
	assert(FontBrowserCurDir && sz >= 1
		&& FontBrowserCurDir[sz - 1] == PATH_SEPC);
	assert(FontBrowserDirEntries
		&& FontBrowserDirEntries[page_start + sender->tag]);
	sz += strlen(FontBrowserDirEntries[page_start + sender->tag]) + 2;
	full_path = mem_alloc(sz);
	strnfmt(full_path, sz, "%s%s%c", FontBrowserCurDir,
		FontBrowserDirEntries[page_start + sender->tag], PATH_SEPC);
	mem_free(FontBrowserCurDir);
	FontBrowserCurDir = full_path;

	/*
	 * Having drilled down into a directory, it'll be possible to go back
	 * up.
	 */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirUp);
	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	button->cap_colour = DefaultCapColour;

	/*
	 * Because there's no longer a selected font, grey out the point size
	 * controls.
	 */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigDec);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeDec);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeInc);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigInc);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;

	/* Refresh the lists of files and directories. */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserRefresh);
	RefreshFontBrowser(button);

	PopUp.need_update = true;
}

static void SelectFileFontBrowser(sdl_Button *sender)
{
	sdl_Button *button;
	char *work;
	size_t sz1, sz2, page_start = FontBrowserFilePage
		* (FONT_BROWSER_PAGE_ENTRIES - 1);
	int nresult;

	assert(sender->tag >= 0 && sender->tag < FONT_BROWSER_PAGE_ENTRIES);
	if (page_start + sender->tag >= FontBrowserFileCount) {
		/* File entry is past the end; do nothing. */
		return;
	}

	if (page_start + sender->tag == FontBrowserFileCur) {
		/* It's the same selection as before, do nothing. */
		return;
	}

	/* Fill in some details about the new font to preview. */
	assert(FontBrowserFileEntries
		&& FontBrowserFileEntries[page_start + sender->tag]);
	string_free(new_font.alloc_name);
	/* If the font is in ANGBAND_DIR_FONTS, it is a preset font. */
	sz1 = strlen(FontBrowserCurDir)
		+ strlen(FontBrowserFileEntries[page_start + sender->tag]) + 2;
	work = mem_alloc(sz1);
	nresult = path_normalize(work, sz1, ANGBAND_DIR_FONTS, true,
		&sz2, NULL);
	if (nresult == 1) {
		/* Try again with a buffer that should hold it. */
		assert(sz2 > sz1);
		work = mem_realloc(work, sz2);
		nresult = path_normalize(work, sz2, ANGBAND_DIR_FONTS, true,
			NULL, NULL);
	}
	if (nresult == 0 && streq(FontBrowserCurDir, work)) {
		new_font.alloc_name = string_make(
			FontBrowserFileEntries[page_start + sender->tag]);
		new_font.preset = true;
		mem_free(work);
	} else {
		path_build(work, sz1, FontBrowserCurDir,
			FontBrowserFileEntries[page_start + sender->tag]);
		new_font.alloc_name = work;
		new_font.preset = false;
	}
	new_font.name = new_font.alloc_name;
	if (is_bitmapped_font(new_font.name)) {
		new_font.size = 0;
		new_font.bitmapped = true;
	} else {
		/*
		 * If were not previewing a scalable font, then either use
		 * the size of the currently selected font for the window or
		 * the default size, as the size to start with the new font.
		 * Otherwise, keep the size that was used for the previous
		 * preview.
		 */
		if (FontBrowserFileCur == (size_t) -1 || new_font.bitmapped) {
			term_window *window = &windows[SelectedTerm];

			if (!window->req_font.bitmapped
					&& window->req_font.size >= MIN_POINT_SIZE
					&& window->req_font.size <= MAX_POINT_SIZE) {
				new_font.size = window->req_font.size;
			} else {
				new_font.size = DEFAULT_POINT_SIZE;
			}
		}
		new_font.bitmapped = false;
	}

	/* Change the color on the button for the currently selected font. */
	sender->cap_colour = AltCapColour;

	/*
	 * Change the color on the button for the old file selection, if
	 * visible.
	 */
	if (FontBrowserFileCur >= page_start && FontBrowserFileCur < page_start
			+ FONT_BROWSER_PAGE_ENTRIES) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserFiles[FontBrowserFileCur - page_start]);
		button->cap_colour = DefaultCapColour;
	}

	/* Remember the selection. */
	FontBrowserFileCur = page_start + sender->tag;

	/* Change the colors on the point size controls as appropriate. */
	if (!new_font.bitmapped && new_font.size > MIN_POINT_SIZE) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigDec);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeDec);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigDec);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeDec);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	if (!new_font.bitmapped && new_font.size < MAX_POINT_SIZE) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeInc);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigInc);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeInc);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigInc);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}

	/* Try to load the newly selected font. */
	if (FontBrowserPreviewFont) {
		sdl_FontFree(FontBrowserPreviewFont);
	} else {
		FontBrowserPreviewFont =
			mem_alloc(sizeof(*FontBrowserPreviewFont));
	}
	if (sdl_FontCreate(FontBrowserPreviewFont, &new_font, PopUp.surface)) {
		/* Failed. */
		mem_free(FontBrowserPreviewFont);
		FontBrowserPreviewFont = NULL;
	}

	PopUp.need_update = true;
}

static void ChangePtSzFontBrowser(sdl_Button *sender)
{
	sdl_Button *button;

	if (FontBrowserFileCur == (size_t) -1 || new_font.bitmapped) {
		/* Size changes aren't relevant. */
		return;
	}
	if ((new_font.size == MIN_POINT_SIZE && sender->tag < 0)
			|| (new_font.size == MAX_POINT_SIZE
			&& sender->tag > 0)) {
		/* Size doesn't change. */
		return;
	}

	new_font.size = MAX(MIN_POINT_SIZE, MIN(MAX_POINT_SIZE,
		new_font.size + sender->tag));

	/* Change colors on the buttons. */
	if (new_font.size > MIN_POINT_SIZE) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigDec);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeDec);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigDec);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeDec);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}
	if (new_font.size < MAX_POINT_SIZE) {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeInc);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigInc);
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	} else {
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeInc);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserPtSizeBigInc);
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	}

	/* Try to create the preview font at the new size. */
	if (FontBrowserPreviewFont) {
		sdl_FontFree(FontBrowserPreviewFont);
	} else {
		FontBrowserPreviewFont =
			mem_alloc(sizeof(*FontBrowserPreviewFont));
	}
	if (sdl_FontCreate(FontBrowserPreviewFont, &new_font, PopUp.surface)) {
		/* Failed. */
		mem_free(FontBrowserPreviewFont);
		FontBrowserPreviewFont = NULL;
	}

	PopUp.need_update = true;
}

static void DrawFontBrowser(sdl_Window *win)
{
	int filepanel_left = FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN
		+ 2 * (FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN)
		+ (FONT_BROWSER_DIR_LENGTH + 1) * PopUp.font.width
		+ FONT_BROWSER_HOR_SPACE + 3 * PopUp.font.width
		+ 2 * PopUp.font.width;
	int subpanel_bottom = FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN
		+ PopUp.font.height + 2 + 2 * (FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN) + FONT_BROWSER_PAGE_ENTRIES
		* (PopUp.font.height + 2);
	sdl_Button *button;
	SDL_Rect rc;

	/* Draw the panel border. */
	RECT(0, 0, win->width, win->height, &rc);
	sdl_DrawBox(win->surface, &rc, AltUnselColour, FONT_BROWSER_BORDER);

	/* Draw the subpanel labels. */
	sdl_WindowText(win, AltUnselColour, FONT_BROWSER_BORDER +
		FONT_BROWSER_MARGIN, FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN,
		"Directories");
	sdl_WindowText(win, AltUnselColour, filepanel_left, FONT_BROWSER_BORDER
		+ FONT_BROWSER_MARGIN, "Fixed-width Fonts");

	/* Draw the directory subpanel border. */
	RECT(FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN, FONT_BROWSER_BORDER
		+ FONT_BROWSER_MARGIN + PopUp.font.height + 2,
		filepanel_left - 2 * PopUp.font.width - FONT_BROWSER_BORDER
		- FONT_BROWSER_MARGIN,
		subpanel_bottom - PopUp.font.height - 2 - FONT_BROWSER_BORDER
		- FONT_BROWSER_MARGIN, &rc);
	sdl_DrawBox(win->surface, &rc, AltUnselColour, FONT_BROWSER_SUB_BORDER);

	/* Draw the file subpanel border. */
	RECT(filepanel_left, FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN
		+ PopUp.font.height + 2, win->width - FONT_BROWSER_BORDER
		- FONT_BROWSER_MARGIN - filepanel_left, subpanel_bottom
		- PopUp.font.height - 2 - FONT_BROWSER_BORDER
		- FONT_BROWSER_MARGIN, &rc);
	sdl_DrawBox(win->surface, &rc, AltUnselColour, FONT_BROWSER_SUB_BORDER);

	/* Draw the point size label as appropriate. */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigDec);
	if (FontBrowserFileCur != (size_t) -1 && !new_font.bitmapped) {
		assert(new_font.size >= MIN_POINT_SIZE
			&& new_font.size <= MAX_POINT_SIZE);
		sdl_WindowText(win, AltUnselColour, button->pos.x
			+ 12 * PopUp.font.width, button->pos.y + 1,
			format("%d pt", new_font.size));
	} else {
		int n = (int) strlen(format("%d pt", MAX_POINT_SIZE));

		RECT(button->pos.x + 12 * PopUp.font.width, button->pos.y + 1,
			n * PopUp.font.width, PopUp.font.height, &rc);
		SDL_FillRect(win->surface, &rc, back_pixel_colour);
	}

	if (FontBrowserPreviewFont) {
		/*
		 * Redraw the preview area.  Like sdl_FontDraw() but do not
		 * use the font associated with the window.  Include x07 in
		 * the preview to see what glyph is there for the code page
		 * 437 centered dot.  Include the UTF-8 sequence xC2 xB7 to
		 * see what glyph is there for U+00B7, the Unicode centered
		 * dot.
		 */
		const char *preview_contents[5] = {
			"abcdefghijklmnopqrst",
			"uvwxyz1234567890-=,.",
			"ABCDEFGHIJKLMNOPQRST",
			"UVWXYZ!@#$%^&*()_+<>",
			"/?;:'\"[{]}\\|`~\x07\xC2\xB7    "
		};
		int preview_bottom, i;

		if (SDL_MUSTLOCK(win->surface)) {
			if (SDL_LockSurface(win->surface) < 0) {
				return;
			}
		}

		if (20 * FontBrowserPreviewFont->width
				> win->width - 2 * (FONT_BROWSER_BORDER
				+ FONT_BROWSER_MARGIN)) {
			/* Doesn't fit horizontally.  Clip. */
			rc.x = FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN;
			rc.w = win->width - 2 * (FONT_BROWSER_BORDER
				+ FONT_BROWSER_MARGIN);
		} else {
			rc.x = (win->width - 20
				* FontBrowserPreviewFont->width) / 2;
			rc.w = 20 * FontBrowserPreviewFont->width;
		}
		rc.y = subpanel_bottom + FONT_BROWSER_VER_SPACE
			+ PopUp.font.height + 2 + FONT_BROWSER_VER_SPACE;
		assert(rc.y < win->height - FONT_BROWSER_PREVIEW_HEIGHT);
		preview_bottom = rc.y + FONT_BROWSER_PREVIEW_HEIGHT;
		if (FontBrowserPreviewFont->height
				> FONT_BROWSER_PREVIEW_HEIGHT) {
			/* One row doesn't fit vertically.  Clip. */
			rc.h = FONT_BROWSER_PREVIEW_HEIGHT;
		} else {
			rc.h = FontBrowserPreviewFont->height;
		}
		for (i = 0; i < 5; ++i) {
			SDL_Surface *text = TTF_RenderUTF8_Solid(
				FontBrowserPreviewFont->sdl_font,
				preview_contents[i], AltUnselColour);

			if (text) {
				SDL_BlitSurface(text, NULL, PopUp.surface, &rc);
				SDL_FreeSurface(text);
			}
			rc.y += FontBrowserPreviewFont->height;
			if (rc.y >= preview_bottom) {
				break;
			}
			if (rc.y + FontBrowserPreviewFont->height
					> preview_bottom) {
				rc.h = preview_bottom - rc.y;
			}
		}

		if (SDL_MUSTLOCK(win->surface)) {
			SDL_UnlockSurface(win->surface);
		}
	}
}

/*
 * See the comment for ActivatePointSize() for why enabling and disabling
 * buttons is done by coloring them rather than using sdl_ButtonVisible().
 */
static void ActivateFontBrowser(sdl_Button *sender)
{
	term_window *window = &windows[SelectedTerm];
	int ptsz_width = ((int) strlen(format("%d pt", MAX_POINT_SIZE)))
		* PopUp.font.width;
	int height, width;
	int subpanel_top, subpanel_bottom;
	int dirpanel_right, filepanel_left, filepanel_right, ptsize_left;
	sdl_Button *button;
	int i;

	RemovePopUp();

	/*
	 * Clear new_font, since a new font file hasn't been selected by this
	 * use of the browser.
	 */
	FontBrowserFileCur = (size_t) -1;
	if (FontBrowserPreviewFont) {
		sdl_FontFree(FontBrowserPreviewFont);
		mem_free(FontBrowserPreviewFont);
		FontBrowserPreviewFont = NULL;
	}
	string_free(new_font.alloc_name);
	new_font.name = NULL;
	new_font.alloc_name = NULL;
	new_font.size = 0;
	new_font.preset = false;
	new_font.bitmapped = false;

	/*
	 * If the current font is not preset, use its directory as the
	 * starting point for the browser; otherwise, use the last directory
	 * browsed by the browser or, if no browsing has been done so far,
	 * ANGBAND_DIR_FONTS.
	 */
	mem_free(FontBrowserCurDir);
	FontBrowserCurDir = NULL;
	if (!window->req_font.preset && window->req_font.name) {
		size_t sz1, sz2, file_index;
		int nresult;

		/*
		 * Normalize the path.  It could have come from the preference
		 * file and may not be already normalized.
		 */
		sz1 = strlen(window->req_font.name) + 1;
		FontBrowserCurDir = mem_alloc(sz1);
		nresult = path_normalize(FontBrowserCurDir, sz1,
			window->req_font.name, false, &sz2, &FontBrowserRootSz);
		if (nresult == 1) {
			/* Try again with a buffer that should hold it. */
			assert(sz2 > sz1);
			FontBrowserCurDir = mem_realloc(FontBrowserCurDir, sz2);
			nresult = path_normalize(FontBrowserCurDir, sz2,
				window->req_font.name, false, NULL,
				&FontBrowserRootSz);
		}
		if (nresult == 0) {
			/*
			 * Terminate just past the last directory separator in
			 * the path name.
			 */
			file_index = path_filename_index(FontBrowserCurDir);
			assert(file_index >= FontBrowserRootSz);
			FontBrowserCurDir[file_index] = '\0';
		} else {
			mem_free(FontBrowserCurDir);
			FontBrowserCurDir = NULL;
		}
	} else if (FontBrowserLastDir) {
		FontBrowserCurDir = string_make(FontBrowserLastDir);
		FontBrowserRootSz = FontBrowserLastRootSz;
	}
	if (!FontBrowserCurDir) {
		size_t sz1, sz2;
		int nresult;

		sz1 = 1024;
		FontBrowserCurDir = mem_alloc(sz1);
		nresult = path_normalize(FontBrowserCurDir, sz1,
			ANGBAND_DIR_FONTS, true, &sz2, &FontBrowserRootSz);
		if (nresult == 1) {
			/* Try again with a buffer that should hold it. */
			assert(sz2 > sz1);
			FontBrowserCurDir = mem_realloc(FontBrowserCurDir, sz2);
			nresult = path_normalize(FontBrowserCurDir, sz2,
				ANGBAND_DIR_FONTS, true, NULL,
				&FontBrowserRootSz);
		}
		if (nresult != 0) {
			quit_fmt("could not normalize %s", ANGBAND_DIR_FONTS);
		}
	}

	/* Set the window for the browser panel. */
	/* First account for the outside border. */
	height = 2 * (FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN);
	width = 2 * (FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN);
	/*
	 * Then for the directory and file subpanels with their labels
	 * and borders.
	 */
	height += 2 * (FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN)
		+ PopUp.font.height + 2
		+ FONT_BROWSER_PAGE_ENTRIES * (PopUp.font.height + 2);
	width += 4 * (FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN)
		+ (FONT_BROWSER_DIR_LENGTH + 1) * PopUp.font.width
		+ (FONT_BROWSER_FILE_LENGTH + 1) * PopUp.font.width;
	/*
	 * Now account for the scroll controls and a two character wide
	 * space between the file and directory subpanels.
	 */
	width += 2 * (FONT_BROWSER_HOR_SPACE + 3 * PopUp.font.width)
		+ 2 * PopUp.font.width;
	/*
	 * Account for the point size control and its separation from the
	 * subpanels.
	 */
	height += FONT_BROWSER_VER_SPACE + PopUp.font.height + 2;
	/*
	 * Account for the preview and its separation from the point size
	 * control.
	 */
	height += FONT_BROWSER_VER_SPACE + FONT_BROWSER_PREVIEW_HEIGHT;
	/*
	 * Account for the action buttons and their one character high
	 * separation from the preview.
	 */
	height += 2 * (PopUp.font.height + 2);
	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = AppWin->w / 2 - width / 2;
	PopUp.top = StatusBar.height + 2;
	PopUp.draw_extra = DrawFontBrowser;

	/* Set buttons for the directory and file subpanels. */
	subpanel_top = FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN
		+ PopUp.font.height + 2;
	dirpanel_right = FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN
		+ 2 * (FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN)
		+ (FONT_BROWSER_DIR_LENGTH + 1) * PopUp.font.width
		+ FONT_BROWSER_HOR_SPACE + 3 * PopUp.font.width;
	FontBrowserDirUp = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirUp);
	/*
	 * Grey it out if there's not at least one element in the path that
	 * can be stripped off while still leaving something in the path.
	 */
	if (strlen(FontBrowserCurDir) <= FontBrowserRootSz) {
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
	} else {
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		button->cap_colour = DefaultCapColour;
	}
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "Up");
	sdl_ButtonMove(button, dirpanel_right - 4 * PopUp.font.width,
		subpanel_top - PopUp.font.height - 2);
	button->activate = GoUpFontBrowser;
	for (i = 0; i < FONT_BROWSER_PAGE_ENTRIES; ++i) {
		FontBrowserDirectories[i] = sdl_ButtonBankNew(&PopUp.buttons);
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserDirectories[i]);
		/* Grey it out until needed. */
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		sdl_ButtonSize(button, (FONT_BROWSER_DIR_LENGTH + 1)
			* PopUp.font.width, PopUp.font.height + 2);
		sdl_ButtonVisible(button, true);
		sdl_ButtonCaption(button, "");
		sdl_ButtonMove(button, FONT_BROWSER_BORDER
			+ FONT_BROWSER_MARGIN + FONT_BROWSER_SUB_BORDER
			+ FONT_BROWSER_SUB_MARGIN, subpanel_top
			+ FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN
			+ i * (PopUp.font.height + 2));
		button->tag = i;
		button->activate = SelectDirFontBrowser;
	}
	dirpanel_right = FONT_BROWSER_BORDER + FONT_BROWSER_MARGIN
		+ 2 * (FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN)
		+ (FONT_BROWSER_DIR_LENGTH + 1) * PopUp.font.width
		+ FONT_BROWSER_HOR_SPACE + 3 * PopUp.font.width;
	FontBrowserDirPageBefore = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirPageBefore);
	/* Grey it out until needed. */
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 3 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "-");
	sdl_ButtonMove(button, dirpanel_right - 3 * PopUp.font.width
		- FONT_BROWSER_SUB_BORDER - FONT_BROWSER_SUB_MARGIN,
		subpanel_top + FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN);
	button->tag = -1;
	button->activate = ChangeDirPageFontBrowser;
	FontBrowserDirPageAfter = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirPageAfter);
	/* Grey it out until needed. */
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 3 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "+");
	sdl_ButtonMove(button, dirpanel_right - FONT_BROWSER_SUB_BORDER
		- FONT_BROWSER_SUB_MARGIN - 3 * PopUp.font.width,
		subpanel_top + FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN + (FONT_BROWSER_PAGE_ENTRIES - 1)
		* (PopUp.font.height + 2));
	button->tag = 1;
	button->activate = ChangeDirPageFontBrowser;
	/*
	 * Have a button that does nothing between the page before and page
	 * after buttons so that stray clicks in that area don't dismiss
	 * the 'Font Browser' panel.  If the button had access to the click
	 * coordinates, it could implement jumping to a page.
	 */
	FontBrowserDirPageDummy = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserDirPageDummy);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 3 * PopUp.font.width, (FONT_BROWSER_PAGE_ENTRIES
		- 2) * (PopUp.font.height + 2));
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "");
	sdl_ButtonMove(button, dirpanel_right - FONT_BROWSER_SUB_BORDER
		- FONT_BROWSER_SUB_MARGIN - 3 * PopUp.font.width,
		subpanel_top + FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN + PopUp.font.height + 2);

	filepanel_left = dirpanel_right + 2 * PopUp.font.width;
	for (i = 0; i < FONT_BROWSER_PAGE_ENTRIES; ++i) {
		FontBrowserFiles[i] = sdl_ButtonBankNew(&PopUp.buttons);
		button = sdl_ButtonBankGet(&PopUp.buttons,
			FontBrowserFiles[i]);
		/* Grey it out until needed. */
		button->unsel_colour = back_colour;
		button->sel_colour = back_colour;
		button->cap_colour = back_colour;
		sdl_ButtonSize(button, (FONT_BROWSER_FILE_LENGTH + 1)
			* PopUp.font.width, PopUp.font.height + 2);
		sdl_ButtonVisible(button, true);
		sdl_ButtonCaption(button, "");
		sdl_ButtonMove(button, filepanel_left + FONT_BROWSER_SUB_BORDER
			+ FONT_BROWSER_SUB_MARGIN, subpanel_top
			+ FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN
			+ i * (PopUp.font.height + 2));
		button->tag = i;
		button->activate = SelectFileFontBrowser;
	}
	filepanel_right = width - FONT_BROWSER_BORDER - FONT_BROWSER_MARGIN;
	FontBrowserFilePageBefore = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserFilePageBefore);
	/* Grey it out until needed. */
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 3 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "-");
	sdl_ButtonMove(button, filepanel_right - FONT_BROWSER_SUB_BORDER
		- FONT_BROWSER_SUB_MARGIN - 3 * PopUp.font.width, subpanel_top
		+ FONT_BROWSER_SUB_BORDER + FONT_BROWSER_SUB_MARGIN);
	button->tag = -1;
	button->activate = ChangeFilePageFontBrowser;
	FontBrowserFilePageAfter = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserFilePageAfter);
	/* Grey it out until needed. */
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 3 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "+");
	sdl_ButtonMove(button, filepanel_right - FONT_BROWSER_SUB_BORDER
		- FONT_BROWSER_SUB_MARGIN - 3 * PopUp.font.width,
		subpanel_top + FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN + (FONT_BROWSER_PAGE_ENTRIES - 1)
		* (PopUp.font.height + 2));
	button->tag = 1;
	button->activate = ChangeFilePageFontBrowser;
	/* As above, this is a dummy button to absorb frequent stray clicks. */
	FontBrowserFilePageDummy = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserFilePageDummy);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 3 * PopUp.font.width, (FONT_BROWSER_PAGE_ENTRIES
		- 2) * (PopUp.font.height + 2));
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "");
	sdl_ButtonMove(button, filepanel_right - FONT_BROWSER_SUB_BORDER
		- FONT_BROWSER_SUB_MARGIN - 3 * PopUp.font.width,
		subpanel_top + FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN + PopUp.font.height + 2);

	/*
	 * Set up the point size controls, all are greyed out until needed.
	 * Center them, if possible, below the file panel.
	 */
	if (24 * PopUp.font.width + ptsz_width
			> filepanel_right - filepanel_left) {
		ptsize_left = filepanel_right - 24 * PopUp.font.width
			- ptsz_width;
	} else {
		ptsize_left = filepanel_left + (filepanel_right
			- filepanel_left - 24 * PopUp.font.width
			- ptsz_width) / 2;
	}
	subpanel_bottom = subpanel_top + 2 * (FONT_BROWSER_SUB_BORDER
		+ FONT_BROWSER_SUB_MARGIN) + FONT_BROWSER_PAGE_ENTRIES
		* (PopUp.font.height + 2);
	FontBrowserPtSizeBigDec = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigDec);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "--");
	sdl_ButtonMove(button, ptsize_left, subpanel_bottom
		+ FONT_BROWSER_VER_SPACE);
	button->tag = -10;
	button->activate = ChangePtSzFontBrowser;
	FontBrowserPtSizeDec = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeDec);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, " -");
	sdl_ButtonMove(button, ptsize_left + 6 * PopUp.font.width,
		subpanel_bottom + FONT_BROWSER_VER_SPACE);
	button->tag = -1;
	button->activate = ChangePtSzFontBrowser;
	FontBrowserPtSizeInc = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeInc);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, " +");
	sdl_ButtonMove(button, ptsize_left + 14 * PopUp.font.width
		+ ptsz_width, subpanel_bottom + FONT_BROWSER_VER_SPACE);
	button->tag = 1;
	button->activate = ChangePtSzFontBrowser;
	FontBrowserPtSizeBigInc = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserPtSizeBigInc);
	button->unsel_colour = back_colour;
	button->sel_colour = back_colour;
	button->cap_colour = back_colour;
	sdl_ButtonSize(button, 4 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "++");
	sdl_ButtonMove(button, ptsize_left + 20 * PopUp.font.width
		+ ptsz_width, subpanel_bottom + FONT_BROWSER_VER_SPACE);
	button->tag = 10;
	button->activate = ChangePtSzFontBrowser;

	FontBrowserOk = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserOk);
	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 10 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "OK");
	sdl_ButtonMove(button, width / 2 - 19 * PopUp.font.width, height
		- FONT_BROWSER_BORDER - FONT_BROWSER_MARGIN
		- PopUp.font.height - 2);
	button->activate = AcceptFontBrowser;

	/*
	 * Have a refresh button since file system change notifications are
	 * not monitored.  That way the user has a way to resync with what's
	 * in the directory without closing and reopening the browser panel
	 * or, for some changes, going to another directory and then back to
	 * the current one.
	 */
	FontBrowserRefresh = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserRefresh);
	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 10 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "Refresh");
	sdl_ButtonMove(button, width / 2 - 5 * PopUp.font.width, height
		- FONT_BROWSER_BORDER - FONT_BROWSER_MARGIN
		- PopUp.font.height - 2);
	/*
	 * Set the tag to zero so this button can be passed to
	 * ChangeDirPageFontBrowser() and ChangeFilePageFontBrowser().
	 */
	button->tag = 0;
	button->activate = RefreshFontBrowser;

	FontBrowserCancel = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserCancel);
	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 10 * PopUp.font.width, PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "Cancel");
	sdl_ButtonMove(button, width / 2 + 9 * PopUp.font.width, height
		- FONT_BROWSER_BORDER - FONT_BROWSER_MARGIN
		- PopUp.font.height - 2);
	button->activate = CancelFontBrowser;

	/*
	 * Update the browser for the directories and files in the directory
	 * being browsed.
	 */
	button = sdl_ButtonBankGet(&PopUp.buttons, FontBrowserRefresh);
	RefreshFontBrowser(button);

	popped = true;
}

static void FontActivate(sdl_Button *sender)
{
	const char *browse_label = "Other ...";
	term_window *window = &windows[SelectedTerm];
	int i, maxl = (int) strlen(browse_label);
	int extra, width, height;
	int h, b;
	sdl_Button *button;

	for (i = 0; i < num_fonts; i++) {
		size_t sl = strlen(FontList[i]);

		/*
		 * Use 49 because that is one less than the maximum size of
		 * a button's caption.
		 */
		maxl = (sl >= 49) ? 49 : MAX(maxl, (int) sl);
	}

	/*
	 * Always add an additional menu item to open a file browser to
	 * select a font file.  If not currently using a preset font, also
	 * add an entry which corresponds to that font.
	 */
	if (window->req_font.preset) {
		extra = 1;
	} else {
		extra = 2;
	}
	width = maxl * StatusBar.font.width + 20;
	height = (num_fonts + extra) * (StatusBar.font.height + 1);

	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = sender->pos.x;
	PopUp.top = sender->pos.y;

	h = PopUp.font.height;
	for (i = 0; i < num_fonts; i++) {
		b = sdl_ButtonBankNew(&PopUp.buttons);
		button = sdl_ButtonBankGet(&PopUp.buttons, b);
		sdl_ButtonSize(button, width - 2 , h);
		sdl_ButtonMove(button, 1, i * (h + 1));
		if (window->req_font.preset
				&& streq(window->req_font.name, FontList[i])) {
			button->cap_colour = AltCapColour;
		}
		sdl_ButtonCaption(button, FontList[i]);
		sdl_ButtonVisible(button, true);
		button->activate = (is_bitmapped_font(FontList[i]))
			? SelectPresetBitmappedFont : SelectPresetScalableFont;
	}

	if (extra == 2) {
		/*
		 * Add a button corresponding to the current, but not preset
		 * font.
		 */
		char caption[50];

		b = sdl_ButtonBankNew(&PopUp.buttons);
		button = sdl_ButtonBankGet(&PopUp.buttons, b);
		sdl_ButtonSize(button, width - 2 , h);
		sdl_ButtonMove(button, 1, num_fonts * (h + 1));
		button->cap_colour = AltCapColour;
		get_font_short_name(caption, sizeof(caption),
			&window->req_font);
		sdl_ButtonCaption(button, caption);
		sdl_ButtonVisible(button, true);
		button->activate = AlterNonPresetFontSize;
	}

	/* Add the button for launching the font file browser dialog. */
	b = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, b);
	sdl_ButtonSize(button, width - 2 , h);
	sdl_ButtonMove(button, 1, (num_fonts + extra - 1) * (h + 1));
	sdl_ButtonCaption(button, browse_label);
	sdl_ButtonVisible(button, true);
	button->activate = ActivateFontBrowser;

	popped = true;
}

static errr load_gfx(void);
static bool do_update = false;

static void SelectGfx(sdl_Button *sender)
{
	SelectedGfx = sender->tag;
}

static void AcceptChanges(sdl_Button *sender)
{
	sdl_Button *button;
	bool do_video_reset = false;

	if (use_graphics != SelectedGfx) {
		do_update = true;
		use_graphics = SelectedGfx;
	}

	if (use_graphics) {
		arg_graphics = true;
		load_gfx();
	} else {
		current_graphics_mode = NULL;
		arg_graphics = false;
		tile_width = 1;
		tile_height = 1;
		reset_visuals(true);
	}

	/* Invalidate all the gfx surfaces */
	if (do_update) {
		int i;
		for (i = 0; i < ANGBAND_TERM_MAX; i++) {
			term_window *win = &windows[i];
			if (win->tiles) {
				SDL_FreeSurface(win->tiles);
				win->tiles = NULL;
			}
			if (win->onebyone) {
				SDL_FreeSurface(win->onebyone);
				win->onebyone = NULL;
			}
		}
	}	

	button = sdl_ButtonBankGet(&PopUp.buttons, MoreFullscreen);

	if (button->tag != fullscreen) {
		fullscreen = !fullscreen;
		
		do_video_reset = true;
	}

	
	SetStatusButtons();

	RemovePopUp();

	if (do_update)
		if (character_dungeon) do_cmd_redraw();

	if (do_video_reset) {
		SDL_Event Event;
		
		memset(&Event, 0, sizeof(SDL_Event));
		
		Event.type = SDL_VIDEORESIZE;
		Event.resize.w = screen_w;
		Event.resize.h = screen_h;
		
		SDL_PushEvent(&Event);
	}

	do_update = false;

}

static void FlipTag(sdl_Button *sender)
{
	if (sender->tag) {
		sender->tag = 0;
		sdl_ButtonCaption(sender, "Off");
	} else {
		sender->tag = 1;
		sdl_ButtonCaption(sender, "On");
	}
}

static void SnapChange(sdl_Button *sender)
{
	SnapRange += sender->tag;
	if (SnapRange < 0) SnapRange = 0;
	if (SnapRange > 20) SnapRange = 20;
	PopUp.need_update = true;
}

static void WidthChange(sdl_Button *sender)
{
	tile_width += sender->tag;
	if (tile_width < 1) tile_width = 1;
	if (tile_width > 12) tile_width = 12;
	do_update = true;
}

static void HeightChange(sdl_Button *sender)
{
	tile_height += sender->tag;
	if (tile_height < 1) tile_height = 1;
	if (tile_height > 8) tile_height = 8;
	do_update = true;
}

static void MoreDraw(sdl_Window *win)
{
	SDL_Rect rc;
	sdl_Button *button;
	int y = 20;
	graphics_mode *mode;

	RECT(0, 0, win->width, win->height, &rc);

	/* Draw a nice box */
	sdl_DrawBox(win->surface, &rc, AltUnselColour, 5);

	/*
	 * Only allow tile multiplier changes when a tile set is used and
	 * the game is at a command prompt.  The latter avoids a multiplier
	 * change causing blank screens for in-game menus or display artifacts
	 * sometime later when the in-game menu is dismissed.
	 */
	if (SelectedGfx) {
		sdl_WindowText(win, AltUnselColour, 20, y,
			format("Tile width is %d.", tile_width));
	}
	button = sdl_ButtonBankGet(&win->buttons, MoreWidthMinus);
	if (SelectedGfx && character_generated && inkey_flag) {
		sdl_ButtonMove(button, 200, y);
		sdl_ButtonVisible(button, true);
	} else {
		sdl_ButtonVisible(button, false);
	}
	button = sdl_ButtonBankGet(&win->buttons, MoreWidthPlus);
	if (SelectedGfx && character_generated && inkey_flag) {
		sdl_ButtonMove(button, 230, y);
		sdl_ButtonVisible(button, true);
	} else {
		sdl_ButtonVisible(button, false);
	}
	if (SelectedGfx) {
		y += 20;
	}

	if (SelectedGfx) {
		sdl_WindowText(win, AltUnselColour, 20, y,
			format("Tile height is %d.", tile_height));
	}
	button = sdl_ButtonBankGet(&win->buttons, MoreHeightMinus);
	if (SelectedGfx && character_generated && inkey_flag) {
		sdl_ButtonMove(button, 200, y);
		sdl_ButtonVisible(button, true);
	} else {
		sdl_ButtonVisible(button, false);
	}
	button = sdl_ButtonBankGet(&win->buttons, MoreHeightPlus);
	if (SelectedGfx && character_generated && inkey_flag) {
		sdl_ButtonMove(button, 230, y);
		sdl_ButtonVisible(button, true);
	} else {
		sdl_ButtonVisible(button, false);
	}
	if (SelectedGfx) {
		y += 20;
	}

	sdl_WindowText(win, AltUnselColour, 20, y, "Selected Graphics:");
	if (current_graphics_mode) {
		sdl_WindowText(win, AltSelColour, 200, y,
			current_graphics_mode->menuname);
	} else {
		sdl_WindowText(win, AltSelColour, 200, y, "None");
	}
	y += 20;

	/*
	 * Only allow changes to the graphics mode when at a command prompt
	 * in game.  Could also allow while at the splash screen, but that
	 * isn't possible to test for with character_generated and
	 * character_dungeon.  In other situations, the saved screens for
	 * overlayed menus could have tile references that become outdated
	 * when the graphics mode is changed.
	 */
	if (character_generated && inkey_flag) {
		sdl_WindowText(win, AltUnselColour, 20, y,
			"Available Graphics:");
	}
	mode = graphics_modes;
	while (mode) {
		if (!mode->menuname[0]) {
			mode = mode->pNext;
			continue;
		}
		button = sdl_ButtonBankGet(&win->buttons, GfxButtons[mode->grafID]);
		if (character_generated && inkey_flag) {
			sdl_ButtonMove(button, 200, y);
			sdl_ButtonVisible(button, true);
			y += 20;
		} else {
			sdl_ButtonVisible(button, false);
		}
		mode = mode->pNext;
	} 

	button = sdl_ButtonBankGet(&win->buttons, MoreFullscreen);
	sdl_WindowText(win, AltUnselColour, 20, y, "Fullscreen is:");

	sdl_ButtonMove(button, 200, y);
	y+= 20;

	sdl_WindowText(win, AltUnselColour, 20, y,
		format("Snap range is %d.", SnapRange));
	button = sdl_ButtonBankGet(&win->buttons, MoreSnapMinus);
	sdl_ButtonMove(button, 200, y);

	button = sdl_ButtonBankGet(&win->buttons, MoreSnapPlus);
	sdl_ButtonMove(button, 230, y);
}

static void MoreActivate(sdl_Button *sender)
{
	int width = 300;
	int height = 300;
	sdl_Button *button;
	graphics_mode *mode;

	sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.req);
	PopUp.left = (AppWin->w / 2) - width / 2;
	PopUp.top = (AppWin->h / 2) - height / 2;
	PopUp.draw_extra = MoreDraw;

	MoreWidthPlus = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreWidthPlus);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 20, PopUp.font.height + 2);
	sdl_ButtonCaption(button, "+");
	button->tag = 1;
	sdl_ButtonVisible(button, true);
	button->activate = WidthChange;

	MoreWidthMinus = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreWidthMinus);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 20, PopUp.font.height + 2);
	sdl_ButtonCaption(button, "-");
	button->tag = -1;
	sdl_ButtonVisible(button, true);
	button->activate = WidthChange;

	MoreHeightPlus = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreHeightPlus);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 20, PopUp.font.height + 2);
	sdl_ButtonCaption(button, "+");
	button->tag = 1;
	sdl_ButtonVisible(button, true);
	button->activate = HeightChange;

	MoreHeightMinus = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreHeightMinus);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 20, PopUp.font.height + 2);
	sdl_ButtonCaption(button, "-");
	button->tag = -1;
	sdl_ButtonVisible(button, true);
	button->activate = HeightChange;

	SelectedGfx = use_graphics;

	mode = graphics_modes;
	while (mode) {
		if (!mode->menuname[0]) {
			mode = mode->pNext;
			continue;
		}
		GfxButtons[mode->grafID] = sdl_ButtonBankNew(&PopUp.buttons);
		button = sdl_ButtonBankGet(&PopUp.buttons, GfxButtons[mode->grafID]);
		
		button->unsel_colour = AltUnselColour;
		button->sel_colour = AltSelColour;
		sdl_ButtonSize(button, 50 , PopUp.font.height + 2);
		sdl_ButtonVisible(button, true);
		sdl_ButtonCaption(button, mode->menuname);
		button->tag = mode->grafID;
		button->activate = SelectGfx;

		mode = mode->pNext;
	} 

	MoreFullscreen = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreFullscreen);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 50 , PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, fullscreen ? "On" : "Off");
	button->tag = fullscreen;
	button->activate = FlipTag;

	MoreSnapPlus = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreSnapPlus);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 20, PopUp.font.height + 2);
	sdl_ButtonCaption(button, "+");
	button->tag = 1;
	sdl_ButtonVisible(button, true);
	button->activate = SnapChange;

	MoreSnapMinus = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreSnapMinus);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 20, PopUp.font.height + 2);
	sdl_ButtonCaption(button, "-");
	button->tag = -1;
	sdl_ButtonVisible(button, true);
	button->activate = SnapChange;

	MoreOK = sdl_ButtonBankNew(&PopUp.buttons);
	button = sdl_ButtonBankGet(&PopUp.buttons, MoreOK);

	button->unsel_colour = AltUnselColour;
	button->sel_colour = AltSelColour;
	sdl_ButtonSize(button, 50 , PopUp.font.height + 2);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "OK");
	sdl_ButtonMove(button, width / 2 - 25, height - 40);
	button->activate = AcceptChanges;

	popped = true;
}

static errr Term_xtra_sdl_clear(void);

/**
 * Make a window with size (x,y) pixels
 * Note: The actual size of the window may end up smaller.
 * This may be called when a window wants resizing,
 * is made visible, or the font has changed.
 * This function doesn't go in for heavy optimization, and doesn't need it - it
 * may initialize a few too many redraws or whatnot, but everything gets done!
 */
static void ResizeWin(term_window* win, int w, int h)
{
	/* Don't bother */
	if (!win->visible) return;

	win->border = 2;
	win->title_height = StatusHeight;

	/* No font - a new font is needed -> get dimensions */
	if (!win->font.data) {
		/* Get font dimensions */
		if (sdl_CheckFont(&win->req_font, &win->tile_wid,
				&win->tile_hgt) || win->tile_wid <= 0
				|| win->tile_hgt <= 0) {
			quit_fmt("unable to find font '%s';\n"
				"note that there are new extended font files "
				"ending in 'x' in %s;\n"
				"please check %s and edit if necessary",
				win->req_font.name, ANGBAND_DIR_FONTS,
				sdl_settings_file);
		}
	}

	/* Get the amount of columns & rows */
	win->cols = (w - (win->border * 2)) / win->tile_wid;
	win->rows = (h - win->border - win->title_height) / win->tile_hgt;

	/* Calculate the width & height */
	win->width = (win->cols * win->tile_wid) + (win->border * 2);
	win->height = (win->rows * win->tile_hgt) + win->border + win->title_height;

	/* Delete the old surface */
	if (win->surface) SDL_FreeSurface(win->surface);

	/* Create a new surface */
	win->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, win->width, win->height,
										AppWin->format->BitsPerPixel,
										AppWin->format->Rmask,
										AppWin->format->Gmask,
										AppWin->format->Bmask,
										AppWin->format->Amask);

	/* Fill it */
	SDL_FillRect(win->surface, NULL, SDL_MapRGB(AppWin->format, 160, 160, 60));

	/* Label it */
	sdl_FontDraw(&SystemFont, win->surface, back_colour, 1, 1,
				 strlen(angband_term_name[win->Term_idx]),
				 angband_term_name[win->Term_idx]);

	/* Mark the whole window for redraw */
	RECT(0, 0, win->width, win->height, &win->uRect);

	/* Create the font if we need to */
	if (!win->font.data)
		sdl_FontCreate(&win->font, &win->req_font, win->surface);

	/* This window was never visible before, or needs resizing */
	if (!angband_term[win->Term_idx]) {
		term *old = Term;
		
		/* Initialize the term data */
		term_data_link_sdl(win);
		
		/* Make it visible to angband */
		angband_term[win->Term_idx] = &win->term_data;
		
		/* Activate it */
		Term_activate((term*)&win->term_data);
		
		/* Redraw */
		Term_redraw();
		
		/* Restore */
		Term_activate(old);
	} else {
		term *old = Term;
		
		/* Activate it */
		Term_activate((term*)&win->term_data);
		
		/* Resize */
		Term_resize(win->cols, win->rows);
		
		/* Redraw */
		Term_redraw();
		
		/* Restore */
		Term_activate(old);
	}

	/* Calculate the hotspot */
	if (win->Term_idx == SelectedTerm) {
		SizingSpot.w = 10;
		SizingSpot.h = 10;
		SizingSpot.x = win->left + win->width - 10;
		SizingSpot.y = win->top + win->height - 10;
	}

	StatusBar.need_update = true;

	/* HACK - Redraw all windows */
	if (character_dungeon) do_cmd_redraw();
}


static errr load_prefs(void)
{
	char buf[1024];
	ang_file *fff;
	term_window *win;
	int i;

	/* Initialize the windows with crappy defaults! */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		win = &windows[i];
		
		/* Clear the data */
		memset(win, 0, sizeof(term_window));
		
		/* Who? */
		win->Term_idx = i;
		
		win->req_font.alloc_name = string_make(default_term_font.name);
		win->req_font.name = win->req_font.alloc_name;
		win->req_font.size = default_term_font.size;
		win->req_font.preset = default_term_font.preset;
		win->req_font.bitmapped = default_term_font.bitmapped;
		
		if (i == 0) {
			win->top = StatusHeight;
			win->width = 600;
			win->height = 380;
			win->keys = 1024;
			win->visible = true;
		} else {
			win->top = 400 + (i * 10);
			win->left = (i - 1) * 10;
			win->width = 400;
			win->height = 200;
			win->keys = 32;
			
			win->visible = false;
		}
	}

	/* Build the path */
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "sdlinit.txt");
	sdl_settings_file = string_make(buf);

	/* Open the file */
	fff = file_open(buf, MODE_READ, -1);

	/* Check it */
	if (!fff) return (1);

	/* Process the file */
	while (file_getl(fff, buf, sizeof(buf))) {
		char *s;
		if (!buf[0]) continue;
		
		if (buf[0] == '#') continue;
		
		s = strchr(buf, '=');
		s++;
		while (!isalnum(*s)) s++;
		
		if (strstr(buf, "Resolution")) {
			screen_w = atoi(s);
			s = strchr(buf, 'x');
			screen_h = atoi(s + 1);
		} else if (strstr(buf, "Fullscreen")) {
			fullscreen = atoi(s);
		} else if (strstr(buf, "Graphics")) {
			use_graphics = atoi(s);
			if (use_graphics) arg_graphics = true;
		} else if (strstr(buf, "TileWidth")) {
			tile_width = atoi(s);
		} else if (strstr(buf, "TileHeight")) {
			tile_height = atoi(s);
		} else if (strstr(buf, "Bigtile")) {
			tile_width += atoi(s);
		} else if (strstr(buf, "Dbltile")) {
			tile_width += tile_width * atoi(s);
			tile_height += tile_height * atoi(s);
		} else if (strstr(buf, "Trptile")) {
			tile_width += 2 * tile_width * atoi(s);
			tile_height += 2 * tile_height * atoi(s);
		} else if (strstr(buf, "Window")) {
			win = &windows[atoi(s)];
		} else if (strstr(buf, "Visible")) {
			win->visible = atoi(s);
		} else if (strstr(buf, "Left")) {
			win->left = atoi(s);
		} else if (strstr(buf, "Top")) {
			win->top = atoi(s);
		} else if (strstr(buf, "Width")) {
			win->width = atoi(s);
		} else if (strstr(buf, "Height")) {
			win->height = atoi(s);
		} else if (strstr(buf, "Keys")) {
			win->keys = atoi(s);
		} else if (strstr(buf, "Font")) {
			const char *garbled_msg = "garbled font entry in pref "
				"file; use the default fault instead\n";
			long fsz;
			char *se;
			int w, h;

			string_free(win->req_font.alloc_name);
			if (prefix(s, "NOTPRESET,")) {
				win->req_font.preset = false;
				fsz = strtol(s + 10, &se, 10);
				if (*se == ',') {
					if (!fsz && (fsz < MIN_POINT_SIZE
							|| fsz > MAX_POINT_SIZE)) {
						(void) fprintf(stderr,
							"invalid point size, "
							"%ld, in pref file; "
							"use the default size "
							"instead\n", fsz);
						fsz = DEFAULT_POINT_SIZE;
					}
					win->req_font.alloc_name =
						string_make(se + 1);
				} else {
					(void) fprintf(stderr, "%s",
						garbled_msg);
					win->req_font.preset =
						default_term_font.preset;
					win->req_font.alloc_name = string_make(
						default_term_font.name);
					fsz = (default_term_font.bitmapped) ?
						0 : default_term_font.size;
				}
			} else {
				win->req_font.preset = true;
				if (prefix(s, "NOTBITMAP,")) {
					fsz = strtol(s + 10, &se, 10);
					if (*se == ',') {
						if (fsz < MIN_POINT_SIZE
								|| fsz > MAX_POINT_SIZE) {
							fsz = DEFAULT_POINT_SIZE;
						}
						win->req_font.alloc_name =
							string_make(se + 1);
					} else {
						(void) fprintf(stderr, "%s",
							garbled_msg);
						win->req_font.preset =
							default_term_font.preset;
						win->req_font.alloc_name =
							string_make(
							default_term_font.name);
						fsz = (default_term_font.bitmapped) ?
							0 : default_term_font.size;
					}
				} else {
					win->req_font.alloc_name =
						string_make(s);
					fsz = 0;
				}
			}
			win->req_font.name = win->req_font.alloc_name;
			win->req_font.size = fsz;
			win->req_font.bitmapped = (fsz == 0);
			if (sdl_CheckFont(&win->req_font, &w, &h)) {
				if (streq(win->req_font.name,
						default_term_font.name)) {
					quit_fmt("could not load the default "
						"font, %s", win->req_font.name);
				}
				(void) fprintf(stderr, "unusable font "
					"file, %s, from pref file; using the "
					"default font\n", win->req_font.name);
				string_free(win->req_font.alloc_name);
				win->req_font.alloc_name = string_make(
					default_term_font.name);
				win->req_font.name = win->req_font.alloc_name;
				win->req_font.size = default_term_font.size;
				win->req_font.preset = default_term_font.preset;
				win->req_font.bitmapped =
					default_term_font.bitmapped;
			}
		}
	}

	if (screen_w < 640) screen_w = 640;
	if (screen_h < 480) screen_h = 480;

	file_close(fff);

	return (0);
}

static errr save_prefs(void)
{
	ang_file *fff;
	int i;

	/* Open the file */
	fff = file_open(sdl_settings_file, MODE_WRITE, FTYPE_TEXT);

	/* Check it */
	if (!fff) return (1);

	file_putf(fff, "Resolution = %dx%d\n", screen_w, screen_h);
	file_putf(fff, "Fullscreen = %d\n", fullscreen);
	file_putf(fff, "Graphics = %d\n", use_graphics);
	file_putf(fff, "TileWidth = %d\n\n", tile_width);
	file_putf(fff, "TileHeight = %d\n\n", tile_height);

	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		term_window *win = &windows[i];
		
		file_putf(fff, "Window = %d\n", i);
		file_putf(fff, "Visible = %d\n", (int)win->visible);
		file_putf(fff, "Left = %d\n", win->left);
		file_putf(fff, "Top = %d\n", win->top);
		file_putf(fff, "Width = %d\n", win->width);
		file_putf(fff, "Height = %d\n", win->height);
		file_putf(fff, "Keys = %d\n", win->keys);
		if (win->req_font.bitmapped) {
			file_putf(fff, "Font = %s%s\n\n",
				(win->req_font.preset) ? "" : "NOTPRESET,0,",
				win->req_font.name);
		} else {
			assert(win->req_font.size >= MIN_POINT_SIZE &&
				win->req_font.size <= MAX_POINT_SIZE);
			file_putf(fff, "Font = %s,%d,%s\n\n",
				(win->req_font.preset) ?
				"NOTBITMAP" : "NOTPRESET",
				win->req_font.size, win->req_font.name);
		}
	}	

	file_close(fff);

	/* Done */
	return (0);
}

static void set_update_rect(term_window *win, SDL_Rect *rc);

static void DrawSizeWidget(void)
{
	Uint32 colour = SDL_MapRGB(AppWin->format, 30, 160, 70);
	SDL_FillRect(AppWin, &SizingSpot, colour);
	SDL_UpdateRects(AppWin, 1, &SizingSpot);
}

static int Movingx;
static int Movingy;

/**
 * Is What within Range units of Origin
 */

#define closeto(Origin, What, Range) \
	((ABS((Origin) - (What))) < (Range))

/**
 * This function keeps the 'mouse' info up to date,
 * and reacts to mouse buttons appropriately.
 */
static void sdl_HandleMouseEvent(SDL_Event *event)
{
	term *old = Term;
	term_window *win;
	switch (event->type) {
		/* Mouse moved */
		case SDL_MOUSEMOTION:
		{
			mouse.x = event->motion.x;
			mouse.y = event->motion.y;
			win = &windows[SelectedTerm];
			
			/* We are moving or resizing a window */
			if (Moving) {
				int i;
				
				/* Move the window */
				win->left = (mouse.x - Movingx);
				win->top = (mouse.y - Movingy);
				
				/* Left bounds check */
				if (win->left < 0) {
					win->left = 0;
					Movingx = mouse.x;
				}
					
				/* Right bounds check */
				if ((win->left + win->width) > AppWin->w) {
					win->left = AppWin->w - win->width;
					Movingx = mouse.x - win->left;
				}
				
				/* Top bounds check */
				if (win->top < StatusHeight) {
					win->top = StatusHeight;
					Movingy = mouse.y - win->top;
				}
				
				/* Bottom bounds check */
				if ((win->top + win->height) > AppWin->h) {
					win->top = AppWin->h - win->height;
					Movingy = mouse.y - win->top;
				}
				
				for (i = 0; i < ANGBAND_TERM_MAX; i++) {
					term_window *snapper = &windows[i];
					
					/* Can't snap to self... */
					if (i == SelectedTerm) continue;
					
					/* Can't snap to the invisible */
					if (!snapper->visible) continue;
					
					/* Check the windows are across from each other */
					if ((snapper->top < win->top + win->height) &&
						(win->top < snapper->top + snapper->height)) {
						/* Lets try to the left... */
						if (closeto(win->left, snapper->left + snapper->width,
									SnapRange)) {
							win->left = snapper->left + snapper->width;
							Movingx = mouse.x - win->left;
						}
						/* Maybe to the right */
						if (closeto(win->left + win->width, snapper->left,
									SnapRange)) {
							win->left = snapper->left - win->width;
							Movingx = mouse.x - win->left;
						}
					}
					
					/* Check the windows are above/below each other */
					if ((snapper->left < win->left + win->width) &&
						(win->left < snapper->left + snapper->width)) {
						/* Lets try to the top... */
						if (closeto(win->top, snapper->top + snapper->height,
									SnapRange)) {
							win->top = snapper->top + snapper->height;
							Movingy = mouse.y - win->top;
						}
						/* Maybe to the bottom */
						if (closeto(win->top + win->height, snapper->top,
									SnapRange)) {
							win->top = snapper->top - win->height;
							Movingy = mouse.y - win->top;
						}
					}
					
				}
				
				/* Show on the screen */
				sdl_BlitAll();
			} else if (Sizing) {
				/* Adjust the sizing rectangle */
				SizingRect.w = win->width - win->left + (mouse.x - Movingx);
				SizingRect.h = win->height - win->top + (mouse.y - Movingy);
				
				/* XXX - The main window can't be too small */
				if (SelectedTerm == 0) {
					int minwidth = (win->tile_wid * 80) + 2 * win->border;
					int minheight = (win->tile_hgt * 24) +
						win->border + win->title_height;
					if (SizingRect.w < minwidth)
						SizingRect.w = minwidth;
					if (SizingRect.h < minheight)
						SizingRect.h = minheight;
				}
				
				/* Show on the screen */				
				sdl_BlitAll();
			} else if (!popped) {
				/* Have a look for the corner stuff */
				if (point_in(&SizingSpot, mouse.x, mouse.y)) {
					if (!Sizingshow) {
						/* Indicate the hotspot */
						Sizingshow = true;
						DrawSizeWidget();
					}
				} else if (Sizingshow) {
					SDL_Rect rc;
					Sizingshow = false;
					RECT(win->width - 10, win->height - 10, 10, 10, &rc);
					set_update_rect(win, &rc);
					sdl_BlitWin(win);
				}
			}
			break;
		}
			
		/* A button has been pressed */
		case SDL_MOUSEBUTTONDOWN:
		{
			
			sdl_Window *window;
			bool res;
			SDLMod mods;
			int button;
			int idx = sdl_LocateWin(mouse.x, mouse.y);
			
			if (event->button.button == SDL_BUTTON_LEFT) {
				bool just_gained_focus = false;
				mouse.left = 1;
				mouse.leftx = event->button.x;
				mouse.lefty = event->button.y;
				button = 1;
				
				/* Pop up window gets priority */
				if (popped) window = &PopUp; else window = &StatusBar;
				
				/* React to a button press */
				res = sdl_ButtonBankMouseDown(&window->buttons,
											  mouse.x - window->left,
											  mouse.y - window->top);
				
				/* If pop-up window active and no reaction, cancel the popup */
				if (popped && !res) {
					RemovePopUp();
					break;
				}
				
				/* Has this mouse press been handled */
				if (res) break;
				
				/* Is the mouse press in a term_window? */
				if (idx < 0) break;
				
				/* The 'focused' window has changed */
				if (idx != SelectedTerm) {
					TermFocus(idx);
					just_gained_focus = true;
				}

				/* A button press has happened on the focused term window */
				win = &windows[idx];
				
				/* Check for mouse press in the title bar... */
				if (mouse.y < win->top + win->title_height) {
					/* Let's get moving */
					Moving = true;
					
					/* BringToTop(idx); */
					
					/* Remember where we started */
					Movingx = mouse.x - win->left;
					Movingy = mouse.y - win->top;
				} else if (point_in(&SizingSpot, mouse.x, mouse.y)) {
					/* ...or the little hotspot in the bottom right corner... */					
					/* Let's get sizing */
					Sizing = true;
					
					/* Create the sizing rectangle */
					RECT(win->left, win->top, win->width, win->height,
						 &SizingRect);
					
					/* Remember where we started */
					Movingx = mouse.x - win->left;
					Movingy = mouse.y - win->top;
					
				} else if (!just_gained_focus) {
					/* ...or signal a mouse press to angband (only if the
					 * window is already focused) */
					if (win->visible) {
						/* Calculate the 'cell' coords */
						int x = (mouse.x - win->left - win->border) 
							/ win->tile_wid;
						int y = (mouse.y - win->top - win->title_height)
							/ win->tile_hgt;
						mods = SDL_GetModState();
						if (mods & KMOD_CTRL) {
							button |= 16;
						}
						if (mods & KMOD_SHIFT) {
							button |= 32;
						}
						if (mods & KMOD_ALT) {
							button |= 64;
						}
						
						/* Send the mousepress to the appropriate term */
						Term_activate(angband_term[idx]);
						Term_mousepress(x, y, button);
						Term_activate(old);
					}
				}
			} else if (event->button.button == SDL_BUTTON_RIGHT) {
				mouse.right = 1;
				mouse.rightx = event->button.x;
				mouse.righty = event->button.y;
				button = 2;

				/* Right-click always cancels the popup */
				if (popped) {
					popped = false;
				} else if (idx != -1) {
					/* Process right clicks inside a term_window */
					int x, y;
					win = &windows[idx];
					
					/* Calculate the 'cell' coords */
					x = (mouse.x - win->left - win->border) /
						win->tile_wid;
					y = (mouse.y - win->top - win->title_height) /
						win->tile_hgt;
					
					/* Bounds check */
					if ((x >= 0) && (y >= 0) && (x < win->cols) &&
						(y < win->rows)) {
						mods = SDL_GetModState();
						if (mods & KMOD_CTRL) {
							button |= 16;
						}
						if (mods & KMOD_SHIFT) {
							button |= 32;
						}
						if (mods & KMOD_ALT) {
							button |= 64;
						}
						/* Send the mousepress to the appropriate term */
						Term_activate(angband_term[idx]);
						Term_mousepress(x, y, button);
						Term_activate(old);
					}
				}
			}
			
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			/* Handle release of left button */
			if (event->button.button == SDL_BUTTON_LEFT) {
				sdl_Window *window;
				bool res;
				mouse.left = 0;
				
				/* Pop up window gets priority */
				if (popped) window = &PopUp; else window = &StatusBar;
				
				/* React to a button release */
				res = sdl_ButtonBankMouseUp(&window->buttons,
											mouse.x - window->left,
											mouse.y - window->top);
				
				/* Cancel popup */
				if (popped && !res)
					RemovePopUp();
				
				/* Finish moving */
				if (Moving) 	{
					Moving = false;
					
					/* Update */
					sdl_BlitAll();
				}
				
				/* Finish sizing */
				if (Sizing) {
					/* Sort out the window */
					ResizeWin(&windows[SelectedTerm], SizingRect.w, SizingRect.h);
					Sizing = false;
					
					/* Update */
					sdl_BlitAll();
				}
			} else if (event->button.button == SDL_BUTTON_RIGHT) {
				mouse.right = 0;
			}
			break;
		}	
			
	}
}

/**
 * Handle keypresses.
 *
 * We treat left and right modifier keys as equivalent.
 * We ignore any key without a valid SDLK index.
 */
static void sdl_keypress(SDL_keysym keysym)
{
	uint16_t key_code = keysym.unicode;
	SDLKey key_sym = keysym.sym;

	int ch = 0;

	/* Store the value of various modifier keys */
	bool mc = (keysym.mod & KMOD_CTRL) > 0;
	bool ms = (keysym.mod & KMOD_SHIFT) > 0;
	bool ma = (keysym.mod & KMOD_ALT) > 0;
	bool mm = (keysym.mod & KMOD_META) > 0;
	bool mg = (keysym.mod & KMOD_MODE) > 0;
	bool kp = false;

	uint8_t mods = (ma ? KC_MOD_ALT : 0) | (mm ? KC_MOD_META : 0);

	/* Ignore if main term is not initialized */
	if (!Term) return;

	/* Handle all other valid SDL keys */
	switch (key_sym) {
		/* keypad */
		case SDLK_KP0: ch = '0'; kp = true; break;
		case SDLK_KP1: ch = '1'; kp = true; break;
		case SDLK_KP2: ch = '2'; kp = true; break;
		case SDLK_KP3: ch = '3'; kp = true; break;
		case SDLK_KP4: ch = '4'; kp = true; break;
		case SDLK_KP5: ch = '5'; kp = true; break;
		case SDLK_KP6: ch = '6'; kp = true; break;
		case SDLK_KP7: ch = '7'; kp = true; break;
		case SDLK_KP8: ch = '8'; kp = true; break;
		case SDLK_KP9: ch = '9'; kp = true; break;
		case SDLK_KP_PERIOD: ch = '.'; kp = true; break;
		case SDLK_KP_DIVIDE: ch = '/'; kp = true; break;
		case SDLK_KP_MULTIPLY: ch = '*'; kp = true; break;
		case SDLK_KP_MINUS: ch = '-'; kp = true; break;
		case SDLK_KP_PLUS: ch = '+'; kp = true; break;
		case SDLK_KP_ENTER: ch = KC_ENTER; kp = true; break;
		case SDLK_KP_EQUALS: ch = '='; kp = true; break;

		/* have have these to get consistent ctrl-shift behaviour */
		case SDLK_0: if ((!ms || mc || ma) && !mg) ch = '0'; break;
		case SDLK_1: if ((!ms || mc || ma) && !mg) ch = '1'; break;
		case SDLK_2: if ((!ms || mc || ma) && !mg) ch = '2'; break;
		case SDLK_3: if ((!ms || mc || ma) && !mg) ch = '3'; break;
		case SDLK_4: if ((!ms || mc || ma) && !mg) ch = '4'; break;
		case SDLK_5: if ((!ms || mc || ma) && !mg) ch = '5'; break;
		case SDLK_6: if ((!ms || mc || ma) && !mg) ch = '6'; break;
		case SDLK_7: if ((!ms || mc || ma) && !mg) ch = '7'; break;
		case SDLK_8: if ((!ms || mc || ma) && !mg) ch = '8'; break;
		case SDLK_9: if ((!ms || mc || ma) && !mg) ch = '9'; break;

		case SDLK_UP: ch = ARROW_UP; break;
		case SDLK_DOWN: ch = ARROW_DOWN; break;
		case SDLK_RIGHT: ch = ARROW_RIGHT; break;
		case SDLK_LEFT: ch = ARROW_LEFT; break;

		case SDLK_INSERT: ch = KC_INSERT; break;
		case SDLK_HOME: ch = KC_HOME; break;
		case SDLK_PAGEUP: ch = KC_PGUP; break;
		case SDLK_DELETE: ch = KC_DELETE; break;
		case SDLK_END: ch = KC_END; break;
		case SDLK_PAGEDOWN: ch = KC_PGDOWN; break;
		case SDLK_ESCAPE: ch = ESCAPE; break;
		case SDLK_BACKSPACE: ch = KC_BACKSPACE; break;
		case SDLK_RETURN: ch = KC_ENTER; break;
		case SDLK_TAB: ch = KC_TAB; break;

		case SDLK_F1: ch = KC_F1; break;
		case SDLK_F2: ch = KC_F2; break;
		case SDLK_F3: ch = KC_F3; break;
		case SDLK_F4: ch = KC_F4; break;
		case SDLK_F5: ch = KC_F5; break;
		case SDLK_F6: ch = KC_F6; break;
		case SDLK_F7: ch = KC_F7; break;
		case SDLK_F8: ch = KC_F8; break;
		case SDLK_F9: ch = KC_F9; break;
		case SDLK_F10: ch = KC_F10; break;
		case SDLK_F11: ch = KC_F11; break;
		case SDLK_F12: ch = KC_F12; break;
		case SDLK_F13: ch = KC_F13; break;
		case SDLK_F14: ch = KC_F14; break;
		case SDLK_F15: ch = KC_F15; break;

		default: break;
	}

	if (ch) {
		if (kp) mods |= KC_MOD_KEYPAD;
		if (mc) mods |= KC_MOD_CONTROL;
		if (ms) mods |= KC_MOD_SHIFT;
		Term_keypress(ch, mods);
	} else if (key_code) {
		/* If the keycode is 7-bit ASCII (except numberpad) send
		 * directly to the game. */
		if (mc && (key_sym == SDLK_TAB || key_sym == SDLK_RETURN ||
				key_sym == SDLK_BACKSPACE ||
				MODS_INCLUDE_CONTROL(key_code)))
			mods |= KC_MOD_CONTROL;
		if (ms && MODS_INCLUDE_SHIFT(key_code)) mods |= KC_MOD_SHIFT;

		Term_keypress(key_code, mods);
	}
}


static void init_windows(void);
static void init_morewindows(void);

/**
 * Handle the response to a SDL_VIDEORESIZE event.
 *
 * \param e is the event whose type is SDL_VIDEORESIZE.
 */
static void sdl_VideoResize(SDL_Event *e)
{
	/* Free the surface */
	SDL_FreeSurface(AppWin);

	if (!fullscreen) {
		/* Make sure */
		vflags &= ~(SDL_FULLSCREEN);
		vflags |= SDL_RESIZABLE;

		screen_w = e->resize.w;
		screen_h = e->resize.h;

		if (screen_w < 640) screen_w = 640;
		if (screen_h < 480) screen_h = 480;

		/* Resize the application surface */
		AppWin = SDL_SetVideoMode(screen_w, screen_h, 0, vflags);
	} else {
		/* Make sure */
		vflags |= SDL_FULLSCREEN;
		vflags &= ~(SDL_RESIZABLE);

		AppWin = SDL_SetVideoMode(full_w, full_h, 0, vflags);
	}
	init_morewindows();
	init_windows();
}

/**
 * Handle a single message sent to the application.
 *
 * Functions that are either called from a separate thread or which need to
 * create a separate thread (such as sounds) need to pass messages to this
 * function in order to execute most operations.  See the useage of
 * "SDL_USEREVENT".
 */
static errr sdl_HandleEvent(SDL_Event *event)
{

	/* Handle message */
	switch (event->type)
	{
		/* Keypresses */
		case SDL_KEYDOWN:
		{
			/* Handle keypress */
			sdl_keypress(event->key.keysym);
			
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			/* Handle mouse stuff */
			sdl_HandleMouseEvent(event);
			break;
		}

		case SDL_MOUSEMOTION:
		{
			int i;  
			SDL_Event events[10];

			/*
			 * If there are a bundle of mouse movements pending,
			 * we'll just take every tenth one - this makes a
			 * simple approach to dragging practical, for instance.
			 */  
			i = SDL_PeepEvents(events, 10, SDL_GETEVENT,
							   SDL_EVENTMASK(SDL_MOUSEMOTION));
			if (i > 0) 
				*event = events[i - 1];

			/* Handle mouse stuff */
			sdl_HandleMouseEvent(event);
			break;
		}
			
		/* Shut down the game */
		case SDL_QUIT:
		{
			handle_quit(false);
			break;
		}

		/* Resize the application */
		case SDL_VIDEORESIZE:
		{
			sdl_VideoResize(event);
			break;
		}

		case WINDOW_DRAW:
		{
			/* Redraw window that have asked */
			sdl_Window *window = (sdl_Window*)event->user.data1;
			sdl_WindowBlit(window);
			break;
		}
		default:
		{
			/* Do nothing */
			break;
		}
	}	
	sdl_WindowUpdate(&StatusBar);
	sdl_WindowUpdate(&PopUp);
	return (0);
}

/**
 * Perform a modal event loop with all input directed to the PopUp window.
 *
 * \return zero if the PopUp window was dismissed normally.  Return one
 * if the PopUp window was dismissed by a SDL_QUIT event.  Return two if
 * the PopUp window was dismissed because SDL_WaitEvent() reported an error
 * with event handling.
 *
 * The following events will dismiss the PopUp window without invoking
 * a callback registered by the caller:  pressing the Escape key, pressing the
 * right mouse button, SDL_QUIT, or SDL_WaitEvent() reporting an error.
 *
 * Uses a simplified form of the event handling from sdl_HandleEvent() and its
 * children.
 */
static int sdl_ModalEventLoop(void)
{
	int result = 0;

	while (popped) {
		SDL_Event e;

		sdl_WindowUpdate(&PopUp);
		if (SDL_WaitEvent(&e)) {
			switch (e.type) {
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					/* Dismiss the dialog. */
					if (!e.key.keysym.mod) {
						RemovePopUp();
					}
					break;

				default:
					/* Do nothing. */
					break;
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT) {
					mouse.left = 1;
					mouse.leftx = e.button.x;
					mouse.lefty = e.button.y;

					(void)sdl_ButtonBankMouseDown(
						&PopUp.buttons,
						e.button.x - PopUp.left,
						e.button.y - PopUp.top);
				} else if (e.button.button
						== SDL_BUTTON_RIGHT) {
					mouse.right = 1;
					mouse.rightx = e.button.x;
					mouse.righty = e.button.y;

					RemovePopUp();
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if (e.button.button == SDL_BUTTON_LEFT) {
					mouse.left = 0;
					(void)sdl_ButtonBankMouseUp(
						&PopUp.buttons,
						e.button.x - PopUp.left,
						e.button.y - PopUp.top);
				} else if (e.button.button
						== SDL_BUTTON_RIGHT) {
					mouse.right = 0;
				}
				break;

			case SDL_MOUSEMOTION:
				{
					SDL_Event motion_events[10];
					int count;

					mouse.x = e.motion.x;
					mouse.y = e.motion.y;
					while ((count = SDL_PeepEvents(motion_events,
							10, SDL_GETEVENT,
							SDL_EVENTMASK(SDL_MOUSEMOTION)))) {
						mouse.x = motion_events[count - 1].motion.x;
						mouse.y = motion_events[count - 1].motion.y;
					}
				}
				break;

			case SDL_QUIT:
				RemovePopUp();
				result = 1;
				break;

			case SDL_VIDEORESIZE:
				sdl_VideoResize(&e);
				PopUp.left = (AppWin->w - PopUp.width) / 2;
				PopUp.top = (AppWin->h - PopUp.height) / 2;
				PopUp.need_update = true;
				break;

			case WINDOW_DRAW:
				sdl_WindowBlit((sdl_Window*)e.user.data1);
				break;

			default:
				/* Do nothing. */
				break;
			}
		} else {
			RemovePopUp();
			result = 2;
		}
	}

	return result;
}

/**
 * Update the redraw rect
 * A simple but effective way to keep track of what
 * parts of a window need to updated.
 * Any new areas that are updated before a blit are simply combined
 * into a new larger rectangle to encompass all changes.
 */
static void set_update_rect(term_window *win, SDL_Rect *rc)
{
	/* No outstanding update areas yet? */
	if (win->uRect.x == -1) {
		/* Simple copy */
		win->uRect = *rc;
	} else {
		/* Combine the old update area with the new */
		int x = MIN(win->uRect.x, rc->x);
		int y = MIN(win->uRect.y, rc->y);
		int x2 = MAX(win->uRect.x + win->uRect.w, rc->x + rc->w);
		int y2 = MAX(win->uRect.y + win->uRect.h, rc->y + rc->h);
		RECT(x, y, x2 - x, y2 - y, &win->uRect);
	}
}

/**
 * Clear a terminal window
 */
static errr Term_xtra_sdl_clear(void)
{
	term_window *win = (term_window*)(Term->data);
	SDL_Rect rc;

	/* Oops */
	if (!win->surface) return (1);

	/* Create the fill area */
	RECT(win->border, win->title_height, win->width - (2 * win->border),
		 win->height - win->border - win->title_height, &rc);

	
	/* Fill the rectangle */
	SDL_FillRect(win->surface, &rc, back_pixel_colour);

	/* Rectangle to update */
	set_update_rect(win, &rc);

	/* Success */
	return (0);
}

/**
 * Process at least one event
 */
static errr Term_xtra_sdl_event(int v)
{
	SDL_Event event;
	errr error = 0;

	/* Wait or check for an event with special casing when exiting */
	if (quit_when_ready) {
		if (inkey_flag && quit_when_ready == 1) {
			/*
			 * The game is at a command prompt and has a
			 * consistent state so it is safe to quit and exit.
			 */
			handle_quit(false);
		} else {
			/*
			 * Send an escape to satisfy whatever the game is
			 * asking for.
			 */
			Term_keypress(ESCAPE, 0);
		}
	} else if (v) {
		/* Wait in 0.02s increments while updating animations every 0.2s */
		int i = 0;
		while (!SDL_PollEvent(&event)) {
			if (i == 0) idle_update();
			usleep(20000);
			i = (i + 1) % 10;
		}

		/* Handle it */
		error = sdl_HandleEvent(&event);
	} else {
		/* Get a single pending event */
		if (SDL_PollEvent(&event)) {
			/* Handle it */
			error = sdl_HandleEvent(&event);
		}
	}

	/* Note success or failure */
	return (error);
}

/**
 * Process all pending events
 */
static errr Term_xtra_sdl_flush(void)
{
	SDL_Event event;

	/* Get all pending events */
	while (SDL_PollEvent(&event)) {
		/* Handle them (ignore errors) */
		(void)sdl_HandleEvent(&event);
	}

	/* Done */
	return (0);
}

/**
 * Delay for "x" milliseconds
 */
static errr Term_xtra_sdl_delay(int v)
{
	/* Sleep */
	if (v > 0) {
		Term_xtra_sdl_event(0);
		SDL_Delay(v);
	}

	/* Success */
	return (0);
}

static errr Term_bigcurs_sdl(int col, int row)
{
	term_window *win = (term_window*)(Term->data);

	SDL_Color colour = text_colours[COLOUR_YELLOW];

	SDL_Rect rc;

	/* Make a rectangle */
	RECT(col * win->tile_wid, row * win->tile_hgt, win->tile_wid * tile_width,
		 win->tile_hgt * tile_height, &rc);

	/* Translate it */
	rc.x += win->border;
	rc.y += win->title_height;

	/* Draw it */
	sdl_DrawBox(win->surface, &rc, colour, 1);

	/* Update area */
	set_update_rect(win, &rc);

	/* Success */
	return (0);
}

static errr Term_curs_sdl(int col, int row)
{
	term_window *win = (term_window*)(Term->data);

	SDL_Color colour = text_colours[COLOUR_YELLOW];

	SDL_Rect rc;

	/* Make a rectangle */
	RECT(col * win->tile_wid, row * win->tile_hgt, win->tile_wid,
		 win->tile_hgt, &rc);

	/* Translate it */
	rc.x += win->border;
	rc.y += win->title_height;

	/* Draw it */
	sdl_DrawBox(win->surface, &rc, colour, 1);

	/* Update area */
	set_update_rect(win, &rc);

	/* Success */
	return (0);
}

static errr Term_xtra_sdl(int n, int v)
{
	switch (n)
	{
		/* Process an event */
		case TERM_XTRA_EVENT:
		{
			return (Term_xtra_sdl_event(v));
		}
		/* Flush all events */
		case TERM_XTRA_FLUSH:
		{
			return (Term_xtra_sdl_flush());
		}
		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		{
			return (Term_xtra_sdl_clear());
		}
		/* Show or hide the cursor */
		case TERM_XTRA_SHAPE:
		{
			int x, y;
			
			/* Obtain the cursor */
			(void)Term_locate(&x, &y);
			
			/* Show or hide the cursor */
			Term_curs_sdl(x, y);
			return (0);
		}
		case TERM_XTRA_FRESH:
		{
			/* Get the current window data */
			term_window *win = (term_window*)(Term->data);
			
			/* Blat it! */
			sdl_BlitWin(win);
			
			/* Done */
			return (0);
		}
		case TERM_XTRA_DELAY:
		{
			return (Term_xtra_sdl_delay(v));
		}
		case TERM_XTRA_REACT:
		{
			int i;
			/* Re-initialize the colours */
			back_colour.r = angband_color_table[COLOUR_DARK][1];
			back_colour.g = angband_color_table[COLOUR_DARK][2];
			back_colour.b = angband_color_table[COLOUR_DARK][3];
			back_pixel_colour = SDL_MapRGB(AppWin->format,
				back_colour.r, back_colour.g, back_colour.b);
			for (i = 0; i < MAX_COLORS; i++)
			{
				text_colours[i].r = angband_color_table[i][1];
				text_colours[i].g = angband_color_table[i][2];
				text_colours[i].b = angband_color_table[i][3];
			}
		}
	}

	return (1);
}

/**
 * Erase a nc x nr block of characters whose upper left corner is at (col, row).
 */
static errr Term_wipe_sdl_helper(int col, int row, int nc, int nr)
{
	term_window *win = (term_window*)(Term->data);

	SDL_Rect rc;

	/* Build the area to black out */
	rc.x = col * win->tile_wid;
	rc.y = row * win->tile_hgt;
	rc.w = win->tile_wid * nc;
	rc.h = win->tile_hgt * nr;

	/* Translate it */
	rc.x += win->border;
	rc.y += win->title_height;

	/* Wipe it */
	SDL_FillRect(win->surface, &rc, back_pixel_colour);

	/* Update */
	set_update_rect(win, &rc);

	return (0);
}

static errr Term_wipe_sdl(int col, int row, int n)
{
	return Term_wipe_sdl_helper(col, row, n, 1);
}

/**
 * Draw some text to a window
 */
static errr Term_text_sdl(int col, int row, int n, int a, const wchar_t *s)
{
	term_window *win = (term_window*)(Term->data);
	SDL_Color colour = text_colours[a % MAX_COLORS];
	SDL_Color bg = text_colours[COLOUR_DARK];
	int x = col * win->tile_wid;
	int y = row * win->tile_hgt;
	wchar_t src[255];
	char mbstr[MB_LEN_MAX * 255];
	size_t len;

	/* Translate */
	x += win->border;
	y += win->title_height;

	/* Not much point really... */
	if (!win->visible) return (0);

	/* Clear the way */
	Term_wipe_sdl_helper(col, row, n, 1);

	/* Take a copy of the incoming string, but truncate it at n chars */
	wcsncpy(src, s, n);
	src[n] = L'\0';
	/* Convert to UTF-8 for display */
	len = wcstombs(mbstr, src, n * MB_LEN_MAX);
	mbstr[len] = '\0';

	/* Handle background */
	switch (a / MULT_BG)
	{
		case BG_BLACK:
			/* Default Background */
			break;
		case BG_SAME:
			/* Background same as foreground*/
			bg = colour;
			break;
		case BG_DARK:
			/* Highlight Background */
			bg = text_colours[COLOUR_SHADE];
			break;
	}

	/* Draw it */
	return (sdl_mapFontDraw(&win->font, win->surface, colour, bg, x, y, n,
							mbstr));
}

/**
 * Do a 'stretched blit'
 * SDL has no support for stretching... What a bastard!
 * 
 */
static void sdl_StretchBlit(SDL_Surface *src, SDL_Rect *srcRect, SDL_Surface *dest, SDL_Rect *destRect)
{
	int x, y;
	int sx, sy, dx, dy;
	Uint8 *ps, *pd;

	for (y = 0; y < destRect->h; y++) {
		for (x = 0; x < destRect->w; x++) {
			/* Actual source coords */
			sx = (srcRect->w * x / (destRect->w)) + srcRect->x;
			sy = (srcRect->h * y / (destRect->h)) + srcRect->y;
			
			/* Find a source pixel */
			ps = (Uint8 *)src->pixels + (sx * src->format->BytesPerPixel) +
				(sy * src->pitch);
#if 0
			/* Do we need to draw it? */
			switch (src->format->BytesPerPixel)
			{
				case 1:
				{
					if (*ps == src->format->colorkey) continue;
					break;
				}
				case 2:
				{
					Uint16 *ps16 = (Uint16*) ps;
					if (*ps16 == src->format->colorkey) continue;
					break;
				}
				case 3:
				case 4:
				{
					Uint32 *ps32 = (Uint32*) ps;
					if (*ps32 == src->format->colorkey) continue;
					break;
				}
			}
#endif
					
			/* Actual destination pixel coords */
			dx = x + destRect->x;
			dy = y + destRect->y;
			
			/* Destination pixel */
			pd = (Uint8 *)dest->pixels + (dx * dest->format->BytesPerPixel) +
				(dy * dest->pitch);
			
			switch (dest->format->BytesPerPixel)
			{
				case 1:
				{
					*pd = *ps;
					break;
				}
				case 2:
				{
					Uint16 *ps16 = (Uint16*) ps;
					Uint16 *pd16 = (Uint16*) pd;
					*pd16 = *ps16;
					break;
				}
				case 3:
				case 4:
				{
					Uint32 *ps32 = (Uint32*) ps;
					Uint32 *pd32 = (Uint32*) pd;
					*pd32 = *ps32;
				}
			}
			
		}
	}

}

/**
 * Make the 'pre-stretched' tiles for this window
 * Assumes the tiles surface was freed elsewhere
 */
static errr sdl_BuildTileset(term_window *win)
{
	int x, y;
	int ta, td;
	int xx, yy;
	graphics_mode *info;

	if (!GfxSurface) return (1);

	info = get_graphics_mode(use_graphics);
	if (info->grafID == 0) return (1);

	/* Calculate the number of tiles across & down*/
	ta = GfxSurface->w / info->cell_width;
	td = GfxSurface->h / info->cell_height;

	/* Calculate the size of the new surface */
	x = ta * win->tile_wid * tile_width;
	y = td * win->tile_hgt * tile_height;

	/* Make it */
	win->tiles = SDL_CreateRGBSurface(SDL_SWSURFACE, x, y,
									  GfxSurface->format->BitsPerPixel,
									  GfxSurface->format->Rmask,
									  GfxSurface->format->Gmask,
									  GfxSurface->format->Bmask,
									  GfxSurface->format->Amask);

	/* Bugger */
	if (!win->tiles) return (1);

	/* For every tile... */
	for (xx = 0; xx < ta; xx++) {
		for (yy = 0; yy < td; yy++) {
			SDL_Rect src, dest;
			int dwid = win->tile_wid * tile_width;
			int dhgt = win->tile_hgt * tile_height;

			/* Source rectangle (on GfxSurface) */
			RECT(xx * info->cell_width, yy * info->cell_height,
				 info->cell_width, info->cell_height, &src);

			/* Destination rectangle (win->tiles) */
			RECT(xx * dwid, yy * dhgt, dwid, dhgt, &dest);

			/* Do the stretch thing */
			sdl_StretchBlit(GfxSurface, &src, win->tiles, &dest);
		}
	}

	/* see if we need to make a separate surface for the map view */
	if (!((tile_width == 1) && (tile_height == 1))) {
		/* Calculate the size of the new surface */
		x = ta * win->tile_wid;
		y = td * win->tile_hgt;

		/* Make it */
		win->onebyone = SDL_CreateRGBSurface(SDL_SWSURFACE, x, y,
				GfxSurface->format->BitsPerPixel,
				GfxSurface->format->Rmask, GfxSurface->format->Gmask,
				GfxSurface->format->Bmask, GfxSurface->format->Amask);

		/* Bugger */
		if (!win->onebyone) return (1);

		/* For every tile... */
		for (xx = 0; xx < ta; xx++) {
			for (yy = 0; yy < td; yy++) {
				SDL_Rect src, dest;
				int dwid = win->tile_wid;
				int dhgt = win->tile_hgt;

				/* Source rectangle (on GfxSurface) */
				RECT(xx * info->cell_width, yy * info->cell_height,
					 info->cell_width, info->cell_height, &src);

				/* Destination rectangle (win->tiles) */
				RECT(xx * dwid, yy * dhgt, dwid, dhgt, &dest);

				/* Do the stretch thing */
				sdl_StretchBlit(GfxSurface, &src, win->onebyone, &dest);
			}
		}
	}

	return (0);
}

/**
 * Put some gfx on the screen
 * XXX - This function _never_ seems to get called with n > 1 ?
 * This needs improvement...
 */
static errr Term_pict_sdl(int col, int row, int n, const int *ap,
						  const wchar_t *cp, const int *tap, const wchar_t *tcp)
{

	/* Get the right window */
	term_window *win = (term_window*)(Term->data);

	SDL_Rect rc, src, ur;
	int i, j;
	bool haddbl = false;
	int dhrclip;

	/* First time a pict is requested we load the tileset in */
	if (!win->tiles) {
		sdl_BuildTileset(win);
		if (!win->tiles) return (1);
	}

	/*
	 * Set exclusive lower bound in y for rendering upper halves of
	 * double-height tiles.
	 */
	if (overdraw) {
		dhrclip = Term_get_first_tile_row(Term) + tile_height - 1;
	} else {
		/*
		 * There's no double-height tiles so the value does not
		 * matter.
		 */
		dhrclip = 0;
	}

	/* Make the destination rectangle */
	RECT(col * win->tile_wid, row * win->tile_hgt, win->tile_wid,
		 win->tile_hgt, &rc);

	/* Translate it */
	rc.x += win->border;
	rc.y += win->title_height;

	/* Stretch for bigtile mode */
	rc.w *= tile_width;
	rc.h *= tile_height;

	/* Get the dimensions of the graphic surface */
	src.w = rc.w;
	src.h = rc.h;

	/* Set up the bounds for what will be updated. */
	ur = rc;
	ur.w *= n;

	/* Clear the way */
	Term_wipe_sdl_helper(col, row, n * tile_width, tile_height);

	/* Blit 'em! (it) */
	for (i = 0; i < n; i++) {
		/* Get the terrain tile */
		j = (tap[i] & 0x7f);
		src.x = (tcp[i] & 0x7F) * src.w;
		src.y = j * src.h;
		
		/* if we are using overdraw, draw the top rectangle */
		if (overdraw && row > dhrclip &&
				 j >= overdraw && j <= overdraw_max) {
			src.y -= rc.h;
			rc.y -= rc.h;
			rc.h = (rc.h << 1); /* double the height */
			src.h = rc.h;
			SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
			rc.h = (rc.h >> 1); /* halve the height */
			rc.y += rc.h;
			haddbl = true;
		} else
			SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
		
		/* If foreground is the same as background, we're done */
		if ((tap[i] == ap[i]) && (tcp[i] == cp[i])) continue;
		
		/* Get the foreground tile */
		j = (ap[i] & 0x7f);
		src.x = (cp[i] & 0x7F) * src.w;
		src.y = j * src.h;
		
		/* if we are using overdraw, draw the top rectangle */
		if (overdraw && row > dhrclip &&
				j >= overdraw && j <= overdraw_max) {
			src.y -= rc.h;
			rc.y -= rc.h;
			rc.h = (rc.h << 1); /* double the height */
			src.h = rc.h;
			SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
			rc.h = (rc.h >> 1); /* halve the height */
			rc.y += rc.h;
			haddbl = true;
		} else
			SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
	}

	/* Update area */
	if (haddbl) {
		ur.y -= ur.h;
		ur.h *= 2;
	}
	set_update_rect(win, &ur);

	return (0);
}

static void Term_view_map_sdl(term *t)
{
	SDL_Surface *fulltiles = NULL;
	/* Get the right window */
	term_window *win = (term_window*)(t->data);

	/* First time a pict is requested we load the tileset in */
	if (!win->tiles)
		sdl_BuildTileset(win);

	/* Override large tiles with small ones for the map view */
	if (win->onebyone) {
		/* Save screen so we can load it again after the tile image
		 * is restored */
		screen_save();

		fulltiles = win->tiles;
		win->tiles = win->onebyone;
		SDL_FreeSurface(fulltiles);
		win->onebyone = NULL;
	}

	t->view_map_hook = NULL;
	do_cmd_view_map();
	t->view_map_hook = Term_view_map_sdl;

	/* Swap back */
	if (fulltiles) {
		/* Free everything and rebuild the tileset */
		if (win->tiles) {
			SDL_FreeSurface(win->tiles);
			win->tiles = NULL;
		}
		sdl_BuildTileset(win);

		/* Load screen with the correct tiles - the screen load in the
		 * view map command was still using the image with small tiles */
		screen_load();
	}
}


/**
 * Create and initialize the Term contined within this window.
 */
static void term_data_link_sdl(term_window *win)
{
	term *t = &win->term_data;

	/* Initialize the term */
	term_init(t, win->cols, win->rows, win->keys);

	t->higher_pict = true;

	/* Use a "software" cursor */
	t->soft_cursor = true;

	/* Never refresh one row */
	t->never_frosh = true;

	/* Differentiate between BS/^h, Tab/^i, etc. */
	t->complex_input = true;

	/* Ignore the init/nuke hooks */

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_sdl;
	t->curs_hook = Term_curs_sdl;
	t->bigcurs_hook = Term_bigcurs_sdl;
	t->wipe_hook = Term_wipe_sdl;
	t->text_hook = Term_text_sdl;
	t->pict_hook = Term_pict_sdl;
	t->view_map_hook = Term_view_map_sdl;
	t->dblh_hook = (use_graphics && overdraw) ? is_dh_tile : NULL;

	/* Remember where we came from */
	t->data = win;
}

/**
 * Initialize the status bar:
 *  Populate it with some buttons
 *  Set the custom draw function for the bar
 */
static void init_morewindows(void)
{
	char buf[128];
	sdl_Button *button;
	int x;

	popped = false;

	/* Make sure */
	sdl_WindowFree(&PopUp);

	/* Initialize the status bar */
	sdl_WindowInit(&StatusBar, AppWin->w, StatusHeight, AppWin,
		&default_term_font);

	/* Cusom drawing function */
	StatusBar.draw_extra = draw_statusbar;

	AboutSelect = sdl_ButtonBankNew(&StatusBar.buttons);
	button = sdl_ButtonBankGet(&StatusBar.buttons, AboutSelect);

	my_strcpy(buf, buildid, sizeof(buf));

	/* Initialize the 'about' button */
	sdl_ButtonSize(button, StatusBar.font.width * strlen(buf) + 5,
				   StatusHeight - 2);
	sdl_ButtonMove(button, 1, 1);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, buf);
	button->activate = AboutActivate;

	
	/* New button */
	TermSelect = sdl_ButtonBankNew(&StatusBar.buttons);
	button = sdl_ButtonBankGet(&StatusBar.buttons, TermSelect);

	/* Initialize the 'term' button */
	sdl_ButtonSize(button, 60, StatusHeight - 2);
	x = 100 + (StatusBar.font.width * 5);
	sdl_ButtonMove(button, x, 1);
	sdl_ButtonVisible(button, true);
	button->activate = TermActivate;

	/* Another new button */
	VisibleSelect = sdl_ButtonBankNew(&StatusBar.buttons);
	button = sdl_ButtonBankGet(&StatusBar.buttons, VisibleSelect);

	/* Initialize the 'visible' button */
	sdl_ButtonSize(button, 60,  StatusHeight - 2);
	x = 200 + (StatusBar.font.width * 8);
	sdl_ButtonMove(button, x, 1);
	sdl_ButtonVisible(button, true);
	button->activate = VisibleActivate;

	/* Another new button */
	FontSelect = sdl_ButtonBankNew(&StatusBar.buttons);
	button = sdl_ButtonBankGet(&StatusBar.buttons, FontSelect);

	/* Initialize the 'font_select' button */
	sdl_ButtonSize(button, 60,  StatusHeight - 2);
	sdl_ButtonMove(button, 400, 1);
	button->activate = FontActivate;

	/* Another new button */
	MoreSelect = sdl_ButtonBankNew(&StatusBar.buttons);
	button = sdl_ButtonBankGet(&StatusBar.buttons, MoreSelect);

	/* Initialize the 'more' button */
	sdl_ButtonSize(button, 60,  StatusHeight - 2);
	sdl_ButtonMove(button, 400, 1);
	sdl_ButtonVisible(button, true);
	sdl_ButtonCaption(button, "Options");
	button->activate = MoreActivate;

	/* Another new button */
	QuitSelect = sdl_ButtonBankNew(&StatusBar.buttons);
	button = sdl_ButtonBankGet(&StatusBar.buttons, QuitSelect);

	/* Initialize the 'quit' button */
	sdl_ButtonSize(button, 60,  StatusHeight - 2);
	sdl_ButtonMove(button, AppWin->w - 61, 1);
	sdl_ButtonCaption(button, "Quit");
	button->activate = QuitActivate;
	sdl_ButtonVisible(button, true);

	SetStatusButtons();

	TermFocus(0);
}

/**
 * The new streamlined graphics loader.
 * Only uses colour keys.
 * Much more tolerant of different bit-planes
 */
static errr load_gfx(void)
{
	char buf[1024];
	const char *filename;
	SDL_Surface *temp;
	int i;

	if (current_graphics_mode && GfxSurface
		&& (use_graphics == current_graphics_mode->grafID)) {
		return (0);
	}

	current_graphics_mode = get_graphics_mode(use_graphics);
	if (current_graphics_mode) {
		filename = current_graphics_mode->file;
	} else {
		filename = NULL;
	}

	/* Free the old surface */
	if (GfxSurface) SDL_FreeSurface(GfxSurface);

	/* This may be called when GRAPHICS_NONE is set */
	if (!filename) return (0);

	/* Find and load the file into a temporary surface */
	path_build(buf, sizeof(buf), current_graphics_mode->path, filename);
	temp = IMG_Load(buf);
	if (!temp) return (1);

	/* Change the surface type to the current video surface format */
	GfxSurface = SDL_DisplayFormatAlpha(temp);

	/* Make sure we know what pref file to use */
	overdraw = current_graphics_mode->overdrawRow;
	overdraw_max = current_graphics_mode->overdrawMax;

	/* Set double-height tile handling for terminals. */
	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		if (angband_term[i]) {
			angband_term[i]->dblh_hook = (overdraw) ?
				is_dh_tile : NULL;
		}
	}

	/* Reset the graphics mapping for this tileset */
	if (character_dungeon) reset_visuals(true);

	/* All good */
	return (0);
}

/*
 * Initialize the graphics
 */
static void init_gfx(void)
{
	int i;

	/* Check for existence of required files */
	i = 0;
	do {
		char path[1024];
		
		/* Check the graphic file */
		if (graphics_modes[i].file[0]) {
			path_build(path, sizeof(path), graphics_modes[i].path,
					   graphics_modes[i].file);

			if (!file_exists(path)) {
				plog_fmt("Can't find file %s - graphics mode '%s' will be disabled.", path, graphics_modes[i].menuname);
				graphics_modes[i].file[0] = 0;
			}

			if ((i + 1) == use_graphics) {
				current_graphics_mode = &(graphics_modes[i]);
			}
		}
	} while (graphics_modes[i++].grafID != 0); 

	/* Check availability (default to no graphics) */
	if (!current_graphics_mode->file[0]) {
		use_graphics = GRAPHICS_NONE;
		arg_graphics = false;
		tile_width = 1;
		tile_height = 1;
	}

	/* Load the graphics stuff in */
	load_gfx();
}

/**
 * Create the windows
 * Called sometime after load_prefs()
 */
static void init_windows(void)
{
	int i;

	for (i = 0; i < ANGBAND_TERM_MAX; i++) {
		term_window *win = &windows[i];
		
		/* Only bother with visible windows */
		if (win->visible) {
			/* Don't crowd out the status bar... */
			if (win->top < StatusHeight) win->top = StatusHeight;

			/* Invalidate the gfx surface */
			if (win->tiles) {
				SDL_FreeSurface(win->tiles);
				win->tiles = NULL;
			}
			if (win->onebyone) {
				SDL_FreeSurface(win->onebyone);
				win->onebyone = NULL;
			}

			/* This will set up the window correctly */
			ResizeWin(win, win->width, win->height);
		} else {
			/* Doesn't exist */
			angband_term[i] = NULL;
		}
		
		/* Term 0 is at the top */
		Zorder[i] = ANGBAND_TERM_MAX - i - 1;
	}

	/* Good to go... */
	Term_activate(term_screen);
}

/**
 * Set up some SDL stuff
 */
static void init_sdl_local(void)
{
	const SDL_VideoInfo *VideoInfo;

	int i;
	int h, w;
	char path[1024];

	/* Get information about the video hardware */
	VideoInfo = SDL_GetVideoInfo();

	/* Require at least 256 colors */
	if (VideoInfo->vfmt->BitsPerPixel < 8)
		quit_fmt("this %s port requires lots of colors", VERSION_NAME);

	full_w = VideoInfo->current_w;
	full_h = VideoInfo->current_h;

	/* Use a software surface - A tad inefficient, but stable... */
	vflags |= SDL_SWSURFACE;

	/* Set fullscreen flag */
	if (fullscreen) vflags |= SDL_FULLSCREEN;

	/* otherwise we make this surface resizable */
	else vflags |= SDL_RESIZABLE;

	/* Create the main window */
	AppWin = SDL_SetVideoMode(fullscreen ? full_w : screen_w,
							  fullscreen ? full_h : screen_h, 0, vflags);

	/* Handle failure */
 	if (!AppWin)
		quit_fmt("failed to create %dx%d window at %d bpp",
			screen_w, screen_h, VideoInfo->vfmt->BitsPerPixel);

	/* Set the window caption */
	SDL_WM_SetCaption(VERSION_NAME, NULL);

	/* Enable key repeating; use defaults */
	(void)SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
							  SDL_DEFAULT_REPEAT_INTERVAL);

	/* Enable Unicode (so we can read key codes) */
	(void)SDL_EnableUNICODE(1);

	/* Build a color for "black" that matches the pixel depth of this surface */
	back_colour.r = angband_color_table[COLOUR_DARK][1];
	back_colour.g = angband_color_table[COLOUR_DARK][2];
	back_colour.b = angband_color_table[COLOUR_DARK][3];
	back_pixel_colour = SDL_MapRGB(AppWin->format, back_colour.r,
						   back_colour.g, back_colour.b);

	/* Initialize the colours */
	for (i = 0; i < MAX_COLORS; i++) {
		text_colours[i].r = angband_color_table[i][1];
		text_colours[i].g = angband_color_table[i][2];
		text_colours[i].b = angband_color_table[i][3];
	}

	/* Get the height of the status bar */
	if (sdl_CheckFont(&default_term_font, &w, &h)) {
		quit_fmt("could not load the default font, %s",
			default_term_font.name);
	}
	StatusHeight = h + 3;

	/* Font used for window titles */
	sdl_FontCreate(&SystemFont, &default_term_font, AppWin);

	/* Get the icon for display in the About box */
	path_build(path, sizeof(path), ANGBAND_DIR_ICONS, "att-128.png");
	if (file_exists(path))
		mratt = IMG_Load(path);
}

/**
 * Font sorting function
 *
 * Orders by width, then height, then face
 */
static int cmp_font(const void *f1, const void *f2)
{
	const char *font1 = *(const char **)f1;
	const char *font2 = *(const char **)f2;
	int height1 = 0, height2 = 0;
	int width1 = 0, width2 = 0;
	char *ew, *face1 = NULL, *ext1 = NULL, *face2 = NULL, *ext2 = NULL;
	long lv;

	lv = strtol(font1, &ew, 10);
	if (ew != font1 && *ew == 'x' && lv > INT_MIN && lv < INT_MAX) {
		width1 = (int)lv;
		lv = strtol(ew + 1, &face1, 10);
		if (face1 != ew + 1 && lv > INT_MIN && lv < INT_MAX) {
			height1 = (int)lv;
			ext1 = strchr(face1, '.');
			if (ext1 == face1) {
				ext1 = NULL;
			}
		}
	}
	lv = strtol(font2, &ew, 10);
	if (ew != font2 && *ew == 'x' && lv > INT_MIN && lv < INT_MAX) {
		width2 = (int)lv;
		lv = strtol(ew + 1, &face2, 10);
		if (face2 != ew + 1 && lv > INT_MIN && lv < INT_MAX) {
			height2 = (int)lv;
			ext2 = strchr(face2, '.');
			if (ext2 == face2) {
				ext2 = NULL;
			}
		}
	}

	if (!ext1) {
		if (!ext2) {
			/*
			 * Neither match the expected pattern.  Sort
			 * alphabetically.
			 */
			return strcmp(font1, font2);
		}
		/* Put f2 first since it matches the expected pattern. */
		return 1;
	}
	if (!ext2) {
		/* Put f1 first since it matches the expected pattern. */
		return -1;
	}
	if (width1 < width2) {
		return -1;
	}
	if (width1 > width2) {
		return 1;
	}
	if (height1 < height2) {
		return -1;
	}
	if (height1 > height2) {
		return 1;
	}
	return strncmp(face1, face2, MAX(ext1 - face1, ext2 - face2));
}

/**
 * This function is now mis-named as paths are set correctly by init_stuff()
 * in main.c before init_sdl calls this. But it still does some other stuff.
 */ 
static void init_paths(void)
{
	int i;
	char buf[1024], path[1024];
	ang_dir *dir;

	/* Validate the basic font */
	if (default_term_font.preset) {
		/* Build the filename */
		path_build(path, sizeof(path), ANGBAND_DIR_FONTS,
			default_term_font.name);
		validate_file(path);
	} else {
		validate_file(default_term_font.name);
	}

	for (i = 0; i < MAX_FONTS; i++)
		FontList[i] = NULL;

	/* Open the fonts directory */
	dir = my_dopen(ANGBAND_DIR_FONTS);
	if (!dir) return;

	/* Read every font to the limit */
	while (my_dread(dir, buf, sizeof(buf))) {
		path_build(path, sizeof(path), ANGBAND_DIR_FONTS,
			buf);
		if (is_font_file(path)) {
			FontList[num_fonts++] = string_make(buf);
		}
		/* Don't grow to long */
		if (num_fonts == MAX_FONTS) break;
	}

	sort(FontList, num_fonts, sizeof(FontList[0]), cmp_font);

	/* Done */
	my_dclose(dir);
}


const char help_sdl[] = "SDL frontend";

/**
 * The SDL port's "main()" function.
 */
int init_sdl(int argc, char *argv[])
{
	/* Initialize SDL:  Timer, video, and audio functions */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		return (2);
	}

	/* Initialize the TTF library */
	if (TTF_Init() < 0) {
		fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
		SDL_Quit();
		return (2);
	}

	/* Init some extra paths */
	init_paths();

	/* load possible graphics modes */
	init_graphics_modes();
	GfxButtons = mem_zalloc(sizeof(int) * (graphics_mode_high_id+1));

	/* Load prefs */
	load_prefs();

	/* Get sdl going */
	init_sdl_local();

	/* Prepare the windows */
	init_windows();

	/* Prepare the gfx */
	init_gfx();

	/* Prepare some more windows(!) */
	init_morewindows();

	/* Activate  quit hook */
	quit_aux = hook_quit;

	/* Paranoia */
	return (0);
}

#endif /* USE_SDL */
