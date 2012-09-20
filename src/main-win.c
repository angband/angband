/* File: main-win.c */

/* Purpose: Support for Windows Angband */

/*
 * Written (2.7.8) by Skirmantas Kligys (kligys@scf.usc.edu)
 *
 * Based loosely on "main-mac.c" and "main-xxx.c" and suggestions
 * by Ben Harrison (benh@voicenet.com).
 *
 * Upgrade to Angband 2.7.9v6 by Ben Harrison (benh@voicenet.com),
 * Ross E Becker (beckerr@cis.ohio-state.edu), and Chris R. Martin
 * (crm7479@tam2000.tamu.edu).
 *
 * Note that the "Windows" version requires several extra files, which
 * must be placed in various places.  These files are distributed in a
 * special "ext-win.zip" archive, with instructions on where to place
 * the various files.  For example, we require that all font files,
 * bitmap files, and sound files, be placed into the "lib/xtra/font/",
 * "/lib/xtra/graf/", and "lib/xtra/sound/" directories, respectively.
 *
 * See "h-config.h" for the extraction of the "WINDOWS" flag based
 * on the "_Windows", "__WINDOWS__", "__WIN32__", "WIN32", "__WINNT__",
 * or "__NT__" flags.  If your compiler uses a different compiler flag,
 * add it to "h-config.h", or, simply place it in the "Makefile".
 *
 *
 * This file still needs some work, possible problems are indicated by
 * the string "XXX XXX XXX" in any comment.
 *
 * XXX XXX XXX
 * The "Term_xtra_win_clear()" function should probably do a low-level
 * clear of the current window, and redraw the borders and other things.
 *
 * XXX XXX XXX
 * The "arg_graphics" option should affect ALL of the windows, not just
 * the main "screen" window.  The "FULL_GRAPHICS" option is supposed to
 * enable this behavior, but it will load one bitmap file for each of
 * the windows, which may take a lot of memory, especially since with
 * a little clever code, we could load each bitmap only when needed,
 * and only free it once nobody is using it any more.
 *
 * XXX XXX XXX
 * The user should be able to select the "bitmap" for a window independant
 * from the "font", and should be allowed to select any bitmap file which
 * is strictly smaller than the font file.  Currently, bitmap selection
 * always imitates the font selection unless the "Font" and "Graf" lines
 * are used in the "ANGBAND.INI" file.  This may require the addition
 * of a menu item for each window to select the bitmaps.
 *
 * XXX XXX XXX
 * The various "warning" messages assume the existance of the "screen.w"
 * window, I think, and only a few calls actually check for its existance,
 * this may be okay since "NULL" means "on top of all windows". (?)
 *
 * XXX XXX XXX
 * Special "Windows Help Files" can be placed into "lib/xtra/help/" for
 * use with the "winhelp.exe" program.  These files *may* be available
 * at the ftp site somewhere.
 *
 * XXX XXX XXX
 * The "prepare menus" command should "gray out" any menu command which
 * is not allowed at the current time.  This will simplify the actual
 * processing of menu commands, which can assume the command is legal.
 *
 * XXX XXX XXX
 * Remove the old "FontFile" entries from the "*.ini" file, and remove
 * the entire section about "sounds", after renaming a few sound files.
 * In fact, the whole "sound" concept has changed...
 */


#include "angband.h"


#ifdef WINDOWS


/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif


/*
 * Menu constants -- see "ANGBAND.RC"
 */
 
#define IDM_FILE_NEW			101
#define IDM_FILE_OPEN			102
#define IDM_FILE_SAVE			103
#define IDM_FILE_EXIT			104
#define IDM_FILE_QUIT			105

#define IDM_TEXT_SCREEN			201
#define IDM_TEXT_MIRROR			202
#define IDM_TEXT_RECALL			203
#define IDM_TEXT_CHOICE			204
#define IDM_TEXT_TERM_4			205
#define IDM_TEXT_TERM_5			206
#define IDM_TEXT_TERM_6			207
#define IDM_TEXT_TERM_7			208

#define IDM_WINDOWS_SCREEN		211
#define IDM_WINDOWS_MIRROR		212
#define IDM_WINDOWS_RECALL		213
#define IDM_WINDOWS_CHOICE		214
#define IDM_WINDOWS_TERM_4		215
#define IDM_WINDOWS_TERM_5		216
#define IDM_WINDOWS_TERM_6		217
#define IDM_WINDOWS_TERM_7		218

#define IDM_OPTIONS_GRAPHICS	221
#define IDM_OPTIONS_SOUND		222
#define IDM_OPTIONS_UNUSED		231
#define IDM_OPTIONS_SAVER		232

#define IDM_HELP_GENERAL		901
#define IDM_HELP_SPOILERS		902


/*
 * This may need to be removed for some compilers XXX XXX XXX
 */
#define STRICT

/*
 * exclude parts of WINDOWS.H that are not needed
 */
#define NOSOUND           /* Sound APIs and definitions */
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
#define NOCOLOR           /* COLOR_* color values */
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
#define NOCTLMGR          /* Control management and controls */
#define NOHELP            /* Help support */

/*
 * exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
#define NONLS             /* All NLS defines and routines */
#define NOSERVICE         /* All Service Controller routines, SERVICE_ equates, etc. */
#define NOKANJI           /* Kanji support stuff. */
#define NOMCX             /* Modem Configuration Extensions */

/*
 * Include the "windows" support file
 */
#include <windows.h>

/*
 * exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          /* Installable driver support */
#define MMNOWAVE         /* Waveform support */
#define MMNOMIDI         /* MIDI support */
#define MMNOAUX          /* Auxiliary audio support */
#define MMNOTIMER        /* Timer support */
#define MMNOJOY          /* Joystick support */
#define MMNOMCI          /* MCI support */
#define MMNOMMIO         /* Multimedia file I/O support */
#define MMNOMMSYSTEM     /* General MMSYSTEM functions */

/*
 * Include the ??? files
 */
#include <mmsystem.h>
#include <commdlg.h>

/*
 * Include the support for loading bitmaps
 */
#ifdef USE_GRAPHICS
# include "readdib.h"
#endif

/*
 * Hack -- allow use of the Borg as a screen-saver
 */
#ifdef ALLOW_BORG
# define ALLOW_SCRSAVER
#endif

/*
 * Cannot include "dos.h", so we define some things by hand.
 */
#ifdef WIN32
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF
#else /* WIN32 */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
#endif /* WIN32 */

/*
 * Silliness in WIN32 drawing routine
 */
#ifdef WIN32
# define MoveTo(H,X,Y) MoveToEx(H, X, Y, NULL)
#endif /* WIN32 */

/*
 * Silliness for Windows 95
 */
#ifndef WS_EX_TOOLWINDOW
# define WS_EX_TOOLWINDOW 0
#endif

/*
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

/*
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT	0x08

/*
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

/*
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BRIGHT	0x80


/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * Extra "term" data
 *
 * Note the use of "font_want" and "graf_want" for the names of the
 * font/graf files requested by the user, and the use of "font_file"
 * and "graf_file" for the currently active font/graf files.
 *
 * The "font_file" and "graf_file" are capitilized, and are of the
 * form "8X13.FON" and "8X13.BMP", while "font_want" and "graf_want"
 * can be in almost any form as long as it could be construed as
 * attempting to represent the name of a font or bitmap or file.
 */
struct _term_data
{
	term     t;

	cptr     s;

	HWND     w;

#ifdef USE_GRAPHICS
	DIBINIT infGraph;
#endif

	DWORD    dwStyle;
	DWORD    dwExStyle;

	uint     keys;

	uint     rows;
	uint     cols;

	uint     pos_x;
	uint     pos_y;
	uint     size_wid;
	uint     size_hgt;
	uint     client_wid;
	uint     client_hgt;
	uint     size_ow1;
	uint     size_oh1;
	uint     size_ow2;
	uint     size_oh2;

	byte     visible;

	byte     size_hack;

	cptr     font_want;
	cptr     graf_want;

	cptr     font_file;
	cptr     graf_file;

	HFONT    font_id;

	uint     font_wid;
	uint     font_hgt;
};


/*
 * Maximum number of windows XXX XXX XXX
 */
#define MAX_TERM_DATA 8

/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];

/*
 * Mega-Hack -- global "window creation" pointer
 */
static term_data *td_ptr;

/*
 * Various boolean flags
 */
bool game_in_progress  = FALSE;  /* game in progress */
bool initialized       = FALSE;  /* note when "open"/"new" become valid */
bool paletted          = FALSE;  /* screen paletted, i.e. 256 colors */
bool colors16          = FALSE;  /* 16 colors screen, don't use RGB() */

/*
 * Saved instance handle
 */
