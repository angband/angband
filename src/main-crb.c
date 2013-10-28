/*
 * File: main-crb.c
 * Purpose: Provide support for OS X, version 10.3 and later.
 *
 * Copyright (c) 2003 pelpel
 * Copyright (c) 2003,2004,2005 Robert Ruehlmann
 * Copyright (c) 2007,2008 pete mack
 * Copyright (c) 2008 Rowan Beentje
 * Copyright (c) 2010 Andi Sidwell
 *
 * Some excerpts quoted under fair use from works by Ben Harrison,
 * Keith Randall, Peter Ammon, and Ron Anderson.
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
#include "files.h"
#include "init.h"

/*
 * Notes:
 *
 * (pelpel) Characters in the ASCII mode are clipped by their bounding
 * rects to reduce redraw artefacts that were quite annoying in certain
 * font-point combos.
 *
 *
 * (pete mack) Nearly all quickdraw removed to improve performance from
 * interleaved Quickdraw and CGContext events.  (Hence the really ugly
 * ATSU font metrics.)
 * IMPORTANT: If you need to use QuickDraw, make sure to call hibernate()
 * and SetPort() first.  This will keep the graphics state consistent.
 *
 * (pete mack) If you wish to add additional heirarchical menus, the following
 * must hold:
 *   All menus and submenus must have unique ids.
 *   All menus must have IDs greater than 0 and less than MAX_MENU_ID
 *   All submenus must have greater IDs than their parents.
 *
 *
 * Important Resources in the resource file:
 *
 *   FREF 130 = ANGBAND_CREATOR / 'APPL' (application)
 *   FREF 129 = ANGBAND_CREATOR / 'SAVE' (save file)
 *   FREF 130 = ANGBAND_CREATOR / 'TEXT' (bone file, generic text file)
 *   FREF 131 = ANGBAND_CREATOR / 'DATA' (binary image file, score file)
 *
 */

/* -------------------------------------------------------
 * PORTING YOUR VARIANT
 * ------------------------------------------------------
 * 0. Requires installation of XCode.  The framework .h files are otherwise
 * absent. New changes also require use of NIB (xml) files, which
 * are most easily edited with Interface Builder. (XCode package.)
 * This reduces the amount of hand-coded menu creation by an order of magnitude.
 *
 *
 * 1. Compiling the binary
 *
 * You might wish to disable some SET_UID features for various reasons:
 * to have user folder within the lib folder, savefile names etc.
 *
 * 2. Installation
 *
 * The "angband" binary must be arranged this way for it to work:
 *
 * lib/ <- the lib folder
 * Angband (OS X).app/
 *   Contents/
 *     MacOS/
 *       angband <- the binary you've just compiled
 *     Info.plist <- to be explained below
 *     Resources/
 *       Angband.icns
 *       Data.icns
 *       Edit.icns
 *       Save.icns
 *       8x8.png <- 8x8 tiles
 *       16x16.png <- 16x16 tiles
 *       angband.rsrc <- see below
 *
 * Graphics resources are moved out of the resource fork and become ordinary
 * PNG files.  Make sure to set its resolution to 72 dpi (<- VERY important)
 * while keeping vertical and horizontal scaling factor to 100% (<- VERY
 * important), when you convert tiles in any formats to PNG.  This means
 * that the real size of an image must shrink or grow when you change it's dpi.
 * Graphics files must be of type png, because OSX 10.3 doesn't do
 * a good job with composited images.
 *
 * Transparency now uses the alpha channel, rather than a background color.
 *
 * ---(end of OS X + gcc porting note)--------------------------------------
 *
 * Because the default font-size combination causes redraw artefact problem
 * (some characters, even in monospace fonts, have negative left bearings),
 * I [pelpel] introduced rather crude hack to clip all character drawings within
 * their bounding rects. If you don't like this, please comment out the line
 * #define CLIP_HACK
 * below. The alternative, #define OVERWRITE_HACK, is based on Julian Lighton's
 * brilliant suggestion, but it doesn't work as expected. This is because
 * DrawText can render the same character with _different_ pixel width,
 * depending on relative position of a character to the pen. Fonts do look
 * very nice on the Mac, but too nice I'd say, in case of Angband.
 *
 * The check for return values of AEProcessAppleEvent is intentionally ignored.
 *
 * Changes 2006 (pete mack)
 * - Removed conditional code. Removed non-quicktime sound.
 * - Reorganized event model to use Carbon events and dispatching.
 * - Removed old font menus & calculations; replaced with Apple font panel.
 * - Removed Quickdraw graphics; replaced with CGContext (Quartz) graphics.
 * - Removed Quickdraw text; replaced with CGFont. ATSUI is used for
 *   finding fonts, font metrics, and Glyph information for fast rendering
 *   of variable-width fonts.
 */


#ifdef MACH_O_CARBON


#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>

#include "osx/osx_tables.h"



/* Default creator signature */
#ifndef huge
#define huge size_t
#endif

/*
 * Width in pixels of white borders around the black screen.
 */
#define BORDER_WID 1


static const bool show_events = 0;

/*
 * A rather crude fix to reduce amount of redraw artifacts.
 * Some fixed width fonts (i.e. Monaco) has characters with negative
 * left bearings, so Term_wipe_mac or overwriting cannot completely
 * erase them.
 */

#define CLIP_HACK 0 /* */
static const int use_clip_hack = CLIP_HACK;

/*
 * Minimum font size
 */
#define MIN_FONT 8

/*
 * Another redraw artifact killer, based on suggestion by Julian Lighton
 */
#define OVERWRITE_HACK (0)

/* 
 * These hacks should never be enabled at the same time.
 * Clip-hack renders overwrite-hack meaningless, and
 * it will cause use of an unitialized variable. - pete mack.
 */

static const int use_overwrite_hack = (OVERWRITE_HACK && !CLIP_HACK);

/*
 * Maximum number of windows.
 */
#define MAX_TERM_DATA 8


/* graphics_modes index of current graphics mode */
static UInt32 graf_mode = 0;

/* Tile dimensions of the current graphics mode */
static int graf_height = 0;
static int graf_width = 0;

/*
 * Use antialiasing.  Without image differencing from
 * OSX  10.4 features, you won't want to use this.
 */
static bool antialias = false;

typedef struct GlyphInfo GlyphInfo;

typedef struct term_data term_data;

/*
 * Extra "term" data
 */
struct term_data
{
	term *t;
	WindowRef w;
	GlyphInfo *ginfo;
	
	Rect wr;        /* Absolute bounds of window (for save/restore) */
	Rect r;            /* Canvas bounds of window (for mouse addressing &c)  */
	CGRect bounds;  /* Relative bounds of border-clipped canvas. */

	int spacing;      /* Text padding (in pixels) for tiling wider than text */

	char title[255];    /* Window title. */

	s16b mapped;    /* Active state. */

	s16b rows;        /* rows in picture */
	s16b cols;        /* columns in picture. */

	char font_name[200]; /* Name of font for storage. */
	ATSUFontID font_id;
	float font_size;    /* Scaled ATSU font size. */

	u16b font_wid;
	u16b font_hgt;

	u16b tile_wid;
	u16b tile_hgt;

	s16b size_wid;    /* Window size in x. */
	s16b size_hgt;    /* Window size in y. */
};

struct GlyphInfo
{
	UInt32 refcount;
	char psname[1000];
	ATSUFontID font_id;
	CGFontRef fontRef;
	float font_size;
	ATSUStyle style;
	ATSUTextLayout layout;
	float font_wid;  /* max character advance. */
	s32b ascent;
	s32b descent;
	bool monospace;
	float offsets[256][3];
	float heights[256][3];
	float widths[256];
};

static GlyphInfo glyph_data[MAX_TERM_DATA+1];

static WindowRef aboutDialog;


static bool CheckEvents(int wait);
static OSStatus RevalidateGraphics(term_data *td, bool reset_tilesize);
static char *locate_lib(char *buf, size_t size);
static void graphics_aux(UInt32 op);
static void Term_wipe_mac_aux(int x, int y, int n);
inline static void term_data_color(int a);
static void install_handlers(WindowRef w);
static void graphics_tiles_nuke(void);
static void play_sound(int num);
static void redrawRecentItemsMenu();
/*
 * Available values for 'wait'
 */

#define CHECK_EVENTS_DRAIN -1
#define CHECK_EVENTS_NO_WAIT    0
#define CHECK_EVENTS_WAIT 1


/*
 * Delay handling of double-clicked savefiles
 */
bool open_when_ready = FALSE;

/*
 * Delay handling of pre-emptive "quit" event
 */
bool quit_when_ready = FALSE;

static long mac_os_version;


/* Out-of-band color identifiers */
/* True black (TERM_BLACK may be altered) */
#define COLOR_BLACK        (256)
/* No current color */
#define COLOR_INVALID    (-1)


/*
 * Keeps track of who owns the QD CGContext, and it's current state.
 * Always use this to change the active graphics port.
 * (It is a parallel structure to the Term variable.)
 */
struct ActivePort {
	WindowRef        active;
	CGContextRef    ctx;
	int                  color;   /* Current fill colorcode */
	/* CGColorRef        color_info[256+1]; */
	float color_info[256+1][3];
}; 

static struct ActivePort focus; /* initialized to 0 */


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];


/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;

/*
 * A mutable array for Recent Items
 */ 
CFMutableArrayRef recentItemsArrayRef = NULL;

/*
 * Support the improved game command handling
 */
#include "textui.h"
static game_command cmd = { CMD_NULL, 0 };


static MenuRef MyGetMenuHandle_aux(int menuID, bool first)
{
	static MenuRef menuRefs[MAX_MENU_ID];

	if(menuID <= 0 || menuID >= MAX_MENU_ID) return 0;

	if(menuRefs[menuID]) return menuRefs[menuID];
	MenuRef m = GetMenuHandle(menuID);
	if(m) {
		menuRefs[menuID] = m;
		return m;
	}

	/*
	 * First heirarchical call, find and initialize all menu IDs.
	 * Subsequent misses will attempt to update the menuRefs array.
	 * This will work for any depth heirarchy, so long as child menus have
	 * higher IDs than their parents.    
	 *
	 * Invariant: all MenuRefs with ID < MenuID(tmp) have been initialized.
	 */
	for(int id = 1; id < MAX_MENU_ID; id++)
	{
		/* Recursion depth is 1 */
		if(first) m = MyGetMenuHandle_aux(id, false);
		if(!m) continue;
		menuRefs[id] = m;
		for(size_t i = 0; i < N_ELEMENTS(menuRefs); i++) {
			MenuRef tmp = 0;
			GetMenuItemHierarchicalMenu (m, i, &tmp);
			if(tmp) {
				menuRefs[GetMenuID(tmp)] = tmp;
			}
		}
	}
	return menuRefs[menuID];
}

/*
 * Provide a flat namespace for OS X menus and submenus. 
 * It's a nuisance doing this via heirarchical calls all the time.
 */
inline static MenuRef MyGetMenuHandle(int menuID)
{
	return MyGetMenuHandle_aux(menuID, true);
}


/*
 * Convert a pathname to a corresponding FSSpec.
 * Returns noErr on success.
 */
static OSErr path_to_spec(const char *path, FSSpec *spec)
{
	OSErr err;
	FSRef ref;

	/* Convert pathname to FSRef ... */
	err = FSPathMakeRef((byte*) path, &ref, NULL);
	if (err != noErr) return (err);

	/* ... then FSRef to FSSpec */
	err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);
	
	/* Inform caller of success or failure */
	return (err);
}


/*
 * Convert a FSSpec to a corresponding pathname.
 * Returns noErr on success.
 */
static OSErr spec_to_path(const FSSpec *spec, char *buf, size_t size)
{
	OSErr err;
	FSRef ref;

	/* Convert FSSpec to FSRef ... */
	err = FSpMakeFSRef(spec, &ref);
	if (err != noErr) return (err);

	/* ... then FSRef to pathname */
	err = FSRefMakePath(&ref, (byte*)buf, size);

	/* Inform caller of success or failure */
	return (err);
}


/*
 * [via path_to_spec]
 * Set creator and filetype of a file specified by POSIX-style pathname.
 * Returns 0 on success, -1 in case of errors.
 */
static void fsetfileinfo(const char *pathname, u32b fcreator, u32b ftype)
{
	OSErr err;
	FSSpec spec;
	FInfo info;

	/* Convert pathname to FSSpec */
	if (path_to_spec(pathname, &spec) != noErr) return;

	/* Obtain current finder info of the file */
	if (FSpGetFInfo(&spec, &info) != noErr) return;

	/* Overwrite creator and type */
	info.fdCreator = fcreator;
	info.fdType = ftype;
	err = FSpSetFInfo(&spec, &info);

	/* Done */
	return;
}

static void osx_file_open_hook(const char *path, file_type ftype)
{
	u32b mac_type = 'TEXT';
		
	if (ftype == FTYPE_RAW)
		mac_type = 'DATA';
	else if (ftype == FTYPE_SAVE)
		mac_type = 'SAVE';
		
	fsetfileinfo(path, 'A271', mac_type);
}



/*
 * Activate a given window, if necessary.  This should normally 
 * be called by Term_activate, when the z-term is updating it's
 * state.  It should also be called prior to any updates when
 * the window CGContext has been closed by hibernate()
 */
static void activate(WindowRef w)
{
	/* Activate */

	if(focus.active && focus.ctx) {
		CGContextSynchronize(focus.ctx);
		if(focus.active != w) {
			/* Change window context */
			QDEndCGContext(GetWindowPort(focus.active), &focus.ctx);
			focus.ctx = 0;
		}
	}
	/* Activate */
	if (w && focus.ctx == 0) {
		focus.color = COLOR_INVALID;

		term_data *td = (term_data*) GetWRefCon(w);

		/* Start queueing graphics events. */
		/* and set up the context */
		QDBeginCGContext(GetWindowPort(w), &focus.ctx);

		/* Shift the origin to inside the border, and use inverted y axis. */
		CGAffineTransform m;
		m = CGAffineTransformMake(BORDER_WID, 0, 0, -1,
								 BORDER_WID, BORDER_WID+td->bounds.size.height);

		CGContextConcatCTM (focus.ctx, m); 
		CGContextClipToRect(focus.ctx, td->bounds);

		CGContextSetShouldAntialias (focus.ctx, antialias); 
		CGContextSetInterpolationQuality(focus.ctx, kCGInterpolationNone);

		/* Invert the text so it's no longer mirrored in y. */
		/* Origin is at still at the bottom of the line, so the ascent must */
		/* be subtracted at display-time. (Not added, because the coordinate */
		/* system is also flipped.) */
		m = CGAffineTransformMake(1, 0, 0, -1, 0, 0);
		CGContextSetTextMatrix(focus.ctx, m);

		CFStringRef font_name = CFStringCreateWithCString(
			kCFAllocatorDefault, td->ginfo->psname, 
			kCFStringEncodingISOLatin1);
		td->ginfo->fontRef = CGFontCreateWithFontName(font_name);
		CGContextSetFont(focus.ctx, td->ginfo->fontRef);
		CGContextSetFontSize(focus.ctx, td->font_size);
		CFRelease(font_name);

		if(td->ginfo->monospace) {
			CGContextSetCharacterSpacing(focus.ctx,
											td->tile_wid - td->ginfo->font_wid);
		}
		else {
			const ATSUAttributeTag itags[] =  { kATSUCGContextTag,
										kATSUImposeWidthTag };
			Fixed advance = (1<<16)*(td->tile_wid - td->font_wid);
			void *ivals[] = { &focus.ctx, &advance };
			const ByteCount isizes[] = {sizeof(focus.ctx), sizeof(advance) };
			ATSUSetAttributes(td->ginfo->style, 2, itags, isizes, ivals);
		}
	}

	/* Remember */
	focus.active = w;
}

