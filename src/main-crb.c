/* File: main-crb.c */

/*
 * Copyright (c) 1997-2006 Ben Harrison, Keith Randall, Peter Ammon,
 * Ron Anderson, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/*
 * Update 2006 - pete mack
 * This update works exclusively with Apple computers running OS X,
 * versions 10.3 and later.  The still-usable old version uses QuickDraw
 * Graphics, "deprecated" as of 10.4
 *
 * Support for legacy Macs and pre-10.3 OSX has been removed.
 * The old carbon code is still available in main-crb.c, 
 * Angband version 3.0.6; pre-carboniferous code in angband main-mac.c
 */

/*
 * Use 
 * make -f Makefile.osx 
 * You must have XCode installed to compile.
 * 
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 * Some code adapted from "MacAngband 2.6.1" by Keith Randall
 * Initial PowerMac port by Maarten Hazewinkel (mmhazewi@cs.ruu.nl).
 * Most Apple Event code provided by Steve Linberg (slinberg@crocker.com).
 * Carbon code adapted from works by Peter Ammon and Ron Anderson.
 * Much modernization and graphics/sound improvement done by pelpel.
 *
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
 * For the best compatibility with the Classic ports and my PEF Carbon
 * ports, my_fopen, fd_make and fd_open [in util.c] should call
 *   (void)fsetfileinfo(buf, _fcreator, _ftype);
 * when a file is successfully opened.  Or you'll see odd icons for some files
 * in the lib folder.  In order to do so, extern.h should contain these lines,
 *
 *   extern int fsetfileinfo(char *path, u32b fcreator, u32b ftype);
 *   extern u32b _fcreator;
 *   extern u32b _ftype;
 * And enable the four FILE_TYPE macros in h-config.h for defined(MACH_O_CARBON)
 *
 * All calls to my_fopen should be preceded by the appropriate FILE_TYPE(xxx),
 * especially those in file.c and save.c
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
 *
 * Sound (.wav) files should be stored in .../lib/xtra/sound
 * Graphics files should be stored in .../lib/xtra/images
 * as is standard on other unix builds.
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

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include "angband.h"

#include "osx/osx_tables.h"


 /*
 * #define ANGBAND_CREATOR four letter code for your variant, if any.
 * or use the default one. (This is used to specify the standard program
 * for opening data files.
 *
 */

/* Default creator signature */
#ifndef ANGBAND_CREATOR
# define ANGBAND_CREATOR 'A271'
#endif

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
static int graf_mode = 0;

/* Tile dimensions of the current graphics mode */
static int graf_height = 0;
static int graf_width = 0;

/*
 * Creator signature and file type - Didn't I say that I abhor file name
 * extentions?  Names and metadata are entirely different set of notions.
 */
OSType _fcreator;
OSType _ftype;

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
	
	Rect r;			// Absolute bounds of window.
	CGRect bounds;  // Relative bounds of border-clipped canvas.

	int spacing;  	// Text padding (in pixels) for tiling wider than text

	Str15 title;	// Window title.

	s16b mapped;	// Active state.

	s16b rows;		// rows in picture
	s16b cols;		// columns in picture.

	ATSUFontID font_id;
	float font_size;	// Scaled ATSU font size.

	s16b font_wid;
	s16b font_hgt;

	s16b tile_wid;
	s16b tile_hgt;

	s16b size_wid;	// Window size in x.
	s16b size_hgt;	// Window size in y.
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
	float font_wid;  // max character advance.
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
static OSStatus RevalidateGraphics(term_data *td, EventRef inEvent);
static char *locate_lib(char *buf, size_t size);
static void graphics_aux(int op);
static void Term_wipe_mac_aux(int x, int y, int n);
inline static void term_data_color(int a);
static void install_handlers(WindowRef w);
static void graphics_tiles_nuke(void);

/*
 * Available values for 'wait'
 */

#define CHECK_EVENTS_DRAIN -1
#define CHECK_EVENTS_NO_WAIT	0
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


/*
 * Hack -- game in progress
 */
static bool game_in_progress = FALSE;


/*
 * Indicate if the user chooses "new" to start a game
 */
static bool new_game = FALSE;


/* Out-of-band color identifiers */
/* True black (TERM_BLACK may be altered) */
#define COLOR_BLACK		(256)
/* No current color */
#define COLOR_INVALID	(-1)


/*
 * Keeps track of who owns the QD CGContext, and it's current state.
 * Always use this to change the active graphics port.
 * (It is a parallel structure to the Term variable.)
 */
