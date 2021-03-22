/**
 * \file main-cocoa.m
 * \brief OS X front end
 *
 * Copyright (c) 2011 Peter Ammon
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
#include "game-world.h"
#include "grafmode.h"
#include "init.h"
#include "obj-util.h"
#include "savefile.h"
#include "ui-command.h"
#include "ui-display.h"
#include "ui-game.h"
#include "ui-init.h"
#include "ui-input.h"
#include "ui-output.h"
#include "ui-prefs.h"


#if defined(SAFE_DIRECTORY)
#import "buildid.h"
#endif

/*#define NSLog(...) ; */


#if defined(MACH_O_CARBON)

/* Default creator signature */
#ifndef ANGBAND_CREATOR
# define ANGBAND_CREATOR 'A271'
#endif

/* Mac headers */
#import "cocoa/AppDelegate.h"
//#include <Carbon/Carbon.h> /* For keycodes */
/* Hack - keycodes to enable compiling in macOS 10.14 */
#define kVK_Return 0x24
#define kVK_Tab    0x30
#define kVK_Delete 0x33
#define kVK_Escape 0x35
#define kVK_ANSI_KeypadEnter 0x4C

static NSString * const AngbandDirectoryNameLib = @"lib";
static NSString * const AngbandDirectoryNameBase = @VERSION_NAME;

static NSString * const AngbandTerminalsDefaultsKey = @"Terminals";
static NSString * const AngbandTerminalRowsDefaultsKey = @"Rows";
static NSString * const AngbandTerminalColumnsDefaultsKey = @"Columns";
static NSString * const AngbandTerminalVisibleDefaultsKey = @"Visible";
static NSString * const AngbandGraphicsDefaultsKey = @"GraphicsID";
static NSString * const AngbandUseDefaultTileMultDefaultsKey =
    @"UseDefaultTileMultiplier";
static NSString * const AngbandTileWidthMultDefaultsKey =
    @"TileWidthMultiplier";
static NSString * const AngbandTileHeightMultDefaultsKey =
    @"TileHeightMultiplier";
static NSString * const AngbandFrameRateDefaultsKey = @"FramesPerSecond";
static NSString * const AngbandSoundDefaultsKey = @"AllowSound";
static NSInteger const AngbandWindowMenuItemTagBase = 1000;
static NSInteger const AngbandCommandMenuItemTagBase = 2000;

/* Application defined event numbers */
enum
{
    AngbandEventWakeup = 1
};

/* Redeclare some 10.7 constants and methods so we can build on 10.6 */
enum
{
    Angband_NSWindowCollectionBehaviorFullScreenPrimary = 1 << 7,
    Angband_NSWindowCollectionBehaviorFullScreenAuxiliary = 1 << 8
};

@interface NSWindow (AngbandLionRedeclares)
- (void)setRestorable:(BOOL)flag;
@end

/* Delay handling of pre-emptive "quit" event */
static BOOL quit_when_ready = FALSE;

/* Set to indicate the game is over and we can quit without delay */
static Boolean game_is_finished = FALSE;

/* Our frames per second (e.g. 60). A value of 0 means unthrottled. */
static int frames_per_second;

/* Force a new game or not? */
static bool new_game = FALSE;

@class AngbandView;

/**
 * Load sound effects based on sound.cfg within the xtra/sound directory;
 * bridge to Cocoa to use NSSound for simple loading and playback, avoiding
 * I/O latency by caching all sounds at the start.  Inherits full sound
 * format support from Quicktime base/plugins.
 * pelpel favoured a plist-based parser for the future but .cfg support
 * improves cross-platform compatibility.
 */
@interface AngbandSoundCatalog : NSObject {
@private
    /**
     * Stores instances of NSSound keyed by path so the same sound can be
     * used for multiple events.
     */
    NSMutableDictionary *soundsByPath;
    /**
     * Stores arrays of NSSound keyed by event number.
     */
    NSMutableDictionary *soundArraysByEvent;
}

/**
 * If NO, then playSound effectively becomes a do nothing operation.
 */
@property (getter=isEnabled) BOOL enabled;

/**
 * Set up for lazy initialization in playSound().  Set enabled to NO.
 */
- (id)init;

/**
 * If self.enabled is YES and the given event has one or more sounds
 * corresponding to it in the catalog, plays one of those sounds, chosen at
 * random.
 */
- (void)playSound:(int)event;

/**
 * Impose an arbitrary limit on the number of possible samples per event.
 * Currently not declaring this as a class property for compatibility with
 * versions of Xcode prior to 8.
 */
+ (int)maxSamples;

/**
 * Return the shared sound catalog instance, creating it if it does not
 * exist yet.  Currently not declaring this as a class property for
 * compatibility with versions of Xcode prior to 8.
 */
+ (AngbandSoundCatalog*)sharedSounds;

/**
 * Release any resources associated with shared sounds.
 */
+ (void)clearSharedSounds;

@end

@implementation AngbandSoundCatalog

- (id)init {
    if (self = [super init]) {
	self->soundsByPath = nil;
	self->soundArraysByEvent = nil;
	self->_enabled = NO;
    }
    return self;
}

- (void)playSound:(int)event {
    if (! self.enabled) {
	return;
    }

    /* Initialize when the first sound is played. */
    if (self->soundArraysByEvent == nil) {
	/* Find and open the config file */
	char path[2048];
	path_build(path, sizeof(path), ANGBAND_DIR_SOUNDS, "sound.cfg");
	ang_file *fff = file_open(path, MODE_READ, -1);

	/* Handle errors */
	if (!fff) {
	    NSLog(@"The sound configuration file could not be opened.");
	    return;
	}

	self->soundsByPath = [[NSMutableDictionary alloc] init];
	self->soundArraysByEvent = [[NSMutableDictionary alloc] init];
	@autoreleasepool {
	    /*
	     * This loop may take a while depending on the count and size of
	     * samples to load.
	     */

	    /* Parse the file */
	    /* Lines are always of the form "name = sample [sample ...]" */
	    char buffer[2048];
	    while (file_getl(fff, buffer, sizeof(buffer))) {
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
		event = message_lookup_by_sound_name(msg_name);
		if (event < 0) continue;

		/*
		 * Advance the sample list pointer so it's at the beginning of
		 * text.
		 */
		cfg_sample_list++;
		if (!cfg_sample_list[0]) continue;

		/* Terminate the current token */
		cur_token = cfg_sample_list;
		search = strchr(cur_token, ' ');
		if (search) {
		    search[0] = '\0';
		    next_token = search + 1;
		} else {
		    next_token = NULL;
		}

		/*
		 * Now we find all the sample names and add them one by one
		 */
		while (cur_token) {
		    NSMutableArray *soundSamples =
			[self->soundArraysByEvent
			     objectForKey:[NSNumber numberWithInteger:event]];
		    if (soundSamples == nil) {
			soundSamples = [[NSMutableArray alloc] init];
			[self->soundArraysByEvent
			     setObject:soundSamples
			     forKey:[NSNumber numberWithInteger:event]];
		    }
		    int num = (int) soundSamples.count;

		    /* Don't allow too many samples */
		    if (num >= [AngbandSoundCatalog maxSamples]) break;

		    NSString *token_string =
			[NSString stringWithUTF8String:cur_token];
		    NSSound *sound =
			[self->soundsByPath objectForKey:token_string];

		    if (! sound) {
			/*
			 * We have to load the sound. Build the path to the
			 * sample.
			 */
			path_build(path, sizeof(path), ANGBAND_DIR_SOUNDS,
				   cur_token);
			if (file_exists(path)) {
			    /* Load the sound into memory */
			    sound = [[NSSound alloc]
					 initWithContentsOfFile:[NSString stringWithUTF8String:path]
					 byReference:YES];
			    if (sound) {
				[self->soundsByPath setObject:sound
					    forKey:token_string];
			    }
			}
		    }

		    /* Store it if we loaded it */
		    if (sound) {
			[soundSamples addObject:sound];
		    }

		    /* Figure out next token */
		    cur_token = next_token;
		    if (next_token) {
			 /* Try to find a space */
			 search = strchr(cur_token, ' ');

			 /*
			  * If we can find one, terminate, and set new "next".
			  */
			 if (search) {
			     search[0] = '\0';
			     next_token = search + 1;
			 } else {
			     /* Otherwise prevent infinite looping */
			     next_token = NULL;
			 }
		    }
		}
	    }
	}

	/* Close the file */
	file_close(fff);
    }

    @autoreleasepool {
	NSMutableArray *samples =
	    [self->soundArraysByEvent
		 objectForKey:[NSNumber numberWithInteger:event]];

	if (samples == nil || samples.count == 0) {
	    return;
	}

	/* Choose a random event. */
	int s = randint0((int) samples.count);
	NSSound *sound = samples[s];

	if ([sound isPlaying])
	    [sound stop];

	/* Play the sound. */
	[sound play];
    }
}

+ (int)maxSamples {
    return 16;
}

/**
 * For sharedSounds and clearSharedSounds.
 */
static __strong AngbandSoundCatalog* gSharedSounds = nil;

+ (AngbandSoundCatalog*)sharedSounds {
    if (gSharedSounds == nil) {
	gSharedSounds = [[AngbandSoundCatalog alloc] init];
    }
    return gSharedSounds;
}

+ (void)clearSharedSounds {
    gSharedSounds = nil;
}

@end

/**
 * Each location in the terminal either stores a character, a tile,
 * padding for a big tile, or padding for a big character (for example a
 * kanji that takes two columns).  These structures represent that.  Note
 * that tiles do not overlap with each other (excepting the double-height
 * tiles, i.e. from the Shockbolt set; that's handled as a special case).
 * Characters can overlap horizontally:  that is for handling fonts that
 * aren't fixed width.
 */
struct TerminalCellChar {
    wchar_t glyph;
    int attr;
};
struct TerminalCellTile {
    /*
     * These are the coordinates, within the tile set, for the foreground
     * tile and background tile.
     */
    char fgdCol, fgdRow, bckCol, bckRow;
};
struct TerminalCellPadding {
       /*
	* If the cell at (x, y) is padding, the cell at (x - hoff, y - voff)
	* has the attributes affecting the padded region.
	*/
    unsigned char hoff, voff;
};
struct TerminalCell {
    union {
	struct TerminalCellChar ch;
	struct TerminalCellTile ti;
	struct TerminalCellPadding pd;
    } v;
    /*
     * Used for big characters or tiles which are hscl x vscl cells.
     * The upper left corner of the big tile or character is marked as
     * TERM_CELL_TILE or TERM_CELL_CHAR.  The remainder are marked as
     * TERM_CELL_TILE_PADDING or TERM_CELL_CHAR_PADDING and have hscl and
     * vscl set to matcn what's in the upper left corner.  Big tiles are
     * tiles scaled up to occupy more space.  Big characters, on the other
     * hand, are characters that naturally take up more space than standard
     * for the font with the assumption that vscl will be one for any big
     * character and hscl will hold the number of columns it occupies (likely
     * just 2, i.e. for Japanese kanji).
     */
    unsigned char hscl;
    unsigned char vscl;
    /*
     * Hold the offsets, as fractions of the tile size expressed as the
     * rational numbers hoff_n / hoff_d and voff_n / voff_d, within the tile
     * or character.  For something that is not a big tile or character, these
     * will be 0, 0, 1, and 1.  For a big tile or character, these will be
     * set when the tile or character is changed to be 0, 0, hscl, and vscl
     * for the upper left corner and i, j, hscl, vscl for the padding element
     * at (i, j) relative to the upper left corner.  For a big tile or
     * character that is partially overwritten, these are not modified in the
     * parts that are not overwritten while hscl, vscl, and, for padding,
     * v.pd.hoff and v.pd.voff are.
     */
    unsigned char hoff_n;
    unsigned char voff_n;
    unsigned char hoff_d;
    unsigned char voff_d;
    /*
     * Is either TERM_CELL_CHAR, TERM_CELL_CHAR_PADDING, TERM_CELL_TILE, or
     * TERM_CELL_TILE_PADDING.
     */
    unsigned char form;
};
#define TERM_CELL_CHAR (0x1)
#define TERM_CELL_CHAR_PADDING (0x2)
#define TERM_CELL_TILE (0x4)
#define TERM_CELL_TILE_PADDING (0x8)

struct TerminalCellBlock {
    int ulcol, ulrow, w, h;
};

struct TerminalCellLocation {
    int col, row;
};

typedef int (*TerminalCellPredicate)(const struct TerminalCell*);

static int isTileTop(const struct TerminalCell *c)
{
    return (c->form == TERM_CELL_TILE ||
	    (c->form == TERM_CELL_TILE_PADDING && c->v.pd.voff == 0)) ? 1 : 0;
}

static int isPartiallyOverwrittenBigChar(const struct TerminalCell *c)
{
    if ((c->form & (TERM_CELL_CHAR | TERM_CELL_CHAR_PADDING)) != 0) {
	/*
	 * When the tile is set in Term_pict_cocoa, hoff_d is the same as hscl
	 * and voff_d is the same as vscl.  hoff_d and voff_d aren't modified
	 * after that, but hscl and vscl are in response to partial overwrites.
	 * If they're diffent, an overwrite has occurred.
	 */
	return ((c->hoff_d > 1 || c->voff_d > 1) &&
		(c->hoff_d != c->hscl || c->voff_d != c->vscl)) ? 1 : 0;
    }
    return 0;
}

static int isCharNoPartial(const struct TerminalCell *c)
{
    return ((c->form & (TERM_CELL_CHAR | TERM_CELL_CHAR_PADDING)) != 0 &&
	    ! isPartiallyOverwrittenBigChar(c)) ? 1 : 0;
}

static int get_background_color_index(int idx);
/*
 * Implicitly assume that the cells considered have already been constrained
 * to be a character or character padding that is in the topmost row of the
 * padded area.
 */
static int hasSameBackground(const struct TerminalCell* c)
{
    if (c->form == TERM_CELL_CHAR) {
	return get_background_color_index(c->v.ch.attr);
    }
    return get_background_color_index((c - c->v.pd.hoff)->v.ch.attr);
}

/**
 * Since the drawing is decoupled from Angband's calls to the text_hook,
 * pict_hook, wipe_hook, curs_hook, and bigcurs_hook callbacks of a terminal,
 * maintain a version of the Terminal contents.
 */
@interface TerminalContents : NSObject {
@private
    struct TerminalCell *cells;
}

/**
 * Initialize with zero columns and zero rows.
 */
- (id)init;

/**
 * Initialize with nCol columns and nRow rows.  All elements will be set to
 * blanks.
 */
- (id)initWithColumns:(int)nCol rows:(int)nRow NS_DESIGNATED_INITIALIZER;

/**
 * Resize to be nCol by nRow.  Current contents still within the new bounds
 * are preserved.  Added areas are filled with blanks.
 */
- (void)resizeWithColumns:(int)nCol rows:(int)nRow;

/**
 * Get the contents of a given cell.
 */
- (const struct TerminalCell*)getCellAtColumn:(int)icol row:(int)irow;

/**
 * Scans the row, irow, starting at the column, icol0, and stopping before the
 * column, icol1.  Returns the column index for the first cell that's within
 * the given type mask, tm.  If all of the cells in that range are not within
 * the given type mask, returns icol1.
 */
- (int)scanForTypeMaskInRow:(int)irow mask:(unsigned int)tm col0:(int)icol0
		       col1:(int)icol1;

/**
 * Scans the w x h block whose upper left corner is at (icol, irow).  The
 * scan starts at (icol + pcurs->col, irow + pcurs->row) and proceeds from
 * left to right and top to bottom.  At exit, pcurs will have the location
 * (relative to icol, irow) of the first cell encountered that's within the
 * given type mask, tm.  If no such cell was found, pcurs->col will be w
 * and pcurs->row will be h.
 */
- (void)scanForTypeMaskInBlockAtColumn:(int)icol row:(int)irow width:(int)w
				height:(int)h mask:(unsigned int)tm
				cursor:(struct TerminalCellLocation*)pcurs;

/**
 * Scans the row, irow, starting at the column, icol0, and stopping before the
 * column, icol1.  Returns the column index for the first cell that
 * func(cell_address) != rval.  If all of the cells in the range satisfy the
 * predicate, returns icol1.
 */
- (int)scanForPredicateInRow:(int)irow
		   predicate:(TerminalCellPredicate)func
		     desired:(int)rval
			col0:(int)icol0
			col1:(int)icol1;

/**
 * Change the contents to have the given string of n characters appear with
 * the leftmost character at (icol, irow).
 */
- (void)setUniformAttributeTextRunAtColumn:(int)icol
				       row:(int)irow
					 n:(int)n
				    glyphs:(const wchar_t*)g
				 attribute:(int)a;

/**
 * Change the contents to have a tile scaled to w x h appear with its upper
 * left corner at (icol, irow).
 */
- (void)setTileAtColumn:(int)icol
		    row:(int)irow
       foregroundColumn:(char)fgdCol
	  foregroundRow:(char)fgdRow
       backgroundColumn:(char)bckCol
	  backgroundRow:(char)bckRow
	      tileWidth:(int)w
	     tileHeight:(int)h;

/**
 * Wipe the w x h block whose upper left corner is at (icol, irow).
 */
- (void)wipeBlockAtColumn:(int)icol row:(int)irow width:(int)w height:(int)h;

/**
 * Wipe all the contents.
 */
- (void)wipe;

/**
 * Wipe any tiles.
 */
- (void)wipeTiles;

/**
 * Thie is a helper function for wipeBlockAtColumn.
 */
- (void)wipeBlockAuxAtColumn:(int)icol row:(int)irow width:(int)w
		      height:(int)h;

/**
 * This is a helper function for checkForBigStuffOverwriteAtColumn.
 */
- (void) splitBlockAtColumn:(int)icol row:(int)irow n:(int)nsub
		     blocks:(const struct TerminalCellBlock*)b;

/**
 * This is a helper function for setUniformAttributeTextRunAtColumn,
 * setTileAtColumn, and wipeBlockAtColumn.  If a modification could partially
 * overwrite a big character or tile, make adjustments so what's left can
 * be handled appropriately in rendering.
 */
- (void)checkForBigStuffOverwriteAtColumn:(int)icol row:(int)irow
				    width:(int)w height:(int)h;

/**
 * Position the upper left corner of the cursor at (icol, irow) and have it
 * encompass w x h cells.
 */
- (void)setCursorAtColumn:(int)icol row:(int)irow width:(int)w height:(int)h;

/**
 * Remove the cursor.  cursorColumn and cursorRow will be -1 until
 * setCursorAtColumn is called.
 */
- (void)removeCursor;

/**
 * Verify that everying is consistent.
 */
- (void)assertInvariants;

/**
 * Is the number of columns.
 */
@property (readonly) int columnCount;

/**
 * Is the number of rows.
 */
@property (readonly) int rowCount;

/**
 * Is the column index for the upper left corner of the cursor.  It will be -1
 * if the cursor is disabled.
 */
@property (readonly) int cursorColumn;

/**
 * Is the row index for the upper left corner of the cursor.  It will be -1
 * if the cursor is disabled.
 */
@property (readonly) int cursorRow;

/**
 * Is the cursor width in number of cells.
 */
@property (readonly) int cursorWidth;

/**
 * Is the cursor height in number of cells.
 */
@property (readonly) int cursorHeight;

/**
 * Return the character to be used for blanks.
 */
+ (wchar_t)getBlankChar;

/**
 * Return the attribute to be used for blanks.
 */
+ (int)getBlankAttribute;

@end

@implementation TerminalContents

- (id)init
{
    return [self initWithColumns:0 rows:0];
}

- (id)initWithColumns:(int)nCol rows:(int)nRow
{
    if (self = [super init]) {
	self->cells = malloc(nCol * nRow * sizeof(struct TerminalCell));
	self->_columnCount = nCol;
	self->_rowCount = nRow;
	self->_cursorColumn = -1;
	self->_cursorRow = -1;
	self->_cursorWidth = 1;
	self->_cursorHeight = 1;
	[self wipe];
    }
    return self;
}

- (void)dealloc
{
    if (self->cells != 0) {
	free(self->cells);
	self->cells = 0;
    }
}

- (void)resizeWithColumns:(int)nCol rows:(int)nRow
{
    /*
     * Potential issue: big tiles or characters can become clipped by the
     * resize.  That will only matter if drawing occurs before the contents
     * are updated by Angband.  Even then, unless the drawing mode is used
     * where AppKit doesn't clip to the window bounds, the only artifact will
     * be clipping when drawn which is acceptable and doesn't require
     * additional logic to either filter out the clipped big stuff here or
     * to just clear it when drawing.
     */
    struct TerminalCell *newCells =
	malloc(nCol * nRow * sizeof(struct TerminalCell));
    struct TerminalCell *cellsOutCursor = newCells;
    const struct TerminalCell *cellsInCursor = self->cells;
    int nColCommon = (nCol < self.columnCount) ? nCol : self.columnCount;
    int nRowCommon = (nRow < self.rowCount) ? nRow : self.rowCount;
    wchar_t blank = [TerminalContents getBlankChar];
    int blank_attr = [TerminalContents getBlankAttribute];
    int i;

    for (i = 0; i < nRowCommon; ++i) {
	(void) memcpy(
	    cellsOutCursor,
	    cellsInCursor,
	    nColCommon * sizeof(struct TerminalCell));
	cellsInCursor += self.columnCount;
	for (int j = nColCommon; j < nCol; ++j) {
	    cellsOutCursor[j].v.ch.glyph = blank;
	    cellsOutCursor[j].v.ch.attr = blank_attr;
	    cellsOutCursor[j].hscl = 1;
	    cellsOutCursor[j].vscl = 1;
	    cellsOutCursor[j].hoff_n = 0;
	    cellsOutCursor[j].voff_n = 0;
	    cellsOutCursor[j].hoff_d = 1;
	    cellsOutCursor[j].voff_d = 1;
	    cellsOutCursor[j].form = TERM_CELL_CHAR;
	}
	cellsOutCursor += nCol;
    }
    while (cellsOutCursor != newCells + nCol * nRow) {
	cellsOutCursor->v.ch.glyph = blank;
	cellsOutCursor->v.ch.attr = blank_attr;
	cellsOutCursor->hscl = 1;
	cellsOutCursor->vscl = 1;
	cellsOutCursor->hoff_n = 0;
	cellsOutCursor->voff_n = 0;
	cellsOutCursor->hoff_d = 1;
	cellsOutCursor->voff_d = 1;
	cellsOutCursor->form = TERM_CELL_CHAR;
	++cellsOutCursor;
    }

    free(self->cells);
    self->cells = newCells;
    self->_columnCount = nCol;
    self->_rowCount = nRow;
    if (self->_cursorColumn >= nCol || self->_cursorRow >= nRow) {
	self->_cursorColumn = -1;
	self->_cursorRow = -1;
    } else {
	if (self->_cursorColumn + self->_cursorWidth > nCol) {
	    self->_cursorWidth = nCol - self->_cursorColumn;
	}
	if (self->_cursorRow + self->_cursorHeight > nRow) {
	    self->_cursorHeight = nRow - self->_cursorRow;
	}
    }
}

- (const struct TerminalCell*)getCellAtColumn:(int)icol row:(int)irow
{
    return self->cells + icol + irow * self.columnCount;
}

- (int)scanForTypeMaskInRow:(int)irow mask:(unsigned int)tm col0:(int)icol0
		       col1:(int)icol1
{
    int i = icol0;
    const struct TerminalCell *cellsRow =
	self->cells + irow * self.columnCount;

    while (1) {
	if (i >= icol1) {
	    return icol1;
	}
	if ((cellsRow[i].form & tm) != 0) {
	    return i;
	}
	++i;
    }
}

- (void)scanForTypeMaskInBlockAtColumn:(int)icol row:(int)irow width:(int)w
				height:(int)h mask:(unsigned int)tm
				cursor:(struct TerminalCellLocation*)pcurs
{
    const struct TerminalCell *cellsRow =
	self->cells + (irow + pcurs->row) * self.columnCount;
    while (1) {
	if (pcurs->col == w) {
	    if (pcurs->row >= h - 1) {
		pcurs->row = h;
		return;
	    }
	    ++pcurs->row;
	    pcurs->col = 0;
	    cellsRow += self.columnCount;
	}

	if ((cellsRow[icol + pcurs->col].form & tm) != 0) {
	    return;
	}

	++pcurs->col;
    }
}

- (int)scanForPredicateInRow:(int)irow
		   predicate:(TerminalCellPredicate)func
		     desired:(int)rval
			col0:(int)icol0
			col1:(int)icol1
{
    int i = icol0;
    const struct TerminalCell *cellsRow =
	self->cells + irow * self.columnCount;

    while (1) {
	if (i >= icol1) {
	    return icol1;
	}
	if (func(cellsRow + i) != rval) {
	    return i;
	}
	++i;
    }
}

- (void)setUniformAttributeTextRunAtColumn:(int)icol
				       row:(int)irow
					 n:(int)n
				    glyphs:(const wchar_t*)g
				 attribute:(int)a
{
    [self checkForBigStuffOverwriteAtColumn:icol row:irow width:n height:1];

    struct TerminalCell *cellsRow = self->cells + irow * self.columnCount;
    for (int i = icol; i < icol + n; ++i) {
	cellsRow[i].v.ch.glyph = *g++;
	cellsRow[i].v.ch.attr = a;
	cellsRow[i].hscl = 1;
	cellsRow[i].vscl = 1;
	cellsRow[i].hoff_n = 0;
	cellsRow[i].voff_n = 0;
	cellsRow[i].hoff_d = 1;
	cellsRow[i].voff_d = 1;
	cellsRow[i].form = TERM_CELL_CHAR;
    }
}