/* 
 * Temporarily give up control of the Quickdraw port.
 * Call when the window becomes inactive.
 * Call when the graphics state changes in any way, to
 * invalidate the current CGContext and force an update.
 */
static void hibernate()
{
	if(focus.ctx != 0)
		QDEndCGContext(GetWindowPort(focus.active), &focus.ctx);

	focus.ctx = 0;
}

/*
 * Display a warning message
 */
static void mac_warning(const char *warning)
{
	CFStringRef msg;
	msg = CFStringCreateWithCString(NULL, warning, kCFStringEncodingISOLatin1);

	DialogRef dlg = 0;
	CreateStandardAlert(kAlertCautionAlert, msg, CFSTR(""), NULL, &dlg);

	DialogItemIndex itemIndex;
	RunStandardAlert(dlg, NULL, &itemIndex);
	/* DisposeDialog(dlg); // NO! */
	CFRelease(msg);
}

/*
 * Notice fully up-to-date status of the main window
 */
static void validate_main_window(void)
{
	WindowRef w;
	Rect r;

	/* Get the main window */
	w = data[0].w;

	/* Get its rectangle */
	GetPortBounds(GetWindowPort(w), &r);

	/* Validate it */
	ValidWindowRect(w, &r);
}


/*** Some generic functions ***/

/*
 * Update color_info with the current values in angband_color_table
 */
static void update_color_info(void)
{
	focus.color = COLOR_INVALID;
	focus.color_info[COLOR_BLACK][0] = 0;
	focus.color_info[COLOR_BLACK][1] = 0;
	focus.color_info[COLOR_BLACK][2] = 0;

	/* Update colors */
	for (int i = 0; i < 256; i++)
	{
		float r = angband_color_table[i][1];
		float g = angband_color_table[i][2];
		float b = angband_color_table[i][3];
		r = r == 255 ? 1 : r / 256;
		g = g == 255 ? 1 : g / 256;
		b = b == 255 ? 1 : b / 256;

		focus.color_info[i][0] = r;
		focus.color_info[i][1] = g;
		focus.color_info[i][2] = b;
	}
}

/*
 * Activate a color (0 to 256)
 * -1 is invalid, 256 is true black.
 */
inline static void term_data_color(int a)
{
	if(!focus.ctx) activate(focus.active);
	if( a == COLOR_INVALID) {
		focus.color = a;
	}
	else if (focus.color != a)  /* Assumes the window exists. */
	{
		focus.color = a;
		/* CGContextSetFillColorWithColor(focus.ctx, focus.color_info[a]); */
		CGContextSetRGBFillColor(focus.ctx, focus.color_info[a][0],
							focus.color_info[a][1], focus.color_info[a][2], 1);
	}
}

/*
 * Get font metrics 
 */
static GlyphInfo *get_glyph_info(ATSUFontID fid, float size)
{
	GlyphInfo *info;

	/* One extra, so a term_data can fetch before it frees. */
			
	for(info = glyph_data; info <= glyph_data+MAX_TERM_DATA; info++)
	{
		if(info->font_id == fid && info->font_size == size)
		{
			info->refcount++;
			return info;
		}
	}

	/* One is always available. */
	info = glyph_data;
	for (int c = 0; info->refcount != 0; info++, c++)
		assert(c <= MAX_TERM_DATA);

	info->style = 0;
	info->layout = 0;

	/* ICK */

	info->font_size = size;
	info->font_id = fid;

	OSStatus err = ATSUCreateStyle(&info->style);
	require_noerr(err, CantInitialize);

	Fixed fsize = (Fixed)(size*(1<<16));
	const ByteCount isizes[] = {sizeof(fid), sizeof(fsize) };
	void *ivals[] = { &fid, &fsize };

	const ATSUAttributeTag itags[] =  { kATSUFontTag, 
										kATSUSizeTag };

	ATSUSetAttributes(info->style, 2, itags, isizes, ivals);
	err = ATSUCreateTextLayout(&info->layout);
	require_noerr(err, CantInitialize);

	/* Dummy text, required to initialize run style. */
	UniChar text[1] = {'@'};
	ATSUSetTextPointerLocation(info->layout, text, 0, 1, 1);
	ATSUSetRunStyle(info->layout, info->style, 0, 1);

	ByteCount oCount = 0;
	FontNameCode oCode;
	err = ATSUGetIndFontName(fid, 6, sizeof(info->psname), info->psname,
													&oCount, &oCode, 0, 0, 0);

	require_noerr(err, CantInitialize);
	if(oCount == 0 || strlen(info->psname) == 0)
		goto CantInitialize;

	info->psname[oCount] = 0;
	/* Is font mono-space? */
	err = ATSUCreateTextLayout(&info->layout);
	require_noerr(err, CantInitialize);

	ATSUSetRunStyle(info->layout, info->style, 0, 0);
	info->monospace = true;
	info->font_wid = 0;
	info->ascent = 0;
	info->descent = 0;

	for(int i = 0; i < 256; i++) {
		OSStatus err;
		text[0] = i;
		Fixed start = 0, stop = 0, ascent = 0, descent = 0;
		ATSUSetTextPointerLocation(info->layout, text, 0, 1, 1);
		/* SetRunStyle doesn't honor runs in layouts of size 0. */
		if(i == 0) ATSUSetRunStyle(info->layout, info->style, 0, 1);
		err = ATSUGetUnjustifiedBounds(info->layout, 0, 1, &start, &stop,
														&ascent, &descent);

		if(info->ascent < ascent) info->ascent = ascent;
		if(info->descent < descent) info->descent = descent;


		info->widths[i] = (stop - start)/(1<<16);

		if(info->font_wid == 0) info->font_wid = stop - start;
		else if((info->font_wid != stop - start) && (stop - start != 0)) {
			info->monospace = false;
		}
		if(info->font_wid < stop - start) {
			info->font_wid = stop - start;
		}
	}

	/* Scale the rough font metrics to screen dimensions. */
	/* Assumes 72dpi resolution. */
	info->ascent = (info->ascent + (1<<16)-1)>>16;
	info->descent = (info->descent + (1<<16)-1)>>16;
	info->font_wid = (info->font_wid)/(1<<16);

	info->refcount++;
	return info;

CantInitialize:
	if(info->layout) ATSUDisposeTextLayout(info->layout);
	if(info->style) ATSUDisposeStyle(info->style);
	info->style = 0;
	info->layout = 0;
	return 0;
}


static void release_glyph_info(GlyphInfo *info)
{
	if(!info) return;
	assert(info->refcount > 0);

	info->refcount--;
	if(0 == info->refcount) {
		ATSUDisposeStyle(info->style);
		info->style = 0;
		CGFontRelease(info->fontRef);
		info->fontRef = 0;
	}
}

/*
 * Hack -- Apply and Verify the "font" info
 *
 * This should usually be followed by "term_data_check_size()"
 *
 * XXX XXX To force (re)initialisation of td->tile_wid and td->tile_hgt
 * you have to reset them to zero before this function is called.
 * XXX XXX This is automatic when the program starts because the term_data
 * array is WIPE'd by term_data_hack, but isn't in the other cases, i.e.
 * font, font style and size changes.
 */
static void term_data_check_font(term_data *td)
{
	GlyphInfo *info = get_glyph_info(td->font_id, td->font_size);
	if(!info) return;

	release_glyph_info(td->ginfo);
	td->ginfo = info;

	td->font_wid = (info->font_wid +.999);
	td->font_hgt = info->ascent + info->descent;
	my_strcpy(td->font_name, info->psname, sizeof(td->font_name));

	/* Set default tile size */
	if (td->tile_wid == 0) td->tile_wid = td->font_wid;
	if (td->tile_hgt == 0) td->tile_hgt = td->font_hgt;
}


/*
 * Hack -- Apply and Verify the "size" info
 */
static void term_data_check_size(term_data *td)
{
	if (td == &data[0])
	{

		/* Enforce minimal size for the main game window */
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;

	}
	else
	{

		/* Information windows can be much smaller */
		if (td->cols < 10) td->cols = 10;
		if (td->rows < 5) td->rows = 5;
	}

	/* Enforce maximal sizes */
	if (td->cols > 255) td->cols = 255;
	if (td->rows > 255) td->rows = 255;

	/* Minimal tile size */
	bool dirty = false;
	if (td->tile_wid < td->font_wid) {
		td->tile_wid = td->font_wid;
		dirty = true;
	}
	if (td->tile_hgt < td->font_hgt) {
		td->tile_hgt = td->font_hgt;
		dirty = true;
	}
	if(dirty) graphics_tiles_nuke();

	/* Calculate full window size */
	td->size_wid = td->cols * td->tile_wid + BORDER_WID * 2;
	td->size_hgt = td->rows * td->tile_hgt + BORDER_WID;


	hibernate(); 
	BitMap tScreen;
	/* Get current screen */
	(void)GetQDGlobalsScreenBits(&tScreen);
	/* Verify the bottom */
	if (td->r.top > tScreen.bounds.bottom - td->size_hgt)
	{
		td->r.top = tScreen.bounds.bottom - td->size_hgt;
	}
	/* Verify the top */
	if (td->r.top < tScreen.bounds.top + 2*GetMBarHeight())
	{
		td->r.top = tScreen.bounds.top + 2*GetMBarHeight();
	}
	/* Verify the right */
	if (td->r.left > tScreen.bounds.right - td->size_wid)
	{
		td->r.left = tScreen.bounds.right - td->size_wid;
	}
	/* Verify the left */
	if (td->r.left < tScreen.bounds.left)
	{
		td->r.left = tScreen.bounds.left;
	}

	/* Calculate bottom right corner */
	td->r.right = td->r.left + td->size_wid;
	td->r.bottom = td->r.top + td->size_hgt;

	td->bounds = (CGRect) {{0, 0},
		{td->cols * td->tile_wid, td->rows * td->tile_hgt}};

	/* Assume no graphics, monospace */
	td->t->higher_pict = FALSE;
	td->t->always_pict = FALSE;

	
	/* Handle graphics */
	if (!td->ginfo->monospace) {
		/* Draw every character */
		td->t->always_pict = TRUE;
	}
	else if (use_graphics)
	{
		/* Use higher pict whenever possible */
		td->t->higher_pict = TRUE;
	}

}


/*
 * resize a term_data
 *
 * This should normally be followed by "term_data_redraw()"
 */
static void term_data_resize(term_data *td)
{
	/* Invalidate the current CGContext. */
	hibernate();
	/*
	 * Actually resize the window
	 * 
	 * ResizeWindow is the preferred API call, but it cannot
	 * be used here.
	 */
	
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);

	/* Cheat a little -- can't use the active view to redraw its own border. */
	CGContextRef tmpCtx;
	QDBeginCGContext(GetWindowPort(td->w), &tmpCtx);
	CGContextSetRGBStrokeColor(tmpCtx, 1, 1, 1, 1);
	CGRect wbounds = {{0, 0}, {td->bounds.size.width+2*BORDER_WID,
								td->bounds.size.height+BORDER_WID+2}};
	CGContextStrokeRect(tmpCtx, wbounds);
	QDEndCGContext(GetWindowPort(td->w), &tmpCtx);
}


/*
 * Hack -- redraw a term_data
 *
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td)
{
	term *old = Term;
	/* Activate the term */
	Term_activate(td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();
	Term_activate(old);
}


/*
 * Graphics support
 */

/*
 * PICT id / file name of image tiles
 */
static const char *pict_id = NULL;

/*
 * Frame
 * Wrapper for CGImages of the current tile set.
 *
 */
static struct
{
	/* The reference image at the original scale. */
	CGImageRef image;

	 /* Numbers of rows and columns in a tileset, */
	int cols;
	int rows;

	/*
	 * Tile images.
	 * These are generated once at each tile scaling.  Since
	 * there are many unused tiles, and since generating tiles takes
	 * a long time, they are generated only on demand.
	 * The tiles at the current scaling.  
	 * Foreground tiles are RGBA
	 * Background tiles are RGB only.
	 * This is detected by DrawTile--background images have the same
	 * tile-id for both foreground and background.
	 * Stride is number of columns.
	 */
	CGImageRef *tile_images;
} frame = {0};

/*
 * Rendevous for font update events.
 */
static struct 
{
	WindowRef focus; /* The most recently focused window. (NOT the fontpanel.) */
} fontInfo;



/* 
 * Replacement for BitMap (from QD2Qz porting guide)
 */
void DrawSubimage (CGContextRef context, CGRect dst,
						CGImageRef image, CGRect src)
{
	/* the default drawing rectangle */
	float w = (float) CGImageGetWidth(image);
	float h = (float) CGImageGetHeight(image);
	CGRect drawRect = {{0, 0}, {w, h}};
	if (!CGRectEqualToRect (src, dst)) 
	{
		float sx = CGRectGetWidth(dst) / CGRectGetWidth(src);
		float sy = CGRectGetHeight(dst) / CGRectGetHeight(src);
		float dx = CGRectGetMinX(dst) - (CGRectGetMinX(src) * sx);
		float dy = CGRectGetMinY(dst) - (CGRectGetMinY(src) * sy);
		drawRect = (CGRect) {{dx, dy}, {w*sx, h*sy}};
	}

	CGContextSaveGState (context);
	CGContextClipToRect (context, dst);
	/* Don't display images upside down. (Works like SetTextMatrix) */
	HIViewDrawCGImage (context, &drawRect, image); 
	/* CGContextDrawImage (context, &drawRect, image); */
	CGContextRestoreGState (context);
}

/*
 * Copy an image with tiles of size src into a new one with
 * tiles of size dst.  Interpolation will not cross tile borders.
 */
