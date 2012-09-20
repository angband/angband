/* File: main-win.c */

/* Purpose: Support for Windows Angband */

/*
 * Written by Skirmantas Kligys (kligys@scf.usc.edu)
 *
 * Based loosely on "main-mac.c" and "main-xxx.c" and suggestions
 * by Ben Harrison (benh@voicenet.com).
 *
 * Angband 2.7.9v4 modifications by Ben Harrison (benh@voicenet.com),
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
 * The "use_graphics" option should affect ALL of the windows,
 * for example, the "recall" window needs the monster pictures.
 * This may require changes to many different pieces of the code,
 * for example, the "infGraph" variable should perhaps be broken
 * into one "infGraph" for each window, or for each active font.
 *
 * XXX XXX XXX
 * Currently we "force" the use of the "10x20" font when "use_graphics"
 * is activated, instead of simply forcing the use of the current font
 * (which would load the various bitmaps files), to prevent any nasty
 * errors in the "term_force_font()" function.
 *
 * XXX XXX XXX
 * Note that starting in "use_graphics" mode with a font that does
 * not have a corresponding bitmap file is not allowed.
 *
 * XXX XXX XXX
 * We need to attempt to handle user modification of the color set
 * in the "Term_xtra_win_react()" function, and to correctly prepare
 * the global "color_table[]" variable at startup.  Also, we should
 * modify the "default" color set.  See "main-ibm.c" for more info.
 *
 * XXX XXX XXX 
 * We would like "term.c" to correctly handle the "Term_pict()" routine,
 * but we need to be careful about verifying the "use_graphics" flag.
 *
 * XXX XXX XXX
 * We need to think about the meaning of "scrollable" windows, and about
 * a "resizable" main window, especially in terms of efficiency.  It seems
 * that it would be sufficient to allow resizing and NOT allow scrolling.
 * How often do you REALLY need to scroll Angband windows, anyway?  And
 * if we MUST scroll them, is there any way to have Windows itself take
 * care of doing the offsets and clipping so we can ignore it?
 *
 * XXX XXX XXX
 * We should allow the use of bitmap sets with a "size" which is SMALLER
 * than the current font, though they may not look very good, and all the
 * bitmaps should be "centered" in the grid if they are too small.  This
 * may also require changes to a lot of the code.
 *
 * XXX XXX XXX
 * Verify the use of the "td_ptr" variable, and the various silliness
 * required to have the "screen" window act in a "special" way.
 *
 * XXX XXX XXX
 * Something is wrong with the creation of all the "sub-windows".
 *
 * XXX XXX XXX
 * Something was wrong with the loading of the fonts, may be fixed.
 *
 * XXX XXX XXX
 * The various "warning" messages assume the existance of the "screen.w"
 * window, I think, and only a few calls actually check for its existance.
 *
 * XXX XXX XXX
 * The "ANGBAND.INI" file needs to use "Visible" for the "window is shown"
 * flags, and needs an entry for the Mirror window.  The "ANGBAND.RC" (?)
 * file needs menu item(s) for the mirror window.  Some of the "sound"
 * files should be slightly renamed.
 *
 * XXX XXX XXX
 * Special "Windows Help Files" can be placed into "lib/xtra/help/" for
 * use with the "winhelp.exe" program.
 *
 * XXX XXX XXX
 * The "prepare menus" command should "gray out" any menu command which
 * is not allowed at the current time.  This will simplify the actual
 * processing of menu commands, which can assume the command is legal.
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
 * Menu constants
 */
#define IDM_FILE_NEW             101
#define IDM_FILE_OPEN            102
#define IDM_FILE_SAVE            103
#define IDM_FILE_EXIT            104
#define IDM_FILE_QUIT            105
#define IDM_OPTIONS_FONT_ANGBAND 201
#define IDM_OPTIONS_FONT_MIRROR  202
#define IDM_OPTIONS_FONT_RECALL  203
#define IDM_OPTIONS_FONT_CHOICE  204
#define IDM_OPTIONS_MIRROR       212
#define IDM_OPTIONS_RECALL       213
#define IDM_OPTIONS_CHOICE       214
#define IDM_OPTIONS_RESIZABLE    221
#define IDM_OPTIONS_GRAPHICS     222
#define IDM_OPTIONS_SOUND        223
#define IDM_OPTIONS_SAVER        231
#define IDM_HELP_GENERAL         901
#define IDM_HELP_SPOILERS        902

/*
 * XXX XXX XXX This may need to be removed for some compilers
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
 * The "itsybitsy" code (?) does not work with Win32
 */
#ifdef WIN32
# undef  USE_ITSYBITSY
#endif

/*
 * Include the "itsybits" support
 */
#ifdef USE_ITSYBITSY
# include "itsybits.h"
#endif

/*
 * Include the support for loading bitmaps
 */
#include "readdib.h"

/*
 * Hack -- allow use of the Borg as a screen-saver
 */
#ifdef ALLOW_BORG
# define ALLOW_SCRSAVER
#endif

/*
 * string.h excludes this because __STDC__=1
 */
int stricmp(const char *, const char *);

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
# define MoveTo(H,X,Y) \
         MoveToEx(H, X, Y, NULL)
#endif /* WIN32 */

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
#define VUD_BLINK	0x80


/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * Extra "term" data
 *
 * XXX XXX XXX The "type_1" variable is TRUE for "closable" windows,
 * that is, everything BUT the main window.  The "type_2" variable
 * should be used to replace the "screen_resizable" global variable,
 * and would mark a window as "resizable".  Some other things check
 * against the actual identity of the window.
 *
 * Note the use of "want_file" for the name of the font or bitmap
 * that the user thinks he wants, and the use of "font_file" and
 * "graf_file" for the names of the actual font and/or bitmap that
 * have actually been loaded.  XXX XXX XXX
 *
 * The "font_file" and "graf_file" are capitilized, and are of the
 * form "7X13.FON" and "7X13.BMP", while "want_file" can be in many
 * formats ("7x13", "7x13.fon", "7x13.bmp", etc).
 */
struct _term_data {

  term     t;

  cptr     s;

  HWND     w;

  DWORD    dwStyle;
  DWORD    dwExStyle;

  byte     type_1;
  byte     type_2;

  uint     keys;

  uint     rows;
  uint     cols;
  uint     vis_rows;
  uint     vis_cols;
  uint     scroll_vpos;
  uint     scroll_hpos;

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

#ifdef USE_ITSYBITSY
  uint     cap_size;
#endif

  byte     visible;
  byte     resizing;

  cptr     want_file;

  cptr     font_file;
  cptr     graf_file;

  HFONT    font_id;

  uint     font_wid;
  uint     font_hgt;
};


/*
 * The "screen" window
 */
static term_data screen;

#ifdef GRAPHIC_MIRROR
/*
 * The "mirror" window
 */
static term_data mirror;
#endif

#ifdef GRAPHIC_RECALL
/*
 * The "recall" window
 */
static term_data recall;
#endif

#ifdef GRAPHIC_CHOICE
/*
 * The "choice" window
 */
static term_data choice;
#endif

/*
 * Hack -- flag used for the window creation routines
 */
static term_data *td_ptr;

/*
 * Various boolean flags
 */
bool game_in_progress  = FALSE;  /* game in progress */
bool save_enabled      = FALSE;  /* game can be saved */
bool screen_resizable  = FALSE;  /* main window ("screen") resizable */
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
 * The actual "graphic data" for the "screen" window XXX XXX XXX
 */
static DIBINIT infGraph;

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
 * XXX XXX XXX These colors are slightly out of date, see "defines.h"
 * XXX XXX XXX See "main-ibm.c" for a more up to date set of colors
 * XXX XXX XXX See "main-ibm.c" for a method to allow color editing
 */
static const COLORREF win_clr[16] = {

  PALETTERGB(0x00, 0x00, 0x00),  /* BLACK */
  PALETTERGB(0xFF, 0xFF, 0xFF),  /* WHITE */
  PALETTERGB(0xA0, 0xA0, 0xA0),  /* GRAY */
  PALETTERGB(0xFF, 0x92, 0x00),  /* ORANGE */
  PALETTERGB(0xB0, 0x00, 0x00),  /* RED */
  PALETTERGB(0x00, 0xB0, 0x00),  /* GREEN */
  PALETTERGB(0x00, 0x00, 0xFF),  /* BLUE */
  PALETTERGB(0xC8, 0x64, 0x00),  /* UMBER */
  PALETTERGB(0x70, 0x70, 0x70),  /* DARKGRAY */
  PALETTERGB(0xD0, 0xD0, 0xD0),  /* LIGHTGRAY */
  PALETTERGB(0xA5, 0x00, 0xFF),  /* VIOLET */
  PALETTERGB(0xFF, 0xFD, 0x00),  /* YELLOW */
  PALETTERGB(0xFF, 0x00, 0xBC),  /* LIGHTRED */
  PALETTERGB(0x00, 0xFF, 0x00),  /* LIGHTGREEN */
  PALETTERGB(0x00, 0xC8, 0xFF),  /* LIGHTBLUE */
  PALETTERGB(0xFF, 0xCC, 0x80)   /* LIGHTUMBER */
};


