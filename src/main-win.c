/**
 * \file main-win.c
 * \brief Provide a front end for Microsoft Windows
 *
 * Copyright (c) 1997 Ben Harrison, Skirmantas Kligys, Robert Ruehlmann,
 * and others
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
 *
 * This file helps Angband work with Windows computers.
 *
 * To use this file, use an appropriate "Makefile" or "Project File",
 * make sure that "WINDOWS" and/or "WIN32" are defined somewhere, and
 * make sure to obtain various extra files as described below.
 *
 * The Windows version has been tested to compile with Visual C++ 5.0
 * and 6.0, Cygwin 1.0, Borland C++ 5.5 command line tools, and lcc-win32.
 *
 *
 * The "lib/customize/font-win.prf" contains attr/char mappings for use with the
 * normal "*.fon" font files in the "lib/fonts/" directory.
 *
 * The "lib/customize/graf-win.prf" contains attr/char mappings for use with the
 * special "*.png" graphics files in the "lib/tiles/" directory, which
 * are activated by a menu item.
 *
 *
 * The "Term_xtra_win_clear()" function should probably do a low-level
 * clear of the current window, and redraw the borders and other things,
 * if only for efficiency.  XXX XXX XXX
 *
 * A simpler method is needed for selecting the "tile size" for windows.
 * XXX XXX XXX
 *
 * ToDo: The screensaver mode should implement ScreenSaverConfigureDialog,
 * DefScreenSaverProc, and ScreenSaverProc.
 *
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Original code by Skirmantas Kligys (kligys@scf.usc.edu).
 *
 * Additional code by Ross E Becker (beckerr@cis.ohio-state.edu),
 * and Chris R. Martin (crm7479@tam2000.tamu.edu).
 *
 * Additional code by Robert Ruehlmann <rr9@thangorodrim.net>.
 */

#include "angband.h"
#include "buildid.h"
#include "cmds.h"
#include "cave.h"
#include "game-world.h"
#include "grafmode.h"
#include "init.h"
#include "savefile.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-init.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-output.h"
#include "ui-prefs.h"
#include "win/win-menu.h"

/* Make sure the winver allows the AlphaBlend function */
#if (WINVER < 0x0500)
#define WINVER 0x0500
#endif

#include <locale.h>

#define uint unsigned int

#if (defined(WINDOWS) && !defined(USE_SDL)) && !defined(USE_SDL2)

#include "sound.h"
#include "snd-win.h"

#define HAS_CLEANUP


/**
 * This may need to be removed for some compilers XXX XXX XXX
 */
#define STRICT

/**
 * Exclude parts of WINDOWS.H that are not needed
 */
#define NOCOMM            /* Comm driver APIs and definitions */
#define NOLOGERROR        /* LogError() and related definitions */
#define NOPROFILER        /* Profiler APIs */
#define NOLFILEIO         /* _l* file I/O routines */
#define NOOPENFILE        /* OpenFile and related definitions */
#define NORESOURCE        /* Resource management */
#define NOATOM            /* Atom management */
#define NOLANGUAGE        /* Character test routines */
#define NOLSTRING         /* lstr* string management routines */
#define NODBCS            /* Double-byte character set routines */
#define NOKEYBOARDINFO    /* Keyboard driver routines */
#define NOCOLOR           /* COLOUR_* color values */
#define NODRAWTEXT        /* DrawText() and related definitions */
#define NOSCALABLEFONT    /* Truetype scalable font support */
#define NOMETAFILE        /* Metafile support */
#define NOSYSTEMPARAMSINFO /* SystemParametersInfo() and SPI_* definitions */
#define NODEFERWINDOWPOS  /* DeferWindowPos and related definitions */
#define NOKEYSTATES       /* MK_* message key state flags */
#define NOWH              /* SetWindowsHook and related WH_* definitions */
#define NOCLIPBOARD       /* Clipboard APIs and definitions */
#define NOICONS           /* IDI_* icon IDs */
#define NOMDI             /* MDI support */
#define NOHELP            /* Help support */

/* Not defined since it breaks Borland C++ 5.5 */
/* #define NOCTLMGR */    /* Control management and controls */

/**
 * Exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
/*#define NONLS*/             /* All NLS defines and routines */
#define NOSERVICE         /* All Service Controller routines, SERVICE_ equates, etc. */
#define NOKANJI           /* Kanji support stuff. */
#define NOMCX             /* Modem Configuration Extensions */

/**
 * Include the "windows" support file
 */
#include <windows.h>
#include <windowsx.h>

#ifndef GetWindowLongPtr
#define GetWindowLongPtr GetWindowLong
#endif
#ifndef SetWindowLongPtr
#define SetWindowLongPtr SetWindowLong
#endif
#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif

/**
 * Exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          /* Installable driver support */
#define MMNOWAVE         /* Waveform support */
#define MMNOMIDI         /* MIDI support */
#define MMNOAUX          /* Auxiliary audio support */
#define MMNOTIMER        /* Timer support */
#define MMNOJOY          /* Joystick support */
/*#define MMNOMCI */         /* MCI support */
#define MMNOMMIO         /* Multimedia file I/O support */
#define MMNOMMSYSTEM     /* General MMSYSTEM functions */

#include <mmsystem.h>

#include <commdlg.h>
#include <shellapi.h>

/**
 * Include the support for loading bitmaps
 */
#include "win/readdib.h"

#include <wingdi.h>

/**
 * Hack -- Fake declarations from "dos.h" XXX XXX XXX
 */
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF

/**
 * Silliness in WIN32 drawing routine
 */
#define MoveTo(H,X,Y) MoveToEx(H, X, Y, NULL)

/**
 * Silliness for Windows 95
 */
#ifndef WS_EX_TOOLWINDOW
# define WS_EX_TOOLWINDOW 0
#endif /* WS_EX_TOOLWINDOW */

/**
 * Foreground color bits (hard-coded by DOS)
 */
#define VID_BLACK	0x00
#define VID_BLUE	0x01
#define VID_GREEN	0x02
#define VID_CYAN	0x03
#define VID_RED		0x04
#define VID_MAGENTA	0x05
#define VID_YELLOW	0x06
#define VID_WHITE	0x07

/**
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT	0x08

/**
 * Background color bits (hard-coded by DOS)
 */
#define VUD_BLACK	0x00
#define VUD_BLUE	0x10
#define VUD_GREEN	0x20
#define VUD_CYAN	0x30
#define VUD_RED		0x40
#define VUD_MAGENTA	0x50
#define VUD_YELLOW	0x60
#define VUD_WHITE	0x70

/**
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BRIGHT	0x80

/**
 * Font settings
 */
#define DEFAULT_FONT	"8X12x.FON"


/**
 * Extra "term" data
 *
 * Note the use of "font_want" for the names of the font file requested by
 * the user, and the use of "font_file" for the currently active font file.
 *
 * The "font_file" is uppercased, and takes the form "8X13.FON", while
 * "font_want" can be in almost any form as long as it could be construed
 * as attempting to represent the name of a font.
 */
#include "win/win-term.h"

bool use_graphics_nice;

/**
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];

/**
 * Hack -- global "window creation" pointer
 */
static term_data *my_td;

/**
 * Default window layout function
 */
int default_layout_win(term_data *data, int maxterms);


/**
 * game in progress
 */
bool game_in_progress = false;

/**
 * note when "open"/"new" become valid
 */
bool initialized = false;

/**
 * This is a handle to a file; used to protect against playing with the same
 * character simultaneously in multiple instances of the application.  Using
 * a named mutex did not appear to work (at least with wine).  Using a file
 * has the disadvantage that a system crash while playing a character will
 * require the player to delete a file in order to bypass the check that
 * another copy of the game is running.
 */
static HANDLE multapp_file = INVALID_HANDLE_VALUE;
static bool create_savefile_tracking_file(bool message_on_failure);
static bool monitor_existing_savefile(void);
static void monitor_new_savefile(game_event_type ev_type,
	game_event_data *ev_data, void *user);
static void finish_monitoring_savefile(game_event_type ev_type,
	game_event_data *ev_data, void *user);

/**
 * screen paletted, i.e. 256 colors
 */
bool paletted = false;

/**
 * 16 colors screen, don't use RGB()
 */
bool colors16 = false;

static bool low_priority = false;

/**
 * Saved instance handle
 */
static HINSTANCE hInstance;

/**
 * Yellow brush for the cursor
 */
static HBRUSH hbrYellow;

/**
 * An icon
 */
static HICON hIcon;

/**
 * A palette
 */
static HPALETTE hPal;


#ifdef USE_SAVER

/**
 * The screen saver window
 */
static HWND hwndSaver;

static HANDLE screensaverSemaphore;

static char saverfilename[1024];

static HMENU main_menu;

#define MOUSE_SENS 10

#endif /* USE_SAVER */

static bool screensaver_active = false;


/**
 * Flag set once "graphics" has been initialized
 */
static bool can_use_graphics = false;

/**
 * Flag set when switching tilesizes
 */
static bool change_tilesize = false;

/**
 * The global bitmap
 */
static DIBINIT infGraph;

/**
 * The global bitmap mask
 */
static DIBINIT infMask;

static int overdraw = 0;
static int overdrawmax = -1;

static int alphablend = 0;
static BLENDFUNCTION blendfn;

/**
 * Full path to ANGBAND.INI
 */
static char *ini_file = NULL;

/**
 * Name of application
 */
static const char *AppName = VERSION_NAME;

/**
 * Name of sub-window type
 */
static const char *AngList = "AngList";

/**
 * The "complex" color values
 */
static COLORREF win_clr[MAX_COLORS];


/**
 * The "simple" color values
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static uint8_t win_pal[MAX_COLORS] =
{
	VID_BLACK,					/* Dark */
	VID_WHITE,					/* White */
	VID_CYAN,					/* Slate XXX */
	VID_RED | VID_BRIGHT,		/* Orange XXX */
	VID_RED,					/* Red */
	VID_GREEN,					/* Green */
	VID_BLUE,					/* Blue */
	VID_YELLOW,					/* Umber XXX */
	VID_BLACK | VID_BRIGHT,		/* Light Dark */
	VID_CYAN | VID_BRIGHT,		/* Light Slate XXX */
	VID_MAGENTA,				/* Violet XXX */
	VID_YELLOW | VID_BRIGHT,	/* Yellow */
	VID_MAGENTA | VID_BRIGHT,	/* Light Red XXX */
	VID_GREEN | VID_BRIGHT,		/* Light Green */
	VID_BLUE | VID_BRIGHT,		/* Light Blue */
	VID_YELLOW					/* Light Umber XXX */
};


static int gamma_correction;



#if 0
/**
 * Hack -- given a pathname, point at the filename
 */
static const char *extract_file_name(const char *s)
{
	const char *p;

	/* Start at the end */
	p = s + strlen(s) - 1;

	/* Back up to divider */
	while ((p >= s) && (*p != ':') && (*p != '\\')) p--;

	/* Return file name */
	return (p+1);
}
#endif /* 0 */


static void show_win_error(void)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	              NULL, GetLastError(),
	              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              (LPTSTR) &lpMsgBuf, 0, NULL);

	MessageBox(NULL, lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION);

	LocalFree(lpMsgBuf);
}


/**
 * Hack -- given a simple filename, extract the "font size" info
 *
 * Return a pointer to a static buffer holding the capitalized base name.
 */
static char *analyze_font(char *path, int *wp, int *hp)
{
	int wid, hgt;

	char *s, *p;

	/* Start at the end */
	p = path + strlen(path) - 1;

	/* Back up to divider */
	while ((p >= path) && (*p != ':') && (*p != '\\')) --p;

	/* Advance to file name */
	++p;

	/* Capitalize */
	for (s = p; *s; ++s) {
		/* Capitalize (be paranoid) */
		if (islower((unsigned char)*s)) *s = toupper((unsigned char)*s);
	}

	/* Find first 'X' */
	s = strchr(p, 'X');

	/* Extract font width */
	wid = atoi(p);

	/* Extract height */
	hgt = s ? atoi(s+1) : 0;

	/* Save results */
	(*wp) = wid;
	(*hp) = hgt;

	/* Result */
	return (p);
}


/**
 * Check for existence of a directory
 */
static bool check_dir(const char *s)
{
	int i;
	char path[1024];
	DWORD attrib;

	/* Copy it */
	my_strcpy(path, s, sizeof(path));

	/* Check length */
	i = strlen(path);

	/* Remove trailing backslash */
	if (i && (path[i-1] == '\\')) path[--i] = '\0';

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (false);

	/* Require directory */
	if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) return (false);

	/* Success */
	return (true);
}


/**
 * Validate a file
 */
static void validate_file(const char *s)
{
	/* Verify or fail */
	if (!file_exists(s))
		quit_fmt("Cannot find required file:\n%s", s);
}


/**
 * Validate a directory
 */
static void validate_dir(const char *s)
{
	/* Verify or fail */
	if (!check_dir(s))
		quit_fmt("Cannot find required directory:\n%s", s);
}


/**
 * Get the "size" for a window
 */
static void term_getsize(term_data *td)
{
	RECT rc;

	int wid, hgt;

	/* Paranoia */
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

	if (use_graphics_nice) {
		if (current_graphics_mode && current_graphics_mode->grafID) {
			if (current_graphics_mode->file[0]) {
                char *end;
                td->tile_wid = strtol(current_graphics_mode->file,&end,10);
                td->tile_hgt = strtol(end+1,NULL,10);
			} else {
                td->tile_wid = current_graphics_mode->cell_width;
                td->tile_hgt = current_graphics_mode->cell_height;
			}
			if ((td->tile_wid == 0) || (td->tile_hgt == 0)) {
                td->tile_wid = current_graphics_mode->cell_width;
                td->tile_hgt = current_graphics_mode->cell_height;
			}
			if ((td->tile_wid == 0) || (td->tile_hgt == 0)) {
                td->tile_wid = td->font_wid;
                td->tile_hgt = td->font_hgt;
			}
		} else {
			/* Reset the tile info */
			td->tile_wid = td->font_wid;
			td->tile_hgt = td->font_hgt;
		}
		
	    tile_width = 1;
	    tile_height = 1;
		
		if ((td->tile_hgt >= td->font_hgt * 3) &&
			(td->tile_wid >= td->font_wid * 3)) {
			tile_width = 3;
			tile_height = 3;
			td->tile_wid /= 3;
			td->tile_hgt /= 3;
		} else if ((td->tile_hgt >= td->font_hgt * 2) &&
				   (td->tile_wid >= td->font_wid * 2)) {
			tile_width = 2;
			tile_height = 2;
			td->tile_wid /= 2;
			td->tile_hgt /= 2;
		}
		
		if (td->tile_wid >= td->font_wid * 2) {
			tile_width *= 2;
			td->tile_wid /= 2;
		}
		
		if (td->tile_wid < td->font_wid) td->tile_wid = td->font_wid;
		if (td->tile_hgt < td->font_hgt) td->tile_hgt = td->font_hgt;
	}
	
	/* Window sizes */
	wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

	/* Client window size */
	rc.left = 0;
	rc.right = rc.left + wid;
	rc.top = 0;
	rc.bottom = rc.top + hgt;

	/* Get total window size (without menu for sub-windows) */
	AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

	/* Total size */
	td->size_wid = rc.right - rc.left;
	td->size_hgt = rc.bottom - rc.top;

	/* See CreateWindowEx */
	if (!td->w) return;

	/* Extract actual location */
	GetWindowRect(td->w, &rc);

	/* Save the location */
	td->pos_x = rc.left;
	td->pos_y = rc.top;
}


