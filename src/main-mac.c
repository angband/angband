/* File: main-mac.c */

/* Purpose: Simple support for MACINTOSH Angband */

/*
 * This file should only be compiled with the "Macintosh" version
 *
 * This file written by "Ben Harrison (benh@voicenet.com)".
 *
 * Some code adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * Maarten Hazewinkel (mmhazewi@cs.ruu.nl) provided some initial
 * suggestions for the PowerMac port.
 *
 * Steve Linberg (slinberg@crocker.com) provided the code surrounded
 * by "USE_SFL_CODE".
 *
 * The graphics code is adapted from an extremely minimal subset of
 * the code from "Sprite World II", an amazing animation package.
 *
 * See "z-term.c" for info on the concept of the "generic terminal"
 *
 * The preference file is now a text file named "Angband preferences".
 *
 * Note that the "preference" file is now a simple text file called
 * "Angband preferences", which contains the versions information, so
 * that obsolete preference files can be ignored (this may be bad).
 *
 * Note that "init1.c", "init2.c", "load1.c", "load2.c", and "birth.c"
 * should probably be "unloaded" as soon as they are no longer needed,
 * to save space, but I do not know how to do this.
 *
 * Stange bug -- The first "ClipRect()" call crashes if the user closes
 * all the windows, switches to another application, switches back, and
 * then re-opens the main window, for example, using "command-a".
 *
 * By default, this file assumes that you will be using a 68020 or better
 * machine, running System 7 and Color Quickdraw.  In fact, the game will
 * refuse to run unless these features are available.  This allows the use
 * of a variety of interesting features such as graphics and sound.
 *
 * To create a version which can be used on 68000 machines, or on machines
 * which are not running System 7 or Color Quickdraw, simply activate the
 * "ANGBAND_LITE_MAC" compilation flag in the proper header file.  This
 * will disable all "modern" features used in this file, including support
 * for multiple sub-windows, color, graphics, and sound.
 *
 * When compiling with the "ANGBAND_LITE_MAC" flag, the "ANGBAND_LITE"
 * flag will be automatically defined, which will disable many of the
 * advanced features of the game itself, reducing the total memory usage.
 *
 * If you are never going to use "graphics" (especially if you are not
 * compiling support for graphics anyway) then you can delete the "pict"
 * resource with id "1001" with no dangerous side effects.
 */


/*
 * Important Resources in the resource file:
 *
 *   FREF 130 = 'A271' / 'APPL' (application)
 *   FREF 129 = 'A271' / 'SAVE' (save file)
 *   FREF 130 = 'A271' / 'TEXT' (bone file, generic text file)
 *   FREF 131 = 'A271' / 'DATA' (binary image file, score file)
 *
 *   DLOG 128 = "About Angband..."
 *
 *   ALRT 128 = unused (?)
 *   ALRT 129 = "Warning..."
 *   ALRT 130 = "Are you sure you want to quit without saving?"
 *
 *   DITL 128 = body for DLOG 128
 *   DITL 129 = body for ALRT 129
 *   DITL 130 = body for ALRT 130
 *
 *   ICON 128 = "warning" icon
 *
 *   MENU 128 = apple (about, -, ...)
 *   MENU 129 = File (new, open, close, save, -, exit, quit)
 *   MENU 130 = Edit (undo, -, cut, copy, paste, clear)
 *
 *   PICT 1001 = Graphics tile set
 */


/*
 * File name patterns:
 *   all 'APEX' files have a filename of the form "*:apex:*" (?)
 *   all 'BONE' files have a filename of the form "*:bone:*" (?)
 *   all 'DATA' files have a filename of the form "*:data:*"
 *   all 'SAVE' files have a filename of the form "*:save:*"
 *   all 'USER' files have a filename of the form "*:user:*" (?)
 *
 * Perhaps we should attempt to set the "_ftype" flag inside this file,
 * to avoid nasty file type information being spread all through the
 * rest of the code.  (?)  This might require adding hooks into the
 * "fd_open()" and "my_fopen()" functions in "util.c".  XXX XXX XXX
 */


/*
 * Reasons for each header file:
 *
 *   angband.h = Angband header file
 *
 *   Types.h = (included anyway)
 *   Gestalt.h = gestalt code
 *   QuickDraw.h = (included anyway)
 *   OSUtils.h = (included anyway)
 *   Files.h = file code
 *   Fonts.h = font code
 *   Menus.h = menu code
 *   Dialogs.h = dialog code
 *   Windows.h = (included anyway)
 *   Palettes.h = palette code
 *   StandardFile.h = file dialog box
 *   DiskInit.h = disk initialization
 *   ToolUtils.h = HiWord() / LoWord()
 *   Desk.h = OpenDeskAcc()
 *   Devices.h = OpenDeskAcc()
 *   Events.h = event code
 *   Resources.h = resource code
 *   Controls.h = button code
 *   SegLoad.h = ExitToShell(), AppFile, etc
 *   Memory.h = SetApplLimit(), NewPtr(), etc
 *   QDOffscreen.h = GWorld code
 *   Sound.h = Sound code
 *
 * For backwards compatibility:
 *   Use GestaltEqu.h instead of Gestalt.h
 *   Add Desk.h to include simply includes Menus.h, Devices.h, Events.h
 */


#include "angband.h"

#include <Types.h>
#include <Gestalt.h>
#include <QuickDraw.h>
#include <Files.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <Windows.h>
#include <Palettes.h>
#include <StandardFile.h>
#include <DiskInit.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Events.h>
#include <Resources.h>
#include <Controls.h>
#include <SegLoad.h>
#include <Memory.h>
#include <QDOffscreen.h>
#include <Sound.h>


/*
 * Use "malloc()" instead of "NewPtr()"
 */
/* #define USE_MALLOC */


#if defined(powerc) || defined(__powerc)

/*
 * Disable "LITE" version
 */
# undef ANGBAND_LITE_MAC

#endif


#ifdef ANGBAND_LITE_MAC

/*
 * Maximum number of windows
 */
# define MAX_TERM_DATA 1

#else /* ANGBAND_LITE_MAC */

/*
 * Maximum number of windows
 */
# define MAX_TERM_DATA 8

/*
 * Activate some special code
 */
# define USE_SFL_CODE

#endif /* ANGBAND_LITE_MAC */



#ifdef USE_SFL_CODE

/*
 * Include the necessary header files
 */
#include <AppleEvents.h>
#include <EPPC.h>
#include <Folders.h>

#endif


#if 0

/*
 * The Angband Color Set (0 to 15):
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 *
 * On the Macintosh, we use color quickdraw, and we use actual "RGB"
 * values below to choose the 16 colors.
 *
 * If we are compiled for ancient machines, we bypass color and simply
 * draw everything in white (letting "z-term.c" automatically convert
 * "black" into "wipe" calls).
 */
static RGBColor foo[16] =
{
	{0x0000, 0x0000, 0x0000},	/* TERM_DARK */
	{0xFFFF, 0xFFFF, 0xFFFF},	/* TERM_WHITE */
	{0x8080, 0x8080, 0x8080},	/* TERM_SLATE */
	{0xFFFF, 0x8080, 0x0000},	/* TERM_ORANGE */
	{0xC0C0, 0x0000, 0x0000},	/* TERM_RED */
	{0x0000, 0x8080, 0x4040},	/* TERM_GREEN */
	{0x0000, 0x0000, 0xFFFF},	/* TERM_BLUE */
	{0x8080, 0x4040, 0x0000},	/* TERM_UMBER */
	{0x4040, 0x4040, 0x4040},	/* TERM_L_DARK */
	{0xC0C0, 0xC0C0, 0xC0C0},	/* TERM_L_WHITE */
	{0xFFFF, 0x0000, 0xFFFF},	/* TERM_VIOLET */
	{0xFFFF, 0xFFFF, 0x0000},	/* TERM_YELLOW */
	{0xFFFF, 0x0000, 0x0000},	/* TERM_L_RED */
	{0x0000, 0xFFFF, 0x0000},	/* TERM_L_GREEN */
	{0x0000, 0xFFFF, 0xFFFF},	/* TERM_L_BLUE */
	{0xC0C0, 0x8080, 0x4040}	/* TERM_L_UMBER */
};

#endif


/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * Extra "term" data
 */
struct term_data
{
	term		*t;

	Rect		r;

	WindowPtr	w;

#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

	short padding;

	short pixelDepth;

	GWorldPtr theGWorld;

	GDHandle theGDH;

	GDHandle mainSWGDH;

#endif /* ANGBAND_LITE_MAC */

	Str15		title;

	s16b		oops;

	s16b		keys;

	s16b		last;

	s16b		mapped;

	s16b		rows;
	s16b		cols;

	s16b		font_id;
	s16b		font_size;
	s16b		font_face;
	s16b		font_mono;

	s16b		font_o_x;
	s16b		font_o_y;
	s16b		font_wid;
	s16b		font_hgt;

	s16b		tile_o_x;
	s16b		tile_o_y;
	s16b		tile_wid;
	s16b		tile_hgt;

	s16b		size_wid;
	s16b		size_hgt;

	s16b		size_ow1;
	s16b		size_oh1;
	s16b		size_ow2;
	s16b		size_oh2;
};




/*
 * Forward declare -- see below
 */
static bool CheckEvents(bool wait);


/*
 * Hack -- location of the main directory
 */
static short app_vol;
static long  app_dir;


/*
 * Delay handling of double-clicked savefiles
 */
Boolean open_when_ready = FALSE;

/*
 * Delay handling of pre-emptive "quit" event
 */
Boolean quit_when_ready = FALSE;


/*
 * Hack -- game in progress
 */
static int game_in_progress = 0;


/*
 * Only do "SetPort()" when needed
 */
static WindowPtr active = NULL;



/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];



/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;



/*
 * CodeWarrior uses Universal Procedure Pointers
 */
static ModalFilterUPP ynfilterUPP;

#ifdef USE_SFL_CODE

