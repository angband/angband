/* File: main-acn.c */

/* Purpose: Support for Acorn RISC OS Angband */

/*
 * Author: Kevin Bracey (kevin@iota.co.uk)
 *
 */

/* Check compiler flag */
#ifdef __riscos

/*
 * === Instructions for compiling Angband for RISC OS ===
 *
 * You will require:
 *   Acorn C/C++ (some other compiler may be suitable but you may
 *                need to make changes to the make file)
 *   OSLib 5.1 or later (available from ftp.acorn.co.uk)
 *   The RISC OS Angband binary distribution (for Resource files etc).
 *
 * Full compilation instructions are provided with the binary distribution,
 * along with a useful little utility to process the source tree into
 * RISC OS form, and bug fixes for OSLib.
 *
 */

#define VERSION "2.7.9v4 (12-Feb-96)"

/* Hack to prevent types clash from OSLib */
typedef unsigned int bits;
#define NONE 0u
#define UNKNOWN 1
#define types_H

#include "angband.h"

#include <signal.h>
#include "kernel.h"

#include "osbyte.h"
#include "osword.h"
#include "buffer.h"
#include "colourtrans.h"
#include "osargs.h"
#include "osfind.h"
#include "osfile.h"
#include "osgbpb.h"
#include "osfscontrol.h"

#include "wimpspriteop.h"

#include "menu.h"
#include "saveas.h"
#include "quit.h"
#include "proginfo.h"
#include "actionbutton.h"

/* Header for eventlib (using OSLib types) */
typedef int event_wimp_handler(wimp_event_no event_code,
      wimp_block *event,toolbox_block *id,
      void *handle);

typedef int event_toolbox_handler(bits event_code,
      toolbox_action *event, toolbox_block *id,
      void *handle);

typedef int event_message_handler(wimp_message *message,
      void *handle);

extern os_error *event_poll(wimp_event_no *event_code, wimp_block *poll_block,
      void *poll_word);

extern os_error *event_set_mask(wimp_poll_flags mask);
extern os_error *event_get_mask(wimp_poll_flags *mask);

extern os_error *event_register_wimp_handler(toolbox_o object_id,
    wimp_event_no event_code, event_wimp_handler *handler, void *handle);

extern os_error *event_register_toolbox_handler(toolbox_o object_id,
    wimp_event_no event_code, event_toolbox_handler *handler, void *handle);

extern os_error *event_register_message_handler(bits msg_no,
    event_message_handler *handler, void *handle);

extern os_error *event_deregister_toolbox_handler(toolbox_o object_id,
    wimp_event_no event_code, event_toolbox_handler *handler, void *handle);

extern os_error *event_deregister_message_handler(bits msg_no,
    event_message_handler *handler, void *handle);

extern os_error *event_initialise(toolbox_block *id_block);


/* Structure to store all info about a window */
typedef struct
{
    term t;
    toolbox_o window;
    wimp_w wimp;
    char olddisp[24][80];
    char oldcol[24][80];
    byte froshed[24];
    int curs_vis;
    int curs_x, curs_y;
    int ocx, ocy;
    os_box changed;
} term_data;

/* Files opened using open(2) - make sure they're closed on exit */
static int filehandle[16];
static int openfiles;

static int wimpver;

/* XXX Not ready yet */
/*static int file_dragged;*/

/*
 * Hack -- correct spelling
 */
#define colour_table color_table

/*
 * Hack -- game in progress
 */
static int initialised;

/*
 * Hack -- game in progress
 */
static int game_in_progress;

/*
 * Hack -- give saved game files the correct type
 */
#define osfile_TYPE_ANGBAND 0x118u
static bits _ftype;

/* Un-hack remove etc (which have been redirected here for name translation) */
#undef remove
#undef rename

/* Colour of cursor (displayed as a box) */
#define TERM_CURSOR 255

/* This is the palette as defined in term.h, with gamma
   correction of 1.3 (it seems to look OK) */
static const os_PALETTE(16) default_palette =
{
  {
    0x00000000,   /* TERM_DARK */
    0xffffff00,   /* TERM_WHITE */
    0x96969600,   /* TERM_SLATE */
    0x0096ff00,   /* TERM_ORANGE */
    0x0000cc00,   /* TERM_RED */
    0x58960000,   /* TERM_GREEN */
    0xff000000,   /* TERM_BLUE */
    0x00589600,   /* TERM_UMBER */
    0x58585800,   /* TERM_L_DARK */
    0xcccccc00,   /* TERM_L_WHITE */
    0xff00ff00,   /* TERM_VIOLET */
    0x00ffff00,   /* TERM_YELLOW */
    0x0000ff00,   /* TERM_L_RED */
    0x00ff0000,   /* TERM_L_GREEN */
    0xffff0000,   /* TERM_L_BLUE */
    0x5896cc00,   /* TERM_L_UMBER */
  }
};

static os_PALETTE(256) palette;

static int coltable[256];
static int solid_colours;
static unsigned ncolours;

/* Toolbox events */
#define action_Quit 1
#define action_Iconbar 2
#define action_Save 3
#define action_New 4
#define action_OpenRecall 5
#define action_OpenChoice 6
#define action_OpenAngband 7
#define action_DefaultCols 8
#define action_SaveChoices 9
#define action_OpenMirror 10

/* Menu entries */
/* Game menu */
#define menu_New ((toolbox_c) 0)
#define menu_Save ((toolbox_c) 1)

/* Colours menu */
#define menu_Dithered ((toolbox_c) 0x100)

/* Windows menu */
#define menu_Angband ((toolbox_c) 0)
#define menu_Recall ((toolbox_c) 2)
#define menu_Choice ((toolbox_c) 3)
#define menu_Mirror ((toolbox_c) 4)

/* Missing from OSLib */
#define event_ANY ((toolbox_o) -1)

/* Are we shutting down the desktop? */
static wimp_t shutting_down;

/* Do we have the caret? */
static int have_caret;
static int have_caret_entity;

/* Flag set by handlers to indicate a keypress */
static int key_pressed;
static int escape_pressed;

static messagetrans_control_block mfd;
static toolbox_block id_block;

static term_data recall;
static term_data choice;
static term_data screen;
static term_data mirror;

/* Forward references */
static int palette_handler(wimp_message *message, void *handle);
static void update_window(term_data *t, int x0, int y0, int x1, int y1);
static void refresh_windows(void);
static errr init_acn(void);
static const char *translate_name(const char *path);
static void savechoices(void);
static void refresh_window(term_data *t);



/*************************************************************
 *                                                           *
 *              Error reporting (lots of it!)                *
 *                                                           *
 *************************************************************/

/*
 * Hook to tell the user something important
 */
static void plog_hook(cptr str)
{
    os_error e;

    e.errnum=1;
    strcpy(e.errmess, str);
    wimp_report_error_by_category(&e,
                wimp_ERROR_BOX_OK_ICON |
                wimp_ERROR_BOX_CATEGORY_INFO << wimp_ERROR_BOX_CATEGORY_SHIFT,
                "Angband", "!angband", wimpspriteop_AREA, 0);
}

/*
 * Hook to tell the user something, and then quit
 */
static void quit_hook(cptr str)
{
    /* Warning if needed */
    if (str)
    {
        os_error e;

        e.errnum=1;
        strcpy(e.errmess, str);
        if (wimpver >= 322)
            wimp_report_error_by_category(&e,
                        wimp_ERROR_BOX_CATEGORY_ERROR << wimp_ERROR_BOX_CATEGORY_SHIFT,
                        "Angband", "!angband", wimpspriteop_AREA, "Quit");
        else
            wimp_report_error(&e, wimp_ERROR_BOX_CANCEL_ICON, "Angband");
    }

    /* Oh yeah, close the high score list */
    /*nuke_scorefile();*/

    /* All done */
    exit(0);
}

/*
 * Hook to tell the user something, and then crash
 */