static HINSTANCE hInstance;

/*
 * Yellow brush for the cursor
 */
static HBRUSH hbrYellow;

/*
 * An icon
 */
static HICON hIcon;

/*
 * A palette
 */
static HPALETTE hPal;

#ifdef ALLOW_SCRSAVER

/*
 * The screen saver
 */
static HWND hwndSaver;

#endif

/*
 * An array of sound file names
 */
static cptr sound_file[SOUND_MAX];

/*
 * Full path to ANGBAND.INI
 */
static cptr ini_file = NULL;

/*
 * Name of application
 */
static cptr AppName  = "ANGBAND";

/*
 * Name of sub-window type
 */
static cptr AngList  = "AngList";

/*
 * Directory names
 */
static cptr ANGBAND_DIR_XTRA_FONT;
static cptr ANGBAND_DIR_XTRA_GRAF;
static cptr ANGBAND_DIR_XTRA_SOUND;
static cptr ANGBAND_DIR_XTRA_HELP;


/*
 * The Angband color set:
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 * Note that on B/W machines, all non-zero colors can be white (on black).
 *
 * Note that all characters are assumed to be drawn on a black background.
 * This may require calling "Term_wipe()" before "Term_text()", etc.
 *
 * XXX XXX XXX See "main-ibm.c" for a method to allow color editing
 *
 * XXX XXX XXX The color codes below were taken from "main-ibm.c".
 */
static COLORREF win_clr[16] =
{
	PALETTERGB(0x00, 0x00, 0x00),  /* 0 0 0  Dark */
	PALETTERGB(0xFF, 0xFF, 0xFF),  /* 4 4 4  White */
	PALETTERGB(0x8D, 0x8D, 0x8D),  /* 2 2 2  Slate */
	PALETTERGB(0xFF, 0x8D, 0x00),  /* 4 2 0  Orange */
	PALETTERGB(0xD7, 0x00, 0x00),  /* 3 0 0  Red (was 2,0,0) */
	PALETTERGB(0x00, 0x8D, 0x44),  /* 0 2 1  Green */
	PALETTERGB(0x00, 0x00, 0xFF),  /* 0 0 4  Blue */
	PALETTERGB(0x8D, 0x44, 0x00),  /* 2 1 0  Umber */
	PALETTERGB(0x44, 0x44, 0x44),  /* 1 1 1  Lt. Dark */
	PALETTERGB(0xD7, 0xD7, 0xD7),  /* 3 3 3  Lt. Slate */
	PALETTERGB(0xFF, 0x00, 0xFF),  /* 4 0 4  Violet (was 2,0,2) */
	PALETTERGB(0xFF, 0xFF, 0x00),  /* 4 4 0  Yellow */
	PALETTERGB(0xFF, 0x00, 0x00),  /* 4 0 0  Lt. Red (was 4,1,3) */
	PALETTERGB(0x00, 0xFF, 0x00),  /* 0 4 0  Lt. Green */
	PALETTERGB(0x00, 0xFF, 0xFF),  /* 0 4 4  Lt. Blue */
	PALETTERGB(0xD7, 0x8D, 0x44)   /* 3 2 1  Lt. Umber */
};


/*
 * Palette indices for 16 colors
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static BYTE win_pal[16] =
{
	VID_BLACK,			/* Dark */
	VID_WHITE,			/* White */
	VID_CYAN,			/* Slate XXX */
	VID_RED | VID_BRIGHT,	/* Orange XXX */
	VID_RED,			/* Red */
	VID_GREEN,			/* Green */
	VID_BLUE,			/* Blue */
	VID_YELLOW,			/* Umber XXX */
	VID_BLACK | VID_BRIGHT,	/* Light Dark */
	VID_CYAN | VID_BRIGHT,	/* Light Slate XXX */
	VID_MAGENTA,		/* Violet XXX */
	VID_YELLOW | VID_BRIGHT,	/* Yellow */
	VID_MAGENTA | VID_BRIGHT,	/* Light Red XXX */
	VID_GREEN | VID_BRIGHT,	/* Light Green */
	VID_BLUE | VID_BRIGHT,	/* Light Blue */
	VID_YELLOW			/* Light Umber XXX */
};



/*
 * Hack -- given a pathname, point at the filename
 */
static cptr extract_file_name(cptr s)
{
	cptr p;

	/* Start at the end */
	p = s + strlen(s) - 1;

	/* Back up to divider */
	while ((p >= s) && (*p != ':') && (*p != '\\')) p--;

	/* Return file name */
	return (p+1);
}



/*
 * Check for existance of a file
 */
static bool check_file(cptr s)
{
	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (FALSE);

	/* Prohibit directory */
	if (attrib & FILE_ATTRIBUTE_DIRECTORY) return (FALSE);

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return (FALSE);

	/* Prohibit something */
	if (attrib & FA_LABEL) return (FALSE);

	/* Prohibit directory */
	if (attrib & FA_DIREC) return (FALSE);

#endif /* WIN32 */

	/* Success */
	return (TRUE);
}


/*
 * Check for existance of a directory
 */
static bool check_dir(cptr s)
{
	int i;

	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

	/* Check length */
	i = strlen(path);

	/* Remove trailing backslash */
	if (i && (path[i-1] == '\\')) path[--i] = '\0';

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (FALSE);

	/* Require directory */
	if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) return (FALSE);

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return (FALSE);

	/* Prohibit something */
	if (attrib & FA_LABEL) return (FALSE);

	/* Require directory */
	if (!(attrib & FA_DIREC)) return (FALSE);

#endif /* WIN32 */

	/* Success */
	return (TRUE);
}


/*
 * Validate a file
 */
static void validate_file(cptr s)
{
	/* Verify or fail */
	if (!check_file(s))
	{
		quit_fmt("Cannot find required file:\n%s", s);
	}
}


/*
 * Validate a directory
 */
static void validate_dir(cptr s)
{
	/* Verify or fail */
	if (!check_dir(s))
	{
		quit_fmt("Cannot find required directory:\n%s", s);
	}
}


/*
 * Get the "size" for a window
 */
static void term_getsize(term_data *td)
{
	RECT        rc;

	/* Paranoia */
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

	/* Paranoia */
	if (td->cols > 80) td->cols = 80;
	if (td->rows > 24) td->rows = 24;

	/* Window sizes */
	td->client_wid = td->cols * td->font_wid + td->size_ow1 + td->size_ow2;
	td->client_hgt = td->rows * td->font_hgt + td->size_oh1 + td->size_oh2;

	/* Fake window size */
	rc.left = rc.top = 0;
	rc.right = rc.left + td->client_wid;
	rc.bottom = rc.top + td->client_hgt;

	/* XXX XXX XXX */
	/* rc.right += 1; */
	/* rc.bottom += 1; */

	/* Adjust */
	AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

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


/*
 * Write the "preference" data for single term
 */
static void save_prefs_aux(term_data *td, cptr sec_name)
{
	char buf[32];

	RECT rc;

	if (!td->w) return;

	/* Visible (Sub-windows) */
	if (td != &data[0])
	{
		strcpy(buf, td->visible ? "1" : "0");
		WritePrivateProfileString(sec_name, "Visible", buf, ini_file);
	}

	/* Desired font */
	if (td->font_file)
	{
		WritePrivateProfileString(sec_name, "Font", td->font_file, ini_file);
	}

	/* Desired graf */
	if (td->graf_file)
	{
		WritePrivateProfileString(sec_name, "Graf", td->graf_file, ini_file);
	}

	/* Current size (x) */
	wsprintf(buf, "%d", td->cols);
	WritePrivateProfileString(sec_name, "Columns", buf, ini_file);

	/* Current size (y) */
	wsprintf(buf, "%d", td->rows);
	WritePrivateProfileString(sec_name, "Rows", buf, ini_file);

	/* Acquire position */
	GetWindowRect(td->w, &rc);

	/* Current position (x) */
	wsprintf(buf, "%d", rc.left);
	WritePrivateProfileString(sec_name, "PositionX", buf, ini_file);

	/* Current position (y) */
	wsprintf(buf, "%d", rc.top);
	WritePrivateProfileString(sec_name, "PositionY", buf, ini_file);
}


/*
 * Write the "preference" data to the .INI file
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)
{
	char       buf[32];

	strcpy(buf, arg_graphics ? "1" : "0");
	WritePrivateProfileString("Angband", "Graphics", buf, ini_file);

	strcpy(buf, arg_sound ? "1" : "0");
	WritePrivateProfileString("Angband", "Sound", buf, ini_file);

	save_prefs_aux(&data[0], "Main window");

	/* XXX XXX XXX XXX */

	save_prefs_aux(&data[1], "Mirror window");

	save_prefs_aux(&data[2], "Recall window");

	save_prefs_aux(&data[3], "Choice window");

	save_prefs_aux(&data[4], "Term-4 window");

	save_prefs_aux(&data[5], "Term-5 window");

	save_prefs_aux(&data[6], "Term-6 window");

	save_prefs_aux(&data[7], "Term-7 window");
}


