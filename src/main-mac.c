/* File: main-mac.c */

/* Purpose: Simple support for MACINTOSH Angband */

/*
 * This file should only be compiled with the "Macintosh" version
 *
 * This file written by "Ben Harrison (benh@voicenet.com)".
 *
 * Some code adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * Maarten Hazewinkel (mmhazewi@cs.ruu.nl) provided the initial support
 * for compilation under the CodeWarrior compiler, including support for
 * compiling on the Power Macintosh.  He has since developed a special
 * "main-mac-cw.c" file and a resource file which create an executable
 * version of Angband using his own favorite interface design choices.
 * His version uses 'MAng' as the file type, calls itself "MacAngband",
 * has special icons (etc), and works on pre-System-7 machines.
 *
 * Steve Linberg (slinberg@crocker.com) provided code which allows
 * double-clicked savefiles to open correctly on the Power Macintosh
 * (via a skeletal AppleEvent structure), and also proper use of the
 * "FindFolder()" function (see USE_SFL_CODE and USE_FIND_FOLDER).
 *
 * See "term.c" for info on the concept of the "generic terminal"
 *
 * Note that, when compiling under Think C 6.0, that this file is the
 * only one that requires "Non-ANSI" extensions to C, in particular,
 * the "Think C Language Extensions" must be on, and the "enums are
 * always ints" must be off, both to allow "Events.h" to compile.
 *
 * This file was seriously updated for Angband 2.7.9, to remove obsolete
 * code, to adapt to some changes in Angband, and to verify compilation
 * under both Think C 6.0 and CodeWarrior 7. -BEN-
 *
 * The preference file is now a text file named "Angband preferences".
 *
 * Note that the "preference" files from "different" versions are now
 * ignored, and the preference file name no longer contains the version,
 * though the version information is contained in the file itself.
 *
 * Note that "init.c", "save-old.c", and "birth.c" should probably be
 * "unloaded" as soon as they have completed execution, to save space.
 *
 * XXX XXX XXX The first "ClipRect()" call crashes if the user closes
 * all the windows, switches to another application, switches back,
 * then re-opens the main window, for example, using "command-a".
 */


/*
 * The Angband Color Set (0 to 15):
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 *
 * On the Macintosh, we have two possibilities, chosen by the "has_color"
 * variable, which is chosen based on the operating system potential.
 *
 * When true, we have color quickdraw, and we use actual "RGB" values
 * to choose the 16 colors.
 *
 * When false, we assume a monochrome machine, and simply draw everything
 * in white (letting "term.c" convert "black" to "wipe" calls).
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
 *   MBAR 128 = menu bar
 *
 *   MENU 128 = apple menu (about, -, ...)
 *   MENU 129 = File menu (new, open, close, save, -, exit, quit)
 *   MENU 130 = Edit menu (undo, -, cut, copy, paste, clear)
 *   MENU 131 = Font menu (bold, wide, -, monaco, courier, ...)
 *   MENU 132 = Size menu (9, 10, 11, 12, 13, 14, 15, 16, 17, 18)
 *   MENU 133 = Window menu (Angband, Mirror, Recall, Choice)
 *   MENU 134 = Special menu (use_sound, use_graphics, use_xxx1, use_xxx2,
 *                            arg_fiddle, arg_wizard, arg_xxx1, arg_xxx2)
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
 *   GestaltEqu.h = gestalt code
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
 *
 * For backwards compatibility:
 *   GestaltEqu.h simply includes Gestalt.h
 *   Desk.h simply includes ???, Devices.h, Events.h
 */


#include "angband.h"

#include <Types.h>
#include <GestaltEqu.h>
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
#include <Desk.h>
#include <Devices.h>
#include <Events.h>
#include <Resources.h>
#include <Controls.h>
#include <SegLoad.h>
#include <Memory.h>