static CGImageRef GetTileImage(int row, int col, bool has_alpha) 
{
	/* Cache hit. */
	assert(col < frame.cols && row < frame.rows);
	if (frame.tile_images[row*frame.cols + col] != 0) {
		return frame.tile_images[row*frame.cols+col];
	}

	term_data *td = &data[0];

	size_t tile_wid = td->tile_wid * tile_width;
	size_t tile_hgt = td->tile_hgt * tile_height;
	size_t nbytes = (tile_hgt * tile_wid) * 4;
	void *data = calloc(1, nbytes);

	CGContextRef map;
	map = CGBitmapContextCreate(data, tile_wid, tile_hgt,
								8,
								nbytes / tile_hgt,
								CGColorSpaceCreateDeviceRGB(),
								kCGImageAlphaPremultipliedLast);

	CGAffineTransform m = {1, 0, 0, -1, 0, tile_hgt};
	CGContextConcatCTM(map, m); 

	/* Attempt to avoid interpolation across cell boundaries by clipping. */
	/* It may be that we need a clip image first. */
	CGRect src_r = {{ graf_width*col, graf_height*row },
						{ graf_width, graf_height }};
	CGRect dst_r = {{ 0, 0 }, { tile_wid, tile_hgt }};
	DrawSubimage(map, dst_r, frame.image, src_r);

	CGDataProviderRef prov;
	prov = CGDataProviderCreateWithData (NULL, data, nbytes, NULL);

	CGImageAlphaInfo alphaInfo = kCGImageAlphaPremultipliedLast;
	size_t pixelBits = 4 * 8;
	if(!has_alpha) {
		alphaInfo = kCGImageAlphaNone;
		pixelBits = 3 * 8;
	}

	CGImageRef timg = CGImageCreate(CGBitmapContextGetWidth(map),
									CGBitmapContextGetHeight(map),
									CGBitmapContextGetBitsPerComponent(map),
									CGBitmapContextGetBitsPerPixel(map),
									CGBitmapContextGetBytesPerRow(map),
									CGBitmapContextGetColorSpace(map),
									alphaInfo,
									prov, NULL, false,
									kCGRenderingIntentDefault);

	CGDataProviderRelease(prov);
	CGContextRelease(map);
	/* free(data); // Duplicate free? */

	frame.tile_images[row*frame.cols+col] = timg;
	return timg; 
}

static void DrawTile(int x, int y, byte a, byte c, byte ta, byte tc)
{
	term_data *td = (term_data*) Term->data;

	int tile_wid = td->tile_wid * tile_width;
	int tile_hgt = td->tile_hgt * tile_height;
	CGRect dst_r = {{x*td->tile_wid, y*td->tile_hgt}, {tile_wid, tile_hgt}};
	CGRect src_r = {{0, 0}, {tile_wid, tile_hgt}};

	tc = (tc&0x7f) % frame.cols;
	ta = (ta&0x7f) % frame.rows;
	c = (c&0x7f) % frame.cols;
	a = (a&0x7f) % frame.rows;
	
	/* Requires alpha-masked foreground images */
	/* Draw terrain.  No alpha or rescaling, should just be a bitblt. */
	if (!use_transparency) tc = c, ta = a;
	CGImageRef image = GetTileImage(ta, tc, false);
	DrawSubimage(focus.ctx, dst_r, image, src_r);

	/* Draw the foreground, if it is distinct from the background */
	/* Use alpha. Rare, so it shouldn't take much time. */
	if(use_transparency && (tc != c || ta != a)) {
		/* This doesn't preserve alpha. Don't know how to fix. */
		image = GetTileImage(a, c, true);
		DrawSubimage(focus.ctx, dst_r, image, src_r);
		/* Use the original source instead. This is SLOW! */
		/*src_r = (CGRect) {{ graf_width*c, graf_height*a }, */
		/*                            { graf_width, graf_height }}; */
		/* DrawSubimage(focus.ctx, dst_r, frame.image, src_r); */
	}
}

static void ShowTextAt(int x, int y, int color, int n, const char *text )
{
	term_data *td = (term_data*) Term->data;
	GlyphInfo *info = td->ginfo;
	/* Overwite the text, unless it's being called recursively. */
	if(use_overwrite_hack && color != COLOR_BLACK) {
		Term_wipe_mac_aux(x, y, n); 
	}

	int c = *(unsigned char*) text;

	int xp = x * td->tile_wid + (td->tile_wid - (info->widths[c] + td->spacing))/2;
	/* Only round once. */
	int yp = y * td->tile_hgt + info->ascent + (td->tile_hgt - td->font_hgt)/2;

	 CGRect r;
	if(use_graphics || !use_overwrite_hack) {
		r = (CGRect) {{x*td->tile_wid, y*td->tile_hgt},
											{n*td->tile_wid, td->tile_hgt}};
		term_data_color(COLOR_BLACK);
		CGContextFillRect(focus.ctx, r);
	}

	if(use_clip_hack) {
		CGContextSaveGState (focus.ctx);
		CGContextClipToRect(focus.ctx, r);
	}

	term_data_color(color);
	/* Monospace; use preset text spacing when tiling is wider than text */
	if(n == 1 || info->monospace) {
		/* See the Accessing Font Metrics section of Apple's Core Text 
		 * Programming Guide for the sample code that inspired this
		 * block. */
		UniChar *characters;
		CGGlyph *glyphs;
		CTFontRef font = CTFontCreateWithGraphicsFont(
			td->ginfo->fontRef, (CGFloat)td->font_size,
			NULL, NULL);
		CFStringRef text_str = CFStringCreateWithCString(
			kCFAllocatorDefault, text, 
			kCFStringEncodingISOLatin1);

		characters = (UniChar *)mem_alloc(sizeof(UniChar) * n);
		assert(characters != NULL);
		glyphs = (CGGlyph *)mem_alloc(sizeof(CGGlyph) * n);
		assert(glyphs != NULL);

		CFStringGetCharacters(text_str, CFRangeMake(0, n), characters);
		CFRelease(text_str);
		CTFontGetGlyphsForCharacters(font, characters, glyphs, n);
		CGContextShowGlyphsAtPoint(focus.ctx, 
			(CGFloat)xp, (CGFloat)yp, glyphs, n);
		mem_free(characters);
		mem_free(glyphs);
		if(use_clip_hack)
			CGContextRestoreGState(focus.ctx);
		return;
	}

	
	GlyphInfo *gi = td->ginfo;

	UniChar utext[n];
	for(int i = 0; i < n; i++) utext[i] = text[i];
	ATSUSetTextPointerLocation(gi->layout, utext, 0, n, n);
	ATSUSetRunStyle(gi->layout, info->style, 0, n);
	ATSUDrawText(info->layout, 0, n, xp*(1<<16), yp*(1<<16));

	if(use_clip_hack)
		CGContextRestoreGState (focus.ctx);

	return;
}


/*
 * Init the graphics "frame"
 */
static errr graphics_init(void)
{
	OSErr err;

	/* Get the tile resources */
	char path[1024];
	locate_lib(path, sizeof(path));
	char *tail = path + strlen(path);
	FSSpec pict_spec;
	snprintf(tail, path+1024-tail, "xtra/graf/%s.png", pict_id);
	if(noErr != path_to_spec(path, &pict_spec))
		return -1;

	 /* Attempt to create a CGImage from FSSpec using Quicktime importer */
	GraphicsImportComponent gi;
	if( (err = GetGraphicsImporterForFile(&pict_spec, &gi)) )
		return err;

	Rect r;
	/* Retrieve the rect of the image */
	err = GraphicsImportGetNaturalBounds(gi, &r);

	/* Calculate and set numbers of rows and columns */
	frame.rows = (r.bottom-r.top) / graf_height;
	frame.cols = (r.right-r.left) / graf_width;

	frame.tile_images = calloc(frame.rows*frame.cols, sizeof(CGImageRef));
	if(!frame.tile_images) {
		return -1; /* E_NO_MEM */
	}

	CGImageRef tile_img;
	err = GraphicsImportCreateCGImage (gi, &tile_img, 0);
	CloseComponent(gi);

	if (err != noErr) {
		free(frame.tile_images);
		frame.tile_images = 0;
		return (err);
	}

	frame.image = tile_img;

	/* Success */
	return (noErr);
}

static void graphics_tiles_nuke(void)
{
	if(frame.tile_images) {
		for(int i = frame.rows*frame.cols; --i >= 0; ) {
			if(frame.tile_images[i]) CGImageRelease(frame.tile_images[i]);
			frame.tile_images[i] = 0;
		}
	}
}

/*
 * Nuke the graphics "frame" and contents.
 */
static errr graphics_nuke(void)
{
	/* Dispose image */
	if (frame.image)
	{
		/* Dispose of the tile image */
		CGImageRelease(frame.image);
		frame.image = NULL;
	}
	if(frame.tile_images) {
		graphics_tiles_nuke();
		free(frame.tile_images);
		frame.tile_images = 0;
	}

	/* Flush events */
	if (initialized) FlushEventQueue(GetMainEventQueue());

	/* Success */
	return (0);
}


/* Arbitary limit on number of possible samples per event */
#define MAX_SAMPLES            8

/* Struct representing all data for a set of event samples */
typedef struct
{
	int num;        /* Number of available samples for this event */
	NSSound *sound[MAX_SAMPLES];
} sound_sample_list;

/* Array of event sound structs */
static sound_sample_list samples[MSG_MAX];

/*
 * Load sound effects based on sound.cfg within the xtra/sound directory;
 * bridge to Cocoa to use NSSound for simple loading and playback, avoiding
 * I/O latency by cacheing all sounds at the start.  Inherits full sound
 * format support from Quicktime base/plugins.
 * pelpel favoured a plist-based parser for the future but .cfg support
 * improves cross-platform compatibility.
 */
static void load_sounds(void)
{
	char path[2048];
	char buffer[2048];
	ang_file *fff;

	/* Build the "sound" path */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "sound");
	ANGBAND_DIR_XTRA_SOUND = string_make(path);

	/* Find and open the config file */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA_SOUND, "sound.cfg");
	fff = file_open(path, MODE_READ, -1);

	/* Handle errors */
	if (!fff)
	{
		mac_warning("The sound configuration file could not be opened.");
		return;
	}
	
	/* Instantiate an autorelease pool for use by NSSound */
	NSAutoreleasePool *autorelease_pool;
	autorelease_pool = [[NSAutoreleasePool alloc] init];

	/*
	 * This loop may take a while depending on the count and size of samples
	 * to load.
	 */

	/* Parse the file */
	/* Lines are always of the form "name = sample [sample ...]" */
	while (file_getl(fff, buffer, sizeof(buffer)))
	{
		char *msg_name;
		char *cfg_sample_list;
		char *search;
		char *cur_token;
		char *next_token;
		int event;

		/* Skip anything not beginning with an alphabetic character */
		if (!buffer[0] || !isalpha((unsigned char)buffer[0])) continue;

		/* Split the line into two: message name, and the rest */
		search = strchr(buffer, ' ');
		cfg_sample_list = strchr(search + 1, ' ');
		if (!search) continue;
		if (!cfg_sample_list) continue;

		/* Set the message name, and terminate at first space */
		msg_name = buffer;
		search[0] = '\0';

		/* Make sure this is a valid event name */
		for (event = MSG_MAX - 1; event >= 0; event--)
		{
			if (strcmp(msg_name, angband_sound_name[event]) == 0)
				break;
		}
		if (event < 0) continue;

		/* Advance the sample list pointer so it's at the beginning of text */
		cfg_sample_list++;
		if (!cfg_sample_list[0]) continue;

		/* Terminate the current token */
		cur_token = cfg_sample_list;
		search = strchr(cur_token, ' ');
		if (search)
		{
			search[0] = '\0';
			next_token = search + 1;
		}
		else
		{
			next_token = NULL;
		}

		/*
		 * Now we find all the sample names and add them one by one
		 */
		while (cur_token)
		{
			int num = samples[event].num;

			/* Don't allow too many samples */
			if (num >= MAX_SAMPLES) break;

			/* Build the path to the sample */
			path_build(path, sizeof(path), ANGBAND_DIR_XTRA_SOUND, cur_token);
			if (file_exists(path)) {
				
				/* Load the sound into memory */
				samples[event].sound[num] = [[NSSound alloc] initWithContentsOfFile:[NSString stringWithUTF8String:path] byReference:NO];
				if (samples[event].sound[num] != nil) {
				
					/* Imcrement the sample count */
					samples[event].num++;
				}
			}

			/* Figure out next token */
			cur_token = next_token;
			if (next_token)
			{
				/* Try to find a space */
				search = strchr(cur_token, ' ');

				/* If we can find one, terminate, and set new "next" */
				if (search)
				{
					search[0] = '\0';
					next_token = search + 1;
				}
				else
				{
					/* Otherwise prevent infinite looping */
					next_token = NULL;
				}
			}
		}
	}

	/* Release the autorelease pool */
	[autorelease_pool release];

	/* Close the file */
	file_close(fff);

	/* Register the sound hook */
	sound_hook = play_sound;
}



/*
 * Play sound effects asynchronously.  Select a sound from any available
 * for the required event, and bridge to Cocoa to play it.
 */

static void play_sound(int event)
{
	/* Paranoia */
	if (event < 0 || event >= MSG_MAX) return;

	/* Check there are samples for this event */
	if (!samples[event].num) return;

	/* Instantiate an autorelease pool for use by NSSound */
	NSAutoreleasePool *autorelease_pool;
	autorelease_pool = [[NSAutoreleasePool alloc] init];

	/* Choose a random event */
	int s = randint0(samples[event].num);
	
	/* Stop the sound if it's currently playing */
	if ([samples[event].sound[s] isPlaying])
		[samples[event].sound[s] stop];

	/* Play the sound */
	[samples[event].sound[s] play];

	/* Release the autorelease pool */
	[autorelease_pool release];
}



/*** Support for the "z-term.c" package ***/

/*
 * Initialize a new Term
 *
 * Note also the "window type" called "noGrowDocProc", which might be more
 * appropriate for the main "screen" window.
 *
 * Note the use of "srcCopy" mode for optimized screen writes.
 */
static void Term_init_mac(term *t)
{
	term_data *td = (term_data*)(t->data);
	WindowAttributes wattrs;
	OSStatus err;

	wattrs = kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute
						| kWindowResizableAttribute;

	/* Make the window  */
	err = CreateNewWindow(
			kDocumentWindowClass,
			wattrs,
			&td->r,
			&td->w);

	Rect tmpR;
	GetWindowBounds((WindowRef)td->w, kWindowTitleBarRgn, &tmpR);
	int trueTop = td->r.top - (tmpR.bottom-tmpR.top);

	/* Enforce a minimum y position to avoid windows positioned vertically off screen */
	if (trueTop < GetMBarHeight()) trueTop = GetMBarHeight();
	
	MoveWindowStructure((WindowRef)td->w, td->r.left, trueTop);
	

	install_handlers(td->w);


	/* Fatal error */
	if (err != noErr) ExitToShell();

	/* Set refcon */
	SetWRefCon(td->w, (long)td);

	/* Set window title */
	SetWindowTitleWithCFString(td->w, CFStringCreateWithCString(NULL, td->title, kTextEncodingUS_ASCII));

	InstallStandardEventHandler(GetWindowEventTarget(td->w));

	/* Apply and Verify */
	term_data_check_font(td);
	term_data_check_size(td);
	term_data_resize(td);

	/* assert(td->mapped) */
	if (td->mapped)
	{
		WindowRef old_win = focus.active;

		ShowWindow(td->w);

		activate(td->w);
		term_data_color(COLOR_BLACK);
		CGContextFillRect(focus.ctx, td->bounds);
		activate(old_win);
	}

	/* Hack -- set "mapped" flag */
	t->mapped_flag = td->mapped;
}

