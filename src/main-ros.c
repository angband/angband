/*
 * File: main-ros.c
 * Purpose: Support for RISC OS versions of Angband
 *
 * Copyright (c) 2000-2007  Musus Umbra, Antony Sidwell, Thomas Harris,
 * Andrew Sidwell, Ben Harrison.
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

#ifdef __riscos

#include "angband.h"

/*
 * Purpose: Support for RISC OS Angband 2.9.x onwards (and variants)
 * Current maintainer: Antony Sidwell <antony@isparp.co.uk>  (ajps)
 *
 * NB: This code is still under continuous development - if you want to use
 * it for your own compilation/variant, please contact me so that I can
 * keep you up to date and give you support :)
 *
 * NB: This frontend will no longer work as-is for modern Zangbands (2.7.3
 * onwards), as the display code has changed, and so our platform-specific
 * menu needs adjusting to use the appropriate new functions.  A version which
 * should work should be supplied with the Z source. -- ajps
 *
 * NB: This frontend will not work with the in-development ToME3, as the file
 * handling (and many other parts) have been completely overhauled.  I suspect
 * it is not worth trying to shoehorn that model into this code, and a fresh,
 * UNIX-porting-tools approach may be better suited. -- ajps
 *
 * Prerequisites to compiling:
 *
 * DeskLib 2.50 or later 
 *
 * An ANSI C compiler (tested with Acorn's C/C++ and GCC, but should be OK
 *                     with any decent compiler)
 *
 * My binary distribution (for the templates and other bits)
 *
 * Note:
 *   The following symbols are *required* and *must* be defined properly.
 */

/*
 * PORTVERSION
 *   This is the port version; it appears in the infobox.
 */
#define PORTVERSION	"1.34 (2007-06-24)"

/*
 * VARIANT & VERSION
 *   These two get variant and version data from Angband itself; older
 *   variants may not have these defined and will have to be altered.
 */
#ifndef VARIANT
#define VARIANT		VERSION_NAME
#endif

#ifndef VERSION
#define VERSION		VERSION_STRING
#endif

/*
 * RISCOS_VARIANT
 *  This must match the entry in the !Variant Obey file, and it must only
 *  contain characters that are valid as part of a RISC OS path variable.
 *  [eg. "Yin-Yangband" is not okay, "EyAngband" is.]
 */
#ifndef RISCOS_VARIANT
#define RISCOS_VARIANT	"Angband"
#endif

/*
 * AUTHORS
 *  For the info box. [eg. "Ben Harrison"]
 */
#ifndef AUTHORS
#define AUTHORS		"Robert Ruehlmann"
#endif

/*
 * PORTERS
 *  For the info box. [eg. "Musus Umbra"]
 */
#ifndef PORTERS
#define PORTERS		"Antony Sidwell"
#endif

/*
 * ICONNAME
 *  Iconbar icon sprite name eg. "!angband".  Note that this must be a valid
 *  sprite name; it may need modifying for long variant names.
 */
#ifndef ICONNAME
#define ICONNAME	"!"RISCOS_VARIANT
#endif

/*
 * PDEADCHK
 *   This should expand to an expression that is true if the player is dead.
 *   Examples (correct as of Feb 2004):
 *   Vanilla (and most variants):  #define PDEADCHK	(p_ptr->is_dead)
 *   Zangband:                     #define PDEADCHK	(p_ptr->state.is_dead)
 *   Tome:                         #define PDEADCHK	(!alive)
 *   SCthAngband:                  #define PDEADCHK	(!alive || death)
 */
#ifndef PDEADCHK
#define PDEADCHK	(p_ptr->is_dead)
#endif

/*
 * MEMTYPE
 *   This defines which of the various sets of memory allocation prototypes
 *   should be used for this variant.
 *
 *   1: The pre 2.9.x type, where g_free returns an errr (largely obsolete)
 *   2: The 2.9.x type, where sizes are type "huge" and g_free takes a size.
 *   3: The 2.9.7(ish)+ type, where they take sizes as size_t
 */
#ifndef MEMTYPE
#define MEMTYPE 3
#endif

/*
 * FD_TYPE
 *   Many of the variants based on older Angbands use huge rather than
 *   size_t in the fd_* functions.  There are other halfway changes too,
 *   so you have to pick one of three for each variant. :(
 */
#ifndef FDTYPE
#define FDTYPE 3
#endif

/*
 * HAS_MY_STRCPY, HAS_MY_STRCAT, HAS_MY_STRNICMP
 *   We require the definition of two functions: my_strcat and my_strcpy.
 *   Some variants already have these defined (e.g. modern Vanilla), and
 *   so these should be defined when compiling those.
 */
/*
#define HAS_MY_STRCPY
#define HAS_MY_STRCAT
#define HAS_MY_STRNICMP
*/

/*
 * BIGSCREEN
 *   This should be TRUE if the variant actually supports bigscreen, FALSE
 *   otherwise.
 */
#ifndef BIGSCREEN
#define BIGSCREEN FALSE
#endif

/*
 * HASNOCORE
 *   In case someone's removed the core() function in an attempt
 *   to clean up the code but just making work for people with no
 *   ultimate benefit.
 */
#define HASNOCORE

/*
 * USE_DA
 *  If defined, it enables the use of dynamic areas (these are still only
 *  used when the !Variant file allows it).  It is likely that this option
 *  will eventually be removed altogether as there is no major advantege
 *  to using DAs over just using the Wimpslot.
 */
/* #define USE_DA */

/*
 * FULLSCREEN_ONLY
 *  If defined, the Wimp window-based interface will be disabled, and the
 *  game will only play in fullscreen mode.  This turns out to be a pointless
 *  feature, as the memory saving is now negligable compared to the overall
 *  size of the game.
 */
/* #define FULLSCREEN_ONLY */ 

/*
 * ZANGBAND_TERM_PACKAGE
 *  New version of Zangband (2.7.3ish and later) use a new term streamlined
 *  term package including a put_fstr function we have to use for our
 *  "platform specific menu" in the game.  Define this if you are compiling
 *  such a version of Zangband or (who knows?) a variant.
 */
/* #define ZANGBAND_TERM_PACKAGE */  


/*
 * The following symbols control the (optional) file-cache:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * NOTE (11th Dec 2004): The file caches have not been used for the last
 * few years - I do not believe they still work with most variants, except
 * possibly some branched off old Zangband (e.g. Cth).  The introduction of
 * scripting into those variants which used to use large plain text files
 * seems to have diminished their usefulness. -- Antony Sidwell
 *
 * NB: Variants that don't repeatedly read any files whilst running
 * (eg. vanilla, sang, etc) should NOT define USE_FILECACHE, etc. as
 * it causes a non-negligable amount of code to be compiled in.
 *
 * NB: The file-cache functions require that some code in files.c is modified
 * to use the cached_* functions.  This should be utterly trivial.
 *
 * NB: The returned handle from cached_fopen() is almost certainly *NOT*
 * a |FILE*| (although it may be if the cache cannot accomodate the file).
 *
 * Therefore, you *MUST* ensure that any file opened with cached_fopen()
 * is only ever accessed via cached_fgets() and cached_fclose().
 *
 * Failure to do so will result in, ahem, unpleasantness.  Extreme
 * unpleasantness.  "Him fall down, go boom."
 *
 * This /may/ change in the near future (ie. to apply caching in a
 * transparent manner), so do keep a backup of files.c (and any other files
 * you modify).  You always keep backups anyway, don't you?  Don't you?!
 */

/*
 * USE_FILECACHE
 *   if defined then some caching functions will be compiled for use by the
 *   various get_rnd_line(), etc. in files.c.  This could be used in a
 *   variety of places that read data repeatedly, but it's up to you to
 *   implement it then.
 */

/* #define USE_FILECACHE */

/*
 * SMART_FILECACHE
 *   This causes lines beginning with '#' (and blank lines) to be discarded
 *   when caching files.  This should help Zangband 2.2.5+ but could cause
 *   trouble for other variants.  If defined, then smart file caching will be
 *   on by default.
 */

/* #define SMART_FILECACHE */

/*
 * ABBR_FILECACHE
 *   ABBR_FILECACHE causes data read into file-cache to be compressed (using a
 *   simple set of abbreviations) by default.  This can be overridden using a
 *   command line option.  If this symbol is not defined then no compression
 *   code will be compiled and the user option will be ignored/unavailable.
 */

/* #define ABBR_FILECACHE */

/*
 * Note:
 *   The following symbols control debugging information.
 */

/*
 * FE_DEBUG_INFO
 *  If defined, some functions will be compiled to display some info. on the
 *  state of the front-end (accessible) from the '!' user menu.
 *
 *  NB: For actual releases you should NOT define this symbol since it causes
 *  a non-negligable amount of code/data to be sucked in.
 */
/* #define FE_DEBUG_INFO */


/* sCthAngband oddities */
#ifdef IS_SCTH
  #define SAVE_PLAYER_PARAM FALSE
  #define PAUSE_LINE_PARAM
  #define TERM_NAME(n) windows[n].name
  #define TERM(i) windows[i].term
  extern errr check_modification_date(int fd, cptr template_file);
#else
  #define SAVE_PLAYER_PARAM
  #define PAUSE_LINE_PARAM 23
  #define TERM_NAME(n) angband_term_name[n]
  #define TERM(i) angband_term[i]
#endif


/* NPP (for now) oddities */
#ifdef HASNOCORE
extern void core(cptr str);
#endif

/* V, post3.0.7, has conflicting types for these we have to #define around */
#undef event_type
#undef menu_flags
#undef menu_item

/* Constants, etc. ---------------------------------------------------------*/

/* Deal with any weird file-caching symbols */
#ifndef USE_FILECACHE
# undef ABBR_FILECACHE
# undef SMART_FILECACHE
#endif

/* Maximum terminals */
#define MAX_TERM_DATA 8

/* Menu entry numbers */
enum
{
	IBAR_MENU_INFO = 0,
	IBAR_MENU_SAVE,
	IBAR_MENU_FULLSCREEN,
	IBAR_MENU_GAMMA,
	IBAR_MENU_SOUND,
	IBAR_MENU_WINDOWS,
	IBAR_MENU_SAVECHOICES,
	IBAR_MENU_QUIT
};

enum
{
	TERM_MENU_INFO = 0,
	TERM_MENU_SAVE,
	TERM_MENU_SIZE,
	TERM_MENU_FONT,
	TERM_MENU_WINDOWS
};

/* Icon numbers */
#define SND_VOL_SLIDER			0
#define SND_VOL_DOWN			1
#define SND_VOL_UP				2
#define SND_ENABLE				3

#define GAMMA_ICN				0
#define GAMMA_DOWN				1
#define GAMMA_UP				2

#define SAVE_ICON				2
#define SAVE_PATH				1
#define SAVE_OK					0
#define SAVE_CANCEL				3

#define SIZE_WIDTH              1
#define SIZE_HEIGHT             4
#define SIZE_CANCEL             7
#define SIZE_SET                6

/* Position and size of the colours strip in the gamma window */
#define GC_XOFF 20
#define GC_YOFF -14
#define GC_WIDTH 512
#define GC_HEIGHT 72

/* Maximum and minimum allowed volume levels */
#define SOUND_VOL_MIN			16
#define SOUND_VOL_MAX			176

/*--------------------------------------------------------------------------*/


#undef rename
#undef remove
#undef UNUSED

#include "Desklib:Event.h"
#include "Desklib:EventMsg.h"
#include "Desklib:Template.h"
#include "Desklib:Window.h"
#include "Desklib:Handler.h"
#include "Desklib:Screen.h"
#include "Desklib:Menu.h"
#include "Desklib:Msgs.h"
#include "Desklib:Icon.h"
#include "Desklib:Resource.h"
#include "Desklib:SWI.h"
#include "DeskLib:Str.h"
#include "Desklib:Time.h"
#include "Desklib:Sound.h"
#include "Desklib:KeyCodes.h"
#include "Desklib:Kbd.h"
#include "Desklib:GFX.h"
#include "Desklib:ColourTran.h"
#include "Desklib:Error.h"
#include "Desklib:Coord.h"
#include "Desklib:Slider.h"
#include "Desklib:Hourglass.h"
#include "Desklib:Save.h"
#include "Desklib:KernelSWIs.h"
#include "DeskLib:File.h"
#include "DeskLib:Filing.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

/*--------------------------------------------------------------------------*/

/*
 | We use the hourglass around calls to Wimp_Poll in an attempt to stop
 | users thinking that the game has 'hung'.
 | Kamband/Zangband and the Borg in particular can have quite long delays at
 | times.
 */

/*
 * Note that empty-bracketed macros aren't ANSI/ISO C89 defined.  GCC and
 * Norcroft don't mind them though.
 */
#define START_HOURGLASS \
	do { if (use_glass && !glass_on) { glass_on=1; Hourglass_Start(50); }} while (0)
#define STOP_HOURGLASS \
	do { if (glass_on) { glass_on=0; Hourglass_Off(); } } while (0)


/*--------------------------------------------------------------------------*/
/* Types																	*/
/*--------------------------------------------------------------------------*/

/*
 | A ZapRedraw block
 */
typedef struct
{
	union
	{
		unsigned int value;
		struct
		{
			unsigned int vdu:1;
			unsigned int double_height:1;
			unsigned int extension:1;
			unsigned int padding:29;
		}
		bits;
	}
	r_flags;

	int r_minx;	/* min x of redraw in pixels from LHS, incl */
	int r_miny;	/* min y of redraw in pixels from top, incl */
	int r_maxx;	/* max x of redraw in pixels from LHS, excl */
	int r_maxy;	/* max y of redraw in pixels from top, excl */

	void *r_screen;	/* DSA: address of screen to write to (0=>read) */
	int r_bpl;	/* DSA: bytes per raster line */

	int r_bpp;	/* log base 2 of bits per pixel */
	int r_charw;	/* width of a character in pixels */
	int r_charh;	/* height of a character in pixels */
	void *r_caddr;	/* DSA: ->character cache | VDU: ->font name */
	int r_cbpl;	/* DSA: #bytes/character line | VDU: x OS offset */
	int r_cbpc;	/* DSA: #bytes/character | VDU: y OS offset */

	int r_linesp;	/* line spacing (pixels) */

	void *r_data;	/* -> text to display */
	int r_scrollx;	/* see Redraw dox */
	int r_scrolly;	/* see Redraw dox */

	void *r_palette;	/* -> palette lookup table */
	int r_for;	/* foreground colour at start of line */
	int r_bac;	/* background colour at start of line */

	void *r_workarea;	/* -> word aligned workspace */

	int r_magx;	/* log2 x OS coords per pixel */
	int r_magy;	/* log2 y OS coords per pixel */

	int r_xsize;	/* width of screen in pixels */
	int r_ysize;	/* height of screen in pixels */

	unsigned int r_mode;	/* current screen mode */
}
ZapRedrawBlock;


/*
 | We cache font data using an array of 'font handles' (since there is a
 | known maximum no. of fonts required).
 | This is what a font 'handle' looks like:
 */
typedef struct
{
	char *name;	/* font name */
	int usage;	/* usage count */
	int w, h;	/* width, height */
	int f, l;	/* first and last character defined */
	void *bpp_1;	/* source bitmap */
	void *bpp_n;	/* bitmap for the current screen mode */
}
ZapFont;

/*
 | A struct to hold all the data relevant to a term window
 */
typedef struct
{
	term t;	/* The Term itself */
	window_handle w;	/* Window handle */
	ZapFont *font;	/* Font */
	wimp_box changed_box;	/* Area out of date */
	struct
	{
		wimp_point pos;	/* Cursor position */
		BOOL visible;	/* visibility flag */
	}
	cursor;
	char name[12];	/* Name to give menus opened from the term */
	int def_open;	/* Open by default? */
	wimp_box def_pos;	/* default position */
	wimp_point def_scroll;	/* default scroll offset */
	int unopened;	/* Has this window not been opened yet? */
}
term_data;



/*--------------------------------------------------------------------------*/
/* ZapRedraw SWI numbers													*/
/*--------------------------------------------------------------------------*/

#define SWI_ZapRedraw_ 0x48480
#define SWI_ZapRedraw_RedrawArea (SWI_ZapRedraw_ + 0x00)
#define SWI_ZapRedraw_GetPaletteEntry (SWI_ZapRedraw_ + 0x01)
#define SWI_ZapRedraw_RedrawRaster (SWI_ZapRedraw_ + 0x02)
#define SWI_ZapRedraw_ConvertBitmap (SWI_ZapRedraw_ + 0x03)
#define SWI_ZapRedraw_PrepareDataLine (SWI_ZapRedraw_ + 0x04)
#define SWI_ZapRedraw_AddCursor (SWI_ZapRedraw_ + 0x05)
#define SWI_ZapRedraw_FindCharacter (SWI_ZapRedraw_ + 0x06)
#define SWI_ZapRedraw_MoveBytes (SWI_ZapRedraw_ + 0x07)
#define SWI_ZapRedraw_CachedCharSize (SWI_ZapRedraw_ + 0x08)
#define SWI_ZapRedraw_ConvBitmapChar (SWI_ZapRedraw_ + 0x09)
#define SWI_ZapRedraw_CreatePalette (SWI_ZapRedraw_ + 0x0a)
#define SWI_ZapRedraw_InsertChar (SWI_ZapRedraw_ + 0x0b)
#define SWI_ZapRedraw_ReadSystemChars (SWI_ZapRedraw_ + 0x0c)
#define SWI_ZapRedraw_ReverseBitmaps (SWI_ZapRedraw_ + 0x0d)
#define SWI_ZapRedraw_ReadVduVars (SWI_ZapRedraw_ + 0x0e)
#define SWI_ZapRedraw_GetRectangle (SWI_ZapRedraw_ + 0x0f)
#define SWI_ZapRedraw_AddVduBitmaps (SWI_ZapRedraw_ + 0x10)
#define SWI_ZapRedraw_CacheFontChars (SWI_ZapRedraw_ + 0x11)
#define SWI_ZapRedraw_SpriteSize (SWI_ZapRedraw_ + 0x12)
#define SWI_ZapRedraw_RedrawWindow (SWI_ZapRedraw_ + 0x13)


/*
 | Other SWI numbers that aren't defined in DeskLib's SWI.h:
 */
#define SWI_ColourTrans_ReturnColourNumber 0x40744
#define SWI_Wimp_ReportError 0x400df
#define SWI_PlayIt_Volume 0x4d146



/*--------------------------------------------------------------------------*
 | File scope variables													 |
 *--------------------------------------------------------------------------*/
static int ftype = 0xffd;	/* hack so saved games get the right type */
static int filehandle[16];	/* we keep track of open files with this */
static int openfiles = 0;	/* how many files are currently open */

/*
 | Paths we use...
 */
static char resource_path[260] = "";	/* Path pointng to "!Angband.Lib." */
static char scrap_path[260] = "";	/* Path to create scrap files on */
static char choices_file[3][260] =
{ "", "", "" };	/* Choices paths (read/write, mirror, read) */
static char alarm_file[2][260] =
{ "", "" };	/* Alarm choices paths (read/write, mirror, read) */
/*
 | So we can use something more meaningful later...
 | NB: Mirror is only meaningful for Choices and we don't
 | even reserve space for alarm_file[CHFILE_MIRROR].
 */
#define CHFILE_WRITE 0
#define CHFILE_READ 1
#define CHFILE_MIRROR 2

/*
 | Other 'globals':
 */
static int initialised = 0;	/* Used to determine whether to try to save */
static int game_in_progress = 0;	/* if Quit (or core() is called),  etc. */

static byte a_palette[256][4];	/* a copy of the raw Angband palette */
static unsigned int palette[256];	/* palette as gamma'd bbggrrxx words */
static unsigned int zpalette[256];	/* And our version for ZapRedraw */
static int gamma = 100;	/* assume gamma of 1.0 if unspecified */

static int enable_sound = 0;	/* enable sound FX */
static int sound_volume = 127;	/* Full volume */
static int force_mono = 0;	/* force monochrome */
static int start_fullscreen = 0;	/* start up full screen (added in 1.18) */
static int hack_flush = 0;	/* Should TERM_XTRA_FLUSH wait for all keys to be released? */
static int flush_scrap = 1;	/* Should any scrapfiles (incl. filecache) be deleted at exit? */
static int max_file_cache_size = 64 << 10;
static unsigned int vfiletype;
static int alarm_type = 0;	/* is there an alarm set? */
static int alarm_h = 0, alarm_m = 0;	/* alarm time (midnight) */
static char alarm_message[80] = "Time for bed!";	/* the message to give */
static int alarm_disp = 0;	/* is the alarm being displayed? */
static int alarm_beep = 0;	/* should be beep? */
static const char *alarm_types[] =
{ "Off", "On (one-shot)", "On (repeating)", "On (one-shot)" };
static unsigned int alarm_lastcheck = 0;


/* A little macro to save some typing later: */
#define COLOUR_CHANGED(x) \
	( (angband_color_table[x][1]!=a_palette[x][1]) || \
	  (angband_color_table[x][2]!=a_palette[x][2]) || \
	  (angband_color_table[x][3]!=a_palette[x][3]) )

static int got_caret = 0;	/* Do we own the caret? */
static int key_pressed = 0;	/* 'Key has been pressed' Flag */
static int use_glass = 1;	/* use the hourglass between WimpPolls? */
static int glass_on = 1;	/* is the hourglass on? */
static int user_menu_active = FALSE;	/* set to TRUE when the user menu is active */

/* Font system variables */
static ZapFont fonts[MAX_TERM_DATA + 1];	/* The +1 is for the system font */

/* The system font is always font 0 */
#define SYSTEM_FONT (&(fonts[0]))

/* Term system variables */
static term_data data[MAX_TERM_DATA];	/* One per term */

#ifndef FULLSCREEN_ONLY
static char *r_data = NULL;	/* buffer for ZapRedraw data */
static int r_maxwid, r_maxhgt;

/* Wimp variables */
static icon_handle ibar_icon;	/* Iconbar icon handle */
static window_handle info_box;	/* handle of the info window */
static window_handle resize_win; /* term resizing window */
static window_handle gamma_win;	/* gamma correction window */
static window_handle sound_win;	/* sound options window */
static window_handle save_box;	/* The savebox */
static menu_ptr ibar_menu;	/* Iconbar menu */
static menu_ptr term_menu;	/* Term window menu */
static menu_ptr wind_menu;	/* windows (sub) menu */
static menu_ptr font_menu;	/* Font (sub)menu */

static save_saveblock *saveblk = NULL;	/* For the save box */

static term_data *menu_term;	/* term the last menu was opened for */

#endif /* FULLSCREEN_ONLY */

static ZapRedrawBlock zrb;	/* a redraw block */

/* Cursor colour */
#define CURSOR_COLOUR	255		/* Cursor's Angband colour */
#define CURSOR_RGB		0x00ffff00	/* if undefined, use bbggrrxx */

static int cursor_rgb = -1;	/* colour to use for cursor */

static int fullscreen_mode = 0;	/* screen mode in use */
static int old_screenmode = 0;	/* Mode we started out in */
static int *fullscreen_font = 0;	/* font data for fullscreen use */
static int *fullscreen_base = 0;	/* base address of screen */
static int fullscreen_height;	/* height of the fullscreen font */
static int fullscreen_topline;	/* raster offset of fullscreen */

#define KEYPRESS_QUIT    0x1cc	/* F12 gets back to the desktop */
#define TERM_TOPLINE_HR  32		/* vertical pixel offset in mode 27 */
#define TERM_TOPLINE_LR  16		/* vertical pixel offset in mode 12 */
#define TIME_LINE        26		/* Line to display the clock on */

/* text to display at the bottom left of the fullscreen display */
static const char *fs_quit_key_text = "Press f12 to return to the desktop";
static const char *alarm_cancel_text = "(Press ^Escape to cancel the alarm)";

/* Debugging flags, etc. */
static int log_g_malloc = 0;	/* Log calls to ralloc, etc */
static int show_sound_alloc = 0;	/* Log sound mappings, etc */

/* Activate file caching? */
#ifdef USE_FILECACHE
static int use_filecache = TRUE;
#else
static int use_filecache = FALSE;
#endif

/* Cripple some things to save memory */
static int minimise_memory = 0;

/* Forward declarations of some of the Full Screen Mode stuff */
static void enter_fullscreen_mode(void);
static void leave_fullscreen_mode(void);
static void set_keys(int claim);

/* Forwards declarations of the sound stuff */
static void initialise_sound(void);
static void play_sound(int event);

/* Forward declarations of Term hooks, etc. */
static void Term_init_acn(term *t);
static errr Term_user_acn(int n);

#ifndef FULLSCREEN_ONLY
static errr Term_curs_acn(int x, int y);
static errr Term_text_acn(int x, int y, int n, byte a, cptr s);
static errr Term_xtra_acn(int n, int v);
static errr Term_wipe_acn(int x, int y, int n);
static errr Term_xtra_acn_check(void);
static errr Term_xtra_acn_event(void);
static errr Term_xtra_acn_react(void);
#endif /* FULLSCREEN_ONLY */

static errr Term_curs_acnFS(int x, int y);
static errr Term_text_acnFS(int x, int y, int n, byte a, cptr s);
static errr Term_wipe_acnFS(int x, int y, int n);
static errr Term_xtra_acn_clearFS(void);
static errr Term_xtra_acn_eventFS(int);
static errr Term_xtra_acn_reactFS(int force);
static void bored(void);
static void redraw_areaFS(int x, int y, int w, int h);
static void draw_cursor(int x, int y);

#ifdef USE_DA
/* Forward declarations of the memory stuff */
static void init_memory(int, int);
#endif

/* Forward declarations of the alarm stuff */
static void check_alarm(void);
#ifndef FULLSCREEN_ONLY
static void trigger_alarm_desktop(void);
#endif /* FULLSCREEN_ONLY */
static void ack_alarm(void);
static void write_alarm_choices(void);
static void read_alarm_choices(void);


/* This just shows some debugging info (if enabled with FE_DEBUG_INFO) */
static void show_debug_info(void);



/* File-caching functions (if enabled at compile time) */
#ifdef USE_FILECACHE
FILE *cached_fopen(char *name, char *mode);
errr cached_fclose(FILE *fch);
errr cached_fgets(FILE *fch, char *buffer, int max_len);
#endif


/*
 * There are various different ways in which the g_malloc and g_free functions
 * are prototyped, depending on the variant.  This is an attempt to unify the
 * codebase across thoe variants.
 */
#if MEMTYPE == 3
  #define G_MALLOC_PROT static void *g_malloc(size_t size)
  #define G_FREE_PROT static void *g_free(void *blk)
#elif MEMTYPE == 2
  #define G_MALLOC_PROT static vptr g_malloc(huge size)
  #define G_FREE_PROT static vptr g_free(vptr blk, huge size)
#elif MEMTYPE == 1
  #define G_MALLOC_PROT static vptr g_malloc(huge size)
  #define G_FREE_PROT static errr g_free(vptr blk, huge size)
#endif

/*
 | These functions act as malloc/free, but (if possible) using memory
 | in the 'Game' Dynamic Area created by init_memory()
 | We attach these functions to the ralloc_aux and rnfree_aux hooks
 | that z-virt.c provides.
 */
#ifdef USE_DA
  G_MALLOC_PROT;
  G_FREE_PROT;
#else  
  #define g_malloc(size) malloc(size);
#if MEMTYPE > 2
  #define g_free(block, size) free(block);
#else
  #define g_free(block) free(block);
#endif
#endif

/*
 | These functions act as malloc/free, but (if possible) using memory
 | in the 'Fonts' Dynamic Area created by init_memory()
 */
#ifdef USE_DA
static void* f_malloc(size_t size);
static void f_free(void *blk);
#else
  #define f_malloc(size) malloc(size);
  #define f_free(block) free(block);
#endif



/*
 | We use this to locate the choices file(s)...
 */
static char *find_choices(int write);
static char *find_choices_mirror(void);
static char *find_alarmfile(int write);



/*
 | This function is supplied as a wrapper to the save_player function.
 |
 | Its purpose is to change the filename that the game will be saved with
 | the leafname "!!PANIC!!" so that panic saves that break the savefile
 | won't overwrite the original savefile.
 |
 | To get this to work, you'll need to ammend files.c and change the call
 | to save_player in the panic save function(s) (search for "panic save")
 | to a call to save_player_panic_acn.  You can declare a prototype for
 | the function if you like.
 */

extern int save_player_panic_acn(void)
{
	char *e, *l;

	/* Find the final / in the savefile name */
	for (l = e = savefile; *e; e++)
		if (*e == '/')
		{
			l = e + 1;
		}

	/* Write over the current leaf with the special panic one */
	strcpy(l, "!!PANIC!!");

	/* save the game */
	return save_player(SAVE_PLAYER_PARAM);
}


/*--------------------------------------------------------------------------*/
/* Error reporting, etc.													*/
/*--------------------------------------------------------------------------*/


/* Tell the user something important */
static void plog_hook(cptr str)
{
	Msgs_Report(1, "err.plog", str);
}

/* Tell the user something, then quit */
static void quit_hook(cptr str)
{
	/* str may be null */
	if (str) Msgs_Report(1, "err.quit", str);
	exit(0);
}

/* Tell the user something then crash ;) */
static void core_hook(cptr str)
{
	Msgs_Report(1, "err.core", str);

	if (game_in_progress && character_generated)
		save_player_panic_acn();

	quit(NULL);
}

