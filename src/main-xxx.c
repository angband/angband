/* File: main-xxx.c */

/* Purpose: Sample visual module for Angband 2.7.9 */


/*
 * This file written by "Ben Harrison (benh@voicenet.com)".
 *
 * This file is intended to show one way to build a "visual module"
 * for Angband to allow it to work with a new system.  It does not
 * actually work, but if the "XXX XXX XXX" comments were replaced
 * with functional code, then it probably would.
 *
 * See "term.c" for info on the concept of the "generic terminal"
 *
 * Basically, there are three ways to port Angband to a new system,
 * first, to modify the "main-gcu.c" file to support a version of
 * "curses" on your machine, second, to write a new "main-xxx.c"
 * file that works in a similar fashion to "main-gcu.c" but which
 * has its own "USE_XXX" define and its own "activation" code in
 * "main.c", and third, to write a new "main-xxx.c" file that works
 * in a similar manner to "main-mac.c" or "main-win.c" and which
 * replaces the "main.c" file entirely.  The second and third ways
 * are shown in this file, based on the "USE_XXX" define.  By the
 * way, if you are using the "USE_XXX" method, and it is possible
 * to include this file without "USE_XXX" being defined, then you
 * should uncomment the "#ifdef USE_XXX" below, and the "#endif"
 * at the end of the file.
 *
 * Note that the "util.c" file often contains functions which must
 * be modified in small ways for various platforms, for example,
 * the "delay()" function often needs to be rewritten.
 *
 * When you complete a port to a new system, you should email both
 * your new files (if any), and any changes you needed to make to
 * existing files, including "h-config.h", "config.h", and "Makefile",
 * to me (benh@voicenet.com) for inclusion in the next version.
 *
 * Try to stick to a "three letter" naming scheme for "main-xxx.c"
 * and "Makefile.xxx" and such for consistency.
 */


#include "angband.h"


/* #ifdef USE_XXX */


/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data {

    term		t;

    cptr		name;

    /* XXX XXX XXX */
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


#if 0	/* Fix the syntax below */

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
static color_data_type color_data[16] = {

    /* XXX XXX XXX 0,0,0 */,		/* TERM_DARK */
    /* XXX XXX XXX 4,4,4 */,		/* TERM_WHITE */
    /* XXX XXX XXX 2,2,2 */,		/* TERM_SLATE */
    /* XXX XXX XXX 4,2,0 */,		/* TERM_ORANGE */
    /* XXX XXX XXX 3,0,0 */,		/* TERM_RED */
    /* XXX XXX XXX 0,2,1 */,		/* TERM_GREEN */
    /* XXX XXX XXX 0,0,4 */,		/* TERM_BLUE */
    /* XXX XXX XXX 2,1,0 */,		/* TERM_UMBER */
    /* XXX XXX XXX 1,1,1 */,		/* TERM_L_DARK */
    /* XXX XXX XXX 3,3,3 */,		/* TERM_L_WHITE */
    /* XXX XXX XXX 4,0,4 */,		/* TERM_VIOLET */
    /* XXX XXX XXX 4,4,0 */,		/* TERM_YELLOW */
    /* XXX XXX XXX 4,0,0 */,		/* TERM_L_RED */
    /* XXX XXX XXX 0,4,0 */,		/* TERM_L_GREEN */
    /* XXX XXX XXX 0,4,4 */,		/* TERM_L_BLUE */
    /* XXX XXX XXX 3,2,1 */		/* TERM_L_UMBER */
};

#endif



/*** Special functions ***/


#ifndef USE_XXX

/*
 * XXX XXX XXX You may need an event handler here
 */

#endif

 

/*** Function hooks needed by "Term" ***/


/*
 * XXX XXX XXX Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_xxx(term *t)
{
    term_data *td = (term_data*)(t->data);

    /* XXX XXX XXX */
}