static void core_hook(cptr str)
{
    wimp_error_box_selection sel;
    os_error e;
    const static char *apology="Sorry, Angband has suffered an internal error and "
                               "must close down immediately. I will attempt to save "
                               "your game.";

    e.errnum=1;

    if (wimpver >= 322)
    {
        e.errnum=1;
        strcpy(e.errmess, apology);
        sel=wimp_report_error_by_category(&e,
                wimp_ERROR_BOX_CATEGORY_PROGRAM << wimp_ERROR_BOX_CATEGORY_SHIFT,
                "Angband", "!angband", wimpspriteop_AREA, str?"Quit,Describe":"Quit");
        if (sel==4) /* User pressed Describe */
        {

            e.errnum=1;
            strcpy(e.errmess, str);

            wimp_report_error_by_category(&e,
                        wimp_ERROR_BOX_CATEGORY_PROGRAM << wimp_ERROR_BOX_CATEGORY_SHIFT,
                        "Angband", "!angband", wimpspriteop_AREA, "Quit");
        }
    }
    else
    {
        e.errnum=1;
        strcpy(e.errmess, str);
        wimp_report_error(&e, wimp_ERROR_BOX_CANCEL_ICON, "Angband");
    }

    if (game_in_progress && character_generated)
    	save_player();

    /* Just quit */
    quit(NULL);
}

static void oserror_handler(int sig)
{
    core(_kernel_last_oserror()->errmess);
}

static int error_handler(bits event_code, toolbox_action *event,
                         toolbox_block *id, void *handle)
{
    wimp_report_error_by_category((os_error *) &event->data.error,
        wimp_ERROR_BOX_OK_ICON |
        wimp_ERROR_BOX_CATEGORY_ERROR << wimp_ERROR_BOX_CATEGORY_SHIFT,
        "Angband",
        "!angband",
        wimpspriteop_AREA,
        0);

    return 1;
}

/*************************************************************
 *                                                           *
 *             Miscellaneous support functions               *
 *                                                           *
 *************************************************************/


static void window_to_front(wimp_w w)
{
    union
    {
        wimp_open           open;
        wimp_window_state   state;
    } a;

    a.state.w=w;
    wimp_get_window_state(&a.state);
    a.open.next=wimp_TOP;
    wimp_open_window(&a.open);
}

static void window_hide(wimp_w w)
{
    union
    {
        wimp_open           open;
        wimp_window_state   state;
    } a;

    a.state.w=w;
    wimp_get_window_state(&a.state);
    a.open.next=wimp_HIDDEN;
    wimp_open_window(&a.open);
}

static void grabcaret(void)
{
    wimp_set_caret_position(screen.wimp, wimp_ICON_WINDOW, 0, 0, (1<<25), 0);
}

/*
 * This general-purpose routine will work for objects of class Window, DCS,
 * FileInfo, FontDbox, PrintDbox, ProgInfo, Quit, SaveAs, Scale, and indeed
 * any other object that supports Toolbox_MiscOp 0 as XXX_GetWindowId, and
 * for which Toolbox_ShowObject_FullSpec takes the same block as the Window
 * class.
 */

static void ShowCentred(toolbox_show_flags flags, toolbox_o obj,
                        toolbox_o parent_obj, toolbox_c parent_cmp)
{
    wimp_window_state state;
    int width, height, scrwidth, scrheight, xeig, yeig;
    toolbox_o window;
    toolbox_class objclass;
    toolbox_position pos;

    objclass=toolbox_get_object_class(NONE, obj);
    if (objclass==class_WINDOW)
      window=obj;
    else
     window=quit_get_window_id(NONE, obj);

    state.w=window_get_wimp_handle(NONE, window);

    wimp_get_window_state(&state);

    width=state.visible.x1-state.visible.x0;
    height=state.visible.y1-state.visible.y0;

    os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XWIND_LIMIT, &scrwidth);
    os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YWIND_LIMIT, &scrheight);
    os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XEIG_FACTOR, &xeig);
    os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YEIG_FACTOR, &yeig);

    scrwidth+=1; scrheight+=1;
    scrwidth<<=xeig; scrheight<<=yeig;

    pos.top_left.x=(scrwidth-width)/2;
    pos.top_left.y=(scrheight+height)/2;

    toolbox_show_object(flags, obj, toolbox_POSITION_TOP_LEFT, &pos,
                        parent_obj, parent_cmp);
}

/*************************************************************
 *                                                           *
 *              Interfaces to the Term package               *
 *                                                           *
 *************************************************************/
static errr Term_xtra_acn_check(int v)
{
    static os_t last_poll;
    int buf, bufhi;
    os_t t;
    wimp_poll_flags mask;

    /*
     * We only do an event_poll when a character is in the keyboard
     * buffer, or every 0.1 seconds. This speeds up running.
     */

    os_byte(osbyte_BUFFER_OP, 255 - buffer_KEYBOARD, 0, &buf, &bufhi);

    buf=(buf & 0xFF) + (bufhi << 8);

    if (buf > 0 && have_caret || (t=os_read_monotonic_time())-last_poll >= 10)
    {
        /* If buf > 0, probably didn't set t in the line above */
        if (buf > 0)
            t=os_read_monotonic_time();
        last_poll=t;
        event_get_mask(&mask);
        event_set_mask(mask &~ wimp_MASK_NULL);
        event_poll(0, 0, 0);
        event_set_mask(mask);
    }

    /* This is necessary to let the user interrupt the borg */
    if (key_pressed)
    {
        key_pressed=0;
        return 0;
    }

    return 1;
}

static errr Term_xtra_acn_event(int v)
{
    while (!key_pressed)
        if (event_poll(0, 0, 0))
            return -1;

    key_pressed=0;

    return 0;
}

/*
 * React to changes
 */
static errr Term_xtra_acn_react(void)
{
    int i;
    os_colour temp;

    /* Check colours */

    /* Grab "colour_table" */
    for (i = 0; i < 256; i++)
    {
        /* Extract the R,G,B data */
        temp = (colour_table[i][3] << 24) |
               (colour_table[i][2] << 16) |
               (colour_table[i][1] << 8);

        if (temp != palette.entries[i])
        {
            /* Need to force refresh of all characters of that colour(!) */

            if (i!=0)
            {
            	int x, y;
                for (y=0; y<24; y++)
                    for (x=0; x<80; x++)
                    {
                        if (screen.t.old->a[y][x] == i)
                            screen.oldcol[y][x]=0;
                        if (recall.t.old->a[y][x] == i)
                            recall.oldcol[y][x]=0;
                        if (choice.t.old->a[y][x] == i)
                            choice.oldcol[y][x]=0;
                        if (mirror.t.old->a[y][x] == i)
                            mirror.oldcol[y][x]=0;
                    }
            }
            else
            {
                memset(screen.olddisp, 0, sizeof screen.olddisp);
                memset(recall.olddisp, 0, sizeof recall.olddisp);
                memset(choice.olddisp, 0, sizeof choice.olddisp);
                memset(mirror.olddisp, 0, sizeof mirror.olddisp);
            }

            palette.entries[i]=temp;
        }
    }

    palette_handler(0, 0);

    update_window(&screen, 0, 0, 80, 24);
    update_window(&recall, 0, 0, 80, 24);
    update_window(&choice, 0, 0, 80, 24);
    update_window(&mirror, 0, 0, 80, 24);

    memset(screen.froshed, 1, sizeof screen.froshed);
    memset(recall.froshed, 1, sizeof recall.froshed);
    memset(choice.froshed, 1, sizeof choice.froshed);
    memset(mirror.froshed, 1, sizeof mirror.froshed);

    refresh_windows();

    /* Success */
    return 0;
}

static void cursor(term *t, int on)
{
    term_data *td=(term_data *) t;

    if (td->curs_vis != on && td->ocx == -1)
    {
        td->ocx=td->curs_x;
        td->ocy=td->curs_y;
    }
    td->curs_vis=on;
    update_window(td, td->curs_x, td->curs_y,
                      td->curs_x+1, td->curs_y+1);
    td->froshed[td->curs_y]=1;
}