/*
 * Nuke an old Term
 */
static void Term_nuke_mac(term *t)
{
#pragma unused(t)
}


/*
 * React to changes
 */
static errr Term_xtra_mac_react(void)
{
	/* Update colors */
	update_color_info();


	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_mac(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	/* Analyze */
	switch (n)
	{
		/* Make a noise */
		case TERM_XTRA_NOISE:
		{
			/* Make a noise */
			SysBeep(1);

			/* Success */
			return (0);
		}

		/* Process random events */
		case TERM_XTRA_BORED:
		{
			return (0);
		}

		/* Process pending events */
		case TERM_XTRA_EVENT:
		{
			/* Process an event */
			(void)CheckEvents(v);

			/* Success */
			return (0);
		}

		/* Flush all pending input events (if any) */
		case TERM_XTRA_FLUSH:
		{
			FlushEventsMatchingListFromQueue(GetMainEventQueue(),
				N_ELEMENTS(input_event_types), input_event_types);

			/* Success */
			return (0);
		}

		/* Hack -- Change the "soft level" */
		case TERM_XTRA_LEVEL:
		{
			/* Activate if requested */
			if (v) activate(td->w);

			/* Success */
			return (0);
		}

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		{
			if(!focus.ctx) activate(td->w);
			term_data_color(COLOR_BLACK);
			CGContextFillRect(focus.ctx, td->bounds);
			CGContextSynchronize(focus.ctx);

			return (0);
		}

		/* React to changes */
		case TERM_XTRA_REACT:
		{
			/* React to changes */
			return (Term_xtra_mac_react());
		}

		/* Delay (milliseconds) */
		case TERM_XTRA_DELAY:
		{
			/*
			 * WaitNextEvent relinquishes CPU as well as
			 * induces a screen refresh on OS X
			 */

			/* If needed */
			if (v > 0)
			{
				EventRecord tmp;
				UInt32 ticks;

				/* Convert millisecs to ticks */
				ticks = (v * 60L) / 1000;

				/*
				 * Hack? - Put the programme into sleep.
				 * No events match ~everyEvent, so nothing
				 * should be lost in Angband's event queue.
				 * Even if ticks are 0, it's worth calling for
				 * the above mentioned reasons.
				 */
				WaitNextEvent((EventMask)~everyEvent, &tmp, ticks, nil);
			}

			/* Success */
			return (0);
		}
		case TERM_XTRA_FRESH:
		{
			assert(focus.active == td->w);
			if(focus.ctx)
				CGContextSynchronize(focus.ctx);
			return (0);
		}
	}

	/* Oops */
	return (1);
}

/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_what()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_curs_mac(int x, int y)
{
	term_data *td = Term->data;

	if (!focus.active)
		activate(focus.active);

	CGContextSaveGState(focus.ctx);

	/* Temporarily set stroke color to yellow */
	char c;
	byte a = TERM_YELLOW;
	CGContextSetRGBStrokeColor(focus.ctx, focus.color_info[a][0],
							focus.color_info[a][1], focus.color_info[a][2], 1);

	/* Frame the grid, staying within the boundary. */
	int tile_wid = td->tile_wid;
	int tile_hgt = td->tile_hgt;

	if (tile_width != 1) {
		Term_what(x + 1, y, &a, &c);
		if (c == (char) 0xff)
			tile_wid *= tile_width;
	}

	if (tile_height != 1) {
		Term_what(x, y + 1, &a, &c);
		if (c == (char) 0xff)
			tile_hgt *= tile_height;
	}

	CGRect r = { { x * td->tile_wid + .5, y * td->tile_hgt + .5 },
	             { tile_wid - 1,          tile_hgt - 1 } };

	CGContextStrokeRectWithWidth(focus.ctx, r, 1.0);

	CGContextRestoreGState(focus.ctx);

	/* Success */
	return (0);
}

/*
 * Low level graphics helper (Assumes valid input)
 *
 * Based on suggestion by Julian Lighton
 *
 * Overwrite "n" old characters starting at    (x,y)
 * with the same ones in the background colour
 */
static void Term_wipe_mac_aux(int x, int y, int n)
{
	/* Use old screen image kept inside the term package */
	const char *cp = &(Term->old->c[y][x]);

	/* And write it in the background color */
	ShowTextAt(x, y, COLOR_BLACK, n, cp);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_mac(int x, int y, int n)
{
	/*
	 * Hack - overstrike the leftmost character with
	 * the background colour. This doesn't interfere with
	 * the graphics modes, because they set always_pict.
	 */
	if (use_overwrite_hack)
		Term_wipe_mac_aux(x, y, n);

	term_data_color(COLOR_BLACK);

	term_data *td = Term->data;
	assert(td);

	CGRect r = { { x * td->tile_wid, y * td->tile_hgt },
	             { n * td->tile_wid, -td->tile_hgt } };
	CGContextFillRect(focus.ctx, r);

	/* Success */
	return (0);
}


/*
 * Given a position in the ISO Latin-1 character set, return
 * the correct character on this system.
 */
 static byte Term_xchar_mac(byte c)
{
 	/* The Mac port uses the Latin-1 standard */
 	return (c);
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 */
static errr Term_text_mac(int x, int y, int n, byte a, const char *cp)
{
	if(!focus.ctx) activate(focus.active);

	/* Draw the string */
	ShowTextAt(x, y, a, n, cp);

	/* Success */
	return (0);
}

static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp,
			  const byte *tap, const char *tcp)
{
	if(!focus.ctx) activate(focus.active);

	/* Scan the input */
	for (int i = 0; i < n; i++)
	{
		byte a = *ap++;
		char c = *cp++;
		byte ta = *tap++;
		char tc = *tcp++;

		/* Hack -- a filler for tiles */
		if (a == 255 && (tile_height != 1 || tile_width != 1))
			continue;

		/* TODO: background should be overridden with neutral color */
		/* if unavailable. */
		/* Graphics -- if Available and Needed */
		if (use_graphics && (a & 0x80) && (c & 0x80))
		{
			DrawTile(x+i, y, a, c, ta, tc);
		}
		/*
		 * Deal with these cases:
		 * (1) the player changed tile width / height, or
		 * (2) fake fixed-width for proportional font
		 */
		else
		{
			ShowTextAt(x+i, y, a, 1, &c);
		}
	}

	/* Success */
	return (0);
}


/*
 * Create and initialize window number "i"
 */
static void term_data_link(int i)
{
	term *old = Term;

	term_data *td = &data[i];

	/* Only once */
	if (td->t) return;

	/* Require mapped */
	if (!td->mapped) return;

	/* Allocate */
	td->t = ZNEW(term);

	/* Initialize the term */
	term_init(td->t, td->cols, td->rows, td == &data[0] ? 100 : 1);

	/* Use a "software" cursor */
	td->t->soft_cursor = TRUE;

	/*
	 * We have an "icky" lower right corner, since
	 * the window resize control is placed there
	 */
	td->t->icky_corner = TRUE;

	/* Erase with "white space" */
	td->t->attr_blank = TERM_WHITE;
	td->t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	td->t->init_hook = Term_init_mac;
	td->t->nuke_hook = Term_nuke_mac;

	/* Prepare the function hooks */
	td->t->xtra_hook = Term_xtra_mac;
	td->t->wipe_hook = Term_wipe_mac;
	td->t->curs_hook = Term_curs_mac;
	td->t->bigcurs_hook = Term_curs_mac;
	td->t->text_hook = Term_text_mac;
	td->t->pict_hook = Term_pict_mac;
	td->t->xchar_hook = Term_xchar_mac; 


	td->t->never_bored = TRUE;
	td->t->never_frosh = TRUE;


	/* Link the local structure */
	td->t->data = (void *)(td);

	/* Activate it */
	Term_activate(td->t);
	/* Global pointer */
	angband_term[i] = td->t;

	/* Activate old */
	Term_activate(old);
}

/*
 * (Carbon, Bundle)
 * Return a POSIX pathname of the lib directory, or NULL if it can't be
 * located.  Caller must supply a buffer along with its size in bytes,
 * where returned pathname will be stored.
 */
static char *locate_lib(char *buf, size_t size)
{
	CFURLRef main_url;
	bool success;

	/* Obtain the URL of the main bundle */
	main_url = CFBundleCopyBundleURL(CFBundleGetMainBundle());

	/* Oops */
	if (!main_url) return (NULL);

	/* Get the URL in the file system's native string representation */
	success = CFURLGetFileSystemRepresentation(main_url, TRUE, (byte*)buf, size);

	/* Free the url */
	CFRelease(main_url);

	/* Oops */
	if (!success) return (NULL);    

	/* Append "/Contents/Resources/lib/" */
	my_strcat(buf, "/Contents/Resources/lib/", size);

	return (buf);
}


/*
 * Using Core Foundation's Preferences services -- pelpel
 *
 * Requires OS 8.6 or greater with CarbonLib 1.1 or greater. Or OS X,
 * of course.
 *
 * Without this, we can support older versions of OS 8 as well
 * (with CarbonLib 1.0.4).
 *
 * Frequent allocation/deallocation of small chunks of data is
 * far from my liking, but since this is only called at the
 * beginning and the end of a session, I hope this hardly matters.
 */


/*
 * Store "value" as the value for preferences item name
 * pointed by key
 */
static void save_preference(const char *key, type_union value)
{
	CFStringRef cf_key;
	CFPropertyListRef cf_value = 0;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* allocate and initialise the value */
	if(value.t == T_INT)
		cf_value = CFNumberCreate(NULL, kCFNumberIntType, &value.u.i);

	else if(value.t == T_STRING)
		cf_value = CFStringCreateWithCString(NULL, value.u.s, kTextEncodingUS_ASCII);

	else quit(format("Unrecognized save type %d\n", value.t));


	if ((cf_key != NULL) && (cf_value != NULL))
	{
		/* Store the key-value pair in the applications preferences */
		CFPreferencesSetAppValue(
			cf_key,
			cf_value,
			kCFPreferencesCurrentApplication);
	}

	/*
	 * Free CF data - the reverse order is a vain attempt to
	 * minimise memory fragmentation.
	 */
	if (cf_value) CFRelease(cf_value);
	if (cf_key) CFRelease(cf_key);
}


/*
 * Load preference value for key, returns TRUE if it succeeds with
 * vptr updated appropriately, FALSE otherwise.
 */
static bool load_preference(const char *key, type_union *vptr, size_t maxlen )
{
	CFStringRef cf_key;
	CFPropertyListRef cf_value;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* Oops */
	if (cf_key == NULL) return (FALSE);

	/* Retrieve value for the key */
	cf_value = CFPreferencesCopyAppValue(
		cf_key,
		kCFPreferencesCurrentApplication);

	/* Value not found */
	if (cf_value == NULL)
	{
		CFRelease(cf_key);
		return (FALSE);
	}

	/* Convert the value to appropriate type */
	if(vptr->t == T_INT)
		CFNumberGetValue( cf_value, kCFNumberIntType, &vptr->u.i);
	else if(vptr->t == T_STRING) {
		CFRange range = { 0, 200};
		(void) CFStringGetBytes (cf_value, range, kCFStringEncodingISOLatin1, 0, 0, (UInt8*)vptr->u.s, maxlen, 0);
	}

	/* Free CF data */
	CFRelease(cf_value);
	CFRelease(cf_key);

	/* Success */
	return (TRUE);
}

/* Convenience wrappers for commonly used type short */
static void save_pref_short(const char *key, short value)
{
	type_union u = i2u(value);
	save_preference(key, u);
}
static bool load_pref_short(const char *key, short *vptr)
{
	bool ret;
	type_union u = { T_INT };
	ret = load_preference(key, &u, 0);
	if( ret == TRUE ) *vptr = u.u.i;
	return ret;
}
static void save_pref_ushort(const char *key, unsigned short value)
{
	type_union u = i2u((int)value);
	save_preference(key, u);
}
static bool load_pref_ushort(const char *key, unsigned short *vptr)
{
	bool ret;
	type_union u = { T_INT };
	ret = load_preference(key, &u, 0);
	if( ret == TRUE ) *vptr = (unsigned short)u.u.i;
	return ret;
}

/* XXX Version number for pref file */
#define VERSION_MAJOR   3
#define VERSION_MINOR   0
#define VERSION_PATCH   14
#define VERSION_EXTRA   0

/*
 * Save preferences to preferences file for current host+current user+
 * current application.
 */
static void cf_save_prefs()
{
	/* Version stamp */
	save_pref_short("version.major", VERSION_MAJOR);
	save_pref_short("version.minor", VERSION_MINOR);
	save_pref_short("version.patch", VERSION_PATCH);
	save_pref_short("version.extra", VERSION_EXTRA);

	/* Gfx settings */
	/* tile width/height */
	save_pref_ushort("arg.tile_wid", tile_width);
	save_pref_ushort("arg.tile_hgt", tile_height);

	/* graphics mode */
	save_pref_ushort("graf_mode", graf_mode);

	/* text antialiasing */
	save_pref_ushort("arg.use_antialiasing", antialias);


	/* Windows */
	for (int i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		save_pref_short(format("term%d.mapped", i), td->mapped);

		save_pref_ushort(format("term%d.tile_wid", i), td->tile_wid);
		save_pref_ushort(format("term%d.tile_hgt", i), td->tile_hgt);

		save_pref_short(format("term%d.cols", i), td->cols);
		save_pref_short(format("term%d.rows", i), td->rows);
		save_pref_short(format("term%d.left", i), td->r.left);
		save_pref_short(format("term%d.top", i), td->r.top);

		/* Integer font sizes only */
		save_preference(format("term%d.font_size", i), i2u((int)td->font_size));
		save_preference(format("term%d.font_name", i), s2u(td->font_name));
	}

	/*
	 * Save the recent items array - directly.
	 */
	CFPreferencesSetAppValue(
		CFSTR("recent_items"),
		recentItemsArrayRef,
		kCFPreferencesCurrentApplication);
	
	/*
	 * Make sure preferences are persistent
	 */
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}


/*
 * Load preferences from preferences file for current host+current user+
 * current application.
 */
static void cf_load_prefs()
{
	short pref_major, pref_minor, pref_patch, pref_extra;
	short valid;

	/* Assume nothing is wrong, yet */
	bool ok = TRUE;

	/* Load version information */
	ok &= load_pref_short("version.major", &pref_major);
	ok &= load_pref_short("version.minor", &pref_minor);
	ok &= load_pref_short("version.patch", &pref_patch);
	ok &= load_pref_short("version.extra", &pref_extra);

	/* Any of the above failed */
	if (!ok)
	{
#if 0
		/* This may be the first run */
		mac_warning("Preferences are not found.");
#endif /* 0 */

		/* Ignore the rest */
		return;
	}


	/* Check version */
	if ((pref_major != VERSION_MAJOR) ||
		(pref_minor != VERSION_MINOR) ||
		(pref_patch != VERSION_PATCH) ||
		(pref_extra != VERSION_EXTRA))
	{
		/* Version 3.0.8 rewrote the preference file format - don't attempt to load previous versions. */
		if ((pref_major < 3) ||
			((pref_major == 3) && (pref_minor == 0) && (pref_patch < 8)))
		{
			
			/* Message */
			mac_warning(
				format("Ignoring %d.%d.%d.%d preferences.",
					pref_major, pref_minor, pref_patch, pref_extra));
	
			/* Ignore */
			return;
		}
		else
		{
			FSRef fsRef;
			char prefpath[1024];
			CFStringRef bundleid = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleIdentifierKey);
			CFIndex bufferlength = CFStringGetMaximumSizeForEncoding(CFStringGetLength(bundleid), kCFStringEncodingASCII) + 1;
			char bundlename[bufferlength];
			
			CFStringGetCString(bundleid, bundlename, bufferlength, kCFStringEncodingASCII);
			FSFindFolder(kOnAppropriateDisk, kPreferencesFolderType, kDontCreateFolder, &fsRef);
			FSRefMakePath(&fsRef, (UInt8 *)prefpath, 1024);
			mac_warning(format("Preference file has changed.  If you have display problems, delete %s/%s.plist and restart.", prefpath, bundlename));
			CFRelease(bundleid);
		}
	}


	/* HACK - Check for broken preferences */
	load_pref_short("term0.mapped", &valid);

	/* Ignore broken preferences */
	if (!valid)
	{
		/* mac_warning("Ignoring broken preferences."); */

		/* Ignore */
		return;
	}

	/* Gfx settings */
	unsigned short pref_tmp;

	if (load_pref_ushort("arg.tile_wid", &pref_tmp))
		tile_width = pref_tmp;

	if (load_pref_ushort("arg.tile_hgt", &pref_tmp))
		tile_height = pref_tmp;

	/* anti-aliasing */
	if(load_pref_ushort("arg.use_antialiasing", &pref_tmp))
		antialias = pref_tmp;

	if(load_pref_ushort("graf_mode", &pref_tmp))
		graf_mode = pref_tmp;


	/* Windows */
	for (int i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		load_pref_short(format("term%d.mapped", i), &td->mapped);
		CheckMenuItem(MyGetMenuHandle(kWindowMenu), kAngbandTerm+i, td->mapped);

		load_pref_ushort(format("term%d.tile_wid", i), &td->tile_wid);
		load_pref_ushort(format("term%d.tile_hgt", i), &td->tile_hgt);

		load_pref_short(format("term%d.cols", i), &td->cols);
		load_pref_short(format("term%d.rows", i), &td->rows);
		load_pref_short(format("term%d.left", i), &td->r.left);
		load_pref_short(format("term%d.top", i), &td->r.top);

		type_union u = {T_INT};
		if(load_preference(format("term%d.font_size", i), &u, sizeof(int)))
			td->font_size = (float) u.u.i;
		u = s2u(td->font_name);
		if(load_preference(format("term%d.font_name", i), &u, sizeof(td->font_name))) {
			ATSUFontID fid = 0;
			ATSUFindFontFromName(td->font_name, strlen(td->font_name),
								kFontPostscriptName, kFontMacintoshPlatform,
								kFontNoScriptCode, kFontNoLanguageCode, &fid);
			if(fid) td->font_id = fid;
			/* Use the default */
			else my_strcpy(td->font_name, "Monaco", sizeof(td->font_name));
		}
	}
	
	/*
	 * Load the recent items array, if present, directly
	 */
	CFArrayRef recentItemsLoaded = (CFArrayRef)CFPreferencesCopyAppValue(
		CFSTR("recent_items"),
		kCFPreferencesCurrentApplication);
	if (recentItemsLoaded != NULL)
	{
		CFRelease(recentItemsArrayRef);
		recentItemsArrayRef = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, recentItemsLoaded);
		CFRelease(recentItemsLoaded);
		redrawRecentItemsMenu();
	}
}