- (void)setTileAtColumn:(int)icol
		    row:(int)irow
       foregroundColumn:(char)fgdCol
	  foregroundRow:(char)fgdRow
       backgroundColumn:(char)bckCol
	  backgroundRow:(char)bckRow
	      tileWidth:(int)w
	     tileHeight:(int)h
{
    [self checkForBigStuffOverwriteAtColumn:icol row:irow width:w height:h];

    struct TerminalCell *cellsRow = self->cells + irow * self.columnCount;

    cellsRow[icol].v.ti.fgdCol = fgdCol;
    cellsRow[icol].v.ti.fgdRow = fgdRow;
    cellsRow[icol].v.ti.bckCol = bckCol;
    cellsRow[icol].v.ti.bckRow = bckRow;
    cellsRow[icol].hscl = w;
    cellsRow[icol].vscl = h;
    cellsRow[icol].hoff_n = 0;
    cellsRow[icol].voff_n = 0;
    cellsRow[icol].hoff_d = w;
    cellsRow[icol].voff_d = h;
    cellsRow[icol].form = TERM_CELL_TILE;

    int ic;
    for (ic = icol + 1; ic < icol + w; ++ic) {
	cellsRow[ic].v.pd.hoff = ic - icol;
	cellsRow[ic].v.pd.voff = 0;
	cellsRow[ic].hscl = w;
	cellsRow[ic].vscl = h;
	cellsRow[ic].hoff_n = ic - icol;
	cellsRow[ic].voff_n = 0;
	cellsRow[ic].hoff_d = w;
	cellsRow[ic].voff_d = h;
	cellsRow[ic].form = TERM_CELL_TILE_PADDING;
    }
    cellsRow += self.columnCount;
    for (int ir = irow + 1; ir < irow + h; ++ir) {
	for (ic = icol; ic < icol + w; ++ic) {
	    cellsRow[ic].v.pd.hoff = ic - icol;
	    cellsRow[ic].v.pd.voff = ir - irow;
	    cellsRow[ic].hscl = w;
	    cellsRow[ic].vscl = h;
	    cellsRow[ic].hoff_n = ic - icol;
	    cellsRow[ic].voff_n = ir - irow;
	    cellsRow[ic].hoff_d = w;
	    cellsRow[ic].voff_d = h;
	    cellsRow[ic].form = TERM_CELL_TILE_PADDING;
	}
	cellsRow += self.columnCount;
    }
}

- (void)wipeBlockAtColumn:(int)icol row:(int)irow width:(int)w height:(int)h
{
    [self checkForBigStuffOverwriteAtColumn:icol row:irow width:w height:h];
    [self wipeBlockAuxAtColumn:icol row:irow width:w height:h];
}

- (void)wipe
{
    wchar_t blank = [TerminalContents getBlankChar];
    int blank_attr = [TerminalContents getBlankAttribute];
    struct TerminalCell *cellCursor = self->cells +
	self.columnCount * self.rowCount;

    while (cellCursor != self->cells) {
	--cellCursor;
	cellCursor->v.ch.glyph = blank;
	cellCursor->v.ch.attr = blank_attr;
	cellCursor->hscl = 1;
	cellCursor->vscl = 1;
	cellCursor->hoff_n = 0;
	cellCursor->voff_n = 0;
	cellCursor->hoff_d = 1;
	cellCursor->voff_d = 1;
	cellCursor->form = TERM_CELL_CHAR;
    }
}

- (void)wipeTiles
{
    wchar_t blank = [TerminalContents getBlankChar];
    int blank_attr = [TerminalContents getBlankAttribute];
    struct TerminalCell *cellCursor = self->cells +
	self.columnCount * self.rowCount;

    while (cellCursor != self->cells) {
	--cellCursor;
	if ((cellCursor->form &
	     (TERM_CELL_TILE | TERM_CELL_TILE_PADDING)) != 0) {
	    cellCursor->v.ch.glyph = blank;
	    cellCursor->v.ch.attr = blank_attr;
	    cellCursor->hscl = 1;
	    cellCursor->vscl = 1;
	    cellCursor->hoff_n = 0;
	    cellCursor->voff_n = 0;
	    cellCursor->hoff_d = 1;
	    cellCursor->voff_d = 1;
	    cellCursor->form = TERM_CELL_CHAR;
	}
    }
}

- (void)wipeBlockAuxAtColumn:(int)icol row:(int)irow width:(int)w
		      height:(int)h
{
    struct TerminalCell *cellsRow = self->cells + irow * self.columnCount;
    wchar_t blank = [TerminalContents getBlankChar];
    int blank_attr = [TerminalContents getBlankAttribute];

    for (int ir = irow; ir < irow + h; ++ir) {
	for (int ic = icol; ic < icol + w; ++ic) {
	    cellsRow[ic].v.ch.glyph = blank;
	    cellsRow[ic].v.ch.attr = blank_attr;
	    cellsRow[ic].hscl = 1;
	    cellsRow[ic].vscl = 1;
	    cellsRow[ic].hoff_n = 0;
	    cellsRow[ic].voff_n = 0;
	    cellsRow[ic].hoff_d = 1;
	    cellsRow[ic].voff_d = 1;
	    cellsRow[ic].form = TERM_CELL_CHAR;
	}
	cellsRow += self.columnCount;
    }
}

- (void) splitBlockAtColumn:(int)icol row:(int)irow n:(int)nsub
		     blocks:(const struct TerminalCellBlock*)b
{
    const struct TerminalCell *pulold = [self getCellAtColumn:icol row:irow];

    for (int isub = 0; isub < nsub; ++isub) {
	struct TerminalCell* cellsRow =
	    self->cells + b[isub].ulrow * self.columnCount;

	/*
	 * Copy the data from the upper left corner of the big block to
	 * the upper left corner of the piece.
	 */
	if (b[isub].ulcol != icol || b[isub].ulrow != irow) {
	    if (pulold->form == TERM_CELL_CHAR) {
		cellsRow[b[isub].ulcol].v.ch = pulold->v.ch;
		cellsRow[b[isub].ulcol].form = TERM_CELL_CHAR;
	    } else {
		cellsRow[b[isub].ulcol].v.ti = pulold->v.ti;
		cellsRow[b[isub].ulcol].form = TERM_CELL_TILE;
	    }
	}
	cellsRow[b[isub].ulcol].hscl = b[isub].w;
	cellsRow[b[isub].ulcol].vscl = b[isub].h;

	/*
	 * Point the padding elements in the piece to the new upper left
	 * corner.
	 */
	int ic;
	for (ic = b[isub].ulcol + 1; ic < b[isub].ulcol + b[isub].w; ++ic) {
	    cellsRow[ic].v.pd.hoff = ic - b[isub].ulcol;
	    cellsRow[ic].v.pd.voff = 0;
	    cellsRow[ic].hscl = b[isub].w;
	    cellsRow[ic].vscl = b[isub].h;
	}
	cellsRow += self.columnCount;
	for (int ir = b[isub].ulrow + 1;
	     ir < b[isub].ulrow + b[isub].h;
	     ++ir) {
	    for (ic = b[isub].ulcol; ic < b[isub].ulcol + b[isub].w; ++ic) {
		cellsRow[ic].v.pd.hoff = ic - b[isub].ulcol;
		cellsRow[ic].v.pd.voff = ir - b[isub].ulrow;
		cellsRow[ic].hscl = b[isub].w;
		cellsRow[ic].vscl = b[isub].h;
	    }
	    cellsRow += self.columnCount;
	}
    }
}

- (void)checkForBigStuffOverwriteAtColumn:(int)icol row:(int)irow
				    width:(int)w height:(int)h
{
    int ire = irow + h, ice = icol + w;

    for (int ir = irow; ir < ire; ++ir) {
	for (int ic = icol; ic < ice; ++ic) {
	    const struct TerminalCell *pcell =
		[self getCellAtColumn:ic row:ir];

	    if ((pcell->form & (TERM_CELL_CHAR | TERM_CELL_TILE)) != 0 &&
		(pcell->hscl > 1 || pcell->vscl > 1)) {
		/*
		 * Lost chunk including upper left corner.  Split into at most
		 * two new blocks.
		 */
		/*
		 * Tolerate blocks that were clipped by a resize at some point.
		 */
		int wb = (ic + pcell->hscl <= self.columnCount) ?
		    pcell->hscl : self.columnCount - ic;
		int hb = (ir + pcell->vscl <= self.rowCount) ?
		    pcell->vscl : self.rowCount - ir;
		struct TerminalCellBlock blocks[2];
		int nsub = 0, ww, hw;

		if (ice < ic + wb) {
		    /* Have something to the right not overwritten. */
		    blocks[nsub].ulcol = ice;
		    blocks[nsub].ulrow = ir;
		    blocks[nsub].w = ic + wb - ice;
		    blocks[nsub].h = (ire < ir + hb) ? ire - ir : hb;
		    ++nsub;
		    ww = ice - ic;
		} else {
		    ww = wb;
		}
		if (ire < ir + hb) {
		    /* Have something below not overwritten. */
		    blocks[nsub].ulcol = ic;
		    blocks[nsub].ulrow = ire;
		    blocks[nsub].w = wb;
		    blocks[nsub].h = ir + hb - ire;
		    ++nsub;
		    hw = ire - ir;
		} else {
		    hw = hb;
		}
		if (nsub > 0) {
		    [self splitBlockAtColumn:ic row:ir n:nsub blocks:blocks];
		}
		/*
		 * Wipe the part of the block that's destined to be overwritten
		 * so it doesn't receive further consideration in this loop.
		 * For efficiency, would like to have the loop skip over it or
		 * fill it with the desired content, but this is easier to
		 * implement.
		 */
		[self wipeBlockAuxAtColumn:ic row:ir width:ww height:hw];
	    } else if ((pcell->form & (TERM_CELL_CHAR_PADDING |
				       TERM_CELL_TILE_PADDING)) != 0) {
		/*
		 * Lost a chunk that doesn't cover the upper left corner.  In
		 * general will split into up to four new blocks (one above,
		 * one to the left, one to the right, and one below).
		 */
		int pcol = ic - pcell->v.pd.hoff;
		int prow = ir - pcell->v.pd.voff;
		const struct TerminalCell *pcell2 =
		    [self getCellAtColumn:pcol row:prow];

		/*
		 * Tolerate blocks that were clipped by a resize at some point.
		 */
		int wb = (pcol + pcell2->hscl <= self.columnCount) ?
		    pcell2->hscl : self.columnCount - pcol;
		int hb = (prow + pcell2->vscl <= self.rowCount) ?
		    pcell2->vscl : self.rowCount - prow;
		struct TerminalCellBlock blocks[4];
		int nsub = 0, ww, hw;

		if (prow < ir) {
		    /* Have something above not overwritten. */
		    blocks[nsub].ulcol = pcol;
		    blocks[nsub].ulrow = prow;
		    blocks[nsub].w = wb;
		    blocks[nsub].h = ir - prow;
		    ++nsub;
		}
		if (pcol < ic) {
		    /* Have something to the left not overwritten. */
		    blocks[nsub].ulcol = pcol;
		    blocks[nsub].ulrow = ir;
		    blocks[nsub].w = ic - pcol;
		    blocks[nsub].h =
			(ire < prow + hb) ? ire - ir : prow + hb - ir;
		    ++nsub;
		}
		if (ice < pcol + wb) {
		    /* Have something to the right not overwritten. */
		    blocks[nsub].ulcol = ice;
		    blocks[nsub].ulrow = ir;
		    blocks[nsub].w = pcol + wb - ice;
		    blocks[nsub].h =
			(ire < prow + hb) ? ire - ir : prow + hb - ir;
		    ++nsub;
		    ww = ice - ic;
		} else {
		    ww = pcol + wb - ic;
		}
		if (ire < prow + hb) {
		    /* Have something below not overwritten. */
		    blocks[nsub].ulcol = pcol;
		    blocks[nsub].ulrow = ire;
		    blocks[nsub].w = wb;
		    blocks[nsub].h = prow + hb - ire;
		    ++nsub;
		    hw = ire - ir;
		} else {
		    hw = prow + hb - ir;
		}

		[self splitBlockAtColumn:pcol row:prow n:nsub blocks:blocks];
		/* Same rationale for wiping as above. */
		[self wipeBlockAuxAtColumn:ic row:ir width:ww height:hw];
	    }
	}
    }
}

- (void)setCursorAtColumn:(int)icol row:(int)irow width:(int)w height:(int)h
{
    self->_cursorColumn = icol;
    self->_cursorRow = irow;
    self->_cursorWidth = w;
    self->_cursorHeight = h;
}

- (void)removeCursor
{
    self->_cursorColumn = -1;
    self->_cursorHeight = -1;
    self->_cursorWidth = 1;
    self->_cursorHeight = 1;
}

- (void)assertInvariants
{
    const struct TerminalCell *cellsRow = self->cells;

    /*
     * The comments with the definition for TerminalCell define the
     * relationships of hoff_n, voff_n, hoff_d, voff_d, hscl, and vscl
     * asserted here.
     */
    for (int ir = 0; ir < self.rowCount; ++ir) {
	for (int ic = 0; ic < self.columnCount; ++ic) {
	    switch (cellsRow[ic].form) {
	    case TERM_CELL_CHAR:
		assert(cellsRow[ic].hscl > 0 && cellsRow[ic].vscl > 0);
		assert(cellsRow[ic].hoff_n < cellsRow[ic].hoff_d &&
		       cellsRow[ic].voff_n < cellsRow[ic].voff_d);
		if (cellsRow[ic].hscl == cellsRow[ic].hoff_d) {
		    assert(cellsRow[ic].hoff_n == 0);
		}
		if (cellsRow[ic].vscl == cellsRow[ic].voff_d) {
		    assert(cellsRow[ic].voff_n == 0);
		}
		/*
		 * Verify that the padding elements have the correct tag
		 * and point back to this cell.
		 */
		if (cellsRow[ic].hscl > 1 || cellsRow[ic].vscl > 1) {
		    const struct TerminalCell *cellsRow2 = cellsRow;

		    for (int ir2 = ir; ir2 < ir + cellsRow[ic].vscl; ++ir2) {
			for (int ic2 = ic;
			     ic2 < ic + cellsRow[ic].hscl;
			     ++ic2) {
			    if (ir2 == ir && ic2 == ic) {
				continue;
			    }
			    assert(cellsRow2[ic2].form ==
				   TERM_CELL_CHAR_PADDING);
			    assert(ic2 - cellsRow2[ic2].v.pd.hoff == ic &&
				   ir2 - cellsRow2[ic2].v.pd.voff == ir);
			}
			cellsRow2 += self.columnCount;
		    }
		}
		break;

	    case TERM_CELL_TILE:
		assert(cellsRow[ic].hscl > 0 && cellsRow[ic].vscl > 0);
		assert(cellsRow[ic].hoff_n < cellsRow[ic].hoff_d &&
		       cellsRow[ic].voff_n < cellsRow[ic].voff_d);
		if (cellsRow[ic].hscl == cellsRow[ic].hoff_d) {
		    assert(cellsRow[ic].hoff_n == 0);
		}
		if (cellsRow[ic].vscl == cellsRow[ic].voff_d) {
		    assert(cellsRow[ic].voff_n == 0);
		}
		/*
		 * Verify that the padding elements have the correct tag
		 * and point back to this cell.
		 */
		if (cellsRow[ic].hscl > 1 || cellsRow[ic].vscl > 1) {
		    const struct TerminalCell *cellsRow2 = cellsRow;

		    for (int ir2 = ir; ir2 < ir + cellsRow[ic].vscl; ++ir2) {
			for (int ic2 = ic;
			     ic2 < ic + cellsRow[ic].hscl;
			     ++ic2) {
			    if (ir2 == ir && ic2 == ic) {
				continue;
			    }
			    assert(cellsRow2[ic2].form ==
				   TERM_CELL_TILE_PADDING);
			    assert(ic2 - cellsRow2[ic2].v.pd.hoff == ic &&
				   ir2 - cellsRow2[ic2].v.pd.voff == ir);
			}
			cellsRow2 += self.columnCount;
		    }
		}
		break;

	    case TERM_CELL_CHAR_PADDING:
		assert(cellsRow[ic].hscl > 0 && cellsRow[ic].vscl > 0);
		assert(cellsRow[ic].hoff_n < cellsRow[ic].hoff_d &&
		       cellsRow[ic].voff_n < cellsRow[ic].voff_d);
		assert(cellsRow[ic].hoff_n > 0 || cellsRow[ic].voff_n > 0);
		if (cellsRow[ic].hscl == cellsRow[ic].hoff_d) {
		    assert(cellsRow[ic].hoff_n == cellsRow[ic].v.pd.hoff);
		}
		if (cellsRow[ic].vscl == cellsRow[ic].voff_d) {
		    assert(cellsRow[ic].voff_n == cellsRow[ic].v.pd.voff);
		}
		assert(ic >= cellsRow[ic].v.pd.hoff &&
		       ir >= cellsRow[ic].v.pd.voff);
		/*
		 * Verify that it's padding for something that can point
		 * back to it.
		 */
		{
		    const struct TerminalCell *parent =
			[self getCellAtColumn:(ic - cellsRow[ic].v.pd.hoff)
			      row:(ir - cellsRow[ic].v.pd.voff)];

		    assert(parent->form == TERM_CELL_CHAR);
		    assert(parent->hscl > cellsRow[ic].v.pd.hoff &&
			   parent->vscl > cellsRow[ic].v.pd.voff);
		    assert(parent->hscl == cellsRow[ic].hscl &&
			   parent->vscl == cellsRow[ic].vscl);
		    assert(parent->hoff_d == cellsRow[ic].hoff_d &&
			   parent->voff_d == cellsRow[ic].voff_d);
		}
		break;

	    case TERM_CELL_TILE_PADDING:
		assert(cellsRow[ic].hscl > 0 && cellsRow[ic].vscl > 0);
		assert(cellsRow[ic].hoff_n < cellsRow[ic].hoff_d &&
		       cellsRow[ic].voff_n < cellsRow[ic].voff_d);
		assert(cellsRow[ic].hoff_n > 0 || cellsRow[ic].voff_n > 0);
		if (cellsRow[ic].hscl == cellsRow[ic].hoff_d) {
		    assert(cellsRow[ic].hoff_n == cellsRow[ic].v.pd.hoff);
		}
		if (cellsRow[ic].vscl == cellsRow[ic].voff_d) {
		    assert(cellsRow[ic].voff_n == cellsRow[ic].v.pd.voff);
		}
		assert(ic >= cellsRow[ic].v.pd.hoff &&
		       ir >= cellsRow[ic].v.pd.voff);
		/*
		 * Verify that it's padding for something that can point
		 * back to it.
		 */
		{
		    const struct TerminalCell *parent =
			[self getCellAtColumn:(ic - cellsRow[ic].v.pd.hoff)
			      row:(ir - cellsRow[ic].v.pd.voff)];

		    assert(parent->form == TERM_CELL_TILE);
		    assert(parent->hscl > cellsRow[ic].v.pd.hoff &&
			   parent->vscl > cellsRow[ic].v.pd.voff);
		    assert(parent->hscl == cellsRow[ic].hscl &&
			   parent->vscl == cellsRow[ic].vscl);
		    assert(parent->hoff_d == cellsRow[ic].hoff_d &&
			   parent->voff_d == cellsRow[ic].voff_d);
		}
		break;

	    default:
		assert(0);
	    }
	}
	cellsRow += self.columnCount;
    }
}

+ (wchar_t)getBlankChar
{
    return L' ';
}

+ (int)getBlankAttribute
{
    return 0;
}

@end

/**
 * TerminalChanges is used to track changes made via the text_hook, pict_hook,
 * wipe_hook, curs_hook, and bigcurs_hook callbacks on the terminal since the
 * last call to xtra_hook for TERM_XTRA_FRESH.  The locations marked as changed
 * can then be used to make bounding rectangles for the regions that need to
 * be redisplayed.
 */
@interface TerminalChanges : NSObject {
    int* colBounds;
    /*
     * Outside of firstChangedRow, lastChangedRow and what's in colBounds, the
     * contents of this are handled lazily.
     */
    BOOL* marks;
}

/**
 * Initialize with zero columns and zero rows.
 */
- (id)init;

/**
 * Initialize with nCol columns and nRow rows.  No changes will be marked.
 */
- (id)initWithColumns:(int)nCol rows:(int)nRow NS_DESIGNATED_INITIALIZER;

/**
 * Resize to be nCol by nRow.  Current contents still within the new bounds
 * are preserved.  Added areas are marked as unchanged.
 */
- (void)resizeWithColumns:(int)nCol rows:(int)nRow;

/**
 * Clears all marked changes.
 */
- (void)clear;

- (BOOL)isChangedAtColumn:(int)icol row:(int)irow;

/**
 * Scans the row, irow, starting at the column, icol0, and stopping before the
 * column, icol1.  Returns the column index for the first cell that is
 * changed.  The returned index will be equal to icol1 if all of the cells in
 * the range are unchanged.
 */
- (int)scanForChangedInRow:(int)irow col0:(int)icol0 col1:(int)icol1;

/**
 * Scans the row, irow, starting at the column, icol0, and stopping before the
 * column, icol1.  returns the column index for the first cell that has not
 * changed.  The returned index will be equal to icol1 if all of the cells in
 * the range have changed.
 */
- (int)scanForUnchangedInRow:(int)irow col0:(int)icol0 col1:(int)icol1;

- (void)markChangedAtColumn:(int)icol row:(int)irow;

- (void)markChangedRangeAtColumn:(int)icol row:(int)irow width:(int)w;

/**
 * Marks the block as changed who's upper left hand corner is at (icol, irow).
 */
- (void)markChangedBlockAtColumn:(int)icol
			     row:(int)irow
			   width:(int)w
			  height:(int)h;

/**
 * Returns the index of the first changed column in the given row.  That index
 * will be equal to the number of columns if there are no changes in the row.
 */
- (int)getFirstChangedColumnInRow:(int)irow;

/**
 * Returns the index of the last changed column in the given row.  That index
 * will be equal to -1 if there are no changes in the row.
 */
- (int)getLastChangedColumnInRow:(int)irow;

/**
 * Is the number of columns.
 */
@property (readonly) int columnCount;

/**
 * Is the number of rows.
 */
@property (readonly) int rowCount;

/**
 * Is the index of the first row with changes.  Will be equal to the number
 * of rows if there are no changes.
 */
@property (readonly) int firstChangedRow;

/**
 * Is the index of the last row with changes.  Will be equal to -1 if there
 * are no changes.
 */
@property (readonly) int lastChangedRow;

@end

@implementation TerminalChanges

- (id)init
{
    return [self initWithColumns:0 rows:0];
}

- (id)initWithColumns:(int)nCol rows:(int)nRow
{
    if (self = [super init]) {
	self->colBounds = malloc(2 * nRow * sizeof(int));
	self->marks = malloc(nCol * nRow * sizeof(BOOL));
	self->_columnCount = nCol;
	self->_rowCount = nRow;
	[self clear];
    }
    return self;
}

- (void)dealloc
{
    if (self->marks != 0) {
	free(self->marks);
	self->marks = 0;
    }
    if (self->colBounds != 0) {
	free(self->colBounds);
	self->colBounds = 0;
    }
}

- (void)resizeWithColumns:(int)nCol rows:(int)nRow
{
    int* newColBounds = malloc(2 * nRow * sizeof(int));
    BOOL* newMarks = malloc(nCol * nRow * sizeof(BOOL));
    int nRowCommon = (nRow < self.rowCount) ? nRow : self.rowCount;

    if (self.firstChangedRow <= self.lastChangedRow &&
	self.firstChangedRow < nRowCommon) {
	BOOL* marksOutCursor = newMarks + self.firstChangedRow * nCol;
	const BOOL* marksInCursor =
	    self->marks + self.firstChangedRow * self.columnCount;
	int nColCommon = (nCol < self.columnCount) ? nCol : self.columnCount;

	if (self.lastChangedRow >= nRowCommon) {
	    self->_lastChangedRow = nRowCommon - 1;
	}
	for (int i = self.firstChangedRow; i <= self.lastChangedRow; ++i) {
	    if (self->colBounds[i + i] < nColCommon) {
		newColBounds[i + i] = self->colBounds[i + i];
		newColBounds[i + i + 1] =
		    (self->colBounds[i + i + 1] < nColCommon) ?
		    self->colBounds[i + i + 1] : nColCommon - 1;
		(void) memcpy(
		    marksOutCursor + self->colBounds[i + i],
		    marksInCursor + self->colBounds[i + i],
		    (newColBounds[i + i + 1] - newColBounds[i + i] + 1) *
		        sizeof(BOOL));
		marksInCursor += self.columnCount;
		marksOutCursor += nCol;
	    } else {
		self->colBounds[i + i] = nCol;
		self->colBounds[i + i + 1] = -1;
	    }
	}
    } else {
	self->_firstChangedRow = nRow;
	self->_lastChangedRow = -1;
    }

    free(self->colBounds);
    self->colBounds = newColBounds;
    free(self->marks);
    self->marks = newMarks;
    self->_columnCount = nCol;
    self->_rowCount = nRow;
}

- (void)clear
{
    self->_firstChangedRow = self.rowCount;
    self->_lastChangedRow = -1;
}

- (BOOL)isChangedAtColumn:(int)icol row:(int)irow
{
    if (irow < self.firstChangedRow || irow > self.lastChangedRow) {
	return NO;
    }
    if (icol < self->colBounds[irow + irow] ||
	icol > self->colBounds[irow + irow + 1]) {
	return NO;
    }
    return self->marks[icol + irow * self.columnCount];
}

- (int)scanForChangedInRow:(int)irow col0:(int)icol0 col1:(int)icol1
{
    if (irow < self.firstChangedRow || irow > self.lastChangedRow ||
	icol0 > self->colBounds[irow + irow + 1]) {
	return icol1;
    }

    int i = (icol0 > self->colBounds[irow + irow]) ?
	icol0 : self->colBounds[irow + irow];
    int i1 = (icol1 <= self->colBounds[irow + irow + 1]) ?
	icol1 : self->colBounds[irow + irow + 1] + 1;
    const BOOL* marksCursor = self->marks + irow * self.columnCount;
    while (1) {
	if (i >= i1) {
	    return icol1;
	}
	if (marksCursor[i]) {
	    return i;
	}
	++i;
    }
}

- (int)scanForUnchangedInRow:(int)irow col0:(int)icol0 col1:(int)icol1
{
    if (irow < self.firstChangedRow || irow > self.lastChangedRow ||
	icol0 < self->colBounds[irow + irow] ||
	icol0 > self->colBounds[irow + irow + 1]) {
	return icol0;
    }

    int i = icol0;
    int i1 = (icol1 <= self->colBounds[irow + irow + 1]) ?
	icol1 : self->colBounds[irow + irow + 1] + 1;
    const BOOL* marksCursor = self->marks + irow * self.columnCount;
    while (1) {
	if (i >= i1 || ! marksCursor[i]) {
	    return i;
	}
	++i;
    }
}