errr Term_xtra_acn(int n, int v)
{
    term_data *t;

    switch (n)
    {
      case TERM_XTRA_CLEAR:
        t=(term_data *) Term;
        update_window(t, 0, 0, t->t.old->w, t->t.old->h);
        return 0;

      case TERM_XTRA_EVENT:
        if (v)
            return Term_xtra_acn_event(v);
        else
            return Term_xtra_acn_check(v);

      case TERM_XTRA_FLUSH:
        if (have_caret)
            osbyte(osbyte_FLUSH_BUFFER, buffer_KEYBOARD, 0);
        return 0;

      case TERM_XTRA_FRESH:
        refresh_window((term_data *) Term);
        /*refresh_windows();*/
        return 0;

      case TERM_XTRA_FROSH:
        t=(term_data *) Term;
        t->froshed[v]=1;
        return 0;

      case TERM_XTRA_INVIS:
        cursor(Term, 0);
        return 0;

      case TERM_XTRA_BEVIS:
        cursor(Term, 1);
        return 0;

      case TERM_XTRA_NOISE:
        os_bell();
        return 0;

      case TERM_XTRA_REACT:
        return Term_xtra_acn_react();

      default:
        return 1;
    }
}

static errr Term_curs_acn(int x, int y)
{
    term_data *t = (term_data *) Term;
    int oldx = t->curs_x;
    int oldy = t->curs_y;

    t->curs_x = x;
    t->curs_y = y;

    if (!t->curs_vis)
    	return 0;

    if ((x != oldx || y != oldy))
    {
        update_window(t, oldx, oldy, oldx+1, oldy+1);
        t->froshed[oldy]=1;

        if (t->ocx == -1)
        {
            t->ocx = oldx;
            t->ocy = oldy;
        }
    }

    update_window(t, t->curs_x, t->curs_y, t->curs_x+1, t->curs_y+1);
    t->froshed[t->curs_y]=1;

    return 0;
}

errr Term_wipe_acn(int x, int y, int n)
{
    update_window((term_data *) Term, x, y, x+n, y+1);

    return 0;
}

static errr Term_text_acn(int x, int y, int n, byte a, cptr s)
{
    update_window((term_data *) Term, x, y, x+n, y+1);

    return 0;
}

void Term_init_acn(term *t)
{
    term_data *term=(term_data *)t;

    term->ocx=-1;
    term->changed.x0=256;
    term->changed.y0=256;
    term->changed.x1=0;
    term->changed.y1=0;
    memset(term->olddisp, ' ', sizeof(term->olddisp));
    memset(term->oldcol, TERM_WHITE, sizeof(term->oldcol));
}


/*************************************************************
 *                                                           *
 *                File loading and saving                    *
 *                                                           *
 *************************************************************/

static int dataload_handler(wimp_message *message, void *handle)
{
    if (message->data.data_xfer.file_type != osfile_TYPE_ANGBAND)
        return 0;

    /*
     * Note - we send a DataLoadAck _before_ loading the file. This
     * may not work if it is an inter-application transfer, as the
     * other application will delete the file on the next Wimp_Poll
     * we do. But this doesn't matter as we don't support inter-
     * application transfer (inwards), anyway.
     */

    message->action=message_DATA_LOAD_ACK;
    message->your_ref=message->my_ref;
    wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

    if (game_in_progress)
    {
        plog("You can't load a new game while you're still playing!");
        return 1;
    }

    strcpy(savefile, translate_name(message->data.data_xfer.file_name));

    window_to_front(screen.wimp);
    grabcaret();

    game_in_progress=1;
    flush();
    play_game(FALSE);
    quit(NULL);

    return 1;
}

static int showsave_handler(bits event_code, toolbox_action *event,
                            toolbox_block *id, void *handle)
{
    saveas_set_file_name(NONE, id->this_obj, translate_name(savefile));
    saveas_set_file_type(NONE, id->this_obj, osfile_TYPE_ANGBAND);
    saveas_set_file_size(NONE, id->this_obj, 50000); /* Rough guess */

    return 1;
}

static int savebutton_handler(bits event_code, toolbox_action *event,
                              toolbox_block *id, void *handle)
{
    if (event->flags & actionbutton_SELECTED_CANCEL)
        if (event->flags & actionbutton_SELECTED_ADJUST)
            saveas_set_file_name(NONE, (toolbox_o) handle, translate_name(savefile));

    return 1;
}

static int save_handler(bits event_code, toolbox_action *event,
                        toolbox_block *id, void *handle)
{
    saveas_action_save_to_file *save=(saveas_action_save_to_file *)&event->data;
    char tempsavefile[256];
    strcpy(tempsavefile, savefile);

    strcpy(savefile, translate_name(save->file_name));

    msg_flag=FALSE;

    do_cmd_save_game();

    saveas_file_save_completed(1, id->this_obj, save->file_name);

    strcpy(savefile, tempsavefile);

    return 1;
}

static int defaultsave_handler(bits event_code, toolbox_action *event,
                               toolbox_block *id, void *handle)
{
    msg_flag=FALSE;

    do_cmd_save_game();

    return 1;
}

static int savecomplete_handler(bits event_code, toolbox_action *event,
                                toolbox_block *id, void *handle)
{
    saveas_action_save_completed *save=(saveas_action_save_completed *)&event->data;

    if (event->flags & saveas_SAVE_SAFE)
        strcpy(savefile, translate_name(save->file_name));

    return 1;
}

#if 0
static int loadfile_handler(wimp_message *message, void *handle)
{
    strcpy(handle, translate_name(message->data.data_xfer.file_name));
    file_dragged=1;

    return 1;
}

static int savefile_handler(bits event_code, toolbox_action *event,
                            toolbox_block *id, void *handle)
{
    saveas_action_save_to_file *save=(saveas_action_save_to_file *)&event->data;
    func_bool saver=(func_bool) handle;
    errr e;
    char buf[256];

    strcpy(buf, translate_name(save->file_name));

    /* In case saver wants to display messages */
    grabcaret();

    msg_flag=FALSE;

    e=saver(buf);

    xsaveas_file_save_completed(e==0, id->this_obj, save->file_name);

    if (e)
        escape_pressed=1;

    return 1;
}

static int cancelsavefile_handler(bits event_code, toolbox_action *event,
                                  toolbox_block *id, void *handle)
{
    escape_pressed=1;

    return 1;
}

static int endsavefile_handler(bits event_code, toolbox_action *event,
                               toolbox_block *id, void *handle)
{
    file_dragged=1;

    return 1;
}

static errr loadfile_hook(cptr defname, int row, func_errr loader)
{
    char ourbuf[256];

    Term_putstr(0, row, -1, TERM_YELLOW, "[Drag a file to the window]");

    Term_fresh();

    event_register_message_handler(message_DATA_LOAD, loadfile_handler, ourbuf);

    file_dragged=escape_pressed=0;

    while (!file_dragged && !escape_pressed)
        event_poll(0, 0, 0);

    event_deregister_message_handler(message_DATA_LOAD, loadfile_handler, ourbuf);

    if (escape_pressed)
        return 1;

    return loader(ourbuf);
}