/*
 * Select some special code for compilation, this will work under
 * CodeWarrior 7, and Think C 8.0, and perhaps others as well.
 */

#if defined(__MWERKS__)

/*
 * Use the special "AppleEventHandler" code
 */
# define USE_SFL_CODE

/*
 * Attempt to use the "FindFolder()" function
 */
# define USE_FIND_FOLDER

#endif



#ifdef USE_SFL_CODE

/*
 * Include the "apple event" header files
 */

#include <AppleEvents.h>

#include <EPPC.h>

/*
 * Apple Even Flag -- open savefile
 */
Boolean open_when_ready = FALSE;

/*
 * Apple Event Flag -- open savefile
 */
Boolean quit_when_ready = FALSE;

#endif


#ifdef USE_FIND_FOLDER

/*
 * Include the "Folders.h" header file, if necessary
 */
#include <Folders.h>

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

	cptr		s;

	bool		oops;
	bool		fake;

	s16b		keys;

	s16b		rows;
	s16b		cols;

	s16b		size_wid;
	s16b		size_hgt;

	s16b		size_ow1;
	s16b		size_oh1;
	s16b		size_ow2;
	s16b		size_oh2;

	s16b		mapped;
	s16b		font_id;
	s16b		font_size;
	s16b		font_face;

	s16b		font_wid;
	s16b		font_hgt;
	s16b		font_o_x;
	s16b		font_o_y;
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
 * Hack -- game in progress
 */
static int game_in_progress = 0;


/*
 * We can use color quickdraw
 */
static bool has_color = FALSE;


/*
 * We can use System 7 Stuff
 */
static bool has_seven = FALSE;


/*
 * Only do "SetPort()" when needed
 */
static WindowPtr active = NULL;



/*
 * Maximum number of windows
 */
#define MAX_TERM_DATA 8

/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];



/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;



/*
 * Actual "RGB" data for color quickdraw
 */
static RGBColor mac_clr[16] =
{
	{0, 0, 0},				/* TERM_DARK */
	{65535, 65535, 65535},	/* TERM_WHITE */
	{32768, 32768, 32768},	/* TERM_SLATE */
	{65535, 32768, 0},		/* TERM_ORANGE */
	{49152, 0, 0},			/* TERM_RED */
	{0, 32768, 16384},		/* TERM_GREEN */
	{0, 0, 65535},			/* TERM_BLUE */
	{32768, 16384, 0},		/* TERM_UMBER */
	{16384, 16384, 16384},	/* TERM_L_DARK */
	{49152, 49152, 49152},	/* TERM_L_WHITE */
	{65535, 0, 65535},		/* TERM_VIOLET */
	{65535, 65535, 0},		/* TERM_YELLOW */
	{65535, 0, 0},			/* TERM_L_RED */
	{0, 65535, 0},			/* TERM_L_GREEN */
	{0, 65535, 65535},		/* TERM_L_BLUE */
	{49152, 32768, 16384}	/* TERM_L_UMBER */
};




#if defined(__MWERKS__)

/*
 * CodeWarrior uses Universal Procedure Pointers
 */
static ModalFilterUPP ynfilterUPP;

# ifdef USE_SFL_CODE

/*
 * Apple Event Hooks
 */
AEEventHandlerUPP AEH_Start_UPP;
AEEventHandlerUPP AEH_Quit_UPP;
AEEventHandlerUPP AEH_Print_UPP;
AEEventHandlerUPP AEH_Open_UPP;

# endif

#else

/*
 * Think C 6.0 uses normal functions.
 */
#define ynfilterUPP ynfilter

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


#if defined(USE_SFL_CODE) || defined(USE_FIND_FOLDER)


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






/*** Hooks for the "term.c" functions ***/

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
}


/*
 * Hack -- react to a new size
 */
static void term_data_resize(term_data *td)
{
	/* Actually resize the window */
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);

	/* Redraw Contents */
	term_data_redraw(td);

	/* No need to redraw */
	ValidRect(&td->w->portRect);
}








