/* File: main-win.c */

/* Purpose: Support for Windows Angband */

/*
 * Written by Skirmantas Kligys (kligys@scf.usc.edu)
 * looking at main-mac.c by Ben Harrison
 *
 * See "term.c" for info on the "generic terminal" that we support.
 *
 * See "recall.c"/"moria1.c" for info on the "recall"/"choice" windows.
 */

#ifdef _Windows

#define USE_GRAPHICS
#undef  COLOR_DEBUG


/* #include "main-win.h" */

/* Note -- Begin "main-win.h" */

/* main-win.h */

#define IDM_FILE_NEW             101
#define IDM_FILE_OPEN            102
#define IDM_FILE_SAVE            103
#define IDM_FILE_EXIT            104
#define IDM_FILE_QUIT            105
#define IDM_OPTIONS_FONT_ANGBAND 201
#define IDM_OPTIONS_FONT_RECALL  202
#define IDM_OPTIONS_FONT_CHOICE  203
#define IDM_OPTIONS_RECALL       212
#define IDM_OPTIONS_CHOICE       213
#define IDM_OPTIONS_RESIZABLE    221
#define IDM_OPTIONS_GRAPHICS     222

/* exclude parts of WINDOWS.H that are not needed */
#define NOSOUND           //Sound APIs and definitions
#define NOCOMM            //Comm driver APIs and definitions
#define NODRIVERS         //Installable driver APIs and definitions
#define NOLOGERROR        //LogError() and related definitions
#define NOPROFILER        //Profiler APIs
//#define NOMEMMGR          //Local and global memory management
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


#define STRICT
#include <windows.h>
#include <commdlg.h>

#include "angband.h"
#include <stdarg.h>

#include "itsybits.h"

#ifdef USE_GRAPHICS
#include "readdib.h"
#endif

/* string.h excludes this because __STDC__=1 */
int stricmp(const char *, const char *);

/* that's all we need from dos.h, cannot include it
 * since peek() is defined already */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);

/*
 * Extra "term" data
 */

#define WTY_SCREEN    1
#define WTY_RECALL    2
#define WTY_CHOICE    3

typedef struct _term_data term_data;

struct _term_data {
  term     t;
  cptr     s;
  HWND     w;
  DWORD    dwStyle;
  uint     type;
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
  uint     cap_size;

  uint     mapped;
  HFONT    font_id;
  char_ptr font_file;
  uint     font_wid;
  uint     font_hgt;
};

/*
 * Some information for every "term" window
 */
term_data screen;
term_data recall;
term_data choice;
#pragma hdrstop

/*
 * Various boolean flags
 */
bool game_in_progress  = FALSE;  /* game in progress */
bool save_enabled      = FALSE;  /* game can be saved */
bool screen_resizable  = FALSE;  /* main window ("screen") resizable */
bool use_graphics      = FALSE;  /* use graphical chars */
bool initialized       = FALSE;  /* note when "open"/"new" become valid */
bool creating          = 0;      /* Hack: set it to WTY_SCREEN, WTY_RECALL or */
				 /* WTY_CHOICE so that WM_NCCREATE handler */
				 /* knows which window is indeed created */
bool paletted          = FALSE;  /* screen paletted, i.e. 256 colors */
bool colors16          = FALSE;  /* 16 colors screen, don't use RGB() */


/*
 * Saved instance handle
 */
static HINSTANCE hInstance = 0;

static char_ptr ANGBAND_DIR_RSRC = 0;

static HBRUSH   hbrYellow = 0;
static HBRUSH   hbrRed    = 0;
static HICON    hIcon     = 0;
static HPALETTE hPal      = 0;

#ifdef USE_GRAPHICS
static DIBINIT  infGraph = {0, 0, 0, 8, 13};
#endif


/* full path to ANGBAND.INI */
static cptr     ini_file = NULL;

static cptr AppName = "ANGBAND";

/*
 * The "term.c" color set:
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * This happens (surprise) to be the same as the "Angband" color set.
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 * Note that on B/W machines, all non-zero colors can be white (on black).
 *
 * Note that all characters are assumed to be drawn on a black background.
 * This may require calling "Term_wipe()" before "Term_text()", etc.
 *
 * The Windows "Term_curs()" ignores the symbol under the cursor.
 */
#if 1
/* The colors come from MAIN-X11.C */
static const COLORREF win_clr[16] = {
  PALETTERGB(0x00, 0x00, 0x00),  /* BLACK 15 */
  PALETTERGB(0xFF, 0xFF, 0xFF),  /* WHITE 0 */
  PALETTERGB(0xA0, 0xA0, 0xA0),  /* GRAY 13 */
  PALETTERGB(0xFF, 0x92, 0x00),  /* ORANGE 2 */
  PALETTERGB(0xB0, 0x00, 0x00),  /* RED 3 */
  PALETTERGB(0x00, 0xB0, 0x00),  /* GREEN 9 */
  PALETTERGB(0x00, 0x00, 0xFF),  /* BLUE 6 */
  PALETTERGB(0xC8, 0x64, 0x00),  /* UMBER 10 */
  PALETTERGB(0x70, 0x70, 0x70),  /* DARKGRAY 14 */
  PALETTERGB(0xD0, 0xD0, 0xD0),  /* LIGHTGRAY 12 */
  PALETTERGB(0xA5, 0x00, 0xFF),  /* VIOLET 5 */
  PALETTERGB(0xFF, 0xFD, 0x00),  /* YELLOW 1 */
  PALETTERGB(0xFF, 0x00, 0xBC),  /* LIGHTRED 4 */
  PALETTERGB(0x00, 0xFF, 0x00),  /* LIGHTGREEN 8 */
  PALETTERGB(0x00, 0xC8, 0xFF),  /* LIGHTBLUE 7 */
  PALETTERGB(0xFF, 0xCC, 0x80)   /* LIGHTUMBER 11 */
};

/* Palette indices for 16 colors */
static const BYTE win_pal[16] =
  {0, 15, 8, 4, 1, 2, 3, 4, 7, 8, 13, 12, 9, 10, 11, 12};
#else
/* These were styled after MAIN-MAC.C and are too dark */
#define TO256(r,g,b) PALETTERGB(r*255/65535L,g*255/65535L,b*255/65535L)

static const COLORREF win_clr[16] = {
    TO256(    0L,    0L,    0L),   /* 15 Black */
    TO256(65535L,65535L,65535L),   /* 0 White */
    TO256(32768L,32768L,32768L),   /* 13 Gray */
    TO256(65535L,25738L,  652L),   /* 2 Orange */
    TO256(45000L, 2000L, 2000L),   /* 3 Red */
    TO256(    0L,25775L, 4528L),   /* 9 Green */
    TO256(    0L,    0L,54272L),   /* 6 Blue */
    TO256(22016L,11421L, 1316L),   /* 10 Brown */
    TO256(16384L,16384L,16384L),   /* 14 Dark Gray */
    TO256(49152L,49152L,49152L),   /* 12 Light Gray */
    TO256(18147L,    0L,42302L),   /* 5 Purple */
    TO256(64512L,62333L, 1327L),   /* 1 Yellow */
    TO256(60000L, 2000L, 2000L),   /* 4 Light Red */
    TO256( 7969L,46995L, 5169L),   /* 8 Light Green */
    TO256(  577L,43860L,60159L),   /* 7 Light Blue */
    TO256(37079L,29024L,14900L)    /* 11 Light Brown */
};
#endif