struct ActivePort {
	WindowRef		active;
	CGContextRef	ctx;
	int		  		color;   // Current fill colorcode
	// CGColorRef		color_info[256+1];
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
		for(int i = 0; i < N_ELEMENTS(menuRefs); i++) {
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
void fsetfileinfo(cptr pathname, OSType fcreator, OSType ftype)
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

		term_data *td = (term_data*)GetWRefCon(w);

		/* Start queueing graphics events. */
		/* and set up the context */
		QDBeginCGContext(GetWindowPort(w), &focus.ctx);

		// Shift the origin to inside the border, and use inverted y axis.
		CGAffineTransform m;
		m = CGAffineTransformMake(BORDER_WID, 0, 0, -1,
					 			BORDER_WID, BORDER_WID+td->bounds.size.height);

		CGContextConcatCTM (focus.ctx, m); 
		CGContextClipToRect(focus.ctx, td->bounds);

		CGContextSetShouldAntialias (focus.ctx, antialias); 
		CGContextSetInterpolationQuality(focus.ctx, kCGInterpolationNone);

		// Invert the text so it's no longer mirrored in y.
		// Origin is at still at the bottom of the line, so the ascent must
		// be subtracted at display-time. (Not added, because the coordinate
		// system is also flipped.)
		m = CGAffineTransformMake(1, 0, 0, -1, 0, 0);
		CGContextSetTextMatrix(focus.ctx, m);

		// I don't know why this doesn't work.
		// CGContextSetFont(focus.ctx, td->ginfo->fontRef);
		// CGContextSetFontSize(focus.ctx, td->font_size);
		// HACK: use full postscript name.
		CGContextSelectFont(focus.ctx, td->ginfo->psname, td->font_size,
														kCGEncodingMacRoman); 

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
static void mac_warning(cptr warning)
{
	CFStringRef msg;
	msg = CFStringCreateWithCString(NULL, warning, kTextEncodingUS_ASCII);

	DialogRef dlg = 0;
	CreateStandardAlert(kAlertCautionAlert, msg, CFSTR(""), NULL, &dlg);

	DialogItemIndex itemIndex;
	RunStandardAlert(dlg, NULL, &itemIndex);
	// DisposeDialog(dlg); // NO!
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
	else if (focus.color != a)  // Assumes the window exists.
	{
		focus.color = a;
		// CGContextSetFillColorWithColor(focus.ctx, focus.color_info[a]);
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

	// One extra, so a term_data can fetch before it frees.
			
	for(info = glyph_data; info <= glyph_data+MAX_TERM_DATA; info++)
	{
		if(info->font_id == fid && info->font_size == size)
		{
			info->refcount++;
			return info;
		}
	}

	// One is always available.
	info = glyph_data;
	for(int c = 0; info->refcount != 0; info++, c++)
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

	// Dummy text, required to initialize run style.
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
	// Is font mono-space?
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
		// SetRunStyle doesn't honor runs in layouts of size 0.
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

		/* Enforce minimal size */
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;

	}

	/* Information windows can be much smaller */
	else
	{
		if (td->cols < 5) td->cols = 10;
		if (td->rows < 10) td->rows = 5;
	}

	/* Enforce maximal sizes */
	if (td->cols > 255) td->cols = 255;
	if (td->rows > 255) td->rows = 255;

	bool dirty = false;
	/* Minimal tile size */
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


	/* Get current screen */
	hibernate();
	BitMap tScreen;
	(void)GetQDGlobalsScreenBits(&tScreen);
	
	/* Verify the bottom */
	if (td->r.top > tScreen.bounds.bottom - td->size_hgt)
	{
		td->r.top = tScreen.bounds.bottom - td->size_hgt;
	}

	/* Verify the top */
	if (td->r.top < tScreen.bounds.top + GetMBarHeight())
	{
		td->r.top = tScreen.bounds.top + GetMBarHeight();
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
	else if (use_graphics && (td == &data[0]))
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
	// Invalidate the current CGContext.
	hibernate();
	/*
	 * Actually resize the window
	 * 
	 * ResizeWindow is the preferred API call, but it cannot
	 * be used here.
	 */
	
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);
	// Cheat a little -- can't use the active view to redraw its own border.
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
	// The reference image at the original scale.
	CGImageRef image;

 	// Numbers of rows and columns in a tileset,
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
	WindowRef focus; // The most recently focused window. (NOT the fontpanel.)
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
	// Don't display images upside down. (Works like SetTextMatrix)
	HIViewDrawCGImage (context, &drawRect, image); 
	// CGContextDrawImage (context, &drawRect, image);
	CGContextRestoreGState (context);
}

/*
 * Copy an image with tiles of size src into a new one with
 * tiles of size dst.  Interpolation will not cross tile borders.
 */
static CGImageRef GetTileImage(int row, int col, bool has_alpha) 
{
	// Cache hit.
	assert(col < frame.cols && row < frame.rows);
	if(frame.tile_images[row*frame.cols + col] != 0) {
		return frame.tile_images[row*frame.cols+col];
	}

	term_data *td = &data[0];

	size_t tile_wid = td->tile_wid *(1+use_bigtile);
	size_t nbytes = (td->tile_hgt * tile_wid) * 4;
	void *data = calloc(1, nbytes);

	CGContextRef map;
	map = CGBitmapContextCreate(data, tile_wid, td->tile_hgt,
								8,
								nbytes/td->tile_hgt,
								CGColorSpaceCreateDeviceRGB(),
								kCGImageAlphaPremultipliedLast);

	CGAffineTransform m = {1, 0, 0, -1, 0, td->tile_hgt};
	CGContextConcatCTM(map, m); 

	// Attempt to avoid interpolation across cell boundaries by clipping.
	// It may be that we need a clip image first.
	CGRect src_r = {{ graf_width*col, graf_height*row },
						{ graf_width, graf_height }};
	CGRect dst_r = {{ 0, 0 }, { tile_wid, td->tile_hgt }};
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
	// free(data); // Duplicate free?

	frame.tile_images[row*frame.cols+col] = timg;
	return timg; 
}

static void DrawTile(int x, int y, byte a, byte c, byte ta, byte tc)
{
	term_data *td = (term_data*) Term->data;

	int tile_wid = (1+use_bigtile)*td->tile_wid;
	CGRect dst_r = {{x*td->tile_wid, y*td->tile_hgt}, {tile_wid, td->tile_hgt}};
	CGRect src_r = {{0, 0}, {tile_wid, td->tile_hgt}};

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
		// This doesn't preserve alpha. Don't know how to fix.
		image = GetTileImage(a, c, true);
		DrawSubimage(focus.ctx, dst_r, image, src_r);
		// Use the original source instead. This is SLOW!
		//src_r = (CGRect) {{ graf_width*c, graf_height*a },
		//							{ graf_width, graf_height }};
		// DrawSubimage(focus.ctx, dst_r, frame.image, src_r);
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
	// Only round once.
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
		CGContextShowTextAtPoint ( focus.ctx, xp, yp, text, n ); 
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
		return -1; // E_NO_MEM
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
		for(int i = frame.rows*frame.cols; --i > 0; ) {
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
	FlushEventQueue(GetMainEventQueue());

	/* Success */
	return (0);
}


/*
 * How many sound channels will be pooled
 */
#define MAX_CHANNELS		8

/*
 * A pool of sound channels
 */
static SndChannelPtr channels[MAX_CHANNELS];

/*
 * Status of the channel pool
 */
static bool channel_initialised = FALSE;

/*
 * Data handles containing sound samples
 */
static SndListHandle samples[MSG_MAX];

/*
 * Reference counts of sound samples
 */
static SInt16 sample_refs[MSG_MAX];

#define SOUND_VOLUME_MIN	0	/* Default minimum sound volume */
#define SOUND_VOLUME_MAX	255	/* Default maximum sound volume */
#define VOLUME_MIN			0	/* Minimum sound volume in % */
#define VOLUME_MAX			100	/* Maximum sound volume in % */
#define VOLUME_INC			5	/* Increment sound volume in % */

/*
 * I'm just too lazy to write a panel for this XXX XXX
 */
static SInt16 sound_volume = SOUND_VOLUME_MAX;

/*
 * QuickTime sound, by Ron Anderson
 *
 * I didn't choose to use Windows-style .ini files (Ron wrote a parser
 * for it, but...), nor did I use lib/xtra directory, hoping someone
 * would code plist-based configuration code in the future -- pelpel
 */

/*
 * (QuickTime)
 * Load sound effects from data-fork resources.  They are wav files
 * with the same names as angband_sound_name[] (variable.c)
 *
 * Globals referenced: angband_sound_name[]
 * Globals updated: samples[] (they can be *huge*)
 */
static void load_sounds(void)
{
	/* Start QuickTime */
	OSErr err = EnterMovies();

	/* Error */
	if (err != noErr) return;

	/*
	 * This loop may take a while depending on the count and size of samples
	 * to load.
	 *
	 * We should use a progress dialog for this.
	 */
	char path[1024];
	locate_lib(path, sizeof(path));
	char *tail = path+strlen(path);
	strncpy(tail, "/xtra/sound/", path+1024-tail);
	tail = tail+strlen(tail);
	for (int i = 1; i < MSG_MAX; i++)
	{
		/* Apple APIs always give me headache :( */
		/* Me too :( */
		FSSpec spec;
		SInt16 file_id;
		SInt16 res_id;
		Str255 movie_name;
		Movie movie;
		Track track;
		Handle h;

		sprintf(tail, "%s.wav", angband_sound_name[i]);
		err = path_to_spec(path, &spec);
		if(err != noErr) continue;

		/* Open the sound file */
		err = OpenMovieFile(&spec, &file_id, fsRdPerm);

		/* Error */
		if (err != noErr) continue;

		/* Create Movie from the file */
		err = NewMovieFromFile(&movie, file_id, &res_id, movie_name,
			newMovieActive, NULL);

		/* Error */
		if (err != noErr) goto close_file;

		/* Get the first track of the movie */
		track = GetMovieIndTrackType(movie, 1, AudioMediaCharacteristic,
			movieTrackCharacteristic | movieTrackEnabledOnly );

		/* Error */
		if (track == NULL) goto close_movie;

		/* Allocate a handle to store sample */
		h = NewHandle(0);

		/* Error */
		if (h == NULL) goto close_track;

		/* Dump the sample into the handle */
		err = PutMovieIntoTypedHandle(movie, track, soundListRsrc, h, 0,
			GetTrackDuration(track), 0L, NULL);

		/* Success */
		if (err == noErr)
		{
			/* Store the handle in the sample list */
			samples[i] = (SndListHandle)h;
		}

		/* Failure */
		else
		{
			/* Free unused handle */
			DisposeHandle(h);
		}

		/* Free the track */
close_track: DisposeMovieTrack(track);

		/* Free the movie */
close_movie: DisposeMovie(movie);

		/* Close the movie file */
close_file: CloseMovieFile(file_id);
	}

	/* Stop QuickTime */
	ExitMovies();
}

/*
 * Return a handle of 'snd ' resource given Angband sound event number,
 * or NULL if it isn't found.
 *
 * Globals referenced: angband_sound_name[] (variable.c)
 */
static SndListHandle get_sound_resource(int num)
{
	SndListHandle h = samples[num];

	if(++sample_refs[num] > 1) {
		return h;
	}
	if(!h) {
		sample_refs[num]--;
		return 0;
	}
	HLockHi((Handle)h);
	return h;
}

void release_sound_resource(int num)
{
	if(sample_refs[num] == 0)
		return;

	/* Decrease refcount */
	if(--sample_refs[num] > 0)
		return;

	/* We can free it now */
	/* Unlock */
	HUnlock((Handle)samples[num]);

}

/*
 * Clean up sound support - to be called when the game exits.
 *
 * Globals referenced: channels[], samples[], sample_refs[].
 */
static void cleanup_sound(void)
{
	/* No need to clean it up */
	if (!channel_initialised) return;

	/* Dispose channels */
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		/* Drain sound commands and free the channel */
		SndDisposeChannel(channels[i], TRUE);
	}

	/* Free sound data */
	for (int i = 1; i < MSG_MAX; i++)
	{
		while(sample_refs[i] > 0)
		{
			release_sound_resource(i);
		}
	}
}


/*
 * Play sound effects asynchronously -- pelpel
 *
 * I don't believe those who first started using the previous implementations
 * imagined this is *much* more complicated as it may seem.  Anyway, 
 * introduced round-robin scheduling of channels and made it much more
 * paranoid about HLock/HUnlock.
 *
 * XXX XXX de-refcounting, HUnlock and ReleaseResource should be done
 * using channel's callback procedures, which set global flags, and
 * a procedure hooked into CheckEvents does housekeeping.  On the other
 * hand, this lazy reclaiming strategy keeps things simple (no interrupt
 * time code) and provides a sort of cache for sound data.
 *
 * Globals referenced: channel_initialised, channels[], samples[],
 *   sample_refs[].
 * Globals updated: channel_initialised, channels[], sample_refs[].
 */

static void play_sound(int num, SInt16 vol)
{
	OSErr err;
	int prev_num;
	SndListHandle h;
	SndChannelPtr chan;
	SCStatus status;

	static int next_chan;
	static SInt16 channel_occupants[MAX_CHANNELS];
	static SndCommand volume_cmd, quiet_cmd;

	/* Initialise sound channels */
	if (!channel_initialised)
	{
		for (int i = 0; i < MAX_CHANNELS; i++)
		{
			/* Paranoia - Clear occupant table */
			/* channel_occupants[i] = 0; */

			/* Create sound channel for all sounds to play from */
			err = SndNewChannel(&channels[i], sampledSynth, initMono, NULL);

			/* Free channels */
			if(err != noErr) {
				while (--i >= 0)
				{
					SndDisposeChannel(channels[i], TRUE);
				}
	
				/* Notify error */
				plog("Cannot initialise sound channels!");

				/* Cancel request */
				use_sound = arg_sound = FALSE;
	
				/* Failure */
				return;
			}
		}

		/* First channel to use */
		next_chan = 0;

		/* Prepare volume command */
		volume_cmd.cmd = volumeCmd;
		volume_cmd.param1 = 0;
		volume_cmd.param2 = 0;

		/* Prepare quiet command */
		quiet_cmd.cmd = quietCmd;
		quiet_cmd.param1 = 0;
		quiet_cmd.param2 = 0;

		/* Initialisation complete */
		channel_initialised = TRUE;
	}

	/* Paranoia */
	if ((num <= 0) || (num >= MSG_MAX)) return;

	/* Prepare volume command */
	volume_cmd.param2 = ((SInt32)vol << 16) | vol;

	/* Channel to use (round robin) */
	chan = channels[next_chan];

	/* Attempt to get a new sound "resource" */
	h = get_sound_resource(num);
	if (h == NULL) return;

	/* Poll the channel */
	err = SndChannelStatus(chan, sizeof(SCStatus), &status);

	/* It isn't available */
	if ((err != noErr) || status.scChannelBusy)
	{
		/* Shut it down */
		SndDoImmediate(chan, &quiet_cmd);
	}

	/* Process previously played sound */
	if ((prev_num = channel_occupants[next_chan]) != 0)
	{
		release_sound_resource(prev_num);
	}

	/* Remember this sound as the current occupant of the channel */
	channel_occupants[next_chan] = num;

	/* Set up volume for channel */
	SndDoImmediate(chan, &volume_cmd);

	/* Play new sound asynchronously */
	SndPlay(chan, h, TRUE);

	/* Schedule next channel (round robin) */
	next_chan++;
	if (next_chan >= MAX_CHANNELS) next_chan = 0;
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

	install_handlers(td->w);


	/* Fatal error */
	if (err != noErr) ExitToShell();

	/* Set refcon */
	SetWRefCon(td->w, (long)td);

	/* Set window title */
	SetWTitle(td->w, td->title);

	InstallStandardEventHandler(GetWindowEventTarget(td->w));

	/* Apply and Verify */
	term_data_check_font(td);
	term_data_check_size(td);
	term_data_resize(td);

	/* assert(td->mapped) */
	if (td->mapped)
	{
		WindowRef old_win = focus.active;

		TransitionWindow(td->w,
			kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL);

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
 * Unused
 */
static errr Term_user_mac(int c)
{
#pragma unused(c)
	return (0);
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

		/* Make a sound */
		case TERM_XTRA_SOUND:
		{
			/* Play sound */
			play_sound(v, sound_volume);

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

		/* Flush all pending events (if any) */
		case TERM_XTRA_FLUSH:
		{
			FlushEventQueue(GetMainEventQueue());

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
	if(!focus.active) activate(focus.active);

	term_data *td = (term_data*)(Term->data);

	CGContextSaveGState(focus.ctx);

	// Temporarily set stroke color to yellow
	int a = TERM_YELLOW;
	CGContextSetRGBStrokeColor(focus.ctx, focus.color_info[a][0],
							focus.color_info[a][1], focus.color_info[a][2], 1);

	/* Frame the grid, staying within the boundary. */
	int tile_wid = td->tile_wid;
	if(use_bigtile) {
		byte a;
		char c;
		Term_what(x+1,y, &a, &c);
		if(c == (char) 0xff) tile_wid *= 2;
	}

	CGRect r = {{x * td->tile_wid + .5, y * td->tile_hgt + .5 },
							{ tile_wid - 1, td->tile_hgt - 1}};

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
 * Overwrite "n" old characters starting at	(x,y)
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
	if(use_overwrite_hack)
		Term_wipe_mac_aux(x, y, n);

	term_data_color(COLOR_BLACK);

	term_data *td = (term_data*)(Term->data);
	int tile_wid = (1+use_bigtile)*td->tile_wid;
 	CGRect r = {{ x*tile_wid, y*td->tile_hgt },
											{ n*tile_wid, -td->tile_hgt }};
	CGContextFillRect(focus.ctx, r);

	/* Success */
	return (0);
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

		/* Hack -- a filler for double-width tile */
		if(use_bigtile && (a == 255)) continue;

		// TODO: background should be overridden with neutral color
		// if unavailable.
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
	MAKE(td->t, term);

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
	td->t->user_hook = Term_user_mac;
	td->t->xtra_hook = Term_xtra_mac;
	td->t->wipe_hook = Term_wipe_mac;
	td->t->curs_hook = Term_curs_mac;
	td->t->bigcurs_hook = Term_curs_mac;
	td->t->text_hook = Term_text_mac;
	td->t->pict_hook = Term_pict_mac;


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
static void save_pref_short(const char *key, short value)
{
	CFStringRef cf_key;
	CFNumberRef cf_value;

	/* allocate and initialise the key */
	cf_key = CFStringCreateWithCString(NULL, key, kTextEncodingUS_ASCII);

	/* allocate and initialise the value */
	cf_value = CFNumberCreate(NULL, kCFNumberShortType, &value);

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
static bool query_load_pref_short(const char *key, short *vptr)
{
	CFStringRef cf_key;
	CFNumberRef cf_value;

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

	/* Convert the value to short */
	CFNumberGetValue(
		cf_value,
		kCFNumberShortType,
		vptr);

	/* Free CF data */
	CFRelease(cf_value);
	CFRelease(cf_key);

	/* Success */
	return (TRUE);
}


/*
 * Update short data pointed by vptr only if preferences
 * value for key is located.
 */
static void load_pref_short(const char *key, short *vptr)
{
	short tmp;

	if (query_load_pref_short(key, &tmp)) *vptr = tmp;
}


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
	save_pref_short("arg.arg_sound", arg_sound);
/* Tile dimensions of the current graphics mode */

	/* Windows */
	for (int i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		save_pref_short(format("term%d.mapped", i), td->mapped);

		save_pref_short(format("term%d.tile_wid", i), td->tile_wid);
		save_pref_short(format("term%d.tile_hgt", i), td->tile_hgt);

		save_pref_short(format("term%d.cols", i), td->cols);
		save_pref_short(format("term%d.rows", i), td->rows);
		save_pref_short(format("term%d.left", i), td->r.left);
		save_pref_short(format("term%d.top", i), td->r.top);
	}

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
	ok &= query_load_pref_short("version.major", &pref_major);
	ok &= query_load_pref_short("version.minor", &pref_minor);
	ok &= query_load_pref_short("version.patch", &pref_patch);
	ok &= query_load_pref_short("version.extra", &pref_extra);

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

#if 0

	/* Check version */
	if ((pref_major != VERSION_MAJOR) ||
		(pref_minor != VERSION_MINOR) ||
		(pref_patch != VERSION_PATCH) ||
		(pref_extra != VERSION_EXTRA))
	{
		/* Message */
		mac_warning(
			format("Ignoring %d.%d.%d.%d preferences.",
				pref_major, pref_minor, pref_patch, pref_extra));

		/* Ignore */
		return;
	}

#endif

	/* HACK - Check for broken preferences */
	load_pref_short("term0.mapped", &valid);

	/* Ignore broken preferences */
	if (!valid)
	{
		// mac_warning("Ignoring broken preferences.");

		/* Ignore */
		return;
	}

	/* Gfx settings */
	short pref_tmp;

	/* sound */
	if (query_load_pref_short("arg.arg_sound", &pref_tmp))
		arg_sound = pref_tmp;

	/* double-width tiles */
	if (query_load_pref_short("arg.big_tile", &pref_tmp))
	{
		use_bigtile = pref_tmp;
	}


	/* Windows */
	for (int i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		load_pref_short(format("term%d.mapped", i), &td->mapped);

		load_pref_short(format("term%d.tile_wid", i), &td->tile_wid);
		load_pref_short(format("term%d.tile_hgt", i), &td->tile_hgt);

		load_pref_short(format("term%d.cols", i), &td->cols);
		load_pref_short(format("term%d.rows", i), &td->rows);
		load_pref_short(format("term%d.left", i), &td->r.left);
		load_pref_short(format("term%d.top", i), &td->r.top);
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

		/* Obtain title */
		cptr s = angband_term_name[i];

		/* Get length */
		int n = strlen(s);

		/* Maximal length */
		if (n > 15) n = 15;

		/* Copy the title */
		strncpy((char*)(td->title) + 1, s, n);

		/* Save the length */
		td->title[0] = n;

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
	NavTypeList types = {ANGBAND_CREATOR, 1, 1, {'SAVE'}};
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

	(void) CreateWindowFromNib(nib, CFSTR("DLOG:about"), &aboutDialog);

	DisposeNibReference(nib);

	MenuRef m = MyGetMenuHandle(kStyleMenu);
	for(int i = 1; i <= CountMenuItems(m); i++) {
		// Invalid entry
		SetMenuItemRefCon(m, i, -1);
	}
	for(int i = 0; i < N_ELEMENTS(graphics_modes); i++) {
		SetMenuItemRefCon(m, graphics_modes[i].menuItem, i);
	}

	for(int j = kTileWidMenu; j <= kTileHgtMenu; j++) {
		m = MyGetMenuHandle(j);
		for(int i = MIN_FONT; i <= 32; i++) {
			char buf[15];
			/* Tile size */
			strnfmt((char*)buf, 15, "%d", i);
			CFStringRef cfstr = CFStringCreateWithBytes ( NULL, (byte*) buf,
									strlen(buf), kCFStringEncodingASCII, false);
			AppendMenuItemTextWithCFString(m, cfstr, 0, j, NULL);
			SetMenuItemRefCon(m, i-MIN_FONT+1, i);
		}
	}
}

// Install the handlers from the Commands table.
static void install_handlers(WindowRef w)
{
	EventHandlerRef prevRef;
	for(int i = 0; i < N_ELEMENTS(event_defs) ; i++) {
		const CommandDef *def = &event_defs[i];

		/* Install window handlers only for kWINDOW events */
		if((!w) == (def->targetID == kWINDOW))
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

/* This initializes all the menus with values that change unpredictably. */
/* Menus that change rarely are done at the time of change */
static void validate_menus(void)
{
	WindowRef w = FrontWindow();
	term_data *td;
	if(!w || !initialized) return;
	td = (term_data*) GetWRefCon(w);
	if(!td) return;

	term_data *td0 = &data[0];
	struct {
		int menu;				/* Radio-style Menu ID to validate */
		int cur;				/* Value in use (Compare to RefCon) */
		int limit;				/* Constraint value */
		int (*cmp) (int, int);	/* Filter function */
	} funcs [] = {
		{ kTileWidMenu, td0->tile_wid, td0->font_wid, funcGTE },
		{ kTileHgtMenu, td0->tile_hgt, td0->font_hgt, funcGTE },
		{ kStyleMenu, graf_mode, 1, funcConst }
	};

	MenuHandle m;

	if(game_in_progress) {
		EnableAllMenuItems(MyGetMenuHandle(kSpecialMenu));
		EnableAllMenuItems(MyGetMenuHandle(kStyleMenu));
	}

	for(int i = 0; i < N_ELEMENTS(funcs); i++) {
		m = MyGetMenuHandle(funcs[i].menu);
		int n = CountMenuItems(m);
		for(int j = 1; j <= n; j++) {
			UInt32 value;
			GetMenuItemRefCon(m, j, &value);
			CheckMenuItem(m, j, funcs[i].cur == value);
			if(funcs[i].cmp(value, funcs[i].limit)) {
				EnableMenuItem(m, j);
			}
			else {
				DisableMenuItem(m, j);
			}
		}
	}

	m = MyGetMenuHandle(kFileMenu);
	if(inkey_flag && character_generated) {
		EnableMenuItem(MyGetMenuHandle(kFileMenu), kSave);
	}
	else {
		DisableMenuItem(MyGetMenuHandle(kFileMenu), kSave);
	}
	for(int i = kNew; i <= kImport; i++) {
		if(!game_in_progress) 
			EnableMenuItem(MyGetMenuHandle(kFileMenu), i);
		else
			DisableMenuItem(MyGetMenuHandle(kFileMenu), i);
	}

	for(int i = 0; i < N_ELEMENTS(toggle_defs); i++) {
		m = MyGetMenuHandle(toggle_defs[i].menuID);
		CheckMenuItem(m, toggle_defs[i].menuItem, *(toggle_defs[i].var));
	}
}

static OSStatus ValidateMenuCommand(EventHandlerCallRef inCallRef,
									EventRef inEvent, void *inUserData )
{
	validate_menus();
	return noErr;
}



static OSStatus AngbandGame(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	/* Initialize  For quartz, this must be within the message loop.*/
	init_angband();
	// Only enabled options are Fonts, Open/New/Import and Quit. 
	DisableAllMenuItems(MyGetMenuHandle(kTileWidMenu));
	DisableAllMenuItems(MyGetMenuHandle(kTileHgtMenu));
	/* Prompt the user - You may have to change this for some variants */
	prt("[Choose 'New', 'Open' or 'Import' from the 'File' menu]", 23, 11);

	SetFontInfoForSelection(kFontSelectionATSUIType, 0, 0, 0);

	for(int i = kNew; i <= kImport; i++)
		EnableMenuItem(MyGetMenuHandle(kFileMenu), i);

	/* Validate graphics, after bootstrapped opening of terminals */
	for(int i = 0; i < N_ELEMENTS(data); i++) {
		if(data[i].mapped)
			RevalidateGraphics(&data[i], 0);
	}

	/* Flush the prompt */
	Term_fresh();
	Term_flush();

	EventTargetRef target = GetEventDispatcherTarget();
	while(!game_in_progress) {
		// if(event is interesting) break;
		OSStatus err;
		EventRef event;
		err = ReceiveNextEvent(0, 0, kEventDurationForever, true, &event);
		if(err == noErr) {
			SendEventToEventTarget (event, target);
			ReleaseEvent(event);
		}
	}

	play_game(new_game);
	quit(0);
	// Not reached
	return noErr;
}

/*
 *  "New" / "Open" /  "Import"
 */
static OSStatus openGame(int op)
{
	/* Let the player to choose savefile */
	if (op != kNew && 0 == select_savefile(op == kImport))
	{
		// Failed to open
		return noErr;
	}

	/* Wait for a keypress */
	pause_line(Term->hgt - 1);

	/* Game is in progress */
	game_in_progress = TRUE;

	/* Use an existing savefile */
	new_game = (op == kNew);

	return noErr;
}

/* Open Document is only remaining apple event that needs to be handled
   explicitly */
static OSStatus AppleCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	EventRecord aevent;
	(void) AEProcessAppleEvent(&aevent);
	if(open_when_ready) {
		game_in_progress = TRUE;
		new_game = false;
	}
	return noErr;
}


static OSStatus QuitCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	if(!game_in_progress && !character_generated)
		quit(0);	
	else Term_key_push('\030');
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
		if(game_in_progress && character_generated)
			Term_key_push(KTRL('S'));
		break;
	case 'open':
		openGame(command.menu.menuItemIndex);
		break;
	case 'font':
	  {
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

	if(!game_in_progress && !character_generated && td == &data[0])
		quit(0);

	hibernate();

	/* Track the go-away box */
	if (td)
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
	}
	return noErr;
}



static OSStatus ResizeCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	int x, y;
	WindowRef w = 0;

	term_data *td;
	term *old = Term;

	GetEventParameter(inEvent, kEventParamDirectObject,
							typeWindowRef, NULL, sizeof(w), NULL, &w);

	td = (term_data*) GetWRefCon(w);

	/* Oops */
	if (!td) return noErr;

	/* Obtain geometry of resized window */
	
	Rect tmpR;
	GetWindowBounds((WindowRef)td->w, kWindowContentRgn, &tmpR);

	/* Extract the new ClipRect size in pixels */
	y = tmpR.bottom - tmpR.top - BORDER_WID;
	x = tmpR.right - tmpR.left - BORDER_WID * 2;

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

	// Close the old (different size) CGContext
	hibernate();
	/* Resize and Redraw */
	term_data_resize(td);

	// Since we don't know what view needs to be updated, recalculate
	// and redraw them all. (term_data_redraw() is not sufficient)
	Term_key_push(KTRL('R'));
	
	return eventNotHandledErr;
}

static OSStatus GraphicsCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	HICommand command;
	command.menu.menuRef = 0;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(HICommand), NULL, &command);

	// Check for valid input
	// assert(kStyleMenu == GetMenuID(command.menu.menuRef));
	if (command.commandID != 'graf' ||
			kStyleMenu != GetMenuID(command.menu.menuRef))
		return eventNotHandledErr;

	// Index in graphics_modes[]
	UInt32 op;
	GetMenuItemRefCon(command.menu.menuRef, command.menu.menuItemIndex, &op);

	if(graf_mode != op)
		graphics_aux(op);

	return noErr;
}

static void graphics_aux(int op)
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
	if (initialized && game_in_progress)
	{
		reset_visuals(TRUE);
	}
	RevalidateGraphics(&data[0], 0);
	Term_key_push(KTRL('R'));
}

static OSStatus TileSizeCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData )
{
	HICommand cmd;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(cmd), NULL, &cmd);
	UInt32 newSize = 0;
	GetMenuItemRefCon(cmd.menu.menuRef, cmd.menu.menuItemIndex, &newSize);
	term_data *td = (term_data*) Term->data;
	if(!td) return noErr;

	if(GetMenuID(cmd.menu.menuRef) == kTileWidMenu) {
		if(td->font_wid > newSize || newSize == td->tile_wid) return noErr;
		else td->tile_wid = newSize;
	}
	else if(GetMenuID(cmd.menu.menuRef) == kTileHgtMenu) {
		if(td->font_hgt > newSize || newSize == td->tile_hgt) return noErr;
		else td->tile_hgt = newSize;
	}
	else {
		return eventNotHandledErr;
	}

	RevalidateGraphics(td, inEvent);

	return noErr;
}

static OSStatus RestoreCommand(EventHandlerCallRef inCallRef,
							EventRef inEvent, void *inUserData)
{
	WindowRef w = 0;
	GetEventParameter(inEvent, kEventParamDirectObject, typeWindowRef,
							NULL, sizeof(w), NULL, &w);
	term_data *td = (term_data*) GetWRefCon(w);
	
	if(!td) return eventNotHandledErr;

	/* Mapped */
	td->mapped = TRUE;

	int i = td - &data[0];

	/* Link */
	term_data_link(i);

	/* Mapped (?) */
	td->t->mapped_flag = TRUE;

	/* Bring to the front */
	SelectWindow(td->w);

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

	term_data_check_font(td);
	term_data_check_size(td);
	term_data_resize(td);
	Term_resize(td->cols, td->rows);
	term_data_redraw(td);

	/* Bring to the front */
	SelectWindow(td->w);

	return noErr;
}

static OSStatus RevalidateGraphics(term_data *td, EventRef inEvent)
{
	if(!td) return noErr;

	HICommand command;
	command.commandID = 0;
	command.menu.menuRef = 0;
	if(inEvent) {
		GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(command), NULL, &command);
	}

	// Only rescale graphics when absolutely necessary.
	if(command.commandID != kTileWidMenu && command.commandID != kTileHgtMenu)
	{
		// Reset tilesize to default when graphics change.
		td->tile_wid = td->tile_hgt = 0;
	}

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

static OSStatus ToggleCommand(EventHandlerCallRef inCallRef,
								EventRef inEvent, void *inUserData )
{
	HICommand command;
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
							NULL, sizeof(command), NULL, &command);

	MenuItemIndex index = command.menu.menuItemIndex;
	int menuID = GetMenuID(command.menu.menuRef);
	for(int i = N_ELEMENTS(toggle_defs);  --i >= 0;)
	{
		if(index == toggle_defs[i].menuItem && menuID == toggle_defs[i].menuID)
		{
			*toggle_defs[i].var = !(*toggle_defs[i].var);
			if(toggle_defs[i].refresh == true) {
				RevalidateGraphics(&data[0], inEvent);
				graphics_tiles_nuke();
				// Force redraw.
				Term_key_push(KTRL('R'));
			}
			return noErr;
		}
	}
	return eventNotHandledErr;
}


static void FontChanged(UInt32 fontID, float size)
{
	if(size < MIN_FONT) return;

	ATSUStyle fontStyle;

	// Font size must be 8 or more.
	if( 8 > size || fontID == 0)
		return;

	term_data *td = (term_data*) GetWRefCon(fontInfo.focus);
	if(!td) return; // paranoia

	// No change.
	if(td->font_id == fontID && td->font_size == size)
		return ;

	const ATSUAttributeTag tags[] = {kATSUFontTag, kATSUSizeTag};


	Fixed fsize = (Fixed)(size*(1<<16));
	const ByteCount sizes[] = {sizeof(fontID), sizeof(fsize)};
	void * values[] = { &fontID, &fsize };

	ATSUCreateStyle(&fontStyle);
	ATSUSetAttributes(fontStyle, 2, tags, sizes, values);

	// Reject italics &c
	const ATSUAttributeTag badtags[] = {kATSUQDItalicTag, 
										kATSUQDUnderlineTag,
										kATSUQDCondensedTag };

	for(int i = 0; i < N_ELEMENTS(badtags); i++)
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
	RevalidateGraphics(td, 0);
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
		if(!GetWRefCon(w)) { //  Window is Font Panel.
			w = 0;
			GetEventParameter(inEvent, kEventParamPreviousWindow,
								typeWindowRef, NULL, sizeof(w), NULL,  &w);
		}
		if(w) fontInfo.focus = w; 
		return noErr;
	}
	else if(class == 'font' && type == kEventFontPanelClosed) {
		SetMenuItemTextWithCFString(GetMenuHandle(kStyleMenu), kFonts, CFSTR("Show Fonts"));
		return noErr;
	}

