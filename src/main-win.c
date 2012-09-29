/* File: main-win.c */

#ifdef _Windows

# define STRICT
# include <windows.h>
# include <commdlg.h>

#include "angband.h"

/* string.h excludes this because __STDC__=1 */
int stricmp(const char *, const char *);

/* same here */
void        _cdecl farfree(void far *);
void far  * _cdecl farmalloc(unsigned long);

#define IDM_FILE_NEW     101
#define IDM_FILE_OPEN    102
#define IDM_FILE_SAVE    103
#define IDM_FILE_EXIT    104
#define IDM_FILE_QUIT    105
#define IDM_OPTIONS_FONT 201

static int game_in_progress=0;
static int save_enabled;

static HWND   hwndMain    = 0;
static HINSTANCE hInstance= 0;
static WNDCLASS wc;
static HFONT hfMain       = 0;

static HBRUSH hbrYellow   = 0;
static HBRUSH hbrRed      = 0;

static int    iFontWidth  = 0;
static int    iFontHeight = 0;

static char ANGBAND_FONT[128];
static const char *AppName = "ANGBAND";

static const COLORREF far win_clr[16] = {
    RGB(0x00,0x00,0x00),        /* 15 Black */
    RGB(0xFF,0xFF,0xFF),        /* 0 White */
    RGB(0x80,0x80,0x80),        /* 13 Gray */
    RGB(0xFF,0x64,0x02),        /* 2 Orange */
    RGB(0xE0,0x08,0x06),        /* 3 Red */
    RGB(0x00,0x62,0x10),        /* 9 Green */
    RGB(0x00,0x00,0xD0),        /* 6 Blue */
    RGB(0x56,0x2C,0x06),        /* 10 Brown */
    RGB(0x40,0x40,0x40),        /* 14 Dark Gray */
    RGB(0xC0,0xC0,0xC0),        /* 12 Light Gray */
    RGB(0x46,0x00,0xA3),        /* 5 Purple */
    RGB(0xF8,0xF0,0x04),        /* 1 Yellow */
    RGB(0xF2,0x08,0x84),        /* 4 Pink */
    RGB(0x20,0xB8,0x14),        /* 8 Light Green */
    RGB(0x02,0xA0,0xE8),        /* 7 Light Blue */
    RGB(0x90,0x70,0x39)         /* 11 Light Brown */
};


/*
 * Delay for "x" milliseconds
 */
void delay(int x)
{
    DWORD t; t=GetTickCount(); while(GetTickCount()<t+x);
}

/*
 * disables new and open from file menu
 */
static void disable_start(void)
{
  HMENU hm = GetMenu(hwndMain);

  EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}

static void setup_menus(void)
{
  HMENU hm = GetMenu(hwndMain);

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
}

static void check_for_save_file(void)
{
  /*** to implement ***/
}


/*
 *  Loads font file specified in ANGBAND_FONT,
 *  initializes iFontWidth, iFontHeight
 */
static void init_angband_font(void)
{
  HFONT      hfOld;
  HDC        hdcDesktop;   /* use desktop DC since we call it once
                              before the main windows is created */
  TEXTMETRIC tm;
  char       fontname[9];
  int        len;
  char       *p, *q, *s, *d;
  int        x, y;

  len = strlen(ANGBAND_FONT);
  q = ANGBAND_FONT + len - 4;
  if ((len < 5) || (stricmp(q, ".FON") != 0))
  {
    ANGBAND_FONT[0] = '\0';
    hfMain = 0;
  }
  else
  {
    /* derive fontname from ANGBAND_FONT */
    for (p = q; (p >= ANGBAND_FONT)&&(*p != ':')&&(*p != '\\'); p--);
    p++;
    if (q - p > 8)
    {
      ANGBAND_FONT[0] = '\0';
      hfMain = 0;
    }
    else
    {
      x = atoi(p);
      y = 0;
      for (s = p, d = fontname; (*s != '\0') && (s < q);)
      {
        if (toupper(*s) == 'X') y = atoi(s+1);
        *(d++) = toupper(*(s++));
      }
      *d = '\0';

      if (AddFontResource(ANGBAND_FONT))
      {
        hfMain = CreateFont(y, x, 0, 0, FW_DONTCARE, 0, 0, 0,
                   ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                   DEFAULT_QUALITY, FIXED_PITCH | FF_DONTCARE,
                   fontname);
      }
    }
  }

  if (!hfMain) hfMain = GetStockObject(ANSI_FIXED_FONT);

  /* all this trouble to get the cell size*/
  hdcDesktop = GetDC(HWND_DESKTOP);
  hfOld = SelectObject(hdcDesktop, hfMain);
  GetTextMetrics(hdcDesktop, &tm);
  SelectObject(hdcDesktop, hfOld);
  ReleaseDC(HWND_DESKTOP, hdcDesktop);
  iFontWidth  = tm.tmAveCharWidth;
  iFontHeight = tm.tmHeight;
}

