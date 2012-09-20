/* File: main-amy.c */

/* Purpose: Visual support for Angband 2.7.9 on Amiga */

/*
 * Author: Andrew Thomson <Andy@andysami.demon.co.uk>
 *
 * Warning: Email conversion has broken several lines in this file.
 *
 * Warning: This file is not complete at this time...
 *
 * It requires OS 2.00 or above and iff.library v18+.
 *
 * See "term.c" for info on the "generic terminal" that we support.
 */

#include "angband.h"

#ifdef USE_AMY

#include "amiga.h"

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <libraries/iff.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <stdio.h>

#include "term.h"


static UWORD __chip MPointer[8] =
{
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000
};

static struct TextAttr my_font =
{
        "topaz.font",
        TOPAZ_EIGHTY,
        FS_NORMAL,
        FPF_ROMFONT
};

/* Main background screen */
static struct NewScreen NewScreen =
{
        0,                             /* LeftEdge */
        0,                             /* TopEdge */
        640,                           /* Width */
        200,                           /* Height */
        4,                             /* No. Bitplanes */
        0, 1,                          /* DetailPen, BlockPen */
        HIRES,                         /* ViewModes */
        CUSTOMSCREEN,                  /* Screen type */
        &my_font,                      /* my font */
        "Amiga screen",                /* Screen Title */
        (struct Gadget *) NULL,        /* Gadget list */
        (struct BitMap *) NULL         /* custom bitmap */
};

/* Main background window */
static struct NewWindow NewWindow =
{
        0,                             /* Left edge */
        0,                             /* Top edge */
        640,                           /* Width */
        200,                           /* Height */
        -1, -1,                        /* Pens */
        RAWKEY | MOUSEBUTTONS,         /* IDCMP flags */
        ACTIVATE | BORDERLESS | RMBTRAP,        /* Window flags */
        NULL,                          /* First gadget */
        NULL,                          /* Image data */
        NULL,                          /* Title */
        NULL,                          /* Pointer to screen structure */
        NULL,                          /* Super BitMap ? */
        0, 0, 0, 0,                    /* MIN/MAX sizing */
        CUSTOMSCREEN                   /* Type of screen */
};

WORD ColourTable[16];

struct Library *IFFBase = NULL;
struct Library *KeymapBase = NULL;
struct Library *IntuitionBase = NULL;
struct Library *GfxBase = NULL;

static IFFL_HANDLE gfxfile = NULL;

#define GFX_FILE    "AngbandGFX.iff"

static struct Screen *AmigaScreen = NULL;
static struct Window *AmigaWindow = NULL;

static struct RastPort *RPort;
static struct ViewPort *VPort;

static unsigned char Flags;            /* Global flags */
static struct IOStdReq ioreq;
static struct MsgPort *port;
static ULONG device;

struct BitMap gfxdatabm;

/* gfxdatasmalllbm is not used yet */

struct BitMap gfxdatasmallbm;

struct IMAGE {
    int width, height;
    int bitplanes;
};

static struct IMAGE GfxData =
{512, 136, 4};                        /* Width must be multiple of 16 */


#define NCOLOURS 16

int LINES = 24, COLS = 80;             /* Defaults */

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data {

    term        t;

    cptr        name;

    /* Amiga OS2.0 - have not worked it out yet */
    /* Other fields if needed */
};


/*
 * One "term_data" for each "window"
 *
 * XXX XXX XXX The only "window" which you MUST support is the
 * main "term_screen" window, the rest are optional.  If you only
 * support a single window, then most of the junk involving the
 * "term_data" structures is actually not needed, since you can
 * use global variables.  But you should avoid global variables
 * when possible as a general rule...
 */
static term_data screen;
static term_data mirror;
static term_data recall;
static term_data choice;


#if 0   /* Fix the syntax below */

/*
 * XXX XXX XXX The "color" array for the visual module
 *
 * This table should be used in whetever way is necessary to
 * convert the Angband Color Indexes into the proper "color data"
 * for the visual system.  On the Macintosh, these are arrays of
 * three shorts, on the IBM, these are combinations of the eight
 * basic color codes with optional "bright" bits, on X11, these
 * are actual "pixel" codes extracted from another table which
 * contains textual color names.
 *
 * The Angband Color Set (0 to 15):
 *   Black, White, Slate, Orange,    Red, Blue, Green, Umber
 *   D-Gray, L-Gray, Violet, Yellow, L-Red, L-Blue, L-Green, L-Umber
 *
 * Colors 8 to 15 are basically "enhanced" versions of Colors 0 to 7.
 *
 * As decribed in one of the header files, in a perfect world, the
 * colors below should fit a nice clean "quartered" specification
 * in RGB codes, but this must often be Gamma Corrected.  The 1/4
 * parts of each Red,Green,Blue are shown in the comments below,
 * again, these values are *before* gamma correction.
 */

