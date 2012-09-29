/* File: main-mac.c */

/* Purpose: Support for MACINTOSH Angband */

/*
 * Adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * See "term.c" for information on the "generic terminal" that we
 * support via "screen_w" and the "Term_xxx()" routines.
 *
 * See "recall.c" for info on the "recall window".
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
 * Hack -- don't reset scroll while scrolling
 */
static int scrolling = FALSE;


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
 * Reduce number of "SetPort()" commands
 */
static WindowPtr active = NULL;


/*
 * Three main windows
 */
static WindowPtr screen_w;
static WindowPtr recall_w;
static WindowPtr choice_w;

/*
 * Some rectangles
 */
static Rect screen_r;
static Rect recall_r;
static Rect choice_r;


/*
 * Two scroll bars
 */
static ControlHandle recall_sb;
static ControlHandle choice_sb;


/*
 * Font info
 */
 
static int screen_font_id;
static int screen_font_size;
static int screen_font_bold;

static int recall_font_id;
static int recall_font_size;
static int recall_font_bold;

static int choice_font_id;
static int choice_font_size;
static int choice_font_bold;


/*
 * Font size info
 */
 
static int screen_font_wid;
static int screen_font_hgt;
static int screen_font_o_x;
static int screen_font_o_y;

static int recall_font_wid;
static int recall_font_hgt;
static int recall_font_o_x;
static int recall_font_o_y;

static int choice_font_wid;
static int choice_font_hgt;
static int choice_font_o_x;
static int choice_font_o_y;


/*
 * Window size info
 */
  
static int screen_rows = 24;
static int screen_cols = 80;
static int screen_size_wid;
static int screen_size_hgt;

static int recall_rows = 8;
static int recall_cols = 80;
static int recall_size_wid;
static int recall_size_hgt;

static int choice_rows = 5;
static int choice_cols = 80;
static int choice_size_wid;
static int choice_size_hgt;





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
    {56683,2242,1698},		/* 3 Red */
    {0,25775,4528},		/* 9 Green */
    {0,0,54272},		/* 6 Blue */
    {22016,11421,1316},		/* 10 Brown */
    {16384,16384,16384},	/* 14 Dark Gray */
    {49152,49152,49152},	/* 12 Light Gray */
    {18147,0,42302},		/* 5 Purple */
    {64512,62333,1327},		/* 1 Yellow */
    {62167,2134,34028},		/* 4 Pink */
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
    /* Only do something if needed */
    if (active != w)
    {
	SetPort(w);
	active = w;
    }
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
 * Initialize the screen
 */
static void screen_init()
{
    /* XXX Force-Expose the window */
    InvalRect(&screen_w->portRect);

    /* Choose "white" color */
    setcolor(COLOR_WHITE);

    /* Erase the whole window */
    EraseRect(&screen_w->portRect);

    /* XXX The main window is now up to date */
    ValidRect(&screen_w->portRect);
}


/*
 * Redraw the screen
 */
static void screen_draw()
{
    /* Redraw the screen */
    Term_redraw();

    /* Flush the output */
    Term_fresh();
}





/*
 * Size the Recall Window.  React to a "new" size.
 */
static void recall_grow()
{
    /* Extract the columns */
    recall_cols = ((recall_w->portRect.right-15-4) / recall_font_wid);

    /* Hack -- set that as the maximum */
    recall_set_width(recall_cols);
}



/*
 * Draw the Recall Window.  Draw the scroll bar and everything.
 */
static void recall_draw()
{
    int i;
    int start;
    Rect r;
    int cols,rows;

    WindowPtr old = active;

    /* Make sure that the recall window is active */
    activate(recall_w);

    DrawControls(recall_w);
    r=recall_w->portRect;
    r.left = r.right-15;
    ClipRect(&r);
    DrawGrowIcon(recall_w);
    ClipRect(&recall_w->portRect);

    start = GetCtlValue(recall_sb);

    r=recall_w->portRect;
    r.right-=15;
    EraseRect(&r);
    cols = (r.right-4) / recall_font_wid;
    rows = r.bottom / recall_font_hgt;
    if (start + rows > 24) rows = 24 - start;

    /* Do not reset the scrollbar */
    scrolling = 1;

    /* Redraw the contents */
    recall_again();

    /* All done */
    scrolling = 0;

    /* Restore the old active window */
    activate(old);
}


/*
 * Make sure the recall window is where we thought it was
 */
static void recall_note()
{
    static last_note = 0;
    
    /* Check the note */
    if (last_note == use_recall_win) return;
    
    /* Update */
    if (use_recall_win)
    {
	ShowWindow(recall_w);
	if (FrontWindow()==recall_w)
	{
	    HiliteControl(recall_sb,0);
	}
	else {
	    HiliteControl(recall_sb,255);
	}

	/* Resize and Redraw */
	recall_grow();
	recall_draw();
    }
    else
    {
	HideWindow(recall_w);
    }

    /* Save the value */
    last_note = use_recall_win;
}


/*
 * Initialize the recall window
 */
static void recall_init()
{
    int i, j;

    /* React to the size */
    recall_grow();

    /* Not scrolling */
    scrolling = 0;

    /* Wipe and clear the recall window */
    recall_clear();

    /* Force-Expose the recall windows */
    InvalRect(&recall_w->portRect);
}




/*
 * Size the Choice Window.  React to a "new" size.
 * XXX Go to a lot of trouble to accomidate the scroll bar.
 * Should probably just require the user to resize the window.
 */
static void choice_grow()
{
    /* Extract the columns */
    choice_cols = ((choice_w->portRect.right-15-4) / choice_font_wid);

#if 0
    /* Hack -- set that as the maximum */
    choice_set_width(choice_cols);
#endif

}


/*
 * Draw the Choice Window.  Draw the scroll bar and everything.
 */
static void choice_draw()
{
    int i;
    int start;
    Rect r;
    int cols,rows;

    WindowPtr old = active;

    /* Make sure that the choice window is active */
    activate(choice_w);

    DrawControls(choice_w);
    r=choice_w->portRect;
    r.left = r.right-15;
    ClipRect(&r);
    DrawGrowIcon(choice_w);
    ClipRect(&choice_w->portRect);

    start = GetCtlValue(recall_sb);

    r=choice_w->portRect;
    r.right-=15;
    EraseRect(&r);
    cols = (r.right-4) / recall_font_wid;
    rows = r.bottom/recall_font_hgt;
    if (start + rows > 24) rows = 24 - start;

    /* Do not reset the scrollbar */
    scrolling = 1;

#if 0
    /* Redraw the contents */
    roff_choice(-1);
#endif

    /* All done */
    scrolling = 0;

    /* Restore the old active window */
    activate(old);
}


