/* File: main-win.c */

/* Purpose: Support for Windows Angband */

/*
 * Written by Skirmantas Kligys (kligys@scf.usc.edu)
 *
 * Based loosely on "main-mac.c" and "main-xxx.c"
 *
 * Angband 2.7.9v3 modifications by Ben Harrison (benh@voicenet.com)
 *
 * Note that the "Windows" version requires several extra files, which
 * must be placed in various places.  These files are distributed in a
 * special "ext-win" directory.  The "ext-win/README" file describes
 * where the various files should be placed.
 *
 * This file still needs some work, problems are indicated by the
 * string "XXX XXX XXX" in a comment.
 *
 * XXX XXX XXX
 * We need to efficiently handle "TERM_XTRA_CLEAR"
 *
 * XXX XXX XXX
 * We need to correctly handle "TERM_XTRA_FLUSH" (!!!)
 *
 * XXX XXX XXX
 * The "use_graphics" option should affect ALL of the windows. (?)
 * Does "starting" in "use_graphics" mode work correctly?
 *
 * XXX XXX XXX 
 * We need to try and use the generic "Term_pict()" routine...
 *
 * XXX XXX XXX
 * We need to think about the meaning of "scrollable" windows, and about
 * a "resizable" main window, especially in terms of efficiency.  It seems
 * that it would be sufficient to allow resizing and NOT allow scrolling.
 *
 * XXX XXX XXX
 * Verify the use of the "td_ptr" variable.
 *
 * XXX XXX XXX
 * Verify the use of "font_file"
 *
 * XXX XXX XXX
 * Verify the use of "lib/xtra/font/" and "lib/xtra/graf/"
 *
 * XXX XXX XXX
 * Verify the keypress handling code, and that the "main-ibm.prf" file
 * can simply be renamed "main-win.prf" since the macros are identical.
 */

#ifdef _Windows

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
 * ???
 */
#define STRICT

/*
 * exclude parts of WINDOWS.H that are not needed
 */
#define NOSOUND           //Sound APIs and definitions
#define NOCOMM            //Comm driver APIs and definitions
#define NOLOGERROR        //LogError() and related definitions
#define NOPROFILER        //Profiler APIs
#define NOLFILEIO         //_l* file I/O routines
#define NOOPENFILE        //OpenFile and related definitions
#define NORESOURCE        //Resource management
#define NOATOM            //Atom management
#define NOLANGUAGE        //Character test routines
#define NOLSTRING         //lstr* string management routines
#define NODBCS            //Double-byte character set routines
#define NOKEYBOARDINFO    //Keyboard driver routines
#define NOCOLOR           //COLOR_* color values
#define NODRAWTEXT        //DrawText() and related definitions
#define NOSCALABLEFONT    //Truetype scalable font support
#define NOMETAFILE        //Metafile support
#define NOSYSTEMPARAMSINFO //SystemParametersInfo() and SPI_* definitions
#define NODEFERWINDOWPOS  //DeferWindowPos and related definitions
#define NOKEYSTATES       //MK_* message key state flags
#define NOWH              //SetWindowsHook and related WH_* definitions
#define NOCLIPBOARD       //Clipboard APIs and definitions
#define NOICONS           //IDI_* icon IDs
#define NOMDI             //MDI support
#define NOCTLMGR          //Control management and controls
#define NOHELP            //Help support

/*
 * exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
#define NONLS             //All NLS defines and routines
#define NOSERVICE         //All Service Controller routines, SERVICE_ equates, etc.
#define NOKANJI           //Kanji support stuff.
#define NOMCX             //Modem Configuration Extensions

/*
 * Include the "windows" support file
 */
#include <windows.h>

/*
 * exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          // Installable driver support
#define MMNOWAVE         // Waveform support
#define MMNOMIDI         // MIDI support
#define MMNOAUX          // Auxiliary audio support
#define MMNOTIMER        // Timer support
#define MMNOJOY          // Joystick support
#define MMNOMCI          // MCI support
#define MMNOMMIO         // Multimedia file I/O support
#define MMNOMMSYSTEM     // General MMSYSTEM functions

/*
 * Include the ??? files
 */
#include <mmsystem.h>
#include <commdlg.h>

/*
 * Include the Angband header file
 */
#include "angband.h"

/*
 * The "itsybitsy" code (?) does not work with Win32
 */
#ifdef __WIN32__
# undef  USE_ITSYBITSY
#endif

/*
 * Include the "itsybits" support
 */
#ifdef USE_ITSYBITSY
# include "itsybits.h"
#endif

/*
 * Include the ??? support
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
#ifndef __WIN32__
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
#else
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF
#endif


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
 * and would mark a window as "resizable".
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

  HFONT    font_id;
  cptr     font_file;
  uint     font_wid;
  uint     font_hgt;
};


/*
 * Some information for every "term" window
 */
static term_data screen;
static term_data mirror;
static term_data recall;
static term_data choice;

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
static HINSTANCE hInstance = 0;

/*
 * Yellow brush for the cursor
 */
static HBRUSH   hbrYellow = 0;

/*
 * An icon
 */
static HICON    hIcon     = 0;

/*
 * A palette
 */
static HPALETTE hPal      = 0;

#ifdef ALLOW_SCRSAVER
/*
 * The screen saver
 */
static HWND     hwndSaver = 0;
#endif

/*
 * Something XXX XXX XXX
 */
