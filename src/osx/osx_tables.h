/*
 * Maximum menu ID.
 * IMPORTANT: see note in main-crb.c if you wish to add menus.
 */
#define MAX_MENU_ID (150)

/* These numbers must agree with the corresponding Menu ID in the nib. */
enum MenuID {
	kAngbandMenu	= 100,
	kFileMenu		= 101,
	kEditMenu		= 102, 
	kStyleMenu		= 103,
	/* deleted */
	kWindowMenu		= 105,
	kSpecialMenu	= 106,

	kTileWidMenu	= 107,
	kTileHgtMenu	= 108
};

// Edit menu
enum {
	kCopy			= 1,	/* C, 'copy' */
	kSelectAll		= 2,	/* A, 'sall' */
	kUndo			= 3		/* Z, 'undo' */
};

// File Menu
enum {
	kNew			= 1,	/* N, 'new' */
	kOpen			= 2,	/* O, 'open' */
	kImport			= 3,	/* I, 'impo' */
	/* \-p */
	kSave 			= 5,	/* S, 'save' */
	kClose			= 6,	/* W, 'clos' */
	/* \-p 
	 setup
	  print 
	 \-p */ 
};


// Window menu
enum {
	kZoomWindow 		= 1, /* Not used */
	kMinimize			= 2, /* Not used */
	kMinimizeAll		= 3, /* Not used */
	kAngbandTerm		= 5,
	kTerminal1			= 6,  /* Terminal ids are relative to Terminal 1 */
	/* ... */
	kBringToFront 		= 14
};

// Special Menu
enum {
	kSound	 			= 1, /* Toggle sound */
							 /* \-p */
	kWizard				= 3, /* Toggle wizard mode */
	kFiddle				= 4  /* Don't know what this is. */
};



// Styles menu
enum {
	kFonts				= 1,
	kAntialias			= 2,
	kGrafNone			= 4,
	kGraf8x8			= 5,
	kGraf16x16			= 6,
	kGraf32x32			= 7,
	kGraf54x54			= 8,
	kInterpolate		= 10,
	kBigTile			= 11,
	kTileWidth			= 12,
	kTileHeight			= 13,
};


enum {
	// Event target are windows, not a menu.
	kWINDOW = -1
};

void fsetfileinfo(cptr pathname, u32b fcreator, u32b ftype);

extern u32b _ftype;
extern u32b _fcreator;

/* Opcodes for various events (OS X specific) */


/* Specifications for graphics modes.  */
/* graf_mode variable is index of current mode */

static const struct {
	int menuItem;		// Index in Graphics Menu
 	cptr file;			// Base name of png file (if any)
	cptr name;			// Value of ANGBAND_GRAF variable
	int size;			// Tile size (in pixels)
	bool trans;			// Use transparent foreground tiles
} graphics_modes [] = {
	{ kGrafNone,	NULL, 		NULL,		0,			false },
	{ kGraf8x8,		"8x8",		"old",		8,			false },
	{ kGraf16x16,	"16x16",	"new",		16,			true },
	{ kGraf32x32,	"32x32",	"david",	32,			true },
	{ kGraf54x54,	"54x54",	"davidiso",	54,			true },
};


/* Event handler specification */
struct CommandDef {
	int				evtClass; // Eventspec class - char-style constant eg 'quit'
	int				evtType;  // Eventspec type - enumeration
	EventHandlerUPP	handler;
	UInt32			targetID; // Menu target (0 if no target)
	void		  * userData; // Event user data (Not used currently)
}; 
typedef struct CommandDef CommandDef;

#define HICOM kEventProcessCommand
#define KYDWN kEventRawKeyDown
#define KYRPT kEventRawKeyRepeat
#define CLICK kEventWindowHandleContentClick
#define WNUPD kEventWindowUpdate
#define WNRSZ kEventWindowResizeCompleted

#define HANDLERDEF(func) \
		static OSStatus func(EventHandlerCallRef inHandlerCallRef, \
							 EventRef inEvent, \
						     void * inUserData )

