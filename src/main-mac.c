/* File: main-mac.c */

/* Purpose: Support for MACINTOSH Angband */

/*
 * Adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * See "term.c" for info on the "generic terminal" that we support.
 *
 * See "recall.c"/"moria1.c" for info on the "recall"/"choice" windows.
 *
 * Note that this file is the only one that requires "Non-ANSI"
 * extensions to C, in particular, the "Think C Language Extensions"
 * must be on, and the "enums are always ints" must be off.  Both of
 * these are to allow the "Events.h" header file to be parsed.
 */

#include "angband.h"


#ifdef MACINTOSH

#include <OSUtils.h>
#include <GestaltEqu.h>
#include <Palettes.h>
#include <Files.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <StandardFile.h>
#include <ToolUtils.h>
#include <Desk.h>
#include <Resources.h>
#include <QuickDraw.h>
#include <Controls.h>



/*
 * Idea from "Maarten Hazewinkel <mmhazewi@cs.ruu.nl>"
 * Optimize (non-blocking) calls to "CheckEvents(TRUE)" 
 */
#define EVENT_TICKS 6


/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * Extra "term" data
 */
struct _term_data {

  term		t;
  
  Rect		r;

  cptr		s;
  
  WindowPtr	w;

  int		extra;
  
  int		rows;
  int		cols;
  
  int		size_wid;
  int		size_hgt;

  int		size_ow1;
  int		size_oh1;
  int		size_ow2;
  int		size_oh2;
  
  int		mapped;
  int		font_id;
  int		font_size;
  int		font_face;

  int		font_wid;
  int		font_hgt;
  int		font_o_x;
  int		font_o_y;
};




/*
 * Forward declare -- see below
 */
static bool CheckEvents(bool scanning);


/*
 * Hack -- request screen to be mapped
 */
static int use_screen_win = TRUE;


/*
 * Hack -- game in progress
 */
static int game_in_progress = 0;


/*
 * Hack -- game can be saved
 */
static int save_enabled = 0;


/*
 * System "color" type
 */
static int rgbcolor = 0;


/*
 * The location of the "lib" folder
 */
static short volhint;
static long dirhint;


/*
 * The "preference" file, while reading/writing
 */
static FILE *fff;



/*
 * Only do "SetPort()" when needed
 */
static WindowPtr active = NULL;



/*
 * Some information for every "term" window
 */ 
term_data screen;
term_data recall;
term_data choice;


/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;



/*
 * The "term.c" color set:
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * This happens (surprise) to be the same as the "Angband" color set.
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 * Note that on B/W machines, all non-zero colors can be white (on black).
 *
 * Note that "Light Red" is now really "light red", not "purple".
 *
 * Note that all characters are assumed to be drawn on a black background.
 * This may require calling "Term_wipe()" before "Term_text()", etc.
 *
 * The Macintosh "Term_curs()" ignores the symbol under the cursor.
 */
static RGBColor mac_clr[16] = {
    {0,0,0},			/* 15 Black */
    {65535,65535,65535},	/* 0 White */
    {32768,32768,32768},        /* 13 Gray */
    {65535,25738,652},		/* 2 Orange */
    {45000,2000,2000},		/* 3 Red */
    {0,25775,4528},		/* 9 Green */
    {0,0,54272},		/* 6 Blue */
    {22016,11421,1316},		/* 10 Brown */
    {16384,16384,16384},	/* 14 Dark Gray */
    {49152,49152,49152},	/* 12 Light Gray */
    {18147,0,42302},		/* 5 Purple */
    {64512,62333,1327},		/* 1 Yellow */
    {60000,2000,2000},		/* 4 Light Red */
    {7969,46995,5169},		/* 8 Light Green */
    {577,43860,60159},		/* 7 Light Blue */
    {37079,29024,14900}		/* 11 Light Brown */
};


/*
 * The "term.c" Color Palette (for older machines), see names above
 * Was: { w,y,o,r,R,v,b,B,G,g, u, U, W, s, D, d };
 * Was: { 0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,1 };
 *
 * I think the indexes are taken from the resource file
 *
 * We should consider attempting to also support the "Quickdraw" palette,
 * which appears to consist of the 8 basic "RGB" colors, Black to White.
 */
static int mac_pal_clr[16] = { 1,0,14,3,4,10,7,11,15,13,6,2,5,9,8,12 };



/*
 * Activate the n'th "color".  We should never activate color "zero".
 * Note that we make use of the current "color type", and that if we
 * are on a Black/White machine, we treat ALL colors as "white".
 * This will cause bizarre results if we are asked to draw "black"
 * things, except for "black spaces" (same as "white spaces").
 */
static void setcolor(int n)
{
    /* Hack -- Stop illegal attrs */
    if ((n < 0) || (n >= 16)) n = TERM_RED;

    /* Activate the color */
    if (rgbcolor==2)
    {
	RGBForeColor(&mac_clr[n]);
    }
    else if (rgbcolor==1)
    {
	PmForeColor(mac_pal_clr[n]);
    }
}


/*
 * Activate a given window, if necessary
 */
static void activate(WindowPtr w)
{
    /* Ignore "silly" effects */
    if (active == w) return;

    /* Activate */    
    if (w) SetPort(w);

    /* Remember */
    active = w;
}


/*
 * Display a warning message
 */
static void mac_warning(cptr warning)
{
    Str255 text;
    int len,i;

    /* Limit of 250 chars */
    len = strlen(warning);
    if (len > 250) len = 250;

    /* Make a "Pascal" string */
    text[0] = len;
    for (i=0; i<len; i++) text[i+1] = warning[i];

    /* Display the dialog box, wait for "Okay" */
    ParamText(text, "\p", "\p", "\p");
    Alert(129,0L);
}


/*
 * Delay for "x" milliseconds
 */
void delay(int x)
{
    long t; t=TickCount(); while (TickCount() < t + (x*60)/1000);
}







/*** Hooks for the "term.c" functions ***/

/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
    term *old = Term;

    Rect r;
    
    /* Activate the term */
    Term_activate(&td->t);

    /* No clipping */
    ClipRect(&td->w->portRect);

    /* Erase the window */
    EraseRect(&td->w->portRect);
    
    /* Frame the window in white */
    setcolor(TERM_WHITE);
    MoveTo(0, 0);
    LineTo(0, td->size_hgt-1);
    LineTo(td->size_wid-1, td->size_hgt-1);
    LineTo(td->size_wid-1, 0);
    
    /* Clip to the new size */
    r.left = td->w->portRect.left + td->size_ow1;
    r.top = td->w->portRect.top + td->size_oh1;
    r.right = td->w->portRect.right - td->size_ow2;
    r.bottom = td->w->portRect.bottom - td->size_oh2;
    ClipRect(&r);

    /* Redraw the contents */
    Term_redraw();

    /* Restore the old term */
    Term_activate(old);
}


/*
 * Hack -- react to a new size
 */
