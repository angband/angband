/*

   File:     main-ami.c

   Version:  2.7.9v4 (15.Feb.96)

   Purpose:  Amiga module for Angband with graphics

   Author:   Lars Haugseth
   Email:    larshau@ifi.uio.no
   WWW:      http://www.ifi.uio.no/~larshau

*/

#ifdef USE_AMI

#define VERSION      "Angband 2.7.9v4"

///{ "includes"
#include "angband.h"
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <devices/inputevent.h>
#include <graphics/gfxbase.h>
#include <graphics/modeid.h>
#include <graphics/scale.h>
#include <graphics/text.h>
#include <proto/exec.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/keymap.h>
///}
///{ "macros"
#define PEN(p)       ((usewb||use_graphics)?penconv[p]:p)
#define GPEN(p)      (usewb?wbpens[16+p]:p)
#define FAIL(str)    return(amiga_fail(str))
///}
///{ "defines"
#define CURSOR_PEN   4

#define TILEW        8
#define TILEH        8

#define GFXW         256
#define GFXH         256
#define GFXB         32

#define MX           2
#define MY           2

#define MGFX         "Angband:xtra/gfx-ami.raw"
///}
///{ "globals"
// Are we active?  Not really needed.
static int active = FALSE;
// The main screen
static term term_screen_body;
// Amiga specific stuff
struct Library *DiskfontBase = NULL;
struct Library *KeymapBase = NULL;
static struct Screen *amiscr = NULL;
static struct Screen *wbscr = NULL;
static struct Window *amiwin = NULL;
static struct RastPort *rp = NULL;
static struct BitMap *gfxbm = NULL;
static struct BitMap *mapbm = NULL;
static char *gfxmask = NULL;
static char chrmem[80][24];
static char colmem[80][24];
static int cursor_xpos = 0;
static int cursor_ypos = 0;
static int cursor_visible = FALSE;
static int cursor_lit = FALSE;
static int cursor_frame = 0;
static int cursor_map = FALSE;
static struct TextFont *font = NULL;
static struct TextAttr attr;
static int ownfont = FALSE;
static int font_w;
static int font_h;
static int font_b;
static int scr_w;
static int scr_h;
static int gfx_w;
static int gfx_h;
static ULONG scr_m = 0;
static int map_w;
static int map_h;
static int map_x;
static int map_y;
static int mpt_w;
static int mpt_h;
static char tmpstr[256];
static int iconified=FALSE;
static struct InputEvent ie;
static int verbose = !TRUE;
static int v39 = FALSE;
static int usewb = FALSE;
static int wblock = FALSE;
static __chip UWORD blankpointer[] = {0,0,0,0,0,0,0,0};
static UWORD amipens[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0xffff};
static UWORD penconv[] = {0,1,2,4,11,15,9,6,3,1,13,4,11,15,8,5,0xffff};
static LONG wbpens[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static ULONG default_colors[] = {
   0x000000, 0xffffff, 0xc7c7c7, 0xff9200,
   0xff0000, 0x00cd00, 0x0000fe, 0xc86400,
   0x8a8a8a, 0xe0e0e0, 0xa500ff, 0xfffd00,
   0xff00bc, 0x00ff00, 0x00c8ff, 0xffcc80,
   0x000000, 0xf0e0d0, 0x808080, 0x505050,
   0xe0b000, 0xc0a070, 0x806040, 0x403020,
   0x00a0f0, 0x0000f0, 0x000070, 0xf00000,
   0x800000, 0x9000b0, 0x006010, 0x60f040
};
static ULONG palette32[32*3+2];
static UWORD palette4[32];
static struct RastPort tmpRastPort;
static struct BitMap tmpBitMap;
static __chip UBYTE tmpPlanes[8][64];
static UBYTE *chunkygfx = NULL;
static UBYTE *chunkytmp = NULL;
static int use_chunky = FALSE;
static int tmp_w;
static int pointer_visible = TRUE;
static char ver[] = "$VER: " VERSION " (" __DATE__ ")";
///}
///{ "prototypes"
extern void map_info(int y, int x, byte *ap, char *cp);
errr init_ami(void);
static void amiga_open(term *t);
static void amiga_nuke(term *t);
static errr amiga_curs(int x, int y);
static errr amiga_wipe(int x, int y, int n);
static errr amiga_clear(void);
static errr amiga_text(int x, int y, int n, byte a, cptr s);
static errr amiga_xtra(int n, int v);
static errr amiga_flush(int v);
static errr amiga_event(int v);
static errr amiga_react(int v);
errr amiga_fail(char *msg);
void cursor_on(void);
void cursor_off(void);
int load_gfx(void);
int conv_gfx(void);
void put_gfx(int x, int y, int chr, int col);
void cursor_anim(void);
void load_palette(void);
///}

///{ "init_ami()" - Initialize all Amiga specific stuff
errr init_ami(void)
{
   term *t = &term_screen_body;
   char *s;
   int x,y;
   int i;
   LONG pen;

   // We *must* have kickstart 37 or later
   if(IntuitionBase->LibNode.lib_Version < 37) {
      FAIL("Sorry, this program requires KickStart 2.04 or later.");
   }

   // Check if we have kickstart 39 or later
   v39 = (IntuitionBase->LibNode.lib_Version >=39);

   // Initialize keyboard stuff
   ie.ie_NextEvent = NULL;
   ie.ie_Class     = IECLASS_RAWKEY;
   ie.ie_SubClass  = 0;
   if((KeymapBase=OpenLibrary("keymap.library",36))==NULL)
      FAIL("Unable to open keymap.library v36+\n");

   // Check if user want to use graphics
   if(GetVar("ANGBAND_NOGFX",tmpstr,256,0)>=0) {
      use_graphics=FALSE;
   } else {
      use_graphics=TRUE;
   }

   // Initialize color palette
   for(i=0; i<32; i++) {
      // If undefined, use default palette
      if(color_table[i][0]==0 && color_table[i][1]==0 && color_table[i][2]==0 && color_table[i][3]==0) {
         color_table[i][0] = 1;
         color_table[i][1] = (default_colors[i] & 0xff0000) >> 16;
         color_table[i][2] = (default_colors[i] & 0x00ff00) >> 8;
         color_table[i][3] = (default_colors[i] & 0x0000ff);
      }
   }

   // Search for prefered screenmode
   if(GetVar("ANGBAND_MODEID",tmpstr,256,0)>0) {
      // Use workbench
      if(!stricmp(tmpstr,"workbench")) {
         if(!v39) {
            FAIL("Workbench can only be used with Kickstart 3.0 or later.");
         }
         scr_m=-1;
         usewb=TRUE;
         use_chunky=use_graphics;
         // Get a lock on the workbench screen
         if((wbscr=LockPubScreen("Workbench"))==NULL) {
            FAIL("Unable to get a lock on the Workbench screen.\n");
         }
         wblock=TRUE;
         // Find suitable pens to use on public screen
         for(i=0; i<32; i++) {
            pen=ObtainBestPen(wbscr->ViewPort.ColorMap,
                              color_table[i][1]<<24,
                              color_table[i][2]<<24,
                              color_table[i][3]<<24,
                              OBP_Precision, PRECISION_EXACT);
            if(pen==-1) {
               FAIL("Unable to obtain suitable pens to use on Workbench screen.\n");
            }
            wbpens[i]=pen;
         }
         for(i=0; i<16; i++) penconv[i]=(UWORD)wbpens[i];
      // Use specified screenmode
      } else {
         scr_m=strtol(tmpstr,&s,0);
         if(ModeNotAvailable(scr_m)) scr_m = 0;
      }
   }

   // Search for prefered font, and open it
   if(GetVar("ANGBAND_FONT",tmpstr,256,0)>0) {
      if(s=strchr(tmpstr,' ')) {
         *s++=0;
         attr.ta_Name  = tmpstr;
         attr.ta_YSize = atoi(s);
         attr.ta_Style = FS_NORMAL;
         attr.ta_Flags = FPF_DISKFONT;
         if(DiskfontBase=OpenLibrary("diskfont.library",0)) {
            if(font=OpenDiskFont(&attr)) {
               ownfont=TRUE;
            } else {
               printf("Unable to open your specified font.\n");
            }
            CloseLibrary(DiskfontBase);
         }
      }
   }

   // If font not available, use default font
   if(font==NULL) {
      font=GfxBase->DefaultFont;
      ownfont=FALSE;
   }

   font_w = font->tf_XSize;
   font_h = font->tf_YSize;
   font_b = font->tf_Baseline;

   // Calculate screen/window dimensions
   scr_w = 80*font_w; if(!usewb && scr_w<640) scr_w = 640;
   scr_h = 24*font_h; if(!usewb && scr_h<200) scr_h = 200;

   // Find a nice screenmode
   if(scr_m==0 && v39) {
      scr_m = BestModeID(BIDTAG_NominalWidth, scr_w, BIDTAG_NominalHeight, scr_h, BIDTAG_Depth, 4, TAG_END);
   }

   // Use default screenmode
   if(scr_m==0 || scr_m==INVALID_ID) {
      scr_m = (DEFAULT_MONITOR_ID | HIRES_KEY);
   }

   // Open intuition screen
   if(!usewb) {
      if((amiscr=OpenScreenTags(NULL,
             SA_Width, scr_w,
             SA_Height, scr_h,
             SA_Depth, 4,
             SA_DisplayID, scr_m,
             ownfont?SA_Font:SA_SysFont, ownfont?&attr:0,
             SA_Pens, amipens,
             SA_Type, CUSTOMSCREEN,
             SA_Title, "Angband Screen",
             SA_ShowTitle, FALSE,
             SA_Quiet, TRUE,
             SA_Behind, TRUE,
             SA_AutoScroll, TRUE,
             SA_Overscan, OSCAN_TEXT,
             TAG_END))==NULL)
      {
         FAIL("Unable to open screen.");
      }
   } else {
      // Check if it is large enough
      if(wbscr->Width < scr_w || wbscr->Height < scr_h)
         FAIL("Workbench screen is too small.\n");
   }
   // Open intuition backdrop window
   if((amiwin=OpenWindowTags(NULL,
          WA_Left, 0,
          WA_Top, 0,
          WA_InnerWidth, scr_w,
          WA_InnerHeight, scr_h,
          usewb ? WA_PubScreen : WA_CustomScreen, usewb?NULL:amiscr,
          WA_Backdrop, !usewb,
          WA_Borderless, !usewb,
          WA_GimmeZeroZero, usewb,
          WA_DragBar, usewb,
          WA_DepthGadget, usewb,
          usewb ? WA_ScreenTitle : TAG_IGNORE, VERSION,
          usewb ? WA_Title : TAG_IGNORE, VERSION,
          WA_Activate, TRUE,
          WA_RMBTrap, TRUE,
          WA_ReportMouse, TRUE,
          WA_IDCMP, IDCMP_RAWKEY | IDCMP_INTUITICKS | IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS,
          TAG_END))==NULL)
   {
      FAIL("Unable to open window.");
   }

   // Unlock public screen
   if(wblock) {
      UnlockPubScreen(NULL,wbscr);
      wblock=FALSE;
   }

   // Create palette for screen
   load_palette();

   // Initialize rastport
   rp=amiwin->RPort;
   SetAPen(rp,1);
   SetBPen(rp,0);
   SetDrMd(rp,JAM2);
   if(ownfont && font) SetFont(rp,font);

   // Initialize chunky graphics
   if(use_chunky) {
      tmp_w = ((font_w+15)>>4)<<4;
      memcpy(&tmpRastPort,rp,sizeof(struct RastPort));
      InitBitMap(&tmpBitMap,8,tmp_w,1);
      tmpRastPort.Layer=NULL;
      tmpRastPort.BitMap=&tmpBitMap;
      tmpRastPort.BitMap->Rows=1;
      tmpRastPort.BitMap->BytesPerRow=tmp_w/8;
      for(i=0; i<8; i++) tmpRastPort.BitMap->Planes[i]=tmpPlanes[i];
   }

   // Load graphics
   if(use_graphics) {
      if(!load_gfx()) {
         FAIL(NULL);
      }
   }

   // Convert graphics to chunky format
   if(use_chunky) {
      if(!conv_gfx()) {
         FAIL(NULL);
      }
   }

   // Fill window with color 0
   if(usewb) {
      SetAPen(rp,PEN(0));
      RectFill(rp,0,0,scr_w-1,scr_h-1);
   }
   // Clear display memory
   for(x=0; x<80; x++) {
      for(y=0; y<24; y++) {
         chrmem[x][y]=' ';
         colmem[x][y]=0;
      }
   }

   // Bring screen to front
   if(usewb) WBenchToFront();
   else ScreenToFront(amiscr);

   // Initialize the term
   term_init(t,80,24,64);
   // Hack -- shutdown hook
   t->init_hook = amiga_open;
   t->nuke_hook = amiga_nuke;
   // Stick in some hooks
   t->text_hook = amiga_text;
   t->wipe_hook = amiga_wipe;
   t->curs_hook = amiga_curs;
   t->xtra_hook = amiga_xtra;
   // Save the term
   term_screen = t;
   // Activate it
   Term_activate(term_screen);
   // Success
   return(0);
}
///}
///{ "amiga_nuke()" - Free all allocated resources
static void amiga_nuke(term *t)
{
   amiga_fail(NULL);
   // Flush the output
   fflush(stdout);
   // No longer active
   active=FALSE;
}
///}
///{ "amiga_open()" - Initialize terminal
static void amiga_open(term *t)
{
   // Assume active
   active=TRUE;
}
///}
///{ "amiga_curs()" - Move the cursor to a new location
static errr amiga_curs(int x, int y)
{
   // Literally move the cursor
   cursor_off();
   cursor_xpos = x;
   cursor_ypos = y;
   cursor_frame = 0;
   if(cursor_visible) cursor_on();
   // Success
   return(0);
}
///}
///{ "amiga_wipe()" - Erase a rectangular area
static errr amiga_wipe(int x, int y, int n)
{
   int xx;

   if((n>0) && !iconified) {
      // Erase rectangular area on screen
      if(usewb) {
         SetAPen(rp,PEN(0));
         RectFill(rp,x*font_w,y*font_h,(x+n)*font_w-1,(y+1)*font_h-1);
      }
      else EraseRect(rp,x*font_w,y*font_h,(x+n)*font_w-1,(y+1)*font_h-1);
      // Erase memory
      for(xx=x; xx<(x+n); xx++) {
         chrmem[xx][y]=' ';
         colmem[xx][y]=0;
      }
   }
   // Success
   return(0);
}
///}
///{ "amiga_clear()" - Clear whole window
static errr amiga_clear(void)
{
   int x,y;

   // Fill window with background color
   SetRast(rp,PEN(0));
   // Fill screen memory with blanks
   for(x=0; x<80; x++) {
      for(y=0; y<24; y++) {
         chrmem[x][y]=' ';
         colmem[x][y]=0;
      }
   }
   // Success
   return(0);
}
///}
///{ "amiga_text()" - Place some text on the screen using an attribute
static errr amiga_text(int x, int y, int n, byte a, cptr s)
{
   int i;

   if(x>=0 && y>=0 && n>0 && !iconified) {
      // Clear cursor
      cursor_off();
      // Update cursor position
      cursor_xpos = x;
      cursor_ypos = y;
      // Draw gfx one char at a time
      if(a&0xc0 && use_graphics) {
         for(i=0; i<n; i++) put_gfx(x+i,y,s[i],a);
      } else {
         // Draw the string on screen
         SetAPen(rp,PEN(a&0x0f));
         SetBPen(rp,PEN(0));
         Move(rp,x*font_w,y*font_h+font_b);
         Text(rp,(char *)s,n);
      }
      // Draw the string in memory
      for(i=0; i<n; i++) {
         chrmem[x+i][y]=s[i];
         colmem[x+i][y]=a;
      }
      // Redraw cursor
      if(cursor_visible) cursor_on();
   }
   // Success
   return(0);
}
///}
///{ "amiga_xtra()" - Handle a "special request"
static errr amiga_xtra(int n, int v)
{
   // Analyze the request
   switch(n) {
      // Make a noise
      case TERM_XTRA_CLEAR:
         return(amiga_clear());
      case TERM_XTRA_NOISE:
         if(amiscr) DisplayBeep(amiscr);
         else DisplayBeep(wbscr);
         return(0);
      // Change the cursor visibility
      case TERM_XTRA_SHAPE:
         if (v) {
             cursor_on();
             cursor_visible=TRUE;
         }
         else {
             cursor_off();
             cursor_visible=FALSE;
         }
         return(0);
      // Wait for event
      case TERM_XTRA_EVENT:
         return(amiga_event(v));
      // Flush input
      case TERM_XTRA_FLUSH:
         return(amiga_flush(v));
      // Flush input
      case TERM_XTRA_REACT:
         return(amiga_react(v));
      // Unknown request type
      default:
         return(1);
   }
   // Shouldn't be able to get here
   return(1);
}
///}
///{ "amiga_flush()" - Flush input buffer
static errr amiga_flush(int v)
{
   struct IntuiMessage *imsg;

   // Ignore all messages at the port
   while(imsg=(struct IntuiMessage *)GetMsg(amiwin->UserPort))
      ReplyMsg((struct Message *)imsg);

   return(1);
}
///}
///{ "amiga_event()" - Wait for an event, and handle it.
static errr amiga_event(int v)
{
   struct IntuiMessage *imsg;
   ULONG iclass;
   UWORD icode;
   UWORD iqual;
   APTR iaddr;
   char buf[80];
   int i,len;

   // Check for messages to the window
   if((imsg=(struct IntuiMessage *)GetMsg(amiwin->UserPort))==NULL) {
      // If we don't want blocking, return
      if(!v) return(0);
      // No messages, so wait for one
      Wait(1<<amiwin->UserPort->mp_SigBit);
      imsg=(struct IntuiMessage *)GetMsg(amiwin->UserPort);
   }
   if(imsg) {
      // Get message attributes
      iclass=imsg->Class;
      icode=imsg->Code;
      iqual=imsg->Qualifier;
      iaddr=imsg->IAddress;
      // Reply the message
      ReplyMsg((struct Message *)imsg);
      // Do we have a keypress?
      if(iclass==IDCMP_RAWKEY) {
         // Use a blank mouse-pointer on this window
         if(!usewb && pointer_visible) {
            SetPointer(amiwin,blankpointer,2,16,0,0);
            pointer_visible=FALSE;
         }
         // Convert raw keycode to ANSI sequence
         ie.ie_Code = icode;
         ie.ie_Qualifier = iqual;
         ie.ie_EventAddress = (APTR *) *((ULONG *)iaddr);
         len=MapRawKey(&ie,buf,80,NULL);
         // Send ANSI sequence to meta-terminal
         for(i=0; i<len; i++) {
            if(!iconified) Term_keypress((unsigned char)buf[i]);
         }
         return(0);
      }
      // Mouse event - Make pointer visible
      if(iclass==IDCMP_MOUSEMOVE || iclass==IDCMP_MOUSEBUTTONS) {
         if(!usewb && !pointer_visible) {
            ClearPointer(amiwin);
            pointer_visible=TRUE;
         }
      }
      // Time for some cursor anim?
      if(iclass==IDCMP_INTUITICKS) {
         cursor_anim();
         return(1);
      }
      return(1);
   }
   // No keypresses
   return(1);
}
///}
///{ "amiga_react()"
static errr amiga_react(int v)
{
   load_palette();
   return(0);
}
///}
///{ "cursor_on()"
void cursor_on(void)
{
   int col,x0,y0,x1,y1;

   if(!cursor_lit && !iconified) {
      cursor_frame=0;
      col=colmem[cursor_xpos][cursor_ypos];
      if(col&0x80 && use_graphics) {
         // Draw an outlined cursor
         x0=cursor_xpos*font_w;
         y0=cursor_ypos*font_h;
         x1=x0+font_w-1;
         y1=y0+font_h-1;
         SetAPen(rp,PEN(CURSOR_PEN));
         Move(rp,x0,y0); Draw(rp,x1,y0); Draw(rp,x1,y1); Draw(rp,x0,y1); Draw(rp,x0,y0);
      } else {
         // Draw a filled cursor
         SetAPen(rp,PEN(col&0x0f));
         SetBPen(rp,PEN(CURSOR_PEN));
         Move(rp,font_w*cursor_xpos,font_h*cursor_ypos+font_b);
         Text(rp,&chrmem[cursor_xpos][cursor_ypos],1);
      }
      cursor_lit=TRUE;
   }
}
///}
///{ "cursor_off()"
void cursor_off(void)
{
   int col,chr;

   if(cursor_lit && !iconified) {
      col=colmem[cursor_xpos][cursor_ypos];
      chr=chrmem[cursor_xpos][cursor_ypos];
      if(col&0xf0 && use_graphics) {
         put_gfx(cursor_xpos,cursor_ypos,chr,col);
      } else {
         SetAPen(rp,PEN(col&0x0f));
         SetBPen(rp,PEN(0));
         Move(rp,font_w*cursor_xpos,font_h*cursor_ypos+font_b);
         Text(rp,&chrmem[cursor_xpos][cursor_ypos],1);
      }
      cursor_lit=FALSE;
   }
}
///}
///{ "load_gfx()"
int load_gfx(void)
{
   BPTR file;
   char *p;
   int plane,row;
   int error=FALSE;
   struct BitMap *tmpbm;
   struct BitScaleArgs bsa;

   // Calculate tile bitmap dimensions
   gfx_w = 32 * font_w;
   gfx_h = 32 * font_h;

   // Calculate map bitmap dimensions
   mpt_w = (80 * font_w) / MAX_WID;
   mpt_h = (24 * font_h) / MAX_HGT;
   if(mpt_w > 8) mpt_w = 8;
   if(mpt_h > 8) mpt_h = 8;
   map_w = mpt_w * 32;
   map_h = mpt_h * 32;

   // Allocate bitmap and bitplanes
   if(v39) {
      // Allocate temp bitmap with bitplanes
      if((tmpbm=AllocBitMap(GFXW,GFXH,5,BMF_INTERLEAVED,rp->BitMap))==NULL) {
         fprintf(stderr,"Unable to allocate temp bitmap.\n");
         return(FALSE);
      }
      // Allocate tile bitmap with bitplanes
      if((gfxbm=AllocBitMap(gfx_w,gfx_h,5,BMF_INTERLEAVED,rp->BitMap))==NULL) {
         fprintf(stderr,"Unable to allocate tile bitmap.\n");
         return(FALSE);
      }
      // Allocate map bitmap with bitplanes
      if((mapbm=AllocBitMap(map_w,map_h,5,BMF_INTERLEAVED,rp->BitMap))==NULL) {
         fprintf(stderr,"Unable to allocate map bitmap.\n");
         return(FALSE);
      }
   } else {
      // Allocate temp bitmap structure
      if((tmpbm=AllocMem(sizeof(struct BitMap), MEMF_PUBLIC))==NULL) {
         fprintf(stderr,"Unable to allocate temp bitmap.\n");
         return(FALSE);
      }
      // Initialize temp bitmap structure
      InitBitMap(tmpbm,5,GFXW,GFXH);
      for(plane=0; plane<5; tmpbm->Planes[plane++]=0);
      // Allocate bitplanes for the bitmap
      for(plane=0; plane<5; plane++) {
         if((p=AllocRaster(GFXW,GFXH))==NULL) {
            fprintf(stderr,"Unable to allocate temp bitplanes.\n");
            return(FALSE);
         }
         tmpbm->Planes[plane]=p;
      }
      // Allocate tile bitmap structure
      if((gfxbm=AllocMem(sizeof(struct BitMap), MEMF_PUBLIC))==NULL) {
         fprintf(stderr,"Unable to allocate tile bitmap.\n");
         return(FALSE);
      }
      // Initialize tile bitmap structure
      InitBitMap(gfxbm,5,gfx_w,gfx_h);
      for(plane=0; plane<5; gfxbm->Planes[plane++]=0);
      // Allocate bitplanes for the bitmap
      for(plane=0; plane<5; plane++) {
         if((p=AllocRaster(gfx_w,gfx_h))==NULL) {
            fprintf(stderr,"Unable to allocate tile bitplanes.\n");
            return(FALSE);
         }
         gfxbm->Planes[plane]=p;
      }
      // Allocate map bitmap structure
      if((mapbm=AllocMem(sizeof(struct BitMap), MEMF_PUBLIC))==NULL) {
         fprintf(stderr,"Unable to allocate tile bitmap.\n");
         return(FALSE);
      }
      // Initialize tile bitmap structure
      InitBitMap(mapbm,5,map_w,map_h);
      for(plane=0; plane<5; mapbm->Planes[plane++]=0);
      // Allocate bitplanes for the bitmap
      for(plane=0; plane<5; plane++) {
         if((p=AllocRaster(map_w,map_h))==NULL) {
            fprintf(stderr,"Unable to allocate tile bitplanes.\n");
            return(FALSE);
         }
         mapbm->Planes[plane]=p;
      }
   }

   // Open file
   if((file=Open(MGFX,MODE_OLDFILE))==NULL) {
      fprintf(stderr,"Unable to open graphics file.\n");
      return(FALSE);
   }

   // Read file into bitmap
   for(plane=0; plane<5 && !error; plane++) {
      p=tmpbm->Planes[plane];
      for(row=0; row<GFXH && !error; row++) {
         error=(Read(file,p,GFXB)!=GFXB);
         p+=tmpbm->BytesPerRow;
      }
   }

   // Close file
   Close(file);

   // Did we get any errors while reading?
   if(error) {
      fprintf(stderr,"Error while reading graphics file.\n");
      return(FALSE);
   }

   // Scale bitmap to fit font size
   bsa.bsa_SrcBitMap   = tmpbm;
   bsa.bsa_DestBitMap  = gfxbm;
   bsa.bsa_SrcX        = 0;
   bsa.bsa_SrcY        = 0;
   bsa.bsa_SrcWidth    = GFXW;
   bsa.bsa_SrcHeight   = GFXH;
   bsa.bsa_DestX       = 0;
   bsa.bsa_DestY       = 0;
   bsa.bsa_XSrcFactor  = GFXW;
   bsa.bsa_YSrcFactor  = GFXH;
   bsa.bsa_XDestFactor = gfx_w;
   bsa.bsa_YDestFactor = gfx_h;
   bsa.bsa_Flags       = 0;
   BitMapScale(&bsa);

   // Scale bitmap to map size
   bsa.bsa_SrcBitMap   = tmpbm;
   bsa.bsa_DestBitMap  = mapbm;
   bsa.bsa_SrcX        = 0;
   bsa.bsa_SrcY        = 0;
   bsa.bsa_SrcWidth    = GFXW;
   bsa.bsa_SrcHeight   = GFXH;
   bsa.bsa_DestX       = 0;
   bsa.bsa_DestY       = 0;
   bsa.bsa_XSrcFactor  = GFXW;
   bsa.bsa_YSrcFactor  = GFXH;
   bsa.bsa_XDestFactor = map_w;
   bsa.bsa_YDestFactor = map_h;
   bsa.bsa_Flags       = 0;
   BitMapScale(&bsa);

   // Free temp bitmap
   if(v39) {
      FreeBitMap(tmpbm);
   } else {
      for(plane=0; plane<5; plane++) if(tmpbm->Planes[plane]) FreeRaster(tmpbm->Planes[plane],GFXW,GFXH);
      FreeMem(tmpbm, sizeof(struct BitMap));
   }
   tmpbm=NULL;

   // Initialize pointer to mask plane
   gfxmask = gfxbm->Planes[4];

   return(TRUE);
}
///}
///{ "conv_gfx()"
int conv_gfx(void)
{
   int x,y,ox,sm,b,p;
   UBYTE *s1,*s2,*s3,*s4,*s5,*dp;

   s1 = gfxbm->Planes[0];
   s2 = gfxbm->Planes[1];
   s3 = gfxbm->Planes[2];
   s4 = gfxbm->Planes[3];
   s5 = gfxbm->Planes[4];

   // Allocate memory for chunky graphics
   if((chunkygfx=AllocMem(gfx_w*gfx_h, MEMF_PUBLIC))==NULL) {
      fprintf(stderr,"Couldn't allocate chunky gfx buffer.\n");
      return(FALSE);
   }
   if((chunkytmp=AllocMem(tmp_w*font_h, MEMF_PUBLIC))==NULL) {
      fprintf(stderr,"Couldn't allocate chunky temp buffer.\n");
      return(FALSE);
   }

   dp = chunkygfx;

   for(y=0; y<gfx_h; y++) {

      ox=0;
      sm=0x80;

      for(x=0; x<gfx_w; x++) {

         b = 0;
         if(*(s1+ox) & sm) b |= 1;
         if(*(s2+ox) & sm) b |= 2;
         if(*(s3+ox) & sm) b |= 4;
         if(*(s4+ox) & sm) b |= 8;

         if(*(s5+ox) & sm)
            *(dp++) = GPEN(b) | 0x80;
         else
            *(dp++) = GPEN(b);

         sm = sm>>1;
         if(sm==0) {
            sm = 0x80;
            ox++;
         }

      }

      s1 += gfxbm->BytesPerRow;
      s2 += gfxbm->BytesPerRow;
      s3 += gfxbm->BytesPerRow;
      s4 += gfxbm->BytesPerRow;
      s5 += gfxbm->BytesPerRow;

   }

   // Free planar graphics
   if(v39) {
      FreeBitMap(gfxbm);
   } else {
      for(p=0; p<5; p++) if(gfxbm->Planes[p]) FreeRaster(gfxbm->Planes[p],gfx_w,gfx_h);
      FreeMem(gfxbm, sizeof(struct BitMap));
   }
   gfxbm=NULL;

   return(TRUE);
}
///}
///{ "put_gfx()"
void put_gfx(int x, int y, int chr, int col)
{
   UBYTE tmp;

   int i,j;

   int x0 = x*font_w;
   int y0 = y*font_h;
   int x1 = x0+font_w-1;
   int y1 = y0+font_h-1;
   int a  = col&0x1f;
   int c  = chr&0x1f;

   // Just a black tile
   if(a==0 && c==0) {
      SetAPen(rp,PEN(0));
      RectFill(rp, x0, y0, x1, y1);
      return;
   }

   // Player - Remap for race and class
   if(a==12 && c==0) {
      a = ((p_ptr->pclass * 10 + p_ptr->prace) >> 5) + 12;
      c = ((p_ptr->pclass * 10 + p_ptr->prace) & 0x1f);
   }

   // Use chunky graphics
   if(use_chunky) {
      // Draw tile with black background (inventory, equipment, store)
      if(col & 0x40) {
         // Copy from chunky buffer to temp buffer
         for(j=0; j<font_h; j++) for(i=0; i<font_w; i++) {
            tmp = *(chunkygfx+c*font_w+i+(a*font_h+j)*gfx_w);
            *(chunkytmp+i+j*tmp_w) = (tmp & 0x80)? tmp & 0x7f : GPEN(0);
         }
      }
      // Draw tile with normal background (dungeon, town)
      else {
         // Copy from chunky buffer to temp buffer
         for(j=0; j<font_h; j++) for(i=0; i<font_w; i++) {
            *(chunkytmp+i+j*tmp_w) = *(chunkygfx+c*font_w+i+(a*font_h+j)*gfx_w) & 0x7f;
         }
      }
      WritePixelArray8(rp, x0, y0, x1, y1, chunkytmp, &tmpRastPort);
   }

   // Use planar graphics
   else {
      // Draw tile with black background (inventory, equipment, store)
      if(col & 0x40) {
         SetAPen(rp,PEN(0));
         RectFill(rp, x0, y0, x1, y1);
         BltMaskBitMapRastPort(gfxbm, c*font_w, a*font_h, rp, x0, y0, font_w, font_h, (ABC|ANBC|ABNC), gfxmask);
      }
      // Draw tile with normal background (dungeon, town)
      else {
         BltBitMapRastPort(gfxbm, c*font_w, a*font_h, rp, x0, y0, font_w, font_h, 0xc0);
      }
   }

}
///}
///{ "cursor_anim()"
void cursor_anim(void)
{
   int col,x0,y0,x1,y1,i=px,j=py;
   byte tc,ta;

   cursor_frame=(++cursor_frame)%8;

   // Small cursor on map
   if(cursor_map) {
      if(cursor_frame&2) {
         SetAPen(rp, PEN(CURSOR_PEN));
         RectFill(rp, map_x+i*mpt_w, map_y+j*mpt_h, map_x+(i+1)*mpt_w-1, map_y+(j+1)*mpt_h-1);
      } else {
         ta = ((p_ptr->pclass * 10 + p_ptr->prace) >> 5) + 12;
         tc = ((p_ptr->pclass * 10 + p_ptr->prace) & 0x1f);
         BltBitMapRastPort(mapbm, tc*mpt_w, ta*mpt_h, rp, map_x+i*mpt_w, map_y+j*mpt_h, mpt_w, mpt_h, 0xc0);
      }
   }
   else if(cursor_visible && !iconified) {
      col=colmem[cursor_xpos][cursor_ypos];
      if(col&0x80 && use_graphics) {
         // First draw the tile under cursor
         put_gfx(cursor_xpos,cursor_ypos,chrmem[cursor_xpos][cursor_ypos],col);
         // Draw an outlined cursor
         if(cursor_frame<4) {
            x0=cursor_xpos*font_w;
            y0=cursor_ypos*font_h;
            x1=x0+font_w-1;
            y1=y0+font_h-1;
            SetAPen(rp,PEN(CURSOR_PEN));
            Move(rp,x0,y0); Draw(rp,x1,y0); Draw(rp,x1,y1); Draw(rp,x0,y1); Draw(rp,x0,y0);
         }
      } else {
         // Draw a filled cursor
         SetAPen(rp,PEN(col&0x0f));
         SetBPen(rp,(cursor_frame<4)?PEN(CURSOR_PEN):PEN(0));
         Move(rp,font_w*cursor_xpos,font_h*cursor_ypos+font_b);
         Text(rp,&chrmem[cursor_xpos][cursor_ypos],1);
      }
   }
}
///}
///{ "amiga_fail()"
int amiga_fail(char *msg)
{
   int i,p;

   // Print error message
   if(msg) fprintf(stderr,"%s\n",msg);

   // Unlock public screen
   if(wblock) {
      UnlockPubScreen(NULL,wbscr);
      wblock=FALSE;
   }

   // Close intuition window
   if(amiwin) {
      CloseWindow(amiwin);
      amiwin=NULL;
   }
   // Free obtained pens
   if(wbscr) {
      for(i=0; i<32; i++) ReleasePen(wbscr->ViewPort.ColorMap,wbpens[i]);
   }
   // Close intuition screen
   if(amiscr) {
      CloseScreen(amiscr);
      amiscr=NULL;
   }
   // Free chunky gfx buffer
   if(chunkygfx) {
      FreeMem(chunkygfx, font_w*font_h);
      chunkygfx=NULL;
   }
   if(chunkytmp) {
      FreeMem(chunkytmp, tmp_w*font_h);
      chunkytmp=NULL;
   }
   // Free bitmap and its bitplanes
   if(gfxbm) {
      if(v39) {
         FreeBitMap(gfxbm);
      } else {
         for(p=0; p<5; p++) if(gfxbm->Planes[p]) FreeRaster(gfxbm->Planes[p],gfx_w,gfx_h);
         FreeMem(gfxbm, sizeof(struct BitMap));
      }
      gfxbm=NULL;
   }
   if(mapbm) {
      if(v39) {
         FreeBitMap(mapbm);
      } else {
         for(p=0; p<5; p++) if(mapbm->Planes[p]) FreeRaster(mapbm->Planes[p],map_w,map_h);
         FreeMem(mapbm, sizeof(struct BitMap));
      }
      mapbm=NULL;
   }
   // Close font
   if(ownfont && font) {
      CloseFont(font);
      font=NULL;
      ownfont=FALSE;
   }
   // Close keymap.library
   if(KeymapBase) {
      CloseLibrary(KeymapBase);
      KeymapBase=NULL;
   }
   // Return failure
   return(-1);
}
///}
///{ "amiga_map()"
void amiga_map(void)
{
   int i,j;
   byte ta,tc;

   // Only in planar graphics mode
   if(!use_graphics || use_chunky) return;

   // Save screen
   Term_save();

   // Clear screen
   Term_clear();
   Term_fresh();

   // Calculate offset values
   map_x = ((font_w * 80) - (mpt_w * cur_wid)) / 2;
   map_y = ((font_h * 24) - (mpt_h * cur_hgt)) / 2;

   // Draw all "interesting" features
   for(i=0; i<cur_wid; i++) {
      for(j=0; j<cur_hgt; j++) {
         // Get frame tile
         if(i==0 || i==cur_wid-1 || j==0 || j==cur_hgt-1) {
            ta = f_info[63].z_attr;
            tc = f_info[63].z_char;
         }
         // Get tile
         else {
            map_info(j, i, &ta, (char *)&tc);
         }
         // Ignore non-graphics
         if(ta&0x80) {
            ta &= 0x1f;
            tc &= 0x1f;
            // Player
            if(ta==12 && tc==0) {
               ta = ((p_ptr->pclass * 10 + p_ptr->prace) >> 5) + 12;
               tc = ((p_ptr->pclass * 10 + p_ptr->prace) & 0x1f);
            }
            BltBitMapRastPort(mapbm, tc*mpt_w, ta*mpt_h, rp, map_x+i*mpt_w, map_y+j*mpt_h, mpt_w, mpt_h, 0xc0);
         }
      }
   }

   // Wait for a keypress, flush key buffer
   cursor_map=TRUE;
   Term_inkey(&tc,TRUE,TRUE);
   Term_flush();
   cursor_map=FALSE;

   // Restore screen
   Term_clear();
   Term_fresh();
   Term_load();
}
///}
///{ "load_palette()"
void load_palette(void)
{
   int i;

   if(amiscr) {
      if(v39) {
         palette32[0]=16<<16;
         palette32[16*3+1]=0;
         for(i=0; i<16; i++) {
            palette32[i*3+1] = color_table[use_graphics?i+16:i][1] << 24;
            palette32[i*3+2] = color_table[use_graphics?i+16:i][2] << 24;
            palette32[i*3+3] = color_table[use_graphics?i+16:i][3] << 24;
         }
         LoadRGB32(&amiscr->ViewPort, palette32);
      } else {
         for(i=0; i<16; i++) {
            palette4[i] =  (color_table[use_graphics?i+16:i][1] >> 4) << 8;
            palette4[i] |= (color_table[use_graphics?i+16:i][2] >> 4) << 4;
            palette4[i] |= (color_table[use_graphics?i+16:i][3] >> 4);
         }
         LoadRGB4(&amiscr->ViewPort, palette4, 16);
      }
   }
}
///}

#endif /* USE_AMI */