- (void)markChangedAtColumn:(int)icol row:(int)irow
{
    [self markChangedBlockAtColumn:icol row:irow width:1 height:1];
}

- (void)markChangedRangeAtColumn:(int)icol row:(int)irow width:(int)w
{
    [self markChangedBlockAtColumn:icol row:irow width:w height:1];
}

- (void)markChangedBlockAtColumn:(int)icol
			     row:(int)irow
			   width:(int)w
			  height:(int)h
{
    if (irow + h <= self.firstChangedRow) {
	/* All prior marked regions are on rows after the requested block. */
	if (self.firstChangedRow > self.lastChangedRow) {
	    self->_lastChangedRow = irow + h - 1;
	} else {
	    for (int i = irow + h; i < self.firstChangedRow; ++i) {
		self->colBounds[i + i] = self.columnCount;
		self->colBounds[i + i + 1] = -1;
	    }
	}
	self->_firstChangedRow = irow;

	BOOL* marksCursor = self->marks + irow * self.columnCount;
	for (int i = irow; i < irow + h; ++i) {
	    self->colBounds[i + i] = icol;
	    self->colBounds[i + i + 1] = icol + w - 1;
	    for (int j = icol; j < icol + w; ++j) {
		marksCursor[j] = YES;
	    }
	    marksCursor += self.columnCount;
	}
    } else if (irow > self.lastChangedRow) {
	/* All prior marked regions are on rows before the requested block. */
	int i;

	for (i = self.lastChangedRow + 1; i < irow; ++i) {
	    self->colBounds[i + i] = self.columnCount;
	    self->colBounds[i + i + 1] = -1;
	}
	self->_lastChangedRow = irow + h - 1;

	BOOL* marksCursor = self->marks + irow * self.columnCount;
	for (i = irow; i < irow + h; ++i) {
	    self->colBounds[i + i] = icol;
	    self->colBounds[i + i + 1] = icol + w - 1;
	    for (int j = icol; j < icol + w; ++j) {
		marksCursor[j] = YES;
	    }
	    marksCursor += self.columnCount;
	}
    } else {
	/*
	 * There's overlap between the rows of the requested block and prior
	 * marked regions.
	 */
	BOOL* marksCursor = self->marks + irow * self.columnCount;
	int irow0, h0;

	if (irow < self.firstChangedRow) {
	    /* Handle any leading rows where there's no overlap. */
	    for (int i = irow; i < self.firstChangedRow; ++i) {
		self->colBounds[i + i] = icol;
		self->colBounds[i + i + 1] = icol + w - 1;
		for (int j = icol; j < icol + w; ++j) {
		    marksCursor[j] = YES;
		}
		marksCursor += self.columnCount;
	    }
	    irow0 = self.firstChangedRow;
	    h0 = irow + h - self.firstChangedRow;
	    self->_firstChangedRow = irow;
	} else {
	    irow0 = irow;
	    h0 = h;
	}

	/* Handle potentially overlapping rows */
	if (irow0 + h0 > self.lastChangedRow + 1) {
	    h0 = self.lastChangedRow + 1 - irow0;
	    self->_lastChangedRow = irow + h - 1;
	}

	int i;
	for (i = irow0; i < irow0 + h0; ++i) {
	    if (icol + w <= self->colBounds[i + i]) {
		int j;

		for (j = icol; j < icol + w; ++j) {
		    marksCursor[j] = YES;
		}
		if (self->colBounds[i + i] > self->colBounds[i + i + 1]) {
		    self->colBounds[i + i + 1] = icol + w - 1;
		} else {
		    for (j = icol + w; j < self->colBounds[i + i]; ++j) {
			marksCursor[j] = NO;
		    }
		}
		self->colBounds[i + i] = icol;
	    } else if (icol > self->colBounds[i + i + 1]) {
		int j;

		for (j = self->colBounds[i + i + 1] + 1; j < icol; ++j) {
		    marksCursor[j] = NO;
		}
		for (j = icol; j < icol + w; ++j) {
		    marksCursor[j] = YES;
		}
		self->colBounds[i + i + 1] = icol + w - 1;
	    } else {
		if (icol < self->colBounds[i + i]) {
		    self->colBounds[i + i] = icol;
		}
		if (icol + w > self->colBounds[i + i + 1]) {
		    self->colBounds[i + i + 1] = icol + w - 1;
		}
		for (int j = icol; j < icol + w; ++j) {
		    marksCursor[j] = YES;
		}
	    }
	    marksCursor += self.columnCount;
	}

	/* Handle any trailing rows where there's no overlap. */
	for (i = irow0 + h0; i < irow + h; ++i) {
	    self->colBounds[i + i] = icol;
	    self->colBounds[i + i + 1] = icol + w - 1;
	    for (int j = icol; j < icol + w; ++j) {
		marksCursor[j] = YES;
	    }
	    marksCursor += self.columnCount;
	}
    }
}

- (int)getFirstChangedColumnInRow:(int)irow
{
    if (irow < self.firstChangedRow || irow > self.lastChangedRow) {
	return self.columnCount;
    }
    return self->colBounds[irow + irow];
}

- (int)getLastChangedColumnInRow:(int)irow
{
    if (irow < self.firstChangedRow || irow > self.lastChangedRow) {
	return -1;
    }
    return self->colBounds[irow + irow + 1];
}

@end


/**
 * Draws one tile as a helper function for AngbandContext's drawRect.
 */
static void draw_image_tile(
    NSGraphicsContext* nsContext,
    CGContextRef cgContext,
    CGImageRef image,
    NSRect srcRect,
    NSRect dstRect,
    NSCompositingOperation op)
{
    /* Flip the source rect since the source image is flipped */
    CGAffineTransform flip = CGAffineTransformIdentity;
    flip = CGAffineTransformTranslate(flip, 0.0, CGImageGetHeight(image));
    flip = CGAffineTransformScale(flip, 1.0, -1.0);
    CGRect flippedSourceRect =
	CGRectApplyAffineTransform(NSRectToCGRect(srcRect), flip);

    /*
     * When we use high-quality resampling to draw a tile, pixels from outside
     * the tile may bleed in, causing graphics artifacts. Work around that.
     */
    CGImageRef subimage =
	CGImageCreateWithImageInRect(image, flippedSourceRect);
    [nsContext setCompositingOperation:op];
    CGContextDrawImage(cgContext, NSRectToCGRect(dstRect), subimage);
    CGImageRelease(subimage);
}


/*
 * The max number of glyphs we support.  Currently this only affects
 * updateGlyphInfo() for the calculation of the tile size, fontAscender,
 * fontDescender, nColPre, and nColPost.  The rendering in drawWChar() will
 * work for a glyph not in updateGlyphInfo()'s set, though there may be
 * clipping or clearing artifacts because it wasn't included in
 * updateGlyphInfo()'s calculations.
 */
#define GLYPH_COUNT 256

/*
 * An AngbandContext represents a logical Term (i.e. what Angband thinks is
 * a window).
 */
@interface AngbandContext : NSObject <NSWindowDelegate>
{
@public

    /* The Angband term */
    term *terminal;

@private
    /* Is the last time we drew, so we can throttle drawing. */
    CFAbsoluteTime lastRefreshTime;

    /* Flags whether or not a fullscreen transition is in progress. */
    BOOL inFullscreenTransition;

    /* Our view */
    AngbandView *angbandView;
}

/* Column and row counts, by default 80 x 24 */
@property (readonly) int cols;
@property (readonly) int rows;

/* The size of the border between the window edge and the contents */
@property (readonly) NSSize borderSize;

/* The font of this context */
@property NSFont *angbandViewFont;

/* The size of one tile */
@property (readonly) NSSize tileSize;

/* Font's ascender and descender */
@property (readonly) CGFloat fontAscender;
@property (readonly) CGFloat fontDescender;

/*
 * These are the number of columns before or after, respectively, a text
 * change that may need to be redrawn.
 */
@property (readonly) int nColPre;
@property (readonly) int nColPost;

/* If this context owns a window, here it is. */
@property NSWindow *primaryWindow;

/* Holds our version of the contents of the terminal. */
@property TerminalContents *contents;

/*
 * Marks which locations have been changed by the text_hook, pict_hook,
 * wipe_hook, curs_hook, and bigcurs_hhok callbacks on the terminal since
 * the last call to xtra_hook with TERM_XTRA_FRESH.
 */
@property TerminalChanges *changes;

@property (nonatomic, assign) BOOL hasSubwindowFlags;
@property (nonatomic, assign) BOOL windowVisibilityChecked;

- (void)resizeWithColumns:(int)nCol rows:(int)nRow;

/**
 * Based on what has been marked as changed, inform AppKit of the bounding
 * rectangles for the changed areas.
 */
- (void)computeInvalidRects;

- (void)drawRect:(NSRect)rect inView:(NSView *)view;

/* Called at initialization to set the term */
- (void)setTerm:(term *)t;

/* Called when the context is going down. */
- (void)dispose;

/*
 * Return the rect in view coordinates for the block of cells whose upper
 * left corner is (x,y).
 */
- (NSRect)viewRectForCellBlockAtX:(int)x y:(int)y width:(int)w height:(int)h;

/* Draw the given wide character into the given tile rect. */
- (void)drawWChar:(wchar_t)wchar inRect:(NSRect)tile screenFont:(NSFont*)font
	  context:(CGContextRef)ctx;

/*
 * Returns the primary window for this angband context, creating it if
 * necessary
 */
- (NSWindow *)makePrimaryWindow;

/* Handle becoming the main window */
- (void)windowDidBecomeMain:(NSNotification *)notification;

/* Return whether the context's primary window is ordered in or not */
- (BOOL)isOrderedIn;

/* Return whether the context's primary window is key */
- (BOOL)isMainWindow;

/* Invalidate the whole image */
- (void)setNeedsDisplay:(BOOL)val;

/* Invalidate part of the image, with the rect expressed in view coordinates */
- (void)setNeedsDisplayInRect:(NSRect)rect;

/* Display (flush) our Angband views */
- (void)displayIfNeeded;

/*
 * Resize context to size of contentRect, and optionally save size to
 * defaults
 */
- (void)resizeTerminalWithContentRect: (NSRect)contentRect saveToDefaults: (BOOL)saveToDefaults;

/*
 * Change the minimum size and size increments for the window associated with
 * the context.  termIdx is the index for the terminal:  pass it so this
 * function can be used when self->terminal has not yet been set.
 */
- (void)constrainWindowSize:(int)termIdx;

- (void)saveWindowVisibleToDefaults: (BOOL)windowVisible;
- (BOOL)windowVisibleUsingDefaults;

/* Class methods */
/**
 * Gets the default font for all contexts.  Currently not declaring this as
 * a class property for compatibility with versions of Xcode prior to 8.
 */
+ (NSFont*)defaultFont;
/**
 * Sets the default font for all contexts.
 */
+ (void)setDefaultFont:(NSFont*)font;

@end

/**
 * Generate a mask for the subwindow flags. The mask is just a safety check to
 * make sure that our windows show and hide as expected.  This function allows
 * for future changes to the set of flags without needed to update it here
 * (unless the underlying types change).
 */
u32b AngbandMaskForValidSubwindowFlags(void)
{
    int windowFlagBits = sizeof(*(window_flag)) * CHAR_BIT;
    int maxBits = MIN( PW_MAX_FLAGS, windowFlagBits );
    u32b mask = 0;

    for( int i = 0; i < maxBits; i++ )
    {
        if( window_flag_desc[i] != NULL )
        {
            mask |= (1 << i);
        }
    }

    return mask;
}

/**
 * Check for changes in the subwindow flags and update window visibility.
 * This seems to be called for every user event, so we don't
 * want to do any unnecessary hiding or showing of windows.
 */
static void AngbandUpdateWindowVisibility(void)
{
    /*
     * Because this function is called frequently, we'll make the mask static.
     * It doesn't change between calls, as the flags themselves are hardcoded
     */
    static u32b validWindowFlagsMask = 0;

    if( validWindowFlagsMask == 0 )
    {
        validWindowFlagsMask = AngbandMaskForValidSubwindowFlags();
    }

    /*
     * Loop through all of the subwindows and see if there is a change in the
     * flags. If so, show or hide the corresponding window. We don't care about
     * the flags themselves; we just want to know if any are set.
     */
    for( int i = 1; i < ANGBAND_TERM_MAX; i++ )
    {
        AngbandContext *angbandContext =
	    (__bridge AngbandContext*) (angband_term[i]->data);

        if( angbandContext == nil )
        {
            continue;
        }

        /*
         * This horrible mess of flags is so that we can try to maintain some
         * user visibility preference. This should allow the user a window and
         * have it stay closed between application launches. However, this
         * means that when a subwindow is turned on, it will no longer appear
         * automatically. Angband has no concept of user control over window
         * visibility, other than the subwindow flags.
         */
        if( !angbandContext.windowVisibilityChecked )
        {
            if( [angbandContext windowVisibleUsingDefaults] )
            {
                [angbandContext.primaryWindow orderFront: nil];
                angbandContext.windowVisibilityChecked = YES;
            }
            else
            {
                [angbandContext.primaryWindow close];
                angbandContext.windowVisibilityChecked = NO;
            }
        }
        else
        {
            BOOL termHasSubwindowFlags = ((window_flag[i] & validWindowFlagsMask) > 0);

            if( angbandContext.hasSubwindowFlags && !termHasSubwindowFlags )
            {
                [angbandContext.primaryWindow close];
                angbandContext.hasSubwindowFlags = NO;
                [angbandContext saveWindowVisibleToDefaults: NO];
            }
            else if( !angbandContext.hasSubwindowFlags && termHasSubwindowFlags )
            {
                [angbandContext.primaryWindow orderFront: nil];
                angbandContext.hasSubwindowFlags = YES;
                [angbandContext saveWindowVisibleToDefaults: YES];
            }
        }
    }

    /* Make the main window key so that user events go to the right spot */
    AngbandContext *mainWindow =
	(__bridge AngbandContext*) (angband_term[0]->data);
    [mainWindow.primaryWindow makeKeyAndOrderFront: nil];
}

/**
 * ------------------------------------------------------------------------
 * Graphics support
 * ------------------------------------------------------------------------ */

/**
 * The tile image
 */
static CGImageRef pict_image;

/**
 * Numbers of rows and columns in a tileset,
 * calculated by the PICT/PNG loading code
 */
static int pict_cols = 0;
static int pict_rows = 0;

/**
 * Requested graphics mode (as a grafID).
 * The current mode is stored in current_graphics_mode.
 */
static int graf_mode_req = 0;

/**
 * Will be nonzero if tile_width and tile_height have changed since the last
 * redraw.
 */
static int tile_multipliers_changed = 0;

/**
 * Helper function to check the various ways that graphics can be enabled,
 * guarding against NULL
 */
static BOOL graphics_are_enabled(void)
{
    return current_graphics_mode && current_graphics_mode->grafID != GRAPHICS_NONE;
}

/**
 * Like graphics_are_enabled(), but test the requested graphics mode.
 */
static BOOL graphics_will_be_enabled(void)
{
    if (graf_mode_req == GRAPHICS_NONE) {
	return NO;
    }

    graphics_mode *new_mode = get_graphics_mode(graf_mode_req);
    return new_mode && new_mode->grafID != GRAPHICS_NONE;
}

/**
 * Hack -- game in progress
 */
static Boolean game_in_progress = FALSE;


#pragma mark Prototypes
static BOOL redraw_for_tiles_or_term0_font(void);
static void wakeup_event_loop(void);
static void hook_plog(const char *str);
static void hook_quit(const char * str);
static NSString* get_lib_directory(void);
static NSString* get_doc_directory(void);
static NSString* AngbandCorrectedDirectoryPath(NSString *originalPath);
static void prepare_paths_and_directories(void);
static void load_prefs(void);
static void init_windows(void);
static void handle_open_when_ready(void);
static void play_sound(game_event_type unused, game_event_data *data, void *user);
static BOOL check_events(int wait);
static void cocoa_file_open_hook(const char *path, file_type ftype);
static bool cocoa_get_file(const char *suggested_name, char *path, size_t len);
static BOOL send_event(NSEvent *event);
static void set_color_for_index(int idx);
static void record_current_savefile(void);

/**
 * Available values for 'wait'
 */
#define CHECK_EVENTS_DRAIN -1
#define CHECK_EVENTS_NO_WAIT	0
#define CHECK_EVENTS_WAIT 1


/**
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;

/* Methods for getting the appropriate NSUserDefaults */
@interface NSUserDefaults (AngbandDefaults)
+ (NSUserDefaults *)angbandDefaults;
@end

@implementation NSUserDefaults (AngbandDefaults)
+ (NSUserDefaults *)angbandDefaults
{
    return [NSUserDefaults standardUserDefaults];
}
@end

/*
 * Methods for pulling images out of the Angband bundle (which may be separate
 * from the current bundle in the case of a screensaver
 */
@interface NSImage (AngbandImages)
+ (NSImage *)angbandImage:(NSString *)name;
@end

/* The NSView subclass that draws our Angband image */
@interface AngbandView : NSView {
@private
    NSBitmapImageRep *cacheForResize;
    NSRect cacheBounds;
}

@property (nonatomic, weak) AngbandContext *angbandContext;

@end

@implementation NSImage (AngbandImages)

/*
 * Returns an image in the resource directoy of the bundle containing the
 * Angband view class.
 */
+ (NSImage *)angbandImage:(NSString *)name
{
    NSBundle *bundle = [NSBundle bundleForClass:[AngbandView class]];
    NSString *path = [bundle pathForImageResource:name];
    return (path) ? [[NSImage alloc] initByReferencingFile:path] : nil;
}

@end


@implementation AngbandContext

- (NSSize)baseSize
{
    /*
     * We round the base size down. If we round it up, I believe we may end up
     * with pixels that nobody "owns" that may accumulate garbage. In general
     * rounding down is harmless, because any lost pixels may be sopped up by
     * the border.
     */
    return NSMakeSize(
	floor(self.cols * self.tileSize.width + 2 * self.borderSize.width),
	floor(self.rows * self.tileSize.height + 2 * self.borderSize.height));
}

/* qsort-compatible compare function for CGSizes */
static int compare_advances(const void *ap, const void *bp)
{
    const CGSize *a = ap, *b = bp;
    return (a->width > b->width) - (a->width < b->width);
}

/**
 * Precompute certain metrics (tileSize, fontAscender, fontDescender, nColPre,
 * and nColPost) for the current font.
 */
- (void)updateGlyphInfo
{
    NSFont *screenFont = [self.angbandViewFont screenFont];

    /* Generate a string containing each MacRoman character */
    /*
     * Here and below, dynamically allocate working arrays rather than put them
     * on the stack in case limited stack space is an issue.
     */
    unsigned char *latinString = malloc(GLYPH_COUNT);
    if (latinString == 0) {
	NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					reason:@"latinString in updateGlyphInfo"
					userInfo:nil];
	@throw exc;
    }
    size_t i;
    for (i=0; i < GLYPH_COUNT; i++) latinString[i] = (unsigned char)i;

    /* Turn that into unichar. Angband uses ISO Latin 1. */
    NSString *allCharsString = [[NSString alloc] initWithBytes:latinString
        length:GLYPH_COUNT encoding:NSISOLatin1StringEncoding];
    unichar *unicharString = malloc(GLYPH_COUNT * sizeof(unichar));
    if (unicharString == 0) {
	free(latinString);
	NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					reason:@"unicharString in updateGlyphInfo"
					userInfo:nil];
	@throw exc;
    }
    unicharString[0] = 0;
    [allCharsString getCharacters:unicharString range:NSMakeRange(0, MIN(GLYPH_COUNT, [allCharsString length]))];
    allCharsString = nil;
    free(latinString);

    /* Get glyphs */
    CGGlyph *glyphArray = calloc(GLYPH_COUNT, sizeof(CGGlyph));
    if (glyphArray == 0) {
	free(unicharString);
	NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					reason:@"glyphArray in updateGlyphInfo"
					userInfo:nil];
	@throw exc;
    }
    CTFontGetGlyphsForCharacters((CTFontRef)screenFont, unicharString,
				 glyphArray, GLYPH_COUNT);
    free(unicharString);

    /* Get advances. Record the max advance. */
    CGSize *advances = malloc(GLYPH_COUNT * sizeof(CGSize));
    if (advances == 0) {
	free(glyphArray);
	NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					reason:@"advances in updateGlyphInfo"
					userInfo:nil];
	@throw exc;
    }
    CTFontGetAdvancesForGlyphs(
	(CTFontRef)screenFont, kCTFontHorizontalOrientation, glyphArray,
	advances, GLYPH_COUNT);
    CGFloat *glyphWidths = malloc(GLYPH_COUNT * sizeof(CGFloat));
    if (glyphWidths == 0) {
	free(glyphArray);
	free(advances);
	NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					reason:@"glyphWidths in updateGlyphInfo"
					userInfo:nil];
	@throw exc;
    }
    for (i=0; i < GLYPH_COUNT; i++) {
        glyphWidths[i] = advances[i].width;
    }

    /*
     * For good non-mono-font support, use the median advance. Start by sorting
     * all advances.
     */
    qsort(advances, GLYPH_COUNT, sizeof *advances, compare_advances);

    /* Skip over any initially empty run */
    size_t startIdx;
    for (startIdx = 0; startIdx < GLYPH_COUNT; startIdx++)
    {
        if (advances[startIdx].width > 0) break;
    }

    /* Pick the center to find the median */
    CGFloat medianAdvance = 0;
    /* In case we have all zero advances for some reason */
    if (startIdx < GLYPH_COUNT)
    {
        medianAdvance = advances[(startIdx + GLYPH_COUNT)/2].width;
    }

    free(advances);

    /*
     * Record the ascender and descender.  Some fonts, for instance DIN
     * Condensed and Rockwell in 10.14, the ascent on '@' exceeds that
     * reported by [screenFont ascender].  Get the overall bounding box
     * for the glyphs and use that instead of the ascender and descender
     * values if the bounding box result extends farther from the baseline.
     */
    CGRect bounds = CTFontGetBoundingRectsForGlyphs(
	(CTFontRef) screenFont, kCTFontHorizontalOrientation, glyphArray,
	NULL, GLYPH_COUNT);
    self->_fontAscender = [screenFont ascender];
    if (self->_fontAscender < bounds.origin.y + bounds.size.height) {
	self->_fontAscender = bounds.origin.y + bounds.size.height;
    }
    self->_fontDescender = [screenFont descender];
    if (self->_fontDescender > bounds.origin.y) {
	self->_fontDescender = bounds.origin.y;
    }

    /*
     * Record the tile size.  Round both values up to have tile boundaries
     * match pixel boundaries.
     */
    self->_tileSize.width = ceil(medianAdvance);
    self->_tileSize.height = ceil(self.fontAscender - self.fontDescender);

    /*
     * Determine whether neighboring columns need to be redrawn when a
     * character changes.
     */
    CGRect *boxes = malloc(GLYPH_COUNT * sizeof(CGRect));
    if (boxes == 0) {
	free(glyphWidths);
	free(glyphArray);
	NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					reason:@"boxes in updateGlyphInfo"
					userInfo:nil];
	@throw exc;
    }
    CGFloat beyond_right = 0.;
    CGFloat beyond_left = 0.;
    CTFontGetBoundingRectsForGlyphs(
	(CTFontRef)screenFont,
	kCTFontHorizontalOrientation,
	glyphArray,
	boxes,
	GLYPH_COUNT);
    for (i = 0; i < GLYPH_COUNT; i++) {
	/* Account for the compression and offset used by drawWChar(). */
	CGFloat compression, offset;
	CGFloat v;

	if (glyphWidths[i] <= self.tileSize.width) {
	    compression = 1.;
	    offset = 0.5 * (self.tileSize.width - glyphWidths[i]);
	} else {
	    compression = self.tileSize.width / glyphWidths[i];
	    offset = 0.;
	}
	v = (offset + boxes[i].origin.x) * compression;
	if (beyond_left > v) {
	    beyond_left = v;
	}
	v = (offset + boxes[i].origin.x + boxes[i].size.width) * compression;
	if (beyond_right < v) {
	    beyond_right = v;
	}
    }
    free(boxes);
    self->_nColPre = ceil(-beyond_left / self.tileSize.width);
    if (beyond_right > self.tileSize.width) {
	self->_nColPost =
	    ceil((beyond_right - self.tileSize.width) / self.tileSize.width);
    } else {
	self->_nColPost = 0;
    }

    free(glyphWidths);
    free(glyphArray);
}


- (void)requestRedraw
{
    if (! self->terminal) return;
    
    term *old = Term;
    
    /* Activate the term */
    Term_activate(self->terminal);
    
    /* Redraw the contents */
    Term_redraw();
    
    /* Flush the output */
    Term_fresh();
    
    /* Restore the old term */
    Term_activate(old);
}

- (void)setTerm:(term *)t
{
    self->terminal = t;
}

/**
 * If we're trying to limit ourselves to a certain number of frames per second,
 * then compute how long it's been since we last drew, and then wait until the
 * next frame has passed. */
- (void)throttle
{
    if (frames_per_second > 0)
    {
        CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
        CFTimeInterval timeSinceLastRefresh = now - self->lastRefreshTime;
        CFTimeInterval timeUntilNextRefresh = (1. / (double)frames_per_second) - timeSinceLastRefresh;
        
        if (timeUntilNextRefresh > 0)
        {
            usleep((unsigned long)(timeUntilNextRefresh * 1000000.));
        }
    }
    self->lastRefreshTime = CFAbsoluteTimeGetCurrent();
}