/*** Function hooks needed by "Term" ***/


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

	Str255 title;

	/* Extract the title */
	strcpy((char*)(title+1), td->s);
	title[0] = strlen(td->s);

	/* Make the window */
	if (has_color)
	{
		td->w = NewCWindow(0, &td->r, title, td->mapped, documentProc, (WindowPtr)-1, 1, 0L);
	}
	else
	{
		td->w = NewWindow(0, &td->r, title, td->mapped, documentProc, (WindowPtr)-1, 1, 0L);
	}

	/* Activate the window */
	activate(td->w);

	/* Hack -- set "mapped" flag */
	t->mapped_flag = td->mapped;

	/* Erase behind words */
	TextMode(srcCopy);

	/* Activate the font */
	TextFont(td->font_id);
	TextSize(td->font_size);
	TextFace(td->font_face);


	/* Prepare the colors (real colors) */
	if (has_color)
	{
		RGBBackColor(&mac_clr[TERM_DARK]);
		RGBForeColor(&mac_clr[TERM_WHITE]);
	}

	/* Prepare the colors (base colors) */
	else
	{
		BackColor(blackColor);
		ForeColor(whiteColor);
	}


	/* Activate the color if needed */
	if (has_color) RGBForeColor(&mac_clr[TERM_WHITE]);


	/* Clip to the window */
	ClipRect(&td->w->portRect);

	/* Erase the window */
	EraseRect(&td->w->portRect);

	/* Invalidate the window */
	InvalRect(&td->w->portRect);
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
 * React to changes
 */