	return eventNotHandledErr;
}

static OSStatus MouseCommand ( EventHandlerCallRef inCallRef,
    EventRef inEvent, void *inUserData )
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

	// X coordinate relative to left side of window exclusive of border.
	p.x -= (BORDER_WID+td->r.left);
	// Y coordinate relative to top of window content region.
	// HACK: assumes title width of 21 pixels.
	p.y -= (td->r.top + 21);

	Term_mousepress(p.x/td->tile_wid, p.y/td->tile_hgt, button);

	return noErr;
}

static OSStatus KeyboardCommand ( EventHandlerCallRef inCallRef,
    EventRef inEvent, void *inUserData )
{

	EventRecord event;
	ConvertEventRefToEventRecord(inEvent, &event);

	/* Extract some modifiers */
	int mc = (event.modifiers & controlKey) ? TRUE : FALSE;
	int ms = (event.modifiers & shiftKey) ? TRUE : FALSE;
	int mo = (event.modifiers & optionKey) ? TRUE : FALSE;
	int mx = (event.modifiers & cmdKey) ? TRUE : FALSE;

	/* Keypress: (only "valid" if ck < 96) */
	int ch = (event.message & charCodeMask) & 255;

	/* Keycode: see table above */
	int ck = ((event.message & keyCodeMask) >> 8) & 255;

	/* Command + "normal key" -> menu action */
	if (mx && (ck < 64))
	{
		return eventNotHandledErr;
	}

	/* Hide the mouse pointer */
	hibernate();
	ObscureCursor();

	/* Normal key -> simple keypress */
	if ((ck < 64) || (ck == 93))
	{
		/* Enqueue the keypress */
		Term_keypress(ch);
	}

	/* Keypad keys -> trigger plus simple keypress */
	else if (!mc && !ms && !mo && !mx && (ck < 96))
	{
		/* Hack -- "enter" is confused */
		if (ck == 76) ch = '\n';

		/* Begin special trigger */
		Term_keypress(31);

		/* Send the "keypad" modifier */
		Term_keypress('K');

		/* Terminate the trigger */
		Term_keypress(13);

		/* Send the "ascii" keypress */
		Term_keypress(ch);
	}

	/* Bizarre key -> encoded keypress */
	else if (ck <= 127)
	{
		/* Begin special trigger */
		Term_keypress(31);

		/* Send some modifier keys */
		if (mc) Term_keypress('C');
		if (ms) Term_keypress('S');
		if (mo) Term_keypress('O');
		if (mx) Term_keypress('X');

		/* Downshift and encode the keycode */
		Term_keypress(I2D((ck - 64) / 10));
		Term_keypress(I2D((ck - 64) % 10));

		/* Terminate the trigger */
		Term_keypress(13);
	}
	return noErr;
}