- (void)drawWChar:(wchar_t)wchar inRect:(NSRect)tile screenFont:(NSFont*)font
	  context:(CGContextRef)ctx
{
    CGFloat tileOffsetY = self.fontAscender;
    CGFloat tileOffsetX = 0.0;
    UniChar unicharString[2];
    int nuni;

    if (CFStringGetSurrogatePairForLongCharacter(wchar, unicharString)) {
	nuni = 2;
    } else {
	unicharString[0] = (UniChar) wchar;
	nuni = 1;
    }

    /* Get glyph and advance */
    CGGlyph thisGlyphArray[2] = { 0, 0 };
    CGSize advances[2] = { { 0, 0 }, { 0, 0 } };
    CTFontGetGlyphsForCharacters(
	(CTFontRef)font, unicharString, thisGlyphArray, nuni);
    CGGlyph glyph = thisGlyphArray[0];
    CTFontGetAdvancesForGlyphs(
	(CTFontRef)font, kCTFontHorizontalOrientation, thisGlyphArray,
	advances, 1);
    CGSize advance = advances[0];

    /*
     * If our font is not monospaced, our tile width is deliberately not big
     * enough for every character. In that event, if our glyph is too wide, we
     * need to compress it horizontally. Compute the compression ratio.
     * 1.0 means no compression.
     */
    double compressionRatio;
    if (advance.width <= NSWidth(tile))
    {
        /* Our glyph fits, so we can just draw it, possibly with an offset */
        compressionRatio = 1.0;
        tileOffsetX = (NSWidth(tile) - advance.width)/2;
    }
    else
    {
        /* Our glyph doesn't fit, so we'll have to compress it */
        compressionRatio = NSWidth(tile) / advance.width;
        tileOffsetX = 0;
    }

    /* Now draw it */
    CGAffineTransform textMatrix = CGContextGetTextMatrix(ctx);
    CGFloat savedA = textMatrix.a;

    /* Set the position */
    textMatrix.tx = tile.origin.x + tileOffsetX;
    textMatrix.ty = tile.origin.y + tileOffsetY;

    /* Maybe squish it horizontally. */
    if (compressionRatio != 1.)
    {
        textMatrix.a *= compressionRatio;
    }

    CGContextSetTextMatrix(ctx, textMatrix);
    CGContextShowGlyphsAtPositions(ctx, &glyph, &CGPointZero, 1);

    /* Restore the text matrix if we messed with the compression ratio */
    if (compressionRatio != 1.)
    {
        textMatrix.a = savedA;
    }

    CGContextSetTextMatrix(ctx, textMatrix);
}

- (NSRect)viewRectForCellBlockAtX:(int)x y:(int)y width:(int)w height:(int)h
{
    return NSMakeRect(
	x * self.tileSize.width + self.borderSize.width,
	y * self.tileSize.height + self.borderSize.height,
	w * self.tileSize.width, h * self.tileSize.height);
}

- (void)setSelectionFont:(NSFont*)font adjustTerminal: (BOOL)adjustTerminal
{
    /* Record the new font */
    self.angbandViewFont = font;

    /* Update our glyph info */
    [self updateGlyphInfo];

    if( adjustTerminal )
    {
        /*
         * Adjust terminal to fit window with new font; save the new columns
         * and rows since they could be changed
         */
        NSRect contentRect =
	    [self.primaryWindow
		 contentRectForFrameRect: [self.primaryWindow frame]];

	[self constrainWindowSize:[self terminalIndex]];
	NSSize size = self.primaryWindow.contentMinSize;
	BOOL windowNeedsResizing = NO;
	if (contentRect.size.width < size.width) {
	    contentRect.size.width = size.width;
	    windowNeedsResizing = YES;
	}
	if (contentRect.size.height < size.height) {
	    contentRect.size.height = size.height;
	    windowNeedsResizing = YES;
	}
	if (windowNeedsResizing) {
	    size.width = contentRect.size.width;
	    size.height = contentRect.size.height;
	    [self.primaryWindow setContentSize:size];
	}
        [self resizeTerminalWithContentRect: contentRect saveToDefaults: YES];
    }
}

- (id)init
{
    if ((self = [super init]))
    {
        /* Default rows and cols */
        self->_cols = 80;
        self->_rows = 24;

        /* Default border size */
        self->_borderSize = NSMakeSize(2, 2);

	self->_nColPre = 0;
	self->_nColPost = 0;

	self->_contents =
	    [[TerminalContents alloc] initWithColumns:self->_cols
				      rows:self->_rows];
	self->_changes =
	    [[TerminalChanges alloc] initWithColumns:self->_cols
				     rows:self->_rows];
	self->lastRefreshTime = CFAbsoluteTimeGetCurrent();
	self->inFullscreenTransition = NO;

        self->_windowVisibilityChecked = NO;
    }
    return self;
}

/**
 * Destroy all the receiver's stuff. This is intended to be callable more than
 * once.
 */
- (void)dispose
{
    self->terminal = NULL;

    /* Disassociate ourselves from our view. */
    [self->angbandView setAngbandContext:nil];
    self->angbandView = nil;

    /* Font */
    self.angbandViewFont = nil;

    /* Window */
    [self.primaryWindow setDelegate:nil];
    [self.primaryWindow close];
    self.primaryWindow = nil;

    /* Contents and pending changes */
    self.contents = nil;
    self.changes = nil;
}

/* Usual Cocoa fare */
- (void)dealloc
{
    [self dispose];
}

- (void)resizeWithColumns:(int)nCol rows:(int)nRow
{
    [self.contents resizeWithColumns:nCol rows:nRow];
    [self.changes resizeWithColumns:nCol rows:nRow];
    self->_cols = nCol;
    self->_rows = nRow;
}

/**
 * For defaultFont and setDefaultFont.
 */
static __strong NSFont* gDefaultFont = nil;

+ (NSFont*)defaultFont
{
    return gDefaultFont;
}

+ (void)setDefaultFont:(NSFont*)font
{
    gDefaultFont = font;
}

- (NSWindow *)makePrimaryWindow
{
    if (! self.primaryWindow)
    {
        /*
         * This has to be done after the font is set, which it already is in
         * term_init_cocoa()
         */
        NSSize sz = self.baseSize;
        NSRect contentRect = NSMakeRect( 0.0, 0.0, sz.width, sz.height );

        NSUInteger styleMask = NSTitledWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask;

        /* Make every window other than the main window closable */
        if ((__bridge AngbandContext*) (angband_term[0]->data) != self)
        {
            styleMask |= NSClosableWindowMask;
        }

        self.primaryWindow = [[NSWindow alloc] initWithContentRect:contentRect styleMask: styleMask backing:NSBackingStoreBuffered defer:YES];

        /* Not to be released when closed */
        [self.primaryWindow setReleasedWhenClosed:NO];
        [self.primaryWindow setExcludedFromWindowsMenu: YES]; /* we're using custom window menu handling */

        /* Make the view */
        self->angbandView = [[AngbandView alloc] initWithFrame:contentRect];
        [angbandView setAngbandContext:self];
        [angbandView setNeedsDisplay:YES];
        [self.primaryWindow setContentView:angbandView];

        /* We are its delegate */
        [self.primaryWindow setDelegate:self];
    }
    return self.primaryWindow;
}


- (void)computeInvalidRects
{
    for (int irow = self.changes.firstChangedRow;
	 irow <= self.changes.lastChangedRow;
	 ++irow) {
	int icol = [self.changes scanForChangedInRow:irow
			col0:0 col1:self.cols];

	while (icol < self.cols) {
	    /* Find the end of the changed region. */
	    int jcol =
		[self.changes scanForUnchangedInRow:irow col0:(icol + 1)
		     col1:self.cols];

	    /*
	     * If the last column is a character, extend the region drawn
	     * because characters can exceed the horizontal bounds of the cell
	     * and those parts will need to be cleared.  Don't extend into a
	     * tile because the clipping is set while drawing to never
	     * extend text into a tile.  For a big character that's been
	     * partially overwritten, allow what comes after the point
	     * where the overwrite occurred to influence the stuff before
	     * but not vice versa.  If extending the region reaches another
	     * changed block, find the end of that block and repeat the
	     * process.
	     */
	    /*
	     * A value of zero means checking for a character immediately
	     * prior to the column, isrch.  A value of one means checking for
	     * something past the end that could either influence the changed
	     * region (within nColPre of it and no intervening tile) or be
	     * influenced by it (within nColPost of it and no intervening
	     * tile or partially overwritten big character).  A value of two
	     * means checking for something past the end which is both changed
	     * and could affect the part of the unchanged region that has to
	     * be redrawn because it is affected by the prior changed region
	     * Values of three and four are like one and two, respectively,
	     * but indicate that a partially overwritten big character was
	     * found.
	     */
	    int stage = 0;
	    int isrch = jcol;
	    int irng0 = jcol;
	    int irng1 = jcol;
	    while (1) {
		if (stage == 0) {
		    const struct TerminalCell *pcell =
			[self.contents getCellAtColumn:(isrch - 1) row:irow];
		    if ((pcell->form &
			 (TERM_CELL_TILE | TERM_CELL_TILE_PADDING)) != 0) {
			break;
		    } else {
			irng0 = isrch + self.nColPre;
			if (irng0 > self.cols) {
			    irng0 = self.cols;
			}
			irng1 = isrch + self.nColPost;
			if (irng1 > self.cols) {
			    irng1 = self.cols;
			}
			if (isrch < irng0 || isrch < irng1) {
			    stage = isPartiallyOverwrittenBigChar(pcell) ?
				3 : 1;
			} else {
			    break;
			}
		    }
		}

		if (stage == 1) {
		    const struct TerminalCell *pcell =
			[self.contents getCellAtColumn:isrch row:irow];

		    if ((pcell->form &
			 (TERM_CELL_TILE | TERM_CELL_TILE_PADDING)) != 0) {
			/*
			 * Check if still in the region that could be
			 * influenced by the changed region.  If so,
			 * everything up to the tile will be redrawn anyways
			 * so combine the regions if the tile has changed
			 * as well.  Otherwise, terminate the search since
			 * the tile doesn't allow influence to propagate
			 * through it and don't want to affect what's in the
			 * tile.
			 */
			if (isrch < irng1) {
			    if ([self.changes isChangedAtColumn:isrch
				     row:irow]) {
				jcol = [self.changes scanForUnchangedInRow:irow
					    col0:(isrch + 1) col1:self.cols];
				if (jcol < self.cols) {
				    stage = 0;
				    isrch = jcol;
				    continue;
				}
			    }
			}
			break;
		    } else {
			/*
			 * With a changed character, combine the regions (if
			 * still in the region affected by the changed region
			 * am going to redraw everything up to this new region
			 * anyway; if only in the region that can affect the
			 * changed region, this changed text could influence
			 * the current changed region).
			 */
			if ([self.changes isChangedAtColumn:isrch row:irow]) {
			    jcol = [self.changes scanForUnchangedInRow:irow
					col0:(isrch + 1) col1:self.cols];
			    if (jcol < self.cols) {
				stage = 0;
				isrch = jcol;
				continue;
			    }
			    break;
			}

			if (isrch < irng1) {
			    /*
			     * Can be affected by the changed region so
			     * has to be redrawn.
			     */
			    ++jcol;
			}
			++isrch;
			if (isrch >= irng1) {
			    irng0 = jcol + self.nColPre;
			    if (irng0 > self.cols) {
				irng0 = self.cols;
			    }
			    if (isrch >= irng0) {
				break;
			    }
			    stage = isPartiallyOverwrittenBigChar(pcell) ?
				4 : 2;
			} else if (isPartiallyOverwrittenBigChar(pcell)) {
			    stage = 3;
			}
		    }
		}

		if (stage == 2) {
		    /*
		     * Looking for a later changed region that could influence
		     * the region that has to be redrawn.  The region that has
		     * to be redrawn ends just before jcol.
		     */
		    const struct TerminalCell *pcell =
			[self.contents getCellAtColumn:isrch row:irow];

		    if ((pcell->form &
			 (TERM_CELL_TILE | TERM_CELL_TILE_PADDING)) != 0) {
			/* Can not spread influence through a tile. */
			break;
		    }
		    if ([self.changes isChangedAtColumn:isrch row:irow]) {
			/*
			 * Found one.  Combine with the one ending just before
			 * jcol.
			 */
			jcol = [self.changes scanForUnchangedInRow:irow
				    col0:(isrch + 1) col1:self.cols];
			if (jcol < self.cols) {
			    stage = 0;
			    isrch = jcol;
			    continue;
			}
			break;
		    }

		    ++isrch;
		    if (isrch >= irng0) {
			break;
		    }
		    if (isPartiallyOverwrittenBigChar(pcell)) {
			stage = 4;
		    }
		}

		if (stage == 3) {
		    const struct TerminalCell *pcell =
			[self.contents getCellAtColumn:isrch row:irow];

		    /*
		     * Have encountered a partially overwritten big character
		     * but still may be in the region that could be influenced
		     * by the changed region.  That influence can not extend
		     * past the past the padding for the partially overwritten
		     * character.
		     */
		    if ((pcell->form & (TERM_CELL_CHAR | TERM_CELL_TILE |
					TERM_CELL_TILE_PADDING)) != 0) {
			if (isrch < irng1) {
			    /*
			     * Still can be affected by the changed region
			     * so everything up to isrch will be redrawn
			     * anyways.  If this location has changed,
			     * merge the changed regions.
			     */
			    if ([self.changes isChangedAtColumn:isrch
				     row:irow]) {
				jcol = [self.changes scanForUnchangedInRow:irow
					    col0:(isrch + 1) col1:self.cols];
				if (jcol < self.cols) {
				    stage = 0;
				    isrch = jcol;
				    continue;
				}
				break;
			    }
			}
			if ((pcell->form &
			     (TERM_CELL_TILE | TERM_CELL_TILE_PADDING)) != 0) {
			    /*
			     * It's a tile.  That blocks influence in either
			     * direction.
			     */
			    break;
			}

			/*
			 * The partially overwritten big character was
			 * overwritten by a character.  Check to see if it
			 * can either influence the unchanged region that
			 * has to redrawn or the changed region prior to
			 * that.
			 */
			if (isrch >= irng0) {
			    break;
			}
			stage = 4;
		    } else {
			if (isrch < irng1) {
			    /*
			     * Can be affected by the changed region so has to
			     * be redrawn.
			     */
			    ++jcol;
			}
			++isrch;
			if (isrch >= irng1) {
			    irng0 = jcol + self.nColPre;
			    if (irng0 > self.cols) {
				irng0 = self.cols;
			    }
			    if (isrch >= irng0) {
				break;
			    }
			    stage = 4;
			}
		    }
		}

		if (stage == 4) {
		    /*
		     * Have already encountered a partially overwritten big
		     * character.  Looking for a later changed region that
		     * could influence the region that has to be redrawn
		     * The region that has to be redrawn ends just before jcol.
		     */
		    const struct TerminalCell *pcell =
			[self.contents getCellAtColumn:isrch row:irow];

		    if ((pcell->form &
			 (TERM_CELL_TILE | TERM_CELL_TILE_PADDING)) != 0) {
			/* Can not spread influence through a tile. */
			break;
		    }
		    if (pcell->form == TERM_CELL_CHAR) {
			if ([self.changes isChangedAtColumn:isrch row:irow]) {
			    /*
			     * Found a changed region.  Combine with the one
			     * ending just before jcol.
			     */
			    jcol = [self.changes scanForUnchangedInRow:irow
					col0:(isrch + 1) col1:self.cols];
			    if (jcol < self.cols) {
				stage = 0;
				isrch = jcol;
				continue;
			    }
			    break;
			}
		    }
		    ++isrch;
		    if (isrch >= irng0) {
			break;
		    }
		}
	    }

	    /*
	     * Check to see if there's characters before the changed region
	     * that would have to be redrawn because it's influenced by the
	     * changed region.  Do not have to check for merging with a prior
	     * region because of the screening already done.
	     */
	    if (self.nColPre > 0 &&
		([self.contents getCellAtColumn:icol row:irow]->form &
		 (TERM_CELL_CHAR | TERM_CELL_CHAR_PADDING)) != 0) {
		int irng = icol - self.nColPre;

		if (irng < 0) {
		    irng = 0;
		}
		while (icol > irng &&
		       ([self.contents getCellAtColumn:(icol - 1)
			     row:irow]->form &
			(TERM_CELL_CHAR | TERM_CELL_CHAR_PADDING)) != 0) {
		    --icol;
		}
	    }

	    NSRect r = [self viewRectForCellBlockAtX:icol y:irow
			     width:(jcol - icol) height:1];
	    [self setNeedsDisplayInRect:r];

	    icol = [self.changes scanForChangedInRow:irow col0:jcol
			col1:self.cols];
	}
    }
}


#pragma mark View/Window Passthrough

/*
 * This is a qsort-compatible compare function for NSRect, to get them in
 * ascending order by y origin.
 */
static int compare_nsrect_yorigin_greater(const void *ap, const void *bp)
{
    const NSRect *arp = ap;
    const NSRect *brp = bp;
    return (arp->origin.y > brp->origin.y) - (arp->origin.y < brp->origin.y);
}

/**
 * This is a helper function for drawRect.
 */
- (void)renderTileRunInRow:(int)irow col0:(int)icol0 col1:(int)icol1
		     nsctx:(NSGraphicsContext*)nsctx ctx:(CGContextRef)ctx
		 grafWidth:(int)graf_width grafHeight:(int)graf_height
	       overdrawRow:(int)overdraw_row overdrawMax:(int)overdraw_max
{
    /* Save the compositing mode since it is modified below. */
    NSCompositingOperation op = nsctx.compositingOperation;

    while (icol0 < icol1) {
	const struct TerminalCell *pcell =
	    [self.contents getCellAtColumn:icol0 row:irow];
	NSRect destinationRect =
	    [self viewRectForCellBlockAtX:icol0 y:irow
		  width:pcell->hscl height:pcell->vscl];
	NSRect fgdRect = NSMakeRect(
	    graf_width * (pcell->v.ti.fgdCol +
			  pcell->hoff_n / (1.0 * pcell->hoff_d)),
	    graf_height * (pcell->v.ti.fgdRow +
			   pcell->voff_n / (1.0 * pcell->voff_d)),
	    graf_width * pcell->hscl / (1.0 * pcell->hoff_d),
	    graf_height * pcell->vscl / (1.0 * pcell->voff_d));
	NSRect bckRect = NSMakeRect(
	    graf_width * (pcell->v.ti.bckCol +
			  pcell->hoff_n / (1.0 * pcell->hoff_d)),
	    graf_height * (pcell->v.ti.bckRow +
			   pcell->voff_n / (1.0 * pcell->voff_d)),
	    graf_width * pcell->hscl / (1.0 * pcell->hoff_d),
	    graf_height * pcell->vscl / (1.0 * pcell->voff_d));
	int dbl_height_bck = overdraw_row && (irow > 2) &&
	    (pcell->v.ti.bckRow >= overdraw_row &&
	     pcell->v.ti.bckRow <= overdraw_max);
	int dbl_height_fgd = overdraw_row && (irow > 2) &&
	    (pcell->v.ti.fgdRow >= overdraw_row) &&
	    (pcell->v.ti.fgdRow <= overdraw_max);
	int aligned_row = 0, aligned_col = 0;
	int is_first_piece = 0, simple_upper = 0;

	/* Initialize stuff for handling a double-height tile. */
	if (dbl_height_bck || dbl_height_fgd) {
	    if (self->terminal == angband_term[0]) {
		aligned_col = ((icol0 - COL_MAP) / pcell->hoff_d) *
		    pcell->hoff_d + COL_MAP;
	    } else {
		aligned_col = (icol0 / pcell->hoff_d) * pcell->hoff_d;
	    }
	    aligned_row = ((irow - ROW_MAP) / pcell->voff_d) *
		pcell->voff_d + ROW_MAP;

	    /*
	     * If the lower half has been broken into multiple pieces, only
	     * do the work of rendering whatever is necessary for the upper
	     * half when drawing the first piece (the one closest to the
	     * upper left corner).
	     */
	    struct TerminalCellLocation curs = { 0, 0 };

	    [self.contents scanForTypeMaskInBlockAtColumn:aligned_col
		 row:aligned_row width:pcell->hoff_d height:pcell->voff_d
		 mask:TERM_CELL_TILE cursor:&curs];
	    if (curs.col + aligned_col == icol0 &&
		curs.row + aligned_row == irow) {
		is_first_piece = 1;

		/*
		 * Hack:  lookup the previous row to determine how much of the
		 * tile there is shown to apply it the upper half of the
		 * double-height tile.  That will do the right thing if there
		 * is a menu displayed in that row but isn't right if there's
		 * an object/creature/feature there that doesn't have a
		 * mapping to the tile set and is rendered with a character.
		 */
		curs.col = 0;
		curs.row = 0;
		[self.contents scanForTypeMaskInBlockAtColumn:aligned_col
		     row:(aligned_row - pcell->voff_d) width:pcell->hoff_d
		     height:pcell->voff_d mask:TERM_CELL_TILE cursor:&curs];
		if (curs.col == 0 && curs.row == 0) {
		    const struct TerminalCell *pcell2 =
			[self.contents
			     getCellAtColumn:(aligned_col + curs.col)
			     row:(aligned_row + curs.row - pcell->voff_d)];

		    if (pcell2->hscl == pcell2->hoff_d &&
			pcell2->vscl == pcell2->voff_d) {
			/*
			 * The tile in the previous row hasn't been clipped
			 * or partially overwritten.  Use a streamlined
			 * rendering procedure.
			 */
			simple_upper = 1;
		    }
		}
	    }
	}

	/*
	 * Draw the background.  For a double-height tile, this is only the
	 * the lower half.
	 */
	draw_image_tile(
	    nsctx, ctx, pict_image, bckRect, destinationRect, NSCompositeCopy);
	if (dbl_height_bck && is_first_piece) {
	    /* Combine upper half with previously drawn row. */
	    if (simple_upper) {
		const struct TerminalCell *pcell2 =
		    [self.contents getCellAtColumn:aligned_col
			 row:(aligned_row - pcell->voff_d)];
		NSRect drect2 =
		    [self viewRectForCellBlockAtX:aligned_col
			  y:(aligned_row - pcell->voff_d)
			  width:pcell2->hscl height:pcell2->vscl];
		NSRect brect2 = NSMakeRect(
		    graf_width * pcell->v.ti.bckCol,
		    graf_height * (pcell->v.ti.bckRow - 1),
		    graf_width, graf_height);

		draw_image_tile(nsctx, ctx, pict_image, brect2, drect2,
				NSCompositeSourceOver);
	    } else {
		struct TerminalCellLocation curs = { 0, 0 };

		[self.contents scanForTypeMaskInBlockAtColumn:aligned_col
		     row:(aligned_row - pcell->voff_d) width:pcell->hoff_d
		     height:pcell->voff_d mask:TERM_CELL_TILE
		     cursor:&curs];
		while (curs.col < pcell->hoff_d &&
		       curs.row < pcell->voff_d) {
		    const struct TerminalCell *pcell2 =
			[self.contents getCellAtColumn:(aligned_col + curs.col)
			     row:(aligned_row + curs.row - pcell->voff_d)];
		    NSRect drect2 =
			[self viewRectForCellBlockAtX:(aligned_col + curs.col)
			      y:(aligned_row + curs.row - pcell->voff_d)
			      width:pcell2->hscl height:pcell2->vscl];
		    /*
		     * Column and row in the tile set are from the
		     * double-height tile at *pcell, but the offsets within
		     * that and size are from what's visible for *pcell2.
		     */
		    NSRect brect2 = NSMakeRect(
			graf_width * (pcell->v.ti.bckCol +
				      pcell2->hoff_n / (1.0 * pcell2->hoff_d)),
			graf_height * (pcell->v.ti.bckRow - 1 +
				       pcell2->voff_n /
				       (1.0 * pcell2->voff_d)),
			graf_width * pcell2->hscl / (1.0 * pcell2->hoff_d),
			graf_height * pcell2->vscl / (1.0 * pcell2->voff_d));

		    draw_image_tile(nsctx, ctx, pict_image, brect2, drect2,
				    NSCompositeSourceOver);
		    curs.col += pcell2->hscl;
		    [self.contents
			 scanForTypeMaskInBlockAtColumn:aligned_col
			 row:(aligned_row - pcell->voff_d)
			 width:pcell->hoff_d height:pcell->voff_d
			 mask:TERM_CELL_TILE cursor:&curs];
		}
	    }
	}

	/* Skip drawing the foreground if it is the same as the background. */
	if (fgdRect.origin.x != bckRect.origin.x ||
	    fgdRect.origin.y != bckRect.origin.y) {
	    if (is_first_piece && dbl_height_fgd) {
		if (simple_upper) {
		    if (pcell->hoff_n == 0 && pcell->voff_n == 0 &&
			pcell->hscl == pcell->hoff_d) {
			/*
			 * Render upper and lower parts as one since they
			 * are contiguous.
			 */
			fgdRect.origin.y -= graf_height;
			fgdRect.size.height += graf_height;
			destinationRect.origin.y -=
			    destinationRect.size.height;
			destinationRect.size.height +=
			    destinationRect.size.height;
		    } else {
			/* Not contiguous.  Render the upper half. */
			NSRect drect2 =
			    [self viewRectForCellBlockAtX:aligned_col
				  y:(aligned_row - pcell->voff_d)
				  width:pcell->hoff_d height:pcell->voff_d];
			NSRect frect2 = NSMakeRect(
			    graf_width * pcell->v.ti.fgdCol,
			    graf_height * (pcell->v.ti.fgdRow - 1),
			    graf_width, graf_height);

			draw_image_tile(
			    nsctx, ctx, pict_image, frect2, drect2,
			    NSCompositeSourceOver);
		    }
		} else {
		    /* Render the upper half pieces. */
		    struct TerminalCellLocation curs = { 0, 0 };

		    while (1) {
			[self.contents
			     scanForTypeMaskInBlockAtColumn:aligned_col
			     row:(aligned_row - pcell->voff_d)
			     width:pcell->hoff_d height:pcell->voff_d
			     mask:TERM_CELL_TILE cursor:&curs];

			if (curs.col >= pcell->hoff_d ||
			    curs.row >= pcell->voff_d) {
			    break;
			}

			const struct TerminalCell *pcell2 =
			    [self.contents
				 getCellAtColumn:(aligned_col + curs.col)
				 row:(aligned_row + curs.row - pcell->voff_d)];
			NSRect drect2 =
			    [self viewRectForCellBlockAtX:(aligned_col + curs.col)
				  y:(aligned_row + curs.row - pcell->voff_d)
				  width:pcell2->hscl height:pcell2->vscl];
			NSRect frect2 = NSMakeRect(
			    graf_width * (pcell->v.ti.fgdCol +
					  pcell2->hoff_n /
					  (1.0 * pcell2->hoff_d)),
			    graf_height * (pcell->v.ti.fgdRow - 1 +
					   pcell2->voff_n /
					   (1.0 * pcell2->voff_d)),
			    graf_width * pcell2->hscl / (1.0 * pcell2->hoff_d),
			    graf_height * pcell2->vscl /
			        (1.0 * pcell2->voff_d));

			draw_image_tile(nsctx, ctx, pict_image, frect2, drect2,
					NSCompositeSourceOver);
			curs.col += pcell2->hscl;
		    }
		}
	    }
	    /*
	     * Render the foreground (if a double height tile and the bottom
	     * part is contiguous with the upper part this also render the
	     * upper part.
	     */
	    draw_image_tile(
		nsctx, ctx, pict_image, fgdRect, destinationRect,
		NSCompositeSourceOver);
	}
	icol0 = [self.contents scanForTypeMaskInRow:irow mask:TERM_CELL_TILE
		     col0:(icol0+pcell->hscl) col1:icol1];
    }

    /* Restore the compositing mode. */
    nsctx.compositingOperation = op;
}