/*
 * Hack -- default data for a window
 */
static void term_data_hack(term_data *td)
{
	/* Default to Monaco font */
	ATSUFontID fid = 0;

	ATSUFindFontFromName("Monaco", strlen("Monaco"), kFontPostscriptName,
							kFontMacintoshPlatform, kFontNoScriptCode,
							kFontNoLanguageCode, &fid);


	if(!fid)
		quit("Failed to find font 'Monaco'");

	/* Wipe it */
	WIPE(td, term_data);

	/* Start hidden */
	td->mapped = FALSE;

	/* Default font */
	td->font_id = fid;
	my_strcpy(td->font_name, "Monaco", sizeof(td->font_name));

	/* Default font size - was 12 */
	td->font_size = 14;

	/* Default size */
	td->rows = 24;
	td->cols = 80;

	/* Default position */
	td->r.left = 10;
	td->r.top = 40;
}


/*
 * Read the preference file, Create the windows.
 *
 * We attempt to use "FindFolder()" to track down the preference file.
 */
static void init_windows(void)
{
	term_data *td;
	/*** Default values ***/

	/* Initialize (backwards) */
	for (int b = 0, i = MAX_TERM_DATA; i-- > 0; )
	{
		/* Obtain */
		td = &data[i];

		/* Defaults */
		term_data_hack(td);

		/* Copy the title */
		my_strcpy((char *)(td->title), angband_term_name[i], sizeof(td->title));

		/* Tile the windows */
		td->r.left += (b * 30);
		td->r.top += (b * 30);

		/* Tile */
		b++;
	}

	/*** Load preferences ***/

	cf_load_prefs();

	/*** Instantiate ***/

	/* Main window */
	td = &data[0];

	/* Start visible */
	td->mapped = TRUE;

	/* Link (backwards, for stacking order) */
	for (int i = MAX_TERM_DATA; i-- > 0; )
	{
		term_data_link(i);
	}

	/* Main window */
	td = &data[0];

	/* Main window */
	Term_activate(td->t);
}


/* Set up the contents of the about dialog */
static void init_aboutdialogcontent()
{
	HIViewRef aboutDialogViewRef;
	OSStatus err;
	
	/* Set the application name from the constants set in defines.h */
	char *applicationName = format("%s", buildid);
	CFStringRef cfstr_applicationName = CFStringCreateWithBytes(NULL, (byte *)applicationName,
										strlen(applicationName), kCFStringEncodingASCII, false);
	HIViewFindByID(HIViewGetRoot(aboutDialog), aboutDialogName, &aboutDialogViewRef);
	SetControlData(aboutDialogViewRef, kControlEntireControl, kControlStaticTextCFStringTag, sizeof(cfstr_applicationName), &cfstr_applicationName);
	CFRelease(cfstr_applicationName);
	
	/* Set the application copyright as set up in variable.c */
	HIViewFindByID(HIViewGetRoot(aboutDialog), aboutDialogCopyright, &aboutDialogViewRef);
	CFStringRef cfstr_applicationCopyright = CFStringCreateWithBytes(NULL, (byte *)copyright,
										strlen(copyright), kCFStringEncodingASCII, false);
	SetControlData(aboutDialogViewRef, kControlEntireControl, kControlStaticTextCFStringTag, sizeof(cfstr_applicationCopyright), &cfstr_applicationCopyright);
	CFRelease(cfstr_applicationCopyright);

	/* Use a small font for the copyright text */
	TXNObject txnObject = HITextViewGetTXNObject(aboutDialogViewRef);
	TXNTypeAttributes typeAttr[1];
	typeAttr[0].tag = kTXNQDFontSizeAttribute;
	typeAttr[0].size = kTXNFontSizeAttributeSize;
	typeAttr[0].data.dataValue = FloatToFixed(10);
	err = TXNSetTypeAttributes(txnObject, 1, typeAttr, kTXNStartOffset, kTXNEndOffset);

	/* Get the application icon and draw it */
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	ControlRef iconControl;
	FSRef ref;
	FSSpec appSpec;
	IconRef iconRef;
	ControlButtonContentInfo cInfo;

	err = GetProcessBundleLocation(&psn, &ref);
	if(err == noErr)
		err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &appSpec, NULL);
	if( err == noErr )
		err = GetIconRefFromFile((const FSSpec *)&appSpec, &iconRef, nil);
	if(err == noErr)
		err = GetControlByID(aboutDialog, &aboutDialogIcon, &iconControl);
	if( err == noErr )
	{
		cInfo.contentType = kControlContentIconRef;
		cInfo.u.iconRef = iconRef;
		err = SetControlData(iconControl, 0, kControlIconContentTag, sizeof(cInfo), (Ptr)&cInfo);
	}
}


/*
 * Save preferences
 */
static void save_pref_file(void)
{
	cf_save_prefs();
}


/*
 * Prepare savefile dialogue and set the variable
 * savefile accordingly. Returns true if it succeeds, false (or
 * aborts) otherwise. If all is false, only allow files whose type
 * is 'SAVE'.
 * Originally written by Peter Ammon
 */
static bool select_savefile(bool all)
{
	OSErr err;
	FSSpec theFolderSpec;
	FSSpec savedGameSpec;
	NavDialogOptions dialogOptions;
	NavReplyRecord reply;
	/* Used only when 'all' is true */
	NavTypeList types = {'A271', 1, 1, {'SAVE'}};
	NavTypeListHandle myTypeList;
	AEDesc defaultLocation;

	/* Look for the "Angband/save/" sub-folder */
	char path[1024];
	path_build(path, sizeof(path), ANGBAND_DIR_USER, "save");
	err = path_to_spec(path, &theFolderSpec);

	if (err != noErr) quit_fmt("Unable to find the savefile folder! (Error %d)", err);

	/* Get default Navigator dialog options */
	err = NavGetDefaultDialogOptions(&dialogOptions);

	/* Clear preview option */
	dialogOptions.dialogOptionFlags &= ~kNavAllowPreviews;

	/* Disable multiple file selection */
	dialogOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;

	/* Make descriptor for default location */
	err = AECreateDesc(typeFSS, &theFolderSpec, sizeof(FSSpec),
		&defaultLocation);

	/* Oops */
	if (err != noErr) quit("Unable to allocate descriptor");

	/* We are indifferent to signature and file types */
	if (all)
	{
		myTypeList = (NavTypeListHandle)nil;
	}

	/* Set up type handle */
	else
	{
		err = PtrToHand(&types, (Handle *)&myTypeList, sizeof(NavTypeList));

		/* Oops */
		if (err != noErr) quit("Error in PtrToHand. Try enlarging heap");

	}

	/* Call NavGetFile() with the types list */
	err = NavChooseFile(&defaultLocation, &reply, &dialogOptions, NULL,
		NULL, NULL, myTypeList, NULL);

	/* Free type list */
	if (!all) DisposeHandle((Handle)myTypeList);

	/* Error */
	if (err != noErr)
	{
		/* Nothing */
	}

	/* Invalid response -- allow the user to cancel */
	else if (!reply.validRecord)
	{
		/* Hack -- Fake error */
		err = -1;
	}

	/* Retrieve FSSpec from the reply */
	else
	{
		AEKeyword theKeyword;
		DescType actualType;
		Size actualSize;

		/* Get a pointer to selected file */
		(void)AEGetNthPtr(&reply.selection, 1, typeFSS, &theKeyword,
			&actualType, &savedGameSpec, sizeof(FSSpec), &actualSize);

		/* Dispose NavReplyRecord, resources and descriptors */
		(void)NavDisposeReply(&reply);
	}

	/* Dispose location info */
	AEDisposeDesc(&defaultLocation);

	/* Error */
	if (err != noErr) return (FALSE);

	/* Convert FSSpec to pathname and store it in variable savefile */
	(void)spec_to_path(&savedGameSpec, savefile, sizeof(savefile));

	/* Success */
	return (TRUE);
}

/*
 * Initialize the menus
 *
 * Fixed top level menus are now loaded all at once by GetNewMBar().
 * Although this simplifies the function a bit, we have to make sure
 * that resources have all the expected entries defined XXX XXX
 */
static void init_menubar(void)
{
	/* Boilerplate nib stuff */
	IBNibRef nib;
	OSStatus err;
	if((err = CreateNibReference(CFSTR("main"), &nib)))
		quit("Cannot find the main nib bundle!");

	if((err = SetMenuBarFromNib(nib, CFSTR("MenuBar"))))
		quit("Cannot prepare menu bar!");

	/* Load the about dialog and set its contents */
	(void) CreateWindowFromNib(nib, CFSTR("DLOG:about"), &aboutDialog);
	init_aboutdialogcontent();

	DisposeNibReference(nib);

	MenuRef m = MyGetMenuHandle(kGraphicsMenu);
	for (int i = 1; i <= CountMenuItems(m); i++) {
		/* Invalid entry */
		SetMenuItemRefCon(m, i, -1);
	}
	for (size_t i = 0; i < N_ELEMENTS(graphics_modes); i++) {
		SetMenuItemRefCon(m, graphics_modes[i].menuItem, i);
	}

	/* Set up bigtile menus */
	m = MyGetMenuHandle(kBigtileWidthMenu);
	for (size_t i = 1; i <= CountMenuItems(m); i++) {
		SetMenuItemRefCon(m, i, i);
	}

	m = MyGetMenuHandle(kBigtileHeightMenu);
	for (size_t i = 1; i <= CountMenuItems(m); i++) {
		SetMenuItemRefCon(m, i, i);
	}

	for(int j = kTileWidMenu; j <= kTileHgtMenu; j++) {
		m = MyGetMenuHandle(j);
		for(UInt32 i = MIN_FONT; i <= 32; i++) {
			char buf[15];
			/* Tile size */
			strnfmt((char*)buf, 15, "%d", i);
			CFStringRef cfstr = CFStringCreateWithBytes ( NULL, (byte*) buf,
									strlen(buf), kCFStringEncodingASCII, false);
			AppendMenuItemTextWithCFString(m, cfstr, 0, j, NULL);
			SetMenuItemRefCon(m, i-MIN_FONT+1, i);
			CFRelease(cfstr);
		}
	}
}

/* Install the handlers from the Commands table. */
static void install_handlers(WindowRef w)
{
	EventHandlerRef prevRef;
	for(size_t i = 0; i < N_ELEMENTS(event_defs) ; i++) {
		const CommandDef *def = &event_defs[i];

		/* Install window handlers only for kWINDOW events */
		if ((!w) == (def->targetID == kWINDOW))
			continue;

		EventHandlerUPP evtUPP = NewEventHandlerUPP( def->handler );
		const EventTypeSpec eventSpec = { def->evtClass, def->evtType };

		EventTargetRef target = GetApplicationEventTarget();
		if(def->targetID == kWINDOW)
			target = GetWindowEventTarget(w);
		else if(def->targetID)
			target = GetMenuEventTarget(MyGetMenuHandle(def->targetID));
		
		OSStatus err = InstallEventHandler(target, evtUPP, 1, &eventSpec,
													def->userData, &prevRef);
		if(err == eventHandlerAlreadyInstalledErr) {
			err = AddEventTypesToHandler(prevRef, 1, &eventSpec);
		}
	}
}