static OSStatus PrintCommand(EventHandlerCallRef inCallRef, EventRef inEvent,
    void *inUserData )
{
	mac_warning((const char*) inUserData);
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
	WindowRef w = FrontWindow();
	term_data *td = (term_data *)GetWRefCon(w);
	if(!td) return noErr;

	hibernate();
	Cursor tempCursor;
	SetPort(GetWindowPort(w));
	SetCursor(GetQDGlobalsArrow(&tempCursor));

	/* Synchronise term */
	term_data_redraw(td);
	return noErr;
}

static OSErr AEH_Open(const AppleEvent *theAppleEvent, AppleEvent* reply,
	SInt32 handlerRefCon)
{
	FSSpec myFSS;
	AEDescList docList;
	OSErr err;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	char msg[128];
	FInfo myFileInfo;

	/* Put the direct parameter (a descriptor list) into a docList */
	err = AEGetParamDesc(
		theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err) return err;

	/*
	 * We ignore the validity check, because we trust the FInder, and we only
	 * allow one savefile to be opened, so we ignore the depth of the list.
	 */
	err = AEGetNthPtr(
		&docList, 1L, typeFSS, &keywd, &returnedType,
		(Ptr) &myFSS, sizeof(myFSS), &actualSize);
	if (err) return err;

	/* Only needed to check savefile type below */
	err = FSpGetFInfo(&myFSS, &myFileInfo);
	if (err)
	{
		strnfmt(msg, sizeof(msg), "Argh!  FSpGetFInfo failed with code %d", err);
		mac_warning(msg);
		return err;
	}

	/* Ignore non 'SAVE' files */
	if (myFileInfo.fdType != 'SAVE') return noErr;

	/* Extract a file name */
	(void)spec_to_path(&myFSS, savefile, sizeof(savefile));

	/* Delay actual open */
	open_when_ready = TRUE;

	/* Dispose */
	err = AEDisposeDesc(&docList);

	/* Success */
	return noErr;
}