/* Note -- End "main-win.h" */



/*
 * General utilities
 */
static char *extract_file_name(char *s)
{
  char *p;

  for (p = s + strlen(s) - 1; (p >= s)&&(*p != ':')&&(*p != '\\'); p--);
  return ++p;
}


static void _cdecl formatted_quit(char *format, ...)
{
  va_list parg;
  char    tmp[128];

  va_start(parg, format);
  vsprintf(tmp, format, parg);
  va_end(parg);
  quit(tmp);
}


static void validate_file(char *s)
{
  unsigned int attrib;

  if (_dos_getfileattr(s, &attrib) || (attrib & (FA_DIREC | FA_LABEL)))
    formatted_quit("Cannot find file:\n%s", s);
}


static void validate_dir(char *s)
{
  unsigned int attrib;

  if (_dos_getfileattr(s, &attrib) || (attrib & FA_LABEL) ||
      !(attrib & FA_DIREC))
    formatted_quit("Cannot find directory:\n%s", s);
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

  /* all this trouble to get the cell size*/
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
  if (td->type == WTY_SCREEN)
  {
    if (screen_resizable)
    {
      rc.right  += GetSystemMetrics(SM_CXVSCROLL) - 1;
      rc.bottom += GetSystemMetrics(SM_CYHSCROLL) - 1;
    }
  }
  else
    rc.right += GetSystemMetrics(SM_CXVSCROLL) - 1;

  if (td->type == WTY_SCREEN)
  {
    AdjustWindowRect(&rc, td->dwStyle, TRUE);
    /* no idea why this needed */
    rc.bottom++;
  }
  else
    ibAdjustWindowRect(&rc, td->dwStyle, FALSE, td->cap_size);

  td->size_wid = rc.right - rc.left;
  td->size_hgt = rc.bottom - rc.top;

  /* don't update td->pos_* if called from inside CreateWindow */
  if (!td->w) return;
  GetWindowRect(td->w, &rc);
  td->pos_x = rc.left;
  td->pos_y = rc.top;
}


/*
 * Write the "preference" data to the .INI file
 */
static void save_prefs(void)
{
  char       buf[32];
  RECT       rc;
  term_data *td;

#ifdef USE_GRAPHICS
  strcpy(buf, use_graphics ? "1" : "0");
  WritePrivateProfileString("Angband", "Graphics", buf, ini_file);
#endif

  strcpy(buf, screen_resizable ? "1" : "0");
  WritePrivateProfileString("Main window", "Resizable", buf, ini_file);

  td = &screen;
  if (td->w)
  {
    if (td->font_file)
      WritePrivateProfileString("Main window", "FontFile", td->font_file, ini_file);
    wsprintf(buf, "%d", td->vis_cols);
    WritePrivateProfileString("Main window", "Columns", buf, ini_file);
    wsprintf(buf, "%d", td->vis_rows);
    WritePrivateProfileString("Main window", "Rows", buf, ini_file);
    GetWindowRect(screen.w, &rc);
    wsprintf(buf, "%d", rc.left);
    WritePrivateProfileString("Main window", "PositionX", buf, ini_file);
    wsprintf(buf, "%d", rc.top);
    WritePrivateProfileString("Main window", "PositionY", buf, ini_file);
  }

  td = &recall;
  if (td->w)
  {
    strcpy(buf, td->mapped ? "1" : "0");
    WritePrivateProfileString("Recall window", "UseRecallWin", buf, ini_file);
    if (td->font_file)
      WritePrivateProfileString("Recall window", "FontFile", td->font_file, ini_file);
    wsprintf(buf, "%d", td->vis_cols);
    WritePrivateProfileString("Recall window", "Columns", buf, ini_file);
    wsprintf(buf, "%d", td->vis_rows);
    WritePrivateProfileString("Recall window", "Rows", buf, ini_file);
    GetWindowRect(td->w, &rc);
    wsprintf(buf, "%d", rc.left);
    WritePrivateProfileString("Recall window", "PositionX", buf, ini_file);
    wsprintf(buf, "%d", rc.top);
    WritePrivateProfileString("Recall window", "PositionY", buf, ini_file);
    wsprintf(buf, "%d", td->cap_size);
    WritePrivateProfileString("Recall window", "CapSize", buf, ini_file);
  }

  td = &choice;
  if (td->w)
  {
    strcpy(buf, td->mapped ? "1" : "0");
    WritePrivateProfileString("Choice window", "UseChoiceWin", buf, ini_file);
    if (td->font_file)
      WritePrivateProfileString("Choice window", "FontFile", td->font_file, ini_file);
    wsprintf(buf, "%d", td->vis_cols);
    WritePrivateProfileString("Choice window", "Columns", buf, ini_file);
    wsprintf(buf, "%d", td->vis_rows);
    WritePrivateProfileString("Choice window", "Rows", buf, ini_file);
    GetWindowRect(td->w, &rc);
    wsprintf(buf, "%d", rc.left);
    WritePrivateProfileString("Choice window", "PositionX", buf, ini_file);
    wsprintf(buf, "%d", rc.top);
    WritePrivateProfileString("Choice window", "PositionY", buf, ini_file);
    wsprintf(buf, "%d", td->cap_size);
    WritePrivateProfileString("Choice window", "CapSize", buf, ini_file);
  }
}


/*
 * Load the preferences from the .INI file
 */
static void load_prefs(void)
{
  char  buf[128];
  char *p;

#ifdef USE_GRAPHICS
  use_graphics = GetPrivateProfileInt("Angband", "Graphics", 0, ini_file) != 0;
#endif

  screen_resizable =
      GetPrivateProfileInt("Main window", "Resizable", 0, ini_file) != 0;
  GetPrivateProfileString("Main window", "FontFile",
		     "\\angband\\lib-win\\rsrc\\7x13.fon", buf, 128, ini_file);
  screen.font_file = (char *)string_make(buf);
  validate_file(screen.font_file);
  screen.vis_cols = GetPrivateProfileInt("Main window", "Columns", 80, ini_file);
  screen.vis_rows = GetPrivateProfileInt("Main window", "Rows", 24, ini_file);
  screen.pos_x = GetPrivateProfileInt("Main window", "PositionX", 0, ini_file);
  screen.pos_y = GetPrivateProfileInt("Main window", "PositionY", 0, ini_file);

  use_recall_win = GetPrivateProfileInt("Recall window", "UseRecallWin", 1, ini_file) != 0;
  recall.mapped = use_recall_win;
  GetPrivateProfileString("Recall window", "FontFile",
		     "\\angband\\lib-win\\rsrc\\7x13.fon", buf, 128, ini_file);
  recall.font_file = (char *)string_make(buf);
  validate_file(recall.font_file);
  recall.vis_cols = GetPrivateProfileInt("Recall window", "Columns", 80, ini_file);
  recall.vis_rows = GetPrivateProfileInt("Recall window", "Rows", 8, ini_file);
  recall.pos_x = GetPrivateProfileInt("Recall window", "PositionX", 0, ini_file);
  recall.pos_y = GetPrivateProfileInt("Recall window", "PositionY", 0, ini_file);
  recall.cap_size = min(
      GetPrivateProfileInt("Recall window", "CapSize", 0, ini_file),
      127);

  use_choice_win = GetPrivateProfileInt("Choice window", "UseChoiceWin", 0, ini_file) != 0;
  choice.mapped = use_choice_win;
  GetPrivateProfileString("Choice window", "FontFile",
		     "\\angband\\lib-win\\rsrc\\7x13.fon", buf, 128, ini_file);
  choice.font_file = (char *)string_make(buf);
  validate_file(choice.font_file);
  choice.vis_cols = GetPrivateProfileInt("Choice window", "Columns", 80, ini_file);
  choice.vis_rows = GetPrivateProfileInt("Choice window", "Rows", 5, ini_file);
  choice.pos_x = GetPrivateProfileInt("Choice window", "PositionX", 0, ini_file);
  choice.pos_y = GetPrivateProfileInt("Choice window", "PositionY", 0, ini_file);
  choice.cap_size = min(
      GetPrivateProfileInt("Choice window", "CapSize", 0, ini_file),
      127);

  strcpy(buf, screen.font_file);
  p = extract_file_name(buf);
  *(--p) = '\0';
  ANGBAND_DIR_RSRC = (char *)string_make(buf);
  validate_dir(ANGBAND_DIR_RSRC);
}