/*
 * Load preference for a single term
 */
static void load_prefs_aux(term_data *td, cptr sec_name)
{
	char tmp[128];

	/* Visibility (Sub-window) */
	if (td != &data[0])
	{
		/* Extract visibility */
		td->visible = (GetPrivateProfileInt(sec_name, "Visible", td->visible, ini_file) != 0);
	}

	/* Desired font, with default */
	GetPrivateProfileString(sec_name, "Font", "8X13.FON", tmp, 127, ini_file);
	td->font_want = string_make(extract_file_name(tmp));

	/* Desired graf, with default */
	GetPrivateProfileString(sec_name, "Graf", "8X13.BMP", tmp, 127, ini_file);
	td->graf_want = string_make(extract_file_name(tmp));

	/* Window size */
	td->cols = GetPrivateProfileInt(sec_name, "Columns", td->cols, ini_file);
	td->rows = GetPrivateProfileInt(sec_name, "Rows", td->rows, ini_file);

	/* Window position */
	td->pos_x = GetPrivateProfileInt(sec_name, "PositionX", td->pos_x, ini_file);
	td->pos_y = GetPrivateProfileInt(sec_name, "PositionY", td->pos_y, ini_file);
}


/*
 * Hack -- load a "sound" preference by index and name
 */
static void load_prefs_sound(int i)
{
	char aux[128];
	char wav[128];
	char tmp[128];
	char buf[1024];

	/* Capitalize the sound name */
	strcpy(aux, sound_names[i]);
	aux[0] = FORCEUPPER(aux[0]);

	/* Default to standard name plus ".wav" */
	strcpy(wav, sound_names[i]);
	strcat(wav, ".wav");

	/* Look up the sound by its proper name, using default */
	GetPrivateProfileString("Sound", aux, wav, tmp, 127, ini_file);

	/* Access the sound */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_SOUND, extract_file_name(tmp));

	/* Save the sound filename, if it exists */
	if (check_file(buf)) sound_file[i] = string_make(buf);
}


/*
 * Load the preferences from the .INI file
 */
static void load_prefs(void)
{
	int i;

	/* Extract the "arg_graphics" flag */
	arg_graphics = (GetPrivateProfileInt("Angband", "Graphics", 0, ini_file) != 0);

	/* Extract the "arg_sound" flag */
	arg_sound = (GetPrivateProfileInt("Angband", "Sound", 0, ini_file) != 0);

	/* Load window prefs */
	load_prefs_aux(&data[0], "Main window");

	/* XXX XXX XXX XXX */

	load_prefs_aux(&data[1], "Mirror window");

	load_prefs_aux(&data[2], "Recall window");

	load_prefs_aux(&data[3], "Choice window");

	load_prefs_aux(&data[4], "Term-4 window");

	load_prefs_aux(&data[5], "Term-5 window");

	load_prefs_aux(&data[6], "Term-6 window");

	load_prefs_aux(&data[7], "Term-7 window");


	/* Prepare the sounds */
	for (i = 1; i < SOUND_MAX; i++) load_prefs_sound(i);
}


/*
 * Create the new global palette based on the bitmap palette
 * (if any) from the given bitmap xxx, and the standard 16
 * entry palette derived from "win_clr[]" which is used for
 * the basic 16 Angband colors.
 *
 * This function is never called before all windows are ready.
 */
static void new_palette(void)
{
	HPALETTE       hBmPal;
	HPALETTE       hNewPal;
	HDC            hdc;
	int            i, nEntries;
	int            pLogPalSize;
	int            lppeSize;
	LPLOGPALETTE   pLogPal;
	LPPALETTEENTRY lppe;


	/* Cannot handle palettes */
	if (!paletted) return;


	/* No palette */
	hBmPal = NULL;

	/* No bitmap */
	lppeSize = 0;
	lppe = NULL;
	nEntries = 0;

#ifdef USE_GRAPHICS

	/* Check windows */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--)
	{
		if (data[i].graf_file)
		{
			/* Check the bitmap palette */
			hBmPal = data[i].infGraph.hPalette;
		}
	}

	/* Use the bitmap */
	if (hBmPal)
	{
		lppeSize = 256*sizeof(PALETTEENTRY);
		lppe = (LPPALETTEENTRY)ralloc(lppeSize);
		nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
		if (nEntries == 0) quit("Corrupted bitmap palette");
		if (nEntries > 220) quit("Bitmap must have no more than 220 colors");
	}

#endif

	/* Size of palette */
	pLogPalSize = sizeof(LOGPALETTE) + (16+nEntries)*sizeof(PALETTEENTRY);

	/* Allocate palette */
	pLogPal = (LPLOGPALETTE)ralloc(pLogPalSize);

	/* Version */
	pLogPal->palVersion = 0x300;

	/* Make room for bitmap and normal data */
	pLogPal->palNumEntries = nEntries + 16;

	/* Save the bitmap data */
	for (i = 0; i < nEntries; i++)
	{
		pLogPal->palPalEntry[i] = lppe[i];
	}

	/* Save the normal data */
	for (i = 0; i < 16; i++)
	{
		LPPALETTEENTRY p;

		/* Access the entry */
		p = &(pLogPal->palPalEntry[i+nEntries]);

		/* Save the colors */
		p->peRed = GetRValue(win_clr[i]);
		p->peGreen = GetGValue(win_clr[i]);
		p->peBlue = GetBValue(win_clr[i]);

		/* Save the flags */
		p->peFlags = PC_NOCOLLAPSE;
	}

	/* Free something */
	if (lppe) rnfree(lppe, lppeSize);

	/* Create a new palette, or fail */
	hNewPal = CreatePalette(pLogPal);
	if (!hNewPal) quit("Cannot create palette");

	/* Free the palette */
	rnfree(pLogPal, pLogPalSize);


	hdc = GetDC(data[0].w);
	SelectPalette(hdc, hNewPal, 0);
	i = RealizePalette(hdc);
	ReleaseDC(data[0].w, hdc);
	if (i == 0) quit("Cannot realize palette");

	/* Check windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		hdc = GetDC(data[i].w);
		SelectPalette(hdc, hNewPal, 0);
		ReleaseDC(data[i].w, hdc);
	}

	/* Delete old palette */
	if (hPal) DeleteObject(hPal);

	/* Save new palette */
	hPal = hNewPal;
}


/*
 * Resize a window
 */
static void term_window_resize(term_data *td)
{
	RECT  rc;
	POINT pt;

	/* Require window */
	if (!td->w) return;

	/* Resize the window */
	SetWindowPos(td->w, 0, 0, 0,
	             td->size_wid, td->size_hgt,
	             SWP_NOMOVE | SWP_NOZORDER);

	/* Redraw later */
	InvalidateRect(td->w, NULL, TRUE);
}



/*
 * Force the use of a new "font file" for a term_data
 *
 * This function may be called before the "window" is ready
 *
 * This function returns zero only if everything succeeds.
 */