/**
 * Write the "prefs" for a single term
 */
static void save_prefs_aux(term_data *td, const char *sec_name)
{
	char buf[1024];

	RECT rc;

	WINDOWPLACEMENT lpwndpl;

	/* Paranoia */
	if (!td->w) return;

	/* Visible */
	strcpy(buf, td->visible ? "1" : "0");
	WritePrivateProfileString(sec_name, "Visible", buf, ini_file);

	/* Font */
	strcpy(buf, td->font_file ? td->font_file : DEFAULT_FONT);
	WritePrivateProfileString(sec_name, "Font", buf, ini_file);

	/* Bizarre */
	strcpy(buf, td->bizarre ? "1" : "0");
	WritePrivateProfileString(sec_name, "Bizarre", buf, ini_file);

	/* Tile size (x) */
	wsprintf(buf, "%d", td->tile_wid);
	WritePrivateProfileString(sec_name, "TileWid", buf, ini_file);

	/* Tile size (y) */
	wsprintf(buf, "%d", td->tile_hgt);
	WritePrivateProfileString(sec_name, "TileHgt", buf, ini_file);

	/* Window size (x) */
	wsprintf(buf, "%d", td->cols);
	WritePrivateProfileString(sec_name, "NumCols", buf, ini_file);

	/* Window size (y) */
	wsprintf(buf, "%d", td->rows);
	WritePrivateProfileString(sec_name, "NumRows", buf, ini_file);

	/* Get window placement and dimensions */
	lpwndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(td->w, &lpwndpl);

	/* Acquire position in *normal* mode (not minimized) */
	rc = lpwndpl.rcNormalPosition;

	/* Get information about the placement of the window */
	if (lpwndpl.showCmd & SW_SHOWMAXIMIZED)
		td->maximized = true;
	else
		td->maximized = false;

	/* Window position (x) */
	wsprintf(buf, "%d", rc.left);
	WritePrivateProfileString(sec_name, "PositionX", buf, ini_file);

	/* Window position (y) */
	wsprintf(buf, "%d", rc.top);
	WritePrivateProfileString(sec_name, "PositionY", buf, ini_file);

	/* Maximized */
	strcpy(buf, td->maximized ? "1" : "0");
	WritePrivateProfileString(sec_name, "Maximized", buf, ini_file);
}


/**
 * Write the "prefs"
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)
{
	int i;

	char buf[128];

	/* Save the "arg_graphics" flag */
	sprintf(buf, "%d", arg_graphics);
	WritePrivateProfileString("Angband", "Graphics", buf, ini_file);

        /* Save the "use_graphics_nice" flag */
        strcpy(buf, arg_graphics_nice ? "1" : "0");
        WritePrivateProfileString("Angband", "Graphics_Nice", buf, ini_file);

        /* Save the tile width */
        wsprintf(buf, "%d", tile_width);
        WritePrivateProfileString("Angband", "TileWidth", buf, ini_file);

        /* Save the tile height */
        wsprintf(buf, "%d", tile_height);
        WritePrivateProfileString("Angband", "TileHeight", buf, ini_file);

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		term_data *td = &data[i];

		sprintf(buf, "Term-%d", i);

		save_prefs_aux(td, buf);
	}
}


/**
 * Load the "prefs" for a single term
 */
static void load_prefs_aux(term_data *td, const char *sec_name)
{
	char tmp[1024];

	int wid, hgt;

	/* Visible */
	td->visible = (GetPrivateProfileInt(sec_name, "Visible", td->visible,
										ini_file) != 0);

	/* Maximized */
	td->maximized = (GetPrivateProfileInt(sec_name, "Maximized", td->maximized,
										  ini_file) != 0);

	/* Desired font, with default */
	GetPrivateProfileString(sec_name, "Font", DEFAULT_FONT, tmp, 127, ini_file);

	/* Bizarre */
	td->bizarre = (GetPrivateProfileInt(sec_name, "Bizarre", true,
										ini_file) != 0);

	/* Analyze font, save desired font name */
	td->font_want = string_make(analyze_font(tmp, &wid, &hgt));

	/* Tile size */
	td->tile_wid = GetPrivateProfileInt(sec_name, "TileWid", wid, ini_file);
	td->tile_hgt = GetPrivateProfileInt(sec_name, "TileHgt", hgt, ini_file);

	/* Window size */
	td->cols = GetPrivateProfileInt(sec_name, "NumCols", td->cols, ini_file);
	td->rows = GetPrivateProfileInt(sec_name, "NumRows", td->rows, ini_file);

	/* Window position */
	td->pos_x = GetPrivateProfileInt(sec_name, "PositionX", td->pos_x,
									 ini_file);
	td->pos_y = GetPrivateProfileInt(sec_name, "PositionY", td->pos_y,
									 ini_file);
}


/**
 * Load the "prefs"
 */
static void load_prefs(void)
{
	int i;

	char buf[1024];
	bool first_start;

	if (file_exists(ini_file)) {
		first_start = false;
	} else {
		first_start = true;
	}

	/* Extract the "arg_graphics" flag */
	arg_graphics = GetPrivateProfileInt("Angband", "Graphics", GRAPHICS_NONE,
										ini_file);

	/* Extract the "arg_graphics_nice" flag */
	arg_graphics_nice = GetPrivateProfileInt("Angband", "Graphics_Nice", true,
											 ini_file);

	/* Extract the tile width */
	tile_width = GetPrivateProfileInt("Angband", "TileWidth", false, ini_file);

	/* Extract the tile height */
	tile_height = GetPrivateProfileInt("Angband", "TileHeight", false,
									   ini_file);

	/* Extract the "arg_wizard" flag */
	arg_wizard = (GetPrivateProfileInt("Angband", "Wizard", 0, ini_file) != 0);

	/* Extract the gamma correction */
	gamma_correction = GetPrivateProfileInt("Angband", "Gamma", 0, ini_file);


	/* Load window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		term_data *td = &data[i];

		sprintf(buf, "Term-%d", i);

		load_prefs_aux(td, buf);
	}

	if (first_start) {
		default_layout_win(data,MAX_TERM_DATA);
	}

	/* Paranoia */
	if (data[0].cols < 80) data[0].cols = 80;
	if (data[0].rows < 24) data[0].rows = 24;
}

/**
 * Create the new global palette based on the bitmap palette
 * (if any), and the standard 16 entry palette derived from
 * "win_clr[]" which is used for the basic 16 Angband colors.
 *
 * This function is never called before all windows are ready.
 *
 * This function returns false if the new palette could not be
 * prepared, which should normally be a fatal error.  XXX XXX
 *
 * Note that only some machines actually use a "palette".
 */
static int new_palette(void)
{
	HPALETTE hBmPal;
	HPALETTE hNewPal;
	HDC hdc;
	int i, nEntries;
	int pLogPalSize;
	LPLOGPALETTE pLogPal;
	LPPALETTEENTRY lppe;

	term_data *td;


	/* This makes no sense */
	if (!paletted) return (true);

	/* No bitmap */
	lppe = NULL;
	nEntries = 0;

	/* Check the bitmap palette */
	hBmPal = infGraph.hPalette;

	/* Use the bitmap */
	if (hBmPal) {
		lppe = mem_alloc(256 * sizeof(PALETTEENTRY));
		nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
		if ((nEntries == 0) || (nEntries > 220)) {
			/* Warn the user */
			plog("Please switch to high- or true-color mode.");

			/* Cleanup */
			mem_free(lppe);

			/* Fail */
			return (false);
		}
	}

	/* Size of palette */
	pLogPalSize = sizeof(LOGPALETTE) + (nEntries + 16) * sizeof(PALETTEENTRY);

	/* Allocate palette */
	pLogPal = (LPLOGPALETTE)mem_alloc(pLogPalSize);

	/* Version */
	pLogPal->palVersion = 0x300;

	/* Make room for bitmap and normal data */
	pLogPal->palNumEntries = nEntries + 16;

	/* Save the bitmap data */
	for (i = 0; i < nEntries; i++)
		pLogPal->palPalEntry[i] = lppe[i];

	/* Save the normal data */
	for (i = 0; i < BASIC_COLORS; i++) {
		LPPALETTEENTRY p;

		/* Access the entry */
		p = &(pLogPal->palPalEntry[i+nEntries]);

		/* Save the colors */
		p->peRed = GetRValue(win_clr[i]);
		p->peGreen = GetGValue(win_clr[i]);
		p->peBlue = GetBValue(win_clr[i]);


		if (gamma_correction > 0) {
			p->peRed = gamma_table[p->peRed];
			p->peGreen = gamma_table[p->peGreen];
			p->peBlue = gamma_table[p->peBlue];
		}


		/* Save the flags */
		p->peFlags = PC_NOCOLLAPSE;
	}

	/* Free something */
	if (lppe) mem_free(lppe);

	/* Create a new palette, or fail */
	hNewPal = CreatePalette(pLogPal);
	if (!hNewPal) quit("Cannot create palette!");

	/* Free the palette */
	mem_free(pLogPal);

	/* Main window */
	td = &data[0];

	/* Realize the palette */
	hdc = GetDC(td->w);
	SelectPalette(hdc, hNewPal, 0);
	i = RealizePalette(hdc);
	ReleaseDC(td->w, hdc);
	if (i == 0) quit("Cannot realize palette!");

	/* Sub-windows */
	for (i = 1; i < MAX_TERM_DATA; i++) {
		td = &data[i];

		hdc = GetDC(td->w);
		SelectPalette(hdc, hNewPal, 0);
		ReleaseDC(td->w, hdc);
	}

	/* Delete old palette */
	if (hPal) DeleteObject(hPal);

	/* Save new palette */
	hPal = hNewPal;

	/* Success */
	return (true);
}


/**
 * Initialize graphics
 */
static bool init_graphics(void)
{
	/* Initialize once */
	char buf[1024];
	int wid, hgt;
	const char *name;
	const char *path;
	graphics_mode *mode = NULL;

	if (arg_graphics) {
		mode = get_graphics_mode(arg_graphics);
	}
	if (!mode) {
		mode = get_graphics_mode(1);
	}
	if (mode) {
		if (!mode->pref[0]) {
			plog_fmt("invalid tile prefname '%s'", mode->menuname);
			return false;
		}
		wid = mode->cell_width;
		hgt = mode->cell_height;
		if ((wid < 2) || (hgt < 2)) {
			plog_fmt("invalid tile dimensions in tileset: '%s'",
					 mode->menuname);
			return false;
		}

		path = mode->path;
		name = mode->file;

		overdraw = mode->overdrawRow;
		overdrawmax = mode->overdrawMax;
		alphablend = mode->alphablend;

		current_graphics_mode = mode;
	} else {
		plog("could not find graphics mode");
		return false;
	}

	/* Access the bitmap file */
	path_build(buf, sizeof(buf), path, name);

	/* Load the image or quit */
	if (alphablend) {
		/* see if the given file is already pre mulitiplied */
		if (strstr(name, "_pre")) {
			/* if so, just load it */
			if (!ReadDIB2_PNG(data[0].w, buf, &infGraph, NULL, false)) {
				plog_fmt("Cannot read file '%s'", name);
				return false;
			}
		} else {
			/* if not, see if there is already a premultiplied tileset */
			/* the there is load it */
			char *ext;
			char modname[1024];
			bool have_space = 0;
			my_strcpy(modname, buf,1024);
			ext = strstr(modname,".png");
			/* make sure we have enough space to make the desired name */
			if (ext && ((ext-buf) < 1019)) {
				have_space = true;
				strcpy(ext, "_pre.png");
				if (!file_exists(modname)) {
					/* if the file does not exist, mark that we need to 
					 * create it, so clear the extension pointer */
					ext = NULL;
				} else if (file_newer(buf, modname)) {
					/* if the base file is newer than the premultiplied file,
					 * mark that we need to recreate the premultiplied file. */
					ext = NULL;
				}
			}
			if (ext && have_space) {
				/* at this point we know the file exists, so load it */
				if (!ReadDIB2_PNG(data[0].w, modname, &infGraph, NULL, false)) {
					plog_fmt("Cannot read premultiplied version of file '%s'",
							 name);
					return false;
				}
			} else {
				/* if not, load the base file and premultiply it */
				if (!ReadDIB2_PNG(data[0].w, buf, &infGraph, NULL, true)) {
					plog_fmt("Cannot read file '%s'", name);
					return false;
				}
				/* save the premultiplied file */
				/* saving alpha without a mask is not working yet
				   if (SavePNG(data[0].w, modname,
				   infGraph.hBitmap,infGraph.hPalette,
				   1, NULL,
				   infGraph.ImageWidth, infGraph.ImageHeight, false) < 0) {
				   plog_fmt("Cannot write premultiplied version of file '%s'", name);
				   }*/
			}
		}
	} else {
		if (!ReadDIB2_PNG(data[0].w, buf, &infGraph, &infMask, false)) {
			plog_fmt("Cannot read file '%s'", name);
			return false;
		}
	}

	/* Save the new sizes */
	infGraph.CellWidth = wid;
	infGraph.CellHeight = hgt;

	/* Activate a palette */
	if (!new_palette()){
		/* Free bitmap XXX XXX XXX */

		/* Oops */
		plog("Cannot activate palette!");
		return (false);
	}

	/* Graphics available */
	can_use_graphics = arg_graphics;

	/* Result */
	return (can_use_graphics);
}

#ifdef SOUND

/* Supported file types */
enum {
	WIN_NULL = 0,
	WIN_MP3,
	WIN_WAV
};

const struct sound_file_type supported_sound_files[] = { {".mp3", WIN_MP3},
							 {".wav", WIN_WAV},
							 {"", WIN_NULL} };

typedef struct
{
	int		type;
	MCI_OPEN_PARMS	op;
	char		*filename;
} win_sample;

/**
 * Load a sound
 */
static bool load_sound_win(const char *filename, int file_type, struct sound_data *data)
{
	win_sample *sample = NULL;

	sample = (win_sample *)(data->plat_data);

	switch (file_type) {
		case WIN_MP3:
			if (!sample)
				sample = mem_zalloc(sizeof(*sample));

			/* Open if not already */
			if (!sample->op.wDeviceID) {
				sample->op.dwCallback = 0;
				sample->op.lpstrDeviceType = (char*)MCI_ALL_DEVICE_ID;
				sample->op.lpstrElementName = filename;
				sample->op.lpstrAlias = NULL;

				/* Open command */
				mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_WAIT, (size_t)(&sample->op));
			}

			data->loaded = (0 != sample->op.wDeviceID);

			if (!data->loaded) {
				mem_free(sample);
				sample = NULL;
			}
			break;

		case WIN_WAV:
			if (!sample)
				sample = mem_alloc(sizeof(*sample));

			sample->filename = mem_zalloc(strlen(filename) + 1);
			my_strcpy(sample->filename, filename, strlen(filename) + 1);
			data->loaded = true;
			break;

		default:
			plog_fmt("Sound: Oops - Unsupported file type");
			data->loaded = false;
			break;
	}

	if (sample) {
		sample->type = file_type;
	}
	data->plat_data = (void *)sample;

	return (NULL != sample);
}

/**
 * Play a sound
 */