static void debug(const char *fmt, ...)
{
	va_list ap;
	char buffer[260];

	va_start(ap, fmt);
	vstrnfmt(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	plog(buffer);
}





/*--------------------------------------------------------------------------*/
/* File handling															*/
/*--------------------------------------------------------------------------*/

/*
 * We use myFile_WriteBytes and myFile_ReadBytes because we require a
 * different return value to that given by the DeskLib "File_" equivalents.
 */
static int myFile_WriteBytes(const int handle, const void *buf, const int n)
{
	int ntf;
	if (SWI(4, 4, SWI_OS_GBPB, 2, handle, buf, n, /**/ NULL, NULL, NULL, &ntf))
		return n;
	return ntf;
}

static int myFile_ReadBytes(const int handle, void *buf, const int n)
{
	int ntf;
	if (SWI(4, 4, SWI_OS_GBPB, 4, handle, buf, n, /**/ NULL, NULL, NULL, &ntf))
		return n;
	return ntf;
}


/*
 | Determine if one file is newer than another.
 |
 | The filenames should be specified in RISC OS style.
 |
 | Returns -1 if 'a' is newer than 'b'.
 */
static int file_is_newer(const char *a, const char *b)
{
	unsigned char a_time[5];
	unsigned char b_time[5];
	int n;

	/* If 'a' doesn't exist then 'b' isn't out of date */
	if (!File_Exists(a)) return 0;

	/* If 'b' doesn't exist then 'b' is out of date */
	if (!File_Exists(b)) return -1;

	/* Get the datestamp of the 'a' file */
	File_Date(a, a_time);

	/* Get the datestamp of the 'b' file */
	File_Date(b, b_time);

	/* Compare timestamps, defaulting to 0 if they are of equal age */
	for (n = 4; n >= 0; n--)
	{
		if (b_time[n] < a_time[n])
		{
			return -1;
		}
		if (b_time[n] > a_time[n]) return 0;
	}

	return 0;
}


/*
 | As fprintf, but output to all files (if their handles are non zero).
 | NB: void type.
 */
static void f2printf(FILE *a, FILE *b, const char *fmt, ...)
{
	va_list ap;
	char buffer[2048];
	va_start(ap, fmt);
	vstrnfmt(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	if (a) fprintf(a, buffer);
	if (b) fprintf(b, buffer);

	va_end(ap);
}




/*--------------------------------------------------------------------------*/
/* Clean up (ie. close files, etc). 										*/
/*--------------------------------------------------------------------------*/

static void final_acn(void)
{
	int i;

	for (i = 0; i < openfiles; i++) File_Close(filehandle[i]);

	if (fullscreen_mode)
	{
		/* Restore the screen mode */
		Wimp_SetMode(old_screenmode);

		/* Restore the various soft keys */
		set_keys(FALSE);
	}

	if (flush_scrap && *scrap_path)
	{
		char tmp[512];
		my_strcpy(tmp, scrap_path, sizeof(tmp));
		tmp[strlen(tmp) - 1] = 0;	/* Remove trailing dot */

		/* ie. "*Wipe <scrapdir> r~c~v~f" */
		SWI(4, 0, SWI_OS_FSControl, 27, tmp, 0, 1);
	}

#ifdef FULLSCREEN_ONLY
	Wimp_CommandWindow(-1);
#endif /* FULLSCREEN_ONLY */

	STOP_HOURGLASS;
}


/*--------------------------------------------------------------------------*
 | Various UNIX-like support funtions									   |
 *--------------------------------------------------------------------------*/

/*
 | Hack: determine whether filenames should be truncated to 10 chars or not.
 |
 | Needed since RO2 (and RO3 with Truncate configured off) will return
 | errors instead of automatically truncating long filenames.
 */
static int truncate_names(void)
{
	int r1, r2;

	/* Okay, so we've got RO3 (or later), so check the CMOS RAM */
	OS_Byte(osbyte_READCMOSRAM, 28, 0, &r1, &r2);

	/* Bit 0 of byte 28 is the Truncate flag */
	return !(r2 & 1);
}


/*
 | The PathName translation is now done by two separate functions:
 | unixify_name() and riscosify_name().
 |
 | This is done because only the UNIX=>RISCOS translation should
 | ever affect the length of the leafname (ie. by truncating it to
 | 10 chars if necessary).
 |
 | Note that the two functions are identical but for the truncation
 | check so all that's really been done is that translate_name() now
 | takes an extra argument: 'trunc' that controls whether truncation
 | is applied, and riscosify and unixify just call translate_name().
 */
static char *translate_name(const char *path, int trunc)
{
	static char buf[260];
	char c, *p;

	/* Copy 'path' into 'buf', swapping dots and slashes */
	p = buf;					/* Output position */
	do
	{
		c = *path++;
		if (c == '/')
			c = '.';
		else if (c == '.')
			c = '/';
		*p++ = c;
	}
	while (c);					/* Terminator /is/ copied */

	/*
	   | When saving a game, the old game is renamed as
	   | "SavedGame.old", the new one is saved as "SavedGame.new",
	   | "SavedGame.old" is deleted, "SavedGame.new" is renamed
	   | as "SavedGame". This will go wrong on a Filecore based filing
	   | system if the saved game has a leafname > 8 chars.
	 */

	if ((p = strstr(buf, "/old")) == NULL)
	{
		p = strstr(buf, "/new");
	}
	if (!p)
	{
		ftype = 0xffd;
	}
	else
	{
		char *q = strrchr(buf, '.');
		if (q)
			if (p - q > 6)
			{
				memmove(q + 6, p, 5);
			}
		ftype = vfiletype;
	}

	/*
	   | Hack: Do we need to truncate the leafname?
	 */
	if (trunc)
	{
		if (truncate_names())
		{
			char *a, *b;
			/*
			   | Assume that only the leafname needs attention
			   | (this should be true for any variant)
			 */
			for (a = b = buf; *a; a++)
				if (*a == '.')
					b = a + 1;
			/*
			   | Now b points to the start of the leafname.
			   | If the leafname is >10 chars, write over the 10th with a
			   | terminator.
			 */
			if (strlen(b) > 10)
			{
				b[10] = 0;
			};
		}
	}

	return buf;
}


extern char *riscosify_name(const char *path)
{
	return translate_name(path, TRUE);
}

static char *unixify_name(const char *path)
{
	return translate_name(path, FALSE);
}


/*--------------------------------------------------------------------------*/


/*
 * Open a file [as fopen()] but translate the requested filename first
 */
FILE *my_fopen(const char *f, const char *m)
{
	FILE *fp;
	char *n = riscosify_name(f);	/* translate for RO */

	/* Try to open the file */
	fp = fopen(n, m);

	/* If it succeded and the file was opened for binary output
	   | then set the type according to the 'ftype' hack.
	   | NB: This will fail on some filing systems.
	 */

	if (fp && strstr(m, "wb"))
	{
		File_SetType(n, ftype);
	}

	return fp;
}


/*
 * Close a file, a la fclose()
 */
#if FCLOSETYPE == 1
  #define RETURN
  void my_fclose(FILE *fp)
#else
  #define RETURN return
  errr my_fclose(FILE *fp)
#endif
{
	/* Close the file, return 1 for an error, 0 otherwise */
	RETURN fclose(fp) ? 1 : 0;
}
#undef RETURN


/*
 * Open/Create a file
 */
int fd_make(cptr file, int mode)
{
	char *real_path;
	file_handle handle;

	/* Translate the filename into a RISCOS one */
	real_path = riscosify_name(file);

	/* Try to OPENOUT the file (no path, error if dir or not found) */
	handle = File_Open(real_path, (file_access) 0x8f);

	/* Check for failure */
	if (!handle) return -1;

	/* Try to set the filetype according to the ftype hack */
	File_SetType(real_path, ftype);

	/* We keep track of up to 16 open files at any given time */
	if (openfiles < 16) filehandle[openfiles++] = handle;

	return (int) handle;
}


/* Delete a file [as remove()] */
errr fd_kill(cptr file)
{
	return remove(riscosify_name(file)) ? 1 : 0;
}


/* Rename a file [as rename()] */
errr fd_move(cptr old, cptr new)
{
	char new_[260];
	my_strcpy(new_, riscosify_name(new), sizeof(new_));
	return rename(riscosify_name(old), new_) ? 1 : 0;
}

/* Open a file */
int fd_open(cptr path, int flags)
{
	file_handle handle = 0;
	char *real_path = riscosify_name(path);

	switch (flags & 0x0f)
	{
		case O_RDONLY:			/* Read only */
			handle = File_Open(real_path, (file_access) 0x4f);
			break;
		case O_WRONLY:			/* Write only */
		case O_RDWR:			/* Read/Write */
			handle = File_Open(real_path, (file_access) 0xcf);
	}

	/* Check for failure */
	if (!handle) return (-1);

	/* Keep track of upto 16 open files... */
	if (openfiles < 16)
		filehandle[openfiles++] = handle;

	return (int) handle;
}


/* Close a file opened with fd_make or fd_open */
errr fd_close(int handle)
{
	int i;

	if (handle <= 0)
	{
		return -1;
	}							/* Illegal handle */

	/* Try to close the file */
	if (File_Close(handle))
	{
		return 1;
	}

	/* Mark the file as closed in our array of file handles */
	openfiles--;
	/* Find the entry in the array (if it exists) */
	for (i = 0; i < 16; i++)
		if (filehandle[i] == handle)
		{
			break;
		}
	/* Shuffle the remaining entries down */
	for (; i < openfiles; i++)
		filehandle[i] = filehandle[i + 1];

	return 0;					/* Sucess */
}



/* Read some bytes from a file */
#if FDTYPE == 1
errr fd_read(int handle, char *buf, huge nbytes)
#elif FDTYPE == 2
errr fd_read(int handle, char *buf, huge nbytes)
#else
errr fd_read(int handle, char *buf, size_t nbytes)
#endif
{
	int unread;

	/* Check the handle is legal */
	if (handle <= 0) return -1;

	unread = myFile_ReadBytes(handle, buf, (int) nbytes);

	return unread ? 1 : 0;
}


/* Write some bytes to a file */
#if FDTYPE == 1
errr fd_write(int handle, const char *buf, huge nbytes)
#elif FDTYPE == 2
errr fd_write(int handle, cptr buf, huge nbytes)
#else
errr fd_write(int handle, const char *buf, size_t nbytes)
#endif
{
	int unwritten;

	/* Check the handle is legal */
	if (handle <= 0) return -1;

	unwritten = myFile_WriteBytes(handle, buf, (int)nbytes);

	return unwritten ? 1 : 0;
}


/* Seek in a file */
#if FDTYPE == 1
errr fd_seek(int handle, huge offset)
#elif FDTYPE == 2
errr fd_seek(int handle, long offset)
#else
errr fd_seek(int handle, long offset)
#endif
{
	os_error *e;

	/* Check the handle is legal */
	if (handle <= 0) return -1;

	e = File_Seek(handle, (int)offset);

	return e ? 1 : 0;
}


/* RISC OS provides no file locking facilities, so: */
errr fd_lock(int handle, int what)
{
	return 0;
}


/* Get a temporary filename */
errr path_temp(char *buf, int max)
{

	/*
	   | New in 1.25 - use the scrap path we decided on earlier, or
	   | fall back on tmpnam() if that fails for some reason.
	 */
	if (*scrap_path)
	{
		time_t t;
		int m;
		char tmp[512];

		/*
		 * We try up to eighty scrapfiles based on the current time
		 * until we find a filenane that doesn't yet exist.
		 */
		time(&t);
		for (m = 0; m < 80; m++)
		{
			sprintf(tmp, "%s0x%08x", scrap_path, (int) t + m);

			if (File_Size(tmp) == -1) break;
		}

		if (m < 80)
		{
			strncpy(buf, unixify_name(tmp), max);
			return 0;
		}
	}

	strncpy(buf, unixify_name(tmpnam(NULL)), max);
	return 0;
}

#ifdef NEEDS_ACCESS
int access(const char *path, int mode)
{
	os_error *e;
	int f;

    e = SWI(2, 1, SWI_OS_Find, (1<<2) | (1<<3) | 0x40, riscosify_name(path), &f);

	if (e || f == 0) return -1;

    SWI(2, 0, SWI_OS_Find, 0, f);

	return 0;
}
#endif


/*
 * Create a new path by appending a file (or directory) to a path
 *
 * This requires no special processing on simple machines, except
 * for verifying the size of the filename, but note the ability to
 * bypass the given "path" with certain special file-names.
 *
 * Note that the "file" may actually be a "sub-path", including
 * a path and a file.
 *
 * Note that this function yields a path which must be "parsed"
 * using the "parse" function above.
 */
#if PATHBUILDTYPE == 1
void path_build(char *buf, int max, cptr path, cptr file)
#elif PATHBUILDTYPE == 2
errr path_build(char *buf, int max, cptr path, cptr file)
#else
errr path_build(char *buf, size_t max, cptr path, cptr file)
#endif
{
	/* Special file */
	if (file[0] == '~')
	{
		/* Use the file itself */
		strnfmt(buf, max, "%s", file);
	}

	/* Absolute file, on "normal" systems */
	else if (prefix(file, PATH_SEP) && !streq(PATH_SEP, ""))
	{
		/* Use the file itself */
		strnfmt(buf, max, "%s", file);
	}

	/* No path given */
	else if (!path[0])
	{
		/* Use the file itself */
		strnfmt(buf, max, "%s", file);
	}

	/* Path and File */
	else
	{
		/* Build the new path */
		strnfmt(buf, max, "%s%s%s", path, PATH_SEP, file);
	}

#if PATHBUILDTYPE > 1
	/* Success */
	return 0;
#endif
}



/*--------------------------------------------------------------------------*/





/*--------------------------------------------------------------------------*/
/* Font Functions															*/
/*--------------------------------------------------------------------------*/

/*
 | Cache the system font as fonts[0]
 | Returns 1 for sucess or 0 for failure.
 | NB: The n_bpp data is *not* cached, just the 1bpp data and font info.
 | Also, the usage is never affected.
 */
static int cache_system_font(void)
{
	ZapFont *sys = SYSTEM_FONT;
	ZapRedrawBlock zrb;
	char work_area[16];
	int i;

	/* Cache the system font (as fonts[0]) */
	if (sys->bpp_1)
	{
		f_free(sys->bpp_1);
		sys->bpp_1 = 0;
	}
	sys->bpp_1 = f_malloc(8 * 256);	/* 2K */
	if (!sys->bpp_1)
	{
		return 0;
	}

	/* Mung so that undefined characters show up as inverted ?s */
	work_area[3] = '?';
	SWI(2, 0, SWI_OS_Word, 10, work_area + 3);
	for (i = 4; i < 12; i++)
		work_area[i] ^= 255;	/* invert colours */
	SWI(4, 0, SWI_ZapRedraw_ReverseBitmaps, 0, work_area + 4, work_area + 4, 8);
	for (i = 0; i < 0x20; i++)
		memcpy(((char *)sys->bpp_1) + i * 8, work_area + 4, 8);

	/* Read the system font */
	zrb.r_workarea = work_area;
	SWI(2, 0, SWI_ZapRedraw_ReadSystemChars, sys->bpp_1, &zrb);

	/* Set up some little bits of info */
	sys->name = (char *) "<System>";
	sys->w = sys->h = 8;
	sys->f = 0;
	sys->l = 255;

	return 1;
}



/*
 | Prepare the font system
 */
static void initialise_fonts(void)
{
	/* Initialise the array */
	memset(fonts, 0, sizeof(fonts));

	/* Cache the system font */
	cache_system_font();
	fonts[0].usage = 0;			/* No users */
}


#ifndef FULLSCREEN_ONLY
/*
 | Find a font (by name) in the array.
 | Returns 0 if the font isn't loaded, or a ZapFont* for it if it is.
 */
static ZapFont *find_font_by_name(char *name)
{
	int i;
	for (i = 0; i <= MAX_TERM_DATA; i++)
		if (fonts[i].name)
			if (!strcmp(fonts[i].name, name))
				return &(fonts[i]);
	return NULL;
}

/*
 | Find a free slot in the fonts array
 */
static ZapFont *find_free_font(void)
{
	int i;
	for (i = 1; i <= MAX_TERM_DATA; i++)
		if (!fonts[i].name)
		{
			return &(fonts[i]);
		}
	return NULL;
}

#ifndef HAS_MY_STRCPY
/*
 * The my_strcpy() function copies up to 'bufsize'-1 characters from 'src'
 * to 'buf' and NUL-terminates the result.  The 'buf' and 'src' strings may
 * not overlap.
 *
 * my_strcpy() returns strlen(src).  This makes checking for truncation
 * easy.  Example: if (my_strcpy(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcpy() function in BSD.
 */
size_t my_strcpy(char *buf, const char *src, size_t bufsize)
{
	size_t len = strlen(src);
	size_t ret = len;

	/* Paranoia */
	if (bufsize == 0) return ret;

	/* Truncate */
	if (len >= bufsize) len = bufsize - 1;

	/* Copy the string and terminate it */
	(void)memcpy(buf, src, len);
	buf[len] = '\0';

	/* Return strlen(src) */
	return ret;
}
#endif /* !HAS_MY_STRCPY */

#ifndef HAS_MY_STRCAT
/*
 * The my_strcat() tries to append a string to an existing NUL-terminated string.
 * It never writes more characters into the buffer than indicated by 'bufsize' and
 * NUL-terminates the buffer.  The 'buf' and 'src' strings may not overlap.
 *
 * my_strcat() returns strlen(buf) + strlen(src).  This makes checking for
 * truncation easy.  Example:
 * if (my_strcat(buf, src, sizeof(buf)) >= sizeof(buf)) ...;
 *
 * This function should be equivalent to the strlcat() function in BSD.
 */
static size_t my_strcat(char *buf, const char *src, size_t bufsize)
{
	size_t dlen = strlen(buf);

	/* Is there room left in the buffer? */
	if (dlen < bufsize - 1)
	{
		/* Append as much as possible  */
		return (dlen + my_strcpy(buf + dlen, src, bufsize - dlen));
	}
	else
	{
		/* Return without appending */
		return (dlen + strlen(src));
	}
}
#endif /* !HAS_MY_STRCAT */


/*
 | Load a font from disc and set up the header info, etc.
 | NB: doesn't cache the nbpp data, just the 1bpp data.
 | (Sets usage to 1)
 | Returns NULL if failed.
 */
static ZapFont *load_font(char *name, ZapFont *f)
{
	int handle, extent;
	char path[260];
	struct
	{
		char id[8];
		int w, h, f, l, r1, r2;
	}
	header;
	char *font_path;
	char *t;
	char *real_name = name;	/* need to preserve this */

	/*
	   | 1.10 - the first element of the name determines the path to load
	   | the font from.
	 */

	/* The font paths start <RISCOS_VARIANT>$ */
	t = path + sprintf(path, "%s$", RISCOS_VARIANT);

	/* Copy the path specifier and move 'name' past it */
	for (; *name != '.'; *t++ = *name++) ;

	/* After this, the name now points to the font name proper */
	name++;

	/* Append the end of the path name */
	strcpy(t, "$FontPath");

	/* Get the path setting */
	font_path = getenv(path);
	if (!font_path || !*font_path)
		my_strcpy(path, "null:$.", sizeof(path));
	else
	{
		my_strcpy(path, font_path, sizeof(path));
		for (t = path; *t > ' '; t++)
			;
		if (t[-1] != '.' && t[-1] != ':')
		{
			*t++ = '.';
		}
		*t = 0;
	}
	my_strcat(path, name, sizeof(path));


	/* Open the file */
	handle = File_Open(path, (file_access) 0x4f);
	if (!handle)
	{
		return NULL;
	}

	/* Read the header */
	if (myFile_ReadBytes(handle, &header, sizeof(header)))
	{
		File_Close(handle);
		return NULL;
	}

	/* Check that it's a zapfont */
	if (strncmp(header.id, "ZapFont\r", 8))
	{
		File_Close(handle);
		return NULL;
	}

	/* Calculate the size of the 1bpp data */
	extent = File_ReadExtent(handle) - sizeof(header);

	/* Allocate the storage for the 1bpp data */
	f->bpp_1 = f_malloc(extent);
	if (!f->bpp_1)
	{
		File_Close(handle);
		return NULL;
	}

	/* Load the 1bpp data */
	if (myFile_ReadBytes(handle, f->bpp_1, extent))
	{
		f_free(f->bpp_1);
		f->bpp_1 = 0;
		File_Close(handle);
		return NULL;
	}

	/* Close the file and set the header, etc. */
	File_Close(handle);
	f->name = f_malloc(strlen(real_name) + 1);
	if (!f->name)
	{
		f_free(f->bpp_1);
		f->bpp_1 = 0;
		return NULL;
	}

	strcpy(f->name, real_name);
	f->w = header.w;
	f->h = header.h;
	f->f = header.f;
	f->l = header.l;
	f->usage = 1;

	return f;
}




/*
 | Cache a font at a suitable number of bpp for the current mode
 | Returns 0 for failure, 1 for sucess.
 | If the call fails then the font's bpp_n entry will be NULL.
 */
static int cache_font_for_mode(ZapFont *f)
{
	ZapRedrawBlock b;
	char work_area[128];
	int size;

	if (!f)
	{
		return 0;
	}
	if (!f->bpp_1)
	{
		return 0;
	}

	b.r_workarea = work_area;
	SWI(2, 0, SWI_ZapRedraw_ReadVduVars, 0, &b);

	b.r_workarea = work_area;	/* Paranoia */
	b.r_charh = f->h;
	b.r_charw = f->w;
	SWI(4, 4, SWI_ZapRedraw_CachedCharSize, b.r_bpp, 0, f->w, f->h,
		NULL, NULL, &(b.r_cbpl), &(b.r_cbpc));

	size = 256 * b.r_cbpc;
	if (f->bpp_n)
	{
		f_free(f->bpp_n);
		f->bpp_n = NULL;
	}
	f->bpp_n = f_malloc(size);
	if (!f->bpp_n)
	{
		return 0;
	}

	b.r_workarea = work_area;	/* Paranoia */
	b.r_caddr = f->bpp_n;
	SWI(5, 0, SWI_ZapRedraw_ConvertBitmap, 0, &b, 0, 255, f->bpp_1);

	return 1;
}



/*
 | Stop using a font.
 | If the font's usage drops to zero then the font data is purged.
 */
static void lose_font(ZapFont *f)
{
	if (--f->usage)
	{
		/*debug("Losing font %s (still cached)",f->name); */
		return;
	}
	/*debug("Losing font %s (no longer in use)",f->name); */
	f_free(f->name);
	f_free(f->bpp_1);
	if (f->bpp_n)
	{
		f_free(f->bpp_n);
	}
	memset(f, 0, sizeof(ZapFont));
}


/*
 | Get a font.
 */
static ZapFont *find_font(char *name)
{
	ZapFont *f;

	/* Check to see if it's already loaded */
	f = find_font_by_name(name);
	if (f)
	{
		/*debug("Find font %s (already cached)",name); */
		f->usage++;
		if (f == SYSTEM_FONT)
		{
			if (!cache_system_font())
				core("Failed to cache system font!");
			if (!cache_font_for_mode(SYSTEM_FONT))
				core("Failed to cache system font!");
		}
		return f;
	}

	/* Ok, now check to see if there's a free slot for it */
	f = find_free_font();
	if (!f)
	{
		return NULL;
	}							/* Oh dear :( */

	/* Load the font */
	/*debug("Find font %s (loading)",name); */
	f = load_font(name, f);
	if (f)
	{
		if (!cache_font_for_mode(f))
			return NULL;
		return f;
	}
	return NULL;
}




/*
 | Cache the n_bpp data for all the active fonts (including system)
 */
static void cache_fonts(void)
{
	int i;
	for (i = 0; i <= MAX_TERM_DATA; i++)
		if (fonts[i].name)
			if (!cache_font_for_mode(&(fonts[i])))
				core("Failed to (re)cache font tables");
}






typedef struct
{
	int load, exec, size, attr, type;
	char name[4];	/* Actual size is unknown */
}
osgbpb10_block;

/*
 | NB: This function is recursive.
 */
static menu_ptr make_zfont_menu(const char *dir)
{
	int entries, entry;
	int read, offset;
	unsigned int max_width;
	menu_ptr m;
	menu_item *mi;
	char *temp;
	filing_direntry *item_info;
	char buffer[1024];	/* 1Kb buffer */

	/* Count the entries in the directory */
	entries = offset = 0;
	while (offset != -1)
	{
		read = 77;

		if (Filing_ReadDirNames(dir, buffer, &read, &offset, sizeof(buffer), (char *) "*"))
		{
			offset = -1;
			read = 0;
		}
		entries += read;
	}

	if (!entries) return NULL;

	/* Allocate a big enough area of storage for the number of entries */
	m = f_malloc(sizeof(menu_block) + entries * sizeof(menu_item));
	if (!m)
	{
		return NULL;
	}
	memset(m, 0, sizeof(menu_block) + entries * sizeof(menu_item));

	/* Set up the menu header */
	strncpy(m->title, Str_LeafName((char *) dir), 12);
	m->titlefore = 7;
	m->titleback = 2;
	m->workfore = 7;
	m->workback = 0;
	m->height = 44;
	m->gap = 0;
	mi = (menu_item *) (((int)m) + sizeof(menu_block));
	max_width = strlen(m->title);

	entry = 0;

	/* Read the entries */
	offset = 0;
	while (offset != -1)
	{
		read = 77;

		if (Filing_ReadDirEntry(dir, (filing_direntry *) buffer, &read, &offset, sizeof(buffer), (char *) "*"))
		{
			offset = -1;
			read = 0;
		}

		item_info = (filing_direntry *) buffer;

		/* Create a menu item for each entry read (if it fits) */
		while (read-- > 0)
		{
			switch (item_info->objtype)
			{
				case filing_FILE:
				{
					if ((item_info->loadaddr & 0xffffff00) == 0xfffffd00)
					{
						/* Data file */
						mi[entry].submenu.value = -1;
						mi[entry].iconflags.data.text = 1;
						mi[entry].iconflags.data.filled = 1;
						mi[entry].iconflags.data.foreground = 7;
						mi[entry].iconflags.data.background = 0;
						strncpy(mi[entry].icondata.text, item_info->name, 12);
						if (strlen(mi[entry].icondata.text) > max_width)
							max_width = strlen(mi[entry].icondata.text);
						entry++;
					}
					break;
				}

				case filing_DIRECTORY:
				case filing_IMAGEFILE:
				{
					menu_ptr sub;
					char new_path[260];
					if (strchr(":.", dir[strlen(dir) - 1]))
						sprintf(new_path, "%s%s", dir, item_info->name);
					else
						sprintf(new_path, "%s.%s", dir, item_info->name);
					sub = make_zfont_menu(new_path);
					if (sub)
					{
						/* Add the submenu */
						mi[entry].submenu.menu = sub;
						mi[entry].iconflags.data.text = 1;
						mi[entry].iconflags.data.filled = 1;
						mi[entry].iconflags.data.foreground = 7;
						mi[entry].iconflags.data.background = 0;
						strncpy(mi[entry].icondata.text, item_info->name, 12);
						if (strlen(mi[entry].icondata.text) > max_width)
							max_width = strlen(mi[entry].icondata.text);
						entry++;
					}

					break;
				}

			}
			temp = ((char *) item_info) + 20;
			while (*temp++);
			item_info = (filing_direntry *) WORDALIGN((int) temp);
		}
	}

	if (entry)
	{
		m->width = (max_width + 2) * 16;
		mi[entry - 1].menuflags.data.last = 1;
		/*
		   | We could possibly realloc() the storage to fit the
		   | actual no. of entries read, but this is probably more
		   | trouble than it's worth.
		 */
	}
	else
	{
		/* No point in returning an empty menu. */
		f_free(m);
		m = NULL;
	}

	return m;
}

#endif /* FULLSCREEN_ONLY */




/*--------------------------------------------------------------------------*/

/*
 | Initialise the palette stuff
 */
static void initialise_palette(void)
{
	memset(a_palette, 0, sizeof(a_palette));
	memset(palette, 0, sizeof(palette));
	memset(zpalette, 0, sizeof(zpalette));
}



#ifndef FULLSCREEN_ONLY

/*
 | Cache the ZapRedraw palette
 */
static void cache_palette(void)
{
	static ZapRedrawBlock b;
	char workspace[128];
	int i;

	static int old_gamma = -1;

	/* Idiocy check: */
	if (gamma < 1)
	{
		plog("Internal error: Attempt to apply zero gamma - recovering...");
		gamma = 100;
	}

	if (gamma != old_gamma)
	{
		memset(a_palette, 0, sizeof(a_palette));
		old_gamma = gamma;
	}

	/* Go through the palette updating any changed values */
	for (i = 0; i < 256; i++)
	{
		if (COLOUR_CHANGED(i))
		{
			int r, g, b;
			r = (int)(255.0 * pow(angband_color_table[i][1] / 255.0, 1.0 / ((double) gamma / 100.0)));
			g = (int)(255.0 * pow(angband_color_table[i][2] / 255.0, 1.0 / ((double) gamma / 100.0)));
			b = (int)(255.0 * pow(angband_color_table[i][3] / 255.0, 1.0 / ((double) gamma / 100.0)));
			palette[i] = (b << 24) | (g << 16) | (r << 8);
			a_palette[i][1] = angband_color_table[i][1];
			a_palette[i][2] = angband_color_table[i][2];
			a_palette[i][3] = angband_color_table[i][3];
		}
	}

	cursor_rgb = palette[CURSOR_COLOUR];

	/* Cache the ZapRedraw palette for it */
	b.r_workarea = workspace;

	if (b.r_mode != screen_mode.screen_mode)
		SWI(2, 0, SWI_ZapRedraw_ReadVduVars, 0, &b);

	SWI(5, 0, SWI_ZapRedraw_CreatePalette, 2, &b, palette, zpalette, 256);
}




/*--------------------------------------------------------------------------*/

/*
 | Functions for dealing with the SaveBox
 */


#ifndef HAS_MY_STRNICMP
/*
 | Hack: can't use Str.h without defining HAS_STRICMP.  Rather than
 | require that the header files are altered we simply provide our
 | own strnicmp() function.
 */
static int my_strnicmp(const char *a, const char *b, int n)
{
	int i;

	n--;

	for (i = 0; i <= n; i++)
	{
		if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
			return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);

		if (a[i] == '\0')
			break;
	}

	return 0;
}
#endif /* HAS_MY_STRNICMP */


/*
 | This is the handler called when a 'save' occurrs.
 | All it does is to update the game's own savefile setting and
 | then (if possible) save the character.
 */
static BOOL SaveHnd_FileSave(char *filename, void *ref)
{
	char old_savefile[1024];

	/* Hack: refuse to save if the character is dead */
	if (PDEADCHK)
	{
		Msgs_Report(0, "err.cheat");
		return FALSE;
	}

	/* Hack: disallow saves to <Wimp$Scrap>* */
	if (!my_strnicmp("<wimp$scrap>", filename, 12))
	{
		Msgs_Report(0, "err.scrap");
		return FALSE;
	}

	/* Preserve the old path, in case something goes wrong... */
	my_strcpy(old_savefile, savefile, sizeof(old_savefile));

	/* Set the new path */
	my_strcpy(savefile, unixify_name(filename), sizeof(savefile));

	/* Try a save (if sensible) */
	if (game_in_progress && character_generated)
	{
		if (inkey_flag)
		{
			if (!save_player(SAVE_PLAYER_PARAM))
			{
				Msgs_Report(0, "err.save", filename);
				my_strcpy(savefile, old_savefile, sizeof(savefile));
				return FALSE;		/* => failure */
			}
		}
		else
		{
			Msgs_Report(0, "err.nosave");
			my_strcpy(savefile, old_savefile, sizeof(savefile));
			return TRUE; /* Failed really, but not unexpectedly */
		}
	}

	/* Set the pathname icon */
	Icon_printf(save_box, SAVE_PATH, "%s", riscosify_name(savefile));

	return TRUE;				/* => Success */
}

/*
 | Create the window and claim various handlers for it
 */
static void init_save_window(void)
{
	/* Create the window */
	save_box = Window_Create("save", template_TITLEMIN);

	/* Set the file icon */
	Icon_printf(save_box, SAVE_ICON, "file_%03x", vfiletype);

	saveblk = Save_InitSaveWindowHandler(save_box,	/* Window handle */
										 TRUE,	/* it's part of a menu */
										 FALSE,	/* not a window */
										 FALSE,	/* Don't auto release the handlers */
										 SAVE_ICON,	/* The file icon */
										 SAVE_OK,	/* The OK icon */
										 SAVE_CANCEL,	/* The cancel icon */
										 SAVE_PATH,	/* The pathname icon */
										 SaveHnd_FileSave,	/* Handler to "save the file" */
										 NULL,	/* No RAM transfer support */
										 NULL,	/* No 'result handler' */
										 100 << 10,	/* Est. size (irelevant anyway) */
										 vfiletype,	/* filetype (irelevant) */
										 NULL	/* ref */
		);
}


/*
 | Handles MenuWarning messages
 */
static BOOL Hnd_MenuWarning(event_pollblock * pb, void *ref)
{
	os_error *e;
	window_handle win;

	if (menu_currentopen == ibar_menu)
	{
		win = save_box;
	}
	else if (menu_currentopen == term_menu)
	{
		switch (pb->data.message.data.menuwarn.selection[0])
		{
			case TERM_MENU_SAVE: win = save_box;    break;
			case TERM_MENU_SIZE: win = resize_win;  break;
			default: return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	if (win == save_box)
	{
		/* Set the pathname */
		Icon_printf(save_box, SAVE_PATH, "%s", riscosify_name(savefile));
	}
	else
	{
		/* Set the size */
		Icon_SetInteger(resize_win, SIZE_WIDTH, menu_term->t.wid);
		Icon_SetInteger(resize_win, SIZE_HEIGHT, menu_term->t.hgt);
	}

	/* Open the submenu */
	e = Wimp_CreateSubMenu((menu_block *) win,
						   pb->data.message.data.menuwarn.openpos.x,
						   pb->data.message.data.menuwarn.openpos.y);

	if (e) Msgs_ReportFatal(0, "err.swi", __LINE__, e->errmess);

	return TRUE;
}


/*--------------------------------------------------------------------------*/


/*
 * Initialise the r_data array, reallocating memory for it if the size of our
 * "theoretical largest term" has changed since the last call.
 *
 * We just set up the line offset pointers and make sure that the
 * lines themselves are 'safe' by writing end-of-line codes to them.
 */
static void initialise_r_data(void)
{
	int maxwid = 0, maxhgt = 0;
	char *new_r_data;

	int *lo;
	char *ld;

	int i;

	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (data[i].t.wid > maxwid) maxwid = data[i].t.wid;
		if (data[i].t.hgt > maxhgt) maxhgt = data[i].t.hgt;
	}

	if (r_maxwid != maxwid || r_maxhgt != maxhgt)
	{
		/*
		 * We allocate enough memory for a theoretical largest zapredrawblock.
		 * This consists of a list of line starts + terminator, then a big
		 * block containing enough space for max_hgt lines of max_wid each
		 * (allowing for possible control codes) and an end-of-line-terminator
		 * for each line.
		 */
	 	new_r_data = realloc(r_data,
		                     (maxhgt + 1) * 4 + maxhgt * (maxwid * 5 + 4));

		if (new_r_data == NULL)
		{
			Msgs_Report(0, "err.resize");
			return;
		}

		r_data = new_r_data;
		zrb.r_data = r_data;

		r_maxwid = maxwid;
		r_maxhgt = maxhgt;
	}

	lo = (int *) r_data;


	/* Make a dummy block where all lines point to the same empty line */
	ld = r_data + (maxhgt + 1) * 4;
	*ld++ = 0;				/* 0,2 ==     */
	*ld = 2;				/* end of line */

	for (i = 0; i < maxhgt; i++)
	{
		/* Offset of line */
		lo[i] = (maxhgt + 1) * 4;
	}
	lo[i] = 0;					/* Terminate line index */
}



/*
 | Create the r_data array for a term
 | This is typically quite fast (1ms or so on a RPC700)
 | so we don't bother caching r_data for each term or using the
 | 'frosh' concept.
 */
static void make_r_data(term_data *t)
{
	char **c = t->t.old->c;	/* char array */
	byte **a = t->t.old->a;	/* attr array */
	char *o;
	int i, j, cf;

	/* First byte of r_data after line index */
	o = r_data + (t->t.hgt + 1) * 4;

	if (force_mono)
	{
		for (j = 0; j < t->t.hgt; j++)
		{
			/* Set up the line offset entry */
			((int *)r_data)[j] = o - r_data;

			for (i = 0; i < t->t.wid; i++)
				*o++ = a[j][i] != TERM_DARK ? c[j][i] : ' ';
			/* 0,2 => end of line */
			*o++ = 0;
			*o++ = 2;
		}
	}
	else
	{
		for (j = 0; j < t->t.hgt; j++)
		{
			/* Set up the line offset entry */
			((int *)r_data)[j] = o - r_data;

			/* Each line starts in white */
			cf = TERM_WHITE;

			for (i = 0; i < t->t.wid; i++)
			{
				if (a[j][i] != cf)
				{
					/* 0,6 => change FG */
					*o++ = 0;
					*o++ = 6;
					cf = *o++ = a[j][i];
				}
				*o++ = c[j][i];
			}
			/* 0,2 => end of line */
			*o++ = 0;
			*o++ = 2;
		}
	}

	/* Terminate line index */
	((int *) r_data)[t->t.hgt] = 0;
}


/*
 | Set up 'zrb' for the current screen mode.
 */
static void set_up_zrb_for_mode(void)
{
	static char work_area[4096];
	zrb.r_workarea = work_area;
	zrb.r_palette = zpalette;
	zrb.r_linesp = 0;
	zrb.r_for = TERM_WHITE;
	zrb.r_bac = 0;
	zrb.r_data = r_data;
	SWI(2, 0, SWI_ZapRedraw_ReadVduVars, 0, &zrb);
}



/*
 | Set up the ZapRedrawBlock ready to redraw term 't'
 | (caches the r_data as part of the process)
 */
static void set_up_zrb(term_data *t)
{
	int fw, fh;

	zrb.r_flags.value = 0;

	/* Set font info up */
	fw = t->font->w;
	fh = t->font->h;
	SWI(4, 4, SWI_ZapRedraw_CachedCharSize, zrb.r_bpp, 0, fw, fh,
		NULL, NULL, &(zrb.r_cbpl), &(zrb.r_cbpc));
	zrb.r_caddr = (void *)(((int)t->font->bpp_n) - (t->font->f * zrb.r_cbpc));

	zrb.r_charw = fw;			/* Character size in pixels */
	zrb.r_charh = fh;

	if (t->font == SYSTEM_FONT)
		zrb.r_flags.bits.double_height = screen_eig.y == 1;
	else
		zrb.r_flags.bits.double_height = 0;

	make_r_data(t);				/* Cache the r_data */
}




/*
 * Redraws the contents of the given term, as initialised by a
 * Wimp_RedrawWindow or Wimp_UpdateWindow call.
 */
static void RO_redraw_window(window_redrawblock * rb, BOOL *more, term_data *t)
{
	int cx = 0, cy = 0, cw = 0, ch = 0;

	/* set GCOL for cursor colour */
	if (t->cursor.visible)
	{
		cw = zrb.r_charw << screen_eig.x;
		ch = -(zrb.r_charh << screen_eig.y);
		if (zrb.r_flags.bits.double_height)
		{
			ch *= 2;
		}
		cx = t->cursor.pos.x * cw;
		cy = t->cursor.pos.y * ch;
		cx += (rb->rect.min.x - rb->scroll.x);
		cy += (rb->rect.max.y - rb->scroll.y);
		cw -= (1 << screen_eig.x);
		ch += (1 << screen_eig.y);
		cy -= (1 << screen_eig.y);
	}

	while (*more)
	{
		SWI(2, 0, SWI_ZapRedraw_GetRectangle, rb, &zrb);
		SWI(2, 0, SWI_ZapRedraw_RedrawArea, NULL, &zrb);
		if (t->cursor.visible)
		{
			ColourTrans_SetGCOL(cursor_rgb, 0, 0);
			GFX_Move(cx, cy);
			GFX_DrawBy(cw, 0);
			GFX_DrawBy(0, ch);
			GFX_DrawBy(-cw, 0);
			GFX_DrawBy(0, -ch);
		}
		Wimp_GetRectangle(rb, more);
	}
}




/*
 * Perform the redraw for the requested term window.
 */
static BOOL Hnd_Redraw(event_pollblock * pb, void *ref)
{
	term_data *t = (term_data *)ref;
	window_redrawblock rb;
	BOOL more;

	rb.window = t->w;
	Wimp_RedrawWindow(&rb, &more);

	set_up_zrb(t);

	RO_redraw_window(&rb, &more, t);

	return TRUE;
}




/*
 * Redraw the out-of-date parts of the given term window
 */
static void refresh_window(term_data *t)
{
	window_redrawblock rb;
	BOOL more;
	int fw, fh;

	if ((t->changed_box.min.x >= t->changed_box.max.x) ||
		(t->changed_box.min.y >= t->changed_box.max.y))
		return;

	set_up_zrb(t);

	fw = zrb.r_charw << screen_eig.x;
	fh = -(zrb.r_charh << screen_eig.y);
	if (zrb.r_flags.bits.double_height)
	{
		fh *= 2;
	}

	rb.window = t->w;
	rb.rect.min.x = fw * t->changed_box.min.x;
	rb.rect.max.x = fw * t->changed_box.max.x;

	rb.rect.max.y = fh * t->changed_box.min.y;
	rb.rect.min.y = fh * t->changed_box.max.y;

	Wimp_UpdateWindow(&rb, &more);
	RO_redraw_window(&rb, &more, t);

	t->changed_box.min.x = t->changed_box.min.y = 255;
	t->changed_box.max.x = t->changed_box.max.y = 0;
}


/*
 * Redraws the out-of-date parts of the all the open term windows.
 */
static void refresh_windows(void)
{
	int i;
	window_info info;

	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		info.window = data[i].w;
		Wimp_GetWindowInfo(&info);
		if (info.block.flags.data.open) refresh_window(&(data[i]));
	}
}


/*
 | Set the size of a window.
 | If the window grows but has no scroll bars then it is re-sized to
 | the new extent.  If it shrinks then it is resized regardless.
 |
 | If the window isn't open then it is opened behind the backwindow,
 | resized and then closed again...  I /did/ have a reason for doing this
 | rather than simply recreating the window at the new size, but for the
 | life of me I can't remember what it was...
 |
 | <ajps> I think this is simply so that the Wimp remembers the position
 |        of the window for when we next call show_windows().
 */
static void set_window_size(window_handle w, int width, int height)
{
	window_state ws;
	int reclose;

	Wimp_GetWindowState(w, &ws);
	Window_SetExtent(w, 0, -height, width, 0);

	reclose = !ws.flags.data.open;
	if (!(ws.flags.value & (0xf << 27)))
	{
		if (reclose)
		{
			ws.openblock.behind = -3;
			Wimp_OpenWindow(&(ws.openblock));
		}

		/* Keep the top right-hand corner of the window fixed. */
		ws.openblock.screenrect.max.x = ws.openblock.screenrect.min.x + width;
		ws.openblock.screenrect.min.y = ws.openblock.screenrect.max.y - height;

		Wimp_OpenWindow(&(ws.openblock));

		if (reclose)
		{
			Wimp_CloseWindow(w);
		}
	}
}















/*
 | Change the size of a window to suit the font displayed in it
 */
static void force_term_resize(term_data *t)
{
	int fw, fh;
	set_up_zrb(t);

	fw = zrb.r_charw << screen_eig.x;
	fh = zrb.r_charh << screen_eig.y;
	if (zrb.r_flags.bits.double_height)
	{
		fh *= 2;
	}

	/* Calculate new size */
	fw *= t->t.wid;
	fh *= t->t.hgt;

	set_window_size(t->w, fw, fh);
}


/*
 * Change the actual size of the terminal (so as to support variable-sized
 * "bigscreen" windows.
 */
static void term_change_size(term_data *t, int wid, int hgt)
{
	term *old_t = Term;

	/* Ignore attempts to resize too small */
	if (t == &data[0] && (wid < 80 || hgt < 24)) return;

	/* Hack -- activate the Term */
	Term_activate(&(t->t));

	/* Only do all the complicated stuff if the resize attempt succeeded */
#ifdef IS_SCTH
        Term_resize(wid, hgt);
#else
	if (Term_resize(wid, hgt) == NULL)
#endif	
	{
		initialise_r_data();
		force_term_resize(t);
	}

	/* Hack -- restore the old Term */
	Term_activate(old_t);
}








static BOOL Hnd_Caret(event_pollblock * pb, void *ref)
{
	if (ref) got_caret = 1;
	else got_caret = 0;

	return TRUE;
}




/*
 | Attach a (named) font to the specified term.
 | If 'font' is NULL then the system font is attached.
 | The bpp_n data is calculated if necessary
 | returns:
 |	1 => the font was attached OK
 |	0 => the system font was substituted
 */
static int attach_font_to_term(term_data *t, char *font)
{
	if (t->font != SYSTEM_FONT) lose_font(t->font);

	if (font) t->font = find_font(font);

	if (!t->font)
	{
		t->font = SYSTEM_FONT;
		if (font) Msgs_Report(1, "err.font_l", font);
	}
	else
	{
		if (!t->font->bpp_n)
		{
			lose_font(t->font);
			t->font = SYSTEM_FONT;
			if (font) Msgs_Report(1, "err.font_c", font);
		}
	}

	force_term_resize(t);

	return !(t->font == SYSTEM_FONT);
}




/*--------------------------------------------------------------------------*/



/*
 | Create a menu of all the (probable!) fonts in the specified location
 | NB: Any file of type 'data' is considered a font.
 |
 | Subdirectories are recursively searched.
 |
 | 1.10 - Uses <variant>$FontPaths to get a (space separated) list of paths
 | to search.  For each path name, the menu text will be the name and the
 | path searched will be <variant>$<name>$FontPath
 |
 | Eg. (for angband):
 | Angband$FontPaths Zap Angband
 | Angband$Zap$FontPath ZapFonts:
 | Angband$Angband$FontPath Angband:xtra.fonts.
 */
static void make_font_menu(void)
{
	char *t;
	char buffer[260];
	char menu_buffer[260];
	int paths;
	int i;
	unsigned int max_width;
	const char *path[64];	/* pointers to path names */
	menu_item *mi;

	font_menu = NULL;

	/* Get the path (ie. dir) to look under */
	t = getenv(RISCOS_VARIANT "$FontPaths");

	/* Hack: cope if the path isn't set */
	if (!t)
	{
		t = "";
	}

	my_strcpy(buffer, t, sizeof(buffer));

	/*
	   | Count how many paths there are, build an array of pointers to them
	   | and terminate them in the buffer
	 */
	paths = 1;					/* including the system font fake path '<System>' */
	for (t = buffer; *t; t++)
	{
		if (*t == ' ')
		{
			*t = 0;
		}
		else
		{
			if (t == buffer || !t[-1])
			{
				path[paths] = t;
				paths++;
			}
		}
	}

	/*
	   | Create the menu
	 */
	path[0] = SYSTEM_FONT->name;

	font_menu = f_malloc(sizeof(menu_block) + paths * sizeof(menu_item));
	if (!font_menu)
	{
		core("Out of memory (building font menu)");
	}
	memset(font_menu, 0, sizeof(menu_block) + paths * sizeof(menu_item));

	strncpy(font_menu->title, "Fonts", 12);
	font_menu->titlefore = 7;
	font_menu->titleback = 2;
	font_menu->workfore = 7;
	font_menu->workback = 0;
	font_menu->height = 44;
	font_menu->gap = 0;
	max_width = strlen(font_menu->title);

	mi = (menu_item *) (font_menu + 1);

	for (i = 0; i < paths; i++)
	{
		mi[i].submenu.value = -1;
		mi[i].iconflags.data.text = 1;
		mi[i].iconflags.data.filled = 1;
		mi[i].iconflags.data.foreground = 7;
		mi[i].iconflags.data.background = 0;
		strncpy(mi[i].icondata.text, path[i], 12);
		if (strlen(mi[i].icondata.text) > max_width)
			max_width = strlen(mi[i].icondata.text);
	}
	font_menu->width = (max_width + 2) * 16;
	mi[i - 1].menuflags.data.last = 1;

	/*
	   | Hack: add a dotted line after the system font entry if appropriate
	 */
	if (paths > 1) mi[0].menuflags.data.dotted = 1;

	/*
	   | Iterate over the paths, building the appropriate submenus
	 */
	for (i = 1; i < paths; i++)
	{
		menu_ptr sub_menu = NULL;

		sprintf(menu_buffer, "%s$%s$FontPath", RISCOS_VARIANT, path[i]);
		t = getenv(menu_buffer);
		/* Hack: cope if the path isn't defined */
		if (!t)
		{
			t = "";
		}

		/* Fudge so that the fontpath can be a path, not just a dir. */
		my_strcpy(menu_buffer, t, sizeof(menu_buffer));
		for (t = menu_buffer; *t > ' '; t++)
			;
		if (t[-1] == '.')
		{
			t--;
		}
		*t = 0;

		/* Build the menu.  Don't bother if the path variable was empty */
		if (*menu_buffer)
			sub_menu = make_zfont_menu(menu_buffer);

		if (!sub_menu)
		{
			mi[i].iconflags.data.shaded = 1;
		}
		else
		{
			mi[i].submenu.menu = sub_menu;
			/* Override the title of the 'root' sub-menu */
			strncpy(sub_menu->title, path[i], 12);
			/* Add the submenu to the main menu */
		}
	}

	return;
}

/* ----------------------------------------------- musus, xxxx-xx-xx ---
 * Create and set up the infobox.
 * --------------------------------------------------------------------- */
static void create_info_box(void)
{
	info_box = Window_Create("info", template_TITLEMIN);
	Icon_printf(info_box, 0, "%s %s", VARIANT, VERSION);
	Icon_SetText(info_box, 2, AUTHORS);
	Icon_SetText(info_box, 3, PORTERS);
	Icon_SetText(info_box, 7, PORTVERSION);

	return;
}


static BOOL Hnd_ResizeSet(event_pollblock *pb, void *ref)
{
	int newwid = Icon_GetInteger(resize_win, SIZE_WIDTH);
	int newhgt = Icon_GetInteger(resize_win, SIZE_HEIGHT);

	if (pb->type != event_CLICK || pb->data.mouse.button.data.adjust == 0)
	{
		Menu_Show((menu_ptr) -1, -1, -1);
	}

	/* Do simple validation of the values */
	if (menu_term == &data[0])
	{
		if (newwid < 80 || newhgt < 24)
		{
			Msgs_Report(0, "err.minsize1");
			return TRUE;
		}
	}
	else if (newwid < 1 || newhgt < 1)
	{
		Msgs_Report(0, "err.minsize2");
		return TRUE;
	}

	term_change_size(menu_term, newwid, newhgt);

	return TRUE;
}


static BOOL Hnd_ResizeCancel(event_pollblock *pb, void *ref)
{
	/* Close menus */
	Menu_Show((menu_ptr) -1, -1, -1);

	return TRUE;
}


static BOOL Hnd_ResizeKeypress(event_pollblock *pb, void *ref)
{
  if(pb->data.key.code == keycode_RETURN)
    return Hnd_ResizeSet(pb, ref);

  return FALSE;
}


/*
 * Create and set up the term-resize window.
 */
static void create_resize_win(void)
{
	resize_win = Window_Create("resize", template_TITLEMIN);

	Event_Claim(event_CLICK, resize_win, SIZE_SET, Hnd_ResizeSet, NULL);
	Event_Claim(event_CLICK, resize_win, SIZE_CANCEL, Hnd_ResizeCancel, NULL);
	Event_Claim(event_KEY, resize_win, event_ANY, Hnd_ResizeKeypress, NULL);

	return;
}



/*
 | Create the various menus
 */
static void init_menus(void)
{
	char buffer1[256];
	char buffer2[32];
	char *o;

	create_info_box();
	make_font_menu();
	create_resize_win();

	Msgs_Lookup("menu.ibar:Info|>Save As|Full screen,Gamma correction,Sound,"
				"Windows|Save choices|Quit (& save)", buffer1, 256);
	ibar_menu = Menu_New(VARIANT, buffer1);
	if (!ibar_menu) core("Can't create Iconbar menu!");

	Msgs_Lookup("menu.term:Info|>Save As|>Size,Font,Windows", buffer1, 256);
	term_menu = Menu_New(VARIANT, buffer1);
	if (!term_menu) core("Can't create Term menu!");

#ifndef OLD_TERM_MENU
	o = buffer1;
	o += sprintf(buffer1, "%s|", VARIANT);
	o += sprintf(o, "%s,%s,%s|", TERM_NAME(1), TERM_NAME(2),
				 TERM_NAME(3));
	sprintf(o, "%s,%s,%s,%s", TERM_NAME(4), TERM_NAME(5),
			TERM_NAME(6), TERM_NAME(7));
#else
	Msgs_printf(buffer1, "menu.windows:%s|Term-1 (Mirror),Term-2 (Recall),"
				"Term-3 (Choice)|Term-4,Term-5,Term-6,Term-7", VARIANT);
#endif
	Msgs_Lookup("menu.winT:Windows", buffer2, 32);
	wind_menu = Menu_New(buffer2, buffer1);
	if (!wind_menu)
	{
		core("Can't create Windows menu!");
	}

	/* Now attach the various submenus to where they belong */
	Menu_AddSubWindow(ibar_menu, IBAR_MENU_INFO, info_box);
	Menu_AddSubWindow(ibar_menu, IBAR_MENU_GAMMA, gamma_win);
	Menu_AddSubWindow(ibar_menu, IBAR_MENU_SOUND, sound_win);
	Menu_AddSubMenu(ibar_menu, IBAR_MENU_WINDOWS, wind_menu);

	Menu_AddSubMenu(term_menu, TERM_MENU_INFO, (menu_ptr) info_box);
	Menu_AddSubMenu(term_menu, TERM_MENU_WINDOWS, wind_menu);

	/* Add the handler for menu warnings */
	EventMsg_Claim(message_MENUWARN, event_ANY, Hnd_MenuWarning, NULL);

	/* Set up these menu items to issue menuwarn messages */
/*	Menu_Warn(ibar_menu, IBAR_MENU_SAVE, TRUE, NULL, NULL);
	Menu_Warn(term_menu, TERM_MENU_SAVE, TRUE, NULL, NULL);
	Menu_Warn(term_menu, TERM_MENU_SIZE, TRUE, NULL, NULL);*/

	if (font_menu)
		/* add the submenu */
		Menu_AddSubMenu(term_menu, TERM_MENU_FONT, font_menu);
	else
		/* If the font menu is buggered, shade its entry */
		/* unticked, shaded */
		Menu_SetFlags(term_menu, TERM_MENU_FONT, FALSE, TRUE);
}




static void grab_caret(void)
{
	caret_block cb;
	cb.window = data[0].w;
	cb.icon = -1;
	cb.height = 1 << 25;		/* Invisible */
	Wimp_SetCaretPosition(&cb);
}




/*
 | (Recursively) clear all ticks from the specified menu
 */
static void clear_all_menu_ticks(menu_ptr mp)
{
	menu_item *mi = (menu_item *) (mp + 1);

	do
	{
		if (mi->menuflags.data.ticked) mi->menuflags.data.ticked = 0;

		if (mi->submenu.value != -1) clear_all_menu_ticks(mi->submenu.menu);

		mi++;
	}
	while (mi[-1].menuflags.data.last != 1);
}






/*
 | Set the font menu's ticks to match the specifed font name.
 |
 | fm is the (sub) menu to scan down (recursing into it's submenus (if any))
 | fn is the font name to match
 | prefix is the menu text to be prepended to the menu entries due to
 | previous menus (eg. "08x16" will cause fn="08x16.fred" to match menu
 | entry "fred".
 |
 | NB: recursive.
 |
 */

static void set_font_menu_ticks(menu_ptr fm, char *fn, const char *prefix)
{
	char buffer[260];
	char *b_leaf;	/* -> menu 'leaf' text in buffer */
	int pl;	/* prefix string length */
	menu_item *mi = (menu_item *) (fm + 1);

	my_strcpy(buffer, prefix, sizeof(buffer));

	pl = strlen(buffer);
	b_leaf = buffer + pl;

	do
	{
		/* Check for (substring) match */
		strncpy(b_leaf, mi->icondata.text, 12);

		/* Is it a sub-menu? */
		if (mi->submenu.value == -1)
		{
			/* No - must be an exact match */
			mi->menuflags.data.ticked = !strcmp(buffer, fn);
		}
		else
		{
			/* Yes - must be a partial match (with a dot on :) */
			my_strcat(b_leaf, ".", sizeof(buffer) - pl);
			mi->menuflags.data.ticked =
				!strncmp(buffer, fn, pl + strlen(b_leaf));
			if (mi->menuflags.data.ticked)
				set_font_menu_ticks(mi->submenu.menu, fn, buffer);
			else
				clear_all_menu_ticks(mi->submenu.menu);
		}

		/* Next item */
		mi++;
	}
	while (mi[-1].menuflags.data.last != 1);	/* Until finished */
}











/*
 | Set ticks, etc. in the term_menu to reflect the current state of the
 | term 't'
 */
static void set_up_term_menu(term_data *t)
{
	int i;
	menu_ptr mp;
	menu_item *mi;
	window_state ws;

	/* First of all, set up menu title to be the term's title */
	strncpy(term_menu->title, t->name, 12);

	/* Now set the ticks in the Windows> submenu (cuz it's easy) */
	mp = wind_menu;				/* Windows submenu */
	mi = (menu_item *) (mp + 1);	/* First entry */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		Wimp_GetWindowState(data[i].w, &ws);
		mi[i].menuflags.data.ticked = ws.flags.data.open;
	}

	/*
	   | Now, the tricky bit:  find out which font is selected in this
	   | term and tick it in the menu.  Untick all the rest.
	 */
	set_font_menu_ticks(font_menu, t->font->name, "");

	/* Shade the 'Save>' entry if saving isn't possible (yet) */
	if (game_in_progress && character_generated)
		Menu_SetFlags(term_menu, TERM_MENU_SAVE, 0, PDEADCHK);
	else
		Menu_SetFlags(term_menu, TERM_MENU_SAVE, 0, TRUE);

	/*
	 * Shade the resize entry if Term->fixed_shape is set, or if
	 * bigscreen isn't supported
	 */
	if (BIGSCREEN == FALSE || t->t.fixed_shape)
		Menu_SetFlags(term_menu, TERM_MENU_SIZE, 0, TRUE);
	else
		Menu_SetFlags(term_menu, TERM_MENU_SIZE, 0, FALSE);
}