/*
 * Apple Event Hooks
 */
AEEventHandlerUPP AEH_Start_UPP;
AEEventHandlerUPP AEH_Quit_UPP;
AEEventHandlerUPP AEH_Print_UPP;
AEEventHandlerUPP AEH_Open_UPP;

#endif





/*
 * Convert refnum+vrefnum+fname into a full file name
 * Store this filename in 'buf' (make sure it is long enough)
 * Note that 'fname' looks to be a "pascal" string
 */
static void refnum_to_name(char *buf, long refnum, short vrefnum, char *fname)
{
	DirInfo pb;
	Str255 name;
	int err;
	int i, j;

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
		err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);
		res[i] = ':'; i--;
		for (j=1; j<=name[0]; j++)
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


#if 0

/*
 * XXX XXX XXX Allow the system to ask us for a filename
 */
static bool askfor_file(char *buf, int len)
{
	SFReply reply;
	Str255 dflt;
	Point topleft;
	short vrefnum;
	long drefnum, junk;

	/* Default file name */
	sprintf((char*)dflt + 1, "%s's description", buf);
	dflt[0] = strlen((char*)dflt + 1);

	/* Ask for a file name */
	topleft.h=(qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
	topleft.v=(2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-188/2;
	SFPutFile(topleft, "\pSelect a filename:", dflt, NULL, &reply);
	/* StandardPutFile("\pSelect a filename:", dflt, &reply); */

	/* Process */
	if (reply.good)
	{
		int fc;

		/* Get info */
		GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);

		/* Extract the name */
		refnum_to_name(buf, drefnum, vrefnum, (char*)reply.fName);

		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
}

#endif



/*
 * Center a rectangle inside another rectangle
 */
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
 * Convert a pascal string in place
 *
 * This function may be defined elsewhere, but since it is so
 * small, it is not worth finding the proper function name for
 * all the different platforms.
 */
static void ptocstr(StringPtr src)
{
	int i;

	/* Hack -- pointer */
	char *s = (char*)(src);

	/* Hack -- convert the string */
	for (i = s[0]; i; i--, s++) s[0] = s[1];

	/* Hack -- terminate the string */
	s[0] = '\0';
}


#if defined(USE_SFL_CODE)


/*
 * The following three routines (pstrcat, pstrinsert, and PathNameFromDirID)
 * were taken from the Think Reference section called "Getting a Full Pathname"
 * (under the File Manager section).  We need PathNameFromDirID to get the
 * full pathname of the opened savefile, making no assumptions about where it
 * is.
 *
 * I had to hack PathNameFromDirID a little for MetroWerks, but it's awfully
 * nice.
 */
static void pstrcat(StringPtr dst, StringPtr src)
{
	/* copy string in */
	BlockMove(src + 1, dst + *dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}

/*
 * pstrinsert - insert string 'src' at beginning of string 'dst'
 */
static void pstrinsert(StringPtr dst, StringPtr src)
{
	/* make room for new string */
	BlockMove(dst + 1, dst + *src + 1, *dst);

	/* copy new string in */
	BlockMove(src + 1, dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}

static void PathNameFromDirID(long dirID, short vRefNum, StringPtr fullPathName)
{
	CInfoPBRec	block;
	Str255	directoryName;
	OSErr	err;

	fullPathName[0] = '\0';

	block.dirInfo.ioDrParID = dirID;
	block.dirInfo.ioNamePtr = directoryName;

	while (1)
	{
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		err = PBGetCatInfo(&block, FALSE);
		pstrcat(directoryName, (StringPtr)"\p:");
		pstrinsert(fullPathName, directoryName);
		if (block.dirInfo.ioDrDirID == 2) break;
	}
}

#endif



/*
 * Activate a given window, if necessary
 */
static void activate(WindowPtr w)
{
	/* Activate */
	if (active != w)
	{
		/* Activate */
		if (w) SetPort(w);

		/* Remember */
		active = w;
	}
}


/*
 * Display a warning message
 */
static void mac_warning(cptr warning)
{
	Str255 text;
	int len, i;

	/* Limit of 250 chars */
	len = strlen(warning);
	if (len > 250) len = 250;

	/* Make a "Pascal" string */
	text[0] = len;
	for (i=0; i<len; i++) text[i+1] = warning[i];

	/* Prepare the dialog box values */
	ParamText(text, "\p", "\p", "\p");

	/* Display the Alert, wait for Okay */
	Alert(129, 0L);
}



/*** Some generic functions ***/


#ifdef ANGBAND_LITE_MAC

/*
 * Hack -- activate a color (0 to 255)
 */
#define term_data_color(TD,A) /* Nothing */

#else /* ANGBAND_LITE_MAC */

/*
 * Hack -- activate a color (0 to 255)
 */
static void term_data_color(term_data *td, int a)
{
	/* Activate the color */
	if (td->last != a)
	{
		u16b rv, gv, bv;

		RGBColor color;

		/* Extract the R,G,B data */
		rv = angband_color_table[a][1];
		gv = angband_color_table[a][2];
		bv = angband_color_table[a][3];

		/* Set the color */
		color.red = (rv | (rv << 8));
		color.green = (gv | (gv << 8));
		color.blue = (bv | (bv << 8));
	
		/* Activate the color */
		RGBForeColor(&color);

		/* Memorize color */
		td->last = a;
	}
}

#endif /* ANGBAND_LITE_MAC */


/*
 * Hack -- Apply and Verify the "font" info
 *
 * This should usually be followed by "term_data_check_size()"
 */
static void term_data_check_font(term_data *td)
{
	int i;

	FontInfo info;

	WindowPtr old = active;


	/* Activate */
	activate(td->w);

	/* Instantiate font */
	TextFont(td->font_id);
	TextSize(td->font_size);
	TextFace(td->font_face);

	/* Extract the font info */
	GetFontInfo(&info);

	/* Assume monospaced */
	td->font_mono = TRUE;

	/* Extract the font sizing values XXX XXX XXX */
	td->font_wid = CharWidth('@'); /* info.widMax; */
	td->font_hgt = info.ascent + info.descent;
	td->font_o_x = 0;
	td->font_o_y = info.ascent;

	/* Check important characters */
	for (i = 33; i < 127; i++)
	{
		/* Hack -- notice non-mono-space */
		if (td->font_wid != CharWidth(i)) td->font_mono = FALSE;

		/* Hack -- collect largest width */
		if (td->font_wid < CharWidth(i)) td->font_wid = CharWidth(i);
	}

	/* Set default offsets */
	td->tile_o_x = td->font_o_x;
	td->tile_o_y = td->font_o_y;

	/* Set default tile size */
	td->tile_wid = td->font_wid;
	td->tile_hgt = td->font_hgt;

	/* Re-activate the old window */
	activate(old);
}


/*
 * Hack -- Apply and Verify the "size" info
 */
static void term_data_check_size(term_data *td)
{
	/* Minimal window size */
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

	/* Minimal tile size */
	if (td->tile_wid < 4) td->tile_wid = 4;
	if (td->tile_hgt < 4) td->tile_hgt = 4;

	/* Default tile offsets */
	td->tile_o_x = (td->tile_wid - td->font_wid) / 2;
	td->tile_o_y = (td->tile_hgt - td->font_hgt) / 2;

	/* Minimal tile offsets */
	if (td->tile_o_x < 0) td->tile_o_x = 0;
	if (td->tile_o_y < 0) td->tile_o_y = 0;

	/* Apply font offsets */
	td->tile_o_x += td->font_o_x;
	td->tile_o_y += td->font_o_y;

	/* Calculate full window size */
	td->size_wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	td->size_hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

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

	/* Assume no graphics */
	td->t->always_pict = FALSE;

#ifdef ANGBAND_LITE_MAC

	/* No graphics */

#else /* ANGBAND_LITE_MAC */

	/* Handle graphics */
	if (use_graphics && (td == &data[0]))
	{
		td->t->always_pict = TRUE;
	}

#endif /* ANGBAND_LITE_MAC */

	/* Fake mono-space */
	if (!td->font_mono ||
	    (td->font_wid != td->tile_wid) ||
	    (td->font_hgt != td->tile_hgt))
	{
		/* Handle fake monospace */
		td->t->always_pict = TRUE;
	}
}


/*
 * Hack -- resize a term_data
 *
 * This should normally be followed by "term_data_resize()"
 */
static void term_data_resize(term_data *td)
{
	/* Actually resize the window */
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);
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

	/* Restore the old term */
	Term_activate(old);

	/* No need to redraw */
	ValidRect(&td->w->portRect);
}




#ifdef ANGBAND_LITE_MAC

/* No graphics */

#else /* ANGBAND_LITE_MAC */


/*
 * Constants
 */

#define kPictID					1001			/* Graf 'pict' resource */

#define kGrafWidth				8				/* Graf Size (X) */
#define kGrafHeight				8				/* Graf Size (Y) */

#define kPictCols				32				/* Number of Cols in Pict */
#define kPictRows				32				/* Number of Rows in Pict */


/*
 * Forward Declare
 */
typedef struct FrameRec FrameRec;

/*
 * Frame
 *
 *	- GWorld for the frame image
 *	- Handle to pix map (saved for unlocking/locking)
 *	- Pointer to color pix map (valid only while locked)
 */
struct FrameRec
{
	GWorldPtr 		framePort;
	PixMapHandle 	framePixHndl;
	PixMapPtr 		framePix;
};


/*
 * The global picture data
 */
static FrameRec *frameP = NULL;


/*
 * Lock a frame
 */
static void BenSWLockFrame(FrameRec *srcFrameP)
{
	PixMapHandle 		pixMapH;

	pixMapH = GetGWorldPixMap(srcFrameP->framePort);
	(void)LockPixels(pixMapH);
	HLockHi((Handle)pixMapH);
	srcFrameP->framePixHndl = pixMapH;
	srcFrameP->framePix = (PixMapPtr)StripAddress(*(Handle)pixMapH);
}


/*
 * Unlock a frame
 */
static void BenSWUnlockFrame(FrameRec *srcFrameP)
{
	if (srcFrameP->framePort != NULL)
	{
		HUnlock((Handle)srcFrameP->framePixHndl);
		UnlockPixels(srcFrameP->framePixHndl);
	}

	srcFrameP->framePix = NULL;
}



static OSErr BenSWCreateGWorldFromPict(
	GWorldPtr *pictGWorld,
	PicHandle pictH)
{
	OSErr err;
	GWorldPtr saveGWorld;
	GDHandle saveGDevice;
	GWorldPtr tempGWorld;
	Rect pictRect;
	short depth;
	GDHandle theGDH;

	/* Reset */
	*pictGWorld = NULL;

	/* Get depth */
	depth = data[0].pixelDepth;

	/* Get GDH */
	theGDH = data[0].theGDH;

	/* Obtain size rectangle */
	pictRect = (**pictH).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);

	/* Create a GWorld */
	err = NewGWorld(&tempGWorld, depth, &pictRect, nil, 
					theGDH, noNewDevice);

	/* Success */
	if (err != noErr)
	{
		return (err);
	}

	/* Save pointer */
	*pictGWorld = tempGWorld;

	/* Save GWorld */
	GetGWorld(&saveGWorld, &saveGDevice);

	/* Activate */
	SetGWorld(tempGWorld, nil);

	/* Dump the pict into the GWorld */
	(void)LockPixels(GetGWorldPixMap(tempGWorld));
	EraseRect(&pictRect);
	DrawPicture(pictH, &pictRect);
	UnlockPixels(GetGWorldPixMap(tempGWorld));

	/* Restore GWorld */
	SetGWorld(saveGWorld, saveGDevice);
	
	/* Success */
	return (0);
}


/*
 * Init the global "frameP"
 */
static errr globe_init(void)
{
	OSErr err;
	
	GWorldPtr tempPictGWorldP;

	PicHandle newPictH;


	/* Use window XXX XXX XXX */
	SetPort(data[0].w);


	/* Get the pict resource */
	newPictH = GetPicture(kPictID);

	/* Analyze result */
	err = (newPictH ? 0 : -1);

	/* Oops */
	if (err == noErr)
	{
		/* Create GWorld */
		err = BenSWCreateGWorldFromPict(&tempPictGWorldP, newPictH);

		/* Release resource */
		ReleaseResource((Handle)newPictH);

		/* Error */
		if (err == noErr)
		{
			/* Create the frame */
			frameP = (FrameRec*)NewPtrClear((Size)sizeof(FrameRec));

			/* Analyze result */
			err = (frameP ? 0 : -1);

			/* Oops */
			if (err == noErr)
			{
				/* Save GWorld */
				frameP->framePort = tempPictGWorldP;

				/* Lock it */
				BenSWLockFrame(frameP);
			}
		}
	}

	/* Result */
	return (err);
}


/*
 * Nuke the global "frameP"
 */
static errr globe_nuke(void)
{
	/* Dispose */
	if (frameP)
	{
		/* Unlock */
		BenSWUnlockFrame(frameP);

		/* Dispose of the GWorld */
		DisposeGWorld(frameP->framePort);

		/* Dispose of the memory */
		DisposePtr((Ptr)frameP);

		/* Forget */
		frameP = NULL;
	}

	/* Flush events */	
	FlushEvents(everyEvent, 0);

	/* Success */
	return (0);
}


#endif /* ANGBAND_LITE_MAC */



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

	static RGBColor black = {0x0000,0x0000,0x0000};
	static RGBColor white = {0xFFFF,0xFFFF,0xFFFF};

#ifdef ANGBAND_LITE_MAC

	/* Make the window */
	td->w = NewWindow(0, &td->r, td->title, 0, noGrowDocProc, (WindowPtr)-1, 1, 0L);

#else /* ANGBAND_LITE_MAC */

	/* Make the window */
	td->w = NewCWindow(0, &td->r, td->title, 0, documentProc, (WindowPtr)-1, 1, 0L);

#endif /* ANGBAND_LITE_MAC */

	/* Activate the window */
	activate(td->w);

	/* Erase behind words */
	TextMode(srcCopy);

	/* Apply and Verify */
	term_data_check_font(td);
	term_data_check_size(td);

	/* Resize the window */
	term_data_resize(td);

#ifdef ANGBAND_LITE_MAC

	/* Prepare the colors (base colors) */
	BackColor(blackColor);
	ForeColor(whiteColor);

#else /* ANGBAND_LITE_MAC */

	/* Prepare the colors (real colors) */
	RGBBackColor(&black);
	RGBForeColor(&white);

	/* Block */
	{
		Rect tempRect;
		Rect globalRect;
		GDHandle mainGDH;
		GDHandle currentGDH;
		GWorldPtr windowGWorld;
		PixMapHandle basePixMap;

		/* Obtain the rect */
		tempRect = td->w->portRect;

		/* Obtain the global rect */	
		globalRect = tempRect;
		LocalToGlobal((Point*)&globalRect.top);
		LocalToGlobal((Point*)&globalRect.bottom);

		/* Obtain the proper GDH */
		mainGDH = GetMaxDevice(&globalRect);

		/* Extract GWorld and GDH */
		GetGWorld(&windowGWorld, &currentGDH);

		/* Obtain base pixmap */
		basePixMap = (**mainGDH).gdPMap;

		/* Save pixel depth */
		td->pixelDepth = (**basePixMap).pixelSize;

		/* Save Window GWorld */
		td->theGWorld = windowGWorld;

		/* Save Window GDH */
		td->theGDH = currentGDH;

		/* Save main GDH */
		td->mainSWGDH = mainGDH;
	}

#endif /* ANGBAND_LITE_MAC */

	/* Clip to the window */
	ClipRect(&td->w->portRect);

	/* Erase the window */
	EraseRect(&td->w->portRect);

	/* Invalidate the window */
	InvalRect(&td->w->portRect);

	/* Display the window if needed */
	if (td->mapped) ShowWindow(td->w);

	/* Hack -- set "mapped" flag */
	t->mapped_flag = td->mapped;

	/* Forget color */
	td->last = -1;
}



/*
 * Nuke an old Term
 */
static void Term_nuke_mac(term *t)
{

#pragma unused (t)

	/* XXX */
}



/*
 * Unused
 */
static errr Term_user_mac(int n)
{

#pragma unused (n)

	/* Success */
	return (0);
}



/*
 * React to changes
 */
static errr Term_xtra_mac_react(void)
{
	term_data *td = (term_data*)(Term->data);


	/* Reset color */
	td->last = -1;

#ifdef ANGBAND_LITE_MAC

	/* Nothing */
	
#else /* ANGBAND_LITE_MAC */

	/* Handle sound */
	if (use_sound != arg_sound)
	{
		/* Apply request */
		use_sound = arg_sound;
	}

	/* Handle graphics */
	if ((td == &data[0]) && (use_graphics != arg_graphics))
	{
		/* Initialize graphics */
		if (!use_graphics && !frameP && (globe_init() != 0))
		{
			plog("Cannot initialize graphics!");
			arg_graphics = FALSE;
		}

		/* Apply request */
		use_graphics = arg_graphics;

		/* Apply and Verify */
		term_data_check_size(td);

		/* Resize the window */
		term_data_resize(td);

		/* Reset visuals */
		reset_visuals();
	}

#endif /* ANGBAND_LITE_MAC */

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_mac(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	Rect r;

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

#ifdef ANGBAND_LITE_MAC

		/* Nothing */

#else /* ANGBAND_LITE_MAC */

		/* Make a sound */
		case TERM_XTRA_SOUND:
		{
			Handle handle;

			Str255 sound;

#if 0
			short oldResFile;
			short newResFile;

			/* Open the resource file */
			oldResFile = CurResFile();
			newResFile = OpenResFile(sound);

			/* Close the resource file */
			CloseResFile(newResFile);
			UseResFile(oldResFile);
#endif

			/* Get the proper sound name */
			sprintf((char*)sound + 1, "%.16s.wav", angband_sound_name[v]);
			sound[0] = strlen((char*)sound + 1);

			/* Obtain resource XXX XXX XXX */
			handle = GetNamedResource('snd ', sound);

			/* Oops */
			if (handle)
			{
				/* Load and Lock */
				LoadResource(handle);
				HLock(handle);

				/* Play sound (wait for completion) */
				SndPlay(nil, (SndListHandle)handle, false);

				/* Unlock and release */
				HUnlock(handle);
				ReleaseResource(handle);
			}

			/* Success */
			return (0);
		}

#endif /* ANGBAND_LITE_MAC */

		/* Process random events */
		case TERM_XTRA_BORED:
		{
			/* Process an event */
			(void)CheckEvents(0);

			/* Success */
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
			/* Hack -- flush all events */
			while (CheckEvents(TRUE)) /* loop */;

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
			/* No clipping XXX XXX XXX */
			ClipRect(&td->w->portRect);

			/* Erase the window */
			EraseRect(&td->w->portRect);

			/* Set the color */
			term_data_color(td, TERM_WHITE);

			/* Frame the window in white */
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

			/* Success */
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
			/* If needed */
			if (v > 0)
			{
				long m = TickCount() + (v * 60L) / 1000;

				/* Wait for it */
				while (TickCount() < m) /* loop */;
			}

			/* Success */
			return (0);
		}
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
static errr Term_curs_mac(int x, int y)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, TERM_YELLOW);

	/* Frame the grid */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	FrameRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_mac(int x, int y, int n)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Erase the block of characters */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + n * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	EraseRect(&r);

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
	int xp, yp;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, (a & 0x0F));

	/* Starting pixel */
	xp = x * td->tile_wid + td->tile_o_x + td->size_ow1;
	yp = y * td->tile_hgt + td->tile_o_y + td->size_oh1;

	/* Move to the correct location */
	MoveTo(xp, yp);

	/* Draw the character */
	if (n == 1) DrawChar(*cp);

	/* Draw the string */
	else DrawText(cp, 0, n);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp)
{
	int i;

	Rect r2;

	term_data *td = (term_data*)(Term->data);


	/* Destination rectangle */
	r2.left = x * td->tile_wid + td->size_ow1;
	r2.right = r2.left + td->tile_wid;
	r2.top = y * td->tile_hgt + td->size_oh1;
	r2.bottom = r2.top + td->tile_hgt;

	/* Scan the input */
	for (i = 0; i < n; i++)
	{
		bool done = FALSE;

		byte a = ap[i];
		char c = cp[i];

#ifdef ANGBAND_LITE_MAC

		/* Nothing */

#else /* ANGBAND_LITE_MAC */

		/* Graphics -- if Available and Needed */
		if (use_graphics && (td == &data[0]) &&
		    ((byte)a & 0x80) && ((byte)c & 0x80))
		{
			int col, row;

			Rect r1;

			/* Row and Col */
			row = ((byte)a & 0x7F) % kPictRows;
			col = ((byte)c & 0x7F) % kPictCols;

			/* Source rectangle */
			r1.left = col * kGrafWidth;
			r1.top = row * kGrafHeight;
			r1.right = r1.left + kGrafWidth;
			r1.bottom = r1.top + kGrafHeight;

			/* Hardwire CopyBits */
			BackColor(whiteColor);
			ForeColor(blackColor);

			/* Draw the picture */
			CopyBits((BitMap*)frameP->framePix,
					 &(td->w->portBits),
					 &r1, &r2, srcCopy, NULL);

			/* Restore colors */
			BackColor(blackColor);
			ForeColor(whiteColor);

			/* Forget color */
			td->last = -1;

			/* Done */
			done = TRUE;
		}

#endif /* ANGBAND_LITE_MAC */

		/* Normal */
		if (!done)
		{
			int xp, yp;

			/* Erase */
			EraseRect(&r2);

			/* Set the color */
			term_data_color(td, (a & 0x0F));

			/* Starting pixel */
			xp = r2.left + td->tile_o_x;
			yp = r2.top + td->tile_o_y;

			/* Move to the correct location */
			MoveTo(xp, yp);

			/* Draw the character */
			DrawChar(c);
		}

		/* Advance */
		r2.left += td->tile_wid;
		r2.right += td->tile_wid;
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
	term_init(td->t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	td->t->soft_cursor = TRUE;

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
	td->t->text_hook = Term_text_mac;
	td->t->pict_hook = Term_pict_mac;

	/* Link the local structure */
	td->t->data = (vptr)(td);

	/* Activate it */
	Term_activate(td->t);

	/* Global pointer */
	angband_term[i] = td->t;

	/* Activate old */
	Term_activate(old);
}




/*
 * Set the "current working directory" (also known as the "default"
 * volume/directory) to the location of the current application.
 *
 * Code by: Maarten Hazewinkel (mmhazewi@cs.ruu.nl)
 *
 * This function does not appear to work correctly with System 6.
 */
static void SetupAppDir(void)
{
	FCBPBRec fcbBlock;
	OSErr err = noErr;
	char errString[100];

	/* Get the location of the Angband executable */
	fcbBlock.ioCompletion = NULL;
	fcbBlock.ioNamePtr = NULL;
	fcbBlock.ioVRefNum = 0;
	fcbBlock.ioRefNum = CurResFile();
	fcbBlock.ioFCBIndx = 0;
	err = PBGetFCBInfo(&fcbBlock, FALSE);
	if (err != noErr)
	{
		sprintf(errString, "Fatal PBGetFCBInfo Error #%d.\r Exiting.", err);
		mac_warning(errString);
		ExitToShell();
	}

	/* Extract the Vol and Dir */
	app_vol = fcbBlock.ioFCBVRefNum;
	app_dir = fcbBlock.ioFCBParID;

	/* Set the current working directory to that location */
	err = HSetVol(NULL, app_vol, app_dir);
	if (err != noErr)
	{
		sprintf(errString, "Fatal HSetVol Error #%d.\r Exiting.", err);
		mac_warning(errString);
		ExitToShell();
	}
}




/*
 * Global "preference" file pointer
 */
static FILE *fff;

/*
 * Read a "short" from the file
 */
static int getshort(void)
{
	int x = 0;
	char buf[256];
	if (0 == my_fgets(fff, buf, 256)) x = atoi(buf);
	return (x);
}

/*
 * Dump a "short" to the file
 */
static void putshort(int x)
{
	fprintf(fff, "%d\n", x);
}



/*
 * Write the "preference" data to the current "file"
 */
static void save_prefs(void)
{
	int i;

	term_data *td;


	/*** The current version ***/

	putshort(VERSION_MAJOR);
	putshort(VERSION_MINOR);
	putshort(VERSION_PATCH);
	putshort(VERSION_EXTRA);


	/* Dump */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access */
		td = &data[i];

		putshort(td->mapped);

		putshort(td->font_id);
		putshort(td->font_size);
		putshort(td->font_face);

		putshort(td->cols);
		putshort(td->rows);

		putshort(td->r.left);
		putshort(td->r.top);
	}
}


/*
 * Load the preferences from the current "file"
 *
 * XXX XXX XXX Being able to undefine various windows is
 * slightly bizarre, and may cause problems.
 */
static void load_prefs(void)
{
	int i;

	int old_major, old_minor, old_patch, old_extra;

	term_data *td;


	/*** Version information ***/

	/* Preferences version */
	old_major = getshort();
	old_minor = getshort();
	old_patch = getshort();
	old_extra = getshort();

	/* Hack -- Verify or ignore */
	if ((old_major != VERSION_MAJOR) ||
	    (old_minor != VERSION_MINOR) ||
	    (old_patch != VERSION_PATCH) ||
	    (old_extra != VERSION_EXTRA))
	{
		/* Message */
		mac_warning("Ignoring old preferences.");

		/* Ignore */
		return;
	}


	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access */
		td = &data[i];

		td->mapped = getshort();

		td->font_id = getshort();
		td->font_size = getshort();
		td->font_face = getshort();

		td->cols = getshort();
		td->rows = getshort();

		td->r.left = getshort();
		td->r.top = getshort();

		/* Done */
		if (feof(fff)) break;
	}
}