/*
 *  Loads font file specified in td->font_file, initializes td->font_xxx,
 *  td->size_xxx.
 */
static void term_load_font(term_data *td)
{
  char       fontname[9];
  int        len;
  char       *p, *q, *s, *d;
  int        x, y;

  if (!td->font_file) quit("Bug: font_file == NULL");
  validate_file(td->font_file);

  td->font_id = 0;
  len = strlen(td->font_file);
  q = td->font_file + len - 4;
  if ((len < 5) || (stricmp(q, ".FON") != 0))
    formatted_quit("font_file doesn't end in .FON:\n%s", td->font_file);

  p = extract_file_name(td->font_file);
  if (q - p > 8)
    formatted_quit("Too long filename in font_file:\n%s", td->font_file);

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
	       DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE,
	       fontname);
  }
  else
    formatted_quit("Font file corrupted:\n%s", td->font_file);

  term_getsize(td);
}


#ifdef USE_GRAPHICS
/*
 *  Loads BMP corresponding to font specified in filename, inits infGraph.
 */
static void term_load_bitmap(char *font)
{
  char *p, *q, *s, *d;

  MessageBox(NULL, NULL, font, MB_OK);
  p = (char *)string_make(font);
  q = p + strlen(p) - 4;
  strcpy(q, ".BMP");
  validate_file(p);

  if (infGraph.hDIB) GlobalFree(infGraph.hDIB);
  if (infGraph.hPalette) DeleteObject(infGraph.hPalette);
  if (infGraph.hBitmap) DeleteObject(infGraph.hBitmap);

  if (!ReadDIB(p, &infGraph))
    formatted_quit("Bitmap corrupted:\n%s", p);

  d = extract_file_name(p);

  infGraph.CellWidth = atoi(d);
  infGraph.CellHeight = 0;
  for (s = d; (*s != '\0') && (s < q); s++)
    if (toupper(*s) == 'X')
    {
      infGraph.CellHeight = atoi(s+1);
      break;
    }

  string_free(p);
}
#endif


static void term_window_resize(term_data *td)
{
  RECT  rc;
  POINT pt;

  if (td->type == WTY_SCREEN)
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
      td->size_wid, td->size_hgt, SWP_NOZORDER);
  }
  else
    SetWindowPos(td->w, 0, 0, 0,
      td->size_wid, td->size_hgt, SWP_NOMOVE | SWP_NOZORDER);

  InvalidateRect(td->w, NULL, TRUE);
}


static void term_change_font(term_data *td)
{
  OPENFILENAME ofn;
  char         tmp[128];

  if (td->font_file) strcpy(tmp, extract_file_name(td->font_file));
  else tmp[0] = '\0';

  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = screen.w;
  ofn.lpstrFilter = "Font Files (*.fon)\0*.fon\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = 128;
  ofn.lpstrInitialDir = ANGBAND_DIR_RSRC;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.lpstrDefExt = "fon";

  if (GetOpenFileName(&ofn))
  {
    if (td->font_id) DeleteObject(td->font_id);
    if ((td != &screen) && (stricmp(screen.font_file, td->font_file) == 0) ||
	(td != &recall) && (stricmp(recall.font_file, td->font_file) == 0) ||
	(td != &choice) && (stricmp(choice.font_file, td->font_file) == 0))
      ; /* do nothing !! */
    else RemoveFontResource(td->font_file);
    string_free(td->font_file);
    td->font_file = (char *)string_make(tmp);
    term_load_font(td);
    term_window_resize(td);
  }
}


static void term_change_bitmap(term_data *td)
{
  OPENFILENAME ofn;
  char         tmp[128];

  if (td->font_file)
  {
    strcpy(tmp, extract_file_name(td->font_file));
    strcpy(tmp + strlen(tmp) - 4, ".bmp");
  }
  else tmp[0] = '\0';

  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = screen.w;
  ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = 128;
  ofn.lpstrInitialDir = ANGBAND_DIR_RSRC;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.lpstrDefExt = "bmp";

  if (GetOpenFileName(&ofn))
  {
    term_load_bitmap(tmp);

    if (td->font_id) DeleteObject(td->font_id);
    if ((td != &screen) && (stricmp(screen.font_file, td->font_file) == 0) ||
	(td != &recall) && (stricmp(recall.font_file, td->font_file) == 0) ||
	(td != &choice) && (stricmp(choice.font_file, td->font_file) == 0))
      ; /* do nothing !! */
    else RemoveFontResource(td->font_file);
    string_free(td->font_file);
    strcpy(tmp + strlen(tmp) - 4, ".fon");
    td->font_file = (char *)string_make(tmp);
    term_load_font(td);
    term_window_resize(td);
  }

}


static void term_force_font(term_data *td, char *font_name)
{
  char tmp[128];

  sprintf(tmp, "%s%s%s", ANGBAND_DIR_RSRC, PATH_SEP, font_name);
  validate_file(tmp);

  if (td->font_id) DeleteObject(td->font_id);
  if ((td != &screen) && (stricmp(screen.font_file, td->font_file) == 0) ||
      (td != &recall) && (stricmp(recall.font_file, td->font_file) == 0) ||
      (td != &choice) && (stricmp(choice.font_file, td->font_file) == 0))
    ; /* do nothing !! */
  else RemoveFontResource(td->font_file);
  string_free(td->font_file);
  td->font_file = (char *)string_make(tmp);
  term_load_font(td);
  term_load_bitmap(td->font_file);
  term_window_resize(td);
}