static errr savefile_hook(cptr defname, int row, func_errr saver)
{
    toolbox_o savebox;
    union
    {
        toolbox_position pos;
        wimp_pointer pointer;
    } mouse;

    Term_fresh();

    savebox=toolbox_create_object(NONE, (toolbox_id)"SaveFile");

    saveas_set_file_name(NONE, savebox, translate_name(defname));
    saveas_set_file_size(NONE, savebox, 1024); /* This'll do */

    event_register_toolbox_handler(savebox, action_SAVE_AS_SAVE_TO_FILE, savefile_handler, (void *) saver);
    event_register_toolbox_handler(savebox, action_SAVE_AS_SAVE_COMPLETED, endsavefile_handler, 0);
    event_register_toolbox_handler(savebox, action_SAVE_AS_DIALOGUE_COMPLETED, cancelsavefile_handler, 0);

    wimp_get_pointer_info(&mouse.pointer);

    mouse.pos.top_left.x-=64;
    mouse.pos.top_left.y+=64;

    toolbox_show_object(toolbox_SHOW_AS_MENU, savebox, toolbox_POSITION_TOP_LEFT,
                        &mouse.pos, toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    file_dragged=escape_pressed=0;

    while (!file_dragged && !escape_pressed)
        event_poll(0, 0, 0);

    event_deregister_toolbox_handler(savebox, action_SAVE_AS_SAVE_TO_FILE, savefile_handler, (void *) saver);
    event_deregister_toolbox_handler(savebox, action_SAVE_AS_SAVE_COMPLETED, endsavefile_handler, 0);
    event_deregister_toolbox_handler(savebox, action_SAVE_AS_DIALOGUE_COMPLETED, cancelsavefile_handler, 0);

    toolbox_delete_object(NONE, savebox);

    if (escape_pressed)
        return 1;

    return 0;
}

static errr askforfile_hook(cptr defname, int row, func_errr filer, int mode)
{
    switch (mode)
    {
      case O_RDONLY:
        return loadfile_hook(defname, row, filer);

      case O_WRONLY:
      case O_RDWR:
        return savefile_hook(defname, row, filer);
    }

    return -1;
}
#endif

static int defaultcols_handler(bits event_code, toolbox_action *event,
                               toolbox_block *id, void *handle)
{
    memcpy(&palette, &default_palette, sizeof default_palette);
    palette.entries[TERM_CURSOR]=0x00ffff00;

    palette_handler(0, 0);

    update_window(&screen, 0, 0, 80, 24);
    update_window(&choice, 0, 0, 80, 24);
    update_window(&recall, 0, 0, 80, 24);
    update_window(&mirror, 0, 0, 80, 24);

    /* Force the over-clever refresh routine to actually replot
       all characters */
    memset(screen.olddisp, 0, sizeof screen.olddisp);
    memset(choice.olddisp, 0, sizeof choice.olddisp);
    memset(recall.olddisp, 0, sizeof recall.olddisp);
    memset(mirror.olddisp, 0, sizeof mirror.olddisp);
    memset(screen.froshed, 1, sizeof screen.froshed);
    memset(choice.froshed, 1, sizeof choice.froshed);
    memset(recall.froshed, 1, sizeof recall.froshed);
    memset(mirror.froshed, 1, sizeof mirror.froshed);

    refresh_windows();

    return 1;
}


/*************************************************************
 *                                                           *
 *     Handlers for various wimp and toolbox events          *
 *                                                           *
 *************************************************************/

static int palette_handler(wimp_message *message, void *handle)
{
    int col;

    os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_NCOLOUR, (int *) &ncolours);
    for (col=0; col<256; col++)
        coltable[col]=colourtrans_return_colour_number(palette.entries[col]);

    return 1;
}

static int proginfo_handler(bits event_code, toolbox_action *event,
                            toolbox_block *id, void *handle)
{
    proginfo_set_version(NONE, id->this_obj, VERSION);

    return 1;
}

static int iconbar_handler(bits event_code, toolbox_action *event,
                           toolbox_block *id, void *handle)
{
    if (term_recall)
        window_to_front(recall.wimp);
    if (term_choice)
        window_to_front(choice.wimp);
    if (term_mirror)
        window_to_front(mirror.wimp);

    window_to_front(screen.wimp);
    grabcaret();

    return 1;
}

static int showfilemenu_handler(bits event_code, toolbox_action *event,
                                   toolbox_block *id, void *handle)
{
    menu_set_fade(NONE, id->this_obj, menu_New, !initialised || game_in_progress);
    menu_set_fade(NONE, id->this_obj, menu_Save, !character_generated);

    return 1;
}

static int showcoloursmenu_handler(bits event_code, toolbox_action *event,
                                   toolbox_block *id, void *handle)
{
    menu_set_tick(NONE, id->this_obj, menu_Dithered, ncolours < 65535 && !solid_colours);
    menu_set_fade(NONE, id->this_obj, menu_Dithered, ncolours >= 65535);

    return 1;
}

static int showwindowsmenu_handler(bits event_code, toolbox_action *event,
                                   toolbox_block *id, void *handle)
{
    /* Bit hacky */
    menu_set_tick(NONE, id->this_obj, menu_Angband, 1);
    menu_set_tick(NONE, id->this_obj, menu_Recall, term_recall != 0);
    menu_set_tick(NONE, id->this_obj, menu_Choice, term_choice != 0);
    menu_set_tick(NONE, id->this_obj, menu_Mirror, term_mirror != 0);

    return 1;
}

static int coloursmenu_handler(bits event_code, toolbox_action *event,
                               toolbox_block *id, void *handle)
{
    int tick;

    switch (id->this_cmp)
    {
      case menu_Dithered:
        tick=!menu_get_tick(NONE, id->this_obj, menu_Dithered);
        menu_set_tick(NONE, id->this_obj, menu_Dithered, tick);
        solid_colours=!tick;
        update_window(&screen, 0, 0, 80, 24);
        update_window(&choice, 0, 0, 80, 24);
        update_window(&recall, 0, 0, 80, 24);
        update_window(&mirror, 0, 0, 80, 24);
        /* Force the over-clever refresh routine to actually replot
           all characters */
        memset(screen.oldcol, 0, sizeof screen.oldcol);
        memset(choice.oldcol, 0, sizeof choice.oldcol);
        memset(recall.oldcol, 0, sizeof recall.oldcol);
        memset(mirror.oldcol, 0, sizeof mirror.oldcol);
        memset(screen.froshed, 1, sizeof screen.froshed);
        memset(choice.froshed, 1, sizeof choice.froshed);
        memset(recall.froshed, 1, sizeof recall.froshed);
        memset(mirror.froshed, 1, sizeof mirror.froshed);
        refresh_windows();
        return 1;
    }

    return 0;
}