static void term_data_resize(term_data *td)
{
    /* Actually resize the window */
    SizeWindow(td->w, td->size_wid, td->size_hgt, 0);

#if 0
    /* XXX Unknown result */
    ClipRect(&td->w->portRect);

    /* Hack -- go ahead and erase */
    EraseRect(&td->w->portRect);

	/* Erase the window */
	SizeWindow(w, LoWord(newsize), HiWord(newsize), 1);
	ClipRect(&w->portRect);
	EraseRect(&w->portRect);

	/* Redraw Contents */
	term_data_redraw(&recall);

	/* we've drawn it all -- no need to redraw */
	ValidRect(&w->portRect);
#endif

    /* This window needs to be redraw */
    InvalRect(&td->w->portRect);
}


/*
 * React to changing global options
 */
static void Term_xtra_mac_react()
{
    /* Show */
    if (use_screen_win && !screen.mapped)
    {
	/* Mapped */
	screen.mapped = TRUE;
	
	/* Show the window */
	ShowWindow(screen.w);

	/* Redraw the window */
	term_data_redraw(&screen);
    }

    /* Hide */
    if (!use_screen_win && screen.mapped)
    {
	/* Not Mapped */
	screen.mapped = FALSE;
	
	/* Hide the window */
	HideWindow(screen.w);
    }


    /* Show */
    if (use_recall_win && !recall.mapped)
    {
	/* Mapped */
	recall.mapped = TRUE;
	
	/* Show the window */
	ShowWindow(recall.w);

	/* Redraw the window */
	term_data_redraw(&recall);
    }

    /* Hide */
    if (!use_recall_win && recall.mapped)
    {
	/* Not Mapped */
	recall.mapped = FALSE;
	
	/* Hide the window */
	HideWindow(recall.w);
    }


    /* Show */
    if (use_choice_win && !choice.mapped)
    {
	/* Mapped */
	choice.mapped = TRUE;
	
	/* Show the window */
	ShowWindow(choice.w);

	/* Redraw the window */
	term_data_redraw(&choice);
    }

    /* Hide */
    if (!use_choice_win && choice.mapped)
    {
	/* Not Mapped */
	choice.mapped = FALSE;
	
	/* Hide the window */
	HideWindow(choice.w);
    }
}








/*** Function hooks needed by "Term" ***/


/*
 * Initialize a new Term
 */
static void Term_init_mac(term *t)
{
    term_data *td = (term_data*)(t->data);

    Str255 title;

    /* Extract the title */
    strcpy((char*)(title+1), td->s);
    title[0] = strlen(td->s);
    
#if 0

    /* Make the window */
    if (rgbcolor)
    {
	td->w = NewCWindow(0,&td->r,title,true,noGrowDocProc,(WindowPtr)-1,0,0);
    }
    else
    {
	td->w = NewWindow(0,&td->r,title,true,noGrowDocProc,(WindowPtr)-1,0,0);
    }

#else

    /* Make the window */
    if (rgbcolor)
    {
	td->w = NewCWindow(0,&td->r,title,td->mapped,documentProc,(WindowPtr)-1,1,0L);
    }
    else
    {
	td->w = NewWindow(0,&td->r,title,td->mapped,documentProc,(WindowPtr)-1,1,0L);
    }

#endif

    /* Activate the window */
    activate(td->w);

    /* Activate the font */
    TextFont(td->font_id);
    TextSize(td->font_size);
    TextFace(td->font_face);

    /* Prepare the colors */
    if (rgbcolor==2)
    {
	RGBColor c;
	c.red=0;
	c.green=0;
	c.blue=0;
	RGBBackColor(&c);
	c.red=0xFFFF;
	c.green=0xFFFF;
	c.blue=0xFFFF;
	RGBForeColor(&c);
    }
    else if (rgbcolor==1)
    {
	PaletteHandle p;
	p=GetNewPalette(128);
	NSetPalette(td->w,p,pmAllUpdates);
	BackColor(blackColor);
	ForeColor(whiteColor);
    }
    else
    {
	BackColor(blackColor);
	ForeColor(whiteColor);
    }


    /* Important -- Use "white" by default */
    setcolor(TERM_WHITE);


    /* Erase the whole window */
    EraseRect(&td->w->portRect);

    /* Force-Expose the window */
    InvalRect(&td->w->portRect);
}



/*
 * Nuke an old Term
 */
static void Term_nuke_mac(term *t)
{
    /* XXX */
}



/*
 * Scan for events (do not block)
 */
static errr Term_xtra_mac_check(int v)
{
    int res;
    
    term *old = Term;
    
    /* Hack -- all keypresses go to the screen */
    Term_activate(term_screen);
    
    /* Check for one event */
    res = (CheckEvents(TRUE) ? 0 : 1);

    /* Hack -- restore the Term */
    Term_activate(old);
    
    /* Hack -- check windows */
    Term_xtra_mac_react();

    /* Success */
    return (res);
}


/*
 * Scan for events (block)
 */
static errr Term_xtra_mac_event(int v)
{
    term *old = Term;
    
    /* Hack -- all keypresses go to the screen */
    Term_activate(term_screen);
    
    /* Block until an event occurs */
    while (!CheckEvents(FALSE));

    /* Hack -- restore the Term */
    Term_activate(old);
    
    /* Hack -- check windows */
    Term_xtra_mac_react();

    /* Success */
    return (0);
}


/*
 * Handle change in the "level"
 */