/*** Hooks for the "term.c" functions ***/
/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
    if ((td->type == WTY_SCREEN) && !screen_resizable)
    {
      HPEN hpenOld, hpenLtGray;
      HDC  hdc = GetDC(td->w);

      /* Frame the window in light_gray */
      hpenLtGray = CreatePen(PS_SOLID, 1, win_clr[TERM_L_GRAY]);
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
/*
 * Initialize a new Term
 */
#pragma argsused
static void Term_init_win(term *t)
{
    /* XXX */
}


/*
 * Nuke an old Term
 */
#pragma argsused
static void Term_nuke_win(term *t)
{
    /* XXX */
}


/*
 * React to changing global options
 */
static errr Term_xtra_win_react(void)
{
  DWORD dw;

  /* Show */
  if (use_screen_win && !screen.mapped)
  {
    screen.mapped = TRUE;
    ShowWindow(screen.w, SW_SHOW);
    term_data_redraw(&screen);
  }
  /* Hide */
  if (!use_screen_win && screen.mapped)
  {
    screen.mapped = FALSE;
    ShowWindow(screen.w, SW_HIDE);
  }

  /* Show */
  if (use_recall_win && !recall.mapped)
  {
    recall.mapped = TRUE;
    ShowWindow(recall.w, SW_SHOWNOACTIVATE);
  }
  /* Hide */
  if (!use_recall_win && recall.mapped)
  {
    recall.mapped = FALSE;
    ShowWindow(recall.w, SW_HIDE);
  }

  /* Show */
  if (use_choice_win && !choice.mapped)
  {
    choice.mapped = TRUE;
    ShowWindow(choice.w, SW_SHOWNOACTIVATE);
  }
  /* Hide */
  if (!use_choice_win && choice.mapped)
  {
    choice.mapped = FALSE;
    ShowWindow(choice.w, SW_HIDE);
  }

  /* do something about resizability of main window */
  dw = GetWindowLong(screen.w, GWL_STYLE);
  /* Make it resizable */
  if (screen_resizable && !(dw & WS_THICKFRAME))
  {
    screen.dwStyle = dw | WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL | WS_MAXIMIZEBOX;
    SetWindowLong(screen.w, GWL_STYLE, screen.dwStyle);
    term_getsize(&screen);
    term_window_resize(&screen);
  }
  /* Make it non-resizable */
  if (!screen_resizable && (dw & WS_THICKFRAME))
  {
    screen.dwStyle = dw & ~WS_THICKFRAME & ~WS_VSCROLL & ~WS_HSCROLL & ~WS_MAXIMIZEBOX;
    SetWindowLong(screen.w, GWL_STYLE, screen.dwStyle);
    screen.vis_rows = screen.rows;
    screen.vis_cols = screen.cols;
    screen.scroll_vpos = 0;
    screen.scroll_hpos = 0;
    term_getsize(&screen);
    term_window_resize(&screen);
  }

  return 0;
}


/*
 * Scan for events (do not block)
 */
#pragma argsused
static errr Term_xtra_win_check(int v)
{
    MSG msg;

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      return 0;
    }

    return 1;
}


/*
 * Scan for events (block)
 */