static int openwindow_handler(bits event_code, toolbox_action *event,
                              toolbox_block *id, void *handle)
{
    term_data *t=(term_data *) handle;

    menu_set_tick(NONE, id->this_obj, id->this_cmp, 1);

    toolbox_show_object(NONE, t->window, toolbox_POSITION_DEFAULT, 0,
                        toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    return 1;
}

static int suppwin_handler(bits event_code, toolbox_action *event,
                           toolbox_block *id, void *handle)
{
    term_data *t=(term_data *) handle;

    switch (event_code)
    {
      case action_WINDOW_ABOUT_TO_BE_SHOWN:
        if (t==&recall)
            term_recall=&t->t;
        else if (t==&choice)
            term_choice=&t->t;
        else if (t==&mirror)
            term_mirror=&t->t;
        else if (t==&screen)
        {
            if (term_recall)
                window_to_front(recall.wimp);
            if (term_choice)
                window_to_front(choice.wimp);
        }
        break;

      case action_WINDOW_DIALOGUE_COMPLETED:
        if (t==&recall)
            term_recall=0;
        else if (t==&choice)
            term_choice=0;
        else if (t==&mirror)
            term_mirror=0;
        else if (t==&screen)
        {
            window_hide(recall.wimp);
            window_hide(choice.wimp);
            window_hide(mirror.wimp);
        }
        break;
    }

    return 1;
}

static int quitbutton_handler(bits event_code, toolbox_action *event,
                              toolbox_block *id, void *handle)
{
    if (shutting_down)
    {
        wimp_key event;
        event.c=wimp_KEY_F12+wimp_KEY_SHIFT+wimp_KEY_CONTROL;
        wimp_send_message(wimp_KEY_PRESSED, (wimp_message *) &event, shutting_down);
        shutting_down=0;
    }

    quit(NULL);

    return 1;
}

static int delquit_handler(bits event_code, toolbox_action *event,
                           toolbox_block *id, void *handle)
{
    event_deregister_toolbox_handler(id->this_obj, action_QUIT_QUIT,
                                         quitbutton_handler, 0);
    event_deregister_toolbox_handler(id->this_obj, action_QUIT_DIALOGUE_COMPLETED,
                                         delquit_handler, 0);

    toolbox_delete_object(NONE, id->this_obj);

    grabcaret();

    return 1;
}

static void popup_quitbox(void)
{
    toolbox_o q;

    q=toolbox_create_object(NONE, (toolbox_id) "Quit");
    ShowCentred(toolbox_SHOW_AS_MENU, q, toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    event_register_toolbox_handler(q, action_QUIT_QUIT, quitbutton_handler, 0);
    event_register_toolbox_handler(q, action_QUIT_DIALOGUE_COMPLETED, delquit_handler, 0);
}

static int quitmenu_handler(bits event_code, toolbox_action *event,
                            toolbox_block *id, void *handle)
{
    shutting_down=0;

    if (game_in_progress && character_generated)
    {
        /* Forget messages */
        msg_flag = FALSE;

        /* Save the game */
        do_cmd_save_game();
    }

    /* Quit */

    quit(NULL);

    return 1;
}

static int quit_handler(wimp_message *message, void *handle)
{
    return quitbutton_handler(0, 0, 0, 0);
}

static int prequit_handler(wimp_message *message, void *handle)
{
    if (game_in_progress && character_generated)
    {
        /* Allow user to cancel "dangerous" exit */
        if (!(message->data.prequit.flags & wimp_PRE_QUIT_TASK_ONLY))
                shutting_down=message->sender;
        else
                shutting_down=0;

        message->your_ref=message->my_ref;
        wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, message, message->sender);
        popup_quitbox();
    }

    return 1;
}

static int new_handler(bits event_code, toolbox_action *event,
                       toolbox_block *id, void *handle)
{
    if (game_in_progress)
    {
        plog("You can't start a new game while you're still playing!");
        return 1;
    }

    osfscontrol_canonicalise_path("<Angband$Dir>.^.SavedGame", savefile, 0, 0,
                                                               sizeof savefile);

    strcpy(savefile, translate_name(savefile));

    game_in_progress=1;
    flush();
    play_game(TRUE);
    quit(NULL);

    return 1;
}


static int key_handler(wimp_event_no event_code, wimp_block *event,
                       toolbox_block *id, void *handle)
{
    /*
     * This ensures that typing 4 into a Save as dialogue box
     * doesn't move your character left, and pressing F12
     * always brings up the command line.
     */
    if (id->this_obj != screen.window)
    {
        wimp_process_key(event->key.c);
        return 1;
    }

    switch (event->key.c)
    {
        /* F12 is always sent to the Wimp */
        case wimp_KEY_F12:
        case wimp_KEY_F12+wimp_KEY_SHIFT:
        case wimp_KEY_F12+wimp_KEY_CONTROL:
        case wimp_KEY_F12+wimp_KEY_SHIFT+wimp_KEY_CONTROL:
            wimp_process_key(event->key.c);
            break;
        default:
            /*
             * Hack - allow Shift & Ctrl as modifiers to the keypad
             */
            if (event->key.c <= '9' && event->key.c >= '0')
            {
                if (osbyte1(osbyte_IN_KEY, 0xFF, 0xFF))     /* Is Shift held down? */
                    event->key.c |= 0x800;
                if (osbyte1(osbyte_IN_KEY, 0xFE, 0xFF))     /* Is Ctrl held down? */
                    event->key.c |= 0x400;
            }
            /* Special keys are sent encoded */
            if (event->key.c >= 0x100 || event->key.c == 31)
            {
                const static char hex[]="0123456789ABCDEF";

            	Term_keypress(31);
            	Term_keypress(hex[(event->key.c & 0xF00) >> 8]);
            	Term_keypress(hex[(event->key.c & 0xF0) >> 4]);
            	Term_keypress(hex[event->key.c & 0xF]);
            	Term_keypress(13);
            }
            else
    	    	Term_keypress(event->key.c);
    	    key_pressed=1;
    	    if (event->key.c == 27)
    	    	escape_pressed=1;
    	    break;
    }

    return 1;
}

static int mouse_handler(wimp_event_no event_code, wimp_block *event,
                         toolbox_block *id, void *handle)
{
    grabcaret();

    return 1;
}

static int caret_handler(wimp_event_no event_code, wimp_block *event,
                         toolbox_block *id, void *handle)
{
    have_caret = event_code==wimp_GAIN_CARET;

    if (event_code == wimp_GAIN_CARET && !have_caret_entity)
    {
    	wimp_message message;

        message.size=24;
        message.your_ref=0;
        message.action=message_CLAIM_ENTITY;
        message.data.claim_entity.flags=3; /* Caret / selection */
        wimp_send_message(wimp_USER_MESSAGE, &message, wimp_BROADCAST);
        have_caret_entity=1;
    }

    return 1;
}

static int entity_handler(wimp_message *message, void *handle)
{
    if (message->data.claim_entity.flags & 3)
        have_caret_entity=0;

    return 1;
}

static int savechoices_handler(bits event_code, toolbox_action *event,
    	    	    	       toolbox_block *id, void *handle)
{
    savechoices();

    return 1;
}

static int create_handler(bits event_code, toolbox_action *event,
                          toolbox_block *id, void *handle)
{
    const char *name=event->data.created.name;

    if (strcmp(name, "File") == 0)
    {
        event_register_toolbox_handler(id->this_obj, action_MENU_ABOUT_TO_BE_SHOWN,
                                        showfilemenu_handler, 0);
        return 1;
    }

    if (strcmp(name, "Colours") == 0)
    {
        event_register_toolbox_handler(id->this_obj, action_MENU_ABOUT_TO_BE_SHOWN,
                                        showcoloursmenu_handler, 0);
        event_register_toolbox_handler(id->this_obj, action_MENU_SELECTION,
                                        coloursmenu_handler, 0);
        return 1;
    }

    if (strcmp(name, "Windows") == 0)
    {
        event_register_toolbox_handler(id->this_obj, action_MENU_ABOUT_TO_BE_SHOWN,
                                        showwindowsmenu_handler, 0);
        return 1;
    }

    if (strcmp(name, "SaveAs") == 0)
    {
        event_register_toolbox_handler(id->this_obj, action_SAVE_AS_ABOUT_TO_BE_SHOWN,
                                                    showsave_handler, 0);
        event_register_toolbox_handler(id->this_obj, action_SAVE_AS_SAVE_TO_FILE,
                                                    save_handler, 0);
        event_register_toolbox_handler(saveas_get_window_id(NONE, id->this_obj),
                                        action_ACTION_BUTTON_SELECTED,
                                        savebutton_handler, id->this_obj);
        return 1;
    }

    return 0;
}

/*************************************************************
 *                                                           *
 *    The difficult bit - keeping the windows up-to-date     *
 *                                                           *
 *************************************************************/

static void do_redraw(wimp_draw *redraw, int more, term_data *t, int blank)
{
    int x0, x1, y0, y1, x, y, rx, ry, c;
    int ox;
    int nodither = solid_colours || ncolours > 255;
    static char olddef[10]={23,135};
    const static char blockdef[10]={23,135,255,255,255,255,255,255,255,255};
    char **ch=t->t.old->c;
    byte **a=t->t.old->a;
    char buf[80];

    /* Hackery - define character 135 (not normally defined) as a solid block */
    osword_read_char_definition((osword_char_definition_block *)(olddef+1));
    os_writen(blockdef, 10);

    while (more)
    {
        rx=redraw->box.x0-redraw->xscroll;
        ry=redraw->box.y1-redraw->yscroll;
        x0=(redraw->clip.x0-rx)/16;
        x1=(redraw->clip.x1-rx-1)/16;
        y0=(768-(redraw->clip.y1-ry))/32;
        y1=(767-(redraw->clip.y0-ry))/32;

        if (blank)
        {
            /*
             * We're updating the window because the contents have changed
             * This code is rather complex, but it's FAST, and doesn't
             * flicker (particularly important when "Draw Viewable Lite brightly"
             * is on)
             */

            /* Set the background colour */
            if (nodither)
                os_set_colour(os_COLOUR_SET_BG, coltable[TERM_DARK]);
            else
                colourtrans_set_gcol(palette.entries[TERM_DARK],
                                     colourtrans_USE_ECFS|colourtrans_SET_BG,
                                     os_ACTION_OVERWRITE, 0);

            c=-1;

            for (y=y0; y<=y1; y++)
            {
                if (!t->froshed[y])
                    continue;

    	    	for (x=x0; x<=x1; x++)
    	    	    buf[x]=ch[y][x] >= ' ' ? ch[y][x] : ' ';

                for (x=x0; x<=x1; )
                {
                    /* Skip past any chars that haven't changed at all*/
                    while (buf[x] == t->olddisp[y][x] &&
                           a[y][x] == t->oldcol[y][x] && x<=x1)
                        x++;

                    if (x>x1)
                        break;

                    /* We're pointing at a character that has changed in some way */

                    /* Blank any newly blanked characters */
                    ox=x;
                    while (buf[x] == ' ' && t->olddisp[y][x] != ' ')
                        x++;

                    if (x>ox)
                    {
                        os_plot(os_MOVE_TO, ox*16+rx, 767-y*32+ry);
                        os_plot(os_PLOT_BG_BY | os_PLOT_RECTANGLE, 16*(x-ox)-1, -31);
                        continue;
                    }

                    /* Change the drawing colour if necessary */
                    if (a[y][x] != c)
                    {
                        c=a[y][x];
                        if (nodither)
                            os_set_colour(NONE, coltable[c]);
                        else
                            colourtrans_set_gcol(palette.entries[c], colourtrans_USE_ECFS,
                                                 os_ACTION_OVERWRITE, 0);
                    }

                    /* Find how many characters have only changed colour */
                    while (x<=x1 &&
                           buf[x] == t->olddisp[y][x] &&
                           a[y][x] == c && t->oldcol[y][x] != c)
                        x++;

                    if (x>ox)
                    {
                        os_plot(os_MOVE_TO, ox*16+rx, 767-y*32+ry);
                        os_writen(buf+ox, x-ox);
                        continue;
                    }

                    /* Now do characters that have actually changed symbol */
                    while (buf[x] != t->olddisp[y][x] &&
                           a[y][x] == c && x<=x1)
                        x++;

                    os_plot(os_MOVE_TO, x*16+rx-1, 767-y*32+ry-31);
                    os_plot(os_PLOT_BG_BY | os_PLOT_RECTANGLE, -(16*(x-ox)-1), +31);
                    os_writen(buf+ox, x-ox);
                }
            }
        }
        else
        {
            /* We're redrawing the window because it's just been uncovered */

            /* Clear the background */
            if (nodither)
                os_set_colour(os_COLOUR_SET_BG, coltable[TERM_DARK]);
            else
                colourtrans_set_gcol(palette.entries[TERM_DARK],
                                     colourtrans_USE_ECFS|colourtrans_SET_BG,
                                     os_ACTION_OVERWRITE, 0);

            os_plot(os_MOVE_TO, redraw->clip.x0, redraw->clip.y0);
            os_plot(os_PLOT_BG_TO|os_PLOT_RECTANGLE, redraw->clip.x1-1, redraw->clip.y1-1);

            for (y=y0; y<=y1; y++)
            {
                os_plot(os_MOVE_TO, x0*16+rx, 767-y*32+ry);

    	    	for (x=x0; x<=x1; x++)
    	    	    buf[x]=ch[y][x] >= ' ' ? ch[y][x] : ' ';

                for (x=x0; x<=x1; )
                {
                    c=a[y][x];
                    if (nodither)
                        os_set_colour(NONE, coltable[c]);
                    else
                        colourtrans_set_gcol(palette.entries[c], colourtrans_USE_ECFS,
                                             os_ACTION_OVERWRITE, 0);

                    ox=x;
                    while (x<=x1 && (buf[x] == ' ' || a[y][x] == c))
                        x++;

                    if (ox!=x)
                        os_writen(buf+ox, x-ox);
                }
            }
        }

        if (t->curs_vis && x0<=t->curs_x && t->curs_x<=x1 &&
                           y0<=t->curs_y && t->curs_y<=y1 && t==&screen)
        {
            if (nodither)
                os_set_colour(NONE, coltable[TERM_CURSOR]);
            else
                colourtrans_set_gcol(palette.entries[TERM_CURSOR], colourtrans_USE_ECFS,
                                     os_ACTION_OVERWRITE, 0);
            os_plot(os_MOVE_TO, t->curs_x*16+rx, 767-t->curs_y*32+ry);
            os_plot(os_PLOT_BY, 15, 0);
            os_plot(os_PLOT_BY, 0, -31);
            os_plot(os_PLOT_BY, -15, 0);
            os_plot(os_PLOT_BY, 0, 31);
        }

        more=wimp_get_rectangle(redraw);
    }

    /* Restore original definition of character 135 */
    os_writen(olddef, 10);
}

static int redraw_handler(int event_code, wimp_block *event, toolbox_block *id, void *handle)
{
    wimp_draw redraw;
    int more;

    redraw.w=event->redraw.w;

    more=wimp_redraw_window(&redraw);

    do_redraw(&redraw, more, (term_data *) handle, FALSE);

    return 1;
}

static void update_window(term_data *t, int x0, int y0, int x1, int y1)
{
    if (x0<t->changed.x0)
        t->changed.x0=x0;
    if (x1>t->changed.x1)
        t->changed.x1=x1;
    if (y0<t->changed.y0)
        t->changed.y0=y0;
    if (y1>t->changed.y1)
        t->changed.y1=y1;
}


static void refresh_window(term_data *t)
{
    wimp_draw redraw;
    int more, i;

    if (t->ocx >= 0)
        t->olddisp[t->ocy][t->ocx]=0;

    t->ocx=-1;

    if (t->changed.x1 <= t->changed.x0 || t->changed.y1 <= t->changed.y0)
        return;

    redraw.w=t->wimp;
    redraw.box.x0=t->changed.x0*16;
    redraw.box.x1=t->changed.x1*16;
    redraw.box.y0=768-t->changed.y1*32;
    redraw.box.y1=768-t->changed.y0*32;

    more=wimp_update_window(&redraw);

    do_redraw(&redraw, more, t, TRUE);

    t->changed.x0=256;
    t->changed.y0=256;
    t->changed.x1=0;
    t->changed.y1=0;

    for (i=0; i<24; i++)
    {
    	memcpy(t->olddisp[i], t->t.old->c[i], 80);
    	memcpy(t->oldcol[i], t->t.old->a[i], 80);
    }

    memset(t->froshed, 0, sizeof t->froshed);
}

static void refresh_windows(void)
{
    refresh_window(&screen);
    refresh_window(&recall);
    refresh_window(&choice);
    refresh_window(&mirror);
}

/*************************************************************
 *                                                           *
 *         Loading and saving our window positions           *
 *                                                           *
 *************************************************************/

static void load_window_pos(FILE *f, term_data *t)
{
    wimp_open open;
    int on;

    fscanf(f, "%d %d %d %d %d %d %d", &on, &open.visible.x0, &open.visible.y0,
                                           &open.visible.x1, &open.visible.y1,
                                           &open.xscroll, &open.yscroll);
    open.next=screen.wimp;
    if (on)
        toolbox_show_object(NONE, t->window, toolbox_POSITION_FULL,
                            (toolbox_position *) &open.visible,
                            toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
    else
    {
        open.w=t->wimp;
        wimp_open_window(&open);
        wimp_close_window(t->wimp);
    }
}

static void show_windows(void)
{
    FILE *f;
    toolbox_position pos;
    int version, i;

    f=my_fopen("<Angband$ChoicesFile>", "r");

    if (f)
    {
        fscanf(f, "%d", &version);
        if (version < 2 || version > 5)
        {
            my_fclose(f);
            f=0;
        }
    }

    if (f==0)
    {
        /* No saved window positions - open only the playing window */
        toolbox_show_object(NONE, screen.window, toolbox_POSITION_DEFAULT, 0,
                            toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);
        return;
    }

    fscanf(f, "%d %d", &pos.top_left.x, &pos.top_left.y);
    toolbox_show_object(NONE, screen.window, toolbox_POSITION_TOP_LEFT,
                        &pos, toolbox_NULL_OBJECT, toolbox_NULL_COMPONENT);

    load_window_pos(f, &recall);
    load_window_pos(f, &choice);
    if (version >= 4)
    	load_window_pos(f, &mirror);

    fscanf(f, "%d", &solid_colours);

    if (version>=3 && version<=4)
    {
        for (i=0; i<16; i++)
            fscanf(f, "%x", &palette.entries[i]);

        fscanf(f, "%x", &palette.entries[TERM_CURSOR]);
    }

    my_fclose(f);
}

static void save_window_pos(FILE *f, term_data *t, void *active)
{
    wimp_window_state state;

    state.w=t->wimp;
    wimp_get_window_state(&state);
    fprintf(f, "%d %d %d %d %d %d %d\n", active ? 1 : 0,
                                         state.visible.x0, state.visible.y0,
                                         state.visible.x1, state.visible.y1,
                                         state.xscroll, state.yscroll);
}

static void savechoices(void)
{
    FILE *f;
    wimp_window_state state;

    f=my_fopen("<Angband$ChoicesFile>", "w");

    if (f==0)
        /* Don't do anything - it's probably in a read-only location */
        return;

    /* File format version */
    fprintf(f, "5\n");

    state.w=screen.wimp;
    wimp_get_window_state(&state);
    fprintf(f, "%d %d\n", state.visible.x0, state.visible.y1);

    save_window_pos(f, &recall, term_recall);
    save_window_pos(f, &choice, term_choice);
    save_window_pos(f, &mirror, term_mirror);

    fprintf(f, "%d\n", solid_colours);

    /*for (i=0; i<17; i++)
        fprintf(f, "%08X\n", palette.entries[i]);*/

    my_fclose(f);
}


/*************************************************************
 *                                                           *
 *        Initialisation and finalisation routines           *
 *                                                           *
 *************************************************************/
static void final_acn(void)
{
    int i;

    for (i=0; i<openfiles; i++)
        xosfind_close(filehandle[i]);
}

static void init_stuff(void)
{
    char buf[1024];

    strcpy(buf, "Angband:");

    init_file_paths(buf);
}


int main(int argc, char *argv[])
{
    int i;

    signal(SIGOSERROR, oserror_handler);

    quit_aux = quit_hook;
    plog_aux = plog_hook;
    core_aux = core_hook;

    /* XXX XXX XXX Not ready yet */
    /* askforfile_aux = askforfile_hook; */

    /* Get the file paths */
    init_stuff();

    /* Initialise the Toolbox, and load user preferences */
    init_acn();

    /* Set up the colour table */
    for (i=0; i<256; i++)
    {
        colour_table[i][1] = (palette.entries[i] >> 8) & 0xFF;
        colour_table[i][2] = (palette.entries[i] >> 16) & 0xFF;
        colour_table[i][3] = palette.entries[i] >> 24;
    }

    /* Catch nasty signals */
    signals_init();

    /* Display the 'news' file */
    show_news();

    /* Initialize the arrays */
    init_some_arrays();

    /* No name (yet) */
    strcpy(player_name, "");

    /* Hack -- assume wizard permissions */
    can_be_wizard = TRUE;

    /* Hack -- Use the "pref-acn.prf" file */
    ANGBAND_SYS = "acn";

    initialised=1;

    if (argc==1)
    {
        /* Prompt the user */
        prt("[Double-click on a saved game, or choose 'New' from the 'File' menu]", 23, 5);
        Term_fresh();
    }

    if (argc >= 2)
    {
        strcpy(savefile, translate_name(argv[1]));
        game_in_progress=1;
        pause_line(23);
        flush();
        play_game(FALSE);
        quit(NULL);
    }

    for (;;)
        event_poll(0,0,0);

    return 0;
}

static void term_data_link(term_data *td, int keys)
{
    term *t= &td->t;

    term_init(t, 80, 24, keys);

    t->soft_cursor = FALSE;
    t->scan_events = FALSE;
    t->dark_blanks = TRUE;

    t->init_hook = Term_init_acn;

    t->xtra_hook = Term_xtra_acn;
    t->wipe_hook = Term_wipe_acn;
    t->curs_hook = Term_curs_acn;
    t->text_hook = Term_text_acn;

    t->data = (vptr) td;

    Term_activate(t);
}


static errr init_acn(void)
{
    errr e=0;

    wimp_event_no event;
    os_error *r;

    const
    static wimp_MESSAGE_LIST(7) messages={message_PREQUIT,
                                          message_PALETTE_CHANGE,
                                          message_MODE_CHANGE,
                                          message_DATA_LOAD,
                                          message_DATA_OPEN,
                                          message_CLAIM_ENTITY,
                                          0};

    const
    static toolbox_ACTION_LIST(27) action_nos={action_ERROR,
                                               action_OBJECT_AUTO_CREATED,
                                               action_MENU_ABOUT_TO_BE_SHOWN,
                                               action_MENU_SELECTION,
                                               action_MENU_SUB_MENU,
                                               action_WINDOW_ABOUT_TO_BE_SHOWN,
                                               action_WINDOW_DIALOGUE_COMPLETED,
                                               action_SAVE_AS_ABOUT_TO_BE_SHOWN,
                                               action_SAVE_AS_SAVE_TO_FILE,
                                               action_SAVE_AS_SAVE_COMPLETED,
                                               action_SAVE_AS_DIALOGUE_COMPLETED,
    	    	    	    	    	       action_PROG_INFO_ABOUT_TO_BE_SHOWN,
                                               action_QUIT_QUIT,
                                               action_QUIT_DIALOGUE_COMPLETED,
                                               action_ACTION_BUTTON_SELECTED,
                                               action_Quit,
                                               action_Iconbar,
                                               action_New,
                                               action_Save,
                                               action_OpenRecall,
                                               action_OpenChoice,
                                               action_OpenAngband,
                                               action_OpenMirror,
                                               action_DefaultCols,
    	    	    	    	    	       action_SaveChoices,
                                               0};

    toolbox_initialise(NONE, wimp_VERSION_RO3, (wimp_message_list *) &messages,
                       (toolbox_action_list *) &action_nos,
                       getenv("Angband$Dir"), &mfd, &id_block, &wimpver, 0);

    recall.window=toolbox_create_object(NONE, (toolbox_id)"Recall");
    recall.wimp=window_get_wimp_handle(NONE, recall.window);
    choice.window=toolbox_create_object(NONE, (toolbox_id)"Choice");
    choice.wimp=window_get_wimp_handle(NONE, choice.window);
    screen.window=toolbox_create_object(NONE, (toolbox_id)"Screen");
    screen.wimp=window_get_wimp_handle(NONE, screen.window);
    mirror.window=toolbox_create_object(NONE, (toolbox_id)"Mirror");
    mirror.wimp=window_get_wimp_handle(NONE, mirror.window);

    memcpy(&palette, &default_palette, sizeof default_palette);
    palette.entries[TERM_CURSOR]=0x00ffff00;

    show_windows();

    r=event_initialise(&id_block);

    if (!r)
        r=event_register_message_handler(message_QUIT, quit_handler, 0);

    if (!r)
        r=event_register_message_handler(message_PALETTE_CHANGE, palette_handler, 0);

    if (!r)
        r=event_register_message_handler(message_PREQUIT, prequit_handler, 0);

    if (!r)
        r=event_register_message_handler(message_DATA_LOAD, dataload_handler, 0);

    if (!r)
        r=event_register_message_handler(message_DATA_OPEN, dataload_handler, 0);

    if (!r)
        r=event_register_message_handler(message_CLAIM_ENTITY, entity_handler, 0);

    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_ERROR,
                                                    error_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_Quit,
                                                    quitmenu_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_Iconbar,
                                                    iconbar_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_New,
                                                    new_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_OpenRecall,
                                                    openwindow_handler, &recall);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_OpenChoice,
                                                    openwindow_handler, &choice);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_OpenAngband,
                                                    openwindow_handler, &screen);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_OpenMirror,
                                                    openwindow_handler, &mirror);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_DefaultCols,
                                                    defaultcols_handler, 0);
    if (!r)
    	r=event_register_toolbox_handler(event_ANY, action_SaveChoices,
    	                                            savechoices_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_OBJECT_AUTO_CREATED,
                                                    create_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_Save,
                                                    defaultsave_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(event_ANY, action_SAVE_AS_SAVE_COMPLETED,
                                                    savecomplete_handler, 0);
    if (!r)
    	r=event_register_toolbox_handler(event_ANY, action_PROG_INFO_ABOUT_TO_BE_SHOWN,
    	                                    	    proginfo_handler, 0);
    if (!r)
        r=event_register_toolbox_handler(recall.window, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                         suppwin_handler, &recall);
    if (!r)
        r=event_register_toolbox_handler(choice.window, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                         suppwin_handler, &choice);
    if (!r)
        r=event_register_toolbox_handler(mirror.window, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                         suppwin_handler, &mirror);
    if (!r)
        r=event_register_toolbox_handler(screen.window, action_WINDOW_ABOUT_TO_BE_SHOWN,
                                         suppwin_handler, &screen);
    if (!r)
        r=event_register_toolbox_handler(recall.window, action_WINDOW_DIALOGUE_COMPLETED,
                                         suppwin_handler, &recall);
    if (!r)
        r=event_register_toolbox_handler(choice.window, action_WINDOW_DIALOGUE_COMPLETED,
                                         suppwin_handler, &choice);
    if (!r)
        r=event_register_toolbox_handler(mirror.window, action_WINDOW_DIALOGUE_COMPLETED,
                                         suppwin_handler, &mirror);
    if (!r)
        r=event_register_toolbox_handler(screen.window, action_WINDOW_DIALOGUE_COMPLETED,
                                         suppwin_handler, &screen);
    if (!r)
        r=event_register_wimp_handler(screen.window, wimp_MOUSE_CLICK,
                                                     mouse_handler, 0);
    if (!r)
        r=event_register_wimp_handler(event_ANY, wimp_KEY_PRESSED,
                                                 key_handler, 0);
    if (!r)
        r=event_register_wimp_handler(screen.window, wimp_REDRAW_WINDOW_REQUEST,
                                                     redraw_handler, &screen);
    if (!r)
        r=event_register_wimp_handler(choice.window, wimp_REDRAW_WINDOW_REQUEST,
                                                     redraw_handler, &choice);
    if (!r)
        r=event_register_wimp_handler(recall.window, wimp_REDRAW_WINDOW_REQUEST,
                                                     redraw_handler, &recall);
    if (!r)
        r=event_register_wimp_handler(mirror.window, wimp_REDRAW_WINDOW_REQUEST,
                                                     redraw_handler, &mirror);
    if (!r)
        r=event_register_wimp_handler(screen.window, wimp_GAIN_CARET,
                                                     caret_handler, 0);
    if (!r)
        r=event_register_wimp_handler(screen.window, wimp_LOSE_CARET,
                                                     caret_handler, 0);
    if (r)
        quit(r->errmess);

    atexit(final_acn);

    term_data_link(&screen, 256);

    term_data_link(&choice, 16);
    term_data_link(&recall, 16);
    term_data_link(&mirror, 16);

    if (toolbox_get_object_info(NONE, choice.window) & toolbox_INFO_SHOWING)
        term_choice=&choice.t;

    if (toolbox_get_object_info(NONE, recall.window) & toolbox_INFO_SHOWING)
        term_recall=&recall.t;

    if (toolbox_get_object_info(NONE, mirror.window) & toolbox_INFO_SHOWING)
        term_mirror=&mirror.t;

    /* Check initial palette */
    palette_handler(0, 0);

    term_screen=&screen.t;

    if (!e)
        e=Term_activate(&screen.t);

    if (e)
        return e;

    /* Poll until a null event - enables window to come up in time
       for status messages */
    event_set_mask(NONE);
    do
    {
        event_poll(&event, 0, 0);
    } while (event != wimp_NULL_REASON_CODE);

    event_set_mask(wimp_MASK_NULL |
                   wimp_MASK_LEAVING |
                   wimp_MASK_ENTERING |
                   wimp_MASK_POLLWORD |
                   wimp_MASK_ACKNOWLEDGE);

    return 0;
}