void process_menus(WPARAM wParam)
{
  OPENFILENAME ofn;

  switch (wParam)
  {
    case IDM_FILE_NEW: /* new game */
      if (game_in_progress)
      {
	MessageBox(hwndMain,
           "You can't start a new game while you're still playing!",
           "Warning", MB_ICONEXCLAMATION | MB_OK);
      }
      else
      {
	game_in_progress=1;
	disable_start();
	Term_flush();
	play_game_mac(TRUE);
	main_exit();
      }
      break;

    case IDM_FILE_OPEN: /* open game */
      if (game_in_progress)
      {
	MessageBox(hwndMain,
           "You can't open a new game while you're still playing!",
           "Warning", MB_ICONEXCLAMATION | MB_OK);
      }
      else
      {
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwndMain;
        ofn.hInstance = hInstance;
        ofn.lpstrFilter = "Save Files (*.)\0*.\0";
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter = 0;
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = savefile;
        ofn.nMaxFile = 1024;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = ANGBAND_DIR_SAVE;
        ofn.lpstrTitle = NULL;
        ofn.Flags = OFN_FILEMUSTEXIST;
        ofn.lpstrDefExt = NULL;

	if (GetOpenFileName(&ofn))
	{
	  /* Load 'savefile' */
	  game_in_progress=1;
	  disable_start();
	  Term_flush();
	  play_game_mac(FALSE);
	  main_exit();
	}
      }
      break;

    case IDM_FILE_SAVE: /* save game */
      if (game_in_progress)
      {
	Term_fresh();

	if (total_winner)
	{
	  msg_print("You are a Total Winner, your character must be retired.");
	  if (rogue_like_commands)
	  {
	    msg_print("Use 'Q' to when you are ready to retire.");
	  }
	  else
	  {
	    msg_print("Use <Control>-K when you are ready to retire.");
	  }
	}
	else
	{
          /* The player is not dead */
          (void)strcpy(died_from, "(saved)");

          /* Save the player, note the result */
          prt("Saving game...",0,0);
          if (save_player()) prt("done.",0,14);
          else prt("Save failed.",0,14);
          Term_fresh();

          /* Forget that the player was saved */
          character_saved = 0;

          /* Hilite the player */
          move_cursor_relative(char_row,char_col);

          /* Note that the player is not dead */
          (void)strcpy(died_from, "(alive and well)");
        }
      }
      break;

    case IDM_FILE_EXIT: /* save and quit */

      save_enabled = character_generated;
  
      if (game_in_progress && save_enabled)
      {
	if (total_winner)
	{
	  msg_print("You are a Total Winner,        your character must be retired.");
	  if (rogue_like_commands)
	      msg_print("Use 'Q' to when you are ready to retire.");
	  else
	      msg_print("Use <Control>-K when you are ready to retire.");
	}
	else
	{
	  (void)strcpy(died_from, "(saved)");
	  prt("Saving game...",0,0);
	  if (save_player()) main_exit();
	  prt("Save failed.",0,14);
	  move_cursor_relative(char_row,char_col);
	  (void)strcpy(died_from, "(alive and well)");
	}
      }
      else
      {
	main_exit();
      }
      break;

    case IDM_FILE_QUIT: /* quit */

      save_enabled = character_generated;
  
      if (game_in_progress && save_enabled)
      {
	if (IDCANCEL == MessageBox(hwndMain,
           "Your character will be not saved!",
           "Warning", MB_ICONEXCLAMATION | MB_OKCANCEL)) break;
      }
      main_exit();
      break;

    case IDM_OPTIONS_FONT:
      if (ANGBAND_FONT[0] != '\0')
        RemoveFontResource(ANGBAND_FONT);

      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hwndOwner = hwndMain;
      ofn.hInstance = hInstance;
      ofn.lpstrFilter = "Font Files (*.fon)\0*.fon\0";
      ofn.lpstrCustomFilter = NULL;
      ofn.nMaxCustFilter = 0;
      ofn.nFilterIndex = 1;
      ofn.lpstrFile = ANGBAND_FONT;
      ofn.nMaxFile = 128;
      ofn.lpstrFileTitle = NULL;
      ofn.nMaxFileTitle = 0;
      ofn.lpstrInitialDir = ANGBAND_DIR_DATA;
      ofn.lpstrTitle = NULL;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
      ofn.lpstrDefExt = "fon";

      if (GetOpenFileName(&ofn))
      {
        RECT  rc;
        POINT pt, ptNewSize;
        char  ini_file[256];

        /* save font file path in ANGBAND.INI */
        GetModuleFileName(hInstance, ini_file, 256);
        strcpy(ini_file + strlen(ini_file) - 4, ".INI");
        WritePrivateProfileString("Angband", "FontFile", ANGBAND_FONT,
          ini_file);

        init_angband_font();

        /* get old window center */
        GetWindowRect(hwndMain, &rc);
        pt.x = (rc.left + rc.right) / 2;
        pt.y = (rc.top  + rc.bottom) / 2;

        ptNewSize.x = 80 * iFontWidth + 2 * GetSystemMetrics(SM_CXBORDER);
        ptNewSize.y = 24 * iFontHeight + 2 * GetSystemMetrics(SM_CYBORDER) +
                      GetSystemMetrics(SM_CYCAPTION) +
                      GetSystemMetrics(SM_CYMENU);
        /* determine left top corner, adjust it */
        pt.x -= ptNewSize.x / 2;
        pt.y -= ptNewSize.y / 2;
        if (pt.x < 0) pt.x = 0;
        if (pt.y < 0) pt.y = 0;

        SetWindowPos(hwndMain, 0, pt.x, pt.y, ptNewSize.x, ptNewSize.y,
          SWP_NOZORDER);
        InvalidateRect(hwndMain, NULL, TRUE);
      }
      break;
  }
}