static bool play_sound_win(struct sound_data *data)
{
	MCI_PLAY_PARMS pp;

	win_sample *sample = (win_sample *)(data->plat_data);

	if (sample) {
		switch (sample->type) {
			case WIN_MP3:
				if (sample->op.wDeviceID) {
					/* Play command */
					pp.dwCallback = 0;
					pp.dwFrom = 0;
					return (!mciSendCommand(sample->op.wDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM, (size_t)&pp));
				}
				break;

			case WIN_WAV:
				if (sample->filename)
				{
					/* If another sound is currently playing, stop it */
					if (PlaySound(NULL, 0, SND_PURGE))
						/* Play the sound, catch errors */
						return PlaySound(sample->filename, 0, SND_FILENAME | SND_ASYNC);
				}
				break;
			default:
				/* Not supported */
				break;
		}
	}

	return true;
}

static bool unload_sound_win(struct sound_data *data)
{
	win_sample *sample = (win_sample *)(data->plat_data);

	if (sample) {
		switch (sample->type) {
			case WIN_MP3:
				if (sample->op.wDeviceID)
					mciSendCommand(sample->op.wDeviceID, MCI_CLOSE, MCI_WAIT, (size_t)(&sample->op));

				break;

			case WIN_WAV:
				mem_free(sample->filename);
				break;

			default:
				break;
		}

		mem_free(sample);
		data->plat_data = NULL;
		data->loaded = false;
	}

	return true;
}

static bool open_audio_win(void)
{
	return true;
}

static bool close_audio_win(void)
{
	return true;
}

const struct sound_file_type *supported_files_win(void)
{
	return supported_sound_files;
}

/**
 * Initialize sound
 */
errr init_sound_win(struct sound_hooks *hooks, int argc, char **argv)
{
	hooks->open_audio_hook = open_audio_win;
	hooks->supported_files_hook = supported_files_win;
	hooks->close_audio_hook = close_audio_win;
	hooks->load_sound_hook = load_sound_win;
	hooks->unload_sound_hook = unload_sound_win;
	hooks->play_sound_hook = play_sound_win;

	/* Success */
	return (0);
}
#endif /* SOUND */


/**
 * Resize a window
 */
static void term_window_resize(const term_data *td)
{
	/* Require window */
	if (!td->w) return;

	/* Resize the window */
	SetWindowPos(td->w, 0, 0, 0, td->size_wid, td->size_hgt,
				 SWP_NOMOVE | SWP_NOZORDER);

	/* Redraw later */
	InvalidateRect(td->w, NULL, true);
}


/**
 * Remove a font, given its filename.
 */
static void term_remove_font(const char *name)
{
	char buf[1024];

	/* Build path to the file */
	my_strcpy(buf, ANGBAND_DIR_FONTS, sizeof(buf));
	my_strcat(buf, "\\", sizeof(buf));
	my_strcat(buf, name, sizeof(buf));

	/* Remove it */
	RemoveFontResource(buf);

	/* Notify other applications of the change  XXX */
	PostMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);

	return;
}


/**
 * Force the use of a new "font file" for a term_data
 *
 * This function may be called before the "window" is ready
 *
 * This function returns zero only if everything succeeds.
 *
 * Note that the "font name" must be capitalized!!!
 */
static errr term_force_font(term_data *td, const char *path)
{
	int i;

	int wid, hgt;

	char *base;

	char buf[1024];


	/* Check we have a path */
	if (!path) return (1);


	/* Forget the old font (if needed) */
	if (td->font_id) DeleteObject(td->font_id);

	/* Forget old font */
	if (td->font_file) {
		bool used = false;

		/* Scan windows */
		for (i = 0; i < MAX_TERM_DATA; i++) {
			/* Check "screen" */
			if ((td != &data[i]) && (data[i].font_file) &&
			    (streq(data[i].font_file, td->font_file))) {
				used = true;
			}
		}

		/* Remove unused font resources */
		if (!used) term_remove_font(td->font_file);

		/* Free the old name */
		string_free(td->font_file);

		/* Forget it */
		td->font_file = NULL;
	}



	/* Local copy */
	my_strcpy(buf, path, sizeof(buf));

	/* Analyze font path */
	base = analyze_font(buf, &wid, &hgt);

	/* Verify suffix */
	if (!suffix(base, ".FON")) return (1);

	/* Verify file */
	if (!file_exists(buf)) return (1);


	/* Load the new font */
	if (!AddFontResourceEx(buf, FR_PRIVATE, 0)) return (1);

	/* Save new font name */
	td->font_file = string_make(base);

	/* Remove the "suffix" */
	base[strlen(base)-4] = '\0';

	/* Create the font (using the 'base' of the font file name!) */
	td->font_id = CreateFont(hgt, wid, 0, 0, FW_DONTCARE, 0, 0, 0,
	                         ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                         FIXED_PITCH | FF_DONTCARE, base);

	/* Hack -- Unknown size */
	if (!wid || !hgt)
	{
		HDC hdcDesktop;
		HFONT hfOld;
		TEXTMETRIC tm;

		/* all this trouble to get the cell size */
		hdcDesktop = GetDC(HWND_DESKTOP);
		hfOld = SelectObject(hdcDesktop, td->font_id);
		GetTextMetrics(hdcDesktop, &tm);
		SelectObject(hdcDesktop, hfOld);
		ReleaseDC(HWND_DESKTOP, hdcDesktop);

		/* Font size info */
		wid = tm.tmAveCharWidth;
		hgt = tm.tmHeight;
	}

	/* Save the size info */
	td->font_wid = wid;
	td->font_hgt = hgt;

	/* Success */
	return (0);
}



/**
 * Allow the user to change the font for this window.
 */
static void term_change_font(term_data *td)
{
	OPENFILENAME ofn;

	char tmp[1024] = "";

	/* Extract a default if possible */
	if (td->font_file) strcpy(tmp, td->font_file);

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = data[0].w;
	ofn.lpstrFilter = "Angband Font Files (*.fon)\0*.fon\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = ANGBAND_DIR_FONTS;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "fon";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn)) {
		/* Remember if tile size matched the font size. */
		bool tile_match_size = !use_graphics_nice
			&& td->tile_wid == td->font_wid
			&& td->tile_hgt == td->font_hgt;

		/* Force the font */
		if (term_force_font(td, tmp)) {
			/* Access the standard font file */
			path_build(tmp, sizeof(tmp), ANGBAND_DIR_FONTS, DEFAULT_FONT);

			/* Force the use of that font */
			(void)term_force_font(td, tmp);

			/* Reset the tile info */
			td->tile_wid = td->font_wid;
			td->tile_hgt = td->font_hgt;
		}

		/* HACK - Assume bizarre */
		td->bizarre = true;

		/*
		 * Reset the tile info (if not already set or if matching
		 * the font size.
		 */
		if (!td->tile_wid || !td->tile_hgt || tile_match_size) {
			td->tile_wid = td->font_wid;
			td->tile_hgt = td->font_hgt;
		}

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}
}


static void windows_map_aux(void);


/**
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
	if (td->map_active) {
		/* Redraw the map */
		windows_map_aux();
	} else {
		/* Activate the term */
		Term_activate(&td->t);

		/* Redraw the contents */
		Term_redraw();

		/* Restore the term */
		Term_activate(term_screen);
	}
}


/**
 * Hack -- redraw a term_data
 */
static void term_data_redraw_section(term_data *td, int x1, int y1,
									 int x2, int y2)
{
	/* Activate the term */
	Term_activate(&td->t);

	/* Redraw the area */
	Term_redraw_section(x1, y1, x2, y2);

	/* Restore the term */
	Term_activate(term_screen);
}



/**
 * ------------------------------------------------------------------------
 *  Function hooks needed by "Term"
 * ------------------------------------------------------------------------ */



#if 0

/**
 * Initialize a new Term
 */
static void Term_init_win(term *t)
{
	/* XXX Unused */
}


/**
 * Nuke an old Term
 */
static void Term_nuke_win(term *t)
{
	/* XXX Unused */
}

#endif /* 0 */


static errr Term_pict_win(int x, int y, int n,
						  const int *ap, const wchar_t *cp,
						  const int *tap, const wchar_t *tcp);
static errr Term_pict_win_alpha(int x, int y, int n,
								const int *ap, const wchar_t *cp,
								const int *tap, const wchar_t *tcp);

/**
 * React to global changes
 */
static errr Term_xtra_win_react(void)
{
	int i;

	/* Get the main window */
	term_data *td = &data[0];

	/* Simple or complex color */
	if (colors16) {
		/* Save the default colors */
		for (i = 0; i < MAX_COLORS; i++) {
			/* Simply accept the desired colors */
			win_pal[i] = angband_color_table[i][0];
		}
	} else {
		COLORREF code;

		uint8_t rv, gv, bv;

		bool change = false;

		/* Save the default colors */
		for (i = 0; i < MAX_COLORS; i++) {
			/* Extract desired values */
			rv = angband_color_table[i][1];
			gv = angband_color_table[i][2];
			bv = angband_color_table[i][3];

			if (gamma_correction > 0) {
				rv = gamma_table[rv];
				gv = gamma_table[gv];
				bv = gamma_table[bv];
			}

			/* Extract a full color code */
			code = PALETTERGB(rv, gv, bv);

			/* Activate changes */
			if (win_clr[i] != code) {
				/* Note the change */
				change = true;

				/* Apply the desired color */
				win_clr[i] = code;
			}
		}

		/* Activate the palette if needed */
		if (change) (void)new_palette();
	}

	/* Handle "arg_graphics_nice" */
	if (use_graphics_nice != arg_graphics_nice) {
		/* Change setting */
		use_graphics_nice = arg_graphics_nice;

		/* HACK - Assume bizarre */
		td->bizarre = true;

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}

	/* Handle "arg_graphics" */
	if (use_graphics != arg_graphics) {
		/* Free the bitmap stuff */
		FreeDIB(&infGraph);
		FreeDIB(&infMask);

		/* Initialize (if needed) */
		if (arg_graphics && !init_graphics()) {
			/* Warning */
			plog("Cannot initialize graphics!");

			/* Cannot enable */
			arg_graphics = GRAPHICS_NONE;
		} else {
			if (alphablend || overdraw) {
				td->t.pict_hook = Term_pict_win_alpha;
			} else {
				td->t.pict_hook = Term_pict_win;
			}
		}
		td->t.dblh_hook = (overdraw) ? is_dh_tile : NULL;

		/* make sure the current graphics mode is set */
		current_graphics_mode = get_graphics_mode(arg_graphics);

		/* Change setting */
		use_graphics = arg_graphics;

		if (use_graphics_nice) {
			/* HACK - Assume bizarre */
			td->bizarre = true;
                  
			/* Analyze the font */
			term_getsize(td);
			
			/* Resize the window */
			term_window_resize(td);
		}

		/* Reset visuals */
		if (character_dungeon) reset_visuals(true);
	}

	/* Handle "change_tilesize" */
	if (change_tilesize) {
		/* Reset visuals */
		reset_visuals(true);

		/* Reset the panel */
		verify_panel();

		/* Reset the flag */
		change_tilesize = false;
	}


	/* Clean up windows */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		term *old = Term;

		term_data *td = &data[i];

		/* Update resized windows */
		if ((td->cols != td->t.wid) || (td->rows != td->t.hgt)) {
			/* Activate */
			Term_activate(&td->t);

			/* Hack -- Resize the term */
			Term_resize(td->cols, td->rows);

			/* Redraw the contents */
			Term_redraw();

			/* Restore */
			Term_activate(old);
		}
	}


	/* Success */
	return (0);
}


/**
 * Process at least one event
 */
static errr Term_xtra_win_event(int v)
{
	MSG msg;

	/* Wait or check for an event */
	if (v)
	{
		int i = 0;
		while (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			/* Do animation updates (once every ten iterations), then
			 * sleep 0.02s and try again */
			if (i == 0) idle_update();
			Sleep(20);
			i = (i + 1) % 10;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} else if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Success */
	return 0;
}


/**
 * Process all pending events
 */
static errr Term_xtra_win_flush(void)
{
	MSG msg;

	/* Process all pending events */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Success */
	return (0);
}


/**
 * Hack -- clear the screen
 *
 * Make this more efficient XXX XXX XXX
 */
static errr Term_xtra_win_clear(void)
{
	term_data *td = (term_data*)(Term->data);

	HDC hdc;
	RECT rc;
	HBRUSH brush;

	/* Rectangle to erase */
	GetClientRect(td->w, &rc);

	/* Erase it */
	hdc = GetDC(td->w);
	brush = CreateSolidBrush((colors16) ? RGB(0, 0, 0) : win_clr[0]);
	FillRect(hdc, &rc, brush);
	DeleteObject(brush);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/**
 * Hack -- make a noise
 */
static errr Term_xtra_win_noise(void)
{
	MessageBeep(MB_ICONASTERISK);
	return (0);
}

/**
 * Delay for "x" milliseconds
 */
static int Term_xtra_win_delay(int v)
{
	/* Sleep */
	if (v > 0) Sleep(v);

	/* Success */
	return (0);
}


/**
 * Do a "special thing"
 */
static errr Term_xtra_win(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a bell sound */
		case TERM_XTRA_NOISE:
		{
			return (Term_xtra_win_noise());
		}

		/* Process random events */
		case TERM_XTRA_BORED:
		{
			return (Term_xtra_win_event(0));
		}

		/* Process an event */
		case TERM_XTRA_EVENT:
		{
			return (Term_xtra_win_event(v));
		}

		/* Flush all events */
		case TERM_XTRA_FLUSH:
		{
			return (Term_xtra_win_flush());
		}

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		{
			return (Term_xtra_win_clear());
		}

		/* React to global changes */
		case TERM_XTRA_REACT:
		{
			return (Term_xtra_win_react());
		}

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
		{
			return (Term_xtra_win_delay(v));
		}
	}

	/* Oops */
	return 1;
}



/**
 * Low level graphics (Assumes valid input).
 *
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_curs_win(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	RECT rc;
	HDC hdc;

	int tile_wid, tile_hgt;

	if (td->map_active) {
		tile_wid = td->map_tile_wid;
		tile_hgt = td->map_tile_hgt;
	} else {
		tile_wid = td->tile_wid;
		tile_hgt = td->tile_hgt;
	}

	/* Frame the grid */
	rc.left = x * tile_wid + td->size_ow1;
	rc.right = rc.left + tile_wid;
	rc.top = y * tile_hgt + td->size_oh1;
	rc.bottom = rc.top + tile_hgt;

	/* Cursor is done as a yellow "box" */
	hdc = GetDC(td->w);
	FrameRect(hdc, &rc, hbrYellow);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/**
 * Low level graphics (Assumes valid input).
 *
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_bigcurs_win(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	RECT rc;
	HDC hdc;

	int tile_wid, tile_hgt;

	if (td->map_active) {
		/* Normal cursor in map window */
		Term_curs_win(x, y);
		return 0;
	} else {
		tile_wid = td->tile_wid;
		tile_hgt = td->tile_hgt;
	}

	/* Frame the grid */
	rc.left = x * tile_wid + td->size_ow1;
        rc.right = rc.left + tile_width * tile_wid;
	rc.top = y * tile_hgt + td->size_oh1;
        rc.bottom = rc.top + tile_height * tile_hgt;

	/* Cursor is done as a yellow "box" */
	hdc = GetDC(td->w);
	FrameRect(hdc, &rc, hbrYellow);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/**
 * Help Term_wipe_win(), Term_pict_win(), and Term_pict_win_alpha():
 * fill a nc x nr block of characters with the color c where the upper
 * left corner of the block is at (x,y).
 */