/*************************************************************
 *                                                           *
 *            Various Unix-like support routines for         *
 *                  the main Angband source                  *
 *                                                           *
 *************************************************************/
void delay(int x)
{
    os_t t;

    t=os_read_monotonic_time();

    while (os_read_monotonic_time() - t < x/10)
        ;
}

/* translate_name: translate a filename from file/data/k_list.raw form to
                   file.data.k_list/raw form.

   This is necessary because the main source allows the separator to be
   defined, but not the extension character.
*/

static const char *translate_name(const char *path)
{
    static char buffer[256];
    char c, *p;

    p=buffer;

    while (c=*path++)
    {
        switch (c)
        {
          case '/':
            *p++='.';
            break;
          case '.':
            *p++='/';
            break;
          default:
            *p++=c;
            break;
        }
    }
    *p='\0';

    /*
     * Hackery - when saving a game, the old game is renamed as
     * "SavedGame.old", the new one is saved as "SavedGame.new",
     * "SavedGame.old" is deleted, "SavedGame.new" is renamed
     * as "SavedGame". This will go majorly wrong if on a
     * FileCore filing system (leafnames <= 10 chars), your
     * saved game file has a leafname > 8 chars.
     */
    if ((p=strstr(buffer, "/old")) || (p=strstr(buffer, "/new")))
    {
        char *q=strrchr(buffer, '.');

        if (q)
            if (p-q > 6)
                memmove(q+6, p, 5);

        _ftype=osfile_TYPE_ANGBAND;
    }
    else
        _ftype=osfile_TYPE_DATA;

    return buffer;
}