/* arrow keys translation */
static char TransDirOrig[8] = {'9','3','1','7','4','8','6','2'};
static char TransDirRogue[8]= {'u','n','b','y','h','k','l','j'};

LRESULT FAR PASCAL _export AngbandWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  MINMAXINFO FAR *lpmmi;
  HDC             hdc;
  RECT            rc;
  int             rx1, rx2, ry1, ry2;
  PAINTSTRUCT     ps;
  char            ch;

  switch (uMsg)
  {
    case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO FAR*) lParam;
      lpmmi->ptMaxSize.x = 80 * iFontWidth +
              2 * GetSystemMetrics(SM_CXBORDER);
      lpmmi->ptMaxSize.y = 24 * iFontHeight +
              2 * GetSystemMetrics(SM_CYBORDER) +
              GetSystemMetrics(SM_CYCAPTION) +
              GetSystemMetrics(SM_CYMENU);
      lpmmi->ptMinTrackSize = lpmmi->ptMaxSize;
      lpmmi->ptMaxTrackSize = lpmmi->ptMaxSize;
      return 0;

    case WM_ERASEBKGND:
      hdc = (HDC)wParam;
      GetClipBox(hdc, &rc);
      SetBkColor(hdc, RGB(0,0,0));
      ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
      return 1;

    case WM_PAINT:
      hdc = BeginPaint(hwndMain, &ps);

#if 0
      rx1 = ps.rcPaint.left / iFontWidth;
      rx2 = ps.rcPaint.right / iFontWidth + 1;
      ry1 = ps.rcPaint.top / iFontHeight;
      ry2 = ps.rcPaint.bottom / iFontHeight + 1;
      Term_redraw_rect(rx1, rx2, ry1, ry2);
#endif

      /* Flush and redraw the "whole" window */
      Term_redraw();

      EndPaint(hwndMain, &ps);
      return 0;

    case WM_KEYDOWN:
      if ((wParam >= VK_PRIOR) && (wParam <= VK_DOWN))
        ch = rogue_like_commands ? TransDirRogue[wParam - VK_PRIOR]
                                 : TransDirOrig [wParam - VK_PRIOR];
      else return(DefWindowProc(hWnd, uMsg, wParam, lParam));
      Term_keypress(ch);
      return 0;

    case WM_CHAR:
      Term_keypress(wParam);
      return 0;

/** debug **/
#if 0
    case WM_SYSCHAR:
    case WM_SYSCOMMAND:
    {
      char buf[80];

      wsprintf(buf, "wParam = $%04X\nlParam = $%04lX", wParam, lParam);
      MessageBox(0, buf, (uMsg == WM_SYSCHAR) ? "WM_SYSCHAR" : "WM_SYSCOMMAND", MB_OK);
      return(DefWindowProc(hWnd, uMsg, wParam, lParam));
    }