static errr Term_wipe_win_helper(int x, int y, int nc, int nr, COLORREF c)
{
	term_data *td = (term_data*)(Term->data);

	HDC hdc;
	RECT rc;

	/* Rectangle to erase in client coords */
	rc.left = x * td->tile_wid + td->size_ow1;
	rc.right = rc.left + nc * td->tile_wid;
	rc.top = y * td->tile_hgt + td->size_oh1;
	rc.bottom = rc.top + nr * td->tile_hgt;

	hdc = GetDC(td->w);
	SetBkColor(hdc, c);
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/**
 * Low level graphics (Assumes valid input).
 *
 * Erase a "block" of "n" characters starting at (x,y).
 */
static errr Term_wipe_win(int x, int y, int n)
{
	return Term_wipe_win_helper(x, y, n, 1,
		(colors16) ? RGB(0, 0, 0) : win_clr[0]);
}


/**
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * All "graphic" data is handled by "Term_pict_win()", below.
 *
 * One would think there is a more efficient method for telling a window
 * what color it should be using to draw with, but perhaps simply changing
 * it every time is not too inefficient.  XXX XXX XXX
 */
static errr Term_text_win(int x, int y, int n, int a, const wchar_t *s)
{
	term_data *td = (term_data*)(Term->data);
	RECT rc;
	HDC hdc;

	/* Total rectangle */
	rc.left = x * td->tile_wid + td->size_ow1;
	rc.right = rc.left + n * td->tile_wid;
	rc.top = y * td->tile_hgt + td->size_oh1;
	rc.bottom = rc.top + td->tile_hgt;

	/* Acquire DC */
	hdc = GetDC(td->w);

	/* Foreground color */
	if (colors16) {
		SetTextColor(hdc, PALETTEINDEX(win_pal[a % MAX_COLORS]));
		/* Background color */
		SetBkColor(hdc, RGB(0, 0, 0));
	} else {
		if (paletted)
			SetTextColor(hdc, win_clr[(a % MAX_COLORS) & 0x1F]);
		else
			SetTextColor(hdc, win_clr[a % MAX_COLORS]);

		/* Determine the background colour - from Sil */
		switch (a / MAX_COLORS)
		{
			case BG_SAME:
				/* Background same as foreground*/
				SetBkColor(hdc, win_clr[a % MAX_COLORS]);
				break;
			case BG_DARK:
				/* Highlight Background */
				SetBkColor(hdc, win_clr[COLOUR_SHADE]);
				break;
			case BG_BLACK:
			default:
				/* Default Background */
				SetBkColor(hdc, win_clr[0]);
				break;
		}
	}

	/* Use the font */
	SelectObject(hdc, td->font_id);

	/* Bizarre or normal size */
	if (td->bizarre ||
	    (td->tile_hgt != td->font_hgt) ||
		(td->tile_wid != td->font_wid)) {
		int i;

		/* Erase complete rectangle */
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

		/* New rectangle */
		rc.left += ((td->tile_wid - td->font_wid) / 2);
		rc.right = rc.left + td->font_wid;
		rc.top += ((td->tile_hgt - td->font_hgt) / 2);
		rc.bottom = rc.top + td->font_hgt;

		/* Dump each character */
		for (i = 0; i < n; i++) {
			/* Dump the text */
			ExtTextOutW(hdc, rc.left, rc.top, 0, &rc,
			            s+i, 1, NULL);

			/* Advance */
			rc.left += td->tile_wid;
			rc.right += td->tile_wid;
		}
	} else {
		/* Dump the text */
		ExtTextOutW(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
		           s, n, NULL);
	}

	/* Release DC */
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/**
 * Low level graphics.  Assumes valid input.
 *
 * Draw an array of "special" attr/char pairs at the given location.
 *
 * We use the "Term_pict_win()" function for "graphic" data, which are
 * encoded by setting the "high-bits" of both the "attr" and the "char"
 * data.  We use the "attr" to represent the "row" of the main bitmap,
 * and the "char" to represent the "col" of the main bitmap.  The use
 * of this function is induced by the "higher_pict" flag.
 *
 * If "graphics" is not available, we simply "wipe" the given grids.
 */
static errr Term_pict_win(int x, int y, int n,
						  const int *ap, const wchar_t *cp,
						  const int *tap, const wchar_t *tcp)
{
	term_data *td = (term_data*)(Term->data);

	int i;
	int mw, mh;
	int x1, y1, w1, h1;
	int x2, y2, w2, h2, tw2, th2;
	int x3, y3;

	HDC hdcMask;
	HDC hdc;
	HDC hdcSrc;
	HBITMAP hbmSrcOld;

	/* Size of bitmap cell */
	w1 = infGraph.CellWidth;
	h1 = infGraph.CellHeight;

	/* Size of window cell */
	if (td->map_active) {
		w2 = td->map_tile_wid;
		h2 = td->map_tile_hgt;
		mw = 1;
		mh = 1;
	} else {
		w2 = td->tile_wid;
		h2 = td->tile_hgt;

		/* Large tile mode */
		mw = tile_width;
		mh = tile_height;
	}
	tw2 = mw * w2;
	th2 = mh * h2;

	/* Erase the grids */
	Term_wipe_win_helper(x, y, n * mw, mh, RGB(0, 0, 0));

	/* Location of window cell */
	x2 = x * w2 + td->size_ow1;
	y2 = y * h2 + td->size_oh1;

	/* Info */
	hdc = GetDC(td->w);

	/* More info */
	hdcSrc = CreateCompatibleDC(hdc);
	hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

	if (infMask.hBitmap) {
		hdcMask = CreateCompatibleDC(hdc);
		SelectObject(hdcMask, infMask.hBitmap);
	} else {
		hdcMask = NULL;
	}

	/* Draw attr/char pairs */
	for (i = n-1; i >= 0; i--, x2 -= w2) {
		int a = ap[i];
		wchar_t c = cp[i];

		/* Extract picture */
		int row = (a & 0x7F);
		int col = (c & 0x7F);

		/* Location of bitmap cell */
		x1 = col * w1;
		y1 = row * h1;

		if (hdcMask) {
			x3 = (tcp[i] & 0x7F) * w1;
			y3 = (tap[i] & 0x7F) * h1;

			/* Perfect size */
			if ((w1 == tw2) && (h1 == th2)) {
				/* Copy the terrain picture from the bitmap to the window */
				BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x3, y3, SRCCOPY);

				/* Only draw if terrain and overlay are different */
				if ((x1 != x3) || (y1 != y3)) {
					/* Mask out the tile */
					BitBlt(hdc, x2, y2, tw2, th2, hdcMask, x1, y1, SRCAND);

					/* Draw the tile */
					BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, SRCPAINT);
				}

			/* Need to stretch */
			} else {
				/* Set the correct mode for stretching the tiles */
				SetStretchBltMode(hdc, COLORONCOLOR);

				/* Copy the terrain picture from the bitmap to the window */
				StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x3, y3, w1, h1,
						   SRCCOPY);

				/* Only draw if terrain and overlay are different */
				if ((x1 != x3) || (y1 != y3)) {
					/* Mask out the tile */
					StretchBlt(hdc, x2, y2, tw2, th2, hdcMask, x1, y1, w1, h1,
							   SRCAND);

					/* Draw the tile */
					StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, w1, h1,
							   SRCPAINT);
				}
			}
		} else {
			/* Perfect size */
			if ((w1 == tw2) && (h1 == th2)) {
				/* Copy the picture from the bitmap to the window */
				BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, SRCCOPY);

			/* Need to stretch */
			} else {
				/* Set the correct mode for stretching the tiles */
				SetStretchBltMode(hdc, COLORONCOLOR);

				/* Copy the picture from the bitmap to the window */
				StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, w1, h1,
						   SRCCOPY);
			}
		}
	}

	/* Release */
	SelectObject(hdcSrc, hbmSrcOld);
	DeleteDC(hdcSrc);

	if (hdcMask) {
		/* Release */
		SelectObject(hdcMask, hbmSrcOld);
		DeleteDC(hdcMask);
	}

	/* Release */
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}

/**
 * Windows cannot naturally handle UTF-8 using the standard locale and
 * C library routines, such as mbstowcs().
 *
 * We assume external files are in UTF-8, and explicitly convert.
 *
 * MultiByteToWideChar returns number of wchars, including terminating L'\0'
 *     mbstowcs requires the count without the terminating L'\0'
 * dest == NULL corresponds to querying for the size needed, achieved in the
 *     Windows fn by setting the dstlen (last) param to 0.
 * If n is too small for all the chars in src, the Windows fn fails, but we
 *     require success and a partial conversion. So allocate space for it to
 *     succeed, and do the partial copy into dest.
 */
size_t Term_mbstowcs_win(wchar_t *dest, const char *src, int n)
{
	int res;
	int required;
	wchar_t *tmp;

	if (dest) {
		res = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1,
								  dest, n);
		if (res)
			return (size_t)(res - 1);
		else {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
											   src, -1, NULL, 0);
				tmp = malloc(required * sizeof(wchar_t));
				MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, tmp,
									required);
				memcpy(dest, tmp, n * sizeof(wchar_t));
				free(tmp);
				return n;
			} else {
				return (size_t)-1;
			}
		}
	} else {
		return (size_t)(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
											src, -1, NULL, 0) - 1);
	}
}


int Term_wcsz_win(void)
{
	/*
	 * Any Unicode code point is representable by at most 4 bytes in UTF-8.
	 */
	return 4;
}


/**
 * Convert back to UTF-8 from a wide character.
 *
 * This is a necessary counterpart to Term_mbstowcs_win():  since the forward
 * conversion from UTF-8 to wide characters was overridden, need to override
 * the reverse conversion so that file output from screen captures and text
 * blocks properly translates any multibyte characters.
 */
int Term_wctomb_win(char *s, wchar_t wchar)
{
	/*
	 * If only want compatibility with Vista and later, could use
	 * WC_ERR_INVALID_CHARS rather than zero for the second argument;
	 * that would give an error if converting something not representable
	 * in UTF-8.
	 */
	int n = WideCharToMultiByte(CP_UTF8, 0, &wchar, 1, s, Term_wcsz_win(),
		NULL, NULL);
	return (n > 0) ? n : -1;
}


/**
 * Return whether a wide character is printable.
 *
 * This is a necessary counterpart to Term_mbstowcs_win() so that screening
 * of wide characters in the core's text_out_to_screen() is consistent with what
 * Term_mbstowcs_win() does.
 */
int Term_iswprint_win(wint_t wc)
{
	/*
	 * It's a UTF-16 value, but can cast and test as UTF-32 (if it's the
	 * leading part of a surrogate pair it'll be treated as unprintable
	 * which is desirable:  on Windows, ui-term as it is can't handle
	 * characters that have to be encoded as surrogate pairs in UTF-16).
	 */
	return utf32_isprint((uint32_t) wc);
}


#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA     0x01
#endif
static errr Term_pict_win_alpha(int x, int y, int n,
								const int *ap, const wchar_t *cp,
								const int *tap, const wchar_t *tcp)
{
	term_data *td = (term_data*)(Term->data);
	int dhrclip = (overdraw) ?
		Term_get_first_tile_row(Term) + tile_height - 1 : 0;
	int i;
	int mw, mh;
	int x1, y1, w1, h1;
	int x2, y2, w2, h2, tw2, th2;
	int x3, y3;

	HDC hdc;
	HDC hdcSrc;
	HBITMAP hbmSrcOld;

	/* Size of bitmap cell */
	w1 = infGraph.CellWidth;
	h1 = infGraph.CellHeight;

	/* Size of window cell */
	if (td->map_active) {
		w2 = td->map_tile_wid;
		h2 = td->map_tile_hgt;
		mw = 1;
		mh = 1;
	} else {
		w2 = td->tile_wid;
		h2 = td->tile_hgt;

		/* Large tile mode */
		mw = tile_width;
		mh = tile_height;
	}
	tw2 = mw * w2;
	th2 = mh * h2;

	/* Erase the grids */
	Term_wipe_win_helper(x, y, n * mw, mh, RGB(0, 0, 0));

	/* Location of window cell */
	x2 = x * w2 + td->size_ow1;
	y2 = y * h2 + td->size_oh1;

	/* Info */
	hdc = GetDC(td->w);

	/* More info */
	hdcSrc = CreateCompatibleDC(hdc);
	hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

	/* Draw attr/char pairs */
	for (i = n-1; i >= 0; i--, x2 -= w2) {
		int a = ap[i];
		wchar_t c = cp[i];

		/* Extract picture */
		int row = (a & 0x7F);
		int col = (c & 0x7F);
		int trow = (tap[i] & 0x7F);

		/* Location of bitmap cell */
		x1 = col * w1;
		y1 = row * h1;

		x3 = (tcp[i] & 0x7F) * w1;
		y3 = trow * h1;
 
		/* Set the correct mode for stretching the tiles */
		SetStretchBltMode(hdc, COLORONCOLOR);

		/* Perfect size */
		if ((w1 == tw2) && (h1 == th2)) {
			/* Copy the terrain picture from the bitmap to the window */
			BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x3, y3, SRCCOPY);
		} else {
			/* Copy the terrain picture from the bitmap to the window */
			StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x3, y3, w1, h1, SRCCOPY);
		}

		if (overdraw && trow >= overdraw && y > dhrclip &&
				trow <= overdrawmax) {
			AlphaBlend(hdc, x2, y2-th2, tw2, th2, hdcSrc, x3, y3-h1, w1, h1,
					   blendfn);
		}

		/* Only draw if terrain and overlay are different */
		if ((x1 != x3) || (y1 != y3))
		{
			/* Copy the picture from the bitmap to the window */
			if (overdraw && row >= overdraw && y > dhrclip &&
					row <= overdrawmax) {
				AlphaBlend(hdc, x2, y2-th2, tw2, th2*2, hdcSrc, x1, y1-h1, w1,
						   h1*2, blendfn);
			} else {
				AlphaBlend(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, w1, h1,
						   blendfn);
			}
		}
	}

	/* Release */
	SelectObject(hdcSrc, hbmSrcOld);
	DeleteDC(hdcSrc);

	/* Release */
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


static void windows_map_aux(void)
{
	term_data *td = &data[0];
	int a;
	wchar_t c;
	int x, min_x, max_x;
	int y, min_y, max_y;
	int ta;
	wchar_t tc;

	td->map_tile_wid = (td->tile_wid * td->cols) / cave->width;
	td->map_tile_hgt = (td->tile_hgt * td->rows) / cave->height;

	min_x = 0;
	min_y = 0;
	max_x = cave->width;
	max_y = cave->height;

	/* Draw the map */
	for (x = min_x; x < max_x; x++) {
		for (y = min_y; y < max_y; y++) {
			struct grid_data g;

			map_info(loc(x, y), &g);
			grid_data_as_text(&g, &a, &c, &ta, &tc);

			/* Ignore non-graphics */
			if (a & 0x80)
				Term_pict_win(x - min_x, y - min_y, 1, &a, &c, &ta, &tc);
		}
	}

	/* Highlight the player */
	Term_curs_win(player->grid.x - min_x, player->grid.y - min_y);
}


/**
 * MEGA_HACK - Display a graphical map of the dungeon.
 */