/**
 * This is what our views call to get us to draw to the window
 */
- (void)drawRect:(NSRect)rect inView:(NSView *)view
{
    /*
     * Take this opportunity to throttle so we don't flush faster than desired.
     */
    [self throttle];

    CGFloat bottomY =
	self.borderSize.height + self.tileSize.height * self.rows;
    CGFloat rightX =
	self.borderSize.width + self.tileSize.width * self.cols;

    const NSRect *invalidRects;
    NSInteger invalidCount;
    [view getRectsBeingDrawn:&invalidRects count:&invalidCount];

    /*
     * If the non-border areas need rendering, set some things up so they can
     * be reused for each invalid rectangle.
     */
    NSGraphicsContext *nsctx = nil;
    CGContextRef ctx = 0;
    NSFont* screenFont = nil;
    int graf_width = 0, graf_height = 0;
    int overdraw_row = 0, overdraw_max = 0;
    wchar_t blank = 0;
    if (rect.origin.x < rightX &&
	rect.origin.x + rect.size.width > self.borderSize.width &&
	rect.origin.y < bottomY &&
	rect.origin.y + rect.size.height > self.borderSize.height) {
	nsctx = [NSGraphicsContext currentContext];
	ctx = [nsctx graphicsPort];
	screenFont = [self.angbandViewFont screenFont];
	[screenFont set];
	blank = [TerminalContents getBlankChar];
	if (use_graphics) {
	    graf_width = current_graphics_mode->cell_width;
	    graf_height = current_graphics_mode->cell_height;
	    overdraw_row = current_graphics_mode->overdrawRow;
	    overdraw_max = current_graphics_mode->overdrawMax;
	}
    }

    /*
     * With double height tiles, need to have rendered prior rows (i.e.
     * smaller y) before the current one.  Since the invalid rectanges are
     * processed in order, ensure that by sorting the invalid rectangles in
     * increasing order of y origin (AppKit guarantees the invalid rectanges
     * are non-overlapping).
     */
    NSRect* sortedRects = 0;
    const NSRect* workingRects;
    if (overdraw_row && invalidCount > 1) {
	sortedRects = malloc(invalidCount * sizeof(NSRect));
	if (sortedRects == 0) {
	    NSException *exc = [NSException exceptionWithName:@"OutOfMemory"
					    reason:@"sorted rects in drawRect"
					    userInfo:nil];
	    @throw exc;
	}
	(void) memcpy(
	    sortedRects, invalidRects, invalidCount * sizeof(NSRect));
	qsort(sortedRects, invalidCount, sizeof(NSRect),
	      compare_nsrect_yorigin_greater);
	workingRects = sortedRects;
    } else {
	workingRects = invalidRects;
    }

    /*
     * Use -2 for unknown.  Use -1 for Cocoa's blackColor.  All others are the
     * Angband color index.
     */
    int alast = -2;
    int redrawCursor = 0;

    for (NSInteger irect = 0; irect < invalidCount; ++irect) {
	NSRect modRect, clearRect;
	CGFloat edge;
	int iRowFirst, iRowLast;
	int iColFirst, iColLast;

	/* Handle the top border. */
	if (workingRects[irect].origin.y < self.borderSize.height) {
	    edge =
		workingRects[irect].origin.y + workingRects[irect].size.height;
	    if (edge <= self.borderSize.height) {
		if (alast != -1) {
		    [[NSColor blackColor] set];
		    alast = -1;
		}
		NSRectFill(workingRects[irect]);
		continue;
	    }
	    clearRect = workingRects[irect];
	    clearRect.size.height =
		self.borderSize.height - workingRects[irect].origin.y;
	    if (alast != -1) {
		[[NSColor blackColor] set];
		alast = -1;
	    }
	    NSRectFill(clearRect);
	    modRect.origin.x = workingRects[irect].origin.x;
	    modRect.origin.y = self.borderSize.height;
	    modRect.size.width = workingRects[irect].size.width;
	    modRect.size.height = edge - self.borderSize.height;
	} else {
	    modRect = workingRects[irect];
	}

	/* Handle the left border. */
	if (modRect.origin.x < self.borderSize.width) {
	    edge = modRect.origin.x + modRect.size.width;
	    if (edge <= self.borderSize.width) {
		if (alast != -1) {
		    alast = -1;
		    [[NSColor blackColor] set];
		}
		NSRectFill(modRect);
		continue;
	    }
	    clearRect = modRect;
	    clearRect.size.width = self.borderSize.width - clearRect.origin.x;
	    if (alast != -1) {
		alast = -1;
		[[NSColor blackColor] set];
	    }
	    NSRectFill(clearRect);
	    modRect.origin.x = self.borderSize.width;
	    modRect.size.width = edge - self.borderSize.width;
	}

	iRowFirst = floor((modRect.origin.y - self.borderSize.height) /
			  self.tileSize.height);
	iColFirst = floor((modRect.origin.x - self.borderSize.width) /
			  self.tileSize.width);
	edge = modRect.origin.y + modRect.size.height;
	if (edge <= bottomY) {
	    iRowLast =
		ceil((edge - self.borderSize.height) / self.tileSize.height);
	} else {
	    iRowLast = self.rows;
	}
	edge = modRect.origin.x + modRect.size.width;
	if (edge <= rightX) {
	    iColLast =
		ceil((edge - self.borderSize.width) / self.tileSize.width);
	} else {
	    iColLast = self.cols;
	}

	if (self.contents.cursorColumn != -1 &&
	    self.contents.cursorRow != -1 &&
	    self.contents.cursorColumn + self.contents.cursorWidth - 1 >=
	    iColFirst &&
	    self.contents.cursorColumn < iColLast &&
	    self.contents.cursorRow + self.contents.cursorHeight - 1 >=
	    iRowFirst &&
	    self.contents.cursorRow < iRowLast) {
	    redrawCursor = 1;
	}

	for (int irow = iRowFirst; irow < iRowLast; ++irow) {
	    int icol =
		[self.contents scanForTypeMaskInRow:irow
		     mask:(TERM_CELL_CHAR | TERM_CELL_TILE)
		     col0:iColFirst col1:iColLast];

	    while (1) {
		if (icol >= iColLast) {
		    break;
		}

		if ([self.contents getCellAtColumn:icol row:irow]->form ==
		    TERM_CELL_TILE) {
		    /*
		     * It is a tile.  Identify how far the run of tiles goes.
		     */
		    int jcol = [self.contents scanForPredicateInRow:irow
				    predicate:isTileTop desired:1
				    col0:(icol + 1) col1:iColLast];

		    [self renderTileRunInRow:irow col0:icol col1:jcol
			  nsctx:nsctx ctx:ctx
			  grafWidth:graf_width grafHeight:graf_height
			  overdrawRow:overdraw_row overdrawMax:overdraw_max];
		    icol = jcol;
		} else {
		    /*
		     * It is a character.  Identify how far the run of
		     * characters goes.
		     */
		    int jcol = [self.contents scanForPredicateInRow:irow
				    predicate:isCharNoPartial desired:1
				    col0:(icol + 1) col1:iColLast];
		    int jcol2;

		    if (jcol < iColLast &&
			isPartiallyOverwrittenBigChar(
			    [self.contents getCellAtColumn:jcol row:irow])) {
			jcol2 = [self.contents scanForTypeMaskInRow:irow
				     mask:~TERM_CELL_CHAR_PADDING
				     col0:(jcol + 1) col1:iColLast];
		    } else {
			jcol2 = jcol;
		    }

		    /*
		     * Set up clipping rectangle for text.  Save the
		     * graphics context so the clipping rectangle can be
		     * forgotten.  Use CGContextBeginPath to clear the current
		     * path so it does not affect clipping.  Do not call
		     * CGContextSetTextDrawingMode() to include clipping since
		     * that does not appear to necessary on 10.14 and is
		     * actually detrimental:  when displaying more than one
		     * character, only the first is visible.
		     */
		    CGContextSaveGState(ctx);
		    CGContextBeginPath(ctx);
		    NSRect r = [self viewRectForCellBlockAtX:icol y:irow
				     width:(jcol2 - icol) height:1];
		    CGContextClipToRect(ctx, r);

		    /*
		     * See if the region to be rendered needs to be expanded:
		     * adjacent text that could influence what's in the clipped
		     * region.
		     */
		    int isrch = icol;
		    int irng = icol - self.nColPost;
		    if (irng < 1) {
			irng = 1;
		    }

		    while (1) {
			if (isrch <= irng) {
			    break;
			}

			const struct TerminalCell *pcell2 =
			    [self.contents getCellAtColumn:(isrch - 1)
				 row:irow];
			if (pcell2->form == TERM_CELL_CHAR) {
			    --isrch;
			    if (pcell2->v.ch.glyph != blank) {
				icol = isrch;
			    }
			} else if (pcell2->form == TERM_CELL_CHAR_PADDING) {
			    /*
			     * Only extend the rendering if this is padding
			     * for a character that hasn't been partially
			     * overwritten.
			     */
			    if (! isPartiallyOverwrittenBigChar(pcell2)) {
				if (isrch - pcell2->v.pd.hoff >= 0) {
				    const struct TerminalCell* pcell3 =
					[self.contents
					     getCellAtColumn:(isrch - pcell2->v.pd.hoff)
					     row:irow];

				    if (pcell3->v.ch.glyph != blank) {
					icol = isrch - pcell2->v.pd.hoff;
					isrch = icol - 1;
				    } else {
					isrch = isrch - pcell2->v.pd.hoff - 1;
				    }
				} else {
				    /* Should not happen, corrupt offset. */
				    --isrch;
				}
			    } else {
				break;
			    }
			} else {
			    /*
			     * Tiles or tile padding block anything before
			     * them from rendering after them.
			     */
			    break;
			}
		    }

		    isrch = jcol2;
		    irng = jcol2 + self.nColPre;
		    if (irng > self.cols) {
			irng = self.cols;
		    }
		    while (1) {
			if (isrch >= irng) {
			    break;
			}

			const struct TerminalCell *pcell2 =
			    [self.contents getCellAtColumn:isrch row:irow];
			if (pcell2->form == TERM_CELL_CHAR) {
			    if (pcell2->v.ch.glyph != blank) {
				jcol2 = isrch;
			    }
			    ++isrch;
			} else if (pcell2->form == TERM_CELL_CHAR_PADDING) {
			    ++isrch;
			} else {
			    break;
			}
		    }

		    /* Render text. */
		    while (icol < jcol) {
			/*
			 * Identify runs with the same background.  Then clear.
			 */
			int kcol = icol;
			while (1) {
			    if (kcol >= jcol) {
				break;
			    }
			    int a = get_background_color_index(
				[self.contents getCellAtColumn:kcol
				     row:irow]->v.ch.attr);
			    int mcol = [self.contents
					    scanForPredicateInRow:irow
					    predicate:hasSameBackground
					    desired:a
					    col0:(kcol + 1)
					    col1:jcol];

			    if (a != alast) {
				if (a == -1) {
				    [[NSColor blackColor] set];
				} else {
				    set_color_for_index(a);
				}
				alast = a;
			    }
			    r = [self viewRectForCellBlockAtX:kcol y:irow
				      width:(mcol - kcol) height:1];
			    NSRectFill(r);

			    while (icol < mcol) {
				const struct TerminalCell *pcell =
				    [self.contents getCellAtColumn:icol
					 row:irow];

				/*
				 * For blanks, clearing was all that was
				 * necessary.  Don't redraw them.
				 */
				if (pcell->v.ch.glyph != blank) {
				    int a = pcell->v.ch.attr % MAX_COLORS;

				    if (alast != a) {
					alast = a;
					set_color_for_index(a);
				    }
				    r = [self viewRectForCellBlockAtX:icol
					      y:irow width:pcell->hscl
					      height:1];
				    [self drawWChar:pcell->v.ch.glyph inRect:r
					  screenFont:screenFont context:ctx];
				}
				icol += pcell->hscl;
			    }

			    kcol = mcol;
			}
		    }

		    /*
		     * Forget the clipping rectangle.  As a side effect, lose
		     * the color.
		     */
		    CGContextRestoreGState(ctx);
		    alast = -2;
		}
		icol =
		    [self.contents scanForTypeMaskInRow:irow
			 mask:(TERM_CELL_CHAR | TERM_CELL_TILE)
			 col0:icol col1:iColLast];
	    }
	}

	/* Handle the right border. */
	edge = modRect.origin.x + modRect.size.width;
	if (edge > rightX) {
	    if (modRect.origin.x >= rightX) {
		if (alast != -1) {
		    alast = -1;
		    [[NSColor blackColor] set];
		}
		NSRectFill(modRect);
		continue;
	    }
	    clearRect = modRect;
	    clearRect.origin.x = rightX;
	    clearRect.size.width = edge - rightX;
	    if (alast != -1) {
		alast = -1;
		[[NSColor blackColor] set];
	    }
	    NSRectFill(clearRect);
	    modRect.size.width = edge - modRect.origin.x;
	}

	/* Handle the bottom border. */
	edge = modRect.origin.y + modRect.size.height;
	if (edge > bottomY) {
	    if (modRect.origin.y < bottomY) {
		modRect.origin.y = bottomY;
		modRect.size.height = edge - bottomY;
	    }
	    if (alast != -1) {
		alast = -1;
		[[NSColor blackColor] set];
	    }
	    NSRectFill(modRect);
	}
    }

    if (redrawCursor) {
	NSRect r = [self viewRectForCellBlockAtX:self.contents.cursorColumn
			 y:self.contents.cursorRow
			 width:self.contents.cursorWidth
			 height:self.contents.cursorHeight];
	[[NSColor yellowColor] set];
	NSFrameRectWithWidth(r, 1);
    }

    free(sortedRects);
}

- (BOOL)isOrderedIn
{
    return [[self->angbandView window] isVisible];
}

- (BOOL)isMainWindow
{
    return [[self->angbandView window] isMainWindow];
}

- (void)setNeedsDisplay:(BOOL)val
{
    [self->angbandView setNeedsDisplay:val];
}

- (void)setNeedsDisplayInRect:(NSRect)rect
{
    [self->angbandView setNeedsDisplayInRect:rect];
}

- (void)displayIfNeeded
{
    [self->angbandView displayIfNeeded];
}

- (int)terminalIndex
{
	int termIndex = 0;

	for( termIndex = 0; termIndex < ANGBAND_TERM_MAX; termIndex++ )
	{
		if( angband_term[termIndex] == self->terminal )
		{
			break;
		}
	}

	return termIndex;
}

- (void)resizeTerminalWithContentRect: (NSRect)contentRect saveToDefaults: (BOOL)saveToDefaults
{
    CGFloat newRows = floor(
	(contentRect.size.height - (self.borderSize.height * 2.0)) /
	self.tileSize.height);
    CGFloat newColumns = floor(
	(contentRect.size.width - (self.borderSize.width * 2.0)) /
	self.tileSize.width);

    if (newRows < 1 || newColumns < 1) return;
    [self resizeWithColumns:newColumns rows:newRows];

    if( saveToDefaults )
    {
        int termIndex = [self terminalIndex];
        NSArray *terminals = [[NSUserDefaults standardUserDefaults] valueForKey: AngbandTerminalsDefaultsKey];

        if( termIndex < (int)[terminals count] )
        {
            NSMutableDictionary *mutableTerm = [[NSMutableDictionary alloc] initWithDictionary: [terminals objectAtIndex: termIndex]];
            [mutableTerm setValue: [NSNumber numberWithInteger: self.cols]
			 forKey: AngbandTerminalColumnsDefaultsKey];
            [mutableTerm setValue: [NSNumber numberWithInteger: self.rows]
			 forKey: AngbandTerminalRowsDefaultsKey];

            NSMutableArray *mutableTerminals = [[NSMutableArray alloc] initWithArray: terminals];
            [mutableTerminals replaceObjectAtIndex: termIndex withObject: mutableTerm];

            [[NSUserDefaults standardUserDefaults] setValue: mutableTerminals forKey: AngbandTerminalsDefaultsKey];
        }
    }

    term *old = Term;
    Term_activate( self->terminal );
    Term_resize( self.cols, self.rows );
    Term_redraw();
    Term_activate( old );
}

- (void)constrainWindowSize:(int)termIdx
{
    NSSize minsize;

    if (termIdx == 0) {
	minsize.width = 80;
	minsize.height = 24;
    } else {
	minsize.width = 1;
	minsize.height = 1;
    }
    minsize.width =
	minsize.width * self.tileSize.width + self.borderSize.width * 2.0;
    minsize.height =
        minsize.height * self.tileSize.height + self.borderSize.height * 2.0;
    [[self makePrimaryWindow] setContentMinSize:minsize];
    self.primaryWindow.contentResizeIncrements = self.tileSize;
}

- (void)saveWindowVisibleToDefaults: (BOOL)windowVisible
{
	int termIndex = [self terminalIndex];
	BOOL safeVisibility = (termIndex == 0) ? YES : windowVisible; /* Ensure main term doesn't go away because of these defaults */
	NSArray *terminals = [[NSUserDefaults standardUserDefaults] valueForKey: AngbandTerminalsDefaultsKey];

	if( termIndex < (int)[terminals count] )
	{
		NSMutableDictionary *mutableTerm = [[NSMutableDictionary alloc] initWithDictionary: [terminals objectAtIndex: termIndex]];
		[mutableTerm setValue: [NSNumber numberWithBool: safeVisibility] forKey: AngbandTerminalVisibleDefaultsKey];

		NSMutableArray *mutableTerminals = [[NSMutableArray alloc] initWithArray: terminals];
		[mutableTerminals replaceObjectAtIndex: termIndex withObject: mutableTerm];

		[[NSUserDefaults standardUserDefaults] setValue: mutableTerminals forKey: AngbandTerminalsDefaultsKey];
	}
}

- (BOOL)windowVisibleUsingDefaults
{
	int termIndex = [self terminalIndex];

	if( termIndex == 0 )
	{
		return YES;
	}

	NSArray *terminals = [[NSUserDefaults standardUserDefaults] valueForKey: AngbandTerminalsDefaultsKey];
	BOOL visible = NO;

	if( termIndex < (int)[terminals count] )
	{
		NSDictionary *term = [terminals objectAtIndex: termIndex];
		NSNumber *visibleValue = [term valueForKey: AngbandTerminalVisibleDefaultsKey];

		if( visibleValue != nil )
		{
			visible = [visibleValue boolValue];
		}
	}

	return visible;
}

#pragma mark -
#pragma mark NSWindowDelegate Methods

/*- (void)windowWillStartLiveResize: (NSNotification *)notification
{ 
}*/ 

- (void)windowDidEndLiveResize: (NSNotification *)notification
{
    NSWindow *window = [notification object];
    NSRect contentRect = [window contentRectForFrameRect: [window frame]];
    [self resizeTerminalWithContentRect: contentRect saveToDefaults: !(self->inFullscreenTransition)];
}

/*- (NSSize)windowWillResize: (NSWindow *)sender toSize: (NSSize)frameSize
{
} */

- (void)windowWillEnterFullScreen: (NSNotification *)notification
{
    self->inFullscreenTransition = YES;
}

- (void)windowDidEnterFullScreen: (NSNotification *)notification
{
    NSWindow *window = [notification object];
    NSRect contentRect = [window contentRectForFrameRect: [window frame]];
    self->inFullscreenTransition = NO;
    [self resizeTerminalWithContentRect: contentRect saveToDefaults: NO];
}

- (void)windowWillExitFullScreen: (NSNotification *)notification
{
    self->inFullscreenTransition = YES;
}

- (void)windowDidExitFullScreen: (NSNotification *)notification
{
    NSWindow *window = [notification object];
    NSRect contentRect = [window contentRectForFrameRect: [window frame]];
    self->inFullscreenTransition = NO;
    [self resizeTerminalWithContentRect: contentRect saveToDefaults: NO];
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    NSWindow *window = [notification object];

    if( window != self.primaryWindow )
    {
        return;
    }

    int termIndex = [self terminalIndex];
    NSMenuItem *item = [[[NSApplication sharedApplication] windowsMenu] itemWithTag: AngbandWindowMenuItemTagBase + termIndex];
    [item setState: NSOnState];

    if( [[NSFontPanel sharedFontPanel] isVisible] )
    {
        [[NSFontPanel sharedFontPanel] setPanelFont:self.angbandViewFont
				       isMultiple: NO];
    }
}

- (void)windowDidResignMain: (NSNotification *)notification
{
    NSWindow *window = [notification object];

    if( window != self.primaryWindow )
    {
        return;
    }

    int termIndex = [self terminalIndex];
    NSMenuItem *item = [[[NSApplication sharedApplication] windowsMenu] itemWithTag: AngbandWindowMenuItemTagBase + termIndex];
    [item setState: NSOffState];
}

- (void)windowWillClose: (NSNotification *)notification
{
    /*
     * If closing only because the application is terminating, don't update
     * the visible state for when the application is relaunched.
     */
    if (! quit_when_ready) {
	[self saveWindowVisibleToDefaults: NO];
    }
}

@end


@implementation AngbandView

- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)rect
{
    if ([self inLiveResize]) {
	/*
	 * Always anchor the cached area to the upper left corner of the view.
	 * Any parts on the right or bottom that can't be drawn from the cached
	 * area are simply cleared.  Will fill them with appropriate content
	 * when resizing is done.
	 */
	const NSRect *rects;
	NSInteger count;

	[self getRectsBeingDrawn:&rects count:&count];
	if (count > 0) {
	    NSRect viewRect = [self visibleRect];

	    [[NSColor blackColor] set];
	    while (count-- > 0) {
		CGFloat drawTop = rects[count].origin.y - viewRect.origin.y;
		CGFloat drawBottom = drawTop + rects[count].size.height;
		CGFloat drawLeft = rects[count].origin.x - viewRect.origin.x;
		CGFloat drawRight = drawLeft + rects[count].size.width;
		/*
		 * modRect and clrRect, like rects[count], are in the view
		 * coordinates with y flipped.  cacheRect is in the bitmap
		 * coordinates and y is not flipped.
		 */
		NSRect modRect, clrRect, cacheRect;

		/*
		 * Clip by bottom edge of cached area.  Clear what's below
		 * that.
		 */
		if (drawTop >= self->cacheBounds.size.height) {
		    NSRectFill(rects[count]);
		    continue;
		}
		modRect.origin.x = rects[count].origin.x;
		modRect.origin.y = rects[count].origin.y;
		modRect.size.width = rects[count].size.width;
		cacheRect.origin.y = drawTop;
		if (drawBottom > self->cacheBounds.size.height) {
		    CGFloat excess =
			drawBottom - self->cacheBounds.size.height;

		    modRect.size.height = rects[count].size.height - excess;
		    cacheRect.origin.y = 0;
		    clrRect.origin.x = modRect.origin.x;
		    clrRect.origin.y = modRect.origin.y + modRect.size.height;
		    clrRect.size.width = modRect.size.width;
		    clrRect.size.height = excess;
		    NSRectFill(clrRect);
		} else {
		    modRect.size.height = rects[count].size.height;
		    cacheRect.origin.y = self->cacheBounds.size.height -
			rects[count].size.height;
		}
		cacheRect.size.height = modRect.size.height;

		/*
		 * Clip by right edge of cached area.  Clear what's to the
		 * right of that and copy the remainder from the cache.
		 */
		if (drawLeft >= self->cacheBounds.size.width) {
		    NSRectFill(modRect);
		    continue;
		}
		cacheRect.origin.x = drawLeft;
		if (drawRight > self->cacheBounds.size.width) {
		    CGFloat excess = drawRight - self->cacheBounds.size.width;

		    modRect.size.width -= excess;
		    cacheRect.size.width =
			self->cacheBounds.size.width - drawLeft;
		    clrRect.origin.x = modRect.origin.x + modRect.size.width;
		    clrRect.origin.y = modRect.origin.y;
		    clrRect.size.width = excess;
		    clrRect.size.height = modRect.size.height;
		    NSRectFill(clrRect);
		} else {
		    cacheRect.size.width = drawRight - drawLeft;
		}
		[self->cacheForResize drawInRect:modRect fromRect:cacheRect
		     operation:NSCompositeCopy fraction:1.0
		     respectFlipped:YES hints:nil];
	    }
	}
    } else if (! self.angbandContext) {
        /* Draw bright orange, 'cause this ain't right */
        [[NSColor orangeColor] set];
        NSRectFill([self bounds]);
    } else {
        /* Tell the Angband context to draw into us */
        [self.angbandContext drawRect:rect inView:self];
    }
}

/**
 * Override NSView's method to set up a cache that's used in drawRect to
 * handle drawing during a resize.
 */
