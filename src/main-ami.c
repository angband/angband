/*

   File:     main-ami.c

   Version:  2.7.9v6 (09.May.96)

   Purpose:  Amiga module for Angband with graphics and sound

   Author:   Lars Haugseth
   Email:    larshau@ifi.uio.no
   WWW:      http://www.ifi.uio.no/~larshau

*/

#define VERSION "Angband 2.7.9v6"

///{ "includes"

#include "angband.h"
#include "sound-ami.h"
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <devices/inputevent.h>
#include <graphics/gfxbase.h>
#include <graphics/modeid.h>
#include <graphics/scale.h>
#include <graphics/text.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <proto/asl.h>
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/keymap.h>
#include <proto/gadtools.h>

///}
///{ "macros"

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

///}
///{ "defines"

/* Color to use for cursor */
#define CURSOR_PEN 4

/* Size of graphical tile */
#define TILEW 8
#define TILEH 8

/* Size of tile image */
#define GFXW 256
#define GFXH 256
#define GFXB 32

/* Size of tombstone image */
#define TOMW 512
#define TOMH 168
#define TOMB 64

/* Filename of tile image */
#define MGFX "Angband:xtra/gfx/tiles.raw"

/* Filename of tombstone image */
#define MTOM "Angband:xtra/gfx/tomb.raw"

/* Filename of preferences file */
#define WPRF "Angband:user/settings.prf"

///}
///{ "globals"

/* DisplayID specified with option -m */
char modestr[ 256 ] = "";

/* Font specified with option -f */
char fontstr[ 32 ] = "";

/* Library bases */
struct Library *DiskfontBase = NULL;
struct Library *KeymapBase = NULL;
struct Library *GadToolsBase = NULL;

/* Term data structure */
typedef struct term_data
{
   term t;                  /* Term structure */
   cptr name;               /* Name string */

   BYTE use;                /* Use this window */

   BYTE cols;               /* Number of columns */
   BYTE rows;               /* Number of rows */

   UWORD wx;                /* Window x-pos */
   UWORD wy;                /* Window y-pos */
   UWORD ww;                /* Window width */
   UWORD wh;                /* Window height */

   BYTE fw;                 /* Font width */
   BYTE fh;                 /* Font height */
   BYTE fb;                 /* Font baseline */

   struct TextFont *font;   /* Font pointers */

   BYTE ownf;               /* Font is owned by this term */

   struct Window *win;      /* Window pointer */
   struct RastPort *wrp;    /* RastPort of window */
   struct RastPort *rp;     /* RastPort of screen or window */

   struct BitMap *gfxbm;
   struct BitMap *mapbm;
   struct BitMap *mskbm;

   int gfx_w;
   int gfx_h;

   int map_w;
   int map_h;
   int map_x;
   int map_y;

   int mpt_w;
   int mpt_h;

   int cursor_xpos;
   int cursor_ypos;
   int cursor_visible;
   int cursor_lit;
   int cursor_frame;
   int cursor_map;

   int notitle;
   int avoidbar;
}
term_data;

/* Term data for all windows */
static term_data screen;
static term_data mirror;
static term_data recall;
static term_data choice;

/* Window names */
static char screen_name[] = "Main";
static char choice_name[] = "Choice";
static char recall_name[] = "Recall";
static char mirror_name[] = "Mirror";

/* Screen pointers */
static struct Screen *amiscr = NULL;
static struct Screen *pubscr = NULL;

/* Visual info for gadtools menus */
static APTR *visinfo;

/* TextAttr for screen font */
static struct TextAttr ScrAttr, *scrattr = &ScrAttr;

/* Screen DisplayID */
static ULONG scr_m = 0;

/* Last term for cursor */
static term_data *term_curs = NULL;

/* Temporary string */
static char tmpstr[ 256 ];

/* Iconify status of windows */
static int iconified = FALSE;

/* Use intuition menus? */
static int use_menus = TRUE;

/* Use mouseblanking? */
static int blankmouse = FALSE;

/* Window input event */
static struct InputEvent ie;

/* KickStart 3.0+ present */
static int v39 = FALSE;

/* Use a public screen */
static int use_pub = FALSE;

/* Public screen lock */
static int publock = FALSE;

/* Use a backdrop main window */
static int backdrop = FALSE;

/* Use 32 colors on custom screen */
static int deep = FALSE;

/* Invisible pointer for blanking */
static __chip UWORD blankpointer[] = { 0,0,0,0,0,0,0,0 };

/* Pointer visibility status */
static int pointer_visible = TRUE;

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

/* Default color palette, 16 for graphics, 16 for text */
static ULONG default_colors[ 32 ] =
{
   0x000000, 0xf0e0d0, 0x808080, 0x505050,
   0xe0b000, 0xc0a070, 0x806040, 0x403020,
   0x00a0f0, 0x0000f0, 0x000070, 0xf00000,
   0x800000, 0x9000b0, 0x006010, 0x60f040,

   0x000000, 0xffffff, 0xc7c7c7, 0xff9200,
   0xff0000, 0x00cd00, 0x0000fe, 0xc86400,
   0x8a8a8a, 0xe0e0e0, 0xa500ff, 0xfffd00,
   0xff00bc, 0x00ff00, 0x00c8ff, 0xffcc80
};

/* Palette, 32 bits per gun */
static ULONG palette32[ 32 * 3 + 2 ];

/* Palette, 4 bits per gun */
static UWORD palette4[ 32 ];

/* Version string */
static char ver[] = "$VER: " VERSION " (" __DATE__ ")";

///}
///{ "sound"

#define NSAMPLES 25

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

static struct AmiSound sound_data[ NSAMPLES ] =
{
   { "intro.8svx",     64, 0, 0, 1, 0, NULL }, /* Intro */
   { "hit.8svx",       64, 0, 0, 1, 1, NULL }, /* Hit */
   { "miss.8svx",      64, 3, 0, 1, 1, NULL }, /* Miss */
   { "flee.8svx",      64, 1, 0, 1, 1, NULL }, /* Flee */
   { "drop.8svx",      64, 2, 0, 1, 1, NULL }, /* Drop */
   { "kill.8svx",      64, 1, 0, 1, 1, NULL }, /* Kill */
   { "study.8svx",     64, 2, 0, 1, 1, NULL }, /* Level */
   { "death.8svx",     64, 0, 0, 1, 0, NULL }, /* Death */
   { "study.8svx",     64, 2, 0, 1, 1, NULL }, /* Study */
   { "teleport.8svx",  64, 3, 0, 1, 1, NULL }, /* Teleport */
   { "shoot.8svx",     64, 0, 0, 1, 1, NULL }, /* Shoot */
   { "quaff.8svx",     64, 1, 0, 1, 1, NULL }, /* Quaff */
   { "zap.8svx",       64, 3, 0, 1, 1, NULL }, /* Zap */
   { "walk.8svx",      64, 0, 0, 1, 1, NULL }, /* Walk */
   { "tpother.8svx",   64, 3, 0, 1, 1, NULL }, /* Teleport Other */
   { "hitwall.8svx",   64, 3, 0, 1, 1, NULL }, /* Hit Wall */
   { "eat.8svx",       64, 3, 0, 1, 1, NULL }, /* Eat */
   { "store1.8svx",    64, 0, 0, 1, 0, NULL }, /* Shopkeeper furious */
   { "store2.8svx",    64, 0, 0, 1, 0, NULL }, /* Shopkeeper angry */
   { "store3.8svx",    64, 0, 0, 1, 0, NULL }, /* Shopkeeper glad */
   { "store4.8svx",    64, 0, 0, 1, 0, NULL }, /* Shopkeeper happy */
   { "dig.8svx",       64, 0, 0, 1, 1, NULL }, /* Dig */
   { "opendoor.8svx",  64, 0, 0, 1, 1, NULL }, /* Open door */
   { "closedoor.8svx", 64, 0, 0, 1, 1, NULL }, /* Close door */
   { "tplevel.8svx",   64, 0, 0, 1, 0, NULL }, /* Teleport level */
};

static int channel_last[ 4 ] = { -1, -1, -1, -1 };

static int has_sound = FALSE;

///}
///{ "menus"

#define MENUMAX 256

/* Menu userdata indexes */
#define MNU_SCALEDMAP   1001

/* Special offset indexes */
#define MNU_KEYCOM      2001
#define MNU_CKEYCOM     3001
#define MNU_OPTION      4001
#define MNU_HELP        5001

/* Macro for menu userdata keycodes and help */
#define MKC( c ) (void *)( MNU_KEYCOM + c )
#define MCC( c ) (void *)( MNU_CKEYCOM + c )
#define MHL( c ) (void *)( MNU_HELP + c )

struct Menu *menu = NULL;

/* Menu items that comes before the options */
struct NewMenu pre_item[] =
{
   { NM_TITLE, "Options", 0, 0, 0, 0 },
   { 255, 0, 0, 0, 0, 0 }
};

/* Option menu items, titles of submenus */
struct NewMenu opt_item[] =
{
   { NM_ITEM, "User Interface", 0, 0, 0, 0 },
   { NM_ITEM, "Disturbance", 0, 0, 0, 0 },
   { NM_ITEM, "Inventory", 0, 0, 0, 0 },
   { NM_ITEM, "Game-Play", 0, 0, 0, 0 },
   { NM_ITEM, "Efficiency", 0, 0, 0, 0 },
   { NM_ITEM, "Special", 0, 0, 0, 0 },
   { NM_ITEM, "Cheating", 0, 0, 0, 0 },
   { 255, 0, 0, 0, 0, 0 }
};