/*
 | Generic 'click' handler for windows - turns a drag on the window
 | into a move-window style drag.
 */
static BOOL Hnd_Click(event_pollblock * pb, void *ref)
{

  if (pb->data.mouse.window == data[0].w)
  {
    if (pb->data.mouse.button.data.select)
    {
	int fw, fh;
        int xpos, ypos;
        wimp_point clickpoint = pb->data.mouse.pos;
        convert_block convert;

	set_up_zrb(&data[0]);

	fw = zrb.r_charw << screen_eig.x;
	fh = zrb.r_charh << screen_eig.y;
	if (zrb.r_flags.bits.double_height)
	{
		fh *= 2;
	}

        /* SO fw & fh are in OS units here */
        Window_GetCoords(data[0].w, &convert);
        Coord_PointToWorkArea(&clickpoint, &convert);

        xpos = clickpoint.x / fw;
        ypos = -clickpoint.y / fh;

        Term_mousepress(xpos, ypos, 1);
        key_pressed = 1;
    }
  }
  else if (pb->data.mouse.button.data.dragselect ||
		pb->data.mouse.button.data.dragadjust)
	{
		drag_block b;
		b.window = pb->data.mouse.window;
		b.screenrect.min.x = b.screenrect.min.y = 0;
		b.screenrect.max.x = screen_size.x;
		b.screenrect.max.y = screen_size.y;
		b.type = drag_MOVEWINDOW;
		Wimp_DragBox(&b);
	}

	return TRUE;
}