/*
 * Make sure the choice window is where we thought it was
 */
static void choice_note()
{
    static last_note = 0;
    
    /* Check the note */
    if (last_note == use_choice_win) return;
    
    /* Update */
    if (use_choice_win)
    {
	ShowWindow(choice_w);
	if (FrontWindow()==choice_w)
	{
	    HiliteControl(recall_sb,0);
	}
	else {
	    HiliteControl(recall_sb,255);
	}

	/* Resize and Redraw */
	choice_grow();
	choice_draw();
    }
    else
    {
	HideWindow(choice_w);
    }

    /* Save the value */
    last_note = use_choice_win;
}


/*
 * Initialze the choice window
 */
static void choice_init()
{
    int i, j;

    /* React to the size */
    choice_grow();

    /* Not scrolling */
    scrolling = 0;

    /* Wipe and clear the choice window */
    choice_clear();

    /* Force-Expose the choice windows */
    InvalRect(&choice_w->portRect);
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

    /* Was: "return &(res[i+1])" */

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

	/* Global flags: use a "TEXT" file */
	_ftype='TEXT';
	_fcreator='ttxt';

	/* File that character */
	fc = file_character(fullname);

	/* Global flags: use a "SAVE" file */
	_ftype='SAVE';
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
    return( !(d->ioFlAttrib & (1<<4)));
}

static pascal short findlibdialoghook(short item, DialogPtr dialog, void *junk);
static pascal short findlibdialoghook(short item, DialogPtr dialog, void *junk)
{
    if (item==2) gotlibdir=0;
    if (item!=10) return(item);

    return(sfItemCancelButton);
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

#if 0

    topleft.h=(qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
    topleft.v=(2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-197/2;

    gotlibdir=1;
    SFPGetFile(topleft,"\pLocate lib directory",foldersonlyfilter,
	       1,t,findlibdialoghook, &reply, 131, NULL);

    /* Abort? */
    if (!gotlibdir) return (0);

    GetWDInfo(reply.vRefNum,&vrefnum,&drefnum,&longjunk);

    HSetVol(0,vrefnum,drefnum);
    CloseWD(reply.vRefNum);
    return(1); /* found & reset volume/directory */

#endif

}





/*
 * Very simple "preference file" read/write routines
 */
 
static short getshort(void)
{
    short x;
    if (fscanf(fff, "%d", &x) != 1) return(0);
    return (x);
}

static void putshort(short x)
{
    fprintf(fff, "%d\n", x);
}


static long getlong(void)
{
    long x;
    if (fscanf(fff,"%ld",&x) != 1) return(0);
    return (x);
}

static void putlong(long x)
{
    fprintf(fff, "%ld\n", x);
}



/*
 * Get the "size" for the screen
 */
static void screen_getsize()
{
    Rect r;
    WindowPtr tmpw;
    FontInfo info;

    WindowPtr old = active;

    r.left = r.right = r.top = r.bottom = 0;
    tmpw = NewWindow(0,&r,"\p",false,documentProc,0,0,0);

    activate(tmpw);

    TextFont(screen_font_id);
    TextSize(screen_font_size);
    TextFace(screen_font_bold);
    
    GetFontInfo(&info);
    
    screen_font_wid = CharWidth('@'); /* info.widMax; */
    screen_font_hgt = info.ascent+info.descent;
    screen_font_o_x = 0;
    screen_font_o_y = info.ascent;

    screen_size_wid = screen_cols * screen_font_wid;
    screen_size_hgt = screen_rows * screen_font_hgt;

    DisposeWindow(tmpw);

    activate(old);
}

static void recall_getsize()
{
    Rect r;
    WindowPtr tmpw;
    FontInfo info;

    WindowPtr old = active;

    r.left = r.right = r.top = r.bottom = 0;
    tmpw = NewWindow(0,&r,"\p",false,documentProc,0,0,0);

    activate(tmpw);

    TextFont(recall_font_id);
    TextSize(recall_font_size);
    TextFace(recall_font_bold);
    
    GetFontInfo(&info);

    recall_font_wid = CharWidth('@'); /* info.widMax; */
    recall_font_hgt = info.ascent+info.descent;
    recall_font_o_x = 0;
    recall_font_o_y = info.ascent;

    DisposeWindow(tmpw);

    activate(old);
}


static void choice_getsize()
{
    Rect r;
    WindowPtr tmpw;
    FontInfo info;

    WindowPtr old = active;

    r.left = r.right = r.top = r.bottom = 0;
    tmpw = NewWindow(0,&r,"\p",false,documentProc,0,0,0);

    activate(tmpw);

    TextFont(choice_font_id);
    TextSize(choice_font_size);
    TextFace(choice_font_bold);
    
    GetFontInfo(&info);

    choice_font_wid = CharWidth('@'); /* info.widMax; */
    choice_font_hgt = info.ascent+info.descent;
    choice_font_o_x = 0;
    choice_font_o_y = info.ascent;

    DisposeWindow(tmpw);

    activate(old);
}



/*
 * Write the "preference" data to the current "file"
 */
static void save_prefs()
{
    Point p;
    WindowPtr old = active;


    /*** Future Data ***/
    
    putshort(0);
    putshort(0);
    putshort(0);
    putshort(0);

    
    /*** The "volume" info ***/
    
    putshort(volhint);
    putlong(dirhint);


    /*** The "screen" info ***/
    
    activate(screen_w);

    putshort(1);
    putshort(screen_font_id);
    putshort(screen_font_size);
    putshort(screen_font_bold);

    putshort(screen_cols);
    putshort(screen_rows);
    putshort(screen_size_wid);
    putshort(screen_size_hgt);
    
    p.h = screen_w->portRect.left;
    p.v = screen_w->portRect.top;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);

    p.h = screen_w->portRect.right;
    p.v = screen_w->portRect.bottom;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);


    /*** The "recall" info ***/
    
    activate(recall_w);

    putshort(use_recall_win);
    putshort(recall_font_id);
    putshort(recall_font_size);
    putshort(recall_font_bold);

    putshort(recall_cols);
    putshort(recall_rows);
    putshort(recall_size_wid);
    putshort(recall_size_hgt);
    
    p.h = recall_w->portRect.left;
    p.v = recall_w->portRect.top;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);

    p.h = recall_w->portRect.right;
    p.v = recall_w->portRect.bottom;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);



    /*** The "choice" info ***/
    
    putshort(use_choice_win);
    putshort(choice_font_id);
    putshort(choice_font_size);
    putshort(choice_font_bold);

    putshort(choice_cols);
    putshort(choice_rows);
    putshort(choice_size_wid);
    putshort(choice_size_hgt);
    
    p.h = choice_w->portRect.left;
    p.v = choice_w->portRect.top;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);

    p.h = choice_w->portRect.right;
    p.v = choice_w->portRect.bottom;
    LocalToGlobal(&p);
    putshort(p.h);
    putshort(p.v);


    /*** Restore the window ***/

    activate(old);
}