/*
 * Hack -- default data for a window
 */
static void term_data_hack(term_data *td)
{
	short fid;

	/* Default to Monaco font */
	GetFNum("\pmonaco", &fid);

	/* Wipe it */
	WIPE(td, term_data);

	/* No color */
	td->last = -1;

	/* Default borders */
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh2 = 2;

	/* Start hidden */
	td->mapped = FALSE;

	/* Default font */
	td->font_id = fid;

	/* Default font size */
	td->font_size = 12;

	/* Default font face */
	td->font_face = 0;

	/* Default size */
	td->rows = 24;
	td->cols = 80;

	/* Default position */
	td->r.left = 10;
	td->r.top = 40;

	/* Minimal keys */
	td->keys = 16;
}


/*
 * Read the preference file, Create the windows.
 *
 * We attempt to use "FindFolder()" to track down the preference file,
 * but if this fails, for any reason, we will try the "SysEnvirons()"
 * method, which may work better with System 6.
 */
static void init_windows(void)
{
	int i, b = 0;

	term_data *td;

	SysEnvRec env;
	short savev;
	long saved;

	bool oops;


	/*** Default values ***/

	/* Initialize (backwards) */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--)
	{
		int n;

		cptr s;

		/* Obtain */
		td = &data[i];

		/* Defaults */
		term_data_hack(td);

		/* Obtain title */
		s = angband_term_name[i];

		/* Get length */
		n = strlen(s);

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

	/* Assume failure */
	oops = TRUE;

	/* Assume failure */
	fff = NULL;

#ifdef USE_SFL_CODE

	/* Block */
	if (TRUE)
	{
		OSErr	err;
		short	vref;
		long	dirID;
		char	foo[128];

		/* Find the folder */
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		                 &vref, &dirID);

		/* Success */
		if (!err)
		{
			/* Extract a path name */
			PathNameFromDirID(dirID, vref, (StringPtr)foo);

			/* Convert the string */
			ptocstr((StringPtr)foo);

			/* Append the preference file name */
			strcat(foo, "Angband Preferences");

			/* Open the preference file */
			fff = fopen(foo, "r");

			/* Success */
			oops = FALSE;
		}
	}