#pragma argsused
static errr Term_xtra_win_event(int v)
{
    MSG msg;

    GetMessage(&msg, NULL, 0, 0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    /* Success */
    return 0;
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_win(int n, int v)
{
    /* Handle a subset of the legal requests */
    switch (n)
    {
       /* Handle "beep" request */
	case TERM_XTRA_NOISE: MessageBeep(-1); return 0;
	case TERM_XTRA_CHECK: return Term_xtra_win_check(v);
	case TERM_XTRA_EVENT: return Term_xtra_win_event(v);
	case TERM_XTRA_REACT: return Term_xtra_win_react();
    }
    /* Oops */
    return 1;
}


/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
#pragma argsused
static errr Term_curs_win(int x, int y, int z)
{
    term_data *td = (term_data*)(Term->data);
    RECT   rc;
    HDC    hdc;

    /* No cursor on recall/choice windows */
    if (td->type != WTY_SCREEN) return 0;

    /* clip to visible part of window */
    x -= td->scroll_hpos;
    y -= td->scroll_vpos;
    if ((x < 0) || (x > td->vis_cols) || (y < 0) || (y > td->vis_rows))
	return 0;

    /* Frame the grid */
    rc.left   = x * td->font_wid + td->size_ow1;
    rc.right  = rc.left + td->font_wid;
    rc.top    = y * td->font_hgt + td->size_oh1;
    rc.bottom = rc.top + td->font_hgt;

    hdc = GetDC(screen.w);
    FrameRect(hdc, &rc, hbrYellow); /* Cursor is done as a yellow "box" */
    ReleaseDC(screen.w, hdc);

    /* Success */
    return 0;
}


/*
 * Low level graphics (Assumes valid input).
 *
 * Erase a "block" of characters starting at (x,y), with size (w,h)
 */
static errr Term_wipe_win(int x, int y, int w, int h)
{
    term_data *td = (term_data*)(Term->data);
    HDC  hdc;
    RECT rc;

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


#ifdef USE_GRAPHICS
#define BM_COLS  64

static u16b char_to_grfx(byte a, byte c)
{
    if (c == 32) return 32;  /* spaces sent with any atributes */
    else return (c + (a-128)*128);
}
#endif


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 * If *s >= 128, draw it all as graphics.
 */
static errr Term_text_win(int x, int y, int n, byte a, const char *s)
{
    term_data *td = (term_data*)(Term->data);
    RECT rc;
    HDC  hdc;
#ifdef USE_GRAPHICS
    HDC hdcMem, hdcSrc;
    HBITMAP hbmSrcOld, hbmMemOld, hbmMem;
    int  scr_x, scr_y, mem_x, i;
    int  w, h;
    u16b g;
#endif

    /* clip to visible part of window */
    x -= td->scroll_hpos;
    y -= td->scroll_vpos;
    if ((x >= (int)td->vis_cols) || (y < 0) || (y >= (int)td->vis_rows))
      return 0;

    if (x < 0)
    {
      if (n <= -x) return 0;
      s += -x;
      n -= -x;
      x = 0;
    }
    n = min(n, td->vis_cols - x);
    if (n <= 0) return 0;

#ifdef USE_GRAPHICS
    if ((a <= 127) || !use_graphics)   /* text output */
    {
#endif
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
#ifdef USE_GRAPHICS
    }
    else        /* graphics output */
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
#endif

    /* Success */
    return 0;
}


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

    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_win;
    t->nuke_hook = Term_nuke_win;
    
    /* Prepare the template hooks */
    t->xtra_hook = Term_xtra_win;
    t->curs_hook = Term_curs_win;
    t->wipe_hook = Term_wipe_win;
    t->text_hook = Term_text_win;

    /* Remember where we came from */
    t->data = (vptr)(td);
    
    /* Activate it */
    //Term_activate(t);
}


/*
 * Read the preference file, Create the windows.
 */
static void init_windows(void)
{
  term_data *td;
  MSG        msg;

  td = &screen;
  WIPE(td, term_data);
  td->s = "Angband 2.7.7";
  td->type = WTY_SCREEN;
  td->keys = 1024;
  td->rows = 24;
  td->cols = 80;
  td->mapped = TRUE;
  td->size_ow1 = 2;
  td->size_ow2 = 2;
  td->size_oh1 = 2;
  td->size_oh2 = 2;

  td = &recall;
  WIPE(td, term_data);
  td->s = "Recall";
  td->type = WTY_RECALL;
  td->keys = 16;
  td->rows = 12;
  td->cols = 80;
  td->mapped = TRUE;
  td->size_ow1 = 1;
  td->size_ow2 = 1;
  td->size_oh1 = 1;
  td->size_oh2 = 1;

  td = &choice;
  WIPE(td, term_data);
  td->s = "Choice";
  td->type = WTY_CHOICE;
  td->keys = 16;
  td->rows = 24;
  td->cols = 80;
  td->mapped = TRUE;
  td->size_ow1 = 1;
  td->size_ow2 = 1;
  td->size_oh1 = 1;
  td->size_oh2 = 1;

  /* Load .INI preferences */
  load_prefs();

  /* Need these before term_getsize gets called */
  screen.dwStyle =
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
  if (screen_resizable)
    screen.dwStyle |= WS_THICKFRAME | WS_VSCROLL | WS_HSCROLL | WS_MAXIMIZEBOX;

  recall.dwStyle =
    IBS_VERTCAPTION | WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL |
    WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
  //if (use_recall_win) recall.dwStyle |= WS_VISIBLE;
  choice.dwStyle =
    IBS_VERTCAPTION | WS_OVERLAPPED | WS_THICKFRAME | WS_VSCROLL |
    WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
  //if (use_choice_win) choice.dwStyle |= WS_VISIBLE;

  /* Load the fonts */
  term_load_font(&screen);
  term_load_font(&recall);
  term_load_font(&choice);
#ifdef USE_GRAPHICS
  if (use_graphics) term_load_bitmap(screen.font_file);
#endif

  /* create two frequently used brushes (for cursor box) */
  hbrRed    = CreateSolidBrush(win_clr[TERM_RED]);
  hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);

  creating = WTY_SCREEN;
  screen.w = CreateWindow(AppName, screen.s, screen.dwStyle,
    screen.pos_x, screen.pos_y, screen.size_wid, screen.size_hgt,
    HWND_DESKTOP, NULL, hInstance, NULL);
  creating = 0;
  if (screen.w == 0) quit("Failed to create Angband window");

  creating = WTY_RECALL;
  recall.w = CreateWindow("AngbandList", recall.s, recall.dwStyle,
    recall.pos_x, recall.pos_y, recall.size_wid, recall.size_hgt,
    HWND_DESKTOP, NULL, hInstance, NULL);
  creating = 0;
  if (recall.w == 0) quit("Failed to create recall window");
  if (recall.cap_size)
    ibSetCaptionSize(recall.w, recall.cap_size);
  if (use_recall_win) ShowWindow(recall.w, SW_SHOWNA);

  creating = WTY_CHOICE;
  choice.w = CreateWindow("AngbandList", choice.s, choice.dwStyle,
    choice.pos_x, choice.pos_y, choice.size_wid, choice.size_hgt,
    HWND_DESKTOP, NULL, hInstance, NULL);
  creating = 0;
  if (choice.w == 0) quit("Failed to create choice window");
  if (choice.cap_size)
    ibSetCaptionSize(choice.w, choice.cap_size);
  if (use_choice_win) ShowWindow(choice.w, SW_SHOWNA);

  /* Link the Choice "term" */
  term_data_link(&choice);
  term_choice = &choice.t;

  /* Link the Recall "term" */
  term_data_link(&recall);
  term_recall = &recall.t;

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
}


/*
 * Delay for "x" seconds
 */
void sleep(unsigned x)
{
    delay(1000 * x);
}


/*
 * disables new and open from file menu
 */
static void disable_start(void)
{
  HMENU hm = GetMenu(screen.w);

  EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}


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

  /* Options/Font/Recall window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_RECALL, MF_BYCOMMAND |
      (use_recall_win ? MF_ENABLED : MF_DISABLED | MF_GRAYED));

  /* Options/Font/Choice window */
  EnableMenuItem(hm, IDM_OPTIONS_FONT_CHOICE, MF_BYCOMMAND |
      (use_choice_win ? MF_ENABLED : MF_DISABLED | MF_GRAYED));

  /* Item "Recall Window" */
  CheckMenuItem(hm, IDM_OPTIONS_RECALL, MF_BYCOMMAND |
      (use_recall_win ? MF_CHECKED : MF_UNCHECKED));

  /* Item "Choice Window" */
  CheckMenuItem(hm, IDM_OPTIONS_CHOICE, MF_BYCOMMAND |
      (use_choice_win ? MF_CHECKED : MF_UNCHECKED));

  /* Item "Main window resizable" */
  CheckMenuItem(hm, IDM_OPTIONS_RESIZABLE, MF_BYCOMMAND |
      (screen_resizable ? MF_CHECKED : MF_UNCHECKED));

#ifdef USE_GRAPHICS
  /* Item "Graphics" */
  CheckMenuItem(hm, IDM_OPTIONS_GRAPHICS, MF_BYCOMMAND |
      (use_graphics ? MF_CHECKED : MF_UNCHECKED));
  EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS, MF_BYCOMMAND |
      (game_in_progress ? MF_ENABLED : MF_DISABLED | MF_GRAYED));
#else
  EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
#endif
}


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
  play_game_mac(FALSE);
}


static void process_menus(WPARAM wParam)
{
  OPENFILENAME ofn;

  switch (wParam)
  {
    case IDM_FILE_NEW: /* new game */
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
        play_game_mac(TRUE);
        quit(NULL);
      }
      break;

    case IDM_FILE_OPEN: /* open game */
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
          play_game_mac(FALSE);
          quit(NULL);
        }
      }
      break;

    case IDM_FILE_SAVE: /* save game */
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

    case IDM_FILE_EXIT: /* save and quit */

      if (game_in_progress && save_enabled)
      {
        /* Save it */
        //validate_file(savefile);
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

    case IDM_FILE_QUIT: /* quit */

      save_enabled = character_generated;

      if (game_in_progress && save_enabled)
      {
        if (IDCANCEL == MessageBox(screen.w,
           "Your character will be not saved!",
           "Warning", MB_ICONEXCLAMATION | MB_OKCANCEL)) break;
      }
      quit(NULL);
      break;

    case IDM_OPTIONS_FONT_ANGBAND:
      if (use_graphics) term_change_bitmap(&screen);
      else term_change_font(&screen);
      break;

    case IDM_OPTIONS_FONT_RECALL:
      term_change_font(&recall);
      break;

    case IDM_OPTIONS_FONT_CHOICE:
      term_change_font(&choice);
      break;

    case IDM_OPTIONS_RECALL:
      use_recall_win = !use_recall_win;
      Term_xtra_win_react();
      break;

    case IDM_OPTIONS_CHOICE:
      use_choice_win = !use_choice_win;
      Term_xtra_win_react();
      break;

    case IDM_OPTIONS_RESIZABLE:
      screen_resizable = !screen_resizable;
      Term_xtra_win_react();
      break;

#ifdef USE_GRAPHICS
    case IDM_OPTIONS_GRAPHICS:
    {
      char buf[120];

      use_graphics = !use_graphics;
      clear_char_remaps();
      if (use_graphics)
      {
          term_force_font(&screen, "8x13b.fon");
	  ANGBAND_SYS = "gfw";
	  //notice_seams = TRUE;
	  view_yellow_lite = FALSE;
          view_bright_lite = FALSE;
      }
      else
	  ANGBAND_SYS = "txw";
      sprintf(buf, "%s%spref-%s.prf", ANGBAND_DIR_PREF, PATH_SEP, ANGBAND_SYS);
      process_pref_file(buf);

      do_cmd_redraw();
      move_cursor_relative(py, px);
      Term_fresh();
      break;
    }
#endif
  }
}