/*
 * Palette indices for 16 colors XXX XXX XXX (?)
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static const BYTE win_pal[16] = {

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
    VID_MAGENTA,		/* Violet */
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
  if (path[i-1] == '\\') path[--i] = '\0';

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
  HDC         hdcDesktop;
  HFONT       hfOld;
  TEXTMETRIC  tm;
  RECT        rc;

  /* all this trouble to get the cell size */
  hdcDesktop = GetDC(HWND_DESKTOP);
  hfOld = SelectObject(hdcDesktop, td->font_id);
  GetTextMetrics(hdcDesktop, &tm);
  SelectObject(hdcDesktop, hfOld);
  ReleaseDC(HWND_DESKTOP, hdcDesktop);

  /* Font size info */
  td->font_wid = tm.tmAveCharWidth;
  td->font_hgt = tm.tmHeight;

  /* Window sizes */
  td->client_wid = td->vis_cols * td->font_wid + td->size_ow1 + td->size_ow2;
  td->client_hgt = td->vis_rows * td->font_hgt + td->size_oh1 + td->size_oh2;

  /* position not important */
  rc.left = rc.top = 0;
  rc.right = rc.left + td->client_wid;
  rc.bottom = rc.top + td->client_hgt;

  /* Main window */
  if (!(td->type_1))
  {
    if (screen_resizable)
    {
      rc.right  += GetSystemMetrics(SM_CXVSCROLL) - 1;
      rc.bottom += GetSystemMetrics(SM_CYHSCROLL) - 1;
    }
  }

  /* Sub-windows */
  else
  {

#ifdef USE_ITSYBITSY

    rc.right += GetSystemMetrics(SM_CXVSCROLL);
    rc.bottom += 1;

#else

    /* Note the nasty off by one silliness */
    rc.right += GetSystemMetrics(SM_CXVSCROLL) + 1;
    rc.bottom -= 1;

#endif

  }

#ifdef USE_ITSYBITSY

  if (!(td->type_1))
  {
    AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);
  }
  else
  {
    ibAdjustWindowRect(&rc, td->dwStyle, FALSE, td->cap_size);
  }

#else

  AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

#endif

  /* Total size */
  td->size_wid = rc.right - rc.left;
  td->size_hgt = rc.bottom - rc.top;

  /* See CreateWindowEx */
  if (!td->w) return;

  /* Extract actual location */
  GetWindowRect(td->w, &rc);
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

  if (td->w)
  {
    /* Main window */
    if (!(td->type_1))
    {
      strcpy(buf, screen_resizable ? "1" : "0");
      WritePrivateProfileString(sec_name, "Resizable", buf, ini_file);
    }

    /* Sub-windows */
    else
    {
      strcpy(buf, td->visible ? "1" : "0");
      WritePrivateProfileString(sec_name, "Visible", buf, ini_file);

#ifdef USE_ITSYBITSY
      wsprintf(buf, "%d", td->cap_size);
      WritePrivateProfileString(sec_name, "CapSize", buf, ini_file);
#endif

    }

    if (td->font_file)
    {
      WritePrivateProfileString(sec_name, "FontFile", td->font_file, ini_file);
    }

    wsprintf(buf, "%d", td->vis_cols);
    WritePrivateProfileString(sec_name, "Columns", buf, ini_file);

    wsprintf(buf, "%d", td->vis_rows);
    WritePrivateProfileString(sec_name, "Rows", buf, ini_file);

    GetWindowRect(td->w, &rc);

    wsprintf(buf, "%d", rc.left);
    WritePrivateProfileString(sec_name, "PositionX", buf, ini_file);

    wsprintf(buf, "%d", rc.top);
    WritePrivateProfileString(sec_name, "PositionY", buf, ini_file);
  }
}


/*
 * Write the "preference" data to the .INI file
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)
{
  char       buf[32];
  RECT       rc;
  term_data *td;

  strcpy(buf, use_graphics ? "1" : "0");
  WritePrivateProfileString("Angband", "Graphics", buf, ini_file);

  strcpy(buf, use_sound ? "1" : "0");
  WritePrivateProfileString("Angband", "Sound", buf, ini_file);

  save_prefs_aux(&screen, "Main window");
  
#ifdef GRAPHIC_MIRROR
  save_prefs_aux(&mirror, "Mirror window");
#endif

#ifdef GRAPHIC_RECALL
  save_prefs_aux(&recall, "Recall window");
#endif

#ifdef GRAPHIC_CHOICE
  save_prefs_aux(&choice, "Choice window");
#endif

}


/*
 * Load preference for a single term
 */
static void load_prefs_aux(term_data *td, cptr sec_name)
{
  char tmp[128];
  char buf[128];

  /* Main window */
  if (!(td->type_1))
  {
    screen_resizable = (GetPrivateProfileInt(sec_name, "Resizable", 0, ini_file) != 0);
  }

  /* Sub-window */
  else
  {
    td->visible = (GetPrivateProfileInt(sec_name, "Visible", 0, ini_file) != 0);

#ifdef USE_ITSYBITSY
    td->cap_size = GetPrivateProfileInt(sec_name, "CapSize", 0, ini_file);
    if (td->cap_size > 127) td->cap_size = 127;
#endif

  }

  /* Extract font choice */
  GetPrivateProfileString(sec_name, "FontFile", "7x13.fon", tmp, 127, ini_file);

  /* Hack -- Save the desired font choice */
  td->want_file = string_make(extract_file_name(tmp));

  /* Window size */
  td->vis_cols = GetPrivateProfileInt(sec_name, "Columns", 80, ini_file);
  td->vis_rows = GetPrivateProfileInt(sec_name, "Rows", 5, ini_file);

  /* Window position */
  td->pos_x = GetPrivateProfileInt(sec_name, "PositionX", 0, ini_file);
  td->pos_y = GetPrivateProfileInt(sec_name, "PositionY", 0, ini_file);
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
  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "sound\\");
  strcat(buf, extract_file_name(tmp));

  /* Save the sound filename, if it exists */
  if (check_file(buf)) sound_file[i] = string_make(buf);
}


/*
 * Load the preferences from the .INI file
 */
static void load_prefs(void)
{
  int i;
  
  /* Extract the "use_graphics" flag */
  use_graphics = (GetPrivateProfileInt("Angband", "Graphics", 0, ini_file) != 0);

  /* Extract the "use_sound" flag */
  use_sound = (GetPrivateProfileInt("Angband", "Sound", 0, ini_file) != 0);

  /* Load window prefs */
  load_prefs_aux(&screen, "Main window");

#ifdef GRAPHIC_MIRROR
  load_prefs_aux(&mirror, "Mirror window");
#endif

#ifdef GRAPHIC_RECALL
  load_prefs_aux(&recall, "Recall window");
#endif

#ifdef GRAPHIC_CHOICE
  load_prefs_aux(&choice, "Choice window");
#endif

  /* Prepare the sounds */
  for (i = 1; i < SOUND_MAX; i++) load_prefs_sound(i);
}


/*
 * Create the new global palette based on the bitmap palette
 * (if any) from the given bitmap xxx, and the standard 16
 * entry palette derived from "win_clr[]" which is used for
 * the basic 16 Angband colors.
 *
 * XXX XXX XXX Use this function to allow changing the colors.
 */