/* Menu items that comes after the options */
struct NewMenu post_item[] =
{
     /*
     { NM_ITEM, "Hitpoint Warning", 0, 0, 0, 0 },
       { NM_SUB, "90%", 0, CHECKIT, ~1, 0 },
       { NM_SUB, "80%", 0, CHECKIT, ~2, 0 },
       { NM_SUB, "70%", 0, CHECKIT, ~4, 0 },
       { NM_SUB, "60%", 0, CHECKIT, ~8, 0 },
       { NM_SUB, "50%", 0, CHECKIT, ~16, 0 },
       { NM_SUB, "40%", 0, CHECKIT, ~32, 0 },
       { NM_SUB, "30%", 0, CHECKIT, ~64, 0 },
       { NM_SUB, "20%", 0, CHECKIT, ~128, 0 },
       { NM_SUB, "10%", 0, CHECKIT, ~256, 0 },
       { NM_SUB, "Off", 0, CHECKIT, ~512, 0 },
     */

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
     { NM_ITEM, "V  Version info", 0, 0, 0, MKC('V') },

   { NM_TITLE, "Cmd4", 0, 0, 0, 0 },
     { NM_ITEM, "@  Interact with macros", 0, 0, 0, MKC('@') },
     { NM_ITEM, "%  Interact with visuals", 0, 0, 0, MKC('%') },
     { NM_ITEM, "&  Interact with colors", 0, 0, 0, MKC('&') },
     { NM_ITEM, "*  Target monster or location", 0, 0, 0, MKC('*') },
     { NM_ITEM, "(  Load screen dump", 0, 0, 0, MKC('(') },
     { NM_ITEM, ")  Dump screen dump", 0, 0, 0, MKC(')') },
     { NM_ITEM, "{  Inscribe an object", 0, 0, 0, MKC('{') },
     { NM_ITEM, "}  Uninscribe an object", 0, 0, 0, MKC('}') },
     { NM_ITEM, "-  Walk (flip pickup)", 0, 0, 0, MKC('-') },