static int process_scrollbar(HWND hWnd, WPARAM wParam, LPARAM lParam, int fnBar)
{
  term_data  *td;
  uint       *scroll_pos;
  int         rows_cols, vis_rows_cols;

  td = (term_data *)GetWindowLong(hWnd, 0);
  switch (fnBar)
  {
    case SB_VERT:
      scroll_pos = &(td->scroll_vpos);
      rows_cols = td->rows;
      vis_rows_cols = td->vis_rows;
      break;
    case SB_HORZ:
      scroll_pos = &(td->scroll_hpos);
      rows_cols = td->cols;
      vis_rows_cols = td->vis_cols;
      break;
    default:
      return 1;
  }

  switch (wParam)
  {
    /* note that SB_TOP == SB_LEFT, SB_BOTTOM == SB_RIGHT etc. */
    case SB_TOP:
      *scroll_pos = 0;
      break;

    case SB_BOTTOM:
      *scroll_pos = rows_cols - vis_rows_cols;
      break;

    case SB_LINEUP:
      if (*scroll_pos > 0) (*scroll_pos)--;
      break;

    case SB_LINEDOWN:
      if (*scroll_pos < rows_cols - vis_rows_cols) (*scroll_pos)++;
      break;

    case SB_PAGEUP:
      *scroll_pos = max((int)*scroll_pos - vis_rows_cols, 0);
      break;

    case SB_PAGEDOWN:
      *scroll_pos = min((int)*scroll_pos + vis_rows_cols, rows_cols - vis_rows_cols);
      break;

    case SB_THUMBPOSITION:
      *scroll_pos = max(min(LOWORD(lParam), rows_cols - vis_rows_cols), 0);
      break;

    default:
      return 1;
  }
  SetScrollPos(hWnd, fnBar, *scroll_pos, TRUE);
  InvalidateRect(hWnd, NULL, TRUE);
  return 0;
}


/* arrow keys translation */
static char TransDir[8] = {'9','3','1','7','4','8','6','2'};