#endif
/** debug **/

    case WM_INITMENU:
      setup_menus();
      return 0;

    case WM_QUIT:
      main_exit();
      return 0;

    case WM_COMMAND:
      process_menus(wParam);
      return 0;

    default:
      return(DefWindowProc(hWnd, uMsg, wParam, lParam));
  }
}


/*** Some Hooks for various routines ***/

/*
 * See "z-util.c" and "z-virt.c"
 */

static vptr hook_ralloc(hugt size)
{
    return (farmalloc(size));
}

static errr hook_rnfree(vptr v, hugt size)
{
    farfree(v);
    return (0);
}

static void hook_plog(cptr str)
{
    MessageBox(hwndMain, str, "Warning", MB_OK);
}

static void hook_quit(cptr str)
{
    if (str)
      MessageBox(hwndMain, str, "Error", MB_OK | MB_ICONSTOP);
    main_exit();
}

static void hook_core(cptr str)
{
    if (str)
      MessageBox(hwndMain, str, "Error", MB_OK);

    MessageBox(hwndMain,
      "I will now attempt to save and quit.",
      "Fatal error", MB_OK | MB_ICONSTOP);
    save_player();
    main_exit();
}


#if 0
/*** Hooks for the "recall" and "choice" windows ***/

/*
 * Flush the recall (no action needed)
 */
static void hook_recall_fresh()
{
  /* not implemented */
}


/*
 * Clear the recall window
 */
static void hook_recall_clear()
{
  /* not implemented */
}


/*
 * Print something in the recall window
 */
static void hook_recall_print(int x, int y, byte a, cptr s)
{
  /* not implemented */
}


/*
 * Flush the choice (no action needed)
 */
static void hook_choice_fresh()
{
  /* not implemented */
}


/*
 * Clear the choice window
 */
static void hook_choice_clear()
{
  /* not implemented */
}


/*
 * Print something in the choice window
 */
static void hook_choice_print(int x, int y, byte a, cptr s)
{
  /* not implemented */
}
#endif


int access(const char *filename, int amode);

/*
 * Reads ANGBAND.INI, gets DEFAULT_PATH, ANGBAND_FONT
 * called by get_file_paths @ARRAY.C
 */
void get_ini_options(char *default_path)
{
  char ini_file[256];

  /* get ANGBAND.INI full path */
  GetModuleFileName(hInstance, ini_file, 256);
  strcpy(ini_file + strlen(ini_file) - 4, ".INI");

  if (access(ini_file, 0) == -1)
  {
    hook_quit("Cannot find file ANGBAND.INI");
  }
  else
  {
    GetPrivateProfileString("Angband", "DefaultPath", ".\\lib",
      default_path, 1024, ini_file);
    GetPrivateProfileString("Angband", "FontFile", ".\\lib\\data\\7x13.fon",
      ANGBAND_FONT, 128, ini_file);
  }
}


/*** Functions needed by "Term" ***/

/*
 * Low level graphics.  Assumes valid input.
 * Erase a block of the screen starting at (x,y), with size (w,h)
 */
static void Term_wipe_win(int x, int y, int w, int h)
{
    HDC  hdc;
    RECT rc;

    /* Erase the character grid */
    rc.left   = x * iFontWidth;
    rc.right  = rc.left + w * iFontWidth;
    rc.top    = y * iFontHeight;
    rc.bottom = rc.top + h * iFontHeight;

    hdc = GetDC(hwndMain);
    SetBkColor(hdc, RGB(0,0,0));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, "", 0, NULL);
    ReleaseDC(hwndMain, hdc);
}


/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_grab()" to determine
 * the current screen contents (for inverting, etc).
 */
#pragma argsused
static void Term_curs_win(int x, int y, int z)
{
    RECT   rc;
    HBRUSH hbr;
    HDC    hdc;

    /* Verify the cursor */
    if ((x < 0) || (y < 0) || (x >= 80) || (y >= 24))
    {
      x = 0; y = 0;
      hbr = hbrRed;
    }
    else
      /* Cursor is done as a yellow "box" */
      hbr = hbrYellow;

    /* Frame the grid */
    rc.left   = x * iFontWidth;
    rc.right  = rc.left + iFontWidth;
    rc.top    = y * iFontHeight;
    rc.bottom = rc.top + iFontHeight;

    hdc = GetDC(hwndMain);
    FrameRect(hdc, &rc, hbr);
    ReleaseDC(hwndMain, hdc);
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 */
static void Term_text_win(int x, int y, int n, byte a, const char *s)
{
    RECT rc;
    HDC  hdc;

    /* Stop illegal attributes */
    if ((a == 0) || (a >= 16)) a = COLOR_RED;

    rc.left = x * iFontWidth;
    rc.right  = rc.left + n * iFontWidth;
    rc.top    = y * iFontHeight;
    rc.bottom = rc.top + iFontHeight;

    /* Draw the string (will only work for mono-spaced fonts) */
    hdc = GetDC(hwndMain);
    SetBkColor(hdc, RGB(0,0,0));
    SetTextColor(hdc, win_clr[a]);
    SelectObject(hdc, hfMain);
    ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
               s, n, NULL);
    ReleaseDC(hwndMain, hdc);
}