/* The clour table of the Amiga is read in from the AngbandGFX iff
 * file and the colours contained in the 16 registers are as listed
 * below so this is left commented out.
 */

static color_data_type color_data[16] = {

    /* XXX XXX XXX 0,0,0 */,        /* TERM_DARK */
    /* XXX XXX XXX 4,4,4 */,        /* TERM_WHITE */
    /* XXX XXX XXX 2,2,2 */,        /* TERM_SLATE */
    /* XXX XXX XXX 4,2,0 */,        /* TERM_ORANGE */
    /* XXX XXX XXX 3,0,0 */,        /* TERM_RED */
    /* XXX XXX XXX 0,2,1 */,        /* TERM_GREEN */
    /* XXX XXX XXX 0,0,4 */,        /* TERM_BLUE */
    /* XXX XXX XXX 2,1,0 */,        /* TERM_UMBER */
    /* XXX XXX XXX 1,1,1 */,        /* TERM_L_DARK */
    /* XXX XXX XXX 3,3,3 */,        /* TERM_L_WHITE */
    /* XXX XXX XXX 4,0,4 */,        /* TERM_VIOLET */
    /* XXX XXX XXX 4,4,0 */,        /* TERM_YELLOW */
    /* XXX XXX XXX 4,0,0 */,        /* TERM_L_RED */
    /* XXX XXX XXX 0,4,0 */,        /* TERM_L_GREEN */
    /* XXX XXX XXX 0,4,4 */,        /* TERM_L_BLUE */
    /* XXX XXX XXX 3,2,1 */         /* TERM_L_UMBER */
};

#endif



/*** Special functions ***/


#ifndef USE_AMY

putgfx(x, y, c)
  int x, y, c;
{
    ULONG gx, gy;

    gx = c % 64;                       /* get corresponding position of gfx of character */
    gy = c / 64;
    if (c == '@') {
        gx = 1;
        gy = 7;
    }
    BltBitMap(&gfxdatabm, gx * 8, gy * 8, RPort->BitMap,
            x * 8, y * 8, 8, 8, 0xC0, 0xFF);
}

/* Handle messages
 *
 * So far this routine only displays the input on stdout as RAW
 * codes and ANSI sequences.
 */

handle_IDCMP()
{
    int i;
    int ret = OK;
    UBYTE buffer[8];
    APTR *eventptr;
    struct IntuiMessage *message;
    struct InputEvent inputevent = {0};

    /* Initialise InputEvent structure - already cleared to 0 */
    inputevent.ie_Class = IECLASS_RAWKEY;

    while (NULL != (message = (struct IntuMessage *)GetMsg(AmigaWindow->UserPort)))
        {
            switch(message->Class)
                {
                case IDCMP_RAWKEY:
                    printf("Qualifier ");
                    if (message->Qualifier & IEQUALIFIER_LSHIFT)    printf("LShift ");
                    if (message->Qualifier & IEQUALIFIER_RSHIFT)    printf("RShift ");
                    if (message->Qualifier & IEQUALIFIER_CAPSLOCK)  ret = ERR;
                    if (message->Qualifier & IEQUALIFIER_CONTROL)   printf("Control ");
                    if (message->Qualifier & IEQUALIFIER_LALT)      printf("LAlt ");
                    if (message->Qualifier & IEQUALIFIER_RALT)      printf("RAlt ");
                    if (message->Qualifier & IEQUALIFIER_LCOMMAND)  printf("LAmiga ");
                    if (message->Qualifier & IEQUALIFIER_RCOMMAND)  printf("RAmiga ");
                    if (message->Qualifier & IEQUALIFIER_NUMERICPAD) printf("NumPad ");
                    if (message->Qualifier & IEQUALIFIER_REPEAT)    printf("Repeat ");
                    if (message->Qualifier & IEQUALIFIER_INTERRUPT) printf("Interrupt ");
                    printf(" Code=$%02x\n",message->Code);

                    inputevent.ie_Code = message->Code;
                    inputevent.ie_Qualifier = message->Qualifier;
                    eventptr = message->IAddress;
                    inputevent.ie_EventAddress = *eventptr;

                    /* Rawkey to ANSI */

                    i = MapRawKey(&inputevent, buffer, 8, NULL);
                    if(i == -1)
                        printf("Overflow in conversion\n");
                    else
                    {
                        Write(Output(), "MAPS TO: ",9);
                        Write(Output(), buffer, i);
                        printf("\n");
                    }
                    break;
                case IDCMP_MOUSEBUTTONS:
                    switch(message->Code)
                        {
                        case SELECTUP:
                            printf("Select up at %d, %d\n",message->MouseX,message->MouseY);
                            break;
                        case SELECTDOWN:
                            printf("Select down at %d, %d\n",message->MouseX,message->MouseY);
                            break;
                        case MENUUP:
                            printf("Menu up\n");
                            break;
                        case MENUDOWN:
                            printf("Menu down\n");
                            break;
                        default:
                            printf("Unknown code\n");
                            break;
                        }
                    break;
                default:
                    printf("Unknown IDCMP Message\n");
                    break;
                }
            ReplyMsg((struct Message *)message);
            }
        return(ret);
        }