#endif /* USE_SFL_CODE */

	/* Oops */
	if (oops)
	{
		/* Save */
		HGetVol(0, &savev, &saved);

		/* Go to the "system" folder */
		SysEnvirons(curSysEnvVers, &env);
		SetVol(0, env.sysVRefNum);

		/* Open the file */
		fff = fopen(":Preferences:Angband Preferences", "r");
		if (!fff) fff = fopen(":Angband Preferences", "r");

		/* Restore */
		HSetVol(0, savev, saved);
	}

	/* Load preferences */
	if (fff)
	{
		/* Load a real preference file */
		load_prefs();

		/* Close the file */
		my_fclose(fff);
	}


	/*** Instantiate ***/

	/* Main window */
	td = &data[0];

	/* Many keys */
	td->keys = 1024;

	/* Start visible */
	td->mapped = TRUE;

	/* Link (backwards, for stacking order) */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--)
	{
		term_data_link(i);
	}

	/* Main window */
	td = &data[0];

	/* Main window */
	Term_activate(td->t);
}


/*
 * Exit the program
 */
static void save_pref_file(void)
{
	bool oops;

	SysEnvRec env;
	short savev;
	long saved;


	/* Assume failure */
	oops = TRUE;

	/* Assume failure */
	fff = NULL;

#if defined(MACINTOSH) && !defined(applec)
	/* Text file */
	_ftype = 'TEXT';
#endif


#ifdef USE_SFL_CODE

	/* Block */
	if (TRUE)
	{
		OSErr	err;
		short	vref;
		long	dirID;
		char	foo[128];

		/* Find the folder */
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		                 &vref, &dirID);

		/* Success */
		if (!err)
		{
			/* Extract a path name */
			PathNameFromDirID(dirID, vref, (StringPtr)foo);

			/* Convert the string */
			ptocstr((StringPtr)foo);

			/* Append the preference file name */
			strcat(foo, "Angband Preferences");

			/* Open the preference file */
			fff = fopen(foo, "w");

			/* Success */
			oops = FALSE;
		}
	}

