/*
	File			: main-ami.c

	Version			: 1.2 (6th January 2002)
	Angband			: 2.9.3+

	Purpose			: Amiga module for Angband with graphics and sound

	Author			: Mark Howson
	Email			: Mark.Howson@ntu.ac.uk

	Original Author		: Lars Haugseth
	Email                   : lars@polygnosis.com

	Current Form		: Bablos
	Email			: angband@blueyonder.co.uk
	WWW			: http://www.angband.pwp.blueyonder.co.uk
*/

/* Variant name and version */
#define VARIANT "Angband 2.9.6 alpha 3"

/* Main 'assign' needed. Kick2.0+ systems usually don't need it anyway */
#define VERPATH "Angband:"

#define CGXSUPPORT		/* Define for RTG support. Leave on */

#ifndef __CEXTRACT__
#include "angband.h"

#include "main.h"

#include "vers.h"

#ifndef __GNUC__
#undef byte					/* Prevents conflicts with dos.h */
#endif /* __GNUC__ */

#include "sound-ami.h"
#include <math.h>
#ifndef __GNUC__
#include <dos.h>
#endif /* __GNUC__ */
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
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>

/* To prevent warnings... */
#undef ID_BMHD
#undef ID_BODY
#undef ID_CAMG
#undef ID_CMAP
#undef ID_CRNG
#undef ID_ILBM

#include <graphics/gfxbase.h>
#include <graphics/modeid.h>
#include <graphics/scale.h>
#include <graphics/text.h>
#include <hardware/blit.h>
#include <libraries/asl.h>
#include <libraries/iff.h>
#include <libraries/gadtools.h>
#include <libraries/reqtools.h>
#include <proto/asl.h>
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/console.h>
#include <proto/gadtools.h>
#include <proto/reqtools.h>

#ifdef CGXSUPPORT
#include <cybergraphx/cybergraphics.h>
#ifdef __GNUC__
#	include <inline/cybergraphics.h>
#else /* __GNUC__ */
#  include <proto/cybergraphics.h>
#  include <pragmas/cybergraphics_pragmas.h>
#endif /* __GNUC__ */
#endif /* CGXSUPPORT */

#ifdef __GNUC__
#  define __near
#endif /* __GNUC__ */

#endif

#define MAX_TERM_DATA 8

/* Maximum length a filename (including a path) can reach. Somewhat arbitary */
#define MAX_PATH_LENGTH		160

/* How much memory to allocate for the blanked out mousepointer */
#define BLANKPOINTER_SIZE 128

#define KICK30 ((kick_ver) >= 39)		/* True if K3.0 or better */
#define KICK21 ((kick_ver) >= 38)		/* True if K2.1 or better */
#define KICK20 ((kick_ver) >= 36)		/* True if K2.0 or better */
#define KICK13 ((kick_ver) < 36)		/* True if K1.3 or worse */

#define PEN( p ) ( penconv[ p ] )			/* Pen number conversion */
#define GPEN( p ) ( use_pub ? pubpens[ p ] : p )	/* Graphics pen number conversion */
#define FAIL( str ) return ( amiga_fail( str ))		/* Failure */
#define MSG( x, y, txt ) amiga_text( x, y, strlen( txt ), 1, txt );

/* Char and attr under cursor */
#define CUR_A ( td->t.scr->a[ td->cursor_ypos ][ td->cursor_xpos ] )
#define CUR_C ( td->t.scr->c[ td->cursor_ypos ][ td->cursor_xpos ] )

#define CURSOR_PEN 4 			/* Colour to use for cursor */

#define MAX_TERM_VERT 24		/* Max num of lines in a term (y) */
#define MAX_TERM_HORIZ 80		/* Max num of chars in a term (x) */

#define AB_GFXW 640
#define AB_GFXH 960
#define AB_GFXB 8

#define DF_GFXW 256
#define DF_GFXH 792
#define DF_GFXB 5

/* Size of current bitmap...initialise by load_gfx() */
int GFXW, GFXH, GFXB;

#define TOMW 512
#define TOMH 168										/* Size of tombstone image */
#define TOMB 4

#define DE_MGFX "gfx/tiles.raw"			/*  8x8 tile image */
#define AB_MGFX "gfx/tiles256.raw"		/* 16x16 tile image */

#define DE_MGFX_CMAP "gfx/tiles.cmap"		/* Colour map for normal tiles */
#define AB_MGFX_CMAP "gfx/tiles256.cmap"	/* Colour map for AB tiles */
#define MTOM "gfx/tomb.raw"			/* Filename of tombstone image */
#define WPRF "settings.prf"			/* Preferences file */

/* DisplayID specified with option -m */
char modestr[ 256 ] = "";

static byte palette256[1024];

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
struct Library *CyberGfxBase = NULL;
struct Library *DataTypesBase = NULL;

struct Device *ConsoleDev = NULL;
struct Device *ConsoleDevice = NULL;

/* Data shared between terms */
typedef struct term_global
{
	byte *chunky_tile;
	byte *chunky_mask;

	struct BitMap *gfxbm;
	struct BitMap *mapbm;
	struct BitMap *mskbm;

	int colours_free;
	byte resources_freed;
} term_global;