static DIBINIT  infGraph  = {0, 0, 0, 8, 13};

/*
 * An array of sound files
 */
static cptr sound_file[SOUND_MAX];

/*
 * Full path to ANGBAND.INI
 */
static cptr     ini_file = NULL;

/*
 * Name of application
 */
static cptr     AppName  = "ANGBAND";

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
 */

static const BYTE win_pal[16] = {
  0, 15, 8, 4, 1, 2, 3, 4, 7, 8, 13, 12, 9, 10, 11, 12
};


/*
 * Hack -- given a pathname, point at the filename
 */
static char *extract_file_name(char *s)
{
  char *p;

  /* Start at the end */
  p = s + strlen(s) - 1;

  /* Back up to divider */  
  while ((p >= s) && (*p != ':') && (*p != '\\')) p--;

  /* Return file name */
  return (p+1);
}


/*
 * Validate a file
 */
static void validate_file(cptr s)
{
  char path[1024];

  /* Copy it */
  strcpy(path, s);

#ifndef __WIN32__

  unsigned int attrib;

  if (_dos_getfileattr(path, &attrib) ||
      (attrib & (FA_DIREC | FA_LABEL)))
  {
    quit_fmt("Cannot find file:\n%s", path);
  }

#else

  DWORD attrib = GetFileAttributes(path);

  if ((attrib == INVALID_FILE_NAME) ||
      (attrib & FILE_ATTRIBUTE_DIRECTORY))
  {
    quit_fmt("Cannot find file:\n%s", path);
  }

#endif

}


/*
 * Validate a directory
 */
static void validate_dir(cptr s)
{
  int i;

  char path[1024];

  /* Copy it */
  strcpy(path, s);

  /* Check length */
  i = strlen(path);

  /* Remove trailing backslash */
  if (path[i-1] == '\\') path[--i] = '\0';

#ifndef __WIN32__

  unsigned int attrib;

  if (_dos_getfileattr(path, &attrib) ||
      (attrib & FA_LABEL) || !(attrib & FA_DIREC))
  {
    quit_fmt("Cannot find directory:\n%s", path);
  }

#else

  DWORD attrib = GetFileAttributes(path);

  if ((attrib == INVALID_FILE_NAME) ||
      !(attrib & FILE_ATTRIBUTE_DIRECTORY))
  {
    quit_fmt("Cannot find directory:\n%s", path);
  }

#endif

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

  td->font_wid = tm.tmAveCharWidth;
  td->font_hgt = tm.tmHeight;

  td->client_wid = td->vis_cols * td->font_wid + td->size_ow1 + td->size_ow2;
  td->client_hgt = td->vis_rows * td->font_hgt + td->size_oh1 + td->size_oh2;

  /* position not important */
  rc.left = rc.top = 0;
  rc.right = rc.left + td->client_wid;
  rc.bottom = rc.top + td->client_hgt;

  /* Main screen */
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

    rc.right += GetSystemMetrics(SM_CXVSCROLL) + 1;   // why, oh why???
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

  td->size_wid = rc.right - rc.left;
  td->size_hgt = rc.bottom - rc.top;

  /* See CreateWindowEx */
  if (!td->w) return;

  GetWindowRect(td->w, &rc);
  td->pos_x = rc.left;
  td->pos_y = rc.top;
}


/*
 * Write the "preference" data for single term
 */