- (void)viewWillStartLiveResize
{
    [super viewWillStartLiveResize];
    self->cacheBounds = [self visibleRect];
    self->cacheForResize =
	[self bitmapImageRepForCachingDisplayInRect:self->cacheBounds];
    if (self->cacheForResize != nil) {
	[self cacheDisplayInRect:self->cacheBounds
	      toBitmapImageRep:self->cacheForResize];
    } else {
	self->cacheBounds.size.width = 0.;
	self->cacheBounds.size.height = 0.;
    }
}

/**
 * Override NSView's method to release the cache set up in
 * viewWillStartLiveResize.
 */
- (void)viewDidEndLiveResize
{
    [super viewDidEndLiveResize];
    self->cacheForResize = nil;
    [self setNeedsDisplay:YES];
}

@end

/**
 * Delay handling of double-clicked savefiles
 */
Boolean open_when_ready = FALSE;



/**
 * ------------------------------------------------------------------------
 * Some generic functions
 * ------------------------------------------------------------------------ */

/**
 * Given an Angband color index, returns the index into angband_color_table
 * to be used as the background color.  The returned index is between -1 and
 * MAX_COLORS - 1 inclusive where -1 means use the RGB triplet, (0, 0, 0).
 */
static int get_background_color_index(int idx)
{
    /*
     * The Cocoa interface has always been using (0,0,0) as the clear color
     * and not angband_color_table[0].  As of January 2020,  the Window's
     * interface uses both ((0,0,0) in Term_xtra_win_clear() and
     * Term_xtra_wipe_win() and win_clr[0], which is set from
     * angband_color_table[0], in Term_text_win() for the BG_BLACK case.
     */
    int ibkg = -1;

    switch (idx / MAX_COLORS) {
    case BG_BLACK:
	/* There's nothing to do. */
	break;

    case BG_SAME:
	ibkg = idx % MAX_COLORS;
	break;

    case BG_DARK:
	ibkg = COLOUR_SHADE;
	break;

    default:
	assert(0);
    }

    return ibkg;
}


/**
 * Sets an Angband color at a given index
 */
static void set_color_for_index(int idx)
{
    u16b rv, gv, bv;
    
    /* Extract the R,G,B data */
    rv = angband_color_table[idx][1];
    gv = angband_color_table[idx][2];
    bv = angband_color_table[idx][3];
    
    CGContextSetRGBFillColor([[NSGraphicsContext currentContext] graphicsPort], rv/255., gv/255., bv/255., 1.);
}

/**
 * Remember the current character in UserDefaults so we can select it by
 * default next time.
 */
static void record_current_savefile(void)
{
    NSString *savefileString = [[NSString stringWithCString:savefile encoding:NSMacOSRomanStringEncoding] lastPathComponent];
    if (savefileString)
    {
        NSUserDefaults *angbandDefs = [NSUserDefaults angbandDefaults];
        [angbandDefs setObject:savefileString forKey:@"SaveFile"];
    }
}


/**
 * ------------------------------------------------------------------------
 * Support for the "ui-term.c" package
 * ------------------------------------------------------------------------ */


/**
 * Initialize a new Term
 */
static void Term_init_cocoa(term *t)
{
    @autoreleasepool {
	AngbandContext *context = [[AngbandContext alloc] init];

	/* Give the term ownership of the context */
	t->data = (void *)CFBridgingRetain(context);

	/* Handle graphics */
	t->higher_pict = !! use_graphics;
	t->always_pict = FALSE;

	NSDisableScreenUpdates();

	/*
	 * Figure out the frame autosave name based on the index of this term
	 */
	NSString *autosaveName = nil;
	int termIdx;
	for (termIdx = 0; termIdx < ANGBAND_TERM_MAX; termIdx++)
	{
	    if (angband_term[termIdx] == t)
	    {
		autosaveName =
		    [NSString stringWithFormat:@"AngbandTerm-%d", termIdx];
		break;
	    }
	}

	/* Set its font. */
	NSString *fontName =
	    [[NSUserDefaults angbandDefaults]
		stringForKey:[NSString stringWithFormat:@"FontName-%d", termIdx]];
	if (! fontName) fontName = [[AngbandContext defaultFont] fontName];

	/*
	 * Use a smaller default font for the other windows, but only if the
	 * font hasn't been explicitly set.
	 */
	float fontSize =
	    (termIdx > 0) ? 10.0 : [[AngbandContext defaultFont] pointSize];
	NSNumber *fontSizeNumber =
	    [[NSUserDefaults angbandDefaults]
		valueForKey: [NSString stringWithFormat: @"FontSize-%d", termIdx]];

	if( fontSizeNumber != nil )
	{
	    fontSize = [fontSizeNumber floatValue];
	}

	[context setSelectionFont:[NSFont fontWithName:fontName size:fontSize]
		 adjustTerminal: NO];

	NSArray *terminalDefaults =
	    [[NSUserDefaults standardUserDefaults]
		valueForKey: AngbandTerminalsDefaultsKey];
	NSInteger rows = 24;
	NSInteger columns = 80;

	if( termIdx < (int)[terminalDefaults count] )
	{
	    NSDictionary *term = [terminalDefaults objectAtIndex: termIdx];
	    NSInteger defaultRows =
		[[term valueForKey: AngbandTerminalRowsDefaultsKey]
		    integerValue];
	    NSInteger defaultColumns =
		[[term valueForKey: AngbandTerminalColumnsDefaultsKey]
		    integerValue];

	    if (defaultRows > 0) rows = defaultRows;
	    if (defaultColumns > 0) columns = defaultColumns;
	}

	[context resizeWithColumns:columns rows:rows];

	/* Get the window */
	NSWindow *window = [context makePrimaryWindow];

	/* Set its title and, for auxiliary terms, tentative size */
	if (termIdx == 0)
	{
	    [window setTitle:@VERSION_NAME];
	}
	else
	{
	    [window setTitle:[NSString stringWithFormat:@"Term %d", termIdx]];
	}
	[context constrainWindowSize:termIdx];

	/*
	 * If this is the first term, and we support full screen (Mac OS X Lion
	 * or later), then allow it to go full screen (sweet). Allow other
	 * terms to be FullScreenAuxilliary, so they can at least show up.
	 * Unfortunately in Lion they don't get brought to the full screen
	 * space; but they would only make sense on multiple displays anyways
	 * so it's not a big loss.
	 */
	if ([window respondsToSelector:@selector(toggleFullScreen:)])
	{
	    NSWindowCollectionBehavior behavior = [window collectionBehavior];
	    behavior |=
		(termIdx == 0 ?
		 Angband_NSWindowCollectionBehaviorFullScreenPrimary :
		 Angband_NSWindowCollectionBehaviorFullScreenAuxiliary);
	    [window setCollectionBehavior:behavior];
	}

	/* No Resume support yet, though it would not be hard to add */
	if ([window respondsToSelector:@selector(setRestorable:)])
	{
	    [window setRestorable:NO];
	}

	/* default window placement */ {
	    static NSRect overallBoundingRect;

	    if( termIdx == 0 )
	    {
		/*
		 * This is a bit of a trick to allow us to display multiple
		 * windows in the "standard default" window position in OS X:
		 * the upper center of the screen.  The term sizes set in
		 * load_prefs() are based on a 5-wide by 3-high grid, with the
		 * main term being 4/5 wide by 2/3 high (hence the scaling to
		 * find what the containing rect would be).
		 */
		NSRect originalMainTermFrame = [window frame];
		NSRect scaledFrame = originalMainTermFrame;
		scaledFrame.size.width *= 5.0 / 4.0;
		scaledFrame.size.height *= 3.0 / 2.0;
		scaledFrame.size.width += 1.0; /* spacing between window columns */
		scaledFrame.size.height += 1.0; /* spacing between window rows */
		[window setFrame: scaledFrame  display: NO];
		[window center];
		overallBoundingRect = [window frame];
		[window setFrame: originalMainTermFrame display: NO];
	    }

	    static NSRect mainTermBaseRect;
	    NSRect windowFrame = [window frame];

	    if( termIdx == 0 )
	    {
		/*
		 * The height and width adjustments were determined
		 * experimentally, so that the rest of the windows line up
		 * nicely without overlapping.
		 */
		windowFrame.size.width += 7.0;
		windowFrame.size.height += 9.0;
		windowFrame.origin.x = NSMinX( overallBoundingRect );
		windowFrame.origin.y =
		    NSMaxY( overallBoundingRect ) - NSHeight( windowFrame );
		mainTermBaseRect = windowFrame;
	    }
	    else if( termIdx == 1 )
	    {
		windowFrame.origin.x = NSMinX( mainTermBaseRect );
		windowFrame.origin.y =
		    NSMinY( mainTermBaseRect ) - NSHeight( windowFrame ) - 1.0;
	    }
	    else if( termIdx == 2 )
	    {
		windowFrame.origin.x = NSMaxX( mainTermBaseRect ) + 1.0;
		windowFrame.origin.y =
		    NSMaxY( mainTermBaseRect ) - NSHeight( windowFrame );
	    }
	    else if( termIdx == 3 )
	    {
		windowFrame.origin.x = NSMaxX( mainTermBaseRect ) + 1.0;
		windowFrame.origin.y =
		    NSMinY( mainTermBaseRect ) - NSHeight( windowFrame ) - 1.0;
	    }
	    else if( termIdx == 4 )
	    {
		windowFrame.origin.x = NSMaxX( mainTermBaseRect ) + 1.0;
		windowFrame.origin.y = NSMinY( mainTermBaseRect );
	    }
	    else if( termIdx == 5 )
	    {
		windowFrame.origin.x =
		    NSMinX( mainTermBaseRect ) + NSWidth( windowFrame ) + 1.0;
		windowFrame.origin.y =
		    NSMinY( mainTermBaseRect ) - NSHeight( windowFrame ) - 1.0;
	    }

	    [window setFrame: windowFrame display: NO];
	}

	/*
	 * Override the default frame above if the user has adjusted windows in
	 * the past
	 */
	if (autosaveName) [window setFrameAutosaveName:autosaveName];

	/*
	 * Tell it about its term. Do this after we've sized it so that the
	 * sizing doesn't trigger redrawing and such.
	 */
	[context setTerm:t];

	/*
	 * Only order front if it's the first term. Other terms will be ordered
	 * front from AngbandUpdateWindowVisibility(). This is to work around a
	 * problem where Angband aggressively tells us to initialize terms that
	 * don't do anything!
	 */
	if (t == angband_term[0])
	    [context.primaryWindow makeKeyAndOrderFront: nil];

	NSEnableScreenUpdates();

	/* Set "mapped" flag */
	t->mapped_flag = true;
    }
}



/**
 * Nuke an old Term
 */
static void Term_nuke_cocoa(term *t)
{
    @autoreleasepool {
        AngbandContext *context = (__bridge AngbandContext*) (t->data);
	if (context)
	{
	    /* Tell the context to get rid of its windows, etc. */
	    [context dispose];

	    /* Balance our CFBridgingRetain from when we created it */
	    CFRelease(t->data);

	    /* Done with it */
	    t->data = NULL;
	}
    }
}

/**
 * Returns the CGImageRef corresponding to an image with the given path.
 * Transfers ownership to the caller.
 */
static CGImageRef create_angband_image(NSString *path)
{
    CGImageRef decodedImage = NULL, result = NULL;
    
    /* Try using ImageIO to load the image */
    if (path)
    {
        NSURL *url = [[NSURL alloc] initFileURLWithPath:path isDirectory:NO];
        if (url)
        {
            NSDictionary *options = [[NSDictionary alloc] initWithObjectsAndKeys:(id)kCFBooleanTrue, kCGImageSourceShouldCache, nil];
            CGImageSourceRef source = CGImageSourceCreateWithURL((CFURLRef)url, (CFDictionaryRef)options);
            if (source)
            {
                /*
                 * We really want the largest image, but in practice there's
                 * only going to be one
                 */
                decodedImage = CGImageSourceCreateImageAtIndex(source, 0, (CFDictionaryRef)options);
                CFRelease(source);
            }
        }
    }
    
    /*
     * Draw the sucker to defeat ImageIO's weird desire to cache and decode on
     * demand. Our images aren't that big!
     */
    if (decodedImage)
    {
        size_t width = CGImageGetWidth(decodedImage), height = CGImageGetHeight(decodedImage);
        
        /* Compute our own bitmap info */
        CGBitmapInfo imageBitmapInfo = CGImageGetBitmapInfo(decodedImage);
        CGBitmapInfo contextBitmapInfo = kCGBitmapByteOrderDefault;
        
        switch (imageBitmapInfo & kCGBitmapAlphaInfoMask) {
            case kCGImageAlphaNone:
            case kCGImageAlphaNoneSkipLast:
            case kCGImageAlphaNoneSkipFirst:
                /* No alpha */
                contextBitmapInfo |= kCGImageAlphaNone;
                break;
            default:
                /* Some alpha, use premultiplied last which is most efficient. */
                contextBitmapInfo |= kCGImageAlphaPremultipliedLast;
                break;
        }

        /* Draw the source image flipped, since the view is flipped */
        CGContextRef ctx = CGBitmapContextCreate(NULL, width, height, CGImageGetBitsPerComponent(decodedImage), CGImageGetBytesPerRow(decodedImage), CGImageGetColorSpace(decodedImage), contextBitmapInfo);
        if (ctx) {
	    CGContextSetBlendMode(ctx, kCGBlendModeCopy);
	    CGContextTranslateCTM(ctx, 0.0, height);
	    CGContextScaleCTM(ctx, 1.0, -1.0);
	    CGContextDrawImage(
		ctx, CGRectMake(0, 0, width, height), decodedImage);
	    result = CGBitmapContextCreateImage(ctx);
	    CFRelease(ctx);
	}

        CGImageRelease(decodedImage);
    }
    return result;
}

/**
 * React to changes
 */
static errr Term_xtra_cocoa_react(void)
{
    /* Don't actually switch graphics until the game is running */
    if (!initialized || !game_in_progress) return (-1);

    @autoreleasepool {
	/* Handle graphics */
	int expected_graf_mode = (current_graphics_mode) ?
	    current_graphics_mode->grafID : GRAPHICS_NONE;
	if (graf_mode_req != expected_graf_mode)
	{
	    graphics_mode *new_mode;
	    if (graf_mode_req != GRAPHICS_NONE) {
		new_mode = get_graphics_mode(graf_mode_req);
	    } else {
		new_mode = NULL;
	    }

	    /* Get rid of the old image. CGImageRelease is NULL-safe. */
	    CGImageRelease(pict_image);
	    pict_image = NULL;

	    /* Try creating the image if we want one */
	    if (new_mode != NULL)
	    {
		NSString *img_path =
		    [NSString stringWithFormat:@"%s/%s", new_mode->path, new_mode->file];
		pict_image = create_angband_image(img_path);

		/* If we failed to create the image, revert to ASCII. */
		if (! pict_image) {
		    new_mode = NULL;
		    if (tile_width != 1 || tile_height != 1) {
			tile_width = 1;
			tile_height = 1;
			tile_multipliers_changed = 1;
		    }
		    [[NSUserDefaults angbandDefaults]
			setInteger:GRAPHICS_NONE
			forKey:AngbandGraphicsDefaultsKey];

		    NSAlert *alert = [[NSAlert alloc] init];
		    alert.messageText = @"Failed to Load Tile Set";
		    alert.informativeText = @"Could not load the tile set.  Switched back to ASCII.";
		    [alert runModal];
		}
	    }

	    if (graphics_are_enabled()) {
		/*
		 * The contents stored in the AngbandContext may have
		 * references to the old tile set.  Out of an abundance
		 * of caution, clear those references in case there's an
		 * attempt to redraw the contents before the core has the
		 * chance to update it via the text_hook, pict_hook, and
		 * wipe_hook.
		 */
		for (int iterm = 0; iterm < ANGBAND_TERM_MAX; ++iterm) {
		    AngbandContext* aContext =
			(__bridge AngbandContext*) (angband_term[iterm]->data);

		    [aContext.contents wipeTiles];
		}
	    }

	    /* Record what we did */
	    use_graphics = new_mode ? new_mode->grafID : 0;
	    current_graphics_mode = new_mode;

	    /* Enable or disable higher picts.  */
	    for (int iterm = 0; iterm < ANGBAND_TERM_MAX; ++iterm) {
		if (angband_term[iterm]) {
		    angband_term[iterm]->higher_pict = !! use_graphics;
		}
	    }

	    if (pict_image && current_graphics_mode)
	    {
		/*
		 * Compute the row and column count via the image height and
		 * width.
		 */
		pict_rows = (int)(CGImageGetHeight(pict_image) /
				  current_graphics_mode->cell_height);
		pict_cols = (int)(CGImageGetWidth(pict_image) /
				  current_graphics_mode->cell_width);
	    }
	    else
	    {
		pict_rows = 0;
		pict_cols = 0;
	    }

	    /* Reset visuals */
	    if (! tile_multipliers_changed)
	    {
		reset_visuals(TRUE);
	    }
	}

	if (tile_multipliers_changed)
	{
	    /* Reset visuals */
	    reset_visuals(TRUE);

	    if (character_dungeon) {
		/*
		 * Reset the panel.  Only do so if have a dungeon; otherwise
		 * can see crashes if changing graphics or the font before
		 * or during character generation.
		 */
		verify_panel();
	    }

	    tile_multipliers_changed = 0;
	}
    }

    /* Success */
    return (0);
}


/**
 * Do a "special thing"
 */
static errr Term_xtra_cocoa(int n, int v)
{
    errr result = 0;
    @autoreleasepool {
	AngbandContext* angbandContext =
	    (__bridge AngbandContext*) (Term->data);

	/* Analyze */
	switch (n) {
	    /* Make a noise */
        case TERM_XTRA_NOISE:
	    NSBeep();
	    break;

	    /* Process random events */
        case TERM_XTRA_BORED:
	    /*
	     * Show or hide cocoa windows based on the subwindow flags set by
	     * the user.
	     */
	    AngbandUpdateWindowVisibility();
	    /* Process an event */
	    (void)check_events(CHECK_EVENTS_NO_WAIT);
	    break;

	    /* Process pending events */
        case TERM_XTRA_EVENT:
	    /* Process an event */
	    (void)check_events(v);
	    break;

	    /* Flush all pending events (if any) */
        case TERM_XTRA_FLUSH:
	    /* Hack -- flush all events */
	    while (check_events(CHECK_EVENTS_DRAIN)) /* loop */;

	    break;

	    /* Hack -- Change the "soft level" */
        case TERM_XTRA_LEVEL:
	    /*
	     * Here we could activate (if requested), but I don't think
	     * Angband should be telling us our window order (the user
	     * should decide that), so do nothing.
	     */
	    break;

	    /* Clear the screen */
        case TERM_XTRA_CLEAR:
	    [angbandContext.contents wipe];
	    [angbandContext setNeedsDisplay:YES];
	    break;

	    /* React to changes */
        case TERM_XTRA_REACT:
	    result = Term_xtra_cocoa_react();
	    break;

	    /* Delay (milliseconds) */
        case TERM_XTRA_DELAY:
	    /* If needed */
	    if (v > 0) {
		double seconds = v / 1000.;
		NSDate* date = [NSDate dateWithTimeIntervalSinceNow:seconds];
		do {
		    NSEvent* event;
		    do {
			event = [NSApp nextEventMatchingMask:-1
				       untilDate:date
				       inMode:NSDefaultRunLoopMode
				       dequeue:YES];
			if (event) send_event(event);
		    } while (event);
		} while ([date timeIntervalSinceNow] >= 0);
	    }
	    break;

	    /* Draw the pending changes. */
        case TERM_XTRA_FRESH:
	    {
		/*
		 * Check the cursor visibility since the core will tell us
		 * explicitly to draw it, but tells us implicitly to forget it
		 * by simply telling us to redraw a location.
		 */
		bool isVisible = 0;

		Term_get_cursor(&isVisible);
		if (! isVisible) {
		    [angbandContext.contents removeCursor];
		}
		[angbandContext computeInvalidRects];
		[angbandContext.changes clear];
	    }
            break;

        default:
            /* Oops */
            result = 1;
            break;
	}
    }

    return result;
}

static errr Term_curs_cocoa(int x, int y)
{
    AngbandContext *angbandContext = (__bridge AngbandContext*) (Term->data);

    [angbandContext.contents setCursorAtColumn:x row:y width:1 height:1];
    /*
     * Unfortunately, this (and the same logic in Term_bigcurs_cocoa) will
     * also trigger what's under the cursor to be redrawn as well, even if
     * it has not changed.  In the current drawing implementation, that
     * inefficiency seems unavoidable.
     */
    [angbandContext.changes markChangedAtColumn:x row:y];

    /* Success */
    return 0;
}

static errr Term_bigcurs_cocoa(int x, int y)
{
    AngbandContext *angbandContext = (__bridge AngbandContext*) (Term->data);

    [angbandContext.contents setCursorAtColumn:x row:y width:tile_width
		   height:tile_height];
    [angbandContext.changes markChangedBlockAtColumn:x row:y width:tile_width
		   height:tile_height];

    /* Success */
    return 0;
}

/**
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_cocoa(int x, int y, int n)
{
    AngbandContext *angbandContext = (__bridge AngbandContext*) (Term->data);

    [angbandContext.contents wipeBlockAtColumn:x row:y width:n height:1];
    [angbandContext.changes markChangedRangeAtColumn:x row:y width:n];

    /* Success */
    return 0;
}

static errr Term_pict_cocoa(int x, int y, int n, const int *ap,
                            const wchar_t *cp, const int *tap,
                            const wchar_t *tcp)
{
    /* Paranoia: Bail if graphics aren't enabled */
    if (! graphics_are_enabled()) return -1;

    AngbandContext* angbandContext = (__bridge AngbandContext*) (Term->data);

    /*
     * At least with 4.2.0 Angband, there's cases where Term_pict is called
     * for the overhead view subwindow and y + tile_width is greater than the
     * number of rows.  Protect both dimensions.
     */
    if (y + tile_height > angbandContext.rows) {
	return -1;
    }
    if (x + n * tile_width >= angbandContext.cols) {
	n = (angbandContext.cols - x) / tile_width;
	if (n < 1) {
	    return -1;
	}
    }

    int overdraw_row, overdraw_max, alphablend;
    if (use_graphics) {
	CGImageAlphaInfo ainfo = CGImageGetAlphaInfo(pict_image);

	overdraw_row = current_graphics_mode->overdrawRow;
	overdraw_max = current_graphics_mode->overdrawMax;
	alphablend = (ainfo & (kCGImageAlphaPremultipliedFirst |
			       kCGImageAlphaPremultipliedLast)) ? 1 : 0;
    } else {
	overdraw_row = 0;
	overdraw_max = 0;
	alphablend = 0;
    }

    for (int i = x; i < x + n * tile_width; i += tile_width) {
	int a = *ap;
	wchar_t c = *cp;
	int ta = *tap;
	char tc = *tcp;

	ap += tile_width;
	cp += tile_width;
	tap += tile_width;
	tcp += tile_width;
	if (use_graphics && (a & 0x80) && (c & 0x80)) {
	    char fgdRow = ((byte)a & 0x7F) % pict_rows;
	    char fgdCol = ((byte)c & 0x7F) % pict_cols;
	    char bckRow, bckCol;

	    if (alphablend) {
		bckRow = ((byte)ta & 0x7F) % pict_rows;
		bckCol = ((byte)tc & 0x7F) % pict_cols;
	    } else {
		/*
		 * Not blending so make the background the same as the
		 * the foreground.
		 */
		bckRow = fgdRow;
		bckCol = fgdCol;
	    }
	    [angbandContext.contents setTileAtColumn:i row:y
			   foregroundColumn:fgdCol
			   foregroundRow:fgdRow
			   backgroundColumn:bckCol
			   backgroundRow:bckRow
			   tileWidth:tile_width
			   tileHeight:tile_height];
	    if (overdraw_row && y > 2 &&
		((bckRow >= overdraw_row && bckRow <= overdraw_max) ||
		 (fgdRow >= overdraw_row && fgdRow <= overdraw_max))) {
		[angbandContext.changes markChangedBlockAtColumn:i
			       row:(y - tile_height) width:tile_width
			       height:(tile_height + tile_height)];
		/*
		 * Either the foreground or the background is a double-height
		 * tile.  Need to tell the core to redraw the upper half in
		 * the next update since what's displayed there no longer
		 * corresponds to what the core thinks is there.  Also tell
		 * the core to redraw the lower half in the next update
		 * because, if it remains a double-height tile, that is
		 * necessary to trigger the drawing of the upper half.
		 */
		Term_mark(i, y - tile_height);
		Term_mark(i, y);
	    } else {
		[angbandContext.changes markChangedBlockAtColumn:i row:y
			       width:tile_width height:tile_height];
	    }
	}
    }

    /* Success */
    return (0);
}

/**
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 */
static errr Term_text_cocoa(int x, int y, int n, int a, const wchar_t *cp)
{
    AngbandContext* angbandContext = (__bridge AngbandContext*) (Term->data);

    [angbandContext.contents setUniformAttributeTextRunAtColumn:x
		   row:y n:n glyphs:cp attribute:a];
    [angbandContext.changes markChangedRangeAtColumn:x row:y width:n];

    /* Success */
    return 0;
}

/**
 * Convert UTF-8 to UTF-32 with each UTF-32 stored in the native byte order as
 * a wchar_t.  Return the total number of code points that would be generated
 * by converting the UTF-8 input.
 *
 * \param dest Points to the buffer in which to store the conversion.  May be
 * NULL.
 * \param src Is a null-terminated UTF-8 sequence.
 * \param n Is the maximum number of code points to store in dest.
 *
 * In case of malformed UTF-8, inserts a U+FFFD in the converted output at the
 * point of the error.
 */