static errr term_force_font(term_data *td, cptr name)
{
	int i;

	int wid, hgt;

	cptr s;

	char base[16];

	char base_font[16];

	char buf[1024];


	/* Forget the old font (if needed) */
	if (td->font_id) DeleteObject(td->font_id);

	/* Forget old font */
	if (td->font_file)
	{
		bool used = FALSE;

		/* Scan windows */
		for (i = 0; i < MAX_TERM_DATA; i++)
		{
			/* Check "screen" */
			if ((td != &data[i]) &&
			    (data[i].font_file) &&
			    (streq(data[i].font_file, td->font_file)))
			{
				used = TRUE;
			}
		}

		/* Remove unused font resources */
		if (!used) RemoveFontResource(td->font_file);

		/* Free the old name */
		string_free(td->font_file);

		/* Forget it */
		td->font_file = NULL;
	}


	/* No name given */
	if (!name) return (1);

	/* Extract the base name (with suffix) */
	s = extract_file_name(name);

	/* Extract font width */
	wid = atoi(s);

	/* Default font height */
	hgt = 0;

	/* Copy, capitalize, remove suffix, extract width */
	for (i = 0; (i < 16 - 1) && s[i] && (s[i] != '.'); i++)
	{
		/* Capitalize */
		base[i] = FORCEUPPER(s[i]);

		/* Extract "hgt" when found */
		if (base[i] == 'X') hgt = atoi(s+i+1);
	}

	/* Terminate */
	base[i] = '\0';


	/* Build base_font */
	strcpy(base_font, base);
	strcat(base_font, ".FON");


	/* Access the font file */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, base_font);

	/* Verify file */
	if (!check_file(buf)) return (1);


	/* Save new font name */
	td->font_file = string_make(base_font);

	/* Load the new font or quit */
	if (!AddFontResource(buf))
	{
		quit_fmt("Font file corrupted:\n%s", buf);
	}

	/* Create the font XXX XXX XXX Note use of "base" */
	td->font_id = CreateFont(hgt, wid, 0, 0, FW_DONTCARE, 0, 0, 0,
	                         ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                         FIXED_PITCH | FF_DONTCARE, base);


	/* Hack -- Unknown size */
	if (!wid || !hgt)
	{
		HDC         hdcDesktop;
		HFONT       hfOld;
		TEXTMETRIC  tm;

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


	/* Analyze the font */
	term_getsize(td);

	/* Resize the window */
	term_window_resize(td);

	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS

/*
 * Force the use of a new "graf file" for a term_data
 *
 * This function is never called before the windows are ready
 *
 * This function returns zero only if everything succeeds.
 */
static errr term_force_graf(term_data *td, cptr name)
{
	int i;

	int wid, hgt;

	cptr s;

	char base[16];

	char base_graf[16];

	char buf[1024];


	/* Forget old stuff */
	if (td->graf_file)
	{
		/* Free the old information */
		if (td->infGraph.hDIB) GlobalFree(td->infGraph.hDIB);
		if (td->infGraph.hPalette) DeleteObject(td->infGraph.hPalette);
		if (td->infGraph.hBitmap) DeleteObject(td->infGraph.hBitmap);

		/* Forget them */
		td->infGraph.hDIB = 0;
		td->infGraph.hPalette = 0;
		td->infGraph.hBitmap = 0;

		/* Forget old graf name */
		string_free(td->graf_file);

		/* Forget it */
		td->graf_file = NULL;
	}


	/* No name */
	if (!name) return (1);

	/* Extract the base name (with suffix) */
	s = extract_file_name(name);

	/* Extract font width */
	wid = atoi(s);

	/* Default font height */
	hgt = 0;

	/* Copy, capitalize, remove suffix, extract width */
	for (i = 0; (i < 16 - 1) && s[i] && (s[i] != '.'); i++)
	{
		/* Capitalize */
		base[i] = FORCEUPPER(s[i]);

		/* Extract "hgt" when found */
		if (base[i] == 'X') hgt = atoi(s+i+1);
	}

	/* Terminate */
	base[i] = '\0';

	/* Require actual sizes */
	if (!wid || !hgt) return (1);

	/* Build base_graf */
	strcpy(base_graf, base);
	strcat(base_graf, ".BMP");


	/* Access the graf file */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_GRAF, base_graf);

	/* Verify file */
	if (!check_file(buf)) return (1);


	/* Save new graf name */
	td->graf_file = string_make(base_graf);

	/* Load the bitmap or quit */
	if (!ReadDIB(td->w, buf, &td->infGraph))
	{
		quit_fmt("Bitmap corrupted:\n%s", buf);
	}

	/* Save the new sizes */
	td->infGraph.CellWidth = wid;
	td->infGraph.CellHeight = hgt;

	/* Activate a palette */
	new_palette();

	/* Success */
	return (0);
}

#endif


/*
 * Allow the user to change the font (and graf) for this window.
 *
 * XXX XXX XXX This is only called for non-graphic windows
 */
static void term_change_font(term_data *td)
{
	OPENFILENAME ofn;

	char tmp[128] = "";

	/* Extract a default if possible */
	if (td->font_file) strcpy(tmp, td->font_file);

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = data[0].w;
	ofn.lpstrFilter = "Font Files (*.fon)\0*.fon\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = ANGBAND_DIR_XTRA_FONT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "fon";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn))
	{
		/* Force the font */
		if (term_force_font(td, tmp))
		{
			/* Oops */
			(void)term_force_font(td, "8X13.FON");
		}
	}
}


#ifdef USE_GRAPHICS

/*
 * Allow the user to change the graf (and font) for a window
 *
 * XXX XXX XXX This is only called for graphic windows, and
 * changes the font and the bitmap for the window, and is
 * only called if "use_graphics" is true.
 */
static void term_change_bitmap(term_data *td)
{
	OPENFILENAME ofn;

	char tmp[128] = "";

	/* Extract a default if possible */
	if (td->graf_file) strcpy(tmp, td->graf_file);

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = data[0].w;
	ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = ANGBAND_DIR_XTRA_GRAF;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "bmp";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn))
	{
		/* XXX XXX XXX */

		/* Force the requested font and bitmap */
		if (term_force_font(td, tmp) ||
		    term_force_graf(td, tmp))
		{
			/* Force the "standard" font */
			(void)term_force_font(td, "8X13.FON");

			/* Force the "standard" bitmap */
			(void)term_force_graf(td, "8X13.BMP");
		}
	}
}

#endif


/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
	/* Activate the term */
	Term_activate(&td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Restore the term */
	Term_activate(term_screen);
}





/*** Function hooks needed by "Term" ***/


#if 0

/*
 * Initialize a new Term
 */
static void Term_init_win(term *t)
{
	/* XXX Unused */
}


/*
 * Nuke an old Term
 */
static void Term_nuke_win(term *t)
{
	/* XXX Unused */
}

#endif


/*
 * Interact with the User
 */
static errr Term_user_win(int n)
{
	/* Success */
	return (0);
}


/*
 * React to global changes
 */
static errr Term_xtra_win_react(void)
{
	int i;


	/* Simple color */
	if (colors16)
	{
		/* Save the default colors */
		for (i = 0; i < 16; i++)
		{
			/* Simply accept the desired colors */
			win_pal[i] = angband_color_table[i][0];
		}
	}

	/* Complex color */
	else
	{
		COLORREF code;

		byte rv, gv, bv;

		bool change = FALSE;

		/* Save the default colors */
		for (i = 0; i < 16; i++)
		{
			/* Extract desired values */
			rv = angband_color_table[i][1];
			gv = angband_color_table[i][2];
			bv = angband_color_table[i][3];

			/* Extract a full color code */
			code = PALETTERGB(rv, gv, bv);

			/* Activate changes */
			if (win_clr[i] != code)
			{
				/* Note the change */
				change = TRUE;

				/* Apply the desired color */
				win_clr[i] = code;
			}
		}

		/* Activate the palette if needed */
		if (change) new_palette();
	}


#ifdef USE_SOUND

	/* Handle "arg_sound" */
	if (use_sound != arg_sound)
	{
		/* Change setting */
		use_sound = arg_sound;
	}

#endif


#ifdef USE_GRAPHICS

	/* Handle "arg_graphics" */
	if (use_graphics != arg_graphics)
	{
		/* Change setting */
		use_graphics = arg_graphics;

		/* Reset visuals */
		reset_visuals();

		/* Activate graphics */
		if (use_graphics)
		{
			/* Hack -- set the player picture */
			r_info[0].x_attr = 0x87;
			r_info[0].x_char = 0x80 | (10 * p_ptr->pclass + p_ptr->prace);

			/* Force the "requested" bitmap XXX XXX XXX */
			if (term_force_graf(&data[0], data[0].graf_want))
			{
				/* XXX XXX XXX Force the "standard" font */
				(void)term_force_font(&data[0], "8X13.FON");

				/* XXX XXX XXX Force the "standard" bitmap */
				(void)term_force_graf(&data[0], "8X13.BMP");
			}

#ifdef FULL_GRAPHICS

			/* Windows */
			for (i = 1; i < MAX_TERM_DATA; i++)
			{
				/* Force the "requested" bitmap XXX XXX XXX */
				if (term_force_graf(&data[i], data[i].graf_want))
				{
					/* XXX XXX XXX Force the "standard" font */
					(void)term_force_font(&data[i], "8X13.FON");

					/* XXX XXX XXX Force the "standard" bitmap */
					(void)term_force_graf(&data[i], "8X13.BMP");
				}
			}

#endif /* FULL_GRAPHICS */

		}
	}

#endif /* USE_GRAPHICS */


	/* Clean up windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term *old = Term;

		term_data *td = &data[i];

		/* Skip non-changes XXX XXX XXX */
		if ((td->cols == td->t.wid) && (td->rows == td->t.hgt)) continue;

		/* Activate */
		Term_activate(&td->t);

		/* Hack -- Resize the term */
		Term_resize(td->cols, td->rows);

		/* Redraw the contents */
		Term_redraw();

		/* Restore */
		Term_activate(old);
	}


	/* Success */
	return (0);
}


/*
 * Process at least one event
 */