/*
 * Load the preferences from the current "file"
 */
static void load_prefs(void)
{
    int temp;
    
    /*** Later ***/
    
    temp = getshort();
    temp = getshort();
    temp = getshort();
    temp = getshort();


    /*** Folder info ***/

    /* get volume and directory hints for lib directory */
    volhint = getshort();
    dirhint = getlong();


    /*** Screen info ***/
    
    temp = getshort();
    screen_font_id = getshort();
    screen_font_size = getshort();
    screen_font_bold = getshort();

    screen_cols = getshort();
    screen_rows = getshort();
    screen_size_wid = getshort();
    screen_size_hgt = getshort();
    
    screen_r.left = getshort();
    screen_r.top = getshort();
    screen_r.right = getshort();
    screen_r.bottom = getshort();


    /*** Recall info ***/
    
    use_recall_win = getshort();
    recall_font_id = getshort();
    recall_font_size = getshort();
    recall_font_bold = getshort();

    recall_cols = getshort();
    recall_rows = getshort();
    recall_size_wid = getshort();
    recall_size_hgt = getshort();
    
    recall_r.left = getshort();
    recall_r.top = getshort();
    recall_r.right = getshort();
    recall_r.bottom = getshort();


    /*** Choice info ***/
    
    use_choice_win = getshort();
    choice_font_id = getshort();
    choice_font_size = getshort();
    choice_font_bold = getshort();

    choice_cols = getshort();
    choice_rows = getshort();
    choice_size_wid = getshort();
    choice_size_hgt = getshort();
    
    choice_r.left = getshort();
    choice_r.top = getshort();
    choice_r.right = getshort();
    choice_r.bottom = getshort();
    

    /*** Analyze that info ***/
        
    /* Get some window sizes */
    screen_getsize();
    recall_getsize();
    choice_getsize();
    
#if 0    
    /* Hack -- stretch the "screen" window */
    screen_r.right = screen_r.left + screen_size_wid;
    screen_r.bottom = screen_r.top + screen_size_hgt;
#endif

}


/*
 * Prepare a new set of preferences
 */
static void init_prefs(void)
{
    short fid;
    
    /* Default to 9pt Monaco */
    GetFNum("\pmonaco", &fid);
    choice_font_id = recall_font_id = screen_font_id = fid;
    choice_font_size = recall_font_size = screen_font_size = 9;
    choice_font_bold = recall_font_bold = screen_font_bold = 0;

    /* Extract "size" info from the font data */
    screen_getsize();
    recall_getsize();
    choice_getsize();

    /* Extract physical locations for the "screen" window */
    screen_r.left = 20;
    screen_r.top = 60;
    screen_r.right = screen_r.left + screen_size_wid;
    screen_r.bottom = screen_r.top + screen_size_hgt;

    /* Extract physical locations for the "recall" window */
    recall_r.left = 20;
    recall_r.top = 60 + screen_size_hgt + 40;
    if (recall_r.top > qd.screenBits.bounds.bottom - recall_font_hgt * recall_rows)
    {
	recall_r.top = qd.screenBits.bounds.bottom - recall_font_hgt * recall_rows;
    }
    recall_r.right = recall_r.left + screen_size_wid;
    recall_r.bottom = recall_r.top + screen_font_hgt * recall_rows;

    /* Extract physical locations for the "recall" window */
    choice_r.top = screen_r.top;
    choice_r.left = screen_r.right + 40;
    if (choice_r.left > qd.screenBits.bounds.right - 200)
    {
	choice_r.left = qd.screenBits.bounds.right - 200;
    }
    choice_r.right = choice_r.left + 200;
    choice_r.bottom = choice_r.top + choice_rows * choice_font_hgt;
}




/*
 * Read the preference file, Create the windows.
 */