static size_t Term_mbcs_cocoa(wchar_t *dest, const char *src, int n)
{
    size_t nout = (n > 0) ? n : 0;
    size_t count = 0;

    while (1) {
	/*
	 * Default to U+FFFD to indicate an erroneous UTF-8 sequence that
	 * could not be decoded.  Follow "best practice" recommended by the
	 * Unicode 6 standard:  an erroneous sequence ends as soon as a
	 * disallowed byte is encountered.
         */
	unsigned int decoded = 0xfffd;

	if (((unsigned int) *src & 0x80) == 0) {
            /* Encoded as single byte:  U+0000 to U+0007F -> 0xxxxxxx. */
	    if (*src == 0) {
		if (dest && count < nout) {
                    dest[count] = 0;
		}
		break;
	    }
	    decoded = *src;
	    ++src;
	} else if (((unsigned int) *src & 0xe0) == 0xc0) {
	    /* Encoded as two bytes:  U+0080 to U+07FF -> 110xxxxx 10xxxxxx. */
	    unsigned int part = ((unsigned int) *src & 0x1f) << 6;

	    ++src;
	    /*
	     * Check that the first two bits of the continuation byte are
	     * valid and the encoding is not overlong.
	     */
	    if (((unsigned int) *src & 0xc0) == 0x80 && part > 0x40) {
		decoded = part + ((unsigned int) *src & 0x3f);
		++src;
	    }
	} else if (((unsigned int) *src & 0xf0) == 0xe0) {
	    /*
	     * Encoded as three bytes:  U+0800 to U+FFFF -> 1110xxxx 10xxxxxx
	     * 10xxxxxx.
	     */
	    unsigned int part = ((unsigned int) *src & 0xf) << 12;

	    ++src;
	    if (((unsigned int) *src & 0xc0) == 0x80) {
		part += ((unsigned int) *src & 0x3f) << 6;
		++src;
		/*
		 * The second part of the test rejects overlong encodings.  The
		 * third part rejects encodings of U+D800 to U+DFFF, reserved
		 * for surrogate pairs.
		 */
		if (((unsigned int) *src & 0xc0) == 0x80 && part >= 0x800 &&
			(part & 0xf800) != 0xd800) {
		    decoded = part + ((unsigned int) *src & 0x3f);
		    ++src;
		}
	    }
	} else if (((unsigned int) *src & 0xf8) == 0xf0) {
	    /*
	     * Encoded as four bytes:  U+10000 to U+1FFFFF -> 11110xxx 10xxxxxx
	     * 10xxxxxx 10xxxxxx.
	     */
	    unsigned int part = ((unsigned int) *src & 0x7) << 18;

	    ++src;
	    if (((unsigned int) *src & 0xc0) == 0x80) {
		part += ((unsigned int) *src & 0x3f) << 12;
		++src;
		/*
		 * The second part of the test rejects overlong encodings.
		 * The third part rejects code points beyond U+10FFFF which
		 * can't be encoded in UTF-16.
		 */
		if (((unsigned int) *src & 0xc0) == 0x80 && part >= 0x10000 &&
			(part & 0xff0000) <= 0x100000) {
		    part += ((unsigned int) *src & 0x3f) << 6;
		    ++src;
		    if (((unsigned int) *src & 0xc0) == 0x80) {
			decoded = part + ((unsigned int) *src & 0x3f);
			++src;
		    }
		}
	    }
	} else {
	    /*
	     * Either an impossible byte or one that signals the start of a
	     * five byte or longer encoding.
	     */
	    ++src;
	}
	if (dest && count < nout) {
	    dest[count] = decoded;
	}
	++count;
    }
    return count;
}

/**
 * Convert a UTF-32 stored in the native byte order to UTF-8.
 * \param s Points to the buffer where the conversion should be stored.
 * That buffer must have at least Term_wcsz_cocoa() bytes.
 * \param wchar Is the UTF-32 value to convert.
 * \return The returned value is the number of bytes written to s or -1
 * if the UTF-32 value could not be converted.
 *
 * This is a necessary counterpart to Term_mbcs_cocoa():  since we are
 * overriding the default multibyte to wide character conversion, need to
 * override the reverse conversion as well.
 */
static int Term_wctomb_cocoa(char *s, wchar_t wchar)
{
    if (wchar < 0) {
        /* Refuse to encode something beyond the range encodeable by UTF-16. */
        return -1;
    }
    if (wchar <= 0x7f) {
        *s = wchar;
        return 1;
    }
    if (wchar <= 0x7ff) {
        *s++ = 0xc0 + (((unsigned int) wchar & 0x7c0) >> 6);
        *s++ = 0x80 + ((unsigned int) wchar & 0x3f);
        return 2;
    }
    if (wchar <= 0xffff) {
        /* Refuse to encode a value reserved for surrogate pairs in UTF-16. */
        if (wchar >= 0xd800 && wchar <= 0xdfff) {
            return -1;
        }
        *s++ = 0xe0 + (((unsigned int) wchar & 0xf000) >> 12);
        *s++ = 0x80 + (((unsigned int) wchar & 0xfc0) >> 6);
        *s++ = 0x80 + ((unsigned int) wchar & 0x3f);
        return 3;
    }
    if (wchar <= 0x10ffff) {
        *s++ = 0xf0 + (((unsigned int) wchar & 0x1c0000) >> 18);
        *s++ = 0x80 + (((unsigned int) wchar & 0x3f000) >> 12);
        *s++ = 0x80 + (((unsigned int) wchar & 0xfc0) >> 6);
        *s++ = 0x80 + ((unsigned int) wchar & 0x3f);
        return 4;
    }
    /* Refuse to encode something beyond the range encodeable by UTF-16. */
    return -1;
}

/**
 * Return whether a UTF-32 value is printable.
 *
 * This is a necessary counterpart to Term_mbcs_cocoa() so that screening
 * of wide characters in the core's text_out_to_screen() is consistent with
 * what Term_mbcs_cocoa() does.
 */
static int Term_iswprint_cocoa(wint_t wc)
{
	return utf32_isprint((u32b) wc);
}

/**
 * Return the maximum number of bytes needed for a multibyte encoding of a
 * wchar.
 */
static int Term_wcsz_cocoa(void)
{
    /* UTF-8 takes at most 4 bytes to encode a Unicode code point. */
    return 4;
}

/**
 * Handle redrawing for a change to the tile set, tile scaling, or main window
 * font.  Returns YES if the redrawing was initiated.  Otherwise returns NO.
 */
static BOOL redraw_for_tiles_or_term0_font(void)
{
    /*
     * do_cmd_redraw() will always clear, but only provides something
     * to replace the erased content if a character has been generated.
     * Therefore, only call it if a character has been generated.
     */
    if (character_generated) {
	do_cmd_redraw();
	wakeup_event_loop();
	return YES;
    }
    return NO;
}

/**
 * Post a nonsense event so that our event loop wakes up
 */
static void wakeup_event_loop(void)
{
    /* Big hack - send a nonsense event to make us update */
    NSEvent *event = [NSEvent otherEventWithType:NSApplicationDefined location:NSZeroPoint modifierFlags:0 timestamp:0 windowNumber:0 context:NULL subtype:AngbandEventWakeup data1:0 data2:0];
    [NSApp postEvent:event atStart:NO];
}


/**
 * Handle the "open_when_ready" flag
 */
static void handle_open_when_ready(void)
{
    /* Check the flag XXX XXX XXX make a function for this */
    if (open_when_ready && initialized && !game_in_progress)
    {
        /* Forget */
        open_when_ready = FALSE;
        
        /* Game is in progress */
        game_in_progress = TRUE;
        
        /* Wait for a keypress */
        pause_line(Term);
    }
}


/**
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
        /* Hack -- Forget messages and term */
        msg_flag = FALSE;
        Term->mapped_flag = FALSE;

        /* Save the game */
        record_current_savefile();
        close_game();

        /* Quit */
        quit(NULL);
    }

    /* Wait until inkey_flag is set */
}



/**
 * Returns YES if we contain an AngbandView (and hence should direct our events
 * to Angband)
 */
static BOOL contains_angband_view(NSView *view)
{
    if ([view isKindOfClass:[AngbandView class]]) return YES;
    for (NSView *subview in [view subviews]) {
        if (contains_angband_view(subview)) return YES;
    }
    return NO;
}


/**
 * Queue mouse presses if they occur in the map section of the main window.
 */
static void AngbandHandleEventMouseDown( NSEvent *event )
{
	AngbandContext *angbandContext = [[[event window] contentView] angbandContext];
	AngbandContext *mainAngbandContext =
	    (__bridge AngbandContext*) (angband_term[0]->data);

	if (mainAngbandContext.primaryWindow &&
	    [[event window] windowNumber] ==
	    [mainAngbandContext.primaryWindow windowNumber])
	{
		int cols, rows, x, y;
		Term_get_size(&cols, &rows);
		NSSize tileSize = angbandContext.tileSize;
		NSSize border = angbandContext.borderSize;
		NSPoint windowPoint = [event locationInWindow];

		/*
		 * Adjust for border; add border height because window origin
		 * is at bottom
		 */
		windowPoint = NSMakePoint( windowPoint.x - border.width, windowPoint.y + border.height );

		NSPoint p = [[[event window] contentView] convertPoint: windowPoint fromView: nil];
		x = floor( p.x / tileSize.width );
		y = floor( p.y / tileSize.height );

		/*
		 * Being safe about this, since xcode doesn't seem to like the
		 * bool_hack stuff
		 */
		BOOL displayingMapInterface = ((int)inkey_flag != 0);

		/* Sidebar plus border == thirteen characters; top row is reserved. */
		/* Coordinates run from (0,0) to (cols-1, rows-1). */
		BOOL mouseInMapSection = (x > 13 && x <= cols - 1 && y > 0  && y <= rows - 2);

		/*
		 * If we are displaying a menu, allow clicks anywhere within
		 * the terminal bounds; if we are displaying the main game
		 * interface, only allow clicks in the map section
		 */
		if ((!displayingMapInterface && x >= 0 && x < cols &&
		     y >= 0 && y < rows) ||
		     (displayingMapInterface && mouseInMapSection))
		{
			/*
			 * [event buttonNumber] will return 0 for left click,
			 * 1 for right click, but this is safer
			 */
			int button = ([event type] == NSLeftMouseDown) ? 1 : 2;

#ifdef KC_MOD_ALT
			NSUInteger eventModifiers = [event modifierFlags];
			byte angbandModifiers = 0;
			angbandModifiers |= (eventModifiers & NSShiftKeyMask) ? KC_MOD_SHIFT : 0;
			angbandModifiers |= (eventModifiers & NSControlKeyMask) ? KC_MOD_CONTROL : 0;
			angbandModifiers |= (eventModifiers & NSAlternateKeyMask) ? KC_MOD_ALT : 0;
			button |= (angbandModifiers & 0x0F) << 4; /* encode modifiers in the button number (see Term_mousepress()) */
#endif

			Term_mousepress(x, y, button);
		}
	}

	/* Pass click through to permit focus change, resize, etc. */
	[NSApp sendEvent:event];
}



/**
 * Encodes an NSEvent Angband-style, or forwards it along.  Returns YES if the
 * event was sent to Angband, NO if Cocoa (or nothing) handled it
 */
static BOOL send_event(NSEvent *event)
{

    /* If the receiving window is not an Angband window, then do nothing */
    if (! contains_angband_view([[event window] contentView]))
    {
        [NSApp sendEvent:event];
        return NO;
    }

    /* Analyze the event */
    switch ([event type])
    {
        case NSKeyDown:
        {
            /* Try performing a key equivalent */
            if ([[NSApp mainMenu] performKeyEquivalent:event]) break;
            
            unsigned modifiers = [event modifierFlags];
            
            /* Send all NSCommandKeyMasks through */
            if (modifiers & NSCommandKeyMask)
            {
                [NSApp sendEvent:event];
                break;
            }
            
            if (! [[event characters] length]) break;
            
            
            /* Extract some modifiers */
            int mc = !! (modifiers & NSControlKeyMask);
            int ms = !! (modifiers & NSShiftKeyMask);
            int mo = !! (modifiers & NSAlternateKeyMask);
            int mx = !! (modifiers & NSCommandKeyMask);
            int kp = !! (modifiers & NSNumericPadKeyMask);
            
            
            /* Get the Angband char corresponding to this unichar */
            unichar c = [[event characters] characterAtIndex:0];
            keycode_t ch;
            switch (c) {
                /*
                 * Note that NSNumericPadKeyMask is set if any of the arrow
                 * keys are pressed. We don't want KC_MOD_KEYPAD set for
                 * those. See #1662 for more details.
                 */
                case NSUpArrowFunctionKey: ch = ARROW_UP; kp = 0; break;
                case NSDownArrowFunctionKey: ch = ARROW_DOWN; kp = 0; break;
                case NSLeftArrowFunctionKey: ch = ARROW_LEFT; kp = 0; break;
                case NSRightArrowFunctionKey: ch = ARROW_RIGHT; kp = 0; break;
                case NSF1FunctionKey: ch = KC_F1; break;
                case NSF2FunctionKey: ch = KC_F2; break;
                case NSF3FunctionKey: ch = KC_F3; break;
                case NSF4FunctionKey: ch = KC_F4; break;
                case NSF5FunctionKey: ch = KC_F5; break;
                case NSF6FunctionKey: ch = KC_F6; break;
                case NSF7FunctionKey: ch = KC_F7; break;
                case NSF8FunctionKey: ch = KC_F8; break;
                case NSF9FunctionKey: ch = KC_F9; break;
                case NSF10FunctionKey: ch = KC_F10; break;
                case NSF11FunctionKey: ch = KC_F11; break;
                case NSF12FunctionKey: ch = KC_F12; break;
                case NSF13FunctionKey: ch = KC_F13; break;
                case NSF14FunctionKey: ch = KC_F14; break;
                case NSF15FunctionKey: ch = KC_F15; break;
                case NSHelpFunctionKey: ch = KC_HELP; break;
                case NSHomeFunctionKey: ch = KC_HOME; break;
                case NSPageUpFunctionKey: ch = KC_PGUP; break;
                case NSPageDownFunctionKey: ch = KC_PGDOWN; break;
                case NSBeginFunctionKey: ch = KC_BEGIN; break;
                case NSEndFunctionKey: ch = KC_END; break;
                case NSInsertFunctionKey: ch = KC_INSERT; break;
                case NSDeleteFunctionKey: ch = KC_DELETE; break;
                case NSPauseFunctionKey: ch = KC_PAUSE; break;
                case NSBreakFunctionKey: ch = KC_BREAK; break;
                    
                default:
                    if (c <= 0x7F) {
                        ch = (char)c;
                    } else if (c > 0x9f) {
                        /*
                         * It's beyond the range of the C1 control
                         * characters.  Pass it on.
                         */
                        if (CFStringIsSurrogateHighCharacter(c)) {
                            ch = CFStringGetLongCharacterForSurrogatePair(c,
                                [[event characters] characterAtIndex:1]);
                        } else {
                            ch = c;
                        }
                    } else {
                        /*
                         * Exclude the control characters not caught by the
                         * special cases.
                         */
                        ch = '\0';
                    }
                    break;
            }
            
            /* override special keys */
            switch([event keyCode]) {
                case kVK_Return: ch = KC_ENTER; break;
                case kVK_Escape: ch = ESCAPE; break;
                case kVK_Tab: ch = KC_TAB; break;
                case kVK_Delete: ch = KC_BACKSPACE; break;
                case kVK_ANSI_KeypadEnter: ch = KC_ENTER; kp = TRUE; break;
            }

            /* Hide the mouse pointer */
            [NSCursor setHiddenUntilMouseMoves:YES];
            
            /* Enqueue it */
            if (ch != '\0')
            {
                
                /* Enqueue the keypress */
#ifdef KC_MOD_ALT
                byte mods = 0;
                if (mo) mods |= KC_MOD_ALT;
                if (mx) mods |= KC_MOD_META;
                if (mc && MODS_INCLUDE_CONTROL(ch)) mods |= KC_MOD_CONTROL;
                if (ms && MODS_INCLUDE_SHIFT(ch)) mods |= KC_MOD_SHIFT;
                if (kp) mods |= KC_MOD_KEYPAD;
                Term_keypress(ch, mods);
#else
                Term_keypress(ch);
#endif
            }
            
            break;
        }
            
        case NSLeftMouseDown:
		case NSRightMouseDown:
			AngbandHandleEventMouseDown(event);
            break;

        case NSApplicationDefined:
        {
            if ([event subtype] == AngbandEventWakeup)
            {
                return YES;
            }
            break;
        }
            
        default:
            [NSApp sendEvent:event];
            return YES;
    }
    return YES;
}

/**
 * Check for Events, return TRUE if we process any
 */
static BOOL check_events(int wait)
{
    BOOL result = YES;

    @autoreleasepool {
	/* Handles the quit_when_ready flag */
	if (quit_when_ready) quit_calmly();

	NSDate* endDate;
	if (wait == CHECK_EVENTS_WAIT) endDate = [NSDate distantFuture];
	else endDate = [NSDate distantPast];

	NSEvent* event;
	for (;;) {
	    if (quit_when_ready)
	    {
		/* send escape events until we quit */
		Term_keypress(0x1B, 0);
		result = NO;
		break;
	    }
	    else {
		event = [NSApp nextEventMatchingMask:-1 untilDate:endDate
			       inMode:NSDefaultRunLoopMode dequeue:YES];

		if (! event) {
		    result = NO;
		    break;
		}
		if (send_event(event)) break;
	    }
	}
    }

    return result;
}

/**
 * Hook to tell the user something important
 */
static void hook_plog(const char * str)
{
    if (str)
    {
		NSLog( @"%s", str );
    }
}


/**
 * Hook to tell the user something, and then quit
 */
static void hook_quit(const char * str)
{
    plog(str);
    exit(0);
}

/**
 * Return the path for Angband's lib directory and bail if it isn't found. The
 * lib directory should be in the bundle's resources directory, since it's
 * copied when built.
 */
static NSString* get_lib_directory(void)
{
    NSString *bundleLibPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: AngbandDirectoryNameLib];
    BOOL isDirectory = NO;
    BOOL libExists = [[NSFileManager defaultManager] fileExistsAtPath: bundleLibPath isDirectory: &isDirectory];

    if( !libExists || !isDirectory )
    {
	NSLog( @"%@: can't find %@/ in bundle: isDirectory: %d libExists: %d", @VERSION_NAME, AngbandDirectoryNameLib, isDirectory, libExists );

	NSAlert *alert = [[NSAlert alloc] init];
	/*
	 * Note that NSCriticalAlertStyle was deprecated in 10.10.  The
	 * replacement is NSAlertStyleCritical.
	 */
	alert.alertStyle = NSCriticalAlertStyle;
	alert.messageText = @"MissingResources";
	alert.informativeText = [NSString stringWithFormat:@"%@ %@",
		@VERSION_NAME,
		@"was unable to find required resources and must quit. Please report a bug on the Angband forums."];
	[alert addButtonWithTitle:@"Quit"];
	[alert runModal];
	exit(0);
    }

    return bundleLibPath;
}

/**
 * Return the path for the directory where Angband should look for its standard
 * user file tree.
 */
static NSString* get_doc_directory(void)
{
	NSString *documents = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];

#if defined(SAFE_DIRECTORY)
	NSString *versionedDirectory = [NSString stringWithFormat: @"%@-%s", AngbandDirectoryNameBase, VERSION_STRING];
	return [documents stringByAppendingPathComponent: versionedDirectory];
#else
	return [documents stringByAppendingPathComponent: AngbandDirectoryNameBase];
#endif
}

/**
 * Adjust directory paths as needed to correct for any differences needed by
 * Angband.  init_file_paths() currently requires that all paths provided have
 * a trailing slash and all other platforms honor this.
 *
 * \param originalPath The directory path to adjust.
 * \return A path suitable for Angband or nil if an error occurred.
 */
static NSString* AngbandCorrectedDirectoryPath(NSString *originalPath)
{
	if ([originalPath length] == 0) {
		return nil;
	}

	if (![originalPath hasSuffix: @"/"]) {
		return [originalPath stringByAppendingString: @"/"];
	}

	return originalPath;
}

/**
 * Give Angband the base paths that should be used for the various directories
 * it needs. It will create any needed directories.
 */
static void prepare_paths_and_directories(void)
{
	char libpath[PATH_MAX + 1] = "\0";
	NSString *libDirectoryPath =
	    AngbandCorrectedDirectoryPath(get_lib_directory());
	[libDirectoryPath getFileSystemRepresentation: libpath maxLength: sizeof(libpath)];

	char basepath[PATH_MAX + 1] = "\0";
	NSString *angbandDocumentsPath =
	    AngbandCorrectedDirectoryPath(get_doc_directory());
	[angbandDocumentsPath getFileSystemRepresentation: basepath maxLength: sizeof(basepath)];

	init_file_paths(libpath, libpath, basepath);
	create_needed_dirs();
}

/**
 * Create and initialize Angband terminal number "i".
 */
static term *term_data_link(int i)
{
    NSArray *terminalDefaults = [[NSUserDefaults standardUserDefaults]
				    valueForKey: AngbandTerminalsDefaultsKey];
    NSInteger rows = 24;
    NSInteger columns = 80;

    if (i < (int)[terminalDefaults count]) {
        NSDictionary *term = [terminalDefaults objectAtIndex:i];
        rows = [[term valueForKey: AngbandTerminalRowsDefaultsKey]
		   integerValue];
        columns = [[term valueForKey: AngbandTerminalColumnsDefaultsKey]
		      integerValue];
    }

    /* Allocate */
    term *newterm = mem_zalloc(sizeof(term));

    /* Initialize the term */
    term_init(newterm, columns, rows, 256 /* keypresses, for some reason? */);

    /* Use a "software" cursor */
    newterm->soft_cursor = TRUE;

    /* Disable the per-row flush notifications since they are not used. */
    newterm->never_frosh = TRUE;

    /*
     * Differentiate between BS/^h, Tab/^i, ... so ^h and ^j work under the
     * roguelike command set.
     */
    newterm->complex_input = TRUE;

    /* Prepare the init/nuke hooks */
    newterm->init_hook = Term_init_cocoa;
    newterm->nuke_hook = Term_nuke_cocoa;

    /* Prepare the function hooks */
    newterm->xtra_hook = Term_xtra_cocoa;
    newterm->wipe_hook = Term_wipe_cocoa;
    newterm->curs_hook = Term_curs_cocoa;
    newterm->bigcurs_hook = Term_bigcurs_cocoa;
    newterm->text_hook = Term_text_cocoa;
    newterm->pict_hook = Term_pict_cocoa;

    /* Global pointer */
    angband_term[i] = newterm;

    return newterm;
}

/**
 * Load preferences from preferences file for current host+current user+
 * current application.
 */
static void load_prefs(void)
{
    NSUserDefaults *defs = [NSUserDefaults angbandDefaults];

    /* Make some default defaults */
    NSMutableArray *defaultTerms = [[NSMutableArray alloc] init];

    /*
     * The following default rows/cols were determined experimentally by first
     * finding the ideal window/font size combinations. But because of awful
     * temporal coupling in Term_init_cocoa(), it's impossible to set up the
     * defaults there, so we do it this way.
     */
    for (NSUInteger i = 0; i < ANGBAND_TERM_MAX; i++) {
	int columns, rows;
	BOOL visible = YES;

	switch (i) {
	case 0:
	    columns = 129;
	    rows = 32;
	    break;
	case 1:
	    columns = 84;
	    rows = 20;
	    break;
	case 2:
	    columns = 42;
	    rows = 24;
	    break;
	case 3:
	    columns = 42;
	    rows = 20;
	    break;
	case 4:
	    columns = 42;
	    rows = 16;
	    break;
	case 5:
	    columns = 84;
	    rows = 20;
	    break;
	default:
	    columns = 80;
	    rows = 24;
	    visible = NO;
	    break;
	}

	NSDictionary *standardTerm =
	    [NSDictionary dictionaryWithObjectsAndKeys:
			  [NSNumber numberWithInt: rows], AngbandTerminalRowsDefaultsKey,
			  [NSNumber numberWithInt: columns], AngbandTerminalColumnsDefaultsKey,
			  [NSNumber numberWithBool: visible], AngbandTerminalVisibleDefaultsKey,
			  nil];
        [defaultTerms addObject: standardTerm];
    }

    NSDictionary *defaults = [[NSDictionary alloc] initWithObjectsAndKeys:
                              @"Menlo", @"FontName-0",
                              [NSNumber numberWithFloat:13.f], @"FontSize-0",
                              [NSNumber numberWithInt:60], AngbandFrameRateDefaultsKey,
                              [NSNumber numberWithBool:YES], AngbandSoundDefaultsKey,
                              [NSNumber numberWithInt:GRAPHICS_NONE], AngbandGraphicsDefaultsKey,
                              [NSNumber numberWithBool:YES], AngbandUseDefaultTileMultDefaultsKey,
                              [NSNumber numberWithInt:1], AngbandTileWidthMultDefaultsKey,
                              [NSNumber numberWithInt:1], AngbandTileHeightMultDefaultsKey,
                              defaultTerms, AngbandTerminalsDefaultsKey,
                              nil];
    [defs registerDefaults:defaults];

    /* Preferred graphics mode */
    graf_mode_req = [defs integerForKey:AngbandGraphicsDefaultsKey];
    if (graphics_will_be_enabled()) {
        tile_width = [defs integerForKey:AngbandTileWidthMultDefaultsKey];
        tile_height = [defs integerForKey:AngbandTileHeightMultDefaultsKey];
    } else {
        tile_width = 1;
        tile_height = 1;
    }

    /* Use sounds */
    [AngbandSoundCatalog sharedSounds].enabled =
	[defs boolForKey:AngbandSoundDefaultsKey];

    /* fps */
    frames_per_second = [defs integerForKey:AngbandFrameRateDefaultsKey];

    /* Font */
    [AngbandContext
	setDefaultFont:[NSFont fontWithName:[defs valueForKey:@"FontName-0"]
			       size:[defs floatForKey:@"FontSize-0"]]];
    if (! [AngbandContext defaultFont])
	[AngbandContext
	    setDefaultFont:[NSFont fontWithName:@"Menlo" size:13.]];
}

/**
 * Play sound effects asynchronously.  Select a sound from any available
 * for the required event, and bridge to Cocoa to play it.
 */
static void play_sound(game_event_type unused, game_event_data *data, void *user)
{
    int event = data->message.type;
    [[AngbandSoundCatalog sharedSounds] playSound:event];
}

/**
 * Allocate the primary Angband terminal and activate it.  Allocate the other
 * Angband terminals.
 */
static void init_windows(void)
{
    /* Create the primary window */
    term *primary = term_data_link(0);

    /* Prepare to create any additional windows */
    for (int i = 1; i < ANGBAND_TERM_MAX; i++) {
        term_data_link(i);
    }

    /* Activate the primary term */
    Term_activate(primary);
}