#endif /* USE_SFL_CODE */

	/* Oops */
	if (oops)
	{
		/* Save */
		HGetVol(0, &savev, &saved);

		/* Go to "system" folder */
		SysEnvirons(curSysEnvVers, &env);
		SetVol(0, env.sysVRefNum);

		/* Open the preference file */
		fff = fopen(":Preferences:Angband Preferences", "w");
		if (!fff) fff = fopen(":Angband Preferences", "w");

		/* Restore */
		HSetVol(0, savev, saved);
	}

	/* Save preferences */
	if (fff)
	{
		/* Write the preferences */
		save_prefs();

		/* Close it */
		my_fclose(fff);
	}
}



/*
 * A simple "Yes/No" filter to parse "key press" events in dialog windows
 */
static pascal Boolean ynfilter(DialogPtr dialog, EventRecord *event, short *ip)
{
	/* Parse key press events */
	if (event->what == keyDown)
	{
		int i = 0;
		char c;

		/* Extract the pressed key */
		c = (event->message & charCodeMask);

		/* Accept "no" and <return> and <enter> */
		if ((c=='n') || (c=='N') || (c==13) || (c==3)) i = 1;

		/* Accept "yes" */
		else if ((c=='y') || (c=='Y')) i = 2;

		/* Handle "yes" or "no" */
		if (i)
		{
			short type;
			ControlHandle control;
			Rect r;

			/* Get the button */
			GetDItem(dialog, i, &type, (Handle*)&control, &r);

			/* Blink button for 1/10 second */
			HiliteControl(control, 1);
			Term_xtra_mac(TERM_XTRA_DELAY, 100);
			HiliteControl(control, 0);

			/* Result */
			*ip = i;
			return (1);
		}
	}

	/* Ignore */
	return (0);
}


/*
 * Handle menu: "File" + "New"
 */
static void do_menu_file_new(void)
{
	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = 1;

	/* Flush input */
	flush();

	/* Play a game */
	play_game(TRUE);

	/* Hack -- quit */
	quit(NULL);
}


/*
 * Handle menu: "File" + "Open"
 */
static void do_menu_file_open(bool all)
{
	int err;
	short vrefnum;
	long drefnum;
	long junk;
	DirInfo pb;
	SFTypeList types;
	SFReply reply;
	Point topleft;


	/* XXX XXX XXX */

	/* vrefnum = GetSFCurVol(); */
	vrefnum = -*((short*)0x214);

	/* drefnum = GetSFCurDir(); */
	drefnum = *((long*)0x398);

	/* Descend into "lib" folder */
	pb.ioCompletion = NULL;
	pb.ioNamePtr = "\plib";
	pb.ioVRefNum = vrefnum;
	pb.ioDrDirID = drefnum;
	pb.ioFDirIndex = 0;

	/* Check for errors */
	err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

	/* Success */
	if ((err == noErr) && (pb.ioFlAttrib & 0x10))
	{
		/* Descend into "lib/save" folder */
		pb.ioCompletion = NULL;
		pb.ioNamePtr = "\psave";
		pb.ioVRefNum = vrefnum;
		pb.ioDrDirID = pb.ioDrDirID;
		pb.ioFDirIndex = 0;

		/* Check for errors */
		err = PBGetCatInfo((CInfoPBPtr)&pb, FALSE);

		/* Success */
		if ((err == noErr) && (pb.ioFlAttrib & 0x10))
		{
			/* SetSFCurDir(pb.ioDrDirID); */
			*((long*)0x398) = pb.ioDrDirID;
		}
	}

	/* Window location */
	topleft.h = (qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2-344/2;
	topleft.v = (2*qd.screenBits.bounds.top+qd.screenBits.bounds.bottom)/3-188/2;

	/* Allow "all" files */
	if (all)
	{
		/* Get any file */
		SFGetFile(topleft, "\p", NULL, -1, types, NULL, &reply);
	}

	/* Allow "save" files */
	else
	{
		/* Legal types */
		types[0] = 'SAVE';

		/* Get a file */
		SFGetFile(topleft, "\p", NULL, 1, types, NULL, &reply);
	}

	/* Allow cancel */
	if (!reply.good) return;

	/* Extract textual file name for save file */
	GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);
	refnum_to_name(savefile, drefnum, vrefnum, (char*)reply.fName);

	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = 1;

	/* Flush input */
	flush();

	/* Play a game */
	play_game(FALSE);

	/* Hack -- quit */
	quit(NULL);
}


/*
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
		game_in_progress = 1;

		/* Wait for it */
		pause_line(23);

		/* Flush input */
		flush();

		/* Play a game */
		play_game(FALSE);

		/* Quit */
		quit(NULL);
	}
}



/*
 * Initialize the menus
 *
 * Verify menus 128, 129, 130
 * Create menus 131, 132, 133, 134
 *
 * The standard menus are:
 *
 *   Apple (128) =   { About, -, ... }
 *   File (129) =    { New,Open,Import,Close,Save,-,Exit,Quit }
 *   Edit (130) =    { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) =    { Bold, Extend, -, Monaco, ..., -, ... }
 *   Size (132) =    { ... }
 *   Window (133) =  { Angband, Mirror, Recall, Choice,
 *                     Term-4, Term-5, Term-6, Term-7 }
 *   Special (134) = { arg_sound, arg_graphics, -,
 *                     arg_fiddle, arg_wizard }
 */
static void init_menubar(void)
{
	int i, n;

	Rect r;

	WindowPtr tmpw;

	MenuHandle m;


	/* Get the "apple" menu */
	m = GetMenu(128);

	/* Insert the menu */
	InsertMenu(m, 0);

	/* Add the DA's to the "apple" menu */
	AddResMenu(m, 'DRVR');


	/* Get the "File" menu */
	m = GetMenu(129);

	/* Insert the menu */
	InsertMenu(m, 0);


	/* Get the "Edit" menu */
	m = GetMenu(130);

	/* Insert the menu */
	InsertMenu(m, 0);


	/* Make the "Font" menu */
	m = NewMenu(131, "\pFont");

	/* Insert the menu */
	InsertMenu(m, 0);

	/* Add "bold" */
	AppendMenu(m, "\pBold");

	/* Add "wide" */
	AppendMenu(m, "\pWide");

	/* Add a separator */
	AppendMenu(m, "\p-");

	/* Fake window */
	r.left = r.right = r.top = r.bottom = 0;

	/* Make the fake window */
	tmpw = NewWindow(0, &r, "\p", false, documentProc, 0, 0, 0);

	/* Activate the "fake" window */
	SetPort(tmpw);

	/* Default mode */
	TextMode(0);

	/* Default size */
	TextSize(12);

	/* Add the fonts to the menu */
	AddResMenu(m, 'FONT');

	/* Size of menu */
	n = CountMItems(m);

	/* Scan the menu */
	for (i = n; i >= 4; i--)
	{
		Str255 tmpName;
		short fontNum;

		/* Acquire the font name */
		/* GetMenuItemText(m, i, tmpName); */
		GetItem(m, i, tmpName);

		/* Acquire the font index */
		GetFNum(tmpName, &fontNum);

		/* Apply the font index */
		TextFont(fontNum);

		/* Remove non-mono-spaced fonts */
		if ((CharWidth('i') != CharWidth('W')) || (CharWidth('W') == 0))
		{
			/* Delete the menu item XXX XXX XXX */
			/* DeleteMenuItem(m, i); */
			DelMenuItem(m, i);
		}
	}

	/* Destroy the old window */
	DisposeWindow(tmpw);

	/* Add a separator */
	AppendMenu(m, "\p-");

	/* Add the fonts to the menu */
	AddResMenu(m, 'FONT');


	/* Make the "Size" menu */
	m = NewMenu(132, "\pSize");

	/* Insert the menu */
	InsertMenu(m, 0);

	/* Add some sizes (stagger choices) */
	for (i = 8; i <= 32; i += ((i / 16) + 1))
	{
		Str15 buf;
		
		/* Textual size */
		sprintf((char*)buf + 1, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);
	}


	/* Make the "Windows" menu */
	m = NewMenu(133, "\pWindows");

	/* Insert the menu */
	InsertMenu(m, 0);

	/* Default choices */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		Str15 buf;
		
		/* Describe the item */
		sprintf((char*)buf + 1, "%.15s", angband_term_name[i]);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);

		/* Command-Key shortcuts */
		if (i < 8) SetItemCmd(m, i + 1, '0' + i);
	}


	/* Make the "Special" menu */
	m = NewMenu(134, "\pSpecial");

	/* Insert the menu */
	InsertMenu(m, 0);

	/* Append the choices */
	AppendMenu(m, "\parg_sound");
	AppendMenu(m, "\parg_graphics");
	AppendMenu(m, "\p-");
	AppendMenu(m, "\parg_fiddle");
	AppendMenu(m, "\parg_wizard");


	/* Make the "TileWidth" menu */
	m = NewMenu(135, "\pTileWidth");

	/* Insert the menu */
	InsertMenu(m, 0);

	/* Add some sizes */
	for (i = 4; i <= 32; i++)
	{
		Str15 buf;
		
		/* Textual size */
		sprintf((char*)buf + 1, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);
	}


	/* Make the "TileHeight" menu */
	m = NewMenu(136, "\pTileHeight");

	/* Insert the menu */
	InsertMenu(m, 255);

	/* Add some sizes */
	for (i = 4; i <= 32; i++)
	{
		Str15 buf;

		/* Textual size */
		sprintf((char*)buf + 1, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);
	}


	/* Update the menu bar */
	DrawMenuBar();
}