HANDLERDEF(CloseCommand);				HANDLERDEF(PrintCommand);
HANDLERDEF(QuitCommand);				HANDLERDEF(TileSizeCommand);
HANDLERDEF(FontCommand);				HANDLERDEF(RestoreCommand);
HANDLERDEF(ToggleCommand);				HANDLERDEF(TerminalCommand);
HANDLERDEF(GraphicsCommand);			HANDLERDEF(KeyboardCommand);
HANDLERDEF(MouseCommand);				HANDLERDEF(ResizeCommand);
HANDLERDEF(UpdateCommand);				HANDLERDEF(AboutCommand);
HANDLERDEF(ValidateMenuCommand);		HANDLERDEF(ResumeCommand);
HANDLERDEF(CommandCommand);				HANDLERDEF(AngbandGame);




/* WARNING: This list must be grouped by (func, userData) pairs */

const CommandDef event_defs [] =
{
	{ 'font', kEventFontSelection,
						FontCommand,	 0, NULL }, // Change Font
	{ 'font', kEventFontPanelClosed,
						FontCommand,	 0, NULL }, // Menu state change
	{ 'appl',  kEventAppActiveWindowChanged,
						FontCommand,	 0, NULL }, // Store "true" focus

	{ 'cmds', HICOM,	AboutCommand,	 kAngbandMenu, NULL},
	{ 'cmds', HICOM,	PrintCommand,	 kEditMenu, "Cheaters never win!"},
	{ 'cmds', HICOM,	TerminalCommand, kWindowMenu, NULL }, // Open a window
	{ 'cmds', HICOM,	ToggleCommand,	 kSpecialMenu, NULL}, // Toggle a bool
	{ 'cmds', HICOM,	ToggleCommand,	 kStyleMenu, NULL},
	{ 'cmds', HICOM,	TileSizeCommand, kTileWidMenu, NULL}, // Change tile wid
	{ 'cmds', HICOM,	TileSizeCommand, kTileHgtMenu, NULL}, // Change tile hgt
	{ 'cmds', HICOM,	GraphicsCommand, kStyleMenu, NULL}, // New Graf-mode
	{ 'cmds', HICOM,	CommandCommand,	 0,		NULL }, // Exec boring commands
	{ 'appl',  kEventAppQuit,
						QuitCommand,	 0,		NULL }, // Exit the game

	{ 'keyb', KYDWN,	KeyboardCommand, 0,		NULL }, // Normal keyboard input
	{ 'keyb', KYRPT,	KeyboardCommand, 0,		NULL }, // Keyboard repeat

	{ 'menu', kEventMenuEnableItems,
						ValidateMenuCommand, 0,	NULL }, // Reset menus
	{ 'wind', kEventWindowClose,
						CloseCommand,	 kWINDOW, NULL },
	{ 'wind', kEventWindowActivated,
						RestoreCommand,	 kWINDOW, NULL },
	{ 'wind', CLICK,	MouseCommand,	 kWINDOW, NULL },
	{ 'wind', WNUPD, 	UpdateCommand,	 kWINDOW, NULL },

	{ 'wind', WNRSZ,	ResizeCommand,	 kWINDOW, NULL }, // Change window size
	{ 'appl', kEventAppActivated,
						ResumeCommand,	 0,		NULL },
	{ 'Play', 'Band',	AngbandGame,	 0,		NULL }, // Start event dispatch
};
 
/*
 * Interpolate images when rescaling them
 */
static bool interpolate = 0;

/*
 * Use antialiasing.  Without image differencing from
 * OSX  10.4 features, you won't want to use this.
 */

static bool antialias = 0;

static struct {
	bool *var;				// Value to toggle (*var = !*var)
	int menuID;				// Menu for this action (MenuRef would be better)
	int menuItem;			// Index of menu item for this acton
	bool refresh; 			// Change requires graphics refresh of main window.
} toggle_defs [] = {
	{ &arg_wizard, kSpecialMenu, kWizard, 	false},
	{ &arg_fiddle, kSpecialMenu, kFiddle, 	false},
	{ &use_sound,  kSpecialMenu, kSound,	false},
	{ &use_bigtile, kStyleMenu,  kBigTile,	true},
	{ &interpolate, kStyleMenu,  kInterpolate, true},
	{ &antialias,	kStyleMenu,	kAntialias,	true}
};