static errr Term_xtra_mac_level(int v)
{
    term_data *td = (term_data*)(Term->data);
    
    /* Hack -- Handle "software activate" */
    if (v == TERM_LEVEL_SOFT_OPEN) activate(td->w);

    /* Success */
    return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_mac(int n, int v)
{
    switch (n)
    {
	case TERM_XTRA_NOISE: SysBeep(1); return (0);
	case TERM_XTRA_CHECK: return (Term_xtra_mac_check(v));
	case TERM_XTRA_EVENT: return (Term_xtra_mac_event(v));
	case TERM_XTRA_LEVEL: return (Term_xtra_mac_level(v)); 
    }

    /* Oops */
    return (1);
}



/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_grab()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_curs_mac(int x, int y, int z)
{
    Rect r;
    
    term_data *td = (term_data*)(Term->data);

    /* Cursor is done as a yellow "box" */
    setcolor(TERM_YELLOW);

    /* Frame the grid */
    r.left = x * td->font_wid + td->size_ow1;
    r.right = r.left + td->font_wid;
    r.top = y * td->font_hgt + td->size_oh1;
    r.bottom = r.top + td->font_hgt;
    FrameRect(&r);
    
    /* Success */
    return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase a "block" of characters starting at (x,y), with size (w,h)
 */
static errr Term_wipe_mac(int x, int y, int w, int h)
{
    Rect r;

    term_data *td = (term_data*)(Term->data);

    /* Erase the block of characters */
    r.left = x * td->font_wid + td->size_ow1;
    r.right = r.left + w * td->font_wid;
    r.top = y * td->font_hgt + td->size_oh1;
    r.bottom = r.top + h * td->font_hgt;
    EraseRect(&r);
    
    /* Success */
    return (0);
}


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 */
static errr Term_text_mac(int x, int y, int n, byte a, cptr s)
{
    term_data *td = (term_data*)(Term->data);

    /* Hack -- Stop illegal chars */
    if (a >= 16) a = TERM_RED;

    /* Erase behind the characters */
    Term_wipe(x, y, n, 1);

    /* Activate the color */
    setcolor(a);

    /* Move to the correct location */
    MoveTo(x * td->font_wid + td->font_o_x + td->size_ow1,
	   y * td->font_hgt + td->font_o_y + td->size_oh1);

    /* Sometimes Just draw a single character */
    if (n == 1) DrawChar(s[0]);

    /* Draw the string (will only work for mono-spaced fonts) */
    else DrawText(s, 0, n);
    
    /* Success */
    return (0);
}





/*
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
    term *t = &td->t;


    /* Initialize the term */
    term_init(t, 80, 24, 64);

    /* Prepare the template values */
    t->soft_cursor = TRUE;
    t->scan_events = TRUE;

    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_mac;
    t->nuke_hook = Term_nuke_mac;
    
    /* Prepare the template hooks */
    t->xtra_hook = Term_xtra_mac;
    t->curs_hook = Term_curs_mac;
    t->wipe_hook = Term_wipe_mac;
    t->text_hook = Term_text_mac;

    /* Remember where we came from */
    t->data = (vptr)(td);
    
    
    /* Activate it */
    Term_activate(t);
}








/*
 * Convert refnum+vrefnum+fname into a full file name
 * Store this filename in 'buf' (make sure it is long enough)
 * Note that 'fname' looks to be a "pascal" string
 */
static void refnum_to_name(char *buf,long refnum,short vrefnum,char *fname)
{
    DirInfo pb;
    Str255 name;
    int err;
    int i,j;

    char res[1000];

    i=999;

    res[i]=0; i--;
    for (j=1; j<=fname[0]; j++)
    {
	res[i-fname[0]+j] = fname[j];
    }
    i-=fname[0];

    pb.ioCompletion=NULL;
    pb.ioNamePtr=name;
    pb.ioVRefNum=vrefnum;
    pb.ioDrParID=refnum;
    pb.ioFDirIndex=-1;

    while (1)
    {
	pb.ioDrDirID=pb.ioDrParID;
	err = PBGetCatInfo((CInfoPBPtr)&pb,FALSE);
	res[i] = ':'; i--;
	for(j=1; j<=name[0]; j++)
	{
	    res[i-name[0]+j] = name[j];
	}
	i -= name[0];

	if (pb.ioDrDirID == fsRtDirID) break;
    }

    /* Extract the result */
    for (j = 0, i++; res[i]; j++, i++) buf[j] = res[i];
    buf[j] = 0;
}


/*
 * Write a file describing the character
 */
int mac_file_character()
{
    SFReply reply;
    Str255 default_name;
    char fullname[1024];
    Point topleft;
    short vrefnum;
    long drefnum,longjunk;

    if ((player_name[strlen(player_name)-1]=='s') ||
	(player_name[strlen(player_name)-1]=='S'))
    {
	sprintf((char*)default_name + 1,"%s' description",player_name);
    }
    else
    {
	sprintf((char*)default_name + 1,"%s's description",player_name);
    }
    default_name[0] = strlen((char*)default_name + 1);

    topleft.h=(qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
    topleft.v=(2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-188/2;
    SFPutFile(topleft,"\pSave description as:",default_name,NULL,&reply);
    /* StandardPutFile("\pSave description as:",default_name,&reply); */

    if (reply.good)
    {
	int fc;
	GetWDInfo(reply.vRefNum,&vrefnum,&drefnum,&longjunk);
	refnum_to_name(fullname,drefnum,vrefnum,(char*)reply.fName);

	/* Global flags: TeachText "TEXT" file */
	_ftype='TEXT';
	_fcreator='ttxt';

	/* File that character */
	fc = file_character(fullname);

	/* Global flags: Angband "TEXT" file */
	_ftype='TEXT';
	_fcreator='A271';

	return (fc);
    }

    return(FALSE);
}



static int gotlibdir;
static short libvol;
static long libdir;


static pascal Boolean foldersonlyfilter(ParmBlkPtr pb, void *junk);
static pascal Boolean foldersonlyfilter(ParmBlkPtr pb, void *junk)
{
    FileParam *d=(FileParam*)pb;
    return (!(d->ioFlAttrib & (1<<4)));
}

static pascal short findlibdialoghook(short item, DialogPtr dialog, void *junk);
static pascal short findlibdialoghook(short item, DialogPtr dialog, void *junk)
{
    if (item==2) gotlibdir=0;
    if (item!=10) return(item);

    return (sfItemCancelButton);
}


static void center_rect(Rect *r, Rect *s)
{
    int centerx = (s->left + s->right)/2;
    int centery = (2*s->top + s->bottom)/3;
    int dx = centerx - (r->right - r->left)/2 - r->left;
    int dy = centery - (r->bottom - r->top)/2 - r->top;
    r->left += dx;
    r->right += dx;
    r->top += dy;
    r->bottom += dy;
}


/*
 * Let the user track down the "lib" folder
 */
static int find_and_set_lib_dir()
{
    Point topleft;
    StandardFileReply reply;
    SFTypeList t;
    short vrefnum;
    long drefnum,longjunk;
    long response;
    AlertTHndl alert;
    FILE *f;

    /* I need StandardGetFile to make this work... */
    Gestalt(gestaltStandardFileAttr,&response);
    if (!(response & (1<<gestaltStandardFile58)))
    {
	alert=(AlertTHndl)GetResource('ALRT',130);
	center_rect(&(*alert)->boundsRect,&qd.screenBits.bounds);
	Alert(130,NULL);
	quit(NULL);
    }

    /* try hints in prefs file */
    if (!HSetVol(0, volhint, dirhint))
    {
	/* Try again */
	f = fopen(ANGBAND_NEWS, "r");

	/* hint was successful! */
	if (f)
	{
	    fclose(f);
	    return(1);
	}
    }

    /* Keep asking for the lib directory */
    while (1)
    {
	char buf[256];

	SysBeep(1);
	topleft.h=topleft.v=-1;
	gotlibdir=1;

	sprintf(buf, "%s\r%s\r%s",
		"Cannot access the Angband 'lib' folder.",
		"Press return and then locate this folder.",
		"You will be given a chance to 'cancel'.");
	mac_warning(buf);

	CustomGetFile(foldersonlyfilter,-1,t,&reply,
	    130,topleft,findlibdialoghook,NULL,NULL,NULL,NULL);

	/* Abort if requested */
	if (!gotlibdir) quit(NULL);

	/* Apply the user's folder choice */
	HSetVol(0,reply.sfFile.vRefNum,reply.sfFile.parID);

	/* Try again */
	f = fopen(ANGBAND_NEWS, "r");

	/* Success */
	if (f)
	{
	    /* hint was successful! */
	    fclose(f);

	    /* save vol/dir for next time */
	    volhint = reply.sfFile.vRefNum;
	    dirhint = reply.sfFile.parID;

	    return(1);
	}
    }
}





/*
 * Very simple "preference file" read/write routines
 */

static int getshort(void)
{
    short x;
    if (fscanf(fff, "%d", &x) != 1) return (0);
    return (x);
}

static void putshort(int x)
{
    fprintf(fff, "%d\n", x);
}

static long getlong(void)
{
    long x;
    if (fscanf(fff, "%ld", &x) != 1) return (0);
    return (x);
}

static void putlong(long x)
{
    fprintf(fff, "%ld\n", x);
}



/*
 * Extract the "font sizing" information
 */
static void term_data_check_font(term_data *td)
{
    Rect r;
    WindowPtr tmpw;
    FontInfo info;

    /* Remember the old window */
    WindowPtr old = active;

    /* Fake window */
    r.left = r.right = r.top = r.bottom = 0;
    tmpw = NewWindow(0,&r,"\p",false,documentProc,0,0,0);

    /* Activate the "fake" window */
    activate(tmpw);

    /* Prepare the font */
    TextFont(td->font_id);
    TextSize(td->font_size);
    TextFace(td->font_face);

    /* Extract the font info */
    GetFontInfo(&info);

    /* Extract the font sizing values */
    td->font_wid = CharWidth('@'); /* info.widMax; */
    td->font_hgt = info.ascent + info.descent;
    td->font_o_x = 0;
    td->font_o_y = info.ascent;

    /* Destroy the old window */
    DisposeWindow(tmpw);

    /* Re-activate the old window */
    activate(old);
}


/*
 * Extract the "window sizing" information
 */
static void term_data_check_size(term_data *td)
{
    /* Calculate full window size */
    td->size_wid = td->cols * td->font_wid + td->size_ow1 + td->size_ow2;
    td->size_hgt = td->rows * td->font_hgt + td->size_oh1 + td->size_oh2;

    /* Verify the top */
    if (td->r.top > qd.screenBits.bounds.bottom - td->size_hgt)
    {
	td->r.top = qd.screenBits.bounds.bottom - td->size_hgt;
    }

    /* Verify the top */
    if (td->r.top < qd.screenBits.bounds.top + 30)
    {
	td->r.top = qd.screenBits.bounds.top + 30;
    }

    /* Verify the left */
    if (td->r.left > qd.screenBits.bounds.right - td->size_wid)
    {
	td->r.left = qd.screenBits.bounds.right - td->size_wid;
    }

    /* Verify the left */
    if (td->r.left < qd.screenBits.bounds.left)
    {
	td->r.left = qd.screenBits.bounds.left;
    }

    /* Calculate bottom right corner */
    td->r.right = td->r.left + td->size_wid;
    td->r.bottom = td->r.top + td->size_hgt;
}


/*
 * Write the "preference" data to the current "file"
 */
static void save_prefs()
{
    Point p;

    term_data *td;
    

    /*** Future Data ***/

    putshort(0);
    putshort(0);
    putshort(0);
    putshort(0);


    /*** The "volume" info ***/

    putshort(0);
    putshort(volhint);
    putlong(dirhint);


    /*** The "screen" info ***/

    td = &screen;
    
    activate(td->w);

    putshort(1); /* Always mapped */
    
    putshort(td->font_id);
    putshort(td->font_size);
    putshort(td->font_face);

    putshort(td->cols);
    putshort(td->rows);

    p.h = td->w->portRect.left;
    p.v = td->w->portRect.top;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);


    /*** The "recall" info ***/

    td = &recall;
    
    activate(td->w);

    putshort(td->mapped);
    
    putshort(td->font_id);
    putshort(td->font_size);
    putshort(td->font_face);

    putshort(td->cols);
    putshort(td->rows);

    p.h = td->w->portRect.left;
    p.v = td->w->portRect.top;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);


    /*** The "choice" info ***/

    td = &choice;
    
    activate(td->w);

    putshort(td->mapped);
    
    putshort(td->font_id);
    putshort(td->font_size);
    putshort(td->font_face);

    putshort(td->cols);
    putshort(td->rows);

    p.h = td->w->portRect.left;
    p.v = td->w->portRect.top;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);
}