/*
 * Prepare the menus
 */
static void setup_menus(void)
{
	int i, n;

	short value;

	Str255 s;

	MenuHandle m;

	term_data *td = NULL;


	/* Relevant "term_data" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Unused */
		if (!data[i].t) continue;

		/* Notice the matching window */
		if (data[i].w == FrontWindow()) td = &data[i];
	}


	/* File menu */
	m = GetMHandle(129);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Enable "new"/"open..."/"import..." */
	if (initialized && !game_in_progress)
	{
		EnableItem(m, 1);
		EnableItem(m, 2);
		EnableItem(m, 3);
	}

	/* Enable "close" */
	if (initialized)
	{
		EnableItem(m, 4);
	}

	/* Enable "save" */
	if (initialized && character_generated)
	{
		EnableItem(m, 5);
	}

	/* Enable "exit"/"quit" */
	if (TRUE)
	{
		EnableItem(m, 7);
		EnableItem(m, 8);
	}


	/* Edit menu */
	m = GetMHandle(130);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Enable "edit" options if "needed" */
	if (!td)
	{
		EnableItem(m, 1);
		EnableItem(m, 3);
		EnableItem(m, 4);
		EnableItem(m, 5);
		EnableItem(m, 6);
	}


	/* Font menu */
	m = GetMHandle(131);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, 1, bold); */

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, 2, extend); */

	/* Active window */
	if (td)
	{
		/* Enable "bold" */
		EnableItem(m, 1);

		/* Enable "extend" */
		EnableItem(m, 2);

		/* Check the appropriate "bold-ness" */
		if (td->font_face & bold) CheckItem(m, 1, TRUE);

		/* Check the appropriate "wide-ness" */
		if (td->font_face & extend) CheckItem(m, 2, TRUE);

		/* Analyze fonts */
		for (i = 4; i <= n; i++)
		{
			/* Enable it */
			EnableItem(m, i);

			/* Analyze font */
			/* GetMenuItemText(m,i,s); */
			GetItem(m, i, s);
			GetFNum(s, &value);

			/* Check active font */
			if (td->font_id == value) CheckItem(m, i, TRUE);
		}
	}


	/* Size menu */
	m = GetMHandle(132);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}
	
	/* Active window */
	if (td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			/* GetMenuItemText(m,i,s); */
			GetItem(m, i, s);
			s[s[0]+1] = '\0';
			value = atoi((char*)(s+1));

			/* Enable the "real" sizes */
			if (RealFont(td->font_id, value)) EnableItem(m, i);

			/* Check the current size */
			if (td->font_size == value) CheckItem(m, i, TRUE);
		}
	}


	/* Windows menu */
	m = GetMHandle(133);

	/* Get menu size */
	n = CountMItems(m);

	/* Check active windows */
	for (i = 1; i <= n; i++)
	{
		/* Check if needed */
		CheckItem(m, i, data[i-1].mapped);
	}


	/* Special menu */
	m = GetMHandle(134);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Item "arg_sound" */
	EnableItem(m, 1);
	CheckItem(m, 1, arg_sound);

	/* Item "arg_graphics" */
	EnableItem(m, 2);
	CheckItem(m, 2, arg_graphics);

	/* Item "arg_fiddle" */
	EnableItem(m, 4);
	CheckItem(m, 4, arg_fiddle);

	/* Item "arg_wizard" */
	EnableItem(m, 5);
	CheckItem(m, 5, arg_wizard);

	/* Item "Hack" */
	/* EnableItem(m, 9); */


	/* TileWidth menu */
	m = GetMHandle(135);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			/* GetMenuItemText(m,i,s); */
			GetItem(m, i, s);
			s[s[0]+1] = '\0';
			value = atoi((char*)(s+1));

			/* Enable */
			EnableItem(m, i);

			/* Check the current size */
			if (td->tile_wid == value) CheckItem(m, i, TRUE);
		}
	}


	/* TileHeight menu */
	m = GetMHandle(136);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			/* GetMenuItemText(m,i,s); */
			GetItem(m, i, s);
			s[s[0]+1] = '\0';
			value = atoi((char*)(s+1));

			/* Enable */
			EnableItem(m, i);

			/* Check the current size */
			if (td->tile_hgt == value) CheckItem(m, i, TRUE);
		}
	}
}


/*
 * Process a menu selection (see above)
 *
 * Hack -- assume that invalid menu selections are disabled above,
 * which I have been informed may not be reliable.  XXX XXX XXX
 */
static void menu(long mc)
{
	int i;

	int menuid, selection;

	static unsigned char s[1000];

	short fid;

	term_data *td = NULL;

	WindowPtr old_win;


	/* Analyze the menu command */
	menuid = HiWord(mc);
	selection = LoWord(mc);


	/* Find the window */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Skip dead windows */
		if (!data[i].t) continue;

		/* Notice matches */
		if (data[i].w == FrontWindow()) td = &data[i];
	}


	/* Branch on the menu */
	switch (menuid)
	{
		/* Apple Menu */
		case 128:
		{
			/* About Angband... */
			if (selection == 1)
			{
				DialogPtr dialog;
				Rect r;
				short item_hit;

				dialog=GetNewDialog(128, 0, (WindowPtr)-1);

				r=dialog->portRect;
				center_rect(&r, &qd.screenBits.bounds);
				MoveWindow(dialog, r.left, r.top, 1);
				ShowWindow(dialog);
				ModalDialog(0, &item_hit);
				DisposDialog(dialog);
				break;
			}

			/* Desk accessory */
			/* GetMenuItemText(GetMHandle(128),selection,s); */
			GetItem(GetMHandle(128), selection, s);
			OpenDeskAcc(s);
			break;
		}

		/* File Menu */
		case 129:
		{
			switch (selection)
			{
				/* New */
				case 1:
				{
					do_menu_file_new();
					break;
				}

				/* Open... */
				case 2:
				{
					do_menu_file_open(FALSE);
					break;
				}

				/* Import... */
				case 3:
				{
					do_menu_file_open(TRUE);
					break;
				}

				/* Close */
				case 4:
				{
					/* No window */
					if (!td) break;

					/* Not Mapped */
					td->mapped = FALSE;

					/* Not Mapped */
					td->t->mapped_flag = FALSE;

					/* Hide the window */
					HideWindow(td->w);

					break;
				}

				/* Save */
				case 5:
				{
					/* Hack -- Forget messages */
					msg_flag = FALSE;

					/* Hack -- Save the game */
					do_cmd_save_game();

					break;
				}

				/* Exit (without save) */
				case 7:
				{
					/* Allow user to cancel "dangerous" exit */
					if (game_in_progress && character_generated)
					{
						AlertTHndl alert;
						short item_hit;

						/* Get the "alert" info */
						alert = (AlertTHndl)GetResource('ALRT', 130);

						/* Center the "alert" rectangle */
						center_rect(&(*alert)->boundsRect,
						            &qd.screenBits.bounds);

						/* Display the Alert, get "No" or "Yes" */
						item_hit = Alert(130, ynfilterUPP);

						/* Require "yes" button */
						if (item_hit != 2) break;
					}

					/* Quit */
					quit(NULL);
					break;
				}

				/* Quit (with save) */
				case 8:
				{
					/* Save the game (if necessary) */
					if (game_in_progress && character_generated)
					{
						/* Hack -- Forget messages */
						msg_flag = FALSE;

						/* Save the game */
						do_cmd_save_game();
					}

					/* Quit */
					quit(NULL);
					break;
				}
			}
			break;
		}

		/* Edit menu */
		case 130:
		{
			/* Unused */
			break;
		}

		/* Font menu */
		case 131:
		{
			/* Require a window */
			if (!td) break;

			/* Memorize old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Toggle the "bold" setting */
			if (selection == 1)
			{
				/* Toggle the setting */
				if (td->font_face & bold)
				{
					td->font_face &= ~bold;
				}
				else
				{
					td->font_face |= bold;
				}

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Toggle the "wide" setting */
			if (selection == 2)
			{
				/* Toggle the setting */
				if (td->font_face & extend)
				{
					td->font_face &= ~extend;
				}
				else
				{
					td->font_face |= extend;
				}

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Get a new font name */
			/* GetMenuItemText(GetMHandle(131), selection, s); */
			GetItem(GetMHandle(131), selection, s);
			GetFNum(s, &fid);

			/* Save the new font id */
			td->font_id = fid;

			/* Current size is bad for new font */
			if (!RealFont(td->font_id, td->font_size))
			{
				/* Find similar size */
				for (i = 1; i <= 32; i++)
				{
					/* Adjust smaller */
					if (td->font_size - i >= 8)
					{
						if (RealFont(td->font_id, td->font_size - i))
						{
							td->font_size -= i;
							break;
						}
					}

					/* Adjust larger */
					if (td->font_size + i <= 128)
					{
						if (RealFont(td->font_id, td->font_size + i))
						{
							td->font_size += i;
							break;
						}
					}
				}
			}

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore the window */
			activate(old_win);

			break;
		}

		/* Size menu */
		case 132:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* GetMenuItemText(GetMHandle(132), selection, s); */
			GetItem(GetMHandle(132), selection, s);
			s[s[0]+1]=0;
			td->font_size = atoi((char*)(s+1));

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* Window menu */
		case 133:
		{
			/* Parse */
			i = selection - 1;

			/* Check legality of choice */
			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			/* Obtain the window */
			td = &data[i];

			/* Mapped */
			td->mapped = TRUE;

			/* Link */	
			term_data_link(i);

			/* Mapped (?) */
			td->t->mapped_flag = TRUE;

			/* Show the window */
			ShowWindow(td->w);

			/* Bring to the front */
			SelectWindow(td->w);

			break;
		}

		/* Special menu */
		case 134:
		{
			switch (selection)
			{
				case 1:
				{
					/* Toggle arg_sound */
					arg_sound = !arg_sound;

					/* React to changes */
					Term_xtra(TERM_XTRA_REACT, 0);

					break;
				}

				case 2:
				{
					/* Toggle arg_graphics */
					arg_graphics = !arg_graphics;

					/* Hack -- Force redraw */
					Term_key_push(KTRL('R'));

					break;
				}

				case 4:
				{
					arg_fiddle = !arg_fiddle;
					break;
				}

				case 5:
				{
					arg_wizard = !arg_wizard;
					break;
				}
			}

			break;
		}

		/* TileWidth menu */
		case 135:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* GetMenuItemText(GetMHandle(135), selection, s); */
			GetItem(GetMHandle(135), selection, s);
			s[s[0]+1]=0;
			td->tile_wid = atoi((char*)(s+1));

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* TileHeight menu */
		case 136:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* GetMenuItemText(GetMHandle(136), selection, s); */
			GetItem(GetMHandle(136), selection, s);
			s[s[0]+1]=0;
			td->tile_hgt = atoi((char*)(s+1));

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}
	}


	/* Clean the menu */
	HiliteMenu(0);
}