/*
 * Apple Event Handler -- Re-open Application
 *
 * If no windows are currently open, show the Angband window.
 * This required AppleEvent was introduced by System 8 -- pelpel
 */
static OSErr AEH_Reopen(const AppleEvent *theAppleEvent,
			     AppleEvent* reply, long handlerRefCon)
{
#pragma unused(theAppleEvent, reply, handlerRefCon)

	term_data *td = NULL;

	/* No open windows */
	if (NULL == FrontWindow())
	{
		/* Obtain the Angband window */
		td = &data[0];

		/* Mapped */
		td->mapped = TRUE;

		/* Link */
		term_data_link(0);

		/* Mapped (?) */
		td->t->mapped_flag = TRUE;

		/* Show the window */
		ShowWindow(td->w);

		/* Bring to the front */
		SelectWindow(td->w);

		/* Make it active */
		activate(td->w);
	}
	/* Event handled */
	return (noErr);
}


/*
 * Handle quit_when_ready, by Peter Ammon,
 * slightly modified to check inkey_flag.
 */
static void quit_calmly(void)
{
	/* Quit immediately if game's not started */
	if (!game_in_progress || !character_generated) quit(NULL);

	/* Save the game and Quit (if it's safe) */
	if (inkey_flag)
	{
		/* Hack -- Forget messages */
		msg_flag = FALSE;

		/* Save the game */
#ifndef ZANG_AUTO_SAVE
		do_cmd_save_game();
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
	} while(err != eventNotHandledErr && err); // DurationForever is -1
	return true;
}