static void windows_map(void)
{
	term_data *td = &data[0];
	ui_event ch;

	/* Only in graphics mode since the fonts can't be scaled */
	if (!use_graphics) return;

	/* Prevent various menu-actions from working */
	initialized = false;

	/* Clear screen */
	Term_xtra_win_clear();

	td->map_active = true;

	/* Draw the map */
	windows_map_aux();

	/* Wait for a keypress, flush key buffer */
	Term_inkey(&ch, true, true);
	Term_flush();

	/* Switch off the map display */
	td->map_active = false;

	/* Restore screen */
	Term_xtra_win_clear();
	Term_redraw();

	/* We are ready again */
	initialized = true;
}


/**
 * ------------------------------------------------------------------------
 *  Other routines
 * ------------------------------------------------------------------------ */



/**
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	t->soft_cursor = true;

	/* Differentiate between BS/^h, Tab/^i, etc. */
	t->complex_input = true;

	/* Use "Term_pict" for "graphic" data */
	t->higher_pict = true;

	/* Erase with "white space" */
	t->attr_blank = COLOUR_WHITE;
	t->char_blank = ' ';

#if 0
	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_win;
	t->nuke_hook = Term_nuke_win;
#endif /* 0 */

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_win;
	t->curs_hook = Term_curs_win;
	t->bigcurs_hook = Term_bigcurs_win;
	t->wipe_hook = Term_wipe_win;
	t->text_hook = Term_text_win;
	t->pict_hook = Term_pict_win;
	t->dblh_hook = NULL;

	/* Remember where we came from */
	t->data = td;
}


/**
 * Create the windows
 *
 * First, instantiate the "default" values, then read the "ini_file"
 * to over-ride selected values, then create the windows, and fonts.
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette.  XXX XXX XXX
 */
static void init_windows(void)
{
	int i;

	term_data *td;

	char buf[1024];

	WINDOWPLACEMENT lpwndpl;

	MENUITEMINFO mii;
	HMENU hm;
	graphics_mode *mode;

	/* Main window */
	td = &data[0];
	memset(td, 0, sizeof(term_data));
	td->s = angband_term_name[0];
	td->keys = 1024;
	td->rows = 24;
	td->cols = 80;
	td->visible = true;
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh1 = 2;
	td->size_oh2 = 2;
	td->pos_x = 30;
	td->pos_y = 20;

	/* Sub windows */
	for (i = 1; i < MAX_TERM_DATA; i++) {
		td = &data[i];
		memset(td, 0, sizeof(term_data));
		td->s = angband_term_name[i];
		td->keys = 16;
		td->rows = 24;
		td->cols = 80;
		td->visible = false;
		td->size_ow1 = 1;
		td->size_ow2 = 1;
		td->size_oh1 = 1;
		td->size_oh2 = 1;
		td->pos_x = (7 - i) * 30;
		td->pos_y = (7 - i) * 20;
	}


	/* Load prefs */
	load_prefs();


	/* Main window (need these before term_getsize gets called) */
	td = &data[0];
	td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU |
	               WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION |
	               WS_VISIBLE);
	if (td->maximized) td->dwStyle |= WS_MAXIMIZE;
	td->dwExStyle = 0;
	td->visible = true;

	/* Sub windows (need these before term_getsize gets called) */
	for (i = 1; i < MAX_TERM_DATA; i++) {
		td = &data[i];
		td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU | WS_CAPTION);
		td->dwExStyle = (WS_EX_TOOLWINDOW);
	}


	/* All windows */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		td = &data[i];

		/* Access the standard font file */
		path_build(buf, sizeof(buf), ANGBAND_DIR_FONTS, td->font_want);

		/* Activate the chosen font */
		if (term_force_font(td, buf)) {
			/* Access the standard font file */
			path_build(buf, sizeof(buf), ANGBAND_DIR_FONTS, DEFAULT_FONT);

			/* Force the use of that font */
			(void)term_force_font(td, buf);

			/* Oops */
			td->tile_wid = 8;
			td->tile_hgt = 13;

			/* HACK - Assume bizarre */
			td->bizarre = true;
		}

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}


	/* Sub windows (reverse order) */
	for (i = MAX_TERM_DATA - 1; i >= 1; --i) {
		td = &data[i];

		my_td = td;
		td->w = CreateWindowEx(td->dwExStyle, AngList,
		                       td->s, td->dwStyle,
		                       td->pos_x, td->pos_y,
		                       td->size_wid, td->size_hgt,
		                       HWND_DESKTOP, NULL, hInstance, NULL);
		my_td = NULL;
		if (!td->w) quit("Failed to create sub-window");

		term_data_link(td);
		angband_term[i] = &td->t;

		if (td->visible) {
			td->size_hack = true;
			ShowWindow(td->w, SW_SHOW);
			td->size_hack = false;

			/* Activate the window */
			SetActiveWindow(td->w);

			/* Bring window to top, place it correctly */
			lpwndpl.length = sizeof(WINDOWPLACEMENT);
			lpwndpl.showCmd = SW_SHOWNORMAL;
			lpwndpl.rcNormalPosition = (RECT) { td->pos_x, td->pos_y,
												td->pos_x + td->size_wid,
												td->pos_y + td->size_hgt };
			SetWindowPlacement(td->w, &lpwndpl);
			//SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	}


	/* Main window */
	td = &data[0];

	/* Main window */
	my_td = td;
	td->w = CreateWindowEx(td->dwExStyle, AppName,
	                       td->s, td->dwStyle,
	                       td->pos_x, td->pos_y,
	                       td->size_wid, td->size_hgt,
	                       HWND_DESKTOP, NULL, hInstance, NULL);
	my_td = NULL;
	if (!td->w) quit_fmt("Failed to create %s window", VERSION_NAME);

	term_data_link(td);
	term_screen = &td->t;
	text_mbcs_hook = Term_mbstowcs_win;
	text_wctomb_hook = Term_wctomb_win;
	text_wcsz_hook = Term_wcsz_win;
	text_iswprint_hook = Term_iswprint_win;

	/* Activate the main window */
	SetActiveWindow(td->w);

	/* Bring window to top, place it correctly */
	lpwndpl.length = sizeof(WINDOWPLACEMENT);
	lpwndpl.showCmd = SW_SHOWNORMAL;
	lpwndpl.rcNormalPosition = (RECT) { td->pos_x, td->pos_y,
										td->pos_x + td->size_wid,
										td->pos_y + td->size_hgt };
	SetWindowPlacement(td->w, &lpwndpl);
	/* Bring main window back to top */
	//SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	if (gamma_correction > 0)
		build_gamma_table(gamma_correction);

	/* New palette XXX XXX XXX */
	(void)new_palette();


	/* Create a "brush" for drawing the "cursor" */
	hbrYellow = CreateSolidBrush(win_clr[COLOUR_YELLOW]);

	/* Populate the graphic options sub menu with the graphics modes */
	hm = GetMenu(data[0].w);
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mode = graphics_modes;
	while (mode) {
		if (mode->grafID != GRAPHICS_NONE) {
			mii.wID = mode->grafID + IDM_OPTIONS_GRAPHICS_NONE;
			mii.dwTypeData = mode->menuname;
			mii.cch = strlen(mode->menuname);
			InsertMenuItem(hm,IDM_OPTIONS_GRAPHICS_NICE, false, &mii);
		}
		mode = mode->pNext;
	}
	/* mii.cbSize = sizeof(MENUITEMINFO); */
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	mii.wID = 399;
	mii.dwTypeData = 0;
	mii.cch = 0;
	InsertMenuItem(hm,IDM_OPTIONS_GRAPHICS_NICE, false, &mii);

	/* setup the alpha blending function */
	blendfn.BlendOp = AC_SRC_OVER;
	blendfn.BlendFlags = 0;
	blendfn.AlphaFormat = AC_SRC_ALPHA;
	blendfn.SourceConstantAlpha = 255;


	/* Process pending messages */
	(void)Term_xtra_win_flush();
}


#ifdef USE_SAVER

/**
 * Stop the screensaver
 */
static void stop_screensaver(void)
{
	if (screensaver)
		SendMessage(data[0].w, WM_CLOSE, 0, 0);
	else
		SendMessage(data[0].w, WM_COMMAND, IDM_OPTIONS_SAVER, 0);
}

#endif /* USE_SAVER */


/**
 * Prepare the menus
 */
static void setup_menus(void)
{
	size_t i;
	graphics_mode *mode;

	HMENU hm = GetMenu(data[0].w);

#ifdef USE_SAVER
	main_menu = hm;
#endif /* USE_SAVER */

	/* Menu "File", Disable all */
	EnableMenuItem(hm, IDM_FILE_NEW,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_OPEN,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_SAVE,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_EXIT,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	EnableMenuItem(hm, IDM_WINDOW_OPT,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* No character available */
	if (!character_generated) {
		/* Menu "File", Item "New" */
		EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_ENABLED);

		/* Menu "File", Item "Open" */
		EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_ENABLED);
	}

	/* A character available */
	if (game_in_progress && character_generated && inkey_flag) {
		/* Menu "File", Item "Save" */
		EnableMenuItem(hm, IDM_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);
		/* Allow accessing the window options */
		EnableMenuItem(hm, IDM_WINDOW_OPT, MF_BYCOMMAND | MF_ENABLED);
	}

	if (!game_in_progress || !character_generated || inkey_flag) {
		/* Menu "File", Item "Exit" */
		EnableMenuItem(hm, IDM_FILE_EXIT, MF_BYCOMMAND | MF_ENABLED);
	}


	/* Menu "Window::Visibility" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		CheckMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		              (data[i].visible ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		               MF_BYCOMMAND | MF_ENABLED);
	}

	/* Menu "Window::Font" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible) {
			EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Bizarre Display" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		CheckMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
		              (data[i].bizarre ? MF_CHECKED : MF_UNCHECKED));

		if (data[i].visible) {
			EnableMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Increase Tile Width" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible) {
			EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Decrease Tile Width" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible) {
			EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Increase Tile Height" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible) {
			EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Decrease Tile Height" */
	for (i = 0; i < MAX_TERM_DATA; i++) {
		EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible) {
			EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Options", disable all */
	mode = graphics_modes;
	while (mode) {
		EnableMenuItem(hm, mode->grafID + IDM_OPTIONS_GRAPHICS_NONE,
					   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		mode = mode->pNext;
	} 

	EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_NICE,
				   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	for (i = IDM_OPTIONS_TILE_1x1; i < IDM_OPTIONS_TILE_16x16; i++) {
		EnableMenuItem(hm, i, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	for (i = IDM_TILE_FONT; i < IDM_TILE_12X13; i++) {
		EnableMenuItem(hm, i, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	EnableMenuItem(hm, IDM_TILE_12X20,
				   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_TILE_16X25,
				   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_OPTIONS_LOW_PRIORITY,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* Menu "Options", Item "Map" */
	if (inkey_flag && initialized && (use_graphics != GRAPHICS_NONE))
		EnableMenuItem(GetMenu(data[0].w), IDM_OPTIONS_MAP,
					   MF_BYCOMMAND | MF_ENABLED);
	else
		EnableMenuItem(GetMenu(data[0].w), IDM_OPTIONS_MAP,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* Menu "Options", update all */
	mode = graphics_modes;
	while (mode) {
		CheckMenuItem(hm, mode->grafID + IDM_OPTIONS_GRAPHICS_NONE,
					  (arg_graphics == mode->grafID ?
					   MF_CHECKED : MF_UNCHECKED));
		mode = mode->pNext;
	} 

	CheckMenuItem(hm, IDM_OPTIONS_GRAPHICS_NICE,
				  (arg_graphics_nice ? MF_CHECKED : MF_UNCHECKED));

	if ((tile_width == 1) && (tile_height == 1))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_1x1, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_1x1, MF_UNCHECKED);

	if ((tile_width == 2) && (tile_height == 1))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x1, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x1, MF_UNCHECKED);

	if ((tile_width == 2) && (tile_height == 2))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x2, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x2, MF_UNCHECKED);

	if ((tile_width == 3) && (tile_height == 1))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x1, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x1, MF_UNCHECKED);

	if ((tile_width == 3) && (tile_height == 3))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x3, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x3, MF_UNCHECKED);

	if ((tile_width == 4) && (tile_height == 2))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x2, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x2, MF_UNCHECKED);

	if ((tile_width == 4) && (tile_height == 4))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x4, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x4, MF_UNCHECKED);

	if ((tile_width == 6) && (tile_height == 3))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x3, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x3, MF_UNCHECKED);

	if ((tile_width == 6) && (tile_height == 6))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x6, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x6, MF_UNCHECKED);

	if ((tile_width == 8) && (tile_height == 4))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x4, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x4, MF_UNCHECKED);

	if ((tile_width == 8) && (tile_height == 8))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x8, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x8, MF_UNCHECKED);

	if ((tile_width == 16) && (tile_height == 8))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x8, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x8, MF_UNCHECKED);

	if ((tile_width == 16) && (tile_height == 16))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x16, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x16, MF_UNCHECKED);

	i = data[0].tile_hgt;
	if ((data[0].tile_wid == data[0].font_wid) && (i == data[0].font_hgt))
		CheckMenuItem(hm, IDM_TILE_FONT, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_FONT, MF_UNCHECKED);

	if ((data[0].tile_wid == 8) && (i == 16))
		CheckMenuItem(hm, IDM_TILE_08X16, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_08X16, MF_UNCHECKED);

#ifdef USE_SAVER
	CheckMenuItem(hm, IDM_OPTIONS_SAVER,
	              (hwndSaver ? MF_CHECKED : MF_UNCHECKED));
#endif /* USE_SAVER */

	CheckMenuItem(hm, IDM_OPTIONS_LOW_PRIORITY,
	              (low_priority ? MF_CHECKED : MF_UNCHECKED));

	if (inkey_flag && initialized) {
		/* Menu "Options", Item "Graphics" */
		mode = graphics_modes;
		while (mode) {
			if ((mode->grafID == 0) || (mode->file && mode->file[0])) {
				EnableMenuItem(hm, mode->grafID + IDM_OPTIONS_GRAPHICS_NONE, MF_ENABLED);
			}
			mode = mode->pNext;
		} 

		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_NICE, MF_ENABLED);

		for (i = IDM_OPTIONS_TILE_1x1; i < IDM_OPTIONS_TILE_16x16; i++) {
			EnableMenuItem(hm, i, MF_ENABLED);
		}
		for (i = IDM_TILE_FONT; i < IDM_TILE_12X13; i++) {
			EnableMenuItem(hm, i, MF_ENABLED);
		}

		EnableMenuItem(hm, IDM_TILE_12X20, MF_ENABLED);
		EnableMenuItem(hm, IDM_TILE_16X25, MF_ENABLED);
	}

#ifdef USE_SAVER
	/* Menu "Options", Item "ScreenSaver" */
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_ENABLED);
#endif /* USE_SAVER */

	EnableMenuItem(hm, IDM_OPTIONS_LOW_PRIORITY,
	               MF_BYCOMMAND | MF_ENABLED);
}


/**
 * Check for double clicked (or dragged) savefile
 *
 * Apparently, Windows copies the entire filename into the first
 * piece of the "command line string".  Perhaps we should extract
 * the "basename" of that filename and append it to the "save" dir.
 */
static void check_for_save_file(LPSTR cmd_line)
{
	char *s, *p;

	/* First arg */
	s = cmd_line;

	/* No args */
	if (!s || !*s) return;

	/* Next arg */
	p = strchr(s, ' ');

	/* Tokenize */
	if (p) *p = '\0';

	/* Extract filename */
	my_strcpy(savefile, s, sizeof(savefile));

	/* Validate the file */
	validate_file(savefile);
	
	/* Start game */
	game_in_progress = true;
	Term_fresh();
	play_game(false);
	quit(NULL);
}