   { NM_TITLE, "Cmd5", 0, 0, 0, 0 },
     { NM_ITEM, "+  Dig tunnel", 0, 0, 0, MKC('+') },
     { NM_ITEM, "=  Set options", 0, 0, 0, MKC('=') },
     { NM_ITEM, ";  Walk (with pickup)", 0, 0, 0, MKC(';') },
     { NM_ITEM, ":  Take notes", 0, 0, 0, MKC(':') },
     { NM_ITEM, "\"  Enter a user pref command", 0, 0, 0, MKC('\"') },
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
     { NM_ITEM, "^k  Quit", 0, 0, 0, MCC('k') },
     { NM_ITEM, "^p  Show previous messages", 0, 0, 0, MCC('p') },
     { NM_ITEM, "^r  Redraw the screen", 0, 0, 0, MCC('r') },
     { NM_ITEM, "^s  Save and don't quit", 0, 0, 0, MCC('s') },
     { NM_ITEM, "^x  Save and quit", 0, 0, 0, MCC('x') },
     { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
     { NM_ITEM, "Draw dungeon map", "m", 0, 0, (void *)MNU_SCALEDMAP },

   { NM_TITLE, "Help", 0, 0, 0, 0 },
     { NM_ITEM, "General Information", 0, 0, 0, MHL('1') },
     { NM_ITEM, "Creating a Character", 0, 0, 0, MHL('2') },
     { NM_ITEM, "Exploring the Dungeon", 0, 0, 0, MHL('3') },
     { NM_ITEM, "Attacking Monsters", 0, 0, 0, MHL('4') },
     { NM_ITEM, "List of Commands", 0, 0, 0, MHL('5') },
     { NM_ITEM, "List of Options", 0, 0, 0, MHL('6') },
     { NM_ITEM, "Version Information", 0, 0, 0, MHL('7') },

   { NM_END, NULL, 0, 0, 0, 0 },
   { 255, 0, 0, 0, 0, 0 }
};

/* Menu array */
struct NewMenu newmenu[ MENUMAX ];

///}
///{ "protos"

extern void map_info( int y, int x, byte *ap, char *cp );
extern void center_string( char *buf, cptr str );
errr init_ami( void );
static void init_term( term_data *td );
static term *link_term( term_data *td );
static void free_term( term_data *td );
static char *stripstr( char *src, char *dst );
void request_font( char *str );
void request_mode( char *str );
int read_prefs( void );
static void amiga_open( term *t );
static void amiga_nuke( term *t );
static errr amiga_curs( int x, int y );
static errr amiga_wipe( int x, int y, int n );
static errr amiga_clear( void );
static errr amiga_pict( int x, int y, byte a, char c );
static errr amiga_text( int x, int y, int n, byte a, cptr s );
static errr amiga_xtra( int n, int v );
static errr amiga_flush( int v );
static errr amiga_event( int v );
static errr amiga_react( int v );
static errr amiga_fail( char *msg );
static void cursor_on( term_data *td );
static void cursor_off( term_data *td );
void amiga_tomp( void );
void tomb_str( int y, char *str );
int load_gfx( void );
int conv_gfx( void );
int size_gfx( term_data *td );
void scale_bitmap( struct BitMap *srcbm, int srcw, int srch, struct BitMap *dstbm, int dstw, int dsth );
void remap_bitmap( struct BitMap *srcbm, struct BitMap *dstbm, long *pens, int width, int height );
static void put_gfx( struct RastPort *rp, int x, int y, int chr, int col );
static void put_gfx_map( term_data *td, int x, int y, int c, int a );
static void cursor_anim( void );
void load_palette( void );
static void amiga_map( void );
int init_sound( void );
void free_sound( void );
static void play_sound( int v );
struct BitMap *alloc_bitmap( int width, int height, int depth, ULONG flags, struct BitMap *friend );
void free_bitmap( struct BitMap *bitmap );
int depth_of_bitmap( struct BitMap *bm );
int create_menus( void );
void update_menus( void );
void handle_menupick( int mnum );
void handle_rawkey( UWORD code, UWORD qual, APTR addr );

///}

///{ "init_ami()" - Initialize all Amiga spesific stuff

errr init_ami( void )
{
   char *s;
   int i;
   LONG pen;
   struct DimensionInfo diminfo;
   int pw,ph,maxw,maxh,th,barh;

   /* Term data pointers */
   term_data *ts = &screen;
   term_data *tc = &choice;
   term_data *tr = &recall;
   term_data *tm = &mirror;

   /* Clear the term data */
   init_term( ts );
   init_term( tc );
   init_term( tr );
   init_term( tm );

   /* Always use the main term */
   ts->use = TRUE;

   /* We *must* have kickstart 37 or later */
   if ( IntuitionBase->LibNode.lib_Version < 37 )
      FAIL( "Sorry, this program requires KickStart 2.04 or later." );

   /* Check if we have kickstart 39 or later */
   v39 = ( IntuitionBase->LibNode.lib_Version >= 39 );

   /* Open diskfont.library */
   if (( DiskfontBase = OpenLibrary( "diskfont.library", 0 )) == NULL )
      FAIL( "Unable to open diskfont.library." );

   /* Read preferences file */
   read_prefs();

   /* Initialize keyboard stuff */
   ie.ie_NextEvent = NULL;
   ie.ie_Class     = IECLASS_RAWKEY;
   ie.ie_SubClass  = 0;
   if (( KeymapBase = OpenLibrary( "keymap.library", 36 )) == NULL )
      FAIL( "Unable to open keymap.library v36+." );

   /* Open gadtools.library */
   if ( use_menus )
   {
      if (( GadToolsBase = OpenLibrary( "gadtools.library", 36 )) == NULL )
      {
         use_menus = FALSE;
      }
   }

   /* Initialize color palette */
   for ( i = 0; i < 32; i++ )
   {
      /* If undefined, use default palette */
      if ( color_table[ i ][ 0 ] == 0 &&
           color_table[ i ][ 1 ] == 0 &&
           color_table[ i ][ 2 ] == 0 &&
           color_table[ i ][ 3 ] == 0 )
      {
         color_table[ i ][ 0 ] = 1;
         color_table[ i ][ 1 ] = ( default_colors[ i ] & 0xff0000 ) >> 16;
         color_table[ i ][ 2 ] = ( default_colors[ i ] & 0x00ff00 ) >> 8;
         color_table[ i ][ 3 ] = ( default_colors[ i ] & 0x0000ff );
      }
   }

   /* Search for prefered screenmode or public screen */
   if ( strlen( modestr ) > 0 )
   {
      /* Convert string to long */
      scr_m = strtol( modestr, &s, 0 );

      /* It was not a number, so treat it as a public screen name */
      if ( scr_m == 0 )
      {
         /* We need kickstart 3.0+ to use a public screen */
         if ( !v39 )
         {
            FAIL( "Public screen can only be used on kickstart 3.0 or later." );
         }

         /* Try to lock the named public screen if it isn't already */
         if ( !pubscr )
         {
            pubscr = LockPubScreen( modestr );
         }

         /* Failed */
         if ( !pubscr ) {
            sprintf( tmpstr, "Unable to get a lock on screen '%s'.", modestr );
            FAIL( tmpstr );
         }

         /* We got a lock now */
         publock = TRUE;

         scr_m = -1;

         use_pub = TRUE;

         /* Don't blank mouse on public screen */
         blankmouse = FALSE;

         /* Find suitable pens to use on public screen */
         for ( i = 0; i < 32; i++ )
         {
            pen = ObtainBestPen( pubscr->ViewPort.ColorMap,
                                 color_table[ i ][ 1 ] << 24,
                                 color_table[ i ][ 2 ] << 24,
                                 color_table[ i ][ 3 ] << 24,
                                 OBP_Precision, PRECISION_EXACT );
            if( pen == -1 )
            {
               FAIL( "Unable to obtain suitable pens to use on public screen. ");
            }

            pubpens[ i ] = pen;
         }

         for ( i = 0; i < 16; i++ ) penconv[ i ] = (UWORD) pubpens[ i + 16 ];
      }

      /* Use specified screenmode if available */
      else
      {
         /* Check if requested mode is available */
         if ( ModeNotAvailable( scr_m )) scr_m = 0;
      }
   }

   if ( !use_pub )
   {
      /* Extra windows not allowed on custom screen */
      tc->use = tr->use = tm->use = FALSE;
   }

   /* Calculate window dimensions */
   ts->ww = ts->fw * ts->cols; ts->wh = ts->fh * ts->rows;
   tc->ww = tc->fw * tc->cols; tc->wh = tc->fh * tc->rows;
   tr->ww = tr->fw * tr->cols; tr->wh = tr->fh * tr->rows;
   tm->ww = tm->fw * tm->cols; tm->wh = tm->fh * tm->rows;

   /* Find a nice screenmode */
   if ( scr_m == 0 && v39 )
   {
      scr_m = BestModeID(
                  BIDTAG_NominalWidth, ts->ww,
                  BIDTAG_NominalHeight, ts->wh,
                  BIDTAG_DesiredWidth, ts->ww,
                  BIDTAG_DesiredHeight, ts->wh,
                  BIDTAG_Depth, 4,
                  TAG_END );
   }

   /* Use default screenmode if we don't have any */
   if ( scr_m == 0 || scr_m == INVALID_ID )
   {
      scr_m = ( DEFAULT_MONITOR_ID | HIRES_KEY );
   }

   /* Open custom screen */
   if ( !use_pub )
   {
      /* Use 32 colors with graphics */
      if ( use_graphics )
      {
         /* Get dimension data for screenmode */
         if ( GetDisplayInfoData( NULL, (UBYTE *) &diminfo, sizeof( struct DimensionInfo ), DTAG_DIMS, scr_m ))
         {
            /* Check if we support deep screens */
            if ( diminfo.MaxDepth > 4 )
            {
               /* Use 32 colors */
               deep = TRUE;

               /* Use colors 16..31 for text */
               for ( i = 0; i < 16; i++ ) penconv[ i ] = i + 16;
            }
         }
      }

      /* Use only 16 colors with no graphics */
      if ( !use_graphics )
      {
         /* Use 16 colors */
         deep = FALSE;

         /* Use colors 0..15 for text */
         for ( i = 0; i < 16; i++ ) penconv[ i ] = i;
      }

      if (( amiscr = OpenScreenTags( NULL,
             SA_Width, ts->ww,
             SA_Height, ts->wh,
             SA_Depth, deep ? 5 : 4,
             SA_DisplayID, scr_m,
             SA_Font, scrattr,
             SA_Type, CUSTOMSCREEN,
             SA_Title, "Angband Screen",
             SA_ShowTitle, FALSE,
             SA_Quiet, TRUE,
             SA_Behind, TRUE,
             SA_AutoScroll, TRUE,
             SA_Overscan, OSCAN_TEXT,
             v39 ? SA_Interleaved : TAG_IGNORE, TRUE,
             TAG_END )) == NULL)
      {
         FAIL( "Unable to open screen." );
      }

      /* Initialize screen rastport */
      ts->rp = &amiscr->RastPort;
      SetRast( ts->rp, PEN( 0 ));
      SetAPen( ts->rp, 1 );
      SetBPen( ts->rp, 0 );
      SetDrMd( ts->rp, JAM2 );
      SetFont( ts->rp, ts->font );

      /* Always use backdrop window on custom screen */
      backdrop = TRUE;
   }

   /* We are using a public screen */
   else
   {
      /* Size of public screen */
      pw = pubscr->Width;
      ph = pubscr->Height;

      /* Height difference between a window with or without a title bar */
      th = pubscr->Font->ta_YSize + 1;

      /* Find width of widest window */
      maxw = ts->ww;
      maxw = tc->ww > maxw ? tc->ww : maxw;
      maxw = tr->ww > maxw ? tr->ww : maxw;
      maxw = tm->ww > maxw ? tm->ww : maxw;
      maxw += pubscr->WBorLeft + pubscr->WBorRight;

      /* Find height of tallest window */
      maxh = ts->wh + ts->notitle ? 0 : th;
      maxh = ( tc->wh + tc->notitle ? 0 : th ) > maxh ? ( tc->wh + tc->notitle ? 0 : th ) : maxh;
      maxh = ( tr->wh + tr->notitle ? 0 : th ) > maxh ? ( tr->wh + tr->notitle ? 0 : th ) : maxh;
      maxh = ( tm->wh + tm->notitle ? 0 : th ) > maxh ? ( tm->wh + tm->notitle ? 0 : th ) : maxh;
      maxh += pubscr->WBorTop + pubscr->WBorBottom;

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

      /* Check if windows are to be positioned at the right or bottom */
      if ( ts->wx == -1 ) ts->wx = pw - 1;
      if ( ts->wy == -1 ) ts->wy = ph - 1;
      if ( tc->wx == -1 ) tc->wx = pw - 1;
      if ( tc->wy == -1 ) tc->wy = ph - 1;
      if ( tr->wx == -1 ) tr->wx = pw - 1;
      if ( tr->wy == -1 ) tr->wy = ph - 1;
      if ( tm->wx == -1 ) tm->wx = pw - 1;
      if ( tm->wy == -1 ) tm->wy = ph - 1;

      /* Position windows below screen bar if requested */
      if ( ts->wy == -2 ) ts->wy = barh;
      if ( tc->wy == -2 ) tc->wy = barh;
      if ( tr->wy == -2 ) tr->wy = barh;
      if ( tm->wy == -2 ) tm->wy = barh;
   }

   /* Get visual info for GadTools */
   if ( use_menus )
   {
      if (( visinfo = GetVisualInfo( use_pub ? pubscr : amiscr, TAG_END )) == NULL )
      {
         use_menus = FALSE;
      }
   }

   /* Open window, backdrop if on custom screen */
   if (( ts->win = OpenWindowTags( NULL,
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
          WA_IDCMP, IDCMP_RAWKEY | IDCMP_INTUITICKS | IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS | IDCMP_MENUPICK | IDCMP_MENUVERIFY,
          TAG_END )) == NULL )
   {
      FAIL("Unable to open window.");
   }

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
   if ( use_pub )
   {
      ts->rp = ts->wrp;
   }

   /* Use mirror window? */
   if ( tm->use )
   {
      /* Open mirror window */
      if (( tm->win = OpenWindowTags( NULL,
              WA_Left, tm->wx,
              WA_Top, tm->wy,
              WA_InnerWidth, tm->ww,
              WA_InnerHeight, tm->wh,
              WA_PubScreen, pubscr,
              WA_GimmeZeroZero, TRUE,
              WA_DragBar, !tm->notitle,
              WA_DepthGadget, !tm->notitle,
              WA_NewLookMenus, TRUE,
              WA_ScreenTitle, VERSION,
              tm->notitle ? TAG_IGNORE : WA_Title, tm->name,
              WA_ReportMouse, TRUE,
              TAG_END )) == NULL )
      {
         FAIL("Unable to open recall window.");
      }

      /* Initialize mirror rastport */
      tm->rp = tm->wrp = tm->win->RPort;
      SetRast( tm->rp, PEN( 0 ));
      SetAPen( tm->rp, 1 );
      SetBPen( tm->rp, 0 );
      SetDrMd( tm->rp, JAM2 );
      SetFont( tm->rp, tm->font );
   }

   /* Use recall window? */
   if ( tr->use )
   {
      /* Open recall window */
      if (( tr->win = OpenWindowTags( NULL,
              WA_Left, tr->wx,
              WA_Top, tr->wy,
              WA_InnerWidth, tr->ww,
              WA_InnerHeight, tr->wh,
              WA_PubScreen, pubscr,
              WA_GimmeZeroZero, TRUE,
              WA_DragBar, !tr->notitle,
              WA_DepthGadget, !tr->notitle,
              WA_NewLookMenus, TRUE,
              WA_ScreenTitle, VERSION,
              tr->notitle ? TAG_IGNORE : WA_Title, tr->name,
              WA_ReportMouse, TRUE,
              TAG_END )) == NULL )
      {
         FAIL("Unable to open recall window.");
      }

      /* Initialize recall rastport */
      tr->rp = tr->wrp = tr->win->RPort;
      SetRast( tr->rp, PEN( 0 ));
      SetAPen( tr->rp, 1 );
      SetBPen( tr->rp, 0 );
      SetDrMd( tr->rp, JAM2 );
      SetFont( tr->rp, tr->font );
   }

   /* Use choice window? */
   if ( tc->use )
   {
      /* Open choice window */
      if (( tc->win = OpenWindowTags( NULL,
              WA_Left, tc->wx,
              WA_Top, tc->wy,
              WA_InnerWidth, tc->ww,
              WA_InnerHeight, tc->wh,
              WA_PubScreen, pubscr,
              WA_GimmeZeroZero, TRUE,
              WA_DragBar, !tc->notitle,
              WA_DepthGadget, !tc->notitle,
              WA_NewLookMenus, TRUE,
              WA_ScreenTitle, VERSION,
              tc->notitle ? TAG_IGNORE : WA_Title, tc->name,
              WA_ReportMouse, TRUE,
              TAG_END )) == NULL )
      {
         FAIL("Unable to open recall window.");
      }

      /* Initialize choice rastport */
      tc->rp = tc->wrp = tc->win->RPort;
      SetRast( tc->rp, PEN( 0 ));
      SetAPen( tc->rp, 1 );
      SetBPen( tc->rp, 0 );
      SetDrMd( tc->rp, JAM2 );
      SetFont( tc->rp, tc->font );
   }

   /* Create palette for screen */
   load_palette();

   /* Link terms with the metaterm */
   if ( tc->use ) term_choice = link_term( tc );
   if ( tr->use ) term_recall = link_term( tr );
   if ( tm->use ) term_mirror = link_term( tm );
   if ( ts->use ) term_screen = link_term( ts );

   /* Bring main window to front */
   if ( !backdrop ) WindowToFront( ts->win );

   /* Bring screen to front */
   ScreenToFront( use_pub ? pubscr : amiscr );

   /* Load and convert graphics */
   if ( use_graphics )
   {
      MSG( 0, 0, "Loading graphics" );
      if ( !load_gfx() ) FAIL( NULL );

      /* Check if conversion is necessary */
      if ( use_pub || ( depth_of_bitmap( ts->rp->BitMap ) != depth_of_bitmap( ts->gfxbm )))
      {
         MSG( 0, 1, "Remapping graphics" );
         if ( !conv_gfx() ) FAIL( "Out of memory while remapping graphics." );
      }

      /* Scale the graphics to fit font sizes */
      if ( tm->use ) if ( !size_gfx( tm ) ) FAIL( "Out of memory while scaling graphics." );
      if ( tr->use ) if ( !size_gfx( tr ) ) FAIL( "Out of memory while scaling graphics." );
      if ( tc->use ) if ( !size_gfx( tc ) ) FAIL( "Out of memory while scaling graphics." );
      if ( ts->use ) if ( !size_gfx( ts ) ) FAIL( "Out of memory while scaling graphics." );
   }

   /* Load sound effects */
   if ( use_sound )
   {
      MSG( 0, 2, "Loading sound effects" );
      init_sound();
   }

   /* Success */
   return ( 0 );
}
///}
///{ "init_term()"

static void init_term( term_data *td )
{
   /* Set term name */
   if ( td == &screen ) td->name = screen_name;
   if ( td == &choice ) td->name = choice_name;
   if ( td == &recall ) td->name = recall_name;
   if ( td == &mirror ) td->name = mirror_name;

   /* Term off */
   td->use = FALSE;

   /* Term size */
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
   td->fw   = td->font->tf_XSize;
   td->fh   = td->font->tf_YSize;
   td->fb   = td->font->tf_Baseline;

   /* No window or rastports */
   td->win  = NULL;
   td->wrp  = NULL;
   td->rp   = NULL;

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

///}
///{ "link_term()"

static term *link_term( term_data *td )
{
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

   /* We are emulating a hardware cursor */
   t->soft_cursor = FALSE;

   /* Draw graphical tiles one by one */
   t->always_text = FALSE;
   t->always_pict = FALSE;
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

   /* Return pointer to the term */
   return( t );
}

///}
///{ "free_term()"

static void free_term( term_data *td )
{
   /* Do we own the font? */
   if ( td->ownf && td->font ) CloseFont( td->font );

   /* Is the window opened? */
   if ( td->win ) CloseWindow( td->win );

   /* Free bitmaps */
   if ( td->gfxbm ) free_bitmap( td->gfxbm );
   if ( td->mskbm ) free_bitmap( td->mskbm );
   if ( td->mapbm ) free_bitmap( td->mapbm );
}

///}
///{ "stripstr()"

static char *stripstr( char *src, char *dst )
{
   int len;

   /* Ignore leading spaces */
   while ( *src == ' ' ) src++;

   /* Copy string */
   for ( len = 0; *src != 0; len++ ) *dst++ = *src++; *dst = 0;

   /* Remove trailing spaces */
   for ( dst--; *dst == ' ' && len; len-- ) *dst-- = 0;

   /* Return pointer to destination */
   return( dst );
}

///}
///{ "yesno()"

static int yesno( char *str )
{
   char tmp[ 256 ];
   char *s;

   /* Strip spaces around string */
   stripstr( str, tmp);

   /* Make lowercase */
   for ( s = tmp; *s != 0; s++ ) *s = tolower( *s );

   return ( !strcmp( tmp, "y" ) || !strcmp( tmp, "yes" ) || !strcmp( tmp, "true" ) || !strcmp( tmp, "on" ));
}

///}
///{ "request_font()"

void request_font( char *str )
{
   struct Library *AslBase = NULL;
   struct FontRequester *req = NULL;

   /* Blank string as default */
   *str = 0;

   /* Open asl.library */
   if ( AslBase = OpenLibrary ( "asl.library", 37 ))
   {
      /* Allocate screenmode requester */
      if ( req = AllocAslRequestTags( ASL_FontRequest, TAG_DONE ))
      {
         /* Open screenmode requester */
         if( AslRequestTags( req, ASLFO_FixedWidthOnly, TRUE, TAG_DONE ))
         {
            /* Store font name and size */
            sprintf( str, "%s/%d", req->fo_Attr.ta_Name, req->fo_Attr.ta_YSize );
         }
         /* Free requester */
         FreeAslRequest( req );
      }
      /* Close asl.library */
      CloseLibrary( AslBase );
   }
}

///}
///{ "request_mode()"

void request_mode( char *str )
{
   struct Library *AslBase = NULL;
   struct ScreenModeRequester *req = NULL;

   /* Blank string as default */
   *str = 0;

   /* Open asl.library */
   if ( AslBase = OpenLibrary ( "asl.library", 37 ))
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
      /* Close asl.library */
      CloseLibrary( AslBase );
   }
}

///}
///{ "read_prefs()"

int read_prefs( void )
{
   BPTR file;
   char line[ 256 ];
   char fname[ 256 ];
   char *tmp, *s;
   int len;
   int fsize;
   int val;
   struct TextAttr attr;
   term_data *td;

   /* Open config file */
   if (( file = Open( WPRF, MODE_OLDFILE )) == NULL )
   {
      fprintf( stderr, "\nUnable to open file '%s'.\n", WPRF );
      return( FALSE );
   }

   /* Read next line from file */
   while ( FGets( file, line, 256 ))
   {
      /* Cut off comments */
      if ( tmp = strchr( line, ';' )) *tmp = 0;

      /* Length of line */
      len = strlen( line );

      /* Cut off trailing newline and blanks */
      for ( tmp = line + len - 1; len > 0 && ( *tmp == '\n' || *tmp == ' ' ); len-- ) *tmp-- = 0;

      /* Ignore blank lines */
      if ( len == 0 ) continue;

      /* Extract term */
      if ( strncmp( line, "MAIN.", 5 ) == 0 ) td = &screen;
      else if ( strncmp( line, "CHOICE.", 7 ) == 0 ) td = &choice;
      else if ( strncmp( line, "RECALL.", 7 ) == 0 ) td = &recall;
      else if ( strncmp( line, "MIRROR.", 7 ) == 0 ) td = &mirror;
      else if ( strncmp( line, "SCREEN.", 7 ) != 0 &&
                strncmp( line, "ANGBAND.", 8 ) != 0 )
      {
         printf( "PREFS: Error in line '%s'\n", line );
         continue;
      }

      /* Find start of option */
      tmp = strchr( line, '.' ) + 1;

      /* Ignore blank options */
      if ( *tmp == 0 ) continue;

      /* Option 'use' - Use this term */
      if ( !strncmp( tmp, "use", 3 ))
      {
         td->use = yesno( tmp + 3 );
      }

      /* Option 'cols' - Set number of columns for term */
      else if ( !strncmp( tmp, "cols", 4 ) && sscanf( tmp + 4, "%d", &val )) td->cols = val;

      /* Option 'rows' - Set number of rows for term */
      else if ( !strncmp( tmp, "rows", 4 ) && sscanf( tmp + 4, "%d", &val )) td->rows = val;

      /* Option 'xpos' - Set horizontal position for window */
      else if ( !strncmp( tmp, "xpos", 4 ) && sscanf( tmp + 4, "%d", &val )) td->wx = val;

      /* Option 'ypos' - Set vertical position for window */
      else if ( !strncmp( tmp, "ypos", 4 ) && sscanf( tmp + 4, "%d", &val )) td->wy = val;

      /* Option 'name' - Set window title */
      else if ( !strncmp( tmp, "title", 5 ))
      {
         /* Get parameter */
         stripstr( tmp + 5, fname );

         /* Make a copy of the title */
         if ( strlen( fname ) > 0 )
         {
            td->name = strdup( fname );
         }

         /* Don't use a title bar on this window */
         else
         {
            td->notitle = TRUE;
         }

         continue;
      }

      /* Option 'font' - Set font for this window */
      else if ( !strncmp( tmp, "font", 4 ))
      {
         /* Get value */
         stripstr( tmp + 4, fname );

         /* Ask for font? */
         if ( !strcmp( fname, "?" ))
         {
            /* Open font requester */
            request_font( fname );
         }

         /* No font specification given */
         if ( strlen( fname ) == 0 )
         {
            /* Main window */
            if ( td == &screen )
            {
               /* System default font */
               td->font = GfxBase->DefaultFont;

               /* Use default font as screen font */
               scrattr = NULL;
            }

            /* Extra window */
            else
            {
               /* Copy main window's font */
               td->font = screen.font;
            }

            /* Set font dimensions */
            td->fw   = td->font->tf_XSize;
            td->fh   = td->font->tf_YSize;
            td->fb   = td->font->tf_Baseline;

            /* This font is not opened by us */
            td->ownf = FALSE;

            /* Next line */
            continue;
         }

         /* Get name and size */
         else
         {
            /* Find font name/size delimiter */
            if (( s = strchr( fname, '/' )) == NULL )
            {
               printf( "\nPREFS: Illegal font specification: '%s'.\n", fname );
               continue;
            }

            /* End fontname here */
            *s++ = 0;

            /* Convert size string to integer */
            fsize = atoi( s );

            /* Make sure the font name ends with .font */
            if ( !strstr( fname, ".font" )) strcat( fname, ".font" );
         }

         /* Set font attributes */
         attr.ta_Name  = fname;
         attr.ta_YSize = fsize;
         attr.ta_Style = FS_NORMAL;
         attr.ta_Flags = ( !strcmp( fname, "topaz.font" ) && ( fsize == 8 || fsize == 9 )) ?
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
            if ( td == &screen )
            {
               scrattr->ta_Name = strdup( fname );
               scrattr->ta_YSize = fsize;
               scrattr->ta_Style = FS_NORMAL;
               scrattr->ta_Flags = attr.ta_Flags;
            }
         }

         /* The font could not be opened */
         else
         {
            /* Fallback to default font */
            td->font = GfxBase->DefaultFont;

            /* Use default font as screen font */
            scrattr = NULL;

            /* Output error message */
            printf( "\nUnable to open font '%s/%d'.\n", fname, fsize );
         }
      }

      /* Option .name - Set public screen name. KS 3.0+ required */
      else if ( !strncmp( tmp, "name", 4 ) && v39 )
      {
         /* Copy name */
         stripstr( tmp + 4, modestr );
      }

      /* Option .mode - Set custom screen mode */
      else if ( !strncmp( tmp, "mode", 4 ))
      {
         /* If a public screen was also specified, check if it exist */
         if ( strlen( modestr ) > 0 )
         {
            /* Don't lock it twice */
            if ( pubscr ) continue;

            /* Try to lock the named public screen */
            if ( pubscr = LockPubScreen( modestr ))
            {
               /* We got a lock now */
               publock = TRUE;

               /* Screen exist. Skip this option */
               continue;
            }
         }

         /* Get parameter */
         stripstr( tmp + 4, modestr );

         /* Ask for mode? */
         if ( !strcmp( modestr, "?" ))
         {
            /* Open screenmode requester */
            request_mode( modestr );
         }
      }

      /* Option .blankmouse - Use mouseblanking */
      else if ( !strncmp( tmp, "blankmouse", 10 ))
      {
         blankmouse = yesno( tmp + 10 );
      }

      /* Option .blankmouse - Use intuition menus */
      else if ( !strncmp( tmp, "menus", 5 ))
      {
         use_menus = yesno( tmp + 5 );
      }

      else if ( !strncmp( tmp, "sound", 5 ))
      {
         use_sound = yesno( tmp + 5 );
      }

      else if ( !strncmp( tmp, "gfx", 3 ))
      {
         use_graphics = yesno( tmp + 5 );
      }

      /* Unknown option */
      else
      {
         /* Output error message */
         printf ( "\nPREFS: Unknown option '%s'.\n", tmp );
      }
   }