static errr Term_xtra_win_event(int v)
{
	MSG msg;

	/* Wait for an event */
	if (v)
	{
		/* Block */
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Check for an event */
	else
	{
		/* Check */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Success */
	return 0;
}


/*
 * Process all pending events
 */
static errr Term_xtra_win_flush(void)
{
	MSG msg;

	/* Process all pending events */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Success */
	return (0);
}


/*
 * Hack -- clear the screen
 *
 * XXX XXX XXX Make this more efficient
 */
static errr Term_xtra_win_clear(void)
{
	term_data *td = (term_data*)(Term->data);

	HDC  hdc;
	RECT rc;

	/* Rectangle to erase */
	rc.left   = td->size_ow1;
	rc.right  = rc.left + td->cols * td->font_wid;
	rc.top    = td->size_oh1;
	rc.bottom = rc.top + td->rows * td->font_hgt;

	/* Erase it */
	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Hack -- make a noise
 */
static errr Term_xtra_win_noise(void)
{
	MessageBeep(MB_ICONASTERISK);
	return (0);
}


/*
 * Hack -- make a sound
 */
static errr Term_xtra_win_sound(int v)
{
	/* Unknown sound */
	if ((v < 0) || (v >= SOUND_MAX)) return (1);

	/* Unknown sound */
	if (!sound_file[v]) return (1);

#ifdef USE_SOUND

#ifdef WIN32

	/* Play the sound, catch errors */
	return (PlaySound(sound_file[v], 0, SND_FILENAME | SND_ASYNC));

#else /* WIN32 */

	/* Play the sound, catch errors */
	return (sndPlaySound(sound_file[v], SND_ASYNC));

#endif /* WIN32 */

#endif /* USE_SOUND */

	/* Oops */
	return (1);
}


/*
 * Delay for "x" milliseconds
 */
int Term_xtra_win_delay(int v)
{

#ifdef WIN32

	/* Sleep */
	Sleep(v);

#else /* WIN32 */

	DWORD t;
	MSG   msg;

	/* Final count */
	t = GetTickCount() + v;

	/* Wait for it */
	while (GetTickCount() < t)
	{
		/* Handle messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#endif /* WIN32 */

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_win(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a bell sound */
		case TERM_XTRA_NOISE:
		return (Term_xtra_win_noise());

		/* Make a special sound */
		case TERM_XTRA_SOUND:
		return (Term_xtra_win_sound(v));

		/* Process random events */
		case TERM_XTRA_BORED:
		return (Term_xtra_win_event(0));

		/* Process an event */
		case TERM_XTRA_EVENT:
		return (Term_xtra_win_event(v));

		/* Flush all events */
		case TERM_XTRA_FLUSH:
		return (Term_xtra_win_flush());

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		return (Term_xtra_win_clear());

		/* React to global changes */
		case TERM_XTRA_REACT:
		return (Term_xtra_win_react());

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
		return (Term_xtra_win_delay(v));
	}

	/* Oops */
	return 1;
}



/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_curs_win(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	RECT   rc;
	HDC    hdc;

	/* Frame the grid */
	rc.left   = x * td->font_wid + td->size_ow1;
	rc.right  = rc.left + td->font_wid;
	rc.top    = y * td->font_hgt + td->size_oh1;
	rc.bottom = rc.top + td->font_hgt;

	/* Cursor is done as a yellow "box" */
	hdc = GetDC(data[0].w);
	FrameRect(hdc, &rc, hbrYellow);
	ReleaseDC(data[0].w, hdc);

	/* Success */
	return 0;
}


/*
 * Low level graphics (Assumes valid input).
 *
 * Erase a "block" of "n" characters starting at (x,y).
 */
static errr Term_wipe_win(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	HDC  hdc;
	RECT rc;

	/* Rectangle to erase in client coords */
	rc.left   = x * td->font_wid + td->size_ow1;
	rc.right  = rc.left + n * td->font_wid;
	rc.top    = y * td->font_hgt + td->size_oh1;
	rc.bottom = rc.top + td->font_hgt;

	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * All "graphic" data is handled by "Term_pict_win()", below.
 *
 * Note that this function assumes the font is monospaced. XXX XXX XXX
 *
 * One would think there is a more efficient method for telling a window
 * what color it should be using to draw with, but perhaps simply changing
 * it every time is not too inefficient.  XXX XXX XXX
 */
static errr Term_text_win(int x, int y, int n, byte a, const char *s)
{
	term_data *td = (term_data*)(Term->data);
	RECT rc;
	HDC  hdc;


	/* Location */
	rc.left   = x * td->font_wid + td->size_ow1;
	rc.right  = rc.left + n * td->font_wid;
	rc.top    = y * td->font_hgt + td->size_oh1;
	rc.bottom = rc.top + td->font_hgt;

	/* Acquire DC */
	hdc = GetDC(td->w);

	/* Background color */
	SetBkColor(hdc, RGB(0, 0, 0));

	/* Foreground color */
	if (colors16)
	{
		SetTextColor(hdc, PALETTEINDEX(win_pal[a&0x0F]));
	}
	else
	{
		SetTextColor(hdc, win_clr[a&0x0F]);
	}

	/* Use the font */
	SelectObject(hdc, td->font_id);

	/* Dump the text */
	ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
	           s, n, NULL);

	/* Release DC */
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw an array of "special" attr/char at the given location.
 *
 * We use the "Term_pict_win()" function for "graphic" data, which are
 * encoded by setting the "high-bits" of both the "attr" and the "char"
 * data.  We use the "attr" to represent the "row" of the main bitmap,
 * and the "char" to represent the "col" of the main bitmap.  The use
 * of this function is induced by the "higher_pict" flag.
 *
 * If we are called for anything but the "screen" window, or if the
 * global "use_graphics" flag is off, we simply "wipe" the given grid.
 *
 * This file should probably be updated to use the files from the Amiga
 * and Macintosh versions, but I am not a Windows programmer yet...
 */
static errr Term_pict_win(int x, int y, int n, const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

#ifdef USE_GRAPHICS

	HDC  hdc;
	HDC hdcSrc;
	HBITMAP hbmSrcOld;
	int i;
	int row, col;
	int x0, y0;
	int x1, y1, w1, h1;
	int x2, y2, w2, h2;

	/* Paranoia */
	if (!use_graphics)
	{
		/* Erase the grids */
		return (Term_wipe_win(x, y, n));
	}

#ifndef FULL_GRAPHICS

	/* Paranoia */
	if (td != &data[0])
	{
		/* Erase the grids */
		return (Term_wipe_win(x, y, n));
	}

#endif

	/* Size of bitmap cell */
	w1 = td->infGraph.CellWidth;
	h1 = td->infGraph.CellHeight;

	/* Size of window cell */
	w2 = td->font_wid;
	h2 = td->font_hgt;

	/* Location of window cell */
	x0 = x * w2 + td->size_ow1;
	y0 = y * h2 + td->size_oh1;

	/* Info */
	hdc = GetDC(td->w);

	/* More info */
	hdcSrc = CreateCompatibleDC(hdc);
	hbmSrcOld = SelectObject(hdcSrc, td->infGraph.hBitmap);

	/* Draw attr/char pairs */
	for (i = 0; i < n; i++, x0 += w2)
	{
		byta a = ap[i];
		char c = cp[i];

		/* Hack XXX XXX XXX */
		row = (a & 0x7F) % 17;
		col = (c & 0x7F) % 64;

		/* Location of bitmap cell */
		x1 = col * w1;
		y1 = row * h1;

		/* Location of screen image */
		x2 = x0;
		y2 = y0;

		/* Handle small bitmaps */
		if ((w1 < w2) || (h1 < h2))
		{
			RECT rc;

			/* Erasure rectangle */
			rc.left   = x2;
			rc.right  = x2 + w2;
			rc.top    = y2;
			rc.bottom = y2 + h2;

			/* Erase the rectangle */
			SetBkColor(hdc, RGB(0, 0, 0));
			SelectObject(hdc, td->font_id);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

			/* Center bitmaps */
			x2 += (w2 - w1) >> 1;
			y2 += (h2 - h1) >> 1;
		}

		/* Copy the picture from the bitmap to the window */
		BitBlt(hdc, x2, y2, w1, h1, hdcSrc, x1, y1, SRCCOPY);
	}

	/* Release */
	SelectObject(hdcSrc, hbmSrcOld);
	DeleteDC(hdcSrc);

	/* Release */
	ReleaseDC(td->w, hdc);

#else

	/* Just erase this grid */
	return (Term_wipe_win(x, y, n));

#endif

	/* Success */
	return 0;
}


/*** Other routines ***/


/*
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	t->soft_cursor = TRUE;

	/* Use "Term_pict" for "graphic" data */
	t->higher_pict = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

#if 0
	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_win;
	t->nuke_hook = Term_nuke_win;
#endif

	/* Prepare the template hooks */
	t->user_hook = Term_user_win;
	t->xtra_hook = Term_xtra_win;
	t->curs_hook = Term_curs_win;
	t->wipe_hook = Term_wipe_win;
	t->text_hook = Term_text_win;
	t->pict_hook = Term_pict_win;

	/* Remember where we came from */
	t->data = (vptr)(td);
}


/*
 * Create the windows
 *
 * First, instantiate the "default" values, then read the "ini_file"
 * to over-ride selected values, then create the windows, and fonts.
 *
 * XXX XXX XXX Need to work on the default window positions
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette. (?)
 */
static void init_windows(void)
{
	int i;

	term_data *td;


	/* Main window */
	td = &data[0];
	WIPE(td, term_data);
	td->s = "Angband 2.8.1";
	td->keys = 1024;
	td->rows = 24;
	td->cols = 80;
	td->visible = TRUE;
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh1 = 2;
	td->size_oh2 = 2;
	td->pos_x = 0;
	td->pos_y = 0;

	/* Sub-windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		/* Mirror window */
		td = &data[i];
		WIPE(td, term_data);
		td->s = angband_term_name[i];
		td->keys = 16;
		td->rows = 24;
		td->cols = 80;
		td->visible = TRUE;
		td->size_ow1 = 1;
		td->size_ow2 = 1;
		td->size_oh1 = 1;
		td->size_oh2 = 1;
		td->pos_x = 0;
		td->pos_y = 0;
	}


	/* Load .INI preferences */
	load_prefs();


	/* Need these before term_getsize gets called */
	data[0].dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU |
	                   WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION |
	                   WS_VISIBLE);
	data[0].dwExStyle = 0;

	/* Windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		data[i].dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU);
		data[i].dwExStyle = (WS_EX_TOOLWINDOW);
	}


	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (term_force_font(&data[i], data[i].font_want))
		{
			(void)term_force_font(&data[i], "8X13.FON");
		}
	}


	/* Screen window */
	td_ptr = &data[0];
	td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AppName,
	                           td_ptr->s, td_ptr->dwStyle,
	                           td_ptr->pos_x, td_ptr->pos_y,
	                           td_ptr->size_wid, td_ptr->size_hgt,
	                           HWND_DESKTOP, NULL, hInstance, NULL);
	if (!td_ptr->w) quit("Failed to create Angband window");
	td_ptr = NULL;
	term_data_link(&data[0]);
	angband_term[0] = &data[0].t;

	/* Windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td_ptr = &data[i];
		td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AngList,
		                           td_ptr->s, td_ptr->dwStyle,
		                           td_ptr->pos_x, td_ptr->pos_y,
		                           td_ptr->size_wid, td_ptr->size_hgt,
		                           HWND_DESKTOP, NULL, hInstance, NULL);
		if (!td_ptr->w) quit("Failed to create sub-window");
		if (td_ptr->visible)
		{
			td_ptr->size_hack = TRUE;
			ShowWindow(td_ptr->w, SW_SHOW);
			td_ptr->size_hack = FALSE;
		}
		td_ptr = NULL;
		term_data_link(&data[i]);
		angband_term[i] = &data[i].t;
	}


	/* Activate the screen window */
	SetActiveWindow(data[0].w);

	/* Bring main screen back to top */
	SetWindowPos(data[0].w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


	/* New palette XXX XXX XXX */
	new_palette();


	/* Create a "brush" for drawing the "cursor" */
	hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);


	/* Process pending messages */
	(void)Term_xtra_win_flush();
}


/*
 * Hack -- disables new and open from file menu
 */
static void disable_start(void)
{
	HMENU hm = GetMenu(data[0].w);

	EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}


/*
 * Prepare the menus
 *
 * XXX XXX XXX See the updated "main-mac.c" for a much nicer
 * system, basically, you explicitly disable any menu option
 * which the user should not be allowed to use, and then you
 * do not have to do any checking when processing the menu.
 */
static void setup_menus(void)
{
	int i;

	HMENU hm = GetMenu(data[0].w);

	/* Save player */
	EnableMenuItem(hm, IDM_FILE_SAVE,
	               MF_BYCOMMAND | (character_generated ? MF_ENABLED : MF_DISABLED | MF_GRAYED));

	/* Exit with save */
	EnableMenuItem(hm, IDM_FILE_EXIT,
	               MF_BYCOMMAND | (character_generated ? MF_ENABLED : MF_DISABLED | MF_GRAYED));


	/* Window font options */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		/* Window font */
		EnableMenuItem(hm, IDM_TEXT_SCREEN + i,
		               MF_BYCOMMAND | (data[i].visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
	}

	/* Window options */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		/* Window */
		CheckMenuItem(hm, IDM_WINDOWS_SCREEN + i,
		              MF_BYCOMMAND | (data[i].visible ? MF_CHECKED : MF_UNCHECKED));
	}

	/* Item "Graphics" */
	CheckMenuItem(hm, IDM_OPTIONS_GRAPHICS,
	              MF_BYCOMMAND | (arg_graphics ? MF_CHECKED : MF_UNCHECKED));

	/* Item "Sound" */
	CheckMenuItem(hm, IDM_OPTIONS_SOUND,
	              MF_BYCOMMAND | (arg_sound ? MF_CHECKED : MF_UNCHECKED));

#ifdef BEN_HACK 
	/* Item "Colors 16" */
	EnableMenuItem(hm, IDM_OPTIONS_UNUSED,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
#endif

#ifndef ALLOW_SCRSAVER
	/* Item "Run as Screensaver" */
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
#endif

}


/*
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

	/* Second arg */
	p = strchr(s, ' ');

	/* Tokenize, advance */
	if (p) *p++ = '\0';

	/* No args */
	if (!*s) return;

	/* Extract filename */
	strcat(savefile, s);

	/* Validate the file */
	validate_file(savefile);

	/* Game in progress */
	game_in_progress = TRUE;

	/* No restarting */
	disable_start();

	/* Play game */
	play_game(FALSE);
}


/*
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
	int i;

	OPENFILENAME ofn;

	/* Analyze */
	switch (wCmd)
	{
		/* New game */
		case IDM_FILE_NEW:
		{
			if (!initialized)
			{
				MessageBox(data[0].w, "You cannot do that yet...",
				           "Warning", MB_ICONEXCLAMATION | MB_OK);
			}
			else if (game_in_progress)
			{
				MessageBox(data[0].w,
				           "You can't start a new game while you're still playing!",
				           "Warning", MB_ICONEXCLAMATION | MB_OK);
			}
			else
			{
				game_in_progress = TRUE;
				disable_start();
				Term_flush();
				play_game(TRUE);
				quit(NULL);
			}
			break;
		}

		/* Open game */
		case IDM_FILE_OPEN:
		{
			if (!initialized)
			{
				MessageBox(data[0].w, "You cannot do that yet...",
				           "Warning", MB_ICONEXCLAMATION | MB_OK);
			}
			else if (game_in_progress)
			{
				MessageBox(data[0].w,
				           "You can't open a new game while you're still playing!",
				           "Warning", MB_ICONEXCLAMATION | MB_OK);
			}
			else
			{
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = data[0].w;
				ofn.lpstrFilter = "Save Files (*.)\0*\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = savefile;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = ANGBAND_DIR_SAVE;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileName(&ofn))
				{
					/* Load 'savefile' */
					validate_file(savefile);
					game_in_progress = TRUE;
					disable_start();
					Term_flush();
					play_game(FALSE);
					quit(NULL);
				}
			}
			break;
		}

		/* Save game */
		case IDM_FILE_SAVE:
		{
			if (!game_in_progress)
			{
				MessageBox(data[0].w, "No game in progress.",
				           "Warning", MB_ICONEXCLAMATION | MB_OK);
			}
			else
			{
				/* Save the game */
				do_cmd_save_game();
			}
			break;
		}

		/* Save and Exit */
		case IDM_FILE_EXIT:
		{
			if (game_in_progress && character_generated)
			{
				/* Hack -- Forget messages */
				msg_flag = FALSE;

				/* Save the game */
				do_cmd_save_game();
			}
			quit(NULL);
			break;
		}

		/* Quit (no save) */
		case IDM_FILE_QUIT:
		{
			if (game_in_progress && character_generated)
			{
				if (MessageBox(data[0].w,
				               "Your character will be not saved!",
				               "Warning", MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
				{
					break;
				}
			}
			quit(NULL);
			break;
		}

		case IDM_TEXT_SCREEN:
		{

#ifdef USE_GRAPHICS
			/* XXX XXX XXX */
			if (use_graphics)
			{
				term_change_bitmap(&data[0]);
				break;
			}
#endif

			term_change_font(&data[0]);
			break;
		}

		/* Window fonts */
		case IDM_TEXT_MIRROR:
		case IDM_TEXT_RECALL:
		case IDM_TEXT_CHOICE:
		case IDM_TEXT_TERM_4:
		case IDM_TEXT_TERM_5:
		case IDM_TEXT_TERM_6:
		case IDM_TEXT_TERM_7:
		{
			i = wCmd - IDM_TEXT_SCREEN;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			term_change_font(&data[i]);

			break;
		}

		/* Window visibility */
		case IDM_WINDOWS_MIRROR:
		case IDM_WINDOWS_RECALL:
		case IDM_WINDOWS_CHOICE:
		case IDM_WINDOWS_TERM_4:
		case IDM_WINDOWS_TERM_5:
		case IDM_WINDOWS_TERM_6:
		case IDM_WINDOWS_TERM_7:
		{
			i = wCmd - IDM_WINDOWS_SCREEN;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			if (!data[i].visible)
			{
				data[i].visible = TRUE;
				ShowWindow(data[i].w, SW_SHOW);
				term_data_redraw(&data[i]);
			}
			else
			{
				data[i].visible = FALSE;
				ShowWindow(data[i].w, SW_HIDE);
			}

			break;
		}

		/* Hack -- unused */
		case IDM_OPTIONS_UNUSED:
		{
			/* XXX XXX XXX */
			break;
		}

		case IDM_OPTIONS_GRAPHICS:
		{
			/* Toggle "arg_graphics" */
			arg_graphics = !arg_graphics;

			/* React to changes */
			Term_xtra_win_react();

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}

		case IDM_OPTIONS_SOUND:
		{
			/* Toggle "arg_sound" */
			arg_sound = !arg_sound;

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}

#ifdef ALLOW_SCRSAVER
		case IDM_OPTIONS_SAVER:
		{
			hwndSaver = CreateWindowEx(WS_EX_TOPMOST, "WindowsScreenSaverClass", "Borg",
			                           WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
			                           0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			                           NULL, NULL, hInstance, NULL);
			if (!hwndSaver)
			{
				MessageBox(data[0].w, "Failed to create saver window", NULL, MB_OK);
			}
			break;
		}
#endif

		case IDM_HELP_GENERAL:
		{
			char buf[1024];
			char tmp[1024];
			path_build(tmp, 1024, ANGBAND_DIR_XTRA_HELP, "angband.hlp");
			sprintf(buf, "winhelp.exe %s", tmp);
			WinExec(buf, SW_NORMAL);
			break;
		}

		case IDM_HELP_SPOILERS:
		{
			char buf[1024];
			char tmp[1024];
			path_build(tmp, 1024, ANGBAND_DIR_XTRA_HELP, "spoilers.hlp");
			sprintf(buf, "winhelp.exe %s", tmp);
			WinExec(buf, SW_NORMAL);
			break;
		}
	}
}



#ifdef BEN_HACK
LRESULT FAR PASCAL _export AngbandWndProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam);
#endif

LRESULT FAR PASCAL _export AngbandWndProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	HDC             hdc;
	term_data      *td;
	MINMAXINFO FAR *lpmmi;
	RECT            rc;
	int             i;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Handle message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(td_ptr));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			lpmmi = (MINMAXINFO FAR *)lParam;

			if (!td) return 1;  /* this message was sent before WM_NCCREATE */

			/* Minimum window size is 8x2 */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 24 * td->font_hgt + td->size_oh1 + td->size_oh2;

			/* Paranoia */
			rc.right  += (td->font_wid - 1);
			rc.bottom += (td->font_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x      = rc.right - rc.left;
			lpmmi->ptMaxSize.y      = rc.bottom - rc.top;

			/* Save maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

		case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			if (td) term_data_redraw(td);
			EndPaint(hWnd, &ps);
			ValidateRect(hWnd, NULL);   /* why needed ?? */
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			BYTE KeyState = 0x00;
			bool enhanced = FALSE;
			bool mc = FALSE;
			bool ms = FALSE;
			bool ma = FALSE;

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
			if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
			if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

			/* Check for non-normal keys */
			if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN)) enhanced = TRUE;
			if ((wParam >= VK_F1) && (wParam <= VK_F12)) enhanced = TRUE;
			if ((wParam == VK_INSERT) || (wParam == VK_DELETE)) enhanced = TRUE;

			/* XXX XXX XXX */
			if (enhanced)
			{
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" from bits 16..23 of lParam */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i/16]);
				Term_keypress(hexsym[i%16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

		case WM_INITMENU:
		{
			setup_menus();
			return 0;
		}

		case WM_CLOSE:
		case WM_QUIT:
		{
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
			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (td->size_hack) return 1; /* was sent from WM_SIZE */

			switch (wParam)
			{
				case SIZE_MINIMIZED:
				{
					for (i = 1; i < MAX_TERM_DATA; i++)
					{
						if (data[i].visible) ShowWindow(data[i].w, SW_HIDE);
					}
					return 0;
				}

				case SIZE_MAXIMIZED:
				{
					/* fall through XXX XXX XXX */
				}

				case SIZE_RESTORED:
				{
					term *old = Term;

					td->size_hack = TRUE;

					td->cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
					td->rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;

					term_getsize(td);

					MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);

					td->size_hack = FALSE;

					/* Windows */
					for (i = 1; i < MAX_TERM_DATA; i++)
					{
						if (data[i].visible) ShowWindow(data[i].w, SW_SHOWNOACTIVATE);
					}

					return 0;
				}
			}
			break;
		}

		case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return FALSE;
			/* otherwise, fall through!!! */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return FALSE;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, FALSE);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);
			return FALSE;
		}

		case WM_ACTIVATE:
		{
			if (wParam && !HIWORD(lParam))
			{
				for (i = 1; i < MAX_TERM_DATA; i++)
				{
					SetWindowPos(data[i].w, hWnd, 0, 0, 0, 0,
					             SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
				}
				SetFocus(hWnd);
				return 0;
			}
			break;
		}
	}


	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef BEN_HACK
