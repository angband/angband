#ifndef INCLUDED_OSX_TABLES_H
#define INCLUDED_OSX_TABLES_H

/*
 * Maximum menu ID.
 * IMPORTANT: see note in main-crb.c if you wish to add menus.
 */
#define MAX_MENU_ID (150)

/* These numbers must agree with the corresponding Menu ID in the nib. */
enum MenuID {
	kAngbandMenu    = 100,
	kFileMenu       = 101,
	kOpenRecentMenu = 102,

	kFontMenu       = 103,
	kTileWidMenu	= 104,
	kTileHgtMenu	= 105,

	kGraphicsMenu   = 106,
	kBigtileWidthMenu = 107,
	kBigtileHeightMenu = 108,

	kWindowMenu     = 109,

	kNiceGraphicsMenu = 110
};

// File Menu
enum {
	kNew			= 1,	/* N, 'new' */
	kOpen			= 2,	/* O, 'open' */
	kOpenRecent		= 3,
	kImport			= 4,	/* I, 'impo' */
	/* \-p */
	kSave 			= 6,	/* S, 'save' */
	kClose			= 7,	/* W, 'clos' */
};

// Window menu
enum {
	kMinimize			= 1, /* Not used */
	kMinimizeAll		= 2, /* Not used */
	kAngbandTerm		= 4,
	kTerminal1			= 5,  /* Terminal ids are relative to Terminal 1 */
	/* ... */
	kBringToFront 		= 13
};

// Font menu
enum {
	kFonts				= 1,
	kAntialias			= 2,
	kTileWidth			= 3,
	kTileHeight			= 4
};

// Graphics menu
enum {
	kGrafNone = 1,
	kGraf8x8 = 2,
	kGraf16x16 = 3,
	kGraf32x32 = 4,
	kGraf8x16 = 5,
	kBigTile = 6
};


enum {
	// Event target are windows, not a menu.
	kWINDOW = -1
};


/* References to HIViews in the dialog window */
static const HIViewID aboutDialogIcon = { 'DLOG', 1 };
static const HIViewID aboutDialogName = { 'DLOG', 2 };
static const HIViewID aboutDialogCopyright = { 'DLOG', 4 };


/* Specifications for graphics modes.  */
/* graf_mode variable is index of current mode */
static const struct {
	int menuItem;		// Index in Graphics Menu
 	  const char *file;			// Base name of png file (if any)
	const char *name;			// Value of ANGBAND_GRAF variable
	int size;			// Tile size (in pixels)
	bool trans;			// Use transparent foreground tiles
} graphics_modes[] = {
	{ kGrafNone, NULL, NULL, 0, false },
	{ kGraf8x8, "8x8", "old",	8,	true },
	{ kGraf16x16, "16x16",	"new",	16,	true },
	{ kGraf32x32, "32x32",	"david", 32,	true },
	{ kGraf8x16, "8x16",	"nomad", 16,	true },
};


/* Event handler specification */
struct CommandDef {
	int	evtClass; // Eventspec class - char-style constant eg 'quit'
	int	evtType;  // Eventspec type - enumeration
	EventHandlerUPP	handler;
	int	targetID; // Menu target (0 if no target)
	void	*userData; // Event user data (Not used currently)
}; 
typedef struct CommandDef CommandDef;


#define HANDLERDEF(func) \
		static OSStatus func(EventHandlerCallRef inHandlerCallRef, \
							 EventRef inEvent, \
						     void * inUserData )

HANDLERDEF(CloseCommand);
HANDLERDEF(QuitCommand);
HANDLERDEF(TileSizeCommand);
HANDLERDEF(FontCommand);
HANDLERDEF(RestoreCommand);
HANDLERDEF(ToggleCommand);
HANDLERDEF(TerminalCommand);
HANDLERDEF(KeyboardCommand);
HANDLERDEF(MouseCommand);
HANDLERDEF(ResizeCommand);
HANDLERDEF(UpdateCommand);
HANDLERDEF(AboutCommand);
HANDLERDEF(ValidateMenuCommand);
HANDLERDEF(OpenRecentCommand);
HANDLERDEF(ResumeCommand);
HANDLERDEF(CommandCommand);
HANDLERDEF(AngbandGame);




/* WARNING: This list must be grouped by (func, userData) pairs */