/*
 * XXX XXX XXX Nuke an old "term"
 *
 * This function is called when an old "term" is no longer needed.  It should
 * do whatever is needed to clean up before the program exits, such as wiping
 * the screen, restoring the cursor, fixing the font, etc.  Often this function
 * does nothing and lets the operating system clean up when the program quits.
 */
static void Term_nuke_xxx(term *t)
{
    term_data *td = (term_data*)(t->data);

    /* XXX XXX XXX */
}



/*
 * XXX XXX XXX Do a "user action" on the current "term"
 *
 * This function allows the visual module to do things.
 *
 * This function is currently unused, but has access to the "info"
 * field of the "term" to hold an extra argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_user_xxx(int n)
{
    term_data *td = (term_data*)(Term->data);

    /* XXX XXX XXX Handle the request */
    
    /* Unknown */
    return (1);
}


/*
 * XXX XXX XXX Do a "special thing" to the current "term"
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
static errr Term_xtra_xxx(int n, int v)
{
    term_data *td = (term_data*)(Term->data);

    /* Analyze */
    switch (n)
    {
        case TERM_XTRA_CLEAR:
        
            /* XXX XXX XXX Clear the "screen" */
            
            return (0);

        case TERM_XTRA_EVENT:
        
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
        
            /* XXX XXX XXX Flush all pending events */
            /* This action should handle all events waiting on the */
            /* queue, optionally discarding all "keypress" events, */
            /* since they will be discarded anyway in "term.c". */
            /* This action is NOT optional */
            
            return (0);

        case TERM_XTRA_FROSH:
        
            /* XXX XXX XXX Flush a row of output (optional) */
            /* This action should make sure that row "v" of the "output" */
            /* to the window will actually appear on the window.  This */
            /* action is optional, but useful for some machines. */
            
            return (0);
            
        case TERM_XTRA_FRESH:
        
            /* XXX XXX XXX Flush output (optional) */
            /* This action should make sure that all "output" to the */
            /* window will actually appear on the window.  This action */
            /* is optional if the "output" will eventually show up on */
            /* its own, or if the various functions which induce "output" */
            /* do so in such a way as to be immediately visible. */
            
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
        
            /* XXX XXX XXX Make a noise (optional) */
            /* This action should produce a "beep" noise.   It is */
            /* optional, but will make the program better */
            
            return (0);

        case TERM_XTRA_SOUND:
        
            /* XXX XXX XXX Make a sound (optional) */
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
 * XXX XXX XXX Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_xxx(int x, int y, int n)
{
    term_data *td = (term_data*)(Term->data);

    /* XXX XXX XXX Erase the block of characters */

    /* Success */
    return (0);
}


/*
 * XXX XXX XXX Display the cursor
 *
 * This routine should display the cursor at the given location
 * (x,y) in some manner.  On some machines this involves actually
 * moving the physical cursor, on others it involves drawing a fake
 * cursor in some form of graphics mode.  Note the "software_cursor"
 * flag which tells "term.c" to treat the "cursor" as a "visual" thing
 * and not as a "hardware" cursor.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You may use the "Term_grab(x, y, &a, &c)" function, if needed,
 * to determine what attr/char should be "under" the new cursor,
 * for "inverting" purposes or whatever.
 */
static errr Term_curs_xxx(int x, int y)
{
    term_data *td = (term_data*)(Term->data);

    /* XXX XXX XXX Display a cursor (see above) */

    /* Success */
    return (0);
}


/*
 * XXX XXX XXX Draw a "picture" on the screen
 *
 * This routine should display a "picture" (with index "p") at the
 * given location (x,y).  This function is currently unused, but in
 * the future will be used to support special "graphic" characters.
 */
static errr Term_pict_xxx(int x, int y, int p)
{
    term_data *td = (term_data*)(Term->data);

    /* XXX XXX XXX Draw a "picture" */

    /* Success */
    return (0);
}


/*
 * XXX XXX XXX Display some text on the screen
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
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * You may ignore the "color" parameter if you are only supporting
 * a monochrome environment, since this routine is never called to
 * display "black" (invisible) text.
 */
static errr Term_text_xxx(int x, int y, int n, byte a, cptr s)
{
    term_data *td = (term_data*)(Term->data);

    /* XXX XXX XXX Use color "color_data[a & 0x0F]" */
    
    /* XXX XXX XXX Draw the string */

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
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 */
static void term_data_link(term_data *td)
{
    term *t = &td->t;

    /* Initialize the term */
    term_init(t, 80, 24, 256);

    /* XXX XXX XXX Choose "soft" or "hard" cursor */
    /* A "soft" cursor must be explicitly "drawn" by the program */
    /* while a "hard" cursor has some "physical" existance and is */
    /* moved whenever text is drawn on the screen.  See "term.c" */
    t->soft_cursor = TRUE;

    /* XXX XXX XXX Choose whether "event scanning" should be done */
    /* at various times.  This is normally true if you have an event */
    /* loop, and false if you only handle keypress checking */
    t->scan_events = TRUE;

    /* Prepare the init/nuke hooks */
    t->init_hook = Term_init_xxx;
    t->nuke_hook = Term_nuke_xxx;

    /* Prepare the template hooks */
    t->user_hook = Term_user_xxx;
    t->xtra_hook = Term_xtra_xxx;
    t->wipe_hook = Term_wipe_xxx;
    t->curs_hook = Term_curs_xxx;
    t->pict_hook = Term_pict_xxx;
    t->text_hook = Term_text_xxx;

    /* Remember where we came from */
    t->data = (vptr)(td);

    /* Activate it */
    Term_activate(t);
}



#ifdef USE_XXX

/*
 * A "normal" system uses "main.c" for the "main()" function, and
 * simply adds a call to "init_xxx()" to that function, conditional
 * on some form of "USE_XXX" define.
 */
 
/*
 * XXX XXX XXX Initialization function
 */
void init_xxx(void)
{
    term_data *td;


    /* XXX XXX XXX Initialize the system */


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

 
#else /* USE_XXX */

/*
 * Some special machines need their own "main()" function, which they
 * can provide here, making sure NOT to compile the "main.c" file.
 *
 * These systems usually have some form of "event loop", run forever
 * as the last step of "main()", which handles things like menus and
 * window movement, and calls "play_game(FALSE)" to load a game after
 * initializing "savefile" to a filename, or "play_game(TRUE)" to make
 * a new game.  The event loop would also be triggered by "Term_xtra()"
 * (the TERM_XTRA_EVENT action), in which case the event loop would not
 * actually "loop", but would run once and return.
 */


/*
 * Init some stuff
 *
 * This function is needed to keep the "path" variable off the stack.
 */
static void init_stuff(void)
{
    char path[1024];

    /* XXX XXX XXX Prepare the path */
    /* This must in some way prepare the "path" variable */
    /* so that it points at the "lib" directory.  Every machine */
    /* handles this in a different way */
    strcpy(path, "XXX XXX XXX");
    
    /* Prepare the filepaths */
    init_file_paths(path);
}


/*
 * Main function
 */
void main(void)
{
    term_data *td;


    /* XXX XXX XXX Initialize the machine */

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


    /* Initialize some stuff */
    init_stuff();

    /* Display the "news" screen */
    show_news();

    /* Initialize some arrays */
    init_some_arrays();

    /* No name (yet) */
    strcpy(player_name, "");

    /* XXX XXX XXX Hack -- assume wizard permissions */
    can_be_wizard = TRUE;

    /* XXX XXX XXX Hack -- Use the "pref-xxx.prf" file */
    ANGBAND_SYS = "xxx";

 
    /* XXX XXX XXX Event loop forever */
    while (TRUE) /* Handle Events */;
}

#endif /* USE_XXX */

#endif