   /* Close the file */
   Close( file );
}

///}
///{ "amiga_nuke()" - Free all allocated resources

static void amiga_nuke( term *t )
{
   if ( t == term_screen )
   {
      amiga_fail( NULL );
      /* Flush the output */
      fflush( stdout );
   }
}

///}
///{ "amiga_open()" - Initialize terminal

static void amiga_open( term *t )
{
   /* Nothing to do here */
}

///}
///{ "amiga_curs()" - Move the cursor to a new location

static errr amiga_curs( int x, int y )
{
   term_data *td = term_curs;

   if ( td->cursor_visible )
   {
      cursor_off( td );
   }

   td->cursor_xpos = x;
   td->cursor_ypos = y;
   td->cursor_frame = 0;

   if ( td->cursor_visible )
   {
      cursor_on( td );
   }

   return ( 0 );
}

///}
///{ "amiga_wipe()" - Erase a part of a line

static errr amiga_wipe( int x, int y, int n )
{
   term_data *td = (term_data*)(Term->data);

   if (( n > 0 ) && !iconified )
   {
      /* Erase rectangular area on screen */
      SetAPen( td->rp, PEN( 0 ) );
      RectFill( td->rp, x * td->fw, y * td->fh, ( x + n ) * td->fw - 1, ( y + 1 ) * td->fh - 1 );
   }

   return ( 0 );
}