/*
 | Handle a click on a Term window.
 */
static BOOL Hnd_TermClick(event_pollblock * pb, void *ref)
{
	term_data *t = (term_data *)ref;

	if (pb->data.mouse.button.data.menu)
	{
		menu_term = t;
		set_up_term_menu(t);
		Menu_Show(term_menu, pb->data.mouse.pos.x - 32,
				  pb->data.mouse.pos.y + 32);
	}
	else
	{
		grab_caret();
		Hnd_Click(pb, ref);
	}

	return TRUE;
}

#endif /* !FULLSCREEN_ONLY */



static void mark_ood(term_data *t, int minx, int miny, int maxx, int maxy)
{
	if (t->changed_box.min.x > minx)
		t->changed_box.min.x = minx;
	if (t->changed_box.max.x < maxx)
		t->changed_box.max.x = maxx;
	if (t->changed_box.min.y > miny)
		t->changed_box.min.y = miny;
	if (t->changed_box.max.y < maxy)
		t->changed_box.max.y = maxy;
}


#ifndef FULLSCREEN_ONLY

/* Check for an event (ie. key press) */
static errr Term_xtra_acn_check(void)
{
	static int last_poll = 0;
	unsigned int curr_time;
	int bh, bl;

	/*
	   | Only poll the wimp if there's something in the keyboard buffer
	   | or every 10cs.  This is presumably so that we process as many
	   | keypresses as possible before any other application gets a go.
	 */

	/* Check the kbd buffer */
	SWI(3, 3, SWI_OS_Byte, 128, 255, 0, /**/ NULL, &bl, &bh);
	bl = (bl & 0xff) + (bh << 8);

	/* Check how long it is since we last polled */
	curr_time = Time_Monotonic();

	if ((bl > 0 && got_caret) || ((curr_time - last_poll) > 9))
	{
		last_poll = curr_time;
		STOP_HOURGLASS;
		Event_Poll();
		START_HOURGLASS;
	}

	/*
	   | This allows the user to interrupt the borg.
	 */
	if (key_pressed) return key_pressed = 0;

	return 1;
}


/*
 | Wait for an event (ie. keypress)
 | Note that we idle poll once a second to allow us to implement the
 | alarm system.
 */
static errr Term_xtra_acn_event(void)
{
	STOP_HOURGLASS;

	while (!key_pressed && !fullscreen_font)
	{
		Event_PollIdle(100);
	}
	START_HOURGLASS;

	return key_pressed = 0;
}



/* React to changes (eg. palette change) */
static errr Term_xtra_acn_react(void)
{
	int c;

	cache_palette();

	/* Mark the entirety of each window as out of date */
	for (c = 0; c < MAX_TERM_DATA; c++)
	{
		mark_ood(&data[c], 0, 0, data[c].t.wid, data[c].t.hgt);
	}

	/* Force a redraw of the windows */
	refresh_windows();

	/* Success */
	return 0;
}

#endif /* FULLSCREEN_ONLY */

/* Do various things to a term */
static errr Term_xtra_acn(int n, int v)
{
	term_data *t = (term_data *)Term;

	switch (n)
	{
		/* Clear the Term */
		case TERM_XTRA_CLEAR:
		{
#ifndef FULLSCREEN_ONLY
			if (fullscreen_font)
			{
#endif
				if (t == (&data[0])) Term_xtra_acn_clearFS();
#ifndef FULLSCREEN_ONLY
			}
			else
			{
				mark_ood(t, 0, 0, Term->wid, Term->hgt);
			}
#endif
			return 0;
		}

		/* Wait/check for an event */
		case TERM_XTRA_EVENT:
		{
#ifndef FULLSCREEN_ONLY
			if (fullscreen_font)
			{
#endif
				Term_xtra_acn_eventFS(v);
#ifndef FULLSCREEN_ONLY
			}
			else
			{
				if (v) return Term_xtra_acn_event();
				else return Term_xtra_acn_check();
			}
#endif
		}

		/* Bored */
		case TERM_XTRA_BORED:
		{
#ifndef FULLSCREEN_ONLY
			if (fullscreen_font)
#endif
				Term_xtra_acn_eventFS(0);
#ifndef FULLSCREEN_ONLY
			else return Term_xtra_acn_check();
#endif
		}

		/* Flush input */
		case TERM_XTRA_FLUSH:
		{
#ifndef FULLSCREEN_ONLY
			if (fullscreen_font || got_caret)
			{
#endif
				/* 1.21 - Hack: wait until no keys are pressed */
				if (hack_flush)
				{
					v = 0;
					while (v != 0xff) SWI(1, 2, SWI_OS_Byte, 122, 0, &v);
				}

                /* Flush Kbd buffer */
				SWI(3, 0, SWI_OS_Byte, 21, 0, 0);
#ifndef FULLSCREEN_ONLY
			}
#endif
			return 0;
		}

		/* Flush output */
		case TERM_XTRA_FRESH:
		{
#ifndef FULLSCREEN_ONLY
			if (!fullscreen_font) refresh_window(t);
#endif
			return 0;
		}

        /* Ensure line 'v' is plotted */
		case TERM_XTRA_FROSH:
		{
			/* Doesn't do anything */
			return 0;
		}

		/* Set cursor visibility */
		case TERM_XTRA_SHAPE:
		{
#ifndef FULLSCREEN_ONLY
			if (fullscreen_font)
			{
#endif
				if (t == (&data[0]))
				{
					t->cursor.visible = v;
					if (v)
						draw_cursor(t->cursor.pos.x, t->cursor.pos.y);
					else
						redraw_areaFS(t->cursor.pos.x, t->cursor.pos.y, 1, 1);
				}
#ifndef FULLSCREEN_ONLY
			}
			else
			{
				t->cursor.visible = v ? TRUE : FALSE;
				mark_ood(t, t->cursor.pos.x, t->cursor.pos.y,
						 t->cursor.pos.x + 1, t->cursor.pos.y + 1);

				refresh_window(t);	/* needed? */
			}
#endif
			return 0;
		}

		/* Make a beep */
		case TERM_XTRA_NOISE:
		{
			Sound_SysBeep();
			return 0;
		}

		/* React to, eg. palette changes */
		case TERM_XTRA_REACT:
		{
#ifndef FULLSCREEN_ONLY
			if (fullscreen_font)
			{
#endif
				return Term_xtra_acn_reactFS(FALSE);
#ifndef FULLSCREEN_ONLY
			}
			else
			{
				return Term_xtra_acn_react();
			}
#endif
		}

		/* Delay for 'v' ms */
		case TERM_XTRA_DELAY:
		{
			if (v > 0)
			{
				unsigned int start = Time_Monotonic();

				/* Round to nearest cs */
				v = (v + 5) / 10;

				/* Wait for vsync for the hell of it. */
				GFX_Wait();
				while ((Time_Monotonic() - start) < v);
			}
			return 0;
		}

/*
 * This is used by ToME2, and presumably will never be picked up by other
 * variants, so it should be safe to #ifdef out like so:
 */
#ifdef TERM_XTRA_SCANSUBDIR
		/* Subdirectory scan */
		case TERM_XTRA_SCANSUBDIR:
		{
		    filing_dirdata directory;
		    filing_direntry *entry;

			scansubdir_max = 0;

			if (Filing_OpenDir(riscosify_name(scansubdir_dir), &directory, sizeof(filing_direntry), readdirtype_DIRENTRY) != NULL)
			{
			  Error_Report(0, "Couldn't open directory \"%s\"", riscosify_name(scansubdir_dir));
			  return 0;
			}

			while ((entry = Filing_ReadDir(&directory)) != NULL)
			{
				if (entry->objtype == filing_DIRECTORY)
				{    
					string_free(scansubdir_result[scansubdir_max]);
					scansubdir_result[scansubdir_max] = string_make(entry->name);
					++scansubdir_max;
				}
			}

			Filing_CloseDir(&directory);

			return 0;
		}
#endif /* TERM_XTRA_SCANSUBDIR */


/*
 * This is used by ToME, and presumably will never be picked up by other
 * variants, so it should be safe to #ifdef out like so:
 */
#ifdef TERM_XTRA_GET_DELAY
		/* Return current "time" in milliseconds */
		case TERM_XTRA_GET_DELAY:
		{
			Term_xtra_long = Time_Monotonic() * 100;

			return 0;
		}
#endif /* TERM_XTRA_GET_DELAY */
                  
/*
 * This is used by ToME, and presumably will never be picked up by other
 * variants, so it should be safe to #ifdef out like so:
 */
#ifdef TERM_XTRA_RENAME_MAIN_WIN
#ifndef FULLSCREEN_ONLY
		/* Rename main window */
		case TERM_XTRA_RENAME_MAIN_WIN:
		{
			Window_SetTitle(data[0].w, angband_term_name[0]);
			return 0;
		}
#endif /* FULLSCREEN_ONLY */
#endif /* TERM_XTRA_RENAME_MAIN_WIN */

		default:
			return 1;			/* Unsupported */
	}
}


#ifndef FULLSCREEN_ONLY


/* Move (but don't necessarily display) the cursor */
static errr Term_curs_acn(int x, int y)
{
	term_data *t = (term_data *)Term;

	if (t->cursor.visible)
		mark_ood(t, t->cursor.pos.x, t->cursor.pos.y,
				 t->cursor.pos.x + 1, t->cursor.pos.y + 1);

	t->cursor.pos.x = x;
	t->cursor.pos.y = y;

	if (t->cursor.visible)
		mark_ood(t, t->cursor.pos.x, t->cursor.pos.y,
				 t->cursor.pos.x + 1, t->cursor.pos.y + 1);

	return 0;
}



/*
 | NB: these two are very simple since we use the Term's contents
 | directly to generate the r_data for ZapRedraw.
 */

/* Erase 'n' characters at (x,y) */
static errr Term_wipe_acn(int x, int y, int n)
{
	mark_ood((term_data *)Term, x, y, x + n, y + 1);
	return 0;
}

/* Write 'n' characters from 's' with attr 'a' at (x,y) */
static errr Term_text_acn(int x, int y, int n, byte a, cptr s)
{
	mark_ood((term_data *)Term, x, y, x + n, y + 1);
	return 0;
}

#endif /* !FULLSCREEN_ONLY */


/* Initialise one of our terms */
static void Term_init_acn(term *t)
{
	term_data *term = (term_data *)t;

	/* Ludicrous changed box settings :) */
	term->changed_box.min.x = 256;
	term->changed_box.min.y = 256;
	term->changed_box.max.x = 0;
	term->changed_box.max.y = 0;
}



static void term_data_link(term_data *td, int k)
{
	term *t = &(td->t);

	/* Initialise the term */
	term_init(t, 80, 24, k);

	/* Set flags and hooks */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Experiment (FS mode requires them) */
	t->always_text = TRUE;
	t->never_frosh = TRUE;
	/* Experiment (FS mode requires them) */

#ifdef FULLSCREEN_ONLY
	t->wipe_hook  = Term_wipe_acnFS;
	t->xtra_hook  = Term_xtra_acnFS;
	t->curs_hook  = Term_curs_acnFS;
	t->text_hook  = Term_text_acnFS;
#else
	t->init_hook = Term_init_acn;
	t->xtra_hook = Term_xtra_acn;
	t->wipe_hook = Term_wipe_acn;
	t->curs_hook = Term_curs_acn;
	t->text_hook = Term_text_acn;
	t->user_hook = Term_user_acn;
#endif /* FULLSCREEN_ONLY */

	t->data = td;

	Term_activate(t);
}


#ifndef FULLSCREEN_ONLY

/* Open default windows (ie. as set in choices) at the appropriate sizes */
static void show_windows(void)
{
	int i;
	for (i = MAX_TERM_DATA; i-- > 0;)
	{
		if (!data[i].unopened)
		{
			if (data[i].def_open)
				Window_Show(data[i].w, open_WHEREVER);
		}
		else
		{
			if (data[i].def_open)
			{
				window_openblock ob;
				ob.window = data[i].w;
				ob.screenrect = data[i].def_pos;
				ob.scroll = data[i].def_scroll;
				ob.behind = -1;
				Wimp_OpenWindow(&ob);
				data[i].unopened = 0;
			}
		}
	}
}


/*
 | 'ref' is used to indicate whether this close is being forced by some other
 | part of the code (eg. the Windows> submenu code).  This is used to modify
 | the 'adjust doesn't close other terms' behavior.
 */

static BOOL Hnd_MainClose(event_pollblock * pb, void *ref)
{
	int i;
	window_state ws;
	mouse_block mb;

	/* New in 1.08: don't close other Terms if closed with adjust */
	Wimp_GetPointerInfo(&mb);
	if (ref || mb.button.data.adjust)
	{
		Wimp_CloseWindow(data[0].w);
	}
	else
	{
		/* Close all the terms, but mark the open ones as 'def_open' */
		for (i = 0; i < MAX_TERM_DATA; i++)
		{
			Wimp_GetWindowState(data[i].w, &ws);
			if (!ws.flags.data.open)
			{
				data[i].def_open = 0;
			}
			else
			{
				Wimp_CloseWindow(data[i].w);
				data[i].def_open = 1;
			}
		}
	}

	return TRUE;
}


static BOOL Hnd_PaletteChange(event_pollblock * pb, void *ref)
{
	cache_palette();
	return TRUE;
}



static BOOL Hnd_ModeChange(event_pollblock * pb, void *ref)
{
	int i;
	Screen_CacheModeInfo();
	set_up_zrb_for_mode();		/* (re)set up the redraw block */
	cache_palette();			/* (re)cache the palette */
	cache_fonts();				/* (re)cache the fonts */

	/* Enforce sizes (eg. if screen_eig.y has changed) */
	for (i = 0; i < MAX_TERM_DATA; i++) force_term_resize(&(data[i]));

	return TRUE;
}




static BOOL Hnd_Keypress(event_pollblock * pb, void *ref)
{
	static const char hex[] = "0123456789ABCDEF";
	int c = pb->data.key.code;
	/* Check whether this key was pressed in Term 0 */
	if (pb->data.key.caret.window == data[0].w)
	{
		switch (c)
		{
			case keycode_F12:
			case keycode_SHIFT_F12:
			case keycode_CTRL_F12:
			case keycode_CTRL_SHIFT_F12:
				/* Never intercept these */
				break;

			case 27:			/* handle escape specially */
				if (Kbd_KeyDown(inkey_CTRL))
				{
					ack_alarm();
					return TRUE;
				}

			/* Send everything else onto the Term package */
			default:
				/* Take care of "special" keypresses */
				switch (c)
				{
					case keycode_TAB:
					{
						c = '\t';
						break;
					}

					case keycode_PAGEUP:
					{
						c = '9';
						break;
					}

					case keycode_PAGEDOWN:
					{
						c = '3';
						break;
					}

					case keycode_COPY:
					{
						c = '1';
						break;
					}

					case keycode_HOME:
					{
						c = '7';
						break;
					}
				}
				/* Pass to the angband engine */
				/* Allow shift & ctrl to modify the keypad keys */
				if (c >= '0' && c <= '9')
				{
					kbd_modifiers m = Kbd_GetModifiers(FALSE);
					if (m.shift)
					{
						c |= 0x800;
					}
					if (m.ctrl)
					{
						c |= 0x400;
					}
					/* Could maybe add ALT as 0x1000 ??? */
				}

				/* Keys >255 have to be send as escape sequences (31=escape) */
				if (c > 255 || c == 31)
				{
					Term_keypress(31);
					Term_keypress(hex[(c & 0xf00) >> 8]);
					Term_keypress(hex[(c & 0x0f0) >> 4]);
					Term_keypress(hex[(c & 0x00f)]);
					c = 13;
				}
				Term_keypress(c);
				key_pressed = 1;
				/*if ( c==27 ) { escape_pressed = 1; } */
				return TRUE;
		}
	}

	Wimp_ProcessKey(c);
	return TRUE;
}


/*--------------------------------------------------------------------------*/
/* Gamma correction window stuff											*/
/*--------------------------------------------------------------------------*/

static void redraw_gamma(window_redrawblock * rb, BOOL *more)
{
	int i, y, x, h, w;
	int bx, by;
	int dither;

	bx = Coord_XToScreen(GC_XOFF, (convert_block *) & (rb->rect));
	by = Coord_YToScreen(GC_YOFF, (convert_block *) & (rb->rect));

	h = GC_HEIGHT / 4;
	w = GC_WIDTH / 16;

	x = bx;

	while (*more)
	{
		for (i = 0; i < 16; i++)
		{
			y = by;
			for (dither = 0; dither < 2; dither++)
			{
				/* Solid block: */
				ColourTrans_SetGCOL(palette[i], dither << 8, 0);
				GFX_RectangleFill(x, y, w, -h);
				y -= h;
				/* Dot on black: */
				ColourTrans_SetGCOL(palette[0], dither << 8, 0);
				GFX_RectangleFill(x, y, w, -h);
				ColourTrans_SetGCOL(palette[i], dither << 8, 0);
				GFX_RectangleFill(x + (w / 2) - 2, y - (h / 2), 2, 2);
				y -= h;
			}
			x += w;
		}
		Wimp_GetRectangle(rb, more);
	}
}


static void update_gamma(void)
{
	window_redrawblock rb;
	BOOL more;

	rb.window = gamma_win;
	rb.rect.min.x = GC_XOFF;
	rb.rect.min.y = GC_YOFF - GC_HEIGHT;
	rb.rect.max.y = GC_XOFF + GC_WIDTH + screen_delta.x;
	rb.rect.max.y = GC_YOFF + screen_delta.y;

	Wimp_UpdateWindow(&rb, &more);
	if (more) redraw_gamma(&rb, &more);
}



static BOOL Hnd_RedrawGamma(event_pollblock * pb, void *ref)
{
	window_redrawblock rb;
	BOOL more;

	rb.window = pb->data.openblock.window;
	Wimp_RedrawWindow(&rb, &more);
	if (more) redraw_gamma(&rb, &more);

	return TRUE;
}


static BOOL Hnd_GammaClick(event_pollblock * pb, void *ref)
{
	int up = (ref == 0);

	if (up)
	{
		if (gamma < 900)
		{
			gamma += 5;
			Icon_SetDouble(gamma_win, 0, (double) gamma / 100, 2);
			update_gamma();
			Term_xtra_acn_react();
		}
	}
	else
	{
		if (gamma > 5)
		{
			gamma -= 5;
			Icon_SetDouble(gamma_win, GAMMA_ICN, (double) gamma / 100, 2);
			update_gamma();
			Term_xtra_acn_react();
		}
	}

	/* Hack: if the user menu is active then force it to redraw */
	if (user_menu_active)
	{
		Term_keypress(18);
		key_pressed = 1;
	}

	return TRUE;
}

/*
 | Reflect the current options in the gamma window
 */
static void set_gamma_window_state(void)
{
	if (minimise_memory) return;

	Icon_SetDouble(gamma_win, 0, (double) gamma / 100, 2);
}


static void init_gamma_window(void)
{
	if (minimise_memory) return;

	gamma_win = Window_Create("gamma", template_TITLEMIN);
	Event_Claim(event_REDRAW, gamma_win, event_ANY, Hnd_RedrawGamma, 0);
	Event_Claim(event_CLICK, gamma_win, GAMMA_DOWN, Hnd_GammaClick, (void *)1);
	Event_Claim(event_CLICK, gamma_win, GAMMA_UP, Hnd_GammaClick, (void *)0);
	set_gamma_window_state();
}


/*--------------------------------------------------------------------------*/
/* Sound options window stuff												*/
/*--------------------------------------------------------------------------*/

static slider_info volume_slider;


/*
 | Reflect the current sound config in the sound options window
 */
static void set_sound_window_state(void)
{
	if (minimise_memory) return;

	Icon_SetSelect(sound_win, SND_ENABLE, enable_sound);
	Slider_SetValue(&volume_slider, sound_volume, NULL, NULL);

	if (sound_volume > 127)
		volume_slider.colour.foreground = colour_RED;
	else
		volume_slider.colour.foreground = colour_GREEN;
}



/*
 | The sound slider has been dragged, so update the sound volume setting
 */
static int update_volume_from_slider(slider_info *si, void *ref)
{
	sound_volume = Slider_ReadValue(si);

	if (sound_volume > 127)
		volume_slider.colour.foreground = colour_RED;
	else
		volume_slider.colour.foreground = colour_GREEN;

	return 0;
}


/*
 | Handle redraw events for the sound options window
 */
static BOOL Hnd_RedrawSnd(event_pollblock * pb, void *ref)
{
	window_redrawblock redraw;
	BOOL more;

	redraw.window = pb->data.openblock.window;
	Wimp_RedrawWindow(&redraw, &more);

	while (more)
	{
		Slider_Redraw(((slider_info *) ref), &redraw.cliprect);
		Wimp_GetRectangle(&redraw, &more);
	}
	return (TRUE);
}


/*
 | Handle clicks on the sound options window
 */
static BOOL Hnd_SndClick(event_pollblock * pb, void *ref)
{
	int icn = pb->data.mouse.icon;
	int adj = pb->data.mouse.button.data.adjust;
	slider_info *si = (slider_info *) ref;

	switch (icn)
	{
			/* Bump arrows for the slider: */
		case SND_VOL_DOWN:
			adj = !adj;

		case SND_VOL_UP:
			adj = adj ? -1 : 1;
			sound_volume += adj;
			if (sound_volume < SOUND_VOL_MIN)
			{
				sound_volume = SOUND_VOL_MIN;
			}
			if (sound_volume > SOUND_VOL_MAX)
			{
				sound_volume = SOUND_VOL_MAX;
			}
			set_sound_window_state();
			break;

			/* The slider itself */
		case SND_VOL_SLIDER:
			Icon_ForceRedraw(sound_win, SND_VOL_SLIDER);
			Slider_Drag(si, NULL, NULL, NULL);
			break;

			/* The enable/disable icon */
		case SND_ENABLE:
			enable_sound = !enable_sound;
			Icon_SetSelect(sound_win, SND_ENABLE, enable_sound);
			if (enable_sound)
			{
				initialise_sound();
			}
			break;
	}

	/* Hack: if the user menu is active then force it to redraw */
	if (user_menu_active)
	{
		Term_keypress(18);
		key_pressed = 1;
	}

	return TRUE;
}



/*
 | Set the sound options window up, ready to rock :)
 */
static void init_sound_window(void)
{
	sound_win = Window_Create("sound", template_TITLEMIN);

	Event_Claim(event_REDRAW, sound_win, event_ANY, Hnd_RedrawSnd,
				(void *)&volume_slider);
	Event_Claim(event_CLICK, sound_win, event_ANY, Hnd_SndClick,
				(void *)&volume_slider);

	/* Set up the slider info */
	volume_slider.window = sound_win;
	volume_slider.icon = SND_VOL_SLIDER;
	volume_slider.limits.min = SOUND_VOL_MIN;
	volume_slider.limits.max = SOUND_VOL_MAX;
	volume_slider.colour.foreground = colour_GREEN;
	volume_slider.colour.background = colour_WHITE;
	volume_slider.border.x = 8;
	volume_slider.border.y = 8;
	volume_slider.update = update_volume_from_slider;

	set_sound_window_state();
}





/*--------------------------------------------------------------------------*/











/*
 | A font has been selected.
 | At this point, menu_term is a pointer to the term for which the
 | menu was opened.
 */
static void handle_font_selection(int *s)
{
	char name[260];
	os_error *e;
	char *r;
	menu_ptr mp = font_menu;
	int *mis;

	/* Follow the >s to the entry specified */
	for (mis = s; *mis != -1; mis++)
		mp = ((menu_item *) (mp + 1))[*mis].submenu.menu;

	/*
	   | Now, check to see if we've hit a leaf entry.
	   | NB: If the entry isn't a leaf entry then the first entry in its submenu
	   | is used instead
	 */
	if (((int)mp) != -1)
	{
		mis[0] = 0;
		mis[1] = -1;
		mp = ((menu_item *) (mp + 1))[0].submenu.menu;
	}

	if (((int)mp) != -1)
		return;

	e = Wimp_DecodeMenu(font_menu, s, name);
	if (e)
	{
		plog(e->errmess);
		return;
	}

	/* Make sure that the string is NULL terminated */
	for (r = name; *r >= ' '; r++)
		;
	*r = 0;

	attach_font_to_term(menu_term, name);
	mark_ood(menu_term, 0, 0, menu_term->t.wid, menu_term->t.hgt);
	refresh_window(menu_term);
}


#endif /* FULLSCREEN_ONLY */




static void load_choices(void)
{
	FILE *fp = NULL;
	char *cf;
	int i;
	char buffer[260];
	int choices_version = 0;

	cf = find_choices(FALSE);
	if (*cf)
		fp = fopen(cf, "r");

	/* Implement default choices */
	data[0].def_open = 1;
	data[0].unopened = 1;		/* ie. force def_pos */
	data[0].def_pos.min.x = (screen_size.x - 1280) / 2;
	data[0].def_pos.max.x = (screen_size.x + 1280) / 2;
	data[0].def_pos.min.y = (screen_size.y - 768) / 2 - 32;
	data[0].def_pos.max.y = (screen_size.y + 768) / 2 - 32;
	data[0].def_scroll.x = data[0].def_scroll.y = 0;
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		data[i].def_open = 0;
		data[i].unopened = 1;	/* ie. force def_pos */
		data[i].def_pos.min.x = (screen_size.x - 1280) / 2;
		data[i].def_pos.max.x = (screen_size.x + 1280) / 2;
		data[i].def_pos.min.y = (screen_size.y - 768) / 2;
		data[i].def_pos.max.y = (screen_size.y + 768) / 2;
		data[i].def_scroll.x = data[i].def_scroll.y = 0;
	}

	if (fp)
	{
		const char *t_;
		char *o_;

		if (!fgets(buffer, sizeof(buffer), fp))
		{
			fclose(fp);
			return;
		}

		if (memcmp(buffer, "[Angband config, Musus' port]\n", 30) == 0)
		{
			choices_version = 1;
		}
		else if (memcmp(buffer, "[Angband config v2, Musus' port]\n", 33) == 0)
		{
			choices_version = 2;
		}

		if (choices_version == 0)
		{
			fclose(fp);
			return;
		}

		/* Load choices */
		while (fgets(buffer, sizeof(buffer), fp))
		{
			t_ = strtok(buffer, " ");	/* Term number (or keyword, "Gamma", etc.) */
			o_ = strtok(NULL, "\n");	/* argument string */
			if (!o_)
			{
				o_ = "";
			}					/* missing (or null) argument? */
			if (t_)
			{
				if (!strcmp(t_, "Gamma"))
					gamma = atof(o_) * 100;
				else if (!strcmp(t_, "Monochrome"))
					force_mono = !strcmp(o_, "on");
				else if (!strcmp(t_, "Sound"))
					enable_sound = !strcmp(o_, "on");
				else if (!strcmp(t_, "Volume"))
					sound_volume = atoi(o_);
				else if (!strcmp(t_, "FullScreen"))
					start_fullscreen = !strcmp(o_, "on");
				else if (!strcmp(t_, "Hourglass"))
					use_glass = !strcmp(o_, "on");
				else if (!strcmp(t_, "HackFlush"))
					hack_flush = !strcmp(o_, "on");
				else if (!strcmp(t_, "AlarmTimeH"))
					alarm_h = atoi(o_);
				else if (!strcmp(t_, "AlarmTimeM"))
					alarm_m = atoi(o_);
				else if (!strcmp(t_, "AlarmText"))
					strcpy(alarm_message, o_);
				else if (!strcmp(t_, "AlarmBeep"))
					alarm_beep = !strcmp(o_, "on");
				else if (!strcmp(t_, "AlarmType"))
				{
					int i;
					for (i = 0; i < 4; i++)
						if (!strcmp(alarm_types[i], o_))
							alarm_type = i;
				}
				else if (isdigit((unsigned char)*t_))
				{
					int t = atoi(t_);
					if (t >= 0 && t < MAX_TERM_DATA)
					{
						char *f, *x0, *y0, *x1, *y1, *sx, *sy, *dx = NULL, *dy = NULL;
						o_ = strtok(o_, " ");	/* first word */
						f = strtok(NULL, " ");	/* font name */
						x0 = strtok(NULL, " ");	/* x posn (min) */
						y0 = strtok(NULL, " ");	/* y posn (min) */
						x1 = strtok(NULL, " ");	/* x posn (max) */
						y1 = strtok(NULL, " ");	/* y posn (max) */
						sx = strtok(NULL, " ");	/* x scroll offset */

						if (choices_version == 1)
						{
							sy = strtok(NULL, "\n");	/* y scroll offset */
						}
						else
						{
							sy = strtok(NULL, " ");	/* y scroll offset */
							dx = strtok(NULL, " ");	/* width */
							dy = strtok(NULL, " ");	/* height */
						}

						data[t].def_open = (t == 0) || atoi(o_);
						data[t].def_pos.min.x = atoi(x0);
						data[t].def_pos.min.y = atoi(y0);
						data[t].def_pos.max.x = atoi(x1);
						data[t].def_pos.max.y = atoi(y1);
						data[t].def_scroll.x = atoi(sx);
						data[t].def_scroll.y = atoi(sy);
						data[t].unopened = 1;	/* ie. force def_pos */
#ifndef FULLSCREEN_ONLY
						/* Ensure we have size before setting it. */
						if (dx && dy)
						{
							term_change_size(&data[t], atoi(dx), atoi(dy));
						}

						attach_font_to_term(&(data[t]), f);
#endif /* FULLSCREEN_ONLY */
					}
				}
			}
		}
		fclose(fp);
	}
}