const CommandDef event_defs [] =
{

	/*
	 * Start game event - posted into the event queue after
	 * any potential open game events from the Finder
	 */
	{ 'Play', 'Band', AngbandGame, 0, NULL },

	/* Quit the game */
	{ 'appl', kEventAppQuit, QuitCommand, 0, NULL },
	
	/* Reactivate the game after it's been in the background */
	{ 'appl', kEventAppActivated, ResumeCommand, 0, NULL },


	/* "About Angband" command */
	{ 'cmds', kEventProcessCommand, AboutCommand, kAngbandMenu, NULL },

	/* Execute "boring" commands - "Save", "Open", and "Show Fonts" */
	{ 'cmds', kEventProcessCommand, CommandCommand, 0, NULL },
	
	/* Menu item within the "Open Recent" submenu */
	{ 'cmds', kEventProcessCommand, OpenRecentCommand, kOpenRecentMenu, NULL },
	
	/* Selection of a terminal within the Window menu */
	{ 'cmds', kEventProcessCommand, TerminalCommand, kWindowMenu, NULL },
	
	/* Activating a menu option */
	{ 'cmds', kEventProcessCommand, ToggleCommand, kFontMenu, NULL },
	{ 'cmds', kEventProcessCommand, ToggleCommand, kBigtileWidthMenu, NULL },
	{ 'cmds', kEventProcessCommand, ToggleCommand, kBigtileHeightMenu, NULL },
	{ 'cmds', kEventProcessCommand, ToggleCommand, kGraphicsMenu, NULL },

	/* Alter tile width and height */
	{ 'cmds', kEventProcessCommand, TileSizeCommand, kTileWidMenu, NULL },
	{ 'cmds', kEventProcessCommand, TileSizeCommand, kTileHgtMenu, NULL },

	/* Font panel - selection of a new font */
	{ 'font', kEventFontSelection, FontCommand, 0, NULL },
	
	/* Font panel closed */
	{ 'font', kEventFontPanelClosed, FontCommand, 0, NULL },
	
	/* Application window focus changes (update font focus status) */
	{ 'appl', kEventAppActiveWindowChanged, FontCommand, 0, NULL },



	/* Update seldom-changed menu item statuses on each menu open */
	{ 'menu', kEventMenuEnableItems, ValidateMenuCommand, 0, NULL },



	/* Keyboard keydown event */
	{ 'keyb', kEventRawKeyDown, KeyboardCommand, 0, NULL },
	
	/* Keyboard key repeat event */
	{ 'keyb', kEventRawKeyRepeat, KeyboardCommand, 0, NULL },
	
	/* Mouse click in a term */
	{ 'wind', kEventWindowHandleContentClick, MouseCommand, kWINDOW, NULL },



	/* Close a term */
	{ 'wind', kEventWindowClose, CloseCommand, kWINDOW, NULL },
	
	/* Activate a term (term brought to front) */
	{ 'wind', kEventWindowActivated, RestoreCommand, kWINDOW, NULL },
	
	/* A term requires redrawing */
	{ 'wind', kEventWindowUpdate, UpdateCommand, kWINDOW, NULL },

	/* Resize and move of a term, update term positions and sizes */
	{ 'wind', kEventWindowResizeCompleted, ResizeCommand, kWINDOW, NULL },
	{ 'wind', kEventWindowDragCompleted, ResizeCommand, kWINDOW, NULL },
};



/*
 * Construct a list of events that should be flushed in order to flush input
 */
static EventTypeSpec input_event_types[] = {
	{ 'keyb', kEventRawKeyDown },
	{ 'keyb', kEventRawKeyRepeat },
	{ 'wind', kEventWindowHandleContentClick },
};

#define CMDHANDLER(x) \
	static void x(HICommand *command, void *data);

CMDHANDLER(toggle_antialias);
CMDHANDLER(reset_wid_hgt);
CMDHANDLER(set_graphics_mode);
CMDHANDLER(set_tile_width);
CMDHANDLER(set_tile_height);
CMDHANDLER(set_nice_graphics_fit);
CMDHANDLER(set_nice_graphics_square);

static struct {
	UInt32 id;				// command id
	void (*handler)(HICommand *command, void *data);	// handler
	void *data;				// data
	bool refresh; 			// Change requires graphics refresh of main window.
} menu_commands[] = {
	{ 'anti', toggle_antialias, NULL, true },
	{ 'rewh', reset_wid_hgt, NULL, true },
	{ 'graf', set_graphics_mode, NULL, true },
	{ 'twid', set_tile_width, NULL, true },
	{ 'thgt', set_tile_height, NULL, true },
	{ 'ngfi', set_nice_graphics_fit, NULL, true },
	{ 'ngsq', set_nice_graphics_square, NULL, true },
};

#endif /* !INCLUDED_OSX_TABLES_H */