/*
 * Scan for events
 */
static void Term_scan_win(int n)
{
  MSG  msg;

  if (n == 0) return;
  if (n > 0)
  {
    do
    {
      GetMessage(&msg, hwndMain, 0, 0);
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      n--;
    }  while (n > 0);
  }
  else
  {
    while (PeekMessage(&msg, hwndMain, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}


/*
 * Handle a "special request"
 */
static void Term_xtra_win(int n)
{
    /* Handle a subset of the legal requests */
    switch (n)
    {
        /* Make a noise */
        case TERM_XTRA_NOISE: MessageBeep(-1); break;
    }
}



#pragma argsused
int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  MSG        msg;

  hInstance = hInst;  /* save in a global var */

  if (hPrevInst == NULL)
  {
    wc.style         = 0;
    wc.lpfnWndProc   = AngbandWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst, AppName);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = AppName;
    wc.lpszClassName = AppName;

    if (!RegisterClass(&wc)) goto exit;
  }

#ifdef GRAPHIC_RECALL
  /* Hook in some "GRAPHIC_RECALL" functions */
  recall_fresh_hook = hook_recall_fresh;
  recall_clear_hook = hook_recall_clear;
  recall_print_hook = hook_recall_print;
  use_recall_win = 1;
#else
  use_recall_win = 0;
#endif

  /* Hook in some functions */
  ralloc_aux = hook_ralloc;
  rnfree_aux = hook_rnfree;
  plog_aux = hook_plog;
  quit_aux = hook_quit;
  core_aux = hook_core;

  /* Nothing to save yet */
  disable_save();

  /* Prepare the filepaths */
  get_file_paths();

  /* Load the font */
  init_angband_font();

  /* create two frequently used brushes (for cursor box) */
  hbrRed    = CreateSolidBrush(win_clr[4]);
  hbrYellow = CreateSolidBrush(win_clr[11]);

  hwndMain = CreateWindow(AppName, "Angband 2.7.1",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    80 * iFontWidth + 2 * GetSystemMetrics(SM_CXBORDER),
    24 * iFontHeight + 2 * GetSystemMetrics(SM_CYBORDER) +
       GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU),
    HWND_DESKTOP, NULL, hInst, NULL);
  if (hwndMain == 0) goto exit;

  /* Prepare the hooks */
  Term_text_hook = Term_text_win;
  Term_wipe_hook = Term_wipe_win;
  Term_curs_hook = Term_curs_win;
  Term_scan_hook = Term_scan_win;
  Term_xtra_hook = Term_xtra_win;
  
  /* We support the latest technology... */
  Term_method(TERM_SOFT_CURSOR, TRUE);
  Term_method(TERM_SCAN_EVENTS, TRUE);

  /* Prepare the "Term" */
  Term_init();

  /* need this, otherwise all client area is redrawn
     the first time a menu is opened */
  ValidateRect(hwndMain, NULL);

  /* Get a pointer to the high score file */
  init_scorefile();

  /* Display the "news" message */
  show_news();

  /* Allocate and Initialize various arrays */
  prt("[Initializing arrays...]", 23, 22);
  Term_fresh();
  init_some_arrays();
  prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 17);
  Term_fresh();

  /* Did the user double click on a save file? */
  check_for_save_file();

  /* Process messages (until "play_game_mac()" is called) */
  while (GetMessage(&msg, hwndMain, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

exit:
  main_exit();
  return 0;
}

/*
 * Exit the program
 */
void main_exit()
{
  if (hwndMain != 0)
  {
    DestroyWindow(hwndMain);
    hwndMain = 0;
  }
  DeleteObject(hbrRed);
  DeleteObject(hbrYellow);
  DeleteObject(hfMain);
  RemoveFontResource(ANGBAND_FONT);
  UnregisterClass(AppName, hInstance);
  DestroyIcon(wc.hIcon);
  exit(0);
}

#endif /* _Windows */