static void save_choices(void)
{
	FILE *fp = NULL;
	FILE *fpm = NULL;
	char *cf;
	int i;

	write_alarm_choices();

	cf = find_choices(TRUE);
	if (!*cf)
	{
		plog("Failed to locate writable choices file!");
		return;
	}

	fp = fopen(cf, "w");
	if (!fp)
	{
		plog("Can't write choices file");
		return;
	}

	fpm = fopen(find_choices_mirror(), "w");

	f2printf(fp, fpm, "[Angband config v2, Musus' port]\n");
	f2printf(fp, fpm, "Gamma %.2lf\n", (double) gamma / 100);
	f2printf(fp, fpm, "Monochrome %s\n", force_mono ? "on" : "off");
	f2printf(fp, fpm, "Sound %s\n", enable_sound ? "on" : "off");
	f2printf(fp, fpm, "Volume %d\n", sound_volume);
	f2printf(fp, fpm, "FullScreen %s\n", start_fullscreen ? "on" : "off");
	f2printf(fp, fpm, "Hourglass %s\n", use_glass ? "on" : "off");
	f2printf(fp, fpm, "HackFlush %s\n", hack_flush ? "on" : "off");

	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		window_state ws;
		Wimp_GetWindowState(data[i].w, &ws);
		f2printf(fp, fpm, "%d %d %s ", i, ws.flags.data.open,
				 data[i].font->name);
		f2printf(fp, fpm, "%d ", ws.openblock.screenrect.min.x);
		f2printf(fp, fpm, "%d ", ws.openblock.screenrect.min.y);
		f2printf(fp, fpm, "%d ", ws.openblock.screenrect.max.x);
		f2printf(fp, fpm, "%d ", ws.openblock.screenrect.max.y);
		f2printf(fp, fpm, "%d %d ", ws.openblock.scroll.x,
				 ws.openblock.scroll.y);
		f2printf(fp, fpm, "%d %d\n", data[i].t.wid, data[i].t.hgt);
	}

	fclose(fp);

	if (fpm)
	{
		fclose(fpm);
	}
}

/*
 | Update the Alarm choices file to reflect changed alarm settings.
 */
static void write_alarm_choices(void)
{
	FILE *fp;
	char *cf;

	/* Open the choices file for reading */
	cf = find_alarmfile(TRUE);
	if (!*cf)
	{
		plog("Can't determine Alarm file location!");
		return;
	}

	fp = fopen(cf, "w");
	if (!fp)
	{
		plog("Can't write Alarm file");
		return;
	}

	/* Write the new alarm options */
	fprintf(fp, "AlarmType %s\n", alarm_types[alarm_type]);
	fprintf(fp, "AlarmTimeH %d\n", alarm_h);
	fprintf(fp, "AlarmTimeM %d\n", alarm_m);
	fprintf(fp, "AlarmText %s\n", alarm_message);
	fprintf(fp, "AlarmBeep %s\n", alarm_beep ? "on" : "off");

	fclose(fp);
}

/*
 | Read the Alarm choices file.
 */
static void read_alarm_choices(void)
{
	char buffer[260];
	FILE *fp;
	char *cf;

	cf = find_alarmfile(FALSE);
	if (!*cf)
	{
		return;
	}

	fp = fopen(cf, "r");
	if (fp)
	{
		const char *t_, *o_;
		/* Load choices */
		while (fgets(buffer, sizeof(buffer), fp))
		{
			t_ = strtok(buffer, " ");	/* Keyword */
			o_ = strtok(NULL, "\n");	/* argument string */
			if (!o_)
			{
				o_ = "";
			}					/* missing (or null) argument? */
			if (t_)
			{
				if (!strcmp(t_, "AlarmTimeH"))
					alarm_h = atoi(o_);
				else if (!strcmp(t_, "AlarmTimeM"))
					alarm_m = atoi(o_);
				else if (!strcmp(t_, "AlarmText"))
					strcpy(alarm_message, o_);
				else if (!strcmp(t_, "AlarmBeep"))
					alarm_beep = !strcmp(o_, "on");
				else if (!strcmp(t_, "AlarmType"))
				{
					int i;
					for (i = 0; i < 4; i++)
						if (!strcmp(alarm_types[i], o_))
							alarm_type = i;
				}
			}
		}
		fclose(fp);
	}
}



#ifndef FULLSCREEN_ONLY
/*
 | Handle selections from the term menu(s)
 */
static BOOL Hnd_TermMenu(event_pollblock * pb, void *ref)
{
	mouse_block mb;
	int i;

	Wimp_GetPointerInfo(&mb);

	switch (pb->data.selection[0])
	{
		case TERM_MENU_INFO:	/* Info> */
			break;
		case TERM_MENU_FONT:	/* Font> */
			/* Sub item selected? */
			if (pb->data.selection[1] == -1)
			{
				break;
			}
			handle_font_selection(pb->data.selection + 1);
			break;
		case TERM_MENU_WINDOWS:	/* Windows> */
			if (pb->data.selection[1] == -1)
			{
				break;
			}
			i = pb->data.selection[1];
			{
				window_state ws;
				Wimp_GetWindowState(data[i].w, &ws);
				if (ws.flags.data.open)
				{
					if (!i)
						Hnd_MainClose(NULL, (void *)TRUE);
					else
						Window_Hide(data[i].w);
				}
				else
				{
					if (!i)
					{
						show_windows();
						grab_caret();
					}
					else
					{
						if (!data[i].unopened)
						{
							Window_Show(data[i].w, open_WHEREVER);
						}
						else
						{
							window_openblock ob;
							ob.window = data[i].w;
							ob.screenrect = data[i].def_pos;
							ob.scroll = data[i].def_scroll;
							ob.behind = -1;	/* could use data[0].w; ? */
							Wimp_OpenWindow(&ob);
							data[i].unopened = 0;
						}
					}
				}
			}
			break;
	}

	if (mb.button.data.adjust)
	{
		set_up_term_menu(menu_term);
		Menu_ShowLast();
	}

	return TRUE;
}






/*
 | Handle selections from the iconbar menu
 */
static BOOL Hnd_IbarMenu(event_pollblock * pb, void *ref)
{
	mouse_block mb;
	Wimp_GetPointerInfo(&mb);

	switch (pb->data.selection[0])
	{
		case IBAR_MENU_INFO:	/* Info> */
			break;
		case IBAR_MENU_FULLSCREEN:	/* Full screen */
			/* Do Full Screen mode */
			enter_fullscreen_mode();
			break;
		case IBAR_MENU_GAMMA:	/* Gamma correction */
			break;
		case IBAR_MENU_SOUND:	/* Sound */
			/*
			   | enable_sound = !enable_sound;
			   | if ( enable_sound ) { initialise_sound(); }
			   | Menu_SetFlags( ibar_menu, IBAR_MENU_SOUND, enable_sound, 0 );
			   | set_sound_window_state();
			 */
			break;
		case IBAR_MENU_WINDOWS:	/* Windows> */
			/*
			   | Hack: pass it off as the equivalent selection from
			   | the term menu.
			 */
			pb->data.selection[0] = TERM_MENU_WINDOWS;
			return Hnd_TermMenu(pb, ref);
			break;
		case IBAR_MENU_SAVECHOICES:	/* Save choices */
			save_choices();
			break;
		case IBAR_MENU_QUIT:	/* Quit */
			if (game_in_progress && character_generated)
			{
				if (inkey_flag)
				{
					save_player(SAVE_PLAYER_PARAM);
					quit(NULL);
				}
				else
				{
					Msgs_Report(0, "err.nosave");
				}
			}
			break;
	}

	if (mb.button.data.adjust)
		Menu_ShowLast();

	return TRUE;
}





static BOOL Hnd_MenuSel(event_pollblock * pb, void *ref)
{
	if (menu_currentopen == ibar_menu)
		return Hnd_IbarMenu(pb, ref);
	else if (menu_currentopen == term_menu)
		return Hnd_TermMenu(pb, ref);
	return FALSE;
}


static BOOL Hnd_IbarClick(event_pollblock * pb, void *ref)
{
	if (pb->data.mouse.button.data.menu)
	{
		set_gamma_window_state();
		set_sound_window_state();

		/* Hack: shade the Save> option if appropriate */
		if (game_in_progress && character_generated)
			Menu_SetFlags(ibar_menu, IBAR_MENU_SAVE, 0, PDEADCHK);
		else
			Menu_SetFlags(ibar_menu, IBAR_MENU_SAVE, 0, TRUE);

		/*
		   | Hack: set up the Term menu as if it was opened over the main
		   | window (so that the Windows> submenu is set correctly)
		 */
		menu_term = (term_data *)&data[0];
		set_up_term_menu(menu_term);

		Menu_Show(ibar_menu, pb->data.mouse.pos.x, -1);
		return TRUE;
	}

	if (pb->data.mouse.button.data.select)
	{
		show_windows();
		grab_caret();
		return TRUE;
	}

	if (pb->data.mouse.button.data.adjust)
	{
		enter_fullscreen_mode();
		return TRUE;
	}

	return FALSE;
}



/*
 * Handler for NULL events (should this check the alarm in the desktop?
 */
static BOOL Hnd_null(event_pollblock *event, void *ref)
{
	/* Really no need to check the alarm more than once per second. */
	if (alarm_type && Time_Monotonic() > alarm_lastcheck + 100)
	{
		check_alarm();
	}

	return TRUE;
}




/*
 | Handler for PreQuit messages (eg. at shutdown).
 */
static BOOL Hnd_PreQuit(event_pollblock * b, void *ref)
{
	BOOL shutdown = (b->data.message.data.words[0] & 1) == 0;
	task_handle originator = b->data.message.header.sender;
	unsigned int quitref;
	message_block mb;
	char buffer1[64];
	os_error e;
	int ok;

	if (!(game_in_progress && character_generated))
		return TRUE;			/* ignore, we're OK to die */

	/* Stop the shutdown/quit */
	memcpy(&mb, &(b->data.message), 24);
	quitref = mb.header.yourref;
	mb.header.yourref = mb.header.myref;
	Wimp_SendMessage(event_ACK, &mb, originator, 0);

	/*
	   | We handle this differently depending on the version of the Wimp;
	   | newer versions give us much more flexibility.
	 */
	if (event_wimpversion < 350)
	{
		/*
		   | Older versions - use 'OK' and 'Cancel'.
		   | There is no "Save & Quit" button.
		 */
		Msgs_Lookup("err.shuttitl:Query from %s", e.errmess, 64);
		sprintf(buffer1, e.errmess, VARIANT);
		Msgs_Lookup("err.shutdown:Unsaved game; are you sure you want to quit?",
					e.errmess, 260);
		e.errnum = 0;
		SWI(3, 2, SWI_Wimp_ReportError, &e, 3 | 16, buffer1, NULL, &ok);

		if (ok != 1)
			return TRUE;		/* no! Pleeeeeease don't kill leeeeddle ol' me! */
	}
	else
	{
		/*
		   | Newer version: can add buttons to the dialog.
		   | we add a 'Save and Quit' button to allow the shutdown to
		   | continue /after/ saving.
		 */
		int flags;
		char buttons[64];

		Msgs_Lookup("err.shutbuts:Save & quit,Don't quit,Quit anyway",
					buttons, 64);
		Msgs_Lookup("err.shuttitl:Query from %s", e.errmess, 64);
		sprintf(buffer1, e.errmess, VARIANT);
		Msgs_Lookup("err.shutdown:Unsaved game; are you sure you want to quit?",
					e.errmess, 260);
		e.errnum = 0;

		flags = 0 | 16 | 256 | (4 << 9);

		SWI(6, 2, SWI_Wimp_ReportError, &e, flags, buffer1, ICONNAME, 0,
			buttons, NULL, &ok);

		if (ok == 4)
			return TRUE;		/* no! Pleeeeeease don't kill leeeeddle ol' me! */

		if (ok == 3)
		{
			if (inkey_flag)
			{
				save_player(SAVE_PLAYER_PARAM);		/* Save & Quit */
			}
			else
			{
				Msgs_Report(0, "err.nosave");
				return FALSE;
			}
		}
	}


	/* RO2 doesn't use the shudown flag */
	if (shutdown && event_wimpversion >= 300 && mb.header.size >= 24)
	{
		key_block kb;
		kb.code = 0x1fc;		/* restart shutdown sequence */
		Wimp_SendMessage(event_KEY, (message_block *) & kb, originator, 0);
	}

	/* "Time... to die." */
	Event_CloseDown();
	exit(0);
	return TRUE;				/* The one great certainty (sic) */
}

#endif /* FULLSCREEN_ONLY */




static void initialise_terms(void)
{
	char t[80];
	int i;

#ifndef FULLSCREEN_ONLY
	if (!minimise_memory)
	{
		/* Create a window for each term.  Term 0 is special (no scroll bars) */
		data[0].w = Window_Create("angband", template_TITLEMIN);
		data[0].font = SYSTEM_FONT;
		data[0].def_open = 1;
		data[0].unopened = 1;
		sprintf(t, "%s %s", VARIANT, VERSION);
		Window_SetTitle(data[0].w, t);
		strncpy(data[0].name, VARIANT, 12);
		Event_Claim(event_KEY, data[0].w, event_ANY, Hnd_Keypress,
					(void *)&(data[0]));
		Event_Claim(event_REDRAW, data[0].w, event_ANY, Hnd_Redraw,
					(void *)&(data[0]));
		Event_Claim(event_CLICK, data[0].w, event_ANY, Hnd_TermClick,
					(void *)&(data[0]));
		Event_Claim(event_CLOSE, data[0].w, event_ANY, Hnd_MainClose, NULL);

		for (i = 1; i < MAX_TERM_DATA; i++)
		{
			data[i].w = Window_Create("term", template_TITLEMIN);
			data[i].font = SYSTEM_FONT;
			data[i].def_open = 0;
			data[i].unopened = 1;
#ifndef OLD_TERM_MENU
			sprintf(t, "%s (%s %s)", TERM_NAME(i), VARIANT, VERSION);
#else
			sprintf(t, "Term-%d (%s %s)", i, VARIANT, VERSION);
#endif
			Window_SetTitle(data[i].w, t);
			strncpy(data[i].name, t, 12);
			Event_Claim(event_CLICK, data[i].w, event_ANY, Hnd_TermClick,
						(void *)&(data[i]));
			Event_Claim(event_REDRAW, data[i].w, event_ANY, Hnd_Redraw,
						(void *)&(data[i]));
		}
	}
#endif /* FULLSCREEN_ONLY */

	term_data_link(&(data[0]), 256);

	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		term_data_link(&(data[i]), 16);
		TERM(i) = &(data[i].t);
	}

	TERM(0) = &(data[0].t);
	Term_activate(&(data[0].t));
}




static unsigned int htoi(char *s)
{
	static const char hex[] = "0123456789ABCDEF";
	unsigned int v = 0;
	while (*s)
	{
		char *m;
		int d = toupper((unsigned char)*s++);
		m = strchr(hex, d);
		if (!m)
		{
			return v;
		}
		v = (v << 4) + (m - hex);
	}
	return v;
}

static int read_unsigned(char *t)
{
	int r;
	if (SWI(2, 3, SWI_OS_ReadUnsigned, 2, t, NULL, NULL, &r))
		r = 0;
	return r;
}

/*
 | Scan the string at 'n', replacing dodgy characters with underbars
 */
static void sanitise_name(char *n)
{
	for (; *n; n++)
	{
		if (strchr("\"$%^&*\\\'@#.,", *n))
			*n = '_';
	}
}


/*
 | Ensure that the path to a given object exists.
 | Ie. if |p| = "a.b.c.d" then we attempt to
 | create directories a, a.b and a.b.c if they don't
 | already exist.
 | Note that 'd' may be absent.
 */
static int ensure_path(char *p)
{
	char tmp[260];
	char *l = tmp;

	while (*p)
	{
		if (*p == '.')
		{
			*l = 0;
			if (SWI(5, 0, SWI_OS_File, 8, tmp, 0, 0, 77))
				return 0;		/* Eeek! */
		}
		*l++ = *p++;
	}

	return 1;
}


/*
 * Set up the Scrap, Choices and Alarm paths, trying for
 * Choices:blah...,etc.  by preference, but falling back on lib/xtra
 * if need be.
 */
static void init_paths(void)
{
	char tmp[512];
	char subpath[128];
	char *v;
	char *t;

	/* Form the sub-path we use for both Choices and Scrap dirs: */
	v = subpath + sprintf(subpath, "%s", VARIANT);
	sanitise_name(subpath);
	sprintf(v, ".%s", VERSION);
	sanitise_name(v + 1);

	/* Do the Scrap path first: */
	*scrap_path = 0;

	/* Try for Wimp$ScrapDir... */
	t = getenv("Wimp$ScrapDir");
	if (t && *t)
	{
		sprintf(tmp, "%s.AngbandEtc.%s.", t, subpath);
		if (ensure_path(tmp))
		{
			strcpy(scrap_path, tmp);
		}
	}

	/* Couldn't use Wimp$ScrapDir, so fall back on lib.xtra.scrap */
	if (!*scrap_path)
	{
		sprintf(tmp, "%sxtra.scrap.", resource_path);
		if (ensure_path(tmp))
		{
			strcpy(scrap_path, tmp);
		}
	}

	/* Now set up the Choices and Alarm files: */

	/* Read only Choices file is always lib.xtra.Choices */
	sprintf(choices_file[CHFILE_READ], "%sXtra.Choices", resource_path);
	/* Default writable Choices file is the same */
	strcpy(choices_file[CHFILE_WRITE], choices_file[CHFILE_READ]);
	/* No default mirror Choices file */
	strcpy(choices_file[CHFILE_MIRROR], "");

	/* Read only Alarm file is always  lib.xtra.Alarm */
	sprintf(alarm_file[CHFILE_READ], "%sXtra.Alarm", resource_path);
	/* Default writable Alarm file is the same */
	strcpy(alarm_file[CHFILE_WRITE], alarm_file[CHFILE_READ]);

	/* Try to use Choices$Path, etc. for the others... */

	t = getenv("Choices$Write");	/* Ie. where choices should be written */
	if (t && *t)
	{
		/* Choices file: */
		sprintf(tmp, "%s.AngbandEtc.%s", t, subpath);
		if (ensure_path(tmp))
		{
			/* Use for writable file: */
			strcpy(choices_file[CHFILE_WRITE], tmp);
			/* Form 'mirror' filename: same path but with a fixed leafname */
			strcpy(v + 1, "Default");
			sprintf(tmp, "%s.AngbandEtc.%s", t, subpath);
			strcpy(choices_file[CHFILE_MIRROR], tmp);
		}

		/* Alarm file (doesn't involve subpath) */
		sprintf(tmp, "%s.AngbandEtc.Global.Alarm", t);
		if (ensure_path(tmp))
		{
			/* Use for read/writable file */
			strcpy(alarm_file[CHFILE_WRITE], tmp);
		}
	}
}







/*
 * Return the appropriate (full) pathname.
 *
 * For write ops, the read/write file is returned.
 *
 * For read ops, either the read/write file, the mirror file,
 * or the read only file will be returned as appropriate.
 */
static char *find_choices(int write)
{
	if (write)
		return choices_file[CHFILE_WRITE];

	if (File_Size(choices_file[CHFILE_WRITE]) > 0)
		return choices_file[CHFILE_WRITE];

	if (File_Size(choices_file[CHFILE_MIRROR]) > 0)
		return choices_file[CHFILE_MIRROR];

	return choices_file[CHFILE_READ];
}

static char *find_choices_mirror(void)
{
	return choices_file[CHFILE_MIRROR];
}

static char *find_alarmfile(int write)
{
	if (write)
		return alarm_file[CHFILE_WRITE];

	if (File_Size(alarm_file[CHFILE_WRITE]) > 0)
		return alarm_file[CHFILE_WRITE];

	return alarm_file[CHFILE_READ];
}


#ifdef HASNOCORE
/*
 * Redefinable "core" action
 */
void (*core_aux)(cptr) = NULL;

/*
 * Dump a core file, after printing a warning message
 * As with "quit()", try to use the "core_aux()" hook first.
 */
void core(cptr str)
{
	char *crash = NULL;

	/* Use the aux function */
	if (core_aux) (*core_aux)(str);

	/* Dump the warning string */
	if (str) plog(str);

	/* Attempt to Crash */
	(*crash) = (*crash);

	/* Be sure we exited */
	quit("core() failed");
}
#endif





int main(int argc, char *argv[])
{
	int i, j;
	int start_full = 0;
	char *arg_savefile = 0;
	char *t;
#ifdef USE_DA
	int da_font = 1, da_game = 1;
#endif

	atexit(final_acn);		/* "I never did care about the little things." */

	START_HOURGLASS;

	/* Parse arguments */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (tolower((unsigned char)argv[i][1]))
			{
				case 'm':
				{
					/* Minimise Memory */
					minimise_memory = 1;

					/* Break */
					break;
				}
				case 'c':		/* -c[a][s][f][<n>] */
					for (j = 2; argv[i][j]; j++)
					{
						int on = isupper((unsigned char)argv[i][j]);

						switch (tolower((unsigned char)argv[i][j]))
						{
#ifdef ABBR_FILECACHE
							case 'a': abbr_filecache =
									on;
								break;
							case 'f': abbr_tmpfile =
									on;
								break;
#endif
#ifdef SMART_FILECACHE
							case 's': smart_filecache =
									on;
								break;
#endif

							case 'p': flush_scrap =
									!on;
								break;

							default:
								if (isdigit((unsigned char)argv[i][j]))
								{
									max_file_cache_size =
										atoi(argv[i] + j) << 10;
									while (isdigit((unsigned char)argv[i][++j]))
										;
									if (max_file_cache_size <= 0)
									{
										use_filecache = 0;
									}
									j--;
								}
								else
								{
									fprintf(stderr, "Unrecognised option: -c%s",
											argv[i] + j);
									exit(EXIT_FAILURE);
								}
						}
					}
					break;
				case 'w':		/* -waitrelease */
					hack_flush = 1;
					break;
				case 's':		/* -s<savefile> */
					if (argv[i][2])
						arg_savefile = argv[i] + 2;
					break;
				case 'f':		/* -fullscreen */
					start_full = 1;
					break;
				case 'h':		/* -hourglass */
					use_glass = 1;
					break;
				case 't':		/* -T<filetype> */
					if (argv[i][2])
						vfiletype = htoi(argv[i] + 2);
					break;
#ifdef USE_DA
				case 'd':		/* -df, -dg, -dc or -d : disable DAs */
					switch (tolower((unsigned char)argv[i][2]))
					{
						case 0:	/* -d => disable both */
							da_font = da_game = 0;
							break;
						case 'f':	/* -df => disable font only */
							da_font = 0;
							break;
						case 'g':	/* -dg => disable game only */
							da_game = 0;
							break;
					}
					break;
#endif
				case '%':		/* -%<debug_opts> */
				{
					int v = read_unsigned(argv[i] + 2);
					log_g_malloc = v & 1;
					show_sound_alloc = v & 2;
				}
					break;
				default:
					fprintf(stderr, "Unrecognised option: %s", argv[i]);
					exit(EXIT_FAILURE);
			}
		}
	}

	/* 1.27 - new handling of -minimise-memory: */
#ifndef FULLSCREEN_ONLY
	if (minimise_memory)
#endif /* FULLSCREEN_ONLY */
	{
		start_full = 1;
		fs_quit_key_text = "(fullscreen only mode)";
	}
#ifdef USE_DA
	init_memory(da_font, da_game);	/* Set up dynamic areas, etc. if possible */

	/* Install memory allocation hooks */
	ralloc_aux = g_malloc;
	rnfree_aux = g_free;
#endif

	/* Install replacement error reporting routines */
	quit_aux = quit_hook;
	plog_aux = plog_hook;
	core_aux = core_hook;

#ifdef IS_SCTH
	/* Hook in the file modification hook in scth*/
    check_modification_date_hook = check_modification_date;
#endif

	/* Expand the (Angband) resource path */
	t = getenv(RISCOS_VARIANT "$Path");
	if (!t || !*t) Msgs_ReportFatal(0, "A resources path could not be formed.");
	strcpy(resource_path, t);

	/* Decide where scrap, choices and alarm files live: */
	init_paths();

	/* Hack: if no savefile specified, use a default */
	if (!arg_savefile)
	{
		arg_savefile = malloc(strlen(resource_path) + 32);
		if (!arg_savefile)
		{
			Msgs_ReportFatal(0, "err.mem");
		}
		sprintf(arg_savefile, "%s%s", resource_path, ">Save.Savefile");
	}

	/* This crap appears here so that plog() will work properly before
	   init_acn() is called... */
	Resource_Initialise(RISCOS_VARIANT);
	Msgs_LoadFile("Messages");

#ifndef FULLSCREEN_ONLY
	if (!minimise_memory)
	{
		Event_Initialise(RISCOS_VARIANT);

		/* Load Templates */
		Template_Initialise();
		Template_LoadFile("Templates");
	}
#endif /* FULLSCREEN_ONLY */

	Screen_CacheModeInfo();

	/* Initialise some ZapRedraw stuff */
	initialise_palette();
	initialise_fonts();			/* Set up the fonts */
#ifndef FULLSCREEN_ONLY

	if (!minimise_memory)
	{
		/* Initialise some Wimp specific stuff */
		init_gamma_window();
		init_sound_window();
		init_save_window();
		init_menus();

		ibar_icon = Icon_BarIcon(ICONNAME, iconbar_RIGHT);

		/* Global handlers */
		Event_Claim(event_OPEN, event_ANY, event_ANY, Handler_OpenWindow, NULL);
		Event_Claim(event_CLOSE, event_ANY, event_ANY, Handler_CloseWindow, NULL);
		Event_Claim(event_GAINCARET, event_ANY, event_ANY, Hnd_Caret, (void *)1);
		Event_Claim(event_LOSECARET, event_ANY, event_ANY, Hnd_Caret, (void *)0);
		Event_Claim(event_MENU, event_ANY, event_ANY, Hnd_MenuSel, NULL);
		Event_Claim(event_CLICK, window_ICONBAR, ibar_icon, Hnd_IbarClick, NULL);
		Event_Claim(event_CLICK, event_ANY, event_ANY, Hnd_Click, NULL);
		EventMsg_Claim(message_PALETTECHANGE, event_ANY, Hnd_PaletteChange, NULL);
		EventMsg_Claim(message_MODECHANGE, event_ANY, Hnd_ModeChange, NULL);
		EventMsg_Claim(message_PREQUIT, event_ANY, Hnd_PreQuit, NULL);

		/* Initialise the sound stuff */
		initialise_sound();
	}
#endif /* FULLSCREEN_ONLY */

	/* Initialise some Angband stuff */
	initialise_terms();

	/* Set up the r_data buffer */
	initialise_r_data();

	load_choices();
	read_alarm_choices();

	init_file_paths(unixify_name(resource_path));

	START_HOURGLASS;			/* Paranoia */

	/* Hack - override the saved options if -F was on the command line */
	start_fullscreen |= start_full;

	/* hack so that the cursor is yellow if undefined */
	if (palette[CURSOR_COLOUR] == palette[0])
	{
		angband_color_table[CURSOR_COLOUR][1] = (CURSOR_RGB & 0xff00) >> 8;
		angband_color_table[CURSOR_COLOUR][2] = (CURSOR_RGB & 0xff0000) >> 16;
		angband_color_table[CURSOR_COLOUR][3] = (CURSOR_RGB & 0xff000000) >> 24;
	}

	/* Catch nasty signals */
	signals_init();

	/* use pref-acn.prf */
	ANGBAND_SYS = "acn";

#ifndef FULLSCREEN_ONLY
	if (start_fullscreen)
	{
#endif /* FULLSCREEN_ONLY */
		Event_Claim(event_NULL, event_ANY, event_ANY, Hnd_null, NULL);
		enter_fullscreen_mode();   
#ifndef FULLSCREEN_ONLY
	}
	else
	{
		START_HOURGLASS;		/* Paranoia */
		Hnd_ModeChange(NULL, NULL);	/* Caches the various fonts/palettes */
		show_windows();
		grab_caret();

		Event_Claim(event_NULL, event_ANY, event_ANY, Hnd_null, NULL);

		/* Wait for a null poll so that the windows can appear */
		do
		{
			Event_Poll();
		}
		while (event_lastevent.type != event_NULL);
	}