/**
 * Set HFS file type and creator codes on a path
 */
static void cocoa_file_open_hook(const char *path, file_type ftype)
{
    @autoreleasepool {
	NSString *pathString = [NSString stringWithUTF8String:path];
	if (pathString)
	{
	    u32b mac_type = 'TEXT';
	    if (ftype == FTYPE_RAW)
		mac_type = 'DATA';
	    else if (ftype == FTYPE_SAVE)
		mac_type = 'SAVE';

	    NSDictionary *attrs =
		[NSDictionary dictionaryWithObjectsAndKeys:
			      [NSNumber numberWithUnsignedLong:mac_type],
			      NSFileHFSTypeCode,
			      [NSNumber numberWithUnsignedLong:ANGBAND_CREATOR],
			      NSFileHFSCreatorCode,
			      nil];
	    [[NSFileManager defaultManager]
		setAttributes:attrs ofItemAtPath:pathString error:NULL];
	}
    }
}

/**
 * A platform-native file save dialogue box, e.g. for saving character dumps
 */
static bool cocoa_get_file(const char *suggested_name, char *path, size_t len)
{
    bool result = FALSE;
    @autoreleasepool {
	NSSavePanel *panel = [NSSavePanel savePanel];
	NSURL *directoryURL = [NSURL URLWithString:[NSString stringWithCString:ANGBAND_DIR_USER encoding:NSASCIIStringEncoding]];
	[panel setDirectoryURL:directoryURL];
	[panel setNameFieldStringValue:[NSString stringWithCString:suggested_name encoding:NSASCIIStringEncoding]];

	if ([panel runModal] == NSOKButton) {
	    const char *p = [[[panel URL] path] UTF8String];
	    my_strcpy(path, p, len);
	    result = TRUE;
	}
    }

    return result;
}

/**
 * Perform (as ui-game.c's reinit_hook) platform-specific actions necessary
 * when restarting without exiting.  Also called directly at startup.
 */
static void cocoa_reinit(void)
{
    /* Register the sound hook */
    event_add_handler(EVENT_SOUND, play_sound, NULL);
}

/**
 * ------------------------------------------------------------------------
 * Main program
 * ------------------------------------------------------------------------ */

@implementation AngbandAppDelegate

@synthesize commandMenu=_commandMenu;
@synthesize commandMenuTagMap=_commandMenuTagMap;

- (IBAction)newGame:sender
{
    /* Game is in progress */
    game_in_progress = TRUE;
    new_game = TRUE;
}

- (IBAction)editFont:sender
{
    NSFontPanel *panel = [NSFontPanel sharedFontPanel];
    NSFont *termFont = [AngbandContext defaultFont];

    int i;
    for (i=0; i < ANGBAND_TERM_MAX; i++) {
	AngbandContext *context =
	    (__bridge AngbandContext*) (angband_term[i]->data);
        if ([context isMainWindow]) {
            termFont = [context angbandViewFont];
            break;
        }
    }

    [panel setPanelFont:termFont isMultiple:NO];
    [panel orderFront:self];
}

/**
 * Implement NSObject's changeFont() method to receive a notification about the
 * changed font.  Note that, as of 10.14, changeFont() is deprecated in
 * NSObject - it will be removed at some point and the application delegate
 * will have to be declared as implementing the NSFontChanging protocol.
 */
- (void)changeFont:(id)sender
{
    int mainTerm;
    for (mainTerm=0; mainTerm < ANGBAND_TERM_MAX; mainTerm++) {
	AngbandContext *context =
	    (__bridge AngbandContext*) (angband_term[mainTerm]->data);
        if ([context isMainWindow]) {
            break;
        }
    }

    /* Bug #1709: Only change font for angband windows */
    if (mainTerm == ANGBAND_TERM_MAX) return;

    NSFont *oldFont = [AngbandContext defaultFont];
    NSFont *newFont = [sender convertFont:oldFont];
    if (! newFont) return; /*paranoia */

    /* Store as the default font if we changed the first term */
    if (mainTerm == 0) {
	[AngbandContext setDefaultFont:newFont];
    }

    /* Record it in the preferences */
    NSUserDefaults *defs = [NSUserDefaults angbandDefaults];
    [defs setValue:[newFont fontName] 
        forKey:[NSString stringWithFormat:@"FontName-%d", mainTerm]];
    [defs setFloat:[newFont pointSize]
        forKey:[NSString stringWithFormat:@"FontSize-%d", mainTerm]];

    NSDisableScreenUpdates();

    /* Update window */
    AngbandContext *angbandContext =
	(__bridge AngbandContext*) (angband_term[mainTerm]->data);
    [(id)angbandContext setSelectionFont:newFont adjustTerminal: YES];

    NSEnableScreenUpdates();

    if (mainTerm == 0) {
	[self recomputeDefaultTileMultipliersIfNecessary];
    }

    if (mainTerm != 0 || ! redraw_for_tiles_or_term0_font()) {
	[(id)angbandContext requestRedraw];
    }
}

- (IBAction)openGame:sender
{
    @autoreleasepool {
	BOOL selectedSomething = NO;
	int panelResult;

	/* Get where we think the save files are */
	NSURL *startingDirectoryURL =
	    [NSURL fileURLWithPath:[NSString stringWithCString:ANGBAND_DIR_SAVE encoding:NSASCIIStringEncoding]
		   isDirectory:YES];

	/* Set up an open panel */
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	[panel setCanChooseFiles:YES];
	[panel setCanChooseDirectories:NO];
	[panel setResolvesAliases:YES];
	[panel setAllowsMultipleSelection:NO];
	[panel setTreatsFilePackagesAsDirectories:YES];
	[panel setDirectoryURL:startingDirectoryURL];

	/* Run it */
	panelResult = [panel runModal];
	if (panelResult == NSOKButton)
	{
	    NSArray* fileURLs = [panel URLs];
	    if ([fileURLs count] > 0)
	    {
		NSURL* savefileURL = (NSURL *)[fileURLs objectAtIndex:0];
		/*
		 * The path property doesn't do the right thing except for
		 * URLs with the file scheme. We had
		 * getFileSystemRepresentation here before, but that wasn't
		 * introduced until OS X 10.9.
		 */
		assert([[savefileURL scheme] isEqualToString:@"file"]);
		selectedSomething = [[savefileURL path]
					getCString:savefile
					maxLength:sizeof savefile
					encoding:NSMacOSRomanStringEncoding];
	    }
	}

	if (selectedSomething)
	{
	    /* Remember this so we can select it by default next time */
	    record_current_savefile();

	    /* Game is in progress */
	    game_in_progress = TRUE;
	}
    }
}

- (IBAction)saveGame:sender
{
    /* Hack -- Forget messages */
    msg_flag = FALSE;
    
    /* Save the game */
    save_game();
    
    /*
     * Record the current save file so we can select it by default next time.
     * It's a little sketchy that this only happens when we save through the
     * menu; ideally game-triggered saves would trigger it too.
     */
    record_current_savefile();
}

- (void)recomputeDefaultTileMultipliersIfNecessary
{
    NSInteger hscl, vscl;

    if (graphics_will_be_enabled()) {
	if ([[NSUserDefaults angbandDefaults]
		boolForKey:AngbandUseDefaultTileMultDefaultsKey]) {
	    [self computeDefaultTileSetScaling:&hscl vertical:&vscl];
	    [[NSUserDefaults angbandDefaults]
		setInteger:hscl forKey:AngbandTileWidthMultDefaultsKey];
	    [[NSUserDefaults angbandDefaults]
		setInteger:vscl forKey:AngbandTileHeightMultDefaultsKey];
	    if (self.scalingPanelController != nil) {
		self.scalingPanelController.horizontalScaling = hscl;
		self.scalingPanelController.verticalScaling = vscl;
		self.scalingPanelController.usesDefaultScaling = YES;
	    }
	} else {
	    hscl = [[NSUserDefaults angbandDefaults]
		       integerForKey:AngbandTileWidthMultDefaultsKey];
	    vscl = [[NSUserDefaults angbandDefaults]
		       integerForKey:AngbandTileHeightMultDefaultsKey];
	}
    } else {
	hscl = 1;
	vscl = 1;
    }
    if (tile_width != hscl || tile_height != vscl) {
	tile_width = hscl;
	tile_height = vscl;
	tile_multipliers_changed = 1;
    }
}

/**
 * Entry point for initializing Angband
 */
- (void)beginGame
{
    @autoreleasepool {
	/* Set the command hook */
	cmd_get_hook = textui_get_cmd;

	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;

	/* Hook in to the file_open routine */
	file_open_hook = cocoa_file_open_hook;

	/* Hook into file saving dialogue routine */
	get_file = cocoa_get_file;

	/* Initialize file paths */
	prepare_paths_and_directories();

	/* Note the "system" */
	ANGBAND_SYS = "mac";

	/* Load possible graphics modes */
	init_graphics_modes("graphics.txt");

	/* Load preferences */
	load_prefs();

	/* Prepare the windows */
	init_windows();
	text_mbcs_hook = Term_mbcs_cocoa;
	text_wctomb_hook = Term_wctomb_cocoa;
	text_wcsz_hook = Term_wcsz_cocoa;
	text_iswprint_hook = Term_iswprint_cocoa;

	/* Set up game event handlers */
	init_display();

	/* Initialise game */
	init_angband();
	textui_init();

	/*
	 * Set action that needs to be done if restarting without exiting.
	 * Also need to do it now.
	 */
	reinit_hook = cocoa_reinit;
	cocoa_reinit();

	/* Initialize some save file stuff */
	player_egid = getegid();

	/* We are now initialized */
	initialized = TRUE;

	/* Handle "open_when_ready" */
	handle_open_when_ready();

	/* Handle pending events (most notably update) and flush input */
	Term_flush();

	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]",
	    (Term->hgt - 23) / 5 + 23, (Term->wid - 45) / 2);
	Term_fresh();
    }

    while (!game_in_progress) {
	@autoreleasepool {
	    NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
	    if (event) [NSApp sendEvent:event];
	}
    }

    /*
     * Play a game -- "new_game" is set by "new", "open" or the open document
     * even handler as appropriate
     */
    Term_fresh();
    play_game(new_game);

    /* Free resources */
    textui_cleanup();
    cleanup_angband();

    quit(NULL);
}

/**
 * Implement NSObject's validateMenuItem() method to override enabling or
 * disabling a menu item.  Note that, as of 10.14, validateMenuItem() is
 * deprecated in NSObject - it will be removed at some point and the
 * application delegate will have to be declared as implementing the
 * NSMenuItemValidation protocol.
 */
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    SEL sel = [menuItem action];
    NSInteger tag = [menuItem tag];

    if( tag >= AngbandWindowMenuItemTagBase && tag < AngbandWindowMenuItemTagBase + ANGBAND_TERM_MAX )
    {
        if( tag == AngbandWindowMenuItemTagBase )
        {
            /* The main window should always be available and visible */
            return YES;
        }
        else
        {
            NSInteger subwindowNumber = tag - AngbandWindowMenuItemTagBase;
            return (window_flag[subwindowNumber] > 0);
        }

        return NO;
    }

    if (sel == @selector(newGame:))
    {
        return ! game_in_progress;
    }
    else if (sel == @selector(editFont:))
    {
        return YES;
    }
    else if (sel == @selector(openGame:))
    {
        return ! game_in_progress;
    }
    else if (sel == @selector(setRefreshRate:) &&
	     [[menuItem parentItem] tag] == 150)
    {
        NSInteger fps = [[NSUserDefaults standardUserDefaults] integerForKey:AngbandFrameRateDefaultsKey];
        [menuItem setState: ([menuItem tag] == fps)];
        return YES;
    }
    else if( sel == @selector(setGraphicsMode:) )
    {
        NSInteger requestedGraphicsMode = [[NSUserDefaults standardUserDefaults] integerForKey:AngbandGraphicsDefaultsKey];
        [menuItem setState: (tag == requestedGraphicsMode)];
        return YES;
    }
    else if( sel == @selector(sendAngbandCommand:) ||
	     sel == @selector(saveGame:) )
    {
        /*
         * we only want to be able to send commands during an active game
         * after the birth screens
         */
        return !!game_in_progress && character_generated;
    }
    else return YES;
}


- (IBAction)setRefreshRate:(NSMenuItem *)menuItem
{
    frames_per_second = [menuItem tag];
    [[NSUserDefaults angbandDefaults] setInteger:frames_per_second forKey:AngbandFrameRateDefaultsKey];
}

- (IBAction)showTileSetScalingPanel:(id)sender
{
    if (self.scalingPanelController == nil) {
	self.scalingPanelController =
	    [[TileSetScalingPanelController alloc] initWithWindow:nil];

	self.scalingPanelController.defaultScalingComputer = self;
	self.scalingPanelController.scalingChangeHandler = self;
    }
    self.scalingPanelController.horizontalScaling = tile_width;
    self.scalingPanelController.verticalScaling = tile_height;
    self.scalingPanelController.usesDefaultScaling =
	[[NSUserDefaults angbandDefaults]
	    boolForKey:AngbandUseDefaultTileMultDefaultsKey];
    [self.scalingPanelController showWindow:sender];
}

- (void)setGraphicsMode:(NSMenuItem *)sender
{
    /* We stashed the graphics mode ID in the menu item's tag */
    graf_mode_req = [sender tag];

    /* Stash it in UserDefaults */
    [[NSUserDefaults angbandDefaults] setInteger:graf_mode_req forKey:AngbandGraphicsDefaultsKey];
    [self recomputeDefaultTileMultipliersIfNecessary];

    redraw_for_tiles_or_term0_font();
}

- (void)selectWindow: (id)sender
{
    NSInteger subwindowNumber =
	[(NSMenuItem *)sender tag] - AngbandWindowMenuItemTagBase;
    AngbandContext *context =
	(__bridge AngbandContext*) (angband_term[subwindowNumber]->data);
    [context.primaryWindow makeKeyAndOrderFront: self];
    [context saveWindowVisibleToDefaults: YES];
}

/**
 * Implement the TileSetDefaultScalingComputing protocol.
 *
 * Assume that angband_term[0] and angbandDefaults are ready to use.
 */
- (void)computeDefaultTileSetScaling:(NSInteger *)pHoriz vertical:(NSInteger *)pVert
{
    if (graf_mode_req != GRAPHICS_NONE) {
	graphics_mode *new_mode = get_graphics_mode(graf_mode_req);

	if (new_mode->grafID != GRAPHICS_NONE) {
	    int tilew = new_mode->cell_width;
	    int tileh = new_mode->cell_height;
	    AngbandContext *term0_context =
		(__bridge AngbandContext*) (angband_term[0]->data);
	    CGFloat textw = term0_context.tileSize.width;
	    CGFloat texth = term0_context.tileSize.height;
	    CGFloat wratio = tilew / textw;
	    CGFloat hratio = tileh / texth;
	    CGFloat extratio = (wratio < hratio) ? wratio : hratio;

	    /*
	     * If a tile is enough smaller in either dimension relative to
	     * a cell, use a scaled up tile as the point of comparison.
	     */
	    if (extratio < 2.0 / 3.0) {
		CGFloat scl = floor(1.0 / extratio + 0.5);

		wratio *= scl;
		hratio *= scl;
	    }
	    /*
	     * Cap the scale factors and try to approximately keep the tile's
	     * aspect ratio if capped.
	     */
	    extratio = (wratio > hratio) ? wratio : hratio;
	    if (extratio > TileSetScalingPanelController.scalingMaximum) {
		CGFloat scl = TileSetScalingPanelController.scalingMaximum /
		    extratio;

		wratio *= scl;
		hratio *= scl;
	    }
	    /* Don't try to scale if can't avoid a scale factor of zero. */
	    extratio = (wratio < hratio) ? wratio : hratio;
	    if (extratio >= 0.5) {
		*pHoriz = (NSInteger) floor(wratio + 0.5);
		*pVert = (NSInteger) floor(hratio + 0.5);
	    } else {
		*pHoriz = 1;
		*pVert = 1;
	    }
	    return;
	}
    }

    /* Not using a tile set so use what is stored in the defaults. */
    *pHoriz = [[NSUserDefaults angbandDefaults]
		  integerForKey:AngbandTileWidthMultDefaultsKey];
    *pVert = [[NSUserDefaults angbandDefaults]
		 integerForKey:AngbandTileHeightMultDefaultsKey];
}

/**
 * Implement the TileSetScalingChanging protocol.
 */
- (void)changeTileSetScaling:(NSInteger)h vertical:(NSInteger)v isDefault:(BOOL)flag
{
    /* Update the defaults. */
    [[NSUserDefaults angbandDefaults]
	setBool:flag forKey:AngbandUseDefaultTileMultDefaultsKey];
    [[NSUserDefaults angbandDefaults]
	setInteger:h forKey:AngbandTileWidthMultDefaultsKey];
    [[NSUserDefaults angbandDefaults]
	setInteger:v forKey:AngbandTileHeightMultDefaultsKey];
    if (graphics_are_enabled() ||
	(! character_generated && graphics_will_be_enabled())) {
	if (tile_width != h || tile_height != v) {
	    tile_width = h;
	    tile_height = v;
	    tile_multipliers_changed = 1;
	    redraw_for_tiles_or_term0_font();
	}
    }
}

- (void)prepareWindowsMenu
{
    @autoreleasepool {
	/*
	 * Get the window menu with default items and add a separator and
	 * item for the main window.
	 */
	NSMenu *windowsMenu = [[NSApplication sharedApplication] windowsMenu];
	[windowsMenu addItem: [NSMenuItem separatorItem]];

	NSMenuItem *angbandItem = [[NSMenuItem alloc] initWithTitle: @VERSION_NAME action: @selector(selectWindow:) keyEquivalent: @"0"];
	[angbandItem setTarget: self];
	[angbandItem setTag: AngbandWindowMenuItemTagBase];
	[windowsMenu addItem: angbandItem];

	/* Add items for the additional term windows */
	for( NSInteger i = 1; i < ANGBAND_TERM_MAX; i++ )
	{
	    NSString *title =
		[NSString stringWithFormat: @"Term %ld", (long)i];
	    NSString *keyEquivalent =
		[NSString stringWithFormat: @"%ld", (long)i];
	    NSMenuItem *windowItem =
		[[NSMenuItem alloc] initWithTitle: title
				    action: @selector(selectWindow:)
				    keyEquivalent: keyEquivalent];
	    [windowItem setTarget: self];
	    [windowItem setTag: AngbandWindowMenuItemTagBase + i];
	    [windowsMenu addItem: windowItem];
	}
    }
}

/**
 * Send a command to Angband via a menu item. This places the appropriate key
 * down events into the queue so that it seems like the user pressed them
 * (instead of trying to use the term directly).
 */
- (void)sendAngbandCommand: (id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    NSString *command = [self.commandMenuTagMap objectForKey: [NSNumber numberWithInteger: [menuItem tag]]];
    AngbandContext* context =
	(__bridge AngbandContext*) (angband_term[0]->data);
    NSInteger windowNumber = [context.primaryWindow windowNumber];

    /* Send a \ to bypass keymaps */
    NSEvent *escape = [NSEvent keyEventWithType: NSKeyDown
                                       location: NSZeroPoint
                                  modifierFlags: 0
                                      timestamp: 0.0
                                   windowNumber: windowNumber
                                        context: nil
                                     characters: @"\\"
                    charactersIgnoringModifiers: @"\\"
                                      isARepeat: NO
                                        keyCode: 0];
    [[NSApplication sharedApplication] postEvent: escape atStart: NO];

    /* Send the actual command (from the original command set) */
    NSEvent *keyDown = [NSEvent keyEventWithType: NSKeyDown
                                        location: NSZeroPoint
                                   modifierFlags: 0
                                       timestamp: 0.0
                                    windowNumber: windowNumber
                                         context: nil
                                      characters: command
                     charactersIgnoringModifiers: command
                                       isARepeat: NO
                                         keyCode: 0];
    [[NSApplication sharedApplication] postEvent: keyDown atStart: NO];
}

/**
 * Set up the command menu dynamically, based on CommandMenu.plist.
 */
- (void)prepareCommandMenu
{
    @autoreleasepool {
	NSString *commandMenuPath =
	    [[NSBundle mainBundle] pathForResource: @"CommandMenu"
				   ofType: @"plist"];
	NSArray *commandMenuItems =
	    [[NSArray alloc] initWithContentsOfFile: commandMenuPath];
	NSMutableDictionary *angbandCommands =
	    [[NSMutableDictionary alloc] init];
	NSInteger tagOffset = 0;

	for( NSDictionary *item in commandMenuItems )
	{
	    BOOL useShiftModifier =
		[[item valueForKey: @"ShiftModifier"] boolValue];
	    BOOL useOptionModifier =
		[[item valueForKey: @"OptionModifier"] boolValue];
	    NSUInteger keyModifiers = NSCommandKeyMask;
	    keyModifiers |= (useShiftModifier) ? NSShiftKeyMask : 0;
	    keyModifiers |= (useOptionModifier) ? NSAlternateKeyMask : 0;

	    NSString *title = [item valueForKey: @"Title"];
	    NSString *key = [item valueForKey: @"KeyEquivalent"];
	    NSMenuItem *menuItem =
		[[NSMenuItem alloc] initWithTitle: title
				    action: @selector(sendAngbandCommand:)
				    keyEquivalent: key];
	    [menuItem setTarget: self];
	    [menuItem setKeyEquivalentModifierMask: keyModifiers];
	    [menuItem setTag: AngbandCommandMenuItemTagBase + tagOffset];
	    [self.commandMenu addItem: menuItem];

	    NSString *angbandCommand = [item valueForKey: @"AngbandCommand"];
	    [angbandCommands setObject: angbandCommand
			     forKey: [NSNumber numberWithInteger: [menuItem tag]]];
	    tagOffset++;
	}

	self.commandMenuTagMap = [[NSDictionary alloc]
				     initWithDictionary: angbandCommands];
    }
}

- (void)awakeFromNib
{
    [super awakeFromNib];

    [self prepareWindowsMenu];
    [self prepareCommandMenu];
}

- (void)applicationDidFinishLaunching:sender
{
    /* Setup timer to refresh animations. */
    NSTimer *animation_timer = [NSTimer scheduledTimerWithTimeInterval:0.2
        repeats:YES block:^(NSTimer *timer) { idle_update(); }];

    /*
     * Requires 10.9. Apple's documentation suggests that the tolerance be
     * at least 10% of the interval for a repeated timer.
     */
    animation_timer.tolerance = 0.02;

    [self beginGame];
    
    /* Remove the animation timer. */
    [animation_timer invalidate];

    /*
     * Once beginGame finished, the game is over - that's how Angband works,
     * and we should quit
     */
    game_is_finished = TRUE;
    [NSApp terminate:self];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    if (player->upkeep->playing == FALSE || game_is_finished == TRUE)
    {
        quit_when_ready = true;
        return NSTerminateNow;
    }
    else if (! inkey_flag)
    {
        /* For compatibility with other ports, do not quit in this case */
        return NSTerminateCancel;
    }
    else
    {
        /* Stop playing */
        player->upkeep->playing = FALSE;

        /*
         * Post an escape event so that we can return from our get-key-event
         * function
         */
        wakeup_event_loop();
        quit_when_ready = true;
        /*
         * Must return Cancel, not Later, because we need to get out of the
         * run loop and back to Angband's loop
         */
        return NSTerminateCancel;
    }
}

/**
 * Dynamically build the Graphics menu
 */
- (void)menuNeedsUpdate:(NSMenu *)menu {
    
    /* Only the graphics menu is dynamic */
    if (! [[menu title] isEqualToString:@"Graphics"])
        return;
    
    /*
     * If it's non-empty, then we've already built it. Currently graphics modes
     * won't change once created; if they ever can we can remove this check.
     * Note that the check mark does change, but that's handled in
     * validateMenuItem: instead of menuNeedsUpdate:
     */
    if ([menu numberOfItems] > 0)
        return;
    
    /* This is the action for all these menu items */
    SEL action = @selector(setGraphicsMode:);
    
    /* Add an initial Classic ASCII menu item */
    NSMenuItem *classicItem = [menu addItemWithTitle:@"Classic ASCII" action:action keyEquivalent:@""];
    [classicItem setTag:GRAPHICS_NONE];
    
    /* Walk through the list of graphics modes */
    NSInteger i;
    for (i=0; graphics_modes[i].pNext; i++)
    {
        const graphics_mode *graf = &graphics_modes[i];
        
        /*
         * Make the title. NSMenuItem throws on a nil title, so ensure it's
         * not nil.
         */
        NSString *title = [[NSString alloc] initWithUTF8String:graf->menuname];
        if (! title) title = [@"(Unknown)" copy];
        
        /* Make the item */
        NSMenuItem *item = [menu addItemWithTitle:title action:action keyEquivalent:@""];
        [item setTag:graf->grafID];
    }
}

/**
 * Delegate method that gets called if we're asked to open a file.
 */
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
    /* Can't open a file once we've started */
    if (game_in_progress) {
	[[NSApplication sharedApplication]
	    replyToOpenOrPrint:NSApplicationDelegateReplyFailure];
	return;
    }

    /* We can only open one file. Use the last one. */
    NSString *file = [filenames lastObject];
    if (! file) {
	[[NSApplication sharedApplication]
	    replyToOpenOrPrint:NSApplicationDelegateReplyFailure];
	return;
    }

    /* Put it in savefile */
    if (! [file getFileSystemRepresentation:savefile maxLength:sizeof savefile]) {
	[[NSApplication sharedApplication]
	    replyToOpenOrPrint:NSApplicationDelegateReplyFailure];
	return;
    }

    game_in_progress = TRUE;

    /*
     * Wake us up in case this arrives while we're sitting at the Welcome
     * screen!
     */
    wakeup_event_loop();

    [[NSApplication sharedApplication]
	replyToOpenOrPrint:NSApplicationDelegateReplySuccess];
}

@end

#if !XCODE
int main(int argc, char* argv[])
{
    NSApplicationMain(argc, (void*)argv);    
    return (0);
}
#endif

#endif /* MACINTOSH || MACH_O_CARBON */