/*** Some Hooks for various routines ***/

/*
 * Mega-Hack -- emergency lifeboat
 */
static void *lifeboat = NULL;


/*
 * Hook to handle "out of memory" errors
 */
static void *hook_rpanic(huge size)
{
#pragma unused(size)

	/* Free the lifeboat */
	if (lifeboat)
	{
		/* Free the lifeboat */
		free(lifeboat);

		/* Forget the lifeboat */
		lifeboat = NULL;

		/* Mega-Hack -- Warning */
		mac_warning("Running out of Memory!\nTerminate this process now!");

		(void) pause();
	}

	/* Mega-Hack -- Crash */
	return (NULL);
}


/*
 * Hook to tell the user something important
 */
static void hook_plog(cptr str)
{
	/* Warning message */
	mac_warning(str);
}


/*
 * Hook to tell the user something, and then quit
 */
static void hook_quit(cptr str)
{
	/* Warning if needed */
	if (str) mac_warning(str);

	/* Clean up sound support */
	cleanup_sound();


	/* Dispose of graphic tiles */
	if(frame.image)
		graphics_nuke();

	/* Write a preference file */
	if (initialized) save_pref_file();

	/* All done */
	ExitToShell();
}


/*
 * Hook to tell the user something, and then crash
 */
static void hook_core(cptr str)
{
	/* XXX Use the debugger */
	/* DebugStr(str); */

	/* Warning */
	if (str) mac_warning(str);
	if(!save_player())
		mac_warning("Fatal error -- save failed!");
	else 
		mac_warning("Fatal error -- game saved successfully");

	/* Quit */
	quit(NULL);
}