static void save_prefs_aux(term_data *td, cptr sec_name, cptr use_key)
{
  char buf[32];
  RECT rc;

  if (td->w)
  {
    if (!(td->type_1))
    {
      strcpy(buf, td->visible ? "1" : "0");
      WritePrivateProfileString(sec_name, use_key, buf, ini_file);

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

  strcpy(buf, screen_resizable ? "1" : "0");
  WritePrivateProfileString("Main window", "Resizable", buf, ini_file);

  save_prefs_aux(&screen, "Main window", NULL);
  save_prefs_aux(&mirror, "Mirror window", "UseMirrorWin");
  save_prefs_aux(&recall, "Recall window", "UseRecallWin");
  save_prefs_aux(&choice, "Choice window", "UseChoiceWin");
}


/*
 * Load preference for a single term
 */
static void load_prefs_aux(term_data *td, cptr sec_name, cptr use_key)
{
  char buf[128];

  char fnt[128];

  if (!(td->type_1))
  {
    td->visible = GetPrivateProfileInt(sec_name, use_key, 0, ini_file) != 0;

#ifdef USE_ITSYBITSY
    td->cap_size = min(GetPrivateProfileInt(sec_name, "CapSize", 0, ini_file), 127);
#endif

  }

  /* Hack -- Default font */
  strcpy(fnt, ANGBAND_DIR_XTRA);
  strcat(fnt, "font\\");
  strcat(fnt, "7x13.fon");

  /* Try for alternative font file */
  GetPrivateProfileString(sec_name, "FontFile", fnt, buf, 127, ini_file);
  td->font_file = string_make(buf);
  validate_file(td->font_file);

  td->vis_cols = GetPrivateProfileInt(sec_name, "Columns", 80, ini_file);
  td->vis_rows = GetPrivateProfileInt(sec_name, "Rows", 5, ini_file);

  td->pos_x = GetPrivateProfileInt(sec_name, "PositionX", 0, ini_file);
  td->pos_y = GetPrivateProfileInt(sec_name, "PositionY", 0, ini_file);
}


/*
 * Hack -- load a "sound" preference by index and name
 */
static void load_prefs_sound(int i, cptr name);
{
  char tmp[128];
  char buf[1024];

  /* Look up the sound by name XXX XXX XXX */
  GetPrivateProfileString("Sound", name, "", tmp, 127, ini_file);

  /* Nothing found XXX XXX XXX */
  if (!tmp[0]) return;

  /* Access the sound */
  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "sound\\");
  strcat(buf, tmp);

  /* Validate the sound */
  validate_file(buf);

  /* Save the sound */
  sound_file[i] = string_make(buf);
}


/*
 * Load the preferences from the .INI file
 */
static void load_prefs(void)
{
  char  buf[128];
  char *p, *tail;
  int   i, len;

  /* General options */
  use_graphics = (GetPrivateProfileInt("Angband", "Graphics", 0, ini_file) != 0);
  use_sound = (GetPrivateProfileInt("Angband", "Sound", 0, ini_file) != 0);

  /* This is a really odd option, what is it for XXX XXX XXX */
  screen_resizable = (GetPrivateProfileInt("Main window", "Resizable", 0, ini_file) != 0);

  /* Load window prefs */
  load_prefs_aux(&screen, "Main window", NULL);
  load_prefs_aux(&mirror, "Mirror window", "UseMirrorWin");
  load_prefs_aux(&recall, "Recall window", "UseRecallWin");
  load_prefs_aux(&choice, "Choice window", "UseChoiceWin");

  /* Prepare the sounds */
  load_prefs_sound(SOUND_HIT, "Hit");  
  load_prefs_sound(SOUND_MISS, "Miss");  
  load_prefs_sound(SOUND_FLEE, "Flee");  
  load_prefs_sound(SOUND_DROP, "Drop");  
  load_prefs_sound(SOUND_KILL, "Kill");  
  load_prefs_sound(SOUND_LEVEL, "Level");  
  load_prefs_sound(SOUND_DEATH, "Death");  
}


/*
 * Loads font file specified in td->font_file,
 * initializes td->font_xxx, td->size_xxx.
 */
static void term_load_font(term_data *td)
{
  char       fontname[9];
  int        len;
  char       *p, *q, *s, *d;
  int        x, y;

  if (!td->font_file) quit("no font selected");

  validate_file(td->font_file);

  td->font_id = 0;
  len = strlen(td->font_file);
  q = td->font_file + len - 4;
  if ((len < 5) || (stricmp(q, ".FON") != 0))
  {
    quit_fmt("font_file doesn't end in .FON:\n%s", td->font_file);
  }
  p = extract_file_name(td->font_file);
  if (q - p > 8)
  {
    quit_fmt("Too long filename in font_file:\n%s", td->font_file);
  }
  x = atoi(p);
  y = 0;
  for (s = p, d = fontname; (*s != '\0') && (s < q);)
  {
    if ((toupper(*s) == 'X') && (y == 0)) y = atoi(s+1);
    *(d++) = toupper(*(s++));
  }
  *d = '\0';

  if (AddFontResource(td->font_file))
  {
    if ((x == 0) || (y == 0)) x = y = 0;
    td->font_id = CreateFont(y, x, 0, 0, FW_DONTCARE, 0, 0, 0,
           ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
           DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE, fontname);
  }
  else
  {
    quit_fmt("Font file corrupted:\n%s", td->font_file);
  }

  term_getsize(td);
}


/*
 * Loads a named bitmap, initialize "infGraph".
 */
static void term_load_bitmap(char *name)
{
  validate_file(name);

  if (infGraph.hDIB) GlobalFree(infGraph.hDIB);
  if (infGraph.hPalette) DeleteObject(infGraph.hPalette);
  if (infGraph.hBitmap) DeleteObject(infGraph.hBitmap);

  if (!ReadDIB(screen.w, p, &infGraph))
  {
    quit_fmt("Bitmap corrupted:\n%s", name);
  }

  d = extract_file_name(name);

  infGraph.CellWidth = atoi(d);
  infGraph.CellHeight = 0;
  for (s = d; (*s != '\0') && (s < q); s++)
  {
    if (toupper(*s) == 'X')
    {
      infGraph.CellHeight = atoi(s+1);
      break;
    }
  }
}


static void term_window_resize(term_data *td)
{
  RECT  rc;
  POINT pt;

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

  InvalidateRect(td->w, NULL, TRUE);
}


static void term_change_font_aux(term_data *td, char *font_name)
{
  if (td->font_id) DeleteObject(td->font_id);

  if ((td != &screen) && (stricmp(screen.font_file, td->font_file) == 0) ||
      (td != &mirror) && (stricmp(mirror.font_file, td->font_file) == 0) ||
      (td != &recall) && (stricmp(recall.font_file, td->font_file) == 0) ||
      (td != &choice) && (stricmp(choice.font_file, td->font_file) == 0)) {

    /* Do nothing */
  }
  else
  {
    RemoveFontResource(td->font_file);
  }

  if (td->font_file) string_free(td->font_file);

  td->font_file = string_make(font_name);
}


static void term_change_font(term_data *td)
{
  OPENFILENAME ofn;
  char         tmp[128];

  char buf[1024];

  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "font");

  tmp[0] = '\0';

  if (td->font_file)
  {
    strcpy(tmp, extract_file_name(td->font_file));
  }

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

  if (GetOpenFileName(&ofn))
  {
    term_change_font_aux(td, tmp);

    term_load_font(td);

    term_window_resize(td);
  }
}