///}
///{ "amiga_clear()" - Clear whole window

static errr amiga_clear( void )
{
   term_data *td = (term_data*)(Term->data);

   /* Fill window with background color */
   SetRast( td->rp, PEN( 0 ));

   return ( 0 );
}

///}
///{ "amiga_pict()" - Place one tile on the screen

static errr amiga_pict( int x, int y, byte a, char c )
{
   term_data *td = (term_data*)(Term->data);
   char s[2];

   /* Graphical tile */
   if ( a & 0x80 )
   {
      put_gfx( td->rp, x, y, c & 0x7f, a & 0x7f );
   }

   /* Textual character */
   else
   {
      s[0] = c;
      s[1] = 0;
      SetAPen( td->rp, PEN( a & 0x0f ));
      SetBPen( td->rp, PEN( 0 ));
      Move( td->rp, x * td->fw, y * td->fh + td->fb );
      Text( td->rp, (char *) s, 1 );
   }

   return ( 0 );
}

///}
///{ "amiga_text()" - Place some text on the screen using an attribute

static errr amiga_text( int x, int y, int n, byte a, cptr s )
{
   term_data *td = (term_data*)(Term->data);
   int i;

   if ( x >= 0 && y >= 0 && n > 0 && !iconified )
   {
      /* Draw gfx one char at a time */
      if (( a & 0xc0 ))
      {
         for ( i = 0; i < n; i++ ) put_gfx( td->rp, x + i, y, s[ i ] & 0x7f, a & 0x7f );
      }

      /* Draw the string on screen */
      else
      {
         SetAPen( td->rp, PEN( a&0x0f ));
         SetBPen( td->rp, PEN( 0 ));
         Move( td->rp, x * td->fw, y * td->fh + td->fb );
         Text( td->rp, (char *) s, n );
      }
   }

   return ( 0 );
}

///}
///{ "amiga_xtra()" - Handle a "special request"

static errr amiga_xtra( int n, int v )
{
   term_data *td = (term_data*)(Term->data);

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
         {
            play_sound( v );
         }
         return ( 0 );


      /* React on global changes */
      case TERM_XTRA_REACT:

         return ( amiga_react( v ));


      case TERM_XTRA_LEVEL:

         term_curs = td;
         return( 0 );

      case TERM_XTRA_DELAY:
      
         if (v >= 20) Delay(v / 20);
         return (0);
      
      /* Unknown request type */
      default:

         return ( 1 );

   }

   /* Shouldn't be able to get here */
   return ( 1 );
}

///}
///{ "amiga_flush()" - Flush input buffer

static errr amiga_flush( int v )
{
   struct IntuiMessage *imsg;

   /* Ignore all messages at the port */
   while ( imsg = (struct IntuiMessage *) GetMsg( screen.win->UserPort ))
   {
      ReplyMsg(( struct Message *) imsg );
   }

   return ( 1 );
}

///}
///{ "amiga_event()" - Wait for an event, and handle it.

static errr amiga_event( int v )
{
   struct IntuiMessage *imsg;
   ULONG iclass;
   UWORD icode;
   UWORD iqual;
   APTR iaddr;

   /* Check for messages to the window */
   if (( imsg = (struct IntuiMessage *) GetMsg( screen.win->UserPort )) == NULL )
   {
      /* If we don't want blocking, return */
      if ( !v )
      {
         return ( 0 );
      }

      /* No messages, so wait for one */
      Wait( 1 << screen.win->UserPort->mp_SigBit );

      /* Get the new message */
      imsg = (struct IntuiMessage *) GetMsg( screen.win->UserPort );
   }

   /* Handle message */
   if ( imsg )
   {
      /* Get message attributes */
      iclass = imsg->Class;
      icode = imsg->Code;
      iqual = imsg->Qualifier;
      iaddr = imsg->IAddress;

      /* Update menus before displaying */
      if ( iclass == IDCMP_MENUVERIFY && icode == MENUHOT && use_menus )
      {
         update_menus();
      }

      /* Reply the message */
      ReplyMsg(( struct Message *) imsg );

      /* Do we have a keypress? */
      if ( iclass == IDCMP_RAWKEY )
      {
         handle_rawkey( icode, iqual, iaddr );
         return ( 0 );
      }

      /* Mouse event - Make pointer visible */
      if ( iclass == IDCMP_MOUSEMOVE ||
           iclass == IDCMP_MOUSEBUTTONS ||
           iclass == IDCMP_MENUVERIFY )
      {
         if ( blankmouse && !pointer_visible )
         {
            ClearPointer( screen.win );
            pointer_visible = TRUE;
         }

         return ( 0 );
      }

      /* Time for some cursor anim? */
      if ( iclass == IDCMP_INTUITICKS )
      {
         cursor_anim();
         return ( 0 );
      }

      /* Menu item picked? */
      if ( iclass == IDCMP_MENUPICK )
      {
         handle_menupick( icode );
      }

      /* Unknown message class */
      return ( 1 );
   }

   /* No events */
   return ( 1 );
}

///}
///{ "amiga_react()"

static errr amiga_react( int v )
{
   /* Apply color palette, in case it has changed */
   load_palette();

   /* Create menus if we don't have any */
   if ( !menu && use_menus )
   {
      create_menus();
   }

   return ( 0 );
}

///}
///{ "amiga_tomb()"