/*
 * Load the preferences from the current "file"
 */
static void load_prefs(void)
{
    int temp;

    term_data *td;


    /*** Later ***/

    temp = getshort();
    temp = getshort();
    temp = getshort();
    temp = getshort();


    /*** Folder info ***/

    /* Volume/Directory hints for lib folder */
    temp = getshort();
    volhint = getshort();
    dirhint = getlong();


    /*** Screen info ***/

    td = &screen;
    
    use_screen_win = td->mapped = getshort();

    td->font_id = getshort();
    td->font_size = getshort();
    td->font_face = getshort();

    term_data_check_font(td);

    td->cols = getshort();
    td->rows = getshort();

    td->r.left = getshort();
    td->r.top = getshort();

    term_data_check_size(td);
    

    /*** Recall info ***/

    td = &recall;
    
    use_recall_win = td->mapped = getshort();

    td->font_id = getshort();
    td->font_size = getshort();
    td->font_face = getshort();

    term_data_check_font(td);

    td->cols = getshort();
    td->rows = getshort();

    td->r.left = getshort();
    td->r.top = getshort();

    term_data_check_size(td);
    

    /*** Choice info ***/

    td = &choice;
    
    use_choice_win = td->mapped = getshort();

    td->font_id = getshort();
    td->font_size = getshort();
    td->font_face = getshort();

    term_data_check_font(td);

    td->cols = getshort();
    td->rows = getshort();

    td->r.left = getshort();
    td->r.top = getshort();

    term_data_check_size(td);
}





/*
 * Read the preference file, Create the windows.
 */