void new_palette(void)
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


    /* Check the bitmap palette */
    hBmPal = infGraph.hPalette;

    /* No bitmap */
    if (!hBmPal)
    {
      lppeSize = 0;
      lppe = NULL;
      nEntries = 0;
    }

    /* Use the bitmap */
    else
    {
      lppeSize = 256*sizeof(PALETTEENTRY);
      lppe = (LPPALETTEENTRY)ralloc(lppeSize);
      nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
      if (nEntries == 0) quit("Corrupted bitmap palette");
      if (nEntries > 220) quit("Bitmap must have no more than 220 colors");
    }

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
      p = &(pLogPal->palPalEntry[i+nEntries]);
      p->peRed = GetRValue(win_clr[i]);
      p->peGreen = GetGValue(win_clr[i]);
      p->peBlue = GetBValue(win_clr[i]);
      p->peFlags = PC_NOCOLLAPSE;
    }

    /* Free something */
    if (lppe) rnfree(lppe, lppeSize);

    /* Create a new palette, or fail */
    hNewPal = CreatePalette(pLogPal);
    if (!hNewPal) quit("Cannot create palette");

    /* Free the palette */
    rnfree(pLogPal, pLogPalSize);


    if (screen.w)
    {
      hdc = GetDC(screen.w);
      SelectPalette(hdc, hNewPal, 0);
      i = RealizePalette(hdc);
      ReleaseDC(screen.w, hdc);
      if (i == 0) quit("Cannot realize palette");
    }
    
#ifdef GRAPHIC_MIRROR
    if (mirror.w)
    {
      hdc = GetDC(mirror.w);
      SelectPalette(hdc, hNewPal, 0);
      ReleaseDC(mirror.w, hdc);
    }
#endif

#ifdef GRAPHIC_RECALL
    if (recall.w)
    {
      hdc = GetDC(recall.w);
      SelectPalette(hdc, hNewPal, 0);
      ReleaseDC(recall.w, hdc);
    }
#endif

#ifdef GRAPHIC_CHOICE
    if (choice.w)
    {
      hdc = GetDC(choice.w);
      SelectPalette(hdc, hNewPal, 0);
      ReleaseDC(choice.w, hdc);
    }
#endif

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

  /* Main window */
  if (!(td->type_1))
  {
    /* get old window center */
    GetWindowRect(td->w, &rc);
    pt.x = (rc.left + rc.right) / 2;
    pt.y = (rc.top  + rc.bottom) / 2;

    /* determine left top corner, adjust it */
    pt.x -= td->size_wid / 2;
    pt.y -= td->size_hgt/ 2;
    if (pt.x < 0) pt.x = 0;
    if (pt.y < 0) pt.y = 0;

    SetWindowPos(td->w, 0, pt.x, pt.y,
                 td->size_wid, td->size_hgt,
                 SWP_NOZORDER);
  }

  /* Sub-windows */
  else
  {
    SetWindowPos(td->w, 0, 0, 0,
                 td->size_wid, td->size_hgt,
                 SWP_NOMOVE | SWP_NOZORDER);
  }

  /* Redraw later */
  InvalidateRect(td->w, NULL, TRUE);
}



/*
 * Mega-Hack -- force the use of a new font for a window
 *
 * XXX XXX XXX This function (or children) may have some bugs.
 */
static void term_force_font(term_data *td, cptr name)
{
  int i;

  int wid, hgt;

  cptr s;

  char base[16];

  char base_font[16];
  char base_graf[16];

  char buf[1024];


  /*** Analyze given name ***/

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
      if (base[i] == 'X') hgt = atoi(s+1);
  }

  /* Terminate */
  base[i] = '\0';

  /* Allow unknown size (?) */
  if (!wid || !hgt) wid = hgt = 0;

  /* Build base_font */
  strcpy(base_font, base);
  strcat(base_font, ".FON");

  /* Build base_graf */
  strcpy(base_graf, base);
  strcat(base_graf, ".BMP");


  /*** Load the new font ***/

  /* Forget the old font (if needed) */
  if (td->font_id) DeleteObject(td->font_id);

  /* XXX XXX XXX Remove old font resource when no longer needed */

  /* Forget the old font name (if needed) */
  if (td->font_file) string_free(td->font_file);

  /* Save new font name */
  td->font_file = string_make(base_font);

  /* Access the font file */
  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "font\\");
  strcat(buf, base_font);

  /* Validate file */
  validate_file(buf);

  /* Load the new font or quit */
  if (!AddFontResource(buf))
  {
    quit_fmt("Font file corrupted:\n%s", buf);
  }

  /* Create the font */
  td->font_id = CreateFont(wid, hgt, 0, 0, FW_DONTCARE, 0, 0, 0,
                           ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                           FIXED_PITCH | FF_DONTCARE, base_font);


  /*** Load the new bitmap ***/

  /* XXX XXX XXX Using graphics, screen, ready */
  if (use_graphics && (td == &screen) && screen.w)
  {
    /* Access the graf file */
    strcpy(buf, ANGBAND_DIR_XTRA);
    strcat(buf, "graf\\");
    strcat(buf, base_graf);

    /* Validate file */
    validate_file(buf);

    /* Free the old information */
    if (infGraph.hDIB) GlobalFree(infGraph.hDIB);
    if (infGraph.hPalette) DeleteObject(infGraph.hPalette);
    if (infGraph.hBitmap) DeleteObject(infGraph.hBitmap);

    /* Forget old graf name (if needed) */
    if (td->graf_file) string_free(td->graf_file);

    /* Save new graf name */
    td->graf_file = string_make(base_graf);

    /* Load the bitmap or quit */
    if (!ReadDIB(screen.w, buf, &infGraph))
    {
      quit_fmt("Bitmap corrupted:\n%s", buf);
    }

    /* Save the new sizes */
    infGraph.CellWidth = wid;
    infGraph.CellHeight = hgt;
  }


  /*** Other stuff ***/

  /* Activate a palette */
  new_palette();

  /* Analyze the font */
  term_getsize(td);

  /* Resize the window */
  term_window_resize(td);
}


/*
 * Allow the user to change the font (and graf) for this window.
 *
 * XXX XXX XXX This is only called for non-graphic windows
 */
static void term_change_font(term_data *td)
{
  OPENFILENAME ofn;

  char tmp[128];

  char buf[1024];

  /* Access the "font" folder */
  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "font");

  /* Assume nothing */
  tmp[0] = '\0';

  /* Extract a default if possible */
  if (td->font_file) strcpy(tmp, td->font_file);

  /* Ask for a choice */
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = screen.w;
  ofn.lpstrFilter = "Font Files (*.fon)\0*.fon\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = 128;
  ofn.lpstrInitialDir = buf;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.lpstrDefExt = "fon";

  /* Force choice if legal */
  if (GetOpenFileName(&ofn)) term_force_font(td, tmp);
}


/*
 * Allow the user to change the graf (and font) for a window
 *
 * XXX XXX XXX This is only called for graphic windows
 */
static void term_change_bitmap(term_data *td)
{
  OPENFILENAME ofn;
  
  char tmp[128];

  char buf[1024];

  /* Access the "graf" folder */
  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "graf");

  /* Assume nothing */
  tmp[0] = '\0';

  /* Extract a default if possible */
  if (td->graf_file) strcpy(tmp, td->graf_file);

  /* Ask for a choice */
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = screen.w;
  ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = 128;
  ofn.lpstrInitialDir = buf;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.lpstrDefExt = "bmp";

  /* Force choice if legal */
  if (GetOpenFileName(&ofn)) term_force_font(td, tmp);
}