LRESULT FAR PASCAL _export AngbandWndProc(HWND hWnd, UINT uMsg,
  WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT     ps;
  HDC             hdc;
  term_data      *td;
  MINMAXINFO FAR *lpmmi;
  RECT            rc;
  BYTE            KeyState;
  int             i;

  switch (uMsg)
  {
    case WM_NCCREATE:
      if (creating == WTY_SCREEN)
        SetWindowLong(hWnd, 0, (LONG)(&screen));
      return DefWindowProc(hWnd, uMsg, wParam, lParam);

    case WM_CREATE:
      if (screen_resizable)
      {
        td = (term_data *)GetWindowLong(hWnd, 0);
        SetScrollRange(hWnd, SB_VERT, 0, max(td->rows - td->vis_rows, 1), FALSE);
        SetScrollRange(hWnd, SB_HORZ, 0, max(td->cols - td->vis_cols, 1), FALSE);
        return 0;
      }
      else
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO FAR *)lParam;
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;  /* this message was sent before WM_NCCREATE */

      /* minimum window size is 15x3, otherwise have problems with */
      /* menu line wrapping to two lines */
      rc.left = rc.top = 0;
      rc.right = rc.left + 15 * td->font_wid + td->size_ow1 + td->size_ow2;
      rc.bottom = rc.top + 3 * td->font_hgt + td->size_oh1 + td->size_oh2;
      if (screen_resizable)
      {
        rc.right  += GetSystemMetrics(SM_CXVSCROLL) - 1;
        rc.bottom += GetSystemMetrics(SM_CYHSCROLL) - 1;
      }
      AdjustWindowRect(&rc, td->dwStyle, TRUE);
      /* no idea why this needed */
      rc.bottom++;
      lpmmi->ptMinTrackSize.x = rc.right - rc.left;
      lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

      /* maximum window size is td->cols x td->rows */
      rc.left = rc.top = 0;
      rc.right = rc.left + td->cols * td->font_wid + td->size_ow1 + td->size_ow2;
      rc.bottom = rc.top + td->rows * td->font_hgt + td->size_oh1 + td->size_oh2;
      if (screen_resizable)
      {
        rc.right  += GetSystemMetrics(SM_CXVSCROLL) - 1;
        rc.bottom += GetSystemMetrics(SM_CYHSCROLL) - 1;
      }
      AdjustWindowRect(&rc, td->dwStyle, TRUE);
      /* no idea why this needed */
      rc.bottom++;
      lpmmi->ptMaxSize.x      = rc.right - rc.left;
      lpmmi->ptMaxSize.y      = rc.bottom - rc.top;
      lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
      lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

      return 0;

    case WM_PAINT:
      td = (term_data *)GetWindowLong(hWnd, 0);
      BeginPaint(hWnd, &ps);
      Term = &td->t;
      term_data_redraw(td);
      Term = term_screen;
      EndPaint(hWnd, &ps);
      ValidateRect(hWnd, NULL);   /* why needed ?? */
      return 0;

    case WM_VSCROLL:
      return process_scrollbar(hWnd, wParam, lParam, SB_VERT);

    case WM_HSCROLL:
      return process_scrollbar(hWnd, wParam, lParam, SB_HORZ);

    case WM_KEYDOWN:
      KeyState = 0x00;
      if (GetKeyState(VK_SHIFT)   & 0x8000) KeyState |= 0x01;
      if (GetKeyState(VK_CONTROL) & 0x8000) KeyState |= 0x02;
      if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN))
      {
        if (!KeyState)
        {
	  /* map plain keypad or arrow keys to corresp. numbers */
          Term_keypress(TransDir[wParam - VK_PRIOR]);
          return 0;
        }
        else goto enhanced;
      }
      else if ((wParam >= VK_F1) && (wParam <= VK_F12) ||
               (wParam == VK_INSERT) || (wParam == VK_DELETE))
      {
enhanced:
        /* Hack -- a special "macro introducer" */
        Term_keypress(31);
        
        /* Send the modifiers */
        if (KeyState & 0x01) Term_keypress('S');
        if (KeyState & 0x02) Term_keypress('C');

        /* The bits 16..23 in lParam are the scancode */
        i = LOBYTE(HIWORD(lParam));

        /* Hack -- encode the keypress (in decimal) */
        Term_keypress('0' + (i % 1000) / 100);
        Term_keypress('0' + (i % 100) / 10);
        Term_keypress('0' + (i % 10));

        /* End the macro with "return" */
        Term_keypress(13);

        return 0;
      }
      else return DefWindowProc(hWnd, uMsg, wParam, lParam);

    case WM_CHAR:
      Term_keypress(wParam);
      return 0;

    case WM_INITMENU:
      setup_menus();
      return 0;

    case WM_CLOSE:
    case WM_QUIT:
      quit(NULL);
      return 0;

    case WM_COMMAND:
      process_menus(wParam);
      return 0;

    case WM_SIZE:
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;    /* this message was sent before WM_NCCREATE */
      if (!td->w) return 1; /* it was sent from inside CreateWindow */

      switch (wParam)
      {
        case SIZE_MINIMIZED:
          if (use_recall_win) ShowWindow(recall.w, SW_HIDE);
          if (use_choice_win) ShowWindow(choice.w, SW_HIDE);
          return 0;

        case SIZE_MAXIMIZED:
          td->vis_cols = td->cols;
          td->vis_rows = td->rows;
          /* fall through!!! */

        case SIZE_RESTORED:
          if (screen_resizable)
          {
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
          if (use_recall_win) ShowWindow(recall.w, SW_SHOWNOACTIVATE);
          if (use_choice_win) ShowWindow(choice.w, SW_SHOWNOACTIVATE);
          return 0;

        default:
           return DefWindowProc(hWnd, uMsg, wParam, lParam);
      }

    case WM_PALETTECHANGED:
      if (!paletted)
	return ibDefWindowProc(hWnd, uMsg, wParam, lParam); /* ignore */
      /* also ignore if palette change caused by itself */
      else if ((HWND)wParam == hWnd) return 0;

      /* otherwise, fall through!!! */

    case WM_QUERYNEWPALETTE:
      if (!paletted)
	return ibDefWindowProc(hWnd, uMsg, wParam, lParam); /* ignore */
      hdc = GetDC(hWnd);
      SelectPalette (hdc, hPal, FALSE);
      i = RealizePalette(hdc);
      ReleaseDC(hWnd, hdc);

      /* if any palette entries changed, repaint the window. */
      if (i > 0) InvalidateRect(hWnd, NULL, TRUE);
      return i;

    case WM_ACTIVATE:
      if (wParam && !HIWORD(lParam))
      {
	SetWindowPos(recall.w, hWnd, 0, 0, 0, 0, SWP_NOACTIVATE |
	  SWP_NOMOVE | SWP_NOSIZE);
	SetWindowPos(choice.w, hWnd, 0, 0, 0, 0, SWP_NOACTIVATE |
	  SWP_NOMOVE | SWP_NOSIZE);
	SetFocus(hWnd);
	return 0;
      }
      /* fall through */

    default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
      if (creating == WTY_RECALL)
	SetWindowLong(hWnd, 0, (LONG)(&recall));
      else if (creating == WTY_CHOICE)
	SetWindowLong(hWnd, 0, (LONG)(&choice));
      return ibDefWindowProc(hWnd, uMsg, wParam, lParam);

    case WM_CREATE:
      td = (term_data *)GetWindowLong(hWnd, 0);
      SetScrollRange(hWnd, SB_VERT, 0, td->rows - td->vis_rows, FALSE);
      return 0;

    case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO FAR *)lParam;
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;  /* this message was sent before WM_NCCREATE */

      /* minimum window size is 5x3 */
      rc.left = rc.top = 0;
      rc.right = rc.left + 5 * td->font_wid + td->size_ow1 + td->size_ow2 +
                 GetSystemMetrics(SM_CXVSCROLL) - 1;
      rc.bottom = rc.top + 3 * td->font_hgt + td->size_oh1 + td->size_oh2;
      ibAdjustWindowRect(&rc, td->dwStyle, FALSE, td->cap_size);
      lpmmi->ptMinTrackSize.x = rc.right - rc.left;
      lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

      /* maximum window size is td->cols x td->rows */
      rc.left = rc.top = 0;
      rc.right = rc.left + td->cols * td->font_wid + td->size_ow1 + td->size_ow2 +
                 GetSystemMetrics(SM_CXVSCROLL) - 1;
      rc.bottom = rc.top + td->rows * td->font_hgt + td->size_oh1 + td->size_oh2;
      ibAdjustWindowRect(&rc, td->dwStyle, FALSE, td->cap_size);
      lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
      lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

      return 0;

    case WM_SIZE:
      td = (term_data *)GetWindowLong(hWnd, 0);
      if (!td) return 1;    /* this message was sent before WM_NCCREATE */
      if (!td->w) return 1; /* it was sent from inside CreateWindow */
      td->vis_cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->font_wid;
      td->vis_rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->font_hgt;
      td->scroll_vpos = 0;
      td->scroll_hpos = 0;
      SetScrollRange(hWnd, SB_VERT, 0, td->rows - td->vis_rows, FALSE);
      term_getsize(td);
      MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, TRUE);
      return 0;

    case WM_PAINT:
      td = (term_data *)GetWindowLong(hWnd, 0);
      Term = &td->t;
      BeginPaint(hWnd, &ps);
      Term_redraw();
      EndPaint(hWnd, &ps);
      Term = &screen.t;
      return 0;

    case WM_VSCROLL:
      return process_scrollbar(hWnd, wParam, lParam, SB_VERT);

    case WM_PALETTECHANGED:
      if (!paletted)
	return ibDefWindowProc(hWnd, uMsg, wParam, lParam); /* ignore */
      /* also ignore if palette change caused by itself */
      else if ((HWND)wParam == hWnd) return 0;

      /* otherwise, fall through!!! */

    case WM_QUERYNEWPALETTE:
      if (!paletted)
	return ibDefWindowProc(hWnd, uMsg, wParam, lParam); /* ignore */
      hdc = GetDC(hWnd);
      SelectPalette (hdc, hPal, FALSE);
      i = RealizePalette(hdc);
      ReleaseDC(hWnd, hdc);

      /* if any palette entries changed, repaint the window. */
      if (i > 0) InvalidateRect(hWnd, NULL, TRUE);
      return i;

    case WM_SYSCOMMAND:
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

    case WM_NCLBUTTONDOWN:
      if (wParam == HTSYSMENU)
      {
          td = (term_data *)GetWindowLong(hWnd, 0);
          if (td->type == WTY_RECALL) use_recall_win = FALSE;
          else if (td->type == WTY_CHOICE) use_choice_win = FALSE;
          Term_xtra_win_react();
          return 0;
      }
    /* fall through */

    default:
      return ibDefWindowProc(hWnd, uMsg, wParam, lParam);
  }
}


/*
 * Reads ANGBAND.INI, gets lib_path, checks for existence,
 * called by get_file_paths @INIT.C
 */
void get_lib_path(char *path)
{
  int   i;

  GetPrivateProfileString("Angband", "LibPath", "c:\\angband\\lib", path, 128, ini_file);

  /* subtract \\ from the end if present */
  if ((i = strlen(path)) == 0)
      quit("LibPath shouldn't be empty in ANGBAND.INI");
  if (path[i-1] == '\\') path[i-1] = '\0';
  else i++;

  validate_dir(path);

  /* add \ back */
  path[i-1] = '\\';
  path[i] = '\0';
}


/*** Some Hooks for various routines ***/
/*
 * See "z-virt.c"
 */
static vptr hook_ralloc(huge size)
{
    if (size > 0xFFFF)
	quit("Tried to malloc more than 64K");
    return (vptr)malloc((size_t)size);
}


#pragma argsused
static errr hook_rnfree(vptr v, huge size)
{
    free(v);
    return 0;
}


/*
 * See "z-util.c"
 */