#ifdef USE_SAVER

/**
 * Start the screensaver
 */
static void start_screensaver(void)
{
	bool file_exist;

	/* Set 'savefile' to a safe name */
	savefile_set_name(saverfilename, true, false);

	/* Does the savefile already exist? */
	file_exist = file_exists(savefile);

	/* Don't try to load a non-existant savefile */
	if (!file_exist) savefile[0] = '\0';

	/* Game in progress */
	game_in_progress = true;

	Term_fresh();

	/* Screensaver mode on */
	SendMessage(data[0].w, WM_COMMAND, IDM_OPTIONS_SAVER, 0);

	/* Low priority */
	SendMessage(data[0].w, WM_COMMAND, IDM_OPTIONS_LOW_PRIORITY, 0);


	/* Play game */
	play_game();
}

#endif /* USE_SAVER */


/**
 * Display a help file
 */
static void display_help(void)
{
	Term_keypress('?',0);
}


/**
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
	int i;

	term_data *td;

	OPENFILENAME ofn;

	/* Analyze */
	switch (wCmd)
	{
		/* New game */
		case IDM_FILE_NEW:
		{
			if (!initialized) {
				plog("You cannot do that yet...");
			} else if (game_in_progress) {
				plog("You can't start a new game while you're still playing!");
			} else {
				/* Start game */
				game_in_progress = true;
				Term_fresh();
				play_game(true);
				quit(NULL);
			}
			break;
		}

		/* Open game */
		case IDM_FILE_OPEN:
		{
			if (!initialized) {
				plog("You cannot do that yet...");
			} else if (game_in_progress) {
				plog("You can't open a new game while you're still playing!");
			} else {
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = data[0].w;
				ofn.lpstrFilter = "Save Files (*.)\0*\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = savefile;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = ANGBAND_DIR_SAVE;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileName(&ofn)) {
					/* Load 'savefile' */
					validate_file(savefile);

					if (monitor_existing_savefile()) {
						/* Start game */
						game_in_progress = true;
						Term_fresh();
						play_game(false);
						quit(NULL);
					}
				}
			}
			break;
		}

		/* Save game */
		case IDM_FILE_SAVE:
		{
			if (game_in_progress && character_generated && inkey_flag) {
				/* Hack -- Forget messages */
				msg_flag = false;

				/* Save the game */
				save_game();
			} else {
				/* Paranoia */
				plog("You may not do that right now.");
			}
			break;
		}

		/* Exit */
		case IDM_FILE_EXIT:
		{
			if (game_in_progress && character_generated) {
				/* Paranoia */
				if (!inkey_flag) {
					plog("You may not do that right now.");
					break;
				}

				/* Hack -- Forget messages */
				msg_flag = false;

				/* Save the game */
				save_game();
				close_game(true);
			}
			quit(NULL);
			break;
		}

		case IDM_WINDOW_VIS_0:
		{
			plog("You are not allowed to do that!");

			break;
		}

		/* Window visibility */
		case IDM_WINDOW_VIS_1:
		case IDM_WINDOW_VIS_2:
		case IDM_WINDOW_VIS_3:
		case IDM_WINDOW_VIS_4:
		case IDM_WINDOW_VIS_5:
		case IDM_WINDOW_VIS_6:
		case IDM_WINDOW_VIS_7:
		{
			i = wCmd - IDM_WINDOW_VIS_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			if (!td->visible) {
				td->visible = true;
				ShowWindow(td->w, SW_SHOW);
				term_data_redraw(td);
			} else {
				td->visible = false;
				ShowWindow(td->w, SW_HIDE);
			}

			break;
		}

		/* Window fonts */
		case IDM_WINDOW_FONT_0:
		case IDM_WINDOW_FONT_1:
		case IDM_WINDOW_FONT_2:
		case IDM_WINDOW_FONT_3:
		case IDM_WINDOW_FONT_4:
		case IDM_WINDOW_FONT_5:
		case IDM_WINDOW_FONT_6:
		case IDM_WINDOW_FONT_7:
		{
			if ((use_graphics_nice) && (!inkey_flag || !initialized)) {
				plog("You may not do that right now.");
				break;
			}
                  
			i = wCmd - IDM_WINDOW_FONT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			term_change_font(td);

			if (use_graphics_nice) {
			        /* Hack -- Force redraw */
			        Term_key_push(KTRL('R'));
			}

			break;
		}

		/* Bizarre Display */
		case IDM_WINDOW_BIZ_0:
		case IDM_WINDOW_BIZ_1:
		case IDM_WINDOW_BIZ_2:
		case IDM_WINDOW_BIZ_3:
		case IDM_WINDOW_BIZ_4:
		case IDM_WINDOW_BIZ_5:
		case IDM_WINDOW_BIZ_6:
		case IDM_WINDOW_BIZ_7:
		{
			i = wCmd - IDM_WINDOW_BIZ_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->bizarre = !td->bizarre;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Increase Tile Width */
		case IDM_WINDOW_I_WID_0:
		case IDM_WINDOW_I_WID_1:
		case IDM_WINDOW_I_WID_2:
		case IDM_WINDOW_I_WID_3:
		case IDM_WINDOW_I_WID_4:
		case IDM_WINDOW_I_WID_5:
		case IDM_WINDOW_I_WID_6:
		case IDM_WINDOW_I_WID_7:
		{
			i = wCmd - IDM_WINDOW_I_WID_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_wid += 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Decrease Tile Height */
		case IDM_WINDOW_D_WID_0:
		case IDM_WINDOW_D_WID_1:
		case IDM_WINDOW_D_WID_2:
		case IDM_WINDOW_D_WID_3:
		case IDM_WINDOW_D_WID_4:
		case IDM_WINDOW_D_WID_5:
		case IDM_WINDOW_D_WID_6:
		case IDM_WINDOW_D_WID_7:
		{
			i = wCmd - IDM_WINDOW_D_WID_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_wid -= 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Increase Tile Height */
		case IDM_WINDOW_I_HGT_0:
		case IDM_WINDOW_I_HGT_1:
		case IDM_WINDOW_I_HGT_2:
		case IDM_WINDOW_I_HGT_3:
		case IDM_WINDOW_I_HGT_4:
		case IDM_WINDOW_I_HGT_5:
		case IDM_WINDOW_I_HGT_6:
		case IDM_WINDOW_I_HGT_7:
		{
			i = wCmd - IDM_WINDOW_I_HGT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_hgt += 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Decrease Tile Height */
		case IDM_WINDOW_D_HGT_0:
		case IDM_WINDOW_D_HGT_1:
		case IDM_WINDOW_D_HGT_2:
		case IDM_WINDOW_D_HGT_3:
		case IDM_WINDOW_D_HGT_4:
		case IDM_WINDOW_D_HGT_5:
		case IDM_WINDOW_D_HGT_6:
		case IDM_WINDOW_D_HGT_7: {
			i = wCmd - IDM_WINDOW_D_HGT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];
			td->tile_hgt -= 1;

			term_getsize(td);
			term_window_resize(td);

			break;
		}

		case IDM_WINDOW_OPT: {
			Term_keypress('=',0);
			Term_keypress('w',0);

			break;
		}
		case IDM_WINDOW_RESET: {
			/* Paranoia */
			if (!inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}
			
			
			if (MessageBox(NULL,
					"This will reset the size and layout of the angband windows\n based on your screen size. Do you want to continue?",
					VERSION_NAME, MB_YESNO|MB_ICONWARNING) == IDYES) {
				term *old = Term;
				int i;
				RECT rc;

				(void)default_layout_win(data,MAX_TERM_DATA);

				for (i = 0; i < MAX_TERM_DATA; i++) {
					/* Activate */
					Term_activate(&(data[i].t));
	        
					/* Resize the term */
					Term_resize(data[i].cols, data[i].rows);
				}
				/* Restore */
				Term_activate(old);

				/* Do something to sub-windows */
				for (i = MAX_TERM_DATA - 1; i >= 0; i--) {
					if (!(data[i].w)) continue;
					
					/* Client window size */
					rc.left = 0;
					rc.top = 0;
					rc.right = rc.left + data[i].cols * data[i].tile_wid +
						data[i].size_ow1 + data[i].size_ow2;
					rc.bottom = rc.top + data[i].rows * data[i].tile_hgt +
						data[i].size_oh1 + data[i].size_oh2;

					/* Get total window size (without menu for sub-windows) */
					AdjustWindowRectEx(&rc, data[i].dwStyle, true,
									   data[i].dwExStyle);

					/* Total size */
					data[i].size_wid = rc.right - rc.left;
					data[i].size_hgt = rc.bottom - rc.top;

					if (i == 0) {
						SetWindowPos(data[i].w, 0, data[i].pos_x, data[i].pos_y,
									 data[i].size_wid, data[i].size_hgt, 0);
					} else {
						SetWindowPos(data[i].w, data[0].w, data[i].pos_x,
									 data[i].pos_y, data[i].size_wid,
									 data[i].size_hgt, 0);
					}
					if (data[i].visible) {
						ShowWindow(data[i].w, SW_SHOW);
					} else {
						ShowWindow(data[i].w, SW_HIDE);
					}

					/* Redraw later */
					InvalidateRect(data[i].w, NULL, true);
				}

				/* Focus on main window */
				SetFocus(data[0].w);

				/* React to changes */
				Term_xtra_win_react();

				/* Hack -- Force redraw */
				Term_key_push(KTRL('R'));			
			}

			break;
		}

		case IDM_OPTIONS_GRAPHICS_NICE: {
			/* Paranoia */
			if (!inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}

			/* Toggle "arg_graphics_nice" */
			arg_graphics_nice = !arg_graphics_nice;

			/* React to changes */
			Term_xtra_win_react();

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));
			
			break;
		}

		case IDM_OPTIONS_TILE_1x1:
		case IDM_OPTIONS_TILE_2x1:
		case IDM_OPTIONS_TILE_2x2:
		case IDM_OPTIONS_TILE_3x1:
		case IDM_OPTIONS_TILE_3x3:
		case IDM_OPTIONS_TILE_4x2:
		case IDM_OPTIONS_TILE_4x4:
		case IDM_OPTIONS_TILE_6x3:
		case IDM_OPTIONS_TILE_6x6:
		case IDM_OPTIONS_TILE_8x4:
		case IDM_OPTIONS_TILE_8x8:
		case IDM_OPTIONS_TILE_16x8:
		case IDM_OPTIONS_TILE_16x16:
		{
			/* Paranoia */
			if (!inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}
			switch (wCmd)
			{
				case IDM_OPTIONS_TILE_1x1:
				{
					tile_width = 1;
					tile_height = 1;
					break;
				}
				case IDM_OPTIONS_TILE_2x1:
				{
					tile_width = 2;
					tile_height = 1;
					break;
				}
				case IDM_OPTIONS_TILE_2x2:
				{
					tile_width = 2;
					tile_height = 2;
					break;
				}
				case IDM_OPTIONS_TILE_3x1:
				{
					tile_width = 3;
					tile_height = 1;
					break;
				}
				case IDM_OPTIONS_TILE_3x3:
				{
					tile_width = 3;
					tile_height = 3;
					break;
				}
				case IDM_OPTIONS_TILE_4x2:
				{
					tile_width = 4;
					tile_height = 2;
					break;
				}
				case IDM_OPTIONS_TILE_4x4:
				{
					tile_width = 4;
					tile_height = 4;
					break;
				}
				case IDM_OPTIONS_TILE_6x3:
				{
					tile_width = 6;
					tile_height = 3;
					break;
				}
				case IDM_OPTIONS_TILE_6x6:
				{
					tile_width = 6;
					tile_height = 6;
					break;
				}
				case IDM_OPTIONS_TILE_8x4:
				{
					tile_width = 8;
					tile_height = 4;
					break;
				}
				case IDM_OPTIONS_TILE_8x8:
				{
					tile_width = 8;
					tile_height = 8;
					break;
				}
				case IDM_OPTIONS_TILE_16x8:
				{
					tile_width = 16;
					tile_height = 8;
					break;
				}
				case IDM_OPTIONS_TILE_16x16:
				{
					tile_width = 16;
					tile_height = 16;
					break;
				}
			}

			/* Set flag */
			change_tilesize = true;

			/* React to changes */
			Term_xtra_win_react();

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}

		case IDM_TILE_FONT:
		case IDM_TILE_08X08:
		case IDM_TILE_16X16:
		case IDM_TILE_32X32:
		case IDM_TILE_08X16:
		case IDM_TILE_10X20:
		case IDM_TILE_16X32:
		case IDM_TILE_08X13:
		case IDM_TILE_10X17:
		case IDM_TILE_16X25:
		case IDM_TILE_12X20:
		{
			/* Paranoia */
			if (!inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}
			td = &data[0];
			switch (wCmd)
			{
				case IDM_TILE_FONT:
				{
					td->tile_wid = td->font_wid;
					td->tile_hgt = td->font_hgt;
					break;
				}
				case IDM_TILE_08X16:
				{
					td->tile_wid = 8;
					td->tile_hgt = 16;
					break;
				}
				case IDM_TILE_08X08:
				{
					td->tile_wid = 8;
					td->tile_hgt = 8;
					break;
				}
				case IDM_TILE_16X16:
				{
					td->tile_wid = 16;
					td->tile_hgt = 16;
					break;
				}
				case IDM_TILE_32X32:
				{
					td->tile_wid = 32;
					td->tile_hgt = 32;
					break;
				}
				case IDM_TILE_10X20:
				{
					td->tile_wid = 10;
					td->tile_hgt = 20;
					break;
				}
				case IDM_TILE_16X32:
				{
					td->tile_wid = 16;
					td->tile_hgt = 32;
					break;
				}
				case IDM_TILE_12X20:
				{
					td->tile_wid = 12;
					td->tile_hgt = 20;
					break;
				}
				case IDM_TILE_16X25:
				{
					td->tile_wid = 16;
					td->tile_hgt = 25;
					break;
				}
			}

			/* React to changes */
			term_getsize(td);

			term_window_resize(td);

			break;
		}

#ifdef USE_SAVER

		case IDM_OPTIONS_SAVER:
		{
			if (hwndSaver) {
				DestroyWindow(hwndSaver);
				hwndSaver = NULL;
				screensaver_active = false;

				/* Switch main menu back on */
				SetMenu(data[0].w, main_menu);

				for (i = MAX_TERM_DATA - 1; i >= 0; --i) {
					td = &data[i];

					if (td->visible) {
						/* Turn the Windows back to normal */
						SetWindowLong(td->w, GWL_STYLE, td->dwStyle);

						/* Push the window to the top */
						SetWindowPos(td->w, HWND_NOTOPMOST, 0, 0, 0, 0,
							   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
					}
				}

				ShowCursor(true);
			} else {
				/* Create a screen saver window */
				hwndSaver = CreateWindowEx(WS_EX_TOPMOST,
										   "WindowsScreenSaverClass",
				                           "Angband Screensaver",
				                           WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
				                           0, 0, GetSystemMetrics(SM_CXSCREEN),
				                           GetSystemMetrics(SM_CYSCREEN),
				                           NULL, NULL, hInstance, NULL);

				if (hwndSaver) {
					for (i = MAX_TERM_DATA - 1; i >= 0; --i) {
						td = &data[i];

						if (td->visible) {
							/* Switch off border and titlebar */
							SetWindowLong(td->w, GWL_STYLE, WS_VISIBLE);

							/* Switch off menu */
							SetMenu(td->w, NULL);

							/* Push the window to the top */
							SetWindowPos(td->w, HWND_TOPMOST, 0, 0, 0, 0,
								   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
						}
					}

					ShowCursor(false);

					screensaver_active = true;
				} else {
					plog("Failed to create saver window");
				}
			}

			break;
		}

#endif /* USE_SAVER */

		case IDM_OPTIONS_LOW_PRIORITY:
		{
			/* Lower or reset the priority of the current process */
			if (low_priority)
				SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
			else
				SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

			/* Toggle priority */
			low_priority = !low_priority;

			break;
		}

		case IDM_OPTIONS_MAP:
		{
			/* Paranoia */
			if (!inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}

			windows_map();
			break;
		}

		case IDM_HELP_GENERAL:
		{
			display_help();
			break;
		}

		case IDM_OPTIONS_SCREENSHOT:{
			char filename[1024];
			char path[1024];
			time_t ltime;
			struct tm *today;
			int len;
			bool SaveWindow_PNG(HWND hWnd, LPSTR lpFileName);

			time( &ltime );
			today = localtime( &ltime );
			strnfmt(filename, sizeof(filename), "%s", player->full_name);
			len = strlen(filename);
			strftime(filename+len, sizeof(filename)-len, "_%Y%b%d_%H%M%S.png",
					 today);

			/* Get the system-specific path */
			path_build(path, sizeof(path), ANGBAND_DIR_USER, filename);
			td = &data[0];
			if (!SaveWindow_PNG(td->w, path)) {
				plog("Screenshot Save Failed.");
			}
			break;
		}

		default: {
			if ((wCmd >= IDM_OPTIONS_GRAPHICS_NONE) &&
				(wCmd <= IDM_OPTIONS_GRAPHICS_NONE + graphics_mode_high_id)) {
				int selected_mode = 0;
				int desired_mode = wCmd - IDM_OPTIONS_GRAPHICS_NONE;

				/* Paranoia */
				if (!inkey_flag || !initialized) {
					plog("You may not do that right now.");
					break;
				}

				i = 0;
				do {
					if (graphics_modes[i].grafID == desired_mode) {
						selected_mode = desired_mode;
						break;
					}
				} while (graphics_modes[i++].grafID != 0); 

				/* Toggle "arg_graphics" */
				if (arg_graphics != selected_mode) {
					arg_graphics = selected_mode;

					/* hard code values when switching to text mode */
					if ((selected_mode == GRAPHICS_NONE)
							&& !use_graphics_nice) {
						td = &data[0];
						td->tile_wid = td->font_wid;
						td->tile_hgt = td->font_hgt;
						tile_width = 1;
						tile_height = 1;

						/* React to changes */
						term_getsize(td);

						term_window_resize(td);
					}


					/* React to changes */
					Term_xtra_win_react();

					/* Hack -- Force redraw */
					Term_key_push(KTRL('R'));
				}
			}
			break;
		}
	}
}


/**
 * Redraw a section of a window
 */
static void handle_wm_paint(HWND hWnd)
{
	int x1, y1, x2, y2;
	PAINTSTRUCT ps;
	term_data *td;

	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	BeginPaint(hWnd, &ps);

	if (td->map_active) {
		/* Redraw the map */
		/* ToDo: Only redraw the necessary parts */
		windows_map_aux();
	} else {
		/* Get the area that should be updated (rounding up/down) */
		/* ToDo: Take the window borders into account */
		x1 = (ps.rcPaint.left / td->tile_wid) - 1;
		x2 = (ps.rcPaint.right / td->tile_wid) + 1;
		y1 = (ps.rcPaint.top / td->tile_hgt) - 1;
		y2 = (ps.rcPaint.bottom / td->tile_hgt) + 1;

		/* Redraw */
		if (td) term_data_redraw_section(td, x1, y1, x2, y2);
	}

	EndPaint(hWnd, &ps);
}


int extract_modifiers(keycode_t ch, bool kp) {
	bool mc = false;
	bool ms = false;
	bool ma = false;

	/* Extract the modifiers */
	if (GetKeyState(VK_CONTROL) & 0x8000) mc = true;
	if (GetKeyState(VK_SHIFT)   & 0x8000) ms = true;
	if (GetKeyState(VK_MENU)    & 0x8000) ma = true;

	return
		(mc && (kp || MODS_INCLUDE_CONTROL(ch)) ? KC_MOD_CONTROL : 0) |
		(ms && (kp || MODS_INCLUDE_SHIFT(ch)) ? KC_MOD_SHIFT : 0) |
		(ma ? KC_MOD_ALT : 0) | (kp ? KC_MOD_KEYPAD : 0);
}

/**
 * We ignore the modifier keys (shift, control, alt, num lock, scroll lock),
 * and the normal keys (escape, tab, return, letters, numbers, etc), but we
 * catch the keypad keys (with and without numlock set, including keypad 5),
 * the function keys (including the "menu" key which maps to F10), and the
 * "pause" key (between scroll lock and numlock).  We also catch a few odd
 * keys which I do not recognize, but which are listed among keys which we
 * do catch, so they should be harmless to catch.
 *
 * return whether the keypress was NOT handled
 */
static bool handle_keydown(WPARAM wParam, LPARAM lParam)
{
	keycode_t ch = 0;

	bool kp = false;

#ifdef USE_SAVER
	if (screensaver_active) {
		stop_screensaver();
		return true;
	}
#endif /* USE_SAVER */

	/* for VK_ http://msdn.microsoft.com/en-us/library/dd375731(v=vs.85).aspx */
	switch (wParam) {
		case VK_F1: ch = KC_F1; break;
		case VK_F2: ch = KC_F2; break;
		case VK_F3: ch = KC_F3; break;
		case VK_F4: ch = KC_F4; break;
		case VK_F5: ch = KC_F5; break;
		case VK_F6: ch = KC_F6; break;
		case VK_F7: ch = KC_F7; break;
		case VK_F8: ch = KC_F8; break;
		case VK_F9: ch = KC_F9; break;
		case VK_F10: ch = KC_F10; break;
		case VK_F11: ch = KC_F11; break;
		case VK_F12: ch = KC_F12; break;
		case VK_F13: ch = KC_F13; break;
		case VK_F14: ch = KC_F14; break;
		case VK_F15: ch = KC_F15; break;

		case VK_INSERT: ch = KC_INSERT; break;
		case VK_DELETE: ch = KC_DELETE; break;
		/* Backspace is calling both backspace and delete
		   Removed the backspace call, so it only calls delete */
		case VK_BACK: break;
		/* Tab is registering as ^i; don't read it here*/
	        case VK_TAB: break;
		case VK_PRIOR: ch = KC_PGUP; break;
		case VK_NEXT: ch = KC_PGDOWN; break;
		case VK_END: ch = KC_END; break;
		case VK_HOME: ch = KC_HOME; break;
		case VK_LEFT: ch = ARROW_LEFT; break;
		case VK_RIGHT: ch = ARROW_RIGHT; break;
		case VK_UP: ch = ARROW_UP; break;
		case VK_DOWN: ch = ARROW_DOWN; break;

		case VK_CLEAR: ch = '5'; kp=true; break;
		case VK_PAUSE: ch = KC_PAUSE; break;
	}

	/* we could fall back on using the scancode */
	/* obtained using LOBYTE(HIWORD(lParam)) */
	/* see http://source.winehq.org/source/include/dinput.h#L468 */

	if (ch) {
		int mods = extract_modifiers(ch, kp);
		/* printf("ch=%d mods=%d\n", ch, mods); */
		/* fflush(stdout); */
		Term_keypress(ch, mods);
		return false;
	}
	return true;
}



static LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	term_data *td;
	int i;

	int xPos, yPos, button, vsc, vk, mods;
	bool kp = false, extended_key;
	keycode_t ch;

#ifdef USE_SAVER
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;
#endif /* USE_SAVER */

	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	/* Handle message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (size_t) my_td);
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR *lpmmi;
			RECT rc;

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* Minimum window size is 80x24 */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->tile_wid + td->size_ow1 +
				td->size_ow2;
			rc.bottom = rc.top + 24 * td->tile_hgt + td->size_oh1 +
				td->size_oh2 + 1;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

		case WM_PAINT:
		{
			handle_wm_paint(hWnd);

			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			return handle_keydown(wParam, lParam);
		}

		case WM_CHAR:
		{
			vsc = LOBYTE(HIWORD(lParam));
			extended_key = HIBYTE(HIWORD(lParam)) & 0x1;
			vk = MapVirtualKey(vsc, 1);
			/* printf("wParam=%d lParam=%d vsc=%d vk=%d kp=%d\n", */
			/*        wParam, lParam, vsc, vk, extended_key); */
			/* fflush(stdout); */

			if (!game_in_progress) {
				/* Handle keyboard shortcuts pre-game */
				switch (wParam) {
					case KTRL('N'): process_menus(IDM_FILE_NEW); break;
					case KTRL('O'): process_menus(IDM_FILE_OPEN); break;
					case KTRL('X'): process_menus(IDM_FILE_EXIT); break;
					default: return true;
				}
				return false;
			}

			// We don't want to translate some keys to their ascii values
			// so we have to intercept them here.
			switch (vk)
			{
				case 8: // fix backspace
					ch = KC_BACKSPACE;
					break;
				case 9: // fix tab
					ch = KC_TAB;
					break;
				case 13: // fix enter
					ch = KC_ENTER;
					if(extended_key) kp = true;
					break;
				case 27: // fix escape
					ch = ESCAPE;
					break;
				default:
					Term_keypress(wParam, 0);
					return 0;
			}

			mods = extract_modifiers(ch, kp);
			Term_keypress(ch, mods);

			return 0;
		}

		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		{
			if (screensaver_active) {
#ifdef USE_SAVER
				stop_screensaver();
#else
				screensaver_active = false;
#endif /* USE_SAVER */
				return 0;
			} else {
				/* Get the text grid */
				xPos = GET_X_LPARAM(lParam);
				yPos = GET_Y_LPARAM(lParam);
				xPos /= td->tile_wid;
				yPos /= td->tile_hgt;

				if (uMsg == WM_LBUTTONDOWN)
					button = 1;
				else if (uMsg == WM_RBUTTONDOWN)
					button = 2;
				else
					button = 3;

				/* Extract the modifiers */
				/* XXX using the numbers below rather than KC_MOD_CONTROL, KCMOD_SHIFT,
				 * and KC_MOD_ALT, to avoid having to shift them all the time. They
				 * need to be shifted because I don't want to change the function
				 * parameters (which would break the other platforms, which I can't
				 * test), so the mods need to be encoded into the button.
				 */
				if (GetKeyState(VK_CONTROL) & 0x8000) button |= 16;
				if (GetKeyState(VK_SHIFT)   & 0x8000) button |= 32;
				if (GetKeyState(VK_MENU)    & 0x8000) button |= 64;

				Term_mousepress(xPos,yPos,button);
			}
			break;
		}

#ifdef USE_SAVER
		case WM_MOUSEMOVE:
		{
			if (!screensaver_active) break;

			if (iMouse) {
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
					stop_screensaver();
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}
#endif /* USE_SAVER */

		case WM_INITMENU:
		{
			setup_menus();
			return 0;
		}

#ifndef WM_QUERYENDSESSION
#define WM_QUERYENDSESSION 0x0011
#endif

		case WM_QUERYENDSESSION:
		case WM_QUIT: {
			if (game_in_progress && character_generated) {
				if (uMsg == WM_QUERYENDSESSION && !inkey_flag) {
					plog("Please exit any open menus before closing the game.");
					return false;
				}

				msg_flag = false;
				save_game();
				close_game(true);
			}

			quit(NULL);
			return true;
		}


		case WM_CLOSE:
		{
			if (game_in_progress && character_generated) {
				if (!inkey_flag) {
					plog("Please exit any open menus before closing the game.");
					return 0;
				}

				/* Hack -- Forget messages */
				msg_flag = false;
				save_game();
				close_game(true);
			}

			quit(NULL);
			return 0;
		}

		case WM_COMMAND:
		{
			process_menus(LOWORD(wParam));
			return 0;
		}

		case WM_SIZE:
		{
			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* it was sent from inside CreateWindowEx */
			if (!td->w) return 1;

			/* was sent from WM_SIZE */
			if (td->size_hack) return 1;

			switch (wParam)
			{
				case SIZE_MINIMIZED:
				{
					/* Hide sub-windows */
					for (i = 1; i < MAX_TERM_DATA; i++)
						if (data[i].visible)
							ShowWindow(data[i].w, SW_HIDE);

					return 0;
				}

				case SIZE_MAXIMIZED:
				{
					/* fall through XXX XXX XXX */
				}

				case SIZE_RESTORED:
				{
					int cols = (LOWORD(lParam) - td->size_ow1) / td->tile_wid;
					int rows = (HIWORD(lParam) - td->size_oh1) / td->tile_hgt;

					/* New size */
					if ((td->cols != cols) || (td->rows != rows)) {
						/* Save the new size */
						td->cols = cols;
						td->rows = rows;

						/* Activate */
						Term_activate(&td->t);

						/* Resize the term */
						Term_resize(td->cols, td->rows);

						/* Redraw later */
						InvalidateRect(td->w, NULL, true);
					}

					td->size_hack = true;

					/* Show sub-windows */
					for (i = 1; i < MAX_TERM_DATA; i++)
						if (data[i].visible)
							ShowWindow(data[i].w, SW_SHOW);

					td->size_hack = false;

					return 0;
				}
			}
			break;
		}

		case WM_PALETTECHANGED:
		{
			/* Ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return 0;

			/* Fall through... */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;

			hdc = GetDC(hWnd);

			SelectPalette(hdc, hPal, false);

			i = RealizePalette(hdc);

			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, true);

			ReleaseDC(hWnd, hdc);

			return 0;
		}

		case WM_ACTIVATE:
		{
			if (wParam && !HIWORD(lParam)) {
				/* Do something to sub-windows */
				for (i = MAX_TERM_DATA - 1; i >= 0; i--)
					SetWindowPos(data[i].w, hWnd, 0, 0, 0, 0,
					             SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

				/* Focus on main window */
				SetFocus(hWnd);

				return 0;
			}

			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam)
{
	term_data *td;
	HDC hdc;
	int i;

#ifdef USE_SAVER
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;
#endif /* USE_SAVER */


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	/* Process message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (size_t)(my_td));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
#if 0
			MINMAXINFO FAR *lpmmi;
			RECT rc;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* Minimum size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->tile_hgt + td->size_oh1 + td->size_oh2;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save the minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->tile_wid + td->size_ow1 +
				td->size_ow2;
			rc.bottom = rc.top + 24 * td->tile_hgt + td->size_oh1 +
				td->size_oh2;

			/* Paranoia */
			rc.right += (td->tile_wid - 1);
			rc.bottom += (td->tile_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x = rc.right - rc.left;
			lpmmi->ptMaxSize.y = rc.bottom - rc.top;

			/* Save the maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;
#endif /* 0 */
			return 0;
		}

		case WM_SIZE:
		{
			int cols;
			int rows;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* it was sent from inside CreateWindowEx */
			if (!td->w) return 1;

			/* was sent from inside WM_SIZE */
			if (td->size_hack) return 1;

			td->size_hack = true;

			cols = (LOWORD(lParam) - td->size_ow1) / td->tile_wid;
			rows = (HIWORD(lParam) - td->size_oh1) / td->tile_hgt;

			/* New size */
			if ((td->cols != cols) || (td->rows != rows)) {
				/* Save old term */
				term *old_term = Term;

				/* Save the new size */
				td->cols = cols;
				td->rows = rows;

				/* Activate */
				Term_activate(&td->t);

				/* Resize the term */
				Term_resize(td->cols, td->rows);

				/* Activate */
				Term_activate(old_term);

				/* Redraw later */
				InvalidateRect(td->w, NULL, true);

				/* HACK - Redraw all windows */
				if (character_dungeon) do_cmd_redraw();
			}

			td->size_hack = false;

			return 0;
		}

		case WM_PAINT:
		{
			handle_wm_paint(hWnd);

			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			return handle_keydown(wParam, lParam);
			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam, 0);
			return 0;
		}


#ifdef USE_SAVER
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		{
			if (screensaver_active) {
				stop_screensaver();
				return 0;
			}
			break;
		}

		case WM_MOUSEMOVE:
		{
			if (!screensaver_active) break;

			if (iMouse) {
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
					stop_screensaver();
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}
#endif /* USE_SAVER */

		case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return false;
			/* otherwise, fall through!!! */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, false);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, true);
			ReleaseDC(hWnd, hdc);
			return 0;
		}

		case WM_NCLBUTTONDOWN:
		{

#ifdef HTCLOSE
			if (wParam == HTCLOSE) wParam = HTSYSMENU;
#endif /* HTCLOSE */

			if (wParam == HTSYSMENU) {
				if (td->visible) {
					td->visible = false;
					ShowWindow(td->w, SW_HIDE);
				}

				return 0;
			}

			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef USE_SAVER

LRESULT FAR PASCAL AngbandSaverProc(HWND hWnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;


	/* Process */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			break;
		}

		case WM_SETCURSOR:
		{
			SetCursor(NULL);
			return 0;
		}

#if 0
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE) break;

			/* else fall through */
		}