static void init_windows()
{
    Rect r;
    RGBColor c;
    SysEnvRec env;
    short savev;
    long saved;
    PaletteHandle p;
    int i;

    /* Attempt to open a preference file */
    SysEnvirons(curSysEnvVers, &env);
    HGetVol(0,&savev,&saved);
    SetVol(0, env.sysVRefNum);
    fff = fopen(":Preferences:Angband 2.7.1 Preferences", "r");
    if (!fff) fff = fopen(":Angband 2.7.1 Preferences", "r");
    HSetVol(0, savev, saved);

    /* Parse it */
    if (fff)
    {
	/* Load a real preference file */
	load_prefs();

	/* Close the file */	
	fclose(fff);
    }
    else
    {
        /* Make a new set */
        init_prefs();
    }
    


    /*** Create the "Choice" window ***/
    
    if (rgbcolor)
    {
	choice_w = NewCWindow(0,&choice_r,"\pChoices",use_choice_win,documentProc,(WindowPtr)-1,1,0);
    }
    else
    {
	choice_w = NewWindow(0,&choice_r,"\pChoices",use_choice_win,documentProc,(WindowPtr)-1,1,0);
    }
    
    activate(choice_w);
	
    TextFont(choice_font_id);
    TextSize(choice_font_size);
    TextFace(choice_font_bold);
	
    if (rgbcolor==2)
    {
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
	p=GetNewPalette(128);
	NSetPalette(choice_w,p,pmAllUpdates);
	BackColor(blackColor);
	ForeColor(whiteColor);
    }
    else
    {
	BackColor(blackColor);
	ForeColor(whiteColor);
    }


    r = choice_w->portRect;
    r.left = r.right-15;
    r.right += 1;
    r.bottom -= 14;
    r.top -= 1;
    choice_sb = NewControl(choice_w, &r, "\p", 1, 0, 0, 21, scrollBarProc, 0L);
    
#if 0

    SetControlMaximum(choice_sb, choice_rows-(choice_w->portRect.bottom - choice_w->portRect.top-1)/choice_font_hgt);
    HiliteControl(choice_sb,255);
	
#endif



    /*** Create the recall window ***/

    if (rgbcolor)
    {
	recall_w=NewCWindow(0,&recall_r,"\pRecall",use_recall_win,documentProc,(WindowPtr)-1,1,0);
    }
    else
    {
	recall_w=NewWindow(0,&recall_r,"\pRecall",use_recall_win,documentProc,(WindowPtr)-1,1,0);
    }

    activate(recall_w);

    TextFont(recall_font_id);
    TextSize(recall_font_size);
    TextFace(recall_font_bold);


    if (rgbcolor==2)
    {
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
	p=GetNewPalette(128);
	NSetPalette(recall_w,p,pmAllUpdates);
	BackColor(blackColor);
	ForeColor(whiteColor);
    }
    else
    {
	BackColor(blackColor);
	ForeColor(whiteColor);
    }

    r = recall_w->portRect;
    r.left = r.right-15;
    r.right += 1;
    r.bottom -= 14;
    r.top -= 1;

    recall_sb = NewControl(recall_w, &r, "\p", 1, 0, 0, 29, scrollBarProc, 0L);


    /*** Create the Angband window ***/

    if (rgbcolor)
    {
	screen_w=NewCWindow(0,&screen_r,"\pAngband",true,noGrowDocProc,(WindowPtr)-1,0,0);
    }
    else
    {
	screen_w=NewWindow(0,&screen_r,"\pAngband",true,noGrowDocProc,(WindowPtr)-1,0,0);
    }

    activate(screen_w);

    TextFont(screen_font_id);
    TextSize(screen_font_size);
    TextFace(screen_font_bold);

    if (rgbcolor==2)
    {
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
	p=GetNewPalette(128);
	NSetPalette(screen_w,p,pmAllUpdates);
	BackColor(blackColor);
	ForeColor(whiteColor);
    }
    else
    {
	BackColor(blackColor);
	ForeColor(whiteColor);
    }
}


/*
 * Exit the program
 */