FILE *my_fopen(const char *filename, const char *mode)
{
    FILE *f;
    const char *name=translate_name(filename);

    f=fopen(name, mode);

    if (f && strstr(mode, "wb"))
        osfile_set_type(name, _ftype);

    return f;
}

errr my_fclose(FILE *fff)
{
    /* Close, check for error */
    if (fclose(fff)) return (1);

    /* Success */
    return (0);
}

int remove_acn(const char *filename)
{
    return remove(translate_name(filename));
}

int rename_acn(const char *old, const char *new)
{
    char buffer[256];

    /* Necessary because of single buffer inside translate_name */

    strcpy(buffer, translate_name(old));

    return rename(buffer, translate_name(new));
}

int fd_open(cptr path, int flags, int mode)
{
    int open_type;
    os_f f;
    os_error *e;
    const char *realpath=translate_name(path);

    switch (flags & 0xF)
    {
      case O_RDONLY:
        open_type=OSFind_Openin;
        break;
      case O_WRONLY:
      case O_RDWR:
        open_type=OSFind_Openup;
        break;
    }

    e=xosfind_openin(osfind_ERROR_IF_DIR, realpath, 0, &f);

    if (e)
        return -1;

    if (f)
        osfind_close(f);

    if ((flags & O_CREAT) && f==0)
        open_type=OSFind_Openout;

    if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL) && f)
        return -1;

    switch (open_type)
    {
      case OSFind_Openin:
        e=xosfind_openin(osfind_ERROR_IF_DIR | osfind_ERROR_IF_ABSENT | osfind_NO_PATH,
                         realpath, 0, &f);
        break;

      case OSFind_Openup:
        e=xosfind_openup(osfind_ERROR_IF_DIR | osfind_ERROR_IF_ABSENT | osfind_NO_PATH,
                         realpath, 0, &f);
        break;

      case OSFind_Openout:
        e=xosfind_openout(osfind_ERROR_IF_DIR | osfind_ERROR_IF_ABSENT | osfind_NO_PATH,
                          realpath, 0, &f);
        if (!e)
            osfile_set_type(realpath, flags & O_BINARY ? _ftype : osfile_TYPE_TEXT);
        break;
    }

    if (e || f==0)
        return -1;

    /* if he opens >16 files, I can't be bothered to keep track */
    if (openfiles<16)
        filehandle[openfiles++]=f;

    if (flags & O_TRUNC)
        osargs_set_ext(f, 0);

    return (int) f;
}