static void init_windows()
{
    Rect r;
    SysEnvRec env;
    short savev;
    long saved;
    int i;

    short fid;

    term_data *td;
    
    
    /* Default to Monaco font */
    GetFNum("\pmonaco", &fid);


    /* Recall window */
    td = &recall;
    WIPE(td, term_data);
    td->s = "Recall";
    td->size_ow1 = 2;
    td->size_ow2 = 2;
    td->size_oh2 = 2;
    
    /* Recall (Monaco 9) */
    td->mapped = TRUE;
    td->font_id = fid;
    td->font_size = 9;
    td->font_face = 0;
    term_data_check_font(td);

    /* Recall (80x24, top left) */
    td->rows = 24;
    td->cols = 80;
    td->r.left = 10;
    td->r.top = 40;
    term_data_check_size(td);


    /* Choice window */
    td = &choice;
    WIPE(td, term_data);
    td->s = "Choice";
    td->size_ow1 = 2;
    td->size_ow2 = 2;
    td->size_oh2 = 2;
    
    /* Choice (Monaco 9) */
    td->mapped = TRUE;
    td->font_id = fid;
    td->font_size = 9;
    td->font_face = 0;
    term_data_check_font(td);

    /* Choice (80x24, bottom left) */
    td->rows = 24;
    td->cols = 80;
    td->r.left = recall.r.left;
    td->r.top = recall.r.bottom + 40;
    term_data_check_size(td);


    /* Screen window */
    td = &screen;
    WIPE(td, term_data);
    td->s = "Angband";
    td->size_ow1 = 2;
    td->size_ow2 = 2;
    td->size_oh2 = 2;
        
    /* Screen (Monaco 12) */
    td->mapped = TRUE;
    td->font_id = fid;
    td->font_size = 12;
    td->font_face = 0;
    term_data_check_font(td);

    /* Screen (80x24, to the right of Choice) */
    td->rows = 24;
    td->cols = 80;
    td->r.left = choice.r.right + 10;
    td->r.top = choice.r.top;
    term_data_check_size(td);


    /* Attempt to open a preference file */
    SysEnvirons(curSysEnvVers, &env);
    HGetVol(0,&savev,&saved);
    SetVol(0, env.sysVRefNum);
    fff = fopen(":Preferences:Angband 2.7.2 Preferences", "r");
    if (!fff) fff = fopen(":Angband 2.7.2 Preferences", "r");
    HSetVol(0, savev, saved);

    /* Parse it */
    if (fff)
    {
	/* Load a real preference file */
	load_prefs();

	/* Close the file */	
	fclose(fff);
    }


    /* Link/Activate the Choice "term" */
    term_data_link(&choice);
    term_choice = &choice.t;
    
    /* Link/Activate the Recall "term" */
    term_data_link(&recall);
    term_recall = &recall.t;

    /* Link/Activate the Screen "term" */
    term_data_link(&screen);
    term_screen = &screen.t;
}


/*
 * Exit the program
 */
static void save_pref_file(void)
{
    SysEnvRec env;

    SysEnvirons(curSysEnvVers, &env);
    SetVol(0, env.sysVRefNum);

    /* It is a text file */
    _ftype = 'TEXT';

    /* Open the preference file */
    fff = fopen(":Preferences:Angband 2.7.2 Preferences", "w");
    if (!fff) fff = fopen(":Angband 2.7.2 Preferences", "w");

    /* Write prefs if allowed */
    if (fff)
    {
	/* Write the preferences */
	save_prefs();

	/* Close it */
	fclose(fff);
    }
}




/*
 * Set up the menus
 */
static void init_menubar()
{
    MenuHandle menu;

    /* Get the menubar from the resource file */
    SetMenuBar(GetNewMBar(128));

    /* Get the "apple" menu and add the "da's" */
    menu=GetMenu(128);
    AddResMenu(menu,'DRVR');

    /* Insert the "apple" menu as the first menu */
    InsertMenu(menu,129);

#if 0
    menu=GetMenu(131);
    AddResMenu(menu,'FONT');
    InsertMenu(menu,132);
#endif

    /* Display the menubar */
    DrawMenuBar();
}


/*
 * Initialize the "colors"
 */
static void init_colors()
{
    long response;

    /* take low-order word of QDV */
    Gestalt(gestaltQuickdrawVersion,&response);
    response = response & 0xFFFFL;

    /* Examine that value */
    if (response>=gestalt32BitQD13)
    {
	rgbcolor = 2; /* use RGBForeColor... */
    }
    else if (response==gestalt8BitQD ||
	     response==gestalt32BitQD ||
	     response==gestalt32BitQD11 ||
	     response==gestalt32BitQD12)
    {
	rgbcolor = 1; /* use PmForeColor... */
    }
    else
    {
	rgbcolor = 0; /* no "color" at all */
    }
}



/*
 * disables new and open from file menu
 */
static void disable_start()
{
    MenuHandle m;
    m=GetMHandle(129);
    DisableItem(m,1);
    DisableItem(m,2);
}



/*
 * Prepare the menus
 *   File (129) = { New,Open,Close,Save,-,Exit,Quit }
 *   Edit (130) = { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) = { Bold, Condensed, -, Monaco, Courier, Angben, ... }
 *   Size (132) = { 9, 10, ..., 18 }
 *   Options (133) = { Angband, -, Recall, Choice }
 */
static void setup_menus()
{
    MenuHandle m;
    int i;
    short fnum,fsize,currfsize;
    Str255 s;

    term_data *td = NULL;
    

    /* Hack -- extract the "can I save" flag */
    save_enabled = character_generated;


    /* Extract the frontmost "term_data" */
    if (FrontWindow() == screen.w) td = &screen;
    if (FrontWindow() == recall.w) td = &recall;
    if (FrontWindow() == choice.w) td = &choice;


    /* File menu ("Save" options) */
    m=GetMHandle(129);

    /* Enable/Disable "save" options */
    if (save_enabled)
    {
	EnableItem(m,4);
    }
    else
    {
	DisableItem(m,4);
    }


    /* Font menu */
    m=GetMHandle(131);

    /* Check the appropriate "bold-ness" */
    CheckItem(m, 1, (td && (td->font_face & bold)));

    /* Check the appropriate "wide-ness" */
    CheckItem(m, 2, (td && (td->font_face & extend)));

    /* Check the appropriate "Font" for the current window */
    for (i = 4; i <= CountMItems(m); i++)
    {
	GetItem(m,i,s);
	GetFNum(s,&fnum);
	CheckItem(m, i, (td && (td->font_id == fnum)));
    }


    /* Size menu */
    m=GetMHandle(132);

    /* Process the "size" options */
    for (i=1; i<=CountMItems(m); i++)
    {
	GetItem(m,i,s);
	s[s[0]+1]=0;
	fsize = atoi((char*)(s+1));

	/* Enable the "real" sizes */
	if (td && RealFont(td->font_id, fsize))
	{
	    EnableItem(m,i);
	}
	else
	{
	    DisableItem(m,i);
	}

	/* Check the current size */
	CheckItem(m, i, (td && (td->font_size == fsize)));
    }


    /* Windows menu */
    m=GetMHandle(133);

    /* Item "Angband Window" */
    CheckItem(m, 1, screen.mapped);

    /* Item "Recall Window" */
    CheckItem(m, 3, recall.mapped);

    /* Item "Choice Window" */
    CheckItem(m, 4, choice.mapped);
}


static pascal Boolean ynfilter (DialogPtr dialog, EventRecord *event, short *item);
static pascal Boolean ynfilter (DialogPtr dialog, EventRecord *event, short *item)
{
    char c;
    long t;
    ControlHandle control;
    Rect r;
    short type;

    if (event->what == keyDown)
    {
	c=event->message&charCodeMask;

	if (c=='y' || c=='Y')
	{
	    GetDItem(dialog,2,&type,(Handle*)&control,&r);
	    HiliteControl(control,1);
	    t=TickCount();
	    while(TickCount()-t<6);
	    HiliteControl(control,0);
	    *item=2;
	    return(1);
	}

	if (c=='n' || c=='N' || c==13 || c==3)
	{
	    GetDItem(dialog,1,&type,(Handle*)&control,&r);
	    HiliteControl(control,1);
	    t=TickCount();
	    while(TickCount()-t<6);
	    HiliteControl(control,0);
	    *item=1;
	    return(1);
	}
    }

    return(0);
}