static void save_pref_file()
{
    SysEnvRec env;
    
    SysEnvirons(curSysEnvVers, &env);
    SetVol(0, env.sysVRefNum);

    /* Open the preference file */
    _ftype='TEXT';
    fff = fopen(":Preferences:Angband 2.7.1 Preferences","w");
    if (!fff) fff = fopen(":Angband 2.7.1 Preferences","w");
    _ftype='SAVE';

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
 *   File (129) = { New,Open,Save,-,Quit+Save,Exit }
 *   Edit (130) = { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) = { Bold, -, Monaco, Courier, ... }
 *   Size (132) = { 9, 10, ..., 18, ... }
 *   Options (133) = { Angband, -, Recall, Choices }
 */
static void setup_menus()
{
    MenuHandle m;
    int i;
    short fnum,fsize,currfsize;
    Str255 s;


    /* Hack -- extract the "can I save" flag */
    save_enabled = character_generated;


    /* File menu ("Save" options) */
    m=GetMHandle(129);

    /* Enable/Disable "save" options */
    if (save_enabled)
    {
	EnableItem(m,3);
	EnableItem(m,5);
    }
    else
    {
	DisableItem(m,3);
	DisableItem(m,5);
    }
    

    /* Font menu */
    m=GetMHandle(131);
    
    /* Check the appropriate "bold-ness" */
    if (FrontWindow()==screen_w)
    {
	CheckItem(m, 1, screen_font_bold ? 1 : 0);
    }
    if (FrontWindow()==recall_w)
    {
	CheckItem(m, 1, recall_font_bold ? 1 : 0);
    }
    if (FrontWindow()==choice_w)
    {
	CheckItem(m, 1, choice_font_bold ? 1 : 0);
    }

    /* Check the appropriate "Font" for the current window */
    for (i=3; i<=CountMItems(m); i++)
    {
	GetItem(m,i,s);
	GetFNum(s,&fnum);
	if (((fnum==screen_font_id) && (FrontWindow()==screen_w)) ||
	    ((fnum==choice_font_id) && (FrontWindow()==choice_w)) ||
	    ((fnum==recall_font_id) && (FrontWindow()==recall_w)))
	{
	    CheckItem(m,i,1);
	}
	else
	{
	    CheckItem(m,i,0);
	}
    }


    /* Size menu */
    m=GetMHandle(132);
    
    if (FrontWindow()==screen_w)
    {
	fnum = screen_font_id;
	currfsize = screen_font_size;
    }
    if (FrontWindow()==recall_w)
    {
	fnum = recall_font_id;
	currfsize = recall_font_size;
    }
    if (FrontWindow()==choice_w)
    {
	fnum = choice_font_id;
	currfsize = choice_font_size;
    }

    /* Scan something */
    for (i=1; i<=CountMItems(m); i++)
    {
	GetItem(m,i,s);
	s[s[0]+1]=0;
	fsize=atoi((char*)(s+1));
	if (RealFont(fnum,fsize))
	{
	    EnableItem(m,i);
	}
	else
	{
	    DisableItem(m,i);
	}
	if (fsize==currfsize)
	{
	    CheckItem(m,i,1);
	}
	else
	{
	    CheckItem(m,i,0);
	}
    }


    /* Options menu */
    m=GetMHandle(133);

    /* Item "Angband Window" -- Hack -- encode "to_be_wizard" */
    CheckItem(m, 1, to_be_wizard ? 1 : 0);

    /* Item "Recall Window" */
    CheckItem(m, 3, use_recall_win ? 1 : 0);

    /* Item "Choice Window" */
    CheckItem(m, 4, use_choice_win ? 1 : 0);
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
static void menu(long choice)
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
    int32 j;
    char *tmp;

    short vrefnum;
    long drefnum;
    long longjunk;
    DirInfo pb;

    /* Hack -- extract "can I save" flag */
    save_enabled = character_generated;
    
    menuid=HiWord(choice);
    selection=LoWord(choice);

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

	case 129:	/* File Menu */

	    switch (selection)
	    {
		case 1:		/* New*/
		    if (game_in_progress)
		    {
			mac_warning("You can't start a new game while you're still playing!");
		    }
		    else
		    {
			HiliteMenu(0);
			game_in_progress=1;
			disable_start();
			Term_flush();
			play_game_mac(TRUE);
			quit(NULL);
		    }
		    break;

		case 2:		/* Open... */

		    if (game_in_progress)
		    {
			mac_warning("You can't open a new game while you're still playing!");
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
			    Term_flush();
			    play_game_mac(FALSE);
			    quit(NULL);
			}

#if 0
			/*  This code is System 7 specific! */
			StandardGetFile(NULL,1,types,&reply);
			if (reply.sfGood)
			{
			    /* Load 'savefile' */
			    refnum_to_name(savefile,
					   reply.sfFile.parID,
					   reply.sfFile.vRefNum,
					   (char*)reply.sfFile.name);
			    game_in_progress=1;
			    disable_start();
			    HiliteMenu(0);
			    Term_flush();
			    play_game_mac(FALSE);
			    quit(NULL);
			}
#endif

		    }
		    break;

		case 3: /* Save */

		    if (game_in_progress)
		    {
			Term_fresh();

			if (total_winner)
			{
			    msg_print("You are a Total Winner, your character must be retired.");
			    if (rogue_like_commands)
			    {
				msg_print("Use 'Q' to when you are ready to retire.");
			    }
			    else
			    {
				msg_print("Use <Control>-K when you are ready to retire.");
			    }
			}
			else
			{
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
		    }
		    
		    break;


		case 5:		/* Quit (and save) */

		    if (game_in_progress && save_enabled)
		    {
			if (total_winner)
			{
			    msg_print("You are a Total Winner,        your character must be retired.");
			    if (rogue_like_commands)
				msg_print("Use 'Q' to when you are ready to retire.");
			    else
				msg_print("Use <Control>-K when you are ready to retire.");
			}
			else
			{
			    /* Save it */
			    (void)strcpy(died_from, "(saved)");
			    prt("Saving game...",0,0);
			    if (save_player()) quit(NULL);

			    /* Oops. */
			    prt("Save failed.",0,14);
			    move_cursor_relative(char_row,char_col);
			    (void)strcpy(died_from, "(alive and well)");
			}
		    }
		    else
		    {
			/* Nothing to save */
			quit(NULL);
		    }
		    break;

		case 6:		/* Exit */
		    if (game_in_progress && save_enabled)
		    {
			alert=(AlertTHndl)GetResource('ALRT',128);
			center_rect(&(*alert)->boundsRect,&qd.screenBits.bounds);
			item_hit=Alert(128,ynfilter);

			/* Did they say "Okay"? */
			if (item_hit != 2) break;
		    }
		    quit(NULL);
		    break;
	    }
	    break;

	case 130:  /* edit menu - nothing's enabled! */
	    break;

	case 131:  /* font menu */
	    if (selection==1)
	    {
		if (FrontWindow()==screen_w)
		{
		    screen_font_bold=1-screen_font_bold;
		    screen_getsize();

		    TextFace(screen_font_bold);
		    SizeWindow(screen_w,screen_size_wid,screen_size_hgt,0);
		    InvalRect(&screen_w->portRect);
		}

		if (FrontWindow()==recall_w)
		{
		    WindowPtr old = active;

		    recall_font_bold=1-recall_font_bold;
		    recall_getsize();

		    activate(recall_w);

		    TextFace(recall_font_bold);

		    recall_grow();
		    recall_draw();

		    activate(old);
		}
		break;
	    }
	    if (FrontWindow()==screen_w)
	    {
	        short fid;
	        
		GetItem(GetMHandle(131),selection,s);
		GetFNum(s,&fid);
		screen_font_id = fid;

		/* current size is bad for new font */
		if (!RealFont(screen_font_id,screen_font_size))
		{
		    /* find good size */
		    for(i=1;i<=18;i++)
		    {
			if (screen_font_size-i>=9)
			{
			    if (RealFont(screen_font_id,screen_font_size-i))
			    {
				screen_font_size-=i;
				break;
			    }
			}
			if (screen_font_size+i<=18)
			{
			    if (RealFont(screen_font_id,screen_font_size+i))
			    {
				screen_font_size+=i;
				break;
			    }

			}
		    }

		    /* default size if can't find a good size */
		    if (i==19) screen_font_size==9;
		}

		screen_getsize();

		TextFont(screen_font_id);
		TextSize(screen_font_size);
		TextFace(screen_font_bold);
		SizeWindow(screen_w,screen_size_wid,screen_size_hgt,0);
		InvalRect(&screen_w->portRect);
	    }
	    
	    if (FrontWindow()==recall_w)
	    {
	        short fid;
		WindowPtr old = active;

		GetItem(GetMHandle(131),selection,s);
		GetFNum(s,&fid);
		recall_font_id = fid;

		/* current size is bad for new font */
		if (!RealFont(recall_font_id,recall_font_size))
		{
		    /* find good size */
		    for (i=1;i<=18;i++)
		    {
			if (recall_font_size-i>=9)
			{
			    if (RealFont(recall_font_id,recall_font_size-i))
			    {
				recall_font_size-=i;
				break;
			    }
			}
			if (recall_font_size + i <= 18)
			{
			    if (RealFont(recall_font_id,recall_font_size+i))
			    {
				recall_font_size += i;
				break;
			    }
			}
		    }

		    /* default size if can't find a good size */
		    if (i==19) recall_font_size==9;
		}

		recall_getsize();

		activate(recall_w);

		TextFont(recall_font_id);
		TextSize(recall_font_size);
		TextFace(recall_font_bold);

		recall_grow();
		recall_draw();

		activate(old);
	    }

	    if (FrontWindow()==choice_w)
	    {
	        short fid;
		WindowPtr old = active;

		GetItem(GetMHandle(131),selection,s);
		GetFNum(s,&fid);
		choice_font_id = fid;

		/* current size is bad for new font */
		if (!RealFont(choice_font_id,choice_font_size))
		{
		    /* find good size */
		    for (i=1;i<=18;i++)
		    {
			if (choice_font_size-i>=9)
			{
			    if (RealFont(choice_font_id,choice_font_size-i))
			    {
				choice_font_size-=i;
				break;
			    }
			}
			if (choice_font_size + i <= 18)
			{
			    if (RealFont(choice_font_id,choice_font_size+i))
			    {
				choice_font_size += i;
				break;
			    }
			}
		    }

		    /* default size if can't find a good size */
		    if (i==19) choice_font_size==9;
		}

		choice_getsize();

		activate(choice_w);

		TextFont(choice_font_id);
		TextSize(choice_font_size);
		TextFace(choice_font_bold);

		choice_grow();
		choice_draw();

		activate(old);
	    }

	    break;



	case 132:  /* size menu */
	
	    if (FrontWindow()==screen_w)
	    {
		GetItem(GetMHandle(132),selection,s);
		s[s[0]+1]=0;
		screen_font_size = atoi((char*)(s+1));
		screen_getsize();

		TextSize(screen_font_size);
		SizeWindow(screen_w,screen_size_wid,screen_size_hgt,0);
		InvalRect(&screen_w->portRect);
	    }
	    
	    if (FrontWindow() == recall_w)
	    {
		WindowPtr old = active;

		GetItem(GetMHandle(132),selection,s);
		s[s[0]+1]=0;
		recall_font_size=atoi((char*)(s+1));

		recall_getsize();

		activate(recall_w);

		TextSize(recall_font_size);

		recall_grow();
		recall_draw();

		activate(old);
	    }

	    if (FrontWindow()==choice_w)
	    {
		WindowPtr old = active;

		GetItem(GetMHandle(132),selection,s);
		s[s[0]+1]=0;
		choice_font_size=atoi((char*)(s+1));
		
		choice_getsize();
		
		activate(choice_w);

		TextSize(choice_font_size);

		/* XXX */
		/* SetControlMaximum(choice_sb, choice_rows - (choice_w->portRect.bottom - choice_w->portRect.top-1)/choice_font_hgt); */

		choice_grow();
		choice_draw();

		activate(old);
	    }

	    break;

	case 133:   /* other menu */
	    switch (selection)
	    {
		case 1:		/* Angband -- Hack -- to_be_wizard */
		    to_be_wizard = 1 - to_be_wizard;
		    if (to_be_wizard) mac_warning("You shouldn't do that...");
		    break;

		case 3:		/* Recall window */
		    use_recall_win = (!use_recall_win);
		    recall_note();
		    break;

		case 4:		/* Choice window */
		    use_choice_win = (!use_choice_win);
		    choice_note();
		    break;
	    }
    }

    /* Clean the menu */
    HiliteMenu(0);
}



