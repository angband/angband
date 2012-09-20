/*

	:ts=3

	File  			 : main-ami.c DEVELOPER VERSION!

	Version  		 : 1.004 (26 Aug 1998)
	Angband  		 : 2.8.3/2.8.2/2.8.1

	Purpose  		 : Amiga module for Angband with graphics and sound

	Author			 : Mark Howson
	Email 			 : Mark.Howson@nottingham.ac.uk

	Original Author : Lars Haugseth
	Email 			 : larshau@ifi.uio.no
	WWW				 : http://www.ifi.uio.no/~larshau

	Tab size 		 : 3

	Todo:

		Window backgrounds. Started, but not ready yet.
		Put menu tables outside this file so they can be modified?
		Improve the new sound system, because it's a major hack now.
*/

/************************************************************************/
/* Please define the following to suit each variant:							*/
/* 																							*/
/* If variant is based upon Ang 281 : #define ANG281							*/
/* If variant is based upon Ang 282 : #define ANG282							*/
/* If variant is based upon Ang 283 : #define ANG283 *and*					*/
/*												  #define ANG282  						*/
/*																								*/
/* Comment out all the unused ANG28x definitions!								*/
/*																								*/
/* This is the 'developer' version, which means I've aimed more for 		*/
/* convenience than for elegance; this file will compile with variants	*/
/* based upon 281, 282 or 283. Yes, it's a hack. Every so often I'll		*/
/* release a 'nice' version which is around 30K smaller, but will be		*/
/* targeted for the latest version of Angband									*/
/************************************************************************/

/* What variant is this? Used in the highscore dump */
#define VERSION "Zangband 2.2.2"

/* Main 'assign' needed. Kick2.0+ systems usually don't need it anyway */
#define VERPATH "Zangband:"

//#define SANGBAND             /* Define if this is Sangband. */
#define ZANGBAND                /* Define if this is Zangband. Zangband now has extra gfx */
//#define KANGBAND				/* Define for Kang, used in Highscore handling */
//#define GFXFUNCS             /* Define if we allow gfx debugging functions. */
#define QUICKGFX                /* Define if we have 'optimap.s' and we'd like to use it */
//#define DEBUG
#define ANG283
#define ANG282                  /* Based upon Angband 2.8.2 ? */
//#define ANG281                /* Based upon Angband 2.8.1 ? */

#include "angband.h"

#undef byte					/* Prevents conflicts with dos.h */

#include "sound-ami.h"
#include <math.h>
#include <dos.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/ports.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <devices/inputevent.h>
#include <devices/conunit.h>
#include <devices/timer.h>
#include <graphics/gfxbase.h>
#include <graphics/modeid.h>
#include <graphics/scale.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/iff.h>
#include <libraries/gadtools.h>
#include <libraries/reqtools.h>
#include <proto/asl.h>
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/console.h>
#include <proto/gadtools.h>
#include <proto/reqtools.h>

#ifdef __GNUC__
#	define __near
#endif

#ifdef QUICKGFX
#	include "optimap.h"
#endif

/* Maximum length a filename (including a path) can reach. Somewhat arbitary */
#define MAX_PATH_LENGTH		160

/* How much memory to allocate for the blanked out mousepointer */
#define BLANKPOINTER_SIZE 128

/* True if Kickstart 3.0 or better present */
#define KICK30 ((kick_ver) >= 39)

/* True if Kickstart 2.1 or better present */
#define KICK21 ((kick_ver) >= 38)

/* True if Kickstart 2.0 or better available */
#define KICK20 ((kick_ver) >= 36)

/* True if Kickstart 1.3 or worse available */
#define KICK13 ((kick_ver) < 36)

#define PROTO

/* Pen number convertion */
#define PEN( p ) ( penconv[ p ] )

/* Graphics pen number convertion */
#define GPEN( p ) ( use_pub ? pubpens[ p ] : p )

/* Failure */
#define FAIL( str ) return ( amiga_fail( str ))

/* Message */
#define MSG( x, y, txt ) amiga_text( x, y, strlen( txt ), 1, txt );

/* Char and attr under cursor */
#define CUR_A ( td->t.scr->a[ td->cursor_ypos ][ td->cursor_xpos ] )
#define CUR_C ( td->t.scr->c[ td->cursor_ypos ][ td->cursor_xpos ] )

/* Colour to use for cursor */
#define CURSOR_PEN 4

// Max number of lines in a term (y)
#define MAX_TERM_VERT 24

// Max number of chars in a term (x)
#define MAX_TERM_HORIZ 80

/* Size of 8x8 tile image */
#define DF_GFXW 256
#define DF_GFXH 256
#define DF_GFXB 4

/* Size of 16x16 tile image */
#define AB_GFXW 512
#define AB_GFXH 848
#define AB_GFXB 8

#ifdef ZANGBAND
#  undef DF_GFXH
#  define DF_GFXH 736
#endif

/* Size of current bitmap...initialise by load_gfx() */
int GFXW, GFXH, GFXB;

/* Size of tombstone image */
#define TOMW 512
#define TOMH 168
#define TOMB 4

/* Filename of 8x8 tile image */
#define DE_MGFX "gfx/tiles.raw"             /* in xtra */

/* Filename of 16x16 tile image */
#define AB_MGFX "gfx/tiles256.raw"             /* in xtra */

/* Colour map for AB tiles */
#define AB_MGFX_CMAP "gfx/tiles256.cmap"

/* Filename of tombstone image */
#define MTOM "gfx/tomb.raw"              /* in xtra */

/* Filename of preferences file */
#define WPRF "settings.prf"              /* in user */

/* DisplayID specified with option -m */
char modestr[ 256 ] = "";

static byte palette256[1024];

/* Library bases */

/* 2.0 and better libraries - may not be available */
struct Library *GadToolsBase = NULL;
struct Library *AslBase = NULL;

#ifdef __GNUC__
struct ReqToolsBase *ReqToolsBase;
#endif
/* Need these libraries */
struct Library *DiskfontBase = NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *IFFBase = NULL;

struct Device *ConsoleDev = NULL;
struct Library *ConsoleDevice = NULL;

/* Term data structure. Needs sorting out */
typedef struct term_data
{
	term t;  					 /* Term structure */
	cptr name;  				 /* Name string, eg. title */

	struct BitMap *background;		/* Bitmap of special background */
	byte *bkgname;

	char fontname[64];		 /* Name of font, ie. 'topaz/8'. Used by Save Windows */

	struct Gadget ygad;      /* Used for window scrollbars */
	struct Gadget xgad;
	struct PropInfo ygadinfo,xgadinfo;
	struct Image ygadimage,xgadimage;

	BYTE use;					 /* Use this window */
	BYTE usegfx;             /* Use graphics ? */

	BYTE iconified;          /* Window is iconified ? */

	BYTE xpos;					 /* Position of data in window. Think of this as an x offset */
	BYTE ypos;					 /* Position of data (y) */
	BYTE scroll;				 /* Put scrollers in borders? */

	BYTE cols;  				 /* Number of columns */
	BYTE rows;  				 /* Number of rows */

	short wx;					 /* Window x-pos, in pixels */
	short wy;					 /* Window y-pos, in pixels */
	short ww;					 /* Window width */
	short wh;					 /* Window height */

	BYTE fw; 					 /* Font width */
	BYTE fh; 					 /* Font height */
	BYTE fb; 					 /* Font baseline */

	struct TextFont *font;   /* Font pointers */

	BYTE ownf;  				 /* Font is owned by this term */

	struct Window *win;  	 /* Window pointer */
	struct RastPort *wrp;	 /* RastPort of window */
	struct RastPort *rp; 	 /* RastPort of screen or window */

	struct BitMap *gfxbm;
	struct BitMap *mapbm;
	struct BitMap *mskbm;

	int gfx_w,gfx_h;

	int map_w, map_h;
	int map_x, map_y;

	int mpt_w, mpt_h;

	int cursor_xpos, cursor_ypos;
	bool cursor_visible;
	bool cursor_lit;
	int cursor_frame;
	int cursor_map;

	BYTE notitle;
	BYTE avoidbar;
}
term_data;

/* XXX XXX XXX Nasty */

struct high_score
{
	char what[8];		/* Version info (string) */

	char pts[10];		/* Total Score (number) */

	char gold[10];		/* Total Gold (number) */

	char turns[10];	/* Turns Taken (number) */

	char day[10];		/* Time stamp (string) */

	char who[16];		/* Player Name (string) */

	char uid[8];		/* Player UID (number) */

	char sex[2];		/* Player Sex (string) */
	char p_r[3];		/* Player Race (number) */
	char p_c[3];		/* Player Class (number) */

	char cur_lev[4];		/* Current Player Level (number) */
	char cur_dun[4];		/* Current Dungeon Level (number) */
	char max_lev[4];		/* Max Player Level (number) */
	char max_dun[4];		/* Max Dungeon Level (number) */

#ifdef KANGBAND
	char arena_number[4];	/* Arena level attained -KMW- */
	char inside_special[4];   /* Did the player die in the arena? -KMW- */
	char exit_bldg[4];	/* Can the player exit arena? Goal obtained? -KMW- */

#endif

	char how[32];		/* Method of death (string) */
//	char pad[9];
};

/* Term data for all windows */
static term_data data[ MAX_TERM_DATA ];

bool use_mask = FALSE;

static char do_after;

/* Window names */
static char *term_name[] =
{
	"Main", "Mirror", "Recall", "Choice",
	"5th Window", "6th Window", "7th Window", "8th Window",
	"Main", "Term-1","Term-2","Term-3",
	"Term-4","Term-5","Term-6","Term-7",
	NULL
};

static char *term_short[] =
{
	"MAIN","MIRROR","RECALL","CHOICE",
	"WIN5TH","WIN6TH","WIN7TH","WIN8TH",
	"MAIN","TERM-1","TERM-2","TERM-3",
	"TERM-4","TERM-5","TERM-6","TERM-7",
	NULL
};

// Nasty Optimise hack : Writes directly to memory to speed things up.
// Note lack of 'static' ; can be activated from main.c

bool nasty_optimise_gfx = FALSE;

bool backup_save = FALSE;

/* Can we display a palette requester? (ie. is screen type ok, reqtools etc. */
bool amiga_palette = FALSE;

/* We don't want to use this hack for the cursor, so we use the next variable
   to prevent that. If you have 'nasty_optimise_gfx' on when the menu's are
   being displayed, the flashing cursor will overwrite the menu on screen :(
*/
static bool block_nasty_gfx = FALSE;

#ifdef QUICKGFX
unsigned long *source_plane_addr;
unsigned long *dest_plane_addr;
unsigned short global_depth;
unsigned long source_row;
unsigned long dest_row;
unsigned long global_map_x;
unsigned long global_map_y;
#endif

/* Screen pointers */
static struct Screen *amiscr = NULL;
static struct Screen *pubscr = NULL;

static struct IOStdReq io_req;

/* Visual info for gadtools menus */
static APTR *visinfo;

/* TextAttr for screen font */
static struct TextAttr ScrAttr, *scrattr = &ScrAttr;

/* Screen characteristics */

static ULONG screen_width = 0;
static ULONG screen_height = 0;
static LONG screen_overscan = -1;
static ULONG screen_depth = 4;
static ULONG screen_cols = 16;
static ULONG scr_m = 0;

static BOOL screen_foreign = FALSE;	/* Use *only* graphics card compatible
													functions? Turn off for AGA */

static BOOL screen_enhanced = FALSE;

static term_data *term_curs = NULL;	/* Last term for cursor */

static char tmpstr[ 256 ];				/* Temp string for general usage */

static BOOL iconified = FALSE;		/* Iconify status of windows */
static BOOL use_menus = TRUE;			/* Use intuition menus? */

static struct InputEvent ie;			/* Window input event */

/*
	Version of KickStart available. Typical values are:

	34 				Kickstart 1.3 only
	36 				Kickstart 2.0
	38					Kickstart 2.1
	39 				Kickstart 3.0 or better
*/

static int kick_ver = 0;

static int use_pub = FALSE;			/* Use public screen? */
static int publock = FALSE;			/* TRUE if public screen Locked() */
static int backdrop = FALSE;			/* Use a backdrop main window */

static BOOL blankmouse = FALSE;		/* Use Mouse Blanking ?? */
static BOOL pointer_visible = TRUE;	/* Pointer visibility status */
static void *blankpointer;				/* Points to memory for blank mouse
													sprite. Must be CHIP MEM! */

static ULONG sigmask = 0L;				/* Holds mask for Wait() - see
													amiga_event() */

/* Convert textual pens to screen pens */
static UWORD penconv[ 16 ] =
{
	0,1,2,4,11,15,9,6,3,1,13,4,11,15,8,5
};

/* Public screen obtained pens */
static LONG pubpens[ 32 ] =
{
	-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1
};

/* Default colour palette, 16 for graphics, 16 for text */
static ULONG default_colors[ 32 ] =
{
	0x000000, 0xf0e0d0, 0x808080, 0x505050,
	0xe0b000, 0xc0a070, 0x806040, 0x403020,
	0x00a0f0, 0x0000f0, 0x000070, 0xf00000,
	0x800000, 0x9000b0, 0x006010, 0x60f040,

	0x000000, 0xffffff, 0xc7c7c7, 0xff9200,
	0xff0000, 0x00cd00, 0x172cff, 0xc86400,
	0x8a8a8a, 0xe0e0e0, 0xa500ff, 0xfffd00,
	0xff00bc, 0x00ff00, 0x00c8ff, 0xffcc80
};

/* Palette, 32 bits per gun */
static ULONG palette32[ 32 * 3 + 2 ];

/* Palette, 4 bits per gun */
static UWORD palette4[ 32 ];

/* Version string */
static char ver[] = "$VER: " VERSION " (" __DATE__ ")";

struct AmiSound
{
	char *Name;
	int Volume;
	int Channel;
	int Rate;
	int Repeats;
	int Memory;
	struct SoundInfo *Address;
};

static char *sound_name_desc = NULL;
static struct AmiSound *sound_data = NULL;
static int sounds_needed = 0;

// Ouch - hack
static struct AmiSound *sound_ref[SOUND_MAX][8];

static int channel_last[ 4 ] = { -1, -1, -1, -1 };
static int channel_num[ 4 ] = { 1, 1, 1, 1 };

static int has_sound = FALSE;

#define MENUMAX 300

/* Menu userdata indexes */

#define MNU_SCALEDMAP		1001
#define MNU_PALETTE			1002
#define MNU_SAVE_PALETTE	1003
#define MNU_LOAD_PALETTE	1004
#define MNU_EXPORT_HS   	1005
#define MNU_GFXMAP			1007
#define MNU_SAVE_WINDOWS 	1008

/* Special offset indexes */
#define MNU_KEYCOM		2001
#define MNU_CKEYCOM  	3001
#define MNU_OPTION		4001
#define MNU_HELP  		5001
#define MNU_WINDOW		6001

/* Macro for menu userdata keycodes and help */
#define MKC( c ) (void *)( MNU_KEYCOM + c )
#define MCC( c ) (void *)( MNU_CKEYCOM + c )
#define MHL( c ) (void *)( MNU_HELP + c )
#define MWI( c ) (void *)( MNU_WINDOW + c )

static struct Menu *menu = NULL;

/* Option code gone. It was rather impractical and was taking a fair bit
   of work to update, so I dumped it. Ought to reinstore this. */

/* Menu items that comes after the options */
struct NewMenu post_item[] =
{
	{ NM_TITLE, "Cmd1", 0, 0, 0, 0 },
	  { NM_ITEM, "a  Aim a wand", 0, 0, 0, MKC('a') },
	  { NM_ITEM, "b  Browse a book", 0, 0, 0, MKC('b') },
	  { NM_ITEM, "c  Close a door", 0, 0, 0, MKC('c') },
	  { NM_ITEM, "d  Drop an item", 0, 0, 0, MKC('d') },
	  { NM_ITEM, "e  Equipment list", 0, 0, 0, MKC('e') },
	  { NM_ITEM, "f  Fire an item", 0, 0, 0, MKC('f') },
	  { NM_ITEM, "g  Stay still", 0, 0, 0, MKC('g') },
	  { NM_ITEM, "i  Inventory list", 0, 0, 0, MKC('i') },
	  { NM_ITEM, "j  Jam a door", 0, 0, 0, MKC('j') },
	  { NM_ITEM, "k  Destroy an item", 0, 0, 0, MKC('k') },
	  { NM_ITEM, "l  Look around", 0, 0, 0, MKC('l') },

	{ NM_TITLE, "Cmd2", 0, 0, 0, 0 },
	  { NM_ITEM, "m  Cast a spell", 0, 0, 0, MKC('m') },
	  { NM_ITEM, "o  Open a door or chest", 0, 0, 0, MKC('o') },
	  { NM_ITEM, "p  Pray a prayer", 0, 0, 0, MKC('p') },
	  { NM_ITEM, "q  Quaff a potion", 0, 0, 0, MKC('q') },
	  { NM_ITEM, "r  Read a scroll", 0, 0, 0, MKC('r') },
	  { NM_ITEM, "s  Search for traps/doors", 0, 0, 0, MKC('s') },
	  { NM_ITEM, "t  Take off equipment", 0, 0, 0, MKC('t') },
	  { NM_ITEM, "u  Use a staff", 0, 0, 0, MKC('u') },
	  { NM_ITEM, "v  Throw an item", 0, 0, 0, MKC('v') },
	  { NM_ITEM, "w  Wear/wield equipment", 0, 0, 0, MKC('w') },
	  { NM_ITEM, "z  Zap a rod", 0, 0, 0, MKC('z') },

	{ NM_TITLE, "Cmd3", 0, 0, 0, 0 },
	  { NM_ITEM, "A  Activate an artifact", 0, 0, 0, MKC('A') },
	  { NM_ITEM, "B  Bash a door", 0, 0, 0, MKC('B') },
	  { NM_ITEM, "C  Character description", 0, 0, 0, MKC('C') },
	  { NM_ITEM, "D  Disarm a trap", 0, 0, 0, MKC('D') },
	  { NM_ITEM, "E  Eat some food", 0, 0, 0, MKC('E') },
	  { NM_ITEM, "F  Fuel your lantern/torch", 0, 0, 0, MKC('F') },
	  { NM_ITEM, "G  Gain new spells/prayers", 0, 0, 0, MKC('G') },
	  { NM_ITEM, "I  Observe an item", 0, 0, 0, MKC('I') },
	  { NM_ITEM, "L  Locate player on map", 0, 0, 0, MKC('L') },
	  { NM_ITEM, "M  Full dungeon map", 0, 0, 0, MKC('M') },
	  { NM_ITEM, "Q  Quit (commit suicide)", 0, 0, 0, MKC('Q') },
	  { NM_ITEM, "R  Rest for a period", 0, 0, 0, MKC('R') },
	  { NM_ITEM, "S  Toggle search mode", 0, 0, 0, MKC('S') },
	  { NM_ITEM, "T  Dig a tunnel", 0, 0, 0, MKC('T') },
#ifdef ZANGBAND
	  { NM_ITEM, "U  Use racial power", 0, 0, 0, MKC('U') },
#endif
	  { NM_ITEM, "V  Version info", 0, 0, 0, MKC('V') },