/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
    /* Activate the term */
    Term_activate(&td->t);

    /* Main screen XXX XXX XXX */
    if (!(td->type_1) && !screen_resizable)
    {
      HPEN hpenOld, hpenLtGray;
      HDC  hdc = GetDC(td->w);

      /* Frame the window in white */
      hpenLtGray = CreatePen(PS_SOLID, 1, win_clr[TERM_WHITE]);
      hpenOld = SelectObject(hdc, hpenLtGray);

      MoveTo(hdc, 0, 0);
      LineTo(hdc, 0, td->client_hgt-1);
      LineTo(hdc, td->client_wid-1, td->client_hgt-1);
      LineTo(hdc, td->client_wid-1, 0);
      LineTo(hdc, 0, 0);

      SelectObject(hdc, hpenOld);
      DeleteObject(hpenLtGray);
      ReleaseDC(td->w, hdc);
    }

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
 * Interact with the User (unused)
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
  static old_use_graphics = FALSE;


  /* XXX XXX XXX Check "color_table[]" */

  
  /* XXX XXX XXX Check "use_graphics" */
  if (use_graphics && !old_use_graphics) {

    /* Hack -- set the player picture */
    r_info[0].l_attr = 0x87;
    r_info[0].l_char = 0x80 | (10 * p_ptr->pclass + p_ptr->prace);

    /* Remember */
    old_use_graphics = TRUE;
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
 * Hack -- clear the screen XXX XXX XXX
 */
static errr Term_xtra_win_clear(void)
{
  term_data *td = (term_data*)(Term->data);

  HDC  hdc;
  RECT rc;

  int w = min(80, td->vis_cols);
  int h = min(24, td->vis_rows);

  /* Rectangle to erase in client coords */
  rc.left   = td->size_ow1;
  rc.right  = rc.left + w * td->font_wid;
  rc.top    = td->size_oh1;
  rc.bottom = rc.top + h * td->font_hgt;

  hdc = GetDC(td->w);
  SetBkColor(hdc, RGB(0,0,0));
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
 * Hack -- make a sound XXX XXX XXX
 */
static errr Term_xtra_win_sound(int v)
{
  /* Unknown sound */
  if ((v < 0) || (v >= SOUND_MAX)) return (1);

  /* Unknown sound */
  if (!sound_file[v]) return (1);

#ifdef WIN32

  /* Play the sound */
  return (PlaySound(sound_file[v], 0, SND_FILENAME | SND_ASYNC));

#else /* WIN32 */

  /* Play the sound */
  return (sndPlaySound(sound_file[v], SND_ASYNC));

#endif /* WIN32 */

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
  }

  /* Oops */
  return 1;
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

    int w = n;
    int h = 1;

    /* Handle scroll bars */
    y -= td->scroll_vpos;
    if (y > td->vis_rows) return (0);
    if (y < 0) { h += y; y = 0; }
    h = min(h, td->vis_rows - y);

    /* Handle scroll bars */
    x -= td->scroll_hpos;
    if (x > td->vis_cols) return (0);
    if (x < 0) { w += x; x = 0; }
    w = min(w, td->vis_cols - x);

    /* Rectangle to erase in client coords */
    rc.left   = x * td->font_wid + td->size_ow1;
    rc.right  = rc.left + w * td->font_wid;
    rc.top    = y * td->font_hgt + td->size_oh1;
    rc.bottom = rc.top + h * td->font_hgt;

    hdc = GetDC(td->w);
    SetBkColor(hdc, RGB(0,0,0));
    SelectObject(hdc, td->font_id);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    ReleaseDC(td->w, hdc);

    /* Success */
    return 0;
}


/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 *
 * XXX XXX XXX Scrolling the main window, why would you do that?
 */
static errr Term_curs_win(int x, int y)
{
    term_data *td = (term_data*)(Term->data);

    RECT   rc;
    HDC    hdc;

    /* Handle scroll bars */
    y -= td->scroll_vpos;
    if ((y < 0) || (y > td->vis_rows)) return (0);

    /* Handle scroll bars */
    x -= td->scroll_hpos;
    if ((x < 0) || (x > td->vis_cols)) return (0);

    /* Frame the grid */
    rc.left   = x * td->font_wid + td->size_ow1;
    rc.right  = rc.left + td->font_wid;
    rc.top    = y * td->font_hgt + td->size_oh1;
    rc.bottom = rc.top + td->font_hgt;

    /* Cursor is done as a yellow "box" */
    hdc = GetDC(screen.w);
    FrameRect(hdc, &rc, hbrYellow);
    ReleaseDC(screen.w, hdc);

    /* Success */
    return 0;
}



/*
 * Low level graphics.  Assumes valid input.
 * Draw a "special" attr/char at the given location.
 */
static errr Term_pict_win(int x, int y, byte a, char c)
{
    term_data *td = (term_data*)(Term->data);

    RECT rc;
    HDC  hdc;
    HDC hdcMem, hdcSrc;
    HBITMAP hbmSrcOld, hbmMemOld, hbmMem;
    int  scr_x, scr_y;
    int  w, h, row, col;

    /* Handle scroll bars */
    y -= td->scroll_vpos;
    if ((y < 0) || (y >= td->vis_rows)) return (0);

    /* Handle scroll bars */
    x -= td->scroll_hpos;
    if ((x < 0) || (x >= td->vis_cols)) return (0);

    /* Location */
    scr_x = x * td->font_wid + td->size_ow1;
    scr_y = y * td->font_hgt + td->size_oh1;

    /* Size */
    w = td->font_wid;
    h = td->font_hgt;

    /* Info */
    hdc = GetDC(td->w);
    hdcSrc = CreateCompatibleDC(hdc);
    hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

    /* Extract picture info */
    row = (a & 0x7F);
    col = (c & 0x7F);

    /* Draw the picture */
    BitBlt(hdc, scr_x, scr_y, w, h, hdcSrc,
           col * infGraph.CellWidth,
           row * infGraph.CellHeight,
           SRCCOPY);

    /* Release */
    SelectObject(hdcSrc, hbmSrcOld);
    DeleteDC(hdcSrc);
    ReleaseDC(td->w, hdc);

    /* Success */
    return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * XXX XXX XXX We use the "Term_pict_win()" function for "graphic" data,
 * which are encoded by setting the "high-bits" of both the "attr" and
 * the "char" data.  We use the "attr" to represent the "row" of the main
 * bitmap, and the "char" to represent the "col" of the main bitmap.  We
 * draw the graphic information one character at a time, since normally
 * we will only get it one datum at a time, and we would have to do a
 * proper "buffering" of the data, flushed by the "TERM_XTRA_FROSH" action.
 *
 * XXX XXX XXX Note that this function assumes the font is monospaced.
 */
static errr Term_text_win(int x, int y, int n, byte a, const char *s)
{
    term_data *td = (term_data*)(Term->data);
    RECT rc;
    HDC  hdc;
    int  w, h;
    int  scr_x, scr_y;


    /* Hack -- Handle "graphics" */
    if ((a & 0x80) && use_graphics) {

        int i;

        /* Dump the "string" */
        for (i = 0; i < n; i++) {

            /* Dump each "graphic" request */
            (void)Term_pict_win(x+i, y, a, s[i]);
        }

        /* Success */
        return (0);
    }

  
    /* Handle scroll bars */
    y -= td->scroll_vpos;
    if ((y < 0) || (y >= (int)td->vis_rows)) return 0;

    /* Handle scroll bars */
    x -= td->scroll_hpos;
    if ((x >= (int)td->vis_cols)) return (0);
    if (x < 0)
    {
        if (n <= -x) return 0;
        s += -x;
        n -= -x;
        x = 0;
    }
    n = min(n, td->vis_cols - x);
    if (n <= 0) return 0;

    /* Catch "illegal" attributes */
    if ((a == 0) || (a >= 16)) a = TERM_RED;

    /* Location */
    rc.left   = x * td->font_wid + td->size_ow1;
    rc.right  = rc.left + n * td->font_wid;
    rc.top    = y * td->font_hgt + td->size_oh1;
    rc.bottom = rc.top + td->font_hgt;

    /* Draw the string */
    hdc = GetDC(td->w);
    SetBkColor(hdc, RGB(0,0,0));
    SetTextColor(hdc, colors16 ? PALETTEINDEX(win_pal[a]) : win_clr[a]);
    SelectObject(hdc, td->font_id);
    ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
               s, n, NULL);
    ReleaseDC(td->w, hdc);

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
    term_init(t, 80, 24, td->keys);

    /* Prepare the template values */
    t->soft_cursor = TRUE;
    t->scan_events = TRUE;

#if 0
    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_win;
    t->nuke_hook = Term_nuke_win;
#endif

    /* Prepare the template hooks */
    t->user_hook = Term_user_win;
    t->xtra_hook = Term_xtra_win;
    t->wipe_hook = Term_wipe_win;
    t->curs_hook = Term_curs_win;
    t->pict_hook = Term_pict_win;
    t->text_hook = Term_text_win;

    /* Remember where we came from */
    t->data = (vptr)(td);
}


/*
 * Read the preference file, Create the windows.
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette. (?)
 */
static void init_windows(void)
{
  term_data *td;

  MSG        msg;


  /* Main window */
  td = &screen;
  WIPE(td, term_data);
  td->s = "Angband";
  td->type_1 = FALSE;
  td->keys = 1024;
  td->rows = 24;
  td->cols = 80;
  td->visible = TRUE;
  td->resizing = FALSE;
  td->size_ow1 = 2;
  td->size_ow2 = 2;
  td->size_oh1 = 2;
  td->size_oh2 = 2;

#ifdef GRAPHIC_MIRROR
  /* Mirror window */
  td = &mirror;
  WIPE(td, term_data);
  td->s = "Mirror";
  td->type_1 = TRUE;
  td->keys = 16;
  td->rows = 24;
  td->cols = 80;
  td->visible = TRUE;
  td->resizing = FALSE;
  td->size_ow1 = 1;
  td->size_ow2 = 1;
  td->size_oh1 = 1;
  td->size_oh2 = 1;
#endif

#ifdef GRAPHIC_RECALL
  /* Recall window */
  td = &recall;
  WIPE(td, term_data);
  td->s = "Recall";
  td->type_1 = TRUE;
  td->keys = 16;
  td->rows = 12;
  td->cols = 80;
  td->visible = TRUE;
  td->resizing = FALSE;
  td->size_ow1 = 1;
  td->size_ow2 = 1;
  td->size_oh1 = 1;
  td->size_oh2 = 1;
#endif

#ifdef GRAPHIC_CHOICE
  /* Choice window */
  td = &choice;
  WIPE(td, term_data);
  td->s = "Choice";
  td->type_1 = TRUE;
  td->keys = 16;
  td->rows = 24;
  td->cols = 80;
  td->visible = TRUE;
  td->resizing = FALSE;
  td->size_ow1 = 1;
  td->size_ow2 = 1;
  td->size_oh1 = 1;
  td->size_oh2 = 1;
#endif

  /* Load .INI preferences */
  load_prefs();


  /* Need these before term_getsize gets called */
  screen.dwStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | 
                    WS_MINIMIZEBOX | WS_VISIBLE);

  /* Resizable screen */
  if (screen_resizable)
  {
    screen.dwStyle |= (WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL | 
                       WS_MAXIMIZEBOX);
  }

  screen.dwExStyle = 0;

  term_force_font(&screen, screen.want_file);
  string_free(screen.want_file);


#ifdef GRAPHIC_MIRROR
  /* Mirror window */
  td_ptr = &mirror;
#ifdef USE_ITSYBITSY
  td_ptr->dwStyle = (IBS_VERTCAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
                     WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  td_ptr->dwExStyle = 0;
#else
  td_ptr->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  td_ptr->dwExStyle = (WS_EX_TOOLWINDOW);
#endif
  term_force_font(&mirror, mirror.want_file);
  string_free(mirror.want_file);
#endif


#ifdef GRAPHIC_RECALL
  /* Recall window */
  td_ptr = &recall;
#ifdef USE_ITSYBITSY
  td_ptr->dwStyle = (IBS_VERTCAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
                     WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  td_ptr->dwExStyle = 0;
#else
  td_ptr->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  td_ptr->dwExStyle = (WS_EX_TOOLWINDOW);
#endif
  term_force_font(&recall, recall.want_file);
  string_free(recall.want_file);
#endif


#ifdef GRAPHIC_CHOICE
  /* Choice window */
  td_ptr = &choice;
#ifdef USE_ITSYBITSY
  td_ptr->dwStyle = (IBS_VERTCAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
                     WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  td_ptr->dwExStyle = 0;
#else
  td_ptr->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  td_ptr->dwExStyle = (WS_EX_TOOLWINDOW);
#endif
  term_force_font(&choice, choice.want_file);
  string_free(choice.want_file);
#endif


  /* Create a "brush" for drawing the "cursor" */
  hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);


  /* Screen window */
  td_ptr = &screen;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AppName,
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (!td_ptr->w) quit("Failed to create Angband window");
  term_data_link(&screen);
  term_screen = &screen.t;


#ifdef GRAPHIC_MIRROR
  /* Mirror window */
  td_ptr = &mirror;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AngList,
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (!td_ptr->w) quit("Failed to create mirror window");
#ifdef USE_ITSYBITSY
  if (td_ptr->cap_size) ibSetCaptionSize(td_ptr->w, td_ptr->cap_size);
#endif
  if (td_ptr->visible) ShowWindow(td_ptr->w, SW_SHOW);
  term_data_link(&mirror);
  term_mirror = &mirror.t;
#endif


#ifdef GRAPHIC_RECALL
  /* Recall window */
  td_ptr = &recall;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AngList,
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (!td_ptr->w) quit("Failed to create recall window");
#ifdef USE_ITSYBITSY
  if (td_ptr->cap_size) ibSetCaptionSize(td_ptr->w, td_ptr->cap_size);
#endif
  if (td_ptr->visible) ShowWindow(td_ptr->w, SW_SHOW);
  term_data_link(&recall);
  term_recall = &recall.t;
#endif


#ifdef GRAPHIC_CHOICE
  /* Choice window */
  td_ptr = &choice;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AngList,
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (!td_ptr->w) quit("Failed to create choice window");
#ifdef USE_ITSYBITSY
  if (td_ptr->cap_size) ibSetCaptionSize(td_ptr->w, td_ptr->cap_size);
#endif
  if (td_ptr->visible) ShowWindow(td_ptr->w, SW_SHOW);
  term_data_link(&choice);
  term_choice = &choice.t;
#endif


  /* Screen window */
  td_ptr = &screen;

  /* Activate the screen window */
  SetActiveWindow(screen.w);

  /* Bring screen.w back to top */
  SetWindowPos(screen.w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


  /* Process pending messages */
  (void)Term_xtra_win_flush();
}


/*
 * Delay for "x" milliseconds
 */
void delay(int x)
{

#ifdef WIN32

    /* Sleep */
    Sleep(x);

#else /* WIN32 */

    DWORD t;
    MSG   msg;

    /* Final count */
    t = GetTickCount() + x;

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

}


/*
 * Hack -- disables new and open from file menu
 */
static void disable_start(void)
{
  HMENU hm = GetMenu(screen.w);

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
  HMENU hm = GetMenu(screen.w);

  /* Hack -- extract the "can I save" flag */
  save_enabled = character_generated;

  if (save_enabled)
  {
    EnableMenuItem(hm, IDM_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(hm, IDM_FILE_EXIT, MF_BYCOMMAND | MF_ENABLED);
  }
  else
  {
    EnableMenuItem(hm, IDM_FILE_SAVE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(hm, IDM_FILE_EXIT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  }

#ifdef GRAPHIC_MIRROR
  /* Options/Font/Mirror window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_MIRROR, MF_BYCOMMAND |
      (mirror.visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
#endif

#ifdef GRAPHIC_RECALL
  /* Options/Font/Recall window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_RECALL, MF_BYCOMMAND |
      (recall.visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
#endif

#ifdef GRAPHIC_CHOICE
  /* Options/Font/Choice window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_CHOICE, MF_BYCOMMAND |
      (choice.visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
#endif

#ifdef GRAPHIC_MIRROR
  /* Item "Mirror Window" */
  CheckMenuItem(hm, IDM_OPTIONS_MIRROR, MF_BYCOMMAND |
      (mirror.visible ? MF_CHECKED : MF_UNCHECKED));
#endif

#ifdef GRAPHIC_RECALL
  /* Item "Recall Window" */
  CheckMenuItem(hm, IDM_OPTIONS_RECALL, MF_BYCOMMAND |
      (recall.visible ? MF_CHECKED : MF_UNCHECKED));
#endif

#ifdef GRAPHIC_CHOICE
  /* Item "Choice Window" */
  CheckMenuItem(hm, IDM_OPTIONS_CHOICE, MF_BYCOMMAND |
      (choice.visible ? MF_CHECKED : MF_UNCHECKED));
#endif

  /* Item "Main window resizable" */
  CheckMenuItem(hm, IDM_OPTIONS_RESIZABLE, MF_BYCOMMAND |
      (screen_resizable ? MF_CHECKED : MF_UNCHECKED));

  /* Item "Graphics" */
  CheckMenuItem(hm, IDM_OPTIONS_GRAPHICS, MF_BYCOMMAND |
      (use_graphics ? MF_CHECKED : MF_UNCHECKED));

  /* Item "Sound" */
  CheckMenuItem(hm, IDM_OPTIONS_SOUND, MF_BYCOMMAND |
      (use_sound ? MF_CHECKED : MF_UNCHECKED));

#ifndef ALLOW_SCRSAVER
  /* Item "Run as Screensaver" */
  EnableMenuItem(hm, IDM_OPTIONS_SAVER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
#endif

}


/*
 * XXX XXX XXX check for double clicked savefile (?)
 */
static void check_for_save_file(LPSTR cmd_line)
{
  char *p;

  /* isolate first argument in command line */
  p = strchr(cmd_line, ' ');
  if (p) *p = '\0';

  if (strlen(cmd_line) == 0) return;

  sprintf(savefile, "%s\\%s", ANGBAND_DIR_SAVE, cmd_line);
  validate_file(savefile);

  game_in_progress = TRUE;
  disable_start();

  /* Hook into "play_game()" */
  play_game(FALSE);
}


/*
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
  OPENFILENAME ofn;

  /* Analyze */
  switch (wCmd)
  {
    /* New game */
    case IDM_FILE_NEW:
    {
      if (!initialized)
      {
        MessageBox(screen.w, "You cannot do that yet...",
           "Warning", MB_ICONEXCLAMATION | MB_OK);
      }
      else if (game_in_progress)
      {
        MessageBox(screen.w,
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
        MessageBox(screen.w, "You cannot do that yet...",
           "Warning", MB_ICONEXCLAMATION | MB_OK);
      }
      else if (game_in_progress)
      {
        MessageBox(screen.w,
           "You can't open a new game while you're still playing!",
           "Warning", MB_ICONEXCLAMATION | MB_OK);
      }
      else
      {
        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = screen.w;
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
        MessageBox(screen.w, "No game in progress.",
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
      if (game_in_progress && save_enabled)
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
      save_enabled = character_generated;

      if (game_in_progress && save_enabled)
      {
        if (IDCANCEL == MessageBox(screen.w,
           "Your character will be not saved!",
           "Warning", MB_ICONEXCLAMATION | MB_OKCANCEL)) break;
      }
      quit(NULL);
      break;
    }

    /* Font XXX XXX XXX */
    case IDM_OPTIONS_FONT_ANGBAND:
    {
      if (use_graphics)
      {
        term_change_bitmap(&screen);
      }
      else
      {
        term_change_font(&screen);
      }
      break;
    }

#ifdef GRAPHIC_MIRROR
    /* XXX XXX XXX */
    case IDM_OPTIONS_FONT_MIRROR:
    {
      term_change_font(&mirror);
      break;
    }
#endif

#ifdef GRAPHIC_RECALL
    /* XXX XXX XXX */
    case IDM_OPTIONS_FONT_RECALL:
    {
      term_change_font(&recall);
      break;
    }
#endif

#ifdef GRAPHIC_CHOICE
    /* XXX XXX XXX */
    case IDM_OPTIONS_FONT_CHOICE:
    {
      term_change_font(&choice);
      break;
    }
#endif

#ifdef GRAPHIC_MIRROR
    /* Mirror visibility */
    case IDM_OPTIONS_MIRROR:
    {
      if (!mirror.visible)
      {
        mirror.visible = TRUE;
        ShowWindow(mirror.w, SW_SHOW);
        term_data_redraw(&mirror);
      }
      else
      {
        mirror.visible = FALSE;
        ShowWindow(mirror.w, SW_HIDE);
      }
      break;
    }
#endif

#ifdef GRAPHIC_RECALL
    /* Recall visibility */
    case IDM_OPTIONS_RECALL:
    {
      if (!recall.visible)
      {
        recall.visible = TRUE;
        ShowWindow(recall.w, SW_SHOW);
        term_data_redraw(&recall);
      }
      else
      {
        recall.visible = FALSE;
        ShowWindow(recall.w, SW_HIDE);
      }
      break;
    }
#endif

#ifdef GRAPHIC_CHOICE
    /* Choice visibility */
    case IDM_OPTIONS_CHOICE:
    {
      if (!choice.visible)
      {
        choice.visible = TRUE;
        ShowWindow(choice.w, SW_SHOW);
        term_data_redraw(&choice);
      }
      else
      {
        choice.visible = FALSE;
        ShowWindow(choice.w, SW_HIDE);
      }
      break;
    }
#endif

    /* Hack -- toggle "resizable" of main screen */
    case IDM_OPTIONS_RESIZABLE:
    {
      DWORD dw;

      /* Check current setting */
      dw = GetWindowLong(screen.w, GWL_STYLE);

      /* Make it resizable */
      if (!screen_resizable)
      {
        screen_resizable = TRUE;
        screen.dwStyle = dw | (WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL | WS_MAXIMIZEBOX);
        SetWindowLong(screen.w, GWL_STYLE, screen.dwStyle);
      }

      /* Make it non-resizable */
      else
      {
        screen_resizable = FALSE;
        screen.dwStyle = dw & ~(WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL | WS_MAXIMIZEBOX);
        SetWindowLong(screen.w, GWL_STYLE, screen.dwStyle);
        screen.vis_rows = screen.rows;
        screen.vis_cols = screen.cols;
        screen.scroll_vpos = 0;
        screen.scroll_hpos = 0;
      }

      term_getsize(&screen);
      term_window_resize(&screen);

      break;
    }

    case IDM_OPTIONS_GRAPHICS:
    {
      char buf[1024];

      /* Reset the visuals */
      reset_visuals();

      /* Use graphics */
      if (!use_graphics)
      {
        /* Turn on graphics */
        use_graphics = TRUE;
        
        /* Access the "graphic" mappings */
        sprintf(buf, "graf-%s.prf", ANGBAND_SYS);

        /* Load the file */
        process_pref_file(buf);

        /* XXX XXX XXX Force a "usable" font */
        term_force_font(&screen, "10x20.fon");

        /* XXX XXX XXX Force the current font */
        /* term_force_font(&screen, screen.font_file); */
      }

      /* Use text */
      else
      {
        /* Turn off graphics */
        use_graphics = FALSE;

        /* Access the "textual" mappings */
        sprintf(buf, "font-%s.prf", ANGBAND_SYS);

        /* Load the file */
        process_pref_file(buf);
      }

      /* React to changes */
      Term_xtra_win_react();

      /* Hack -- Force redraw */
      Term_key_push(KTRL('R'));

      break;
    }

    case IDM_OPTIONS_SOUND:
    {
      use_sound = !use_sound;
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
        MessageBox(screen.w, "Failed to create saver window", NULL, MB_OK);
      }
      break;
    }
#endif

    case IDM_HELP_GENERAL:
    {
      char buf[256];
      char tmp[256];
      strcpy(tmp, ANGBAND_DIR_XTRA);
      strcat(tmp, "help\\");
      strcat(tmp, "angband.hlp");
      sprintf(buf, "winhelp.exe %s", tmp);
      WinExec(buf, SW_NORMAL);
      break;
    }

    case IDM_HELP_SPOILERS:
    {
      char buf[256];
      char tmp[256];
      strcpy(tmp, ANGBAND_DIR_XTRA);
      strcat(tmp, "help\\");
      strcat(tmp, "spoilers.hlp");
      sprintf(buf, "winhelp.exe %s", tmp);
      WinExec(buf, SW_NORMAL);
      break;
    }
  }
}


/*
 * Hack -- process a scrollbar
 *
 * note that SB_TOP == SB_LEFT, SB_BOTTOM == SB_RIGHT etc.
 */
static int process_scrollbar(HWND hWnd, WPARAM wParam, LPARAM lParam, int fnBar)
{
  term_data  *td;
  uint       *scroll_pos;
  int         rows_cols, vis_rows_cols;

  td = (term_data *)GetWindowLong(hWnd, 0);

  switch (fnBar)
  {
    case SB_VERT:
    {
      scroll_pos = &(td->scroll_vpos);
      rows_cols = td->rows;
      vis_rows_cols = td->vis_rows;
      break;
    }
    case SB_HORZ:
    {
      scroll_pos = &(td->scroll_hpos);
      rows_cols = td->cols;
      vis_rows_cols = td->vis_cols;
      break;
    }
    default:
    {
      return 1;
    }
  }

  switch (wParam)
  {
    case SB_TOP:
    {
      *scroll_pos = 0;
      break;
    }

    case SB_BOTTOM:
    {
      *scroll_pos = rows_cols - vis_rows_cols;
      break;
    }

    case SB_LINEUP:
    {
      if (*scroll_pos > 0) (*scroll_pos)--;
      break;
    }

    case SB_LINEDOWN:
    {
      if (*scroll_pos < rows_cols - vis_rows_cols) (*scroll_pos)++;
      break;
    }

    case SB_PAGEUP:
    {
      *scroll_pos = max((int)(*scroll_pos) - vis_rows_cols, 0);
      break;
    }

    case SB_PAGEDOWN:
    {
      *scroll_pos = min((int)(*scroll_pos) + vis_rows_cols, rows_cols - vis_rows_cols);
      break;
    }

    case SB_THUMBPOSITION:
    {
      *scroll_pos = max(min(LOWORD(lParam), rows_cols - vis_rows_cols), 0);
      break;
    }

    default:
    {
      return 1;
    }
  }

  SetScrollPos(hWnd, fnBar, *scroll_pos, TRUE);

  InvalidateRect(hWnd, NULL, TRUE);

  return 0;
}


LRESULT FAR PASCAL _export AngbandWndProc(HWND hWnd, UINT uMsg,
  WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT     ps;
  HDC             hdc;
  term_data      *td;
  MINMAXINFO FAR *lpmmi;
  RECT            rc;
  int             i;
  HPALETTE        hOldPal;

  switch (uMsg)
  {
    /* XXX XXX XXX */
    case WM_NCCREATE:
    {
      if (td_ptr == &screen)
      {
        SetWindowLong(hWnd, 0, (LONG)(td_ptr));
      }
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    /* XXX XXX XXX */
    case WM_CREATE:
    {
      if (screen_resizable)
      {
        td = (term_data *)GetWindowLong(hWnd, 0);
        SetScrollRange(hWnd, SB_VERT, 0, max(td->rows - td->vis_rows, 1), FALSE);
        SetScrollRange(hWnd, SB_HORZ, 0, max(td->cols - td->vis_cols, 1), FALSE);
        return 0;
      }
      else
      {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
      }
    }

    case WM_GETMINMAXINFO:
    {
      lpmmi = (MINMAXINFO FAR *)lParam;
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;  /* this message was sent before WM_NCCREATE */

      /* minimum window size is 15x3, otherwise have problems with */
      /* menu line wrapping to two lines */
      rc.left = rc.top = 0;
      rc.right = rc.left + 15 * td->font_wid + td->size_ow1 + td->size_ow2;
      rc.bottom = rc.top + 3 * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;
      if (screen_resizable)
      {
        rc.right  += GetSystemMetrics(SM_CXVSCROLL) - 1;
        rc.bottom += GetSystemMetrics(SM_CYHSCROLL) - 1;
      }
      AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);
      lpmmi->ptMinTrackSize.x = rc.right - rc.left;
      lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

      /* maximum window size is td->cols x td->rows */
      rc.left = rc.top = 0;
      rc.right = rc.left + td->cols * td->font_wid + td->size_ow1 + td->size_ow2;
      rc.bottom = rc.top + td->rows * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;
      if (screen_resizable)
      {
        rc.right  += GetSystemMetrics(SM_CXVSCROLL) - 1;
        rc.bottom += GetSystemMetrics(SM_CYHSCROLL) - 1;
      }
      AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);
      lpmmi->ptMaxSize.x      = rc.right - rc.left;
      lpmmi->ptMaxSize.y      = rc.bottom - rc.top;
      lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
      lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

      return 0;
    }

    case WM_PAINT:
    {
      td = (term_data *)GetWindowLong(hWnd, 0);
      BeginPaint(hWnd, &ps);
      if (td) term_data_redraw(td);
      EndPaint(hWnd, &ps);
      ValidateRect(hWnd, NULL);   /* why needed ?? */
      return 0;
    }

    case WM_VSCROLL:
    {
      return process_scrollbar(hWnd, wParam, lParam, SB_VERT);
    }

    case WM_HSCROLL:
    {
      return process_scrollbar(hWnd, wParam, lParam, SB_HORZ);
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

      return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;    /* this message was sent before WM_NCCREATE */
      if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
      if (td->resizing) return 1; /* was sent from WM_SIZE */

      switch (wParam)
      {
        case SIZE_MINIMIZED:
#ifdef GRAPHIC_MIRROR
          if (mirror.visible) ShowWindow(mirror.w, SW_HIDE);
#endif
#ifdef GRAPHIC_RECALL
          if (recall.visible) ShowWindow(recall.w, SW_HIDE);
#endif
#ifdef GRAPHIC_CHOICE
          if (choice.visible) ShowWindow(choice.w, SW_HIDE);
#endif
          return 0;

        case SIZE_MAXIMIZED:
          td->resizing = TRUE;
          td->vis_cols = td->cols;
          td->vis_rows = td->rows;
          /* fall through!!! */

        case SIZE_RESTORED:
          if (screen_resizable)
          {
            td->resizing = TRUE;
            td->vis_cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
            td->vis_cols = min(td->vis_cols, td->cols);
            td->vis_rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;
            td->vis_rows = min(td->vis_rows, td->rows);
            td->scroll_vpos = 0;
            td->scroll_hpos = 0;
            SetScrollRange(hWnd, SB_VERT, 0, max(td->rows - td->vis_rows, 1), FALSE);
            SetScrollRange(hWnd, SB_HORZ, 0, max(td->cols - td->vis_cols, 1), FALSE);
            term_getsize(td);
            MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);
          }
#ifdef GRAPHIC_MIRROR
          if (mirror.visible) ShowWindow(mirror.w, SW_SHOWNOACTIVATE);
#endif
#ifdef GRAPHIC_RECALL
          if (recall.visible) ShowWindow(recall.w, SW_SHOWNOACTIVATE);
#endif
#ifdef GRAPHIC_CHOICE
          if (choice.visible) ShowWindow(choice.w, SW_SHOWNOACTIVATE);
#endif
          td->resizing = FALSE;
          return 0;

        default:
           return DefWindowProc(hWnd, uMsg, wParam, lParam);
      }
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
#ifdef GRAPHIC_MIRROR
        SetWindowPos(mirror.w, hWnd, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
#endif
#ifdef GRAPHIC_RECALL
        SetWindowPos(recall.w, hWnd, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
#endif
#ifdef GRAPHIC_CHOICE
        SetWindowPos(choice.w, hWnd, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
#endif
        SetFocus(hWnd);
        return 0;
      }
      /* fall through */
    }

    default:
    {
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
  }
}



LRESULT FAR PASCAL _export AngbandListProc(HWND hWnd, UINT uMsg,
  WPARAM wParam, LPARAM lParam)
{
  term_data      *td;
  MINMAXINFO FAR *lpmmi;
  RECT            rc;
  PAINTSTRUCT     ps;
  HDC             hdc;
  int             i;

  switch (uMsg)
  {
    /* XXX XXX XXX */
    case WM_NCCREATE:
    {
      if (td_ptr != &screen)
      {
        SetWindowLong(hWnd, 0, (LONG)(td_ptr));
      }
#ifdef USE_ITSYBITSY
      return ibDefWindowProc(hWnd, uMsg, wParam, lParam);
#else
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
#endif
    }

    case WM_CREATE:
    {
      td = (term_data *)GetWindowLong(hWnd, 0);
      SetScrollRange(hWnd, SB_VERT, 0, td->rows - td->vis_rows, FALSE);
      return 0;
    }

    case WM_GETMINMAXINFO:
    {
      lpmmi = (MINMAXINFO FAR *)lParam;
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;  /* this message was sent before WM_NCCREATE */

      rc.left = rc.top = 0;
      rc.right = rc.left + 5 * td->font_wid + td->size_ow1 + td->size_ow2 +
                 GetSystemMetrics(SM_CXVSCROLL);
      rc.bottom = rc.top + 3 * td->font_hgt + td->size_oh1 + td->size_oh2;
#ifdef USE_ITSYBITSY
      rc.bottom += 1;
      ibAdjustWindowRect(&rc, td->dwStyle, FALSE, td->cap_size);
#else
      rc.right += 1;
      rc.bottom -= 1;
      AdjustWindowRectEx(&rc, td->dwStyle, FALSE, td->dwExStyle);
#endif
      lpmmi->ptMinTrackSize.x = rc.right - rc.left;
      lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

      /* maximum window size is td->cols x td->rows */
      rc.left = rc.top = 0;
      rc.right = rc.left + td->cols * td->font_wid + td->size_ow1 + td->size_ow2 +
                 GetSystemMetrics(SM_CXVSCROLL);
      rc.bottom = rc.top + td->rows * td->font_hgt + td->size_oh1 + td->size_oh2;
#ifdef USE_ITSYBITSY
      rc.bottom += 1;
      ibAdjustWindowRect(&rc, td->dwStyle, FALSE, td->cap_size);
#else
      rc.right += 1;
      rc.bottom -= 1;
      AdjustWindowRectEx(&rc, td->dwStyle, FALSE, td->dwExStyle);
#endif
      lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
      lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

      return 0;
    }

    case WM_SIZE:
    {
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;    /* this message was sent before WM_NCCREATE */
      if (!td->w) return 1; /* it was sent from inside CreateWindowEx */
      if (td->resizing) return 1; /* was sent from inside WM_SIZE */
      td->resizing = TRUE;
      td->vis_cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
      td->vis_rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;
      td->scroll_vpos = 0;
      td->scroll_hpos = 0;
      SetScrollRange(hWnd, SB_VERT, 0, td->rows - td->vis_rows, FALSE);
      term_getsize(td);
      MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);
      td->resizing = FALSE;
      return 0;
    }

    case WM_PAINT:
    {
      td = (term_data *)GetWindowLong(hWnd, 0);
      BeginPaint(hWnd, &ps);
      if (td) term_data_redraw(td);
      EndPaint(hWnd, &ps);
      return 0;
    }

    case WM_VSCROLL:
    {
      return process_scrollbar(hWnd, wParam, lParam, SB_VERT);
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

#ifdef USE_ITSYBITSY
    case WM_SYSCOMMAND:
    {
      switch (wParam)
      {
        case SC_MINIMIZE:
          td = (term_data *)GetWindowLong(hWnd, 0);
          td->cap_size = max(ibGetCaptionSize(hWnd) - 1, 0);
          ibSetCaptionSize(hWnd, td->cap_size);
          SendMessage(hWnd, WM_NCPAINT, 0, 0);
          return 0;
        case SC_MAXIMIZE:
          td = (term_data *)GetWindowLong(hWnd, 0);
          td->cap_size = min(ibGetCaptionSize(hWnd) + 1, 127);
          ibSetCaptionSize(hWnd, td->cap_size);
          SendMessage(hWnd, WM_NCPAINT, 0, 0);
          return 0;
        default:
          return ibDefWindowProc(hWnd, uMsg, wParam, lParam);
      }
    }
#endif

    case WM_NCLBUTTONDOWN:
    {
      if (wParam == HTSYSMENU)
      {
        td = (term_data *)GetWindowLong(hWnd, 0);

        /* Hide "type_1" windows */
        if (td->type_1)
        {
          if (td->visible)
          {
            td->visible = FALSE;
            ShowWindow(td->w, SW_HIDE);
          }
        }

        return 0;
      }

      /* fall through */
    }

    default:
    {
#ifdef USE_ITSYBITSY
      return ibDefWindowProc(hWnd, uMsg, wParam, lParam);
#else
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
#endif
    }
  }
  
  /* Oops */
  return (0);
}


#ifdef ALLOW_SCRSAVER

#define MOUSE_SENS 40

LRESULT FAR PASCAL _export AngbandSaverProc(HWND hWnd, UINT uMsg,
  WPARAM wParam, LPARAM lParam)
{
  static WORD xMouse = 0xFFFF;
  static WORD yMouse = 0xFFFF;
  int dx, dy;

  switch (uMsg)
  {
    /* XXX XXX XXX */
    case WM_NCCREATE:
    {
      if (td_ptr == &screen)
      {
        SetWindowLong(hWnd, 0, (LONG)(td_ptr));
      }
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    case WM_SETCURSOR:
    {
      SetCursor(NULL);
      return 0;
    }

#if 0
    case WM_ACTIVATE:
    {
      if (LOWORD(wParam) == WA_INACTIVE)
      {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
      }

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
      if (xMouse == 0xFFFF || yMouse == 0xFFFF)
      {
        xMouse = LOWORD(lParam);
        yMouse = HIWORD(lParam);
        return 0;
      }
      dx = LOWORD(lParam) - xMouse;
      if (dx < 0) dx = -dx;
      dy = HIWORD(lParam) - yMouse;
      if (dy < 0) dy = -dy;
      if (dx > MOUSE_SENS || dy > MOUSE_SENS)
      {
        SendMessage(hWnd, WM_CLOSE, 0, 0);
      }
      xMouse = LOWORD(lParam);
      yMouse = HIWORD(lParam);
      return 0;
    }

    case WM_CLOSE:
    {
      DestroyWindow(hWnd);
      return 0;
    }

    default:
    {
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
  }
}

#endif


/*** Temporary Hooks ***/


/*
 * Error message -- See "z-util.c"
 */
static void hack_plog(cptr str)
{
  /* XXX XXX XXX Give a warning */
  /* if (str) MessageBox(screen.w, str, "Warning", MB_OK); */
}


/*
 * Quit with error message -- See "z-util.c"
 */
static void hack_quit(cptr str)
{
  /* XXX XXX XXX Give a warning */
  /* if (str) MessageBox(screen.w, str, "Error", MB_OK | MB_ICONSTOP); */

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
  /* XXX XXX XXX Give a warning */
  /* if (str) MessageBox(screen.w, str, "Error", MB_OK | MB_ICONSTOP); */

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
  if (str) MessageBox(screen.w, str, "Warning", MB_OK);
}


/*
 * Quit with error message -- See "z-util.c"
 *
 * A lot of this function should be handled by actually calling
 * the "term_nuke()" function hook for each window.  XXX XXX XXX
 */
static void hook_quit(cptr str)
{
  /* Give a warning */
  if (str) MessageBox(screen.w, str, "Error", MB_OK | MB_ICONSTOP);

  /* Save the preferences */
  save_prefs();


  /*** Destroy the windows ***/

#ifdef GRAPHIC_CHOICE
  DestroyWindow(choice.w);
  choice.w = 0;
#endif

#ifdef GRAPHIC_RECALL
  DestroyWindow(recall.w);
  recall.w = 0;
#endif

#ifdef GRAPHIC_MIRROR
  DestroyWindow(mirror.w);
  mirror.w = 0;
#endif

  DestroyWindow(screen.w);
  screen.w = 0;


  /*** Free some stuff ***/

  DeleteObject(hbrYellow);

  if (hPal) DeleteObject(hPal);

  if (infGraph.hDIB) GlobalFree(infGraph.hDIB);
  if (infGraph.hPalette) DeleteObject(infGraph.hPalette);
  if (infGraph.hBitmap) DeleteObject(infGraph.hBitmap);


  /*** Free some fonts ***/
  
  if (screen.font_id)
  {
    DeleteObject(screen.font_id);
  }

#ifdef GRAPHIC_MIRROR
  if (mirror.font_id)
  {
    DeleteObject(mirror.font_id);
  }
#endif

#ifdef GRAPHIC_RECALL
  if (recall.font_id)
  {
    DeleteObject(recall.font_id);
  }
#endif

#ifdef GRAPHIC_CHOICE
  if (choice.font_id)
  {
    DeleteObject(choice.font_id);
  }
#endif


  /*** Free some font resources XXX XXX XXX ***/

  /* RemoveFontResource(screen.font_file); */


  /*** Free some stuff ***/
  
  if (screen.font_file) string_free(screen.font_file);

#ifdef GRAPHIC_MIRROR
  if (mirror.font_file) string_free(mirror.font_file);
#endif
  
#ifdef GRAPHIC_RECALL
  if (recall.font_file) string_free(recall.font_file);
#endif

#ifdef GRAPHIC_CHOICE
  if (choice.font_file) string_free(choice.font_file);
#endif


  /*** Free some stuff ***/
  
  if (screen.graf_file) string_free(screen.graf_file);

#ifdef GRAPHIC_MIRROR
  if (mirror.graf_file) string_free(mirror.graf_file);
#endif
  
#ifdef GRAPHIC_RECALL
  if (recall.graf_file) string_free(recall.graf_file);
#endif

#ifdef GRAPHIC_CHOICE
  if (choice.graf_file) string_free(choice.graf_file);
#endif


  /*** Free some other stuff ***/

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
  if (str) MessageBox(screen.w, str, "Error", MB_OK);

  /* Another message */
  MessageBox(screen.w, "I will now attempt to save and quit.",
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
  if (path[i-1] != '\\')
  {
    path[i++] = '\\';
    path[i] = '\0';
  }

  /* Validate the path */
  validate_dir(path);

  /* Init the file paths */  
  init_file_paths(path);

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
}


#pragma argsused
int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
  LPSTR lpCmdLine, int nCmdShow)
{
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

  /* Prepare the windows */
  init_windows();

  /* Activate hooks */
  plog_aux = hook_plog;
  quit_aux = hook_quit;
  core_aux = hook_core;

  /* Display the "news" message */
  show_news();

  /* Allocate and Initialize various arrays */
  init_some_arrays();

  /* Hack -- assume wizard permissions */
  can_be_wizard = TRUE;

  /* Set the system suffix */
  ANGBAND_SYS = "win";

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