/*
 * Scroll bar processor (recall window)
 */
static pascal void recall_sb_proc (ControlHandle c, short code)
{
    int pageSize;
    int amount;
    int oldamt;

    if (code == 0) return;

    oldamt = amount = GetCtlValue(c);

    switch (code)
    {
	case inUpButton:
	    amount += -1;
	    break;
	case inDownButton:
	    amount += 1;
	    break;
	case inPageUp:
	    amount += -(recall_w->portRect.bottom / recall_font_hgt)+1;
	    break;
	case inPageDown:
	    amount += (recall_w->portRect.bottom / recall_font_hgt)-1;
	    break;
    }

    if (amount < 0) amount = 0;
    if (amount > 24) amount = 24;

#if 0
    if (amount > GetControlMaximum(c)) amount = GetControlMaximum(c);
#endif

    if (amount != oldamt)
    {
	SetCtlValue(c, amount);
	recall_draw();
    }

    /* XXX */
}



/*
 * Scroll bar processor (choice window)
 */
static pascal void choice_sb_proc (ControlHandle c, short code)
{
	int	pageSize;
	int	amount;
	int oldamt;
	
	if (code == 0) return;
	
	oldamt = amount = GetCtlValue(c);
	
	switch (code) {
		case inUpButton: 
			amount += -1;
			break;
		case inDownButton: 
			amount += 1;
			break;
		case inPageUp: 
			amount += -(choice_w->portRect.bottom / choice_font_hgt)+1;
			break;
		case inPageDown: 
			amount += (choice_w->portRect.bottom / choice_font_hgt)-1;
			break;
	}
	if (amount < 0) amount = 0;
	if (amount > 24) amount = 24;

#if 0	
	if (amount > GetControlMaximum(c)) amount = GetControlMaximum(c);
#endif

    if (amount != oldamt)
    {
	SetCtlValue(c, amount);
	choice_draw();
    }

    /* XXX */
}






/*
 * Check Events, return "Did something happen?"
 */