int amiga_tomb( void )
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
   BYTE *src, *dst;
   int byt;

   int tw = screen.fw * 64;
   int th = screen.fh * 21;

   /* Allocate bitmap for tomb graphics file */
   if (( filebm = alloc_bitmap( 512, 168, 4, BMF_CLEAR, screen.rp->BitMap )) == NULL )
   {
      return( FALSE );
   }

   /* Open tomb file */
   if (( file = Open( MTOM, MODE_OLDFILE )) == NULL )
   {
      free_bitmap( filebm );
      return( FALSE );
   }

   /* Read file into bitmap */
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
   depth = depth_of_bitmap( screen.rp->BitMap );

   /* Remapping needed? */
   if ( depth != 4 || use_pub )
   {
      /* Allocate bitmap for remapped image */
      if (( convbm = alloc_bitmap( 512, 168, depth, BMF_CLEAR, screen.rp->BitMap )) == NULL )
      {
         free_bitmap( filebm );
         return( FALSE );
      }

      /* Simple remapping from 4 to 5 planes? */
      if ( depth == 5 && !use_pub )
      {
         for ( plane = 0; plane < 4; plane++ )
         {
            src = filebm->Planes[ plane ];
            dst = convbm->Planes[ plane ];
            for ( row = 0; row < 168; row++ )
            {
               for ( byt = 0; byt < 64; byt++ )
               {
                  dst[ byt ] = src[ byt ];
               }
               src += filebm->BytesPerRow;
               dst += convbm->BytesPerRow;
            }
         }
      }

      /* Complex remapping */
      else
      {
         /* Remap old bitmap into new bitmap */
         remap_bitmap( filebm, convbm, use_pub ? pubpens : stdpens, 512, 168 );
      }

      /* Free original bitmap */
      free_bitmap( filebm );
   }

   /* No remapping needed */
   else
   {
      convbm = filebm;
   }

   /* Allocate bitmap for scaled graphics */
   if (( scalbm = alloc_bitmap( tw, th, depth, BMF_CLEAR, screen.rp->BitMap )) == NULL )
   {
      free_bitmap( convbm );
      return ( FALSE );
   }

   /* Scale the tomb bitmap */
   scale_bitmap( convbm, 512, 168, scalbm, tw, th );

   /* Free old bitmap */
   free_bitmap( convbm );

   /* Copy the tomb graphics to the screen, centered */
   BltBitMapRastPort( scalbm, 0, 0, screen.rp, ( screen.ww - tw ) / 2, 0, tw, th, 0xc0 );

   /* Free bitmap */
   free_bitmap( scalbm );

   /* King or Queen */
   if (total_winner || (p_ptr->lev > PY_MAX_LEVEL))
   {
      p = "Magnificent";
   }

   /* Normal */
   else
   {
      p =  player_title[p_ptr->pclass][(p_ptr->lev-1)/5];
   }

   tomb_str( 3, " R.I.P." );

   tomb_str( 5, player_name );

   tomb_str( 6, "the" );

   tomb_str( 7, (char *)p );

   tomb_str( 9, (char *)cp_ptr->title );

   sprintf( tmp, "Level: %d", (int)p_ptr->lev );
   tomb_str( 10, tmp );

   sprintf( tmp, "Exp: %ld", (long)p_ptr->exp );
   tomb_str( 11, tmp );

   sprintf( tmp, "AU: %ld", (long)p_ptr->au );
   tomb_str( 12, tmp );

   sprintf( tmp, "Killed on Level %d", dun_level );
   tomb_str( 13, tmp );

   sprintf( tmp, "by %s", died_from );
   tomb_str( 14, tmp );

   sprintf( tmp, "%-.24s", ctime(&ct));
   tomb_str( 16, tmp );

   return( TRUE );
}

///}
///{ "tomb_str()"

void tomb_str( int y, char *str )
{
   term_data *td = &screen;
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

///}
///{ "handle_rawkey()"

void handle_rawkey( UWORD code, UWORD qual, APTR addr )
{
   char buf[ 80 ];
   int i;
   int len;
   UWORD q;

   /* Use a blank mouse-pointer on this window */
   if ( blankmouse && pointer_visible )
   {
      SetPointer( screen.win, blankpointer, 2, 16, 0, 0 );
      pointer_visible = FALSE;
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
   }

   /* Convert raw keycode to ANSI sequence */
   ie.ie_Code = code;
   ie.ie_Qualifier = qual;
   ie.ie_EventAddress = (APTR *) *((ULONG *) addr );
   len = MapRawKey( &ie, buf, 80, NULL );

   /* Send ANSI sequence to meta-terminal */
   for ( i = 0; i < len; i++ )
   {
      if( !iconified )
      {
         Term_keypress( (unsigned char) buf[ i ]);
      }
   }
}

///}
///{ "handle_menupick()"

void handle_menupick( int mnum )
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

      /* Is this a help item? */
      if ( ud >= MNU_HELP )
      {
         /* Send keypresses */
         Term_keypress( '\\' );
         Term_keypress( '?' );
         Term_keypress( ud - MNU_HELP );
      }

      /* Is this an option item? */
      else if ( ud >= MNU_OPTION )
      {
         /* Option index */
         i = ud - MNU_OPTION;

         /* Set option according to checkmark status */
         *options[ i ].o_var = item->Flags & CHECKED ? TRUE : FALSE;

         /* Fake a dummy keypress to cause update */
         Term_keypress( ' ' );
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

      /* Scaled down Map of the dungeon */
      else if ( ud == MNU_SCALEDMAP )
      {
         /* Draw the map */
         amiga_map();
      }

      /* Find next menunumber */
      mnum = item->NextSelect;
   }
}

///}
///{ "cursor_on()"

static void cursor_on( term_data *td )
{
   int x0, y0, x1, y1;

   if ( !td->cursor_lit && !iconified )
   {
      td->cursor_frame = 0;

      /* Hack - Don't draw cursor at (0,0) */
      if ( td->cursor_xpos == 0 && td->cursor_ypos == 0 ) return;

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
         SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
         SetBPen( td->wrp, PEN( CURSOR_PEN ));
         Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
         Text( td->wrp, &CUR_C, 1 );
      }

      td->cursor_lit = TRUE;
   }
}

///}
///{ "cursor_off()"

static void cursor_off( term_data *td )
{
   if ( td->cursor_lit && !iconified )
   {
      /* Restore graphics under cursor */
      if ( CUR_A & 0xf0 && use_graphics )
      {
         put_gfx( td->wrp, td->cursor_xpos, td->cursor_ypos, CUR_C, CUR_A );
      }

      /* Restore char/attr under cursor */
      else
      {
         SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
         SetBPen( td->wrp, PEN( 0 ));
         Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
         Text( td->wrp, &CUR_C, 1 );
      }
      td->cursor_lit = FALSE;
   }
}

///}
///{ "cursor_anim()"

static void cursor_anim( void )
{
   term_data *td = term_curs;
   int x0, y0, x1, y1, i = px, j = py;
   byte tc,ta;

   if ( !term_curs ) return;

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
      {
         ta = (( p_ptr->pclass * 10 + p_ptr->prace) >> 5 ) + 12;
         tc = (( p_ptr->pclass * 10 + p_ptr->prace) & 0x1f );
         put_gfx_map( td, i, j, tc, ta );
      }
   }

   else if ( td->cursor_visible && !iconified )
   {
      /* Hack - Don't draw cursor at (0,0) */
      if ( td->cursor_xpos == 0 && td->cursor_ypos == 0 ) return;

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
         SetAPen( td->wrp, PEN( CUR_A & 0x0f ));
         SetBPen( td->wrp, ( td->cursor_frame < 4 ) ? PEN( CURSOR_PEN ) : PEN( 0 ));
         Move( td->wrp, td->fw * td->cursor_xpos, td->fh * td->cursor_ypos + td->fb );
         Text( td->wrp, &CUR_C, 1 );
      }
   }
}

///}
///{ "load_gfx()"

int load_gfx( void )
{
   term_data *ts = &screen;
   BPTR file;
   char *p;
   int plane, row, error = FALSE;

   /* Allocate bitmaps */
   if (( ts->gfxbm = alloc_bitmap( GFXW, GFXH, 4, BMF_CLEAR, ts->rp->BitMap )) == NULL ) return( FALSE );
   if (( ts->mskbm = alloc_bitmap( GFXW, GFXH, 1, BMF_CLEAR, ts->rp->BitMap )) == NULL ) return( FALSE );

   /* Open file */
   if (( file = Open( MGFX, MODE_OLDFILE )) == NULL )
   {
      MSG( 0, 0, "Unable to open graphics file" ); Delay( 100 );
      return ( FALSE );
   }

   /* Read file into bitmap */
   for ( plane = 0; plane < 4 && !error; plane++ )
   {
      p = ts->gfxbm->Planes[ plane ];
      for ( row = 0; row < GFXH && !error; row++ )
      {
         error = ( Read( file, p, GFXB ) != GFXB );
         p += ts->gfxbm->BytesPerRow;
      }
   }

   /* Read mask data into bitmap */
   p = ts->mskbm->Planes[ 0 ];
   for ( row = 0; row < GFXH && !error; row++ )
   {
      error = ( Read( file, p, GFXB ) != GFXB );
      p += ts->mskbm->BytesPerRow;
   }

   /* Close file */
   Close( file );

   /* Did we get any errors while reading? */
   if ( error )
   {
      MSG( 0, 0, "Error while reading graphics file" ); Delay( 100 );
      return ( FALSE );
   }

   /* Success */
   return ( TRUE );
}
///}
///{ "conv_gfx()"