#ifdef USE_SFL_CODE


/*
 * Check for extra required parameters -- From "Maarten Hazewinkel"
 */
static OSErr CheckRequiredAEParams(const AppleEvent *theAppleEvent)
{
	OSErr	aeError;
	DescType	returnedType;
	Size	actualSize;

	aeError = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
	                            &returnedType, NULL, 0, &actualSize);

	if (aeError == errAEDescNotFound) return (noErr);

	if (aeError == noErr) return (errAEParamMissed);

	return (aeError);
}


/*
 * Apple Event Handler -- Open Application
 */
static pascal OSErr AEH_Start(const AppleEvent *theAppleEvent,
                              const AppleEvent *reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Quit Application
 */
static pascal OSErr AEH_Quit(const AppleEvent *theAppleEvent,
                             const AppleEvent *reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	/* Quit later */
	quit_when_ready = TRUE;

	/* Check arguments */
	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Print Documents
 */
static pascal OSErr AEH_Print(const AppleEvent *theAppleEvent,
                              const AppleEvent *reply, long handlerRefCon)
{
#pragma unused(theAppleEvent, reply, handlerRefCon)

	return (errAEEventNotHandled);
}


/*
 * Apple Event Handler by Steve Linberg (slinberg@crocker.com).
 *
 * The old method of opening savefiles from the finder does not work
 * on the Power Macintosh, because CountAppFiles and GetAppFiles,
 * used to return information about the selected document files when
 * an application is launched, are part of the Segment Loader, which
 * is not present in the RISC OS due to the new memory architecture.
 *
 * The "correct" way to do this is with AppleEvents.  The following
 * code is modeled on the "Getting Files Selected from the Finder"
 * snippet from Think Reference 2.0.  (The prior sentence could read
 * "shamelessly swiped & hacked")
 */
static pascal OSErr AEH_Open(AppleEvent *theAppleEvent,
                             AppleEvent* reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	FSSpec		myFSS;
	AEDescList	docList;
	OSErr		err;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	char		foo[128];
	FInfo		myFileInfo;

	/* Put the direct parameter (a descriptor list) into a docList */
	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err) return err;

	/*
	 * We ignore the validity check, because we trust the FInder, and we only
	 * allow one savefile to be opened, so we ignore the depth of the list.
	 */

	err = AEGetNthPtr(&docList, 1L, typeFSS, &keywd,
	                  &returnedType, (Ptr) &myFSS, sizeof(myFSS), &actualSize);
	if (err) return err;

	/* Only needed to check savefile type below */
	err = FSpGetFInfo(&myFSS, &myFileInfo);
	if (err)
	{
		sprintf(foo, "Arg!  FSpGetFInfo failed with code %d", err);
		mac_warning (foo);
		return err;
	}

	/* Ignore non 'SAVE' files */
	if (myFileInfo.fdType != 'SAVE') return noErr;

	/* XXX XXX XXX Extract a file name */
	PathNameFromDirID(myFSS.parID, myFSS.vRefNum, (StringPtr)savefile);
	pstrcat((StringPtr)savefile, (StringPtr)&myFSS.name);

	/* Convert the string */
	ptocstr((StringPtr)savefile);

	/* Delay actual open */
	open_when_ready = TRUE;

	/* Dispose */
	err = AEDisposeDesc(&docList);

	/* Success */
	return noErr;
}


#endif



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
 * Optimize non-blocking calls to "CheckEvents()"
 * Idea from "Maarten Hazewinkel <mmhazewi@cs.ruu.nl>"
 */
#define EVENT_TICKS 6


/*
 * Check for Events, return TRUE if we process any
 *
 * Hack -- Handle AppleEvents if appropriate (ignore result code).
 */
static bool CheckEvents(bool wait)
{
	EventRecord event;

	WindowPtr w;

	Rect r;

	long newsize;

	int ch, ck;

	int mc, ms, mo, mx;

	int i;

	term_data *td = NULL;

	huge curTicks;

	static huge lastTicks = 0L;


	/* Access the clock */
	curTicks = TickCount();

	/* Hack -- Allow efficient checking for non-pending events */
	if (!wait && (curTicks < lastTicks + EVENT_TICKS)) return (FALSE);

	/* Timestamp last check */
	lastTicks = curTicks;

	/* Let the "system" run */
	SystemTask();

	/* Get an event (or null) */
	GetNextEvent(everyEvent, &event);

	/* Hack -- Nothing is ready yet */
	if (event.what == nullEvent) return (FALSE);


	/* Analyze the event */
	switch (event.what)
	{

#if 0

		case activateEvt:
		{
			w = (WindowPtr)event.message;

			activate(w);

			break;
		}

#endif

		case updateEvt:
		{
			/* Extract the window */
			w = (WindowPtr)event.message;

			/* Find the window */
			for (i = 0; i < MAX_TERM_DATA; i++)
			{
				/* Skip dead windows */
				if (!data[i].t) continue;

				/* Notice matches */
				if (data[i].w == w) td = &data[i];
			}

			/* Hack XXX XXX XXX */
			BeginUpdate(w);
			EndUpdate(w);

			/* Redraw the window */
			if (td) term_data_redraw(td);

			break;
		}

		case keyDown:
		case autoKey:
		{
			/* Extract some modifiers */
			mc = (event.modifiers & controlKey) ? TRUE : FALSE;
			ms = (event.modifiers & shiftKey) ? TRUE : FALSE;
			mo = (event.modifiers & optionKey) ? TRUE : FALSE;
			mx = (event.modifiers & cmdKey) ? TRUE : FALSE;

			/* Keypress: (only "valid" if ck < 96) */
			ch = (event.message & charCodeMask) & 255;

			/* Keycode: see table above */
			ck = ((event.message & keyCodeMask) >> 8) & 255;

			/* Command + "normal key" -> menu action */
			if (mx && (ck < 64))
			{
				/* Hack -- Prepare the menus */
				setup_menus();

				/* Mega-Hack -- allow easy exit if nothing to save */
				if (!character_generated && (ch=='Q' || ch=='q')) ch = 'e';

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

			/* Hack -- normal "keypad keys" -> special keypress */
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
			else if (ck <= 127)
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
		}

		case mouseDown:
		{
			int code;

			/* Analyze click location */
			code = FindWindow(event.where, &w);

			/* Find the window */
			for (i = 0; i < MAX_TERM_DATA; i++)
			{
				/* Skip dead windows */
				if (!data[i].t) continue;

				/* Notice matches */
				if (data[i].w == w) td = &data[i];
			}

			/* Analyze */
			switch (code)
			{
				case inMenuBar:
				{
					setup_menus();
					menu(MenuSelect(event.where));
					HiliteMenu(0);
					break;
				}

				case inSysWindow:
				{
					SystemClick(&event, w);
					break;
				}

				case inDrag:
				{
					Point p;

					WindowPtr old_win;

					r = qd.screenBits.bounds;
					r.top += 20; /* GetMBarHeight() XXX XXX XXX */
					InsetRect(&r, 4, 4);
					DragWindow(w, event.where, &r);

					/* Oops */
					if (!td) break;

					/* Save */
					old_win = active;

					/* Activate */
					activate(td->w);

					/* Analyze */
					p.h = td->w->portRect.left;
					p.v = td->w->portRect.top;
					LocalToGlobal(&p);
					td->r.left = p.h;
					td->r.top = p.v;

					/* Restore */
					activate(old_win);

					/* Apply and Verify */
					term_data_check_size(td);

					break;
				}

				case inGoAway:
				{
					/* Oops */
					if (!td) break;

					/* Track the go-away box */
					if (TrackGoAway(w, event.where))
					{
						/* Not Mapped */
						td->mapped = FALSE;

						/* Not Mapped */
						td->t->mapped_flag = FALSE;

						/* Hide the window */
						HideWindow(td->w);
					}

					break;
				}

				case inGrow:
				{
					int x, y;

					term *old = Term;

					/* Oops */
					if (!td) break;

					/* Fake rectangle */
					r.left = 20 * td->tile_wid + td->size_ow1;
					r.right = 80 * td->tile_wid + td->size_ow1 + td->size_ow2 + 1;
					r.top = 1 * td->tile_hgt + td->size_oh1;
					r.bottom = 24 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

					/* Grow the rectangle */
					newsize = GrowWindow(w, event.where, &r);

					/* Handle abort */
					if (!newsize) break;

					/* Extract the new size in pixels */
					y = HiWord(newsize) - td->size_oh1 - td->size_oh2;
					x = LoWord(newsize) - td->size_ow1 - td->size_ow2;

					/* Extract a "close" approximation */
					td->rows = y / td->tile_hgt;
					td->cols = x / td->tile_wid;

					/* Apply and Verify */
					term_data_check_size(td);

					/* Activate */
					Term_activate(td->t);

					/* Hack -- Resize the term */
					Term_resize(td->cols, td->rows);

					/* Resize and Redraw */
					term_data_resize(td);
					term_data_redraw(td);

					/* Restore */
					Term_activate(old);

					break;
				}

				case inContent:
				{
					SelectWindow(w);

					break;
				}
			}

			break;
		}

		/* Disk Event -- From "Maarten Hazewinkel" */
		case diskEvt:
		{
			/* check for error when mounting the disk */
			if (HiWord(event.message) != noErr)
			{
				Point p =
				{120, 120};

				DILoad();
				DIBadMount(p, event.message);
				DIUnload();
			}

			break;
		}

		/* OS Event -- From "Maarten Hazewinkel" */
		case osEvt:
		{
			switch ((event.message >> 24) & 0x000000FF)
			{
				case suspendResumeMessage:

				/* Resuming: activate the front window */
				if (event.message & resumeFlag)
				{
					SetPort(FrontWindow());
					SetCursor(&qd.arrow);
				}

				/* Suspend: deactivate the front window */
				else
				{
					/* Nothing */
				}

				break;
			}

			break;
		}

#ifdef USE_SFL_CODE

		/* From "Steve Linberg" and "Maarten Hazewinkel" */
		case kHighLevelEvent:
		{
			/* Process apple events */
			if (AEProcessAppleEvent(&event) != noErr)
			{
				plog("Error in Apple Event Handler!");
			}

			/* Handle "quit_when_ready" */
			if (quit_when_ready)
			{
				/* Forget */
				quit_when_ready = FALSE;

				/* Do the menu key */
				menu(MenuKey('q'));

				/* Turn off the menus */
				HiliteMenu(0);
			}

			/* Handle "open_when_ready" */
			handle_open_when_ready();

			break;
		}

#endif

	}


	/* Something happened */
	return (TRUE);
}




/*** Some Hooks for various routines ***/


/*
 * Mega-Hack -- emergency lifeboat
 */
static vptr lifeboat = NULL;


/*
 * Hook to "release" memory
 */
static errr hook_rnfree(vptr v, huge size)
{

#pragma unused (size)

#ifdef USE_MALLOC

	/* Alternative method */
	free(v);

#else

	/* Dispose */
	DisposePtr(v);

#endif

	/* Success */
	return (0);
}

/*
 * Hook to "allocate" memory
 */
static vptr hook_ralloc(huge size)
{

#ifdef USE_MALLOC

	/* Make a new pointer */
	return (malloc(size));

#else

	/* Make a new pointer */
	return (NewPtr(size));

#endif

}

/*
 * Hook to handle "out of memory" errors
 */
static vptr hook_rpanic(huge size)
{

#pragma unused (size)

	vptr mem = NULL;

	/* Free the lifeboat */
	if (lifeboat)
	{
		/* Free the lifeboat */
		DisposePtr(lifeboat);

		/* Forget the lifeboat */
		lifeboat = NULL;

		/* Mega-Hack -- Warning */
		mac_warning("Running out of Memory!\rAbort this process now!");

		/* Mega-Hack -- Never leave this function */
		while (TRUE) CheckEvents(TRUE);
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

	/* Write a preference file */
	save_pref_file();

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

	/* Warn, then save player */
	mac_warning("Fatal error.\rI will now attempt to save and quit.");

	/* Attempt to save */
	if (!save_player()) mac_warning("Warning -- save failed!");

	/* Quit */
	quit(NULL);
}



/*** Main program ***/


/*
 * Init some stuff
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "Macintosh Save Bug" by using "absolute" path names, since on
 * System 7 machines anyway, the "current working directory" often
 * "changes" due to background processes, invalidating any "relative"
 * path names.  Note that the Macintosh is limited to 255 character
 * path names, so be careful about deeply embedded directories...
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "missing lib folder bug" by allowing the user to help find the
 * "lib" folder by hand if the "application folder" code fails...
 */
static void init_stuff(void)
{
	int i;

	short vrefnum;
	long drefnum;
	long junk;

	SFTypeList types;
	SFReply reply;

	Rect r;
	Point topleft;

	char path[1024];


	/* Fake rectangle */
	r.left = 0;
	r.top = 0;
	r.right = 344;
	r.bottom = 188;

	/* Center it */
	center_rect(&r, &qd.screenBits.bounds);

	/* Extract corner */
	topleft.v = r.top;
	topleft.h = r.left;


	/* Default to the "lib" folder with the application */
	refnum_to_name(path, app_dir, app_vol, (char*)("\plib:"));


	/* Check until done */
	while (1)
	{
		/* Prepare the paths */
		init_file_paths(path);

		/* Build the filename */
		path_build(path, 1024, ANGBAND_DIR_FILE, "news.txt");

		/* Attempt to open and close that file */
		if (0 == fd_close(fd_open(path, O_RDONLY))) break;

		/* Warning */
		plog_fmt("Unable to open the '%s' file.", path);

		/* Warning */
		plog("The Angband 'lib' folder is probably missing or misplaced.");

		/* Warning */
		plog("Please 'open' any file in any sub-folder of the 'lib' folder.");

		/* Allow "text" files */
		types[0] = 'TEXT';

		/* Allow "save" files */
		types[1] = 'SAVE';

		/* Allow "data" files */
		types[2] = 'DATA';

		/* Get any file */
		SFGetFile(topleft, "\p", NULL, 3, types, NULL, &reply);

		/* Allow cancel */
		if (!reply.good) quit(NULL);

		/* Extract textual file name for given file */
		GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);
		refnum_to_name(path, drefnum, vrefnum, (char*)reply.fName);

		/* Hack -- Remove the "filename" */
		i = strlen(path) - 1;
		while ((i > 0) && (path[i] != ':')) i--;
		if (path[i] == ':') path[i+1] = '\0';

		/* Hack -- allow "lib" folders */
		if (suffix(path, "lib:")) continue;

		/* Hack -- Remove the "sub-folder" */
		i = i - 1;
		while ((i > 1) && (path[i] != ':')) i--;
		if (path[i] == ':') path[i+1] = '\0';
	}
}


/*
 * Macintosh Main loop
 */
void main(void)
{
	EventRecord tempEvent;
	int numberOfMasters = 10;

	/* Increase stack space by 64K */
	SetApplLimit(GetApplLimit() - 65536L);

	/* Stretch out the heap to full size */
	MaxApplZone();

	/* Get more Masters */
	while (numberOfMasters--) MoreMasters();

	/* Set up the Macintosh */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	/* TEInit(); */
	InitDialogs(NULL);
	InitCursor();

	/* Flush events */
	FlushEvents(everyEvent, 0);

	/* Flush events some more (?) */
	(void)EventAvail(everyEvent, &tempEvent);
	(void)EventAvail(everyEvent, &tempEvent);
	(void)EventAvail(everyEvent, &tempEvent);


#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

# if defined(powerc) || defined(__powerc)

	/* Assume System 7 */
	
	/* Assume Color Quickdraw */

# else

	/* Block */
	if (TRUE)
	{
		OSErr err;
		long versionNumber;

		/* Check the Gestalt */
		err = Gestalt(gestaltSystemVersion, &versionNumber);

		/* Check the version */
		if ((err != noErr) || (versionNumber < 0x0700))
		{
			quit("You must have System 7 to use this program.");
		}
	}

	/* Block */
	if (TRUE)
	{
		SysEnvRec env;

		/* Check the environs */
		if (SysEnvirons(1, &env) != noErr)
		{
			quit("The SysEnvirons call failed!");
		}

		/* Check for System Seven Stuff */
		if (env.systemVersion < 0x0700)
		{
			quit("You must have System 7 to use this program.");
		}

		/* Check for Color Quickdraw */
		if (!env.hasColorQD)
		{
			quit("You must have Color Quickdraw to use this program.");
		}
	}

# endif

#endif /* ANGBAND_LITE_MAC */


#ifdef USE_SFL_CODE

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Start_UPP = NewAEEventHandlerProc(AEH_Start);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, AEH_Start_UPP,
	                      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Quit_UPP = NewAEEventHandlerProc(AEH_Quit);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, AEH_Quit_UPP,
	                      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Print_UPP = NewAEEventHandlerProc(AEH_Print);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, AEH_Print_UPP,
	                      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Open_UPP = NewAEEventHandlerProc(AEH_Open);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, AEH_Open_UPP,
	                      0L, FALSE);

#endif


	/* Find the current application */
	SetupAppDir();


#if defined(MACINTOSH) && !defined(applec)

	/* Mark ourself as the file creator */
	_fcreator = 'A271';

	/* Default to saving a "text" file */
	_ftype = 'TEXT';

#endif


#if defined(__MWERKS__)

	/* Obtian a "Universal Procedure Pointer" */
	ynfilterUPP = NewModalFilterProc(ynfilter);

#endif


	/* Hook in some "z-virt.c" hooks */
	rnfree_aux = hook_rnfree;
	ralloc_aux = hook_ralloc;
	rpanic_aux = hook_rpanic;

	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_core;



	/* Show the "watch" cursor */
	SetCursor(*(GetCursor(watchCursor)));

	/* Prepare the menubar */
	init_menubar();

	/* Prepare the windows */
	init_windows();

	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;

	/* Reset the cursor */
	SetCursor(&qd.arrow);


	/* Mega-Hack -- Allocate a "lifeboat" */
	lifeboat = NewPtr(16384);

	/* Note the "system" */
	ANGBAND_SYS = "mac";


	/* Initialize */
	init_stuff();

	/* Initialize */
	init_angband();


	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;


	/* We are now initialized */
	initialized = TRUE;


	/* Handle "open_when_ready" */
	handle_open_when_ready();


	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);

	/* Flush the prompt */
	Term_fresh();


	/* Hack -- Process Events Forever */
	while (TRUE) CheckEvents(TRUE);
}