static bool CheckEvents(void)
{
    EventRecord event;
    WindowPtr w;
    char ch;
    Rect r,clipsave;
    ControlHandle c;
    int cntlcode;
    long newsize;

    /* Hack -- extract the "can I save" flag */
    save_enabled = character_generated;
    
    /* Let the "system" run */
    SystemTask();

    /* Get an event (or null) */
    GetNextEvent(everyEvent,&event);

    /* Nothing is ready */
    if (event.what == nullEvent) return (FALSE);

    /* Analyze the event */
    switch (event.what)
    {
	case activateEvt:

	    w = (WindowPtr)event.message;

	    if (w==recall_w)
	    {
		WindowPtr old = active;

		activate(recall_w);

		if (event.modifiers & activeFlag)
		{
		    HiliteControl(recall_sb, 0);
		}
		else
		{
		    HiliteControl(recall_sb, 255);
		}

		r = w->portRect;
		r.left = r.right-15;
		ClipRect(&r);
		DrawGrowIcon(recall_w);
		ClipRect(&w->portRect);

		activate(old);
	    }
	    
	    if (w==choice_w)
	    {
		WindowPtr old = active;

		activate(choice_w);

		if (event.modifiers & activeFlag)
		{
		    HiliteControl(choice_sb,0);
		}
		else
		{
		    HiliteControl(choice_sb,255);
		}

		r = w->portRect;
		r.left = r.right-15;
		ClipRect(&r);
		DrawGrowIcon(choice_w);
		ClipRect(&w->portRect);

		activate(old);
	    }
	    
	    break;

	case updateEvt:

		w = (WindowPtr)event.message;

		BeginUpdate(w);

		/* refresh window */
		if (w==screen_w) screen_draw();

		/* refresh recall */
		if (w==recall_w) recall_draw();

		/* refresh choice */
		if (w==choice_w) choice_draw();

		EndUpdate(w);

		break;

	case keyDown:
	case autoKey:
	
	    ch = event.message & charCodeMask;

	    if (event.modifiers & cmdKey)
	    {
		setup_menus();
		
		/* Hack -- allow easy exit if nothing to save */
		if (!save_enabled && (ch=='Q' || ch=='q')) ch = 'e';

		/* Run the Menu-Handler */
		menu(MenuKey(ch));

		/* Turn off the menus */
		HiliteMenu(0);
	    }
	    else
	    {
		/* Hide the mouse pointer */
		ObscureCursor();

		/* Enter --> Return */
		if (ch==3) ch=13;

		/* XXX XXX Mega-Hack -- Convert Arrow keys into directions */
		if (ch==28) ch = rogue_like_commands ? 'h' : '4';
		if (ch==29) ch = rogue_like_commands ? 'l' : '6';
		if (ch==30) ch = rogue_like_commands ? 'k' : '8';
		if (ch==31) ch = rogue_like_commands ? 'j' : '2';

		/* Queue the keypress */
		Term_keypress(ch);
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
			r=qd.screenBits.bounds;
			r.top+=20;
			InsetRect(&r,4,4);
			DragWindow(w,event.where,&r);
			break;

		case inGoAway:
			if (TrackGoAway(w,event.where))
			{
			    if (w == recall_w)
			    {
				use_recall_win = 0;
				recall_note();
			    }
			    if (w == choice_w)
			    {
				use_choice_win = 0;
				choice_note();
			    }
			}
			break;

		case inGrow:

		    if (w == recall_w)
		    {
			WindowPtr old = active;

			activate(w);

			r.left=20*screen_font_wid;
			r.right=200*screen_font_wid;
			r.top=screen_font_hgt;
			r.bottom=30*screen_font_hgt;
			newsize=GrowWindow(w,event.where,&r);

			if (!newsize)
			{
			    activate(old);
			    break;
			}

			SizeWindow(w,LoWord(newsize),HiWord(newsize),1);
			ClipRect(&w->portRect);

			/* move scroll bar to correct location */
			r=w->portRect;
			r.right++;
			r.left=r.right-16;
			r.top--;
			r.bottom-=14;
			(*recall_sb)->contrlRect=r;

			EraseRect(&w->portRect);
			DrawControls(recall_w);
			r=w->portRect;
			r.left=r.right-15;
			ClipRect(&r);
			DrawGrowIcon(recall_w);
			ClipRect(&w->portRect);

			/* Note new size */
			recall_grow();

			/* Redraw Contents */
			recall_draw();

			/* Reset the port */			
			activate(w);

			/* we've drawn it all -- no need to redraw */
			ValidRect(&w->portRect);

			/* Reset the port */
			activate(old);
		    }

		    if (w == choice_w)
		    {
			WindowPtr old = active;

			activate(w);

			r.left=20*screen_font_wid;
			r.right=200*screen_font_wid;
			r.top=screen_font_hgt;
			r.bottom=30*screen_font_hgt;
			newsize=GrowWindow(w,event.where,&r);

			if (!newsize)
			{
			    activate(old);
			    break;
			}

			SizeWindow(w,LoWord(newsize),HiWord(newsize),1);
			ClipRect(&w->portRect);

			/* move scroll bar to correct location */
			r=w->portRect;
			r.right++;
			r.left=r.right-16;
			r.top--;
			r.bottom-=14;
			(*choice_sb)->contrlRect=r;

			EraseRect(&w->portRect);
			DrawControls(choice_w);
			r=w->portRect;
			r.left=r.right-15;
			ClipRect(&r);
			DrawGrowIcon(choice_w);
			ClipRect(&w->portRect);

			/* Note new size */
			choice_grow();

			/* Redraw Contents */
			choice_draw();

			/* Reset the port */			
			activate(w);

			/* we've drawn it all -- no need to redraw */
			ValidRect(&w->portRect);

			/* Reset the port */
			activate(old);
		    }

		    break;

		case inContent:

			if (w == screen_w)
			{
			    WindowPtr old = active;

			    activate(w);

			    GlobalToLocal(&event.where);
			    cntlcode=FindControl(event.where, w, &c);
			    SelectWindow(w);

			    activate(old);
			}

			if (w == recall_w)
			{
			    WindowPtr old = active;

			    activate(w);

			    GlobalToLocal(&event.where);
			    cntlcode=FindControl(event.where, w, &c);
			    if (cntlcode==0)
			    {
				SelectWindow(w);
			    }
			    else if (cntlcode==inThumb)
			    {
				TrackControl(c, event.where, 0L);
				recall_draw();
			    }
			    else
			    {
				TrackControl(c, event.where, ((void *)recall_sb_proc));
			    }

			    activate(old);
			}

			if (w == choice_w)
			{
			    WindowPtr old = active;

			    activate(w);

			    GlobalToLocal(&event.where);
			    cntlcode=FindControl(event.where, w, &c);
			    if (cntlcode==0)
			    {
				SelectWindow(w);
			    }
			    else if (cntlcode==inThumb)
			    {
				TrackControl(c, event.where, 0L);
				choice_draw();
			    }
			    else
			    {
				TrackControl(c, event.where, ((void *)choice_sb_proc));
			    }

			    activate(old);
			}

			break;
		}
		break;
    }

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
	    Term_flush();

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




/*** Hooks for the "recall" and "choice" windows ***/


/*
 * Clear the recall window
 */
static void recall_clear_mac(void)
{
    int i,j;
    Rect r;

    WindowPtr old = active;

    /* Activate the recall window */
    activate(recall_w);

    /* Actually erase it (and reset the scroll bar) */
    if (!scrolling) SetCtlValue(recall_sb,0);

    /* Erase the recall window (but not the scroll bar) */
    r = recall_w->portRect;
    r.right -= 15;
    EraseRect(&r);

    /* Re-activate the main window */
    activate(old);
}


/*
 * Print something in the recall window
 */
static void recall_putstr_mac(int x, int y, int n, byte a, cptr s)
{
    Rect r;
    int i, start;

    WindowPtr old = active;

    /* Assume no more than 24 rows used */
    if (y >= 24) return;

    /* Activate the recall window */
    activate(recall_w);

    /* Set the color */
    setcolor(a);

    /* Get the current scroll-bar setting */
    start = GetCtlValue(recall_sb);

    /* Get the length */
    if (n < 0) n = strlen(s);
    
    /* Draw the text */
    MoveTo(recall_font_wid * x + recall_font_o_x + 4,
	   recall_font_hgt * (y-start) + recall_font_o_y);
    DrawText(s,0,n);

    /* Re-Activate the main window */
    activate(old);
}




#ifdef GRAPHIC_CHOICE

/*
 * Clear the choice window
 */
static void choice_clear_mac(void)
{
    int i,j;
    Rect r;

    WindowPtr old = active;

    /* Activate the choice window */
    activate(choice_w);

    /* Actually erase it (and reset the scroll bar) */
    if (!scrolling) SetCtlValue(choice_sb,0);

    /* Erase the choice window (but not the scroll bar) */
    r = choice_w->portRect;
    r.right -= 15;
    EraseRect(&r);

    /* Re-activate the main window */
    activate(old);
}


/*
 * Print something in the choice window
 */
static void choice_putstr_mac(int x, int y, int n, byte a, cptr s)
{
    Rect r;
    int i,len;
    int start;

    WindowPtr old = active;

    /* Assume no more than 24 rows used */
    if (y >= 24) return;

    /* Activate the choice window */
    activate(choice_w);

    /* Set the color */
    setcolor(a);

    /* Get the current scroll-bar setting */
    start = GetCtlValue(choice_sb);

    /* Get the length */
    if (n < 0) n = strlen(s);
    
    /* Draw the text */
    MoveTo(choice_font_wid * x + choice_font_o_x + 4,
	   choice_font_hgt * (y-start) + choice_font_o_y);
    DrawText(s,0,n);

    /* Re-Activate the main window */
    activate(old);
}


#endif



/*** Function hooks needed by "Term" ***/


/*
 * Low level graphics.  Assumes valid input.
 * Draw several ("n") chars, with an attr, at a given location.
 */
static void Term_text_mac(int x, int y, int n, byte a, cptr s)
{
    /* Hack -- Stop illegal chars */
    if ((a == 0) || (a >= 16)) a = COLOR_RED;

    /* Erase behind the characters */
    Term_wipe(x, y, n, 1);

    /* Activate the color */
    setcolor(a);

    /* Move to the correct location */
    MoveTo(x * screen_font_wid + screen_font_o_x, y * screen_font_hgt + screen_font_o_y);

    /* Sometimes Just draw a single character */
    if (n == 1) DrawChar(s[0]);

    /* Draw the string (will only work for mono-spaced fonts) */
    else DrawText(s, 0, n);
}



/*
 * Low level graphics (Assumes valid input)
 *
 * Erase a "block" of characters starting at (x,y), with size (w,h)
 */
static void Term_wipe_mac(int x, int y, int w, int h)
{
    Rect r;

    /* Erase the character grid */
    r.left = x * screen_font_wid;
    r.right = r.left + w * screen_font_wid;
    r.top = y * screen_font_hgt;
    r.bottom = r.top + h * screen_font_hgt;
    EraseRect(&r);
}



/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_grab()" to determine
 * the current screen contents (for inverting, etc).
 */
static void Term_curs_mac(int x, int y, int z)
{
    Rect r;

    /* Cursor is done as a yellow "box" */
    setcolor(COLOR_YELLOW);

    /* Paranoia -- verify the cursor */
    if ((x < 0) || (y < 0) || (x >= screen_cols) || (y >= screen_rows))
    {
	x = 0, y = 0, setcolor(COLOR_RED);
    }

    /* Frame the grid */
    r.left = x * screen_font_wid;
    r.right = r.left + screen_font_wid;
    r.top = y * screen_font_hgt;
    r.bottom = r.top + screen_font_hgt;
    FrameRect(&r);
}



/*
 * Scan for events
 *
 * Hack -- react to "events" like "user setting options that should
 * make us change the visibility of certain windows".
 */
static void Term_scan_mac(int n)
{
    if (n < 0)
    {
	/* Scan until no events left */
	while (CheckEvents());
    }

    else
    {
	/* Scan until 'n' events have been handled */
	while (n > 0) if (CheckEvents()) n--;
    }

    /* Hack -- check windows */
    recall_note();
    choice_note();
}


/*
 * Do a "special thing"
 */
static void Term_xtra_mac(int n)
{
    /* Handle a sub-set of the legal requests */
    switch (n)
    {
	/* Handle "beep" requests */
	case TERM_XTRA_NOISE: SysBeep(1); break;
    }
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
    /* Mac-Hack -- about 100K of extra stack */
    SetApplLimit(GetApplLimit()-100000L);
    MaxApplZone();

    /* Set up the Macintosh */
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    InitDialogs(0);
    InitCursor();


    /* Mark ourself as the file creator */
    _fcreator='A271';

    /* Default to saving a "save" file */
    _ftype='SAVE';


    /* Hook in some "z-virt.c" hooks */
    ralloc_aux = hook_ralloc;
    rnfree_aux = hook_rnfree;

    /* Hooks in some "z-util.c" hooks */
    plog_aux = hook_plog;
    quit_aux = hook_quit;
    core_aux = hook_core;


    /* Hook in some hooks for "term.c" */
    Term_text_hook = Term_text_mac;
    Term_wipe_hook = Term_wipe_mac;
    Term_curs_hook = Term_curs_mac;
    Term_scan_hook = Term_scan_mac;
    Term_xtra_hook = Term_xtra_mac;


#ifdef GRAPHIC_RECALL
    /* Hook in some hooks for "recall.c" */
    recall_clear_hook = recall_clear_mac;
    recall_putstr_hook = recall_putstr_mac;
    use_recall_win = 1;
#endif

#ifdef GRAPHIC_CHOICE
    /* Hook in some hooks for the "choice" window */
    choice_clear_hook = choice_clear_mac;
    choice_putstr_hook = choice_putstr_mac;
    use_choice_win = 1;
#endif


    /* Prepare the "colors" */
    init_colors();

    /* Prepare the menubar */
    init_menubar();

    /* Prepare the windows */
    init_windows();


    /* Init the Screen itself */
    screen_init();

    /* Init the Recall window */
    recall_init();

    /* Init the Choice window */
    choice_init();


    /* Hack -- Set some "methods" for "Term" */
    Term_method(TERM_SOFT_CURSOR, TRUE);
    Term_method(TERM_SCAN_EVENTS, TRUE);


    /* Prepare the "Term" */
    Term_init();


    /* Prepare the filepaths */
    get_file_paths();

    /* Be sure that we are in the right directory */
    mac_check_lib_folder();

    /* Get a pointer to the high score file */
    init_scorefile();

    /* Display the "news" message */
    show_news();

    /* Allocate and Initialize various arrays */
    prt("[Initializing arrays...]", 23, 30);
    Term_fresh();
    init_some_arrays();
    prt("[Initializing arrays... done]", 23, 30);
    Term_fresh();

    /* Did the user double click on a save file? */
    check_for_save_file();

    /* Prompt the user */
    prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);
    Term_fresh();

    /* Process Events (until "play_game_mac()" is called) */
    while (1) CheckEvents();
}






#endif /* MACINTOSH */