static errr Term_xtra_mac_react(void)
{
	int i;

	/* Check colors */
	if (has_color)
	{
		u16b rv, gv, bv;

		/* Grab "color_table" */
		for (i = 0; i < 16; i++)
		{
			/* Extract the R,G,B data */
			rv = color_table[i][1];
			gv = color_table[i][2];
			bv = color_table[i][3];

			/* Save the new R,G,B values */
			mac_clr[i].red = (rv | (rv << 8));
			mac_clr[i].green = (gv | (gv << 8));
			mac_clr[i].blue = (bv | (bv << 8));
		}
	}

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

			/* Activate the color if needed */
			if (has_color) RGBForeColor(&mac_clr[TERM_WHITE]);

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

	/* Activate the color if needed */
	if (has_color) RGBForeColor(&mac_clr[TERM_YELLOW]);

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
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_mac(int x, int y, int n)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Erase the block of characters */
	r.left = x * td->font_wid + td->size_ow1;
	r.right = r.left + n * td->font_wid;
	r.top = y * td->font_hgt + td->size_oh1;
	r.bottom = r.top + td->font_hgt;
	EraseRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * Hack -- attempt to allow non-mono-spaced fonts at low expense.
 */
static errr Term_text_mac(int x, int y, int n, byte a, cptr s)
{
	int xp, yp;

	term_data *td = (term_data*)(Term->data);

	/* Activate the color if needed */
	if (has_color) RGBForeColor(&mac_clr[a & 0x0F]);

	/* Starting pixel */
	xp = x * td->font_wid + td->font_o_x + td->size_ow1;
	yp = y * td->font_hgt + td->font_o_y + td->size_oh1;

	/* Move to the correct location */
	MoveTo(xp, yp);

	/* Draw the character */
	if (n == 1) DrawChar(*s);

	/* Draw the string */
	else DrawText(s, 0, n);

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
	td->t->xtra_hook = Term_xtra_mac;
	td->t->curs_hook = Term_curs_mac;
	td->t->wipe_hook = Term_wipe_mac;
	td->t->text_hook = Term_text_mac;

	/* Link the local structure */
	td->t->data = (vptr)(td);

	/* Activate it */
	Term_activate(td->t);

	/* Global pointer */
	ang_term[i] = td->t;

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
 * Hack -- Extract the "font sizing" information
 */
static void term_data_check_font(term_data *td)
{
	Rect r;
	WindowPtr tmpw;
	FontInfo info;

	WindowPtr old = active;

	/* Fake window */
	r.left = r.right = r.top = r.bottom = 0;
	tmpw = NewWindow(0, &r, "\p", false, documentProc, 0, 0, 0);

	/* Activate the "fake" window */
	activate(tmpw);

	/* Prepare the font */
	TextFont(td->font_id);
	TextSize(td->font_size);
	TextFace(td->font_face);

	/* Extract the font info */
	GetFontInfo(&info);

	/* Extract the font sizing values XXX XXX XXX */
	td->font_wid = CharWidth('@'); /* info.widMax; */
	td->font_hgt = info.ascent + info.descent;
	td->font_o_x = 0;
	td->font_o_y = info.ascent;

	/* Assume mono-spaced */
	td->fake = FALSE;

	/* Hack -- notice non-mono-space */
	if (CharWidth('i') != td->font_wid) td->fake = TRUE;
	if (CharWidth('1') != td->font_wid) td->fake = TRUE;

	/* Mega-Hack -- no width at all */
	if (td->font_wid < 1) td->fake = TRUE;

	/* Mega-Hack -- enforce some width */
	if (td->font_wid < 1) td->font_wid = 8;

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
	/* Minimal size */
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

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

	/* Default name */
	td->s = "Window";

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
		/* Obtain */
		td = &data[i];

		/* Defaults */
		term_data_hack(td);

		/* Title it */
		td->s = ang_term_name[i];

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

#ifdef USE_FIND_FOLDER

	/* System 7 */
	if (has_seven)
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

#endif /* FIND_FOLDER */

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
		td = &data[i];

		/* Check the font */
		term_data_check_font(td);

		/* Check the size */
		term_data_check_size(td);

		/* Require mapped */
		if (!td->mapped) continue;

		/* Link the term */
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


#ifdef USE_FIND_FOLDER

	/* System 7 */
	if (has_seven)
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

#endif /* FIND_FOLDER */

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
 * Set up the menus
 *
 * Font menu creation from "Maarten Hazewinkel"
 */
static void init_menubar(void)
{
	int i, n;
	Rect r;
	WindowPtr tmpw;

	MenuHandle menu;

	/* Show the "watch" cursor */
	/* SetCursor(*(GetCursor(watchCursor))); */

	/* Get the menubar from the resource file */
	SetMenuBar(GetNewMBar(128));

	/* Get the "apple" menu */
	menu = GetMenu(128);

	/* Oops */
	if (!menu) quit(NULL);

	/* Add the DA's to the "apple" menu */
	AddResMenu(menu, 'DRVR');

	/* Insert the "apple" menu as the first menu */
	InsertMenu(menu, 129);

	/* Get the "Font" menu */
	menu = GetMenu(131);

	/* Oops */
	if (!menu) quit(NULL);

	/* Add the fonts to the menu */
	AddResMenu(menu, 'FONT');

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

	/* Size of menu */
	n = CountMItems(menu);

	/* Scan the menu */
	for (i = n; i >= 4; i--)
	{
		Str255 tmpName;
		short fontNum;

		/* Acquire the font name XXX XXX XXX */
		/* GetMenuItemText(menu, i, tmpName); */
		GetItem(menu, i, tmpName);

		/* Acquire the font index */
		GetFNum(tmpName, &fontNum);

		/* Apply the font index */
		TextFont(fontNum);

		/* Remove non-mono-spaced fonts */
		if ((CharWidth('i') != CharWidth('W')) || (CharWidth('W') == 0))
		{
			/* Delete the menu item XXX XXX XXX */
			/* DeleteMenuItem(menu, i); */
			DelMenuItem(menu, i);
		}
	}

	/* Destroy the old window */
	DisposeWindow(tmpw);

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(menu, 1, bold); */

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(menu, 2, extend); */

	/* Update the menu bar */
	DrawMenuBar();

	/* Reset the cursor */
	/* SetCursor(&qd.arrow); */
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
			Term_xtra(TERM_XTRA_DELAY, 100);
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
 * Prepare the menus
 *   File (129) = { New,Open,Import,Close,Save,-,Exit,Quit }
 *   Edit (130) = { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) = { Bold, Extend, -, Monaco, Courier, ... }
 *   Size (132) = { 9, 10, ..., 18 }
 *   Window (133) = { Angband, Mirror, Recall, Choice }
 *   Special (134) = { use_sound, use_graphics, use_xxx1, use_xxx2,
 *                     arg_fiddle, arg_wizard, arg_xxx1, arg_xxx2 }
 */
static void setup_menus(void)
{
	int i;
	MenuHandle m;
	short fnum, fsize;
	Str255 s;

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

	/* Nothing is legal */
	for (i = 1; i <= CountMItems(m); i++)
	{
		/* Disable everything */
		DisableItem(m, i);
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

	/* Nothing is legal */
	for (i = 1; i <= CountMItems(m); i++)
	{
		/* Disable everything */
		DisableItem(m, i);
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

	/* Nothing is legal (or checked) */
	for (i = 1; i <= CountMItems(m); i++)
	{
		/* Disable everything */
		DisableItem(m, i);

		/* Un-Check everything */
		CheckItem(m, i, FALSE);
	}

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

		/* Check the appropriate "Font" for the current window */
		for (i = 4; i <= CountMItems(m); i++)
		{
			/* Enable it */
			EnableItem(m, i);

			/* Analyze font */
			/* GetMenuItemText(m,i,s); */
			GetItem(m, i, s);
			GetFNum(s, &fnum);

			/* Check active font */
			if (td->font_id == fnum) CheckItem(m, i, TRUE);
		}
	}


	/* Size menu */
	m = GetMHandle(132);

	/* Nothing is legal (or checked) */
	for (i = 1; i <= CountMItems(m); i++)
	{
		/* Disable everything */
		DisableItem(m, i);

		/* Un-Check everything */
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (td)
	{
		/* Process the "size" options */
		for (i=1; i<=CountMItems(m); i++)
		{
			/* Analyze size */
			/* GetMenuItemText(m,i,s); */
			GetItem(m, i, s);
			s[s[0]+1] = '\0';
			fsize = atoi((char*)(s+1));

			/* Enable the "real" sizes */
			if (RealFont(td->font_id, fsize)) EnableItem(m, i);

			/* Check the current size */
			if (td->font_size == fsize) CheckItem(m, i, TRUE);
		}
	}


	/* Windows menu */
	m = GetMHandle(133);

	/* Check windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Check if needed */
		CheckItem(m, i + 1, data[i].mapped);
	}


	/* Special menu */
	m = GetMHandle(134);

	/* Item "use_sound" */
	CheckItem(m, 1, use_sound);

	/* Item "use_graphics" */
	CheckItem(m, 2, use_graphics);

	/* Item "use_xxx1" */
	DisableItem(m, 3);

	/* Item "use_xxx2" */
	DisableItem(m, 4);

	/* Item "arg_fiddle" */
	CheckItem(m, 5, arg_fiddle);

	/* Item "arg_wizard" */
	CheckItem(m, 6, arg_wizard);

	/* Item "arg_xxx1" */
	DisableItem(m, 7);

	/* Item "arg_xxx2" */
	DisableItem(m, 8);
}


/*
 * Process a menu selection (see above)
 *
 * Hack -- assume that invalid menu selections are disabled above
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
		/* Apple Menu {About,-,...} */
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

		/* File Menu {New,Open,Import,Close,Save,-,Exit,Quit} */
		case 129:
		{
			switch (selection)
			{
				case 1:		/* New */
				{
					do_menu_file_new();
					break;
				}

				case 2:		/* Open... */
				{
					do_menu_file_open(FALSE);
					break;
				}

				case 3:		/* Import... */
				{
					do_menu_file_open(TRUE);
					break;
				}

				case 4:		/* Close */
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

				case 5:		/* Save */
				{
					/* Hack -- Forget messages */
					msg_flag = FALSE;

					/* Hack -- Save the game */
					do_cmd_save_game();

					break;
				}

				case 7:		/* Exit (without save) */
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

				case 8:		/* Quit (with save) */
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

		/* Edit menu {Undo,-,Cut,Copy,Paste,Clear} */
		case 130:
		{
			/* Unused */
			break;
		}

		/* Font menu {Bold,Extend,-,...} */
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
				if (td->font_face & extend)
				{
					td->font_face &= ~extend;
				}
				else
				{
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
			/* GetMenuItemText(GetMHandle(131), selection, s); */
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

			/* Restore the window */
			activate(old_win);

			break;
		}

		/* Size menu {...} */
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

			/* Instantiate the new size */
			TextSize(td->font_size);

			/* React to the font */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize the window */
			term_data_resize(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* Window menu {Angband,Mirror,Recall,Choice,etc} */
		case 133:
		{
			/* Parse */
			i = selection - 1;

			/* Check legality of choice */
			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			/* Obtain the window */
			td = &data[i];

			/* Allocate */
			if (!td->t)
			{
				/* Mapped */
				td->mapped = TRUE;

				/* Link */	
				term_data_link(i);
			}

			/* Show */
			else
			{
				/* Mapped */
				td->mapped = TRUE;

				/* Mapped */
				td->t->mapped_flag = TRUE;

				/* Show the window */
				ShowWindow(td->w);
			}

			/* Bring to the front */
			SelectWindow(td->w);

			break;
		}

		/* Special menu {u_s, u_g, u_1, u_2, a_f, a_w, a_1, a_2} */
		case 134:
		{
			switch (selection)
			{
				case 1:
				{
					use_sound = !use_sound;
					break;
				}

				case 2:
				{
					use_graphics = !use_graphics;
					break;
				}

				case 5:
				{
					arg_fiddle = !arg_fiddle;
					break;
				}

				case 6:
				{
					arg_wizard = !arg_wizard;
					break;
				}
			}

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


/*
 * This function will allow us to process "open" apple events received before
 * we have completed our initialization process.  Personally, I tend to have
 * the opposite problem, in that the apple event is received even later.
 */
static void check_for_save_file(void)
{
	/* Check the flag */
	if (open_when_ready)
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


#else


/*
 * Check for "open via double click on a savefile"
 *
 * For some reason, this function seems to function correctly
 * with Think C 6.0 but not with CodeWarrior 7.  In the latter
 * case, the CountAppFiles() function loads bizarre values.
 */
static void check_for_save_file(void)
{
	int i;
	short message, n;
	AppFile fileinfo;
	OSErr err;
	short vrefnum;
	long drefnum, junk;

	/* Access startup info */
	CountAppFiles(&message, &n);

	/* Only open files */
	if (message != appOpen) n = 0;

	/* Scan for a savefile */
	for (i = 1; i <= n; i++)
	{
		/* Access the file */
		GetAppFiles(i, &fileinfo);

		/* Only handle savefiles */
		if (fileinfo.fType != 'SAVE') continue;

		/* Look at the file */
		err = GetWDInfo(fileinfo.vRefNum, &vrefnum, &drefnum, &junk);

		/* Load it and play */
		if (err == noErr)
		{
			/* Load "savefile" */
			refnum_to_name(savefile, drefnum, vrefnum, (char*)fileinfo.fName);

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

		/* Oops */
		else
		{
			/* Warning */
			mac_warning("Could not open save file");
		}

		/* Stop looking */
		break;
	}

	/* Forget the files */
	for (i = 1; i <= n; i++) ClrAppFiles(i);
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

					/* Check size */
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
					r.left = 20 * td->font_wid + td->size_ow1;
					r.right = 80 * td->font_wid + td->size_ow1 + td->size_ow2 + 1;
					r.top = 1 * td->font_hgt + td->size_oh1;
					r.bottom = 24 * td->font_hgt + td->size_oh1 + td->size_oh2 + 1;

					/* Grow the rectangle */
					newsize = GrowWindow(w, event.where, &r);

					/* Handle abort */
					if (!newsize) break;

					/* Extract the new size in pixels */
					y = HiWord(newsize) - td->size_oh1 - td->size_oh2;
					x = LoWord(newsize) - td->size_ow1 - td->size_ow2;

					/* Extract a "close" approximation */
					td->rows = y / td->font_hgt;
					td->cols = x / td->font_wid;

					/* React to the new size */
					term_data_check_size(td);

					/* Activate */
					Term_activate(td->t);

					/* Hack -- Resize the term */
					Term_resize(td->cols, td->rows);

					/* Resize the window */
					term_data_resize(td);

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

			/* Hack -- handle "quit" event */
			if (quit_when_ready)
			{
				/* Forget */
				quit_when_ready = FALSE;

				/* Do the menu key */
				menu(MenuKey('q'));

				/* Turn off the menus */
				HiliteMenu(0);
			}

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

	/* Dispose */
	DisposePtr(v);

	/* Success */
	return (0);
}

/*
 * Hook to "allocate" memory
 */
static vptr hook_ralloc(huge size)
{
	/* Make a new pointer */
	return (NewPtr(size));
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
	int i;


	/* Increase stack space by 64K */
	SetApplLimit(GetApplLimit() - 65536L);

	/* Stretch out the heap to full size */
	MaxApplZone();

	/* Set up the Macintosh */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	/* TEInit(); */
	InitDialogs(0);
	InitCursor();


#if defined(powerc) || defined(__powerc)

	/* Assume many things */
	has_seven = has_color = TRUE;

#else

	/* Block */
	if (TRUE)
	{
		SysEnvRec env;

		/* Check the environs */
		if (SysEnvirons(1, &env) == noErr)
		{
			/* Check for Color Quickdraw */
			if (env.hasColorQD) has_color = TRUE;

			/* Check for System Seven Stuff */
			if (env.systemVersion >= 0x0700) has_seven = TRUE;
		}
	}

#endif


#ifdef USE_SFL_CODE

	/* System 7 */
	if (has_seven)
	{
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
	}

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


	/* Prepare the menubar */
	init_menubar();

	/* Prepare the windows */
	init_windows();


	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;


	/* Hack -- extract the "color_table" data */
	for (i = 0; i < 16; i++)
	{
		/* Extract the R,G,B values */
		color_table[i][1] = (mac_clr[i].red >> 8);
		color_table[i][2] = (mac_clr[i].green >> 8);
		color_table[i][3] = (mac_clr[i].blue >> 8);
	}


	/* Initialize some stuff */
	init_stuff();

	/* Display the "news" screen */
	show_news();

	/* Initialize some arrays */
	init_some_arrays();

	/* Hack -- assume wizard permissions */
	can_be_wizard = TRUE;

	/* Hack -- Use the "pref-mac.prf" file */
	ANGBAND_SYS = "mac";

	/* Mega-Hack -- Allocate a "lifeboat" */
	lifeboat = NewPtr(16384);


	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;


	/* We are now initialized */
	initialized = TRUE;


	/* Check for double-clicked save file */
	check_for_save_file();


	/* Prompt the user */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);

	/* Flush the prompt */
	Term_fresh();


	/* Hack -- Process Events Forever */
	while (TRUE) CheckEvents(TRUE);
}