void clear()
{
    SetAPen(RPort, 0);
    SetDrMd(RPort, JAM2);
    RectFill(RPort, 0, 0, 639, 199);
    WaitBlit();
}

void clrtoeol(row, col)
  int row, col;
{
    SetAPen(RPort, 0);
    SetDrMd(RPort, JAM2);
    RectFill(RPort, col * 8, row * 8, 639, row * 8 + 7);
    WaitBlit();
}

amiga_bell()
{
    DisplayBeep(AmigaScreen);
}

#endif



/*** Function hooks needed by "Term" ***/


/*
 * Amiga OS2.0 Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_amy(term *t)
{
    term_data *td = (term_data*)(t->data);

    return(0)
}


/*
 * Amiga OS2.0 Nuke an old "term"
 *
 */
static void Term_nuke_amy(term *t)
{
    term_data *td = (term_data*)(t->data);

    CloseWindow(AmigaWindow);
    CloseScreen(AmigaScreen);

    if (gfxfile) {
        IFFL_CloseIFF(gfxfile);
        gfxfile = NULL;
    }
    if (IFFBase) {
        CloseLibrary(IFFBase);
        IFFBase = NULL;
    }
    if (KeymapBase) {
        CloseLibrary(KeymapBase);
        KeymapBase = NULL;
    }
    if (GfxBase) {
        CloseLibrary(GfxBase);
        GfxBase = NULL;
    }
    if (IntuitionBase) {
        CloseLibrary(IntuitionBase);
        IntuitionBase = NULL;
    }
}



/*
 * Amiga OS2.0 Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "term.c" package.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 *
 * The most important action is the "TERM_XTRA_EVENT" action, without which
 * there is no way to interact with the user.  Make sure that this action
 * works as expected, or various nasty things will happen at various times.
 */