	{ NM_TITLE, "Cmd4", 0, 0, 0, 0 },
	  { NM_ITEM, "@  Interact with macros", 0, 0, 0, MKC('@') },
	  { NM_ITEM, "%  Interact with visuals", 0, 0, 0, MKC('%') },
	  { NM_ITEM, "&  Interact with colours", 0, 0, 0, MKC('&') },
	  { NM_ITEM, "*  Target monster or location", 0, 0, 0, MKC('*') },
	  { NM_ITEM, "(  Load screen dump", 0, 0, 0, MKC('(') },
	  { NM_ITEM, ")  Dump screen dump", 0, 0, 0, MKC(')') },
	  { NM_ITEM, "{  Inscribe an object", 0, 0, 0, MKC('{') },
	  { NM_ITEM, "}  Uninscribe an object", 0, 0, 0, MKC('}') },
	  { NM_ITEM, "[  Wear/Wield equipment", 0, 0, 0, MKC('[') },
	  { NM_ITEM, "]  Take off equipment", 0, 0, 0, MKC(']') },
	  { NM_ITEM, "-  Walk (flip pickup)", 0, 0, 0, MKC('-') },

	{ NM_TITLE, "Cmd5", 0, 0, 0, 0 },
	  { NM_ITEM, "+  Dig tunnel", 0, 0, 0, MKC('+') },
	  { NM_ITEM, "=  Set options", 0, 0, 0, MKC('=') },
	  { NM_ITEM, ";  Walk (with pickup)", 0, 0, 0, MKC(';') },
	  { NM_ITEM, ":  Take notes", 0, 0, 0, MKC(':') },
	  { NM_ITEM, "\" Enter a user pref command", 0, 0, 0, MKC('\"') },
	  { NM_ITEM, ",  Stay still (with pickup)", 0, 0, 0, MKC(',') },
	  { NM_ITEM, "<  Go up staircase", 0, 0, 0, MKC('<') },
	  { NM_ITEM, ".  Run", 0, 0, 0, MKC('.') },
	  { NM_ITEM, ">  Go down staircase", 0, 0, 0, MKC('>') },
	  { NM_ITEM, "/  Identify symbol", 0, 0, 0, MKC('/') },
	  { NM_ITEM, "|  Check uniques", 0, 0, 0, MKC('|') },
	  { NM_ITEM, "~  Check artifacts", 0, 0, 0, MKC('~') },
	  { NM_ITEM, "?  Help", 0, 0, 0, MKC('?') },