#endif /* FULLSCREEN_ONLY */

	/* Initialise Angband */
	START_HOURGLASS;			/* Paranoia */

	strncpy(savefile, unixify_name(arg_savefile), sizeof(savefile));
	savefile[sizeof(savefile) - 1] = '\0';

	use_sound = 1;
	init_angband();
	initialised = 1;
	game_in_progress = 1;
	pause_line(PAUSE_LINE_PARAM);
	flush();

	play_game(FALSE);

	if (fullscreen_mode) leave_fullscreen_mode();

	STOP_HOURGLASS;

	quit(NULL);

	return 0;

	debug("to stop the 'unused' warning :)");
}













/*
 | We use this to keep the mouse in the same place on return from full-screen
 | mode.
 */
static wimp_point old_mouse_posn;

/* Nasty hack to remember how big the main window is when not in fullscreen */
static int main_wid, main_hgt;

/*
 | Take a copy of the current mode descriptor/number and return either
 | a pointer to it (as an int) if it's a new mode, or the mode number.
 | NB: A static pointer is used and the descriptor returned is only
 | valid until the next call to this function.
 |
 | Basically, a replacement for OS_Byte 135 / OS_ScreenMode1, IYSWIM.
 */
static int current_mode(void)
{
	static void *descriptor = NULL;
	int mode;
	int size;
	int i;
	int *vals;

	if (descriptor)
	{
		free(descriptor);
		descriptor = NULL;
	}

	SWI(1, 3, SWI_OS_Byte, 135, NULL, NULL, &mode);
	if (mode < 256)
	{
		return mode;
	}

	vals = (int *)(mode + 20);
	for (i = 0; vals[i] != -1; i += 2)
		;

	size = 24 + 8 * i;			/* Size of data */
	descriptor = malloc(size);
	if (!descriptor)
	{
		core("Out of memory!");
	}
	memcpy(descriptor, (void *)mode, size);

	return (int)descriptor;
}



/*
 | Select the best mode we can for full screen.
 | Returns 12 for (low-res, ie. mode 12) or 27 for high-res,
 | or a pointer to a mode descriptor (as an int).
 */
static int select_fullscreen_mode(void)
{
	static struct
	{
		int flags, x, y, l2bpp, hz, term;
	}
	desc;
	int mode = 0;

	desc.flags = 1;				/* format 0 */
	desc.x = 640;
	desc.y = 480;				/* 640x480 */
	desc.l2bpp = 2;				/* 16 colours */
	desc.hz = -1;				/* best we can get */
	desc.term = -1;	/* don't fuss about modevars */

	SWI(1, 1, SWI_OS_CheckModeValid, &desc, &mode);
	if (mode != (int)&desc)
	{
		SWI(1, 1, SWI_OS_CheckModeValid, 27, /**/ &mode);
		if (mode != 27)
		{
			SWI(1, 1, SWI_OS_CheckModeValid, 12, /**/ &mode);
			if (mode != 12)
			{
				mode = 0;
			}
		}
	}

	return mode;
}


/*
 | Change screen mode
 */
static void change_screenmode(int to)
{
	if (SWI(2, 0, SWI_OS_ScreenMode, 0, to))
	{
		if (to < 256)
		{
			GFX_VDU(22);
			GFX_VDU(to);
		}
		else
		{
			/* Finished with my woman / cos she couldn't help me with ... */
			core("Eeek! mode isn't valid, but it /should/ be...");
		}
	}
}


/*
 | Constrain the mouse pointer to a point - this means that the damn
 | hourglass won't move around with the mouse :)
 */
static void constrain_pointer(void)
{
	mouse_block ptr;
	wimp_rect r;
	int ys = screen_eig.y == 1 ? 32 : 64;	/* Cope with dbl height glass */

	Screen_CacheModeInfo();		/* Make sure we know the screen size */
	r.min.x = r.max.x = screen_size.x - 32;
	r.min.y = r.max.y = screen_size.y - ys;

	/* Retrieve and store old (wimp) pointer position */
	Wimp_GetPointerInfo(&ptr);
	old_mouse_posn = ptr.pos;

	Pointer_RestrictToRect(r);

	/* Turn the pointer off also */
	SWI(2, 0, SWI_OS_Byte, 106, 0);
}

static void release_pointer(void)
{
	wimp_rect r;

	r.min.x = r.max.x = old_mouse_posn.x;
	r.min.y = r.max.y = old_mouse_posn.y;

	Pointer_RestrictToRect(r);

	Pointer_Unrestrict();

	/* Turn the pointer back on also */
	SWI(2, 0, SWI_OS_Byte, 106, 1);
}


/*
 | Convert a 1bpp bitmap into a 4bpp bitmap (bit flipped)
 */
static int byte_to_word_flipped(int b)
{
	int w;
	if (b & 128)
	{
		w = 0xf0000000;
	}
	else
	{
		w = 0;
	}
	if (b & 64)
	{
		w |= 0x0f000000;
	}
	if (b & 32)
	{
		w |= 0x00f00000;
	}
	if (b & 16)
	{
		w |= 0x000f0000;
	}
	if (b & 8)
	{
		w |= 0x0000f000;
	}
	if (b & 4)
	{
		w |= 0x00000f00;
	}
	if (b & 2)
	{
		w |= 0x000000f0;
	}
	if (b & 1)
	{
		w |= 0x0000000f;
	}
	return w;
}




/*
 | try to load the fallback fullscreen font and convert it to 4bpp
 */
static int cache_zapfontHR(void)
{
	int handle;
	unsigned int extent;
	char buffer[260];
	struct
	{
		char id[8];
		int w, h, f, l, r1, r2;
	}
	zfh;
	int *op;
	char *ip;
	int l, i;

	/* Try to open the file */
	sprintf(buffer, "%s%s", resource_path, "xtra.FullScreen");
	handle = File_Open(buffer, (file_access) 0x4f);
	if (!handle)
	{
		return 0;
	}

	/* Check file's extent */
	extent = File_ReadExtent(handle);
	if (extent > sizeof(zfh) + 256 * 16)
	{
		File_Close(handle);
		return 0;
	}

	/* Load the header */
	if (myFile_ReadBytes(handle, &zfh, sizeof(zfh)))
	{
		File_Close(handle);
		return 0;
	}

	/* Check font size */
	if ((zfh.w != 8) || (zfh.h > 16))
	{
		File_Close(handle);
		return 0;
	}

	/* Load the 1bpp data */
	if (myFile_ReadBytes(handle, fullscreen_font, extent - sizeof(zfh)))
	{
		File_Close(handle);
		return 0;
	}

	File_Close(handle);

	l = zfh.l > 255 ? 255 : zfh.l;

	if (zfh.h > 8)
	{
		op = (int *)(((int)fullscreen_font) + (l + 1) * zfh.h * 4);
		ip = (char *)(((int)fullscreen_font) + (l + 1) * zfh.h -
					  (zfh.f * zfh.h));
		while (l-- >= zfh.f)
		{
			for (i = 0; i < zfh.h; i++)
			{
				*--op = byte_to_word_flipped(*--ip);
			}
		}
		fullscreen_height = zfh.h;
	}
	else
	{
		op = (int *)(((int)fullscreen_font) + (l + 1) * zfh.h * 8);
		ip = (char *)(((int)fullscreen_font) + (l + 1) * zfh.h -
					  (zfh.f * zfh.h));
		while (l-- >= zfh.f)
		{
			for (i = -zfh.h; i < zfh.h; i++)
			{
				int t = byte_to_word_flipped(*--ip);
				*--op = t;
				*--op = t;
			}
		}
		fullscreen_height = zfh.h * 2;
	}

	fullscreen_topline = TERM_TOPLINE_HR;
	fullscreen_topline += ((16 - fullscreen_height) * 13);

	return 1;
}





static int cache_fs_fontHR(void)
{
	ZapFont *src = SYSTEM_FONT;
	int c;
	int *op;
	char *ip;

	/* Allocate the storage for the font */
	fullscreen_font = f_malloc(256 * 4 * 16);
	if (!fullscreen_font)
	{
		return 0;
	}
	op = (int *)fullscreen_font;

	/* Check to see if the main term's font is suitable (ie. 8x16 or 8x8) */
	if ((data[0].font->w == 8) && (data[0].font->h <= 16))
		src = data[0].font;

	/*
	   | Hack: if we're forced to use the system font, try to load the
	   | 'fullscreen' font from lib.xtra.  If that fails, then I guess we're
	   | stuck with the system font.
	 */

	if (src == SYSTEM_FONT)
		if (cache_zapfontHR())
		{
			return 1;
		}

	ip = (char *)(src->bpp_1);

	/* Now, create the font */
	if (src->h > 8)
	{
		int e = src->h * (src->l > 256 ? 256 : src->l);
		op += (src->f * src->h);
		for (c = src->f * src->h; c < e; c++)
			*op++ = byte_to_word_flipped(*ip++);
		fullscreen_height = src->h;
	}
	else
	{
		int e = src->h * (src->l > 256 ? 256 : src->l);
		op += (src->f * src->h) * 2;
		for (c = src->f * src->h; c < e; c++)
		{
			int t = byte_to_word_flipped(*ip++);
			*op++ = t;
			*op++ = t;
		}
		fullscreen_height = src->h * 2;
	}

	fullscreen_topline = TERM_TOPLINE_HR;
	fullscreen_topline += ((16 - fullscreen_height) * 13);

	return 1;
}





static int cache_fs_fontLR(void)
{
	ZapFont *src = SYSTEM_FONT;
	int c, e;
	int *op;
	char *ip;

	/* Allocate the storage for the font */
	fullscreen_font = f_malloc(256 * 4 * 8);
	if (!fullscreen_font)
	{
		return 0;
	}
	op = (int *)fullscreen_font;

	/* Check to see if the main term's font is suitable (ie. 8x8) */
	if ((data[0].font->w == 8) && (data[0].font->h <= 8))
		src = data[0].font;

	ip = (char *)(src->bpp_1);

	/* Now, create the font */
	e = src->h * (src->l > 256 ? 256 : src->l);
	op += (src->f * src->h);
	for (c = src->f * src->h; c < e; c++)
		*op++ = byte_to_word_flipped(*ip++);

	fullscreen_height = src->h;
	fullscreen_topline = TERM_TOPLINE_LR;
	fullscreen_topline += ((8 - fullscreen_height) * 13);

	return 1;
}



static void set_keys(int claim)
{
	static int old_c_state;
	static int old_f_state[8];
	int i;

	if (claim)
	{
		/* Cursors/copy act as function keys */
		/* f0-f9, cursors, generate 0x80-0x8f */
		/* sh-f0-f9,cursors, generate 0x90-0x9f */
		/* ctrl f0-f9,cursors, generate 0xa0-0xaf */
		/* sh-c-f0-f9,cursors, generate 0xb0-0xbf */
		/* f10-f12 generate 0xca-0xcc */
		/* shift f10-f12 generate 0xda-0xdc */
		/* ctrl f10-f12 generate 0xea-0xec */
		/* ctrlshift f10-f12 generate 0xfa-0xfc */

		SWI(3, 2, SWI_OS_Byte, 4, 2, 0, /**/ NULL, &old_c_state);

		for (i = 0; i < 4; i++)
		{
			SWI(3, 2, SWI_OS_Byte, 225 + i, 0x80 + (i * 0x10), 0, NULL,
				old_f_state + i);
			SWI(3, 2, SWI_OS_Byte, 221 + i, 0xc0 + (i * 0x10), 0, NULL,
				old_f_state + i + 4);
		}
	}
	else
	{
		SWI(3, 0, SWI_OS_Byte, 4, old_c_state, 0);
		for (i = 0; i < 4; i++)
		{
			SWI(3, 0, SWI_OS_Byte, 225 + i, old_f_state[i], 0);
			SWI(3, 0, SWI_OS_Byte, 221 + i, old_f_state[i + 4], 0);
		}
	}
}






/*
 | Enter the full screen mode.
 |
 | Full screen display uses either mode 27 (if supported) and 8x16 fonts
 | (or system font 'twiddled' to double height), or mode 12 (if mode 27
 | is unavailable) and the system font (or an 8x8 font).
 |
 */

static void enter_fullscreen_mode(void)
{
	int vduvars[2] =
	{ 149, -1 };
	int i;

	/* New in 1.18 - protect against 're-entracy' */
	if (fullscreen_font)
		return;

	/* Choose the mode we want */
	fullscreen_mode = select_fullscreen_mode();

	if (!fullscreen_mode)
	{
		plog("Unable to select a suitable screen mode (27 or 12)");
		return;
	}

	if (!((fullscreen_mode == 12) ? cache_fs_fontLR() : cache_fs_fontHR()))
	{
		plog("Unable to cache a font for full screen mode");
		return;
	}

	/* Read the current screen mode */
	/* SWI( 1,3, SWI_OS_Byte, 135, NULL, NULL, &old_screenmode ); */
	old_screenmode = current_mode();

	STOP_HOURGLASS;

	/* Change to the chosen screen mode */
	change_screenmode(fullscreen_mode);

	/* Restrict the pointer */
	constrain_pointer();

	/* Remove the cursors */
	SWI(0, 0, SWI_OS_RemoveCursors);

	START_HOURGLASS;

	/* Get the base address of screen memory */
	SWI(2, 0, SWI_OS_ReadVduVariables, vduvars, vduvars);
	fullscreen_base = (int *)(vduvars[0]);

	/* Fudge the Term interface */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term *t = &(data[i].t);
		t->wipe_hook = Term_wipe_acnFS;
		t->curs_hook = Term_curs_acnFS;
		t->text_hook = Term_text_acnFS;
	}

	/* Grab the palette */
	Term_xtra_acn_reactFS(TRUE);

	/* Make sure that the keys work properly */
	set_keys(TRUE);

	main_wid = data[0].t.wid;
	main_hgt = data[0].t.hgt;
	term_change_size(&data[0], 80, 24);

	/* refresh the term */
	/*Term_activate( &(data[0].t) ); */
	redraw_areaFS(0, 0, 80, 24);
	if (data[0].cursor.visible)
		draw_cursor(data[0].cursor.pos.x, data[0].cursor.pos.y);

	/* Display a reminder of how to get back... */
	/* Hack: disable force_mono */
	i = force_mono;
	force_mono = 0;
	Term_text_acnFS(0, TIME_LINE, strlen(fs_quit_key_text), 8,
					fs_quit_key_text);
	force_mono = i;
}




static void leave_fullscreen_mode(void)
{
	int i;

	/* New in 1.18 - protect against 're-entracy' */
	if (!fullscreen_font) return;

	/* Restore the screen mode */
	Wimp_SetMode(old_screenmode);

	/* Restore the Term interface */
	term_change_size(&data[0], main_wid, main_hgt);

	for (i = 0; i < MAX_TERM_DATA; i++)
	{
#ifndef FULLSCREEN_ONLY
		term *t = &(data[i].t);
		t->wipe_hook = Term_wipe_acn;
		t->curs_hook = Term_curs_acn;
		t->text_hook = Term_text_acn;
#endif
		mark_ood(&(data[i]), 0, 0, data[i].t.wid, data[i].t.hgt);
	}

	/* Deallocate the font */
	f_free(fullscreen_font);
	fullscreen_font = 0;
	fullscreen_mode = 0;

	STOP_HOURGLASS;

	/* Restore the pointer */
	release_pointer();

	START_HOURGLASS;

	/* Restore the various soft keys */
	set_keys(FALSE);

#ifndef FULLSCREEN_ONLY
	/* Refresh the windows - this probably isn't necessary anyway */
	if (!minimise_memory) refresh_windows();
#endif /* FULLSCREEN_ONLY */
}





static void fs_writechars(int x, int y, int n, const char *chars, char attr)
{
	int *scr, *scrb;
	int *cdat;
	int j;
	unsigned int fgm;

	if (force_mono)
	{
		if (attr != TERM_DARK)
		{
			attr = TERM_WHITE;
		}
	}
	fgm = (unsigned int)zpalette[(unsigned int) attr];

	scrb = (int *)(((int)fullscreen_base) + y * fullscreen_height * 320
				   + x * 4 + 320 * fullscreen_topline);

	while (n--)
	{
		scr = scrb++;
		cdat = (int *)(((int)fullscreen_font)
					   + (*chars++) * (fullscreen_height << 2));
		for (j = 0; j < fullscreen_height; j++)
		{
			*scr = *cdat++ & fgm;
			scr += 80;
		}
	}
}


static void fs_writechar(int x, int y, char c, char attr)
{
	int *scrb;
	int *cdat;
	int j;
	unsigned int fgm;

	if (force_mono)
	{
		if (attr != TERM_DARK)
		{
			attr = TERM_WHITE;
		}
	}
	fgm = (unsigned int)zpalette[(unsigned int) attr];

	scrb = (int *)(((int)fullscreen_base) + y * fullscreen_height * 320
				   + x * 4 + 320 * fullscreen_topline);
	cdat = (int *)(((int)fullscreen_font) + (c * (fullscreen_height << 2)));
	for (j = 0; j < fullscreen_height; j++)
	{
		*scrb = *cdat++ & fgm;
		scrb += 80;
	}
}



static void draw_cursorHR(int x, int y)
{
	ColourTrans_SetGCOL(cursor_rgb, 0, 0);
	GFX_Move(x * 16,
			 959 - y * (fullscreen_height * 2) - fullscreen_topline * 2);
	GFX_DrawBy(14, 0);
	GFX_DrawBy(0, -(fullscreen_height * 2 - 2));
	GFX_DrawBy(-14, 0);
	GFX_DrawBy(0, fullscreen_height * 2 - 2);
}

static void draw_cursorLR(int x, int y)
{
	ColourTrans_SetGCOL(cursor_rgb, 0, 0);
	GFX_Move(x * 16,
			 1023 - y * (fullscreen_height * 4) - fullscreen_topline * 4);
	GFX_DrawBy(14, 0);
	GFX_DrawBy(0, -(fullscreen_height * 4 - 4));
	GFX_DrawBy(-14, 0);
	GFX_DrawBy(0, fullscreen_height * 4 - 4);
}





static void draw_cursor(int x, int y)
{
	if (fullscreen_mode == 12)
		draw_cursorLR(x, y);
	else
		draw_cursorHR(x, y);
}



static void redraw_areaFS(int x, int y, int w, int h)
{
	int i, j;
	for (j = y; j < y + h; j++)
		for (i = x; i < x + w; i++)
			fs_writechar(i, j, data[0].t.old->c[j][i], data[0].t.old->a[j][i]);
}



static int wimp_code(int c)
{
	/* shift/ctrl keypad? */
	if (c >= '0' && c <= '9')
	{
		kbd_modifiers m = Kbd_GetModifiers(FALSE);
		if (m.shift)
		{
			c |= 0x800;
		}
		if (m.ctrl)
		{
			c |= 0x400;
		}
		return c;
	}
	if (c == 9)
	{
		return 0x18a;
	}							/* Tab */
	if (c <= 127)
	{
		return c;
	}							/* normal ASCII/ctrl */
	if (c >= 0x80 && c <= 0xff)
	{
		return c + 0x100;
	}							/* f0-f9, etc. */

	return -1;					/* unknown */
}




static void do_keypress(int code)
{
	static const char hex[] = "0123456789ABCDEF";

	if (code == KEYPRESS_QUIT && !minimise_memory)
	{
#ifdef FULLSCREEN_ONLY
		Sound_SysBeep();
#else
		leave_fullscreen_mode();
#endif
		return;
	}

	if (code == 27)
	{
		if (Kbd_KeyDown(inkey_CTRL))
		{
			ack_alarm();
			return;
		}
	}

	if (code <= 255)
	{
		Term_keypress(code);
	}
	else
	{
		Term_keypress(31);
		Term_keypress(hex[(code & 0xf00) >> 8]);
		Term_keypress(hex[(code & 0x0f0) >> 4]);
		Term_keypress(hex[(code & 0x00f)]);
		Term_keypress(13);
	}
}




static errr Term_xtra_acn_eventFS(int valid)
{
	int bh, bl = -1;
	int c;
	int w = -1;

	STOP_HOURGLASS;

	/* Loop if we want validation of the keypress */
	do
	{
		bored();

		/* Check if there are keypresses in the buffer */
		SWI(3, 3, SWI_OS_Byte, 128, 255, 0, NULL, &bl, &bh);
		bl = (bl & 0xff) | (bh << 8);

		if (bl)
		{
			/* Read a keypress */
			SWI(0, 1, SWI_OS_ReadC, &c);

			/* If valid, process it */
			w = wimp_code(c);
			if (w >= 0) do_keypress(w);
		}
	} while (valid && w == -1);

	START_HOURGLASS;

	return 0;
}



/*
 * React to changes
 */
static errr Term_xtra_acn_reactFS(int force)
{
	unsigned int i;
	int p, r, g, b;

	static int old_gamma = -1;

	if (gamma != old_gamma)
	{
		force = 1;
		old_gamma = gamma;
	}

	/* Set the screen colours */
	for (i = 0; i < 16; i++)
	{
		if (COLOUR_CHANGED(i) || force)
		{
			r = (int)(255.0 *
				  pow(angband_color_table[i][1] / 255.0, 1.0 / ((double) gamma / 100)));
			g = (int)(255.0 *
				  pow(angband_color_table[i][2] / 255.0, 1.0 / ((double) gamma / 100)));
			b = (int)(255.0 *
				  pow(angband_color_table[i][3] / 255.0, 1.0 / ((double) gamma / 100)));
			GFX_VDU(19);
			GFX_VDU(i);
			GFX_VDU(16);
			GFX_VDU(r);
			GFX_VDU(g);
			GFX_VDU(b);

			palette[i] = (b << 24) | (g << 16) | (r << 8);
			p = i;
			p |= (p << 4);
			p |= (p << 8);
			p |= (p << 16);
			zpalette[i] = p;

			a_palette[i][1] = angband_color_table[i][1];
			a_palette[i][2] = angband_color_table[i][2];
			a_palette[i][3] = angband_color_table[i][3];

			/* Find any higher colour numbers and make them "wrong" */
			for (p = 16; p < 256; p++)
				if ((zpalette[p] & 0xf) == i)
					a_palette[p][1] = angband_color_table[p][1] + 2;
		}
	}


	/* Go through the palette updating any changed values */
	for (i = 16; i < 256; i++)
	{
		if (COLOUR_CHANGED(i) || force)
		{
			r = (int)(255.0 *
				  pow(angband_color_table[i][1] / 255.0, 1.0 / ((double) gamma / 100)));
			g = (int)(255.0 *
				  pow(angband_color_table[i][2] / 255.0, 1.0 / ((double) gamma / 100)));
			b = (int)(255.0 *
				  pow(angband_color_table[i][3] / 255.0, 1.0 / ((double) gamma / 100)));
			p = (b << 24) | (g << 16) | (r << 8);
			palette[i] = p;
			SWI(1, 1, SWI_ColourTrans_ReturnColourNumber, palette[i], &p);
			p |= (p << 4);
			p |= (p << 8);
			p |= (p << 16);
			zpalette[i] = p;
			a_palette[i][1] = angband_color_table[i][1];
			a_palette[i][2] = angband_color_table[i][2];
			a_palette[i][3] = angband_color_table[i][3];
		}
	}

	cursor_rgb = palette[CURSOR_COLOUR];

	return 0;
}


static errr Term_curs_acnFS(int x, int y)
{
	if (Term == &(data[0].t))
	{
		if (data[0].cursor.visible)
			redraw_areaFS(data[0].cursor.pos.x, data[0].cursor.pos.y, 1, 1);
		data[0].cursor.pos.x = x;
		data[0].cursor.pos.y = y;
		if (data[0].cursor.visible)
			draw_cursor(x, y);
	}
	return 0;
}

static errr Term_xtra_acn_clearFS(void)
{
	char e[80];
	int j;

	if (Term == &(data[0].t))
	{
		for (j = 0; j < 80; j++)
			e[j] = ' ';

		GFX_Wait();

		for (j = 0; j < 24; j++)
			fs_writechars(0, j, 80, e, 0);
	}

	return 0;
}





static errr Term_wipe_acnFS(int x, int y, int n)
{
	if (Term == &(data[0].t))
		while (n--)
			fs_writechar(x++, y, ' ', 0);
	return 0;
}

static errr Term_text_acnFS(int x, int y, int n, byte a, cptr s)
{
	if (Term == &(data[0].t))
		fs_writechars(x, y, n, s, (char)a);
	return 0;
}



static void bored()
{
	static unsigned int last;
	char ts[80];
	time_t ct;
	struct tm *lt;
	unsigned int l;
	int ofm;
	static int alarm_flash = 1;


	/* Really no need to check the alarm more than once per second. */
	if (alarm_type && Time_Monotonic() > alarm_lastcheck + 100)
	{
		check_alarm();
	}

	l = Time_Monotonic();
	if ((l - last) < (alarm_flash ? 25 : 50))
	{
		return;
	}
	last = l;

	time(&ct);
	lt = localtime(&ct);
	l = strftime(ts, 80, "%c %Z", lt);

	/* Hack: disable force_mono around printing the time */
	ofm = force_mono;
	force_mono = 0;

	/* Hack: Is the alarm supposed to be going off? */
	if (alarm_disp || alarm_flash)
	{
		char blk[60];
		int c = 8;
		if (!alarm_disp)
		{
			alarm_flash = 11;
		}
		switch (alarm_flash / 2)
		{
			case 4: sprintf(blk, "%-57s", alarm_cancel_text);
				break;
			case 5: sprintf(blk, "%-57s", fs_quit_key_text);
				break;
			default:
				c = alarm_flash & 1 ? TERM_RED : TERM_WHITE;
				sprintf(blk, "%02d:%02d %-51s", alarm_h, alarm_m,
						alarm_message);
		}
		fs_writechars(0, TIME_LINE, 57, blk, c);
		if (++alarm_flash > 11)
		{
			alarm_flash = 0;
		}
	}

	/* Display time */
	fs_writechar(79 - l, TIME_LINE, ' ', 0);
	fs_writechars(80 - l, TIME_LINE, l, ts, 8);

	force_mono = ofm;
}






#ifdef USE_DA
/*--------------------------------------------------------------------------*/
/* (Simple) Heap management (using OS_Heap)									*/
/*--------------------------------------------------------------------------*/

typedef void *heap;

static os_error *Heap_Initialise(heap h, size_t size)
{
	return SWI(4, 0, SWI_OS_Heap, 0, h, 0, size);
}

static void *Heap_Claim(heap h, size_t size)
{
	void *fred;
	os_error *e;
	e = SWI(4, 3, SWI_OS_Heap, 2, h, 0, size, NULL, NULL, &fred);
	return e ? NULL : fred;
}

static os_error *Heap_Release(heap h, void *block)
{
	return SWI(3, 0, SWI_OS_Heap, 3, h, block);
}

static int Heap_ChangeHeapSize(heap h, int resize_by)
{
	int by;
	SWI(4, 4, SWI_OS_Heap, 5, h, 0, resize_by, 0, 0, 0, &by);
	return by;
}



/*--------------------------------------------------------------------------*/
/* Stuff below here is for using Dynamic areas (under RO3.5+)				*/
/*--------------------------------------------------------------------------*/

static int game_area = -1;	/* The DA the game is using */
static int font_area = -1;	/* The DA the fonts are using */

static void *game_area_base;	/* base address of game area */
static void *font_area_base;	/* base address of font area */

static int font_area_size;	/* size of the fonts' DA */
static int font_heap_size;	/* size of the fonts' heap */
static int game_area_size;	/* size of the game's DA */
static int game_heap_size;	/* size of the game's heap */

#define MAX_F_DA_SIZE (2<<20)	/* Max size of font area (2Mb) */
#define MAX_G_DA_SIZE (4<<20)	/* Max size of game area (4Mb) */
#define SHRINK_GRAN (4<<10)		/* Try to recalaim wastage > this (4Kb) */


/*
 | Free dynamic areas when we exit
 */
static void cleanup_memory(void)
{
	if (game_area != -1)
	{
		SWI(2, 0, SWI_OS_DynamicArea, 1, game_area);
		game_area = -1;
	}

	if (font_area != -1)
	{
		SWI(2, 0, SWI_OS_DynamicArea, 1, font_area);
		font_area = -1;
	}

}



/*
 | Set up the memory allocation stuff.
 | We check to see if DAs are possible and if so initialise two:
 | one for the game's use (via the rnalloc() hooks) and one for
 | our own use (for fonts, etc).
 |
 | Each area is created 16Kb in size, with a max size of 2/4Mb.
 |
 | If 'daf' is TRUE, an area is created for the fonts.
 | If 'dag' is TRUE, an area is created for the game.
 */