static errr Term_xtra_amy(int n, int v)
{
    term_data *td = (term_data*)(Term->data);
    int i;

    /* Analyze */
    switch (n)
    {
        /* XXX XXX XXX */
        case TERM_XTRA_CLEAR:
            xxx xxx xxx;
            return (0);

        case TERM_XTRA_EVENT:
            signals = Wait(1L << AmigaWindow->UserPort->mp_SigBit);
            if (signals & (1L << AmigaWindow->UserPort->mp_SigBit))
                i = handle_IDCMP();

            /* XXX XXX XXX Process some pending events */
            /* Wait for at least one event if "v" is non-zero */
            /* otherwise, if no events are ready, return at once. */
            /* When "keypress" events are encountered, the "ascii" */
            /* value corresponding to the key should be sent to the */
            /* "Term_keypress()" function.  Certain "bizarre" keys, */
            /* such as function keys or arrow keys, may send special */
            /* sequences of characters, such as control-underscore, */
            /* plus letters corresponding to modifier keys, plus an */
            /* underscore, plus carriage return, which can be used by */
            /* the main program for "macro" triggers.  This action */
            /* should handle as many events as is efficiently possible */
            /* but is only required to handle a single event, and then */
            /* only if one is ready or "v" is true */

            return (0);

        case TERM_XTRA_FLUSH:
            handle_IDCMP();

            /* XXX XXX XXX Flush all pending events */
            /* This action should handle all events waiting on the */
            /* queue, optionally discarding all "keypress" events, */
            /* since they will be discarded anyway in "term.c". */
            /* This action is NOT optional */
            /* I need to add an option to the above function to */
            /* discard events. */
            return (0);

        case TERM_XTRA_FRESH:

            /* Amiga OS2.0 Flush output */
            /* The "output" from the functions defined will go directly */
            /* to the window so this case is defined as do nothing */

            return (0);

        case TERM_XTRA_INVIS:

            /* XXX XXX XXX Make cursor invisible (optional) */
            /* This action should hide the visual cursor, if possible */
            /* This action is optional, but if used can improve both */
            /* the efficiency and attractiveness of the program */

            return (0);

        case TERM_XTRA_BEVIS:

            /* XXX XXX XXX Make cursor visible (optional) */
            /* This action should show the visual cursor, if possible */
            /* This action is optional, but if used can improve both */
            /* the efficiency and attractiveness of the program. */

            return (0);

        case TERM_XTRA_NOISE:
            amiga_bell();
            return (0);

        case TERM_XTRA_SOUND:

            /* Amiga OS2.0 Make a sound (optional) */
            /* This action is optional and still in beta test */

            return (0);

        case TERM_XTRA_ALIVE:

            /* XXX XXX XXX Change the "hard" level (optional) */
            /* This action is used if the program is "suspended" */
            /* or "resumed", with a "v" value of "FALSE" or "TRUE" */
            /* This action is optional unless the computer uses the */
            /* same "physical screen" for multiple programs, in which */
            /* case this action should clean up to let other programs */
            /* use the screen, or resume from such a cleaned up state */
            /* This action is currently only used on UNIX machines */

            return (0);

        case TERM_XTRA_LEVEL:

            /* XXX XXX XXX Change the "soft" level (optional) */
            /* This action is used when the term window changes "activation" */
            /* either by becoming inactive ("v" is FALSE) or active ("v" is */
            /* TRUE).  This action is optional but often does things like */
            /* activates the proper font or drawing mode for the newly active */
            /* term window.  This action should NOT change which window has */
            /* the "focus" or which window is "raised" or anything like that. */
            /* This action is completely optional if all the other things which */
            /* depend on what term is active explicitly check to be sure the */
            /* proper term is activated interally first. */

            return (0);
    }

    /* Unknown or Unhandled action */
    return (1);
}



/*
 * XXX XXX XXX Display the cursor
 *
 * This routine should display the cursor at the given location
 * (x,y) in some manner.  On some machines this involves actually
 * moving the physical cursor, on others it involves drawing a fake
 * cursor in some form of graphics mode.  In either case, the "z"
 * parameter is TRUE if the cursor should be visible, and FALSE if
 * it should be invisible.  This information is available elsewhere,
 * but it may be simpler to handle it here.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You may use the "Term_grab(x, y, &a, &c)" function, if needed,
 * to determine what attr/char should be "under" the new cursor,
 * for "inverting" purposes or whatever.
 *
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_curs_amy(int x, int y)
{
    term_data *td = (term_data*)(Term->data);
    struct Border b;
    WORD b_data[] =
        {
        0,0, 7,0, 7,7, 0,7, 0,0
        };

    /* Cursor is done as a yellow "box" */
    b.FrontPen = TERM_YELLOW;
    b.DrawMode = JAM1;
    b.Count = 5;
    b.XY = b_data;
    b.NextBorder = NULL;

    /* Frame the grid */
    b.LeftEdge = 0;
    b.TopEdge = 0;

    /* Save Graphic block */
    BltBitMap(RPort->BitMap, x * 8, y * 8, &gfxdatabm,
              504, 128, 8, 8, 0xC0, 0xFF);

    /* Draw border */
    DrawBorder(RPort,&b,(x * 8),(y * 8));

    /* Success */
    return (0);
}


/*
 * Amiga OS2.0 Erase a block of characters
 *
 * This function should actually erase a block of characters on
 * the screen, starting at the given (x,y) location and extending
 * "n" characters wide.
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_amy(int x, int y, int n)
{
    term_data *td = (term_data*)(Term->data);
    UBYTE oldapen;

    oldapen = RPort->FgPen;
    SetAPen(RPort, 0);
    RectFill(RPort, x * 8, y * 8, ((x+n) * 8) - 1, ((y+1) * 8) - 1);
    SetAPen(RPort, oldapen);

    /* Success */
    return (0);
}


/*
 * Amiga OS2.0 Display some text on the screen
 *
 * This function should actually display a string of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which is terminated
 * with a nul character and which has exactly "n" characters.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_amy()" to clear the area.
 *
 * You may ignore the "color" parameter if you are only supporting
 * a monochrome environment, since this routine is never called to
 * display "black" (invisible) text.
 */