LRESULT FAR PASCAL _export AngbandListProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam);
#endif

LRESULT FAR PASCAL _export AngbandListProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam)
{
	term_data      *td;
	MINMAXINFO FAR *lpmmi;
	RECT            rc;
	PAINTSTRUCT     ps;
	HDC             hdc;
	int             i;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Process message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(td_ptr));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			if (!td) return 1;  /* this message was sent before WM_NCCREATE */

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* Minimum size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->font_hgt + td->size_oh1 + td->size_oh2;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save the minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->font_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 24 * td->font_hgt + td->size_oh1 + td->size_oh2;

			/* Paranoia */
			rc.right += (td->font_wid - 1);
			rc.bottom += (td->font_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x      = rc.right - rc.left;
			lpmmi->ptMaxSize.y      = rc.bottom - rc.top;

			/* Save the maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

		case WM_SIZE:
		{
			term *old = Term;

			if (!td) return 1;    /* this message was sent before WM_NCCREATE */
			if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
			if (td->size_hack) return 1; /* was sent from inside WM_SIZE */

			td->size_hack = TRUE;

			td->cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
			td->rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;

			term_getsize(td);

			MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);

			td->size_hack = FALSE;

			return 0;
		}

		case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			if (td) term_data_redraw(td);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			BYTE KeyState = 0x00;
			bool enhanced = FALSE;
			bool mc = FALSE;
			bool ms = FALSE;
			bool ma = FALSE;

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
			if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
			if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

			/* Check for non-normal keys */
			if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN)) enhanced = TRUE;
			if ((wParam >= VK_F1) && (wParam <= VK_F12)) enhanced = TRUE;
			if ((wParam == VK_INSERT) || (wParam == VK_DELETE)) enhanced = TRUE;

			/* XXX XXX XXX */
			if (enhanced)
			{
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" from bits 16..23 of lParam */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i/16]);
				Term_keypress(hexsym[i%16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

		case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return FALSE;
			/* otherwise, fall through!!! */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, FALSE);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);
			return 0;
		}

		case WM_NCLBUTTONDOWN:
		{
			if (wParam == HTSYSMENU)
			{
				/* Hide sub-windows */
				if (td != &data[0])
				{
					if (td->visible)
					{
						td->visible = FALSE;
						ShowWindow(td->w, SW_HIDE);
					}
				}
				return 0;
			}
			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef ALLOW_SCRSAVER

#define MOUSE_SENS 40

LRESULT FAR PASCAL _export AngbandSaverProc(HWND hWnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;

	term_data *td;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Process */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(td_ptr));
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
#endif

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_KEYDOWN:
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			if (iMouse)
			{
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
				{
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}
			}

			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);
			return 0;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
			return 0;
		}
	}

	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif





/*** Temporary Hooks ***/


/*
 * Error message -- See "z-util.c"
 */
static void hack_plog(cptr str)
{
	/* Give a warning */
	if (str) MessageBox(NULL, str, "Warning", MB_OK);
}


/*
 * Quit with error message -- See "z-util.c"
 */
static void hack_quit(cptr str)
{
	/* Give a warning */
	if (str) MessageBox(NULL, str, "Error", MB_OK | MB_ICONSTOP);

	/* Unregister the classes */
	UnregisterClass(AppName, hInstance);

	/* Destroy the icon */
	if (hIcon) DestroyIcon(hIcon);

	/* Exit */
	exit (0);
}


/*
 * Fatal error (see "z-util.c")
 */
static void hack_core(cptr str)
{
	/* Give a warning */
	if (str) MessageBox(NULL, str, "Error", MB_OK | MB_ICONSTOP);

	/* Quit */
	quit (NULL);
}


/*** Various hooks ***/


/*
 * Error message -- See "z-util.c"
 */
static void hook_plog(cptr str)
{
	/* Warning */
	if (str) MessageBox(data[0].w, str, "Warning", MB_OK);
}


/*
 * Quit with error message -- See "z-util.c"
 *
 * A lot of this function should be handled by actually calling
 * the "term_nuke()" function hook for each window.  XXX XXX XXX
 */