/*
 * Process a menu selection
 */
static void menu(long mc)
{
    int menuid,selection;
    static unsigned char s[1000];
    SFTypeList types;
    SFReply reply;
    int err;
    DialogPtr dialog;
    short item_hit;
    Point topleft;
    Point botright;
    Rect r;
    AlertTHndl alert;
    int i;
    s32b j;
    char *tmp;

    short vrefnum;
    long drefnum;
    long longjunk;
    DirInfo pb;

    short fid;

    term_data *td = NULL;


    /* Hack -- extract "can I save" flag */
    save_enabled = character_generated;

    /* Analyze the menu command */
    menuid = HiWord(mc);
    selection = LoWord(mc);

    /* Activate the current "term" */
    if (FrontWindow() == screen.w) td = &screen;
    if (FrontWindow() == recall.w) td = &recall;
    if (FrontWindow() == choice.w) td = &choice;
    
    /* Hack -- Activate the appropriate term */
    if (td) Term_activate(&td->t);


    /* Branch on the menu */
    switch (menuid)
    {
	case 128:	/* Apple Menu */

	    if (selection==1)
	    {
		dialog=GetNewDialog(128,0,(WindowPtr)-1);

		r=dialog->portRect;
		center_rect(&r,&qd.screenBits.bounds);
		MoveWindow(dialog,r.left,r.top,1);
		ShowWindow(dialog);
		ModalDialog(0,&item_hit);
		DisposDialog(dialog);
		break;
	    }

	    GetItem(GetMHandle(128),selection,s);
	    OpenDeskAcc(s);
	    break;

	case 129:	/* File Menu {New,Open,Close,Save,-,Exit,Quit} */

	    switch (selection)
	    {
		case 1:		/* New */
		    if (!initialized)
		    {
			mac_warning("You cannot do that yet...");
		    }
		    else if (game_in_progress)
		    {
			mac_warning("You can't start a new game while you're still playing!");
		    }
		    else if (td != &screen)
		    {
			mac_warning("The Angband window must be in front.");
		    }
		    else
		    {
			HiliteMenu(0);
			game_in_progress=1;
			disable_start();
			flush();
			play_game_mac(TRUE);
			quit(NULL);
		    }
		    break;

		case 2:		/* Open... */

		    if (!initialized)
		    {
			mac_warning("You cannot do that yet...");
		    }
		    else if (game_in_progress)
		    {
			mac_warning("You can't open a new game while you're still playing!");
		    }
		    else if (td != &screen)
		    {
			mac_warning("The Angband window must be in front.");
		    }
		    else
		    {
			types[0]='SAVE';
			vrefnum=-*((short*)0x214); /* vrefnum=GetSFCurVol(); */
			drefnum=*((long*)0x398);   /* drefnum=GetSFCurDir(); */

			/* descend into lib directory if possible */
			pb.ioCompletion=NULL;
			pb.ioNamePtr="\plib";
			pb.ioVRefNum=vrefnum;
			pb.ioDrDirID=drefnum;
			pb.ioFDirIndex=0;

			err=PBGetCatInfo((CInfoPBPtr)&pb,FALSE);
			if (err!=noErr) goto do_sf;
			if (!(pb.ioFlAttrib & 0x10)) goto do_sf;

			/* descend into save directory if possible */
			pb.ioCompletion=NULL;
			pb.ioNamePtr="\psave";
			pb.ioVRefNum=vrefnum;
			pb.ioDrDirID=pb.ioDrDirID;
			pb.ioFDirIndex=0;

			err=PBGetCatInfo((CInfoPBPtr)&pb,FALSE);
			if (err!=noErr) goto do_sf;
			if (!(pb.ioFlAttrib & 0x10)) goto do_sf;

			*((long*)0x398)=pb.ioDrDirID; /* SetSFCurDir(pb.ioDrDirID); */

			do_sf:

			topleft.h=(qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
			topleft.v=(2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-188/2;
			SFGetFile(topleft,"\p",NULL,1,types,NULL,&reply);
			if (reply.good)
			{
			    /* Load 'savefile' */
			    GetWDInfo(reply.vRefNum,&vrefnum,&drefnum,&longjunk);
			    refnum_to_name(savefile,drefnum,vrefnum,
					   (char*)reply.fName);

			    game_in_progress=1;
			    disable_start();
			    HiliteMenu(0);
			    flush();
			    play_game_mac(FALSE);
			    quit(NULL);
			}
		    }
		    break;

		case 3: /* Close */

		    if (td == &screen) use_screen_win = 0;
		    if (td == &recall) use_recall_win = 0;
		    if (td == &choice) use_choice_win = 0;

		    Term_xtra_mac_react();

		    break;
		    
		case 4: /* Save */

		    if (!game_in_progress)
		    {
			mac_warning("No game in progress.");
		    }
		    else if (td != &screen)
		    {
			mac_warning("The Angband window must be in front.");
		    }
		    else
		    {
			Term_fresh();

			/* The player is not dead */
			(void)strcpy(died_from, "(saved)");

			/* Save the player, note the result */
			prt("Saving game...",0,0);
			if (save_player()) prt("done.",0,14);
			else prt("Save failed.",0,14);

			/* Forget that the player was saved */
			character_saved = 0;

			/* Hilite the player */
			move_cursor_relative(char_row,char_col);

			/* Note that the player is not dead */
			(void)strcpy(died_from, "(alive and well)");
		    }

		    break;

		case 6:		/* Exit */

		    if (game_in_progress && save_enabled)
		    {
			alert=(AlertTHndl)GetResource('ALRT',128);
			center_rect(&(*alert)->boundsRect,&qd.screenBits.bounds);
			item_hit=Alert(128,ynfilter);

			/* Handle the "cancel" command */
			if (item_hit != 2) break;
		    }

		    /* Quit */
		    quit(NULL);
		    break;

		case 7:		/* Quit (and save) */

		    if (game_in_progress && save_enabled)
		    {
			/* Save it */
			(void)strcpy(died_from, "(saved)");
			prt("Saving game...", 0, 0);
			if (!save_player()) quit("Save failed!");
		    }

		    /* Quit */
		    quit(NULL);
		    break;
	    }
	    break;

	case 130:  /* edit menu (unused) */
	    break;

	case 131:  /* font menu */
	
	    if (!td) break;

	    /* Toggle the "bold" setting */
	    if (selection == 1)
	    {		
		/* Toggle the setting */
		if (td->font_face & bold) {
		    td->font_face &= ~bold;
		}
		else {
		    td->font_face |= bold;
		}
		
		/* Instantiate the new text face */
		TextFace(td->font_face);

		/* Resize */
		term_data_check_font(td);
		term_data_check_size(td);

		/* Resize the window */
		term_data_resize(td);

		break;
	    }

	    /* Toggle the "wide" setting */
	    if (selection == 2)
	    {		
		/* Toggle the setting */
		if (td->font_face & extend) {
		    td->font_face &= ~extend;
		}
		else {
		    td->font_face |= extend;
		}
		
		/* Instantiate the new text face */
		TextFace(td->font_face);

		/* Resize */
		term_data_check_font(td);
		term_data_check_size(td);

		/* Resize the window */
		term_data_resize(td);

		break;
	    }

	    /* Get a new font name */
	    GetItem(GetMHandle(131), selection, s);
	    GetFNum(s, &fid);
	    
	    /* Save the new font id */
	    td->font_id = fid;

	    /* current size is bad for new font */
	    if (!RealFont(td->font_id, td->font_size))
	    {
		/* find good size */
		for (i=1; i<=18; i++)
		{
		    if (td->font_size - i >= 9)
		    {
			if (RealFont(td->font_id, td->font_size - i))
			{
			    td->font_size -= i;
			    break;
			}
		    }
		    if (td->font_size + i <= 18)
		    {
			if (RealFont(td->font_id, td->font_size + i))
			{
			    td->font_size += i;
			    break;
			}
		    }
		}

		/* Default size if can't find a good size */
		if (i==19) td->font_size = 9;
	    }
	    
	    /* Instantiate new properties */
	    TextFont(td->font_id);
	    TextSize(td->font_size);
	    TextFace(td->font_face);

	    /* Adapt to the new size */
	    term_data_check_font(td);
	    term_data_check_size(td);
		
	    /* Resize the window */
	    term_data_resize(td);
	    
	    break;


	case 132:  /* size menu */

	    if (!td) break;
	    
	    GetItem(GetMHandle(132),selection,s);
	    s[s[0]+1]=0;
	    td->font_size = atoi((char*)(s+1));

	    /* Instantiate the new size */
	    TextSize(td->font_size);

	    /* React to the font */
	    term_data_check_font(td);
	    term_data_check_size(td);

	    /* Resize the window */
	    term_data_resize(td);

	    break;

	case 133:   /* window menu */
	
	    switch (selection)
	    {
		case 1:		/* Angband window */

		    /* Use the screen window */
		    use_screen_win = TRUE;

		    /* React */
		    Term_xtra_mac_react();
		    
		    /* Bring to the front */
		    SelectWindow(screen.w);
		    
		    break;

		case 3:		/* Recall window */

		    /* Use the recall window */
		    use_recall_win = TRUE;

		    /* React */
		    Term_xtra_mac_react();
		    
		    /* Bring to the front */
		    SelectWindow(recall.w);
		    
		    break;

		case 4:		/* Choice window */

		    /* Use the choice window */
		    use_choice_win = TRUE;

		    /* React */
		    Term_xtra_mac_react();
		    		    
		    /* Bring to the front */
		    SelectWindow(choice.w);
		    
		    break;
	    }
    }


    /* Activate the main screen */
    Term_activate(term_screen);

    /* Clean the menu */
    HiliteMenu(0);
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
 * Check Events, return "Did something happen?"
 */
static bool CheckEvents(bool scanning)
{
    EventRecord event;
    WindowPtr w;
    Rect r,clipsave;
    ControlHandle c;
    int cntlcode;
    long newsize;

    int ch, ck, mc, ms, mo, mx;

    WindowPtr old = active;


#ifdef EVENT_TICKS

    long curTicks = TickCount();
    static long nextTicks = 0L;

    /* Be efficient when "scanning" for events */
    if (scanning && (curTicks < nextTicks)) {

	/* Paranoia -- handle over-flow */
	if ((curTicks < 0) && (nextTicks >= 0)) nextTicks = curTicks;

	/* Do not bother to check */
        return (FALSE);
    }
    
    /* Maintain efficiency */
    nextTicks = curTicks + EVENT_TICKS;

#endif


    /* Let the "system" run */
    SystemTask();

    /* Get an event (or null) */
    GetNextEvent(everyEvent, &event);

    /* Nothing is ready */
    if (event.what == nullEvent) return (FALSE);


    /* Hack -- extract the "can I save" flag */
    save_enabled = character_generated;


    /* Analyze the event */
    switch (event.what)
    {
	case activateEvt:

	    w = (WindowPtr)event.message;

	    activate(w);

	    break;

	case updateEvt:

	    w = (WindowPtr)event.message;

	    BeginUpdate(w);

	    /* Redraw the window */
	    if (w==screen.w) term_data_redraw(&screen);
	    if (w==recall.w) term_data_redraw(&recall);
	    if (w==choice.w) term_data_redraw(&choice);

	    EndUpdate(w);

	    break;

	case keyDown:
	case autoKey:

	    /* Extract some modifiers */
	    mc = (event.modifiers & controlKey) ? TRUE : FALSE;
	    ms = (event.modifiers & shiftKey) ? TRUE : FALSE;
	    mo = (event.modifiers & optionKey) ? TRUE : FALSE;
	    mx = (event.modifiers & cmdKey) ? TRUE : FALSE;

	    /* Keypress: (only valid if ck < 96) */
	    ch = (event.message & charCodeMask) & 255;

	    /* Keycode: see above */
	    ck = ((event.message & keyCodeMask) >> 8) & 255;
	    
	    /* Normal Command key -> menu action */	    
	    if (mx && (ck < 64))
	    {
		/* Prepare the menus */
		setup_menus();

		/* Hack -- allow easy exit if nothing to save */
		if (!save_enabled && (ch=='Q' || ch=='q')) ch = 'e';

		/* Run the Menu-Handler */
		menu(MenuKey(ch));

		/* Turn off the menus */
		HiliteMenu(0);

		/* Done */
		break;
	    }


	    /* Hide the mouse pointer */
	    ObscureCursor();


	    /* Normal key -> simple keypress */
	    if (ck < 64)
	    {
		/* Enqueue the keypress */
		Term_keypress(ch);
	    }

	    /* Hack -- normal "Keypad keys" */
	    else if (!mc && !ms && !mo && !mx && (ck < 96))
	    {
		/* Hack -- "enter" is confused */
		if (ck == 76) ch = '\n';

		/* Send control-caret as a trigger */
		Term_keypress(30);
		
		/* Send the "ascii" keypress */
		Term_keypress(ch);
	    }

	    /* Bizarre key -> encoded keypress */
	    else if (ck < 127)
	    {
		/* Hack -- introduce with control-underscore */
		Term_keypress(31);
		
		/* Send some modifier keys */
		if (mc) Term_keypress('C');
		if (ms) Term_keypress('S');
		if (mo) Term_keypress('O');
		if (mx) Term_keypress('X');
		
		/* Hack -- Downshift and encode the keycode */
		Term_keypress('0' + (ck - 64) / 10);
		Term_keypress('0' + (ck - 64) % 10);
		
		/* Hack -- Terminate the sequence */
		Term_keypress(13);
	    }

	    break;

	case mouseDown:

	    switch (FindWindow(event.where,&w))
	    {
		case inMenuBar:
		
		    setup_menus();
		    menu(MenuSelect(event.where));
		    HiliteMenu(0);
		    break;

		case inSysWindow:
		
		    SystemClick(&event,w);
		    break;

		case inDrag:
		
		    r = qd.screenBits.bounds;
		    r.top += 20;
		    InsetRect(&r,4,4);
		    DragWindow(w,event.where,&r);
		    break;

		case inGoAway:
		
		    if (TrackGoAway(w,event.where))
		    {
			if (w == screen.w) use_screen_win = 0;
			if (w == recall.w) use_recall_win = 0;
			if (w == choice.w) use_choice_win = 0;

			Term_xtra_mac_react();
		    }
		    break;

		case inGrow:

		    if (w == recall.w)
		    {
			int x, y;
			
			term_data *td = &recall;
			
			activate(w);
			
			r.left = 20 * td->font_wid + td->size_ow1;
			r.right = 80 * td->font_wid + td->size_ow1 + td->size_ow2 + 1;
			r.top = 1 * td->font_hgt + td->size_oh1;
			r.bottom = 24 * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;
			newsize = GrowWindow(w, event.where, &r);
			if (!newsize) break;

			/* Extract the new size in pixels */
			y = HiWord(newsize) - td->size_oh1 - td->size_oh2;
			x = LoWord(newsize) - td->size_ow1 - td->size_ow2;
			
			/* Extract a "close" approximation */
			td->rows = y / td->font_hgt;
			td->cols = x / td->font_wid;
			
			/* React to the new size */
			term_data_check_size(td);
			
			/* Resize the window */
			term_data_resize(td);
		    }

		    if (w == choice.w)
		    {
			int x, y;
			
			term_data *td = &choice;
			
			activate(w);
			
			r.left = 20 * td->font_wid + td->size_ow1;
			r.right = 80 * td->font_wid + td->size_ow1 + td->size_ow2 + 1;
			r.top = 1 * td->font_hgt + td->size_oh1;
			r.bottom = 24 * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;
			newsize = GrowWindow(w, event.where, &r);
			if (!newsize) break;

			/* Extract the new size in pixels */
			y = HiWord(newsize) - td->size_oh1 - td->size_oh2;
			x = LoWord(newsize) - td->size_ow1 - td->size_ow2;
			
			/* Extract a "close" approximation */
			td->rows = y / td->font_hgt;
			td->cols = x / td->font_wid;
			
			/* React to the new size */
			term_data_check_size(td);
			
			/* Resize the window */
			term_data_resize(td);
		    }

		    break;

		case inContent:

		    if (w == screen.w)
		    {
			activate(w);
			GlobalToLocal(&event.where);
			SelectWindow(w);
		    }

		    if (w == recall.w)
		    {
			activate(w);
			GlobalToLocal(&event.where);
			SelectWindow(w);
		    }

		    if (w == choice.w)
		    {
			activate(w);
			GlobalToLocal(&event.where);
			SelectWindow(w);
		    }

		    break;
		}

		break;
    }


    /* Re-activate the old active window */
    activate(old);


    /* Something happened */
    return (TRUE);
}



/*
 * Check for "open via double click on a savefile"
 */
static void check_for_save_file()
{
    short message,n;
    AppFile fileinfo;
    OSErr err;
    short vrefnum;
    long drefnum,junk;

    CountAppFiles(&message,&n);
    if (message!=appOpen) n=0;

    /* can only open 1 save file at a time */
    if (n>0) n=1;

    /* Open a Save File */
    if (n==1)
    {
	GetAppFiles(1,&fileinfo);

	/* user clicked on help file or something: ignore */
	if (fileinfo.fType != 'SAVE') return;

	err = GetWDInfo(fileinfo.vRefNum,&vrefnum,&drefnum,&junk);

	if (err==noErr)
	{
	    /* Load "savefile" */
	    refnum_to_name(savefile,drefnum,vrefnum,(char*)fileinfo.fName);

	    game_in_progress=1;
	    disable_start();

	    /* Wait for it... */
	    pause_line(23);
	    flush();

	    /* Hook into "play_game()" */
	    play_game_mac(FALSE);
	}
	else
	{
	    mac_warning("Could not open save file");
	}
    }
}


/*
 * Check the lib folder
 */
static void mac_check_lib_folder(void);
static void mac_check_lib_folder()
{
    FILE *f;

    /* Try to find the "lib" folder */
    while (1)
    {

	/* Try to open the News file */
	f = my_tfopen(ANGBAND_NEWS, "r");

	/* Try to open the News file */
	if (f) break;

	/* Let the User track it down */
	find_and_set_lib_dir();
    }

    /* Close the file */
    fclose(f);
}




/*** Some Hooks for various routines ***/


/*
 * See "z-virt.c"
 */

static vptr hook_ralloc(huge size)
{
    return (NewPtr(size));
}

static errr hook_rnfree(vptr v, huge size)
{
    DisposePtr(v);
    return (0);
}


/*
 * See "z-util.c"
 */

static void hook_plog(cptr str)
{
    mac_warning(str);
}

static void hook_quit(cptr str)
{
    /* Warning if needed */
    if (str) mac_warning(str);

    /* Write a preference file */
    save_pref_file();

    /* Oh yeah, close the high score list */
    nuke_scorefile();

    /* All done */
    ExitToShell();
}

static void hook_core(cptr str)
{
    /* Warning */
    if (str) mac_warning(str);

    /* Warn, then save player */
    mac_warning("Fatal error.  I will now attempt to save and quit.");
    save_player();

    /* Just quit */
    quit(NULL);
}



/*** Main program ***/


/*
 * Macintosh Main loop
 */
void main(void)
{
    /* Mac-Mega-Hack -- about 200K of extra stack */
    SetApplLimit(GetApplLimit()-200000L);
    MaxApplZone();

    /* Set up the Macintosh */
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    InitDialogs(0);
    InitCursor();


    /* Mark ourself as the file creator */
    _fcreator = 'A271';

    /* Default to saving a "text" file */
    _ftype = 'TEXT';


    /* Hook in some "z-virt.c" hooks */
    ralloc_aux = hook_ralloc;
    rnfree_aux = hook_rnfree;

    /* Hooks in some "z-util.c" hooks */
    plog_aux = hook_plog;
    quit_aux = hook_quit;
    core_aux = hook_core;


    /* Prepare the "colors" */
    init_colors();

    /* Prepare the menubar */
    init_menubar();

    /* Prepare the windows */
    init_windows();


    /* Prepare the filepaths */
    get_file_paths();

    /* Be sure that we are in the right directory */
    mac_check_lib_folder();

    /* Get a pointer to the high score file */
    init_scorefile();

    /* Display the "news" message */
    show_news();

    /* Allocate and Initialize various arrays */
    init_some_arrays();

    /* We are now initialized */
    initialized = TRUE;
    
    /* Did the user double click on a save file? */
    check_for_save_file();

    /* Prompt the user */
    prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);
    Term_fresh();

    /* Process Events Forever */
    while (1) CheckEvents(FALSE);
}






#endif /* MACINTOSH */