static void init_memory(int daf, int dag)
{
	os_error *e = NULL;

	if (!daf)
	{
		/* Paranoia */
		font_area = -1;
		font_area_base = 0;
	}
	else
	{
		e = SWI(9, 4, SWI_OS_DynamicArea, 0,	/* Create */
				-1,				/* Let OS allocate no. */
				16 << 10,		/* Initial size */
				-1,				/* Let OS allocate address */
				1 << 7,			/* Cacheable, bufferable, RW */
				MAX_F_DA_SIZE,	/* Max size */
				0,				/* handler */
				0,				/* handler workspace */
				VARIANT " font data",	/* Name */
				/* */
				NULL,			/* r0 */
				&font_area,		/* area number allocated */
				NULL,			/* r2 */
				&font_area_base	/* base address of area */
			);

		if (e)
		{
			game_area = font_area = -1;
			game_area_base = font_area_base = 0;	/* paranoia */
			return;
		}
		else
		{
			e = SWI(2, 3, SWI_OS_DynamicArea, 2, font_area,
					NULL, NULL, &font_area_size);
			if (e)
			{
				Error_ReportFatal(e->errnum, "%d:%s", e->errmess);
			}

			e = Heap_Initialise((heap) font_area_base, font_area_size);
			if (e)
			{
				Error_ReportFatal(e->errnum, "%d:%s", e->errmess);
			}
			font_heap_size = font_area_size;
		}
	}

	/* Make sure DA(s) are removed when we quit */
	atexit(cleanup_memory);

	if (!dag)
	{
		/* Paranoia */
		game_area = -1;
		game_area_base = 0;
	}
	else
	{
		e = SWI(9, 4, SWI_OS_DynamicArea, 0,	/* Create */
				-1,				/* Let OS allocate no. */
				16 << 10,		/* Initial size */
				-1,				/* Let OS allocate address */
				1 << 7,			/* Cacheable, bufferable, RW */
				MAX_G_DA_SIZE,	/* Max size */
				0,				/* handler */
				0,				/* handler workspace */
				VARIANT " game data",	/* Name */
				/* */
				NULL,			/* r0 */
				&game_area,		/* area number allocated */
				NULL,			/* r2 */
				&game_area_base	/* base address of area */
			);

		if (e)
		{
			game_area = -1;
			game_area_base = 0;	/* paranoia */
		}
		else
		{
			e = SWI(2, 3, SWI_OS_DynamicArea, 2, game_area,
					NULL, NULL, &game_area_size);
			if (e)
			{
				Error_ReportFatal(e->errnum, "%d:%s", e->errmess);
			}

			e = Heap_Initialise((heap) game_area_base, game_area_size);
			if (e)
			{
				Error_ReportFatal(e->errnum, "%d:%s", e->errmess);
			}
			game_heap_size = game_area_size;
		}
	}
}

static int grow_dynamicarea(int area, int by)
{
	os_error *e;
	e = SWI(2, 2, SWI_OS_ChangeDynamicArea, area, by, /**/ NULL, &by);
	/* Can't check errors since a 'failed' shrink returns one... */
	return by;
}


/*
 | Try to shrink the font-cache heap and area as much as possible.
 */
static void f_shrink_heap(void)
{
	int s;
	/* Shrink the heap as far as possible */
	font_heap_size -=
		Heap_ChangeHeapSize((heap) font_area_base, -MAX_F_DA_SIZE);
	/* Shrink the dynamic area if necessary */
	s = font_area_size - font_heap_size;
	if (s >= SHRINK_GRAN)
		font_area_size -= grow_dynamicarea(font_area, -s);
}

/*
 | Allocate a block of memory in the font heap
 */
static void *f_malloc(size_t size)
{
	void *c;
	int s;
	if (font_area == -1)
	{
		return malloc(size);
	}
	c = Heap_Claim((heap) font_area_base, size);

	if (!c)
	{
		/* The Claim failed.  Try to grow the area by the size of the block */
		s = grow_dynamicarea(font_area, size + 64);	/* 64 is overkill */
		if (!s)
		{
			return NULL;
		}
		font_area_size += s;
		s = font_area_size - font_heap_size;
		font_heap_size += Heap_ChangeHeapSize((heap) font_area_base, s);
		c = Heap_Claim((heap) font_area_base, size);
		if (c)
		{
			f_shrink_heap();
		}
	}
	return c;
}


/*
 | Free a block of memory in the font heap
 */
static void f_free(void *blk)
{
	os_error *e;
	if (font_area == -1)
	{
		free(blk);
		return;
	}
	e = Heap_Release((heap) font_area_base, blk);
	if (e)
		Msgs_ReportFatal(e->errnum, "err.swi", __LINE__, e->errmess);
	f_shrink_heap();
}





/*
 | Allocate a block of memory in the game heap
 */
G_MALLOC_PROT
{
	void *c;
	int s;

	if (game_area == -1)
	{
		return malloc((size_t) size);
	}
	c = Heap_Claim((heap) game_area_base, (size_t) size + 4);
	if (!c)
	{
		/* The Claim failed.  Try to grow the area by the size of the block */
		s = grow_dynamicarea(game_area, (size_t) size + 64);	/* 64 is overkill */
		if (!s)
		{
			return NULL;
		}
		game_area_size += s;
		s = game_area_size - game_heap_size;
		game_heap_size += Heap_ChangeHeapSize((heap) game_area_base, s);
		c = Heap_Claim((heap) game_area_base, (size_t) size + 4);
	}

	if (c)
	{
		strcpy((char *)c, "MUSH");
		c = (void *)(((int)c) + 4);
	}

	if (log_g_malloc)
		fprintf(stderr, "ralloc(%ld) == %p\n", (long)size, c);

	return c;
}


/*
 | Free a block of memory in the game heap
 |
 | The 'len' is to be compatible with z-virt.c (we don't need/use it)
 | Returns NULL.
 */
G_FREE_PROT
{
	os_error *e;
	int s;

	if (game_area == -1)
	{
		free(blk);
		return NULL;
	}

	if (log_g_malloc)
		fprintf(stderr, "rnfree(%p)\n", blk);

	if (strncmp(((char *)blk) - 4, "MUSH", 4))
		core("game heap corrupt / bad attempt to free memory");

	blk = (void *)(((int)blk) - 4);

	e = Heap_Release((heap) game_area_base, blk);
	if (e)
		Msgs_ReportFatal(e->errnum, "err.swi", __LINE__, e->errmess);

	/* Shrink the heap as far as possible */
	game_heap_size -=
		Heap_ChangeHeapSize((heap) game_area_base, -MAX_G_DA_SIZE);

	/* Shrink the dynamic area if necessary */
	s = game_area_size - game_heap_size;
	if (s >= SHRINK_GRAN)
		game_area_size -= grow_dynamicarea(game_area, -s);

	return NULL;
}


#endif /* USE_DA */



/*--------------------------------------------------------------------------*/

/*
 | New to 1.04: Sound support :)
 |
 | We use the PlayIt module (for convenience).
 |
 | The Lib/xtra/sound/sound.cfg file is used to map sample names onto
 | event names.
 |
 | Since textual names are used in the .cfg file, we need to have a lookup
 | table to translate them into numbers.  At present we use the
 | angband_sound_name array defined in variable.c
 |
 | Since there can be multiple sounds for each event we need to use a
 | list to store them.
 */

/* NB: This will be clipped to 10 under RISC OS 2 */
#define MAX_SAMPNAME_LEN 64




/*
 | The list format:
 */
typedef struct samp_node
{
	char sample_name[MAX_SAMPNAME_LEN + 1];	/* Sample name */
	struct samp_node *next;	/* -> next node */
}
SampNode;

typedef struct samp_info
{
	int samples;	/* # samples for this event */
	SampNode *samplist;	/* list of sample names */
}
SampInfo;


/*
 | Just need an array of SampInfos
 */
static SampInfo sample[SOUND_MAX];

/*
 | This flag will only be set non-zero if the SampInfo array is
 | valid.
 */
static int sound_initd = 0;


static void read_sound_config(void)
{
	int i;
	char buffer[2048];
	FILE *f;
	int max_sampname_len = truncate_names()? 10 : MAX_SAMPNAME_LEN;
	FILE *dbo = NULL;

	if (show_sound_alloc)
	{
		sprintf(buffer, "%s%s", resource_path, "sndmap/out");
		dbo = fopen(buffer, "w");
		if (!dbo)
		{
			core("can't create sndmap/out debugging file");
		}
	}

	if (!sound_initd)
	{
		/* Initialise the sample array */
		for (i = 0; i < SOUND_MAX; i++)
		{
			sample[i].samples = 0;
			sample[i].samplist = NULL;
		}
		sound_initd = 1;
	}
	else
	{
		/* Deallocate the sample lists */
		for (i = 0; i < SOUND_MAX; i++)
		{
			SampNode *si = sample[i].samplist;
			sample[i].samples = 0;
			sample[i].samplist = NULL;
			while (si)
			{
				SampNode *ns = si->next;
				free(si);
				si = ns;
			}
		}
	}


	/* Open the config file */
	sprintf(buffer, "%sSound:%s", RISCOS_VARIANT, "sound/cfg");
	f = fopen(buffer, "r");

	/* No cfg file => no sounds */
	if (!f)
	{
		if (show_sound_alloc)
		{
			fprintf(dbo, "** Can't open cfg file '%s'\n", buffer);
			fclose(dbo);
		}
		return;
	}

	/* Parse the file */
	while (fgets(buffer, sizeof(buffer), f))
	{
		char *sample_name;
		int event_number;

		/* Skip comments and lines that begin with whitespace */
		if (*buffer == '#' || isspace((unsigned char)*buffer))
		{
			continue;
		}

		/* Hack: ignore any line beginning '[' (section marker) */
		if (*buffer == '[')
		{
			continue;
		}

		/* Place a NULL after the event name and find the first sample name */
		sample_name = buffer;
		while (*sample_name && !isspace((unsigned char)*sample_name))
			sample_name++;

		/* Bad line? */
		if (*sample_name == 0)
		{
			continue;
		}						/* just ignore it */

		/* Terminate the sample name */
		*sample_name++ = 0;

		/* Look up the event name to get the event number */
		for (event_number = SOUND_MAX - 1; event_number >= 0; event_number--)
			if (!strcmp(buffer, angband_sound_name[event_number]))
				break;

		/* No match -> just ignore the line */
		if (event_number < 0)
		{
			if (show_sound_alloc)
				fprintf(dbo, "* Ignoring unknown event '%s'\n", buffer);
			continue;
		}

		/* Find the = */
		while (*sample_name && *sample_name != '=')
			sample_name++;

		/* Bad line? */
		if (*sample_name == 0)
		{
			continue;
		}						/* just ignore it */

		/* Skip the '=' */
		sample_name++;


		/*
		   | Now we find all the sample names and add them to the
		   | appropriate list in the sample mapping array
		 */

		while (*sample_name)
		{
			char *s;
			SampNode *sn;

			/* Find the start of the next word */
			while (isspace((unsigned char)*sample_name) && *sample_name)
				sample_name++;

			/* End of line? */
			if (!*sample_name)
			{
				break;
			}

			/* Find the end of the sample name */
			s = sample_name;	/* start of the name */
			while (!isspace((unsigned char)*sample_name) && *sample_name)
				sample_name++;

			/* Hack: shorten sample names that are too long */
			if ((sample_name - s) > max_sampname_len)
				s[max_sampname_len] = ' ';

			/* Allocate a node in the sample list for the event */
			if ((sn = malloc(sizeof(SampNode))) == NULL)
				core("Out of memory (scanning sound.cfg)");

			/* Link the node to the list */
			sn->next = sample[event_number].samplist;
			sample[event_number].samplist = sn;

			/* Imcrement the sample count for that event */
			sample[event_number].samples++;

			/*
			   | Copy the sample name into the node, converting it into
			   | RISC OS style as we go.
			 */
			for (i = 0; !isspace((unsigned char)s[i]) && s[i]; i++)
			{
				if (s[i] == '.')
					sn->sample_name[i] = '/';
				else if (s[i] == '/')
					sn->sample_name[i] = '.';
				else
					sn->sample_name[i] = s[i];
			}
			/*
			   | The sample name '*' is special and means "no new sound"
			   | so don't store a filename for these mappings.
			 */
			if (i == 1 && sn->sample_name[0] == '*')
			{
				i = 0;
			}
			sn->sample_name[i] = 0;
		}
	}

	/* Close the file */
	fclose(f);

	if (show_sound_alloc)
	{
		int i;
		SampNode *l;

		for (i = 0; i < SOUND_MAX; i++)
		{
			fprintf(dbo, "\n\nEvent '%s'", angband_sound_name[i]);
			fprintf(dbo, " (%d sounds)\n", sample[i].samples);
			for (l = sample[i].samplist; l; l = l->next)
				fprintf(dbo, "\t%s\n", l->sample_name);
		}
		fclose(dbo);
	}

}



/*
 | Try to make sure that PlayIt is loaded.
 | This requires AngSound rel. 4
 */
static void check_playit(void)
{
	if (SWI(2, 0, SWI_OS_Module, 18, "PlayIt"))
	{
		int t;
		SWI(2, 1, SWI_OS_File, 17, "Angsound:LoadPlayIt", &t);
		if (t == 1)
			SWI(1, 0, SWI_OS_CLI,
				"RMEnsure PlayIt 0.00 Run AngSound:LoadPlayIt");
	}
}



static void initialise_sound(void)
{
	/* Load the configuration file */
	Hourglass_On();
	read_sound_config();
	check_playit();

	/* Set the sound hook */
	sound_hook = play_sound;

	Hourglass_Off();
}



static void play_sample(char *leafname)
{
	char buffer[260];

	strcpy(buffer, "%playit_stop");

	if (!SWI(1, 0, SWI_OS_CLI, buffer))
	{
		SWI(1, 0, SWI_PlayIt_Volume, sound_volume);
		sprintf(buffer, "%%playit_play %sSound:%s", RISCOS_VARIANT, leafname);
		SWI(1, 0, SWI_OS_CLI, buffer);
	}

	return;
}



static void play_sound(int event)
{
	/* Paranoia */
	if (!sound_initd || !enable_sound)
		return;

	/* Paranoia */
	if (event < 0 || event >= SOUND_MAX)
		return;

	/* msg_format("Sound '%s'",angband_sound_name[event]); */

	/* Choose a sample */
	if (sample[event].samples)
	{
		int s = rand() % sample[event].samples;
		SampNode *sn = sample[event].samplist;
		while (s--)
		{
			sn = sn->next;
			if (!sn)
			{
				plog("Adny botched the sound config - please send him a copy of your sound/cfg file.");
			}
		}
		if (*(sn->sample_name))
			play_sample(sn->sample_name);
	}
}




/*--------------------------------------------------------------------------*/

/*
 | This stuff is for the Term_user hook
 */
#ifdef ZANGBAND_TERM_PACKAGE
  #define PUT_FSTR put_fstr
  #define COL(colour) CLR_##colour
#else
  #define PUT_FSTR display_line
  #define COL(colour) TERM_##colour

static void display_line(int x, int y, int c, const char *fmt, ...)
{
	va_list ap;
	char buffer[260];

	va_start(ap, fmt);
	vstrnfmt(buffer, sizeof(buffer), fmt, ap);
	Term_putstr(x, y, -1, c, buffer);
	va_end(ap);
}

#endif


/*
 | Let the user change the alarm message
 */
static void do_alarm_message_input(int y)
{
	int k;
	int inspos = strlen(alarm_message);
	char old_message[52];

	strcpy(old_message, alarm_message);

	do
	{
		PUT_FSTR(26, y, COL(YELLOW), "%-51s", alarm_message);
		Term_gotoxy(26 + inspos, y);
		k = inkey();
		switch (k)
		{
			case 21:			/* ^U */
				*alarm_message = 0;
				inspos = 0;
				break;
			case 128:  case 8:	/* delete */
				if (inspos > 0)
				{
					alarm_message[--inspos] = 0;
				}
				break;
			case 27:			/* escape */
				strcpy(alarm_message, old_message);
				k = 13;
				break;
			default:
				if (k > 31 && k < 127 && inspos < 50)
				{
					alarm_message[inspos++] = k;
					alarm_message[inspos] = 0;
				}
		}
	}
	while (k != 13);

	PUT_FSTR(26, y, COL(WHITE), "%-51s", alarm_message);
}


#define tum_col(X)  ((X) ? COL(L_BLUE) : COL(WHITE) )
#define tum_onoff(X)  ((X) ? "On " : "Off")

static errr Term_user_acn(int n)
{
	bool cursor_state;
	int optn = 0;
	int k, adj;
	int redraw_mung = 0;
	int max_opt = 11;

	/* Will be true if the alarm choices need to be (re)saved */
	int alarm_modified = 0;

	/*
	   | Hack: let the desktop front end know that
	   | the user menu is active...
	 */
	user_menu_active = TRUE;


	/*
	   | Hack: alarm type 1 /looks/ the same as type 3 but doesn't get
	   | cancelled as a type 3 would.  This allows alarms to go off and
	   | be cancelled without affecting the alarm type whilst it's being
	   | set up here.
	 */
	if (alarm_type == 3)
	{
		alarm_type = 1;
	}

	/*
	   | Store the screen
	 */
	Term_activate(&(data[0].t));
	Term_save();
	Term_get_cursor(&cursor_state);
	Term_set_cursor(TRUE);

	do
	{
		redraw_mung = 0;
		Term_clear();
		PUT_FSTR(2, 1, COL(YELLOW), "%s %s", VARIANT, VERSION);
		PUT_FSTR(2, 2, COL(SLATE), "Front-end %s", PORTVERSION);
		PUT_FSTR(2, 4, COL(WHITE), "Use cursor up/down to select an option then cursor left/right to alter it.");
		PUT_FSTR(2, 5, COL(WHITE), "Hit 'S' to save these settings (alarm settings are saved automatically).");
		PUT_FSTR(2, 6, COL(WHITE), "Hit ESC to return to the game.");

		for (k = 0; k < 32; k++) Term_putch(31 + k + (k / 2), 8, k / 2, '#');

		do
		{
			PUT_FSTR(2, 8, tum_col(optn == 0),
			             "     Gamma correction : %i.%02i", gamma / 100, gamma % 100);
			PUT_FSTR(2, 9, tum_col(optn == 1),
			             "     Force monochrome : %s", tum_onoff(force_mono));
			PUT_FSTR(2, 10, tum_col(optn == 2),
			             "        Sound effects : %s", tum_onoff(enable_sound));
			PUT_FSTR(2, 11, tum_col(optn == 3),
			             "  Sound effect volume : ");
			PUT_FSTR(26, 11,
			             sound_volume > 127 ? COL(RED) : tum_col(optn == 3),
			             "%-3d", sound_volume);
			PUT_FSTR(30, 11, tum_col(optn == 3), "(127 = full volume)");
			PUT_FSTR(2, 12, tum_col(optn == 4),
			             "     Start fullscreen : %s",
			             tum_onoff(start_fullscreen));
			PUT_FSTR(30, 12, tum_col(optn == 4),
			             "(also selects fullscreen/desktop now)");
			PUT_FSTR(2, 13, tum_col(optn == 5),
			             "        Use hourglass : %s", tum_onoff(use_glass));
			PUT_FSTR(2, 14, tum_col(optn == 6),
			             "'Hard' input flushing : %s", tum_onoff(hack_flush));

			PUT_FSTR(2, 16, tum_col(optn == 7),
			             "           Alarm type : %-20s",
			             alarm_types[alarm_type]);
			PUT_FSTR(2, 17, COL(WHITE),
			             "                 Time : ");
			PUT_FSTR(26, 17, tum_col(optn == 8), "%02d", alarm_h);
			PUT_FSTR(28, 17, COL(WHITE), ":");
			PUT_FSTR(29, 17, tum_col(optn == 9), "%02d", alarm_m);
			PUT_FSTR(2, 18, tum_col(optn == 10),
			             "              Message : %-51s", alarm_message);
			PUT_FSTR(2, 19, tum_col(optn == 11),
			             "                 Beep : %s", tum_onoff(alarm_beep));

#ifdef FE_DEBUG_INFO
			PUT_FSTR(2, 23, tum_col(optn == 23), "Show debug info");
			max_opt = 12;
#endif

			switch (optn)
			{
				case 12: Term_gotoxy(26, 23);
					break;
				case 11: Term_gotoxy(26, 19);
					break;
				case 10: Term_gotoxy(26, 18);
					break;
				case 9: Term_gotoxy(29, 17);
					break;
				case 8: Term_gotoxy(26, 17);
					break;
				case 7: Term_gotoxy(26, 16);
					break;
				default: Term_gotoxy(26, optn + 8);
			}

			k = inkey();
			adj = (k == '4' || k == 'h') ? -1 : (k == '6' || k == 'l') ? 1 : 0;

			switch (k)
			{
				case 18:		/* Hack: force the screen to update */
					redraw_mung = 1;
					k = 27;
					break;
				case 's':  case 'S':
					save_choices();
					PUT_FSTR(2, 23, COL(YELLOW), "Options saved.     ");
					Term_fresh();
					Term_xtra(TERM_XTRA_DELAY, 750);
					Term_erase(2, 23, 60);
					break;
				case '8':  case 'k':
					if (--optn < 0)
					{
						optn = max_opt;
					}
					break;
				case '2':  case 'j':
					if (++optn > max_opt)
					{
						optn = 0;
					}
					break;
				case 13:  case 32:  case 't':	/* Allow return, space and t to toggle some options */
				case '4':  case 'h':
				case '6':  case 'l':
				{
					switch (optn)
					{
						case 0:	/* Gamma correction */
							gamma += adj * 5;
							if (gamma > 900)
							{
								gamma = 900;
							}
							if (gamma < 5)
							{
								gamma = 5;
							}
							Term_xtra(TERM_XTRA_REACT, 0);
#ifndef FULLSCREEN_ONLY
							set_gamma_window_state();
#endif /* FULLSCREEN_ONLY */
							/* flush(); */
							Term_fresh();
							break;
						case 1:	/* Force monochrome */
							force_mono = !force_mono;
							if (fullscreen_font)
								redraw_areaFS(0, 0, 80, 24);
							else
								Term_xtra(TERM_XTRA_REACT, 0);
							/* flush(); */
							Term_fresh();
							break;
						case 2:	/* Sound enable / disable */
							enable_sound = !enable_sound;
#ifndef FULLSCREEN_ONLY
							set_sound_window_state();
#endif /* FULLSCREEN_ONLY */
							if (enable_sound)
							{
								initialise_sound();
							}
							break;
						case 3:	/* Sound volume */
							sound_volume += adj;
							if (sound_volume < SOUND_VOL_MIN)
								sound_volume = SOUND_VOL_MIN;
							if (sound_volume > SOUND_VOL_MAX)
								sound_volume = SOUND_VOL_MAX;
#ifndef FULLSCREEN_ONLY
							set_sound_window_state();
#endif /* FULLSCREEN_ONLY */
							break;
						case 4:	/* Start fullscreen */
							start_fullscreen = !start_fullscreen;
							if (start_fullscreen)
								enter_fullscreen_mode();
							else if (!minimise_memory)
								leave_fullscreen_mode();
							break;
						case 5:	/* Start fullscreen */
							use_glass = !use_glass;
							if (!use_glass)
							{
								if (glass_on)
								{
									Hourglass_Off();
								}
								glass_on = 0;
							}
							break;
						case 6:	/* hack flush */
							hack_flush = !hack_flush;
							break;
						case 7:	/* Alarm on/off */
							alarm_type += adj;
							if (adj)
							{
								alarm_modified = 1;
							}
							if (alarm_type > 2)
							{
								alarm_type = 0;
							}
							if (alarm_type < 0)
							{
								alarm_type = 2;
							}
							if (!alarm_type && alarm_disp)
							{
								ack_alarm();
							}	/* XXXXX Cancel an already active alarm? */
							break;
						case 8:	/* Alarm hours */
							alarm_h += adj;
							if (adj)
							{
								alarm_modified = 1;
							}
							if (alarm_h < 0)
							{
								alarm_h += 24;
							}
							if (alarm_h > 23)
							{
								alarm_h -= 24;
							}
							if (alarm_disp)
							{
								ack_alarm();
							}
							break;
						case 9:	/* Alarm minutes */
							alarm_m += adj;
							if (adj)
							{
								alarm_modified = 1;
							}
							if (alarm_m < 0)
							{
								alarm_m += 60;
							}
							if (alarm_m > 59)
							{
								alarm_m -= 60;
							}
							if (alarm_disp)
							{
								ack_alarm();
							}
							break;
						case 10:
							alarm_modified = 1;
							do_alarm_message_input(18);
							break;
						case 11:
							alarm_modified = 1;
							alarm_beep = !alarm_beep;
							break;
						case 12:
							show_debug_info();
							redraw_mung = 1;
							k = 27;
							break;
					}
				}
			}
		}
		while (k != 27);
	}
	while (redraw_mung);

	/* Rehack the alarm type: */
	if (alarm_type == 1)
	{
		alarm_type = 3;
	}

	if (alarm_modified)
	{
		write_alarm_choices();
	}

	Term_set_cursor(cursor_state);

	/* Restore the screen */
	Term_load();

	/*
	   | Hack: tell the desktop front end that we're done.
	 */
	user_menu_active = FALSE;

	return 0;
}




/*--------------------------------------------------------------------------*/

#ifdef USE_FILECACHE

/*
 | 'Random' File-cacheing for *band.
 |
 | Rewritten since as of Zang 225 the mechanism for handling
 | these files has changed dramatically and the old system
 | is no longer viable.
 |
 | These new functions basically provide an alternative to the
 | normal my_fopen() (or fopen()) and my_fgets() functions.
 |
 | To use the file caching it is therefore necessary to alter
 | files.c to call cached_fopen(), cached_fclose() and cached_fgets()
 | rather than the normal functions.
 |
 | Note that these funtions will only work for files that are intended
 | to be read as a series of \n terminated lines of ASCII text using my_fgets().
 |
 */

/*
 | Hack: use the game's dynamic area if possible:
 */
#define fc_malloc(X) (g_malloc(X))
#define fc_free(X) (g_free(X,0))

#ifndef ABBR_FILECACHE
/*
 | Make these to do nothing.  They'll never
 | be called anyway.  Having them present makes
 | for neater code later on (ie. we use a variable
 | rather than the pre-processor to decide whether
 | to do compression).
 */
static int compress_string(char *os, char *s)
{
	core("main-acn internal logic error 001");
	return 0;
}
static int decompress_string(char *d, char *s, int max_len)
{
	core("main-acn internal logic error 002");
	return 0;
}
static int compressed_length(char *s)
{
	core("main-acn internal logic error 003");
	return 0;
}

#else

/*
 | When caching files we try to use some abbreviations.
 | We use both whole words and pairs of letters.
 | NB: For this to work, the file must contain only
 | 7 bit characters.
 */
static char *abbrv_w[] =
{
	/* These words all begin with a space */
	" of ", " the ", " you ", " to ", " a ", " says", " is ", " that ", " and ",
	" your ", " are ", " it ", " be ", " for ", " me", " will ", " in ",
	" not ", " this ", " have ", " can ", " on ", " my ", " with ", " say ",
	" all", " by ", " get ", " but ", " just ", " die", " as ", " time ",
		" if ",
	" like ",
	/* These words do not */
	"I ", "The ", "You ", "They ", "It ", "don", 0
};

/* Number of words */
#define FC_ABBRV_NUMWORDS 41

/* Number of them that don't start with a space */
#define FC_ABBRV_NONSPC   6

/*
 | NB: No letter pair may start with \0.
 */
static char abbrv_lp[] =
	"e  ttht s heiner aoure'\0, anonf  sd y r  ongofator.\0"
	"n arllstha wes m ieaisen bl  yndtoo yometele d f hve"
	"ayuralitneelN: chig ilroassaseliti lraa otedbede 'ri" "..u  nntno!'ee\0\0";


/*
 | Compress the given string using the abbreviation tables above.
 | Returns compressed length *including* terminator (it may
 | be part of an abbreviation, you see...)
 | Note that we can compress the string in-place, ie. 'os' may be
 | the same as 's'.
 */
static int compress_string(char *os, char *s)
{
	char *o, *f, *d;
	int i;

	o = os;

	while (*s)
	{
		int fw, lw;
		if (*s == ' ')
		{
			fw = 0;
			lw = FC_ABBRV_NUMWORDS - FC_ABBRV_NONSPC;
		}
		else
		{
			fw = FC_ABBRV_NUMWORDS - FC_ABBRV_NONSPC;
			lw = FC_ABBRV_NUMWORDS;
		}
		for (i = fw; i < lw; i++)
		{
			d = abbrv_w[i];		/* Word to check against */
			for (f = s; *f && *f == *d; f++, d++)
				;
			if (*d == 0)		/* Match? */
			{
				s = *f ? f : f - 1;	/* Update string pointer */
				*o++ = 128 + i;	/* store code */
				break;			/* Quit looking for words */
			}
		}

		/* Do we need to check the letter pairs? */
		if (i == lw)
		{
			for (i = 0; abbrv_lp[i]; i += 2)
			{
				if (s[0] == abbrv_lp[i] && s[1] == abbrv_lp[i + 1])
				{
					*o++ = 128 + FC_ABBRV_NUMWORDS + i / 2;
					/* NB: If the next character is the terminator then we're done. */
					if (!s[1])
					{
						return (o - os);
					}
					s += 2;		/* Quit looking for letters */
					break;
				}
			}
			/* NB: This next check is only safe because no letter pair starts with a NULL */
			if (!abbrv_lp[i])
				*o++ = *s++;
		}
	}

	/* Don't forget that terminator! */
	*o++ = 0;

	return o - os;
}

/*
 | As compress_string (above), but stores nothing and
 | only returns the length of the compressed string.
 */
static int compressed_length(char *s)
{
	char *f, *d;
	int i, l;

	l = 0;
	while (*s)
	{
		int fw, lw;
		if (*s == ' ')
		{
			fw = 0;
			lw = FC_ABBRV_NUMWORDS - FC_ABBRV_NONSPC;
		}
		else
		{
			fw = FC_ABBRV_NUMWORDS - FC_ABBRV_NONSPC;
			lw = FC_ABBRV_NUMWORDS;
		}
		for (i = fw; i < lw; i++)
		{
			d = abbrv_w[i];
			for (f = s; *f && *f == *d; f++, d++)
				;
			if (*d == 0)		/* Match? */
			{
				s = *f ? f : f - 1;	/* Update string pointer */
				l++;			/* increment output length */
				break;			/* Quit looking for words */
			}
		}

		/* Do we need to check the letter pairs? */
		if (i == lw)
		{
			for (i = 0; abbrv_lp[i]; i += 2)
			{
				if (s[0] == abbrv_lp[i] && s[1] == abbrv_lp[i + 1])
				{
					l++;		/* increment output length */
					/* NB: If the next character is the terminator then we're done. */
					if (!s[1])
					{
						return l;
					}
					s += 2;		/* Quit looking for letters */
					break;
				}
			}
			/* NB: This next check is only safe because no letter pair starts with a NULL */
			if (!abbrv_lp[i])
			{
				l++;
				s++;
			}
		}
	}
	/* Don't forget that terminator! */
	return l + 1;
}