/*
 * Create and select into DCs of all windows a palette combining
 * the text palette from win_clr[] and the bitmap palette hBmPal.
 * Destroy previous hPal.
 */
void new_palette(HPALETTE *pPal, HPALETTE hBmPal)
{
    HPALETTE       hNewPal;
    HDC            hdc;
    int            i, nEntries;
    LPLOGPALETTE   pLogPal;
    LPPALETTEENTRY lppe, p;

    if (!hBmPal)
    {
      lppe = NULL;
      nEntries = 0;
    }
    else
    {
      lppe = (LPPALETTEENTRY)ralloc(256*sizeof(PALETTEENTRY));
      nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
      if (nEntries == 0) quit("Corrupted bitmap palette");
      else if (nEntries > 220) quit("Bitmap must have no more than 220 colors");
    }

    pLogPal = (LPLOGPALETTE)ralloc(sizeof(LOGPALETTE) +
              (16+nEntries)*sizeof(PALETTEENTRY));
    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = 16+nEntries;
    for (i = 0; i < nEntries; i++)
      pLogPal->palPalEntry[i] = lppe[i];
    for (i = 0; i < 16; i++)
    {
      p = &(pLogPal->palPalEntry[i+nEntries]);
      p->peRed = GetRValue(win_clr[i]);
      p->peGreen = GetGValue(win_clr[i]);
      p->peBlue = GetBValue(win_clr[i]);
      p->peFlags = PC_NOCOLLAPSE;
    }

    hNewPal = CreatePalette((LPLOGPALETTE) pLogPal);
    if (!hNewPal) quit("Cannot create palette");

    if (lppe) rnfree(lppe, 1);

    rnfree(pLogPal, 1);

    hdc = GetDC(screen.w);
    SelectPalette(hdc, hNewPal, 0);
    i = RealizePalette(hdc);
    ReleaseDC(screen.w, hdc);
    if (i == 0) quit("Cannot realize palette");

    hdc = GetDC(mirror.w);
    SelectPalette(hdc, hNewPal, 0);
    //RealizePalette(hdc);
    ReleaseDC(mirror.w, hdc);

    hdc = GetDC(recall.w);
    SelectPalette(hdc, hNewPal, 0);
    //RealizePalette(hdc);
    ReleaseDC(recall.w, hdc);

    hdc = GetDC(choice.w);
    SelectPalette(hdc, hNewPal, 0);
    //RealizePalette(hdc);
    ReleaseDC(choice.w, hdc);

    if (*pPal) DeleteObject(*pPal);
    *pPal = hNewPal;
}


static void term_change_bitmap(term_data *td)
{
  OPENFILENAME ofn;
  char         tmp[128];

  char buf[1024];

  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "graf");

  tmp[0] = '\0';

  if (td->font_file)
  {
    strcpy(tmp, extract_file_name(td->font_file));
    strcpy(tmp + strlen(tmp) - 4, ".BMP");
  }

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

  if (GetOpenFileName(&ofn))
  {
    term_load_bitmap(tmp);

    if (paletted) new_palette(&hPal, infGraph.hPalette);

    strcpy(buf, ANGBAND_DIR_XTRA);
    strcat(buf, "font\\");
    strcat(buf, extract_file_name(tmp));
    strcpy(buf + strlen(tmp) - 4, ".FON");

    term_change_font_aux(td, buf);

    term_load_font(td);

    term_window_resize(td);
  }
}


static void term_force_font(term_data *td, char *font_name)
{
  char tmp[128];

  char buf[1024];

  /* Access the font */
  strcpy(tmp, ANGBAND_DIR_XTRA);
  strcat(tmp, "font\\");
  strcat(tmp, font_name);

  /* Validate the font */
  validate_file(tmp);

  term_change_font_aux(td, tmp);

  term_load_font(td);

  /* Extract a bitmap name XXX XXX XXX */
  strcpy(buf, ANGBAND_DIR_XTRA);
  strcat(buf, "graf\\");
  strcat(buf, extract_file_name(font_name));
  strcpy(buf + strlen(buf) - 4, ".BMP");

  term_load_bitmap(buf);

  if (paletted) new_palette(&hPal, infGraph.hPalette);

  term_window_resize(td);
}