errr fd_close(int d)
{
    os_error *e;
    os_f f=(os_f) d;
    int i;

    if (d <= 0) return -1;

    e=xosfind_close(f);

    if (e)
        return 1;

    openfiles--;

    for (i=0; filehandle[i] != f; i++)
        ;

    for (; i<openfiles; i++)
        filehandle[i]=filehandle[i+1];

    return 0;
}

/*int unlink(const char *path)
{
    xosfile_delete(translate_name(path), 0, 0, 0, 0, 0);

    return 0;
}

int access(const char *path, int mode)
{
    os_error *e;
    os_f f;

    e=xosfind_openin(osfind_ERROR_IF_DIR, translate_name(path), 0, &f);

    if (e || f==0)
        return -1;

    osfind_close(f);

    return 0;
}*/

errr fd_read(int d, char *buf, huge nbytes)
{
    os_error *e;
    int unread;

    if (d <= 0) return -1;

    e=xosgbpb_read((os_f) d, (byte *) buf, nbytes, &unread);

    if (e || unread)
        return 1;

    return 0;
}

errr fd_write(int d, const char *buf, huge nbytes)
{
    os_error *e;
    int unwritten;

    if (d <= 0) return -1;

    e=xosgbpb_write((os_f) d, (byte *) buf, nbytes, &unwritten);

    if (e || unwritten)
        return 1;

    return 0;
}

errr fd_seek(int fd, huge offset)
{
    os_error *e;

    if (fd < 0) return -1;

    e=xosargs_set_ptr((os_f) fd, (int) offset);
    if (e)
        return 1;

    return 0;
}

errr fd_lock(int fd, int what)
{
    return 0;
}
#endif /* __riscos */