/*** Main program ***/


/*
 * Initialize and verify file and dir paths
 *
 */
static void init_paths(void)
{
	char path[1024];

	/* Default to the "lib" folder with the application */
	if (locate_lib(path, sizeof(path)) == NULL)
	{
		quit("unable to find 'lib' dir");
	}
	/* Create directories for the users files */
	create_user_dirs();

	/* Prepare the paths */
	init_file_paths(path);

	/* Build the filename */
	path_build(path, sizeof(path), ANGBAND_DIR_FILE, "news.txt");

	/* Attempt to open and close that file */
	if (0 != fd_close(fd_open(path, O_RDONLY)))
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

	/* Mark ourself as the file creator */
	_fcreator = ANGBAND_CREATOR;
	/* Default to saving a "text" file */
	_ftype = 'TEXT';


	/* Hook in some "z-virt.c" hooks */
	rnfree_aux = NULL;
	ralloc_aux = NULL;
	rpanic_aux = hook_rpanic;

	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_core;

	/* Initialize colors */
	update_color_info();

	/* Show the "watch" cursor */
	SetCursor(*(GetCursor(watchCursor)));


	/* Prepare the menubar */
	init_menubar();

	/* Initialize */
	init_paths();

	/* Prepare the windows */
	init_windows();

#if 0
	/* Handle 'apple' events */
    /* Install the open event hook (ignore error codes) */
    (void)AEInstallEventHandler(
        kCoreEventClass,
        kAEOpenDocuments,
        NewAEEventHandlerUPP(AEH_Open),
        0L,
        FALSE);
#endif

	/* Install menu and application handlers */
	install_handlers(0);

	/* Hack -- process all events */
	FlushEventQueue(GetMainEventQueue());


	/* Reset the cursor */
	Cursor tempCursor;
	SetCursor(GetQDGlobalsArrow(&tempCursor));

	/* Mega-Hack -- Allocate a "lifeboat" */
	lifeboat = malloc(16384);

	/* Quicktime -- Load sound effect resources */

	load_sounds();

	/* Note the "system" */
	ANGBAND_SYS = "mac";


	/* Validate the contents of the main window */
	validate_main_window();

	/* Reset event queue */
	FlushEventQueue(GetMainEventQueue());

	/* We are now initialized */
	initialized = TRUE;

	validate_menus();

	/* Start playing! */
	EventRef newGameEvent = nil;
	CreateEvent ( nil, 'Play', 'Band', GetCurrentEventTime(),    
									kEventAttributeNone, &newGameEvent ); 
	PostEventToQueue(GetMainEventQueue(), newGameEvent, kEventPriorityHigh);

	RunApplicationEventLoop();

	/* Quit */
	quit(NULL);

	/* Since it's an int function */
	return (0);
}

#endif /* MACH_O_CARBON */