	{ NM_TITLE, "Cmd6", 0, 0, 0, 0 },
	  { NM_ITEM, "^f  Repeat level feeling", 0, 0, 0, MCC('f') },
	  { NM_ITEM, "^Q  Quit", 0, 0, 0, MCC('k') },
	  { NM_ITEM, "^p  Show previous messages", 0, 0, 0, MCC('p') },
	  { NM_ITEM, "^r  Redraw the screen", 0, 0, 0, MCC('r') },
	  { NM_ITEM, "^s  Save and don't quit", 0, 0, 0, MCC('s') },
	  { NM_ITEM, "^x  Save and quit", 0, 0, 0, MCC('x') },
	  { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
	  { NM_ITEM, "Draw dungeon map", "m", 0, 0, (void *)MNU_SCALEDMAP },
	  { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
	  { NM_ITEM, "Palette Requester", "r", 0, 0, (void *)MNU_PALETTE },
	  { NM_ITEM, "Load Palette","l", 0, 0, (void *)MNU_LOAD_PALETTE },
	  { NM_ITEM, "Save Palette","s", 0, 0, (void *)MNU_SAVE_PALETTE },
	  { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
	  { NM_ITEM, "Save highscores as ASCII", "h", 0, 0, (void *)MNU_EXPORT_HS },
#ifdef GFXFUNCS
	  { NM_ITEM, "Gfx mapper", "g", 0, 0, (void *)MNU_GFXMAP },
#endif
/*		 { NM_ITEM, "Merge highscore files", "f", 0, 0, (void *)MNU_MERGE_HS }, */

	{ NM_TITLE, "Help", 0, 0, 0, 0 },
	  { NM_ITEM, "General Information", 0, 0, 0, MHL('1') },
	  { NM_ITEM, "Creating a Character", 0, 0, 0, MHL('2') },
	  { NM_ITEM, "Exploring the Dungeon", 0, 0, 0, MHL('3') },
	  { NM_ITEM, "Attacking Monsters", 0, 0, 0, MHL('4') },
	  { NM_ITEM, "List of Commands", 0, 0, 0, MHL('5') },
	  { NM_ITEM, "List of Options", 0, 0, 0, MHL('6') },
	  { NM_ITEM, "Version Information", 0, 0, 0, MHL('7') },

	{ NM_TITLE, "Windows", 0, 0, 0, 0 },
	  { NM_ITEM, "Term 1", 0, CHECKIT | MENUTOGGLE, 0, MWI( 1 ) },
	  { NM_ITEM, "Term 2", 0, CHECKIT | MENUTOGGLE, 0, MWI( 2 ) },
	  { NM_ITEM, "Term 3", 0, CHECKIT | MENUTOGGLE, 0, MWI( 3 ) },
	  { NM_ITEM, "Term 4", 0, CHECKIT | MENUTOGGLE, 0, MWI( 4 ) },
	  { NM_ITEM, "Term 5", 0, CHECKIT | MENUTOGGLE, 0, MWI( 5 ) },
	  { NM_ITEM, "Term 6", 0, CHECKIT | MENUTOGGLE, 0, MWI( 6 ) },
	  { NM_ITEM, "Term 7", 0, CHECKIT | MENUTOGGLE, 0, MWI( 7 ) },
     { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
     { NM_ITEM, "Save Windows", "w",0,0,(void *)MNU_SAVE_WINDOWS },

   { NM_END, NULL, 0, 0, 0, 0 },
   { 255, 0, 0, 0, 0, 0 }
};

/* Menu array */
static struct NewMenu newmenu[ MENUMAX ];

extern void map_info( int y, int x, byte *ap, char *cp );
extern void center_string( char *buf, cptr str );
extern void amiga_gfxmap(void);

errr init_ami( void );
static int load_backpic(term_data *t, char *name);
static BOOL get_screenmode( char *modestr );
void amiga_open_libs( void );
void open_term( int n, bool doall );
void close_term( int n );
static void init_term( term_data *td );
static void link_term( int i );
static void free_term( term_data *td );
static BOOL strreq( char *s, char *d);
static void request_font( char *str );
static void request_mode( char *str );
int read_prefs( void );
static BOOL process_bool(char *param);
static void process_gfx(char *param);
static errr amiga_user( int n );
static void amiga_nuke( term *t );
static void amiga_open( term *t );
static errr amiga_curs( int x, int y );
static errr amiga_wipe( int x, int y, int n );
static errr amiga_clear( void );
static errr amiga_pict( int x, int y, int n, const byte *ap, const char *cp );
static errr amiga_text( int x, int y, int n, byte a, cptr s );
static errr amiga_xtra( int n, int v );
static errr amiga_flush( int v );
static void process_msg(int i,ULONG iclass, UWORD icode, UWORD iqual, APTR iaddr);
static void calc_sigmask(void);
errr amiga_event( int v );
static errr amiga_react( int v );
int amiga_tomb( void );
void tomb_str( int y, char *str );
void handle_rawkey( UWORD code, UWORD qual, APTR addr );
void handle_menupick( int mnum );
static void amiga_save_file( void );
static char get_bool( BOOL opt );
static void cursor_on( term_data *td );
static void cursor_off( term_data *td );
static void cursor_anim( void );
static int load_gfx( void );
static int conv_gfx( void );
static void copy_bitmap(struct BitMap *src, struct BitMap *dst, int x, int y, int d);
static int size_gfx( term_data *td );
static void put_gfx( struct RastPort *rp, int x, int y, int chr, int col );
static int amiga_fail( char *msg );
static void amiga_map( void );
void load_palette( void );
ULONG trans( byte g );
static void allocate_nearpens( void );
int create_menus( void );
void update_menus( void );
int init_sound( void );
void free_sound( void );
static void play_sound( int v );
void put_gfx_map( term_data *td, int x, int y, int c, int a );
struct BitMap *alloc_bitmap( int width, int height, int depth, ULONG flags, struct BitMap *friend );
void free_bitmap( struct BitMap *bitmap );
void scale_bitmap( struct BitMap *srcbm, int srcw, int srch, struct BitMap *dstbm, int dstw, int dsth );
void remap_bitmap( struct BitMap *srcbm, struct BitMap *dstbm, long *pens, int width, int height );
int depth_of_bitmap( struct BitMap *bm );
void amiga_show( char *str );
void amiga_redefine_colours( void );
void amiga_makepath( char *name );
void amiga_save_palette( void );
void amiga_load_palette( void );
static void amiga_hs_to_ascii(void);
void amiga_user_name( char *buf, int id );
void amiga_write_user_name( char *name );
static int get_p_attr( void );
static int get_p_char( void );
void amiga_register(char *ourname);

PROTO errr init_ami( void )
{
	int i;
	struct NewScreen new_scr;
	struct NewWindow new_win;
	struct DimensionInfo diminfo;
	int pw,ph,px,py,maxw,maxh,th,barh;

	BOOL changed = FALSE;
	term_data *ts = &data[ 0 ];
	term_data *tt = NULL;

	/* Open Amiga libraries */
	amiga_open_libs();

	/* Can't have palette requester if we don't have reqtools */
	if (ReqToolsBase)
		amiga_palette = TRUE;

	/* Ought to check the result of this, but if we don't have 128 bytes of
      chip memory left we're in all sorts of trouble... */
	blankpointer = AllocMem(BLANKPOINTER_SIZE,MEMF_CLEAR | MEMF_CHIP);

	/* See if we specified graphics | sound via command line */

	use_sound = arg_sound;
	use_graphics = arg_graphics;

	/* Initialise all terms */
   for ( i = 0; i < MAX_TERM_DATA; i++ )
		init_term( &data[ i ] );

	/* Always use the main term */
	ts->use = ts->usegfx = TRUE;
	ts->iconified = FALSE;

	/* We *must* have kickstart 34 or later, which should be no problem */
	/* *Maybe* V33 would be alright; maybe not */

	if ( IntuitionBase->LibNode.lib_Version < 34 )
		FAIL( "Sorry, this program requires Kickstart 1.3 or later." );

	/* Read preferences file */
	read_prefs();

	/* XXX XXX XXX  Command line options have priority */
	if (arg_graphics)
		use_graphics = 1;
	if (arg_sound)
		use_sound = 1;

	arg_graphics = use_graphics;
	arg_sound = use_sound;

	/* Initialize keyboard stuff */
	ie.ie_NextEvent = NULL;
	ie.ie_Class 	 = IECLASS_RAWKEY;
	ie.ie_SubClass  = 0;

	/* Need gadtools.library to use menus */
	if ( !GadToolsBase )
		use_menus = FALSE;

	/* Initialize colour palette */
	for ( i = 0; i < 32; i++ )
	{
		/* If undefined, use default palette */
		if ( angband_color_table[ i ][ 0 ] == 0 )
		{
			angband_color_table[ i ][ 0 ] = 1;
			angband_color_table[ i ][ 1 ] = ( default_colors[ i ] & 0xff0000 ) >> 16;
			angband_color_table[ i ][ 2 ] = ( default_colors[ i ] & 0x00ff00 ) >> 8;
			angband_color_table[ i ][ 3 ] = ( default_colors[ i ] & 0x0000ff );
		}
	}

	/* Search for prefered screenmode or public screen */
	if (KICK20 && strlen( modestr ) > 0 )
	{
		if (!get_screenmode( modestr ))
			FAIL("Display error.");
	}

	if (KICK13)
	{
      bool bad = FALSE;
		for (i = 1 ; i < MAX_TERM_DATA; i++)
		{
			if (!bad && data[ i ].use)
			{
				puts("Sorry... Extra windows not currently supported under KS1.3");
				bad = TRUE;
         }
			data[ i ].use = FALSE;
      }
	}
//	if ( !use_pub )
//	{
//		/* Extra windows not allowed on custom screen */

//      for (i = 1 ; i < MAX_TERM_DATA; i++)
//			data[ i ].use = FALSE;
//	}

	/* Calculate window dimensions */

	for ( i = 0 ; i < MAX_TERM_DATA; i++)
	{
		data[ i ].ww = data[ i ].fw * data[ i ].cols;
		data[ i ].wh = data[ i ].fh * data[ i ].rows;
//		data[ i ].wy = data[ i ].fh * data[ i ].wy;
//		data[ i ].wx = data[ i ].fw * data[ i ].wx;
	}

	/* Find a nice screenmode */
	if ( (scr_m == 0) && (KICK30) )
	{
		scr_m = BestModeID(
						BIDTAG_NominalWidth, ts->ww,
						BIDTAG_NominalHeight, ts->wh,
//						BIDTAG_DesiredWidth, ts->ww,
//						BIDTAG_DesiredHeight, ts->wh,
						BIDTAG_Depth, 4,
						TAG_END );
	}

	/* Use default screenmode if we don't have any */
	if ( scr_m == 0 || scr_m == INVALID_ID )
		scr_m = ( DEFAULT_MONITOR_ID | HIRES_KEY );

	/* Open custom screen */
	if ( !use_pub )
	{
		/* Need minimum screen depth of 4 */
		if (screen_depth < 4)
		{
			screen_depth = 4;
			screen_cols = 16;
		}
		for ( i = 0 ; i < 16; i++ )
			penconv[ i ] = i;

		/* We want to use 32 colours with graphics */
		if ( use_graphics && KICK20)
		{
			/* Get dimension data for screenmode */

			if ( GetDisplayInfoData( NULL, (UBYTE *) &diminfo, sizeof( struct DimensionInfo ), DTAG_DIMS, scr_m ))
			{
				/* Check if we support deep screens */
				if ( diminfo.MaxDepth > 4 )
				{
					/* Use 32 colors */
					if (screen_depth < 5)
					{
						screen_depth = 5;
						screen_cols = 32;
					}

					/* Use colors 16..31 for text */
					for ( i = 0; i < 16; i++ )
						penconv[ i ] = i + 16;
				}
			}
		}

		if ( KICK20 )
		{
			amiscr = OpenScreenTags( NULL,
				 screen_width ? SA_Width : TAG_IGNORE,
				 screen_width ? screen_width : TAG_IGNORE,
				 screen_height ? SA_Height : TAG_IGNORE,
				 screen_height ? screen_height : TAG_IGNORE,
				 (screen_overscan > 0) ? SA_Overscan : TAG_IGNORE,
				 (screen_overscan > 0) ? screen_overscan : TAG_IGNORE,
				 SA_Depth, screen_depth,
				 SA_DisplayID, scr_m,
				 SA_Font, scrattr,
				 SA_Type, CUSTOMSCREEN,
				 SA_Title, "Angband Screen",
				 SA_ShowTitle, FALSE,
				 SA_Quiet, TRUE,
				 SA_Behind, TRUE,
				 SA_AutoScroll, TRUE,
				 KICK30 ? SA_Interleaved : TAG_IGNORE, TRUE,
				 TAG_END );
		}
		else
		{
			new_scr.LeftEdge = 0;
			new_scr.TopEdge = 0;
			new_scr.Width = ts->ww;
			new_scr.Height = ts->wh;
			new_scr.Depth = screen_depth;
			new_scr.DetailPen = 0;
			new_scr.BlockPen = 1;
			new_scr.ViewModes = HIRES;  /* XXX XXX XXX */
			new_scr.Type = CUSTOMSCREEN;
			new_scr.Font = scrattr;
			new_scr.DefaultTitle = "Angband Screen";
			new_scr.Gadgets = NULL;
			new_scr.CustomBitMap = NULL;
			amiscr = OpenScreen( &new_scr );
		}

		if (!amiscr)
			FAIL( "Unable to open Amiga screen." );

		/* Initialize screen rastport */
		ts->rp = &amiscr->RastPort;
		SetRast( ts->rp, PEN( 0 ));
		SetAPen( ts->rp, 1 );
		SetBPen( ts->rp, 0 );
		SetDrMd( ts->rp, JAM2 );
		SetFont( ts->rp, ts->font );

		if (KICK20 && GetDisplayInfoData( NULL, (UBYTE *) &diminfo, sizeof( struct DimensionInfo ), DTAG_DIMS, scr_m ))
			backdrop = TRUE;

		px = amiscr->LeftEdge;
		py = amiscr->TopEdge;
		pw = amiscr->Width;
		ph = amiscr->Height;
	}

	/* We are using a public screen */
	else
	{
		/* Size of public screen */
		px = pubscr->LeftEdge;
		py = pubscr->TopEdge;
		pw = pubscr->Width;
		ph = pubscr->Height;

		/* Height difference between a window with or without a title bar */
		th = pubscr->Font->ta_YSize + 1;

		maxw = 0;
		/* Find width of widest window */
		for ( i = 0; i < MAX_TERM_DATA; i++ )
			maxw = MAX( maxw, data[ i ].ww );

		/* Find height of tallest window */
		maxh = ts->wh + ts->notitle ? 0 : th;
		for ( i = 0; i < MAX_TERM_DATA; i++ )
		{
			int tmp;

			tmp = data[ i ].wh + ( data[ i ].notitle ? 0 : th);
			if (data[i].use && tmp > maxh)
				maxh = tmp;
		}
//      maxh += pubscr->WBorTop + pubscr->WBorBottom;

		/* Check if the public screen is large enough */
		if ( pw < maxw || ph < maxh )
		{
			sprintf( tmpstr, "Public screen is too small for window (%d x %d).", maxw, maxh );
			FAIL( tmpstr );
		}

		/* Use backdrop window if pubscreen is quiet */
		backdrop = ( pubscr->Flags & SCREENQUIET ) ? TRUE : FALSE;

		/* Calculate screen bar height */
		barh = pubscr->BarHeight + 1;

		/* Check for special window positions */
		for ( i = 0; i < MAX_TERM_DATA; i++ )
		{
			/* Position window at the left side of the screen */
			if ( data[ i ].wx == -1 ) data[ i ].wx = pw - 1;

			/* Position window at the bottom of the screen */
			if ( data[ i ].wy == -1 ) data[ i ].wy = ph - 1;

			/* Position window below screen bar */
			if ( data[ i ].wy == -2 ) data[ i ].wy = barh;
		}
	}

	for (i = 0 ; i < MAX_TERM_DATA; i++)
	{
		changed = 0;
		if (data[i].wx < px)
		{
			data[i].wx = px;
			changed = 1;
		}
		if (data[i].wy < py)
		{
			data[i].wy = py;
			changed = 1;
		}
		if (data[i].wy > (py + ph))
			data[i].wy = 0;
		if (data[i].wx > (px + pw))
			data[i].wx = 0;

		if ((data[i].ww + data[i].wx) > (px + pw))
		{
			data[i].ww = (px + pw - data[i].wx);
			changed = 1;
		}
		if ((data[i].wh + data[i].wy) > (py + ph))
		{
			data[i].wh = (py + ph - data[i].wy);
			changed = 1;
		}
		if (changed && data[i].use)
			printf("Window %d resized...Please change the window dimensions\n",i);
	}

	/* Get visual info for GadTools */
	if ( use_menus )
	{
		if (( visinfo = GetVisualInfo( use_pub ? pubscr : amiscr, TAG_END )) == NULL )
			use_menus = FALSE;
	}

	if ( KICK20 )
	{
		ts->win = OpenWindowTags( NULL,
			 WA_Left, ts->wx,
			 WA_Top, ts->wy,
			 WA_InnerWidth, ts->ww,
			 WA_InnerHeight, ts->wh,
			 use_pub ? WA_PubScreen : WA_CustomScreen, use_pub ? pubscr : amiscr,
			 WA_Backdrop, backdrop,
			 WA_Borderless, backdrop,
			 WA_GimmeZeroZero, !backdrop,
			 WA_DragBar, !backdrop && !ts->notitle,
			 WA_DepthGadget, !backdrop && !ts->notitle,
			 WA_NewLookMenus, TRUE,
			 backdrop ? TAG_IGNORE : WA_ScreenTitle, VERSION,
			 ( backdrop || ts->notitle ) ? TAG_IGNORE : WA_Title, ts->name,
			 WA_Activate, TRUE,
			 WA_RMBTrap, !use_menus,
			 WA_ReportMouse, TRUE,
			 WA_IDCMP, IDCMP_RAWKEY | IDCMP_INTUITICKS | IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_MENUVERIFY | IDCMP_INACTIVEWINDOW | IDCMP_ACTIVEWINDOW | IDCMP_CLOSEWINDOW,
			 TAG_END );
   }
		else
	{
		new_win.LeftEdge = ts->wx;
		new_win.TopEdge = ts->wy;
		new_win.Width = ts->ww;
		new_win.Height = ts->wh;
		new_win.DetailPen = 255;
		new_win.BlockPen = 255;
		new_win.IDCMPFlags = IDCMP_RAWKEY | IDCMP_INTUITICKS | IDCMP_MOUSEMOVE |
					IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_MENUVERIFY;
		new_win.Flags = WFLG_BACKDROP | WFLG_BORDERLESS | WFLG_REPORTMOUSE | WFLG_SMART_REFRESH | WFLG_ACTIVATE | WFLG_RMBTRAP;
		if ( backdrop )
			new_win.Flags |= ( WFLG_BORDERLESS | WFLG_BACKDROP );
		new_win.FirstGadget = NULL;
		new_win.CheckMark = NULL;
		new_win.Title = NULL;
		new_win.Screen = amiscr;
		new_win.BitMap = NULL;
		new_win.MinWidth = new_win.MinHeight = new_win.MaxWidth = new_win.MaxHeight = 0;
		new_win.Type = CUSTOMSCREEN;

		ts->win = OpenWindow( &new_win );
	}
	if (!ts->win)
		FAIL( "Cannot open Amiga window.");

	/* Unlock public screen */
	if ( publock )
	{
		UnlockPubScreen( NULL, pubscr );
		publock = FALSE;
	}

	/* Initialize main rastport */
	ts->wrp = ts->win->RPort;
	SetRast( ts->wrp, PEN( 0 ));
	SetAPen( ts->wrp, 1 );
	SetBPen( ts->wrp, 0 );
	SetDrMd( ts->wrp, JAM2 );
	SetFont( ts->wrp, ts->font );

	/* Never use screen's rastport on public screen */
//	if ( use_pub )
		ts->rp = ts->wrp;

	if (IFFBase && ts->bkgname)
		load_backpic(ts,ts->bkgname);

	/* Handle other terms */
	for ( i = 1; i < MAX_TERM_DATA; i++ )
	{
		/* Term pointer */
		tt = &data[ i ];

		/* Skip this term if iconified */
		if ( tt->iconified || !tt->use)
			continue;

		if (KICK13)
			FAIL("Extra term windows not supported under 1.3");

		/* Load background picture, if possible */

		if (IFFBase && tt->bkgname)
			load_backpic(tt,tt->bkgname);
		open_term(i,FALSE);
	}

	/* Create palette for screen */
	load_palette();

	if (screen_enhanced)
		allocate_nearpens();

	for ( i = MAX_TERM_DATA; i-- > 0 ; )
	{
		if ( data[ i ].use )
			link_term( i );
	}

	/* Bring main window to front */
	if ( !backdrop )
		WindowToFront( ts->win );

	/* Bring screen to front */
	ScreenToFront( use_pub ? pubscr : amiscr );

	/* Load and convert graphics */
	if ( use_graphics )
	{
		MSG( 0, 0, "Loading graphics" );
		if ( !load_gfx() )
			FAIL( NULL );

		MSG( 0, 1, "Remapping graphics" );
		if ( !conv_gfx() )
		{
			FAIL( "Not enough memory to remap graphics." );
		}

		/* Scale the graphics to fit font sizes */
		for ( i = 0; i < MAX_TERM_DATA; i++ )
		{
			if ( data[ i ].use && data[ i ].usegfx )
			{
				if ( !size_gfx( &data[ i ] ) )
				{
					break;
//					FAIL( "Out of memory while scaling graphics." );
				}
			}
		}
	}

	/* Load sound effects */
	if ( use_sound )
	{
		MSG( 0, 2, "Loading sound effects" );
		sound_name_desc = malloc(4000);
		init_sound();
	}

	if (pubscr)
	{
		//amiga_palette = FALSE;
		if (nasty_optimise_gfx)
			printf("Sorry, can't use quick graphics mode on public screens\n");
		nasty_optimise_gfx = FALSE;
	}
	/* Success */
	return ( 0 );
}

PROTO static int load_backpic(term_data *t, char *name)
{
	IFFL_HANDLE iff;
	struct IFFL_BMHD *bmhd;
	UBYTE *colour_table;
	int i;

	if (iff = IFFL_OpenIFF(name, IFFL_MODE_READ) )
	{
		int cols;
		int start;

		if (colour_table = IFFL_FindChunk(iff, ID_CMAP) )
		{
			ULONG *l = (ULONG *)(colour_table + 4);
			int r,g,b;

			/* Calculate how many colours we need */
			if (use_graphics)
				start = 32;
			else
				start = 16;

			cols = i = *l / 3;
			colour_table += 8;
			while (i)
			{
				/* Calculate colour... */
				r = *colour_table++;g = *colour_table ++ ; b = *colour_table++;
				if (start < screen_cols)
				{
					if (KICK30)
						SetRGB32( &amiscr->ViewPort, start, (r << 24) | 0xFFFFFF, (g << 24) | 0xFFFFFF, (b << 24) | 0xFFFFFF );
					else
						SetRGB4( &amiscr->ViewPort, start, r >> 4, g >> 4, b >> 4 );

					start++;
				}
				i--;
			}
		}
		if (bmhd = IFFL_GetBMHD( iff ) )
		{
			struct BitMap *bkg;
			long pens[64];

			if (use_graphics)
				start = 32;
			else
				start = 16;

			for (i = 0 ; i < cols ; i++)
				pens[i] = start + i;

			bkg = alloc_bitmap( bmhd->w, bmhd->h, screen_depth, 0, NULL );
			t->background = alloc_bitmap( bmhd->w, bmhd->h, bmhd->nPlanes, 0, NULL );
			if (!t->background || !bkg)
			{
				puts("Not enough memory for bitmaps!");
				if (t->background)
				{
					free_bitmap(t->background);
					t->background = NULL;
				}
				if (bkg)
					free_bitmap(bkg);
				IFFL_CloseIFF( iff );
				return 1;
			}
			IFFL_DecodePic(iff, t->background);
			remap_bitmap(t->background, bkg, pens, bmhd->w, bmhd->h);
			free_bitmap( t->background );
			t->background = bkg;

//			free_bitmap(bkg);
		}
		IFFL_CloseIFF( iff );
		return 0;
	}
	else
	{
		printf("Can't open file %s\n",name);
		return 1;
	}
}

// TRUE if ok
PROTO static BOOL get_screenmode( char *modestr )
{
	LONG pen;
	int i;

	/* Convert string to long */
	scr_m = strtol( modestr, NULL, 0 );

	/* It was not a number, so treat it as a public screen name */
	if ( scr_m == 0)
	{
		/* We need kickstart 3.0+ to use a public screen */
		if ( !(KICK20) )
		{
			puts( "Public screen can only be used on Kickstart 2.0 or later." );
			return FALSE;
		}

		/* Try to lock the named public screen if it isn't already */
		if ( !pubscr )
			pubscr = LockPubScreen( modestr );

		// Failed?
		if ( !pubscr )
		{
			printf( "Unable to get a lock on screen '%s'\n", modestr );
			return FALSE;
		}

		// We got a lock now
		publock = TRUE;

		scr_m = -1;
		use_pub = TRUE;

		/* Don't blank mouse on public screen */
		blankmouse = FALSE;

		if (KICK30)
		{
			/* Find suitable pens to use on public screen */
			for ( i = 0; i < 32; i++ )
			{
				pen = ObtainBestPen( pubscr->ViewPort.ColorMap,
											angband_color_table[ i ][ 1 ] << 24,
											angband_color_table[ i ][ 2 ] << 24,
											angband_color_table[ i ][ 3 ] << 24,
											OBP_Precision, PRECISION_EXACT );
				if ( pen == -1 )
				{
					puts( "Unable to obtain suitable pens to use on public screen. ");
					return FALSE;
				}
				pubpens[ i ] = pen;
			}
		}
		else
		{
			for (i = 0 ; i < 32 ; i++)
				pubpens[ i ] = i;
		}
		for ( i = 0; i < 16; i++ )
			penconv[ i ] = (UWORD) pubpens[ i + 16 ];

	}
	/* Use specified screenmode if available */
	else
	{
		/* Check if requested mode is available */
		if ( ModeNotAvailable( scr_m ))
			scr_m = 0;
	}
	return TRUE;
}

/* -------------------------------------------------------------------- */
/*  amiga_open_libs( void )                                             */
/*                                                                      */
/*  Opens Amiga libraries; including Intuition, Graphics etc.           */
/*  Dos is assumed to be open.                                          */
/* -------------------------------------------------------------------- */

PROTO void amiga_open_libs( void )
{
	static char tmpname[128];
	void *lib;

	/* No need to test if intuition.library opens. Ahem */

	IntuitionBase = (struct IntuitionBase *)OpenLibrary( "intuition.library", 0L);
	DiskfontBase = OpenLibrary( "diskfont.library", 0L);
	GfxBase = (struct GfxBase *)OpenLibrary( "graphics.library", 0L);
	IFFBase = OpenLibrary("iff.library", 0L);

	if (!DiskfontBase)
		amiga_fail( "Sorry, this program needs diskfont.library" );

	/* Decide which version of the system we have. Ought to use version.library */

	kick_ver = IntuitionBase->LibNode.lib_Version;

	/* Open some 2.0+ or better libraries. No point even trying under 1.3 */
	if (KICK20)
	{
		AslBase = OpenLibrary( "asl.library", 36L);
		GadToolsBase = OpenLibrary( "gadtools.library", 36L);
	}

	/* Initialise console (only using RawKeyConvert) */
	ConsoleDev = (struct Device *)OpenDevice("console.device",CONU_LIBRARY,(struct IORequest *)&io_req,0L);
	ConsoleDevice = (struct Library *)io_req.io_Device;

	/* Primitive test for RTG */
	lib = (void *)OpenLibrary( "cybergraphics.library", 0L);
	if (lib)
	{
		screen_foreign = TRUE;
		CloseLibrary( lib );
	}
	ReqToolsBase = (struct ReqToolsBase *)OpenLibrary( "reqtools.library",0L);
	if (!ReqToolsBase)
	{
		sprintf(tmpname,"%slibs/reqtools.library",ANGBAND_DIR);
		ReqToolsBase = (struct ReqToolsBase *)OpenLibrary( tmpname, 0L);
	}
}

/* -------------------------------------------------------------------- */
/*  open_term( int n)                                                   */
/* -------------------------------------------------------------------- */

PROTO void open_term( int n, bool doall )
{
	term_data *tt = &data[ n ];
   int i;

	/* Skip this term if not in use */
	if ( doall && !tt->use )
		return;

	/* If already open, don't reopen */
	if ( tt->win )
		return;

	// Initialise vertical prop gadget

	if (tt->scroll)
	{
		i = tt->rows;
		if (i > MAX_TERM_VERT)
			i = MAX_TERM_VERT;

		memset(&tt->ygadinfo,0,sizeof(struct PropInfo));
		memset(&tt->ygad,0,sizeof(struct Gadget));
		memset(&tt->ygadimage,0,sizeof(struct Image));

		tt->ygadinfo.Flags     = AUTOKNOB | FREEVERT | PROPNEWLOOK;
		tt->ygadinfo.HorizPot  = tt->ygadinfo.VertPot = 0;
		tt->ygadinfo.HorizBody = MAXBODY;
		tt->ygadinfo.VertBody  = MAXBODY / (MAX_TERM_VERT / i);

		tt->xgadinfo.Flags     = AUTOKNOB | FREEVERT | PROPNEWLOOK;
		tt->xgadinfo.HorizPot  = tt->xgadinfo.VertPot = 0;
		tt->xgadinfo.HorizBody = MAXBODY / (MAX_TERM_HORIZ / i);
		tt->xgadinfo.VertBody  = MAXBODY;

		tt->ygad.LeftEdge   = -14;
		tt->ygad.TopEdge    = amiscr->WBorTop + amiscr->Font->ta_YSize + 2;
		tt->ygad.Width      = 12;
		tt->ygad.Height     = -tt->ygad.TopEdge - 11;

		tt->xgad.LeftEdge   = -3;
		tt->xgad.TopEdge    = -7;
		tt->xgad.Width      = -23;
		tt->xgad.Height     = 6;

		tt->ygad.Flags        = GFLG_RELRIGHT | GFLG_RELHEIGHT;
		tt->ygad.Activation   = GACT_RELVERIFY | GACT_IMMEDIATE | GACT_RIGHTBORDER;
		tt->ygad.GadgetType   = GTYP_PROPGADGET | GTYP_GZZGADGET;
		tt->ygad.GadgetRender = (APTR)&(tt->ygadimage);
		tt->ygad.SpecialInfo  = (APTR)&(tt->ygadinfo);
		tt->ygad.GadgetID     = 0;
		tt->ygad.NextGadget   = NULL;

		tt->xgad.Flags        = GFLG_RELBOTTOM | GFLG_RELWIDTH;
		tt->xgad.Activation   = GACT_RELVERIFY | GACT_IMMEDIATE | GACT_BOTTOMBORDER;
		tt->xgad.GadgetType   = GTYP_PROPGADGET | GTYP_GZZGADGET;
		tt->xgad.GadgetRender = (APTR)&(tt->xgadimage);
		tt->xgad.SpecialInfo  = (APTR)&(tt->xgadinfo);
		tt->xgad.GadgetID     = 1;
		tt->xgad.NextGadget   = NULL;
	}

//	printf("Window %d   x %d y %d w %d h %d\n",n,tt->wx,tt->wy,tt->ww,tt->wh);
//	Delay(100);
	if (( tt->win = OpenWindowTags( NULL,
			WA_Left, tt->wx,
			WA_Top, tt->wy,
			WA_Width, tt->ww,
			WA_Height, tt->wh,
			WA_MinWidth,-1,
			WA_MaxWidth,-1,
			WA_MinHeight,-1,
			WA_MaxHeight,-1,
			WA_DetailPen,4,
			WA_BlockPen,2,  // makes no difference, as newlook specified
			use_pub ? WA_PubScreen : WA_CustomScreen, use_pub ? pubscr : amiscr,
			WA_GimmeZeroZero, TRUE,
			WA_DragBar, !tt->notitle,
			WA_SizeGadget,TRUE,
			WA_CloseGadget,TRUE,
			WA_DepthGadget, !tt->notitle,
			WA_NewLookMenus, TRUE,
			WA_ScreenTitle, VERSION,
			WA_IDCMP, IDCMP_NEWSIZE | IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_GADGETDOWN,
			tt->notitle ? TAG_IGNORE : WA_Title, tt->name,
			WA_ReportMouse, TRUE,
			tt->scroll ? WA_Gadgets : TAG_IGNORE,
			tt->scroll ? &(tt->ygad) : NULL,
			TAG_END )) == NULL )
		{
			amiga_fail("Unable to open term window.");
		}

	/* Initialize rastport */
	tt->rp = tt->wrp = tt->win->RPort;
	SetRast( tt->rp, PEN( 0 ));
	SetAPen( tt->rp, 1 );
	SetBPen( tt->rp, 0 );
	SetDrMd( tt->rp, JAM2 );
	SetFont( tt->rp, tt->font );

	if (tt->background)
		BltBitMapRastPort( tt->background, 0, 0, tt->rp, 0, 0, tt->ww, tt->wh, 0xC0);

	calc_sigmask();
	if (doall)
	{
		/* Term is no longer iconified */
		tt->iconified = FALSE;

		/* Refresh term */
		Term_activate( angband_term[ n ] );
		Term_redraw();
		Term_activate( angband_term[ 0 ] );
	}
}

PROTO void close_term( int n )
{
	term_data *tt = &data[ n ];

	/* Skip this term if not in use, or already closed */
	if ( !tt->use || tt->iconified)
		return;

	/* Close window */
	if ( tt->win )
		CloseWindow( tt->win );

	tt->win = NULL;
	tt->wrp = tt->rp = NULL;

	/* Recalculate signal mask, as window has now disappeared */
	calc_sigmask();

	/* Term is now iconified */
	tt->iconified = TRUE;
}

PROTO static void init_term( term_data *td )
{
	td->name = "term_name_xxx";

	td->use = FALSE;
	td->usegfx = FALSE;
	td->iconified = FALSE;
	td->scroll = FALSE;

	/* Term size */
	td->xpos = td->ypos = 0;

	td->cols = 80;
	td->rows = 24;

	/* Term dimension */
	td->wx = 0;
	td->wy = 0;
	td->ww = 0;
	td->wh = 0;

	/* System default font */
	td->font = GfxBase->DefaultFont;
	td->ownf = FALSE;
	td->fw	= td->font->tf_XSize;
	td->fh	= td->font->tf_YSize;
	td->fb	= td->font->tf_Baseline;

	/* Background bitmap data and path of IFF picture */
	td->background = NULL;
	td->bkgname = NULL;

	/* No window or rastports */
	td->win  = NULL;
	td->wrp  = NULL;
	td->rp	= NULL;

	/* No bitmaps */
	td->gfxbm = NULL;
	td->mskbm = NULL;
	td->mapbm = NULL;

	/* Cursor status */
	td->cursor_xpos = 0;
	td->cursor_ypos = 0;
	td->cursor_visible = FALSE;
	td->cursor_lit = FALSE;
	td->cursor_frame = 0;
	td->cursor_map = FALSE;

	/* Use window title */
	td->notitle = FALSE;
}

PROTO static void link_term( int i )
{
	term_data *td = &data[ i ];
	term *t;

	/* Term pointer */
	t = &td->t;

	/* Initialize the term */
	term_init( t, td->cols, td->rows, 256 );

	/* Hooks */
	t->init_hook = amiga_open;
	t->nuke_hook = amiga_nuke;
	t->text_hook = amiga_text;
	t->pict_hook = amiga_pict;
	t->wipe_hook = amiga_wipe;
	t->curs_hook = amiga_curs;
	t->xtra_hook = amiga_xtra;
	t->user_hook = amiga_user;

	/* We are emulating a hardware cursor */
	t->soft_cursor = FALSE;

	/* Draw graphical tiles one by one */

	t->always_text = TRUE;
	t->always_pict = FALSE;
	t->higher_pict = FALSE;

   /* Misc. efficiency flags */
   t->never_bored = TRUE;
   t->never_frosh = TRUE;

   /* Erase with "white space" */
   t->attr_blank = TERM_WHITE;
   t->char_blank = ' ';

   /* Remember where we come from */
   t->data = (vptr) td;

   /* Activate it */
   Term_activate( t );

   /* Global pointer */
   angband_term[ i ] = t;
}

PROTO static void free_term( term_data *td )
{
	/* Do we own the font? */
	if ( td->ownf && td->font )
	{
		CloseFont( td->font );
		td->font = NULL;
	}
	if ( td->win )
	{
		CloseWindow( td->win );
		td->win = NULL;
	}

	/* Free bitmaps */
	if ( td->gfxbm )
	{
		free_bitmap( td->gfxbm );
		td->gfxbm = NULL;
	}
	if ( td->mskbm )
	{
		free_bitmap( td->mskbm );
		td->mskbm = NULL;
	}
	if ( td->mapbm )
	{
		free_bitmap( td->mapbm );
		td->mapbm = NULL;
	}
	if ( td->background )
	{
		free_bitmap( td->background );
		td->background = NULL;
	}
}

PROTO static BOOL strreq( char *s, char *d)
{
	return((BOOL)!stricmp(s,d));
}

PROTO static void request_font( char *str )
{
	struct FontRequester *req = NULL;

	/* Blank string as default */
	*str = 0;

	if (AslBase)
	{
		/* Allocate screenmode requester */
		if ( req = AllocAslRequestTags( ASL_FontRequest, TAG_DONE ))
		{
			/* Open screenmode requester */
			if ( AslRequestTags( req, ASLFO_FixedWidthOnly, TRUE, TAG_DONE ))
			{
				/* Store font name and size */
				sprintf( str, "%s/%d", req->fo_Attr.ta_Name, req->fo_Attr.ta_YSize );
			}
			/* Free requester */
			FreeAslRequest( req );
		}
	}
}

PROTO static void request_mode( char *str )
{
	struct ScreenModeRequester *req = NULL;

	/* Blank string as default */
	*str = 0;

	if (AslBase)
	{
		/* Allocate screenmode requester */
		if ( req = AllocAslRequestTags( ASL_ScreenModeRequest, TAG_DONE ))
		{
			/* Open screenmode requester */
			if( AslRequestTags( req, TAG_DONE ))
			{
				/* Store font name and size */
				sprintf( str, "0x%X", req->sm_DisplayID );
			}
			/* Free requester */
			FreeAslRequest( req );
		}
	}
}

/*
 * read_prefs()
 *
 * Parse `settings.prf` file, if found.
 *
 * Could still do with some work
 *
 */

PROTO int read_prefs( void )
{
	static char errorstr[] = "PREFS:Unrecognised option";
	FILE *file;
	static char line[ 256 ];
	static char fname[ MAX_PATH_LENGTH ];
	char fontname[ 256 ];
	char custom_str[200];
	char public_str[200];
	char *first,*type,*param;
	char *s;
	int i,k;
	int fsize;
	struct TextAttr attr;
	term_data *td;

	enum { S_CUSTOM, S_PUBLIC, S_NONE };
	int force_mode = S_NONE;

	public_str[0] = custom_str[0] = 0;

	path_build( fname , MAX_PATH_LENGTH , ANGBAND_DIR_USER , WPRF);
	file = fopen( fname , "r" );
	if (!file)
	{
		printf("\nUnable to open file '%s'.\n", fname );
		return( FALSE );
	}

	/* Read next line from file */
	while ( fgets( line, 256, file ))
	{
      for (i = 0; line[i] && line[i] <= 32 ; i++)
			;
		if (line[i] == '#' || line[i] == ';' || !line[i])
			continue;

		k = i;
		while (line[i] && line[i] != '.')
			i++;

		if (!line[i])
		{
			printf("PREFS:Error in line '%s' in settings.prf\n",line);
			continue;
		}
		line[i++] = 0;
		first = (char *)(line + k);
   	while (line[i] == 32 || line[i] == 9)
			i++;
		type = (char *)(line + i);
		while (line[i] > 32)
			i++;
		line[i++] = 0;
		param = (char *)(line + i);
   	while (line[i] == 32 || line[i] == 9)
			i++;
		while (line[i] > 32)
			i++;
		line[i] = 0;

		if (param[0] == 0)
			param = NULL;
		if (strreq(first,"ANGMAN"))
			continue;

      if (strreq(first,"ANGBAND"))
		{
			if (strreq(type,"gfx"))
			{
				process_gfx(param);
			}
			else if (strreq(type,"sound"))
				use_sound = process_bool(param);
			else if (strreq(type,"version"))
				;
			else if (strreq(type,"backup"))
				backup_save = process_bool(param);
			else
				printf("%s %s.%s\n",errorstr,first,type);
			continue;
		}
		if (strreq(first,"SCREEN"))
		{
			if (strreq(type,"menus"))
				use_menus = process_bool(param);
			else if (strreq(type,"blankmouse"))
				blankmouse = process_bool(param);
			else if (strreq(type,"quick"))
			{
				/* Only allow quick graphics on non-RTG systems! */
				if (!screen_foreign)
					nasty_optimise_gfx = process_bool(param);
			}
			else if (strreq(type,"use"))
			{
				if (param[0] == 'p' || param[0] == 'P')
					force_mode = S_PUBLIC;
				else
					force_mode = S_CUSTOM;
			}
			else if (strreq(type,"width"))
				screen_width = atoi(param);
			else if (strreq(type,"height"))
				screen_height = atoi(param);
			else if (strreq(type,"depth"))
			{
				screen_depth = atoi(param);
				screen_cols = 1 << screen_depth;
			}
			else if (strreq(type,"overscan"))
				screen_overscan = atoi(param);
			else if (strreq(type,"rtg"))
			{
				screen_foreign = process_bool(param);
				if (screen_foreign)
					nasty_optimise_gfx = FALSE;
			}
			else if (strreq(type,"name"))
			{
				if ( KICK13 )
				{
					puts("Sorry - you need KS2.0 or better for SCREEN.name\n");
					continue;
				}
				strcpy(public_str, param);
			}
			else if (strreq(type,"mode"))
			{
				if ( KICK13 )
				{
					puts("Sorry - you need KS2.0 or better for SCREEN.mode\n");
					continue;
				}
				strcpy( custom_str,param );
				if (modestr[0] == '?')
					request_mode( custom_str );
			}
			else
				printf("%s %s.%s\n",errorstr,first,type);
			continue;
		}
		k = -1;
		for (i = 0 ; term_short[i] ; i++)
		{
			if (strreq( first, term_short[i] ))
			{
				k = i & 7;
				break;
			}
		}

		if (k != -1)
			td = &data[ k ];
		else
		{
//			printf( "PREFS: Error in line '%s'\n", line );
			continue;
		}

		/* Option 'use' - Use this term */
		if ( strreq( type, "use" ))
			td->use = process_bool( param );

		else if ( strreq( type, "scroll" ))
			td->scroll = process_bool( param );

		/* Option 'show' - Don't iconify */
		else if ( strreq( type, "show" ))
			td->iconified = !process_bool( param );

		/* Option 'cols' - Set number of columns for term */
		else if ( strreq( type, "cols" ))
			td->cols = atoi(param);

		/* Option 'rows' - Set number of rows for term */
		else if ( strreq( type, "rows" ))
			td->rows = atoi(param);

		/* Option 'xpos' - Set horizontal position for window in pixels */
		else if ( strreq( type, "xpos"))
			td->wx = atoi(param);

		/* Option 'ypos' - Set vertical position for window in pixels */
		else if ( strreq( type, "ypos" ))
			td->wy = atoi(param);

		else if ( strreq( type, "background" ))
		{
			td->bkgname = strdup( param );
			nasty_optimise_gfx = FALSE;
		}
		/* Option 'name' - Set window title */
		else if ( strreq( type, "title" ))
		{
			if (param)
				td->name = strdup( param );
			/* Don't use a title bar on this window */
			else
				td->notitle = TRUE;
		}
		/* Option 'font' - Set font for this window */
		else if ( strreq( type, "font" ))
		{
			/* Get value */

			strcpy(td->fontname,param);
			if (param[0] == '?')
				request_font(fontname);
			else
				strcpy(fontname,param);

			/* No font specification given, so use system font */
			if ( fontname[0] == 0)
			{
				/* Main window*/
				if ( td == &data[ 0 ] )
				{
					td->font = GfxBase->DefaultFont;

					/* Use default font as screen font */
					scrattr = NULL;
            }
				else
					td->font = data[ 0 ].font;

				/* Set font dimensions */
				td->fw	= td->font->tf_XSize;
				td->fh	= td->font->tf_YSize;
				td->fb	= td->font->tf_Baseline;

				/* This font is not opened by us */
				td->ownf = FALSE;

				/* Next line */
				continue;
         }
				else
			{
				/* Find font name/size delimiter */
				if (( s = strchr( fontname, '/' )) == NULL )
				{
					printf( "PREFS: Illegal font specification: '%s'.\n", fontname );
					continue;
				}

				/* Now get size of font */
				*s++ = 0;
				fsize = atoi( s );

				/* Make sure the font name ends with .font */
				if ( !strstr( fontname, ".font" ))
					strcat( fontname, ".font" );

				/* Set font attributes */
				attr.ta_Name  = fontname;
				attr.ta_YSize = fsize;
				attr.ta_Style = FS_NORMAL;
				attr.ta_Flags = ( !strcmp( fontname, "topaz.font" ) && ( fsize == 8 || fsize == 9 )) ?
							 FPF_ROMFONT : FPF_DISKFONT;

				/* Open font from disk */
				if ( td->font = OpenDiskFont( &attr ))
				{
					/* We own this font */
					td->ownf = TRUE;

					/* Set font dimensions */
					td->fw = td->font->tf_XSize;
					td->fh = td->font->tf_YSize;
					td->fb = td->font->tf_Baseline;

					/* Copy font attr to screen font */
					if ( td == &data[ 0 ] )
					{
						scrattr->ta_Name = strdup( fontname );
						scrattr->ta_YSize = fsize;
						scrattr->ta_Style = FS_NORMAL;
						scrattr->ta_Flags = attr.ta_Flags;
					}
				}
				else
				{
					/* Couldn't open, so use default font instead */
					td->font = GfxBase->DefaultFont;

					/* Use default font as screen font */
					scrattr = NULL;

					/* Output error message */
					printf( "PREFS:Unable to open font '%s/%d'.\n", fname, fsize );
				}
			}
		}

		/* Unknown option */
		else
		{
			/* Output error message */
			printf ( "\nPREFS: Unknown option '%s'.%s\n", first,type );
		}
	}

	if ((custom_str[0] && !public_str[0]) || (force_mode == S_CUSTOM) )
		strcpy(modestr,custom_str);
	else if ((public_str[0] && !custom_str[0]) || (force_mode == S_PUBLIC) )
   {
		int turnoff = FALSE;

		strcpy(modestr,public_str);
		for ( i = 0; i < MAX_TERM_DATA; i++ )
		{
			if ( data[ i ].bkgname )
			{
				turnoff = TRUE;
				data[ i ].bkgname = NULL;
			}
		}

		if (turnoff)
		{
			puts("Backgrounds not (yet) supported on public screens.");
			puts("Please use a custom screen or turn backgrounds off");
		}
	}
	else
	{
//		puts("Don't know whether to use a public or custom screen. Check prefs!");
		modestr[0] = 0;
	}
	fclose( file );
}

PROTO static BOOL process_bool(char *param)
{
	if (*param == 'Y' || *param == 'y' || *param == '1' ||
		 *param == 'T' || *param == 't')
		return TRUE;
	else
   	return FALSE;
}

PROTO static void process_gfx(char *param)
{
	use_graphics = FALSE;
	if (*param == 'Y' || *param == 'y' || *param == '1' ||
		 *param == 'T' || *param == 't')
		use_graphics = TRUE;
	if (*param == 'E' || *param == 'e')
	{
		use_graphics = TRUE;
		screen_enhanced = TRUE;
#ifdef ZANGBAND
		ANGBAND_GRAF = "new";
#endif
	}
}

PROTO static errr amiga_user( int n )
{
	return( 1 );
}

PROTO static void amiga_nuke( term *t )
{
   term_data *td = (term_data *)( t->data );
	if ( td == &data[ 0 ] )
	{
		amiga_fail( NULL );
		/* Flush the output */
		fflush( stdout );
	}
}

PROTO static void amiga_open( term *t )
{
	/* Nothing to do here */
}

PROTO static errr amiga_curs( int x, int y )
{
	term_data *td = term_curs;

	if ( td->cursor_visible )
		cursor_off( td );

	td->cursor_xpos = x;
	td->cursor_ypos = y;
	td->cursor_frame = 0;

	if ( td->cursor_visible )
		cursor_on( td );

	return ( 0 );
}

PROTO static errr amiga_wipe( int x, int y, int n )
{
	term_data *td = (term_data*)(Term->data);

	if ( (n > 0 ) && !td->iconified )
	{
		/* Erase rectangular area on screen */
		if (!td->background)
		{
			SetAPen( td->rp, PEN( 0 ) );
			RectFill( td->rp, x * td->fw, y * td->fh, ( x + n ) * td->fw - 1, ( y + 1 ) * td->fh - 1 );
		}
		else
			BltBitMapRastPort( td->background, x * td->fw, y * td->fh, td->rp, x * td->fw, y * td->fh, n * td->fw, td->fh, 0xC0);
	}

	return ( 0 );
}

PROTO static errr amiga_clear( void )
{
	term_data *td = (term_data*)(Term->data);

	if (td->iconified)
		return;

	/* Fill window with background colour, or background */
	if ( !td->background )
		SetRast( td->rp, PEN( 0 ));
	else
		BltBitMapRastPort( td->background, 0, 0, td->rp, 0, 0, td->ww, td->wh, 0xC0);
	return ( 0 );
}

PROTO static errr amiga_pict( int x, int y, int n, const byte *ap, const char *cp )
{
	term_data *td = (term_data*)(Term->data);

	char s[2];
	int i;
	byte a;
	char c;

	if ( td->iconified )
		return ( 0 );

	s[1] = 0;
	for ( i = 0; i < n; i++ )
	{
		a = *ap++;
		c = *cp++;

		/* Graphical tile */
		if ( a & 0x80 )
			put_gfx( td->rp, x, y, c & 0x7f, a & 0x7f );

		/* Textual character */
		else
		{
			*s = c;

			SetAPen( td->rp, PEN( a & 0x0f ));
			SetBPen( td->rp, PEN( 0 ));
			Move( td->rp, x * td->fw, y * td->fh + td->fb );
			Text( td->rp, (char *) s, 1 );
		}

		x++;
	}
	return ( 0 );
}

PROTO static errr amiga_text( int x, int y, int n, byte a, cptr s )
{
	term_data *td = (term_data*)(Term->data);
	int i;

	if (td->iconified)
		return( 1 );

	y -= td->ypos;
	x -= td->xpos;

	if ( y >= td->rows )
		return( 0 );
	if ( x >= td->cols )
		return( 0 );

	if ( x >= 0 && y >= 0 && n > 0 )
	{
		/* Draw gfx one char at a time */
		if (( a & 0xc0 ))
		{
			for ( i = 0; i < n; i++ )
				put_gfx( td->rp, x + i, y, s[ i ] & 0x7f, a & 0x7f );
		}

		/* Draw the string on screen */
		else
		{
			if (td->background)
			{
				BltBitMapRastPort( td->background, x * td->fw, y * td->fh, td->rp, x * td->fw, y * td->fh, n * td->fw, td->fh, 0xC0);
				if (KICK30)
					SetABPenDrMd( td->rp, PEN( a & 0x0F ), PEN( 0 ), JAM1);
				else
				{
					SetDrMd( td->rp, JAM1 );
					SetAPen( td->rp, PEN( a & 0x0f ));
					SetBPen( td->rp, PEN( 0 ));
				}
			}
			else
			{
				if (KICK30)
					SetABPenDrMd( td->rp, PEN( a & 0x0F ), PEN( 0 ), JAM2);
				else
				{
					SetDrMd( td->rp, JAM2 );
					SetAPen( td->rp, PEN( a & 0x0f ));
					SetBPen( td->rp, PEN( 0 ));
				}
			}
			Move( td->rp, x * td->fw, y * td->fh + td->fb );
			Text( td->rp, (char *) s, n );
		}
	}
	return ( 0 );
}

PROTO static errr amiga_xtra( int n, int v )
{
	term_data *td = (term_data *)(Term->data);

	long mytime;
	unsigned int clock[2], clock2[2];

	/* Analyze the request */
	switch ( n )
	{
		/* Wait for event */
		case TERM_XTRA_EVENT:

			return ( amiga_event( v ));

		/* Flush input */
		case TERM_XTRA_FLUSH:

			return ( amiga_flush( v ));

		/* Make a noise */
		case TERM_XTRA_CLEAR:

			return ( amiga_clear());

		/* Change cursor visibility */
		case TERM_XTRA_SHAPE:

			/* Cursor on */
			if ( v )
			{
				cursor_on( td );
				td->cursor_visible = TRUE;
			}
			/* Cursor off */
			else
			{
				cursor_off( td );
				td->cursor_visible = FALSE;
			}
			return ( 0 );

		/* Flash screen */
		case TERM_XTRA_NOISE:

			DisplayBeep( use_pub ? pubscr : amiscr );
			return ( 0 );

		/* Play a sound */
		case TERM_XTRA_SOUND:

			if ( has_sound )
				play_sound( v );

			return ( 0 );

		/* React on global changes */
		case TERM_XTRA_REACT:

			return ( amiga_react( v ));

		case TERM_XTRA_LEVEL:

			term_curs = td;
			return( 0 );

		case TERM_XTRA_DELAY:

			/* SAS/C code. I have GNU code for this, but not included it yet */
			timer(clock);
			v *= 1000;
			do
			{
				timer(clock2);
				mytime = (clock2[0] - clock[0]) * 1000*1000 + (clock2[1] - clock[1]);
				if (clock2[0] < clock[0])
					break;
			} while (mytime < v);

			/* This is wrong, I think. Delay() does not have enough resolution
				to do this sensibly.
//         v = (v * 50) / 1000;
//			if (v)
//				Delay( v );
			return (0);

		/* Unknown request type */
		default:
			return ( 1 );
	}
	/* Can't get here */
	return ( 1 );
}

PROTO static errr amiga_flush( int v )
{
	struct IntuiMessage *imsg;

	/* Ignore all messages at the port XXX */
	while ( imsg = (struct IntuiMessage *) GetMsg( data[ 0 ].win->UserPort ))
		ReplyMsg(( struct Message *) imsg );

	return ( 1 );
}

PROTO static void process_msg(int i,ULONG iclass, UWORD icode, UWORD iqual, APTR iaddr)
{
	struct Window *win;
	struct IntuiMessage *imsg;
	int nw,nh;

	double ud,tmpa,tmpb;
	term *old_term = Term;

	switch( iclass )
	{
		case IDCMP_GADGETUP:
			if (((struct Gadget *)iaddr)->GadgetID == 0)
			{
				// maxbody * nh ) / max_term_vert
				// vb * max term / maxbody
				ud = (double)data[ i ].ygadinfo.VertPot / (double)0xFFFF;
				ud *= (MAX_TERM_VERT - data[ i ].rows);
//				if (data[i].rows >= MAX_TERM_VERT)
//					puts("sdsdsdsj dhsjd shjd");
				tmpa = modf(ud,&tmpb);
				if (tmpa >= 0.5)
					ud = ceil(ud);
				else
					ud = floor(ud);
//				printf("%d %f\n",data[ i ].ygadinfo.VertPot,ud);
				data[ i ].ypos = ud;

				Term_activate(angband_term[i]);
            Term_redraw();
            Term_fresh();
				Term_activate(old_term);
//				do_cmd_redraw();
			}
			break;
		case IDCMP_INACTIVEWINDOW:
		case IDCMP_ACTIVEWINDOW:
			break;
		case IDCMP_CLOSEWINDOW:
			// Respond to any messages before closing
			win = data[i].win;
			while (imsg = (struct IntuiMessage *)GetMsg( win->UserPort ))
				ReplyMsg(( struct Message *) imsg );
			close_term(i);
			calc_sigmask();
			break;
		case IDCMP_RAWKEY:
			handle_rawkey( icode, iqual, iaddr );

			if (do_after == 'h' || do_after == 'H')
				amiga_hs_to_ascii();
			else if (do_after == 'm' || do_after == 'M')
	         amiga_map();
			else if (do_after == 'r' || do_after == 'R')
				amiga_redefine_colours();
			else if (do_after == 's' || do_after == 'S')
				amiga_save_palette();
			else if (do_after == 'l' || do_after == 'L')
				amiga_load_palette();
			break;
		case IDCMP_MOUSEMOVE:
		case IDCMP_MOUSEBUTTONS:
		case IDCMP_MENUVERIFY:
			if ( blankmouse && !pointer_visible )
			{
				ClearPointer( data[ i ].win );
				pointer_visible = TRUE;
			}
			break;
		case IDCMP_INTUITICKS:
			cursor_anim();
			break;
		case IDCMP_NEWSIZE:
			// Calculate new rows & cols.

			win = data[ i ].win;
			data[ i ].ww = (nw = win->Width / data[ i ].fw) * data[ i ].fw;
			data[ i ].wh = (nh = win->Height / data[ i ].fh) * data[ i ].fh;

			data[ i ].wx = win->LeftEdge;
			data[ i ].wy = win->TopEdge;
			nw = (data[i].ww - win->BorderRight - win->BorderLeft) / data[i].fw;
			nh = (data[i].wh - win->BorderTop - win->BorderBottom) / data[i].fh;
			data[ i ].cols = nw;
			data[ i ].rows = nh;

			// Don`t let user have huuuge windows
			if (nh > MAX_TERM_VERT)
				nh = MAX_TERM_VERT;
			if (nw > MAX_TERM_HORIZ)
				nw = MAX_TERM_HORIZ;

			if (KICK20)
			{
				ULONG f;
            UWORD temp;

				// maxbody * nh ) / max_term_vert
				f = (ULONG)MAXBODY * nh;
				f /= MAX_TERM_VERT;
				temp = (UWORD)f;
				ChangeWindowBox(win, data[ i ].wx, data[ i ].wy, data[ i ].ww, data[ i ].wh);

				// Update any gadgets inside window
				if (data[ i ].scroll)
				{
					NewModifyProp(&data[ i ].ygad, win, NULL,AUTOKNOB | FREEVERT | PROPNEWLOOK,
							data[i].ygadinfo.HorizPot,
							0,
							data[i].ygadinfo.HorizBody,
							temp,
							1);
				}
				data[ i ].ypos = 0;
				data[ i ].xpos = 0;
			}
			Term_activate(angband_term[ i ]);

			// If window scrolling is on, then we always want `band to draw
			// everything, and we do our own clipping. Otherwise, set the window
			// to the correct size and let `band handle it.

			if (data[ i ].scroll)
				Term_resize(80,24);
			else
				Term_resize(nw,nh);

			Term_redraw();
			Term_fresh();

			Term_activate(angband_term[ 0 ]);

			// Eat the IDCMP_NEWSIZE event coming from ChangeWindowBox(). A hack.
			while (imsg = (struct IntuiMessage *)GetMsg( win->UserPort ))
				ReplyMsg(( struct Message *) imsg );
			break;

		case IDCMP_MENUPICK:
			handle_menupick( icode );
         break;
	}
}

PROTO static void calc_sigmask(void)
{
	struct Window *win;
	int i;

	for (sigmask = i = 0 ; i < MAX_TERM_DATA ; i++)
	{
		if (win = data[i].win)
			sigmask |= (1 << win->UserPort->mp_SigBit);
	}
}

PROTO errr amiga_event( int v )
{
	BOOL messages;
	struct IntuiMessage *imsg;
	ULONG iclass;
	UWORD icode;
	UWORD iqual;
	APTR iaddr;
	int i;

	// Create mask for Wait(), as we want to watch all of our windows
	// XXX XXX XXX

	if (!sigmask)
		calc_sigmask();

	// First, respond to any existing messages
	// Ought to replace with something more efficient
   do
	{
		messages = FALSE;
		for (i = 0 ; i < MAX_TERM_DATA ; i++)
		{
			if (!data[i].win)
				continue;
			imsg = (struct IntuiMessage *)GetMsg( data[i].win->UserPort );
			if (imsg)
			{
				// We'll try and reply fairly quickly to keep Intuition happy
				iclass = imsg->Class;
				icode = imsg->Code;
				iqual = imsg->Qualifier;
				iaddr = imsg->IAddress;

				if ( iclass == IDCMP_MENUVERIFY && icode == MENUHOT && use_menus )
					update_menus();

				ReplyMsg(( struct Message *) imsg );
				process_msg(i,iclass,icode,iqual,iaddr);
				messages = TRUE;
			}
		}
	} while (messages);

	// Wait for an event if caller wants us to
	if (v)
   {
		Wait(sigmask);

		// Ought to handle messages here
		return 0;
	}
		else
	return 1;
}


PROTO static errr amiga_react( int v )
{
	/* Apply color palette, in case it has changed */
	load_palette();

	/* Create menus if we don't have any */
	if ( use_menus && !menu )
		create_menus();

	return ( 0 );
}

PROTO int amiga_tomb( void )
{
	cptr p;
	char tmp[160];
	time_t ct = time((time_t)0);
	BPTR file;

	char *pp;
	int plane, row, error = FALSE;
	struct BitMap *filebm, *scalbm, *convbm;
	long stdpens[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	int depth;

	int tw = data[ 0 ].fw * 64;
	int th = data[ 0 ].fh * 21;

	/* Allocate bitmap for tomb graphics file */
	filebm = alloc_bitmap( TOMW, TOMH, TOMB, BMF_CLEAR | BMF_STANDARD, data[ 0 ].rp->BitMap );
	if (!filebm)
		return( FALSE );

	/* Open tomb file */
	path_build( tmp , MAX_PATH_LENGTH , ANGBAND_DIR_XTRA , MTOM);
	file = Open( tmp , MODE_OLDFILE );
	if (!file)
	{
		free_bitmap( filebm );
		return( FALSE );
	}

	/* Read file into bitmap. This is OK on gfx cards. I guess */
	for ( plane = 0; plane < 4 && !error; plane++ )
	{
		pp = filebm->Planes[ plane ];
		for ( row = 0; row < 168 && !error; row++ )
		{
			error = ( Read( file, pp, 64 ) != 64 );
			pp += filebm->BytesPerRow;
		}
	}

	/* Close tomb file */
	Close( file );

	/* Get depth of display */
	depth = depth_of_bitmap( data[ 0 ].rp->BitMap );

	/* Remapping needed? */
	if ( TOMB <= depth && !use_pub )
	{
		/* Allocate bitmap for remapped image */
		convbm = alloc_bitmap( 512, 168, depth, BMF_CLEAR | BMF_STANDARD, data[ 0 ].rp->BitMap );
		if (!convbm)
		{
			free_bitmap( filebm );
			return( FALSE );
		}

		copy_bitmap(filebm, convbm, 512, 168, TOMB);
	}
	else
	{
		/* Remap old bitmap into new bitmap */
		remap_bitmap( filebm, convbm, use_pub ? pubpens : stdpens, 512, 168 );
	}

	free_bitmap( filebm );
	/* Allocate bitmap for scaled graphics */
	if (KICK20)
	{
		scalbm = alloc_bitmap( tw, th, depth, BMF_CLEAR | BMF_STANDARD, data[ 0 ].rp->BitMap );
		if (!scalbm)
		{
			free_bitmap( convbm );
			return ( FALSE );
		}

		/* Scale the tomb bitmap */
		scale_bitmap( convbm, 512, 168, scalbm, tw, th );

		/* Free old bitmap */
		free_bitmap( convbm );

		/* Copy the tomb graphics to the screen, centered */
		BltBitMapRastPort( scalbm, 0, 0, data[ 0 ].rp, ( data[ 0 ].ww - tw ) / 2, 0, tw, th, 0xc0 );

		/* Free bitmap */
		free_bitmap( scalbm );
	}
	else
		scalbm = convbm;

	/* King or Queen */
#ifdef ANG282
#ifdef SANGBAND
	if (p_ptr->total_winner)
#else
	if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
#endif
#else
	if (total_winner || (p_ptr->lev > PY_MAX_LEVEL))
#endif
	{
		p = "Magnificent";
	}

	/* Normal */
	else
	{
#ifdef SANGBAND
		p =  mp_ptr->title;
#else
		p = player_title[p_ptr->pclass][(p_ptr->lev-1)/5];
#endif
	}

	tomb_str( 3, " R.I.P." );

#ifdef ANG282
	tomb_str( 5, op_ptr->full_name );
#else
	tomb_str( 5, player_name );
#endif
	tomb_str( 6, "the" );

	tomb_str( 7, (char *)p );

/*	tomb_str( 9, (char *)cp_ptr->title ); */

#ifndef SANGBAND
	sprintf( tmp, "Level: %d", (int)p_ptr->lev );
	tomb_str( 10, tmp );
#endif

	sprintf( tmp, "Exp: %ld", (long)p_ptr->exp );
	tomb_str( 11, tmp );

	sprintf( tmp, "AU: %ld", (long)p_ptr->au );
	tomb_str( 12, tmp );

#ifdef ANG282
	sprintf( tmp, "Killed on Level %d", p_ptr->depth );
#else
	sprintf( tmp, "Killed on Level %d", dun_level );
#endif
	tomb_str( 13, tmp );

#ifdef ANG282
	sprintf( tmp, "by %s", p_ptr->died_from );
#else
	sprintf( tmp, "by %s", died_from );
#endif
	tomb_str( 14, tmp );

	sprintf( tmp, "%-.24s", ctime(&ct));
	tomb_str( 16, tmp );

	return( TRUE );
}

PROTO void tomb_str( int y, char *str )
{
	term_data *td = &data[ 0 ];

	int l = strlen( str );
	int xp = ( 39 * td->fw ) - (( l + 1 ) * td->fw ) / 2;

	SetDrMd( td->rp, JAM1 );

	SetAPen( td->rp, PEN( 1 ));
	Move( td->rp, xp + 1, y * td->fh + td->fb + 1 );
	Text( td->rp, str, l );

	SetAPen( td->rp, PEN( 0 ));
	Move( td->rp, xp, y * td->fh + td->fb );
	Text( td->rp, str, l );

	SetDrMd( td->rp, JAM2 );
}

PROTO void handle_rawkey( UWORD code, UWORD qual, APTR addr )
{
   BOOL shift;
	char buf[ 80 ];
	int i;
	int len;
	UWORD q;

	do_after = 0;
	/* Use a blank mouse-pointer on this window */
	if ( blankmouse && pointer_visible )
	{
		SetPointer( data[ 0 ].win, blankpointer, 2, 16, 0, 0 );
		pointer_visible = FALSE;
	}

	/* Handle cursor keys */

	if ( code > 0x4B && code < 0x50 )
   {
      shift = qual & ( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT );

		/* Cursor Up */
		if ( code == 0x4C )
			Term_keypress( shift ? '8' : '8' );

		/* Cursor Right */
		else if ( code == 0x4E )
			Term_keypress( shift ? '6' : '6' );

		/* Cursor Down */
		else if ( code == 0x4D )
         Term_keypress( shift ? '2' : '2' );

		/* Cursor Left */
		else
			Term_keypress( shift ? '4' : '4' );
		return;
	}

	/* Numeric keypad pressed with qualifier? */
	if (( qual & IEQUALIFIER_NUMERICPAD ) && ( qual & 0xff ))
	{
		/* Direction key? (1,2,3,4,6,7,8,9) */
		if (( code >= 0x1d && code <= 0x1f ) ||
			 ( code == 0x2d || code == 0x2f ) ||
			 ( code >= 0x3d && code <= 0x3f ))
		{
			/* Shift/Ctrl/Alt/Amiga keys */
			q = qual & 0xff;

			/* Shift + Direction */
			if ( q == IEQUALIFIER_LSHIFT || q == IEQUALIFIER_RSHIFT )
			{
				/* Fake a keypress 'run' */
				Term_keypress( '.' );

				/* Remove shift key from event */
				qual &= ~( IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT );
			}

			/* Alt + Direction */
			else if ( q == IEQUALIFIER_LALT || q == IEQUALIFIER_RALT )
			{
				/* Fake a keypress 'tunnel' */
				Term_keypress( 'T' );

				/* Remove alt key from event */
				qual &= ~( IEQUALIFIER_LALT | IEQUALIFIER_RALT );
			}

			/* Ctrl + Direction */
			else if ( q == IEQUALIFIER_CONTROL )
			{
				/* Fake a keypress 'open' */
				Term_keypress( 'o' );

				/* Remove ctrl key from event */
				qual &= ~IEQUALIFIER_CONTROL;
			}
		}
		/* Numeric '5' key */
		else if ( code == 0x2E )
		{
			/* Fake some 'search' commands */
			Term_keypress( 's' );
			Term_keypress( 's' );
			Term_keypress( 's' );
			Term_keypress( 's' );
      }
	}
	else if (( qual & IEQUALIFIER_RCOMMAND ) && (qual & 0xFF))
	{
		ie.ie_Class = IECLASS_RAWKEY;
		ie.ie_Code = code;
		ie.ie_Qualifier = qual;
		ie.ie_EventAddress = (APTR *) *((ULONG *) addr );
		len = RawKeyConvert( &ie, buf, 80, NULL );

		if (len)
			do_after = buf[0];
		return;
	}

	/* Convert raw keycode to ANSI sequence */
	ie.ie_Class = IECLASS_RAWKEY;
	ie.ie_Code = code;
	ie.ie_Qualifier = qual;
	ie.ie_EventAddress = (APTR *) *((ULONG *) addr );

	/* Removed 2.0+ .library method - no need for it now */
	len = RawKeyConvert( &ie, buf, 80, NULL );

	/* Send ANSI sequence to meta-terminal */
	for ( i = 0; i < len; i++ )
		Term_keypress( (unsigned char) buf[ i ]);
}

PROTO void handle_menupick( int mnum )
{
	struct MenuItem *item;
	ULONG ud;
	int i;

	/* Be sure to handle all selections */
	while ( mnum != MENUNULL )
	{
		/* Find address of menuitem */
		item = ItemAddress( menu, mnum );

		/* Find userdata of menuitem */
		ud = (ULONG) GTMENUITEM_USERDATA( item );

		/* Is this a window item? */
		if ( ud >= MNU_WINDOW )
		{
			i = ud - MNU_WINDOW;
			if ( item->Flags & CHECKED )
				open_term( i, TRUE );
         else
				close_term( i );
		}
		/* Is this a help item? */
		else if ( ud >= MNU_HELP )
		{
			/* Send keypresses */
			Term_keypress( '\\' );
			Term_keypress( '?' );
			Term_keypress( ud - MNU_HELP );
		}

		/* Control key shortcuts */
		else if ( ud >= MNU_CKEYCOM )
		{
			/* Send keycode */
			Term_keypress( '\\' );
			Term_keypress( '^' );
			Term_keypress( ud - MNU_CKEYCOM );
		}

		/* Key shortcuts */
		else if ( ud >= MNU_KEYCOM )
		{
			/* Key code */
			i = ud - MNU_KEYCOM;

			/* Some functions need underlying keymap */
			if ( i != 't' && i != 'w' && i != 'T' )
			{
				Term_keypress( '\\' );
			}

			/* Send keycode */
			Term_keypress( i );
		}
		/* Amiga palette requester */
		else
		{
			switch( ud )
			{
				case MNU_SAVE_WINDOWS:
					amiga_save_file();
					break;
				case MNU_PALETTE:
					amiga_redefine_colours();
					break;
				case MNU_SAVE_PALETTE:
					amiga_save_palette();
					break;
				case MNU_LOAD_PALETTE:
					amiga_load_palette();
					break;
				case MNU_SCALEDMAP:
					amiga_map();
					break;
				case MNU_EXPORT_HS:
					amiga_hs_to_ascii();
					break;
#ifdef GFXFUNCS
				case MNU_GFXMAP:
					amiga_gfxmap();
					break;
#endif
				default:
					printf("handle_menupick() : bad menu item %d\n",ud);
					break;
         }
		}

		/* Find next menunumber */
		mnum = item->NextSelect;
	}
}

/* Write settings back to settings.prf. This is another hack, but I can`t
   think of a really nice way to do this. */

PROTO static void amiga_save_file( void )
{
	char fname[ MAX_PATH_LENGTH ];
	char buf[ 256 ];
	ULONG fsrc,fdst;
	int i,k;
	FILE *f,*fh;

	fh = fopen("ram:temp","w");
	if (!fh)
		return;

	path_build( fname , MAX_PATH_LENGTH , ANGBAND_DIR_USER , WPRF);
	f = fopen( fname , "r" );
	if (!f)
	{
		amiga_show("Can't open settings.prf");
		fclose(fh);
		return;
	}

	while (1)
	{
		BOOL matched = 1;
		if (!fgets(buf,200,f))
			break;
		i = 0;
		while (buf[i] == 32 || buf[i] == 9)
			i++;
		if (buf[i] == '\n')
         matched = 0;
		for (k = 0 ; term_short[k] ; k++)
		{
			if (!strnicmp(buf + i,term_short[k],strlen(term_short[k])))
				matched = 0;
		}
		if (matched)
			fprintf(fh,"%s",buf);
	}

	for (i = 0 ; i < MAX_TERM_DATA ; i++)
	{
		if (data[i].win)
		{
			data[i].wx = data[i].win->LeftEdge;
			data[i].wy = data[i].win->TopEdge;
		}
		fprintf(fh,"%s.use %c\n",term_short[i],get_bool(data[i].use));
		fprintf(fh,"%s.show %c\n",term_short[i],get_bool(!data[i].iconified));
		fprintf(fh,"%s.scroll %c\n",term_short[i],get_bool(data[i].scroll));
		fprintf(fh,"%s.title %s\n",term_short[i],data[i].name);
		fprintf(fh,"%s.font %s\n",term_short[i],data[i].fontname);
		fprintf(fh,"%s.xpos %d\n",term_short[i],data[i].wx);
		fprintf(fh,"%s.ypos %d\n",term_short[i],data[i].wy);
		fprintf(fh,"%s.cols %d\n",term_short[i],data[i].cols);
		fprintf(fh,"%s.rows %d\n",term_short[i],data[i].rows);
		fprintf(fh,"\n");
	}

   fclose(f);
	fclose(fh);

	// Copy temp file to other one.
	fsrc = (ULONG)Open("ram:temp",1005);
	if (!fsrc)
	{
		amiga_show("Can't open temporary file");
		return;
	}
	fdst = (ULONG)Open(fname,1006);
	if (!fdst)
	{
		amiga_show("Can't write to settings.prf");
		Close(fsrc);
		return;
	}
	i = 128;
	while (i == 128)
	{
		i = Read(fsrc,buf,128);
		if (i)
			Write(fdst,buf,i);
   }
	Close(fsrc);
	Close(fdst);
	remove("ram:temp");
}

PROTO static char get_bool( BOOL opt )
{
	if (opt)
		return 'Y';
	else
		return 'N';
}

PROTO static void cursor_on( term_data *td )
{
	int x0, y0, x1, y1;

	if ( !td->cursor_lit && !td->iconified )
	{
		td->cursor_frame = 0;

		/* Hack - Don't draw cursor at (0,0) */
		if ( !td->cursor_xpos && !td->cursor_ypos )
			return;

		/* Draw an outlined cursor */
		if( CUR_A & 0xf0 && use_graphics )
		{
			x0 = td->cursor_xpos * td->fw;
			y0 = td->cursor_ypos * td->fh;
			x1 = x0 + td->fw - 1;
			y1 = y0 + td->fh - 1;
			SetAPen( td->wrp, PEN( CURSOR_PEN ));
			Move( td->wrp, x0, y0 );
			Draw( td->wrp, x1, y0 );
			Draw( td->wrp, x1, y1 );
			Draw( td->wrp, x0, y1 );
			Draw( td->wrp, x0, y0 );
		}

		/* Draw a filled cursor */
		else
		{
			SetBPen( td->wrp, PEN( CURSOR_PEN ));
			SetAPen( td->wrp, PEN( CURSOR_PEN ));
			RectFill(td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos, td->fw * (td->cursor_xpos + 1) - 1, td->fh * (td->cursor_ypos + 1) - 1);
			SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
			Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
			Text( td->wrp, &CUR_C, 1 );
		}

		td->cursor_lit = TRUE;
	}
}

PROTO static void cursor_off( term_data *td )
{
	if ( td->cursor_lit && !td->iconified )
	{
		/* Restore graphics under cursor */
		if ( CUR_A & 0xf0 && use_graphics )
		{
			put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, CUR_C, CUR_A );
		}

		/* Restore char/attr under cursor */
		else
		{
			if (td->background)
			{
				BltBitMapRastPort( td->background, td->cursor_xpos * td->fw, td->cursor_ypos * td->fh, td->wrp, td->cursor_xpos * td->fw, td->cursor_ypos * td->fh, td->fw,td->fh, 0xC0);
				SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
				SetBPen( td->wrp, PEN( 0 ));
				Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
				Text( td->wrp, &CUR_C, 1 );
			}
			else
			{
				SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
				SetBPen( td->wrp, PEN( 0 ));
				Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
				Text( td->wrp, &CUR_C, 1 );
			}
		}
		td->cursor_lit = FALSE;
	}
}

PROTO static void cursor_anim( void )
{
	term_data *td = term_curs;
	int x0, y0, x1, y1;
#ifdef ANG282
	int i = p_ptr->px,j = p_ptr->py;
#else
   int i = px, j = py;
#endif

	if ( !term_curs || td->iconified || !td->wrp)
		return;

	/* Don't want cursor blit optimised */
	block_nasty_gfx = TRUE;

	td->cursor_frame = ++(td->cursor_frame) % 8;

	/* Small cursor on map */
	if ( td->cursor_map )
	{
		if ( td->cursor_frame & 2 )
		{
			SetAPen( td->wrp, PEN( CURSOR_PEN ));
			RectFill( td->wrp,
						 td->map_x + i * td->mpt_w,
						 td->map_y + j * td->mpt_h,
						 td->map_x + ( i + 1) * td->mpt_w - 1,
						 td->map_y + ( j + 1 ) * td->mpt_h - 1
					  );
		}
		else
			put_gfx_map( td, i, j, get_p_char(),get_p_attr() );
	}

	else if ( td->cursor_visible && !iconified )
	{
		/* Hack - Don't draw cursor at (0,0) */
		if ( td->cursor_xpos == 0 && td->cursor_ypos == 0 )
		{
			block_nasty_gfx = FALSE;
			return;
      }
		/* Draw an outlined cursor */
		if ( CUR_A & 0x80 && use_graphics )
		{
			/* First draw the tile under cursor */
			put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, CUR_C, CUR_A );

			if ( td->cursor_frame < 4 )
			{
				x0 = td->cursor_xpos * td->fw;
				y0 = td->cursor_ypos * td->fh;
				x1 = x0 + td->fw - 1;
				y1 = y0 + td->fh - 1;
				SetAPen( td->wrp, PEN( CURSOR_PEN ));
				Move( td->wrp, x0, y0 );
				Draw( td->wrp, x1, y0 );
				Draw( td->wrp, x1, y1 );
				Draw( td->wrp, x0, y1 );
				Draw( td->wrp, x0, y0 );
			}
		}

		/* Draw a filled cursor */
		else
		{
			SetBPen( td->wrp, PEN( CURSOR_PEN ));
			SetAPen( td->wrp, PEN( CURSOR_PEN ));
			RectFill(td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos, td->fw * (td->cursor_xpos + 1) - 1, td->fh * (td->cursor_ypos + 1) - 1);
			SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
			Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
			Text( td->wrp, &CUR_C, 1 );
		}
	}
	block_nasty_gfx = FALSE;
}

PROTO static int load_gfx( void )
{
	char tmp[MAX_PATH_LENGTH];
	term_data *ts = &data[ 0 ];
	BPTR file;
	char *p;
	int plane, row, error = FALSE;

	if (screen_enhanced)
	{
		GFXW = AB_GFXW;
		GFXH = AB_GFXH;
		GFXB = AB_GFXB;
	}
	else
	{
		GFXW = DF_GFXW;
		GFXH = DF_GFXH;
		GFXB = DF_GFXB;
	}
	/* Allocate bitmaps */
	ts->gfxbm = alloc_bitmap( GFXW, GFXH, GFXB, BMF_CLEAR | BMF_STANDARD, ts->rp->BitMap );
	if ( !ts->gfxbm )
		return( FALSE );

	ts->mskbm = alloc_bitmap( GFXW, GFXH, 1, BMF_CLEAR | BMF_STANDARD, ts->rp->BitMap );
	if ( !ts->mskbm )
	{
		free_bitmap( ts->gfxbm );
		return( FALSE);
	}

	/* Open file */
	path_build( tmp , MAX_PATH_LENGTH , ANGBAND_DIR_XTRA , screen_enhanced ? AB_MGFX : DE_MGFX );
	file = Open( tmp, MODE_OLDFILE );
	if (!file)
	{
		MSG( 0, 0, "Unable to open graphics file" );
		Delay( 100 );
		return ( FALSE );
	}

	MSG( 0, 0, "Loading graphics ." );

	/* Read file into bitmap */
	for ( plane = 0; plane < GFXB && !error; plane++ )
	{
		p = ts->gfxbm->Planes[ plane ];
		for ( row = 0; row < GFXH && !error; row++ )
		{
			error = ( Read( file, p, GFXW >> 3 ) != (GFXW >> 3) );
			p += ts->gfxbm->BytesPerRow;
		}
	}

	MSG( 0, 0, "Loading graphics .." );

//	if (screen_enhanced)
//	{
//		Close( file );
//		return;
//	}
	/* Read mask data into bitmap */
	p = ts->mskbm->Planes[ 0 ];
	for ( row = 0; row < GFXH && !error; row++ )
	{
		int i;
		error = ( Read( file, p, GFXW >> 3 ) != (GFXW >> 3) );
		if (screen_enhanced)
		{
			for (i = 0 ; i < (GFXW >> 3) ; i++)
			{
				*(p + i) = 255 - *(p + i);
			}
		}
		p += ts->mskbm->BytesPerRow;
	}

	MSG( 0, 0, "Loading graphics ..." );
	/* Close file */
	Close( file );

	/* Did we get any errors while reading? */
	if ( error )
	{
		MSG( 0, 0, "Error while reading graphics file" );
		Delay( 100 );
		return ( FALSE );
	}

	/* Success */
	return ( TRUE );
}

PROTO static int conv_gfx( void )
{
	term_data *ts = &data[ 0 ];
	struct BitMap *tmpbm;
	struct BitMap *sbm = ts->rp->BitMap;
	long stdpens[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	long mskpens[] = { 0, -1 };
	int depth;

	/* Get depth of display */
	depth = depth_of_bitmap( sbm );

	/* We don't want 16 or 24 bit displays (yet) */
	if ( depth > 32 )
	{
		MSG( 0, 1, "Sorry, max. 32 bit display supported." );
		Delay( 100 );
		return ( FALSE );
	}

	if (screen_foreign)
	{
		/* Use graphics card */

		tmpbm = alloc_bitmap( GFXW, GFXH, depth, BMF_CLEAR | BMF_STANDARD, sbm);
		if (!tmpbm)
		{
			MSG( 0, 1, "Can`t allocate a temporary bitmap." );
			Delay( 100 );
			return FALSE;
		}
		remap_bitmap( ts->gfxbm, tmpbm, use_pub ? pubpens : stdpens, GFXW, GFXH );
		free_bitmap( ts->gfxbm );
		ts->gfxbm = tmpbm;

		return TRUE;
	}
	else
	{
		/* Allocate new bitmap with screen's depth */
		if (( tmpbm = alloc_bitmap( GFXW, GFXH, depth, BMF_CLEAR | BMF_STANDARD, sbm )) == NULL )
		{
			MSG( 0, 1, "Unable to allocate temporary bitmap." );
			Delay( 100 );
			return ( FALSE );
		}

		/* Simple remapping, ie. from 'GFXB' planes to the same or more planes */
		if (GFXB <= depth && !use_pub)
			copy_bitmap(ts->gfxbm,tmpbm,GFXW,GFXH,GFXB);

		/* Complex remapping */
		else
		{
			/* Remap old bitmap into new bitmap */
			remap_bitmap( ts->gfxbm, tmpbm, use_pub ? pubpens : stdpens, GFXW, GFXH );
		}

		/* Free old bitmap */
		free_bitmap( ts->gfxbm );
		ts->gfxbm = tmpbm;

		/* Allocate new bitmap with screen's depth */
		if (( tmpbm = alloc_bitmap( GFXW, GFXH, depth, BMF_CLEAR | BMF_STANDARD, sbm )) == NULL )
		{
			MSG( 0, 1, "Unable to allocate temporary bitmap." );
			Delay( 100 );
			return ( FALSE );
		}

		/* Remap mask bitmap */
		if (GFXB <= depth && !use_pub)
			copy_bitmap(ts->mskbm,tmpbm,GFXW,GFXH,1);

		/* Complex remapping */
		else
		{
			/* Remap old bitmap into new bitmap */
			remap_bitmap( ts->mskbm, tmpbm, mskpens, GFXW, GFXH );
		}

		/* Free old bitmap */
		free_bitmap( ts->mskbm );
		ts->mskbm = tmpbm;
	}
	/* Done */
	return ( TRUE );
}

PROTO static void copy_bitmap(struct BitMap *src, struct BitMap *dst, int x, int y, int d)
{
	BYTE *dstp,*srcp;
	short plane,row,col;
	int z = x >> 3;

	for ( plane = 0; plane < depth_of_bitmap(src); plane++ )
	{
		if (plane >= d)
			srcp = src->Planes[ d ];
      else
			srcp = src->Planes[ plane ];

		dstp = dst->Planes[ plane ];
		for ( row = 0; row < y; row++ )
		{
			for ( col = 0; col < z; col++ )
				dstp[ col ] = srcp[ col ];

			srcp += src->BytesPerRow;
			dstp += dst->BytesPerRow;
		}
	}
}

PROTO static int size_gfx( term_data *td )
{
	term_data *ts = &data[ 0 ];
	int depth;
	struct BitMap *sbm = td->rp->BitMap;
	struct BitMap *tmpbm;

	int tilew = 8, tileh = 8;
/*
	DANGER DANGER!!

	This is now a big hack. I really must sort this out.
*/

	if (KICK13)
		return( TRUE );

	if (screen_enhanced)
		tilew = tileh = 16;

	/* Calculate tile bitmap dimensions */
	td->gfx_w = (GFXW / tilew) * td->fw;
	td->gfx_h = (GFXH / tileh) * td->fh;

	/* Calculate map bitmap dimensions */
#ifdef ANG282
	td->mpt_w = td->ww / DUNGEON_WID;
	td->mpt_h = td->wh / DUNGEON_HGT;
#else
	td->mpt_w = td->ww / MAX_WID;
	td->mpt_h = td->wh / MAX_HGT;
#endif
	td->map_w = td->mpt_w * 32;
	td->map_h = td->mpt_h * 32;

	/* Friend bitmap */
	if ( td->rp )
		sbm = td->rp->BitMap;

	/* Scale tile graphics into map size */
	depth = depth_of_bitmap( ts->gfxbm );
	if (( td->mapbm = alloc_bitmap( td->map_w, td->map_h, depth, BMF_CLEAR, sbm )) == NULL )
		return( FALSE );

	scale_bitmap( ts->gfxbm, GFXW, GFXH, td->mapbm, td->map_w, td->map_h );

	/* Scale tile graphics */
	depth = depth_of_bitmap( ts->gfxbm );
	if (( tmpbm = alloc_bitmap( td->gfx_w, td->gfx_h, depth, BMF_CLEAR, sbm )) == NULL )
		return( FALSE );
	scale_bitmap( ts->gfxbm, GFXW, GFXH, tmpbm, td->gfx_w, td->gfx_h );
	if ( td->gfxbm )
		free_bitmap( td->gfxbm );
	td->gfxbm = tmpbm;

	/* Scale tile mask */
	depth = depth_of_bitmap( ts->mskbm );
	if (( tmpbm = alloc_bitmap( td->gfx_w, td->gfx_h, depth, BMF_CLEAR, sbm )) == NULL )
		return( FALSE );
	scale_bitmap( ts->mskbm, GFXW, GFXH, tmpbm, td->gfx_w, td->gfx_h );
	if ( td->mskbm )
		free_bitmap( td->mskbm );
	td->mskbm = tmpbm;

	/* Success */
	return( TRUE );
}

PROTO static void put_gfx( struct RastPort *rp, int x, int y, int chr, int col )
{
	term_data *td = (term_data *)(Term->data);
	int fw = td->fw;
	int fh = td->fh;
	int x0 = x * fw;
	int y0 = y * fh;
	int x1 = x0 + fw - 1;
	int y1 = y0 + fh - 1;
	int a = col & 0x7F; // ((GFXH >> 3) - 1);
	int c = chr & 0x7F; //((GFXW >> 3) - 1);

	/* Paranoia */

	if (screen_enhanced)
	{
		x0 = x * 16;
		y0 = y * 16;
		x1 = x0 + 15;
		y1 = y0 + 15;
		fw = fh = 16;
	}

	if (( td->iconified ) || ( !rp ))
		return;

	if ((fw != 8) && (nasty_optimise_gfx))
	{
		printf("Sorry; can't use quick graphics option on this size screen\n");
		printf("Quick graphics turned off.\n\n");
		nasty_optimise_gfx = FALSE;
	}

	/* Just a black tile */
	if ( a == 0 && c == 0 )
	{
#ifdef QUICKGFX
		if (nasty_optimise_gfx && !block_nasty_gfx)
		{
			global_depth = td->gfxbm->Depth;
			dest_row = rp->BitMap->BytesPerRow;
			dest_plane_addr = (unsigned long *)&(td->rp->BitMap->Planes[0]);

			optimised_clear( x0, y0, PEN( 0 ));
		}
		else
#endif
		{
			SetAPen( rp, PEN( 0 ));
			RectFill( rp, x0, y0, x1, y1 );
		}
		return;
	}

	/* Remap player for race and class */
	if ( a == 12 && c == 0 )
	{
		a = get_p_attr();
		c = get_p_char();
	}

	/* Draw tile through mask */
	if ( use_mask || screen_enhanced) // col & 0x40 )
	{
		if (td->background)
			BltBitMapRastPort( td->background, x * td->fw, y * td->fh, td->rp, x0, y0, fw, fh, 0xC0);
		else
		{
//			SetAPen( rp, PEN(0) );
//			RectFill( rp, x0, y0, x1, y1 );
		}
		BltMaskBitMapRastPort( td->gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, (ABC|ANBC|ABNC), td->mskbm->Planes[ 0 ] );
	}

	/* Draw full tile */
	else
	{
		if (fw != 8)
			nasty_optimise_gfx = FALSE;

#ifdef QUICKGFX
		if (!block_nasty_gfx && nasty_optimise_gfx)
		{
			global_depth = td->gfxbm->Depth;
			dest_row = rp->BitMap->BytesPerRow;
			source_row = td->gfxbm->BytesPerRow;
			source_plane_addr = (unsigned long *)&(td->gfxbm->Planes[0]);
			dest_plane_addr = (unsigned long *)&(td->rp->BitMap->Planes[0]);

			optimised_put_gfx( x0, y0, a, c );
		}
		else
#endif
			BltBitMapRastPort( td->gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, 0xc0 );
	}
}

/* Be prepared for this to be called multiple times! */
PROTO static int amiga_fail( char *msg )
{
	int i;

	/* Print error message */
	if ( msg )
		 puts( msg );

	/* Free sound memory */
	free_sound();

	/* Unlock public screen */
	if ( publock )
	{
		UnlockPubScreen( NULL, pubscr );
		publock = FALSE;
	}

	/* Remove menu from window */
	if ( menu && data[ 0 ].win )
		ClearMenuStrip( data[ 0 ].win );

	/* Free menus */
	if ( menu )
	{
		FreeMenus( menu );
		menu = NULL;
	}

	if (blankpointer)
	{
		FreeMem( blankpointer, BLANKPOINTER_SIZE );
		blankpointer = NULL;
	}

	if (sound_name_desc)
	{
		free(sound_name_desc);
		sound_name_desc = NULL;
	}

	if (sound_data)
	{
		free(sound_data);
		sound_data = NULL;
	}

	/* Free term resources */
	for ( i = MAX_TERM_DATA; i-- > 0; )
		free_term( &data[ i ] );

	/* Free obtained pens */
	if ( pubscr && KICK30 )
	{
		for ( i = 0; i < 32; i++)
		{
			if ( pubpens[ i ] != -1 )
				ReleasePen( pubscr->ViewPort.ColorMap, pubpens[ i ]);
		}
	}

	/* Free visual info . Note paranoia */
	if ( visinfo && KICK30)
	{
		FreeVisualInfo( visinfo );
		visinfo = NULL;
	}

	/* Close intuition screen */
	if ( amiscr )
	{
		CloseScreen( amiscr );
		amiscr = NULL;
	}

	/* Close gadtools.library */
	if ( GadToolsBase )
	{
		CloseLibrary( GadToolsBase );
		GadToolsBase = NULL;
	}

	/* Close console.device */
	if ( ConsoleDev )
	{
		CloseDevice( (struct IORequest *)ConsoleDev );
		ConsoleDev = NULL;
	}

	/* Close diskfont.library */
	if ( DiskfontBase )
	{
		CloseLibrary( DiskfontBase );
		DiskfontBase = NULL;
	}

	/* Close reqtools.library */
	if ( ReqToolsBase )
   {
		CloseLibrary( (struct Library *)ReqToolsBase );
		ReqToolsBase = NULL;
	}

	/* Close asl.library */
	if ( AslBase )
   {
		CloseLibrary( AslBase );
		AslBase = NULL;
	}

	if ( IFFBase )
	{
		CloseLibrary( IFFBase );
		IFFBase = NULL;
	}

//	if ( IntuitionBase )
//	{
//		CloseLibrary( IntuitionBase );
//		IntuitionBase = NULL;
//	}

//	if (GfxBase)
//	{
//		CloseLibrary( GfxBase );
//		GfxBase = NULL;
//	}

	return( -1 );
}

PROTO static void amiga_map( void )
{
	term_data *td = &data[ 0 ];
	int i,j;
	byte ta,tc;
#ifdef ANG282
	int cur_wid = DUNGEON_WID,cur_hgt = DUNGEON_HGT;
#endif

	/* Only in graphics mode, and not on Kickstart1.3 */
	if ( !use_graphics || KICK13)
		return;

	/* Turn off cursor */
	if ( td->cursor_visible )
		cursor_off( td );

	/* Save screen */
	Term_save();

	/* Clear screen */
	Term_clear();
	Term_fresh();

	/* Calculate offset values */
	td->map_x = (( td->fw * 80 ) - ( td->mpt_w * cur_wid )) / 2;
	td->map_y = (( td->fh * 24 ) - ( td->mpt_h * cur_hgt )) / 2;

	if (td->map_x < 0)
		td->map_x = 0;
	if (td->map_y < 0)
		td->map_y = 0;

	/* Inefficient */
#ifdef QUICKGFX
	global_map_x = td->map_x;
	global_map_y = td->map_y;
	source_plane_addr = (unsigned long *)&(td->mapbm->Planes[0]);
	dest_plane_addr = (unsigned long *)&(td->rp->BitMap->Planes[0]);
	source_row = td->mapbm->BytesPerRow;
	global_depth = td->mapbm->Depth;
	dest_row = td->rp->BitMap->BytesPerRow;
#endif

	/* Draw all "interesting" features */
	for ( i = 0; i < cur_wid; i++ )
	{
		for ( j = 0; j < cur_hgt; j++ )
		{
			/* Get frame tile */
			if ( i==0 || i == cur_wid - 1 || j == 0 || j == cur_hgt - 1 )
			{
#ifdef ANG283
				ta = f_info[ 63 ].x_attr;
				tc = f_info[ 63 ].x_char;
#else
#ifdef ZANGBAND
				ta = f_info[ 63 ].x_attr;
				tc = f_info[ 63 ].x_char;
#else
				ta = f_info[ 63 ].z_attr;
				tc = f_info[ 63 ].z_char;
#endif
#endif
			}

			/* Get tile from cave table */
			else
			{
				map_info( j, i, &ta, (char *) &tc );
			}

			/* Ignore non-graphics */
			if ( ta & 0x80 )
			{
				ta = ta & ((GFXH >> 3) - 1);
				tc = tc & ((GFXW >> 3) - 1);

				/* Player XXX XXX XXX */
				if ( ta == 12 && tc == 0 )
				{
					ta = get_p_attr();
					tc = get_p_char();
				}

				/* Put the graphics to the screen */
				put_gfx_map( td, i, j, tc, ta );
			}
		}
	}

	/* Draw a small cursor now */
	td->cursor_map = TRUE;

	/* Wait for a keypress, flush key buffer */
	Term_inkey( &tc, TRUE, TRUE );
	Term_flush();

	/* Normal cursor again */
	td->cursor_map = FALSE;

	/* Restore screen */
	Term_clear();
	Term_fresh();
	Term_load();
	Term_fresh();

	/* Turn cursor back on */
	if ( td->cursor_visible )
		cursor_on( td );
}

PROTO void load_palette( void )
{
	int i;
	int n;

	if ( !amiscr )
		return;

	(screen_cols > 32) ? (n = 32) : (n = screen_cols);

	if ( KICK30 )
	{
		palette32[ 0 ] = n << 16;
		palette32[ n * 3 + 1 ] = 0;
		for ( i = 0; i < n; i++ )
		{
			palette32[ i * 3 + 1 ] = angband_color_table[ use_graphics ? i : i + 16 ][ 1 ] << 24;
			palette32[ i * 3 + 2 ] = angband_color_table[ use_graphics ? i : i + 16 ][ 2 ] << 24;
			palette32[ i * 3 + 3 ] = angband_color_table[ use_graphics ? i : i + 16 ][ 3 ] << 24;
		}
		LoadRGB32( &amiscr->ViewPort, palette32 );
	}
	else
	{
		for ( i = 0; i < n; i++ )
		{
			palette4[ i ] =  ( angband_color_table[ use_graphics ? i : i + 16 ][ 1 ] >> 4 ) << 8;
			palette4[ i ] |= ( angband_color_table[ use_graphics ? i : i + 16 ][ 2 ] >> 4 ) << 4;
			palette4[ i ] |= ( angband_color_table[ use_graphics ? i : i + 16 ][ 3 ] >> 4 );
		}
		LoadRGB4( &amiscr->ViewPort, palette4, n );
	}
	if (screen_enhanced)
	{
		char buffer[256];
		FILE *f;

		path_build( buffer, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, AB_MGFX_CMAP );
		f = fopen(buffer, "r");
		if (f)
		{
			long *a = (long *)palette256;
			if ( 1 == fread(palette256,1024,1,f))
			{
				for (i = 0 ; i < 256 ; i++)
				{
					SetRGB32( &amiscr->ViewPort, i, trans((*a & 0x00FF0000) >> 16),trans((*a & 0x0000FF00) >> 8),trans(*a & 0x000000FF));
					a++;
				}
			}
		}
		fclose(f);
		return;
	}
}

PROTO ULONG trans( byte g )
{
	ULONG h;

	h = (g << 8) | (g << 16) | (g << 24) | g;
	return h;
}

PROTO static void allocate_nearpens( void )
{
	int i,pen;

	if (KICK30)
	{
		for (i = 0 ; i < 16 ; i++)
		{
			int z,x;
			int r,g,b;
			ULONG d,maxd;

			x = penconv[i];
			pen = 0;
			for (z = 0 ; z < 256 ; z++)
			{
				r = (angband_color_table[x][1] - palette256[(z << 2) + 1]);
				g = (angband_color_table[x][2] - palette256[(z << 2) + 2]);
				b = (angband_color_table[x][3] - palette256[(z << 2) + 3]);
				d = r*r + g*g + b*b;
				if (!z)
					maxd = d;
				else
				{
					if (d < maxd)
               {
						pen = z;
						maxd = d;
					}
				}
			}
			if (pen != -1)
				penconv[i] = pen;
		}
	}
}

PROTO int create_menus( void )
{
	struct NewMenu *item = newmenu;
	int nmsize = sizeof ( struct NewMenu );
	int i;

	/* Option code deleted. It's just such a pain to do :( */

	/* Copy all post-items into array */
	for ( i = 0; post_item[ i ].nm_Type != 255; i++ )
		memcpy( item++, &post_item[ i ], nmsize );

	/* Actually create the menu structures */
  	menu = CreateMenus( newmenu, NULL );

	if ( menu )
	{
		/* Layout menus */
		if ( LayoutMenus( menu, visinfo, KICK30 ? GTMN_NewLookMenus : TAG_IGNORE, TRUE, TAG_END ))
		{
			/* Attach menu to window */
			SetMenuStrip( data[ 0 ].win , menu );
		}

		/* Free menus */
		else
		{
			FreeMenus( menu );
			menu = NULL;
		}
	}

	/* Success */
	return ( menu != NULL );
}

PROTO void update_menus( void )
{
	struct MenuItem *item;
	int i;

	/* Require a window and a menu */
	if ( !data[ 0].win || !menu )
      return;

	/* Detach the menu from the window */

	/* Enable/Disable the amiga map according to use_graphics */
	if ( item = ItemAddress( menu, FULLMENUNUM( 5, 7, 0 )))
		item->Flags = use_graphics ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;

	/* Enable/Disable the palette requester */
	if ( item = ItemAddress( menu, FULLMENUNUM( 5, 9, 0 )))
		item->Flags = amiga_palette ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;

	/* Enable/Disable and check window menu items according to use and iconified status */
	for ( i = 1; i < MAX_TERM_DATA; i++ )
	{
		if ( item = ItemAddress( menu, FULLMENUNUM( 7, ( i - 1 ), 0 )))
		{
			item->Flags = data[ i ].use ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;
			item->Flags = ( data[ i ].use && !data[ i ].iconified ) ? item->Flags | CHECKED : item->Flags & ~CHECKED;
		}
	}
	/* Attach menu to window again */
}

PROTO int init_sound( void )
{
	static char tmp[MAX_PATH_LENGTH];
	static char buf[MAX_PATH_LENGTH];
	static char line[256];
	struct AmiSound *snd;
	FILE *f;
	char *s = sound_name_desc;
	int i,j,k,slev;
	BOOL memory;

	path_build(buf, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, "cfg/sound.cfg");

	/* Look for .cfg file */
	f = fopen(buf,"r");
	if (!f)
	{
		puts("Can't find xtra/cfg/sound.cfg - sound support disabled");
		return (has_sound = use_sound = FALSE);
	}
	while (fgets( line, 200, f ))
	{
		for (i = strlen(line) - 1; i >= 0 ; i--)
		{
			if (line[i] == 10 || line[i] == 13)
				line[i] = 32;
		}
		for (i = 0; line[i] && line[i] <= 32 ; i++)
			;
		if (line[i] == '#' || line[i] == ';' || !line[i])
			continue;
		k = i;

		while (line[i] && line[i] != ':')
			i++;
		if (!line[i])
			continue;
		line[i] = 0;
		for (j = k ; j < i ; j++)
		{
			if (line[j] == 32 || line[j] == 9)
			{
				line[j] = 0;
				break;
			}
		}
		slev = -1;
		for (j = 1 ; j < SOUND_MAX ; j++)
		{
			if (strreq(angband_sound_name[j] , line + k))
			{
				slev = j;
				break;
			}
		}
		if (slev == -1)
			continue;
		i++;
		while (line[i] && (line[i] == 32 || line[i] == 9))
			i++;
		if (!line[i])
			continue;
		/* Remember sample name, if necessary */

		do
		{
			memory = TRUE;
			if (line[i] == '!')
			{
				memory = FALSE;
				i++;
			}
			/* Frankly, this whole thing is a hack. But I won't tell if you
				don't. */

			while (line[i] > 32)
				*s++ = line[i++];
			*s++ = 0;
			*s++ = slev;
			*s++ = memory;
			while (line[i] && (line[i] == 32 || line[i] == 9))
				i++;
			sounds_needed++;
		}
		while (line[i]);

	}
	fclose(f);

	path_build(buf, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, "sound");
	sound_data = snd = malloc( sizeof(struct AmiSound) * sounds_needed );
	if (!sound_data)
		return( has_sound = use_sound = FALSE );

	for (i = 0 ; i < SOUND_MAX ; i++)
		sound_ref[i][0] = (struct AmiSound *)0;

	s = sound_name_desc;
	for (i = 0 ; i < sounds_needed ; i++)
	{
		snd->Name = s;
		snd->Volume = 64;
		snd->Channel = 1;
		snd->Rate = 0;
		snd->Repeats = 1;
		snd->Memory = 1;
		snd->Address = NULL;
		while (*s++)
			;
		j = *s++;
		if (!*s++)
			snd->Memory = 0;
		sound_ref[j][0] = (struct AmiSound *)((int)sound_ref[j][0] + 1);
		sound_ref[j][ (int)sound_ref[j][0] ] = snd;
		if ((int)sound_ref[j][0] == 8)
		{
			puts("Too many sounds for one sound_event (8 is max)");
			sound_ref[j][0] = (struct AmiSound *)7;
		}

		if ( snd->Memory )
		{
			/* Construct filename */
			path_build(tmp, MAX_PATH_LENGTH, buf, snd->Name );

			/* Load the sample into memory */
			snd->Address = (struct SoundInfo *) PrepareSound( tmp );
		}

		snd++;
	}
	has_sound = use_sound = TRUE;

	return ( TRUE );
}

PROTO void free_sound( void )
{
	int i;
	struct AmiSound *snd;

	/* Stop all channels */
	StopSound( LEFT0 );
	StopSound( LEFT1 );
	StopSound( RIGHT0 );
	StopSound( RIGHT1 );

	snd = sound_data;

	/* Remove all sounds from memory */
	for ( i = 0; i < sounds_needed; i++ )
	{
		if ( snd->Address )
		{
			/* Remove the sound from memory */
			RemoveSound( snd->Address );

			/* Clear address field */
			snd->Address = NULL;
		}
		snd++;
	}

	/* Done */
	has_sound = FALSE;
	use_sound = FALSE;
}

PROTO static void play_sound( int v )
{
	struct AmiSound *snd;
	struct AmiSound *old_snd;
	int rate;
	int channel;
	int old,vnum;
	char buf[MAX_PATH_LENGTH];
	char tmp[MAX_PATH_LENGTH];

	if ( has_sound )
	{
		/* If no sounds are available for chosen event, skip */
		if ((int)sound_ref[v][0] == 0)
			return;
		if (v > SOUND_MAX)
			return;

		/* Just pick 1st sound available at the moment */
		snd = sound_ref[v][vnum = 1 + rand_int( (int)sound_ref[v][0] )];

//		printf("%d %d\n",vnum,(int)sound_ref[v][0] );

		/* Channel number */
		channel = snd->Channel;

//		printf("channel %d\n");
		/* Last sample played on channel */
		old = channel_last[ channel ];

		/* Sample Rate */
		rate = snd->Rate;

		/* Random rate on some sounds */
		if ( v == SOUND_HIT || v == SOUND_MISS )
			rate = rate - 50 + rand_int( 150 );

		/* Pointer to old sound data */
		old_snd = (old >= 0) ? (sound_ref[old][channel_num[channel]]) : NULL;

		if (old_snd)
		{
			/* Stop sound currently playing on this channel */
			StopSound( channel );

			/* Free old sample if required */
			if ( !old_snd->Memory && old_snd->Address && old != v )
			{
				/* Remove it from memory */
				RemoveSound( old_snd->Address );

				/* Clear address field */
				old_snd->Address = NULL;
			}
		}

		/* Load new sample into memory if required */
		if ( !snd->Memory && snd->Address == NULL )
		{
			/* Construct filename */
			path_build(buf, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, "sound");

			/* Construct filename */
			path_build(tmp, MAX_PATH_LENGTH, buf, snd->Name );

			/* Load the sample into memory */
			snd->Address = (struct SoundInfo *) PrepareSound( tmp );
		}

		/* Make sure the sample is loaded into memory */
		if ( snd->Address )
		{
			/* Start playing the sound */
			PlaySound( snd->Address, snd->Volume, channel, rate, snd->Repeats );
		}

		/* Store sample number */
		channel_last[ channel ] = v;
		channel_num[ channel ] = vnum;
	}
}

PROTO void put_gfx_map( term_data *td, int x, int y, int c, int a )
{
	if (( td->iconified ) || ( td->wrp == NULL ) || ( td->mapbm == NULL ))
		return;

#ifdef QUICKGFX
	if (nasty_optimise_gfx)
	{
		source_plane_addr = (unsigned long *)&(td->mapbm->Planes[0]);
		dest_plane_addr = (unsigned long *)&(td->wrp->BitMap->Planes[0]);
		source_row = td->mapbm->BytesPerRow;
		global_depth = td->mapbm->Depth;
	   dest_row = td->wrp->BitMap->BytesPerRow;

		optimised_bltmap(c,a,td->mpt_w,td->mpt_h,x,y);
		return;
	}
#endif

	BltBitMapRastPort(
		td->mapbm,
		c * td->mpt_w,
		a * td->mpt_h,
		td->wrp,
		td->map_x + x * td->mpt_w,
		td->map_y + y * td->mpt_h,
		td->mpt_w,
		td->mpt_h,
		0xC0
	);
}

PROTO struct BitMap *alloc_bitmap( int width, int height, int depth, ULONG flags, struct BitMap *friend )
{
	int p;
	struct BitMap *bitmap;
	unsigned char *bp;

	if ( KICK30 )
	{
		/* True-color */
		if ( depth > 8 )
			return ( AllocBitMap( width, height, depth, flags | BMF_MINPLANES, friend ));
		else
		{
			return(AllocBitMap( width, height, depth, flags, friend ));
		}
	}

	else
	{
		/* Allocate bitmap structure */
		if (( bitmap = AllocMem( sizeof( struct BitMap ), MEMF_PUBLIC | MEMF_CLEAR )))
		{
			InitBitMap( bitmap, depth, width, height );
			/* Allocate bitplanes */

			for ( p = 0; p < depth; p++ )
			{
				bp = AllocRaster( width, height );
				if ( !bp )
					break;
				bitmap->Planes[ p ] = bp;
			}

			/* Out of memory */
			if ( p != depth )
			{
				/* Free bitplanes */
				while ( --p >= 0 )
					FreeRaster( bitmap->Planes[ p ], width, height );

				/* Free bitmap structure */
				FreeMem( bitmap, sizeof( struct BitMap ));
				bitmap = NULL;
			}
		}
		return ( bitmap );
	}
}

PROTO void free_bitmap( struct BitMap *bitmap )
{
	int p;

	WaitBlit();

	if ( KICK30 )
		FreeBitMap( bitmap );
	else
	{
		/* Free bitplanes */
		for ( p = 0; p < bitmap->Depth; p++ )
			FreeRaster( bitmap->Planes[ p ], bitmap->BytesPerRow * 8, bitmap->Rows );

		/* Free bitmap structure */
		FreeMem( bitmap, sizeof( struct BitMap ));
	}
}

PROTO void scale_bitmap( struct BitMap *srcbm, int srcw, int srch, struct BitMap *dstbm, int dstw, int dsth )
{
	struct BitScaleArgs bsa;

	/* Scale bitmap */
	bsa.bsa_SrcBitMap   = srcbm;
	bsa.bsa_DestBitMap  = dstbm;
	bsa.bsa_SrcX		  = bsa.bsa_SrcY = 0;
	bsa.bsa_SrcWidth    = srcw;
	bsa.bsa_SrcHeight   = srch;
	bsa.bsa_DestX  	  = bsa.bsa_DestY = 0;
	bsa.bsa_XSrcFactor  = srcw;
	bsa.bsa_YSrcFactor  = srch;
	bsa.bsa_XDestFactor = dstw;
	bsa.bsa_YDestFactor = dsth;
	bsa.bsa_Flags  	  = 0;
	BitMapScale( &bsa );
}

PROTO void remap_bitmap( struct BitMap *srcbm, struct BitMap *dstbm, long *pens, int width, int height )
{
	int x,y,p,c,ox,lpr,sd,dd;
	int bm[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	UBYTE *sp[ 32 ];
	UBYTE *dp[ 32 ];
	ULONG ls[ 32 ];
	ULONG ld[ 32 ];
	ULONG mask;

	// Handle gfx cards. Use the AGA/ECS/OCS code if at all possible!

	if (screen_foreign)
	{
		struct RastPort mainrast,newrast;
		LONG colour;

		InitRastPort(&mainrast);
		InitRastPort(&newrast);
		mainrast.BitMap = srcbm;
		newrast.BitMap = dstbm;

		ox = 19;
		p = height / 10;
		for (y = 0 ; y < height ; y++)
		{
			for (x = 0 ; x < width ; x++)
			{
				colour = pens[ ReadPixel(&mainrast,x,y) ];
				SetAPen(&newrast,colour);
				WritePixel(&newrast,x,y);
			}
		}
		return;
	}

	/* Source bitplanes */
	sd = depth_of_bitmap( srcbm );
	for ( p = 0; p < sd; p++ )
		sp[ p ] = srcbm->Planes[ p ];

	/* Destination bitplanes */
	dd = depth_of_bitmap( dstbm );
	for ( p = 0; p < dd; p++ )
		dp[ p ] = dstbm->Planes[ p ];

	/* Number of longwords per row */
	lpr = width / 32;

	/* Convert graphics */
	for ( y = 0; y < height; y++ )
	{
		ox = 0;
		for ( x = 0 ; x < lpr; x++ )
		{
			/* Read source longwords */
			for ( p = 0; p < sd; p++ )
				ls[ p ] = *(ULONG *)( sp[ p ] + ox);

			/* Clear destination longwords */
			for ( p = 0; p < dd; ld[ p++ ] = 0 );

			/* Remap */
			for ( mask = 0x80000000; mask != 0; mask >>= 1)
			{
				/* Find color index */
				for ( p = c = 0; p < sd; p++ )
				{
					if ( ls[ p ] & mask )
						c |= bm[ p ];
				}

				/* Remap */
				c = pens[ c ];

				/* Update destination longwords */
				for ( p = 0; p < dd; p++ )
				{
					if ( c & bm[ p ] )
						ld[ p ] |= mask;
				}
			}

			/* Write destination longwords */
			for ( p = 0; p < dd; p++ )
				*(ULONG *)( dp[ p ] + ox ) = ld[ p ];

			/* Update offset */
			ox += 4;
		}

		/* Update pointers to get to next line */
		for ( p = 0; p < sd; sp[ p++ ] += srcbm->BytesPerRow );
		for ( p = 0; p < dd; dp[ p++ ] += dstbm->BytesPerRow );
	}
}

PROTO int depth_of_bitmap( struct BitMap *bm )
{
	if ( KICK30 )
		return ( (int)GetBitMapAttr( bm, BMA_DEPTH ) );
	else
		return (bm->Depth);
}

/* -------------------------------------------------------------------- */
/*  amiga_show( char *str )                                             */
/*                                                                      */
/*  Put message on screen, allow time for it to be read, then erase it  */
/* -------------------------------------------------------------------- */

PROTO void amiga_show( char *str )
{
	char *spaces = "                                                   ";

	amiga_text( 0, 0, strlen( str ), 1, str );
	Delay(80);
	amiga_text( 0, 0, strlen( spaces ), 1, spaces );
}

/*
	amiga_redefine_colours(void)

   Brings up Reqtools palette requester, extracts new colours and
    places in angband_color_table.
*/

PROTO void amiga_redefine_colours( void )
{
	term_data *ts = &data[ 0 ];
	struct BitMap *sbm = ts->rp->BitMap;
	int cols;
	int i;

	/* Paranoia */
	if (!amiga_palette || pubscr || !ReqToolsBase)
		return;

	/* Get number of colours on screen */
   cols = (1 << depth_of_bitmap(sbm));

	i = rtPaletteRequest("Redefine the colours",NULL,RT_Screen,amiscr,TAG_END);

	/* If colours not changed, say bye bye */
	if (i == -1)
		return;

	/* Hack values into angband_color_table */
	if (KICK30)
	{
		unsigned long *ctable,*c;

		c = ctable = (long *)AllocMem(cols << 4,0);
		if (!ctable)
			return;
		GetRGB32(amiscr->ViewPort.ColorMap,0,cols,ctable);
		for (i = 0 ; i < cols ; i++)
		{
			angband_color_table[use_graphics ? i : i + 16][0] = 1;
			angband_color_table[use_graphics ? i : i + 16][1] = *c++ >> 24;
			angband_color_table[use_graphics ? i : i + 16][2] = *c++ >> 24;
			angband_color_table[use_graphics ? i : i + 16][3] = *c++ >> 24;
		}
		FreeMem(ctable,cols << 4);
	}
		else
	{
		unsigned long w;

      for (i = 0 ; i < cols ; i++)
		{
			w = GetRGB4(amiscr->ViewPort.ColorMap,i);
			angband_color_table[use_graphics ? i : i + 16][0] = 1;
			angband_color_table[use_graphics ? i : i + 16][1] = (w & 0xF00) >> 4;
			angband_color_table[use_graphics ? i : i + 16][2] = (w & 0xF0);
			angband_color_table[use_graphics ? i : i + 16][3] = (w & 0xF) << 4;
		}
	}
}

/* -------------------------------------------------------------------- */
/*  amiga_makepath( path )                                              */
/*                                                                      */
/*  Attempt to pick a sensible path for Angband to load from. If we     */
/*  have OS2.0 or better, then we can use 'PROGDIR:' - we don't need an */
/*  explicit assign.                                                    */
/*                                                                      */
/*  Under OS1.3 (or if PROGDIR: is not available for some bizarre       */
/*  reason), we'll use a game-specific assign (ie. 'Zangband:')         */
/*                                                                      */
/*  This is patched into the 'main.c' file at the moment. 				*/
/* -------------------------------------------------------------------- */

PROTO void amiga_makepath( char *name )
{
	FILE *f;
   struct IntuitionBase *l;

	/* XXX XXX XXX Ugh. Nasty */
	l = (struct IntuitionBase *)OpenLibrary( "intuition.library", 0L);
	kick_ver = l->LibNode.lib_Version;
	CloseLibrary((struct Library *)l);

	/* If KS1.3, user will have to have assigned 'angband' manually */
	if (KICK13)
	{
		strcpy(name,VERPATH);
		return;
	}
	/* Use PROGDIR if available; check if progdir points to correct path */

	f = fopen("PROGDIR:/EDIT/f_info.txt","r");
	if (!f)
		f = fopen("PROGDIR:/DATA/f_info.raw","r");
	if (!f)
		f = fopen("/DATA/r_info.raw","r");
	if (!f)
		f = fopen("/EDIT/r_info.txt","r");
	if (f && KICK20)
	{
		char c;

		/* Found, so we'll use PROGDIR:/ as the path */
		fclose(f);

		NameFromLock(ParentDir(GetProgramDir()),name,500);
		c = name[strlen(name) - 1];
		if (c != '/' && c != ':')
			strcat(name,"/");
//		strcpy(name,"PROGDIR:/");
		return;
	}
	if (f)
		fclose(f);
   strcpy(name,VERPATH);
	return;
}

PROTO void amiga_save_palette( void )
{
	FILE *fff;
   int i;
	char buf[MAX_PATH_LENGTH];

	/* Build the filename */
 	path_build(buf, MAX_PATH_LENGTH, ANGBAND_DIR_USER, "colours.prf");

	/* Append to the file */
	fff = fopen(buf, "w");

	/* Failure */
	if (!fff)
	{
      amiga_show("Can't save palette!");
		return;
	}

	/* Start dumping */
	fprintf(fff, "\n\n");
	fprintf(fff, "# Color redefinitions\n\n");

	/* Dump colours */
	for (i = 0; i < 256; i++)
	{
		int kv = angband_color_table[i][0];
		int rv = angband_color_table[i][1];
		int gv = angband_color_table[i][2];
		int bv = angband_color_table[i][3];

		cptr name = "unknown";

		/* Skip non-entries */
		if (!kv && !rv && !gv && !bv)
			continue;

		/* Extract the color name */
		if (i < 16)
			name = color_names[i];

		/* Dump a comment */
		fprintf(fff, "# Color '%s'\n", name);

		/* Dump the monster attr/char info */
		fprintf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
				        i, kv, rv, gv, bv);
	}

	/* All done */
	fprintf(fff, "\n\n\n\n");

	/* Close */
	fclose(fff);
}

/* -------------------------------------------------------------------- */
/*  amiga_load_palette(void)                                            */
/*                                                                      */
/*  Loads the current Amiga palette... Used by the menu function        */
/*  (wait for it...) 'Load Palette'                                     */
/*                                                                      */
/*  Ought to be added in the Angband 'change colour' functions too.     */
/* -------------------------------------------------------------------- */

PROTO void amiga_load_palette( void )
{
	process_pref_file("colours.prf");
	Term_xtra(TERM_XTRA_REACT, 0);
	Term_redraw();
}

/* -------------------------------------------------------------------- */
/*  amiga_hs_to_ascii( void )                                           */
/*                                                                      */
/*  Dumps the highscore table to the 'apex/scores.txt' file. This is    */
/*  just *too* hacky to document right now.                             */
/* -------------------------------------------------------------------- */

PROTO static void amiga_hs_to_ascii(void)
{
	char filename[MAX_PATH_LENGTH];
	char destfile[MAX_PATH_LENGTH];
	char temp[200];
	struct high_score h;
	int i;
	FILE *f,*d;

	int pr, pc, clev, mlev, cdun, mdun;
	cptr user, gold, when, aged;

	path_build(filename,MAX_PATH_LENGTH,ANGBAND_DIR_APEX,"scores.raw");
	f = fopen(filename,"r");
	if (!f)
	{
		amiga_show("Can't open highscore file!");
		return;
	}

	path_build(destfile,MAX_PATH_LENGTH,ANGBAND_DIR_APEX,"scores.txt");
	d = fopen(destfile,"w");
	if (!d)
	{
      amiga_show("Can't open destination file!");
		fclose(f);
		return;
   }

	/* Print header, and underline it*/

	sprintf(temp,"Highscore file for %s",VERSION);
	fprintf(d,"%s\n",temp);
	temp[ i = strlen(temp) ] = 0;
	while (i)
		temp[--i] = '-';
	fprintf(d,"%s\n\n",temp);

	for (i = 0 ; i < MAX_HISCORES; i++)
	{
      if (!fread(&h,sizeof(h),1,f))
			break;

	/* Extract the race/class */
	pr = atoi(h.p_r);
	pc = atoi(h.p_c);

	/* Extract the level info */
	clev = atoi(h.cur_lev);
	mlev = atoi(h.max_lev);
	cdun = atoi(h.cur_dun);
	mdun = atoi(h.max_dun);

#ifdef KANGBAND
	ia = atoi(h.inside_special);    /* -KMW- */
#endif
	/* Hack -- extract the gold and such */
	for (user = h.uid; isspace(*user); user++) /* loop */;
	for (when = h.day; isspace(*when); when++) /* loop */;
	for (gold = h.gold; isspace(*gold); gold++) /* loop */;
	for (aged = h.turns; isspace(*aged); aged++) /* loop */;

	/* Dump some info */
#ifdef KANGBAND
	sprintf(temp, "%3d.%9s  %s the %s %s, Level %d",
	        i + 1, h.pts, h.who,
	        p_info[pr].title, class_info[pc].title,
	        clev);
#else
/*	sprintf(temp, "%3d.%9s  %s the %s %s",
	        i + 1, h.pts, h.who,
	        p_info[pr].title,magic_info[pc].title); */
	sprintf(temp, "%3d.%9s  %s the %s %s, Level %d",
	        i + 1, h.pts, h.who,
	        p_info[pr].title, class_info[pc].title,
	        clev);

#endif

	/* Dump the first line */
	fprintf(d, "%s\n",temp);

		sprintf(temp, "               Killed by %s on %s %d",
		    h.how, "Dungeon Level", cdun);

			if (!cdun) /* -KMW- */
				sprintf(temp, "               Killed by %s in the Town",
				    h.how);

	/* Append a "maximum level" */
	if (mdun > cdun) strcat(temp, format(" (Max %d)", mdun));

	/* Dump the info */
	fprintf(d, "%s\n",temp);

	/* And still another line of info */
	sprintf(temp,
	       "               (User %s, Date %s, Gold %s, Turn %s).",
			        user, when, gold, aged);

	fprintf(d, "%s\n\n",temp);

	}
	fclose(d);
	fclose(f);
}

/* -------------------------------------------------------------------- */
/*  amiga_user_name( char *buf, int id )                                */
/*                                                                      */
/*  Provides the name of the last used save file in 'buf'. If not       */
/*  available, sets buf to 'PLAYER'. 'id' is currently ignored.         */
/*  Uses the file 'data-ami.prf' (which must be available); see the     */
/*  function 'amiga_write_user_name()' below.                           */
/*                                                                      */
/*  Note this must be explicitly added to 'main.c' (equiv. to '-u')     */
/* -------------------------------------------------------------------- */

PROTO void amiga_user_name( char *buf, int id )
{
	char temp[MAX_PATH_LENGTH];
	char name[24];
	int i;
	FILE *f;

	/* Check if our data file exists; if not, return 'PLAYER' */
	path_build(temp,MAX_PATH_LENGTH,ANGBAND_DIR_USER,"data-ami.prf");
	f = fopen(temp,"r");
	if (!f)
	{
		strcpy(buf,"PLAYER");
		return;
	}
	/* Paranoia - if line not valid, return "PLAYER" */
	if (!fgets(name,24,f))
	{
		fclose(f);
		strcpy(buf,"PLAYER");
		return;
   }

	/* Kill white space */
   for (i = strlen(name) - 1; i && (name[i] <= 32) ; i--)
		name[i] = 0;

	strcpy(buf,name);
	fclose(f);
}

/* -------------------------------------------------------------------- */
/*  amiga_write_user_name( char *name )                                 */
/*                                                                      */
/*  Writes the 'name' argument to the file 'user/data-ami.prf'.         */
/*  Generally, the 'name' argument will be 'player_name' i.e. 'Gandalf' */
/*  This is used in the 'save_player()' routine (see save.c) to         */
/*  automagically load the last used player...                          */
/*                                                                      */
/*  Note that this has to be explicitly added to save.c                 */
/* -------------------------------------------------------------------- */

PROTO void amiga_write_user_name( char *name )
{
	char temp[MAX_PATH_LENGTH];
	FILE *f;

	path_build(temp,MAX_PATH_LENGTH,ANGBAND_DIR_USER,"data-ami.prf");
	f = fopen(temp,"w");
	if (!f)
		return;
	fprintf(f,"%s\n",name);
	fclose(f);
}

/* -------------------------------------------------------------------- */
/*  get_p_attr( void )                                                  */
/*                                                                      */
/*  Returns an integer representing the 'attr' attribute of the current */
/*  player graphic. This will usually vary depending upon player's      */
/*  chosen class.                                                       */
/* -------------------------------------------------------------------- */

PROTO static int get_p_attr( void )
{
#ifdef ZANGBAND
	return p_ptr->pclass + 36;
#else
#ifdef SANGBAND
	int pc = 1,pr = p_ptr->prace;
#else
	int pc = p_ptr->pclass,pr = p_ptr->prace;
#endif
	pc = pc % 6;
	pr = pr % 5;
	return((( pc * 10 + pr) >> 5) + 12);
#endif
}

/* -------------------------------------------------------------------- */
/*  get_p_char( void )                                                  */
/*                                                                      */
/*  Returns an integer representing the 'char' attribute of the current */
/*  player graphic. This will usually vary depending upon player's      */
/*  chosen class.                                                       */
/* -------------------------------------------------------------------- */

PROTO static int get_p_char( void )
{
#ifdef ZANGBAND
	return p_ptr->prace;
#else
#ifdef SANGBAND
	int pc = 1,pr = p_ptr->prace;
#else
	int pc = p_ptr->pclass,pr = p_ptr->prace;
#endif
	pc = pc % 6;
	pr = pr % 5;
	return(( pc * 10 + pr) & 0x1f );
#endif
}

PROTO void amiga_register(char *ourname)
{
	char tmp[500];
	char buf[200];
	char *p;
	FILE *f;
	BPTR templock;

	if (KICK13)
		return;

	/* Add ENVARC: stuff */

	if (templock = Lock("ENVARC:Angband",ACCESS_READ))
		UnLock(templock);
	else
	{
		templock = CreateDir("ENVARC:Angband");
		if (templock)
			UnLock(templock);
		else
			return;
	}

	if (getasn("ENVARC"))
	{
		f = fopen("ENVARC:Angband/AngMUI","r");
		if (!f)
		{
			f = fopen("ENVARC:Angband/AngMUI","w");
			if (!f)
				return;
		}
		while (fgets(tmp,500,f))
		{
			int z = strlen(tmp) - 1;

			if (tmp[z] == '\n')
				tmp[z] = 0;

			p = strstr(tmp,",");
			if (p) *p = 0;
			if (streq(tmp, VERSION))
			{
				fclose(f);
				return;
			}
		}
		fclose(f);
		f = fopen("ENVARC:Angband/AngMUI","a");
		if (f)
		{
			NameFromLock(GetProgramDir(),tmp,400);
			GetProgramName(buf,200);
			AddPart(tmp,buf,500);
			fprintf(f,"%s,%s,%s,%d\n",VERSION,ourname,tmp,SOUND_MAX);
			fclose(f);
		}
	}
}