static int funcGTE(int a, int b) { return a >= b; }
static int funcConst(int a, int c) {return c; }

/* This initializes all the menus with values that change unpredictably.
 * This function is called on every menu draw and therefore should be kept
 * light and fast; menus that change rarely are done at the time of change
 */
static void validate_menus(void)
{
	WindowRef w = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (!w || !initialized)
		return;

	term_data *td = (term_data *) GetWRefCon(w);
	if (!td)
		return;

	struct {
		int menu;                /* Radio-style Menu ID to validate */
		URefCon cur;             /* Value in use (Compare to RefCon) */
		int limit;               /* Constraint value */
		int (*cmp) (int, int);   /* Filter function */
	} funcs [] = {
		{ kTileWidMenu, td->tile_wid, td->font_wid, funcGTE },
		{ kTileHgtMenu, td->tile_hgt, td->font_hgt, funcGTE },
		{ kGraphicsMenu, graf_mode, 1, funcConst },
		{ kBigtileWidthMenu, tile_width, 1, funcGTE },
		{ kBigtileHeightMenu, tile_height, 1, funcGTE },
	};

	MenuHandle m;

	if (cmd.command != CMD_NULL) {
		EnableAllMenuItems(MyGetMenuHandle(kGraphicsMenu));
	}

	for(size_t i = 0; i < N_ELEMENTS(funcs); i++) {
		m = MyGetMenuHandle(funcs[i].menu);
		int n = CountMenuItems(m);
		for (int j = 1; j <= n; j++) {
			URefCon value;
			GetMenuItemRefCon(m, j, &value);
			CheckMenuItem(m, j, funcs[i].cur == value);
			if (funcs[i].cmp(value, funcs[i].limit))
				EnableMenuItem(m, j);
			else
				DisableMenuItem(m, j);
		}
	}

	if (cmd.command != CMD_NULL && character_generated)
		EnableMenuItem(MyGetMenuHandle(kFileMenu), kSave);
	else
		DisableMenuItem(MyGetMenuHandle(kFileMenu), kSave);

	m = MyGetMenuHandle(kFontMenu);
	CheckMenuItem(m, kAntialias, antialias);
}

static OSStatus ValidateMenuCommand(EventHandlerCallRef inCallRef,
									EventRef inEvent, void *inUserData )
{
	validate_menus();
	return noErr;
}

/* Populate the recent items menu */
static void redrawRecentItemsMenu()
{
	DeleteMenuItems(MyGetMenuHandle(kOpenRecentMenu), 1, CountMenuItems(MyGetMenuHandle(kOpenRecentMenu)));
	if (!recentItemsArrayRef || CFArrayGetCount(recentItemsArrayRef) == 0)
	{
		AppendMenuItemTextWithCFString(MyGetMenuHandle(kOpenRecentMenu), CFSTR("No recent items"), kMenuItemAttrDisabled, kOpenRecentMenu, NULL);
	}
	else
	{
		for (int i = 0; i < CFArrayGetCount(recentItemsArrayRef); i++)
		{
			OSErr err;
			FSRef recentFileRef;
			CFDataRef recentFileData;
			Boolean updateAlias = FALSE;
			MenuItemIndex item;

			recentFileData = CFArrayGetValueAtIndex(recentItemsArrayRef, i);
			if (CFDataGetTypeID() != CFGetTypeID(recentFileData)) continue;
			AliasHandle recentFileAlias = (AliasHandle)NewHandle(CFDataGetLength(recentFileData));
			CFDataGetBytes(recentFileData, CFRangeMake(0, CFDataGetLength(recentFileData)), (UInt8 *) *recentFileAlias);
			
			err = FSResolveAlias(NULL, recentFileAlias, &recentFileRef, &updateAlias);
			if (err != noErr) continue;
			
			HFSUniStr255 recentFileName;
			err = FSGetCatalogInfo(&recentFileRef, kFSCatInfoNone, NULL, &recentFileName, NULL, NULL);
			if (err != noErr) continue;
			
			CFStringRef cfstr = CFStringCreateWithCharacters(kCFAllocatorDefault, recentFileName.unicode, recentFileName.length);
			AppendMenuItemTextWithCFString(MyGetMenuHandle(kOpenRecentMenu), cfstr, 0, i, &item);

			/* Add a shortcut key -- command-alt-number */
			if (i < 9)
			{
				SetMenuItemCommandKey(MyGetMenuHandle(kOpenRecentMenu), item, false, 0x31+i);
				SetMenuItemModifiers(MyGetMenuHandle(kOpenRecentMenu), item, kMenuOptionModifier);
			}

			CFRelease(cfstr);
		}
		AppendMenuItemTextWithCFString(MyGetMenuHandle(kOpenRecentMenu), CFSTR("-"), kMenuItemAttrSeparator, FOUR_CHAR_CODE('Rsep'), NULL);
		AppendMenuItemTextWithCFString(MyGetMenuHandle(kOpenRecentMenu), CFSTR("Clear menu"), 0, FOUR_CHAR_CODE('Rclr'), NULL);
	}
}

/* Add a savefile to the recent items list, or update its existing status */
static void updateRecentItems(char *savefile)
{
	OSErr err;
	FSRef recentFileRef;
	AliasHandle recentFileAlias;
	CFDataRef newRecentFileData;
	CFDataRef recentFileData;

	/* Convert the save path to an FSRef, then an Alias, and convert to data ready for storage */
	err = FSPathMakeRef((byte *)savefile, &recentFileRef, NULL);
	if (err != noErr) return;
	err = FSNewAlias(NULL, &recentFileRef, &recentFileAlias);
	if (err != noErr) return;
	newRecentFileData = CFDataCreate(kCFAllocatorDefault, (UInt8 *) *recentFileAlias, GetHandleSize((Handle)recentFileAlias));
	
	/* Loop through the recent items array, and delete any matches */
	for (int i = CFArrayGetCount(recentItemsArrayRef) - 1; i >= 0; i--)
	{
		Boolean updateAlias = FALSE;
		char recentFilePath[1024];

		/* Retrieve the recent item, resolve the alias, and extract a path */
		recentFileData = CFArrayGetValueAtIndex(recentItemsArrayRef, i);
		if (CFDataGetTypeID() != CFGetTypeID(recentFileData)) continue;
		AliasHandle recentFileAlias = (AliasHandle)NewHandle(CFDataGetLength(recentFileData));
		CFDataGetBytes(recentFileData, CFRangeMake(0, CFDataGetLength(recentFileData)), (UInt8 *) *recentFileAlias);
		
		/* If resolving an alias fails, don't delete it - the array is size-limited
		 * anyway, and this allows network shares or removeable drives to come back later */
		err = FSResolveAlias(NULL, recentFileAlias, &recentFileRef, &updateAlias);
		if (err != noErr) continue;
		err = FSRefMakePath(&recentFileRef, (byte *)recentFilePath, 1024);
		if (err != noErr) continue;

		/* Remove the item from the array if the paths match */
		if (strcmp(recentFilePath, savefile) == 0)
		{
			CFArrayRemoveValueAtIndex(recentItemsArrayRef, i);
			continue;
		}

		/* If performing a file search via the alias updated it, save changes */
		if (updateAlias)
		{
			recentFileData = CFDataCreate(kCFAllocatorDefault, (UInt8 *) *recentFileAlias, GetHandleSize((Handle)recentFileAlias));
			CFArraySetValueAtIndex(recentItemsArrayRef, i, newRecentFileData);
		}
	}
	
	/* Insert the encoded alias at the start of the recent items array */
	CFArrayInsertValueAtIndex(recentItemsArrayRef, 0, newRecentFileData);
	
	/* Limit to ten items */
	if (CFArrayGetCount(recentItemsArrayRef) > 10)
		CFArrayRemoveValueAtIndex(recentItemsArrayRef, 10);

	/* Redraw the menu */
	redrawRecentItemsMenu();
}

/* Handle a selection in the recent items menu */
static OSStatus OpenRecentCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	HICommand command;
	command.commandID = 0;
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(command), NULL, &command);
	
	/* If the 'Clear menu' command was selected, flush the recent items array */
	if (command.commandID == FOUR_CHAR_CODE('Rclr')) {
		CFArrayRemoveAllValues(recentItemsArrayRef);
	
	/* Otherwise locate the correct filepath and open it. */
	} else {
		if (cmd.command != CMD_NULL || command.commandID >= (UInt32)CFArrayGetCount(recentItemsArrayRef))
			return eventNotHandledErr;

		OSErr err;
		FSRef recentFileRef;
		AliasHandle recentFileAlias;
		CFDataRef recentFileData;
		Boolean updateAlias = FALSE;

		recentFileData = CFArrayGetValueAtIndex(recentItemsArrayRef, command.commandID);
		if (CFDataGetTypeID() != CFGetTypeID(recentFileData)) return eventNotHandledErr;
		recentFileAlias = (AliasHandle)NewHandle(CFDataGetLength(recentFileData));
		CFDataGetBytes(recentFileData, CFRangeMake(0, CFDataGetLength(recentFileData)), (UInt8 *) *recentFileAlias);
		err = FSResolveAlias(NULL, recentFileAlias, &recentFileRef, &updateAlias);
		if (err != noErr) return eventNotHandledErr;
		err = FSRefMakePath(&recentFileRef, (byte *)savefile, 1024);
		if (err != noErr) return eventNotHandledErr;
		
		cmd.command = CMD_LOADFILE;
	}

	/* Redraw the menu */
	redrawRecentItemsMenu();

	return noErr;
}

static OSStatus AngbandGame(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	UInt32 i;

	/* Only enabled options are Fonts, Open/New/Import and Quit. */
	DisableAllMenuItems(MyGetMenuHandle(kTileWidMenu));
	DisableAllMenuItems(MyGetMenuHandle(kTileHgtMenu));

	SetFontInfoForSelection(kFontSelectionATSUIType, 0, 0, 0);

	for(i = kNew; i <= kImport; i++)
		EnableMenuItem(MyGetMenuHandle(kFileMenu), i);

	/* Validate graphics, after bootstrapped opening of terminals */
	for(i = 0; i < N_ELEMENTS(data); i++) {
		if(data[i].mapped)
			RevalidateGraphics(&data[i], FALSE);
	}

	/* Flush the prompt */
	Term_fresh();
	Term_flush();

	play_game();
	quit(0);
	/* Not reached */
	return noErr;
}

/*
 *  "New" / "Open" /  "Import"
 */
static OSStatus openGame(int op)
{
	/* If a game is in progress, do not proceed */
	if (cmd.command != CMD_NULL) return noErr;

	/* Let the player to choose savefile */
	if (op != kNew && 0 == select_savefile(op == kImport))
	{
		/* Failed to open */
		return noErr;
	}

	/* Disable the file-handling options in the file menu */
	for(int i = kNew; i <= kImport; i++)
		DisableMenuItem(MyGetMenuHandle(kFileMenu), i);

	/* Wait for a keypress */
	pause_line(Term);

	/* Set the game status */
	if (op == kNew)
		cmd.command = CMD_NEWGAME;
	else
		cmd.command = CMD_LOADFILE;
		
	return noErr;
}

/*
 *    Run the event loop and return a gameplay status to init_angband
 */
static errr get_cmd_init()
{ 
	EventTargetRef target = GetEventDispatcherTarget();
	OSStatus err;
	EventRef event;
	
	/* Prompt the user */ 
	prt("[Choose 'New', 'Open' or 'Import' from the 'File' menu]", 23, 11);
	Term_fresh();
	
	while (cmd.command == CMD_NULL) {
		err = ReceiveNextEvent(0, 0, kEventDurationForever, true, &event);
		if(err == noErr) {
			SendEventToEventTarget (event, target);
			ReleaseEvent(event);
		}
	}

	/* Push the command to the game. */
	cmd_insert_s(&cmd);
		
	/* A game is starting - update status and tracking as appropriate. */ 
	term_data *td0 = &data[0];
	ChangeWindowAttributes(td0->w, kWindowNoAttributes, kWindowCloseBoxAttribute);
	DisableMenuItem(MyGetMenuHandle(kFileMenu), kClose);

	/* Disable the file-handling options in the file menu.
	 * This has to be done separately for new/open due to messages/prompts
	 * which may delay open while the menus are still accessibile. */
	for(int i = kNew; i <= kImport; i++)
		DisableMenuItem(MyGetMenuHandle(kFileMenu), i);
			
	/* If supplied with a savefile, update the Recent Items list */
	if (savefile[0])
	{
		updateRecentItems(savefile);
		redrawRecentItemsMenu();
	}
	
	return 0; 
} 

static errr crb_get_cmd(cmd_context context, bool wait)
{
	if (context == CMD_INIT) 
		return get_cmd_init();
	else 
		return textui_get_cmd(context, wait);
}

static OSStatus QuitCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	if (cmd.command == CMD_NULL && !character_generated)
		quit(0);    
	else Term_key_push(KTRL('x'));
	return noErr;
}

static OSStatus CommandCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	HICommand command;
	command.commandID = 0;
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(command), NULL, &command);
	UInt32 attrib;
	GetEventParameter( inEvent, kEventParamKeyModifiers, typeUInt32,
							NULL, sizeof(attrib), NULL, &attrib);
	switch(command.commandID) {
	default:
		return eventNotHandledErr;
	case 'save':
		if(cmd.command != CMD_NULL && character_generated)
			Term_key_push(KTRL('S'));
		break;
	case 'open':
		openGame(command.menu.menuItemIndex);
		break;
	case 'font':
	  {
		if(!FPIsFontPanelVisible()) {
			WindowRef w = FrontWindow();
			if (w) {
				term_data *td = (term_data*) GetWRefCon(w);
				SetFontInfoForSelection(kFontSelectionATSUIType, 1,
										&td->ginfo->style, NULL);
			}
		}
		CFStringRef tags[] = {CFSTR("Show Fonts"), CFSTR("Hide Fonts")};
		FPShowHideFontPanel(); 
		SetMenuItemTextWithCFString(command.menu.menuRef, kFonts,
											tags[FPIsFontPanelVisible()] );
		break;
	  }
	}

	return noErr;
}


static OSStatus CloseCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	WindowRef w = 0;
	term_data *td;
	GetEventParameter(inEvent, kEventParamDirectObject,
							typeWindowRef, NULL, sizeof(w), NULL, &w);

	td = (term_data*) GetWRefCon(w);

	if(cmd.command == CMD_NULL && !character_generated && td == &data[0])
		quit(0);

	hibernate();

	/* Track the go-away box */
	if (td && td != &data[0])
	{
		/* Not Mapped */
		td->mapped = FALSE;

		/* Not Mapped */
		td->t->mapped_flag = FALSE;

		/* Hide the window */
		TransitionWindow(td->w,
						kWindowZoomTransitionEffect,
						kWindowHideTransitionAction,
						NULL);

		/* Update the menu status */
		CheckMenuItem(MyGetMenuHandle(kWindowMenu), kAngbandTerm+(td - &data[0]), FALSE);
	}
	return noErr;
}