#endif /* 0 */

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_KEYDOWN:
		{
			stop_screensaver();
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			if (iMouse) {
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
					stop_screensaver();
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hwndSaver);
			if (screensaver)
				SendMessage(data[0].w, WM_CLOSE, 0, 0);
			hwndSaver = NULL;
			return 0;
		}
	}

	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif /* USE_SAVER */





/**
 * ------------------------------------------------------------------------
 *  Temporary Hooks
 * ------------------------------------------------------------------------ */



/**
 * Display warning message (see "z-util.c")
 */
static void hack_plog(const char *str)
{
	/* Give a warning */
	if (str)
		MessageBox(NULL, str, "Warning", MB_ICONEXCLAMATION | MB_OK);
}


/**
 * Display error message and quit (see "z-util.c")
 */
static void hack_quit(const char *str)
{
	/* Give a warning */
	if (str)
		MessageBox(NULL, str, "Error",
				   MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);

	/* Unregister the classes */
	UnregisterClass(AppName, hInstance);

	/* Destroy the icon */
	if (hIcon) DestroyIcon(hIcon);

	/* Clean up guarding access to the savefile. */
	finish_monitoring_savefile(EVENT_LEAVE_GAME, NULL, NULL);

#ifdef USE_SAVER
	if (screensaverSemaphore)
		CloseHandle(screensaverSemaphore);
#endif /* USE_SAVER */

	/* Exit */
	exit(0);
}