/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
    /* Main screen XXX XXX XXX */
    if (!(td->type_1) && !screen_resizable)
    {
      HPEN hpenOld, hpenLtGray;
      HDC  hdc = GetDC(td->w);

      /* Frame the window in light_gray */
      hpenLtGray = CreatePen(PS_SOLID, 1, win_clr[TERM_L_WHITE]);
      hpenOld = SelectObject(hdc, hpenLtGray);

#ifdef __WIN32__
      MoveToEx(hdc, 0, 0, NULL);
#else
      MoveTo(hdc, 0, 0);
#endif
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
 * Mega-Hack -- Interact with the User
 */
static errr Term_user_win(int n)
{
  /* Save */
  Term_save();

  /* Clear */
  Term_clear();

  /* Describe */
  prt("Screen: ", 1, 1);
  if (paletted) prt("paletted,", 1, 10);
  else prt("not paletted,", 1, 10);
  if (colors16) prt("16 colors", 1, 24);
  else prt(">16 colors", 1, 24);

  /* Display the colors */
  Term_putstr(1,  3, -1, TERM_DARK,    "### Dark");
  Term_putstr(1,  4, -1, TERM_WHITE,   "### White");
  Term_putstr(1,  5, -1, TERM_SLATE,   "### Slate");
  Term_putstr(1,  6, -1, TERM_ORANGE,  "### Orange");
  Term_putstr(1,  7, -1, TERM_RED,     "### Red");
  Term_putstr(1,  8, -1, TERM_GREEN,   "### Green");
  Term_putstr(1,  9, -1, TERM_BLUE,    "### Blue");
  Term_putstr(1, 10, -1, TERM_UMBER,   "### Umber");
  Term_putstr(1, 11, -1, TERM_L_DARK,  "### Light Dark");
  Term_putstr(1, 12, -1, TERM_L_WHITE, "### Light Slate");
  Term_putstr(1, 13, -1, TERM_VIOLET,  "### Violet");
  Term_putstr(1, 14, -1, TERM_YELLOW,  "### Yellow");
  Term_putstr(1, 15, -1, TERM_L_RED,   "### Light Red");
  Term_putstr(1, 16, -1, TERM_L_GREEN, "### Light Green");
  Term_putstr(1, 17, -1, TERM_L_BLUE,  "### Light Blue");
  Term_putstr(1, 18, -1, TERM_L_UMBER, "### Light Umber");

  /* Wait */
  inkey();

  /* Restore */
  Term_load();
}


/*
 * Hack -- process at least one event
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
 * Hack -- flush all keypress events
 */
static errr Term_xtra_win_flush(void)
{
  /* XXX XXX XXX XXX */
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
static errr Term_xtra_win_noise(int v)
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

#ifndef __WIN32__
  return (sndPlaySound(sound_file[v], SND_ASYNC));
#else
  return (PlaySound(sound_file[v], 0, SND_FILENAME | SND_ASYNC));
#endif

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

    /* clip to visible part of window */
    x -= td->scroll_hpos;
    y -= td->scroll_vpos;
    if ((x > td->vis_cols) || (y > td->vis_rows)) return 0;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    w = min(w, td->vis_cols - x);
    h = min(h, td->vis_rows - y);

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

    /* clip to visible part of window */
    x -= td->scroll_hpos;
    y -= td->scroll_vpos;

    /* Off screen */
    if ((x < 0) || (x > td->vis_cols)) return (0);
    if ((y < 0) || (y > td->vis_rows)) return (0);

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


#define BM_COLS  64

/*
 * XXX XXX XXX
 */
static u16b char_to_grfx(byte a, byte c)
{
  /* XXX XXX XXX Oops */
  if (c == 32) return 32;

  /* Analyze it */
  return (c + (a-128)*128);
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw a special picture at the given location.
 */
static errr Term_pict_win(int x, int y, int p)
{
    term_data *td = (term_data*)(Term->data);

#if 0

    RECT rc;
    HDC  hdc;
    HDC hdcMem, hdcSrc;
    HBITMAP hbmSrcOld, hbmMemOld, hbmMem;
    int  scr_x, scr_y, mem_x, i;
    int  w, h;

    /* clip to visible part of window */
    x -= td->scroll_hpos;
    y -= td->scroll_vpos;
    if ((x < 0) || (x >= (int)td->vis_cols)) return (0);
    if ((y < 0) || (y >= (int)td->vis_rows)) return 0;

    scr_x = x * td->font_wid + td->size_ow1;
    scr_y = y * td->font_hgt + td->size_oh1;
    w = td->font_wid;
    h = td->font_hgt;

    hdc = GetDC(td->w);
    hdcSrc = CreateCompatibleDC(hdc);
    hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

    /* Only double-buffer if more than one graphics char */
    BitBlt(hdc, scr_x, scr_y, w, h,
           hdcSrc, p % BM_COLS * infGraph.CellWidth,
           p / BM_COLS * infGraph.CellHeight, SRCCOPY);

    SelectObject(hdcSrc, hbmSrcOld);
    DeleteDC(hdcSrc);
    ReleaseDC(td->w, hdc);
  }

#endif

  /* Success */
  return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * Hack -- If *s >= 128, draw it all as graphics.
 * Eventually, use the "Term_pict_win()" interface instead.
 */
static errr Term_text_win(int x, int y, int n, byte a, const char *s)
{
  term_data *td = (term_data*)(Term->data);
  RECT rc;
  HDC  hdc;
  HDC hdcMem, hdcSrc;
  HBITMAP hbmSrcOld, hbmMemOld, hbmMem;
  int  scr_x, scr_y, mem_x, i;
  int  w, h;
  u16b g;

  /* clip to visible part of window */
  x -= td->scroll_hpos;
  y -= td->scroll_vpos;
  if ((x >= (int)td->vis_cols)) return (0);
  if ((y < 0) || (y >= (int)td->vis_rows)) return 0;

  if (x < 0)
  {
    if (n <= -x) return 0;
    s += -x;
    n -= -x;
    x = 0;
  }
  n = min(n, td->vis_cols - x);
  if (n <= 0) return 0;

  /* Hack -- text output */
  if (!(a & 0x80) || !use_graphics)
  {
    /* Stop illegal attributes */
    if ((a == 0) || (a >= 16)) a = TERM_RED;

    rc.left   = x * td->font_wid + td->size_ow1;
    rc.right  = rc.left + n * td->font_wid;
    rc.top    = y * td->font_hgt + td->size_oh1;
    rc.bottom = rc.top + td->font_hgt;

    /* Draw the string (will only work for mono-spaced fonts) */
    hdc = GetDC(td->w);
    SetBkColor(hdc, RGB(0,0,0));

    SetTextColor(hdc, colors16 ? PALETTEINDEX(win_pal[a]) : win_clr[a]);
    SelectObject(hdc, td->font_id);
    ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
               s, n, NULL);
    ReleaseDC(td->w, hdc);
  }

  /* graphics output */
  else
  {
    scr_x = x * td->font_wid + td->size_ow1;
    scr_y = y * td->font_hgt + td->size_oh1;
    w = td->font_wid;
    h = td->font_hgt;

    hdc = GetDC(td->w);
    hdcSrc = CreateCompatibleDC(hdc);
    hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

    /* Only double-buffer if more than one graphics char */
    if (n == 1)
    {
      g = char_to_grfx(a, *s);
      BitBlt(hdc, scr_x, scr_y, w, h,
             hdcSrc, g % BM_COLS * infGraph.CellWidth,
             g / BM_COLS * infGraph.CellHeight, SRCCOPY);
    }
    else
    {
      hdcMem = CreateCompatibleDC(hdc);
      hbmMem = CreateCompatibleBitmap(hdc, n * w, h);
      hbmMemOld = SelectObject(hdcMem, hbmMem);

      for (i = 0, mem_x = 0; i < n; i++, mem_x += w)
      {
        /* Translate to graphics index */
        g = char_to_grfx(a, s[i]);
        BitBlt(hdcMem, mem_x, 0, w, h,
               hdcSrc, g % BM_COLS * infGraph.CellWidth,
               g / BM_COLS * infGraph.CellHeight, SRCCOPY);
      }

      BitBlt(hdc, scr_x, scr_y, n * w, h, hdcMem, 0, 0, SRCCOPY);

      hbmMem = SelectObject(hdcMem, hbmMemOld);
      DeleteObject(hbmMem);
      DeleteDC(hdcMem);
    }

    SelectObject(hdcSrc, hbmSrcOld);
    DeleteDC(hdcSrc);
    ReleaseDC(td->w, hdc);
  }

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

#ifdef USE_ITSYBITSY

  mirror.dwStyle = (IBS_VERTCAPTION | WS_OVERLAPPED | WS_THICKFRAME | 
                    WS_VSCROLL | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  mirror.dwExStyle = 0;

  recall.dwStyle = (IBS_VERTCAPTION | WS_OVERLAPPED | WS_THICKFRAME | 
                    WS_VSCROLL | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  recall.dwExStyle = 0;

  choice.dwStyle = (IBS_VERTCAPTION | WS_OVERLAPPED | WS_THICKFRAME | 
                    WS_VSCROLL | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  choice.dwExStyle = 0;

#else

  mirror.dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  mirror.dwExStyle = (WS_EX_TOOLWINDOW);

  recall.dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  recall.dwExStyle = (WS_EX_TOOLWINDOW);

  choice.dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL | WS_SYSMENU);
  choice.dwExStyle = (WS_EX_TOOLWINDOW);

#endif

  /* Load the fonts */
  term_load_font(&screen);
  term_load_font(&mirror);
  term_load_font(&recall);
  term_load_font(&choice);

  /* Use graphics XXX XXX XXX */
  if (use_graphics)
  {
    char buf[1024];

    /* term_force_font(&screen, "10x20.FON"); */

    ANGBAND_SYS = "gfw";

#if 0
    player_k_ptr->x_attr = 131;
    player_k_ptr->x_char = 64 + 10*p_ptr->pclass + p_ptr->prace;
#endif

    /* Extract a bitmap name XXX XXX XXX */
    strcpy(buf, ANGBAND_DIR_XTRA);
    strcat(buf, "graf\\");
    strcat(buf, extract_file_name(screen.font_file));
    strcpy(buf + strlen(buf) - 4, ".BMP");

    term_load_bitmap(buf);
  }

  if (paletted) new_palette(&hPal, infGraph.hPalette);

  /* Create a "brush" for drawing the "cursor" */
  hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);

  /* Screen window */
  td_ptr = &screen;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, AppName,
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (td_ptr->w == 0) quit("Failed to create Angband window");

  /* Mirror window */
  td_ptr = &mirror;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, "AngbandList",
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (td_ptr->w == 0) quit("Failed to create mirror window");

  /* Recall window */
  td_ptr = &recall;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, "AngbandList",
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (td_ptr->w == 0) quit("Failed to create recall window");

  /* Choice window */
  td_ptr = &choice;
  td_ptr->w = CreateWindowEx(td_ptr->dwExStyle, "AngbandList",
                             td_ptr->s, td_ptr->dwStyle,
                             td_ptr->pos_x, td_ptr->pos_y,
                             td_ptr->size_wid, td_ptr->size_hgt,
                             HWND_DESKTOP, NULL, hInstance, NULL);
  if (td_ptr->w == 0) quit("Failed to create choice window");


#ifdef USE_ITSYBITSY
  if (mirror.cap_size)
  {
    ibSetCaptionSize(mirror.w, mirror.cap_size);
  }

  if (recall.cap_size)
  {
    ibSetCaptionSize(recall.w, recall.cap_size);
  }

  if (choice.cap_size)
  {
    ibSetCaptionSize(choice.w, choice.cap_size);
  }
#endif


  if (mirror.visible) ShowWindow(mirror.w, SW_SHOW);

  if (recall.visible) ShowWindow(recall.w, SW_SHOW);

  if (choice.visible) ShowWindow(choice.w, SW_SHOW);


  SetActiveWindow(screen.w);


  /* Link the Choice "term" */
  term_data_link(&choice);
  term_choice = &choice.t;

  /* Link the Recall "term" */
  term_data_link(&recall);
  term_recall = &recall.t;

  /* Link the Mirror "term" */
  term_data_link(&mirror);
  term_choice = &mirror.t;

  /* Link the Screen "term" */
  term_data_link(&screen);
  term_screen = &screen.t;

  /* Bring screen.w back to top */
  SetWindowPos(screen.w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  /* Process WM_PAINT messages before printing anything */
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}


/*
 * Delay for "x" milliseconds
 */
void delay(int x)
{

#ifdef __WIN32__

    Sleep(x);

#else

    DWORD t;
    MSG   msg;

    t = GetTickCount() + x;
    while(GetTickCount() < t)
    {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

#endif

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

  /* Options/Font/Mirror window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_MIRROR, MF_BYCOMMAND |
      (mirror.visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));

  /* Options/Font/Recall window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_RECALL, MF_BYCOMMAND |
      (recall.visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));

  /* Options/Font/Choice window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_CHOICE, MF_BYCOMMAND |
      (choice.visible ? MF_ENABLED : MF_DISABLED | MF_GRAYED));

  /* Item "Mirror Window" */
  CheckMenuItem(hm, IDM_OPTIONS_MIRROR, MF_BYCOMMAND |
      (mirror.visible ? MF_CHECKED : MF_UNCHECKED));

  /* Item "Recall Window" */
  CheckMenuItem(hm, IDM_OPTIONS_RECALL, MF_BYCOMMAND |
      (recall.visible ? MF_CHECKED : MF_UNCHECKED));

  /* Item "Choice Window" */
  CheckMenuItem(hm, IDM_OPTIONS_CHOICE, MF_BYCOMMAND |
      (choice.visible ? MF_CHECKED : MF_UNCHECKED));

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
        Term_fresh();
        //validate_file(savefile);

        /* The player is not dead */
        (void)strcpy(died_from, "(saved)");

        /* Save the player, note the result */
        prt("Saving game...", 0, 0);
        if (save_player()) prt("done.", 0, 14);
        else prt("Save failed.", 0, 14);
        Term_fresh();
        Term_keypress(ESCAPE);

        /* Forget that the player was saved */
        character_saved = 0;

        /* Hilite the player */
        move_cursor_relative(py,px);

        /* Note that the player is not dead */
        (void)strcpy(died_from, "(alive and well)");
      }
      break;
    }

    /* Save and Exit */
    case IDM_FILE_EXIT:
    {
      if (game_in_progress && save_enabled)
      {
        (void)strcpy(died_from, "(saved)");
        prt("Saving game...", 0, 0);
        if (!save_player())
        {
           prt("Save failed.", 0, 14);
           Term_fresh();
        }
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

    /* Font */
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

    /* XXX XXX XXX */
    case IDM_OPTIONS_FONT_MIRROR:
    {
      term_change_font(&mirror);
      break;
    }

    /* XXX XXX XXX */
    case IDM_OPTIONS_FONT_RECALL:
    {
      term_change_font(&recall);
      break;
    }

    /* XXX XXX XXX */
    case IDM_OPTIONS_FONT_CHOICE:
    {
      term_change_font(&choice);
      break;
    }

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
      char buf[128];

      /* Reset the visuals */
      reset_visuals();

      /* Use graphics */
      if (!use_graphics)
      {
        use_graphics = TRUE;

        term_force_font(&screen, "10x20.FON");

        ANGBAND_SYS = "gfw";

#if 0
        player_k_ptr->x_attr = 131;
        player_k_ptr->x_char = 64 + 10*p_ptr->pclass + p_ptr->prace;
#endif

      }

      /* Use text */
      else
      {
        use_graphics = FALSE;

        ANGBAND_SYS = "txw";
      }

      /* XXX XXX XXX Load the "pref" file */
      sprintf(buf, "pref-%s.prf", ANGBAND_SYS);
      process_pref_file(buf);

      /* Hack -- redraw */
      do_cmd_redraw();
      move_cursor_relative(py, px);
      Term_fresh();

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


/*
 * Hack -- arrow keys translation
 */
static char TransDir[8] = {
  '9','3','1','7','4','8','6','2'
};


/*
 * Hack -- convert a number (0 to 15) to a uppercase hecidecimal digit
 */
static char hexsym[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};



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
        SetWindowLong(hWnd, 0, (LONG)(&screen));
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
      Term = &td->t;
      term_data_redraw(td);
      Term = term_screen;
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
          if (mirror.visible) ShowWindow(mirror.w, SW_HIDE);
          if (recall.visible) ShowWindow(recall.w, SW_HIDE);
          if (choice.visible) ShowWindow(choice.w, SW_HIDE);
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
          if (mirror.visible) ShowWindow(mirror.w, SW_SHOWNOACTIVATE);
          if (recall.visible) ShowWindow(recall.w, SW_SHOWNOACTIVATE);
          if (choice.visible) ShowWindow(choice.w, SW_SHOWNOACTIVATE);
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
        SetWindowPos(mirror.w, hWnd, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(recall.w, hWnd, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(choice.w, hWnd, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
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


#pragma argsused
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
    case WM_NCCREATE:
    {
      if (td_ptr == &screen)
      {
#ifdef USE_ITSYBITSY
        return ibDefWindowProc(hWnd, uMsg, wParam, lParam);
#else
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
#endif
      }
      else
      {
        SetWindowLong(hWnd, 0, (LONG)(td_ptr));
      }
      break;
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
      Term = &td->t;
      BeginPaint(hWnd, &ps);
      Term_redraw();
      EndPaint(hWnd, &ps);
      Term = &screen.t;
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
        SetWindowLong(hWnd, 0, (LONG)(&screen));
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


/*** Some Hooks for various routines ***/

/*
 * Allocate memory -- See "z-virt.c"
 */
static vptr hook_ralloc(huge size)
{

#ifndef __WIN32__
  if (size > 0xFFFF) quit("Tried to malloc more than 64K");
#endif

  return ((vptr)malloc((size_t)size));
}


/*
 * De-Allocate memory -- See "z-virt.c"
 */
#pragma argsused
static errr hook_rnfree(vptr v, huge size)
{
  free(v);
  return 0;
}


/*
 * Error message -- See "z-util.c"
 */
static void hook_plog(cptr str)
{
  MessageBox(screen.w, str, "Warning", MB_OK);
}


/*
 * Quit with error message -- See "z-util.c"
 */
static void hook_quit(cptr str)
{
  if (str) MessageBox(screen.w, str, "Error", MB_OK | MB_ICONSTOP);

  save_prefs();

  if (choice.w)
  {
    DestroyWindow(choice.w);
    choice.w = 0;
  }

  if (recall.w)
  {
    DestroyWindow(recall.w);
    recall.w = 0;
  }

  if (mirror.w)
  {
    DestroyWindow(mirror.w);
    mirror.w = 0;
  }

  if (screen.w)
  {
    DestroyWindow(screen.w);
    screen.w = 0;
  }

  DeleteObject(hbrYellow);
  if (hPal) DeleteObject(hPal);
  if (infGraph.hDIB) GlobalFree(infGraph.hDIB);
  if (infGraph.hPalette) DeleteObject(infGraph.hPalette);
  if (infGraph.hBitmap) DeleteObject(infGraph.hBitmap);

  if (screen.font_id)
  {
    DeleteObject(screen.font_id);
  }
  if (screen.font_file)
  {
    RemoveFontResource(screen.font_file);
    string_free(screen.font_file);
  }

  if (mirror.font_id)
  {
    DeleteObject(mirror.font_id);
  }
  if (mirror.font_file)
  {
    RemoveFontResource(mirror.font_file);
    string_free(mirror.font_file);
  }

  if (recall.font_id)
  {
    DeleteObject(recall.font_id);
  }
  if (recall.font_file)
  {
    RemoveFontResource(recall.font_file);
    string_free(recall.font_file);
  }

  if (choice.font_id)
  {
    DeleteObject(choice.font_id);
  }
  if (choice.font_file)
  {
    RemoveFontResource(choice.font_file);
    string_free(choice.font_file);
  }

  UnregisterClass(AppName, hInstance);

  if (hIcon) DestroyIcon(hIcon);

  exit(0);
}


static void hook_core(cptr str)
{
  if (str) MessageBox(screen.w, str, "Error", MB_OK);

  MessageBox(screen.w, "I will now attempt to save and quit.",
             "Fatal error", MB_OK | MB_ICONSTOP);

  save_player();

  quit(NULL);
}


/*
 * Init some stuff
 */
static void init_stuff(void)
{
  int   i;

  char path[1024];

  /* Hack -- access "ANGBAND.INI" */
  GetModuleFileName(hInstance, path, 512);
  strcpy(tmp + strlen(tpathp) - 4, ".INI");

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
    wc.lpszClassName = "AngbandList";

    if (!RegisterClass(&wc)) exit(2);

#ifdef ALLOW_SCRSAVER

    wc.style          = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
    wc.lpfnWndProc    = AngbandSaverProc;
    wc.hCursor        = NULL;
    //wc.hIcon          = LoadIcon(hInst, MAKEINTATOM(ID_APP));
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = "WindowsScreenSaverClass";

    if (!RegisterClass(&wc)) exit(3);

#endif

  }

  /* Assume text */
  ANGBAND_SYS = "txw";

  /* Hook in some "z-virt.c" hooks */
  ralloc_aux = hook_ralloc;
  rnfree_aux = hook_rnfree;

  /* Hook in some "z-util.c" hooks */
  plog_aux = hook_plog;
  quit_aux = hook_quit;
  core_aux = hook_core;

  /* Prepare the filepaths */
  init_stuff();

  /* Determine if display is 16/256/true color */
  hdc = GetDC(NULL);
  colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
  paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE);
  ReleaseDC(NULL, hdc);

  /* Prepare the windows */
  init_windows();

  /* Display the "news" message */
  show_news();

  /* Allocate and Initialize various arrays */
  init_some_arrays();

  /* No name (yet) */
  strcpy(player_name, "");

  /* Hack -- assume wizard permissions */
  can_be_wizard = TRUE;

  /* We are now initialized */
  initialized = TRUE;

  /* Did the user double click on a save file? */
  check_for_save_file(lpCmdLine);

  /* Prompt the user */
  prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
  Term_fresh();

  /* Process messages (until "play_game()" is called) */
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  /* Quit */
  quit(NULL);

  /* Paranoia */
  return (0);
}

#endif /* _Windows */