static errr Term_text_amy(int x, int y, int n, byte a, cptr s)
{
    term_data *td = (term_data*)(Term->data);
    struct IntuiText text;
    UBYTE attr;
    int i;

    attr = (a & 0x7f);

    /* Hack -- Stop illigal characters */
    if (attr > 16) attr = TERM_RED;

    /* Clear cells before print */
    Term_wipe_amy(x, y, n);

    if (a < 0x80)
        {
        /* Write out text sting s */

        text.FrontPen = attr;
        text.BackPen = 0;
        text.DrawMode = JAM1;
        text.LeftEdge = 0;
        text.TopEdge = 0;
        text.ITextFont = NULL;  /* Use screen font */
        text.IText = s;
        text.NextText = NULL;   /* No next text */
        /* Print text */
        PrintIText(RPort,&text,x * 8,y * 8);
        }
        else
        {
        /* Print gfx characters in s */

        for(i = 1; i <= n; i++)
            {
            putgfx(x,y,(*s + (0x80 * attr)));
            x++;
            s++;
            }
         }

    /* Success */
    return (0);
}




/*
 * XXX XXX XXX Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.
 *
 * Note that "activation" calls the "Term_init_amy()" hook for
 * the "term" structure, if needed.
 */
static void term_data_link(term_data *td)
{
    term *t = &td->t;

    /* Initialize the term */
    term_init(t, 80, 24, 256);

    t->soft_cursor = TRUE;

    t->scan_events = TRUE;

    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_amy
    t->nuke_hook = Term_nuke_amy

    /* Prepare the template hooks */
    t->xtra_hook = Term_xtra_amy
    t->curs_hook = Term_curs_amy
    t->wipe_hook = Term_wipe_amy
    t->text_hook = Term_text_amy

    /* Remember where we came from */
    t->data = (vptr)(td);

    /* Activate it */
    Term_activate(t);
}


/*
 * Amiga OS2.0 Initialization function
 *
 * Called by "main()" in "main.c".
 */
void init_amy(void)
{
    term_data *td;


    /* Amiga OS2.0 Initialize the system */

    int i;
    struct IFFL_BMHD *bmhd;

    IntuitionBase = (struct Library *) OpenLibrary("intuition.library",INT_VERSION);

    GfxBase = (struct Library *) OpenLibrary("graphics.library",GFX_VERSION);

    IFFBase = (struct Library *) OpenLibrary("iff.library",IFF_VERSION);

    KeymapBase = (struct Library *) OpenLibrary("keymap.library",KEYMAP_VERSION);

    AmigaScreen = (struct Screen *) OpenScreen(&NewScreen);

    RPort = &(AmigaScreen->RastPort);
    VPort = &(AmigaScreen->ViewPort);

    SetDrMd(RPort, JAM2);
    SetAPen(RPort, 1);

    NewWindow.Screen = AmigaScreen;

    AmigaWindow = (struct Window *) OpenWindow(&NewWindow);

    SetPointer(AmigaWindow, MPointer, 2, 16, 0, 0);

    gfxfile = IFFL_OpenIFF(GFX_FILE, IFFL_MODE_READ);
    bmhd = IFFL_GetBMHD(gfxfile);
    IFFL_GetColorTab(gfxfile, ColourTable);
    LoadRGB4(VPort,ColourTable,16);

    gfxdatabm.BytesPerRow = GfxData.width / 8;
    gfxdatabm.Rows = GfxData.height;
    gfxdatabm.Flags = 0;
    gfxdatabm.Depth = 4;
    for (i = 0; i < 4; i++) {
        gfxdatabm.Planes[i] = (PLANEPTR) AllocRaster(GfxData.width, GfxData.height);
        if (gfxdatabm.Planes[i] == NULL)
            fprintf(stderr, "Not enough memory to allocate bitmap!\n");
    }
    IFFL_DecodePic(gfxfile, &gfxdatabm);

    /* Recall window */
    td = &recall;
    WIPE(td, term_data);
    td->name = "Recall";
    /* XXX XXX XXX Extra stuff */
    term_data_link(&recall);
    term_recall = &recall.t;

    /* Choice window */
    td = &choice;
    WIPE(td, term_data);
    td->name = "Choice";
    /* XXX XXX XXX Extra stuff */
    term_data_link(&choice);
    term_choice = &choice.t;

    /* Mirror window */
    td = &mirror;
    WIPE(td, term_data);
    td->name = "Mirror";
    /* XXX XXX XXX Extra stuff */
    term_data_link(&mirror);
    term_mirror = &mirror.t;

    /* Screen window */
    td = &screen;
    WIPE(td, term_data);
    td->name = "Screen";
    /* XXX XXX XXX Extra stuff */
    term_data_link(&screen);
    term_screen = &screen.t;
}


#endif /* USE_AMY */