/**
 * ------------------------------------------------------------------------
 *  Various hooks
 * ------------------------------------------------------------------------ */



/**
 * Display warning message (see "z-util.c")
 */
static void hook_plog(const char *str)
{
#ifdef USE_SAVER
	if (screensaver_active) return;
#endif /* USE_SAVER */

	/* Warning */
	if (str)
		MessageBox(data[0].w, str, "Warning", MB_ICONEXCLAMATION | MB_OK);
}


/**
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(const char *str)
{
	int i;


#ifdef USE_SAVER
	if (!screensaver_active)
#endif /* USE_SAVER */
	{
		/* Give a warning */
		if (str)
			MessageBox(data[0].w, str, "Error",
			           MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);

		/* Save the preferences */
		save_prefs();
	}

	/*** Could use 'Term_nuke_win()' XXX XXX XXX */

	/* Destroy all windows */
	for (i = MAX_TERM_DATA - 1; i >= 0; --i) {
		/* Remove all fonts from the system, free resources */
		if (data[i].font_file) term_remove_font(data[i].font_file);
		if (data[i].font_id) DeleteObject(data[i].font_id);
		if (data[i].font_want) string_free(data[i].font_want);

		/* Kill the window */
		if (data[i].w) DestroyWindow(data[i].w);
		data[i].w = 0;
		term_nuke(&data[i].t);
	}

	/* Free the bitmap stuff */
	FreeDIB(&infGraph);
	FreeDIB(&infMask);

	close_graphics_modes();

	/*** Free some other stuff ***/

	DeleteObject(hbrYellow);

	if (hPal) DeleteObject(hPal);

	UnregisterClass(AppName, hInstance);

	if (hIcon) DestroyIcon(hIcon);

	/* Free strings */
	string_free(ini_file);
	string_free(argv0);

#ifdef HAS_CLEANUP
	textui_cleanup();
	cleanup_angband();
	close_sound();
#endif /* HAS_CLEANUP */

	exit(0);
}


/**
 * ------------------------------------------------------------------------
 *  Initialize
 * ------------------------------------------------------------------------ */

/**
 * Init some stuff
 */
static void init_stuff(void)
{
	int i;

	char path[1024];
#ifdef USE_SAVER
	char tmp[1024];
#endif /* USE_SAVER */

	/* Get program name with full path */
	if (GetModuleFileName(hInstance, path, sizeof(path)) == 0)
		show_win_error();

	/* Paranoia */
	path[sizeof(path) - 1] = '\0';

	/* Save the "program name" */
	argv0 = string_make(path);

	/* Get the name of the "*.ini" file */
	strcpy(path + strlen(path) - 4, ".INI");

#ifdef USE_SAVER

	/* Try to get the path to the Angband folder */
	if (screensaver) {
		/* Extract the filename of the savefile for the screensaver */
		GetPrivateProfileString("Angband", "SaverFile", "", saverfilename,
								sizeof(saverfilename), path);

		GetPrivateProfileString("Angband", "AngbandPath", "", tmp,
								sizeof(tmp), path);

		sprintf(path, "%sangband.ini", tmp);
	}

#endif /* USE_SAVER */

	/* Save the the name of the ini-file */
	ini_file = string_make(path);

	/* Analyze the path */
	i = strlen(path);

	/* Get the path */
	for (; i > 0; i--) {
		if (path[i] == '\\') {
			/* End of path */
			break;
		}
	}

	/* Add "lib" to the path */
	strcpy(path + i + 1, "lib\\");

	/* Validate the path */
	validate_dir(path);

	/* Init the file paths */
	init_file_paths(path, path, path);

	/* Hack -- Validate the paths */
	validate_dir(ANGBAND_DIR_GAMEDATA);
	validate_dir(ANGBAND_DIR_CUSTOMIZE);
	validate_dir(ANGBAND_DIR_HELP);
	validate_dir(ANGBAND_DIR_SCREENS);
	validate_dir(ANGBAND_DIR_FONTS);
	validate_dir(ANGBAND_DIR_TILES);
	validate_dir(ANGBAND_DIR_SOUNDS);
	validate_dir(ANGBAND_DIR_ICONS);
	validate_dir(ANGBAND_DIR_USER);
	validate_dir(ANGBAND_DIR_SAVE);
	validate_dir(ANGBAND_DIR_PANIC);
	validate_dir(ANGBAND_DIR_SCORES);
	validate_dir(ANGBAND_DIR_INFO);

	/* Build the filename */
	path_build(path, sizeof(path), ANGBAND_DIR_SCREENS, "news.txt");

	/* Hack -- Validate the "news.txt" file */
	validate_file(path);

	/* Build the filename */
	path_build(path, sizeof(path), ANGBAND_DIR_FONTS, DEFAULT_FONT);

	/* Hack -- Validate the basic font */
	validate_file(path);

	/* Validate the "graf" directory */
	validate_dir(ANGBAND_DIR_TILES);

	/* Validate the "sound" directory */
	validate_dir(ANGBAND_DIR_SOUNDS);
}


/**
 * Perform (as ui-game.c's reinit_hook) platform-specific actions necessary
 * when restarting without exiting.  Also called directly at startup.
 */
static void win_reinit(void)
{
	/* Initialise sound. */
	init_sound("win", 0, NULL);

	/*
	 * Watch for these events to set up and tear down protection against
	 * against accessing the savefile from multiple application instances.
	 */
	event_add_handler(EVENT_LEAVE_INIT, monitor_new_savefile, NULL);
	event_add_handler(EVENT_LEAVE_GAME, finish_monitoring_savefile, NULL);
}


int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPSTR lpCmdLine, int nCmdShow)
{
	int i;

	WNDCLASS wc;
	HDC hdc;
	MSG msg;

	/* Unused parameter */
	(void)nCmdShow;

#ifdef USE_SAVER
	if (lpCmdLine && ((*lpCmdLine == '-') || (*lpCmdLine == '/'))) {
		lpCmdLine++;

		switch (*lpCmdLine)
		{
			case 's':
			case 'S':
			{
				screensaver = true;

				/* Only run one screensaver at the time */
				screensaverSemaphore = CreateSemaphore(NULL, 0, 1,
													   "AngbandSaverSemaphore");

				if (!screensaverSemaphore) exit(0);

				if (GetLastError() == ERROR_ALREADY_EXISTS) {
					CloseHandle(screensaverSemaphore);
					exit(0);
				}

				break;
			}

			case 'P':
			case 'p':
			case 'C':
			case 'c':
			case 'A':
			case 'a':
			{
				/*
				 * ToDo: implement preview, configuration, and changing
				 * the password (as well as checking it).
				 */
				exit(0);
			}
		}
	}

#endif /* USE_SAVER */

	/* Initialize */
	if (hPrevInst == NULL) {
		wc.style         = CS_CLASSDC;
		wc.lpfnWndProc   = AngbandWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 4; /* one long pointer to term_data */
		wc.hInstance     = hInst;
		wc.hIcon         = hIcon = LoadIcon(hInst, "ANGBAND");
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName  = "ANGBAND";
		wc.lpszClassName = AppName;

		if (!RegisterClass(&wc)) exit(1);

		wc.lpfnWndProc   = AngbandListProc;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = AngList;

		if (!RegisterClass(&wc)) exit(2);

#ifdef USE_SAVER

		wc.style          = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
		wc.lpfnWndProc    = AngbandSaverProc;
		wc.hCursor        = NULL;
		wc.lpszMenuName   = NULL;
		wc.lpszClassName  = "WindowsScreenSaverClass";

		if (!RegisterClass(&wc)) exit(3);

#endif /* USE_SAVER */

	}

	setlocale(LC_CTYPE, "");

	/* Save globally */
	hInstance = hInst;

	/* Temporary hooks */
	plog_aux = hack_plog;
	quit_aux = hack_quit;

	/* Prepare the filepaths */
	init_stuff();

	/* Determine if display is 16/256/true color */
	hdc = GetDC(NULL);
	colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
	paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? true : false);
	ReleaseDC(NULL, hdc);

	/* Initialize the colors */
	for (i = 0; i < MAX_COLORS; i++) {
		uint8_t rv, gv, bv;

		/* Extract desired values */
		rv = angband_color_table[i][1];
		gv = angband_color_table[i][2];
		bv = angband_color_table[i][3];

		/* Extract the "complex" code */
		win_clr[i] = PALETTERGB(rv, gv, bv);

		/* Save the "simple" code */
		angband_color_table[i][0] = win_pal[i];
	}

	/* load the possible graphics modes */
	if (!init_graphics_modes()) {
		plog_fmt("Graphics list load failed");
	}

	/* Prepare the windows */
	init_windows();

	/* Activate hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;

	/* Set the system suffix */
	ANGBAND_SYS = "win";

#ifdef USE_SAVER
	if (screensaver) {
		/* Start the screensaver */
		start_screensaver();

		/* Paranoia */
		quit(NULL);
	}
#endif /* USE_SAVER */

	/* Set command hook */
	cmd_get_hook = textui_get_cmd;

	/*
	 * Set action that needs to be done if restarting without exiting.
	 * Also need to do it now.
	 */
	reinit_hook = win_reinit;
	win_reinit();

	/* Set up the display handlers and things. */
	init_display();
	init_angband();

	textui_init();

	initialized = true;

	/* Did the user double click on a save file? */
	check_for_save_file(lpCmdLine);

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]",
		(Term->hgt - 23) / 5 + 23, (Term->wid - 45) / 2);
	Term_fresh();

	/* Process messages forever */
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Paranoia */
	quit(NULL);

	/* Paranoia */
	return (0);
}

/**
 * Use the existence or absence of a file in the user directory to guard against
 * accessing a savefile simultaneously from multiple instances of the
 * application.  Return true if, according to that mechanism, no other instance
 * is accessing the savefile.  Return false if another instance appears to be
 * accessing the savefile.
 */
static bool create_savefile_tracking_file(bool message_on_failure)
{
	const char *tracking_extension = ".lok";
	char *name = mem_alloc(MAX_PATH);
	size_t offset = path_filename_index(savefile);
	bool result = true;

	path_build(name, MAX_PATH, ANGBAND_DIR_USER, savefile + offset);
	my_strcat(name, tracking_extension, MAX_PATH);

	if (!suffix(name, tracking_extension)) {
		/* Ugh, no room for it all.  Skip checking. */
		mem_free(name);
		return result;
	}

	multapp_file = CreateFileA(name, GENERIC_READ | GENERIC_WRITE,
		0, NULL, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (multapp_file == INVALID_HANDLE_VALUE) {
		result = false;
		if (message_on_failure) {
			plog_fmt("Another instance of the game appears to using that savefile.  If that's incorrect, delete %s and retry.", name);
		}
	}

	mem_free(name);
	return result;
}

/**
 * Set up to monitor an existing savefile so it isn't accessed by multiple
 * application instances.
 */
static bool monitor_existing_savefile(void)
{
	assert(multapp_file == INVALID_HANDLE_VALUE);
	return create_savefile_tracking_file(true);
}

/**
 * Respond to EVENT_LEAVE_INIT events by monitoring the savefile, if not
 * already monitored from the request to open it, so it won't be accessed
 * simultaneously by multiple application instances.
 */
static void monitor_new_savefile(game_event_type ev_type,
		game_event_data *ev_data, void *user)
{
	assert(ev_type == EVENT_LEAVE_INIT && ev_data == NULL && user == NULL);
	if (multapp_file == INVALID_HANDLE_VALUE) {
		(void) create_savefile_tracking_file(false);
	}
}

/**
 * Respond to EVENT_LEAVE_WORLD events by ceasing to monitor the savefile.
 */
static void finish_monitoring_savefile(game_event_type ev_type,
		game_event_data *ev_data, void *user)
{
	assert(ev_type == EVENT_LEAVE_GAME && ev_data == NULL && user == NULL);
	if (multapp_file != INVALID_HANDLE_VALUE) {
		(void) CloseHandle(multapp_file);
		multapp_file = INVALID_HANDLE_VALUE;
	}
}

#endif /* WINDOWS */