/*
 | Decompress the given string 's' into the buffer at 'd'.
 | At most, max_len characters (incl. \0 terminator) will be
 | written into d.
 | Returns the length of 's'.
 */
static int decompress_string(char *d, char *s, int max_len)
{
	char *os = s;

	while (max_len > 1)
	{
		int nc = *s++;	/* Get next character */

		if (nc < 128)			/* Is it a plain character? */
		{
			if (0 == (*d++ = nc))
			{
				break;
			}
			max_len--;
		}
		else					/* Abbreviation to expand. */
		{
			if (nc >= FC_ABBRV_NUMWORDS + 128)	/* Letter pair? */
			{
				*d++ = abbrv_lp[(nc - (FC_ABBRV_NUMWORDS + 128)) * 2];
				if (0 ==
					(*d++ = abbrv_lp[(nc - (FC_ABBRV_NUMWORDS + 128)) * 2 + 1]))
					break;
				max_len -= 2;
			}
			else				/* It's a word */
			{
				char *ws = abbrv_w[nc - 128];
				while (*ws && max_len > 1)
				{
					*d++ = *ws++;
					max_len--;
				}
			}
		}
	}

	/* Skip over the rest of the abbreviated string if we ran out of space */
	if (max_len <= 1)			/* Out of space? */
	{
		int nc;
		*d = 0;					/* Terminate */
		do
		{
			nc = *s++;			/* Next char */
			if (nc >= 128 + FC_ABBRV_NUMWORDS)	/* Ignore words */
				nc = abbrv_lp[(nc - (FC_ABBRV_NUMWORDS + 128) * 2) + 1];	/* Only check 2nd letter of pair */
		}
		while (nc);
	}
	return s - os;				/* Length of abbreviated string */
}

#endif /* ABBR_FILECACHE */


/* Each entry in the cache looks like this: */
typedef struct fce_
{
	char *name;	/* canonical pathname of file */
	char *text;	/* text (be it compressed or not) */
	char *eof;	/* byte beyond the last byte of text */
	int used;	/* access counter when the file was last used */
	int compressed;	/* compression method, (ie. 0 for none or 1 for abbreviations) */
}
FileCacheEntry;

/*
 | The handles we chuck around are pointers to one of these structs.
 | Note that since we actually return (and take) |FILE*|s we just
 | compare the value of a |FILE*| with the limits of the array of
 | |CachedFileHandle|s to decide whether its 'ours' or a genuine
 | (ie. stdio) file handle.  This /is/ pretty lame, I know...
 */
typedef struct cfh_
{
	char *ptr;	/* sequential file pointer, as it were */
	FileCacheEntry *fce;	/* ->the file-cache entry data */
}
CachedFileHandle;

#define MAX_OPEN_CACHED_FILES	16	/* We allow up to 16 of these files open at once */
#define MAX_CACHE_ENTRIES		64	/* We allow up to 64 cache entries */

static FileCacheEntry *file_cache;	/* to be used as file_cache[MAX_CACHE_ENTRIES] */
static CachedFileHandle *cached_file_handle;	/* to be used as cached_file_handle[MAX_OPEN_CACHED_FILES] */
static int file_cache_initd = 0;	/* Is the cache initialised? */
static int file_cache_size = 0;	/* Total size of the cached files */
static int fc_access_counter = 1;	/* incremented on each cache access */
/*
 | Pre-calculate max. possible value of a FILE* (ie. address in memory)
 | that could be a valid |CachedFileHandle*|.
 */
static FILE *max_cfh_addr;	/* == (FILE*) (&(cached_file_handle[MAX_OPEN_CACHED_FILES-1])) */

/*
 | Initialise the file cache
 */
static void init_file_cache(void)
{
	int i;

	/* Allocate storage */
	file_cache = fc_malloc(MAX_CACHE_ENTRIES * sizeof(FileCacheEntry));
	cached_file_handle =
		fc_malloc(MAX_OPEN_CACHED_FILES * sizeof(CachedFileHandle));

	if (!file_cache || !cached_file_handle)
	{
		/* Disable file-caching */
		if (file_cache)
		{
			fc_free(file_cache);
		}
		if (cached_file_handle)
		{
			fc_free(cached_file_handle);
		}
		use_filecache = 0;
	}
	else
	{
		/* Initialise the cache */
		for (i = 0; i < MAX_CACHE_ENTRIES; i++)
			file_cache[i].name = NULL;
		for (i = 0; i < MAX_OPEN_CACHED_FILES; i++)
			cached_file_handle[i].fce = NULL;
		fc_access_counter = 1;
		file_cache_size = 0;
		max_cfh_addr =
			(FILE *)(&(cached_file_handle[MAX_OPEN_CACHED_FILES - 1]));
	}
	file_cache_initd = 1;
}



/*
 | Cache the specified file, returning either the cache entry
 | that it has been cached at, or NULL for failure.
 |
 | Note that for the abbreviated file cache a temporary file
 | is used to allow the compression to be applied just once.
 | (otherwise it has to be done twice - once to determine the
 | eventual compressed size and once to actually store and compress
 | it).
 */
static FileCacheEntry *cache_file(char *name)
{
	int i, size = 0;
	FILE *fp;
	char buffer[1024];
	char *d;

	FILE *tf = NULL;	/* Used if abbr_filecache and abbr_tmpfile are set */
	char cfn[1024];	/* Used if abbr_filecache and abbr_tmpfile are set */

	/* Find the first free slot in the cache */
	for (i = 0; i < MAX_CACHE_ENTRIES; i++)
		if (!file_cache[i].name)
			break;

	/* No more entries? */
	if (i >= MAX_CACHE_ENTRIES)
	{
		return NULL;
	}

	/* Set up the info on the file */
	if ((file_cache[i].name = string_make(name)) == NULL)
	{
		return NULL;
	}

	/* Open the file */
	fp = my_fopen(name, "r");
	if (!fp)
	{
		fc_free(file_cache[i].name);
		file_cache[i].name = 0;
		return NULL;
	}

	/* Open/create tempfile if need be: */
	if (abbr_filecache && abbr_tmpfile)
	{
		/* Hack: Form the pathname of the cached compressed file (in canonical form) */
		sprintf(cfn, "%s%s", scrap_path,
				riscosify_name(name + strlen(resource_path)));
		/* Ensure that that particular directory exists... */
		ensure_path(cfn);
		/* Check whether cache file is out of date */
		if (file_is_newer(riscosify_name(name), cfn))
		{
			tf = fopen(cfn, "wb");
			size = 0;
		}
		else
		{
			tf = fopen(cfn, "rb");
			if (tf)
			{
				size = File_Size(cfn);
			}
		}
	}

	/* If we don't have the cached file (but want it), compress the source text to it */
	if (tf)
	{
		if (!size)
		{
			int k;
			while (!my_fgets(fp, buffer, sizeof(buffer)))
			{
				if (smart_filecache && (!*buffer || *buffer == '#'))
					continue;
				k = compress_string(buffer, buffer);
				if (fwrite(buffer, 1, k, tf) != k)
				{
					fclose(tf);
					remove(cfn);
					core("error writing tempfile");
				}
				size += k;
			}
			fclose(tf);
			tf = fopen(cfn, "rb");
		}
	}
	else
	{
		/* Count the number of bytes */
		while (!my_fgets(fp, buffer, sizeof(buffer)))
		{
			if (smart_filecache && (!*buffer || *buffer == '#'))
				continue;
			if (abbr_filecache)
				size += compressed_length(buffer);
			else
				size += strlen(buffer) + 1;
		}
	}

	/* Close the (source) file */
	my_fclose(fp);

	/* Allocate enough storage for the text */
	file_cache[i].text = fc_malloc(size + 1L);
	if (!file_cache[i].text)
	{
		fc_free(file_cache[i].name);
		file_cache[i].name = 0;
		if (tf)
		{
			fclose(tf);
		}
		return NULL;
	}

	/* Do we have a tempfile to load? */
	if (tf)
	{
		if (fread(file_cache[i].text, 1, size, tf) != size)
			core("error reading tempfile");
		fclose(tf);
	}
	else
	{
		/* Re-open the file... */
		fp = my_fopen(name, "r");
		if (!fp)
		{
			fc_free(file_cache[i].name);
			fc_free(file_cache[i].text);
			file_cache[i].name = 0;
			return NULL;
		}

		/* And read it into the buffer... */
		d = file_cache[i].text;
		while (!my_fgets(fp, buffer, sizeof(buffer)))
		{
			if (smart_filecache && (!*buffer || *buffer == '#'))
				continue;
			if (abbr_filecache)
				d += compress_string(d, buffer);
			else
			{
				strcpy(d, buffer);
				d += strlen(buffer) + 1;
			}
		}

		if ((d - file_cache[i].text) != size)
		{
			debug("Calculated size is %d, pointer offset is %d", size,
				  (d - file_cache[i].text));
			core("Cached file is larger than calculated!");
		}

		/* Close the file */
		my_fclose(fp);
	}

	/* Set up the 'last accessed' value, etc. */
	file_cache[i].used = fc_access_counter++;
	file_cache[i].eof = file_cache[i].text + size;
	file_cache[i].compressed = abbr_filecache;
	file_cache_size += size;

	/* Return success */
	return &(file_cache[i]);
}


/*
 | Discard a file from the cache
 */
static void discard_cached_file(int i)
{
	if (!file_cache[i].name)
	{
		return;
	}							/* invalid request */
	fc_free(file_cache[i].text);
	fc_free(file_cache[i].name);
	file_cache_size -= (file_cache[i].eof) - (file_cache[i].text);
	file_cache[i].name = 0;
}


/*
 | Attempt to flush as much of the cache as required
 | to bring it within the size limit.
 | If protect != 0 then that entry in the cache won't be flushed.
 */
static void flush_file_cache(FileCacheEntry * protect)
{
	int i, j, done;
	int oldest_u, oldest_e;
	FileCacheEntry *fce;
	int needed = (4 << 10);	/* Hack: try to free at least 4K */

	done = (file_cache_size + needed) <= max_file_cache_size;

	while (!done)
	{
		oldest_u = fc_access_counter;
		oldest_e = -1;

		fce = file_cache;
		/* Find oldest entry that isn't in use */
		for (i = 0; i < MAX_CACHE_ENTRIES; fce++, i++)
		{
			if (fce == protect)
			{
				continue;
			}					/* Hack ;) */
			if (fce->name)		/* Is this cache slot full? */
			{
				for (j = 0; j < MAX_OPEN_CACHED_FILES; j++)
					if (cached_file_handle[j].fce == fce)
						break;
				if (j < MAX_OPEN_CACHED_FILES)
					continue;	/* Cached file is still open */
				if (fce->used < oldest_u)
				{
					oldest_e = i;
					oldest_u = file_cache[i].used;
				}
			}
		}

		if (oldest_e < 0)
			done = 1;			/* We can flush nothing more */
		else
		{
			discard_cached_file(oldest_e);
			done = (file_cache_size + needed) <= max_file_cache_size;
		}
	}
}


/*
 | Locate the specified file within the cache.
 | Returns NULL if the file is not cached
 */
static FileCacheEntry *find_cached_file(char *name)
{
	int i;
	FileCacheEntry *fce = file_cache;

	for (i = 0; i < MAX_CACHE_ENTRIES; i++, fce++)
	{
		if (fce->name)
			if (streq(fce->name, name))
				return fce;
	}
	return NULL;
}



/*--------------------------------------------------------------------------*/
/* Externally visible file cache stuff										*/
/*--------------------------------------------------------------------------*/

/*
 | Open a file...
 | Returns the file cache handle of the file, or NULL for failure.
 | Note that if mode is anything other than "r" the call defers to
 | my_fopen().
 |
 | NB: The returned handle is almost certainly *NOT* a |FILE*|
 | (although it may be if the cache cannot accomodate the file).
 |
 | Therefore, you *MUST* ensure that any file opened with cached_fopen()
 | is only ever accessed via cached_fgets() and cached_fclose().
 |
 | Failure to do so will result in, ahem, unpleasantness.  Extreme
 | unpleasantness.
 */
FILE *cached_fopen(char *name, char *mode)
{
	FileCacheEntry *fcs = NULL;
	int fch;

	if (strcmp(mode, "r") || !use_filecache)
		return my_fopen(name, mode);

	if (!file_cache_initd)
	{
		init_file_cache();
	}

	if (max_file_cache_size >= 0)
	{
		/* Find a free cache entry */
		for (fch = 0; fch < MAX_OPEN_CACHED_FILES; fch++)
			if (!cached_file_handle[fch].fce)
				break;

		/* Out of handles? */
		if (fch >= MAX_OPEN_CACHED_FILES)
			return my_fopen(name, mode);

		/* Is the file already cached? */
		fcs = find_cached_file(name);
		if (!fcs)
		{
			/* File wasn't cached, so cache it */
			flush_file_cache(NULL);	/* Clean stuff out of the cache if need be */
			fcs = cache_file(name);	/* Cache the new file */
			flush_file_cache(fcs);	/* Flush, but keep the latest file */
		}
	}

	/* Did we fail to cache the file? */
	if (!fcs)
	{
		return my_fopen(name, mode);
	}

	/* File was cached OK */
	cached_file_handle[fch].ptr = fcs->text;	/* Init sequential pointer */
	cached_file_handle[fch].fce = fcs;	/* Cache block pointer */
	fcs->used = fc_access_counter++;	/* Opening the file counts as an access */

	return (FILE *)(&cached_file_handle[fch]);
}


/*
 | Close a file
 */
errr cached_fclose(FILE *fch_)
{
	CachedFileHandle *fch;

	/* Is the FILE* genuine? */
	if ((fch_ < (FILE *)cached_file_handle) || (fch_ > max_cfh_addr))
		return my_fclose(fch_);

	fch = (CachedFileHandle *) fch_;

	/* Check for "Ooopses": */
	if (fch->fce == NULL)
		core("cached_fclose called on a non-open file handle");

	flush_file_cache(NULL);		/* Clean out the cache if need be */
	fch->fce = NULL;			/* Mark file handle as inactive */

	return 0;
}


/*
 | Do the my_fgets thing on a file
 */
errr cached_fgets(FILE *fch_, char *buffer, int max_len)
{
	CachedFileHandle *fch;
	char *eof;
	char *ptr;

	/* Is the FILE* genuine? */
	if ((fch_ < (FILE *)cached_file_handle) || (fch_ > max_cfh_addr))
		return my_fgets(fch_, buffer, max_len);

	fch = (CachedFileHandle *) fch_;

	/* Check for "Oopses": */
	if (!file_cache_initd)
		core("cached_fgets() on uninitialised file-cache");
	if (!fch->fce)
		core("cached_fgets called for a un-open file");

	eof = fch->fce->eof;
	ptr = fch->ptr;

	/* Out of bounds? */
	if (ptr >= eof)
	{
		return 1;
	}							/* Read failed */

	/*
	   | Read the next line, up to \0 (which would have v=been \n in the original file),
	   | or max_len-1 characters
	 */
	if (fch->fce->compressed)
		ptr += decompress_string(buffer, ptr, max_len);
	else
	{
		if (eof - ptr < max_len)
		{
			max_len = eof - ptr;
		}
		for (; max_len >= 1; max_len--)
			if ((*buffer++ = *ptr++) == 0)
				break;
		*buffer = 0;			/* terminate (paranoia) */
	}

	/* Update sequential pointer */
	fch->ptr = ptr;

	return 0;
}

#endif /* USE_FILECACHE */


/*
 | This section deals with checking that the .raw files are up to date
 | wrt to the .txt files.
 |
 | For this to work, the equivalent function (in init2.c) needs to be
 | #if-d out (and this function should be declared).  You'll probably
 | also need to zap the UNIX #includes at the top of the file
 */

extern errr check_modification_date(int fd, cptr template_file)
{
	char raw_buf[1024];
	char txt_buf[1024];
	int i;
	os_error *e;

	/* Use OS_Args 7 to find out the pathname 'fd' refers to */
	e = SWI(6, 0, SWI_OS_Args,
			/* In: */
			7,					/* Get path from filehandle */
			fd,					/* file handle */
			raw_buf,			/* buffer */
			0, 0,				/* unused */
			1024				/* size of buffer */
			/* No output regs used */
		);
	if (e)
	{
		core(e->errmess);
	}

	/* Build the path to the template_file */
	path_build(txt_buf, sizeof(txt_buf), ANGBAND_DIR_EDIT, template_file);

	i = file_is_newer(riscosify_name(txt_buf), raw_buf);

	return i;
}


/* Alarm functions */
static int alarm_ackd = 0;	/* has the alarm been acknowledged? */
static window_handle aw = 0;	/* alarm window */

/*
 | Is the alarm due to go off, ie. is it enabled, and if so
 | does the current time match the alarm time?
 */
static void check_alarm()
{
	time_t t;
	struct tm *lt;

	alarm_lastcheck = Time_Monotonic();

	time(&t);
	lt = localtime(&t);
	if (lt->tm_hour == alarm_h && lt->tm_min == alarm_m)
	{
		if (!alarm_ackd) alarm_disp = 1;
	}
	else
	{
		alarm_ackd = 0;
	}

	/* Hack: if the alarm has already been acknowledged then don't re-trigger it */
	if (alarm_ackd)
	{
		alarm_disp = 0;
	}

	/* Hack: if the alarm should make a noise, then make one: */
	if (alarm_disp && alarm_beep == 1)
	{
		static unsigned int last_beep = 0;
		unsigned int t = Time_Monotonic();
		if (t > last_beep + 100)
		{
			Sound_SysBeep();
			last_beep = t;
		}
	}

	/*
	   | If we're in the desktop then fire the alarm off if need be.
	   | If we aren't then do nothing - the fullscreen bored() function
	   | will take care of the alarm.
	 */
#ifndef FULLSCREEN_ONLY
	if (!fullscreen_font && alarm_disp)
		trigger_alarm_desktop();
#endif /* FULLSCREEN_ONLY */
}


static void ack_alarm(void)
{
	if (aw)
	{
		Window_Delete(aw);
		aw = 0;
	}
	alarm_ackd = 1;
	alarm_disp = 0;

	if (alarm_type == 3)
	{
		/* One shot alarm */
		alarm_type = 0;
		write_alarm_choices();
	}

}

#ifndef FULLSCREEN_ONLY
/*
 | Click in the (desktop) alarm window
 */
static BOOL Hnd_AlarmClick(event_pollblock * pb, void *ref)
{
	ack_alarm();
	return TRUE;
}


/*
 | The alarm has gone off in the desktop
 */
static void trigger_alarm_desktop(void)
{
	char buffer[120];
	if (aw)
		return;

	aw = Window_Create("alarm", template_TITLEMIN);
	if (!aw) core("failed to create Alarm window!");

	sprintf(buffer, "Alarm from %s", VARIANT);
	Window_SetTitle(aw, buffer);
	Event_Claim(event_CLICK, aw, 0, Hnd_AlarmClick, NULL);
	Event_Claim(event_CLOSE, aw, event_ANY, Hnd_AlarmClick, NULL);

	Icon_printf(aw, 1, "An alarm was set for %02d:%02d", alarm_h, alarm_m);
	Icon_SetText(aw, 2, alarm_message);
	Window_Show(aw, open_CENTERED);
}

#endif /* FULLSCREEN_ONLY */


/*--------------------------------------------------------------------------*/

#ifndef FE_DEBUG_INFO
static void show_debug_info(void)
{
	core("main-acn internal logic error 004");
}
#else

static int debug_cx = 0;
static int debug_cy = 0;
static int debug_cl = COL(WHITE);
static int debug_sl = 0;


static void debug_cls(void)
{
	Term_clear();
	debug_cx = debug_cy = debug_sl = 0;
}

static void debug_tcol(int c)
{
	debug_cl = c;
}


static void debug_scroll(void)
{
	char **c = ((term_data *)Term)->t.scr->c;	/* char array */
	byte **a = ((term_data *)Term)->t.scr->a;	/* attr array */
	int cc;
	char tmp[82];
	int y, x, p;

	cc = a[1][0];

	for (y = 1; y < 23; y++)
	{
		for (x = p = 0; x < 80; x++)
		{
			if (a[y][x] != cc)
			{
				tmp[p] = 0;
				Term_putstr(x - p, y - 1, p, cc, tmp);
				p = 0;
				cc = a[y][x];
			}
			tmp[p++] = c[y][x];
		}
		Term_putstr(x - p, y - 1, p, cc, tmp);
	}

	Term_erase(0, 22, 80);
}


static void debug_print_line(char *l)
{
	char *le;
	int cr = 0;

	/* Handle scrolling */
	if (debug_cy > 22)
	{
		debug_cy = 22;
		if (--debug_sl < 0)
		{
			int k;
			PUT_FSTR(0, 23, COL(YELLOW), "[RET one line, SPC one page]");
			do
			{
				k = inkey();
			}
			while (k != 32 && k != 13);
			Term_erase(0, 23, 79);
			debug_sl = k == 32 ? 21 : 0;
		}
		debug_scroll();
	}

	/* Hack: check for NL */
	for (le = l; *le; le++)
		if (*le == '\n')
		{
			cr = 1;
			break;
		}

	/* display text */
	Term_putstr(debug_cx, debug_cy, le - l, debug_cl, l);

	/* move cursor */
	if (!cr)
	{
		debug_cx += (le - l);
		if (debug_cx >= 80)
		{
			cr = 1;
		}
	}
	if (cr)
	{
		debug_cx = 0;
		debug_cy += 1;
	}

	Term_gotoxy(debug_cx, debug_cy);

}


static int debug_next_line(char *lb, char **t, int cx)
{
	int i = 0;
	char *lt = *t;

	if (!*lt)
	{
		return -1;
	}							/* Out of text */

	while (*lt && cx < 80)
	{
		lb[i] = *lt++;

		if (lb[i] == '\n')		/* New line */
		{
			cx = 0;				/* Cursor x will be 0 after displaying */
			i++;				/* Keep the \n in the output */
			break;				/* All done */
		}
		else if (lb[i] == '\t')	/* Tab */
		{
			while (cx < 80)
			{
				lb[i++] = ' ';
				cx++;
				if ((cx & 7) == 0)
				{
					break;
				}
			}
		}
		else					/* Anything else */
		{
			cx++;
			i++;
		}
	}

	lb[i] = 0;					/* terminate line buffer */
	*t = lt;					/* update text pointer */
	return cx;					/* return cursor x after printing */
}

static void debug_printf(char *fmt, ...)
{
	char buffer[1024];
	char line[82];
	va_list ap;
	char *p = buffer;

	va_start(ap, fmt);
	vstrnfmt(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	/* Now split the string into display lines */
	while (debug_next_line(line, &p, debug_cx) >= 0)
		debug_print_line(line);
}


static void debug_version_info(void)
{
	debug_tcol(COL(YELLOW));

	debug_printf("\n\nMisc. Info:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\tVariant name = \"%s\"\n", VARIANT);
	debug_printf("\tFront-end version: %s\n", PORTVERSION);
	debug_printf("\tFront-end compiled: %s %s\n", __TIME__, __DATE__);
	debug_printf("\tCompile time flags:\n");

#ifdef USE_FILECACHE
	debug_printf("\t\tUSE_FILECACHE\n");
#endif

#ifdef ABBR_FILECACHE
	debug_printf("\t\tABBR_FILECACHE\n");
#endif

#ifdef SMART_FILECACHE
	debug_printf("\t\tSMART_FILECACHE\n");
#endif

	debug_tcol(COL(YELLOW));
	debug_printf("\nResource path:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\t\"%s\"\n", resource_path);

	debug_tcol(COL(YELLOW));
	debug_printf("\nTempfile path:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\t\"%s\"\n", scrap_path);
	debug_printf("\tScrapfiles are %s deleted at exit.\n",
				 (flush_scrap ? "" : "NOT"));

	debug_tcol(COL(YELLOW));
	debug_printf("\nChoices files:\n");
	debug_tcol(COL(L_BLUE));
	debug_printf("\tDesired files:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\tPrimary (r/w): \"%s\"\n", choices_file[CHFILE_WRITE]);
	debug_printf("\t Fallback (r): \"%s\"\n", choices_file[CHFILE_READ]);
	debug_printf("\t Mirror (r/w): \"%s\"\n", choices_file[CHFILE_MIRROR]);
	debug_tcol(COL(L_BLUE));
	debug_printf("\tActual files:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\t		Write: \"%s\"\n", find_choices(TRUE));
	debug_printf("\t		 Read: \"%s\"\n", find_choices(FALSE));

	debug_tcol(COL(YELLOW));
	debug_printf("\nAlarm files:\n");
	debug_tcol(COL(L_BLUE));
	debug_printf("\tDesired files:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\tPrimary (r/w): \"%s\"\n", alarm_file[CHFILE_WRITE]);
	debug_printf("\t Fallback (r): \"%s\"\n", alarm_file[CHFILE_READ]);
	debug_tcol(COL(L_BLUE));
	debug_printf("\tActual files:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\t		Write: \"%s\"\n", find_alarmfile(TRUE));
	debug_printf("\t		 Read: \"%s\"\n", find_alarmfile(FALSE));
#ifdef USE_DA
	debug_tcol(COL(YELLOW));
	debug_printf("\nDynamic areas:\n");
	debug_tcol(COL(WHITE));
	debug_printf("\tFontcache DA = %d\t", font_area);
	debug_printf("size = %d\theap size = %d\n", font_area_size, font_heap_size);
	debug_printf("\t   ralloc DA = %d\t", game_area);
	debug_printf("size = %d\theap size = %d\n", game_area_size, game_heap_size);
#endif
}


static void debug_filecache_info(void)
{
#ifndef USE_FILECACHE
	debug_tcol(COL(L_DARK));
	debug_printf("File cache disabled at compile time.\n");
#else
	int j, k;
	int t, cs, ucs, cf;

	cf = cs = ucs = j = k = 0;	/* To stop an usused warning if USE_FILECACHE is undefined */
	t = strlen(resource_path);

	if (!file_cache_initd)
	{
		init_file_cache();
	}							/* Paranoia */
	debug_tcol(COL(YELLOW));
	debug_printf("\nFilecache contents:\n");
	debug_tcol(COL(L_BLUE));
	debug_printf("Flags: Smart=%d;  Abbrv=%d;  Slave=%d;   Enable=%d\n",
				 smart_filecache, abbr_filecache, abbr_tmpfile, use_filecache);
	debug_tcol(COL(SLATE));
	if (smart_filecache || abbr_filecache)
		debug_printf("\t\t%3s  %6s/%-6s  %6s  %6s  Path (relative to lib/)\n",
					 "Hnd", "Cache", "Disc", "Time", "Status");
	else
		debug_printf("\t\t%3s  %6s  %6s  %6s  Path (relative to lib/)\n", "Hnd",
					 "Size", "Time", "Status");

	for (j = 0; j < MAX_CACHE_ENTRIES; j++)
	{
		FileCacheEntry *fce = &(file_cache[j]);
		if (fce->name)
		{
			cf++;
			debug_tcol(COL(L_GREEN));
			debug_printf("\t\t%3d  ", j);
			debug_tcol(COL(L_UMBER));
			if (!smart_filecache && !abbr_filecache)
				debug_printf("%6d  ", fce->eof - fce->text);
			else
			{
				debug_printf("%6d/", fce->eof - fce->text);
				k = File_Size(riscosify_name(fce->name));
				debug_printf("%-6d  ", k);
				if (k > 0)
				{
					ucs += k;
				}
			}
			cs += fce->eof - fce->text;
			debug_printf("%6d  ", fce->used);
			for (k = 0; k < MAX_OPEN_CACHED_FILES; k++)
				if (cached_file_handle[k].fce == fce)
					break;
			debug_tcol(COL(RED));
			debug_printf("%-6s  ", k < MAX_OPEN_CACHED_FILES ? "Open" : "");
			debug_tcol(COL(L_UMBER));
			debug_printf("%s\n", fce->name + t);
		}
	}

	debug_tcol(COL(L_BLUE));
	debug_printf("\tTotal:\t%3d  ", cf);
	if (ucs)
		debug_printf("%6d/%-6d\n", cs, ucs);
	else
		debug_printf("%6d\n", cs);
	debug_tcol(COL(BLUE));
#endif /* USE_FILECACHE */
}


static void show_debug_info(void)
{
	int k;
	/* blank the term */
	debug_cls();

	/* Repeatedly prompt for a command */
	do
	{
		debug_tcol(COL(VIOLET));
		debug_printf("\nInfo: (V)ersion, (F)ilecache, ESC=exit ");
		do
		{
			k = inkey();
			switch (k)
			{
				case 'v':  case 'V': debug_version_info();
					break;
				case 'f':  case 'F': debug_filecache_info
						();
					break;
				case 27: break;
				default: k = 0;
			}
		}
		while (!k);
	}
	while (k != 27);

	Term_clear();
}

#endif /* FE_DEBUG_INFO */

#endif /* __riscos */