static void hook_quit(cptr str)
{
	int i;


	/* Give a warning */
	if (str) MessageBox(data[0].w, str, "Error", MB_OK | MB_ICONSTOP);


	/* Save the preferences */
	save_prefs();

	/* Sub-Windows */
	for (i = MAX_TERM_DATA - 1; i >= 1; i--)
	{
		term_force_font(&data[i], NULL);
		if (data[i].font_want) string_free(data[i].font_want);
		if (data[i].graf_want) string_free(data[i].graf_want);
		if (data[i].w) DestroyWindow(data[i].w);
		data[i].w = 0;
	}

#ifdef USE_GRAPHICS
	term_force_graf(&data[0], NULL);
#endif
	term_force_font(&data[0], NULL);
	if (data[0].font_want) string_free(data[0].font_want);
	if (data[0].graf_want) string_free(data[0].graf_want);
	if (data[0].w) DestroyWindow(data[0].w);
	data[0].w = 0;


	/*** Free some other stuff ***/

	DeleteObject(hbrYellow);

	if (hPal) DeleteObject(hPal);

	UnregisterClass(AppName, hInstance);

	if (hIcon) DestroyIcon(hIcon);

	exit(0);
}


/*
 * Fatal error (see "z-util.c")
 */
static void hook_core(cptr str)
{
	/* Message */
	if (str) MessageBox(data[0].w, str, "Error", MB_OK);

	/* Another message */
	MessageBox(data[0].w, "I will now attempt to save and quit.",
	           "Fatal error", MB_OK | MB_ICONSTOP);

	/* Save the player XXX XXX XXX */
	save_player();

	/* Quit */
	quit(NULL);
}




/*** Initialize ***/


/*
 * Init some stuff
 */
static void init_stuff(void)
{
	int   i;

	char path[1024];


	/* Hack -- access "ANGBAND.INI" */
	GetModuleFileName(hInstance, path, 512);
	strcpy(path + strlen(path) - 4, ".INI");

	/* Save "ANGBAND.INI" */
	ini_file = string_make(path);

	/* Validate "ANGBAND.INI" */
	validate_file(ini_file);


	/* XXX XXX XXX */
	GetPrivateProfileString("Angband", "LibPath", "c:\\angband\\lib",
	                        path, 1000, ini_file);

	/* Analyze the path */
	i = strlen(path);

	/* Require a path */
	if (!i) quit("LibPath shouldn't be empty in ANGBAND.INI");

	/* Nuke terminal backslash */
	if (i && (path[i-1] != '\\'))
	{
		path[i++] = '\\';
		path[i] = '\0';
	}

	/* Validate the path */
	validate_dir(path);


	/* Init the file paths */
	init_file_paths(path);

#if 0
	/* Mega-Hack XXX XXX XXX */
	if (!check_dir(ANGBAND_DIR_APEX))
	{
		mkdir(ANGBAND_DIR_APEX);
	}
#endif

	/* Hack -- Validate the paths */
	validate_dir(ANGBAND_DIR_APEX);
	validate_dir(ANGBAND_DIR_BONE);
	validate_dir(ANGBAND_DIR_DATA);
	validate_dir(ANGBAND_DIR_EDIT);
	validate_dir(ANGBAND_DIR_FILE);
	validate_dir(ANGBAND_DIR_HELP);
	validate_dir(ANGBAND_DIR_INFO);
	validate_dir(ANGBAND_DIR_SAVE);
	validate_dir(ANGBAND_DIR_USER);
	validate_dir(ANGBAND_DIR_XTRA);


	/* Build the "font" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "font");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_FONT = string_make(path);

	/* Validate the "font" directory */
	validate_dir(ANGBAND_DIR_XTRA_FONT);

	/* Build the filename */
	path_build(path, 1024, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

	/* Hack -- Validate the basic font */
	validate_file(path);


#ifdef USE_GRAPHICS

	/* Build the "graf" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "graf");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_GRAF = string_make(path);

	/* Validate the "graf" directory */
	validate_dir(ANGBAND_DIR_XTRA_GRAF);

	/* Build the filename */
	path_build(path, 1024, ANGBAND_DIR_XTRA_GRAF, "8X13.BMP");

	/* Hack -- Validate the basic graf */
	validate_file(path);

#endif


#ifdef USE_SOUND

	/* Build the "sound" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "sound");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_SOUND = string_make(path);

	/* Validate the "sound" directory */
	validate_dir(ANGBAND_DIR_XTRA_SOUND);

#endif


	/* Build the "help" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "help");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_HELP = string_make(path);

	/* Validate the "help" directory */
	/* validate_dir(ANGBAND_DIR_XTRA_HELP); */


	/* Build the filename */
	path_build(path, 1024, ANGBAND_DIR_FILE, "news.txt");

	/* Hack -- Validate the "news.txt" file */
	validate_file(path);
}


int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPSTR lpCmdLine, int nCmdShow)
{
	int i;

	WNDCLASS wc;
	HDC      hdc;
	MSG      msg;

	hInstance = hInst;  /* save in a global var */

	if (hPrevInst == NULL)
	{
		wc.style         = CS_CLASSDC;
		wc.lpfnWndProc   = AngbandWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 4; /* one long pointer to term_data */
		wc.hInstance     = hInst;
		wc.hIcon         = hIcon = LoadIcon(hInst, AppName);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName  = AppName;
		wc.lpszClassName = AppName;

		if (!RegisterClass(&wc)) exit(1);

		wc.lpfnWndProc   = AngbandListProc;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = AngList;

		if (!RegisterClass(&wc)) exit(2);

#ifdef ALLOW_SCRSAVER

		wc.style          = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
		wc.lpfnWndProc    = AngbandSaverProc;
		wc.hCursor        = NULL;
#if 0
		wc.hIcon          = LoadIcon(hInst, MAKEINTATOM(ID_APP));
#endif
		wc.lpszMenuName   = NULL;
		wc.lpszClassName  = "WindowsScreenSaverClass";

		if (!RegisterClass(&wc)) exit(3);

#endif

	}

	/* Temporary hooks */
	plog_aux = hack_plog;
	quit_aux = hack_quit;
	core_aux = hack_core;

	/* Prepare the filepaths */
	init_stuff();

	/* Determine if display is 16/256/true color */
	hdc = GetDC(NULL);
	colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
	paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE);
	ReleaseDC(NULL, hdc);

	/* Initialize "color_table" */
	for (i = 0; i < 16; i++)
	{
		/* Save the "complex" codes */
		color_table[i][1] = GetRValue(win_clr[i]);
		color_table[i][2] = GetGValue(win_clr[i]);
		color_table[i][3] = GetBValue(win_clr[i]);

		/* Save the "simple" code */
		color_table[i][0] = win_pal[i];
	}

	/* Prepare the windows */
	init_windows();

	/* Activate hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_core;

	/* Set the system suffix */
	ANGBAND_SYS = "win";

	/* Initialize */
	init_angband();

	/* We are now initialized */
	initialized = TRUE;

	/* Did the user double click on a save file? */
	check_for_save_file(lpCmdLine);

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
	Term_fresh();

	/* Process messages forever */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Paranoia */
	quit(NULL);

	/* Paranoia */
	return (0);
}

#endif /* _Windows */