static OSStatus ResizeCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	int x, y;
	WindowRef w = 0;
	unsigned flags;

	term_data *td;
	term *old = Term;
	int err ;

	GetEventParameter(inEvent, kEventParamDirectObject,
							typeWindowRef, NULL, sizeof(w), NULL, &w);
	err = GetEventParameter(inEvent, kEventParamAttributes,
							typeUInt32, NULL, sizeof(flags), NULL, &flags);

	td = (term_data*) GetWRefCon(w);

	/* Oops */
	if (!td) return noErr;


	/* Obtain geometry of resized window */

	Rect tmpR;
	GetWindowBounds((WindowRef)td->w, kWindowContentRgn, &tmpR);
	td->r = tmpR;
	if(td->r.top < 40) td->r.top = 40;

	/* Extract the new ClipRect size in pixels */
	y = tmpR.bottom - tmpR.top - BORDER_WID;
	x = tmpR.right - tmpR.left - BORDER_WID * 2;

	/* Ignore drag effects, other than for moving the mouse origin */
	if(td->rows * td->tile_hgt == y && td->cols * td->tile_wid == x)
		return noErr;

	/* Extract a "close" approximation */
	td->rows = y / td->tile_hgt;
	td->cols = x / td->tile_wid;


	/* Apply and Verify */
	term_data_check_size(td);

	/* Activate */
	Term_activate(td->t);
	/* Resize the term to correspond to new count. */
	Term_resize(td->cols, td->rows);
	Term_activate(old);

	/* Close the old (different size) CGContext */
	hibernate();
	/* Resize and Redraw */
	term_data_resize(td);

	/* Since we don't know what view needs to be updated, recalculate */
	/* and redraw them all. (term_data_redraw() is not sufficient) */
	Term_key_push(KTRL('R'));
	
	return eventNotHandledErr;
}

static void graphics_aux(UInt32 op)
{
	graf_mode = op;
	use_transparency = graphics_modes[op].trans;
	pict_id = graphics_modes[op].file;
	graf_width = graf_height = graphics_modes[op].size;
	use_graphics = (op != 0);
	graf_mode = op;
	ANGBAND_GRAF = graphics_modes[op].name;
	arg_graphics = op;

	graphics_nuke();

	/* load tiles and setup GWorlds if tiles are requested */
	if (use_graphics && (graphics_init() != 0))
	{
		/* Oops */
		plog("Cannot initialize graphics!");

		/* reset graphics flags */
		use_graphics = 0;
		graf_mode = 0;
		ANGBAND_GRAF = 0;

		/* reset transparency mode */
		use_transparency = false;
	}
	/* Reset visuals, without updating the screen */
	if (initialized && cmd.command != CMD_NULL)
	{
		reset_visuals(TRUE);
	}
	RevalidateGraphics(&data[0], FALSE);
	Term_key_push(KTRL('R'));
}

static OSStatus TileSizeCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	term_data *td = Term->data;
	assert(td);

	HICommand cmd;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof(cmd), NULL, &cmd);
	UInt32 newSize = 0;
	GetMenuItemRefCon(cmd.menu.menuRef, cmd.menu.menuItemIndex, &newSize);

	if (GetMenuID(cmd.menu.menuRef) == kTileWidMenu) {
		if (td->font_wid > newSize || newSize == td->tile_wid)
			return noErr;
		else
			td->tile_wid = newSize;
	} else if (GetMenuID(cmd.menu.menuRef) == kTileHgtMenu) {
		if (td->font_hgt > newSize || newSize == td->tile_hgt)
			return noErr;
		else
			td->tile_hgt = newSize;
	} else {
		return eventNotHandledErr;
	}

	RevalidateGraphics(td, FALSE);

	return noErr;
}

static OSStatus RestoreCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData)
{
	WindowRef w = 0;
	GetEventParameter(inEvent, kEventParamDirectObject, typeWindowRef,
							NULL, sizeof(w), NULL, &w);

	term_data *td = (term_data*) GetWRefCon(w);
	if (!td)
		return eventNotHandledErr;

	/* Mapped */
	td->mapped = TRUE;

	int i = td - &data[0];

	/* Link */
	term_data_link(i);

	/* Mapped (?) */
	td->t->mapped_flag = TRUE;

	/* Bring to the front */
	SelectWindow(td->w);

	/* Update menu states */
	if (td == &data[0] && cmd.command != CMD_NULL)
		DisableMenuItem(MyGetMenuHandle(kFileMenu), kClose);
	else
		EnableMenuItem(MyGetMenuHandle(kFileMenu), kClose);
	
	return noErr;
}

static OSStatus TerminalCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	HICommand command;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(HICommand), NULL, &command);

	/* Offset of Angband term in Window menu. */
	int i = command.menu.menuItemIndex - kAngbandTerm;

	/* Check legality of choice */
	if ((i < 0) || (i >= MAX_TERM_DATA)) return eventNotHandledErr;

	/* Obtain the window */
	term_data *td = &data[i];

	/* Mapped */
	td->mapped = TRUE;

	/* Link */
	term_data_link(i);

	/* Mapped (?) */
	td->t->mapped_flag = TRUE;

	/* Show the window */
	TransitionWindow(td->w,
				kWindowZoomTransitionEffect,
				kWindowShowTransitionAction,
				NULL);

	/* Update the menu status */
	CheckMenuItem(MyGetMenuHandle(kWindowMenu), kAngbandTerm+i, TRUE);

	term_data_check_font(td);
	term_data_check_size(td);
	term_data_resize(td);
	Term_resize(td->cols, td->rows);
	term_data_redraw(td);

	/* Bring to the front */
	SelectWindow(td->w);

	return noErr;
}

static OSStatus RevalidateGraphics(term_data *td, bool reset_tilesize)
{
	if (!td) return noErr;

	/*
	 * Reset the tilesize on graphics changes; term_data_check_font recalculates
	 * this after it's been reset.  However, only reset the tilesize for default
	 * and font events (not startup - 'Play' 'Band' - or manual changes)
	*/
	if (reset_tilesize)
	{
		td->tile_wid = td->tile_hgt = 0;
	}

	/* Clear the graphics tile cache. */
	graphics_tiles_nuke();

	/* Sanity check for rows, columns, tilesize. */
	term_data_check_font(td);
	term_data_check_size(td);
	
	/* Window size changes */
	term_data_resize(td);
	term_data_redraw(td);

	return noErr;
}

static OSStatus UpdateCommand(EventHandlerCallRef inCallRef,
								EventRef inEvent, void *inUserData)
{
	WindowRef w = 0;
	term_data *td;
	
	GetEventParameter(inEvent, kEventParamDirectObject,
							typeWindowRef, NULL, sizeof(w), NULL, &w);
	
	if(!w) return noErr;

	/* Clear window's update region and clip drawings with it */
	BeginUpdate(w);
	td = (term_data*) GetWRefCon(w);
	if (td) term_data_redraw(td);
	EndUpdate(w);

	return noErr;
}

static void toggle_antialias(HICommand *command, void *data)
{
	antialias = !antialias;
}

static void set_graphics_mode(HICommand *command, void *data)
{
	/* Index in graphics_modes[] */
	UInt32 op;
	GetMenuItemRefCon(command->menu.menuRef, command->menu.menuItemIndex, &op);

	if (graf_mode != op)
		graphics_aux(op);
}

static void set_tile_width(HICommand *command, void *data)
{
	UInt32 op;
	GetMenuItemRefCon(command->menu.menuRef, command->menu.menuItemIndex, &op);
	assert(op != 0);

	tile_width = op;
}

static void set_tile_height(HICommand *command, void *data)
{
	UInt32 op;
	GetMenuItemRefCon(command->menu.menuRef, command->menu.menuItemIndex, &op);
	assert(op != 0);

	tile_height = op;
}

static void reset_wid_hgt(HICommand *command, void *data)
{
	term_data *td = Term->data;
	assert(td);

	td->tile_wid = td->font_wid;
	td->tile_hgt = td->font_hgt;
}

static void seek_graphics_size(term_data *td, int seek_wid, int seek_hgt)
{
	td->tile_wid = td->font_wid;
	td->tile_hgt = td->font_hgt;

	tile_width = MAX(seek_wid / td->font_wid, 1);
	tile_height = MAX(seek_hgt / td->font_hgt, 1);

	td->tile_wid += (seek_wid - (tile_width * td->font_wid)) / tile_width;
	td->tile_hgt += (seek_hgt - (tile_height * td->font_hgt)) / tile_height;
}

static void set_nice_graphics_fit(HICommand *command, void *data)
{
	term_data *td = Term->data;
	assert(td);

	seek_graphics_size(td, graf_width, graf_height);
}

static void set_nice_graphics_square(HICommand *command, void *data)
{
	term_data *td = Term->data;
	assert(td);

	int max_dimension = MAX(td->font_wid, td->font_hgt);
	seek_graphics_size(td, max_dimension, max_dimension);
}

static OSStatus ToggleCommand(EventHandlerCallRef inCallRef, EventRef inEvent,
		void *inUserData)
{
	HICommand command;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof(command), NULL, &command);

	for (size_t i = 0; i < N_ELEMENTS(menu_commands); i++) {
		if (command.commandID != menu_commands[i].id)
			continue;

		menu_commands[i].handler(&command, menu_commands[i].data);

		if (menu_commands[i].refresh == true) {
			RevalidateGraphics(&data[0], FALSE);
			Term_key_push(KTRL('R'));
		}

		return noErr;
	}

	return eventNotHandledErr;
}


static void FontChanged(UInt32 fontID, float size)
{
	if (size < MIN_FONT || fontID == 0)
		return;

	ATSUStyle fontStyle;

	term_data *td = (term_data *) GetWRefCon(fontInfo.focus);
	assert(td);

	/* No change. */
	if (td->font_id == fontID && td->font_size == size)
		return;

	const ATSUAttributeTag tags[] = {kATSUFontTag, kATSUSizeTag};


	Fixed fsize = (Fixed)(size*(1<<16));
	const ByteCount sizes[] = {sizeof(fontID), sizeof(fsize)};
	void * values[] = { &fontID, &fsize };

	ATSUCreateStyle(&fontStyle);
	ATSUSetAttributes(fontStyle, 2, tags, sizes, values);

	/* Reject italics &c */
	const ATSUAttributeTag badtags[] = {kATSUQDItalicTag, 
										kATSUQDUnderlineTag,
										kATSUQDCondensedTag };

	for (size_t i = 0; i < N_ELEMENTS(badtags); i++)
	{
		bool ital = false;
		ByteCount ssize = sizeof(ital);
		ATSUGetAttribute(fontStyle, badtags[i], ssize, &ital, 0);
		if(ital) {
			ATSUDisposeStyle(fontStyle);
			return;
		}
	}

	ATSUDisposeStyle(fontStyle);

	td->font_id = fontID;
	td->font_size = size;
	RevalidateGraphics(td, TRUE);
}

/*
 * Bookkeeping for font-related events.
 */
static OSStatus FontCommand(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	int class = GetEventClass(inEvent);
	int type = GetEventKind(inEvent);

	if(class == 'font' && type == kEventFontSelection) {
		UInt32 fid = 0;
		Fixed size = 0;
		(void) GetEventParameter (inEvent, kEventParamATSUFontSize,
							typeATSUSize, NULL, sizeof (size), NULL, &size);

		(void) GetEventParameter (inEvent, kEventParamATSUFontID,
							typeATSUFontID, NULL, sizeof(fid), NULL, &fid);

		if(size > 32*(1<<16)) size = 32*(1<<16);
		float fsize = 1.0*size/(1<<16);
		FontChanged(fid, fsize);
		return noErr;
	}
	else if(class == 'appl' && type == kEventAppActiveWindowChanged)
	{
		WindowRef w = 0;
		GetEventParameter(inEvent, kEventParamCurrentWindow, typeWindowRef,
							NULL, sizeof(w), NULL,  &w);
		if(!GetWRefCon(w)) { /*  Window is Font Panel. */
			w = 0;
			GetEventParameter(inEvent, kEventParamPreviousWindow,
								typeWindowRef, NULL, sizeof(w), NULL,  &w);
		}
		if(w) fontInfo.focus = w; 
		return noErr;
	}
	else if(class == 'font' && type == kEventFontPanelClosed) {
		SetMenuItemTextWithCFString(GetMenuHandle(kFontMenu), kFonts, CFSTR("Show Fonts"));
		return noErr;
	}

	return eventNotHandledErr;
}

static OSStatus MouseCommand (EventHandlerCallRef inCallRef, EventRef inEvent,
		void *inUserData)
{
	WindowRef w = 0;
	GetEventParameter(inEvent, kEventParamWindowRef, typeWindowRef, 
						NULL, sizeof(w), NULL, &w);

	/* Relevant "term_data" */
	term_data *td = (term_data *) GetWRefCon(w);
	if(&data[0] != td)
		return eventNotHandledErr;

	short button = 0;
	GetEventParameter(inEvent, kEventParamMouseButton, typeMouseButton,
						NULL, sizeof(button), NULL, &button);
	UInt32 modifiers = 0;
	GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL,
						sizeof(modifiers), NULL, &modifiers);
	HIPoint p = {0, 0};
	GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL,
						sizeof(p),  NULL, &p);
	if(button == -1)
		button = 1;

	if(button == 1 && modifiers & cmdKey) 
		button = 2;
	else if(button == 1 && modifiers & shiftKey)
		button = 3;

	/* X coordinate relative to left side of window exclusive of border. */
	p.x -= (BORDER_WID+td->r.left);
	/* Y coordinate relative to top of window content region. */
	p.y -= td->r.top;

	Term_mousepress(p.x/td->tile_wid, p.y/td->tile_hgt, button);

	return noErr;
}