int conv_gfx( void )
{
   term_data *ts = &screen;
   struct BitMap *tmpbm;
   struct BitMap *sbm = ts->rp->BitMap;
   long stdpens[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
   long mskpens[] = { 0, -1 };
   int depth;
   BYTE *src, *dst;
   int plane, row, byt;

   /* Get depth of display */
   depth = depth_of_bitmap( sbm );

   /* We don't want 16 or 24 bit displays (yet) */
   if ( depth > 8 )
   {
      MSG( 0, 1, "Sorry, max. 8 bit display supported." ); Delay( 100 );
      return ( FALSE );
   }

   /* Allocate new bitmap with screen's depth */
   if (( tmpbm = alloc_bitmap( GFXW, GFXH, depth, BMF_CLEAR, sbm )) == NULL )
   {
      MSG( 0, 1, "Unable to allocate temporary bitmap." ); Delay( 100 );
      return ( FALSE );
   }

   /* Simple remapping from 4 to 5 planes? */
   if ( depth == 5 && !use_pub )
   {
      for ( plane = 0; plane < 4; plane++ )
      {
         src = ts->gfxbm->Planes[ plane ];
         dst = tmpbm->Planes[ plane ];
         for ( row = 0; row < GFXH; row++ )
         {
            for ( byt = 0; byt < GFXB; byt++ )
            {
               dst[ byt ] = src[ byt ];
            }
            src += ts->gfxbm->BytesPerRow;
            dst += tmpbm->BytesPerRow;
         }
      }
   }

   /* Complex remapping */
   else
   {
      /* Remap old bitmat into new bitmap */
      remap_bitmap( ts->gfxbm, tmpbm, use_pub ? pubpens : stdpens, GFXW, GFXH );
   }

   /* Free old bitmap */
   free_bitmap( ts->gfxbm );
   ts->gfxbm = tmpbm;

   /* Allocate new bitmap with screen's depth */
   if (( tmpbm = alloc_bitmap( GFXW, GFXH, depth, BMF_CLEAR, sbm )) == NULL )
   {
      MSG( 0, 1, "Unable to allocate temporary bitmap." ); Delay( 100 );
      return ( FALSE );
   }

   /* Simple remapping from 4 to 5 planes? */
   if ( depth == 5 && !use_pub )
   {
      for ( plane = 0; plane < 4; plane++ )
      {
         src = ts->mskbm->Planes[ 0 ];
         dst = tmpbm->Planes[ plane ];
         for ( row = 0; row < GFXH; row++ )
         {
            for ( byt = 0; byt < GFXB; byt++ )
            {
               dst[ byt ] = src[ byt ];
            }
            src += ts->mskbm->BytesPerRow;
            dst += tmpbm->BytesPerRow;
         }
      }
   }

   /* Complex remapping */
   else
   {
      /* Remap old bitmap into new bitmap */
      remap_bitmap( ts->mskbm, tmpbm, mskpens, GFXW, GFXH );
   }

   /* Free old bitmap */
   free_bitmap( ts->mskbm );
   ts->mskbm = tmpbm;

   /* Done */
   return ( TRUE );
}

///}
///{ "size_gfx()"

int size_gfx( term_data *td )
{
   term_data *ts = &screen;
   int depth;
   struct BitMap *sbm = td->rp->BitMap;
   struct BitMap *tmpbm;

   /* Calculate tile bitmap dimensions */
   td->gfx_w = 32 * td->fw;
   td->gfx_h = 32 * td->fh;

   /* Calculate map bitmap dimensions */
   td->mpt_w = td->ww / MAX_WID;
   td->mpt_h = td->wh / MAX_HGT;
   td->map_w = td->mpt_w * 32;
   td->map_h = td->mpt_h * 32;

   /* Scale tile graphics into map size */
   depth = depth_of_bitmap( ts->gfxbm );
   if (( td->mapbm = alloc_bitmap( td->map_w, td->map_h, depth, BMF_CLEAR, sbm )) == NULL ) return( FALSE );
   scale_bitmap( ts->gfxbm, GFXW, GFXH, td->mapbm, td->map_w, td->map_h );

   /* Scale tile graphics */
   depth = depth_of_bitmap( ts->gfxbm );
   if (( tmpbm = alloc_bitmap( td->gfx_w, td->gfx_h, depth, BMF_CLEAR, sbm )) == NULL ) return( FALSE );
   scale_bitmap( ts->gfxbm, GFXW, GFXH, tmpbm, td->gfx_w, td->gfx_h );
   if ( td->gfxbm ) free_bitmap( td->gfxbm );
   td->gfxbm = tmpbm;

   /* Scale tile mask */
   depth = depth_of_bitmap( ts->mskbm );
   if (( tmpbm = alloc_bitmap( td->gfx_w, td->gfx_h, depth, BMF_CLEAR, sbm )) == NULL ) return( FALSE );
   scale_bitmap( ts->mskbm, GFXW, GFXH, tmpbm, td->gfx_w, td->gfx_h );
   if ( td->mskbm ) free_bitmap( td->mskbm );
   td->mskbm = tmpbm;

   /* Success */
   return( TRUE );
}

///}
///{ "put_gfx()"

static void put_gfx( struct RastPort *rp, int x, int y, int chr, int col )
{
   term_data *td = (term_data*)(Term->data);
   int fw = td->fw;
   int fh = td->fh;
   int x0 = x * fw;
   int y0 = y * fh;
   int x1 = x0 + fw - 1;
   int y1 = y0 + fh - 1;
   int a  = col & 0x1f;
   int c  = chr & 0x1f;

   /* Just a black tile */
   if ( a == 0 && c == 0 )
   {
      SetAPen( rp, PEN( 0 ));
      RectFill( rp, x0, y0, x1, y1 );
      return;
   }

   /* Player - Remap for race and class */
   if ( a == 12 && c == 0 )
   {
      a = (( p_ptr->pclass * 10 + p_ptr->prace) >> 5 ) + 12;
      c = (( p_ptr->pclass * 10 + p_ptr->prace) & 0x1f );
   }

   /* Draw tile through mask */
   if ( col & 0x40 )
   {
      SetAPen( rp, PEN(0) );
      RectFill( rp, x0, y0, x1, y1 );
      BltMaskBitMapRastPort( td->gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, (ABC|ANBC|ABNC), td->mskbm->Planes[ 0 ] );
   }

   /* Draw full tile */
   else
   {
      BltBitMapRastPort( td->gfxbm, c * fw, a * fh, rp, x0, y0, fw, fh, 0xc0 );
   }
}

///}
///{ "amiga_fail()"

static int amiga_fail( char *msg )
{
   int i;

   /* Print error message */
   if ( msg )
   {
       fprintf( stderr, "%s\n", msg );
   }

   /* Free sound memory */
   free_sound();

   /* Unlock public screen */
   if ( publock )
   {
      UnlockPubScreen( NULL, pubscr );
      publock = FALSE;
   }

   /* Remove menu from window */
   if ( menu && screen.win )
   {
      ClearMenuStrip( screen.win );
   }

   /* Free menus */
   if ( menu )
   {
      FreeMenus( menu );
      menu = NULL;
   }

   /* Free term resources */
   free_term( &mirror );
   free_term( &recall );
   free_term( &choice );
   free_term( &screen );

   /* Free obtained pens */
   if ( pubscr )
   {
      for ( i = 0; i < 32; i++)
      {
         if ( pubpens[ i ] != -1 )
         {
            ReleasePen( pubscr->ViewPort.ColorMap, pubpens[ i ]);
         }
      }
   }

   /* Free visual info */
   if ( visinfo )
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

   /* Close keymap.library */
   if ( KeymapBase )
   {
      CloseLibrary( KeymapBase );
      KeymapBase = NULL;
   }

   /* Close keymap.library */
   if ( DiskfontBase )
   {
      CloseLibrary( DiskfontBase );
      DiskfontBase = NULL;
   }

   // Return failure
   return(-1);
}

///}
///{ "amiga_map()"

static void amiga_map( void )
{
   term_data *td = &screen; // (term_data*)(Term->data);
   int i,j;
   byte ta,tc;

   /* Only in graphics mode */
   if ( !use_graphics ) return;

   /* Turn off cursor */
   if ( td->cursor_visible ) cursor_off( td );

   /* Save screen */
   Term_save();

   /* Clear screen */
   Term_clear();
   Term_fresh();

   /* Calculate offset values */
   td->map_x = (( td->fw * 80 ) - ( td->mpt_w * cur_wid )) / 2;
   td->map_y = (( td->fh * 24 ) - ( td->mpt_h * cur_hgt )) / 2;

   /* Draw all "interesting" features */
   for ( i = 0; i < cur_wid; i++ )
   {
      for ( j = 0; j < cur_hgt; j++ )
      {
         /* Get frame tile */
         if ( i==0 || i == cur_wid - 1 || j == 0 || j == cur_hgt - 1 )
         {
            ta = f_info[ 63 ].z_attr;
            tc = f_info[ 63 ].z_char;
         }

         /* Get tile from cave table */
         else
         {
            map_info( j, i, &ta, (char *) &tc );
         }

         /* Ignore non-graphics */
         if ( ta & 0x80 )
         {
            ta &= 0x1f;
            tc &= 0x1f;

            /* Player XXX XXX XXX */
            if ( ta == 12 && tc ==0 )
            {
               ta = (( p_ptr->pclass * 10 + p_ptr->prace ) >> 5 ) + 12;
               tc = (( p_ptr->pclass * 10 + p_ptr->prace ) & 0x1f );
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
   if ( td->cursor_visible ) cursor_on( td );
}

///}
///{ "load_palette()"

void load_palette( void )
{
   int i;
   int n;

   if ( amiscr == NULL ) return;

   n = deep ? 32 : 16;

   if ( v39 )
   {
      palette32[ 0 ] = n << 16;
      palette32[ n * 3 + 1 ] = 0;
      for ( i = 0; i < n; i++ )
      {
         palette32[ i * 3 + 1 ] = color_table[ use_graphics ? i : i + 16 ][ 1 ] << 24;
         palette32[ i * 3 + 2 ] = color_table[ use_graphics ? i : i + 16 ][ 2 ] << 24;
         palette32[ i * 3 + 3 ] = color_table[ use_graphics ? i : i + 16 ][ 3 ] << 24;
      }
      LoadRGB32( &amiscr->ViewPort, palette32 );
   }
   else
   {
      for ( i = 0; i < n; i++ )
      {
         palette4[ i ] =  ( color_table[ use_graphics ? i : i + 16 ][ 1 ] >> 4 ) << 8;
         palette4[ i ] |= ( color_table[ use_graphics ? i : i + 16 ][ 2 ] >> 4 ) << 4;
         palette4[ i ] |= ( color_table[ use_graphics ? i : i + 16 ][ 3 ] >> 4 );
      }
      LoadRGB4( &amiscr->ViewPort, palette4, n );
   }
}

///}
///{ "create_menus()"

int create_menus( void )
{
   option_type *opt;
   struct NewMenu *item = newmenu;
   int nmsize = sizeof ( struct NewMenu );
   int page = -1;
   int pg = 0;
   int i,o;

   /* Copy all pre-items into array */
   for ( i = 0; pre_item[ i ].nm_Type != 255; i++ )
   {
      memcpy( item++, &pre_item[ i ], nmsize );
   }

   /* Find next option */
   for ( opt = options, o = 0; opt->o_desc; opt++, o++ )
   {
      /* Cheating options are skipped */
      if ( opt->o_page > 6 ) continue;

      /* New page of options? */
      if ( page != opt->o_page )
      {
         page = opt->o_page;

         /* Copy option header */
         memcpy( item++, &opt_item[ pg++ ], nmsize );
      }

      /* Insert fields for this option */
      item->nm_Type = NM_SUB;
      item->nm_CommKey = 0;
      item->nm_MutualExclude = 0;

      /* Use option description as menu text */
      item->nm_Label = (STRPTR) opt->o_desc;

      /* Insert option index into userdata field */
      item->nm_UserData = (void *)( MNU_OPTION + o );

      /* Use checkmark on this item */
      item->nm_Flags = CHECKIT | MENUTOGGLE;

      /* Done with this item */
      item++;
   }

   /* Copy all post-items into array */
   for ( i = 0; post_item[ i ].nm_Type != 255; i++ )
   {
      memcpy( item++, &post_item[ i ], nmsize );
   }

   /* Actually create the menu structures */
   menu = CreateMenus( newmenu, NULL );

   if ( menu )
   {
      /* Layout menus */
      if ( LayoutMenus( menu, visinfo, v39 ? GTMN_NewLookMenus : TAG_IGNORE, TRUE, TAG_END ))
      {
         /* Attach menu to window */
         SetMenuStrip( screen.win , menu );
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

///}
///{ "update_menus()"

void update_menus( void )
{
   struct MenuItem *father, *item;
   int i;

   /* Require a window and a menu */
   if ( !screen.win || !menu ) return;

   /* Detach the menu from the window */
//   ClearMenuStrip( screen.win );

   /* Initial menuitem and subitem for options */
   father = menu->FirstItem;
   item   = father->SubItem;

   /* Find next option */
   for ( i = 0; item && options[ i ].o_desc; i++ )
   {
      /* Did we find it? */
      if ( item )
      {
         /* Option is set, add a checkmark */
         if ( *(options[ i ].o_var ))
         {
            item->Flags |= CHECKED;
         }

         /* Option is not set, remove checkmark */
         else
         {
            item->Flags &= ~CHECKED;
         }
      }

      /* Menuitem not found */
      else
      {
         fprintf( stderr, "ERROR: menuitem #%d not found.\n", i );
         return;
      }

      /* Find next menuitem */
      if ((item = item->NextItem ) == NULL )
      {
         /* New set */
         father = father->NextItem;
         if ( father )
         {
            item = father->SubItem;
         }
      }
   }

   /* Enable/Disable the amiga map according to use_graphics */
   if ( item = ItemAddress( menu, FULLMENUNUM( 6, 7, 0 )))
   {
      item->Flags = use_graphics ? item->Flags | ITEMENABLED : item->Flags & ~ITEMENABLED;
   }

   /* Attach menu to window again */
//   ResetMenuStrip( screen.win, menu );
}

///}
///{ "init_sound()"

int init_sound( void )
{
   int i;
   char tmp[256];
   char buf[256];
   struct AmiSound *snd;

   /* Load samples */
   for ( i = 0; i < NSAMPLES; i++ )
   {
      /* Pointer to sound data */
      snd = &sound_data[ i ];

      /* Should this sample be loaded into memory? */
      if ( snd->Memory )
      {
         /* Construct filename */
         path_build(buf, 255, ANGBAND_DIR_XTRA, "sound");

         /* Construct filename */
         path_build(tmp, 255, buf, snd->Name );

         /* Load the sample into memory */
         snd->Address = (struct SoundInfo *) PrepareSound( tmp );
      }
   }

   /* Success */
   has_sound = TRUE;

   return ( TRUE );
}

///}
///{ "free_sound()"

void free_sound( void )
{
   int i;

   /* Stop all channels */
   StopSound( LEFT0 );
   StopSound( LEFT1 );
   StopSound( RIGHT0 );
   StopSound( RIGHT1 );

   /* Remove all sounds from memory */
   for ( i = 0; i < NSAMPLES; i++ )
   {
      if ( sound_data[ i ].Address )
      {
         /* Remove the sound from memory */
         RemoveSound( sound_data[ i ].Address );

         /* Clear address field */
         sound_data[ i ].Address = NULL;
      }
   }

   /* Done */
   has_sound = FALSE;
}

///}
///{ "play_sound()"

static void play_sound( int v )
{
   struct AmiSound *snd;
   struct AmiSound *old_snd;
   int rate;
   int channel;
   int old;
   char buf[256];
   char tmp[256];

   if ( has_sound )
   {
      /* Pointer to sound data */
      snd = &sound_data[ v ];

      /* Channel number */
      channel = snd->Channel;

      /* Last sample played on channel */
      old = channel_last[ channel ];

      /* Sample Rate */
      rate = snd->Rate;

      /* Random rate on some sounds */
      if ( v == SOUND_HIT || v == SOUND_MISS )
      {
         rate = rate - 50 + rand_int( 150 );
      }

      /* Pointer to old sound data */
      old_snd = old >= 0 ? &sound_data[ old ] : NULL;

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

      /* Load new sample into memory if required */
      if ( !snd->Memory && snd->Address == NULL )
      {
         /* Construct filename */
         path_build(buf, 255, ANGBAND_DIR_XTRA, "sound");

         /* Construct filename */
         path_build(tmp, 255, buf, snd->Name );

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
   }
}

///}
///{ put_gfx_map()

static void put_gfx_map( term_data *td, int x, int y, int c, int a )
{
   BltBitMapRastPort(
      td->mapbm,
      c * td->mpt_w,
      a * td->mpt_h,
      td->wrp,
      td->map_x + x * td->mpt_w,
      td->map_y + y * td->mpt_h,
      td->mpt_w,
      td->mpt_h,
      0xc0
   );
}

///}
///{ "alloc_bitmap()"

struct BitMap *alloc_bitmap( int width, int height, int depth, ULONG flags, struct BitMap *friend )
{
   int p;
   struct BitMap *bitmap;
   unsigned char *bp;

   /* Kickstart 39+ */
   if( v39 ) return ( AllocBitMap( width, height, depth, flags, friend ));

   /* Kickstart 38- */
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
            if ( !bp ) break;
            bitmap->Planes[ p ] = bp;
         }

         /* Out of memory */
         if ( p != depth )
         {
            /* Free bitplanes */
            while ( --p >= 0 )
            {
               FreeRaster( bitmap->Planes[ p ], width, height );
            }
            /* Free bitmap structure */
            FreeMem( bitmap, sizeof( struct BitMap ));
            bitmap = NULL;
         }
      }
      return ( bitmap );
   }
}

///}
///{ "free_bitmap()"

void free_bitmap( struct BitMap *bitmap )
{
   int p;

   /* Check for NULL */
   if ( !bitmap ) return;

   /* Kickstart 39+ */
   if ( v39 ) FreeBitMap( bitmap );

   /* Kickstart 38- */
   else
   {
      /* Free bitplanes */
      for ( p = 0; p < bitmap->Depth; p++ )
      {
         FreeRaster( bitmap->Planes[ p ], bitmap->BytesPerRow * 8, bitmap->Rows );
      }
      /* Free bitmap structure */
      FreeMem( bitmap, sizeof( struct BitMap ));
   }
}

///}
///{ scale_bitmap()

void scale_bitmap( struct BitMap *srcbm, int srcw, int srch, struct BitMap *dstbm, int dstw, int dsth )
{
   struct BitScaleArgs bsa;

   /* Scale bitmap */
   bsa.bsa_SrcBitMap   = srcbm;
   bsa.bsa_DestBitMap  = dstbm;
   bsa.bsa_SrcX        = 0;
   bsa.bsa_SrcY        = 0;
   bsa.bsa_SrcWidth    = srcw;
   bsa.bsa_SrcHeight   = srch;
   bsa.bsa_DestX       = 0;
   bsa.bsa_DestY       = 0;
   bsa.bsa_XSrcFactor  = srcw;
   bsa.bsa_YSrcFactor  = srch;
   bsa.bsa_XDestFactor = dstw;
   bsa.bsa_YDestFactor = dsth;
   bsa.bsa_Flags       = 0;
   BitMapScale( &bsa );
}

///}
///{ "remap_bitmap()"

void remap_bitmap( struct BitMap *srcbm, struct BitMap *dstbm, long *pens, int width, int height )
{
   int x,y,p,c,ox,lpr,sd,dd;
   int bm[] = { 1, 2, 4, 8, 16, 32, 64, 128 };
   UBYTE *sp[ 4 ];
   UBYTE *dp[ 8 ];
   ULONG ls[ 4 ];
   ULONG ld[ 8 ];
   ULONG mask;

   /* Source bitplanes */
   sd = depth_of_bitmap( srcbm );
   for ( p = 0; p < sd; p++ )
   {
      sp[ p ] = srcbm->Planes[ p ];
   }

   /* Destination bitplanes */
   dd = depth_of_bitmap( dstbm );
   for ( p = 0; p < dd; p++ )
   {
      dp[ p ] = dstbm->Planes[ p ];
   }

   /* Number of longwords per row */
   lpr = width / 32;

   /* Convert graphics */
   for( y = 0; y < height; y++ )
   {
      ox = 0;
      for ( x = 0 ; x < lpr; x++ )
      {
         /* Read source longwords */
         for ( p = 0; p < sd; p++ ) ls[ p ] = *(ULONG *)( sp[ p ] + ox);

         /* Clear destination longwords */
         for ( p = 0; p < dd; ld[ p++ ] = 0 );

         /* Remap */
         for ( mask = 0x80000000; mask != 0; mask >>= 1)
         {
            /* Find color index */
            for ( p = c = 0; p < sd; p++ ) if ( ls[ p ] & mask ) c |= bm[ p ];

            /* Remap */
            c = pens[ c ];

            /* Update destination longwords */
            for ( p = 0; p < dd; p++ ) if ( c & bm[ p ] ) ld[ p ] |= mask;
         }

         /* Write destination longwords */
         for ( p = 0; p < dd; p++ ) *(ULONG *)( dp[ p ] + ox ) = ld[ p ];

         /* Update offset */
         ox += 4;
      }

      /* Update pointers to get to next line */
      for ( p = 0; p < sd; sp[ p++ ] += srcbm->BytesPerRow );
      for ( p = 0; p < dd; dp[ p++ ] += dstbm->BytesPerRow );
   }
}

///}
///{ "depth_of_bitmap()"

int depth_of_bitmap( struct BitMap *bm )
{
   int depth;

   if ( v39 )
   {
      depth = GetBitMapAttr( bm, BMA_DEPTH );
   }
   else
   {
      depth = bm->Depth;
   }

   return ( depth );
}

///}