/* Term data structure. *Really* needs sorting out */
typedef struct term_data
{
	term t;  					 /* Term structure */
	cptr name;  				 /* Name string, eg. title */

	short backw, backh;
	struct BitMap *background;		/* Bitmap of special background */
	struct Menu *menu;				/* Ptr to menu strip, or NULL */
	byte *bkgname;

	char fontname[64];		 /* Name of font, ie. 'topaz/8'. Used by Save Windows */

	struct Gadget ygad;      /* Used for window scrollbars */
	struct Gadget xgad;
	struct PropInfo ygadinfo,xgadinfo;
	struct Image ygadimage,xgadimage;

	BYTE use;					 /* Use this window */

	BYTE iconified;          /* Window is iconified ? */

	BYTE xpos;					 /* Position of data in window. Think of this as an x offset */
	BYTE ypos;					 /* Position of data (y) */
	BYTE scroll;				 /* Put scrollers in borders? */
	BYTE backdrop;				 /* TRUE for no window borders */
	BYTE autoscale;

	BYTE cols;  				 /* Number of columns */
	BYTE rows;  				 /* Number of rows */

	short wx;					 /* Window x-pos, in pixels */
	short wy;					 /* Window y-pos, in pixels */
	short ww;					 /* Window width */
	short wh;					 /* Window height */

	BYTE fix_w;
	BYTE fix_h;
	BYTE fw; 					 /* Font width */
	BYTE fh; 					 /* Font height */
	BYTE fb; 					 /* Font baseline */

	struct TextFont *font;   /* Font pointers */

	BYTE ownf;  				 /* Font is owned by this term */

	struct Window *win;  	 /* Window pointer */
	struct RastPort *wrp;	 /* RastPort of window */
	struct RastPort *rp; 	 /* RastPort of screen or window */

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

/* Term data for all windows */
static term_data data[ MAX_TERM_DATA ];
static term_global tglob;

bool use_mask = FALSE;
bool use_bkg = FALSE;

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
bool nasty_optimise_gfx = FALSE;

/* Can we display a palette requester? (ie. is screen type ok, reqtools etc. */
bool amiga_palette = FALSE;

/* We don't want to use this hack for the cursor, so we use the next variable
   to prevent that. If you have 'nasty_optimise_gfx' on when the menu's are
   being displayed, the flashing cursor will overwrite the menu on screen :(
*/
bool block_nasty_gfx = FALSE;

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

static BOOL use_aga = FALSE;
static BOOL use_cyber = FALSE;

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

static byte custpens[ 512 ];
static ULONG custpalette[ 512 * 3 + 4];

static LONG gfxpens[ 512 ];
static byte obtain_mask[ 512 ];

/* Default colour palette; 16 for text. Depends upon values of TERM_<colour>
   in defines.h */
static ULONG default_colours[ 16 ] =
{
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
static char ver[] = "\0$VER: "VARIANT"  "__BABLOSDATE__;

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
#define MNU_GRAPHICS_OFF	1006
#define MNU_GFXMAP			1007
#define MNU_SAVE_WINDOWS 	1008
#define MNU_GRAPHICS_8		1009
#define MNU_GRAPHICS_16		1010

#define MNU_WINDOW_FONT		1014
#define MNU_WINDOW_REDRAW	1015
#define MNU_WINDOW_TOGGLEBORDERS	1016

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

struct NewMenu menu_ptr[MENUMAX];

/* Menu for each of the term windows */
struct NewMenu window_menu[] =
{
	{ NM_TITLE, "Window", 0, 0, 0, 0 },
		{ NM_ITEM, "Font", "f", 0, 0, (void *)MNU_WINDOW_FONT },
		{ NM_ITEM, "Redraw", "r", 0, 0, (void *)MNU_WINDOW_REDRAW },
		{ NM_ITEM, "Toggle Borders", "b", 0, 0, (void *)MNU_WINDOW_TOGGLEBORDERS },

		{ NM_END, NULL, 0, 0, 0, 0 },
		{ 255, 0, 0, 0, 0, 0 }
};

/* Menu array */
static struct NewMenu newmenu[ MENUMAX ];

extern void map_info(int y, int x, byte *ap, char *cp, byte *tap, char *tcp);
extern void center_string( char *buf, cptr str );

static errr amiga_pict( int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp );

static int load_backpic ( term_data *t, char *name );
static BOOL get_screenmode ( char *modestr );
void amiga_open_libs ( void );
void open_term ( int n, bool doall );
void close_term ( int n );
static void init_term ( term_data *td );
static void link_term ( int i );
static void free_term ( term_data *td );
static BOOL strreq ( char *s, char *d );
static void request_font ( char *str );
static void request_mode ( char *str );
static char *token_line(char *s, char *p);
int read_prefs( void );
int read_menus( void );
static BOOL process_bool ( char *param );
static void process_gfx ( char *param );
static errr amiga_user ( int n );
static void amiga_nuke ( term *t );
static void amiga_open ( term *t );
static errr amiga_curs ( int x, int y );
static errr amiga_wipe ( int x, int y, int n );
static errr amiga_clear ( void );
static errr amiga_text ( int x, int y, int n, byte a, cptr s );
static errr amiga_xtra ( int n, int v );
static errr amiga_flush ( int v );
static void process_msg ( int i, ULONG iclass, UWORD icode, UWORD iqual, APTR iaddr );
static void calc_sigmask ( void );
errr amiga_event ( int v );
static errr amiga_react ( int v );
int amiga_tomb ( void );
void tomb_str ( int y, char *str );
void handle_rawkey ( UWORD code, UWORD qual, APTR addr );
void handle_menupick ( int mnum, int term );
static void amiga_save_file ( void );
static char get_bool ( BOOL opt );
static void cursor_on ( term_data *td );
static void cursor_off ( term_data *td );
static void cursor_anim ( void );
static int load_gfx ( void );
static int conv_gfx ( void );
static int size_gfx ( term_data *td );
static void put_gfx ( struct RastPort *rp, int x, int y, int chr, int col );
static int breakfunc(void);
static int amiga_fail ( char *msg );
static void amiga_map ( void );
static void free_pen(int pen);
static int abandon_pen(void);
static int alloc_pen(ULONG r, ULONG g, ULONG b);
void load_palette ( void );
static int read_enhanced_palette ( void );
static int read_normal_palette(void);
static char *handle_font(struct term_data *td, char *fontname);
ULONG trans ( byte g );
int create_menus ( void );
void update_menus ( void );
int init_sound ( void );
void free_sound ( void );
static void play_sound ( int v );
void put_gfx_map ( term_data *td, int x, int y, int c, int a );
struct BitMap *alloc_bitmap ( int width, int height, int depth, ULONG flags, struct BitMap *friend );
void free_bitmap ( struct BitMap *bitmap );
void scale_bitmap ( struct BitMap *srcbm, int srcw, int srch, struct BitMap *dstbm, int dstw, int dsth );
void remap_bitmap ( struct BitMap *srcbm, struct BitMap *dstbm, long *pens, int width, int height );
int depth_of_bitmap ( struct BitMap *bm );
void amiga_show ( char *str );
void amiga_redefine_colours ( void );
void amiga_makepath ( char *name );
void amiga_save_palette ( void );
void amiga_load_palette ( void );
void amiga_hs_to_ascii ( void );
void amiga_user_name ( char *buf );
void amiga_write_user_name ( char *name );
static int get_p_attr ( void );
static int get_p_char ( void );
static void init_default_palette(void);
static void amiga_gfx(int type);
static int find_menuitem(int *rmenu, int *item, void *ud);
static void quick_BltBitMapRastPort( struct BitMap *src, int x, int y, struct RastPort *rp, int dx, int dy, int dw, int dh, int mode);
static void quick_Text(struct RastPort *rp, int col, char *s, int n, int dx, int dy);
static int get_p_attr(void);
static int get_p_char(void);


const char help_ami[] = "Amiga module with graphics and sound";


errr init_ami(int argc, char **argv)
{
	int i;
	struct NewScreen new_scr;
	struct NewWindow new_win;
	struct DimensionInfo diminfo;
	int pw,ph,px,py,maxw,maxh,th,barh;
	int fsize;
	BOOL changed;

	term_data *ts = &data[ 0 ];
	term_data *tt = NULL;

	/* Unused parameters */
	(void)argc;
	(void)argv;

	/* Open Amiga libraries */
	amiga_open_libs();

#ifndef __GNUC__
	onbreak(breakfunc);
#endif

	/* Can't have palette requester if we don't have reqtools */
	if (ReqToolsBase)
		amiga_palette = TRUE;

	/* Ought to check the result of this, but if we don't have 128 bytes of
      chip memory left we're in all sorts of trouble... */
	blankpointer = AllocMem(BLANKPOINTER_SIZE,MEMF_CLEAR | MEMF_CHIP);

	/* See if we specified graphics | sound via command line */
	use_sound = arg_sound;
	use_graphics = arg_graphics;

	/* Initialise global data */
	tglob.resources_freed = FALSE;
	tglob.chunky_tile = NULL;
	tglob.chunky_mask = NULL;

	/* Ugh. XXX XXX XXX Clean up! */
	for (i = 0 ; i < 512 ; i++)
		custpens[i] = 0;

	for (i = 0 ; i < 256 ; i++)
		gfxpens[i] = -1;

	for (i = 0 ; i < 512 ; i++)
		obtain_mask[i] = 0;

	/* Initialise all terms */
	for ( i = 0; i < MAX_TERM_DATA; i++ )
		init_term( &data[ i ] );

	/* Always use the main term */
	ts->use = TRUE;
	ts->iconified = FALSE;

	/* We *must* have kickstart 34 or later, which should be no problem */

	if ( IntuitionBase->LibNode.lib_Version < 34 )
		FAIL( "Sorry, this program requires Kickstart 1.3 or later." );

	/* Read preferences file */
	read_prefs();

	/* Read menus */
	read_menus();

	/* XXX XXX XXX  Command line options have priority */
	if (arg_graphics)
		use_graphics = 1;
	if (arg_sound)
		use_sound = 1;

	arg_graphics = use_graphics;
	arg_sound = use_sound;

	init_default_palette();

	if (screen_enhanced)
	{
		if (!read_enhanced_palette())
			FAIL("Can't read 256 colour palette! Need file tiles256.cmap, 1024 bytes");
	}
	else if (use_graphics)
	{
		if (!read_normal_palette())
			FAIL("Can't read normal colour palette! Need file tiles.cmap, 128 bytes");
	}

	/* Initialize keyboard stuff */
	ie.ie_NextEvent = NULL;
	ie.ie_Class = IECLASS_RAWKEY;
	ie.ie_SubClass  = 0;

	/* Need gadtools.library to use menus */
	if ( !GadToolsBase )
		use_menus = FALSE;

	/* Search for prefered screenmode or public screen */
	if (KICK20 && strlen( modestr ) > 0 )
	{
		if (!get_screenmode( modestr ))
			FAIL("Display error.");
	}

	/* No extra term windows under K1.3 ever, unless someone really *wants* them */
	if (KICK13)
	{
		bool bad = FALSE;
		for (i = 1 ; i < MAX_TERM_DATA; i++)
		{
			if (!bad && data[ i ].use)
			{
				puts("Sorry... Extra windows not currently supported under KS1.3 (upgrade!)");
				bad = TRUE;
			}
			data[i].use = FALSE;
		}
	}

	/* Handle 'fix' attribute */
	for ( i = 0 ; i < MAX_TERM_DATA; i++)
	{
		if (data[i].fix_w)
			data[i].fw = data[i].fix_w;
		if (data[i].fix_h)
			data[i].fh = data[i].fix_h;
	}

	/* Calculate window dimensions */
	for ( i = 0 ; i < MAX_TERM_DATA; i++)
	{
		data[ i ].ww = data[ i ].fw * data[ i ].cols;
		data[ i ].wh = data[ i ].fh * data[ i ].rows;
	}

	/* Find a nice screenmode */
	if ( (scr_m == 0) && (KICK30) )
	{
		if (CyberGfxBase)
		{
			scr_m = BestCModeIDTags(
						CYBRBIDTG_NominalWidth, ts->ww,
						CYBRBIDTG_NominalHeight, ts->wh,
						TAG_END );
		}
			else
		scr_m = BestModeID(
						BIDTAG_NominalWidth, ts->ww,
						BIDTAG_NominalHeight, ts->wh,
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
			screen_depth = 4;

		/* If trying to use Adam Bolt tiles with < 256 col screen, silently push
			to 256 colours */
		if (screen_enhanced && screen_depth < 8)
			screen_depth = 8;

		/* We want to use 32+ colours with graphics */
		if (use_graphics)
		{
			if (KICK20 && screen_depth < (GFXB + 1))
			{
				/* Get dimension data for screenmode */
				if ( GetDisplayInfoData( NULL, (UBYTE *) &diminfo, sizeof( struct DimensionInfo ), DTAG_DIMS, scr_m ))
				{
					if ( diminfo.MaxDepth < GFXB + 1)
						screen_depth = GFXB + 1;
				}
			}
		}
		screen_cols = 1 << screen_depth;

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
//				 SA_Font, scrattr,
				 SA_Type, CUSTOMSCREEN,
				 SA_Title, "Angband Screen",
				 SA_ShowTitle, FALSE,
				 SA_Quiet, TRUE,
				 SA_Behind, TRUE,
				 SA_AutoScroll, TRUE,
				 SA_Interleaved, (KICK30) ? TRUE : FALSE,
				 TAG_END );
		}
		else
		{
			new_scr.LeftEdge = 0;
			new_scr.TopEdge = 0;
			new_scr.Width = screen_width;
			new_scr.Height = screen_height;
			new_scr.Depth = screen_depth;
			new_scr.DetailPen = 0;
			new_scr.BlockPen = 1;
			new_scr.ViewModes = HIRES;  /* XXX XXX XXX */
			new_scr.Type = CUSTOMSCREEN;
//			new_scr.Font = scrattr;
			new_scr.DefaultTitle = "Angband Screen";
			new_scr.Gadgets = NULL;
			new_scr.CustomBitMap = NULL;
			amiscr = OpenScreen( &new_scr );
		}

		if (!amiscr)
			FAIL( "Unable to open Amiga screen." );

#ifdef CGXSUPPORT
		if (use_cyber)
		{
			if (!IsCyberModeID( scr_m ))
				use_cyber = FALSE;
		}
#endif
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

		screen_width = pw;
		screen_height = ph;
	}

	/* We are using a public screen */
	else
	{
		/* Get depth */
		screen_depth = depth_of_bitmap(pubscr->RastPort.BitMap);
		if (screen_depth > 8)
			screen_depth = 8;

		screen_cols = 1 << screen_depth;

		use_aga = FALSE;

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

	/* Now we need to load the palette for the screen (text pens!) */
	init_default_palette();
	load_palette();

	/* Font autoscaling, which ought to be in a function... not enough time... */
	if (ts->autoscale)
	{
		struct TextAttr attr;
		int maxsize = 0;
		int our_max = 24;
		fsize = 3;

		while (fsize < our_max)
		{
			/* Make sure the font name ends with .font */
			if ( !strstr( ts->fontname, ".font" ))
				strcat( ts->fontname, ".font" );

			/* Set font attributes */
			attr.ta_Name  = ts->fontname;
			attr.ta_YSize = fsize;
			attr.ta_Style = FS_NORMAL;
			attr.ta_Flags = ( !strcmp( ts->fontname, "topaz.font" ) && ( fsize == 8 || fsize == 9 )) ?
					 FPF_ROMFONT : FPF_DISKFONT;

			/* Open font from disk */
			ts->font = OpenDiskFont( &attr );
			if (ts->font)
			{
				if (ts->font->tf_XSize * MAX_TERM_HORIZ <= pw &&
						ts->font->tf_YSize * MAX_TERM_VERT <= ph)
					maxsize = fsize;
				CloseFont(ts->font);
			}
			fsize++;
		}
		if (maxsize)
		{
//			printf("Accepted size %d\n",maxsize);
			attr.ta_YSize = maxsize;
			ts->font = OpenDiskFont( &attr );
			if (!ts->font)
				maxsize = 0;
			else
			{
				ts->ownf = TRUE;

				/* Set font dimensions */
				ts->fw = ts->font->tf_XSize;
				ts->fh = ts->font->tf_YSize;
				ts->fb = ts->font->tf_Baseline;

//				printf("Auto x %d y %d b %d\n", ts->fw, ts->fh, ts->fb);
				SetFont(ts->rp, ts->font);
			}
			/* Recalc window widths */
			for ( i = 0 ; i < MAX_TERM_DATA; i++)
			{
				data[ i ].ww = data[ i ].fw * data[ i ].cols;
				data[ i ].wh = data[ i ].fh * data[ i ].rows;
			}
		}
		if (!maxsize)
			FAIL("Autoscale failed!");
	}

	/* Check window bounds, else Intuition might a) sulk b) crash */
	for (i = 0 ; i < MAX_TERM_DATA; i++)
	{
		changed = FALSE;
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
			 WA_InnerWidth, (backdrop) ? screen_width : ts->ww,
			 WA_InnerHeight, (backdrop) ? screen_height : ts->wh,
//			 WA_InnerWidth, ts->ww,
//			 WA_InnerHeight, ts->wh,

			 use_pub ? WA_PubScreen : WA_CustomScreen, use_pub ? pubscr : amiscr,
			 WA_Backdrop, backdrop,
			 WA_Borderless, backdrop,
			 WA_GimmeZeroZero, !backdrop,
			 WA_DragBar, !backdrop && !ts->notitle,
			 WA_DepthGadget, !backdrop && !ts->notitle,
			 WA_NewLookMenus, TRUE,
			 backdrop ? TAG_IGNORE : WA_ScreenTitle, VARIANT,
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

	if ((IFFBase || DataTypesBase) && ts->bkgname)
		load_backpic(ts,ts->bkgname);

	/* Handle other terms */
	for ( i = 1; i < MAX_TERM_DATA; i++ )
	{
		/* Term pointer */
		tt = &data[ i ];

		/* Skip this term if iconified */
		if (!tt->use)
			continue;

		if ((IFFBase || DataTypesBase) && tt->bkgname)
			load_backpic(tt,tt->bkgname);

		if (tt->iconified)
			continue;

		if (KICK13)
			FAIL("Extra term windows not supported under 1.3");

		/* Load background picture, if possible */
		open_term(i,FALSE);
	}

	for ( i = MAX_TERM_DATA - 1; i >= 0 ; i--)
	{
		if ( data[ i ].use )
			link_term( i );
	}

	/* Bring main window to front */
	if ( !backdrop )
		WindowToFront( ts->win );

	/* Bring screen to front */
	ScreenToFront( use_pub ? pubscr : amiscr );

	amiga_clear();

	/* Load and convert graphics */
	if ( use_graphics )
	{
		MSG( 0, 0, "Loading graphics" );
		if ( !load_gfx() )
			FAIL( NULL );

		/* Scale the graphics to fit font sizes. AGA /may/ run out of memory here, but we'll ignore it. */
		if (use_graphics)
			size_gfx( &data[ 0 ] );

		MSG( 0, 1, "Remapping graphics" );
		if ( !conv_gfx() )
			FAIL( "Not enough memory to remap graphics." );
	}

	/* Load sound effects */
	if ( use_sound )
	{
		MSG( 0, 2, "Loading sound effects" );
		sound_name_desc = malloc(8000); /* Ugh :( */
		init_sound();
	}

	/* Success */
	return ( 0 );
}

static int load_backpic(term_data *t, char *name)
{
	IFFL_HANDLE iff;
	long pens[64];
	struct IFFL_BMHD *bmhd;
	ULONG cols;
	struct BitMap *bkg;

	UBYTE *colour_table;
	if (DataTypesBase)
	{
		Object *o;
		ULONG *cregs = NULL;
		struct dtFrameBox dtf = {NULL};
		struct FrameInfo fri = {NULL};
		struct gpLayout gpl;

		if (o = NewDTObject (name,
					DTA_SourceType, DTST_FILE,
					DTA_GroupID, GID_PICTURE,
					PDTA_Remap, FALSE,
					TAG_DONE))
		{
			dtf.MethodID = DTM_FRAMEBOX;
			dtf.dtf_FrameInfo = &fri;
			dtf.dtf_ContentsInfo = &fri;
			dtf.dtf_SizeFrameInfo = sizeof (struct FrameInfo);

			if (DoMethodA (o, (Msg)&dtf) && fri.fri_Dimensions.Depth)
			{
				int z = 0;

				gpl.MethodID = DTM_PROCLAYOUT;
				gpl.gpl_GInfo = NULL;
				gpl.gpl_Initial = 1;
				if (DoMethodA (o, (Msg)&gpl))
				{
					/* Get the object information */
					GetDTAttrs (o,
						PDTA_CRegs, &cregs,
						PDTA_NumColors, &cols,
						PDTA_BitMap, &bkg,
						TAG_DONE);

					t->backw = fri.fri_Dimensions.Width;
					t->backh = fri.fri_Dimensions.Height;

					/* Calculate how many colours we need */
					while (z < cols)
					{
						pens[z] = alloc_pen( 	cregs[z * 3] >> 24,
														cregs[z * 3 + 1] >> 24,
														cregs[z * 3 + 2] >> 24);
						z++;
					}
					t->background = alloc_bitmap( fri.fri_Dimensions.Width, fri.fri_Dimensions.Height, screen_depth, BMF_STANDARD, NULL );
					remap_bitmap( bkg, t->background, pens, fri.fri_Dimensions.Width, fri.fri_Dimensions.Height);
					DisposeDTObject(o);
				}
			}
		}
		return 1;
	}

	if (!IFFBase)
		return 0;

	if (iff = IFFL_OpenIFF(name, IFFL_MODE_READ) )
	{
		if (colour_table = IFFL_FindChunk(iff, ID_CMAP) )
		{
			ULONG *l = (ULONG *)(colour_table + 4);
			int r,g,b;
			int z = 0;

			/* Calculate how many colours we need */

			cols = *l / 3;
			colour_table += 8;
			while (z < cols)
			{
				/* Calculate colour... */
				r = *colour_table++; g = *colour_table ++ ; b = *colour_table++;

				pens[z] = alloc_pen( 	r,
												g,
												b);
				z++;
			}
		}
		if (bmhd = IFFL_GetBMHD( iff ) )
		{
			struct BitMap *bkg;

			bkg = alloc_bitmap( bmhd->w, bmhd->h, screen_depth, 0, NULL );
			t->background = alloc_bitmap( bmhd->w, bmhd->h, bmhd->nPlanes, BMF_STANDARD, NULL );
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
			t->backw = bmhd->w;
			t->backh = bmhd->h;
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

static void free_pen(int pen)
{
	if (use_pub && obtain_mask[pen])
	{
		while (obtain_mask[pen])
		{
			ReleasePen( pubscr->ViewPort.ColorMap, pen );
			obtain_mask[pen]--;
		}
	}
	else
		custpens[pen] = 0;
}

static int abandon_pen(void)
{
	int i;
	int sc;

	if (screen_cols > 512)
		sc = 512;
	else
		sc = screen_cols;

	if (use_pub)
	{
		int val = obtain_mask[sc - 1];
		int pval = 0;

		/* Find lowest value, and release first */
		for (i = sc - 1; i >= 0 ; i--)
		{
			if (obtain_mask[i] < val)
			{
				val = obtain_mask[i];
				pval = i;
			}
		}
		return pval;
	}
	else
	{
		int val = custpens[sc - 1];
		int pval = 0;

		for (i = sc - 1; i >= 0 ; i--)
		{
			if (custpens[i] < val)
			{
				val = custpens[i];
				pval = i;
			}
		}
		return pval;
	}
}

/* r, g, b in range 0..0xFF */
static int alloc_pen(ULONG r, ULONG g, ULONG b)
{
	ULONG mr, mg, mb;
	ULONG d, maxd;
	int pen, i;
	int sc;

	if (screen_cols > 512)
		sc = 512;
	else
		sc = screen_cols;

	if (use_pub)
	{
		pen = ObtainBestPen( pubscr->ViewPort.ColorMap,
									(r << 24) | 0xFFFFFF,
									(g << 24) | 0xFFFFFF,
									(b << 24) | 0xFFFFFF,
									OBP_Precision, PRECISION_EXACT );
		if (pen != -1)
			obtain_mask[pen]++;
		return pen;
	}
	else
	{
		/* If we can allocate a new pen, do so */
		for (i = 0 ; i < sc ; i++)
		{
			if (!custpens[i])
			{
				if (KICK30)
				{
					SetRGB32( &amiscr->ViewPort, i,
						(r << 24) | 0xFFFFFF,
						(g << 24) | 0xFFFFFF,
						(b << 24) | 0xFFFFFF);
				}
				else
				{
					SetRGB4( &amiscr->ViewPort, i,
						r >> 4,
						g >> 4,
						b >> 4);
				}
				custpalette[i * 3] = r;
				custpalette[i * 3 + 1] = g;
				custpalette[i * 3 + 2] = b;

				custpens[i]++;
				return i;
			}
		}
		/* No free pens, so search for pen with a near colour */
		for (i = pen = 0 ; i < sc ; i++)
		{
			mr = (custpalette[ i * 3 ] - r);
			mg = (custpalette[ i * 3 + 1] - g);
			mb = (custpalette[ i * 3 + 2] - b);
			d = mr*mr + mg*mg + mb*mb;

			if (!i)
				maxd = d;
			else
			{
				if (d < maxd)
				{
					pen = i;
					maxd = d;
				}
			}
		}
		return pen;
	}
}

/* Setup palette for text pens */
static void init_default_palette(void)
{
	int i;

	/* Initialize colour palette */
	for ( i = 0; i < 16; i++ )
	{
		/* If undefined, use default palette */
		if ( angband_color_table[ i ][ 0 ] == 0 )
		{
			angband_color_table[ i ][ 0 ] = 1;
			angband_color_table[ i ][ 1 ] = ( default_colours[ i ] & 0xff0000 ) >> 16;
			angband_color_table[ i ][ 2 ] = ( default_colours[ i ] & 0x00ff00 ) >> 8;
			angband_color_table[ i ][ 3 ] = ( default_colours[ i ] & 0x0000ff );
		}
	}
}

// TRUE if ok
static BOOL get_screenmode( char *modestr )
{
	scr_m = strtol( modestr, NULL, 0 );

	/* It was not a number, so treat it as a public screen name */
	if ( !scr_m)
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
/* -------------------------------------------------------------------- */

void amiga_open_libs( void )
{
	IntuitionBase = (struct IntuitionBase *)OpenLibrary( "intuition.library", 0L);
	DiskfontBase = OpenLibrary( "diskfont.library", 0L);
	GfxBase = (struct GfxBase *)OpenLibrary( "graphics.library", 0L);
	IFFBase = OpenLibrary("iff.library", 0L);
	DataTypesBase = OpenLibrary("datatypes.library", 0L);
	if (!DiskfontBase)
		amiga_fail( "Sorry, this program needs diskfont.library" );

	/* Decide which version of the system we have. Ought to use version.library */

	kick_ver = IntuitionBase->LibNode.lib_Version;
	AslBase = OpenLibrary( "asl.library", 36L);
	GadToolsBase = OpenLibrary( "gadtools.library", 36L);

	/* Initialise console (only using RawKeyConvert) */
	ConsoleDev = (struct Device *)OpenDevice("console.device",CONU_LIBRARY,(struct IORequest *)&io_req,0L);
	ConsoleDevice = (struct Device *)io_req.io_Device;
	CyberGfxBase = (struct Library *)OpenLibrary( "cybergraphics.library", 41L);

	/* Is this evil?? */
	if (CyberGfxBase)
		use_cyber = TRUE;

	ReqToolsBase = (struct ReqToolsBase *)OpenLibrary( "reqtools.library",0L);
}

/* Open a term window! (n = window number) */
void open_term( int n, bool doall )
{
	term_data *tt = &data[ n ];
	int i;
	int notitle;

	/* Skip this term if not in use */
	if ( doall && !tt->use )
		return;

	/* If already open, don't reopen */
	if ( tt->win )
		return;

	/* Initialise vertical prop gadget */
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

	notitle = tt->notitle;
	if (tt->backdrop)
		notitle = TRUE;

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
			WA_DragBar, !tt->backdrop,
			WA_Borderless, tt->backdrop,
			WA_SizeGadget, !tt->backdrop,
			WA_CloseGadget, !tt->backdrop,
			WA_DepthGadget, !tt->backdrop,
			WA_NewLookMenus, TRUE,
			WA_ScreenTitle, VARIANT,
			WA_IDCMP, IDCMP_NEWSIZE | IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK | IDCMP_MENUVERIFY,
			notitle ? TAG_IGNORE : WA_Title, tt->name,
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

	tt->menu = CreateMenus( window_menu, GTMN_FrontPen, (long)penconv[ TERM_WHITE ], NULL );

	if ( tt->menu )
	{
		/* Layout menus */
		if ( LayoutMenus( tt->menu, visinfo, KICK30 ? GTMN_NewLookMenus : TAG_IGNORE, TRUE, TAG_END ))
			SetMenuStrip( tt->win , tt->menu );
		else
		{
			FreeMenus( tt->menu );
			tt->menu = NULL;
		}
	}

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

/* Close a window (n = term number) */
void close_term( int n )
{
	term_data *tt = &data[ n ];

	/* Skip this term if not in use, or already closed */
	if ( !tt->use || tt->iconified)
		return;

	amiga_flush(1);

	/* Remove menustrip */
	if (tt->menu && tt->win)
		ClearMenuStrip( tt->win );

	/* Close window */
	if ( tt->win )
		CloseWindow( tt->win );

	tt->menu = NULL;
	tt->win = NULL;
	tt->wrp = tt->rp = NULL;

	/* Recalculate signal mask, as window has now disappeared */
	calc_sigmask();

	/* Term is now iconified */
	tt->iconified = TRUE;
}

/* Prepare a term */
static void init_term( term_data *td )
{
	td->name = "term_unknown";

	td->use = FALSE;
	td->iconified = FALSE;
	td->scroll = FALSE;
	td->autoscale = FALSE;
	td->backdrop = FALSE;

	/* Term size */
	td->xpos = td->ypos = 0;

	td->cols = 80;
	td->rows = 24;

	td->fix_w = td->fix_h = 0;

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

static void link_term( int i )
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
	t->higher_pict = TRUE;

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

static void free_term( term_data *td )
{
	/* Remove menus, if any */
	if ( td->menu )
	{
		ClearMenuStrip( td->win );
		FreeMenus( td->menu );
	}
	if ( td->background )
	{
		free_bitmap( td->background );
		td->background = NULL;
	}
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
}

static BOOL strreq( char *s, char *d)
{
	return((BOOL)!stricmp(s,d));
}

/* Get font name from user, return in str as 'topaz.font/8' or '' if cancel */
static void request_font( char *str )
{
	struct FontRequester *req = NULL;

	/* Blank string as default */
	*str = 0;

	/* Open ASL screenmode requester, if possible */
	if (AslBase)
	{
		if ( req = AllocAslRequestTags( ASL_FontRequest, TAG_DONE ))
		{
			if (amiscr || pubscr)
			{
				if ( AslRequestTags( req,
						ASLFO_FixedWidthOnly, TRUE,
						ASLFO_Screen, (use_pub ? pubscr : amiscr),
						TAG_DONE, TAG_DONE ))
					sprintf( str, "%s/%d", req->fo_Attr.ta_Name, req->fo_Attr.ta_YSize );
			}
			else
				if ( AslRequestTags( req,
						ASLFO_FixedWidthOnly, TRUE,
						TAG_DONE, TAG_DONE ))
					sprintf( str, "%s/%d", req->fo_Attr.ta_Name, req->fo_Attr.ta_YSize );
			FreeAslRequest( req );
		}
	}
}

static void request_mode( char *str )
{
	struct ScreenModeRequester *req;

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

/* Build menus */
int read_menus( void )
{
	static char fname[ MAX_PATH_LENGTH ];
	static char line[ 512 ];
	static char buf[ 512 ];
	struct NewMenu nm;
	int mn = 0;
	FILE *file;
	char *s;

	path_build(fname, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, "cfg/menu.cfg");
	file = fopen( fname , "r" );
	if (!file)
	{
		printf("\nUnable to open menu file 'xtra/cfg/menu.cfg'\n");
		return( FALSE );
	}

	while ( fgets( line, 511, file ))
	{
		s = line;

		line[strlen(line) - 1] = 0;

		if (*s == '#' || *s == ';')
			continue;

		s = token_line(s, buf);
		if (buf[0] == 'T')
		{
			s = token_line(s, buf);

			nm.nm_Type = NM_TITLE;
			nm.nm_Label = strdup(buf);
			nm.nm_CommKey = 0;
			nm.nm_Flags = 0;
			nm.nm_MutualExclude = 0;
			nm.nm_UserData = 0;
			memmove(&menu_ptr[mn++], &nm, sizeof(struct NewMenu));
		}
		else if (buf[0] == 'I')
		{
			char keycode[2];
			bool ctrl_key = FALSE;
			bool help_key = FALSE;
			bool ramiga_key = FALSE;
			int internal = 0;

			keycode[1] = 0;
			s = token_line(s, buf);

			nm.nm_Type = NM_ITEM;
			nm.nm_Label = strdup(buf);

			/* Parse key code */
			while (*s)
			{
				s = token_line(s, buf);

				if (!stricmp(buf, "dungeon_map"))
					internal = MNU_SCALEDMAP;
				else if (!stricmp(buf, "palette_requester"))
					internal = MNU_PALETTE;
				else if (!stricmp(buf, "load_palette"))
					internal = MNU_LOAD_PALETTE;
				else if (!stricmp(buf, "graphics_off"))
					internal = MNU_GRAPHICS_OFF;
				else if (!stricmp(buf, "graphics_8x8"))
					internal = MNU_GRAPHICS_8;
				else if (!stricmp(buf, "graphics_16x16"))
					internal = MNU_GRAPHICS_16;
				else if (!stricmp(buf, "save_palette"))
					internal = MNU_SAVE_PALETTE;
				else if (!stricmp(buf, "export_hs"))
					internal = MNU_EXPORT_HS;
				else if (!stricmp(buf, "CTRL"))
					ctrl_key = TRUE;
				else if (!stricmp(buf, "HELP"))
					help_key = TRUE;
				else if (!stricmp(buf, "RAMIGA"))
					ramiga_key = TRUE;
				else
					keycode[0] = buf[0];
			}

			nm.nm_Flags = 0;
			nm.nm_MutualExclude = 0;
			nm.nm_UserData = NULL;

			if (ramiga_key)
				nm.nm_CommKey = strdup(keycode);
			if (ctrl_key)
				nm.nm_UserData = (APTR) (MNU_CKEYCOM + keycode[0]);
			else if (help_key)
				nm.nm_UserData = (APTR) (MNU_HELP + keycode[0]);
			else
				nm.nm_UserData = (APTR) (MNU_KEYCOM + keycode[0]);

			if (internal)
				nm.nm_UserData = (APTR) internal;
			memmove(&menu_ptr[mn++], &nm, sizeof(struct NewMenu));
		}
		else if (buf[0] == 'B')
		{
			nm.nm_Type = NM_ITEM;
			nm.nm_Label = NM_BARLABEL;
			nm.nm_CommKey = 0;
			nm.nm_Flags = 0;
			nm.nm_MutualExclude = 0;
			nm.nm_UserData = 0;

			memmove(&menu_ptr[mn++], &nm, sizeof(struct NewMenu));
		}
	}

	nm.nm_Type = NM_END;
	nm.nm_Label = NULL;
	nm.nm_CommKey = 0;
	nm.nm_Flags = 0;
	nm.nm_MutualExclude = 0;
	nm.nm_UserData = 0;

	memmove(&menu_ptr[mn], &nm, sizeof(struct NewMenu));
	fclose(file);
}

static char *token_line(char *s, char *p)
{
	while (*s == 9 || *s == 32)
		s++;

	if (*s == '\"')
	{
		do
			*p++ = *++s;
		while (*s && *s != '\"');
		*(p - 1) = 0;
		s++;

		return s;
	}
	while (*s && *s != 32 && *s != 9)
		*p++ = *s++;

	*p = 0;
	return s;
}

/* Parse `settings.prf` file, if found. */
int read_prefs( void )
{
	static char errorstr[] = "PREFS:Unrecognised option";
	FILE *file;
	static char line[ 256 ];
	static char fname[ MAX_PATH_LENGTH ];
	char fontname[ 256 ];
	char custom_str[200];
	char public_str[200];
	char *first,*type,*param;
	int i,k;
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

		if (strreq(first,"ANGBAND"))
		{
			if (strreq(type,"gfx"))
				process_gfx(param);
			else if (strreq(type,"sound"))
				use_sound = process_bool(param);
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
				use_aga = process_bool(param);
			}
			else if (strreq(type,"aga"))
			{
				/* Only allow quick graphics on non-RTG systems! */
				use_aga = process_bool(param);
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
				if (screen_depth > 8)
					screen_depth = 8;
				screen_cols = 1 << screen_depth;
			}
			else if (strreq(type,"overscan"))
				screen_overscan = atoi(param);
			else if (strreq(type,"rtg"))
			{
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

		/* 'noborders' - set to TRUE for no window borders */
		else if ( strreq( type, "noborders"))
			td->backdrop = process_bool( param );

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
		else if ( strreq( type, "fix" ))
		{
			td->fix_h = td->fix_w = atoi(param);
		}
		/* Option 'name' - Set window title */
		else if ( strreq( type, "title" ))
		{
			if (param)
				td->name = strdup( param );
			else
				td->notitle = TRUE;
		}
		/* Option 'font' - Set font for this window */
		else if ( strreq( type, "font" ))
		{
			char *s;

			/* Get value */
			strcpy(td->fontname,param);
			if (param[0] == '?')
				request_font(fontname);
			else
				strcpy(fontname,param);

			s = handle_font(td, fontname);
			if (s)
				puts(s);
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
		strcpy(modestr,public_str);
	}
	else
		modestr[0] = 0;
	fclose( file );
}

static char *handle_font(struct term_data *td, char *fontname)
{
	static char error[128];
	struct TextAttr attr;
	char *s;
	int fsize;

	/* No font specification given, so use system font, or inherit 'main' font */
	if ( !fontname[0] )
	{
		if ( td == &data[ 0 ] )
		{
			td->font = GfxBase->DefaultFont;

			/* Use default font as screen font */
			scrattr = NULL;
		}
		else
			td->font = data[ 0 ].font;

		/* Set font dimensions */
		td->fw = td->font->tf_XSize;
		td->fh = td->font->tf_YSize;
		td->fb = td->font->tf_Baseline;

		/* This font is not opened by us */
		td->ownf = FALSE;

		return NULL;
	}
		else
	{
		/* Find font name/size delimiter */
		if (( s = strchr( fontname, '/' )) == NULL )
		{
			sprintf(error,"PREFS: Illegal font specification: '%s'.\n", fontname );
			return error;
		}

		/* Now get size of font */
		*s++ = 0;

		/* Check for autoscaling */
		if (*s == '?')
		{
			strcpy(td->fontname, fontname);
			td->autoscale = TRUE;
			return NULL;
		}
		else
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

//			printf("Font x %d y %d b %d\n",td->fw, td->fh, td->fb);
			/* Copy font attr to screen font */
			if ( td == &data[ 0 ] )
			{
				scrattr->ta_Name = strdup( fontname );
				scrattr->ta_YSize = fsize;
				scrattr->ta_Style = FS_NORMAL;
				scrattr->ta_Flags = attr.ta_Flags;
			}
			return NULL;
		}
		else
		{
			/* Couldn't open, so use default font instead */
			td->font = GfxBase->DefaultFont;

			/* Use default font as screen font */
			scrattr = NULL;

			/* Output error message */
			sprintf(error, "PREFS:Unable to open font '%s/%d'.\n", fontname, fsize );
			return error;
		}
	}
	return NULL;
}

static BOOL process_bool(char *param)
{
	if (*param == 'Y' || *param == 'y' || *param == '1' ||
		 *param == 'T' || *param == 't')
		return TRUE;
	else
	 	return FALSE;
}

static void process_gfx(char *param)
{
	use_graphics = FALSE;
	if (*param == 'Y' || *param == 'y' || *param == '1' ||
		 *param == 'T' || *param == 't')
		use_graphics = TRUE;
	if (*param == 'E' || *param == 'e')
	{
		use_graphics = TRUE;
		screen_enhanced = TRUE;
		ANGBAND_GRAF = "new";
	}
}

static errr amiga_user( int n )
{
	return( 1 );
}

static void amiga_nuke( term *t )
{
	term_data *td = (term_data *)( t->data );
	if ( td == &data[ 0 ] )
	{
		amiga_fail( NULL );
		/* Flush the output */
		fflush( stdout );
	}
}

static void amiga_open( term *t )
{
	/* Nothing to do here */
}

static errr amiga_curs( int x, int y )
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

static errr amiga_wipe( int x, int y, int n )
{
	term_data *td = (term_data *)(Term->data);

	y -= td->ypos;
	x -= td->xpos;

	if ( y >= td->rows )
		return( 0 );
	if ( x >= td->cols )
		return( 0 );

	if ( (n > 0 ) && !td->iconified )
	{
		/* Erase rectangular area on screen */
		if (!td->background)
		{
			SetAPen( td->rp, PEN( 0 ) );
			RectFill( td->rp, x * td->fw, y * td->fh, ( x + n ) * td->fw - 1, ( y + 1 ) * td->fh - 1 );
		}
		else
			BltBitMapRastPort( td->background, (x * td->fw) % td->backw, (y * td->fh) % td->backh, td->rp, x * td->fw, y * td->fh, n * td->fw, td->fh, 0xC0);
	}

	return ( 0 );
}

static errr amiga_clear( void )
{
	term_data *td = (term_data*)(Term->data);

	if (td->iconified)
		return 1;

	/* Fill window with background colour, or background XXX XXX XXX Tile background */
	if ( !td->background )
		SetRast( td->rp, PEN( 0 ));
	else
	{
		if (use_pub)
			BltBitMapRastPort( td->background, 0, 0, td->rp, 0, 0, td->ww, td->wh, 0xC0);
		else
		{
			int x, y;
			int mx, my;

			int sw = td->ww;
			int sh = td->wh;
			struct RastPort *rp = td->rp;

			if (td == &data[0])
			{
				sw = screen_width;
				sh = screen_height;
				rp = &amiscr->RastPort;
			}
			for (y = 0 ; y < sh ; y += td->backh)
			{
				for (x = 0 ; x < sw ; x += td->backw)
				{
					mx = min(td->backw, sw - x);
					my = min(td->backh, sh - y);
					BltBitMapRastPort( td->background, x % td->backw, y % td->backh, rp, x, y, mx, my, 0xC0);
				}
			}
		}
	}
	return ( 0 );
}

static errr amiga_pict( int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp )
{
	term_data *td = (term_data *)(Term->data);

	int i;
	byte a;
	char c;

	if ( td->iconified )
		return ( 0 );

	y -= td->ypos;
	x -= td->xpos;

	if ( y >= td->rows )
		return( 0 );
	if ( x >= td->cols )
		return( 0 );

	for ( i = 0; i < n; i++ )
	{
		a = ap[i];
		c = cp[i];

		if (screen_enhanced)
		{
			put_gfx(  td->rp, x, y, tcp[i], tap[i] );
			use_mask = 1;
		}
		put_gfx(  td->rp, x, y, c, a );
		use_mask = 0;
		x++;
	}
	return ( 0 );
}

static errr amiga_text( int x, int y, int n, byte a, cptr s )
{
	term_data *td = (term_data*)(Term->data);

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
		if (td->background)
		{
			BltBitMapRastPort( td->background, (x * td->fw) % td->backw, (y * td->fh) % td->backh, td->rp, x * td->fw, y * td->fh, n * td->fw, td->fh, 0xC0);
			if (KICK30)
				SetABPenDrMd( td->rp, PEN( a & 0x0F ), PEN( 0 ), JAM1);
			else
			{
				SetDrMd( td->rp, JAM1 );
				SetAPen( td->rp, PEN( a & 0x0f ));
				SetBPen( td->rp, PEN( 0 ));
			}
			Move( td->rp, x * td->fw, y * td->fh + td->fb );
			Text( td->rp, (char *) s, n );
		}
		else
		{
//			if (use_aga)
//				quick_Text( td->rp, PEN( a & 0xF ), (char *) s, n, x * td->fw, y * td->fh + td->fb );
//			else
//			{
				if (KICK30)
					SetABPenDrMd( td->rp, PEN( a & 0x0F ), PEN( 0 ), JAM2);
				else
				{
					SetDrMd( td->rp, JAM2 );
					SetAPen( td->rp, PEN( a & 0x0f ));
					SetBPen( td->rp, PEN( 0 ));
				}
				Move( td->rp, x * td->fw, y * td->fh + td->fb );
				Text( td->rp, (char *) s, n );
//			}
		}
	}
	return ( 0 );
}

/* Various 'xtra' events */
static errr amiga_xtra( int n, int v )
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

			v *= 1000;

			/* SAS/C code. I have GNU code for this, but not included it yet */
			timer(clock);
			do
			{
				timer(clock2);
				mytime = (clock2[0] - clock[0]) * 1000*1000 + (clock2[1] - clock[1]);
				if (clock2[0] < clock[0])
					break;
			} while (mytime < v);

			return (0);

		/* Unknown request type */
		default:
			return ( 1 );
	}
	/* Can't get here */
	return ( 1 );
}

static errr amiga_flush( int v )
{
	struct IntuiMessage *imsg;

	/* Ignore all messages at the port XXX */
	while ( imsg = (struct IntuiMessage *) GetMsg( data[ 0 ].win->UserPort ))
		ReplyMsg(( struct Message *) imsg );

	return ( 1 );
}

static void process_msg(int i,ULONG iclass, UWORD icode, UWORD iqual, APTR iaddr)
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
				tmpa = modf(ud,&tmpb);
				if (tmpa >= 0.5)
					ud = ceil(ud);
				else
					ud = floor(ud);
				data[ i ].ypos = ud;

				Term_activate(angband_term[i]);
				Term_redraw();
				Term_fresh();
				Term_activate(old_term);
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

			// Don`t let user have huge windows
			if (nh > MAX_TERM_VERT)
				nh = MAX_TERM_VERT;
			if (nw > MAX_TERM_HORIZ)
				nw = MAX_TERM_HORIZ;

			data[ i ].cols = nw;
			data[ i ].rows = nh;

			if (KICK20)
			{
				ULONG f;
			        UWORD temp;

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

			// Eat the IDCMP_NEWSIZE event coming from ChangeWindowBox(). Icky hack.
			while (imsg = (struct IntuiMessage *)GetMsg( win->UserPort ))
				ReplyMsg(( struct Message *) imsg );
			break;

		case IDCMP_MENUPICK:
			handle_menupick( icode, i );
			break;
	}
}

/* Work out 'signal' mask for all our windows */
static void calc_sigmask(void)
{
	struct Window *win;
	int i;

	for (sigmask = i = 0 ; i < MAX_TERM_DATA ; i++)
	{
		if (win = data[i].win)
			sigmask |= (1 << win->UserPort->mp_SigBit);
	}
}

errr amiga_event( int v )
{
	BOOL messages;
	struct IntuiMessage *imsg;
	ULONG iclass;
	UWORD icode;
	UWORD iqual;
	APTR iaddr;
	int i;

#ifndef __GNUC__
	chkabort();
#endif
	// Create mask for Wait(), as we want to watch all of our windows

	if (!sigmask)
		calc_sigmask();

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


static errr amiga_react( int v )
{
	/* Apply color palette, in case it has changed */
	load_palette();

	/* Create menus if we don't have any */
	if ( use_menus && !menu )
		create_menus();

	return ( 0 );
}

/* Display graphical tombstone. Note this changes the palette so a load_palette
   after termination is a *must*! */
int amiga_tomb( void )
{
	/* This stinks :( */
	static ULONG tomb_col[ 16 ] =
	{
		0x000000, 0xf0e0d0, 0x808080, 0x505050,
		0xe0b000, 0xc0a070, 0x806040, 0x403020,
		0x00a0f0, 0x0000f0, 0x000070, 0xf00000,
		0x800000, 0x9000b0, 0x006010, 0x60f040,
	};

	cptr p;
	char tmp[160];
	time_t ct = time((time_t)0);
	FILE *file;

	char *pp;
	int plane, row, error = FALSE;
	struct BitMap *filebm, *scalbm, *convbm;
	long stdpens[256];
	int depth,i;

	int tw = data[ 0 ].fw * 64;
	int th = data[ 0 ].fh * 21;

	/* Release old pens. This might have bad side effects, not sure yet */
	for (i = 0 ; i < 16 ; i++)
		free_pen(abandon_pen());

	/* This is naughty :) */
	for (i = 0 ; i < 16 ; i++)
	{
		stdpens[i] = alloc_pen( 	(tomb_col[i] >> 16) & 0xFF,
						(tomb_col[i] >> 8) & 0xFF,
						tomb_col[i] & 0xFF);
	}
	/* Allocate bitmap for tomb graphics file */
	filebm = alloc_bitmap( TOMW, TOMH, TOMB, BMF_CLEAR | BMF_STANDARD, data[ 0 ].rp->BitMap );
	if (!filebm)
		return( FALSE );

	/* Open tomb file */
	path_build( tmp , MAX_PATH_LENGTH , ANGBAND_DIR_XTRA , MTOM);
	file = fopen( tmp, "r" );
	if (!file)
	{
		free_bitmap( filebm );
		return( FALSE );
	}

	/* Read file into bitmap */
	for ( plane = 0; plane < TOMB && !error; plane++ )
	{
		pp = filebm->Planes[ plane ];
		for ( row = 0; row < TOMH && !error; row++ )
		{
			error = ( fread( pp, 1, TOMW >> 3, file) != (TOMW >> 3));
			pp += filebm->BytesPerRow;
		}
	}
	fclose( file);

	/* Get depth of display */
	depth = depth_of_bitmap( data[ 0 ].rp->BitMap );

	/* Allocate bitmap for remapped image */
	convbm = alloc_bitmap( TOMW, TOMH, depth, BMF_CLEAR, data[ 0 ].rp->BitMap );
	if (!convbm)
	{
		free_bitmap( filebm );
		return( FALSE );
	}
	/* Remap old bitmap into new bitmap */
	remap_bitmap( filebm, convbm, stdpens, TOMW, TOMH );
	free_bitmap( filebm );

	/* Allocate bitmap for scaled graphics */
	if (KICK20)
	{
		scalbm = alloc_bitmap( tw, th, screen_depth, BMF_CLEAR | BMF_STANDARD, data[ 0 ].rp->BitMap );
		if (!scalbm)
		{
			free_bitmap( convbm );
			return ( FALSE );
		}

		/* Scale the tomb bitmap */
		scale_bitmap( convbm, TOMW, TOMH, scalbm, tw, th );

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
	if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = "Magnificent";
	}

	/* Normal */
	else
	{
		p = c_text + cp_ptr->title[(p_ptr->lev-1)/5];
	}

	tomb_str( 3, " R.I.P." );

	tomb_str( 5, op_ptr->full_name );

	tomb_str( 6, "the" );

	tomb_str( 7, (char *)p );

	tomb_str( 8, (char *)(c_name + cp_ptr->name) );

	sprintf( tmp, "Level: %d", (int)p_ptr->lev);
	tomb_str( 10, tmp );

	sprintf( tmp, "Exp: %ld", (long)p_ptr->exp );
	tomb_str( 11, tmp );

	sprintf( tmp, "AU: %ld", (long)p_ptr->au );
	tomb_str( 12, tmp );

	sprintf( tmp, "Killed on Level %d", p_ptr->depth );
	tomb_str( 13, tmp );

	sprintf( tmp, "by %s", p_ptr->died_from );
	tomb_str( 14, tmp );

	sprintf( tmp, "%-.24s", ctime(&ct));
	tomb_str( 16, tmp );

	return( TRUE );
}

void tomb_str( int y, char *str )
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

void handle_rawkey( UWORD code, UWORD qual, APTR addr )
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

void handle_menupick( int mnum, int term )
{
	struct MenuItem *item;
	ULONG ud;
	int i;

	/* Be sure to handle all selections */
	while ( mnum != MENUNULL )
	{
		/* Find address of menuitem */
		if (term)
			item = ItemAddress( data[term].menu, mnum);
		else
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
				Term_keypress( '\\' );

			/* Send keycode */
			Term_keypress( i );
		}
		/* Amiga palette requester */
		else
		{
			switch( ud )
			{
				case MNU_WINDOW_FONT:
				{
					char fontname[128];
					term_data *td = &data[term];

					if ( td->ownf && td->font )
					{
						CloseFont( td->font );
						td->font = NULL;
					}

					request_font(fontname);
					handle_font(td, fontname);

					SetFont(td->rp, td->font);
					Term_activate( angband_term[ term ] );
					Term_redraw();
					Term_fresh();
					break;
				}
				case MNU_WINDOW_REDRAW:
					Term_redraw();
					Term_fresh();
					break;
				case MNU_WINDOW_TOGGLEBORDERS:
					data[term].wx = data[term].win->LeftEdge;
					data[term].wy = data[term].win->TopEdge;
					close_term(term);
					data[term].backdrop = ~(data[term].backdrop);
					open_term(term, TRUE);
					amiga_flush(1);
					Term_redraw();
					Term_fresh();
					break;
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
				case MNU_GRAPHICS_OFF:
					amiga_gfx(0);
					break;
				case MNU_GRAPHICS_8:
					amiga_gfx(1);
					break;
				case MNU_GRAPHICS_16:
					amiga_gfx(2);
					break;
				case MNU_EXPORT_HS:
					amiga_hs_to_ascii();
					break;
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

static void amiga_save_file( void )
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

static char get_bool( BOOL opt )
{
	if (opt)
		return 'Y';
	else
		return 'N';
}

static void cursor_on( term_data *td )
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

static void cursor_off( term_data *td )
{
	if ( td->cursor_lit && !td->iconified )
	{
		/* Restore graphics under cursor */
		if ( CUR_A & 0xf0 && use_graphics )
		{
		put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, Term->scr->tc[ td->cursor_ypos ][ td->cursor_xpos ],
				Term->scr->ta[ td->cursor_ypos ][ td->cursor_xpos ]);
		use_mask = 1;
		put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, Term->scr->c[ td->cursor_ypos ][ td->cursor_xpos ],
				Term->scr->a[ td->cursor_ypos ][ td->cursor_xpos ]);
		use_mask = 0;
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

static void cursor_anim( void )
{
	term_data *td = term_curs;
	int x0, y0, x1, y1;
	int i = p_ptr->px,j = p_ptr->py;

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
			put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, Term->scr->tc[ td->cursor_ypos ][ td->cursor_xpos ],
				Term->scr->ta[ td->cursor_ypos ][ td->cursor_xpos ]);
			use_mask = 1;
			put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, Term->scr->c[ td->cursor_ypos ][ td->cursor_xpos ],
				Term->scr->a[ td->cursor_ypos ][ td->cursor_xpos ]);
			use_mask = 0;

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

static void free_gfx(void)
{
	if (tglob.mskbm)
		free_bitmap(tglob.mskbm);
	if (tglob.gfxbm)
		free_bitmap(tglob.gfxbm);
	if (tglob.mapbm)
		free_bitmap(tglob.mapbm);
	if (tglob.chunky_mask)
		free(tglob.chunky_mask);
	if (tglob.chunky_tile)
		free(tglob.chunky_tile);

	tglob.mapbm = tglob.mskbm = tglob.gfxbm = NULL;
	tglob.chunky_tile = tglob.chunky_mask = NULL;
}

#define PROGRESS(num, offset) \
RectFill(ts->rp, ts->fw * 22 + 1, ts->fh * offset + 1, ts->fw * (22 + num) - 1, ts->fh * (offset + 1) - 1);

#define CLEAR_PROGRESS(offset) \
RectFill(ts->rp, ts->fw * 22, ts->fh * offset, ts->fw * 29, ts->fh * (offset + 1));

/* Load the tile graphics and mask into gfxbm/mskbm */
static int load_gfx( void )
{
	char tmp[MAX_PATH_LENGTH];
	term_data *ts = &data[ 0 ];
	FILE *file;
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
	tglob.gfxbm = alloc_bitmap( GFXW, GFXH, GFXB, BMF_CLEAR | BMF_STANDARD, ts->rp->BitMap );
	if ( !tglob.gfxbm )
		return( FALSE );

	tglob.mskbm = alloc_bitmap( GFXW, GFXH, 1, BMF_CLEAR | BMF_STANDARD, ts->rp->BitMap );
	if ( !tglob.mskbm )
		return( FALSE);

	/* Open file */
	path_build( tmp , MAX_PATH_LENGTH , ANGBAND_DIR_XTRA , screen_enhanced ? AB_MGFX : DE_MGFX );
	file = fopen( tmp, "r" );
	if (!file)
	{
		MSG( 0, 0, "Unable to open graphics file" );
		Delay( 100 );
		return ( FALSE );
	}

	MSG( 0, 0, "Loading graphics ." );

	/* Read file into bitmap */
	if ( tglob.gfxbm->BytesPerRow == (GFXW >> 3) )
	{
		for ( plane = 0; plane < GFXB && !error; plane++ )
		{
			p = tglob.gfxbm->Planes[ plane ];
			error = fread(p, 1, (GFXW >> 3) * GFXH, file) != ((GFXW >> 3) * GFXH);
		}
	}
	else
	{
		for ( plane = 0; plane < GFXB && !error; plane++ )
		{
			p = tglob.gfxbm->Planes[ plane ];
			for ( row = 0; row < GFXH && !error; row++ )
			{
				error = (fread(p, 1, GFXW >> 3, file) != (GFXW >> 3));
				p += tglob.gfxbm->BytesPerRow;
			}
		}
	}

	MSG( 0, 0, "Loading graphics .." );

	/* Read mask data into bitmap */
	if ( tglob.mskbm->BytesPerRow == (GFXW >> 3) )
		fread(tglob.mskbm->Planes[0], 1, (GFXW >> 3) * GFXH, file);
	else
	{
		p = tglob.mskbm->Planes[ 0 ];
		for ( row = 0; row < GFXH && !error; row++ )
		{
			error = (fread(p, 1, GFXW >> 3, file) != (GFXW >> 3));
			p += tglob.mskbm->BytesPerRow;
		}
	}

	MSG( 0, 0, "Loading graphics ..." );
	fclose( file );

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

static int conv_gfx( void )
{
	term_data *ts = &data[ 0 ];
	struct BitMap *tmpbm;
	struct BitMap *sbm = ts->rp->BitMap;
	static long stdpens[256];
	long mskpens[] = { 0, 1 };
	long *pen_ptr;
	int depth;
	int proposed_depth;

	for (depth = 0 ; depth < 256 ; depth++)
		stdpens[depth] = depth;

	/* Get depth of display */
	depth = proposed_depth = depth_of_bitmap( sbm );

	SetAPen(ts->rp,PEN( TERM_WHITE ));
	Move(ts->rp, ts->fw * 22, ts->fh * 1);
	Draw(ts->rp, ts->fw * 22, ts->fh * 2);
	Draw(ts->rp, ts->fw * 29, ts->fh * 2);
	SetAPen(ts->rp,PEN( TERM_L_DARK ));
	Draw(ts->rp, ts->fw * 29, ts->fh * 1);
	Draw(ts->rp, ts->fw * 22, ts->fh * 1);

	SetAPen(ts->rp, PEN( TERM_BLUE ));

	pen_ptr = stdpens;
	if (use_graphics)
		pen_ptr = gfxpens;

	/* Remap graphics and mask bitmaps to correct colours:
		a) AGA/ECS       : Write/read from memory directly
		b) CGX           : Use Cybergraphics library
		c) Other         : Just use normal intuition calls and hope for the best

		Note all this only applies to graphics, not text. All text is handled as
		normal in the 68k versions. */

	/* With CGX we only need a maximum of 8 bits for gfx */
	if (use_cyber && proposed_depth > 8)
		proposed_depth = 8;

	tmpbm = alloc_bitmap( ts->map_w, ts->map_h, proposed_depth, BMF_CLEAR | BMF_STANDARD, sbm );
	if (!tmpbm)
	{
		MSG( 0, 1, "Can`t allocate a temporary bitmap (tiles)" );
		Delay( 100 );
		return FALSE;
	}

	/* Remap mini map tiles */
	remap_bitmap( tglob.mapbm, tmpbm, pen_ptr, ts->map_w, ts->map_h );
	free_bitmap( tglob.mapbm);
	tglob.mapbm = tmpbm;

	PROGRESS(1,1);
	tmpbm = alloc_bitmap( ts->gfx_w, ts->gfx_h, proposed_depth, BMF_CLEAR, sbm);
	if (!tmpbm)
	{
		MSG( 0, 1, "Can`t allocate a temporary bitmap (tiles)" );
		Delay( 100 );
		return FALSE;
	}
	PROGRESS(2,1);
	remap_bitmap( tglob.gfxbm, tmpbm, pen_ptr, ts->gfx_w, ts->gfx_h );
	free_bitmap( tglob.gfxbm );
	PROGRESS(3,1);
	tglob.gfxbm = tmpbm;

	PROGRESS(4,1);

	if (use_cyber)
	{
		byte *cm;
		byte *cd;

		int lx, ly;

		tglob.chunky_mask = cd = malloc( ts->gfx_w * ts->gfx_h );
		cm = (byte *)tglob.mskbm->Planes[0];
		cd = tglob.chunky_mask;
		for (ly = 0 ; ly < ts->gfx_h ; ly++)
		{
			for (lx = 0 ; lx < (ts->gfx_w >> 3) ; lx++)
				*cd++ = *cm++;
			if ((ts->gfx_w / 8) & 1)
				cd++;
			if (ly == (ts->gfx_h >> 1))
				PROGRESS(5,1);
			cm = cm - (ts->gfx_w >> 3);
			cm += tglob.mskbm->BytesPerRow;
		}

		PROGRESS(7,1);
		SetAPen(ts->rp, PEN(TERM_DARK) );
		CLEAR_PROGRESS(1);

		return TRUE;
	}
	/* Try to remap mask bitmap */
	tmpbm = alloc_bitmap( ts->gfx_w, ts->gfx_h, depth, BMF_CLEAR, sbm );
	if (!tmpbm)
	{
		MSG( 0, 1, "Can`t allocate a temporary bitmap (mask)" );
		Delay( 100 );
		return FALSE;
	}

	PROGRESS(6,1);
	/* XXX XXX XXX Replace with copy_bitmap. Oops, I deleted that :) */

	remap_bitmap( tglob.mskbm, tmpbm, mskpens, ts->gfx_w, ts->gfx_h );
	free_bitmap( tglob.mskbm );
	tglob.mskbm = tmpbm;

	PROGRESS(7,1);
	SetAPen(ts->rp, PEN(TERM_DARK) );
	CLEAR_PROGRESS(1);

	/* Done */
	return TRUE;
}

/* Scale graphics to correct size */
static int size_gfx( term_data *td )
{
	int depth;
	struct BitMap *sbm = td->rp->BitMap;
	struct BitMap *tmpbm;

	int tilew = 8, tileh = 8;
	int tilenumw = (DF_GFXW / tilew);
	int tilenumh = (DF_GFXH / tileh);

	if (KICK13)
		return( TRUE );

	if (screen_enhanced)
	{
		tilew = tileh = 16;
		tilenumw = (AB_GFXW / tilew);
		tilenumh = (AB_GFXH / tileh);
	}
	/* Calculate tile bitmap dimensions */
	td->gfx_w = (GFXW / tilew) * td->fw;
	td->gfx_h = (GFXH / tileh) * td->fh;

	/* Calculate map bitmap dimensions */
	td->mpt_w = td->ww / DUNGEON_WID;
	td->mpt_h = td->wh / DUNGEON_HGT;

	td->map_w = td->mpt_w * tilenumw;
	td->map_h = td->mpt_h * tilenumh;

	/* Friend bitmap */
	if ( td->rp )
		sbm = td->rp->BitMap;

	/* Scale tile graphics into map size */
	depth = depth_of_bitmap( tglob.gfxbm );
	if (( tglob.mapbm = alloc_bitmap( td->map_w, td->map_h, depth, BMF_CLEAR | BMF_STANDARD, sbm )) == NULL )
		return( FALSE );

	scale_bitmap( tglob.gfxbm, GFXW, GFXH, tglob.mapbm, td->map_w, td->map_h );

	/* Scale tile graphics */
	depth = depth_of_bitmap( tglob.gfxbm );
	if (( tmpbm = alloc_bitmap( td->gfx_w, td->gfx_h, depth, BMF_CLEAR | BMF_STANDARD, sbm )) == NULL )
		return( FALSE );
	scale_bitmap( tglob.gfxbm, GFXW, GFXH, tmpbm, td->gfx_w, td->gfx_h );
	if ( tglob.gfxbm )
		free_bitmap( tglob.gfxbm );
	tglob.gfxbm = tmpbm;

	/* Scale tile mask */
	depth = depth_of_bitmap( tglob.mskbm );
	if (( tmpbm = alloc_bitmap( td->gfx_w, td->gfx_h, depth, BMF_CLEAR | BMF_STANDARD, sbm )) == NULL )
		return( FALSE );
	scale_bitmap( tglob.mskbm, GFXW, GFXH, tmpbm, td->gfx_w, td->gfx_h );
	if ( tglob.mskbm )
		free_bitmap( tglob.mskbm );
	tglob.mskbm = tmpbm;

	/* Success */
	return( TRUE );
}

static void put_gfx( struct RastPort *rp, int x, int y, int chr, int col )
{
	term_data *td = (term_data *)(Term->data);
	int fw = td->fw;
	int fh = td->fh;
	int x0 = x * fw;
	int y0 = y * fh;
	int x1 = x0 + fw - 1;
	int y1 = y0 + fh - 1;
	int a = col & 0x7F;
	int c = chr & 0x7F;

	if (( td->iconified ) || ( !rp ))
		return;

	/* Just a black tile */
	if ( a == 0 && c == 0 )
	{
		if ( use_bkg )
			BltBitMapRastPort( td->background, c * fw, a * fh, rp, x0, y0, fw, fh, 0xC0);
		else
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
	if ( use_mask )
	{
		if ( use_bkg )
			BltBitMapRastPort( td->background, c * fw, a * fh, rp, x0, y0, fw, fh, 0xC0);

		if (use_cyber)
			BltMaskBitMapRastPort( tglob.gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, (ABC|ANBC|ABNC), tglob.chunky_mask );
		else
			BltMaskBitMapRastPort( tglob.gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, (ABC|ANBC|ABNC), tglob.mskbm->Planes[ 0 ] );
	}

	/* Draw full tile */
	else
	{
		if (use_aga)
			quick_BltBitMapRastPort( tglob.gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, 0xc0 );
		else
			BltBitMapRastPort( tglob.gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, 0xc0 );
	}
}

#ifndef __GNUC__
static int breakfunc(void)
{
	puts("break!");
	amiga_fail("Ctrl-C received....quitting");
	return 1;
}
#endif

/* Be prepared for this to be called multiple times! */
static int amiga_fail( char *msg )
{
	int i;

	/* Print error message */
	if ( msg )
		 puts( msg );

	if (tglob.resources_freed)
		return(-1);

	/* Free shared term data */
	if (tglob.chunky_tile)
		free(tglob.chunky_tile);

	if (tglob.chunky_mask)
		free(tglob.chunky_mask);

	if (tglob.gfxbm)
		free_bitmap(tglob.gfxbm);

	if (tglob.mskbm)
		free_bitmap(tglob.mskbm);

	if (tglob.mapbm)
		free_bitmap(tglob.mapbm);

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
		FreeMenus( menu );

	FreeMem( blankpointer, BLANKPOINTER_SIZE );

	if (sound_name_desc)
		free(sound_name_desc);

	if (sound_data)
		free(sound_data);

	/* Free term resources */
	for ( i = MAX_TERM_DATA - 1; i >= 0; i--)
		free_term( &data[ i ] );

	/* Free obtained pens */
	for ( i = 0; i < 512; i++)
	{
		while ( obtain_mask[ i ] )
		{
			ReleasePen( pubscr->ViewPort.ColorMap, i );
			obtain_mask[i]--;
		}
	}

	/* Free visual info . It may seem like paranoia here, but it simplifies other code
		to do this */
	if ( visinfo && KICK30)
	{
		FreeVisualInfo( visinfo );
		visinfo = NULL;
	}

	/* Close intuition screen */
	if ( amiscr )
		CloseScreen( amiscr );

	/* Close console.device */
	if ( ConsoleDev )
		CloseDevice( (struct IORequest *)ConsoleDev );

	/* Close various libraries */
	if ( CyberGfxBase )
		CloseLibrary( CyberGfxBase );

	if ( DataTypesBase )
		CloseLibrary( DataTypesBase );

	if ( DiskfontBase )
		CloseLibrary( DiskfontBase );

	if ( GadToolsBase )
		CloseLibrary( GadToolsBase );

	if ( ReqToolsBase )
		CloseLibrary( (struct Library *)ReqToolsBase );

	if ( AslBase )
		CloseLibrary( AslBase );

	if ( IFFBase )
		CloseLibrary( IFFBase );

	tglob.resources_freed = TRUE;
	return( -1 );
}

static void amiga_map( void )
{
	term_data *td = &data[ 0 ];
	int i,j;
	byte ta,tc;
	int cur_wid = DUNGEON_WID,cur_hgt = DUNGEON_HGT;

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

	/* Draw all "interesting" features */
	for ( i = 0; i < cur_wid; i++ )
	{
		for ( j = 0; j < cur_hgt; j++ )
		{
			/* Get frame tile */
			if ( i==0 || i == cur_wid - 1 || j == 0 || j == cur_hgt - 1 )
			{
				ta = f_info[ 63 ].x_attr;
				tc = f_info[ 63 ].x_char;
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







void load_palette( void )
{
	int i;

	/* Release all the pens we have */
	for (i = 0 ; i < 512 ; i++)
		free_pen(i);

	/* Now create the text colours */
	for (i = 0 ; i < 16 ; i++)
	{
		penconv[i] = alloc_pen(	angband_color_table[i][1],
										angband_color_table[i][2],
										angband_color_table[i][3]);

		/* This error message should never happen. */
		if (penconv[i] == -1)
			FAIL("Can't allocate any pens for text colours!");
	}

	/* Now do the rest */
	if (use_graphics)
	{
		/* We may run out of colours here; in that case we'll attempt to pick sensible ones
			via alloc_pen. I reckon text colour is more important than gfx colour. */
		for (i = 0 ; i < (screen_enhanced ? 256 : 32 ); i++)
		{
			gfxpens[i] = alloc_pen(	palette256[(i << 2) + 1],
											palette256[(i << 2) + 2],
											palette256[(i << 2) + 3]);
		}
	}
}

/* Returns TRUE if can read AB palette, FALSE if not */
static int read_enhanced_palette(void)
{
	static char buffer[1024];
	FILE *f;

	path_build( buffer, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, AB_MGFX_CMAP );
	f = fopen(buffer, "r");
	if (f)
	{
		int success = fread(palette256,1024,1,f);

		fclose(f);
		if (success == 1)
			return TRUE;
	}
	return FALSE;
}

/* Returns TRUE if can read normal 8x8 palette, FALSE if not */
static int read_normal_palette(void)
{
	static char buffer[1024];
	FILE *f;

	path_build( buffer, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, DE_MGFX_CMAP );
	f = fopen(buffer, "r");
	if (f)
	{
		int success = fread(palette256,128,1,f);

		fclose(f);
		if (success == 1)
			return TRUE;
	}
	return FALSE;
}

ULONG trans( byte g )
{
	ULONG h;

	h = (g << 8) | (g << 16) | (g << 24) | g;
	return h;
}

int create_menus( void )
{
	struct NewMenu *item = newmenu;
	int nmsize = sizeof ( struct NewMenu );
	int i;

	/* Option code deleted. It's just such a pain to do :( */

	for ( i = 0; menu_ptr[ i ].nm_Type != NM_END; i++ )
		memcpy( item++, &menu_ptr[ i ], nmsize );

	/* Copy all post-items into array */
	for ( i = 0; post_item[ i ].nm_Type != NM_END; i++ )
		memcpy( item++, &post_item[ i ], nmsize );

	/* Actually create the menu structures */
	menu = CreateMenus( newmenu, GTMN_FrontPen, (long)penconv[ TERM_WHITE ], NULL );

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

static int find_menuitem(int *rmenu, int *item, void *ud)
{
	int i = -1;
	int a = -1, b = -1;
	do
	{
		i++;
		if (newmenu[i].nm_Type == NM_TITLE)
		{
			a++;
			b = 0;
		}
		else if (newmenu[i].nm_Type == NM_ITEM)
			b++;
		if (newmenu[i].nm_UserData == ud)
		{
			*rmenu = a;
			*item = b;
			return 1;
		}
	}
	while (newmenu[i].nm_Type != NM_END);
	return 0;
}

void update_menus( void )
{
	struct MenuItem *item;
	int i, a, b;

	/* Require a window and a menu */
	if ( !data[ 0].win || !menu )
		return;

	/* Detach the menu from the window */

	/* Enable/Disable the amiga map according to use_graphics */
	if (find_menuitem(&a, &b, (void *)MNU_SCALEDMAP))
	{
		if ( item = ItemAddress( menu, FULLMENUNUM(a, b, 0)))
			item->Flags = use_graphics ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;
	}
	/* Enable/Disable the palette requester */

	if (find_menuitem(&a, &b, (void *)MNU_PALETTE))
	{
		if ( item = ItemAddress( menu, FULLMENUNUM(a, b, 0)))
			item->Flags = amiga_palette ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;
	}

	/* Enable/Disable and check window menu items according to use and iconified status */
	for ( i = 1; i < MAX_TERM_DATA; i++ )
	{
		if (find_menuitem(&a, &b, MWI( i )))
		{
			if ( item = ItemAddress( menu, FULLMENUNUM( a, b, 0 )))
			{
				item->Flags = data[ i ].use ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;
				item->Flags = ( data[ i ].use && !data[ i ].iconified ) ? item->Flags | CHECKED : item->Flags & ~CHECKED;
			}
		}
	}
	/* Attach menu to window again */
}

int init_sound( void )
{
	static char tmp[MAX_PATH_LENGTH];
	static char buf[MAX_PATH_LENGTH];
	static char line[256];
	struct AmiSound *snd;
	static int use_angsound = FALSE;
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

	if (getasn("AngSound"))
		use_angsound = TRUE;

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
			FILE *f;

			/* Construct filename */
			path_build(tmp, MAX_PATH_LENGTH, buf, snd->Name );

			if (!(f = fopen(tmp, "r")))
			{
				if (use_angsound)
					sprintf(tmp, "AngSound:%s",snd->Name);
			}
				else
			fclose(f);

			/* Load the sample into memory */
			snd->Address = (struct SoundInfo *) PrepareSound( tmp );
		}

		snd++;
	}
	has_sound = use_sound = TRUE;

	return ( TRUE );
}

void free_sound( void )
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
			RemoveSound( snd->Address );
			snd->Address = NULL;
		}
		snd++;
	}

	has_sound = use_sound = FALSE;
}

static void play_sound( int v )
{
	struct AmiSound *snd;
	struct AmiSound *old_snd;
	int rate;
	int channel;
	int old,vnum;
	char buf[MAX_PATH_LENGTH];
	char tmp[MAX_PATH_LENGTH];
	static int try_angsound = FALSE;
	static int use_angsound = FALSE;

	if (use_sound && !try_angsound)
	{
		try_angsound = TRUE;
		use_angsound = (int)(getasn("AngSound"));
	}

	if ( has_sound )
	{
		/* If no sounds are available for chosen event, skip */
		if ((int)sound_ref[v][0] == 0)
			return;
		if (v > SOUND_MAX)
			return;

		/* Just pick 1st sound available at the moment */
		snd = sound_ref[v][vnum = 1 + rand_int( (int)sound_ref[v][0] )];

		/* Channel number */
		channel = snd->Channel;

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
			FILE *f;

			/* Construct filename */
			path_build(buf, MAX_PATH_LENGTH, ANGBAND_DIR_XTRA, "sound");
			path_build(tmp, MAX_PATH_LENGTH, buf, snd->Name );

			if (!(f = fopen(tmp, "r")))
			{
				if (use_angsound)
					sprintf(tmp, "AngSound:%s",snd->Name);
			}
				else
			fclose(f);
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

void put_gfx_map( term_data *td, int x, int y, int c, int a )
{
	if ( td->iconified || !td->wrp)
		return;

	if (use_aga)
		quick_BltBitMapRastPort(
		tglob.mapbm,
		c * td->mpt_w,
		a * td->mpt_h,
		td->wrp,
		td->map_x + x * td->mpt_w,
		td->map_y + y * td->mpt_h,
		td->mpt_w,
		td->mpt_h,
		0xC0);
	else
		BltBitMapRastPort(
		tglob.mapbm,
		c * td->mpt_w,
		a * td->mpt_h,
		td->wrp,
		td->map_x + x * td->mpt_w,
		td->map_y + y * td->mpt_h,
		td->mpt_w,
		td->mpt_h,
		0xC0);

}

struct BitMap *alloc_bitmap( int width, int height, int depth, ULONG flags, struct BitMap *friend )
{
	int p;
	struct BitMap *bitmap;
	unsigned char *bp;

	if ( KICK30 )
	{
		/* Should this work for 8-bit too? */
		if (use_cyber && ((flags & BMF_STANDARD) == 0) )
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

void free_bitmap( struct BitMap *bitmap )
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

void scale_bitmap( struct BitMap *srcbm, int srcw, int srch, struct BitMap *dstbm, int dstw, int dsth )
{
	struct BitScaleArgs bsa;

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

void remap_bitmap( struct BitMap *srcbm, struct BitMap *dstbm, long *pens, int width, int height )
{
	static struct RastPort tmprp;
	int x,y,p,c,ox,lpr,sd,dd;
	int bm[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
	UBYTE *sp[ 32 ];
	UBYTE *dp[ 32 ];
	ULONG ls[ 32 ];
	ULONG ld[ 32 ];
	ULONG mask;

	int enforce_aga = TRUE;

	// Handle gfx cards. Use the AGA/ECS/OCS code if at all possible!
	if (use_cyber)
	{
		if (GetCyberMapAttr(srcbm, CYBRMATTR_ISCYBERGFX) || GetCyberMapAttr(dstbm, CYBRMATTR_ISCYBERGFX))
			enforce_aga = 0;
	}

	if (!enforce_aga)
	{
		struct RastPort mainrast,newrast;
		struct BitMap *tmpbm;

		LONG colour;
		LONG twidth;
		byte *s;

		twidth = ((width + 15) >> 4) << 1;
		s = malloc(width * 2);
		InitRastPort(&mainrast);
		InitRastPort(&newrast);
		mainrast.BitMap = srcbm;
		newrast.BitMap = dstbm;

		tmpbm = alloc_bitmap( width, 1, depth_of_bitmap(dstbm), NULL, NULL );
//		tmpbm->BytesPerRow = (((width + 15)>>4)<<1);
//		tmprp = mainrast;
		tmprp.Layer = NULL;
		tmprp.BitMap = tmpbm;

		/* V40 uses ChunkyPixel commands */
		if ( GfxBase->LibNode.lib_Version >= 40 )
		{
			for (y = 0 ; y < height ; y++)
			{
//				ReadPixelLine8(&mainrast, 0, y, width, s, &tmprp);
				for (x = 0 ; x < width ; x++)
					s[x] = pens[ ReadPixel(&mainrast,x,y) ];
				WriteChunkyPixels(&newrast, 0, y, width, y, s, width);
			}
		}
		else
		{
			for (y = 0 ; y < height ; y++)
			{
				for (x = 0 ; x < width ; x++)
				{
					colour = pens[ ReadPixel(&mainrast,x,y) ];
					SetAPen(&newrast,colour);
					WritePixel(&newrast,x,y);
				}
			}
		}
		free_bitmap(tmpbm);
		free(s);
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

int depth_of_bitmap( struct BitMap *bm )
{
	if ( KICK30 )
		return ( (int)GetBitMapAttr( bm, BMA_DEPTH ) );
	else
		return (bm->Depth);
}

/*  Put message on screen, allow time for it to be read, then erase it  */
void amiga_show( char *str )
{
	char *spaces = "                                                   ";

	amiga_text( 0, 0, strlen( str ), 1, str );
	Delay(80);
	amiga_text( 0, 0, strlen( spaces ), 1, spaces );
}

/* Brings up Reqtools palette requester, extracts new colours and places in
   angband_color_table. */
void amiga_redefine_colours( void )
{
	term_data *ts = &data[ 0 ];
	struct BitMap *sbm = ts->rp->BitMap;
	int cols;
	int i;

	/* Paranoia */
	if (!amiga_palette || pubscr || !ReqToolsBase)
		return;

	/* Get number of colours on screen */
	cols = 1 << depth_of_bitmap(sbm);
	if (cols > 256)
		cols = 256;

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
/*  This is patched into the 'main.c' file at the moment. 					*/
/* -------------------------------------------------------------------- */

void amiga_makepath( char *name )
{
	FILE *f;
	static char temp[512];
	static char temp2[512];
	int pass = 0;
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
	/* This is ugly code, but keeps the PPC version happy. */

	do
	{
		if (!pass)
			NameFromLock(GetProgramDir(),temp,400);
		else
			NameFromLock(ParentDir(GetProgramDir()),temp,400);
		strcpy(temp2, temp);

		AddPart(temp, "edit/monster.txt", 500);
		if (f = fopen(temp,"r"))
			break;
		strcpy(temp, temp2);
		AddPart(temp, "data/monster.raw", 500);
		if (f = fopen(temp,"r"))
			break;
		pass++;
	} while (pass < 2);

	if (f)
	{
		char c;

		fclose(f);

		c = temp2[strlen(temp2) - 1];
		if (c != '/' && c != ':')
			strcat(temp2,"/");
		strcpy(name, temp2);
		return;
	}
	strcpy(name,VERPATH);
	return;
}

/* Dump palette in same format as Angband; this is not ideal */
void amiga_save_palette( void )
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
	fprintf(fff, "\n\n# Color redefinitions\n\n");

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

		/* Dump the palette info */
		fprintf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
				        i, kv, rv, gv, bv);
	}

	/* All done */
	fprintf(fff, "\n\n\n\n");

	/* Close */
	fclose(fff);
}

/* Loads the current palette */

void amiga_load_palette( void )
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

void amiga_hs_to_ascii(void)
{
	char filename[MAX_PATH_LENGTH];
	char destfile[MAX_PATH_LENGTH];
	char temp[200];
	char date_temp[15];
	struct high_score h;
	int i;
	FILE *f,*d;

	int pr, pc, clev, mlev, cdun, mdun;
	cptr gold, when, aged;

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

	sprintf(temp,"Highscore file for %s",VARIANT);
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

	/* Hack -- extract the gold and such */
	for (when = h.day; isspace(*when); when++) /* loop */;
	for (gold = h.gold; isspace(*gold); gold++) /* loop */;
	for (aged = h.turns; isspace(*aged); aged++) /* loop */;

	/* Reconfigure Date */
	if ((*when == '@') && strlen(when) == 9)
	{
		sprintf(date_temp, "%.2s-%.2s-%.4s", 
			when+7, when+5, when+1);
	}
	else
	{
		sprintf(date_temp, "%.2s-%.2s-20%.2s",
			when+3, when, when+6);
	}
	when = date_temp;

	/* Dump some info */
	sprintf(temp, "%3d.%9s  %s the %s %s, Level %d",
	        i + 1, h.pts, h.who,
	        p_name + p_info[pr].name,
		c_name+c_info[pc].name,
	        clev);

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
	       "               (Date %s, Gold %s, Turn %s).",
			        when, gold, aged);

	fprintf(d, "%s\n\n",temp);

	}
	fclose(d);
	fclose(f);
}

/* Provides name of the last used save file in 'buf', by reading
   'user/data-ami.prf'. This is a hack, but works for most player names. 
   Insert in main.c */
void amiga_user_name( char *buf )
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
/*  amiga_write_user_name( char *name )					*/
/*									*/
/*  Writes the 'name' argument to the file 'user/data-ami.prf'.		*/
/*  Generally, the 'name' argument will be 'player_name' i.e. 'Gandalf' */
/*  This is used in the 'save_player()' routine (see save.c) to		*/
/*  automatically load the last used player.				*/
/* 									*/
/*  Note that this has to be explicitly added to save.c			*/
/* -------------------------------------------------------------------- */

void amiga_write_user_name( char *name )
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

static int get_p_attr( void )
{
	int pc = p_ptr->pclass,pr = p_ptr->prace;
	pc = pc % 6;
	pr = pr % 5;
	return((( pc * 10 + pr) >> 5) + 12);
}

/* -------------------------------------------------------------------- */
/*  get_p_char( void )                                                  */
/*                                                                      */
/*  Returns an integer representing the 'char' attribute of the current */
/*  player graphic. This will usually vary depending upon player's      */
/*  chosen class.                                                       */
/* -------------------------------------------------------------------- */

static int get_p_char( void )
{
	int pc = p_ptr->pclass,pr = p_ptr->prace;
	pc = pc % 6;
	pr = pr % 5;
	return(( pc * 10 + pr) & 0x1f );
}

static void amiga_gfx(int type)
{
	if (!type)
	{
		use_graphics = screen_enhanced = 0;
		reset_visuals(0);
		free_gfx();
		do_cmd_redraw();
	}
	if (type)
	{
		use_graphics = 1;
		screen_enhanced = (type > 1);
		free_gfx();

		init_default_palette();
		if (screen_enhanced)
			read_enhanced_palette();
		else
			read_normal_palette();

		load_palette();
		Term_save();
		SetRast( data[0].rp, PEN( 0 ));

		MSG( 0, 0, "Loading graphics" );
		if ( !load_gfx() )
		{
			Term_load();
			use_graphics = 0;
			return;
		}
		size_gfx( &data[ 0 ] );
		MSG( 0, 1, "Remapping graphics" );
		if ( !conv_gfx() )
		{
			Term_load();
			use_graphics = 0;
			return;
		}
		Term_load();
		/* XXX XXX XXX */
		if (screen_enhanced)
			ANGBAND_GRAF = "new";
		else
			ANGBAND_GRAF = "old";

		reset_visuals(0);
		do_cmd_redraw();
	}
}

/* NOTE : The font for this had better not be NULL else all
   hell will break loose. You Have Been Warned */
static void quick_Text(struct RastPort *rp, int col, char *s, int n, int dx, int dy)
{
	ULONG *dest_plane_addr;
	ULONG dest_row = rp->BitMap->BytesPerRow;
	short depth;

	int x;
	struct TextFont *font = rp->Font;
	ULONG source_row = font->tf_Modulo;
	static ULONG left_mask[] = { 0, 0x80000000L, 0xC0000000L, 0xE0000000L, 0xF0000000L, 0xF8000000L, 0xFC000000L, 0xFE000000L, 0xFF000000L,
											0xFF800000L, 0xFFC00000L, 0xFFE00000L, 0xFFF00000L, 0xFFF80000L, 0xFFFC0000L, 0xFFFE0000L };
	byte *source;
	byte *dest;
	ULONG mask;
	UWORD *floc = (UWORD *)font->tf_CharLoc;
	short t;
	ULONG sourceoffset, destoffset, sval;
	static byte *p;
	static byte *m;
	int dbit;

	/* This is very possibly wrong, because it's a complete guess */
	dx += rp->Layer->bounds.MinX;
	dy += rp->Layer->bounds.MinY;

	dy -= font->tf_Baseline;

	/* AGA only, or at least AGA *bitmaps* only */

startme:
	if (!n)
		return;
	dbit = 1;
	dest_plane_addr = (ULONG *)&(rp->BitMap->Planes[0]);
	depth = rp->BitMap->Depth;

	/* Find point in dest bitmap */
	destoffset = (dy * dest_row) + dx / 8;
	sourceoffset = floc[ (*s - font->tf_LoChar) << 1 ];
	x = sourceoffset & 7;
	sourceoffset >>= 3;

	mask = left_mask[ font->tf_XSize ];
	mask >>= (dx & 7);
	while (depth)
	{
		source = (byte *)font->tf_CharData + sourceoffset;
		dest = (byte *)((*dest_plane_addr++) + destoffset);

		for (t = font->tf_YSize ; t ; t--)
		{
			p = (byte *)&sval;
			m = (byte *)&mask;
			sval = (*((ULONG *)source) << (ULONG)(x & 7));
			sval >>= (dx & 7);
			if (!(col & dbit))
				sval = 0;
			*dest = (~(*m) & *dest) | (*m & *p++);
			m++;
			dest[1] = (~(*m) & dest[1]) | (*m & *p);

			dest += dest_row;
			source += source_row;
		}
		depth--;
		dbit += dbit;
	}
	n--;
	dx += font->tf_XSize;
	s++;
	goto startme;
}

static void quick_BltBitMapRastPort( struct BitMap *src, int x, int y, struct RastPort *rp, int dx, int dy, int dw, int dh, int mode)
{
	ULONG *dest_plane_addr = (ULONG *)&(rp->BitMap->Planes[0]);
	ULONG *source_plane_addr = (ULONG *)&(src->Planes[0]);
	ULONG dest_row = rp->BitMap->BytesPerRow;
	ULONG source_row = src->BytesPerRow;
	short depth = src->Depth;

	static ULONG left_mask[] = { 0, 0x80000000L, 0xC0000000L, 0xE0000000L, 0xF0000000L, 0xF8000000L, 0xFC000000L, 0xFE000000L, 0xFF000000L,
											0xFF800000L, 0xFFC00000L, 0xFFE00000L, 0xFFF00000L, 0xFFF80000L, 0xFFFC0000L, 0xFFFE0000L };
	byte *source;
	byte *dest;
	ULONG mask;
	short t;
	ULONG sourceoffset, destoffset, sval;
	static byte *p;
	static byte *m;

	/* This is very possibly wrong, because it's a complete guess */
	dx += rp->Layer->bounds.MinX;
	dy += rp->Layer->bounds.MinY;

	/* AGA only, or at least AGA *bitmaps* only */

	/* Find point in dest bitmap */
	destoffset = (dy * dest_row) + dx / 8;
	sourceoffset = (y * source_row) + x / 8;
	mask = left_mask[ dw ];
	mask >>= (dx & 7);
	while (depth)
	{
		source = (byte *)((*source_plane_addr++) + sourceoffset);
		dest = (byte *)((*dest_plane_addr++) + destoffset);

		for (t = dh ; t ; t--)
		{
			p = (byte *)&sval;
			m = (byte *)&mask;
			sval = (*((ULONG *)source) << (ULONG)(x & 7));
			sval >>= (dx & 7);
			*dest = (~(*m) & *dest) | (*m & *p++);
			m++;
			dest[1] = (~(*m) & dest[1]) | (*m & *p);

			dest += dest_row;
			source += source_row;
		}
		depth--;
	}
}

#if 0
static void quick_clearBltBitMapRastPort( struct BitMap *src, int x, int y, struct RastPort *rp, int dx, int dy, int dw, int dh, int mode)
{
	ULONG *dest_plane_addr = (ULONG *)&(rp->BitMap->Planes[0]);
	ULONG *source_plane_addr = (ULONG *)&(src->Planes[0]);
	ULONG dest_row = rp->BitMap->BytesPerRow;
	ULONG source_row = src->BytesPerRow;
	short depth = src->Depth;

	static ULONG left_mask[] = { 0, 0x80000000L, 0xC0000000L, 0xE0000000L, 0xF0000000L, 0xF8000000L, 0xFC000000L, 0xFE000000L, 0xFF000000L,
											0xFF800000L, 0xFFC00000L, 0xFFE00000L, 0xFFF00000L, 0xFFF80000L, 0xFFFC0000L, 0xFFFE0000L };
	byte *source;
	byte *dest;
	ULONG mask;
	short t;
	ULONG sourceoffset, destoffset, sval;
	static byte *p;
	static byte *m;

	/* AGA only, or at least AGA *bitmaps* only */

	/* Find point in dest bitmap */

	destoffset = (dy * dest_row) + dx / 8;
	sourceoffset = (y * source_row) + x / 8;
	mask = left_mask[ dw ];
	mask >>= (dx & 7);
	while (depth)
	{
		source = (byte *)((*source_plane_addr++) + sourceoffset);
		dest = (byte *)((*dest_plane_addr++) + destoffset);

		for (t = dh ; t ; t--)
		{
			p = &sval;
			m = &mask;
			sval = 0;
			sval = (*((ULONG *)source) << (ULONG)(x & 7));
			sval >>= (dx & 7);
			*dest = (~(*m) & *dest) | (*m & *p++);
			m++;
			dest[1] = (~(*m) & dest[1]) | (*m & *p);

			dest += dest_row;
			source += source_row;
		}
		depth--;
	}
}

static void quick_BltMaskBitMapRastPort( struct BitMap *src, int x, int y, struct RastPort *rp, int dx, int dy, int dw, int dh, int mode, byte *mask)
{
//	ULONG *dest_plane_addr = (ULONG *)&(rp->BitMap->Planes[0]);
	ULONG *dest_plane_addr = (ULONG *)&(rp->Layer->RastPort->BitMap->Planes[0]);
	ULONG *source_plane_addr = (ULONG *)&(src->Planes[0]);
	ULONG dest_row = rp->BitMap->BytesPerRow;
	ULONG source_row = src->BytesPerRow;
	short depth = src->Depth;

	static ULONG left_mask[] = { 0, 0x80000000L, 0xC0000000L, 0xE0000000L, 0xF0000000L, 0xF8000000L, 0xFC000000L, 0xFE000000L, 0xFF000000L,
											0xFF800000L, 0xFFC00000L, 0xFFE00000L, 0xFFF00000L, 0xFFF80000L, 0xFFFC0000L, 0xFFFE0000L };
	byte *source;
	byte *dest;
	ULONG mask;
	short t;
	ULONG sourceoffset, destoffset, sval;
	static byte *p;
	static byte *m;

	/* NOT FINISHED!! */
	/* AGA only, or at least AGA *bitmaps* only */

	/* Find point in dest bitmap */

	destoffset = (dy * dest_row) + dx / 8;
	sourceoffset = (y * source_row) + x / 8;
	mask = left_mask[ dw ];
	mask >>= (dx & 7);
	while (depth)
	{
		source = (byte *)((*source_plane_addr++) + sourceoffset);
		dest = (byte *)((*dest_plane_addr++) + destoffset);

		for (t = dh ; t ; t--)
		{
			p = &sval;
			m = &mask;
			sval = (*((ULONG *)source) << (ULONG)(x & 7));
			sval >>= (dx & 7);
			*dest = (~(*m) & (*dest) | (*m & *p++);
			m++;
			dest[1] = (~(*m) & dest[1]) | (*m & *p);

			dest += dest_row;
			source += source_row;
		}
		depth--;
	}
}

#endif