static OSStatus KeyboardCommand ( EventHandlerCallRef inCallRef,
	EventRef inEvent, void *inUserData )
{
	/* Don't handle keyboard events in open/save dialogs, to prevent a 10.4 keyboard interaction bug */
	UInt32 windowClass;
	GetWindowClass(GetUserFocusWindow(), &windowClass);
	if (windowClass == kMovableModalWindowClass)
		return eventNotHandledErr;

	UInt32 evt_mods;
	UInt32 evt_keycode;

	/* Get various aspects of the keycode */
	GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL,
			sizeof(UInt32), NULL, &evt_mods);
	GetEventParameter(inEvent, kEventParamKeyCode, typeUInt32, NULL,
			sizeof(UInt32), NULL, &evt_keycode);

	/* Extract some modifiers */
	bool mc = (evt_mods & controlKey) ? TRUE : FALSE;
	bool ms = (evt_mods & shiftKey) ? TRUE : FALSE;
	bool mo = (evt_mods & optionKey) ? TRUE : FALSE;
	bool mx = (evt_mods & cmdKey) ? TRUE : FALSE;
	bool kp = FALSE;
	byte mods = (mo ? KC_MOD_ALT : 0) | (mx ? KC_MOD_META : 0);

	keycode_t ch = 0;

	/* Command + "normal key" -> menu action */
	if (mx && evt_keycode < 64)
		return eventNotHandledErr;

	/* Hide the mouse pointer */
	hibernate();
	ObscureCursor();

	/* see http://www.classicteck.com/rbarticles/mackeyboard.php */
	switch (evt_keycode) {
		/* top number keys */
		case 18: if (!ms || mo) ch = '1'; break;
		case 19: if (!ms || mo) ch = '2'; break;
		case 20: if (!ms || mo) ch = '3'; break;
		case 21: if (!ms || mo) ch = '4'; break;
		case 23: if (!ms || mo) ch = '5'; break;
		case 22: if (!ms || mo) ch = '6'; break;
		case 26: if (!ms || mo) ch = '7'; break;
		case 28: if (!ms || mo) ch = '8'; break;
		case 25: if (!ms || mo) ch = '9'; break;
		case 29: if (!ms || mo) ch = '0'; break;

		/* keypad keys */
		case 65: ch = '.'; kp = TRUE; break;
		case 67: ch = '*'; kp = TRUE; break;
		case 69: ch = '+'; kp = TRUE; break;
		case 75: ch = '/'; kp = TRUE; break;
		case 76: ch = '\n'; kp = TRUE; break;
		case 78: ch = '-'; kp = TRUE; break;
		case 81: ch = '='; kp = TRUE; break;
		case 82: ch = '0'; kp = TRUE; break;
		case 83: ch = '1'; kp = TRUE; break;
		case 84: ch = '2'; kp = TRUE; break;
		case 85: ch = '3'; kp = TRUE; break;
		case 86: ch = '4'; kp = TRUE; break;
		case 87: ch = '5'; kp = TRUE; break;
		case 88: ch = '6'; kp = TRUE; break;
		case 89: ch = '7'; kp = TRUE; break;
		case 91: ch = '8'; kp = TRUE; break;
		case 92: ch = '9'; kp = TRUE; break;

		/* main keyboard but deal with here */
		case 48: ch = KC_TAB; break;
		case 36: ch = KC_RETURN; break;
		case 51: ch = KC_BACKSPACE; break;

		/* middle bit */
		case 114: ch = KC_HELP; break;
		case 115: ch = KC_HOME; break;
		case 116: ch = KC_PGUP; break;
		case 117: ch = KC_DELETE; break;
		case 119: ch = KC_END; break;
		case 121: ch = KC_PGDOWN; break;

		case 123: ch = ARROW_LEFT; break;
		case 124: ch = ARROW_RIGHT; break;
		case 125: ch = ARROW_DOWN; break;
		case 126: ch = ARROW_UP; break;

		/* function keys */
		case 122: ch = KC_F1; break;
		case 120: ch = KC_F2; break;
		case 99: ch = KC_F3; break;
		case 118: ch = KC_F4; break;
		case 96: ch = KC_F5; break;
		case 97: ch = KC_F6; break;
		case 98: ch = KC_F7; break;
		case 100: ch = KC_F8; break;
		case 101: ch = KC_F9; break;
		case 109: ch = KC_F10; break;
		case 103: ch = KC_F11; break;
		case 111: ch = KC_F12; break;
		case 105: ch = KC_F13; break;
		case 107: ch = KC_F14; break;
		case 113: ch = KC_F15; break;
	}

	if (ch) {
		mods |= (mc ? KC_MOD_CONTROL : 0) | (ms ? KC_MOD_SHIFT : 0) |
				(kp ? KC_MOD_KEYPAD : 0);

		Term_keypress(ch, mods);
	} else if (evt_keycode < 64) {
		/* Keycodes under 64 = main part of the keyboard, printables (mostly) */
		char ch;
		GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar, NULL,
				sizeof(char), NULL, &ch);

		if (mc && MODS_INCLUDE_CONTROL(ch)) mods |= KC_MOD_CONTROL;
		if (ms && MODS_INCLUDE_SHIFT(ch)) mods |= KC_MOD_SHIFT;

		Term_keypress(ch, mods);
	}

	return noErr;
}

/* About angband... */
static OSStatus AboutCommand(EventHandlerCallRef inCallRef, EventRef inEvent,
	void *inUserData )
{
	HICommand command;
	command.commandID = 0;
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(command), NULL, &command);

	if(command.commandID != 'abou')
		return eventNotHandledErr;

	/* Move it to the middle of the screen */
	RepositionWindow(aboutDialog,  NULL, kWindowCenterOnMainScreen);

	/* Show the dialog */
	TransitionWindow(aboutDialog,
		kWindowZoomTransitionEffect,
		kWindowShowTransitionAction,
		NULL);

	/* wait for user input */
	for(;;) {
		EventTargetRef target = GetEventDispatcherTarget();
		EventRef event;
		OSStatus err = ReceiveNextEvent(0, 0, kEventDurationForever, true, &event);
		EventClass evc = GetEventClass(event);
		EventType evt = GetEventKind(event);
		if(err == noErr) {
			SendEventToEventTarget (event, target);
			ReleaseEvent(event);
		}
		if(evc == 'keyb' || (evc == 'mous' && evt == kEventMouseDown))
			break;
	}

	/* Hide the dialogue */
	TransitionWindow(aboutDialog,
		kWindowZoomTransitionEffect,
		kWindowHideTransitionAction,
		NULL);

	return noErr;
}

static OSStatus ResumeCommand (EventHandlerCallRef inCallRef,
								EventRef inEvent, void *inUserData )
{
	term_data *td;

	hibernate();
	Cursor tempCursor;
	SetCursor(GetQDGlobalsArrow(&tempCursor));

	/* Redraw all visible terms */
	for (int i = 0; i < MAX_TERM_DATA; i++ )
	{
		/* Obtain */
		td = &data[i];

		/* Redraw if mapped */
		if (td->mapped)
			term_data_redraw(td);
	}

	return noErr;
}

static OSErr AEH_Open(const AppleEvent *theAppleEvent, AppleEvent* reply,
	SInt32 handlerRefCon)
{
	FSSpec myFSS;
	AEDescList docList;
	long fileindex;
	long filecount;
	OSErr err;
	FInfo myFileInfo;

	/* If a game is in progress, do not proceed */
	if (cmd.command != CMD_NULL) return noErr;
	
	/* Put the direct parameter (a descriptor list) into a docList */
	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err) return err;
	
	err = AECountItems(&docList, &filecount);
	if (err) return err;

	/* Only open one file, but check for the first valid file in the list */
	for (fileindex = 1; fileindex <= filecount; fileindex++)
	{
		err = AEGetNthPtr(&docList, fileindex, typeFSS, NULL, NULL, &myFSS, sizeof(myFSS), NULL);
		if (err) continue;
		
		err = FSpGetFInfo(&myFSS, &myFileInfo);
		if (err) continue;
		
		if (myFileInfo.fdType == 'SAVE')
		{
			
			/* Extract the filename and delay the open */
			(void)spec_to_path(&myFSS, savefile, sizeof(savefile));
			cmd.command = CMD_LOADFILE;

			break;
		}
	}

	/* Dispose */
	err = AEDisposeDesc(&docList);

	/* Success */
	return noErr;
}


/*
 * Handle quit_when_ready, by Peter Ammon,
 * slightly modified to check inkey_flag.
 */
static void quit_calmly(void)
{
	/* Quit immediately if game's not started */
	if (cmd.command == CMD_NULL || !character_generated) quit(NULL);

	/* Save the game and Quit (if it's safe) */
	if (inkey_flag)
	{
		/* Hack -- Forget messages */
		msg_flag = FALSE;

		/* Save the game */
#ifndef ZANG_AUTO_SAVE
		save_game();
#else
		do_cmd_save_game(FALSE);
#endif /* !ZANG_AUTO_SAVE */

		/* Quit */
		quit(NULL);
	}

	/* Wait until inkey_flag is set */
}


/*
 * Macintosh modifiers (event.modifier & ccc):
 *   cmdKey, optionKey, shiftKey, alphaLock, controlKey
 *
 *
 * Macintosh Keycodes (0-63 normal, 64-95 keypad, 96-127 extra):
 *
 * Return:36
 * Delete:51
 *
 * Period:65
 * Star:67
 * Plus:69
 * Clear:71
 * Slash:75
 * Enter:76
 * Minus:78
 * Equal:81
 * 0-7:82-89
 * 8-9:91-92
 *
 * backslash/vertical bar (Japanese keyboard):93
 *
 * F5: 96
 * F6: 97
 * F7: 98
 * F3:99
 * F8:100
 * F10:101
 * F11:103
 * F13:105
 * F14:107
 * F9:109
 * F12:111
 * F15:113
 * Help:114
 * Home:115
 * PgUp:116
 * Del:117
 * F4: 118
 * End:119
 * F2:120
 * PgDn:121
 * F1:122
 * Lt:123
 * Rt:124
 * Dn:125
 * Up:126
 */


/*
 * optimize non-blocking calls to "CheckEvents()"
 * idea from "maarten hazewinkel <mmhazewi@cs.ruu.nl>"
 *
 * was: 6. the value of one (~ 60 fps) seems to work better with the borg,
 * and so should be for other cpu-intensive features like the autoroller.
 */
#define event_ticks 1


/*
 * check for events, return true if we process any
 */
static bool CheckEvents(int wait)
{
	static EventTimeout lastticks;
	EventTimeout curticks;
	/* access the clock */
	curticks = TickCount();
	EventTimeout sleep_ticks;
	EventRef event;
	OSStatus err;

	/* hack -- allow efficient checking for non-pending events */
	if ((wait == CHECK_EVENTS_NO_WAIT) &&
		(curticks < lastticks + event_ticks)) return (false);

	/* timestamp last check */
	lastticks = curticks;

	/* handles the quit_when_ready flag */
	if (quit_when_ready) quit_calmly();

	/* blocking call to waitnextevent - should use max_int xxx xxx */
	if (wait == CHECK_EVENTS_WAIT) sleep_ticks = kEventDurationForever;

	/* non-blocking */
	else sleep_ticks = 0l;

	EventTargetRef target = GetEventDispatcherTarget();
	/* get an event (or null)  */
	do {
		err = ReceiveNextEvent(0, 0, wait ? kEventDurationForever : 0, true, &event);
		if(show_events) {
			EventClass evc = GetEventClass(event);
			EventType evt = GetEventKind(event);
			if(evc != 'mous' || evt != kEventMouseDragged )
				printf("%d (%4s) %d\n", (int)evc, (char*)&evc, (int)evt);
		}
		if(err == noErr) {
			err = SendEventToEventTarget (event, target);
			ReleaseEvent(event);
		}
		else if(err != eventNotHandledErr && sleep_ticks >= 0) {
			curticks = TickCount();
			sleep_ticks -= curticks - lastticks;
			lastticks = curticks;
			if(sleep_ticks <= 0) return false;
		}
	} while(err != eventNotHandledErr && err); /* DurationForever is -1 */
	return true;
}


/*** Some Hooks for various routines ***/



/*
 * Hook to tell the user something important
 */
static void hook_plog(const char *str)
{
	/* Warning message */
	mac_warning(str);
}


/*
 * Hook to tell the user something, and then quit
 */
static void hook_quit(const char *str)
{
	/* Warning if needed */
	if (str) mac_warning(str);

	/* Update the Recent Items list - inserts newly created characters */
	if (savefile[0])
		updateRecentItems(savefile);

	/* Dispose of graphic tiles */
	if(frame.image)
		graphics_nuke();

	/* Write a preference file */
	if (initialized) save_pref_file();

	/* All done */
	ExitToShell();
}



/*** Main program ***/


/*
 * Initialize and verify file and dir paths
 *
 */
static void init_paths(void)
{
	char path[1024];

	/* Hook in to the file_open routine */
	file_open_hook = osx_file_open_hook;

	/* Default to the "lib" folder with the application */
	if (locate_lib(path, sizeof(path)) == NULL)
	{
		quit("unable to find 'lib' dir");
	}

	/* Prepare the paths */
	init_file_paths(path, path, path);

	/* Create any missing directories */
	create_needed_dirs();

	/* Build the filename */
	path_build(path, sizeof(path), ANGBAND_DIR_FILE, "news.txt");

	/* Attempt to open and close that file */
	if (!file_exists(path))
	{
		/* Warning */
		plog_fmt("Unable to open the '%s' file.", path);
		quit("The Angband 'lib' folder is probably missing or misplaced.");
	}
}

/*
 * Macintosh Main loop
 */
int main(void)
{
	/* Initialise the cursor and turn it into an "arrow" */
	InitCursor();

	/* 
	 * Remember Mac OS version, in case we have to cope with version-specific
	 * problems
	 */
	(void)Gestalt(gestaltSystemVersion, &mac_os_version);

	/* Initiliases Cocoa */
	NSApplicationLoad();
	
	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;

	/* Initialize colors */
	update_color_info();

	/* Show the "watch" cursor */
	SetCursor(*(GetCursor(watchCursor)));

	/* Prepare the menubar */
	init_menubar();

	/* Ensure that the recent items array is always an array and start with an empty menu */
	recentItemsArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	redrawRecentItemsMenu();

	/* Initialize */
	init_paths();

	/* Prepare the windows */
	init_windows();

	/* Install the 'Apple Event' handler hook (ignore error codes) */
	(void)AEInstallEventHandler(
		kCoreEventClass,
		kAEOpenDocuments,
		NewAEEventHandlerUPP(AEH_Open),
		0L,
		FALSE);

	/* Install menu and application handlers */
	install_handlers(0);

	/* Reset the cursor */
	Cursor tempCursor;
	SetCursor(GetQDGlobalsArrow(&tempCursor));


	/* Quicktime -- Load sound effect resources */

	load_sounds();

	/* Note the "system" */
	ANGBAND_SYS = "mac";

	/* Validate the contents of the main window */
	validate_main_window();

	/* Flush input commands from the event queue */
	FlushEventsMatchingListFromQueue(GetMainEventQueue(),
		N_ELEMENTS(input_event_types), input_event_types);

	/* Set command hook */ 
	cmd_get_hook = crb_get_cmd; 

	/* Set up the display handlers and things. */
	init_display();

	if(graf_mode) graphics_aux(graf_mode);

	/* We are now initialized */
	initialized = TRUE;

	validate_menus();

	/* Start playing! */
	EventRef newGameEvent = nil;
	CreateEvent ( nil, 'Play', 'Band', GetCurrentEventTime(),
									kEventAttributeNone, &newGameEvent ); 
	PostEventToQueue(GetMainEventQueue(), newGameEvent, kEventPriorityLow);

	RunApplicationEventLoop();

	/* Quit */
	quit(NULL);

	/* Since it's an int function */
	return (0);
}

#endif /* MACH_O_CARBON */