static void hook_plog(cptr str)
{
    MessageBox(screen.w, str, "Warning", MB_OK);
}


static void hook_quit(cptr str)
{
    if (str)
	MessageBox(screen.w, str, "Error", MB_OK | MB_ICONSTOP);

    /* Oh yeah, close the high score list */
    nuke_scorefile();

    save_prefs();

    if (recall.w)
    {
        DestroyWindow(recall.w);
        recall.w = 0;
    }
    if (choice.w)
    {
        DestroyWindow(choice.w);
        choice.w = 0;
    }
    if (screen.w)
    {
        DestroyWindow(screen.w);
        screen.w = 0;
    }

    DeleteObject(hbrRed);
    DeleteObject(hbrYellow);
    if (hPal) DeleteObject(hPal);
#ifdef USE_GRAPHICS
    if (infGraph.hDIB) GlobalFree(infGraph.hDIB);
    if (infGraph.hPalette) DeleteObject(infGraph.hPalette);
    if (infGraph.hBitmap) DeleteObject(infGraph.hBitmap);
#endif

    if (screen.font_id) DeleteObject(screen.font_id);
    if (screen.font_file)
    {
      RemoveFontResource(screen.font_file);
      string_free(screen.font_file);
    }
    if (recall.font_id) DeleteObject(recall.font_id);
    if (recall.font_file)
    {
      RemoveFontResource(recall.font_file);
      string_free(recall.font_file);
    }
    if (choice.font_id) DeleteObject(choice.font_id);
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
    if (str)
      MessageBox(screen.w, str, "Error", MB_OK);

    MessageBox(screen.w, "I will now attempt to save and quit.",
      "Fatal error", MB_OK | MB_ICONSTOP);
    save_player();
    quit(NULL);
}


#pragma argsused
int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
  LPSTR lpCmdLine, int nCmdShow)
{
  WNDCLASS wc;
  HDC      hdc;
  MSG      msg;
  char    *tmp;

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
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wc.lpszMenuName  = AppName;
    wc.lpszClassName = AppName;

    if (!RegisterClass(&wc)) exit(1);

    wc.lpfnWndProc   = AngbandListProc;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "AngbandList";

    if (!RegisterClass(&wc)) exit(2);
  }

  /* Hook in some "z-virt.c" hooks */
  ralloc_aux = hook_ralloc;
  rnfree_aux = hook_rnfree;

  /* Hooks in some "z-util.c" hooks */
  plog_aux = hook_plog;
  quit_aux = hook_quit;
  core_aux = hook_core;

  /* get ANGBAND.INI full path */
  tmp = (char *)ralloc(512);
  GetModuleFileName(hInstance, tmp, 512);
  strcpy(tmp + strlen(tmp) - 4, ".INI");
  ini_file = string_make(tmp);
  rnfree(tmp, 512);
  validate_file((char *)ini_file);

  /* Prepare the filepaths */
  get_file_paths();

  /* validate everything */
  validate_dir((char *)ANGBAND_DIR_FILE);
  validate_dir((char *)ANGBAND_DIR_HELP);
  validate_dir((char *)ANGBAND_DIR_BONE);
  validate_dir((char *)ANGBAND_DIR_DATA);
  validate_dir((char *)ANGBAND_DIR_SAVE);
  validate_dir((char *)ANGBAND_DIR_PREF);
  validate_file((char *)ANGBAND_NEWS);

  /* Prepare the windows */
  init_windows();

  /* Determine if display is 16/256/true color */
  hdc = GetDC(NULL);
  colors16 = GetDeviceCaps(hdc, BITSPIXEL) == 4;
  paletted =
    (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE;
  ReleaseDC(NULL, hdc);

  /* If 256 colors, create/realize palette to use with 16 entries */
  if (paletted)
  {
    int          i;
    LPLOGPALETTE pLogPal =
      (LPLOGPALETTE)ralloc(sizeof(LOGPALETTE) + 16*sizeof(PALETTEENTRY));

    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = 16;
    for (i = 0; i < 16; i++)
    {
      pLogPal->palPalEntry[i].peRed = GetRValue(win_clr[i]);
      pLogPal->palPalEntry[i].peGreen = GetGValue(win_clr[i]);
      pLogPal->palPalEntry[i].peBlue = GetBValue(win_clr[i]);
      pLogPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
    }
    if (!(hPal = CreatePalette((LPLOGPALETTE) pLogPal)))
      quit("Cannot create palette");
    rnfree(pLogPal, 1);

    hdc = GetDC(screen.w);
    SelectPalette(hdc, hPal, 0);
    i = RealizePalette(hdc);
    ReleaseDC(screen.w, hdc);
    if (i == 0) quit("Cannot realize palette");

    hdc = GetDC(recall.w);
    SelectPalette(hdc, hPal, 0);
    //i = RealizePalette(hdc);
    ReleaseDC(recall.w, hdc);
    //if (i == 0) quit("Cannot realize palette");

    hdc = GetDC(choice.w);
    SelectPalette(hdc, hPal, 0);
    //i = RealizePalette(hdc);
    ReleaseDC(choice.w, hdc);
    //if (i == 0) quit("Cannot realize palette");
  }

#ifdef COLOR_DEBUG
  prt("Screen: ", 1, 1);
  if (paletted) prt("paletted,", 1, 10);
  else prt("not paletted,", 1, 10);
  if (colors16) prt("16 colors", 1, 24);
  else prt(">16 colors", 1, 24);

  /* Debugging hack: check that all colors work */
  c_prt(TERM_BLACK,   "Black",        3, 1);
  c_prt(TERM_WHITE,   "White",        4, 1);
  c_prt(TERM_GRAY,    "Gray",         5, 1);
  c_prt(TERM_ORANGE,  "Orange",       6, 1);
  c_prt(TERM_RED,     "Red",          7, 1);
  c_prt(TERM_GREEN,   "Green",        8, 1);
  c_prt(TERM_BLUE,    "Blue",         9, 1);
  c_prt(TERM_UMBER,   "Umber",       10, 1);
  c_prt(TERM_D_GRAY,  "Dark Gray",   11, 1);
  c_prt(TERM_L_GRAY,  "Light Gray",  12, 1);
  c_prt(TERM_VIOLET,  "Violet",      13, 1);
  c_prt(TERM_YELLOW,  "Yellow",      14, 1);
  c_prt(TERM_L_RED,   "Light Red",   15, 1);
  c_prt(TERM_L_GREEN, "Light Green", 16, 1);
  c_prt(TERM_L_BLUE,  "Light Blue",  17, 1);
  c_prt(TERM_L_UMBER, "Light Umber", 18, 1);
  inkey();
#endif

  /* Get a pointer to the high score file */
  init_scorefile();

  /* Display the "news" message */
  Term_activate(term_screen);
  show_news();

  /* Allocate and Initialize various arrays */
  init_some_arrays();

  /* We are now initialized */
  initialized = TRUE;
    
  /* Did the user double click on a save file? */
  check_for_save_file(lpCmdLine);

  /* Prompt the user */
  Term_activate(term_screen);
  prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
  Term_fresh();

  /* Process messages (until "play_game_mac()" is called) */
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  quit(NULL);
  return 0;
}

#endif /* _Windows */

